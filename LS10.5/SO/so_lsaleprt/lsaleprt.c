/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lsaleprt.c,v 5.3 2002/07/17 09:58:09 scott Exp $
|  Program Name  : (so_lsalesprt.c)                                 
|  Program Desc  : (Print Inventory Lost Sales File)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap    | Date Written  : 11/03/88         |
|---------------------------------------------------------------------|
| $Log: lsaleprt.c,v $
| Revision 5.3  2002/07/17 09:58:09  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:21:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:02  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/16 04:34:28  scott
| Updated to clean code and add app.schema
| Updated to ensure selection screen displays correctly with LS10-GUI
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lsaleprt.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lsaleprt/lsaleprt.c,v 5.3 2002/07/17 09:58:09 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define		BY_COMPANY	 (local_rec.byWhat [0] == 'C')
#define		BY_BRANCH	 (local_rec.byWhat [0] == 'B')
#define		BY_WAREHOUSE (local_rec.byWhat [0] == 'W')

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inlsRecord	inls_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;

	char	printerString [11];

	int		printerNumber 	= 1,
			firstTime 		= TRUE,
			foundInls 		= FALSE,
			envVarDbMcurr	= 0;

	char	systemDate [11], 
			defaultDate [11], 
			startDate [11], 
			endDate [11], 
			dataString [140], 
			previousWhNo [3], 
			currentWhNo [3], 
			previousBrNo [3], 
			currentBrNo [3], 
			areaCode [3], 
			saleCode [3], 
			reasonCode [3], 
			reasonDesc [61], 
			lostSaleDate [11], 
			previousItemNo [17], 
			currentItemNo [17];
	
	long	hhcuHash = 0L, 
			hhbrHash = 0L, 
			lsystemDate;

	float	qty 			= 0.0,
			itemQuantity	= 0.0,
			coBrQuantity 	= 0.0,
			brBrQuantity 	= 0.0,
			whQuantity 		= 0.0;

	double	extend 			= 0.00,
			value 			= 0.00,
			itemValue 		= 0.00,
			coBrValue 		= 0.00,
			brBrValue 		= 0.00,
			whValue 		= 0.00;

	FILE	*fout,
			*fin,
			*fsort;

	extern	int	TruePosition;

#include	<get_lpno.h>

/*============================ 
| Local & Screen Structures, |
============================*/
struct {
	char	dummy [11];
	long	startDate;
	long	endDate;
	char	back [2];
	char	backDesc [21];
	char	onite [2];
	char	oniteDesc [21];
	char	byWhat [2];
	char	byWhatDesc [21];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "byWhat", 	 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Company/Branch/Warehouse  ", "Enter C(ompany, B(ranch or W(arehouse ", 
		YES, NO, JUSTLEFT, "CBW", "", local_rec.byWhat}, 
	{1, LIN, "byWhatDesc", 	 4, 33, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.byWhatDesc}, 
	{1, LIN, "startDate", 	 5, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", defaultDate, "Start Date                ", "Default To 1st Day Of Current Month ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate}, 
	{1, LIN, "endDate", 	 6, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "End   Date                ", "Default To Current Date ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate}, 
	{1, LIN, "printerNumber", 	 8, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No                ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&printerNumber}, 
	{1, LIN, "back", 	 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background                ", "Enter Y(es) or N(o), ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 	 9, 30, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTRIGHT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite", 	10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight                 ", "Enter Y(es) or N(o), ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 	 10, 30, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTRIGHT, "", "", local_rec.oniteDesc}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*=======================
| Function Declarations |
=======================*/
int  	ValidRange 			(void);
int  	check_page 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	OpenDB 				(void);
void 	PrintDetail 		(void);
void 	PrintItem 			(void);
void 	PrintTotal 			(char *);
void 	ProcessCcmr 		(void);
void 	ProcessData 		(void);
void 	ProcessInls 		(void);
void 	RunProgram 			(char *);
void 	SetupDefault		(void);
void 	StoreData 			(void);
void 	shutdown_prog 		(void);

/*===========================
| Main Processing Routine , |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *) 0) ? 0 :  atoi (sptr);

	TruePosition	=	TRUE;

	if (argc != 1 && argc != 5)
	{
		print_at (0, 0, mlSoMess770, argv [0]);
		print_at (1, 0, mlSoMess771, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	lsystemDate = TodaysDate ();
	strcpy (systemDate, DateToString (lsystemDate));

	if (argc == 5)
	{
		switch (argv [1] [0])
		{
		case	'C':
			strcpy (local_rec.byWhat, "C");
			break;

		case	'B':
			strcpy (local_rec.byWhat, "B");
			break;

		case	'W':
			strcpy (local_rec.byWhat, "W");
			break;

		default :
			print_at (21, 0, mlSoMess207);
			return (EXIT_FAILURE);
		}
		if (StringToDate (argv [2]) < 0L)
			strcpy (startDate, systemDate);
		else
			sprintf (startDate, "%-10.10s", argv [2]);

		if (StringToDate (argv [3]) < 0L)
			strcpy (endDate, systemDate);
		else
			sprintf (endDate, "%-10.10s", argv [3]);

		/*-------------------
		| Printer Number	|
		-------------------*/
		printerNumber = atoi (argv [4]);

		dsp_screen ("Processing : Printing Inventory Lost Sales.", 
					comm_rec.co_no, comm_rec.co_name);

		if ((fin = pr_open ("so_lsaleprt.p")) == NULL)
			file_err (errno, "so_lsaleprt.p", "pr_open");

		if ((fout = popen ("pformat", "w")) == NULL)
			file_err (errno, "pformat", "propen");

		HeadingOutput ();

		fsort = sort_open ("lsale");

		switch (local_rec.byWhat [0])
		{
		case	'C':
			/* BY COMPANY */
			ProcessCcmr ();
			break;

		case	'B':
		case	'W':
			/* BY BRANCH/WAREHOUSE */
			ProcessInls ();
			break;
		}

		ProcessData ();

		fprintf (fout, ".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();        		/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*----------------------
	| Reset Control Flags  |
	----------------------*/
	search_ok 		= TRUE;
	entry_exit 		= TRUE;
	prog_exit 		= FALSE;
	restart 		= FALSE;
	SetupDefault ();
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit 		= TRUE;

    if (!restart)
       RunProgram (argv [0]);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence, |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open data base files |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

/*=========================
| Close data base files , |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inls);
	abc_fclose (ccmr);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_dbclose ("data");
}
int
spec_valid (
 int field)
{
	if (LCHECK ("byWhat"))
	{
		switch (local_rec.byWhat [0])
		{
		case	'C':
			strcpy (local_rec.byWhatDesc, ML ("Company"));
			break;

		case	'B':
			strcpy (local_rec.byWhatDesc, ML ("Branch"));
			break;

		case	'W':
			strcpy (local_rec.byWhatDesc, ML ("Warehouse"));
			break;
		}
		DSP_FLD ("byWhatDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startDate"))
	{
		if (local_rec.startDate > local_rec.endDate)
		{
			errmess (ML (mlStdMess019));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (dflt_used)
		{
			DSP_FLD ("startDate");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate"))
	{
		if (local_rec.startDate > local_rec.endDate)
		{
			errmess (ML (mlStdMess026));
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (dflt_used)
		{
			DSP_FLD ("endDate");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Field Selection Lpno option, 	|
	---------------------------------------*/
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (printerNumber))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		sprintf (printerString, "%2d", printerNumber);
		return (EXIT_SUCCESS);
	}
	
	/*---------------------------------------------
	| Validate Field Selection background option, |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
					(local_rec.back [0] == 'N') ? ML ("No") : ML ("Yes"));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option, |
	--------------------------------------------*/
	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc, 
					(local_rec.onite [0] == 'N') ? ML ("No") : ML ("Yes"));

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void		
SetupDefault (
 void)
{
	printerNumber = 1;
	strcpy (local_rec.byWhat, "C");
	strcpy (local_rec.byWhatDesc, ML ("Company"));
	strcpy (local_rec.back, "N");
	strcpy (local_rec.backDesc, ML ("No"));
	strcpy (local_rec.onite,"N");
	strcpy (local_rec.oniteDesc, ML ("No"));
	local_rec.startDate	=	MonthStart (lsystemDate);
	strcpy (defaultDate, DateToString (local_rec.startDate));
	local_rec.endDate 	= StringToDate (systemDate);
}

void
RunProgram (
 char *filename)
{
	sprintf (printerString, "%d", printerNumber);

	strcpy (startDate, DateToString (local_rec.startDate));
	strcpy (endDate,   DateToString (local_rec.endDate));

	shutdown_prog ();

	/*================================
	| Test for Overnight Processing, | 
	================================*/
	if (local_rec.onite [0] == 'Y')
	{ 
		if (fork () == 0)
		{
			execlp 
			(
				"ONIGHT", 
				"ONIGHT", 
				filename, 
				local_rec.byWhat, 
				startDate, 
				endDate, 
				printerString, 
				ML (mlSoMess208), (char *)0
			);
		}
	}
	/*====================================
	| Test for forground or background , |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp 
			(
				filename, 
				filename, 
				local_rec.byWhat, 
				startDate, 
				endDate, 
				printerString, (char *)0
			);
		}
	}
	else 
	{
		execlp 
		(
			filename, 
			filename, 
			local_rec.byWhat, 
			startDate, 
			endDate, 
			printerString, (char *)0
		);
	}
}

/*===================================
| Start Out Put To Standard Print , |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);

	if (envVarDbMcurr)
		fprintf (fout, ".12\n");
	else
		fprintf (fout, ".11\n");

	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L155\n");
	
	switch (local_rec.byWhat [0])
	{
	case	'C':
		fprintf (fout, ".EINVENTORY LOST SALES REPORT BY COMPANY\n");
		break;

	case	'B':
		fprintf (fout, ".EINVENTORY LOST SALES REPORT BY BRANCH\n");
		break;

	case	'W':
		fprintf (fout, ".EINVENTORY LOST SALES REPORT BY WAREHOUSE\n");
		break;
	}

	fprintf (fout, ".ECOMPANY %s - %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".EFrom : %s to %s\n", startDate, endDate);

	if (envVarDbMcurr)
		fprintf (fout, ".E%s\n", ML ("All Values In Local Currency"));

	pr_format (fin, fout, "LINE1", 0, 0);
	pr_format (fin, fout, "HEAD1", 0, 0);
	pr_format (fin, fout, "HEAD2", 0, 0);
	pr_format (fin, fout, "LINE2", 0, 0);
	pr_format (fin, fout, "RULER", 0, 0);

	fflush (fout);
}

/*---------------------------------------
| Process All Warehouse In This Company |
---------------------------------------*/
void
ProcessCcmr (
 void)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (inls_rec.co_no, ccmr_rec.co_no);
		strcpy (inls_rec.est_no, ccmr_rec.est_no);
		cc = find_rec (inls, &inls_rec, GTEQ, "r");
		while (!cc && !strcmp (inls_rec.co_no, ccmr_rec.co_no))
		{
			if (ValidRange ())
				StoreData ();

			cc = find_rec (inls, &inls_rec, NEXT, "r");
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
}

void
ProcessInls (
 void)
{
	strcpy (inls_rec.co_no, comm_rec.co_no);
	strcpy (inls_rec.est_no, comm_rec.est_no);
	cc = find_rec (inls, &inls_rec, GTEQ, "r");

	while (!cc && !strcmp (inls_rec.co_no, comm_rec.co_no))
	{
		if (ValidRange ())
			StoreData ();

		cc = find_rec (inls, &inls_rec, NEXT, "r");
	}
}

int
ValidRange (void)
{
	long	longStartDate = 0L;
	long	longEndDate = 0L;

	longStartDate = StringToDate (startDate);
	longEndDate = StringToDate (endDate);

	if (inls_rec.date < longStartDate || inls_rec.date > longEndDate)
		return (FALSE);

	if (BY_COMPANY)
	{
		if (inls_rec.hhcc_hash == ccmr_rec.hhcc_hash)
			return (TRUE);

		return (FALSE);
	}

	if (BY_BRANCH && !strcmp (inls_rec.est_no, ccmr_rec.est_no))
		return (TRUE);

	if ( BY_WAREHOUSE && inls_rec.hhcc_hash == ccmr_rec.hhcc_hash)
		return (TRUE);

	return (FALSE);
}

/*======================
| Store relevent Data, |
======================*/
void
StoreData (
 void)
{
	inmr_rec.hhbr_hash	=	inls_rec.hhbr_hash;
	if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
		return;

	dsp_process ("Item No, : ", inmr_rec.item_no);

	sprintf 
	(
		dataString, 
		"%-2.2s %-2.2s %-16.16s %-10.10s %-2.2s %6ld %6ld %-2.2s %-2.2s %8.2f %10.2f %-60.60s\n", 
		inls_rec.est_no, 
		ccmr_rec.cc_no, 
		inmr_rec.item_no, 
		DateToString (inls_rec.date), 
		inls_rec.res_code, 
		inls_rec.hhcu_hash, 
		inls_rec.hhbr_hash, 
		inls_rec.area_code, 
		inls_rec.sale_code, 
		inls_rec.qty, 
		inls_rec.value, 
		inls_rec.res_desc
	);
	sort_save (fsort, dataString);
}

/*================================
| Process Data Stored and print, | 
================================*/
void
ProcessData (
 void)
{
	int		okToPrintItem = TRUE;
	char	*sptr;

	fsort 	= sort_sort (fsort, "lsale");
	sptr 	= sort_read (fsort);

	while (sptr != (char *)0)
	{
		foundInls = TRUE;

		sprintf (currentBrNo, 	"%-2.2s", 	sptr);
		sprintf (currentWhNo, 	"%-2.2s", 	sptr + 3);
		sprintf (currentItemNo, "%-16.16s", sptr + 6);
		sprintf (lostSaleDate, 	"%-10.10s", sptr + 23);
		sprintf (reasonCode, 	"%-2.2s", 	sptr + 34);
		sprintf (areaCode, 		"%-2.2s", 	sptr + 51);
		sprintf (saleCode, 		"%-2.2s", 	sptr + 54);
		sprintf (reasonDesc, 	"%-60.60s", sptr + 77);
		hhcuHash = atol (sptr + 37);
		hhbrHash = atol (sptr + 44);
		qty = (float) atof (sptr + 57);
		value = atof (sptr + 66);
	
		if (firstTime)
		{
			PrintItem ();
			PrintDetail ();
			firstTime = FALSE;
		}
		else
		{
			if (BY_COMPANY || BY_BRANCH)
			{
				if (strcmp (currentBrNo, previousBrNo))
				{
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintTotal ("I");
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintTotal ("B");
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintItem ();
					PrintDetail ();
					okToPrintItem = FALSE;
				}
			}

			if (BY_WAREHOUSE)
			{
				if (strcmp (currentWhNo, previousWhNo))
				{
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintTotal ("I");
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintTotal ("W");
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintItem ();
					PrintDetail ();
					okToPrintItem = FALSE;
				}
			}

			if (strcmp (currentItemNo, previousItemNo) && okToPrintItem)
			{
				pr_format (fin, fout, "SEPARATOR", 0, 0);
				PrintTotal ("I");
				if (!strcmp (currentWhNo, previousWhNo))
				{
					pr_format (fin, fout, "SEPARATOR", 0, 0);
					PrintItem ();
					PrintDetail ();
				}
			}
			else
			{
				if (okToPrintItem)
					PrintDetail ();
			}

			okToPrintItem = TRUE;
		}

		strcpy (previousWhNo, currentWhNo);
		strcpy (previousBrNo, currentBrNo);
		strcpy (previousItemNo, currentItemNo);

		itemQuantity 	+= qty;
		itemValue 		+= extend;
		brBrQuantity 	+= qty;
		coBrQuantity 	+= qty;
		coBrValue 		+= extend;
		brBrValue 		+= extend;
		whQuantity 		+= qty;
		whValue 		+= extend;
			
		sptr = sort_read (fsort);
	}

	/*-----------------------------------------------
	| On last record, print the total lost sales	|
	-----------------------------------------------*/
	if (foundInls)
	{
		pr_format (fin, fout, "SEPARATOR", 0, 0);
		PrintTotal ("I");
		pr_format (fin, fout, "SEPARATOR", 0, 0);
		switch (local_rec.byWhat [0])
		{
		case	'C':
		case	'B':
			PrintTotal ("B");
			pr_format (fin, fout, "SEPARATOR", 0, 0);
			break;

		case	'W':
			PrintTotal ("W");
			pr_format (fin, fout, "SEPARATOR", 0, 0);
			break;
		}
		if (BY_COMPANY)
			PrintTotal ("C");
	}
	fflush (fout);
	sort_delete (fsort, "lsal");
}

void
PrintItem (
 void)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (inmr_rec.item_no, 		"%-16.16s", " ");
		sprintf (inmr_rec.description, 	"%-40.40s", "Unknown Item");
	}

	expand (err_str, inmr_rec.item_no);
	pr_format (fin, fout, "ITEM_HEAD", 1, err_str);

	expand (err_str, inmr_rec.description);
	pr_format (fin, fout, "ITEM_HEAD", 2, err_str);
}

void
PrintDetail (
 void)
{
	extend = 0.00;

	cumr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (cumr_rec.dbt_no, 	"%-6.6s", " ");
		sprintf (cumr_rec.dbt_acronym, 	"%-9.9s", "Unknown  ");
	}

	extend = (double)qty * value;

	pr_format (fin, fout, "REASON", 1, currentBrNo);
	pr_format (fin, fout, "REASON", 2, currentWhNo);
	pr_format (fin, fout, "REASON", 3, lostSaleDate);
	pr_format (fin, fout, "REASON", 4, reasonCode);
	pr_format (fin, fout, "REASON", 5, reasonDesc);
	pr_format (fin, fout, "REASON", 6, cumr_rec.dbt_no);
	pr_format (fin, fout, "REASON", 7, cumr_rec.dbt_acronym);
	pr_format (fin, fout, "REASON", 8, areaCode);
	pr_format (fin, fout, "REASON", 9, saleCode);
	pr_format (fin, fout, "REASON", 10, qty);
	pr_format (fin, fout, "REASON", 11, value);
	pr_format (fin, fout, "REASON", 12, extend);

	fflush (fout);
}

void
PrintTotal (
 char *flag)
{
	switch (flag [0])
	{
	case	'C':
		pr_format (fin, fout, "CO_TOTAL", 1, coBrQuantity);
		pr_format (fin, fout, "CO_TOTAL", 2, coBrValue);
		coBrQuantity = 0, 00;
		coBrValue = 0, 00;
		break;

	case	'B':
		pr_format (fin, fout, "BR_TOTAL", 1, previousBrNo);
		pr_format (fin, fout, "BR_TOTAL", 2, brBrQuantity);
		pr_format (fin, fout, "BR_TOTAL", 3, brBrValue);
		brBrQuantity = 0, 00;
		brBrValue = 0, 00;
		break;

	case	'W':
		pr_format (fin, fout, "WH_TOTAL", 1, previousWhNo);
		pr_format (fin, fout, "WH_TOTAL", 2, whQuantity);
		pr_format (fin, fout, "WH_TOTAL", 3, whValue);
		whQuantity = 0, 00;
		whValue = 0, 00;
		break;

	case	'I':
		pr_format (fin, fout, "ITEM_TOTAL", 1, previousItemNo);
		pr_format (fin, fout, "ITEM_TOTAL", 2, itemQuantity);
		pr_format (fin, fout, "ITEM_TOTAL", 3, itemValue);
		itemQuantity = 0, 00;
		itemValue = 0, 00;
		break;
	}
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
	int	scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlSoMess208), 25, 0, 1);

	box (0, 3, 80, 7);
	line_at (7, 1, 79);
	line_at (20, 0, 80);
	print_at (21,0,ML (mlStdMess038), comm_rec.co_no, clip (comm_rec.co_name));
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no, clip(comm_rec.est_name));
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

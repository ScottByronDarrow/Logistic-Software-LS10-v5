/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_bysuprt.c,v 5.5 2002/07/17 09:57:32 scott Exp $
|  Program Name  : (po_bysuprt.c)                                     |
|  Program Desc  : (Print Purchase Order by Supplier.    	 )        |	
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, pohr, poln, inmr, inex,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 01/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (07/09/87)      | Modified  by  : Fui Choo Yap.    |
|                : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|                : (09/12/88)      | Modified  by  : Bee Chwee Lim.   |
|                : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|                : (24/04/89)      | Modified  by  : Scott Darrow.    |
|                : (19/06/89)      | Modified  by  : Fui Choo Yap.    |
|                : (13/08/90)      | Modified  by  : Scott Darrow.    |
|                : (07/07/92)      | Modified  by  : Simon Dubey.     |
|                : (10/02/94)      | Modified  by  : Campbell Mander. |
|                : (10/04/94)      | Modified  by  : Roel Michels     |
|                : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                : (14/10/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Print PNAME on top right of report.                |
|                : Modified program to allow a range of supplier to   |
|                : be selected as in so_bycusprt.c                    |
|                : Change Printing of land_cost from DOLLARS.         |
|  Date Modified : (24/04/89) - Added test to ignore -ve Po (s).       |
|    (19/06/89)  : Put defaults for start/end supplier.               |
|                : print Rule-line after supplier name; remove double |
|                : spacing between parts.                             |
|                : (13/08/90) - General update + fixed shipment no.   |
|                : (07/07/92) - To include inex desc lines SC DFH 7287|
|    (10/02/94)  : PSL 10414. Change no_lps () to valid_lp ().          |
|    (10/04/94)  : PSL 10673 - Online conversion                      |
|    (13/09/97)  : Updated for Multilingual Conversion.               |
|    (14/10/97)  : Fixed MLDB error.              				   	  |
|                :                                                    |
|                                                                     |
| $Log: po_bysuprt.c,v $
| Revision 5.5  2002/07/17 09:57:32  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/08 03:33:07  scott
| Updated to add delay to error message.
|
| Revision 5.3  2001/10/19 02:53:04  cha
| Fix issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:15:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:41  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:05  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/16 08:14:22  scott
| Updated to fix screen selection to work correctly with LS10-GUI
| Updated / Cleaned code and added app.schema
|
| Revision 4.1  2001/03/15 03:36:26  scott
| Updated to add sleep (2) for warning messages - LS10-GUI.
|
| Revision 4.0  2001/03/09 02:32:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:13  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:30:59  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:04:57  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/01/27 07:05:11  ramon
| For GVision compatibility, I added description fields for background and overnight fields.
|
| Revision 1.14  1999/12/06 01:32:29  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/11 06:43:12  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.12  1999/11/05 05:17:07  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.11  1999/10/14 03:04:20  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.10  1999/09/29 10:11:51  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/21 04:37:55  scott
| Updated from Ansi project
|
| Revision 1.8  1999/06/17 10:06:15  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_bysuprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_bysuprt/po_bysuprt.c,v 5.5 2002/07/17 09:57:32 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inexRecord	inex_rec;
struct poslRecord	posl_rec;
struct inumRecord	inum_rec;
struct poshRecord	posh_rec;

	extern	int	TruePosition;

	int		envVarCrCo 		= 0,
			envVarCrFind 	= 0,
			supplierFlag	= TRUE,
			firstPoln 		= TRUE,
			recordFound 	= FALSE,
			firstFlag 		= 0,
			poDetails 		= 0,
			partialReceipt 	= FALSE;

	char	branchNumber [3];

	float	quantity		= 0.0,
			totalPoQty 		= 0.0,
			totalSuppQty 	= 0.0,
			totalGrandQty	= 0.0;

	double	extend 			= 0.00,
			totalPoShip 	= 0.00,
			totalSuppShip 	= 0.00,
			totalGrandShip 	= 0.00;

	FILE	*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startSupplier [7];
	char	endSupplier [7];
	char	supplierName [2] [41];
	int		printerNumber;
	char	printerString [3];
	char 	back [2];
	char	backDesc [11];
	char	onight [2];
	char	onightDesc [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from_crdt",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplier},
	{1, LIN, "name1",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierName [0]},
	{1, LIN, "to_crdt",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End   Supplier    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplier},
	{1, LIN, "name2",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierName [1]},
	{1, LIN, "printerNumber",	 7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 8, 23, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight         ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 9, 23, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	RunProgram 		(char *);
void 	HeadingOutput 	(void);
void 	PrintLine 		(void);
void 	TotalLine 		(int);
void 	ProcessFile 	(void);
void 	PrintSupplier 	(void);
void 	ProcessPohr 	(void);
void 	FindPoln 		(void);
char	*GetShipment 	(void);
int 	heading 		(int);
void 	PrintInex 		(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 1 && argc != 4)
	{
		print_at (0,0, mlPoMess724);
        return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	if (argc == 4)
	{
		sprintf (local_rec.startSupplier,"%-6.6s",argv [1]);
		sprintf (local_rec.endSupplier,"%-6.6s",argv [2]);
		local_rec.printerNumber = atoi (argv [3]);

		dsp_screen ("Processing : Printing Purchase Order By Supplier.",
					comm_rec.co_no,comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy (err_str, ML (mlPoMess222));
		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
							    							 : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pohr);
	abc_fclose (posh);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (posl);
	abc_fclose (inum);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("from_crdt"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.supplierName [0],"%-40.40s","Start Supplier");
			DSP_FLD ("name1");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && strcmp (local_rec.startSupplier,local_rec.endSupplier) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.startSupplier));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [0],sumr_rec.crd_name);
		DSP_FLD ("name1");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to_crdt"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.supplierName [1],"%-40.40s","End   Supplier");
			DSP_FLD ("name2");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.endSupplier));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startSupplier,local_rec.endSupplier) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [1],sumr_rec.crd_name);
		DSP_FLD ("name2");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
				(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, 
				(local_rec.onight [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
 char *prog_name)
{
	sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.startSupplier,
				local_rec.endSupplier,
				local_rec.printerString,
				err_str, (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.startSupplier,
				local_rec.endSupplier,
				local_rec.printerString, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.startSupplier,
			local_rec.endSupplier,
			local_rec.printerString, (char *)0);
	}
}


/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);

	fprintf (fout,".15\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".ECompany : %s - %s\n",
				comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".EBranch  : %s - %s\n",
				comm_rec.est_no,clip (comm_rec.est_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EPURCHASE ORDER BY SUPPLIER\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout,".R================");
	fprintf (fout,"===========");
	fprintf (fout,"===========");
	fprintf (fout,"=============");
	fprintf (fout,"=====");
	fprintf (fout,"===========");
	fprintf (fout,"=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"===========");
	fprintf (fout,"===============\n");

	fprintf (fout,"================");
	fprintf (fout,"===========");
	fprintf (fout,"===========");
	fprintf (fout,"=============");
	fprintf (fout,"=====");
	fprintf (fout,"===========");
	fprintf (fout,"=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"===========");
	fprintf (fout,"===============\n");

	fprintf (fout,"!     P.O.      ");
	fprintf (fout,"!   DATE   ");
	fprintf (fout,"!   DATE   ");
	fprintf (fout,"!  SHIPMENT  ");
	fprintf (fout,"!UOM.");
	fprintf (fout,"! QUANTITY ");
	fprintf (fout,"!       PART     ");
	fprintf (fout,"!               DESCRIPTION              ");
	fprintf (fout,"!   DATE   ");
	fprintf (fout,"!    LANDED   !\n");

	fprintf (fout,"!    NUMBER     ");
	fprintf (fout,"! ORDERED  ");
	fprintf (fout,"!   DUE    ");
	fprintf (fout,"!   NUMBER   ");
	fprintf (fout,"!    ");
	fprintf (fout,"! OUTSTDNG ");
	fprintf (fout,"!     NUMBER     ");
	fprintf (fout,"!                                        ");
	fprintf (fout,"!   DUE    ");
	fprintf (fout,"!     COST    !\n");
	PrintLine ();
	firstFlag = TRUE;
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"!---------------");
	fprintf (fout,"!----------");
	fprintf (fout,"!----------");
	fprintf (fout,"!------------");
	fprintf (fout,"!----");
	fprintf (fout,"!----------");
	fprintf (fout,"!----------------");
	fprintf (fout,"!----------------------------------------");
	fprintf (fout,"!----------");
	fprintf (fout,"!-------------!\n");

	fflush (fout);
}

void
TotalLine (
 int pr_tot)
{
	fprintf (fout,"!               ");
	fprintf (fout,"!          ");
	fprintf (fout,"!          ");
	fprintf (fout,"!            ");
	fprintf (fout,"!    ");
	fprintf (fout, (pr_tot) ? "!----------" : "!          ");
	fprintf (fout,"!                ");
	fprintf (fout,"!                                        ");
	fprintf (fout,"!          ");
	fprintf (fout, (pr_tot) ? "!-------------!\n" : "!    LANDED   !\n");

	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.est_no,branchNumber);
	strcpy (sumr_rec.crd_no,local_rec.startSupplier);

	cc = find_rec ("sumr",&sumr_rec,GTEQ,"r");

	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sumr_rec.est_no,branchNumber) && 
		      strcmp (sumr_rec.crd_no,local_rec.startSupplier) >= 0 && 
		      strcmp (sumr_rec.crd_no,local_rec.endSupplier) <= 0) 
	{
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.type,"O");
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no,"       ");

		cc = find_rec ("pohr",&pohr_rec,GTEQ,"r");

		while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
			      !strcmp (pohr_rec.br_no,comm_rec.est_no) && 
			      pohr_rec.type [0] == 'O' && 
			      pohr_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			if (pohr_rec.status [0] != 'D')
			{
				ProcessPohr ();
				firstPoln = TRUE;
			}
			cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
		}
		/*---------------------------
		| Print supplier totals.	|
		---------------------------*/
		if (recordFound)
		{
			PrintLine ();
			fprintf (fout,".LRP3\n");

			fprintf (fout,"! %-25.25s", ML ("TOTAL FOR SUPPLIER"));
			fprintf (fout,"!          ");
			fprintf (fout,"!            ");
			fprintf (fout,"!    ");
			fprintf (fout,"!%9.2f ", totalSuppQty);
			fprintf (fout,"!                ");
			fprintf (fout,"!                                        ");
			fprintf (fout,"!          ");
			fprintf (fout,"!%12.2f !\n", totalSuppShip);
			recordFound = FALSE;
			totalSuppQty = 0.0;
			totalSuppShip = 0.0;
		}
		cc = find_rec ("sumr",&sumr_rec,NEXT,"r");
		supplierFlag = TRUE;
	}
	/*-----------------------------------------------
	| On last record,print the total backlog.	|
	-----------------------------------------------*/
	PrintLine ();
	fprintf (fout,"! %-25.25s", ML ("TOTAL PURCHASE ORDERS"));
	fprintf (fout,"!          ");
	fprintf (fout,"!            ");
	fprintf (fout,"!    ");
	fprintf (fout,"!%9.2f ", totalGrandQty);
	fprintf (fout,"!                ");
	fprintf (fout,"!                                        ");
	fprintf (fout,"!          ");
	fprintf (fout,"!%12.2f !\n", totalGrandShip);

	fflush (fout);
}

void
PrintSupplier (
 void)
{
	fprintf (fout,".LRP3\n");
	if (!firstFlag)
		PrintLine ();

	PrintLine ();
	fprintf (fout,"! %6.6s (%9.9s)  (%40.40s)   ",
				sumr_rec.crd_no, sumr_rec.acronym, sumr_rec.crd_name);
	fprintf (fout,"%83.83s!\n", " ");
	TotalLine (FALSE);
	supplierFlag = FALSE;
	firstFlag = FALSE;
}

void
ProcessPohr (
 void)
{
	float	quantityLeft = 0.00;

	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = 0;

	totalPoQty = 0.0;
	totalPoShip = 0.0;
	poDetails = 0;
	partialReceipt = FALSE;

	cc = find_rec ("poln",&poln_rec,GTEQ,"r");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		quantityLeft = poln_rec.qty_ord - poln_rec.qty_rec;
		if (quantityLeft <= 0.00)
		{
			cc = find_rec ("poln",&poln_rec,NEXT,"r");
			continue;
		}
		dsp_process ("Supplier No. : ",sumr_rec.crd_no);
		if (supplierFlag)
			PrintSupplier ();

		if (!poDetails)
		{
			fprintf (fout,"!%15.15s",pohr_rec.pur_ord_no);
			fprintf (fout,"!%10.10s", (pohr_rec.date_raised == 0L) 
					 ? "        " : DateToString (pohr_rec.date_raised));
			fprintf (fout,"!%10.10s", (pohr_rec.due_date == 0L) 
					 ? "        " : DateToString (pohr_rec.due_date));
		}

		FindPoln ();
		PrintInex ();
		fflush (fout);

		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
	/*-----------------------------------------------------------------
	| If more than one line for a p.o.,print total for detail lines. |
	-----------------------------------------------------------------*/
	if (poDetails > 1)
	{
		fprintf (fout,".LRP2\n");
		if (partialReceipt)
		{
			TotalLine (TRUE);
			fprintf (fout,"! %-18.18s %-30.30s",
									ML ("TOTAL FOR P.O.    "), 
									ML ("*** (PARTLY RECEIVED)*** "));
		}
		else
		{
			TotalLine (TRUE);
			fprintf (fout,"! %-18.18s %30.30s",
									ML ("TOTAL FOR P.O.    "), " ");
		}
		fprintf (fout,"!%s", inmr_rec.sale_unit);
		fprintf (fout,"!%9.2f ",totalPoQty);
		fprintf (fout,"!                ");
		fprintf (fout,"!                                        ");
		fprintf (fout,"!          ");
		fprintf (fout,"!%12.2f !\n",totalPoShip);
		TotalLine (FALSE);
	}
	else
	{

		if (poDetails)
		{
			fprintf (fout,".LRP2\n");
			if (partialReceipt)
				fprintf (fout,"! %18.18s %-30.30s", " ",
								ML ("*** (PARTLY RECEIVED)*** "));
			else
				fprintf (fout,"! %18.18s %30.30s", " ", " ");

			fprintf (fout,"!    ");
			fprintf (fout,"!          ");
			fprintf (fout,"!                ");
			fprintf (fout,"!                                        ");
			fprintf (fout,"!          ");
			fprintf (fout,"!%12.2f !\n",totalPoShip);
		}
	}
}

void
FindPoln (
 void)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct 		= 1.00,

	cc = find_hash ("inmr",&inmr_rec,COMPARISON,"r",poln_rec.hhbr_hash);
	if (cc)
		sprintf (inmr_rec.item_no,"%-16.16s"," ");

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	CnvFct	=	StdCnvFct / PurCnvFct;

	if (poln_rec.qty_rec > 0.0)
		partialReceipt = TRUE;
	else
		partialReceipt = FALSE;

	quantity = poln_rec.qty_ord - poln_rec.qty_rec;

	extend = twodec (poln_rec.land_cst);
	extend = out_cost (extend,inmr_rec.outer_size);
	extend *= (double) quantity;

	totalPoQty 	 	+= quantity;
	totalPoShip 	+= extend;
	totalSuppQty 	+= quantity;
	totalSuppShip  	+= extend;
	totalGrandQty 	+= quantity;
	totalGrandShip 	+= extend;

	if (!firstPoln)
		fprintf (fout,"!               !          !          ");
		
	fprintf (fout,"!%12.12s", GetShipment ());
	fprintf (fout,"!%4.4s",inum_rec.uom);
	fprintf (fout,"!%9.2f ",quantity * CnvFct);
	fprintf (fout,"!%-16.16s",inmr_rec.item_no);
	fprintf (fout,"!%-40.40s",poln_rec.item_desc);
	fprintf (fout,"!%10.10s", (poln_rec.due_date == 0L) 
			     ? "          " : DateToString (poln_rec.due_date));

	fprintf (fout,"! %11.2f !\n",extend);
	firstPoln = FALSE;

	poDetails++;
}
/*======================
| Get shipment number. |
======================*/
char *
GetShipment (
 void)
{
	posl_rec.hhpl_hash	=	poln_rec.hhpl_hash;
	cc = find_rec ("posl", &posl_rec, GTEQ, "r");
	if (!cc && posl_rec.hhpl_hash == poln_rec.hhpl_hash)
	{
		if (posl_rec.ship_qty > 0.00)
		{
			posh_rec.hhsh_hash = posl_rec.hhsh_hash;
			cc = find_rec (posh, &posh_rec, COMPARISON, "r");
			if (cc)
				return (" ");

			return (posh_rec.csm_no);
		}
		cc = find_rec ("posl", &posl_rec, NEXT, "r");
	}
	return (" ");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlPoMess130),25,0,1);

	line_at (1,0,80);
	line_at (6,1,79);

	box (0,3,80,6);

	line_at (20,0,80);
	print_at (21,0,  ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec ("inex", &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout,"!               ");
		fprintf (fout,"!          ");
		fprintf (fout,"!          ");
		fprintf (fout,"!            ");
		fprintf (fout,"!    ");
		fprintf (fout,"!          ");
		fprintf (fout,"!                ");
		fprintf (fout,"!%-40.40s",inex_rec.desc);
		fprintf (fout,"!          ");
		fprintf (fout,"!             !\n");
		cc = find_rec ("inex", &inex_rec, NEXT, "r");
	}
}

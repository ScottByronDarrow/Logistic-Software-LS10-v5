/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_invprnt.i.c,v 5.5 2002/11/26 07:07:52 kaarlo Exp $
|  Program Name  : (so_invprnt.i.c)                                   |
|  Program Desc  : (Invoice / Credit Note / Packing Slip Input  )     |
|                  (For Print & Reprint incl Reflagging         )     |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/10/88         |
|---------------------------------------------------------------------|
| $Log: so_invprnt.i.c,v $
| Revision 5.5  2002/11/26 07:07:52  kaarlo
| SC0057. Updated to fix SC0057.
|
| Revision 5.4  2002/08/14 06:50:44  scott
| Updated for Linux error
|
| Revision 5.3  2002/07/18 07:18:25  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/12/11 02:55:51  scott
| Minor changes
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_invprnt.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invprnt.i/so_invprnt.i.c,v 5.5 2002/11/26 07:07:52 kaarlo Exp $";

#define	MOD	1
#define	MAXWIDTH	140
#define	MAXPRINT	10000
#define	MAXLINES	100
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>
#include <arralloc.h>

#define	REPRINT		 (printFlag [0] == 'Y')
#define	INVOICE		0
#define	CREDIT_NOTE	1
#define	PACK_SLIP	2
#define	STD_ORDER	3
#define	STD_INVOICE	4
#define	CCN			5

#define	TYPE		program [typeFlag]
#define	STANDARD	 (typeFlag == STD_ORDER || typeFlag == STD_INVOICE)

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#define	fflush	Remote_fflush
#endif	/* GVISION */

struct	{
	char	*_type;
	char	*_prt;
	char	*_prmpt;
	char	*_desc;
	char	*_env;
	char	*_prg;
	char	*_alt_type;
} program [] = {
	{"I","I","Tax Invoice No  ","Tax Invoice", 	 "SO_CTR_INV","so_ctr_inv","I"},
	{"C","C","Credit Note No  ","Credit Note", 	 "SO_CTR_INV","so_ctr_inv","C"},
	{"P","P","Packing Slip No ","Packing Slip",  "SO_CTR_PAC","so_ctr_pac","T"},
	{"S","P","Packing Slip No ","Standard Order","SO_CTR_PAC","so_ctr_pac","P"},
	{"S","I","Tax Invoice No  ","Std. Invoice",  "SO_CTR_INV","so_ctr_inv","I"},
	{"N","N","CC. Note  No    ","CC. Note.  ", 	 "SO_CTR_CCN","so_ctr_ccn","N"},
	{"", "", "", "", "", "", ""}
};

#include	"schema"

	struct	commRecord	comm_rec;
	struct	cohrRecord	cohr_rec;
	struct	colnRecord	coln_rec;
	struct	cumrRecord	cumr_rec;

	int		envDbCo			= 0;
	int		envDbFind		= 0;
	int		printerNumber	= 0;
	int		typeFlag		= 0;
	int		summary			= FALSE;
	int		combInvPack		= FALSE;
	int		sortFlag		= FALSE;

char	branchNumber [3];
char	printFlag [2];
char	runPrintProgram [81];

char	*data  = "data";

int		findStatus	=	FALSE;

FILE	*pout;

char	totalStr [31];
char	startStr [21];
char	endStr [18];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	inv_no [2] [9];
	char	customer [2] [31];
	char	find_codes [13];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "start_inv_no", MAXLINES, 7, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "1234567890123456", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.inv_no [0]}, 
	{1, TAB, "start_customer", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  C u s t o m e r   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customer [0]}, 
	{1, TAB, "end_inv_no", 0, 5, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "1234567890123456", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.inv_no [1]}, 
	{1, TAB, "end_customer", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  C u s t o m e r   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customer [1]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

	int		CustInvSort		 (const	void *,	const void *);
	int		fullRePrint		=	FALSE;

/*---------------------------------------------------------------------------
|	Structure for dynamic array,  for the sorting of invoices using qsort	|
---------------------------------------------------------------------------*/
struct InvoiceStructure
{
	char	sortField [20];
	long	hhcoHash;
}	*invoice;
	DArray invoice_d;
	int	invoiceCnt = 0;

/*=======================
| Function Declarations |
=======================*/
void	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	PrintEveryInvoice 	(void);
void 	Process 			(void);
int  	spec_valid 			(int);
void 	PrintAll 			(void);
int  	DeleteLine 			(void);
void 	SrchInvoice 		(char *, char *, char *);
void	PrintFunction 		(int, long, char *, char *);
int  	heading 			(int);
char 	*CheckVariable 		(char *, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 4)
	{
		/*-----------------------------------
		| Usage : %s   	<printerNumber> 	|
		|				<reprintFlag/N]>	| 
		|				<type I/C/P/O/S/N]>	|
		|				<findFlag>          |
		|				<sortFlag>          |
		-----------------------------------*/
		print_at (0, 0, mlSoMess751, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	fullRePrint	=	FALSE;
	sortFlag	=	FALSE;
	if (!strcmp (sptr, "so_fullrpt"))
	{
		fullRePrint	=	TRUE;
		sortFlag	=	TRUE;
	}
	SETUP_SCR (vars);

	printerNumber = atoi (argv [1]);

	switch (argv [2] [0])
	{
	case	'Y':
	case	'y':
		strcpy (printFlag, "Y");
		break;

	case	'N':
	case	'n':
		strcpy (printFlag, "N");
		break;

	default:
		print_at (0, 0, mlSoMess751, argv [0]);
		return (EXIT_FAILURE);
	}

	switch (argv [3] [0])
	{
	case	'I':
	case	'i':
		typeFlag = INVOICE;
		sprintf (totalStr," %s ",(REPRINT)? ML (mlSoMess314):ML (mlSoMess319));
		sprintf (startStr," %s ", 	ML (mlSoMess324));
		sprintf (endStr, " %s ", 	ML (mlSoMess329));
		break;

	case	'C':
	case	'c':
		typeFlag = CREDIT_NOTE;
		sprintf (totalStr, " %s ", (REPRINT)?ML (mlSoMess315):ML (mlSoMess320));
		sprintf (startStr, " %s ", ML (mlSoMess325));
		sprintf (endStr, " %s ", ML (mlSoMess330));
		break;

	case	'N':
	case	'n':
		typeFlag = CCN;
		sprintf (totalStr, " %s ", (REPRINT)?ML (mlSoMess317):ML (mlSoMess322));
		sprintf (startStr, " %s ", ML (mlSoMess327));
		sprintf (endStr, " %s ", ML (mlSoMess332));
		break;

	case	'P':
	case	'p':
		typeFlag = PACK_SLIP;
		sprintf (totalStr, " %s ", (REPRINT)?ML (mlSoMess316):ML (mlSoMess321));
		sprintf (startStr, " %s ", ML (mlSoMess326));
		sprintf (endStr, " %s ", ML (mlSoMess331));
		break;

	case	'O':
	case	'o':
/*
		typeFlag = STD_ORDER;*/
		break;

	case	'S':
	case	's':
/*
		typeFlag = STD_INVOICE;
		break;
*/
		/*---------------------------------------------------
		| Standard Order / Invoice Printing Has not been 	|
		| implemented yet\007\n\r							|
		---------------------------------------------------*/
		print_at (0, 0, ML (mlSoMess127));

		sprintf (totalStr, " %s ", 
				 (REPRINT) ? ML (mlSoMess318) : ML (mlSoMess323));
		sprintf (startStr, " %s ", ML (mlSoMess328));
		sprintf (endStr, " %s ",  ML (mlSoMess333));

		return (EXIT_FAILURE);

	default:
		print_at (0, 0, ML (mlSoMess752));
		return (EXIT_FAILURE);
	}

	findStatus = FALSE;
	sprintf (local_rec.find_codes, "%-12.12s", " ");
	if (argc == 5 && !REPRINT)
	{
		sprintf (local_rec.find_codes, "%-12.12s", argv [4]);
		clip (local_rec.find_codes);
		findStatus = TRUE;
	}
	vars [0].prmpt = TYPE._prmpt;
	vars [2].prmpt = TYPE._prmpt;

	envDbCo		= atoi (get_env ("DB_CO"));
	envDbFind	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (runPrintProgram, CheckVariable ("COMB_INV_PAC", "N"));
	if (runPrintProgram [0] == 'Y' || runPrintProgram [0] == 'y')
		combInvPack = TRUE;

	strcpy (runPrintProgram, CheckVariable (TYPE._env, TYPE._prg));

	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.est_no);

	init_scr	();
	set_tty		(); 
	set_masks	();

	if (typeFlag == PACK_SLIP)
	{
		sptr = chk_env ("PACK_SUMMARY");
		if (sptr != (char *)0 && (*sptr == 'Y' || *sptr == 'y'))
			summary = TRUE;
	}

	if (!REPRINT || fullRePrint)
	{
		PrintEveryInvoice ();
		prog_exit = 1;
	}

	while (prog_exit == 0)
	{
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		init_vars (1);
		lcount [1] = 0;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			break;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			break;

		prog_exit	=	TRUE;

		if (lcount [1] != 0)
			Process ();
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
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (typeFlag == PACK_SLIP)
		open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no4");
	else if (typeFlag == CCN)
		open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	else
		open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no3");

	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cumr);
	abc_dbclose (data);
}


/*====================================
| Print all records not yet printed. |
====================================*/
void
PrintEveryInvoice (
 void)
{
	int		firstTime = 1;
	int		i;

	long	startDate	=	MonthStart (comm_rec.dbt_date),
			endDate		=	MonthEnd   (comm_rec.dbt_date);

	sprintf (err_str, "%srinting %ss", (REPRINT) ? "Rep" : "P", TYPE._desc);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	if ((pout = popen (runPrintProgram, "w")) == 0)
	{
		sprintf (err_str, "Error in %s during (POPEN)", runPrintProgram);
		sys_err (err_str, errno, PNAME);
	}
	/*----------------------------- 
	| Allocate the initial array. |
	-----------------------------*/
	ArrAlloc (&invoice_d, &invoice, sizeof (struct InvoiceStructure), 1000);

	invoiceCnt = 0;

	strcpy (cohr_rec.co_no, 	comm_rec.co_no);
	strcpy (cohr_rec.br_no, 	comm_rec.est_no);
	strcpy (cohr_rec.type, 		TYPE._type);
	strcpy (cohr_rec.inv_print, printFlag);
	strcpy (cohr_rec.ps_print, 	printFlag);
	strcpy (cohr_rec.ccn_print, printFlag);
	sprintf (cohr_rec.inv_no, "%-8.8s", " ");
	cc = find_rec (cohr, &cohr_rec, GTEQ, "u");

	while (	!cc && 
			!strcmp (cohr_rec.co_no, comm_rec.co_no) && 
			!strcmp (cohr_rec.br_no, comm_rec.est_no) && 
		 	(cohr_rec.type [0] == TYPE._type [0] || 
		  	 cohr_rec.type [0] == TYPE._alt_type [0]))
	{
		if ((cohr_rec.date_raised < startDate || 
			 cohr_rec.date_raised > endDate) && fullRePrint)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}
		if (findStatus && !strstr (local_rec.find_codes, cohr_rec.stat_flag))
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}
		
		if (typeFlag == PACK_SLIP)
		{
			if (cohr_rec.ps_print [0] != printFlag [0])
			{
				abc_unlock (cohr);
				break;
			}
		}
		else if (typeFlag == CCN)
		{
			if (cohr_rec.ccn_print [0] != printFlag [0])
			{
				abc_unlock (cohr);
				cc = find_rec (cohr, &cohr_rec, NEXT, "u");
				continue;
			}
		}
		/*------------------------------------------
		| Changes from continue and next to break. |
		------------------------------------------*/
		else
		{
			if (cohr_rec.inv_print [0] != printFlag [0])
			{
				abc_unlock (cohr);
				break;
			}
		}

		if (fullRePrint	==	FALSE)
		{
			if (cohr_rec.printing [0] == 'Y')
			{
				abc_unlock (cohr);
				cc = find_rec (cohr, &cohr_rec, NEXT, "u");
				continue;
			}
			else
			{
				strcpy (cohr_rec.printing, "Y");
				cc = abc_update (cohr, &cohr_rec);
				if (cc)
					file_err (cc, "cohr", "DBUPDATE");
			}
		}
		else
			abc_unlock (cohr);

		/*-------------------------------------------------
		| Check the array size before adding new element. |
		-------------------------------------------------*/
		if (!ArrChkLimit (&invoice_d, invoice, invoiceCnt))
			sys_err ("ArrChkLimit (invoice)", ENOMEM, PNAME);

		if (firstTime)
		{
			firstTime = FALSE;
			PrintFunction (printerNumber, -1L, "M", TYPE._prt);
		}

		coln_rec.hhco_hash	= cohr_rec.hhco_hash;
		coln_rec.line_no		= 0;
		cc = find_rec (coln, &coln_rec, GTEQ, "r");
		if (cc || cohr_rec.hhco_hash != coln_rec.hhco_hash)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}
		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (cumr_rec.dbt_no, "      ");
		/*----------------------------------------- 
		| Load values into array element shipCnt. |
		-----------------------------------------*/
		sprintf (invoice [invoiceCnt].sortField, "%-6.6s%-8.8s", 
									cumr_rec.dbt_no, cohr_rec.inv_no );
		invoice [invoiceCnt].hhcoHash = cohr_rec.hhco_hash;

		/*--------------------------
		| Increment array counter. |
		--------------------------*/
		invoiceCnt++;

		dsp_process (TYPE._desc, cohr_rec.inv_no);

		cc = find_rec (cohr, &cohr_rec, NEXT, "u");
	}
	if (sortFlag == TRUE)
	{
		/*-------------------------------------------
		| Sort the array in item description order. |
		-------------------------------------------*/
		qsort 
		(
			invoice, 
			invoiceCnt, 
			sizeof (struct InvoiceStructure), 
			CustInvSort
		);
	}
	for (i = 0; i < invoiceCnt; i++)
	{
		/*-------------
		| Print data. |
		-------------*/
		PrintFunction (printerNumber, invoice [i].hhcoHash, "M", TYPE._prt);

		dsp_process (TYPE._desc, cohr_rec.inv_no);
	}
	abc_unlock (cohr);

	if (!firstTime)
	{
		if (summary)
			PrintFunction (printerNumber, -2L, "M", TYPE._prt);

		PrintFunction (printerNumber, 0L, "M", TYPE._prt);
	}

	abc_selfield (cohr, "cohr_hhco_hash");

	pclose (pout);
	
	for (i = 0; i < invoiceCnt; i++)
	{
		/*---------------------------
		| No more records to print. |
		---------------------------*/
		if (invoice [i].hhcoHash == 0L)
		{
			/*--------------------------
			| Free up the array memory |
			--------------------------*/
			ArrDelete (&invoice_d);
			return;
		}

		/*---------------------------------------
		| then read record again with new index |
		---------------------------------------*/
		cohr_rec.hhco_hash = invoice [i].hhcoHash;
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
			continue;

		if (combInvPack)
		{
			strcpy (cohr_rec.ps_print, "Y");
			strcpy (cohr_rec.inv_print, "Y");
		}
		else
		{
			if (typeFlag == PACK_SLIP)
				strcpy (cohr_rec.ps_print, "Y");
			else if (typeFlag == CCN)
				strcpy (cohr_rec.ccn_print, "Y");
			else
				strcpy (cohr_rec.inv_print, "Y");
		}
		strcpy (cohr_rec.printing, " ");
		cc = abc_update (cohr, &cohr_rec);
		if (cc)
		{
			/*-----------------------------------------------
			| Error Occurred In Update of cohr_inv_no %s - 	|
			| Note And Press Any Key 						|
			-----------------------------------------------*/
			sprintf (err_str , ML (mlSoMess293), cohr_rec.inv_no);
			print_mess (err_str); 
			sleep (sleepTime);
			clear_mess ();
			continue;
		}

	}
	/*--------------------------
	| Free up the array memory |
	--------------------------*/
	ArrDelete (&invoice_d);

	if (typeFlag == PACK_SLIP)
		abc_selfield (cohr, "cohr_id_no4");
	else if (typeFlag == CCN)
		abc_selfield (cohr, "cohr_id_no2");
	else
		abc_selfield (cohr, "cohr_id_no3");
}

void
Process (
 void)
{
	int	firstTime = 1;

	clear ();
	print_at (0, 0, ML (mlStdMess035)); 

	if ((pout = popen (runPrintProgram, "w")) == 0)
	{
		sprintf (err_str, "Error in %s during (POPEN)", runPrintProgram);
		sys_err (err_str, errno, PNAME);
	}

	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);

		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		strcpy (cohr_rec.type, TYPE._type);
		strcpy (cohr_rec.inv_print, printFlag);
		strcpy (cohr_rec.ccn_print, printFlag);
		strcpy (cohr_rec.ps_print, printFlag);
		sprintf (cohr_rec.inv_no, "%-8.8s", local_rec.inv_no [0]);
		cc = find_rec (cohr, &cohr_rec, GTEQ, (REPRINT) ? "r" : "u");

		while (	!cc && 
				!strcmp (cohr_rec.co_no, comm_rec.co_no) && 
				!strcmp (cohr_rec.br_no, comm_rec.est_no) && 
			 	(cohr_rec.type [0] == TYPE._type [0] ||
			 	cohr_rec.type [0] == TYPE._alt_type [0]) &&
				strcmp (cohr_rec.inv_no, local_rec.inv_no [0]) >= 0 &&
				strcmp (cohr_rec.inv_no, local_rec.inv_no [1]) <= 0)
		{
			if (typeFlag != PACK_SLIP && typeFlag != CCN)
			{
				if (cohr_rec.inv_print [0] != printFlag [0])
				{
					if (!REPRINT)	
						abc_unlock (cohr);
					break;
				}
			}
			if (typeFlag == PACK_SLIP)
			{
				if (cohr_rec.ps_print [0] != printFlag [0])
				{
					if (!REPRINT)	
						abc_unlock (cohr);
					cc = find_rec (cohr, &cohr_rec, NEXT, (REPRINT) ? "r" : "u");
					continue;
				}
			}
			if (typeFlag == CCN)
			{
				if (cohr_rec.ccn_print [0] != printFlag [0])
				{
					if (!REPRINT)	
						abc_unlock (cohr);
					cc = find_rec (cohr, &cohr_rec, NEXT, (REPRINT) ? "r" : "u");
					continue;
				}
			}
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = 0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			if (cc || cohr_rec.hhco_hash != coln_rec.hhco_hash)
			{
				if (!REPRINT)	
					abc_unlock (cohr);
				cc = find_rec (cohr, &cohr_rec, NEXT, (REPRINT) ? "r" : "u");
				continue;
			}

			if (firstTime)
				PrintFunction (printerNumber, -1L, "M", TYPE._prt);

			PrintFunction (printerNumber, cohr_rec.hhco_hash, "M", TYPE._prt);

			if (firstTime)
			{
				sprintf (err_str, "%srinting %ss", (REPRINT) ? "Rep" : "P", TYPE._desc);
				dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
			}

			firstTime = 0;
			dsp_process (TYPE._desc, cohr_rec.inv_no);
		
			cc = find_rec (cohr, &cohr_rec, NEXT, (REPRINT) ? "r":"u");
		}
	}
	if (!firstTime)
		PrintFunction (printerNumber, 0L, "M", TYPE._prt);

	pclose (pout);
}

int
spec_valid (
 int field)
{
	char	lowInvoice [9];
	char	highInvoice [9];

	if (LCHECK ("start_inv_no")) 
	{
		if (dflt_used)
			return (DeleteLine ());

		if (!strcmp (local_rec.inv_no [0], "ALL     "))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.inv_no [0], zero_pad (local_rec.inv_no [0], 8));

		strcpy (lowInvoice, "        ");
		strcpy (highInvoice, (prog_status == ENTRY) ? "~~~~~~~~" : local_rec.inv_no [1]);
		if (SRCH_KEY)
		{
			SrchInvoice (lowInvoice, temp_str, highInvoice);
			return (EXIT_SUCCESS);
		}

		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		strcpy (cohr_rec.type, TYPE._type);
		strcpy (cohr_rec.inv_print, printFlag);
		strcpy (cohr_rec.ccn_print, printFlag);
		strcpy (cohr_rec.ps_print, printFlag);
		strcpy (cohr_rec.inv_no, local_rec.inv_no [0]);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cohr_rec.type, TYPE._alt_type);
			cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*------------------------------------------------------
			| %s No %s not on file", TYPE._desc, local_rec.inv_no [0] |
			------------------------------------------------------*/
			switch (typeFlag)
			{
				case  INVOICE		:
					strcpy (err_str, ML (mlStdMess115));
					break;
				case CREDIT_NOTE	:
					strcpy (err_str, ML (mlStdMess116));
					break;
				case STD_INVOICE	:
					strcpy (err_str, ML (mlStdMess122));
					break;
				case PACK_SLIP		:
					strcpy (err_str, ML (mlSoMess227));
					break;
				case CCN			:
					strcpy (err_str, ML (mlSoMess376));
					break;

				default	: break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (typeFlag != PACK_SLIP && typeFlag != CCN)
		{
			if (cohr_rec.inv_print [0] != printFlag [0])
			{
				print_mess (ML ("Cannot reprint as document has not been printed"));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		if (typeFlag == CCN)
		{
			if (cohr_rec.ccn_print [0] != printFlag [0])
			{
				print_mess (ML ("Cannot reprint as document has not been printed"));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		if (typeFlag == PACK_SLIP)
		{
			if (cohr_rec.ps_print [0] != printFlag [0])
			{
				print_mess (ML ("Cannot reprint as document has not been printed"));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (strcmp (cohr_rec.inv_no, highInvoice) > 0)
		{
			/*----------------
			| Invalid range. |
			----------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------
			| Cannot find Customer (%ld) |
			--------------------------*/
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.customer [0], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("start_customer");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_inv_no")) 
	{
		if (dflt_used)
			strcpy (local_rec.inv_no [1], local_rec.inv_no [0]);

		if (!strcmp (local_rec.inv_no [1], "ALL     "))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.inv_no [1], zero_pad (local_rec.inv_no [1], 8));

		strcpy (lowInvoice, local_rec.inv_no [0]);
		strcpy (highInvoice, "~~~~~~~~");
		if (SRCH_KEY)
		{
			SrchInvoice (lowInvoice, temp_str, highInvoice);
			return (EXIT_SUCCESS);
		}

		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		strcpy (cohr_rec.type, TYPE._type);
		strcpy (cohr_rec.inv_print, printFlag);
		strcpy (cohr_rec.ccn_print, printFlag);
		strcpy (cohr_rec.ps_print, printFlag);
		strcpy (cohr_rec.inv_no, local_rec.inv_no [1]);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cohr_rec.type, TYPE._alt_type);
			cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-------------------------------------------------------
			| %s No %s not on file", TYPE._desc, local_rec.inv_no [1]) |
			-------------------------------------------------------*/
			switch (typeFlag)
			{
				case  INVOICE		:
					strcpy (err_str, ML (mlStdMess115));
					break;
				case CREDIT_NOTE	:
					strcpy (err_str, ML (mlStdMess116));
					break;
				case STD_INVOICE	:
					strcpy (err_str, ML (mlStdMess122));
					break;
				case PACK_SLIP		:
					strcpy (err_str, ML (mlSoMess227));
					break;
				case CCN			:
					strcpy (err_str, ML (mlSoMess376));
					break;

				default	: break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (typeFlag == PACK_SLIP)
		{
			if (cohr_rec.ps_print [0] != printFlag [0])
			{
				print_mess (ML (mlSoMess227));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		if (typeFlag == CCN)
		{
			if (cohr_rec.ccn_print [0] != printFlag [0])
			{
                sprintf (err_str, ML (mlStdMess116));
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (strcmp (cohr_rec.inv_no, lowInvoice) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.customer [1], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("end_customer");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
PrintAll (
 void)
{
	sprintf (local_rec.inv_no [0], "%-8.8s", "00000000");
	sprintf (local_rec.inv_no [1], "%-8.8s", "~~~~~~~~");
	sprintf (local_rec.customer [0], "%-30.30s", ML ("ALL Customers"));
	sprintf (local_rec.customer [1], "%-30.30s", ML ("ALL Customers"));
	DSP_FLD ("start_inv_no");
	DSP_FLD ("end_inv_no");
	DSP_FLD ("start_customer");
	DSP_FLD ("end_customer");

	entry_exit = 1;
	edit_exit = 1;

	if (prog_status == ENTRY)
		putval (line_cnt++);
}

int
DeleteLine (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	strcpy (local_rec.inv_no [0], "        ");
	strcpy (local_rec.inv_no [1], "        ");
	strcpy (local_rec.customer [0], "                    ");
	strcpy (local_rec.customer [1], "                    ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*==========================
| Search for order number. |
==========================*/
void
SrchInvoice (
	 char	*lowInvoice, 
	 char	*keyValue, 
	 char	*highInvoice)
{
	work_open ();
	sprintf (err_str, "#%s", TYPE._prmpt);
	save_rec (err_str, "#Customer");

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, TYPE._type);
	strcpy (cohr_rec.inv_print, printFlag);
	strcpy (cohr_rec.ccn_print, printFlag);
	strcpy (cohr_rec.ps_print, printFlag);
	sprintf (cohr_rec.inv_no, "%-8.8s", keyValue);

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
		!strcmp (cohr_rec.br_no, comm_rec.est_no))
	{
		if (typeFlag != PACK_SLIP && typeFlag != CCN)
		{
			if (cohr_rec.inv_print [0] != printFlag [0])
				break;
		}
		if (typeFlag == PACK_SLIP)
		{
			if (cohr_rec.ps_print [0] != printFlag [0] 
					|| cohr_rec.type [0] != TYPE._type [0])
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
		}
		if (typeFlag == CCN)
		{
			if (cohr_rec.ccn_print [0] != printFlag [0])
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
		}
		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = 0;

		cc = find_rec (coln, &coln_rec, GTEQ, "r");

		if (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash && 
		     strcmp (cohr_rec.inv_no, lowInvoice) >= 0 && 
		     strcmp (cohr_rec.inv_no, highInvoice) <= 0)
		{
			cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (!cc)
			{
				cc = save_rec (cohr_rec.inv_no, cumr_rec.dbt_name);
				if (cc)
					break;
			}
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, 	TYPE._type);
	strcpy (cohr_rec.inv_print, printFlag);
	strcpy (cohr_rec.ps_print, printFlag);
	strcpy (cohr_rec.ccn_print, printFlag);
	sprintf (cohr_rec.inv_no, "%-8.8s", temp_str);
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cohr_rec.type, TYPE._alt_type);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	}
	if (cc)
		file_err (cc, "cohr", "DBFIND");
}

int 
CustInvSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct InvoiceStructure a = * (const struct InvoiceStructure *) a1;
	const struct InvoiceStructure b = * (const struct InvoiceStructure *) b1;

	result = strcmp (a.sortField, b.sortField);

	return (result);
}

void
PrintFunction (
	int		printerNumber,
	long	hhcoHash,
	char 	*mode,
	char 	*typeFlag)
{
	int		c	=	0;
	static int	printAll = 0;
	char	*sptr;

	if (!printAll)
	{
		sptr = chk_env ("LINE_UP");
		if (sptr != (char *)0)
		{
			c = atoi (sptr);
			printAll = !(c);
		}

		fprintf (pout,"%d\n",printerNumber);
		fprintf (pout,"%s\n",mode);
		fflush (pout);

		if (printAll)
		{
			fprintf (pout,"0\n");
			fflush (pout);
			return;
		}
	}

	do
	{
		fprintf (pout,"%ld\n",hhcoHash);
		fflush (pout);
		if (!printAll)
		{
			switch (typeFlag [0])
			{
			case	'I':
			case	'i':
			strcpy (err_str,ML ("Reprint Invoice (for lineup) <Y/N> ? "));
			break;

			case	'C':
			case	'c':
			strcpy (err_str,ML ("Reprint Credit Note (for lineup) <Y/N> ? "));
			break;

			case	'P':
			case	'p':
			strcpy (err_str,ML ("Reprint Packing Slip (for lineup) <Y/N> ? "));
			break;

			case	'R':
			case	'r':
			strcpy (err_str,ML ("Reprint Remittance (for lineup) <Y/N> ? "));
			break;

			case	'S':
			case	's':
			strcpy (err_str,ML ("Reprint Statement (for lineup) <Y/N> ? "));
			break;

			case	'X':
			case	'x':
			strcpy (err_str,ML ("Reprint Insp. Slip (for lineup) <Y/N> ? "));
			break;

			default:
				return;
			}

			sleep (sleepTime);
			clear ();
			c = prmptmsg (err_str,"YyNn",26,1);
			if (mode[0] == 'M' && (c == 'N' || c == 'n'))
			{
				fprintf (pout,"0\n");
				fflush (pout);
			}
			clear ();
		}
	} while (!printAll && (c == 'Y' || c == 'y'));
	printAll = 1;
}

/*=============================================
| Check for environment for run program name. |
=============================================*/
char *   
CheckVariable (
	char *environmentName, 
	char *programName)
{
	char	*sptr;
	char	runPrint [41];

	/*---------------------------
	| Check Company & Branch	|
	---------------------------*/
	sprintf (runPrint,"%s%s%s",programName, comm_rec.co_no,comm_rec.est_no);
	sptr = chk_env (runPrint);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf (runPrint,"%s%s",environmentName,comm_rec.co_no);
		sptr = chk_env (runPrint);
		if (sptr == (char *)0)
		{
			sprintf (runPrint,"%s",environmentName);
			sptr = chk_env (runPrint);
			return ((sptr == (char *)0) ? programName : sptr);
		}
		else
			return (sptr);
	}
	else
		return (sptr);
}
int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	clear ();

	rv_pr (totalStr, (80 - strlen (totalStr)) / 2, 0, 1);

	move (0, 1);
	line (80);

	rv_pr (startStr, (40 - strlen (startStr)) / 2, tab_row - 2, 1);
	rv_pr (endStr, 38 + (40 - strlen (err_str)) / 2, tab_row - 2, 1);

	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21, 0, "%s", err_str);

	sprintf (err_str, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0, "%s", err_str);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

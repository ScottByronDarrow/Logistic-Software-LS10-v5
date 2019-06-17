/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_bktrsfr.c,v 5.3 2001/08/20 23:15:40 scott Exp $
|  Program Name  : (cr_bktrsfr.c) 
|  Program Desc  : (Input Bank Transfer Journals)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 01/06/89         |
|---------------------------------------------------------------------|
| $Log: cr_bktrsfr.c,v $
| Revision 5.3  2001/08/20 23:15:40  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 08:51:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:11  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_bktrsfr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_bktrsfr/cr_bktrsfr.c,v 5.3 2001/08/20 23:15:40 scott Exp $";

#define	MCURR		 (envVarCrMcurr [0] == 'Y')
#define MAXWIDTH	150 
#define MAXLINES	100 
#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int		printerNumber = TRUE,
			printerOpened = FALSE,
			OpenFlag	  = FALSE;

	int		updateData = FALSE;

	char	envVarCrMcurr [2];

	/*==============================	
	| Define file open for pformat |
	==============================*/
	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	dp_no [3];
	char	dc1_flag [2];
	char	dc2_flag [2];
	char	ca_desc [26];
	char	d_accdesc [26];
	char	gl_acc_no [MAXLEVEL + 1];
	char	gl_narrative [21];
	char	bk1_id [6];
	char	bk1_name [41];
	char	bk1_glacct [MAXLEVEL + 1];
	char	cur1_code [4];
	char	cur1_desc [41];
	double	bk1_orig_amt;
	double	bk1_rate;
	double	df1_rate;
	double	bk1_loc_amt;
	char	bk2_id [6];
	char	bk2_name [41];
	char	bk2_glacct [MAXLEVEL + 1];
	char	cur2_code [4];
	char	cur2_desc [41];
	double	bk2_rate;
	double	df2_rate;
	double	bk2_acct_amt;
	long	trsfr_date;
	long	post_date;
	} local_rec;

/*==========================
| Screen Static Structure. |
==========================*/

static	struct	var	vars []	={	
	{1, LIN, "trsfr_date", 4, 25, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", local_rec.systemDate, "Transfer Date", "<retn> defaults to system (todays) date ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.trsfr_date}, 
	{1, LIN, "bank1_id", 5, 25, CHARTYPE, 
		"UUUUU", "          ", 
		" ", "", "Issuing Bank Code", "Enter code or [SEARCH] key ", 
		YES, NO, JUSTLEFT, "", "", local_rec.bk1_id}, 
	{1, LIN, "bk1_name", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.bk1_name}, 
	{1, LIN, "bk1_curr", 6, 25, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Currency Code", "Enter code or [SEARCH] Key. ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cur1_code}, 
	{1, LIN, "cur1_desc", 6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cur1_desc}, 
	{1, LIN, "bk1_orgamt", 7, 25, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Amt Transfr (acct curr)", " ", 
		YES, NO, JUSTRIGHT, "0", "999999999.99", (char *)&local_rec.bk1_orig_amt}, 
	{1, LIN, "bk1_exch", 8, 25, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", "1.0000", "Exchange Rate", "<Retn> defaults to current rate for currency ", 
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.bk1_rate}, 
	{1, LIN, "bk1_locamt", 9, 25, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Amt Transfr (local curr)", "<Retn> defaults to calc. value, else recalcs exch. rate ", 
		YES, NO, JUSTRIGHT, "0", "999999999.99", (char *)&local_rec.bk1_loc_amt}, 
	{1, LIN, "bank2_id", 11, 25, CHARTYPE, 
		"UUUUU", "          ", 
		" ", "", "Receiving Bank Code", "Enter code or [SEARCH] key. ", 
		YES, NO, JUSTLEFT, "", "", local_rec.bk2_id}, 
	{1, LIN, "bk2_name", 11, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.bk2_name}, 
	{1, LIN, "bk2_curr", 12, 25, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Currency Code", "Enter code or [SEARCH] key. ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cur2_code}, 
	{1, LIN, "cur2_desc", 12, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cur2_desc}, 
	{1, LIN, "bk2_exch", 13, 25, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", "1.0000", "Exchange Rate", "<Retn> defaults to current rate for currency ", 
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.bk2_rate}, 
	{1, LIN, "bk2_amt", 14, 25, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Amt Recvd (acct curr)", "<Retn> defaults to calc. value, else recalcs exch. rate ", 
		YES, NO, JUSTRIGHT, "0", "999999999.99", (char *)&local_rec.bk2_acct_amt}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include 	<twodec.h>
#include	<CashBook.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		   	 (void);
int		FindGlmr		 (char *);
int		spec_valid		 (int);
void	SrchCrbk		 (char *);
int		Update			 (void);
void	PrintCopyLine	 (void);
int		OpenAudit		 (void);
int		CloseAudit		 (void);
int		heading			 (int);
int		Check_class		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 2 || (printerNumber = atoi (argv [1])) < 1)
	{
		print_at (0,0, "Usage: %s <printerNumber>",argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (envVarCrMcurr, "%-1.1s", get_env ("CR_MCURR"));
	
	if (!MCURR)
	{
		print_at (0,0, ML ("Program terminated : This option is only used for MULTI CURRENCY SUPPLIERS."));
		return (EXIT_FAILURE);
	}

	/*----------------------------
	| Setup required parameters. |
	-------------------------------------------------------------*/
	SETUP_SCR (vars);	/*  sets vars for scrgen functions. 	|*/
	init_scr ();		/*  sets terminal from termcap.     	|*/
	set_tty ();         /*  get into raw mode.              	|*/
	set_masks ();		/*  setup print using masks.        	|*/
	init_vars (1);		/*  set default values.             	|*/
                        /*--------------------------------------|*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.post_date = TodaysDate ();

	strcpy (local_rec.gl_narrative, "Bank Transfer Jnl   ");

	OpenDB ();

	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		if (Update () == -1)
			return (EXIT_FAILURE);
	}	/* end of input control loop	*/

	if (updateData)
	{
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	else
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	if (OpenFlag)
		CloseAudit ();

	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlmr ();
	OpenPocr ();
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenCashBook ();
}

/*=========================
| Close Data Base Files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	GL_CloseBatch (printerNumber);
	CloseCashBook ();
	GL_Close ();
	abc_dbclose ("data");
}

/*============================
| Read gl account master.    |
============================*/
int
FindGlmr (
 char *account)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P') 
	      return (2);
	
	return (EXIT_SUCCESS);
}

/*============================
| Special Validation Routine.|
============================*/
int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Transfer Posting Date. |
	---------------------------------*/
	if (LCHECK ("trsfr_date"))
	{
		if (dflt_used)
			DSP_FLD ("trsfr_date");

		if (local_rec.trsfr_date > StringToDate (local_rec.systemDate))
		{
			errmess (ML ("Date may not be greater than today."));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Bank Id Code And Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("bank1_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		strcpy (crbk_rec.bank_id,local_rec.bk1_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Bank not found."));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.bk1_name, crbk_rec.bank_name);
		DSP_FLD ("bk1_name");

		/*--------------------------------
		| Read Bank GL Account Record.   |
		--------------------------------*/
		sprintf (local_rec.bk1_glacct,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,local_rec.bk1_glacct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Account is not in the General Ledger."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (Check_class ())
			return (EXIT_FAILURE);

		/*--------------------------------
		| Read Bank Currency Record.     |
		--------------------------------*/
		strcpy (local_rec.cur1_code, crbk_rec.curr_code);
		DSP_FLD ("bk1_curr");
		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code, crbk_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Currency not found"));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.cur1_desc, pocrRec.description);
		DSP_FLD ("cur1_desc");
		local_rec.df1_rate = pocrRec.ex1_factor;
		if (local_rec.df1_rate == 0.00)
			local_rec.df1_rate = 1.0000;

		return (EXIT_SUCCESS);
	}
	/*------------------------------------------------------
	| Validate Overseas Original Amount To Be Transferred. |
	------------------------------------------------------*/
	if (LCHECK ("bk1_orgamt"))
	{
		if (prog_status != ENTRY)
		{
			local_rec.bk1_loc_amt = local_rec.bk1_orig_amt / local_rec.bk1_rate;
			local_rec.bk1_loc_amt = no_dec (local_rec.bk1_loc_amt);
			DSP_FLD ("bk1_locamt");
			local_rec.bk2_acct_amt = local_rec.bk1_loc_amt * local_rec.bk2_rate;
			local_rec.bk2_acct_amt = no_dec (local_rec.bk2_acct_amt);
			DSP_FLD ("bk2_amt");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("bk1_exch")) 
	{
		if (dflt_used || local_rec.bk1_rate == 0.0)
			local_rec.bk1_rate = local_rec.df1_rate;

		local_rec.bk1_loc_amt = local_rec.bk1_orig_amt / local_rec.bk1_rate;
		local_rec.bk1_loc_amt = no_dec (local_rec.bk1_loc_amt);
		DSP_FLD ("bk1_exch");
		DSP_FLD ("bk1_locamt");

		if (prog_status != ENTRY)
		{
			local_rec.bk2_acct_amt = local_rec.bk1_loc_amt * local_rec.bk2_rate;
			local_rec.bk2_acct_amt = no_dec (local_rec.bk2_acct_amt);
			DSP_FLD ("bk2_exch");
		}
	    	return (EXIT_SUCCESS);
	}

	/*-------------------------------------------------
	| Validate Local Amount To Be Transferred.        |
	-------------------------------------------------*/
	if (LCHECK ("bk1_locamt"))
	{
		if (dflt_used)
		{
			local_rec.bk1_loc_amt = local_rec.bk1_orig_amt / local_rec.bk1_rate;
			local_rec.bk1_loc_amt = no_dec (local_rec.bk1_loc_amt);
			DSP_FLD ("bk1_locamt");
			return (EXIT_SUCCESS);
		}

		local_rec.bk1_rate = local_rec.bk1_orig_amt / local_rec.bk1_loc_amt;
		DSP_FLD ("bk1_exch");

		if (prog_status != ENTRY)
		{
			local_rec.bk2_acct_amt = local_rec.bk1_loc_amt * local_rec.bk2_rate;
			local_rec.bk2_acct_amt = no_dec (local_rec.bk2_acct_amt);
			DSP_FLD ("bk2_exch");
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Bank Id Code And Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("bank2_id")) 
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		strcpy (crbk_rec.bank_id,local_rec.bk2_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Bank not found."));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.bk2_name, crbk_rec.bank_name);
		DSP_FLD ("bk2_name");

		/*--------------------------------
		| Read Bank GL Account Record.   |
		--------------------------------*/
		sprintf (local_rec.bk2_glacct,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,local_rec.bk2_glacct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Account is not in the General Ledger."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (Check_class ())
			return (EXIT_FAILURE);

		/*--------------------------------
		| Read Bank Currency Record.     |
		--------------------------------*/
		strcpy (local_rec.cur2_code, crbk_rec.curr_code);
		DSP_FLD ("bk2_curr");
		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code, crbk_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML ("Currency not found"));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.cur2_desc, pocrRec.description);
		DSP_FLD ("cur2_desc");
		local_rec.df2_rate = pocrRec.ex1_factor;
		if (local_rec.df2_rate == 0.00)
			local_rec.df2_rate = 1.0000;
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("bk2_exch"))
	{
		if (dflt_used || local_rec.bk2_rate == 0.0)
			local_rec.bk2_rate = local_rec.df2_rate;

		local_rec.bk2_acct_amt = local_rec.bk1_loc_amt * local_rec.bk2_rate;
		local_rec.bk2_acct_amt = no_dec (local_rec.bk2_acct_amt);
		DSP_FLD ("bk2_exch");
		DSP_FLD ("bk2_amt");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------------------
	| Validate Local Amount To Be Transferred.        |
	-------------------------------------------------*/
	if (LCHECK ("bk2_amt"))
	{
		if (dflt_used)
		{
			local_rec.bk2_acct_amt = local_rec.bk1_loc_amt * local_rec.bk2_rate;
			local_rec.bk2_acct_amt = no_dec (local_rec.bk2_acct_amt);
			DSP_FLD ("bk2_exch");
	    		return (EXIT_SUCCESS);
		}

		local_rec.bk2_rate = local_rec.bk2_acct_amt / local_rec.bk1_loc_amt;
		DSP_FLD ("bk2_exch");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Suppliers Bank File. |
=========================================*/
void
SrchCrbk (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code", "#Bank Name");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
				  !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id,crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (crbk_rec.co_no,comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND");
}

/*=======================
| Update & Print Journal|
=======================*/
int
Update (
 void)
{
	int		monthPeriod;
	clear ();
	print_at (0,0, ML ("Updating bank transfers.... "));
	fflush (stdout);

	updateData = TRUE;

	/*------------------------------------
	| Set switch if something is printed |
	------------------------------------*/
	if (!printerOpened) 
	{ 
		if (OpenAudit () != 0)
		{
			shutdown_prog ();
			return (-1);
		}
	}

	printerOpened = 1;

	/*------------------------------------
	| Print transfer bank journal detail.|
	------------------------------------*/
	fprintf (fout, "| %s", local_rec.bk1_id);
	fprintf (fout, "|%-25.25s", local_rec.bk1_name);
	fprintf (fout, "|%-10.10s", DateToString (local_rec.trsfr_date));
	fprintf (fout, "| %s ", local_rec.cur1_code);
	fprintf (fout, "|%13.2f",DOLLARS (local_rec.bk1_orig_amt));
	fprintf (fout, "|%9.4f",local_rec.bk1_rate);
	fprintf (fout, "|%11.2f",DOLLARS (local_rec.bk1_loc_amt));
	fprintf (fout, "|%-16.16s", local_rec.bk1_glacct);
	fprintf (fout, "| C ");
	fprintf (fout, "|%10.2f ", DOLLARS (local_rec.bk1_loc_amt));
	DateToDMY (local_rec.trsfr_date, NULL, &monthPeriod, NULL);
	fprintf (fout, "| %2.2d",mth2fisc (monthPeriod , comm_rec.fiscal));
	fprintf (fout, "|%s|\n", local_rec.gl_narrative);


	/*---------------------------------
	| Write entry to cash book system |
	---------------------------------*/
	WriteCashBook
	 (								/*--------------------------*/
		comm_rec.co_no,			/* Company Number			*/
		comm_rec.est_no,			/* Branch Number.			*/
		local_rec.bk1_id, 			/* Bank Id.					*/
		local_rec.trsfr_date,		/* Transaction Date			*/
		"Bank Transfer Journal",	/* Transaction Narrative.	*/
		"T",						/* Transaction Type.		*/
		local_rec.bk1_loc_amt * -1,	/* Amount posted to bank.	*/
		"0",						/* Status flag.				*/
		local_rec.post_date 		/* System/period date.		*/
	);								/*--------------------------*/

	/*------------------------------------------------
	| Get gl acct hash & write glwk transaction.     |
	------------------------------------------------*/
	cc = FindGlmr (local_rec.bk1_glacct);
	if (cc) 
		file_err (cc, glmr, "DBFIND");
	else
	{
		sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,local_rec.bk1_glacct);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	}
	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, "18");
	glwkRec.post_date = local_rec.post_date;
	glwkRec.tran_date = local_rec.trsfr_date;
	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	sprintf (glwkRec.user_ref, 		"%5.5s / %5.5s", 
						local_rec.bk1_id, local_rec.bk2_id);
	sprintf (glwkRec.alt_desc1, 		"%20.20s", " ");
	sprintf (glwkRec.alt_desc2, 		"%20.20s", " ");
	sprintf (glwkRec.alt_desc3, 		"%20.20s", " ");
	sprintf (glwkRec.batch_no, 	"%10.10s", " ");
	sprintf (glwkRec.stat_flag, "2");
	
	DateToDMY (local_rec.trsfr_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);
	strcpy (glwkRec.narrative, "Bank Transfer Jnl   ");

	glwkRec.amount = local_rec.bk1_orig_amt;
	glwkRec.loc_amount = local_rec.bk1_loc_amt;
	strcpy (glwkRec.currency, local_rec.cur1_code);
	glwkRec.exch_rate = local_rec.bk1_rate;
	strcpy (glwkRec.jnl_type,"2");

	GL_AddBatch ();

	/*------------------------------------
	| Print receipt bank journal detail. |
	------------------------------------*/
	fprintf (fout, "| %s", local_rec.bk2_id);
	fprintf (fout, "|%-25.25s", local_rec.bk2_name);
	fprintf (fout, "|%-10.10s", DateToString (local_rec.trsfr_date));
	fprintf (fout, "| %s ", local_rec.cur2_code);

	fprintf (fout,"|%13.2f",DOLLARS (local_rec.bk2_acct_amt));
	fprintf (fout,"|%9.4f",local_rec.bk2_rate);
	fprintf (fout,"|%11.2f",DOLLARS (local_rec.bk1_loc_amt));

	fprintf (fout,"|%-16.16s", local_rec.bk2_glacct);
	fprintf (fout,"| D |");
	fprintf (fout,"%10.2f ", DOLLARS (local_rec.bk2_acct_amt));
	DateToDMY (local_rec.trsfr_date, NULL, &monthPeriod, NULL);
	fprintf (fout,"| %2.2d",mth2fisc (monthPeriod , comm_rec.fiscal));
	fprintf (fout,"|%s|\n", local_rec.gl_narrative);

	/*---------------------------------
	| Write entry to cash book system |
	---------------------------------*/
	WriteCashBook
	 (								/*--------------------------*/
		comm_rec.co_no,			/* Company Number			*/
		comm_rec.est_no,			/* Branch Number.			*/
		local_rec.bk2_id, 			/* Bank Id.					*/
		local_rec.trsfr_date,		/* Transaction Date			*/
		"Bank Transfer Journal",	/* Transaction Narrative.	*/
		"T",						/* Transaction Type.		*/
		local_rec.bk1_loc_amt,		/* Amount posted to bank.	*/
		"0",						/* Status flag.				*/
		local_rec.post_date 		/* System/period date.		*/
	);								/*--------------------------*/

	/*------------------------------------------------
	| Get gl acct hash & write glwk transaction.     |
	------------------------------------------------*/
	cc = FindGlmr (local_rec.bk2_glacct);
	if (cc) 
		file_err (cc, glmr, "DBFIND");
	else
	{
		sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,local_rec.bk2_glacct);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	}
	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, "18");
	glwkRec.post_date = local_rec.post_date;
	glwkRec.tran_date = local_rec.trsfr_date;
	DateToDMY (local_rec.trsfr_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);
	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	sprintf (glwkRec.user_ref,"%5.5s / %5.5s", local_rec.bk1_id, local_rec.bk2_id);
	sprintf (glwkRec.alt_desc1, 		"%20.20s", " ");
	sprintf (glwkRec.alt_desc2, 		"%20.20s", " ");
	sprintf (glwkRec.alt_desc3, 		"%20.20s", " ");
	sprintf (glwkRec.batch_no, 	"%10.10s", " ");
	strcpy (glwkRec.stat_flag, "2");
	strcpy (glwkRec.narrative, local_rec.gl_narrative);
	strcpy (glwkRec.jnl_type,"1");

	glwkRec.amount = local_rec.bk2_acct_amt;
	glwkRec.loc_amount = local_rec.bk1_loc_amt;
	strcpy (glwkRec.currency, local_rec.cur2_code);
	glwkRec.exch_rate = local_rec.bk2_rate;

	GL_AddBatch ();
	return (EXIT_SUCCESS);
}

/*==================================
| Print Company/Branch/Department. |
==================================*/
void
PrintCopyLine (
 void)
{
	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,  err_str, comm_rec.co_no, comm_rec.co_short);

	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0, err_str, comm_rec.est_no, comm_rec.est_name);
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
int
OpenAudit (
 void)
{
	OpenFlag = TRUE;

	if ( (fout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".11\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".ESUPPLIER BANK TRANSFER JOURNALS\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s as at %s\n", clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B1\n");
	fprintf (fout,".R=============================================");
	fprintf (fout,"==========================================");
	fprintf (fout,"==========================================================\n");

	fprintf (fout,"=============================================");
	fprintf (fout,"==========================================");
	fprintf (fout,"==========================================================\n");

	fprintf (fout,"| BANK | BANK NAME               | DATE  OF |");
	fprintf (fout," CURR|    AMOUNT   | EXCHANGE| EQUIV.LOC.|");
	fprintf (fout,"    ACCOUNT     |D/C|G/L  AMOUNT|PER|   G/L  NARRATIVE   |\n");
	
	fprintf (fout,"| CODE |                         | JOURNAL  |");
	fprintf (fout," CODE|             |   RATE  |   AMOUNT  |");
	fprintf (fout,"    NUMBER      |   |           |NO |                    |\n");

	fprintf (fout,"|------|-------------------------|----------|");
	fprintf (fout,"-----|-------------|---------|-----------|");
	fprintf (fout,"----------------|---|-----------|---|--------------------|\n");

	fprintf (fout,".PI12\n");
	return (EXIT_SUCCESS);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
int
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
	return (EXIT_SUCCESS);
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
	rv_pr (ML ("Bank Transfer Journal Input"),26,0,1);

	line_at (1,0,80);

	move (1,input_row);
	if (scn == 1)
	{
		box (0,3,80,11);
		line_at (10,1,79);
	}

	line_at (20,0,80);
	PrintCopyLine ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

int
Check_class (
 void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
		return print_err (ML ("Account is not a Posting Level Account."));

	return (EXIT_SUCCESS);
}

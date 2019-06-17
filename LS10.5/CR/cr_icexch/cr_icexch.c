/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_icexch.c,v 5.2 2001/08/09 08:51:53 scott Exp $
|  Program Name  : (cr_icexch.c )                                     |
|  Program Desc  : (Update Exchange Rate For Selected Invoices &  )   |
|                 (Credit Notes.                                 )    |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 07/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_icexch.c,v $
| Revision 5.2  2001/08/09 08:51:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:25  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_icexch.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_icexch/cr_icexch.c,v 5.2 2001/08/09 08:51:53 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#define PAGELINES	65
#define	MCURR		 (multiCurrency [0] == 'Y')
#define	MOD  		1	/* alters frequency for dsp_process */

#include <pslscr.h>
#include <GlUtils.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#define	BY_INVOICE	 (selectType [0] == 'I')
#define	BY_CURRENCY	 (selectType [0] == 'C')
#define	BY_ALL		 (selectType [0] == 'A')

#define	EXCH_OK		 (suin_rec.er_fixed [0] != 'Y')

#define	DATE_OK		 (suin_rec.date_of_inv >= local_rec.from_date && \
	                  suin_rec.date_of_inv <= local_rec.to_date)

#define	AMT_OK 		 (suin_rec.amt != suin_rec.amt_paid)

#define	CURR_OK		 (!strcmp (sumr_rec.curr_code,pocrRec.code))

#define	APPROVED	 (suin_rec.approved [0] == 'Y')

#define	INVOICE		 (suin_rec.type [0] == '1')
#define	CREDIT 		 (suin_rec.type [0] == '2')

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

   	int		pidNumber,
			glTrandFound = 0,
			printerNumber = 1,
			envCrCo = 0,
			envCrFind = 0;

	char 	multiCurrency [2],
	    	branchNumber [3],
	    	selectType [2];

	char	*sptr;

   	double 	exchangeVariance = 0.00,
			exchVarianceTotal = 0.00,
			newDocumentBalance = 0.00;

	FILE	*fin,
			*ferr,
			*fout,
			*fsort;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct sumrRecord	sumr_rec;
struct suinRecord	suin_rec;
struct wfbcRecord	wfbc_rec;

	struct {
		long	wk_hash;
	} wk_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	crd_no [7];
	double	inv_balance;
	double	old_rate;
	double	new_rate;
	long	from_date;
	long	to_date;
	char 	systemDate [11];
	char 	acc_no [MAXLEVEL + 1];
	char 	inv_no [16];
	char 	gl_user_ref [21];
	char	loc_curr [4];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplier",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "A", "Select Supplier", "Enter Supplier No., [SEARCH], or <retn> for ALL",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{1, LIN, "name",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "doc_no",	 6, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Select Document", "Enter Document No., [SEARCH], or <retn> for ALL",
		 NE, NO,  JUSTLEFT, "", "", local_rec.inv_no},
	{1, LIN, "old_rate",	 7, 20, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "Old Exchange Rate", " ",
		 NA, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.old_rate},
	{1, LIN, "from_date",	 9, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Document From Date", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.from_date},
	{1, LIN, "to_date",	10, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Document  To  Date", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.to_date},
	{1, LIN, "curr",	12, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", " ",
		YES, NO,  JUSTLEFT, "", "", pocrRec.code},
	{1, LIN, "curr_desc",	12, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "new_rate",	13, 20, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "New Exchange Rate", " ",
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.new_rate},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
int		FindGlmr			 (char *);
int		spec_valid			 (int);
int		FindSupplier		 (long);
int		PrintControlAccount	 (char *, double, char *);
int		WriteGlTrans		 (char *, double, char *);
int		check_page			 (void);
int		heading				 (int scn);
void	CloseDB			 	 (void);
void	shutdown_prog		 (void);
void	OpenDB				 (void);
void	SrchSuin			 (char *);
void	ProcessInvoice		 (void);
void	ProcessDocument		 (void);
void	StoreAccount		 (void);
void	ProcessSortedList	 (void);
void	UpdateWfbc			 (void);
void	HeadingOutput		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 3)
	{
		print_at (0,0,mlCrMess704, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber  	= atoi (argv [1]);
	pidNumber   	= atoi (argv [2]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	init_scr ();

	if (!MCURR) 
	{
		no_option ("CR_MCURR (Multi Currency Suppliers.)");
		return (EXIT_FAILURE);
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	set_tty 	 ();
	set_masks 	 ();
	init_vars 	 (1);
    
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	envCrCo 	= atoi (get_env ("CR_CO"));
	envCrFind  	= atoi (get_env ("CR_FIND"));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		abc_unlock (sumr);
		abc_unlock (suin);
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		FLD ("doc_no") 	 	= YES;
		FLD ("from_date") 	= YES;
		FLD ("to_date")   	= YES;
		FLD ("curr")      	= YES;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		/*--------------------------------
		| Update invoice & glwk records. |
		--------------------------------*/
		ProcessInvoice ();
	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
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

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (local_rec.loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");
	OpenPocr ();
	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (esmr);
	abc_fclose (suin);
	abc_fclose (wfbc);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	abc_dbclose ("data");
}

/*============================
| Read gl account master.    |
============================*/
int
FindGlmr (
 char *	account)
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

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Supplier Number Input. |
	---------------------------------*/
	if (LCHECK ("supplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.crd_no,"ALL   ");
			sprintf (sumr_rec.crd_name, "%-40.40s", " ");
			strcpy (local_rec.inv_no,"ALL            ");

			DSP_FLD ("supplier");
			DSP_FLD ("name");
			DSP_FLD ("doc_no");

			strcpy (selectType, "A");

			FLD ("doc_no")    = NA;
			FLD ("from_date") = YES;
			FLD ("to_date")   = YES;
			FLD ("curr")      = YES;
			FLD ("new_rate")  = YES;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		cc = FindPocr (comm_rec.co_no, sumr_rec.curr_code, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		FLD ("doc_no")    = YES;
		FLD ("from_date") = YES;
		FLD ("to_date")   = YES;
		FLD ("curr")      = NA;
		FLD ("new_rate")  = YES;

		DSP_FLD ("name");
		DSP_FLD ("curr");
		DSP_FLD ("curr_desc");
		strcpy (selectType,"C");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Invoice Number Input. |
	--------------------------------*/
	if (LCHECK ("doc_no"))
	{
		if (FLD ("doc_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.inv_no,"ALL            ");
			FLD ("doc_no")	   = YES;
			FLD ("from_date") = YES;
			FLD ("to_date")  = YES;
			FLD ("curr")      = NA;
			FLD ("new_rate")  = YES;
			DSP_FLD ("doc_no");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchSuin (temp_str);
			return (EXIT_SUCCESS);
		}
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no,local_rec.inv_no);
		cc = find_rec (suin, &suin_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (err_str, ML (mlCrMess032),suin_rec.inv_no);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (strcmp (sumr_rec.curr_code, suin_rec.currency))
		{
			sprintf (err_str,ML (mlCrMess033),suin_rec.currency);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		local_rec.old_rate = suin_rec.exch_rate;
		strcpy (selectType,"I");

		FLD ("doc_no")    = YES;
		FLD ("from_date") = NA;
		FLD ("to_date")   = NA;
		FLD ("curr")      = NA;
		FLD ("new_rate")  = YES;

		DSP_FLD ("doc_no");
		DSP_FLD ("old_rate");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Inv/Crd From Date. |	
	-----------------------------*/
	if (LCHECK ("from_date"))
	{
		if (dflt_used)
			local_rec.from_date	=	MonthStart (comm_rec.crd_date);
		/*	local_rec.from_date	=	MonthStart (TodaysDate ());*/

		local_rec.from_date	=	MonthStart (comm_rec.crd_date);
		local_rec.to_date	=	MonthEnd (comm_rec.crd_date);
	
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Inv/Crd To Date.   |	
	-----------------------------*/
	if (LCHECK ("to_date"))
	{
		if (dflt_used)
			local_rec.to_date	=	MonthEnd (comm_rec.crd_date);
		/*	local_rec.to_date	=	MonthEnd (TodaysDate ());*/

		if (local_rec.from_date > local_rec.to_date)
		{ 
			errmess (ML (mlCrMess034));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Currency Code.       |
	-------------------------------*/
	if (LCHECK ("curr"))
	{
		if (SRCH_KEY)
		{
		   SearchPocr (comm_rec.co_no, temp_str);
		   return (EXIT_SUCCESS);
		}
		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		cc = FindPocr (comm_rec.co_no, pocrRec.code, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("new_rate"))
	{
		if (dflt_used)
			local_rec.new_rate = pocrRec.ex1_factor;

		DSP_FLD ("new_rate");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchSuin (
 char *	key_val)
{
	char	disp_amt [51];

	_work_open (15,0,50);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no,key_val);
	save_rec ("#Invoice","#Tran Type|    Amount.     | App ");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val,strlen (key_val)) && 
			suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if (INVOICE || CREDIT)
		{
			local_rec.inv_balance = suin_rec.amt - suin_rec.amt_paid;

			sprintf (disp_amt, " %7.7s | %-14.2f | %-3.3s ",
					 (INVOICE) ? "Invoice" : "C/Note.",
					DOLLARS (local_rec.inv_balance), 
					 (APPROVED) ? "Yes" : "No ");

			cc = save_rec (suin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no,temp_str);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");
}

/*================================
| Update Invoice & GL Work File. |
================================*/
void
ProcessInvoice (
 void)
{
	dsp_screen ("Processing Invoices", comm_rec.co_no, comm_rec.co_name);

	/*----------------------------------------
	| Start audit report & open sort file.   |
	----------------------------------------*/
	glTrandFound = 0;
	HeadingOutput ();

	fsort = sort_open ("cr_exvn");

	/*----------------------------------------
	| Update selection on inv/cn document.   |
	----------------------------------------*/
	if (BY_INVOICE)
	{
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no,local_rec.inv_no);
		cc = find_rec (suin, &suin_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, suin, "DBFIND");

		if (EXCH_OK && AMT_OK)
			ProcessDocument ();

		abc_unlock (suin);
	}

	/*----------------------------------------
	| Update selection on supplier.          |
	----------------------------------------*/
	if (BY_CURRENCY)
	{
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no,"               ");
		cc = find_rec (suin, &suin_rec, GTEQ, "u");
		while (!cc && (suin_rec.hhsu_hash == sumr_rec.hhsu_hash))
		{
			if (DATE_OK && EXCH_OK && AMT_OK)
				ProcessDocument ();

			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
		abc_unlock (suin);
	}

	/*----------------------------------------
	| Update selection on all suppliers.     |
	----------------------------------------*/
	if (BY_ALL)
	{
		abc_selfield (sumr,"sumr_hhsu_hash");

		suin_rec.hhsu_hash = 0;
		strcpy (suin_rec.inv_no,"               ");
		cc = find_rec (suin, &suin_rec, GTEQ, "u");
		while (!cc)
		{
			if (FindSupplier (suin_rec.hhsu_hash))
			{
				abc_unlock (suin);
				cc = find_rec (suin, &suin_rec, NEXT, "u");
				continue;
			}
			if (DATE_OK && EXCH_OK && AMT_OK && CURR_OK)
				ProcessDocument ();
			
			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
		abc_unlock (suin);
	}
	if (glTrandFound != 0)
	{
		ProcessSortedList ();
		UpdateWfbc ();
	}
}

/*=============================================
| Find supplier and check for company number. |
=============================================*/
int
FindSupplier (
 long	hhsu_hash)
{
	cc = find_hash (sumr,&sumr_rec,COMPARISON,"r", hhsu_hash);
	if (cc || strcmp (sumr_rec.co_no, comm_rec.co_no))
		return (TRUE);

	return (FALSE);
}

/*=====================================
| Process Invoice / Cn Document.      |
=====================================*/
void
ProcessDocument (
 void)
{
	char	value_str [13];
	double	dbcr_value = 0.00;
   	double 	tmpExchVar = 0.00;

	dsp_process ("Invoice", suin_rec.inv_no);

	if (local_rec.new_rate == 0.00 || suin_rec.exch_rate == 0.00)
	{
		tmpExchVar = 0.00;	
	}
	else
	{
		tmpExchVar = (1.0 / local_rec.new_rate) - 
			     (1.0 / suin_rec.exch_rate);
	}

	local_rec.old_rate = psl_round (suin_rec.exch_rate,8);
	newDocumentBalance = twodec (suin_rec.amt - suin_rec.amt_paid);
	exchangeVariance = tmpExchVar * newDocumentBalance;
	exchangeVariance = twodec (exchangeVariance);

	if (exchangeVariance == 0.0)
	{
		abc_unlock (suin);
		return;
	}

	/*--------------------
	| Update Inv/Cn      |
	--------------------*/
	suin_rec.exch_rate = local_rec.new_rate;
	cc = abc_update (suin,&suin_rec);
	if (cc)
		file_err (cc, suin, "DBUPDATE");

	/*--------------------
	| Store In Sort File |
	--------------------*/
	if (APPROVED)
		StoreAccount ();

	/*--------------------
	| Print Inv/Cn Detail|
	--------------------*/
	pr_format (fin,fout,"DOC_DET",1,sumr_rec.crd_no);
	pr_format (fin,fout,"DOC_DET",2,suin_rec.inv_no);
	pr_format (fin,fout,"DOC_DET",3,newDocumentBalance);
	pr_format (fin,fout,"DOC_DET",4,local_rec.old_rate);
	pr_format (fin,fout,"DOC_DET",5,local_rec.new_rate);
	pr_format (fin,fout,"DOC_DET",6,exchangeVariance);

	/*--------------------
	| Credit Value       |
	--------------------*/
	if (exchangeVariance > 0)
	{
 		sprintf (value_str,"%12.2f",DOLLARS (exchangeVariance));
		pr_format (fin,fout,"DOC_DET",7,"            ");
		pr_format (fin,fout,"DOC_DET",8,value_str);
	}

	/*--------------------
	| Debit Value        |
	--------------------*/
	else
	{
		dbcr_value = exchangeVariance * -1;
		dbcr_value = twodec (dbcr_value);
 		sprintf (value_str,"%12.2f",DOLLARS (dbcr_value));
		pr_format (fin,fout,"DOC_DET",7,value_str);
		pr_format (fin,fout,"DOC_DET",8,"            ");
	}
}

/*====================================
| Store Account Detail In Sort File. |
====================================*/
void
StoreAccount (
 void)
{
	char	data_str [31],
			account_no [MAXLEVEL + 1];

	strcpy (account_no, sumr_rec.gl_ctrl_acct);
	if (FindGlmr (account_no))
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"  ",
			"ACCT PAY  ",
			"   ",
			" "
		);
		strcpy (account_no, glmrRec.acc_no);
	}
	sprintf (data_str,"%-*.*s %12.2f \n", 
					MAXLEVEL, MAXLEVEL, account_no, DOLLARS (exchangeVariance));

	sort_save (fsort, data_str);
	glTrandFound++;
}

/*========================================
| Process Account Detail From Sort File. |
========================================*/
void
ProcessSortedList (
 void)
{
	char	previousAccount [MAXLEVEL + 1];
	char	currentAccount [MAXLEVEL + 1];
	char	currentValueString [13];
	int		firstRecord = 1;
	double	currentValue = 0.00;
	double	accountTotal = 0.00;

	exchVarianceTotal = 0.0;

	fsort = sort_sort (fsort,"cr_exvn");
	sptr = sort_read (fsort);

	if (sptr)
		sprintf (previousAccount,"%-*.*s",MAXLEVEL,MAXLEVEL,"                ");

	while (sptr)
	{
		sprintf (currentAccount,"%-*.*s", MAXLEVEL, MAXLEVEL, sptr);
		sprintf (currentValueString,"%-12.12s",sptr + MAXLEVEL + 1);
		currentValue = atof (currentValueString);

		if (firstRecord == 1)
		{
			pr_format (fin,fout,"LINE1",0,0);
			firstRecord = 0;
			strcpy (previousAccount,currentAccount);
			sprintf (temp_str,ML (mlCrMess184), currentAccount);
			us_pr (temp_str,20,18,1);
		}

		if (strcmp (previousAccount,currentAccount))
		{
			PrintControlAccount 
			 (
				previousAccount,
				accountTotal,
				"Ctrl Acct"
			);
			WriteGlTrans 
			 (
				previousAccount,
				accountTotal,
				"Ctrl Acct"
			);
			exchVarianceTotal -= accountTotal;
			accountTotal = currentValue;
			strcpy (previousAccount,currentAccount);
			sprintf (temp_str,ML (mlCrMess184), currentAccount);
			us_pr (temp_str,20,18,1);
		}
		else
			accountTotal += currentValue;

		sptr = sort_read (fsort);
	}
	PrintControlAccount	 
	 (
		previousAccount,
		accountTotal,
		"Ctrl Acct"
	);
	WriteGlTrans	 
	 (
		previousAccount,
		accountTotal,
		"Ctrl Acct"
	);
	exchVarianceTotal -= accountTotal;

	PrintControlAccount	
	 (
		pocrRec.gl_exch_var,
		exchVarianceTotal,
		"Exch Var."
	);
	WriteGlTrans 
	 (
		pocrRec.gl_exch_var,
		exchVarianceTotal,
		"Exch Var."
	);

	sort_delete (fsort, "cr_exvn");
	pr_format (fin,fout, "END_FILE", 0, 0);
	pclose (fout);
}

/*==========================================
| Print Summarised Control Account Detail. |
==========================================*/
int
PrintControlAccount (
 char *	acct,
 double	value,
 char *	type)
{
	char	value_str [13];

	if (!value)
		return (EXIT_SUCCESS);

	fprintf (fout, ".LRP7\n");

	pr_format (fin,fout,"ACCT_DET",1,type);
	pr_format (fin,fout,"ACCT_DET",2,acct);

	/*--------------------
	| Credit Value       |
	--------------------*/
 	sprintf (value_str,"%12.2f", (value > 0.00) ? value : value * -1);

	pr_format (fin,fout,"ACCT_DET",3, (value > 0.00) 
					? "            " : value_str);

	pr_format (fin,fout,"ACCT_DET",4, (value > 0.00) 
					? value_str : "            ");
	return (EXIT_SUCCESS);
}

/*=================================
| Write GLWK Total Records.       |
=================================*/
int
WriteGlTrans (
 char *	acct,
 double	value,
 char *	type)
{
	int	month;

	if (value == 0.00)
		return (EXIT_SUCCESS);

	/*-------------------------------------------------
	| Post control total debits & credits .           |
	-------------------------------------------------*/
	strcpy (glwkRec.jnl_type, (value > 0.00) ? "2" : "1");
	glwkRec.amount = (value > 0.00) 
					? CENTS (value) : CENTS (value * -1);

	glwkRec.loc_amount = glwkRec.amount;
					
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,acct);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	/*-------------------------------------------
	| Add transaction for account .             |
	-------------------------------------------*/
	sprintf (glwkRec.acc_no,"%-*.*s", MAXLEVEL, MAXLEVEL, acct);
	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.est_no,comm_rec.est_no);
	strcpy (glwkRec.acronym,"         ");
	strcpy (glwkRec.name,"                              ");
	sprintf (glwkRec.chq_inv_no, "%-15.15s", " ");
	glwkRec.ci_amt 		= 0;
	glwkRec.o1_amt 		= CENTS ( (double) local_rec.old_rate);
	glwkRec.o2_amt 		= CENTS ( (double) local_rec.new_rate);
	glwkRec.exch_rate	= 1.0;
	glwkRec.o3_amt 		= 0;
	glwkRec.o4_amt 		= 0;
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.tran_type,"15");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.tran_date = comm_rec.crd_date;
	strcpy (glwkRec.currency, local_rec.loc_curr);
	
	DateToDMY (glwkRec.tran_date, NULL, &month, NULL);
	sprintf (glwkRec.period_no, "%02d", month);

	glwkRec.post_date = TodaysDate ();
	strcpy (glwkRec.narrative,"inv/cn exch rate adj");
	strcpy (glwkRec.user_ref,"Exch. Rate Adj.");
	strcpy (glwkRec.alt_desc1," ");
	strcpy (glwkRec.alt_desc2," ");
	strcpy (glwkRec.alt_desc3," ");
	strcpy (glwkRec.batch_no, " ");
	strcpy (glwkRec.stat_flag,"2");

	GL_AddBatch ();
	return (EXIT_SUCCESS);
}

/*====================================
| Update GLBC Batch Control Records. |
====================================*/
void
UpdateWfbc (
 void)
{
	/*---------------------------------------------
	| Add general ledger batch control record.    |
	---------------------------------------------*/
	strcpy (wfbc_rec.co_no, comm_rec.co_no);
	wfbc_rec.pid_no = pidNumber;
	sprintf (wfbc_rec.work_file,"gl_work%05d",pidNumber);
	cc = find_rec (wfbc, &wfbc_rec, COMPARISON , "u");
    if (cc)
	{
		strcpy (wfbc_rec.system, "CR");
		wfbc_rec.date_create = StringToDate (local_rec.systemDate);
		strcpy (wfbc_rec.stat_flag, "1");
		wfbc_rec.batch_tot_1 = CENTS (exchVarianceTotal);
		cc = abc_add (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, wfbc, "DBADD");
	}
	else
	{
		wfbc_rec.batch_tot_1 += CENTS (exchVarianceTotal);
		cc = abc_update (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, wfbc, "DBUPDATE");
	}
	abc_unlock (wfbc);
}
		
/*========================
| Page Count Check.      |
========================*/
int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/
	if ( (fin = pr_open ("cr_icexch.p")) == NULL)
		sys_err ("Error in opening cr_icexch.p during (FOPEN)", errno,PNAME);

	if ( (fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);


	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".L131\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	pr_format (fin,fout,"BLANK",0,0);
	pr_format (fin,fout,"HEAD1",1,pocrRec.code);
	pr_format (fin,fout,"BLANK",0,0);
	pr_format (fin,fout,"LINE1",0,0);
	pr_format (fin,fout,"DET_HEAD1",0,0);
	pr_format (fin,fout,"DET_HEAD2",1,pocrRec.code);
	pr_format (fin,fout,"DET_HEAD2",2,local_rec.loc_curr);
	pr_format (fin,fout,"LINE1",0,0);
	pr_format (fin,fout,"RULER",0,0);

	fflush (fout);
}

/*===============================================
| Screen Heading Display Routine.               |
===============================================*/
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	fflush (stdout);
	rv_pr (ML (mlCrMess035),16,0,1);

	fflush (stdout);
	move (0,1);
	line (80);

	move (1,input_row);
	box (0,3,80,10);
	move (1,8);
	line (79);
	move (1,11);
	line (79);

	move (1,20);
	line (78);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}


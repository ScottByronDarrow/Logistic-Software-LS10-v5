/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_journlmc.c,v 5.2 2002/07/24 08:38:45 scott Exp $
|  Program Name  : (cr_journlmc.c) 
|  Program Desc  : (Input Suppliers Journals) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/06/87         |
|---------------------------------------------------------------------|
| $Log: cr_journlmc.c,v $
| Revision 5.2  2002/07/24 08:38:45  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.1  2002/07/16 06:03:41  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_journlmc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_journlmc/cr_journlmc.c,v 5.2 2002/07/24 08:38:45 scott Exp $";

#define		MCURR		 (multiCurrency [0] == 'Y')
#define 	MAXWIDTH	150 
#define 	MAXLINES	800 
#define 	D_VALUE(a)	 (store  [a].invDebitCreditValues)
#define 	G_VALUE(a)	 (store2 [a].glDebitCreditValues)

#define		SCN_HEAD	1
#define		SCN_INV		2
#define		SCN_GL		3

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_cr_mess.h>

#include	"schema"

	/*
	 * Special fields and flags.
	 */
   	int		journalProof	= TRUE,
			envCrCo			= 0,
			envCrFind		= 0,
			printerNumber	= 1,
			workFileNumber 	= 0,
			pidNumber		= 0,
			auditFileOpened = FALSE,
			auditPrinted	= FALSE,
			dataUpdated		= FALSE;

	struct 	storeRec {
		long	hhsuHash;
		long	hhsiHash;
		char	acronym [10];
		double	invDebitCreditValues;
		double	defaultExchangeRate;
	} store [MAXLINES];


	struct 	store2Rec {
		double	glDebitCreditValues;
	} store2 [MAXLINES];

	char	controlAccount [MAXLEVEL + 1];
	char	multiCurrency [2];
	char	branchNumber [3];

	/*
	 * Define file open for pformat 
	 */
	FILE *	fout;

	struct	commRecord 	comm_rec;
	struct	comrRecord 	comr_rec;
	struct	esmrRecord 	esmr_rec;
	struct	sumrRecord 	sumr_rec,
						sumr2_rec;
	struct	suinRecord 	suin_rec,
						suin2_rec;
	struct	suhdRecord 	suhd_rec;
	struct	sudtRecord 	sudt_rec;

	struct {
		long	wkHash;
	} wkRec;

/*
 * File names
 */
static char *data	= "data",
			*suin2  = "suin2",
			*sumr2  = "sumr2";

/*
 * Local & Screen Structures 
 */
	char	*scn_desc [] = {
		"HEADER SCREEN.",
		"SUPPLIER SCREEN.",
		"GENERAL LEDGER SCREEN."
	};

struct {
	char	dummy [11];
	char	systemDate [11];
	char	cur_date [11];
	char	br_no 		[sizeof esmr_rec.est_no];
	char	journalNo 	[sizeof glwkRec.user_ref];
	char	app_no 		[sizeof suin_rec.inv_no];
	char	cr_narrative [21];
	char	gl_narrative [sizeof glwkRec.narrative];
	char	dc1_flag [2];
	char	dc2_flag [2];
	char	supplierNo [sizeof sumr_rec.crd_no];
	char	supplierName [sizeof sumr_rec.crd_name];
	char	ca_desc [26];
	char	d_accdesc [26];
	char	gl_period [3];
	char	journ_period [3];
	char	gl_acc_no [MAXLEVEL + 1];
	char	gl_user_ref [9];
	double	gl_loc_amt;
	double	orig_cr_amt;
	double	exch_rate;
	double	dflt_ex_rate;
	double	loc_cr_amt;
	long	doj;
	long	date_post;
	char	curr_code [4];
	char	loc_curr [4];
	char	gl_loc_curr [4];
} local_rec;

static	struct	var	vars [] =
{
	{SCN_HEAD, LIN, "br_no",	 4, 20, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no, "Branch No.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{SCN_HEAD, LIN, "journalNo",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Journal No.", "Journal Number must not be on file.",
		YES, NO,  JUSTLEFT, "", "", local_rec.journalNo},
	{SCN_HEAD, LIN, "jnl_doi",	 7, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.cur_date, "Date of Journal", " ",
		YES, YES, JUSTRIGHT, "", "", (char *)&local_rec.doj},
	{SCN_HEAD, LIN, "date_post",	 8, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Date Posted", " ",
		YES, YES, JUSTRIGHT, "", "", (char *)&local_rec.date_post},
	{SCN_HEAD, LIN, "cr_narrative",	 9, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.cr_narrative, "Journal Narrative", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.cr_narrative},
	{SCN_INV, TAB, "supplierNo",	MAXLINES, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.supplierNo},
	{SCN_INV, TAB, "supplierName",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "      S u p p l i e r s   N a m e       ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierName},
	{SCN_INV, TAB, "dc1",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "", "D/C", "Must Be D(ebit) or C(redit). ",
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc1_flag},
	{SCN_INV, TAB, "orig_amt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", " Origin Amount. ", " ",
		YES, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.orig_cr_amt},
	{SCN_INV, TAB, "app_to",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "  Applied to   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.app_no},
	{SCN_INV, TAB, "curr_code",	 0, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Curr.", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{SCN_INV, TAB, "ex_rate",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "Exchange Rate", "<retn> defaults to inv/cn rate (if nominated)",
		 YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.exch_rate},
	{SCN_INV, TAB, "loc_amt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "0", "  Local Amount  ", " ",
		 NI, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.loc_cr_amt},
	{SCN_GL, TAB, "glacct",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", " ", GlDesc, "Enter account or [SEARCH] Key. ",
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.gl_acc_no},
	{SCN_GL, TAB, "gl_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.cr_narrative, "  Account Description    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ca_desc},
	{SCN_GL, TAB, "dc2",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "", "D/C", "Must Be D(ebit) or C(redit). ",
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc2_flag},
	{SCN_GL, TAB, "gl_loc_amt",	 0, 0, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Local Currency", " ",
		YES, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.gl_loc_amt},
	{SCN_GL, TAB, "loc_curr",	 0, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Curr", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.gl_loc_curr},
	{SCN_GL, TAB, "gl_narrative",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.cr_narrative, "    Narrative       ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.gl_narrative},
	{SCN_GL, TAB, "gl_user_ref",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", local_rec.journalNo, "User Reference ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.gl_user_ref},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>

/*
 * Local function prototypes 
 */
void	shutdown_prog		 (void);
void	OpenDB				 (void);
void	CloseDB				 (void);
void	ReadComr			 (void);
int		FindGlmr			 (char *);
int		spec_valid			 (int);
int		ProofTrans			 (void);
int		Update				 (void);
void	AddSuhd				 (long, long);
void	AddJournal			 (long);
void	SrchEsmr			 (char *);
void	SrchSuin			 (char *, int);
void	PrintCopyLine		 (void);
void	CalcJournalTotal	 (void);
void	PrintJournalTotal	 (void);
int		OpenAudit			 (void);
void	CloseAudit			 (void);
int		CheckJournal		 (char *);
int		heading				 (int);
int		CheckClass 			 (char *);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	if (argc < 3 || (pidNumber = atoi (argv [1])) < 1 || (printerNumber = atoi (argv [2])) < 1)
	{
		print_at (0, 0, mlCrMess015, argv [0]);
		return (EXIT_FAILURE);
	}

	envCrFind 	= atoi (get_env ("CR_FIND"));
	envCrCo 	= atoi (get_env ("CR_CO"));

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);	/*  setup vars for scrgen	    	|*/

	set_tty ();         
	set_masks ();		
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_INV, 	store,  sizeof (struct storeRec));
	SetSortArray (SCN_GL, 	store2, sizeof (struct store2Rec));
#endif
	init_scr ();		
	init_vars (SCN_HEAD);	
	init_vars (SCN_INV);	
	init_vars (SCN_GL);	

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	
	swide ();

	if (!MCURR) 
	{
		FLD ("loc_amt")		= NO;
		FLD ("ex_rate")		= ND;
		FLD ("orig_amt")	= ND;
	}

	OpenDB ();
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");
	GL_SetMask (GlFormat);

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	while (prog_exit == 0)
	{
		abc_unlock (sumr);
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		journalProof = TRUE;
		init_vars (SCN_HEAD);
		init_vars (SCN_INV);
		init_vars (SCN_GL);
		lcount [SCN_INV]	= 0;
		lcount [SCN_GL]		= 0;

		for (i = 0; i < MAXLINES; i++)
		{
			memset (store  + i, 0, sizeof (struct storeRec));
			memset (store2 + i, 0, sizeof (struct store2Rec));
		}

		/*
		 * Enter Screen 1 Linear Input 
		 */
		heading (SCN_HEAD);
		entry (SCN_HEAD);
		if (prog_exit || restart) 
			continue;

		/*
		 * Enter Screen SCN_INV Tabular Input 
		 */
		heading (SCN_INV);
		entry (SCN_INV);
		if (restart) 
			continue;

		/*
		 * Enter Screen SCN_GL Tabular Input 
		 */
		heading (SCN_GL);
		PrintJournalTotal ();
		entry (SCN_GL);
		if (restart) 
			continue;

		/*
		 * re-edit tabular if proof total incorrect. 
		 */
		while (journalProof == TRUE)
		{
	    	PrintJournalTotal ();
			edit_all ();
			if (restart) 
				break;

			ProofTrans ();
		}

		if (restart) 
			continue;

		/*
		 * Create entry on transaction file. 
		 */
		if (Update () == -1)
		{
			return (EXIT_FAILURE);
		}
	}

	if (dataUpdated)
	{
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	shutdown_prog ();
	return (EXIT_FAILURE);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	if (auditPrinted == TRUE)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files. 
 */
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename, "%s/WORK/cr_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);
	cc = RF_OPEN (filename, sizeof (wkRec), "w", &workFileNumber);
	if (cc) 
		file_err (cc, "wkRec", "WKOPEN");

	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	ReadComr ();

	abc_alias (suin2, suin);
	abc_alias (sumr2, sumr);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (envCrFind) ? "sumr_id_no3" 
							                               : "sumr_id_no");

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_id_no");
	OpenGlmr ();
	OpenPocr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close Data Base Files . 
 */
void
CloseDB (void)
{
	abc_fclose (esmr);
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_fclose (sudt);
	abc_fclose (sumr2);
	abc_fclose (suin2);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	abc_dbclose (data);

	cc = RF_CLOSE (workFileNumber);
	if (cc) 
		file_err (cc, "db_per", "WKCLOSE");
}

void
ReadComr (void)
{
	int		month;

	DateToDMY (comm_rec.crd_date, NULL, &month, NULL);
	strcpy (local_rec.cur_date, DateToString (comm_rec.crd_date));
	sprintf (local_rec.gl_period,"%02d", month);
	
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
    cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

/*
 * Read gl account master.    
 */
int
FindGlmr (
	char	*account)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	GL_FormAccNo (account, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [0][0] != 'F' ||
		glmrRec.glmr_class [2][0] != 'P')
	      return (2);
	
	return (EXIT_SUCCESS);
}

/*
 * Special Validation Routine.
 */
int
spec_valid (
 int field)
{
	if (cur_screen == SCN_INV || cur_screen == SCN_GL)
		PrintJournalTotal ();

	/*
	 * Validate Branch Number Input. 
	 */
	if (LCHECK ("br_no"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess073));
			return (EXIT_FAILURE); 
		}

		PrintCopyLine ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Journal Number Input.  
	 */
	if (LCHECK ("journalNo"))
	{
		if (CheckJournal (local_rec.journalNo))
		{
			errmess (ML (mlCrMess112));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Journal Date.  
	 */
	if (LCHECK ("jnl_doi"))
	{
		int		journalMonth;

		if (dflt_used)
			DSP_FLD ("jnl_doi");

		DateToDMY (local_rec.doj, NULL, &journalMonth, NULL);
		sprintf (local_rec.journ_period,"%02d", journalMonth);

		if (chq_date (local_rec.doj, comm_rec.crd_date))
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Supplier Posting Date. 
	 */
	if (LCHECK ("date_post"))
	{
		if (dflt_used)
			DSP_FLD ("date_post");

		if (local_rec.date_post > StringToDate (local_rec.systemDate))
		{
			errmess (ML (mlStdMess068));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Supplier Number Input. 
	 */
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNo));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
 		if (prog_status != ENTRY &&
 		    sumr_rec.hhsu_hash != store [line_cnt].hhsuHash)
 		{
 			store [line_cnt].hhsiHash = 0L;
 			strcpy (local_rec.app_no, "               ");
 			DSP_FLD ("app_to");
 		}

		strcpy (local_rec.supplierName, sumr_rec.crd_name);
		DSP_FLD ("supplierName");
		store [line_cnt].hhsuHash = sumr_rec.hhsu_hash;
		strcpy (store [line_cnt].acronym, sumr_rec.acronym);

		/*
		 * Read Supplier Currency Record. 
		 */
		store [line_cnt].defaultExchangeRate = 1.0000;
		strcpy (pocrRec.co_no, comm_rec.co_no);
		strcpy (pocrRec.code, sumr_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (!cc && pocrRec.ex1_factor != 0.0) 
	      		store [line_cnt].defaultExchangeRate = pocrRec.ex1_factor; 

		strcpy (local_rec.curr_code, sumr_rec.curr_code);
		DSP_FLD ("curr_code");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Flag input for Supplier.  
	 */
	if (LCHECK ("dc1"))
	{
		if (prog_status != ENTRY)
			CalcJournalTotal ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Overseas Original Amount.         
	 */
	if (LCHECK ("orig_amt"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		FLD ("loc_amt")	= (prog_status == ENTRY && dflt_used) ? YES : NI;

		if (prog_status != ENTRY)
		{
			local_rec.loc_cr_amt = no_dec (local_rec.orig_cr_amt / 
										   local_rec.exch_rate);
			DSP_FLD ("loc_amt");
			CalcJournalTotal ();
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Applied to Number Input. 
	 */
	if (LCHECK ("app_to"))
	{
		if (dflt_used)
		{
			store [line_cnt].hhsiHash = 0L;
			strcpy (local_rec.app_no, "               ");
			DSP_FLD ("app_to");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSuin (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}
		suin_rec.hhsu_hash = store [line_cnt].hhsuHash;
		sprintf (suin_rec.inv_no, "%-15.15s", local_rec.app_no);
		cc = find_rec (suin, &suin_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlCrMess113));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (suin_rec.approved [0] != 'Y')
		{
		    if (suin_rec.type [0] != '3')
		    {
				errmess (ML (mlCrMess114));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
		    }
		}
		store [line_cnt].hhsiHash = suin_rec.hhsi_hash;

		if (MCURR)
		{
			if (suin_rec.exch_rate > 0.0)
				local_rec.exch_rate =  suin_rec.exch_rate;
			else 
				local_rec.exch_rate = 1.0000;
	      	
			store [line_cnt].defaultExchangeRate = local_rec.exch_rate;
		
			local_rec.loc_cr_amt = no_dec (local_rec.orig_cr_amt / 
						       local_rec.exch_rate);
			DSP_FLD ("ex_rate");
			DSP_FLD ("loc_amt");
			CalcJournalTotal ();
			skip_entry = 2;
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Exchange Rate Input For Supplier. 
	 */
	if (LCHECK ("ex_rate"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (strcmp (local_rec.app_no, "               "))
		{
		   	local_rec.exch_rate = store [line_cnt].defaultExchangeRate;
			DSP_FLD ("ex_rate");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			local_rec.exch_rate = store [line_cnt].defaultExchangeRate;

		local_rec.loc_cr_amt = local_rec.orig_cr_amt / 
							   local_rec.exch_rate;

		local_rec.loc_cr_amt = no_dec (local_rec.loc_cr_amt);
		DSP_FLD ("ex_rate");
		DSP_FLD ("loc_amt");

		if (prog_status != ENTRY)
			CalcJournalTotal ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Amount input for Supplier. 
	 */
	if (LCHECK ("loc_amt"))
	{
		if (MCURR && strcmp (local_rec.app_no, "               "))
		{
			local_rec.loc_cr_amt = local_rec.orig_cr_amt / 
								   local_rec.exch_rate;
			local_rec.loc_cr_amt = no_dec (local_rec.loc_cr_amt);
		
 		}
		else
		{
			local_rec.orig_cr_amt = local_rec.loc_cr_amt *
									local_rec.exch_rate;
			local_rec.orig_cr_amt = no_dec (local_rec.orig_cr_amt);
		}

		DSP_FLD ("loc_amt");
		DSP_FLD ("orig_amt");
		CalcJournalTotal ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Amount input for General Ledger. 
	 */
	if (LCHECK ("gl_loc_amt") || LCHECK ("dc2"))
	{
		G_VALUE (line_cnt) = no_dec ( (local_rec.dc2_flag [0] == 'D') 
				? local_rec.gl_loc_amt : local_rec.gl_loc_amt * -1);
		PrintJournalTotal ();

		strcpy (local_rec.gl_loc_curr, comr_rec.base_curr);
		DSP_FLD ("loc_curr");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Narrative. 
	 */
	if (LCHECK ("gl_narrative"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.gl_narrative, local_rec.cr_narrative);
			DSP_FLD ("gl_narrative");
		}
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate General Ledger Account Input. 
	 */
	if (LCHECK ("glacct"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		if (dflt_used)
		{
			GL_GLI 
			(
				esmr_rec.co_no,
				esmr_rec.est_no,
				"  ",
				"DFT EXPEN.",
				"   ",
				" "
			);
			strcpy (local_rec.gl_acc_no,glmrRec.acc_no);
		}
		strcpy (glmrRec.co_no, comm_rec.co_no);

		GL_FormAccNo (local_rec.gl_acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (CheckClass (" "))
			return (EXIT_FAILURE);

		strcpy (local_rec.ca_desc, glmrRec.desc);
		DSP_FLD ("gl_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Validate Proof total. 
 */
int
ProofTrans (void)
{
	double	chk_total = 0.00;
	int	i;

	for (i = 0; i < MAXLINES; i++)
		chk_total += no_dec (D_VALUE (i) + G_VALUE (i));

	if (chk_total == 0.00)
		journalProof = FALSE;
	else
	{
		journalProof = TRUE;
		errmess (ML (mlCrMess115));
		sleep (sleepTime);
	}	
	return (EXIT_SUCCESS);
}
/*
 * Update & Print Journal
 */
int
Update (void)
{
	int	wk_line;

	dataUpdated = TRUE;

	clear ();
	print_at (0, 0, ML (mlCrMess116));
	fflush (stdout);


    /*
	 * Set switch if something is printed 
     */
	if (!auditFileOpened) 
	{ 
		if (OpenAudit () != 0)
		{
			shutdown_prog ();
			return (-1);
		}
	}
	auditFileOpened = TRUE;

	/*
	 * Print journal header details.      
	 */
	fprintf (fout, "|%s", local_rec.br_no);
	fprintf (fout, "|%s", local_rec.journalNo);
	fprintf (fout, "|%10.10s", DateToString (local_rec.doj));
	fprintf (fout, "|%10.10s", DateToString (local_rec.date_post));
	fprintf (fout, "|         |   |             |               |         |           |");
	fprintf (fout, "                |   |           |   |                    |\n");

	/*
	 * Set Screen to second page tabular. 
	 */
	scn_set (SCN_INV);

	abc_selfield (sumr, "sumr_hhsu_hash");

	for (wk_line = 0; wk_line < lcount [SCN_INV]; wk_line++)
	{
	   	/*
		 * Get current line for screen two. 
		 */
		getval (wk_line);
	
		fprintf (fout, "|  |        |          |          ");
		fprintf (fout, "|%s", store [wk_line].acronym);
		fprintf (fout, "| %s ", local_rec.dc1_flag);
	
		if (MCURR)
		{
			fprintf (fout, "|%13.2f", DOLLARS (local_rec.orig_cr_amt));
			fprintf (fout, "|%s", local_rec.app_no);
			fprintf (fout, "|%9.4f", local_rec.exch_rate);
			fprintf (fout, "|%11.2f", DOLLARS (local_rec.loc_cr_amt));
		}
		else
		{
			fprintf (fout, "|%13.2f", DOLLARS (local_rec.loc_cr_amt));
			fprintf (fout, "|%s|", local_rec.app_no);
			fprintf (fout, "|         |           ");
		}

	   	/*
		 * Add suin journal or suhd cheque. 
		 */
		if (!strcmp (local_rec.app_no, "               "))
			AddJournal (store [wk_line].hhsuHash);
		else
			AddSuhd (store [wk_line].hhsuHash, store [wk_line].hhsiHash);

		wkRec.wkHash = store [wk_line].hhsuHash;
		cc = RF_ADD (workFileNumber, (char *) &wkRec);
		if (cc) 
			file_err (cc, "WK_REC", "WKADD");
  
		/*
		 * Get control account for supplier.              
		 */
		sumr_rec.hhsu_hash	=	store [wk_line].hhsuHash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, sumr, "DBFIND");

		sprintf (controlAccount, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,sumr_rec.gl_ctrl_acct);
		cc = FindGlmr (controlAccount);
		if (cc)
		{
			/*
			 * Get currency control account.                  
			 */
			strcpy (pocrRec.co_no, sumr_rec.co_no);
			strcpy (pocrRec.code, sumr_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");

			sprintf (controlAccount, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,pocrRec.gl_ctrl_acct);
			cc = FindGlmr (controlAccount);
			if (cc)
			{
				GL_GLI 
				(
					esmr_rec.co_no,
					esmr_rec.est_no,
					"  ",
					"ACCT PAY  ",
					"   ",
					" "
				);

				strcpy (controlAccount, glmrRec.acc_no);
			}
		}
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;

		/*
		 * Print Supplier Control Transaction 
		 */
		fprintf (fout, "|%-16.16s|", controlAccount);
		fprintf (fout, " %s |", local_rec.dc1_flag);
		fprintf (fout, "%10.2f |", DOLLARS (local_rec.loc_cr_amt));
		fprintf (fout, "%s |", local_rec.journ_period);  /*local_rec.gl_period*/
		fprintf (fout, "%s|\n", local_rec.cr_narrative);

		/*
		 * Write glwk trans for supplier control acct.    
		 */
		sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,controlAccount);
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.tran_type, "17");
		glwkRec.post_date = local_rec.date_post;
		glwkRec.tran_date = local_rec.doj;  /*comm_rec.crd_date*/
		sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
		strcpy (glwkRec.user_ref, local_rec.gl_user_ref);
		strcpy (glwkRec.stat_flag, "2");
		strcpy (glwkRec.narrative, local_rec.cr_narrative);
		strcpy (glwkRec.period_no, local_rec.journ_period);

		if (local_rec.dc1_flag [0] == 'D')
			strcpy (glwkRec.jnl_type, "1");
		else
			strcpy (glwkRec.jnl_type, "2");

		glwkRec.exch_rate 	= 1.00;
		glwkRec.amount 		= local_rec.loc_cr_amt;
		glwkRec.loc_amount 	= local_rec.loc_cr_amt;
		strcpy (glwkRec.currency, comr_rec.base_curr);

		GL_AddBatch ();
	}
	/*
	 * Set Screen to third page tabular. 
	 */
	scn_set (SCN_GL);

	for (wk_line = 0; wk_line < lcount [SCN_GL]; wk_line++)
	{
		/*
		 * Get current line for screen three. 
		 */
		getval (wk_line);	

		/*
		 * Print glwk journal detail.    
		 */
		fprintf (fout, "|  |        |          |          |");
		fprintf (fout, "         |   |             |               |         |           |");
		fprintf (fout, "%-16.16s|", local_rec.gl_acc_no);
		fprintf (fout, " %s |", local_rec.dc2_flag);
		fprintf (fout, "%10.2f |", DOLLARS (local_rec.gl_loc_amt));
		fprintf (fout, "%s |", local_rec.journ_period);
		fprintf (fout, "%s|\n", local_rec.gl_narrative);

		/*
		 * Get gl acct hash & write glwk transaction.     
		 */
		cc = FindGlmr (local_rec.gl_acc_no);
		if (cc) 
		   	file_err (cc, glmr, "DBFIND");
		else
		{
			sprintf (glwkRec.acc_no, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,local_rec.gl_acc_no);
			glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		}
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.tran_type, "17");
		glwkRec.post_date = local_rec.date_post;
		glwkRec.tran_date = local_rec.doj;
		sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
		strcpy (glwkRec.user_ref, local_rec.gl_user_ref);
		strcpy (glwkRec.stat_flag, "2");

		strcpy (glwkRec.narrative, local_rec.gl_narrative);
		strcpy (glwkRec.period_no, local_rec.journ_period);
		glwkRec.amount 		= local_rec.gl_loc_amt;
		glwkRec.loc_amount 	= local_rec.gl_loc_amt;
		strcpy (glwkRec.currency, local_rec.gl_loc_curr);
		glwkRec.exch_rate = 1.00;

		if (local_rec.dc2_flag [0] == 'D')
			strcpy (glwkRec.jnl_type, "1");
		else
			strcpy (glwkRec.jnl_type, "2");

		GL_AddBatch ();
	} 
	abc_selfield (sumr, (envCrFind) ? "sumr_id_no3" : "sumr_id_no");
	return (EXIT_SUCCESS);
}

/*
 * Add Cheque/Journal header  & detail record. 
 */
void
AddSuhd (
	long	hhsuHash,
	long	hhsiHash)
{
	double	loc_amt,
			orig_amt;

	suhd_rec.hhsu_hash = hhsuHash;
	sprintf (suhd_rec.cheq_no,"%-12.12s", local_rec.journalNo);
	suhd_rec.hhsp_hash = 0L;
	strcpy (suhd_rec.narrative, local_rec.cr_narrative);
	suhd_rec.date_payment = local_rec.doj;
	suhd_rec.date_post = local_rec.date_post;
	if (MCURR)
	{
		if (local_rec.dc1_flag [0] == 'D')
		{
			orig_amt = local_rec.orig_cr_amt;
			loc_amt = local_rec.loc_cr_amt;
		}
		else
		{
			orig_amt = no_dec (local_rec.orig_cr_amt  * -1.0);
			loc_amt = no_dec (local_rec.loc_cr_amt  * -1.0);
		}
	}
	else
	{
		if (local_rec.dc1_flag [0] == 'D')
			loc_amt = local_rec.loc_cr_amt;
		else
			loc_amt = no_dec (local_rec.loc_cr_amt  * -1.0);

		orig_amt = loc_amt;
	}
	suhd_rec.loc_amt_paid = loc_amt;
	suhd_rec.tot_amt_paid = orig_amt;

	suhd_rec.disc_taken = 0.00;
	suhd_rec.loc_disc_take = 0.00;
	suhd_rec.exch_variance = 0.00;
	strcpy (suhd_rec.pay_type, "J");
	strcpy (suhd_rec.stat_flag, "0");

	cc = find_rec (suhd, &suhd_rec, COMPARISON, "r");
	if (cc)
	{
		cc = abc_add (suhd, &suhd_rec);
		if (!cc)
			cc = find_rec (suhd, &suhd_rec, COMPARISON, "r");
	}
	else
	{
		suhd_rec.loc_amt_paid += loc_amt;
		suhd_rec.tot_amt_paid += orig_amt;
		cc = abc_update (suhd, &suhd_rec);
	}
	if (cc)
		file_err (cc, suhd, "DBADD/DBUPDATE");

	/*
	 * Add Cheque/Journal detail record. 
	 */
	sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
	sudt_rec.hhsi_hash = hhsiHash;
	sudt_rec.amt_paid_inv = orig_amt;
	sudt_rec.loc_paid_inv = loc_amt;
	sudt_rec.exch_variatio = 0.00;
	if (MCURR)
		sudt_rec.exch_rate = local_rec.exch_rate;
	else
		sudt_rec.exch_rate = 1.0000;

	strcpy (sudt_rec.stat_flag, "0");

	cc = abc_add (sudt, &sudt_rec);
	if (cc)
		file_err (cc, sudt, "DBADD");

	/*
	 * Update Invoice Applied To.        
	 */
	suin_rec.hhsu_hash = hhsuHash;
	sprintf (suin_rec.inv_no, "%-15.15s", local_rec.app_no);
	cc = find_rec (suin, &suin_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, suin, "DBFIND");

	suin_rec.amt_paid += orig_amt;
	suin_rec.pay_amt = suin_rec.amt - suin_rec.amt_paid;
	cc = abc_update (suin, &suin_rec);
	if (cc)
		file_err (cc, suin, "DBUPDATE");
}

/*
 * Add Journal record to suin record. 
 */
void
AddJournal (
	long	hhsuHash)
{
	int	new_jnl = FALSE;
	double	wk_amt = 0.00;

	suin_rec.hhsu_hash = hhsuHash;
	sprintf (suin_rec.inv_no, "%-15.15s", local_rec.journalNo);
	new_jnl = find_rec (suin, &suin_rec, COMPARISON, "r");

	strcpy (suin_rec.type, "3");
	strcpy (suin_rec.est, local_rec.br_no);
	strcpy (suin_rec.narrative, local_rec.cr_narrative);
	suin_rec.date_of_inv = local_rec.doj;
	suin_rec.date_posted = local_rec.date_post;
	suin_rec.pay_date 	 = local_rec.date_post;
	
	if (MCURR)
	{
		wk_amt = (local_rec.dc1_flag [0] == 'C') ? 
					no_dec (local_rec.orig_cr_amt) : 
					no_dec (local_rec.orig_cr_amt * -1);
	}
	else
	{
		wk_amt = (local_rec.dc1_flag [0] == 'C') ? 
					no_dec (local_rec.loc_cr_amt) : 
					no_dec (local_rec.loc_cr_amt * -1);
	}
	if (new_jnl)
		suin_rec.amt = wk_amt;
	else
		suin_rec.amt += wk_amt;

	if (MCURR)
		suin_rec.exch_rate = local_rec.exch_rate;
	else
		suin_rec.exch_rate = 1.0000;
	

	suin_rec.pay_amt = suin_rec.amt;

	suin_rec.amt_paid = 0.00;
	strcpy (suin_rec.hold_reason, "   ");
	strcpy (suin_rec.cus_po_no, "      ");
	strcpy (suin_rec.currency, sumr_rec.curr_code);
	strcpy (suin_rec.er_fixed, "N");
	strcpy (suin_rec.destin, "                    ");
	strcpy (suin_rec.approved, "Y");
	strcpy (suin_rec.stat_flag, "0");
	
	if (new_jnl)
	{
		cc = abc_add (suin, &suin_rec);
		if (cc)
			file_err (cc, suin, "DBADD");
	}
	else
	{
		cc = abc_update (suin, &suin_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");
	}
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");
}

/*
 * Search routine for Establishment Master File. 
 */
void
SrchEsmr (
	char *	key_val)
{
	work_open ();
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, key_val);
	save_rec ("#Br", "#Br Name");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strncmp (esmr_rec.est_no, key_val, strlen (key_val)) && 
		      !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}

/*
 * Search routine for Invoice Number 
 */
void
SrchSuin (
	char	*key_val,
	int		in_line)
{
	_work_open (16,0,40);
	suin_rec.hhsu_hash = store [in_line].hhsuHash;
	strcpy (suin_rec.inv_no, key_val);
	save_rec ("#Invoice No", "#Supplier Name");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val, strlen (key_val)) && 
		       suin_rec.hhsu_hash == store [in_line].hhsuHash)
	{
		cc = save_rec (suin_rec.inv_no, sumr_rec.crd_name);
		if (cc)
			break;
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	suin_rec.hhsu_hash = store [in_line].hhsuHash;
	strcpy (suin_rec.inv_no, temp_str);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");
}

/*
 * Print Company/Branch/Department. 
 */
void
PrintCopyLine (void)
{
	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,  err_str,
					comm_rec.co_no, clip (comm_rec.co_short));

	strcpy (err_str, ML (mlStdMess039));
	print_at (21,45, err_str,
					comm_rec.est_no, clip (comm_rec.est_name));

	if (MCURR)
		line_at (22,0,130);
	else
		line_at (22,0,80);
}

/*
 * Calculate Journal proof total. 
 */
void
CalcJournalTotal (void)
{
	D_VALUE (line_cnt) = no_dec ( (local_rec.dc1_flag [0] == 'D') 
			? local_rec.loc_cr_amt : local_rec.loc_cr_amt * -1);
	PrintJournalTotal ();
}

/*
 * Print Journal proof total. 
 */
void
PrintJournalTotal (void)
{
	int	i;
	double	total = 0.00;
	
	for (i = 0; i < MAXLINES; i++)
		total += no_dec (D_VALUE (i) + G_VALUE (i));

	move (1, 19);
	cl_line ();
	print_at (19,1, ML (mlCrMess117) ,DOLLARS (total));
}

/*
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.          
 */
int
OpenAudit (void)
{
	auditPrinted = TRUE;

	if ( (fout = popen ("pformat", "w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".13\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".ESUPPLIERS JOURNALS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s as at %24.24s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R");
	fprintf (fout, "===================================");
	fprintf (fout, "==================================================================");
	fprintf (fout, "==========================================================\n");

	fprintf (fout, "===================================");
	fprintf (fout, "==================================================================");
	fprintf (fout, "==========================================================\n");

	fprintf (fout, "|  J O U R N A L  D E T A I L S   |");
	fprintf (fout, "            S U P P L I E R S     D E T A I L S                  |");
	fprintf (fout, "  G E N E R A L    L E D G E R    D E T A I L S          |\n");

	fprintf (fout, "|---------------------------------|");
	fprintf (fout, "-----------------------------------------------------------------|");
	fprintf (fout, "---------------------------------------------------------|\n");

	if (MCURR)
	{
		fprintf (fout, "|BR");
		fprintf (fout, "|JOURNAL ");
		fprintf (fout, "| DATE  OF ");
		fprintf (fout, "|   DATE   ");
		fprintf (fout, "| SUPPIER ");
		fprintf (fout, "|D/C");
		fprintf (fout, "|    AMOUNT   ");
		fprintf (fout, "|   APPLIED TO  ");
		fprintf (fout, "| EXCHANGE");
		fprintf (fout, "| EQUIV.LOC.");
		fprintf (fout, "|     ACCOUNT    ");
		fprintf (fout, "|D/C");
		fprintf (fout, "|G/L  AMOUNT");
		fprintf (fout, "|PER");
		fprintf (fout, "|   G/L  NARRATIVE   |\n");
	
		fprintf (fout, "|NO");
		fprintf (fout, "|NUMBER  ");
		fprintf (fout, "| JOURNAL  ");
		fprintf (fout, "|  POSTED  ");
		fprintf (fout, "| ACRONYM ");
		fprintf (fout, "|   ");
		fprintf (fout, "|             ");
		fprintf (fout, "|               ");
		fprintf (fout, "|   RATE  ");
		fprintf (fout, "|   AMOUNT  ");
		fprintf (fout, "|     NUMBER     ");
		fprintf (fout, "|   ");
		fprintf (fout, "|           ");
		fprintf (fout, "|NO.");
		fprintf (fout, "|                    |\n");
	}
	else
	{
		fprintf (fout, "|BR");
		fprintf (fout, "|JOURNAL ");
		fprintf (fout, "| DATE  OF ");
		fprintf (fout, "|   DATE   ");
		fprintf (fout, "| SUPPIER ");
		fprintf (fout, "|D/C");
		fprintf (fout, "|    AMOUNT   ");
		fprintf (fout, "|   APPLIED TO  ");
		fprintf (fout, "|         ");
		fprintf (fout, "|           ");
		fprintf (fout, "|    ACCOUNT     ");
		fprintf (fout, "|D/C");
		fprintf (fout, "|G/L  AMOUNT");
		fprintf (fout, "|PER");
		fprintf (fout, "|   G/L  NARRATIVE   |\n");
	
		fprintf (fout, "|NO");
		fprintf (fout, "|NUMBER  ");
		fprintf (fout, "| JOURNAL  ");
		fprintf (fout, "|  POSTED  ");
		fprintf (fout, "| ACRONYM ");
		fprintf (fout, "|   ");
		fprintf (fout, "|             ");
		fprintf (fout, "|               ");
		fprintf (fout, "|         ");
		fprintf (fout, "|           ");
		fprintf (fout, "|     NUMBER     ");
		fprintf (fout, "|   ");
		fprintf (fout, "|           ");
		fprintf (fout, "|NO.");
		fprintf (fout, "|                    |\n");
	}

	fprintf (fout, "|--");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---");
	fprintf (fout, "|-------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|---");
	fprintf (fout, "|-----------");
	fprintf (fout, "|---");
	fprintf (fout, "|--------------------|\n");

	fprintf (fout, ".PI12\n");
	return (EXIT_SUCCESS);
}

/*
 * Routine to close the audit trail output file. 
 */
void
CloseAudit (void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*
 * Check if journal number not already used in current company. 
 */
int
CheckJournal (
	char	*journalNo)
{
	char	wkjournalNo [16];

	open_rec (suin2, suin_list, SUIN_NO_FIELDS, "suin_inv_no");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

	sprintf (wkjournalNo, "%-15.15s", journalNo);
	sprintf (suin2_rec.inv_no, "%-15.15s", journalNo);

	cc = find_rec (suin2, &suin2_rec, GTEQ, "r");
	while (!cc && !strcmp (suin2_rec.inv_no, wkjournalNo))
	{
		sumr2_rec.hhsu_hash = suin2_rec.hhsu_hash;
		cc = find_hash (sumr2, &sumr2_rec, EQUAL, "r", suin2_rec.hhsu_hash);
		if (cc)
		{
			abc_fclose (sumr2);
			abc_fclose (suin2);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (sumr2_rec.co_no, comm_rec.co_no))
		{
			abc_fclose (sumr2);
			abc_fclose (suin2);
			return (EXIT_FAILURE);
		}
		cc = find_rec (suin2, &suin2_rec, NEXT, "r");
	}
	abc_fclose (sumr2);
	abc_fclose (suin2);
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
	PrintJournalTotal ();
	rv_pr (ML (mlCrMess118), 40, 0, 1);
	if (MCURR)
		line_at (1,0,130);
	else
		line_at (1,0,80);

	if (scn == 1)
	{
		if (MCURR)
		{
			box (0, 3, 130, 6);
			line_at (6,1,128);
		}
		else
		{
			box (0, 3, 80, 6);
			line_at (6,1,78);
		}
	}

	if (MCURR)
		line_at (20,0,130);
	else
		line_at (20,0,80);
		
	PrintCopyLine ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	
	return (EXIT_SUCCESS);
}

int
CheckClass (
	char	*msg)
{
	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
		return (print_err (ML (mlStdMess025)));

	return (EXIT_SUCCESS);
}
	

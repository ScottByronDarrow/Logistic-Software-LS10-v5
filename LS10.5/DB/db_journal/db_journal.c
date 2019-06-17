/*====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_journal.c,v 5.8 2002/07/24 08:38:48 scott Exp $
|  Program Name  : (db_journal.c)
|  Program Desc  : (Input Customer Journals)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/07/92         |
|---------------------------------------------------------------------|
| $Log: db_journal.c,v $
| Revision 5.8  2002/07/24 08:38:48  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/16 06:18:21  scott
| Updated from service calls and general maintenance.
|
| Revision 5.6  2002/06/26 04:34:16  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/06/26 04:26:52  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/08/20 23:10:27  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 09:05:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:22:08  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:14  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_journal.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_journal/db_journal.c,v 5.8 2002/07/24 08:38:48 scott Exp $";

#define 	MAXWIDTH	150 
#define 	MAXLINES	500 
#define 	D_VALUE(a)	 (store  [a].invoiceValues)
#define 	G_VALUE(a)	 (store2 [a].glValues)
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<GlUtils.h>

#define		SCN_HEAD	1
#define		SCN_INV		2
#define		SCN_GL		3
	/*
	 * Special fields and flags  ##################################.
	 */
   	int		jnlProof 			= 1,
			envDbCo 			= 0,
			envDbFind 			= 0,
	   		printerNumber 		= 1,
			wk_no				= 0,
			processID			= 0,
			Audit_open 			= FALSE,
   			period 				= 0,
			envDbHoOnly			= FALSE,
			envDbMcurr 			= FALSE,
			envGlByClass 		= TRUE,
			envSaCommission 	= FALSE,
			dataUpdate	 		= FALSE,
			monthPeriod			= 0;

	struct	storeRec {
		long	hhhoHash;
		long	hhcuHash;
		long	hhciHash;
		char	creditPeriod	[4];
		char	acronym			[10];
		double	invoiceValues;
		double	dflt_exch;
	} store [MAXLINES];

	struct	store2Rec {
		double	glValues;
	} store2 [MAXLINES];
	
	char	ctrl_acct [MAXLEVEL + 1],
			branchNo [3];

	/*
	 * Define file open for pformat
	 */
	FILE	*fout;
	
#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct cudpRecord	cudp_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuinRecord	cuin_rec;
struct cuinRecord	cuin2_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct sachRecord	sach_rec;
struct saclRecord	sacl_rec;

	struct {
		long	hhcuHash;
	} wkRec;

	char	*data  	= 	"data",
			*cuin2 	= 	"cuin2",
			*cumr2 	= 	"cumr2";

	char	*scn_desc [] = {
			"HEADER SCREEN.",
			"CUSTOMER SCREEN.",
			"GENERAL LEDGER SCREEN."
		};

/*
 * Local & Screen Structures  
 */
struct {
	char	dummy 				[11];
	char	systemDate 			[11];
	char	currentDate 		[11];
	char	branchNo 			[3];
	char	departmentNo 		[3];
	char	journalNo 			[9];
	char	appliedNo 			[9];
	char	customerNarrative 	[21];
	char	glNarrative 		[21];
	char	dc1_flag 			[2];
	char	dc2_flag 			[2];
	char	customerNo 			[7];
	char	customerName 		[31];
	char	ca_desc 			[26];
	char	glPeriod 			[3];
	char	journalPeriod 		[3];
	char	glAccountNo 		[MAXLEVEL + 1];
	char	glUserRef 			[sizeof glwkRec.user_ref];
	char	glLocalCurr 		[4];
	double	localGlAmt;
	double	originAmount;
	double	exchRate;
	double	localAmount;
	long	dateJournal;
	long	datePosted;
} local_rec;


static	struct	var	vars [] =
{
	{SCN_HEAD, LIN, "branchNo",	 4, 20, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no, "Branch No.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.branchNo},
	{SCN_HEAD, LIN, "departmentNo",	 5, 20, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Department No.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.departmentNo},
	{SCN_HEAD, LIN, "journalNo",	 6, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Journal No.", "Journal Number must not be on file.",
		YES, NO,  JUSTLEFT, "", "", local_rec.journalNo},
	{SCN_HEAD, LIN, "jnl_doi",	 8, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.currentDate, "Date of Journal", " ",
		YES, YES, JUSTRIGHT, "", "", (char *)&local_rec.dateJournal},
	{SCN_HEAD, LIN, "datePosted",	 9, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Date Posted", " ",
		YES, YES, JUSTRIGHT, "", "", (char *)&local_rec.datePosted},
	{SCN_HEAD, LIN, "customerNarrative",	 10, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.customerNarrative, "Journal Narrative", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNarrative},

	{SCN_INV, TAB, "customerNo",	MAXLINES, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " Customer ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{SCN_INV, TAB, "customerName",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "    D e b t o r s    N a m e   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerName},
	{SCN_INV, TAB, "dc1",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "", " D/C ", "Must Be D (ebit) or C (redit). ",
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc1_flag},
	{SCN_INV, TAB, "orig_amt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", " Origin Amount. ", " ",
		YES, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.originAmount},
	{SCN_INV, TAB, "app_to",	 0, 3, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", " Applied to ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.appliedNo},
	{SCN_INV, TAB, "ex_rate",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "  Exch Rate  ", "<retn> defaults to inv/cn rate (if nominated)",
		 NA, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.exchRate},
	{SCN_INV, TAB, "loc_amt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "  Local Amount  ", " ",
		 NA, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.localAmount},

	{SCN_GL, TAB, "glacct",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "Enter account or [SEARCH] ",
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.glAccountNo},
	{SCN_GL, TAB, "gl_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.customerNarrative, "   Account Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ca_desc},
	{SCN_GL, TAB, "dc2",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "", "D/C", "Must Be D (ebit) or C (redit). ",
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc2_flag},
	{SCN_GL, TAB, "localGlAmt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", " Local Currency", " ",
		YES, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.localGlAmt},
	{SCN_GL, TAB, "loc_curr",	 0, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Curr.", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.glLocalCurr},
	{SCN_GL, TAB, "glNarrative",	 0, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.customerNarrative, "      Narrative         ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.glNarrative},
	{SCN_GL, TAB, "glUserRef",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", local_rec.journalNo, "User Reference ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.glUserRef},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
#include <twodec.h>
#include <FindCumr.h>

/*
 * Local Function Prototypes.
 */
int 	CheckJournal 		(char *);
int 	heading 			(int);
int 	OpenAudit 			(void);
int 	ReadGlAccount		(char *);
int 	spec_valid 			(int);
int 	Update 				(void);
static 	int CheckClass 		(char *);
void 	AddCommission 		(long, long, long, double);
void 	AddCuhd 			(long, long, long);
void 	AddJnl 				(long, long, char *);
void 	CalcJournalTotal 	(void);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	PrintCoLine 		(void);
void 	PrintJournalTotal 	(void);
void 	ProofTrans 			(void);
void 	ReadComr 			(void);
void 	shutdown_prog 		(void);
void 	SrchCudp 			(char *);
void 	SrchCuin 			(char *, int);
void 	SrchEsmr 			(char *);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	int		i;

	if (argc < 3 || (processID = atoi (argv [1])) < 1 || (printerNumber = atoi (argv [2])) < 1)
	{
		print_at (0,0,"Usage : %s <processID> <printerNumber>",argv [0]);
        return (EXIT_FAILURE);
	}

	sptr	= chk_env ("DB_FIND");
	envDbFind	= (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr	= chk_env ("DB_CO");
	envDbCo	= (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for multi-currency.
	 */
	sptr	= chk_env ("DB_MCURR");
	envDbMcurr	= (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * General ledger environment to choose by class of salesman.
	 */
	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Sales analysis commission.
	 */
	sptr = chk_env ("SA_COMMISSION");
	envSaCommission = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Customer head office rules.
	 */
	sptr = chk_env ("DB_HO_ONLY");
	envDbHoOnly = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);


	init_scr ();		
	set_tty ();      
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_INV, store,  sizeof (struct storeRec));
	SetSortArray (SCN_GL, store2, sizeof (struct store2Rec));
#endif
	init_vars (SCN_HEAD);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	swide ();

	if (!envDbMcurr)
	{
		FLD ("loc_amt")  = NO;
		FLD ("ex_rate")  = ND;
		FLD ("orig_amt") = ND;
	}

	OpenDB ();

	GL_SetMask (GlFormat);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");


	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	while (prog_exit == 0)
	{
		abc_unlock (cumr);

		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		jnlProof 	= TRUE;

		for (i = 0; i < MAXLINES; i++)
		{
			memset (store  + i, 0, sizeof (struct storeRec));
			memset (store2 + i, 0, sizeof (struct store2Rec));
		}

		init_vars (SCN_HEAD);
		init_vars (SCN_INV);
		init_vars (SCN_GL);
		lcount [SCN_INV] =	0;
		lcount [SCN_GL]	 =	0;

		/*
		 * Enter Screen SCN_HEAD Linear Input.
		 */
		heading (SCN_HEAD);
		entry (SCN_HEAD);
		if (prog_exit || restart) 
			continue;

		/*
		 * Enter Screen SCN_INV Tabular Input.
		 */
		heading (SCN_INV);
		entry (SCN_INV);
		if (restart) 
			continue;

		/*
		 * Enter Screen SCN_GL Tabular Input.
		 */
		heading (SCN_GL);
	   	PrintJournalTotal ();
		entry (SCN_GL);

		if (restart) 
			continue;

		/*
		 * re-edit tabular if proof total incorrect.
		 */
		while (jnlProof)
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
		if (Update () == 1)
        {
            return (EXIT_FAILURE);
        }
	}
	if (dataUpdate)
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

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	if (Audit_open)
		CloseAudit ();

	FinishProgram ();;
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/db_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);

	cc = RF_OPEN (filename,sizeof (wkRec),"w",&wk_no);
	if (cc) 
		file_err (cc, "wkRec", "WKOPEN");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	ReadComr ();

	abc_alias (cumr2, cumr);
	abc_alias (cuin2, cuin);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
							       							: "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_ho_id");
	open_rec (cuin2,cuin_list, CUIN_NO_FIELDS, "cuin_inv_no");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_id_no");

	OpenGlmr ();
	OpenPocr ();

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);

	if (envSaCommission)
	{
		open_rec (sach,  sach_list, SACH_NO_FIELDS, "sach_hhci_hash");
		open_rec (sacl,  sacl_list, SACL_NO_FIELDS, "sacl_id_no");
	}
}

/*
 * Close Data Base Files.
 */
void
CloseDB (void)
{
	abc_fclose (esmr);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuin2);
	abc_fclose (cumr2);
	abc_fclose (cuhd);
	abc_fclose (cudt);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	if (envSaCommission)
	{
		abc_fclose (sach);
		abc_fclose (sacl);
	}
	abc_dbclose (data);

	cc = RF_CLOSE (wk_no);
	if (cc) 
		file_err (cc, "db_per", "WKCLOSE");
}

void
ReadComr (void)
{
	strcpy (local_rec.currentDate, DateToString (comm_rec.dbt_date));

	DateToDMY (comm_rec.dbt_date, NULL, &monthPeriod, NULL);
	sprintf (local_rec.glPeriod, "%02d", monthPeriod);
	
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

/*
 * Read gl account master.  
 */
int
ReadGlAccount (
 char*              account)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
        return (EXIT_FAILURE);

	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
		return (2);
	
	return (EXIT_SUCCESS);
}

/*
 * Special Validation Routine.
 */
int
spec_valid (
 int                field)
{
	if (cur_screen == SCN_INV || cur_screen == SCN_GL)
		PrintJournalTotal ();

	/*
	 * Validate Branch Number Input.
	 */
	if (LCHECK ("branchNo"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,local_rec.branchNo);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess073));
			sleep (sleepTime);
        	return (EXIT_FAILURE);
		}

		PrintCoLine ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate department Number and allow search.
	 */
	if (LCHECK ("departmentNo"))
	{
		open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");

		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,local_rec.branchNo);
		strcpy (cudp_rec.dp_no, local_rec.departmentNo);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess084));
			sleep (sleepTime);
			abc_fclose (cudp);
        	return (EXIT_FAILURE);
		}

		abc_fclose (cudp);
		PrintCoLine ();
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Journal Number Input. 
	 */
	if (LCHECK ("journalNo"))
	{
		strcpy (local_rec.journalNo, (zero_pad (local_rec.journalNo,8)));
		if (CheckJournal (local_rec.journalNo))
		{
			sprintf (err_str, ML (mlDbMess218),local_rec.journalNo);
			errmess (err_str);
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
		if (dflt_used)
			DSP_FLD ("jnl_doi");

		DateToDMY (local_rec.dateJournal, NULL, &monthPeriod, NULL);
		sprintf (local_rec.journalPeriod, "%02d", monthPeriod);

		if (chq_date (local_rec.dateJournal,comm_rec.dbt_date))
        	return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Posting Date.
	 */
	if (LCHECK ("datePosted"))
	{
		if (dflt_used)
			DSP_FLD ("datePosted");

		if (local_rec.datePosted > StringToDate (local_rec.systemDate))
		{
			sprintf (err_str,ML (mlDbMess219),local_rec.systemDate);
			errmess (err_str);
			sleep (sleepTime);
        	return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Number Input.
	 */
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.customerNo));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
        	return (EXIT_FAILURE);
		}

		if (cumr_rec.ho_dbt_hash > 0L && envDbHoOnly)
		{
			sprintf (err_str, ML (mlDbMess207), local_rec.customerNo);
			errmess (err_str);
			sleep (sleepTime);
        	return (EXIT_FAILURE);
		}
 		if (prog_status != ENTRY &&
 		    cumr_rec.hhcu_hash != store [line_cnt].hhhoHash)
 		{
 			store [line_cnt].hhciHash = 0L;
 			strcpy (local_rec.appliedNo, "        ");
 			DSP_FLD ("app_to");
 		}

		sprintf (local_rec.customerName,"%-30.30s",cumr_rec.dbt_name);
		DSP_FLD ("customerName");
    	store [line_cnt].hhhoHash = (cumr_rec.ho_dbt_hash > 0L)
									? cumr_rec.ho_dbt_hash
									: cumr_rec.hhcu_hash;
    	store [line_cnt].hhcuHash =	cumr_rec.hhcu_hash;
		strcpy (store [line_cnt].acronym, cumr_rec.dbt_acronym);
		strcpy (store [line_cnt].creditPeriod, cumr_rec.crd_prd);

		/*
		 * Read Customer Currency Record.
		 */
	    store [line_cnt].dflt_exch = 1.0000;
		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code,cumr_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (!cc && pocrRec.ex1_factor != 0.0) 
	      		store [line_cnt].dflt_exch = pocrRec.ex1_factor; 

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Flag input for Customer. 
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
		if (!envDbMcurr)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY)
		{
			local_rec.localAmount = no_dec (local_rec.originAmount / 
						       local_rec.exchRate);
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
			store [line_cnt].hhciHash = 0L;
			strcpy (local_rec.appliedNo,"        ");
			DSP_FLD ("app_to");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCuin (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}
		cuin_rec.ho_hash = store [line_cnt].hhhoHash;
		strcpy (cuin_rec.inv_no, local_rec.appliedNo);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess115));
			sleep (sleepTime);
        	return (EXIT_FAILURE);
		}
		store [line_cnt].hhciHash = cuin_rec.hhci_hash;

		if (envDbMcurr)
		{
			if (cuin_rec.exch_rate > 0.0)
				local_rec.exchRate =  cuin_rec.exch_rate;
			else 
				local_rec.exchRate = 1.0000;
	      	
			store [line_cnt].dflt_exch = local_rec.exchRate;
		
			local_rec.localAmount = no_dec (local_rec.originAmount / 
						       local_rec.exchRate);
			DSP_FLD ("ex_rate");
			DSP_FLD ("loc_amt");
			CalcJournalTotal ();
			skip_entry = 2;
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Exchange Rate Input For Customer.
	 */
	if (LCHECK ("ex_rate"))
	{
		if (!envDbMcurr)
			return (EXIT_SUCCESS);

		if (strcmp (local_rec.appliedNo,"        "))
		{
		   	local_rec.exchRate = store [line_cnt].dflt_exch;
			DSP_FLD ("ex_rate");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			local_rec.exchRate = store [line_cnt].dflt_exch;

		local_rec.localAmount = local_rec.originAmount / 
				       local_rec.exchRate;

		local_rec.localAmount = no_dec (local_rec.localAmount);
		DSP_FLD ("ex_rate");
		DSP_FLD ("loc_amt");

		if (prog_status != ENTRY)
			CalcJournalTotal ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Amount input for Customer.
	 */
	if (LCHECK ("loc_amt"))
	{
		if (envDbMcurr && strcmp (local_rec.appliedNo,"        "))
		{
			local_rec.localAmount = local_rec.originAmount / local_rec.exchRate;
			local_rec.localAmount = no_dec (local_rec.localAmount);
			DSP_FLD ("loc_amt");
			return (EXIT_SUCCESS);
		}

		if (envDbMcurr && dflt_used)
		{
			local_rec.localAmount = local_rec.originAmount / local_rec.exchRate;
			local_rec.localAmount = no_dec (local_rec.localAmount);
			DSP_FLD ("loc_amt");
			CalcJournalTotal ();
			return (EXIT_SUCCESS);
		}

		if (envDbMcurr)
		{
			local_rec.exchRate = local_rec.originAmount / local_rec.localAmount;
			store [line_cnt].dflt_exch = local_rec.exchRate;
			DSP_FLD ("ex_rate");
		}

		DSP_FLD ("loc_amt");
		CalcJournalTotal ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Amount input for General Ledger.
	 */
	if (LCHECK ("localGlAmt") || LCHECK ("dc2"))
	{
		G_VALUE (line_cnt) = no_dec ((local_rec.dc2_flag [0] == 'D') 
				? local_rec.localGlAmt : local_rec.localGlAmt * -1);
		PrintJournalTotal ();
		strcpy (local_rec.glLocalCurr, comr_rec.base_curr);
		DSP_FLD ("loc_curr");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Narrative.
	 */
	if (LCHECK ("glNarrative"))
	{
	   	if (dflt_used)
		{
			strcpy (local_rec.glNarrative, local_rec.customerNarrative);
			DSP_FLD ("glNarrative");
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
			

		/*
		 * Default used so get default account for cash or charge.
		 */
		if (dflt_used)
		{
			GL_GLI 
			(
				comm_rec.co_no,
				local_rec.branchNo,
				"  ",
				"JOURNAL   ",
				(envGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
				" "
			);
			strcpy (local_rec.glAccountNo, glmrRec.acc_no);
		}
		strcpy (glmrRec.co_no,comm_rec.co_no);
		GL_FormAccNo (local_rec.glAccountNo, glmrRec.acc_no, 0);
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
void
ProofTrans (void)
{
	double	chk_total = 0.00;
	int		i;

	for (i = 0; i < MAXLINES; i++)
		chk_total += no_dec (D_VALUE (i) + G_VALUE (i));

	if (chk_total == 0.00)
		jnlProof = 0;
	else
	{
		jnlProof = 1;
		errmess (ML (mlDbMess222));
		sleep (sleepTime);
	}	
}
/*
 * Update & Print Journal
 */
int
Update (void)
{
	int		wk_line;

	dataUpdate = TRUE;

	clear ();
	print_at (0,0,ML (mlDbMess208)); 
	fflush (stdout);


	/*
	 * Set switch if something is printed
	 */
	if (!Audit_open) 
	{ 
		if (OpenAudit () != 0)
        {
			shutdown_prog ();
        	return (EXIT_FAILURE);
        }
	}

	Audit_open = TRUE;

	/*
	 * Print journal header details.     
	 */
	fprintf (fout, "|%s|%s", local_rec.branchNo, local_rec.departmentNo);
	fprintf (fout, "|%s", local_rec.journalNo);
	fprintf (fout, "|%-10.10s", DateToString (local_rec.dateJournal));
	fprintf (fout, "|%-10.10s", DateToString (local_rec.datePosted));
	fprintf (fout, "|      |         |   |             |        |         |           |");
	fprintf (fout,"                |   |           |   |                    |\n");

	/*
	 * Set Screen to second page tabular.
	 */
	scn_set (SCN_INV);

	abc_selfield ("cumr", "cumr_hhcu_hash");

	for (wk_line = 0; wk_line < lcount [SCN_INV]; wk_line++)
	{
	   	/*
	     * Get current line for screen two.
	     */
	    getval (wk_line);
	
	    fprintf (fout,"|  |  |        |          |          ");
	    fprintf (fout,"|%s",local_rec.customerNo);
	    fprintf (fout,"|%s",store [wk_line].acronym);
	    fprintf (fout,"| %s ",local_rec.dc1_flag);
	
        if (envDbMcurr)
	    {
	    	fprintf (fout,"|%13.2f",DOLLARS (local_rec.originAmount));
	    	fprintf (fout,"|%s",local_rec.appliedNo);
	    	fprintf (fout,"|%9.6f",local_rec.exchRate);
	    	fprintf (fout,"|%11.2f",DOLLARS (local_rec.localAmount));
	    }
	    else
	    {
	    	fprintf (fout,"|%13.2f",DOLLARS (local_rec.localAmount));
			fprintf (fout,"|%s|",local_rec.appliedNo);
			fprintf (fout,"|         |           ");
	    }

	   	/*
	     * Add cuin journal or cuhd cheque.
	     */
	    if (!strcmp (local_rec.appliedNo, "        "))
		{
            AddJnl 
			(
				store [wk_line].hhhoHash,
				store [wk_line].hhcuHash,
				store [wk_line].creditPeriod
			);
		}
	    else
		{
			AddCuhd
			(
				store [wk_line].hhhoHash,
				store [wk_line].hhcuHash,
				store [wk_line].hhciHash
			);
		}

	    wkRec.hhcuHash = store [wk_line].hhcuHash;
	    cc = RF_ADD (wk_no, (char *) &wkRec);
	    if (cc) 
			file_err (cc, "db_per", "WKADD");
  
		/*
		 * Get control account for creditor.
		 */
		cc = find_hash (cumr, &cumr_rec, COMPARISON, "r", store [wk_line].hhcuHash);
		if (!cc) 
		{
			sprintf (ctrl_acct, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cumr_rec.gl_ctrl_acct);

			if (ReadGlAccount (ctrl_acct))
			{
				GL_GLI 
				(
					comm_rec.co_no,
					local_rec.branchNo,
					"  ",
					"ACCT REC  ",
					(envGlByClass) ? cumr_rec.class_type
					 : cumr_rec.sman_code,
					" "
				);
				strcpy (ctrl_acct, glmrRec.acc_no);
				cc = ReadGlAccount (ctrl_acct);
				if (cc)
					file_err (cc, glmr, "DBFIND");
			}
			glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		}

		/*
		 * Print Creditor Control Transaction.    
		 */
		fprintf (fout,"|%s|", ctrl_acct);
	    fprintf (fout," %s |",local_rec.dc1_flag);
		fprintf (fout,"%10.2f |", DOLLARS (local_rec.localAmount));
		fprintf (fout,"%s |", local_rec.journalPeriod);
		fprintf (fout,"%s|\n", local_rec.customerNarrative);

		/*
		 * Write glwk trans for creditor control acct.
		 */
		strcpy (glwkRec.acc_no, ctrl_acct);
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.tran_type, "16");
		glwkRec.post_date = local_rec.datePosted;
		glwkRec.tran_date = local_rec.dateJournal;  /* comm_rec.dbt_date */
		sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
		strcpy (glwkRec.user_ref, local_rec.glUserRef);
		strcpy (glwkRec.stat_flag, "2");
		strcpy (glwkRec.narrative, local_rec.glNarrative);
		strcpy (glwkRec.alt_desc1, local_rec.customerNarrative);
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no, " ");
		strcpy (glwkRec.period_no, local_rec.journalPeriod); 

		strcpy (glwkRec.jnl_type,"1");

		if (local_rec.dc1_flag [0] == 'C')
			strcpy (glwkRec.jnl_type,"2");

		glwkRec.exch_rate	= 1.00;
		glwkRec.amount 		= local_rec.localAmount;
		glwkRec.loc_amount 	= local_rec.localAmount;
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
		fprintf (fout,"|  |  |        |          |          |");
		fprintf (fout,"      |         |   |             |        |         |           |");
		fprintf (fout,"%s|", local_rec.glAccountNo);
		fprintf (fout," %s |", local_rec.dc2_flag);
		fprintf (fout,"%10.2f |", DOLLARS (local_rec.localGlAmt));
		fprintf (fout,"%s |", local_rec.journalPeriod);
		fprintf (fout,"%s|\n", local_rec.glNarrative);

		/*
		 * Get gl acct hash & write glwk transaction.    
		 */
		cc = ReadGlAccount (local_rec.glAccountNo);
		if (cc) 
			file_err (cc, glmr, "DBFIND");
		else
		{
			strcpy (glwkRec.acc_no, local_rec.glAccountNo);
			glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		}
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.tran_type, "16");
		glwkRec.post_date = local_rec.datePosted;
		glwkRec.tran_date = local_rec.dateJournal;
		sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
		strcpy (glwkRec.user_ref, local_rec.glUserRef);
		strcpy (glwkRec.stat_flag, "2");

		strcpy (glwkRec.narrative, local_rec.glNarrative);
		strcpy (glwkRec.alt_desc1, local_rec.customerNarrative);
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no, " ");
		strcpy (glwkRec.period_no, local_rec.journalPeriod);
		glwkRec.exch_rate	= 1.00;
		glwkRec.amount 		= local_rec.localGlAmt;
		glwkRec.loc_amount 	= local_rec.localGlAmt;
		strcpy (glwkRec.currency, local_rec.glLocalCurr);

		if (local_rec.dc2_flag [0] == 'D')
			strcpy (glwkRec.jnl_type,"1");
		else
			strcpy (glwkRec.jnl_type,"2");

		GL_AddBatch ();
	} 
	abc_selfield ("cumr", (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	return (EXIT_SUCCESS);
}

/*
 * Add Cheque/Journal header  & detail record.
 */
void
AddCuhd (
	long	hhhoHash,
	long	hhcuHash,
	long	hhciHash)
{
	double	loc_amt,
			orig_amt;

	cuhd_rec.hhcu_hash = (envDbHoOnly && hhhoHash > 0L) ? hhhoHash : hhcuHash;
	strcpy (cuhd_rec.receipt_no, local_rec.journalNo);
	strcpy (cuhd_rec.narrative, local_rec.customerNarrative);
	cuhd_rec.hhcp_hash 		=	0L;
	cuhd_rec.date_payment 	=	local_rec.dateJournal;
	cuhd_rec.index_date		=	local_rec.dateJournal;
	cuhd_rec.date_posted 	=	local_rec.datePosted;
	if (envDbMcurr)
	{
		if (local_rec.dc1_flag [0] == 'C')
		{
			orig_amt	=	local_rec.originAmount;
			loc_amt		=	local_rec.localAmount;
		}
		else
		{
			orig_amt	=	no_dec (local_rec.originAmount  * -1.0);
			loc_amt		=	no_dec (local_rec.localAmount  * -1.0);
		}
	}
	else
	{
		if (local_rec.dc1_flag [0] == 'C')
			loc_amt = local_rec.localAmount;
		else
			loc_amt = no_dec (local_rec.localAmount  * -1.0);

		orig_amt = loc_amt;
	}
	cuhd_rec.loc_amt_paid	=	loc_amt;
	cuhd_rec.tot_amt_paid	=	orig_amt;

	cuhd_rec.disc_given		=	0.00;
	cuhd_rec.loc_disc_give	=	0.00;
	cuhd_rec.exch_variance		=	0.00;
	strcpy (cuhd_rec.type, "2");
	strcpy (cuhd_rec.stat_flag, "0");

	cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "u");
	if (cc)
	{
		cc = abc_add (cuhd, &cuhd_rec);
		if (!cc)
			cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "r");
	}
	else
	{
		cuhd_rec.loc_amt_paid += loc_amt;
		cuhd_rec.tot_amt_paid += orig_amt;
		cc = abc_update (cuhd, &cuhd_rec);
	}
	if (cc)
		file_err (cc, cuhd, "DBADD/DBUPDATE");

	/*
	 * Add Cheque/Journal detail record.
	 */
	cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;
	cudt_rec.hhci_hash = hhciHash;
	cudt_rec.amt_paid_inv = orig_amt;
	cudt_rec.loc_paid_inv = loc_amt;
	cudt_rec.exch_variatio = 0.00;
	if (envDbMcurr)
		cudt_rec.exch_rate = local_rec.exchRate;
	else
		cudt_rec.exch_rate = 1.0000;

	strcpy (cudt_rec.stat_flag, "0");

	cc = abc_add (cudt, &cudt_rec);
	if (cc)
		file_err (cc, cudt, "DBADD");

	/*
	 * Add Commission record for receipts made against an invoice.
	 */
	if (envSaCommission)
	{
		AddCommission (
           (envDbHoOnly && hhhoHash > 0L) ? hhhoHash : hhcuHash,
			hhciHash, 
			cuhd_rec.hhcp_hash,
			cudt_rec.loc_paid_inv);
	}
}

/*
 * Add Journal record to cuin record.
 */
void
AddJnl (
	long	hhhoHash,
	long	hhcuHash,
	char	*pay_terms)
{
	int		new_jnl = FALSE;
	double	wk_amt = 0.00;

	cuin_rec.amt = 0.00;

	cuin_rec.hhcu_hash = hhcuHash;
	cuin_rec.ho_hash   = hhhoHash;
	strcpy (cuin_rec.inv_no,local_rec.journalNo);
	new_jnl = find_rec (cuin, &cuin_rec, COMPARISON, "u");

	strcpy (cuin_rec.type, "3");
	strcpy (cuin_rec.co_no, comm_rec.co_no);
	strcpy (cuin_rec.est, local_rec.branchNo);
	strcpy (cuin_rec.dp,  local_rec.departmentNo);
	strcpy (cuin_rec.narrative, local_rec.customerNarrative);
	sprintf (cuin_rec.pay_terms, "%-3.3s", "0  ");
	cuin_rec.date_of_inv 	= local_rec.dateJournal;
	cuin_rec.date_posted 	= local_rec.datePosted;
	cuin_rec.due_date 	= local_rec.dateJournal;
	
	if (envDbMcurr)
	{
		wk_amt = (local_rec.dc1_flag [0] == 'C') ? 
					no_dec (local_rec.originAmount * -1) : 
					no_dec (local_rec.originAmount);
	}
	else
	{
		wk_amt = (local_rec.dc1_flag [0] == 'C') ? 
					no_dec (local_rec.localAmount * -1) : 
					no_dec (local_rec.localAmount);
	}
	if (new_jnl)
		cuin_rec.amt = wk_amt;
	else
		cuin_rec.amt += wk_amt;

	if (envDbMcurr)
		cuin_rec.exch_rate = local_rec.exchRate;
	else
		cuin_rec.exch_rate = 1.0000;

	strcpy (cuin_rec.currency, cumr_rec.curr_code);
	strcpy (cuin_rec.er_fixed, "N");
	strcpy (cuin_rec.stat_flag, "0");
	if (new_jnl)
	{
		cc = abc_add (cuin, &cuin_rec);
		if (cc)
			file_err (cc, cuin, "DBADD");
	}
	else
	{
		cc = abc_update (cuin, &cuin_rec);
		if (cc)
			file_err (cc, cuin, "DBUPDATE");
	}
	cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");
}

/*
 * Search routine for Establishment Master File. 
 */
void
SrchEsmr (
	char	*key_val)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,key_val);
	save_rec ("#No", "#Branch description.");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strncmp (esmr_rec.est_no, key_val,strlen (key_val)) && 
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
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*
 * Search routine for Invoice Number
 */
void
SrchCuin (
	char	*key_val,
	int      in_line)
{
	long	hhcuHash = 0L;

	_work_open (8,0,60);
	cuin_rec.ho_hash = store [in_line].hhhoHash;
	strcpy (cuin_rec.inv_no, key_val);
	save_rec ("#Invoice ", "#Number / Customer name.");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && !strncmp (cuin_rec.inv_no, key_val,strlen (key_val)) && 
		       cuin_rec.ho_hash == store [in_line].hhhoHash)
	{
		if (hhcuHash != cuin_rec.hhcu_hash)
		{
			cumr2_rec.hhcu_hash = cuin_rec.hhcu_hash;
			cc = find_hash (cumr2,&cumr2_rec,EQUAL,"r",cuin_rec.hhcu_hash);
			if (cc)
				sprintf (cumr_rec.dbt_name, "%40.40s", " ");
	
			hhcuHash = cuin_rec.hhcu_hash;
		}
		sprintf (err_str, "%-6.6s / %-40.40s", 
								cumr2_rec.dbt_no,cumr2_rec.dbt_name);

		cc = save_rec (cuin_rec.inv_no, err_str);
		if (cc)
			break;

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cuin_rec.ho_hash = store [in_line].hhhoHash;
	strcpy (cuin_rec.inv_no,temp_str);
	cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");
}

/*
 * Search routine for Department Master File.
 */
void
SrchCudp (
	char	*key_val)
{
	_work_open (2,0,40);
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,local_rec.branchNo);
	strcpy (cudp_rec.dp_no,key_val);
	save_rec ("#No", "#Department description");
	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && !strncmp (cudp_rec.dp_no, key_val,strlen (key_val)) && 
		      !strcmp (cudp_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cudp_rec.br_no,local_rec.branchNo))
	{
		save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,local_rec.branchNo);
	strcpy (cudp_rec.dp_no,temp_str);
	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cudp, "DBFIND");
}

/*
 * Print Company/Branch/Department.
 */
void
PrintCoLine (void)
{
	line_at (20,1,130); 
	strcpy (err_str,ML (mlStdMess038));
	print_at (21, 0, err_str, comm_rec.co_no,comm_rec.co_name);
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0, err_str,local_rec.branchNo,esmr_rec.est_name);
	strcpy (err_str,ML (mlStdMess085));
	print_at (22, 60, err_str,local_rec.departmentNo,cudp_rec.dp_name);
}

/*
 * Calculate Journal proof total. 
 */
void
CalcJournalTotal (void)
{
	D_VALUE (line_cnt) = no_dec ((local_rec.dc1_flag [0] == 'D') 
			? local_rec.localAmount : local_rec.localAmount * -1);
	PrintJournalTotal ();
}

/*
 * Print Journal proof total. 
 */
void
PrintJournalTotal (void)
{
	int		i;
	double	total = 0.00;
	
	for (i = 0; i < MAXLINES; i++)
		total += no_dec (D_VALUE (i) + G_VALUE (i));

	move (1,19);
	cl_line ();
	print_at (19,1,ML (mlDbMess209),DOLLARS (total));
}

/*
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.         
 */
int
OpenAudit (void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L164\n");
	fprintf (fout,".ECUSTOMER JOURNALS\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s as at %s\n", clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B1\n");
	fprintf (fout,".R");
	fprintf (fout,"==================================");
	fprintf (fout,"==================================================================");
	fprintf (fout,"==============================================================\n");

	fprintf (fout,"==================================");
	fprintf (fout,"==================================================================");
	fprintf (fout,"==============================================================\n");

	fprintf (fout,"|   J O U R N A L     D E T A I L S  |");
	fprintf (fout,"                 D E B T O R S      D E T A I L S                |");
	fprintf (fout,"  G E N E R A L      L E D G E R       D E T A I L S     |\n");

	fprintf (fout,"|------------------------------------|");
	fprintf (fout,"-----------------------------------------------------------------|");
	fprintf (fout,"---------------------------------------------------------|\n");

	if (envDbMcurr)
	{
		fprintf (fout,"|BR|DP|JOURNAL.| DATE  OF |   DATE   |");
		fprintf (fout," CUST.| CUSTOMER|D/C|    AMOUNT   |APPLIED | EXCHANGE| EQUIV.LOC.|");
		fprintf (fout,"     ACCOUNT    |D/C|G/L  AMOUNT|PER|   G/L  NARRATIVE   |\n");
	
		fprintf (fout,"|NO|NO| NUMBER | JOURNAL  |  POSTED  |");
		fprintf (fout,"NUMBER| ACRONYM |   |             |   TO   |   RATE  |   AMOUNT  |");
		fprintf (fout,"     NUMBER     |   |           |NO.|                    |\n");
	}
	else
	{
		fprintf (fout,"|BR|DP|JOURNAL | DATE  OF |   DATE   |");
		fprintf (fout," CUST.|CUSTOMER |D/C|    AMOUNT   |APPLIED |         |           |");
		fprintf (fout,"    ACCOUNT     |D/C|G/L  AMOUNT|PER|   G/L  NARRATIVE   |\n");
	
		fprintf (fout,"|NO|NO| NUMBER | JOURNAL  |  POSTED  |");
		fprintf (fout,"NUMBER| ACRONYM |   |             |        |         |           |");
		fprintf (fout,"    NUMBER      |   |           |NO.|                    |\n");
	}

	fprintf (fout,"|--|--|--------|----------|----------|");
	fprintf (fout,"------|---------|---|-------------|--------|---------|-----------|");
	fprintf (fout,"----------------|---|-----------|---|--------------------|\n");

	return (EXIT_SUCCESS);
}

/*
 * Routine to close the audit trail output file. 
 */
void
CloseAudit (void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}

/*
 * Check if journal number not already used in current company. 
 */
int
CheckJournal (
	char	*journalNo)
{
	char	workJournalNo [9];

	sprintf (workJournalNo, "%-8.8s", journalNo);
	sprintf (cuin2_rec.inv_no, "%-8.8s", journalNo);

	cc = find_rec (cuin2, &cuin2_rec, GTEQ, "r");
	while (!cc && !strcmp (cuin2_rec.inv_no,workJournalNo))
	{
		cumr2_rec.hhcu_hash = cuin2_rec.hhcu_hash;
		cc = find_hash (cumr2,&cumr2_rec,EQUAL,"r",cuin2_rec.hhcu_hash);
		if (cc)
			return (EXIT_SUCCESS);
		
		if (!strcmp (cumr2_rec.co_no, comm_rec.co_no))
        	return (EXIT_FAILURE);
		
		cc = find_rec (cuin2, &cuin2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		PrintJournalTotal ();
		rv_pr (ML (mlDbMess210),40,0,1);
		line_at (1,0,130);

		if (scn == SCN_HEAD)
		{
			box (0,3,130, 7);
			line_at (7,1,129);
		}
		PrintCoLine ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

static int
CheckClass (
	char	*msg)
{
	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
		return print_err (ML (mlStdMess025));

	return (EXIT_SUCCESS);
}
	
/*
 * Add Commission record for receipts made against an invoice. 
 */
void
AddCommission (
	long	hhcuHash,
	long	hhciHash,
	long	hhcpHash,
	double	payAmount)
{
	double	PayPercent = 0.00,
			CommAmt    = 0.00;

	sach_rec.hhci_hash = hhciHash;
	cc = find_rec (sach, &sach_rec, GTEQ, "r");
	while (!cc && sach_rec.hhci_hash == hhciHash)
	{
		PayPercent = payAmount / sach_rec.inv_amt;
		CommAmt    = sach_rec.com_val * PayPercent;
	
		sacl_rec.sach_hash	=	sach_rec.sach_hash;
		sacl_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		sacl_rec.rec_amt	=	payAmount;
		sacl_rec.rec_date	=	local_rec.dateJournal;
		sacl_rec.com_amt	=	CommAmt;
		strcpy (sacl_rec.status, "0");

		cc = find_rec (sacl, &sacl_rec, COMPARISON, "u");
		if (cc)
		{
			sacl_rec.rec_amt	=	payAmount;
			sacl_rec.rec_date	=	local_rec.dateJournal;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_add (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBADD");
		}
		else
		{
			sacl_rec.rec_amt	=	payAmount;
			sacl_rec.rec_date	=	local_rec.dateJournal;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_update (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBUPDATE");
		}
		cc = find_rec (sach, &sach_rec, NEXT, "r");
	}
}

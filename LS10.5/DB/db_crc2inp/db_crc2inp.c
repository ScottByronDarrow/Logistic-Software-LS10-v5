/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crc2inp.c,v 5.8 2002/07/24 08:38:47 scott Exp $
|  Program Name  : (db_crc2inp.c) 
|  Program Desc  : (Customer Sundry Receipts Input)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (09/12/92)       |
|---------------------------------------------------------------------|
| $Log: db_crc2inp.c,v $
| Revision 5.8  2002/07/24 08:38:47  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 06:24:13  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/06/26 04:34:08  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/06/26 04:26:44  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/08/20 23:10:24  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 08:23:25  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:49  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:02  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crc2inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crc2inp/db_crc2inp.c,v 5.8 2002/07/24 08:38:47 scott Exp $";

#define MAXSCNS		3
#define MAXLINES	800

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include 	<twodec.h>

#define	BANK_DRAFT	 (local_rec.rec_type [0] == 'B')
#define	DIRECT_CRD	 (local_rec.rec_type [0] == 'D')
#define	CASH_PYMNT	 (local_rec.rec_type [0] == 'A')
#define	CHQ_PAYMNT	 (local_rec.rec_type [0] == 'C')

#define	CF(x)	 (comma_fmt ( (x), "N,NNN,NNN,NNN.NN"))

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct crbkRecord	crbk_rec;
struct bkcrRecord	bkcr_rec;
struct blhdRecord	blhd_rec;
struct bldtRecord	bldt_rec;
struct bldtRecord	bldt2_rec;

	/*
	 * Customer Sundry Receipts Work File Record.
	 */
	struct {
		char        co_no [3];
		char        est [3];
		char        rpt_no [9];
		long        date_of_rpt;
		char        type [2];
		char        pay_type [2];
		char        gl_acc_no [MAXLEVEL + 1];
		char        p_no [3];
		char        name [41];
		char        acronym [10];
		char        narrative [21];
		char        bank_code [4];
		char        branch_code [21];
		Money		amt; 
		Money      	disc; 
		Money      	p_amt; 
		Money      	tax;   
		Money      	gst;    
		Money      	freight; 
		char        stat_flag [2];

		char        bk_id [6];
		char        bk_curr [4];
		double      bk_exch;
		Money      	bk_rec_amt;
		Money      	bk_charge; 
		Money      	bk_lcl_exch;

		Money	    o_p_amt;	
		char	    o_curr [4];
		double      o_exch;
		Money      	o_disc;  
		Money      	o_amount;
	} cusr_rec;

	char	*data  = "data",
			*bldt2 = "bldt2";

	int		envDbMcurr		=	FALSE,
			fwd_message	=	TRUE,
			wk_line		=	0,
			pid,	
       		sel_yr		=	0,
			cusr_no,
			chq_error	=	TRUE,
			dtls_cnt	=	0,
			valid_lc	=	FALSE,
			envDbAutoRec	=	FALSE,
			defaultReference = FALSE;

	long	next_no		=	0; 
	long	mend_date	=	0L;
	long	ALT_DATE	=	0L;

   	double	rpt_total	=	0;
	double  proof_total =	0;
	double  batch_tot	= 	0;
	double	bk_lcl_exch;
	double	alloc_tot;

	char	envCurrCode [4];
	char	envDbLdgPres [2];

struct	storeRec {
	double	amt_opay;
	double	amt_lpay;
} store [MAXLINES];

	char	*scn_desc [] = {
		"HEADER SCREEN.",
		"ALLOCATION SCREEN.",
		""
	};

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	prev_receipt [9];
	char	systemDate [11];
	long	longSystemDate;

	char	bk_curr_desc [41];
	char	rec_no [9];
	long	rec_date;
	double	rec_amt;
	double	bank_chg;
	double	rec_oamt;
	double	rec_odis;
	double	rec_lamt;
	double	rec_ldis;
	double	rec_gross;
	char	oamt_prmt 	[31];
	char	odis_prmt 	[31];
	char	lamt_prmt 	[31];
	char	bk_ex_prmt 	[31];
	char	gross_prmt 	[31];
	char	narrative 	[21];
	char	rec_type 	[2];
	char	receiptTypeDesc [14];
	double	bk_exch_rate;
	double	exch_rate;
	char	curr_code [4];
	char	curr_desc [41];
	char	name [41];

	char	glacc [MAXLEVEL + 1];
	char	glacc_desc [26];
	char	l_curr [4];
	double	l_exch_rate;
	double	l_loc_amt;
	double	l_rec_oamt;
	double	l_rec_lamt;
	char	db_bank [4];
	char	db_branch [21];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "bank_id",	 3, 26, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Bank Code             ", "Enter Bank Code. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 3, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Name             ", "Enter Bank name.",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "bk_curr",	 4, 26, CHARTYPE,
		"AAA", "          ",
		" ", "", "Currency              ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "bk_curr_desc",	 4, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency Name         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},

	{1, LIN, "receipt",	 6, 26, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Cheque/Receipt No.", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.rec_no},
	{1, LIN, "name",	 	 6, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Name ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.name}, 
	{1, LIN, "bank",	7, 26, CHARTYPE,
		"UUU", "          ",
		" ", "", "Bank Code.  ", " Enter Bank code. ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.db_bank},
	{1, LIN, "branch",	7, 80, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Branch Code. ", "Enter Branch Code ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.db_branch},
	{1, LIN, "rec_date",	 8, 26, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date of Receipt", "Enter Receipt date. < default = System date > ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.rec_date},
	{1, LIN, "rec_type",	9, 26, CHARTYPE,
		"U", "          ",
		" ", "C", "Receipt Type.         ", "Enter A-Cash, (B)ank Draft, (C)heque, (D)irect Credit.",
		YES, NO,  JUSTLEFT, "CBAD", "", local_rec.rec_type},
	{1, LIN, "receiptTypeDesc",9, 30, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.receiptTypeDesc},
	{1, LIN, "ref",	 	10, 26, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Reference             ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.narrative}, 
	{1, LIN, "bk_exch_rate",	 10, 80, DOUBLETYPE,
		"NNNNN.NNNNNNNN", "          ",
		" ", "0", local_rec.bk_ex_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.bk_exch_rate},
	{1, LIN, "bank_chg",	 11, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Bank Charges.", "Enter amount of Bank charges. ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.bank_chg},
	{1, LIN, "rec_amt",	 11, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Bank Receipt Amount.", "Enter amount of Receipt. ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_amt},
	{1, LIN, "curr_code",	 13, 26, CHARTYPE,
		"UUU", "          ",
		" ", crbk_rec.curr_code, "Currency              ", " ",
		 YES, YES,  JUSTLEFT, "", "", local_rec.curr_code},
	{1, LIN, "curr_desc",	 13, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency Name         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},
	{1, LIN, "orec_amt",	 14, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.oamt_prmt, " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_oamt},
	{1, LIN, "orec_dis",	 14, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.odis_prmt, " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_odis},
	{1, LIN, "exch_rate",	 15, 26, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Exchange rate.", " ",
		 NI, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.exch_rate},
	{1, LIN, "lrec_amt",	 16, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.lamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_lamt},
	{1, LIN, "rec_gross",	 16, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.gross_prmt, " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_gross},
	{1, LIN, "lrec_dis",	 16, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_ldis},

	{2, TAB, "glacc",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.glacc},
	{2, TAB, "glacc_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Account Description           ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.glacc_desc},
	{2, TAB, "l_rec_oamt",	 0, 1, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "  Foreign Amount  ", " ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_oamt},
	{2, TAB, "l_rec_lamt",	 0, 1, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Local Receipt Amt ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_lamt},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
int 	DeleteLine 			(void);
void 	Update 				(void);
void 	AddCusr 			(int, int);
void 	SrchCrbk 			(char *);
void 	PrintTotals 		(void);
void 	proof_trans 		(void);
int 	heading 			(int);
static 	int 	CheckClass (void);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	int	i;

	if (argc < 2)
	{
		print_at (0,0, "Usage %s <PID>\n", argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check program name.
	 */
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	pid  = atoi (argv [1]);

	/*
	 * Multi-currency debtors.
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Auto generated receipt numbers.
	 */
	sptr = chk_env ("DB_AUTO_REC");
	if (sptr)
		envDbAutoRec = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envDbAutoRec = FALSE;

	/*
	 * Lodgements presented flag.
	 */
	sptr = chk_env ("DB_LDG_PRES");
	if (sptr == (char *) 0)
		strcpy (envDbLdgPres, "N");
	else
		sprintf (envDbLdgPres, "%-1.1s", sptr);

	/*
	 * Set up Screen Prompts.
	 */
	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	SETUP_SCR (vars);


	init_scr ();
	set_tty (); 
	if (envDbMcurr)
		_set_masks ("db_crc2inpM.s");
	else
		_set_masks ("db_crc2inpS.s");

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	for (i = 0; i < 4; i++)
		tab_data [i]._desc = scn_desc [i];

	/*
	 * Disable Currency fields.
	 */
	if (!envDbMcurr)
	{	
		FLD ("bk_curr") 		= ND;
		FLD ("bk_curr_desc") 	= ND;
		FLD ("bk_exch_rate") 	= ND;
		FLD ("rec_amt") 		= ND;
		FLD ("curr_code") 		= ND;
		FLD ("curr_desc") 		= ND;
		FLD ("exch_rate") 		= ND;
		FLD ("lrec_amt") 		= ND;

		for (i = label ("receipt"); i <= label ("bank_chg"); i++)
			vars [i].row -= 1;

		vars [label ("orec_amt")].row = 11;
		vars [label ("orec_dis")].row = 11;
		vars [label ("rec_gross")].row = 12;
		vars [label ("rec_gross")].col = 26;

		FLD ("l_rec_lamt") = ND;
	}

	init_vars (1);	

	tab_col = 20;
	tab_row = 6;

	swide ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.longSystemDate = TodaysDate ();

	strcpy (local_rec.oamt_prmt,   ML ("Receipt Amount        "));
	strcpy (local_rec.odis_prmt,   ML ("Discount Amount       "));
	strcpy (local_rec.bk_ex_prmt,  ML ("Bank exch rate        "));
	sprintf (local_rec.lamt_prmt,  "%s (%-3.3s) ", 
								  ML ("Receipt Amount  "),envCurrCode);
	if (envDbMcurr)
	{
		sprintf (local_rec.gross_prmt, "%s (%-3.3s) ", 
								  ML ("Gross receipt  "), envCurrCode);
	}
	else
	{
		strcpy (local_rec.gross_prmt, ML ("Gross receipt        "));
	}

	OpenDB ();

	GL_SetMask (GlFormat);

	prog_exit = 0;

	/*
	 * Beginning of Input Control Loop.
	 */
	while (prog_exit == 0) 
	{
		edit_exit = 0;
   		entry_exit = 0;
   		prog_exit = 0;
   		restart = 0;
		init_vars (1);	/*  set default values  */
		lcount [2] = 0;
		local_rec.rec_date = local_rec.longSystemDate;
		rpt_total = 0;
		search_ok = 1;
		chq_error = TRUE;
		for (i = 0; i < MAXLINES; i++)
		{
			store [i].amt_lpay	= 0.00;
			store [i].amt_opay	= 0.00;
		}

		/*
		 * Enter screen 1 linear input.
		 */
		defaultReference = FALSE;
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		init_vars (2);
		heading (2);
		PrintTotals ();
		entry (2);
		if (lcount [2] == 0) 
			restart = TRUE;

		if (restart) 
			continue;	

		edit_all ();
		if (restart) 
			continue;	

		proof_trans ();

		/*
		 * re-edit tabular if proof total incorrect. 
		 */
		while (chq_error)
		{
			edit_all ();
			if (restart)
				break;

			if (prog_exit)
			{
				prog_exit = 0;
				continue;
			}
			proof_trans ();
		}
		if (!restart)
			Update ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program Exit Sequence. 
 */
void
shutdown_prog (void)
{
    char message [55];
    
	clear ();
	snorm ();

    sprintf (message, ML (mlDbMess042), DOLLARS (batch_tot));
    strcat (message, ". ");
    strcat (message, ML (mlStdMess042));
    PauseForKey (0, 0, message, 0);

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

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	mend_date = MonthEnd (comm_rec.dbt_date);

	/*
	 * Open work file used for cash receipts journals etc.
	 */
	sprintf (filename,
		"%s/WORK/db_crc%05d", 
		 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	cc = RF_OPEN (filename, sizeof (cusr_rec), "w", &cusr_no);
	if (cc) 
		file_err (cc, filename, "RF_OPEN");

	/*
	 * Open database files. 
	 */
	abc_alias (bldt2, bldt);

	open_rec (bkcr,  bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
	open_rec (blhd,  blhd_list, BLHD_NO_FIELDS, "blhd_id_no");
	open_rec (bldt,  bldt_list, BLDT_NO_FIELDS, "bldt_id_no3");
	open_rec (bldt2, bldt_list, BLDT_NO_FIELDS, "bldt_hhcp_hash");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (crbk,  crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	OpenGlmr ();
	OpenPocr ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (bkcr);
	abc_fclose (blhd);
	abc_fclose (bldt);
	abc_fclose (bldt2);
	abc_fclose (comr);
	abc_fclose (crbk);
	abc_fclose (esmr);

	GL_Close ();
	abc_dbclose (data);

	cc = RF_CLOSE (cusr_no);
	if (cc) 
		file_err (cc, "cusr_no", "WKCLOSE");
}

int
spec_valid (
	int		field)
{
	int	i = 0;
    /* thispage used before initialized, added initialization */
	int	this_page = line_cnt / TABLINES;

	long	MonthStart (long); 
	long	MonthEnd (long);
	long	max_fdate = 0L;
	
	/*
	 * Validate Creditor Number And Allow Search.
	 */
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (!cc)
				strcpy (crbk_rec.bank_id, esmr_rec.dflt_bank);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (envDbMcurr)
		{
			strcpy (pocrRec.co_no, comm_rec.co_no);
			strcpy (pocrRec.code, crbk_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			bk_lcl_exch = pocrRec.ex1_factor;
		}
		else
			bk_lcl_exch = 1.00;

		/*
		 * Find lodgement header.
		 */
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		cc = find_rec (blhd, &blhd_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess078));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.bk_curr_desc, pocrRec.description);
		DSP_FLD ("bk_name");
		DSP_FLD ("bk_curr_desc");
		DSP_FLD ("bk_curr");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Receipt Number.
	 */
	if (LCHECK ("receipt"))
	{
		if (dflt_used)
		{
			if (envDbAutoRec)
			{
				defaultReference = TRUE;
				strcpy (local_rec.rec_no, "NEW RCPT");
				DSP_FLD ("receipt");
			}
			else
			{
				print_mess (ML (mlStdMess007));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status == ENTRY)
			lcount [2] = 0;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate currency code.
	 */
	if (LCHECK ("curr_code"))
	{
		if (!envDbMcurr)
		{
			local_rec.exch_rate = 1.00;
			local_rec.bk_exch_rate = 1.00;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}
	
		/*
		 * Check customers currency code.
		 */
		cc = FindPocr (comm_rec.co_no, local_rec.curr_code, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Find Foreign to Bank Exch rate.
		 */
		strcpy (bkcr_rec.co_no, comm_rec.co_no);
		sprintf (bkcr_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		sprintf (bkcr_rec.curr_code, "%-3.3s", local_rec.curr_code);
		cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.exch_rate 	= pocrRec.ex1_factor;
		local_rec.bk_exch_rate 	= bkcr_rec.ex1_factor;
		sprintf (local_rec.curr_desc, "%-40.40s", pocrRec.description);
		DSP_FLD ("bk_exch_rate");
		DSP_FLD ("exch_rate");
		DSP_FLD ("curr_desc");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("name"))
	{
		if (strlen (clip (local_rec.name)) == 0)
		{
			sprintf (local_rec.db_bank,   "%4.4s",   " ");
			sprintf (local_rec.db_branch, "%20.20s", " ");

			DSP_FLD ("name");
			DSP_FLD ("bank");
			DSP_FLD ("branch");

			skip_entry = goto_field (field, label ("rec_date"));
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Overseas receipt amount.
	 */
	if (LCHECK ("orec_amt"))
	{
		if (FLD ("orec_amt") == NA || cur_screen == 2)
			return (EXIT_SUCCESS);

		local_rec.rec_amt  = local_rec.rec_oamt;
		if (local_rec.bk_exch_rate != 0.00)
			local_rec.rec_amt /= local_rec.bk_exch_rate;

		local_rec.rec_lamt = local_rec.rec_oamt;
		if (local_rec.exch_rate != 0.00)
			local_rec.rec_lamt /= local_rec.exch_rate;

		DSP_FLD ("rec_amt");
		DSP_FLD ("lrec_amt");
		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (local_rec.exch_rate != 0.00)
		{
			local_rec.rec_gross /= local_rec.exch_rate;
			local_rec.rec_ldis  /= local_rec.exch_rate;
		}
		DSP_FLD ("rec_gross");
	}

	/*
	 * Validate Overseas Receipt Amount.
	 */
	if (LCHECK ("orec_dis"))
	{
		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (local_rec.exch_rate != 0.00)
		{
			local_rec.rec_gross /= local_rec.exch_rate;
			local_rec.rec_ldis  /= local_rec.exch_rate;
		}

		DSP_FLD ("rec_gross");
	}

	/*
	 * Validate receipt Date.
	 */
	if (LCHECK ("rec_date"))
	{
		FLD ("rec_type") = YES;

		if (dflt_used)
			local_rec.rec_date = local_rec.longSystemDate;

		DSP_FLD ("rec_date");
		max_fdate = MonthEnd (comm_rec.dbt_date) + 1L;

		if (local_rec.rec_date < MonthStart (comm_rec.dbt_date)) 
			return print_err (ML (mlDbMess099));

		if (local_rec.rec_date > MonthEnd (comm_rec.dbt_date) &&
		     fwd_message == TRUE)
		{
			i = prmptmsg (ML (mlDbMess097), "YyNn", 1, 2);
			if (i == 'y' || i == 'Y')
			{
				i = prmptmsg (ML (mlDbMess098), "YyNn", 1, 2);
				fwd_message = (i == 'y' || i == 'Y') ? FALSE : TRUE;
			}
			else
			{
				line_at (2,1,131);
				return (EXIT_FAILURE);
			}
			line_at (2,1,131);
		}
	
		return (EXIT_SUCCESS);
	}
			
	/*
	 * Validate receipt type.
	 */
	if (LCHECK ("rec_type"))
	{
		if (FLD ("rec_type") == NA)
			return (EXIT_SUCCESS);

		if (CASH_PYMNT)
			strcpy (local_rec.receiptTypeDesc, ML ("Cash         "));

		if (BANK_DRAFT)
		{
			strcpy (local_rec.receiptTypeDesc, ML ("Bank Draft   "));
			FLD ("ref")      = YES;
		}

		if (CHQ_PAYMNT)
			strcpy (local_rec.receiptTypeDesc, ML ("Cheque       "));

		if (DIRECT_CRD)
			strcpy (local_rec.receiptTypeDesc, ML ("Direct Credit"));

		DSP_FLD ("receiptTypeDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate GL Account.
	 */
	if (LCHECK ("glacc"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		if (last_char == DELLINE)
			return (DeleteLine ());
		/*
		 * Delete GL Allocn Line If Null Entry.   
		 */
		if (dflt_used && prog_status != ENTRY)
		{
			lcount [2]--;
			for (wk_line = line_cnt;line_cnt < lcount [2];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				line_display ();
			}
			sprintf (local_rec.glacc, "%*.*s", MAXLEVEL,MAXLEVEL," ");
			local_rec.l_rec_oamt = 0.00;
			strcpy (local_rec.l_curr, " ");
			local_rec.l_exch_rate = 0.00;
			local_rec.l_rec_lamt = 0.00;
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				blank_display ();

			line_cnt = wk_line;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		strcpy (glmrRec.co_no,comm_rec.co_no);
		GL_FormAccNo (local_rec.glacc, glmrRec.acc_no, 0);
		cc = find_rec (glmr , &glmrRec, COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckClass ())
			return (EXIT_FAILURE);
		
		sprintf (local_rec.glacc_desc, "%-25.25s", glmrRec.desc);	

		DSP_FLD ("glacc_desc");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Payment Amount.
	 */
	if (LCHECK ("l_rec_oamt"))
	{
		if (local_rec.exch_rate != 0.00)
		{
			local_rec.l_rec_lamt = local_rec.l_rec_oamt;
			local_rec.l_rec_lamt /= local_rec.exch_rate;
		}
		store [line_cnt].amt_opay = local_rec.l_rec_oamt;
		store [line_cnt].amt_lpay = local_rec.l_rec_lamt;

		DSP_FLD ("l_rec_oamt");
		DSP_FLD ("l_rec_lamt");
		putval (line_cnt);
		PrintTotals ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	lcount [2]--;

	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		store [line_cnt].amt_opay = store [line_cnt + 1].amt_opay;
		store [line_cnt].amt_lpay = store [line_cnt + 1].amt_lpay;
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	strcpy (local_rec.l_curr, "   ");
	local_rec.l_exch_rate = 0.00;
	local_rec.l_loc_amt = 0.00;
	local_rec.l_rec_oamt = 0.00;
	local_rec.l_rec_lamt = 0.00;
	store [line_cnt].amt_opay = 0.00;
	store [line_cnt].amt_lpay = 0.00;
	
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	print_at (2,0, "                          ");
	fflush (stdout);

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}
	
/*
 * Main Update Routine.
 */
void
Update (void)
{
	int		receiptPeriod;

	DateToDMY (local_rec.rec_date, NULL, &receiptPeriod, NULL);

	/*
	 * Add or Update Receipt Payment Header.
	 */
	print_at (3,1, ML (mlDbMess043),local_rec.rec_no);

	/*
	 * Find lodgement header.
	 */
	strcpy (blhd_rec.co_no, comm_rec.co_no);
	sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
	cc = find_rec (blhd, &blhd_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, blhd, "DBFIND");

	/*
	 * Add bank lodgement detail record.
	 */
	bldt_rec.hhbl_hash 	= blhd_rec.hhbl_hash;
	bldt_rec.lodge_no  	= blhd_rec.nx_lodge_no;
	sprintf (bldt_rec.rec_type, "%-1.1s", local_rec.rec_type);
	bldt_rec.lodge_date = local_rec.rec_date;
	bldt_rec.lodge_time = 0L;
	bldt_rec.hhcu_hash 	= 0L;
	sprintf (bldt_rec.dbt_name, "%-40.40s", local_rec.name);
	strcpy (bldt_rec.bank_code, local_rec.db_bank);
	strcpy (bldt_rec.branch_code, local_rec.db_branch);

	bldt_rec.hhcp_hash = 0L;
	strcpy (bldt_rec.sundry_rec, local_rec.rec_no);
	bldt_rec.amount = local_rec.rec_amt;
	bldt_rec.bank_chg = local_rec.bank_chg;
	bldt_rec.bk_lcl_exch = bk_lcl_exch;
	if (CHQ_PAYMNT)
		bldt_rec.chq_fees = crbk_rec.clear_fee;
	else
		bldt_rec.chq_fees = 0.00;

	bldt_rec.due_date = 0L;
	strcpy (bldt_rec.posted_gl, "Y");
	sprintf (bldt_rec.presented, "%-1.1s", envDbLdgPres);
	strcpy (bldt_rec.reconcile, "N");
	strcpy (bldt_rec.stat_flag, "0");

	if (defaultReference)
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");
		
		bldt2_rec.hhbl_hash = bldt_rec.hhbl_hash;
		sprintf (bldt2_rec.sundry_rec, "%08ld", ++comr_rec.nx_rec_no);
		for (cc = find_rec (bldt, &bldt2_rec, EQUAL, "r");
			 !cc;
			 cc = find_rec (bldt, &bldt2_rec, EQUAL, "r"))
		{
			sprintf (bldt2_rec.sundry_rec, "%08ld", ++comr_rec.nx_rec_no);
		}
		strcpy (bldt_rec.sundry_rec, bldt2_rec.sundry_rec);
		cc = abc_add (bldt, &bldt_rec);
		if (cc)
			file_err (cc, bldt, "DBADD");

		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		strcpy (local_rec.rec_no, bldt_rec.sundry_rec);
	}
	else
	{
		cc = abc_add (bldt, &bldt_rec);
		if (cc)
			file_err (cc, bldt, "DBADD");
	}

	scn_set (2);

	/*
	 * Set Sundry Receipts Workfile Records.
	 */
	for (wk_line = 0; wk_line < lcount [2]; wk_line++) 
	{
	    getval (wk_line);
	    AddCusr (receiptPeriod, FALSE);
	}

	scn_set (1);

	strcpy (local_rec.prev_receipt, local_rec.rec_no);
	batch_tot += local_rec.rec_oamt;
}

void
AddCusr (
	int		receiptPeriod,
	int		rvs_dep)
{
	strcpy (cusr_rec.co_no,   comm_rec.co_no);
	strcpy (cusr_rec.est,     comm_rec.est_no);
	strcpy (cusr_rec.rpt_no,  local_rec.rec_no);
	cusr_rec.date_of_rpt  = local_rec.rec_date;
	strcpy (cusr_rec.type,         "1");
	sprintf (cusr_rec.pay_type,    "%-1.1s",   local_rec.rec_type);
	sprintf (cusr_rec.gl_acc_no,   "%-*.*s",   
								MAXLEVEL, MAXLEVEL, local_rec.glacc);
	sprintf (cusr_rec.p_no,        "%02d",     receiptPeriod);
	sprintf (cusr_rec.name,        "%-40.40s", local_rec.name);
	sprintf (cusr_rec.acronym,     "%-9.9s",   local_rec.narrative);
	sprintf (cusr_rec.narrative,   "%-20.20s", local_rec.narrative);
	sprintf (cusr_rec.bank_code,   "%-3.3s",   " ");
	sprintf (cusr_rec.branch_code, "%-20.20s", " ");
	cusr_rec.amt     	= no_dec (local_rec.rec_lamt + local_rec.rec_ldis);
	cusr_rec.disc    	= no_dec (local_rec.rec_ldis);
	cusr_rec.o_p_amt   	= no_dec (local_rec.l_rec_oamt);
	cusr_rec.p_amt   	= no_dec (local_rec.l_rec_lamt);
	cusr_rec.tax     	= 0.00;
	cusr_rec.gst     	= 0.00;
	cusr_rec.freight 	= 0.00;
	strcpy (cusr_rec.stat_flag, "1");

	sprintf (cusr_rec.bk_id,   "%-5.5s", crbk_rec.bank_id);
	sprintf (cusr_rec.bk_curr, "%-3.3s", crbk_rec.curr_code);
	cusr_rec.bk_exch     = local_rec.bk_exch_rate;
	cusr_rec.bk_rec_amt  = no_dec (local_rec.rec_amt);
	cusr_rec.bk_charge   = no_dec (local_rec.bank_chg);
	cusr_rec.bk_lcl_exch = bk_lcl_exch;

	sprintf (cusr_rec.o_curr, "%-3.3s", local_rec.curr_code);
	cusr_rec.o_exch   = local_rec.exch_rate;
	cusr_rec.o_disc   = no_dec (local_rec.rec_odis);
	cusr_rec.o_amount = no_dec (local_rec.rec_oamt + local_rec.rec_odis);

	cc = RF_ADD (cusr_no, (char *) &cusr_rec);
	if (cc) 
		file_err (cc, "cusr_rec", "WKADD");
}

/*
 * Search routine for Creditors Bank File.
 */
void
SrchCrbk (
	char	*key_val)
{
	_work_open (5,0,40);
	save_rec ("#Bank ","#Bank Name ");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
		      !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;

		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, "crbk", "DBFIND");
}

/*
 * Print payments total.
 */
void
PrintTotals (void)
{
	int		OffSet;
	int 	i; 
	int		no_lines = (prog_status == ENTRY) ? line_cnt : lcount [2] - 1;
	double	rec_lamt = 0.00,
			rec_oamt = 0.00,
			all_amt = 0.00;

	rpt_total = 0;
	OffSet = strlen (GlMask);

	for (i = 0; i <= no_lines; i++) 
	{
		rec_lamt += store [i].amt_lpay;
		rec_oamt += store [i].amt_opay;
	}

	rec_oamt = DOLLARS (rec_oamt);
	rec_lamt = DOLLARS (rec_lamt);
	all_amt  = DOLLARS (local_rec.rec_oamt + local_rec.rec_odis);

	if (envDbMcurr)
	{
		sprintf (err_str, ML (mlDbMess048), bkcr_rec.curr_code, all_amt);
		rv_pr (err_str, 2,19,1);
	}
	else
	{
		sprintf (err_str, ML (mlDbMess049), all_amt);
		rv_pr (err_str, 2,19,1);
	}

	sprintf (err_str, "%16.16s", CF (rec_oamt));
	rv_pr (err_str, 56 + OffSet, 19, 1);

	if (envDbMcurr)
		print_at (19, 75 + OffSet, "%16.16s", CF (rec_lamt));
}

void
proof_trans (void)
{
	int	i;

	double	head_tot = 0.00;

	head_tot = local_rec.rec_oamt + local_rec.rec_odis;

	alloc_tot = 0.00;
	for (i = 0;i < lcount [2]; i++)
		alloc_tot += store [i].amt_opay;

	proof_total = alloc_tot - head_tot;

	if (proof_total == 0.00)
		chq_error = FALSE;
	else
	{
		chq_error = TRUE;
		sprintf (err_str,ML (mlDbMess032),
			 (envDbMcurr) ? bkcr_rec.curr_code : "",
			DOLLARS (head_tot), 
			DOLLARS (alloc_tot));
		errmess (err_str);
		sleep (sleepTime);
	}	
}

int
heading (
	int		scn)
{
	int		OffSet;

	if (!restart) 
	{
		clear ();
		if (scn != cur_screen)
			scn_set (scn);


		rv_pr (ML (mlDbMess100),48,0,1);
		print_at (0,90,ML (mlDbMess044),local_rec.prev_receipt);
		move (0,1);
		line (132);

		pr_box_lines (scn);

		if (scn == 2)
		{
			box (0, 3, 130, 1);
			if (envDbMcurr)
			{
				print_at (4, 5, ML (mlStdMess104), 
					local_rec.curr_code, local_rec.curr_desc);
				print_at (4, 60, ML (mlStdMess105),
					local_rec.exch_rate);
			}
			else
			{
				print_at (4, 5,ML (mlStdMess106), crbk_rec.bank_id);
				print_at (4, 60,ML (mlStdMess083), crbk_rec.bank_name);
			}
			PrintTotals ();
			OffSet = strlen (GlMask);

			line_at (18, 54 + OffSet, 19);
			line_at (18, 73 + OffSet, 19);
			line_at (20, 54 + OffSet, 19);
			line_at (20, 73 + OffSet, 19);
		}

		sprintf (err_str,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,err_str);

		sprintf (err_str,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		print_at (22,45,err_str);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_FAILURE);
}

static int
CheckClass (void)
{
	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0]!= 'P')
	{
		print_err (ML (mlStdMess025));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

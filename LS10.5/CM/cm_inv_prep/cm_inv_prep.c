/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_inv_prep.c,v 5.3 2002/07/18 06:12:21 scott Exp $
|  Program Name  : (cm_inv_prep.c)
|  Program Desc  : (Performs consolidations of trans/t/sheets)
|                  (and creation of cohrs/colns)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 11/03/93         |
|---------------------------------------------------------------------|
| $Log: cm_inv_prep.c,v $
| Revision 5.3  2002/07/18 06:12:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2002/01/22 08:59:45  robert
| Updated to fix display problems on LS10-GUI
|
| Revision 5.1  2002/01/22 06:39:51  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_inv_prep.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_inv_prep/cm_inv_prep.c,v 5.3 2002/07/18 06:12:21 scott Exp $";

#define MAXLINES	1000
#define MAXWIDTH	700
#define	SCN_SELECT	1
#define	SCN_INVOICE	2
#define	SCN_DETAILS	3
#define	LAST_CONT	(local_rec.endContNo)
#define	COMPANY		 2
#define	BRANCH		 1
#define	USER		 0
#define	TXT_REQD
#define	COST		0
#define	SALE		1
#define	IS_LIKE		(cmcb_rec.dtl_lvl [0] == 'L')
#define	IS_NONE		(cmcb_rec.dtl_lvl [0] == 'N')
#define	IS_ALL		(cmcb_rec.dtl_lvl [0] == 'A')
#define	IS_FIXED	(cmhr_rec.quote_type [0] == 'F')
#define	PROGRESS 	(cmhr_rec.progress [0] == 'Y')
#define		BY_BRANCH	1
#define		BY_DEPART	2

#define		CASH	(cumr_rec.cash_flag [0] == 'Y')

#define		USE_WIN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<p_terms.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>
extern		int	_win_func;

#include	"schema"

struct commRecord	comm_rec;
struct cmpbRecord	cmpb_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct cmcbRecord	cmcb_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmtrRecord	cmtr_rec;
struct cmtsRecord	cmts_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct inmrRecord	inmr_rec;
struct cmcmRecord	cmcm_rec;
struct cudpRecord	cudp_rec;
struct esmrRecord	esmr_rec;

	char	*data     = "data", 
	    	*cumr2    = "cumr2", 
	    	*cohr2    = "cohr2";

	char	branchNo 	[3],
			envLogname 	[15];

	int		maxInvScn 		= 0, 
			envSoNumbers 	= BY_BRANCH, 
			curInvScn		= 0, 
			colnAdded 		= FALSE, 
			keepCohr 		= FALSE, 
			counter 		= 0;

	extern	int		TruePosition;

/*
 * Local & Screen Structures. 
 */
struct {
	char	startContNo 	[7];
	char	startContDesc 	[71];
	char	endContNo 		[7];
	char	endContDesc 	[71];
	char	desc 			[7][71];
	char	invoiceDetails 	[71];
	char	systemDate 		[11];
	char	tagged 			[2];
	char	taggedDesc 		[4];
	char	terms 			[41];
	char	dummy 			[11];
	long	lsystemDate;
	long	invoiceDate;
	double	invoiceAmount;
	double	remainingAmount;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startContNo", 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Contract No.     ", "Enter First Contract Number . ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.startContNo}, 
	{1, LIN, "first_desc", 4, 54, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.startContDesc}, 
	{1, LIN, "last_cont", 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Last  Contract No.     ", "Enter Last Contract Number . ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.endContNo}, 
	{1, LIN, "last_desc", 5, 54, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.endContDesc}, 
	{2, LIN, "cont_no", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Contract No.           ", "", 
		 NA, NO, JUSTLEFT, "", "", cmhr_rec.cont_no}, 
	{2, LIN, "cust_no", 	 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Customer No.           ", "", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.dbt_no}, 
	{2, LIN, "name", 	 	 5, 33, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{2, LIN, "mcust_no", 	 6, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Master Cust No.        ", "", 
		 NA, NO, JUSTLEFT, "", "", cumr2_rec.dbt_no}, 
	{2, LIN, "mname", 	 6, 33, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", cumr2_rec.dbt_name}, 
	{2, LIN, "cus_ord_ref", 	 7, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Customer Order Ref.    ", " ", 
		NA, NO, JUSTLEFT, "", "", cmhr_rec.cus_ref}, 
	{2, LIN, "desc1", 	 9, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Contract Description   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [0]}, 
	{2, LIN, "desc2", 	 10, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [1]}, 
	{2, LIN, "desc3", 	11, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [2]}, 
	{2, LIN, "desc4", 	12, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [3]}, 
	{2, LIN, "desc5", 	13, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [4]}, 
	{2, LIN, "desc6", 	14, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [5]}, 
	{2, LIN, "desc7", 	15, 2, 	CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc [6]}, 
	{2, LIN, "invoiceAmount", 	17, 2, MONEYTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", " ", "Invoice Value          ", " ", 
		 NA, NO, JUSTRIGHT, "0.00", "99999999.99", (char *) &local_rec.invoiceAmount}, 
	{2, LIN, "invoiceDate", 	 17, 45, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Invoice To date  ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.invoiceDate}, 
	{2, LIN, "tag", 		17, 90, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Tag For Invoicing      ", " ", 
		 NI, NO, JUSTLEFT, "YN", "", local_rec.tagged}, 
	{2, LIN, "tag_desc", 		17, 120, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.taggedDesc}, 
	{2, LIN, 	"hash", 			18, 4, LONGTYPE, 
		"NNNNNNNNNN", 	"          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", (char *) &cmhr_rec.hhhr_hash}, 
	{2, LIN, 	"quote_type", 			18, 4, CHARTYPE, 
		"U", 	"          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", cmhr_rec.quote_type}, 
	{2, LIN, "quote_val", 	18, 4, MONEYTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "0.00", "99999999.99", (char *) &cmhr_rec.quote_val}, 
	{2, LIN, 	"internal", 			18, 4, CHARTYPE, 
		"U", 	"          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", cmhr_rec.internal}, 
	{2, LIN, 	"progress", 			18, 4, CHARTYPE, 
		"U", 	"          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", cmhr_rec.progress}, 
	{2, LIN, 	"status", 			18, 4, CHARTYPE, 
		"U", 	"          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", cmhr_rec.status}, 
	{3, TXT, "", 	 4, 2, 0, 
		"", "          ", 
		"", "", "                      Charge Details For Invoice                      ", "", 
		 11, 70, 500, "", "", local_rec.invoiceDetails}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * Local function prototypes 
 */
int		CheckCohr		(char *);
int		CheckOK			(char *);
int		heading			(int);
int		spec_valid		(int);
int		use_window		(int);
int		win_function	(int, int, int, int);
void	AddColn			(int, char *, double, double, float, double);
void	CloseDB		 	(void);
void	LoadCmcds		(void);
void	LoadFields		(void);
void	LoadInvoices	(void);
void	OpenDB			(void);
void	ProcessCmhr		(void);
void	SetFields		(void);
void	shutdown_prog	(void);
void	SrchCmhr		(char *);
void	UpdateCmcd		(void);
void	UpdateStat		(char *);
void	Update			(void);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc, 
	char 	*argv [])
{
	int		x			 = 0, 
			envCmAutoCon = FALSE;

	char	*sptr;

	TruePosition	=	TRUE;
	sprintf (envLogname, "%-14.14s", getenv ("LOGNAME"));
	SETUP_SCR (vars);

	_win_func = TRUE;

	sptr = chk_env ("SO_NUMBERS");
	envSoNumbers = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	/*
	 * Read common terminal record . 
	 */
	OpenDB ();

	/*
	 * Check contract number level. 
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? 2 : atoi (sptr);
	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	/*
	 * setup required parameters. 
	 */
	init_scr 	();				/*  sets terminal from termcap	*/
	set_tty 	();
	set_masks 	();				/*  setup print using masks	*/

	init_vars (SCN_SELECT);		/*  set default values		*/
	init_vars (SCN_INVOICE);	/*  set default values		*/
	init_vars (SCN_DETAILS);	/*  set default values		*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate 	= TodaysDate ();
	local_rec.invoiceDate  	= TodaysDate ();

	clear ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		init_ok 	= TRUE;
		eoi_ok 		= FALSE;
		restart 	= FALSE;
		lcount [SCN_INVOICE] = 0;

		init_vars (SCN_SELECT);
		heading (SCN_SELECT);
		entry (SCN_SELECT);
		if (prog_exit || restart)
			continue;

		heading (SCN_SELECT);
		scn_display (SCN_SELECT);
		edit (SCN_SELECT);
		if (restart)
			continue;

		init_ok 	= FALSE;
		eoi_ok 		= FALSE;
		restart 	= FALSE;
		entry_exit 	= FALSE;

		dsp_screen ("Loading Invoices", comm_rec.co_no, comm_rec.co_name);
		LoadInvoices ();
#ifdef GVISION
		dsp_screen_close ();
		snorm ();
#endif

		if (maxInvScn == 0)
		{
			/*
			 * No Valid Invoices loaded. 
			 */
			print_mess (ML (mlCmMess003));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}

		curInvScn = 0;

		getval (curInvScn);

		while (prog_exit == 0)
		{
			prog_status = EDIT;
			heading (SCN_INVOICE);

			SetFields ();

			scn_display (SCN_INVOICE);
			edit (SCN_INVOICE);

			if (restart)
			{
				/*
				 * Are You Sure That You Wish To Abort [Y/N] ? 
				 */
				x = prmptmsg (ML (mlCmMess117), "YNny", 20, 23);
				move (20, 23);
				cl_line ();
				if (x == 'N' || x == 'n')
				{
					restart = FALSE;
					prog_exit = FALSE;
					continue;
				}
				else
					break;
			}
			
			putval (curInvScn);
			Update ();
			break;
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence . 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files  
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (cohr2, cohr);

	open_rec (cumr , cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_id_no2");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_id_no");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_hhhr_hash");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

}
/*
 * Close data base files . 
 */
void
CloseDB (void)
{
	abc_fclose (cmcb);
	abc_fclose (cumr);
	abc_fclose (cmhr);
	abc_fclose (cmcd);
	abc_fclose (cmts);
	abc_fclose (coln);
	abc_fclose (cohr);
	abc_fclose (cohr2);
	abc_fclose (inmr);
	abc_fclose (cmtr);
	abc_fclose (cmcm);
	abc_fclose (ccmr);
	abc_fclose (cmpb);
	abc_fclose (cudp);
	abc_fclose (esmr);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("invoiceAmount"))
	{
		if (local_rec.invoiceAmount > cmhr_rec.quote_val)
		{
			/*
			 * Invoice Amt > Quoted Value 
			 */
			print_mess (ML (mlCmMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.invoiceAmount > local_rec.remainingAmount)
		{
			/*
			 * Invoice Amt > Remaining Value 
			 */
			print_mess (ML (mlCmMess005));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("tag"))
	{
		if (local_rec.tagged [0] == 'Y')
		 	strcpy (local_rec.taggedDesc, ML ("Yes"));
		else
		 	strcpy (local_rec.taggedDesc, ML ("No "));
		
		DSP_FLD ("tag_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startContNo"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startContNo, "%-6.6s", " ");
			strcpy (local_rec.startContDesc, ML ("First Contract"));
			DSP_FLD ("startContNo");
			DSP_FLD ("first_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.startContNo, zero_pad (local_rec.startContNo, 6));

		if (prog_status != ENTRY && 
			(strcmp (local_rec.startContNo, local_rec.endContNo) > 0))
		{
			/*
			 * Invalid range 
			 */
			print_mess (ML (mlCmMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		strcpy (cmhr_rec.cont_no, zero_pad (local_rec.startContNo, 6));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Contract Not Available For Invoicing 
			 */
			print_mess (ML (mlCmMess007));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.startContDesc, ML ("No Description Found"));
		else
			strcpy (local_rec.startContDesc, cmcd_rec.text);

		DSP_FLD ("first_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("last_cont"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.endContNo, "%-6.6s", "~~~~~~");
			strcpy (local_rec.endContDesc, ML ("Last Contract"));
			DSP_FLD ("last_cont");
			DSP_FLD ("last_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.endContNo, zero_pad (local_rec.endContNo, 6));
		if (strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
		{
			/*
			 * Invalid range 
			 */
			print_mess (ML (mlCmMess008));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		strcpy (cmhr_rec.cont_no, local_rec.endContNo);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Contract Not Available For Invoicing 
			 */
			print_mess (ML (mlCmMess007));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.endContDesc, ML ("No Description Found"));
		else
			strcpy (local_rec.endContDesc, cmcd_rec.text);

		DSP_FLD ("last_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
LoadInvoices (void)
{
	init_vars (SCN_INVOICE);	
	scn_set   (SCN_INVOICE);

	lcount [SCN_INVOICE] = 0;
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	strcpy (cmhr_rec.cont_no, local_rec.startContNo);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
			strcmp (cmhr_rec.cont_no, LAST_CONT) < 1)
	{
		if (strcmp (cmhr_rec.br_no, branchNo) > 0)
				break;
		/*
		 * Contract is not equal to O(pen or B(illing
		 */
		if (cmhr_rec.status [0] != 'O' && cmhr_rec.status [0] != 'B')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Contract is open.
		 */
		if (cmhr_rec.status [0] == 'O')
		{
			/*
			 * Contract is open not progressive billing not allowed.
			 */
			if (cmhr_rec.progress [0] != 'Y')
			{
				cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
				continue;
			}
		}
		dsp_process ("Contract No #", cmhr_rec.cont_no);
	 	strcpy (local_rec.tagged, "N");
	 	strcpy (local_rec.taggedDesc, ML ("No "));
		local_rec.invoiceAmount = 0.00;

		LoadFields ();

		putval (lcount [SCN_INVOICE]++);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}
	maxInvScn = lcount [SCN_INVOICE];
	return;
}

int
use_window (
	int		key)
{
	if (cur_screen != SCN_INVOICE)
		return (key);
	
	if (maxInvScn == 0)
	{
		/*
		 * No Invoices Loaded 
		 */
		print_mess (ML (mlCmMess009));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	putval (curInvScn);

	if (key == FN14 && ((curInvScn + 1) < maxInvScn))
	{
		curInvScn++;
		getval (curInvScn);
		SetFields ();
		scn_display (SCN_INVOICE);
		return (key);
	}
	if (key == FN15 && curInvScn > 0)
	{
		curInvScn--;
		getval (curInvScn);
		SetFields ();
		scn_display (SCN_INVOICE);
		return (key);
	}
	putchar (BELL);
	return (key);
}

void
SrchCmhr (
	char	*keyValue)
{
	_work_open (6, 0, 40);
	save_rec ("#No", "#Customer Order No.");
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);

	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
				   !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		if (strcmp (cmhr_rec.br_no, branchNo) > 0)
				break;
		/*
		 * Contract is not equal to O(pen or B(illing
		 */
		if (cmhr_rec.status [0] != 'O' && cmhr_rec.status [0] != 'B')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Contract is open not progressive billing not allowed.
		 */
		if (cmhr_rec.status [0] == 'O')
		{
			if (cmhr_rec.progress [0] != 'Y')
			{
				cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
				continue;
			}
		}
		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
LoadFields (void)
{
	int		count;

	cumr_rec.hhcu_hash	= cmhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
	
	if (cumr_rec.ho_dbt_hash)
	{
		cumr2_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
		cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (cumr2_rec.dbt_no, "000000");
			sprintf (cumr2_rec.dbt_name, "%-s", ML ("No Master Account"));
		}
	}
	else
	{
		strcpy (cumr2_rec.dbt_no, "000000");
		sprintf (cumr2_rec.dbt_name, "%-s", ML ("No Master Account"));
	}
	/* 
	 * Read contract descriptions.
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	for (count = 0; count < 7; count ++)
	{
		cmcd_rec.line_no = count;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
		if (!cc)
			strcpy (local_rec.desc [count], cmcd_rec.text);
		else
			sprintf (local_rec.desc [count], "%-70.70s", " ");
	}

}

int
win_function (
	int		field, 
	int		line, 
	int		scn, 
	int		scn_mode)
{

	int	save_status;

	if (scn != SCN_INVOICE)
		return (EXIT_SUCCESS);
	
	putval (curInvScn);
	save_status = prog_status;
	_win_func 	= FALSE;
	init_ok 	= TRUE;
	restart 	= FALSE;

	init_vars (SCN_DETAILS);
	heading (SCN_DETAILS);
	LoadCmcds ();

	edit (SCN_DETAILS);
	if (!restart)
		UpdateCmcd ();

	restart 	= FALSE;
	_win_func 	= TRUE;
	prog_status = save_status;

	getval (curInvScn);
	heading (SCN_INVOICE);
	scn_display (SCN_INVOICE);
	return (EXIT_SUCCESS);
}

void
LoadCmcds (void)
{
	int	count = 0;
	int	reply = 'N';

	/*
	 * Load invoice detail narrative.
	 */
	strcpy (cmcd_rec.stat_flag, "I");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	if (!cc && cmcd_rec.stat_flag [0] == 'I' && 
		cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		/*
		 * Do You Wish To Load Existing Charge Details [Y/N] ? 
		 */
		reply = prmptmsg (ML (mlCmMess110), "YNny", 10, 13);

		if (reply == 'Y' || reply == 'y')
		{
			while (!cc && cmcd_rec.stat_flag [0] == 'I' && 
				cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
			{
				strcpy (local_rec.invoiceDetails, cmcd_rec.text);
				putval (count++);
				cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
			}
		}
	}
	if (reply == 'N' || reply == 'n')
	{
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 7;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");

		while (!cc && cmcd_rec.stat_flag [0] == 'D' && 
			cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			strcpy (local_rec.invoiceDetails, cmcd_rec.text);
			putval (count++);
			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}
	}
}

void
UpdateCmcd (void)
{
	int 	count;
	char	*sptr;

	print_at (0, 0, ML (mlStdMess035));

	scn_set (SCN_DETAILS);

	for (count = 0; count < 3; count++)
	{
		getval (count);
		/*
		 * ignore if blank line 
		 */
		sptr = clip (local_rec.invoiceDetails);
		if (strlen (sptr) == 0)
			continue;

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;

		strcpy (cmcd_rec.stat_flag, "I");
		cmcd_rec.line_no = count;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "u");
		strcpy (cmcd_rec.text, local_rec.invoiceDetails);
		if (cc)
			cc = abc_add (cmcd, &cmcd_rec);
		else
			cc = abc_update (cmcd, &cmcd_rec);

		if (cc)
			file_err (cc, cmcd, "DBADD/DBUPDATE");
	}
}

void
SetFields (void)
{
	double	amtCharged = 0.00;

	FLD ("invoiceAmount") = NA;
	FLD ("invoiceDate") = NA;

	if (PROGRESS && IS_FIXED)
	{
		cmpb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cc = find_rec (cmpb, &cmpb_rec, GTEQ, "r");
		while (!cc && cmpb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			amtCharged += cmpb_rec.amount;
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
		}

		local_rec.invoiceAmount 	= cmhr_rec.quote_val - amtCharged;
		local_rec.remainingAmount 	= local_rec.invoiceAmount;

		/*
		 * Fixed Quote 
		 */
		print_at (2, 2, ML (mlCmMess047));
		DSP_FLD ("invoiceAmount");

		/*
		 * Progressive Billing 
		 */
		print_at (2, 22, ML (mlCmMess048));

		if (cmhr_rec.internal [0] == 'Y')
			print_at (2, 45, ML (mlCmMess049));
		else
			print_at (2, 45, ML (mlCmMess050));
	}

	/*
	 * If quote type is V(ariable and P(rogressive billing and O(pen contract
	 */
	if (cmhr_rec.quote_type [0] == 'V' &&
	     cmhr_rec.progress [0] == 'Y' && 
	     cmhr_rec.status [0] == 'O')
	{
		print_at (2, 2, ML (mlCmMess051));
		FLD ("invoiceAmount") = NA;
		DSP_FLD ("invoiceAmount");

		print_at (2, 22, ML (mlCmMess048));
		FLD ("invoiceDate") = YES;
		DSP_FLD ("invoiceDate");

		if (cmhr_rec.internal [0] == 'Y')
			print_at (2, 45, ML (mlCmMess049));
		else
			print_at (2, 45, ML (mlCmMess050));
		
		return;
	}

	/*
	 * If quote type is F(ixed and P(rogressive billing and O(pen contract
	 */
	if (cmhr_rec.quote_type [0] == 'F' &&
	     cmhr_rec.progress [0] == 'Y' && 
	     cmhr_rec.status [0] == 'O')
	{
		print_at (2, 2, ML (mlCmMess047));
		FLD ("invoiceAmount") = YES;
		DSP_FLD ("invoiceAmount");
		print_at (2, 22, ML (mlCmMess048));
		FLD ("invoiceDate") = YES;
		DSP_FLD ("invoiceDate");

		if (cmhr_rec.internal [0] == 'Y')
			print_at (2, 45, ML (mlCmMess049));
		else
			print_at (2, 45, ML (mlCmMess050));

		return;
	}

	/*
	 * If quote type is F(ixed and NOT P(rogressive billing 
	 */
	if (cmhr_rec.quote_type [0] == 'F' && cmhr_rec.progress [0] != 'Y')
	{
		local_rec.invoiceAmount = cmhr_rec.quote_val;
		print_at (2, 2, ML (mlCmMess047));
		print_at (2, 22, ML (mlCmMess052));
		DSP_FLD ("invoiceAmount");
	}
	
	/*
	 * If quote type is V(ariable and NOT P(rogressive billing 
	 */
	if (cmhr_rec.quote_type [0] == 'V' && cmhr_rec.progress [0] != 'Y')
	{
		local_rec.invoiceAmount = 0.00;
		print_at (2, 2, ML (mlCmMess051));
		print_at (2, 22, ML (mlCmMess052));
		DSP_FLD ("invoiceAmount");
	}
	if (cmhr_rec.internal [0] == 'Y')
		print_at (2, 45, ML (mlCmMess049));
	else
		print_at (2, 45, ML (mlCmMess050));

}

void
Update (void)
{
	int		count	= 0,
			i		= 0,
			len 	= 8;

	char	tmpPrefix	[3],
			tmpInvNo	[9],
			tmpMask		[12];

	long	invoiceNo	=	0L;

	print_mess ("Processing Invoices");

	abc_selfield (cmhr, "cmhr_hhhr_hash");

	scn_set (SCN_INVOICE);

	for (count = 0; count < lcount [SCN_INVOICE]; count++)
	{
		getval (count);
		/*
		 * Line not tagged for update.
		 */
		if (local_rec.tagged [0] != 'Y')
			continue;

		counter = 0;

		/*
		 * read the contract 
		 */
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, cmhr, "DBFIND");

		/*
		 * Add All the Header Stuff	
		 */
		cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		if (cumr_rec.ho_dbt_hash)
		{
			cumr_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND");
		}
		
		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cumr_rec.crd_prd, p_terms [i]._pcode, 3))
			{
				sprintf (local_rec.terms, "%-40.40s", p_terms [i]._pterm);
				break;
			}
		}
		/*
		 * Is invoice number to come from department of branch. 
		 */
		if (envSoNumbers == BY_DEPART)
		{
			strcpy (cudp_rec.co_no, comm_rec.co_no);
			strcpy (cudp_rec.br_no, comm_rec.est_no);
			strcpy (cudp_rec.dp_no, cumr_rec.department);
			cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
			if (cc)
			file_err (cc, cudp, "DBFIND");

			invoiceNo	=	(CASH) ? cudp_rec.nx_csh_no : cudp_rec.nx_chg_no;
			invoiceNo++;
		}
		else
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, esmr, "DBFIND");
		
			invoiceNo	=	(CASH) ? esmr_rec.nx_csh_inv : esmr_rec.nx_inv_no;
			invoiceNo++;
		}

		if (envSoNumbers == BY_BRANCH)
		{
			if (CASH)
				strcpy (tmpPrefix, esmr_rec.csh_pref);
			else
				strcpy (tmpPrefix, esmr_rec.chg_pref);
		}
		else
		{
			if (CASH)
				strcpy (tmpPrefix, cudp_rec.csh_pref);
			else
				strcpy (tmpPrefix, cudp_rec.chg_pref);
		}

		clip (tmpPrefix);
		len = strlen (tmpPrefix);

		sprintf (tmpMask, "%%s%%0%dld", 8 - len);
		sprintf (tmpInvNo, tmpMask, tmpPrefix, invoiceNo);

		/*
		 * Check if Invoice / Credit Note No Already
		 * Allocated. If it has been then skip	
		 */
		while (CheckCohr (tmpInvNo) == 0)
			sprintf (tmpInvNo, tmpMask, tmpPrefix, invoiceNo++);

		if (envSoNumbers == BY_DEPART)
		{
			if (CASH)
				cudp_rec.nx_csh_no	=	invoiceNo;
			else
				cudp_rec.nx_chg_no	=	invoiceNo;

			cc = abc_update (cudp, &cudp_rec);
			if (cc)
				file_err (cc, cudp, "DBUPDATE");
		}
		else
		{
			if (CASH)
				esmr_rec.nx_csh_inv	=	invoiceNo;
			else
				esmr_rec.nx_inv_no	=	invoiceNo;

			cc = abc_update (esmr, &esmr_rec);
			if (cc)
				file_err (cc, esmr, "DBUPDATE");
		}
		sprintf (cohr_rec.inv_no, "%-8.8s", tmpInvNo);

		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cohr_rec.type, "I");
		strcpy (cohr_rec.op_id, envLogname);
		cohr_rec.date_create = TodaysDate ();
		strcpy (cohr_rec.time_create, TimeHHMM ());
		sprintf (cohr_rec.cus_ord_ref, "%-20.20s", cmhr_rec.cus_ref);
		cohr_rec.date_raised 	= local_rec.lsystemDate;
		cohr_rec.date_required 	= local_rec.lsystemDate;
		cohr_rec.gross 			= 0.00;
		cohr_rec.disc 			= 0.00;
		cohr_rec.gst 			= 0.00;
		strcpy (cohr_rec.dl_name, cmhr_rec.contact);
		strcpy (cohr_rec.dl_add1, cmhr_rec.adr1);
		strcpy (cohr_rec.dl_add2, cmhr_rec.adr2);
		strcpy (cohr_rec.dl_add3, cmhr_rec.adr3);
		strcpy (cohr_rec.pay_terms, local_rec.terms);
		strcpy (cohr_rec.status, "C");
		strcpy (cohr_rec.stat_flag, "C");
		strcpy (cohr_rec.inv_print, "N");
		strcpy (cohr_rec.frei_req, "N");
		strcpy (cohr_rec.tax_code, cumr_rec.tax_code);
		strcpy (cohr_rec.tax_no, cumr_rec.tax_no);
		strcpy (cohr_rec.area_code, cumr_rec.area_code);
		strcpy (cohr_rec.sale_code, cumr_rec.sman_code);
		strcpy (cohr_rec.fix_exch, "Y");
		strcpy (cohr_rec.pri_type, cumr_rec.price_type);
		strcpy (cohr_rec.ord_type, "D");
		strcpy (cohr_rec.ps_print, "N");
		strcpy (cohr_rec.prt_price, "Y");

		cc = abc_add (cohr, &cohr_rec);
		if (cc)
			file_err (cc, cohr, "DBADD");

		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, cohr, "DBFIND");

		cmcd_rec.hhhr_hash 		= cmhr_rec.hhhr_hash;

		keepCohr 	= FALSE;
		colnAdded 	= FALSE;

		ProcessCmhr ();

		if (keepCohr)
		{
			cc = abc_update (cohr, &cohr_rec);
			if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}
		else
		{
			cc = abc_delete (cohr);
			if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}
		/*
		 * Contract was B(illing so change to C(losed
		 */
		if (cmhr_rec.status [0] == 'B')
		{
			cmhr_rec.end_date = local_rec.lsystemDate;
			strcpy (cmhr_rec.status, "C");
		}
		
		cc = abc_update (cmhr, &cmhr_rec);
		if (cc)
			file_err (cc, cmhr, "DBUPDATE");

		/*
		 * create cmpb 
		 */
		cmpb_rec.inv_date  = local_rec.invoiceDate;
		cmpb_rec.date      = local_rec.lsystemDate;
		cmpb_rec.hhco_hash = cohr_rec.hhco_hash;
		cmpb_rec.amount    = cohr_rec.gross - cohr_rec.disc;
		cmpb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cc = abc_add (cmpb, &cmpb_rec);
		if (cc)
			file_err (cc, cmpb, "DBADD");
	}
	abc_selfield (cmhr, "cmhr_id_no2");
	clear_mess ();
}

int
CheckCohr (
	char	*invoiceNo)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type, "I");
	sprintf (cohr2_rec.inv_no, "%-8.8s", invoiceNo);

	return (find_rec (cohr2, &cohr2_rec, COMPARISON, "r"));
}

void
ProcessCmhr (void)
{
	int		firstTime 	= TRUE;

	float	qty		  	= 0.00,
			tmp		 	= 0.00;

	double	lastValue 	= 0.00,
			values [2]	= {0.00, 0.00};

	/*
	 * reset values 
	 */
	values [COST] 	= 0.00;
	values [SALE] 	= 0.00;
	qty		= 0.00;

	/*
	 * loop through contract/cost heads details (cmcb).
	 */
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cc = find_rec (cmcb, &cmcb_rec, GTEQ, "u");
	while (!cc && cmcb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		values [COST] 	= 0.00;
		values [SALE] 	= 0.00;
		qty		= 0.00;
		tmp		= 0.00;

		/*
		 * Get cost head master file.
		 */
		cmcm_rec.hhcm_hash = cmcb_rec.hhcm_hash;
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmcm, "DBFIND");

		/*
		 * Read through time sheet transaction file. (cmts)
		 */
		cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cmts_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		cmts_rec.sale = 0.00;

		cc = find_rec (cmts, &cmts_rec, GTEQ, "u");
		while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
					   cmts_rec.hhcm_hash == cmcm_rec.hhcm_hash)
		{
			/*
			 * A status of 'P' indicates transaction posted.
			 */
			if (cmts_rec.stat_flag [0] != ' ')
			{
				abc_unlock (cmts);
				cc = find_rec (cmts, &cmts_rec, NEXT, "u");
				continue;
			}
			/*
			 * Check date is less than invoice date.
			 */
			if (CheckOK ("cmts"))
				continue;

			/*
			 * will add one coln after reading cmtr
			 */
			if (IS_ALL)
			{
				tmp	= (float) (cmts_rec.time_ord +
					  		  (cmts_rec.time_hlf * 1.5) +
					  		  (cmts_rec.time_dbl * 2.0) + cmts_rec.units);

				qty		+= tmp;
				values [COST] 	+= (cmts_rec.lab_cost + cmts_rec.oh_cost) * tmp;
				values [SALE] 	+= cmts_rec.sale * tmp;
			
				UpdateStat ("cmts");
			}

			if (IS_NONE)
			{
				qty	= (float) (cmts_rec.time_ord +
					  		  (cmts_rec.time_hlf * 1.5) +
					  		  (cmts_rec.time_dbl * 2.0) + cmts_rec.units);
			
				/*
				 * get emp name 
				 */
				AddColn (FALSE, cmts_rec.hheq_hash ? 
					   		"Plant" : "Labour ", 
					  		(cmts_rec.lab_cost + 
					  		cmts_rec.oh_cost) * qty, 
					  		cmts_rec.sale * qty, 
					  		qty, 1.00);

				UpdateStat ("cmts");
			}
	
			if (IS_LIKE)
			{
				if (firstTime)
					lastValue = cmts_rec.sale;

				if (lastValue != cmts_rec.sale)
				{
					/*
					 * Add one coln for plant and labour costs.
					 */
					AddColn 
					(
						FALSE, 
						"Plant/Labour ", 
						values [COST], 
						values [SALE], 
						qty, 
						lastValue
					);

					/*
					 * reset values 
					 */
					values [COST] 	= 0.00;
					values [SALE] 	= 0.00;
					qty		= 0.00;
					lastValue 	= cmts_rec.sale;
				}

				tmp	= (float) (cmts_rec.time_ord +
					  		  (cmts_rec.time_hlf * 1.5) +
					  		  (cmts_rec.time_dbl * 2.0) + cmts_rec.units);

				qty		+= tmp;

				values [COST] 	+= (cmts_rec.lab_cost + cmts_rec.oh_cost) * tmp;
				values [SALE] 	+= cmts_rec.sale * tmp;
			
				firstTime = FALSE;
				UpdateStat ("cmts");
			}
		}

		abc_unlock (cmts);

		if (!firstTime && IS_LIKE)
		{
			AddColn 
			(
				FALSE, 
				"Plant/Labour ", 
				values [COST], 
				values [SALE], 
				qty, 
				lastValue
			);
			/*
			 * reset values 
			 */
			values [COST] 	= 0.00;
			values [SALE] 	= 0.00;
			qty				= 0.00;
			firstTime 		= TRUE;
		}

		/*
		 * read thru' cmtr 
		 */
		cmtr_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cmtr_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		cmtr_rec.sale_price = -99999999999999.99;
		cmtr_rec.disc_pc = (float) -99999.99;

		cc = find_rec (cmtr, &cmtr_rec, GTEQ, "u");
		while (!cc && 
			cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
			cmtr_rec.hhcm_hash == cmcm_rec.hhcm_hash)
		{
			if (cmtr_rec.stat_flag [0] != ' ')
			{
				abc_unlock (cmtr);
				cc = find_rec (cmtr, &cmtr_rec, NEXT, "u");
				continue;
			}
			if (CheckOK ("cmtr"))
				continue;

			if (IS_ALL)
			/*
			 * will add one coln when out of loop 
			 */
			{
				/*
				 * b'cos may have different discount will pass as net
				 */
				qty 			+= cmtr_rec.qty;
				values [COST] 	+= cmtr_rec.cost_price * cmtr_rec.qty;
				values [SALE] 	+= cmtr_rec.sale_price * cmtr_rec.qty *
						  ((100.00 - cmtr_rec.disc_pc) / 100.00);
				UpdateStat ("cmtr");
			}

			if (IS_NONE)
			{
				inmr_rec.hhbr_hash = cmtr_rec.hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr, "DBFIND");

				qty = cmtr_rec.qty;
				values [SALE]	= cmtr_rec.sale_price * 
								  cmtr_rec.qty * ((100.00 - 
					          	 cmtr_rec.disc_pc) / 100.00);

				AddColn 
				(
					FALSE, 
					inmr_rec.description, 
					cmtr_rec.cost_price * cmtr_rec.qty, 
					values [SALE], 
					qty, 
					1.00
				);
				UpdateStat ("cmtr");
			}
	
			if (IS_LIKE)
			{
				if (firstTime)
					lastValue = cmtr_rec.sale_price * ((100.00 - cmtr_rec.disc_pc) / 100.00);

				firstTime = FALSE;

				if (lastValue != cmtr_rec.sale_price * ((100.00 - cmtr_rec.disc_pc) / 100.00))
				{
					sprintf (err_str, "Materials @ $ %.2f ", DOLLARS (lastValue));

					AddColn 
					(
						FALSE, 
						err_str, 
						values [COST] * qty, 
						values [SALE], 
						qty, 
						lastValue
					);

					/*
					 * reset values 
					 */
					values [COST] 	= 0.00;
					values [SALE] 	= 0.00;
					qty		= 0.00;
					lastValue = cmtr_rec.sale_price * ((100.00 - cmtr_rec.disc_pc) / 100.00);
				}

				qty		+= cmtr_rec.qty;
				values [COST] 	+= cmtr_rec.cost_price;
				values [SALE]	+= cmtr_rec.sale_price * 
					          cmtr_rec.qty * ((100.00 - 
					          cmtr_rec.disc_pc) / 100.00);
			
				UpdateStat ("cmtr");
			}
		}

		abc_unlock (cmtr);

		if (!firstTime && IS_LIKE)
		{
			sprintf (err_str, "Materials @ $ %.2f ",DOLLARS (lastValue));
			AddColn 
			(
				FALSE, 
				err_str, 
				values [COST] * qty, 
				values [SALE], 
				qty, 
				lastValue
			);

			/*
			 * reset values 
			 */
			values [COST] 	= 0.00;
			values [SALE] 	= 0.00;
			qty		= 0.00;
			firstTime 	= TRUE;
		}

		if (IS_ALL)
		{
			AddColn 
			(
				FALSE, 
				cmcm_rec.desc, 
				values [COST], 
				values [SALE], 
				qty, 
				1.00
			);

			/*
			 * reset values 
			 */
			values [COST] 	= 0.00;
			values [SALE] 	= 0.00;
			qty		= 0.00;
			firstTime 	= TRUE;
		}
		cc = find_rec (cmcb, &cmcb_rec, NEXT, "u");
	}
	/*
	 * if FIXED AddColn just in case no cmts/cmtr's 
	 * IF already added will make no difference     
	 */
	if (!colnAdded && IS_FIXED)
	{
		cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cc = find_rec (cmcb, &cmcb_rec, GTEQ, "u");
		if (cc)
			file_err (cc, cmcb, "DBFIND");

		AddColn 
		(
			TRUE, 
			"~~~~~", 
			999999.99, 
			999999.99, 
			1.00, 
			1.00
		);
	}
}

void
UpdateStat (
	char	*fileName)
{
	if (!strcmp (fileName, cmts))
	{
		strcpy (cmts_rec.stat_flag, "P");
		cc = abc_update (cmts, &cmts_rec);
		if (cc)
			file_err (cc, cmts, "DBUPDATE");

		cc = find_rec (cmts, &cmts_rec, NEXT, "u");
	}

	if (!strcmp (fileName, cmtr))
	{
		strcpy (cmtr_rec.stat_flag, "P");
		cc = abc_update (cmtr, &cmtr_rec);
		if (cc)
			file_err (cc, cmtr, "DBUPDATE");

		cc = find_rec (cmtr, &cmtr_rec, NEXT, "u");
	}
}

void
AddColn (
	int		NoCost, 
	char 	*item, 
	double	cost, 
	double	sale, 
	float	qty, 
	double	unitPrice)
{
	double tmp;

	if (sale == 0.00 && cost == 0.00 && qty == 0.00)
		return;

	if (IS_FIXED || IS_ALL)
		coln_rec.q_order = 1.00;
	else
	{
		coln_rec.q_order = qty;
		sale /= qty;
		cost /= qty;
	}

	coln_rec.disc_pc = 0.00;
	coln_rec.amt_disc = 0.00;

	if (!colnAdded)
	{
		keepCohr = TRUE;
		if (IS_FIXED)
			colnAdded = TRUE;

		coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
		coln_rec.line_no 	= counter++;
		coln_rec.hhbr_hash 	= cmcm_rec.hhbr_hash;
		coln_rec.incc_hash 	= ccmr_rec.hhcc_hash;

		if (NoCost)
			coln_rec.cost_price = 0.00;
		else
			coln_rec.cost_price = cost;
			
		if (IS_FIXED)
		{
			coln_rec.sale_price	= local_rec.invoiceAmount;
			coln_rec.gross 		= coln_rec.sale_price;
			sprintf (coln_rec.item_desc, "%-40.40s", "Quoted Price");
		}
		else
		{
			coln_rec.sale_price = sale;
			sprintf (coln_rec.item_desc, "%-40.40s", item);
			if (IS_LIKE || IS_NONE)
			    coln_rec.gross = coln_rec.sale_price * qty;
			else
			    coln_rec.gross = coln_rec.sale_price;
		}

		sprintf (coln_rec.cus_ord_ref, "%-20.20s", cmhr_rec.cus_ref);

		coln_rec.due_date = local_rec.lsystemDate;

		strcpy (coln_rec.sman_code, cumr_rec.sman_code);
		strcpy (coln_rec.bonus_flag, "N");
		strcpy (coln_rec.hide_flag, "N");
		strcpy (coln_rec.status, "C");
		strcpy (coln_rec.stat_flag, "C");
		coln_rec.hhum_hash = inmr_rec.std_uom;

		tmp = coln_rec.gross * ((100.00 - comr_rec.gst_rate) / 100.00);
				
		coln_rec.amt_gst = coln_rec.gross - tmp;
		cohr_rec.gst	+= coln_rec.amt_gst;
		cohr_rec.gross += coln_rec.gross;

		cc = abc_add (coln, &coln_rec);
		if (cc)
			file_err (cc, coln, "DBADD");
	}
	else
	{
		coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
		coln_rec.line_no 	= counter;

		/*
		 * Type Will Be FIXED update cost on coln 
		 */
		cc = find_rec (coln, &coln_rec, EQUAL, "u");
		if (cc)
			file_err (cc, coln, "DBFIND");

		coln_rec.cost_price += cost;
		coln_rec.hhum_hash = inmr_rec.std_uom;

		cc = abc_update (coln, &coln_rec);
		if (cc)
			file_err (cc, coln, "DBUPDATE");
	}

	if (IS_FIXED || IS_ALL)
	{
		cmcb_rec.sum_cost 	+= cost;
		cmcb_rec.sum_value 	+= sale;
	}
	else
	{
		cmcb_rec.sum_cost 	+= cost * qty;
		cmcb_rec.sum_value 	+= sale * qty;
	}

	cmcb_rec.sum_qty += qty;

	if (NoCost)
	{
		abc_unlock (cmcb);
	}
	else
	{
		cc = abc_update (cmcb, &cmcb_rec);
		if (cc)
			file_err (cc, cmcb, "DBUPDATE");
	}
}

int
CheckOK (
	char	*fileName)
{
	if (cmhr_rec.status [0] == 'O')
	{
		/*
		 * means progressive & if date > entered we do not update now
		 */
		if (!strcmp (fileName, cmts))
		{
			if (cmts_rec.date > local_rec.invoiceDate)
			{
				abc_unlock (cmts);
				cc = find_rec (cmts, &cmts_rec, NEXT, "u");
				return (EXIT_FAILURE); 
			}
			else 
				return (EXIT_SUCCESS);
		}
		else
		{
			if (cmtr_rec.date > local_rec.invoiceDate)
			{
				abc_unlock (cmtr);
				cc = find_rec (cmtr, &cmtr_rec, NEXT, "u");
				return (EXIT_FAILURE); 
			}
			else 
				return (EXIT_SUCCESS);
		}
	}
	else
		return (EXIT_SUCCESS);
}
/*
 * Print Heading. 
 */
int
heading (
	int		scn)
{
	if (!restart)
	{
		tab_row = (scn == SCN_DETAILS) ? 8 : 6;
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		switch (scn)
		{
		case	SCN_SELECT:
			/*
			 * Selection Of Contracts For Invoicing 
			 */
			swide ();
			line_at (1, 0, 132);
			centre_at (0, 132, ML (mlCmMess106)); 
			box (0, 3, 132, 2);
			break;

		case	SCN_INVOICE:
			/*
			 * Prepare Invoices For Contracts 
			 */
			swide ();
			centre_at (0, 132, ML (mlCmMess107));
			box (0, 3, 132, 14);
			centre_at (20, 132, ML (mlCmMess108)); 
			line_at (1, 0, 132);
			line_at (8, 1, 131);
			line_at (16, 1, 131);
			line_at (21, 1, 131);
			break;

		case	SCN_DETAILS:
			/*
			 * Charge Details 
			 */
			snorm ();
			line_at (1, 0, 80);
			line_at (7, 0, 80);
			centre_at (0, 80, ML (mlCmMess109)); 
			break;

		default:
			break;
		}

		line_cnt = 0;
		scn_write (scn);
	}

	if (scn != SCN_INVOICE)
		line_at (21, 0, (scn == 1) ? 132 : 80);
	
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}

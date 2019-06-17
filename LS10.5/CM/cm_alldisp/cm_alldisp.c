/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_alldisp.c,v 5.5 2002/07/17 09:56:57 scott Exp $
|  Program Name  : (cm_alldisp.c)
|  Program Desc  : (Contracts All Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (25/03/93)       |
|---------------------------------------------------------------------|
| $Log: cm_alldisp.c,v $
| Revision 5.5  2002/07/17 09:56:57  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/01/23 05:05:52  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_alldisp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_alldisp/cm_alldisp.c,v 5.5 2002/07/17 09:56:57 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<hot_keys.h>
#include 	<ring_menu.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>
#include	<tabdisp.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	IN_POP_A 1
#define	IN_POP_B 2

#define	CAL(amt, pc)	(no_dec (amt * DOLLARS (pc)))
#define	POP_X	 1
#define	POP_Y	 13


char	*underline = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

	extern int				lp_x_off;
	extern int				lp_y_off;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cmcbRecord	cmcb_rec;
struct cmcdRecord	cmcd_rec;
struct cmcmRecord	cmcm_rec;
struct cmemRecord	cmem_rec;
struct cmeqRecord	cmeq_rec;
struct cmhrRecord	cmhr_rec;
struct cmitRecord	cmit_rec;
struct cmjtRecord	cmjt_rec;
struct cmpbRecord	cmpb_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct cmtrRecord	cmtr_rec;
struct cmtsRecord	cmts_rec;
struct cmwsRecord	cmws_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct inmrRecord	inmr_rec;

	int		envCmAutoCon	= FALSE, 
			singleContract 	= FALSE, 
			noInTab 		= 0, 
			clearOK 		= TRUE, 
			mainWindowOpen	= 0, 
			popupSelect 	= IN_POP_A, 
			printAll 		= FALSE, 
			pipeOpen 		= FALSE, 
			nowPrinting 	= FALSE, 
			firstContract	= 0;

	FILE	*fout;

	char	branchNo [3], 
			*sptr;

	char	contractDesc [7][71];
	
	char	*cmcm2 	= "cmcm2", 
			*cmhr2 	= "cmhr2", 
			*cmit2 	= "cmit2", 
			*cmjt2 	= "cmjt2", 
			*cmrh2 	= "cmrh2", 
			*cumr2 	= "cumr2", 
			*data  	= "data";
/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy 				[11];
	char	endContDesc 		[71];
	char	endContNo 			[7];
	char	endCustomer 		[7];
	char	endCustomerName 	[41];
	char	endIssueDesc 		[41];
	char	endIssueTo 			[11];
	char	endJobDesc 			[41];
	char	endJobType 			[5];
	char	sort 				[2];
	char	startContDesc 		[71];
	char	startContNo 		[7];
	char	startCustomer 		[7];
	char	startCustomerName 	[41];
	char	startIssueDesc 		[41];
	char	startIssueTo 		[11];
	char	startJobDesc 		[41];
	char	startJobType 		[5];
	char	status 				[41];
	char	statusDesc 			[8];
	char	systemDate 			[11];
	int		printerNo;	
	long	lsystemDate;
} local_rec;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "contractNo", 	19, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Contract Number    ", "Enter Contract Number", 
		YES, NO, JUSTLEFT, "", "", local_rec.startContNo}, 

	{2, LIN, "startCustomer", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer     ", "Enter Customer Number , Full Search Available. ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.startCustomer}, 
	{2, LIN, "startCustomerName", 	 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.startCustomerName}, 
	{2, LIN, "endCustomer", 	 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Customer       ", "Enter Customer Number , Full Search Available. ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.endCustomer}, 
	{2, LIN, "endCustomerName", 	 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.endCustomerName}, 

	{2, LIN, "startContNo", 	7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Contract     ", "Enter Starting Contract Number", 
		YES, NO, JUSTLEFT, "", "", local_rec.startContNo}, 
	{2, LIN, "startContDesc", 	7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.startContDesc}, 
	{2, LIN, "endContNo", 	8, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Contract       ", "Enter End Contract Number", 
		YES, NO, JUSTLEFT, "", "", local_rec.endContNo}, 
	{2, LIN, "endContDesc", 8, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.endContDesc}, 

	{2, LIN, "startJobType", 	10, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Start Job Type     ", "Enter the starting job type", 
		YES, NO, JUSTLEFT, "", "", local_rec.startJobType}, 
	{2, LIN, "startJobDesc", 10, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.startJobDesc}, 
	{2, LIN, "endJobType", 	11, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "End Job Type       ", "Enter the ending job type", 
		YES, NO, JUSTLEFT, "", "", local_rec.endJobType}, 
	{2, LIN, "endJobDesc", 11, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.endJobDesc}, 

	{2, LIN, "startIssueTo", 	13, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Start Issue To     ", "Enter Start Issue To Code . ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startIssueTo}, 
	{2, LIN, "startIssueDesc", 	13, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startIssueDesc}, 
	{2, LIN, "endIssueTo", 	14, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "End Issue To       ", "Enter End Issue To Code . ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endIssueTo}, 
	{2, LIN, "endIssueDesc", 	14, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endIssueDesc}, 

	{2, LIN, "status", 	16, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Contract Status    ", "Enter Contract Status (O, X, B, C, H). Default is A(ll).", 
		YES, NO, JUSTRIGHT, "OXBCHA", "", local_rec.status}, 
	{2, LIN, "statusDesc", 	16, 35, CHARTYPE, 
		"AAAAAAAAAAAAA", "          ", 
		" ", "All    ", "", "", 
		NA, NO, JUSTRIGHT, "", "", local_rec.statusDesc}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

static	int	ContractDesc		(int, KEY_TAB *);
static	int	PrintAllCostSheets	(int, KEY_TAB *);
static	int	PrintStatus			(int, KEY_TAB *);
static	int	DisplayContract		(int, KEY_TAB *);

#ifndef GVISION
static	KEY_TAB slct_keys [] =
{
   { " [D]ISPLAY DESCRIPTION ", 	'D', 		ContractDesc, 
	"Display Contract Description.", 				"A" }, 
   { " [P]RINT COST SHEETS ", 	'P', 		PrintAllCostSheets, 
	"Display contract.", 						"A" }, 
   { " [FN5 - PRINT STATUS] ", 	FN5, 		PrintStatus, 
	"Print List Of Selected Contracts.", 				"A" }, 
   { NULL, 			'\r', 		DisplayContract, 
	"Display contract.", 						"A" }, 
   END_KEYS
};
#else
static	KEY_TAB slct_keys [] =
{
   { " DISPLAY DESCRIPTION ", 	'D', 		ContractDesc, 
	"Display Contract Description.", 				"A" }, 
   { " PRINT COST SHEETS ", 	'P', 		PrintAllCostSheets, 
	"Display contract.", 						"A" }, 
   { " [PRINT STATUS] ", 	FN5, 		PrintStatus, 
	"Print List Of Selected Contracts.", 				"A" }, 
   { NULL, 			'\r', 		DisplayContract, 
	"Display contract.", 						"A" }, 
   END_KEYS
};
#endif

int		SelectPopupA	(void);
int		SelectPopupB	(void);
int		BudgetDisplay	(void);
int		XDescDisp		(void);
int		TimeDisplay		(void);
int		MaterialDisplay	(void);
int		ReqDisplay		(void);
int		PrintCostSheet	(void);
int		RedrawDisplay	(void);
void	NextContract	(void);
void	PrevContract	(void);

#ifndef GVISION
menu_type	_main_menu [] = {
  {"<S1>", "Select display of Popup Window One. [1]", 
	SelectPopupA, "1", 	  }, 
  {"<S2>", "Select display of Popup Window One. [2]", 
	SelectPopupB, "2", 	  }, 
  {"<Budget>", "Display Costhead Budgets. [B]", 
	BudgetDisplay, "Bb", 	  }, 
  {"<Description>", "Display Contract Ongoing Description. [D]", 
	XDescDisp, "Dd", 	  }, 
  {"<Employee/Plant Time>", "Display Employee/Plant Time For Contract. [T]", 
	TimeDisplay, "Tt", 	  }, 
  {"<Materials>", "Display Materials Used For Contract. [M]", 
	MaterialDisplay, "Mm", 	  }, 
  {"<Requisitions>", "Display Requisitions For Contract. [R]", 
	ReqDisplay, "Rr", 	  }, 
  {"<Print Cost Sheet>", "Print Costsheet For Current Contract. [P]", 
	PrintCostSheet, "Pp", 	  }, 
  {"<FN03>", "Redraw Display", RedrawDisplay, "", FN3, 			  }, 
  {"<FN16>", "Exit Display", _no_option, "", FN16, EXIT | SELECT	  }, 
  {"", 									  }, 
};
#else
menu_type	_main_menu [] = {
  {0, "<S1>", "", 					SelectPopupA, }, 
  {0, "<S2>", "", 					SelectPopupB, }, 
  {0, "<Budget>", "", 				BudgetDisplay, }, 
  {0, "<Description>", "", 			XDescDisp, }, 
  {0, "<Employee/Plant Time>", "", 	TimeDisplay, }, 
  {0, "<Materials>", "", 			MaterialDisplay, }, 
  {0, "<Requisitions>", "", 		ReqDisplay, }, 
  {0, "<Print Cost Sheet>", "", 	PrintCostSheet, }, 
  {0, "<FN03>", "Redraw Display", 	RedrawDisplay, FN3, }, 
  {0, "", 									  }, 
};
#endif

/*
 * Function prototypes.      
 */
int		heading				(int);
int		ProcessRange		(void);
int		ReqDetails			(char *);
int		spec_valid			(int);
int		SrchCmhr			(char *);
int		SrchCmjt			(char *);
void	AllDisplay			(void);
void	BudgetHead			(int);
void	CalculateProfit		(long, double *, double *, char *);
void	CloseDB		 		(void);
void	MaterialHead		(int);
void	OpenDB				(void);
void	PrintMissGraphics	(void);
void	PrintPopup			(int);
void	ReDraw				(void);
void	shutdown_prog		(void);
void	SrchCmit			(char *);
void	TimeHeader			(int);
void	XDescHeader			(void);

#include 	<get_lpno.h>
#include	<FindCumr.h>

/*
 * Main Processing Routine . 
 */
int
main (
 int	argc, 
 char * argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, ML (mlCmMess728), argv [0]);

		return (argc);
	}
	
	TruePosition	=	TRUE;
	/*
	 * Multiple OR Single contract (s). 
	 */
	singleContract = (*argv [1] == 'S');

	/*
	 * Check environment. 
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	OpenDB ();
	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*
	 * Prepare screen. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	input_row = 19;

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		restart 	= FALSE;
		search_ok 	= TRUE;

		if (singleContract)
		{
			init_vars (1);	
			clearOK = TRUE;
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;
			clearOK = FALSE;

			/*
			 * Display Contract. 
			 */
			DisplayContract (0, (KEY_TAB *) NULL);
		}
		else
		{
			init_vars (2);	
			clearOK = TRUE;
			heading (2);
			entry (2);
			if (prog_exit || restart)
				continue;

			heading (2);
			scn_display (2);
			edit (2);
			if (restart)
				continue;
	
			clearOK = FALSE;
			ProcessRange ();
		}

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cmcm2, cmcm);
	abc_alias (cmhr2, cmhr);
	abc_alias (cmit2, cmit);
	abc_alias (cmjt2, cmjt);
	abc_alias (cmrh2, cmrh);
	abc_alias (cumr2, cumr);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmcm2,cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmem, cmem_list, CMEM_NO_FIELDS, "cmem_hhem_hash");
	open_rec (cmeq, cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2,cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmit2,cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2,cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (cmrh, cmrh_list, CMRH_NO_FIELDS, "cmrh_hhhr_hash");
	open_rec (cmrh2,cmrh_list, CMRH_NO_FIELDS, "cmrh_hhrq_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_id_no2");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_id_no3");
	open_rec (cmws, cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close data base files  
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmit);
	abc_fclose (cmit2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cumr);
	abc_fclose (cmpb);
	abc_fclose (cmrd);
	abc_fclose (cmrh);
	abc_fclose (cmrh2);
	abc_fclose (cmtr);
	abc_fclose (cmts);
	abc_fclose (cmws);
	abc_fclose (cumr2);
	abc_fclose (inmr);
	abc_dbclose (data);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
	int		field)
{
/*
 * Screen ONE Validation. 
 */
	/*
	 * Validate Contract Number. 
	 */
	if (LCHECK ("contractNo"))
	{
		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);	
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		strcpy (cmhr_rec.cont_no, zero_pad (local_rec.startContNo, 6));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

/*
 * Screen TWO Validation. 
 */
	/*
	 * Validate Customer Range. 
	 */
	if (LCHECK ("startCustomer"))
	{
		if (prog_status == ENTRY && last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.startCustomer, "      ");
			strcpy (local_rec.startCustomerName, ML ("First Customer"));
			DSP_FLD ("startCustomer");
			DSP_FLD ("startCustomerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", zero_pad (local_rec.startCustomer, 6));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startCustomerName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("startCustomerName");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endCustomer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCustomer, "~~~~~~");
			strcpy (local_rec.endCustomerName, ML ("Last Customer"));
			DSP_FLD ("endCustomer");
			DSP_FLD ("endCustomerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", zero_pad (local_rec.endCustomer, 6));
		cc = find_rec (cumr, &cumr_rec, EQUAL , "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.endCustomerName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("endCustomerName");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Contract Range. 
	 */
	if (LCHECK ("startContNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startContNo, "      ");
			strcpy (local_rec.startContDesc, ML ("First Contract"));
			DSP_FLD ("startContNo");
			DSP_FLD ("startContDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);	
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		strcpy (cmhr_rec.cont_no, zero_pad (local_rec.startContNo, 6));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cmcd_rec.text, "%-70.70s", " ");

		sprintf (local_rec.startContDesc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("startContDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endContNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endContNo, "~~~~~~");
			strcpy (local_rec.endContDesc, ML ("Last Contract"));
			DSP_FLD ("endContNo");
			DSP_FLD ("endContDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);	
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		strcpy (cmhr_rec.cont_no, zero_pad (local_rec.endContNo, 6));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cmcd_rec.text, "%-70.70s", " ");

		sprintf (local_rec.endContDesc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("endContDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Job Type Range. 
	 */
	if (LCHECK ("startJobType"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startJobType, "    ");
			strcpy (local_rec.startJobDesc, ML ("First Job Type"));
			DSP_FLD ("startJobDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);	
			return (EXIT_SUCCESS);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		strcpy (cmjt_rec.job_type, local_rec.startJobType);
		cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess088));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.startJobType, local_rec.endJobType) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startJobDesc, cmjt_rec.desc);
		DSP_FLD ("startJobDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endJobType"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endJobType, "~~~~");
			strcpy (local_rec.endJobDesc, ML ("Last Job Type"));
			DSP_FLD ("endJobDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);	
			return (EXIT_SUCCESS);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		strcpy (cmjt_rec.job_type, local_rec.endJobType);
		cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess088));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startJobType, local_rec.endJobType) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endJobDesc, cmjt_rec.desc);
		DSP_FLD ("endJobDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Issue To Range. 
	 */
	if (LCHECK ("startIssueTo"))
	{
		if (FLD ("startIssueTo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startIssueTo, "          ");
			strcpy (local_rec.startIssueDesc, ML ("First Issue To Code"));
			DSP_FLD ("startIssueTo");
			DSP_FLD ("startIssueDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmit_rec.co_no, comm_rec.co_no);
		sprintf (cmit_rec.issto, "%-10.10s", local_rec.startIssueTo);
		cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess014));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.startIssueDesc, "%-40.40s", cmit_rec.iss_name);
		DSP_FLD ("startIssueDesc");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endIssueTo"))
	{
		if (FLD ("endIssueTo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endIssueTo, "~~~~~~~~~~");
			strcpy (local_rec.endIssueDesc, ML ("Last Issue To Code"));
			DSP_FLD ("endIssueTo");
			DSP_FLD ("endIssueDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmit_rec.co_no, comm_rec.co_no);
		sprintf (cmit_rec.issto, "%-10.10s", local_rec.endIssueTo);
		cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess014));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.endIssueDesc, "%-40.40s", cmit_rec.iss_name);
		DSP_FLD ("endIssueDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Contract Status 
	 */
	if (LCHECK ("status"))
	{
		if (dflt_used)
			strcpy (local_rec.status, "A");

		switch (local_rec.status [0])
		{
		case 'O':
			strcpy (local_rec.statusDesc, ML ("Open       "));
			break;

		case 'X':
			strcpy (local_rec.statusDesc, ML ("Credit Hold"));
			break;

		case 'B':
			strcpy (local_rec.statusDesc, ML ("Billing    "));
			break;

		case 'C':
			strcpy (local_rec.statusDesc, ML ("Closed     "));
			break;

		case 'H':
			strcpy (local_rec.statusDesc, ML ("History    "));
			break;

		case 'A':
			strcpy (local_rec.statusDesc, ML ("All        "));
			break;

		}
		DSP_FLD ("statusDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCmit (
	char	*keyValue)
{
	_work_open (10, 0, 40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", keyValue);
	cc = find_rec (cmit, &cmit_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit_rec.issto, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmit_rec.issto, cmit_rec.iss_name);
		if (cc)
			break;

		cc = find_rec (cmit, &cmit_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", temp_str);
	cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmit, "DBFIND");
}

int
SrchCmjt (
	char	*keyValue)
{
	_work_open (4, 0, 40);
	save_rec ("#Type", "#Job Type Description");

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	strncpy (cmjt_rec.job_type, keyValue, strlen (keyValue));
	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmjt_rec.br_no, comm_rec.est_no) &&
	       !strncmp (cmjt_rec.job_type, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmjt_rec.job_type, cmjt_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_SUCCESS);

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);
	cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, cmjt, "DBFIND");

	return (EXIT_SUCCESS);
}

int
SrchCmhr (
	char	*keyValue)
{
	_work_open (6, 0, 40);
	save_rec ("#No", "#Customer Ref");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		if (cc)
			break;

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_SUCCESS);
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, cmhr, "DBFIND");
	return (EXIT_SUCCESS);
}


/*
 * Process selected range. 
 */
int
ProcessRange (void)
{
	double	costToDate;
	double	invToDate;
	char	profitPc [8];

	/*
	 * Load range into tabdisp. 
	 */
	tab_open ("cnt_lst", slct_keys, 2, 0, 13, FALSE);
	tab_add ("cnt_lst", 
		"#%-8.8s  %-8.8s %-40.40s %-16.16s %-6.6s   %-12.12s   %-16.16s   %-8.8s ", 
		"CONTRACT", 
		"CUSTOMER", 
		"          CUSTOMER NAME", 
		"ORDER REFERENCE", 
		"STATUS", 
		"COST TO DATE", 
		"INVOICED TO DATE", 
		"PROFIT %%");
	noInTab = 0;

	/*
	 * Read contract records. 
	 */
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.startContNo);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       strcmp (cmhr_rec.cont_no, local_rec.endContNo) <= 0)
	{
		/*
		 * Lookup customer for contract. 
		 */
		cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (strcmp (cumr_rec.dbt_no, local_rec.startCustomer) < 0 ||
		    strcmp (cumr_rec.dbt_no, local_rec.endCustomer) > 0)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Lookup job type code for contract. 
		 */
		cmjt_rec.hhjt_hash	= cmhr_rec.hhjt_hash;
		cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (strcmp (cmjt_rec.job_type, local_rec.startJobType) < 0 ||
		    strcmp (cmjt_rec.job_type, local_rec.endJobType) > 0)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Lookup issue to code for contract. 
		 */
		if (cmhr_rec.hhit_hash == 0L)
			strcpy (cmit_rec.issto, "          ");
		else
		{
			cmit_rec.hhit_hash = cmhr_rec.hhit_hash;
			cc = find_rec (cmit2, &cmit_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
				continue;
			}
		}
		if (strcmp (cmit_rec.issto, local_rec.startIssueTo) < 0 ||
		    strcmp (cmit_rec.issto, local_rec.endIssueTo) > 0)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check that contract has valid status for the option being run.
		 */
		if (local_rec.status [0] != 'A' && 
		    cmhr_rec.status [0] != local_rec.status [0])
		{	
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Calculate profitability. 
		 */
		costToDate 	= 0.00;
		invToDate 	= 0.00;
		CalculateProfit (cmhr_rec.hhhr_hash, &costToDate, &invToDate, profitPc);

		/*
		 * Add to table. 
		 */
		tab_add 
		(
			"cnt_lst", 
			" %-6.6s    %-6.6s  %-40.40s   %-16.16s  %-1.1s     %12.2f    %12.2f       %-7.7s    %010ld", 
			cmhr_rec.cont_no, 
			cumr_rec.dbt_no, 
			cumr_rec.dbt_name, 
			cmhr_rec.cus_ref, 
			cmhr_rec.status, 
			DOLLARS (costToDate), 
			DOLLARS (invToDate), 
			profitPc, 
			cmhr_rec.hhhr_hash
		);
		noInTab++;

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	if (noInTab == 0)
	{
		tab_add ("cnt_lst", "There are no valid contracts in the selected range. ");
		tab_display ("cnt_lst", TRUE);
		sleep (sleepTime);
		tab_close ("cnt_lst", TRUE);
		return (EXIT_SUCCESS);
	}
	else
	{
		tab_scan ("cnt_lst");
		tab_close ("cnt_lst", TRUE);
	}
	
	return (EXIT_SUCCESS);
}

/*
 * Calculate profitability. 
 */
void
CalculateProfit (
	long	hhhrHash, 
	double	*cost, 
	double 	*invoiceAmt, 
	char 	*profit)
{
	double	lcl_units;
	double	lcl_cost;
	double	lcl_profit;

	/*
	 * Accumulate Material Transactions 
	 */
	cmtr_rec.hhhr_hash 	= hhhrHash;
	cmtr_rec.hhcm_hash 	= 0L;
	cmtr_rec.date 		= 0L;
	cc = find_rec (cmtr, &cmtr_rec, GTEQ, "r");
	while (!cc && cmtr_rec.hhhr_hash == hhhrHash)
	{
		if (cmtr_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
			continue;
		}
		*cost += (cmtr_rec.cost_price * (double)cmtr_rec.qty);

		cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
	}

	/*
	 * Accumulate Employee and Plant Transactions 
	 */
	cmts_rec.hhhr_hash = hhhrHash;
	cmts_rec.hhcm_hash = 0L;
	cmts_rec.date = 0L;
	cc = find_rec (cmts, &cmts_rec, GTEQ, "r");
	while (!cc && cmts_rec.hhhr_hash == hhhrHash)
	{
		if (cmts_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmts, &cmts_rec, NEXT, "r");
			continue;
		}

		if (cmts_rec.hheq_hash == 0)
		{
			lcl_units = (cmts_rec.time_ord + 
				    	(cmts_rec.time_hlf * 1.5) + 
				    	(cmts_rec.time_dbl * 2.0));
			lcl_cost = cmts_rec.lab_cost + cmts_rec.oh_cost;
		}
		else
		{
			lcl_units = cmts_rec.units;
			lcl_cost  = cmts_rec.oh_cost;
		}
		*cost += ((double)lcl_units * lcl_cost);

		cc = find_rec (cmts, &cmts_rec, NEXT, "r");
	}

	/*
	 * Calculate amount invoiced to date. 
	 */
	cmpb_rec.hhhr_hash = hhhrHash;
	cc = find_rec (cmpb, &cmpb_rec, GTEQ, "r");
	while (!cc && cmpb_rec.hhhr_hash == hhhrHash)
	{
		if (cmpb_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}
		*invoiceAmt += cmpb_rec.amount;

		cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
	}

	if (*cost == 0.00 || *invoiceAmt == 0.00)
		strcpy (profit, "*******");
	else
	{
		lcl_profit = ((*invoiceAmt - *cost) / *cost) * 100.00;
		sprintf (profit, "%7.2f", lcl_profit);
	}
}

/*
 * Display selected contract 
 */
static int
DisplayContract (
	int			c, 
	KEY_TAB 	*psUnused)
{
	int		currLine	=	0;
	char	getBuffer [200];
	long	hhcuHoHash;

	/*
	 * Lookup contract header for selected contract. 
	 */
	if (!singleContract)
	{
		currLine = tab_tline ("cnt_lst");
		tab_get ("cnt_lst", getBuffer, EQUAL, currLine);
		cmhr_rec.hhhr_hash = atol (getBuffer + 132);
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();	
			return (c);
		}
	}

	/*
	 * Lookup customer record. 
	 */
	cumr_rec.hhcu_hash	=	cmhr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr2, "DBFIND");

	/*
	 * Lookup HO customer record. 
	 */
	hhcuHoHash = cumr_rec.ho_dbt_hash;
	if (hhcuHoHash == 0L)
		hhcuHoHash = cumr_rec.hhcu_hash;

	cumr2_rec.hhcu_hash = hhcuHoHash;
	cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr2, "DBFIND");

	/*
	 * Lookup contract description records. 
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc &&
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       !strcmp (cmcd_rec.stat_flag, "D") &&
	       cmcd_rec.line_no < 7)
	{
		sprintf (contractDesc [cmcd_rec.line_no], "%-70.70s", cmcd_rec.text);
		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}

	/*
	 * Lookup Issue To Details. 
	 */
	cmit_rec.hhit_hash = cmhr_rec.hhit_hash;
	cc = find_rec (cmit2, &cmit_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cmit_rec.issto, "          ");
		sprintf (cmit_rec.iss_name, "%-40.40s", "          ");
	}

	/*
	 * Lookup Job Type Details. 
	 */
	cmjt_rec.hhjt_hash	= cmhr_rec.hhjt_hash;
	cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cmjt_rec.job_type, " ");
		strcpy (cmjt_rec.desc, " ");
	}

	/*
	 * Lookup WIP Details. 
	 */
	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", cmhr_rec.wip_status);
	cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cmws_rec.wp_stat, "ERR ");
		sprintf (cmws_rec.desc, "%-40.40s", "NOT FOUND ");
	}

	/*
	 * Display main screen. 
	 */
	AllDisplay ();

#ifndef GVISION
	run_menu (_main_menu, "", input_row);
#else
	run_menu (NULL, _main_menu);
#endif

	/*
	 * Redraw tabdisp table. 
	 */
	if (!singleContract)
	{
		move (0, 19); cl_line ();
		move (0, 20); cl_line ();

		tab_display ("cnt_lst", TRUE);
		redraw_keys ("cnt_lst");
		tab_get ("cnt_lst", getBuffer, EQUAL, currLine);
	}
	return (c);
}

/*
 * Display main screen. 
 */
void
AllDisplay (void)
{
	double	costToDate;
	double	invToDate;
	char	statusDesc [15];
	char	finishedDate [11];
	char	profitPc [8];

	switch (cmhr_rec.status [0])
	{
	case 'O':
		strcpy (statusDesc, "Open       ");
		break;

	case 'X':
		strcpy (statusDesc, "Credit Hold");
		break;

	case 'B':
		strcpy (statusDesc, "Billing    ");
		break;

	case 'C':
		strcpy (statusDesc, "Closed     ");
		break;

	case 'H':
		strcpy (statusDesc, "History    ");
		break;
	}

	if (mainWindowOpen)
		Dsp_close ();

	Dsp_open (0, 2, 13);
	sprintf (err_str, 
		" Contract Number : %-6.6s (%-70.70s)  Current Status : %-11.11s ", 
			cmhr_rec.cont_no, contractDesc [0], statusDesc);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec ("");
	mainWindowOpen = TRUE;

	/*
	 * Contract Description. 
	 */
	sprintf (err_str, 
			" Description : %-70.70s ^E Order Date    : %-10.10s", 
				contractDesc [0], DateToString (cmhr_rec.st_date));
	Dsp_saverec (err_str);

	sprintf (err_str, 
			"             : %-70.70s ^E Required Date : %-10.10s", 
				contractDesc [1], DateToString (cmhr_rec.due_date));
	Dsp_saverec (err_str);

	if (cmhr_rec.end_date == 0L)
		strcpy (finishedDate, "        ");
	else
		sprintf (finishedDate, "%-10.10s", DateToString (cmhr_rec.end_date));
	
	sprintf (err_str, 
			"             : %-70.70s ^E Finished Date : %-10.10s", 
				contractDesc [2], finishedDate);
	Dsp_saverec (err_str);

	sprintf (err_str, "             : %-70.70s ^E", contractDesc [3]);
	Dsp_saverec (err_str);

	/*
	 * Calculate profitability. 
	 */
	costToDate 	= 0.00;
	invToDate 	= 0.00;
	CalculateProfit (cmhr_rec.hhhr_hash, &costToDate, &invToDate, profitPc);
	sprintf (err_str, 
		"             : %-70.70s ^E Cost To Date           : %14.14s", 
		contractDesc [4], comma_fmt (DOLLARS (costToDate), "NNN,NNN,NNN.NN"));
	Dsp_saverec (err_str);

	sprintf (err_str, 
		"             : %-70.70s ^E Invoice Amount To Date : %14.14s", 
		contractDesc [5], comma_fmt (DOLLARS (invToDate), "NNN,NNN,NNN.NN"));
	Dsp_saverec (err_str);

	sprintf (err_str, 
		"             : %-70.70s ^E Profit Percentage      : %14.14s", 
		contractDesc [6], profitPc);
	Dsp_saverec (err_str);

	Dsp_saverec (underline);

	/*
	 * Customer Details. 
	 */
	sprintf (err_str, 
		" Customer       : %-6.6s  %-32.32s ^E", 
		cumr_rec.dbt_no, 
		cumr_rec.dbt_name);
	Dsp_saverec (err_str);

	sprintf (err_str, 
		" Contact Name   : %-20.20s%-20.20s ^E", cmhr_rec.contact, " ");
	Dsp_saverec (err_str);

	sprintf (err_str, " Customer Addr. : %-40.40s ^E", cmhr_rec.adr1);
	Dsp_saverec (err_str);

	sprintf (err_str, "                : %-40.40s ^E", cmhr_rec.adr2);
	Dsp_saverec (err_str);

	sprintf (err_str, "                : %-40.40s ^E", cmhr_rec.adr3);
	Dsp_saverec (err_str);

	Dsp_srch ();

	/*
	 * Print missing graphics characters. 
	 */
	PrintMissGraphics ();

	/*
	 * Print popup window selected. 
	 */
	PrintPopup (popupSelect);
}

/*
 * Display contract description 
 */
static int
ContractDesc (
	int			c, 
	KEY_TAB 	*psUnused)
{
	int		currLine;
	long	hhhrHash;
	char	getBuffer [200];

	currLine = tab_tline ("cnt_lst");
	tab_get ("cnt_lst", getBuffer, EQUAL, currLine);
	hhhrHash = atol (getBuffer + 132);

	/*
	 * Open Dsp window. 
	 */
	Dsp_open (1, 6, 8);
	sprintf (err_str, 
		"%-20.20s Description For Contract Number %-6.6s %-20.20s", 
		" " , 
		getBuffer + 1, 
		" ");
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec ("    [NEXT][PREV][END INPUT]     ");

	/*
	 * Lookup description. 
	 */
	cmcd_rec.hhhr_hash = hhhrHash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc &&
	       cmcd_rec.hhhr_hash == hhhrHash &&
	       !strcmp (cmcd_rec.stat_flag, "D") &&
	       cmcd_rec.line_no < 7)
	{
		sprintf (err_str, "   %-70.70s   ", cmcd_rec.text);
		Dsp_saverec (err_str);

		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();

	tab_display ("cnt_lst", TRUE);
	redraw_keys ("cnt_lst");
	tab_get ("cnt_lst", getBuffer, EQUAL, currLine);

	return (c);
}

/*
 * Print Cost Sheets For All Contracts In Table. 
 */
static int
PrintAllCostSheets (
	int			c, 
	KEY_TAB 	*psUnused)
{
	int		i;
	int		currLine;
	int		rvs_flag;
	long	hhcuHoHash;
	long	hhhrHash;
	char	getBuffer [200];

	currLine = tab_tline ("cnt_lst");
	/*
	 * Get print info. 
	 */
	lp_x_off = 0;
	lp_y_off = 2;
	local_rec.printerNo = get_lpno (0);

	tab_display ("cnt_lst", TRUE);
	redraw_keys ("cnt_lst");
	move (0, 19);
	cl_line ();

	printAll = TRUE;
	firstContract = TRUE;
	for (i = 0; i < noInTab; i++)
	{
		tab_get ("cnt_lst", getBuffer, EQUAL, i);
		hhhrHash = atol (getBuffer + 132);

		/*
		 * Find contract master record. 
		 */
		cmhr_rec.hhhr_hash = hhhrHash;
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
		if (cc)
			continue;
	
		/*
		 * Find customer record. 
		 */
		cumr_rec.hhcu_hash	=	cmhr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
		if (cc)
			continue;
	
		/*
		 * Lookup HO customer record. 
		 */
		hhcuHoHash = cumr_rec.ho_dbt_hash;
		if (hhcuHoHash == 0L)
			hhcuHoHash = cumr_rec.hhcu_hash;

		cumr2_rec.hhcu_hash	 = hhcuHoHash;
		cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
		if (cc)
			continue;

		/*
		 * Lookup Issue To Details. 
		 */
		cmit_rec.hhit_hash = cmhr_rec.hhit_hash;
		cc = find_rec (cmit2, &cmit_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cmit_rec.issto, "          ");
			sprintf (cmit_rec.iss_name, "%-40.40s", "          ");
		}
	
		/*
		 * Lookup Job Type Details. 
		 */
		cmjt_rec.hhjt_hash = cmhr_rec.hhjt_hash;
		cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cmjt_rec.job_type, " ");
			strcpy (cmjt_rec.desc,    " ");
		}
	
		/*
		 * Lookup WIP Details. 
		 */
		strcpy (cmws_rec.co_no, comm_rec.co_no);
		sprintf (cmws_rec.wp_stat, "%-4.4s", cmhr_rec.wip_status);
		cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cmws_rec.wp_stat, " ");
			strcpy (cmws_rec.desc,    " ");
		}

		/*
		 * Print cost sheet for current contract. 
		 */
		rvs_flag = (i % 2 == 0);
		rv_pr (ML (mlStdMess035), 50, 19, rvs_flag);
		PrintCostSheet ();
	}
	printAll = FALSE;

	tab_display ("cnt_lst", TRUE);
	redraw_keys ("cnt_lst");
	tab_get ("cnt_lst", getBuffer, EQUAL, currLine);

	/*
	 * Close pformat. 
	 */
	if (pipeOpen)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
		pipeOpen = FALSE;
	}

	move (0, 19);
	cl_line ();

	return (c);
}

/*
 * Print cost sheet for current contract.      
 * cmhr_rec.hhhr_hash WILL contain the hhhrHash for the contract to be printed.
 */
int
PrintCostSheet (void)
{
	char	statusDesc [12];

	/*
	 * Get print info. 
	 */
	if (!printAll)
	{
		lp_x_off = 0;
		lp_y_off = 2;
		local_rec.printerNo = get_lpno (0);
		ReDraw ();
	}

	if (!pipeOpen)
	{
		if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.printerNo);
		fprintf (fout, ".6\n");
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L158\n");

		fprintf (fout, ".E CONTRACT COST DETAILS \n");
		fprintf (fout, ".E Company : %2.2s - %s \n", 
						comm_rec.co_no, clip (comm_rec.co_name));
		fprintf (fout, ".E AS AT : %s \n", SystemTime ());
		fprintf (fout, ".B1\n");

		fprintf (fout, ".R=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
	
		pipeOpen = TRUE;
	}

	if (!printAll)
		print_at (19, 50, ML (mlCmMess094));

	/*
	 * Define Section For The Heading Of This Contract.
	 */
	fprintf (fout, ".DS2\n");
	fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
	fprintf (fout, "=======================================");
	fprintf (fout, "=======================================");
	fprintf (fout, "=======================================");
	fprintf (fout, "==================================\n");

	/*
	 * Page break if we are printing all contracts and this is not the first
	 * contract printed.                 
	 */
	if (printAll && !firstContract)
		fprintf (fout, ".PA\n");

	nowPrinting = TRUE;
	/*
	 * Print Header Information. 
	 */
	fprintf (fout, "| Charge To Customer : %-6.6s %-40.40s      ", 
		cumr2_rec.dbt_no, cumr2_rec.dbt_name);
	fprintf (fout, "For Customer     : %-6.6s %-40.40s        |\n", 
		cumr_rec.dbt_no, cumr_rec.dbt_name);
	fprintf (fout, "| Charge To Address  : %-40.40s             ", 
		cumr2_rec.ch_adr1);
	fprintf (fout, "Customer Address : %-40.40s               |\n", 
		cmhr_rec.adr1);
	fprintf (fout, "|                    : %-40.40s             ", 
		cumr2_rec.ch_adr2);
	fprintf (fout, "                 : %-40.40s               |\n", 
		cmhr_rec.adr2);
	fprintf (fout, "|                    : %-40.40s             ", 
		cumr2_rec.ch_adr3);
	fprintf (fout, "                 : %-40.40s               |\n", 
		cmhr_rec.adr3);

	fprintf (fout, "| Contact    : %-20.20s     %-35.35s", 
		cmhr_rec.contact, " ");
	switch (cmhr_rec.status [0])
	{
	case 'O':
		strcpy (statusDesc, ML ("Open       "));
		break;

	case 'X':
		strcpy (statusDesc, ML ("Credit Held"));
		break;

	case 'B':
		strcpy (statusDesc, ML ("Billing    "));
		break;

	case 'C':
		strcpy (statusDesc, ML ("Closed     "));
		break;

	case 'H':
		strcpy (statusDesc, ML ("History    "));
		break;

	}
	fprintf (fout, " Status        : %-1.1s - %-11.11s   %-40.40s|\n", 
		cmhr_rec.status, 
		statusDesc, 
		" ");

	fprintf (fout, "| Job Type   : %-4.4s %-30.30s     %-20.20s", 
		cmjt_rec.job_type, 
		cmjt_rec.desc, 
		" ");
	fprintf (fout, " Issue To Code : %-10.10s %-40.40s       |\n", 
		cmit_rec.issto, 
		cmit_rec.iss_name);

	fprintf (fout, "| WIP Status : %-4.4s %-40.40s%-15.15s", 
		cmws_rec.wp_stat, 
		cmws_rec.desc, 
		" ");
	fprintf (fout, " WIP Date      : %-10.10s%-48.48s|\n", 
		DateToString (cmhr_rec.wip_date), 
		" ");
	fprintf (fout, "|%-149.149s|\n", " ");

	/*
	 * Print Contract Description. 
	 */
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------|\n");
	fprintf (fout, "|%-60.60s     CONTRACT DESCRIPTION    %-60.60s|\n", " ", " ");
	fprintf (fout, "|%-149.149s|\n", " ");
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc &&
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       !strcmp (cmcd_rec.stat_flag, "D") &&
	       cmcd_rec.line_no < 7)
	{
		fprintf (fout, 
			"|%-38.38s %-70.70s  %-38.38s|\n", 
			" ", 
			cmcd_rec.text, 
			" ");

		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}
	fprintf (fout, "|%-149.149s|\n", " ");

	/*
	 * Print Extra Description. 
	 */
	XDescHeader ();
	XDescDisp ();
	fprintf (fout, "|%-149.149s|\n", " ");
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------|\n");

	/*
	 * Print Budget Information. 
	 */
	BudgetHead (TRUE);
	BudgetHead (FALSE);
	BudgetDisplay ();
	fprintf (fout, "| -------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------------------------- |\n");
	fprintf (fout, "|%-149.149s|\n", " ");
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------|\n");

	/*
	 * Print Employee/Plant Time. 
	 */
	TimeHeader (FALSE);
	TimeHeader (TRUE);
	TimeDisplay ();
	fprintf (fout, "|           ---------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------            |\n");
	fprintf (fout, "|%-149.149s|\n", " ");
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------|\n");

	/*
	 * Print Materials. 
	 */
	MaterialHead (FALSE);
	MaterialHead (TRUE);
	MaterialDisplay ();
	fprintf (fout, "|                   -------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------                    |\n");
	fprintf (fout, "|%-149.149s|\n", " ");

	nowPrinting = FALSE;
	/*
	 * Close pformat. 
	 */
	if (!printAll)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
		pipeOpen = FALSE;
	}

	firstContract = FALSE;

	return (EXIT_SUCCESS);
}

/*
 * Print header for Extra Description section. 
 */
void
XDescHeader (void)
{
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------|\n");
	fprintf (fout, ".DS4\n");
	fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
	fprintf (fout, "=======================================");
	fprintf (fout, "=======================================");
	fprintf (fout, "=======================================");
	fprintf (fout, "==================================\n");
	fprintf (fout, 
		"|%-60.60s    ADDITIONAL DESCRIPTION   %-60.60s|\n", 
		" ", " ");
	fprintf (fout, "|%-149.149s|\n", " ");

	/*
	 * Print heading now. Previous 2 lines are used by the .DS 
	 */
	fprintf (fout, 
		"|%-60.60s   ADDITIONAL DESCRIPTION    %-60.60s|\n", 
		" ", " ");
	fprintf (fout, "|%-149.149s|\n", " ");

	return;
}

/*
 * Print header for Budget section. 
 */
void
BudgetHead (
 int	define_section)
{
	if (define_section)
	{
		fprintf (fout, ".DS8\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
	}
	else
	{
		fprintf (fout, ".DS2\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
		fprintf (fout, ".LRP8\n");
	}
	
	fprintf (fout, "|%-65.65s COSTHEAD BUDGETS  %-65.65s|\n", " ", " ");

	fprintf (fout, "|%-149.149s|\n", " ");

	fprintf (fout, "| -------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------------------------- |\n");

	fprintf (fout, "| |COSTHEAD");
	fprintf (fout, "|                                         ");
	fprintf (fout, "| BUDGET  ");
	fprintf (fout, "|  BUDGET  ");
	fprintf (fout, "|  ACTUAL  ");
	fprintf (fout, "|     VARIANCE      ");
	fprintf (fout, "|  BUDGET  ");
	fprintf (fout, "|  ACTUAL  ");
	fprintf (fout, "|     VARIANCE       | |\n");

	fprintf (fout, "| |  CODE  ");
	fprintf (fout, "|          COSTHEAD DESCRIPTION           ");
	fprintf (fout, "|  TYPE   ");
	fprintf (fout, "| QUANTITY ");
	fprintf (fout, "| QUANTITY ");
	fprintf (fout, "| QUANTITY |   %%    ");
	fprintf (fout, "|  VALUE   ");
	fprintf (fout, "|  VALUE   ");
	fprintf (fout, "|  VALUE   |   %%     | |\n");

	fprintf (fout, "| |--------");
	fprintf (fout, "+-----------------------------------------");
	fprintf (fout, "+---------");
	fprintf (fout, "+----------");
	fprintf (fout, "+----------");
	fprintf (fout, "+-------------------");
	fprintf (fout, "+----------");
	fprintf (fout, "+----------");
	fprintf (fout, "+--------------------| |\n");
}

/*
 * Print header for Employee/Plant Time section. 
 */
void
TimeHeader (
 int	define_section)
{
	if (define_section)
	{
		fprintf (fout, ".DS8\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
	}
	else
	{
		fprintf (fout, ".DS2\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
		fprintf (fout, ".LRP12\n");
	}
	
	fprintf (fout, "|%-65.65sEMPLOYEE/PLANT TIME%-65.65s|\n", " ", " ");

	fprintf (fout, "|%-149.149s|\n", " ");

	fprintf (fout, "|           ---------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------            |\n");

	fprintf (fout, "|           |        ");
	fprintf (fout, "| EMPLOYEE ");
	fprintf (fout, "|          ");
	fprintf (fout, "| UNITS ");
	fprintf (fout, "| 1.0   ");
	fprintf (fout, "| 1.5   ");
	fprintf (fout, "| 2.0   ");
	fprintf (fout, "|LABOUR COST");
	fprintf (fout, "|O/HEAD COST");
	fprintf (fout, "|  LABOUR   ");
	fprintf (fout, "| OVERHEAD  ");
	fprintf (fout, "|    TOTAL    |            |\n");

	fprintf (fout, "|           |  DATE  ");
	fprintf (fout, "|   CODE   ");
	fprintf (fout, "|PLANT CODE");
	fprintf (fout, "| USED  ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| PER UNIT  ");
	fprintf (fout, "| PER UNIT  ");
	fprintf (fout, "|   COSTS   ");
	fprintf (fout, "|   COSTS   ");
	fprintf (fout, "|   COSTS     |            |\n");

	fprintf (fout, "|           |--------");
	fprintf (fout, "+----------");
	fprintf (fout, "+----------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-------------|            |\n");
}

/*
 * Print header for Materials section. 
 */
void
MaterialHead (
 int	define_section)
{
	if (define_section)
	{
		fprintf (fout, ".DS8\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
	}
	else
	{
		fprintf (fout, ".DS2\n");
		fprintf (fout, ".E CONTRACT NUMBER : %-6.6s\n", cmhr_rec.cont_no);
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "=======================================");
		fprintf (fout, "==================================\n");
		fprintf (fout, ".LRP12\n");
	}
	
	fprintf (fout, "|%-65.65s  MATERIALS USED   %-65.65s|\n", " ", " ");

	fprintf (fout, "|%-149.149s|\n", " ");

	fprintf (fout, "|                   -------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------                    |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|          ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "| QUANTITY  ");
	fprintf (fout, "|   UNIT    ");
	fprintf (fout, "|   TOTAL   ");
	fprintf (fout, "|                    |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|             ITEM DESCRIPTION             ");
	fprintf (fout, "|   USED    ");
	fprintf (fout, "|   COST    ");
	fprintf (fout, "|   COST    ");
	fprintf (fout, "|                    |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|                    |\n");

}

static int
PrintStatus (
 int		c, 
 KEY_TAB *	psUnused)
{
	int		i;
	int		currLine;
	char	stat_desc [12];
	char	getBuffer [200];

	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*
	 * Get print info. 
	 */
	lp_x_off = 0;
	lp_y_off = 2;
	local_rec.printerNo = get_lpno (0);

	/*
	 * Redraw Screen. 
	 */
	currLine = tab_tline ("cnt_lst");
	tab_display ("cnt_lst", TRUE);
	redraw_keys ("cnt_lst");
	tab_get ("cnt_lst", getBuffer, EQUAL, currLine);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".6\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L135\n");

	fprintf (fout, ".E CONTRACT STATUS DETAILS \n");
	fprintf (fout, 
		".E Company : %2.2s - %s \n", 
		comm_rec.co_no, 
		clip (comm_rec.co_name));
	fprintf (fout, ".E AS AT : %-24.24s \n", SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R=========");
	fprintf (fout, "=========");
	fprintf (fout, "===========================================");
	fprintf (fout, "===================");
	fprintf (fout, "================");
	fprintf (fout, "=============");
	fprintf (fout, "===============");
	fprintf (fout, "==========\n");

	/*
	 * Heading for report. 
	 */
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "===========================================");
	fprintf (fout, "===================");
	fprintf (fout, "===============");
	fprintf (fout, "=============");
	fprintf (fout, "===============");
	fprintf (fout, "===========\n");

	fprintf (fout, "|CONTRACT");
	fprintf (fout, "|CUSTOMER");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|              ");
	fprintf (fout, "|            ");
	fprintf (fout, "|INVOICE AMOUNT");
	fprintf (fout, "|         |\n");

	fprintf (fout, "| NUMBER ");
	fprintf (fout, "| NUMBER ");
	fprintf (fout, "|            CUSTOMER NAME                 ");
	fprintf (fout, "| ORDER REFERENCE  ");
	fprintf (fout, "|    STATUS    ");
	fprintf (fout, "|COST TO DATE");
	fprintf (fout, "|   TO DATE    ");
	fprintf (fout, "|PROFIT %% |\n");

	fprintf (fout, "|--------");
	fprintf (fout, "+--------");
	fprintf (fout, "+------------------------------------------");
	fprintf (fout, "+------------------");
	fprintf (fout, "+--------------");
	fprintf (fout, "+------------");
	fprintf (fout, "+--------------");
	fprintf (fout, "+---------|\n");

	for (i = 0; i < noInTab; i++)
	{
		tab_get ("cnt_lst", getBuffer, EQUAL, i);

		switch (getBuffer [80])
		{
		case 'O':
			strcpy (stat_desc, ML ("Open       "));
			break;
	
		case 'X':
			strcpy (stat_desc, ML ("Credit Held"));
			break;
	
		case 'B':
			strcpy (stat_desc, ML ("Billing    "));
			break;
	
		case 'C':
			strcpy (stat_desc, ML ("Closed     "));
			break;
	
		case 'H':
			strcpy (stat_desc, ML ("History    "));
			break;
		}

		fprintf (fout, "| %-6.6s ", getBuffer + 1);
		fprintf (fout, "| %-6.6s ", getBuffer + 11);
		fprintf (fout, "| %-40.40s ", getBuffer + 19);
		fprintf (fout, "| %-16.16s ", getBuffer + 62);
		fprintf (fout, "| %-11.11s  ", stat_desc);
		fprintf (fout, "|%-12.12s", getBuffer + 86);
		fprintf (fout, "| %-12.12s ", getBuffer + 102);
		fprintf (fout, "| %-7.7s |\n", getBuffer + 121);
	}

	tab_display ("cnt_lst", TRUE);
	redraw_keys ("cnt_lst");
	tab_get ("cnt_lst", getBuffer, EQUAL, currLine);

	fprintf (fout, ".EOF\n");
	pclose (fout);

	return (c);
}


int
SelectPopupA (void)
{
	PrintPopup (IN_POP_A);
	return (EXIT_SUCCESS);
}
int
SelectPopupB (void)
{
	PrintPopup (IN_POP_B);
	return (EXIT_SUCCESS);
}

void
PrintPopup (
 int	type)
{

	crsr_off ();
	if (type)
		popupSelect = type;

	switch (popupSelect)
	{
	case IN_POP_A :
		/*
		 * Charge To Customer Number and Name 
		 */
		print_at (POP_Y, POP_X + 61, 
				ML (mlCmMess095), cumr2_rec.dbt_no, cumr2_rec.dbt_name);
	
		/*
		 * Charge To Contact Name. 
		 */
		print_at (POP_Y + 1, POP_X + 61, 
				ML (mlCmMess096), cumr2_rec.contact_name, " ");
	
		/*
		 * Charge To Address 1 
		 */
		print_at (POP_Y + 2, POP_X + 61, 
				ML (mlCmMess104), cumr2_rec.ch_adr1, " ");
	
		/*
		 * Charge To Address 2 
		 */
		print_at (POP_Y + 3, POP_X + 61, 
			"               : %-40.40s %-2.2s", cumr2_rec.ch_adr2, " ");
	
		/*
		 * Contract Address 3 
		 */
		print_at (POP_Y + 4, POP_X + 61, 
			"               : %-40.40s %-2.2s", cumr2_rec.ch_adr3, " ");

		break;

	    case IN_POP_B :
		/*
		 * Job Type Details 
		 */
		print_at (POP_Y, POP_X + 61, 
			ML (mlCmMess097), cmjt_rec.job_type, cmjt_rec.desc);
	
		/*
		 * Issue To Details 
		 */
		print_at (POP_Y + 1, POP_X + 61, 
			ML (mlCmMess098), cmit_rec.issto, cmit_rec.iss_name);
	
		/*
		 * WIP Details 
		 */
		print_at (POP_Y + 2, POP_X + 61, 
			ML (mlCmMess099), cmws_rec.wp_stat, cmws_rec.desc);

		print_at (POP_Y + 3, POP_X + 61, 
			ML (mlCmMess100), DateToString (cmhr_rec.wip_date), " ");

		/*
		 * Analysis Codes 
		 */
		sprintf (err_str, " %-4.4s  %-4.4s  %-4.4s  %-4.4s  %-4.4s %-20.20s", 
			cmhr_rec.usr_ref1, cmhr_rec.usr_ref2, 
			cmhr_rec.usr_ref3, cmhr_rec.usr_ref4, 
			cmhr_rec.usr_ref5, " ");

		print_at (POP_Y + 4, POP_X + 61, ML (mlCmMess101), err_str);	
		break;
	}

	print_at (12, 92, "<S%d>", popupSelect) ;
}

/*
 * Display Costhead Budgets for contract. 
 */
int
BudgetDisplay (void)
{
	double	qty_var;
	double	qty_var_pc;
	double	val_var;
	double	val_var_pc;
	char	qty_pc_str [8];
	char	val_pc_str [8];

	if (!nowPrinting)
	{
		lp_x_off = 1;
		lp_y_off = 2;
		Dsp_open (1, 4, 10);
		Dsp_saverec ("Costhead|           Costhead           | Budget |  Budget  | Actual  |    Variance     |   Budget | Actual  |   Variance      ");
		Dsp_saverec ("  Code  |           Description        |  Type  |   Qty.   |  Qty.   |  Qty    |   %   |   Value  | Value   | Value   |   %   ");
		Dsp_saverec ("  [REDRAW][NEXT][PREV][END INPUT]   ");
	}

	/*
	 * Process cmcb records. 
	 */
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;
	cc = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	while (!cc && cmcb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		/*
		 * Look up costhead record. 
		 */
		cmcm_rec.hhcm_hash	=	cmcb_rec.hhcm_hash;
		cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
		if (cc)
		{
			cc = find_rec (cmcb, &cmcb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Calculate variance. 
		 */
		qty_var = (cmcb_rec.sum_qty - cmcb_rec.budg_qty);
		if (cmcb_rec.budg_qty == 0.00)
			strcpy (qty_pc_str, "*******");
		else
		{
			qty_var_pc = (qty_var / cmcb_rec.budg_qty) * 100.00;
			sprintf (qty_pc_str, "%7.2f", qty_var_pc);
		}

		val_var = (cmcb_rec.sum_value - cmcb_rec.budg_value);
		if (cmcb_rec.budg_value == 0.00)
			strcpy (val_pc_str, "*******");
		else
		{
			val_var_pc = (val_var / cmcb_rec.budg_value) * 100.00;
			sprintf (val_pc_str, "%7.2f", val_var_pc);
		}

		if (nowPrinting)
		{
			fprintf (fout, ".LRP4\n");
			fprintf (fout, "| |  %-4.4s  ", cmcm_rec.ch_code);
			fprintf (fout, "| %-40.40s", cmcm_rec.desc);
			fprintf (fout, "| %-8.8s", (cmcb_rec.budg_type [0] == 'F') ? "Fixed" : "Variable");
			fprintf (fout, "| %9.2f", cmcb_rec.budg_qty);
			fprintf (fout, "| %9.2f", cmcb_rec.sum_qty);
			fprintf (fout, "| %9.2f", qty_var);
			fprintf (fout, "| %7.7s", qty_pc_str);
			fprintf (fout, "| %9.2f", DOLLARS (cmcb_rec.budg_value));
			fprintf (fout, "| %9.2f", DOLLARS (cmcb_rec.sum_value));
			fprintf (fout, "| %9.2f", DOLLARS (val_var));
			fprintf (fout, "| %7.7s | |\n", val_pc_str);
		}
		else
		{
			sprintf (err_str, 
				"  %-4.4s  ^E%-30.30s^E%-8.8s^E %9.2f^E%9.2f^E%9.2f^E%7.7s^E %9.2f^E%9.2f^E%9.2f^E%7.7s", 
				cmcm_rec.ch_code, 
				cmcm_rec.desc, 
				(cmcb_rec.budg_type [0] == 'F') ? "Fixed" : "Variable", 
				cmcb_rec.budg_qty, 
				cmcb_rec.sum_qty, 
				qty_var, 
				qty_pc_str, 
				DOLLARS (cmcb_rec.budg_value), 
				DOLLARS (cmcb_rec.sum_value), 
				DOLLARS (val_var), 
				val_pc_str);
			Dsp_saverec (err_str);
		}

		cc = find_rec (cmcb, &cmcb_rec, NEXT, "r");
	}

	if (!nowPrinting)
	{
		Dsp_srch ();
		Dsp_close ();

		ReDraw ();
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Materials Used for contract. 
 */
int
MaterialDisplay (void)
{
	int		first_trans;
	int		first_csthd;
	int		data_found;
	double	csthd_total;
	double	all_total;

	if (!nowPrinting)
	{
		Dsp_open (1, 4, 11);
		Dsp_saverec ("    Date    |   Item Number    |            Item   Description            |  Qty Used  |  Unit Cost  |  Total  Cost ");
		Dsp_saverec ("");
		Dsp_saverec ("  [REDRAW][NEXT][PREV][END INPUT]   ");
	}

	all_total = 0.00;

	/*
	 * Process cmcm records. 
	 */
	first_csthd = TRUE;
	data_found = FALSE;
	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", " ");
	cc = find_rec (cmcm2, &cmcm_rec, GTEQ, "r");
	while (!cc && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
		csthd_total = 0.00;

		/*
		 * Process cmtr records. 
		 */
		first_trans = TRUE;
		cmtr_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cmtr_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		cmtr_rec.date = 0L;
		cc = find_rec (cmtr, &cmtr_rec, GTEQ, "r");
		while (!cc && 
		       cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		       cmtr_rec.hhcm_hash == cmcm_rec.hhcm_hash)
		{
			/*
			 * Look up inventory record. 
			 */
			inmr_rec.hhbr_hash = cmtr_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
			if (cc)
			{
				cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
				continue;
			}
	
			if (first_trans)
			{
				if (!first_csthd)
				{
					if (!nowPrinting)
					{
						Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGG^^");
					}
				}
				if (nowPrinting)
				{
					fprintf (fout, 
						"|%-19.19s| Costhead : %-4.4s  %-40.40s%-50.50s|                    |\n", 
						" ", 
						cmcm_rec.ch_code, 
						cmcm_rec.desc, 
						" ");
				}
				else
				{
					sprintf (err_str, 
						"^1 Costhead : %-4.4s  %-40.40s ^6", 
						cmcm_rec.ch_code, 
						cmcm_rec.desc);
					Dsp_saverec (err_str);
				}
			}
			first_trans = FALSE;
			first_csthd = FALSE;
	
			data_found = TRUE;
			if (nowPrinting)
			{
				fprintf (fout, ".LRP6\n");
				fprintf (fout, "|%-19.19s", " ");
				fprintf (fout, "|%-10.10s", DateToString (cmtr_rec.date));
				fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
				fprintf (fout, "| %-40.40s ", inmr_rec.description);
				fprintf (fout, "| %9.2f ", cmtr_rec.qty);
				fprintf (fout, "| %9.2f ", DOLLARS (cmtr_rec.cost_price));
				fprintf (fout, "| %9.2f ", DOLLARS (cmtr_rec.cost_price * cmtr_rec.qty));
				fprintf (fout, "|%-20.20s|\n", " ");
			}
			else
			{
				sprintf (err_str, " %-10.10s ^E %-16.16s ^E %-40.40s ^E  %9.2f ^E   %9.2f ^E   %9.2f ", 
					DateToString (cmtr_rec.date), 
					inmr_rec.item_no, 
					inmr_rec.description, 
					cmtr_rec.qty, 
					DOLLARS (cmtr_rec.cost_price), 
					DOLLARS (cmtr_rec.cost_price * cmtr_rec.qty));
				Dsp_saverec (err_str);
			}

			csthd_total += (cmtr_rec.cost_price * cmtr_rec.qty);
	
			cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
		}

		/*
		 * Costhead Total. 
		 */
		if (csthd_total != 0.00)
		{
			if (nowPrinting)
			{
				fprintf (fout, 
					"|%-19.19s| TOTAL FOR COSTHEAD :   %-4.4s  %-40.40s %-25.25s    |%10.2f |%-20.20s|\n", 
					" ", 
					cmcm_rec.ch_code, 
					cmcm_rec.desc, 
					" ", 
					DOLLARS (csthd_total), 
					" ");
	
				fprintf (fout, "|%-19.19s|", " ");
				fprintf (fout, " %-105.105s  ", " ");
				fprintf (fout, "|%-20.20s|\n", " ");
			}
			else
			{
				sprintf (err_str, 
					" TOTAL FOR COSTHEAD : %-4.4s  %-40.40s %-30.30s  ^E%12.2f", 
					cmcm_rec.ch_code, 
					cmcm_rec.desc, 
					" ", 
					DOLLARS (csthd_total));
				Dsp_saverec (err_str);
			}
		
			all_total += csthd_total;
		}

		cc = find_rec (cmcm2, &cmcm_rec, NEXT, "r");
	}

	/*
	 * Materials Total. 
	 */
	if (all_total != 0.00)
	{
		if (nowPrinting)
		{
			fprintf (fout, 
				"|%-19.19s| GRAND TOTAL : %-80.80s   |%10.2f |%-20.20s|\n", 
				" ", 
				" ", 
				DOLLARS (all_total), 
				" ");
		}
		else
		{
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGG^^");
		
			sprintf (err_str, 
				" GRAND TOTAL : %-80.80s      ^E%12.2f", 
				" ", 
				DOLLARS (all_total));
			Dsp_saverec (err_str);
	
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGG^^");
		}
	}

	if (nowPrinting)
	{
		if (!data_found)
		{
			fprintf (fout, 
				"|%-19.19s| %-106.106s |%-20.20s|\n", 
				" ", 
				"                               ****     NO MATERIALS FOR CONTRACT     **** ", 
				" ");
		}
	}
	else
	{
		Dsp_srch ();
		Dsp_close ();

		ReDraw ();
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Requisitions for contract. 
 */
int
ReqDisplay (void)
{
	float	ord_qty;
	double	tmp_value;
	double	req_cost;
	double	req_sale;
	char	req_date [11];
	char	rqrd_date [11];
	char	head_text [120];
	char	req_status [19];
	char	save_key [11];

	req_cost = 0.00;
	req_sale = 0.00;

	sprintf (head_text, 
		" Contract Number : %-6.6s (%-70.70s)", 
		cmhr_rec.cont_no, 
		contractDesc [0]);

	lp_x_off = 1;
	lp_y_off = 2;
	Dsp_prn_open (1, 4, 10, 
		      head_text, 
		      comm_rec.co_no, comm_rec.co_name, 
		      comm_rec.est_no, comm_rec.est_name, 
		     (char *) 0, (char *) 0);

	Dsp_saverec ("  REQUISITION |    REQUISITIONED BY    |    ORDER    |    DUE      | FULL   |   COST     |  SALE VALUE  |      STATUS         ");
	Dsp_saverec ("     NUMBER   |                        |    DATE.    |    DATE.    | SUPPLY |            |              |                     ");
	Dsp_saverec (" [REDRAW][PRINT][NEXT][PREV][END]");

	cmrh_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && cmrh_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		/*
		 * Add up totals for requisition. 
		 */
		cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
		cmrd_rec.line_no = 0;
		cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
		while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
		{
			if ((cmrd_rec.qty_order + cmrd_rec.qty_border) > 0.00)
			{
				ord_qty = cmrd_rec.qty_order + cmrd_rec.qty_border;

				/*
				 * Calculate sale value. 
				 */
				tmp_value = out_cost (cmrd_rec.sale_price, inmr_rec.outer_size);
				tmp_value -= CAL (tmp_value, cmrd_rec.disc_pc);
				tmp_value *= (double)ord_qty;
				req_sale += tmp_value;
			
				/*
				 * Calculate cost. 
				 */
				tmp_value = out_cost (cmrd_rec.cost, 
					   	     inmr_rec.outer_size);
				tmp_value *= (double)ord_qty;
				req_cost += tmp_value;
			}
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
		}

		sprintf (req_date, "%-10.10s", DateToString (cmrh_rec.req_date));
		sprintf (rqrd_date, "%-10.10s", DateToString (cmrh_rec.rqrd_date));

		switch (cmrh_rec.stat_flag [0])
		{
		case 'R':
			strcpy (req_status, "Ready For Despatch");
			break;

		case 'B':
			strcpy (req_status, "Backordered       ");
			break;

		case 'F':
			strcpy (req_status, "Forward Ordered   ");
			break;

		case 'C':
			strcpy (req_status, "Complete          ");
			break;
		}

		sprintf (err_str, "    %06ld    ^E  %-20.20s  ^E %-10.10s  ^E %-10.10s  ^E  %-3.3s   ^E  %9.2f ^E %12.2f ^E %-1.1s)%-17.17s ", 
			cmrh_rec.req_no, 
			cmrh_rec.req_by, 
			req_date, 
			rqrd_date, 
			(cmrh_rec.full_supply [0] == 'Y') ? "Yes" : "No ", 
			DOLLARS (req_cost), 
			DOLLARS (req_sale), 
			req_status, 
			&req_status [1]);
		sprintf (save_key, "%010ld", cmrh_rec.hhrq_hash);

		if (cmrh_rec.stat_flag [0] == 'C')
			Dsp_saverec (err_str);
		else
			Dsp_save_fn (err_str, save_key);

		cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
	}

	Dsp_srch_fn (ReqDetails);
	Dsp_close ();

	ReDraw ();
	return (EXIT_SUCCESS);
}

/*
 * Display requisition details. 
 */
int
ReqDetails (
 char *	hash_str)
{
	long	hhrqHash;
	float	ord_qty;
	double	o_tot_val;
	double	ord_val;
	double	unit_price;
	char	req_date [11];
	char	rqrd_date [11];
	char	req_status [19];
	char	line_status [19];
	char	head_text [120];

	/*
	 * Look up requisition header. 
	 */
	hhrqHash = atol (hash_str);
	cmrh_rec.hhrq_hash	= hhrqHash;
	cc = find_rec (cmrh2, &cmrh_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_SUCCESS);

	/*
	 * Header string. 
	 */
	sprintf (req_date, "%-10.10s", DateToString (cmrh_rec.req_date));
	sprintf (rqrd_date, "%-10.10s", DateToString (cmrh_rec.rqrd_date));
	switch (cmrh_rec.stat_flag [0])
	{
	case 'R':
		strcpy (req_status, "Ready For Despatch");
		break;

	case 'B':
		strcpy (req_status, "Backordered       ");
		break;

	case 'F':
		strcpy (req_status, "Forward Ordered   ");
		break;

	case 'C':
		strcpy (req_status, "Complete          ");
		break;
	}
	sprintf (head_text, 
		" Requisition Number : %06ld  Requested By : %-20.20s  Order Date : %-10.10s  Required Date : %-10.10s  Status : %-18.18s " , 
		cmrh_rec.req_no, 
		cmrh_rec.req_by, 
		req_date, 
		rqrd_date, 
		req_status);

	lp_x_off = 1;
	lp_y_off = 2;
	Dsp_prn_open (1, 4, 10, 
		      head_text, 
		      comm_rec.co_no, comm_rec.co_name, 
		      comm_rec.est_no, comm_rec.est_name, 
		     (char *) 0, (char *) 0);

	Dsp_saverec ("BR|WH|  ITEM NUMBER   |           ITEM DESCRIPTION             |  QTY  |  QTY  |  UNIT   | SALE VALUE |     STATUS        ");
	Dsp_saverec ("NO|NO|                |                                        |ORDERED| B/ORD |  PRICE  |            |                   ");
	Dsp_saverec (" [REDRAW][PRINT][NEXT][PREV][END]");

	o_tot_val = 0.00;
	/*
	 * Display requisition lines. 
	 */
	cmrd_rec.hhrq_hash = hhrqHash;
	cmrd_rec.line_no = 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == hhrqHash)
	{
		/*
		 * Find ccmr record. 
		 */
		ccmr_rec.hhcc_hash = cmrd_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		/*
		 * Find inmr record. 
		 */
		inmr_rec.hhbr_hash	= cmrd_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		if ((cmrd_rec.qty_order + cmrd_rec.qty_border) > 0.00)
		{
			ord_qty = cmrd_rec.qty_order + cmrd_rec.qty_border;
			ord_val = out_cost (cmrd_rec.sale_price, 
					   inmr_rec.outer_size);
			ord_val -= CAL (ord_val, cmrd_rec.disc_pc);
			unit_price = ord_val;
			ord_val *= (double)ord_qty;
		
			/*
			 * Save line to Dsp window. 
			 */
			switch (cmrh_rec.stat_flag [0])
			{
			case 'R':
				strcpy (line_status, "Ready For Despatch");
				break;
		
			case 'B':
				strcpy (line_status, "Backordered       ");
				break;
		
			case 'F':
				strcpy (line_status, "Forward Ordered   ");
				break;
		
			case 'C':
				strcpy (line_status, "Complete          ");
				break;
			}
			sprintf (err_str, 
				"%2.2s^E%2.2s^E%-16.16s^E%-40.40s^E%7.2f^E%7.2f^E%9.2f^E%12.2f^E^1%-1.1s^6%-17.17s", 
				ccmr_rec.est_no, 
				ccmr_rec.cc_no, 
				inmr_rec.item_no, 
				inmr_rec.description, 
				cmrd_rec.qty_order, 
				cmrd_rec.qty_border, 
				DOLLARS (unit_price), 
				DOLLARS (ord_val), 
				line_status, 
				&line_status [1]);

			Dsp_saverec (err_str);

			o_tot_val += ord_val;
		}

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
	}

	Dsp_saverec ("^^GGJGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGJGGGGGGGJGGGGGGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGG^^");
	sprintf (err_str, 
		" TOTAL                                                                                   ^E%12.2f^E", 
		DOLLARS (o_tot_val));
	Dsp_saverec (err_str);

	Dsp_srch ();
	Dsp_close ();

	return (EXIT_SUCCESS);
}

/*
 * Display Transactions for contract. 
 */
int
TimeDisplay (void)
{
	int		first_trans;
	int		first_csthd;
	int		proc_equip;
	int		data_found;
	double	tot_time;
	double	tot_lab;
	double	tot_oh;
	double	tot_cost;
	double	csthd_total;
	double	all_total;
	char	units [8];
	char	time_ord [8];
	char	time_hlf [8];
	char	time_dbl [8];
	char	lab_unit_cst [11];
	char	oh_unit_cst [11];
	char	lab_cost [11];
	char	oh_cost [11];
	char	tot_cost_str [13];

	if (!nowPrinting)
	{
	    Dsp_open (1, 4, 10);
	    Dsp_saverec ("  Date  | Employee |Plant Code| Units | 1.0t  | 1.5t  | 2.0t  | Lab Cost | O/H Cost |  Labour  |   O/H    |  Total      ");
	    Dsp_saverec ("        |          |          | Used  | Hours | Hours | Hours | Per Unit | Per Unit |  Costs   |  Costs   |  Costs      ");
	    Dsp_saverec ("  [REDRAW][NEXT][PREV][END INPUT]   ");
	}

	all_total = 0.00;

	/*
	 * Process cmcm records. 
	 */
	first_csthd = TRUE;
	data_found = FALSE;
	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", " ");
	cc = find_rec (cmcm2, &cmcm_rec, GTEQ, "r");
	while (!cc && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
	    csthd_total = 0.00;

	    /*
	     * Process cmts records. 
	     */
	    first_trans = TRUE;
	    cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	    cmts_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	    cmts_rec.date = 0L;
	    cc = find_rec (cmts, &cmts_rec, GTEQ, "r");
	    while (!cc && 
		   cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		   cmts_rec.hhcm_hash == cmcm_rec.hhcm_hash)
	    {
		/*
		 * Look up employee record. 
		 */
		cmem_rec.hhem_hash	=	cmts_rec.hhem_hash;
		cc = find_rec (cmem, &cmem_rec, GTEQ, "r");
		if (cc)
		{
		    cc = find_rec (cmts, &cmts_rec, NEXT, "r");
		    continue;
		}
	
		/*
		 * Look up plant record if necessary. 
		 */
		proc_equip = FALSE;
		if (cmts_rec.hheq_hash == 0L)
		    sprintf (cmeq_rec.eq_name, "%-10.10s", " ");
		else
		{
			cmeq_rec.hheq_hash = cmts_rec.hheq_hash;
		    cc = find_rec (cmeq, &cmeq_rec, GTEQ, "r");
		    if (cc)
		    {
				cc = find_rec (cmts, &cmts_rec, NEXT, "r");
				continue;
		    }
		    proc_equip = TRUE;
		}
	
		/*
		 * Costhead Heading. 
		 */
		if (first_trans)
		{
		    /*
		     * Previous Costhead Ruleoff. 
		     */
		    if (!first_csthd)
		    {	
			if (!nowPrinting)
			{
			    Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGG^^");
			}
		    }
		    if (nowPrinting)
		    {
			fprintf (fout, 
				"|%-11.11s| Costhead : %-4.4s  %-40.40s %-65.65s|            |\n", 
				" ", 
				cmcm_rec.ch_code, 
				cmcm_rec.desc, 
				" ");
		    }
		    else
		    {
			sprintf (err_str, 
				"^1 Costhead : %-4.4s  %-40.40s ^6", 
				cmcm_rec.ch_code, 
				cmcm_rec.desc);
			Dsp_saverec (err_str);
		    }
		}
		first_csthd = FALSE;
		first_trans = FALSE;
	
		/*
		 * Units. 
		 */
		if (cmts_rec.units == 0.00)
		    strcpy (units, "       ");
		else
		    sprintf (units, "%7.2f", cmts_rec.units);

		/*
		 * Ordinary Time. 
		 */
		if (cmts_rec.time_ord == 0.00)
		    strcpy (time_ord, "       ");
		else
		    sprintf (time_ord, "%7.2f", cmts_rec.time_ord);

		/*
		 * Time and a Half
		 */
		if (cmts_rec.time_hlf == 0.00)
		    strcpy (time_hlf, "       ");
		else
		    sprintf (time_hlf, "%7.2f", cmts_rec.time_hlf);

		/*
		 * Double Time. 
		 */
		if (cmts_rec.time_dbl == 0.00)
		    strcpy (time_dbl, "       ");
		else
		    sprintf (time_dbl, "%7.2f", cmts_rec.time_dbl);

		/*
		 * Labour Unit Cost. 
		 */
		if (cmts_rec.lab_cost == 0.00)
		    strcpy (lab_unit_cst, "          ");
		else
		{
		    sprintf (lab_unit_cst, 
			    "%10.2f", 
			    DOLLARS (cmts_rec.lab_cost));
		}

		/*
		 * Overhead Unit Cost. 
		 */
		if (cmts_rec.oh_cost == 0.00)
		    strcpy (oh_unit_cst, "          ");
		else
		{
		    sprintf (oh_unit_cst, 
			    "%10.2f", 
			    DOLLARS (cmts_rec.oh_cost));
		}

		/*
		 * Calculate total hours. 
		 */
		if (proc_equip)
		    tot_time = cmts_rec.units;
		else
		{
		    tot_time = cmts_rec.time_ord +
			      (cmts_rec.time_hlf * 1.5) +
			      (cmts_rec.time_dbl * 2.0);
		}

		/*
		 * Labour Cost. 
		 */
		if (proc_equip)
		{
		    tot_lab = 0.00;
		    strcpy (lab_cost, "          ");
		}
		else
		{
		    tot_lab = tot_time * cmts_rec.lab_cost;
		    if (tot_lab == 0.00)
			strcpy (lab_cost, "          ");
		    else
		    {
			sprintf (lab_cost, 
				"%10.2f", 
				DOLLARS (tot_lab));
		    }
		}

		/*
		 * Overhead Cost. 
		 */
		tot_oh = tot_time * cmts_rec.oh_cost;
		if (tot_oh == 0.00)
		    strcpy (oh_cost, "          ");
		else
		{
		    sprintf (oh_cost, 
			    "%10.2f", 
			    DOLLARS (tot_oh));
		}

		/*
		 * TOTAL Cost. 
		 */
		tot_cost = tot_lab + tot_oh;
		if (tot_cost == 0.00)
		    strcpy (tot_cost_str, "            ");
		else
		{
		    sprintf (tot_cost_str, "%12.2f", DOLLARS (tot_cost));
		}

		/*
		 * Display String. 
		 */
		data_found = TRUE;
		if (nowPrinting)
		{		
		    fprintf (fout, ".LRP7\n");
			fprintf (fout, "|%-11.11s|%-10.10s", " ", DateToString (cmts_rec.date));
		    fprintf (fout, "|%-10.10s", cmem_rec.emp_no);
		    fprintf (fout, "|%-10.10s", cmeq_rec.eq_name);
		    fprintf (fout, "|%-7.7s", units);
		    fprintf (fout, "|%-7.7s", time_ord);
		    fprintf (fout, "|%-7.7s", time_hlf);
		    fprintf (fout, "|%-7.7s", time_dbl);
		    fprintf (fout, "| %10.10s", lab_unit_cst);
		    fprintf (fout, "| %10.10s", oh_unit_cst);
		    fprintf (fout, "| %10.10s", lab_cost);
		    fprintf (fout, "| %10.10s", oh_cost);
		    fprintf (fout, "| %12.12s|            |\n", tot_cost_str);
		}
		else
		{
		    sprintf (err_str, 
			    "%-10.10s^E%-10.10s^E%-10.10s^E%-7.7s^E%-7.7s^E%-7.7s^E%-7.7s^E%10.10s^E%10.10s^E%10.10s^E%10.10s^E%12.12s", 
			    DateToString (cmts_rec.date), 
			    cmem_rec.emp_no, 
			    cmeq_rec.eq_name, 
			    units, 
			    time_ord, 
			    time_hlf, 
			    time_dbl, 
			    lab_unit_cst, 
			    oh_unit_cst, 
			    lab_cost, 
			    oh_cost, 
			    tot_cost_str);
				
		    Dsp_saverec (err_str);
		}

		csthd_total += tot_cost;
	
		cc = find_rec (cmts, &cmts_rec, NEXT, "r");
	    }

	    /*
	     * Costhead Total. 
	     */
	    if (csthd_total != 0.00)
	    {
		if (nowPrinting)
		{
		    fprintf (fout, "|           |TOTAL FOR COSTHEAD:  ");
		    fprintf (fout, 
			    "%-4.4s  %-40.40s  %-40.40s ", 
			    cmcm_rec.ch_code, 
			    cmcm_rec.desc, 
			    " ");
		    fprintf (fout, 
			    "| %12.2f|%12.12s|\n", 
			    DOLLARS (csthd_total), 
			    " ");

		    fprintf (fout, "|           |");
		    fprintf (fout, "  %-120.120s  ", " ");
		    fprintf (fout, "|            |\n");
		}
		else
		{
		    Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGG^^");

		    sprintf (err_str, 
			    " TOTAL FOR COSTHEAD : %-4.4s  %-40.40s%-38.38s  ^E%12.2f", 
			    cmcm_rec.ch_code, 
			    cmcm_rec.desc, 
			    " ", 
			    DOLLARS (csthd_total));
		    Dsp_saverec (err_str);
		}
		
		all_total += csthd_total;
	    }

	    cc = find_rec (cmcm2, &cmcm_rec, NEXT, "r");
	}

	/*
	 * Timesheets Total. 
	 */
	if (all_total != 0.00)
	{
	    if (nowPrinting)
	    {
		fprintf (fout, 
			"|           | GRAND TOTAL OF EMPLOYEE / PLANT TIME  %-70.70s | %12.2f|            |\n", 
			" ", 
			DOLLARS (all_total));
	    }
	    else
	    {
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGG^^");
		
		sprintf (err_str, 
			" GRAND TOTAL : %-90.90s   ^E%12.2f", 
			" ", 
			DOLLARS (all_total));
		Dsp_saverec (err_str);
	
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGG^^");
	    }
	}

	if (nowPrinting)
	{
		if (!data_found)
		{
			fprintf (fout, 
				"|           | %-122.122s |            |\n", 
				"                                 ****     NO EMPLOYEE / PLANT TIME FOR CONTRACT     **** ");
		}
	}
	else
	{
	    Dsp_srch ();
	    Dsp_close ();
	
	    ReDraw ();
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Extended Description for contract. 
 */
int
XDescDisp (void)
{
	/*
	 * Open Dsp window. 
	 */
	if (!nowPrinting)
	{
		Dsp_open (1, 4, 10);
		sprintf (err_str, 
			"%-20.20s Ongoing Description For Contract Number %-6.6s %-20.20s", 
			" " , 
			cmhr_rec.cont_no, 
			" ");
		Dsp_saverec (err_str);
		Dsp_saverec ("");
		Dsp_saverec ("    [NEXT][PREV][END INPUT]     ");
	}

	/*
	 * Lookup description. 
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 7;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc &&
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       !strcmp (cmcd_rec.stat_flag, "D"))
	{
		if (nowPrinting)
		{
			fprintf (fout, ".LRP3\n");
			fprintf (fout, 
				"|%-38.38s %-70.70s  %-38.38s|\n", 
				" ", 
				cmcd_rec.text, 
				" ");
		}
		else
		{
			sprintf (err_str, "   %-70.70s   ", cmcd_rec.text);
			Dsp_saverec (err_str);
		}

		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}

	if (!nowPrinting)
	{
		Dsp_srch ();
		Dsp_close ();
		
		ReDraw ();
	}
	return (EXIT_SUCCESS);
}

/*
 * Clear and redraw the main screen. 
 */
int
RedrawDisplay (void)
{
	clear ();
	heading (1);
	AllDisplay ();
	return (EXIT_SUCCESS);
}

/*
 * redraw the main screen without clearing. 
 */
void
ReDraw (void)
{
	heading (1);

	move (0, 19);
	cl_line ();
	move (0, 20);
	cl_line ();

	AllDisplay ();
}

/*
 * Move to next contract on file. 
 */
void
NextContract (void)
{
}

/*
 * Move to prev contract on file. 
 */
void
PrevContract (void)
{
}

/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input                       
 */
int
heading (
 int	scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	if (clearOK)
	{
		swide ();
		clear ();
	}

	rv_pr (ML (mlCmMess061), (130 - strlen (ML (mlCmMess061))) / 2, 0, 1);
	line_at (1, 0, 132);

	if (scn == 1)
	{
		box (0, 2, 131, 15);
	}

	if (scn == 2)
	{
		box (0, 3, 130, 13);

		line_at (6, 1,129);
		line_at (9, 1,129);
		line_at (12,1,129);
		line_at (15,1,129);
	}

	if (scn == 3)
	{
		cl_box (1, 4, 50, 3);

		us_pr (ML (mlCmMess062), 15, 4, 1);
	}

	line_at (21,0,132);
	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 0, err_str, comm_rec.co_no, clip (comm_rec.co_name));
	strcpy (err_str, ML (mlStdMess039));
	print_at (22, 45, err_str, comm_rec.est_no, clip (comm_rec.est_name));

	scn_write (scn);
	return (EXIT_SUCCESS);
}

/*
 * Print missing graphics characters. 
 */
void
PrintMissGraphics (void)
{
	crsr_off ();
	move (87, 4);   PGCHAR (8);
	move (60, 12);  PGCHAR (8);
	move (0, 12);   PGCHAR (10);
	move (87, 12);  PGCHAR (9);
	move (130, 12); PGCHAR (11);
	move (60, 18);  PGCHAR (9);
}

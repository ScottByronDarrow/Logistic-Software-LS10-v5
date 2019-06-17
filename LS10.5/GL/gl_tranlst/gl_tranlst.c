/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_tranlst.c,v 5.7 2002/07/17 09:57:20 scott Exp $
|  Program Name  : (gl_tranlst.c)
|  Program Desc  : (Display and Prints Summarised listing of)
|                (specified range of accounts)
|---------------------------------------------------------------------|
|  Date Written  : (07/08/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_tranlst.c,v $
| Revision 5.7  2002/07/17 09:57:20  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.6  2002/01/25 05:01:15  scott
| Updated to add clear_mess ();
|
| Revision 5.5  2001/09/12 09:15:35  robert
| updated to correct tab display in LS10.5-GUI
|
| Revision 5.4  2001/09/11 09:04:30  robert
| updated to correct hot_key display in LS10.5-GUI
|
| Revision 5.3  2001/08/09 09:13:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:37  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:02  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_tranlst.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_tranlst/gl_tranlst.c,v 5.7 2002/07/17 09:57:20 scott Exp $";

#define	PSL_PRINT
#define	X_OFF		20
#define	Y_OFF		5
#define	SEL_PSIZE		11

#include	<pslscr.h>
#include	<tabdisp.h>
#include	<GlUtils.h>
#include	<getnum.h>
#include	<hot_keys.h>
#include	<get_lpno.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;

#define MONEY (dollarAmt) ( (dollarAmt) / DOLLARS (1.0))
#define NFORM_LEN	 (FORM_LEN - 31)
#define	MAX_JNL	26

#define		SORTBYNARRATIVE			1
#define		SORTBYUSERREF			2
#define		SORTBYJNLTYPE			3

#define		CONS_NONE		0
#define		CONS_HEADER		1
#define		CONS_DETAIL		2

static	char		lastString	[61];
/*
 *	Structure for dynamic array,  for the shipment lines for qsort
 */
struct Trans
{
	char	sortKey1	[61];	/*	Narrative		*/
	char	sortKey2	[61];	/*	User Reference	*/
	char	sortKey3	[61];	/*	Journal Type.	*/
	char	narrative 	[21];	
	char	jnlType 	[10];
	char	userRef		[16];
	char	currCode	[4];
	int		consolFlag;
	int		expandFlag;
	double	locAmount;
	double	fgnAmount;
	float	exchRate;
	long	gltrHash;
	long	hhmrHash;
	long	tranDate;
}	*tranRec;
	DArray tranDetails;
	int	tranCnt = 0;

	char	*data	= "data";

	extern	int	GV_cur_level,
				GV_max_level,
				tab_max_page;

	FILE	*fout;

	int		PV_curr_fyear	 = 0,
			PV_budget		 = 0,
			print_ok 		 = FALSE,
			PV_print 		 = FALSE,
			printerNo 		 = 1,
			envGlTranlstSubt = 2,
			inDetail		 = FALSE,
			ln_num 			 = 0;

static	int	currentSort		=	SORTBYJNLTYPE,
			expanded		=	FALSE,
			compressed		=	TRUE,
			sortChanged		=	FALSE,
			firstTime		= 	TRUE,
			moreThanOneLine	=	-1,
			transFound 		= 	FALSE;

	char	prog_path [150],
			PV_end_acc	 [FORM_LEN + 1],
			prev_acc	 [FORM_LEN + 1],
			PV_start_acc [FORM_LEN + 1],
			classDesc [4];

	double	locOpeningBalance	=	0.00,
			locMonthBalance		=	0.00,
			locClosingBalance	=	0.00,
			fgnOpeningBalance	=	0.00,
			fgnMonthBalance		=	0.00,
			fgnClosingBalance	=	0.00,
			locSubTotal			= 	0.00,
			fgnSubTotal			= 	0.00,
			workValue			=	0.00;
struct
{
	char	loc_start [FORM_LEN + 1],
			loc_end [FORM_LEN + 1],
			loc_acct_type [2];
	int		loc_year,
			loc_start_prd,
			loc_end_prd;
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "acc_no",	 2, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "          ",
		" ", "0", "Start Account    ", " ",
		 NE, NO,  JUSTLEFT, "0123456789*-", "", local_rec.loc_start},
	{1, LIN, "end_acc",	 2, 70, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "          ",
		" ", "9999999999999999", "End Account      ", " ",
		 NE, NO,  JUSTLEFT, "0123456789*-", "", local_rec.loc_end},
	{1, LIN, "start_prd",	 3, 2, INTTYPE,
		"NN", "          ",
		" ", "0", "Start Period     ", " ",
		 NE, NO,  JUSTLEFT, "0", "12", (char *) &local_rec.loc_start_prd},
	{1, LIN, "end_prd",	 3, 70, INTTYPE,
		"NN", "          ",
		" ", "0", "End Period       ", " ",
		 NE, NO,  JUSTLEFT, "0", "12", (char *) &local_rec.loc_end_prd},
	{1, LIN, "year",	 4, 2, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Year             ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.loc_year},
	{1, LIN, "act_typ",	 4, 70, CHARTYPE,
		"U", "          ",
		" ", "F", "Account Type     ", "Enter either F)inancial or N)on financial",
		 NE, NO,  JUSTLEFT, "FN", "", local_rec.loc_acct_type},
	{0, TAB, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL},
};

static	int	PrintTranFunc 		(int, KEY_TAB *);
static	int	DisplayTranFunc 	(int, KEY_TAB *);
static	int	YearFunc 			(int, KEY_TAB *);
static	int	HistoryFunc 		(int, KEY_TAB *);
static	int	TranDetailsFunc 	(int, KEY_TAB *);
static	int	RedrawFunc 			(int, KEY_TAB *);
/*
 * Local function prototypes 
 */

#ifdef	GVISION
KEY_TAB	transKeys [] =
{
	{ " DISPLAY ", 'D', DisplayTranFunc,
		"Display Summarised Account listing to screen." 	  },
	{ " PRINT ",	  'P', PrintTranFunc,
		"Print Summarised Account listing to printer."		  },
	{ " YEAR ",	  'Y', YearFunc,
		"Toggle year between Next / Previous year."		  },
    { " User Ref sort ",	'U', TranDetailsFunc,
	    "Sort list by user reference.",		    },
    { " Narrative sort ",	'N', TranDetailsFunc,
	    "Sort list by narrative.",		    },
    { " Journal Type sort ",	'J', TranDetailsFunc,
	    "Sort list by transaction Type.",		    },
	END_KEYS
};

static KEY_TAB tranDetailKeys [] =
{
   { " User Ref sort ",	'U', TranDetailsFunc,
	"Sort list by user reference.",		    },
   { " Narrative sort ",	'N', TranDetailsFunc,
	"Sort list by narrative.",		    },
   { " Journal Type sort ",	'J', TranDetailsFunc,
	"Sort list by journal Type.",		    },
   { " Consolidate Toggle ",	'C', TranDetailsFunc,
	"Toggles between Consolidate and no consolidate mode.",		    },
   { "Extra Description ", 'E', TranDetailsFunc,
    "View Extra Narrative details" },
   { "SubTotal [+] ", '+', TranDetailsFunc,
    "Add to subtotal lines." },
   { "SubTotal [-] ", '-', TranDetailsFunc,
    "Subtract from subtotal lines." },
   { NULL,		FN3,  RedrawFunc					    },
	   END_KEYS
};
#else
KEY_TAB	transKeys [] =
{
	{ "D)isplay ", 'D', DisplayTranFunc,
		"Display Summarised Account listing to screen." 	  },
	{ "P)rint ",	  'P', PrintTranFunc,
		"Print Summarised Account listing to printer."		  },
	{ "H)istory ", 'H', HistoryFunc,
		"Toggle year between Current / History years."		  },
	{ "Y)ear ",	  'Y', YearFunc,
		"Toggle year between Next / Previous year."		  },
    { "U)ser Ref sort",	'U', TranDetailsFunc,
	   "Sort list by user reference.",		    },
    { "N)arrative sort",	'N', TranDetailsFunc,
	   "Sort list by narrative.",		    },
    { "J)ournal Type sort",	'J', TranDetailsFunc,
	   "Sort list by transaction type.",		    },
	END_KEYS
};
static KEY_TAB tranDetailKeys [] =
{
   { "U)ser Ref sort",	'U', TranDetailsFunc,
	"Sort list by user reference.",		    },
   { "N)arrative sort",	'N', TranDetailsFunc,
	"Sort list by narrative.",		    },
   { "J)ournal Type",	'J', TranDetailsFunc,
	"Sort list by transaction type.",		    },
   { "C)onsolidate Toggle ",	'C', TranDetailsFunc,
	"Toggles between Consolidate and no consolidate mode.",		    },
   { "E)xtra Description ", 'E', TranDetailsFunc,
    "View Extra Narrative details" },
   { "ST [+] ", '+', TranDetailsFunc,
    "Add to subtotal lines." },
   { "ST [-] ", '-', TranDetailsFunc,
    "Subtract from subtotal lines." },
   { NULL,		FN3,  RedrawFunc					    },
	   END_KEYS
};
#endif
/*
 * Local Function Prototypes.
 */
int 	heading 					(int);
int 	spec_valid 					(int);
static	char	*SetAmount 			(double);
static	char	*SetUnderline 		(double);
static 	int 	RunTransHots 		(void);
static 	int 	SetEndAccount 		(void);
static 	int 	SetStartAccount 	(void);
static	int		TranSort1		 	(const	void *,	const void *);
static	int		TranSort2		 	(const	void *,	const void *);
static	int		TranSort3		 	(const	void *,	const void *);
static	void 	CheckBreak 			(int,int);
static	void 	CloseDB 			(void);
static	void	DisplayGltr 		(int, int);
static	void	DisplayInfo 		(void);
static 	void	DisplayTranHots 	(void);
static	void 	InitPrint 			(void);
static	void 	OpenDB 				(void);
static	void 	PrintAccountNo 		(void);
static	void	ProcessClosing 		(void);
static	void 	ProcessFile 		(void);
static	void	ProcessOpening 		(long, long);
static	void 	ProcessTrans 		(long, int, int, int);
static	void	Ruler 				(void);
void 			psl_print 			(void);
void 			shutdown_prog 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	sptr = getenv ("PROG_PATH");
	sprintf (prog_path, "%s", (sptr) ? sptr : "/usr/LS10.5");

	tab_max_page = 5000;

	/*
	 * Check for number of transactions before sub total.
	 */
	sptr = chk_env ("GL_TRANLST_SUBT");
	envGlTranlstSubt = (sptr == (char *)0) ? 2 : atoi (sptr);

	search_ok 		= TRUE;
	TruePosition	= TRUE;

	if (getenv ("BUDGET"))
		PV_budget = atoi (getenv ("BUDGET"));

	OpenDB ();

	set_help (FN6, "FN6");
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (strcmp (sptr, "gl_tranprt"))
	{
		init_scr ();		/*  sets terminal from termcap	  */
		set_tty ();

		SETUP_SCR (vars);
		if (argc == 2 && ! (printerNo = atoi (argv [1])))
			printerNo = 1;

		vars [label ("acc_no")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
		vars [label ("end_acc")].mask = vars [label ("acc_no")].mask;

		set_masks ();		/*  setup print using masks	  */
		swide ();

		do
		{
			prog_exit = restart = 0;
			init_vars (1);	/*  set default values		  */
			PV_curr_fyear = local_rec.loc_year = fisc_year (comm_rec.gl_date);
			heading (1);
			init_ok = FALSE;
			entry (1);
			init_ok = TRUE;
			if (restart)
				continue;

			if (!prog_exit)
			{
				clear ();
				fflush (stdout);
				crsr_off ();
				RunTransHots ();
			}
		} while (!prog_exit);
	}
	else
	{
		print_mess (ML ("General Ledger Transaction Listing "));
		GL_SetAccWidth (comm_rec.co_no, TRUE);
		PV_print = TRUE;
		strcpy (local_rec.loc_start, argv [1]);
		strcpy (local_rec.loc_end, argv [2]);
		local_rec.loc_year 		= atoi (argv [3]);
		local_rec.loc_start_prd = atoi (argv [4]);
		local_rec.loc_end_prd 	= atoi (argv [5]);
		strcpy (local_rec.loc_acct_type, argv [6]);
		printerNo = atoi (argv [7]);

		InitPrint ();
		ProcessFile ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

static int
DisplayTranFunc (
 int                iUnused,
 KEY_TAB*           psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /*GVISION*/

	ProcessFile ();
	DisplayTranHots ();
    return (iUnused);
}

static int
PrintTranFunc (
 int                iUnused,
 KEY_TAB*           psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /*GVISION*/

	char	sys_str [128];

	printerNo = get_lpno (0);

	sprintf (sys_str, "gl_tranprt \"%s\" \"%s\" %d %d %d \"%s\" %d",
		local_rec.loc_start, local_rec.loc_end,
		local_rec.loc_year,
		local_rec.loc_start_prd, local_rec.loc_end_prd,
		local_rec.loc_acct_type, printerNo);

	PrintReport (sys_str, "Account Transaction Listing", 132);
	heading (1);
	DisplayTranHots ();
    return (iUnused);
}
static void
InitPrint (void)
{
	/*
	 * Heading and Page Format For Printer Output Display.
	 */
	if (! (fout = popen ("pformat", "w")))
		file_err (errno, "pformat", "POPEN");

	/*
	 * Start output to standard print.
	 */

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".PL60\n");
	fprintf (fout, ".11\n");
	fprintf (fout, ".L140\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".B1\n");

	fprintf (fout,".ETRANSACTION LISTING - %-2.2s  %s\n",
				comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".Efor Period %02d/%04d to %02d/%04d %s\n",
			local_rec.loc_start_prd, local_rec.loc_year,
			local_rec.loc_end_prd, local_rec.loc_year,
			 (currentSort == SORTBYJNLTYPE) ? "- Sorted by Transaction Type" :
			 (currentSort == SORTBYNARRATIVE) ? "- Sorted by Narrative" :
			 (currentSort == SORTBYUSERREF) ? "- Sorted by User Reference" : " ");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R====================================================================================================================================\n");
	fprintf (fout, "====================================================================================================================================\n");
	fprintf (fout, "|Date       Trans narrative.     Jnl. type User Reference             Debit Amount      Credit Amount Each rate Cur  Foreign amount|\n");
	fprintf (fout, "|----------------------------------------------------------------------------------------------------------------------------------|\n");
}

static	int		s_cnt = 1;

static	void
ProcessFile (void)
{
	int		first_time = 1,
			len,
			printed = 0;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&tranDetails, &tranRec, sizeof (struct Trans), 1000);
	tranCnt 	= 0;

	s_cnt = 1;
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.glmr_class [0], "F");
	SetEndAccount ();
	SetStartAccount ();

	cc = find_rec (glmr, &glmrRec, GTEQ , "r");
	len = strlen (GL_GetBit (GV_cur_level));

	while (!cc &&
		!strcmp (glmrRec.co_no, comm_rec.co_no) &&
		strcmp (glmrRec.acc_no, PV_end_acc) <= 0)
	{
		if (glmrRec.glmr_class [0][0] == local_rec.loc_acct_type [0] &&
		    glmrRec.glmr_class [2][0] == 'P')
		{
			if (first_time)
				strcpy (prev_acc, glmrRec.acc_no);

			PrintAccountNo ();
			printed = 1;
			first_time = 0;

			if (!strcmp (local_rec.loc_start, local_rec.loc_end))
				break;
		}
		cc = find_rec (glmr, &glmrRec, NEXT , "r");
	}

	while (1)
	{
		inDetail	= TRUE,
		sortChanged	= FALSE;
		if (!PV_print)
		{
			tab_open ("tabGltr", tranDetailKeys, 6, 0, 12, FALSE);
			tab_add ("tabGltr",
			 "#Date       Trans narrative.     Jnl. type User Reference             Debit Amount      Credit Amount Each rate Cur  Foreign amount");
		 }

		DisplayGltr (expanded, compressed);

		if (!PV_print)
		{
			if (!tab_display ("tabGltr", TRUE))
				tab_scan ("tabGltr");

			tab_close ("tabGltr", TRUE);

			if (sortChanged == FALSE)
				break;
		}
		else
			break;
	}
	inDetail	= FALSE,

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&tranDetails);
}


static	void
PrintAccountNo (void)
{
	char	form_acc [FORM_LEN + 1];

	strcpy (form_acc, glmrRec.acc_no);
	GL_FormAccNo (form_acc, glmrRec.acc_no, 0);

	ProcessTrans 
	(
		glmrRec.hhmr_hash,
		local_rec.loc_year,
		local_rec.loc_start_prd,
		local_rec.loc_end_prd
	);
}

static	void	
ProcessTrans (
	long	hhmrHash,
	int     yearNo,
	int     startPeriod,
	int     endPeriod)
{
	int		journalNo	=	0,
			fmonth		=	0, 
			fyear		=	0;

	static	char 	*jtype [MAX_JNL + 1] =
			{
			  "Unknown. ", "General  ", "Standing ",
			  "Accrual  ", "Sales    ", "Credits  ",
			  "Receipts ", "Payables ", "Returns  ",
			  "Dispur.  ", "Inventory", "Purchases",
			  "Stk. Adj.", "Cost Sale", "Stk Count",
			  "Exch Var.", "Cust. JNL", "Supp. Jnl",
			  "Bank Tran", "P-Iss/Rec", "Bills Rev",
			  "Cont. Lab", "Cont. Mat", "Cont. Com", 
			  "Cont. Adj", "Fwd. Rec.", "Sund. Rec"
			} ;

	gltrRec.hhmr_hash	=	hhmrHash;
	gltrRec.tran_date	= 	FinDMYToDate 
							(
								comm_rec.fiscal, 
								1,
								startPeriod,
								yearNo
							);
	DateToFinDMY 
	(
		gltrRec.tran_date, 
		comm_rec.fiscal, 
		NULL, 
		&fmonth, 
		&fyear
	);
	cc = find_rec (gltr, &gltrRec, GTEQ,"r");
	if (cc || gltrRec.hhmr_hash != glmrRec.hhmr_hash)
	{
		/*
		 * Check the array size before adding new element.
	     */
		if (!ArrChkLimit (&tranDetails, tranRec, tranCnt))
			sys_err ("ArrChkLimit (tranRec)", ENOMEM, PNAME);

		journalNo = 0;
		/*
		 * Load values into array element tranCnt.
		 */
		sprintf 
		(
			tranRec [tranCnt].sortKey1, 
			"%-16.16s%20.20s%010ld000", 
			glmrRec.acc_no,
			" ",
			-1L
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey2, 
			"%-16.16s%15.15s%010ld000",
			glmrRec.acc_no,
			" ",
			-1L
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey3, 
			"%-16.16s%10.10s%010ld000",
			glmrRec.acc_no,
			" ",
			-1L
		);
		tranRec [tranCnt].exchRate	= 0.00;
		tranRec [tranCnt].locAmount	= 0.00;
		tranRec [tranCnt].fgnAmount	= 0.00;
		strcpy (tranRec [tranCnt].narrative, " ");
		strcpy (tranRec [tranCnt].jnlType,   " ");
		strcpy (tranRec [tranCnt].userRef,	 " ");
		strcpy (tranRec [tranCnt].currCode,	 " ");
		tranRec [tranCnt].expandFlag	=	FALSE;
		tranRec [tranCnt].consolFlag	=	CONS_NONE;
		tranRec [tranCnt].gltrHash		=	0L;
		tranRec [tranCnt].hhmrHash		=	glmrRec.hhmr_hash;
		tranRec [tranCnt].tranDate		=	0L;
		/*
		 * Increment array counter.
	   	 */
		tranCnt++;
		return;
	}

	DateToFinDMY 
	 (
		gltrRec.tran_date, 
		comm_rec.fiscal, 
		NULL, 
		&fmonth, 
		&fyear
	);
	while (!cc && gltrRec.hhmr_hash == hhmrHash && 
				  fmonth <= endPeriod && fyear == yearNo)
	{
		if ((journalNo = atoi (gltrRec.tran_type)) > MAX_JNL)
			journalNo = 0;

		/*
		 * Check the array size before adding new element.
	     */
		if (!ArrChkLimit (&tranDetails, tranRec, tranCnt))
			sys_err ("ArrChkLimit (tranRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element tranCnt.
		 */
		sprintf 
		(
			tranRec [tranCnt].sortKey1, 
			"%-16.16s%-20.20s%010ld000", 
			glmrRec.acc_no,
			gltrRec.narrative,
			gltrRec.gltr_hash
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey2, 
			"%-16.16s%-15.15s%010ld000",
			glmrRec.acc_no,
			gltrRec.user_ref,
			gltrRec.gltr_hash
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey3, 
			"%-16.16s%-10.10s%010ld000",
			glmrRec.acc_no,
			jtype [journalNo],
			gltrRec.gltr_hash
		);

		tranRec [tranCnt].exchRate	=	gltrRec.exch_rate;
		tranRec [tranCnt].locAmount	=	gltrRec.amount;
		tranRec [tranCnt].fgnAmount = 	gltrRec.amt_origin;
		strcpy (tranRec [tranCnt].narrative,  gltrRec.narrative);
		strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
		strcpy (tranRec [tranCnt].userRef	, gltrRec.user_ref);
		strcpy (tranRec [tranCnt].currCode,	  gltrRec.currency);
		tranRec [tranCnt].expandFlag	=	FALSE;
		if (strncmp (gltrRec.narrative, "CP ", 3))
			tranRec [tranCnt].consolFlag	=	CONS_NONE;
		else
			tranRec [tranCnt].consolFlag	=	CONS_HEADER;

		tranRec [tranCnt].gltrHash		=	gltrRec.gltr_hash;
		tranRec [tranCnt].hhmrHash		=	gltrRec.hhmr_hash;
		tranRec [tranCnt].tranDate		=	gltrRec.tran_date;

		/*
		 * Increment array counter.
	   	 */
		tranCnt++;

		gltcRec.gltr_hash	=	gltrRec.gltr_hash;
		gltcRec.tran_date	=	0L;
		cc = find_rec (gltc, &gltcRec, GTEQ, "r");
		while (!cc && gltcRec.gltr_hash	== gltrRec.gltr_hash)
		{
			if ((journalNo = atoi (gltcRec.tran_type)) > MAX_JNL)
				journalNo = 0;

			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&tranDetails, tranRec, tranCnt))
				sys_err ("ArrChkLimit (tranRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element tranCnt.
			 */
			sprintf 
			(
				tranRec [tranCnt].sortKey1, 
				"%-16.16s%-20.20s%010ld000", 
				glmrRec.acc_no,
				gltcRec.narrative,
				gltcRec.gltr_hash
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey2, 
				"%-16.16s%-15.15s%010ld000",
				glmrRec.acc_no,
				gltcRec.user_ref,
				gltcRec.gltr_hash
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%-16.16s%-10.10s%010ld000",
				glmrRec.acc_no,
				jtype [journalNo],
				gltcRec.gltr_hash
			);

			tranRec [tranCnt].exchRate	=	gltcRec.exch_rate;
			tranRec [tranCnt].locAmount	=	gltcRec.amount;
			tranRec [tranCnt].fgnAmount	=	gltcRec.amt_origin;
			strcpy (tranRec [tranCnt].narrative,  gltcRec.narrative);
			strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
			strcpy (tranRec [tranCnt].userRef	, gltcRec.user_ref);
			strcpy (tranRec [tranCnt].currCode,	  gltcRec.currency);
			tranRec [tranCnt].expandFlag	=	FALSE;
			tranRec [tranCnt].consolFlag	=	CONS_DETAIL;
			tranRec [tranCnt].hhmrHash		=	gltrRec.hhmr_hash;
			tranRec [tranCnt].gltrHash		=	gltcRec.gltr_hash;
			tranRec [tranCnt].tranDate		=	gltcRec.tran_date;
			/*
			 * Increment array counter.
			 */
			tranCnt++;

			cc = find_rec (gltc, &gltcRec, NEXT, "r");
		}

		glnaRec.gltr_hash	=	gltrRec.gltr_hash;
		glnaRec.line_no		=	0;
		cc = find_rec (glna, &glnaRec, GTEQ, "r");
		while (!cc && glnaRec.gltr_hash	== gltrRec.gltr_hash)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&tranDetails, tranRec, tranCnt))
				sys_err ("ArrChkLimit (tranRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element tranCnt.
			 */
			sprintf 
			(
				tranRec [tranCnt].sortKey1, 
				"%-16.16s%-20.20s%010ld%03d",
				glmrRec.acc_no,
				gltrRec.narrative, 
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey2, 
				"%-16.16s%-15.15s%010ld%03d",
				glmrRec.acc_no,
				gltrRec.user_ref, 
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%-16.16s%-10.10s%010ld%03d",
				glmrRec.acc_no,
				jtype [journalNo],
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);

			tranRec [tranCnt].exchRate	=	0.00;
			tranRec [tranCnt].locAmount	=	0.00;
			tranRec [tranCnt].fgnAmount	=	0.00;
			strcpy (tranRec [tranCnt].narrative,  glnaRec.narrative);
			strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
			strcpy (tranRec [tranCnt].userRef	, gltrRec.user_ref);
			strcpy (tranRec [tranCnt].currCode,	  " ");
			tranRec [tranCnt].expandFlag	=	TRUE;
			tranRec [tranCnt].consolFlag	=	CONS_NONE;
			tranRec [tranCnt].hhmrHash		=	gltrRec.hhmr_hash;
			tranRec [tranCnt].gltrHash		=	gltrRec.gltr_hash;
			tranRec [tranCnt].tranDate		=	gltrRec.tran_date;
			/*
		 	 * Increment array counter.
	   	 	 */
			tranCnt++;
			cc = find_rec (glna, &glnaRec, NEXT, "r");
		}

		cc = find_rec (gltr, &gltrRec, NEXT,"r");
		DateToFinDMY 
		 (
			gltrRec.tran_date, 
			comm_rec.fiscal, 
			NULL, 
			&fmonth, 
			&fyear
		);
	}   /* end of while loop */
}

/*
 * Open data base files.
 */
static	void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlmr (); abc_selfield (glmr, "glmr_id_no2");
	OpenGlpd ();
	OpenGltr (); abc_selfield (gltr, "gltr_id_no2");
	OpenGltc ();
	OpenGlna ();
}

/*
 * Close data base files
 */
static	void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose (data);
	/*
	if (PV_tfile != NULL)
	{
		fclose (PV_tfile);
		unlink (PV_tran_file);
	}
	*/
}

void
shutdown_prog (void)
{
	if (PV_print)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
	else
	{
		clear ();
		snorm ();
		rset_tty ();
		crsr_on ();
	}
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("acc_no")) 
	{
		if (vars [label ("acc_no")].mask [0] == '*')
		{
			print_mess (ML (mlGlMess001));
			restart = TRUE;
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.loc_start, GL_GetDfltSfaccCode ());
		
		if (SRCH_KEY)
		{
			abc_selfield (glmr, "glmr_id_no");
			if (!strlen (temp_str))
				strcpy (temp_str, GL_GetfUserCode ());
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			abc_selfield (glmr, "glmr_id_no2");
		}
		return (SetStartAccount ());
	}

	if (LCHECK ("end_acc")) 
	{
		if (SRCH_KEY)
		{
			abc_selfield (glmr, "glmr_id_no");
			if (!strlen (temp_str))
				strcpy (temp_str, GL_GetfUserCode ());
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			abc_selfield (glmr, "glmr_id_no2");
		}
		if (dflt_used)
		{
			if (!strcmp (glmrRec.acc_no, GL_GetDfltSaccCode ()))
				strcpy (local_rec.loc_end, GL_GetDfltEfaccCode ());
			else
				strcpy (local_rec.loc_end, local_rec.loc_start);
		}
		return (SetEndAccount ());
	}

	if (LCHECK ("start_prd") && dflt_used)
	{
		DateToFinDMY (comm_rec.gl_date,
					  comm_rec.fiscal,
					  NULL, &local_rec.loc_start_prd, NULL);
		DSP_FLD ("start_prd");
	}
	
	if (LCHECK ("end_prd") && dflt_used)
	{
		DateToFinDMY (comm_rec.gl_date,
					  comm_rec.fiscal,
					  NULL, &local_rec.loc_end_prd, NULL);
		DSP_FLD ("end_prd");
	}
	return (EXIT_SUCCESS);
}

static int
SetStartAccount (void)
{
	if (GL_FormAccNo (local_rec.loc_start, glmrRec.acc_no, 0))
		return (EXIT_FAILURE);
	strcpy (PV_start_acc, glmrRec.acc_no);

	return (EXIT_SUCCESS);
}

static int
SetEndAccount (void)
{
	if (GL_FormAccNo (local_rec.loc_end, PV_end_acc, 0))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
static int
RunTransHots (void)
{
	heading (1);

	restart		= FALSE;
	inDetail	= FALSE,
	DisplayTranHots ();
	if (run_hotkeys (transKeys, null_func, null_func))
		restart = TRUE;

	return (FALSE);
}

static	void
DisplayTranHots (void)
{
	line_at (21,0,132);
	inDetail	= FALSE,
	disp_hotkeys (21, 0, 132, transKeys);
}

static int
YearFunc (
 int                iUnused,
 KEY_TAB*           psUnused)
{
	local_rec.loc_year += (local_rec.loc_year == PV_curr_fyear) ? 1 :
			    (local_rec.loc_year < PV_curr_fyear)  ?
				PV_curr_fyear - local_rec.loc_year : -1 ;
	display_field (label ("year"));
	return (TRUE);
}

static	int
HistoryFunc (
 int                iUnused,
 KEY_TAB*           psUnused)
{
	static	int	hist_incr = 0;

	if (!glctRec.history)
	{
		print_mess (ML (mlGlMess137));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	if (glctRec.history == 1)
		local_rec.loc_year = PV_curr_fyear - 1;

	else if (local_rec.loc_year >= PV_curr_fyear)
	{
		hist_incr = -1;
		local_rec.loc_year = PV_curr_fyear;
	}
	else if (local_rec.loc_year == PV_curr_fyear - 1)
		hist_incr = -1;

	else if (local_rec.loc_year == PV_curr_fyear - glctRec.history)
		hist_incr = 1;

	local_rec.loc_year += hist_incr;

	display_field (label ("year"));

	return (TRUE);
}

void
psl_print (void)
{
	if (!print_ok)
		return;

	printerNo = get_lpno (0);
#ifndef GVISION
	rv_pr (ML (mlStdMess035), 22, 6, 1);
#endif	/* GVISION */
	
	PV_print = TRUE;
	InitPrint ();
	ProcessFile ();
	fprintf (fout, ".EOF\n");
	pclose (fout);
	PV_print = FALSE;
}

static	int
TranDetailsFunc (
 int      c, 
 KEY_TAB *psUnused)
{
	if (c == 'N')
	{
		currentSort	=	SORTBYNARRATIVE;
		sortChanged	=	TRUE;
		DisplayInfo ();
		return ((inDetail) ? FN1 : c);
	}
	if (c == 'U')
	{
		currentSort	=	SORTBYUSERREF;
		sortChanged	=	TRUE;
		DisplayInfo ();
		return ((inDetail) ? FN1 : c);
	}
	if (c == 'J')
	{
		currentSort	=	SORTBYJNLTYPE;
		sortChanged	=	TRUE;
		DisplayInfo ();
		return ((inDetail) ? FN1 : c);
	}
	if (c == 'E')
	{
		if (expanded == TRUE)
			expanded = FALSE;
		else
			expanded = TRUE;

		sortChanged	= TRUE;
		DisplayInfo ();
		return ((inDetail) ? FN1 : c);
	}
	if (c == 'C')
	{
		if (compressed 	== FALSE)
			compressed 	= TRUE;
		else
			compressed 	= FALSE;

		sortChanged	= TRUE;
		DisplayInfo ();
		return ((inDetail) ? FN1 : c);
	}
	if (c == '+')
	{
		envGlTranlstSubt++;
		DisplayInfo ();
		sortChanged	=	TRUE;
		return ((inDetail) ? FN1 : c);
	}
	if (c == '-')
	{
		envGlTranlstSubt--;
		if (envGlTranlstSubt < 2)
			envGlTranlstSubt = 2;
		DisplayInfo ();
		sortChanged	=	TRUE;
		return ((inDetail) ? FN1 : c);
	}
	sortChanged	=	FALSE;
	return (EXIT_SUCCESS);
}

static	void
DisplayGltr (
	int		expand,
	int		compressed)
{
	int		i;
	char	tranDate [11];
	char	workAmount [21];

	long	lastHhmrHash	=	0;

	strcpy (lastString, " ");
	moreThanOneLine = 0;


	firstTime		=	TRUE;
	abc_selfield (glmr, "glmr_hhmr_hash");
	/*
	 * Sort the array depending on sort key.
	 */
	if (currentSort	== SORTBYNARRATIVE)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort1);
	else if (currentSort	== SORTBYUSERREF)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort2);
	else if (currentSort	== SORTBYJNLTYPE)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort3);

	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (i = 0; i < tranCnt; i++)
	{
		if (expand == FALSE && tranRec [i].expandFlag == TRUE)
			continue;

		if (compressed == TRUE && tranRec [i].consolFlag == CONS_DETAIL)
			continue;

		if (compressed == FALSE && tranRec [i].consolFlag == CONS_HEADER)
			continue;
		
		if (lastHhmrHash != tranRec [i].hhmrHash)
		{
			if (firstTime == FALSE && transFound == TRUE)
			{
				CheckBreak (FALSE, i);
				ProcessClosing ();
			}
			
			if (firstTime == FALSE)
			{
				if (tranRec [i-1].gltrHash == 0L && tranRec [i].gltrHash != 0L)
					Ruler ();
			}

			ProcessOpening  (tranRec [i].hhmrHash, tranRec [i].gltrHash);
		}
		lastHhmrHash = tranRec [i].hhmrHash;

		if (tranRec [i].gltrHash != 0L)
		{
			transFound = TRUE;
			strcpy (tranDate,DateToString (tranRec [i].tranDate));

			strcpy (workAmount, comma_fmt (DOLLARS (tranRec [i].fgnAmount),"NNNN,NNN,NNN.NN"));
			if (tranRec [i].expandFlag == FALSE)
			{
				CheckBreak (firstTime, i);
				if (PV_print)
				{
        			fprintf 
					(
						fout, 
						"|%-10.10s %-20.20s %-9.9s %-20.20s %-37.37s %9.4f %-3.3s %15.15s|\n",
						tranDate,
						tranRec [i].narrative, 
						tranRec [i].jnlType,
						tranRec [i].userRef,
						SetAmount (DOLLARS (tranRec [i].locAmount)),
						tranRec [i].exchRate,
						tranRec [i].currCode,
						workAmount
					); 
				}
				else
				{
        			tab_add 
					(
						"tabGltr", 
						"%-10.10s %-20.20s %-9.9s %-20.20s %-37.37s %9.4f %-3.3s %15.15s",
						tranDate,
						tranRec [i].narrative, 
						tranRec [i].jnlType,
						tranRec [i].userRef,
						SetAmount (DOLLARS (tranRec [i].locAmount)),
						tranRec [i].exchRate,
						tranRec [i].currCode,
						workAmount
					); 
				}
			}
			else
			{
				if (PV_print)
				{
        			fprintf 
					(
						fout, 
						"|%10.10s %-20.20s %-98.98s|\n",
						" ",
						tranRec [i].narrative, 
						" "
					);
				}
				else
				{
        			tab_add 
					(
						"tabGltr", 
						"%10.10s %-20.20s %-98.98s",
						" ",
						tranRec [i].narrative, 
						" "
					);
				}

			}
		}
		else
		{
			transFound = FALSE;
		}

		firstTime	=	FALSE;
	}
	abc_selfield (glmr, "glmr_id_no2");
}

static	void
CheckBreak (
	int		firstTime,
	int		i)
{
	int		changeRecord	=	FALSE;
	char	workAmount [21];

	if (firstTime)
	{
		if (currentSort == SORTBYNARRATIVE)
			strcpy (lastString, tranRec [i].narrative);
		else if (currentSort == SORTBYUSERREF)
			strcpy (lastString, tranRec [i].userRef);
		else if (currentSort == SORTBYJNLTYPE)
			strcpy (lastString, tranRec [i].jnlType);

		moreThanOneLine = -1;
	}
	changeRecord	=	FALSE;

	if (currentSort == SORTBYNARRATIVE)
	{
		if (strncmp (lastString, tranRec [i].narrative, 20))
			changeRecord	=	TRUE;
	}
	else if (currentSort == SORTBYUSERREF)
	{
		if (strcmp (lastString, tranRec [i].userRef))
			changeRecord	=	TRUE;
	}
	else if (currentSort == SORTBYJNLTYPE)
	{
		if (strcmp (lastString, tranRec [i].jnlType))
			changeRecord	=	TRUE;
	}
	if (changeRecord == TRUE)
	{
		if (moreThanOneLine >= (envGlTranlstSubt - 1))
		{
			if (PV_print)
			{
				fprintf 
				(
					fout,
					"|%-62.62s %-37.37s %13.13s %15.15s|\n",
					" ",
					SetUnderline (DOLLARS (locSubTotal)),
					" ",
					"---------------"
				);
			}
			else
			{
				tab_add 
				(
					"tabGltr", 
					"%-62.62s %-37.37s %13.13s %15.15s",
					" ",
					SetUnderline (DOLLARS (locSubTotal)),
					" ",
					"---------------"
				);
			}

			strcpy (workAmount, comma_fmt (DOLLARS (fgnSubTotal),"NNNN,NNN,NNN.NN"));
			if (PV_print)
			{
				fprintf 
				(
					fout, 
					"|%-62.62s %-37.37s %13.13s %15.15s|\n",
					" ",
					SetAmount (DOLLARS (locSubTotal)),
					" ",
					workAmount
				);
				fprintf 
				(
					fout, 
					"|%-62.62s %-37.37s %13.13s %15.15s|\n",
				" ",
					SetUnderline (DOLLARS (locSubTotal)),
					" ",
					"---------------"
				);
			}
			else
			{
				tab_add 
				(
					"tabGltr", 
					"%-62.62s %-37.37s %13.13s %15.15s",
					" ",
					SetAmount (DOLLARS (locSubTotal)),
					" ",
					workAmount
				);
				tab_add 
				(
					"tabGltr", 
					"%-62.62s %-37.37s %13.13s %15.15s",
				" ",
					SetUnderline (DOLLARS (locSubTotal)),
					" ",
					"---------------"
				);
			}
		}
		locSubTotal	= 0.00;
		fgnSubTotal	= 0.00;
		moreThanOneLine = 0;
	}
	else
		moreThanOneLine++;
	
	locSubTotal	+= tranRec [i].locAmount;
	fgnSubTotal	+= tranRec [i].fgnAmount;

	if (currentSort == SORTBYNARRATIVE)
		strcpy (lastString, tranRec [i].narrative);
	else if (currentSort == SORTBYUSERREF)
		strcpy (lastString, tranRec [i].userRef);
	else if (currentSort == SORTBYJNLTYPE)
		strcpy (lastString, tranRec [i].jnlType);
}
		
static	void
Ruler (void)
{
	if (PV_print)
	{
		fprintf 
		(
			fout, 
			"|__________________________________________________________________________________________________________________________________|\n"
		);
	}
	else
	{
		tab_add 
		(
			"tabGltr",
			"__________________________________________________________________________________________________________________________________"
		);
	}
}

static	void
ProcessOpening (
	long	hhmrHash,
	long	gltrHash)
{
	char	form_acc [FORM_LEN + 1];
	char	workStr [21];

	glmrRec.hhmr_hash	=	hhmrHash;
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		return;
	

	strcpy (form_acc, glmrRec.acc_no);
	GL_FormAccNo (form_acc, glmrRec.acc_no, 0);

	sprintf (classDesc, "%c%c%c",	glmrRec.glmr_class [0][0],
									glmrRec.glmr_class [1][0],
									glmrRec.glmr_class [2][0]);

	locOpeningBalance	=	GL_LocTotGlpd 
							(
								glmrRec.hhmr_hash,
								PV_budget,
								local_rec.loc_year,
								1,
								local_rec.loc_start_prd - 1
							);
	locMonthBalance		=	GL_LocTotGlpd 
							(
								glmrRec.hhmr_hash,
								PV_budget,
								local_rec.loc_year,
								local_rec.loc_start_prd,
								local_rec.loc_end_prd
							);
	fgnOpeningBalance	=	GL_FgnTotGlpd 
							(
								glmrRec.hhmr_hash,
								PV_budget,
								local_rec.loc_year,
								1,
								local_rec.loc_start_prd - 1
							);
	fgnMonthBalance		=	GL_FgnTotGlpd 
							(
								glmrRec.hhmr_hash,
								PV_budget,
								local_rec.loc_year,
								local_rec.loc_start_prd,
								local_rec.loc_end_prd
							);
	locClosingBalance 	= locOpeningBalance + locMonthBalance;
	fgnClosingBalance 	= fgnOpeningBalance + fgnMonthBalance;
	strcpy (workStr, 
			comma_fmt (DOLLARS (fgnOpeningBalance), "NNNN,NNN,NNN.NN"));
			
	if (!gltrHash)
	{
		strcpy (workStr, 
			comma_fmt (DOLLARS (fgnClosingBalance), "NNNN,NNN,NNN.NN"));
		if (PV_print)
		{
			fprintf (fout, ".LRP8\n");
			fprintf 
			(
				fout, 
				"|%-18.18s - %25.25s (%3.3s)          %37.37s %14.14s %15.15s|\n",
				form_acc,
				glmrRec.desc,
				classDesc,
				SetAmount (DOLLARS (locClosingBalance)),
				"no movements   ",
				workStr
			);
			fprintf 
			(
				fout, 
				".PD|%-18.18s - %25.25s (%3.3s)          %37.37s %14.14s %15.15s|\n",
				form_acc,
				glmrRec.desc,
				classDesc,
				SetAmount (DOLLARS (locClosingBalance)),
				"no movements   ",
				workStr
			);
		}
		else
		{
			tab_add 
			(
				"tabGltr", 
				"%-18.18s - %25.25s (%3.3s)          %37.37s %14.14s %15.15s",
				form_acc,
				glmrRec.desc,
				classDesc,
				SetAmount (DOLLARS (locClosingBalance)),
				"no movements   ",
				workStr
			);
		}
	}
	else
	{	
		if (PV_print)
		{
			fprintf 
			(
				fout, 
				"|%-18.18s - %-25.25s (%3.3s) %77.77s|\n",
				form_acc,
				glmrRec.desc,
				classDesc,
				" "
			);
			fprintf 
			(
				fout, 
				"|%43.43s OPENING BALANCE    %37.37s %13.13s %15.15s|\n",
				" ",
				SetAmount (DOLLARS (locOpeningBalance)),
				" ",
				workStr
			);
		}
		else
		{
			tab_add 
			(
				"tabGltr", 
				"%-18.18s - %-25.25s (%3.3s)",
				form_acc,
				glmrRec.desc,
				classDesc
			);
			tab_add 
			(
				"tabGltr",
				"%43.43s OPENING BALANCE    %37.37s %13.13s %15.15s",
				" ",
				SetAmount (DOLLARS (locOpeningBalance)),
				" ",
				workStr
			);
		}
	}
}

static	void
ProcessClosing (void)
{
	char	workStr [21];

	strcpy (workStr, 
			comma_fmt (DOLLARS (fgnClosingBalance),"NNNN,NNN,NNN.NN"));
	if (PV_print)
	{
		fprintf 
		(
			fout, 
			"|%43.43s CLOSING BALANCE    %37.37s %13.13s %15.15s|\n",
			" ",
			SetAmount (DOLLARS (locClosingBalance)),
			" ",
			workStr
		);
	}
	else
	{
		tab_add 
		(
			"tabGltr",
			"%43.43s CLOSING BALANCE    %37.37s %13.13s %15.15s",
			" ",
			SetAmount (DOLLARS (locClosingBalance)),
			" ",
			workStr
		);
	}
	Ruler ();
}
static	int
RedrawFunc (
int		c, 
	KEY_TAB *psUnused)
{

	last_char = REDRAW;
	return (c);
}

static	void
DisplayInfo (void)
{
	strcpy (err_str, (expanded == TRUE) ? ML ("Extra Description On ") 
										: ML("Extra Description Off"));
	rv_pr (err_str, 90,5,1);

	strcpy (err_str, (compressed == TRUE) 	? ML ("Consolidate On ") 
											: ML("Consolidate Off"));
	rv_pr (err_str, 70,5,1);

	sprintf (err_str, ML ("Subtotal on %d like transactions"),envGlTranlstSubt);
	rv_pr (err_str, 34,5,1);

	if (currentSort == SORTBYNARRATIVE)
		rv_pr(ML(mlGlMess120), 2, 5, 1);
	else if (currentSort == SORTBYUSERREF)
		rv_pr(ML(mlGlMess117), 2, 5, 1);
	else if (currentSort == SORTBYJNLTYPE)
		rv_pr(ML(mlGlMess118), 2, 5, 1);
	else
		rv_pr(ML(mlGlMess119), 2, 5, 1);
}
			 
static char	*
SetAmount (
	double	setAmount)
{
	static	char	tmp_str	[141];

	if (setAmount < 0.00)
	{
		sprintf (tmp_str, "                   %18.18s", 
							comma_fmt (-setAmount,"NNN,NNN,NNN,NNN.NN"));
	}
	else
	{
		sprintf (tmp_str, "%18.18s                   ", 
							comma_fmt (setAmount, "NNN,NNN,NNN,NNN.NN"));
	}
	return (tmp_str);
}
static char	*
SetUnderline (
	double	setAmount)
{
	static	char	tmp_str	[141];

	if (setAmount < 0.00)
		strcpy (tmp_str, "                   ------------------");
	else
		strcpy (tmp_str, "------------------                   ");

	return (tmp_str);
}

/*
 * Sort routines. See TranRec structure for details.
 */
static	int 
TranSort1 (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct Trans a = * (const struct Trans *) a1;
	const struct Trans b = * (const struct Trans *) b1;

	result = strcmp (a.sortKey1, b.sortKey1);

	return (result);
}
/*
 * Sort routines. See TranRec structure for details.
 */
static	int 
TranSort2 (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct Trans a = * (const struct Trans *) a1;
	const struct Trans b = * (const struct Trans *) b1;

	result = strcmp (a.sortKey2, b.sortKey2);

	return (result);
}
/*
 * Sort routines. See TranRec structure for details.
 */
static	int 
TranSort3 (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct Trans a = * (const struct Trans *) a1;
	const struct Trans b = * (const struct Trans *) b1;

	result = strcmp (a.sortKey3, b.sortKey3);

	return (result);
}
/*
 * Display Screen Heading 
 */
int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	centre_at (0, 132, ML (mlGlMess115));
	box (0, 1, 132, 3);

	line_at (21,0,132);
	
	print_at ( 22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	scn_display (scn);
    return (EXIT_SUCCESS);
}


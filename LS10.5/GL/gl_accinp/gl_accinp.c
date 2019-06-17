/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_accinp.c,v 5.10 2002/07/25 11:17:28 scott Exp $
|  Program Name  : (gl_accinp.c) 
|  Program Desc  : (General Ledger Account Maintenance)
|---------------------------------------------------------------------|
| $Log: gl_accinp.c,v $
| Revision 5.10  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.9  2002/07/09 04:02:56  scott
| S/C 004047 - When the Consolidation Toggle is clicked at detail, a title will be displayed at upper right corner, then press F1 to cancel, or click on [End] buttons to go back to header and choose another account number record, the display at upper right corner stays.
|
| Revision 5.8  2002/01/25 03:40:01  scott
| Updated as message not cleared.
|
| Revision 5.7  2001/09/26 07:12:57  robert
| Updated to fixed memory dump problem in LS10-GUI
|
| Revision 5.6  2001/08/28 08:46:01  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.5  2001/08/23 11:50:22  scott
| Updated from scotts machine
|
| Revision 5.4  2001/08/20 23:12:41  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 09:13:19  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:26:58  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:20  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_accinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_accinp/gl_accinp.c,v 5.10 2002/07/25 11:17:28 scott Exp $";

/*
 * Include file dependencies.
 */
#include <pslscr.h>
#include <getnum.h>
#include <tabdisp.h>
#include <hot_keys.h>
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <GlUtils.h>
#include <arralloc.h>

#define		SORTBYNARRATIVE			1
#define		SORTBYSYSREF			2
#define		SORTBYUSERREF			3
#define		SORTBYTRANDATE			4

#define		CONS_NONE		0
#define		CONS_HEADER		1
#define		CONS_DETAIL		2

static	int	currentSort	=	SORTBYTRANDATE,
			expanded	=	FALSE,
			compressed	=	TRUE,
			sortChanged	=	FALSE;

double		tranTotal	=	0.00;

/*
 * Constants, defines and stuff 
 */
#define	GL_BALANCES		1
#define	GL_BREAKDOWN	2

#define	LINK_CREATE	'C'
#define	LINK_DELETE	'R'

#define	MAX_JNL		26

#define	GET_ACC(x)	 ((GLMR_STRUCT *) GL_GetAccount (x))

extern	int	GV_link_cnt, GV_cur_level, GV_max_level;
extern	int tab_max_page;

char 	*gltr2 	= "gltr2",
		*glpd2 	= "glpd2",
		*glmr2 	= "glmr2",
		*data  	= "data";

#include	"schema"

struct	commRecord	comm_rec;
struct	comrRecord	comr_rec;

/*
 * Local variables.
 */
static	int	PV_budg_no 		= 0,
			PV_curr_fyear	= 0,
			PV_display 		= FALSE,
			PV_disp_type 	= GL_BALANCES,
			PV_glpd_display	= 0,
			PV_new_account	= 0,
			companySelect	= FALSE;


GLMR_STRUCT	glmrRec2;
GLMR_STRUCT	*glmrRec3;
GLMR_STRUCT	glmrRec4;
GLLN_STRUCT	gllnRec2;
GLTR_STRUCT gltrRec2;

extern	int		TruePosition;

/*
 *	Structure for dynamic array,  for the shipment lines for qsort
 */
struct Trans
{
	char	sortKey1	[41];	/*	gltrHash + narrative	*/
	char	sortKey2	[41];	/*	gltrHash + sysRef		*/
	char	sortKey3	[41];	/*	gltrHash + userRef		*/
	char	sortKey4	[41];	/*	gltrHash + tranDate		*/
	char	narrative 	[21];	
	char	jnlType 	[10];
	char	sysRef 		[11];
	char	userRef		[16];
	char	currCode	[4];
	int		consolFlag;
	int		expandFlag;
	char	locAmount	[21];	
	char	fgnAmount	[21];
	char	exchRate	[21];
	long	gltrHash;
	long	tranDate;
	long	postDate;
}	*tranRec;
	DArray tranDetails;
	int	tranCnt = 0;

static struct
{
	  char	locAccountNo 	[FORM_LEN + 1],
			locClass 		[4],
			locClassDesc 	[25],
			locYearType 	[10],
			loc_bdg_no 		[3],
			locCurrCode 	[4],
			locCurrDesc 	[41],
			systemAccount	[2];
	  int	locYear;
} local_rec;
		
static	struct var vars [] =
{
	{1, LIN, "accountNo",	2, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ",
		" ", " ", "Account Number      ", " ",
		 NE, NO,  JUSTLEFT, "0123456789*-", "", local_rec.locAccountNo},
	{1, LIN, "accountDesc",	2, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "Description         ", "Enter Account Description ",
		YES, NO,  JUSTLEFT, "", "", glmrRec.desc},
	{1, LIN, "class",	3, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Class               ", "CC - FLS, FAS - FIS, FES - FLP, FAP, FIP, FEP | NFC - NFS - NFP",
		YES, NO,  JUSTLEFT, "ACEFILNPS", "", local_rec.locClass},
	{1, LIN, "classDesc",	3, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAA", "              ",
		" ", " ", "Description         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.locClassDesc},
	{1, LIN, "systemAccount",	3, 100, CHARTYPE,
		"U", "             ",
		" ", "M", "System Posting      ", "Enter S(ystem or machine generated posting account. M(anual) posted account. ",
		 YES, NO, JUSTLEFT, "MS", "", (char *) local_rec.systemAccount},
	{1, LIN, "bdgtno",	4, 50, INTTYPE,
		"NN", "             ",
		" ", local_rec.loc_bdg_no, "Budget Number       ", " ",
		 NA, NO, JUSTLEFT, "", "", (char *) &PV_budg_no},
	{1, LIN, "year",		4, 2, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Year                ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.locYear} ,
	{1, LIN, "currCode",	4, 80, CHARTYPE,
		"UUU", "             ",
		" ", " ", "Currency - ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.locCurrCode},
	{1, LIN, "currDesc",	4, 100, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.locCurrDesc},

	{2, LIN, "companyNo",	 2, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Company Number       ", "Enter Company Number. [SEARCH] available ",
		 NE, NO, JUSTRIGHT, "1", "99", comr_rec.co_no},
	{2, LIN, "companyName",	 2, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", comr_rec.co_name},
	{2, LIN, "coCurrCode",	2, 90, CHARTYPE,
		"UUU", "             ",
		" ", " ", "Currency             ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.locCurrCode},
	{2, LIN, "coCurrDesc",	2, 100, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "                          ",
		"", "", " ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.locCurrDesc},
	{0, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

/*
 * Local function prototypes 
 */
int		TranDetailsFunc 	(int, KEY_TAB *);
int		balanceFunc 		(int, KEY_TAB *);
int		EditFunc 			(int, KEY_TAB *);
int		YearFunc 			(int, KEY_TAB *);
int		HistoryFunc 		(int, KEY_TAB *);
int		BreakdownFunc 		(int, KEY_TAB *);
int		TransactionFunc 	(int, KEY_TAB *);
int		NextFunc 			(int, KEY_TAB *);
int		BudgetFunc 			(int, KEY_TAB *);
int		TranscationFunc 	(int, KEY_TAB *);
int		LinkFunc 			(int, KEY_TAB *);
int		SelectLinks 		(int, KEY_TAB *);
int		MaintainLinks 		(int, KEY_TAB *);
int		TagLink 			(int, KEY_TAB *);
int		RedrawFunc 			(int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB bal_keys [] =
{
   { " TRANSACTIONS ",	'T', TransactionFunc,
	"Display transactions for current account.",			"BT" },
   { " BREAKDOWN ",	'B', BreakdownFunc,
	"Display account breakdown for current account.",		"B" },
   { " BALANCES ",	'B', balanceFunc,
	"Display balances for the current account.",			"b" },
   { " HISTORY ",	'H', HistoryFunc,
	"Display historical balances for the current account.",		    },
   { " YEAR " ,		'Y', YearFunc,
	"Display Next / Previous year's balances for the current account."  },
   { " BUDGET ",	'G', BudgetFunc,
	"Change the currently selected buget number.",			    },
   { " EDIT ",		'E', EditFunc,
	"Edit the current account.",			    		"M" },
   { " LINK ",		'L', LinkFunc,
	"Create statistical links for the current account.",		"L" },
   { " NEXT ",	  	FN14, NextFunc,
	"Display the next account."			    		    },
   { " PREV ",		FN15, NextFunc,
	"Display the previous account."			       		    },
   { NULL,		FN3,  RedrawFunc					    },
   END_KEYS
};
static KEY_TAB tran_keys [] =
{
    { " RETURN ", '\r', TranscationFunc,
      "View the transactions for the current period. (All if BALANCE)" },
	   END_KEYS
};
static KEY_TAB tranDetailKeys [] =
{
   { " User Ref sort ",	'U', TranDetailsFunc,
	"Sort list by user reference.",		    },
   { " Journal Ref sort ",	'J', TranDetailsFunc,
	"Sort list by Journal reference.",		    },
   { " Narrative sort ",	'N', TranDetailsFunc,
	"Sort list by narrative.",		    },
   { " Date sort ",	'D', TranDetailsFunc,
	"Sort list by transaction date.",		    },
   { " Consolidate Toggle ",	'C', TranDetailsFunc,
	"Toggles between Consolidate and no consolidate mode.",		    },
   { "Extra Description ", 'E', TranDetailsFunc,
    "View Extra Narrative details" },
   { NULL,		FN3,  RedrawFunc					    },
	   END_KEYS
};

static KEY_TAB link_keys [] =
{
	   { " DISPLAY ", 'D',		MaintainLinks,
		"Display statistical links for current account." },
	   { " CREATE ",  LINK_CREATE,	SelectLinks,
		"Crete statistical links for current account."	 },
	   { " REMOVE ",  LINK_DELETE,	MaintainLinks,
		"Remove statistical links from current account." },
	   END_KEYS
};

static KEY_TAB slct_keys [] =
{
	   { " Accept All ",'A',TagLink, "Tag/Un-tag all accounts in list." },
	   { " TAG ",  'T',      TagLink, "Tag/Un-tag current account."      },
	   END_KEYS
};
#else
static KEY_TAB bal_keys [] =
{
   { " [T]RANSACTIONS ",	'T', TransactionFunc,
	"Display transactions for current account.",			"BT" },
   { " [B]REAKDOWN ",	'B', BreakdownFunc,
	"Display account breakdown for current account.",		"B" },
   { " [B]ALANCES ",	'B', balanceFunc,
	"Display balances for the current account.",			"b" },
   { " [H]ISTORY ",	'H', HistoryFunc,
	"Display historical balances for the current account.",		    },
   { " [Y]EAR ",		'Y', YearFunc,
	"Display Next / Previous year's balances for the current account."  },
   { "BUD[G]ET ",	'G', BudgetFunc,
	"Change the currently selected buget number.",			    },
   { " [E]DIT ",		'E', EditFunc,
	"Edit the current account.",			    		"M" },
   { " [L]INK ",		'L', LinkFunc,
	"Create statistical links for the current account.",		"L" },
   { " [NEXT] ",	  	FN14, NextFunc,
	"Display the next account."			    		    },
   { " [PREV] ",		FN15, NextFunc,
	"Display the previous account."			       		    },
   { NULL,		FN3,  RedrawFunc					    },
   END_KEYS
};

static KEY_TAB tranDetailKeys [] =
{
   { " U)ser Ref sort",	'U', TranDetailsFunc,
	"Sort list by user reference.",		    },
   { " J)ournal Ref sort",	'J', TranDetailsFunc,
	"Sort list by system reference.",		    },
   { " N)arrative sort",	'N', TranDetailsFunc,
	"Sort list by narrative.",		    },
   { " D)ate sort",	'D', TranDetailsFunc,
	"Sort list by narrative.",		    },
   { " C)onsolidate Toggle ",	'C', TranDetailsFunc,
	"Toggles between Consolidate and no consolidate mode.",		    },
   { "E)xtra Description ", 'E', TranDetailsFunc,
    "View Extra Narrative details" },
   { NULL,		FN3,  RedrawFunc					    },
	   END_KEYS
};
static KEY_TAB tran_keys [] =
{
    { " [RETN] ", '\r', TranscationFunc,
      "View the transactions for the current period. (All if BALANCE)" },
	   END_KEYS
};

static KEY_TAB link_keys [] =
{
	   { " [D]ISPLAY ", 'D',		MaintainLinks,
		"Display statistical links for current account." },
	   { " [C]REATE ",  LINK_CREATE,	SelectLinks,
		"Create statistical links for current account."	 },
	   { " [R]EMOVE ",  LINK_DELETE,	MaintainLinks,
		"Remove statistical links from current account." },
	   END_KEYS
};

static KEY_TAB slct_keys [] =
{
	   { " [A]ll Tag ", 'A',TagLink, "Tag/Un-tag all accounts in list." },
	   { " [T]AG Line ",  'T',      TagLink, "Tag/Un-tag current account."      },
	   END_KEYS
};
#endif

static KEY_TAB null_keys [] =
{
	   END_KEYS
};

static struct
{
    char *classCode,	/*  Class code           */
		 *classDesc,   	/*  Class Description    */
		 *extent,       /*  Class extent (0123)  */
		 *type;         /*  Valid owner type     */
} *class_ptr, class_tab [] = 
	{
		{"CC ",	"Company Control",			"C",	NULL	},
		{"NFC",	"Non-financial Control",	"T",	NULL	},
		{"NFS",	"Non-financial Summary",	"TMB",	"F"		},
		{"NFP",	"Non-financial Posting",	"TMB",	"F"		},
		{"FAS",	"Assets Summary",			"TM",	"LA"	},
		{"FES",	"Expense Summary",			"TM",	"LEIA"	},
		{"FIS",	"Income Summary",			"TM",	"IL"	},
		{"FLS",	"Liability Summary",		"TM",	"L"		},
		{"FAP",	"Assets Posting",			"TMB",	"LA"	},
		{"FEP",	"Expense Posting",			"TMB",	"ILEA"	},
		{"FIP",	"Income Posting",			"TMB",	"IL"	},
		{"FLP",	"Liability Posting",		"TMB",	"L"		},
		{NULL}
	};

/*
 * Local function prototypes  
 */
int  	AddLink 			(int);
int		CheckInterface 		(char *);
int  	DeleteLink 			(int);
int  	ErrorKidClass 		(char *);
int  	heading 			(int scn);
int  	LinkOK 				(void);
int  	LoadBalances 		(long);
int  	LookupAccountNo 	(void);
int  	NoChildren 			(void);
int  	NoFChildren 		(void);
int		NoGltr 				(void);
int  	NoNChildren 		(void);
int  	ReadGlmrRecord 		(GLMR_STRUCT *, char *, int);
int  	ReadLink 			(void);
int  	RunGlacHots 		(void);
int  	spec_valid 			(int);
int		TranSort1		 	(const	void *,	const void *);
int		TranSort2		 	(const	void *,	const void *);
int		TranSort3		 	(const	void *,	const void *);
int		TranSort4		 	(const	void *,	const void *);
int  	UpdateBalances 		(int, int, char *, char *, long, long);
int  	UpdateGlmr 			(void);
int  	ValidateAccountNo	(int);
int  	ValidateClass 		(int, char *, int);
int  	ValidateClassType	(void);
int  	ValidateCurrCode 	(void);
int  	ValidateExtent 		(void);
int  	ValidateFKidClass	(void);
int  	ValidateKidClass	(void);
int  	ValidateNKidClass 	(void);
int  	ValidateYear 		(int);
void 	ActionList 			(int);
void 	CloseDB 			(void);
void 	DecodeGlmr 			(void);
void 	DisplayAccount 		(int);
void 	DisplayBalances		(void);
void 	DisplayBreak 		(void);
void 	DisplayGlacHots 	(void);
void 	DisplayTable 		(void);
void 	DisplayTrans 		(char *);
void	DisplayGltr 		(int, int);
void	DisplayInfo 		(void);
void 	EntryFunc 			(void);
void 	LoadTrans 			(long, int, int);
void 	OpenDB 				(void);
void 	RepostTrans 		(long, char *, long, char *);
void 	RestoreGlpd 		(void);
void 	RunLinkHots 		(void);
void    SrchComr 			(char *);
void 	SetAll 				(void);
void 	SetAmounts 			(char *, char *, double, char *, char *, double);
void 	SetGlacKeys 		(char *);
void 	ptab_scan (void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char 	*argv [])
{
	char	*programNamePtr;
	int     crsr_stat;

	search_ok 		= TRUE;
	TruePosition	= TRUE;
	tab_max_page = 1000;
	
	if (getenv ("BUDGET"))
	{
		PV_budg_no = atoi (getenv ("BUDGET"));
		sprintf (local_rec.loc_bdg_no, "%-2.2d", PV_budg_no);
	}

	/*
	 *	Turn off edit key for display only program.
	 */
	programNamePtr = strrchr (*argv, '/');
	if (!programNamePtr)
    {
		programNamePtr = *argv;
    }
	if (!strcmp (programNamePtr, "gl_accdsp"))
	{
		PV_display = TRUE;
		set_keys (bal_keys, "M", KEY_PASSIVE);
	}
	if (!strcmp (programNamePtr, "gl_coaccdsp"))
	{
		PV_display 		= TRUE;
		companySelect	= TRUE;
		set_keys (bal_keys, "M", KEY_PASSIVE);
	}

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	  */
	set_tty ();
	OpenDB ();

	vars [label ("accountNo")].mask = GL_SetAccWidth (comr_rec.co_no, TRUE);
	set_masks ();			/*  setup print using masks	  */
	swide ();

	crsr_stat = crsr_toggle (FALSE);
	clear ();

	set_help (FN6, "FN6");
	/*
	 * Beginning of input control loop
	 */
	while (!prog_exit)
	{
		entry_exit		=	FALSE;
		edit_exit		=	FALSE;
		search_ok		=	TRUE;
		init_ok			=	TRUE;

		if (companySelect == TRUE)
		{
			last_char	=	REDRAW;
			init_vars (2);
			scn_set (2);
	
			heading (2);
			scn_display (2);
			entry (2);
	
			if (prog_exit || restart)
				break;
	
			heading (2);
			scn_display (2);
			edit (2);

			if (restart)
				break;

			last_char	=	REDRAW;
			companySelect = FALSE;
		}
		/*
		 * Reset Control Flags            
		 */
		PV_disp_type = GL_BALANCES;
		set_keys (bal_keys, "B", KEY_ACTIVE);
		set_keys (bal_keys, "b", KEY_PASSIVE);

		entry_exit = edit_exit = prog_exit = restart = 0;
		PV_new_account = -1;
		PV_glpd_display = TRUE;

		EntryFunc ();

		if (prog_exit)
			break;

		if (restart)
            continue;

		if (PV_new_account)
			EditFunc (0, (KEY_TAB *) NULL);

		if (!restart)
			RunGlacHots ();
	}
	crsr_toggle (TRUE);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
CloseDB (void)
{
	tab_close ("tabGlpd", TRUE);

	GL_PostTerminate ();

	abc_fclose (gltr);
	abc_fclose (gltr2);
	abc_fclose (glln);
	abc_fclose (glmr2);
	abc_fclose (comr);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	if (LCHECK ("accountNo"))
		return (ValidateAccountNo (field));
	
	if (LCHECK ("class")) 
		return (ValidateClass (field, local_rec.locClass, TRUE));

	if (LCHECK ("year"))
		return ValidateYear (local_rec.locYear);

	if (LCHECK ("currCode"))
		return ValidateCurrCode ();

	if (LCHECK ("systemAccount"))
	{
		if (local_rec.systemAccount [0] == 'M')
		{
			if (CheckInterface (GET_ACC (GV_cur_level-1)->acc_no))
			{
				print_mess (ML (mlGlMess182));
				sleep (sleepTime);
				clear_mess ();
				strcpy (local_rec.systemAccount, "S");
				DSP_FLD ("systemAccount");
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("companyNo"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
			
    	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess129));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.locCurrCode, "%-3.3s", comr_rec.base_curr);
		cc = FindPocr (comr_rec.co_no, local_rec.locCurrCode, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.locCurrDesc, "%-25.25s", pocrRec.description);
		DSP_FLD ("coCurrCode");
		DSP_FLD ("coCurrDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ValidateCurrCode (void)
{
	if (dflt_used)
		sprintf (local_rec.locCurrCode, "%-3.3s", comr_rec.base_curr);

	if (SRCH_KEY)
	{
		SearchPocr (comr_rec.co_no, temp_str);
		return (EXIT_SUCCESS);
	}

	cc = FindPocr (comr_rec.co_no, local_rec.locCurrCode, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess040));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	
	sprintf (local_rec.locCurrDesc, "%-25.25s", pocrRec.description);
	DSP_FLD ("currCode");
	DSP_FLD ("currDesc");
	return (EXIT_SUCCESS);
}

int	
ValidateAccountNo (
 int field)
{
	if (vars [label ("accountNo")].mask [0] == '*')
	{
		print_mess (ML (mlGlMess001));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	if (dflt_used)
		strcpy (local_rec.locAccountNo, GL_GetDfltSaccCode ());

	if (SRCH_KEY)
	{
		if (strlen (temp_str) == 0)
			strcpy (temp_str, GL_GetfUserCode ());
        
		return SearchGlmr_F (comr_rec.co_no, temp_str, "***");
	}

	if (GL_FormAccNo (local_rec.locAccountNo, glmrRec.acc_no, 0))
		return (EXIT_FAILURE);
    
	if (!ReadGlmrRecord (&glmrRec, PV_display ? "r" : "u", COMPARISON))
	{
		cc = FindPocr (comr_rec.co_no, glmrRec.curr_code, "r");
		if (cc)
		{
			strcpy (local_rec.locCurrCode, " ");
			strcpy (local_rec.locCurrDesc, " ");
		}
		else
		{
			strcpy (local_rec.locCurrCode, pocrRec.code);
			sprintf (local_rec.locCurrDesc,"%-25.25s", pocrRec.description);
		}
		if (glmrRec.system_acc [0] != 'S')
			strcpy (local_rec.systemAccount, "M");

		PV_new_account = FALSE;
		entry_exit = TRUE;
	}
	else
	{
		if (PV_display)
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.locClass [0] != 'N' && GV_cur_level > 1)
		{
			if (GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec))
				return (EXIT_FAILURE);

			cc = CheckInterface (GET_ACC (GV_cur_level-1)->acc_no);
			if (cc)
			{
				sprintf (err_str, ML (mlGlMess177),
						GET_ACC (GV_cur_level-1)->acc_no,
						glihRec.int_code);
			    print_err (err_str);
				sleep (sleepTime);
			    clear_mess ();
				return (EXIT_FAILURE);
			}
			/*
			if (GET_ACC (GV_cur_level-1)->glmr_class [2][0] != 'P')
			{
			    print_err (ML (mlStdMess025));
				sleep (sleepTime);
			    clear_mess ();
			}
			*/
		}
		entry_exit = FALSE;
		PV_new_account = TRUE;
	}
	
	if (PV_new_account)
		FLD ("currCode") = YES;
	else
	{
		if (NoGltr ())
			FLD ("currCode") = YES;
		else
			FLD ("currCode") = NA;
	}

	goto_field (field, label ("class"));  /*  Skip description */

	return (EXIT_SUCCESS);
}

int	
ValidateClass (
 int   field,       /* unreferenced formal parameter */
 char *classCode, 
 int   err_flag)
{
	int	cnt = 0;

	if (err_flag && !NoChildren () && * (classCode + 2) == 'P')
		return print_err (ML (mlGlMess149));

	if (!PV_new_account)
    {
        if (!ValidateKidClass ())
			return (EXIT_FAILURE);
    }

	for (class_ptr = class_tab; class_ptr->classCode; class_ptr++)
    {
		if (!strcmp (class_ptr->classCode, classCode))
		{
			if (ValidateExtent () || LookupAccountNo ())
				return (EXIT_FAILURE);

			sprintf (local_rec.locClassDesc, "%-21.21s", class_ptr->classDesc);
			DSP_FLD ("classDesc");

			if (!ValidateClassType ())
				return (EXIT_FAILURE);

			for (cnt = 0; cnt < 3; cnt++)
				sprintf (glmrRec.glmr_class [cnt], "%-1.1s", classCode + cnt);

			return (EXIT_SUCCESS);
		}
    }

	if (last_char != REDRAW)
		return print_err (ML (mlGlMess161));

    return (EXIT_SUCCESS);
}

int
ValidateClassType (void)
{
	char tmpAccountNo [MAXLEVEL + 1];
	char class_dad [4];
	int	 class_ok = TRUE;

	if (!strcmp (local_rec.locClass, "CC "))
	{
		GL_StripForm (tmpAccountNo, local_rec.locAccountNo);
		if (strncmp (tmpAccountNo, "0000000000000000", MAXLEVEL))
		{
			print_err (ML (mlGlMess108));
			return (FALSE);
		}
	}

	if (!class_ptr->type)
    {
		return (TRUE);
    }

	if (GET_ACC (GV_cur_level-1)->glmr_class [0][0] == 'C')
    {
		return (TRUE);
    }

	sprintf (class_dad, 
             "%c%c%c",
             GET_ACC (GV_cur_level-1)->glmr_class [0][0],
             GET_ACC (GV_cur_level-1)->glmr_class [1][0],
             GET_ACC (GV_cur_level-1)->glmr_class [2][0]);
/*
	Don't check on level 1 accounts.
*/
	if ((GV_cur_level - 1) &&
        (local_rec.locClass [0] != 'N') &&
	    !strchr (class_ptr->type, GET_ACC (GV_cur_level-1)->glmr_class [1][0]))
    {
		class_ok = FALSE;
    }

	if (!strcmp (local_rec.locClass, "NFS"))
    {
		if (strcmp (class_dad, "NFS") &&
		    strcmp (class_dad, "NFC"))
        {
			class_ok = FALSE;
        }
    }

	if (!strcmp (local_rec.locClass, "NFP"))
    {
		if (strcmp (class_dad, "NFS"))
        {
			class_ok = FALSE;
        }
    }

	if (!class_ok)
	{
		print_err (ML (mlGlMess109), local_rec.locClass, class_dad);
		return (FALSE);
	}

	return (TRUE);
}

int
ValidateKidClass (void)
{
	if (GV_cur_level == GV_max_level)
		return (TRUE);

	if (local_rec.locClass [0] == 'N')
		return (ValidateNKidClass ());

	return (ValidateFKidClass ());
}

int	
ValidateNKidClass (void)
{
	char	kid_class [4];

	abc_selfield (glmr2, "glmr_hhmr_hash");

	gllnRec.parent_hash = glmrRec.hhmr_hash;
	gllnRec.child_hash = 0L;
	
    cc = find_rec (glln, &gllnRec, GTEQ, "r");
	while (!cc && 
           (gllnRec.parent_hash == glmrRec.hhmr_hash))
	{
		glmrRec2.hhmr_hash = gllnRec.child_hash;
		
        cc = find_rec (glmr2, &glmrRec2, EQUAL, "r");
		if (cc)
        {
			file_err (cc, glmr2, "DBFIND");
        }

		sprintf (kid_class, 
                 "%s%s%s",
                 glmrRec2.glmr_class [0],
                 glmrRec2.glmr_class [1],
                 glmrRec2.glmr_class [2]);
        
        if (glmrRec2.glmr_class [0][0] == 'C')
        {
			return (ErrorKidClass (kid_class));
        }

		if (kid_class [0] == 'N')
		{
			if (strcmp (local_rec.locClass, "NFP") == 0)
            {
				return (ErrorKidClass (kid_class));
            }

			if (strcmp (local_rec.locClass, "NFS") == 0)
            {
				if (strcmp (kid_class, "NFS") &&
				    strcmp (kid_class, "NFP"))
                {
					return (ErrorKidClass (kid_class));
                }
            }
		}
		cc = find_rec (glln, &gllnRec, NEXT, "r");
	}

	return (TRUE);
}

int	
ValidateFKidClass (void)
{
	int  	levelCnt;
    char 	tmpAccountNo [MAXLEVEL + 1],
			kidClass 	 [4];

	if (!strcmp (local_rec.locClass, "CC "))
    {
		return (TRUE);
    }

	abc_selfield (glmr2, "glmr_id_no");
	strcpy (glmrRec2.co_no, comr_rec.co_no);

	tmpAccountNo [0] = (char) NULL;

	for (levelCnt = 0; levelCnt < GV_cur_level; levelCnt++)
    {
		strcat (tmpAccountNo, GL_GetBit (levelCnt + 1));
    }

	strcpy (glmrRec2.co_no, comr_rec.co_no);
	GL_StripForm (glmrRec2.acc_no, local_rec.locAccountNo);

	cc = find_rec (glmr2, &glmrRec2, GTEQ, "r");
	if (!cc)
		cc = find_rec (glmr2, &glmrRec2, NEXT, "r");
	while (!cc &&
           !strcmp (glmrRec2.co_no, comr_rec.co_no) &&
           !strncmp (glmrRec2.acc_no, tmpAccountNo, strlen (tmpAccountNo)))
	{
		sprintf (kidClass, 
                 "%s%s%s",
                 glmrRec2.glmr_class [0],
                 glmrRec2.glmr_class [1],
                 glmrRec2.glmr_class [2]);
        
        if (glmrRec2.glmr_class [0][0] != local_rec.locClass [0])
        {
            return (ErrorKidClass (kidClass));
        }

		if (!strcmp (local_rec.locClass, "FAS"))
        {
			if ((glmrRec2.glmr_class [1][0] != 'A') &&
				 (glmrRec2.glmr_class [1][0] != 'E'))
            {
				return (ErrorKidClass (kidClass));
            }
        }

		if (!strcmp (local_rec.locClass, "FES"))
        {
			if (glmrRec2.glmr_class [1][0] != 'E')
            {
				return (ErrorKidClass (kidClass));
            }
        }

		if (!strcmp (local_rec.locClass, "FIS"))
        {
			if ((glmrRec2.glmr_class [1][0] != 'I') && strcmp (kidClass, "FEP"))
				return (ErrorKidClass (kidClass));
        }

		if (!strcmp (local_rec.locClass, "NFC"))
        {
			if (glmrRec2.glmr_class [2][0] != 'S')
				return (ErrorKidClass (kidClass));
        }

		if (!strcmp (local_rec.locClass, "NFS"))
        {
			if ((glmrRec2.glmr_class [2][0] != 'S') &&
				 (glmrRec2.glmr_class [2][0] != 'P'))
            {
				return (ErrorKidClass (kidClass));
            }
        }

		cc = find_rec (glmr2, &glmrRec2, NEXT, "r");
	}

	return (TRUE);
}

int
ErrorKidClass (
	char *kid_class)
{
	print_err (ML (mlGlMess110),local_rec.locClass, kid_class, glmrRec2.acc_no);
	return (FALSE);
}

int	
NoChildren (void)
{
	if ((GV_cur_level == GV_max_level) || PV_new_account)
		return (TRUE);

	if (local_rec.locClass [0] == 'N')
		return NoNChildren ();

	return NoFChildren ();
}

int	
NoNChildren (void)
{
	gllnRec2.parent_hash = glmrRec.hhmr_hash;
	gllnRec2.child_hash = 0L;

	if (!find_rec (glln, &gllnRec, GTEQ,"r") &&
        (gllnRec.parent_hash == glmrRec.hhmr_hash))
	{
		print_err (ML (mlGlMess092), local_rec.locAccountNo);
		return (FALSE);
	}

	return (TRUE);
}

int	
NoFChildren (void)
{
	int		ret_val = FALSE, levelCnt;
	char		*GL_GetBit (int), tmpAccountNo [MAXLEVEL + 1];
	GLMR_STRUCT	tmp_glmr;

	tmpAccountNo [0] = (char) NULL;

	for (levelCnt = 0; levelCnt < GV_cur_level; levelCnt++)
		strcat (tmpAccountNo, GL_GetBit (levelCnt + 1));

	strcpy (tmp_glmr.co_no, comr_rec.co_no);
	GL_StripForm (tmp_glmr.acc_no, local_rec.locAccountNo);

	cc = ReadGlmrRecord (&tmp_glmr, "r", GTEQ);
	if (cc || ReadGlmrRecord (&tmp_glmr, "r", NEXT) ||
		strcmp (tmp_glmr.co_no, comr_rec.co_no) ||
		strncmp (tmp_glmr.acc_no, tmpAccountNo, strlen (tmpAccountNo)))
	{
		ret_val = TRUE;
	}

    cc = ReadGlmrRecord (&glmrRec, "u", COMPARISON);
	if (cc)
		file_err (cc, glmr, "DBFIND");

	if (!ret_val)
		 print_err (ML (mlGlMess093), tmp_glmr.acc_no, local_rec.locAccountNo);

	return (ret_val);
}

int	
LookupAccountNo (void)
{
	strcpy (glmrRec.co_no,comr_rec.co_no);
	if (GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec))
        return (EXIT_FAILURE);

	if (PV_new_account)
        GL_GetDesc (99, glmrRec.desc, 25);

	DSP_FLD ("accountDesc");
	return (EXIT_SUCCESS);
}

int	
ValidateExtent (void)
{
	char	*xt_ptr;

	for (xt_ptr = class_ptr->extent; *xt_ptr; xt_ptr++)
	{
		if ((*xt_ptr == 'B') && (GV_cur_level == GV_max_level))
			break;
		else if ((*xt_ptr == 'M') && (GV_cur_level < GV_max_level) && 
                (GV_cur_level > 1))
            break;
		else if ((*xt_ptr == 'T') && (GV_cur_level == 1))
			break;
		else if (*xt_ptr == 'C')
			break;
	}

	if ((!*xt_ptr) && (last_char != REDRAW))
		return print_err (ML (mlGlMess160));

	return (EXIT_SUCCESS);
}
int	
ValidateYear (
	int	year)
{
	if (year > PV_curr_fyear + 1)
		return print_err (ML (mlGlMess094));

	strcpy (local_rec.locYearType, "(Current)");
	if (year > PV_curr_fyear)
		strcpy (local_rec.locYearType, "(Future) ");
	else
    {
		if (year < PV_curr_fyear)
			strcpy (local_rec.locYearType, "(History)");
    }
	print_at (4,27,local_rec.locYearType);
	return (EXIT_SUCCESS);
}

int	
ReadGlmrRecord (
	GLMR_STRUCT *glmr_obj, 
	char   		*rtype, 
	int    		findFlag)
{
	abc_unlock (glmr);
	strcpy (glmr_obj->co_no,comr_rec.co_no);
    return (find_rec (glmr, glmr_obj, findFlag, rtype));
}

void
DisplayAccount (
 int t_flag)
{
#ifdef GVISION
	if (t_flag)
		DisplayTable ();
	
	DecodeGlmr ();
	scn_display (1);
	crsr_toggle (FALSE);
#else /*GVISION*/

	DecodeGlmr ();
	scn_display (1);
	crsr_toggle (FALSE);
	if (t_flag)
		DisplayTable ();
#endif /*GVISION*/
}

void 
DecodeGlmr (void)
{
	strcpy (local_rec.locAccountNo, glmrRec.acc_no);
	GL_FormAccNo (local_rec.locAccountNo, glmrRec.acc_no, 0);
	sprintf 
	(
		local_rec.locClass, 
		"%c%c%c", 
		glmrRec.glmr_class [0][0],
		glmrRec.glmr_class [1][0],
		glmrRec.glmr_class [2][0]
	);
	ValidateClass (label ("accountNo"), local_rec.locClass, FALSE);
	ValidateYear (local_rec.locYear);
	
	SetGlacKeys (local_rec.locClass);
}

void 
SetGlacKeys (
	char	*classCode)
{
	int		l_keys	= 0,
			t_keys 	= 0;

	static	int	Old_disp_type = 0;

	if (*classCode == 'N')
	{
		if (* (classCode + 2) != 'P')
			l_keys = set_keys (bal_keys, "L", KEY_ACTIVE);
		else
			l_keys = set_keys (bal_keys, "L", KEY_PASSIVE);

		if (set_keys (bal_keys, "Bb", KEY_PASSIVE))
			l_keys = TRUE;

		PV_disp_type = GL_BALANCES;
	}
	else
	{
		l_keys = set_keys (bal_keys, "L", KEY_PASSIVE);
		if (PV_disp_type == GL_BALANCES)
		{
			if (set_keys (bal_keys, "B", KEY_ACTIVE))
				l_keys = TRUE;
			if (set_keys (bal_keys, "b", KEY_PASSIVE))
				l_keys = TRUE;
		}
		else
		{
			if (set_keys (bal_keys, "b", KEY_ACTIVE))
				l_keys = TRUE;
			if (set_keys (bal_keys, "B", KEY_PASSIVE))
				l_keys = TRUE;
		}
	}

	if (l_keys || 
        t_keys || 
        (Old_disp_type != PV_disp_type))
	{
		Old_disp_type = PV_disp_type;
#ifndef GVISION		/* Remove it here */
		DisplayGlacHots ();
#endif /*GVISION*/
	}

#ifdef GVISION		/* and place it here */
	DisplayGlacHots ();
#endif /*GVISION*/
}

int
UpdateGlmr (void)
{
	/*
	 * Add or update database record.
	 */
	rv_pr (ML (mlGlMess100), 0, 23, 1);
	glmrRec.mod_date = TodaysDate ();

	if (PV_new_account) 
	{
		strcpy (glmrRec.co_no,comr_rec.co_no);
		strcpy (glmrRec.curr_code, local_rec.locCurrCode);
		strcpy (glmrRec.stat_flag,"0");
		strcpy (glmrRec.system_acc, local_rec.systemAccount);
		glmrRec.hhca_hash = GL_GetHhca (GV_cur_level);

        cc = abc_add (glmr,&glmrRec);
        if (cc)
			file_err (cc, glmr, "DBADD");
	}
	else 
	{
		strcpy (glmrRec.curr_code, local_rec.locCurrCode);
		strcpy (glmrRec.system_acc, local_rec.systemAccount);

        cc = abc_update (glmr,&glmrRec);
		if (cc)
			file_err (cc, glmr, "DBUPDATE");
	}

    cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBUPDATE");

	strcpy (local_rec.locAccountNo, glmrRec.acc_no);
	GL_FormAccNo (local_rec.locAccountNo, glmrRec.acc_no, 0);
	if (GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec) == 0)
	{
		if ((GV_cur_level > 1) &&
		    (GET_ACC (GV_cur_level-1)->glmr_class [2][0] == 'P') &&
		    (GET_ACC (GV_cur_level-1)->glmr_class [0][0] == 'F'))
		{
			abc_selfield (glmr2, "glmr_id_no");
			strcpy (glmrRec2.co_no, comr_rec.co_no);
			strcpy (glmrRec2.acc_no,
                    GET_ACC (GV_cur_level-1)->acc_no);

            cc = find_rec (glmr2, &glmrRec2, EQUAL, "u");
			if (cc)
            {
				file_err (cc, glmr2, "DBFIND");
            }
			strcpy (glmrRec2.glmr_class [1],glmrRec.glmr_class [1]);
			strcpy (glmrRec2.glmr_class [2], "S");

            cc = abc_update (glmr2, &glmrRec2);
			if (cc)
            {
				file_err (cc, glmr2, "DBUPDATE");
            }
			print_err (ML (mlGlMess146),
                       glmrRec2.acc_no,
                       glmrRec2.glmr_class [0][0],
                       glmrRec2.glmr_class [1][0],
                       glmrRec2.glmr_class [2][0]);

			RepostTrans 
			(
				glmrRec2.hhmr_hash, 
				glmrRec2.curr_code,
				glmrRec.hhmr_hash,
				glmrRec.curr_code
			);
		}
	}
	PV_new_account = FALSE;

	sleep (sleepTime);
	clear_mess ();

#ifdef GVISION
	rv_pr ("                                             ", 0, 23, 1);
#endif

	return (EXIT_SUCCESS);
}

void	
RepostTrans (
	long	fromHhmrHash, 
	char *	fromCurr,
	long	toHhmrHash,
	char *	toCurr)
{
	if (fromHhmrHash == toHhmrHash)
		return;

	open_rec (glpd2, glpd_list, GLPD_NO_FIELDS, "glpd_id_no");

	/*
	 * Move transactions to new account from gltr. 
 	 */
	gltrRec.hhmr_hash = fromHhmrHash;
	gltrRec.tran_date = -1L;
    cc = find_rec (gltr, &gltrRec, GTEQ, "r");
	while (!cc && (gltrRec.hhmr_hash == fromHhmrHash))
	{
		gltrRec.hhmr_hash = toHhmrHash;

        cc = abc_update (gltr, &gltrRec);
		if (cc)
			file_err (cc, gltr, "DBADD");

        gltrRec.hhmr_hash = fromHhmrHash;
		gltrRec.tran_date = -1L;
		cc = find_rec (gltr, &gltrRec, GTEQ, "r");
	}

	/*
	 * Move transactions to new account from glpd. 
 	 */
	glpdRec.hhmr_hash 	= fromHhmrHash;
	glpdRec.hhmr_hash 	= fromHhmrHash;
	glpdRec.budg_no 	= -1;

    cc = find_rec (glpd, &glpdRec, GTEQ, "r");
	while (!cc && 
           (glpdRec.hhmr_hash == fromHhmrHash))
	{
		glpdRec.hhmr_hash = toHhmrHash;

		/*
		 * Convert foreign amount.
	  	 */	
		if (strcmp (fromCurr, toCurr))
		{
			cc = FindPocr (comr_rec.co_no, toCurr, "r");
			if (cc)
			{
				strcpy (pocrRec.code, " ");
				strcpy (pocrRec.description, " ");
			}
			glpdRec.fx_balance = CurrencyFgnAmt (glpdRec.balance);
		}

		cc = abc_add (glpd2, &glpdRec);
		if (cc)
			file_err (cc, glpd2, "DBADD");

		glpdRec.hhmr_hash = fromHhmrHash;
		cc = find_rec (glpd, &glpdRec, NEXT, "r");
	}
	abc_fclose (glpd2);
}

void 
DisplayTable (void)
{
	switch (PV_disp_type)
	{
		case GL_BALANCES	:
			DisplayBalances ();
			break;

		case GL_BREAKDOWN	:
			BreakdownFunc (0, (KEY_TAB *) NULL);
			break;

		default :
			break;
	}
}

void 
DisplayBalances (void)
{
	if (LoadBalances (glmrRec.hhmr_hash))
	{
		tab_display ("tabGlpd", PV_glpd_display);
		PV_glpd_display = FALSE;
	}
}

int	
LoadBalances (
	long	hhmrHash)
{
	int	    periodCnt = 0;
	double	locBalance	= 0.0,
			fgnBalance	= 0.00;

	char	fgnDebitStr		[17], 
			fgnCreditStr 	[17],
			locDebitStr 	[17], 
			locCreditStr 	[17];

	tab_add ("tabGlpd", 
		 	"# Period  | Last Date | Last Time |   User   |  Debit  (Local)  |  Credit (Local)  |   Debit  (%3.3s)   |   Credit (%3.3s)   ",
			local_rec.locCurrCode, 
			local_rec.locCurrCode);

	glpdRec.hhmr_hash 	= hhmrHash;
	glpdRec.budg_no 	= PV_budg_no;
	glpdRec.year 		= local_rec.locYear;
	glpdRec.prd_no 		= 1;

	cc = find_rec (glpd, &glpdRec, GTEQ,"r");
	while (!cc && 
           (glpdRec.hhmr_hash == hhmrHash) &&
           (glpdRec.budg_no == PV_budg_no) &&
           (glpdRec.year == local_rec.locYear))
	{
		periodCnt++;
		SetAmounts 
		(
			fgnDebitStr,
			fgnCreditStr,
			DOLLARS (glpdRec.fx_balance),
			locDebitStr, 
			locCreditStr, 
			DOLLARS (glpdRec.balance)
		);
		
		if (glpdRec.prd_no > periodCnt)
		{
			while (glpdRec.prd_no > periodCnt)
			{
				cc = tab_add ("tabGlpd", "   %02d    |           |           |          |                  |                  |                  |                  ",
							periodCnt++);
				if (cc)
                    break;
			}
		}
			
		cc	=	tab_add 
				(
					"tabGlpd",
		 			"   %02d    |%-10.10s |   %-5.5s   | %8.8s | %16.16s | %16.16s | %16.16s | %16.16s",
					periodCnt,
					DateToString (TodaysDate ()),
					TimeHHMM (),
					glpdRec.user_id, 
					locDebitStr, 
					locCreditStr,
					fgnDebitStr, 
					fgnCreditStr
				);
		if (cc)
			break;

		fgnBalance 	+= DOLLARS (glpdRec.fx_balance);
		locBalance 	+= DOLLARS (glpdRec.balance);

		cc = find_rec (glpd, &glpdRec, NEXT,"r");
	}

	while (++periodCnt <= 12)
	{
		cc = tab_add ("tabGlpd", "   %02d    |           |           |          |                  |                  |                  |                  ",
							periodCnt);
		if (cc)
			break;
	}

	SetAmounts 
	(
		fgnDebitStr, 
		fgnCreditStr, 
		fgnBalance,
		locDebitStr, 
		locCreditStr, 
		locBalance
	);
	tab_add 
	(
		"tabGlpd", 
		"%-9.9s|%-10.10s |   %-5.5s   | %8.8s | %16.16s | %16.16s | %16.16s | %16.16s",
		ML ("BALANCE"), 
		" ", " ", " ", 
		locDebitStr, 
		locCreditStr,
		fgnDebitStr, 
		fgnCreditStr
	);
	return (TRUE);
}

void	
SetAmounts (
	char   *fgnDebit, 
	char   *fgnCredit, 
	double fgnAmount,
	char   *locDebit,
	char   *locCredit,	
	double locAmount)
{
	if (locAmount <= 0.001)
	{
		sprintf (locDebit, "%-16.16s", " ");
		sprintf (fgnDebit, "%-16.16s", " ");
		sprintf (locCredit,"%16.16s",comma_fmt (-locAmount,"N,NNN,NNN,NNN.NN"));
		sprintf (fgnCredit,"%16.16s",comma_fmt (-fgnAmount,"N,NNN,NNN,NNN.NN"));
	}
	else
	{
		sprintf (locCredit, "%-16.16s", " ");
		sprintf (fgnCredit, "%-16.16s", " ");
		sprintf (locDebit,"%16.16s",comma_fmt (locAmount, "N,NNN,NNN,NNN.NN"));
		sprintf (fgnDebit,"%16.16s",comma_fmt (fgnAmount, "N,NNN,NNN,NNN.NN"));
	}
}

void		
DisplayTrans (
	char	*period)
{
	glpdRec.hhmr_hash	= glmrRec.hhmr_hash;
	glpdRec.budg_no 	= PV_budg_no;
	glpdRec.year 		= local_rec.locYear;
	glpdRec.prd_no 		= atoi (period);

	if ((glpdRec.prd_no != 99) &&
        find_rec (glpd, &glpdRec, EQUAL,"r"))
	{
		print_mess (ML (mlGlMess024));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&tranDetails, &tranRec, sizeof (struct Trans), 1000);
	tranCnt 	= 0;
	tranTotal	= 0.00;

	LoadTrans (glpdRec.hhmr_hash, glpdRec.year, glpdRec.prd_no);

	while (1)
	{
		sortChanged	= FALSE;
		tab_open ("tabGltr", tranDetailKeys, 5, 0, 13, FALSE);
		tab_add ("tabGltr",
			 "#   Date   |  Posted  | Trans. narrative.  |Jnl. type|Journal no|   User reference   | Local amount  |Each rate|Cur| Foreign amount");

		DisplayGltr (expanded, compressed);

		if (!tab_display ("tabGltr", TRUE))
			tab_scan ("tabGltr");

		tab_close ("tabGltr", TRUE);

		if (sortChanged == FALSE)
			break;
	}

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&tranDetails);

	move (0,1);cl_line ();
	box (0,1, 132, 3);
	line_at (21, 0, 132);
}

void
LoadTrans (
	long   hhmrHash, 
	int    yearNo,
	int    periodNo)
{
    int     continueLoop	= 0,
			fmonth			= 0,
			fyear			= 0,
			journalNo		= 0;

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

	gltrRec.hhmr_hash = hhmrHash;
	gltrRec.tran_date = FinDMYToDate 
						(
							comr_rec.fiscal, 
							1,
							(periodNo == 99) ? 1: periodNo, 
							yearNo
						);
    cc = find_rec (gltr, &gltrRec, GTEQ,"r");
	if (cc)
        return;

	DateToFinDMY 
	(
		gltrRec.tran_date, 
		comr_rec.fiscal, 
		NULL, 
		&fmonth, 
		&fyear
	);

    continueLoop = TRUE;
    while (continueLoop == TRUE)
	{
		if (!cc && gltrRec.hhmr_hash == hhmrHash &&	
			   fmonth <= periodNo && fyear == yearNo)
		{
			continueLoop = TRUE;
		}
		else
		{
			continueLoop = FALSE;
			break;
		}
        /*  actual loop processing begins here */

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
			"%-20.20s%010ld000", 
			gltrRec.narrative,
			gltrRec.gltr_hash
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey2, 
			"%-10.10s%010ld000",
			zero_pad (gltrRec.sys_ref,10),
			gltrRec.gltr_hash
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey3, 
			"%-20.20s%010ld000",
			gltrRec.user_ref,
			gltrRec.gltr_hash
		);
		sprintf 
		(
			tranRec [tranCnt].sortKey3, 
			"%010ld%010ld000",
			gltrRec.tran_date,
			gltrRec.gltr_hash
		);

		sprintf (tranRec [tranCnt].exchRate,"%9.4f", gltrRec.exch_rate);
		sprintf (tranRec [tranCnt].locAmount, "%15.15s", 
                comma_fmt (DOLLARS (gltrRec.amount), "NNNN,NNN,NNN.NN"));
		sprintf (tranRec [tranCnt].fgnAmount, "%15.15s", 
                comma_fmt (DOLLARS (gltrRec.amt_origin), "NNNN,NNN,NNN.NN"));
		strcpy (tranRec [tranCnt].narrative,  gltrRec.narrative);
		strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
		strcpy (tranRec [tranCnt].sysRef 	, gltrRec.sys_ref);
		strcpy (tranRec [tranCnt].userRef	, gltrRec.user_ref);
		strcpy (tranRec [tranCnt].currCode,	  gltrRec.currency);
		tranRec [tranCnt].expandFlag	=	FALSE;
		if (strncmp (gltrRec.narrative, "CP ", 3))
			tranRec [tranCnt].consolFlag	=	CONS_NONE;
		else
			tranRec [tranCnt].consolFlag	=	CONS_HEADER;

		tranRec [tranCnt].gltrHash		=	gltrRec.gltr_hash;
		tranRec [tranCnt].tranDate		=	gltrRec.tran_date;
		tranRec [tranCnt].postDate		=	gltrRec.post_date;

		tranTotal += DOLLARS (gltrRec.amount);

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
				"%-20.20s%010ld000", 
				gltcRec.narrative,
				gltcRec.gltr_hash
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey2, 
				"%-10.10s%010ld000",
				zero_pad (gltcRec.sys_ref,10),
				gltcRec.gltr_hash
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%-20.20s%010ld000",
				gltcRec.user_ref,
				gltcRec.gltr_hash
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%010ld%010ld000",
				gltcRec.tran_date,
				gltcRec.gltr_hash
			);

			sprintf (tranRec [tranCnt].exchRate,"%9.4f", gltcRec.exch_rate);
			sprintf (tranRec [tranCnt].locAmount, "%15.15s", 
					comma_fmt (DOLLARS (gltcRec.amount), "NNNN,NNN,NNN.NN"));
			sprintf (tranRec [tranCnt].fgnAmount, "%15.15s", 
					comma_fmt (DOLLARS (gltcRec.amt_origin), "NNNN,NNN,NNN.NN"));
			strcpy (tranRec [tranCnt].narrative,  gltcRec.narrative);
			strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
			strcpy (tranRec [tranCnt].sysRef 	, gltcRec.sys_ref);
			strcpy (tranRec [tranCnt].userRef	, gltcRec.user_ref);
			strcpy (tranRec [tranCnt].currCode,	  gltcRec.currency);
			tranRec [tranCnt].expandFlag	=	FALSE;
			tranRec [tranCnt].consolFlag	=	CONS_DETAIL;
			tranRec [tranCnt].gltrHash		=	gltcRec.gltr_hash;
			tranRec [tranCnt].tranDate		=	gltcRec.tran_date;
			tranRec [tranCnt].postDate		=	gltcRec.post_date;
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
				"%-20.20s%010ld%03d",
				gltrRec.narrative, 
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey2, 
				"%-10.10s%010ld%03d",
				zero_pad (gltrRec.sys_ref,10),
				gltrRec.gltr_hash,
				glnaRec.line_no + 1
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%-20.20s%010ld%03d",
				gltrRec.user_ref, 
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);
			sprintf 
			(
				tranRec [tranCnt].sortKey3, 
				"%010ld%010ld%03d",
				gltrRec.tran_date, 
				gltrRec.gltr_hash, 
				glnaRec.line_no + 1
			);

			sprintf (tranRec [tranCnt].exchRate,"%9.9s", " ");
			sprintf (tranRec [tranCnt].locAmount, "%15.15s", " ");
			sprintf (tranRec [tranCnt].fgnAmount, "%15.15s", " ");
			strcpy (tranRec [tranCnt].narrative,  glnaRec.narrative);
			strcpy (tranRec [tranCnt].jnlType 	, jtype [journalNo]);
			strcpy (tranRec [tranCnt].sysRef 	, gltrRec.sys_ref);
			strcpy (tranRec [tranCnt].userRef	, gltrRec.user_ref);
			strcpy (tranRec [tranCnt].currCode,	  " ");
			tranRec [tranCnt].expandFlag	=	TRUE;
			tranRec [tranCnt].consolFlag	=	CONS_NONE;
			tranRec [tranCnt].gltrHash		=	gltrRec.gltr_hash;
			tranRec [tranCnt].tranDate		=	gltrRec.tran_date;
			tranRec [tranCnt].postDate		=	gltrRec.post_date;
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
			comr_rec.fiscal, 
			NULL, 
			&fmonth, 
			&fyear
		);
	}   /* end of while loop */


    return;
}

void
DisplayGltr (
	int		expand,
	int		compressed)
{
	int		i;
	char	postDate [11],
			tranDate [11];

	/*
	 * Sort the array depending on sort key.
	 */
	if (currentSort	== SORTBYNARRATIVE)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort1);
	else if (currentSort	== SORTBYSYSREF)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort2);
	else if (currentSort	== SORTBYUSERREF)
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort3);
	else 
		qsort (tranRec, tranCnt, sizeof (struct Trans), TranSort4);

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
		
		strcpy (postDate,DateToString (tranRec [i].postDate));
		strcpy (tranDate,DateToString (tranRec [i].tranDate));

        cc =	tab_add 
				(
					"tabGltr", 
					"%-10.10s|%-10.10s|%-20.20s|%-9.9s|%-10.10s|%-20.20s|%15.15s|%9.9s|%-3.3s|%15.15s",
					tranDate,
					postDate,
					tranRec [i].narrative, 
					tranRec [i].jnlType,
					tranRec [i].sysRef,
					tranRec [i].userRef,
					tranRec [i].locAmount,
					tranRec [i].exchRate,
					tranRec [i].currCode,
					tranRec [i].fgnAmount
				); 
		if (cc)
			break;

	}
	tab_add ("tabGltr",
	 "__________|__________|____________________|_________|__________|____________________|_______________|_________|___|_______________");
	tab_add ("tabGltr", 
	 "          |          |                    |         |          | *** %5.5s ***      |%15.15s|         |   |",
			ML ("TOTAL"),
			comma_fmt (tranTotal, "NNNN,NNN,NNN.NN"));
	tab_add ("tabGltr",
	 "__________|__________|____________________|_________|__________|____________________|_______________|_________|___|_______________");

}

int
TranDetailsFunc (
 int      c, 
 KEY_TAB *psUnused)
{
	DisplayInfo ();
	if (c == 'N')
	{
		currentSort	=	SORTBYNARRATIVE;
		sortChanged	=	TRUE;
		return (FN1);
	}
	if (c == 'J')
	{
		currentSort	=	SORTBYSYSREF;
		sortChanged	=	TRUE;
		return (FN1);
	}
	if (c == 'U')
	{
		currentSort	=	SORTBYUSERREF;
		sortChanged	=	TRUE;
		return (FN1);
	}
	if (c == 'D')
	{
		currentSort	=	SORTBYTRANDATE;
		sortChanged	=	TRUE;
		return (FN1);
	}
	if (c == 'E')
	{
		if (expanded == TRUE)
			expanded = FALSE;
		else
			expanded = TRUE;

		sortChanged	= TRUE;
		DisplayInfo ();
		return (FN1);
	}
	if (c == 'C')
	{
		if (compressed 	== FALSE)
			compressed 	= TRUE;
		else
			compressed 	= FALSE;

		sortChanged	= TRUE;
		DisplayInfo ();
		return (FN1);
	}
	sortChanged	=	FALSE;
	return (EXIT_SUCCESS);
}

int
TranscationFunc (
 int      c, 
 KEY_TAB *psUnused)
{

	line_at (21, 0, 132);
	if (!strncmp ("BA", &temp_str [2], 2))
		DisplayTrans ("99");
	else
		DisplayTrans (&temp_str [2]);
	line_at (21, 0, 132);
	redraw_table ("tabGlpd");
	return (restart ? FN1 : 1);
}

void
EntryFunc (
 void)
{
	int	crsr_stat;

	init_ok 	= TRUE;
	init_vars (1);
	PV_curr_fyear = local_rec.locYear = fisc_year (comr_rec.gl_date);
	heading (1);
	scn_display (1);
	crsr_stat = crsr_toggle (TRUE);
	init_ok = FALSE;
	entry (1);
	init_ok = TRUE;
	crsr_toggle (crsr_stat);
}

int
EditFunc (
 int      c, 
 KEY_TAB *psUnused)
{

	restart = 0;
	if (PV_display)
    {
		return (FALSE);
    }

	crsr_toggle (TRUE);
	heading (1);
	scn_display (1);
	edit (1);
	crsr_toggle (FALSE);
	if (!restart)
		UpdateGlmr ();
	else
	{
		restart = 0;
		if (!PV_new_account)
			DisplayAccount (FALSE);
	}

#ifdef GVISION
	heading (1);
#else
	scn_write (1);
#endif
	crsr_toggle (FALSE);
	SetGlacKeys (local_rec.locClass);
	return (TRUE);
}

int
BudgetFunc (
	int     c, 
	KEY_TAB *psUnused)
{

	FLD ("class")		= NA;
	FLD ("accountDesc")	= NA;
	FLD ("bdgtno")		= YES;

	restart = 0;
	crsr_toggle (TRUE);
	edit (1);
	crsr_toggle (FALSE);
	if (restart)
	{
		restart = 0;
		if (!PV_new_account)
			DisplayAccount (FALSE);
	}

	scn_write (1);
	crsr_toggle (FALSE);
	FLD ("bdgtno")		= NA;
	FLD ("class")		= YES;
	FLD ("accountDesc")	= YES;
	DisplayAccount (TRUE);
	return (TRUE);
}

int
YearFunc (
	int     c, 
	KEY_TAB *psUnused)
{
    int local_year_adjustment;
    
    if (local_rec.locYear == PV_curr_fyear)
        local_year_adjustment = 1;
    else if (local_rec.locYear < PV_curr_fyear)
        local_year_adjustment = (PV_curr_fyear - local_rec.locYear);
    else
        local_year_adjustment = -1;

    local_rec.locYear += local_year_adjustment;
	
    DisplayAccount (TRUE);


	return (TRUE);
}

int
HistoryFunc (
	int      c, 
	KEY_TAB *psUnused)
{
	static int hist_incr = 0;

	if (!glctRec.history)
	{
		print_mess (ML (mlGlMess137));
		sleep (sleepTime);
		clear_mess ();
		return (TRUE);
	}

	if (glctRec.history == 1)
		local_rec.locYear = PV_curr_fyear - 1;

	else if (local_rec.locYear >= PV_curr_fyear)
	{
		hist_incr = -1;
		local_rec.locYear = PV_curr_fyear;
	}
	else if (local_rec.locYear == (PV_curr_fyear - 1))
		hist_incr = -1;
	else if (local_rec.locYear == (PV_curr_fyear - glctRec.history))
		hist_incr = 1;

	local_rec.locYear += hist_incr;

	DisplayAccount (TRUE);

	return (TRUE);
}

int
NextFunc (
	int     c, 
	KEY_TAB *psUnused)
{
	cc	=	ReadGlmrRecord 
			(
				&glmrRec, 
				PV_display ? "r" : "u", 
				c == FN14 ? GTEQ : LTEQ
			);
	if (cc || strcmp (glmrRec.co_no, comr_rec.co_no))
	{ 
		/*
	     *	Reset Current
		 */
		ReadGlmrRecord (&glmrRec, PV_display ? "r" : "u", CURRENT);
		if (c == FN14)
			print_err (ML (mlGlMess147));
		else
			print_err (ML (mlGlMess148));
	}
	else
	{
		cc	=	ReadGlmrRecord 
				(
					&glmrRec, 
					PV_display ? "r" : "u", 
					c == FN14 ? NEXT : PREVIOUS
				);
		if (!cc)
		{
			cc = FindPocr (comr_rec.co_no, glmrRec.curr_code, "r");
			if (cc)
			{
				strcpy (local_rec.locCurrCode, " ");
				strcpy (local_rec.locCurrDesc, " ");
			}
			else
			{
				strcpy (local_rec.locCurrCode, pocrRec.code);
				sprintf (local_rec.locCurrDesc,"%-25.25s",pocrRec.description);
			}
			PV_new_account = FALSE;

			/*
			 * Enable / disable currency code
			 */
			if (NoGltr ())
				FLD ("currCode") = YES;
			else
				FLD ("currCode") = NA;
        	
			DisplayAccount (TRUE);
		}
	}
	clear_mess ();
	return (TRUE);
}

int
BreakdownFunc (
	int     c,
	KEY_TAB *psUnused)
{

	set_keys (bal_keys, "b", KEY_ACTIVE);
	set_keys (bal_keys, "B", KEY_PASSIVE);
	if ((PV_disp_type != GL_BREAKDOWN) || 
        (last_char == REDRAW))
	{
		if (last_char == REDRAW)
        {
			redraw_form ("tabGlpd");
        }
		blank_table ("tabGlpd");

		/*ACCOUNT BREAKDOWN : */
#ifndef GVISION
		rv_pr (ML (mlGlMess096), 7, 6, 0);
		
		line_at (7, 7, 121);
#else /*GVISION*/

		tab_header_text ("tabGlpd", ML (mlGlMess096));
#endif /*GVISION*/
		DisplayGlacHots ();
	}

	DisplayBreak ();

	PV_disp_type = GL_BREAKDOWN;
	scn_display (1);
	return (TRUE);
}

void
DisplayBreak (void)
{
	char tmp_desc [131];
	int	 level = 0, row;

#ifndef GVISION 
	GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec);

	for (row = 8; level < GV_max_level && row < 20; level++, row += 2)
	{
		if ((PV_disp_type != GL_BREAKDOWN) || 
            (last_char == REDRAW))
        {
			box (15 + ((level + 1) * 3), row, 30, 1);
        }

		sprintf (tmp_desc, "%25.25s", " ");
		GL_GetDesc (level, tmp_desc, 25);

		if (!strncmp (tmp_desc, "                         ", 25))
		{
			sprintf (tmp_desc, "%-25.25s", "      ---- N/A ----");
		}

		rv_pr (tmp_desc, 18 + ((level + 1) * 3),row + 1, 0);
	}
#else /*GVISION*/

	char *	pad = "   ";

	GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec);

	tab_cleartext ("tabGlpd");

	for (row = 8; level < GV_max_level && row < 20; level++, row += 2)
	{
		sprintf (tmp_desc, "%-3.3s+----------------------------+", pad);
		tab_text ("tabGlpd",
				  tmp_desc,
				  (11 + ((level + 1) * 3)) - (7 + 3),
				  (row) - 6);

		pad = "+--";

		sprintf (tmp_desc, "%25.25s", " ");
		GL_GetDesc (level, tmp_desc, 25);
	
		if (!strncmp (tmp_desc, "                         ", 25))
			sprintf (tmp_desc, "|%-28.28s|", "       ---- N/A ----");
		else
			strcpy (tmp_desc, "|                            |");

		tab_text ("tabGlpd", tmp_desc, (11 + ((level + 1) * 3)) - 7, (row + 1) - 6);
	}

	level--;
	tab_text ("tabGlpd",
			  "+----------------------------+",
			  (11 + ((level + 1) * 3)) - 7,
			  (row) - 6);
#endif /*GVISION*/
}

int
balanceFunc (
	int     c, 
	KEY_TAB *psUnused)
{

	PV_disp_type = GL_BALANCES;
	set_keys (bal_keys, "B", KEY_ACTIVE);
	set_keys (bal_keys, "b", KEY_PASSIVE);
	DisplayAccount (TRUE);
	DisplayGlacHots ();
	return (TRUE);
}

int
TransactionFunc (
	int     c, 
	KEY_TAB *psUnused)
{
	line_at (21, 0, 132);
	tab_scan ("tabGlpd");
	DisplayGlacHots ();
	return (restart ? FN1 : 1);
}

int
LinkFunc (
	int     c, 
	KEY_TAB *psUnused)
{

	if ((glmrRec.glmr_class [2][0] != 'C') && !glmrRec.parent_cnt)
        return print_err (ML (mlGlMess095));

	if (!PV_display)
		RunLinkHots ();
	else
		MaintainLinks ('D', (KEY_TAB *) NULL);

	DisplayAccount (TRUE);

	DisplayGlacHots ();

	return (TRUE);
}

int
MaintainLinks (
	int     c, 
	KEY_TAB *psUnused)
{
	int     len, tmp_level;
	char    formAccount [FORM_LEN + 1];

	tmp_level = GV_cur_level;

	line_at (21, 0, 132);
	tab_open ("tabGlmr2", c == 'D' ? null_keys : slct_keys, 5, 6, 13, FALSE);
	tab_add ("tabGlmr2", c != LINK_DELETE ? 
		    "# Account Number                    Acc Description               " :
		    "# DELETE STATISTICAL LINKS.                                       ");

	abc_selfield (glmr2, "glmr_hhmr_hash");

	len = strlen (vars [label ("accountNo")].mask);

	gllnRec.parent_hash = glmrRec.hhmr_hash;
	gllnRec.child_hash = 0L;

    cc = find_rec (glln, &gllnRec, GTEQ,"r");
	while (!cc && 
           (gllnRec.parent_hash == glmrRec.hhmr_hash))
	{
		glmrRec2.hhmr_hash	=	gllnRec.child_hash;
        cc = find_rec (glmr2, &glmrRec2, EQUAL, "r");
		if (cc)
			file_err (cc, glmr2, "FIND_HASH");

		strcpy (formAccount, glmrRec2.acc_no);
		GL_FormAccNo (formAccount, glmrRec2.acc_no, 0);

		if (tab_add ("tabGlmr2", " %-*.*s  %-25.25s", 
                     len, len, formAccount, glmrRec2.desc))
        {
			break;
        }

		cc = find_rec (glln, &gllnRec, NEXT,"r");
	}

	if (!tab_display ("tabGlmr2", TRUE))
    {
		if (!tab_scan ("tabGlmr2"))
			ActionList (c);
    }

	tab_close ("tabGlmr2", TRUE);

	RestoreGlpd ();

	return (TRUE);
}

int
SelectLinks (
	int     c, 
	KEY_TAB *psUnused)
{
	int		AcctLen;
	int		crsr_stat, len, tmp_level, level;
	char	tmpAccountNo [MAXLEVEL + 1], 
			formAccount [FORM_LEN + 1], 
			srch_acc [FORM_LEN + 1];


    tmp_level = GV_cur_level;

	line_at (21, 0, 132);
	tab_open ("tabGlmr2", slct_keys, 5, 6, 13, FALSE);
	tab_add ("tabGlmr2", "# Select - Level :   Account (s) :                                 ");

	redraw_heading (glmr2, FALSE);
    crsr_stat = crsr_toggle (TRUE);

    do
	{
		while ((level = getint (25, 6, "N")) < 0 || 
               (level > GV_max_level))
        {
			print_err (ML (mlGlMess160));
        }

		do
		{
			if (last_char == FN1)
				break;

			getalpha (40, 6, vars [label ("accountNo")].mask, formAccount);
			GL_StripForm (srch_acc, formAccount);

        } while (GL_FormAccNo (formAccount, tmpAccountNo, 0));
    
    } while ((last_char != FN1) && 
             (last_char != FN16) && 
             (last_char != '\r'));

	crsr_toggle (crsr_stat);

	AcctLen = strlen (vars [label ("accountNo")].mask);

	abc_selfield (glmr2, "glmr_id_no");
	len = strlen (srch_acc);
	tab_add ("tabGlmr2", "# CREATE STATISTICAL LINKS.                                       ");

	strcpy (glmrRec2.co_no,comr_rec.co_no);
	sprintf (glmrRec2.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,tmpAccountNo);

    cc = find_rec (glmr2, &glmrRec2, GTEQ,"r");
	while (!cc && 
           !restart &&
           !strncmp (glmrRec2.acc_no,tmpAccountNo,len) &&
           !strcmp (glmrRec2.co_no, comr_rec.co_no))
	{
		strcpy (formAccount, glmrRec2.acc_no);
		GL_FormAccNo (formAccount, glmrRec2.acc_no, 0);

		/*
		 *	Will not link to Control accounts or to current parent account.
		 *	Will not allow muliple use of the same NF account in the same NFC.
		 */
		if ((!level || level == GV_cur_level) && LinkOK ())
		{
			cc = tab_add ("tabGlmr2", " %-*.*s %-25.25s",
                           AcctLen, AcctLen, formAccount, glmrRec2.desc);
			if (cc)
                break;
		}
		cc = find_rec (glmr2, &glmrRec2, NEXT,"r");
	}

	if (!tab_display ("tabGlmr2", TRUE))
    {
		if (!tab_scan ("tabGlmr2"))
			ActionList (LINK_CREATE);
    }
	tab_close ("tabGlmr2", TRUE);

	RestoreGlpd ();

	return (TRUE);
}

void
RestoreGlpd (void)
{
	/*
	 * Reset level information etc.
	 */
	GL_FormAccNo (local_rec.locAccountNo, glmrRec.acc_no, 0);
	switch (PV_disp_type)	/*	Redraw previous window.	*/
	{
		case GL_BALANCES :
			redraw_table ("tabGlpd");
			break;

		case GL_BREAKDOWN :
			PV_disp_type = FALSE;
			redraw_form ("tabGlpd");
			BreakdownFunc (0, (KEY_TAB *) NULL);
			break;

		default :
			break;
	}

	line_at (21, 0, 132);

	if (!PV_display)
		disp_hotkeys (21, 0, 132, link_keys);
	else
		DisplayGlacHots ();
}

int
TagLink (
	int     c, 
	KEY_TAB *psUnused)
{

	if (c != 'T')
		SetAll ();
	else
		tag_toggle (glmr2);
	
	return (TRUE);
}

void 
SetAll (void)
{
	rv_pr (ML (mlGlMess097), 0, 23, 1);
	tag_all (glmr2);
	
	return;
}

void 
ActionList (
	int		action)
{
	char tmpAccountNo [FORM_LEN + 1], tabLine [131], tmpString [131];
	int	 ln, len;

	if (restart || 
        ((action != LINK_CREATE) && (action != LINK_DELETE)))
	{
		restart = 0;
		return ;
	}

	len = strlen (vars [label ("accountNo")].mask);

	abc_selfield (glmr2, "glmr_id_no");

	for (ln = 0; !tab_get ("tabGlmr2", tabLine, EQUAL, ln); ln++)
	{
		if (tagged (tabLine))
		{
			redraw_line (glmr2, TRUE);
			sprintf (tmpAccountNo, "%-*.*s", len, len, &tabLine [1]);
			if (action == LINK_DELETE)
				sprintf (tmpString, ML (mlGlMess098),tmpAccountNo);
			else
				sprintf (tmpString, ML (mlGlMess099),tmpAccountNo);
            
			rv_pr (tmpString, 0, 23, 1);

			strcpy (glmrRec2.co_no,comr_rec.co_no);
			GL_FormAccNo (tmpAccountNo, glmrRec2.acc_no, 0);

            cc = find_rec (glmr2, &glmrRec2, EQUAL,"w");
			if (cc)
				file_err (cc, glmr2, "FIND_REC");

			if (action == LINK_DELETE)
			{
				if (DeleteLink (strlen (tmpString)))
                    break;
			}
			else if (AddLink (strlen (tmpString)))
				break;

            cc = abc_update (glmr2, &glmrRec2);
			if (cc)
                file_err (cc, glmr2, "DBUPDATE");

			tabLine [0] = ' ';
			tab_update ("tabGlmr2", "%s", tabLine);

			redraw_line (glmr2, FALSE);
		}
	}

    cc = abc_update (glmr, &glmrRec);
	if (cc)
		file_err (cc, glmr, "DBUPDATE");

	LoadBalances (glmrRec.hhmr_hash); /* Reload new balances.   */
}

int
DeleteLink (
	int		offset)
{
    cc = ReadLink ();
	if (cc)
		file_err (cc, glln, "FIND_REC");

    cc = abc_delete (glln);
	if (cc)
		file_err (cc, glln, "DBDELETE");

	glmrRec2.parent_cnt--;
	glmrRec.child_cnt--;

	return  
	(
		UpdateBalances 
		(
			LINK_DELETE, 
			offset, 
			glmrRec.acc_no,
			glmrRec.curr_code, 
			gllnRec.parent_hash,
			gllnRec.child_hash
		)
	);
}

int	
AddLink (
	int		offset)
{
	gllnRec.parent_hash = glmrRec.hhmr_hash;
	gllnRec.child_hash = glmrRec2.hhmr_hash;

    cc = abc_add (glln, &gllnRec);
	if (cc)
		file_err (cc, glln, "DBADD");

	glmrRec2.parent_cnt++;
	glmrRec.child_cnt++;

	return	
	(
		UpdateBalances 
		(
			LINK_CREATE, 
			offset, 
			glmrRec.acc_no,
			glmrRec.curr_code,
			gllnRec.parent_hash, 
			gllnRec.child_hash
		)
	);
}

int	
LinkOK (void)
{
	if (glmrRec2.parent_cnt + 1 > glctRec.link_max)
		return (FALSE);

	if ((glmrRec2.glmr_class [2][0] == 'C') ||
        (glmrRec2.glmr_class [1][0] == 'C') ||
        !strncmp (glmrRec.acc_no, glmrRec2.acc_no,MAXLEVEL))
    {
		return (FALSE);
    }

	/*
		Can't link Financial accounts to NFC's.
	*/
	if ((glmrRec2.glmr_class [2][0] == 'C') && 
        (glmrRec.glmr_class [0][0] != 'F'))
    {
		return (FALSE);
    }

	if (glmrRec2.glmr_class [0][0] == 'N') /*  Non-financial Acc. */
	{
		gllnRec2.child_hash 	= glmrRec2.hhmr_hash;
		gllnRec2.parent_hash 	= 0L;
	
        if (!find_rec (glln, &gllnRec2, GTEQ,"r") &&
            (gllnRec2.child_hash == glmrRec2.hhmr_hash))
        {
			return (FALSE);
        }
	}
	return (ReadLink ());
}

int	
ReadLink (void)
{
	gllnRec.parent_hash	= glmrRec.hhmr_hash;
	gllnRec.child_hash	= glmrRec2.hhmr_hash;
	return (find_rec (glln, &gllnRec, EQUAL, "r"));
}

int	
UpdateBalances (
	int    op, 
	int    offset, 
	char   *accountNo, /* unreferenced formal parameter */
	char   *currCode,  /* unreferenced formal parameter */
	long   p_hhmrHash, /* unreferenced formal parameter */
	long   c_hhmrHash)
{
	GLPD_STRUCT	tmp_glpd;
	int	   cnt = 1;
	double	fgnAmount	= 0.00,
			locAmount	= 0.00;

	tmp_glpd.hhmr_hash 	= c_hhmrHash;
	tmp_glpd.budg_no 	= 0;
	tmp_glpd.year 		= 0;
	tmp_glpd.prd_no 	= 0;
    cc = find_rec (glpd, &tmp_glpd, GTEQ,"r");
	if (cc)
	{
		sleep (sleepTime);
		return (EXIT_SUCCESS);
	}

	GL_PostStamp ();
	while (!cc && (tmp_glpd.hhmr_hash == c_hhmrHash))
	{
		rv_pr (ML (mlGlMess116), offset, 23, cnt++ % 2);

		fgnAmount = (op == LINK_CREATE) ? tmp_glpd.fx_balance :
					      				 -tmp_glpd.fx_balance;
		locAmount = (op == LINK_CREATE) ? tmp_glpd.balance :
					      				 -tmp_glpd.balance;

		GL_PostBudget ((int) tmp_glpd.budg_no);

		if 
		(
			_PostAccount 
			(
				accountNo, 
			  	tmp_glpd.year,
			  	tmp_glpd.prd_no, 
			  	currCode,
			  	fgnAmount, 
			  	locAmount
			)
		)
		{
			GL_PostBudget ((int) local_rec.loc_bdg_no);
			return (EXIT_FAILURE);
		}
		cc = find_rec (glpd, &tmp_glpd, GTEQ,"r");
		if (!cc)
			cc = find_rec (glpd, &tmp_glpd, NEXT,"r");
	}

	rv_pr ("                         ", offset, 23 , 0);

	GL_PostBudget ((int) local_rec.loc_bdg_no);

	return (EXIT_SUCCESS);
}

int	
RunGlacHots (void)
{
	heading (1);

	restart = FALSE;
#ifndef GVISION  /*redundant call to DisplayGlacHots*/
	DisplayGlacHots ();
#endif /*GVISION*/
	if (run_hotkeys (bal_keys, null_func, null_func))
	{
		PV_disp_type = 0;
		restart = TRUE;
	}

#ifdef GVISION
	tab_clear ("tabGlpd");
#endif /*GVISION*/
	return (FALSE);
}

void
RunLinkHots (void)
{
	line_at (21, 0, 132);
	disp_hotkeys (21, 0, 132, link_keys);
	run_hotkeys (link_keys, null_func, null_func);
}

void 
DisplayGlacHots (void)
{
	line_at (21, 0, 132);
	disp_hotkeys (21, 0, 132, bal_keys);
}

void 
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);			/* Get base currency */
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_alias (glmr2, glmr);
	abc_alias (glpd2, glpd);
	abc_alias (gltr2, gltr);

	open_rec (gltr2, gltr_list, GLTR_NO_FIELDS, "gltr_hhmr_hash");

	tab_open ("tabGlpd", tran_keys, 5, 6, 13, TRUE);

	GL_PostSetup (comr_rec.co_no, PV_budg_no);

	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	strcpy (glmrRec.co_no, comr_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
             MAXLEVEL, MAXLEVEL,"0000000000000000");
	cc = find_rec (glmr2, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		strcpy (glmrRec.desc, "                         ");
		strcpy (glmrRec.glmr_class [0], "C");
		strcpy (glmrRec.glmr_class [1], "C");
		strcpy (glmrRec.glmr_class [2], " ");
		glmrRec.hhca_hash = 0L;
		glmrRec.parent_cnt = 0;
		glmrRec.child_cnt = 0;
		glmrRec.mod_date = TodaysDate ();
		strcpy (glmrRec.stat_flag, "0");
		strcpy (glmrRec.curr_code, "   ");
	
    	cc = abc_add (glmr2, &glmrRec);
		if (cc)
			file_err (cc, glmr2, "DBADD");
	}
	OpenGlmr ();
	OpenGlpd ();
	OpenGlln ();
	OpenGltr ();
	OpenGltc ();
	OpenGlna ();
	OpenGlid ();
	OpenGlih ();
	abc_selfield (glid, "glid_acct_no");
	abc_selfield (glih, "glih_hhih_hash");
	abc_selfield (gltr, "gltr_id_no2");

}

int
RedrawFunc (
	int		c, 
	KEY_TAB *psUnused)
{

	last_char = REDRAW;
	clear ();
	heading (1);
	DisplayGlacHots ();

	return (EXIT_FAILURE);
}

int
NoGltr (void)
{
	int	 len;
	int	 levelCnt;
	char checkAccountNo [FORM_LEN + 1];

	memset (checkAccountNo, '0', MAXLEVEL);

	checkAccountNo [0] = (char) NULL;
	for (levelCnt = 0; levelCnt < GV_cur_level; levelCnt++)
		strcat (checkAccountNo, GL_GetBit (levelCnt + 1));
    
	len = strlen (checkAccountNo);

	strcpy (glmrRec4.co_no,comr_rec.co_no);
	strcpy (glmrRec4.acc_no, checkAccountNo);

    cc = find_rec (glmr, &glmrRec4, GTEQ, "r");
	while (!cc && !strncmp (glmrRec4.co_no, comr_rec.co_no, 2) &&
           		  !strncmp (glmrRec4.acc_no, checkAccountNo, len))
	{
		gltrRec2.hhmr_hash	=	glmrRec4.hhmr_hash;
		cc = find_rec (gltr2, &gltrRec2, COMPARISON, "r");
		if (!cc)
			return (FALSE);
        
		cc = find_rec (glmr, &glmrRec4, NEXT, "r");
	}
	return (TRUE);
}

/*
 * Sort routines. See TranRec structure for details.
 */
int 
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
int 
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
int 
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
 * Sort routines. See TranRec structure for details.
 */
int 
TranSort4 (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct Trans a = * (const struct Trans *) a1;
	const struct Trans b = * (const struct Trans *) b1;

	result = strcmp (a.sortKey4, b.sortKey4);

	return (result);
}

int
CheckInterface (
	char	*accNo)
{
	sprintf (glidRec.acct_no, "%-16.16s", accNo);
	cc = find_rec (glid, &glidRec, COMPARISON, "r");
	if (cc)
		return (EXIT_SUCCESS);

	glihRec.hhih_hash	=	glidRec.hhih_hash;
	cc = find_rec (glih, &glihRec, COMPARISON, "r");
	if (cc || strcmp (glihRec.co_no, comr_rec.co_no))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}
/*
 * Search company file.
 */
void
SrchComr (
	char	*key_val)
{
	_work_open (2,0,60);
	save_rec ("#Co", "#Company Name                           Curr");

	sprintf (comr_rec.co_no, "%-2.2s", key_val);

    cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc && 
           !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
	{
		sprintf (err_str,"%s %s",comr_rec.co_name, comr_rec.base_curr);
		cc = save_rec (comr_rec.co_no, err_str);
		if (cc)
            break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
        return;

	sprintf (comr_rec.co_no, "%-2.2s", temp_str);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	return;
}

/*
 * Display Screen Heading.
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (last_char == REDRAW)
		{
			clear ();
			PV_glpd_display = TRUE;
		}

		if (scn != cur_screen)
			scn_set (scn);

		if (PV_display)
			rv_pr (ML (mlGlMess102),48,0,1);
		else
			rv_pr (ML (mlGlMess101),50,0,1);
		disp_help (132);
		if (scn == 1)
			box (0,1, 132, 3);
		else
			box (0,1, 132, 1);

		line_at (21, 0, 132);
		print_at (22,0,ML (mlStdMess038), comr_rec.co_no, comr_rec.co_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		if (PV_new_account == FALSE && scn == 1)
			DisplayAccount (TRUE);
	}
    return EXIT_SUCCESS;
}

void
DisplayInfo (void)
{
	strcpy (err_str, 
		(expanded == TRUE) ? ML ("Extra Description On ") : ML("Extra Description Off"));
	rv_pr (err_str, 110,1,1);

	strcpy (err_str, 
		(compressed == TRUE) ? ML ("Consolidate On ") : ML("Consolidate Off"));
	rv_pr (err_str, 91,1,1);
}

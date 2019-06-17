/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_budginp.c,v 5.3 2001/08/09 09:13:33 scott Exp $
|  Program Name  : (gl_budginp.c)
|  Program Desc  : (General Ledger Budget Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (19/07/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_budginp.c,v $
| Revision 5.3  2001/08/09 09:13:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:09  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:38  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_budginp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_budginp/gl_budginp.c,v 5.3 2001/08/09 09:13:33 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <getnum.h>
#include <hot_keys.h>
#include <GlUtils.h>
#include <twodec.h>
#include <tabdisp.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glshRecord	glsh_rec;
struct glsdRecord	glsd_rec;

/*
 *   Constants, defines and stuff   
 */
#define	SET_VALUES	1
#define	SET_BALANCES	2
#define	CHECK_ACCOUNTS	3

char 	*spreadTab	=	"spreadTab",
		*glmrTab	=	"glmrTab",
		*glshTab	=	"glshTab",
		*data 		= 	"data";

/*
 *   Local variables  
 */

extern int GV_cur_level, GV_max_level;

static char *PV_acc_mask;
static int  PV_acc_printed = FALSE;
static int  PV_tab_printed = FALSE;
static int  PV_budg_no = 0,
    		PV_curr_fyear,
            PV_new_budget   = TRUE,
            PV_new_table    = TRUE,
            PV_no_table     = TRUE,
            PV_no_accs      = TRUE;

static char baseCurr [4];

       char mlBudgInp [10][101];


static struct
{
    char *mask;
    int	offset;

} *form_ptr, format_tab [] =
		{
            { "%02d",		0  },
            { "  %6.2f%%",		4  },
            { "  %-20.20s",		13 },
            { "     %6.2f",		37 },
            { (char *) NULL }
		};

char *format_line = (char *) NULL;
char  form_line [81];

static struct
{
    char    loc_no [18],
            loc_desc [41];
	
    int     loc_year;
} local_rec;

extern	int		TruePosition;

static struct var vars [] =
{
	{1, LIN, "budg_no",	 2, 2, INTTYPE,
		"NN", "            ",
		" ", " ", "Budget Number  ", " ",
		 NE, NO,  JUSTLEFT, "1", "20", (char *) &glbdRec.budg_no},
	{1, LIN, "budg_desc",	 2, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "Title         ", " ",
		YES, NO,  JUSTLEFT, "", "", glbdRec.desc},
	{1, LIN, "year",	 4, 2, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Year          ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.loc_year},
	{2, LIN, "glsh_code",	 6, 87, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Code ", " ",
		 NE, NO,  JUSTLEFT, "", "", glsh_rec.code},
	{2, LIN, "glsh_desc",	 6, 100, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Name ", " ",
		YES, NO,  JUSTLEFT, "", "", glsh_rec.desc},
	{0, TAB, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL},
};

static	int	AccountFunc 	 (int, KEY_TAB *);
static	int	SpreadFunc 		 (int, KEY_TAB *);
static	int	TableFunc 		 (int, KEY_TAB *);
static	int	YearFunc 		 (int, KEY_TAB *);
static	int	EditFunc 		 (int, KEY_TAB *);

static KEY_TAB glbdKeys [] =
{
    { " [A]CCOUNTS",  'A', AccountFunc, 
       "Select/Maintain Accounts for which balances will be created." },
	{ " [S]PREAD",    'S', SpreadFunc,
      "Spread entered budget amounts useing selected Spread-table."  },
	{ " [T]ABLE",     'T', TableFunc,
      "Select/Maintain a table of spread percentages."		    },
	{ " [Y]EAR",      'Y', YearFunc,
      "Select year for which budget balances will be created."	    },
	{ " [E]DIT",	    'E', EditFunc,
      "Edit the budget Title."					    },
      END_KEYS
};
static	int	SelectAccounts 	 (int, KEY_TAB *);
static	int	TagAccounts 	 (int, KEY_TAB *);

static KEY_TAB accountKeys [] =
{
    { " [S]ELECT", 'S', SelectAccounts,
	  "Select a list of accounts."		  },
	{ " [T]AG",    'T', TagAccounts,
      "Tag/'Enter values' for selected accounts." },
      END_KEYS
};
static	int	UpdateTable 	 (int, KEY_TAB *);
static	int	SetectTable 	 (int, KEY_TAB *);
static	int	ModifyTable 	 (int, KEY_TAB *);

static KEY_TAB tableKeys [] =
{
    { " [U]PDATE", 'U', UpdateTable,
      "Save current Spread-table."				},
	{ " [S]ELECT", 'S', SetectTable,
      "Select/Enter a Spread-table."				},
	{ " [M]ODIFY", 'M', ModifyTable,
      "Modify spread percentages for current Spread-table."	},
      END_KEYS
};

static	int	SelectGlsh (int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB glshKeys [] =	
{
    { " SELECT ",	 '\r',  SelectGlsh, "Select current Spread-table." },
      END_KEYS
};
#else
static KEY_TAB glshKeys [] =	
{
    { " [RETN]",	 '\r',  SelectGlsh, "Select current Spread-table." },
      END_KEYS
};
#endif

static	int	SpreadValue 	 (int, KEY_TAB *);
static	int	SpreadAllValues (int, KEY_TAB *);
static	int	SpreadText 		 (int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB tableSpreadKeys [] =
{
    { " ALL VALUE ", CTRL ('V'), SpreadAllValues,
      "Enter a spread percentage for all following periods."	},
	{ " VALUE ",	 'V',     SpreadValue,
      "Enter a spread percentage for the current period."	},
	{ " TEXT ",	 'T',     SpreadText,
      "Enter text for the current period."			},
      END_KEYS
};
#else
static KEY_TAB tableSpreadKeys [] =
{
    { " [^V]ALUE", CTRL ('V'), SpreadAllValues,
      "Enter a spread percentage for all following periods."	},
	{ " [V]ALUE",	 'V',     SpreadValue,
      "Enter a spread percentage for the current period."	},
	{ " [T]EXT",	 'T',     SpreadText,
      "Enter text for the current period."			},
      END_KEYS
};
#endif

static	int	SetValue 		 (int, KEY_TAB *);
static	int	SetTaggedValue 	 (int, KEY_TAB *);
static	int	TagAcctNo 		 (int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB selectKeys [] =
{
    { " ALL VALUE ",'M', SetTaggedValue,
      "Enter values for all following tagged accounts in current list."},
	{ " VALUE ",	'V',     SetValue,
      "Enter a value to be spread for the current account."	      },
	{ " ALL TAG ",	'A',  TagAcctNo,
      "Tag all accounts in the current list."			      },
	{ " TAG ",   	'T',      TagAcctNo,
      "Tag/Un-tag the current account."			              },
      END_KEYS
};
#else
static KEY_TAB selectKeys [] =
{
    { " [M]ultiple Values ", 'M', SetTaggedValue,
      "Enter values for all following tagged accounts in current list."},
	{ " [S]ingle value ",	 'V',     SetValue,
      "Enter a value to be spread for the current account."	      },
	{ " [A]ll Tag ",  'A',  TagAcctNo,
      "Tag all accounts in the current list."			      },
	{ " [T]ag single ",   'T',      TagAcctNo,
      "Tag/Un-tag the current account."			              },
      END_KEYS
};
#endif

/*
 *   Local function prototypes  
 */
int  	AccountOK 			 (GLMR_STRUCT *, int);
int  	ActionList 			 (int, int);
int  	CheckAccNo 			 (char *);
int  	CheckPercent 		 (void);
int  	GetValueFunc 		 (void);
int  	heading 			 (int);
int  	PercentLeft 		 (void);
int  	ReadGlbd 			 (int);
int  	ReadGlsh 			 (int);
int  	RunGlbdHots 		 (void);
int  	spec_valid 			 (int);
int  	UpdateGlbd 			 (void);
int  	ValidateBudgetNo 	 (int);
int  	ValidateSpread 		 (int);
int  	ValueOffSet 		 (char *);
void 	CloseDB 			 (void);
void 	DisplayAcctHots 	 (void);
void 	DisplayGlbdHots 	 (void);
void 	DisplayTableHots 	 (void);
void 	EntryFunc 			 (void);
void 	InitML 				 (void);
void 	LoadDetails 		 (void);
void 	OpenDB 				 (void);
void 	PostBudget 			 (char  *, int, int, char  *, double, double);
void 	RunAcctHots 		 (void);
void 	RunTableHots 		 (void);
void 	shutdown_prog 		 (void);
void 	SpreadAccNo 		 (char *);
void 	SpreadTotal 		 (int);
void 	SrchGlbd 			 (char *);
void 	SrchGlsh 			 (char *);
void 	TableFormat 		 (char  *, int, double, char  *, double);
void 	TagAllAccounts 		 (void);
void 	UpdateGlsd 			 (char *);
void 	UpdateSpreadTable 	 (void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	int     crsr_stat;

	search_ok = TRUE;
	TruePosition	=	TRUE;
	if (argc == 2)
    {
		PV_budg_no = atoi (argv [1]);
    }

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	  */
	set_tty ();
	set_masks ();			/*  setup print using masks	  */
	init_vars (1);			/*  set default values		  */
	swide ();

	OpenDB ();

	InitML ();

	PV_acc_mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
	set_help (FN6, "FN6");

	/*
	 * Beginning of input control loop.
	 */
	crsr_stat = crsr_toggle (FALSE);
	while (!prog_exit)
	{
		/*
		 * Reset Control Flags             
		 */
		entry_exit = edit_exit = prog_exit = restart = 0;
		PV_new_budget = -1;


#ifdef GVISION
		if (PV_acc_printed)
		{
			tab_cleartext (glmrTab);
			tab_show (glmrTab, 0);
			ClearCurrentTabDisp ();
			PV_acc_printed = FALSE;
		}
#endif /* GVISION */


		EntryFunc ();

		if (prog_exit)
        {
			break;
        }

		if (restart)
        {
			continue;
        }

		if (!restart)
		{
			EditFunc (0, (KEY_TAB *) 0);
			while (!restart && RunGlbdHots ())
            {
                ;/*  do absolutely nothing */
            }
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void 	
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
	crsr_toggle (TRUE);
}



void
InitML (
 void)
{
	strcpy (mlBudgInp [1], ML ("Code"));
	strcpy (mlBudgInp [2], ML ("Name"));
}

void
CloseDB (
 void)
{
	GL_PostTerminate ();
	tab_close (glmrTab, 	TRUE);
	tab_close (spreadTab, 	TRUE);
	tab_close (glshTab, 	TRUE);

	GL_Close ();
	abc_fclose (glsd);
	abc_fclose (glsh);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("budg_no")) 
		return (ValidateBudgetNo (field));
	
	if (LCHECK ("glsh_code"))
		return (ValidateSpread (field));

	return (EXIT_SUCCESS);
}

int
ValidateBudgetNo (
 int field)
{
	if (SRCH_KEY)
	{
		SrchGlbd (temp_str);
		return (EXIT_SUCCESS);
	}

	if (!ReadGlbd (COMPARISON))
	{
		scn_display (1);
		PV_new_budget = FALSE;
		entry_exit = TRUE;
	}
	else
	{
		entry_exit = FALSE;
		PV_new_budget = TRUE;
	}
	GL_PostBudget (glbdRec.budg_no);
	
	return (EXIT_SUCCESS);
}

int ValidateSpread (
 int field)
{
	if (SRCH_KEY)
	{
		SrchGlsh (temp_str);
		move (FIELD.col + strlen (temp_str) + 3, FIELD.row);
		last_char = 0;
		return (EXIT_SUCCESS);
	}

	if (!strcmp (glsh_rec.code, "      "))
	{
		print_err (ML (mlGlMess170));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

    PV_new_table = ReadGlsh (COMPARISON);
	if (!PV_new_table)
	{
		scn_display (2);
		entry_exit = TRUE;
		LoadDetails ();
	}
	else
		entry_exit = FALSE;

	return (EXIT_SUCCESS);
}

int
YearFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
    if (local_rec.loc_year == PV_curr_fyear)
    {
        local_rec.loc_year += 1;
    }
    else if (local_rec.loc_year < PV_curr_fyear)
    {
        local_rec.loc_year += (PV_curr_fyear - local_rec.loc_year);
    }
    else
    {
        local_rec.loc_year += -1;
    }

    scn_display (1);
	crsr_toggle (FALSE);

	return (TRUE);
}

int
ReadGlbd (
	int		findFlag)
{
	strcpy (glbdRec.co_no, comm_rec.co_no);
    return (find_rec (glbd, &glbdRec, findFlag, "w"));
}

int
UpdateGlbd (
 void)
{
	int	invalid = 0;

	/*
	 * Add or update database record.
	 */
	glmrRec.mod_date = TodaysDate ();

	if (PV_new_budget) 
	{
		strcpy (glbdRec.co_no,comm_rec.co_no);
		strcpy (glbdRec.stat_flag,"0");

        cc = abc_add (glbd,&glbdRec);
		if (cc)
			file_err (cc, glbd, "DBADD");
	}
	else 
    {
        cc = abc_update (glbd,&glbdRec);
        if (cc)
            file_err (cc, glbd, "DBUPDATE");
    }

	return (invalid);
}

/*
 * Display Screen Heading 
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {      
            scn_set (scn);
        }

		clear ();
		rv_pr (ML (mlGlMess028),50,0,1);
		disp_help (132);
		box (0,1, 131, 3);
		line_at (3,1,129);
		line_at (21,0,132);

		print_at (22,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		if (PV_new_budget != -1)
			scn_display (scn);
	}

    return (EXIT_SUCCESS);
}

void
SrchGlbd (
 char *key_val)
{
    _work_open (2,0,40);
    save_rec ("#No","#Title");

	strcpy (glbdRec.co_no, comm_rec.co_no);
	glbdRec.budg_no = 0;

    cc = find_rec (glbd, &glbdRec, GTEQ, "r");
    while (!cc && !strcmp (glbdRec.co_no, comm_rec.co_no))
    {
		char	budgStr [16];

		sprintf (budgStr, "%02d", glbdRec.budg_no);
        cc = save_rec (budgStr, glbdRec.desc);
        if (cc)
            break;

    	cc = find_rec (glbd, &glbdRec, NEXT, "r");
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (glbdRec.co_no, comm_rec.co_no);
	glbdRec.budg_no = atoi (temp_str);

    cc = find_rec (glbd, &glbdRec, COMPARISON, "r");
    if (cc)
        file_err (cc, glbd, "DBFIND");
}

void
SrchGlsh (
 char *key_val)
{
	int     len;
	int		noData	=	TRUE;
	char    tmp_temp [81];

	strcpy (tmp_temp, temp_str);
	tab_add (glshTab, "#%-12.12s %-25.25s      ", "Code", "Name");

	len = strlen (key_val);
	strcpy (glsh_rec.co_no, comm_rec.co_no);
	sprintf (glsh_rec.code, "%-6.6s", key_val);

    cc = find_rec (glsh, &glsh_rec, GTEQ,"r");
	while (!cc && 
           !strcmp (glsh_rec.co_no, comm_rec.co_no) &&
           !strncmp (glsh_rec.code, key_val, len))
	{
		noData	= FALSE;
		cc = tab_add (glshTab, "%-12.12s %-25.25s",glsh_rec.code,glsh_rec.desc);
		if (cc)
			break;

		cc = find_rec (glsh, &glsh_rec, NEXT,"r");
	}
	if (noData == TRUE)
		cc = tab_add (glshTab, "%-12.12s %-25.25s"," ", " ");

	if (!tab_display (glshTab, TRUE))
	{
		if (noData == FALSE)
		{
			if (tab_scan (glshTab))
				strcpy (temp_str, tmp_temp);
		}
	}
	redraw_table (spreadTab);
	redraw_heading (spreadTab, FALSE);
	scn_display (2);
	strcpy (glbdRec.co_no, comm_rec.co_no);
}


int
SelectGlsh (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	char	tempString [81];

	tab_get (glshTab, tempString, CURRENT, 0);
	sprintf (temp_str, "%-6.6s", tempString);
	sprintf (glsh_rec.code, "%-6.6s", tempString);
	sprintf (glsh_rec.desc, "%-25.25s", " ");

	return (FN16);
}

void
EntryFunc (void)
{
	int	crsr_stat;

	scn_set (1);
	init_vars (1);
	PV_curr_fyear = local_rec.loc_year = fisc_year (comm_rec.gl_date);
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
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	int	crsr_stat;

	restart = 0;

	crsr_stat = crsr_toggle (TRUE);
	scn_set (1);
	heading (1);
	scn_display (1);
	edit (1);
	crsr_toggle (crsr_stat);
	if (!restart)
    {
		UpdateGlbd ();
    }

	else if (!PV_new_budget)
	{
		restart = 0;
		scn_display (1);
		scn_write (1);
		restart = 1;
	}
	else
    {
		scn_write (1);
    }

	crsr_toggle (crsr_stat);
	return (TRUE);
}

int 
AccountFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	if (PV_acc_printed)
	{
		tab_get (glmrTab, (char *) NULL, FIRST, 0);
		redraw_form (glmrTab);
		redraw_page (glmrTab, FALSE);
	}

	line_at (21,0,132);
	RunAcctHots ();
	DisplayGlbdHots ();

	return (TRUE);
}

int
SelectAccounts (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	int     crsr_stat, 
            len, 
            acc_len, 
            level;
	char	tmp_acc [MAXLEVEL + 1], 
			form_acc [FORM_LEN + 1], 
			srch_acc [FORM_LEN + 1];

#ifdef GVISION
	destroy_hotbuttons ();
	if (PV_acc_printed)
		tab_show (glmrTab, 0);
#endif /* GVISION */

	line_at (21,0,132);
	acc_len = strlen (PV_acc_mask);
	if (!PV_acc_printed)
	{
		tab_add (glmrTab, "# SELECT BUDGET ACCOUNTS.%-*.*s",
					49 + acc_len, 49 + acc_len, " ");
		PV_acc_printed = TRUE;
	}

	print_at (6, 1, ML (mlGlMess029), acc_len + 27, acc_len + 27, " ", " ");
	crsr_stat = crsr_toggle (TRUE);
	do
	{
		while ((level = getint (30, 6, "N")) < 0 || (level > GV_max_level))
			print_err (ML (mlStdMess025));
		do
		{
			if (last_char == FN1)
				break;
            
			getalpha (46, 6, PV_acc_mask, form_acc);
			GL_StripForm (srch_acc, form_acc);

        } while (GL_FormAccNo (form_acc, tmp_acc, 0));

    } while ((last_char != FN1) && 
             (last_char != FN16) && 
             (last_char != '\r'));

	crsr_toggle (crsr_stat);
	if (last_char == FN1)
	{
		redraw_heading (glmrTab, TRUE);
		DisplayAcctHots ();
		return (TRUE);
	}

#ifdef GVISION
	if (PV_acc_printed)
		tab_show (glmrTab, 1);
#endif /* GVISION */

	tab_add 
	(
		glmrTab, 
		"# SELECT BUDGET ACCOUNTS.%-*.*s", 
		49 + acc_len, 
		49 + acc_len, 
		" "
	);
	len = strlen (srch_acc);
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s",MAXLEVEL,MAXLEVEL,tmp_acc);

    cc = find_rec (glmr, &glmrRec, GTEQ,"r");
	while (!cc && 
           !strncmp (glmrRec.acc_no, tmp_acc, len) &&
           !strcmp (glmrRec.co_no, comm_rec.co_no))
	{
		strcpy (form_acc, glmrRec.acc_no);
		GL_FormAccNo (form_acc, glmrRec.acc_no, 0);

		if (!GL_ValidUserCode (glmrRec.acc_no))
		{
			cc = find_rec (glmr, &glmrRec, NEXT, "r");
			continue;
		}

		if (AccountOK (&glmrRec, level))
		{
			cc = 	tab_add 
				 	(
						glmrTab, 
						" %-*.*s  %-25.25s  %16.2f %3.3s  %16.2f %3.3s ",
						acc_len, 
						acc_len,
						form_acc,
						glmrRec.desc,
						DOLLARS 
						(
							GL_LocTotGlpd 
							(
								glmrRec.hhmr_hash, 
								glbdRec.budg_no,
								local_rec.loc_year, 
								1, 
								12
							)
						),
						glmrRec.curr_code,
						DOLLARS 
						(
							GL_LocTotGlpd 
							(
								glmrRec.hhmr_hash, 
								glbdRec.budg_no, 
								local_rec.loc_year, 
								1, 
								12
							)
						),
						baseCurr
				  	);
			if (cc)
            {
				break;
            }
		}

		cc = find_rec (glmr, &glmrRec, NEXT,"r");
	}

	PV_no_accs = tab_display (glmrTab, TRUE);
	DisplayAcctHots ();

	return (TRUE);
}

int
TagAccounts (
 int iUnused, 
 KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	line_at (21,0,132);

	if (!PV_no_accs)
		tab_scan (glmrTab);
	else
		print_err (ML (mlGlMess171));

	DisplayAcctHots ();
	return (TRUE);
}

int
TagAcctNo (
	int		c, 
	KEY_TAB *psUnused)
{

	if (c != 'T')
		TagAllAccounts ();
	else
		tag_toggle (glmrTab);
	
	return (TRUE);
}

void
TagAllAccounts (void)
{
	rv_pr (ML (mlGlMess031), 0, 23, 1);
	tag_all (glmrTab);
}

int
SetValue (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	tempString [120];
	int	    len, crsr_stat;
	char	currency [4];
	double	fx_tmp_val;
	double	tmp_val;

	tab_get (glmrTab, tempString, CURRENT, 0);
	len = ValueOffSet (tempString);
	sprintf (currency, "%3.3s", tempString + len + 17);
	cc = FindPocr (comm_rec.co_no, currency, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
	
	crsr_stat = crsr_toggle (TRUE);
	fx_tmp_val = getdouble (len + 1, tab_sline (glmrTab), "NNNNNNNNNNNNN.NN");
	tmp_val = CurrencyLocAmt (fx_tmp_val);
	crsr_toggle (crsr_stat);

	if (last_char != FN1)
    {
		sprintf (&tempString [len], 
                 "%16.2f %3.3s  %16.2f %3.3s", 
                 fx_tmp_val, currency, tmp_val, baseCurr);
    }
	tab_update (glmrTab, "%s", tempString);

	return (TRUE);
}

int 
SetTaggedValue (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	int	start_line;

	start_line = tab_tline (glmrTab);
	ActionList (SET_VALUES, start_line);

	return (TRUE);
}

int 
ActionList (
	int		op, 
	int		startLine)
{
	char	tab_line [256];
	int	    cc, len;

	len = strlen (PV_acc_mask);

	cc = tab_get (glmrTab, tab_line, EQUAL, startLine);

	rv_pr (op == SET_VALUES     ? ML (mlGlMess032) :
	       op == SET_BALANCES   ? ML (mlGlMess033) :
				      			  ML (mlGlMess034) , 0, 23, 1);

	while (!cc)
	{
		if (tagged (tab_line))
		{
			redraw_line (glmrTab, FALSE);
			if (op == SET_VALUES)
			{
				SetValue (0, (KEY_TAB *) 0);
				if (last_char == FN1)
					break;
			}
			else
            {
				if (op == SET_BALANCES)
                {
					SpreadAccNo (tab_line);
                }
				else
                {
					if (!CheckAccNo (tab_line))
                    {
						return (FALSE);
                    }
                }
            }
		}
		cc = tab_get (glmrTab, tab_line, NEXT, 0);
	}
	tab_get (glmrTab, (char *) NULL, EQUAL, startLine);
	clear_mess ();

	return (TRUE);
}

/*
 *  Generates format line and is scanned for offsets.  
 */
void
SpreadAccNo (
 char *s)
{
	char	tmp_acc [FORM_LEN + 1], 
            tempString [81];

	int	    len				= 0, 
            periodCnt 		= 0,
			accountLen		= 0, 
            src_ofs			= 0, 
            dst_ofs			= 0;

	long	hhmrHash		= 0L;

	double	fgnTempValue	= 0.00, 
            locTempValue	= 0.00, 
			percentValue	= 0.00,
			fgnPostValue	= 0.00, 
            locPostValue	= 0.00,
			fgnLeftover		= 0.00, 
            locLeftover		= 0.00;

	redraw_line (glmrTab, TRUE);
	len = strlen (PV_acc_mask);
	sprintf (tmp_acc, "%-*.*s", len, len, s + 1);
	len = ValueOffSet (s);
	fgnTempValue = CENTS (atof (s + len));
	locTempValue = CENTS (atof (s + len + 22));
	print_at (23, 22, ML (mlGlMess030), tmp_acc, fgnTempValue / 100);

	strcpy (glmrRec.co_no, comm_rec.co_no);
	accountLen = strlen (tmp_acc);
	for (src_ofs = dst_ofs = 0; src_ofs < accountLen; src_ofs++)
	{
		if (tmp_acc [src_ofs] != '-')
		{
			glmrRec.acc_no [dst_ofs++] = tmp_acc [src_ofs];
			glmrRec.acc_no [dst_ofs] = 0;
		}
	}

    cc = find_rec (glmr, &glmrRec, EQUAL, "r");
    if (cc)
		file_err (cc, glmr, "FIND_REC");
    
	hhmrHash = glmrRec.hhmr_hash;

	len = format_tab [1].offset;
	cc = tab_get (spreadTab, tempString, FIRST, 0);
	fgnLeftover = fgnTempValue;
	locLeftover = locTempValue;
	while (!cc && 
           (++periodCnt <= 12))
	{
		percentValue = atof (&tempString [len]);
		if (percentValue)
		{
			fgnPostValue = (fgnTempValue * percentValue) / 100.00;
			locPostValue = (locTempValue * percentValue) / 100.00;
			fgnLeftover -= fgnPostValue;
			locLeftover -= locPostValue;
			PostBudget 
			(
				tmp_acc, 
				local_rec.loc_year, 
				periodCnt,
				glmrRec.curr_code,
				(periodCnt == 12) ? fgnPostValue + fgnLeftover : fgnPostValue,
				(periodCnt == 12) ? locPostValue + locLeftover : locPostValue
			);
		}
		else
		{
			glpdRec.hhmr_hash	= hhmrHash;
			glpdRec.year	= local_rec.loc_year;
			glpdRec.budg_no	= glbdRec.budg_no;
			glpdRec.prd_no	= periodCnt;

            cc = find_rec (glpd, &glpdRec, EQUAL, "u");
			if (cc)
				abc_unlock (glpd);
			else
				abc_delete (glpd);
		}
		cc = tab_get (spreadTab, tempString, NEXT, 0);
	}
	tag_unset (glmrTab);
	redraw_line (glmrTab, FALSE);
}

int
CheckAccNo (
	char *s)
{
	int     len, crsr_stat;
	double	tmp_val;

	len = ValueOffSet (s);
	
	if (!atof (s + len))
	{
		print_mess (ML (mlGlMess158));
		crsr_stat = crsr_toggle (TRUE);
		tmp_val = getdouble (len + 1, tab_sline (glmrTab), "NNNNNNNNNNNNN.NN");
		crsr_toggle (crsr_stat);

		if (last_char == FN1)
			return (FALSE);

		sprintf (s + len, "%16.2f", tmp_val);
		tab_get (glmrTab, (char *) NULL, CURRENT, 0);
		tab_update (glmrTab, "%s", s);
	}
	return (TRUE);
}

int
ValueOffSet (
	char *s)
{
	register char *o_ptr;
	register int   offset;

	offset = strlen (s) - 26;
	o_ptr = s + offset;

	while (*o_ptr != '.')
	{
		o_ptr--;
		offset--;
	}

	return (offset - 13);
}

int
AccountOK (
 GLMR_STRUCT *acc_ptr, 
 int level)
{
	if (acc_ptr->glmr_class [0][0] != 'N' && 
		acc_ptr->glmr_class [0][0] != 'C' &&
        (!level || level == GV_cur_level))
    {
		return (TRUE);
    }

	return (FALSE);
}


int
SpreadFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	if (PV_no_accs)
	{
		print_err (ML (mlGlMess171));
	}
	else if (PV_no_table)
	{
		print_err (ML (mlGlMess172));
	}
	else if (CheckPercent ())
	{
		tab_get (glmrTab, (char *) NULL, FIRST, 0);
		redraw_form (glmrTab);
		redraw_page (glmrTab, FALSE);

        if (ActionList (CHECK_ACCOUNTS, 0))
			ActionList (SET_BALANCES, 0);
	}
	DisplayGlbdHots ();
	return (TRUE);
}

int
TableFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	if (PV_tab_printed)
	{
		redraw_form (spreadTab);
		redraw_page (spreadTab, FALSE);
	}
	RunTableHots ();
	DisplayGlbdHots ();
	return (TRUE);
}

int
SetectTable (
 int iUnused, 
 KEY_TAB *psUnused)
{
	int	crsr_stat;

#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	line_at (21,0,132);
	PV_tab_printed = TRUE;
	restart = 0;

	if (PV_new_table)
	{
		tab_add (spreadTab,"#Code         Name                           ");
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		scn_set 	(2);
		scn_write 	(2);
		scn_display (2);
		init_vars 	(2);
	}
	crsr_stat = crsr_toggle (TRUE);
	redraw_heading (spreadTab, FALSE);
	entry (2);

	if (!restart)
	{
		redraw_heading (spreadTab, FALSE);
		scn_display (2);
		edit (2);
		scn_write (2);
		redraw_heading (spreadTab, TRUE);
	}
	crsr_toggle (crsr_stat);

	if (!restart && PV_new_table)
		LoadDetails ();

	if (!restart)
		PV_no_table = FALSE;

	DisplayTableHots ();
	return (TRUE);
}

int
ModifyTable (
 int iUnused, 
 KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /* GVISION */

	if (PV_no_table)
	{
		print_err (ML (mlGlMess172));
		return (TRUE);
	}

	line_at (21,0,132);
	tab_scan (spreadTab);

	DisplayTableHots ();
	return (TRUE);
}

int 
UpdateTable (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	if (!PV_no_table && CheckPercent ())
		UpdateSpreadTable ();
	else
        print_err (ML (mlGlMess172));

	return (TRUE);
}

void
LoadDetails (
 void)
{
	int     prd_cnt = 0;
	char	tempString [81];
	double	tab_perc = 100.00;

	tab_add 
	(
		spreadTab, 
		"#.%-4.4s  %-6.6s %-4.4s  %-25.25s", 
		mlBudgInp [1],
		glsh_rec.code,
		mlBudgInp [2],
		glsh_rec.desc
	);
	glsd_rec.hhsh_hash 	= glsh_rec.hhsh_hash;
	glsd_rec.prd_no 	= 1;

    cc = find_rec (glsd, &glsd_rec, GTEQ,"r");
	while (!cc && glsd_rec.hhsh_hash == glsh_rec.hhsh_hash)
	{
		tab_perc -= DOLLARS (glsd_rec.amount);
		TableFormat 
		(
			tempString, 
			glsd_rec.prd_no,
			DOLLARS (glsd_rec.amount),
			glsd_rec.desc,
			tab_perc
		);

        if (tab_add (spreadTab, "%s", tempString))
			break;

		prd_cnt++;
		cc = find_rec (glsd, &glsd_rec, NEXT,"r");
	}
	while (++prd_cnt <= 12)
	{
		TableFormat 
		(
			tempString, 
			prd_cnt,
			(prd_cnt == 1) ? 100.0 : 0.0, 
			" ",
			0.0
		);
		if (tab_add (spreadTab, "%s", tempString))
			break;
	}
	tab_display (spreadTab, TRUE);
}

void
TableFormat (
	char	*f_line,
	int		periodNo,
	double	amount,
	char	*text,
	double	leftOver)
{
	if (!format_line)
	{
		format_line = form_line;
		*format_line = (char) NULL;
		for (form_ptr = format_tab; form_ptr->mask; form_ptr++)
        {
			strcat (format_line, form_ptr->mask);
        }
	}

	sprintf (f_line, format_line, periodNo, amount, text, leftOver);
}

int 
SpreadValue (
	int 	iUnused, 
	KEY_TAB *psUnused)
{

	GetValueFunc ();

	return (TRUE);
}

int
GetValueFunc (void)
{
	char	tmp_num [12], 
            tempString [81], 
            old_str [81];
	int     len, 
            crsr_stat;
	double	tmp_val;

	tab_get (spreadTab, tempString, CURRENT, 0);
	strcpy (old_str, tempString);
	len = format_tab [1].offset;
	
	crsr_stat = crsr_toggle (TRUE);
	tmp_val = getdouble (len + 87, tab_sline (spreadTab), "NNN.NN");
	crsr_toggle (crsr_stat);

	if (last_char != FN1)
	{
		sprintf (tmp_num, format_tab [1].mask, tmp_val);
		strncpy (&tempString [len - 2], tmp_num, strlen (tmp_num));
	}
	tab_update (spreadTab, "%s", tempString);
	if (last_char == FN1 || !PercentLeft ())
	{
		tab_update (spreadTab, "%s", old_str);
		redraw_line (spreadTab, FALSE);
		return (FALSE);
	}

	return (TRUE);
}

int
SpreadAllValues (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	tempString [81];
	int     prd_cnt, 
            curr_line, 
            ret_val = TRUE;


	prd_cnt = curr_line = tab_tline (spreadTab);

	cc = tab_get (spreadTab, tempString, EQUAL, curr_line);
	while (!cc && 
           (prd_cnt++ <= 12))
	{
		if (!GetValueFunc ())
		{
			ret_val = FALSE;
			break;
		}
		cc = tab_get (spreadTab, tempString, NEXT, 0);
	}
	tab_get (spreadTab, tempString, EQUAL, curr_line);

	return (ret_val);
}


int 
PercentLeft (void)
{
	char	tempString [81];
	int     len, 
            prd_cnt = 0, 
            curr_line, 
            ret_val = TRUE;
	double	used_perc = 0.00, 
            bit_perc = 0.0;

	curr_line = tab_tline (spreadTab);

	len = format_tab [1].offset;
	cc = tab_get (spreadTab, tempString, FIRST, 0);
	while (!cc && (++prd_cnt <= 12))
	{
		bit_perc = atof (&tempString [len]);
		used_perc += twodec (bit_perc);
		used_perc  = twodec (used_perc);

        if (twodec (used_perc) > 100.00)
		{
			print_err (ML (mlGlMess173));
			ret_val = FALSE;
			break;
		}
		cc = tab_get (spreadTab, tempString, NEXT, 0);
	}
	if (ret_val)
		SpreadTotal (curr_line);

	tab_get (spreadTab, tempString, EQUAL, curr_line);

	return (ret_val);
}

int
CheckPercent (void)
{
	char	tempString [81];

	cc = tab_get (spreadTab, tempString, LAST, 0);
	if (atof (&tempString [format_tab [3].offset]) > 0.001)
	{
		print_err (ML (mlGlMess174));
		return (FALSE);
	}

	return (TRUE);
}

void
SpreadTotal (
	int		currPeriod)
{
	int     prd_cnt = 0;
	char	tempString [81];
	double	tab_perc = 100.00, 
            bit_perc = 0.0;

	cc = tab_get (spreadTab, tempString, FIRST, 0);
	while (!cc && 
           (++prd_cnt <= 12))
	{
		bit_perc = atof (&tempString [format_tab [1].offset]);
		tab_perc -= bit_perc;
		
		if (prd_cnt >= currPeriod)
		{
			TableFormat (tempString, prd_cnt, bit_perc, " ", tab_perc);
			tab_update (spreadTab, "%s", tempString);
			redraw_line (spreadTab, FALSE);
		}
		cc = tab_get (spreadTab, tempString, NEXT, 0);
	}
}

int
SpreadText (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char    tempString [81], 
            tmp_desc [81];
	int     len, 
            crsr_stat;


	tab_get (spreadTab, tempString, CURRENT, 0);
	len = format_tab [2].offset;
	
	crsr_stat = crsr_toggle (TRUE);
	last_char = FN4;
	sprintf (tmp_desc, "%-20.20s", &tempString [len]);
	clip (tmp_desc);
	getalpha (len+87, tab_sline (spreadTab), "UUUUUUUUUUUUUUUUUUUUU", tmp_desc);
	crsr_toggle (crsr_stat);

	if (last_char != FN1)
    {
		strncpy (&tempString [len], tmp_desc, strlen (tmp_desc));
    }
	tab_update (spreadTab, "%s", tempString);

	return (TRUE);
}

void
UpdateSpreadTable (void)
{
	int     findFlag = FIRST;
	char	tmp_line [81];

	print_mess (ML (mlStdMess035));

	glsh_rec.mod_date = TodaysDate ();
	if (PV_new_table)
	{
		strcpy (glsh_rec.co_no, comm_rec.co_no);
		strcpy (glsh_rec.stat_flag,"0");

        cc = abc_add (glsh, &glsh_rec);
		if (cc)
			file_err (cc, glsh, "DBADD");
        
		if (ReadGlsh (EQUAL)) /*	Re-read to get hash	*/
			file_err (cc, glsh, "DBFIND");
	}
	else 
    {
        cc = abc_update (glsh, &glsh_rec);
        if (cc)
            file_err (cc, glsh, "DBUPDATE");
    }

	while (!tab_get (spreadTab, tmp_line, findFlag, 0))
	{
		UpdateGlsd (tmp_line);
		findFlag = NEXT;
	}
	PV_new_table = FALSE;
	clear_mess ();
}

void
UpdateGlsd (
	char *tab_line)
{
	int	new_line;

	glsd_rec.hhsh_hash = glsh_rec.hhsh_hash;
	glsd_rec.prd_no = atoi (tab_line + format_tab [0].offset);
	new_line = find_rec (glsd, &glsd_rec, EQUAL,"u");

	glsd_rec.amount = CENTS (atof (tab_line + format_tab [1].offset));
	strcpy (glsd_rec.action, "P");
	sprintf (glsd_rec.desc, 
             "%-20.20s",
             tab_line + format_tab [2].offset);
	strcpy (glsd_rec.stat_flag, "0");

	if (new_line)
	{
        cc = abc_add (glsd, &glsd_rec);
		if (cc)
			file_err (cc, glsd, "DBADD");
	}
	else 
    {
        cc = abc_update (glsd, &glsd_rec);
        if (cc)
            file_err (cc, glsd, "DBUPDATE");
    }
}

int
RunGlbdHots (void)
{
	heading (1);

	restart = FALSE;
	DisplayGlbdHots ();
	if (run_hotkeys (glbdKeys, null_func, null_func))
		restart = TRUE;

	return (FALSE);
}

void
DisplayGlbdHots (void)
{
	line_at (21,0,132);
	disp_hotkeys (21, 0, 132, glbdKeys);
}

void
RunAcctHots (void)
{
	restart = FALSE;
	DisplayAcctHots ();
	run_hotkeys (accountKeys, null_func, null_func);
}

void
DisplayAcctHots (void)
{
	line_at (21,0,132);
	disp_hotkeys (21, 0, 132, accountKeys);
}

void
RunTableHots (void)
{
	restart = FALSE;
	DisplayTableHots ();
	run_hotkeys (tableKeys, null_func, null_func);
}

void
DisplayTableHots (void)
{
	line_at (21,0,132);
	disp_hotkeys (21, 0, 132, tableKeys);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	GL_PostSetup (comm_rec.co_no, PV_budg_no);
	open_rec (glsh, glsh_list, GLSH_NO_FIELDS, "glsh_id_no");
	open_rec (glsd, glsd_list, GLSD_NO_FIELDS, "glsd_id_no");
	tab_open (glmrTab, selectKeys, 5,  0, 13, FALSE);
	tab_open (glshTab, glshKeys, 5, 86, 12, FALSE);
	tab_open (spreadTab, tableSpreadKeys, 5, 86, 12, TRUE);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);
	
	abc_fclose (comr);

	OpenGlmr ();
	OpenGlbd ();
}

int
ReadGlsh (
	int findFlag)
{
	strcpy (glsh_rec.co_no,comm_rec.co_no);

    return (find_rec (glsh, &glsh_rec, findFlag, "w"));
}

void
PostBudget (
	char   *accNo, 
	int    year, 
	int    period, 
	char   *currCode, 
	double fgnValue, 
	double locValue)
{
	glpdRec.hhmr_hash	= glmrRec.hhmr_hash;
	glpdRec.year		= year;
	glpdRec.budg_no		= glbdRec.budg_no;
	glpdRec.prd_no		= period;
    cc = find_rec (glpd, &glpdRec, EQUAL, "r");
	if (!cc)
	{
		fgnValue -= glpdRec.fx_balance;
		locValue -= glpdRec.balance;
	}
	_PostAccount (accNo, year, period, currCode, fgnValue, locValue);
}

/* [ end of file ] */

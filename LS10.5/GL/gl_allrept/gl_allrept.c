/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_allrept.c,v 5.6 2002/07/17 09:57:12 scott Exp $
|  Program Name  : (gl_allrept.c)
|  Program Desc  : (Display and Prints General Ledger Summary)
|                  (Trial Balance)
|---------------------------------------------------------------------|
|  Date Written  : (04/08/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_allrept.c,v $
| Revision 5.6  2002/07/17 09:57:12  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2001/08/28 10:11:43  robert
| additional update for LS10.5-GUI
|
| Revision 5.4  2001/08/28 08:46:02  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/09 09:13:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:00  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:25  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_allrept.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_allrept/gl_allrept.c,v 5.6 2002/07/17 09:57:12 scott Exp $";

/*
 * Include file dependencies 
 */
#include <pslscr.h>
#include <tabdisp.h>
#include <GlUtils.h>
#include <getnum.h>
#include <hot_keys.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 * Local variables.
 */
int     ln_num 			= 0,
		localPrint 		= FALSE,
		LtdReport 		= FALSE,
		PV_print 		= FALSE,
		PV_curr_fyear	= 0,
        PV_budget 		= 0,
        printerNo		= 1;

FILE    *fout;

char    previousAccNo [FORM_LEN + 1];

double	totalDebit		=	0.00, 
		totalCredit		=	0.00;

extern	int	GV_cur_level, GV_max_level;

char 	*data = "data";

/*
 * Constants, defines and stuff 
 */
#define InternalPageSize		15

#define	PSL_PRINT
#define	X_OFF			26
#define	Y_OFF			1
#define SCREEN_WIDTH	132

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

static struct
{
	char 	yearType [10];
	int	 	accountLevel,
         	periodNo,
         	budgetNo,
         	year;
} local_rec;
		
static	struct	var	vars []	=	
{
	{1, LIN, "accountLevel", 2, 36, INTTYPE, 
		"N", "          ", 
		" ", " ", "Account Level", " ", 
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.accountLevel}, 
	{1, LIN, "periodNo", 2, 69, INTTYPE, 
		"NN", "                          ", 
		" ", " ", "Period", " ", 
		NE, NO, JUSTLEFT, "", "12", (char *) &local_rec.periodNo}, 
	{1, LIN, "year", 3, 36, INTTYPE, 
		"NNNN", "                          ", 
		" ", " ", "Year", " ", 
		NA, NO, JUSTLEFT, "", "", (char *) &local_rec.year}, 
	{1, LIN, "type", 3, 41, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.yearType}, 
	{1, LIN, "budgetNo", 3, 69, INTTYPE, 
		"NN", "                          ", 
		" ", " ", "Budget", " ", 
		NE, NO, JUSTLEFT, "", "", (char *) &local_rec.budgetNo}, 
	{0, TAB, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Local function prototypes 
 */
int	 HistoryFunction 		 (int, KEY_TAB *);
int	 YearFunction 			 (int, KEY_TAB *);
int	 PrintTrialBalance 		 (int, KEY_TAB *); 
int	 DisplayTrialBalance 	 (int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB chartKeys [] =
{
	   { " DISPLAY ",    'D', DisplayTrialBalance,
		"Display Chart of Accounts to screen." },
	   { " PRINT ", 'P', PrintTrialBalance,
		"Print Chart of Accounts to printer."		  },
	   { " HISTORY ",  'H', HistoryFunction,
		"Rotate through valid history years."		  },
	   { " YEAR ",  'Y', YearFunction,
		"Toggle year between Next / Previous year."	  },
	   END_KEYS
};
#else
static KEY_TAB chartKeys [] =
{
	   { " [D]isplay ",    'D', DisplayTrialBalance,
		"Display Chart of Accounts to screen." },
	   { " [P]rint ", 'P', PrintTrialBalance,
		"Print Chart of Accounts to printer."		  },
	   { " [H]istory ",  'H', HistoryFunction,
		"Rotate through valid history years."		  },
	   { " [Y]ear ",  'Y', YearFunction,
		"Toggle year between Next / Previous year."	  },
	   END_KEYS
};
#endif
/*
 * Local function prototypes.
 */
int  	spec_valid 			 (int);
int  	heading 			 (int);
void 	shutdown_prog 		 (void);
void 	psl_print 			 (void);
void 	OpenDB 				 (void);
void 	CloseDB				 (void);
void 	ProcTotals 			 (void);
void 	RunTrialHots 		 (void);
void 	DisplayTrialHotKeys (void);
void 	ProcessFile			 (void);
void 	PrintAccounts		 (void);
void 	InitPrintOutput 	 (void);
void 	InitScreenOutput 	 (void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	char	levelComment [81];
	char	*sptr;

	OpenDB ();

	GL_SetAccWidth (comm_rec.co_no, TRUE);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (strcmp (sptr, "gl_allprnt"))
	{
		init_scr ();	/*  sets terminal from termcap	  */
		set_tty ();
		swide ();

		SETUP_SCR (vars);

		if (argc == 2)
        {
        	printerNo = atoi (argv [1]);
			if (printerNo == 0)
            	printerNo = 1;
        }

		set_help (FN6, "FN6");

		sprintf (levelComment,
                 ML ("Level must be in range (1 to %d)."), GV_max_level);
        vars [label ("accountLevel")].comment = levelComment;

		set_masks ();		/*  setup print using masks	  */
		do
		{
            prog_exit	= FALSE;
			restart 	= FALSE;
            init_vars (1);	/*  set default values		  */
            PV_curr_fyear = local_rec.year = fisc_year (comm_rec.gl_date);
			strcpy (local_rec.yearType, ML ("(Current)"));
			heading (1);
			init_ok		= FALSE;
			entry (1);
			init_ok 	= TRUE;
			if (restart)
				continue;
		
			if (!prog_exit)
			{
				clear ();
				fflush (stdout);
				crsr_off ();
				RunTrialHots ();
			}
		} while (!prog_exit);
	}
	else
	{
		dsp_screen ("Printing Summary trial balance", 
                     comm_rec.co_no,
                     comm_rec.co_name);
		if (argc != 5)
		{
			clear ();
			print_at (0,0, "Usage %s", argv [0]);
			print_at (1,2, "#1 - [Level Number]");
			print_at (2,2, "#2 - [Year]");
			print_at (3,2, "#3 - [Period Number]");
			print_at (4,2, "#4 - [Printer Number]");
			sleep (10);
			shutdown_prog ();
            return (EXIT_SUCCESS);
		}
		PV_print = TRUE;
		local_rec.accountLevel	= atoi (argv [1]);
		local_rec.year 			= atoi (argv [2]);
		local_rec.periodNo 		= atoi (argv [3]);
		printerNo 				= atoi (argv [4]);

		GL_SetMask (glctRec.format);

		InitPrintOutput ();
		ProcessFile ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
DisplayTrialBalance (
	int		iUnused, 
	KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif /*GVISION*/
	InitScreenOutput ();
	ProcessFile ();
	heading (1);
	DisplayTrialHotKeys ();

	return (EXIT_SUCCESS);
}

int
PrintTrialBalance (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	char	executeString [128];

#ifdef GVISION
	destroy_hotbuttons ();
#endif /*GVISION*/

	printerNo = get_lpno (0);
	sprintf 
	 (
		executeString, 
		"gl_allprnt %d %d %d %d",
		local_rec.accountLevel,
		local_rec.year, 
		local_rec.periodNo,
		printerNo
	);
		
	PrintReport (executeString, "Summary Trial Balance", 132);
	heading (1);
	DisplayTrialHotKeys ();
#ifdef GVISION
	set_hotkeys (chartKeys, null_func, null_func);
#endif	/* GVISION */

    return (EXIT_SUCCESS);
}

void
InitScreenOutput (void)
{
	char	envLine [160];
	char	baseCurr [4];

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);
	/*
	 * Heading and Page Format For Screen Output Display.
	 */
	Dsp_open (0,1,InternalPageSize);
	sprintf (envLine, 
            "SUMMARY TRIAL BALANCE DISPLAY - %-2.2s %-40.40s                                        LEVEL  :  %2d  ",
            comm_rec.co_no,comm_rec.co_name,
            local_rec.accountLevel);
	Dsp_saverec (envLine);
	sprintf (err_str, "         Account number        | Narrative               |Cls|Cur| Debit amount  | Credit amount |  Debit (%3.3s)  | Credit (%3.3s)  ", baseCurr, baseCurr);
	Dsp_saverec (err_str);
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN]  [PREV SCREEN] [EDIT/END]  ");
}

void
InitPrintOutput (
 void)
{
	char baseCurr [4];

	/*
	 * Heading and Page Format For Printer Output Display.
	 */
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);

    fout = popen ("pformat", "w");
    if (!fout)
    {
		file_err (errno, "pformat", "POPEN");
    }

	/*
	 * Start output to standard print.
	 */
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n", printerNo);
	fprintf (fout,".PL60\n");
	fprintf (fout,".12\n");
	fprintf (fout,".PI10\n");
	fprintf (fout,".L132\n");
	fprintf (fout,".B1\n");

	fprintf (fout,".ESUMMARY TRIAL BALANCE : Level : %d\n",
							local_rec.accountLevel);
	fprintf (fout,".E For %-2.2s  %-40.40s\n", comm_rec.co_no,comm_rec.co_name);
	fprintf (fout,".B1\n");
	fprintf (fout,".Efor Period %02d/%04d\n", local_rec.periodNo,
						 local_rec.year);
	fprintf (fout,".B1\n");

	fprintf (fout, ".R===========================================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=============================================\n");

	fprintf (fout, "===========================================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=============================================\n");

	fprintf (fout, "|        ACCOUNT NUMBER         ");
	fprintf (fout, "| NARRATIVE               |CLS|CUR");
	fprintf (fout, "|     DEBIT     |     CREDIT    |  DEBIT (%3.3s)  |  CREDIT (%3.3s) |\n", baseCurr, baseCurr);

	fprintf (fout, "|-------------------------------");
	fprintf (fout, "|-------------------------|---|---");
	fprintf (fout, "|---------------|---------------|---------------|---------------|\n");
}


void
ProcessFile (void)
{

	char tempGlCode [FORM_LEN + 1];
	int	 firstTime = TRUE;
	int	 len, printed = 0;

	totalDebit = totalCredit = 0.0;

	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.glmr_class [0], "F");
	sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL," ");
	
	cc = find_rec (glmr, &glmrRec, GTEQ , "r");
	len = strlen (GL_GetBit (GV_cur_level));

	LtdReport	=	FALSE;
	GL_StripForm (tempGlCode, GL_GetSecure (comm_rec.co_no));

	while (!cc && 
           !strcmp (glmrRec.co_no,comm_rec.co_no) &&
           (glmrRec.glmr_class [0][0] == 'F'))
	{
		if (strncmp (glmrRec.acc_no, tempGlCode, strlen (tempGlCode)) > 0)
			break;

		if (strncmp (glmrRec.acc_no, tempGlCode, strlen (tempGlCode)))
		{
			cc = find_rec (glmr, &glmrRec, NEXT , "r");
			LtdReport	=	TRUE;
			continue;
		}
		if (firstTime)	
		{
			strcpy (previousAccNo,glmrRec.acc_no);
			firstTime = FALSE;
		}

		if (PV_print && !localPrint)
        {
			dsp_process ("Account",glmrRec.acc_no);
        }
		PrintAccounts ();
		printed = TRUE;

		cc = find_rec (glmr, &glmrRec, NEXT , "r");
	}

	if (printed)
		ProcTotals ();
	else if (!PV_print)
	{
		for (len = 0; len < 7; len++)
            Dsp_saverec ("        ");
		Dsp_saverec (ML ("No Accounts Found in Entered Range for your security."));
	}
	if (!PV_print)
	{
		Dsp_srch ();
		Dsp_close ();
	}
}

void
PrintAccounts (void)
{

	char	accBalance [128],
			accountClass [4],
			envLine [160],
			formAccNo [FORM_LEN + 1];

	double	locPeriodBalance = 0.0, 
			fgnPeriodBalance = 0.0;
	int		len	=	0;

	strcpy (formAccNo, glmrRec.acc_no);
	GL_FormAccNo (formAccNo, glmrRec.acc_no, 0);

	/*
	 * Ignore Account with level greater than selected level.
 	 */
	if (GV_cur_level > local_rec.accountLevel)
		return;

	/*
	 * Ignore if Account greater than current and not a posting account.
	 */
    if ( (glmrRec.glmr_class [2][0] != 'P') &&
        (GV_cur_level < local_rec.accountLevel))
    {
        return;
    }

	locPeriodBalance	=	GL_LocTotGlpd
							(
								glmrRec.hhmr_hash, 
								PV_budget,
								local_rec.year, 
								1, 
								local_rec.periodNo
							);
	locPeriodBalance	=	DOLLARS (locPeriodBalance);

	fgnPeriodBalance	=	GL_FgnTotGlpd
							(
								glmrRec.hhmr_hash, 
								PV_budget,
								local_rec.year, 
								1, 
								local_rec.periodNo
							);
	fgnPeriodBalance	=	DOLLARS (fgnPeriodBalance);

	len = strlen (GL_GetBit (GV_cur_level));

	strcpy (previousAccNo,glmrRec.acc_no);

	if (ln_num >= InternalPageSize)
    {
		ln_num = ln_num % InternalPageSize;
    }

	sprintf (accountClass, "%c%c%c",	glmrRec.glmr_class [0][0],
										glmrRec.glmr_class [1][0],
										glmrRec.glmr_class [2][0]);

	if (locPeriodBalance < 0.0)
	{
		if (!PV_print)
        {
            sprintf (accBalance, "               ^E%15.2f^E               ^E%15.2f|",-fgnPeriodBalance, -locPeriodBalance);
        }
		else
        {
            sprintf (accBalance, "               |%15.2f|               |%15.2f", -fgnPeriodBalance, -locPeriodBalance);
        }
		totalCredit -= locPeriodBalance;
	}
	else if (locPeriodBalance > 0.0)
	{
		if (!PV_print)
        {
			sprintf (accBalance, "%15.2f^E               ^E%15.2f^E               |", fgnPeriodBalance, locPeriodBalance);
        }
		else
        {
			sprintf (accBalance, "%15.2f|               |%15.2f|               ", fgnPeriodBalance, locPeriodBalance);
        }
		totalDebit += locPeriodBalance;
	}
	else
    {
		if (!PV_print)
        {
			strcpy (accBalance, "               ^E               ^E               ^E               ");
        }
		else
        {
			strcpy (accBalance, "               |               |               |               ");
        }
    }

	if (!PV_print)
	{
		sprintf (envLine,
                 "%-31.31s^E%25.25s^E%3.3s^E%3.3s^E%s",
                 formAccNo, 
                 glmrRec.desc, 
                 accountClass, 
                 glmrRec.curr_code, 
                 accBalance);

		Dsp_saverec (envLine);
	}
	else
	{
		sprintf (envLine,
                "%-31.31s|%25.25s|%3.3s|%3.3s|%s",
                formAccNo, 
                glmrRec.desc, 
                accountClass, 
                glmrRec.curr_code, 
                accBalance);

		fprintf (fout, "|%s|\n", envLine);
	}
	ln_num++;
}

void
ProcTotals (void)
{
	char	line [160];

	if (!PV_print)
	{
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGJGGGJGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGG^^");
		if (LtdReport)
		{
			Dsp_saverec (ML (" ^1 PLEASE NOTE THAT FULL TRIAL BALANCE COULD NOT BE PRODUCED DUE TO SECURITY LIMITATION OF USER.^6      "));
		}
		sprintf (line,
				 "%-67.67s                              ^E%15.2f^E%15.2f", 
				 ML ("TOTALS"), 
				 totalDebit, 
				 totalCredit);
		Dsp_saverec (line);
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGG^^");
	}
	else
	{
		if (LtdReport)
		{
			strcpy (err_str, ML ("PLEASE NOTE THAT FULL TRIAL BALANCE COULD NOT BE PRODUCED DUE TO SECURITY LIMITATION OF USER"));
			fprintf (fout, "| %-127.127s |\n", err_str);
		}
		fprintf (fout, "|-------------------------------");
		fprintf (fout, "|-------------------------");
		fprintf (fout, "|---|---|---------------");
		fprintf (fout, "|---------------|---------------|---------------|\n");
		fprintf (fout, "|   TOTALS                      ");
		fprintf (fout, "                                                  ");
		fprintf (fout, "                |%15.2f|%15.2f|\n", totalDebit,totalCredit);
	}
}

/*
 * Open data base files.
 */
void	
OpenDB (void)
{
	abc_dbopen (data);

	read_comm	 (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec	 (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");

	OpenGlpd ();
	OpenGlmr ();
	abc_selfield (glmr, "glmr_id_no2");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	if (PV_print)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
	CloseDB (); 
	FinishProgram ();
}
int
spec_valid (
	int		field)
{
	if (LCHECK ("accountLevel"))
	{
		if (dflt_used)
			local_rec.accountLevel = GV_max_level;
		else if (local_rec.accountLevel > GV_max_level)
        {
			return print_err (ML (mlGlMess138),
                              local_rec.accountLevel, 
                              GV_max_level);	
        }
	}
	if (LCHECK ("periodNo") && dflt_used)
	{
		if (local_rec.year >= PV_curr_fyear)
		{
			DateToFinDMY 
			(
				comm_rec.gl_date,
				comm_rec.fiscal, 
				NULL, 
				&local_rec.periodNo,
				NULL
			);
		}
		else
			local_rec.periodNo = 12;
	}

	if (LCHECK ("budgetNo"))
	{
		if (getenv ("BUDGET") && dflt_used)
			local_rec.budgetNo = atoi (getenv ("BUDGET"));
		PV_budget = local_rec.budgetNo;
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Screen Heading 
 */
int
heading (
	int	scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	sprintf (err_str, " %s ", ML (mlGlMess139));
	rv_pr (err_str, 112, 0, 1);
	rv_pr (ML (mlGlMess036), 30,0,1);
	box (20,1, 90, 2);

	line_at (21, 0, SCREEN_WIDTH);
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	line_cnt = 0;
	scn_write (scn);
	scn_display (scn);
	
    return (EXIT_SUCCESS);
}

void
RunTrialHots (void)
{
	heading (1);

	restart = FALSE;
	DisplayTrialHotKeys ();
	if (run_hotkeys (chartKeys, null_func, null_func))
		restart = TRUE;

	return;
}

void
DisplayTrialHotKeys (void)
{
	line_at (21, 0, SCREEN_WIDTH);
	disp_hotkeys (21, 0, SCREEN_WIDTH, chartKeys);
}

int	
YearFunction (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
    int 	localYearAdjustment;

	if (local_rec.year == PV_curr_fyear)
		strcpy (local_rec.yearType, ML ("(Future) "));
	else
		strcpy (local_rec.yearType, ML ("(Current)"));

    if (local_rec.year == PV_curr_fyear)
    {
        localYearAdjustment = 1;
    }
    else if (local_rec.year < PV_curr_fyear)
    {
        localYearAdjustment = (PV_curr_fyear - local_rec.year);
    }
    else
    {
        localYearAdjustment = -1;
    }
    local_rec.year += localYearAdjustment;

	DSP_FLD ("year");
	DSP_FLD ("type");
	return (TRUE);
}

int	
HistoryFunction (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	static	int	historyIncr = 0;

	if (!glctRec.history)
	{
		/*
		 * No history is held on this system!
		 */
		print_mess (ML (mlGlMess137));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (glctRec.history == 1)
    {
		local_rec.year = PV_curr_fyear - 1;
    }
	else
    {
		if (local_rec.year >= PV_curr_fyear)
		{
			historyIncr = -1;
			local_rec.year = PV_curr_fyear;
		}
		else
        {
			if (local_rec.year == PV_curr_fyear - 1)
            {
				historyIncr = -1;
            }
			else
            {
				if (local_rec.year == PV_curr_fyear - glctRec.history)
                {
                    historyIncr = 1;
                }
            }
        }
    }
	local_rec.year += historyIncr;
	DSP_FLD ("year");
	strcpy (local_rec.yearType, ML ("(History)"));
	DSP_FLD ("type");
	return (TRUE);
}

void
psl_print (void)
{
	printerNo = get_lpno (0);
	rv_pr (ML (mlStdMess035), 28, 2, 1);

	PV_print 	= TRUE;
	localPrint 	= TRUE;
	InitPrintOutput ();
	ProcessFile ();
	fprintf (fout, ".EOF\n");
	pclose (fout);
	localPrint 	= FALSE;
	PV_print 	= FALSE;
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_acclst.c,v 5.5 2002/07/17 09:57:12 scott Exp $
|  Program Name  : (gl_acclst.c)
|  Program Desc  : (Display and Prints Summarised listing of)
|                  (specified range of accounts)
|---------------------------------------------------------------------|
|  Date Written  : (11/07/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_acclst.c,v $
| Revision 5.5  2002/07/17 09:57:12  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/08/28 08:46:01  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/09 09:13:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:26:59  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:21  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_acclst.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_acclst/gl_acclst.c,v 5.5 2002/07/17 09:57:12 scott Exp $";

/*
 * Include file dependencies 
 */
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <GlUtils.h>
#include <getnum.h>
#include <hot_keys.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <get_lpno.h>

/*
 * Constants, defines and stuff 
 */
#define	PSL_PRINT
#define	X_OFF   36
#define	Y_OFF   6

#include	"schema"

struct	commRecord	comm_rec;
struct	comrRecord	comr_rec;

#define InternalPageSize   11

char 	*glln2 = "glln2",
		*glmr2 = "glmr2",
		*data  = "data";

/*
 * Local variables 
 */
int     localPrint 		= FALSE,
		ln_num 			= 0,
		lineDrawn 		= FALSE,
        PV_print 		= FALSE,
        PV_budget		= 0,
        PV_curr_fyear	= 0,
		link_level 		= 0,
        printerNumber 	= 1;

extern	int		GV_cur_level, 
				GV_max_level;

FILE    *fout;

GLLN_STRUCT glln_rec2;

char    baseCurr 			[4],
		previousAccount 	[FORM_LEN + 1],
		PV_endAccount 		[FORM_LEN + 1], 
        PV_startAccount 	[FORM_LEN + 1],
		lineString 			[200],
		graphString			[200],
		accCurrDispFormat	[31],
		commCurrDispFormat	[31];

double	openingTotal	=	0.00,
		monthTotal		=  	0.00,
		carriedTotal	=	0.00;


static struct
{	  
    char    startAccNo 	[FORM_LEN + 1],
            endAccNo 	[FORM_LEN + 1],
            accountType [2],
            yearType 	[10];
    int     accLevel,
            year,
            budgetNo,
            startPeriod,
            endPeriod;
    char    currCode [4];
    char    currDisp [30];
} local_rec;

		
static	struct	var	vars []	=	
{
	{1, LIN, "accountType", 2, 22, CHARTYPE, 
		"U", "                          ", 
		" ", "F", "Account Type ", "Account type F)inancial or N)on-financial.", 
		NE, NO, JUSTLEFT, "NF", "", local_rec.accountType}, 
	{1, LIN, "levelNo", 2, 66, INTTYPE, 
		"N", "          ", 
		" ", "0", "Account Level", " ", 
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.accLevel}, 
	{1, LIN, "startAccNo", 3, 22, CHARTYPE, 
		"NNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ", 
		" ", "0", "Start Account", " ", 
		NE, NO, JUSTLEFT, "0123456789-", "", local_rec.startAccNo}, 
	{1, LIN, "endAccNo", 3, 66, CHARTYPE, 
		"NNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ", 
		" ", "99999999999999", "End Account", " ", 
		NE, NO, JUSTLEFT, "0123456789-", "", local_rec.endAccNo}, 
	{1, LIN, "start_prd", 4, 22, INTTYPE, 
		"NN", "              ", 
		" ", "0", "Start Period", " ", 
		NE, NO, JUSTLEFT, "0", "12", (char *) &local_rec.startPeriod}, 
	{1, LIN, "end_prd", 4, 66, INTTYPE, 
		"NN", "              ", 
		" ", "0", "End Period", " ", 
		NE, NO, JUSTLEFT, "0", "12", (char *) &local_rec.endPeriod}, 
	{1, LIN, "curr", 4, 100, CHARTYPE, 
		"UUU", "                          ", 
		" ", " ", "Currency", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.currCode}, 
	{1, LIN, "year", 5, 22, INTTYPE, 
		"NNNN", "              ", 
		" ", " ", "Year", " ", 
		NA, NO, JUSTLEFT, "", "", (char *) &local_rec.year}, 
	{1, LIN, "ytype", 5, 27, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.yearType}, 
	{1, LIN, "bdg_no", 5, 66, INTTYPE, 
		"NN", "                          ", 
		" ", " ", "Budget", " ", 
		NE, NO, JUSTLEFT, "", "", (char *) &local_rec.budgetNo}, 
	{1, LIN, "dispCurr", 5, 100, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAA", "                          ", 
		" ", " ", "Currency Mode", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.currDisp}, 
	{0, TAB, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*===============================
|   Local function prototypes   |
===============================*/
int 	PrintSummary 	(int, KEY_TAB *);
int 	DisplaySummary 	(int, KEY_TAB *);
int 	YearFunction 	(int, KEY_TAB *);
int 	HistoryFunction (int, KEY_TAB *);
int 	CurrFunction 	(int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB summaryKeys [] =
{
	   { " DISPLAY ", 'D', DisplaySummary,
		"Display Summarised Account listing to screen." },
	   { " PRINT ",	  'P', PrintSummary,
		"Print Summarised Account listing to printer."	},
	   { " HISTORY ", 'H', HistoryFunction,
		"Rotate through valid history years."		},
	   { " YEAR ",	  'Y', YearFunction,
		"Toggle year between Next / Previous year."	},
	   { " CURRENCY ",	  'C', CurrFunction,
		"Toggle Currency Mode."	},
	   END_KEYS
};

#else
static KEY_TAB summaryKeys [] =
{
	   { " [D]ISPLAY ", 'D', DisplaySummary,
		"Display Summarised Account listing to screen." },
	   { " [P]RINT ",	  'P', PrintSummary,
		"Print Summarised Account listing to printer."	},
	   { " [H]ISTORY ", 'H', HistoryFunction,
		"Rotate through valid history years."		},
	   { " [Y]EAR ",	  'Y', YearFunction,
		"Toggle year between Next / Previous year."	},
	   { " [C]URRENCY ",	  'C', CurrFunction,
		"Toggle Currency Mode."	},
	   END_KEYS
};
#endif

/*
 * Local function prototypes 
 */
int	 	SetStartAccount 		(void);
int  	SetEndAccount 			(void);
int  	spec_valid 				(int);
int  	heading 				(int);
void 	DisplaySummaryHotKeys 	(void);
void 	PrintTotals 			(void);
void 	DrawLineStuff 			(void);
void 	RunSummaryHots 			(void);
void 	InitScreen 				(void);
void 	InitPrint 				(void);
void 	ProcessFile 			(void);
void 	PrintLinks 				(long, long, long);
void 	PrintAccount 			(int);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	shutdown_prog 			(void);
void 	psl_print 				(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv [])
{
	static  char levelComment [81];
	char    *sptr;

	strcpy (lineString, "-----------------------------|--------------------------|-------|-----------------|-----------------|-----------------");
	strcpy (graphString, "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGG^^");

	OpenDB ();

	strcpy (local_rec.currCode, "");
	strcpy (local_rec.currDisp, "");

	strcpy (accCurrDispFormat, 	ML ("GL Account (%-3.3s)"));
	strcpy (commCurrDispFormat,	ML ("Company Base (%-3.3s)"));

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
        sptr = argv [0];
	else
		sptr++;

	if (strcmp (sptr, "gl_accprt"))
	{
		init_scr ();		/*  sets terminal from termcap	  */
		set_tty ();

		SETUP_SCR (vars);

		set_help (FN6, "FN6");

		if (argc == 2)
        	printerNumber = atoi (argv [1]);
		else
			printerNumber = 1;

		vars [label ("startAccNo")].mask	=	GL_SetAccWidth 
												(
													comm_rec.co_no, 
													TRUE
												);
		vars [label ("endAccNo")].mask = vars [label ("startAccNo")].mask;

		sprintf (levelComment,
                 ML ("Level must be in range (1 to %d) or '0' for all levels."),
                 GV_max_level);
		vars [label ("levelNo")].comment = levelComment;

		set_masks ();		/*  setup print using masks	  */
		swide ();

		do
		{
			strcpy (local_rec.currCode, "");
			strcpy (local_rec.currDisp, "");
			prog_exit	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			init_vars (1);	/*  set default values		  */
			PV_curr_fyear = local_rec.year = fisc_year (comm_rec.gl_date);
			strcpy (local_rec.yearType, "(Current)");
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
				RunSummaryHots ();
			}

        } while (!prog_exit);
	}
	else
	{
		dsp_screen ("General Ledger Summary Account Listing ",
                    comm_rec.co_no,comm_rec.co_name);
		if (argc != 11)
		{
			clear ();
			print_at (0,0, "Usage %s", argv [0]);
			print_at (1,2, "#1 - [F)inancial or N)on-financial]");
			print_at (2,2, "#2 - [Level Number]");
			print_at (3,2, "#3 - [Start Account Number]");
			print_at (4,2, "#4 - [End   Account Number]");
			print_at (5,2, "#5 - [Start Period Number]");
			print_at (6,2, "#6 - [End   Period Number]");
			print_at (7,2, "#7 - [Printer Number.]");
			print_at (8,2, "#8 - [Currency Code.]");
			print_at (9,2, "#9 - [Currency Mode.]");
			sleep (10);

			shutdown_prog ();
            return (EXIT_SUCCESS);
		}

		GL_SetAccWidth (comm_rec.co_no, TRUE);
		PV_print = TRUE;
		strcpy (local_rec.accountType, argv [1]);
		local_rec.accLevel = atoi (argv [2]);
		strcpy (local_rec.startAccNo, argv [3]);
		strcpy (local_rec.endAccNo, argv [4]);
		local_rec.year = atoi (argv [5]);
		local_rec.startPeriod = atoi (argv [6]);
		local_rec.endPeriod = atoi (argv [7]);
		printerNumber = atoi (argv [8]);
		strcpy (local_rec.currCode, argv [9]);
		strcpy (local_rec.currDisp, argv [10]);

		InitPrint ();
		ProcessFile ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
DisplaySummary (
	int		iUnused, 
	KEY_TAB *psUnused)
{
#ifdef GVISION
	destroy_hotbuttons ();
#endif
	InitScreen ();
	ProcessFile ();
	heading (1);
	DisplaySummaryHotKeys ();

    return (EXIT_SUCCESS);
}

int
PrintSummary (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	char	systemExecString [138];

	printerNumber = get_lpno (0);
	sprintf (systemExecString, 
             "gl_accprt \"%s\" %d \"%s\" \"%s\" %d %d %d %d \"%s\" \"%s\"",
             local_rec.accountType, local_rec.accLevel,
             local_rec.startAccNo, local_rec.endAccNo,
             local_rec.year,
             local_rec.startPeriod, local_rec.endPeriod, printerNumber,
             local_rec.currCode, local_rec.currDisp);

	sys_exec (systemExecString);

	heading (1);
	DisplaySummaryHotKeys ();

    return (EXIT_SUCCESS);
}

void
InitScreen (
 void)
{
	/*
	 * Heading and Page Format For Screen Output Display.
	 */
	
	lineDrawn = FALSE;

	Dsp_open (6, 6, InternalPageSize);
	Dsp_saverec ("Account Number               | Narrative                | Class | Opening Balance | Nett Movement   | Closing Balance ");
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCN] [PREV SCN] [INPUT/END] ");
}

void
InitPrint (
 void)
{
	/*
	 * Heading and Page Format For Printer Output Display.
	 */
	lineDrawn = FALSE;
    fout = popen ("pformat", "w");
	if (!fout)
    {
		file_err (errno, "pformat", "POPEN");
    }
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".PL60\n");
	fprintf (fout, ".11\n");
	fprintf (fout, ".L120\n");
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".ESUMMARY ACCOUNT LISTING - %-2.2s  %-40.40s\n",
					comm_rec.co_no, comm_rec.co_name);
	fprintf (fout, ".E Currency as %s\n", local_rec.currDisp);
	fprintf (fout, ".Efor Period %02d/%04d to %02d/%04d\n",
			local_rec.startPeriod, local_rec.year,
			local_rec.endPeriod, local_rec.year);
	fprintf (fout, ".B1\n");

	fprintf (fout, "=====================================================");
	fprintf (fout, "=================================================");
	fprintf (fout, "==================\n");

	fprintf (fout, ".R====================================================");
	fprintf (fout, "=================================================");
	fprintf (fout, "===================\n");

	fprintf (fout, "|ACCOUNT NUMBER               | NARRATIVE            ");
	fprintf (fout, "    | CLASS | OPENING BALANCE | NETT MOVEMENT   |");
	fprintf (fout, " CLOSING BALANCE |\n");

	fprintf (fout, "|-----------------------------|----------------------");
	fprintf (fout, "----|-------|-----------------|-----------------|");
	fprintf (fout, "-----------------|\n");
}

void
ProcessFile (
 void)
{
	int	firstTime = TRUE, len, printed = FALSE;

	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.glmr_class [0], local_rec.accountType);
	SetEndAccount ();
	SetStartAccount ();
	
	cc = find_rec (glmr, &glmrRec, GTEQ , "r");
	len = strlen (GL_GetBit (GV_cur_level));

	openingTotal = monthTotal = carriedTotal = 0.0;

	while (!cc && !strcmp (glmrRec.co_no, comm_rec.co_no) &&
           		  strcmp (glmrRec.acc_no, PV_endAccount) <= 0)
    {
        if (glmrRec.glmr_class [0][0] != local_rec.accountType [0])
		{
			cc = find_rec (glmr, &glmrRec, NEXT , "r");
			continue;
		}
		if (strncmp (glmrRec.curr_code,local_rec.currCode,3))
		{
			cc = find_rec (glmr, &glmrRec, NEXT , "r");
			continue;
		}
		if ((local_rec.accountType [0] == 'F') ||
            ((glmrRec.glmr_class [0][0] == 'N') &&
            (glmrRec.glmr_class [2][0] == 'C')))
		{
			if (PV_print && !localPrint)
            {
				dsp_process ("Account", glmrRec.acc_no);
            }

			if (firstTime)	
            {
                strcpy (previousAccount, glmrRec.acc_no);
            }

			PrintAccount (local_rec.accountType [0] == 'N' ? TRUE : FALSE);
			PrintLinks (glmrRec.hhmr_hash, 0L, 0L);
			printed 	= TRUE;
			firstTime 	= FALSE;

			if (local_rec.accountType [0] == 'N')
				DrawLineStuff ();

 			if (!strcmp (local_rec.startAccNo, local_rec.endAccNo))
				break;
		}
		cc = find_rec (glmr, &glmrRec, NEXT , "r");
	}

	PrintTotals ();

	if (printed && !PV_print)
    {
        Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGG^^");
    }
	else
    {
		if (!PV_print)
		{
	   		for (len = 0; len < 7; len++)
				Dsp_saverec ("        ");

	   		Dsp_saverec (ML (mlGlMess708));
		}
    }
	if (!PV_print)
	{
		Dsp_srch ();
		Dsp_close ();
	}
}

void
PrintLinks (
	long	hhmrHash, 
	long	currentParent, 
	long	currentChild)
{
	int	print_stat = FALSE;

	if (local_rec.accountType [0] != 'N')
		return ;

	gllnRec.parent_hash = hhmrHash;
	gllnRec.child_hash 	= 0L;

    cc = find_rec (glln, &gllnRec, GTEQ,"r");
	while (!cc && gllnRec.parent_hash == hhmrHash)
	{
		if (!print_stat)
		{
			if (!link_level)
				DrawLineStuff ();

			link_level++;
			print_stat = TRUE;
		}
		glmrRec.hhmr_hash	=	gllnRec.child_hash;
        cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
		if (cc)
			file_err (cc, glmr2, "DBFIND");

		PrintAccount (TRUE);
		PrintLinks (glmrRec.hhmr_hash, gllnRec.parent_hash, gllnRec.child_hash);
		cc = find_rec (glln, &gllnRec, NEXT, "r");
	}

	if (print_stat)
		link_level--;

	/*
		If recursing reposition file pointers after read.
	*/
	if (currentChild && currentParent)
	{
		gllnRec.parent_hash	= currentParent;
		gllnRec.child_hash	= currentChild;
        cc = find_rec (glln, &gllnRec, GTEQ, "r");
		if (cc)
            file_err (cc, glln, "DBFIND");
	}
}

void
PrintAccount (
	int		lFlag)
{
	char	accountDesc [4],
			accNo [60],
			totalLine [200],
			formAccNo [FORM_LEN + 1];

	int	    len;
	double	openingBalance	=	0.00,
			monthBalance	=	0.00,
			carriedForward	=	0.00;

	strcpy (formAccNo, glmrRec.acc_no);
	GL_FormAccNo (formAccNo, glmrRec.acc_no, 0);
	/*
	 * Only display up to specified level.
	 */
	if ((!lFlag && GV_cur_level > local_rec.accLevel) ||
        (lFlag && link_level >= local_rec.accLevel))
    {
		return ;
    }

	if (!lFlag)
	{
		len = strlen (GL_GetBit (GV_cur_level));
		if (previousAccount [0] != glmrRec.acc_no [0] && PV_print)
			fprintf (fout, ".PA\n");
		else
        {
			if (strncmp (previousAccount, glmrRec.acc_no, len) ||
                local_rec.accountType [0] == 'N')
                DrawLineStuff ();
        }
		strcpy (previousAccount, glmrRec.acc_no);
	}

	if (ln_num >= InternalPageSize)
		ln_num = ln_num % InternalPageSize;

	sprintf (accountDesc, 
             "%c%c%c",	
             glmrRec.glmr_class [0][0],
             glmrRec.glmr_class [1][0],
             glmrRec.glmr_class [2][0]);

	if (!lFlag)
	{
		sprintf (accNo, 
                 "%-*.*s%s",
                 (GV_cur_level - 1) * 2, 
                 (GV_cur_level - 1) * 2, 
                 " ", 
                 formAccNo);
	}
	else
	{
		len = (link_level > 16) ? 16 : link_level * 2;
		sprintf (accNo, "%-*.*s%s",	len, len, " ", formAccNo);
	}

	if (local_rec.currDisp [0] == 'C')
	{ 
		openingBalance	=	GL_LocTotGlpd 	
							(
								glmrRec.hhmr_hash, 
								PV_budget,
								local_rec.year, 
								1, 
								local_rec.startPeriod - 1
							);
		openingBalance	=	DOLLARS (openingBalance);

		monthBalance	= 	GL_LocTotGlpd 
							(
								glmrRec.hhmr_hash, 
					   			PV_budget,
								local_rec.year,
								local_rec.startPeriod, 
								local_rec.endPeriod
							);
		monthBalance	=	DOLLARS (monthBalance);
	}
	else
	{
		openingBalance	=	GL_FgnTotGlpd 
							(
								glmrRec.hhmr_hash, 
								PV_budget,
								local_rec.year, 
								1, 
								local_rec.startPeriod - 1
							);
		openingBalance	=	DOLLARS (openingBalance);

		monthBalance	=	GL_FgnTotGlpd 
							(
								glmrRec.hhmr_hash, 
								PV_budget,
								local_rec.year,
								local_rec.startPeriod, 
								local_rec.endPeriod
							);
		monthBalance	=	DOLLARS (monthBalance);
	}
	carriedForward = openingBalance + monthBalance;

	if (!PV_print)
	{
		sprintf 
		(
			totalLine,
			"%-29.29s^E %25.25s^E %3.3s   ^E%16.2f ^E%16.2f ^E%16.2f ",
			accNo,
			glmrRec.desc,
			accountDesc,
			openingBalance,
			monthBalance,
			carriedForward
		);
		Dsp_saverec (totalLine);
	}
	else
	{
		sprintf 
		(
			totalLine,
			"%-29.29s| %25.25s| %3.3s   |%16.2f |%16.2f |%16.2f ",
			accNo,
			glmrRec.desc,
			accountDesc,
			openingBalance,
			monthBalance,
			carriedForward
		);
		fprintf (fout, "|%s|\n", totalLine);
	}

	lineDrawn = FALSE;
	ln_num++;

	if ((glmrRec.glmr_class [2][0] == 'P') ||
        (!lFlag && GV_cur_level == local_rec.accLevel) || 
        (lFlag && link_level == GV_cur_level))
	{
		openingTotal 	+= openingBalance;
		monthTotal 		+= monthBalance;
		carriedTotal 	+= carriedForward;
	}
}

void	
PrintTotals (void)
{
	char	totalLine [200];

	if (!PV_print)
	{
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGG^^");
		sprintf 
		(
			totalLine,
			 "%-6.6s (%3.3s)%-52.52s^E%16.2f ^E%16.2f ^E%16.2f ",
			 ML ("TOTALS"),
			 (local_rec.currDisp [0]=='G' ? local_rec.currCode : baseCurr),
			 " ",
			 openingTotal,
			 monthTotal,
			 carriedTotal
		);
		Dsp_saverec (totalLine);
	}
	else
	{
		fprintf (fout, "|----------------------------------------------------------------|-----------------|-----------------|-----------------|\n");
		fprintf 
		(
			fout,
			"|%-6.6s (%3.3s)%-52.52s|%16.2f |%16.2f |%16.2f |\n",
			ML ("TOTALS"),
			(local_rec.currDisp [0]=='G' ? local_rec.currCode : baseCurr),
			" ",
			openingTotal,
			monthTotal,
			carriedTotal
		);
	}
	lineDrawn = FALSE;
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias	(glmr2, glmr);
	abc_alias	(glln2, glln);
	open_rec	(glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec	(glln2, glln_list, GLLN_NO_FIELDS, "glln_id_no2");
	open_rec	(comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");

	OpenGlpd 	();
	OpenGlln 	();
	OpenGlmr 	();

	abc_selfield (glmr, "glmr_id_no2");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);
}

/*
 * Close data base files.
 */
void
CloseDB (
 void)
{
	abc_fclose (glln2);
	abc_fclose (glmr2);
	GL_Close ();
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	if (PV_print)
	{
		fprintf (fout, ".EOF\n");
		fflush (fout);
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
	int		field)
{
	if (LCHECK ("levelNo"))
	{
		if (dflt_used)
        {
			if (local_rec.accountType [0] == 'N')
				local_rec.accLevel = 99;
			else
				local_rec.accLevel = GV_max_level;
        }
		DSP_FLD ("levelNo");
	}

	if (LCHECK ("startAccNo")) 
	{
		if (vars [label ("startAccNo")].mask [0] == '*')
		{
			print_mess (ML (mlGlMess001));
			restart = TRUE;
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.startAccNo, GL_GetDfltSfaccCode ());
		
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

	if (LCHECK ("endAccNo")) 
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
				strcpy (local_rec.endAccNo, GL_GetDfltEfaccCode ());
			else
				strcpy (local_rec.endAccNo, local_rec.startAccNo);
		}
		return (SetEndAccount ());
	}

	if (LCHECK ("start_prd") && dflt_used)
	{
		DateToFinDMY 
		(
			comm_rec.gl_date,
			comm_rec.fiscal,
			NULL, 
			&local_rec.startPeriod, 
			NULL
		);
		DSP_FLD ("start_prd");
	}
	
	if (LCHECK ("end_prd") && dflt_used)
	{
		DateToFinDMY 
		(
			comm_rec.gl_date,
			comm_rec.fiscal,
			NULL, 
			&local_rec.endPeriod, 
			NULL
		);
		DSP_FLD ("end_prd");
	}
	if (LCHECK ("bdg_no"))
	{
		if (getenv ("BUDGET") && dflt_used)
			local_rec.budgetNo = atoi (getenv ("BUDGET"));
        
		PV_budget = local_rec.budgetNo;
	}

	if (LCHECK ("curr"))
	{
		if (dflt_used)
			strcpy (local_rec.currCode, baseCurr);
		
		if (SRCH_KEY)
		{
			SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}
		if (FindPocr (comm_rec.co_no,local_rec.currCode, "r"))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (!strcmp (local_rec.currCode, baseCurr))
        {
			sprintf (local_rec.currDisp, commCurrDispFormat,local_rec.currCode);
        }
		else
        {
			sprintf (local_rec.currDisp, accCurrDispFormat, local_rec.currCode);
        }
		DSP_FLD ("dispCurr");
	}
	return (EXIT_SUCCESS);
}

int	
SetStartAccount (void)
{
	if (GL_FormAccNo (local_rec.startAccNo, glmrRec.acc_no, 0))
    {
		return (EXIT_FAILURE);
    }
	strcpy (PV_startAccount, glmrRec.acc_no);
	
	return (EXIT_SUCCESS);
}

int	
SetEndAccount (void)
{
	if (GL_FormAccNo (local_rec.endAccNo, PV_endAccount, 0))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
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
			scn_set (scn);

		clear ();

		rv_pr (ML (mlGlMess139),114,0,1);
		rv_pr (ML (mlGlMess167),38,0,1);
		box (6,1, 120, 4);

		line_at (21,0,132);
		print_at (22,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		scn_display (scn);
	}
    return (EXIT_SUCCESS);
}


void	
DrawLineStuff (void)
{
	if (!lineDrawn)
	{
		if (!PV_print)
			Dsp_saverec (graphString);
		else
			fprintf (fout, "|%s|\n", lineString);

		ln_num++;
		lineDrawn = TRUE;
	}
}

void	
RunSummaryHots (void)
{
	heading (1);

	restart = FALSE;
	DisplaySummaryHotKeys ();
	if (run_hotkeys (summaryKeys, null_func, null_func))
    {
		restart = TRUE;
    }

	return;
}

void	
DisplaySummaryHotKeys (void)
{
	line_at (21,0,132);
	disp_hotkeys (21, 0, 132, summaryKeys);
}

int
YearFunction (
	int		iUnused, 
	KEY_TAB *psUnused)
{
    int 	localYearAdjustment;

    if (local_rec.year == PV_curr_fyear)
		strcpy (local_rec.yearType, ML ("(Future) "));
	else
		strcpy (local_rec.yearType, ML ("(Current)"));

    if (local_rec.year == PV_curr_fyear)
        localYearAdjustment = 1;
    else if (local_rec.year < PV_curr_fyear)
        localYearAdjustment = (PV_curr_fyear - local_rec.year);
    else
        localYearAdjustment = -1;
    local_rec.year += localYearAdjustment;

	DSP_FLD ("year");
	DSP_FLD ("ytype");
	return (TRUE);
}

int
CurrFunction (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
    if (local_rec.currDisp [0] == 'G')
		sprintf (local_rec.currDisp, commCurrDispFormat, baseCurr);
	else
		sprintf (local_rec.currDisp, accCurrDispFormat, local_rec.currCode);

	DSP_FLD ("dispCurr");
    return (EXIT_SUCCESS);
}

int
HistoryFunction (
	int		iUnused, 
	KEY_TAB *psUnused)
{
	static	int	historyIncr = 0;

	if (!glctRec.history)
	{
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
	DSP_FLD ("ytype");
	return (TRUE);
}

void
psl_print (void)
{
	printerNumber = get_lpno (0);
	rv_pr (ML (mlStdMess035),38,7,1);

	PV_print 	= TRUE;
	localPrint 	= TRUE;
	InitPrint ();
	ProcessFile ();
	fprintf (fout, ".EOF\n");
	pclose (fout);
	localPrint 	= FALSE;
	PV_print 	= FALSE;
}

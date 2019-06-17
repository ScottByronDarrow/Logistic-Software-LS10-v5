/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: batch_list.c,v 5.9 2002/10/07 03:33:55 robert Exp $
|  Program Name  : (gl_batch_list.c)
|  Program Desc  : (GL Transaction Listing By Batch No)
|---------------------------------------------------------------------|
|  Author        : Richard Lau     | Date Written  : 17/03/94         |
|---------------------------------------------------------------------|
| $Log: batch_list.c,v $
| Revision 5.9  2002/10/07 03:33:55  robert
| SC 4294 - fixed button alignment
|
| Revision 5.8  2002/02/04 02:41:25  scott
| Updated to ensure clear_mess used.
|
| Revision 5.7  2001/11/23 04:37:49  scott
| Updated to remove ^A
|
| Revision 5.6  2001/11/22 00:45:48  scott
| Updated from testing
|
| Revision 5.5  2001/08/09 09:13:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.4  2001/08/06 23:27:08  scott
| RELEASE 5.0
|
| Revision 5.3  2001/07/25 02:17:34  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: batch_list.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_batch_list/batch_list.c,v 5.9 2002/10/07 03:33:55 robert Exp $";

/*
 * Include file dependencies 
 */
#define MAXWIDTH	150
#define MAXLINES	100
#define MOD			1   

#include <pslscr.h>
#include <hot_keys.h>
#include <tabdisp.h>
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <pr_format3.h>
#include <GlUtils.h>

/*
 *   Constants, defines and stuff   
 */
#define PAGELINES	65
#define	STANDING	 (!strcmp (glbhRec.jnl_type," 2"))
#define	ALLTYPE		 (!strcmp (local_rec.jnl_type, "ALL"))
#define	WK_DEPTH	14

	char	*data = "data";

extern int tab_max_page;

	/*
	 * Special fields and flags  ##################################.
	 */
    int     GL_BATCH_BRANCH = 1;
  	int 	printerNumber = 0;

	char	*sptr;

	FILE	*fin;
	FILE	*fout;

	char	passedJournalType [3];
	char	mlBatchList [10][101];

	int		dmy [3];
	int		dayApply;
	long	lSystemDate;
	int		day1;
	int		day_app;
	int		firstLine = 0;

#include	"schema"

struct commRecord	comm_rec;
struct glsjRecord	glsj_rec;

/*
 * Local & Screen Structures.
 */
struct 
{
	char	dummy [11];
	char	systemDate [11];
	char	jnl_type [5];
	char	jnl_type_desc [16];
	char	batch_no [sizeof glbhRec.batch_no];
} local_rec;

/*
 *   Local function prototypes  
 */
int ConfirmFunc		(int, KEY_TAB *);
int ConfirmAllFunc 	(int, KEY_TAB *);
int RejectFunc		(int, KEY_TAB *);
int RejectAllFunc	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	tab_keys [] =
{
	{ " Accept ",		'1',	ConfirmFunc,
		"Accept printing of the current batch journal"	},
	{ " Accept all ",		'2',	ConfirmAllFunc,
		"Confirm printing of ALL batch journal"				},
	{ " Cancel ",		'3',	RejectFunc,
		"Cancel printing of the current batch journal"	},
	{ " Cancel all ",		'4',	RejectAllFunc,
		"Cancel printing of ALL batch journal"				},
	END_KEYS
};
#else
static	KEY_TAB	tab_keys [] =
{
	{ " 1=Accept",		'1',		ConfirmFunc,
		"Accept printing of the current batch journal"	},
	{ " 2=Accept All",	'2',	ConfirmAllFunc,
		"Confirm printing of ALL batch journal"				},
	{ " 3=Cancel",		'3',		RejectFunc,
		"Cancel printing of the current batch journal"	},
	{ " 4=Cancel All",	'4',	RejectAllFunc,
		"Cancel printing of ALL batch journal"				},
	END_KEYS
};
#endif

static	struct	var	vars [] =
{
	{1, LIN, "jnl_type", 3, 20, CHARTYPE,
		"AA", "         ",
		" ", "ALL",        "Journal Type    :", " ",
		YES, NO,  JUSTRIGHT, "", "", local_rec.jnl_type},
	{1, LIN, "jnl_type_desc", 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "         ",
		"", "", "", "",
		NA, NO,  JUSTRIGHT, "", "", local_rec.jnl_type_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 *   Local variables  
 */
double  totalCredit,
        totalDebit;

/*
 *   Local function prototypes  
 */
void	PrintStandingDesc	(char	*);
void	PrintExtraDesc 		(char	*);
void 	InitML 			 	(void);
void 	OpenDB 			 	(void);
void 	CloseDB 		 	(void);
void 	PrintBatch 		 	(void);
void 	ProcessGlbh 	 	(void);
void 	ProcessDocument 	(void);
void 	PrintTotal 		 	(void);
void 	HeadingOutput 	 	(void);
void 	SrchGljc 		 	(char *);
void 	shutdown_prog 	 	(void);
int  	heading 		 	(int);
int  	spec_valid 		 	(int);
int  	LoadBatch 		 	(void);
int  	LoadOneType 	 	(char *);
						
/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	if ((argc != 2)&& (argc != 4))
	{
		print_at (0,0,mlGlMess152, argv [0]);
		return (EXIT_SUCCESS);
	}

	printerNumber  	= atoi (argv [1]);
	tab_max_page 	= 1000;

	if (argc == 4)
	{
		strcpy (passedJournalType, argv [2]);
		sprintf (local_rec.batch_no,"%-10.10s",argv [3]);
	}

	sptr = chk_env ("GL_BATCH_BRANCH");
	GL_BATCH_BRANCH = (sptr == (char *)0) ? 1 : atoi (sptr);

	SETUP_SCR (vars);

	/*
	 * Setup required parameters.
	 */
	init_scr ();
	set_tty ();
	set_masks ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lSystemDate = TodaysDate ();

	/*
	 * Read common terminal record.
	 */
	OpenDB ();

	InitML ();

	DateToDMY (comm_rec.gl_date, &dayApply,NULL, NULL);

	if (argc == 2)
	{
		while (prog_exit == 0) 
		{
			entry_exit	= FALSE;
			edit_exit	= FALSE;
			prog_exit	= FALSE;
			restart		= FALSE;
			search_ok	= TRUE;
			init_vars (1);
	
		   	heading (1);
			entry (1);

			if (prog_exit || restart) 
				continue;

			if (LoadBatch ())
			{
				if (!tab_scan ("tabGlbh"))
                {
                    PrintBatch ();
                }
			}
			else
			{
				print_mess (ML (mlStdMess153));
				sleep (sleepTime);
				clear_mess ();
			}
			tab_close ("tabGlbh", TRUE);
		}
	}	
	else
		ProcessGlbh ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
InitML (void)
{
	strcpy (mlBatchList [1], ML ("at"));
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

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (glsj, glsj_list, GLSJ_NO_FIELDS, "glsj_hhbh_hash");
	OpenGlbh ();
    if (!GL_BATCH_BRANCH)	
		abc_selfield (glbh, "glbh_id_no3");

	OpenGlbl ();
	OpenGlmr ();
	OpenGljc ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (glsj);
	abc_dbclose (data);
}

/*
 * Special validation on screen entry.      
 */
int
spec_valid (
 int field)
{
	if (LCHECK ("jnl_type"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.jnl_type_desc, "%15.15s", " ");
			DSP_FLD ("jnl_type_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchGljc (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (gljcRec.co_no, comm_rec.co_no);
		strcpy (gljcRec.journ_type, local_rec.jnl_type);

        cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlGlMess128));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		print_at (8, 35, "   ");
		strcpy (local_rec.jnl_type_desc, gljcRec.jnl_desc);
		DSP_FLD ("jnl_type_desc");
	}

	return (EXIT_SUCCESS);
}

/*
 * Screen Heading Display Routine.        
 */
int
heading (
 int scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	fflush (stdout);
	rv_pr (ML (mlGlMess106), 24, 0, 1);

	fflush (stdout);
	line_at (1,0,80);
	box (0,2,80,1);

	line_at (21,0,80);
	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
LoadBatch (void)
{
	int dataFound = FALSE;

	tab_open ("tabGlbh", tab_keys, 2, 8, WK_DEPTH, FALSE);
	tab_add ("tabGlbh", 
			"#%3.3s %9.9s %-18.18s %-12.12s %-5.5s %4.4s %-8.8s ",
			"C/R", "Batch No.", "Journal Type", "Date", "Time", "Per.", "User");

	if (!strcmp (local_rec.jnl_type, "ALL"))
	{
		sprintf (gljcRec.co_no, comm_rec.co_no);
		strcpy (gljcRec.journ_type, "");

        cc = find_rec (gljc, &gljcRec, GTEQ, "r");
		while (!cc && !strcmp (gljcRec.co_no, comm_rec.co_no))
		{
			if (LoadOneType (gljcRec.journ_type))
				dataFound = TRUE;

			cc = find_rec (gljc, &gljcRec, NEXT, "r");
		}
	}
	else
	{
		if (LoadOneType (local_rec.jnl_type))
			dataFound = TRUE;
	}

	return (dataFound);
}

int
LoadOneType (
	char *journal_type)
{
	int		lposted_dmy [3];
	int		foundData = FALSE;
	int		displayMonth;

	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	strcpy (glbhRec.jnl_type, journal_type);
	sprintf (glbhRec.batch_no, "%10.10s", " ");
	cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	while (	!cc && 
			!strcmp (glbhRec.co_no, comm_rec.co_no) &&
			!strcmp (glbhRec.jnl_type, journal_type))
	{
		if (glbhRec.stat_flag [0] == 'P' && !STANDING)
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}

		if (STANDING)
		{
			cc = find_hash (glsj,&glsj_rec,COMPARISON,"r",glbhRec.hhbh_hash);
			day_app = glsj_rec.dt_apply;

			DateToDMY (glsj_rec.from_dt, &dmy [0], &dmy [1],&dmy [2]);
			day1 = dmy [0];
			if (day1 > 28)
				day1 = 28;
			if (day_app < day1)
				glsj_rec.from_dt += 28;

			DateToDMY (glsj_rec.to_dt, &dmy [0], &dmy [1],&dmy [2]);
			day1 = dmy [0];
			if (day1 > 28)
				day1 = 28;
			if (day_app > day1)
				glsj_rec.to_dt -= 28;

			DateToDMY (glsj_rec.dt_posted, &dmy [0], &dmy [1],&dmy [2]);

			if (comm_rec.gl_date < glsj_rec.from_dt ||
				comm_rec.gl_date > glsj_rec.to_dt ||
				day_app > dayApply)
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}

			DateToDMY 
			 (
				glsj_rec.dt_posted, 
				&lposted_dmy [0], 
				&lposted_dmy [1],
				&lposted_dmy [2]
			);
			DateToDMY (comm_rec.gl_date, &dmy [0], &dmy [1],&dmy [2]);
			if (lposted_dmy [1] == dmy [1] && 
				lposted_dmy [2] == dmy [2])
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}
		}
		DateToDMY (comm_rec.gl_date, &dmy [0], &dmy [1],&dmy [2]);
		displayMonth = (STANDING) ? 
						mth2fisc (dmy [1], comm_rec.fiscal): 
						mth2fisc (glbhRec.mth, comm_rec.fiscal); 

		tab_add ("tabGlbh", 
		"%-3.3s %-10.10s %2.2s %-15.15s %-12.12s %-5.5s %2.2d   %-8.8s",
			"C", 							/* 0 */
			glbhRec.batch_no, 				/* 4 */
			glbhRec.jnl_type, 				/* 15 */
			gljcRec.jnl_desc,			
			DateToString (glbhRec.glbh_date), 
			glbhRec.glbh_time, 		
			displayMonth, 
			glbhRec.user);

		foundData = TRUE;
		cc = find_rec (glbh, &glbhRec, NEXT, "r");
	}
	return (foundData);
}

int
ConfirmFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	rec_buffer [256];
	int		old_line;


	old_line = tab_tline ("tabGlbh");
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer [0] = 'A';
		tab_update ("tabGlbh", "%s", rec_buffer);

        cc = tab_get ("tabGlbh", rec_buffer, NEXT, 0);
		if (cc)
        {
			cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
        }
	}

	old_line = tab_tline ("tabGlbh");
	if ((old_line % WK_DEPTH) == 0)
		load_page ("tabGlbh", FALSE);

	redraw_page ("tabGlbh", TRUE);

    return (EXIT_SUCCESS);
}

int
ConfirmAllFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	rec_buffer [256];
	int		old_line;

    old_line = tab_tline ("tabGlbh");
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, firstLine);
	while (!cc)
	{
		rec_buffer [0] = 'A';
		tab_update ("tabGlbh", "%s", rec_buffer);
		cc = tab_get ("tabGlbh", rec_buffer, NEXT, 0);
	}
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
	load_page ("tabGlbh", FALSE);
	redraw_page ("tabGlbh", TRUE);

    return (EXIT_SUCCESS);
}

int
RejectFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	rec_buffer [256];
	int		old_line;


	old_line = tab_tline ("tabGlbh");
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer [0] = 'C';
		tab_update ("tabGlbh", "%s", rec_buffer);

        cc = tab_get ("tabGlbh", rec_buffer, NEXT, 0);
		if (cc)
        {
			cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
        }
	}
	old_line = tab_tline ("tabGlbh");
	if ((old_line % WK_DEPTH) == 0)
		load_page ("tabGlbh", FALSE);
    
	redraw_page ("tabGlbh", TRUE);

    return (EXIT_SUCCESS);
}

int
RejectAllFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	rec_buffer [256];
	int		old_line;


	old_line = tab_tline ("tabGlbh");
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, firstLine);
	while (!cc)
	{
		rec_buffer [0] = 'C';
		tab_update ("tabGlbh", "%s", rec_buffer);
		cc = tab_get ("tabGlbh", rec_buffer, NEXT, 0);
	}
	cc = tab_get ("tabGlbh", rec_buffer, EQUAL, old_line);
	load_page ("tabGlbh", FALSE);
	redraw_page ("tabGlbh", TRUE);

    return (EXIT_SUCCESS);
}
void
PrintBatch (void)
{
	char	rec_buffer [256];

	if (tab_get ("tabGlbh", rec_buffer, FIRST, 0))
		return;

	while (TRUE) 
	{
		if (rec_buffer [0] != 'A')
		{
			if (tab_get ("tabGlbh", rec_buffer, NEXT, 0))
				break;

			continue;
		}

		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.br_no, comm_rec.est_no);
		sprintf (glbhRec.jnl_type, "%-2.2s", rec_buffer + 15);
		sprintf (glbhRec.batch_no, "%-10.10s", rec_buffer + 4);
	
		cc = find_rec (glbh, &glbhRec, COMPARISON, "r");
		if (cc)
		{
			if (tab_get ("tabGlbh", rec_buffer, NEXT, 0))
				return;
            else
                continue;
		}

		strcpy (local_rec.batch_no, glbhRec.batch_no);
		strcpy (passedJournalType, glbhRec.jnl_type);
		ProcessGlbh ();
	
	    if (tab_get ("tabGlbh", rec_buffer, NEXT, 0))
			break;
	}
}

/*
 * Process Master File. 
 */
void
ProcessGlbh (void)
{
	/*
	 * Process All Stock Item. 
	 */
	strcpy (glbhRec.co_no,comm_rec.co_no);
	strcpy (glbhRec.br_no,comm_rec.est_no);
	strcpy (glbhRec.jnl_type, passedJournalType);
	strcpy (glbhRec.batch_no, local_rec.batch_no);

	cc = find_rec (glbh, &glbhRec, COMPARISON, "r");
	if (cc)
    {
		return;
    }

	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, glbhRec.jnl_type);
    cc = find_rec (gljc, &gljcRec, COMPARISON, "r");

	sprintf (err_str, "%s %s", ML ("Printing Batch"), glbhRec.batch_no);
	print_mess (err_str);

	/*
	 * Print heading
	 */
	HeadingOutput ();

	totalDebit = 0.0;
	totalCredit = 0.0;
	glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
	glblRec.line_no 	= 0;

    cc = find_rec (glbl, &glblRec, GTEQ, "r");
	while ((!cc) && 
           (glblRec.hhbh_hash == glbhRec.hhbh_hash))
	{                        
		/*
		 * Skip Invalid Account. (Comment line)
		 */
		sprintf (glmrRec.co_no, "%-2.2s", comm_rec.co_no);
		sprintf (glmrRec.acc_no, "%-17.17s", glblRec.acc_no);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (glbl, &glblRec, NEXT, "r");
			continue;
		}

		ProcessDocument ();
		/*
		 * Even is credit and Odd is debit
		 */
		if (atoi (glblRec.dc_flag) % 2 != 0)
			totalDebit	+= DOLLARS (glblRec.local_amt);
		else
			totalCredit += DOLLARS (glblRec.local_amt);

		cc = find_rec (glbl, &glblRec, NEXT, "r");
	}

	/*
	 * Print Batch Totals.                   
	 */
	PrintTotal ();
	fprintf (fout, ".EOF\n");
	pclose (fout);
	return;
}

/*
 * Process Line.
 */
void
ProcessDocument (void)
{
	int		monthPeriod;

	/*
	 * Even is credit and Odd is debit
	 */
	if (atoi (glblRec.dc_flag) % 2 != 0)
	{
		if (!STANDING)
		{
   			fprintf (fout,"|%-10.10s",DateToString (glblRec.tran_date));
		    fprintf (fout,"| %-16.16s ",glblRec.acc_no);
			fprintf (fout,"| %-25.25s ",glmrRec.desc);
			fprintf (fout,"|%-8.8s",glblRec.user_ref);
			fprintf (fout,"|%14.2f ",DOLLARS (glblRec.local_amt));
			fprintf (fout,"|%-15.15s"," ");
			fprintf (fout,"|%-20.20s",glblRec.narrative);
			fprintf (fout,"|%-3.3s ",glblRec.currency);
			fprintf (fout,"| %13.2f ",DOLLARS (glblRec.fx_amt));
			fprintf (fout,"| %12.8f",glblRec.exch_rate);
			fprintf (fout,"| %2.2d |\n",mth2fisc (atoi (glblRec.period_no), comm_rec.fiscal));
			if (strlen (clip (glblRec.alt_desc1)))
				PrintStandingDesc (glblRec.alt_desc1);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintStandingDesc (glblRec.alt_desc2);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintStandingDesc (glblRec.alt_desc3);
		
		}
		else
		{
			fprintf (fout,"| %-16.16s ",glblRec.acc_no);
			fprintf (fout,"| %25.25s ", glmrRec.desc);
			fprintf (fout,"|%-8.8s",	glblRec.user_ref);
			fprintf (fout,"|%14.2f ",	DOLLARS (glblRec.local_amt));
			fprintf (fout,"|%-15.15s"," ");
			fprintf (fout,"|%-20.20s",glblRec.narrative);
			fprintf (fout,"|%-3.3s ",	glblRec.currency);
			fprintf (fout,"| %13.2f ",	DOLLARS (glblRec.fx_amt));
			fprintf (fout,"| %12.8f",	glblRec.exch_rate);

			DateToDMY (comm_rec.gl_date, NULL, &monthPeriod, NULL);
			fprintf (fout,"| %2.2d |\n",mth2fisc (monthPeriod,comm_rec.fiscal));
			if (strlen (clip (glblRec.alt_desc1)))
				PrintExtraDesc (glblRec.alt_desc1);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintExtraDesc (glblRec.alt_desc2);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintExtraDesc (glblRec.alt_desc3);
		}
	}
	else
	{
		if (!STANDING)
		{
			fprintf (fout,"|%-10.10s",DateToString (glblRec.tran_date));
			fprintf (fout,"| %-16.16s ",glblRec.acc_no);
			fprintf (fout,"| %-25.25s ",glmrRec.desc);
			fprintf (fout,"|%-8.8s",glblRec.user_ref);
			fprintf (fout,"|%-15.15s"," ");
			fprintf (fout,"|%14.2f ",DOLLARS (glblRec.local_amt));
			fprintf (fout,"|%-20.20s",glblRec.narrative);
			fprintf (fout,"|%-3.3s ",glblRec.currency);
			fprintf (fout,"|(%13.2f)",DOLLARS (glblRec.fx_amt));
			fprintf (fout,"| %12.8f",glblRec.exch_rate);
			fprintf (fout,"| %2.2d |\n",mth2fisc (atoi (glblRec.period_no), comm_rec.fiscal));
			if (strlen (clip (glblRec.alt_desc1)))
				PrintStandingDesc (glblRec.alt_desc1);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintStandingDesc (glblRec.alt_desc2);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintStandingDesc (glblRec.alt_desc3);
		}
		else
		{
			fprintf (fout,"| %-16.16s ",glblRec.acc_no);
			fprintf (fout,"| %-25.25s ",glmrRec.desc);
			fprintf (fout,"|%-8.8s",glblRec.user_ref);
			fprintf (fout,"|%-15.15s"," ");
			fprintf (fout,"|%14.2f ",DOLLARS (glblRec.local_amt));
			fprintf (fout,"|%-20.20s",glblRec.narrative);
			fprintf (fout,"|%-3.3s ",glblRec.currency);
			fprintf (fout,"|(%13.2f)",DOLLARS (glblRec.fx_amt));
			fprintf (fout,"| %12.8f",glblRec.exch_rate);
			DateToDMY (comm_rec.gl_date, NULL, &monthPeriod, NULL);
			fprintf (fout,"| %2.2d |\n",mth2fisc (monthPeriod,comm_rec.fiscal));

			if (strlen (clip (glblRec.alt_desc1)))
				PrintExtraDesc (glblRec.alt_desc1);

			if (strlen (clip (glblRec.alt_desc2)))
				PrintExtraDesc (glblRec.alt_desc2);

			if (strlen (clip (glblRec.alt_desc3)))
				PrintExtraDesc (glblRec.alt_desc3);
		}
	}
	return;
}

void
PrintStandingDesc (
	char	*extraDesc)
{
	fprintf (fout,"|%10.10s", 	" ");
	fprintf (fout,"|%18.18s", 	" ");
	fprintf (fout,"|%27.27s", 	" ");
	fprintf (fout,"|%8.8s",		" ");
	fprintf (fout,"|%15.15s", 	" ");
	fprintf (fout,"|%15.15s",	" ");
	fprintf (fout,"|%-20.20s",	extraDesc);
	fprintf (fout,"|%4.4s",		" ");
	fprintf (fout,"|%15.15s",	" ");
	fprintf (fout,"|%13.13s",	" ");
	fprintf (fout,"|%4.4s|\n",	" ");
}
void
PrintExtraDesc (
	char	*extraDesc)
{
	fprintf (fout,"|%18.18s",	" ");
	fprintf (fout,"|%27.27s",	" ");
	fprintf (fout,"|%8.8s",		" ");
	fprintf (fout,"|%15.15s",	" ");
	fprintf (fout,"|%15.15s",	" ");
	fprintf (fout,"|%-20.20s",extraDesc);
	fprintf (fout,"|%4.4s",		" ");
	fprintf (fout,"|%15.15s",	" ");
	fprintf (fout,"|%13.13s",	" ");
	fprintf (fout,"|%4.4s|\n",	" ");
}

/*
 * Page Count Check.   
 */
int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*
 * Print Grand Totals.              
 */
void
PrintTotal (void)
{
	/*
	 * Print Detailed Total Line.   
	 */
	if (!STANDING)
	{
		fprintf (fout,"|---------------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
		 fprintf (fout,"|%-66.66s| %13.2f | %13.2f |%-60.60s|\n"," ",totalDebit, totalCredit," ");
	}          
	else
	{
		fprintf (fout,"|---------------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
		fprintf (fout,"|%-66.66s| %13.2f | %13.2f |%-60.60s|\n"," ", totalDebit, totalCredit," ");
	}
}

/*
 * Start Out Put To Standard Print.
 */
void
HeadingOutput (void)
{
	/*
	 * Open format file
	 */
	if ((fin = pr_open ("gl_batch_list.p")) == NULL)
		sys_err ("Error in opening gl_batch_list.p during (FOPEN)",errno,PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".PI12\n");
//	fprintf (fout, ".L141\n");
	fprintf (fout, ".L163\n");
	fprintf (fout, ".PL64\n");
	fprintf (fout, ".10\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));

	fprintf (fout, ".EGL TRANSACTION LISTING BY BATCH NO\n");           
	sprintf (err_str, "<%s>", clip (gljcRec.jnl_desc));                 
	fprintf (fout, ".EBATCH NO %s %s\n", local_rec.batch_no, err_str);  

	fprintf (fout, ".EINPUT BY %s AT %s %s\n\n", glbhRec.user, DateToString (glbhRec.glbh_date), glbhRec.glbh_time);
	 if (!STANDING)
	 {
		 fprintf (fout, "=================================================================================================================================================================\n");
		 fprintf (fout, "|   DATE   |  ACCOUNT NUMBER  |     ACCT. DESCRIPTION     |  REF   | DEBIT  AMOUNT | CREDIT AMOUNT |      NARRATIVE     |CURR|   FGN AMOUNT  |EXCHANGE RATE|PER.|\n");
		 fprintf (fout, "=================================================================================================================================================================\n");
		 fprintf (fout,".R=================================================================================================================================================================\n");
	}
	else
	{
		fprintf (fout,"======================================================================================================================================================\n");
		fprintf (fout,"|  ACCOUNT NUMBER  |     ACCT. DESCRIPTION     |  REF   | DEBIT AMOUNT  | CREDIT AMOUNT |      NARRATIVE     |CURR|   FGN AMOUNT  |EXCHANGE RATE|PER.|\n");
		fprintf (fout,"======================================================================================================================================================\n");
		fprintf (fout,".R======================================================================================================================================================\n");
	}
	fprintf (fout, ".PI12\n");
	fflush (fout);
}
void
SrchGljc (
	char *keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Journal Description");
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, keyValue);
	
    cc = find_rec (gljc, &gljcRec, GTEQ, "r");
	while (!cc && 
           !strcmp (gljcRec.co_no, comm_rec.co_no) &&
           !strncmp (gljcRec.journ_type, keyValue, strlen (keyValue)))
	{
		cc = save_rec (gljcRec.journ_type, gljcRec.jnl_desc);
		if (cc)
        {
			break;
        }
		cc = find_rec (gljc,&gljcRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (local_rec.jnl_type, temp_str);
}

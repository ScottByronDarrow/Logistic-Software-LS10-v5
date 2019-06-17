/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_postbat.c,v 5.8 2001/12/18 02:08:18 robert Exp $
|  Program Name  : (gl_postbat.c  )                                |
|  Program Desc  : (Post General Ledger by Batch No.           )   |
|---------------------------------------------------------------------|
| $Log: gl_postbat.c,v $
| Revision 5.8  2001/12/18 02:08:18  robert
| Updated change incorrect function name wkConfirmAll to WkConfirmAll in LS10-GUI
|
| Revision 5.7  2001/11/23 04:43:05  scott
| Issues List Reference Number = 00033
| Program Description = GLJL5-Batch Journal Posting
| Status = Open
| Classification = Error
| Priority = Low
| ReOpen / Return Frequency = 00
| Worksheet = Errors
|
| Revision 5.6  2001/10/17 07:37:14  robert
| Updated to fix synchronous problem with execvp in LS10-GUI
|
| Revision 5.5  2001/10/09 08:34:46  robert
| Updated ExcutedCommand for LS10-GUI
|
| Revision 5.4  2001/08/09 09:13:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/06 23:27:28  scott
| RELEASE 5.0
|
| Revision 5.2  2001/07/25 02:17:55  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_postbat.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_postbat/gl_postbat.c,v 5.8 2001/12/18 02:08:18 robert Exp $";

#define TOTSCNS		1	/*  the number of screens used (incl lin.)*/
#define MAXSCNS		1	/*  the max. number of screens allowed    */
#define	CODE	Journal [indx].lbl

#include <pslscr.h>	
#include <hot_keys.h>
#include <GlUtils.h>	
#include <twodec.h>	
#include <tabdisp.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

#define		STANDING	(!strcmp (glbhRec.jnl_type," 2"))
#define		ALLTYPE		(!strcmp (local_rec.journalNo, "ALL"))
#define		WK_DEPTH	14

#define		HEADER_POSTED	(glbhRec.stat_flag [0] == 'P')
#define		DETAIL_POSTED	(glblRec.stat_flag [0] == 'P')

#include	"schema"

extern		int		tab_max_page;

#ifdef GVISION
extern 		bool	gbSysExec;
#endif

struct commRecord	comm_rec;
struct glsjRecord	glsj_rec;

/*
 *	Initialise structure type 'JnlStruct' and set label members, 
 *	value members are assigned later passed passed to the Substit () function 
 *	for variable expansion.                      |
 */
struct	JnlStruct {
	char 	*lbl;
	char 	*val;
} Journal [] = {
	{"TYPE",0},
	{"LPNO",0},
	{"PID",0},
	{"",""}
};
	char	*data		= "data",
			*gl_postbat	= "gl_postbat";

	int		ProcessID			= 0,	
			glwkNo				= 0,
			envPriorPost 		= 0,
			envGlBatchBranch 	= 1,
			printerNo			= 1,
			dmy [3]				= {0,0,0},
			moduleDmy [3]		= {0,0,0},
			systemDmy [3]		= {0,0,0},
			testDay				= 0,
			dayApplied			= 0,
			first_line 			= 0,
			BatchError 			= FALSE,
			workFileOpen		= FALSE;
	
	char *	args [20];

	long  	lSystemDate		= 0L,
			lModuleDate		= 0L;

	char  	reportPara [10][41],
			runCommand [256],
			printerString [5],
			processString [11];

static int wkConfirm	(int, KEY_TAB *);
static int WkConfirmAll (int, KEY_TAB *);
static int WkReject		(int, KEY_TAB *);
static int WkRejectAll	(int, KEY_TAB *);
static int WkView		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	tab_keys [] =
{
	{ " ACCEPT ",		'1',	wkConfirm,
		"Accept posting of the current batch journal"	},
	{ " ALL ACCEPT ",	'2',	WkConfirmAll,
		"Confirm posting ALL batch journal"				},
	{ " CANCEL ",		'3',	WkReject,
		"Cancel posting of the current batch journal"	},
	{ " ALL CANCEL ",	'4',	WkRejectAll,
		"Cancel posting ALL batch journal"				},
	{ " VIEW BATCH ", 	'V',		WkView,
		"View the batch"								},
	END_KEYS
};
#else
static	KEY_TAB	tab_keys [] =
{
	{ "1=Accept",		'1',		wkConfirm,
		"Accept posting of the current batch journal"	},
	{ "2=Accept All",	'2',	WkConfirmAll,
		"Confirm posting ALL batch journal"				},
	{ "3=Cancel",		'3',		WkReject,
		"Cancel posting of the current batch journal"	},
	{ "4=Cancel All",	'4',	WkRejectAll,
		"Cancel posting ALL batch journal"				},
	{ "5=View batch", 	'5',		WkView,
		"View the batch"								},
	END_KEYS
};
#endif

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	systemDate [11];
	char	journalNo [5];
	char	journalDesc [16];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "journalNo", 3, 20, CHARTYPE,
		"AA", "         ",
		" ", "ALL",        "Journal Type    :", " ",
		YES, NO,  JUSTRIGHT, "", "", local_rec.journalNo},
	{1, LIN, "journalDesc", 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "         ",
		"", "", "", "",
		NA, NO,  JUSTRIGHT, "", "", local_rec.journalDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
int		ExcuteCommand			(void);
int 	NotBlank 				(char *);
int 	LoadBatch 				(void);
int 	UpdateToGl 				(char *);
int 	heading 				(int);
int 	spec_valid 				(int);
void 	CloseDB 				(void);
void 	LoadOneJournalType 		(char *);
void 	OpenDB 					(void);
void 	SrchGljc 				(char *);
void 	UpdateBatch 			(void);
void 	UpdateOneJournalType 	(char *);
void 	shutdown_prog 			(void);
void	OpenGlWork 				(void);
void	CloseGlWork 			(void);
void	ProcessLine				(char *);
char 	*Substitute				(char *);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	if (argc > 4 || argc < 3)
	{
		print_at (0,0,mlGlMess702, argv [0]);
        return (EXIT_FAILURE);
	}

	ProcessID	= atoi (argv [1]);
	printerNo 	= atoi (argv [2]);

	sprintf (printerString, "%d", printerNo);
	sprintf (processString, "%d", ProcessID);

	tab_max_page 	= 1000;

	sptr = chk_env ("PRIOR_POST");
	envPriorPost	=	(sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("GL_BATCH_BRANCH");
	envGlBatchBranch = (sptr == (char *)0) ? 1 : atoi (sptr);

	OpenDB ();
	GL_SetAccWidth (comm_rec.co_no, TRUE);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lSystemDate = TodaysDate ();
	DateToDMY (lSystemDate,&systemDmy [0],&systemDmy [1],&systemDmy [2]);

   	/*
   	 * Setup required parameters.
     */
    SETUP_SCR (vars);

    init_scr ();
    set_tty ();
    set_masks ();

    init_vars (1);

	if (argc == 4)
	{
		UpdateOneJournalType (argv [3]);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	entry_exit = edit_exit = prog_exit = restart = 0;
	init_vars (1);
	search_ok	=	TRUE;

	/*
   	 * Enter screen 1 linear input.
	 */
   	heading (1);
	entry (1);

	if (!LoadBatch ())
	{
		if (!tab_scan (gl_postbat))
	   		UpdateBatch ();
	}
	tab_close (gl_postbat, TRUE);

	/*
	 * Program exit sequence.
	 */
	shutdown_prog ();
    return (EXIT_SUCCESS);
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
	DateToDMY 
	(
		comm_rec.gl_date, 
		&moduleDmy [0], 
		&moduleDmy [1],
		&moduleDmy [2]
	);

	OpenGlmr ();
	OpenGljc ();
	OpenGlbl (); 
	OpenGlbh (); 
	if (!envGlBatchBranch)
		abc_selfield (glbh, "glbh_id_no3");

	open_rec (glsj, glsj_list, GLSJ_NO_FIELDS, "glsj_hhbh_hash");
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

int
spec_valid (
	int	field)
{
	if (LCHECK ("journalNo"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.journalDesc, "%15.15s", " ");
			DSP_FLD ("journalDesc");
        	return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchGljc (temp_str);
        	return (EXIT_SUCCESS);
		}
		strcpy (gljcRec.co_no, comm_rec.co_no);
		strcpy (gljcRec.journ_type, local_rec.journalNo);
		cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess128));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		print_at (8, 35, "   ");
		strcpy (local_rec.journalDesc, gljcRec.jnl_desc);
		DSP_FLD ("journalDesc");
	}
	return (EXIT_SUCCESS);
}

/*
 * UpdateOneJournalType  will post all the avaiable batch   
 * for that particular journal type                        
 */
void
UpdateOneJournalType (
	char	*journalType)
{
	int		posted 			= FALSE,
			lastPostDmy [3]	= {0,0,0};

	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	strcpy (glbhRec.jnl_type, journalType);
	sprintf (glbhRec.batch_no, "%10.10s", " ");

	cc = find_rec (glbh, &glbhRec, GTEQ, "u");
	while (	!cc && 
			!strcmp (glbhRec.co_no, comm_rec.co_no) &&
			!strcmp (glbhRec.jnl_type, journalType))
	{
		if (HEADER_POSTED && !STANDING)
		{
			abc_unlock (glbh);
			cc = find_rec (glbh, &glbhRec, NEXT, "u");
			continue;
		}

		if (STANDING)
		{
			glsj_rec.hhbh_hash	=	glbhRec.hhbh_hash;
			cc = find_rec (glsj, &glsj_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (glbh);
				cc = find_rec (glbh, &glbhRec, NEXT, "u");
				continue;
			}
			dayApplied = glsj_rec.dt_apply;

			DateToDMY (glsj_rec.from_dt, &dmy [0],&dmy [1],&dmy [2]);

			testDay = dmy [0];
			if (testDay > 28)
				testDay = 28;
			if (dayApplied < testDay)
				glsj_rec.from_dt += 28;

			DateToDMY (glsj_rec.to_dt, &dmy [0],&dmy [1],&dmy [2]);
			testDay = dmy [0];
			if (testDay > 28)
				testDay = 28;
			if (dayApplied > testDay)
				glsj_rec.to_dt -= 28;

			DateToDMY (glsj_rec.dt_posted, &dmy [0],&dmy [1],&dmy [2]);

			if (comm_rec.gl_date < glsj_rec.from_dt ||
				comm_rec.gl_date > glsj_rec.to_dt ||
				dayApplied > moduleDmy [0])
			{
				abc_unlock (glbh);
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}

			DateToDMY 
			(
				glsj_rec.dt_posted, 
				&lastPostDmy [0],
				&lastPostDmy [1],
				&lastPostDmy [2]
			);
			DateToDMY (comm_rec.gl_date, &dmy [0],&dmy [1],&dmy [2]);
			if (lastPostDmy [1] == dmy [1] && lastPostDmy [2] == dmy [2])
			{
				abc_unlock (glbh);
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}
		}

		glblRec.hhbh_hash = glbhRec.hhbh_hash;
		glblRec.line_no   = 0;
		cc = find_rec (glbl, &glblRec, GTEQ, "u");
		while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
		{
			if (DETAIL_POSTED)
			{
				abc_unlock (glbl);
				cc = find_rec (glbl, &glblRec, NEXT, "u");
				continue;
			}
			/*
			 * Open Work file if not already done. 
			 */
			OpenGlWork ();

			strcpy (glwkRec.acc_no, glblRec.acc_no);
			strcpy (glwkRec.co_no,  comm_rec.co_no);
			strcpy (glwkRec.est_no, comm_rec.est_no);
			strcpy (glwkRec.acronym, glblRec.acronym);
			strcpy (glwkRec.name, glblRec.name);
			strcpy (glwkRec.chq_inv_no, glblRec.chq_inv_no);
			glwkRec.ci_amt = glblRec.ci_amt;
			glwkRec.o1_amt = glblRec.o1_amt;
			glwkRec.o2_amt = glblRec.o2_amt;
			glwkRec.o3_amt = glblRec.o3_amt;
			glwkRec.o4_amt = glblRec.o4_amt;
			glwkRec.hhgl_hash = glblRec.hhgl_hash;
			strcpy (glwkRec.tran_type, glblRec.tran_type);
			strcpy (glwkRec.sys_ref, glblRec.sys_ref);
			if (STANDING)
			{
				glwkRec.tran_date = comm_rec.gl_date;
				sprintf (glwkRec.period_no, "%02d", moduleDmy [1]);
			}
			else
			{
				glwkRec.tran_date = glblRec.tran_date;
				sprintf (glwkRec.period_no, "%02d", glbhRec.mth);
			}
			glwkRec.post_date = TodaysDate ();
			strcpy (glwkRec.narrative, glblRec.narrative);
			strcpy (glwkRec.alt_desc1, glblRec.alt_desc1);
			strcpy (glwkRec.alt_desc2, glblRec.alt_desc2);
			strcpy (glwkRec.alt_desc3, glblRec.alt_desc3);
			strcpy (glwkRec.batch_no,  glblRec.batch_no);
			strcpy (glwkRec.user_ref,  glblRec.user_ref);
			strcpy (glwkRec.jnl_type,  glblRec.dc_flag);
			strcpy (glwkRec.currency,  glblRec.currency);
			glwkRec.amount 		= glblRec.fx_amt;
			glwkRec.loc_amount 	= glblRec.local_amt;
			glwkRec.exch_rate 	= glblRec.exch_rate;
			strcpy (glwkRec.stat_flag, "2");
			strcpy (glwkRec.run_no, glbhRec.batch_no);

			cc = RF_ADD (glwkNo, (char *) &glwkRec);
			if (cc) 
				break;

			posted = TRUE;
			if (!STANDING)
			{
				strcpy (glblRec.stat_flag,"P");
				cc = abc_update (glbl, &glblRec);
				if (cc)
					break;
			}
			cc = find_rec (glbl, &glblRec, NEXT, "u");
		} 
		abc_unlock (glbl);
		if (STANDING && posted)
		{
			glsj_rec.dt_posted = comm_rec.gl_date;
			cc = abc_update (glsj, &glsj_rec);
			if (cc)
				file_err (cc, glsj, "DBUPDATE");
		}
		posted = FALSE;
		if (!STANDING)
			strcpy (glbhRec.stat_flag, "P");

		glbhRec.glbh_date = TodaysDate ();
		strcpy (glbhRec.glbh_time, TimeHHMM ());
		cc = abc_update (glbh, &glbhRec);
		if (cc)
			file_err (cc, glbh, "DBUPDATE");

		/*
	 	 * Close Work file if not already done. 
	 	 */
		if (workFileOpen)
		{
			CloseGlWork ();
			Journal [0].val = strdup (journalType);
			Journal [1].val = strdup (printerString);
			Journal [2].val = strdup (processString);
			cc = UpdateToGl (journalType);
			if (cc && !STANDING)
			{
				print_mess (ML ("Error in Posting, Reversing transactions"));
				sleep (sleepTime);
				/*
				 * Reverse out postings.
				 */
				cc = find_rec (glbh, &glbhRec, COMPARISON, "u");
				if (cc)
					file_err (cc, glbh, "DBFIND");

				glblRec.hhbh_hash = glbhRec.hhbh_hash;
				glblRec.line_no   = 0;
				cc = find_rec (glbl, &glblRec, GTEQ, "u");
				while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
				{
					if (DETAIL_POSTED)
					{
						strcpy (glblRec.stat_flag,"N");
						cc = abc_update (glbl, &glblRec);
						if (cc)
							file_err (cc, glbl, "DBUPDATE");
					}
					else
						abc_unlock (glbl);

					cc = find_rec (glbl, &glblRec, NEXT, "u");
				}
				strcpy (glbhRec.stat_flag, "N");
				cc = abc_update (glbh, &glbhRec);
				if (cc)
					file_err (cc, glbh, "DBUPDATE");

				print_mess (ML ("Reversing of transactions complete"));
				sleep (sleepTime);
			}
		}
		cc = find_rec (glbh, &glbhRec, NEXT, "u");
	}
	abc_unlock (glbh);
}

int
heading (
	int		scn)
{
	int	s_size = 81;

	if (restart) 
		return (EXIT_SUCCESS);

	scn_set (scn);
	clear ();

	rv_pr (ML (mlGlMess134),28,0,1);

	box (0,2,79,1);
	line_at (1,0, s_size);
	line_at (20,0,s_size);
	line_at (22,0,s_size);

	print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

	line_cnt = 0;
	scn_write (scn);
	
	return (EXIT_SUCCESS);
}

void
SrchGljc (
	char	*searchValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Journal number description");
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, searchValue);
	cc = find_rec (gljc, &gljcRec, GTEQ, "r");
	while (!cc && !strcmp (gljcRec.co_no, comm_rec.co_no) &&
			!strncmp (gljcRec.journ_type, searchValue, strlen (searchValue)))
	{
		cc = save_rec (gljcRec.journ_type, gljcRec.jnl_desc);
		if (cc)
			break;
		cc = find_rec (gljc,&gljcRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (local_rec.journalNo, temp_str);
}

int
NotBlank (
	char	*checkString)
{
	int		length = strlen (clip (checkString));
	return (length);
}

/*
 * Update journals to general ledger.
 */
int
UpdateToGl (
	char	*journalType)
{
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, journalType);
	cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess128));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*
	 * Split line into parameters etc & Substit
	 * Execute cmd line
 	 */
	strcpy (runCommand, "gl_wupdate~TYPE~PID~LPNO~");
	ProcessLine (runCommand);
	cc = ExcuteCommand ();
	if (cc)
		return (EXIT_FAILURE);

	/*
	 * Execute the report parameter if and only if rep_prog1 is not NotBlank
	 */
	if (NotBlank (gljcRec.rep_prog1))
    {
		ProcessLine (gljcRec.rep_prog1);
		cc = ExcuteCommand ();
		if (cc)
			return (EXIT_FAILURE);
    }

	/*
	 * Execute the report parameter if and only if rep_prog2 is not NotBlank
	 */
	if (NotBlank (gljcRec.rep_prog2))
    {
		ProcessLine (gljcRec.rep_prog2);
		cc = ExcuteCommand ();
		if (cc)
			return (EXIT_FAILURE);
    }

	strcpy (runCommand, "gl_wdelwk~PID~");
	ProcessLine (runCommand);
	cc = ExcuteCommand ();
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Load batch details.
 */
int
LoadBatch (void)
{
	BatchError = TRUE;

	tab_open (gl_postbat, tab_keys, 2, 2, WK_DEPTH, FALSE);
	tab_add (gl_postbat, 
			"# %-5.5s%-11.11s% -20.20s %-12.12s %-6.6s %5.5s  %-10.10s",
			"C/R", "Batch No.", "Journal Type", "Date", 
			"Time", "Per.", "User");

	if (!strcmp (local_rec.journalNo, "ALL"))
	{
		sprintf (gljcRec.co_no, comm_rec.co_no);
		sprintf (gljcRec.journ_type, "  ");
		cc = find_rec (gljc, &gljcRec, GTEQ, "r");
		while (!cc && !strcmp (gljcRec.co_no, comm_rec.co_no))
		{
			LoadOneJournalType (gljcRec.journ_type);
			cc = find_rec (gljc, &gljcRec, NEXT, "r");
		}
	}
	else
    {
		LoadOneJournalType (local_rec.journalNo);
    }
	return (BatchError);
}

void
LoadOneJournalType (
	char	*journalType)
{
	int		lastPostDmy [3]	=	{0,0,0},
			displayMonth	= 	0;

	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	strcpy (glbhRec.jnl_type, journalType);
	sprintf (glbhRec.batch_no, "%10.10s", " ");

	cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	while (	!cc && 
			!strcmp (glbhRec.co_no, comm_rec.co_no) &&
			!strcmp (glbhRec.jnl_type, journalType))
	{
		if (HEADER_POSTED && !STANDING)
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}

		if (STANDING)
		{
			glsj_rec.hhbh_hash	=	glbhRec.hhbh_hash;
			cc = find_rec (glsj,&glsj_rec,COMPARISON,"r");
			if (cc)
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}
			dayApplied = glsj_rec.dt_apply;

			DateToDMY (glsj_rec.from_dt, &dmy [0],&dmy [1],&dmy [2]);
			testDay = dmy [0];
			if (testDay > 28)
				testDay = 28;
			if (dayApplied < testDay)
				glsj_rec.from_dt += 28;

			DateToDMY (glsj_rec.to_dt, &dmy [0],&dmy [1],&dmy [2]);
			testDay = dmy [0];
			if (testDay > 28)
				testDay = 28;
			if (dayApplied > testDay)
				glsj_rec.to_dt -= 28;

			DateToDMY (glsj_rec.dt_posted, &dmy [0],&dmy [1],&dmy [2]);

			if (comm_rec.gl_date < glsj_rec.from_dt ||
				comm_rec.gl_date > glsj_rec.to_dt ||
				dayApplied > moduleDmy [0])
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}

			DateToDMY 
			(
				glsj_rec.dt_posted, 
				&lastPostDmy [0],
				&lastPostDmy [1],
				&lastPostDmy [2]
			);
			DateToDMY (comm_rec.gl_date, &dmy [0],&dmy [1],&dmy [2]);
			if (lastPostDmy [1] == dmy [1] && lastPostDmy [2] == dmy [2])
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}
		}

		BatchError = FALSE;
    
        DateToDMY (comm_rec.gl_date, &dmy [0],&dmy [1],&dmy [2]);
		displayMonth = (STANDING) ? 
						mth2fisc (dmy [1], comm_rec.fiscal): 
						mth2fisc (glbhRec.mth, comm_rec.fiscal); 

		tab_add 
		(
			gl_postbat, 
		" %-5.5s %-10.10s %-3.3s %-16.16s %-12.12s %-6.6s  %2.2d    %-10.10s",
			"C", 							/* 1 */
			glbhRec.batch_no, 				/* 7 */
			glbhRec.jnl_type, 				/* 18 */
			gljcRec.jnl_desc,			
			DateToString (glbhRec.glbh_date), 
			glbhRec.glbh_time, 		
			displayMonth, 
			glbhRec.user
		);
		cc = find_rec (glbh, &glbhRec, NEXT, "r");
	}
	return;
}
static	int
wkConfirm (
		int			iUnused,
		KEY_TAB		*psUnused)
{
	char	recBuffer [256];
	int		oldLine;

	oldLine = tab_tline (gl_postbat);
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	if (!cc)
	{
		recBuffer [1] = 'A';
		tab_update (gl_postbat, "%s", recBuffer);
		cc = tab_get (gl_postbat, recBuffer, NEXT, 0);
		if (cc)
			cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	}
	oldLine = tab_tline (gl_postbat);
	if ( (oldLine % WK_DEPTH) == 0)
		load_page (gl_postbat, FALSE);
	redraw_page (gl_postbat, TRUE);
    return (iUnused);
}

static	int
WkConfirmAll (
	int			iUnused,
	KEY_TAB*	psUnused)
{
	char	recBuffer [256];
	int		oldLine;

	oldLine = tab_tline (gl_postbat);
	cc = tab_get (gl_postbat, recBuffer, EQUAL, first_line);
	while (!cc)
	{
		recBuffer [1] = 'A';
		tab_update (gl_postbat, "%s", recBuffer);
		cc = tab_get (gl_postbat, recBuffer, NEXT, 0);
	}
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	load_page (gl_postbat, FALSE);
	redraw_page (gl_postbat, TRUE);
    return (iUnused);
}

static	int
WkReject (
	int			iUnused,
	KEY_TAB*	psUnused)
{
	char	recBuffer [256];
	int		oldLine;

	oldLine = tab_tline (gl_postbat);
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	if (!cc)
	{
		recBuffer [1] = 'C';
		tab_update (gl_postbat, "%s", recBuffer);
		cc = tab_get (gl_postbat, recBuffer, NEXT, 0);
		if (cc)
			cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	}
	oldLine = tab_tline (gl_postbat);
	if ( (oldLine % WK_DEPTH) == 0)
		load_page (gl_postbat, FALSE);
	redraw_page (gl_postbat, TRUE);
    return (iUnused);
}

static	int
WkRejectAll (
	int			iUnused,
	KEY_TAB*	psUnused)
{
	char	recBuffer [256];
	int		oldLine;

	oldLine = tab_tline (gl_postbat);
	cc = tab_get (gl_postbat, recBuffer, EQUAL, first_line);
	while (!cc)
	{
		recBuffer [1] = 'C';
		tab_update (gl_postbat, "%s", recBuffer);
		cc = tab_get (gl_postbat, recBuffer, NEXT, 0);
	}
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	load_page (gl_postbat, FALSE);
	redraw_page (gl_postbat, TRUE);
    return (iUnused);
}

static	int
WkView (
	int			iUnused,
	KEY_TAB*	psUnused)
{
	char	recBuffer [256];
	int		oldLine;
	char	batch_no [sizeof glbhRec.batch_no];
	char	batch_type [3];
	char	runCommand [50];

	oldLine = tab_tline (gl_postbat);
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);

	sprintf (batch_no, 	"%-10.10s", recBuffer + 7);
	sprintf (batch_type,"%-2.2s", recBuffer + 18);
	sprintf (runCommand, "gl_batch_disp~%-2.2s~%-10.10s~",batch_type, batch_no);
	ProcessLine (runCommand);
	cc = ExcuteCommand ();
	
	cc = tab_get (gl_postbat, recBuffer, EQUAL, oldLine);
	heading (1);
	redraw_table (gl_postbat);
	redraw_page (gl_postbat, TRUE);
    return (iUnused);
}

void
UpdateBatch (void)
{
	char	recBuffer [256];
	int		posted = FALSE;

	clear ();
	print_at (0,0,ML (mlStdMess035));

	if (tab_get (gl_postbat, recBuffer, FIRST, 0))
		return;

	while (TRUE) 
	{
		if (recBuffer [1] != 'A')
		{
			if (tab_get (gl_postbat, recBuffer, NEXT, 0))
				break;
			continue;
		}

		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.br_no, comm_rec.est_no);
		sprintf (glbhRec.jnl_type, "%-2.2s", recBuffer + 18);
		sprintf (glbhRec.batch_no, "%-10.10s", recBuffer + 7);
	
		cc = find_rec (glbh, &glbhRec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (glbh);
			if (tab_get (gl_postbat, recBuffer, NEXT, 0))
				return;
			continue;
		}

		if (STANDING)
		{
			glsj_rec.hhbh_hash	=	glbhRec.hhbh_hash;
			cc = find_rec (glsj, &glsj_rec, COMPARISON, "u");
			if (cc)
			{
				abc_unlock (glsj);
				if (tab_get (gl_postbat, recBuffer, NEXT, 0))
					return;

				continue;
			}
		}
		glblRec.hhbh_hash = glbhRec.hhbh_hash;
		glblRec.line_no   = 0;
		cc = find_rec (glbl, &glblRec, GTEQ, "u");
		while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
		{
			if (DETAIL_POSTED && !STANDING)
			{
				abc_unlock (glbl);
				cc = find_rec (glbl, &glblRec, NEXT, "u");
				continue;
			}
			/*
			 * Open Work file if not already done. 
			 */
			OpenGlWork ();
	
			strcpy (glwkRec.acc_no, glblRec.acc_no);
			strcpy (glwkRec.co_no,  comm_rec.co_no);
			strcpy (glwkRec.est_no, comm_rec.est_no);
			strcpy (glwkRec.acronym, glblRec.acronym);
			strcpy (glwkRec.name, glblRec.name);
			strcpy (glwkRec.chq_inv_no, glblRec.chq_inv_no);
			glwkRec.ci_amt = glblRec.ci_amt;
			glwkRec.o1_amt = glblRec.o1_amt;
			glwkRec.o2_amt = glblRec.o2_amt;
			glwkRec.o3_amt = glblRec.o3_amt;
			glwkRec.o4_amt = glblRec.o4_amt;
			glwkRec.hhgl_hash = glblRec.hhgl_hash;
			strcpy (glwkRec.tran_type, glblRec.tran_type);
			strcpy (glwkRec.sys_ref, glblRec.sys_ref);
			if (STANDING)
			{
				glwkRec.tran_date = comm_rec.gl_date;
				sprintf (glwkRec.period_no, "%02d", moduleDmy [1]);
			}
			else
			{
				glwkRec.tran_date = glblRec.tran_date;
				sprintf (glwkRec.period_no, "%02d", glbhRec.mth);
			}
			glwkRec.post_date = TodaysDate ();
			strcpy (glwkRec.narrative, glblRec.narrative);
			strcpy (glwkRec.alt_desc1, glblRec.alt_desc1);
			strcpy (glwkRec.alt_desc2, glblRec.alt_desc2);
			strcpy (glwkRec.alt_desc3, glblRec.alt_desc3);
			strcpy (glwkRec.batch_no,  glblRec.batch_no);
			strcpy (glwkRec.user_ref,  glblRec.user_ref);
			strcpy (glwkRec.jnl_type,  glblRec.dc_flag);
			strcpy (glwkRec.currency,  glblRec.currency);
			glwkRec.amount 		= glblRec.fx_amt;
			glwkRec.loc_amount 	= glblRec.local_amt;
			glwkRec.exch_rate 	= glblRec.exch_rate;
			strcpy (glwkRec.stat_flag, "2");
			strcpy (glwkRec.run_no, glbhRec.batch_no);
	
			cc = RF_ADD (glwkNo, (char *) &glwkRec);
			if (cc) 
				break;
	
			posted = TRUE;
			strcpy (glblRec.stat_flag,"P");
			cc = abc_update (glbl, &glblRec);
			if (cc)
				break;
			cc = find_rec (glbl, &glblRec, NEXT, "u");
		} 
		abc_unlock (glbl);
		/*
		 * If Standing journal and posted successfully,
		 * update last posted date.                    
		 */
		if (STANDING && posted)
		{
			glsj_rec.dt_posted = comm_rec.gl_date;
			cc = abc_update (glsj, &glsj_rec);
			if (cc)
				file_err (cc, glsj, "DBUPDATE");
		}
		posted = FALSE;
		strcpy (glbhRec.stat_flag, "P");
		glbhRec.glbh_date = TodaysDate ();
		strcpy (glbhRec.glbh_time, TimeHHMM ());
		cc = abc_update (glbh, &glbhRec);
		if (cc)
			file_err (cc, glbh, "DBUPDATE");
 
		/*
	 	 * Close Work file if not already done. 
	 	 */
		if (workFileOpen == TRUE)
		{
			CloseGlWork ();

			Journal [0].val = strdup (glbhRec.jnl_type);
			Journal [1].val = strdup (printerString);
			Journal [2].val = strdup (processString);
			cc = UpdateToGl (glbhRec.jnl_type);
			if (cc && !STANDING)
			{
				print_mess (ML ("Error in Posting, Reversing transactions"));
				sleep (sleepTime);
				/*
				 * Reverse out postings.
				 */
				strcpy (glbhRec.co_no, comm_rec.co_no);
				strcpy (glbhRec.br_no, comm_rec.est_no);
				sprintf (glbhRec.jnl_type, "%-2.2s", recBuffer + 18);
				sprintf (glbhRec.batch_no, "%-10.10s", recBuffer + 7);
				cc = find_rec (glbh, &glbhRec, COMPARISON, "u");
				if (cc)
					file_err (cc, glbh, "DBFIND");

				glblRec.hhbh_hash = glbhRec.hhbh_hash;
				glblRec.line_no   = 0;
				cc = find_rec (glbl, &glblRec, GTEQ, "u");
				while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
				{
					if (DETAIL_POSTED)
					{
						strcpy (glblRec.stat_flag,"N");
						cc = abc_update (glbl, &glblRec);
						if (cc)
							file_err (cc, glbl, "DBUPDATE");
					}
					else
						abc_unlock (glbl);

					cc = find_rec (glbl, &glblRec, NEXT, "u");
				}
				strcpy (glbhRec.stat_flag, "N");
				cc = abc_update (glbh, &glbhRec);
				if (cc)
					file_err (cc, glbh, "DBUPDATE");

				print_mess (ML ("Reversing of transactions complete"));
				sleep (sleepTime);
			}
		}
	    if (tab_get (gl_postbat, recBuffer, NEXT, 0))
			break;
	}
}

void
OpenGlWork (void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	filename [256];

	if (workFileOpen == TRUE)
		return;

	sprintf (filename, "%s/WORK/gl_work%05d", 
				(sptr == (char *) 0) ? "/usr/ver9.10" : sptr, ProcessID);

	cc = RF_OPEN (filename,sizeof (GLWK_STRUCT),"w",&glwkNo);
	if (cc)
		file_err (cc, filename, "RF_OPEN");

	workFileOpen = TRUE;
}

void
CloseGlWork (void)
{
	if (workFileOpen == FALSE)
		return;

	cc = RF_CLOSE (glwkNo);
	if (cc) 
		file_err (cc, glwk, "WKCLOSE");

	workFileOpen = FALSE;
}
/*
 * Process Line for journal type.
 */
void
ProcessLine (
	char	*line)
{
	char *	sptr = line;
	char *	tptr = line;
	int		indx = 0;
	int		quote = 0;

	while (*tptr)
	{
		if (*tptr == '~' && !quote)
		{
			*tptr = (char) NULL;

			if (indx == 0)
				args [indx++] = sptr;
			else
				args [indx++] = Substitute (sptr);

			args [indx] = NULL;
			sptr = tptr + 1;
		}

		if (*tptr == '"')
			quote = !quote;
		tptr++;
	}
}

/*
 * Substitute values.
 */
char *
Substitute (
	char	*string)
{
	int	indx;

	for (indx = 0; strlen (Journal [indx].lbl); indx++)
	{
		if (!strcmp (CODE,string))
			return (Journal [indx].val);
	}
	return (string);
}
/*
 * Execute commend.
 */
int
ExcuteCommand (void)
{
	int		status	=	0;

#ifdef GVISION

	// gbSysExec, if set to TRUE waits for the child process to exit.
	gbSysExec = TRUE;
	status  =   execvp (args [0], args);	
#else
	void 	(*old_sigvec) ();

	old_sigvec	=	signal (SIGCLD, SIG_DFL);

	switch (fork ())
	{
	case -1:
		signal (SIGCLD, old_sigvec);
		return 0;

	case 0:
		/*
		 *	Child process
		 */
		status	=	execvp (args [0], args);
		signal (SIGCLD, old_sigvec);
		exit (EXIT_FAILURE);

	default:
		/*
		 *	Parent process
		 */
		wait ( (int *)0);
	}
	signal (SIGCLD, old_sigvec);
#endif

	return (status);
}

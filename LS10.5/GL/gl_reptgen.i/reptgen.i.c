/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: reptgen.i.c,v 5.4 2002/07/17 09:57:13 scott Exp $
|  Program Name  : (gl_reptgen.i.c)                      
|  Program Desc  : (General Ledger report Generator Input)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: reptgen.i.c,v $
| Revision 5.4  2002/07/17 09:57:13  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:13:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:34  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:01  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: reptgen.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_reptgen.i/reptgen.i.c,v 5.4 2002/07/17 09:57:13 scott Exp $";

#include	<pslscr.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;

#define	MAXNAMES	1000

	char	fnam [MAXNAMES][15],
			*PROGDIR,
			periodPlus [3],
			periodMinus [3];

	int	nonames;

	/*
	 * Set up Array to hold Days of the week used with wday in time struct.
	 */
	char	*mth [] =
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};
	char	yesPrompt 	[11],
			noPrompt	[11];

	extern	int	TruePosition;
	extern	int	EnvScreenOK;

void 	OpenDB 				 (void);
void 	CloseDB 			 (void);

/*
 * Local & Screen Structures. 
 */
struct
{
	char	dummy [11];
	char	progname [15];
	int		printerNo;
	int		budgetOneNo;
	int		budgetTwoNo;
	char	budgetOneStr [3];
	char	budgetTwoStr [3];
	long	periodDate;
	char	printerString [3];
	char	accNoReq [2];
	char	accNoReqDesc [11];
	char	wholeDollars [2];
	char	wholeDollarsDesc [11];
	char	dateStr [11];
	char	systemDate [11];
	Date	lsystemDate;
	char	back [2];
	char	backDesc [11];
	char	onight [2];
	char	onightDesc [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "prog_name",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		"", "", "Report Name.         ", " [SEARCH] Available Files. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.progname},
	{1, LIN, "accNoReq",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print Account Nos.   ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.accNoReq},
	{1, LIN, "accNoReqDesc",	 6, 26, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.accNoReqDesc},
	{1, LIN, "wholeDollars",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Print Whole Values.  ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.wholeDollars},
	{1, LIN, "wholeDollarsDesc",	 7, 26, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.wholeDollarsDesc},
	{1, LIN, "budgetOneNo",	 8, 2, INTTYPE,
		"NN", "          ",
		" ", "0", "Budget No.           ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.budgetOneNo},
	{1, LIN, "budgetTwoNo",	 9, 2, INTTYPE,
		"NN", "          ",
		" ", "0", "Budget No.           ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.budgetTwoNo},
	{1, LIN, "periodDate",	10, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "As at                ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.periodDate},
	{1, LIN, "printerNo",	12, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No.          ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",		13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background.          ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 13, 26, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight.           ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 14, 26, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
int 	spec_valid 			(int);
void 	LoadDiectory 		(char *);
int 	heading 			(int);
void 	ShowFiles 			(char *);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	glDate [11];
	char	fileName	[256];
	int	glper = 0;

	if (argc != 4)
	{
		print_at (0,0, mlGlMess705, argv [0]);
		return (EXIT_FAILURE);
	}
	OpenDB ();

	local_rec.lsystemDate	=	TodaysDate ();
	local_rec.lsystemDate	=	MonthEnd (local_rec.lsystemDate);
	strcpy (local_rec.systemDate , DateToString (local_rec.lsystemDate));

	/*
	 *	Examine the source_dir argument. If it's not an absolute pathname,
	 *	we take it relative to $PROG_PATH
	 */
	if (*argv [1] == '/')
		PROGDIR = strdup (argv [1]);
	else
	{
		char	*prog_path = getenv ("PROG_PATH");		/* assume there! */

		PROGDIR = (char *) malloc (strlen (prog_path) + strlen (argv [1]) + 2);
		sprintf (PROGDIR, "%s/%s", prog_path, argv [1]);
	}

	strcpy (glDate, DateToString (TodaysDate ()));

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	if (access (PROGDIR, 00))
	{
		print_at (0,0, ML (mlStdMess132));
		return (EXIT_FAILURE);
	}

	LoadDiectory (PROGDIR);

	glper = atoi (glDate + 3);
	if (glper == 12)
		sprintf (periodPlus, "%02d", 1);
	else
		sprintf (periodPlus, "%02d", glper + 1);

	sprintf (periodMinus, "%02d", (glper > 12) ? 1 : glper);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	entry_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	search_ok 	= TRUE;
	init_vars (1);

	/*
	 * Enter screen 1 linear input.
	 */
	heading (1);
	entry (1);
	if (prog_exit)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	/*
	 * Edit screen 1 input.
	 */
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
	if (restart)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	rset_tty ();
	sprintf (fileName, "%s/%s", PROGDIR, local_rec.progname);
	local_rec.accNoReq [1] = 0;
	local_rec.wholeDollars [1] = 0;

	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT gl_reptgen %d %s %s %s %d %d %s %s",
			local_rec.printerNo,
			fileName,
			local_rec.accNoReq,
			local_rec.wholeDollars,
			local_rec.budgetOneNo,
			local_rec.budgetTwoNo,
			local_rec.dateStr,
			argv [3]
		);
		SystemExec (err_str, TRUE);
	}
	else 
	{
		sprintf 
		(
			err_str, 
			"gl_reptgen %d %s %s %s %d %d %s",
			local_rec.printerNo,
			fileName,
			local_rec.accNoReq,
			local_rec.wholeDollars,
			local_rec.budgetOneNo,
			local_rec.budgetTwoNo,
			local_rec.dateStr
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}
/*
 * Open database files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (yesPrompt, ML ("Yes "));
	strcpy (noPrompt,  ML ("No  "));
}

/*
 * Close database files.
 */
void
CloseDB (void)
{
	abc_dbclose ("data");
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	free (PROGDIR);
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
	int		field)
{
	int	i,
		notfound;

	/*
	 * Display directory.
	 */
	if (LCHECK ("prog_name"))
	{
		if (SRCH_KEY)
		{
			ShowFiles (temp_str);
			return (EXIT_SUCCESS);
		}
		notfound = 1;

		for (i = 0;i < nonames && notfound;i++)
			notfound = strcmp (clip (fnam [i]), local_rec.progname);

		if (notfound)
		{
			sprintf (err_str, ML (mlGlMess156), BELL, local_rec.progname);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.printerString, "%d", local_rec.printerNo);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Budget Number
	 */
	if (LCHECK ("budgetOneNo"))
		return (EXIT_SUCCESS);

	/*
	 * Validate Budget Number
	 */
	if (LCHECK ("budgetTwoNo"))
		return (EXIT_SUCCESS);

	/*
	 * Validate Period Date
	 */
	if (LCHECK ("periodDate"))
	{
		strcpy (local_rec.dateStr,DateToString (local_rec.periodDate));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("accNoReq"))
	{
		strcpy (local_rec.accNoReqDesc, 
			(local_rec.accNoReq [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("accNoReqDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wholeDollars"))
	{
		strcpy (local_rec.wholeDollarsDesc,
			(local_rec.wholeDollars [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("wholeDollarsDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
				(local_rec.back [0] == 'Y') ? yesPrompt : noPrompt);

		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.onight, "N");
			strcpy (local_rec.onightDesc, noPrompt);
		}
		DSP_FLD ("back");
		DSP_FLD ("backDesc");
		DSP_FLD ("onight");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc,
				(local_rec.onight [0] == 'Y') ? yesPrompt : noPrompt);

		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.back, "N");
			strcpy (local_rec.backDesc, noPrompt);
		}
		DSP_FLD ("back");
		DSP_FLD ("backDesc");
		DSP_FLD ("onight");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Read all Files in Directory and then display.
 */
void
LoadDiectory (
 char*              name)
{
	DIR*            fd;
	struct dirent*  dirbuf;

	nonames = 0;

	if (! (fd = opendir (name)))
		return;

	while ( (dirbuf = readdir (fd)))
	{
		if (!strcmp (dirbuf -> d_name, ".") || !strcmp (dirbuf -> d_name, ".."))
			continue;

		if (nonames < MAXNAMES)
			sprintf (fnam [nonames++], "%-14.14s", dirbuf -> d_name);
		else
			break;
	}
	closedir (fd);
}

int
heading (
 int                scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		centre_at (0, 80, ML (mlGlMess053));

		line_at (1,0,80);

		box (0, 3, 80, 11);
		line_at (5,1,79);
		line_at (11,1,79);

		line_at (20,0,80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		line_at (22,0,80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*
 * Search routine for Serial Item master file. |
 */
void
ShowFiles (
 char*              key_val)
{
	int	i;

	work_open ();
	save_rec ("#   File Name.  ", "#Directory.");

	for (i = 0; i < nonames; i++)
	{
		if (!strncmp (key_val, fnam [i], strlen (key_val)))
		{
			cc = save_rec (fnam [i], PROGDIR);
			if (cc)
				break;
		}
	}
	cc = disp_srch ();
	work_close ();
	sprintf (local_rec.progname, "%-14.14s", temp_str);
}

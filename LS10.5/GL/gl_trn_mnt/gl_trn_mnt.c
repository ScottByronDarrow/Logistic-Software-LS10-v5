/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_trn_mnt.c,v 5.8 2002/07/08 07:06:08 kaarlo Exp $
|  Program Name  : (gl_trn_mnt.c)   
|  Program Desc  : (General Ledger Narrative Maintenance) 
|---------------------------------------------------------------------|
|  Author        : Trev van Bremen | Date Written  : 17/01/91         |
|---------------------------------------------------------------------|
| $Log: gl_trn_mnt.c,v $
| Revision 5.8  2002/07/08 07:06:08  kaarlo
| S/C 4088. Updated to make QuitFunc work.
|
| Revision 5.7  2002/01/30 02:27:02  cha
| Updated to fix blank account nos.
|
| Revision 5.6  2002/01/28 08:49:44  cha
| S/C 579 fixed error when account is not found.
| clears field acctNo.
|
| Revision 5.5  2002/01/25 06:29:59  scott
| ..
|
| Revision 5.4  2002/01/25 03:27:48  scott
| Updated to fix message staying in screen
|
| Revision 5.3  2001/08/09 09:13:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:38  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:03  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_trn_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_trn_mnt/gl_trn_mnt.c,v 5.8 2002/07/08 07:06:08 kaarlo Exp $";

#include <pslscr.h>
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <GlUtils.h>
#include <hot_keys.h>
#include <getnum.h>
#include <tabdisp.h>

#define		MAX_JNL		27
#define		SCNPOSOFF	9

static	int	QuitFunc 	 (int, KEY_TAB *);
static	int	ChangeFunc 	 (int, KEY_TAB *);
static	int	UpdateFunc 	 (int, KEY_TAB *);

char	*gltrTab	=	"gltrTab";

#ifdef	GVISION
static	KEY_TAB	gltrKeys [] =
{
  { " Quit ", 'Q', QuitFunc, "Quit ALL changes to this account",	},
  { " Change Narrative ", 'C', ChangeFunc, "Update the current line",		},
  { NULL, FN16, UpdateFunc, "Save ALL changes to this account",	},
  END_KEYS
};
#else
static	KEY_TAB	gltrKeys [] =
{
  { " [Q]uit ", 'Q', QuitFunc, "Quit ALL changes to this account",	},
  { " [C]hange Narrative ", 'C', ChangeFunc, "Update the current line",		},
  { NULL, FN16, UpdateFunc, "Save ALL changes to this account",	},
  END_KEYS
};
#endif

	static	char 	*journalDesc [MAX_JNL + 1] =
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

#include	"schema"

struct commRecord	comm_rec;

extern	int	TruePosition;
/*
 * Local & Screen Structures.
 */
struct
{
	char	acctNo [sizeof glmrRec.acc_no];
	char 	dummy [10];
	long	startDate;
	long	endDate;
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "acctNo", 2, 2, CHARTYPE, 
		"NNNNNNNNNNNNNN", "          ", 
		"", "", "Account number        ", " ", 
		YES, NO, JUSTLEFT, "0123456789-", "", local_rec.acctNo}, 
	{1, LIN, "acctDesc", 2, 70, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		"", "", "Account Description   ", " ", 
		NA, NO, JUSTLEFT, "", "", glmrRec.desc}, 
	{1, LIN, "startDate", 3, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		"", " ", "Start Date            ", " ", 
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.startDate}, 
	{1, LIN, "endDate", 3, 70, EDATETYPE, 
		"DD/DD/DD", "          ", 
		"", " ", "End Date              ", " ", 
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.endDate}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Local Function Prototypes.
 */

void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);
void 	ProcessAcc 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*
	 * Setup required parameters.
	 */
	init_scr ();
	set_tty ();

	set_masks ();		
	init_vars (1);	

	OpenDB ();

	vars [label ("acctNo")].mask 	= GL_SetAccWidth (comm_rec.co_no, TRUE);

	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		/*
		 * Process g/l acc record.
		 */
		ProcessAcc ();
	}	
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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlmr ();
	OpenGltr ();	abc_selfield (gltr, "gltr_id_no2");
	OpenGlna ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose ("data");
}

/*
 * Standard Screen Heading Routine
 */
int
heading (
 int                scn)
{
	int	s_size = 132;

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		swide ();

		rv_pr (ML (mlGlMess025), 45, 0, 1);

		box (0, 1, s_size, 2);

		line_at (20,0,s_size);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,s_size);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	long	tmpDate;

	/*
	 * Validate Account Number.
	 */
	if (LCHECK ("acctNo"))
	{
		if (dflt_used)
			strcpy (local_rec.acctNo, GL_GetDfltSfaccCode ());

		if (SRCH_KEY)
		{
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			strcpy (local_rec.acctNo, temp_str);
		}

		GL_FormAccNo (local_rec.acctNo, glmrRec.acc_no, 0);

		strcpy (glmrRec.co_no, comm_rec.co_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			strcpy (local_rec.acctNo,"");
			return (EXIT_FAILURE);
		}
		DSP_FLD ("acctDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Start Date.	
	 */
	if (LCHECK ("startDate"))
	{
		if (dflt_used)
			local_rec.startDate	=	MonthStart (TodaysDate ());
	
		if (local_rec.startDate > local_rec.endDate && last_char == EOI)
		{
			tmpDate 				= local_rec.startDate;
			local_rec.startDate 	= local_rec.endDate;
			local_rec.endDate 		= tmpDate;
			DSP_FLD ("startDate");
			DSP_FLD ("endDate");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Date.
	 */
	if (LCHECK ("endDate"))
	{
		if (dflt_used)
			local_rec.endDate	=	MonthEnd (TodaysDate ());

		if (local_rec.startDate > local_rec.endDate)
		{
			local_rec.startDate	= MonthStart (local_rec.endDate);
			DSP_FLD ("startDate");
			DSP_FLD ("endDate");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Function loads data from gltr and glna into tab routines.
 */
void
ProcessAcc (void)
{
	char	tranDate	[11],
			postDate	[11];

	int	jnl_no;

	cc = find_rec (glmr, &glmrRec, EQUAL, "u");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	gltrRec.hhmr_hash = glmrRec.hhmr_hash;
	gltrRec.tran_date = local_rec.startDate;
	cc = find_rec (gltr, &gltrRec, GTEQ, "r");
	if (cc || gltrRec.hhmr_hash != glmrRec.hhmr_hash ||
			  gltrRec.tran_date > local_rec.endDate)
	{
		abc_unlock (glmr);
		print_mess (ML (mlGlMess024));
		sleep (sleepTime);
		clear_mess ();
		return;
	}

	tab_open (gltrTab, gltrKeys, 4, SCNPOSOFF, 12, FALSE);
	
	tab_add (gltrTab, "# Date Trans  |Date Posted |Transaction  Narrative|Journal Type|Journal No| User Reference  |Curr| Foreign Amount ");

	while (!cc &&
		gltrRec.hhmr_hash == glmrRec.hhmr_hash &&
		gltrRec.tran_date <= local_rec.endDate)
	{
		jnl_no = atoi (gltrRec.tran_type);
		if (jnl_no > MAX_JNL)
			jnl_no = 0;

		strcpy (tranDate,	DateToString (gltrRec.tran_date));
		strcpy (postDate,	DateToString (gltrRec.post_date));
		tab_add 
		(
			gltrTab, 
			"  %10.10s | %10.10s | %-20.20s | %10.10s |%-10.10s| %15.15s |%4.4s| %15.2lf %010ld-%02d",
			tranDate, 
			postDate, 
			gltrRec.narrative,
			journalDesc [jnl_no],
			gltrRec.sys_ref,
			gltrRec.user_ref,
			gltrRec.currency,
			DOLLARS (gltrRec.amount),
			gltrRec.gltr_hash,
			-1
		);
		glnaRec.gltr_hash	=	gltrRec.gltr_hash;
		glnaRec.line_no		=	0;
		cc = find_rec (glna, &glnaRec, GTEQ, "r");
		while (!cc && glnaRec.gltr_hash	== gltrRec.gltr_hash)
		{
			tab_add 
			(
				gltrTab, 
				" %12.12s|%12.12s| %-20.20s |%12.12s|%10.10s|%17.17s|%4.4s|%16.16s %010ld-%02d",
				" ",
				" ",
				glnaRec.narrative,
				" ",
				" ",
				" ",
				" ",
				" ",
				glnaRec.gltr_hash,
				glnaRec.line_no
			);
			cc = find_rec (glna, &glnaRec, NEXT, "r");
		}
		cc = find_rec (gltr, &gltrRec, NEXT, "r");
	}

#ifdef GVISION
	heading (1);
	scn_display (1);
#endif

	if (!tab_display (gltrTab, TRUE))
		tab_scan (gltrTab);

	tab_close (gltrTab, TRUE);
	snorm ();

	abc_unlock (glmr);
}

/*
 * Function aborts input.
 */
static int
QuitFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	return (FN16);
}

/*
 * Function allows user to change narrative from gltr and glna
 */
static	int
ChangeFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	int		validNarrative	=	FALSE;
	int		yPosition;
	char	tempLine [256];
	char	narrative [21];

	tab_get (gltrTab, tempLine, EQUAL, tab_tline (gltrTab));

	yPosition	=	tab_sline (gltrTab);

	sprintf (narrative, "%-20.20s", tempLine + 28);

	while (!validNarrative)
	{
		crsr_on ();
		sprintf (narrative, "%-20.20s", tempLine + 28);
		getalpha (SCNPOSOFF + 29, yPosition, "AAAAAAAAAAAAAAAAAAAA", narrative);
		crsr_off ();
		
		if (last_char == FN4)
		{
			redraw_table ("LL_lst");
			last_char	= ' ';
		}
		if (last_char == FN2)
			sprintf (narrative, "%-20.20s", tempLine + 28);

		if (last_char == FN1)
			break;

		if (last_char == FN16)
			continue;

		validNarrative = TRUE;
	}
	sprintf (err_str, "%-28.28s%-20.20s%-s", tempLine, narrative,tempLine + 48);
	tab_update (gltrTab, "%s", err_str);

	return (c);
}

/*
 * Function that reads information from tab screen and updates gltr and glna
 */
static	int
UpdateFunc (
	int		c,
	KEY_TAB	*psUnused)
{
	long	gltrHash	=	0L;
	int		gltrLine	=	-1;
	char	tempLine [256];
	char	narrative [21];

	abc_selfield (gltr, "gltr_gltr_hash");

	cc = tab_get (gltrTab, tempLine, EQUAL, 0);
	while (!cc)
	{
		gltrHash	=	atol (tempLine + 114);
		gltrLine	=	atoi (tempLine + 125);
		sprintf (narrative, "%20.20s", tempLine + 28);

		/*
		 * Process normal general ledger transactions.
		 */
		if (gltrLine == -1)
		{
			gltrRec.gltr_hash	=	gltrHash;
			cc = find_rec (gltr, &gltrRec, COMPARISON, "u");
			if (cc)
				file_err (cc, gltr, "DBFIND");

			if (strcmp (gltrRec.narrative, narrative))
			{
				sprintf (gltrRec.narrative, "%-20.20s", narrative);
				cc = abc_update (gltr, &gltrRec);
				if (cc)
					file_err (cc, gltr, "DBUPDATE");
			}
			else
				abc_unlock (gltr);
		}
		/*
		 * Process extra general ledger transactions.
		 */
		else
		{
			glnaRec.gltr_hash	=	gltrHash;
			glnaRec.line_no		=	gltrLine;
			cc = find_rec (glna, &glnaRec, COMPARISON, "u");
			if (cc)
				file_err (cc, glna, "DBFIND");

			if (strcmp (glnaRec.narrative, narrative))
			{
				sprintf (glnaRec.narrative, "%-20.20s", narrative);
				cc = abc_update (glna, &glnaRec);
				if (cc)
					file_err (cc, glna, "DBUPDATE");
			}
			else
				abc_unlock (glna);
		}
		cc = tab_get (gltrTab, tempLine, NEXT, 0);
	}
	abc_selfield (gltr, "gltr_id_no2");

	return (c);
}

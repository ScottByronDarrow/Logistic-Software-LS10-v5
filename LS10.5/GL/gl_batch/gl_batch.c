/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_batch.c,v 5.14 2002/07/24 08:38:52 scott Exp $
|  Program Name  : (gl_batch.c) 
|  Program Desc  : (Input General Ledger batch)
|---------------------------------------------------------------------|
| $Log: gl_batch.c,v $
| Revision 5.14  2002/07/24 08:38:52  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.13  2002/07/23 05:14:56  scott
| .
|
| Revision 5.12  2002/07/08 07:05:12  scott
| S/C 004067 - Updated as error on currency not working.
|
| Revision 5.11  2002/07/08 03:23:43  scott
| Updated to fix memory problem with getval being -ve.
|
| Revision 5.10  2002/06/26 05:08:35  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.9  2002/06/25 04:44:25  scott
| No changes, cosmetic only
|
| Revision 5.8  2001/10/11 09:41:23  scott
| Updated to add DELLINE
|
| Revision 5.7  2001/10/11 09:04:53  scott
| Updated to fix problem with deletion of lines.
|
| Revision 5.6  2001/10/11 08:29:36  scott
| Updated to fix problem with deletion of lines saving wrong records.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_batch.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_batch/gl_batch.c,v 5.14 2002/07/24 08:38:52 scott Exp $";

/*
 *   Include file dependencies.
 */
#define TOTSCNS		3		/*  the number of screens used (incl lin.)*/
#define MAXSCNS		3		/*  the max. number of screens allowed    */
#define MAXLINES	5000	/*  the max. number of lines allowed      */

#define	MAX_NARR		3
#define	TXT_REQD
#include <pslscr.h>	
#include <GlUtils.h>	
#include <twodec.h>	
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 * Constants, defines and stuff.
 */
#define	CREDIT		 (local_rec.dc_flag [0] == 'C') 
#define	DEBIT		 (local_rec.dc_flag [0] == 'D')

#define	SR			store [line_cnt]
#define GENERAL     (atoi (local_rec.jnl_type) == 1)
#define STANDING    (atoi (local_rec.jnl_type) == 2)
#define ACCRUAL     (atoi (local_rec.jnl_type) == 3)

#define	NEW_RECORD	 (flag == 'A') 

#define	D_TOT(a)	 (store [a].debitValue)
#define	C_TOT(a)	 (store [a].creditValue)
#define	ACCNO(a)	 (store [a].accno)

#define	BASE_CURR(a) 	 (!strcmp (a, baseCurr))

#define	HEAD_SCN	1
#define	DETL_SCN	2

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glzhRecord	glzh_rec;
struct glzlRecord	glzl_rec;
struct glsjRecord	glsj_rec;
struct glsjRecord	glsj2_rec;

GLBH_STRUCT			glbh2Rec;

char 	*data = "data";

extern 	int 	GV_fiscal;
extern	int		_win_func;

	struct	storeRec {
		char	desc [MAX_NARR + 1] [21];
		long	hhmrHash;
		char	accno  	[MAXLEVEL + 1];
		double	debitValue;
		double	creditValue;
	} store [MAXLINES];

	/*
	 * Special fields and flags  ##################################.
 	 */	
   	char 	lcl_acc_type 	[4] = "**P", 
			accountDesc		[256],
			narrative 		[sizeof glblRec.narrative], 
			userReference 	[sizeof glblRec.user_ref], 
			flag, 
			baseCurr		[4], 
			batch_no	 	[1000] [sizeof glbhRec.batch_no], 
			batch_type	 	[1000] [3], 
			prior_mth 		[37];

	int		ALLOC 					= FALSE, 
			in_edit					= FALSE, 
			batchCounter 			= 0, 
  			priorPostingAllowed 	= 0, 
   			journalProofFlag 		= 0, 
			monthPeriod				= 0, 
			controlJournalMonth		= 0, 
			envGlJnlMaint 			= FALSE,
			newLineNo				= 0;

	long	defaultGlDate			= 0L, 
			batchNumber				= 0L;

	double	allocateTotalAmt		= 0.00;

/*
 * Local & Screen Structures.
 */
struct
{
	char	jnl_type [4];
	char	jnl_type_desc [21];
	char	curr_desc [21];
	char	batch_type_desc [21];
	char	dummy [11];
	char	userReference [sizeof glblRec.user_ref];
	int		line_no;
	char	systemDate [11];
	char	dc_flag [2];
	double	local_amt;
	long	hhbh_hash;
	long	tran_date;
	char	acctNo [sizeof glblRec.acc_no];
	long	glmr_hash;
	char	narrative [sizeof glblRec.narrative];
	char	tran_type [3];
	char	stat_flag [2];
	char	lpno [3];
	double	fxamt;
	double	fxrate;
	double	pass_amt;
	char	fxcurr [4];
	char	fxcurr_desc [21];
	char	allocation [5];
	int	mth;
} local_rec;

char	*scn_desc [] = 
{
	" Batch Header  ", 
	" Batch Details ", 
};

static	struct	var	vars [] =
{
	{HEAD_SCN, LIN, "jnl_type", 3, 16, CHARTYPE, 
		"AA", "         ", 
		" ", "",  "Journal Type :", " ", 
		NE, NO,  JUSTRIGHT, "", "", local_rec.jnl_type}, 
	{HEAD_SCN, LIN, "jnl_type_desc", 3, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "         ", 
		"", "", "", "", 
		NA, NO,  JUSTRIGHT, "", "", local_rec.jnl_type_desc}, 
	{HEAD_SCN, LIN, "batch_no", 4, 16, CHARTYPE, 
		"AAAAAAAAAA", "         ", 
		" ", " ", "Batch No.    :", " ", 
		NE, NO, JUSTLEFT, "", "", glbhRec.batch_no}, 
	{HEAD_SCN, LIN, "tran_mth", 5, 16, INTTYPE, 
		"NN", "               ", 
		" ", " ", "Tran Month   :", "Range : 1 to 12 ", 
		NE, NO, JUSTRIGHT, "", "", (char *)&glbhRec.mth}, 
	{HEAD_SCN, LIN, "day_app", 5, 16, INTTYPE, 
		"NN", "                  ", 
		"", "",   "Day apply    :", "Enter day of month for this journal to post. ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.dt_apply}, 
	{HEAD_SCN, LIN, "eff_from", 6, 16, EDATETYPE, 
		"DD/DD/DD", "                  ", 
		"", "",   "Eff. From    :", "Enter effective from date for standing post journal. ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.from_dt}, 
	{HEAD_SCN, LIN, "eff_to", 6, 44, EDATETYPE, 
		"DD/DD/DD", "                  ", 
		" ", "",  "Eff. To      :", "Enter effective to date for standing post journal. ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.to_dt}, 
	{DETL_SCN, TAB, 	"trans_no", MAXLINES, 0, INTTYPE, 
		"NNNN", "            ", 
		" ", " ", "Line", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.line_no}, 
	{DETL_SCN, TAB, 	"alloc", 0, 0, CHARTYPE, 
		"UUUU", "            ", 
		" ", " ", "Alcn", "Enter an allocation code, search or enter to ignore. ", 
		NE, NO, JUSTLEFT, "", "", local_rec.allocation}, 
	{DETL_SCN, TAB, "glacct", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		" ", "", "Account      ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.acctNo}, 
	{DETL_SCN, TAB, "glmr_hash", 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "           ", 
		"", "", "", "", 
		ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.glmr_hash}, 
	{DETL_SCN, TAB, "trans_date", 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Tran Date.", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.tran_date}, 
	{DETL_SCN, TAB, "d_c", 	 0, 0, CHARTYPE, 
		"U", "            ", 
		" ", "", " ", "Must Be D(ebit) or C(redit). ", 
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc_flag}, 
	{DETL_SCN, TAB, "fxcurr", 0, 0, CHARTYPE, 
		"UUU", "               ", 
		" ", " ", "Cur", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.fxcurr}, 
	{DETL_SCN, TAB, "fxamt", 0, 0, MONEYTYPE, 
		"NNNN,NNN,NNN.NN", "            ", 
		" ", "0", "    Amount     ", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999999", (char *) &local_rec.fxamt}, 
	{DETL_SCN, TAB, "fxrate", 0, 0, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "            ", 
		" ", " ", "Exchange Rate", "Enter currency for journal, [SEARCH] available.", 
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.fxrate}, 
	{DETL_SCN, TAB, "glamt", 0, 0, MONEYTYPE, 
		"NNNN,NNN,NNN.NN", "            ", 
		" ", " ", " Local Amount. ", " ", 
		NA, NO, JUSTRIGHT, "0", "99999999999", (char *) &local_rec.local_amt}, 
	{DETL_SCN, TAB, "userReference", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "        ", 
		" ", "", "User Reference.", " ", 
		 NO, NO,  JUSTLEFT, "", "", local_rec.userReference}, 
	{DETL_SCN, TAB, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "     Narrative      ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.narrative}, 
	{DETL_SCN, TAB, 	"trans_no", 0, 0, INTTYPE, 
		"NNNN", "            ", 
		" ", " ", "", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *) &glblRec.line_no}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};


/*
 *   Local function prototypes  
 */
int  	CheckBatchTotal 		(char *);
int  	ProcessTransactions 	(void);
int  	ReadComr 				(void);
int  	heading 				(int);
int  	spec_valid 				(int);
int 	DeleteLine 				(void);
int 	LoadGLBL 				(long);
int 	ValidateDate 			(int);
void 	AddDetailLine 			(int);
void 	CalculateJournalTotal 	(void);
void 	CloseDB 				(void);
void 	LoadGlsj 				(long);
void 	OpenDB 					(void);
void 	PrintAndProcessList 	(void);
void 	PrintJournalTotal 		(int);
void 	ProcBatchDetail 		(void);
void 	ProcBatchHeader 		(void);
void 	ProcessAllocation 		(char *, double);
void 	SrchGlbh 				(char *);
void 	SrchGljc 				(char *);
void 	SrchGlzh 				(char *);
void 	Update 					(void);
void 	UpdateDetailLine 		(int);
void 	shutdown_prog			(void);
void 	tab_other 				(int);
void	GetAccountDesc 			(void);
void	InputGlDesc				(int);;
/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	char	*sptr;
	int	i;

	if (argc < 2)
	{
		print_at (0, 0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	_win_func	=	TRUE;
	strcpy (local_rec.lpno, argv [1]);

	sptr = chk_env ("PRIOR_POST");
	priorPostingAllowed = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("GL_JNL_MAINT");
	envGlJnlMaint = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB (); 

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);


	tab_col = 0;
	tab_row = 9;

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 

	GV_fiscal = comm_rec.fiscal;
	DateToDMY (comm_rec.gl_date, NULL, &controlJournalMonth, NULL);
	defaultGlDate = comm_rec.gl_date;

	if (ReadComr () == EXIT_FAILURE)
    {
        prog_exit = TRUE;
        return (EXIT_FAILURE);
    }

	vars [label ("glacct")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);

	init_scr 	();
	set_tty 	();
	swide 		();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (DETL_SCN, store, sizeof (struct storeRec));
#endif
	init_ok = TRUE;
	init_vars (HEAD_SCN);
	init_vars (DETL_SCN);
	for (i = 0; i < 3; i++)
    {
		tab_data [i]._desc = scn_desc [i];
    }

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	
	while (!prog_exit) 
	{ 
		int		cnt;

		sprintf (narrative, "%-20.20s", " ");
		sprintf (userReference, "%15.15s", " ");
        abc_unlock (glbh);
        abc_unlock (glbl);
		for (i = 0; i < MAXLINES; i++) 
		{
			D_TOT (i)  = 0.00;
			C_TOT (i)  = 0.00;
			for (cnt = 0; cnt < MAX_NARR; cnt++)
				sprintf (store [i].desc [cnt], "%20.20s", " ");
		}

		entry_exit	= 	FALSE;
		edit_exit	= 	FALSE;
		prog_exit	=	FALSE;
		restart		=	FALSE;
		search_ok	= 	TRUE;
        init_ok		=	TRUE;

		init_vars (HEAD_SCN);
		init_vars (DETL_SCN);
		lcount [DETL_SCN] = 0;
		line_cnt = 0;

		/*
		 * Enter screen HEAD_SCN linear input.
		 */
		heading (HEAD_SCN);
		entry (HEAD_SCN);

		if (last_char == FN16)
			break;

		if (prog_exit || restart) 
			continue;

		in_edit = FALSE;
		/*
		 * Display screen DETL_SCN table input.
		 */
		if (NEW_RECORD)
		{
			line_cnt = 0;
			heading (DETL_SCN);
			scn_set (DETL_SCN);
			entry (DETL_SCN);
		}
		else
		{
			heading (DETL_SCN);
			scn_display (DETL_SCN);
		}

		if (restart)
			continue;

		in_edit = TRUE;
		line_cnt = 0;
		if (STANDING)
			edit_all ();
		else
		{
			heading (DETL_SCN);
			scn_display (DETL_SCN);
			edit (DETL_SCN);
		}

		if (restart)
			continue;

		ProcessTransactions ();

		/*
		 * re-edit tabular if proof total incorrect.
		 */
		while (!journalProofFlag)
		{	
			line_cnt = 0;
			if (STANDING)
				edit_all ();
			else
			{
				heading (DETL_SCN);
				scn_display (DETL_SCN);
				edit (DETL_SCN);
			}
			if (restart)
				break;

			ProcessTransactions ();
		}

		if (restart) 
			continue;

		/*
		 * Create entry on transaction file.
		 */
		if (journalProofFlag && lcount [DETL_SCN] != 0)
		{
			Update ();
			strcpy (batch_no	 [batchCounter], 	glbhRec.batch_no);
			strcpy (batch_type	 [batchCounter++], 	glbhRec.jnl_type);

			if (NEW_RECORD) 
			{
				clear ();
				sprintf (err_str, ML (mlGlMess142), glbhRec.batch_no); 
				print_at (0, 1, "%s\n", err_str);
				print_at (1, 1, "%s\n", ML (mlStdMess042));
                PauseForKey (2, 0, "\x000", 0);
			}
		} 
	} 

	/*
	 * Program exit sequence. 
	 */
	snorm ();
	rset_tty ();

    shutdown_prog ();
    return (EXIT_SUCCESS);
} 

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
	PrintAndProcessList ();
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	open_rec (glsj, glsj_list, GLSJ_NO_FIELDS, "glsj_hhbh_hash");
	open_rec (glzh, glzh_list, GLZH_NO_FIELDS, "glzh_id_no");
	open_rec (glzl, glzl_list, GLZL_NO_FIELDS, "glzl_id_no");
	OpenGljc ();
	OpenGlmr ();
	OpenGlbh ();
	OpenGlbl ();
}

/*
 * Close data base files.
 */
void
CloseDB (
 void)
{
	abc_fclose (glsj);
	abc_fclose (glzh);
	abc_fclose (glzl);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	char 	s_month [3];
	int		redraw_flag;

	if (LCHECK ("eff_from"))
	{
		if (F_HIDE (label ("eff_from")))
			return (EXIT_SUCCESS);

		if (glsj_rec.from_dt <= 0L)
		{
			print_mess (ML ("Start Date must be keyed"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("eff_to"))
	{
		if (F_HIDE (label ("eff_to")))
			return (EXIT_SUCCESS);

		if (glsj_rec.to_dt < glsj_rec.from_dt)
		{
			print_mess (ML ("End Date must be greater than Start Date"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("jnl_type"))
	{
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
			/*
			 * Journal type is not on file
			 */
			print_mess (ML (mlStdMess128));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (!GENERAL && !STANDING && !ACCRUAL && !envGlJnlMaint)
		{
			/*
			 * Cannot manually Add New Journal for this type
			 */
			sprintf (err_str, "Only %s, %s and %s can be maintained", 
						mlGlMess122, mlGlMess123, mlGlMess124); 
			print_mess (ML (err_str));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		print_at (8, 35, "   ");
		if (strlen (clip (gljcRec.jnl_desc)) > 12)
			strcpy (local_rec.jnl_type_desc, gljcRec.jnl_desc);
		else
		{
			sprintf (local_rec.jnl_type_desc, "%s %-7.7s", 
					 clip (gljcRec.jnl_desc), ML ("Journal"));
		}
		DSP_FLD ("jnl_type_desc");
		if (STANDING)
		{
			if (FLD ("tran_mth") == ND)
				redraw_flag = FALSE;
			else
				redraw_flag = TRUE;

			FLD ("tran_mth")	= ND;
			FLD ("day_app")		= YES;
			FLD ("eff_from")	= YES;
			FLD ("eff_to")		= YES;
			FLD ("alloc")  		= NA;
			FLD ("trans_date")	= ND;
		}
		else
		{
			if (FLD ("tran_mth") == NE)
				redraw_flag = FALSE;
			else
				redraw_flag = TRUE;

			FLD ("tran_mth")	= NE;
			FLD ("day_app")		= ND;
			FLD ("eff_from")	= ND;
			FLD ("eff_to")		= ND;
			FLD ("alloc") 		= NE;
			FLD ("trans_date")	= YES;
		}

		fld_reposition (DETL_SCN, (int *) NULL);
		if (redraw_flag)
		{
			heading (HEAD_SCN);
			DSP_FLD ("jnl_type_desc");
		}
		scn_set (HEAD_SCN);
	}

	/*
	 * Validate Batch No.
	 */
	if (LCHECK ("batch_no"))
	{
		if (dflt_used)
		{
			if (!GENERAL && !STANDING && !ACCRUAL)
			{
				/*
				 * Cannot manually Add New Journal for this type
				 */
				print_mess (ML (mlGlMess038)); 
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			flag = 'A';
			strcpy (glbhRec.batch_no, "NEW JNL.  ");
			if (!STANDING)
				FLD ("tran_mth") = YES;
            
			DSP_FLD ("batch_no");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchGlbh (temp_str);
			return (EXIT_FAILURE); 
		} 

		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.br_no, comm_rec.est_no);
		sprintf (glbhRec.jnl_type, "%-2.2s", local_rec.jnl_type);
		batchNumber = atol (glbhRec.batch_no);
		sprintf (glbhRec.batch_no, "%010ld", batchNumber);

		cc = find_rec (glbh, &glbhRec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Batch Number not found!
			 */
			print_mess (ML (mlStdMess153)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (!cc && glbhRec.stat_flag [0] == 'P' && !STANDING)
		{
			/*
			 * This Journal was posted!
			 */
			print_mess (ML (mlGlMess039)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (STANDING)
        {
			LoadGlsj (glbhRec.hhbh_hash);
        }

		DSP_FLD ("day_app");
		DSP_FLD ("eff_from");
		DSP_FLD ("eff_to");

		if (LoadGLBL (glbhRec.hhbh_hash))
		{
			/*
			 * No transaction attached!
			 */
			print_mess (ML (mlGlMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		entry_exit = TRUE;
		DSP_FLD ("tran_mth");
		flag = 'U';
		FLD ("tran_mth") = NA;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Transaction Mth
	 */
	if (LCHECK ("tran_mth"))
	{
		if (STANDING)
			return (EXIT_SUCCESS);

		if (dflt_used)
			glbhRec.mth = controlJournalMonth;

		if (glbhRec.mth < 1 || glbhRec.mth > 12)
		{
			/*
			 * Transaction Month out of range (1 - 12)
			 */
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!priorPostingAllowed)
		{
			sprintf (s_month, "%02d", glbhRec.mth);
			if (strstr (prior_mth, s_month))
			{
				/*
				 * Prior posting is not allowed
				 */
				print_mess (ML (mlGlMess042));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("day_app"))
	{
		if ( (glsj_rec.dt_apply < 0) || 
            (glsj_rec.dt_apply > 28))
        {
			return (EXIT_FAILURE);
        }
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Transaction No.
	 */
	if (LCHECK ("trans_no"))
	{
		if ((dflt_used || last_char  == DELLINE) && prog_status != ENTRY)
		{
			if (GENERAL || STANDING || ACCRUAL)
				return (DeleteLine ());
			
			/*
			 * Can only delete lines from General, Standing or Accrual journals
			 */
			print_mess (ML (mlGlMess043));
			sleep (sleepTime);
		}
		local_rec.line_no = line_cnt + 1;
		DSP_FLD ("trans_no");
		return (EXIT_SUCCESS);
	}

	/*
	 * Check for an allocation code
	 */
	if (LCHECK ("alloc"))
	{
		if (STANDING)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			ALLOC = FALSE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchGlzh (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (glzh_rec.co_no, comm_rec.co_no);
		
		sprintf (glzh_rec.code, "%-4.4s", local_rec.allocation);
		cc = find_rec (glzh, &glzh_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Allocation code is not on file. 
			 */
			print_mess (ML (mlStdMess138)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
        else 
			ALLOC = TRUE;

		/*
		 * Set currency according to glmr account
		 */
		cc = FindPocr (comm_rec.co_no, glzh_rec.currency, "r");
		if (!cc)
		{
			strcpy (local_rec.fxcurr, pocrRec.code);
			local_rec.fxrate = pocrRec.ex1_factor;
		}

		DSP_FLD ("fxrate");
		DSP_FLD ("fxcurr");

		skip_entry	=	goto_field (label ("alloc"), label ("trans_date"));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fxcurr")) 
	{
		/*
		 * Set currency according to glmr account.
		 */
		if (dflt_used)
		{
			cc = FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
			sprintf (local_rec.fxcurr, "%-3.3s", glmrRec.curr_code);
			local_rec.fxrate = (cc) ? 1.00 : pocrRec.ex1_factor;
			DSP_FLD ("fxrate");
			DSP_FLD ("fxcurr");
		}

		cc = FindPocr (comm_rec.co_no, local_rec.fxcurr, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		local_rec.fxrate = (cc) ? 1.00 : pocrRec.ex1_factor;
		
		DSP_FLD ("fxrate");
		DSP_FLD ("fxcurr");

	   local_rec.local_amt = CurrencyLocAmt (local_rec.fxamt);
	   DSP_FLD ("glamt");

	   CalculateJournalTotal ();
	   PrintJournalTotal (TRUE);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fxrate")) 
	{
		if (dflt_used)
		{
			cc = FindPocr (comm_rec.co_no, local_rec.fxcurr, "r");
			local_rec.fxrate = (cc) ? 1.00 : pocrRec.ex1_factor;
			DSP_FLD ("fxcurr");
		}
		/*
		local_rec.fxrate	=	pocrRec.ex1_factor;
		*/
		local_rec.local_amt = CurrencyLocAmt (local_rec.fxamt);
		DSP_FLD ("fxrate");
		DSP_FLD ("glamt");

		CalculateJournalTotal ();
		PrintJournalTotal (TRUE);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate General Ledger Account Number.
	 */
	if (LCHECK ("glacct")) 
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (vars [label ("glacct")].mask [0] == '*')
		{
			/*
			 * Access denied to General Ledger Module.
			 */
			print_mess (ML (mlGlMess001)); 
			restart = TRUE;
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());
			return SearchGlmr_F (comm_rec.co_no, temp_str, lcl_acc_type);
		}

		if (GL_FormAccNo (local_rec.acctNo, glmrRec.acc_no, 0))
			return (EXIT_FAILURE);

		if (prog_status == ENTRY)
		      print_at (0, 85, ML (mlGlMess051), MAXLINES- (line_cnt + 1));

		/*
		 * Account %s Not in General Ledger.
		 */
		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_StripForm (glmrRec.acc_no, local_rec.acctNo);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			return print_err (ML (mlStdMess024));

		if (glmrRec.system_acc [0] == 'S')
		{
			errmess (ML ("System accounts cannot accept manual postings."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [2] [0] != 'P')
       	{
			/*
			 * Account %s Not a Posting Level Account
			 */
			print_err (ML (mlStdMess025));
			return (EXIT_FAILURE);
		}

		if (lcl_acc_type [0] == '*')
        {
			lcl_acc_type [0] = glmrRec.glmr_class [0] [0];
        }
		else
		{
			if (lcl_acc_type [0] != glmrRec.glmr_class [0] [0])
			{
				/*
				 * Sorry, it is Illegal to mix and 	
				 * match G/L A/C types on one journal
				 */
				print_err (	ML (mlGlMess003));
				return (EXIT_FAILURE);
			}
		}

		sprintf (ACCNO (line_cnt), "%-16.16s", glmrRec.acc_no);

		/*
		 * Set currency according to glmr account.
		 */
		cc = FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
		local_rec.fxrate = (cc) ? 1.00 : pocrRec.ex1_factor;
		strcpy (local_rec.fxcurr, (cc) ? " " : pocrRec.code);

		if (prog_status == EDIT)
		{
			local_rec.fxamt = CurrencyFgnAmt (local_rec.local_amt);
			DSP_FLD ("fxamt");
			PrintJournalTotal (TRUE);
		}
		GetAccountDesc ();
		rv_pr (accountDesc, 0, 8, 1);
		local_rec.glmr_hash = glmrRec.hhmr_hash;
		DSP_FLD ("fxrate");
		DSP_FLD ("fxcurr");
		SR.hhmrHash	=	glmrRec.hhmr_hash;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Transaction Date.
	 */
	if (LCHECK ("trans_date"))
	{
		if (STANDING)
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.tran_date = defaultGlDate;

		if (ValidateDate (field))
			return (EXIT_FAILURE);

		if (line_cnt == 0)
			defaultGlDate = local_rec.tran_date;

		return (EXIT_SUCCESS);
	}

	/*
	 * Calculate/reprint run total.
	 */
	if (LCHECK ("d_c"))
	{
		if (prog_status == EDIT)
		{
			CalculateJournalTotal ();
			PrintJournalTotal (FALSE);
		}
	}

	/*
	 * Check input amount - FX or local if defaulted
	 */
	if (LCHECK ("fxamt"))
	{
		if (local_rec.fxamt <= 0)
		{
			/*
			 * Transaction amount must be greater than 0! 
			 */
			print_mess (ML (mlGlMess140)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (ALLOC)
			allocateTotalAmt = local_rec.fxamt;

		cc = FindPocr (comm_rec.co_no, local_rec.fxcurr, "r");
		local_rec.fxrate	=	(cc) ? 1.00 : pocrRec.ex1_factor;
		local_rec.local_amt = 	CurrencyLocAmt (local_rec.fxamt);
		DSP_FLD ("glamt");

		CalculateJournalTotal ();
		PrintJournalTotal (TRUE);
	}

	/*
	 * Update reference.
	 */
	if (LCHECK ("userReference")) 
	{
		if (end_input) 
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.userReference, userReference);
			DSP_FLD ("userReference");
		}

		if (prog_status == ENTRY)
			strcpy (userReference, local_rec.userReference);
		
		return (EXIT_SUCCESS);
	}

	/*
	 * Narrative
	 */
	if (LCHECK ("desc")) 
	{
		if (end_input) 
        {
			return (EXIT_SUCCESS);
        }

		if (dflt_used)
		{
			sprintf (local_rec.narrative, "%-20.20s", narrative);
			DSP_FLD ("desc");
		}
		if (prog_status == ENTRY)
			strcpy (narrative, local_rec.narrative);

		if (ALLOC && 
            (prog_status == ENTRY) && !STANDING)
		{
			ProcessAllocation (local_rec.allocation, allocateTotalAmt);
			PrintJournalTotal (FALSE);
			ALLOC = FALSE;
			if (in_edit)
				lcount [DETL_SCN] = lcount [DETL_SCN] - 1;
            
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Load glsj
 */
void
LoadGlsj (
	long	hhbhHash) /* unreferenced formal parameter */
{
	glsj_rec.hhbh_hash = hhbhHash;

    cc = find_rec (glsj, &glsj_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, glsj, "DBFIND");
}

/*
 * Load Batch Line Details.
 */
int
LoadGLBL (
	long	hhbhHash)
{
	int		len;
	char	temp_accno [MAXLEVEL + 1];

	scn_set (DETL_SCN);
	line_cnt = 0;
	lcount [DETL_SCN] = 0;
	print_at (2, 0, ML (mlStdMess035)); 

	glblRec.hhbh_hash 	= hhbhHash;
	glblRec.line_no 	= 0;
	
	cc = find_rec (glbl, &glblRec, GTEQ, "r");
	defaultGlDate = local_rec.tran_date;
	while (!cc && (glblRec.hhbh_hash == hhbhHash))
	{
		local_rec.line_no = lcount [DETL_SCN] + 1;
		strcpy (local_rec.allocation, "");
		strcpy (local_rec.acctNo, glblRec.acc_no);
		GL_FormAccNo (local_rec.acctNo, temp_accno, 0);
		if (!strncmp (glblRec.acc_no, "                ", 16))
		{
			if (!GENERAL && !STANDING && !ACCRUAL)
			{
				cc = find_rec (glbl, &glblRec, NEXT, "r");
				continue;
			}
			len = strlen (local_rec.acctNo);
			sprintf (local_rec.acctNo, "%*.*s", len, len, " ");
			sprintf (ACCNO (line_cnt), "                ");
		}
		else
			sprintf (ACCNO (line_cnt), "-%16.16s", glblRec.acc_no);

		strcpy (glmrRec.co_no,  comm_rec.co_no);
		strcpy (glmrRec.acc_no, glblRec.acc_no);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			glmrRec.hhmr_hash = 0;

		SR.hhmrHash	=	glmrRec.hhmr_hash;
		local_rec.glmr_hash = glmrRec.hhmr_hash;
		local_rec.tran_date = glblRec.tran_date;
		local_rec.fxamt 	= glblRec.fx_amt;
		local_rec.fxrate 	= glblRec.exch_rate;
		local_rec.local_amt = glblRec.local_amt;
		strcpy (local_rec.dc_flag, (atoi (glblRec.dc_flag) % 2 ==0) ? "C": "D");
		strcpy (local_rec.fxcurr, glblRec.currency);
		strcpy (local_rec.userReference, glblRec.user_ref);
		strcpy (local_rec.narrative, glblRec.narrative);
		strcpy (SR.desc [0], glblRec.alt_desc1);
		strcpy (SR.desc [1], glblRec.alt_desc2);
		strcpy (SR.desc [2], glblRec.alt_desc3);

		CalculateJournalTotal ();
		putval (lcount [DETL_SCN]++);
		line_cnt++;
		cc = find_rec (glbl, &glblRec, NEXT, "r");
	}
	if (lcount [DETL_SCN]==0)
		return (TRUE);

	/*
	 * Don't allow lines to be added for system generated journals.
	 */
	if (!GENERAL && !STANDING && !ACCRUAL)
		vars [scn_start].row = lcount [2];

	line_cnt = 0;
	scn_set (HEAD_SCN);
	return (FALSE);
}

/*
 * Print Journal proof total. 
 */
void
PrintJournalTotal (
	int printAmountOnly)
{
	int	i;
	int upper;
	double	total  = 0.00, 
			d_tot  = 0.00, 
			c_tot  = 0.00;

	upper = ( (lcount [DETL_SCN]>line_cnt + 1) ? lcount [DETL_SCN] : line_cnt + 1);

	for (i = 0; i < upper; i++)
	{
		if (strncmp (ACCNO (i), "                ", 16))
		{
			d_tot += no_dec (D_TOT (i)); 
			c_tot += no_dec (C_TOT (i)); 
		}
	} 
	total =  c_tot - d_tot;

    if (!printAmountOnly)
    {
	    move (68, 3); cl_line (); 
	    move (68, 4); cl_line (); 
	    move (68, 5); cl_line (); 
	    move (68, 6); cl_line (); 
	    move (68, 7); cl_line (); 

		box (0, 2, 70, 4);
	    box (75, 2, 50, 4); 

	    print_at (3, 77, ML (mlGlMess041), baseCurr); 
	    print_at (4, 77, ML (mlGlMess044), baseCurr);
	    print_at (5, 77, ML (mlGlMess050), baseCurr);

    }
	sprintf (err_str, "$%.2f", DOLLARS (total));
	print_at (3, 100, "%-12.12s", err_str);

	sprintf (err_str, "$%.2f", DOLLARS (d_tot));
	print_at (4, 100, "%-12.12s", err_str);

	sprintf (err_str, "$%.2f", DOLLARS (c_tot));
	print_at (5, 100, "%-12.12s", err_str);
}

void
CalculateJournalTotal (void)
{
	D_TOT (line_cnt)  = (CREDIT) ? 0.00 : local_rec.local_amt;
	C_TOT (line_cnt)  = (CREDIT) ? local_rec.local_amt : 0.00;
}

int
ValidateDate (
 int field) /* unreferenced formal parameter */
{	
	long	beg_d	=	0L;
	int		mth		=	0,
			tr_year	=	0, 
			year	=	0;

	DateToDMY (local_rec.tran_date, NULL, &mth, NULL);
	if (mth != glbhRec.mth && (GENERAL || STANDING || ACCRUAL))
	{
		/*
		 * Invalid Transaction Month
		 */
		print_mess (ML (mlStdMess195));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	beg_d = MonthStart (comm_rec.gl_date);

	if (priorPostingAllowed)
		beg_d = FinYearStart (comr_rec.yend_date, comm_rec.fiscal);

	DateToDMY (beg_d, NULL, NULL, &year);
	DateToDMY (local_rec.tran_date, NULL, NULL, &tr_year);

	/*
	 * Date before Cutoff Date %s
	 */
	if (local_rec.tran_date < beg_d )
    {
		return print_err (ML (mlGlMess141), DateToString (beg_d));
    }

	return (EXIT_SUCCESS);
}

int 
DeleteLine (void)
{
	int		i;
	int		cur_line	=	line_cnt;

	if (prog_status == ENTRY)
	{
		/*
		 * Cannot Delete Lines On Entry
		 */
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	if (lcount [DETL_SCN] < 1)
	{
		/*
		 * No Lines to Delete 
		 */
		print_mess (ML (mlStdMess032)); 
		return (EXIT_FAILURE);
	}

	lcount [DETL_SCN]--;
	for (i = line_cnt; i < lcount [DETL_SCN]; i++)
	{
		getval (i + 1);
		local_rec.line_no = i + 1;
		putval (i);

		D_TOT (i) = D_TOT (i+1);
		C_TOT (i) = C_TOT (i+1);
		strcpy (ACCNO (i), ACCNO (i+1));
	}
	D_TOT (lcount [DETL_SCN]) = 0;
	C_TOT (lcount [DETL_SCN]) = 0;

	i = line_cnt % TABLINES;
	line_cnt -= i;

	for (i = 0; i < TABLINES; i++, line_cnt++)
	{
		getval (line_cnt);
		if (line_cnt < lcount [DETL_SCN])
			line_display ();
		else
			blank_display ();
	}
	line_cnt = cur_line;
	PrintJournalTotal (TRUE);
	print_at (0, 85, ML (mlGlMess051), MAXLINES-lcount [DETL_SCN]);
	getval (line_cnt);

    return (EXIT_SUCCESS);
}

int
ProcessTransactions (
 void)
{
	int	i;
	double	checkTotal 	= 0.00,
			totalCredit = 0.00,
			totalDebit 	= 0.00;
	
	for (i = 0; i < lcount [DETL_SCN]; i++)
	{
		if (strncmp (ACCNO (i), "                ", 16))
		{
			totalCredit += no_dec (C_TOT (i));
			totalDebit 	+= no_dec (D_TOT (i));
		}
	}
	checkTotal = totalCredit - totalDebit;

	if (totalCredit == totalDebit)
	{
		journalProofFlag = TRUE;
		return (EXIT_SUCCESS);
	}

	journalProofFlag = FALSE;
	/*
	 * Journal total = %lf , Must Equal 0 To Update
	 */
	sprintf (err_str, ML (mlGlMess037), DOLLARS (checkTotal));
	errmess (err_str);
	sleep (sleepTime);
	return (EXIT_FAILURE);
}

void
Update (void)
{
	ProcBatchHeader ();
	ProcBatchDetail ();
}

void
ProcBatchHeader (void)
{
	scn_set (HEAD_SCN);

	if (NEW_RECORD)
	{
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.br_no, comm_rec.est_no);
		sprintf (glbhRec.jnl_type, "%-2.2s", local_rec.jnl_type);

		strcpy (gljcRec.co_no, comm_rec.co_no);
		sprintf (gljcRec.journ_type, "%-2.2s", local_rec.jnl_type);

        cc = find_rec (gljc, &gljcRec, COMPARISON, "u");
		if (cc)
			file_err (cc, gljc, "DBFIND");

		sprintf (glbhRec.batch_no, "%010ld", gljcRec.run_no++);

		while (CheckBatchTotal (glbhRec.batch_no) == 0)
            sprintf (glbhRec.batch_no, "%010ld", gljcRec.run_no++);
			
		sprintf (glbhRec.user, "%-8.8s", getenv ("LOGNAME")); 
		glbhRec.glbh_date = TodaysDate ();
		strcpy (glbhRec.glbh_time, TimeHHMM ());
		strcpy (glbhRec.stat_flag, "N"); 
		glbhRec.other_module = FALSE;

		if (STANDING)
			glbhRec.mth = 0;

		cc = abc_add (glbh, &glbhRec);
		if (cc)
            file_err (cc, glbh, "DBADD");

		cc = find_rec (glbh, &glbhRec, COMPARISON, "r");
		if (cc)
            file_err (cc, glbh, "DBFIND1"); 

		cc = abc_update (gljc, &gljcRec);
		if (cc)
            file_err (cc, gljc, "DBUPDATE");
	}

	if (STANDING)
	{
		glsj_rec.hhbh_hash = glbhRec.hhbh_hash;
		glsj_rec.dt_posted = 0;
		if (NEW_RECORD)
		{
			cc = abc_add (glsj, &glsj_rec);
			if (cc)
				file_err (cc, glsj, "DBADD");
		}
		else
		{
			memcpy 
			( 
				(char *) &glsj2_rec, 
				(char *) &glsj_rec, 	
				sizeof (struct glsjRecord)
			);

            cc = find_rec (glsj, &glsj_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, glsj, "DBFIND");

			memcpy 
			(
				(char *) &glsj_rec, 
				(char *) &glsj2_rec, 	
				sizeof (struct glsjRecord)
			);

			cc = abc_update (glsj, &glsj_rec);
			if (cc)
				file_err (cc, glsj, "DBUPDATE");
		}
	}
	return;
}

int
CheckBatchTotal (
	char *batchNumber)
{
	strcpy (glbh2Rec.co_no, comm_rec.co_no);
	strcpy (glbh2Rec.br_no, comm_rec.est_no);
	sprintf (glbh2Rec.jnl_type, "%-2.2s", local_rec.jnl_type);
	sprintf (glbh2Rec.batch_no, "%-10.10s",  batchNumber);
	return (find_rec (glbh, &glbh2Rec, COMPARISON, "r"));
}

void
ProcBatchDetail (void)
{
	int i;

    scn_set (DETL_SCN);
	for (i = 0; i < lcount [DETL_SCN];i++)
		UpdateDetailLine (i);
	
	if (GENERAL || STANDING || ACCRUAL || envGlJnlMaint)
	{
		/*
	 	 *	Delete trailing glbl records
	 	 */
		glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
		glblRec.line_no 	= 0;
		cc = find_rec (glbl, &glblRec, GTEQ, "u");
		while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
		{
			cc = abc_delete (glbl);
			if (cc) 
				file_err (cc, glbl, "DBDELETE");

			cc = find_rec (glbl, &glblRec, NEXT, "u");
		}
		abc_unlock (glbl);

		/*
		 *	Resequence line numbers
		 *
		 *	Flip the numbers back to +ve.
		 */

		glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
		glblRec.line_no 	= -1;
		cc = find_rec (glbl, &glblRec, LTEQ, "u");
		while (!cc && glblRec.hhbh_hash == glbhRec.hhbh_hash)
		{
			glblRec.line_no = -glblRec.line_no - 1;
			if ((cc = abc_update (glbl, &glblRec)))
				file_err (cc, glbl, "abc_update");

			glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
			glblRec.line_no 	= -1;
			cc = find_rec (glbl, &glblRec, LTEQ, "u");
		}
		abc_unlock (glbl);
	}
}

void
AddDetailLine (
	int		lineNo)
{
	glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
	glblRec.line_no 	= lineNo;
	if (!strncmp (ACCNO (lineNo), "                ", 16))
		sprintf (glblRec.acc_no, "                ");
    
	else
		GL_StripForm (glblRec.acc_no, local_rec.acctNo);
    
	sprintf (glblRec.acronym, "%9.9s", " ");
	sprintf (glblRec.name, "%30.30s", " ");
	sprintf (glblRec.chq_inv_no, "%15.15s", " ");
	glblRec.ci_amt = 0;
	glblRec.o1_amt = 0;
	glblRec.o2_amt = 0;
	glblRec.o3_amt = 0;
	glblRec.o4_amt = 0;
	glblRec.hhgl_hash = local_rec.glmr_hash;
	sprintf (glblRec.tran_type, "%-2.2s", local_rec.jnl_type);
	sprintf (glblRec.sys_ref, "%010ld", (long) comm_rec.term);
	if (STANDING)
	{
		glblRec.tran_date = 0;
		sprintf (glblRec.period_no, " 0");
	}
	else
	{
		glblRec.tran_date = local_rec.tran_date;
		DateToDMY (local_rec.tran_date, NULL, &monthPeriod, NULL);
		sprintf (glblRec.period_no, "%2d", monthPeriod);
	}
	strcpy (glblRec.narrative, local_rec.narrative);
	sprintf (glblRec.alt_desc1, store [lineNo].desc [0]);
	sprintf (glblRec.alt_desc2, store [lineNo].desc [1]);
	sprintf (glblRec.alt_desc3, store [lineNo].desc [2]);
	strcpy (glblRec.user_ref, local_rec.userReference);
	glblRec.fx_amt		= local_rec.fxamt;
	glblRec.local_amt 	= local_rec.local_amt;
	strcpy (glblRec.dc_flag, (DEBIT) ? "1" : "2");
	strcpy (glblRec.currency, local_rec.fxcurr);
	glblRec.exch_rate 	= local_rec.fxrate;
	strcpy (glblRec.stat_flag, "N");

    cc = abc_add (glbl, &glblRec);
	if (cc)
    {
		file_err (cc, glbl, "DBADD");
    }

	return;
}

void
UpdateDetailLine (
	int		lineNo)
{
	newLineNo	=	-lineNo - 1;

	getval (lineNo);

	glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
	glblRec.line_no		= (glblRec.line_no) ? glblRec.line_no : lineNo;

	cc = find_rec (glbl, &glblRec, COMPARISON, "u");
	if (cc)
	{
		getval (lineNo);

		if (GENERAL || STANDING || ACCRUAL || envGlJnlMaint)
			AddDetailLine (newLineNo);
		else
			AddDetailLine (lineNo);

		return;
	}
	GL_StripForm (glblRec.acc_no, local_rec.acctNo);
	glblRec.hhgl_hash = local_rec.glmr_hash;
	if (!STANDING)
	{
		glblRec.tran_date = local_rec.tran_date;
		DateToDMY (local_rec.tran_date, NULL, &monthPeriod, NULL);
		sprintf (glblRec.period_no, "%2d", monthPeriod);
	}
	strcpy (glblRec.narrative, local_rec.narrative);
	strcpy (glblRec.user_ref, local_rec.userReference);
	glblRec.fx_amt 		= local_rec.fxamt;
	glblRec.local_amt 	= local_rec.local_amt;
	glblRec.exch_rate 	= local_rec.fxrate;
	strcpy (glblRec.dc_flag, (DEBIT) ? "1" : "2");
	strcpy (glblRec.currency, local_rec.fxcurr);
	glblRec.line_no		= newLineNo;

    cc = abc_update (glbl, &glblRec);
	if (cc)
		file_err (cc, glbl, "DBUPDATE");

	return;
}

void
PrintAndProcessList (void)
{
	int	i;

	clear ();
	for (i = 0; i < batchCounter; i++)
	{
		sprintf (err_str, "gl_batch_list \"%s\" \"%s\" \"%s\"", 
							local_rec.lpno, 
							batch_type [i], 
							batch_no [i]);
		cc = sys_exec (err_str);
		if (cc)
			file_err (cc, "gl_batch_list", "sys_exec");
	}
}

int
heading (
 int scn)
{
	int	s_size = 132;

	if (restart) 
		return (EXIT_SUCCESS); 
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	sprintf (err_str, " %s ", ML (mlGlMess045));
	rv_pr (err_str, 55, 0, 1);

	line_at (1,0,s_size);

	if (scn == HEAD_SCN || scn == DETL_SCN)
		box (0, 2, 70, 4);

	line_at (21,0,s_size);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	if (scn == HEAD_SCN || scn == DETL_SCN)
	{
	    PrintJournalTotal (FALSE);

		/*  reset this variable for new screen NOT page	*/
		box (0, 2, 70, 4);
	}
	if (scn == DETL_SCN)
	{
		scn_set (HEAD_SCN);
		scn_write (HEAD_SCN);
		scn_display (HEAD_SCN);
	}
	if (scn == HEAD_SCN)
	{
		rv_pr (ML ("Alt. Window #1 Active"), 110, 8, 1);
		scn_set (DETL_SCN);
		scn_write (DETL_SCN);
		if (in_edit)
			scn_display (DETL_SCN);
	}

	if (scn != cur_screen)
		scn_set (scn);
    
	scn_write (scn);

    return (EXIT_SUCCESS);
}

void
SrchGljc (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Journal Description");
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, key_val);

    cc = find_rec (gljc, &gljcRec, GTEQ, "r");
	while (!cc && 
           !strcmp (gljcRec.co_no, comm_rec.co_no) &&
           !strncmp (gljcRec.journ_type, key_val, strlen (key_val)))
	{
		strcpy (local_rec.jnl_type, gljcRec.journ_type);

        cc = save_rec (gljcRec.journ_type, gljcRec.jnl_desc);
		if (cc)
			break;
        
		cc = find_rec (gljc, &gljcRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

    if (cc)
    {
		return;
    }
	strcpy (local_rec.jnl_type, temp_str);
}

void
tab_other (
 int line_no)
{
	move  (0,8); cl_line ();
	if (line_no >= lcount [DETL_SCN])
	{
		sprintf (accountDesc, ML (mlStdMess087), " ");
		rv_pr (accountDesc, 0, 8, 1);
		return;
	}

	if (prog_status == ENTRY)
	{
		if (!STANDING)
			FLD ("trans_date") = YES;
        
		FLD ("trans_no")		= YES;
		FLD ("glacct")			= YES;
		FLD ("d_c")				= YES;
		FLD ("fxamt")			= YES;
		FLD ("userReference")	= NO;
		FLD ("desc")			= NO;
		return;
	}

	getval (line_no);
	strcpy (glmrRec.co_no, comm_rec.co_no);
	GL_FormAccNo (local_rec.acctNo, glmrRec.acc_no, 0);

    cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		rv_pr (ML (mlGlMess046), 0, 8, 1);
		if (!STANDING)
			FLD ("trans_date") = NE;
        
		FLD ("trans_no")		= NE;
		FLD ("glacct")			= NE;
		FLD ("d_c")				= NE;
		FLD ("fxamt")			= NE;
		FLD ("userReference")	= NE;
		FLD ("desc")			= NE;
	}
	else
	{
		GetAccountDesc ();
		rv_pr (accountDesc, 0, 8, 1);
		if (!STANDING)
        {
			FLD ("trans_date") = YES;
        }
		FLD ("trans_no")		= YES;
		FLD ("glacct")			= YES;
		FLD ("d_c")				= YES;
		FLD ("fxamt")			= YES;
		FLD ("userReference")	= NO;
		FLD ("desc")			= NO;
	}
	rv_pr (ML ("Alt. Window #1 Active"), 110, 8, 1);
}

void
GetAccountDesc (void)
{
	int		i;
	int		firstTime	=	TRUE;
	char	workString [256];

	move (0,8);cl_line ();
	GL_CheckAccNo (TRUE, glmrRec.acc_no, &glmrRec);

	for (i = 0; i < GV_max_level; i++)
	{
		GL_GetDesc (i, err_str, 25);	
		if (strlen (clip (err_str)))
		{
			if (firstTime == FALSE)
			{
				strcat (workString, "->");
				strcat (workString, "(");
			}
			else
				strcpy (workString, "(");
			firstTime	=	FALSE;
			strcat (workString, err_str);
			strcat (workString, ")");
		}
	}
	sprintf (accountDesc, ML (mlStdMess087), workString);
}
/*
 * Read comr, get base currency information
 */
int
ReadComr (void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	sprintf (comr_rec.co_no, "%-2.2s", comm_rec.co_no);

    cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
	{
		/*
		 * No company set up in comr
		 */
		print_mess (ML (mlStdMess130));
		sleep (sleepTime);

        return (EXIT_FAILURE);
	}
	cc = FindPocr (comm_rec.co_no, comr_rec.base_curr, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");

	strcpy (baseCurr, pocrRec.code);
	abc_fclose (comr);

    return (EXIT_SUCCESS);
}

/*
 * Specific code to handle GL allocations.
 */
void
ProcessAllocation (
	char  	*alloc_code, 
	double 	amount) /* unreferenced formal parameter */
{
	int		i;
	int		this_page;
	int		old_line_cnt = line_cnt;

	this_page = line_cnt / TABLINES;

	strcpy (glzh_rec.co_no, comm_rec.co_no);
	sprintf (glzh_rec.code, "%-4.4s", alloc_code);

    cc = find_rec (glzh, &glzh_rec, COMPARISON, "r");
	if (cc)
	{
		/*
		 * Allocation code is not on file.
		 */
		print_mess (ML (mlStdMess138));
		sleep (sleepTime);
		return;
	}

	glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
	strcpy (glzl_rec.gl_acc_no, "                ");
	abc_selfield (glmr, "glmr_hhmr_hash");

    cc = find_rec (glzl, &glzl_rec, GTEQ, "r");
	while (!cc && glzh_rec.hhbh_hash == glzl_rec.hhbh_hash)
	{
		glmrRec.hhmr_hash	=	glzl_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (glzl, &glzl_rec, NEXT, "r");
			continue;
		}

		cc = FindPocr (comm_rec.co_no, local_rec.fxcurr, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		strcpy (local_rec.acctNo, glzl_rec.gl_acc_no);
		GL_FormAccNo (local_rec.acctNo, glzl_rec.gl_acc_no, 0);
		local_rec.fxamt		= (allocateTotalAmt * glzl_rec.percentage / 100);
		local_rec.local_amt = CurrencyLocAmt (local_rec.fxamt);
		local_rec.glmr_hash = glmrRec.hhmr_hash;
		local_rec.line_no 	= line_cnt + 1;
		putval (line_cnt);
		CalculateJournalTotal ();
		line_cnt++;
		cc = find_rec (glzl, &glzl_rec, NEXT, "r");
	}

	lcount [DETL_SCN] = line_cnt;

	if (in_edit)
	{
		for (i = old_line_cnt % TABLINES, line_cnt = old_line_cnt; 
			 i < TABLINES; 
             i++, line_cnt++)
		{
			getval (line_cnt);
			if (line_cnt < lcount [DETL_SCN])
				line_display ();
			else
				blank_display ();
		}
		line_cnt = old_line_cnt;
	}
	else
	{
		i = line_cnt % TABLINES;
		line_cnt -= i;

		for (i = 0; i < TABLINES; i++, line_cnt++)
		{
			getval (line_cnt);
			if (line_cnt < lcount [DETL_SCN])
                line_display ();
			else
				blank_display ();
		}
		line_cnt = lcount [DETL_SCN] - 1;
	}

	abc_selfield (glmr, "glmr_id_no");
	getval (line_cnt);
}

/*
 * Search for Allocation Codes.
 */
void
SrchGlzh (
	char *key_val)
{
	_work_open (4, 0, 40);
	save_rec ("#Code", "# Description ");

	strcpy (glzh_rec.co_no, comm_rec.co_no);
	sprintf (glzh_rec.code, "%-4.4s", key_val);

    cc = find_rec (glzh, &glzh_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (glzh_rec.co_no, comm_rec.co_no) &&
           !strncmp (glzh_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (glzh_rec.code, glzh_rec.description);
		if (cc)
			break;

		cc = find_rec (glzh, &glzh_rec, NEXT, "r");
	}

    cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (glzh_rec.code, "%-4.4s", temp_str);
}

/*
 * Search for General Ledger Batch Header.
 */
void
SrchGlbh (
 char *key_val)
{
	char    desc [31];

	_work_open (10,0,30);
	save_rec ("#Batch no. ", "#Date     Time  ");

	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	sprintf (glbhRec.jnl_type, "%-2.2s", local_rec.jnl_type);
	strcpy (glbhRec.batch_no, key_val);

	cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	while (	!cc && 
			!strcmp (glbhRec.co_no, comm_rec.co_no) &&
			!strcmp (glbhRec.br_no, comm_rec.est_no) &&
			!strcmp (glbhRec.jnl_type, local_rec.jnl_type))
	{
		if (glbhRec.stat_flag [0] == 'P' && !STANDING)
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}
		sprintf (desc, "%s %-5.5s", DateToString (glbhRec.glbh_date), glbhRec.glbh_time);
        cc = save_rec (glbhRec.batch_no, desc);
		if (cc)
            break;

		cc = find_rec (glbh, &glbhRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (glbhRec.batch_no, temp_str);
}


/*
 * Input not stock description lines for non-stock products.
 */
void	
InputGlDesc (
	int		line_cnt)
{
	int 	i, 
			tx_window;
	char	disp_str [200];

	int		offSet;

	offSet = ((tab_row + (line_cnt % TABLINES)) > 15) 
						? tab_row + (line_cnt % TABLINES) - 3
					 	: tab_row + (line_cnt % TABLINES) + 3;
		
	strcpy (err_str, ML ("Additional Description"));

	tx_window	=	txt_open 
					(
						offSet, 
						89, 
						3, 
						40, 
						MAX_NARR, 
						err_str
					);

	for (i = 0; i < MAX_NARR ; i++)
	{
		sprintf (disp_str, "%-20.20s", SR.desc [i]);
		txt_pval (tx_window, disp_str, 0);
	}

	txt_edit (tx_window);

	for (i = 0; i < MAX_NARR ; i++)
		sprintf (SR.desc [i], "%-20.20s", txt_gval (tx_window, i + 1));

	txt_close (tx_window, FALSE);
}
int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (store [lin].hhmrHash == 0L)
	{
		putchar (BELL);
		return (FALSE);
	}
	InputGlDesc (lin); 
	restart = FALSE;
	return (PSLW);
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: batch_disp.c,v 5.11 2002/07/24 08:38:53 scott Exp $
|  Program Name  : (gl_batch_disp.c)
|  Program Desc  : (General Ledger Batch Journal Enquiry Program)
|---------------------------------------------------------------------|
| $Log: batch_disp.c,v $
| Revision 5.11  2002/07/24 08:38:53  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.10  2002/06/26 05:08:35  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.9  2002/02/04 02:41:22  scott
| Updated to ensure clear_mess used.
|
| Revision 5.8  2001/11/09 05:41:01  scott
| Updated to fix minor display issue.
|
| Revision 5.7  2001/08/24 06:19:45  scott
| Updated to that blank lines are displayed.
|
| Revision 5.6  2001/08/09 09:13:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.5  2001/08/06 23:27:08  scott
| RELEASE 5.0
|
| Revision 5.4  2001/07/25 02:17:33  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: batch_disp.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_batch_disp/batch_disp.c,v 5.11 2002/07/24 08:38:53 scott Exp $";

/*
 *   Include file dependencies  
 */
#define     MAXSCNS		3		
#define     MAXLINES	5000	
#include <pslscr.h>	
#include <hot_keys.h>
#include <GlUtils.h>	
#include <twodec.h>	
#include <ml_std_mess.h>	
#include <ml_gl_mess.h>
#include <tabdisp.h>

/*
 *   Constants, defines and stuff   
 */
#define		CREDIT		(local_rec.dcFlag [0] == 'C') 
#define		DEBIT		(local_rec.dcFlag [0] == 'D')

#define     STANDING    	(!strcmp (local_rec.journalType, " 2"))
#define     IS_STANDING(x)  (!strcmp (x, " 2"))

#define		D_TOT(a)	(store [a].debitValue)
#define		C_TOT(a)	(store [a].creditValue)
#define		ACCNO(a)	(store [a].accno)

#define     TOTSCNS		3		/*  the number of screens used (incl lin.)*/
#define		WK_DEPTH	12

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glsjRecord	glsj_rec;

	char	*glbhWork		=	"glbhWork",
			*data			=	"data",
			*sixteenSpaces	=	"                ";

	/*
	 * Special fields and flags  ##################################.
	 */
	char	baseCurr 			[4],
			baseCurrDesc 		[21];

	struct	storeRec
	{
		char	accno  [sizeof glblRec.acc_no];
		double	debitValue;
		double	creditValue;
	} store [MAXLINES];

/*
 * Local & Screen Structures.
 */
struct
{
	char	journalType	[4];
	char	journalDesc	[41];
	char	dummy 		[11];
	char	refer 		[sizeof glblRec.user_ref];
	int		lineNo;
	char	dcFlag 		[2];
	double	locAmount;
	long	tranDate;
	char	acctNo 		[FORM_LEN + 1];
	char	narrative 	[sizeof glblRec.narrative];
	char	tran_type 	[3];
	char	stat_flag 	[2];
	double	fgnAmount;
	double	fxrate;
	double	pass_amt;
	char	fxcurr 		[4];
	char	allocation 	[5];
	int	mth;
} local_rec;

/*
 *   Local function prototypes  
 */
int		workView 		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	tab_keys [] =
{
	{ " View Batch ", 	'\r',		workView,
		"View the batch"								},
	END_KEYS
};
#else
static	KEY_TAB	tab_keys [] =
{
	{ "RTN", 	'\r',		workView,
		"View the batch"								},
	END_KEYS
};
#endif

static	struct	var	vars [] =
{ 
	{1, LIN, "journalType", 3, 20, CHARTYPE,
		"AAA", "         ",
		" ", "ALL",        "Journal Type    :", " ",
		YES, NO,  JUSTRIGHT, "", "", local_rec.journalType},
	{1, LIN, "journalDesc", 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "         ",
		"", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.journalDesc},
	{2, LIN, "batch_no", 3, 15, CHARTYPE, 
		"AAAAAAAAAA", "         ",
		" ", " ", " Batch No. : ", " ",
		NA, NO, JUSTLEFT, "", "", glbhRec.batch_no},
	{2, LIN, "batch_type_desc", 3, 29, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "         ",
		" ", " ", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.journalDesc},
	{2, LIN, "tran_mth", 4, 15, INTTYPE,
		"NN", "               ",
		" ", " ", " Tran Mth  : ", "Range : 1 to 12 ",
		NA, NO, JUSTRIGHT, "", "", (char *)&glbhRec.mth},
	{2, LIN, "dayApplied", 4, 15, INTTYPE,
		"NN", "                  ",
		"", "", " Day apply : ", "Enter day apply for standing post journal. ",
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.dt_apply}, 
	{2, LIN, "eff_from", 5, 15, EDATETYPE,
		"DD/DD/DD", "                  ",
		"", "", " Eff. From : ", "Enter effective from date for standing post journal. ",
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.from_dt}, 
	{2, LIN, "eff_to", 5, 43, EDATETYPE,
		"DD/DD/DD", "                  ",
		" ", "", " Eff. To   : ", "Enter effective to date for standing post journal. ",
		ND, NO, JUSTRIGHT, "", "", (char *)&glsj_rec.to_dt}, 
	{3, TAB,	"trans_no", MAXLINES, 0, INTTYPE,
		"NNNN", "            ",
		" ", " ", "Line", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.lineNo},
	{3, TAB, "glacct", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ", "", "Account      ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.acctNo},
	{3, TAB, "trans_date", 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Tran Date ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.tranDate},
	{3, TAB, "d_c",	 0, 1, CHARTYPE,
		"U", "            ",
		" ", "", "DC", "Must Be D(ebit) or C(redit). ",
		NA, NO,  JUSTLEFT, "DC", "", local_rec.dcFlag},
	{3, TAB, "fxcurr", 0, 0, CHARTYPE,
		"UUU", "               ",
		" ", " ", "Cur", "Enter currency for journal, [SEARCH].",
		NA, NO, JUSTLEFT, "", "", local_rec.fxcurr},
	{3, TAB, "fgnAmount", 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "            ",
		" ", "0", "Amount    ", " ",
		NA, NO, JUSTRIGHT, "0", "9999999999", (char *) &local_rec.fgnAmount},
	{3, TAB, "fxrate", 0, 0, DOUBLETYPE,
		"NNNN.NNNN", "            ",
		" ", " ", " Rate   ", "Enter currency for journal, [SEARCH].",
		NA, NO, JUSTLEFT, "", "", (char *) &local_rec.fxrate},
	{3, TAB, "glamt", 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "            ",
		" ", " ", " Local Amt   ", " ",
		NA, NO, JUSTRIGHT, "0", "9999999999", (char *) &local_rec.locAmount},
	{3, TAB, "refer", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "        ",
		" ", "", "User  Reference", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.refer},
	{3, TAB, "desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "      Narrative       ", " ",
		NA, YES, JUSTLEFT, "", "", local_rec.narrative}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 *   Local variables  
 */
int	 	dmy [3];
int	 	gdmy [3];
long  	lsystemDate;
char  	systemDate [11];
int   	day1;
int   	dayApplied;
int		envVarGlBatchBranch = 1;
int		tableOpened = FALSE;

extern int  tab_max_page;
extern int  GV_fiscal;

int  	spec_valid 			(int);
int  	LoadLine 			(long);
int  	heading 			(int);
int  	ReadComr 			(void);
int  	LoadBatch 			(void);
int  	LoadOneJnlType 		(char *);
void 	tab_other 			(int);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	PrintJournalTotal 	(int);
void 	CalcJournalTotal 	(void);
void 	SrchGljc 			(char *);
void 	DisplayBatchDetails (char *, char *);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	char	*sptr;

	if (argc < 1 || argc > 3)
	{
		print_at (0,0,mlGlMess707,argv [0]);
		return (EXIT_SUCCESS);
	}

	sptr = chk_env ("GL_BATCH_BRANCH");
	envVarGlBatchBranch = (sptr == (char *)0) ? 1 : atoi (sptr);

	tab_max_page = 1000;
	OpenDB (); 

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);

	tab_col = 1;
	tab_row = 9;

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 
	DateToDMY (comm_rec.gl_date, &gdmy [0], &gdmy [1], &gdmy [2]);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	GV_fiscal = comm_rec.fiscal;

	if (ReadComr () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

	vars [label ("glacct")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);

	init_scr ();
	set_tty ();
	swide ();
	set_masks ();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	init_ok = TRUE;
	init_vars (1);
	init_vars (2);
	init_vars (3);

	if (argc == 3)
	{
		entry_exit 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		search_ok 		= TRUE;
        init_ok 		= TRUE;
		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [3] = 0;
		line_cnt = 0;
		DisplayBatchDetails (argv [1], argv [2]);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	while (!prog_exit) 
	{ 
		entry_exit = edit_exit = prog_exit = restart = 0;
		search_ok = 1;
        init_ok = TRUE;
		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [3] = 0;
		line_cnt = 0;

		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);

		if (last_char == FN16)
			break;

		if (prog_exit || restart) 
			continue;

		if (restart)
			continue;

		heading (1);
		if (LoadBatch ())
			tab_scan (glbhWork);
		else
		{
			print_mess (ML (mlGlMess064));
			sleep (sleepTime);
			clear_mess ();
		}
		tab_close (glbhWork, TRUE);
		tableOpened = FALSE;
	} 

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
	open_rec (glsj, glsj_list, GLSJ_NO_FIELDS, "glsj_hhbh_hash");

	OpenGlmr ();
	OpenGlbh ();
	OpenGlbl ();
	OpenGljc ();
	if (!envVarGlBatchBranch)
		abc_selfield (glbh, "glbh_id_no3");
}

/*
 * Close data base files.
 */
void
CloseDB (
 void)
{
	GL_Close ();
	abc_fclose (glsj);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("journalType"))
	{
		char szTemp [5];
		memset (szTemp, 0, sizeof (szTemp));
		if (dflt_used)
		{
			sprintf (local_rec.journalDesc, "%30.30s", " ");
			DSP_FLD ("journalDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchGljc (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (gljcRec.co_no, comm_rec.co_no);
		if (local_rec.journalType [0] == ' ')
		{
			strncpy (szTemp, &local_rec.journalType [1], 2);
			strcpy (local_rec.journalType, szTemp);
		}
		else
			strcpy (local_rec.journalType, "  ");
		strcpy (gljcRec.journ_type, local_rec.journalType);
		cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess128));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.journalDesc, "%s Journal", clip (gljcRec.jnl_desc));
        
		DSP_FLD ("journalDesc");
		if (STANDING)
		{
			FLD ("trans_date") 	= ND;
			FLD ("tran_mth")   	= ND;
			FLD ("eff_from")   	= NA;
			FLD ("eff_to")	  	= NA;
			FLD ("dayApplied")	  	= NA;
		}
		else
		{
			FLD ("trans_date") 	= NA;
			FLD ("tran_mth")   	= NA;
			FLD ("eff_from")   	= ND;
			FLD ("eff_to")	  	= ND;
			FLD ("dayApplied")	  	= ND;
		}
	}

	return (EXIT_SUCCESS);
}

/*
 * Load Batch Line Details.  
 */
int
LoadLine (
	long	hhbhHash)
{
	char	tmp_accno [sizeof glblRec.acc_no];

	scn_set (3);
	line_cnt 	= 0;
	lcount [3] 	= 0;
	print_at (2,0,ML (mlStdMess035));

	glblRec.hhbh_hash 	= hhbhHash;
	glblRec.line_no 	= 0;
	
	cc = find_rec (glbl, &glblRec, GTEQ, "r");
	while (!cc && (glblRec.hhbh_hash == hhbhHash))
	{
		local_rec.lineNo = glblRec.line_no + 1;
		strcpy (local_rec.allocation, "");
		strcpy (local_rec.acctNo, glblRec.acc_no);
		GL_FormAccNo (local_rec.acctNo, tmp_accno, 0);

		sprintf (ACCNO (line_cnt), "%16.16s", glblRec.acc_no);
		local_rec.tranDate		= glblRec.tran_date;
		local_rec.fgnAmount		= glblRec.fx_amt;
		local_rec.fxrate 		= glblRec.exch_rate;
		local_rec.locAmount 	= glblRec.local_amt;
		strcpy (local_rec.fxcurr, glblRec.currency);
		strcpy (local_rec.refer, glblRec.user_ref);
		strcpy (local_rec.narrative, glblRec.narrative);
		strcpy (local_rec.dcFlag,(atoi (glblRec.dc_flag) % 2 == 0)? "C" : "D");

		CalcJournalTotal ();
		putval (lcount [3]++);
		line_cnt++;
		cc = find_rec (glbl, &glblRec, NEXT, "r");
	}
	if (lcount [3]==0)
		return (TRUE);

	vars [scn_start].row = lcount [3];

	line_cnt = 0;
	return (FALSE);
}

/*
 * Print Journal proof total.
 */
void
PrintJournalTotal (
	int		printAmountOnly)
{
	int		i			= 0,
			upper		= 0;
	double	total  		= 0.00,
			totalDebit  = 0.00,
			totalCredit = 0.00;

	upper = ((lcount [3] > line_cnt + 1) ? lcount [3] : line_cnt + 1);

	for (i = 0; i < upper; i++)
	{
		if (strncmp (ACCNO (i), sixteenSpaces, 16))
		{
			totalDebit 	+= no_dec (D_TOT (i)); 
			totalCredit += no_dec (C_TOT (i)); 
		}
	} 
	total =  totalCredit - totalDebit;
    if (!printAmountOnly)
    {
		cl_box (0,2,75,3);
	    cl_box (75,2,50,3); 

	    print_at (3, 77, ML (mlGlMess041), baseCurr); 
	    print_at (4, 77, ML (mlGlMess044), baseCurr);
	    print_at (5, 77, ML (mlGlMess050), baseCurr);

    }
	sprintf (err_str, "$%.2f", DOLLARS (total));
	print_at (3, 100, "%-16.16s", err_str);

	sprintf (err_str, "$%.2f", DOLLARS (totalDebit));
	print_at (4, 100, "%-16.16s", err_str);

	sprintf (err_str, "$%.2f", DOLLARS (totalCredit));
	print_at (5, 100, "%-16.16s", err_str);
}

void
CalcJournalTotal (void)
{
	if (CREDIT)
	{
		D_TOT (line_cnt)  = 0;
		C_TOT (line_cnt)  = local_rec.locAmount;
	}
	else
	{
		C_TOT (line_cnt)  = 0;
		D_TOT (line_cnt)  = local_rec.locAmount;
	}
}

int
heading (
	 int	scn)
{
	int	s_size = 132;

	if (restart) 
		return (EXIT_SUCCESS); 
	
	if (scn != cur_screen)
    {
		scn_set (scn);
    }

	clear ();

	rv_pr (ML (mlGlMess060), 50, 0, 1);
	if (scn == 1)
    {
		box (0,2,75,1);
    }

	line_at (1,0, s_size);
	line_at (21,0, s_size);

	if (scn != 1)
	{
		move (1,input_row);
   	 	PrintJournalTotal (FALSE);
		box (0,2,75,3);
	}

	/*  reset this variable for new screen NOT page	*/
	if (scn == 3)
	{
		scn_set (2);
		scn_write (2);
		scn_display (2);
	}
	if (scn == 2)
	{
		scn_set (3);
		scn_write (3);
		scn_display (3);
	}

	if (scn != cur_screen)
    {
		scn_set (scn);
    }
	scn_write (scn);
	scn_display (scn);
    return (EXIT_SUCCESS);
}

void
tab_other (
 int lineNo)
{
	if (lineNo >= lcount [3])
	{
		sprintf (err_str,ML (mlStdMess087), " ");
		rv_pr (err_str,1,8,1);
		return;
	}

	if (prog_status == ENTRY)
    {
        return;
    }

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_StripForm (glmrRec.acc_no, local_rec.acctNo);

    cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str,"%-35.35s", ML (mlGlMess046));
		rv_pr (err_str,1,8,1);
	}
	else
	{
		sprintf (err_str,ML (mlStdMess087), glmrRec.desc);
		rv_pr (err_str,1,8,1);
	}
}

/*
 * Read comr, get base currency information.
 */
int
ReadComr (void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	sprintf (comr_rec.co_no,"%-2.2s",comm_rec.co_no);

    cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess129));
		sleep (sleepTime);
		clear_mess ();
        return (EXIT_FAILURE);
	}
	cc = FindPocr (comm_rec.co_no, comr_rec.base_curr, "r");
	strcpy (baseCurr, pocrRec.code);
	sprintf (baseCurrDesc, "%20.20s", pocrRec.description);
	abc_fclose (comr);

    return (EXIT_SUCCESS);
}

void
SrchGljc (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Journal code description");
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, keyValue);

    cc = find_rec (gljc, &gljcRec, GTEQ, "r");
	while (!cc && 
           !strcmp (gljcRec.co_no, comm_rec.co_no) &&
           !strncmp (gljcRec.journ_type, keyValue, strlen (keyValue)))
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
    
	strcpy (local_rec.journalType, temp_str);
}

int
LoadBatch (
 void)
{
	int dataFound = FALSE;

	tab_open (glbhWork, tab_keys, 5, 27, WK_DEPTH, FALSE);
	tableOpened = TRUE;
	tab_add (glbhWork, 
             "#  %12.12s  %-18.18s  %-8.8s  %-5.5s  %6.6s  %-8.8s  ",
             "Batch Number", 
             "Journal Type", 
             "Date", 
             "Time", 
             "Period", 
             "User");
	if (!strcmp (local_rec.journalType, "ALL"))
	{
		strcpy (gljcRec.co_no, comm_rec.co_no);
		strcpy (gljcRec.journ_type, "  ");

        cc = find_rec (gljc, &gljcRec, GTEQ, "r");
		while (!cc && 
               !strcmp (gljcRec.co_no, comm_rec.co_no))
		{
			if (LoadOneJnlType (gljcRec.journ_type))
				dataFound = TRUE;

			cc = find_rec (gljc, &gljcRec, NEXT, "r");
		}
	}
	else
	{
		if (LoadOneJnlType (local_rec.journalType))
			dataFound = TRUE;
	}

	return (dataFound);
}

int
LoadOneJnlType (
 char *journalType)
{
	int		lposted_dmy [3];
	int		dataFound = FALSE;
	int		displayMonth;

	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	strcpy (glbhRec.jnl_type, journalType);
	sprintf (glbhRec.batch_no, "%10.10s", " ");

	cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	while (	!cc && 
			!strcmp (glbhRec.co_no, comm_rec.co_no) &&
			!strcmp (glbhRec.jnl_type, journalType))
	{
		if (glbhRec.stat_flag [0] == 'P' && !IS_STANDING (journalType))
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}
		if (IS_STANDING (journalType))
		{
			glsj_rec.hhbh_hash	=	glbhRec.hhbh_hash;
			cc = find_rec (glsj,&glsj_rec,COMPARISON,"r");
			dayApplied = glsj_rec.dt_apply;

			DateToDMY (glsj_rec.from_dt, &dmy [0],&dmy [1],&dmy [2]);
			day1 = dmy [0];
			if (day1 > 28)
				day1 = 28;
			if (dayApplied < day1)
				glsj_rec.from_dt += 28;

			DateToDMY (glsj_rec.to_dt, &dmy [0],&dmy [1],&dmy [2]);
			day1 = dmy [0];
			if (day1 > 28)
				day1 = 28;
			if (dayApplied > day1)
				glsj_rec.to_dt -= 28;

			if (comm_rec.gl_date < glsj_rec.from_dt || 
				(comm_rec.gl_date > glsj_rec.to_dt ) || 
				dayApplied > gdmy [0]  ) 
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
			DateToDMY
			(
				comm_rec.gl_date, 
				&dmy [0],
				&dmy [1],
				&dmy [2]
			);
			if (lposted_dmy [1] == dmy [1] && 
				lposted_dmy [2] == dmy [2])
			{
				cc = find_rec (glbh, &glbhRec, NEXT, "r");
				continue;
			}
		}

		dataFound = TRUE;

		DateToDMY
		(
			comm_rec.gl_date, 
			&dmy [0],
			&dmy [1],
			&dmy [2]
		);

		displayMonth = (IS_STANDING (journalType)) ?
						mth2fisc (dmy [1], comm_rec.fiscal):
						mth2fisc (glbhRec.mth, comm_rec.fiscal);
		tab_add  (glbhWork, 
	"  %-12.12s  %2.2s %-15.15s %-10.10s %-5.5s  %2.2d  %-8.8s  ",
				glbhRec.batch_no, 				/* 2 */
				glbhRec.jnl_type, 				/* 16 */
				gljcRec.jnl_desc,			
				DateToString (glbhRec.glbh_date), 
				glbhRec.glbh_time, 		
				displayMonth, 
				glbhRec.user);
		cc = find_rec (glbh, &glbhRec, NEXT, "r");
	}
	return (dataFound);
}

int
workView (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	rec_buffer [256];
	char	journalType [3];
	char	batch_no [sizeof glbhRec.batch_no];
	int		old_line;

	old_line = tab_tline (glbhWork);
	cc = tab_get (glbhWork, rec_buffer, EQUAL, old_line);
	
	sprintf (journalType, "%2.2s", rec_buffer + 16);
	sprintf (batch_no, "%10.10s", rec_buffer + 2);
	DisplayBatchDetails (journalType, batch_no);

	redraw_table (glbhWork);
	redraw_page (glbhWork, TRUE);

    return (EXIT_SUCCESS);
}

void
DisplayBatchDetails (
	char 	*journalType, 
	char 	*batch_no)
{
	int i;
	for (i = 0; i < MAXLINES; i++) 
	{
		D_TOT (i)  = 0.00;
		C_TOT (i)  = 0.00;
	}

    entry_exit = edit_exit = prog_exit = restart = 0;
	search_ok = 1;
    init_ok = TRUE;
	init_vars (2);
	init_vars (3);
	lcount [3] = 0;
	line_cnt = 0;
	strcpy (gljcRec.co_no, comm_rec.co_no);
	strcpy (gljcRec.journ_type, journalType);

    cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
	if (cc)
		file_err (cc, gljc, "DBFIND");
    
	sprintf (local_rec.journalDesc,"%s Journal", clip (gljcRec.jnl_desc));
		
	strcpy (glbhRec.co_no, comm_rec.co_no);
	strcpy (glbhRec.br_no, comm_rec.est_no);
	sprintf (glbhRec.jnl_type, "%-2.2s", journalType);
	sprintf (glbhRec.batch_no, "%-10.10s", batch_no);
	
	cc = find_rec (glbh, &glbhRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glbh, "DBFIND");

	if (STANDING)
	{
		glsj_rec.hhbh_hash = glbhRec.hhbh_hash;

        cc = find_rec (glsj, &glsj_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, glsj, "DBFIND");
	}

	if (LoadLine (glbhRec.hhbh_hash))
	{
		print_mess (ML (mlGlMess040));
		redraw_page (glbhWork, TRUE);
		return;
	}

	if (tableOpened)
		tab_clear (glbhWork);
	heading (3);
	scn_write (3);
	scn_display (3);
	edit (3);
}

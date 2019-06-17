/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_bk_rec.c,v 5.6 2002/07/09 09:23:08 scott Exp $
|  Program Name  : (ca_bk_rec.c)
|  Program Desc  : System Bank Transaction Reconciliation
|---------------------------------------------------------------------|
|  Author        : Bernard.M.delaVega | Date Written  : 28/06/96      |
|---------------------------------------------------------------------|
| $Log: ca_bk_rec.c,v $
| Revision 5.6  2002/07/09 09:23:08  scott
| Updated to convert to app.schema
|
| Revision 5.5  2002/02/19 06:05:04  scott
| Major rewrite, what a mess.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_bk_rec.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_bk_rec/ca_bk_rec.c,v 5.6 2002/07/09 09:23:08 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <GlUtils.h>
#include <assert.h>
#include <minimenu.h>
#include <ml_ca_mess.h>
#include <ml_std_mess.h>
#include <hot_keys.h>
#include <tabdisp.h>
#include <twodec.h>
#include <arralloc.h>

/*
 * Global flags and variables.
 */
int		glb_newstmt 		= 1,     /* New Bank Statement */
		edit_mode 			= 0,
		glb_editexit		= FALSE,
		scn 				= 1,
		glb_unreconcilestmt = FALSE,/* True if the statement was unreconciled */
		glb_reconciledstmt	= FALSE, /*True if Bank Stmt is already reconciled*/
		glb_dirty			= 0,
		glb_updatefiles 	= FALSE,
		glb_reconciled 		= FALSE, /* True if amt cleared is reconciliable */
		glb_entrymode		= TRUE;

Money	glb_debitClearAmt	=	0.0,
		glb_creditClearAmt	=	0.0,
		glb_posAmt			=	0.0,
		glb_negAmt			=	0.0,
		glb_amtClear		=	0.0,
		glb_amtUnClear		=	0.0,
		glb_startEnd		=	0.0;

char	glb_stmtprd [50],
		glb_unstmtrec [50],
		glb_tableBuff [256];

/*
 * Table Display 
 */
#define	MAXLINE		15000
#define	TABLINE	13
#define	STATUS_CONFIRMED	"C"
#define	STATUS_REJECTED		" "
#define	ENTER_KEY	'\r'

static char *headFmt 	= "%-10.10s   %-12.12s  %-40.40s   %16.16s     %-6.6s";
static char	*lineFmt = "%-10.10s   %010ld  %-40.40s     %16.16s     %-1.1s";

static char *tableName = NULL;

static	Money	glb_amount_confirmed;

/*
 * Functions for the Hot Keys. 
 */
static int DateSortFunc		(int, KEY_TAB *);
static int TranSortFunc		(int, KEY_TAB *);
static int SelectFunc		(int, KEY_TAB *);
static int UnreconFunc		(int, KEY_TAB *);
static int RestartFunc		(int, KEY_TAB *);
static int AcceptFunc		(int, KEY_TAB *);

/*
 * Hot Keys for the System Transaction Reconciliation. 
 */

#ifndef GVISION
KEY_TAB	user_keys [] =
{
	{" [D]ate Sort", 'D',DateSortFunc,
	 "Sort the transactions by date.","A",0,0,0},
	{" [T]ransaction Sort",'T',TranSortFunc,
	 "Sort the transactions by no.","A",0,0,0},
	{"Select", ENTER_KEY, SelectFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN1, RestartFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN16, AcceptFunc,
	 "", "A", 0, 0, 0},
	END_KEYS
};
#else
KEY_TAB	user_keys [] =
{
	{" Date Sort", 'D',DateSortFunc,
	 "Sort the transactions by date.","A",0,0,0},
	{" Transaction Sort",'T',TranSortFunc,
	 "Sort the transactions by no.","A",0,0,0},
	{"Select", ENTER_KEY, SelectFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN1, RestartFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN16, AcceptFunc,
	 "", "A", 0, 0, 0},
	END_KEYS
};
#endif

/*
 * Hot Keys for the System Transaction Unreconciliation. 
 */
#ifndef GVISION
KEY_TAB	user_keys2 [] =
{
	{" [D]ate Sort", 'D', DateSortFunc,
	 "Sort the transactions by date.", "A", 0, 0, 0},
	{" [T]rans Sort ", 'T', TranSortFunc,
	 "Sort the transactions by no.", "A", 0, 0, 0},
	{" [U]nreconcile ", 'U',UnreconFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN1, RestartFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN16, AcceptFunc,
     "", "A", 0, 0, 0},
	END_KEYS
};
#else
KEY_TAB	user_keys2 [] =
{
	{" Date Sort ", 'D', DateSortFunc,
	 "Sort the transactions by date.", "A", 0, 0, 0},
	{" Transaction Sort ", 'T', TranSortFunc,
	 "Sort the transactions by no.", "A", 0, 0, 0},
	{" Unreconcile ", 'U',UnreconFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN1, RestartFunc,
	 "", "A", 0, 0, 0},
	{NULL, FN16, AcceptFunc,
     "", "A", 0, 0, 0},
	END_KEYS
};
#endif

/*
 *	Structure for dynamic array,  for the xxxxxx lines for qsort	|
 */
struct CashArray
{
	char	transSort [21];
	long	transDate;
	long	transNumber;
	char	transDesc	[41];
	double	transAmount;
	char	transStatus	[2];
}	*cash;
	DArray cash_d;
	int	cashCnt = 0;
	int	noInTab = 0;

/*
 * AcceptFunc's  Menu 
 */
enum tagScreen
{
	Scn_Null,
    Save_Session,
    Save_Reconcile,
	Close_Menu,
    Menu_Max
};

static MENUTAB upd_menu [Menu_Max + 1] =
{
    { "              SAVE               ", "" }, /* Menu title */
    { " 1. Save Session                 ", "" },
    { " 2. Save Reconciliation          ", "" },
    { " 3. Cancel                       ", "" },
    { ENDMENU }
};

/*
 * Mini Menu No. 2 
 */
enum tagScreen2
{
	Unrec_Scn_Null,
    Unrec_Save_Session,
	Unrecon_Cancel,
    Unrecon_Menu_Max
};

static MENUTAB upd_menu2 [Unrecon_Menu_Max + 1] =
{
    { "    UNRECONCILE BANK STATEMENT   ", "" }, /* Menu title */
    { " 1. Save Unreconciliation        ", "" },
    { " 2. Cancel                       ", "" },
    { ENDMENU }
};

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct cbbsRecord	cbbs_rec;
struct cbbsRecord	cbbs2_rec;
struct cbbtRecord	cbbt_rec;

char	*data	= "data",
		*cbbs2	= "cbbs2";

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	stmt_prd_desc [4];
	long	prd_end_date;
	Money	amtCleared;
	Money	amtUnCleared;
	char	over_draft [12];
	char	over_cleared [14];
} local_rec;

static	struct var vars [] =
{
	{1, LIN, "bank_id",   3, 18, CHARTYPE,
        "UUUUU", "          ",
        " ", "", "Bank Code", "Enter Bank Code. [SEARCH] available.  ",
		NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",   3, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "bank_acct_no",    4, 18, CHARTYPE,
        "AAAAAAAAAAAAAAA", "          ",
        " ", "", "Account No       ", "",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
    {1, LIN, "acct_name",  4, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
    {1, LIN, "curcode",  5, 18, CHARTYPE,
        "UUU", "          ",
        " ", "", "Currency Code    ", "",
        NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
    {1, LIN, "curdesc",  5, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", " ",
        NA, NO,  JUSTLEFT, "", "", pocrRec.description},
    {1, LIN, "stmt_prd",  7, 18, CHARTYPE,
        "UU/UUUU", "          ",
        " ", "", "Statement Period ", "Enter format month and year [MM/YYYY].",
         NE, NO,  JUSTLEFT, "0123456789/","",cbbs_rec.period},
    {1, LIN, "stmt_prd_desc",  7, 35, CHARTYPE,
        "UUU", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.stmt_prd_desc},
    {1, LIN, "prd_end_date",  8, 18, EDATETYPE,
        "DD/DD/DD", "          ",
        " ", "", "Period End Date  ", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.prd_end_date},
    {1, LIN, "open_bal",  10, 18, MONEYTYPE,
        "NNN,NNN,NNN.NN", "          ",
        " ", "0.00", "Opening Balance", "Enter opening balance.",
         YES, NO,  JUSTRIGHT, "", "", (char *)&cbbs_rec.st_bal},
    {1, LIN, "close_bal",  11, 18, MONEYTYPE,
        "NNN,NNN,NNN.NN", "          ",
        " ", "0.00", "Closing Balance", "Enter closing balance.",
         YES, NO,  JUSTRIGHT, "", "", (char *)&cbbs_rec.end_bal},
    {1, LIN, "over_draft",  11, 35, CHARTYPE,
        "AAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTRIGHT, "", "", local_rec.over_draft},
    {1, LIN, "amt_clr",  13, 18, MONEYTYPE,
        "NNN,NNN,NNN.NN", "          ",
        " ", "0.00", "Amount Cleared", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.amtCleared},
    {1, LIN, "amt_unclr",  14, 18, MONEYTYPE,
        "NNN,NNN,NNN.NN", "          ",
        " ", "0.00", "Amount Uncleared", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.amtUnCleared},
    {1, LIN, "over_cleared",  14, 35, CHARTYPE,
        "AAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTRIGHT, "", "", local_rec.over_cleared},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};


/*
 * Local Function Prototypes.
 */
int		CashSort 				(const void *, const void *);
int		heading 				(int);
int     LoadTransRec 			(void);
int     MiniMenu2 				(void);
int     MiniMenu 				(void);
int     TableLineStatusUpdate 	(char *);
void 	StripComma 				(char *, char *);
void    CheckIfRecon 			(Money, Money);
void    CloseDB 				(void);
void    CrbkSearch 				(char *);
void    DisplayAmount 			(void);
void    FieldStatusUpdate 		(char *, char *);
void    GetCloseBal 			(void);
void    InitFlagVar 			(void);
void    OpenDB 					(void);
void    shutdown_prog 			(void);
void    TransTabCreate 			(void);
void    UpdateCbbs 				(void);
void    UpdateCbbt 				(void);

extern	int	tab_max_page;

/*
 * Function		:	main (int argc, char* argv [])
 *
 * Description	:	Main Processing Routine.
 *
 * Parameters	:	int     argc
 *                  char*   argv []
 *
 * Return		:	status
 */
int
main (
 int argc,
 char* argv [])
{
	SETUP_SCR (vars);

	tab_max_page	=	10000;

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	while (!prog_exit)
	{
		InitFlagVar ();

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);

		if (restart || prog_exit)
		{
			abc_unlock (crbk);
			continue;
		}

		heading (1);
		scn_display (1);
		glb_entrymode = FALSE;

		if (glb_reconciledstmt)
		{
			FLD ("open_bal") = NE;
			FLD ("close_bal") = NE;
		}
		else
		{
			FLD ("open_bal") = YES;
			FLD ("close_bal") = YES;
		}

		edit (1);

		if (restart)
			continue;

		if (noInTab >= MAXLINE)
		{
			/*
			 * No of lines exceeds MAXLINE defined. 
			 */
			print_err (ML (mlStdMess008));
			continue;
		}

		if (!noInTab)
		{
			/*
			 * No record found. 
			 */
			print_err (ML (mlStdMess009));
			sleep (sleepTime);
			continue;
		}

		while (!glb_editexit)
		{
			restart = FALSE;
			prog_exit = FALSE;

			/*
			 * Displays Screen 2 and table. 
			 */
			heading (2);

			TransTabCreate ();

			if (restart)
			{
				abc_unlock (cbbt);

				scn_set (1);
				break;
			}
		}

		if (glb_updatefiles)
		{
			UpdateCbbs ();

			if (glb_dirty)
				UpdateCbbt ();

			tab_close (tableName, TRUE);
			tableName = NULL;
			assert (tableName == NULL);

			break;
		}

		if (restart)
			continue;

		if (prog_exit)
			break;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Function		:	shutdown_prog (void)
 *
 * Description	:	Program exit sequence.
 *
 * Parameters	:	None.
 *
 * Return		:	None.
 */
void
shutdown_prog (void)
{
	CloseDB ();
	FinishProgram ();
}

/*
 * Function		:	OpenDB (void)
 *
 * Description	:	Open Database Files.
 *
 * Parameters	:	None.
 *
 * Return		:	None.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	abc_alias (cbbs2, cbbs);

	open_rec (crbk,  crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cbbs,  cbbs_list, CBBS_NO_FIELDS, "cbbs_id_no");
	open_rec (cbbs2, cbbs_list, CBBS_NO_FIELDS, "cbbs_id_no");
	open_rec (cbbt,  cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no2");
	OpenPocr ();
}

/*
 * Function		:	CloseDB (void)
 *
 * Description	:	Close Database Files.
 *
 * Parameters	:	None.
 *
 * Return		:	None.
 */
void
CloseDB (void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_fclose (cbbs);
	abc_fclose (cbbs2);
	GL_Close ();
	abc_dbclose (data);
}

/*
 * Function		:	spec_valid (int field)
 *
 * Description	:	Special Validation for screen fields.
 *
 * Parameters	:	int field
 *
 * Return		:	int
 */
int
spec_valid (
	int		field)
{
	Money	tempOpenBalance = 0.00,
			tempCloseBalance = 0.00;

	/*
	 * Validate Creditor Number And Allow Search. 
	 */
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			CrbkSearch (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (crbk_rec.bank_id, "     "))
			return (EXIT_FAILURE);

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Bank Code not found. 
			 */
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
            strcpy (pocrRec.co_no, comm_rec.co_no);
            strcpy (pocrRec.code, crbk_rec.curr_code);
            cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			DSP_FLD ("bank_id");
			DSP_FLD ("bk_name");
			DSP_FLD ("bank_acct_no");
			DSP_FLD ("acct_name");
			DSP_FLD ("curcode");
			DSP_FLD ("curdesc");

			return (EXIT_SUCCESS);
		}
	}

	/*
	 * Validate Statement Period.
	 */
	if (LCHECK ("stmt_prd"))
	{
		char 	data_string [8];
		int 	mth;
		int 	yr;
		long	CalcDate;

		strcpy (data_string, cbbs_rec.period);
		mth	=	atoi (data_string);
		yr	=	atoi (data_string + 3);

		if (mth <= 0 || mth >= 13 || yr < 0)
		{
			/*
			 * Incorrect date 
			 */
			print_mess (ML (mlCaMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.stmt_prd_desc, "%3.3s", ShortMonthName (mth));

		CalcDate	=	DMYToDate (1, mth, yr);
		local_rec.prd_end_date	=	MonthEnd (CalcDate);

		sprintf (glb_stmtprd, ML (mlCaMess066), ShortMonthName (mth), yr);
		DSP_FLD ("stmt_prd_desc");
		DSP_FLD ("prd_end_date");

		/*
		 * Check if new statement. 
		 */
		strcpy (cbbs_rec.co_no, comm_rec.co_no);
		strcpy (cbbs_rec.bank_id, crbk_rec.bank_id);
		cbbs_rec.end_period = local_rec.prd_end_date;
		glb_newstmt = find_rec (cbbs, &cbbs_rec, COMPARISON, "u");
		if (!glb_newstmt)
		{
			if (!strcmp (cbbs_rec.co_no, comm_rec.co_no) &&
				!strcmp (cbbs_rec.bank_id, crbk_rec.bank_id))
			{
				if (cbbs_rec.stat [0] == 'R')
				{
					/*
					 * Statement Already Reconciled. 
					 */
					glb_reconciledstmt = TRUE;
					print_err (ML (mlCaMess009));
					sprintf (glb_unstmtrec, ML (mlCaMess067), ShortMonthName (mth), yr);
				}

				skip_entry = 4;

				DSP_FLD ("open_bal");

				LoadTransRec ();

				DSP_FLD ("close_bal");

				tempOpenBalance	 = cbbs_rec.st_bal;
				tempCloseBalance = cbbs_rec.end_bal;
			}
		}
		else
			LoadTransRec ();

		if (cbbs_rec.end_bal < 0)
		{
			strcpy (local_rec.over_draft, "(Overdraft)");
			DSP_FLD ("over_draft");
		}

		if (local_rec.amtUnCleared < 0)
		{
			strcpy (local_rec.over_cleared, "(Over cleared)");
			DSP_FLD ("over_cleared");
		}

		DSP_FLD ("amt_clr");
		DSP_FLD ("amt_unclr");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Opening Balance. 
	 */
	if (LCHECK ("open_bal"))
	{
		if (glb_newstmt)
		{
			int		cnt = 0;
			double  dTemp = 0;

			strcpy (cbbs2_rec.co_no, comm_rec.co_no);
			strcpy (cbbs2_rec.bank_id, crbk_rec.bank_id);
			cbbs2_rec.end_period =  0L;
			cc = find_rec (cbbs2, &cbbs2_rec, GTEQ, "r");
			while (!cc)
			{
				if (!strcmp (cbbs2_rec.bank_id, crbk_rec.bank_id) &&
					cbbs_rec.end_period > cbbs2_rec.end_period)
				{
						cnt ++;
				}
				cc = find_rec (cbbs2, &cbbs2_rec, NEXT, "r");
			}

			/* 
			 * initial statement and entry mode 
			 */
			if (!cnt && glb_entrymode) 
			{
				if (dflt_used)
				{
					cbbs_rec.st_bal = 0.00;
				}
			}
			/* 
			 * Additional statement and entry mode 
			 */
			else if (cnt && glb_entrymode)
			{
				if (dflt_used)   /* default is used */
				{
					strcpy (cbbs2_rec.co_no, comm_rec.co_no);
					strcpy (cbbs2_rec.bank_id, crbk_rec.bank_id);
					cbbs2_rec.end_period = 0L;
					cc = find_rec (cbbs2, &cbbs2_rec, GTEQ, "r");
					while (!cc)
					{
						if (!strcmp (cbbs2_rec.bank_id, crbk_rec.bank_id) &&
						cbbs2_rec.end_period < local_rec.prd_end_date)
						{
							dTemp = cbbs2_rec.end_bal;
						}
						cc = find_rec (cbbs2, &cbbs2_rec, NEXT, "r");
					}

					cbbs_rec.st_bal = dTemp;
					DSP_FLD ("open_bal");
				}
			}
		}
		else 		/* existing statement */
		{
			if (dflt_used && !glb_entrymode)
				cbbs_rec.st_bal = tempOpenBalance;

			DSP_FLD ("open_bal");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Closing Balance. 
	 */
	if (LCHECK ("close_bal"))
	{
		if (glb_newstmt)
		{
			int		cnt = 0;

			strcpy (cbbs2_rec.co_no, comm_rec.co_no);
			strcpy (cbbs2_rec.bank_id, crbk_rec.bank_id);
			cbbs2_rec.end_period =  0L;
			cc = find_rec (cbbs2, &cbbs2_rec, GTEQ, "r");
			while (!cc)
			{
				if (!strcmp (cbbs2_rec.bank_id, crbk_rec.bank_id) &&
					cbbs_rec.end_period > cbbs2_rec.end_period)
				{
						cnt ++;
				}
				cc = find_rec (cbbs2, &cbbs2_rec, NEXT, "r");
			}

			if (!cnt)       /* initial statement */
			{
				if (dflt_used)
					cbbs_rec.end_bal = cbbs_rec.st_bal + glb_posAmt + glb_negAmt;

				DSP_FLD ("close_bal");
			}
			else
			{
				if (dflt_used)
					cbbs_rec.end_bal = cbbs_rec.st_bal + glb_posAmt + glb_negAmt;

				DSP_FLD ("close_bal");
			}

		}
		else
		{
			if (dflt_used && !glb_entrymode)
				cbbs_rec.end_bal = tempCloseBalance;

			DSP_FLD ("close_bal");
		}

		if (local_rec.amtUnCleared < 0)
		{
			strcpy (local_rec.over_cleared, "(Over cleared)");
			DSP_FLD ("over_cleared");
		}

		DSP_FLD ("amt_clr");
		DSP_FLD ("amt_unclr");
	}
	return (EXIT_SUCCESS);
}

/*
 * Function		:	CrbkSearch (char* keyValue)
 *
 * Description	:	Search routine for Creditors Bank File.
 *
 * Parameters	:	char*   keyValue
 *
 * Return		:	None.
 */
void
CrbkSearch (
	char	*keyValue)
{
	work_open ();
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s",keyValue);
	cc = save_rec ("#Bank Id ","#Bank Name");
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc &&
			!strncmp (crbk_rec.bank_id,keyValue,strlen (keyValue)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!restart)
	{
		init_vars (1);
		return;
	}

	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "crbk", "DBFIND");
}

/*
 * Function		:	heading (int scn)
 *
 * Description	:	Display Screen Headings.
 *
 * Parameters	:	int scn
 *
 * Return		:	0 - Ok; 1 - Error
 */
int
heading (
	int	scn)
{
	if (restart)
		return (EXIT_FAILURE);

    if (scn == 1)
	{
		snorm ();

		clear ();

		rv_pr (ML (mlCaMess065), 23,0,1);

		line_at (1,0,80);

		box (0,2,80,12);

		line_at (6,1,79);
		line_at (9,1,79);
		line_at (12,1,79);
		line_at (20,0,80);
		line_at (23,0,80);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	}
	else if (scn == 2)
	{
		swide ();

		clear ();
		tab_row = 18 - TABLINE;
		tab_col = 17;

		if (glb_reconciledstmt)
			rv_pr (glb_unstmtrec,35,0,1);
		else
			rv_pr (glb_stmtprd,35,0,1);

		box (0,1,132,3);

		print_at (2,4, ML (mlCaMess059), crbk_rec.bank_acct_no);
		print_at (2,35, "%s", crbk_rec.acct_name);

		print_at (3,4, ML (mlCaMess060),
				comma_fmt (DOLLARS (cbbs_rec.st_bal), "NNN,NNN,NNN.NN"));

		print_at (4,4, ML (mlCaMess062),
				comma_fmt (DOLLARS (cbbs_rec.end_bal), "NNN,NNN,NNN.NN"));

		print_at (22,0, ML (mlStdMess038),
				comm_rec.co_no,clip (comm_rec.co_name));
	}
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*
 * Function		:	UpdateCbbs (void)
 *
 * Description	:	Update cbbs file.
 *
 * Parameters	:
 *
 * Return		:	None.
 */
void
UpdateCbbs (void)
{
	clear ();

	if (cbbs_rec.end_bal == 0.00 && !glb_reconciled)
		GetCloseBal ();

	if (glb_newstmt)
	{
		cc = abc_add (cbbs, &cbbs_rec);
		if (cc)
			file_err (cc, "cbbs", "DBADD");
	}
	else
	{
		cc = abc_update (cbbs, &cbbs_rec);
		if (cc)
			file_err (cc, "cbbs", "DBUPDATE");
	}
}

/*
 * Function		:	UpdateCbbt (void)
 *
 * Description	:	Update cbbt file.
 *
 * Parameters	:   None.
 *
 * Return		:	None.
 */
void
UpdateCbbt (void)
{
	int		count;

	char	select [2];

	abc_selfield (cbbt, "cbbt_id_no1");

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);

	for (count = 0; count < noInTab; count ++)
	{
		tab_get (tableName, glb_tableBuff, EQUAL, count);


		cbbt_rec.tran_no = atol (glb_tableBuff + 13);
		sprintf (select, "%-1.1s", glb_tableBuff + 91);
		cc = find_rec (cbbt, &cbbt_rec, COMPARISON, "u");
		if (!cc)
		{
			if (!glb_reconciledstmt)
			{
				if (select [0] == 'C' && glb_reconciled)
				{
					strcpy (cbbt_rec.reconciled, "Y");
					cbbt_rec.period = cbbs_rec.end_period;
				}

				sprintf (cbbt_rec.select, "%-1.1s", select);
				cc = abc_update (cbbt, &cbbt_rec);
				if (cc)
					file_err (cc, "cbbt", "DBUPDATE");
			}
			else
			{
				strcpy (cbbt_rec.reconciled, "N");
				cbbt_rec.period = 0;

				cc = abc_update (cbbt, &cbbt_rec);
				if (cc)
					file_err (cc, "cbbt", "DBUPDATE");
			}
		}
	}
	abc_selfield (cbbt, "cbbt_id_no2");
}

/*
 * Function		:	UpdateReconsileField (void)
 *
 * Description	:	Update reconciled field on cbbt file.
 *
 * Parameters	:   None.
 *
 * Return		:	None.
 */
void
UpdateReconcileField (void)
{
	int		count = 0;

	char	select [2];

	abc_selfield (cbbt, "cbbt_id_no1");

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);

	while (count < noInTab)
	{
		tab_get (tableName, glb_tableBuff, EQUAL, count);

		cbbt_rec.tran_no = atol (glb_tableBuff + 13);
		sprintf (select, "%-1.1s", glb_tableBuff + 91);
		cc = find_rec (cbbt, &cbbt_rec, EQUAL, "u");
		count ++;
	}
	abc_selfield (cbbt, "cbbt_id_no2");
}

/*
 * Function		:	UpdateReconsileField (void)
 *
 * Description	:	Update status field on cbbs file for reconciled
 *                  transactions only.
 *
 * Parameters	:   None.
 *
 * Return		:	0 - Okay, else error.
 */
int
UpdateBankStmtStat (void)
{
	strcpy (cbbs_rec.stat, "R");
	return 0;
}

/*
 * Function		:	LoadTransRec (void)
 *
 * Description	:	Load Bank Transaction file.
 *
 * Parameters	:   None.
 *
 * Return		:	0 - Okay, else error.
 */
int
LoadTransRec (void)
{
	double	transAmount	=	0.00;
	/*
	 * Setup Array with 500 entries to start. 
	 */
	ArrAlloc (&cash_d, &cash, sizeof (struct CashArray), 500);
	cashCnt = 0;
	noInTab = 0;

	local_rec.amtCleared = 0.00;
	local_rec.amtUnCleared = 0.00;

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, cbbs_rec.bank_id);
	cbbt_rec.tran_date = 0L;
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc && !strcmp (cbbt_rec.co_no, comm_rec.co_no) &&
				  !strcmp (cbbt_rec.bank_id, cbbs_rec.bank_id))
	{
	strcpy (err_str, DateToString (cbbs_rec.end_period));
	print_at (0,0, "[%s][%s] [%s]", DateToString (cbbt_rec.tran_date), err_str, cbbt_rec.reconciled);getchar();

		if (cbbt_rec.tran_date <= cbbs_rec.end_period &&
			(cbbt_rec.reconciled [0] == 'N') &&
			!glb_reconciledstmt)
		{

			transAmount	= no_dec (cbbt_rec.tran_amt);
			/*
			 * Check the array size before adding new element. 
			 */
			if (!ArrChkLimit (&cash_d, cash, cashCnt))
				sys_err ("ArrChkLimit (cash)", ENOMEM, PNAME);

			assert (glb_reconciledstmt == FALSE);
			assert (cbbt_rec.reconciled [0] == 'N');

			/*
			 * Load values into array element cashCnt. 
			 */
			sprintf 
			(
				cash [cashCnt].transSort, 
				"%010ld%010ld",
				cbbt_rec.tran_date,
				cbbt_rec.tran_no
			);
			cash [cashCnt].transDate	=	cbbt_rec.tran_date;
			cash [cashCnt].transAmount	=	transAmount;
			cash [cashCnt].transNumber	=	cbbt_rec.tran_no;
			sprintf (cash [cashCnt].transDesc, 	"%-40.40s", cbbt_rec.tran_desc);
			sprintf (cash [cashCnt].transStatus, "%-1.1s",  cbbt_rec.select);
			/*
			 * Increment array counter. 
			 */
			cashCnt++;
			noInTab++;

			if (cbbt_rec.tran_amt >= 0.00)
				glb_posAmt += transAmount;
			else
				glb_negAmt += transAmount;

			if (!strcmp (cbbt_rec.select, STATUS_CONFIRMED))
			{
				local_rec.amtCleared += transAmount;

				if (cbbt_rec.tran_amt >= 0.00)
					glb_creditClearAmt += DOLLARS (transAmount);
				else
					glb_debitClearAmt += DOLLARS (transAmount);
			}
			else
				local_rec.amtUnCleared += transAmount;
		}
		else if (cbbt_rec.period == cbbs_rec.end_period &&
				(cbbt_rec.reconciled [0] == 'Y') &&
				glb_reconciledstmt)
		{
			assert (glb_reconciledstmt == TRUE);

			transAmount	= no_dec (cbbt_rec.tran_amt);
			/*
			 * Check the array size before adding new element. 
			 */
			if (!ArrChkLimit (&cash_d, cash, cashCnt))
				sys_err ("ArrChkLimit (cash)", ENOMEM, PNAME);

			/*
			 * Load values into array element cashCnt. 
			 */
			sprintf 
			(
				cash [cashCnt].transSort, 
				"%010ld%010ld",
				cbbt_rec.tran_date,
				cbbt_rec.tran_no
			);
			cash [cashCnt].transDate	=	cbbt_rec.tran_date;
			cash [cashCnt].transAmount	=	transAmount;
			cash [cashCnt].transNumber	=	cbbt_rec.tran_no;
			sprintf (cash [cashCnt].transDesc, 	"%-40.40s", cbbt_rec.tran_desc);
			sprintf (cash [cashCnt].transStatus, "%-1.1s",  cbbt_rec.select);
			/*
			 * Increment array counter. 
			 */
			cashCnt++;

			if (!strcmp (cbbt_rec.select, STATUS_CONFIRMED))
			{
				local_rec.amtCleared += transAmount;
			}
			else
			{
				local_rec.amtUnCleared += transAmount;
			}
		}
		cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Function		:	TransTabCreate (void)
 *
 * Description	:	Creates the table containing the Bank Transactions.
 *
 * Parameters	:   None.
 *
 * Return		:	None.
 */
void
TransTabCreate (void)
{
	int		count;

	if (!glb_reconciledstmt)
	{
		if (tableName != NULL)
			tab_close (tableName, TRUE);

		tableName = "initdate";
		tab_open (tableName, user_keys, tab_row, tab_col, TABLINE, FALSE);

		sprintf 
		(
			glb_tableBuff, 
			headFmt,
			"Trans.Date",
			"Trans.Number",
			"           Description",
			"Trans. Amount",
			"Status"
		);
	}
	else
	{
		assert (glb_reconciledstmt == TRUE);

		if (tableName != NULL)
			tab_close (tableName, TRUE);

		tableName = "initdate";
		tab_open (tableName, user_keys2, tab_row, tab_col, TABLINE, FALSE);

		sprintf 
		(
			glb_tableBuff, 
			headFmt,
			"Trans.Date",
			"Trans.Number",
			"           Description",
			"Trans. Amount",
			"Status"
		);
	}

	assert (strlen (glb_tableBuff) < sizeof (glb_tableBuff));

	tab_add (tableName, "#%s", glb_tableBuff);

	for (count = 0; count < noInTab; count ++)
	{
		tab_add 
		(
			tableName, 
			lineFmt,
			DateToString (cash [count].transDate),
			cash [count].transNumber,
			cash [count].transDesc,
			comma_fmt (DOLLARS (cash [count].transAmount), "NNN,NNN,NNN.NN"),
			cash [count].transStatus
		);
	}

	/*
	 * Free up the array memory. 
	 */
	ArrDelete (&cash_d);

	DisplayAmount ();

	if (!glb_reconciledstmt)
	{
		CheckIfRecon (glb_amtClear, glb_startEnd);

		if (glb_reconciled)
		{
			/*
			 * Amount is reconciliable. 
			 */
			print_err (ML (mlCaMess010));
			glb_reconciled = FALSE;
		}
	}

	tab_scan (tableName);

	if (prog_exit || glb_updatefiles)
		return ;

	if (restart)
	{
		tab_close (tableName, TRUE);
		tableName = NULL;
		assert (tableName == NULL);

		return ;
	}
}


/*
 * Function		:	SelectFunc (int key)
 *
 * Description	:	Select function.
 *
 * Parameters	:   int key
 *
 * Return		:	int key
 */
static int
SelectFunc (
	int    	key,
	KEY_TAB	*psUnused)
{
	int     st_line;
    char    status [2];

    assert (tableName != NULL);

	glb_dirty = TRUE;

    st_line = tab_tline (tableName);
    tab_get (tableName, glb_tableBuff, EQUAL, st_line);
	sprintf (status, "%-1.1s", glb_tableBuff + 91);

	key = ENTER_KEY;

	TableLineStatusUpdate (status);

	CheckIfRecon (glb_amtClear, glb_startEnd);

	if (glb_reconciled)
	{
		/*
		 * Statement reconciled and can be saved. 
		 */
		print_err (ML (mlCaMess011));
		glb_reconciled = FALSE;
	}

	return key;
}

/*
 * Function		:	UnreconFunc (int key)
 *
 * Description	:	Unreconsile Function.
 *
 * Parameters	:   int key
 *
 * Return		:	int key
 */
int
UnreconFunc (
	int		key,
 	KEY_TAB*   psUnused)
{
	/*
	 * Unreconcile bank statement status field from recon to unrecon. 
	 */
	strcpy (cbbs_rec.co_no, comm_rec.co_no);
	strcpy (cbbs_rec.bank_id, crbk_rec.bank_id);
	cbbs_rec.end_period = local_rec.prd_end_date;
	cc = find_rec (cbbs, &cbbs_rec, COMPARISON, "u");
	if (!cc)
	{
		if (!strcmp (cbbs_rec.co_no, comm_rec.co_no) &&
			!strcmp (cbbs_rec.bank_id, crbk_rec.bank_id))
		{
			assert (cbbs_rec.end_period != 0);

			strcpy (cbbs_rec.stat, " ");
		}
	}
	/*
	 * Statement unreconciled and can be saved. 
	 */
	glb_unreconcilestmt = TRUE;
	print_err (ML (mlCaMess012));

	return key;
}

/*
 * Function		:	RestartFunc (int key)
 *
 * Description	:	Restart Function
 *
 * Parameters	:   int key
 *
 * Return		:	int key
 */
int
RestartFunc (
	int    	key,
	KEY_TAB	*psUnused)
{
	restart = 1;

	return key;
}

/*
 * Function		:	AcceptFunc (int key)
 *
 * Description	:	Accept Funciton.
 *
 * Parameters	:   int key
 *
 * Return		:	int key
 */
int
AcceptFunc (
	int    	key,
	KEY_TAB	*psUnused)
{
	if (!glb_reconciledstmt)
	{
		assert (!glb_reconciledstmt);
		MiniMenu ();
	}
	else if (glb_reconciledstmt)
	{
		assert (glb_reconciledstmt);
		MiniMenu2 ();
	}

	if (glb_updatefiles)
		key = FN16;

	if (!prog_exit && !restart)
	{
		key = TRUE;		/* this MAY be a value other than FN16 ?? */
	}

	return key;
}

/*
 * Function		:	DateSortFunc (int key)
 *
 * Description	:	Sort the records by date.
 *
 * Parameters	:   int key
 *
 * Return		:	?
 */
int
DateSortFunc (
	int		key,
 	KEY_TAB	*psUnused)
{
	char	workString [21],
			finalString [21];

	int		count;
	int		i;

	/*
	 * Setup Array with 500 entries to start. 
	 */
	ArrAlloc (&cash_d, &cash, sizeof (struct CashArray), 500);
	cashCnt = 0;

	for (count = 0; count < noInTab; count ++)
	{
		/*
		 * Check the array size before adding new element. 
		 */
		if (!ArrChkLimit (&cash_d, cash, cashCnt))
			sys_err ("ArrChkLimit (cash)", ENOMEM, PNAME);

		tab_get (tableName, glb_tableBuff, EQUAL, count);
		 
		/*
		 * Load values into array element cashCnt. 
		 */
		sprintf 
		(
			cash [cashCnt].transSort, 
			"%-10.10s%-10.10s",
			glb_tableBuff,
			glb_tableBuff + 13
		);
		sprintf (workString, "%16.16s", glb_tableBuff + 70);

		StripComma (finalString, workString);

		cash [cashCnt].transDate	=	StringToDate (glb_tableBuff);
		cash [cashCnt].transAmount	=	atof (finalString);
		sprintf (cash [cashCnt].transDesc, 	 "%-40.40s", glb_tableBuff + 25);
		cash [cashCnt].transNumber	= 	atol (glb_tableBuff + 13);
		sprintf (cash [cashCnt].transDesc, 	"%-40.40s", glb_tableBuff + 25);
		sprintf (cash [cashCnt].transStatus, "%-1.1s", glb_tableBuff + 91);
		/*
		 * Increment array counter. 
		 */
		cashCnt++;
	}

 	tab_clear (tableName);
	sprintf 
	(
		glb_tableBuff, 
		headFmt,
		"Trans.Date",
		"Trans.Number",
		"           Description",
		"Trans. Amount",
		"Status"
	);
	tab_add (tableName, "#%s", glb_tableBuff);

	/*
	 * Sort the array in item description order.
	 */
	qsort (cash, cashCnt, sizeof (struct CashArray), CashSort);

	for (i = 0; i < cashCnt; i++)
	{
		tab_add 
		(
			tableName, 
			lineFmt,
			DateToString (cash [i].transDate),
			cash [i].transNumber,
			cash [i].transDesc,
			comma_fmt (cash [i].transAmount, "NNN,NNN,NNN.NN"),
			cash [i].transStatus
		);
	}
	/*
	 * Free up the array memory. 
	 */
	ArrDelete (&cash_d);

	DisplayAmount ();

	tab_display (tableName, TRUE);
	redraw_table (tableName);

    return (EXIT_SUCCESS);
}

/*
 * Function		:	TranSortFunc (int key)
 *
 * Description	:	Sort the records by transaction no.
 *
 * Parameters	:   int key
 *
 * Return		:	?
 */
int
TranSortFunc (
	int		key,
 	KEY_TAB	*psUnused)
{
	char	workString [21],
			finalString [21];

	int		count,
			i;
	/*
	 * Setup Array with 500 entries to start. 
	 */
	ArrAlloc (&cash_d, &cash, sizeof (struct CashArray), 500);
	cashCnt = 0;

	for (count = 0; count < noInTab; count ++)
	{
		/*
		 * Check the array size before adding new element. 
		 */
		if (!ArrChkLimit (&cash_d, cash, cashCnt))
			sys_err ("ArrChkLimit (cash)", ENOMEM, PNAME);

		tab_get (tableName, glb_tableBuff, EQUAL, count);

		/*
		 * Load values into array element cashCnt. 
		 */
		sprintf 
		(
			cash [cashCnt].transSort, 
			"%-10.10s%-10.10s",
			glb_tableBuff + 13,
			glb_tableBuff
		);
		sprintf (workString, "%16.16s", glb_tableBuff + 70);

		StripComma (finalString, workString);

		cash [cashCnt].transDate	=	StringToDate (glb_tableBuff);
		cash [cashCnt].transAmount	=	atof (finalString);
		cash [cashCnt].transNumber  = 	atol (glb_tableBuff + 13);
		sprintf (cash [cashCnt].transDesc, 	 "%-40.40s", glb_tableBuff + 25);
		sprintf (cash [cashCnt].transStatus, "%-1.1s", glb_tableBuff + 91);
		/*
		 * Increment array counter. 
		 */
		cashCnt++;
	}

	tab_clear (tableName);
	sprintf 
	(
		glb_tableBuff, 
		headFmt,
		"Trans.Date",
		"Trans.Number",
		"           Description",
		"Trans. Amount",
		"Status"
	);

	tab_add (tableName, "#%s", glb_tableBuff);

	/*
	 * Sort the array in item description order.
	 */
	qsort (cash, cashCnt, sizeof (struct CashArray), CashSort);

	for (i = 0; i < cashCnt; i++)
	{

		tab_add 
		(
			tableName, 
			lineFmt,
			DateToString (cash [i].transDate),
			cash [i].transNumber,
			cash [i].transDesc,
			comma_fmt (cash [i].transAmount, "NNN,NNN,NNN.NN"),
			cash [i].transStatus
		);
	}
	/*
	 * Free up the array memory. 
	 */
	ArrDelete (&cash_d);

	DisplayAmount ();

	tab_display (tableName, TRUE);
	redraw_table (tableName);
    return (EXIT_FAILURE);
}

/*
 * Function		:	DisplayAmount (void)
 *
 * Description	:	Displays and updates the amtCleared and
 *                  amtUnCleared screen fields.
 *
 * Parameters	:   None.
 *
 * Return		:	None.
 */
void
DisplayAmount (void)
{
	int		firstTime = TRUE;

	glb_startEnd = DOLLARS (cbbs_rec.st_bal - cbbs_rec.end_bal);

	if (!glb_reconciledstmt)
	{
		print_at (2, 65, ML (mlCaMess061));
		print_at (2, 89, ML (mlCaMess063));
	}
	else
    {
		print_at (3,65, ML (mlCaMess061));
    }

	print_at (4, 65, ML (mlCaMess064));

	glb_amtClear = DOLLARS (local_rec.amtCleared) +  glb_amount_confirmed;
	print_at (3, 114, "%16.16s", comma_fmt (glb_amtClear, "NNN,NNN,NNN.NN"));

	glb_amtUnClear = DOLLARS (local_rec.amtUnCleared) - glb_amount_confirmed;
	print_at (4, 114, "%16.16s", comma_fmt (glb_amtUnClear, "NNN,NNN,NNN.NN"));

	if (!glb_reconciledstmt)
	{
		print_at (3,
                  78,
                  "%16.16s",
                  comma_fmt (fabs (glb_debitClearAmt), "NNN,NNN,NNN.NN"));
		print_at (3,
                  96,
                  "%16.16s",
                  comma_fmt (glb_creditClearAmt, "NNN,NNN,NNN.NN"));
	}

	if (firstTime && !glb_dirty)
    {
		firstTime = FALSE;
    }
}

/*
 * Function		:	TableLineStatusUpdate (char* status)
 *
 * Description	:	Checks status of the current line if
 *                  STATUS_CONFIRMED OR STATUS_REJECTED.
 *
 * Parameters	:   new status.
 *
 * Return		:	1
 */
int
TableLineStatusUpdate (
	char	*status)
{
	int		st_line;

	assert (tableName != NULL);

	st_line = tab_tline (tableName);
	tab_get (tableName, glb_tableBuff, EQUAL, st_line);

	assert (strlen (glb_tableBuff) < sizeof (glb_tableBuff));

	FieldStatusUpdate (glb_tableBuff, status);
	tab_update (tableName, "%s", glb_tableBuff);

	DisplayAmount ();

	return (EXIT_FAILURE);
}

/*
 * Function		:	FieldStatusUpdate (char* glb_tableBuff, char* status)
 *
 * Description	:	Gets the amount of the current line and updates
 *                  the status field of the current table line.
 *
 * Parameters	:   char*   glb_tableBuff
 *                  char*   status
 *
 * Return		:	None.
 */
void
FieldStatusUpdate (
	char	*tableBuff,
	char	*status)
{
	char	workString 	[21],
			finalString [21];

	sprintf (workString, "%16.16s", &tableBuff [70]);

	StripComma (finalString, workString);

	if (!strcmp (status, STATUS_REJECTED))
	{
		glb_amount_confirmed += atof (finalString);

		if (atof (finalString) >= 0.00)
			glb_creditClearAmt += atof (finalString);
		else
			glb_debitClearAmt += atof (finalString);

		sprintf (status, "%-1.1s", STATUS_CONFIRMED);
		strncpy (&tableBuff [91], status, strlen (status));
	}
	else
	{
		glb_amount_confirmed -= atof (finalString);

		if (atof (finalString) >= 0.00)
			glb_creditClearAmt -= atof (finalString);
		else
			glb_debitClearAmt -= atof (finalString);

		sprintf (status, "%-1.1s", STATUS_REJECTED);
		strncpy (&tableBuff [91], status, strlen (status));
	}
}

/*
 * Function		:	CheckIfRecon (Money amtclr, Money stend)
 *
 * Description	:	Check if amount cleared is equal to the difference
 *                  of the open and close balance.
 *
 * Parameters	:   Money   amtclr  Amount cleared.
 *                  Money   stdend
 *
 * Return		:	None.
 */
void
CheckIfRecon (
	Money  amtclr,
	Money  stend)
{
	if ( (fabs (amtclr) == fabs (stend)) && glb_amtClear != 0.00)
    {
		glb_reconciled = TRUE;
    }
}

/*
 * Function		: IsYesReponse (char* prompt)
 *
 * Description	: Check if amount cleared is equal to the difference
 *                 of the open and close balance.
 *
 * Parameters	: char*   prompt
 *
 * Return	: non-zero - yes; 0 - no
 */
int
IsYesResponse (
	char	*prompt)
{
    int	    ch;
    char    promptYN [256];

    strcpy (promptYN, prompt);

    /*
     * Substitute ? for standard Y/N 
     */
    if (!strchr (promptYN, '?'))
    {
        strcat (clip (promptYN), "?");
    }
    strcpy (strrchr (promptYN,'?'), "? (Y/N) ");

    assert (strlen (promptYN) < sizeof promptYN);

    ch = prmptmsg (promptYN, "YyNn", 1, 20);
    return (ch == 'y' || ch == 'Y');
}

/*
 * Function		:	GetOpenBal (void)
 *
 * Description	:	Get this month's open balance from last month's
 *                  closing balance.
 *
 * Parameters	:   None.
 *
 * Return		:	None.
 */
void
GetOpenBal (void)
{
	Money	temp = 0.00;
	int		cnt = 0;

	strcpy (cbbs2_rec.co_no, comm_rec.co_no);
	strcpy (cbbs2_rec.bank_id, crbk_rec.bank_id);
	cbbs2_rec.end_period = 0L;
	cc = find_rec (cbbs2, &cbbs2_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (cbbs2_rec.bank_id, crbk_rec.bank_id))
		{
			if (cbbs2_rec.end_period == cbbs_rec.end_period &&
                cnt == 0)
			{
				temp = cbbs2_rec.st_bal;
				break;
			}

			if (cbbs2_rec.end_period == cbbs_rec.end_period &&
                cnt != 0)
			{
				cbbs_rec.st_bal = temp;
				break;
			}

			temp = cbbs2_rec.end_bal;
			cbbs_rec.st_bal = temp;
		}
		cnt ++;
		cc = find_rec (cbbs2, &cbbs2_rec, NEXT, "r");
	}
}

/*
 * Function     : GetCloseBal (void)
 *
 * Description	: Get this month's closing balance.
 *
 * Parameters	: None.
 *
 * Return       : None.
 */
void
GetCloseBal (void)
{
	cbbs_rec.end_bal = cbbs_rec.st_bal -
                      (local_rec.amtCleared - local_rec.amtUnCleared) *
                       100;
}

/*
 * Function	: MiniMenu (void)
 *
 * Description	: Menu Function.
 *
 * Parameters	: None.
 *
 * Return	: 0 - Okay; 1 - Error.
 */
int
MiniMenu (void)
{
	int		exit_func = 0;
	while (!exit_func)
	{
		int		option = 0;

		mmenu_print (upd_menu [0].menu_opt, &upd_menu [1],0);
		option = mmenu_select (&upd_menu [1]);
		switch (option + 1)
		{
			case Save_Session:
				glb_updatefiles = TRUE;
				exit_func = 1;
				glb_editexit = TRUE;
				prog_exit = 1;
				break;

			case Save_Reconcile:
				CheckIfRecon (glb_amtClear, glb_startEnd);

				if (glb_reconciled)
				{
					assert (glb_amtClear != 0.00);
					UpdateReconcileField ();
					UpdateBankStmtStat ();
					glb_updatefiles = TRUE;
					glb_dirty = 1;
					exit_func = 1;
					glb_editexit = TRUE;
					prog_exit = 1;
				}
				else
				{
					/*
					 * Statement is not yet reconciled. 
					 */
					print_err (ML (mlCaMess013));
					glb_reconciled = FALSE;
					prog_exit = 0;
					glb_updatefiles = FALSE;
				}
				break;

			case Close_Menu:
				clear ();
				heading (2);
				DisplayAmount ();
				redraw_table (tableName);
				exit_func = 1;
				return (EXIT_FAILURE);

			default:
				/*
				 * Invalid option. 
				 */
				print_err (ML (mlStdMess011));
				break;
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Function	    : MiniMenu2 (void)
 *
 * Description	: Menu Function 2.
 *
 * Parameters	: None.
 *
 * Return	    : 0 - Okay; 1 - Error.
 */
int
MiniMenu2 (void)
{
	int		exit_func = 0;

	while (!exit_func)
	{
		int		option = 0;

		mmenu_print (upd_menu2 [0].menu_opt, &upd_menu2 [1],0);
		option = mmenu_select (&upd_menu2 [1]);
		switch (option + 1)
		{
			case Unrec_Save_Session:
				if (glb_unreconcilestmt)
				{
					glb_updatefiles = TRUE;
					glb_dirty = 1;
					exit_func = 1;
					glb_editexit = TRUE;
					prog_exit = 1;
				}
				else
				{
					/*
					 * Statement must be unreconciled first. 
					 */
					assert (!glb_unreconcilestmt);
					print_err (ML (mlCaMess014));
				}

				break;

			case Unrecon_Cancel:
				clear ();
				heading (2);
				DisplayAmount ();
				redraw_table (tableName);
				exit_func = 1;
				return (EXIT_FAILURE);

			default:
				/*
				 * Invalid option. 
				 */
				print_err (ML (mlStdMess011));
				break;
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Function	:   InitFlagVar (void)
 *
 * Description	:
 *
 * Parameters	:   None.
 *
 * Return	:   None.
 */
void
InitFlagVar (void)
{
	strcpy (glb_stmtprd, "");

	entry_exit = 0;
	edit_exit = 0;
	search_ok = 1;

	glb_dirty = FALSE;
	glb_entrymode = TRUE;
	glb_reconciledstmt = FALSE;
	glb_reconciled = FALSE;
	glb_editexit = FALSE;
	prog_exit = 0;
	restart = 0;
	edit_mode = 0;
	init_vars (1);

	glb_amount_confirmed = 0.00;
	glb_amtClear = 0.00;
	glb_amtUnClear = 0.00;
	glb_startEnd = 0.00;

	glb_debitClearAmt = 0.00;
	glb_creditClearAmt = 0.00;

	glb_posAmt = 0.00;
	glb_negAmt = 0.00;
}
/*
 * Sort by Container/Purchase order OR by Purchase order/Container
 */
int 
CashSort (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct CashArray a = * (const struct CashArray *) a1;
	const struct CashArray b = * (const struct CashArray *) b1;

	result = strcmp (a.transSort, b.transSort);

	return (result);
}
void
StripComma (
	char	*s1,
	char	*s2)
{
	while (*s2)
	{
		while (*s2 == ',')
			s2++;

		*s1++ = *s2++;
	}
	*s1 = '\0';
}

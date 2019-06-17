/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_flow_enq.c,v 5.2 2002/02/22 03:58:19 scott Exp $
|  Program Name  : (ca_flow_enq.c)
|  Program Desc  : (Cash Flow Enquiry)
|---------------------------------------------------------------------|
|  Date Written  : 03/07/96    | Author	 : Liza Santos                |
|---------------------------------------------------------------------|
| $Log: ca_flow_enq.c,v $
| Revision 5.2  2002/02/22 03:58:19  scott
| Updated to remove disk sort files.
|
| Revision 5.1  2002/02/19 08:47:51  scott
| Updated to add app.schema and clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_flow_enq.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_flow_enq/ca_flow_enq.c,v 5.2 2002/02/22 03:58:19 scott Exp $";

#define		MAXSCNS		2
#define		MAXLINES	600

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<hot_keys.h>
#include 	<stddef.h>
#include 	<ml_std_mess.h>
#include 	<ml_ca_mess.h>
#include	<std_decs.h>
#include 	<twodec.h>
#include 	<arralloc.h>

void	InitAll 			(void);
void	SrchCrbk 		 	(char *);
int		ProcessTransaction 	(void);
void	LoadTransactions 	(void);
void	DisplayTransaction 	(void);
void	InitTotal 			(void);
void	DspGrandTot 		(void);
void	OpenDB 				(void);
void	CloseDB 			(void);
int		heading 			(int);
int		spec_valid 			(int);
int		CbSort 				(const void *, const void *);

#define		SCN_HEADER	1

#define		DAILY(x) 	(!strcmp (x,"D"))
#define		WEEKLY(x) 	(!strcmp (x,"W"))
#define		MONTHLY(x) 	(!strcmp (x,"M"))
#define		CF(x)		comma_fmt (DOLLARS (x),"NNN,NNN,NNN.NN")

#define	DIMOF (array) (sizeof (array) / sizeof (array [0]))

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct cbbtRecord	cbbt_rec;
struct pocrRecord	pocr_rec;

static char	*data	  = "data";

	/*
	 * Set up Array to hold Months of Year used with mon in time struct.
	 */
	static char *WrkMth [] = 
	{
			"Jan",	"Feb",	"Mar",	"Apr", 	"May", 	"Jun", 
			"Jul", 	"Aug", 	"Sep", 	"Oct", 	"Nov", 	"Dec"
	};

static	char	dispStr	[256];

char *dailyFmt	 	= "  %-10.10s  %16.16s      %16.16s      %16.16s";
char *monthlyFmt 	= " %04d  %-3.3s  %16.16s  %16.16s  %16.16s";

/*
 * Local & Screen Structures 
 */
struct {
	char	mode		 [2];
	char	modeDesc	 [7];
	char	systemDate	 [11];
	long	startDate;
	long	endDate;
	long	lsystemDate;
	char	dummy		 [11];
} local_rec;

/*
 *  Other variables used 
 */
static	int		daily;
static	int		monthly;
long	oldDate;
int		oldYear;
int		oldMonth;
double	processIncome	= 0.00;
double	processExpense	= 0.00;
double	grandIncome		= 0.00;
double	grandExpense 	= 0.00;
double	grandCash	 	= 0.00;

static	struct	var	vars [] =
{
	{SCN_HEADER, LIN, "bankid",	 3, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Bank Number      ", "Enter Bank Number. [SEARCH] available.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},

	{SCN_HEADER, LIN, "bankname",	 3, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", "",
		NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},

	{SCN_HEADER, LIN, "acctcode",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "Account Code     ", "",
		NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},

	{SCN_HEADER, LIN, "acctname",	 4, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},

	{SCN_HEADER, LIN, "currcode", 	5, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "",   "Currency Code    ", "", 
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code}, 

	{SCN_HEADER, LIN, "currdesc",	 5, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", pocr_rec.description},

	{SCN_HEADER, LIN, "mode",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Mode of Enquiry  ", " [D]aily [M]onthly - Default = Daily",
		YES, NO,  JUSTLEFT, "DM", "", local_rec.mode},

	{SCN_HEADER, LIN, "modeDesc",	 7, 25, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.modeDesc},

	{SCN_HEADER, LIN, "startdate",	 8, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "From             ", "Enter Start Date Range.",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.startDate},

	{SCN_HEADER, LIN, "endDate",	 8, 38, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "To               ", "Enter End Date Range.",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.endDate},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
extern	int	TruePosition;

/*
 *	Structure for dynamic array,  for the xxxxxx lines for qsort	|
 */
struct CashBookArray
{
	char	sortField	[11];
	long	oldDate;
	int		oldMonth;
	int		oldYear;
	double	incomeAmount;
	double	expenseAmount;
}	*cashBook;
	DArray cashBook_d;
	int	cbCnt = 0;

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char 	*argv [])
{
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = StringToDate (local_rec.systemDate);

	TruePosition	=	TRUE;

	SETUP_SCR 	(vars);
	init_scr 	();
	set_tty 	();
	set_masks 	();
	
	/*
	tab_col = 1;
	tab_row = 12;
	*/

	OpenDB	();
	clear ();

	while (!prog_exit)
	{
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	TRUE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		init_ok 	= 	TRUE;

        init_vars (SCN_HEADER);

		/*
		 *  Entry Screen 1 Linear Input 
		 */
		heading (SCN_HEADER);
		entry (SCN_HEADER);
		if (restart || prog_exit)
			continue;

		/*
		 *  Edit Screen 1 Linear Input 
		 */
		heading (SCN_HEADER);
		scn_display (SCN_HEADER);
		edit (SCN_HEADER);
		if (restart)
			continue;

		if ((DAILY (local_rec.mode)))
		{
			daily 	= 	TRUE;
			monthly	=	FALSE;
		}

		if ((MONTHLY (local_rec.mode)))
		{
			daily 	= 	FALSE;
			monthly	= 	TRUE;
		}

		heading (SCN_HEADER);
		scn_display (SCN_HEADER);

		if (!restart && !prog_exit)
		{
			InitAll ();
			ProcessTransaction ();
		}

		if (restart)
			continue;
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Initializations 
 */
void
InitAll (void)
{
	processIncome	= 0.00;
	processExpense	= 0.00;
	grandIncome		= 0.00;
	grandExpense 	= 0.00;
	grandCash	 	= 0.00;
}

/*
 * Special Validation 
 */
int
spec_valid (int field)
{
       
	/*
	 * Validate Bank ID and allow search. 
	 */
	if (LCHECK ("bankid"))
	{
		if (SRCH_KEY)
		{
	 		SrchCrbk (temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (crbk_rec.co_no, comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}


		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, crbk_rec.curr_code);
		cc	=	find_rec (pocr, &pocr_rec, COMPARISON,"r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		DSP_FLD ("bankid");
		DSP_FLD ("bankname");
		DSP_FLD ("acctcode");
		DSP_FLD ("acctname");
		DSP_FLD ("currcode");
		DSP_FLD ("currdesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Mode of Enquiry 
	 */
	if (LCHECK ("mode"))
	{
		if (local_rec.mode [0] == 'D')
			strcpy (local_rec.modeDesc, ML ("Daily")); 

		if (local_rec.mode [0] == 'M')
			strcpy (local_rec.modeDesc, ML ("Monthly")); 

		DSP_FLD ("modeDesc");
	}

	if (LCHECK ("startdate"))
	{
		if (dflt_used)
		{
			local_rec.startDate = 0l;
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && 
				local_rec.startDate > local_rec.endDate)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			local_rec.endDate = MonthEnd (local_rec.lsystemDate);
			DSP_FLD ("endDate");
		}

		if (local_rec.startDate > local_rec.endDate)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY && 
				local_rec.startDate > local_rec.endDate)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for Bank ID File. 
 */
void
SrchCrbk (
	char 	*keyValue)
{
	_work_open (5,0,40);

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id,"%-5.5s",keyValue);
	save_rec ("#Bank ID ","#Branch Name");
	cc = find_rec (crbk,&crbk_rec,GTEQ,"r");
	while (!cc && !strncmp (crbk_rec.bank_id,keyValue,strlen (keyValue)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id,"%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, crbk, "DBFIND");
}



/*
 * Processing of Transactions 
 */
int
ProcessTransaction (void)
{
	Dsp_open (0,1,16);
	if (daily)
	{
		Dsp_saverec (" TRANS. DATE            INCOME               EXPENSE             CASH FLOW ");
	}
	else
	{
		Dsp_saverec (" YEAR MONTH           INCOME           EXPENSE         CASH FLOW ");
	}
	Dsp_saverec ("");
	Dsp_saverec (" [NEXT]  [PREV]  [EDIT/END] ");

	LoadTransactions ();

	if (!cbCnt)
	{
		sprintf (err_str, "%s %s", ML ("There Are No Transactions for"), 
						crbk_rec.bank_id);
		Dsp_saverec (err_str);
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_FAILURE);
	}
	Dsp_srch ();
	Dsp_close ();
	return (EXIT_SUCCESS);
}
/*
 * Loading Transactions to Sort File 
 */
void
LoadTransactions (void)
{
	long	newDate		=	0L;
	int		newYear		=	0,
			newMonth	=	0,
			firstTime 	= TRUE,
			recordProcessed	=	FALSE;


	/*
	 * Setup Array with 500 entries to start. 
	 */
	ArrAlloc (&cashBook_d, &cashBook, sizeof (struct CashBookArray), 500);

	cbCnt = 0;

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
  	cbbt_rec.tran_date = local_rec.startDate;
	cc	= find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc && !strcmp (cbbt_rec.bank_id, crbk_rec.bank_id))
	{
		if (cbbt_rec.tran_date > local_rec.endDate)
			break;
		
		recordProcessed	=	TRUE;
		if (firstTime)
		{
			if (cbbt_rec.tran_amt >= 0)
				processIncome		+=	cbbt_rec.tran_amt;

			if (cbbt_rec.tran_amt < 0)
				processExpense	+=	fabs (cbbt_rec.tran_amt);

			if (daily)
				oldDate	=	cbbt_rec.tran_date;

			if (monthly)
				DateToDMY (cbbt_rec.tran_date, NULL, &oldMonth, &oldYear);

			firstTime = FALSE;
		}
		else
		{
			if (daily)
				newDate	=	cbbt_rec.tran_date;

			if (monthly)
				DateToDMY (cbbt_rec.tran_date, NULL, &newMonth, &newYear);
				
			if ((oldDate == newDate && daily)  ||
				  (oldMonth == newMonth  && 
					oldYear == newYear && monthly))
			{
				if (cbbt_rec.tran_amt >= 0)
					processIncome		+=	cbbt_rec.tran_amt;

				if (cbbt_rec.tran_amt < 0)
					processExpense	+=	fabs (cbbt_rec.tran_amt);
			}
			else
			{
				/*
				 * Check the array size before adding new element. 
				 */
				if (!ArrChkLimit (&cashBook_d, cashBook, cbCnt))
					sys_err ("ArrChkLimit (cashBook)", ENOMEM, PNAME);

				/*
				 * Load values into array element cbCnt. 
				 */
				if (daily)
				{
					sprintf 
					(
						cashBook [cbCnt].sortField, 
						"%010ld", 
						oldDate
					);
				}
				else
				{
					sprintf 
					(
						cashBook [cbCnt].sortField, 
						"%04d%02d", 
						oldYear, 
						oldMonth
					);
				}
				cashBook [cbCnt].oldDate 		= 	oldDate;
				cashBook [cbCnt].oldMonth		=	oldMonth;
				cashBook [cbCnt].oldYear		=	oldYear;
				cashBook [cbCnt].incomeAmount	=	processIncome;
				cashBook [cbCnt].expenseAmount	=	processExpense;

				/*
				 * Increment array counter. 
				 */
				cbCnt++;

				processIncome = 0.00;
				processExpense = 0.00;

				if (cbbt_rec.tran_amt >= 0)
					processIncome		+=	cbbt_rec.tran_amt;

				if (cbbt_rec.tran_amt < 0)
					processExpense	+=	fabs (cbbt_rec.tran_amt);

				if (daily)
					oldDate	=	cbbt_rec.tran_date;

				if (monthly)
					DateToDMY (cbbt_rec.tran_date, NULL, &oldMonth, &oldYear);
			}
			if (oldDate == cbbt_rec.tran_date)
				firstTime = FALSE;
		}
		cc	= find_rec (cbbt, &cbbt_rec, NEXT, "r");
	}
	if (recordProcessed	== TRUE)
	{
		/*
		 * Check the array size before adding new element. 
		 */
		if (!ArrChkLimit (&cashBook_d, cashBook, cbCnt))
			sys_err ("ArrChkLimit (cashBook)", ENOMEM, PNAME);

		/*
		 * Load values into array element cbCnt. 
		 */
		if (daily)
		{
			sprintf 
			(
				cashBook [cbCnt].sortField, 
				"%010ld", 
				oldDate
			);
		}
		else
		{
			sprintf 
			(
				cashBook [cbCnt].sortField, 
				"%04d%02d", 
				oldYear, 
				oldMonth
			);
		}
		cashBook [cbCnt].oldDate 		= 	oldDate;
		cashBook [cbCnt].oldMonth		=	oldMonth;
		cashBook [cbCnt].oldYear		=	oldYear;
		cashBook [cbCnt].incomeAmount	=	processIncome;
		cashBook [cbCnt].expenseAmount	=	processExpense;

		/*
		 * Increment array counter. 
		 */
		cbCnt++;
	}

	/*
	 * Sort the array
	 */
	qsort (cashBook, cbCnt, sizeof (struct CashBookArray), CbSort);

	DisplayTransaction ();

	/*
	 * Free up the array memory. 
	 */
	ArrDelete (&cashBook_d);
}
/*
 *  Screen Display  
 */
void
DisplayTransaction (void)
{
	int		i;
	char	DspStr [3][17];

	InitTotal ();

	for (i = 0; i < cbCnt; i++)
	{
		strcpy (DspStr [0], CF (cashBook [i].incomeAmount));
		strcpy (DspStr [1], CF (cashBook [i].expenseAmount));
		strcpy (DspStr [2], CF ((cashBook [i].incomeAmount - 
								 cashBook [i].expenseAmount)));

		if (daily)
		{
			sprintf 
			(
				dispStr,  
				dailyFmt,
				DateToString (cashBook [i].oldDate),
				DspStr [0],
				DspStr [1],
				DspStr [2]
			);
			Dsp_saverec (dispStr);
		}
		else if (monthly)
		{
			sprintf 
			(
				dispStr, 
				monthlyFmt,
				cashBook [i].oldYear,
				WrkMth [ cashBook [i].oldMonth - 1],
				DspStr [0],
				DspStr [1],
				DspStr [2]
			);
			Dsp_saverec (dispStr);
		}
		grandIncome		+= twodec (cashBook [i].incomeAmount);
		grandExpense	+= twodec (cashBook [i].expenseAmount);
		grandCash		+= twodec (cashBook [i].incomeAmount - 
								   cashBook [i].expenseAmount);
	}
	DspGrandTot ();

	return; 
}


/*
 *  Initialize variable 
 */
void
InitTotal (void)
{
	grandIncome		= 0.00;
	grandExpense 	= 0.00;
	grandCash	 	= 0.00;
}

/*
 * Display Grand Totals  
 */
void
DspGrandTot (void)
{
	char	DspGrand [3][17];

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	strcpy (DspGrand [0], CF (grandIncome));
	strcpy (DspGrand [1], CF (grandExpense));
	strcpy (DspGrand [2], CF (grandCash));

    if (daily)
	{
		sprintf (dispStr, "^1GRAND TOTAL^6   %16.16s      %16.16s      %16.16s   ", 
				DspGrand [0],
				DspGrand [1],
				DspGrand [2]);
		Dsp_saverec (dispStr);
	}

	if (monthly)
	{
		sprintf (dispStr, "^1GRAND TOTAL^6 %16.16s  %16.16s  %16.16s   ", 
				DspGrand [0],
				DspGrand [1],
				DspGrand [2]);

		Dsp_saverec (dispStr);
	}
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (crbk	, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cbbt	, cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no2");
	open_rec (pocr	, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_fclose (pocr);
	abc_dbclose (data);
}
/*
 * Sort array details.
 */
int 
CbSort (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct CashBookArray a = * (const struct CashBookArray *) a1;
	const struct CashBookArray b = * (const struct CashBookArray *) b1;

	result = strcmp (a.sortField, b.sortField);

	return (result);
}
/*
 * Heading 
 */
int
heading (int scn)
{
	if (scn != cur_screen)
		scn_set (scn);    

	clear ();
	rv_pr (ML (mlCaMess015), 30, 0, 1);
	line_at (1,0,80);

	box (0, 2, 80, 6);
	line_at (6,1,79);
	line_at (21,0,80);

	strcpy (err_str,ML (mlStdMess038));
    print_at (22,0,err_str, comm_rec.co_no, clip (comm_rec.co_name));

	line_cnt = 0;
	scn_write (scn);   
	scn_display (scn);   
	return (EXIT_SUCCESS);
}

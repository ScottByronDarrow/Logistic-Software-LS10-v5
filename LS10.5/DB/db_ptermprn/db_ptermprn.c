/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_ptermprn.c,v 5.3 2002/07/23 07:18:42 scott Exp $
|  Program Name  : (db_ptermprn.c)
|  Program Desc  : (Print Customer Payment Terms)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_ptermprn.c,v $
| Revision 5.3  2002/07/23 07:18:42  scott
| Updated to use new sort routines
|
| Revision 5.2  2002/07/17 09:57:08  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/12/04 07:47:31  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_ptermprn.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_ptermprn/db_ptermprn.c,v 5.3 2002/07/23 07:18:42 scott Exp $";

#include 	<pslscr.h>
#include 	<arralloc.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include    <errno.h>
#include 	<arralloc.h>

#define		CF(x)		comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN")
#define		NUM_SORT	(local_rec.sort_type [0] == 'N')

FILE	*pp;

#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuccRecord	cucc_rec;
struct pocrRecord	pocrRec;

	char	*data  = "data", 
			*cumr2 = "cumr2";

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;
/*
 * The structures 'dtls' are initialised in function .                
 * the number of details is stored in external variable 'detCnt'.  
 */
struct Detail
{
	long	hhci_hash;	
	double	inv_amt;	
	double	lcl_amt;	
}	*dtls;
	DArray	dtls_d;		
	int		detCnt;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[13];
	char	currCode	[sizeof cumr_rec.curr_code]; 
	char	custCode	[sizeof cumr_rec.dbt_acronym];
	long	hhcuHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

	int		envDbCo, 
	   		envDbFind, 
	   		firstPrint = 0, 
	   		firstCurrency = TRUE, 
	   		printerNo = 1, 
	   		linePrinted, 
	   		envDbNettUsed = TRUE, 
	   		envDbMcurr = FALSE;

	double	invoiceTotal, 
	      	localTotal;

	char	branchNo [3], 
	    	defaultStart [11], 
	    	defaultEnd [11], 
	    	prevCurrCode [4], 
	    	currCode [4];

	static	char	*suffix [] = {
		"ST", "ND", "RD", "TH", "TH", "TH", "TH", "TH", "TH", 
        "TH", "TH", "TH", "TH", "TH", "TH", "TH", "TH", "TH", 
        "TH", "TH", "ST", "ND", "RD", "TH", "TH", "TH"
	};

	extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	long	startDate;
	long	endDate;
	int		lpno;
	char	lp_str [3];
	char 	back [2];
	char 	backDesc [11];
	char	sort_type [2];
	char	onite [2];
	char	oniteDesc [11];
	double	amt_invoice;
	char	startCurr [4];
	char	startCurrDesc [41];
	char	endCurr [4];
	char	endCurrDesc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "fromDate", 	4, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", defaultStart, "Start Date                     ", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.startDate}, 
	{1, LIN, "toDate", 	5, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", defaultEnd, "End Date                       ", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.endDate}, 
	{1, LIN, "amt", 	 	6, 2, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "Print invoices Greater than    ", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.amt_invoice}, 
	{1, LIN, "sort_type", 	7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Sort By N(umber) or A(cronym)  ", " ", 
		YES, NO,  JUSTLEFT, "NnAa", "", local_rec.sort_type}, 

	{1, LIN, "startCurr", 	8, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Start Currency                 ", "Enter Start Currency", 
		ND, NO,  JUSTLEFT, "", "", local_rec.startCurr}, 
	{1, LIN, "startCurrDesc", 	8, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO,  JUSTLEFT, "", "", local_rec.startCurrDesc}, 
	{1, LIN, "endCurr", 	9, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "End Currency                   ", "Enter End Currency", 
		ND, NO,  JUSTLEFT, "", "", local_rec.endCurr}, 
	{1, LIN, "endCurrDesc", 9, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO,  JUSTLEFT, "", "", local_rec.endCurrDesc}, 
	{1, LIN, "lpno", 	 	9, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number                 ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 		10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background                     ", " ", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 		10, 38, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 	11, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight                      ", " ", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "onightDesc", 		11, 38, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	run_prog 		(char *);
int 	spec_valid 		(int);
int		SortFunc		(const	void *,	const void *);
void 	SrchPocr 		(char *);
void 	ProcessCumr 	(void);
void 	ProcessFile		(void);
void 	PrintHeader 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	ReportHeading 	(int);
void 	PrintLine 		(void);
void 	CheckBreak 		(void);
void 	CurrencyHeader	(char *);
void 	PrintTotal 		(void);
void 	GetCheques 		(void);
int 	heading 		(int);

int
main (
 int                argc, 
 char*              argv [])
{
	char	*sptr;

 	envDbCo = atoi (get_env ("DB_CO"));
 	envDbFind  = atoi (get_env ("DB_FIND"));

	TruePosition	=	TRUE;

	if (argc != 1 && argc != 8)
	{
	    print_at (0, 0, mlDbMess161);
	    print_at (1, 0, mlDbMess162);
	    print_at (2, 0, mlDbMess158);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 *	Allocate initial detail for 1000 items
	 */
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	OpenDB ();

	strcpy (defaultStart, DateToString (MonthStart (comm_rec.dbt_date)));
	strcpy (defaultEnd,   DateToString (MonthEnd (comm_rec.dbt_date)));

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (argc == 8)
	{
		printerNo = atoi (argv [1]);
		sprintf (local_rec.sort_type, "%-1.1s", argv [2]);
		local_rec.amt_invoice = atof (argv [3]);
		local_rec.startDate = StringToDate (argv [4]);
		local_rec.endDate = StringToDate (argv [5]);
		sprintf (local_rec.startCurr, "%-3.3s", argv [6]);
		sprintf (local_rec.endCurr, "%-3.3s", argv [7]);
		
		ReportHeading (printerNo);
		dsp_screen ("Printing Customers Payment terms Report.", 
										comm_rec.co_no, comm_rec.co_name);
		ProcessCumr ();

		fprintf (pp, ".EOF\n");
		pclose (pp);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	if (envDbMcurr)
	{
		FLD ("startCurr") 		= YES;
		FLD ("endCurr") 		= YES;
		FLD ("startCurrDesc") 	= NA;
		FLD ("endCurrDesc") 	= NA;
		SCN_ROW ("lpno")		= 11;
		SCN_ROW ("back")		= 12;
		SCN_ROW ("onight")		= 13;
		SCN_ROW ("backDesc")	= 12;
		SCN_ROW ("onightDesc")	= 13;
	}

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy (err_str, ML (mlDbMess231));

		run_prog (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
run_prog (
 char*              prog_name)
{
	char	wk_amt [16], 
			wk_startDate [11], 
			wk_endDate [11], 
			wk_sort_type [2];

	sprintf (local_rec.lp_str, "%d", local_rec.lpno);
	sprintf (wk_sort_type, "%1.1s", local_rec.sort_type);

	sprintf (wk_startDate, "%-10.10s", DateToString (local_rec.startDate));
	sprintf (wk_endDate, "%-10.10s", DateToString (local_rec.endDate));

	sprintf (wk_amt, "%-15.2f", local_rec.amt_invoice);
	
	rset_tty ();

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);
	ArrDelete (&dtls_d);
	CloseDB (); 
	FinishProgram ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT", 
					"ONIGHT", 
					prog_name, 
					local_rec.lp_str, 
					wk_sort_type, 
					wk_amt, 
					wk_startDate, 
					wk_endDate, 
					local_rec.startCurr, 
					local_rec.endCurr, 
					err_str, (char *)0);
	}
    else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name, 
					prog_name, 
					local_rec.lp_str, 
					wk_sort_type, 
					wk_amt, 
					wk_startDate, 
					wk_endDate, 
					local_rec.startCurr, 
					local_rec.endCurr, 
					(char *)0);
	}
	else 
	{
		execlp (prog_name, 
				prog_name, 
				local_rec.lp_str, 
				wk_sort_type, 
				wk_amt, 
				wk_startDate, 
				wk_endDate, 
				local_rec.startCurr, 
				local_rec.endCurr, 
				(char *)0);
	}
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("toDate"))
	{
		if (local_rec.startDate > local_rec.endDate)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("fromDate"))
	{
		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (local_rec.endDate < local_rec.startDate)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
			(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.oniteDesc, 
			(local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCurr"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCurr, "   ");
			strcpy (local_rec.startCurrDesc, ML ("Start Currency"));
			DSP_FLD ("startCurr");
			DSP_FLD ("startCurrDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocrRec.co_no, comm_rec.co_no);
		sprintf (pocrRec.code, "%-3.3s", local_rec.startCurr);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startCurrDesc, pocrRec.description);
		DSP_FLD ("startCurrDesc");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCurr"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCurr, "~~~");
			strcpy (local_rec.endCurrDesc, ML ("End Currency"));
			DSP_FLD ("endCurr");
			DSP_FLD ("endCurrDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocrRec.co_no, comm_rec.co_no);
		sprintf (pocrRec.code, "%-3.3s", local_rec.endCurr);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endCurrDesc, pocrRec.description);
		DSP_FLD ("endCurrDesc");
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchPocr (
	char	*key_val)
{
    _work_open (3,0,40);
	save_rec ("#No.", "#Currency ");                       
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
    while (!cc && !strcmp (pocrRec.co_no, comm_rec.co_no) && 
		          !strncmp (pocrRec.code, key_val, strlen (key_val)))
    {                        
		cc = save_rec (pocrRec.code, pocrRec.description);                       
		if (cc)
			break;

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
}

void
ProcessCumr (void)
{
	int		i;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	strcpy (cumr_rec.dbt_no, "      ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * Exclude child customer for now.
		 */
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		if (envDbMcurr && (strcmp (cumr_rec.curr_code, local_rec.startCurr) < 0 ||
		              strcmp (cumr_rec.curr_code, local_rec.endCurr) > 0))
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode,
			"%-3.3s%-9.9s",
			(envDbMcurr) ? cumr_rec.curr_code : " ", 
			(NUM_SORT) ? cumr_rec.dbt_no : cumr_rec.dbt_acronym
		);
		strcpy (sortRec [sortCnt].currCode, (envDbMcurr) ? cumr_rec.curr_code : " ");
		strcpy (sortRec [sortCnt].custCode, (NUM_SORT) ? cumr_rec.dbt_no : cumr_rec.dbt_acronym);
		sortRec [sortCnt].hhcuHash = cumr_rec.hhcu_hash;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	abc_selfield (cumr, "cumr_hhcu_hash");

	strcpy (prevCurrCode, "");
	for (i = 0; i < sortCnt; i++)
	{
		cumr_rec.hhcu_hash	=	sortRec [i].hhcuHash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
			continue;
		
		dsp_process ("Customer : ", cumr_rec.dbt_no);

		sprintf (currCode, "%-3.3s", sortRec [i].currCode);

		ProcessFile ();
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
}

void
ProcessFile (void)
{
	int		i;

	double	tot_balance = 0.00;
	double	cumr_bo [6];

	dsp_process ("Customer", cumr_rec.dbt_acronym);

	for (i = 0; i < 6; i++)
		cumr_bo [i] = cumr_balance [i];

	cumr2_rec.ho_dbt_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		for (i = 0; i < 6; i++)
			cumr_bo [i] += cumr2_balance [i];
		
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	tot_balance = (cumr_bo [0] + cumr_bo [1] + cumr_bo [2] + 
				   cumr_bo [3] + cumr_bo [4] + cumr_bo [5]);

	if (tot_balance == 0.00)
		return;

	invoiceTotal 	= 0.00;
	localTotal 		= 0.00;
	linePrinted 	= FALSE;
	firstPrint 		= 0;
	GetCheques ();

	cuin_rec.ho_hash = cumr_rec.hhcu_hash;
	cuin_rec.date_of_inv = 0L;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuin_rec.ho_hash) 
	{
		if (cuin_rec.due_date < local_rec.startDate ||
		     cuin_rec.due_date > local_rec.endDate)
		{
			cc = find_rec (cuin, &cuin_rec, NEXT, "r");
			continue;
		}
		PrintLine ();
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}

	if (linePrinted)
		PrintTotal ();
}

void
PrintHeader (void)
{

	fprintf (pp, "|%s %s", cumr_rec.dbt_no, cumr_rec.dbt_name);
	if (envDbMcurr)
		fprintf (pp, "               ");
	fprintf (pp, "                                  ");
	fprintf (pp, "                              ");
	fprintf (pp, "|%14.14s|%10.10s|\n", 
		CF (cumr_rec.amt_lastpay),
		DateToString (cumr_rec.date_lastpay));
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumr2, cumr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_ho_cron");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
													        : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no2");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cucc);
	abc_fclose (pocr);

	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
ReportHeading (
 int                prnt_no)
{
	char	dt1 [11], 
			dt2 [11];

	strcpy (dt1, DateToString (local_rec.startDate));
	strcpy (dt2, DateToString (local_rec.endDate));

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat", "w")) == NULL)
		file_err (cc, "PFORMAT", "POPEN");

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp, ".LP%d\n", prnt_no);
	fprintf (pp, ".9\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".E%s\n", clip (comm_rec.co_name));
	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s\n", "PAYMENT TERMS REPORT.");
	fprintf (pp, ".C%s%.2f (NOTE : Report includes forward Cheques.)\n", 
				"INVOICES GREATER THAN :", local_rec.amt_invoice);
	fprintf (pp, "Date From %10.10s %-112.112s Date to %10.10s\n", dt1, " ", dt2);

	fprintf (pp, ".EAS AT : %s\n", SystemTime ());

	fprintf (pp, ".B1\n");

	fprintf (pp, ".R===");
	fprintf (pp, "===");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	fprintf (pp, "===============");
	if (envDbMcurr)
		fprintf (pp, "===============");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "===========================\n");

	if (envDbMcurr)
		return;

	/*-------------------------
	| Headings for non-MCURR. |
	-------------------------*/
	fprintf (pp, "===");
	fprintf (pp, "===");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	fprintf (pp, "===============");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "===========================\n");

	fprintf (pp, "|BR");
	fprintf (pp, "|DP");
	fprintf (pp, "|INVOICE ");
	fprintf (pp, "| INVOICE  ");
	fprintf (pp, "| INVOICE  ");
	fprintf (pp, "|  DAYS ");
	fprintf (pp, "| TOTAL OWING  ");
	fprintf (pp, "|          P A Y M E N T  ");
	fprintf (pp, "  T E R M S               ");
	fprintf (pp, "|       LAST  PAYMENT     |\n");

	fprintf (pp, "|NO");
	fprintf (pp, "|NO");
	fprintf (pp, "| NUMBER ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "| DUE DATE ");
	fprintf (pp, "|OVERDUE");
	fprintf (pp, "|              ");
	fprintf (pp, "|                         ");
	fprintf (pp, "                          ");
	fprintf (pp, "|    AMOUNT    ");
	fprintf (pp, "|   DATE   |\n");

	fprintf (pp, "|--");
	fprintf (pp, "|--");
	fprintf (pp, "|--------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------");
	fprintf (pp, "|-------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|-------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|----------|\n");

}

void
PrintLine (void)
{
	int		i; 
	int		offset;
	long	doverdue = 0L;
	double	balance = 0.00;
	double	lcl_amt;
	double	fgn_payments = 0.00;
	double	lcl_payments = 0.00;
	double	chq_bal = 0.00;

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			              : cuin_rec.amt;

	for (i = 0; i < detCnt; i++)
	{
		if (cuin_rec.hhci_hash == dtls [i].hhci_hash)
		{
			lcl_payments += dtls [i].lcl_amt;
			fgn_payments += dtls [i].inv_amt;
		}
	}
	if (fabs (cuin_rec.exch_rate) < 0.00001)
		cuin_rec.exch_rate = 1.00;

	/*-------------------------
	| Calculate local amount. |
	-------------------------*/
	lcl_amt = balance / cuin_rec.exch_rate;
	lcl_amt -= lcl_payments;

	/*------------------------------------------
	| Calculate balance in currency of origin. |
	------------------------------------------*/
	balance -= fgn_payments;

	if (balance == 0)
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cucc_rec.hold_ref, cuin_rec.inv_no);
		cc = find_rec (cucc, &cucc_rec, COMPARISON, "r");
		if (!cc)
		{
			cc = abc_delete ("cucc");
			if (cc)
				file_err (cc, "cucc", "DBDELETE");
		}
		return;
	}

	chq_bal = DOLLARS (lcl_amt);

	if (chq_bal < local_rec.amt_invoice)
		return;

	/*---------------------------
	| Check for currency break. |
	---------------------------*/
	CheckBreak ();

	linePrinted = TRUE;

	if (firstPrint == 0)
	{
		PrintHeader ();
		firstPrint = 1;
	}
	cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cucc_rec.hold_ref, cuin_rec.inv_no);
	if (find_rec (cucc, &cucc_rec, COMPARISON, "r"))
	{
		if (cuin_rec.pay_terms [2] >= 'A')
		{
			offset = cuin_rec.pay_terms [2] - 'A';

			sprintf (cucc_rec.comment, "20TH OF THE %d%s MONTH FOLLOWING DATE OF INVOICE", offset + 1, suffix [offset]);
		}
		else
			sprintf (cucc_rec.comment, "NETT %d DAYS FROM DATE OF INVOICE.", atoi (cuin_rec.pay_terms));
	}

	doverdue = cuin_rec.due_date - comm_rec.dbt_date;

	fprintf (pp, "|%2.2s", cuin_rec.est);
	fprintf (pp, "|%2.2s", cuin_rec.dp);
	fprintf (pp, "|%8.8s", cuin_rec.inv_no);

	fprintf (pp, "|%10.10s", DateToString (cuin_rec.date_of_inv));
	fprintf (pp, "|%10.10s", DateToString (cuin_rec.due_date));

	if (doverdue < 0L)
		fprintf (pp, "|%6ld ", doverdue * -1L);
	else
		fprintf (pp, "|       ");

	fprintf (pp, "|%14.14s", CF (balance));
	if (envDbMcurr)
		fprintf (pp, "|%14.14s", CF (lcl_amt));
	
	fprintf (pp, "| %-50.50s", cucc_rec.comment);
	fprintf (pp, "|              |          |\n");

	invoiceTotal += balance;
	localTotal += lcl_amt;
}

void
CheckBreak (void)
{
	if (!envDbMcurr)
		return;

	/*---------------------
	| Change in currency. |
	---------------------*/
	if (strcmp (prevCurrCode, currCode))
	{
		CurrencyHeader (currCode);

		strcpy (prevCurrCode, currCode);
		firstCurrency = FALSE;
	}
}

void
CurrencyHeader (
 char*              crrcy)
{
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", crrcy);
	cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
	if (cc)
		sprintf (pocrRec.description, "%-40.40s", "Currency Not Found");

	fprintf (pp, ".DS5\n");

	fprintf (pp, 
		".E CURRENCY : %-3.3s - %-s\n", 
		crrcy, 
		clip (pocrRec.description));

	fprintf (pp, "===");
	fprintf (pp, "===");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	fprintf (pp, "===============");
	if (envDbMcurr)
		fprintf (pp, "===============");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "===========================\n");

	fprintf (pp, "|BR");
	fprintf (pp, "|DP");
	fprintf (pp, "|INVOICE ");
	fprintf (pp, "| INVOICE  ");
	fprintf (pp, "| INVOICE  ");
	fprintf (pp, "|  DAYS ");
	fprintf (pp, "| TOTAL OWING  ");
	if (envDbMcurr)
		fprintf (pp, "| LOCAL AMOUNT ");
	fprintf (pp, "|          P A Y M E N T  ");
	fprintf (pp, "  T E R M S               ");
	fprintf (pp, "|       LAST  PAYMENT     |\n");

	fprintf (pp, "|NO");
	fprintf (pp, "|NO");
	fprintf (pp, "| NUMBER ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "| DUE DATE ");
	fprintf (pp, "|OVERDUE");
	fprintf (pp, "|              ");
	if (envDbMcurr)
		fprintf (pp, "|              ");
	fprintf (pp, "|                         ");
	fprintf (pp, "                          ");
	fprintf (pp, "|    AMOUNT    ");
	fprintf (pp, "|   DATE   |\n");

	fprintf (pp, "|--");
	fprintf (pp, "|--");
	fprintf (pp, "|--------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------");
	fprintf (pp, "|-------");
	fprintf (pp, "|--------------");
	if (envDbMcurr)
		fprintf (pp, "|--------------");
	fprintf (pp, "|-------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|----------|\n");

	if (!firstCurrency)
		fprintf (pp, ".PA\n");

	fflush (pp);
}

void
PrintTotal (void)
{
	fprintf (pp, "|  TOTAL %-35.35s ", " ");
	fprintf (pp, "|%14.14s", CF (invoiceTotal));
	if (envDbMcurr)
		fprintf (pp, "|%14.14s", CF (localTotal));
	
	fprintf (pp, "| %-50.50s", " ");
	fprintf (pp, "|              |          |\n");

	fprintf (pp, "|--");
	fprintf (pp, "|--");
	fprintf (pp, "|--------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------");
	fprintf (pp, "|-------");
	fprintf (pp, "|--------------");
	if (envDbMcurr)
		fprintf (pp, "|--------------");
	fprintf (pp, "|-------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|----------|\n");
}

void
GetCheques (void)
{
	detCnt = 0;

	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;
		cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
		while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
		{
			if (!ArrChkLimit (&dtls_d, dtls, detCnt))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

			dtls [detCnt].hhci_hash = cudt_rec.hhci_hash;
			dtls [detCnt].inv_amt = cudt_rec.amt_paid_inv;
			dtls [detCnt].lcl_amt = cudt_rec.loc_paid_inv;
			++detCnt;

			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

int
heading (
	int		scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlDbMess134), 25, 0, 1);
	line_at (1,1,79);

	if (envDbMcurr)
	{
		box (0, 3, 80, 10);
		line_at (10,1,79);
	}
	else
	{
		box (0, 3, 80, 8);
		line_at (8,1,79);
	}

	line_at (20,0,80);

	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.co_no, comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}

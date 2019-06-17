/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lfullpr.c,v 5.5 2002/08/14 04:26:11 scott Exp $
|  Program Name  : (db_lfullpr.c) 
|  Program Desc  : (Detailed Customers Ageing Schedule)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_lfullpr.c,v $
| Revision 5.5  2002/08/14 04:26:11  scott
| Updated for Linux warning
|
| Revision 5.4  2002/03/12 03:47:06  scott
| Updated to remove disk based sorting.
|
| Revision 5.3  2001/11/22 10:24:39  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lfullpr.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lfullpr/db_lfullpr.c,v 5.5 2002/08/14 04:26:11 scott Exp $";

#include 	<pslscr.h>
#include 	<arralloc.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include    <errno.h>

#define	MONTH_1		1
#define	MONTH_2		2
#define	MONTH_3		3
#define	MONTH_4		4
#define	NORMAL		5
#define	OVER_LIMIT	6
#define	STOP_CREDIT	7

#define	BY_CO		1
#define	BY_BR		2
#define	BY_DP		3

#define	NUMBER		1
#define	ACRONYM		2

#define	EXCLUDE		1
#define	PRINT_0		2
#define	PRINT_ALL	3

#define	LCL		0
#define	FGN		1

#define	CO_DBT		(envCoClose [0] == '1')

#define	LCL_CURR	(local_rec.currencyDisplay [0] == 'L' || !envDbMcurr)
#define	CURR_SORT	(local_rec.currencySort [0] == 'Y' && envDbMcurr)
#define	DBT_NUM		(sortType == NUMBER)

#define	CF(x)		comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN")

FILE	*pp;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cudpRecord	cudp_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct pocrRecord	pocr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

	char	*data  = "data", 
			*cumr2 = "cumr2", 
			*cumr3 = "cumr3";

/*
 * The structures 'detailRec' are initialised in function 'GetCheques'
 * the number of details is stored in external variable 'dtlsCnt'. 
 */
struct	Detail	
{            				/*===============================*/
	long	hhciHash;	    /* detail invoice reference.	 */
	double	fgnAmount;	    /* detail invoice amount.	     */
	double	locAmount;    	/* Local  invoice amount.	     */
	double	exchVar;    	/* Local  exchange variation     */
	char	type [2];	    /* Transaction type.		     */
	long	invDueDate;		/* Invoice due date.             */
	long	invDop;	    	/* Invoice date of payment.      */
	char	receiptNo [9];	/* Invoice->receipt No.		     */
}	*detailRec;             /*===============================*/

	DArray	dtls_d;
	int		dtlsCnt;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [23];
	char	currCode	[sizeof cumr_rec.curr_code]; 
	long	hhcuHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

	int		SortFunc			(const	void *,	const void *);

	double	g_tot [2]	= {0,0}, 
			g_tots 		= 0.00, 
			total [2][6], 
			totals [6]  = {0, 0, 0, 0, 0, 0}, 
			debits [6]  = {0, 0, 0, 0, 0, 0}, 
			credits [6] = {0, 0, 0, 0, 0, 0},
			percent [6]	= {0, 0, 0, 0, 0, 0};

	long	mendDate 	= 0L,
			useDate 	= 0L,
			lsystemDate = 0L;

	static char *inv_type [] = {
		"IN", 
		"CR", 
		"JN", 
		"CH", 
		"JN", 
		"JN", 
	};

	static char *run_desc [] = {
		"DETAILED AGEING SCHEDULE BY COMPANY", 
		"DETAILED AGEING SCHEDULE BY BRANCH", 
		"DETAILED AGEING SCHEDULE FOR DEPARTMENT"
	};

	static char *sort_desc [] = {
		"Sorted By Customer Number ", 
		"Sorted By Customer Acronym "
	};

	static char *bal_desc [] = {
        	"Exclude zero balance Customers.", 
        	"Excluding non-zero balance Customers.", 
        	"Including zero and non-zero balance Customers."
	};

	int		envDbNettUsed 	= TRUE,
			envDbMcurr   	= FALSE,
			envDbCo			= FALSE,
			envDbTotalAge 	= FALSE,
			envDbDaysAgeing = FALSE,
			reportType 		= 5,
			runType 		= 2,
			balanceType 	= 1,
			sortType 		= 2,
			firstPrint 		= 0,
			printerNo		= 1,
			expandedReport 	= FALSE,
			firstCurrency	= 0,
			systemAge 		= FALSE,
			currencyDisplay	= 0;

	char	department 			[3],
			branchNo 			[3],
			ageingType 			[2],
			previousCurrency	[4],
			currentCurrency 	[4],
			envCoClose 			[6];

/*
 * The CURR_LST linked list is used to store the totals for each currency. 
 */
struct	CURR_LST	
{
	char	code [4];
	char	desc [41];
	double	total [6];
	double	lcl_tot [6];
	struct	CURR_LST    *prev;
	struct	CURR_LST    *next;
};
#define	CURR_NULL	((struct CURR_LST *) NULL)
struct	CURR_LST *currentHead = CURR_NULL;
struct	CURR_LST *currentTail = CURR_NULL;

struct
{
	char	currencyDisplay [2];
	char	currencySort 	[2];
	char	startCurrency 	[4];
	char	endCurrency 	[4];
} local_rec;

/*
 * Local Function Prototypes.
 */
static int MoneyZero 				(double);
struct CURR_LST *CurrencyAllocate 	(void);
int 	CheckBreak 					(void);
int 	CurrencyTotal 				(char *);
int 	Process 					(void);
int 	SumCurrency 				(char *, double, double, int);
long 	ReadEsmr 					(char *, char *);
void 	CloseDB 					(void);
void 	CurrencyHeader 				(char *);
void 	EndReport 					(void);
void 	GetCheques 					(int, long);
void 	OpenDB 						(void);
void 	PrintCurrencySum 			(void);
void 	PrintHeader 				(void);
void 	PrintLine 					(void);
void 	ProcessFile					(void);
void 	ProcessInvoice 				(int, long);
void 	ReportHeading 				(int);

int
main (
 int                argc, 
 char*              argv [])
{
	char	*sptr;
	char	systemDate [11];

	/*
	 * Company Close ? 
	 */
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);
 
	/*
	 * Company Owned Customer.
	 */
 	envDbCo = atoi (get_env ("DB_CO"));

	if (!strncmp (argv [0], "db_lfullpr", 10))
		expandedReport = FALSE;
	else
		expandedReport = TRUE;
		
	/*
	 * Nett or Gross Values.
	 */
	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Multi-Currency Customer ?
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if ageing is by days overdue or as per standard ageing. 
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * True Ageing or Module Ageing. 
	 */
	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	/*
	 * Report Types are as follows :                            
     *                                                          
 	 * 1 = Analysis period 1                                    
 	 * 2 = Analysis period 2                                    
 	 * 3 = Analysis period 3                                    
 	 * 4 = Analysis period 4                                    
 	 * 5 = Normal.                                              
 	 * 6 = Analysis of Customers over credit limit.             
	 *                                                          
	 * Report run type.                                         
	 *                                                          
	 * 1 = By company.                                          
	 * 2 = By branch inside company.                            
	 * 3 = By department inside branch inside comany.           
	 *                                                          
	 *                                                          
	 * Sort Type                                                
	 *                                                          
	 * 1 = By Customer Number.                                  
	 * 2 = By Acronym.                                          
	 *                                                          
	 * Zero balance flag.                                       
	 *                                                          
	 * 1 = Exclude zero balance customers.                      
	 * 2 = Print 0 balance customers only.                      
	 * 3 = Print all customers.                                 
	 */
	if (argc < 12)
	{
		print_at (0, 0, "Usage: %s <lpno>", argv [0]);
		print_at (1, 0, "           <rep_type> (1-8)");
		print_at (2, 0, "           <run_type> (1-3)");
		print_at (3, 0, "           <bal_type> (1-3)");
		print_at (4, 0, "           <sort_type> (1-2)");
		print_at (5, 0, "           <true_age> (M or T)");
		print_at (6, 0, "           <Local/Overseas Currency>");
		print_at (7, 0, "           <Sort by Currency, Y|N> ");
		print_at (8, 0, "           <Start Currency> <End Currency>");
		print_at (9, 0, "           <Age Days> <optional department> ");
		return (EXIT_FAILURE);
	}
	printerNo 	= atoi (argv [1]);
	reportType 	= atoi (argv [2]);
	runType 	= atoi (argv [3]);
	balanceType = atoi (argv [4]);
	sortType 	= atoi (argv [5]);
	sprintf (ageingType, "%-1.1s", argv [6]);
	sprintf (local_rec.currencyDisplay,	"%-1.1s", argv [7]);
	sprintf (local_rec.currencySort, 	"%-1.1s", argv [8]);
	sprintf (local_rec.startCurrency, 	"%-3.3s", argv [9]);
	sprintf (local_rec.endCurrency, 	"%-3.3s", argv [10]);

	currencyDisplay = (LCL_CURR) ? LCL : FGN;

	if (ageingType [0] == 'M')
		systemAge = FALSE;
	else
	{
		systemAge = TRUE;
		envDbTotalAge = TRUE;
	}

	envDbDaysAgeing = atoi (argv [11]);
	if (argc > 12)
		strcpy (department, argv [12]);

	/*
	 *	Allocate initial space for Details
	 */
	ArrAlloc (&dtls_d, &detailRec, sizeof (struct Detail), 1000);

	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	mendDate = (systemAge) ? MonthEnd (lsystemDate) 
						  : MonthEnd (comm_rec.dbt_date);


	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");
		
	dsp_screen ("Printing Customers Detailed Ageing Schedule.", 
			comm_rec.co_no, comm_rec.co_name);

	ReportHeading (printerNo);

	Process ();

	EndReport ();
	ArrDelete (&dtls_d);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
Process (void)
{
	int		dataFound;
	int		i;

	dataFound = FALSE;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;
	
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, (runType == BY_CO) ? "  " : branchNo);
	strcpy (cumr_rec.dbt_no, "      ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * Don't produce a trial balance for child customer.    
	     * Processing code will allow for this to be done but  
         * currently inputs don't allow for this flag. i.e    
         * process account to H/O ?                          
		 */
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		if (CURR_SORT &&
		   (strcmp (cumr_rec.curr_code, local_rec.startCurrency) < 0 ||
		     strcmp (cumr_rec.curr_code, local_rec.endCurrency) > 0))
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		switch (runType)	
		{
		case BY_BR:
			if (strcmp (cumr_rec.est_no, branchNo))
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}

			break;

		case BY_DP:
			if (strcmp (cumr_rec.est_no, branchNo))
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}
			else
			{
				if (strcmp (cumr_rec.department, department))
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}
			}

			break;
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
			"%-3.3s%-9.9s%010ld", 
			(CURR_SORT) ? cumr_rec.curr_code : " ", 
			(DBT_NUM) ? cumr_rec.dbt_no : cumr_rec.dbt_acronym, 
			 cumr_rec.hhcu_hash
		 );
		strcpy 
		(
			sortRec [sortCnt].currCode, 
			(CURR_SORT) ? cumr_rec.curr_code : " "
		);
		sortRec [sortCnt].hhcuHash = cumr_rec.hhcu_hash;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		dataFound = TRUE;

		dsp_process ("Customer : ", cumr_rec.dbt_no);

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	if (!dataFound)
		return (EXIT_SUCCESS);

	strcpy (previousCurrency, "");
	firstCurrency = TRUE;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (currentCurrency, "%-3.3s", sortRec [i].currCode);

		/*
		 * Lookup customer details.
		 */
		cumr_rec.hhcu_hash = sortRec [i].hhcuHash;
		cc = find_rec (cumr3, &cumr_rec, COMPARISON, "r");
		if (cc)
			continue;

		dsp_process ("Customer : ", cumr_rec.dbt_no);

		ProcessFile ();
	}
	if (CURR_SORT && !firstCurrency)
		CurrencyTotal (previousCurrency);
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);

	return (EXIT_SUCCESS);
}

void
EndReport (void)
{
	double	wk_dbt = 0.00, 
 		wk_crd = 0.00;

	int		i;

	if (CURR_SORT)
	{
		fprintf (pp, "!-------------");
		fprintf (pp, "+----------");
		fprintf (pp, "+----------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------|\n");
	}

	/*---------------------
	| print final totals. |
	---------------------*/
	fprintf (pp, ".LRP4\n");
	wk_dbt = debits [0] + debits [1] + debits [2] + 
		 debits [3] + debits [4] + debits [5];

	fprintf (pp, "! TOTAL DEBITS FOR RUN.             !");
	fprintf (pp, " %14.14s!", CF (wk_dbt));
	for (i = 0; i < 6; i++)
	{
		if (debits [i] != 0.00)
		{
			fprintf (pp, " %14.14s!", CF (debits [i]));
		}
		else
			fprintf (pp, "               !");
	}
	fprintf (pp, "\n");

	wk_crd = credits [0] + credits [1] + credits [2] + 
		 credits [3] + credits [4] + credits [5];

	fprintf (pp, "! TOTAL CREDITS FOR RUN.            !");
	fprintf (pp, " %14.14s!", CF (wk_crd));
	for (i = 0; i < 6; i++)
	{
		if (credits [i] != 0.00)
			fprintf (pp, " %14.14s!", CF (credits [i]));
		else
			fprintf (pp, "               !");
	}
	fprintf (pp, "\n");

	fprintf (pp, "!-------------");
	fprintf (pp, "!----------");
	fprintf (pp, "!----------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------!\n");
	
	if (runType == BY_CO)
		fprintf (pp, "! TOTAL BALANCE DUE FOR COMPANY     !");
	if (runType == BY_BR)
		fprintf (pp, "! TOTAL BALANCE DUE FOR BRANCH      !");
	if (runType == BY_DP)
		fprintf (pp, "! TOTAL BALANCE DUE FOR DEPARTMENT  !");

	fprintf (pp, " %14.14s!", CF (g_tots));
	for (i = 0; i < 6; i++)
	{
		if (g_tots != 0.00)
			percent [i] = totals [i] / g_tots * 100;
		else
			percent [i] = 0.00;

		if (totals [i] != 0.00)
			fprintf (pp, " %14.14s!", CF (totals [i]));
		else
			fprintf (pp, "               !");
	}
	fprintf (pp, "\n");

	fprintf (pp, "!-------------");
	fprintf (pp, "!----------");
	fprintf (pp, "!----------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------!\n");

       	fprintf (pp, "!* TOTAL PERCENTAGE BREAKDOWN. *    ");
       	fprintf (pp, "!        %6.2f%%", 100.00);
       	fprintf (pp, "!        %6.2f%%", percent [0]);
       	fprintf (pp, "!        %6.2f%%", percent [1]);
       	fprintf (pp, "!        %6.2f%%", percent [2]);
       	fprintf (pp, "!        %6.2f%%", percent [3]);
       	fprintf (pp, "!        %6.2f%%", percent [4]);
       	fprintf (pp, "!        %6.2f%%!\n", percent [5]);

	if (envDbMcurr)
		PrintCurrencySum ();

	fprintf (pp, ".EOF\n");

	pclose (pp);
}

void
ProcessFile (void)
{
	int		ok_print = 0, 
			i;

	double	cumr_bo [6];

	double	tot_balance = 0.00, 
			over1 = 0.00, 
			over2 = 0.00, 
			over3 = 0.00, 
			over4 = 0.00;

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
	tot_balance = cumr_bo [0] + cumr_bo [1] +
                  cumr_bo [2] + cumr_bo [3] +
                  cumr_bo [4] + cumr_bo [5];

	over1 =  cumr_bo [1] + cumr_bo [2] + 
		 	 cumr_bo [3] + cumr_bo [4];

	over2 =  cumr_bo [2] + cumr_bo [3] + 
	         cumr_bo [4];

	over3 =  cumr_bo [3] + cumr_bo [4];

	over4 =  cumr_bo [4];

	switch (reportType)	
	{
	    case MONTH_1:
		if (over1 != 0.00)
			ok_print = 1;
	    	break;

	    case MONTH_2:
		if (over2 != 0.00)
			ok_print = 1;
	    	break;

	    case MONTH_3:
		if (over3 != 0.00)
			ok_print = 1;
	    	break;

	    case MONTH_4:
		if (over4 != 0.00)
			ok_print = 1;
	    	break;

	    case NORMAL:
		ok_print = 1;
		break;

	    case OVER_LIMIT:
		if (tot_balance > cumr_rec.credit_limit)
			ok_print = 1;
	    	break;
	
	    case STOP_CREDIT:
		if (cumr_rec.stop_credit [0] == 'Y')
			ok_print = 1;
	    	break;
	}

	switch (balanceType)
	{
	case	EXCLUDE:
		if (MoneyZero (tot_balance))
			ok_print = 0;
		break;

	case	PRINT_0:
		if (!MoneyZero (tot_balance))
			ok_print = 0;
		break;

	default:
		break;
	}

	if (!ok_print)
		return;

	if (CO_DBT)
		useDate = comm_rec.dbt_date;
	else
		useDate = ReadEsmr (cumr_rec.co_no, cumr_rec.est_no);

	if (systemAge)
		mendDate = MonthEnd (lsystemDate);
	else
		mendDate = MonthEnd (useDate);

	firstPrint = 0;

	/*
	 * Process main customer.
	 */
	GetCheques (TRUE, cumr_rec.hhcu_hash);

	/*
	 * Process head office customers for a child.
	 */
	if (cumr_rec.ho_dbt_hash > 0L)
		GetCheques (FALSE, cumr_rec.ho_dbt_hash);

	/*
	 * Process all child customers for a head office.
	 */
	cumr2_rec.ho_dbt_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		GetCheques (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	ProcessInvoice (TRUE, cumr_rec.hhcu_hash);
	cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		ProcessInvoice (FALSE, cumr2_rec.hhcu_hash);
		cc = find_hash (cumr2, &cumr2_rec, NEXT, "r", cumr_rec.hhcu_hash);
	}

	if (firstPrint == 0)
	{
		PrintHeader ();
		firstPrint = 1;
	}

	/*
	 * print totals for customer. if totals != 0
	 */
	for (i = 0; i < 6; i++)
	{
		g_tot [LCL] += total [LCL][i];
		g_tot [FGN] += total [FGN][i];
	}

	if (firstPrint) 
	{
		g_tots += g_tot [LCL];
		fprintf (pp, "! *TOTAL DUE* !          !          !");
		fprintf (pp, " %14.14s!", CF (g_tot [currencyDisplay]));

		for (i = 0; i < 6; i++) 
		{
			if (total [currencyDisplay][i] != 0.00) 
			{
				fprintf (pp, " %14.14s!", CF (total [currencyDisplay][i]));
				totals [i] += total [LCL][i];
			}
			else
				fprintf (pp, "               !");
		}
		fprintf (pp, "\n");
		fprintf (pp, "!-------------");
		fprintf (pp, "+----------");
		fprintf (pp, "+----------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------");
		fprintf (pp, "+---------------|\n");
		for (i = 0; i < 6; i++)
		{
			total [LCL][i] = 0.00;
			total [FGN][i] = 0.00;
		}

		g_tot [LCL] = 0.00;
		g_tot [FGN] = 0.00;
	}
}

void
ProcessInvoice (
	int		clearTotal, 
	long	hhcuHash)
{
	cuin_rec.date_of_inv 	= 0L;
	cuin_rec.ho_hash 		= hhcuHash;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.ho_hash == hhcuHash)
	{
		if (envDbCo || runType == BY_CO)
		{
			PrintLine ();
			cc = find_rec (cuin, &cuin_rec, NEXT, "r");
			continue;
		}
        if (runType == BY_DP)
		{
			if (!strcmp (cuin_rec.est, comm_rec.est_no) &&
			     !strcmp (cuin_rec.dp, department))
						PrintLine ();
		}
		if (runType == BY_BR)
		{
			if (!strcmp (cuin_rec.est, comm_rec.est_no))
				PrintLine ();
		}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}

void
PrintHeader (void)
{
	fprintf (pp, ".LRP6\n");
	fprintf (pp, "! %-6.6s (%9.9s) (%-40.40s)     !CONTACT:%-20.20s  PHONE NO:%-10.10s                              !\n", 
			cumr_rec.dbt_no, 
			cumr_rec.dbt_acronym, 
			cumr_rec.dbt_name, 
			cumr_rec.contact_name, 
			cumr_rec.phone_no);

	fprintf (pp, "!   CREDIT LIMIT: %14.14s  ", CF (cumr_rec.credit_limit));

	fprintf (pp, "LAST PAYMENT:%10.10s %14.14s %-75.75s!\n", 
		DateToString (cumr_rec.date_lastpay), CF (cumr_rec.amt_lastpay), " ");
}

/*
 * Open database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	
	/*
	 * Read common terminal record .
	 */
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (cumr3, cumr);

	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_ho_cron");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumr3, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudp,  cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*
 * Close database Files.
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cumr3);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (esmr);
	abc_fclose (cudp);
	abc_fclose (pocr);

	abc_dbclose (data);
}

void
ReportHeading (
	int		printerNo)
{
	char	rep_desc [51];
	char	age_desc [51];

	/*
	 * Open pipe to pformat 
 	 */
	if ((pp = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
	
	if (envDbTotalAge)
	{
		if (systemAge)
			strcpy (age_desc, "Aged based on Current Date.");
		else
			strcpy (age_desc, "Aged based on Current Module Date.");
	}
	else
		strcpy (age_desc, "Aged based on Module month end Date.");

	switch (reportType)
	{

	case	MONTH_1:
		strcpy (rep_desc, "Analysis Overdue Period 1.");
		break;

	case	MONTH_2:
		strcpy (rep_desc, "Analysis Overdue Period 2.");
		break;

	case	MONTH_3:
		strcpy (rep_desc, "Analysis Overdue Period 3.");
		break;

	case	MONTH_4:
		strcpy (rep_desc, "Analysis Overdue Period 4+.");
		break;

	case	NORMAL:
		strcpy (rep_desc, "Analysis (ALL) ");
		break;

	case	OVER_LIMIT:
		strcpy (rep_desc, "Analysis (Customers over credit limit) ");
		break;

	case	STOP_CREDIT:
		sprintf (rep_desc, "Analysis (Customers on Stop Credit) ");
		break;
	}
	if (runType == BY_DP)
	{
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, department);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
			strcpy (cudp_rec.dp_short, "               ");
	}
		
	/*
	 * Start output to standard print.
	 */
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp, ".LP%d\n", printerNo);
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".%d\n", (CURR_SORT) ? 15 : 14);
	fprintf (pp, ".L158\n");
	fprintf (pp, ".E%s\n", clip (comm_rec.co_name));
	fprintf (pp, ".E%s\n", run_desc [runType - 1]);
	fprintf (pp, ".B1\n");
	fprintf (pp, ".C%s, %s, %s, %s\n", clip (rep_desc), 
			sort_desc [sortType -1], bal_desc [balanceType - 1], 
			age_desc);

	fprintf (pp, ".CNOTE : Forward dated payments are not included in report.\n");
	if (CURR_SORT)
	{
		fprintf (pp, 
			".CSorted By Currency For Range %s to %s. Values expressed in %s currency.\n", 
			local_rec.startCurrency, 
			local_rec.endCurrency, 
			(local_rec.currencyDisplay [0] == 'L') ? "Local" : "Overseas");
	}

	if (runType == BY_CO)
		fprintf (pp, ".EAS AT : %s\n", SystemTime ());

	if (runType == BY_BR)
		fprintf (pp, ".E%s AS AT : %s\n", clip (comm_rec.est_name), SystemTime ());

	if (runType == BY_DP)
		fprintf (pp, ".E%s / %s AS AT : %s\n", 
			clip (comm_rec.est_name), clip (cudp_rec.dp_short), SystemTime ());

	fprintf (pp, ".R==============");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "=================\n");

	fprintf (pp, "==============");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "=================\n");

	/*
	 * Line 1 of heading.
	 */
	if (runType == BY_CO)
	{
	    fprintf (pp, "!   BR/TRAN NO");
	    fprintf (pp, "!   DATE   ");
	    fprintf (pp, "!  TRANS   !");
	}

	if (runType == BY_BR || runType == BY_DP)
	{
	    fprintf (pp, "!   DP/TRAN NO");
	    fprintf (pp, "!   DATE   ");
	    fprintf (pp, "!  TRANS   !");
	}

	fprintf (pp, "%-45.45sB A L A N C E   D U E%-45.45s!\n", " ", " ");

	/*
	 * Line 2 of heading.
	 */
	fprintf (pp, "!             ");
	fprintf (pp, "!    OF    ");
	fprintf (pp, "!    DUE   ");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------!\n");

	/*
	 * Line 3 of heading.
	 */
	fprintf (pp, "!    NUMBER   ");
	fprintf (pp, "! TRANS.   ");
	fprintf (pp, "!   DATE   ");
	fprintf (pp, "!   TOTAL AMT   ");
	fprintf (pp, "!  CURRENT AMT  ");

	if (envDbDaysAgeing)
	{
		fprintf (pp, "!  1-%2d DAYS OD ",  envDbDaysAgeing);
		fprintf (pp, "! %2d-%2d DAYS OD ",  envDbDaysAgeing + 1, envDbDaysAgeing * 2);
		fprintf (pp, "! %2d-%2d DAYS OD ", (envDbDaysAgeing * 2) + 1, envDbDaysAgeing * 3);
		fprintf (pp, "!OVER %2d DAYS OD",  envDbDaysAgeing * 3);
	}
	else
	{
		fprintf (pp, "!   O/D PER 1   ");
		fprintf (pp, "!   O/D PER 2   ");
		fprintf (pp, "!   O/D PER 3   ");
		fprintf (pp, "!   O/D PER 4+  ");
	}
	fprintf (pp, "!   FWD AMOUNT  !\n");

	fprintf (pp, "!-------------");
	fprintf (pp, "!----------");
	fprintf (pp, "!----------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------");
	fprintf (pp, "!---------------!\n");

}

void
PrintLine (void)
{
	int		x = 0, 
			i; 
	int		inv_printed = FALSE;
	double	balance = 0.00;
	double	in_balance = 0.00;
	double	var_value = 0.00;
	double	tmp_value;
	double	fgn_value;
	double	fgn_payments;
	double	lcl_payments;
	double	var_exchange;
	char	wk_date [11];
	char	wk_date2 [11];
	char	wk_date3 [11];
	char	wk_date4 [11];
	int		prt_ok = FALSE;

	/*
	 * for each invoice, print details if dbt - crd <> 0.
	 */
	balance    = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			         	     : cuin_rec.amt;

	in_balance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			         	     : cuin_rec.amt;
	
	fgn_payments = 0.00;
	lcl_payments = 0.00;
	var_exchange = 0.00;
	for (i = 0; i < dtlsCnt; i++)
	{
		if (cuin_rec.hhci_hash == detailRec [i].hhciHash)
		{
			if (detailRec [i].invDueDate <= lsystemDate)
			{
				fgn_payments += detailRec [i].fgnAmount;
				lcl_payments += detailRec [i].locAmount;
				var_exchange += detailRec [i].exchVar;
			}
		}
	}

	if (cuin_rec.amt == 0.00 && fgn_payments == 0.00)
		return;
	/*
	 * Subtract payments from invoice amount.                
	 * Convert balance into local currency if Multi-Currency.
	 * Store currency summary information if Multi-Currency. 
	 */
	fgn_value = balance;
	if (envDbMcurr)
	{
		/*
		 * Convert balance into local currency.
	 	 */
		if (MoneyZero (cuin_rec.exch_rate))
			cuin_rec.exch_rate = 1.00;

		balance /= cuin_rec.exch_rate;
		if (LCL_CURR)
			in_balance = balance;
	}
	
	/*
	 * Subtract payments (local currency) from local value.
	 */
	balance   -= (lcl_payments - var_exchange);
	fgn_value -= fgn_payments;

	if (fgn_value == 0.00 && !expandedReport)
		return;

	/*
	 * Check for break in currency. 
	 */
	CheckBreak ();

	if (firstPrint == 0)
	{
		PrintHeader ();
		firstPrint = 1;
	}

	sprintf (wk_date2, "%-10.10s", DateToString (cuin_rec.date_of_inv));
	sprintf (wk_date3, "%-10.10s", DateToString (cuin_rec.due_date));
	
	fprintf (pp, ".LRP3\n");
	fprintf (pp, "!%s%s/%-8.8s!%10.10s!%10.10s!", 
	 		inv_type [atoi (cuin_rec.type) - 1], 
			(runType == BY_CO) ? cuin_rec.est : cuin_rec.dp, 
			cuin_rec.inv_no, 
			wk_date2, wk_date3);

	inv_printed = FALSE ;

	if (expandedReport)
	{
		var_value = 0.00;
	    for (i = 0; i < dtlsCnt; i++)
	    {
			if (cuin_rec.hhci_hash == detailRec [i].hhciHash)
			{
				if (!inv_printed)
					fprintf (pp, " %14.14s!", CF (in_balance));

				fprintf (pp, "               ");
				fprintf (pp, "!               ");
				fprintf (pp, "!               ");
				fprintf (pp, "!               ");
				fprintf (pp, "!               ");
				fprintf (pp, "!               !\n");

				if (LCL_CURR)
					in_balance = - detailRec [i].locAmount;
				else
					in_balance = - detailRec [i].fgnAmount;
				
				strcpy (wk_date, DateToString (detailRec [i].invDop));
				strcpy (wk_date4, DateToString (detailRec [i].invDueDate));

				fprintf (pp, "! %s /%-8.8s!%10.10s!%-10.10s! %14.14s!", 
					inv_type [atoi (cuin_rec.type) + 2], 
					detailRec [i].receiptNo, 
					wk_date, 
					wk_date4, 
					CF (in_balance));

				inv_printed = TRUE;
				var_value += detailRec [i].exchVar;
			}
	    }
	}
	if (!inv_printed)
		fprintf (pp, "               !");
	else
	if (expandedReport)
	{
		fprintf (pp, "               ");
		fprintf (pp, "!               ");
		fprintf (pp, "!               ");
		fprintf (pp, "!               ");
		fprintf (pp, "!               ");
		fprintf (pp, "!               !\n");

		if (var_value != 0.00)
		{
			fprintf (pp, "! %s   %-6.6s!%-10.10s!          ! %-14.14s!", 
				"VAR", " ", " ", CF (var_value));
			if (balance == 0.00)
			{
				for (i = 0; i < 6; i++) 
					fprintf (pp, "               !");
				fprintf (pp, "\n");
				prt_ok = TRUE;
			}
		}
		else if (balance != 0.00)
		{
			fprintf (pp, "! %-3.3s   %-6.6s!%-10.10s!          ! %-14.14s!", 
				" ", " ", " ", " ");
		}
	}

	/*
	 * Before deposits are being aged even if due date is zero.
	 */
	if (cuin_rec.due_date > 0L)
	{
		x = AgePeriod 
			(
				cuin_rec.pay_terms, 
				cuin_rec.date_of_inv, 
				(systemAge) ? lsystemDate : mendDate, 
				cuin_rec.due_date, 
				envDbDaysAgeing, 
				envDbTotalAge
			);

		if (x == -1)
			x = 5;
	}

	if (envDbMcurr)
        SumCurrency (cuin_rec.currency, fgn_value, balance, x);

	if (balance == 0.00 && var_value == 0.00)
		prt_ok = TRUE;

	if (balance != 0.00)
	{
		for (i = 0; i < 6; i++) 
		{
			if (x == i)
			{
				tmp_value = (LCL_CURR) ? balance : fgn_value;
				fprintf (pp, " %14.14s!", CF (tmp_value));
				if (balance < 0.00)
					credits [i] += balance;
				else
					debits [i]  += balance;

				total [LCL][i] += balance;
				total [FGN][i] += fgn_value;
			} 
			else
				fprintf (pp, "               !");
		}
		fprintf (pp, "\n");
	}
	else if (!prt_ok)
	{	
		for (i = 0; i < 6; i++) 
			fprintf (pp, "               !");
		fprintf (pp, "\n");
	}
}

int
CheckBreak (void)
{
	if (!CURR_SORT)
		return (EXIT_SUCCESS);

	/*
	 * Change in currency.
	 */
	if (strcmp (previousCurrency, currentCurrency))
	{
		if (!firstCurrency)
			CurrencyTotal (previousCurrency);
		
		CurrencyHeader (currentCurrency);

		strcpy (previousCurrency, currentCurrency);
		firstCurrency = FALSE;
	}

	return (EXIT_SUCCESS);
}

void
CurrencyHeader (
	char	*currency)
{
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", currency);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", "Currency Not Found");

	fprintf (pp, 
		".PD| CURRENCY : %-3.3s - %-40.40s  %-85.85s  |\n", 
		currency, 
		pocr_rec.description, 
		" ");

	if (!firstCurrency)
		fprintf (pp, ".PA\n");

	fflush (pp);
}

/*
 *  Print currency totals.
 */
int
CurrencyTotal (
 char*              crrcy)
{
	int		i;
	int		curr_fnd;
	double	curr_tot;
	struct	CURR_LST  *lcl_ptr;

	curr_fnd = FALSE;
	lcl_ptr = currentHead; 
	while (lcl_ptr != CURR_NULL)
	{
		if (!strcmp (lcl_ptr->code, crrcy))
		{
			curr_fnd = TRUE;
			break;
		}
		lcl_ptr = lcl_ptr->next;
	}

	if (!curr_fnd)
		return (EXIT_SUCCESS);

	/*
	 * Foreign currency totals.
	 */
	curr_tot = 0.00;
	for (i = 0; i < 6; i++)
		curr_tot += lcl_ptr->total [i];

	fprintf (pp, "| TOTAL (%-3.3s)                       ", lcl_ptr->code); 

	fprintf (pp, "| %14.14s", CF (curr_tot));

	for (i = 0; i < 6; i++)
		fprintf (pp, "| %14.14s", CF (lcl_ptr->total [i]));
	
	fprintf (pp, "|\n");

	/*
	 * Foreign currency percentages.
	 */
	fprintf (pp, "|    PERCENTAGE BREAKDOWN           ");
	fprintf (pp, "|        100.00%%");
	fprintf (pp, "|       %7.2f%%", (lcl_ptr->total [0] / curr_tot) * 100.00);
	fprintf (pp, "|       %7.2f%%", (lcl_ptr->total [1] / curr_tot) * 100.00);
	fprintf (pp, "|       %7.2f%%", (lcl_ptr->total [2] / curr_tot) * 100.00);
	fprintf (pp, "|       %7.2f%%", (lcl_ptr->total [3] / curr_tot) * 100.00);
	fprintf (pp, "|       %7.2f%%", (lcl_ptr->total [4] / curr_tot) * 100.00);
	fprintf (pp, "|       %7.2f%%|\n",(lcl_ptr->total [5] / curr_tot) * 100.00);

	/*
	 * Local currency totals
	 */
	curr_tot = 0.00;
	for (i = 0; i < 6; i++)
		curr_tot += lcl_ptr->lcl_tot [i];

	fprintf (pp, "| TOTAL %-25.25s   ", "LOCAL CURRENCY");

	fprintf (pp, "| %14.14s", CF (curr_tot));

	for (i = 0; i < 6; i++)
		fprintf (pp, "| %14.14s", CF (lcl_ptr->lcl_tot [i]));
	
	fprintf (pp, "|\n");

	return (EXIT_SUCCESS);
}

/*
 * Print currency summary.
 */
void
PrintCurrencySum (void)
{
	int		i;
	double	curr_total;
	struct	CURR_LST  *lcl_ptr;

	fprintf (pp, "|=================");
	fprintf (pp, "==================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================|\n");

	fprintf (pp, "|%-8.8sCURRENCY BREAKDOWN%-8.8s ", " ", " ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               |\n");

	lcl_ptr = currentHead;
	while (lcl_ptr != CURR_NULL)
	{
		curr_total = 0.00;
		for (i = 0; i < 6; i++)
			curr_total += lcl_ptr->total [i];

		fprintf (pp, "| %-3.3s %-30.30s", lcl_ptr->code, lcl_ptr->desc);

		fprintf (pp, "| %14.14s", CF (curr_total));

		for (i = 0; i < 6; i++)
			fprintf (pp, "| %14.14s", CF (lcl_ptr->total [i]));
		
		fprintf (pp, "|\n");

		lcl_ptr = lcl_ptr->next;
	}

}

void
GetCheques (
	int    	clearTotal, 
	long	hhcuHash)
{
	if (clearTotal)
		dtlsCnt = 0;

	cuhd_rec.hhcu_hash	=	hhcuHash;
    cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
    while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
    {
		if (cuhd_rec.date_payment > mendDate)
		{
    		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		}
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
    	cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
		while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
		{
			if (!ArrChkLimit (&dtls_d, detailRec, dtlsCnt))
				sys_err ("ArrChkLimit (detailRec)", ENOMEM, PNAME);

	    	strcpy (detailRec [dtlsCnt].receiptNo, cuhd_rec.receipt_no);
	    	detailRec [dtlsCnt].hhciHash = cudt_rec.hhci_hash;
	    	detailRec [dtlsCnt].invDop = cuhd_rec.date_payment;
			if (cuhd_rec.present_date > lsystemDate)
	    		detailRec [dtlsCnt].invDueDate = cuhd_rec.present_date;
			else
	    		detailRec [dtlsCnt].invDueDate = cuhd_rec.date_posted;

			detailRec [dtlsCnt].fgnAmount = cudt_rec.amt_paid_inv;
			detailRec [dtlsCnt].locAmount = cudt_rec.loc_paid_inv;
	    	detailRec [dtlsCnt++].exchVar = cudt_rec.exch_variatio;
    		cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}
    	cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*
 * This must stay to ensure current date all ways used.
 */
long
ReadEsmr (
	char	*coNo, 
	char	*brNo)
{
	sprintf (esmr_rec.co_no, "%-2.2s", coNo);
	sprintf (esmr_rec.est_no, "%-2.2s", brNo);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		return (comm_rec.dbt_date);

	return (esmr_rec.dbt_date);
}

/*
 * Store the value against its currency
 */
int
SumCurrency (
	char	*currencyCode, 
	double	storeValue, 
	double	localValue, 
	int		period)
{
	int		node_fnd;
	int		pos_fnd;
	struct	CURR_LST *lcl_ptr, *tmp_ptr;

	if (storeValue == 0.00)
		return (EXIT_SUCCESS);

	if (period == -1)
		period = 5;

	/*
	 * Find node in linked list for currency if it exists. Otherwise make one.
	 */
	node_fnd = FALSE;
	lcl_ptr = currentHead;
	while (lcl_ptr != CURR_NULL)
	{
		if (!strcmp (lcl_ptr->code, currencyCode))
		{
			lcl_ptr->total [period] += storeValue;
			lcl_ptr->lcl_tot [period] += localValue;

			node_fnd = TRUE;
			return (EXIT_SUCCESS);
		}

		lcl_ptr = lcl_ptr->next;
	}

	/*
	 * Create a new currency node.
	 */
	if (!node_fnd)
	{
		/*
		 * Lookup currency description.
		 */
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", currencyCode);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			sprintf (pocr_rec.description, "%-40.40s", " ");

		/*
		 * Allocate memory and store data.
		 */
		lcl_ptr = CurrencyAllocate ();
		sprintf (lcl_ptr->code, "%-3.3s", currencyCode);
		sprintf (lcl_ptr->desc, "%-40.40s", pocr_rec.description);
		lcl_ptr->total [period] = storeValue;
		lcl_ptr->lcl_tot [period] = localValue;

		/*
		 * Insert into list.
		 */
		pos_fnd = FALSE;
		tmp_ptr = currentHead;
		while (tmp_ptr != CURR_NULL)
		{
			if (strcmp (lcl_ptr->code, tmp_ptr->code) < 0)
			{
				pos_fnd = TRUE;
				break;
			}

			tmp_ptr = tmp_ptr->next;
		}

		if (!pos_fnd)
		{
			/*
			 * No position found so append to end
			 */
			if (tmp_ptr == currentHead)
			{
				/*
				 * Insert at head
				 */
				currentHead = lcl_ptr;
				lcl_ptr->prev = CURR_NULL;
				lcl_ptr->next = CURR_NULL;
				currentTail = currentHead;
			}
			else
			{
				/*
				 * Append to tail
				 */
				currentTail->next = lcl_ptr;
				lcl_ptr->prev = currentTail;
				lcl_ptr->next = CURR_NULL;
				currentTail = lcl_ptr;
			}
		}
		else
		{
			/*
			 * Position found so insert
			 */
			if (tmp_ptr == currentHead)
			{
				/*
				 * Insert before head
				 */
				lcl_ptr->next = currentHead;
				tmp_ptr->prev = lcl_ptr;
				lcl_ptr->prev = CURR_NULL;
				currentHead = lcl_ptr;

			}
			else
			{
				/*
				 * Insert in middle
				 */
				tmp_ptr->prev->next = lcl_ptr;
				lcl_ptr->prev = tmp_ptr->prev;
				lcl_ptr->next = tmp_ptr;
				tmp_ptr->prev = lcl_ptr;
			}
		}
	}

	return (EXIT_SUCCESS);
}

/*
 * Allocate memory to store currency details. 
 */
struct CURR_LST *
CurrencyAllocate (void)
{
	int		i;
	struct CURR_LST *lcl_ptr	=	NULL;

	i = 0;
	while (i < 100)
	{
		lcl_ptr = (struct CURR_LST *)malloc (sizeof (struct CURR_LST));
		if (lcl_ptr != CURR_NULL)
			break;
		i++;
		sleep (sleepTime);
	}

	if (lcl_ptr == CURR_NULL)
		sys_err ("Error in CurrencyAllocate () During (MALLOC)", 12, PNAME);

	strcpy (lcl_ptr->code, "   ");
	sprintf (lcl_ptr->desc, "%-40.40s", " ");
	for (i = 0; i < 6; i++)
	{
		lcl_ptr->total [i] = 0.00;
		lcl_ptr->lcl_tot [i] = 0.00;
	}
	lcl_ptr->prev = CURR_NULL;
	lcl_ptr->next = CURR_NULL;

	return (lcl_ptr);
}

/*
 *	Minor support functions
 */
static int
MoneyZero (
 double	m)
{
	return (fabs (m) < 0.0001);
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

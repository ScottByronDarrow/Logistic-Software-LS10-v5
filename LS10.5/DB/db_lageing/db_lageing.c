/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lageing.c,v 5.4 2002/08/14 04:25:34 scott Exp $
|  Program Name  : (db_lageing.c  )                                   |
|  Program Desc  : (Summary Customers Ageing Schedule           )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/01/87         |
|---------------------------------------------------------------------|
| $Log: db_lageing.c,v $
| Revision 5.4  2002/08/14 04:25:34  scott
| Updated for Linux warning
|
| Revision 5.3  2002/03/07 05:34:44  scott
| Updated to remove sort routines.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lageing.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lageing/db_lageing.c,v 5.4 2002/08/14 04:25:34 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<arralloc.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include    <errno.h>
#include 	<arralloc.h>

#include	"schema"

	int		trueAge;
	int		DaysAgeing;


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

#define	SMAN		1
#define	CUST_TYPE	2

#define	EXCLUDE		1
#define	PRINT_0		2
#define	PRINT_ALL	3

#define	LCL		0
#define	FGN		1

#define	CO_DBT		 (envCoClose [0] == '1')

#define	LCL_CURR	 (local_rec.currencyPrint [0] == 'L' || !envDbMcurr)
#define	ALT_SORT	 (salesmanCust == SMAN || salesmanCust == CUST_TYPE)
#define	CURR_SORT	 (local_rec.currencySort [0] == 'Y' && envDbMcurr)
#define	DBT_NUM		 (sortType == NUMBER)

	struct	commRecord	comm_rec;
	struct	esmrRecord	esmr_rec;
	struct	cumrRecord	cumr_rec;
	struct	cumrRecord	cumr2_rec;
	struct	exsfRecord	exsf_rec;
	struct	exclRecord	excl_rec;
	struct	cudpRecord	cudp_rec;
	struct	cuinRecord	cuin_rec;
	struct	cuhdRecord	cuhd_rec;
	struct	cudtRecord	cudt_rec;
	struct	pocrRecord	pocr_rec;

	char	*data  	= "data",
			*cumr2  = "cumr2",
			*cumr3  = "cumr3";

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;


/*
 * The CURR_LST linked list is  used to store the totals for each currency. 
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
#define	CURR_NULL	 ((struct CURR_LST *) NULL)
struct	CURR_LST *curr_head = CURR_NULL;
struct	CURR_LST *curr_tail = CURR_NULL;

/*
 * The structures 'dtls' are initialised in function 'GetCheques'      
 * the number of details is stored in external variable 'dtlsCount'.  
 */
struct	Detail {      
	long	hhci_hash;	/* detail invoice reference.	     */
	long	inv_date;   /* detail posting date/ present date */
	double	inv_amt;	/* detail invoice amount.	         */
	double	exch_var;	/* detail exchange variation.        */
	double	lcl_amt;	/* detail invoice amount. (lcl curr) */
}	*dtls;

	DArray	dtls_d;		/* state info for dyanamic array "dtls" */
	int		dtlsCount;

struct SortStruct
{
	char	sortCode [27];
	char	currCode [4];
	char	altSort  [4];	
	long	hhcuHash;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

	double	grandTotal [2];
	double	total [2] [6];

	double	reportTotal [2] [6];
	double	singleGrandTotal;

	double	gSubTotal [2];
	double	subTotals [2] [6];

	int		reportType		= 5;
	int		runType			= 2;
	int		balanceType		= 1;
	int		sortType		= 2;
	int		salesmanCust	= 0;
	int		printerNumber	= 1;
	int		systemAge		= FALSE;
	int		envDbCo			= FALSE;
	int		envDbNettUsed	= TRUE;
	int		envDbMcurr		= FALSE;
	int		firstCurrency	= TRUE;
	int		firstAlternate	= TRUE;

	long	useDate = 0L;
	long	monthEndDate = 0L;
	long	lsystemDate = 0L;

	double	reportValue [4];
	double	percent [6];

	char	envCoClose [6];
	char	department [3];
	char	branchNumber [3];
	char	ageType [2];
	char	previousCurrency [4];
	char	currentCurrency [4];
	char	previousValue [4];
	char	currentValue [4];
	
	FILE	*pp;
	
	static char *run_desc [] = {
		"SUMMARY AGEING SCHEDULE BY COMPANY",
		"SUMMARY AGEING SCHEDULE BY BRANCH",
		"SUMMARY AGEING SCHEDULE BY DEPARTMENT"
	};

	static char *sort_desc [] = {
		"Sorted By Customer Number ",
		"Sorted By Customer Acronym ",
		"Sorted By Salesman Number / Customer Number ",
		"Sorted By Salesman Number / Customer Acronym",
		"Sorted By Customer Type / Customer Number ",
		"Sorted By Customer Type / Customer Acronym",
	};

	static char *bal_desc [] = {
		"Exclude zero balance Customers.",
		"Excluding non-zero balance Customers.",
		"Including zero and non-zero balance Customers."
	};

struct
{
	char	currencyPrint [2];
	char	currencySort [2];
	char	startCurrency [4];
	char	endCurrency [4];

} local_rec;

/*
 * Local Function Prototypes.
 */
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	shutdown_prog 	 (void);
void 	ReportHeading 	 (int);
int 	MainProcessLoop  (void);
void 	ProcessFile 	 (void);
int 	BalanceCheck 	 (int);
void 	CheckBreak 		 (void);
void 	CurrencyHeader 	 (char *);
int 	CurrencyTotals 	 (char *);
void 	PrintTotal 		 (int);
void 	PrintHeader 	 (int);
void 	GetDetails 		 (void);
void 	ProcessInvoice 	 (int, long);
int 	PrintLine 		 (void);
void 	GetCheques 		 (int, long);
void 	PrintCurrSumm 	 (void);
long 	ReadEsmr 		 (char *, char *);
int 	SumCurrency 	 (char *, double, double, int);
struct 	CURR_LST *curr_alloc (void);
void 	ProcessList 	 (void);
int		SortFunc		 (const	void *,	const void *);

int
main (
 int                argc,
 char*              argv [])
{
	char	systemDate [11];
	char	*sptr;

	/*
	 * True Ageing or Module Ageing. 
	 */
	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		trueAge = FALSE;
	else
		trueAge = (*sptr == 'T' || *sptr == 't');

	/*
	 * Multi-Currency Customers ? 
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if ageing is by days overdue or as per standard ageing. 
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	DaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Nett Value Customers ? 
	 */
	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Company Month End ? 
	 */
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	/*
	 * Company Owned Customers ? 
	 */
 	envDbCo = atoi (get_env ("DB_CO"));

	/*
	 *  Report Types are as follows :                             
     *                                                            
 	 *  1 = Analysis Period 1                                     
 	 *  2 = Analysis Period 2                                     
 	 *  3 = Analysis Period 3                                     
 	 *  4 = Analysis Period 4                                     
 	 *  5 = Normal.                                               
 	 *  6 = Analysis of Customers over credit limit.              
 	 *  7 = Stop Credit Listing.                                  
     *                                                            
	 *  Report run type.                                          
	 *                                                            
	 *  1 = By company.                                           
	 *  2 = By branch inside company.                             
	 *  3 = By department inside branch inside comany.            
	 *                                                            
	 *                                                            
	 *  Sort Type                                                 
	 *                                                            
	 *  1 = By Customer Number.                                   
	 *  2 = By Acronym.                                           
	 *                                                            
	 *  Zero balance flag.                                        
	 *                                                            
	 *  1 = Exclude zero balance customer.                        
	 *  2 = Print 0 balance customer only.                        
	 *  3 = Print all customers.                                  
	 *                                                            
	 *  1 = By Salesman.                                          
	 *  2 = By Customer Type.                                     
	 */

	if (argc != 17 && argc != 18)
	{
		print_at (0,0, mlStdMess036 ,argv [0]);
		print_at (1,0, mlDbMess145);
		print_at (2,0, mlDbMess146);
		print_at (3,0, mlDbMess147);
		print_at (4,0, mlDbMess148);
		print_at (5,0, mlDbMess149);
		print_at (6,0, mlDbMess151);
		print_at (7,0, mlDbMess152);
		print_at (8,0, mlDbMess153);
		print_at (9,0, mlDbMess154);
		print_at (10,0, mlDbMess155);
		print_at (11,0, mlDbMess156);
		print_at (12,0, mlDbMess157);
		print_at (13,0, mlDbMess158);
		print_at (14,0, mlDbMess159);
        return (EXIT_FAILURE);
	}

	printerNumber   = atoi (argv [1]);
	reportType  	= atoi (argv [2]);
	runType  		= atoi (argv [3]);
	balanceType  	= atoi (argv [4]);
	sortType 		= atoi (argv [5]);
	salesmanCust 	= atoi (argv [6]);
	sprintf (ageType, "%-1.1s", argv [7]);
	
	reportValue [0] = CENTS (atof (argv [8]));
	reportValue [1] = CENTS (atof (argv [9]));
	reportValue [2] = CENTS (atof (argv [10]));
	reportValue [3] = CENTS (atof (argv [11]));

	if (envDbMcurr)
	{
		sprintf (local_rec.currencyPrint, 	"%-1.1s", argv [12]);
		sprintf (local_rec.currencySort, 	"%-1.1s", argv [13]);
		sprintf (local_rec.startCurrency, 	"%-3.3s", argv [14]);
		sprintf (local_rec.endCurrency, 	"%-3.3s", argv [15]);
	}
	else
	{
		strcpy (local_rec.currencyPrint, "L");
		strcpy (local_rec.currencySort, "N");
	}

	DaysAgeing = atoi (argv [16]);
	if (argc == 18)
		sprintf (department, "%-2.2s", argv [17]);

	if (ageType [0] == 'M')
		systemAge = FALSE;
	else
	{
		trueAge = TRUE;
		systemAge  = TRUE;
	}

	/*
	 *	Allocate initial 1000 lines of detail
	 */
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	monthEndDate = (systemAge) ? MonthEnd (lsystemDate) 
						  	 : MonthEnd (comm_rec.dbt_date);

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	dsp_screen ("Printing Customers Summary Ageing Schedule.",
		   comm_rec.co_no, comm_rec.co_name);

	ReportHeading (printerNumber);

	if (MainProcessLoop () == 1)
        return (EXIT_FAILURE);

	shutdown_prog ();
    return (EXIT_SUCCESS);
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

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumr3, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	if (salesmanCust == SMAN)
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	else
	if (salesmanCust == CUST_TYPE)
		open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_ho_cron");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*
 * Close database Files. 
 */
void
CloseDB (void)
{
	if (salesmanCust == SMAN)
		abc_fclose (exsf);
	else
		if (salesmanCust == CUST_TYPE)
			abc_fclose (excl);

	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cumr3);
	abc_fclose (esmr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cudp);
	abc_fclose (pocr);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	PrintTotal (TRUE);
	pclose (pp);
	CloseDB (); 
	ArrDelete (&dtls_d);
	FinishProgram ();
}

void
ReportHeading (
 int                prnt_no)
{
	char	rep_desc [41];
	char	age_desc [41];

	/*
	 * Open pipe to pformat 
 	 */
	if ((pp = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if (trueAge)
	{
		if (systemAge)
			strcpy (age_desc, ML ("Aged based on Current Date."));
		else
			strcpy (age_desc, ML ("Aged based on Current Module Date."));
	}
	else
		strcpy (age_desc, ML ("Aged based on Module month end Date."));

	switch (reportType)
	{

	case	MONTH_1:
		strcpy (rep_desc, ML ("Analysis Overdue Period 1."));
		break;

	case	MONTH_2:
		strcpy (rep_desc, ML ("Analysis Overdue Period 2."));
		break;

	case	MONTH_3:
		strcpy (rep_desc, ML ("Analysis Overdue Period 3."));
		break;

	case	MONTH_4:
		strcpy (rep_desc, ML ("Analysis Overdue Period 4+."));
		break;

	case	NORMAL:
		strcpy (rep_desc, ML ("Analysis (ALL) "));
		break;

	case	OVER_LIMIT:
		strcpy (rep_desc, ML ("Analysis (Customers over credit limit) "));
		break;

	case	STOP_CREDIT:
		sprintf (rep_desc, ML ("Analysis (Customers on Stop Credit) "));
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
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	fprintf (pp,".PI12\n");
	fprintf (pp,".%d\n", (CURR_SORT) ? 15 : 14);
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",run_desc [runType - 1]);
	fprintf (pp,".CNOTE : Forward Dated Payments are not included in report.\n");

	if (CURR_SORT)
	{
		fprintf (pp, 
			".CSorted By Currency For Range %s to %s. Values expressed in %s currency.\n",
			local_rec.startCurrency,
			local_rec.endCurrency,
			 (local_rec.currencyPrint [0] == 'L') ? "Local" : "Overseas");
	}

	switch (salesmanCust)
	{
	case	0:
		fprintf (pp, ".C%s,%s,%s,%s\n", clip (rep_desc),
						sort_desc [sortType -1],
						bal_desc [balanceType - 1],
						age_desc);
		break;

	case	1:
		fprintf (pp, ".C%s,%s,%s,%s\n", clip (rep_desc),
						sort_desc [sortType + 1],
						bal_desc [balanceType - 1],
						age_desc);
		break;

	case	2:
		fprintf (pp, ".C%s,%s,%s,%s\n", clip (rep_desc),
						sort_desc [sortType + 3],
						bal_desc [balanceType - 1],
						age_desc);
		break;
	}

	if (runType == BY_CO)
		fprintf (pp,".B1\n");

	if (runType == BY_BR)
		fprintf (pp,".E%s\n",clip (comm_rec.est_name));

	if (runType == BY_DP)
	{
		fprintf (pp,
			".E%s / %s \n",
			clip (comm_rec.est_name),
			clip (cudp_rec.dp_short));
	}

	/*
	 * print overdue cutoff			
	 */
	fprintf (pp, ".CValue in Overdue Period 1 > %4.2f  ", DOLLARS (reportValue [0]));
	fprintf (pp, "  Value in Overdue Period 2 > %4.2f  ", DOLLARS (reportValue [1]));
	fprintf (pp, "  Value in Overdue Period 3 > %4.2f  ", DOLLARS (reportValue [2]));
	fprintf (pp, "  Value in Overdue Period 4 > %4.2f\n", DOLLARS (reportValue [3]));

	fprintf (pp,".EAS AT : %s\n",SystemTime ());

	fprintf (pp, ".R=================================");
	fprintf (pp, "================================");
	fprintf (pp, "==============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "==============\n");

	fprintf (pp, "=================================");
	fprintf (pp, "================================");
	fprintf (pp, "==============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "==============\n");

	fprintf (pp, "|  CUSTOMER            CREDIT      ");
	fprintf (pp, " LAST PAYMENT    ");
	fprintf (pp, "|    TOTAL     ");
	fprintf (pp, "|   CURRENT    ");

	if (DaysAgeing)
	{
		fprintf (pp, "|  1 - %2d DAYS ",  DaysAgeing);
		fprintf (pp, "| %2d - %2d DAYS ", DaysAgeing + 1, 	  DaysAgeing * 2);
		fprintf (pp, "| %2d - %2d DAYS ", (DaysAgeing * 2) + 1,DaysAgeing * 3);
		fprintf (pp, "| OVER %2d DAYS ", DaysAgeing * 3);
	}
	else
	{
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
	}

	fprintf (pp, "|   FORWARD    |\n");

	fprintf (pp, "|                   LIMIT TERMS ");
	fprintf (pp, "  DATE      AMOUNT  ");
	fprintf (pp, "|   AMOUNT     ");
	fprintf (pp, "|   AMOUNT     ");

	if (DaysAgeing)
	{
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
		fprintf (pp, "|   OVERDUE    ");
	}
	else
	{
		fprintf (pp, "|  PERIOD 1    ");
		fprintf (pp, "|  PERIOD 2    ");
		fprintf (pp, "|  PERIOD 3    ");
		fprintf (pp, "|  PERIOD 4+   ");
	}
	fprintf (pp, "|   AMOUNT     |\n");

	fprintf (pp, "|-------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------|\n");

	fflush (pp);
}

int
MainProcessLoop (void)
{
	int		i;
	int		data_fnd;
	char	alt_sort [4];

	switch (sortType)
	{
	case	NUMBER :
		abc_selfield (cumr, "cumr_id_no");
		strcpy (cumr_rec.dbt_no, "      ");
		break;

	case	ACRONYM :
		abc_selfield (cumr, "cumr_id_no2");
		strcpy (cumr_rec.dbt_acronym, "         ");
		break;
	}

	data_fnd = FALSE;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;
	
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, (runType == BY_CO) ? "  " : branchNumber);

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
			if (strcmp (cumr_rec.est_no, branchNumber))
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}
			break;

		case BY_DP:
			if (strcmp (cumr_rec.est_no, branchNumber))
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

		default:
			break;
		}

		switch (salesmanCust)
		{
		case SMAN:
			sprintf (alt_sort, "%2.2s ", cumr_rec.sman_code);
			break;

		case CUST_TYPE:
			sprintf (alt_sort, "%-3.3s", cumr_rec.class_type);
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
			"%-3.3s%3.3s%-9.9s%010ld",
			(CURR_SORT) ? cumr_rec.curr_code : " ",
			alt_sort,
			(DBT_NUM) ? cumr_rec.dbt_no : cumr_rec.dbt_acronym,
			cumr_rec.hhcu_hash
		);
		strcpy (sortRec [sortCnt].currCode, cumr_rec.curr_code);
		strcpy (sortRec [sortCnt].altSort, 	alt_sort);
		sortRec [sortCnt].hhcuHash = cumr_rec.hhcu_hash;
		/*
		 * Increment array counter.
		 */
		sortCnt++;
	
		data_fnd = TRUE;

		dsp_process ("Customer : ", cumr_rec.dbt_no);
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	if (!data_fnd)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	/*
	 * Process sort file. 
	 */
	strcpy (previousCurrency, "");
	strcpy (previousValue, 	  "");
	firstCurrency	= TRUE;
	firstAlternate	= TRUE;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		sprintf (currentCurrency, "%-3.3s", sortRec [i].currCode);
		sprintf (currentValue,   "%3.3s",   sortRec [i].altSort);

		/*
		 * Lookup customer details. 
		 */
		cumr_rec.hhcu_hash	=	sortRec [i].hhcuHash;
		cc = find_rec (cumr3,&cumr_rec,COMPARISON, "r");
		if (cc)
			continue;
		
		dsp_process ("Customer : ", cumr_rec.dbt_no);
		ProcessFile ();
	}

	/*
	 * Print last sman/ctype totals 
	 */
	if (ALT_SORT)
		PrintTotal (FALSE);

	/*
	 * Print last currency totals 
	 */
	if (CURR_SORT && !firstCurrency)
		CurrencyTotals (previousCurrency);

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);

	return (EXIT_SUCCESS);
}

/*
 * Validate and print lines. 
 */
void
ProcessFile (void)
{
	char	wk_limit [32];
	char	wk_last [32];
	double	cumr_bo [6];

	int		okToPrint		= FALSE,
			validBalance	= FALSE,
			i;

	int		currencyPrint;
	double	totalBalance = 0.00,
			over1 = 0.00,
			over2 = 0.00,
			over3 = 0.00,
			over4 = 0.00,
			tmp_total;

	currencyPrint = (LCL_CURR) ? LCL : FGN;

	for (i = 0; i < 6; i++)
		cumr_bo [i] = cumr_balance [i];

	cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		for (i = 0; i < 6; i++)
			cumr_bo [i] += cumr2_balance [i];
		
		cc = find_hash (cumr2, &cumr2_rec, NEXT, "r", cumr_rec.hhcu_hash);
	}
	totalBalance = cumr_bo [0] + cumr_bo [1] +
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
			okToPrint = TRUE;
		break;

	case MONTH_2:
		if (over2 != 0.00)
			okToPrint = TRUE;
		break;

	case MONTH_3:
		if (over3 != 0.00)
			okToPrint = TRUE;
		break;

	case MONTH_4:
		if (over4 != 0.00)
			okToPrint = TRUE;
		break;

	case NORMAL:
		okToPrint = TRUE;
		break;

	case OVER_LIMIT:
		if (totalBalance > cumr_rec.credit_limit)
			okToPrint = TRUE;
	    	break;
	
	case STOP_CREDIT:
		if (cumr_rec.stop_credit [0] == 'Y')
			okToPrint = TRUE;
	    	break;
	}

	switch (balanceType)
	{
	case	EXCLUDE:
		if (totalBalance == 0.00)
			okToPrint = FALSE;
		break;

	case	PRINT_0:
		if (totalBalance != 0.00)
			okToPrint = FALSE;
		break;

	default:
		break;
	}

	/*
	 * Validate overdue period one values. 
	 */
	if (reportValue [0] > 0.00)
	{
		validBalance = BalanceCheck (1);
		if (validBalance == 0 && reportValue [1] > 0.00)
			validBalance = BalanceCheck (2);

		if (validBalance == 0 && reportValue [2] > 0.00)
			validBalance = BalanceCheck (3);

		if (validBalance == 0 && reportValue [3] > 0.00)
			validBalance = BalanceCheck (4);
	}
	/*
	 * Validate overdue period two values. 
	 */
	if (!validBalance && reportValue [1] > 0.00)
	{
		validBalance = BalanceCheck (2);
		if (validBalance == 0 && reportValue [2] > 0.00)
			validBalance = BalanceCheck (3);

		if (validBalance == 0 && reportValue [3] > 0.00)
			validBalance = BalanceCheck (4);
	}
	/*
	 * Validate overdue period three values. 
	 */
	if (!validBalance && reportValue [2] > 0.00)
	{
		validBalance = BalanceCheck (3);
		if (validBalance == 0 && reportValue [3] > 0.00)
			validBalance = BalanceCheck (4);
	}
	/*
	 * Validate overdue period four values. 
	 */
	if (!validBalance && reportValue [3] > 0.00)
		validBalance = BalanceCheck (4);

	if (reportValue [0] == 0.00 && reportValue [1] == 0.00 && 
             	reportValue [2] == 0.00 && reportValue [3] == 0.00)
		validBalance = TRUE;

	if (okToPrint == FALSE || validBalance == FALSE)
		return;

	/*
	 * Check for change of currency OR sman OR cust_type.        
	 */
	CheckBreak ();

	if (CO_DBT)
		useDate = comm_rec.dbt_date;
	else
		useDate = ReadEsmr (cumr_rec.co_no, cumr_rec.est_no);

	if (systemAge)
		monthEndDate = MonthEnd (lsystemDate);
	else
		monthEndDate = MonthEnd (useDate);

	GetDetails ();

	/*
	 * Print totals for customer.
	 */
	for (i = 0; i < 6; i++)
	{
		grandTotal [LCL] += total [LCL] [i];
		grandTotal [FGN] += total [FGN] [i];
	}

	if (cumr_rec.credit_limit == 0)
		strcpy (wk_limit," None. ");
	else
		sprintf (wk_limit,"%7.0f", DOLLARS (cumr_rec.credit_limit));

	if (cumr_rec.amt_lastpay == 0)
		strcpy (wk_last, " None.    ");
	else
		sprintf (wk_last, "%10.2f",DOLLARS (cumr_rec.amt_lastpay));

	fprintf (pp, "|%-6.6s", cumr_rec.dbt_no);
	fprintf (pp, "(%-9.9s)", cumr_rec.dbt_acronym);
	fprintf (pp, " %7.7s (%3.3s)%10.10s%10.10s",
		    wk_limit,
		    cumr_rec.crd_prd,
		    DateToString (cumr_rec.date_lastpay),
		    wk_last);
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              |\n");

	fprintf (pp, "| %-40.40s          |", cumr_rec.dbt_name);
	fprintf (pp, "              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              |\n");

	fprintf (pp, "|CONTACT %-20.20s", cumr_rec.contact_name);
	fprintf (pp, "PHONE   %-15.15s|", cumr_rec.phone_no);

	fprintf (pp,"%14.14s", comma_fmt (DOLLARS (grandTotal [currencyPrint]),"NNN,NNN,NNN.NN"));
	for (i = 0; i < 6; i++) 
	{
		tmp_total = total [currencyPrint] [i];
		if (tmp_total != 0.00) 
		{
			fprintf (pp, 
				"|%14.14s",
				comma_fmt (DOLLARS (tmp_total),"NNN,NNN,NNN.NN"));
		}
		else
			fprintf (pp,"|              ");
	}
	fprintf (pp, "|\n");

	for (i = 0; i < 6; i++)
	{
		reportTotal [LCL] [i] += total [LCL] [i];
		reportTotal [FGN] [i] += total [FGN] [i];
		if (ALT_SORT)
		{
			subTotals [LCL] [i] += total [LCL] [i];
			subTotals [FGN] [i] += total [FGN] [i];
		}
	}

	fprintf (pp, "|                         ");
	fprintf (pp, "                          ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              |\n");

	if (ALT_SORT)
	{
		gSubTotal [LCL] += grandTotal [LCL];
		gSubTotal [FGN] += grandTotal [FGN];
	}

	singleGrandTotal += grandTotal [LCL];
	grandTotal [LCL] = 0.00;
	grandTotal [FGN] = 0.00;

	for (i = 0; i < 6; i++)
	{
		total [LCL] [i] = 0.00;
		total [FGN] [i] = 0.00;
	}
}

/*
 * Check backorder balance. 
 */
int
BalanceCheck (
 int                bal_per)
{
	if (cumr_balance [bal_per] > reportValue [bal_per - 1])
		return (TRUE);

	return (FALSE);
}

void
CheckBreak (void)
{
	if (CURR_SORT)
	{
		/*
		 * Print first headings. 
		 */
		if (firstCurrency)
		{
			CurrencyHeader (currentCurrency);
			strcpy (previousCurrency, currentCurrency);
			firstCurrency = FALSE;
	
			if (ALT_SORT)
			{
				PrintHeader (TRUE);
				strcpy (previousValue, currentValue);
				firstAlternate	= FALSE;
			}
		}

		/*
		 * Change in currency. 
		 */
		if (strcmp (previousCurrency, currentCurrency))
		{
			if (ALT_SORT)
			{
				strcpy (previousValue, currentValue);
				PrintTotal (FALSE);
			}

			CurrencyTotals (previousCurrency);
			CurrencyHeader (currentCurrency);

			if (ALT_SORT)
				PrintHeader (firstAlternate);
	
			strcpy (previousCurrency, currentCurrency);
		}

		if (ALT_SORT && strcmp (previousValue, currentValue))
		{
			strcpy (previousValue, currentValue);

			PrintTotal (FALSE);
			PrintHeader (firstAlternate);
		}

		return;
	}

	if (ALT_SORT && (firstAlternate || strcmp (previousValue, currentValue)))
	{
		strcpy (previousValue, currentValue);
		if (!firstAlternate)
			PrintTotal (FALSE);
		PrintHeader (firstAlternate);
		firstAlternate = FALSE;
	}
}

void
CurrencyHeader (
 char*              crrcy)
{
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", crrcy);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", "Currency Not Found");

	fprintf (pp,
		".PD| CURRENCY : %-3.3s - %-40.40s  %-95.95s |\n",
		crrcy,
		pocr_rec.description,
		" ");

	if (!firstCurrency)
		fprintf (pp, ".PA\n");

	fflush (pp);
}

/*
 * Print currency totals.
 */
int
CurrencyTotals (
 char*              crrcy)
{
	int		i;
	int		curr_fnd;
	double	curr_tot;
	struct	CURR_LST  *lcl_ptr;

	curr_fnd = FALSE;
	lcl_ptr = curr_head; 
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

	fprintf (pp, "| TOTAL (%-40.40s)  ", lcl_ptr->desc); 

	fprintf (pp, 
		"|%14.14s", 
		comma_fmt (DOLLARS (curr_tot), "NNN,NNN,NNN.NN"));

	for (i = 0; i < 6; i++)
	{
		fprintf (pp, "|%14.14s", 
			comma_fmt (DOLLARS (lcl_ptr->total [i]),"NNN,NNN,NNN.NN"));
	}
	fprintf (pp, "|\n");

	/*
	 * Foreign currency percentages. 
	 */
	fprintf (pp, "|    PERCENTAGE BREAKDOWN        ");
	fprintf (pp, "                   ");
	fprintf (pp, "|       100.00%%");
	fprintf (pp, "|      %7.2f%%",  (lcl_ptr->total [0] / curr_tot) * 100.00);
	fprintf (pp, "|      %7.2f%%",  (lcl_ptr->total [1] / curr_tot) * 100.00);
	fprintf (pp, "|      %7.2f%%",  (lcl_ptr->total [2] / curr_tot) * 100.00);
	fprintf (pp, "|      %7.2f%%",  (lcl_ptr->total [3] / curr_tot) * 100.00);
	fprintf (pp, "|      %7.2f%%",  (lcl_ptr->total [4] / curr_tot) * 100.00);
	fprintf (pp,"|      %7.2f%%|\n", (lcl_ptr->total [5] / curr_tot) * 100.00);

	/*
	 * Local currency totals 
	 */
	curr_tot = 0.00;
	for (i = 0; i < 6; i++)
		curr_tot += lcl_ptr->lcl_tot [i];

	fprintf (pp, "| TOTAL %-40.40s    ", "LOCAL CURRENCY");

	fprintf (pp, "|%14.14s", comma_fmt (DOLLARS (curr_tot), "NNN,NNN,NNN.NN"));

	for (i = 0; i < 6; i++)
	{
		fprintf (pp, "|%14.14s", 
			comma_fmt (DOLLARS (lcl_ptr->lcl_tot [i]),"NNN,NNN,NNN.NN"));
	}
	fprintf (pp, "|\n");

	return (EXIT_SUCCESS);
}

/*
 * Print totals and end report to pformat. 
 */
void
PrintTotal (
	int		end)
{
	int		i;
	int		currencyPrint;

	currencyPrint = (LCL_CURR) ? LCL : FGN;

	if (!CURR_SORT)
	{
		fprintf (pp, "|--------------------------------");
		fprintf (pp, "-------------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------|\n");
	}

	if (end)
	{
		switch (runType)
		{
		case	BY_BR:
			fprintf (pp, "| *** TOTAL BALANCE DUE FOR BRANC");
			fprintf (pp, "H ***              ");
			break;

		case	BY_DP:
			fprintf (pp, "| *** TOTAL BALANCE DUE FOR DEPAR");
			fprintf (pp, "TMENT ***          ");
			break;

		case	BY_CO:
		default:
			fprintf (pp, "| *** TOTAL BALANCE DUE FOR COMPA");
			fprintf (pp, "NY ***             ");
			break;
		}
		
		for (i = 0; i < 6; i++)
		{
			percent [i] = (singleGrandTotal != 0.00) ? reportTotal [LCL] [i] / 
				      singleGrandTotal * 100 : 0.00;
		}

		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (singleGrandTotal), "NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (reportTotal [LCL] [0]),"NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (reportTotal [LCL] [1]),"NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (reportTotal [LCL] [2]),"NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (reportTotal [LCL] [3]),"NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s",    
			comma_fmt (DOLLARS (reportTotal [LCL] [4]),"NNN,NNN,NNN.NN"));
		fprintf (pp, "|%14.14s|\n", 
			comma_fmt (DOLLARS (reportTotal [LCL] [5]),"NNN,NNN,NNN.NN"));

		fprintf (pp, "| *** TOTAL PERCENTAGE BREAKDOWN ");
		fprintf (pp, "***                ");
		fprintf (pp, "|       100.00%%");
		fprintf (pp, "|      %7.2f%%",   percent [0]);
		fprintf (pp, "|      %7.2f%%",   percent [1]);
		fprintf (pp, "|      %7.2f%%",   percent [2]);
		fprintf (pp, "|      %7.2f%%",   percent [3]);
		fprintf (pp, "|      %7.2f%%",   percent [4]);
		fprintf (pp, "|      %7.2f%%|\n",percent [5]);

		/*
		 * Print currency summary. 
		 */
		if (envDbMcurr)
			PrintCurrSumm ();

		fprintf (pp,".EOF\n");
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			percent [i] = (gSubTotal [currencyPrint] != 0.00) ? 
					 (subTotals [currencyPrint] [i] / gSubTotal [currencyPrint] * 100) : 0.00;
		}

		switch (salesmanCust)
		{
		case	SMAN:
			fprintf (pp, "| *** TOTAL BALANCE FOR SALESMAN ");
			fprintf (pp, "***                ");
			fprintf (pp, 
				"|%14.14s",    
				comma_fmt (DOLLARS (gSubTotal [currencyPrint]), "NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [0]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [1]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [2]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [3]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [4]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s|\n", 
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [5]),"NNN,NNN,NNN.NN"));

			break;
		case	CUST_TYPE:
			fprintf (pp, "| *** TOTAL BALANCE FOR CUSTOMER ");
			fprintf (pp, "TYPE ***           ");
			fprintf (pp, 
				"|%14.14s",    
				comma_fmt (DOLLARS (gSubTotal [currencyPrint]), "NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [0]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [1]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [2]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [3]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s",    
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [4]),"NNN,NNN,NNN.NN"));
			fprintf (pp, 
			       "|%14.14s|\n", 
			       comma_fmt (DOLLARS (subTotals [currencyPrint] [5]),"NNN,NNN,NNN.NN"));
			break;
		}
		fprintf (pp, "| *** TOTAL PERCENTAGE BREAKDOWN ");
		fprintf (pp, "***                ");
		fprintf (pp, "|       100.00%%");
		fprintf (pp, "|      %7.2f%%",   percent [0]);
		fprintf (pp, "|      %7.2f%%",   percent [1]);
		fprintf (pp, "|      %7.2f%%",   percent [2]);
		fprintf (pp, "|      %7.2f%%",   percent [3]);
		fprintf (pp, "|      %7.2f%%",   percent [4]);
		fprintf (pp, "|      %7.2f%%|\n",percent [5]);
		
		gSubTotal [LCL] = 0;
		gSubTotal [FGN] = 0;
		for (i = 0; i < 6; i++)
		{
			subTotals [LCL] [i] = 0;
			subTotals [FGN] [i] = 0;
			percent [i] = 0;
		}

		if (CURR_SORT)
		{
			fprintf (pp, "|--------------------------------");
			fprintf (pp, "-------------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------");
			fprintf (pp, "|--------------|\n");
		}
	}
}

void
PrintHeader (
 int                first_time)
{
	switch (salesmanCust)
	{
	case	SMAN:
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		sprintf (exsf_rec.salesman_no,"%-2.2s",cumr_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			strcpy (exsf_rec.salesman,"No Salesman Found");

		strcpy (err_str,exsf_rec.salesman);
		break;

	case	CUST_TYPE:
		strcpy (excl_rec.co_no,comm_rec.co_no);
		sprintf (excl_rec.class_type,"%-3.3s",cumr_rec.class_type);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
			strcpy (excl_rec.class_desc,"No Description Found");

		strcpy (err_str,excl_rec.class_desc);
		break;
	}

	switch (salesmanCust)
	{
	case SMAN:
		fprintf (pp, 
			"%s| %2.2s - %40.40s     ",
			 (CURR_SORT) ? "" : ".PD",
			cumr_rec.sman_code, 
			err_str);
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              |\n");
		break;
	
	case CUST_TYPE:
		fprintf (pp, 
			"%s| %3.3s - %40.40s    ",
			 (CURR_SORT) ? "" : ".PD",
			cumr_rec.class_type, 
			err_str);
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              ");
		fprintf (pp, "|              |\n");
		break;
	}
	
	if (!first_time && !CURR_SORT)
		fprintf (pp,".PA\n");
}

void
GetDetails (void)
{
	/*
	 * Process main customer. 
	 */
	GetCheques (TRUE, cumr_rec.hhcu_hash);

	/*
	 * Process head office customer for a child. 
	 */
	if (cumr_rec.ho_dbt_hash > 0L)
		GetCheques (FALSE, cumr_rec.ho_dbt_hash);

	/*
	 * Process all child customer for a head office. 
	 */
	cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		GetCheques (FALSE, cumr2_rec.hhcu_hash);
		cc = find_hash (cumr2, &cumr2_rec, NEXT,"r", cumr_rec.hhcu_hash);
	}

	ProcessInvoice (TRUE, cumr_rec.hhcu_hash);
	cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		ProcessInvoice (FALSE, cumr2_rec.hhcu_hash);
		cc = find_hash (cumr2, &cumr2_rec, NEXT, "r", cumr_rec.hhcu_hash);
	}
}

void
ProcessInvoice (
 int                clear_tot,
 long               hhcu_hash)
{
	cuin_rec.date_of_inv = 0L;
	cuin_rec.ho_hash = hhcu_hash;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.ho_hash == hhcu_hash)
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
			{
				PrintLine ();
			}
		}
		if (runType == BY_BR)
		{
	    	if (!strcmp (cuin_rec.est, comm_rec.est_no))
				PrintLine ();
	
    	}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}

int
PrintLine (void)
{
	int		x = 0, 
		i; 
	double	balance = 0.00;
	double	fgn_value;
	double	fgn_payments;
	double	lcl_payments;
	double	var_exchange;

	/*
	 * for each invoice, print details if dbt - crd <> 0. 
	 */
	balance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			      		      : cuin_rec.amt;

	/*
	 * Accumulate payments for the invoice. 
	 */
	fgn_payments = 0.00;
	lcl_payments = 0.00;
	var_exchange = 0.00;
	for (i = 0; i < dtlsCount; i++)
	{
		if (cuin_rec.hhci_hash == dtls [i].hhci_hash  &&
				dtls [i].inv_date <= lsystemDate)
		{
			fgn_payments += dtls [i].inv_amt;
			lcl_payments += dtls [i].lcl_amt;
			var_exchange += dtls [i].exch_var;
		}
	}
 	if (cuin_rec.amt == 0.00 && fgn_payments == 0.00)
        return (FALSE);

	/*
	 * Calculate aged period. 
	 */
	if (cuin_rec.due_date > 0L)
	{
		x  = AgePeriod 
		 (
			cuin_rec.pay_terms,
			cuin_rec.date_of_inv,
			 (systemAge) ? lsystemDate : monthEndDate,
			cuin_rec.due_date,
			DaysAgeing,
			trueAge
		);
	}

	/*
	 * Subtract payments from invoice amount.
	 * Convert balance to local currency if Multi-Currency
	 * Store currency summary information if Multi-Currency
	 */
	fgn_value = balance;
	if (envDbMcurr)
	{
		/*
		 * Convert foreign value to local currency. 
		 */
		if (cuin_rec.exch_rate != 0.00)
			balance /= cuin_rec.exch_rate;

		/*
		 * Subtract payments (local currency) from local value. 
		 */
		balance -= (lcl_payments - var_exchange);
	}
	else
		balance -= lcl_payments;

	/*
	 * Store currency details. 
	 */
	if (envDbMcurr)
	{
		SumCurrency 
		(
			cuin_rec.currency, 
			fgn_value - fgn_payments, 
			balance, 
			x
		);
	}

	if (x == -1)
	{
		total [LCL] [5] += balance;
		total [FGN] [5] += (fgn_value - fgn_payments);
	}
	else
	{
		total [LCL] [x] += balance;
		total [FGN] [x] += (fgn_value - fgn_payments);
	}
	return (TRUE);
}

void
GetCheques (
 int                clear_tot,
 long               hhcuHash)
{
	if (clear_tot)
		dtlsCount = 0;

	cuhd_rec.hhcu_hash	=	hhcuHash;
    cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
    while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
    {
		if (cuhd_rec.date_payment > monthEndDate)
		{
    		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		}
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
    	cc = find_rec (cudt, &cudt_rec,GTEQ,"r");
		while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
		{
			if (!ArrChkLimit (&dtls_d, dtls, dtlsCount))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

			if (cuhd_rec.present_date > lsystemDate)
                dtls [dtlsCount].inv_date = cuhd_rec.present_date;
            else
                dtls [dtlsCount].inv_date = cuhd_rec.date_payment;

	    	dtls [dtlsCount].hhci_hash	= cudt_rec.hhci_hash;
	    	dtls [dtlsCount].inv_amt		= cudt_rec.amt_paid_inv;
	    	dtls [dtlsCount].exch_var	= cudt_rec.exch_variatio;
	    	dtls [dtlsCount++].lcl_amt	= cudt_rec.loc_paid_inv;
    		cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}
    	cc = find_rec (cuhd,&cuhd_rec, NEXT,"r");
	}
}

/*
 * Print currency summary.
 */
void
PrintCurrSumm (void)
{
	int		i;
	double	curr_total;
	struct	CURR_LST  *lcl_ptr;

	fprintf (pp, "|=========================");
	fprintf (pp, "==========================");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============|\n");

	fprintf (pp, "|%-16.16sCURRENCY BREAKDOWN%-16.16s ", " ", " ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              |\n");

	lcl_ptr = curr_head;
	while (lcl_ptr != CURR_NULL)
	{
		curr_total = 0.00;
		for (i = 0; i < 6; i++)
			curr_total += lcl_ptr->total [i];

		fprintf (pp, 
			"|  %-3.3s     %-40.40s ", 
			lcl_ptr->code, 
			lcl_ptr->desc);

		fprintf (pp, 
			"|%14.14s", 
			comma_fmt (DOLLARS (curr_total), "NNN,NNN,NNN.NN"));

		for (i = 0; i < 6; i++)
		{
			fprintf (pp, "|%14.14s", 
				comma_fmt (DOLLARS (lcl_ptr->total [i]),"NNN,NNN,NNN.NN"));
		}
		fprintf (pp, "|\n");

		lcl_ptr = lcl_ptr->next;
	}
}

/*======================================================
| This must stay to ensure current date all ways used. |
======================================================*/
long
ReadEsmr (
 char*              co_no,
 char*              br_no)
{
	sprintf (esmr_rec.co_no, "%-2.2s", co_no);
	sprintf (esmr_rec.est_no,"%-2.2s", br_no);
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
 char*              curr_code,
 double             store_val,
 double             lcl_val,
 int                period)
{
	int		node_fnd;
	int		pos_fnd;
	struct	CURR_LST *lcl_ptr, *tmp_ptr;

	if (store_val == 0.00)
		return (EXIT_SUCCESS);

	if (period == -1)
		period = 5;

	/*
	 * Find node in linked list for currency if it exists. Otherwise make one
	 */
	node_fnd = FALSE;
	lcl_ptr = curr_head;
	while (lcl_ptr != CURR_NULL)
	{
		if (!strcmp (lcl_ptr->code, curr_code))
		{
			lcl_ptr->total [period] += store_val;
			lcl_ptr->lcl_tot [period] += lcl_val;

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
		sprintf (pocr_rec.code, "%-3.3s", curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			sprintf (pocr_rec.description, "%-40.40s", " ");

		/*
		 * Allocate memory and store data. 
		 */
		lcl_ptr = curr_alloc ();
		sprintf (lcl_ptr->code, "%-3.3s", curr_code);
		sprintf (lcl_ptr->desc, "%-40.40s", pocr_rec.description);
		lcl_ptr->total [period] = store_val;
		lcl_ptr->lcl_tot [period] = lcl_val;

		/*
		 * Insert into list. 
		 */
		pos_fnd = FALSE;
		tmp_ptr = curr_head;
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
			/*------------------------------------
			| No position found so append to end |
			------------------------------------*/
			if (tmp_ptr == curr_head)
			{
				/*----------------
				| Insert at head |
				----------------*/
				curr_head = lcl_ptr;
				lcl_ptr->prev = CURR_NULL;
				lcl_ptr->next = CURR_NULL;
				curr_tail = curr_head;
			}
			else
			{
				/*----------------
				| Append to tail |
				----------------*/
				curr_tail->next = lcl_ptr;
				lcl_ptr->prev = curr_tail;
				lcl_ptr->next = CURR_NULL;
				curr_tail = lcl_ptr;
			}
		}
		else
		{
			/*--------------------------
			| Position found so insert |
			--------------------------*/
			if (tmp_ptr == curr_head)
			{
				/*--------------------
				| Insert before head |
				--------------------*/
				lcl_ptr->next = curr_head;
				tmp_ptr->prev = lcl_ptr;
				lcl_ptr->prev = CURR_NULL;
				curr_head = lcl_ptr;

			}
			else
			{
				/*------------------
				| Insert in middle |
				------------------*/
				tmp_ptr->prev->next = lcl_ptr;
				lcl_ptr->prev = tmp_ptr->prev;
				lcl_ptr->next = tmp_ptr;
				tmp_ptr->prev = lcl_ptr;
			}
		}
	}

	return (EXIT_SUCCESS);
}

/*--------------------------
| Allocate memory to store |
| currency details.        |
--------------------------*/
struct CURR_LST *
curr_alloc (void)
{
	int		i;
	struct CURR_LST *lcl_ptr = NULL;

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
		sys_err ("Error in curr_alloc () During (MALLOC)", 12, PNAME);

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

void
ProcessList (void)
{
	int		i;
	static	int		in_wide = FALSE;
	struct	CURR_LST *lcl_ptr;

	if (!in_wide)
	{
		swide ();
		in_wide = TRUE;
	}

	i = 0;
	lcl_ptr = curr_head;
	while (lcl_ptr != CURR_NULL)
	{
		if (i > 20)
		{
			print_at (23, 0, ML (mlStdMess042));
			getchar ();
			clear ();
			i = 0;
		}

		print_at (i++, 
			0, 
			" CURRENCY [%s]  [%f]  [%f]  [%f]  [%f]  [%f]  [%f]",
			lcl_ptr->code,
			DOLLARS (lcl_ptr->total [0]),
			DOLLARS (lcl_ptr->total [1]),
			DOLLARS (lcl_ptr->total [2]),
			DOLLARS (lcl_ptr->total [3]),
			DOLLARS (lcl_ptr->total [4]),
			DOLLARS (lcl_ptr->total [5]));

		lcl_ptr = lcl_ptr->next;
	}

	print_at (23, 0, ML (mlStdMess042));
	getchar ();
	clear ();
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

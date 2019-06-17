/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cl_prt.c,v 5.5 2002/08/14 04:03:59 scott Exp $
|  Program Name  : (db_cl_prt.c)
|  Program Desc  : (Print Customer Information Report)
|---------------------------------------------------------------------|
|  Date Written  : (15/06/1998)    | Author      : Ana Marie C. Tario |
|---------------------------------------------------------------------|
| $Log: db_cl_prt.c,v $
| Revision 5.5  2002/08/14 04:03:59  scott
| Updated for Linux warnings
|
| Revision 5.4  2002/07/17 09:57:04  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/21 07:40:06  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cl_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cl_prt/db_cl_prt.c,v 5.5 2002/08/14 04:03:59 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>

#define LCL	  	  1
#define FGN	  	  0
#define LOC_CURR   (strcmp (cumr_rec.curr_code, comr_rec.base_curr) == 0)
#define	NUM_SORT	 (local_rec.sortType [0] == 'N')


#define	CF(x)		comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN")
FILE	*pp, *fsort;

#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct comrRecord	comr_rec;
struct pocrRecord	pocr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

	char	*data  = "data",
			*cumr2 = "cumr2";

	float	whtax_pc			= 	0.00;

	double 	currentTotal		=	0.00, 
			totalAR				=	0.00,
			creditSales			=	0.00,
			totalCreditSales	=	0.00,
			totalCreditLimit	=	0.00,
			totalAccReceipt		=	0.00,
			totalCurrent		=	0.00,
			overDue1			=	0.00,
			overDue2			=	0.00,
			totalOverdue1		=	0.00,
			totalOverdue2		=	0.00;

	int		envDbCo,
			envDbDaysAgeing,
	   		envDbFind,
	   		printerNo = 1,
	   		envDbNettUsed = TRUE,
	   		envDbMcurr = FALSE;
	
	char	branchNo [3];

	extern	int		TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	int		printerNo;
	char 	back [2];
	char 	backDesc [5];
	char	sortType [2];
	char	sortTypeDesc [2];
	char	onite [2];
	char	oniteDesc [5];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "sortType",	3, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Sort By N(umber) or A(cronym)  ", " ",
		YES, NO,  JUSTLEFT, "NA", "", local_rec.sortType},
	{1, LIN, "printerNo",	 	5, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number                 ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",		6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background                     ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	6, 36, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	7, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight                      ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onightDesc",	7, 36, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*------------------------------
| The CURR_LST linked list is  |
| used to store the totals for |
| each currency.               |
------------------------------*/
struct  CURR_LST
{
    char    code [4];
    char    desc [41];
    double  credit_limit;
    double  creditSales;
    double  totalAR;
    double  cur_total;
    double  overDue1;
    double  overDue2;
    struct  CURR_LST    *prev;
    struct  CURR_LST    *next;
};

#define CURR_NULL  ((struct CURR_LST *) NULL)
struct  CURR_LST *curr_head = CURR_NULL;
struct  CURR_LST *curr_tail = CURR_NULL;

/*
 * Local Function Prototypes.
 */
void 	RunProgram 				(char *);
int 	spec_valid 				(int);
void 	ProcessCumr 			(void);
void 	GetCreditSales 			(double *);
void 	ProcessFile 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	shutdown_prog 			(void);
void 	ReportHeading 			(int);
void 	PrintRuleLine 			(void);
void 	PrintTotal 				(void);
struct 	CURR_LST *CurrAlloc 	(void);
int 	SummCurrency 			(void);
int 	heading 				(int);

int
main (
 int                argc,
 char*              argv [])
{
	char    *sptr;

	TruePosition	=	TRUE;

 	envDbCo 	= atoi (get_env ("DB_CO"));
 	envDbFind  	= atoi (get_env ("DB_FIND"));

	if (argc != 1 && argc != 3)
	{
	    print_at (0,0, "Usage %s <printerNo> Optional <sortType>", argv [0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	OpenDB ();

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
 		file_err (cc, comr, "DBFIND");
	
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (argc == 3)
	{
		printerNo = atoi (argv [1]);
		sprintf (local_rec.sortType,"%-1.1s", argv [2]);
		ReportHeading (printerNo);
		dsp_screen ("Printing Customer Credit Sales Report.",
										comm_rec.co_no, comm_rec.co_name);

		totalCreditSales 	= 0,
		totalCreditLimit 	= 0,
		totalAccReceipt   	= 0,
		totalCurrent   		= 0,
		totalOverdue1  		= 0,
		totalOverdue2  		= 0;

		ProcessCumr ();

		fprintf (pp,".EOF\n");
		pclose (pp);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*
	 * Setup required parameters
	 */
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*
		 * Entry screen 1 linear input
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
	char	*programName)
{
	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT \"%s\" \"%d\" \"%s\" \"%s\"", 
			programName,
			local_rec.printerNo,
			local_rec.sortType,
            "Print Customer Sales Information Report."
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"%s\"",
			programName,
			local_rec.printerNo,
			local_rec.sortType
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

int
spec_valid (
	int		field)
{
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

	return (EXIT_SUCCESS);
}

void
ProcessCumr (void)
{
	if (local_rec.sortType [0] == 'A')
		abc_selfield (cumr, "cumr_id_no2");

	memset (&cumr_rec, 0, sizeof cumr_rec);
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, comm_rec.est_no);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
  		/*
		 * Exclude child debtors for now.
		 */
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		ProcessFile ();

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");

	PrintTotal ();

}

void
GetCreditSales (
	double	*balance)
{
	*balance = 0.00;

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuin_rec.hhcu_hash) 
	{
		if (envDbNettUsed)
			*balance += cuin_rec.amt - cuin_rec.disc;
		else
			*balance += cuin_rec.amt;
		cudt_rec.hhci_hash = cuin_rec.hhci_hash;
		cc = find_rec (cudt, &cudt_rec, EQUAL, "r");
		while (!cc && cuin_rec.hhci_hash == cudt_rec.hhci_hash)
		{
			*balance -= cudt_rec.amt_paid_inv;
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}

void
ProcessFile (void)
{
	int		i;

	double	tot_balance = 0.00;
	double	cumr_bo [6];

	dsp_process ("Customer  ",cumr_rec.dbt_acronym);


	for (i = 0; i < 6; i++)
		cumr_bo [i] = cumr_balance [i];

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		for (i = 0; i < 6; i++)
			cumr_bo [i] += cumr2_balance [i];
		
		cc = find_rec (cumr2, &cumr2_rec, NEXT,"r");
	}
	tot_balance = 	(cumr_bo [0] + cumr_bo [1] + cumr_bo [2] + 
                     cumr_bo [3] + cumr_bo [4] + cumr_bo [5]);

	if (tot_balance == 0.00)
		return;


	fprintf (pp, "|  %-8.8s",   cumr_rec.dbt_no);
	fprintf (pp, "   (%-40.40s) ", cumr_rec.dbt_name);
	fprintf (pp, "%-3.3s%-79.79s|\n",   cumr_rec.curr_code," ");

	currentTotal 	  = 0.00;
	overDue1	  = 0.00;
	overDue2     = 0.00;	 		


	GetCreditSales (&creditSales);

	totalAR     = 	cumr_balance [0] + 
					cumr_balance [1] + 
					cumr_balance [2] + 
					cumr_balance [3] + 
					cumr_balance [4] + 
					cumr_balance [5];

	currentTotal = 	cumr_balance [0] + 
					cumr_balance [5];

	overDue1	= 	cumr_balance [1];

	overDue2	= 	cumr_balance [2] +
					cumr_balance [3] +
					cumr_balance [4];

	if (!LOC_CURR)
	{
 		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (pocr_rec.description, " ");
			pocr_rec.ex1_factor = 0.00;
		}
	}
	else
		pocr_rec.ex1_factor = 1.00;

	if (pocr_rec.ex1_factor > 0.00)
	{
		totalCreditSales 	+= creditSales 				/ pocr_rec.ex1_factor;
		totalCreditLimit 	+= cumr_rec.credit_limit 	/ pocr_rec.ex1_factor;
		totalAccReceipt  	+= totalAR 					/ pocr_rec.ex1_factor;
		totalCurrent  		+= currentTotal 			/ pocr_rec.ex1_factor;
		totalOverdue1  		+= overDue1 				/ pocr_rec.ex1_factor;
		totalOverdue2  		+= overDue2 				/ pocr_rec.ex1_factor;
	}
	
	if (envDbMcurr)
		SummCurrency ();

	fprintf (pp, "|%14.14s",   CF (cumr_rec.credit_limit));
	fprintf (pp, "| %-3.3s ",	cumr_rec.crd_prd);
	fprintf (pp, "|%16.16s",	CF (creditSales));
	fprintf (pp, "|%14.14s", 	CF (totalAR));
	fprintf (pp, "|%14.14s", 	CF (currentTotal));
	fprintf (pp, "|%14.14s", 	CF (overDue1));
	fprintf (pp, "|%14.14s",	CF (overDue2));
	fprintf (pp, "|%14.14s", 	CF (cumr_rec.ord_value));
	fprintf (pp, "|%-10.10s",	DateToString (cumr_rec.date_lastpay));
	fprintf (pp, "|%14.14s|\n",CF (cumr_rec.amt_lastpay));
	PrintRuleLine ();
}


/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (comm,  comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_hhcu_hash");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
													      : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
  	open_rec (pocr, pocr_list,  POCR_NO_FIELDS, "pocr_id_no");
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
	abc_fclose (comm);

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
	int		printerNo)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		file_err (cc, "PFORMAT", "POPEN");

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".13\n");
	fprintf (pp,".L158\n");
	fprintf (pp,".PI12\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n","CUSTOMER CREDIT INFORMATION REPORT.");
	fprintf (pp,".C (NOTE : Report includes forward Cheques.)\n");

	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp,".B1\n");

	fprintf (pp, ".R===============");
	fprintf (pp, "======");
	fprintf (pp, "=================");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===========");
	fprintf (pp, "================\n");

	fprintf (pp, "===============");
	fprintf (pp, "======");
	fprintf (pp, "=================");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===========");
	fprintf (pp, "================\n");

	fprintf (pp, "|    CREDIT    ");
	fprintf (pp, "| CRD ");
	fprintf (pp, "|  CREDIT SALES  ");
	fprintf (pp, "|  TOTAL A/R   ");
	fprintf (pp, "|   CURRENT    ");
	fprintf (pp, "|   1 - %d     ", envDbDaysAgeing);
	fprintf (pp, "|   OVER %d    ", envDbDaysAgeing);
	fprintf (pp, "| OUTSTANDING  ");
	fprintf (pp, "| LAST REC ");
	fprintf (pp, "| LAST RECEIPT |\n");

	fprintf (pp, "|    LIMIT     ");
	fprintf (pp, "| TRM ");
	fprintf (pp, "|     TOTAL      ");
	fprintf (pp, "|              ");
	fprintf (pp, "|    TOTAL     ");
	fprintf (pp, "|    DAYS      ");
	fprintf (pp, "|    DAYS      ");
	fprintf (pp, "| ORDER TOTAL  ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "|    AMOUNT    |\n");

	PrintRuleLine ();
}

void
PrintRuleLine (void)
{
	fprintf (pp, "|--------------");
	fprintf (pp, "|-----");
	fprintf (pp, "|----------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|--------------|\n");
}


void
PrintTotal (void)
{
 	struct  CURR_LST  *lcl_ptr;

	fprintf (pp, "|%14.14s",    CF (totalCreditLimit));
	fprintf (pp, "|     ");
	fprintf (pp, "|%16.16s",	CF (totalCreditSales));
	fprintf (pp, "|%14.14s",    CF (totalAccReceipt));
	fprintf (pp, "|%14.14s",    CF (totalCurrent));
	fprintf (pp, "|%14.14s",    CF (totalOverdue1));
	fprintf (pp, "|%14.14s",   CF (totalOverdue2));
	fprintf (pp, "|%14.14s",    " ");
	fprintf (pp, "|%-10.10s",	   " ");
	fprintf (pp, "|%14.14s|\n", " ");

	if (envDbMcurr)
	{
		fprintf (pp, "|--------------");
		fprintf (pp, "------");
		fprintf (pp, "-----------------");
		fprintf (pp, "---------------");
		fprintf (pp, "---------------");
		fprintf (pp, "---------------");
		fprintf (pp, "---------------");
		fprintf (pp, "---------------");
		fprintf (pp, "-----------");
		fprintf (pp, "---------------|\n");

		fprintf (pp, "|%-11.11sCURRENCY BREAKDOWN%-11.11s ", " ", " ");
		fprintf (pp, "|  CREDIT LIMIT  ");
		fprintf (pp, "| CREDIT SALES  ");
		fprintf (pp, "|  TOTAL A/R    ");
		fprintf (pp, "|CURRENT TOTAL  ");
		fprintf (pp, "| 1 TO 30 DAYS  ");
		fprintf (pp, "| OVER 30 DAYS  |\n");

		fprintf (pp, "|-----------------------------------------");
		fprintf (pp, "|----------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------|\n");

		lcl_ptr = curr_head;
		while (lcl_ptr != CURR_NULL)
		{

			fprintf (pp, "|%-3.3s  %-35.35s ", lcl_ptr->code, lcl_ptr->desc);
			fprintf (pp, "|%16.16s", CF (lcl_ptr->credit_limit));
			fprintf (pp, "|%14.14s ", CF (lcl_ptr->creditSales));
			fprintf (pp, "|%14.14s ", CF (lcl_ptr->totalAR));
			fprintf (pp, "|%14.14s ", CF (lcl_ptr->cur_total));
			fprintf (pp, "|%14.14s ", CF (lcl_ptr->overDue1));
			fprintf (pp, "|%14.14s ", CF (lcl_ptr->overDue2));
			fprintf (pp, "|\n");

			lcl_ptr = lcl_ptr->next;
		}
	}
}


/*--------------------------
| Allocate memory to store |
| currency details.        |
--------------------------*/
struct CURR_LST *
CurrAlloc (void)
{
    int     i;
    struct CURR_LST *lcl_ptr = NULL;

    i = 0;
    while (i < 100)
    {
        lcl_ptr = (struct CURR_LST *)malloc (sizeof (struct CURR_LST));
        if (lcl_ptr != CURR_NULL)
            break;
        i++;
    }

    if (lcl_ptr == CURR_NULL)
        sys_err ("Error in CurrAlloc () During (MALLOC)", 12, PNAME);

    strcpy (lcl_ptr->code, "   ");
	sprintf (lcl_ptr->desc, "%-40.40s", " ");
    for (i = 0; i < 6; i++)
    {
        lcl_ptr->credit_limit 	= 0.00;
        lcl_ptr->creditSales 	= 0.00;
        lcl_ptr->totalAR 		= 0.00;
        lcl_ptr->cur_total 		= 0.00;
        lcl_ptr->overDue1 		= 0.00;
        lcl_ptr->overDue2 		= 0.00;
    }
    lcl_ptr->prev = CURR_NULL;
    lcl_ptr->next = CURR_NULL;

    return (lcl_ptr);
}


/*
 * Store the value against its currency
 */
int
SummCurrency (void)
{
    int     node_fnd;
    int     pos_fnd;
    struct  CURR_LST *lcl_ptr, *tmp_ptr;

    /*
     * Find node in linked list for currency if it exists. Otherwise make one.
     */
  	node_fnd = FALSE;
    lcl_ptr = curr_head;
    while (lcl_ptr != CURR_NULL)
    {
        if (!strcmp (lcl_ptr->code, cumr_rec.curr_code))
        {
			lcl_ptr->credit_limit += cumr_rec.credit_limit / pocr_rec.ex1_factor;
			lcl_ptr->creditSales += creditSales 	/ pocr_rec.ex1_factor;
			lcl_ptr->totalAR     += totalAR 		/ pocr_rec.ex1_factor;
			lcl_ptr->cur_total   += currentTotal 	/ pocr_rec.ex1_factor;
			lcl_ptr->overDue1    += overDue1 		/ pocr_rec.ex1_factor;
			lcl_ptr->overDue2    += overDue2 		/ pocr_rec.ex1_factor;
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
        sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
        cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
        if (cc)
            sprintf (pocr_rec.description, "%-40.40s", "Unknown Currency");

        /*
         * Allocate memory and store data.
         */
        lcl_ptr = CurrAlloc ();
        sprintf (lcl_ptr->code, "%-3.3s", cumr_rec.curr_code);
        sprintf (lcl_ptr->desc, "%-40.40s", pocr_rec.description);
		lcl_ptr->credit_limit 	+= cumr_rec.credit_limit / pocr_rec.ex1_factor;
		lcl_ptr->creditSales 	+= creditSales 			 / pocr_rec.ex1_factor;
		lcl_ptr->totalAR     	+= totalAR 			 	 / pocr_rec.ex1_factor;
		lcl_ptr->cur_total    	+= currentTotal 		 / pocr_rec.ex1_factor;
		lcl_ptr->overDue1    	+= overDue1 			 / pocr_rec.ex1_factor;
		lcl_ptr->overDue2    	+= overDue2 			 / pocr_rec.ex1_factor;

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
            /*
             * No position found so append to end
             */
            if (tmp_ptr == curr_head)
            {
                /*
                 * Insert at head
                 */
   				curr_head = lcl_ptr;
                lcl_ptr->prev = CURR_NULL;
                lcl_ptr->next = CURR_NULL;
                curr_tail = curr_head;
            }
            else
            {
                /*
                 * Append to tail 
                 */
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

int
heading (
 int                scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML ("Customer Credit Information Report."),20,0,1);

	box (0, 2, 80, 5);
	line_at (1,0,80);
	line_at (4,1,79);

	line_at (20,0,80);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

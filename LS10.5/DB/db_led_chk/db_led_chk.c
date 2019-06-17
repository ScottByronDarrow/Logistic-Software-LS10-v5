/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_led_chk.c,v 5.2 2001/11/26 04:44:44 scott Exp $
|  Program Name  : (db_led_chk.c)
|  Program Desc  : (Customer ledger check program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_led_chk.c,v $
| Revision 5.2  2001/11/26 04:44:44  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_led_chk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_led_chk/db_led_chk.c,v 5.2 2001/11/26 04:44:44 scott Exp $";

#include 	<pslscr.h>
#include 	<arralloc.h>
#include 	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include    <errno.h>
#define		CF(x)	comma_fmt (x, "NNN,NNN,NNN,NNN.NN")

	int		envDbTotalAge	= 0,
			envDbDaysAgeing = 0,
			envDbNettUsed 	= TRUE,
			printerNumber 	= 1,
			detailPrinted 	= FALSE;
	
	char	*cumr2	=	"cumr2",
			*cuin2	=	"cuin2";


#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuinRecord	cuin2_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

/*====================================================================
| The structures 'dtls' are initialised in function .                |
| the number of details is stored in external variable 'dtlsCnt'.    |
====================================================================*/
struct	Detail
{                   		/*===============================*/
	long	hhciHash;		/* detail invoice reference.	|*/
	double	invoiceAmt;		/* detail invoice amount.	    |*/
} *dtls;           			/*===============================*/
	DArray	dtls_d;			/* state-info for dynamic reallocation */
	int		dtlsCnt;

/*=====================================================================
| The structures 'cust' are initialised in function .                 |
| the number of customers is stored in external variable 'custCnt'.  |
=====================================================================*/
struct Customer                   
{        
        char    dbtNo [7];      /* Customer number       |*/
        char    acronym [10];   /* Customer Acronym      |*/
        char    name [41];      /* Customer name         |*/
        double  currentBal;
        double  updatedBal;
        double  invChqBal;
        double  ledgerBal;
}       *cust;
        DArray  cust_d;       	/* state-info for dynamic reallocation */                               
        int             custCnt	=	0;

	FILE	*fout;

	char 	branchNumber 	[3],
			emailAddr 		[130],
			*def_email;

	
	double	origBalance 	= 0.00,
			updateBalance  	= 0.00,
			tranBalance  	= 0.00,
			tranForward		= 0.00,
			totalForward 	= 0.00,
			totalCheques	= 0.00,
			totalForwardH	= 0.00,
			totalForwardD	= 0.00,
			totalInvoices 	= 0.00;

	long	mendDate = 0L;
	
	double	calcTotal [6] = {0,0,0,0,0,0}; 


/*
 * Local Function Prototypes.
 */
void 	OpenDB 					 (void);
void 	CloseDB 				 (void);
void 	shutdown_prog 			 (void);
void 	CalculateBalance 		 (int, long);
void 	CalculateTransactions 	 (long);
void 	ProcessLine				 (void);
void 	GetCheques 				 (int, long);
void 	GetForwardD 			 (long);
void 	PrintDetails 			 (void);
void 	HeadingOpen 			 (void);
static 	int MoneyZero 			 (double);
void 	MailUser 				 (void);

int
main (
	int		argc,
	char	*argv [])
{
	int		i; 

	double	cumr_bo [6];

	char	*sptr;

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if ageing is by days overdue or as per standard ageing.
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	/*
	 *	Allocate initial 1000 lines of details
	 */
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);
        ArrAlloc (&cust_d, &cust, sizeof (struct Customer), 100);

	OpenDB ();


	mendDate = MonthEnd (comm_rec.dbt_date);

	if (argc > 1)
		strcpy (branchNumber,argv [1]);
	else
		strcpy (branchNumber,comm_rec.est_no);

	if (argc > 2)
		strcpy (cumr_rec.dbt_acronym,argv [2]);
	else
		strcpy (cumr_rec.dbt_acronym, "         ");

	if (argc > 3)
		sprintf (emailAddr, "/usr/lib/sendmail %-30.30s", argv [3]); 
	else
	{	
		def_email = "/usr/lib/sendmail support";
		strcpy (emailAddr, def_email);
	}

	HeadingOpen ();

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	dsp_screen ("Customers Ledger Check.", comm_rec.co_no, comm_rec.co_name);

	cc = find_rec (cumr , &cumr_rec, GTEQ, "r");
    while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no) && 
	    	       !strcmp (cumr_rec.est_no, branchNumber))
    {
		/*
		 * Exclude child customers for now. 
		 */
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr , &cumr_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Customer",cumr_rec.dbt_acronym);

		for (i = 0; i < 6; i++)
			cumr_bo [i] = cumr_balance [i];

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				cumr_bo [i] += cumr2_balance [i];

			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}

	 	origBalance =  	cumr_bo [0] + cumr_bo [1] +
					    cumr_bo [2] + cumr_bo [3] +
					    cumr_bo [4] + cumr_bo [5];

		GetCheques (TRUE, cumr_rec.hhcu_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			GetCheques (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
	
		totalInvoices = 0.00;
		CalculateBalance (TRUE, cumr_rec.hhcu_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			CalculateBalance (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}

		totalCheques 	= 0.00;
		totalForward 	= 0.00;
		totalForwardH 	= 0.00;
		totalForwardD 	= 0.00;

		CalculateTransactions (cumr_rec.hhcu_hash);
		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			CalculateTransactions (cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		tranBalance = (totalInvoices - totalCheques);
		tranForward = totalForward;
		
		GetForwardD (cumr_rec.hhcu_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			GetForwardD (cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		PrintDetails ();

		if (no_dec (fabs (origBalance - updateBalance)) ||
			no_dec (fabs (origBalance - tranBalance)))
		{
			if (!ArrChkLimit (&cust_d, cust, custCnt))             
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);
	
			sprintf (cust [custCnt].dbtNo, "%-6.6s", cumr_rec.dbt_no);                   
			sprintf (cust [custCnt].acronym, "%-9.9s", cumr_rec.dbt_acronym);
			sprintf (cust [custCnt].name, "%-40.40s", cumr_rec.dbt_name);
			cust [custCnt].currentBal = DOLLARS (origBalance);
			cust [custCnt].updatedBal = DOLLARS (updateBalance);
			cust [custCnt].invChqBal = DOLLARS (tranBalance);
			cust [custCnt].ledgerBal = DOLLARS (updateBalance-tranBalance);
			++custCnt;
		}
		cc = find_rec (cumr , &cumr_rec, NEXT, "r");
    }
	if (!detailPrinted)
	{
		fprintf (fout, "|                                                                         ****** ");
		fprintf (fout, " CUSTOMER LEDGER IS IN BALANCE *****");
		fprintf (fout, "                                                                                |\n");
	}
	else
	{
		MailUser ();
	}
	
	fprintf (fout, ".EOF\n");
	ArrDelete (&dtls_d);
	pclose (fout);
    shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (cuin2, cuin);
	open_rec (cuin , cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cuin2, cuin_list, CUIN_NO_FIELDS, "cuin_hhcu_hash");
	open_rec (cudt , cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd , cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr , cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cuin2);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
    CloseDB (); 
	FinishProgram ();
}

/*
 * Calculate transaction balances.
 */
void
CalculateBalance (
	int		clearTotal,
	long	hhcuHash)
{
	int		i;

	if (clearTotal)
		updateBalance = 0.00;

	cuin_rec.date_of_inv = 0L;
	cuin_rec.hhcu_hash = hhcuHash;
	strcpy (cuin_rec.est, "  ");
	
	cc = find_rec (cuin , &cuin_rec, GTEQ, "r");
	while (!cc && hhcuHash == cuin_rec.hhcu_hash)
	{
		ProcessLine ();
		cc = find_rec (cuin , &cuin_rec, NEXT, "r");
	}
	for (i = 0; i < 6; i++)
	{
		updateBalance += calcTotal [i];
		calcTotal [i] = 0;
	} 
}

/*
 * Calculate transaction balances.
 */
void
CalculateTransactions (
	long	hhcuHash)
{
	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd , &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		if (cuhd_rec.date_payment > mendDate)
		{
			totalForwardH += (cuhd_rec.tot_amt_paid +   
					cuhd_rec.disc_given);
			cc = find_rec (cuhd , &cuhd_rec, NEXT, "r");
			continue;
		}
			
		/*
	 	totalCheques += (cuhd_rec.tot_amt_paid + cuhd_rec.disc_given);
		*/
	 	totalCheques += (cuhd_rec.tot_amt_paid);
		cc = find_rec (cuhd , &cuhd_rec, NEXT, "r");
	}
}

/*
 * Process lines for customer.
 */
void
ProcessLine (void)
{
	int		i;

	double	balance = 0.00;

	balance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

	totalInvoices += no_dec (balance);

	for (i = 0; i < dtlsCnt; i++)
		if (cuin_rec.hhci_hash == dtls [i].hhciHash)
			balance -= no_dec (dtls [i].invoiceAmt);

	if (balance == 0)
		return;

	i = AgePeriod (cuin_rec.pay_terms,
					cuin_rec.date_of_inv,
					comm_rec.dbt_date,
					cuin_rec.due_date,
					envDbDaysAgeing,
					envDbTotalAge);

	if (i == -1)	
		i = 5;

	calcTotal [i] += no_dec (balance);
}

/*
 * Process cheques for customer.
 */
void
GetCheques (
	int		clearTotal,
	long	hhcuHash)
{
	double dt_tot;


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
		dt_tot = 0.00;
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	    cc = find_rec (cudt , &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			if (!ArrChkLimit (&dtls_d, dtls, dtlsCnt))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

			dtls [dtlsCnt].hhciHash 	= cudt_rec.hhci_hash;
			dtls [dtlsCnt].invoiceAmt 	= cudt_rec.amt_paid_inv;
			dt_tot += cudt_rec.amt_paid_inv;
	    	cc = find_rec (cudt , &cudt_rec, NEXT, "r");
			++dtlsCnt;
		}
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*
 * Process forward payments.
 */
void
GetForwardD (
	long	hhcuHash)
{

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		if (cuhd_rec.date_payment <= mendDate)
		{
			cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		} 
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	    cc = find_rec (cudt , &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			totalForwardD += cudt_rec.amt_paid_inv;
			cc = find_rec (cudt , &cudt_rec, NEXT, "r");
		}
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}
/*
 * Print report details for customer.
 */
void
PrintDetails (void)
{
	if (MoneyZero (origBalance - updateBalance) &&
		 MoneyZero (origBalance - tranBalance) && 
		 MoneyZero (totalForwardH - totalForwardD))
		return;

	detailPrinted = TRUE;
	fprintf (fout, "| %-6.6s ", cumr_rec.dbt_no);
	fprintf (fout, "| %-9.9s ", cumr_rec.dbt_acronym);
	fprintf (fout, "|%18.18s ", CF (origBalance));
	fprintf (fout, "|%18.18s ", CF (updateBalance));
	fprintf (fout, "|     %18.18s ", CF (tranBalance));
	fprintf (fout, "|  %18.18s ", CF (totalForwardH));
	fprintf (fout, "|  %18.18s ", CF (totalForwardD));
	fprintf (fout, "|  %18.18s ", CF (totalForwardH - totalForwardD));
	fprintf (fout, "|   %18.18s |\n",CF (updateBalance-tranBalance));
}

/*
 * Print report heading.
 */
void
HeadingOpen (void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".PI16\n");
	fprintf (fout, ".8\n");
	fprintf (fout, ".L188\n");
	fprintf (fout, ".E%s AS AT %s\n",clip (comm_rec.co_name), SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R=========");
	fprintf (fout, "============");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "=========================");
	fprintf (fout, "======================");
	fprintf (fout, "======================");
	fprintf (fout, "======================");
	fprintf (fout, "========================\n");

	fprintf (fout, "=========");
	fprintf (fout, "============");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "=========================");
	fprintf (fout, "======================");
	fprintf (fout, "======================");
	fprintf (fout, "======================");
	fprintf (fout, "========================\n");

	fprintf (fout, "|CUSTOMER");
	fprintf (fout, "|  ACRONYM  ");
	fprintf (fout, "| CURRENT BALANCE   ");
	fprintf (fout, "| UPDATED BALANCE   ");
	fprintf (fout, "| INVOICE/CHEQUE BALANCE ");
	fprintf (fout, "| FORWARD RECEIPT (H) ");
	fprintf (fout, "| FORWARD RECEIPT (D) ");
	fprintf (fout, "| IN BALANCE ON LEDGER");
	fprintf (fout, "| IN BALANCE ON LEDGER |\n");
	
	fprintf (fout, "|        ");
	fprintf (fout, "|           ");
	fprintf (fout, "|Cust Masterfile (a)");
	fprintf (fout, "|Inv Tot-Chq Det (b)");
	fprintf (fout, "| Inv Tot - Chq Tot (c)  ");
	fprintf (fout, "|   Foward Rec. (d)   ");
	fprintf (fout, "|   Foward Rec. (e)   ");
	fprintf (fout, "|       d - e         ");
	fprintf (fout, "|       b - c          |\n");
	
	fprintf (fout, "|--------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-------------------");
	fprintf (fout, "|-------------------");
	fprintf (fout, "|------------------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|----------------------|\n");

}

void MailUser (void)
{
	FILE *sendmail;
	int     i;
                     
	if (! (sendmail = popen (emailAddr, "w")))
		return;
        
	fprintf (sendmail, "Subject: Customers Ledger Imbalance\n\n");
	fprintf (sendmail, "Program: %s\n", PNAME);
	fprintf (sendmail, "Version: %s\n", PROG_VERSION);
	fprintf (sendmail, "\n");
	fprintf (sendmail, "The LS10 Customers Ledger check program has found ");
	fprintf (sendmail, "an imbalance with the following customer accounts:\n");

	for (i = 0; i < custCnt; i++)
	{
		fprintf (sendmail, "\n");
		fprintf (sendmail, "Customer - %s %s %s\n", cust [i].dbtNo,
						 			cust [i].acronym, cust [i].name);
		fprintf (sendmail, "\tCurrent Balance %-13.2f\n", cust [i].currentBal);
		fprintf (sendmail, "\tUpdated Balance %-13.2f\n", cust [i].updatedBal);
		fprintf (sendmail, "\tInv/Chq Balance %-13.2f\n", cust [i].invChqBal);
		fprintf (sendmail, "\tInbalance on Ledger %-13.2f\n",
                                 	cust [i].ledgerBal);
	}
	fclose (sendmail);
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

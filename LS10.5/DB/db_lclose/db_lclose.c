/*====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lclose.c,v 5.3 2001/11/20 08:10:04 scott Exp $
|  Program Name  : (db_lclose.c) 
|  Program Desc  : (Account Receivable month end ledger close.)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 12/09/88         |
|---------------------------------------------------------------------|
| $Log: db_lclose.c,v $
| Revision 5.3  2001/11/20 08:10:04  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lclose.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lclose/db_lclose.c,v 5.3 2001/11/20 08:10:04 scott Exp $";

#include 	<pslscr.h>
#include 	<arralloc.h>
#include	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include    <errno.h>

#define		CO_DBT		(envCoClose [0] == '1')



#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

static	char		*data	= "data";

/*
 * The structures 'details' are initialised in function .              
 * the number of details is stored in external variable 'dtlsCnt'.   
 */
struct	Detail {        /*===================================*/
	long	hhci_hash;	/* detail invoice reference.	    |*/
	double	inv_amt;	/* detail invoice amount.	        |*/
}	*details;			/*===================================*/

	DArray	dtlsD;		/* state info for dyanamic array "details" */
	int		dtlsCnt;

	char	envCoClose 		[6],
			branchNumber 	[3];
	
	int		envDbCo 		= 0,
			envDbGraceLimit = 0,
			envDbTotalAge	= 0,
			envDbDaysAgeing	= 0,
			allBranches 	= FALSE, 
			envDbNettUsed 	= TRUE;

	Date	calculateDate = 0L;
	long	lsystemDate = 0L;

	double	total 	[6]	= {0, 0, 0, 0, 0, 0},
			odTotal [6]	= {0, 0, 0, 0, 0, 0}; 


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	ProcessInvoice 	 	(long);
void 	GetNextDate 		(Date);
int  	ValidateCustomer	(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	ProcessLines 		(int);
void 	GetCheques 			(int, long);
void 	Update 				(double);

int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	int		i; 

	double	oldCumr = 0.00,
			newCumr = 0.00;

	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	/*
	 * Check if ageing is by days overdue or as per standard ageing.
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_GRACE_LIMIT");
	envDbGraceLimit = (sptr == (char *)0) ? 0 : atoi (sptr);

	envDbCo = atoi (get_env ("DB_CO"));

	/*
	 *	Allocate initial 1000 lines of detail
	 */
	ArrAlloc (&dtlsD, &details, sizeof (struct Detail), 1000);

	OpenDB ();


	lsystemDate = TodaysDate ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	if (CO_DBT)
	{
		dsp_screen ("Closing Company Customers.", 
					comm_rec.co_no, comm_rec.co_name);
		allBranches = TRUE;
	}
	else
	{
		sprintf (err_str, "Closing Customers for Branch %s - %s",
				comm_rec.est_no,clip (comm_rec.est_name));

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		allBranches = FALSE;
	}

	GetNextDate (comm_rec.dbt_date);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, (allBranches) ? "  " : branchNumber);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "u");
    while (!cc && ValidateCustomer ())
    {
		GetCheques (TRUE, cumr_rec.hhcu_hash);

		if (cumr_rec.ho_dbt_hash > 0L)
			GetCheques (FALSE, cumr_rec.ho_dbt_hash);

		ProcessInvoice (cumr_rec.hhcu_hash);

		cumr_rec.od_flag = 0;

		for (i = 0; i < 6; i++)
		{
			oldCumr += cumr_balance [i];
			cumr_balance [i] = total [i];
			newCumr += total [i];
			total [i] = 0;
		} 

		for (i = 1; i < 5; i++)
		{
			if (no_dec (odTotal [i]) > 0)
				cumr_rec.od_flag = i;

			odTotal [i] = 0;
		}
	
		dsp_process ("Customer : ", cumr_rec.dbt_acronym);
	
		Update (newCumr);

		newCumr = 0.00;
		oldCumr = 0.00;
		cc = find_rec (cumr, &cumr_rec, NEXT, "u");
    }
	abc_unlock (cumr);

	CloseDB (); 
	FinishProgram ();
	
	/*
	 * Free array memory to OS  
	 */
	ArrDelete (&dtlsD);

	return (EXIT_SUCCESS);
}

void
ProcessInvoice (
	long	hhcuHash)
{
	cuin_rec.date_of_inv = 0L;
	cuin_rec.hhcu_hash = hhcuHash;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && hhcuHash == cuin_rec.hhcu_hash)
	{
		ProcessLines (envDbTotalAge);
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}

/*
 * Calculate  the last day of next month
 */
void
GetNextDate (
	Date	currentDate)
{
	calculateDate = AddMonths (currentDate, 1);
}

int
ValidateCustomer (void)
{
    if (!allBranches && !strcmp (cumr_rec.co_no,comm_rec.co_no) && 
	                !strcmp (cumr_rec.est_no,branchNumber))
		return (EXIT_FAILURE);

    if (allBranches && !strcmp (cumr_rec.co_no,comm_rec.co_no))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_dbclose (data);
}

int
ProcessLines (
	int   _envDbTotalAge)
{
	int		i;

	double	balance = 0.00;

	balance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

	for (i = 0; i < dtlsCnt; i++)
		if (cuin_rec.hhci_hash == details [i].hhci_hash)
			balance -= details [i].inv_amt;

	if (balance == 0)
		return (EXIT_FAILURE);

	i = AgePeriod 
		(
			cuin_rec.pay_terms,
			cuin_rec.date_of_inv,
			(_envDbTotalAge) ? comm_rec.dbt_date + 1L : calculateDate,
			cuin_rec.due_date,
			envDbDaysAgeing,
			_envDbTotalAge
		);

	if (i == -1)
		total [5] += balance;
	else
		total [i] += balance;

	i = AgePeriod 
		(
			cuin_rec.pay_terms,
			cuin_rec.date_of_inv,
			lsystemDate - (long) cumr_rec.crd_ext - (long) envDbGraceLimit,
			cuin_rec.due_date,
			envDbDaysAgeing,
			TRUE
		);

	if (i == -1)
		odTotal [5] += balance;
	else
		odTotal [i] += balance;

    return (EXIT_SUCCESS);
}

void
GetCheques (
	int		clearTotal,
	long   	hhcuHash)
{
	if (clearTotal)
		dtlsCnt = 0;

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			/*
			 * Increase details array as required  
			 */
			if (!ArrChkLimit (&dtlsD, details, dtlsCnt))
				sys_err ("ArrChkLimit (details)", ENOMEM, PNAME);

			details [dtlsCnt].hhci_hash = cudt_rec.hhci_hash;
			details [dtlsCnt].inv_amt 	= cudt_rec.amt_paid_inv;
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
			++dtlsCnt;
	    }
	    cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*
 * Update Customers Periods.
 */
void
Update (
	double	balance)
{
	if (cumr_balance [4] != 0.00)
	{
		if (cumr_rec.payment_flag <= 4)
			cumr_rec.payment_flag = 4;

		else if (cumr_rec.payment_flag > 4)
			cumr_rec.payment_flag++;
	}
	if (cumr_balance [3] != 0.00)
	{
		if (cumr_rec.payment_flag <= 3)
			cumr_rec.payment_flag = 3;
	}
	if (cumr_balance [2] != 0.00)
	{
		if (cumr_rec.payment_flag <= 2)
			cumr_rec.payment_flag = 2;
	}
	if (cumr_balance [1] != 0.00)
	{
		if (cumr_rec.payment_flag <= 1)
			cumr_rec.payment_flag = 1;
	}
	/*
	 * If Balance = 0 then set status = 'O'
	 * otherwise           set status = 'B'
	 */
	strcpy (cumr_rec.stat_flag, (balance == 0.00) ? "0" : "B");

	cc = abc_update (cumr, &cumr_rec);
	if (cc)
		file_err (cc, cumr, "DBUPDATE");
}

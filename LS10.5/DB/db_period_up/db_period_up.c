/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_period_up.c,v 5.1 2001/12/07 03:59:09 scott Exp $
|  Program Name  : (db_period_up.c)
|  Program Desc  : (Update Customer Current , Period1 - 3 Periods) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: db_period_up.c,v $
| Revision 5.1  2001/12/07 03:59:09  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_period_up.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_period_up/db_period_up.c,v 5.1 2001/12/07 03:59:09 scott Exp $";

#include 	<pslscr.h>
#include 	<arralloc.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include    <errno.h>

#include    "schema"

	int		true_age;
	int		DaysAgeing;

	struct	commRecord	comm_rec;
	struct	cumrRecord	cumr_rec;
	struct	cuinRecord	cuin_rec;
	struct	cuhdRecord	cuhd_rec;
	struct	cudtRecord	cudt_rec;
	
/*=======================================================================
| The structures 'details' are initialised in function .                |
| the number of details is stored in external variable 'detailsCounter'.|
=======================================================================*/
struct	Detail {        /*===================================*/
	long	hhciHash;	/* detail invoice reference.	    |*/
	double	invAmount;	/* detail invoice amount.	        |*/
}	*details;			/*===================================*/
	DArray	dtls_d;		/* state info for dynamic reallocation */
	int		detailsCounter;

	int		graceLimit = 0;

	char 	branchNumber [3];
	
	double	total [6] 	= {0,0,0,0,0,0}; 
	double	odTotal [6] = {0,0,0,0,0,0}; 
	Money	*cumr_period	=	&cumr_rec.bo_current;

	long	mendDate = 0L;
	long	lsystemDate = 0L;

	int		DB_NETT = TRUE;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void ProcessInvoice (long);
void OpenDB 		(void);
void CloseDB 		(void);
void shutdown_prog 	(void);
void ProcessLine 	(int);
void GetCheque 		(int, long);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	int		i, 
			numberProcess = 0; 

	int		envDbCo;
	
	char	*sptr;

	double	oldBalance = 0.00,
			newBalance = 0.00;

	sptr = chk_env ("DB_GRACE_LIMIT");
	graceLimit = (sptr == (char *)0) ? 0 : atoi (sptr);
	
	sptr = chk_env ("DB_NETT_USED");
	DB_NETT = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		true_age = FALSE;
	else
		true_age = (*sptr == 'T' || *sptr == 't');

	/*---------------------------------------------------------------
	| Check if ageing is by days overdue or as per standard ageing. |
	---------------------------------------------------------------*/
	sptr = chk_env ("DB_DAYS_AGEING");
	DaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	envDbCo = atoi (get_env ("DB_CO"));

	/*
	 *	Allocate initial detail for 1000 items
	 */
	ArrAlloc (&dtls_d, &details, sizeof (struct Detail), 1000);

	OpenDB ();
	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	mendDate = MonthEnd (comm_rec.dbt_date);

	lsystemDate = TodaysDate ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	strcpy (cumr_rec.dbt_acronym, (argc > 1) ? argv [1] : "         ");

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	dsp_screen ("Updating Customer Periods.", 
					comm_rec.co_no, comm_rec.co_name);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "u");
    while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no) && 
		         !strcmp (cumr_rec.est_no, branchNumber))
    {
		GetCheque (TRUE, cumr_rec.hhcu_hash);

		if (cumr_rec.ho_dbt_hash > 0L)
			GetCheque (FALSE, cumr_rec.ho_dbt_hash);

		ProcessInvoice (cumr_rec.hhcu_hash);
		
		cumr_rec.od_flag = 0;
		for (i = 0; i < 6; i++)
		{
			oldBalance		+=	cumr_period [i];
			cumr_period[i]	=	total [i];
			newBalance		+=	total [i];
			total [i]		=	0;
		} 
		for (i = 1; i < 5; i++)
		{
			if (no_dec (odTotal [i]) > 1.0)
				cumr_rec.od_flag = i;

			odTotal [i] = 0.00;
		}

		sprintf (err_str, "%5.2f/%5.2f", DOLLARS (newBalance),
					        			 DOLLARS (oldBalance));
		if (no_dec (newBalance) != no_dec (oldBalance))
		{
			dsp_process ("ERROR: ", err_str);
			fflush (stdout);
			sleep (sleepTime);
		}
	
		dsp_process ("Customer : ", cumr_rec.dbt_acronym);
	
		if (newBalance <= 0.00)
			cumr_rec.od_flag = 0;

		cc = abc_update (cumr, &cumr_rec);
		if (cc)
			file_err (cc, cumr, "DBUPDATE");
		
		newBalance = 0.00;
		oldBalance = 0.00;
		if (argc > 2 && numberProcess++ > 10)
			break;

		cc = find_rec (cumr, &cumr_rec, NEXT, "u");
   	}
	ArrDelete (&dtls_d);
   	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
ProcessInvoice (
 long               hhcuHash)
{
	long	due_date;

	cuin_rec.date_of_inv = 0L;
	cuin_rec.hhcu_hash = hhcuHash;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
	while (!cc && hhcuHash == cuin_rec.hhcu_hash)
	{
		due_date = cuin_rec.due_date;

		cuin_rec.due_date = CalcDueDate 
							(
								cuin_rec.pay_terms,
								cuin_rec.date_of_inv
							);
	
		if (cuin_rec.due_date == due_date)
			abc_unlock (cuin);
		else
		{
			cc = abc_update (cuin, &cuin_rec);
			if (cc)
				file_err (cc, cuin, "DBUPDATE");
		}
		ProcessLine (true_age);
		cc = find_rec (cuin, &cuin_rec, NEXT, "u");
	}
	abc_unlock (cuin);
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	open_rec (comm,  comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (comm);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
   	CloseDB (); 
	FinishProgram ();
}

void
ProcessLine (
 int	_true_age)
{
	int		i;

	double	balance = 0.00;

	balance = (DB_NETT) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;
	
	for (i = 0; i < detailsCounter; i++)
		if (cuin_rec.hhci_hash == details [i].hhciHash)
			balance -= details [i].invAmount;

	if (balance == 0)
		return;

	i = AgePeriod 
		(
			cuin_rec.pay_terms, 
			cuin_rec.date_of_inv, 
			comm_rec.dbt_date,
			cuin_rec.due_date,
			DaysAgeing,
			_true_age
		);
	if (i == -1)
		total [5] += no_dec (balance);
	else
		total [i] += no_dec (balance);
	
	i = AgePeriod 
		(
			cuin_rec.pay_terms,
			cuin_rec.date_of_inv,
			lsystemDate - (long) cumr_rec.crd_ext - (long) graceLimit,
			cuin_rec.due_date,
			DaysAgeing,
			TRUE
		);

	if (i == -1)
		odTotal [5] += no_dec (balance);
	else
		odTotal [i] += no_dec (balance);

}

void
GetCheque (
 int                clear_tot,
 long               hhcuHash)
{

	if (clear_tot)
		detailsCounter = 0;

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "u");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		Money	locPayments	=	0.00;
		Money	fgnPayments	=	0.00;
		Money	varExchange	=	0.00;

		if (cuhd_rec.date_payment > mendDate)
		{
			cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		}
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			if (!ArrChkLimit (&dtls_d, details, detailsCounter))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

			details [detailsCounter].hhciHash 	= cudt_rec.hhci_hash;
			details [detailsCounter].invAmount 	= cudt_rec.amt_paid_inv;
			locPayments	+=	cudt_rec.loc_paid_inv;
			varExchange	+=	cudt_rec.exch_variatio;
			fgnPayments	+=	cudt_rec.amt_paid_inv;
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
			++detailsCounter;
	    }
		if (twodec (cuhd_rec.tot_amt_paid) != twodec (fgnPayments) ||
		    twodec (cuhd_rec.loc_amt_paid) != twodec (locPayments) ||
		    twodec (cuhd_rec.exch_variance) != twodec (varExchange))
		{
			cuhd_rec.tot_amt_paid	=	twodec (fgnPayments);
			cuhd_rec.loc_amt_paid	=	twodec (locPayments);
			cuhd_rec.exch_variance	=	twodec (varExchange);
			cc = abc_update (cuhd, &cuhd_rec);
			if (cc)
				file_err (cc, cuhd, "DBUPDATE");
		}
		else
			abc_unlock (cuhd);

	    cc = find_rec (cuhd, &cuhd_rec, NEXT, "u");
	}
	abc_unlock (cuhd);
}

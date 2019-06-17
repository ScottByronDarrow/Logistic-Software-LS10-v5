/*=====================================================================
|  Copyright (C) 1999 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (db_percalc.c  )                                   |
|  Program Desc  : (Update Customer Current , Period1 - 3 Periods)    |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, cuhd, cudt, cuin,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cumr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (01/06/87)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
|  Date Modified : (01/06/87)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (29/10/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (21/02/92)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (29/10/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (13/04/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/08/93)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (06/04/96)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (09/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      :                                                    |
|  (29/10/90)    : General Update for New Scrgen. S.B.D.              |
|  (22/02/92)    : Updated for MOD S/C KIL-6531                       |
|  (29/10/92)    : Increase DTLS from 4000 to 6000. OCT 8030          |
|  (13/04/93)    : OCT 8782. This time, 8000.                         |
|  (30/08/93)    : HGP 9487. New head office structure.               |
|  (06/04/96)   : PDL - Updated for new ageing routine AgePeriod ().  |
|  (09/09/97)   : Incorporate multilingual conversion.                |
|                                                                     |
| $Log: db_percalc.c,v $
| Revision 5.2  2001/08/09 09:03:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:22:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:16  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:25:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:52:55  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.19  2000/07/10 01:52:26  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.18  2000/05/15 23:24:01  scott
| S/C ASL-16313 / LSDI-2897
| Updated to fix problem with deposits showing outside current period.
|
| Revision 1.17  2000/01/31 23:43:41  scott
| S/C LSANZ-15848 Updated as reported customer credit limit due to fraction of a cent. Adjusted program to use decimal round routines.
| S/C LSDI-2391
|
| Revision 1.16  2000/01/06 08:49:53  scott
| Updated to ensure it works for correctly from hashes passed by input.
|
| Revision 1.15  1999/11/26 01:18:43  jonc
| AgePeriod () modified to supply true_age flag as an argument.
|
| Revision 1.14  1999/10/29 00:04:41  scott
| Updated for cumr_crd_ext (credit extention) from GMS.
|
| Revision 1.13  1999/10/12 05:18:00  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.12  1999/10/05 07:34:33  scott
| Updated from ansi project.
|
| Revision 1.11  1999/06/14 23:26:00  scott
| Updated to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_percalc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_percalc/db_percalc.c,v 5.2 2001/08/09 09:03:45 scott Exp $";


#define	NO_SCRGEN
#include <pslscr.h>
#include <arralloc.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <errno.h>

#include    "schema"

	int		true_age;
	int		DaysAgeing;

	struct	commRecord	comm_rec;
	struct	cumrRecord	cumr_rec;
	struct	cuinRecord	cuin_rec;
	struct	cuhdRecord	cuhd_rec;
	struct	cudtRecord	cudt_rec;

	Money	*cumr_period	=	&cumr_rec.bo_current;

	struct {
		long	wk_hash;
	} wk_rec;

/*======================================================================
| The structures 'details' are initialised in function .               |
| the number of details is stored in external variable 'detailsCounter'|
======================================================================*/
struct	Detail {		/*===================================*/
	long	hhci_hash;	/* detail invoice reference.	    |*/
	double	inv_amt;	/* detail invoice amount.	        |*/
} *details;				/*===================================*/
	DArray	dtls_d;			/* state-info for dynamic dtls */
	int		detailsCounter;

	int		wk_no,
			pid,
			graceLimit = 0;

	char 	branchNo [3];
	
	double	total [6] = {0,0,0,0,0,0}; 
	double	odTotal [6] = {0,0,0,0,0,0}; 

	long	mendDate 	= 0L;
	long	lsystemDate = 0L;

	int		DB_NETT = TRUE;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int		OpenDB 			 (void);
void	CloseDB 		 (void);
void	shutdown_prog 	 (void);
void	ProcessInvoice 	 (long);
void	ProcessLine 	 (int);
void	GetCheques 	 	 (int, long);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int   argc,
 char* argv [])
{

	char	*sptr;
	int		i; 
	long	lastHash		=	0L;
	double	CustomerTotal	=	0.00;

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

	if (argc < 2 || (pid = atoi (argv [1])) < 1)
	{
		print_at (0,0,mlStdMess046,argv [0]);
        return (EXIT_FAILURE);
    }

	/*
	 *	Allocate 1000 detail lines initially
	 */
	ArrAlloc (&dtls_d, &details, sizeof (struct Detail), 1000);

	if ((cc = OpenDB ()))
    {
        return (cc);
    }

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	mendDate = MonthEnd (comm_rec.dbt_date);

	lsystemDate = TodaysDate ();

	strcpy (err_str, ML ("Calculating Customers Periods."));
	dsp_screen (err_str, comm_rec.co_no,comm_rec.co_name);

	cc = RF_READ (wk_no, (char *) &wk_rec);
    while (!cc) 
    {
		if (lastHash == wk_rec.wk_hash)
		{
			cc = RF_READ (wk_no, (char *) &wk_rec);
			continue;
		}
		cumr_rec.hhcu_hash	=	wk_rec.wk_hash;
		cc = find_rec ("cumr", &cumr_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock ("cumr");
			cc = RF_READ (wk_no, (char *) &wk_rec);
			continue;
		}
		GetCheques (TRUE, cumr_rec.hhcu_hash);

		if (cumr_rec.ho_dbt_hash > 0L)
			GetCheques (FALSE, cumr_rec.ho_dbt_hash);
	
		ProcessInvoice (cumr_rec.hhcu_hash);

		cumr_rec.od_flag = 0;

		CustomerTotal	=	0.00;
		for (i = 0; i < 6; i++)
		{
			cumr_period [i] 	= 	total [i];
			CustomerTotal		+=	total [i];
			total [i] = 0.00;
		} 
		for (i = 1; i < 5; i++)
		{
			if (no_dec (odTotal [i]) > 1.0)
				cumr_rec.od_flag = i;

			odTotal [i] = 0.00;
		}
		if (CustomerTotal <= 0.00)
			cumr_rec.od_flag = 0;

		dsp_process ("Customer : ", cumr_rec.dbt_acronym);

		cc = abc_update ("cumr", &cumr_rec);
		if (cc)
			file_err (cc, "cumr", "DBUPDATE");

		lastHash	=	cumr_rec.hhcu_hash;

		cc = RF_READ (wk_no, (char *) &wk_rec);
    }
	ArrDelete (&dtls_d);
    shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=================================
| Open All Database files needed. |
=================================*/
int
OpenDB (void)
{
	char	fileName [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (fileName,"%s/WORK/db_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	abc_dbopen ("data");

	open_rec ("cuin",  cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec ("cudt",  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec ("cuhd",  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec ("cumr",  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	cc = RF_OPEN (fileName,sizeof (wk_rec),"r",&wk_no);

	if (cc)
		shutdown_prog ();

    return (cc);
}

void
CloseDB (void)
{
	abc_fclose ("cumr");
	abc_fclose ("cuin");
	abc_fclose ("cudt");
	abc_fclose ("cuhd");
	abc_fclose ("comm");
	abc_dbclose ("data");
	RF_DELETE (wk_no);
}

void
shutdown_prog (void)
{
    CloseDB (); 
	FinishProgram ();
}

/*====================================
| Process invoices for current hash. |
====================================*/
void
ProcessInvoice (
	long	hhcuHash)
{
	long	dueDate;

	cuin_rec.date_of_inv = 0L;
	cuin_rec.hhcu_hash = hhcuHash;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec ("cuin", &cuin_rec, GTEQ, "u");
	while (!cc && hhcuHash == cuin_rec.hhcu_hash)
	{
		dueDate = cuin_rec.due_date;

		cuin_rec.due_date = CalcDueDate 
							 (
								cuin_rec.pay_terms,
								cuin_rec.date_of_inv
							);
	
		if (cuin_rec.due_date == dueDate)
			abc_unlock ("cuin");
		else
		{
			cc = abc_update ("cuin", &cuin_rec);
			if (cc)
				file_err (cc, "cuin", "DBUPDATE");
		}
		ProcessLine (true_age);
		cc = find_rec ("cuin", &cuin_rec, NEXT, "u");
	}
	abc_unlock ("cuin");
}

/*=========================
| Process invoices lines. |
=========================*/
void
ProcessLine (
	int		_true_age)
{
	int		i;

	double	balance = 0.00;

	balance = (DB_NETT) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;
	
	for (i = 0; i < detailsCounter; i++)
		if (cuin_rec.hhci_hash == details [i].hhci_hash)
			balance -= details [i].inv_amt;

	if (balance == 0)
		return;

	i =	AgePeriod 
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
	
	i =	AgePeriod 
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

/*========================
| Process cheques lines. |
========================*/
void
GetCheques (
	int		clear_tot,
	long	hhcuHash)
{
	if (clear_tot)
		detailsCounter = 0;

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec ("cuhd", &cuhd_rec, GTEQ, "u");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		Money	locPayments	=	0.00;
		Money	fgnPayments	=	0.00;
		Money	varExchange	=	0.00;

		if (cuhd_rec.date_payment > mendDate)
		{
			cc = find_rec ("cuhd", &cuhd_rec, NEXT, "r");
			continue;
		}
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec ("cudt", &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			if (!ArrChkLimit (&dtls_d, details, detailsCounter))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

			details [detailsCounter].hhci_hash 	= cudt_rec.hhci_hash;
			details [detailsCounter].inv_amt 	= cudt_rec.amt_paid_inv;
			locPayments	+=	cudt_rec.loc_paid_inv;
			varExchange	+=	cudt_rec.exch_variatio;
			fgnPayments	+=	cudt_rec.amt_paid_inv;
			cc = find_rec ("cudt", &cudt_rec, NEXT, "r");
			++detailsCounter;
	    }
		if (twodec (cuhd_rec.tot_amt_paid) != twodec (fgnPayments) ||
		    twodec (cuhd_rec.loc_amt_paid) != twodec (locPayments) ||
		    twodec (cuhd_rec.exch_variance) != twodec (varExchange))
		{
			cuhd_rec.tot_amt_paid	=	twodec (fgnPayments);
			cuhd_rec.loc_amt_paid	=	twodec (locPayments);
			cuhd_rec.exch_variance	=	twodec (varExchange);
			cc = abc_update ("cuhd", &cuhd_rec);
			if (cc)
				file_err (cc, "cuhd", "DBUPDATE");
		}
		else
			abc_unlock ("cuhd");

	    cc = find_rec ("cuhd", &cuhd_rec, NEXT, "u");
	}
	abc_unlock (cuhd);
}

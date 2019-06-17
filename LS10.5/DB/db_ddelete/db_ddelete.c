/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (db_ddelete.c )                                    |
|  Program Desc  : (Customer Cheque/Invoice Delete.            )      |
|                  (                                           )      |
|---------------------------------------------------------------------|
|  Access files  :  comm, cuin, cuhd, cudt,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cuin, cuhd, cudt,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (23/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/06/91)      | Modified  by  :                  |
|  Date Modified : (08/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (10/04/94)      | Modified  by  : Roel Michels     |
|                                                                     |
|  Comments      : (23/10/90) - General Update for New Scrgen. S.B.D. |
|                : (05/06/91) - Updated for mb closes.                |
|                : (08/04/94) - PSL 10673, Online conversion          |
|                : (10/04/94) - PSL 10673, Online conversion          |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: db_ddelete.c,v $
| Revision 5.3  2001/09/11 23:59:17  scott
| Updated from Scott machine - 12th Sep 2001
|
| Revision 5.2  2001/08/09 08:23:47  scott
| Added FinishProgram ();
|
| Revision 5.1  2001/08/06 23:21:58  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:04:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:24:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:13:43  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:52:26  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.11  2000/07/10 01:52:23  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.10  2000/06/09 05:03:08  scott
| S/C LSANZ 16397
| Updated to add group purge option that existed in the LS/10 version before the
| merger Pinnacle version.
| Basically B/F and non-group purging is the same.
| Transactions that are > 1st of month - number of days are not purged.
|
| Revision 1.9  1999/10/29 03:15:25  scott
| Updated for warning due to usage of -Wall flag on compiler.
|
| Revision 1.8  1999/10/05 07:34:07  scott
| Updated from ansi project.
|
| Revision 1.7  1999/06/14 23:25:50  scott
| Updated to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_ddelete.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_ddelete/db_ddelete.c,v 5.3 2001/09/11 23:59:17 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>

#define		DELETE_INVOICE 	 (cuin_rec.stat_flag [0] == '9')
#define		DELETE_CHEQUE 	 (cuhd_rec.stat_flag [0] == '9')
#define		DELETE_DETAIL 	 (cudt_rec.stat_flag [0] == '9')
#define		CO_DBT		 (envCoClose [0] == '1')

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;

	char	envCoClose [6],
			branchNumber [3];

	int		allBranches = FALSE,
			envDbCo 	= 0;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	ProcessCumr 	(void);
void 	DeleteInvoice 	(long);
void 	DeleteCheque 	(long);
void 	DeleteDetail 	(long);
int 	CheckCudt 		(long);
int 	ValidCustomer 	(void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	envDbCo = atoi (get_env ("DB_CO"));

	OpenDB ();
	
	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	if (CO_DBT)
	{
		dsp_screen ("Purging Company Customers.", 
					comm_rec.co_no, comm_rec.co_name);
		allBranches = TRUE;
	}
	else
	{
		sprintf (err_str, "Purging Customers for Branch %s - %s",
				comm_rec.est_no,clip (comm_rec.est_name));

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		allBranches = FALSE;
	}

	ProcessCumr ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhcu_hash");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
}

void
CloseDB (void)
{
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cumr);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=====================================================
| Process customers master file from start to finish. |
=====================================================*/
void
ProcessCumr (void)
{
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, (allBranches) ? "  " : branchNumber);
	strcpy (cumr_rec.dbt_acronym, "         ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	while (!cc && ValidCustomer ())
	{
		dsp_process ("Customer : ", cumr_rec.dbt_no);

		DeleteInvoice (cumr_rec.hhcu_hash);
		DeleteCheque (cumr_rec.hhcu_hash);

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

int
ValidCustomer (void)
{
    if (!allBranches && !strcmp (cumr_rec.co_no,comm_rec.co_no) && 
        !strcmp (cumr_rec.est_no,branchNumber))
        return (EXIT_FAILURE);

    if (allBranches &&
        !strcmp (cumr_rec.co_no,comm_rec.co_no))
        return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
/*==============================================
| Delete all cuin records with '9' stat_flags. |
==============================================*/
void
DeleteInvoice (
	long	hhcuHash)
{
	/*---------------------------------------------------------
 	| Delete all cuin records for debtor with '9' stat_flags. |
	---------------------------------------------------------*/
	cuin_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.hhcu_hash == hhcuHash)
	{
		dsp_process ("Delete Invoices", "Paid");

		if (DELETE_INVOICE)
		{
			cc = abc_delete (cuin);
			if (cc)
				file_err (cc, cuin, "DBDELETE");

			cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
		}
		else
			cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}
/*==========================================
| Delete cheque header and cheque details. |
==========================================*/
void
DeleteCheque (
	long	hhcuHash)
{
	/*------------------------------------
	| Process and delete cheque details. |
	------------------------------------*/
	cuhd_rec.hhcu_hash	=	hhcuHash;
   	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		if (!DELETE_CHEQUE)
		{
   			cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		}
		DeleteDetail (cuhd_rec.hhcp_hash);

   		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}

	cuhd_rec.hhcu_hash	=	hhcuHash;
   	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "u");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		if (!DELETE_CHEQUE)
		{
			abc_unlock (cuhd);

   			cc = find_rec (cuhd, &cuhd_rec, NEXT, "u");
			continue;
		}
		if (CheckCudt (cuhd_rec.hhcp_hash))
		{
			abc_unlock (cuhd);
			cc = abc_delete (cuhd);
			if (cc)
				file_err (cc, cuhd, "DBDELETE");
				
   			cc = find_rec (cuhd, &cuhd_rec, GTEQ, "u");
		}
		else
   			cc = find_rec (cuhd, &cuhd_rec, NEXT, "u");
	}
}
/*====================================================
| Routine to check for the existance of cudt records |
| and return (1) if none found, else update cuhd.     |
====================================================*/
int
CheckCudt (
	long	hhcpHash)
{
	double	fgnTotal = 0.00;
	double	locTotal = 0.00;

	/*-------------------------------------
	| check if any cheque deteils remain. |
	-------------------------------------*/
	cudt_rec.hhcp_hash	=	hhcpHash;
	cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	if (cc || cudt_rec.hhcp_hash != hhcpHash)
		return (EXIT_FAILURE);

	/*---------------------------
	| Add up remaining details. |
	---------------------------*/
	while (!cc && cudt_rec.hhcp_hash == hhcpHash)
	{
		fgnTotal += cudt_rec.amt_paid_inv;
		locTotal += cudt_rec.loc_paid_inv;
		cc = find_rec (cudt, &cudt_rec, NEXT, "r");
	}

	/*--------------------------------
	| Update cheque With new Totals. |
	--------------------------------*/
	cuhd_rec.tot_amt_paid	= fgnTotal;
	cuhd_rec.loc_amt_paid 	= locTotal;
	cuhd_rec.disc_given 	= 0.00;
	cuhd_rec.loc_disc_give 	= 0.00;
	strcpy (cuhd_rec.stat_flag, "0");
	cc = abc_update (cuhd, &cuhd_rec);
	if (cc)
		file_err (cc, cuhd, "DBUPDATE");

	return (EXIT_SUCCESS);
}

/*===========================================
| Routine to delete all valid cudt records. |
===========================================*/
void
DeleteDetail (
	long	hhcpHash)
{
	cudt_rec.hhcp_hash	=	hhcpHash;
	cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	while (!cc && cudt_rec.hhcp_hash == hhcpHash)
	{
		if (DELETE_DETAIL)
		{
			dsp_process ("Delete Cheques", "Zero Balance");
			cc = abc_delete (cudt);
			if (cc)
				file_err (cc, cudt, "DBDELETE");
			
			cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
		}
		else
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
	}
}

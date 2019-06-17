/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: period_up.c,v 5.3 2002/06/25 03:44:45 scott Exp $
|  Program Name  : (cr_period_up.c)
|  Program Desc  : (Update Supplier Periods In sumr)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: period_up.c,v $
| Revision 5.3  2002/06/25 03:44:45  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: period_up.c,v $", 
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_period_up/period_up.c,v 5.3 2002/06/25 03:44:45 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<arralloc.h>

#include "schema"

	struct	commRecord	comm_rec;
	struct	suinRecord	suin_rec;
	struct	suhdRecord	suhd_rec;
	struct	sudtRecord	sudt_rec;
	struct	sumrRecord	sumr_rec;

/*
 * The structures 'cheq'&'dtls' are initialised in function 'get_cheq' 
 * the number of cheques is stored in external variable 'chequeCount'.
 * the number of details is stored in external variable 'detailsCount'.
 */
struct Cheque
{						/*-------------------------------*/
	char	no[16];		/*| cheque receipt number.	    |*/
	double	amount;		/*| total amount of cheque.	    |*/
}	*cheq;				/*|                             |*/
	DArray	cheq_d;		/*| dynamic info for cheq 		|*/
	int		chequeCount;/*-------------------------------*/

struct Detail
{						/*-----------------------------------*/
	long	hhsiHash;	/*| detail invoice hash.            |*/
	double	inv_amt;	/*| detail invoice amount.          |*/
	int	cheq_hash;		/*| cheq structure pointer.         |*/
}	*dtls;				/*-----------------------------------*/
	DArray	dtls_d;	
	int		detailsCount;

	double	totalBalance [4]	=	{0, 0, 0, 0};
/*
 * Local function prototypes 
 */
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
int		ProcessFile		 (void);
void	ValidCheque		 (void);
void	GetCheques		 (void);

int
main (
	int		argc, 
	char 	*argv [])
{
	int	i; 

	OpenDB ();

	/*
	 *	Allocate initial lines for cheques and details
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 2000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 2000);

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, "  ");
	strcpy (sumr_rec.acronym, "         ");
	
	dsp_screen ("Updating Supplier Periods.", comm_rec.co_no, comm_rec.co_name);
	cc = find_rec (sumr, &sumr_rec, GTEQ , "u");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		ValidCheque ();
		GetCheques ();
	
		suin_rec.date_of_inv = 0L;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, GTEQ, "u");
		while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			ProcessFile ();
			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
		abc_unlock (suin);

		dsp_process ("Supplier :", sumr_rec.acronym);

		sumr_rec.bo_curr	=	totalBalance [0];
		sumr_rec.bo_per1	=	totalBalance [1];
		sumr_rec.bo_per2	=	totalBalance [2];
		sumr_rec.bo_per3	=	totalBalance [3];

		for (i = 0; i < 4; i++)
			totalBalance [i] = 0;

		cc = abc_update (sumr, &sumr_rec);
		if (cc)
			file_err (cc, sumr, "DBUPDATE");

		cc = find_rec (sumr, &sumr_rec, NEXT, "u");
	}
	abc_unlock (sumr);

	ArrDelete (&dtls_d);
	ArrDelete (&cheq_d);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
ProcessFile (void)
{
	int	i;

	double	balance = 0.00;

	balance = suin_rec.amt;

	for (i = 0; i < detailsCount; i++)
		if (dtls[i].hhsiHash == suin_rec.hhsi_hash)
			balance -= dtls[i].inv_amt;

	suin_rec.amt_paid = suin_rec.amt - balance;

	cc = abc_update (suin, &suin_rec);
	if (cc)
		file_err (cc, suin, "DBUPDATE");

	if (balance == 0)
		return (EXIT_FAILURE);

	i = age_bals
		(
			comm_rec.pay_terms, 
			suin_rec.date_of_inv, 
			comm_rec.crd_date
		);
	totalBalance[i] += balance;

	return (EXIT_SUCCESS);
}

/*
 * Read cheque routine for supplier.        
 */
void
ValidCheque (void)
{
	abc_selfield (suin, "suin_id_no2");

	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;

		cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
		/*-----------------------------
		| Check for missing deposits. |
		-----------------------------*/
		if (cc || suhd_rec.hhsp_hash != sudt_rec.hhsp_hash)
		{
			suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
			strcpy (suin_rec.inv_no, "DEPOSIT->SYSTEM");
			
			if (find_rec (suin, &suin_rec, COMPARISON, "r"))
			{
				cc = find_rec (suhd, &suhd_rec, NEXT, "r");
				continue;
			}
			sudt_rec.hhsp_hash 		= suhd_rec.hhsp_hash;
			sudt_rec.hhsi_hash 		= suin_rec.hhsi_hash;
			sudt_rec.amt_paid_inv 	= suhd_rec.tot_amt_paid;
			sudt_rec.loc_paid_inv 	= suhd_rec.loc_amt_paid;
			sudt_rec.exch_variatio  = suhd_rec.exch_variance;
			sudt_rec.exch_rate 		= 1.0;
			strcmp (sudt_rec.stat_flag, "0");
			cc = abc_add (sudt, &sudt_rec);
			if (cc)
				file_err (cc, sudt, "DBADD");
		}
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
	abc_selfield (suin, "suin_cron");
}

/*
 * Read cheque routine for supplier.        
 */
void
GetCheques (void)
{
	chequeCount 	= 0;
	detailsCount 	= 0;

	suhd_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	for (cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
		!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash;
		cc = find_rec (suhd, &suhd_rec, NEXT, "r"))
	{
		if (!ArrChkLimit (&cheq_d, cheq, chequeCount))
			sys_err ("ArrChkLimit (cheques)", ENOMEM, PNAME);

		strcpy (cheq [chequeCount].no, 	  suhd_rec.cheq_no);
		cheq [chequeCount].amount		= suhd_rec.tot_amt_paid;
		/*
		 *	Read in associated details
		 */
		sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
		for (cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
			!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash;
			cc = find_rec (sudt, &sudt_rec, NEXT, "r"))
		{
			if (!ArrChkLimit (&dtls_d, dtls, detailsCount))
				sys_err ("ArrChkLimit (dtls)", ENOMEM, PNAME);

			dtls [detailsCount].hhsiHash		= sudt_rec.hhsi_hash;
			dtls [detailsCount].inv_amt			= sudt_rec.amt_paid_inv;
			dtls [detailsCount].cheq_hash		= chequeCount;
			++detailsCount;
		}
		++chequeCount;
	}
}

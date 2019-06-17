/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_ddelete.c,v 5.3 2001/12/10 05:08:21 scott Exp $
|  Program Name  : (cr_ddelete.c)
|  Program Desc  : (Delete Suppliers Payed Invoices/Cheques)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_ddelete.c,v $
| Revision 5.3  2001/12/10 05:08:21  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_ddelete.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_ddelete/cr_ddelete.c,v 5.3 2001/12/10 05:08:21 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define		DELETE_INVOICE 	 (suin_rec.stat_flag [0] == '9')
#define		DELETE_CHEQUE 	 (suhd_rec.stat_flag [0] == '9')
#define		DELETE_DETAIL 	 (sudt_rec.stat_flag [0] == '9')

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct suinRecord	suin_rec;
struct suhdRecord	suhd_rec;
struct sudtRecord	sudt_rec;

	int		envCrCo = 0;
	char	branchNumber [3];

/*
 * Local function prototypes
 */
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
void	ProcessSumr		 (void);
void	DeleteInvoice	 (long);
void	DeleteCheque	 (long);
int		CheckSudt		 (long);
void	DeleteDetail	 (long);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char 	*argv [])
{
	envCrCo = atoi (get_env ("CR_CO"));

	OpenDB ();

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	sprintf (err_str, ML ("Deleting Suppliers Transactions."));
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	ProcessSumr ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open database Files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comm, comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsu_hash");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
}

/*
 * Close database Files.
 */
void
CloseDB (void)
{
	abc_fclose (suin);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_fclose (comm);
	abc_fclose (sumr);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	rset_tty ();
	crsr_on ();
	CloseDB (); 
	FinishProgram ();
}

/*
 * Process customers master file from start to finish.
 */
void
ProcessSumr (void)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.acronym, "         ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");

	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
                      !strcmp (sumr_rec.est_no, branchNumber))
	{
		dsp_process ("Supplier : ", sumr_rec.crd_no);

		DeleteInvoice (sumr_rec.hhsu_hash);
		DeleteCheque (sumr_rec.hhsu_hash);

		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
}

/*
 * Delete all suin records with '9' stat_flags.
 */
void
DeleteInvoice (
	long		hhsuHash)
{
	/*
 	 * Delete all suin records for supplier with '9' stat_flags.
	 */
	suin_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (suin, &suin_rec, GTEQ, "u");
	while (!cc && suin_rec.hhsu_hash == hhsuHash)
	{
		dsp_process ("Delete Invoices", "Paid");

		if (DELETE_INVOICE)
		{
			cc = abc_delete (suin);
			if (cc)
				file_err (cc, suin, "DBDELETE");
				
			cc = find_rec (suin, &suin_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
	}
	abc_unlock (suin);
}

/*
 * Delete cheque header and cheque details.
 */
void
DeleteCheque (
	long	hhsuHash)
{
	/*
	 * Process and delete cheque details.
	 */
	suhd_rec.hhsu_hash	=	hhsuHash;
   	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsu_hash == hhsuHash)
	{
		if (!DELETE_CHEQUE)
		{
   			cc = find_rec (suhd, &suhd_rec, NEXT, "r");
			continue;
		}
		DeleteDetail (suhd_rec.hhsp_hash);

   		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}

	suhd_rec.hhsu_hash	=	hhsuHash;
   	cc = find_rec (suhd, &suhd_rec, GTEQ, "u");
	while (!cc && suhd_rec.hhsu_hash == hhsuHash)
	{
		if (!DELETE_CHEQUE)
		{
			abc_unlock (suhd);
   			cc = find_rec (suhd, &suhd_rec, NEXT, "u");
			continue;
		}
		if (CheckSudt (suhd_rec.hhsp_hash))
		{
			cc = abc_delete (suhd);
			if (cc)
				file_err (cc, suhd, "DBDELETE");

   			cc = find_rec (suhd, &suhd_rec, GTEQ, "u");
		}
		else
   			cc = find_rec (suhd, &suhd_rec, NEXT, "u");
	}
	abc_unlock (suhd);
}

/*
 * Routine to check for the existance of sudt records
 * and return (1) if none found, else update suhd.    
 */
int
CheckSudt (
	long	hhspHash)
{
	double	locTotal = 0.00, 
			fgnTotal = 0.00;

	/*
	 * check if any cheque deteils remain.
	 */
	sudt_rec.hhsp_hash = hhspHash;
	cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
	if (cc || sudt_rec.hhsp_hash != hhspHash)
		return (EXIT_FAILURE);

	/*
	 * Add up remaining details.
	 */
	while (!cc && sudt_rec.hhsp_hash == hhspHash)
	{
		locTotal += sudt_rec.loc_paid_inv;
		fgnTotal += sudt_rec.amt_paid_inv;

		cc = find_rec (sudt, &sudt_rec, NEXT, "r");
	}

	/*
	 * Update cheque With new Totals.
	 */
	suhd_rec.loc_amt_paid 	= locTotal;
	suhd_rec.tot_amt_paid 	= fgnTotal;
	suhd_rec.loc_disc_take  = 0.00;
	suhd_rec.disc_taken 	= 0.00;
	strcpy (suhd_rec.stat_flag, "0");
	cc = abc_update (suhd, &suhd_rec);
	if (cc)
		file_err (cc, suhd, "DBFIND");

	return (EXIT_SUCCESS);
}

/*
 * Routine to delete all valid cudt records.
 */
void
DeleteDetail (
	long	hhspHash)
{
	sudt_rec.hhsp_hash = hhspHash;
	cc = find_rec (sudt, &sudt_rec, GTEQ, "u");
	while (!cc && sudt_rec.hhsp_hash == hhspHash)
	{
		if (DELETE_DETAIL)
		{
			dsp_process ("Delete Cheques", "Zero Balance");
			cc = abc_delete (sudt);
			if (cc)
				file_err (cc, sudt, "DBDELETE");

			cc = find_rec (sudt, &sudt_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (sudt);
			cc = find_rec (sudt, &sudt_rec, NEXT, "r");
		}
	}
	abc_unlock (sudt);
}

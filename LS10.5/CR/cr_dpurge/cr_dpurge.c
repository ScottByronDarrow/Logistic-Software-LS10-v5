/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_dpurge.c,v 5.6 2001/12/06 09:03:02 scott Exp $
|  Program Name  : (cr_dpurge.c  ) 
|  Program Desc  : (Prints and Flags Supplier  Invoices/ Cheques.) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_dpurge.c,v $
| Revision 5.6  2001/12/06 09:03:02  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.5  2001/12/06 08:17:07  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char *PNAME = "$RCSfile: cr_dpurge.c,v $";
char *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_dpurge/cr_dpurge.c,v 5.6 2001/12/06 09:03:02 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_cr_mess.h>
#include 	<DeleteControl.h>

#include    "schema"

struct commRecord   comm_rec;
struct suinRecord   suin_rec;
struct suhdRecord   suhd_rec;
struct sudtRecord   sudt_rec;
struct sumrRecord   sumr_rec;

typedef int	BOOL;

/*
 * The structure 'dtls' is initialised in function 'GetCheques'
 * the number of details is stored in external variable 'detailsCount'.
 */

int detailsCount;

#define MAX_DTLS 4000
struct
{
	long 	hhspHash;		/* Supplier Payments header         */
	long 	hhsiHash;		/* detail invoice reference.        */
	double	invAmount;		/* detail invoice amount.           */
	char 	expired;	/* 'Y' if suin_amt == sumof payments*/  
	long	dop;			/* Date of Payment - from suhd      */
}
dtls [MAX_DTLS];

char oldName [sizeof sumr_rec.crd_name];

int		envCrCo			= 0,
		printerNumber	= 1;

long	envCrPurge		= 0;

FILE 	*p;

char 	envCoClose [6],
		branchNumber [3];

/*
 * Local function prototypes
 */
static BOOL	IsDeleteOk_suhd	 (long);
static BOOL	IsDeleteOk_suin	 (long);
void		OpenDB			 (void);
void		CloseDB			 (void);
void		SupplierHeading	 (void);
void		ProcessLine		 (void);
void		StartReport		 (void);
void		PurgeData		 (void);
void		GetCheques		 (long);

int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	long	compDate;
	char 	*sptr;

	printerNumber = atoi (get_env ("MEND_LP"));
	envCrCo = atoi (get_env ("CR_CO"));
	envCrPurge = atol (get_env ("CR_PURGE"));

	sptr = chk_env ("CO_CLOSE");
    if (sptr == (char *)0)
        sprintf (envCoClose, "%-5.5s", "11111");
    else
        sprintf (envCoClose, "%-5.5s", sptr);

	OpenDB ();
	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "SUPPLIER-LEDGER");
	if (!cc)
	{
		envCrPurge		= (long) delhRec.purge_days;
	}
	/*
	 * Continue only if files locked successfully.
	 */
	sprintf (err_str, "Printing Suppliers Deleted Transactions.");
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	/*
	 * Initialise standard print.
	 */
	StartReport ();

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, (envCoClose [1] == '1') ? "  " : branchNumber);
	strcpy (sumr_rec.acronym, "         ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");

	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) &&
			 (envCoClose [1] == '1' || !strcmp (sumr_rec.est_no,branchNumber)))
	{
		dsp_process ("Supplier : ", sumr_rec.crd_no);

		/*
		 * Initialise structure 'dtls []'.
		 */
		GetCheques (sumr_rec.hhsu_hash);

		suin_rec.hhsu_hash 		= sumr_rec.hhsu_hash;
		suin_rec.date_of_inv 	= 0L;
		cc = find_rec (suin, &suin_rec, GTEQ, "u");
		while (!cc && suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			compDate = 0L;
			for (i = 0; i < detailsCount; i++)
			{
				if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
				{
					if (compDate < dtls [i].dop)
						compDate = dtls [i].dop;
				}
			}
			if ((comm_rec.crd_date - envCrPurge) >= compDate)
				ProcessLine ();

			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
		abc_unlock (suin);

		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	fprintf (p, ".EOF\n");
	pclose (p);
	CloseDB (); 
	FinishProgram ();

	sprintf (err_str, "cr_ddelete");
	SystemExec (err_str, FALSE);

	return (EXIT_SUCCESS);
}

/*
 * Return TRUE if all invoices referenced by this payment are paid in full
 */
static BOOL 
IsDeleteOk_suhd (
	long	hhspHash)
{
	int i;
	BOOL found = FALSE;

	for (i = 0; i < detailsCount; i++)
	{
		if (dtls [i].hhspHash == hhspHash)
		{
			if (dtls [i].expired != 'Y')	
				return (FALSE);
			found = TRUE;
		}
	}
	if (!found)
		return (FALSE);

	return (TRUE);
}

/*
 * Return TRUE if all payment records referenced
 * by this invoice can be deleted
 */
static BOOL 
IsDeleteOk_suin (
	long	hhsiHash)
{
	int i;
	BOOL found = FALSE;

	for (i = 0; i < detailsCount; i++)
	{
		if (dtls [i].hhsiHash == hhsiHash) 
		{
			if (!IsDeleteOk_suhd (dtls [i].hhspHash))	
				return (FALSE);
			found = TRUE;
		}
	}
	if (!found)
		return (FALSE);

	return (TRUE);
}

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
SupplierHeading (void)
{
	fprintf (p, ".LRP4\n");
	fprintf (p, "|           |           |                    |          |");
	fprintf (p, "             |          |           |\n");
	fprintf (p, "| %s (%s)                                         |\n", 
			 sumr_rec.crd_no, sumr_rec.crd_name);
	fprintf (p, "|           |           |                    |          |");
	fprintf (p, "             |          |           |\n");
}

void
ProcessLine (void)
{
	/*
	 * INVOICE CAN BE PURGED. Do a double check in invAmount == paid amt
	 */
	if (IsDeleteOk_suin (suin_rec.hhsi_hash))
	{
		if (strcmp (sumr_rec.crd_name, oldName))
		{
			/*
			 * Print debtor name on change of debtor.
			 */
			SupplierHeading ();
			strcpy (oldName, sumr_rec.crd_name);
		}
		PurgeData ();
	}
}

void
StartReport (void)
{
	if ((p = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (p, ".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (p, ".LP%d\n", printerNumber);
	fprintf (p, ".PI12\n");
	fprintf (p, ".11\n");
	fprintf (p, ".L124\n");
	fprintf (p, ".CSUPPLIER DELETED TRANSACTIONS\n");
	if (strcmp (branchNumber, " 0") != 0)
		fprintf (p, ".B1\n.C%s%s\n", "FOR ", clip (comm_rec.est_short));
	else
		fprintf (p, ".B1\n");
	fprintf (p, ".C%s AS AT%s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (p, ".B1\n");

	fprintf (p, ".R======================================================");
	fprintf (p, "========================================\n");

	fprintf (p, "========================================================");
	fprintf (p, "======================================\n");

	fprintf (p, "|  DATE OF  |    DATE   |       INVOICE      |  INVOICE ");
	fprintf (p, "|    PAYMENT  |  PAYMENT |  DATE OF  |\n");

	fprintf (p, "|TRANSACTION|   POSTED  |       NUMBER       |  AMOUNT  ");
	fprintf (p, "|    NUMBER   |  AMOUNT  |   CHEQUE  |\n");

	fprintf (p, "|-----------+-----------+--------------------+----------");
	fprintf (p, "+-------------+----------+-----------|\n");

	fprintf (p, ".PI12\n");
}

void
PurgeData (void)
{
	char date1 [11], date2 [11];

	int i;
	BOOL none = TRUE;
	BOOL first = TRUE;

	abc_selfield (suhd, "suhd_hhsp_hash");
	abc_selfield (sudt, "sudt_id_no");

	/*
	 * Flag suin record.
	 */
	strcpy (suin_rec.stat_flag, "9");
	cc = abc_update (suin, &suin_rec);
	if (cc)
		file_err (cc, suin, "DBUPDATE");

	strcpy (date1, DateToString (suin_rec.date_of_inv));
	strcpy (date2, DateToString (suin_rec.date_posted));

	fprintf (p, "| %10.10s| %10.10s| %s/%s |%10.2f", 
		 	 date1,
			 date2,
			 suin_rec.est,
			 suin_rec.inv_no,
			 DOLLARS (suin_rec.amt));

	for (i = 0; i < detailsCount; i++)
	{
		if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
		{
			/* Note: dtls [] is sorted by suhd_cheq_no */

			none = FALSE;

			/*
			 * Find & Flag suhd record.
			 */
			suhd_rec.hhsp_hash = dtls [i].hhspHash;
			cc = find_rec (suhd, &suhd_rec, EQUAL, "u");
			if (!cc)
			{
				strcpy (suhd_rec.stat_flag, "9");
				cc = abc_update (suhd, &suhd_rec);
				if (cc)
					file_err (cc, suhd, "DBUPDATE");
			}
			else
			{
				abc_unlock (suhd);
				continue;
			}

			/*
			 * Find & Flag sudt record.
			 */
			sudt_rec.hhsi_hash = dtls [i].hhsiHash;
			sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
			cc = find_rec (sudt, &sudt_rec, GTEQ, "u");
			while (!cc &&
				   sudt_rec.hhsi_hash == dtls [i].hhsiHash &&
				   sudt_rec.hhsp_hash == suhd_rec.hhsp_hash)
			{
				if (dtls [i].invAmount == sudt_rec.amt_paid_inv)
				{
					strcpy (sudt_rec.stat_flag, "9");
					cc = abc_update (sudt, &sudt_rec);
					if (cc)
						file_err (cc, sudt, "DBUPDATE");
				}
				abc_unlock (sudt);
				cc = find_rec (sudt, &sudt_rec, NEXT, "u");
			}
			abc_unlock (sudt);

			if (first)
				first = FALSE;
			else
				fprintf (p, "|           |           |                    |          ");

			fprintf (p, "|%s|%10.2f| %10.10s|\n", suhd_rec.cheq_no,
					 DOLLARS (dtls [i].invAmount), 
					 DateToString (suhd_rec.date_payment));
		}
	}

	if (none)
		fprintf (p, "|    NONE     |       0.0|           |\n");

	abc_selfield (suhd, "suhd_id_no");
	abc_selfield (sudt, "sudt_hhsp_hash");
}

void
GetCheques (
	long 	hhsuHash)
{
	int		i;
	long	compDate;
	BOOL dtls_full = FALSE;
	detailsCount = 0;

	memset (&suhd_rec, 0, sizeof (suhd_rec));
	suhd_rec.hhsu_hash = hhsuHash;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");

	while (!cc && !dtls_full && suhd_rec.hhsu_hash == hhsuHash)
	{
		sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
		cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
		while (!cc && sudt_rec.hhsp_hash == suhd_rec.hhsp_hash)
		{
			if (detailsCount == MAX_DTLS)
			{
				/* Cannot read all details of last payment so unwind */
				while (dtls [--detailsCount].hhspHash == suhd_rec.hhsp_hash)
					;
				dtls_full = TRUE;
				break;
			}
			dtls [detailsCount].hhspHash = sudt_rec.hhsp_hash;
			dtls [detailsCount].hhsiHash = sudt_rec.hhsi_hash;
			dtls [detailsCount].invAmount = sudt_rec.amt_paid_inv;
			dtls [detailsCount].expired = 'N';
			dtls [detailsCount].dop	= suhd_rec.date_payment;
			++detailsCount;

			cc = find_rec (sudt, &sudt_rec, NEXT, "r");
		}
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}

	/* Check if all invoices are paid in full and expired */
	suin_rec.hhsu_hash = hhsuHash;
	suin_rec.date_of_inv = 0L;
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && suin_rec.hhsu_hash == hhsuHash)
	{
		compDate = 0L;
		for (i = 0; i < detailsCount; i++)
			if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
			{
				if (compDate < dtls [i].dop)
					compDate = dtls [i].dop;
			}

		if ((comm_rec.crd_date - envCrPurge) >= compDate)
		{
			double balance = suin_rec.amt;

			for (i = 0; i < detailsCount; i++)
				if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
					balance -= dtls [i].invAmount;
			if (balance == 0.0)
			{
				for (i = 0; i < detailsCount; i++)
					if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
						dtls [i].expired = 'Y';
			}
		}
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
}

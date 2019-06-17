/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_percalc.c,v 5.3 2002/06/25 03:17:05 scott Exp $
|  Program Name  : (cr_percalc.c)
|  Program Desc  : (Update Suppliers Current , Period 1 - 3 Periods)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 01/06/87          |
|---------------------------------------------------------------------|
| $Log: cr_percalc.c,v $
| Revision 5.3  2002/06/25 03:17:05  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_percalc.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_percalc/cr_percalc.c,v 5.3 2002/06/25 03:17:05 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

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
{
	char	cheqNo [16];/* cheque receipt number.	    */
	double	amount;		/* total amount of cheque.	    */
}	*cheq;
	DArray	cheq_d;		/* dynamic info for cheq 		*/
	int		chequeCount;

struct Detail
{
	long	hhsiHash;		/* detail invoice hash.            */
	double	invoiceAmount;	/* detail invoice amount.          */
	int		cheqHash;		/* cheq structure pointer.         */
}	*dtls;
	DArray	dtls_d;		/* dynamic info for dtls */
	int		detailsCount;

	double	totalBalance [4] = {0, 0, 0, 0};

	struct {
		long	workHash;
	} hashWorkRecord;

	int		workFileNumber, 
			processIdNumber;

/*
 * Local function prototypes 
 */
int		OpenDB		(void);
void	CloseDB		(void);
void	ValidCheque	(void);
void	GetCheques	(void);
int 	ProcessFile (void);


/*
 * Main Processing Routine.                    
 */
int
main (
 int	argc, 
 char *	argv [])
{
	int	i; 

	if (argc < 2 || (processIdNumber = atoi(argv [1])) < 1)
	{
		print_at(0, 0, mlStdMess046, argv [0]);
		return (EXIT_FAILURE);
	}
	if (OpenDB())
	{
		crsr_on();
		return (EXIT_FAILURE);
	}

	/*
	 *	Allocate initial lines for cheques and details
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 1000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	dsp_screen("Calculating Suppliers Periods.", comm_rec.co_no, comm_rec.co_name);
	cc = RF_READ(workFileNumber, (char *) &hashWorkRecord);
	while (!cc)
	{
		/*---------------------------------------------
		| Read supplier record from work file hash.   |
		---------------------------------------------*/
		sumr_rec.hhsu_hash = hashWorkRecord.workHash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (sumr);
			cc = RF_READ(workFileNumber, (char *) &hashWorkRecord);
			continue;
		}

		ValidCheque();
		GetCheques();
	
		suin_rec.date_of_inv = 0L;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, GTEQ, "u");
		while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			ProcessFile();
			abc_unlock(suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
		}
		abc_unlock(suin);

		dsp_process("Supplier :", sumr_rec.acronym);
		
		sumr_rec.bo_curr	=	totalBalance [0];
		sumr_rec.bo_per1	=	totalBalance [1];
		sumr_rec.bo_per2	=	totalBalance [2];
		sumr_rec.bo_per3	=	totalBalance [3];

		for (i = 0; i < 4; i++)
			totalBalance [i] = 0;

		cc = abc_update(sumr, &sumr_rec);
		if (cc)
			file_err (cc, sumr, "DBUPDATE");

		cc = RF_READ(workFileNumber, (char *) &hashWorkRecord);
	}
	ArrDelete (&dtls_d);
	ArrDelete (&cheq_d);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open All Database files needed. 
 */
int
OpenDB (void)
{
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	sprintf(filename, "%s/WORK/cr_per%05d", 
				(sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processIdNumber);

	abc_dbopen("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

	return (RF_OPEN(filename, sizeof(hashWorkRecord), "r", &workFileNumber));
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
	RF_DELETE(workFileNumber);
}

int
ProcessFile (void)
{
	int	i;

	double	balance = 0.00;

	balance = suin_rec.amt;

	for (i = 0; i < detailsCount; i++)
		if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
			balance -= dtls [i].invoiceAmount;

	suin_rec.amt_paid = suin_rec.amt - balance;

	cc = abc_update(suin, &suin_rec);
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

	totalBalance [i] += balance;

	return (EXIT_SUCCESS);
}

/*
 * Read cheque routine for supplier.        
 */
void
ValidCheque (void)
{
	abc_selfield(suin, "suin_id_no2");

	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;

		cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
		/*
		 * Check for missing deposits. 
		 */
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
			strcmp(sudt_rec.stat_flag, "0");
			cc = abc_add(sudt, &sudt_rec);
			if (cc)
				file_err (cc, sudt, "DBADD");
		}
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
	abc_selfield(suin, "suin_cron");
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
			sys_err ("ArrChkLimit(cheques)", ENOMEM, PNAME);

		strcpy (cheq [chequeCount].cheqNo, 	  suhd_rec.cheq_no);
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
				sys_err ("ArrChkLimit(dtls)", ENOMEM, PNAME);

			dtls [detailsCount].hhsiHash		= sudt_rec.hhsi_hash;
			dtls [detailsCount].invoiceAmount	= sudt_rec.amt_paid_inv;
			dtls [detailsCount].cheqHash		= chequeCount;
			++detailsCount;
		}
		++chequeCount;
	}
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_chqtogl.c,v 5.5 2002/06/25 03:46:23 scott Exp $
|  Program Name  : (cr_chqtogl.c  )                                 |
|  Program Desc  : (Generates sudp, suht records from suhd, sudt)   |
|                 (records as well as gl_work transactions     )   |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 27/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_chqtogl.c,v $
| Revision 5.5  2002/06/25 03:46:23  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.4  2002/06/25 03:16:59  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqtogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqtogl/cr_chqtogl.c,v 5.5 2002/06/25 03:46:23 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>

#include	"schema"

#define MAXBRANCHES	100

	struct	commRecord	comm_rec;
	struct	crbkRecord	crbk_rec;
	struct	sumrRecord	sumr_rec;
	struct	suhdRecord	suhd_rec;
	struct	sudtRecord	sudt_rec;
	struct	suinRecord	suin_rec;
	struct	suhpRecord	suhp_rec;
	struct	suhtRecord	suht_rec;
	struct	wfbcRecord	wfbc_rec;

	struct {
		long	wk_hash;
	} wk_rec;

/*
 * Table names
 */
static char *data	= "data";

/*
 * Globals 
 */

static int	currCrMonth,
			printerNumber,
			noDataFound = 0;

static long	pid;

static char	bankGLAccount	 [MAXLEVEL + 1],
	    	bankGLCharge	 [MAXLEVEL + 1],
			bankExchVar		 [MAXLEVEL + 1],
			passDocumentType [2],
			passHistoryType	 [2];

static long	systemDate;

static	double	batchTotal;

/*
 * Total for each branchNolishment. 
 */
static double	branchTotal [MAXBRANCHES];
static double	branchLocal [MAXBRANCHES];

/*
 * Local function prototypes 
 */
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
int		FindGlmr		 (char *);
int		ProcessSuhd		 (void);
void	SumCheque		 (void);
void	MakeCheque		 (void);
void	WriteCharges	 (char *, long, double, double, double, char *);
void	WriteBankAcc	 (char *, long, double, double, double, char *);
void	WriteExchVar	 (char *, long, double, double, double, char *);
void	WriteDiscount	 (char *, long, double, double, double, char *);
void	WriteGlwk		 (char *, long, double, double, double, char *);
void	UpdateWfbc		 (void);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	/*
	 * Handle args
	 */
	switch (argc)
	{
	case 5	:
		pid = atol (argv [1]);

		sprintf (passDocumentType, "%-1.1s", argv [2]);
		sprintf (passHistoryType, "%-1.1s", argv [3]);
		printerNumber = atoi (argv [4]);
		if (passDocumentType [0] == 'C' || passDocumentType [0] == 'D' || passDocumentType [0] == 'T')
			break;

		/* fall thru' */

	default	:
		print_at (0,0,mlCrMess701, argv [0]);
		return (EXIT_FAILURE);
	}

	systemDate = TodaysDate ();

	OpenDB ();

	DateToDMY (comm_rec.crd_date, NULL, &currCrMonth, NULL);

	set_tty ();
	init_scr ();

	dsp_screen (" Processing : Creating GL Workfile ",
					comm_rec.co_no, comm_rec.co_name);

	/*
	 * Process all cheques in range given. 
	 */
	suhd_rec.pid = pid;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "u");
	while (!cc && suhd_rec.pid == pid)
	{
		if (ProcessSuhd ())
		{
			abc_unlock (suhd);
			cc = find_rec (suhd, &suhd_rec, NEXT, "u");
		}
		else
		{
			abc_unlock (suhd);
			cc = find_rec (suhd, &suhd_rec, GTEQ, "u");

		}
	}
	abc_unlock (suhd);

	if (noDataFound == 1)
		UpdateWfbc ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_pid");
	open_rec (suhp, suhp_list, SUHP_NO_FIELDS, "suhp_id_no");
	open_rec (suht, suht_list, SUHT_NO_FIELDS, "suht_hhsq_hash");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");
	OpenGlmr ();

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close Database Files. 
 */
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	abc_fclose (glmr);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_fclose (suhp);
	abc_fclose (suht);
	abc_fclose (suin);
	abc_fclose (sumr);
	abc_fclose (wfbc);
	GL_CloseBatch (printerNumber);
	GL_Close ();

	abc_dbclose (data);
}

/*
 * Exit Program Routine. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Read gl account master.    
 */
int
FindGlmr (
	char	*account)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,account);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P' && glmrRec.glmr_class [0][0] != 'F')
	      return (2);

	return (EXIT_SUCCESS);
}

/*
 *	Routine to process cheque.  							
 *	Checks if there is supplier record on file for cheque.	
 *	- if there isn't then control is returned to main.		
 *	Returns: non-zero if not ok.							
 */
int
ProcessSuhd (void)
{
	if (suhd_rec.tot_amt_paid == 0 && suhd_rec.disc_taken == 0)
	{
		suhd_rec.pid = 0L;

		cc = abc_update (suhd, &suhd_rec);
		if (cc)
			file_err (cc, suhd, "DBUPDATE");

		return (EXIT_SUCCESS);
	}
	if (passDocumentType [0] != suhd_rec.pay_type [0])
		return (EXIT_FAILURE);

	/*
	 * Read supplier master record.   
  	 */
	sumr_rec.hhsu_hash = suhd_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sumr, "DBFIND");

	dsp_process ("Cheque : ", suhd_rec.cheq_no);

	SumCheque ();
	MakeCheque ();

	suhd_rec.pid = 0L;

	cc = abc_update (suhd, &suhd_rec);
	if (cc)
		file_err (cc, suhd, "DBUPDATE");

	noDataFound = 1;
	return (EXIT_SUCCESS);
}


/*
 *	Routine to sum the cheque detail into the			
 *	correct branchNolishments.  Also creates the glwk	
 *	records as it goes for each sudt record.			
 *	Returns: 0 if ok, non-zero otherwise.				
 */
void
SumCheque (void)
{
	int		branchNumber,
			chequePeriodNo,
			i;

	char	chequePeriodString [3];

	long	workHash		= 0L;
	long	chargesHash 	= 0L;

	double	fgnChequeAmount		= 0.00;
	double	localChequeAmount 	= 0.00;
	double	exchangeRate		= 0.00;

	/*
	 * Add gl transaction for cheque header.     
	 */
	if (suhd_rec.date_payment <= MonthEnd (comm_rec.gl_date))
		chequePeriodNo = currCrMonth;
	else
		chequePeriodNo = 13;

	sprintf (chequePeriodString, "%02d", chequePeriodNo);

	cc = FindGlmr (sumr_rec.gl_ctrl_acct);
	if (cc)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"  ",
			"SUSPENSE  ",
			"   ",
			" "
		);
	}
	sprintf (glwkRec.acc_no, glmrRec.acc_no);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acronym, "%-9.6s", sumr_rec.crd_no);
	sprintf (glwkRec.name, "%-30.30s", sumr_rec.crd_name);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", suhd_rec.cheq_no);
	glwkRec.ci_amt = 0.0;
	glwkRec.o1_amt = no_dec (suhd_rec.tot_amt_paid);

	glwkRec.o2_amt = 0.0;
	glwkRec.o3_amt = 0.0;
	glwkRec.o4_amt = 0.0;
	strcpy (glwkRec.tran_type, " 9");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.tran_date = suhd_rec.date_payment;
	strcpy (glwkRec.period_no, chequePeriodString);
	glwkRec.post_date = systemDate;
	sprintf (glwkRec.narrative, "AP : %15.15s", suhd_rec.cheq_no);
	strcpy (glwkRec.user_ref, sumr_rec.crd_no);
	sprintf (glwkRec.alt_desc1, " ");
	sprintf (glwkRec.alt_desc2, " ");
	sprintf (glwkRec.alt_desc3, " ");
	sprintf (glwkRec.batch_no, " ");

	/*
	 * changed this so that disc added back on to be put to suppliers gl
	 */
	if (suhd_rec.loc_amt_paid != 0.0)
		glwkRec.exch_rate = suhd_rec.tot_amt_paid / suhd_rec.loc_amt_paid;
	else
		glwkRec.exch_rate = 1.0;

	glwkRec.loc_amount =
		no_dec (suhd_rec.loc_amt_paid + suhd_rec.loc_disc_take);
	glwkRec.amount =
		no_dec (suhd_rec.tot_amt_paid + suhd_rec.disc_taken);
	strcpy (glwkRec.currency, sumr_rec.curr_code);
	strcpy (glwkRec.stat_flag, "2");
	strcpy (glwkRec.jnl_type, "1");

	GL_AddBatch ();

	/*
	 * Add cheque header amount to bank ctrl total.   
	 */
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", suhd_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (!cc)
	{
		sprintf (bankGLAccount, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
		cc = FindGlmr (bankGLAccount);
		workHash = glmrRec.hhmr_hash;
	}
	if (cc)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"  ",
			"BANK ACCT ",
			"   ",
			" "
		);
		workHash = glmrRec.hhmr_hash;
		strcpy (bankGLAccount, glmrRec.acc_no);
	}
	/*
	 * same again for charges 
	 */
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (!cc)
	{
		sprintf (bankGLCharge, "%-*.*s",MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_chg);
		cc = FindGlmr (crbk_rec.gl_bank_chg);
		chargesHash = glmrRec.hhmr_hash;
	}
	if (cc)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"  ",
			"BANK ACCT ",
			"   ",
			" "
		);
		strcpy (bankGLCharge, glmrRec.acc_no);
		chargesHash = glmrRec.hhmr_hash;
	}

	localChequeAmount 	= no_dec (suhd_rec.loc_amt_paid + suhd_rec.bank_chg);
	fgnChequeAmount  	= no_dec (suhd_rec.tot_amt_paid);
	exchangeRate 		= suhd_rec.tot_amt_paid / suhd_rec.loc_amt_paid;

	WriteBankAcc 
	(
		bankGLAccount,
		workHash,
		fgnChequeAmount,
		localChequeAmount,
		exchangeRate,
		crbk_rec.curr_code
	);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		"  ",
		"DISC ALLOW",
		"   ",
		" "
	);
	WriteDiscount 
	(
		glmrRec.acc_no,
		glmrRec.hhmr_hash,
		suhd_rec.disc_taken,
		suhd_rec.loc_disc_take,
		exchangeRate,
		sumr_rec.curr_code
	);

	WriteCharges 
	(
		bankGLCharge,
		chargesHash,
		suhd_rec.bank_chg,
		no_dec (suhd_rec.bank_chg / exchangeRate),
		exchangeRate,
		crbk_rec.curr_code
	);

	/*
	 * Add exchange variation transaction for cheque. 
	 */
	if (suhd_rec.exch_variance)
	{
		/*
		 * Add cheque header exch var. to bank ctrl total.
		 */
		strcpy (crbk_rec.co_no, comm_rec.co_no);
		sprintf (crbk_rec.bank_id, "%-5.5s", suhd_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (bankExchVar, crbk_rec.gl_exch_var);
			cc = FindGlmr (bankExchVar);
			workHash = glmrRec.hhmr_hash;
		}
		if (cc)
		{
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"SUSPENSE  ",
				"   ",
				" "
			);
			strcpy (bankExchVar, glmrRec.acc_no);
			workHash = glmrRec.hhmr_hash;
		}

		WriteExchVar 
		(
			bankExchVar,
			workHash,
			0.00,	/* Fgn cannot have a variance	*/
			suhd_rec.exch_variance, 
			1.00, 
			crbk_rec.curr_code
		);
		if (FindGlmr (sumr_rec.gl_ctrl_acct))
		{
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"ACCT PAY  ",
				"   ",
				" "
			);
			strcpy (glwkRec.acc_no, glmrRec.acc_no);
			glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		}
		else
		{
			sprintf (glwkRec.acc_no, sumr_rec.gl_ctrl_acct);
			glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		}
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.est_no, comm_rec.est_no);
		sprintf (glwkRec.acronym, "%-9.6s", sumr_rec.crd_no);
		sprintf (glwkRec.name, "%-30.30s", sumr_rec.crd_name);
		sprintf (glwkRec.chq_inv_no, "%-15.15s", suhd_rec.cheq_no);
		glwkRec.ci_amt = 0.0;
		glwkRec.o1_amt = 0.0;
		glwkRec.o2_amt = 0.0;
		glwkRec.o3_amt = 0.0;
		glwkRec.o4_amt = 0.0;
		strcpy (glwkRec.tran_type, " 9");
		sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
		glwkRec.tran_date = suhd_rec.date_payment;
		strcpy (glwkRec.period_no, chequePeriodString);
		glwkRec.post_date = systemDate;
		sprintf (glwkRec.narrative, "EXVAR%15.15s", suhd_rec.cheq_no);
		sprintf (glwkRec.alt_desc1, " ");
		sprintf (glwkRec.alt_desc2, " ");
		sprintf (glwkRec.alt_desc3, " ");
		sprintf (glwkRec.batch_no, " ");
		strcpy (glwkRec.user_ref,  sumr_rec.crd_no);
		if (suhd_rec.exch_variance >= 0.0)
		{
			strcpy (glwkRec.jnl_type, "1");
			glwkRec.amount 		= 0.00;
			glwkRec.loc_amount	= no_dec (suhd_rec.exch_variance);
		}
		else
		{
			strcpy (glwkRec.jnl_type, "2");
			glwkRec.amount 		= 0.00;
			glwkRec.loc_amount	= no_dec (suhd_rec.exch_variance * -1.0);
		}
		strcpy (glwkRec.stat_flag, "2");

		GL_AddBatch ();
	}

	/*
	 * Initialise total array	
	 */
	for (i = 0;i < MAXBRANCHES; i++)
	{
		branchTotal [i] = 0.0;
		branchLocal [i] = 0.0;
	}

	/*
	 *  Sum cheque details into branchNolishments And create glwk records 
	 */
	sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
	cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
	{
		suin_rec.hhsi_hash = sudt_rec.hhsi_hash;
		cc = find_rec (suin, &suin_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (suin_rec.est, "00");
			strcpy (suin_rec.inv_no, "???????????????");
		}
		branchNumber = atoi (suin_rec.est);
		branchTotal [branchNumber] += no_dec (sudt_rec.amt_paid_inv);
		branchLocal [branchNumber] += no_dec (sudt_rec.loc_paid_inv);

		/*
		 * Create glwk record (does not update GL - for jnl print) 
		 */
		strcpy (glwkRec.acc_no, "         ");
		sprintf (glwkRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL," ");
		glwkRec.hhgl_hash = 0L;
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.est_no, suin_rec.est);
		sprintf (glwkRec.acronym, "%-9.6s", sumr_rec.crd_no);
		sprintf (glwkRec.name, "%-15.15s%-15.15s", suin_rec.inv_no, " ");
		sprintf (glwkRec.alt_desc1, " ");
		sprintf (glwkRec.alt_desc2, " ");
		sprintf (glwkRec.alt_desc3, " ");
		sprintf (glwkRec.batch_no, " ");

		sprintf (glwkRec.chq_inv_no, "%-15.15s", suhd_rec.cheq_no);
		glwkRec.ci_amt 		= no_dec (sudt_rec.loc_paid_inv);
		glwkRec.o1_amt 		= no_dec (sudt_rec.amt_paid_inv);
		glwkRec.exch_rate 	= sudt_rec.exch_rate;
		glwkRec.o2_amt 		= 0.0;
		glwkRec.o3_amt 		= 0.0;
		glwkRec.o4_amt 		= 0.0;
		strcpy (glwkRec.tran_type, " 9");
		sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
		glwkRec.tran_date = suhd_rec.date_payment;
		strcpy (glwkRec.period_no, chequePeriodString);
		glwkRec.post_date = systemDate;
		sprintf (glwkRec.narrative, "DISP.%15.15s", suhd_rec.cheq_no);
		sprintf (glwkRec.alt_desc1, " ");
		sprintf (glwkRec.alt_desc2, " ");
		sprintf (glwkRec.alt_desc3, " ");
		sprintf (glwkRec.batch_no, " ");
		strcpy (glwkRec.user_ref, sumr_rec.crd_no);
		glwkRec.amount = 0.00;
		glwkRec.loc_amount = 0.00;
		strcpy (glwkRec.currency, sumr_rec.curr_code);
		strcpy (glwkRec.stat_flag, "2");
		strcpy (glwkRec.jnl_type, "1");

		GL_AddBatch ();

		cc = find_rec (sudt, &sudt_rec, NEXT, "r");
	}
}

/*
 *	Routine to create suhp, suht records. Creates header then rereads it 
 *  (for the hash) and	then creates the suht records.			
 *	Returns: 0 if ok, non-zero otherwise.
 */
void
MakeCheque (void)
{
	int	i;

	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, suhd_rec.cheq_no);
	cc = find_rec (suhp, &suhp_rec, COMPARISON, "u");
	if (cc)
	{
		strcpy (suhp_rec.payee_name, sumr_rec.crd_name);
		strcpy (suhp_rec.payee_acr, sumr_rec.acronym);
		strcpy (suhp_rec.narrative, suhd_rec.narrative);
		suhp_rec.date_payment = suhd_rec.date_payment;
		suhp_rec.date_post 	  = systemDate;
		suhp_rec.tot_amt_paid   = no_dec (suhd_rec.tot_amt_paid);
		suhp_rec.loc_amt_paid   = no_dec (suhd_rec.loc_amt_paid);
		suhp_rec.disc_taken     = no_dec (suhd_rec.disc_taken);
		suhp_rec.loc_disc_take = no_dec (suhd_rec.loc_disc_take);
		strcpy (suhp_rec.bank_id, suhd_rec.bank_id);
		strcpy (suhp_rec.pay_type, passHistoryType);
		strcpy (suhp_rec.stat_flag, "0");
		abc_unlock ("suhp");
		cc = abc_add (suhp, &suhp_rec);
		if (cc)
			file_err (cc, "suhp", "DBADD");

		strcpy (suhp_rec.co_no, comm_rec.co_no);
		strcpy (suhp_rec.cheq_no, suhd_rec.cheq_no);
		cc = find_rec (suhp, &suhp_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "suhp", "DBFIND");
	}
	else
	{
		suhp_rec.tot_amt_paid   += no_dec (suhd_rec.tot_amt_paid);
		suhp_rec.loc_amt_paid   += no_dec (suhd_rec.loc_amt_paid);
		suhp_rec.disc_taken     += no_dec (suhd_rec.disc_taken);
		suhp_rec.loc_disc_take += no_dec (suhd_rec.loc_disc_take);
		cc = abc_update (suhp, &suhp_rec);
		if (cc)
			file_err (cc, "suhp", "DBUPDATE");
	}
	for (i = 0; i < MAXBRANCHES; i++)
	{
		if (branchTotal [i] != 0.0)
		{
			suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
			sprintf (suht_rec.est_no, "%2d", i);
			suht_rec.est_amt_paid = no_dec (branchTotal [i]);
			suht_rec.est_loc_amt  = no_dec (branchLocal [i]);
			strcpy (suht_rec.stat_flag, "0");
			cc = abc_add (suht, &suht_rec);
			if (cc)
				file_err (cc, "suht", "DBADD");
		}
	}
}

void	
WriteCharges (
 char *	acct, 	
 long	hash, 
 double	fx_amount,
 double	loc_amount,
 double	exchangeRate,
 char *	curr_code)
{
	strcpy (glwkRec.user_ref, "bcctrl");
	strcpy (glwkRec.jnl_type, "5");
	batchTotal += no_dec (loc_amount);
	WriteGlwk (acct,
			   hash, 
			   fx_amount, 
			   loc_amount, 
			   exchangeRate, 
			   curr_code);
}

void
WriteBankAcc (
 char *	acct,
 long	hash,
 double	fx_amount,
 double	loc_amount,
 double	exchangeRate,
 char *	curr_code)
{
	strcpy (glwkRec.user_ref, "bkctrl");
	strcpy (glwkRec.jnl_type, "2");
	batchTotal += no_dec (loc_amount);
	WriteGlwk (acct,
			   hash, 
			   fx_amount, 
			   loc_amount, 
			   exchangeRate, 
			   curr_code);
}

void
WriteExchVar (
 char *	acct,
 long	hash,
 double	fx_amount,
 double	loc_amount,
 double	exchangeRate,
 char *	curr_code)
{
	strcpy (glwkRec.user_ref, "evctrl");
	strcpy (glwkRec.jnl_type, "2");
	WriteGlwk (acct,
			   hash, 
			   fx_amount, 
			   loc_amount, 
			   exchangeRate, 
			   curr_code);
}

void
WriteDiscount (
 char *	acct,
 long	hash,
 double	fx_amount,
 double	loc_amount,
 double	exchangeRate,
 char *	curr_code)
{
	strcpy (glwkRec.user_ref, "dtctrl");
	strcpy (glwkRec.jnl_type, "4");
	WriteGlwk (acct,
			   hash,
			   fx_amount,
			   loc_amount,
			   exchangeRate,
			   curr_code);
}

/*
 * Write GLWK Total Records.       
 */
void
WriteGlwk (
	char 	*acct,
	long	hash,
	double	fx_amount,
	double	loc_amount,
	double	exchangeRate,
	char	*curr_code)
{
	if (!loc_amount && !fx_amount)
		return;

	/*
	 * Add transaction for account .             
	 */
	sprintf (glwkRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL, acct);
	strcpy (glwkRec.co_no, 	comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	strcpy (glwkRec.acronym,"         ");
	sprintf (glwkRec.name, 	"%-30.30s", "                              ");
	strcpy (glwkRec.chq_inv_no, "               ");
	glwkRec.ci_amt 		= 0.0;
	glwkRec.o1_amt 		= 0.0;
	glwkRec.o2_amt 		= 0.0;
	glwkRec.o3_amt 		= 0.0;
	glwkRec.o4_amt 		= 0.0;
	glwkRec.hhgl_hash 	= hash;
	glwkRec.tran_date 		= suhd_rec.date_payment;
	glwkRec.post_date 	= systemDate;
	glwkRec.amount 		= no_dec (fx_amount);
	glwkRec.loc_amount	= no_dec (loc_amount);
	glwkRec.exch_rate	= exchangeRate;
	sprintf (glwkRec.currency, "%-3.3s", curr_code);
	strcpy (glwkRec.tran_type, " 9");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	sprintf (glwkRec.narrative, "DISP.%15.15s", suhd_rec.cheq_no);
	sprintf (glwkRec.alt_desc1, " ");
	sprintf (glwkRec.alt_desc2, " ");
	sprintf (glwkRec.alt_desc3, " ");
	sprintf (glwkRec.batch_no, " ");
	strcpy (glwkRec.stat_flag, "2");

	GL_AddBatch ();
}

/*
 * Update GLBC Batch Control Records. 
 */
void
UpdateWfbc (void)
{
	/*
     * Add general ledger batch control record.    
	 */
	strcpy (wfbc_rec.co_no, comm_rec.co_no);
	wfbc_rec.pid_no = pid;
	sprintf (wfbc_rec.work_file, "gl_work%05ld", pid);
	cc = find_rec (wfbc, &wfbc_rec, COMPARISON , "u");
    if (cc)
	{
		strcpy (wfbc_rec.system, "CR");
		wfbc_rec.date_create = systemDate;
		strcpy (wfbc_rec.stat_flag, "1");
		wfbc_rec.batch_tot_1 = no_dec (batchTotal);
		cc = abc_add (wfbc, &wfbc_rec);
		if (cc)
			file_err (cc, "wfbc", "DBADD");
	}
	else
	{
		wfbc_rec.batch_tot_1 += no_dec (batchTotal);
		cc = abc_update (wfbc, &wfbc_rec);
		if (cc)
			file_err (cc, "wfbc", "DBUPDATE");
	}
	abc_unlock (wfbc);
}

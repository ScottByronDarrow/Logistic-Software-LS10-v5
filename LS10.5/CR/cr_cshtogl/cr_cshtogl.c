/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_cshtogl.c,v 5.3 2002/06/25 03:17:03 scott Exp $
|  Program Name  : (cr_cshtogl.c)
|  Program Desc  : (Update General Ledger Work File From Cheque) 
|                 (File (sudr), Set Tran_flag to 2) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 18/03/96         |
|---------------------------------------------------------------------|
| $Log: cr_cshtogl.c,v $
| Revision 5.3  2002/06/25 03:17:03  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.2  2001/08/09 08:51:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:23  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_cshtogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_cshtogl/cr_cshtogl.c,v 5.3 2002/06/25 03:17:03 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>

FILE	*fsort;

	char	*fifteenSpaces	=	"               ";

	/*
	 * Suppliers/Supplier Cheque Work File Record. 
	 */
	struct SUDR_REC
	{
		char	co_no [3];
		char	br_no [3];
		char	chq_no [16];
		char	period [3];
		long	hhsp_hash;

		char	bank_id [6];
		char	bank_name [41];
		char	bk_ccode [4];
		char	bk_cdesc [41];
		double	bk_exch;		/* Origin to bank exchange 	*/
		double	bk_rec_amt;		/* Bank cheque amount 		*/
		double	bk_charge;		/* Bank cheque amount 		*/
		double	bk_l_exch;		/* Bank to local exch rate 	*/

		char	crd_no [7];		/* Supplier Number 			*/
		char	crd_name [41];	/* Supplier Name 			*/
		long	hhsu_hash;		/* Supplier Hash 			*/

		long	chq_date;		/* Cheque Date 				*/
		char	chq_type [2];	/* Cheque Type 				*/
		char	narrative [21];	/* Bank Draft Reference 	*/

		char	invoice [16];	/* Invoice being paid 		*/
		double	inv_exch;		/* Invoice Exchange Rate 	*/
		double	inv_amt;		/* Invoice Amount 			*/

		char	o_curr [4];		/* Origin to currency code. */
		double	o_exch;			/* Origin to local exchange */
		Money	o_disc;		
		Money	o_total_amt;	
		Money	o_amt_pay;
		char	gl_disc [MAXLEVEL + 1];		/* GL Discount Account */
		Money	l_disc;			
		Money	l_total_amt;	
		Money	l_amt_pay;
	} sudr2_rec, sudr_rec, sudr3_rec;

	#include	"schema"

	struct commRecord	comm_rec;
	struct crbkRecord	crbk_rec;
	struct sudtRecord	sudt_rec;
	struct suinRecord	suin_rec;
	struct sumrRecord	sumr_rec;

	/*
	 * Special fields and flags. 
	 */
   	int		pidNumber			= 0,
			sudrNumber			= 0,
			envGSTApplies		= 0,
			sortOpen 			= FALSE,
			supplierSeqNumber	= 0,
			envSeparateGST 		= FALSE,
			firstCheque			= FALSE,
			envMultiCurrency 	= FALSE,
			printerNumber		= 1,
			BankCharged 		= TRUE;

   	double 	discountFgnAmount 	= 0.00,
			discountLocAmount 	= 0.00,
   			bankFgnAmount 	  	= 0.00,
   			bankLocAmount 	  	= 0.00,
   			exchLocVariance 	= 0.00,
   			exchFgnVariance 	= 0.00,
   			chequeLocAmount		= 0.00,
   			chequeFgnAmount		= 0.00,
   			previousAmount		= 0.00;

	float	GstRate				= 0.00;

   	char 	accountNumber [MAXLEVEL + 1],
   			previousCheque [16],
			envGSTCode [4],
			currentSupplierNo [7],
			currentBank [6];

	char	*sortRecordOffset [256];

	char	*data = "data";

#include	<CashBook.h>

/*
 * Local function prototypes 
 */
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	shutdown_prog	 (void);
void	Update			 (void);
int		WriteGlwk		 (struct SUDR_REC *);
int		ProcessChqTrans	 (void);
int		ProcessSortedGlwk (void);
int		CheckBankDetails (void);
char 	*_sort_read		 (FILE *);

int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	/*
	 * Check if gst applies. 
	 */
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envGSTApplies = 0;
	else
		envGSTApplies = (*sptr == 'Y' || *sptr == 'y');

	if (envGSTApplies)
		sprintf (envGSTCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envGSTCode, "%-3.3s", "Tax");
		

	sptr = chk_env ("CR_GST_POST");
	if (sptr)
		envSeparateGST = (*sptr == 'Y') ? TRUE : FALSE;

	/*
	 * Multi-currency debtors. 
	 */
	sptr = chk_env ("CR_MCURR");
	if (sptr)
		envMultiCurrency = (*sptr == 'Y' || *sptr == 'y') ? TRUE : FALSE;
	else
		envMultiCurrency = FALSE;

	if (argc < 3)
	{
		print_at (0, 0, ML (mlCrMess015), argv [0]);
		return (EXIT_FAILURE);
	}
	pidNumber   = atoi (argv [1]);
	printerNumber  = atoi (argv [2]);

   	strcpy (previousCheque, fifteenSpaces);
   	strcpy (currentBank, "     ");

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char*) &comm_rec);
	GstRate = comm_rec.gst_rate;
	dsp_screen ("Creating Supplier Dispursements Journals.",
				comm_rec.co_no,comm_rec.co_name);

	if (ProcessChqTrans ())
	{
		ProcessSortedGlwk ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	else
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
}

/*
 * Open Database Files. 
 */
void
OpenDB (
 void)
{
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/cr_csh%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);
	cc = RF_OPEN (filename, sizeof (sudr_rec), "r", &sudrNumber);
	if (cc) 
		sys_err ("Error in sudr During (WKOPEN)",cc, PNAME);

	abc_dbopen (data);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no3");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenCashBook ();
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	RF_CLOSE (sudrNumber);
	abc_fclose (sumr);
	abc_fclose (crbk);
	abc_fclose (suin);
	abc_fclose (sudt);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	CloseCashBook ();
	abc_dbclose (data);
}

/*
 * Process cheque transactions. 
 */
int
ProcessChqTrans (void)
{
	int			firstTime = TRUE;
	static int 	processed = FALSE;
	
	firstCheque = TRUE;
	supplierSeqNumber = 0;
	
	cc = RF_READ (sudrNumber, (char *) &sudr_rec);
	
	while (!cc)
	{
		if (firstTime)
		{
			sprintf (currentSupplierNo, "%-6.6s", sudr_rec.crd_no);
			firstTime = FALSE;
		}

		/*
		 * Read sumr record 
		 */
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		sprintf (sumr_rec.crd_no, "%-6.6s", sudr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (sumr_rec.tax_code, "C");

		/*
		 * If debtor still does not have a control   
		 * account use the default off GL_GLI.       
		 */
		strcpy (glmrRec.co_no,  sudr_rec.co_no);
		strcpy (glmrRec.acc_no, sumr_rec.gl_ctrl_acct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			GL_GLI 
			(
				sudr_rec.co_no,		/*	Company Number 	*/
				sudr_rec.br_no,		/*	Branch No.		*/
				"  ",				/*	Warehouse No.	*/
				"ACCT PAY  ",		/*	Interface Code. */
				"   ",				/*	Customer Type.	*/
				" "					/*	Category.		*/
			);
			strcpy (sumr_rec.gl_ctrl_acct, glmrRec.acc_no);
		}

		Update ();

		/*
		 * Store sudr_rec as sudr2_rec. 
		 */
		memcpy ((char *)&sudr2_rec, (char *)&sudr_rec,sizeof (struct SUDR_REC));

		cc = RF_READ (sudrNumber, (char *) &sudr_rec);
		
		processed = TRUE;
	}

	/*
	 * Debit the bank account for the previous cheque.     
	 */
	if (!firstCheque)
	{
		sprintf (err_str, "Cash Disb. %15.15s / Ref. %6.6s",
							sudr2_rec.chq_no, sudr2_rec.crd_no);
			
		/*
		 * Write entry to cash book system 
		 */
		WriteCashBook
		(								/*--------------------------*/
			sudr2_rec.co_no,			/* Company Number			*/
			sudr2_rec.br_no,			/* Branch Number.			*/
			crbk_rec.bank_id,			/* Bank Id.					*/
			sudr2_rec.chq_date,			/* Transaction Date			*/
			err_str,					/* Transaction Narrative.	*/
			"D",						/* Transaction Type.		*/
			sudr2_rec.l_total_amt*-1,	/* Amount posted to bank.	*/
			"0",						/* Status flag.				*/
			TodaysDate ()				/* System/period date.		*/
		);								/*--------------------------*/
		
		sprintf (accountNumber, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
	 
		strcpy (glwkRec.jnl_type, "2");
		strcpy (sudr2_rec.invoice, fifteenSpaces);
		strcpy (sudr2_rec.co_no, comm_rec.co_no);
		sudr2_rec.l_amt_pay = sudr2_rec.l_total_amt;
		sudr2_rec.o_amt_pay = sudr2_rec.o_total_amt;
		sudr2_rec.l_amt_pay -= sudr2_rec.l_disc;
		sudr2_rec.o_amt_pay -= sudr2_rec.o_disc;
		sudr2_rec.l_amt_pay += bankLocAmount;
		sudr2_rec.o_amt_pay += bankFgnAmount;
	
		WriteGlwk (&sudr2_rec);
	}
	return processed;
}
/*
 * Transaction records  
 */
void
Update (void)
{
	int		per,
			bank_p;
	float	gst_div;
	double	disc_gst;
	double	lcl_inv_amt;

	/*
	 * Create glwk records 
	 */
	dsp_process ("Cheque : ", sudr_rec.chq_no);

	bank_p = atoi (sudr_rec.period) - 1;

	per = atoi (sudr_rec.period);

	/*
	 * Change in cheque  number or amount. 
	 */

	if (strcmp (sudr_rec.chq_no, previousCheque) || 
	     previousAmount != sudr_rec.l_total_amt)
	{
		BankCharged = FALSE;
		/*
		 * Debit the bank account for the previous cheque.
		 */
		if (!firstCheque)
		{
			sprintf (err_str, "Cash Disb. %15.15s / Ref. %6.6s",
								sudr2_rec.chq_no, sudr2_rec.crd_no);
				
			/*
			 * Write entry to cash book system 
			 */
			WriteCashBook
			 (								/*--------------------------*/
				sudr2_rec.co_no,			/* Company Number			*/
				sudr2_rec.br_no,			/* Branch Number.			*/
				crbk_rec.bank_id,			/* Bank Id.					*/
				sudr2_rec.chq_date,			/* Transaction Date			*/
				err_str,					/* Transaction Narrative.	*/
				"D",						/* Transaction Type.		*/
				sudr2_rec.l_total_amt*-1,	/* Amount posted to bank.	*/
				"0",						/* Status flag.				*/
				TodaysDate ()				/* System/period date.		*/
			);								/*--------------------------*/
			
			sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
			
			strcpy (glwkRec.jnl_type, "2");
			strcpy (sudr2_rec.invoice, fifteenSpaces);
			strcpy (sudr2_rec.co_no, comm_rec.co_no);
			sudr2_rec.l_amt_pay = sudr2_rec.l_total_amt;
			sudr2_rec.o_amt_pay = sudr2_rec.o_total_amt;
			sudr2_rec.l_amt_pay -= sudr2_rec.l_disc;
			sudr2_rec.o_amt_pay -= sudr2_rec.o_disc;
			sudr2_rec.l_amt_pay += bankLocAmount;
			sudr2_rec.o_amt_pay += bankFgnAmount;
			WriteGlwk (&sudr2_rec);
			bankLocAmount = 0.00;
			bankFgnAmount = 0.00;
		}

		firstCheque = FALSE;

		/*
		 * Get bank details if bank has changed.   
		 */
		if (!CheckBankDetails ())
			return;

		previousAmount 	  = sudr_rec.l_total_amt;
		discountFgnAmount = sudr_rec.o_disc;
		discountLocAmount = sudr_rec.l_disc;

		/*
		 * Calculate bank charges (in local currency). 
		 */
		bankFgnAmount = no_dec (sudr_rec.bk_charge);
		bankLocAmount = no_dec (sudr_rec.bk_charge);
		if (sudr_rec.l_total_amt != 0.00)
		{
			bankLocAmount += no_dec (crbk_rec.clear_fee);
			bankFgnAmount += no_dec (crbk_rec.clear_fee);
		}
		if (sudr_rec.bk_exch != 0.00)
			bankFgnAmount *= sudr_rec.bk_exch;

		bankLocAmount = no_dec (bankLocAmount);
		bankFgnAmount = no_dec (bankFgnAmount);

		/*
		 * Credit the debtors account. 
		 */

		strcpy (previousCheque, sudr_rec.chq_no);

		sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,sumr_rec.gl_ctrl_acct);
		chequeLocAmount = sudr_rec.l_amt_pay;
		chequeFgnAmount = sudr_rec.o_amt_pay;
		sudr_rec.l_amt_pay = sudr_rec.l_total_amt;
		sudr_rec.o_amt_pay = sudr_rec.o_total_amt;
		strcpy (glwkRec.jnl_type, "1");
		WriteGlwk (&sudr_rec);

		memcpy ( (char *)&sudr3_rec, (char *)&sudr_rec, sizeof (struct SUDR_REC));

		sudr_rec.o_amt_pay = chequeFgnAmount;
		sudr_rec.l_amt_pay = chequeLocAmount;

		/*
		 *  Sum cheque details into branchNolishments And create glwk records 
		 */
		sudt_rec.hhsp_hash = sudr_rec.hhsp_hash;
		cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
		while (!cc && sudr_rec.hhsp_hash == sudt_rec.hhsp_hash)
		{
			suin_rec.hhsi_hash = sudt_rec.hhsi_hash;
			cc = find_rec (suin, &suin_rec, EQUAL, "r");
			if (cc)
			{
				strcpy (suin_rec.est, "00");
				strcpy (suin_rec.inv_no, "???????????????");
			}
			strcpy (sudr3_rec.br_no, suin_rec.est);
			strcpy (sudr3_rec.invoice, suin_rec.inv_no);
			sudr3_rec.l_total_amt	= no_dec (sudt_rec.loc_paid_inv);
			sudr3_rec.o_total_amt	= no_dec (sudt_rec.amt_paid_inv);
			strcpy (sudr3_rec.narrative, "Suppliers Payments  ");
			sudr3_rec.o_exch 	= sudt_rec.exch_rate;
			sudr3_rec.l_amt_pay	= 	0.00;
			sudr3_rec.o_amt_pay	=	0.00;
			sprintf (accountNumber, "%*.*s", MAXLEVEL,MAXLEVEL,"                ");

			WriteGlwk (&sudr3_rec);

			cc = find_rec (sudt, &sudt_rec, NEXT, "r");
		}
	}

	/*
	 * Post discount. 
	 */
  	if (discountFgnAmount != 0.00 || discountLocAmount != 0.00)
	{
		if (envGSTApplies && envSeparateGST && sumr_rec.tax_code [0] == 'C')
		{
			/*
			 * Calculate amount of GST in discount 
			 */
			gst_div = (float) ( (100.00 + GstRate) / GstRate);
			disc_gst = twodec (discountFgnAmount / (double)gst_div);
			discountFgnAmount -= disc_gst;
			disc_gst = twodec (discountLocAmount / (double)gst_div);
			discountLocAmount -= disc_gst;

			/*
			 * Debit the GST charged account 
			 */
			GL_GLI 
			(
				sudr_rec.co_no,		/*	Company Number 	*/
				sudr_rec.br_no,		/*	Branch No.		*/
				"  ",				/*	Warehouse No.	*/
				"G.S.T PAID",		/*	Interface Code. */
				"   ", 				/*	Customer Type.	*/
				" "					/*	Category.		*/
			);
			strcpy (accountNumber,glmrRec.acc_no);

			strcpy (glwkRec.jnl_type, "4");
			sprintf (sudr_rec.period, "%02d", bank_p + 1);
			sudr_rec.l_amt_pay = no_dec (disc_gst);
			WriteGlwk (&sudr_rec);
		}

		/*
		 * Debit the discount account 
		 */
		sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,sudr_rec.gl_disc);
		strcpy (glwkRec.jnl_type, "2");
		sprintf (sudr_rec.period, "%02d", bank_p + 1);
		sudr_rec.l_amt_pay = no_dec (discountLocAmount);
		sudr_rec.o_amt_pay = no_dec (discountFgnAmount);
		WriteGlwk (&sudr_rec);

		sprintf (sudr_rec.period, "%02d", per);
		discountFgnAmount = 0;
		discountLocAmount = 0;
		sudr_rec.l_amt_pay = chequeLocAmount;
		sudr_rec.o_amt_pay = chequeFgnAmount;
	}

	/*
	 * Post Bank Charges. 
	 */
  	if (bankFgnAmount != 0.00 || bankLocAmount != 0.00)
	{
		if (!BankCharged)
		{
			/*
			 * Debit the bank charges account 
			 */
			sprintf (accountNumber, "%-*.*s", 
									MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_chg);
			strcpy (glwkRec.jnl_type, "5");
			sprintf (sudr_rec.period, "%02d", bank_p + 1);
			sudr_rec.l_amt_pay = no_dec (bankLocAmount);
			sudr_rec.o_amt_pay = no_dec (bankFgnAmount);
			WriteGlwk (&sudr_rec);

			sudr_rec.l_amt_pay = chequeLocAmount;
			sudr_rec.o_amt_pay = chequeFgnAmount;
		}
		BankCharged = TRUE;
	}

	/*
	 * Calculate and post Exchange Variation. 
	 */
	lcl_inv_amt = sudr_rec.l_amt_pay;


	if (sudr_rec.inv_exch != 0.00)
	{
		if (sudr_rec.inv_exch == 1.00)
			lcl_inv_amt /= sudr_rec.inv_exch;
		else
			lcl_inv_amt = sudr_rec.o_amt_pay / sudr_rec.inv_exch;
	}
	
	
	exchLocVariance = no_dec (sudr_rec.l_amt_pay) - no_dec (lcl_inv_amt);
	exchFgnVariance = no_dec (sudr_rec.o_amt_pay) 
					- no_dec (sudr_rec.o_amt_pay);

  	if (exchLocVariance != 0.00 && envMultiCurrency)
	{
		/*
		 * Write to exchange variation account 
		 */
		sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_exch_var);
		if (exchLocVariance > 0.00)
		{
			strcpy (glwkRec.jnl_type, "5");
			sudr_rec.l_amt_pay = exchLocVariance;
			sudr_rec.o_amt_pay = exchFgnVariance;
		}
		else
		{
			sudr_rec.l_amt_pay = exchLocVariance * -1.00;
			sudr_rec.o_amt_pay = exchFgnVariance * -1.00;
			strcpy (glwkRec.jnl_type, "6");
		}
		sprintf (sudr_rec.period, "%02d", bank_p + 1);
		WriteGlwk (&sudr_rec);

		/*
		 * Write to debtor account 
		 */
		sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,sumr_rec.gl_ctrl_acct);
		if (exchLocVariance > 0.00)
		{
			sudr_rec.l_amt_pay = exchLocVariance;
			sudr_rec.o_amt_pay = exchFgnVariance;
			strcpy (glwkRec.jnl_type,"6");
		}
		else
		{
			sudr_rec.l_amt_pay = exchLocVariance * -1.00;
			sudr_rec.o_amt_pay = exchFgnVariance * -1.00;
			strcpy (glwkRec.jnl_type,"5");
		}
		sprintf (sudr_rec.period, "%02d", bank_p + 1);
		WriteGlwk (&sudr_rec);

		exchLocVariance = 0.00;
		exchFgnVariance = 0.00;
		sudr_rec.l_amt_pay = chequeLocAmount;
		sudr_rec.o_amt_pay = chequeFgnAmount;
	}
}

/*
 * Write data required for glwk to a sort file. 
 */
int
WriteGlwk (
 struct SUDR_REC *	cash_rec)
{
	char	data_str [512];
	char	crd_srt [6];

	if (!sortOpen)
	{
		fsort = sort_open ("srt_glwk");
		sortOpen = TRUE;
	}

	glmrRec.hhmr_hash = 0;
	if (strncmp (accountNumber, "                ", MAXLEVEL))
	{
		strcpy (glmrRec.co_no,  comm_rec.co_no);
		sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,accountNumber);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			GL_GLI 
			(
				sudr_rec.co_no,		/*	Company Number 	*/
				sudr_rec.br_no,		/*	Branch No.		*/
				"  ",				/*	Warehouse No.	*/
				"SUSPENSE  ",		/*	Interface Code. */
				" ",				/*	Customer Type.	*/
				" "					/*	Category.		*/
			);
			strcpy (accountNumber, glmrRec.acc_no);
		}
	}
		
	/*
	 * The following code is to keep each supplier header record with 
	 * its detail records	
	 */
	if (!strncmp (cash_rec->crd_no, "      ", 6))
	{
		sprintf (cash_rec->crd_no, "%-6.6s", currentSupplierNo);
		sprintf (crd_srt, "%04d%-1.1s", supplierSeqNumber++, "N");
	}
	else
	{
		sprintf (currentSupplierNo, "%-6.6s", cash_rec->crd_no);
		sprintf (crd_srt, "%04d%-1.1s", supplierSeqNumber++, " ");
	}

	/*
	 * Add transaction to sort file 
	 */
	sprintf (data_str,
		"%s%c%s%c%s%c%s%c%s%c%f%c%f%c%s%c%s%c%f%c%f%c%s%c%s%c%d%c%s%c%ld%c%ld%c%s%c%4.8f%c%s\n",
		cash_rec->co_no,       	1,	/* sortRecordOffset = 0  */
		cash_rec->br_no,       	1,	/* sortRecordOffset = 1  */
		cash_rec->crd_no,      	1,	/* sortRecordOffset = 2  */
		crd_srt,               	1,	/* sortRecordOffset = 3  */
		cash_rec->chq_no,      	1,	/* sortRecordOffset = 4  */
		cash_rec->l_total_amt, 	1,	/* sortRecordOffset = 5  */
		cash_rec->o_total_amt, 	1,	/* sortRecordOffset = 6  */
		cash_rec->invoice,     	1,	/* sortRecordOffset = 7  */
		cash_rec->o_curr,      	1,	/* sortRecordOffset = 8  */
		cash_rec->l_amt_pay,   	1,	/* sortRecordOffset = 9  */
		cash_rec->o_amt_pay,   	1,	/* sortRecordOffset = 10 */
		accountNumber,			1,	/* sortRecordOffset = 11 */
		cash_rec->period,      	1,	/* sortRecordOffset = 12 */
		comm_rec.term,          1,	/* sortRecordOffset = 13 */
		glwkRec.jnl_type,     	1,	/* sortRecordOffset = 14 */
		glmrRec.hhmr_hash,  	1,	/* sortRecordOffset = 15 */
		cash_rec->chq_date,    	1,	/* sortRecordOffset = 16 */
		cash_rec->narrative,	1,	/* sortRecordOffset = 17 */
		cash_rec->o_exch,	  	1,	/* sortRecordOffset = 18 */
	    " ");   					/* sortRecordOffset = 19 */

	sort_save (fsort, data_str);

	return (EXIT_SUCCESS);
}

int
ProcessSortedGlwk (void)
{
	char	*sptr;

	fsort = sort_sort (fsort, "srt_glwk");
	
	sptr = _sort_read (fsort);
	while (sptr)
	{
		/*
		 * Add line to glwk for account if required  
		 */
		strcpy (glwkRec.tran_type,  " 9");
		strcpy (glwkRec.stat_flag,  "2");

		sprintf (glwkRec.sys_ref,   "%010ld",   atol (sortRecordOffset [13]));
		sprintf (glwkRec.period_no, "%-2.2s",   sortRecordOffset [12]);
		sprintf (glwkRec.acc_no,    "%-16.16s", sortRecordOffset [11]);
		sprintf (glwkRec.co_no,     "%2.2s",    sortRecordOffset [0]);
		sprintf (glwkRec.est_no,    "%2.2s",    sortRecordOffset [1]);
		sprintf (glwkRec.name,      "%-30.15s", sortRecordOffset [7]);
		sprintf (glwkRec.narrative, "%-20.20s", sortRecordOffset [17]);
		sprintf (glwkRec.alt_desc1, "%-20.20s",  sortRecordOffset [7]);
		sprintf (glwkRec.alt_desc2, "%20.20s", " ");
		sprintf (glwkRec.alt_desc3, "%20.20s", " ");
		sprintf (glwkRec.batch_no,  "%10.10s", " ");
		if (!strncmp (sortRecordOffset [3] + 4, "N", 1))
			sprintf (glwkRec.acronym, "%-9.6s", " ");
		else
			sprintf (glwkRec.acronym, "%-9.6s", sortRecordOffset [2]);

		sprintf (glwkRec.user_ref,   "%-15.15s", sortRecordOffset [4]);
		sprintf (glwkRec.chq_inv_no, "%-15.15s", sortRecordOffset [4]);
		sprintf (glwkRec.jnl_type,   "%-1.1s",   sortRecordOffset [14]);

		glwkRec.hhgl_hash 		= atol (sortRecordOffset [15]);
		glwkRec.tran_date     	= atol (sortRecordOffset [16]);
		glwkRec.ci_amt    		= atof (sortRecordOffset [5]);
		glwkRec.o1_amt    		= atof (sortRecordOffset [6]);
		glwkRec.amount    		= atof (sortRecordOffset [10]);
		glwkRec.loc_amount    	= atof (sortRecordOffset [9]);
		glwkRec.exch_rate    	= atof (sortRecordOffset [18]);
		sprintf (glwkRec.currency, "%-3.3s",sortRecordOffset [8]);

		glwkRec.post_date = TodaysDate ();
		if (glwkRec.post_date < 0)
			glwkRec.post_date = glwkRec.tran_date;
			
		GL_AddBatch ();

		sptr = _sort_read (fsort);
	}

	sort_delete (fsort, "srt_glwk");
	return (EXIT_SUCCESS);
}

/*
 * Get bank details if bank has changed.   
 */
int
CheckBankDetails (void)
{
	/*
	 * Bank hasn't changed. 
	 */
	if (!strcmp (currentBank, sudr_rec.bank_id))
		return (TRUE);

	strcpy (crbk_rec.co_no, sudr_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", sudr_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	sprintf (currentBank, "%-5.5s", crbk_rec.bank_id);

	return (TRUE);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Save offsets for each numerical field.      
 */
char *
_sort_read (
 FILE *	srt_fil)
{
	char *	sptr;
	char *	tptr;
	int		fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	sortRecordOffset [0] = sptr;

	tptr = sptr;
	while (fld_no < 19)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		sortRecordOffset [fld_no++] = sptr + (tptr - sptr);
	}
	return (sptr);
}


/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crctogl.c,v 5.4 2001/10/30 02:07:40 cha Exp $
|  Program Name  : (db_crctogl.c)
|  Program Desc  : (Update General Ledger Work File From Receipts)
|                 (File (cudr), Set Tran_flag to 2)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crctogl.c,v $
| Revision 5.4  2001/10/30 02:07:40  cha
| Fix   Issue #00655 - GLJL3-Batch Journal Inquiry
| Changes done by Scott
|
| Revision 5.3  2001/08/09 08:23:41  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:55  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:09  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crctogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crctogl/db_crctogl.c,v 5.4 2001/10/30 02:07:40 cha Exp $";

#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<twodec.h>

FILE	*fsort;

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int		processID			= 0,
			cudrFileNumber		= 0,	
			envVarGst			= 0,
			envVarDbCo			= 0,
			envVarDbFind		= 0,
			envVarDbGstPost 	= FALSE,
			envVarDbMcurr 		= FALSE,
			envVarGlByClass 	= TRUE,
			BankCharged 		= TRUE,
			firstTimeFlag 		= TRUE,
			sortOpen 			= FALSE,
			debtorsSequenceNo	= 0,
			firstReceiptFlag	= TRUE,
			printerNumber		= 1,
			bankPeriod			= 0;

	long	lsystemDate;

   	double 	disc_o_amt = 0.00,
			disc_l_amt = 0.00,
   			bank_o_chg = 0.00,
   			bank_l_chg = 0.00,
   			exch_l_var = 0.00,
   			exch_o_var = 0.00,
   			rpt_l_amt  = 0.00,
   			rpt_o_amt  = 0.00,
   			prev_amt   = 0.00;

   	char 	accountNumber [MAXLEVEL + 1],
   			previousReceipt [9],
   			previousUserRef [sizeof glwkRec.user_ref],
			term_no			[sizeof glwkRec.sys_ref],
			envVarGstTaxName [4],
   			systemDate [11],
			*srt_offset [256],
			currendDbtNo [7],
			currentBank [6],
			printReport [2],
			batchNumber [sizeof glwkRec.sys_ref];

	char	*sixteenSpaces	=	"                ";
	float	envVarGstInclusive	=	0.00;

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct bldtRecord	bldt_rec;
struct cumrRecord	cumr_rec;

	/*==============================================
	| Creditors/Customer Receipts Work File Record. |
	==============================================*/
	struct CUDR_REC
	{
		char	co_no [3];
		char	br_no [3];
		char	rec_no [9];
		char	reversal [2];
		char	period [3];
		long	hhcp_hash;
		char	bank_id [6];
		char	bank_name [41];
		char	bk_ccode [4];
		char	bk_cdesc [41];
		double	bk_exch;		/* Origin to bank exchange */
		double	bk_rec_amt;		/* Bank receipt amount */
		double	bk_charge;		/* Bank charges */
		double	bk_clear_fee;	/* Bank clearance fee */
		double	bk_l_exch;		/* Bank to local exch rate */
		char	dbt_no [7];		/* Customer Number */
		char	dbt_name [41];	/* Customer Name */
		long	hhcu_hash;		/* Customer Hash */
		long	rec_date;		/* Receipt Date */
		char	rec_type [2];	/* Receipt Type */
		long	due_date;		/* Due Date For Bank Drafts */
		char	narrative [21];	/* Bank Draft Reference */
		char	invoice [9];	/* Invoice being paid */
		double	inv_exch;		/* Invoice Exchange Rate */
		double	inv_amt;		/* Invoice Amount */
		char	o_curr [4];		/* Invocice currency  */
		double	o_exch;			/* Origin to local exchange */
		Money	o_disc;		
		Money	o_total_amt;	
		Money	o_amt_pay;	
		char	gl_disc [MAXLEVEL + 1];	/* GL discount account */
		Money	l_disc;		
		Money	l_total_amt;
		Money	l_amt_pay;		
	} cudr2_rec, cudr_rec;

	char	*data = "data";


#include	<CashBook.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	UpdateData 			 (void);
void 	WriteGlwk 			 (struct CUDR_REC *);
void 	ProcessSortGlwk 	 (void);
int 	CheckBankDetails 	 (void);
int 	upd_lodge 			 (char *);
void 	shutdown_prog 		 (void);
char 	*_SortRead 			 (FILE *);
void	PostCashBookEntrys 	 (void);
void	SaveBankRecDetails 	 (char	*, double);
int		BankIdSort		 	 (const	void *,	const void *);

/*---------------------------------------------------------------
|	Structure for dynamic array,  for the Bank lines for qsort	|
---------------------------------------------------------------*/
struct BankRecord
{
	char	bankId [7];
	double	bankAmount;
}	*bank;
	DArray bank_d;
	int	bankCnt = 0;

char	cashBookDesc [41];
int
main (
 int                argc,
 char*              argv [])
{
	int		nodata = TRUE;
	int		firstTimeFlag = TRUE;
	char	*sptr;

	if (argc < 4)
	{
		print_at (0,0, "Usage: %s <PID> <LPNO> print 2nd Rep [Y/N]", argv [0]);
        return (EXIT_FAILURE);
	}

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVarGst = 0;
	else
		envVarGst = (*sptr == 'Y' || *sptr == 'y');

	if (envVarGst)
		sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVarGstTaxName, "%-3.3s", "Tax");
		
	sptr = chk_env ("GL_BYCLASS");
	envVarGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_GST_POST");
	if (sptr)
		envVarDbGstPost = (*sptr == 'Y') ? TRUE : FALSE;

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*----------------------------------
	| Company or Branch owned debtors. |
	----------------------------------*/
	envVarDbCo = atoi (get_env ("DB_CO"));

	/*--------------------------
	| Invoice from anywhere ?. |
	--------------------------*/
	envVarDbFind 	= atoi (get_env ("DB_FIND"));
	processID   	= atoi (argv [1]);
	printerNumber  	= atoi (argv [2]);
	printReport [0] = argv [3][0];

   	strcpy (previousReceipt, "        ");
   	strcpy (currentBank, "     ");

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	sprintf (cashBookDesc, "Cash receipt posting for %s", systemDate);

	OpenDB ();

	if ( (sptr = chk_env ("GST_INCLUSIVE")) && atof (sptr) != 0)
		envVarGstInclusive = (float) atof (sptr);
	else
		envVarGstInclusive = comm_rec.gst_rate;

	init_scr ();

    print_mess (ML ("Creating Customers Receipts Journals."));

	/*-----------------------------
	| Allocate the initial array. |
	-----------------------------*/
	ArrAlloc (&bank_d, &bank, sizeof (struct BankRecord), 100);
	bankCnt = 0;

	/*-------------------------------- 
	| Process receipts transactions. |
	--------------------------------*/
	firstReceiptFlag = TRUE;
	debtorsSequenceNo = 0;
	cc = RF_READ (cudrFileNumber, (char *) &cudr_rec);
	while (!cc)
	{
		if (firstTimeFlag)
		{
			sprintf (currendDbtNo, "%-6.6s", cudr_rec.dbt_no);
			firstTimeFlag = FALSE;
		}

		/*------------------
		| Read cumr record |
		------------------*/
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, (envVarDbCo) ? cudr_rec.br_no : " 0");
		sprintf (cumr_rec.dbt_no, "%-6.6s", cudr_rec.dbt_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (cumr_rec.tax_code, "C");

		/*-------------------------------------------
		| If debtor still does not have a control   |
		| account use the default off GL_GLI.       |
		-------------------------------------------*/
		strcpy (glmrRec.co_no,  cudr_rec.co_no);
		strcpy (glmrRec.acc_no, cumr_rec.gl_ctrl_acct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			GL_GLI 
			(
				cudr_rec.co_no,		/*	Company Number 	*/
				cudr_rec.br_no,		/*	Branch No.		*/
				"  ",				/*	Warehouse No.	*/
				"ACCT REC  ",		/*	Interface Code. */
				 (envVarGlByClass) 	? cumr_rec.class_type
						 			: cumr_rec.sman_code,
				" "					/* Category			*/
			);

            strcpy (cumr_rec.gl_ctrl_acct,	glmrRec.acc_no);
		}

		UpdateData ();

		/*------------------------------
		| Store cudr_rec as cudr2_rec. |
		------------------------------*/
		memcpy ( (char *) &cudr2_rec, (char *) &cudr_rec,sizeof (struct CUDR_REC));

		cc = RF_READ (cudrFileNumber, (char *) &cudr_rec);
		nodata = FALSE;
	}

	/*----------------------------
	| Debit the bank account for |
	| the previous receipt.      |
	----------------------------*/
	if (!firstReceiptFlag)
	{
		if (cudr2_rec.rec_type [0] == 'A' ||
		 	cudr2_rec.rec_type [0] == 'D' || 
			cudr2_rec.due_date <= lsystemDate)
		{
			sprintf (accountNumber, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);

			strcpy (glmrRec.co_no,  comm_rec.co_no);
			strcpy (glmrRec.acc_no, accountNumber);
			cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
			if (cc)
				sprintf (accountNumber, "NOT VALID");

			/*--------------------------
			| Update lodgement record. |
			--------------------------*/
			upd_lodge ("Y");

			/*
				Add to save routine unique bank id and amount
			*/
			SaveBankRecDetails 
			 (
				cudr2_rec.bank_id,
				cudr2_rec.o_total_amt
			);

		}
		/*--------------------------------	
		| Data from the previous receipt |
		| is stored in cudr2_rec.        |
		--------------------------------*/
		else 
		{
			if (cudr2_rec.rec_type [0] == 'B')
				strcpy (accountNumber, crbk_rec.gl_bill_rec);
			else
				strcpy (accountNumber, crbk_rec.gl_fwd_rec);
			
			strcpy (glmrRec.co_no,  comm_rec.co_no);
			strcpy (glmrRec.acc_no, accountNumber);
			cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
			if (cc)
				sprintf (accountNumber, "NOT VALID");

			/*--------------------------
			| Update lodgement record. |
			--------------------------*/
			upd_lodge ("N");
		}
		if (cudr2_rec.rec_type [0] == 'I' ||
			cudr2_rec.reversal [0] == 'Y')
			strcpy (glwkRec.jnl_type, "2");
		else
			strcpy (glwkRec.jnl_type, "1");
		strcpy (cudr2_rec.invoice, "        ");
		strcpy (cudr2_rec.dbt_no, "      ");
		strcpy (cudr2_rec.rec_no, "        ");
		strcpy (cudr2_rec.co_no, comm_rec.co_no);
		cudr2_rec.l_amt_pay = cudr2_rec.l_total_amt;
		cudr2_rec.o_amt_pay = cudr2_rec.o_total_amt;
		cudr2_rec.l_amt_pay -= cudr2_rec.l_disc;
		cudr2_rec.o_amt_pay -= cudr2_rec.o_disc;
		cudr2_rec.l_amt_pay -= bank_l_chg;
		cudr2_rec.o_amt_pay -= bank_o_chg;
	
		WriteGlwk (&cudr2_rec);
	}

	if (nodata == FALSE)
	{
		ProcessSortGlwk ();
		PostCashBookEntrys ();
		shutdown_prog ();
	}
	else
    {
		shutdown_prog ();
    }
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/db_crc%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);
	cc = RF_OPEN (filename, sizeof (cudr_rec), "r", &cudrFileNumber);
	if (cc) 
		sys_err ("Error in cudr During (WKOPEN)",cc, PNAME);

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (term_no, "%010ld", (long) comm_rec.term);
	OpenGlmr ();

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (bldt, bldt_list, BLDT_NO_FIELDS, "bldt_hhcp_hash");

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenCashBook ();
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	char	cmd [35];

	cc = RF_CLOSE (cudrFileNumber);
	if (cc) 
		sys_err ("Error in cudr During (WKCLOSE)", cc, PNAME);

	abc_fclose (cumr);
	abc_fclose (bldt);
	abc_fclose (crbk);
	GL_CloseBatch (printerNumber);
	sprintf (cmd, "db_crcrep %d \"%d\" \"%-1.1s\" \"%-10.10s\"", printerNumber, processID, printReport, batchNumber);
	sys_exec (cmd);

	GL_Close ();
	CloseCashBook ();
	abc_dbclose (data);
}

/*========================
| Transaction records  . |
========================*/
void
UpdateData (void)
{
	int		per;
	float	gst_div;
	double	disc_gst;
	double	lcl_inv_amt;

	/*-----------------------
	| Create glwk records . |
	-----------------------*/
    DateToDMY (comm_rec.dbt_date, NULL, &bankPeriod, NULL);  
    per	=	bankPeriod;

	/*-------------------------------------
	| Change in receipt number or amount. |
	-------------------------------------*/
	if (strcmp (cudr_rec.rec_no, previousReceipt) || 
	     prev_amt != cudr_rec.l_total_amt)
	{
		BankCharged = FALSE;
		/*----------------------------
		| Debit the bank account for |
		| the previous receipt.      |
		----------------------------*/
		if (!firstReceiptFlag)
		{
			if (cudr2_rec.rec_type [0] == 'A' ||
				cudr2_rec.rec_type [0] == 'D' || 
				cudr2_rec.due_date <= lsystemDate)
			{
				strcpy (accountNumber, crbk_rec.gl_bank_acct);
				strcpy (glmrRec.co_no,  comm_rec.co_no);
				strcpy (glmrRec.acc_no, accountNumber);
				cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
				if (cc)
					sprintf (accountNumber, "NOT VALID");

				/*--------------------------
				| Update lodgement record. |
				--------------------------*/
				upd_lodge ("Y");

				SaveBankRecDetails 
				 (
					cudr2_rec.bank_id,
					cudr2_rec.o_total_amt
				);
			}
			/*--------------------------------	
			| Data from the previous receipt |
			| is stored in cudr2_rec.        |
			--------------------------------*/
			else 
			{
				if (cudr2_rec.rec_type [0] == 'B')
					strcpy (accountNumber, crbk_rec.gl_bill_rec);
				else
					strcpy (accountNumber, crbk_rec.gl_fwd_rec);
				
				strcpy (glmrRec.co_no,  comm_rec.co_no);
				strcpy (glmrRec.acc_no, accountNumber);
				cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
				if (cc)
					sprintf (accountNumber, "NOT VALID");

				/*--------------------------
				| Update lodgement record. |
				--------------------------*/
				upd_lodge ("N");
			}
			if (cudr2_rec.rec_type [0] == 'I' ||
				cudr2_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type, "2");
			else
				strcpy (glwkRec.jnl_type, "1");
			strcpy (cudr2_rec.invoice, "        ");
			strcpy (cudr2_rec.dbt_no, "      ");
			strcpy (cudr2_rec.rec_no, "        ");
			strcpy (cudr2_rec.co_no, comm_rec.co_no);
			cudr2_rec.l_amt_pay = cudr2_rec.l_total_amt;
			cudr2_rec.o_amt_pay = cudr2_rec.o_total_amt;
			cudr2_rec.l_amt_pay -= cudr2_rec.l_disc;
			cudr2_rec.o_amt_pay -= cudr2_rec.o_disc;
			cudr2_rec.l_amt_pay -= bank_l_chg;
			cudr2_rec.o_amt_pay -= bank_o_chg;
			WriteGlwk (&cudr2_rec);
			bank_l_chg = 0.00;
			bank_o_chg = 0.00;
		}

		firstReceiptFlag = FALSE;

		/*---------------------
		| Get bank details if |
		| bank has changed.   |
		---------------------*/
		if (!CheckBankDetails ())
			return;

		prev_amt = cudr_rec.l_total_amt;
		disc_o_amt = cudr_rec.o_disc;
		disc_l_amt = cudr_rec.l_disc;

		/*---------------------------------------------
		| Calculate bank charges (in local currency). |
		---------------------------------------------*/
		bank_o_chg = cudr_rec.bk_charge;
		bank_l_chg = cudr_rec.bk_charge;
		if (cudr_rec.rec_type [0] != 'D' &&
			cudr_rec.rec_type [0] != 'B' &&
			cudr_rec.rec_type [0] != 'A' &&
		    cudr_rec.l_total_amt > 0.00 &&
			cudr2_rec.due_date <= lsystemDate)
		{
			bank_l_chg += crbk_rec.clear_fee;
			bank_o_chg += crbk_rec.clear_fee;
		}
		if (cudr_rec.bk_exch != 0.00)
			bank_o_chg *= cudr_rec.bk_exch;

		bank_l_chg = no_dec (bank_l_chg);
		bank_o_chg = no_dec (bank_o_chg);

		/*-----------------------------
		| Credit the debtors account. |
		-----------------------------*/
		strcpy (previousReceipt, cudr_rec.rec_no);
		strcpy (accountNumber, cumr_rec.gl_ctrl_acct);

		strcpy (glmrRec.co_no,  comm_rec.co_no);
		strcpy (glmrRec.acc_no, accountNumber);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			sprintf (accountNumber, "NOT VALID");

		rpt_l_amt = cudr_rec.l_amt_pay;
		rpt_o_amt = cudr_rec.o_amt_pay;
		cudr_rec.l_amt_pay = cudr_rec.l_total_amt;
		cudr_rec.o_amt_pay = cudr_rec.o_total_amt;
		if (cudr_rec.rec_type [0] == 'I' ||
			cudr_rec.reversal [0] == 'Y')
			strcpy (glwkRec.jnl_type, "1");
		else
			strcpy (glwkRec.jnl_type, "2");
		WriteGlwk (&cudr_rec);

		cudr_rec.o_amt_pay = rpt_o_amt;
		cudr_rec.l_amt_pay = rpt_l_amt;
	}

	strcpy (cudr_rec.rec_no, "        ");
	strcpy (cudr_rec.dbt_no, "      ");
	strcpy (cudr_rec.narrative, "                    ");

	/*----------------
	| Post discount. |
	----------------*/
  	if (disc_o_amt != 0.00 || disc_l_amt != 0.00)
	{
		if (envVarGst && envVarDbGstPost && cumr_rec.tax_code [0] == 'C')
		{
			/*-------------------------------------
			| Calculate amount of GST in discount |
			-------------------------------------*/
			gst_div = (float) (100.00 + envVarGstInclusive) / envVarGstInclusive;
			disc_gst = twodec (disc_o_amt / (double)gst_div);
			disc_o_amt -= disc_gst;
			disc_gst = twodec (disc_l_amt / (double)gst_div);
			disc_l_amt -= disc_gst;

			/*-------------------------------
			| Debit the GST charged account |
			-------------------------------*/
			GL_GLI 
			(
				cudr_rec.co_no,		
				cudr_rec.br_no,	
				"  ",		
				"G.S.T CHRG",	
				 (envVarGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
				" "
			);
			strcpy (accountNumber, glmrRec.acc_no);

			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type, "4");
			else
				strcpy (glwkRec.jnl_type, "3");
			sprintf (cudr_rec.period, "%02d", bankPeriod);
			cudr_rec.l_amt_pay = no_dec (disc_gst);
			cudr_rec.o_amt_pay = no_dec (disc_gst);
			WriteGlwk (&cudr_rec);
		}

		/*----------------------------
		| Debit the discount account |
		----------------------------*/
		strcpy (accountNumber, cudr_rec.gl_disc);
		if (cudr_rec.rec_type [0] == 'I' ||
			cudr_rec.reversal [0] == 'Y')
			strcpy (glwkRec.jnl_type, "2");
		else
			strcpy (glwkRec.jnl_type, "1");
		sprintf (cudr_rec.period, "%02d", bankPeriod);
		cudr_rec.l_amt_pay = no_dec (disc_l_amt);
		cudr_rec.o_amt_pay = no_dec (disc_o_amt);
		WriteGlwk (&cudr_rec);

		sprintf (cudr_rec.period, "%02d", per);
		disc_o_amt = 0;
		disc_l_amt = 0;
		cudr_rec.l_amt_pay = rpt_l_amt;
		cudr_rec.o_amt_pay = rpt_o_amt;
	}

	/*--------------------
	| Post Bank Charges. |
	--------------------*/
  	if (bank_o_chg != 0.00 || bank_l_chg != 0.00)
	{
		if (!BankCharged)
		{
			/*--------------------------------
			| Debit the bank charges account |
			--------------------------------*/
			strcpy (accountNumber,crbk_rec.gl_bank_chg);
			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type, "6");
			else
				strcpy (glwkRec.jnl_type, "5");
			sprintf (cudr_rec.period, "%02d", bankPeriod);
			cudr_rec.l_amt_pay = no_dec (bank_l_chg);
			cudr_rec.o_amt_pay = no_dec (bank_o_chg);
			WriteGlwk (&cudr_rec);

			cudr_rec.l_amt_pay = rpt_l_amt;
			cudr_rec.o_amt_pay = rpt_o_amt;
		}
		BankCharged = TRUE;
	}

	/*---------------------
	| Calculate and post  |
	| Exchange Variation. |
	---------------------*/
	lcl_inv_amt = cudr_rec.o_amt_pay;
	if (cudr_rec.inv_exch != 0.00)
	{
		lcl_inv_amt /= cudr_rec.inv_exch;

		exch_l_var = no_dec (cudr_rec.l_amt_pay) - no_dec (lcl_inv_amt);
		exch_o_var = no_dec (exch_l_var * cudr_rec.inv_exch);
	}
	else
	{
		exch_l_var = 0.00;
		exch_o_var = 0.00;
	}

  	if (exch_l_var != 0.00 && envVarDbMcurr)
	{
		/*-------------------------------------
		| Write to exchange variation account |
		-------------------------------------*/
		strcpy (accountNumber, crbk_rec.gl_exch_var);
		if (exch_l_var > 0.00)
		{
			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type, "5");
			else
				strcpy (glwkRec.jnl_type, "6");
			cudr_rec.l_amt_pay = exch_l_var;
			cudr_rec.o_amt_pay = exch_o_var;
		}
		else
		{
			cudr_rec.l_amt_pay = exch_l_var * -1.00;
			cudr_rec.o_amt_pay = exch_o_var * -1.00;
			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type, "6");
			else
				strcpy (glwkRec.jnl_type, "5");
		}
		sprintf (cudr_rec.period, "%02d", bankPeriod);
		WriteGlwk (&cudr_rec);

		/*-------------------------
		| Write to debtor account |
		-------------------------*/
		strcpy (accountNumber, cumr_rec.gl_ctrl_acct);
		if (exch_l_var > 0.00)
		{
			cudr_rec.l_amt_pay = exch_l_var;
			cudr_rec.o_amt_pay = exch_o_var;
			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type,"6");
			else
				strcpy (glwkRec.jnl_type,"5");
		}
		else
		{
			cudr_rec.l_amt_pay = exch_l_var * -1.00;
			cudr_rec.o_amt_pay = exch_o_var * -1.00;
			if (cudr_rec.rec_type [0] == 'I' ||
				cudr_rec.reversal [0] == 'Y')
				strcpy (glwkRec.jnl_type,"5");
			else
				strcpy (glwkRec.jnl_type,"6");
		}
		sprintf (cudr_rec.period, "%02d", bankPeriod);
		WriteGlwk (&cudr_rec);

		exch_l_var = 0.00;
		exch_o_var = 0.00;
		cudr_rec.l_amt_pay = rpt_l_amt;
		cudr_rec.o_amt_pay = rpt_o_amt;
	}
		
	/*-------------------------------------------
	| Write record for the current detail line. |
	-------------------------------------------*/
	/*
	strcpy (accountNumber, sixteenSpaces);
	if (cudr_rec.rec_type [0] == 'I' ||
		cudr_rec.reversal [0] == 'Y')
		strcpy (glwkRec.jnl_type,"1");
	else
		strcpy (glwkRec.jnl_type,"2");
	WriteGlwk (&cudr_rec);
	*/
}

/*----------------------------------------------
| Write data required for glwk to a sort file. |
----------------------------------------------*/
void
WriteGlwk (
 struct CUDR_REC*   cash_rec)
{
	char	data_str [301];
	char	dbt_srt [6];

	if (!sortOpen)
	{
		fsort = sort_open ("srt_glwk");
		sortOpen = TRUE;
	}

	glmrRec.hhmr_hash = 0;
	if (strcmp (accountNumber, sixteenSpaces))
	{
		strcpy (glmrRec.co_no,  comm_rec.co_no);
		strcpy (glmrRec.acc_no, accountNumber);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			GL_GLI 
			(
				cudr_rec.co_no,	
				cudr_rec.br_no,	
				"  ",			
				"SUSPENSE  ",
				" ",			
				" "
			);
			strcpy (accountNumber,	glmrRec.acc_no);
		}
	}
		
	/*---------------------------------------
	| The following code is to keep each	|
	| debtor header record with its detail  |
	| records								|
	---------------------------------------*/
	if (!strncmp (cash_rec->dbt_no, "      ", 6))
	{
		sprintf (cash_rec->dbt_no, "%-6.6s", currendDbtNo);
		sprintf (dbt_srt, "%04d%-1.1s", debtorsSequenceNo++, "N");
	}
	else
	{
		sprintf (currendDbtNo, "%-6.6s", cash_rec->dbt_no);
		sprintf (dbt_srt, "%04d%-1.1s", debtorsSequenceNo++, " ");
	}

    DateToDMY (comm_rec.dbt_date, NULL, &bankPeriod, NULL);
	sprintf (cash_rec->period, "%02d", bankPeriod);
	cash_rec->rec_date = comm_rec.dbt_date;
	/*---------------------------------
	| Add transaction to sort file  . |
	---------------------------------*/
	sprintf (data_str,
		"%2.2s%c%2.2s%c%-9.9s%c%-15.15s%c%-15.15s%c%f%c%f%c%s%c%s%c%f%c%f%c%s%c%s%c%s%c%s%c%ld%c%ld%c%s%c%f%c%s%c\n",
		cash_rec->co_no,       1,	/* srt_offset = 0  */
		cash_rec->br_no,       1,	/* srt_offset = 1  */
		cash_rec->dbt_no,      1,	/* srt_offset = 2  */
		dbt_srt,               1,	/* srt_offset = 3  */
		cash_rec->rec_no,      1,	/* srt_offset = 4  */
		cash_rec->l_total_amt, 1,	/* srt_offset = 5  */
		cash_rec->o_total_amt, 1,	/* srt_offset = 6  */
		cash_rec->invoice,     1,	/* srt_offset = 7  */
		cash_rec->o_curr,      1,	/* srt_offset = 8  */
		cash_rec->l_amt_pay,   1,	/* srt_offset = 9  */
		cash_rec->o_amt_pay,   1,	/* srt_offset = 10  */
		accountNumber,		   1,	/* srt_offset = 11  */
		cash_rec->period,      1,	/* srt_offset = 12  */
		term_no,               1,	/* srt_offset = 13 */
		glwkRec.jnl_type,  	   1,	/* srt_offset = 14 */
		glmrRec.hhmr_hash,     1,	/* srt_offset = 15 */
		cash_rec->rec_date,    1,	/* srt_offset = 16 */
		cash_rec->narrative,   1,	/* srt_offset = 17 */
		cash_rec->o_exch,	   1,	/* str_offset = 18 */
		cash_rec->dbt_name,	   1	/* str_offset = 19 */
	);

	sort_save (fsort, data_str);
}

void
ProcessSortGlwk (void)
{
	char	*sptr;

	fsort = sort_sort (fsort, "srt_glwk");
	
	sptr = _SortRead (fsort);
	while (sptr)
	{
		/*---------------------------------------------
		| Add line to glwk for account if required  . |
		---------------------------------------------*/
		strcpy (glwkRec.tran_type,  " 6");
		strcpy (glwkRec.stat_flag,  "2");

		if (firstTimeFlag || strcmp (srt_offset [4], "               "))
		{
			sprintf (previousUserRef, "%-15.15s", srt_offset [4]);
			sprintf (glwkRec.user_ref,"%-15.15s", srt_offset [4]);
			firstTimeFlag = FALSE;
		}
		else
			if (strcmp (previousUserRef, srt_offset [4])
			 && strcmp (srt_offset [11],sixteenSpaces))
			{
				strcpy (glwkRec.user_ref, previousUserRef);
			}
			else
				sprintf (glwkRec.user_ref, "%-15.15s", srt_offset [4]);


		sprintf (glwkRec.sys_ref,   "%-10.10s", srt_offset [13]);
		sprintf (glwkRec.period_no, "%-2.2s",   srt_offset [12]);
		sprintf (glwkRec.acc_no,    "%-16.16s", srt_offset [11]);
		sprintf (glwkRec.co_no,     "%2.2s",    srt_offset [0]);
		sprintf (glwkRec.est_no,    "%2.2s",    srt_offset [1]);
		sprintf (glwkRec.name,      "%-30.30s", srt_offset [7]);
		sprintf (glwkRec.narrative, "%-20.20s", srt_offset [17]);
		sprintf (glwkRec.alt_desc1, "%-20.20s", srt_offset [19]);
		sprintf (glwkRec.alt_desc2, "%-20.20s", srt_offset [19] + 20);
		sprintf (glwkRec.alt_desc3, "%-20.20s", " ");
		sprintf (glwkRec.batch_no,  "%-10.10s",   srt_offset [13]);
		if (!strncmp (srt_offset [3] + 4, "N", 1))
			sprintf (glwkRec.acronym, "%-9.9s", " ");
		else
			sprintf (glwkRec.acronym, "%-9.9s", srt_offset [2]);

		sprintf (glwkRec.chq_inv_no, "%-15.15s", srt_offset [4]);
		sprintf (glwkRec.jnl_type,   "%-1.1s",   srt_offset [14]);

		glwkRec.hhgl_hash 		= (long) atof (srt_offset [15]);
		glwkRec.tran_date      	= atol (srt_offset [16]);
		glwkRec.ci_amt    		= atof (srt_offset [5]);
		glwkRec.o1_amt    		= atof (srt_offset [6]);
		glwkRec.amount    		= atof (srt_offset [10]);
		glwkRec.loc_amount    	= atof (srt_offset [9]);
		glwkRec.exch_rate    	= atof (srt_offset [18]);
		sprintf (glwkRec.currency, "%-3.3s",srt_offset [8]);

		glwkRec.post_date = lsystemDate;  
 
		if (glwkRec.post_date < 0)
			glwkRec.post_date = glwkRec.tran_date;
			
	    strcpy (batchNumber, GL_AddBatch ());

		sptr = _SortRead (fsort);
	}

	sort_delete (fsort, "srt_glwk");
}

/*---------------------
| Get bank details if |
| bank has changed.   |
---------------------*/
int
CheckBankDetails (void)
{
	/*----------------------
	| Bank hasn't changed. |
	----------------------*/
	if (!strcmp (currentBank, cudr_rec.bank_id))
		return (TRUE);

	strcpy (crbk_rec.co_no, cudr_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", cudr_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	sprintf (currentBank, "%-5.5s", crbk_rec.bank_id);

	return (TRUE);
}

int
upd_lodge (
 char*              to_stat)
{
	cc = find_hash (bldt, &bldt_rec, COMPARISON, "u",cudr2_rec.hhcp_hash);
	if (cc)
		return (FALSE);

	sprintf (bldt_rec.posted_gl, "%-1.1s", to_stat);
	cc = abc_update (bldt, &bldt_rec);
	if (cc)
		file_err (cc, "bldt", "DBUPDATE");

	return (TRUE);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_SortRead (
 FILE*              srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset [0] = sptr;

	tptr = sptr;
	while (fld_no < 20)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset [fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}
/*====================================
| Routine to save bank transactions. |
====================================*/
void	
SaveBankRecDetails 	 (
	char	*bankId,
	double	postingAmount)
{
	/*-------------------------------------------------
	| Check the array size before adding new element. |
	-------------------------------------------------*/
	if (!ArrChkLimit (&bank_d, bank, bankCnt))
		sys_err ("ArrChkLimit (bank)", ENOMEM, PNAME);

	/*-----------------------------------------
	| Load values into array element bankCnt. |
	-----------------------------------------*/
	sprintf (bank [bankCnt].bankId, "%-6.6s", bankId);
	bank [bankCnt].bankAmount = postingAmount;

	/*--------------------------
	| Increment array counter. |
	--------------------------*/
	bankCnt++;
}
void
PostCashBookEntrys (void)
{
	int		i	=	0;
	char	prevBankId [7];
	double	totalBankAmount =	0.00;

	/*-------------------------------------------
	| Sort the array in item description order. |
	-------------------------------------------*/
	qsort (bank, bankCnt, sizeof (struct BankRecord), BankIdSort);

	/*----------------------------------------------------------------
	| Step through the sorted array getting the appropriate records. |
	----------------------------------------------------------------*/
	for (i = 0; i < bankCnt; i++)
	{
		if (!i)
			strcpy (prevBankId, bank [i].bankId);

		if (!strcmp (bank [i].bankId, prevBankId))
			totalBankAmount += bank [i].bankAmount;
		else
		{
			/*---------------------------------
			| Write entry to cash book system |
			---------------------------------*/
			sprintf (err_str, "Cash receipt posting for %s", systemDate);
			WriteCashBook
			(								/*--------------------------*/
				comm_rec.co_no,				/* Company Number			*/
				comm_rec.est_no,			/* Branch Number.			*/
				prevBankId,					/* Bank Id.					*/
				comm_rec.dbt_date, 			/* Transaction Date			*/
				cashBookDesc,				/* Transaction Narrative.	*/
				"R",						/* Transaction Type.		*/
				totalBankAmount,			/* Amount posted to bank.	*/
				"0",						/* Status flag.				*/
				lsystemDate					/* System/period date.		*/
			);								/*--------------------------*/
			totalBankAmount	=	bank [i].bankAmount;
		}
		strcpy (prevBankId, bank [i].bankId);
	}
	/*---------------------------------
	| Write entry to cash book system |
	---------------------------------*/
	WriteCashBook
	 (								/*--------------------------*/
		comm_rec.co_no,				/* Company Number			*/
		comm_rec.est_no,			/* Branch Number.			*/
		prevBankId,					/* Bank Id.					*/
		comm_rec.dbt_date, 			/* Transaction Date			*/
		cashBookDesc,				/* Transaction Narrative.	*/
		"R",						/* Transaction Type.		*/
		totalBankAmount,			/* Amount posted to bank.	*/
		"0",						/* Status flag.				*/
		lsystemDate					/* System/period date.		*/
	);								/*--------------------------*/

	/*--------------------------
	| Free up the array memory |
	--------------------------*/
	ArrDelete (&bank_d);
}

/*=====================
| Quick Sort routine. |
=====================*/
int 
BankIdSort (
 const void *a1, 
 const void *b1)
{
	int		result;
	const struct BankRecord a = * (const struct BankRecord *) a1;
	const struct BankRecord b = * (const struct BankRecord *) b1;

	result = strcmp (a.bankId, b.bankId);

	return (result);
}


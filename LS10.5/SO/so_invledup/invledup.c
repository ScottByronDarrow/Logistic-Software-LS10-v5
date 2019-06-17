/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: invledup.c,v 5.5 2001/08/26 22:46:38 scott Exp $
|  Program Name  : (so_invledup.c & so_crdledup.c) 
|  Program Desc  : (Customer Update To ledger And G/L)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: invledup.c,v $
| Revision 5.5  2001/08/26 22:46:38  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 5.4  2001/08/24 05:59:50  scott
| Updated from scotts machine
|
| Revision 5.2  2001/08/09 09:21:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:22  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: invledup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invledup/invledup.c,v 5.5 2001/08/26 22:46:38 scott Exp $";

#define	INVOICE		(processTypeFlag [0] == 'I' || processTypeFlag [0] == 'P')
#define	NOTAX		(cohr_rec.tax_code [0] == 'A' || \
					  cohr_rec.tax_code [0] == 'B')
#define	GST		 	(envGst [0] == 'Y')

#include 	<pslscr.h>
#include 	<twodec.h>
#include 	<GlUtils.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>

#define		CASH	(!strcmp (cumr_rec.dbt_no, esmr_rec.sales_acc))

#define POS		1
#define NEG		0
#define HEAD	1
#define LINE	0

		int 	envDbTotalAge;
	    int		envDbDaysAgeing;

FILE	*fout,
		*fin;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct bkcrRecord	bkcr_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct cuinRecord	cuin_rec;
struct cuinRecord	cuin2_rec;
struct somiRecord	somi_rec;
struct exstRecord	exst_rec;
struct cuccRecord	cucc_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct sobgRecord	sobg_rec;
struct felnRecord	feln_rec;
struct sachRecord	sach_rec;
struct saclRecord	sacl_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	char	*data	= "data",
			*cuin2	= "cuin2";

	int		printerNumber 	= 1,
			FirstTime 		= TRUE,
			envDbCo 		= 0,
			envDbFind 		= 0,
			envDbNettUsed 	= TRUE,
			envCnNettUsed 	= TRUE,
			envFeInstall 	= FALSE,
			envSaCommission = FALSE,
			envDbMcurr		= 0,
			taxYear			= 0,
			taxMonth		= 0;

	long	accRecHash		= 0L,
			bankAccHash		= 0L,
			origHhciHash	= 0L,
			longSystemDate	= 0L;

	float	envGstInclusive	= 0.00,
			gstDivide		= 0.00,
			gstPercent		= 0.00;

	double	grandTotals [7]	=	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
			localExchRate	=	0.00;

	char	processTypeFlag 	[2],
			findStatus 			[2],
			updateStatus 		[2],
			cashReceiptNumber 	[sizeof cuhd_rec.receipt_no],
			cashPeriod 			[3],
			envGst 				[2],
			currentBranch 		[3],
			env_string 			[16],
			systemDate 			[11],
			paymentTerms 		[4],
			gstCode 			[4],
			mlInvLedup 			[101],
			accountsRec 		[sizeof glmrRec.acc_no],
			bankAccount 		[sizeof glmrRec.acc_no];

#include 	<p_terms.h>
#include 	<pr_format3.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>

/*
 * Function Declarations
 */
double 	ExtractGst 			(double, int, int);
int  	check_page 			(void);
int  	MissingCustomer 	(long);
int  	ProcessLines 		(long);
int  	ValidPaymentTerms 	(char *);
void 	AddCashSale 		(double);
void 	AddCommission 		(long, long, double);
void 	AddCucc 			(void);
void 	AddCudt 			(long, double, long);
void 	AddCuhd 			(char *, double, char *, char *, long);
void 	AddExst 			(float, double, double, char *);
void 	AddFeln 			(int, double);
void 	AddSobg 			(int, char *, long);
void 	CloseDB 			(void);
void 	InitML 				(void);
void 	NoCohrLines 		(void);
void 	NormalGstPosting 	(int);
void 	OpenAuditFile 		(void);
void 	OpenDB 				(void);
void 	PrintFooter 		(void);
void 	PrintLineDetails 	(char *, char *, char *, double, double, double, double, double, double, double, char *);
void 	ProcessCohr 		(void);
void 	ReadBranch 			(void);
void 	ReadGlDefaults 		(char *);
void 	SplitGst 			(int);
void 	WriteGlRecord 		(int, char *, char *, long, double);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	strcpy (systemDate, DateToString (TodaysDate ()));
	longSystemDate = TodaysDate ();
	
	/*
	 * Forward Exchange Enabled?
	 */
	sptr = chk_env ("FE_INSTALL");
	envFeInstall = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Process sales commission. 
	 */
	sptr = chk_env ("SA_COMMISSION");
	envSaCommission = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if ageing is by days overdue or as per standard ageing. 
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for multi-currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envCnNettUsed = (sptr == (char *)0) ? envDbNettUsed : atoi (sptr);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	sprintf (envGst, "%-1.1s",get_env ("GST"));

	/*
	 * Get gst code. 
	 */
	if (GST)
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (gstCode, "%-3.3s", "TAX");

	envGstInclusive = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (envGstInclusive != 0.00)
	{
		gstDivide = (float) ((100.00 + envGstInclusive) / envGstInclusive);
		gstPercent  = envGstInclusive;
	}
	else
	{
		gstDivide = 0.00;
		gstPercent  = 0.00;
	}

	if (argc < 5)
	{
		print_at (0,0,"Usage %s : <findStatusFlag> <updateStatusFlag> <transactionType> <LPNO> <Optional Branch Number>", argv [0]);
		return (EXIT_FAILURE);
	}
	
	strcpy (findStatus, 	argv [1]);
	strcpy (updateStatus, 	argv [2]);
	strcpy (processTypeFlag, argv [3]);
	printerNumber = atoi (argv [4]);

	set_tty ();

	OpenDB ();

	InitML ();

	if (argc == 6)
		strcpy (currentBranch, argv [5]);
	else
		strcpy (currentBranch, comm_rec.est_no);

	ReadBranch ();
		
	init_scr ();

	sprintf (err_str,
		" Processing %s to Customer Ledger.",
		(INVOICE) ? "Invoices" : "Credits");
	print_mess (err_str);

	longSystemDate = TodaysDate ();


	ProcessCohr ();
	if (FirstTime == FALSE)
		PrintFooter ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
InitML (void)
{
	strcpy (mlInvLedup, ML ("Pay. terms for Inv %8.8s not standard. %-38.38s"));
}

/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	/*
	 * Read common terminal record . 
	 */
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cuin2, cuin);

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no");
	open_rec (cuin2,cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (exst, exst_list, EXST_NO_FIELDS, "exst_id_no");
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
	open_rec (somi, somi_list, SOMI_NO_FIELDS, "somi_id_no");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	if (envDbMcurr)
	{
		open_rec (bkcr, bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	}
	if (envFeInstall)
	{
		open_rec (feln, feln_list, FELN_NO_FIELDS, "feln_id_no");
	}
	if (envSaCommission)
	{
		open_rec (sach,  sach_list, SACH_NO_FIELDS, "sach_hhci_hash");
		open_rec (sacl,  sacl_list, SACL_NO_FIELDS, "sacl_id_no");
	}
	OpenGljc ();
	OpenGlmr ();
	
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close Database files.
 */
void
CloseDB (void)
{
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cuin);
	abc_fclose (cuin2);
	abc_fclose (cumr);
	abc_fclose (exst);
	abc_fclose (sobg);
	abc_fclose (somi);
	abc_fclose (cuhd);
	abc_fclose (cudt);
	abc_fclose (cucc);
	if (envDbMcurr)
	{
		abc_fclose (bkcr);
		abc_fclose (esmr);
	}
	if (envFeInstall)
		abc_fclose (feln);

	if (envSaCommission)
	{
		abc_fclose (sach);
		abc_fclose (sacl);
	}
	GL_CloseBatch (printerNumber);
	GL_Close ();

	abc_dbclose (data);
}

/*
 * Read branch specific information. 
 */
void
ReadBranch (void)
{
	int		tmp_dmy [3];

	/*
	 * Read branch record. 
	 */
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	strcpy (esmr_rec.est_no, currentBranch);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
	
	abc_fclose (esmr);

	/*
	 * Set period and cashReceiptNumber based on date. 
	 */
	DateToDMY (comm_rec.dbt_date, &tmp_dmy [0],&tmp_dmy [1],&tmp_dmy [2]);
	sprintf (cashReceiptNumber, "CR%02d%02d", tmp_dmy [0], tmp_dmy [1]);
	sprintf (cashPeriod, "%02d", tmp_dmy [1]);

	ReadGlDefaults (comm_rec.est_no);

	return;
}

/*
 * Process whole cohr file looking for stat = findStatus              
 * and updating record appropriately.                               
 * Returns: 0 if ok, non-zero if not ok.                           
 */
void
ProcessCohr (void)
{
	int		moreRecordsToProcess	=	TRUE;

	/*
	 * Read whole cohr file. 
	 */
	while (moreRecordsToProcess) 
	{
		strcpy (cohr_rec.co_no,     comm_rec.co_no);
		strcpy (cohr_rec.br_no,     currentBranch);
		strcpy (cohr_rec.type,      processTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatus);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc) 
		{
			abc_unlock (cohr);
			moreRecordsToProcess = FALSE;
			continue;
		}
		DateToDMY 
		(
			cohr_rec.date_raised,
			NULL, 
			&taxMonth, 
			&taxYear
		);
	
		/*
		 * Check cohr exchange rate. 
		 */
		localExchRate	=	cohr_rec.exch_rate;
		if (localExchRate == 0.00)
			localExchRate = 1.00;

		/*
		 * Get payment Terms from Invoice. 
		 */
		sprintf (paymentTerms, "%-3.3s", cohr_rec.pay_terms);

		/*
		 * Validate terms against those on file. 
		 */
		if (ValidPaymentTerms (paymentTerms))
			strcpy (paymentTerms, "20A");

		if (gstDivide != 0.00 && !NOTAX)
			SplitGst (TRUE);
		else
			NormalGstPosting (TRUE);

		/*
		 * Get branch record. 
		 */
		if (envDbMcurr)
		{
			if (strcmp (cohr_rec.co_no, esmr_rec.co_no) ||
			    strcmp (cohr_rec.br_no, esmr_rec.est_no))
			{
				strcpy (esmr_rec.co_no, cohr_rec.co_no);
				strcpy (esmr_rec.est_no, cohr_rec.br_no);
				cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
				if (cc)
					file_err (cc, esmr, "DBFIND");

			}
		}
		else
			bkcr_rec.ex1_factor = 1.00;

		/*
		 * Process line records. 
		 */
		cc = ProcessLines (cohr_rec.hhco_hash);
		if (cc) 
		{
			moreRecordsToProcess = FALSE;
			continue;
		}

		/*
		 * Payment terms are not standard 20A so add to file. 
		 */
		if (strncmp (paymentTerms, cumr_rec.crd_prd, 3) && INVOICE)
			AddCucc ();


	}
	return;
}

/*
 * Validate payment terms.
 */
int
ValidPaymentTerms (
 char	*terms)
{
	int	i;

	for (i = 0; strlen (p_terms [i]._pcode); i++)
		if (!strcmp (terms, p_terms [i]._pcode))
			return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}
/*
 * Procesess All Customer Invoice Lines.
 */
int
ProcessLines (
	long	hhcoHash)
{                                       /*===================================*/
	int		newInvoice		= TRUE,		/*| Assume new invoice header.      |*/
			perNo			= 0;        /*| Holds period number for ageing. |*/
                                        /*|---------------------------------|*/
	double  totInv			= 0.00,     /*| Total Invoice.                  |*/
			totDisc			= 0.00,  	/*| Total Discount.                 |*/
			tempDiscount   	= 0.00;	   	/*| Temp  Discount.                 |*/
                                        /*===================================*/
	long	age_date = 0L;

	double	grossSaleAmount		= 0.00;
	double	freightOtherCosts	= 0.00;

	cumr_rec.hhcu_hash	= (cohr_rec.chg_hhcu_hash > 0L)
								? cohr_rec.chg_hhcu_hash
								: cohr_rec.hhcu_hash;
	
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (cumr);
		if (cohr_rec.chg_hhcu_hash > 0L)
		{
			cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "u");
		}
		if (cc)
		{
			abc_unlock (cumr);
			MissingCustomer (cumr_rec.hhcu_hash);
		}
	}
	if (envDbMcurr)
	{
		/*
		 * Get bkcr record. 
		 */
		strcpy (bkcr_rec.co_no, cohr_rec.co_no);
		sprintf (bkcr_rec.bank_id, "%-5.5s", esmr_rec.dflt_bank);
		sprintf (bkcr_rec.curr_code, "%-3.3s", cumr_rec.curr_code);
		cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
		if (cc)
			bkcr_rec.ex1_factor = 1.00;
	}
	/*
	 * If Zero date gets through then set to debtors module date. 
	 */
	if (cohr_rec.date_raised == 0L)
		cohr_rec.date_raised = comm_rec.dbt_date;

	/*
	 * See if cuin record already on file. 
	 */
	cuin_rec.hhcu_hash		=	cumr_rec.hhcu_hash;
	cuin_rec.ho_hash		=	(cumr_rec.ho_dbt_hash > 0L)
									? cumr_rec.ho_dbt_hash
									: cumr_rec.hhcu_hash;
	cuin_rec.ord_hhcu_hash	=	cohr_rec.hhcu_hash;

	strcpy (cuin_rec.est,    cohr_rec.br_no);
	strcpy (cuin_rec.inv_no, cohr_rec.inv_no);
	cc = find_rec (cuin, &cuin_rec, COMPARISON, "u");
	newInvoice = (cc) ? TRUE : FALSE;

	if (envDbFind)
		strcpy (cuin_rec.est, cohr_rec.br_no);

	strcpy (cuin_rec.pay_terms,  paymentTerms);
	strcpy (cuin_rec.er_fixed,   cohr_rec.fix_exch);
	sprintf (cuin_rec.currency, "%-3.3s", cumr_rec.curr_code);
	cuin_rec.exch_rate		= localExchRate;
	cuin_rec.date_of_inv	= cohr_rec.date_raised;

	cuin_rec.due_date	= CalcDueDate (cuin_rec.pay_terms,cuin_rec.date_of_inv);
	
	freightOtherCosts = no_dec (cohr2_rec.freight) 		+ 
			  			no_dec (cohr2_rec.insurance) 	+ 
			  			no_dec (cohr2_rec.other_cost_1) + 
			  			no_dec (cohr2_rec.other_cost_2) + 
			  			no_dec (cohr2_rec.other_cost_3) +
			  			no_dec (cohr2_rec.item_levy) 	+
			  			no_dec (cohr2_rec.sos)			-
						no_dec (cohr2_rec.ex_disc);

	if (INVOICE)
	{
		if (envDbNettUsed)
		{
			PrintLineDetails 
			(
				cohr_rec.inv_no, 
				cumr_rec.dbt_no, 
				cumr_rec.dbt_name, 
				cohr2_rec.gross,
				cohr2_rec.disc,
				cohr2_rec.gross - 
				cohr2_rec.disc,
				freightOtherCosts, 
				cohr2_rec.tax, 
				cohr2_rec.gst,
				(cohr2_rec.gross + cohr2_rec.tax + 
				freightOtherCosts + cohr2_rec.gst) - 
				cohr2_rec.disc,
				cohr_rec.batch_no
			);
		}
		else
		{
			PrintLineDetails 
			(
				cohr_rec.inv_no, 
				cumr_rec.dbt_no, 
				cumr_rec.dbt_name, 
				cohr2_rec.gross,
				cohr2_rec.disc,
				cohr2_rec.gross,
				freightOtherCosts, 
				cohr2_rec.tax, 
				cohr2_rec.gst,
				(cohr2_rec.gross + cohr2_rec.tax + 
				freightOtherCosts + cohr2_rec.gst),
				cohr_rec.batch_no
			);
		}
	}
	else
	{
		if (envCnNettUsed)
		{
			PrintLineDetails 
			(
				cohr_rec.inv_no, 
				cumr_rec.dbt_no, 
				cumr_rec.dbt_name, 
				cohr2_rec.gross,
				cohr2_rec.disc,
				cohr2_rec.gross - 
				(cohr2_rec.disc),
				freightOtherCosts, 
				cohr2_rec.tax, 
				cohr2_rec.gst,
				(cohr2_rec.gross + cohr2_rec.tax + 
				freightOtherCosts + cohr2_rec.gst) - 
				cohr2_rec.disc, 
				cohr_rec.batch_no
			);
		}
		else
		{
			PrintLineDetails 
			(
				cohr_rec.inv_no, 
				cumr_rec.dbt_no, 
				cumr_rec.dbt_name, 
				cohr2_rec.gross,
				cohr2_rec.disc,
				cohr2_rec.gross,
				freightOtherCosts, 
				cohr2_rec.tax, 
				cohr2_rec.gst,
				(cohr2_rec.gross + cohr2_rec.tax + 
				freightOtherCosts + cohr2_rec.gst),
				cohr_rec.batch_no
			);
		}
	}
		
	/*
	 * Process all order lines. 
	 */
	coln_rec.hhco_hash	=	hhcoHash;
	coln_rec.line_no		=	0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	if (cc || coln_rec.hhco_hash != hhcoHash)
	{
		abc_unlock (coln);
		NoCohrLines ();
	}
	while (!cc && hhcoHash == coln_rec.hhco_hash)
	{
		if (INVOICE && !envDbNettUsed)
		{
			tempDiscount	=	coln_rec.gst_pc + 100.00;
			tempDiscount	*=	coln_rec.amt_disc;
			totDisc			+=	DOLLARS (tempDiscount);
		}

		if (gstDivide != 0.00 && !NOTAX)
			SplitGst (FALSE);
		else
			NormalGstPosting (FALSE);
		
		if (GST)
		{
			if (INVOICE)
			{
				if (envDbNettUsed)
				{
					AddExst
					(
						coln2_rec.gst_pc, 
						coln2_rec.amt_gst,
						coln2_rec.gross - 
						coln2_rec.amt_disc, 	
						cohr_rec.tax_code 
					);
				}
				else
				{
					AddExst
					(
						coln2_rec.gst_pc, 
						coln2_rec.amt_gst,
						coln2_rec.gross, 
						cohr_rec.tax_code 
					);
				}
			}
			else

			{
				if (envCnNettUsed)
				{
					AddExst
					(
						coln2_rec.gst_pc, 
						coln2_rec.amt_gst,
						coln2_rec.gross - 
						coln2_rec.amt_disc, 	
						cohr_rec.tax_code 	
					);
				}
				else
				{
					AddExst
					(
						coln2_rec.gst_pc, 
						coln2_rec.amt_gst,
						coln2_rec.gross, 
						cohr_rec.tax_code 
					);
				}
			}
		}
		else
		{
			if (INVOICE)
			{
				if (envDbNettUsed)
				{
					AddExst
					(
						coln2_rec.tax_pc, 
						coln2_rec.amt_tax,
						coln2_rec.gross - 
						coln2_rec.amt_disc, 	
						cohr_rec.tax_code 
					);
				}
				else
				{
					AddExst
					(
						coln2_rec.tax_pc, 
						coln2_rec.amt_tax,
						coln2_rec.gross, 
						cohr_rec.tax_code 
					);
				}
			}
			else
			{
				if (envCnNettUsed)
				{
					AddExst
					(
						coln2_rec.tax_pc, 
						coln2_rec.amt_tax,
						coln2_rec.gross - 
						coln2_rec.amt_disc, 	
						cohr_rec.tax_code 
					);
				}
				else
				{
					AddExst
					(
						coln2_rec.tax_pc, 
						coln2_rec.amt_tax,
						coln2_rec.gross, 
						cohr_rec.tax_code 
					);
				}
			}
		}
		
		strcpy (coln_rec.stat_flag, updateStatus);
		abc_update (coln, &coln_rec);

		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	abc_unlock (coln);

	freightOtherCosts = no_dec (cohr2_rec.freight) 		+ 
					    no_dec (cohr2_rec.insurance) 	+ 
					    no_dec (cohr2_rec.other_cost_1) + 
					    no_dec (cohr2_rec.other_cost_2) + 
					    no_dec (cohr2_rec.other_cost_3) +
					    no_dec (cohr2_rec.item_levy) 	+
					    no_dec (cohr2_rec.sos)			-
						no_dec (cohr2_rec.ex_disc);

	/*
	 * Work Out Total Amount of invoice. 
	 */
	totInv = 	no_dec (cohr2_rec.gross) 	+ 
				no_dec (cohr2_rec.tax) 		+ 
				no_dec (cohr2_rec.gst) 		+ 
				no_dec (freightOtherCosts);

	if (INVOICE)
	{
	    if (envDbNettUsed)
	    {
		    grossSaleAmount =	no_dec (cohr2_rec.gross - cohr2_rec.disc);
	    }
	    else
	    {
		    grossSaleAmount = no_dec (cohr2_rec.gross);
	    }
	}
	else
	{
	    if (envCnNettUsed)
	    {
		    grossSaleAmount = no_dec (cohr2_rec.gross - cohr2_rec.disc);
	    }
	    else
	    {
		    grossSaleAmount = no_dec (cohr2_rec.gross);
	    }
	}

	if (INVOICE && envDbNettUsed)
		totDisc = no_dec (cohr2_rec.disc);
	
	if (!INVOICE && envCnNettUsed)
		totDisc = no_dec (cohr2_rec.disc);
	
 	/*
	 * Update existing Customer invoice.  
	 */
	if (!newInvoice) 
	{
	    if (INVOICE)
	    {
	 		cuin_rec.amt  += no_dec (totInv);
			cuin_rec.disc += no_dec (totDisc);
	    }
	    else
	    {
	 		cuin_rec.amt  -= no_dec (totInv);
			cuin_rec.disc -= no_dec (totDisc);
	    }

	    cuin_rec.date_posted = longSystemDate;

		cuin_rec.due_date = CalcDueDate (cuin_rec.pay_terms, cuin_rec.date_of_inv);

	    cc = abc_update (cuin, &cuin_rec);
	    if (cc)
	 		file_err (cc, cuin, "DBUPDATE");
    
	}
	else 
	{
	    cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
    	cuin_rec.ho_hash = (cumr_rec.ho_dbt_hash > 0L)
								? cumr_rec.ho_dbt_hash
								: cumr_rec.hhcu_hash;

	   	strcpy (cuin_rec.co_no, cohr_rec.co_no);
	   	strcpy (cuin_rec.est, cohr_rec.br_no);
	   	strcpy (cuin_rec.dp,  cohr_rec.dp_no);
	
	   	strcpy (cuin_rec.inv_no,cohr_rec.inv_no);
	   	strcpy (cuin_rec.narrative, "                    ");
	   	strcpy (cuin_rec.stat_flag, "0");
	   	strcpy (cuin_rec.drop_ship, cohr_rec.drop_ship);
	   	cuin_rec.date_posted = longSystemDate;
		strcpy (cuin_rec.grn_no, cohr_rec.grn_no);

		/*
		 * Type '1' is Invoice, type '2' is Credit. 
		 */
	    if (INVOICE)
	    {
			strcpy (cuin_rec.type, "1");
			cuin_rec.amt  = no_dec (totInv);
			cuin_rec.disc = no_dec (totDisc);
	    }
	    else
	    {
			strcpy (cuin_rec.type, "2");
			cuin_rec.amt  = no_dec (totInv * -1);
			cuin_rec.disc = no_dec (totDisc * -1);
	   	}
		cuin_rec.due_date = CalcDueDate (cuin_rec.pay_terms,
										 	 cuin_rec.date_of_inv);
	   	cc = abc_add (cuin, &cuin_rec);
	   	if (cc) 
	       	file_err (cc, cuin, "DBADD");

	   	abc_unlock (cuin);
		
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
	       	file_err (cc, cuin, "DBFIND");
	}
	
	if (envFeInstall)
		AddFeln (newInvoice, totInv);

	strcpy (somi_rec.co_no,  cohr_rec.co_no);
	strcpy (somi_rec.br_no,  cohr_rec.br_no);
	strcpy (somi_rec.inv_no, cohr_rec.inv_no);

	if (find_rec (somi, &somi_rec, COMPARISON, "r"))
	{
		/*
		 * Add somi. 
		 */
		cc = abc_add (somi, &somi_rec);
		if (cc)
	   	    	file_err (cc, somi, "DBADD");
	}
	age_date = cohr_rec.date_raised;

	origHhciHash	=	cuin_rec.hhci_hash;

	/*
 	 * Credit is to be applied to an invoice or if invoice not there 
	 * Create a transaction to show invoice created against credit.  
	 */
	if (strcmp (cohr_rec.app_inv_no, "        ") && 
	 	 strcmp (cohr_rec.app_inv_no, "00000000") && 
		 !INVOICE && !CASH) 
	{
		/*
		 * See if invoice that is being applied    
		 * to is on file. If not then leave as is. 
		 */
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.est,    cohr_rec.br_no);
		strcpy (cuin_rec.inv_no, cohr_rec.app_inv_no);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc || cuin_rec.type [0] != '1')
		{
			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
			strcpy (cuin_rec.inv_no, cohr_rec.app_inv_no);
			cc = find_rec (cuin2, &cuin_rec, COMPARISON, "r");
		}
		if (!cc && cuin_rec.type [0] == '1')
		{
			age_date = cuin_rec.date_of_inv;

			AddCuhd 
			(							/*----------------------*/
				"2",					/*	Linking Journal		*/
				totInv - totDisc, 		/*	Nett Amount			*/
				cohr_rec.inv_no,		/* 	Invoice Number		*/
				cohr_rec.app_inv_no,	/*	Applied to Number	*/
				cuin_rec.hhci_hash		/* 	Invoice Unique Hash	*/
			);							/*----------------------*/

			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
			strcpy (cuin_rec.est, cohr_rec.br_no);
			strcpy (cuin_rec.inv_no, cohr_rec.inv_no);
			cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
			if (cc || cuin_rec.type [0] != '1')
			{
				cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
				strcpy (cuin_rec.inv_no, cohr_rec.inv_no);
				cc = find_rec (cuin2, &cuin_rec,COMPARISON,"r");
			}
			AddCuhd 
			(								/*----------------------*/
				"2",						/*	Linking Journal		*/
				(totInv - totDisc) * -1,	/*	Nett Amount			*/
				cohr_rec.app_inv_no,		/*	Applied to Number	*/
				cohr_rec.inv_no,			/* 	Invoice Number		*/
				cuin_rec.hhci_hash			/* 	Invoice Unique Hash	*/
			);								/*----------------------*/
		}
	}
	if (CASH)
		AddCashSale (totInv - totDisc);
	
	perNo = 	AgePeriod 
				(
					cuin_rec.pay_terms,
					age_date,
					comm_rec.dbt_date,
					cuin_rec.due_date,
					envDbDaysAgeing,
					envDbTotalAge
				);

	if (!CASH)
	{
		if (INVOICE)
		{
			if (perNo == -1)
			{
				if (envDbNettUsed)
					cumr_balance [5] += no_dec (totInv - totDisc);
				else
					cumr_balance [5] += no_dec (totInv);
			}
			else
			{
				if (envDbNettUsed)
					cumr_balance [perNo] += no_dec (totInv - totDisc);
				else
					cumr_balance [perNo] += no_dec (totInv);
			}

			if (perNo == 0 || perNo == -1)
				cumr_rec.mtd_sales += no_dec (grossSaleAmount);
		} 
		else
		{
			if (perNo == -1)
			{
				if (envCnNettUsed)
					cumr_balance [5] -= no_dec (totInv - totDisc);
				else
					cumr_balance [5] -= no_dec (totInv);
			}
			else
			{
				if (envCnNettUsed)
					cumr_balance [perNo] -= no_dec (totInv - totDisc);
				else
					cumr_balance [perNo] -= no_dec (totInv);
			}
			if (perNo == 0 || perNo == -1)
				cumr_rec.mtd_sales -= no_dec (grossSaleAmount);
		} 
	}

	/*
	 * Update year-to-date balance. 
	 */
	if (INVOICE)
		cumr_rec.ytd_sales += no_dec (grossSaleAmount);
	else
		cumr_rec.ytd_sales -= no_dec (grossSaleAmount);

	/*
	 * Update Date of last invoice.  
	 */
	cumr_rec.date_lastinv = cohr_rec.date_raised;

	if (CASH)
		cumr_rec.date_lastpay = cohr_rec.date_raised;

	cc = abc_update (cumr, &cumr_rec);
	if (cc)
		file_err (cc, cumr, "DBUPDATE");
	
	strcpy (cohr_rec.stat_flag, updateStatus);
	cc = abc_update (cohr, &cohr_rec);
	if (cc) 
		file_err (cc, cohr, "DBUPDATE");

	AddSobg (0, "RO", cumr_rec.hhcu_hash);

	return (EXIT_SUCCESS);
}

/*
 * Add cash sale entry.
 */
void
AddCashSale (
 double	postAmoumt)
{

	/*
	 * Reverse Sign for Credit Notes. 
	 */
	if (!INVOICE)
		postAmoumt *= -1;

	/*
	 * Add Cheque header record. 
	 */
	AddCuhd
	(
		"1", 
		postAmoumt , 
		cashReceiptNumber, 
		cohr_rec.inv_no, 
		origHhciHash
	);

	/*
	 * Write General Ledger entrys  
	 */
	WriteGlRecord
	(
		TRUE,  
		"2", 
		accountsRec, 
		accRecHash, 
		postAmoumt
	);
	WriteGlRecord
	(
		FALSE, 
		"2", 
		"         ", 
		0L, 
		postAmoumt
	);
	WriteGlRecord
	(
		FALSE, 
		"1", 
		bankAccount, 
		bankAccHash, 
		postAmoumt
	);
}
/*
 * Write General Ledger Record.
 */
void
WriteGlRecord (
 int	first_tran,
 char	*type,
 char	*account,
 long	gl_hash,
 double	amt)
{
	/*
	 * Add transaction for account if required  . 
	 */
	strcpy (glwkRec.tran_type, " 6");
	sprintf (glwkRec.sys_ref,  "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.period_no, cashPeriod);
	sprintf (glwkRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,account);
	strcpy (glwkRec.co_no,     comm_rec.co_no);
	strcpy (glwkRec.est_no,    comm_rec.est_no);
	sprintf (glwkRec.name,     "%-30.30s", cohr_rec.inv_no);
	sprintf (glwkRec.narrative,"C/SALE REC# %8.8s", cashReceiptNumber);
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");
	sprintf (glwkRec.acronym,  "%-9.6s", (first_tran) ? cumr_rec.dbt_no
							  : "         ");
	sprintf (glwkRec.user_ref,  "%-15.15s", cohr_rec.inv_no);
	strcpy (glwkRec.chq_inv_no, (first_tran) ? cashReceiptNumber 
						    : "               ");
	strcpy (glwkRec.stat_flag,"2");
	strcpy (glwkRec.jnl_type,type);
	glwkRec.hhgl_hash 	= gl_hash;
	glwkRec.tran_date      	= comm_rec.dbt_date;
	glwkRec.ci_amt    	= no_dec (amt);
	glwkRec.post_date 	= longSystemDate;
	glwkRec.loc_amount	= no_dec (amt / cohr_rec.exch_rate);
	glwkRec.amount    	= no_dec (amt);
	glwkRec.exch_rate 	= cohr_rec.exch_rate;
	glwkRec.o2_amt    	= cohr_rec.exch_rate;
	strcpy (glwkRec.currency, cumr_rec.curr_code);

	GL_AddBatch ();
}

/*
 * Get General Ledger Defaults.
 */
void
ReadGlDefaults (
 char	*branchNumber)
{
	GL_GLI 
	(
		comm_rec.co_no,
		branchNumber,
		"  ",
		"ACCT REC  ",
		" ",
		" "
	);
	strcpy (accountsRec, glmrRec.acc_no);
	accRecHash = glmrRec.hhmr_hash;
	
	GL_GLI 
	(
		comm_rec.co_no,
		branchNumber,
		"  ",
		"BANK ACCT ",
		" ",
		" "
	);
	strcpy (bankAccount, glmrRec.acc_no);
	bankAccHash = glmrRec.hhmr_hash;
}

void
AddFeln (
 int	newInv,
 double	totInv)
{
	long	hhfe_hash;

	/*
	 * Reduce feln record for cohr by posted amount and delete if zero.
	 * Add/Update feln for cuin by the same.                            
	 */
	strcpy (feln_rec.index_by, "I");
	feln_rec.index_hash = cohr_rec.hhco_hash;
	cc = find_rec (feln, &feln_rec, EQUAL,"u");
	if (cc)
	{
		abc_unlock (feln);
		return;
	}
	
	hhfe_hash = feln_rec.hhfe_hash;
	feln_rec.value -= totInv;
	if (feln_rec.value <= 0.00)
	{
		cc = abc_delete (feln);
		if (cc) 
			file_err (cc, feln, "DBDELETE");
	}
	else
	{
		cc = abc_update (feln, &feln_rec);
		if (cc) 
			file_err (cc, feln, "DBUPDATE");
	}

	/*
	 * Add/Update feln record for invoice 
	 */
	if (newInv)
	{
		strcpy (feln_rec.index_by, "C");
		feln_rec.hhfe_hash	= hhfe_hash;
		feln_rec.index_hash	= cuin_rec.hhci_hash;
		feln_rec.value		= totInv;
		cc = abc_add (feln, &feln_rec);
		if (cc) 
			file_err (cc, feln, "DBADD");
	}
	else
	{
		strcpy (feln_rec.index_by, "C");
		feln_rec.index_hash = cuin_rec.hhci_hash;
		cc = find_rec (feln, &feln_rec, EQUAL,"u");
		if (cc)
			file_err (cc, feln, "DBFIND");

		feln_rec.value += totInv;
		cc = abc_update (feln, &feln_rec);
		if (cc) 
			file_err (cc, feln, "DBUPDATE");
	}
}

void
AddCucc (
 void)
{
	cucc_rec.hhcu_hash = cohr_rec.hhcu_hash;
	cucc_rec.record_no = 0L;
	sprintf (err_str, mlInvLedup, cohr_rec.inv_no, cohr_rec.pay_terms);
	sprintf (cucc_rec.comment, "%-80.80s", err_str);
	cucc_rec.cont_date = cohr_rec.date_raised;
	strcpy (cucc_rec.hold_flag, "N");
	strcpy (cucc_rec.hold_ref, cohr_rec.inv_no);
	cc = abc_add (cucc, &cucc_rec);
	if (cc) 
		file_err (cc, cucc, "DBADD");
}

/*
 * Add Tax Amount to tax file.
 */
void
AddExst (
 float	percentageTax,
 double	taxAmount,
 double	saleAmount,
 char	*taxCode)
{
	/*
	 * Convert to local values. 
	 */
	if (envDbMcurr && localExchRate != 0.00)
	{
		if (taxAmount != 0.00)
		{
			taxAmount /= localExchRate;
			taxAmount = no_dec (taxAmount);
		}
		if (saleAmount != 0.00)
		{
			saleAmount /= localExchRate;
			saleAmount = no_dec (saleAmount);
		}
	}

	strcpy (exst_rec.co_no,  comm_rec.co_no);
	sprintf (exst_rec.period, "%02d", taxMonth);
	exst_rec.year	=	taxYear;
	exst_rec.tax_percent = percentageTax;
	sprintf (exst_rec.tax_code, "%-1.1s", taxCode);
	cc = find_rec (exst, &exst_rec, COMPARISON, "u");
	/*
	 * Update an Existing Sales Tax Record .
	 */
	if (!cc)
	{
		if (INVOICE)
		{
			exst_rec.sales_value += no_dec (saleAmount);
			exst_rec.tax_value   += no_dec (taxAmount);
		}
		else
		{
			exst_rec.sales_value -= no_dec (saleAmount);
			exst_rec.tax_value   -= no_dec (taxAmount);
		}
		cc = abc_update (exst, &exst_rec);
		if (cc) 
		    	file_err (cc, exst, "DBUPDATE");
	}
	else
	{	
		strcpy (exst_rec.co_no,  comm_rec.co_no);
		sprintf (exst_rec.period, "%02d", taxMonth);
		exst_rec.year	=	taxYear;
		exst_rec.tax_percent = percentageTax;
		sprintf (exst_rec.tax_code, "%-1.1s", taxCode);
		if (INVOICE)
		{
			exst_rec.sales_value = no_dec (saleAmount);
			exst_rec.tax_value   = no_dec (taxAmount);
		}
		else
		{
			exst_rec.sales_value = no_dec (saleAmount * -1);
			exst_rec.tax_value   = no_dec (taxAmount * -1);
		}
		strcpy (exst_rec.stat_flag,"0");
		cc = abc_add (exst, &exst_rec);
		if (cc) 
		       	file_err (cc, exst, "DBADD");

		abc_unlock (exst);
	}
}

/*
 * Report on missing Customer.
 */
int
MissingCustomer (
 long	hhcuHash)
{
	printf ("cohr_inv %s", cohr_rec.inv_no);getchar ();
	sprintf (err_str, ML (mlStdMess258),hhcuHash);
	errmess (err_str);
	sleep (10);
	return (EXIT_FAILURE);
}

void
NoCohrLines (
 void)
{
	/*
	 * character will be printed. 
	 */

	strcpy (cohr_rec.stat_flag, "Z");
	cc = abc_update (cohr, &cohr_rec);
	if (cc) 
		file_err (cc, cohr, "DBUPDATE");
	
	abc_unlock (cohr);
}

/*
 * Routine to open output pipe to standard print to provide an audit trail
 * of events. This also sends the output straight to the spooler.        
 */
void
OpenAuditFile (
 void)
{
	char	tax_name [10];

	strcpy (gljcRec.co_no, comm_rec.co_no);

	if (INVOICE)
		strcpy (gljcRec.journ_type, " 4");
	else
		strcpy (gljcRec.journ_type, " 5");

	cc = find_rec (gljc, &gljcRec, COMPARISON, "r"); 
	if (cc)
		file_err (cc, gljc, "DBFIND");

	gljcRec.run_no++;

	sprintf (tax_name, " %-3.3s  ", gstCode);

	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("so_invledup.p")) == 0) 
		sys_err ("Error in so_invledup.p During (FOPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".14\n");
	fprintf (fout, ".L156\n");
	if (INVOICE)
	{
	       fprintf (fout, ".EINVOICE POSTING AUDIT\n");
	       fprintf (fout, ".B1\n");
	       fprintf (fout, ".EINVOICE RUN NUMBER %010ld\n",gljcRec.run_no);
	}
	else
	{
	       fprintf (fout, ".ECREDIT NOTE POSTING AUDIT\n");
	       fprintf (fout, ".B1\n");
	       fprintf (fout, ".ECREDIT RUN NUMBER %010ld\n",gljcRec.run_no);
	}

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n",clip (comm_rec.co_name));
	fprintf (fout, ".E at at %s\n", SystemTime ());
	if (envDbMcurr)
		fprintf (fout, ".EAll Values Are In Local Currency\n");
	else
		fprintf (fout, ".B1\n");

	fprintf (fout, ".EBranch %s\n", clip (comm_rec.est_name));

	pr_format (fin, fout, "RULEOFF", 0,0);
	pr_format (fin, fout, "RULER", 0,0);
	if (INVOICE)
	{
		pr_format (fin, fout, "HEAD1", 1,"  INVOICE  ");
		pr_format (fin, fout, "HEAD1", 2,tax_name);
		pr_format (fin, fout, "HEAD1", 3,"  INVOICE  ");
	}
	else
	{
		pr_format (fin, fout, "HEAD1", 1,"CREDIT NOTE");
		pr_format (fin, fout, "HEAD1", 2,tax_name);
		pr_format (fin, fout, "HEAD1", 3,"CREDIT NOTE");
	}
	pr_format (fin, fout, "HEAD2", 0,0);
	pr_format (fin, fout, "HEAD3", 0,0);
	fprintf (fout, ".PI12\n");
}

void
PrintLineDetails (
	char	*invoiceNo, 
	char	*debtorNumber,
	char	*debtorName,
	double	amountGross,
	double	amountDisc,
	double	amountNett,
	double	amountFreight,
	double	amountTax,
	double	amountGst,
	double	amountTotal,
	char	*batchNumber)
{
	if (FirstTime == TRUE)
		OpenAuditFile ();

	amountGross   = no_dec (amountGross   / localExchRate);
	amountDisc    = no_dec (amountDisc    / localExchRate);
	amountNett    = no_dec (amountNett    / localExchRate);
	amountFreight = no_dec (amountFreight / localExchRate);
	amountTax     = no_dec (amountTax     / localExchRate);
	amountGst     = no_dec (amountGst     / localExchRate);
	amountTotal   = no_dec (amountTotal   / localExchRate);

	FirstTime = FALSE;

	pr_format (fin, fout, "LINE1", 1, invoiceNo);
	pr_format (fin, fout, "LINE1", 2, debtorNumber);
	pr_format (fin, fout, "LINE1", 3, debtorName);
	pr_format (fin, fout, "LINE1", 4, amountGross);
	pr_format (fin, fout, "LINE1", 5, amountDisc);
	pr_format (fin, fout, "LINE1", 6, amountNett);
	pr_format (fin, fout, "LINE1", 7, amountFreight);
	if (GST)
		pr_format (fin, fout, "LINE1", 8,amountGst);
	else
		pr_format (fin, fout, "LINE1", 8,amountTax);

	pr_format (fin, fout, "LINE1", 9,amountTotal);

	grandTotals [0] += amountGross;
	grandTotals [1] += amountDisc;
	grandTotals [2] += amountNett;
	grandTotals [3] += amountFreight;
	grandTotals [4] += amountTax;
	grandTotals [5] += amountGst;
	grandTotals [6] += amountTotal;
}

/*
 * Print Page footers.
 */
void
PrintFooter (
 void)
{
	pr_format (fin, fout, "TAIL1", 0,0);
	pr_format (fin, fout, "TAIL2", 1,grandTotals [0]);
	pr_format (fin, fout, "TAIL2", 2,grandTotals [1]);
	pr_format (fin, fout, "TAIL2", 3,grandTotals [2]);
	pr_format (fin, fout, "TAIL2", 4,grandTotals [3]);
	if (GST)
		pr_format (fin, fout, "TAIL2", 5,grandTotals [5]);
	else
		pr_format (fin, fout, "TAIL2", 5,grandTotals [4]);

	pr_format (fin, fout, "TAIL2", 6,grandTotals [6]);
	pr_format (fin, fout, "RULER", 0,0);
	fprintf (fout, ".EOF\n");
	pclose (fout);
	fclose (fin);
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*
 * Take out Gst from all the header lines of invoice.
 */
void
SplitGst (
 int	headerSection)
{
    if (headerSection)
    {
		cohr2_rec.gst = 0.00;

		cohr2_rec.gross     	= ExtractGst (cohr_rec.gross,     	 HEAD,POS);
		cohr2_rec.freight   	= ExtractGst (cohr_rec.freight,   	 HEAD,POS);
		cohr2_rec.insurance 	= ExtractGst (cohr_rec.insurance, 	 HEAD,POS);
		cohr2_rec.other_cost_1 	= ExtractGst (cohr_rec.other_cost_1, HEAD,POS);
		cohr2_rec.other_cost_2 	= ExtractGst (cohr_rec.other_cost_2, HEAD,POS);
		cohr2_rec.other_cost_3 	= ExtractGst (cohr_rec.other_cost_3, HEAD,POS);
		cohr2_rec.item_levy 	= ExtractGst (cohr_rec.item_levy, 	 HEAD,POS);
		cohr2_rec.disc      	= ExtractGst (cohr_rec.disc,      	 HEAD,NEG);
		cohr2_rec.ex_disc   	= ExtractGst (cohr_rec.ex_disc,   	 HEAD,NEG);
		cohr2_rec.erate_var 	= ExtractGst (cohr_rec.erate_var, 	 HEAD,POS);
		cohr2_rec.sos       	= ExtractGst (cohr_rec.sos,       	 HEAD,POS);
		cohr2_rec.tax       	= cohr_rec.tax;
    }
    else
    {
		coln2_rec.amt_gst = 0.00;

		coln2_rec.gross     = ExtractGst (coln_rec.gross,      LINE,POS);
		coln2_rec.amt_disc  = ExtractGst (coln_rec.amt_disc,   LINE,NEG);
		coln2_rec.amt_tax   = ExtractGst (coln_rec.amt_tax,    LINE,POS);
		coln2_rec.tax_pc    = coln_rec.tax_pc;
		coln2_rec.gst_pc    = gstPercent;
    }
}

/*
 * Extract Tax amount from values.
 */
double	
ExtractGst (
 double	totalAmount,
 int	headerSection,
 int	positiveNegative)
{
	double	gst_amount = 0.00;

	if (totalAmount == 0)
		return (0.00);

	gst_amount = no_dec (totalAmount / gstDivide);
	
	totalAmount -= no_dec (gst_amount);
	
	if (headerSection)
	{
		if (positiveNegative)
			cohr2_rec.gst += no_dec (gst_amount);
		else
			cohr2_rec.gst -= no_dec (gst_amount);
	}
	else
	{
		if (positiveNegative)
			coln2_rec.amt_gst += no_dec (gst_amount);
		else
			coln2_rec.amt_gst -= no_dec (gst_amount);
	}
	return (totalAmount);
}

/*
 * Normal posting, don't extract Tax.
 */
void
NormalGstPosting (
 int	headerSection)
{
	if (headerSection)
	{
		cohr2_rec.gross     	= no_dec (cohr_rec.gross);
		cohr2_rec.freight   	= no_dec (cohr_rec.freight);
		cohr2_rec.insurance 	= no_dec (cohr_rec.insurance);
		cohr2_rec.other_cost_1 	= no_dec (cohr_rec.other_cost_1);
		cohr2_rec.other_cost_2 	= no_dec (cohr_rec.other_cost_2);
		cohr2_rec.other_cost_3 	= no_dec (cohr_rec.other_cost_3);
		cohr2_rec.item_levy 	= no_dec (cohr_rec.item_levy);
		cohr2_rec.tax       	= no_dec (cohr_rec.tax);
		cohr2_rec.gst       	= no_dec (cohr_rec.gst);
		cohr2_rec.disc      	= no_dec (cohr_rec.disc);
		cohr2_rec.deposit   	= no_dec (cohr_rec.deposit);
		cohr2_rec.ex_disc   	= no_dec (cohr_rec.ex_disc);
		cohr2_rec.erate_var 	= no_dec (cohr_rec.erate_var);
		cohr2_rec.sos       	= no_dec (cohr_rec.sos);
	}
	else
	{
		coln2_rec.tax_pc   	= no_dec (coln_rec.tax_pc);
		coln2_rec.gst_pc   	= no_dec (coln_rec.gst_pc);
		coln2_rec.gross    	= no_dec (coln_rec.gross);
		coln2_rec.amt_disc 	= no_dec (coln_rec.amt_disc);
		coln2_rec.amt_tax  	= no_dec (coln_rec.amt_tax);
		coln2_rec.amt_gst  	= no_dec (coln_rec.amt_gst);
	}
}

/*
 * Add Cheque/Journal header record.
 */
void
AddCuhd (
	char	*chequeType,
	double	chequeAmount,
	char	*receiptReference,
	char	*linkReference,
	long	hhciHash)
{
	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, receiptReference);
	cuhd_rec.index_date	=	cohr_rec.date_raised;
	strcpy (cuhd_rec.type, chequeType);

	sprintf (cuhd_rec.bank_id, "%-5.5s", esmr_rec.dflt_bank);
	cuhd_rec.bank_amt  = no_dec (chequeAmount / bkcr_rec.ex1_factor);
	cuhd_rec.bank_exch = bkcr_rec.ex1_factor;
	cuhd_rec.bank_chg  = 0.00;
	strcpy (cuhd_rec.rec_type, "A");
	strcpy (cuhd_rec.alt_drawer, "                    ");
	cuhd_rec.due_date = 0L;

	cuhd_rec.hhcp_hash = 0L;
	sprintf (cuhd_rec.narrative, "APP %s%s",receiptReference,linkReference);
	cuhd_rec.date_payment  = cohr_rec.date_raised;
	cuhd_rec.date_posted   = cohr_rec.date_raised;
	cuhd_rec.tot_amt_paid  = chequeAmount;
	cuhd_rec.loc_amt_paid  = no_dec (chequeAmount / cohr_rec.exch_rate);
	cuhd_rec.disc_given    = 0.00;
	cuhd_rec.loc_disc_give = 0.00;
	strcpy (cuhd_rec.stat_flag, "0");
	cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (cuhd);
		cc = abc_add (cuhd, &cuhd_rec);
		if (cc)
			file_err (cc, cuhd, "DBADD");

		cc = find_rec (cuhd , &cuhd_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cuhd, "DBFIND");
	}
	else
	{
		cuhd_rec.tot_amt_paid += no_dec (chequeAmount);
		cuhd_rec.loc_amt_paid += no_dec (chequeAmount / cohr_rec.exch_rate);
		cc = abc_update (cuhd, &cuhd_rec);
	}
	AddCudt (cuhd_rec.hhcp_hash, chequeAmount, hhciHash);
}

/*
 * Add Cheque/Journal detail record.
 */
void
AddCudt (
	long	hhcpHash,
	double	detailAmount,
	long	hhciHash)
{
	cudt_rec.hhcp_hash    = hhcpHash;
	cudt_rec.hhci_hash    = hhciHash;
	cudt_rec.amt_paid_inv = no_dec (detailAmount);
	cudt_rec.loc_paid_inv = no_dec (detailAmount / cohr_rec.exch_rate);
	cudt_rec.exch_rate    = 0.00;
	strcpy (cudt_rec.stat_flag, "0");
	cc = abc_add (cudt, &cudt_rec);
	if (cc)
		file_err (cc, cudt, "DBADD");

	if (envSaCommission)
	{
		AddCommission
		(
			hhciHash, 
			hhcpHash,
			cudt_rec.loc_paid_inv 
		);
	}
}

/*
 * Add record to background processing file.
 */
void
AddSobg (
	int		_printerNumber,
	char	*_processTypeFlag,
	long	_hash)
{
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _processTypeFlag);
	sobg_rec.lpno = _printerNumber;
	sobg_rec.hash = _hash;

	cc = find_rec (sobg,&sobg_rec,COMPARISON,"r");
	/*
	 * Add the record iff an identical one doesn't already exist	
	 */
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _processTypeFlag);
		sobg_rec.lpno = _printerNumber;
		sobg_rec.hash = _hash;

		cc = abc_add (sobg, &sobg_rec);
		if (cc)
			file_err (cc, sobg, "DBADD");
	}
}

/*
 * Add Commission record for receipts made against an invoice.
 */
void
AddCommission (
	long	hhciHash,
	long	hhcpHash,
	double	paymentAmount)
{
	double	PayPercent = 0.00,
			CommAmt    = 0.00;

	paymentAmount	=	no_dec (paymentAmount);

	sach_rec.hhci_hash = hhciHash;
	cc = find_rec (sach, &sach_rec, GTEQ, "r");
	while (!cc && sach_rec.hhci_hash == hhciHash)
	{
		PayPercent = no_dec (paymentAmount / sach_rec.inv_amt);
		CommAmt    = no_dec (sach_rec.com_val * PayPercent);
	
		sacl_rec.sach_hash	=	sach_rec.sach_hash;
		sacl_rec.hhcp_hash	=	hhcpHash;
		sacl_rec.rec_amt	=	paymentAmount;
		sacl_rec.rec_date	=	longSystemDate;
		sacl_rec.com_amt	=	CommAmt;
		strcpy (sacl_rec.status, "0");

		cc = find_rec (sacl, &sacl_rec, COMPARISON, "r");
		if (cc)
		{
			sacl_rec.rec_amt	=	paymentAmount;
			sacl_rec.rec_date	=	longSystemDate;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_add (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBADD");
		}
		else
		{
			sacl_rec.rec_amt	=	paymentAmount;
			sacl_rec.rec_date	=	longSystemDate;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_update (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBUPDATE");
		}
		cc = find_rec (sach, &sach_rec, NEXT, "r");
	}
}

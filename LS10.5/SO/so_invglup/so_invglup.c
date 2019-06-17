/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_invglup.c,v 5.6 2001/08/26 22:46:37 scott Exp $
|  Program Name  : (so_invglup.c)                                  |
|  Program Desc  : (Invoice / Credit note update to G/Ledger. )   |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 10/05/1986       |
|---------------------------------------------------------------------|
| $Log: so_invglup.c,v $
| Revision 5.6  2001/08/26 22:46:37  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 5.5  2001/08/24 05:59:13  scott
| Updated from scotts machine
|
| Revision 5.3  2001/08/20 23:45:32  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 09:21:19  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:21  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_invglup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invglup/so_invglup.c,v 5.6 2001/08/26 22:46:37 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<ml_std_mess.h>
#include 	<ml_gl_mess.h>
#define		INVOICE	 (transactionTypeFlag [0] == 'I' || \
					 transactionTypeFlag [0] == 'P')

#define		NOTAX	 (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')

#include 	<twodec.h>

#define MAXILINES	8000
#define	WK		cuwkRec [numberRecords]

#define	FAULTY (coln_rec.crd_type [0] == 'F' || \
		  		  coln_rec.crd_type [0] == 'D')

#define	COST_SALES (envVar.costSales [0] == 'Y')

/*
 * NB. If COST_SALES == TRUE then cost of sales are posted 
 * directly to glwk (database file not work file) to be   
 * processed later by gl_format etc.                     
 * ALL other postings are added to cuwk for later processing
 * by db_winvtogl.                                         
 */

#include	"schema"

struct cncdRecord	cncd_rec;
struct cnchRecord	cnch_rec;
struct cnreRecord	cnre_rec;
struct curhRecord	curh_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

/*
 * Customer Invoice/Credits Work File.
 */
#define	CUWK_NO_FIELDS	33

	struct dbview	cuwk_list [CUWK_NO_FIELDS] =
	{
		{"cuwk_co_no"},
		{"cuwk_est"},
		{"cuwk_inv_no"},
		{"cuwk_type"},
		{"cuwk_dbt_no"},
		{"cuwk_date_of_inv"},
		{"cuwk_post_date"},
		{"cuwk_exch_rate"},
		{"cuwk_disc"},
		{"cuwk_cus_po_no"},
		{"cuwk_tax"},
		{"cuwk_freight"},
		{"cuwk_insurance"},
		{"cuwk_other_cost1"},
		{"cuwk_other_cost2"},
		{"cuwk_other_cost3"},
		{"cuwk_dd_oncost"},
		{"cuwk_exch_var"},
		{"cuwk_sos"},
		{"cuwk_restock_fee"},
		{"cuwk_item_levy"},
		{"cuwk_gst"},
		{"cuwk_narrative"},
		{"cuwk_tot_loc"},
		{"cuwk_tot_fx"},
		{"cuwk_gl_levy"},
		{"cuwk_gl_control"},
		{"cuwk_gl_acc_no"},
		{"cuwk_period_no"},
		{"cuwk_loc_amt"},
		{"cuwk_fx_amt"},
		{"cuwk_currency"},
		{"cuwk_stat_flag"}
	};

	struct tag_cuwkRecord
	{
		char	co_no [3];
		char	est [3];
		char	inv_no [9];
		char	type [2];
		char	dbt_no [7];
		Date	date_of_inv;
		Date	post_date;
		double	exch_rate;
		Money	disc;
		char	cus_po_no [17];
		Money	tax;
		Money	freight;
		Money	insurance;
		Money	other_cost1;
		Money	other_cost2;
		Money	other_cost3;
		Money	dd_oncost;
		Money	exch_var;
		Money	sos;
		Money	restock_fee;
		Money	item_levy;
		Money	gst;
		char	narrative [21];
		Money	tot_loc;
		Money	tot_fx;
		char	gl_levy [17];
		char	gl_control [17];
		char	gl_acc_no [17];
		char	period_no [3];
		Money	loc_amt;
		Money	fx_amt;
		char	currency [4];
		char	stat_flag [2];
	} cuwkRec [MAXILINES];

	char	*data = "data",
			*cuwk = "cuwk";


	long	lsystemDate		= 0L,
			salesDate  		= 0L,
			costSalesDate	= 0L,
			startMonthDate 	= 0L,
			monthEndDate  	= 0L,
			maximumMendDate	= 0L,
			suspenseHash  	= 0L;

	char	transactionTypeFlag [2],
			findStatusFlag [2],
			updateStatusFlag [2],
			passedBranchNumber [3],
			suspenseAccount [MAXLEVEL + 1];

/*
 * The structure envVar groups the values of
 * environment settings together.           
 */
struct tagEnvVar
{
	float	gstInclude;
	float	gstDivide;
	char	saleDateType [8];
	char	csaleDateType [8];
	char	costSales [2];
	int 	SO_REBATE_USED;
	int 	DB_NETT;
	int		CN_NETT;
	int		GL_ONCOST;
	int		GL_BYCLASS;
	int		DISCOUNT_OK;
} envVar;


/*
 * Function Declarations
 */
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ProcessHeader 		 (void);
long 	MakeADate 			 (char *, int);
void 	ProcessLines 		 (long);
void 	AddGeneralLedger 	 (void);
void 	SplitOutGst 		 (void);
double 	OutGst 				 (double, int, int);
void 	NormalGstProcessing (void);
void 	PostRebate 			 (void);
void 	AddCurh 			 (void);
void 	CheckEnvironment 	 (void);


/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	if (argc < 4)
	{
		print_at (0,0, "Usage : %s <findStatusFlag> <updateStatusFlag> <transactionTypeFlag> <Optional - branchNumber>",argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();

	OpenDB ();

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	/*
	 * Get beginning of Current Month.
	 */
	startMonthDate = MonthStart (comm_rec.dbt_date);

	/*
	 * Get end of Current Month.
	 */
	monthEndDate   = MonthEnd (comm_rec.dbt_date);

	/*
	 * Get end of Current month + 1
	 */
	maximumMendDate  = MonthEnd (monthEndDate + 1L);

	sprintf (findStatusFlag, 	  "%-1.1s", argv [1]);
	sprintf (updateStatusFlag, 	  "%-1.1s", argv [2]);
	sprintf (transactionTypeFlag, "%-1.1s", argv [3]);

	if (argc == 5)
		strcpy (passedBranchNumber, argv [4]);
	else
		strcpy (passedBranchNumber, comm_rec.est_no);

	init_scr ();
	set_tty ();

	sprintf (err_str,
		" Posting %s to General ledger.",
		 (INVOICE) ? "Invoices" : "Credits");
	print_mess (err_str);
	
	lsystemDate = TodaysDate ();

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		"  ",
		"SUSPENSE  ",
		" ",
		" "
	);
	strcpy (suspenseAccount, glmrRec.acc_no);
	suspenseHash = glmrRec.hhmr_hash;

	ProcessHeader ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files.
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cuwk, cuwk_list, CUWK_NO_FIELDS, "cuwk_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no2");
	open_rec (cnre, cnre_list, CNRE_NO_FIELDS, "cnre_id_no");
	open_rec (curh, curh_list, CURH_NO_FIELDS, "curh_hhbr_hash");

	OpenGlwk ();
	OpenGlmr ();
}

/*
 * Close Database files.
 */
void
CloseDB (
 void)
{
	abc_fclose (comr);
	abc_fclose (cumr);
	abc_fclose (ccmr);
	abc_fclose (cuwk);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inmr);
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (cnre);
	abc_fclose (curh);
	GL_Close ();

	abc_dbclose (data);
}

/*
 * Process whole coln file looking for stat_flag = findStatusFlag
 * and updating inmr record appropriately.                      
 * Returns: 0 if ok, non-zero if not ok.                       
 */
void
ProcessHeader (
 void)
{
	/*
	 * Read whole cohr file.
	 */
	while (1) 
	{
		strcpy (cohr_rec.co_no,     comm_rec.co_no);
		strcpy (cohr_rec.br_no,     passedBranchNumber);
		strcpy (cohr_rec.type,      transactionTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatusFlag);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc) 
		{
			abc_unlock ("cohr");
			break;
		}
		
		if (envVar.gstDivide != 0.00 && !NOTAX)
			SplitOutGst ();
		else
			NormalGstProcessing ();

		salesDate  		= MakeADate (envVar.saleDateType,  TRUE);
		costSalesDate 	= MakeADate (envVar.csaleDateType, FALSE);

		/*
		 * Save current stat flag.
		 */
		ProcessLines (cohr_rec.hhco_hash);
	}
	abc_unlock ("cohr");
}

/*
 * Get Correct type of date for Postings.
 */
long	
MakeADate (
 char	*dateUse,
 int	sales)
{
	/*
	 * Post using Stock Date.
	 */
	if (!strncmp (dateUse, "SK_DATE", 7))
		return (comm_rec.inv_date);

	/*
	 * Post using Customer Date.
	 */
	if (!strncmp (dateUse, "DB_DATE", 7))
		return (comm_rec.dbt_date);

	/*
	 * Post using G/L Date.
	 */
	if (!strncmp (dateUse, "GL_DATE", 7))
		return (comm_rec.gl_date);

	/*
	 * Post using Invoice Date.
	 */
	if (!strncmp (dateUse, "IN_DATE", 7))
	{
		if (cohr_rec.date_raised < startMonthDate)
			return (comm_rec.dbt_date);
	
		if (cohr_rec.date_raised > maximumMendDate)
			return (maximumMendDate);
 
		return (cohr_rec.date_raised);
	}
	if (sales)
	{
		/*
		 * Default to Customer Date.
		 */
		return (comm_rec.dbt_date);
	}

	return (comm_rec.inv_date);
}

/*
 * Procesess All Customer Invoice Lines.
 */
void
ProcessLines (
 long	hhcoHash)
{                                   /*===================================*/
	int		i = 0;					/*| Counts no of items read in.     |*/
	int		numberRecords = 0;		/*| Counts no. of cuwk records.     |*/
                                    /*|---------------------------------|*/
	double  totalInvoice	= 0.00; /*| Total Invoice.                  |*/
	double  totalDiscount	= 0.00; /*| Total Discount.                 |*/
	double  totalOnCost		= 0.00; /*| Total On-Costs.                 |*/
	 								/*|                                 |*/
	double	periodAmount	= 0.00;	/*| Period Amount for Each Line.    |*/
	double	itemLevy		= 0.00;	/*| Item Levy for each line.        |*/
	double	grossAmount		= 0.00;	/*| Gross Amount for Line.          |*/
	double	onCost			= 0.00;	/*| On-Cost Amount for Line.        |*/
	double	discAmount		= 0.00;	/*| Discount Amount for Line.       |*/
	double	exDiscAmt		= 0.00;	/*| Extra Discount applied to Line. |*/
	 								/*|                                 |*/
	double  diff_calc = 0.00;		/*| Calculated per line.            |*/
	double	nett_amt  = 0.00;		/*| Nett Amount per line.           |*/
	double	diff_total = 0.00;
                                    /*===================================*/

	int		monthPeriod;

	cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	if (cohr_rec.exch_rate == 0.0)
		cohr_rec.exch_rate	=	1.00;

	/*
	 * Process all order lines for current cohr record .
	 */
	coln_rec.hhco_hash = hhcoHash;
	coln_rec.line_no = 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	while (!cc && numberRecords < MAXILINES && hhcoHash == coln_rec.hhco_hash)
	{
	   	/*
	   	 * Create cuwk record.
	   	 */
	   	strcpy (WK.co_no, comm_rec.co_no);
	   	strcpy (WK.est, cohr_rec.br_no);
	   	strcpy (WK.inv_no, cohr_rec.inv_no);
	   	strcpy (WK.type, (INVOICE) ? "1" : "2");

	   	strcpy (WK.dbt_no, cumr_rec.dbt_no);

	   	WK.date_of_inv 	= salesDate;
	   	WK.post_date 	= lsystemDate;
		
		/*
		 * Gst is inclusive in price so split it out.
		 */
		if (envVar.gstDivide != 0.00 && !NOTAX)
		{
			grossAmount = OutGst (no_dec (coln_rec.gross), 		FALSE, FALSE);
			discAmount  = OutGst (no_dec (coln_rec.amt_disc) , 	FALSE, FALSE);
			itemLevy	= OutGst (no_dec (coln_rec.item_levy), 	FALSE, FALSE);
			if (numberRecords == 0)
				exDiscAmt	= OutGst (no_dec (cohr_rec.ex_disc),FALSE, FALSE);
			else
				exDiscAmt	= 0.00;
		}
		/*
		 * Gst is exclusive in price. 
		 */
		else
		{
			grossAmount	 = no_dec (coln_rec.gross);
			discAmount 	 = no_dec (coln_rec.amt_disc);
			itemLevy 	 = no_dec (coln_rec.item_levy);
			if (numberRecords == 0)
				exDiscAmt	= no_dec (cohr_rec.ex_disc);
			else
				exDiscAmt	= 0.00;
		}
		
		if (INVOICE)
		{
			if (envVar.DISCOUNT_OK || !envVar.DB_NETT) 
				periodAmount = grossAmount;
			else
				periodAmount = (grossAmount - discAmount - exDiscAmt);
		}
		else
		{
			if (envVar.DISCOUNT_OK || !envVar.CN_NETT) 
				periodAmount = grossAmount;
			else
				periodAmount = (grossAmount - discAmount - exDiscAmt);
		}
			
		if (cohr_rec.drop_ship [0] == 'Y' && envVar.GL_ONCOST)
		{
			onCost  =	no_dec (coln_rec.on_cost * (double) coln_rec.q_order);
			onCost	*=	cohr_rec.exch_rate;
			onCost	=	no_dec (onCost);
		}
		else
			onCost  = 0.00;

		periodAmount	-= onCost;
		totalOnCost		+= onCost;

		if (INVOICE)
		{
			WK.freight		= no_dec (cohr2_rec.freight);
			WK.item_levy	= no_dec (itemLevy);
			WK.insurance	= no_dec (cohr2_rec.insurance);
			WK.other_cost1	= no_dec (cohr2_rec.other_cost_1);
			WK.other_cost2	= no_dec (cohr2_rec.other_cost_2);
			WK.other_cost3	= no_dec (cohr2_rec.other_cost_3);
			WK.sos			= no_dec (cohr2_rec.sos);
			WK.restock_fee 	= 0.00;
    		WK.fx_amt 		= no_dec (periodAmount);
    		WK.loc_amt 		= no_dec (periodAmount / cohr_rec.exch_rate);
    		WK.loc_amt 		= no_dec (WK.loc_amt);
    		WK.dd_oncost	= no_dec (onCost);
    		WK.exch_var 	= no_dec (cohr2_rec.erate_var);
		}
		else
		{
			/*
			 * Reverse Amount for Credit.
			 */
			WK.item_levy	= no_dec (itemLevy)					* -1;
			WK.freight		= no_dec (cohr2_rec.freight)		* -1;
			WK.insurance	= no_dec (cohr2_rec.insurance)		* -1;
			WK.other_cost1	= no_dec (cohr2_rec.other_cost_1)	* -1;
			WK.other_cost2	= no_dec (cohr2_rec.other_cost_2)	* -1;
			WK.other_cost3	= 0.00;
			WK.restock_fee 	= no_dec (cohr_rec.other_cost_3)	* -1;
	    	WK.fx_amt 		= no_dec (periodAmount)				* -1;
    		WK.loc_amt 		= no_dec (periodAmount / cohr_rec.exch_rate);
			WK.loc_amt		= no_dec (WK.loc_amt) 				* -1;
    		WK.dd_oncost	= onCost 							* -1;
    		WK.exch_var 	= no_dec (cohr2_rec.erate_var) 		* -1;
		}
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			inmr_rec.outer_size = 0.00;
			strcpy (inmr_rec.category, "DELETED    ");
		}
		if (!cc && inmr_rec.hhsi_hash != 0L)
		{
			inmr_rec.hhbr_hash = inmr_rec.hhsi_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			inmr_rec.outer_size = 0.00;
			strcpy (inmr_rec.category, "DELETED    ");
		}
		strcpy (WK.stat_flag,"0");

		ccmr_rec.hhcc_hash = coln_rec.incc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		/*
		 * Get interface code for general ledger account receivable account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"ACCT REC  ",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (WK.gl_control, glmrRec.acc_no);

		/*
		 * Get interface code for general ledger account item levy account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"ITEM LEVY ",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (WK.gl_levy, glmrRec.acc_no);

		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			(INVOICE) ? "INVOICE   " : "CREDIT    ",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (WK.gl_acc_no, glmrRec.acc_no);

		DateToDMY (salesDate, NULL, &monthPeriod, NULL);
		sprintf (WK.period_no, "%02d", monthPeriod);
		numberRecords++;

		strcpy (coln_rec.stat_flag,updateStatusFlag);
		cc = abc_update (coln, &coln_rec);
		if (cc) 
			file_err (cc, "coln", "DBFIND");

		/*
		 * if coln line has contract
		 * number on it the post to 
		 * to cnre (contract rebate)
		 */
		if (coln_rec.cont_status && envVar.SO_REBATE_USED)
			PostRebate ();
		
		/*
		 * add curh rebate different from above
		 */
		if (envVar.SO_REBATE_USED)
			AddCurh ();

		if (!FAULTY && coln_rec.q_order != 0.0)
			AddGeneralLedger ();

		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}	
	abc_unlock (coln);

	/*
	 * Work Out Total Amount of invoice.
	 */
	totalInvoice	=	no_dec 
						 (
							cohr2_rec.gross + 
							cohr2_rec.item_levy + 
							cohr2_rec.tax +
							cohr2_rec.gst + 
							cohr2_rec.freight + 
							cohr2_rec.insurance + 
							cohr2_rec.other_cost_1 + 
							cohr2_rec.other_cost_2 +
							cohr2_rec.other_cost_3 + 
							cohr2_rec.sos 
						);
	
	totalDiscount 		= no_dec (cohr2_rec.disc + cohr2_rec.ex_disc);
	totalInvoice 		= no_dec (totalInvoice);
	totalOnCost 		= no_dec (totalOnCost);

	/*
	 * Store Amount lines should add upto.
	 */
	if (INVOICE)
	{
		if (envVar.DISCOUNT_OK || !envVar.DB_NETT)
			nett_amt = cohr2_rec.gross;
		else
			nett_amt = (cohr2_rec.gross - totalDiscount);
	}
	else
	{
		if (envVar.DISCOUNT_OK || !envVar.CN_NETT)
			nett_amt = (cohr2_rec.gross) * -1; 
		else
			nett_amt = (cohr2_rec.gross - totalDiscount) * -1;
	}

	nett_amt -= totalOnCost;
	nett_amt = no_dec (nett_amt);

	/*
	 * Update amount in each record. 
	 */
	for (i = 0;i < numberRecords; i++) 	
	{	 
    	if (INVOICE)
    	{
			if (envVar.DISCOUNT_OK || !envVar.DB_NETT) 
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice);
			else
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice - totalDiscount);
			cuwkRec [i].tot_loc		=  	cuwkRec [i].tot_fx / cohr_rec.exch_rate;
			cuwkRec [i].tot_loc		=  	no_dec (cuwkRec [i].tot_loc);
			cuwkRec [i].disc 		= 	no_dec (totalDiscount);
			cuwkRec [i].dd_oncost	= 	no_dec (totalOnCost);
			cuwkRec [i].tax 	 	=  	no_dec (cohr2_rec.tax);
			cuwkRec [i].gst 	 	=  	no_dec (cohr2_rec.gst);
    	}
    	else
    	{
			if (envVar.DISCOUNT_OK || !envVar.CN_NETT) 
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice);
			else
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice - totalDiscount);
			cuwkRec [i].tot_fx 		= 	cuwkRec [i].tot_fx * -1;
			cuwkRec [i].tot_fx 		= 	no_dec (cuwkRec [i].tot_fx);

			if (envVar.DISCOUNT_OK || !envVar.CN_NETT) 
				cuwkRec [i].tot_loc =  	no_dec (totalInvoice);
			else
				cuwkRec [i].tot_loc =  	no_dec (totalInvoice - totalDiscount);
			cuwkRec [i].tot_loc	=  	cuwkRec [i].tot_loc / cohr_rec.exch_rate;
			cuwkRec [i].tot_loc		=  	no_dec (cuwkRec [i].tot_loc) * -1;

			cuwkRec [i].disc 	 	= 	no_dec (totalDiscount)		* -1;
			cuwkRec [i].dd_oncost 	= 	no_dec (totalOnCost) 		* -1;
			cuwkRec [i].tax 	 	=  	no_dec (cohr2_rec.tax) 		* -1;
			cuwkRec [i].gst 	 	=  	no_dec (cohr2_rec.gst) 		* -1;
    	}
	
    	/*
    	 * Store Amount for each line.
    	 */
    	diff_calc += no_dec (cuwkRec [i].fx_amt);
	
		sprintf (cuwkRec [i].cus_po_no, "%04d%8.8s",i,cohr_rec.inv_no);
		sprintf (cuwkRec [i].narrative,"%6.6s /%3.3s %8.8s",
						cumr_rec.dbt_no, (INVOICE) ? "INV" : "CRD", 
						cohr_rec.inv_no);

    	/*
    	 * Check For Diff and add to line if required.
    	 */
    	if ((i + 1) == numberRecords)
		{
			if ((nett_amt - diff_calc) >  1.00 || 
             (nett_amt - diff_calc) < -1.00)
			{
				sprintf (cuwkRec [i].narrative, "* DIFF %11.2f *",
											DOLLARS (nett_amt - diff_calc));

				diff_total = nett_amt - diff_calc;
				cuwkRec [i].fx_amt += no_dec (diff_total);

				diff_total = (nett_amt - diff_calc);
				cuwkRec [i].loc_amt += no_dec (diff_total);
				cuwkRec [i].loc_amt /= cohr_rec.exch_rate;
				cuwkRec [i].loc_amt = no_dec (cuwkRec [i].loc_amt);
			}	
			sprintf (cuwkRec [i].cus_po_no, "%04d%8.8sEND",i,cohr_rec.inv_no);
		}

		cuwkRec [i].exch_rate = cohr_rec.exch_rate;
	
		strcpy (cuwkRec [i].currency, cumr_rec.curr_code); 
	
    	cc = abc_add (cuwk, &cuwkRec [i]);
    	if (cc)
			file_err (cc, "cuwk", "DBADD");
	}

	/*
	 * Update amount if no coln record
	 */
	if (numberRecords == 0)  
	{	 
	   	/*
	   	 * Create cuwk record.
	   	 */
	   	strcpy (WK.co_no, comm_rec.co_no);
	   	strcpy (WK.est, cohr_rec.br_no);
	   	strcpy (WK.inv_no, cohr_rec.inv_no);
	   	strcpy (WK.type, (INVOICE) ? "1" : "2");

	   	strcpy (WK.dbt_no, cumr_rec.dbt_no);

	   	WK.date_of_inv	= salesDate;
	   	WK.post_date 	= lsystemDate;

		if (INVOICE)
		{
			WK.item_levy	= no_dec (cohr2_rec.item_levy);
			WK.freight		= no_dec (cohr2_rec.freight);
			WK.insurance	= no_dec (cohr2_rec.insurance);
			WK.other_cost1	= no_dec (cohr2_rec.other_cost_1);
			WK.other_cost2	= no_dec (cohr2_rec.other_cost_2);
			WK.other_cost3	= no_dec (cohr2_rec.other_cost_3);
			WK.sos			= no_dec (cohr2_rec.sos);

			WK.restock_fee 	= 0.00;
    		WK.fx_amt 		= no_dec (periodAmount);
    		WK.loc_amt 		= no_dec (periodAmount / cohr_rec.exch_rate);
    		WK.loc_amt 		= no_dec (WK.loc_amt);
    		WK.dd_oncost	= no_dec (onCost);
    		WK.exch_var 	= no_dec (cohr2_rec.erate_var);
		}
		else
		{
			/*
			 * Reverse Amount for Credit.
			 */
			WK.item_levy	= no_dec (cohr2_rec.item_levy) 		* -1;
			WK.freight		= no_dec (cohr2_rec.freight)		* -1;
			WK.insurance	= no_dec (cohr2_rec.insurance)		* -1;
			WK.other_cost1 	= no_dec (cohr2_rec.other_cost_1)	* -1;
			WK.other_cost2 	= no_dec (cohr2_rec.other_cost_2)	* -1;
			WK.other_cost3 	= 0.00;
			WK.sos			= no_dec (cohr2_rec.sos)			* -1;
			WK.restock_fee 	= no_dec (cohr_rec.other_cost_3)	* -1;
	    	WK.fx_amt 		= no_dec (periodAmount)				* -1;
    		WK.loc_amt 		= no_dec (periodAmount / cohr_rec.exch_rate);
			WK.loc_amt		= no_dec (WK.loc_amt) 				* -1;
    		WK.dd_oncost	= no_dec (onCost) 					* -1;
    		WK.exch_var 	= no_dec (cohr2_rec.erate_var) 		* -1;
		}

		/*
		 * Get interface code for general ledger account receivable account.
		 */
		GL_GLI 
		(
			cohr_rec.co_no,
			cohr_rec.br_no,
			"  ",
			"ACCT REC  ",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (WK.gl_control, glmrRec.acc_no);

		/*
		 * Get interface code for invoice / credit account. 
		 */
		GL_GLI 
		(
			cohr_rec.co_no,
			cohr_rec.br_no,
			"  ",
			(INVOICE) ? "INVOICE   " : "CREDIT    ",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (WK.gl_acc_no,      glmrRec.acc_no);

		DateToDMY (salesDate, NULL, &monthPeriod, NULL);
		sprintf (WK.period_no, "%02d", monthPeriod);
		strcpy (WK.stat_flag,"0");
		
    	if (INVOICE)
    	{
			if (envVar.DISCOUNT_OK || !envVar.DB_NETT) 
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice);
			else
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice - totalDiscount);
			cuwkRec [i].tot_loc		=  	no_dec (cuwkRec [i].tot_fx / cohr_rec.exch_rate);
			cuwkRec [i].tot_loc		=  	no_dec (cuwkRec [i].tot_loc);
			cuwkRec [i].disc 	 	= 	no_dec (totalDiscount);
			cuwkRec [i].dd_oncost	= 	no_dec (totalOnCost);
			cuwkRec [i].tax 	 	=  	no_dec (cohr2_rec.tax);
			cuwkRec [i].gst 	 	=  	no_dec (cohr2_rec.gst);
    	}
    	else
    	{
			if (envVar.DISCOUNT_OK || !envVar.CN_NETT) 
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice) * -1;
			else
				cuwkRec [i].tot_fx 	=  	no_dec (totalInvoice - totalDiscount) * -1;
			if (envVar.DISCOUNT_OK || !envVar.CN_NETT) 
				cuwkRec [i].tot_loc =  	no_dec 
										(
											cuwkRec [i].tot_fx /
											cohr_rec.exch_rate
										);
			else
				cuwkRec [i].tot_loc =  	no_dec 
										(
											(totalInvoice - totalDiscount) / 
											cohr_rec.exch_rate
										);

			cuwkRec [i].disc 	 	= 	no_dec (totalDiscount) 		* -1;
			cuwkRec [i].dd_oncost 	= 	no_dec (totalOnCost) 		* -1;
			cuwkRec [i].tax 	 	=  	no_dec (cohr2_rec.tax) 		* -1;
			cuwkRec [i].gst 	 	=  	no_dec (cohr2_rec.gst) 		* -1;
    	}
	

    	/*
    	 * Store Amount for each line.
    	 */
    	diff_calc = no_dec (cuwkRec [i].fx_amt);
	
		sprintf (cuwkRec [i].cus_po_no, "%04d%8.8s",i,cohr_rec.inv_no);
		sprintf (cuwkRec [i].narrative,"%6.6s /%3.3s %8.8s",
						cumr_rec.dbt_no, (INVOICE) ? "INV" : "CRD", 
						cohr_rec.inv_no);

    	/*
    	 * Check For Diff and add to line if required.
    	 */
		sprintf (cuwkRec [i].narrative, "* DIFF %11.2f *",
							DOLLARS (nett_amt - diff_calc));

		diff_total = no_dec (nett_amt - diff_calc);
		cuwkRec [i].fx_amt += no_dec (diff_total);

		diff_total = no_dec ((nett_amt - diff_calc));
		cuwkRec [i].loc_amt += no_dec (diff_total / cohr_rec.exch_rate);

		sprintf (cuwkRec [i].cus_po_no, "%04d%8.8sEND",i,cohr_rec.inv_no);

		cuwkRec [i].exch_rate = cohr_rec.exch_rate;
	
		strcpy (cuwkRec [i].currency, cumr_rec.curr_code); 
	
    	cc = abc_add (cuwk, &cuwkRec [i]);
    	if (cc)
			file_err (cc, "cuwk", "DBADD");
	}

	strcpy (cohr_rec.stat_flag, updateStatusFlag);
	cc = abc_update (cohr, &cohr_rec);
	if (cc) 
		file_err (cc, "cohr", "DBUPDATE");
}

/*
 * Add transactions to glwk file.
 */
void
AddGeneralLedger (
 void)
{
	double	wk_value;
	int		monthPeriod;

	if (!COST_SALES)
		return;

	strcpy (glwkRec.co_no,cohr_rec.co_no);
	strcpy (glwkRec.est_no,cohr_rec.br_no);
	strcpy (glwkRec.tran_type,"13");
	glwkRec.post_date = lsystemDate;
	glwkRec.tran_date = costSalesDate;
	DateToDMY (costSalesDate, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no,"%02d", monthPeriod);
	sprintf (glwkRec.sys_ref,"%5.1d",comm_rec.term);
	sprintf (glwkRec.user_ref,"%8.8s",cohr_rec.inv_no);
	strcpy (glwkRec.stat_flag,"2");

	if (INVOICE)
		sprintf (glwkRec.narrative,"Sales from   W.H. %s",
							ccmr_rec.cc_no);
	else
		sprintf (glwkRec.narrative,"Credit to    W.H. %s",
							ccmr_rec.cc_no);

	strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");

	inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		inmr_rec.outer_size = 1.00;

	if (!cc && inmr_rec.hhsi_hash != 0L)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
	}
	if (cc)
		inmr_rec.outer_size = 1.00;

	if (cohr_rec.drop_ship [0] == 'Y' && envVar.GL_ONCOST)
	{
		/*
		 * Debit on-costs to Direct Delivery On-Costs Debit a/c
		 */
		wk_value = out_cost (coln_rec.on_cost, inmr_rec.outer_size);
		glwkRec.amount 		= ((double) coln_rec.q_order * wk_value);
		glwkRec.loc_amount 	= ((double) coln_rec.q_order * wk_value);

		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"DD CST CST",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (glwkRec.acc_no, glmrRec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;

		strcpy (glwkRec.currency, comr_rec.base_curr);
		glwkRec.exch_rate = 1.00;

		cc = abc_add (glwk, &glwkRec);
		if (cc) 
			file_err (cc, "glwk", "DBADD");

		/*
		 * Post COS exclusive of on-costs normally 
		 */
		wk_value = out_cost ((coln_rec.cost_price - coln_rec.on_cost), 
							 inmr_rec.outer_size);

		glwkRec.amount 		= ((double) coln_rec.q_order * wk_value);
		glwkRec.loc_amount 	= ((double) coln_rec.q_order * wk_value);

		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"COSTSALE D",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (glwkRec.acc_no, glmrRec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		
	}
	else
	{
		wk_value = out_cost (coln_rec.cost_price, inmr_rec.outer_size);
		glwkRec.amount 		= ((double) coln_rec.q_order * wk_value);
		glwkRec.loc_amount 	= ((double) coln_rec.q_order * wk_value);

		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"COSTSALE D",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (glwkRec.acc_no, glmrRec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	}

	strcpy (glwkRec.currency, comr_rec.base_curr);
	glwkRec.exch_rate = 1.00;
	cc = abc_add (glwk, &glwkRec);
	if (cc) 
		file_err (cc, "glwk", "DBADD");

	wk_value = out_cost (coln_rec.cost_price, inmr_rec.outer_size);
	glwkRec.amount 		= ((double) coln_rec.q_order * wk_value);
	glwkRec.loc_amount 	= ((double) coln_rec.q_order * wk_value);

	if (cohr_rec.drop_ship [0] == 'Y')
	{
		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"PO CLEAR D",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (glwkRec.acc_no, glmrRec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		
	}
	else
	{
		/*
		 * Get interface code for invoice / credit account.
		 */
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"COSTSALE C",
			(envVar.GL_BYCLASS) ? cumr_rec.class_type : cumr_rec.sman_code,
			inmr_rec.category
		);
		strcpy (glwkRec.acc_no, glmrRec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	}

	strcpy (glwkRec.jnl_type, (INVOICE) ? "2" : "1");

	strcpy (glwkRec.currency, comr_rec.base_curr);
	glwkRec.exch_rate = 1.00;

	cc = abc_add (glwk, &glwkRec);
	if (cc) 
		file_err (cc, glwk, "DBADD");

	return;
}

/*
 * Take out Gst from all the header lines of invoice.
 */
void
SplitOutGst (
 void)
{
	cohr2_rec.gst = 0.00;
	cohr2_rec.item_levy 	= OutGst (no_dec (cohr_rec.item_levy), 	TRUE,TRUE);
	cohr2_rec.gross     	= OutGst (no_dec (cohr_rec.gross),     	TRUE,TRUE);
	cohr2_rec.freight   	= OutGst (no_dec (cohr_rec.freight),   	TRUE,TRUE);
	cohr2_rec.insurance 	= OutGst (no_dec (cohr_rec.insurance), 	TRUE,TRUE);
	cohr2_rec.other_cost_1 	= OutGst (no_dec (cohr_rec.other_cost_1),TRUE,TRUE);
	cohr2_rec.other_cost_2 	= OutGst (no_dec (cohr_rec.other_cost_2),TRUE,TRUE);
	cohr2_rec.other_cost_3 	= OutGst (no_dec (cohr_rec.other_cost_3),TRUE,TRUE);
	cohr2_rec.ex_disc   	= OutGst (no_dec (cohr_rec.ex_disc),   	TRUE,FALSE);
	cohr2_rec.disc      	= OutGst (no_dec (cohr_rec.disc),      	TRUE,FALSE);
	cohr2_rec.erate_var 	= OutGst (cohr_rec.erate_var, 			TRUE,TRUE);
	cohr2_rec.sos       	= OutGst (no_dec (cohr_rec.sos),       	TRUE,TRUE);
	cohr2_rec.tax       	= no_dec 	 (cohr_rec.tax);
}

/*
 * Extract Gst.  TOTAL_AMOUNT, ADD_INTO_GST, POSITIVE_NEGATIVE
 */
double	
OutGst (
 double	totalAmount,
 int 	addGst,
 int	pos)
{
	double	gst_amount = 0.00;

	if (totalAmount == 0)
		return (0.00);

	gst_amount = no_dec (totalAmount / envVar.gstDivide);
	
	totalAmount -= no_dec (gst_amount);
	
	if (addGst)
	{
		if (pos)
			cohr2_rec.gst += no_dec (gst_amount);
		else
			cohr2_rec.gst -= no_dec (gst_amount);
	
	}
	return (totalAmount);
}

void
NormalGstProcessing (void)
{
	cohr2_rec.gross     	= no_dec (cohr_rec.gross);
	cohr2_rec.item_levy 	= no_dec (cohr_rec.item_levy);
	cohr2_rec.freight   	= no_dec (cohr_rec.freight);
	cohr2_rec.insurance 	= no_dec (cohr_rec.insurance);
	cohr2_rec.other_cost_1 	= no_dec (cohr_rec.other_cost_1);
	cohr2_rec.other_cost_2 	= no_dec (cohr_rec.other_cost_2);
	cohr2_rec.other_cost_3 	= no_dec (cohr_rec.other_cost_3);
	cohr2_rec.tax       	= no_dec (cohr_rec.tax);
	cohr2_rec.gst       	= no_dec (cohr_rec.gst);
	cohr2_rec.disc      	= no_dec (cohr_rec.disc);
	cohr2_rec.deposit   	= no_dec (cohr_rec.deposit);
	cohr2_rec.ex_disc   	= no_dec (cohr_rec.ex_disc);
	cohr2_rec.erate_var 	= no_dec (cohr_rec.erate_var);
	cohr2_rec.sos       	= no_dec (cohr_rec.sos);
}

void
PostRebate (
 void)
{
	/*
	 * if credit note and 'D' crd_type then this does not constitute
	 * a reduction in the number sold
	 */
	if (!INVOICE && coln_rec.crd_type [0] == 'D')
		return;

	/*
	 * first of all lets look up the contract
	 */
	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
	cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cnch, "DBFIND");

	/*
	 * now lets see if the line is a supplier rebate item
	 */
	cncd_rec.hhch_hash = cnch_rec.hhch_hash;
	cncd_rec.hhbr_hash = coln_rec.hhbr_hash;

	if (cnch_rec.exch_type [0] == 'F')
		strcpy (cncd_rec.curr_code, cumr_rec.curr_code);
	else
		sprintf (cncd_rec.curr_code, "%3.3s", get_env ("CURR_CODE"));

	cc = find_rec (cncd, &cncd_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cncd, "DBFIND");

	if (cncd_rec.hhsu_hash == 0L)
		return;

	/*
	 * now lets add the cnre record
	 */
	memset (&cnre_rec, 0, sizeof (cnre_rec));
	strcpy (cnre_rec.co_no, comm_rec.co_no);
	cnre_rec.hhch_hash = cnch_rec.hhch_hash;
	cnre_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cnre_rec.hhbr_hash = coln_rec.hhbr_hash;
	cnre_rec.hhsu_hash = cncd_rec.hhsu_hash;
	cnre_rec.qty_sold  = coln_rec.q_order;
	cnre_rec.sup_cost  = cncd_rec.cost;
	cnre_rec.std_cost  =out_cost (coln_rec.cost_price,inmr_rec.outer_size);
	strcpy (cnre_rec.inv_no, cohr_rec.inv_no);
	cnre_rec.inv_date = cohr_rec.date_raised;
	strcpy (cnre_rec.status, " ");
	cc = abc_add (cnre, &cnre_rec);
	if (cc)
		file_err (cc, cnre, "DBADD");
}

void
AddCurh (
 void)
{
	memset (&curh_rec, 0, sizeof (curh_rec));

	curh_rec.hhbr_hash = coln_rec.hhbr_hash;
	curh_rec.hhcu_hash = cohr_rec.hhcu_hash;
	curh_rec.ord_date  = cohr_rec.date_raised;
	curh_rec.ord_qty   = (float) (coln_rec.q_order * ((INVOICE) ? 1.00 : -1.00));
	strcpy (curh_rec.inv_no, cohr_rec.inv_no);
	curh_rec.line_cost = ((coln_rec.sale_price * (double) coln_rec.q_order) *
						 ((INVOICE) ? 1.00 : -1.00)) 
						 - coln_rec.amt_disc
						 + coln_rec.amt_tax 
						 + coln_rec.amt_gst;

	cc = abc_add (curh, &curh_rec);
	if (cc)
		file_err (cc, curh, "DBADD");
}

/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	envVar.DB_NETT		=	TRUE;
	envVar.CN_NETT		=	TRUE;
	envVar.GL_ONCOST	=	FALSE;
	envVar.GL_BYCLASS	=	TRUE;
	envVar.DISCOUNT_OK	=	FALSE;

	envVar.gstInclude = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (envVar.gstInclude != 0.00)
		envVar.gstDivide = (float) ((100.00 + envVar.gstInclude) / envVar.gstInclude);

	sprintf (envVar.costSales, "%-1.1s", get_env ("COST_SALES"));

	sptr = chk_env ("GL_SALE_DATE");
	if (sptr == (char *)0)
		strcpy (envVar.saleDateType,"DB_DATE");
	else
		sprintf (envVar.saleDateType,"%-7.7s", sptr);

	sptr = chk_env ("GL_CSALE_DATE");
	if (sptr == (char *)0)
		strcpy (envVar.csaleDateType,"SK_DATE");
	else
		sprintf (envVar.csaleDateType,"%-7.7s", sptr);

	sptr = chk_env ("GL_BYCLASS");
	envVar.GL_BYCLASS = (sptr == (char *)0) ? TRUE : atoi (sptr);

	envVar.DISCOUNT_OK = atoi (get_env ("DIS_OK"));

	sptr = chk_env ("DB_NETT_USED");
	envVar.DB_NETT = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVar.CN_NETT = (sptr == (char *)0) ? envVar.CN_NETT : atoi (sptr);

	sptr = chk_env ("GL_ONCOST");
	envVar.GL_ONCOST = (sptr == (char *)0) ? envVar.GL_ONCOST : atoi (sptr);

	sptr = chk_env ("SO_REBATE_USED");
	envVar.SO_REBATE_USED = (sptr == (char *)0) ? 0 : atoi (sptr);
}

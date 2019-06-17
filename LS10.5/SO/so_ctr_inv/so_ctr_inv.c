/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_ctr_inv.c,v 5.6 2001/11/26 03:21:56 scott Exp $
|  Program Name  : (so_ctr_inv.c)              
|  Program Desc  : (Counter packing slip print. 
|---------------------------------------------------------------------
|  Date Written  : (MM/DD/YYYY     | Author      :                    
|---------------------------------------------------------------------
| $Log: so_ctr_inv.c,v $
| Revision 5.6  2001/11/26 03:21:56  scott
| Updated for spelling mistake
|
| Revision 5.5  2001/11/23 04:46:38  scott
| Issues List Reference Number = 00035
| Program Description = SOMR8-Packing Slip Despatch Confirmation
| Status = Open
| Classification = Error
| Priority = Low
| ReOpen / Return Frequency = 00
| Worksheet = Errors
| Description of the Issue:
| The description 'Packaging and handin' is to be changed to 'Packaging and handling' (it displays when there is Freight amount ).
|
| Revision 5.4  2001/08/28 02:47:31  scott
| Updated to print seperate line for extra discount.
|
| Revision 5.3  2001/08/20 23:45:30  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 09:21:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:07  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_ctr_inv.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ctr_inv/so_ctr_inv.c,v 5.6 2001/11/26 03:21:56 scott Exp $";

#define MAXSCNS 	2
#define MAXWIDTH 	180
#define MAXLINES	500
#define	USE_WIN		1

#define bool int
#define true	1
#define false	0

#include <pslscr.h>	
#include <FormatP.h>
#include "schema"

/*=================================
| Local Record for Screen Editing |
=================================*/
struct
{
	int		iDummy;				/* used as place holder */
} local_rec;

struct	cumrRecord	cumr_rec;
struct	cohrRecord	cohr_rec, cohr_recApp;
struct	comrRecord	comr_rec;
struct	sohrRecord	sohr_rec;
struct	colnRecord	coln_rec;
struct	inmrRecord	inmr_rec;
struct	inlaRecord	inla_rec;
struct	inloRecord	inlo_rec;
struct	inumRecord	inum_rec;
struct	sonsRecord	sons_rec;

/*
 *	Alternate bodies
 */
static const char
	*BodySonsColn	= "sons-body-coln",	/* Comments 			*/
	*BodySonsCohr	= "sons-body-cohr",	/* Comments 			*/
	*BodyKits		= "kits-body",   	/* On flay Kits 		*/
	*BodyBackOrder	= "backorder-body", /* Backorder 			*/
	*BodyMiscChg	= "misc-chg-body";  /* Misc. charge body.	*/

/*
 *	List of program generated value-registers (aside from column values)
 */

/*
 *	Generic value-registers
 */
static const char
	*EvalReprint			= "eval_reprint",
	*EvalTransactionType	= "eval_transaction_type",
	*EvalTransactionNo		= "eval_transaction_no",
	*EvalSalesOrderNo		= "eval_sales_order_no",
	*EvalItemNumber			= "eval_item_no",
	*EvalStrength			= "eval_strength",
	*EvalSalePrice 			= "eval_sale_price",
	*EvalSalePriceDisc		= "eval_sale_price_disc",
	*EvalDiscount			= "eval_discount",
	*EvalItemDiscValue		= "eval_item_disc_value",
	*EvalLineNett			= "eval_line_nett",
	*EvalLineTaxValue		= "eval_line_tax_value",
	*EvalNettCosts			= "eval_nett_costs",
	*EvalExtendedAmt		= "eval_extended_amount",
	*EvalQtySupplied 		= "eval_qty_supplied",
	*EvalQtyOrderTotal		= "eval_q_order_total",
	*EvalMiscNarrative		= "eval_misc_narrative",
	*EvalMiscValue			= "eval_misc_value",
	*EvalOtherCost			= "eval_other_cost",
	*EvalItemLevy			= "eval_item_levy",
	*EvalExDisc				= "eval_ex_disc",
	*EvalDeposit			= "eval_deposit",
	*EvalGST				= "eval_gst",
	*EvalSubTotal	    	= "eval_sub_total",
	*EvalTotal	    		= "eval_total",
	*EvalSubTotal0      	= "eval_sub_total0",
	*EvalTotal0	    		= "eval_total0",
	*EvalSubTotal1	    	= "eval_sub_total1",
	*EvalTotal1	    		= "eval_total1",
	*EvalSubTotal2	    	= "eval_sub_total2",
	*EvalTotal2	    		= "eval_total2",
	*EvalMiscTotal    		= "eval_misc_total",
	*EvalSubTotal3	    	= "eval_sub_total3",
	*EvalTotal3	    		= "eval_total3",
	*EvalSubTotal4	    	= "eval_sub_total4",
	*EvalTaxGst		    	= "eval_tax_gst",
	*EvalTotal4	    		= "eval_total4",
	*EvalCreditInvoiceNo	= "eval_credit_invoice_no",	
	*EvalReasonForCredit	= "eval_reason_for_credit",	
	*EvalCusOrdRef			= "eval_cus_ord_ref",	
	*EvalCurrencyCode		= "eval_currency_code",
	*EvalKitsComments		= "eval_kits_comments",
	*EvalQtyOrder			= "eval_q_order",
	*EvalItemDescription	= "eval_item_desc";

	/*=============
	| Table Names |
	=============*/
	static 	char	*data	= "data",
					*cohr2	= "cohr2";


	/*==================
	| Global Variables |
	==================*/
	bool    multiprint;
	FILE    *fout;
	char    pathName [128];

	static  const char  *so_ctr_inv_fp	= "so_ctr_inv.fp",
						*so_ctr_crd_fp	= "so_ctr_crd.fp";
	char	miscCharges [3][41];


	int		printerNumber	=	1,
			prtInv 			= 	TRUE,
			envSoCtrPacCost	=	0;

/*====================
| Standard Functions |
====================*/
void    StartProgram		(void);	
void    OpenDB				(void);	
void    CloseDB				(void);	
void    shutdown_prog		(void);	
void    Dump				(long);
void    SonsCommentsColn	(long);
void    SonsCommentsCohr 	(long);
void    UpdateCohr			(long);
void    MiscChargeEntry		(const char *, Money);
void    ProcessKits			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	mode [16];
	char	*sptr;
	long	hhcoHash;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	prtInv = TRUE;
	if (!strcmp (sptr, "so_ctr_crd"))
		prtInv = FALSE;
	
	/*
	 *	Begin program, call initialization, abort if init fails |
	 */

	StartProgram ();
		

	sptr = OpenSpecial ("LAYOUT", (prtInv) ? so_ctr_inv_fp : so_ctr_crd_fp);
	
	if (!sptr)
	{
		fprintf (stderr, "Layout file [%s] not found", 
							(prtInv) ? so_ctr_inv_fp : so_ctr_crd_fp);
		return (EXIT_FAILURE);
	}
	strcpy (pathName, sptr);

	if (scanf ("%d", &printerNumber) == EOF || scanf ("%s", mode) == EOF)
		return (EXIT_FAILURE);
	
	if (mode[0] == 'M')
		multiprint = true;

	/*
	 *  Read in stream of hhco-hash from stdin.
	 *
	 *  For Multi-print:
	 *      0
	 *      hashes
	 *      0
	 *
	 *  For Single-print
	 *      hashes
	 *      0
	 */
	 	 			 
	while (scanf ("%ld", &hhcoHash) != EOF)
	{
		if (hhcoHash)
			Dump (hhcoHash);
	}
	
	/*
	 *	Clean up and return SUCCESS 
	 */
	shutdown_prog ();
	return (EXIT_SUCCESS);

}

/*
 * Open the databases and the necessary tables
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cohr2, cohr);

	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inla,  inla_list, INLA_NO_FIELDS, "inla_hhcl_id");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (sons,  sons_list, SONS_NO_FIELDS, "sons_id_no2");
}

/*
 * Close the databases and the necessary tables
 */
void
CloseDB (void)
{
	abc_fclose (cohr2);
	abc_fclose (sons);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (sohr);
	abc_fclose (inmr);
	abc_fclose (inla);
	abc_fclose (cumr);
	abc_fclose (coln);
	abc_fclose (comr);
	abc_fclose (cohr);
	abc_dbclose (data);
}

/*
 *	Program initializations
 */
void
StartProgram (void)
{
	char	*sptr;

	OpenDB ();
	sptr	=	chk_env ("SO_CTR_PAC_COST");
	envSoCtrPacCost	=	(sptr == (char *)0) ? 0 : atoi (sptr);
}

/*
 *	Program clean up
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
Dump (
	long	hhcoHash)
{
	/*
	 *	Open a line to format-p and dump information to it
	 */
	bool	isCredit;
 	char	salesOrderNo [9];
 	char 	envVarCurrCode [4],
 			strength [6];
	char	*sptr;
 	int 	cc;

	Money	otherCost		=	0.00,
			itemLevy		=	0.00,
			deposit			=	0.00,
			otherTotal		=	0.00,
			nettTotal		=	0.00,
			miscTotal		=	0.00,
			noTaxTotal		=	0.00,
			taxGst			=	0.00,
			taxGstTotal		=	0.00,
			taxTotal		=	0.00,
			lineTaxValue	=	0.00,
			lineNett		=	0.00,
			itemTax			=	0.00,
			itemNett		=	0.00,
			itemDisc		=	0.00;

	double 	subTotal 		= 	0.00,
			subTotal0		= 	0.00,
 			subTotal1  		=	0.00,  
 			subTotal2 		=	0.00,
 			subTotal3  		=	0.00,  
			total			=	0.00,
			total0			=	0.00,
			total1      	=	0.00,
			total2      	=	0.00,
			total3      	=	0.00;

	float	qtyOrder		=  	0.00,
			discPercent		=	0.00,
			salePrice		=	0.00,
			salePriceDisc	=	0.00,
			extended_amount	=	0.00,
			nettCosts		=	0.00,
			qtyOrderTotal	=	0.00,
			qtySupplied		=	0.00; 	

        static int formatPOpen = 0;

	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
		No packing slip header found.
	*/
	cohr_rec.hhco_hash = hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	No company master record fould.
	 */
	strcpy (comr_rec.co_no, cohr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return;
		
	/*
	 *	No customer master record found.
	 */
	cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	No Sales order record found.
	 */
	sohr_rec.hhso_hash = cohr_rec.hhso_hash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (!cc)
		strcpy (salesOrderNo, sohr_rec.order_no);
	else
		strcpy (salesOrderNo, "");

	/*
	 *	Worthwhile submitting something to format-p
	 */

	if (!formatPOpen)
	{
		fout = FormatPOpenLpNo (pathName, printerNumber, NULL);
		formatPOpen = 1;
	}

	if (multiprint)
		FormatPReset (fout);

	/*
	 *	Submit tables
	 */
	FormatPSubmitTable (fout, cohr);
	FormatPSubmitTable (fout, comr);
	FormatPSubmitTable (fout, cumr);
	FormatPSubmitTable (fout, sohr);

	/*
	 *	Submit generic header & trailer evaluated values
	 */
	if (!strcmp (cohr_rec.inv_print, "Y"))
		FormatPSubmitChars (fout, EvalReprint, ML ("REPRINT"));

	switch (cohr_rec.type[0])
	{
	case 'I':
		FormatPSubmitChars (fout, EvalTransactionType, ML ("TAX INVOICE"));
		FormatPSubmitChars (fout, EvalTransactionNo,   ML ("Invoice No"));
		isCredit = false;
		break;

	case 'C':
		FormatPSubmitChars (fout, EvalTransactionType, ML ("CREDIT NOTE"));
		FormatPSubmitChars (fout, EvalTransactionNo,   ML ("Credit Note No"));
		isCredit = true;
		break;

	default:
		FormatPSubmitChars (fout, EvalTransactionType, ML ("UNKNOWN"));
		isCredit = false;
	}
	/* 
	 *	Print credit invoice number
	 */
	if (isCredit)
	{
		FormatPSubmitChars (fout, EvalCreditInvoiceNo, cohr_rec.app_inv_no);
		FormatPSubmitChars (fout, EvalReasonForCredit, cohr_rec.cus_ord_ref);

		strcpy (cohr_recApp.co_no, cohr_rec.co_no);
		strcpy (cohr_recApp.br_no, cohr_rec.br_no);
		strcpy (cohr_recApp.inv_no, cohr_rec.app_inv_no);
		strcpy (cohr_recApp.type, "C");
		cc = find_rec (cohr, &cohr_recApp, COMPARISON, "r");
		if (!cc)
			FormatPSubmitChars (fout, EvalCusOrdRef, cohr_recApp.cus_ord_ref);	
	}
	else
	{
		FormatPSubmitChars (fout, EvalCreditInvoiceNo, "");
		FormatPSubmitChars (fout, EvalReasonForCredit, "");
		FormatPSubmitChars (fout, EvalCusOrdRef, cohr_rec.cus_ord_ref);
	}

	if (!strcmp (cumr_rec.curr_code, envVarCurrCode))
		FormatPSubmitChars (fout, EvalCurrencyCode, "");
	else
		FormatPSubmitChars (fout, EvalCurrencyCode, cumr_rec.curr_code);
		
		
	/*
	 *  Calculate Subtotal & Total for Page Trailer
	 */
 	otherCost	=	cohr_rec.insurance    +
					cohr_rec.other_cost_1 +
					cohr_rec.other_cost_2 +
					cohr_rec.other_cost_3 -
					cohr_rec.ex_disc;

	itemLevy	=	cohr_rec.item_levy;

	deposit		=	isCredit ? (Money)0.00 :  cohr_rec.deposit;
	otherTotal 	=	otherCost + 
					itemLevy + 
					cohr_rec.freight -
					cohr_rec.ex_disc - 
					deposit;
	/*
	 *	
	 */
	nettTotal  	=	cohr_rec.gross - cohr_rec.disc;
	miscTotal  	=	cohr_rec.freight 		+ 
					cohr_rec.insurance 		+ 
					cohr_rec.sos 			+
					cohr_rec.item_levy		+
					cohr_rec.other_cost_1 	+
					cohr_rec.other_cost_2 	+
					cohr_rec.other_cost_3	-
					cohr_rec.ex_disc;
	noTaxTotal 	=	miscTotal + nettTotal;
	taxGst		=	cohr_rec.gst + cohr_rec.tax;
	taxGstTotal =	noTaxTotal + taxGst;
	taxTotal   	=	noTaxTotal 	 + 
					cohr_rec.tax     -
					cohr_rec.deposit -
					cohr_rec.ex_disc;

	FormatPSubmitChars (fout, EvalSalesOrderNo, salesOrderNo);
	FormatPSubmitMoney (fout, EvalExDisc,
			 (int) cohr_rec.ex_disc ? (-1) * cohr_rec.ex_disc : (Money) 0.00);

	FormatPSubmitMoney (fout, EvalDeposit, 
			 (int) deposit? (-1) * deposit : (Money) 0.00);

	FormatPSubmitMoney (fout, EvalOtherCost, otherCost);
	FormatPSubmitMoney (fout, EvalItemLevy,  itemLevy);
	FormatPSubmitMoney (fout, EvalSubTotal4, noTaxTotal);
	FormatPSubmitMoney (fout, EvalTaxGst,    taxGst);
	FormatPSubmitMoney (fout, EvalTotal4,    taxGstTotal);

	/*
	 *	Dump line info
	 */

	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		FormatPBody (fout, "Body");		
		FormatPSubmitTable (fout, coln);
		/*
		 *	Process of on the fly kits
		 */
		if (strstr (coln_rec.item_desc, "START OF KIT"))
			ProcessKits ();
		else
		{
			FormatPSubmitMoney (fout, EvalQtyOrder, coln_rec.q_order);
			FormatPSubmitChars (fout, EvalItemDescription, coln_rec.item_desc);
		}
		/*
		 */
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc)
		{
			FormatPSubmitTable (fout, inmr);

			if (strstr (inmr_rec.item_no, "\\"))
				FormatPSubmitChars (fout, EvalItemNumber, "");
			else
				FormatPSubmitChars (fout, EvalItemNumber, inmr_rec.item_no);
		}

		/*
		 *	Check for lot allocations. 
		 */
		inla_rec.hhcl_hash = coln_rec.hhcl_hash;
		inla_rec.inlo_hash = 0;
		cc = find_rec (inla, &inla_rec, GTEQ, "r");
		if (!cc)
		{
			FormatPSubmitTable (fout, inla);
			/*
		 	*	Check for locations. 
		 	*/
			inlo_rec.inlo_hash = inla_rec.inlo_hash;
			cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
			if (!cc)
				FormatPSubmitTable (fout, inlo);
		}

		/*
		 *	Check for unit of measure. 
		 */
		inum_rec.hhum_hash = coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, inum);

		/*
		 *	Calculate generic line evaluated values 
		 */
		qtyOrder		=	coln_rec.q_order;
		lineTaxValue 	=	coln_rec.gross + 
							coln_rec.amt_tax - 
							coln_rec.amt_disc;
		lineNett      	=	coln_rec.gross - coln_rec.amt_disc;

		extended_amount =	(double) coln_rec.q_order;
		extended_amount *=	(envSoCtrPacCost) 	? coln_rec.cost_price
												: coln_rec.sale_price;
			
		nettCosts		=	extended_amount - (float) coln_rec.amt_disc;
		qtyOrderTotal 	=	coln_rec.q_order + coln_rec.q_backorder;
		qtySupplied    	=	coln_rec.q_order - coln_rec.q_backorder;

		if (qtyOrder != 0)
		{
			itemTax 	= coln_rec.amt_gst / qtyOrder;
			itemDisc 	= coln_rec.amt_disc / qtyOrder;
			itemNett 	= lineNett / qtyOrder;
		}
		else
			itemDisc 	= (Money)0.00;

		if (!strcmp (cumr_rec.nett_pri_prt, "Y"))  
		{  
			salePrice 	= (envSoCtrPacCost) ? coln_rec.cost_price 
											: coln_rec.sale_price;

			salePrice 	*= (100 - (float) coln_rec.disc_pc) /100; 
			discPercent = coln_rec.disc_pc;
		}
		else
		{
			salePrice 	= (envSoCtrPacCost) ? coln_rec.cost_price
											: coln_rec.sale_price;
			discPercent = 0.00;
		}

		salePriceDisc 	= (envSoCtrPacCost) ? coln_rec.cost_price
											: coln_rec.sale_price;
		salePriceDisc 	*= (100 - (float) coln_rec.disc_pc) /100; 

		strncpy (strength, (coln_rec.item_desc + 35), 5);
		strength [5] = '\0';
		 
		subTotal	+= (double) lineTaxValue;
		subTotal0 	+= (double) lineTaxValue;
		subTotal1 	+= (double) nettCosts;
		subTotal2  	= subTotal1 + (double) cohr_rec.freight;
		subTotal3   = (double) taxTotal;

		total		= subTotal	+ (double) otherTotal;
		total0      = subTotal0 + (double) cohr_rec.gst;
		total1      = subTotal1 + (double) cohr_rec.gst;
		total2      = subTotal2 + (double) cohr_rec.gst;
		total3      = subTotal3 + (double) cohr_rec.gst;
		
		/*
		 *	Submit generic line evaluated values 
		 */
		FormatPSubmitMoney (fout, EvalNettCosts,     nettCosts);
		FormatPSubmitMoney (fout, EvalItemDiscValue, itemDisc);
		FormatPSubmitMoney (fout, EvalQtyOrderTotal, qtyOrderTotal);
		FormatPSubmitMoney (fout, EvalQtySupplied,   qtySupplied);
		FormatPSubmitChars (fout, EvalStrength,	  	 strength);
		FormatPSubmitMoney (fout, EvalSalePrice,     salePrice);
		FormatPSubmitMoney (fout, EvalSalePriceDisc, salePriceDisc);
		FormatPSubmitMoney (fout, EvalDiscount,      coln_rec.disc_pc);
		FormatPSubmitMoney (fout, EvalLineNett,  	 lineNett);
		FormatPSubmitMoney (fout, EvalLineTaxValue,  lineTaxValue);
		FormatPSubmitMoney (fout, EvalExtendedAmt,   extended_amount);

		FormatPSubmitChars (fout, EvalGST,           "G.S.T.");
		FormatPSubmitMoney (fout, EvalSubTotal,      subTotal);
		FormatPSubmitMoney (fout, EvalTotal,         total);
		FormatPSubmitMoney (fout, EvalSubTotal0, 	 subTotal0);
		FormatPSubmitMoney (fout, EvalTotal0,        total0);
		FormatPSubmitMoney (fout, EvalSubTotal1,     subTotal1);
		FormatPSubmitMoney (fout, EvalTotal1,        total1);
		FormatPSubmitMoney (fout, EvalSubTotal2,     subTotal2);
		FormatPSubmitMoney (fout, EvalTotal2,        total2);
		FormatPSubmitMoney (fout, EvalMiscTotal,     miscTotal);
		FormatPSubmitMoney (fout, EvalSubTotal3,     subTotal3);
		FormatPSubmitMoney (fout, EvalTotal3,        total3);

		/*
		 *	Here endeth the major part
		 */
		FormatPBatchEnd (fout);
		 
		if (coln_rec.q_backorder)
		{
			FormatPBody (fout, BodyBackOrder);
			FormatPSubmitTable (fout, coln);
			FormatPBatchEnd (fout);
			FormatPBody (fout, "body");
		}

		/*
		 *	Append possible comment lines
		 */
		if (coln_rec.hhcl_hash)
			SonsCommentsColn (coln_rec.hhcl_hash);
			
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}

	if (miscTotal != 0)
	{
		FormatPBody (fout, BodyMiscChg);

		MiscChargeEntry (ML("Packaging + handling"), cohr_rec.freight);
		MiscChargeEntry (ML("Minimum order fee"), cohr_rec.sos);
		MiscChargeEntry (ML("Insurance"), cohr_rec.insurance);
		MiscChargeEntry (ML("Item Levy"), cohr_rec.item_levy);
		MiscChargeEntry (ML("Extra Discount"), cohr_rec.ex_disc);
		sptr	=	get_env ("SO_OTHER_1");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, cohr_rec.other_cost_1);
		sptr	=	get_env ("SO_OTHER_2");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, cohr_rec.other_cost_2);
		sptr	=	get_env ("SO_OTHER_3");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, cohr_rec.other_cost_3);		

		FormatPBody (fout, "body");	/* revert back to std body */
	}
	/*
	 *	Comments allocated to each lines of sales order.
	 */
	SonsCommentsCohr (cohr_rec.hhco_hash);
	/*
	 *	Comments allocated to order header.
	*/
	if (!multiprint)
		FormatPClose (fout);
	/* 
	 *  After print Invoice/Invoice Slip
	 *  update cohr table set inv_print="Y"
	 *  and set cohr_print back to blank 
	 */
	UpdateCohr (hhcoHash);
}

void
ProcessKits (void)
{
	int cc = 0;

	FormatPBody (fout, BodyKits);
	cc = find_rec (coln, &coln_rec, NEXT, "r");
	while (	!cc && (strcmp (coln_rec.item_desc,"***: END OF KIT :****")))
	{
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc && (!strcmp (inmr_rec.item_no, "NS")))
		{
			FormatPSubmitChars (fout, 	EvalKitsComments, coln_rec.item_desc);
			FormatPBatchEnd (fout);
		}
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	
	FormatPBody (fout, "body");

	FormatPSubmitMoney (fout, EvalQtyOrder, coln_rec.q_order);
	FormatPSubmitChars (fout, EvalItemDescription, coln_rec.item_desc);
}

void
SonsCommentsColn (
	long hhclHash)
{
	int cc;

	sons_rec.hhcl_hash = hhclHash;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc && sons_rec.hhcl_hash == hhclHash)
	{
		FormatPBody (fout, BodySonsColn);
		do
		{
			clip (sons_rec.desc);
			if (!strcmp (sons_rec.desc, ""))
				FormatPBody (fout, BodySonsColn);

			FormatPSubmitTable (fout, sons);
			FormatPBatchEnd (fout);
			
			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}	while (!cc &&
					  sons_rec.hhcl_hash == hhclHash);
		FormatPBody (fout, "body");		/* revert back to std body */
	}
}

void
SonsCommentsCohr (
	long	hhcoHash)
{
	int cc;

	abc_selfield (sons, "sons_id_no4");

	sons_rec.hhco_hash 	= hhcoHash;
	sons_rec.line_no 	= 0;

	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc)
	{
		FormatPBody (fout, BodySonsCohr);
		do
		{
			if (strlen (clip (sons_rec.desc)))
				FormatPBody (fout, BodySonsCohr);

			FormatPSubmitTable (fout, sons);
			FormatPBatchEnd (fout);
			
			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}	while (!cc && sons_rec.hhco_hash == hhcoHash);

		FormatPBody (fout, "body");		/* revert back to std body */
	}
	abc_selfield (sons, "sons_id_no2");
}

void
MiscChargeEntry (
 	const char * narrative,
 	Money value)
{
	if (value != 0)
	{
		FormatPSubmitChars (fout, EvalMiscNarrative, narrative);
		FormatPSubmitMoney (fout, EvalMiscValue, value);
		FormatPBatchEnd (fout);
	}
}


void
UpdateCohr (
	long	hhcoHash)
{
	int		cc;
	cohr_rec.hhco_hash = hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
	if (cc)
		return;
		
	/*
	 *	Update
	 */
	strcpy (cohr_rec.printing, "N");
	/*
	 *	After Invoice Slip is printed
	 */
	strcpy (cohr_rec.inv_print, "Y");

	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBUPDATE");
}


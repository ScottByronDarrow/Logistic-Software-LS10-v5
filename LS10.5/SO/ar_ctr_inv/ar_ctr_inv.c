/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ar_ctr_inv.c,v 5.0 2002/05/07 10:21:39 scott Exp $
|  Program Name  : (ar_ctr_inv.c)              
|  Program Desc  : (Arhcive Invoice / Credit print.
|---------------------------------------------------------------------
|  Date Written  : (MM/DD/YYYY     | Author      :                    
|---------------------------------------------------------------------
| $Log: ar_ctr_inv.c,v $
| Revision 5.0  2002/05/07 10:21:39  scott
| Updated to bring version number to 5.0
|
| Revision 1.1  2001/12/11 02:42:57  scott
| Archive Invoice Print.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ar_ctr_inv.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/ar_ctr_inv/ar_ctr_inv.c,v 5.0 2002/05/07 10:21:39 scott Exp $";

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
struct	arhrRecord	arhr_rec, arhr_recApp;
struct	comrRecord	comr_rec;
struct	sohrRecord	sohr_rec;
struct	arlnRecord	arln_rec;
struct	inmrRecord	inmr_rec;
struct	inlaRecord	inla_rec;
struct	inloRecord	inlo_rec;
struct	inumRecord	inum_rec;
struct	sonsRecord	sons_rec;

/*
 *	Alternate bodies
 */
static const char
	*BodySonsArln	= "sons-body-arln",	/* Comments 			*/
	*BodySonsArhr	= "sons-body-arhr",	/* Comments 			*/
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
					*arhr2	= "arhr2";


	/*==================
	| Global Variables |
	==================*/
	bool    multiprint;
	FILE    *fout;
	char    pathName [128];

	static  const char  *ar_ctr_inv_fp	= "ar_ctr_inv.fp",
						*ar_ctr_crd_fp	= "ar_ctr_crd.fp";
	char	miscCharges [3][41];


	int		printerNumber	=	1,
			prtInv 			= 	TRUE;

/*====================
| Standard Functions |
====================*/
void    StartProgram		(void);	
void    OpenDB				(void);	
void    CloseDB				(void);	
void    shutdown_prog		(void);	
void    Dump				(long);
void    SonsCommentsArln	(long);
void    SonsCommentsArhr 	(long);
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
	if (!strcmp (sptr, "ar_ctr_crd"))
		prtInv = FALSE;
	
	/*
	 *	Begin program, call initialization, abort if init fails |
	 */

	StartProgram ();
		

	sptr = OpenSpecial ("LAYOUT", (prtInv) ? ar_ctr_inv_fp : ar_ctr_crd_fp);
	
	if (!sptr)
	{
		fprintf (stderr, "Layout file [%s] not found", 
							(prtInv) ? ar_ctr_inv_fp : ar_ctr_crd_fp);
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

	abc_alias (arhr2, arhr);

	open_rec (arhr,  arhr_list, ARHR_NO_FIELDS, "arhr_hhco_hash");
	open_rec (arhr2, arhr_list, ARHR_NO_FIELDS, "arhr_id_no");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (arln,  arln_list, ARLN_NO_FIELDS, "arln_id_no");
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
	abc_fclose (arhr2);
	abc_fclose (sons);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (sohr);
	abc_fclose (inmr);
	abc_fclose (inla);
	abc_fclose (cumr);
	abc_fclose (arln);
	abc_fclose (comr);
	abc_fclose (arhr);
	abc_dbclose (data);
}

/*
 *	Program initializations
 */
void
StartProgram (void)
{
	OpenDB ();
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
	arhr_rec.hhco_hash = hhcoHash;
	cc = find_rec (arhr, &arhr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	No company master record fould.
	 */
	strcpy (comr_rec.co_no, arhr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return;
		
	/*
	 *	No customer master record found.
	 */
	cumr_rec.hhcu_hash = arhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	No Sales order record found.
	 */
	sohr_rec.hhso_hash = arhr_rec.hhso_hash;
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
	FormatPSubmitTable (fout, arhr);
	FormatPSubmitTable (fout, comr);
	FormatPSubmitTable (fout, cumr);
	FormatPSubmitTable (fout, sohr);

	/*
	 *	Submit generic header & trailer evaluated values
	 */
	if (!strcmp (arhr_rec.inv_print, "Y"))
		FormatPSubmitChars (fout, EvalReprint, ML ("REPRINT"));

	switch (arhr_rec.type[0])
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
		FormatPSubmitChars (fout, EvalCreditInvoiceNo, arhr_rec.app_inv_no);
		FormatPSubmitChars (fout, EvalReasonForCredit, arhr_rec.cus_ord_ref);

		strcpy (arhr_recApp.co_no, arhr_rec.co_no);
		strcpy (arhr_recApp.br_no, arhr_rec.br_no);
		strcpy (arhr_recApp.inv_no, arhr_rec.app_inv_no);
		strcpy (arhr_recApp.type, "C");
		cc = find_rec (arhr, &arhr_recApp, COMPARISON, "r");
		if (!cc)
			FormatPSubmitChars (fout, EvalCusOrdRef, arhr_recApp.cus_ord_ref);	
	}
	else
	{
		FormatPSubmitChars (fout, EvalCreditInvoiceNo, "");
		FormatPSubmitChars (fout, EvalReasonForCredit, "");
		FormatPSubmitChars (fout, EvalCusOrdRef, arhr_rec.cus_ord_ref);
	}

	if (!strcmp (cumr_rec.curr_code, envVarCurrCode))
		FormatPSubmitChars (fout, EvalCurrencyCode, "");
	else
		FormatPSubmitChars (fout, EvalCurrencyCode, cumr_rec.curr_code);
		
		
	/*
	 *  Calculate Subtotal & Total for Page Trailer
	 */
 	otherCost	=	arhr_rec.insurance    +
					arhr_rec.other_cost_1 +
					arhr_rec.other_cost_2 +
					arhr_rec.other_cost_3 -
					arhr_rec.ex_disc;

	itemLevy	=	arhr_rec.item_levy;

	deposit		=	isCredit ? (Money)0.00 :  arhr_rec.deposit;
	otherTotal 	=	otherCost + 
					itemLevy + 
					arhr_rec.freight -
					arhr_rec.ex_disc - 
					deposit;
	/*
	 *	
	 */
	nettTotal  	=	arhr_rec.gross - arhr_rec.disc;
	miscTotal  	=	arhr_rec.freight 		+ 
					arhr_rec.insurance 		+ 
					arhr_rec.sos 			+
					arhr_rec.item_levy		+
					arhr_rec.other_cost_1 	+
					arhr_rec.other_cost_2 	+
					arhr_rec.other_cost_3	-
					arhr_rec.ex_disc;
	noTaxTotal 	=	miscTotal + nettTotal;
	taxGst		=	arhr_rec.gst + arhr_rec.tax;
	taxGstTotal =	noTaxTotal + taxGst;
	taxTotal   	=	noTaxTotal 	 + 
					arhr_rec.tax     -
					arhr_rec.deposit -
					arhr_rec.ex_disc;

	FormatPSubmitChars (fout, EvalSalesOrderNo, salesOrderNo);
	FormatPSubmitMoney (fout, EvalExDisc,
			 (int) arhr_rec.ex_disc ? (-1) * arhr_rec.ex_disc : (Money) 0.00);

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

	arln_rec.hhco_hash 	= hhcoHash;
	arln_rec.line_no 	= 0;
	cc = find_rec (arln, &arln_rec, GTEQ, "r");
	while (!cc && arln_rec.hhco_hash == hhcoHash)
	{
		FormatPBody (fout, "Body");		
		FormatPSubmitTable (fout, arln);
		/*
		 *	Process of on the fly kits
		 */
		if (strstr (arln_rec.item_desc, "START OF KIT"))
			ProcessKits ();
		else
		{
			FormatPSubmitMoney (fout, EvalQtyOrder, arln_rec.q_order);
			FormatPSubmitChars (fout, EvalItemDescription, arln_rec.item_desc);
		}
		/*
		 */
		inmr_rec.hhbr_hash = arln_rec.hhbr_hash;
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
		inla_rec.hhcl_hash = arln_rec.hhcl_hash;
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
		inum_rec.hhum_hash = arln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, inum);

		/*
		 *	Calculate generic line evaluated values 
		 */
		qtyOrder		=	arln_rec.q_order;
		lineTaxValue 	=	arln_rec.gross + 
							arln_rec.amt_tax - 
							arln_rec.amt_disc;
		lineNett      	=	arln_rec.gross - arln_rec.amt_disc;

		extended_amount =	(double) arln_rec.q_order;
		extended_amount *=	arln_rec.sale_price;
			
		nettCosts		=	extended_amount - (float) arln_rec.amt_disc;
		qtyOrderTotal 	=	arln_rec.q_order + arln_rec.q_backorder;
		qtySupplied    	=	arln_rec.q_order - arln_rec.q_backorder;

		if (qtyOrder != 0)
		{
			itemTax 	= arln_rec.amt_gst / qtyOrder;
			itemDisc 	= arln_rec.amt_disc / qtyOrder;
			itemNett 	= lineNett / qtyOrder;
		}
		else
			itemDisc 	= (Money)0.00;

		if (!strcmp (cumr_rec.nett_pri_prt, "Y"))  
		{  
			salePrice 	=  arln_rec.sale_price;

			salePrice 	*= (100 - (float) arln_rec.disc_pc) /100; 
			discPercent = arln_rec.disc_pc;
		}
		else
		{
			salePrice 	= arln_rec.sale_price;
			discPercent = 0.00;
		}

		salePriceDisc 	= arln_rec.sale_price;
		salePriceDisc 	*= (100 - (float) arln_rec.disc_pc) /100; 

		strncpy (strength, (arln_rec.item_desc + 35), 5);
		strength [5] = '\0';
		 
		subTotal	+= (double) lineTaxValue;
		subTotal0 	+= (double) lineTaxValue;
		subTotal1 	+= (double) nettCosts;
		subTotal2  	= subTotal1 + (double) arhr_rec.freight;
		subTotal3   = (double) taxTotal;

		total		= subTotal	+ (double) otherTotal;
		total0      = subTotal0 + (double) arhr_rec.gst;
		total1      = subTotal1 + (double) arhr_rec.gst;
		total2      = subTotal2 + (double) arhr_rec.gst;
		total3      = subTotal3 + (double) arhr_rec.gst;
		
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
		FormatPSubmitMoney (fout, EvalDiscount,      arln_rec.disc_pc);
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
		 
		if (arln_rec.q_backorder)
		{
			FormatPBody (fout, BodyBackOrder);
			FormatPSubmitTable (fout, arln);
			FormatPBatchEnd (fout);
			FormatPBody (fout, "body");
		}

		/*
		 *	Append possible comment lines
		 */
		if (arln_rec.hhcl_hash)
			SonsCommentsArln (arln_rec.hhcl_hash);
			
		cc = find_rec (arln, &arln_rec, NEXT, "r");
	}

	if (miscTotal != 0)
	{
		FormatPBody (fout, BodyMiscChg);

		MiscChargeEntry (ML("Packaging + handling"), arhr_rec.freight);
		MiscChargeEntry (ML("Minimum order fee"), arhr_rec.sos);
		MiscChargeEntry (ML("Insurance"), arhr_rec.insurance);
		MiscChargeEntry (ML("Item Levy"), arhr_rec.item_levy);
		MiscChargeEntry (ML("Extra Discount"), arhr_rec.ex_disc);
		sptr	=	get_env ("SO_OTHER_1");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, arhr_rec.other_cost_1);
		sptr	=	get_env ("SO_OTHER_2");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, arhr_rec.other_cost_2);
		sptr	=	get_env ("SO_OTHER_3");
		if (sptr != (char *)0)
			MiscChargeEntry (sptr, arhr_rec.other_cost_3);		

		FormatPBody (fout, "body");	/* revert back to std body */
	}
	/*
	 *	Comments allocated to each lines of sales order.
	 */
	SonsCommentsArhr (arhr_rec.hhco_hash);
	/*
	 *	Comments allocated to order header.
	*/
	if (!multiprint)
		FormatPClose (fout);
}

void
ProcessKits (void)
{
	int cc = 0;

	FormatPBody (fout, BodyKits);
	cc = find_rec (arln, &arln_rec, NEXT, "r");
	while (	!cc && (strcmp (arln_rec.item_desc,"***: END OF KIT :****")))
	{
		inmr_rec.hhbr_hash = arln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc && (!strcmp (inmr_rec.item_no, "NS")))
		{
			FormatPSubmitChars (fout, 	EvalKitsComments, arln_rec.item_desc);
			FormatPBatchEnd (fout);
		}
		cc = find_rec (arln, &arln_rec, NEXT, "r");
	}
	
	FormatPBody (fout, "body");

	FormatPSubmitMoney (fout, EvalQtyOrder, arln_rec.q_order);
	FormatPSubmitChars (fout, EvalItemDescription, arln_rec.item_desc);
}

void
SonsCommentsArln (
	long hhclHash)
{
	int cc;

	sons_rec.hhcl_hash = hhclHash;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc && sons_rec.hhcl_hash == hhclHash)
	{
		FormatPBody (fout, BodySonsArln);
		do
		{
			clip (sons_rec.desc);
			if (!strcmp (sons_rec.desc, ""))
				FormatPBody (fout, BodySonsArln);

			FormatPSubmitTable (fout, sons);
			FormatPBatchEnd (fout);
			
			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}	while (!cc &&
					  sons_rec.hhcl_hash == hhclHash);
		FormatPBody (fout, "body");		/* revert back to std body */
	}
}

void
SonsCommentsArhr (
	long	hhcoHash)
{
	int cc;

	abc_selfield (sons, "sons_id_no4");

	sons_rec.hhco_hash 	= hhcoHash;
	sons_rec.line_no 	= 0;

	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc)
	{
		FormatPBody (fout, BodySonsArhr);
		do
		{
			if (strlen (clip (sons_rec.desc)))
				FormatPBody (fout, BodySonsArhr);

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

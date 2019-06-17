/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_ctr_pac.c,v 5.7 2001/11/26 03:22:00 scott Exp $
|  Program Name  : ( so_ctr_pac.c )                                   |
|  Program Desc  : ( Counter packing slip print                     ) |
|---------------------------------------------------------------------|
|  Date Written  : (MM/DD/YYYY     | Author      :                    |
|---------------------------------------------------------------------|
| $Log: so_ctr_pac.c,v $
| Revision 5.7  2001/11/26 03:22:00  scott
| Updated for spelling mistake
|
| Revision 5.6  2001/11/23 04:46:39  scott
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
| Revision 5.5  2001/10/24 08:49:04  cha
| Updated to ensure that inputs are properly
| scanned in.
|
| Revision 5.4  2001/09/11 23:46:02  scott
| Updated from Scott machine - 12th Sep 2001
|
| Revision 5.3  2001/08/20 23:45:31  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 09:21:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:08  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_ctr_pac.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ctr_pac/so_ctr_pac.c,v 5.7 2001/11/26 03:22:00 scott Exp $";

#define MAXSCNS 	2
#define MAXWIDTH 	180
#define MAXLINES	500
#define	USE_WIN		1

#define bool int
#define true 1
#define false 0

#include <pslscr.h>	
#include <stdio.h>
#include <FormatP.h>

#include "schema"

	int		envSoCtrPacCost	=	0;

/*=================================
| Local Record for Screen Editing |
=================================*/
struct
{
	int	iDummy;		/* used as place holder */
} local_rec;

/*=============
| Table Names |
=============*/
static char *data = "data";

FILE *fout;

struct	cumrRecord	cumr_rec;
struct	cohrRecord	cohr_rec;
struct	comrRecord	comr_rec;
struct	sohrRecord	sohr_rec;
struct	exsfRecord	exsf_rec;		
struct	colnRecord	coln_rec;
struct	ccmrRecord	ccmr_rec;
struct	inmrRecord	inmr_rec;
struct	inlaRecord	inla_rec;
struct	inccRecord	incc_rec;
struct	inloRecord	inlo_rec;
struct	inumRecord	inum_rec;

/*
 *	Alternate bodies
 */
static const char							/*------------------------------*/
	*BodyBackOrder	= "backorder-body",		/* Backorder body				*/
	*BodySonsColn	= "sons-body-coln",		/* Packing Slip line comments 	*/
	*BodySonsCohr	= "sons-body-cohr",		/* Packing slip header comments */
	*BodyMiscChg	= "misc-chg-body",		/* Misc. charge(s) body			*/
	*BodySpace		= "space-body";     	/* Space Line 					*/
											/*------------------------------*/

/*
 *	List of program generated value-registers (aside from column values)
 */

/*
 *	Generic value-registers
 */
static const char
	*EvalReprint			= "eval_reprint",
	*EvalSalesOrderNo		= "eval_sales_order_no",
	*EvalPayTerms			= "eval_pay_terms",
	*EvalSalePrice 			= "eval_sale_price",
	*EvalSalePriceDisc		= "eval_sale_price_disc",
	*EvalDiscount			= "eval_discount",
	*EvalItemDiscValue		= "eval_item_disc_value",
	*EvalLineTaxValue		= "eval_lineTaxValue",
	*EvalQtySupplied 		= "eval_qty_supplied",
	*EvalQtyOrderTotal		= "eval_q_order_total",
	*EvalLineNett			= "eval_line_nett",
	*EvalSalePrice1			= "eval_sale_price1",
	*EvalInumUom			= "eval_inum_uom",
	*EvalDiscPc				= "eval_disc_pc",
	*EvalAmtDisc			= "eval_amt_disc",
	*EvalMiscNarrative		= "eval_misc_narrative",
	*EvalMiscValue			= "eval_misc_value",
	*EvalGST				= "eval_gst",
	*EvalTaxGst				= "eval_tax_gst",
	*EvalOtherCost			= "eval_other_cost",
	*EvalItemLevy			= "eval_item_levy",
	*EvalExDisc				= "eval_ex_disc",
	*EvalDeposit			= "eval_deposite",
	*EvalSubTotal	    	= "eval_sub_total",
	*EvalTotal	    		= "eval_total",
	*EvalSubTotal1	    	= "eval_sub_total1",
	*EvalTotal1	    		= "eval_total1",
	*EvalBackOrderQty		= "eval_backorder_quantity",
	*EvalBackOrderString1	= "eval_backorder_string1",
	*EvalBackOrderString2	= "eval_backorder_string2",
	*EvalBackOrderItemNo  	= "eval_backorder_item_no",
	*EvalBackOrderSalePrice	= "eval_backorder_sale_price",
	*EvalBackOrderOuterSize = "eval_backorder_outer_size",
	*EvalBackOrderDiscount	= "eval_backorder_discount";
/*
 *
 */

/*====================
| Standard Functions |
====================*/
void  	StartProgram 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	shutdown_prog 			(void);
void 	Dump 					(long);
void 	SonsCommentsColn 		(long);
void 	SonsCommentsCohr 		(long);
void 	UpdateCohr 				(long);
void 	MiscChargeEntry 		(const char *, Money, int);


/*==================
| Global Variables |
==================*/
bool 	multiprint;
char 	pathName [128];
static 	const 	char *so_ctr_pac_fp	= "so_ctr_pac.fp";

int	lpno;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	mode [16];
	char	*sptr;
	long    hhcoHash;
	
	fflush (stdout);
	/*---------------------------------------------------------
	| Begin program, call initialization, abort if init fails |
	---------------------------------------------------------*/
	StartProgram ();

	sptr	=	OpenSpecial ("LAYOUT", so_ctr_pac_fp);
	if (!sptr)
	{
		fprintf (stderr, "Layout file [%s] not found", so_ctr_pac_fp);
		return (EXIT_FAILURE);
	}
	strcpy (pathName, sptr);
	if (scanf ("%d", &lpno) == EOF || scanf ("%s", mode) == EOF)
		return (EXIT_FAILURE);
	
	if (mode [0] == 'M')
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
	
	/*-----------------------------
	| Clean up and return SUCCESS |
	-----------------------------*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=============================================
| Open the databases and the necessary tables |
=============================================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_hhcl_id");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (sons, sons_list, SONS_NO_FIELDS, "sons_id_no2");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*===========================================================
| Close the tables that were opened, and close the database |
===========================================================*/
void
CloseDB (void)
{
	abc_fclose (exsf);
	abc_fclose (sons);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (sohr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inla);
	abc_fclose (ccmr);
	abc_fclose (cumr);
	abc_fclose (coln);
	abc_fclose (comr);
	abc_fclose (cohr);
	
	abc_dbclose (data);
}



/*=========================
| Program initializations |
=========================*/
void
StartProgram (void)
{
	char	*sptr;
	sptr	=	chk_env ("SO_CTR_PAC_COST");
	envSoCtrPacCost	=	(sptr == (char *)0) ? 0 : atoi (sptr);

   OpenDB ();
}

/*==================
| Program clean up |
==================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}


void 
Dump (
	long hhcoHash)
{
	/*
	 *	Open a line to format-p and dump information to it
	 */
	static int formatPOpen = 0;

	char	salesOrderNumber [9],
			payTerms [60],
			backorder_string1 [100],
			backorder_string2 [100];

	int		printPrice;

	Money 	otherCost		= 0.00,
			itemLevy		= 0.00,
			deposit			= 0.00,
			otherTotal		= 0.00,
			nettTotal		= 0.00,
			miscTotal		= 0.00,
			noTaxTotal		= 0.00,
			taxGst			= 0.00,
			taxGstTotal		= 0.00,
			total 			= 0.00,
			subTotal 		= 0.00,
			lineNett		= 0.00,
			lineTaxValue	= 0.00,
			itemTax			= 0.00,
			itemNett		= 0.00,
			itemDisc		= 0.00,
			discPercent		= 0.00,
			salePrice		= 0.00,
			salePriceDisc	= 0.00,
			qtyOrder		= 0.00,
			qtyOrderTotal	= 0.00,
			qtySupplied		= 0.00;

	/*
	 *	Find packing slip header
	 */
	cohr_rec.hhco_hash = hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	Find company master
	 */
	strcpy (comr_rec.co_no, cohr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	*	Find Charge to customer master.
	*/
	if (cohr_rec.chg_hhcu_hash)
	{
		cumr_rec.hhcu_hash = cohr_rec.chg_hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*
	 		*	Find normal customer master as charge to not found.
	 		*/
			cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		}
		if (cc)
			return;
	}
	/*
	*	Find Normal customer master
	*/
	else
	{
		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			return;
	}

	/*
	 *	Find sales order header
	 */
	sohr_rec.hhso_hash = cohr_rec.hhso_hash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (!cc)
		strcpy (salesOrderNumber, sohr_rec.order_no);
	else
		strcpy (salesOrderNumber, "");
	
	strcpy (exsf_rec.salesman_no, cohr_rec.sale_code);
	strcpy (exsf_rec.co_no, cohr_rec.co_no);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	
	/*
	 *	Worthwhile submitting something to format-p
	 */
	if (!formatPOpen)	
	{
		fout = FormatPOpenLpNo (pathName, lpno, NULL); 
		formatPOpen = 1;
	}
	
	if (multiprint)
		FormatPReset (fout);
	/*
	 *	Submit tables
	 */

	sprintf (payTerms, "PAYMENT TERMS:%s", cohr_rec.pay_terms);

	FormatPSubmitTable (fout, cohr);
	FormatPSubmitTable (fout, comr);
	FormatPSubmitTable (fout, cumr);
	FormatPSubmitTable (fout, sohr);
	FormatPSubmitTable (fout, exsf);
	FormatPSubmitChars (fout, EvalPayTerms, payTerms);

	/*
	 *  Calculate Subtotal & Total for Page Trailer
	 */
	otherCost	=	cohr_rec.insurance    +
				 	cohr_rec.other_cost_1 +
				 	cohr_rec.other_cost_2 +
				 	cohr_rec.other_cost_3;
						    	 
	itemLevy	=	cohr_rec.item_levy;

	deposit		=	cohr_rec.deposit;
	otherTotal	=	otherCost + 
				  	cohr_rec.item_levy -
				  	cohr_rec.freight -
				  	cohr_rec.ex_disc - 
	              	deposit;

	nettTotal 	=	cohr_rec.gross - cohr_rec.disc;
	
	miscTotal	=	cohr_rec.freight + 
					cohr_rec.item_levy + 
					cohr_rec.insurance + 
					cohr_rec.sos +
					cohr_rec.other_cost_1 +
					cohr_rec.other_cost_2 +
					cohr_rec.other_cost_3 -
					cohr_rec.ex_disc;
						   	 
	noTaxTotal 	= 	miscTotal + nettTotal;
	taxGst 		= 	cohr_rec.gst + cohr_rec.tax;
	taxGstTotal = 	noTaxTotal + taxGst;

	/*
	 *	Submit generic header & trailer evaluated values
	 */
	if (!strcmp (cohr_rec.ps_print, "Y"))
	{
		FormatPSubmitChars (fout, EvalReprint, "REPRINT");
	}
	if (!strcmp (cohr_rec.prt_price, "Y"))
		printPrice = true;
	else
		printPrice = false;

	FormatPBody (fout, "body");
	FormatPSubmitChars (fout, EvalSalesOrderNo, salesOrderNumber);
	FormatPSubmitMoney (fout, EvalExDisc,
				 (int)cohr_rec.ex_disc ? (-1.00) * cohr_rec.ex_disc : 0.00);
	FormatPSubmitMoney (fout, EvalDeposit, 
			     (int) deposit ? (-1) * deposit : 0.00);
	FormatPSubmitMoney (fout, EvalOtherCost, otherCost);
	FormatPSubmitMoney (fout, EvalItemLevy, itemLevy);
	FormatPSubmitChars (fout, EvalGST, "G.S.T.");
	FormatPSubmitMoney (fout, EvalTotal, total);
	if (printPrice)
	{
		FormatPSubmitMoney (fout, EvalTaxGst, taxGst);
		FormatPSubmitMoney (fout, EvalSubTotal1, noTaxTotal);
		FormatPSubmitMoney (fout, EvalTotal1, taxGstTotal);
	}

	/*
	 *	Dump line info
	 */

	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");

	while (!cc &&
			  coln_rec.hhco_hash == hhcoHash)
	{
		FormatPBody (fout, "body");
		
		FormatPSubmitTable (fout, coln);
				
		/*
		 * DPL specific, print wharhouse on the header
		 */
		ccmr_rec.hhcc_hash = coln_rec.incc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (!cc)
			FormatPSubmitTable (fout, ccmr);
		
		/*
		 * Generic: Print Body 
		 */
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, inmr);		
		
		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, incc);

		inla_rec.hhcl_hash = coln_rec.hhcl_hash;
		inla_rec.inlo_hash = 0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "r");
		if (!cc)
			FormatPSubmitTable (fout, inla);

		inlo_rec.inlo_hash = inla_rec.inlo_hash;
		cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, inlo);

		inum_rec.hhum_hash = coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (!cc)
			FormatPSubmitTable (fout, inum);
	
		/*
		 *	Calculate generic line evaluated values 
		 */
		qtyOrder 		=	coln_rec.q_order;
		lineTaxValue 	= 	coln_rec.gross + 
							coln_rec.amt_tax - 
							coln_rec.amt_disc ,
		lineNett  		=   coln_rec.gross - coln_rec.amt_disc;
		qtyOrderTotal 	=	coln_rec.q_order + coln_rec.q_backorder,
		qtySupplied    	=	coln_rec.q_order - coln_rec.q_backorder;

		if (qtyOrder != 0)
		{
			itemTax 	= coln_rec.amt_gst / qtyOrder;
			itemDisc 	= coln_rec.amt_disc / qtyOrder;
			itemNett 	= lineNett / qtyOrder;
		}
		else
			itemDisc 	= (Money)0.00;

		if (cumr_rec.nett_pri_prt [0] != 'Y')
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

		salePriceDisc	*=	(100 - (float) coln_rec.disc_pc) /100; 

		subTotal		+= (double)lineTaxValue;
		total      		= subTotal + (double) otherTotal;

		/*
		 *	Submit generic line evaluated values 
		 */
		FormatPSubmitMoney (fout, EvalItemDiscValue, itemDisc);
		FormatPSubmitMoney (fout, EvalQtyOrderTotal, qtyOrderTotal*100);
		FormatPSubmitMoney (fout, EvalQtySupplied,   qtySupplied*100);
		FormatPSubmitMoney (fout, EvalSalePrice,     salePrice);
		FormatPSubmitMoney (fout, EvalSalePriceDisc, salePriceDisc);
		FormatPSubmitMoney (fout, EvalDiscount,      discPercent * 100);
		FormatPSubmitMoney (fout, EvalLineTaxValue,  lineTaxValue);
		FormatPSubmitMoney (fout, EvalSubTotal,      subTotal);
		FormatPSubmitMoney (fout, EvalTotal,         total);

		if (printPrice) /* USL specific */
		{
			FormatPSubmitMoney (fout, EvalLineNett, 	lineNett);	
			FormatPSubmitMoney (fout, EvalSalePrice1, 	
				(envSoCtrPacCost) ? coln_rec.cost_price : coln_rec.sale_price);
			FormatPSubmitChars (fout, EvalInumUom,		inum_rec.uom);
			FormatPSubmitMoney (fout, EvalDiscPc, 		coln_rec.disc_pc * 100);
			FormatPSubmitMoney (fout, EvalAmtDisc,		coln_rec.amt_disc);
		}

		/*
		 *	Here endeth the major part
		 */
		FormatPBatchEnd (fout);
		if (coln_rec.q_backorder)
		{
			FormatPBody (fout, BodyBackOrder);

			strcat (backorder_string1, "***************   ");
			strcat (backorder_string2, "BACK ORDERED");

			FormatPSubmitMoney (fout, EvalBackOrderQty, coln_rec.q_backorder);
			FormatPSubmitChars (fout, EvalBackOrderString1, backorder_string1);
			FormatPSubmitChars (fout, EvalBackOrderString2, backorder_string2);
			FormatPSubmitChars (fout, EvalBackOrderItemNo, inmr_rec.item_no);
			FormatPSubmitMoney (fout, EvalBackOrderSalePrice, salePrice);
			FormatPSubmitMoney (fout, EvalBackOrderOuterSize, inmr_rec.outer_size);
			FormatPSubmitMoney (fout, EvalBackOrderDiscount, discPercent * 100);

			FormatPBatchEnd (fout);
			FormatPBody (fout, "body");			/* revert back to std body */
		}
		/*
		 *	Append possible comment lines
		 */
		if (coln_rec.hhcl_hash)
			SonsCommentsColn (coln_rec.hhcl_hash);
		/*
		 *	Print space line
		 */
		FormatPBody (fout, BodySpace);
		FormatPBatchEnd (fout);
		FormatPBody (fout, "body");				/* revert back to std body */
		
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}

	if (miscTotal != 0)
	{
		FormatPBody (fout, BodyMiscChg);

		MiscChargeEntry (ML ("Packaging + handling"), cohr_rec.freight, printPrice);
		MiscChargeEntry (ML ("Minimum order fee"), cohr_rec.sos, printPrice);
		MiscChargeEntry (ML ("Insurance"), cohr_rec.insurance, printPrice);

		FormatPBody (fout, "body");			/* revert back to std body */
	}
	SonsCommentsCohr (cohr_rec.hhco_hash);
	/*
	*/
	if (!multiprint)
		FormatPClose (fout);
	/* 
	 *  After print Invoice/Packing Slip
	 *  update cohr table set ps_print="Y"
	 *  and set cohr_print back to blank 
	 */
	UpdateCohr (hhcoHash);
}

void
SonsCommentsColn (
	long hhcl_hash)
{
	struct sonsRecord	sons_rec;
	int cc;

	sons_rec.hhcl_hash 	= hhcl_hash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc)
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
					 sons_rec.hhcl_hash == hhcl_hash);

		FormatPBody (fout, "body");				/* revert back to std body */
	}
}


void
SonsCommentsCohr ( long hhcoHash )
{
	struct sonsRecord	sons_rec;
	int cc;

	abc_selfield (sons, "sons_id_no4");

	sons_rec.hhco_hash = hhcoHash;
	sons_rec.line_no = 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	if (!cc)
	{
		FormatPBody (fout, BodySonsCohr);
		do
		{
			clip (sons_rec.desc);
			if (!strcmp (sons_rec.desc, ""))
				FormatPBody (fout, BodySonsCohr);

			FormatPSubmitTable (fout, sons);
			FormatPBatchEnd (fout);
			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}	while (!cc &&
					 sons_rec.hhco_hash == hhcoHash);

		FormatPBody (fout, "body");				// revert back to std body
	}
	
	abc_selfield (sons, "sons_id_no2");
}


void
MiscChargeEntry (
 const char * narrative,
 Money value,
 int prtPrice)
{
	if (value != 0)
	{
		FormatPSubmitChars (fout, EvalMiscNarrative, narrative);
		if (prtPrice)
			FormatPSubmitMoney (fout, EvalMiscValue, value);
		FormatPBatchEnd (fout);
	}
}
/*
 *
 */

void
UpdateCohr (
	long hhcoHash)
{
	int cc;
	
	cohr_rec.hhco_hash = hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
	if (cc)
		return;

	/*
	 *	Update
	 */
	strcpy (cohr_rec.printing, "N");
	strcpy (cohr_rec.ps_print, "Y");
	cohr_rec.ps_print_no++;

	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBUPDATE");
}

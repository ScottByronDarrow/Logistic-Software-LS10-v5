/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_invoice.c,v 5.21 2002/11/28 04:09:50 scott Exp $
|  Program Name  : (so_invoice.c) 
|  Program Desc  : (Direct / Manual Invoice entry)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/11/1988       |
|---------------------------------------------------------------------|
| $Log: so_invoice.c,v $
| Revision 5.21  2002/11/28 04:09:50  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.20  2002/07/24 08:39:27  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.19  2002/07/17 09:32:02  robert
| Use HideStockWindow instead of CloseStockWindow during window clear
|
| Revision 5.18  2002/07/17 08:36:39  scott
| Updated as GUI window not closed.
|
| Revision 5.17  2002/07/10 07:33:08  robert
| S/C 4112 - Fixed reverse display overlap
|
| Revision 5.16  2002/07/09 07:17:22  scott
| S/C 004039. Updated to close window in GUI
|
| Revision 5.15  2002/06/20 07:16:04  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.14  2002/06/20 05:49:03  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.13  2002/03/07 02:23:01  scott
| .
|
| Revision 5.12  2002/03/06 07:25:05  scott
| S/C 00810 - INMR3-Display Invoices; The Contract Description is not displayed
|
| Revision 5.11  2001/10/23 07:16:42  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.10  2001/10/19 10:50:09  robert
| Updated to be consistent in rounding of discount with other modules
|
| Revision 5.9  2001/09/27 03:02:54  scott
| Updated to add AllocationRestore () and AllocationComplete ()
| Fixed problem with allocation to lots being lost if restart performed.
| Updated to ensure allocation performed correctly.
|
| Revision 5.8  2001/09/04 08:12:49  scott
| Updated to not recalculate item levy for display.
|
| Revision 5.7  2001/08/28 06:14:37  scott
| Updated to change " ( to "(
|
| Revision 5.6  2001/08/23 11:46:17  scott
| Updated from scotts machine
|
| Revision 5.5  2001/08/09 09:21:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.4  2001/08/06 23:51:23  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_invoice.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invoice/so_invoice.c,v 5.21 2002/11/28 04:09:50 scott Exp $";

extern	int		X_EALL;
extern	int		Y_EALL;
extern	int		_win_func;

#define	ADJ_VAL(x)	(adj_val (DOLLARS (x)) * 100.00)
#define	USE_WIN		1
#define MAXSCNS 	6
#define MAXWIDTH 	180
#define MAXLINES	500
#define	EXPORT		(cohr_rec.ord_type [0] == 'E')
#define	BUSY_ON			1
#define	BUSY_OFF		0

#define	SCN_HEADER		1
#define	SCN_ITEMS		2
#define	SCN_FREIGHT		3
#define	SCN_MISC		4
#define	SCN_PROOF		5

#define ErrorReturn 	1

#define	OVER_AMT(a, b, c)	((a - b) / c)

#define	SLEEP_TIME	2

#define	PRINT_LINES	1
#define	CALC_LINES	1

#define	SR				store [line_cnt]
#define	LSR				store [lcount [SCN_ITEMS]]
#define	SERIAL			(SR.serialFlag [0] == 'Y')
#define	BONUS			(SR.bonusItem [0] == 'Y')
#define	NO_COST			(SR.itemClass [0] == 'N')
#define KIT_START		(SR.kitFlag == K_START)
#define KIT_END			(SR.kitFlag == K_END)
#define OLD_INSF		(SR.oldInsf [0] == 'Y')
#define ONLINE_INVOICE	(type_flag [0] == 'O')
#define	NON_STOCK(x)	(store [x].itemClass [0] == 'Z')
#define	MARG_MESS1		(envVar.salesOrderMargin [0] == '0')
#define	MARG_MESS2		(envVar.salesOrderMargin [0] == '1')
#define	MARG_MESS3		(envVar.salesOrderMargin [0] == '2')
#define	KIT_ITEM		(SR.itemClass [0] == 'K' && prog_status == ENTRY)
#define	PHANTOM			(SR.itemClass [0] == 'P' && prog_status == ENTRY)
#define	CASH			(cumr_rec.cash_flag [0] == 'Y')
#define	BY_BRANCH		1
#define	BY_DEPART		2

#define	TXT_REQD
#define	MAX_SONS		10

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <ml_tr_mess.h>
#include <getnum.h>
#include <Costing.h>

#ifdef GVISION
#include <StockWindow.h>

#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#endif	/* GVISION */

#define	INPUT	0
#define	MAINT	1

#define IS_ENTRY     (prog_status == ENTRY)
#define NOT_ENTRY    (prog_status != ENTRY)
#define CASH_INVOICE (cumr_rec.cash_flag [0] == 'Y')                 
#define CHARGE_INVOICE (cumr_rec.cash_flag [0] != 'Y')

#define K_NONE      0
#define K_START     1
#define K_ITEM      2
#define K_END       3

#define	STANDARD	(invoiceTypeFlag [0] == 'I' && programRunType != DISPLAY)
#define AUTO_SK_UP	(createStatusFlag [0] == envVar.automaticStockUpdate [0])

#define	INV_DISPLAY	(programRunType == DISPLAY)
#define	INV_MAINT	(programRunType == MAINT)
#define	INV_INPUT	(programRunType == INPUT)

#define	FREIGHT_CHG	(cohr_rec.frei_req [0] == 'Y' && envVar.automaticFreight)

#define	PROMPT	prompts [promptType]._prompt
#define	SRCH	prompts [promptType]._srch

#define	FGN_CURR		(envVar.dbMcurr && strcmp (cumr_rec.curr_code, envVar.currencyCode))

	struct	{
		char	*_prompt;
		char	*_srch;
	} prompts [] = {
		{"Invoice Number      : ","#Inv No."},
		{"Packing Slip Number : ","#P/Slip."},
		{"Invoice Number      : ","#Inv No."},
		{"Delivery Order No.  : ","#Del Ord"},
	};

	int		noTaxCharged		= 0,	/* charge gst & tax			*/
			newInvoice			= 0,	/* creating new invoice		*/
			insertLineFlag		= 0,	/* inserting line item		*/
			inputDiscValue		= 0,	/* input of Discount.    	*/
			inputSaleValue		= 0,	/* input of Sales Price. 	*/
			inputDescValue		= 0,	/* input of Description. 	*/
			invoiceProofFlag 	= 0,	/* proof total ok			*/
			processingByBatch	= 0,	/* require proof total		*/
			functionFileNo		= 0,
			programRunType		= 0,
			promptType			= 0,
			specialDisplay 		= FALSE,
			inScreenFlag   		= FALSE,
			pastEntry 			= FALSE,
			Recalc_kit 			= FALSE,
			SysGenInv 			= FALSE,
			ins_flag			= 0, /* inserting line item			*/
			LotSelectFlag		= 0;

	long	kitHash = 0L;

	double	creditRemain	= 0.00,		/* credit remaining				*/
			lineLevyAmt		= 0.00,		/* line levy amount				*/
			lineLevyPc		= 0.00,		/* line levy percent			*/
			lineLevy 		= 0.00,		/* line item Levy				*/
			lineTaxAmt		= 0.00,		/* line item gross for tax Amt  */
			lineGross 		= 0.00,		/* line item gross				*/
			lineDisc 		= 0.00,		/* line item discount			*/
			lineTax 		= 0.00,		/* line item tax				*/
			lineGst 		= 0.00,		/* line item gst				*/
			totalInvoice 	= 0.00,		/* invoiced total				*/
			totalDisc 		= 0.00,		/* discount total				*/
			totalTax 		= 0.00,		/* tax total					*/
			totalGrand 		= 0.00,		/* Total all.					*/
			totalGst 		= 0.00,		/* gst total					*/
			totalLevy 		= 0.00,		/* item levy total				*/
			proofTotal 		= 0.00,		/* invoice proof total			*/
			batch_tot 		= 0.00,		/* batch total					*/
			invTotalAmt 	= 0.00,		/* 								*/
			estimFreight 	= 0.00;

	char	defaultBranchNo 	[3],	/* branch number				*/
			createStatusFlag 	[2],	/* create status for invoice	*/
			invoiceTypeFlag 	[2],	/* I (nvoice or C (redit Note	*/
			recordLockFlag 		[2],
			bonusFlag 			[3],
			hiddenFlag 			[3],
			creat_flag 			[2], 	/* create status for invoice    */
			type_flag 			[2],	/* I (nvoice or C (redit Note 	*/
			read_flag 			[2],
			manual_pref 		[3],	
			price_type 			[2],
			stat_flag 			[2],
			HeaderDesc 			[MAX_SONS + 1][41],
			*currentUser;

#include	"schema"

struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct sobgRecord	sobg_rec;
struct cuccRecord	cucc_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct pocrRecord	pocrRec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct soicRecord	soic_rec;
struct soicRecord	soic2_rec;
struct cudpRecord	cudp_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct inalRecord	inal_rec;
struct exsiRecord	exsi_rec;
struct cudiRecord	cudi_rec;
struct sonsRecord	sons_rec;
struct trshRecord	trsh_rec;
struct trchRecord	trch_rec;
struct trzmRecord	trzm_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;
struct soktRecord	sokt_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	char	*data	= "data",
			*cohr2	= "cohr2",
			*cumr2	= "cumr2",
			*inum2	= "inum2",
			*soic2	= "soic2",
			*ser_space 		= "                         ",
			*ns_space 		= "                                        ",
			*sixteen_space 	= "                ",
			*twenty_spaces 	= "                    ";

	struct	storeRec {
		long 	hhbrHash;		/* inmr_hhbr_hash					*/
		long 	hhsiHash;		/* inmr_hhsi_hash					*/
		long 	hhwhHash;		/* incc_hhwh_hash					*/
		long 	hhccHash;		/* incc_hhwh_hash					*/
		long 	hhahHash;		/* inbc_hhah_hash					*/
		long 	hhclHash;		/* coln_hhcl_hash					*/
		long 	hhumHash;		/* inum_hhum_hash					*/
		char	nsDesc [MAX_SONS + 1][41]; 	/*Non stock description.    */
								/*									*/
		float 	qtyAvail;		/* incc_closing_stock				*/
		float 	qtySup;			/* local_rec.qty_sup				*/
		float	qtyOrd;			/* local_rec.qty_ord				*/
		float	gstPc;			/* inmr_gst_pc or 0.00 if notax		*/
		float	taxPc;			/* inmr_tax_pc or 0.00 if notax		*/
		float	dfltDisc;		/* inmr_disc_pc						*/
		float	discPc;			/* coln_disc_pc						*/
		float	calcDisc;		/*                       			*/
		float	outerSize;		/* inmr_outer_size					*/
		float	minMarg;		/* Min margin for category.     	*/
		float	regPc;			/* Regulatory percent.      		*/
		float	discA;			/* Discount percent A.      		*/
		float	discB;			/* Discount percent A.      		*/
		float	discC;			/* Discount percent A.      		*/
		float	convFct;		/* Conversion factor.	      		*/
		float	stdConvFct;		/* Standard Conversion factor.	    */
								/*									*/
		double	taxAmount;		/* inmr_tax_amt " 0.00 if notax		*/
		double	margCost;		/* Cost price for Margins.			*/
		double	salePrice;		/* coln_sale_price					*/
		double	calcSprice;		/*                 					*/
		double	actSale;		/*                 					*/
		double	gSalePrice;		/* coln_gsale_price					*/
		double	dfltPrice;		/* inmr_price [i]					*/
		double	advertLevyAmt;	/* Advertising Levy.				*/
		double	advertLevyPc;	/* Advertising Levy.				*/
		double	itemLevy;		/* coln_item_levy.					*/
		double	itemWeight;		/* inmr_weight.						*/
		double	extendTotal;	/* Extended total. 					*/
								/*									*/
		char	lotDone [2]; 	/* Lot complete Y/N 				*/
		char	lotControl [2];	/* Lot control Y/N 					*/
		char	serialItem [26];/* serial number for line			*/
		char	origSerial [26];/* serial number for line			*/
		char	category [12];	/* serial number for line			*/
		char	sellGroup [7];	/* inmr_sellgrp.           			*/
		char	bonusItem [2];	/* bonus item ?						*/
		char	itemClass [2];	/* item's class for line			*/
		char	packSize [6];	/* inmr_pack_size					*/
		char	serialFlag [2]; /* inmr_serial_item 				*/
		char	priceOveride [2];/* Price overide.     				*/
		char	discOveride [2];/* Discount overide.				*/
		char	UOM [5];		/* Unit of Measure					*/
       	char    uomGroup [21];	/* UOM Group						*/
								/*									*/
		char    oldInsf [2];	/* Y = insf record already allocated*/
		int		oldLine;		/* False if new line.				*/
		int     kitFlag;      	/* Start End or None.               */
		int		pricingCheck;	/* Set if pricing has been  		*/
								/* called for line.         		*/
		int		cumulative;		/* Cumulative 1 or 0				*/
		int		conPrice;		/* Contract price.					*/
		int		indentItem;		/* Indent applies.					*/
		int		decimalPt;		/* Decimal point.					*/
		int		contStatus;		/* 0 = not contract line	*/
								/* 1 = contract no disc		*/
		int		commitRef;		/* soic Committed reference	*/
		int		deCommitRef;	/* soic DeCommitted reference	*/
		long	origHhbrHash;	/* Original hhbr hash 		*/
		long	origOrdQty;		/* Original order Qty 		*/
	} store [MAXLINES];

    struct {
        char    _scode [4];
        char    _sdesc [32];
    } STerms [] = {
		{"   ","Local                 "},
		{"CIF","Cost Insurance Freight"},
		{"C&F","Cost & Freight"},
		{"FIS","Free Into Store"},
		{"FOB","Free On Board"},
		{"",""},
	};

	int		wpipe_open = FALSE;

	char	*scn_desc [] = 
	{
		"HEADER SCREEN.",
		"ITEM SCREEN.",
		"FREIGHT DETAILS SCREEN.",
		"MISCELLANEOUS DETAIL SCREEN.",
	};

	FILE	*wout;

	int		nextSoicRef = 0;
	long	progPid;

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	char	automaticStockUpdate [2];
	char	currencyCode [4];
	char	gstCode [4];
	char	manualPrint [2];
	char	other [3][31];
	char	overrideQuantity [2];
	char	priceTypeComment [25];
	char	salesOrderMargin [3];
	char	soSpecial [5];
	int		advertLevy;
	int		automaticFreight;
	int		creditOver;
	int		creditStop;
	int		creditTerms;
	int		dbCo;
	int		dbFind;
	int		dbMcurr;
	int		dbNettUsed;
	int		discountIndents;
	int		gstApplies;
	int		includeForwardStock;
	int		lostSales;
	int		numberPrices;
	int		perminantWindow;
	int		qCApplies;
	int		qCAvailable;
	int		reverseDiscount;
	int		salesOrderSales;
	int		serialItemsOk;
	int		soFreightCharge;
	int		useSystemDate;
	int		windowPopupOk;
	int		invoiceNumbers;
	int		KitDiscount;
} envVar;

char	*norm_invoice = "M0123456789";

#include	<CheckIndent.h>
#include	<p_terms.h>

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	lot_done [2];
	char	lot_ctrl [2];
	long	dflt_inv_no;
	char	_date_raised [11];
	char	_date_required [11];
	char	dummy [11];
	char	dflt_ord [2];
	char	dflt_batch [6];
	double	dflt_freight;
	char	cust_no [7];
	char	invoice_no [7];
	char	inv_prmpt [31];
	char	pri_desc [16];
	char	pri_fulldesc [16];
	char	ord_desc [10];
	char	ord_fulldesc [10];
	char	pp_desc [4];
	char	dbt_date [11];
	char	systemDate [11];
	long	longSystemDate;
	char	item_no [17];
	char	item_desc [41];
	char	sup_part [17];
	char	spinst [3][61];
	char	sell_desc [31];
	char	UOM [5];
	float	qty_ord;
	float	qty_sup;
	char	prev_dbt_no [7];
	char	prev_inv_no [9];
	float	disc_over;
	double	inp_total;
	double  each;
	double	extend;
	char	dflt_sale_no [3];
	char	carr_area [3];
	char	norm_area [41];
	char	carr_adesc [41];
	char	curr_code [6];
	char	cont_desc [31];
	char	serial_no [26];
	char	LL [2];
	char	headOfficeCustomer [7];
	char	chargeToCustomer [7];
	char	chargeToName [36];
	char	defaultDelZone [7];
} local_rec;            

static	struct	var	vars [] =
{
	{SCN_HEADER, LIN, "debtor",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Customer Number     : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{SCN_HEADER, LIN, "curr_code",	 4, 35, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{SCN_HEADER, LIN, "name",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{SCN_HEADER, LIN, "invoice_no",	 5, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "00000000", local_rec.inv_prmpt, " ",
		 NE, NO,  JUSTLEFT, "", "", cohr_rec.inv_no},
	{SCN_HEADER, LIN, "cus_ord_ref",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Customer Order Ref. : ", " ",
		 NI, NO,  JUSTLEFT, "", "", cohr_rec.cus_ord_ref},
	{SCN_HEADER, LIN, "dp_no",	 6, 2, CHARTYPE,
		"AA", "          ",
		" ", cumr_rec.department, "Department No.      : ", " ",
		 NA, NO, JUSTRIGHT, "", "", cohr_rec.dp_no},
	{SCN_HEADER, LIN, "dp_name",	 6, 28, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cudp_rec.dp_short},
	{SCN_HEADER, LIN, "headOfficeAccount",	 6, 66, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Head Office Account : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.headOfficeCustomer},
	{SCN_HEADER, LIN, "cont_no",	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract            : ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 NE, NO,  JUSTLEFT, "", "", cohr_rec.cont_no},
	{SCN_HEADER, LIN, "cont_desc",	7, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cont_desc},
	{SCN_HEADER, LIN, "chargeToCustomer", 7, 66, CHARTYPE,
		"UUUUUU", "          ",

		" ", "0", "Charge To Customer  : ", "Enter Charge to Customer Number, [SEARCH]. ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.chargeToCustomer},
	{SCN_HEADER, LIN, "chargeToName",	 7, 96, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.chargeToName},
	{SCN_HEADER, LIN, "chargeToHash", 7, 66, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "0", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&cohr_rec.chg_hhcu_hash},
	{SCN_HEADER, LIN, "cus_addr1",	 9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Charge Address      : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr1},
	{SCN_HEADER, LIN, "cus_addr2",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr2},
	{SCN_HEADER, LIN, "cus_addr3",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr3},
	{SCN_HEADER, LIN, "cus_addr4",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr4},
	{SCN_HEADER, LIN, "del_name",	 9, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Name       : ", " Select Delivery Name and Address. Search available. ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_name},
	{SCN_HEADER, LIN, "del_addr1",	10, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address.   : ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{SCN_HEADER, LIN, "del_addr2",	11, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    : ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add2},
	{SCN_HEADER, LIN, "del_addr3",	12, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    : ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add3},
	{SCN_HEADER, LIN, "batch_no",	14, 2, CHARTYPE,
		"UUUUU", "        ",
		" ", local_rec.dflt_batch, "Batch number        : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.batch_no},
	{SCN_HEADER, LIN, "ord_type",	14, 66, CHARTYPE,
		"U", "          ",
		" ", "D", "Order Type          : ", " D (omestic  E (xport ",
		 NA, NO,  JUSTLEFT, "DE", "", local_rec.ord_desc},
	{SCN_HEADER, LIN, "ord_type_desc",	14, 91, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ord_fulldesc},
	{SCN_HEADER, LIN, "pri_type",	15, 2, CHARTYPE,
		"N", "        ",
		" ", cumr_rec.price_type, "Price Type          : ", envVar.priceTypeComment,
		 NA, NO,  JUSTLEFT, "123456789", "", local_rec.pri_desc},
	{SCN_HEADER, LIN, "pri_type_desc",	15, 27, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.pri_fulldesc},
	{SCN_HEADER, LIN, "date_raised",	15, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec._date_raised, "Invoice Date        : ", " ",
		 NI, NO,  JUSTLEFT, " ", "", (char *)&cohr_rec.date_raised},
	{SCN_HEADER, LIN, "fix_exch",	15, 66, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exchange Rate : ", " ",
		 ND, NO,  JUSTLEFT, "YN", "", cohr_rec.fix_exch},
	{SCN_HEADER, LIN, "tax_code",	16, 2, CHARTYPE,
		"U", "        ",
		" ", cumr_rec.tax_code, "Tax Code            : ", " ",
		 ND, NO,  JUSTLEFT, "ABCD", "", cohr_rec.tax_code},
	{SCN_HEADER, LIN, "tax_no",	16, 66, CHARTYPE,
		"AAAAAAAAAAAAAAA", "        ",
		" ", cumr_rec.tax_no, "Tax Number          : ", " ",
		 ND, NO,  JUSTLEFT, "", "", cohr_rec.tax_no},
	{SCN_HEADER, LIN, "sale_code",	17, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.sman_code, "Salesman            : ", " ",
		 NI, NO, JUSTRIGHT, "", "", cohr_rec.sale_code},
	{SCN_HEADER, LIN, "sman_desc",	17, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesman Desc.      : ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{SCN_HEADER, LIN, "area_code",	18, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.area_code, "Area code           : ", " ",
		 NI, NO, JUSTRIGHT, "", "", cohr_rec.area_code},
	{SCN_HEADER, LIN, "area_desc",	18, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Area Description    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{SCN_HEADER, LIN, "disc_over",	20, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Discount Overide    : ", " This Discount is Added to Each Invoice Line ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.disc_over},
	{SCN_HEADER, LIN, "prt_price",	20, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print Price Details : ", "Y(es) print prices on invoice, N(o) Don't print prices on Invoice.",
		 NI, NO,  JUSTLEFT, "NY", "", cohr_rec.prt_price},
	{SCN_HEADER, LIN, "inp_total",	20, 66, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.00", "Invoice Total       : ", " Total Value Override for Invoice ",
		 NI, NO, JUSTRIGHT, "", "", (char *)&local_rec.inp_total},
	{SCN_ITEMS, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " Default Deletes Line ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{SCN_ITEMS, TAB, "hide",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "H", " ",
		 NA, NO,  JUSTLEFT, "", "", coln_rec.hide_flag},
	{SCN_ITEMS, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "     D e s c r i p t i o n    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{SCN_ITEMS, TAB, "sman_code",	 0, 1, CHARTYPE,
		"UU", "          ",
		" ", cohr_rec.sale_code, "Sale", " Salesman ",
		 ND, NO, JUSTRIGHT, "", "", coln_rec.sman_code},
	{SCN_ITEMS, TAB, "ord_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cohr_rec.cus_ord_ref, " Cust Order Ref. ", "Customer Order Ref.",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.cus_ord_ref},
	{SCN_ITEMS, TAB, "pack_size",	 0, 0, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Pack ", " ",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.pack_size},
	{SCN_ITEMS, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", "", "UOM.", " Unit of Measure ",
		 YES, NO, JUSTLEFT, "", "", local_rec.UOM},
	{SCN_ITEMS, TAB, "qty_ord",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "1.00", " Qty Ord ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_ord},
	{SCN_ITEMS, TAB, "qty_sup",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "1.00", " Qty Sup ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_sup},
	{SCN_ITEMS, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Cst Price", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&coln_rec.cost_price},
	{SCN_ITEMS, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0.00", "Sal Price", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&coln_rec.sale_price},
	{SCN_ITEMS, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Disc ", " ",
		YES, NO, JUSTRIGHT, "-100.00", "100.00", (char *)&coln_rec.disc_pc},
	{SCN_ITEMS, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Serial Number      ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{SCN_ITEMS, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{SCN_ITEMS, TAB, "extend",	 0, 0, MONEYTYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "0.00", "  Extended  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extend},
	{SCN_FREIGHT, LIN, "carrierCode",	 4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Carrier Code.       : ", "Enter carrier code, [SEARCH] available.",
		YES, NO,  JUSTLEFT, "", "", trcm_rec.carr_code},
	{SCN_FREIGHT, LIN, "carr_desc",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Carrier Description : ", " ",
		 NA, NO,  JUSTLEFT, "", "", trcm_rec.carr_desc},
	{SCN_FREIGHT, LIN, "deliveryZoneCode",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultDelZone, "Delivery Zone       : ", "Enter Delivery Zone Code [SEARCH]. ",
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{SCN_FREIGHT, LIN, "deliveryZoneDesc",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Zone Desc  : ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{SCN_FREIGHT, LIN, "deliveryRequired",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Delivery Reqd. (Y/N): ", "Enter Y(es) for Delivery. <default = N(o)> ",
		 YES, NO, JUSTLEFT, "YN", "", cohr_rec.del_req},
	{SCN_FREIGHT, LIN, "deliveryDate",	6, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Delivery Date       : ", " ",
		NA, NO,  JUSTLEFT, " ", "", (char *)&cohr_rec.del_date},
	{SCN_FREIGHT, LIN, "cons_no",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Consignment no.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.cons_no},
	{SCN_FREIGHT, LIN, "no_cartons",	 7, 66, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Number Cartons.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cohr_rec.no_cartons},
	{SCN_FREIGHT, LIN, "est_freight",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Est Freight         : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&estimFreight},
	{SCN_FREIGHT, LIN, "tot_kg",	 9, 66, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Total Kgs.          : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.no_kgs},
	{SCN_FREIGHT, LIN, "frei_req",	 10, 2, CHARTYPE,
		"U", "          ",
		" ", cumr_rec.freight_chg, "Freight Required.   : ", "Enter Y(es) if freight required Default = Customer master file default. ",
		YES, NO, JUSTRIGHT, "YN", "", cohr_rec.frei_req},
	{SCN_FREIGHT, LIN, "freight",	 10, 66, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.     : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.freight},
	{SCN_FREIGHT, LIN, "shipname",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship to name        : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_name},
	{SCN_FREIGHT, LIN, "shipaddr1",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship to address 1   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{SCN_FREIGHT, LIN, "shipaddr2",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship to address 2   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add2},
	{SCN_FREIGHT, LIN, "shipaddr3",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship to address 3   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add3},
	{SCN_FREIGHT, LIN, "ship_method",	17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipment method     : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{SCN_FREIGHT, LIN, "spcode1",	18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 1       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{SCN_FREIGHT, LIN, "spcode2",	19, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 2       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{SCN_MISC, LIN, "pay_term",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.      : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.pay_terms},
	{SCN_MISC, LIN, "sell_terms",	 6, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Sell Terms.         : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.sell_terms},
	{SCN_MISC, LIN, "sell_desc",	7 , 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Sell Description.   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sell_desc},
	{SCN_MISC, LIN, "insurance",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Insurance.          : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.insurance},
	{SCN_MISC, LIN, "ins_det",	9, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Insurance Details   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.ins_det},
	{SCN_MISC, LIN, "deposit",	11, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Deposit.            : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.deposit},
	{SCN_MISC, LIN, "discount",	12, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Special Discount.   : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.ex_disc},
	{SCN_MISC, LIN, "other_1",	14, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [0], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_1},
	{SCN_MISC, LIN, "other_2",	15, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [1], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_2},
	{SCN_MISC, LIN, "other_3",	16, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [2], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_3},
	{SCN_PROOF, LIN, "proof",	 4, 20, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Proof Total", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&proofTotal},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<cus_price.h>
#include	<cus_disc.h>
#include	<neg_win.h>
#include	<FindCumr.h>
#include	<LocHeader.h>
#include	<ItemLevy.h>

extern	int		TruePosition;


/*=======================
| Function Declarations |
=======================*/
char	*GetPostingStatus		(char *);
char	*GetPriceDesc 			(int);
float	CalculateOverride		(double);
float	ProcessPhantom			(long);
float	ReCalcAvail				(void);
float	RealTimeCommitted		(long,long,int);
float	ScreenDisc				(float);
float	ToLclUom				(float);
float	ToStdUom				(float);
int		CheckKitting			(void);
int		DeleteInvoiceLine		(void);
int		FoundOpenKit			(void);
int		InsertInvoiceLine		(void);
int		KitHasNS				(void);
int		LineInKit				(int);
int		AddIncc					(long,long);
int		AddInei					(void);
int		CheckBonusLine			(char *);
int		CheckCohr				(char *);
int		CheckDuplicateInsf		(char *,long,int);
int		CheckHiddenLine			(char *);
int		CheckIncc				(void);
int		DeleteLine				(void);
int		FindCucc				(int,long);
int		GenSoicRef				(void);
int		InsertLine				(void);
int		LoadDisplay				(char *);
int		LoadItemScreen			(long);
int		OpenStockWindow			(void);
int		SrchCudi				(int);
int		ValidateItemNumber		(int);
int		WarnUser				(char *,int,int);
int		heading					(int);
int		spec_valid				(int);
int		use_window				(int);
int		win_function			(int,int,int,int);
int		win_function2			(int,int,int,int);
void	CalcKitLine				(void);
void	CheckKit				(int);
void	ForceKitCompletion		(void);
void	Init_store				(int);
void	RemoveSerialNo			(long,long,char *);
void	ResetKit				(int,int);
void	RunningKit				(int);
void	SetInvoiceDefaults		(int);
void	UpdateKitSoicQty		(int);
void	AddCarrierDetails		(void);
void	AddNewSoic				(int);
void	AddSobg					(int,char *,long);
void	BusyFunction			(int);
void	CalculateBoxTotals		(int,int);
void	CalculateExtendedTotal	(int);
void	CalculateFreight		(float,double,double,double);
void	CalculateInputTotal		(void);
void	CalculateLineTotal		(int);
void	CheckEnvironment		(void);
void	CheckSerialQty			(float *);
void	ClearSoic				(void);
void	ClearWindow				(void);
void	CloseDB					(void);
void	CloseTransportFiles		(void);
void	CommitInsf				(int,char *);
void	DeleteSONS				(int,long);
void	DiscProcess				(void);
void	DispSONS				(int);
void	DrawTotals				(void);
void	FreeInsf				(int,char *);
void	InitML					(void);
void	InputResponce			(float *);
void	InputSONS				(int,int);
void	LoadSONS				(int,int,long);
void	LogLostSales			(float);
void	MarginCheckOk			(double,float,double,float);
void	OpenDB					(void);
void	OpenTransportFiles		(char *);
void	PriceProcess			(void);
void	PrintCompanyDetails		(void);
void	PrintExtendedTotal		(int);
void	PrintTotalBoxValues		(void);
void	ProcSoic				(int,int);
void	ProcessKitItem			(long,float);
void	ReadMisc				(void);
void	SetInvoiceDefaults		(int);
void	SillyBusyFunc			(int);
void	SrchCnch				(char *);
void	SrchCohr				(char *);
void	SrchCudp				(char *);
void	SrchExaf				(char *);
void	SrchExsf				(char *);
void	SrchExsi				(char *);
void	SrchInsf				(char *,int);
void	SrchInum				(char *);
void	SrchPay					(void);
void	SrchPrice				(void);
void	SrchSell				(void);
void	SrchTrcm				(char *);
void	SrchTrzm				(char *);
void	TidySonsScreen			(void);
void	Update				 	(void);
void	UpdateSONS			 	(int, int, long);
void	UpdateSoicQty		 	(void);
void	shutdown_prog		 	(void);
void	tab_other			 	(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		printerNumber;
	int		i;
	int		cnt;
	int		first_time = TRUE;
	int		field;
	int		key;

	_win_func = TRUE;

	currentUser = getenv ("LOGNAME");

	TruePosition	=	TRUE;

	progPid = (long) getpid ();

	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	X_EALL = 3;
	Y_EALL = 3;

	SETUP_SCR (vars);


	if (!strcmp (argv [0],"so_invoice"))
	{
		programRunType = INPUT;
		strcpy (recordLockFlag,"u");
	}
	else
	{
		if (!strcmp (argv [0],"so_invdisp") || 
		     !strcmp (argv [0],"so_sinvdisp"))
		{
			if (!strcmp (argv [0],"so_sinvdisp"))
				specialDisplay = TRUE;
			else
				specialDisplay = FALSE;

			programRunType = DISPLAY;
			strcpy (recordLockFlag,"r");
		}
		else
		{
			programRunType = MAINT;
			strcpy (recordLockFlag,"u");
		}
	}
	if (argc < 5)
	{
		print_at (0,0,"Usage : %s <batch_flag> <createStatusFlag> <invoiceTypeFlag> <text_file> - optional <printerNumber>\007\n",argv [0]);
		print_at (1,0,"<batch_flag> - Y(es By Batch\n\r");
		print_at (2,0,"             - N(o\n\r");
		print_at (3,0,"<invoiceTypeFlag>  - I (nvoice\n\r");
		print_at (4,0,"             - P (acking Slip\n\r");
		print_at (5,0,"             - S (tandard Invoice\n\r");
		print_at (6,0,"             - D (elivery Order\n\r");
		print_at (7,0,"<text_file_name>\n\r");
		return (EXIT_FAILURE);
	}


	switch (argv [1][0])
	{
	case	'Y':
	case	'y':
		processingByBatch = TRUE;
		break;

	case	'N':
	case	'n':
		processingByBatch = FALSE;
		break;

	default:
		print_at (0,0,"<batch_flag> - Y(es By Batch\n\r");
		print_at (1,0,"             - N(o\n\r");
		return (EXIT_FAILURE);
	}

	sprintf (createStatusFlag,"%-1.1s",argv [2]);

	switch (argv [3][0])
	{
	case	'I':
	case	'i':
		strcpy (invoiceTypeFlag,"I");
		promptType = 0;
		break;

	case	'P':
	case	'p':
		strcpy (invoiceTypeFlag,"P");
		promptType = 1;
		programRunType = DISPLAY;
		strcpy (recordLockFlag,"r");
		vars [label ("invoice_no")].just = JUSTRIGHT;
		break;

	case	'T':
	case	't':
		strcpy (invoiceTypeFlag,"T");
		promptType = 1;
		programRunType = DISPLAY;
		strcpy (recordLockFlag,"r");
		vars [label ("invoice_no")].just = JUSTRIGHT;
		break;

	case	'D':
	case	'd':
		strcpy (invoiceTypeFlag,"P");
		promptType = 3;
		break;

	case	'S':
	case	's':
		promptType = 2;
		strcpy (invoiceTypeFlag,"S");
		break;

	default:
		print_at (3,0,"<invoiceTypeFlag>  - I (nvoice\n\r");
		print_at (4,0,"             - P (acking Slip\n\r");
		print_at (5,0,"             - S (tandard Invoice\n\r");
		print_at (6,0,"             - D (elivery Order\n\r");
		print_at (7,0,"<text_file_name>\n\r");
		return (EXIT_FAILURE);
	}


	tab_row = 6;

	sprintf (local_rec.inv_prmpt,"%s",PROMPT);

	printerNumber = (argc == 6) ? atoi (argv [5]) : 0;

	strcpy (local_rec.dflt_ord,"D");
	strcpy (local_rec.dflt_batch,"00000");

	InitML ();

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	if (specialDisplay &&	argc < 7)
		specialDisplay = FALSE;

	/*---------------------------------
	| Open Output to Counter Printer. |
	---------------------------------*/
	if (argc == 6 && printerNumber) 
	{
		_ip_open (comm_rec.co_no, 
				  comm_rec.est_no, 
				  printerNumber, 
				  "SO_CTR_DEL",
				  "so_ctr_del");
	}


	if (STANDARD)
	{
		vars [label ("invoice_no")].fill = " ";
		vars [label ("invoice_no")].lowval = alpha;
	}
	else
	{
		if (programRunType != DISPLAY)
			vars [label ("invoice_no")].lowval = norm_invoice;
	}

	init_scr ();

	inputSaleValue	= FLD ("sale_price") ;
	inputDescValue	= FLD ("descr") ;

	_set_masks (argv [4]);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_ITEMS, store, sizeof (struct storeRec));
#endif

	if (envVar.gstApplies)
	{
		FLD ("tax_code") = ND;
		FLD ("tax_no")   = ND;
	}

	if	((invoiceTypeFlag [0] == 'P')   || (invoiceTypeFlag [0] == 'P'))   
		FLD ("item_no") = NE;

	if (INV_DISPLAY)
	{
		for (field = label ("cus_ord_ref");FIELD.scn != 0;field++)
			if (FIELD.required != ND)
				FIELD.required = NA;
	}

	/*-----------------------------------------------
	| Update to No Input if fields has been set to  |
	| input as prog is will not work this way.      |
	-----------------------------------------------*/
	if (FLD ("inp_total") == YES || FLD ("inp_total") == NO)
		FLD ("inp_total") = NI;

	if (envVar.serialItemsOk)
	{
		FLD ("ser_no")	=	YES;
		FLD ("hide")	=	ND;
	}
	else
		FLD ("ser_no")	=	ND;

	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];

	inputDiscValue = 0;
	inputSaleValue = 0;

	inputSaleValue = FLD ("sale_price");
	inputDiscValue = FLD ("disc");

	no_edit (5);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.longSystemDate = TodaysDate ();
	local_rec.dflt_inv_no = 0L;
	strcpy (manual_pref, "  ");	


	strcpy (defaultBranchNo, (envVar.dbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	/*----------------------------------
	| Open Discount and Pricing files. |
	----------------------------------*/
	OpenPrice ();
	OpenDisc ();

	strcpy (local_rec.prev_inv_no,"00000000");
	strcpy (local_rec.prev_dbt_no,"000000");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		set_tty (); 
		strcpy (local_rec.dbt_date,DateToString (comm_rec.dbt_date));

		if (envVar.useSystemDate)
			strcpy (local_rec._date_raised,local_rec.systemDate);
		else
			strcpy (local_rec._date_raised,local_rec.dbt_date);

		strcpy (local_rec._date_required,local_rec.systemDate);

		sprintf (local_rec.inv_prmpt,"%s",PROMPT);

		if (INV_INPUT && !F_HIDE (label ("disc_over")))
			FLD ("disc_over") = NI;

		if (restart) 
		{
			if (SK_BATCH_CONT || MULT_LOC)
				AllocationRestore ();

			if (first_time && INV_INPUT)
			{
				strcpy (local_rec.dflt_batch,"00000");

				if (envVar.useSystemDate)
					strcpy (local_rec._date_raised,local_rec.systemDate);
				else
					strcpy (local_rec._date_raised,local_rec.dbt_date);

				strcpy (local_rec._date_required,local_rec.systemDate);
				strcpy (local_rec.dflt_ord,"D");

				if (!F_NOKEY (label ("batch_no")))
					FLD ("batch_no") = YES;

				if (!F_NOKEY (label ("date_raised")))
					FLD ("date_raised") = YES;

				if (!F_NOKEY (label ("ord_type")))
					FLD ("ord_type") = YES;
			}
			/*-----------------------------------
			| Free any outstanding insfRecords |
			-----------------------------------*/
			if (line_cnt > lcount [SCN_ITEMS])
				lcount [SCN_ITEMS] = line_cnt;

			for (i = 0; i < lcount [SCN_ITEMS]; i++)
			{
				if (strcmp (store [i].serialItem, ser_space))
				{
					cc	=	UpdateInsf 
							(
								store [i].hhwhHash,
								0L,
								store [i].serialItem,
								"C",
								"F"
							);

					if (cc && cc < 1000)
						file_err (cc, insf, "DBUPDATE");
				}
			}

			/*----------------------------------
			| Clear soic records for this PID. |
			----------------------------------*/
			ClearSoic ();
		}

		for (cnt = 0; cnt < MAX_SONS; cnt++)
			sprintf (HeaderDesc [cnt], "%40.40s", " ");

		for (i = 0; i < MAXLINES; i++)
		{

			memset ((char *) &store [i], '\0', sizeof (struct storeRec));
			strcpy (store [i].priceOveride, "N");
			strcpy (store [i].discOveride, "N");
			strcpy (store [i].serialItem, ser_space);

			for (cnt = 0; cnt < MAX_SONS; cnt++)
				sprintf (store [i].nsDesc [cnt], "%40.40s", " ");
		}

		kitHash = 0;
		noTaxCharged		= 0;
		lcount [SCN_ITEMS]	= 0;
		insertLineFlag		= 0;
		totalInvoice		= 0.00;
		totalLevy			= 0.00;
		totalDisc			= 0.00;
		totalTax			= 0.00;
		totalGrand			= 0.00;
		totalGst			= 0.00;

		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		pastEntry	= FALSE;
		restart		= FALSE;
		init_vars (SCN_HEADER);	

		if (!specialDisplay)
		{
			heading (SCN_HEADER);
			entry (SCN_HEADER);
			if (prog_exit || restart)
				continue;
		}
		else
		{
			if (LoadDisplay (argv [6]))
			{
				clear ();
				box (0,0, 36, 1);
				print_at (1,1,"%R %s ", ML (mlStdMess016));
		
				fflush (stdout);
				sleep (sleepTime);
				shutdown_prog ();
                return (EXIT_FAILURE);
			}
			heading (SCN_HEADER);
			scn_display (SCN_HEADER);
		}
		if (newInvoice)
		{
			line_cnt = -1;
			heading (SCN_ITEMS);
			entry (SCN_ITEMS);
			if (restart)
				continue;
		}
		else
			scn_display (SCN_HEADER);

		prog_status = !(ENTRY);

		if (FREIGHT_CHG && INV_INPUT && newInvoice)
		{
			heading (SCN_FREIGHT);
			scn_display (SCN_FREIGHT);
			init_ok = FALSE;
			entry (SCN_FREIGHT);
			init_ok = TRUE;
		}
		do {
			edit_all ();
		} while (INV_INPUT && !restart && CheckKitting ());

		pastEntry = TRUE;
		ForceKitCompletion ();

		if (specialDisplay)
			break;

		if (INV_DISPLAY)
			continue;

		if (restart)
			continue;
		
		if (processingByBatch)
		{
			CalculateExtendedTotal (TRUE);
			heading (5);
			entry (5);

			while (invoiceProofFlag)
			{
				do {
					edit_all ();
				} while (!restart && CheckKitting ());

				ForceKitCompletion ();

				if (restart)
					break;

				heading (5);
				scn_display (5);
				entry (5);
				if (restart)
					break;
			}
		}
		else
			invoiceProofFlag = 0;

		if (invoiceProofFlag == 0 && !restart) 
		{
			Update ();

			if (SK_BATCH_CONT || MULT_LOC)
				AllocationComplete ();

			if (argc == 6 && printerNumber)
			{
				print_at (2,0,ML (mlSoMess304));

				key = prmptmsg ((mlSoMess304), alpha, 0,2);
				if (key == 'P' || key == 'p')
				{
					print_at (2,80,ML (mlStdMess035));
					ip_print (cohr_rec.hhco_hash);
				}
			}

			if (!F_HIDE (label ("batch_no")) && 
			     !F_NEED (label ("batch_no")))
				FLD ("batch_no") = NI;

			if (!F_HIDE (label ("date_raised")) &&
			     !F_NEED (label ("date_raised")))
				FLD ("date_raised") = NI;

			if (!F_HIDE (label ("ord_type")) &&
			     !F_NEED (label ("ord_type")))
				FLD ("ord_type") = NI;

			strcpy (local_rec.dflt_batch,cohr_rec.batch_no);
			strcpy (local_rec.dflt_ord,cohr_rec.ord_type);
			strcpy (local_rec._date_raised,DateToString (cohr_rec.date_raised));
			first_time = FALSE;
		}
	}
	/*---------------------------------
	| Open Output to Counter Printer. |
	---------------------------------*/
	if (argc == 6 && printerNumber) 
		ip_close ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (void)
{
	clear ();
	if (processingByBatch)	
	{
		print_at (0,0,ML (mlSoMess228),DOLLARS (batch_tot));
		if (envVar.dbMcurr)
			print_at (0,30,"(%-3.3s) ", envVar.currencyCode);
		PauseForKey (1,0,ML (mlStdMess042),0);
	}
	CloseCosting ();

#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (wout);
		IP_CLOSE (functionFileNo);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	ReadMisc ();

	abc_alias (cumr2, cumr);
	abc_alias (inum2, inum);
	open_rec (cumr2,cumr_list,CUMR_NO_FIELDS,"cumr_hhcu_hash");

	open_rec (cumr,cumr_list,CUMR_NO_FIELDS, (!envVar.dbFind) ? "cumr_id_no" 
							  : "cumr_id_no3");

	abc_alias (cohr2, cohr);
	abc_alias (soic2, soic);

	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncl,  cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (cudp,  cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (soic,  soic_list, SOIC_NO_FIELDS, "soic_id_no2");
	open_rec (soic2, soic_list, SOIC_NO_FIELDS, "soic_id_no");
	open_rec (sons,  sons_list, SONS_NO_FIELDS, "sons_id_no2");
    open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
    open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	if (envVar.advertLevy)
		open_rec (inal,inal_list,INAL_NO_FIELDS,"inal_id_no");

	OpenLocation (ccmr_rec.hhcc_hash);
	OpenInsf ();

	if (llctInvoice [0] == 'V')
		LotSelectFlag	=	INP_VIEW;
	if (llctInvoice [0] == 'A')
		LotSelectFlag	=	INP_AUTO;
	if (llctInvoice [0] == 'M')
	{
		strcpy (StockTake, "Y");
		LotSelectFlag	=	INP_VIEW;
	}
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose (cnch);
	abc_fclose (cncl);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cucc);
	abc_fclose (cudp);
	abc_fclose (cuit);
	abc_fclose (esmr);
	abc_fclose (exaf);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (soic);
	abc_fclose (soic2);
	abc_fclose (sons);
    abc_fclose (inum);
    abc_fclose (inum2);
	if (envVar.advertLevy)
		abc_fclose (inal);

	CloseLocation ();

	/*-----------------------------------
	| Close discount and pricing files. |
	-----------------------------------*/
	ClosePrice ();
	CloseDisc ();

	SearchFindClose ();
	abc_dbclose (data);
}

/*========================================
| Initialise misc text for multilingual. |
========================================*/
void
InitML (void)
{
	int		i;

	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		strcpy (err_str, STerms [i]._sdesc);
		strcpy (STerms [i]._sdesc, strdup (err_str));
	}
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");

	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (comr);
	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	int		i = 0;
	int		this_page;
	int		val_pterms = FALSE;
	double	total_owing = 0.00;
	long	CheckDate = 0L;
	int		TempLine;


	/*
	 * Validate Customer Number.
	 */
	if (LCHECK ("debtor")) 
	{
		if (INV_DISPLAY && dflt_used)
		{
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}
		skip_entry = 0;
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,defaultBranchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*--------------------------------------
		| Check if customer is on stop credit. |
		--------------------------------------*/
		if (STANDARD && cumr_rec.stop_credit [0] == 'Y')
		{
			if (envVar.creditStop)
			{
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				strcpy (err_str,ML ("Customer is on stop credit."));
				cc = WarnUser (err_str,0,2);
				if (cc)
					return (cc);
			}
		}
		total_owing = cumr_balance [0] + cumr_balance [1] +
					  cumr_balance [2] + cumr_balance [3] +
					  cumr_balance [4] + cumr_balance [5];

		creditRemain = total_owing - cumr_rec.credit_limit;

		/*---------------------------------------------
		| Check if customer is over his credit limit. |
		---------------------------------------------*/
		if (STANDARD && cumr_rec.credit_limit <= total_owing && 
			        cumr_rec.credit_limit != 0.00)
		{
			if (envVar.creditOver)
			{
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML ("Customer is over the credit limit."),0,2);
				if (cc)
					return (EXIT_FAILURE);

			}
		}
		/*-----------------------
		| Check Credit Terms	|
		-----------------------*/
		if (STANDARD && cumr_rec.od_flag)
		{
			if (envVar.creditTerms)
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str,ML ("Customer credit terms exceeded by %d period (s)."), cumr_rec.od_flag);
				cc = WarnUser (err_str,0,2);
				if (cc)
					return (EXIT_FAILURE);
			}
			
		}
		strcpy (local_rec.headOfficeCustomer, "N/A   ");
		if (cumr_rec.ho_dbt_hash != 0L)
		{
			cumr2_rec.hhcu_hash	=	cumr_rec.ho_dbt_hash;
			cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
			if (!cc)
				strcpy (local_rec.headOfficeCustomer, cumr2_rec.dbt_no);
		}
		DSP_FLD ("headOfficeAccount");
		DSP_FLD ("name");
		DSP_FLD ("cus_addr1");
		DSP_FLD ("cus_addr2");
		DSP_FLD ("cus_addr3");
		DSP_FLD ("cus_addr4");

		/*--------------------------
		| Process delivery address |
		--------------------------*/
		sprintf (cohr_rec.dl_name, "%-40.40s", cumr_rec.dbt_name);
		sprintf (cohr_rec.dl_add1, "%-40.40s", cumr_rec.dl_adr1);
		sprintf (cohr_rec.dl_add2, "%-40.40s", cumr_rec.dl_adr2);
		sprintf (cohr_rec.dl_add3, "%-40.40s", cumr_rec.dl_adr3);
		DSP_FLD ("del_name");
		DSP_FLD ("del_addr1");
		DSP_FLD ("del_addr2");
		DSP_FLD ("del_addr3");

		use_window (FN14);
		strcpy (local_rec.dflt_sale_no, (!strcmp (ccmr_rec.sman_no,"  ")) ? cumr_rec.sman_code : ccmr_rec.sman_no);
		
		/*-------------------------------
		| if cus_ord_ref must be input	|
		-------------------------------*/
		if (programRunType != DISPLAY)
		{
			if (cumr_rec.po_flag [0] == 'Y')
				FLD ("cus_ord_ref") = YES;
			else
				FLD ("cus_ord_ref") = (F_NOKEY (label ("cus_ord_ref"))) ? NA : YES;
		}

		/*----------------------------------
		| Get currency code and exch rate. |
		----------------------------------*/
		if (envVar.dbMcurr)
		{
			strcpy (pocrRec.co_no, comm_rec.co_no);
			sprintf (pocrRec.code, "%-3.3s", cumr_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			sprintf (local_rec.curr_code, "(%-3.3s)", cumr_rec.curr_code);
			DSP_FLD ("curr_code");
		}
		else
			pocrRec.ex1_factor = 1.00;

		return (EXIT_SUCCESS);
	}
		
	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ("invoice_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.inv_no,"00000000");
			DSP_FLD ("invoice_no");
		}

		if (dflt_used)
		{
			if (SysGenInv)
				strcpy (cohr_rec.inv_no,"00000000");
			else
				sprintf (cohr_rec.inv_no,"%-2.2s%06ld", manual_pref, local_rec.dflt_inv_no);
			DSP_FLD ("invoice_no");
		}
		/*---------------------------
		| Maintaining Sales Orders	|
		---------------------------*/
		if (INV_DISPLAY || SRCH_KEY || 
		  (strcmp (cohr_rec.inv_no,"00000000") && 
		      strcmp (cohr_rec.inv_no,"        ")))
		{
			if (SRCH_KEY)
			{
				if (INV_DISPLAY && cumr_rec.hhcu_hash == 0L)
					abc_selfield (cohr,"cohr_id_no2");
				else
					abc_selfield (cohr,"cohr_id_no");

				SrchCohr (temp_str);
				abc_selfield (cohr,"cohr_id_no2");
				return (EXIT_SUCCESS);
			}
			/*------------------------------
			| Check if invoice is on file. |
			------------------------------*/
			strcpy (cohr_rec.co_no,comm_rec.co_no);
			strcpy (cohr_rec.br_no,comm_rec.est_no);
			strcpy (cohr_rec.type,invoiceTypeFlag);
			cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
			strcpy (cohr_rec.inv_no, zero_pad (cohr_rec.inv_no, 8));
			cc = find_rec (cohr,&cohr_rec,COMPARISON,recordLockFlag);
			if (cc)
			{
				if (INV_DISPLAY)
				{
					print_mess (ML (mlStdMess115));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE); 
				}
				newInvoice = 1;
				SetInvoiceDefaults (newInvoice);
				return (EXIT_SUCCESS);
			}

			if (INV_DISPLAY && cumr_rec.hhcu_hash == 0L)
			{
				abc_selfield (cumr,"cumr_hhcu_hash");
				cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
				cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
				if (!cc)
				{
					strcpy (local_rec.cust_no,cumr_rec.dbt_no);
					DSP_FLD ("debtor");
					DSP_FLD ("name");
				}
				abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
			}

			if (cumr_rec.hhcu_hash != cohr_rec.hhcu_hash)
			{
				print_mess (ML (mlSoMess075));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}

			if (INV_INPUT && cohr_rec.stat_flag [0] != createStatusFlag [0])
			{
				abc_unlock (cohr);
				print_mess (ML (mlSoMess076));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}

			if (LoadItemScreen (cohr_rec.hhco_hash))
				return (EXIT_FAILURE);

			strcpy (local_rec.spinst [0],cohr_rec.din_1);
			strcpy (local_rec.spinst [1],cohr_rec.din_2);
			strcpy (local_rec.spinst [2],cohr_rec.din_3);
			newInvoice = 0;
			entry_exit = 1;
		}
		else
			newInvoice = 1;
		
		SetInvoiceDefaults (newInvoice);
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Charge To Number. |
	----------------------------*/
	if (LCHECK ("chargeToCustomer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.chargeToCustomer, "      ");
			strcpy (local_rec.chargeToName, " ");
			cohr_rec.chg_hhcu_hash	=	0L;
			DSP_FLD ("chargeToCustomer");
			DSP_FLD ("chargeToName");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			cohr_rec.chg_hhcu_hash	=	0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr2_rec.co_no,comm_rec.co_no);
		strcpy (cumr2_rec.est_no,defaultBranchNo);
		strcpy (cumr2_rec.dbt_no,zero_pad (local_rec.chargeToCustomer, 6));
		cc = find_rec (cumr,&cumr2_rec,COMPARISON,"r");
		if (cc)
		{
			/*---------------------
			| Customer not found. |
			---------------------*/
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		cohr_rec.chg_hhcu_hash	=	cumr2_rec.hhcu_hash;
		strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
		sprintf (local_rec.chargeToName, "%-35.35s", cumr2_rec.dbt_name);
		DSP_FLD ("chargeToCustomer");
		DSP_FLD ("chargeToName");
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| validate debtors contract |
	---------------------------*/
	if (LCHECK ("cont_no"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (local_rec.cont_desc, " ");
			strcpy (cnch_rec.exch_type, " ");
			cnch_rec.hhch_hash	=	0L;
			return (EXIT_SUCCESS);
		}

		if ((local_rec.inp_total || local_rec.disc_over) && dflt_used == FALSE)
		{
			print_mess (ML (mlSoMess243));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.cont_desc, " ");
			strcpy (cnch_rec.exch_type, " ");
			cnch_rec.hhch_hash	=	0L;
			return (EXIT_SUCCESS);
		}
		
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		CheckDate = (prog_status == ENTRY) ? comm_rec.dbt_date : cohr_rec.date_raised;
		/*------------------------------------------
		| now see if contract is still current.    |
		------------------------------------------*/
		if (cnch_rec.date_exp < CheckDate)
		{
			sprintf (err_str, 
					ML (mlSoMess270), 
					DateToString (cnch_rec.date_exp));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cnch_rec.date_wef > CheckDate)
		{
			sprintf (err_str, 
					ML (mlSoMess271), 
					DateToString (cnch_rec.date_wef));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*-------------------------------------------
		| now see if contract is assigned to debtor |
		-------------------------------------------*/
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlSoMess275));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.cont_desc, "%-30.30s", cnch_rec.desc);
		DSP_FLD ("cont_desc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("batch_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (cohr_rec.batch_no,local_rec.dflt_batch);

		DSP_FLD ("batch_no");
		sprintf (local_rec.pri_desc, "%-15.15s", cumr_rec.price_type);
		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type) - 1));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cus_ord_ref")) 
	{
		if (!strcmp (cohr_rec.cus_ord_ref,twenty_spaces))
		{
			if (cumr_rec.po_flag [0] == 'Y')
			{
				print_mess (ML (mlStdMess280));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dp_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (cohr_rec.dp_no,cumr_rec.department);

		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,cohr_rec.dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		PrintCompanyDetails ();
		DSP_FLD ("dp_no");
		DSP_FLD ("dp_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("del_name") || LNCHECK ("del_addr", 8))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (cohr_rec.dl_name, "%-40.40s", cumr_rec.dbt_name);
			sprintf (cohr_rec.dl_add1, "%-40.40s", cumr_rec.dl_adr1);
			sprintf (cohr_rec.dl_add2, "%-40.40s", cumr_rec.dl_adr2);
			sprintf (cohr_rec.dl_add3, "%-40.40s", cumr_rec.dl_adr3);
			DSP_FLD ("del_name");
			DSP_FLD ("del_addr1");
			DSP_FLD ("del_addr2");
			DSP_FLD ("del_addr3");
		}

		if (SRCH_KEY)
		{
			open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");

			i = SrchCudi (field - label ("del_name"));

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (cohr_rec.dl_name, cudi_rec.name);
			strcpy (cohr_rec.dl_add1, cudi_rec.adr1);
			strcpy (cohr_rec.dl_add2, cudi_rec.adr2);
			strcpy (cohr_rec.dl_add3, cudi_rec.adr3);
		}
		skip_entry = goto_field (field, label ("batch_no"));
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Payment Terms. |
	-------------------------*/
	if (LCHECK ("pay_term"))
	{
		val_pterms = FALSE;

		if (SRCH_KEY)
		{
			SrchPay ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms,"%-40.40s",p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_term");

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Selling Terms. |
	-------------------------*/
	if (LCHECK ("sell_terms"))
	{
		if (SRCH_KEY)
		{
			SrchSell ();
			return (EXIT_SUCCESS);
		}

		for (i = 0; (int) strlen (STerms [i]._scode);i++)
		{
			if (!strncmp (cohr_rec.sell_terms,STerms [i]._scode,strlen (STerms [i]._scode)))
			{
				sprintf (local_rec.sell_desc,"%-30.30s",STerms [i]._sdesc);
				break;
			}
		}

		if (!strlen (STerms [i]._scode))
		{
			print_mess (ML (mlSoMess246));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		DSP_FLD ("sell_terms");
		DSP_FLD ("sell_desc");

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Tax Code. |
	--------------------*/
	if (LCHECK ("tax_code")) 
	{
		if (F_HIDE (label ("tax_code")))
		{
			strcpy (cohr_rec.tax_code,cumr_rec.tax_code);
			if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
				noTaxCharged = 1;
			else
				noTaxCharged = 0;
			return (EXIT_SUCCESS);
		}

		if (cohr_rec.tax_code [0] == 'C' || cohr_rec.tax_code [0] == 'D')
			FLD ("tax_no") = NO;
		else
			FLD ("tax_no") = YES;

		if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
			noTaxCharged = 1;
		else
			noTaxCharged = 0;

		DSP_FLD ("tax_no");
		return (EXIT_SUCCESS);
	}
	/*------------------
	| Validate Tax No. |
	------------------*/
	if (LCHECK ("tax_no")) 
	{
		if (F_HIDE (label ("tax_no")))
		{
			strcpy (cohr_rec.tax_no,cumr_rec.tax_no);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (cohr_rec.tax_no,"               ") && 
		     FLD ("tax_no") == YES)
		{
			print_mess (ML (mlStdMess200));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Processed Date. |
	--------------------------*/
	if (LCHECK ("date_raised")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			cohr_rec.date_raised = StringToDate (local_rec._date_raised);

		if (FLD ("date_raised") == NA || FLD ("date_raised") == ND)
			return (EXIT_SUCCESS);

		while (chq_date (cohr_rec.date_raised,comm_rec.dbt_date))
		{
			get_entry (label ("date_raised"));
			cohr_rec.date_raised = StringToDate (local_rec._date_raised);
		}

		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Price type. |
	----------------------*/
	if (LCHECK ("pri_type"))
	{
		int		junk;

		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (local_rec.pri_desc, cumr_rec.price_type);

		if (SRCH_KEY)
		{
			SrchPrice ();
			strcpy (local_rec.pri_desc,"               ");

			return (EXIT_SUCCESS);
		}

		junk = atoi (clip (local_rec.pri_desc));

		if (junk < 1 || junk > envVar.numberPrices)
			return (EXIT_FAILURE);

		sprintf (cohr_rec.pri_type,"%-1.1s",clip (local_rec.pri_desc));
		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type) - 1));
		DSP_FLD ("pri_type");
		DSP_FLD ("pri_type_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ord_type")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (local_rec.ord_desc,local_rec.dflt_ord);

		strcpy (local_rec.ord_fulldesc, (local_rec.ord_desc [0] == 'D') 
						? "Domestic" : "Export  ");
		sprintf (cohr_rec.ord_type,"%-1.1s",local_rec.ord_desc);
		DSP_FLD ("ord_type");
		DSP_FLD ("ord_type_desc");
	}
	if (LCHECK ("prt_price")) 
	{
		if (prog_status == ENTRY && F_NOKEY (label ("prt_price")))
		{
			strcpy (cohr_rec.prt_price, "Y");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("sale_code")) 
	{
		open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.sale_code,local_rec.dflt_sale_no);
			DSP_FLD ("sale_code");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("sman_desc");
		abc_fclose (exsf);

		/*----------------------------------------
		| If sman is not displayed at line level |
		| and we a re in edit mode then force    |
		| all sman codes at line level to be the |
		| same as the header salesman code.      |
		----------------------------------------*/
		if (prog_status != ENTRY &&
		(FLD ("sman_code") == NA || FLD ("sman_code") == ND))
		{
			scn_set (SCN_ITEMS);
			for (i = 0 ; i < lcount [SCN_ITEMS]; i++)
			{
				getval (i);
				strcpy (coln_rec.sman_code, cohr_rec.sale_code);
				putval (i);
			}

			scn_set (SCN_HEADER);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("area_code")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.area_code,cumr_rec.area_code);
			DSP_FLD ("area_code");
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,cohr_rec.area_code);
		cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("area_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("inp_total")) 
	{
		if (strcmp (cohr_rec.cont_no, "      ") && local_rec.inp_total)
		{
			print_mess (ML (mlSoMess243));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("disc_over")) 
	{
		if (INV_DISPLAY)
			return (EXIT_SUCCESS);
		
		if (strcmp (cohr_rec.cont_no, "      ") && local_rec.disc_over)
		{
			print_mess (ML (mlSoMess243));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			CalculateBoxTotals (TRUE,FALSE);
			scn_set (SCN_HEADER);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. | 
	-----------------------*/
	if (LCHECK ("item_no")) 
	{
		int		cc1;

		if (INV_DISPLAY)
		{
			FLD ("item_no") = NE;
			entry_exit = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch 
			(
				comm_rec.co_no,
				temp_str,
				cumr_rec.hhcu_hash,
				cumr_rec.item_codes
			);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return (DeleteLine ());

		if (last_char == INSLINE)
			return (InsertLine ());

		if (strlen (clip (local_rec.item_no)) == 0)
			return (EXIT_FAILURE);

		cc1 = ValidateItemNumber (TRUE);
		if (cc1 == 0)
			RunningKit (line_cnt);

		if (strlen (clip (local_rec.item_desc)) == 0 || NON_STOCK (line_cnt)
			|| KIT_START || KIT_END)
			FLD ("descr")	=	YES;
		else
			FLD ("descr")	=	inputDescValue;

		return (cc1);
	}

	if (LCHECK ("descr")) 
	{
		if (NON_STOCK (line_cnt))
			skip_entry = goto_field (field,label ("extend"));

		tab_other (line_cnt);

		line_display ();
		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| Validate Salesman Code At Item Level. |
	---------------------------------------*/
	if (LCHECK ("sman_code")) 
	{
		open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
		if (prog_status == ENTRY)
		{
			strcpy (coln_rec.sman_code,cohr_rec.sale_code);
			DSP_FLD ("sman_code");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,coln_rec.sman_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}

		if (FIELD.required != ND)
			print_at (2,1,ML (mlStdMess202),exsf_rec.salesman_no,exsf_rec.salesman);
		
		abc_fclose (exsf);
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Customer Order Ref At Item Level. |
	--------------------------------------------*/
	if (LCHECK ("ord_ref")) 
	{
		if (prog_status == ENTRY)
		{
			strcpy (coln_rec.cus_ord_ref,cohr_rec.cus_ord_ref);
			DSP_FLD ("ord_ref");
		}

		if (FIELD.required != ND && prog_status != ENTRY)
		{
			if (!strcmp (coln_rec.cus_ord_ref,twenty_spaces))
			{
				if (cumr_rec.po_flag [0] == 'Y')
				{
					print_mess (ML (mlStdMess280));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pack_size")) 
	{
		if (dflt_used || (prog_status == ENTRY && F_NOKEY (field)))
		{
			strcpy (coln_rec.pack_size,SR.packSize);
			DSP_FLD ("pack_size");
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
			strcpy (local_rec.UOM, 	SR.UOM);

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, 	SR.uomGroup);
		strcpy (inum2_rec.uom, 			local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, 	inum2_rec.uom);
		strcpy (SR.UOM, 		inum2_rec.uom);
		SR.hhumHash 		= 	inum2_rec.hhum_hash;

		if (SR.stdConvFct == 0.00)
			SR.stdConvFct = 1.00;

		SR.convFct 	= inum2_rec.cnv_fct/SR.stdConvFct;
		PriceProcess ();
		DiscProcess ();	/*here*/
		DSP_FLD ("UOM");

		if (prog_status != ENTRY)
        {
            /*-------------------
            | Reenter Qty. Sup. |
            --------------------*/
            do
            {
                get_entry (label ("qty_sup"));
                cc = spec_valid (label ("qty_sup"));
            } while (cc);
        }
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qty_ord")) 
	{
		if (prog_status != ENTRY && local_rec.qty_ord < local_rec.qty_sup)
		{
			print_mess (ML (mlSoMess294));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);

		}
		SR.qtyOrd = ToLclUom (local_rec.qty_ord);
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Quantity Input. |
	--------------------------*/
	if (LCHECK ("qty_sup")) 
	{

 		if (prog_status == ENTRY && local_rec.qty_sup <= 0.00)
		{
			print_mess (ML (mlSoMess299));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

 		if (prog_status == EDIT && local_rec.qty_sup < 0.00)
 		{
 			print_mess (ML (mlSoMess300));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*--------------------------------------------------
		| Reset sale price and discount to default values. |
		--------------------------------------------------*/
		FLD ("sale_price")	= 	inputSaleValue;
		FLD ("disc")		= 	inputDiscValue;

		if (KIT_ITEM)
		{
			SR.qtySup = local_rec.qty_sup;
			this_page 	= line_cnt / TABLINES;
			ProcessKitItem 
			(
				inmr_rec.hhbr_hash,
				local_rec.qty_sup
			);
			memset (&local_rec, 0, sizeof (local_rec));
			memset (&coln_rec,  0, sizeof (coln_rec));
			putval (line_cnt);
			skip_entry = goto_field (label ("qty_ord"),
		                 	         label ("item_no")) - 1;
			local_rec.qty_ord = 0.00;

			if (this_page == (line_cnt / TABLINES))
				blank_display ();

			if (prog_status != ENTRY)
			{
				/*-------------------
				| Reenter Location. |
				--------------------*/
				do
				{
					strcpy (local_rec.LL, "N");
					get_entry (label ("LL"));
					cc = spec_valid (label ("LL"));
				} while (cc && !restart);
			}

			if (pastEntry || in_sub_edit)
			{
				entry_exit = 1;
				line_cnt = line_cnt + 4;
				line_cnt--;
			}
			return (EXIT_SUCCESS);
		}
		
		if (PHANTOM)
			SR.qtyAvail = ProcessPhantom (inmr_rec.hhbr_hash);

		if (NON_STOCK (line_cnt))
		{
			local_rec.qty_ord = 0.00;
			local_rec.qty_sup = 0.00;
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_sup");
			CalculateBoxTotals (FALSE,FALSE);
			skip_entry = goto_field (field,label ("extend"));
			return (EXIT_SUCCESS);
		}

		if (KIT_START)
		{
			SR.qtySup         	=   0.00;
			SR.salePrice      	=   0.00;
			SR.gSalePrice     	=   0.00;
			local_rec.qty_sup   =   0.00;
			CalculateBoxTotals (FALSE,FALSE);
			DSP_FLD ("qty_sup");
			return (EXIT_SUCCESS);
		}

		if (dflt_used && !F_HIDE (label ("qty_ord")))
			local_rec.qty_sup = local_rec.qty_ord;

		if (!F_HIDE (label ("qty_ord")) && local_rec.qty_sup > local_rec.qty_ord)
		{
			print_mess (ML (mlSoMess301));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*---------------------------------------------------
		| Serial Items Can only have Qty of 0.00 or 1.00	|
		---------------------------------------------------*/
		if (SERIAL && local_rec.qty_sup != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-------------------------------------------------
		| Recalculate the actual current available stock. |
		-------------------------------------------------*/
		if (!PHANTOM)
			SR.qtyAvail = ReCalcAvail ();

		PriceProcess ();
		DiscProcess (); 

		/*----------------------------------------------------
		| Validate to see if on hand is less than input qty. |
		----------------------------------------------------*/
		if (STANDARD && envVar.windowPopupOk 
				&& !KIT_END 
				&& ((SR.qtyAvail - ToStdUom (local_rec.qty_sup)) < 0.00) 
				&& !NO_COST && !NON_STOCK (line_cnt))
		{
			sprintf (err_str,ML (mlStdMess090),SR.qtyAvail,clip (local_rec.item_no));
			BusyFunction (BUSY_ON);
			cc = WarnUser (err_str,1,2);

			InputResponce (&local_rec.qty_sup);

			if (skip_entry != 0)
				return (EXIT_SUCCESS);

			if (prog_status == ENTRY)
			{
				if (!insertLineFlag)
					lcount [SCN_ITEMS] = line_cnt;
			}
		}

		if (prog_status == ENTRY && NO_COST && F_HIDE (label ("cost_price")))
		{
			BusyFunction (BUSY_OFF);
			print_at (2,1,ML (mlSoMess238));
			coln_rec.cost_price = getmoney (20,2,"NNNNNNN.NN");
		}
			
		if (F_HIDE (label ("qty_ord")))
			local_rec.qty_ord = local_rec.qty_sup;

		SR.qtySup = local_rec.qty_sup;

		if (prog_status != ENTRY)
			PrintExtendedTotal (line_cnt);

		if (KIT_END && Recalc_kit && NOT_ENTRY)
		{
			CalcKitLine ();
			coln_rec.sale_price     =   SR.extendTotal;
			if (local_rec.qty_sup)
				coln_rec.sale_price /=  local_rec.qty_sup;

			coln_rec.sale_price     =   no_dec (coln_rec.sale_price);
			coln_rec.gsale_price    =   coln_rec.sale_price;
			DSP_FLD ("pack_size");
			SR.salePrice          	=   coln_rec.sale_price;
			SR.gSalePrice         	=   coln_rec.sale_price;
		}

		if (!NO_COST && !F_HIDE (label ("cost_price")))
		{
			coln_rec.cost_price = 0.00;
			DSP_FLD ("cost_price");
		}
		if (BONUS || SR.contStatus)
			skip_entry = goto_field (field,label ("LL"));   /* "extend")); */
		else
		{
			if (NO_COST && F_HIDE (label ("cost_price")))
				skip_entry = 1;
			else
				skip_entry = 0;

			if (coln_rec.sale_price == 0.00)
				   FLD ("sale_price") = YES;
		}
		DSP_FLD ("disc");
		SR.qtySup = local_rec.qty_sup;

		RunningKit (line_cnt);
		if ((KIT_END && NOT_ENTRY) || !KIT_END)
			CalculateBoxTotals (FALSE,FALSE);

		MarginCheckOk (SR.actSale, SR.discPc, SR.margCost, SR.minMarg);

		/*----------------------------------------
		| Update the real-time committal record. |
		----------------------------------------*/

		UpdateSoicQty ();

		if (prog_status != ENTRY)
		{
			DSP_FLD ("qty_sup");

			/*-------------------
			| Reenter Location. |
			--------------------*/
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				cc = spec_valid (label ("LL"));
			} while (cc && !restart);
		}

		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Price Input.	|
	-----------------------*/
	if (LCHECK ("sale_price")) 
	{
		if (dflt_used)
		{
			strcpy (SR.priceOveride, "N");
			PriceProcess ();
			DiscProcess ();
			DSP_FLD ("sale_price");
		}

		if (BONUS)
		{
			print_mess (ML (mlSoMess233));
			sleep (sleepTime);
			clear_mess ();
			coln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");
			return (EXIT_SUCCESS);
		}

		if (KIT_START)
		{
			coln_rec.sale_price =   0.00;
			SR.gSalePrice     	=   0.00;
			SR.salePrice      	=   0.00;
			SR.actSale        	=   0.00;
			DSP_FLD ("sale_price");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			if (KIT_END)
			{
				CalcKitLine ();
				coln_rec.sale_price     =   SR.extendTotal;
				if (local_rec.qty_sup)
					coln_rec.sale_price /=  local_rec.qty_sup;

				coln_rec.sale_price     =   no_dec (coln_rec.sale_price);
				coln_rec.gsale_price    =   coln_rec.sale_price;
				SR.actSale            	=   coln_rec.sale_price;
				SR.gSalePrice         	=   coln_rec.sale_price;
				SR.salePrice          	=   coln_rec.sale_price;
				DSP_FLD ("sale_price");
			}
		}

		if (coln_rec.sale_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess031),"YyNn",1,2);
			BusyFunction (BUSY_OFF);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		SR.salePrice 	= coln_rec.sale_price;

		if (SR.calcSprice != coln_rec.sale_price)
			strcpy (SR.priceOveride, "Y");

		/*---------------------------------	
		| Calculate new GROSS sale price. |
		---------------------------------*/
		SR.gSalePrice = SR.salePrice / (1.00 - (SR.regPc / 100.00));
		SR.salePrice = GetCusGprice (SR.gSalePrice, SR.regPc);
		coln_rec.sale_price = SR.salePrice;

		if (prog_status == ENTRY ||
			(prog_status != ENTRY && !LineInKit (line_cnt)))
		{
			DiscProcess ();
		}

		SR.actSale = coln_rec.sale_price; 

		if (SR.taxAmount == 0.00)
			SR.taxAmount = SR.salePrice;

		PrintExtendedTotal (line_cnt);

		if (prog_status != ENTRY || !F_NOKEY (field))
		{
			if (LineInKit (line_cnt))
				RunningKit (line_cnt);

			CalculateBoxTotals (FALSE,FALSE);
		}

		MarginCheckOk (SR.actSale, SR.discPc, SR.margCost, SR.minMarg);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("disc")) 
	{
		if (LineInKit (line_cnt))
		{
			if (envVar.KitDiscount)
			{
				if (dflt_used)
				DiscProcess ();
			}
			else
			{
				coln_rec.disc_pc     = 0.00;
				SR.discPc          = 0.00;
				SR.calcDisc       = 0.00;
				SR.discA          = 0.00;
				SR.discB          = 0.00;
				SR.discC          = 0.00;
			}
			SR.discPc   = ScreenDisc (coln_rec.disc_pc);

			/*------------------------------
			| Discount has been entered so |
			| set disc B & C to zero.      |
			------------------------------*/
			if (!dflt_used)
			{
				SR.discA = SR.discPc;
				SR.discB = 0.00;
				SR.discC = 0.00;
			}
			DSP_FLD ("disc");
			PrintExtendedTotal (line_cnt);

			RunningKit (line_cnt);
			CalculateBoxTotals (FALSE, FALSE);
			MarginCheckOk
			(
				coln_rec.sale_price,
				SR.discPc,
				SR.margCost,
				SR.minMarg
			);
			return (EXIT_SUCCESS);
		}

		if (FLD ("disc") == NI && prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			if (KIT_END)
			{
				coln_rec.disc_pc     =   0.00;
				SR.discA          =   0.00;
				SR.discB          =   0.00;
				SR.discC          =   0.00;
			}
			else
			{
				strcpy (SR.discOveride, "N");
				DiscProcess ();
			}
		}
		
		if (SR.conPrice || SR.contStatus == 2)
		{
			coln_rec.disc_pc 	=	0.00;
			SR.discA			=	0.00;
			SR.discB			=	0.00;
			SR.discC			=	0.00;
			DSP_FLD ("disc");
		}
		SR.discPc = ScreenDisc (coln_rec.disc_pc);

		if (SR.calcDisc != ScreenDisc (coln_rec.disc_pc))
			strcpy (SR.discOveride, "Y");

		/*------------------------------
		| Discount has been entered so |
		| set disc B & C to zero.      |
		------------------------------*/
		if (!dflt_used)
		{
			SR.discA = SR.discPc;
			SR.discB = 0.00;
			SR.discC = 0.00;
		}
		PrintExtendedTotal (line_cnt);
		CalculateBoxTotals (FALSE, FALSE);

		MarginCheckOk (SR.actSale, SR.discPc, SR.margCost, SR.minMarg);
	}

	/*-------------------------------
	| Validate serial number input. |
	-------------------------------*/
	if (LCHECK ("ser_no")) 
	{
		if (F_HIDE (field) || FIELD.required == NA || SR.serialFlag [0] != 'Y')
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchInsf (temp_str,line_cnt);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (local_rec.qty_sup != 0.00)
			{
				print_mess (ML (mlStdMess201));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				if (strcmp (SR.serialItem, ser_space))
				{
					/*------------------------------------
					| Free previous serial number if any |
					------------------------------------*/
					if (strcmp (SR.serialItem, ser_space))
					{
						cc = UpdateInsf (SR.hhwhHash,0L,SR.serialItem,"C","F");
	
						if (cc && cc < 1000)
							file_err (cc, insf, "DBUPDATE");
					}
				}

				strcpy (local_rec.serial_no, ser_space);
				strcpy (SR.serialItem, local_rec.serial_no);
				return (EXIT_SUCCESS);
			}
		}
		/*
		 *	Check if serial number free.
		 */
		cc	=	FindInsf 
				(
					SR.hhwhHash, 
					0L,
					local_rec.serial_no,
					"F",
					"r"
				);
		if (cc)
		{
			cc	=	FindInsf 
					(
						0L,
						SR.hhbrHash, 
						local_rec.serial_no,
						"F",
						"r"
					);
		}
		if (cc)
		{
			/*
			 *	Check if serial number committed.
			 */
			cc	=	FindInsf 
					(
						SR.hhwhHash, 
						0L,
						local_rec.serial_no,
						"C",
						"r"
					);
			if (cc)
			{
				/*
				 *	Check if serial number Sold.
				 */
				cc	=	FindInsf 
						(
							SR.hhwhHash, 
							0L,
							local_rec.serial_no,
							"S",
							"r"
						);
				if (!cc)	
				{
					print_mess (ML (mlSoMess303));
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}
			}
			else
			{
				print_mess (ML (mlSoMess302));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (CheckDuplicateInsf (local_rec.serial_no, SR.hhsiHash, line_cnt))
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*------------------------------------
		| Free previous serial number if any |
		------------------------------------*/
		if (strcmp (SR.serialItem, ser_space))
		{
			cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialItem, "C", "F");

			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}

		strcpy (SR.serialItem, local_rec.serial_no);

		cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialItem, "F", "C");

		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");

		DSP_FLD ("ser_no");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("extend")) 
	{
		PrintExtendedTotal (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		int		LLReturnValue	=	0;

		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		if (KIT_START || KIT_END)
			return (EXIT_SUCCESS);

		TempLine	=	lcount [SCN_ITEMS];
		LLReturnValue = DisplayLL
			(										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.UOM,							/* UOM					*/
				SR.qtySup,						/* Quantity.			*/
				SR.convFct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				LotSelectFlag, 						/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR.lotControl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [SCN_ITEMS] = (line_cnt + 1 > lcount [SCN_ITEMS]) ? line_cnt + 1 : lcount [SCN_ITEMS];
		scn_write (SCN_ITEMS);
		scn_display (SCN_ITEMS);
		lcount [SCN_ITEMS] = TempLine;
		PrintCompanyDetails ();
		pr_box_lines (SCN_ITEMS);

		if (LLReturnValue)
			return (EXIT_FAILURE);

		if (BONUS || SR.contStatus)
			skip_entry = goto_field (field,label ("extend")); 
		
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("other_3")) 
	{
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Shipment Name And Addresses. |
	---------------------------------------*/
	if (!strcmp (FIELD.label,"shipname") || !strncmp (FIELD.label,"shipaddr",8))
	{
		if (SRCH_KEY)
		{
			open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");

			i = SrchCudi (field - label ("shipname"));

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (cohr_rec.dl_name,cudi_rec.name);
			strcpy (cohr_rec.dl_add1,cudi_rec.adr1);
			strcpy (cohr_rec.dl_add2,cudi_rec.adr2);
			strcpy (cohr_rec.dl_add3,cudi_rec.adr3);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("ship_method") || LCHECK ("spcode1") || LCHECK ("spcode2"))
	{
		i = field - label ("ship_method") ;

		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			abc_fclose (exsi);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		if (dflt_used)
		{
			if (i == 0)
				exsi_rec.inst_code = cumr_rec.inst_fg1;
			else if (i == 1)
				exsi_rec.inst_code = cumr_rec.inst_fg2;
			else if (i == 2)
				exsi_rec.inst_code = cumr_rec.inst_fg3;
		}
		else
			exsi_rec.inst_code = atoi (local_rec.spinst [i]);

		if (!find_rec (exsi,&exsi_rec,COMPARISON,"r"))
			sprintf (local_rec.spinst [i],"%-60.60s",exsi_rec.inst_text);

		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Freight Required Flag. |
	---------------------------------*/
	if (LCHECK ("freightRequiredFlag"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (FREIGHT_CHG)
		{
			CalculateFreight 
			(
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Carrier Code. |
	------------------------*/
	if (LCHECK ("carrierCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				cohr_rec.freight = 0.00;

			CalculateFreight 
			(
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			trcm_rec.trcm_hash	=	0L;
			return (EXIT_SUCCESS);
		}
			
		OpenTransportFiles ("trzm_trzm_hash");

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			CloseTransportFiles ();
			return (EXIT_SUCCESS);
		}
			
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
		if (!cc)
		{
			trcl_rec.trcm_hash = trcm_rec.trcm_hash;
			trcl_rec.trzm_hash = trzm_rec.trzm_hash;
			cc = find_rec (trcl, &trcl_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess134));
				sleep (sleepTime);
				clear_mess ();

				CloseTransportFiles ();
				trcm_rec.trcm_hash	=	0L;
				return (EXIT_FAILURE);
			}
			CloseTransportFiles ();

			DSP_FLD ("carr_desc");

			CalculateFreight 
			(
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			return (EXIT_SUCCESS);
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		clear_mess ();

		CloseTransportFiles ();
		return (EXIT_FAILURE);
	}

	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("deliveryZoneCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				cohr_rec.freight = 0.00;

			strcpy (trzm_rec.del_zone, "      ");
			strcpy (trzm_rec.desc, "      ");
			trzm_rec.trzm_hash	=	0L;
			trzm_rec.dflt_chg	=	0.0;
			trzm_rec.chg_kg		=	0.0;
			CalculateFreight 
			(
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			DSP_FLD ("deliveryZoneDesc");
			DSP_FLD ("deliveryZoneCode");
			return (EXIT_SUCCESS);
		}
		OpenTransportFiles ("trzm_id_no");

		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (trzm_rec.del_zone, cumr_rec.del_zone);
			DSP_FLD ("deliveryZoneCode");
		}
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			CloseTransportFiles ();
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			CloseTransportFiles ();
			return (EXIT_FAILURE); 
		}
		if (trcm_rec.trcm_hash > 0L)
		{
			strcpy (trcm_rec.co_no, comm_rec.co_no);
			strcpy (trcm_rec.br_no, comm_rec.est_no);
			cc = find_rec (trcm, &trcm_rec, COMPARISON,"r");
			if (!cc)
			{
				trcl_rec.trcm_hash = trcm_rec.trcm_hash;
				trcl_rec.trzm_hash = trzm_rec.trzm_hash;
				cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess134));
					sleep (sleepTime);
					clear_mess ();

					CloseTransportFiles ();
					return (EXIT_FAILURE);
				}
				CloseTransportFiles ();

				CalculateFreight 
				(
					trcm_rec.markup_pc, 
					trcl_rec.cost_kg,
					trzm_rec.chg_kg,
					trzm_rec.dflt_chg
				);

				DSP_FLD ("est_freight");
				DSP_FLD ("freight");
				DSP_FLD ("tot_kg");
				return (EXIT_SUCCESS);
			}
		}
		DSP_FLD ("deliveryZoneDesc");
		DSP_FLD ("deliveryZoneCode");

		CloseTransportFiles ();
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate delivery required. |
	-----------------------------*/
	if (LCHECK ("deliveryRequired"))
	{
		if (!newInvoice)
		{
			move (0,2);cl_line ();
			i = prmptmsg (ML (mlTrMess063) ,"YyNn",0,2);
			BusyFunction (0);
			if (i == 'N' || i == 'n') 
				return (EXIT_SUCCESS);

			sprintf (err_str,"tr_trsh_mnt I %010ld LOCK",cohr_rec.hhco_hash);
			sys_exec (err_str);
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhco_hash");
			trsh_rec.hhco_hash	=	cohr_rec.hhco_hash;
			cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (cohr_rec.s_timeslot, trsh_rec.sdel_slot);
				strcpy (cohr_rec.e_timeslot, trsh_rec.edel_slot);
				cohr_rec.del_date	=	trsh_rec.del_date;
				cc = abc_update (cohr, &cohr_rec);
				if (cc)
					file_err (cc, cohr, "DBUPDATE");
			}
			heading (SCN_FREIGHT);
			scn_write (SCN_FREIGHT);
			scn_display (SCN_FREIGHT);
			print_mess (ML (mlTrMess076));
			sleep (sleepTime);
			DSP_FLD ("deliveryDate");
		}
		return (EXIT_SUCCESS); 
	}
	/*----------------------
	| Validate Proof Total |
	----------------------*/
	if (LCHECK ("proof")) 
	{
		clear_mess ();
		invoiceProofFlag = 1;
		if (end_input) 
		{
			entry_exit = 1;
			return (EXIT_SUCCESS);
		}
		CalculateExtendedTotal (TRUE);
		/*------------------------
		| Reset to proof screen. |
		------------------------*/
		scn_set (SCN_PROOF);
		proofTotal = no_dec (proofTotal);
		invTotalAmt = no_dec (invTotalAmt);
		if (proofTotal < invTotalAmt - .01 || proofTotal > invTotalAmt + .01)
		{
			print_mess (ML (mlSoMess165));
			invoiceProofFlag = 1;
			return (EXIT_FAILURE); 
		}
		invoiceProofFlag = 0;
		if (envVar.dbMcurr)
			invTotalAmt = no_dec (invTotalAmt / pocrRec.ex1_factor);
		batch_tot += invTotalAmt;
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Process soic record for current line. |
---------------------------------------*/
void
ProcSoic (
 int	delLine,
 int	procLine)
{
	/*-------------------
	| Deleting a line ? |
	-------------------*/
	if (delLine)
	{
		/*------------------------------------
		| Find the soic record/s and delete. |
		------------------------------------*/
		if (store [procLine].hhbrHash == store [procLine].origHhbrHash)
		{
			/*-----------------------------------
			| Ensure whole line is decommitted. |
			-----------------------------------*/
			if (store [procLine].deCommitRef == 0)
			{
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = GenSoicRef ();

				soic_rec.hhbr_hash = store [procLine].origHhbrHash;
				soic_rec.hhcc_hash = ccmr_rec.hhcc_hash;
				soic_rec.qty = (float) (store [procLine].origOrdQty * -1.00);
				sprintf (soic_rec.program, "%-20.20s", PNAME);
				sprintf (soic_rec.op_id, "%-14.14s", currentUser);
				strcpy (err_str, TimeHHMMSS ());
				soic_rec.time_create = atot (err_str);
				soic_rec.date_create = TodaysDate ();
		
				cc = abc_add (soic2, &soic_rec);
				if (cc)
					file_err (cc, soic2, "DBADD");
			}	
			else
			{
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = store [procLine].deCommitRef;
				cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
				if (cc)
					file_err (cc, soic2, "DBFIND");

				soic_rec.qty = (float) (store [procLine].origOrdQty * -1.00);
				cc = abc_update (soic2, &soic_rec);
				if (cc)
					file_err (cc, soic2, "DBUPDATE");
			}
		}

		/*----------------------------
		| Remove commit soic record. |
		----------------------------*/
		strcpy (soic_rec.status, "A");
		soic_rec.pid  = progPid;
		soic_rec.line = store [procLine].commitRef;
		cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
		if (!cc)
		{
			cc = abc_delete (soic2);
			if (cc)
				file_err (cc, soic2, "DBDELETE");
		}

		return;
	}

	/*----------------------
	| Add new soic record. |
	----------------------*/
	if (store [procLine].commitRef == 0)
		AddNewSoic (procLine);
	else
	/*------------------------------
	| Update existing soic record. |
	------------------------------*/
	{
		/*--------------------------------------------
		| Soic exists but line not loaded from file. |
		--------------------------------------------*/
		if (store [procLine].origHhbrHash == 0L)
		{
			strcpy (soic_rec.status, "A");
			soic_rec.pid  = progPid;
			soic_rec.line = store [procLine].commitRef;
			cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, soic2, "DBFIND");

			soic_rec.hhbr_hash = store [procLine].hhbrHash;
			soic_rec.qty       = 0.00;

			cc = abc_update (soic2, &soic_rec);
			if (cc)
				file_err (cc, soic2, "DBUPDATE");
		}
		else
		/*----------------------------------------
		| Soic exists and line loaded from file. |
		----------------------------------------*/
		{
			/*------------------------------------------
			| We have changed the item back to what it |
			| was originally so remove any commit and  |
			| decommit records for the line.           |
			| (Commit records will be created if reqd  |
			|  when the qty is re-entered for the      |
			|  original item.)                         |
			------------------------------------------*/
			if (store [procLine].hhbrHash == store [procLine].origHhbrHash)
			{
				/*----------------------------
				| Remove commit soic record. |
				----------------------------*/
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = store [procLine].commitRef;
				cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
				if (!cc)
				{
					cc = abc_delete (soic2);
					if (cc)
						file_err (cc, soic2, "DBDELETE");
				}
				store [procLine].commitRef = -1;

				/*-------------------------------
				| Remove de-commit soic record. |
				-------------------------------*/
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = store [procLine].deCommitRef;
				cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
				if (!cc)
				{
					cc = abc_delete (soic2);
					if (cc)
						file_err (cc, soic2, "DBDELETE");
				}
				store [procLine].deCommitRef = 0;
			}
			else
			/*----------------------------------------
			| We have changed the item for this line |
			| therefore we need to de-commit the qty |
			| for the old item.                      | 
			----------------------------------------*/
			{	
				/*-------------------------------------------
				| Add a de-commit record for the total qty. |
				-------------------------------------------*/
				if (store [procLine].deCommitRef == 0)
				{
					strcpy (soic_rec.status, "A");
					soic_rec.pid  = progPid;
					soic_rec.line = GenSoicRef ();

					store [procLine].deCommitRef = soic_rec.line;
					soic_rec.hhbr_hash = store [procLine].origHhbrHash;
					soic_rec.hhcc_hash = ccmr_rec.hhcc_hash;
					soic_rec.qty = (float) (store [procLine].origOrdQty * -1.00);
					sprintf (soic_rec.program, "%-20.20s", PNAME);
					sprintf (soic_rec.op_id, "%-14.14s", currentUser);
					strcpy (err_str, TimeHHMMSS ());
					soic_rec.time_create = atot (err_str);
					soic_rec.date_create = TodaysDate ();
			
					cc = abc_add (soic2, &soic_rec);
					if (cc)
						file_err (cc, soic2, "DBADD");
				}	

				/*------------------------------------------------
				| Add / update a commit record for the new item. |
				------------------------------------------------*/
				if (store [procLine].commitRef == -1)
				{
					/*--------------------------------------------
					| Add a new commit soic record for new item. |
					--------------------------------------------*/
					AddNewSoic (procLine);
				}	
				else
				{
					/*------------------------------------------------------
					| Update the existing commit soic record for new item. |
					------------------------------------------------------*/
					strcpy (soic_rec.status, "A");
					soic_rec.pid  = progPid;
					soic_rec.line = store [procLine].commitRef;
					cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
					if (cc)
						file_err (cc, soic2, "DBFIND");
		
					soic_rec.hhbr_hash = store [procLine].hhbrHash;
					soic_rec.qty       = 0.00;
		
					cc = abc_update (soic2, &soic_rec);
					if (cc)
						file_err (cc, soic2, "DBUPDATE");
				}	
			}
		}
	}
}

void
AddNewSoic (
 int	lineNo)
{
	strcpy (soic_rec.status, "A");
	soic_rec.pid  = progPid;
	soic_rec.line = GenSoicRef ();

	store [lineNo].commitRef 	= soic_rec.line;
	soic_rec.hhbr_hash 			= store [lineNo].hhbrHash;
	soic_rec.hhcc_hash 			= 0L;
	soic_rec.qty 				= (float) 0.00;
	sprintf (soic_rec.program, 	"%-20.20s", PNAME);
	sprintf (soic_rec.op_id, 	"%-14.14s", currentUser);
	strcpy (err_str, TimeHHMMSS ());
	soic_rec.time_create = atot (err_str);
	soic_rec.date_create = TodaysDate ();

	cc = abc_add (soic2, &soic_rec);
	if (cc)
		file_err (cc, soic2, "DBADD");
}
/*--------------------------------------
| Generate new reference to be used as |
| a commitRef or a deCommitRef.        |
--------------------------------------*/
int
GenSoicRef (void)
{
	return (++nextSoicRef);
}
/*----------------------------------------
| Update soic quantity for current line. |
----------------------------------------*/
void
UpdateSoicQty (void)
{
	float	updQty;
	float	lineQty;

	/*---------------------------------------------------------
	| If it's the end of kit update all kit's item quantities |
	| in soic table.                                          |
	---------------------------------------------------------*/
	if (SR.kitFlag == K_END)
	{
		UpdateKitSoicQty (line_cnt);
		return;
	}

	lineQty = ToStdUom (local_rec.qty_sup);

	/*------------------------------------
	| Line is back at its original item. |
	------------------------------------*/
	if (store [line_cnt].hhbrHash == store [line_cnt].origHhbrHash)
	{
		/*----------------------------------------------
		| Qty differs but no soic record exists so add |
		| a commit soic record ready for qty update.   |
		----------------------------------------------*/
		if (store [line_cnt].commitRef == -1)
		{
			if (lineQty != store [line_cnt].origOrdQty)
			{
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = GenSoicRef ();
				store [line_cnt].commitRef = soic_rec.line;
				soic_rec.hhbr_hash = store [line_cnt].hhbrHash;
				soic_rec.hhcc_hash = 0L;
				soic_rec.qty = (float) 0.00;
				sprintf (soic_rec.program, "%-20.20s", PNAME);
				sprintf (soic_rec.op_id, "%-14.14s", currentUser);
				strcpy (err_str, TimeHHMMSS ());
				soic_rec.time_create = atot (err_str);
				soic_rec.date_create = TodaysDate ();
		
				cc = abc_add (soic2, &soic_rec);
				if (cc)
					file_err (cc, soic2, "DBADD");
			}
			else
				return;
		}
	}

	if (store [line_cnt].hhbrHash == store [line_cnt].origHhbrHash)
	{
		updQty = lineQty - store [line_cnt].origOrdQty;
		
	}
		
	else
		updQty = lineQty;

	/*-------------------------------------
	| Find the soic record for commitRef. |
	-------------------------------------*/
	strcpy (soic_rec.status, "A");
	soic_rec.pid = progPid;
	soic_rec.line = store [line_cnt].commitRef;
	cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, soic2, "DBFIND");

	soic_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	soic_rec.qty = updQty;


	cc = abc_update (soic2, &soic_rec);
	if (cc)
		file_err (cc, soic2, "DBUPDATE");
}

/*==========================================
| Update the current kit items' quantities |
==========================================*/
void
UpdateKitSoicQty (
 int	nLineCount)
{
	int		line_no;
	float	kitQty = ToStdUom (local_rec.qty_sup);
	float	lineQty;
	float	updQty;

	if (kitQty == store [nLineCount].origOrdQty)
		return;

	for (line_no = nLineCount - 1;
		 line_no >= 0 && store [line_no].kitFlag != K_START;
		 line_no--)
	{
		if (store [line_no].itemClass [0] == 'Z')
			continue;

		/*--------------------------
		| Compute actual quantity  |
		--------------------------*/
		lineQty = kitQty * ToStdUom (store [line_no].qtySup);

		if (store [line_no].hhbrHash == store [line_no].origHhbrHash)
		{
			updQty = lineQty - store [line_no].origOrdQty;
		}
	 	else
			updQty = lineQty;

		/*-------------------------------------
		| Find the soic record for commitRef. |
		-------------------------------------*/
		strcpy (soic_rec.status, "A");
		soic_rec.pid = progPid;
		soic_rec.line = store [line_no].commitRef;
		cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, soic2, "DBFIND");

		soic_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		soic_rec.qty = updQty;

		cc = abc_update (soic2, &soic_rec);
		if (cc)
			file_err (cc, soic2, "DBUPDATE");
	}
}

/*-------------------------------------
| Clear soic records for current PID. |
-------------------------------------*/
void
ClearSoic (void)
{
	strcpy (soic_rec.status, "A");
	soic_rec.pid  = progPid;
	soic_rec.line = 0;
	cc = find_rec (soic2, &soic_rec, GTEQ, "u");
	while (!cc && 
		   soic_rec.status [0] == 'A' &&
		   soic_rec.pid == progPid)
	{
		/*----------------
		| Delete record. |
		----------------*/
		cc = abc_delete (soic2);
		if (cc)
			file_err (cc, soic2, "DBDELETE");

		strcpy (soic_rec.status, "A");
		soic_rec.pid  = progPid;
		soic_rec.line = 0;
		cc = find_rec (soic2, &soic_rec, GTEQ, "u");
	}
	abc_unlock (soic2);
}

/*------------------------------------------
| Recalculate the current available stock. |
| Ignore the soic_line relating to         |
| commitRef for the current line.          |
------------------------------------------*/
float 
ReCalcAvail (void)
{
	float	realStock;
	float	realCommitted;

	/*----------------------
	| Look up incc record. |
	----------------------*/
    incc2_rec.hhcc_hash = ccmr_rec.hhcc_hash;
    incc2_rec.hhbr_hash = SR.hhbrHash;
    cc = find_rec (incc, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	/*---------------------------------
	| Calculate actual committed.     |
	| Ignore record for current line. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (SR.hhbrHash,
									   ccmr_rec.hhcc_hash,
									   SR.commitRef);
	if (envVar.includeForwardStock)
	{
		realStock = incc2_rec.closing_stock -
					(incc2_rec.committed + realCommitted) -
					incc2_rec.backorder - 
					incc2_rec.forward;
	}
	else
	{
		realStock = incc2_rec.closing_stock -
					(incc2_rec.committed + realCommitted) -
					incc2_rec.backorder;
	}
	if (envVar.qCApplies && envVar.qCAvailable)
		realStock -= incc2_rec.qc_qty;

	/*------------------------------------------------------------
	| Add into available any stock that was on line when loaded. |
	------------------------------------------------------------*/
	if (SR.hhbrHash == SR.origHhbrHash)
		realStock += SR.origOrdQty;

	return (realStock);
}

/*-----------------------------------------------------------
| Function    : RealTimeCommitted ()                        |
| Description : Calculates the actual real-time committed   |
|               quantity of stock for a warehouse (or all   |
|               warehouses).  Data is taken from the soic   |
|               file which is updated real-time by a        |
|               number of programs including so_input etc   |
| Parameters  : hhbrHash - link to inventory master (inmr)  |
|               hhccHash - link to warehouse master (ccmr)  |
|                        - If hhccHash is 0L then calculate |
|                          for all warehouses.              |
|               ignLine  - line to ignore.                  |
| Returns     : A float containing the actual quantity      |
|               committed at this time.                     |
-----------------------------------------------------------*/
float 
RealTimeCommitted (
	long	hhbrHash,
	long	hhccHash,
	int	ignLine)
{
	float	commQty;	/* Accumulator of committed quantity */

	/*---------------------------------
	| Initialise calculated quantity. |
	---------------------------------*/
	commQty = 0.00;

	/*---------------------------------------
	| Read soic records.  If hhccHash is 0L |
	| then read for all warehouses.         |
	---------------------------------------*/
	strcpy (soic_rec.status, "A");
	soic_rec.hhbr_hash = hhbrHash;
	soic_rec.hhcc_hash = hhccHash;
	cc = find_rec (soic, &soic_rec, GTEQ, "r");
	while (!cc &&
		   soic_rec.status [0] == 'A' &&
		   soic_rec.hhbr_hash == hhbrHash &&
		(hhccHash == 0L || 
			(hhccHash != 0L && soic_rec.hhcc_hash == hhccHash)))
	{
		if (soic_rec.pid == progPid && soic_rec.line == ignLine)
		{
			cc = find_rec (soic, &soic_rec, NEXT, "r");
			continue;
		}

		commQty += soic_rec.qty;

		cc = find_rec (soic, &soic_rec, NEXT, "r");
	}

	return (commQty);
}

int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn != cur_screen || scn == 3 || scn == 4 || scn == 5)
	{
		putchar (BELL);
		return (FALSE);
	}

	if (scn == 1)
	{
		InputSONS (TRUE, 0); 
		restart = FALSE;
		return (TRUE);
	}

	if (store [lin].hhbrHash == 0L)
	{
		putchar (BELL);
		return (FALSE);
	}
	InputSONS (FALSE, lin); 
	restart = FALSE;

	print_at (4,1, ML ("Customer : %s %s"), cumr_rec.dbt_no, 
									clip (cumr_rec.dbt_name));

	if (envVar.dbMcurr)
		print_at (4, 65,"   (%-3.3s)", cumr_rec.curr_code);

	return (PSLW);
}

int
win_function2 (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn != 2)
		return (FALSE);

	if (store [lin].hhbrHash == 0L)
		return (FALSE);
	
	if (prog_status == ENTRY)
		return (FALSE);

	/*---------------------
	| Check for contract. |
	---------------------*/
	if (store [lin].contStatus)
	{
		print_mess (ML (mlSoMess239));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	/*-------------------------------
	| Disable edit of qty BO field. |
	-------------------------------*/
	negoScn [1].fldEdit = 0;

	/*-------------------------------------------
	| Initialise values for negotiation window. |
	-------------------------------------------*/
	negoRec.qOrd			=	store [lin].qtySup;
	negoRec.qBord			=	0.00;

	negoRec.regPc			=  	store [lin].regPc;
	negoRec.discArray [0]	=	store [lin].discA;
	negoRec.discArray [1]	=	store [lin].discB;
	negoRec.discArray [2]	=	store [lin].discC;
	negoRec.grossPrice		=	store [lin].gSalePrice;
	negoRec.salePrice		=	store [lin].salePrice;
	negoRec.margCost		= 	store [lin].margCost;
	negoRec.outer_size		= 	store [lin].outerSize;

	NegPrice (2, 7, local_rec.item_no, local_rec.item_desc, 
				   store [lin].cumulative, scn);

	if (!restart && !INV_DISPLAY)
	{
		local_rec.qty_sup 			=   negoRec.qOrd;

		store [lin].qtySup		=	negoRec.qOrd;
		store [lin].regPc		= 	negoRec.regPc;
		store [lin].discA		= 	negoRec.discArray [0];
		store [lin].discB		= 	negoRec.discArray [1];
		store [lin].discC		= 	negoRec.discArray [2];
		store [lin].discPc		=	CalcOneDisc (store [lin].cumulative,
													 negoRec.discArray [0],
													 negoRec.discArray [1],
													 negoRec.discArray [2]);
		store [lin].gSalePrice 	= 	negoRec.grossPrice;
		store [lin].salePrice	=	negoRec.salePrice;
		store [lin].actSale		=	negoRec.salePrice;
		store [lin].margCost 	=	negoRec.margCost;

		coln_rec.disc_pc  			= 	ScreenDisc (store [lin].discPc);
		coln_rec.sale_price 		= 	n_dec (store [lin].salePrice, 5);

		if (store [lin].calcSprice != coln_rec.sale_price)
			strcpy (store [lin].priceOveride, "Y");

		if (store [lin].calcDisc != ScreenDisc (coln_rec.disc_pc))
			strcpy (store [lin].discOveride, "Y");

		PrintExtendedTotal (lin);
		putval (lin);
	}
	CalculateBoxTotals (FALSE, FALSE);
	
	restart = FALSE;

	return (TRUE);
}

/*---------------------------------
| Tidy up alternate supply screen |
---------------------------------*/
void
TidySonsScreen (void)
{
	int		i;

	for (i = 0; i < 4 ; i++)
		print_at (i+2,0, "%90.90s", ns_space);
	
	/*-----------------------------------------------------------
	| NOTE : Active Keys [Window #1], [Window #2] & [Window #3] |
	-----------------------------------------------------------*/
	print_at (3,1, ML ("Active Keys [Window #1], [Window #2], [Window #3]"));

	print_at (4,1, ML ("Customer : %s %s"),
						cumr_rec.dbt_no,
						clip (cumr_rec.dbt_name));
	if (envVar.dbMcurr)
		print_at (4,65,"   (%-3.3s)", cumr_rec.curr_code);
	
}

int
LoadDisplay (
 char	*runString)
{
	long	hhcuHash;
	char	invoiceNo [9];

	hhcuHash = atol (runString + 8);
	sprintf (invoiceNo, "%-8.8s", runString + 19);

	abc_selfield (cumr, "cumr_hhcu_hash");
	cumr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		abc_selfield (cumr, (envVar.dbFind) ? "cumr_id_no3" : "cumr_id_no");
		return (EXIT_FAILURE);
	}
	strcpy (local_rec.cust_no, cumr_rec.dbt_no);
	/*------------------------------
	| Check if invoice is on file. |
	------------------------------*/
	sprintf (cohr_rec.co_no, "%-2.2s", runString);
	sprintf (cohr_rec.br_no, "%-2.2s", runString + 3);
	sprintf (cohr_rec.type, "%-1.1s", runString + 6);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cohr_rec.inv_no,zero_pad (invoiceNo, 8));
	cc = find_rec (cohr,&cohr_rec,COMPARISON,"r");
	if (cc)
		return (EXIT_FAILURE);

	if (LoadItemScreen (cohr_rec.hhco_hash))
		return (EXIT_FAILURE);

	strcpy (local_rec.spinst [0],cohr_rec.din_1);
	strcpy (local_rec.spinst [1],cohr_rec.din_2);
	strcpy (local_rec.spinst [2],cohr_rec.din_3);
	newInvoice = 0;
	entry_exit = 1;

	SetInvoiceDefaults (newInvoice);

	return (EXIT_SUCCESS);
}

void
PrintExtendedTotal (
 int	CurLine)
{
	CalculateLineTotal (CurLine);
	local_rec.extend = store [CurLine].extendTotal;

	DSP_FLD ("extend");
}

int
ValidateItemNumber (
 int	getFields)
{
	int		itemChanged = FALSE;
	long	orig_hhbr_hash;
	long	hhcc_hash;
	float	realCommitted;
	char	*sptr;


	hhcc_hash = ccmr_rec.hhcc_hash;
	skip_entry = 0;

	abc_selfield (inmr,"inmr_id_no");

	if (prog_status == ENTRY)
		sprintf (local_rec.serial_no,"%25.25s"," ");

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.item_no);

	SR.bonusItem [0] = (CheckBonusLine (inmr_rec.item_no)) ? 'Y' : 'N';
	coln_rec.hide_flag [0] = (CheckHiddenLine (inmr_rec.item_no)) ? 'Y' : 'N';
		
	cc	=	FindInmr 
			(
				comm_rec.co_no, 
				inmr_rec.item_no,
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		cc =	check_indent
				(
					comm_rec.co_no, 
					comm_rec.est_no,
					hhcc_hash,
					inmr_rec.item_no
				);
		if (cc)
		{
			/*-----------------
			| Item not found. |
			-----------------*/
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			cc	=	FindInmr 
					(
						comm_rec.co_no, 
						inmr_rec.item_no,
						cumr_rec.hhcu_hash, 
						cumr_rec.item_codes
					);
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				/*-----------------
				| Item not found. |
				-----------------*/
				errmess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			/*----------------------
			| Customer No : %s (%s)|
			----------------------*/
			print_at (4,0, ML (mlStdMess012),
								cumr_rec.dbt_no,
								clip (cumr_rec.dbt_name));
			if (envVar.dbMcurr)
				print_at (4,65,"   (%-3.3s)", cumr_rec.curr_code);
			BusyFunction (BUSY_OFF);
		}
	}

	/*-------------------------------
	| First character is a '\'		|
	| \\	- start/end of kit		|
	| \D	- delete current line	|
	| \I	- insert before current	|
	-------------------------------*/
	SR.kitFlag = K_NONE;

	if (local_rec.item_no [0] == 92)	/*	If Backslash.   */
	{
		if (!F_HIDE (label ("ser_no")))
			FLD ("ser_no") = NA;

		if (CheckIncc ())
			return (EXIT_SUCCESS);

		if (local_rec.item_no [1] == 92)
		{
			memset ((char *) &SR, '\0', sizeof (struct storeRec));

			SR.hhbrHash	=	kitHash = inmr_rec.hhbr_hash;
			SR.hhumHash	=	inmr_rec.std_uom;
			SR.hhsiHash	=	alt_hash
								(
									inmr_rec.hhbr_hash,
									inmr_rec.hhbr_hash
								);
			SR.hhwhHash	=	-1L;
			SR.gstPc		= (float) ((noTaxCharged) ? 0.00 : inmr_rec.gst_pc);
			sprintf (SR.serialItem,"%-25.25s"," ");
			strcpy	(SR.category,"           ");
			SR.bonusItem [0]	= 'N';
			SR.itemClass [0]	= inmr_rec.inmr_class [0];
			SR.kitFlag	= K_START;
			SR.extendTotal	= 0.00;
			strcpy	(SR.packSize,		"     ");
			SR.itemWeight		=	0.00;
			SR.conPrice	=	FALSE;
			SR.indentItem		=	FALSE;
			SR.convFct		=	1.0;					/*KIT w/ UOM*/
			sprintf (SR.nsDesc [0],  "%40.40s", " ");

			local_rec.qty_ord		= 0.00;
			local_rec.qty_sup		= 0.00;
			coln_rec.sale_price		= 0.00;
			coln_rec.gsale_price	= 0.00;
			coln_rec.disc_pc		= 0.00;
			coln_rec.tax_pc			= 0.00;
			coln_rec.due_date		= 0L;
			sprintf (coln_rec.serial_no,"%-25.25s"," ");

			putval (line_cnt);

			CheckKit (PRINT_LINES);

			/*-------------------------------------------
			| If this is a KIT_END, then we now need to |
			| check whether this kit has an NS item.    |
			-------------------------------------------*/
			if (KIT_END && !KitHasNS ())
			{
				print_mess (ML ("Please add an NS item to this kit before closing it."));
				sleep (sleepTime);
				return (ErrorReturn);
			}

			if (KIT_START)
				skip_entry = goto_field (label ("item_no"), label ("extend"));
			else
				skip_entry = goto_field (label ("item_no"), label ("qty_sup"));
			return (EXIT_SUCCESS);
		}

		if (local_rec.item_no [1] == 'D')
			return (DeleteInvoiceLine ());

		if (local_rec.item_no [1] == 'I')
			return (InsertInvoiceLine ());
	}

	orig_hhbr_hash = inmr_rec.hhbr_hash;

	SuperSynonymError ();

	sprintf (local_rec.item_no,"%-16.16s",inmr_rec.item_no);

	if (prog_status   != ENTRY &&
		SR.hhbrHash != inmr_rec.hhbr_hash &&
		SR.hhbrHash != 0)
	{
		if (inmr_rec.inmr_class [0] == 'K')
		{
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		itemChanged = TRUE;
		local_rec.qty_ord = 0.00;
		if (strcmp (SR.serialItem, ser_space) && SR.serialFlag [0] == 'Y')
		{
			cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialItem, "C", "F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
			/*-----------------------------------------------
			| Because we have freed the insf record we must |
			| blank the serial number field on the soln in  |
			| case of a restart.                            |
			-----------------------------------------------*/
			if (SR.hhclHash > 0)
			{
				abc_selfield (coln,"coln_hhcl_hash");
				coln_rec.hhcl_hash	=	SR.hhclHash;
				cc = find_rec (coln,&coln_rec,EQUAL,"u");
				if (!cc)
				{
					strcpy (coln_rec.serial_no, ser_space);
					cc = abc_update (coln, &coln_rec);
					if (cc)
						file_err (cc, coln, "DBUPDATE");
				}
				abc_selfield (coln,"coln_id_no");
			}
		}
		strcpy (local_rec.serial_no, ser_space);
		strcpy (SR.serialItem, local_rec.serial_no);
		strcpy (SR.origSerial, local_rec.serial_no);
		DSP_FLD ("ser_no");

	}

	strcpy (SR.serialFlag,inmr_rec.serial_item);
	if (SR.serialFlag [0] == 'Y' && F_HIDE (label ("ser_no")))
	{
		print_mess (ML (mlSoMess166));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	SR.hhbrHash 	= 	inmr_rec.hhbr_hash;
	SR.hhsiHash 	= 	alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	SR.itemWeight 		= 	inmr_rec.weight;
	SR.outerSize 		= 	inmr_rec.outer_size;
	SR.dfltDisc 	= 	inmr_rec.disc_pc;
	SR.itemClass [0] 	= 	inmr_rec.inmr_class [0];
	SR.decimalPt 		= 	inmr_rec.dec_pt;
	SR.outerSize 		= 	inmr_rec.outer_size;
	strcpy (SR.category			, inmr_rec.category);
	strcpy (SR.sellGroup			, inmr_rec.sellgrp);
	strcpy (	SR.lotControl		, inmr_rec.lot_ctrl);
	strcpy (	local_rec.lot_ctrl	, inmr_rec.lot_ctrl);
	strcpy (SR.packSize		, inmr_rec.pack_size);
	strcpy (local_rec.UOM, inmr_rec.sale_unit);

    /*---------------------
    | Find for UOM GROUP. |
    ----------------------*/
    strcpy (inum_rec.uom, inmr_rec.sale_unit);
    cc = find_rec (inum, &inum_rec, EQUAL, "r");
    if (cc)
        file_err (cc, inum, "DBFIND");

    SR.hhumHash   	= inum_rec.hhum_hash;
    SR.convFct     	= inum_rec.cnv_fct;
    SR.stdConvFct     = inum_rec.cnv_fct;

	strcpy (SR.UOM, 		inum_rec.uom);
	strcpy (SR.uomGroup, 	inum_rec.uom_group);

	/*-------------------------
	| Check for Indent items. |
	-------------------------*/
	if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
		SR.indentItem = FALSE;
	else
		SR.indentItem = TRUE;

	if (CheckIncc ())
		return (EXIT_SUCCESS);

	if (inmr_rec.hhbr_hash != orig_hhbr_hash) 
	{
		BusyFunction (BUSY_OFF);
		sprintf (err_str,ML (mlSoMess234),clip (local_rec.sup_part),clip (local_rec.item_no),BELL);
		print_mess (err_str);
		sleep (sleepTime);
	}
	SR.margCost = FindIneiCosts ("A", comm_rec.est_no, SR.hhsiHash);
	if (SR.margCost <= 0.00)
		SR.margCost = FindIneiCosts ("L", comm_rec.est_no, SR.hhsiHash);

	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,SR.category);
	cc = find_rec (excf,&excf_rec,COMPARISON,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess004));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	SR.minMarg = (float) (twodec (excf_rec.min_marg));

	sprintf (local_rec.item_desc, "%-30.30s", inmr_rec.description);

	if (!BONUS)
	{
		coln_rec.tax_pc		= 	inmr_rec.tax_pc;
		coln_rec.gst_pc		= 	inmr_rec.gst_pc;
		SR.taxPc 			= 	inmr_rec.tax_pc;
		SR.gstPc 			= 	inmr_rec.gst_pc;
		SR.taxAmount 		=	inmr_rec.tax_amount;
	}
	else
	{
		sprintf (bonusFlag,"%-2.2s", envVar.soSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no,"%-s%-.*s",sptr,16 - (int) strlen (sptr),bonusFlag);
		coln_rec.tax_pc  	= 0.00;
		coln_rec.gst_pc  	= 0.00;
		SR.taxAmount 		= 0.00;
	}
	DSP_FLD ("item_no");
	DSP_FLD ("descr");

	SR.hhwhHash = incc_rec.hhwh_hash;
	SR.hhccHash = incc_rec.hhcc_hash;

	/*------------------------
	| Item is a serial item. |
	------------------------*/
	if (SR.serialFlag [0] == 'Y')
	{
		if (!F_HIDE (label ("ser_no")))
			FLD ("ser_no") = YES;
	}
	else
	{
		if (!F_HIDE (label ("ser_no")))
			FLD ("ser_no") = NA;
	}

	if (itemChanged)
	{
		local_rec.qty_ord = 0.00;
		local_rec.qty_sup = 0.00;
		SR.qtySup = local_rec.qty_sup;
		DSP_FLD ("qty_ord");
		DSP_FLD ("qty_sup");
		PriceProcess ();
		DiscProcess ();
		CalculateBoxTotals (FALSE, FALSE);
	}

	realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 
									   ccmr_rec.hhcc_hash, 
									   0);
	if (envVar.includeForwardStock)
	{
		SR.qtyAvail = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder - 
						incc_rec.forward;
	}
	else
	{
		SR.qtyAvail = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder;
	}
	if (envVar.qCApplies && envVar.qCAvailable)
		SR.qtyAvail -= incc_rec.qc_qty;

	SR.itemWeight = inmr_rec.weight;
	if (envVar.advertLevy)
	{
		ItemLevy
		(
			inmr_rec.hhbr_hash,
			comm_rec.est_no,
			cumr_rec.curr_code,
			comm_rec.dbt_date
		);
		SR.advertLevyPc  = inal_rec.percent;
		SR.advertLevyAmt = DPP (inal_rec.value * 100);
	}
	else
	{
		SR.advertLevyPc  = 0.00;
		SR.advertLevyAmt = 0.00;
	}

	/*---------------
	| Process Soic. |
	---------------*/
	ProcSoic (FALSE, line_cnt);

	if (itemChanged && getFields)
	{
		if (SR.serialFlag [0] == 'Y')
		{
			local_rec.qty_sup = 1.00;
			DSP_FLD ("qty_sup");
			cc = spec_valid (label ("qty_sup"));
			while (cc && !restart && !skip_entry)
			{
				get_entry (label ("qty_sup"));
				cc = spec_valid (label ("qty_sup"));
			}
			if (skip_entry)
			{
				SR.hhbrHash = -1;
				return (EXIT_FAILURE);
			}
			SR.qtySup = local_rec.qty_sup;
			do
			{
				get_entry (label ("ser_no"));
				cc = spec_valid (label ("ser_no"));
			} while (cc && !restart);
		}
		else
		{
			do
			{
				get_entry (label ("qty_sup"));
				cc = spec_valid (label ("qty_sup"));
			} while (cc && !restart && !skip_entry);
			DSP_FLD ("qty_sup");
			if (skip_entry)
			{
				SR.hhbrHash = -1;
				return (EXIT_FAILURE);
			}
		}

		/*-----------------------
		| Update soic quantity. |
		-----------------------*/
		UpdateSoicQty ();
	}

	if (NON_STOCK (line_cnt))
		skip_entry = goto_field (label ("item_no"),label ("extend"));
	else
		skip_entry = 2;

	sptr = clip (inmr_rec.description);
	
	if (strlen (sptr) == 0)
		skip_entry = 1;

	if (!F_HIDE (label ("hide")))
		DSP_FLD ("hide");

	return (EXIT_SUCCESS);
}

/*===================================
| Determine whether the current kit |
| contains an NS item.              |
===================================*/
int
KitHasNS ()
{
    int     tmpLineCnt = line_cnt;
	int     i;

	/*-------------------------------------
	| Start at the current line and work  |
	| backwards until we get to the start |
	| of the kit                          |
	-------------------------------------*/
	for (i = line_cnt - 1; i >= 0; i--)
	{
		if (store [i].kitFlag == K_START)
			break;

   		getval (i);
		if (store [i].itemClass [0] == 'Z' ||
			!strcmp (local_rec.item_no, "NS              "))
		{
			getval (tmpLineCnt);
			return (TRUE);
		}
	}

	getval (tmpLineCnt);

	return (FALSE);
}

/*===============================================================
| Reset Kit Flags to handle Insertion / Deletion of kits	|
===============================================================*/
void
ResetKit 
 (
	int		this_page,
	int		print_it
)
{
	static	int	k_status;
	static	int	in_kit;

	/*---------------------------
	| Reset Static Variables	|
	---------------------------*/
	if (!line_cnt)
	{
		k_status = K_END;
		in_kit = 0;
	}

	/*-----------------------
	| Start Or End of Kit	|
	-----------------------*/
	if (SR.kitFlag != K_NONE)
	{
		SR.outerSize = 1.00;
		/*---------------
		| Start Of Kit	|
		---------------*/
		if (k_status == K_END)
		{
			sprintf (coln_rec.item_desc,"%-40.40s","***: START OF KIT :****");
			k_status 				= K_START;
			in_kit 					= TRUE;
			coln_rec.sale_price 	= 0.00;
			coln_rec.gsale_price 	= 0.00;
			coln_rec.disc_pc 		= 0.00;
			coln_rec.tax_pc 		= 0.00;
		}
		else
		{
			sprintf (coln_rec.item_desc,"%-40.40s", "***: END OF KIT :****");
			k_status 	= K_END;
			in_kit 		= FALSE;
		}

		/*-------------------------------------
		| Copy description to local_rec so it |
		| can be displayed                    |
		-------------------------------------*/
		sprintf (local_rec.item_desc, "%-30.30s", coln_rec.item_desc);

		SR.kitFlag = k_status;
	}
	else
		CalculateLineTotal (line_cnt);

	if (print_it && this_page == line_cnt / TABLINES)
		line_display ();
}

int
CheckIncc (void)
{
	int		i;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					  inmr_rec.hhsi_hash);

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc) 
	{
		BusyFunction (BUSY_OFF);
		i = prmptmsg (ML (mlStdMess033),"YyNn",1,2);
		if (i == 'n' || i == 'N') 
		{
			skip_entry = -1 ;
			return (EXIT_FAILURE); 
		}
		else 
		{
			cc = AddIncc (incc_rec.hhcc_hash, incc_rec.hhbr_hash);
			if (cc)
				file_err (cc, incc, "DBADD");
			SR.qtyAvail = 0.00;
		}
	}
	return (EXIT_SUCCESS);
}

/*=======================================================
| For every line in 2nd tabular screen                  |
| go thru and set kit_flag for start or end of kit      |
| also set part descriptions for start & end of kit     |
| if print_it is true print the lines on the current    |
| screen.                                               |
=======================================================*/
void
CheckKit
 (
    int     print_it
)
{
	int i;
	int this_page = line_cnt / TABLINES;
	int high_val = (!ins_flag && IS_ENTRY) ? line_cnt : lcount [2] - 1;

	if (IS_ENTRY)
		scn_page = line_cnt / TABLINES;

	for (i = line_cnt,line_cnt = 0;line_cnt <= high_val;line_cnt++)
	{
		getval (line_cnt);
		CalculateBoxTotals (FALSE, FALSE);
		ResetKit (this_page,print_it);
		putval (line_cnt);
	}
	line_cnt = i;
	getval (line_cnt);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [SCN_ITEMS] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	cc = UpdateInsf (SR.hhwhHash,0L, SR.serialItem,"C","F");
	if (cc && cc < 1000)
		file_err (cc, insf, "DBUPDATE");

	/*-------------------------------
	| Process soic record for line. |
	-------------------------------*/
	ProcSoic (TRUE, line_cnt);

	lcount [SCN_ITEMS]--;

	for (i = line_cnt,line_cnt = 0;line_cnt < lcount [SCN_ITEMS];line_cnt++)
	{
		if (line_cnt >= i)
		{
			memcpy ((char *) &SR, 
					(char *) &store [line_cnt + 1],
					   sizeof (struct storeRec));

			/*-----------------------
			| Move lot information. |
			-----------------------*/
			LotMove (line_cnt, line_cnt + 1);

			getval (line_cnt + 1);
		}
		else
			getval (line_cnt);
		
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();

	}

	sprintf (local_rec.item_no,"%-16.16s"," ");
	sprintf (local_rec.item_desc,"%-30.30s"," ");
	strcpy (coln_rec.hide_flag," ");
	local_rec.qty_ord = 0.00;
	local_rec.qty_sup = 0.00;
	coln_rec.sale_price = 0.00;
	coln_rec.disc_pc = 0.00;
	coln_rec.tax_pc = 0.00;
	coln_rec.due_date = 0L;
	sprintf (local_rec.serial_no,"%-25.25s"," ");
	putval (line_cnt);

	memset ((char *) &SR, '\0', sizeof (struct storeRec));

	/*----------------------------
	| Clear out lot information. |
	----------------------------*/
	LotClear (line_cnt);

	strcpy (SR.priceOveride, "N");
	strcpy (SR.discOveride, "N");
	strcpy (SR.serialItem, ser_space);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	BusyFunction (BUSY_OFF);

	line_cnt = i;
	getval (line_cnt);
	CalculateBoxTotals (FALSE,FALSE);
	return (EXIT_SUCCESS);
}

int
InsertLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [SCN_ITEMS] >= vars [label ("item_no")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	for (i = line_cnt,line_cnt = lcount [SCN_ITEMS];line_cnt > i;line_cnt--)
	{
		memcpy ((char *) &SR, (char *) &store [line_cnt - 1],
						sizeof (struct storeRec));
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();

		/*-----------------------
		| Move lot information. |
		-----------------------*/
		LotMove (line_cnt, line_cnt - 1);
	}
	lcount [SCN_ITEMS]++;
	line_cnt = i;

	sprintf (local_rec.item_no,"%-16.16s"," ");
	sprintf (local_rec.item_desc,"%-30.30s", " ");
	local_rec.qty_ord 	= 0.00;
	local_rec.qty_sup 	= 0.00;
	coln_rec.sale_price = 0.00;
	coln_rec.disc_pc 	= 0.00;
	coln_rec.tax_pc 	= 0.00;
	coln_rec.due_date 	= 0L;
	strcpy (coln_rec.hide_flag,"N");
	sprintf (local_rec.serial_no,"%-25.25s"," ");

	memset ((char *) &SR, '\0', sizeof (struct storeRec));
	strcpy (SR.lotDone	,	" ");
	strcpy (SR.lotControl	,	" ");
	strcpy (SR.serialItem		,	ser_space);
	strcpy (SR.origSerial		,	ser_space);
	strcpy (SR.category	,   "           ");
	strcpy (SR.sellGroup		,   "      ");
	strcpy (SR.bonusItem		,   " ");
	strcpy (SR.itemClass		,   " ");
	strcpy (SR.packSize	,   "     ");
	strcpy (SR.serialFlag	,   " ");
	strcpy (SR.priceOveride		,   " ");
	strcpy (SR.discOveride		,   " ");
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	/*----------------------------
	| Clear out lot information. |
	----------------------------*/
	LotClear (line_cnt);

	BusyFunction (BUSY_OFF);

	insertLineFlag = 1;
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	insertLineFlag = 0;
	line_cnt = i;
	getval (line_cnt);
	CalculateBoxTotals (FALSE,FALSE);
	return (EXIT_SUCCESS);
}

void
SetInvoiceDefaults (
 int	newInvoice)
{
	int		i;

	strcpy (local_rec.pri_desc, cohr_rec.pri_type);
	strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type)-1));

	if (newInvoice)
	{
		init_vars (SCN_FREIGHT);	
		init_vars (SCN_PROOF);	

		strcpy (cohr_rec.tax_code,cumr_rec.tax_code);
		strcpy (cohr_rec.tax_no,cumr_rec.tax_no);
		strcpy (cohr_rec.area_code,cumr_rec.area_code);
		strcpy (cohr_rec.sale_code,local_rec.dflt_sale_no);
		strcpy (cohr_rec.pri_type,cumr_rec.price_type);

		/*--------------------------------
		| Get any special instrunctions. |
		--------------------------------*/
		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg1;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [0],"%-60.60s",
									(cc) ? " " : exsi_rec.inst_text);

		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg2;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [1],"%-60.60s",
									(cc) ? " " : exsi_rec.inst_text);

		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg3;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [2],"%-60.60s",
									(cc) ? " " : exsi_rec.inst_text);

		abc_fclose (exsi);

		/*------------------------
		| Get charge to address. |
		------------------------*/
		strcpy (cohr_rec.dl_name, cumr_rec.dbt_name);
		strcpy (cohr_rec.dl_add1, cumr_rec.dl_adr1);
		strcpy (cohr_rec.dl_add2, cumr_rec.dl_adr2);
		strcpy (cohr_rec.dl_add3, cumr_rec.dl_adr3);

		strcpy (cohr_rec.cons_no,sixteen_space);
		strcpy (cohr_rec.cus_ord_ref,twenty_spaces);
		strcpy (cohr_rec.carr_code,"    ");
		cohr_rec.no_cartons = 0;
		cohr_rec.no_kgs = 0.00;
		sprintf (cohr_rec.op_id, "%-14.14s", currentUser);
		
		cohr_rec.date_create = TodaysDate ();
		strcpy (cohr_rec.time_create, TimeHHMM ());

		strcpy (cohr_rec.fix_exch,"N");
		strcpy (cohr_rec.frei_req, cumr_rec.freight_chg);
		strcpy (cohr_rec.area_code,cumr_rec.area_code);
		strcpy (cohr_rec.sale_code,local_rec.dflt_sale_no);
		strcpy (cohr_rec.pri_type,cumr_rec.price_type);
		cohr_rec.date_required = cohr_rec.date_raised;
		strcpy (cohr_rec.sell_terms,"   ");
		sprintf (cohr_rec.pay_terms,"%-40.40s",cumr_rec.crd_prd);

		sprintf (cohr_rec.ins_det,"%-30.30s"," ");

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms,p_terms [i]._pcode,
						strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms,"%-40.40s", p_terms [i]._pterm);
				break;
			}
		}
	}
	else
	{
		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,cohr_rec.dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
			sprintf (cudp_rec.dp_name,"%40.40s"," ");
		strcpy (local_rec.ord_desc, (cohr_rec.ord_type [0] == 'D') ? "D" : "E");
		strcpy (local_rec.ord_fulldesc, (cohr_rec.ord_type [0] == 'D') 
						? "Domestic" : "Export  ");

		/*----------------------------------------------
		| If maintenance then need to re-read contract |
		----------------------------------------------*/
		if (strcmp (cohr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cnch, "DBFIND");

			sprintf (local_rec.cont_desc, "%-30.30s", cnch_rec.desc);
		}
	}
	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		if (!strncmp (cohr_rec.sell_terms,STerms [i]._scode,
					strlen (STerms [i]._scode)))
		{
			sprintf (local_rec.sell_desc,"%-30.30s", STerms [i]._sdesc);
			break;
		}
	}	
	open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exsf_rec.salesman,"%40.40s"," ");

	abc_fclose (exsf);

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,cohr_rec.area_code);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exaf_rec.area_code,"%40.40s"," ");

	if (newInvoice)
	{
		strcpy (cohr_rec.frei_req, cumr_rec.freight_chg);
		strcpy (cohr_rec.carr_code, "    ");
	}
	OpenTransportFiles ("trzm_id_no");
		
	strcpy (trzm_rec.co_no,comm_rec.co_no);
	strcpy (trzm_rec.br_no,comm_rec.est_no);
	strcpy (trzm_rec.del_zone, (newInvoice) ? 
						cumr_rec.del_zone : cohr_rec.del_zone);

	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (trzm_rec.del_zone, "      ");
		trzm_rec.trzm_hash	=	0L;
		trzm_rec.dflt_chg	=	0.0;
		trzm_rec.chg_kg		=	0.0;
	}
	strcpy (cohr_rec.del_zone,  trzm_rec.del_zone);
	strcpy (trcm_rec.carr_code, cohr_rec.carr_code);
	sprintf (trcm_rec.carr_desc,"%40.40s", " ");
	estimFreight 		= 	0.00;
	trcm_rec.trcm_hash	=	0L;

	if (strcmp (cohr_rec.carr_code, "    ") && trzm_rec.trzm_hash > 0L)
	{
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		strcpy (trcm_rec.carr_code, cohr_rec.carr_code);
		cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, trcm, "DBFIND");
		
		trcl_rec.trcm_hash = trcm_rec.trcm_hash;
		trcl_rec.trzm_hash = trzm_rec.trzm_hash;
		cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, trcl, "DBFIND");

		CalculateFreight 
		(
			trcm_rec.markup_pc, 
			trcl_rec.cost_kg,
			trzm_rec.chg_kg,
			trzm_rec.dflt_chg
		);
	}
	CloseTransportFiles ();

	strcpy (local_rec.chargeToCustomer, "      ");
	strcpy (local_rec.chargeToName, " ");
	if (cohr_rec.chg_hhcu_hash != 0L)
	{
		cumr2_rec.hhcu_hash	=	cohr_rec.chg_hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
			sprintf (local_rec.chargeToName, "%-35.35s", cumr2_rec.dbt_name);
		}
	}
	scn_set (SCN_HEADER);
	return;
}

/*=======================================================================
| Routine to read all coln records whoose hash matches the one on the   |
| cohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
int
LoadItemScreen (
 long	hhco_hash)
{
	float	realCommitted;
	float	std_cnv_fct;
	int		in_kit	=	FALSE;
	char	*sptr;

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	FLD ("item_no") = NE;
	scn_set (SCN_ITEMS);
	lcount [SCN_ITEMS] = 0;

	if (!specialDisplay)
	{
		BusyFunction (BUSY_ON);
		move (10,2);
	}

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (inum,"inum_hhum_hash");

	LoadSONS (TRUE, 0, hhco_hash);

	coln_rec.hhco_hash = hhco_hash;
	coln_rec.line_no = 0;

	cc = find_rec (coln,&coln_rec,GTEQ,"r");

	while (!cc && hhco_hash == coln_rec.hhco_hash) 
	{
	   	/*------------------
	    | Get part number. |
	    ------------------*/
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
	    cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		sprintf (local_rec.item_desc,"%-30.30s", coln_rec.item_desc);

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		std_cnv_fct = inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.UOM, "    ");

		strcpy (local_rec.UOM, 				inum_rec.uom);
		strcpy (LSR.UOM, 		inum_rec.uom);
		strcpy (LSR.uomGroup, inum_rec.uom_group);
		LSR.hhumHash		=	coln_rec.hhum_hash;

		line_cnt = lcount [SCN_ITEMS];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		LSR.convFct 		= inum_rec.cnv_fct/std_cnv_fct;
		LSR.stdConvFct 	= std_cnv_fct;
		coln_rec.sale_price	 *= LSR.convFct;
		coln_rec.gsale_price *= LSR.convFct;

		if (coln_rec.bonus_flag [0] == 'Y')
		{
			sprintf (bonusFlag,"%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",sptr,
				16 - (int) strlen (sptr),bonusFlag);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);

		if (coln_rec.hide_flag [0] == 'Y')
		{
			sprintf (hiddenFlag,"%-2.2s", envVar.soSpecial + 2);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",sptr,16- (int) strlen (sptr),hiddenFlag);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);
	
		LSR.margCost = FindIneiCosts ("A", comm_rec.est_no, SR.hhsiHash);
		if (LSR.margCost <= 0.00)
			LSR.margCost = FindIneiCosts ("L", comm_rec.est_no, SR.hhsiHash);

		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						  			      inmr_rec.hhsi_hash);

		LSR.hhccHash	=	coln_rec.incc_hash;

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		LSR.hhwhHash = incc_rec.hhwh_hash;
		LSR.hhccHash = incc_rec.hhcc_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			if (F_HIDE (label ("ser_no")))
			{
				print_mess (ML (mlSoMess166));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.serial_no,"%-25.25s",coln_rec.serial_no);
			sprintf (LSR.serialItem,"%-25.25s",coln_rec.serial_no);
			sprintf (LSR.origSerial,"%-25.25s",coln_rec.serial_no);
		}
		else
		{
			sprintf (local_rec.serial_no,"%-25.25s"," ");
			sprintf (LSR.serialItem,"%-25.25s"," ");
			sprintf (LSR.origSerial,"%-25.25s"," ");
		}


		/*----------------------------------------------
		| if the line has a contract on it then  user  |
		| not allowed to edit price or disc            |
		----------------------------------------------*/
		LSR.contStatus = coln_rec.cont_status;

		/*-------------------------
		| Check for Indent items. |
		-------------------------*/
		if (strncmp (inmr_rec.item_no,"INDENT",6) || envVar.discountIndents)
			LSR.indentItem = FALSE;
		else
			LSR.indentItem = TRUE;

		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "excf", "DBFIND");

		LSR.minMarg 		= (float) (twodec (excf_rec.min_marg));
		LSR.dfltDisc 		= inmr_rec.disc_pc;
		LSR.taxPc 			= coln_rec.tax_pc;
		LSR.gstPc 			= coln_rec.gst_pc;
		LSR.discPc 			= coln_rec.disc_pc;
		LSR.taxAmount 		= inmr_rec.tax_amount;
		LSR.itemWeight 		= inmr_rec.weight;
		LSR.outerSize 		= inmr_rec.outer_size;
		LSR.itemClass [0] 	= inmr_rec.inmr_class [0];
		if (envVar.advertLevy)
		{
			ItemLevy
			(
				inmr_rec.hhbr_hash,
				comm_rec.est_no,
				cumr_rec.curr_code,
				comm_rec.dbt_date
			);
			SR.advertLevyAmt = DPP (inal_rec.value * 100);
			LSR.advertLevyPc  = inal_rec.percent;
		}
		else
		{
			LSR.advertLevyAmt = 0.00;
			LSR.advertLevyPc  = 0.00;
		}
		LSR.itemLevy 		= coln_rec.item_levy;
		LSR.bonusItem [0] 	= coln_rec.bonus_flag [0];
		LSR.decimalPt 		= inmr_rec.dec_pt;
		LSR.actSale 		= coln_rec.sale_price;
		LSR.salePrice 		= coln_rec.sale_price;
		LSR.regPc 			= coln_rec.reg_pc;
		LSR.discA 			= coln_rec.disc_a;
		LSR.discB 			= coln_rec.disc_b;
		LSR.discC 			= coln_rec.disc_c;
		LSR.cumulative 		= coln_rec.cumulative;
		LSR.gSalePrice 		= coln_rec.gsale_price;
		LSR.hhbrHash 		= coln_rec.hhbr_hash;
		LSR.hhclHash 		= coln_rec.hhcl_hash;
		LSR.pricingCheck 	= TRUE;
		LSR.oldLine 		= TRUE;
		LSR.hhsiHash 		= alt_hash (inmr_rec.hhbr_hash,
										inmr_rec.hhsi_hash);
		strcpy (LSR.category, inmr_rec.category);
		strcpy (LSR.sellGroup,  inmr_rec.sellgrp);
		strcpy (LSR.packSize,inmr_rec.pack_size);
		strcpy (LSR.serialFlag, inmr_rec.serial_item);
		strcpy (LSR.lotControl, inmr_rec.lot_ctrl);
		strcpy (LSR.priceOveride,   "N");
		strcpy (LSR.discOveride,   "N");
			
		local_rec.qty_ord = ToLclUom (coln_rec.q_order + coln_rec.q_backorder);

		strcpy (local_rec.lot_done, "N");
		LSR.hhahHash = coln_rec.hhah_hash;
		if (coln_rec.hhah_hash != 0L)
			strcpy (local_rec.lot_done, "Y");

		local_rec.qty_sup = coln_rec.q_order / LSR.convFct;

		realCommitted	=	RealTimeCommitted 
							(
								inmr_rec.hhbr_hash,
							   	incc_rec.hhcc_hash,
							   	0
							);

		if (local_rec.item_no [0] == 92 && local_rec.item_no [1] == 92)
		{
			kitHash = inmr_rec.hhbr_hash;
			if (in_kit)
			{
				LSR.kitFlag		= K_END;
				LSR.qtySup		= coln_rec.q_order;
				LSR.gstPc		= inmr_rec.gst_pc;
				LSR.taxPc		= inmr_rec.tax_pc;
				LSR.salePrice 	= coln_rec.sale_price;
				LSR.gSalePrice 	= coln_rec.gsale_price;
				LSR.dfltPrice 	= 0.00;
				in_kit = FALSE;
			}
			else
			{
				LSR.kitFlag		= K_START;
				LSR.qtySup		= 0.00;
				LSR.gstPc			= 0.00;
				LSR.taxPc			= 0.00;
				LSR.taxAmount		= 0.00;
				LSR.salePrice		= 0.00;
				LSR.gSalePrice	= 0.00;
				LSR.dfltPrice		= 0.00;
				LSR.actSale 		= 0.00;
				in_kit = TRUE;
			
				local_rec.qty_ord	= 0.00;
				local_rec.qty_sup	= 0.00;
				coln_rec.sale_price = 0.00;
				coln_rec.gsale_price = 0.00;
				coln_rec.disc_pc	= 0.00;
				coln_rec.tax_pc		= 0.00;
				coln_rec.due_date	= 0L;
				sprintf (coln_rec.serial_no,"%-25.25s"," ");
			}
			LSR.extendTotal	= 0.00;
			LSR.qtyAvail 		= 0.00;
		}
		else
		{
			if (envVar.includeForwardStock)
			{
				LSR.qtyAvail = incc_rec.closing_stock -
							 	(incc_rec.committed + realCommitted) -
							 	incc_rec.backorder - 
							 	incc_rec.forward;
			}
			else
			{
				LSR.qtyAvail = incc_rec.closing_stock -
							 	(incc_rec.committed + realCommitted) -
							 	incc_rec.backorder;
			}
			LSR.kitFlag		=	K_NONE;
			LSR.extendTotal	=	0.00;
		}
		if (envVar.qCApplies && envVar.qCAvailable)
			LSR.qtyAvail -= incc_rec.qc_qty;
			
		LSR.qtySup = ToLclUom (coln_rec.q_order);

		/*---------------------------
		| Calculate Extended Total. |
		---------------------------*/
		CalculateLineTotal (lcount [SCN_ITEMS]);

		local_rec.extend 	= LSR.extendTotal;
		if (KIT_END)
			local_rec.extend *=  local_rec.qty_sup;

		LSR.commitRef 	= -1;
		LSR.origHhbrHash 	= coln_rec.hhbr_hash;
		LSR.origOrdQty = (long) (ToLclUom (coln_rec.q_order + coln_rec.q_backorder));

		coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);

		LoadSONS (FALSE, lcount [SCN_ITEMS], coln_rec.hhcl_hash);
		
		if (FLD ("LL") != ND)
		{
			cc = Load_LL_Lines
			(
				lcount [SCN_ITEMS],
				LL_LOAD_INV,
				coln_rec.hhcl_hash,
				LSR.hhccHash,
				LSR.UOM,
				LSR.convFct,
				TRUE
			);
			if (cc)
			{
				cc = DisplayLL
					(										
						line_cnt,							
						tab_row + 3 + (line_cnt % TABLINES),
						tab_col + 22,						
						4,									
						LSR.hhwhHash, 						
						LSR.hhumHash,						
						LSR.hhccHash,						
						LSR.UOM,							
						LSR.qtySup,						
						LSR.convFct,						
						TodaysDate (),
						TRUE,
						FALSE,
						LSR.lotControl						
					);
			}
			strcpy (local_rec.LL, "Y");
		}
	   	putval (lcount [SCN_ITEMS]++);
	   	if (lcount [SCN_ITEMS] > MAXLINES) 
			break;

		cc = find_rec (coln,&coln_rec,NEXT,"r");
	}

	if (INV_DISPLAY)
		vars [scn_start].row = lcount [SCN_ITEMS];
	
	prog_status = !ENTRY;

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		noTaxCharged = 1;
	else
		noTaxCharged = 0;

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (inum, "inum_uom");

	scn_set (SCN_ITEMS);
	return (EXIT_SUCCESS);
}

/*===================
| Update all files. |
===================*/
void
Update (void)
{
	char	tmp_prefix 	 [3]; 
	char	tmp_inv_no [9];
	char	tmp_mask [12];
	int		len = 8;
	int		new_coln;
	int		manual_inv = TRUE;
	long	inv_no;
	int		i;

	int		startKit	=	-1;
	int		insideKit	=	FALSE;

	clear ();
	fflush (stdout);

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		noTaxCharged = 1;
	else
		noTaxCharged = 0;

	strcpy (cohr_rec.din_1, local_rec.spinst [0]);
	strcpy (cohr_rec.din_2, local_rec.spinst [1]);
	strcpy (cohr_rec.din_3, local_rec.spinst [2]);
	strcpy (cohr_rec.carr_code, trcm_rec.carr_code);
	strcpy (cohr_rec.del_zone, trzm_rec.del_zone);

	invTotalAmt 		= 0.00;
	cohr_rec.item_levy  = 0.00;
	cohr_rec.gross  	= 0.00;
	cohr_rec.disc   	= 0.00;
	cohr_rec.tax    	= 0.00;
	cohr_rec.gst    	= 0.00;

	if (newInvoice && lcount [2] == 0)
	{
		for (i = 0; i < 6; i++)
		{
			rv_pr (ML (mlSoMess103), 0,0,i % 2);
			sleep (SLEEP_TIME - 1);
		}
		return;
	}
	if (local_rec.inp_total != 0.00)
		CalculateInputTotal ();

	if (newInvoice)
	{
		if (!strcmp (cohr_rec.inv_no,"00000000") || 
		     !strcmp (cohr_rec.inv_no,"        ")) 
		{

			/*------------------------------------------------------
			| Is invoice number to come from department of branch. |
			------------------------------------------------------*/
			if (envVar.invoiceNumbers == BY_DEPART)
			{
				strcpy (cudp_rec.co_no, comm_rec.co_no);
				strcpy (cudp_rec.br_no, comm_rec.est_no);
				strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
				cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
				if (cc)
				file_err (cc, "cudp", "DBFIND");

				inv_no	=	cudp_rec.nx_man_no;
				inv_no++;
			}
			else
			{
				strcpy (esmr_rec.co_no, comm_rec.co_no);
				strcpy (esmr_rec.est_no, comm_rec.est_no);
				cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
				if (cc)
					file_err (cc, "esmr", "DBFIND");

				inv_no	=	esmr_rec.nx_man_no;
				inv_no++;
			}

			if (envVar.invoiceNumbers == BY_BRANCH)
				strcpy (tmp_prefix, esmr_rec.man_pref);
			else
				strcpy (tmp_prefix, cudp_rec.man_pref);

			clip (tmp_prefix);
			len = strlen (tmp_prefix);

			sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
			sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no);

			open_rec (cohr2,cohr_list,COHR_NO_FIELDS,"cohr_id_no2");

			/*-------------------------------------------
			| Check if Invoice / Credit Note No Already	|
			| Allocated. If it has been then skip		|
			-------------------------------------------*/
			while (CheckCohr (tmp_inv_no) == 0)
				sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no++);

			abc_fclose (cohr2);

			if (envVar.invoiceNumbers == BY_DEPART)
			{
				cudp_rec.nx_man_no	=	inv_no;

				cc = abc_update (cudp, &cudp_rec);
				if (cc)
					file_err (cc, cudp, "DBUPDATE");
			}
			else
			{
				esmr_rec.nx_man_no	=	inv_no;

				cc = abc_update (esmr, &esmr_rec);
				if (cc)
					file_err (cc, esmr, "DBUPDATE");
			}

			sprintf (cohr_rec.inv_no, "%-8.8s", tmp_inv_no);

			local_rec.dflt_inv_no = 0L;
			strcpy (manual_pref, "  ");		
		}
		else
		{
			sprintf (manual_pref, "%-2.2s", cohr_rec.inv_no);
			inv_no = atol (cohr_rec.inv_no+2);
			local_rec.dflt_inv_no = inv_no + 1L;
			if (envVar.manualPrint [0] == 'N')
				manual_inv = FALSE;
		}

		print_at (0,0,ML (mlSoMess093),PROMPT);
		sprintf (err_str,ML (mlSoMess094),PROMPT, cohr_rec.inv_no);
		PauseForKey (2,1,err_str, 0);

		strcpy (cohr_rec.co_no,comm_rec.co_no);
		strcpy (cohr_rec.br_no,comm_rec.est_no);
		strcpy (cohr_rec.type,invoiceTypeFlag);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cohr_rec.exch_rate = pocrRec.ex1_factor;

		if (CASH)
			strcpy (cohr_rec.inv_print,"Y");
		else
			strcpy (cohr_rec.inv_print, (manual_inv) ? "N" : "Y");

		strcpy (cohr_rec.stat_flag,createStatusFlag);
		cc = abc_add (cohr,&cohr_rec);
		if (cc) 
			file_err (cc, "cohr", "DBADD");

		cc = find_rec (cohr,&cohr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, "cohr", "DBFIND");


		print_at (4,0,ML (mlSoMess095),PROMPT);
	}
	else
		print_at (5,0,ML (mlSoMess096),PROMPT);

	fflush (stdout);
	abc_selfield (inmr,"inmr_hhbr_hash");

	scn_set (SCN_ITEMS);
	for (line_cnt = 0;line_cnt < lcount [SCN_ITEMS];line_cnt++) 
	{
		if (KIT_START)
		{
			/*-----------------------------------
			| Zero totals for kit components	|
			-----------------------------------*/
			startKit 	=	line_cnt;
			insideKit	=	TRUE;
			continue;
		}
		if (KIT_END)
			insideKit	=	FALSE;

		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		new_coln = find_rec (coln,&coln_rec,COMPARISON,"u");

		getval (line_cnt);
		CalculateLineTotal (line_cnt);

		if (insideKit == FALSE)
		{
			cohr_rec.item_levy 	+= lineLevy;
			cohr_rec.gross 		+= lineGross;
			cohr_rec.disc 		+= lineDisc;
			cohr_rec.tax 		+= lineTax;
			cohr_rec.gst 		+= lineGst;
		}
		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		new_coln = find_rec (coln,&coln_rec,COMPARISON,"u");

		inmr_rec.hhbr_hash	=	SR.hhbrHash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;

		if (inmr_rec.hhbr_hash == coln_rec.hhbr_hash)
			if (incc_rec.hhcc_hash != coln_rec.incc_hash)
				incc_rec.hhcc_hash = coln_rec.incc_hash;

		coln_rec.hhco_hash 		= 	cohr_rec.hhco_hash;
		coln_rec.line_no 		= 	line_cnt;
		coln_rec.hhbr_hash 		= 	SR.hhbrHash;
		coln_rec.hhah_hash 		= 	SR.hhahHash;
		coln_rec.hhum_hash 		= 	SR.hhumHash;
		coln_rec.incc_hash 		= 	incc_rec.hhcc_hash;
		coln_rec.q_order  		= 	ToStdUom (local_rec.qty_sup);
		coln_rec.q_backorder  	= 	ToStdUom (local_rec.qty_ord - local_rec.qty_sup);

		coln_rec.gst_pc 		= 	(float) ((noTaxCharged) ? 0.00 : SR.gstPc);
		coln_rec.tax_pc 		= 	(float) ((noTaxCharged) ? 0.00 : SR.taxPc);
		strcpy (coln_rec.bonus_flag, (BONUS) ? "Y" : "N");
		coln_rec.reg_pc			= 	SR.regPc;
		coln_rec.disc_a			= 	SR.discA;
		coln_rec.disc_b			= 	SR.discB;
		coln_rec.disc_c			= 	SR.discC;
		coln_rec.cumulative		= 	SR.cumulative;

		if (SR.convFct == 0.00)
			SR.convFct = 1.00;

		coln_rec.gsale_price	= 	SR.gSalePrice / SR.convFct;
		coln_rec.sale_price		= 	SR.salePrice / SR.convFct;
		coln_rec.cont_status	= 	SR.contStatus;

		coln_rec.gross 			= 	lineGross;
		coln_rec.item_levy 		= 	lineLevy;
		coln_rec.amt_disc 		= 	lineDisc;
		coln_rec.amt_tax 		= 	lineTax;
		coln_rec.amt_gst 		= 	lineGst;
		coln_rec.due_date 		= 	cohr_rec.date_raised;
		strcpy (coln_rec.stat_flag,createStatusFlag);
		strcpy (coln_rec.status, invoiceTypeFlag);
		strcpy (coln_rec.serial_no, SR.serialItem);

		if (!new_coln)
		{
			/*------------------------
			| Update existing order. |
			------------------------*/
			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			cc = abc_update (coln,&coln_rec);
			if (cc) 
				file_err (cc, coln, "DBUPDATE");

			if (envVar.salesOrderSales)
				AddSobg (0,"AI", coln_rec.hhcl_hash);

		}
		else 
		{
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = line_cnt;

			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			if (strncmp (inmr_rec.description, local_rec.item_desc, 30))
				sprintf (coln_rec.item_desc, "%-40.40s", local_rec.item_desc);
			else
				strcpy (coln_rec.item_desc, inmr_rec.description);
			cc = abc_add (coln,&coln_rec);
			if (cc) 
				file_err (cc, coln, "DBADD");

			abc_unlock (coln);

			coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
			coln_rec.line_no 	= line_cnt;
			cc = find_rec (coln,&coln_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, coln, "DBFIND");

			if (envVar.salesOrderSales)
				AddSobg (0,"AI",coln_rec.hhcl_hash);
		}
		UpdateSONS (FALSE, line_cnt, coln_rec.hhcl_hash);

		if (!KIT_START && !KIT_END)
		{
			if (SK_BATCH_CONT || MULT_LOC)
			{
				AllocLotLocation 
			 	(
					line_cnt,
					TRUE,
					LL_LOAD_INV,
					coln_rec.hhcl_hash
				);
			}
		}
		if (KIT_END)
		{
			coln2_rec.hhco_hash 	=	cohr_rec.hhco_hash;
			coln2_rec.line_no 	=	startKit;

			new_coln = find_rec (coln,&coln2_rec,COMPARISON,"u");

			coln_rec.hhco_hash 		=	cohr_rec.hhco_hash;
			coln_rec.line_no 		=	startKit;
			strcpy (coln_rec.item_desc,"***: START OF KIT :***");
			coln_rec.gsale_price 	=	-coln_rec.gsale_price;
			coln_rec.sale_price 	=	-coln_rec.sale_price;
			coln_rec.gross 			=	-coln_rec.gross;
			strcpy (coln_rec.bonus_flag, (BONUS) ? "Y" : "N");
			strcpy (coln_rec.hide_flag,	"N");
			if (!new_coln)
			{
				cc = abc_update (coln,&coln_rec);
				if (cc)
					file_err (cc, coln, "DBUPDATE");
			}
			else 
			{
				cc = abc_add (coln,&coln_rec);
				if (cc)
					file_err (cc, coln, "DBADD");
			}
		}
	}

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = line_cnt;
	cc = find_rec (coln,&coln_rec,GTEQ,"u");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		FreeInsf (coln_rec.line_no,SR.serialItem);
		if (envVar.salesOrderSales)
			AddSobg (0,"AI", coln_rec.hhcl_hash);

		cc = abc_delete (coln);
		if (cc) 
			file_err (cc, coln, "DBDELETE");

		DeleteSONS (FALSE, coln_rec.hhcl_hash);

		cc = find_rec (coln,&coln_rec,GTEQ,"u");
	}
	abc_unlock (coln);

	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (!newInvoice) 
	{	
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (lcount [SCN_ITEMS] == 0) 
		{
			DeleteSONS (TRUE, cohr_rec.hhco_hash);

			print_at (4,0,ML (mlSoMess175),PROMPT);
			cc = abc_delete (cohr);
			if (cc)
				file_err (cc, "cohr", "DBDELETE");
		}
		else
			print_at (4,0,ML (mlSoMess090));
	}
	else
		print_at (6,0,ML (mlSoMess063));

	/*---------------------------------------
	| Calc Totals of Gst etc for cohr	|
	---------------------------------------*/
	CalculateExtendedTotal (FALSE);
	cohr_rec.date_required = cohr_rec.date_raised;
	cohr_rec.exch_rate = pocrRec.ex1_factor;

	if (lcount [SCN_ITEMS] != 0) 
	{
		UpdateSONS (TRUE, 0, cohr_rec.hhco_hash);

		cc = abc_update (cohr,&cohr_rec);
		if (cc)
			file_err (cc, "cohr", "DBUPDATE");
	}
	else
		abc_unlock (cohr);

	if (envVar.automaticFreight)
		AddCarrierDetails ();
		
	if (cohr_rec.del_req [0] == 'Y' && newInvoice)
	{
		sprintf (err_str,"tr_trsh_mnt P %010ld",cohr_rec.hhco_hash);
		sys_exec (err_str);
	}

	if (AUTO_SK_UP)
		AddSobg (0,"SU",cohr_rec.hhco_hash);

	/*----------------------------------
	| Clear soic records for this PID. |
	----------------------------------*/
	ClearSoic ();

	strcpy (local_rec.prev_inv_no,cohr_rec.inv_no);
	strcpy (local_rec.prev_dbt_no,cumr_rec.dbt_no);

	UpdateSONS (TRUE, 0, cohr_rec.hhco_hash);

	return;
}

int
CheckCohr (
 char	*inv_no)
{
	strcpy (cohr2_rec.co_no,comm_rec.co_no);
	strcpy (cohr2_rec.br_no,comm_rec.est_no);
	strcpy (cohr2_rec.type,invoiceTypeFlag);
	sprintf (cohr2_rec.inv_no,"%-8.8s",inv_no);
	return (find_rec (cohr2,&cohr2_rec,COMPARISON,"r"));
}

void
CalculateExtendedTotal (
 int	cal_line)
{
	int		i;
	double	wk_value = 0.00;
	double	value;

	if (cal_line)
	{
		scn_set (SCN_ITEMS);

		invTotalAmt = 0.00;

		cohr_rec.item_levy	= 0.00;
		cohr_rec.gross 		= 0.00;
		cohr_rec.disc 		= 0.00;
		cohr_rec.tax 		= 0.00;
		cohr_rec.gst 		= 0.00;

		for (i = 0;i < lcount [SCN_ITEMS];i++)
		{
			if (LineInKit (i))
				continue;

			getval (i);
			CalculateLineTotal (i);
			cohr_rec.item_levy	+= lineLevy;
			cohr_rec.gross 		+= lineGross;
			cohr_rec.disc 		+= lineDisc;
			cohr_rec.tax 		+= lineTax;
			cohr_rec.gst 		+= lineGst;

		}
	}

	if (noTaxCharged)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	CalculateFreight 
	(
		trcm_rec.markup_pc,
		trcl_rec.cost_kg,
		trzm_rec.chg_kg, 
		trzm_rec.dflt_chg
	);


	value = cohr_rec.freight + 
			cohr_rec.sos + 
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3;

	wk_value *= value;
	cohr_rec.gst += wk_value;
	
	cohr_rec.gst = no_dec (cohr_rec.gst);
	
	invTotalAmt 	=	cohr_rec.gross + 
						cohr_rec.item_levy + 
						cohr_rec.tax + 
						cohr_rec.freight +
						cohr_rec.sos +
						cohr_rec.gst - 
						cohr_rec.deposit -
						cohr_rec.ex_disc + 
						cohr_rec.insurance +
						cohr_rec.other_cost_1 + 
						cohr_rec.other_cost_2 +
						cohr_rec.other_cost_3;
	
	if (envVar.dbNettUsed)
		invTotalAmt -= cohr_rec.disc;
}

void
CalculateLineTotal (
 int	line_no)
{
	if (NON_STOCK (line_no))
	{
		store [line_no].qtySup		= 0.00;
		store [line_no].salePrice	= 0.00;
		store [line_no].gSalePrice 	= 0.00;
		store [line_no].outerSize	= 0.00;
		store [line_no].discPc		= 0.00;
		store [line_no].taxPc		= 0.00;
		store [line_no].taxAmount	= 0.00;
		store [line_no].gstPc		= 0.00;
	}
	/*-----------------------------------------------
	| Update coln gross tax and disc for each line. |
	-----------------------------------------------*/
	lineGross = (double) store [line_no].qtySup;
	lineGross *= out_cost (store [line_no].actSale, store [line_no].outerSize);
	lineGross = no_dec (lineGross); 

	if (noTaxCharged)
		lineTaxAmt = 0.00;
	else
	{
		lineTaxAmt = (double) store [line_no].qtySup;
		lineTaxAmt *= out_cost (store [line_no].taxAmount, store [line_no].outerSize);
		lineTaxAmt = no_dec (lineTax);
	}

	lineDisc = (double) (store [line_no].discPc);
	lineDisc = DOLLARS (lineDisc);
	lineDisc *= lineGross;
	lineDisc = no_dec (lineDisc);

	if (envVar.advertLevy)
	{
		lineLevyPc 	= (double) (store [line_no].advertLevyPc);
		lineLevyPc 	= DOLLARS (lineLevyPc);
		lineLevyPc 	*= lineGross;
		lineLevyPc 	= no_dec (lineLevyPc);

		lineLevyAmt = store [line_no].advertLevyAmt;
		lineLevyAmt *= (double) store [line_no].qtySup;
		lineLevyAmt = no_dec (lineLevyAmt);
		lineLevy	=	lineLevyPc + lineLevyAmt;
		/*
		 * Display needs to use old value.
		 */
		if (INV_DISPLAY)
			lineLevy	=	store [line_no].itemLevy;
	}

	if (noTaxCharged)
		lineTax = 0.00;
	else
	{
		lineTax = (double) (store [line_no].taxPc);
		lineTax = DOLLARS (lineTax);

		if (cohr_rec.tax_code [0] == 'D')
			lineTax *= lineTaxAmt;
		else
		{
			if (envVar.dbNettUsed)
				lineTax *= (lineGross - lineDisc) + lineLevy;
			else
				lineTax *= lineGross + lineLevy ;
		}
		lineTax = no_dec (lineTax);
	}
	
	if (noTaxCharged)
		lineGst = 0.00;
	else
	{
		lineGst = (double) (store [line_no].gstPc);
		if (envVar.dbNettUsed)
			lineGst *= ((lineGross - lineDisc) + lineTax + lineLevy);
		else
			lineGst *= (lineGross + lineTax + lineLevy);
		lineGst = DOLLARS (lineGst);
	}
	store [line_no].extendTotal = (lineGross - lineDisc + lineTax + lineLevy);
}
/*=======================
|	Free insf record	|
=======================*/
void
FreeInsf (
 int	line_no,
 char	*ser_no)
{
	if (!strcmp (ser_no,ser_space))
		return;

	/*---------------------------------------
	| serial_item and serial number input	|
	---------------------------------------*/
	if (SERIAL)
	{
		cc = UpdateInsf (store [line_no].hhwhHash,0L, ser_no,"S","F");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}

/*=======================
|	Commit insf record	|
=======================*/
void
CommitInsf (
 int	line_no,
 char	*ser_no)
{
	if (!strcmp (ser_no,ser_space))
		return;

	/*---------------------------------------
	| serial_item and serial number input	|
	---------------------------------------*/
	if (SERIAL)
	{
		cc = UpdateInsf (store [line_no].hhwhHash,0L, ser_no,"F","S");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}

/*===========================
| Search On Department      |
===========================*/
void
SrchCudp (
 char    *key_val)
{
    work_open ();
    save_rec ("#Dp", "#Department Name");

	/*--------------------------
	| Flush record buffer first |
	---------------------------*/
	memset (&cudp_rec, 0, sizeof (cudp_rec));

	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
    sprintf (cudp_rec.dp_no, "%2.2s", key_val);
    for (cc = find_rec (cudp, &cudp_rec,  GTEQ,"r");
		 !cc && !strcmp (cudp_rec.co_no, comm_rec.co_no)
		  && !strcmp (cudp_rec.dp_no, comm_rec.est_no)
		  && !strncmp (cudp_rec.dp_no, key_val, strlen (clip (key_val)));
         cc = find_rec (cudp, &cudp_rec,  NEXT, "r"))
    {
        cc = save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
        if (cc)
            break;
    }

    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

    strcpy (cudp_rec.co_no, comm_rec.co_no);
    strcpy (cudp_rec.br_no, comm_rec.est_no);
    sprintf (cudp_rec.dp_no, "%2.2s", temp_str);
    cc = find_rec (cudp, &cudp_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, cudp, "DBFIND");
}

void
SrchPay (void)
{
	int		i = 0;
	work_open ();
	save_rec ("#Cde","#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

void
SrchSell (void)
{
	int		i = 0;
	work_open ();
	save_rec ("#Cde","#Selling Terms ");

	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		cc = save_rec (STerms [i]._scode,STerms [i]._sdesc);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

void
SrchPrice (
 void)
{
	int		i = 0;
	work_open ();
	save_rec ("# ","#Price ");

	for (i = 0;i < envVar.numberPrices;i++)
	{
		sprintf (err_str,"%d",i + 1);
		cc = save_rec (err_str, GetPriceDesc (i));
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*======================
| Search for salesman. |
======================*/
void
SrchExsf (
 char	*key_val)
{
	work_open ();
	save_rec ("#Sm","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
				  !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

/*==================
| Search for area. |
==================*/
void
SrchExaf (
 char	*key_val)
{
	work_open ();
	save_rec ("#No","#Area Description.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
			      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf,&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

/*=============================================
| Search routine for Serial Item master file. |
=============================================*/
void
SrchInsf (
 char	*key_val,
 int	line_no)
{
	work_open ();
	save_rec ("#      Serial Item.         ","# ");
	cc = FindInsf (SR.hhwhHash,0L, "","F","r");
	while (!cc && SR.hhwhHash == insfRec.hhwh_hash &&
		    	strncmp (insfRec.serial_no,key_val,strlen (key_val)) < 0)

	cc = FindInsf (0L, 0L, "", "F", "r");
	while (!cc && SR.hhwhHash == insfRec.hhwh_hash &&
	!strncmp (insfRec.serial_no,key_val,strlen (key_val)))
	{
		if (!CheckDuplicateInsf (insfRec.serial_no,store [line_no].hhsiHash,line_no))
		{
			cc = save_rec (insfRec.serial_no,inmr_rec.item_no);
			if (cc)
				break;
		}
		cc = FindInsf (0L,0L, "","F","r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cc = FindInsf (SR.hhwhHash,0L, temp_str,"F","r");

	if (cc && cc != 1)
		file_err (cc, insf, "DBFIND");

	strcpy (coln_rec.serial_no,insfRec.serial_no);
}
void
SrchCohr (
 char	*key_val)
{
	work_open ();
	save_rec (SRCH,"#Cust Order");
	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	strcpy (cohr_rec.type,invoiceTypeFlag);
	sprintf (cohr_rec.inv_no,"%-8.8s",key_val);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cohr,&cohr_rec,GTEQ,"r");

	while (!cc && !strcmp (cohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (cohr_rec.br_no,comm_rec.est_no) && 
		      !strncmp (cohr_rec.inv_no,key_val,strlen (key_val)))
	{
		if (cumr_rec.hhcu_hash > 0L &&
		     cumr_rec.hhcu_hash != cohr_rec.hhcu_hash)
			break;
		
		if (cohr_rec.type [0] != invoiceTypeFlag [0])
		{
			cc = find_rec (cohr,&cohr_rec,NEXT,"r");
			continue;
		}
		if ((INV_DISPLAY || (cohr_rec.inv_print [0] == 'N' && 
		(cohr_rec.stat_flag [0] == createStatusFlag [0] && INV_INPUT))) && 
		(cumr_rec.hhcu_hash == cohr_rec.hhcu_hash || 
		   cumr_rec.hhcu_hash == 0L))
		{
			cc = save_rec (cohr_rec.inv_no,cohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (cohr,&cohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	strcpy (cohr_rec.type,invoiceTypeFlag);
	sprintf (cohr_rec.inv_no,"%-8.8s",temp_str);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

	cc = find_rec (cohr,&cohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "cohr", "DBFIND");
}
/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char	*key_val)
{
	char	wk_code [4];

	work_open ();
	save_rec ("#Spec Inst","#Instruction description.");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);
	exsi_rec.inst_code = 0;

	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");
	while (!cc && !strcmp (exsi_rec.co_no,comm_rec.co_no) &&
			exsi_rec.inst_code >= atoi (key_val))
	{
		sprintf (wk_code, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wk_code, err_str);
		if (cc)
			break;

		cc = find_rec (exsi,&exsi_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsi", "DBFIND");
}
/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.  Return 1 if duplicate			|
=======================================================*/
int
CheckDuplicateInsf (
 char	*serial_no,
 long	hhbr_hash,
 int	line_no)
{
	int		i;
	int		no_items = (prog_status == ENTRY) ? line_cnt : lcount [SCN_ITEMS];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------------
		| cannot duplicate item_no/serial_no	|
		| unless serial no was not input	|
		---------------------------------------*/
		if (!strcmp (store [i].serialItem,ser_space))
			continue;

		/*---------------------------------------
		| Only compare serial numbers for	|
		| the same item number			|
		---------------------------------------*/
		if (store [i].hhsiHash == hhbr_hash)
		{
			if (!strcmp (store [i].serialItem,serial_no))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

void
PriceProcess (void)
{
	int		pType;
	float	regPc;
	double	grossPrice;

	SR.pricingCheck	= FALSE;

	if (BONUS)
	{
		coln_rec.sale_price 	= 0.00;
		SR.actSale 				= 0.00;
		SR.calcSprice			= 0.00;
		SR.salePrice 			= 0.00;
		DSP_FLD ("sale_price");

		coln_rec.disc_pc  		= 0.00;
		SR.discPc 	 			= 0.00;
		SR.calcDisc 			= 0.00;
		DSP_FLD ("disc");
		return;
	}
	pType = atoi (cohr_rec.pri_type);
	grossPrice = GetCusPrice 
				(
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
		 			cohr_rec.area_code,
					cumr_rec.class_type,
					SR.sellGroup,
					cumr_rec.curr_code,
					pType,
					cumr_rec.disc_code,
					cnch_rec.exch_type,
					cumr_rec.hhcu_hash,
					ccmr_rec.hhcc_hash,
					SR.hhbrHash,
					SR.category,
					cnch_rec.hhch_hash,
					(envVar.useSystemDate) ? local_rec.longSystemDate : comm_rec.dbt_date,
					ToStdUom (local_rec.qty_sup),
					pocrRec.ex1_factor,
					FGN_CURR,
					&regPc
				);

	SR.pricingCheck	= TRUE;

	/*----------------------------------------------------------
	| Inclusion of the conversion factor for the multiple unit |
	| of measure in computing the SR.calcSprice.             |
	----------------------------------------------------------*/
	SR.calcSprice = GetCusGprice (grossPrice, regPc) * SR.convFct;

	if (SR.priceOveride [0] == 'N')
	{
		SR.gSalePrice 		= 	grossPrice * SR.convFct;
		SR.salePrice 		=	SR.calcSprice;
		SR.regPc 			= 	regPc;
		coln_rec.sale_price = 	SR.calcSprice;
		SR.actSale 			= 	SR.calcSprice;
	}
	SR.conPrice 		= (_CON_PRICE) ? TRUE : FALSE;
	SR.contStatus  	= _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess (void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (SR.contStatus == 2 || SR.conPrice || SR.indentItem)
	{
		coln_rec.disc_pc  	= 0.00;
		SR.discPc 	 		= 0.00;
		SR.calcDisc 		= 0.00;
		SR.discA			= 0.00;
		SR.discB			= 0.00;
		SR.discC			= 0.00;
		DSP_FLD ("disc");
		return;
	}

	if (SR.pricingCheck == FALSE)
		PriceProcess ();

	pType = atoi (cohr_rec.pri_type);
	cumDisc		=	GetCusDisc (	comm_rec.co_no,
								comm_rec.est_no,
								ccmr_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								SR.hhsiHash,
								SR.category,
								SR.sellGroup,
								pType,
								SR.gSalePrice,
								SR.regPc,
								ToStdUom (local_rec.qty_sup),
								discArray);


	if (SR.discOveride [0] == 'Y')
	{
		DSP_FLD ("disc");
		return;
	}
	SR.calcDisc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	if (SR.discOveride [0] == 'N')
	{
		coln_rec.disc_pc 	=	ScreenDisc (SR.calcDisc);
		SR.discPc			=	SR.calcDisc;

		SR.discA 			= 	discArray [0];
		SR.discB 			= 	discArray [1];
		SR.discC 			= 	discArray [2];
		SR.cumulative 		= 	cumDisc;

		if (SR.dfltDisc > ScreenDisc (coln_rec.disc_pc) &&
			SR.dfltDisc != 0.0)
		{
			coln_rec.disc_pc = 	ScreenDisc (SR.dfltDisc);
			SR.calcDisc	=	SR.dfltDisc;
			SR.discPc		=	SR.dfltDisc;
			SR.discA 		= 	SR.dfltDisc;
			SR.discB 		= 	0.00;
			SR.discC 		= 	0.00;
		}
	}
	DSP_FLD ("disc");
}

int
DeleteInvoiceLine
 (
	void
)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (IS_ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (ErrorReturn);
	}

	if (!lcount [SCN_ITEMS])
	{
		/*-----------------------------------------
		| Cannot Delete Line - No Lines to Delete |
		-----------------------------------------*/
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (ErrorReturn);
	}

	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	if (OLD_INSF)
	{
		if (ONLINE_INVOICE)
			cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialItem, "C", "F");
		else
			cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialItem, "C", "S");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
	else
	{
		RemoveSerialNo (SR.hhwhHash, SR.hhsiHash, SR.serialItem);
	}

	lcount [SCN_ITEMS]--;

	for (i = line_cnt,line_cnt = 0;line_cnt < lcount [SCN_ITEMS];line_cnt++)
	{
		if (line_cnt >= i)
		{
			memcpy ((char *) &SR, (char *) &store [line_cnt + 1],
						sizeof (struct storeRec));
			getval (line_cnt + 1);
			ResetKit (this_page, PRINT_LINES);
			if (this_page == line_cnt / TABLINES)
				line_display ();
		}
		else
		{
			getval (line_cnt);
			ResetKit (this_page, !PRINT_LINES);
		}
		putval (line_cnt);
	}
	Init_store (lcount [SCN_ITEMS] + 1);

	sprintf (local_rec.item_no,  "%-16.16s", " ");
	sprintf (coln_rec.item_desc, "%-40.40s", " ");
	local_rec.qty_ord 	= 0.00;
	local_rec.qty_sup 	= 0.00;
	coln_rec.sale_price = 0.00;
	coln_rec.disc_pc 	= 0.00;
	coln_rec.tax_pc 	= 0.00;
	coln_rec.due_date 	= 0L;
	sprintf (local_rec.serial_no, "%-25.25s", " ");
	sprintf (local_rec.serial_no, "%-25.25s", " ");
	putval (line_cnt);
	memset ((char *) &SR, '\0', sizeof (struct storeRec));

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	SillyBusyFunc (0);

	line_cnt = i;
	getval (line_cnt);

	RunningKit (line_cnt);
	CalculateBoxTotals (FALSE, FALSE);

	return (EXIT_SUCCESS);
}

int
InsertInvoiceLine
 (
	void
)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (IS_ENTRY)
	{
		/*------------------------------
		| Cannot Insert Lines On Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (ErrorReturn);
	}

	if (lcount [SCN_ITEMS] >= vars [label ("item_no")].row)
	{
		/*------------------------------------
		| Cannot Insert Line - Table is Full |
		------------------------------------*/
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (ErrorReturn);
	}

	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	for (i = line_cnt,line_cnt = lcount [SCN_ITEMS];line_cnt > i;line_cnt--)
	{
		memcpy ((char *) &SR, (char *) &store [line_cnt - 1],
						sizeof (struct storeRec));
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [SCN_ITEMS]++;
	line_cnt = i;

	sprintf (local_rec.item_no,  "%-16.16s", " ");
	sprintf (coln_rec.item_desc, "%-40.40s", " ");
	local_rec.qty_ord 	= 0.00;
	local_rec.qty_sup 	= 0.00;
	coln_rec.sale_price = 0.00;
	coln_rec.disc_pc 	= 0.00;
	coln_rec.tax_pc 	= 0.00;
	coln_rec.due_date 	= 0L;
	sprintf (local_rec.serial_no, "%-25.25s", " ");

	memset ((char *) &store [line_cnt], '\0', sizeof (struct storeRec));
	Init_store (line_cnt);

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	SillyBusyFunc (0);

	ins_flag 	= 1;
	init_ok 	= 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok 	= 1;
	ins_flag 	= 0;
	line_cnt 	= i;
	getval (line_cnt);
	CheckKit (!PRINT_LINES);
	return (EXIT_SUCCESS);
}
/*=======================================
| Add warehouse record for current W/H. |
=======================================*/
int
AddIncc (
	long	hhccHash,
	long	hhbrHash)
{

	memset (&incc_rec, 0, sizeof (incc_rec));

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	incc_rec.hhwh_hash = 0L;
	sprintf  
	(
		incc_rec.sort,
		"%s%11.11s%-16.16s",
		inmr_rec.inmr_class,
		inmr_rec.category,
		inmr_rec.item_no
	);

	incc_rec.first_stocked = TodaysDate ();
	strcpy (incc_rec.stocking_unit, inmr_rec.sale_unit);
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.abc_code,  "A");
	strcpy (incc_rec.abc_update,"Y");
	strcpy (incc_rec.stat_flag,"0");
	
	cc = abc_add (incc,&incc_rec);
	if (cc) 
		return (cc);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	return (find_rec (incc,&incc_rec,COMPARISON,"r"));
}

/*==============================
| Calculate Defaults for Levy. |
==============================*/
void
CalculateFreight (
 float	freightMarkup,
 double	carrierCostKg,
 double	zoneCostKg,
 double	zoneFixedAmount)
{
	int		i;
	float	totalKgs		= 0.0;
	double	freightValue	= 0.00;
	float	weight			= 0.00;
	double	calcMarkup		= 0.00;

	cohr_rec.no_kgs		= 0.00;
	estimFreight 		= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		weight = (store [i].itemWeight > 0.00) ? store [i].itemWeight 
										     : comr_rec.frt_mweight;
		totalKgs += (weight * store [i].qtyOrd);
	}
	/*-------------------------------
	| Cost by Kg by Carrier / Zone. |
	-------------------------------*/
	if (envVar.soFreightCharge == 3)
		freightValue = (double) totalKgs * carrierCostKg;

	/*---------------------
	| Cost by Kg by Zone. |
	---------------------*/
	if (envVar.soFreightCharge == 2)
		freightValue = (double) totalKgs * zoneCostKg;

	calcMarkup = (double) freightMarkup;
	calcMarkup *= freightValue;
	calcMarkup = DOLLARS (calcMarkup);
	
	freightValue += calcMarkup;
	freightValue = twodec (CENTS (freightValue));

	if (freightValue < comr_rec.frt_min_amt && freightValue > 0.00)
		estimFreight = comr_rec.frt_min_amt;
	else
		estimFreight = freightValue;
	
	if (FREIGHT_CHG && envVar.automaticFreight)
	{
		if (envVar.soFreightCharge == 1)
		{
			cohr_rec.freight = CENTS (zoneFixedAmount);
			estimFreight = CENTS (zoneFixedAmount);
		}
		else
			cohr_rec.freight = estimFreight;
	}
	cohr_rec.no_kgs = totalKgs;

	if (inScreenFlag)
		PrintTotalBoxValues ();

	return;
}

void
InputResponce (
 float	*lcl_qty)
{
	int		i;
	int		displayed = FALSE;
	float	wk_qty;
	char	val_keys [150];
	char	disp_str [150];

	cc = 0;
	skip_entry = 0;

	BusyFunction (BUSY_OFF);

	if (SERIAL)
	{
		sprintf (disp_str, ML (mlSoMess356), ta [8], ta [9], ta [8], ta [9]);
		strcpy (val_keys, "CcNnAa");

		if ((SR.qtyAvail) > 0.00)
		{
			sprintf (err_str, ML (mlSoMess357), ta [8], ta [9]);

			strcat (val_keys, "Rr");
			strcat (disp_str, err_str);
		}
	}
	else
	{

		if (envVar.overrideQuantity [0] == 'Y')
		{
			sprintf (disp_str,
			   ML (mlSoMess359),ta [8],ta [9],ta [8],ta [9],ta [8],ta [9],ta [8],ta [9]);
			strcpy (val_keys, "OoCcNnAaRr");
		}
		else 
		{
			sprintf (disp_str,
			   ML (mlSoMess383), ta [8], ta [9], ta [8], ta [9], ta [8], ta [9]);
			strcpy (val_keys, "CcNnAaRr");
		}
	}

	if (strcmp (inmr_rec.alternate,sixteen_space))
	{
		sprintf (err_str, ML (mlSoMess358), ta [8], ta [9]);

		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}
	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, 2);
		BusyFunction (BUSY_OFF);

		switch (i)
		{
		/*------------------------
		| Accept Quantity input. |
		------------------------*/
		case	'O':
		case	'o':
			break;

		/*------------------------------------------------------
		| Cancel Quantity input and check if log to lost sale. |
		------------------------------------------------------*/
		case	'C':
		case	'c':
			wk_qty = *lcl_qty;
			LogLostSales (ToStdUom (wk_qty));
			*lcl_qty = 0.00;
			skip_entry = goto_field (label ("qty_sup"),
					         		label ("item_no"));
			break;

		/*------------------------------
		| Display Stock Status Window. |
		------------------------------*/
		case	'N':
		case	'n':
		case	'A':
		case	'a':
#ifdef GVISION
			displayed = TRUE;
			if (i == 'N' || i == 'n')
				DisplayStockWindow (SR.hhsiHash, ccmr_rec.hhcc_hash);
			else   
				DisplayStockWindow (SR.hhsiHash, 0L);
#else
			BusyFunction (BUSY_ON);
			if (!wpipe_open)
			{
				if (OpenStockWindow ())
					break;
			}
			if (i == 'N' || i == 'n')
				fprintf (wout,"%10ld%10ld\n", SR.hhsiHash,
						      ccmr_rec.hhcc_hash);
			else
				fprintf (wout,"%10ld%10ld\n",SR.hhsiHash,0L);

			fflush (wout);
			IP_READ (functionFileNo);
			BusyFunction (BUSY_OFF);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*------------------------------------------------------
		| Quantity has been reduced to equal quantity on hand. |
		------------------------------------------------------*/
		case	'R':
		case	'r':
			wk_qty = ToStdUom (*lcl_qty);
			*lcl_qty = ToLclUom (SR.qtyAvail);
			if (*lcl_qty < 0.00)
				*lcl_qty = 0.00;

			LogLostSales (wk_qty - *lcl_qty);
			break;

		/*------------------------------
		| Substitute Alternate number. |
		------------------------------*/
		case	'S':
		case	's':
			sprintf (bonusFlag,"%-2.2s", envVar.soSpecial);
			sprintf (err_str,"%s%s",clip (inmr_rec.alternate),
						(BONUS) ? bonusFlag : " ");

			sprintf (local_rec.item_no,"%-16.16s",err_str);
			if (ValidateItemNumber (FALSE))
			{
				skip_entry = goto_field (label ("qty_sup"),
					         	  label ("item_no"));
			}
			else
			{
				CheckSerialQty (lcl_qty);
				skip_entry = -1;
			}

			DSP_FLD ("item_no");
			DSP_FLD ("descr");
			break;
		}
		print_at (2,1,"%90.90s"," ");

		if (i != 'D' && i != 'd' && i != 'A' && i != 'a')
			break;
	}

	if (displayed)
		ClearWindow ();

	return;
}

#ifndef GVISION
int
OpenStockWindow (void)
{
	functionFileNo = IP_CREATE (getpid ());
	if (functionFileNo < 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	if ((wout = popen ("so_pwindow","w")) == 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	wpipe_open = TRUE;
	fprintf (wout, "%06d\n", getpid ());
	return (EXIT_SUCCESS);
}
#endif	/* GVISION */

/*======================================================================
| Log lost sales from stock quantity on hand less-than input quantity. |
======================================================================*/
void
LogLostSales (
	float	lost_qty)
{
	int		i;
	char	shhbr_hash [10];
	char	shhcc_hash [10];
	char	shhcu_hash [10];
	char	wk_qty [11];
	char	wk_value [11];
		
	if (!envVar.lostSales)
		return;

	BusyFunction (BUSY_ON);
	i = prmptmsg (ML (mlStdMess177),"YyNn",1,2);
	if (i == 'N' || i == 'n')
	{
		BusyFunction (BUSY_OFF);
		return;
	}

	sprintf (shhbr_hash,"%09ld", alt_hash (inmr_rec.hhbr_hash,
					      inmr_rec.hhsi_hash));
	sprintf (shhcc_hash,"%09ld",incc_rec.hhcc_hash);
	sprintf (shhcu_hash,"%09ld",cumr_rec.hhcu_hash);
	sprintf (wk_qty,"%10.2f",lost_qty);

	if (envVar.dbMcurr && cohr_rec.exch_rate != 0.00)
	{
		sprintf (wk_value,"%10.2f",no_dec (coln_rec.sale_price / cohr_rec.exch_rate));
	}
	else
		sprintf (wk_value,"%10.2f",coln_rec.sale_price);


	* (arg) = "so_lostsale";
	* (arg+ (1)) = shhbr_hash;
	* (arg+ (2)) = shhcc_hash;
	* (arg+ (3)) = shhcu_hash;
	* (arg+ (4)) = cohr_rec.area_code;
	* (arg+ (5)) = cohr_rec.sale_code;
	* (arg+ (6)) = wk_qty;
	* (arg+ (7)) = wk_value;
	* (arg+ (8)) = "F";
	* (arg+ (9)) = (char *)0;
	shell_prog (2);
	ClearWindow ();
	BusyFunction (BUSY_OFF);
	return;
}

/*========================================
| Clear popup window ready for new item. |
========================================*/
void
ClearWindow (void)
{
	int		i;
#ifdef GVISION
	HideStockWindow ();
#else
	for (i = 17; i < 24 ; i++)
	{
		move (0,i); 
		fflush (stdout);
		cl_line ();
	}
#endif	/* GVISION */

	PrintCompanyDetails ();
}

/*============================
| Warn user about something. |
============================*/
int
WarnUser (
 char	*wn_mess,
 int	wn_flip,
 int	mess_len)
{
	int		wn_cnt;	
	int		i;
	
	if (INV_DISPLAY)
		return (EXIT_SUCCESS);

	for (wn_cnt = 1; wn_cnt < mess_len + 1 ; wn_cnt++)
	{
		clear_mess ();
		print_mess (wn_mess);
		sleep (SLEEP_TIME - 1);
	}
	clear_mess ();

	if (!wn_flip)
	{
		BusyFunction (BUSY_OFF);
		i = prmptmsg (ML ("Enter 'Y' to continue / 'N' to cancel / 'M' for more information on credit details [Y/N/M]"),"YyNnMm",1,2);
		move (1,2);
		cl_line ();
		if (i == 'Y' || i == 'y') 
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML ("Do you wish to continue?"),"YyNn",1,2);
			heading (SCN_HEADER);
			scn_display (SCN_HEADER);
			BusyFunction (BUSY_OFF);
			if (i == 'Y' || i == 'y') 
				return (EXIT_SUCCESS);
		}
		return (EXIT_FAILURE);
	}

	if (wn_flip == 9)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

void
BusyFunction (
 int	flip)
{
	print_at (2,1,"%-90.90s", (flip) ? ML (mlStdMess035) : " ");
	fflush (stdout);
}

void
PrintCompanyDetails (void)
{
	print_at (22,0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_short);
	print_at (22,30, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22,60, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_name);
	print_at (22,100, ML (mlStdMess085), cudp_rec.dp_no,  cudp_rec.dp_short);
}

void
CalculateBoxTotals (
 int	override,
 int	clear_disc)
{
	register	int		i;
	register	int		no_lines;
	double		other 		= 0.00;
	double		wk_gst 		= 0.00;
	float		gst_pc 		= 0.00;
	float		gst_div 	= 0.00;
	double		diff 		= 0.00;
	double		gst_amount 	= 0.00;

	no_lines = (prog_status == ENTRY && (lcount [SCN_ITEMS] - 1 < line_cnt)) 
					? line_cnt : lcount [SCN_ITEMS] - 1;

	totalInvoice 	= 0.00;
	totalLevy 		= 0.00;
	totalDisc 		= 0.00;
	totalTax 		= 0.00;
	totalGrand 		= 0.00;
	totalGst 		= 0.00;

	if (override)
		scn_set (SCN_ITEMS);

	for (i = 0;i <= no_lines;i++) 
	{
		if (LineInKit (i))
			continue;

		if (override)
		{
			getval (i);
			if (clear_disc)
			{
				coln_rec.disc_pc = 0.00;
				store [i].discPc = 0.00;
			}
			else
			{
				coln_rec.disc_pc += ScreenDisc (local_rec.disc_over);
				store [i].discPc = ScreenDisc (coln_rec.disc_pc);
			}
		}

		CalculateLineTotal (i);

		if (override)
		{
			local_rec.extend = store [i].extendTotal;
			putval (i);
		}

		totalInvoice 	+= lineGross;
		totalLevy 		+= lineLevy;
		totalDisc 		+= lineDisc;
		totalTax 		+= lineTax;
		totalGst 		+= lineGst;
	}
	totalGst = no_dec (totalGst);

	if (override && clear_disc)
		cohr_rec.other_cost_1 = 0.00;

	CalculateFreight 
	(
		trcm_rec.markup_pc, 
		trcl_rec.cost_kg,
		trzm_rec.chg_kg,
		trzm_rec.dflt_chg
	);

	other = cohr_rec.freight + 	
			cohr_rec.sos +
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 	
			cohr_rec.other_cost_3;

	if (noTaxCharged)
		wk_gst = 0.00;
	else
		wk_gst = (double) (comm_rec.gst_rate / 100.00);

	wk_gst *= other;
	totalGst += wk_gst;
	totalGst = no_dec (totalGst);

	if (envVar.dbNettUsed)
		totalGrand = no_dec (totalInvoice - totalDisc + totalTax + totalGst + other + totalLevy);
	else
		totalGrand = no_dec (totalInvoice + totalTax + totalGst + other + totalLevy);

	if (override > 1 && !clear_disc)
	{
		if (totalGrand != local_rec.inp_total)
		{
			diff = no_dec (local_rec.inp_total - totalGrand);

			if (noTaxCharged)
				wk_gst = 0.00;
			else
			{
				gst_pc = comm_rec.gst_rate;

				if (gst_pc != 0.00)
					gst_div = ((100 + gst_pc) / gst_pc);
				else
					gst_div = 0.00;

			}
			if (gst_div != 0.00)
				gst_amount = no_dec (diff / gst_div);
			else
				gst_amount = 0.00;

			cohr_rec.other_cost_1 = diff;
			cohr_rec.other_cost_1 -= gst_amount;
			totalGst += gst_amount;
			totalGst = no_dec (totalGst);
		}
	}
	if (!override)
		PrintTotalBoxValues ();

	local_rec.disc_over = 0.00;
}

void
DrawTotals (void)
{
	box (96,0,35,4);
	print_at (1,97,ML (mlSoMess064));
	print_at (2,97,ML (mlSoMess065));
	print_at (3,97,ML (mlSoMess066), envVar.gstCode);
	print_at (4,97,ML (mlSoMess068));

	PrintTotalBoxValues ();
}

void
PrintTotalBoxValues (void)
{
	double	f_other = 0.00;
	
	f_other	=	cohr_rec.freight + 	
		  		cohr_rec.sos +
		  		cohr_rec.insurance -
		  		cohr_rec.ex_disc + 
		  		cohr_rec.other_cost_1 +
		  		cohr_rec.other_cost_2 + 	
		  		cohr_rec.other_cost_3;

	if (envVar.dbNettUsed)
	{
		totalGrand = no_dec (totalInvoice - totalDisc + totalTax + totalGst + f_other + totalLevy);
		print_at (1,115,"%14.2f",DOLLARS (totalInvoice - totalDisc + totalLevy));
	}
	else
	{
		totalGrand = no_dec (totalInvoice + totalTax + totalGst + f_other + totalLevy);
		print_at (1,115,"%14.2f",DOLLARS (totalInvoice + totalLevy));
	}

	print_at (2,115,"%14.2f",DOLLARS (f_other));
	print_at (3,115,"%14.2f",DOLLARS (totalGst + totalTax));
	print_at (4,115,"%14.2f",DOLLARS (totalGrand));
}

/*==========================
| Validate margin percent. |
==========================*/
void
MarginCheckOk (
 double	salesMargin,
 float	discountMargin,
 double	costSaleMargin,
 float	minumumMargin)
{
	float	marg = 0.00;

	if (minumumMargin == 0.00)
		return;

	if (BONUS)
		return;

	salesMargin /= 100;

	salesMargin -= (salesMargin * ((double) discountMargin / 100));
	
	/*-------------------------
	| Convert to local value. |
	-------------------------*/
	if (envVar.dbMcurr)
		salesMargin /= pocrRec.ex1_factor;

	/*---------------------------
	| Calculate margin percent. |
	---------------------------*/
	marg = (float) salesMargin - (float) costSaleMargin;
	if (salesMargin != 0.00)
		marg /= (float) salesMargin;
	else
		marg = 0.00;
	
	marg *= 100.00;
	
	if (marg < minumumMargin && !MARG_MESS1)
	{
		if (MARG_MESS2)
			PauseForKey (2,1,ML (mlSoMess081),0);
		else
			PauseForKey (2,1,ML (mlSoMess081),0);
		print_at (2,0,"%-90.90s"," ");
	}
	return;
}

void
CalculateInputTotal (void)
{
	CalculateBoxTotals (2,TRUE);
	if (cohr_rec.tax_code [0] == 'D')
		local_rec.disc_over = CalculateOverride (totalGrand - totalTax);
	else
		local_rec.disc_over = CalculateOverride (totalGrand);
	CalculateBoxTotals (2,FALSE);
}

/*===========================
| Calculate overide amount. |
===========================*/
float	
CalculateOverride (
	double	amt)
{
	float	wk_disc = 0.00;

	if (amt == 0.00)
		return (0.00);

	wk_disc = (float) (OVER_AMT (totalGrand, local_rec.inp_total, amt));
	wk_disc *= 100;
	wk_disc = (float) (twodec (wk_disc));
	return (wk_disc);
}

/*=======================================================================
| Routine to check if hidden flag has been set (this is indicated by a  |
| '/H' on the end of the part number. If hidden flag is set then '/H'   |
| is removed from part number.                                          |
| Returns 0 if hidden flag has not been set, 1 if it has.               |
=======================================================================*/
int
CheckHiddenLine (
 char	*item_no)
{
	char	hidden_item [17];
	char	*sptr;

	sprintf (hidden_item,"%-16.16s",item_no);
	sptr = clip (hidden_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [2] && * (sptr + 1) == envVar.soSpecial [3])
		{
			*sptr = '\0';
			sprintf (item_no,"%-16.16s",hidden_item);
			if (cohr_rec.prt_price [0] == 'Y')
			{
				print_mess (ML (mlSoMess055));
				sleep (sleepTime);
				return (EXIT_SUCCESS);
			}
			else
				return (EXIT_FAILURE);
		}
	}
	sprintf (item_no,"%-16.16s",hidden_item);
	return (EXIT_SUCCESS);
}

/*======================================================================
| Routine to check if bonus flag has been set (this is indicated by a  |
| '/B' on the end of the part number. If bonus flag is set then '/B'   |
| is removed from part number.                                         |
| Returns 0 if bonus flag has not been set, 1 if it has.               |
======================================================================*/
int
CheckBonusLine (
 char	*item_no)
{
	char	bonus_item [17];
	char	*sptr;

	sprintf (bonus_item,"%-16.16s",item_no);
	sptr = clip (bonus_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [0]  && * (sptr + 1) == envVar.soSpecial [1])
		{
			*sptr = '\0';
			sprintf (item_no,"%-16.16s",bonus_item);
			return (EXIT_FAILURE);
		}
	}
	sprintf (item_no,"%-16.16s",bonus_item);
	return (EXIT_SUCCESS);
}

/*===============================================
| use_window is a procedure called by scrgen	|
| when FN14 or FN15 is pressed.					|
| _key is normally the same as last_char		|
| but by passing it as a parameter it allows	|
| the programmer to do some sneaky things		|
===============================================*/
int
use_window (
 int _key)
{
	static	long	last_hhcu;
	char	comment [132];

	if (cur_screen == 1)
		line_at (21,3,90);
	
	/*-----------------------------------------------
	| Only do anything when we are on screen 1 and	|
	| we've read a valid cumr.						|
	-----------------------------------------------*/
	if (cumr_rec.hhcu_hash == 0L)
	{
		last_hhcu = 0L;
		return (EXIT_SUCCESS);
	}
	if (cur_screen != 1)
	{
		last_hhcu = 0L;
		return (EXIT_SUCCESS);
	}

	if (FindCucc (_key,last_hhcu))
		return (EXIT_SUCCESS);

	if (last_hhcu != cumr_rec.hhcu_hash)
		last_hhcu = cumr_rec.hhcu_hash;

	crsr_off ();
	sprintf (comment, " [%-80.80s] ", cucc_rec.comment);
	us_pr (comment, 22, 3, 1);
	crsr_on ();
    return (EXIT_SUCCESS);
}

int
FindCucc (
 int	_key,
 long	last_hhcu)
{
	if (_key == 0)
	{
		cc = find_rec (cucc,&cucc_rec,COMPARISON,"r");
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);

		return (EXIT_FAILURE);
	}

	if (last_hhcu != 0L)
	{
		/*-------------------------------------------------------
		| Find the NEXT / PREVIOUS record to the current one	|
		-------------------------------------------------------*/
		cc = find_rec (cucc,&cucc_rec, (_key == FN14) ? GREATER : LT,"r");

		/*-----------------------------------------------
		| Woops, looks like we need to loop around	|
		-----------------------------------------------*/
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);
	}

	/*---------------
	| Finding Next	|
	---------------*/
	if (_key == FN14)
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = 0L;
		cc = find_rec (cucc,&cucc_rec,GTEQ,"r");
	}
	else
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash + 1L;
		cucc_rec.record_no = 0L;

		cc = find_rec (cucc,&cucc_rec,GTEQ,"r");

		/*-----------------------------------------------
		| Probably the last hhcu group in the cucc	|
		| so find the last record in the file.		|
		-----------------------------------------------*/
		if (cc)
			cc = find_rec (cucc,&cucc_rec,LAST,"r");
		else
			cc = find_rec (cucc,&cucc_rec,LT,"r");
	}

	if (cc || cucc_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	pr_box_lines (scn);

	if (INV_INPUT)
	{
		switch (invoiceTypeFlag [0])
		{
		case	'I':
			rv_pr (ML (mlSoMess167),24,0,1);
			break;

		case	'C':
			rv_pr (ML (mlSoMess168),24,0,1);
			break;

		case	'S':
			rv_pr (ML (mlSoMess169),24,0,1);
			break;

		case	'P':
			rv_pr (ML (mlSoMess170),18,0,1);
			break;

		default:
			break;
		}
	}
	else
	{
		switch (invoiceTypeFlag [0])
		{
		case	'I':
			rv_pr (ML (mlSoMess171),24,0,1);
			break;

		case	'C':
			rv_pr (ML (mlSoMess072),24,0,1);
			break;

		case	'P':
			rv_pr (ML (mlSoMess172),24,0,1);
			break;

		case	'S':
			rv_pr (ML (mlSoMess173),24,0,1);
			break;

		default:
			break;
		}
	}

	print_at (0,50,ML (mlSoMess174),PROMPT,local_rec.prev_dbt_no,
							local_rec.prev_inv_no);

	switch (scn)
	{
	case	SCN_HEADER:
		use_window (0);
		inScreenFlag	=	FALSE;
		break;

	case	SCN_ITEMS:
		inScreenFlag	=	TRUE;
		print_at (3,1, ML ("Active Keys [Window #1], [Window #2]"));
		print_at (4,0,ML (mlStdMess012),cumr_rec.dbt_no,clip (cumr_rec.dbt_name));
		if (envVar.dbMcurr)
			print_at (4,65,"   (%-3.3s)", cumr_rec.curr_code);
		fflush (stdout);

		DrawTotals ();
		if (local_rec.inp_total != 0.00 && prog_status != ENTRY)
		{
			CalculateInputTotal ();
			PrintTotalBoxValues ();
		}
		else
			CalculateBoxTotals (FALSE,FALSE);
		break;

	case	SCN_FREIGHT:
	case	SCN_MISC:
		inScreenFlag	=	FALSE;
		print_at (2,1,ML (mlStdMess012),
			cumr_rec.dbt_no, clip (cumr_rec.dbt_name));

		if (envVar.dbMcurr)
			print_at (2,65,"   (%-3.3s)", cumr_rec.curr_code);
		fflush (stdout);
		break;
	}

	PrintCompanyDetails ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	if (specialDisplay)
		print_at (2,1, "%R%s", GetPostingStatus (cohr_rec.stat_flag));

    return (EXIT_SUCCESS);
}

/*==================================
| Add freight history information. |
==================================*/
void
AddCarrierDetails (void)
{
	open_rec (trch, trch_list, TRCH_NO_FIELDS, "trch_id_no");

	strcpy (trch_rec.co_no, 	cohr_rec.co_no);
	strcpy (trch_rec.br_no, 	cohr_rec.br_no);
	strcpy (trch_rec.wh_no, 	ccmr_rec.cc_no);
	strcpy (trch_rec.ref_no, 	cohr_rec.inv_no);
	trch_rec.date 			= 	cohr_rec.date_raised;
	trch_rec.hhcu_hash 		= 	cohr_rec.hhcu_hash;
	strcpy (trch_rec.cons_no, 	cohr_rec.cons_no);
	strcpy (trch_rec.carr_code, cohr_rec.carr_code);
	strcpy (trch_rec.del_zone, 	cohr_rec.del_zone);
	trch_rec.no_cartons 	= 	cohr_rec.no_cartons;
	trch_rec.no_kgs 		= 	cohr_rec.no_kgs;
	trch_rec.est_frt_cst 	= 	DOLLARS (estimFreight);
	trch_rec.act_frt_cst 	= 	DOLLARS (cohr_rec.freight);
	strcpy (trch_rec.cumr_chg,	(cohr_rec.freight > 0.00) ?  "Y" : "N");
	strcpy (trch_rec.stat_flag, "0");

	cc = abc_add (trch, &trch_rec);
	if (cc)
		file_err (cc, trch, "DBADD");
}
/*===========================================
| Add record to background processing file. |
===========================================*/
void
AddSobg (
 int	_printerNumber,
 char	*_invoiceTypeFlag,
 long	_hash)
{
	open_rec (sobg,sobg_list,SOBG_NO_FIELDS,"sobg_id_no");
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _invoiceTypeFlag);
	sobg_rec.lpno = _printerNumber;
	sobg_rec.hash = _hash;
	if (!strcmp (sobg_rec.type, "SU"))
		sobg_rec.pid = progPid;
	else
		sobg_rec.pid = 0L;

	cc = find_rec (sobg,&sobg_rec,COMPARISON,"r");
	/*--------------------------------------------------------------
	| Add the record if an identical one doesn't already exist     |
	--------------------------------------------------------------*/
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _invoiceTypeFlag);
		sobg_rec.lpno = _printerNumber;
		sobg_rec.hash = _hash;
		if (!strcmp (sobg_rec.type, "SU"))
			sobg_rec.pid = progPid;
		else
			sobg_rec.pid = 0L;

		cc = abc_add (sobg,&sobg_rec);
		if (cc)
			file_err (cc, "sobg", "DBADD");

	}
	abc_fclose (sobg);
}
	
void
CheckSerialQty (
 float		*ser_qty)
{
	/*----------------------------------------------------
	| Validate to see if on hand is less than input qty. |
	----------------------------------------------------*/

	if (STANDARD && 
		envVar.windowPopupOk && 
		((SR.qtyAvail - *ser_qty) < 0.00) && 
		!NO_COST && 
		!NON_STOCK (line_cnt))
	{
		sprintf (err_str,ML (mlStdMess090),SR.qtyAvail,clip (local_rec.item_no));

		BusyFunction (BUSY_ON);
		cc = WarnUser (err_str,1,2);

		InputResponce (ser_qty);
	}
	return;
}
			
/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
void
ProcessKitItem (
 long		hhbr_hash,
 float		qty)
{
	int		i;
	int		this_page;
	char	*sptr;
	long	hold_date = cohr_rec.date_required;
	float	lcl_qty;

	this_page = line_cnt / TABLINES;

	cc = open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_id_no");
	if (cc)
		file_err (cc, sokt, "OPEN_REC");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = hhbr_hash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
		   sokt_rec.hhbr_hash == hhbr_hash)
	{
		abc_selfield (inmr,"inmr_hhbr_hash");
		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.bonus [0] == 'Y')
		{
			sprintf (bonusFlag, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), bonusFlag);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);

		dflt_used = FALSE;

		if (ValidateItemNumber (FALSE))
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (SR.hhwhHash < 0L)
		{
			sprintf (local_rec.serial_no, "%25.25s", " ");
			strcpy (SR.serialItem, local_rec.serial_no);
			DSP_FLD ("ser_no");
		}

		if (SERIAL)
		{
			lcl_qty = sokt_rec.matl_qty * qty;
			local_rec.qty_sup = 1.00;
			local_rec.qty_ord = local_rec.qty_sup;
		}
		else
		{
			local_rec.qty_sup = sokt_rec.matl_qty * qty;
			local_rec.qty_ord = local_rec.qty_sup;
		}

		if (local_rec.qty_ord == 0.00)
			get_entry (label ("qty_ord"));
		
		if (local_rec.qty_ord == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.due_date == 0L)
			cohr_rec.date_required = hold_date;
		else
			cohr_rec.date_required = sokt_rec.due_date;

		/*-----------------------------
		| if serial we need to to load
		| one line per qty ordered.
		------------------------------*/

		if (SERIAL)
		{
			int	count;

			abc_selfield (inmr, "inmr_hhbr_hash");
			inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
			cc = find_rec (inmr, &inmr_rec,COMPARISON,"r");

			/*--------------------------
			| Check Quantity available |
			--------------------------*/
			CheckSerialQty (&lcl_qty);

			for (count = 0; count < lcl_qty; count++)
			{

				if (sokt_rec.bonus [0] == 'Y')
				{
					sprintf (bonusFlag, "%-2.2s", envVar.soSpecial);
					sptr = clip (inmr_rec.item_no);
					sprintf (local_rec.item_no, "%-s%-.*s",
						     sptr, 16 - (int) strlen (sptr), bonusFlag);
				}
				else
					strcpy (local_rec.item_no, inmr_rec.item_no);

				dflt_used = FALSE;

				if (ValidateItemNumber (FALSE))
					break;

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					DSP_FLD ("item_no");
					DSP_FLD ("descr");
					lcount [SCN_ITEMS] = line_cnt;
					this_page = line_cnt / TABLINES;
				}

				for (i = label ("qty_sup"); i <= label ("extend"); i++)
				{
					skip_entry = 0;
					do
					{
						if (SERIAL && i == label ("ser_no"))
							get_entry (i);

						if (restart)
							return;

						cc = spec_valid (i);

						/*-----------------------------------------
						| if spec_valid returns 1, re-enter field |
						| eg. if kit item has no sale value,      |
						| re-prompt for sal value if required.    |
						-----------------------------------------*/
						if (cc && !(SERIAL && i == label ("ser_no")))
							get_entry (i);

						if (restart)
							return;

					} while (cc);
					i += skip_entry;
					if (i == label ("item_no") - 1)
						break;
				}
		
				if (i != label ("item_no") - 1)
				{
					putval (line_cnt);
					SR.qtySup = local_rec.qty_sup;
	
					if (this_page != (line_cnt / TABLINES))
					{
						scn_write (cur_screen);
						lcount [SCN_ITEMS] = line_cnt;
						this_page = line_cnt / TABLINES;
					}
					lcount [SCN_ITEMS] = line_cnt;
					
					line_display ();
					line_cnt++;
				}
				else
					blank_display ();
			}
		}
		else
		{
			if (this_page != (line_cnt / TABLINES))
			{
				scn_write (cur_screen);
				DSP_FLD ("item_no");
				DSP_FLD ("descr");
				lcount [SCN_ITEMS] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			for (i = label ("qty_sup"); i <= label ("extend"); i++)
			{
				skip_entry = 0;
				do
				{
					if (SERIAL && i == label ("ser_no"))
						get_entry (i);

					if (restart)
						return;

					cc = spec_valid (i);
					/*-----------------------------------------
					| if spec_valid returns 1, re-enter field |
					| eg. if kit item has no sale value,      |
					| re-prompt for sal value if required.    |
					-----------------------------------------*/
					if (cc && !(SERIAL && i == label ("ser_no")))
						get_entry (i);

					if (restart)
						return;

				} while (cc);
				i += skip_entry;

				if (i == label ("item_no") - 1)
					break;
			}
		
			if (i != label ("item_no") - 1)
			{
				putval (line_cnt);
				SR.qtySup = local_rec.qty_sup;

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					lcount [SCN_ITEMS] = line_cnt;
					this_page = line_cnt / TABLINES;
				}
				lcount [SCN_ITEMS] = line_cnt;
			
				line_display ();
				line_cnt++;
			}
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [SCN_ITEMS] = line_cnt;
	abc_fclose (sokt);
	cohr_rec.date_required = hold_date;
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
float	
ProcessPhantom (
	long	hhbrHash)
{
	int		FirstPass = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;
	float	realCommitted;

	open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (sokt,&sokt_rec,NEXT,"r");
			continue;
		}
	
		realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 
										   incc_rec.hhcc_hash,
										   0);
		if (envVar.includeForwardStock)
		{
			on_hand = incc_rec.closing_stock -
					(incc_rec.committed + realCommitted)-
					  incc_rec.backorder - 
		   	          incc_rec.forward;
		}
		else
		{
			on_hand = incc_rec.closing_stock -
					(incc_rec.committed + realCommitted) -
					  incc_rec.backorder;
		}
		if (envVar.qCApplies && envVar.qCAvailable)
			on_hand -= incc_rec.qc_qty;
		on_hand /= sokt_rec.matl_qty;
		if (FirstPass)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		FirstPass = FALSE;

		cc = find_rec (sokt,&sokt_rec,NEXT,"r");
	}
	abc_fclose (sokt);

	return (min_qty);
}
/*=================================
| Gets posting status description |
=================================*/
char 	*
GetPostingStatus (
 char	*stat)
{
	char	*sptr = getenv ("PROG_PATH");
	char	*tptr;
	char	filename [100];
	static	char	description [91];
	char	Data [256];
	FILE	*fin;

#ifdef GVISION
	sptr = ServerPROG_PATH ();
#endif

	sprintf (filename,"%s/BIN/Post_Status", (sptr != (char *)0) 
							? sptr : "/usr/DB");

	if ((fin = fopen (filename,"r")) == 0)
	{
		sprintf (description, "%88.88s", " ");
		return (description);
	}

	sptr = fgets (Data,150,fin);
	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';

		/*------------------------
		| Look for end of status |
		------------------------*/
		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		if (*tptr)
		{
			*tptr = '\0';

			/*-------------------
			| Grab printer name |
			-------------------*/
			if (strchr (sptr, stat [0]))
			{
				fclose (fin);
				sprintf (description, "%-88.88s", ++tptr);
				return (description);
			}
		}
		sptr = fgets (Data,150,fin);
	}
	fclose (fin);
	sprintf (description, "%88.88s", " ");
	return (description);
}

void
tab_other (
 int line_no)
{
	/*-------------------------------------------------------------------------
	| turn off and on editing of fields depending on whether contract or not. |
	------------------------------------------------------------------------*/
	FLD ("disc") 		= inputDiscValue;
	FLD ("sale_price") 	= inputSaleValue;

	if (store [line_no].contStatus)
	{
		FLD ("disc") = (store [line_no].contStatus == 1) ? NA : inputDiscValue;
		FLD ("sale_price") 	= NA;
	}


	if (store [line_no].serialFlag [0] == 'Y')
	{
		if (!F_HIDE (label ("ser_no")))
			FLD ("ser_no") = YES;
	}
	else
	{
		if (!F_HIDE (label ("ser_no")))
			FLD ("ser_no") = NA;
	}
	if (invoiceTypeFlag [0] == 'P')
		FLD ("item_no") = (store [line_no].oldLine) ? NE : YES;
	else
	{
		if (!INV_DISPLAY)
			FLD ("item_no") = YES;
		else
			FLD ("item_no") = NA;
	}	

	if (cur_screen == 2)
	{
		TidySonsScreen ();

		DispSONS (line_no); 
	}
}

/*============================================  
| Routine to remove item at warehouse level. |
============================================*/
void            
RemoveSerialNo
 (
    long    hhwh_hash,
	long    hhbr_hash,  
	char    *ser_no
)
{
	insfRec.hhwh_hash           = hhwh_hash;
	strcpy (insfRec.status,     "C");
	strcpy (insfRec.serial_no,  ser_no);
												
	cc = find_rec (insf,&insfRec,EQUAL,"u");
	if (!cc)
	{
		cc = abc_delete (insf);
		if (cc)
																						file_err (cc, insf, "DBDELETE");
	}
}

void
Init_store
 (
 int        line_no
)
{
	memset ((char *) &store [line_no], '\0', sizeof (struct storeRec));

	strcpy (store [line_no].serialItem,  ser_space);
	strcpy (store [line_no].origSerial,    ser_space);
	strcpy (store [line_no].category,    "           ");
	strcpy (store [line_no].sellGroup, "      ");
	strcpy (store [line_no].bonusItem,   " ");
	strcpy (store [line_no].itemClass,   " ");
	strcpy (store [line_no].discOveride,  "N");
	strcpy (store [line_no].priceOveride,  "N");
	strcpy (store [line_no].packSize,"     ");
	strcpy (store [line_no].oldInsf, "N");
	store [line_no].kitFlag  =   K_NONE;
	store [line_no].extendTotal   =   0.00;
}

int
SrchCudi (
	int		indx)
{
	char	workString [170];

	_work_open (5,0,80);
	save_rec ("#DelNo","#Delivery Details");
	cudi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{                        
		sprintf 
		(
			workString,"%s, %s, %s, %s",
			clip (cudi_rec.name),
			clip (cudi_rec.adr1),
			clip (cudi_rec.adr2),
			clip (cudi_rec.adr3)
		);
		sprintf (err_str, "%05d", cudi_rec.del_no);
		cc = save_rec (err_str, workString); 
		if (cc)
			break;

		cc = find_rec (cudi, &cudi_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (-1);

	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= atoi (temp_str);
	cc = find_rec (cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cudi, "DBFIND");

	switch (indx)
	{
	case	0:
		sprintf (temp_str,"%-40.40s",cudi_rec.name);
		break;

	case	1:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr1);
		break;

	case	2:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr2);
		break;

	case	3:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr3);
		break;

	default:
		break;
	}
	return (cudi_rec.del_no);
}

/*===============================
| Search on Contract (cnch)     |
===============================*/
void
SrchCnch (
 char    *key_val)
{
	work_open ();
	save_rec ("#Contract     ","#Description");

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, key_val);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no))
	{                        
		if (strncmp (cnch_rec.cont_no, key_val, strlen (key_val)))
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}

		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (cnch_rec.cont_no,cnch_rec.desc);
		if (cc)
				break;
		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, cnch, "DBFIND");
}

/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc (
 float	DiscountPercent)
{
	if (envVar.reverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
SrchInum (
 char    *key_val)
{
    work_open ();
    save_rec ("#UOM ","#Description");

    strcpy (inum2_rec.uom_group, SR.uomGroup);
    strcpy (inum2_rec.uom, key_val);
    cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
    while (!cc && !strcmp (inum2_rec.uom_group, SR.uomGroup))
    {
        if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
        {
            cc = find_rec (inum2, &inum2_rec, NEXT, "r");
            continue;
        }

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom,inum2_rec.desc);
			if (cc)
				break;
		}

        cc = find_rec (inum2, &inum2_rec, NEXT, "r");
    }

    cc = disp_srch ();
    work_close ();
    if (cc)
		return;

    strcpy (inum2_rec.uom_group, 	SR.uomGroup);
    strcpy (inum2_rec.uom, 			key_val);
    cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
    if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char *key_val)
{
	work_open ();

	save_rec ("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.est_no) &&
				  !strncmp (trzm_rec.del_zone, key_val, strlen (key_val)))
	{
		cc = save_rec (trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;
		
		cc = find_rec (trzm, &trzm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");

	return;
}
/*==========================
| Search for carrier code. |
==========================*/
void
SrchTrcm (
 char	*key_val)
{
	char	key_string [31];
	long 	currentZoneHash	=	trzm_rec.trzm_hash;

	_work_open (20, 11, 50);

	save_rec ("#Carrier","# Rate Kg. | Carrier Name.");
	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec,GTEQ,"r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) && 
		      	  !strncmp (trcm_rec.carr_code, key_val,strlen (key_val)))
	{
		trcl_rec.trcm_hash	=	trcm_rec.trcm_hash;
		trcl_rec.trzm_hash	=	0L;
		cc = find_rec (trcl, &trcl_rec,GTEQ,"r");
		while (!cc && trcl_rec.trcm_hash == trcm_rec.trcm_hash)
		{
	
			if (currentZoneHash	> 0L && trcl_rec.trzm_hash != currentZoneHash)
			{
				cc = find_rec (trcl, &trcl_rec,NEXT,"r");
				continue;
			}
			trzm_rec.trzm_hash	=	trcl_rec.trzm_hash;
			cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (err_str, " %8.2f | %-40.40s", 
							trcl_rec.cost_kg,
							trzm_rec.desc);

				sprintf (key_string, "%-4.4s-%-6.6s %010ld",
							trcm_rec.carr_code,
							trzm_rec.del_zone,
							trzm_rec.trzm_hash);

				cc = save_rec (key_string, err_str);
				if (cc)
					break;
			}
			cc = find_rec (trcl, &trcl_rec,NEXT,"r");
		}
		cc = find_rec (trcm, &trcm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, trcm, "DBFIND");

	trzm_rec.trzm_hash	=	atol (temp_str + 12);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (!cc)
		sprintf (local_rec.defaultDelZone, "%-6.6s", trzm_rec.del_zone);

}
/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment (void)
{
	char	*sptr;

	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("ADVERT_LEVY");
	envVar.advertLevy = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*--------------------------------------
	| Check for discounts on Indent items. |
	--------------------------------------*/
	sptr = chk_env ("SO_DIS_INDENT");
	envVar.discountIndents = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*---------------------------------
	| Check if lost sales are logged. |
	---------------------------------*/
	sptr = chk_env ("SO_LOST_SALES");
	envVar.lostSales = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_NUMBER");
	envVar.invoiceNumbers = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	/*------------------------------
	| Get value of override option |
	------------------------------*/
	sptr	=	chk_env ("SO_OVERRIDE_QTY");
	if (sptr == (char *)0)
		strcpy (envVar.overrideQuantity, "Y");
	else
		sprintf (envVar.overrideQuantity, "%-1.1s", sptr);

	/*-------------------------------------
	| Check for Automatic freight charge. |
	-------------------------------------*/
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*--------------------------------
	| Check if nett pricing is used. |
	--------------------------------*/
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sprintf (envVar.automaticStockUpdate, "%-1.1s",get_env ("AUTO_SK_UP"));

	/*---------------------------
	| Check for Currency Code.  |
	---------------------------*/
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVar.gstApplies = 0;
	else
		envVar.gstApplies = (*sptr == 'Y' || *sptr == 'y');

	/*---------------
	| Get gst code. |
	---------------*/
	if (envVar.gstApplies)
		sprintf (envVar.gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVar.gstCode, "%-3.3s", "Tax");

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envVar.numberPrices = 5;
	else
	{
		envVar.numberPrices = atoi (sptr);
		if (envVar.numberPrices > 9 || envVar.numberPrices < 1)
			envVar.numberPrices = 9;
	}
	sprintf (envVar.priceTypeComment,"Enter price type [1 - %1d]",
													envVar.numberPrices);

	sprintf (envVar.manualPrint,"%-1.1s",get_env ("SO_MAN_PRINT"));

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*-------------------------------------------------------------
	| Check if special codes for bonus and hidden lines are used. |
	-------------------------------------------------------------*/
	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVar.soSpecial,"/B/H");
	else
		sprintf (envVar.soSpecial,"%-4.4s", sptr);

	/*--------------------------------------
	| Check for sales order margin checks. |
	--------------------------------------*/
	sptr = chk_env ("SO_MARGIN");
	sprintf (envVar.salesOrderMargin, "%-2.2s", 
									(sptr == (char *) 0) ? "00" : sptr);


	/*---------------------------------
	| Check for sales order analysis. |
	---------------------------------*/
	sptr = chk_env ("SO_SALES");
	envVar.salesOrderSales = (sptr == (char *)0) ? 0 : atoi (sptr);

	/* QC module is active or not. */
	envVar.qCApplies = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/*-----------------------------------------------
	| Whether to include QC qty in available stock. |
	-----------------------------------------------*/
	envVar.qCAvailable = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*-----------------------------------
	| Validate is serial items allowed. |
	-----------------------------------*/
	sptr = chk_env ("SK_SERIAL_OK");
	envVar.serialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	envVar.useSystemDate = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	/*---------------------------------------
	| Check if available stock is included. |
	---------------------------------------*/
	sptr = chk_env ("SO_FWD_AVL");
	envVar.includeForwardStock = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVar.windowPopupOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-----------------------------------------------------------
	| Check if stock information window is loaded at load time. |
	-----------------------------------------------------------*/
	sptr = chk_env ("SO_PERM_WIN");
	envVar.perminantWindow = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envVar.perminantWindow)
	{
		if (OpenStockWindow ())
			envVar.windowPopupOk = FALSE;
	}
	/*-----------------------------
	| Check and Get Credit terms. |
	-----------------------------*/
	sptr = get_env ("SO_CRD_TERMS");
	envVar.creditStop 	= (* (sptr + 0) == 'S');
	envVar.creditTerms 	= (* (sptr + 1) == 'S');
	envVar.creditOver 	= (* (sptr + 2) == 'S');

	/*---------------------------------------
	| Get descriptions for other costs 1-3. |
	---------------------------------------*/
	sptr = chk_env ("SO_OTHER_1");
	sprintf (envVar.other [0],"%.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (envVar.other [1],"%.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (envVar.other [2],"%.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	/*--------------------------
	| Customer Company Owned.  |
	--------------------------*/
	sptr = chk_env ("DB_CO");
	envVar.dbCo = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*------------------------------------
	| Customer Find variable for Search. |
	------------------------------------*/
	sptr = chk_env ("DB_FIND");
	envVar.dbFind = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*--------------------------
	| How if Freight Charged.  |
	--------------------------*/
	sptr = chk_env ("SO_FREIGHT_CHG");
	envVar.soFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);

	sptr = chk_env ("KIT_DISC");
	envVar.KitDiscount = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("RECALC_KIT");
	Recalc_kit = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*------------------------------------------------------
	| To check if the default inv. no. is system generated.|
	------------------------------------------------------*/
	sptr = chk_env ("SO_NUM_FORCE");
	SysGenInv = (sptr == (char *)0) ? FALSE : atoi (sptr);
}

/*===============================
| Open Transport related files. |
===============================*/
void
OpenTransportFiles (
	char	*ZoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, ZoneIndex);
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
}
/*================================
| Close Transport related files. |
================================*/
void
CloseTransportFiles (void)
{
	abc_fclose (trzm);
	abc_fclose (trcm);
	abc_fclose (trcl);
}
/*=============================
| To standard unit of measure |
=============================*/
float
ToStdUom (
 float   lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.convFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty * SR.convFct;

    return (cnvQty);
}

/*==========================
| To local unit of measure |
==========================*/
float
ToLclUom (
 float   lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.convFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty / SR.convFct;

    return (cnvQty);
}

/*===========================================================
| Input not stock description lines for non-stock products. |
===========================================================*/
void
InputSONS (
 int	Header,
 int	line_cnt)
{
	int 	i, 
			tx_window;
	char	disp_str [200];

	int		OffSet;

	OffSet = tab_row + 3;
		
	if (Header)
		strcpy (err_str,"Additional Description for Sales Order  ");
	else
		sprintf (err_str,"Additional Desc.  for (%-16.16s)",local_rec.item_no);

	tx_window = txt_open ((Header) ? tab_row : OffSet, 
						(Header) ? 45 : 20, 
						(Header) ? 10 : 3, 
						  40, 
						  MAX_SONS, 
						  err_str);

	for (i = 0; i < MAX_SONS ; i++)
	{
		if (Header)
			sprintf (disp_str, "%-40.40s", HeaderDesc [i]);
		else
			sprintf (disp_str, "%-40.40s", SR.nsDesc [i]);

		txt_pval (tx_window, disp_str, 0);
	}

	txt_edit (tx_window);

	for (i = 0; i < MAX_SONS ; i++)
	{
		if (Header)
			sprintf (HeaderDesc [i],"%-40.40s", txt_gval (tx_window, i + 1));
		else
			sprintf (SR.nsDesc [i], "%-40.40s", txt_gval (tx_window, i + 1));
	}

	txt_close (tx_window, FALSE);
}

/*===========================================================
| Input not stock description lines for non-stock products. |
===========================================================*/
void
DispSONS (
 int	line_cnt)
{
	char	disp_str [200];

	int		i;

	if (!strcmp (SR.nsDesc [0],  ns_space)) 
		return;
	
	for (i = 0; i < 4 ; i++)
	{
		if (i)
			sprintf (disp_str, "%38.38s: %-40.40s", ns_space, SR.nsDesc [i]);
		else
		{
			sprintf (disp_str, ML ("Additional desc. for %16.16s : %40.40s"),local_rec.item_no, SR.nsDesc [i]);
		}

		print_at (i+2,1, "%R %-82.82s", disp_str);
	}
}
/*=============================================
| Update purchase order non stock lines file. |
=============================================*/
void
UpdateSONS (
 int	Header,
 int	line_cnt, 
 long	Hash)
{
	int	i;

	abc_selfield (sons, (Header) ? "sons_id_no4" : "sons_id_no2");

	for (i = 0; i < MAX_SONS; i++)
	{
		memset (&sons_rec, 0, sizeof (sons_rec));
		if (Header)
			sons_rec.hhco_hash 	= Hash;
		else
			sons_rec.hhcl_hash 	= Hash;
		sons_rec.line_no 	= i;
		cc = find_rec (sons, &sons_rec, COMPARISON, "u");
		if (cc)
		{
			if (Header)
				sprintf (sons_rec.desc, "%-40.40s", HeaderDesc [i]);
			else
				sprintf (sons_rec.desc, "%-40.40s", SR.nsDesc [i]);

			/*-----------------------------------
			| Add line only if it is not blank. |
			-----------------------------------*/
			if (strcmp (sons_rec.desc, ns_space))
			{
				cc = abc_add (sons, &sons_rec);
				if (cc)
					file_err (cc, sons, "DBADD");
			}
		}
		else
		{
			if (Header)
				sprintf (sons_rec.desc, "%-40.40s", HeaderDesc [i]);
			else
				sprintf (sons_rec.desc, "%-40.40s", SR.nsDesc [i]);

			/*--------------------------------------
			| Update line only if it is not blank. |
			--------------------------------------*/
			if (strcmp (sons_rec.desc, ns_space))
			{
				cc = abc_update (sons, &sons_rec);
				if (cc)
					file_err (cc, sons, "DBUPDATE");
			}
			else
			{
				cc = abc_delete (sons);
				if (cc)
					file_err (cc, sons, "DBDELETE");
			}
		}
	}
}

/*===========================================
| Load purchase order non stock lines file. |
===========================================*/
void
LoadSONS (
 int	Header,
 int	line_cnt,
 long	Hash)
{
	int	i;

	abc_selfield (sons, (Header) ? "sons_id_no4" : "sons_id_no2");

	for (i = 0; i < MAX_SONS; i++)
	{
		if (Header)
			sprintf (HeaderDesc [i], "%40.40s", " ");
		else
			sprintf (SR.nsDesc [i],  "%40.40s", " ");
	}

	if (Header)
	{
		sons_rec.hhco_hash 	= Hash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhco_hash == Hash)
		{
			sprintf (HeaderDesc [sons_rec.line_no], "%40.40s", sons_rec.desc);

			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}
	}
	else
	{
		sons_rec.hhcl_hash 	= Hash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhcl_hash == Hash)
		{
			sprintf (SR.nsDesc [sons_rec.line_no], "%40.40s", sons_rec.desc);

			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}
	}
}

/*=============================================
| Delete purchase order non stock lines file. |
=============================================*/
void
DeleteSONS (
 int	Header,
 long	Hash)
{
	if (Header)
	{
		abc_selfield (sons, "sons_id_no4");
		sons_rec.hhco_hash 	= Hash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhco_hash == Hash)
		{
			cc = abc_delete (sons);
			if (cc)
				file_err (cc, sons, "DBDELETE");

			sons_rec.hhco_hash 	= Hash;
			sons_rec.line_no 	= 0;
			cc = find_rec (sons, &sons_rec, GTEQ, "r");
		}
	}
	else
	{
		abc_selfield (sons, "sons_id_no2");
		sons_rec.hhcl_hash 	= Hash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhcl_hash == Hash)
		{
			cc = abc_delete (sons);
			if (cc)
				file_err (cc, sons, "DBDELETE");

			sons_rec.hhcl_hash 	= Hash;
			sons_rec.line_no 	= 0;
			cc = find_rec (sons, &sons_rec, GTEQ, "r");
		}
	}
}

void
RunningKit
 (
    int curr_line
)
{
	int     tmp_line;

	if (IS_ENTRY || !Recalc_kit ||
		(!LineInKit (curr_line) && store [curr_line].kitFlag != K_END))
		return;

	tmp_line = line_cnt;
	putval (line_cnt);
	while (!KIT_END && line_cnt < lcount [SCN_ITEMS])
		line_cnt++;

	if (line_cnt < lcount [SCN_ITEMS])
	{
		getval (line_cnt);
		CalcKitLine ();
		coln_rec.sale_price = SR.extendTotal;
		if (SR.qtySup)
			coln_rec.sale_price /= SR.qtySup;

		coln_rec.sale_price = no_dec (coln_rec.sale_price);
		SR.salePrice 		= coln_rec.sale_price;
		SR.actSale 			= coln_rec.sale_price;
		DSP_FLD ("sale_price");
		PrintExtendedTotal (line_cnt);
		putval (line_cnt);
	}
	line_cnt = tmp_line;
	getval (line_cnt);
}

int
LineInKit
 (
    int line_no
)
{
	register    int i;
	int in_kit;

	for (in_kit = FALSE,i = 0;i <= line_no;i++)
	{
		if (store [i].kitFlag == K_START)
			in_kit = TRUE;

		if (store [i].kitFlag == K_END)
			in_kit = FALSE;
	}
	return (in_kit);
}

void
CalcKitLine
 (
    void
)
{
	register    int line_no;
	double  k_value = (double) 0;

	for (line_no = 0;line_no <= line_cnt;line_no++)
	{
		switch (store [line_no].kitFlag)
		{
		case    K_START:
			k_value = 0.00;
			break;

		case    K_END:
			store [line_no].extendTotal = k_value;
			break;

		default:
			k_value += store [line_no].extendTotal;
			break;
		}
	}
}

void
SillyBusyFunc (
	int flip)
{
	print_at (2,1,"%-90.90s", (flip) ? ML (mlStdMess035) : " ");
	fflush (stdout);
}

/*===================================
| Force the completion of open kits |
| on the item screen                |
===================================*/
void
ForceKitCompletion 
 (
	void
)
{
	int		tmpLineCnt;
	int		firstTime = TRUE;

	/*---------------------------------
	| Don't perfrom this check if the |
	| user has pressed FN1 to restart |
	---------------------------------*/
	if (restart)
		return;

	/*---------------------------------
	| Put the user on the item screen |
	| to complete the kit.            |
	---------------------------------*/
	while (FoundOpenKit ())
	{
		print_mess (ML ("Please complete the open kit on the item screen"));
		sleep (sleepTime);
		clear_mess ();
		
		if (firstTime)
		{
			heading (SCN_ITEMS);
			firstTime = FALSE;
		}
		tmpLineCnt = line_cnt;
		line_cnt = lcount [SCN_ITEMS] - 1;
		scn_display (SCN_ITEMS);
		line_cnt = tmpLineCnt;

		_edit (SCN_ITEMS, lcount [SCN_ITEMS] - 1, 0);

		if (restart)
			break;
	}
}

/*=================================
| Determine whether there is an   |
| open kit on the item screen.    |
=================================*/
int
FoundOpenKit 
 (
	void
)
{
	int		i;
	int		foundOpenKit;

	foundOpenKit = FALSE;
	for (i = 0; i < lcount [SCN_ITEMS]; i++)
	{
		if (store [i].kitFlag == K_END &&
			foundOpenKit)
		{
			foundOpenKit = FALSE;
			continue;
		}
		
		if (store [i].kitFlag == K_START)
			foundOpenKit = TRUE;
	}

	return (foundOpenKit);
}

/*===============================================
| Check that kits are contructed properly	|
===============================================*/
int
CheckKitting (void)
{
	int	line_no;
	int	inKit 		= FALSE;
	int	nonStock 	= FALSE;

	/*-----------------------
	| Kitting Not Used	|
	-----------------------*/
	if (kitHash == 0L)
		return (EXIT_SUCCESS);

	for (line_no = 0;line_no < lcount [SCN_ITEMS];line_no++)
	{
		/*-----------------------
		| Start / End of Kit	|
		-----------------------*/
		if (store [line_no].hhbrHash == kitHash)
		{
			if (inKit == TRUE)
				inKit = FALSE;
			else
				inKit = TRUE;
		}
		if (inKit == TRUE && NON_STOCK (line_no))
			nonStock	=	TRUE;
	}

	if (nonStock == FALSE)
	{
		sprintf (err_str,ML ("Kit has no comment line."));
		print_mess (err_str);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	if (inKit == TRUE)
	{
		print_mess (ML ("No End of Kit for Last Kit"));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

char	*
GetPriceDesc (
	int		priceNumber)
{

	switch (priceNumber)
	{
		case	0:
			return (comm_rec.price1_desc);
		break;

		case	1:
			return (comm_rec.price2_desc);
		break;

		case	2:
			return (comm_rec.price3_desc);
		break;

		case	3:
			return (comm_rec.price4_desc);
		break;

		case	4:
			return (comm_rec.price5_desc);
		break;

		case	5:
			return (comm_rec.price6_desc);
		break;

		case	6:
			return (comm_rec.price7_desc);
		break;

		case	7:
			return (comm_rec.price8_desc);
		break;

		case	8:
			return (comm_rec.price9_desc);
		break;

		default:
			return ("??????????");
	}
}

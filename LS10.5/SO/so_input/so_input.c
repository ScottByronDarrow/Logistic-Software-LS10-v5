/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: so_input.c,v 5.40 2002/11/28 04:09:50 scott Exp $
|  Program Name  : (so_input.c    )                                  |
|  Program Desc  : (Sales Order Input / Maintenance (Standard)  )    |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21st June 1988   |
|---------------------------------------------------------------------|
| $Log: so_input.c,v $
| Revision 5.40  2002/11/28 04:09:50  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.39  2002/08/14 04:38:42  scott
| Updated for Linux warning
|
| Revision 5.38  2002/07/24 08:39:26  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.37  2002/07/17 09:58:08  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.36  2002/07/17 09:29:45  robert
| Use HideStockWindow instead of CloseStockWindow during window clear
|
| Revision 5.35  2002/07/17 08:37:46  scott
| Updated for GUI as window not cleared
|
| Revision 5.34  2002/07/09 07:17:13  scott
| S/C 004039. Updated to close window in GUI
|
| Revision 5.33  2002/06/25 08:04:26  scott
| SC 3971 - The Department No is enabled when all the rest are not.  Though the program does not save whatever changes there would be.
|
| Revision 5.32  2002/06/20 07:16:02  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.31  2002/06/20 05:49:01  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.30  2002/04/15 09:08:10  cha
| S/C 691. Updated to make sure that sale_price will be edited during KIT ITEM.
| Updated to make sure that required in KIT items will be equal to system date
| or date required.
|
| Revision 5.29  2002/02/15 02:56:05  cha
| Updated to fix spec_valid for no charge item "sale_price".
|
| Revision 5.28  2002/02/04 10:33:02  cha
| S/C 691.
| Updated to fix Location in Kit items.
| Also fixed screens when dealing with kit items.
|
| Revision 5.27  2002/01/29 07:38:22  robert
| SC-00705: Updated to fix memory dump in LS10-GUI.
|
| Revision 5.26  2002/01/21 08:05:15  scott
| Updated to only show warning with Maintenance.
|
| Revision 5.25  2002/01/18 02:49:30  scott
| Updated to never allow Phantoms, Indent and non stock items to enter location selection
|
| Revision 5.24  2002/01/17 03:21:02  scott
| S/C 00685
|
| Revision 5.23  2002/01/16 08:16:48  scott
| Updated to work for indents related to locations.
| Updated to ensure serial numbers working correctly.
|
| Revision 5.22  2002/01/09 08:49:50  scott
| Updated to ensure Phantom items are handled correctly
|
| Revision 5.21  2002/01/09 01:12:11  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.20  2001/12/13 05:44:05  scott
| Updated to fix search on price showing wrong code.
|
| Revision 5.19  2001/11/29 03:28:38  cha
| Updated to make sure that WO is generated in LS10-GUI.
|
| Revision 5.18  2001/11/22 07:33:15  scott
| Small clean up
|
| Revision 5.17  2001/11/14 03:51:04  scott
| Updated to ensure contract description and head office description
| are displayed when order maintained.
|
| Revision 5.16  2001/11/08 09:35:03  scott
| Updated to fix overlap of contract description
|
| Revision 5.15  2001/10/23 07:16:41  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.14  2001/10/18 10:56:01  robert
| Updated to fix floating point problem with LS10-GUI
|
| Revision 5.13  2001/10/09 22:56:37  scott
| Updated from Scotts Machine
|
| Revision 5.12  2001/09/28 08:09:27  cha
| Updated to correct some rounding off
| problems in the display only.
|
| Revision 5.11  2001/09/27 03:02:52  scott
| Updated to add AllocationRestore () and AllocationComplete ()
| Fixed problem with allocation to lots being lost if restart performed.
| Updated to ensure allocation performed correctly.
|
| Revision 5.10  2001/09/26 23:16:33  scott
| Updated from Scott's Machine
|
| Revision 5.9  2001/09/11 23:46:03  scott
| Updated from Scott machine - 12th Sep 2001
|
| Revision 5.8  2001/09/11 06:26:07  cha
| SE-254. Updated to put delays in error messages.
|
| Revision 5.7  2001/08/28 06:05:37  scott
| Updated to fix currency overwriting memory space for customer
|
| Revision 5.6  2001/08/28 02:42:12  scott
| Update to include special discount
|
| Revision 5.5  2001/08/23 11:45:56  scott
| Updated from scotts machine
|
| Revision 5.4  2001/08/09 09:21:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/06 23:51:19  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_input.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_input/so_input.c,v 5.40 2002/11/28 04:09:50 scott Exp $";

extern	int		SR_X_POS;
extern	int		SR_Y_POS;
extern	int		Y_EALL;
extern	int		X_EALL;
extern	int		_win_func;

int		orig_disc;
int		orig_sale;

#define	USE_WIN		1
#define MAXWIDTH 	240
#define MAXSCNS 	4
#define	MAXLINES	500 
#define	OVER_AMT(a,b,c)	((a - b) / c)

#define	F_SUPP		0x0001
#define	OVERRIDE	0x0002
#define	B_ORDER		0x0004
#define	CANCEL		0x0008
#define	REDUCE		0x0010
#define	FORCE_BO	0x0020

#define	HEADER_SCN		1
#define	ITEM_SCN		2
#define	FREIGHT_SCN		3
#define	MISC_SCN		4

#define	TXT_REQD
#define	MAX_SONS		10
#include <pslscr.h>
#include <getnum.h>
#include <twodec.h>
#include <fcntl.h>
#include <hot_keys.h>
#include <get_lpno.h>
#include <ml_so_mess.h>
#include <ml_ol_mess.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <CustomerService.h>
#include <tabdisp.h>

#ifdef GVISION
	#include <RemotePipe.h>
	#include <RemoteFile.h>
	#define popen Remote_popen
	#define fprintf Remote_fprintf
	#define pclose Remote_pclose
#endif

#ifdef EXPORT
#undef EXPORT
#endif
#define	EXPORT		(sohr_rec.ord_type [0] == 'E')
#define DESPATCHED	(sohr_rec.status [0] == 'D')

#define	SERIAL		(SR.hhwhHash > 0L)
#define	SR			store [line_cnt]
#define	LSR			store [lcount [ITEM_SCN]]
#define	BONUS		(SR.bonusFlag [0] == 'Y')
#define	NON_STOCK	(SR.itemClass [0] == 'N')
#define	COMM_LINE	(SR.itemClass [0] == 'Z')
#define	KIT_ITEM	(SR.itemClass [0] == 'K' && prog_status == ENTRY)
#define	PHANTOM		(SR.itemClass [0] == 'P')
#define	INDENT		(SR.indent == TRUE)
#define	BO_OK		((SR.backorderFlag [0] == 'Y' || SR.backorderFlag [0] == 'F') && \
			    		cumr_rec.bo_flag [0] == 'Y')
#define	MANF_ITEM	(SR.source [0] == 'M')

#define	FULL_BO		(SR.backorderFlag [0] == 'F' && cumr_rec.bo_flag [0] == 'Y')
#define	MULT_QTY	(SR.costingFlag [0] != 'S')
#define	WO_LINE		(soln_rec.status [0] == 'W' && envVar.soWoAllowed)
#define	SPLIT_LINE	(local_rec.qty_ord > 0.00 && local_rec.qty_back > 0.00 && \
					 newOrder && envVar.soSplitBo)

#define	INPUT		0
#define	MAINT		1

#define	MARG_HOLD	(envVar.salesOrderMargin [1] == '1')
#define	MARG_MESS1	(envVar.salesOrderMargin [0] == '0')
#define	MARG_MESS2	(envVar.salesOrderMargin [0] == '1')

#define	TWO_STEP	(createStatusFlag [0] == 'M')
#define	ONE_STEP	(createStatusFlag [0] == 'R')
#define	STANDARD	(createStatusFlag [0] == 'S')

#define	ORD_DISPLAY	(programType == DISPLAY)
#define	ORD_MAINT	(programType == MAINT)
#define	ORD_INPUT	(programType == INPUT)

#define NO_KEY(x)	(vars [x].required == NA || \
			  		(vars [x].required == NI && prog_status == ENTRY) || \
			  	  	 vars [x].required == ND)

#define HIDE(x)		(vars [x].required == ND)
#define NEED(x)		(vars [x].required == YES)

#define	VALID_MAINT(x)	(x [0] == 'F' || x [0] == 'B' || \
                         x [0] == 'M' || x [0] == 'H' || \
                         x [0] == 'O' || x [0] == 'C' || \
						 x [0] == 'G')

#define	SOLN_HELD	(soln_rec.status [0] == 'H' || soln_rec.status [0] == 'O')

#define	MARGIN_OK	(strcmp (envVar.salesOrderMargin, "00"))

#define	FREIGHT_CHG	(sohr_rec.frei_req [0] == 'Y' && envVar.automaticFreight)

#define	FGN_CURR	(envVar.dbMcurr && strcmp (cumr_rec.curr_code, envVar.currencyCode))

	int		namesPipeFileName,
			newOrder			= TRUE,
			programType			= INPUT,
			inputDescr 			= 0,
			noSalesTax 			= 0,
			heldOrder 			= FALSE,
			overMargin 			= FALSE,
			passedPrinterNo 	= 0,
			selectedPrinterNo 	= 0,
			firstInput 			= 1,
			inScreenFlag 		= FALSE,
			specialDisplayMode 	= FALSE,
			discountFieldMode,
			saveRequiredFields [100],
			newNumberPlate		=	TRUE,
			nextSoicRef 		= 0,
			LotSelectFlag,
			PrevKit				= FALSE;

	char	mlSoInput [20][101];
	char	*worksOrderProgram	=	"so_wogen";

	float	highestDiscountPc 	= 0.00,
			lowestDiscountPc 	= 0.00,
			itemTotal			= 0.00;

	double	inv_tot 	= 0.00,
			dis_tot 	= 0.00,
			tax_tot 	= 0.00,
			gst_tot 	= 0.00,
			tot_tot 	= 0.00,
			lev_tot 	= 0.00,
			t_total 	= 0.00,
			l_total 	= 0.00,
			l_disc  	= 0.00,
			l_levy  	= 0.00,
			l_gst   	= 0.00,
			l_tax   	= 0.00,		
			lineLevyAmt = 0.00,
			lineLevyPc	= 0.00,
			est_freight	= 0.00;

	long	alternateHhccHash;

	char	alternateBranchWarehouseNo [5],
			readLockFlag [2],
			createStatusFlag [2],
			soBonus [3],
			soHide [3],
			*currentUserName,
			headerDesc [MAX_SONS + 1][41];

	struct	storeRec {

		long 	hhbrHash;			/* inmr_hhbr_hash			*/
		long 	hhsiHash;			/* inmr_hhsi_hash			*/
		long 	hhccHash;			/* ccmr_hhcc_hash			*/
		long 	hhwhHash;			/* > 0L if serial item		*/
		long 	actHhwhHash;		/* Actual hhwhHash.     	*/
		long 	hhslHash;			/* > 0L if existing soln.	*/
		long 	hhumHash;			/* inum_hhum_hash 			*/
		long	origHhbr;			/* Original hhbr hash 		*/
		float 	qtyAvailable;		/* incc_closing_stock		*/
		float 	qtyTotal;			/* total on order for line	*/
		float 	qtyOrigional;		/* Origional quantity order*/
		float 	qtyOrder;			/* qty suppliable for line	*/
		float	defaultDisc;		/* inmr_disc_pc				*/
		float	discPc;				/*              			*/
		float	calcDisc;			/* calculated discount PC.  */
		float	taxPc;				/* inmr_tax_pc  			*/
		float	gstPc;				/* inmr_gst_pc  			*/
		float	weight;				/* inmr_weight				*/
		float	outerSize;			/* inmr_outer_size			*/
		float	minMargin;			/* Min margin for category. */
		float	regPc;				/* Regulatory percent.      */
		float	discA;				/* Discount percent A.      */
		float	discB;				/* Discount percent A.      */
		float	discC;				/* Discount percent A.      */
		float	cnvFct;
		float	stdCnvFct;
		float	exchRate;			/* Exch rate for Category.  */
		float	origOrderQty;		/* Original order Qty 		*/
		double	marginCost;			/* Cost price for Margins.	*/
		double	taxAmt;				/* inmr_tax_amt 			*/
		double	advertLevyAmt;		/* advert levy amt for line	*/
		double	advertLevyPc;		/* advert levy pc for line	*/
		double	itemLevy;			/* item levy				*/
		double	costPrice;			/* cost price				*/
		double	gSalePrice;			/* gross sale price			*/
		double	salePrice;			/* default sale price		*/
		double	calcSalePrice;		/* calculated sale price	*/
		double	actSalePrice;		/* default sale price		*/
		char	nsDesc [MAX_SONS + 1][41]; /* Non stock description.    */
		char	_desc2 [41];
		char	serialNo [26];
		char	orgSerialNo [26];	/* serial number for line	*/
		char	category [12];		/* inmr_category			*/
		char	sellGroup [7];		/* inmr_sellgrp				*/
		char	bonusFlag [2];		/* Y iff bonus item			*/
		char	itemClass [2];		/* inmr_class				*/
		char	backorderFlag [2];	/* inmr_bo_flag				*/
		char	releaseFlag [2];	/* inmr_bo_release			*/
		char	packSize [6];		/* inmr_pack_size			*/
		char	costingFlag [2];	/* inmr_costing_flag		*/
		char	priceOR [2];		/* soln_pri_or				*/
		char	discOR [2];			/* soln_dis_or				*/
		char	marginFailed [2];	/* Margin failed. 			*/
		char	lotControlled [2];	/* Lot Control.  	        */
		char	uom [5];			/* UOM.						*/
		char	uomGroup [21];		/* UOM Group				*/
		char	status [2];			/* Current Status.			*/
		char	source [3];			/* Current product source.	*/
		int		contractPrice;		/* Contract price.			*/
		int		indent;				/* Indent					*/
		int		canEdit;			/* Can this line be edited. */
		int		contractStatus;		/* 0 = not contract line	*/
									/* 1 = contract no disc		*/
									/* 2 = contract disc ok    	*/
		int		pricingCheck;		/* Set if pricing has been  */
		int		cumulative;			/* Cumulative 1 or 0 		*/
		int		commitRef;			/* soic Committed reference	*/
		int		deCommitRef;		/* soic DeCommitted reference	*/
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

	char	defaultBranchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct cudpRecord	cudp_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct pocrRecord	pocrRec;
struct esmrRecord	esmr_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exsiRecord	exsi_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct insfRecord	insf_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	sohr2_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln2_rec;
struct soicRecord	soic_rec;
struct soicRecord	soic2_rec;
struct sknhRecord	sknh_rec;
struct skndRecord	sknd_rec;
struct skniRecord	skni_rec;
struct cuccRecord	cucc_rec;
struct cushRecord	cush_rec;
struct inalRecord	inal_rec;
struct cudiRecord	cudi_rec;
struct trshRecord	trsh_rec;
struct trzmRecord	trzm_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;
struct sonsRecord	sons_rec;
struct soktRecord	sokt_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	char 	*cumr2 	= "cumr2",
			*data 	= "data",
			*inum2 	= "inum2",
			*soic2 	= "soic2",
			*sohr2 	= "sohr2",
			*soln2	= "soln2",
			*serialSpace = "                         ",
			*ns_space = "                                        ",
			*sixteen_space = "                ",
			*twenty_space = "                    ";

	FILE	*pout;
	FILE	*worksOrderOutputFile;

	int		wpipe_open = FALSE;
	int		exit_alt;

	char	*scn_desc [] = {
				"HEADER SCREEN.",
				"ITEM SCREEN.",
				"FREIGHT DETAILS SCREEN.",
				"MISCELLANEOUS DETAIL SCREEN."
			};

	int		dueDateReqd;
	long	progPid;

#define		NBOX_TOP	18
#define		NBOX_LEFT	0

#include <cus_price.h>
#include <cus_disc.h>
#include <neg_win.h>
#include <ItemLevy.h>

struct	WH_LIST	{
	char	br_no [3];
	char	wh_no [3];
	char	wh_name [41];
	long	hhccHash;
	char	orderNo [9];
	long	hhsoHash;
	char	ordered_from [2];
	int		whPrinterNo;
	struct	WH_LIST *next;
};
#define	WH_NULL	((struct WH_LIST *) NULL)

struct	WH_LIST	*wh_head = WH_NULL;
struct	WH_LIST	*wh_tail = WH_NULL;

/*
 * The structure envVar groups the values of  environment settings together. 
 */
struct tagEnvVar
{
	char	currencyCode [4];
	char	gstCode [4];
	char	other [3][31];
	char	overrideQuantity [2];
	char	priceTypeComment [25];
	char	salesOrderMargin [3];
	char	soPrintProgram [15];
	char	soSpecial [5];
	int		addressPrompt;
	int		advertLevy;
	int		automaticFreight;
	int		combineInvoicePack;
	int		creditOver;
	int		creditStop;
	int		creditTerms;
	int		dbCo;
	int		dbFind;
	int		dbMcurr;
	int		dbNettUsed;
	int		discountIndents;
	int		fullSupplyOrder;
	int		gstApplies;
	int		includeForwardStock;
	int		lostSales;
	int		numberPrices;
	int		perminantWindow;
	int		qCApplies;
	int		qCAvailable;
	int		reverseDiscount;
	int		salesOrderPrint;
	int		salesOrderSales;
	int		serialItemsOk;
	int		skGrinNoPlate;
	int		soFreightCharge;
	int		soWoAllowed;
	int		soSplitBo;
	int		supplyFromAlternate;
	int		threePl;
	int		useSystemDate;
	int		windowPopupOk;

} envVar;

/*
 * Local & Screen Structures. 
 */
struct {
	/*
	 * Screen 1. 
	 */
	char	currCode [6];
	char 	customerNo [7];
	char 	headOfficeCustomer [7];
	char	cont_desc [31];
	char	schedOrder [2];
	char	ord_desc [10];
	char	ordFullDesc [10];
	char	priceDesc [16];
	char	priceFullDesc [16];
	float	discOverride;
	double	inputTotal;
	char	chargeToCustomer [7];
	char	chargeToName [36];

	/*
	 * Screen 2.
	 */
	char	item_no [17];
	int		line_no;
	char	UOM [5];
	float	qty_ord;
	float	qty_back;
	float	unitQty;
	char	serialNo [26];
	char	deliveryNo [6];
	long	hhccHash;
	char	sup_br [3];
	char	sup_wh [3];
	long	hhwhHash;

	/*
	 * Screen 3.
	 */
	char	spinst [3][61];
	char	defaultDelZone [7];
	char	origDelRequired [2];

	/*
	 * Screen 4.
	 */
	char	sell_desc [31];

	/*
	 * Miscellaneous.
	 */
	char	dummy [11];
	char	defaultOrderType [2];
	char	dbtDate [11];
	long	previousPack;
	char	prev_dbt_no [7];
	char	systemDate [11];
	long	lsystemDate;
	char	defaultDate [11];
	char	pri_type [2];
	char	supplierPart [17];
	char	orderNo [9];
	char	LL [2];
	char	numberPlate [sizeof	sknh_rec.plate_no];
} local_rec;

static	struct	var	vars [] = 
{
	{HEADER_SCN, LIN, "debtor",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",

		" ", "0", "Customer No         : ", "Enter Customer Number, Full Search Available. ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{HEADER_SCN, LIN, "currCode",	 4, 35, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.currCode},
	{HEADER_SCN, LIN, "name",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{HEADER_SCN, LIN, "dp_no",	 5, 2, CHARTYPE,
		"AA", "          ",
		" ", cumr_rec.department, "Department No       : ", " ",
		NI, NO, JUSTRIGHT, "", "", sohr_rec.dp_no},
	{HEADER_SCN, LIN, "dp_name",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cudp_rec.dp_short},
	{HEADER_SCN, LIN, "cust_type",	 5, 66, CHARTYPE,
		"AAA", "          ",
		" ", "", "Cust Type           : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.class_type},
	{HEADER_SCN, LIN, "headOfficeAccount",	 5, 96, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Head Office Account : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.headOfficeCustomer},
	{HEADER_SCN, LIN, "orderNo",	 6, 2, CHARTYPE,
		"UUUUUUUU", "        ",
		"0", "0", "Sales Order No      : ", " ",
		 NE, NO, JUSTRIGHT, "", "", sohr_rec.order_no},
	{HEADER_SCN, LIN, "cus_ord_ref",	 6, 66, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Customer Order Ref  : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cus_ord_ref},
	{HEADER_SCN, LIN, "cont_no",	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract            : ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 NE, NO,  JUSTLEFT, "", "", sohr_rec.cont_no},
	{HEADER_SCN, LIN, "cont_desc",	7, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cont_desc},
	{1, LIN, "numberPlate",	 7, 66, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Number Plate No.    : ", "Enter number plate / goods receipt number.",
		YES, NO,  JUSTLEFT, "", "", local_rec.numberPlate},
	{HEADER_SCN, LIN, "chargeToCustomer", 7, 66, CHARTYPE,
		"UUUUUU", "          ",

		" ", "0", "Charge To Customer  : ", "Enter Charge to Customer Number, [SEARCH]. ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.chargeToCustomer},
	{HEADER_SCN, LIN, "chargeToName",	 7, 96, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.chargeToName},
	{HEADER_SCN, LIN, "chargeToHash", 7, 66, LONGTYPE,
		"NNNNNNNNNN", "          ",

		" ", "0", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&sohr_rec.chg_hhcu_hash},
	{HEADER_SCN, LIN, "contact_name",	 9, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.contact_name, "Contact Name        : ", "Enter contact name - Default is off customer master file. ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cont_name},
	{HEADER_SCN, LIN, "contact_phone",	 9, 66, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.phone_no, "Contact Phone       : ", "Enter contact phone - Default is off customer master file. ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cont_phone},
	{HEADER_SCN, LIN, "cus_addr1",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Charge Address      : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr1},
	{HEADER_SCN, LIN, "cus_addr2",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr2},
	{HEADER_SCN, LIN, "cus_addr3",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr3},
	{HEADER_SCN, LIN, "cus_addr4",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr4},
	{HEADER_SCN, LIN, "del_name",	 10, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Name       : ", " Select Delivery Name and Address. Search available. ",
		 NO, NO,  JUSTLEFT, "", "", sohr_rec.del_name},
	{HEADER_SCN, LIN, "del_addr1",	11, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address    : ", " ",
		 NO, NO,  JUSTLEFT, "", "", sohr_rec.del_add1},
	{HEADER_SCN, LIN, "del_addr2",	12, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NO, NO,  JUSTLEFT, "", "", sohr_rec.del_add2},
	{HEADER_SCN, LIN, "del_addr3",	13, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NO, NO,  JUSTLEFT, "", "", sohr_rec.del_add3},
	{HEADER_SCN, LIN, "dt_raised",	15, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.defaultDate, "Order date          : ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&sohr_rec.dt_raised},
	{HEADER_SCN, LIN, "dt_required",	16, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Required date       : ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&sohr_rec.dt_required},
	{HEADER_SCN, LIN, "freightRequiredFlag",	 15, 66, CHARTYPE,
		"U", "          ",
		" ", cumr_rec.freight_chg, "Freight Required.   : ", "Enter Y(es) if freight required Default = Customer master file default. ",
		YES, NO, JUSTRIGHT, "YN", "", sohr_rec.frei_req},
	{HEADER_SCN, LIN, "schedOrder",	16, 66, CHARTYPE,
		"U", "          ",
		" ", "N", "Scheduled Order     : ", " ",
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.schedOrder},
	{HEADER_SCN, LIN, "fix_exch",	16, 66, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exchange Rate : ", " ",
		ND, NO,  JUSTLEFT, "YN", "", sohr_rec.fix_exch},
	{HEADER_SCN, LIN, "tax_code",	17, 2, CHARTYPE,
		"U", "        ",
		" ", cumr_rec.tax_code, "Tax Code            : ", " ",
		ND, NO,  JUSTLEFT, "ABCD", "", sohr_rec.tax_code},
	{HEADER_SCN, LIN, "tax_no",	17, 66, CHARTYPE,
		"AAAAAAAAAAAAAAA", "        ",
		" ", cumr_rec.tax_no, "Tax Number          : ", " ",
		ND, NO,  JUSTLEFT, "", "", sohr_rec.tax_no},
	{HEADER_SCN, LIN, "ord_type",	18, 2, CHARTYPE,
		"U", "          ",
		" ", local_rec.defaultOrderType, "Order Type          : ", " D (omestic  E (xport ",
		YES, NO,  JUSTLEFT, "DE", "", local_rec.ord_desc},
	{HEADER_SCN, LIN, "ord_type_desc",	18, 27, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ordFullDesc},
	{HEADER_SCN, LIN, "pri_type",	19, 2, CHARTYPE,
		"N", "        ",
		" ", cumr_rec.price_type, "Price Type          : ", envVar.priceTypeComment,
		 NA, NO,  JUSTLEFT, "", "", local_rec.priceDesc},
	{HEADER_SCN, LIN, "pri_type_desc",	19, 27, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.priceFullDesc},
	{HEADER_SCN, LIN, "sman_code",	18, 66, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.sman_code, "Salesman            : ", " ",
		NI, NO, JUSTRIGHT, "", "", sohr_rec.sman_code},
	{HEADER_SCN, LIN, "sman_desc",	18, 91, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{HEADER_SCN, LIN, "area_code",	19, 66, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.area_code, "Area code           : ", " ",
		NI, NO, JUSTRIGHT, "", "", sohr_rec.area_code},
	{HEADER_SCN, LIN, "area_desc",	19, 91, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{HEADER_SCN, LIN, "discOverride",	20, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Disc O'ride         : ", " This Discount is Added to Each Invoice Line ",
		 ND, NO, JUSTRIGHT, "-99.99", "99.99", (char *)&local_rec.discOverride},
	{HEADER_SCN, LIN, "prt_price",	20, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Price Details       : ", "Y(es) print prices on invoice, N(o) Don't print prices on Invoice.",
		 YES, NO,  JUSTLEFT, "NY", "", sohr_rec.prt_price},
	{HEADER_SCN, LIN, "inputTotal",	20, 66, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Inv Total           : ", " Total Value Override for Invoice ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.inputTotal},

	{ITEM_SCN, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{ITEM_SCN, TAB, "line_no",	0, 0, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Line", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.line_no},
	{ITEM_SCN, TAB, "hide",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "H", " ",
		 ND, NO,  JUSTLEFT, "", "", soln_rec.hide_flag},
	{ITEM_SCN, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  I t e m   D e s c r i p t i o n .   ", " ",
		NI, NO,  JUSTLEFT, "", "", soln_rec.item_desc},
	{ITEM_SCN, TAB, "sale_code",	 0, 1, CHARTYPE,
		"UU", "          ",
		" ", sohr_rec.sman_code, "Sale", " Salesman ",
		 ND, NO, JUSTRIGHT, "", "", soln_rec.sman_code},
	{ITEM_SCN, TAB, "unitQty",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Unit Qty ", " ",
		 ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.unitQty},
	{ITEM_SCN, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", " Unit of Measure ",
		 NO, NO, JUSTLEFT, "", "", local_rec.UOM},
	{ITEM_SCN, TAB, "ord_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", sohr_rec.cus_ord_ref, " Cust Order Ref. ", "Customer Order Ref.",
		 ND, NO,  JUSTLEFT, "", "", soln_rec.cus_ord_ref},
	{ITEM_SCN, TAB, "pack_size",	 0, 0, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Pack ", " Pack Size ",
		 ND, NO,  JUSTLEFT, "", "", soln_rec.pack_size},
	{ITEM_SCN, TAB, "qty_ord",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Qty Order", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.qty_ord},
	{ITEM_SCN, TAB, "qty_back",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Qty B/ord", " ",
		 NI, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.qty_back},
	{ITEM_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM_SCN, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Cst Price", " Cost Price ",
		 ND, NO, JUSTRIGHT, "-99999.99", "999999.99", (char *)&soln_rec.cost_price},
	{ITEM_SCN, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0.00", " Sale Price.", " Sale Price ",
		YES, NO, JUSTRIGHT, "-99999999.99", "999999999.99", (char *)&soln_rec.sale_price},
	{ITEM_SCN, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "0.00", " Disc  ", " ",
		 NO, NO, JUSTRIGHT, "-999.99", "999.99", (char *)&soln_rec.dis_pc},
	{ITEM_SCN, TAB, "due_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " Due Date ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&soln_rec.due_date},
	{ITEM_SCN, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Serial Number      ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.serialNo},
	{ITEM_SCN, TAB, "deliveryNo",	 0, 0, CHARTYPE,
		"AAAAA", "          ",
		" ", " ", "DelNo", " Enter Delivery Address Number For This Line ",
		 NO, NO, JUSTRIGHT, "0", "32000", local_rec.deliveryNo},
	{ITEM_SCN, TAB, "skndHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&sknd_rec.sknd_hash},
	{ITEM_SCN, TAB, "hhccHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhccHash},
	{ITEM_SCN, TAB, "sup_br",	 0, 0, CHARTYPE,
		"UU", "          ",
		" ", " ", "BR", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sup_br},
	{ITEM_SCN, TAB, "sup_wh",	MAXLINES, 0, CHARTYPE,
		"UU", "          ",
		" ", " ", "WH", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sup_wh},
	{ITEM_SCN, TAB, "hhwhHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhwhHash},
	{FREIGHT_SCN, LIN, "carrierCode",	 4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Carrier Code.       : ", "Enter carrier code, [SEARCH] available.",
		YES, NO,  JUSTLEFT, "", "", trcm_rec.carr_code},
	{FREIGHT_SCN, LIN, "carr_desc",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Carrier Description : ", " ",
		 NA, NO,  JUSTLEFT, "", "", trcm_rec.carr_desc},
	{FREIGHT_SCN, LIN, "deliveryZoneCode",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultDelZone, "Delivery Zone       : ", "Enter Delivery Zone Code [SEARCH]. ",
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{FREIGHT_SCN, LIN, "deliveryZoneDesc",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Zone Desc  : ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{FREIGHT_SCN, LIN, "deliveryRequired",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Delivery Reqd. (Y/N): ", "Enter Y(es) for Delivery. <default = N(o)> ",
		 YES, NO, JUSTLEFT, "YN", "", sohr_rec.del_req},
	{FREIGHT_SCN, LIN, "deliveryDate",	6, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.defaultDate, "Delivery Date       : ", " ",
		NA, NO,  JUSTLEFT, " ", "", (char *)&sohr_rec.del_date},
	{FREIGHT_SCN, LIN, "cons_no",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Consignment no.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cons_no},
	{FREIGHT_SCN, LIN, "no_cartons",	 7, 66, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Number Cartons.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&sohr_rec.no_cartons},
	{FREIGHT_SCN, LIN, "est_freight",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Est Freight         : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&est_freight},
	{FREIGHT_SCN, LIN, "tot_kg",	 9, 66, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Total Kgs.          : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.no_kgs},
	{FREIGHT_SCN, LIN, "freight",	 10, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.     : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.freight},
	{FREIGHT_SCN, LIN, "shipname",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship to name        : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_name},
	{FREIGHT_SCN, LIN, "shipaddr1",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship to address 1   : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add1},
	{FREIGHT_SCN, LIN, "shipaddr2",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship to address 2   : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add2},
	{FREIGHT_SCN, LIN, "shipaddr3",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship to address 3   : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add3},
	{FREIGHT_SCN, LIN, "ship_method",	17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipment method     : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{FREIGHT_SCN, LIN, "spcode1",	18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 1       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{FREIGHT_SCN, LIN, "spcode2",	19, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 2       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{MISC_SCN, LIN, "sos_ok",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "S.O Surcharge (Y/N) : ", "Enter N(o) to Overide Small Order Surcharge.",
		YES, NO, JUSTRIGHT, "YN", "", sohr_rec.sohr_new},
	{MISC_SCN, LIN, "fullSupplyOrder",	 4, 66, CHARTYPE,
		"U", "          ",
		" ", "Y", "Full Supply Order. (Y/N) : ", "Enter Y(es) for full sypply Order.",
		YES, NO, JUSTRIGHT, "YN", "", sohr_rec.full_supply},
	{MISC_SCN, LIN, "pay_term",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.      : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.pay_term},
	{MISC_SCN, LIN, "sell_terms",	 7, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Sell Terms.         : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.sell_terms},
	{MISC_SCN, LIN, "sell_desc",	8 , 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Sell Description.   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sell_desc},
	{MISC_SCN, LIN, "insurance",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Insurance.          : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.insurance},
	{MISC_SCN, LIN, "ins_det",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Insurance Details   : ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.ins_det},
	{MISC_SCN, LIN, "deposit",	12, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Deposit.            : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.deposit},
	{MISC_SCN, LIN, "discount",	13, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Special Discount.   : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.discount},
	{MISC_SCN, LIN, "other_1",	15, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [0], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.other_cost_1},
	{MISC_SCN, LIN, "other_2",	16, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [1], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.other_cost_2},
	{MISC_SCN, LIN, "other_3",	17, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [2], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&sohr_rec.other_cost_3},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

static	int	BrDispFunc 	(int, KEY_TAB *);
static	int	WhDispFunc 	(int, KEY_TAB *);
static	int	SelectFunc 	(int, KEY_TAB *);
static	int	ExitFunc 	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	wh_keys [] =
{
    { " BRANCH STATUS ",    'B', BrDispFunc,
	" Display Branch Status For Current Item",			"A" },
    { " STOCK STATUS ",     'S', WhDispFunc,
	" Display Stock Status ",					"A" },
    { NULL,		    '\r', SelectFunc,
	" Select warehouse as alternate stocking point ",		"A" },
    { NULL,		    FN1,  ExitFunc,
	" ",								"A" },
    { NULL,		   FN16, ExitFunc,
	" Exit Selection Of Alternate Stocking Point ",			"A" },
    END_KEYS
};
#else
static	KEY_TAB	wh_keys [] =
{
    { " [B]RANCH STATUS",    'B', BrDispFunc,
	" Display Branch Status For Current Item",			"A" },
    { " [S]TOCK STATUS",     'S', WhDispFunc,
	" Display Stock Status ",					"A" },
    { NULL,		    '\r', SelectFunc,
	" Select warehouse as alternate stocking point ",		"A" },
    { NULL,		    FN1,  ExitFunc,
	" ",								"A" },
    { NULL,		   FN16, ExitFunc,
	" Exit Selection Of Alternate Stocking Point ",			"A" },
    END_KEYS
};
#endif

#include 	<CheckIndent.h>
#include 	<p_terms.h>
#include 	<proc_sobg.h>
#include 	<LocHeader.h>
#include    <FindCumr.h>

#ifdef GVISION
#include <StockWindow.h>
#endif

/*
 * Function Declarations
 */
const char	*GetPriceDesc 		(int);
double	OrderValue				(void);
float	CalculateOverideDisc	(double);
float	ProcessPhantom			(long,long);
float	ReCalcAvail				(void);
float	RealTimeCommitted		(long,long,int);
float	ScreenDisc				(float);
float	ToLclUom				(float);
float	ToStdUom				(float);
float	TotalItemQty 			(long, int, float);
int		AddIncc					(long,long);
int		AddInei					(void);
int		AlternateInputResponce	(float);
int		CheckBonusItem			(char *);
int		CheckContract			(void);
int		CheckCudi				(void);
int		CheckCumr				(double);
int		CheckDuplicateSerial	(char *,long,int);
int		CheckHidden				(char *);
int		CheckSohr				(long,char *);
int		DeleteLine				(int);
int		FindCucc				(int,long);
int		GenSoicRef				(void);
int		InputResponce			(void);
int		LoadDisplay				(char *);
int		LoadItemScreen			(long);
int		MarginCheckOk			(double,float,double,float,int);
int		OpenStockWindow			(void);
int		ProcessAltSupWh			(void);
int		SrchCudi 				(int);
int		Update					(void);
int		UpdateInsf				(long,long,char *,char *,char *);
int		ValidateItemNumber		(int);
int		WarnUser				(char *,int);
int		heading					(int);
int		spec_valid				(int);
int		use_window				(int);
int		win_function			(int,int,int,int);
int		win_function2			(int,int,int,int);
int		win_function3			(int,int,int,int);
int		LoadNumberPlateLines 	(long);
struct	WH_LIST	*wh_alloc		(void);
void	AddNewSoic				(int);
void	BusyFunction			(int);
void	CalcExtendedTotal		(int);
void	CalcInputTotal			(void);
void	CalculateFreight		(float,double,double,double);
void	CalculateTotalBox		(int,int);
void	CheckEnvironment		(void);
void	ClearSoic				(void);
void	ClearWindow				(void);
void	CloseDB					(void);
void	CloseTransportFiles		(void);
void	CreateWorksOrder 		(long);
void	DeleteSONS				(int,long);
void	DiscProcess				(int);
void	DispSONS				(int);
void	DrawTotals				(void);
void	InitML					(void);
void	InitSTerms				(void);
void	InitWarehouse			(void);
void	InputSONS				(int,int);
void	LoadSONS				(int,int,long);
void	LoadWarehouseInfo		(void);
void	LoadSknh				(void);
void	LogLostSales			(float);
void	OpenDB					(void);
void	OpenTransportFiles		(char *);
void	PriceProcess			(int);
void	PrintBoxTotals			(void);
void	PrintCompanyDetails		(void);
void	ProcSoic				(int,int);
void	ProcessKitItem			(long,float);
void	ReadMisc				(void);
void	SetOrderDefaults		(int);
void	SetSTerm				(int,char *,char *);
void	SetSTermCode			(int,char *);
void	SrchCnch				(char *);
void	SrchCudp				(char *);
void	SrchExaf				(char *);
void	SrchExsf				(char *);
void	SrchExsi				(char *);
void	SrchInsf				(char *,int);
void	SrchInum				(char *);
void	SrchPay					(void);
void	SrchPrice				(void);
void	SrchSell				(void);
void	SrchSohr				(char *);
void	SrchSknh 		 		(char *);
void	SrchTrcm				(char *);
void	SrchTrzm				(char *);
void	TidyAlternateScreen		(void);
void	TidySonsScreen			(void);
void	UpdSoicQty				(void);
void	UpdateSONS				(int,int,long);
void	proc_cusa				(int);
void	set_scn2_vars			(int);
void	shutdown_prog			(void);
void	tab_other				(int);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char	*argv [])
{
	int		i;
	int		cnt;
	int		field;
	char	*sptr;

	currentUserName = getenv ("LOGNAME");
	SETUP_SCR (vars);


	progPid = (long) getpid ();

	TruePosition	=	TRUE;

	Y_EALL = 3;
	X_EALL = 0;

	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();

	_win_func = TRUE;

	if (argc < 3)
	{
		/*
		 * Usage : %s <createStatusFlag> <screen_file> - optional <lpno>
		 *	<createStatusFlag>  - R (elease Order Automatically
		 *		            	- M (anual Release of Order
		 *            			- L (ate or Delay Release of Order
		 *		            	- S (tandard Order - No Release
		 * <screen_file>
		 */
		print_at (0,0, mlSoMess745,argv [0]);
		print_at (1,0, mlSoMess746);
		print_at (2,0, mlSoMess747);
		print_at (3,0, mlSoMess748);
		print_at (4,0, mlSoMess749);
		print_at (5,0, mlSoMess750);
		return (EXIT_FAILURE);
	}

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "so_input"))
		programType = INPUT;
	else
	{
		if (!strcmp (sptr, "so_display") || !strcmp (sptr, "so_sdisplay"))
		{
			if (!strcmp (sptr, "so_sdisplay"))
				specialDisplayMode = TRUE;
			else
				specialDisplayMode = FALSE;

			programType = DISPLAY;
		}
		else
			programType = MAINT;
	}
	if (!ORD_INPUT)
		envVar.supplyFromAlternate = FALSE;

	argv [1][0] = toupper (argv [1][0]);

	sprintf (createStatusFlag,"%-1.1s",argv [1]);


	passedPrinterNo = (argc == 4) ? atoi (argv [3]) : 0;

	strcpy (local_rec.defaultOrderType,"D");

	/*
	 * Read common terminal record. 
	 */
	ReadMisc ();

	OpenDB ();

	if (specialDisplayMode && argc < 5)
		specialDisplayMode = FALSE;

	strcpy (readLockFlag, (ORD_DISPLAY) ? "r" : "u");


	if (STANDARD)
	{
		vars [label ("orderNo")].fill = " ";
		vars [label ("orderNo")].just = JUSTLEFT;
	}

	init_scr ();
	_set_masks (argv [2]);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM_SCN, store, sizeof (struct storeRec));
#endif


	inputDescr	=	FLD ("descr");

	if (envVar.gstApplies)
	{
		FLD ("tax_code") = ND;
		FLD ("tax_no")   = ND;
	}
	if (envVar.serialItemsOk)
	{
		FLD ("ser_no") 		= YES;
		FLD ("due_date")	= ND;
		FLD ("UOM") 		= ND;
		FLD ("hide") 		= ND;
		FLD ("line_no")		= ND;
		FLD ("schedOrder") 	= ND;
	}
	else
		FLD ("ser_no") 	= ND;

	dueDateReqd = FLD ("due_date");

	FLD ("numberPlate")	= (envVar.skGrinNoPlate) ? YES : ND;
	if (envVar.skGrinNoPlate)
	{
		FLD ("chargeToCustomer")	=	ND;
		FLD ("chargeToName")		=	ND;
	}
	if (envVar.threePl)
	{
		FLD ("unitQty")		=	NA;
		FLD ("due_date")	=	ND;
	}
	if (ORD_DISPLAY)
	{
		for (field = label ("cus_ord_ref"); FIELD.scn != 0; field++)
			if (FIELD.required != ND)
				FIELD.required = NA;

		FLD ("dp_no")    = NA;
		FLD ("due_date") = NA;
	}

	if (ORD_MAINT && !HIDE (label ("schedOrder")))
		FLD ("schedOrder") = NA;

	/*
	 * Update to No Input if fields has been set to 
	 * input as prog is will not work this way.    
	 */
	if (FLD ("inputTotal") == YES || FLD ("inputTotal") == NO)
		FLD ("inputTotal") = NI;

	/*
	 * Delivery addresses at line level are currently
	 * only available for scheduled orders.         
	 */
	if (FLD ("schedOrder") == ND)
		FLD ("deliveryNo") = ND;

	discountFieldMode = FLD ("disc");

	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];


	strcpy (defaultBranchNo, (envVar.dbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	strcpy (local_rec.dbtDate, DateToString (comm_rec.dbt_date));

	if (envVar.useSystemDate)
		strcpy (local_rec.defaultDate,	local_rec.systemDate);
	else
		strcpy (local_rec.defaultDate,	local_rec.dbtDate);

	local_rec.lsystemDate = TodaysDate ();

	swide ();
	clear ();

	/*
	 * Open main database files. 
	 */
	print_at (0,0, "%s\n", ML (mlStdMess035));
	fflush (stdout);

	InitML ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = (ORD_DISPLAY) ? NA : YES;

	OpenPrice ();
	OpenDisc ();

	local_rec.previousPack = 0L;
	strcpy (local_rec.prev_dbt_no,"000000");

	/*
	 * Store the current value of ALL screen 2 fields for later
	 */
	set_scn2_vars (0);

	orig_disc = FLD ("disc");
	orig_sale = FLD ("sale_price");

	while (prog_exit == 0)
	{
		if (restart) 
		{
			if (SK_BATCH_CONT || MULT_LOC)
				AllocationRestore ();

			/*
			 * first time through &				
			 * input version of program.		
			 * Not set to ignore by environment &
			 */
			if (firstInput && ORD_INPUT && !NO_KEY (label ("ord_type")))
			{
				strcpy (local_rec.defaultOrderType,"D");
				FLD ("ord_type") = YES;
			}
			abc_unlock (sohr);
			abc_unlock (soln);

			/*
			 * Free any outstanding insf_records 
			 */		 
			if (line_cnt > lcount [ITEM_SCN])
				lcount [ITEM_SCN] = line_cnt;

			for (i = 0; i < lcount [ITEM_SCN]; i++)
			{
				if (strcmp (store [i].serialNo, serialSpace))
				{
					cc = UpdateInsf (store [i].hhwhHash,
									store [i].hhsiHash,
									store [i].serialNo,
									"C","F");

					if (cc && cc < 1000)
						file_err (cc, (char *)insf, "DBUPDATE");
				}
			}
			for (i = 0; i < lcount [ITEM_SCN]; i++)
			{
				if (strcmp (store [i].orgSerialNo, serialSpace))
				{
					cc = UpdateInsf (store [i].hhwhHash,
									store [i].hhsiHash,
									store [i].orgSerialNo,
									"F","C");

					if (cc && cc < 1000)
						file_err (cc, (char *)insf, "DBUPDATE");
				}
			}
			/*
			 * Clear soic records for this PID. 
			 */
			ClearSoic ();

		}
		for (cnt = 0; cnt < MAX_SONS; cnt++)
			sprintf (headerDesc [cnt], "%40.40s", " ");

		/*
		 *	Clear the backing store
		 */
		for (i = 0; i < MAXLINES; i++)
		{
			memset (store + i, 0, sizeof (struct storeRec));
			store [i].canEdit	 = TRUE;
			strcpy (store [i].status, " ");
			strcpy (store [i].serialNo, serialSpace);
			strcpy (store [i].orgSerialNo, serialSpace);
			strcpy (store [i]._desc2, ns_space);
			for (cnt = 0; cnt < MAX_SONS; cnt++)
				sprintf (store [i].nsDesc [cnt], "%40.40s", " ");
		}
		set_tty (); 
		noSalesTax = 0;
		cumr_rec.hhcu_hash = 0L;
		if (restart) 
		{
			/*
			 * first time through &					
			 * input version of program.		
			 * Not set to ignore by environment &
			 */
			if (firstInput && ORD_INPUT && !NO_KEY (label ("ord_type")))
			{
				strcpy (local_rec.defaultOrderType,"D");
				FLD ("ord_type") = YES;
			}
			abc_unlock (sohr);
			abc_unlock (soln);
		}

		est_freight			=	0.00;
		search_ok 			= 	1;
		newOrder 			= 	0;
		entry_exit 			= 	0;
		edit_exit 			= 	0;
		prog_exit 			= 	0;
		restart 			= 	0;
		init_ok 			= 	1;
		tab_row 			= 	8;
		lcount [ITEM_SCN] 	= 	0;
		dis_tot 			= 	0.00;
		inv_tot 			= 	0.00;
		tax_tot 			= 	0.00;
		tot_tot 			= 	0.00;
		gst_tot 			= 	0.00;
		lev_tot 			= 	0.00;
		highestDiscountPc	= 	0.00;
		lowestDiscountPc 	= 	0.00;
		trcm_rec.markup_pc 	= 	0.00;
		trcl_rec.cost_kg 	= 	0.00;
		memset (&cnch_rec, 0, sizeof (cnch_rec));

		if (!specialDisplayMode)
		{
			/*
			 * Enter screen 1 linear input. 
			 */
			heading (HEADER_SCN);
			entry (HEADER_SCN);
			
			if (prog_exit || restart)
			{
				abc_unlock (sohr);
				continue;
			}
		}
		else
		{
			if (LoadDisplay (argv [4]))
			{
				clear ();
				box (0, 0, 36, 1);
				/*
				 * SORRY UNABLE TO FIND TRANSACTION 
				 */
				print_at (1,1,"%R %s ", ML (mlSoMess280)); 
				fflush (stdout);
				sleep (sleepTime);
				shutdown_prog ();
                return (EXIT_FAILURE);
			}
			heading (HEADER_SCN);
			scn_display (HEADER_SCN);
		}
		if (newOrder && newNumberPlate)
		{
			line_cnt = -1;
			heading (ITEM_SCN);
			use_window (0);
			entry (ITEM_SCN);

			if (restart)
			{
				abc_unlock (sohr);
				continue;
			}
		}
		else
			scn_display (HEADER_SCN);

		prog_status = !(ENTRY);

		if (FREIGHT_CHG && ORD_INPUT && newOrder && newNumberPlate)
		{
			heading (FREIGHT_SCN);
			scn_display (FREIGHT_SCN);
			if (envVar.combineInvoicePack)
			{
				init_ok = FALSE;
				entry (FREIGHT_SCN);
				init_ok = TRUE;
			}
			else
				edit (FREIGHT_SCN);
		}
		
		edit_all ();
		
		if (specialDisplayMode)
			break;

		while (FREIGHT_CHG && sohr_rec.freight == 0.00 && 
				envVar.combineInvoicePack)
		{
			/*
			 * Freight required flag is Yes and no freight has been entered. 
			 */
			print_mess (ML (mlStdMess175));
			sleep (sleepTime);

			edit_all ();

			if (restart)
				break;
		}
	

		if (restart)
		{
			abc_unlock (sohr);
			continue;
		}

		if (!ORD_DISPLAY)
		{
			if (Update ())
				continue;
				
			if (SK_BATCH_CONT || MULT_LOC)
				AllocationComplete ();
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
InitML (void)
{
	int		i;

	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		strcpy (err_str, STerms [i]._sdesc);
		strcpy (STerms [i]._sdesc, strdup (err_str));
	}
	strcpy (mlSoInput [1], ML ("Domestic"));
	strcpy (mlSoInput [2], ML ("Export"));
	sprintf (mlSoInput [5], " *** %-2.2s ***", ML ("NO"));
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (
 void)
{
#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (pout);
		IP_CLOSE (namesPipeFileName);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

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

	abc_alias (inum2, inum);
	abc_alias (cumr2, cumr);
	abc_alias (soic2, soic);
	abc_alias (soln2, soln);
	abc_alias (sohr2, sohr);

	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncl,  cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (cudp,  cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (insf,  insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (soic,  soic_list, SOIC_NO_FIELDS, "soic_id_no2");
	open_rec (soic2, soic_list, SOIC_NO_FIELDS, "soic_id_no");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (soln2, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (sons,  sons_list, SONS_NO_FIELDS, "sons_id_no");

	if (envVar.advertLevy)
		open_rec (inal,inal_list,INAL_NO_FIELDS,"inal_id_no");

	if (envVar.skGrinNoPlate)
	{
		open_rec (sknh,  sknh_list, SKNH_NO_FIELDS, "sknh_id_no");
		open_rec (sknd,  sknd_list, SKND_NO_FIELDS, "sknd_id_no");
		open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_sknd_hash");
	}
	OpenLocation (ccmr_rec.hhcc_hash);

	if (llctInput [0] == 'V')
		LotSelectFlag	=	INP_VIEW;
	if (llctInput [0] == 'A')
		LotSelectFlag	=	INP_AUTO;
	if (llctInput [0] == 'M')
	{
		strcpy (StockTake, "Y");
		LotSelectFlag	=	INP_VIEW;
	}
	if (llctInput [0] == 'N')
	{
		SK_BATCH_CONT 	=	FALSE;
		MULT_LOC		=	FALSE;
		FLD ("LL") 		= 	ND;
	}
}
/*
 * Close Database Files. 
 */
void
CloseDB (
 void)
{
	abc_fclose (cnch);
	abc_fclose (cncl);
	abc_fclose (cucc);
	abc_fclose (cudp);
	abc_fclose (cuit);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (insf);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (pocr);
	abc_fclose (sohr);
	abc_fclose (soic);
	abc_fclose (soic2);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_fclose (sons);
	if (envVar.advertLevy)
		abc_fclose (inal);

	if (envVar.skGrinNoPlate)
	{
		abc_fclose (sknh);
		abc_fclose (sknd);
		abc_fclose (skni);
	}
	CloseLocation ();
	ClosePrice ();
	CloseDisc ();
 
	SearchFindClose ();
	abc_dbclose (data);
}

/*
 * Open database and misc control files. 
 */
void
ReadMisc (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	/*
	 * Load warehouses into linked list 
	 */
	LoadWarehouseInfo ();

	/*
	 * Load current warehouse 
	 */
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
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
	int		val_pterms;
	int		sup_frm_alt;
	double	total_owing = 0.00;
	int		TempLine;
	long	CheckDate = 0L;
	int		dmy [3];
	int		inv_mo;
	int		inv_yr;
	int		due_mo;
	int		due_yr;

	if (soln_rec.due_date == 0L || sohr_rec.dt_required == 0L)
		soln_rec.due_date = local_rec.lsystemDate;

	/*
	 * Validate Customer Number. 
	 */
	if (LCHECK ("debtor"))
	{
		if (dflt_used && ORD_DISPLAY)
		{
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,defaultBranchNo);
		strcpy (cumr_rec.dbt_no,zero_pad (local_rec.customerNo, 6));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * Customer not found.
			 */
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		heldOrder	= FALSE;
		overMargin 	= FALSE;
		/*
		 * Check if customer is on stop credit. 
		 */
		if (!ORD_DISPLAY && !STANDARD && cumr_rec.stop_credit [0] == 'Y')
		{
			if (envVar.creditStop)
			{
				/*
				 * Customer is on Stop Credit,	
				 * Cannot Process Any Orders.
				 */
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML ("Customer is on stop credit."),0);
				if (cc)
					return (cc);
				heldOrder = TRUE;
			}
		}

		total_owing = cumr_balance [0] + cumr_balance [1] +
					  cumr_balance [2] + cumr_balance [3] +
					  cumr_balance [4] + cumr_balance [5];

		/*
		 * Check if customer is over his credit limit.
		 */
		if (!ORD_DISPLAY && !STANDARD && cumr_rec.credit_limit <= total_owing && 
			 cumr_rec.credit_limit != 0.00 && cumr_rec.crd_flag [0] != 'Y')
		{
			if (envVar.creditOver)
			{
				/*
				 * Customer is Over Credit Limit, 
				 * Cannot Process Any Orders
				 */
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML ("Customer is over the credit limit."),0);
				if (cc)
					return (EXIT_FAILURE);
				heldOrder = TRUE;
			}
		}
		/*
		 * Check Credit Terms
		 */
		if (!ORD_DISPLAY && cumr_rec.od_flag && 
		      cumr_rec.crd_flag [0] != 'Y')
		{
			if (envVar.creditTerms)
			{
				/*
				 * Customer credit terms exceeded by %d period (s), 
				 * CANNOT PROCESS ORDER
				 */
				sprintf (err_str, ML (mlSoMess363),cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				/*
				 * WARNING Customer credit terms exceeded by %d period (s).
				 */
				sprintf (err_str, ML ("Customer credit terms exceeded by %d period (s)."),cumr_rec.od_flag);
				cc = WarnUser (err_str,0);
				if (cc)
					return (EXIT_FAILURE);

				heldOrder = TRUE;
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

		DSP_FLD ("cust_type");
		DSP_FLD ("headOfficeAccount");
		DSP_FLD ("name");
		DSP_FLD ("cus_addr1");
		DSP_FLD ("cus_addr2");
		DSP_FLD ("cus_addr3");
		DSP_FLD ("cus_addr4");

		/*
		 * Process delivery address
		 */
		sprintf (sohr_rec.del_name, "%-40.40s", cumr_rec.dbt_name);
		sprintf (sohr_rec.del_add1, "%-40.40s", cumr_rec.dl_adr1);
		sprintf (sohr_rec.del_add2, "%-40.40s", cumr_rec.dl_adr2);
		sprintf (sohr_rec.del_add3, "%-40.40s", cumr_rec.dl_adr3);
		DSP_FLD ("del_name");
		DSP_FLD ("del_addr1");
		DSP_FLD ("del_addr2");
		DSP_FLD ("del_addr3");
		
		use_window (FN14);

		/*
		 * if cus_ord_ref must be input	
		 */
		if (!ORD_DISPLAY)
			FLD ("cus_ord_ref") = (cumr_rec.po_flag [0] == 'Y') ? YES : NO;

		/*
		 * Find currency record for customer currency.       
		 */
		if (envVar.dbMcurr)
		{
			strcpy (pocrRec.co_no, comm_rec.co_no);
			sprintf (pocrRec.code, "%-3.3s", cumr_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
			{
				/*
				 * Customers Currency Not Found 
				 */
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.currCode, "(%-3.3s)", cumr_rec.curr_code);
			DSP_FLD ("currCode");
		}
		
		if (!envVar.dbMcurr || !pocrRec.ex1_factor)
			pocrRec.ex1_factor = 1.00;

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Charge To Number.
	 */
	if (LCHECK ("chargeToCustomer"))
	{
		if (dflt_used || NO_KEY (field))
		{
			strcpy (local_rec.chargeToCustomer, "      ");
			strcpy (local_rec.chargeToName, " ");
			sohr_rec.chg_hhcu_hash	=	0L;
			DSP_FLD ("chargeToCustomer");
			DSP_FLD ("chargeToName");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			sohr_rec.chg_hhcu_hash	=	0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr2_rec.co_no,comm_rec.co_no);
		strcpy (cumr2_rec.est_no,defaultBranchNo);
		strcpy (cumr2_rec.dbt_no,zero_pad (local_rec.chargeToCustomer, 6));
		cc = find_rec (cumr,&cumr2_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * Customer not found. 
			 */
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sohr_rec.chg_hhcu_hash	=	cumr2_rec.hhcu_hash;
		strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
		sprintf (local_rec.chargeToName, "%-35.35s", cumr2_rec.dbt_name);
		DSP_FLD ("chargeToCustomer");
		DSP_FLD ("chargeToName");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Goods Receipt Number.  
	 */
	if (LCHECK ("numberPlate"))
	{
		if (!envVar.skGrinNoPlate || FLD ("numberPlate") == ND)
		{
			newNumberPlate	=	TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSknh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.numberPlate, " ");
			DSP_FLD ("numberPlate");
			newNumberPlate	=	TRUE;
			return (EXIT_SUCCESS);
		}
		LoadSknh ();
		if (newNumberPlate)
		{
			print_mess (ML (mlStdMess049));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		cc = LoadNumberPlateLines (sknh_rec.sknh_hash);
		if (cc)
		{
			print_mess (ML ("Number plate has no lines"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("numberPlate");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Sales Order Number.
	 */
	if (LCHECK ("orderNo"))
	{
		if (NO_KEY (field))
			strcpy (sohr_rec.order_no,"00000000");

		/*
		 * Maintaining Sales Orders
		 */
		if (!ORD_INPUT)
		{
			if (SRCH_KEY)
			{
				if (ORD_DISPLAY && cumr_rec.hhcu_hash == 0L)
					abc_selfield (sohr,"sohr_id_no2");
				else
					abc_selfield (sohr,"sohr_id_no");

				SrchSohr (temp_str);
				abc_selfield (sohr,"sohr_id_no2");
				return (EXIT_SUCCESS);
			}

			/*
			 * Check if order is on file.
			 */
			strcpy (sohr_rec.co_no,comm_rec.co_no);
			strcpy (sohr_rec.br_no,comm_rec.est_no);
			sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

			cc = find_rec (sohr,&sohr_rec,COMPARISON,readLockFlag);
			if (cc)
			{
				/*
				 * Sales Order not found.
				 */
				print_mess (ML (mlStdMess102));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
			if (ORD_DISPLAY && cumr_rec.hhcu_hash == 0L)
			{
				abc_selfield (cumr,"cumr_hhcu_hash");
				cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (!cc)
				{
					strcpy (local_rec.customerNo,cumr_rec.dbt_no);
					strcpy (local_rec.headOfficeCustomer, "N/A   ");
					if (cumr_rec.ho_dbt_hash != 0L)
					{
						cumr2_rec.hhcu_hash	=	cumr_rec.ho_dbt_hash;
						cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
						if (!cc)
							strcpy (local_rec.headOfficeCustomer, cumr2_rec.dbt_no);
					}
				}
				DSP_FLD ("cust_type");
				DSP_FLD ("headOfficeAccount");
				DSP_FLD ("name");
				DSP_FLD ("cus_addr1");
				DSP_FLD ("cus_addr2");
				
				abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
			}

			if (sohr_rec.hhcu_hash != cumr_rec.hhcu_hash)
			{
				/*
				 * Sales Order on file for another Customer.
				 */
				sprintf (err_str, ML (mlSoMess273), sohr_rec.order_no);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}

			/*
			 * Scheduled Order.
			 */
			strcpy (local_rec.schedOrder, sohr_rec.sch_ord);

			if (LoadItemScreen (sohr_rec.hhso_hash))
			{
				/*
				 * Order cannot be changed until all lines are at 
				 * a valid maintenance status. 					 
				 */
				scn_set (HEADER_SCN);
				print_mess (ML (mlSoMess108));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			entry_exit = TRUE;
			strcpy (local_rec.spinst [0],sohr_rec.din_1);
			strcpy (local_rec.spinst [1],sohr_rec.din_2);
			strcpy (local_rec.spinst [2],sohr_rec.din_3);
			newOrder = 0;
		}
		/*
		 * Inputting Sales Orders
		 */
		else
		{
			strcpy (sohr_rec.co_no,comm_rec.co_no);
			strcpy (sohr_rec.br_no,comm_rec.est_no);
			cc = find_rec (sohr,&sohr_rec,COMPARISON,"w");

			/*
			 * Already On File
			 */
			if (!cc)
			{
				print_mess (ML (mlSoMess274));
				sleep (sleepTime);
				abc_unlock (sohr);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
			abc_unlock (sohr);
			newOrder = TRUE;
		}
		SetOrderDefaults (newOrder);
		DSP_FLD ("contact_name");
		DSP_FLD ("contact_phone");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cus_ord_ref"))
	{
		if (!strcmp (sohr_rec.cus_ord_ref,twenty_space))
		{
			if (cumr_rec.po_flag [0] == 'Y')
			{
				/*
				 * Purchase Order Number Must be Input 
				 */
				print_mess (ML (mlStdMess280));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		DSP_FLD ("fix_exch");
		DSP_FLD ("dt_required");

		if (!HIDE (label ("cont_no")))
			FLD ("cont_no") = (CheckContract ()) ? YES : NA;
		
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate department Number and allow search. 
	 */
	if (LCHECK ("dp_no"))
	{
		if (dflt_used || NO_KEY (field))
			strcpy (sohr_rec.dp_no,cumr_rec.department);

		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,sohr_rec.dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * Department Number is not on file.
			 */
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
	/*
	 * validate debtors contract 
	 */
	if (LCHECK ("cont_no"))
	{
		if (NO_KEY (field))
			return (EXIT_SUCCESS);

		if ((local_rec.inputTotal || local_rec.discOverride) && dflt_used == FALSE)
		{
			/*
			 * Can Not Mix Overrides With Contracts 
			 */
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
			cnch_rec.hhch_hash = 0L;
			strcpy (local_rec.cont_desc, " ");
			strcpy (cnch_rec.exch_type, " ");
			return (EXIT_SUCCESS);
		}
		
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Contract not on file.
			 */
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		CheckDate = (prog_status == ENTRY) ? comm_rec.dbt_date : sohr_rec.dt_raised;
		/*
		 * now see if contract is still current.  
		 */
		if (cnch_rec.date_exp < CheckDate)
		{
			/*
			 * Expired Contract
			 */
			sprintf (err_str, ML (mlSoMess270), DateToString (cnch_rec.date_exp));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cnch_rec.date_wef > CheckDate)
		{
			/*
			 * Contract not yet current - effective %s 
			 */
			sprintf (err_str, ML (mlSoMess271), DateToString (cnch_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * now see if contract is assigned to debtor
		 */
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Contract Not Assigned To This Customer
			 */
			print_mess (ML (mlSoMess275));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.cont_desc, "%-30.30s", cnch_rec.desc);
		DSP_FLD ("cont_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("contact_phone"))
	{
		if (!envVar.addressPrompt)    /*  || !CheckCudi ()) */
			skip_entry = goto_field (field, label ("dt_raised"));
	}

	if (LCHECK ("del_name") || LNCHECK ("del_addr", 8))
	{
		if (NO_KEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used && LCHECK ("del_name") && 
					!strcmp (sohr_rec.del_name, ns_space))
		{
			sprintf (sohr_rec.del_name, "%-40.40s", cumr_rec.dbt_name);
			sprintf (sohr_rec.del_add1, "%-40.40s", cumr_rec.dl_adr1);
			sprintf (sohr_rec.del_add2, "%-40.40s", cumr_rec.dl_adr2);
			sprintf (sohr_rec.del_add3, "%-40.40s", cumr_rec.dl_adr3);
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

			strcpy (sohr_rec.del_name, cudi_rec.name);
			strcpy (sohr_rec.del_add1, cudi_rec.adr1);
			strcpy (sohr_rec.del_add2, cudi_rec.adr2);
			strcpy (sohr_rec.del_add3, cudi_rec.adr3);
		}

		DSP_FLD ("del_name");
		DSP_FLD ("del_addr1");
		DSP_FLD ("del_addr2");
		DSP_FLD ("del_addr3");

		if (!envVar.addressPrompt)    /*  || !CheckCudi ()) */
			skip_entry = goto_field (field, label ("dt_raised"));

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Processed Date.
	 */
	if (LCHECK ("dt_raised"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			if (envVar.useSystemDate)
				sohr_rec.dt_raised = TodaysDate ();
			else
				sohr_rec.dt_raised = comm_rec.dbt_date;	
		}

		if (chq_date (sohr_rec.dt_raised,comm_rec.dbt_date))
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Price type.
	 */
	if (LCHECK ("pri_type"))
	{
		int		junk;

		if (prog_status == ENTRY && NO_KEY (field))
			strcpy (local_rec.priceDesc,cumr_rec.price_type);

		if (SRCH_KEY)
		{
			SrchPrice ();
			strcpy (local_rec.priceDesc,"               ");
			return (EXIT_SUCCESS);
		}

		junk = atoi (local_rec.priceDesc);

		if (junk < 1 || junk > envVar.numberPrices)
			return (EXIT_FAILURE);

		sprintf (sohr_rec.pri_type,"%-1.1s",local_rec.priceDesc);
		strcpy (local_rec.priceFullDesc, GetPriceDesc (atoi (sohr_rec.pri_type)));
		DSP_FLD ("pri_type");
		DSP_FLD ("pri_type_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Order type.
	 */
	if (LCHECK ("ord_type"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
			strcpy (local_rec.ord_desc, local_rec.defaultOrderType);

		strcpy (local_rec.ordFullDesc, (local_rec.ord_desc [0] == 'D') 
						? mlSoInput [1] : mlSoInput [2]);

		sprintf (sohr_rec.ord_type,"%-1.1s", local_rec.ord_desc);
		DSP_FLD ("ord_type");
		DSP_FLD ("ord_type_desc");
	}

	/*
	 * Validate Scheduled Order Flag. 
	 */
	if (LCHECK ("schedOrder"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
			strcpy (local_rec.schedOrder, "N");

		if (HIDE (label ("schedOrder")))
			return (EXIT_SUCCESS);

		if (local_rec.schedOrder [0] == 'Y')
		{
			if (cumr_rec.bo_cons [0] != 'Y')
			{
				/*
				 * Customer does not allow consolidation therefore	
				 * scheuled orders cannot be created. 			
				 */
				print_mess (ML (mlSoMess276));
				sleep (sleepTime);
				clear_mess ();

				strcpy (local_rec.schedOrder, "N");
				if (!HIDE (label ("deliveryNo")))
					FLD ("deliveryNo") = NA;

				return (EXIT_SUCCESS);
			}
			strcpy (local_rec.schedOrder, "Y");
			if (!ORD_DISPLAY)
			{
				if (!HIDE (label ("deliveryNo")))
					FLD ("deliveryNo") = NO;
				if (!HIDE (label ("due_date")))
					FLD ("due_date") = NO;
			}
		}
		else
		{
			strcpy (local_rec.schedOrder, "N");
			if (!ORD_DISPLAY)
			{
				if (!HIDE (label ("deliveryNo")))
					FLD ("deliveryNo") = NA;
				if (!HIDE (label ("due_date")))
					FLD ("due_date") = dueDateReqd;
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Tax Code.
	 */
	if (LCHECK ("tax_code"))
	{
		/*
		 * Tax code not input so default to customer master file.
		 */
		if (prog_status == ENTRY && NO_KEY (field))
			strcpy (sohr_rec.tax_code,cumr_rec.tax_code);

		/*
		 * Set noSalesTax for tax that is input and not input.
		 */
		if (sohr_rec.tax_code [0] == 'A' || 
		     sohr_rec.tax_code [0] == 'B')
			noSalesTax = TRUE;
		else
			noSalesTax = FALSE;

		DSP_FLD ("tax_code");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Tax Number. 
	 */
	if (LCHECK ("tax_no"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			DSP_FLD ("tax_no");
			return (EXIT_SUCCESS);
		}

		if (sohr_rec.tax_code [0] == 'C' || sohr_rec.tax_code [0] == 'D')
			return (EXIT_SUCCESS);

		if (!strcmp (sohr_rec.tax_no,"               "))
		{
			/*
			 * Tax code %s so Tax number Must be input.
			 */
			print_mess (ML (mlStdMess200));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Salesman as header Level.
	 */
	if (LCHECK ("sman_code"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (sohr_rec.sman_code,cumr_rec.sman_code);
			DSP_FLD ("sman_code");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used && NEED (label ("sman_code")))
		{
			/*
			 * Input required on Salesman.
			 */
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,sohr_rec.sman_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * No such Salesman on file
			 */
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("sman_desc");

		/*
		 * If sman is not displayed at line level 
		 * and we are in edit mode then force all
		 * sman codes at line level to be the    
		 * same as the header salesman code.    
		 */
		if (prog_status != ENTRY &&
		   (FLD ("sale_code") == NA || FLD ("sale_code") == ND))
		{
			scn_set (ITEM_SCN);
			for (i = 0; i < lcount [ITEM_SCN]; i++)
			{
				getval (i);
				strcpy (soln_rec.sman_code, sohr_rec.sman_code);
				putval (i);
			}
			scn_set (HEADER_SCN);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Area Code.
	 */
	if (LCHECK ("area_code"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (sohr_rec.area_code,cumr_rec.area_code);
			DSP_FLD ("area_code");
		}
		if (dflt_used && NEED (label ("area_code")))
		{
			/*
			 * Input required on Area Code.
			 */
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,sohr_rec.area_code);
		cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * No such Area on file 
			 */
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("area_desc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("inputTotal")) 
	{
		if (strcmp (sohr_rec.cont_no, "      ") && local_rec.inputTotal)
		{
			/*
			 * Can Not Mix Overrides With Contracts
			 */
			print_mess (ML (mlSoMess243));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Overide discount.
	 */
	if (LCHECK ("discOverride")) 
	{
		if (strcmp (sohr_rec.cont_no, "      ") && local_rec.discOverride)
		{
			print_mess (ML (mlSoMess243));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			CalculateTotalBox (TRUE,FALSE);
			scn_set (HEADER_SCN);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
	{
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
			return (DeleteLine (TRUE));

		cc = ValidateItemNumber (TRUE);

		local_rec.line_no 	=	line_cnt + 1;
		DSP_FLD ("line_no");

		DSP_FLD ("ser_no");

		if (strlen (clip (soln_rec.item_desc)) == 0)
			FLD ("descr")	=	YES;
		else
			FLD ("descr")	=	inputDescr;

		return (cc);

	}
	/*
	 * Validate Line Number. 
	 */
	if (LCHECK ("line_no"))
	{
		local_rec.line_no 	=	line_cnt + 1;
		DSP_FLD ("line_no");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Description.
	 */
	if (LCHECK ("descr"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (soln_rec.item_desc,inmr_rec.description);
			DSP_FLD ("descr");
		}
	   	if (COMM_LINE || !strcmp (inmr_rec.item_no,"NS              "))
		  skip_entry = goto_field (field, label ("ser_no"));

		if (prog_status == ENTRY && dflt_used)
			strcpy (soln_rec.item_desc,inmr_rec.description);

		DSP_FLD ("descr");

		tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Unit of Measure. 
	 */
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
			strcpy (local_rec.UOM, SR.uom);

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
			/*
			 * Invalid Unit of Measure
			 */
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			/*
			 * Invalid Unit of Measure
			 */
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, 	inum2_rec.uom);
		strcpy (SR.uom, 		inum2_rec.uom);
		SR.hhumHash 		= 	inum2_rec.hhum_hash;

		if (SR.stdCnvFct == 0.00)
			SR.stdCnvFct = 1.00;

		SR.cnvFct 	= inum2_rec.cnv_fct/SR.stdCnvFct;

		PriceProcess (TRUE);
		DiscProcess (TRUE);
		CalculateTotalBox (FALSE,FALSE);

		DSP_FLD ("UOM");

		if (prog_status != ENTRY)
        {
            /*
             * Reenter Qty. Ord.
             */
            do
            {
                get_entry (label ("qty_ord"));
                cc = spec_valid (label ("qty_ord"));
            } while (cc);
        }

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lots and locations.
	 */
	if (LCHECK ("LL"))
	{
		int		LLReturnValue	=	0;

		if (PHANTOM || INDENT || NON_STOCK)
			return (EXIT_SUCCESS);

		if (llctInput [0] == 'V')
		{
			if (local_rec.LL [0] == 'N')
				LotSelectFlag	=	INP_AUTO;
			else
				LotSelectFlag	=	INP_VIEW;

			if (prog_status == ENTRY)
				strcpy (local_rec.LL, "N");
		}
		if (FLD ("LL") == ND || !SR.canEdit)
			return (EXIT_SUCCESS);

		if (SR.qtyOrder == 0.00 && SR.hhslHash == 0L && prog_status == ENTRY)
			return (EXIT_SUCCESS);
		
		if (SR.hhslHash)
		{
			cc = Load_LL_Lines
			(
				line_cnt,
				LL_LOAD_SO,
				SR.hhslHash,
				SR.hhccHash,
				SR.uom,
				SR.cnvFct,
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
						SR.actHhwhHash, 						
						SR.hhumHash,						
						SR.hhccHash,						
						SR.uom,							
						SR.qtyOrder,						
						SR.cnvFct,						
						TodaysDate (),
						TRUE,
						FALSE,
						SR.lotControlled						
					);
			}
			strcpy (local_rec.LL, "Y");
		}
		TempLine	=	lcount [2];
		LLReturnValue = DisplayLL
		(											/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.actHhwhHash, 					/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.uom,							/* UOM					*/
				SR.qtyOrder,						/* Quantity.			*/
				SR.cnvFct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				LotSelectFlag,						/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR.lotControlled						/* Lot controled item. 	*/
													/*----------------------*/
		);

		DSP_FLD ("LL");
		/*
		 * Redraw screens.
		 */
		putval (line_cnt);

		lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		scn_write (2);
		scn_display (2);
		lcount [2] = TempLine;
		pr_box_lines (cur_screen);
		PrintCompanyDetails ();

		if (LLReturnValue)
			return (EXIT_FAILURE);

		strcpy (local_rec.LL, "Y");

		if (BONUS || SR.contractStatus)
		{
			soln_rec.due_date = sohr_rec.dt_required;
			DSP_FLD ("due_date");
			skip_entry = goto_field (field, label ("ser_no")); 
		}

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Salesman Code At Item Level.
	 */
	if (LCHECK ("sale_code"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (soln_rec.sman_code,sohr_rec.sman_code);
			DSP_FLD ("sale_code");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,soln_rec.sman_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			/*
			 * No such Salesman on file
			 */
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!HIDE (field))
			print_at (2,1, ML (mlStdMess202),
							exsf_rec.salesman_no,exsf_rec.salesman);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prt_price")) 
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (sohr_rec.prt_price, "Y");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Customer Order Ref At Item Level.
	 */
	if (LCHECK ("ord_ref"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (soln_rec.cus_ord_ref,sohr_rec.cus_ord_ref);
			DSP_FLD ("ord_ref");
		}

		if (FIELD.required != ND && prog_status != ENTRY)
		{
			if (!strcmp (soln_rec.cus_ord_ref,twenty_space))
			{
				if (cumr_rec.po_flag [0] == 'Y')
				{
					/*
					 * Purchase Order Number Must be Input
					 */
					print_mess (ML (mlStdMess280));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Pack Size At Item Level.
	 */
	if (LCHECK ("pack_size"))
	{
		if (dflt_used || FIELD.required != YES)
		{
			strcpy (soln_rec.pack_size,SR.packSize);
			DSP_FLD ("pack_size");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Quantity Input.
	 */
	if (LCHECK ("qty_ord"))
	{
		if (prog_status != ENTRY)
			skip_entry = 0;


		SR.qtyOrigional = local_rec.qty_ord;

		if (COMM_LINE && local_rec.qty_ord != 0.00)
		{
			/*
			 * Class 'Z' (Description items) cannot have a quantity
			 */
			local_rec.qty_ord = 0.00;
			SR.qtyOrigional	=	0.00;
			print_mess (ML (mlSoMess277));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		SR.qtyTotal = ToStdUom (local_rec.qty_ord);

		if (KIT_ITEM)
		{
			SR.qtyOrder 	= local_rec.qty_ord;
			SR.hhbrHash 	= 0;
			this_page 		= line_cnt / TABLINES;
			ProcessKitItem 
			(
				inmr_rec.hhbr_hash,
				ToStdUom (local_rec.qty_ord)
			);
			strcpy (temp_str, "");
			strcpy (local_rec.item_no, "");
			skip_entry = goto_field (label ("qty_ord"), label ("item_no"));
			SR.qtyTotal = 0.00;
			local_rec.qty_ord = 0.00;
			if (this_page == (line_cnt / TABLINES))
				blank_display ();

			if (prog_status != ENTRY)
			{
				/*
				 * Reenter Location.
				 */
				do
				{
					strcpy (local_rec.LL, "N");
					get_entry (label ("LL"));
					if (restart)
						break;
					cc = spec_valid (label ("LL"));
				} while (cc && !HIDE (label ("LL")));
			}

			return (EXIT_SUCCESS);
		}

		if (PHANTOM)
		{
        	SR.qtyAvailable = ProcessPhantom
						 	(
								inmr_rec.hhbr_hash, 
								ccmr_rec.hhcc_hash
						 	);
		}

		if (prog_status != ENTRY)
			SR.qtyTotal += ToStdUom (local_rec.qty_back);

		if (prog_status == ENTRY)
			local_rec.qty_back = 0.00;
			
		/*
		 * Serial Items Can only have Qty of 0.00 or 1.00
		 */
		if (SERIAL && local_rec.qty_ord != 0.00 && local_rec.qty_ord != 1.00)
		{
			/*
			 * Quantity can only be 0.00 or 1.00 for Serial Items 
			 */
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * if backorder set order quantity to zero        
		 */
		if (SERIAL && local_rec.qty_ord == 0.00)
		{
			if (strcmp (SR.serialNo, serialSpace))
			{
				cc = UpdateInsf (SR.hhwhHash,SR.hhsiHash,SR.serialNo,"C","F");
				if (cc && cc < 1000)
					file_err (cc, (char *)insf, "DBUPDATE");
			}
			if (!NO_KEY (label ("qty_back")))
				local_rec.qty_back = 1.00;

			strcpy (local_rec.serialNo, serialSpace);
			strcpy (SR.serialNo, serialSpace);
			strcpy (soln_rec.serial_no, serialSpace);
			DSP_FLD ("qty_back");
			DSP_FLD ("ser_no");
		}

		if (SERIAL && local_rec.qty_ord > 0.00)
		{
			local_rec.qty_back = 0.00;
			DSP_FLD ("qty_back");
		}

		/*
		 * Recalculate the actual current available stock.
		 */
		if (!PHANTOM)
			SR.qtyAvailable = ReCalcAvail ();

		PriceProcess (TRUE);
		DiscProcess (TRUE);

		/*
		 * Update the real-time committal record.
		 */
		UpdSoicQty ();

		if (!STANDARD && 
			envVar.windowPopupOk && 
			((SR.qtyAvailable - ToStdUom (local_rec.qty_ord)) < 0.00) && 
			!NON_STOCK && !COMM_LINE && !(MANF_ITEM && envVar.soWoAllowed))
		{
			/*
			 * WARNING only %.2f Available for part # %s
			 */
		    sprintf (err_str, ML (mlStdMess090),
									SR.qtyAvailable,
									clip (local_rec.item_no));
		    cc = WarnUser (err_str, 1);
		    BusyFunction (TRUE);
		    sup_frm_alt = FALSE;
		    if (envVar.supplyFromAlternate)
				sup_frm_alt = ProcessAltSupWh ();

		    if (!sup_frm_alt)
		    {
				if (InputResponce () && 
					sohr_rec.full_supply [0] == 'N' &&
					envVar.fullSupplyOrder &&
					local_rec.schedOrder [0] == 'N')
				{
					sprintf (err_str, "%s %s ? ", ML (mlSoMess386), 
												 cumr_rec.dbt_no);
					i = prmptmsg (err_str,"YyNn",1,2);
					BusyFunction (FALSE);
					if (i == 'Y' || i == 'y')
						strcpy (sohr_rec.full_supply, "Y");
					else
						strcpy (sohr_rec.full_supply, "N");
				}
		    }
		
		    if (skip_entry != 0)
		    	return (EXIT_SUCCESS);

		    DSP_FLD ("qty_ord");
		}
		if (MANF_ITEM && envVar.soWoAllowed)
		{
			 if ((SR.qtyAvailable - ToStdUom (local_rec.qty_ord)) < 0.00)
			 {
				local_rec.qty_back 	= local_rec.qty_ord;
				local_rec.qty_ord 	= ToLclUom (SR.qtyAvailable);
				if (local_rec.qty_ord < 0.00)
					local_rec.qty_ord = 0.00;

				local_rec.qty_back -= local_rec.qty_ord;

				DSP_FLD ("qty_ord");
				DSP_FLD ("qty_back");
			}
		}
		if (prog_status != ENTRY)
		{
			SR.qtyOrder = local_rec.qty_ord;
			DSP_FLD ("qty_ord");

			/*
			 * Reenter Location.
			 */
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				if (restart)
					break;
				cc = spec_valid (label ("LL"));
			} while (cc && !HIDE (label ("LL")));
		}

		if (prog_status == ENTRY && NON_STOCK && HIDE (label ("cost_price")))
		{
			print_at (2,1,"%-90.90s"," ");
			fflush (stdout);
			/*
			 * Input Cost Price.
			 */
			print_at (2,1, ML (mlSoMess238));
			SR.costPrice = getmoney (20,2,"NNNNNNN.NN");
			BusyFunction (FALSE);
		}
		else
			SR.costPrice = 0.00;

		if (prog_status == ENTRY)
			DSP_FLD ("qty_back");

		if (BONUS || SR.contractStatus)
		{
			soln_rec.due_date = sohr_rec.dt_required;
			DSP_FLD ("due_date");
			skip_entry = goto_field (field, label ("LL"));
		}
		else
		{
			if (NON_STOCK)
				skip_entry = 1;
		}
		SR.qtyOrder = local_rec.qty_ord;

		if (prog_status != ENTRY || NO_KEY (label ("sale_price")))
			CalculateTotalBox (FALSE, FALSE);

		if (MARGIN_OK)
		{
			if (
				MarginCheckOk (
					SR.actSalePrice, 
					SR.discPc, 
					SR.marginCost, 
					SR.minMargin, 
					field)
				)
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
			else
				strcpy (SR.marginFailed, "N");
		}
		else
			strcpy (SR.marginFailed, "N");

		/*
		 * Update the real-time committal record.
		 */
		UpdSoicQty ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Quantity Backordered.
	 */
	if (LCHECK ("qty_back"))
	{
		if (COMM_LINE && local_rec.qty_back != 0.00)
		{
			/*
			 * Class 'Z' (Description items) cannot have a quantity. 
			 */
			local_rec.qty_back = 0.00;
			print_mess (ML (mlSoMess277));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		if (!BO_OK)
		{
			if (local_rec.qty_back != 0.00)
			{
				/*
				 * Cannot Backorder for this item
				 */
				print_mess (ML (mlStdMess030));

				sleep (sleepTime);
				local_rec.qty_back = 0.00;
				DSP_FLD ("qty_back");
			}
			return (EXIT_SUCCESS);
		}

		if (ToStdUom (local_rec.qty_back) > SR.qtyTotal)
		{
			/*
			 * Back Order Qty of %8.2f Exceeds Total Order Qty of %8.2f
			 */
			print_mess (ML (mlSoMess106));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Serial Items Can only have Qty of 0.00 or 1.00	
		 */
		if (SERIAL && local_rec.qty_back != 0.00 && local_rec.qty_back != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * if backorder then set order to zero  
		 */
		if (SERIAL && local_rec.qty_back > 0.00)
		{
			if (strcmp (SR.serialNo, serialSpace))
			{
				cc = UpdateInsf (SR.hhwhHash,SR.hhsiHash,SR.serialNo,"C","F");
				if (cc && cc < 1000)
					file_err (cc, (char *)insf, "DBUPDATE");
			}

			local_rec.qty_ord = 0.00;
			strcpy (local_rec.serialNo, serialSpace);
			strcpy (SR.serialNo, serialSpace);
			strcpy (soln_rec.serial_no, serialSpace);
			DSP_FLD ("qty_ord");
			DSP_FLD ("ser_no");
		}

		if (SERIAL && local_rec.qty_back > 0.00)
		{
			local_rec.qty_ord = 0.00;
			SR.qtyOrder = local_rec.qty_ord;
			CalculateTotalBox (FALSE, FALSE);
			DSP_FLD ("qty_ord");
		}

		PriceProcess (TRUE);
		DiscProcess (TRUE);

		SR.qtyTotal = ToStdUom (local_rec.qty_ord);
		SR.qtyTotal += ToStdUom (local_rec.qty_back);
		CalculateTotalBox (FALSE, FALSE);

		/*
		 * Update the real-time committal record.
		 */
		UpdSoicQty ();
	
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Cost Price Input. 
	 */
	if (LCHECK ("cost_price"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
		{
			soln_rec.cost_price = 0.00;
			DSP_FLD ("cost_price");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Sale Price Input. 
	 */
	if (LCHECK ("sale_price"))
	{
		double	previousPrice	=	SR.salePrice;

		if (BONUS)
		{
			/*
			 * Item is A bonus Item - So it cannot have a Price
			 */
			print_mess (ML (mlSoMess233));
			sleep (sleepTime);
			soln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (SR.priceOR,"N");
			PriceProcess (TRUE);
			DiscProcess (TRUE);
			DSP_FLD ("sale_price");
		}

		if (soln_rec.sale_price == 0.00)
		{
			/*
			 * Is This Really a no Charge Item ?
			 */
			i = prmptmsg (ML (mlStdMess031), "YyNn",1,2);              
			BusyFunction (FALSE);

			if (i == 'N' || i == 'n')
			{
				FLD ("sale_price")  =   YES;
				skip_entry = goto_field (field, label ("sale_price"));
				if (skip_entry < 0 && PrevKit)
					return (EXIT_FAILURE);
				strcpy (SR.priceOR,"Y");
				return (EXIT_SUCCESS);
			}
		}
		if (SR.calcSalePrice != soln_rec.sale_price)
			strcpy (SR.priceOR,"Y");

		/*
		 * Calculate _new GROSS sale price. 
		 */
		SR.gSalePrice = DPP (soln_rec.sale_price) / (1.00 - (SR.regPc / 100.00)); 
		SR.salePrice  = GetCusGprice (SR.gSalePrice, SR.regPc); 
		soln_rec.sale_price = SR.salePrice;

		DiscProcess (TRUE);

		SR.actSalePrice = DPP (soln_rec.sale_price);

		if (prog_status != ENTRY || SR.salePrice == 0.00)
			CalculateTotalBox (FALSE, FALSE);

		strcpy (SR.marginFailed, "N");
		if (MARGIN_OK && previousPrice != SR.salePrice)
		{
			if (
				MarginCheckOk 
				(
					SR.actSalePrice, 
					SR.discPc, 
				    SR.marginCost, 
					SR.minMargin, 
					field)
				)
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Date Required. 
	 */
	if (LCHECK ("dt_required"))
	{
		if (prog_status == ENTRY && NO_KEY (field))
			sohr_rec.dt_required = local_rec.lsystemDate;

		if (sohr_rec.dt_required > local_rec.lsystemDate)
		{
			/*
			 * Forward Order,Delay Release ? 
			 */
			i = prmptmsg (ML (mlSoMess109),"YyNn",1,2);
			BusyFunction (FALSE);
			if (i == 'N' || i == 'n')
			{
				sohr_rec.dt_required = local_rec.lsystemDate;
				DSP_FLD ("dt_required");
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Discount Percent. 
	 */
	if (LCHECK ("disc"))
	{
		float	previousDisc	=	SR.discPc;

		if (prog_status == ENTRY && NO_KEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (SR.discOR,"N");
			DiscProcess (TRUE);
		}

		if (SR.contractPrice || SR.contractStatus == 2)
		{
			soln_rec.dis_pc = 0.00;
			SR.discA		= 0.00;
			SR.discB		= 0.00;
			SR.discC		= 0.00;
			DSP_FLD ("disc");
		}

		SR.discPc   = ScreenDisc (soln_rec.dis_pc);

		if (SR.calcDisc != ScreenDisc (soln_rec.dis_pc))
			strcpy (SR.discOR,"Y");

		/*
		 * Discount has been entered so set disc B & C to zero.     
		 */
		if (!dflt_used)
		{
			SR.discA = SR.discPc;
			SR.discB = 0.00;
			SR.discC = 0.00;
		}

		CalculateTotalBox (FALSE, FALSE);

		if (MARGIN_OK && previousDisc != SR.discPc)
		{
			if (MarginCheckOk 
				(
					SR.actSalePrice, 
					SR.discPc, 
					SR.marginCost, 
					SR.minMargin,
					field)
				)
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
			else
				strcpy (SR.marginFailed, "N");
		}
		else
			strcpy (SR.marginFailed, "N");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Due Date , Item Level. 
	 */
	if (LCHECK ("due_date"))
	{
		if (((prog_status == ENTRY && NO_KEY (field)) || dflt_used) && !PrevKit)
		{
			soln_rec.due_date = sohr_rec.dt_required;
			DSP_FLD ("due_date");
		}

		if (soln_rec.due_date > local_rec.lsystemDate && 
		     sohr_rec.dt_required != soln_rec.due_date &&
			 local_rec.schedOrder [0] != 'Y')
		{
			/*
			 * Forward Order,Delay Release ?
			 */
			i = prmptmsg (ML (mlSoMess109),"YyNn",1,2);
			BusyFunction (FALSE);
			if (i == 'N' || i == 'n')
			{
				soln_rec.due_date = local_rec.lsystemDate;
				DSP_FLD ("due_date");
			}
		}

		DateToDMY (comm_rec.inv_date, &dmy [0], &dmy [1], &dmy [2]);
		inv_mo =  dmy [1];
		inv_yr =  dmy [2];
		DateToDMY (soln_rec.due_date, &dmy [0], &dmy [1], &dmy [2]);
		due_mo =  dmy [1];
		due_yr =  dmy [2];

		if ((due_mo < inv_mo && due_yr == inv_yr) || (due_yr < inv_yr)) 
		{
			/*
			 * Due Date is less than the beginning of the current inv mod mo 
			 */
			i = prmptmsg ("Due Date is less than current Inventory Module Month. continue? ",  "YyNn",1,2);
			BusyFunction (FALSE);
			if (i == 'N' || i == 'n')
			{
				soln_rec.due_date = local_rec.lsystemDate;
				DSP_FLD ("due_date");
				return (EXIT_FAILURE);
			}
		}

		if (prog_status == ENTRY)
			skip_entry = (local_rec.qty_ord != 0.00 && 
				    inmr_rec.serial_item [0] == 'Y') ? 0 : 1;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate serial number input. 
	 */
	if (LCHECK ("ser_no"))
	{
		if (SR.hhwhHash < 0L)
			return (EXIT_SUCCESS);

		if (end_input)
			return (EXIT_SUCCESS);

		abc_selfield (insf,"insf_id_no");

		if (SRCH_KEY)
		{
			SrchInsf (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.serialNo, SR.serialNo))
			return (EXIT_SUCCESS);

		if (dflt_used || !strcmp (local_rec.serialNo, serialSpace))
		{
			/*
			 * Free previous serial number if any 
			 */
			if (strcmp (SR.serialNo, serialSpace))
			{
				cc = 	UpdateInsf 
						(
							SR.hhwhHash, 
							SR.hhsiHash, 
							SR.serialNo, 
							"C", 
							"F"
						);
	
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}

			strcpy (local_rec.serialNo, serialSpace);
			strcpy (SR.serialNo, serialSpace);
			return (EXIT_SUCCESS);
		}

		insf_rec.hhwh_hash = SR.hhwhHash;
		insf_rec.hhbr_hash = SR.hhsiHash;
		strcpy (insf_rec.status,"F");
		sprintf (insf_rec.serial_no, "%-25.25s", local_rec.serialNo);
		cc = find_rec (insf,&insf_rec,COMPARISON,"r");
		if (cc)
		{
			abc_selfield (insf,"insf_hhbr_id");

			insf_rec.hhwh_hash = SR.hhwhHash;
			insf_rec.hhbr_hash = SR.hhsiHash;
			strcpy (insf_rec.status,"F");
			sprintf (insf_rec.serial_no, "%-25.25s", local_rec.serialNo);
			cc = find_rec (insf,&insf_rec,COMPARISON,"r");
		}
		if (cc)
		{
			insf_rec.hhwh_hash = SR.hhwhHash;
			insf_rec.hhbr_hash = SR.hhsiHash;
			strcpy (insf_rec.status,"C");
			sprintf (insf_rec.serial_no, "%-25.25s", local_rec.serialNo);
			cc = find_rec (insf,&insf_rec,COMPARISON,"r");
			if (!cc)
			{
				/*
				 * Serial item is already committed. 
				 */
				print_mess (ML (mlSoMess052));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			insf_rec.hhwh_hash = SR.hhwhHash;
			insf_rec.hhbr_hash = SR.hhsiHash;
			strcpy (insf_rec.status,"S");
			sprintf (insf_rec.serial_no, "%-25.25s", local_rec.serialNo);
			cc = find_rec (insf,&insf_rec,COMPARISON,"r");
			if (!cc)
			{
				/*
				 * Serial item is already sold. 
				 */
				print_mess (ML (mlSoMess278));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			/*
			 * No such serial no for item 
			 */
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckDuplicateSerial (local_rec.serialNo, SR.hhsiHash, line_cnt))
		{
			/*
			 * Duplicate Serial Number 
			 */
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Free previous serial number if any 
		 */
		if (strcmp (SR.serialNo, serialSpace))
		{
			cc = UpdateInsf (SR.hhwhHash, SR.hhsiHash, SR.serialNo, "C", "F");

			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}

		strcpy (SR.serialNo, local_rec.serialNo);

		cc = UpdateInsf (SR.hhwhHash, SR.hhsiHash, SR.serialNo, "F", "C");

		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");

		DSP_FLD ("ser_no");
		abc_selfield (insf,"insf_id_no");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate delivery address number. 
	 */
	if (LCHECK ("deliveryNo"))
	{
		if (prog_status == ENTRY && (NO_KEY (field) || dflt_used))
		{
			strcpy (local_rec.deliveryNo, "     ");
			return (EXIT_SUCCESS);
		}

		open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");
		if (SRCH_KEY)
		{
			i = SrchCudi (field - label ("shipname"));

			if (i < 0)
			{
				abc_fclose (cudi);
				return (EXIT_SUCCESS);
			}
			
			sprintf (local_rec.deliveryNo, "%05d", i);
		}

		cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
		cudi_rec.del_no 	= atoi (local_rec.deliveryNo);
		cc = find_rec (cudi, &cudi_rec, COMPARISON, "r");
		abc_fclose (cudi);
		if (cc)
		{
			/*
			 * Address Not Found 
			 */
			print_mess (ML (mlStdMess213));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate freight. 
	 */
	if (LCHECK ("freight"))
	{
		if (dflt_used && FREIGHT_CHG)
		{
			CalculateFreight 
			(
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
				
			DSP_FLD ("freight");
			DSP_FLD ("est_freight");
			DSP_FLD ("tot_kg");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Selling Terms. 
	 */
	if (LCHECK ("sell_terms"))
	{
		if (SRCH_KEY)
		{
			SR_Y_POS = vars [label ("sos_ok")].row - 1;
			SrchSell ();
			SR_Y_POS = 0;
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (STerms [i]._scode);i++)
		{
			if (!strncmp (sohr_rec.sell_terms,STerms [i]._scode,strlen (STerms [i]._scode)))
			{
				sprintf (local_rec.sell_desc,"%-30.30s",STerms [i]._sdesc);
				break;
			}
		}

		if (!strlen (STerms [i]._scode))
		{
			/*
			 * Invalid Selling Terms
			 */
			print_mess (ML (mlStdMess214));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		DSP_FLD ("sell_terms");
		DSP_FLD ("sell_desc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Payment Terms. 
	 */
	if (LCHECK ("pay_term"))
	{
		val_pterms = FALSE;

		if (SRCH_KEY)
		{
			SR_Y_POS = vars [label ("sos_ok")].row - 1;

			SrchPay ();

			SR_Y_POS = 0;
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (sohr_rec.pay_term,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (sohr_rec.pay_term,"%-40.40s",p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			/*
			 * %s not valid Payment Terms. 
			 */
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_term");

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Other Cost Three. 
	 */
	if (LCHECK ("other_3"))
	{
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Shipment Name And Addresses. 
	 */
	if (!strcmp (FIELD.label,"shipname") || 
	     !strncmp (FIELD.label,"shipaddr",8)) 
	{
		if (SRCH_KEY)
		{
			open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");

			SR_Y_POS = vars [label ("carrierCode")].row - 1;

			i = SrchCudi (field - label ("shipname"));

			SR_Y_POS = 0;

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (sohr_rec.del_name,cudi_rec.name);
			strcpy (sohr_rec.del_add1,cudi_rec.adr1);
			strcpy (sohr_rec.del_add2,cudi_rec.adr2);
			strcpy (sohr_rec.del_add3,cudi_rec.adr3);

			DSP_FLD ("shipname");
			DSP_FLD ("shipaddr1");
			DSP_FLD ("shipaddr2");
			DSP_FLD ("shipaddr3");
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Shipment method (s) 
	 */
	if (LCHECK ("ship_method") || LCHECK ("spcode1") || LCHECK ("spcode2"))
	{
		i = field - label ("ship_method") ;

		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (SRCH_KEY)
		{
			SR_Y_POS = vars [label ("carrierCode")].row - 1;

			SrchExsi (temp_str);

			SR_Y_POS = 0;

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
	/*
	 * Validate Freight Required Flag. 
	 */
	if (LCHECK ("freightRequiredFlag"))
	{
		if (NO_KEY (field))
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

	/*
	 * Validate Carrier Code. 
	 */
	if (LCHECK ("carrierCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				sohr_rec.freight = 0.00;

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
			SR_Y_POS = vars [label ("carrierCode")].row - 1;

			SrchTrcm (temp_str);

			SR_Y_POS = 0;
			
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

	/*
	 * Validate Reference Number.
	 */
	if (LCHECK ("deliveryZoneCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				sohr_rec.freight = 0.00;

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

		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (trzm_rec.del_zone, cumr_rec.del_zone);
			DSP_FLD ("deliveryZoneCode");
		}
		if (SRCH_KEY)
		{
			SR_Y_POS = vars [label ("deliveryZoneCode")].row - 1;

			SrchTrzm (temp_str);

			SR_Y_POS = 0;

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
			clear_mess ();
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
	/*
	 * Validate delivery required.
	 */
	if (LCHECK ("deliveryRequired"))
	{
		if (newOrder)
			return (EXIT_SUCCESS);

		/*
		 * Schedule required. 
		 */
		if (sohr_rec.del_req [0] == 'Y')
		{
			move (0,2);cl_line ();
			i = prmptmsg (ML (mlTrMess063) ,"YyNn",0,2);
			BusyFunction (FALSE);
			if (i == 'N' || i == 'n') 
				return (EXIT_SUCCESS);

			sprintf (err_str,"tr_trsh_mnt O %010ld LOCK",sohr_rec.hhso_hash);
			sys_exec (err_str);
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
			trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
			cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (sohr_rec.s_timeslot, trsh_rec.sdel_slot);
				strcpy (sohr_rec.e_timeslot, trsh_rec.edel_slot);
				sohr_rec.del_date	=	trsh_rec.del_date;
				cc = abc_update (sohr, &sohr_rec);
				if (cc)
					file_err (cc, (char *)sohr, "DBUPDATE");
			}
			abc_fclose (trsh);
			heading (FREIGHT_SCN);
			scn_write (FREIGHT_SCN);
			scn_display (FREIGHT_SCN);
			print_mess (ML (mlTrMess076));
			sleep (sleepTime);
			DSP_FLD ("deliveryDate");
		}
		else
		{
			/*
			 * Schedule not required and was not an origional delivery. 
			 */
			if (local_rec.origDelRequired [0] == 'N')
				return (EXIT_SUCCESS);

			/*
			 * Schedule not required but was set to delivery before. 
			 */
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
			trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
			cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
			while (!cc && trsh_rec.hhso_hash == sohr_rec.hhso_hash)
			{
				abc_delete (trsh);
				trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
				cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
			}
			abc_fclose (trsh);
		}
		return (EXIT_SUCCESS); 
	}
	return (EXIT_SUCCESS);
}

/*
 * Process soic record for current line. 
 */
void
ProcSoic (
 int	delLine,
 int	procLine)
{
	/*
	 * Deleting a line ? 
	 */
	if (delLine)
	{
		/*
		 * Find the soic record/s and delete. 
		 */
		if (store [procLine].hhbrHash == store [procLine].origHhbr)
		{
			/*
			 * Ensure whole line is decommitted. 
			 */
			if (store [procLine].deCommitRef == 0)
			{
				strcpy (soic_rec.status, "A");
				soic_rec.pid  = progPid;
				soic_rec.line = GenSoicRef ();

				soic_rec.hhbr_hash = store [procLine].origHhbr;
				soic_rec.hhcc_hash = store [procLine].hhccHash;
				soic_rec.qty = (float) (store [procLine].origOrderQty * -1.00);
				sprintf (soic_rec.program, "%-20.20s", PNAME);
				sprintf (soic_rec.op_id, "%-14.14s", currentUserName);
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
				{
					abc_unlock (soic2);
					return;
				}
				else
				{
					soic_rec.qty = (float) (store [procLine].origOrderQty * -1.00);
					cc = abc_update (soic2, &soic_rec);
					if (cc)
						file_err (cc, soic2, "DBUPDATE");
				}
			}
		}

		/*
		 * Remove commit soic record. 
		 */
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

	/*
	 * Add _new soic record. 
	 */
	if (store [procLine].commitRef == 0)
		AddNewSoic (procLine);
	else
	/*
	 * Update existing soic record. 
	 */
	{
		/*
		 * Soic exists but line not loaded from file. 
		 */
		if (store [procLine].origHhbr == 0L)
		{
			strcpy (soic_rec.status, "A");
			soic_rec.pid  = progPid;
			soic_rec.line = store [procLine].commitRef;
			cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
			if (cc)
			{
				abc_unlock (soic2);
				return;
			}
			soic_rec.hhbr_hash = store [procLine].hhbrHash;
			soic_rec.qty       = 0.00;

			cc = abc_update (soic2, &soic_rec);
			if (cc)
				file_err (cc, soic2, "DBUPDATE");
		}
		else
		/*
		 * Soic exists and line loaded from file. 
		 */
		{
			/*
			 * We have changed the item back to what it
			 * was originally so remove any commit and
			 * decommit records for the line.         
			 * (Commit records will be created if required
			 *  when the qty is re-entered for the    
			 *  original item.)                      
			 */
			if (store [procLine].hhbrHash == store [procLine].origHhbr)
			{
				/*
				 * Remove commit soic record. 
				 */
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

				/*
				 * Remove de-commit soic record. 
				 */
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
			/*
			 * We have changed the item for this line 
			 * therefore we need to de-commit the qty
			 * for the old item.                    
			 */
			{	
				/*
				 * Add a de-commit record for the total qty. 
				 */
				if (store [procLine].deCommitRef == 0)
				{
					strcpy (soic_rec.status, "A");
					soic_rec.pid  = progPid;
					soic_rec.line = GenSoicRef ();

					store [procLine].deCommitRef = soic_rec.line;
					soic_rec.hhbr_hash = store [procLine].origHhbr;
					soic_rec.hhcc_hash = store [procLine].hhccHash;
					soic_rec.qty = (float) (store [procLine].origOrderQty * -1.00);
					sprintf (soic_rec.program, "%-20.20s", PNAME);
					sprintf (soic_rec.op_id, "%-14.14s", currentUserName);
					strcpy (err_str, TimeHHMMSS ());
					soic_rec.time_create = atot (err_str);
					soic_rec.date_create = TodaysDate ();
			
					cc = abc_add (soic2, &soic_rec);
					if (cc)
						file_err (cc, soic2, "DBADD");
				}	

				/*
				 * Add / update a commit record for the new item. 
				 */
				if (store [procLine].commitRef == -1)
				{
					/*
					 * Add a _new commit soic record for _new item.
					 */
					AddNewSoic (procLine);
				}	
				else
				{
					/*
					 * Update the existing commit soic record for _new item.
					 */
					strcpy (soic_rec.status, "A");
					soic_rec.pid  = progPid;
					soic_rec.line = store [procLine].commitRef;
					cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
					if (cc)
					{
						abc_unlock (soic2);
						return;
					}
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

	store [lineNo].commitRef = soic_rec.line;
	soic_rec.hhbr_hash = store [lineNo].hhbrHash;
	soic_rec.hhcc_hash = 0L;
	soic_rec.qty = (float) 0.00;
	sprintf (soic_rec.program, "%-20.20s", PNAME);
	sprintf (soic_rec.op_id, "%-14.14s", currentUserName);
	strcpy (err_str, TimeHHMMSS ());
	soic_rec.time_create = atot (err_str);
	soic_rec.date_create = TodaysDate ();

	cc = abc_add (soic2, &soic_rec);
	if (cc)
		file_err (cc, soic2, "DBADD");
}

/*
 * Generate _new reference to be used as a commitRef or a deCommitRef. 
 */
int
GenSoicRef (void)
{
	return (++nextSoicRef);
}

/*
 * Update soic quantity for current line. 
 */
void
UpdSoicQty (
 void)
{
	float	updQty;
	float	lineQty;

	lineQty = ToStdUom (local_rec.qty_ord + local_rec.qty_back);
	/*
	 * Line is back at its original item. 
	 */
	if (store [line_cnt].hhbrHash == store [line_cnt].origHhbr)
	{
		/*
		 * Qty differs but no soic record exists so add
		 * a commit soic record ready for qty update. 
		 */
		if (store [line_cnt].commitRef == -1)
		{
			if (lineQty != store [line_cnt].origOrderQty)
			{
				strcpy (soic_rec.status, "A");
				soic_rec.pid = progPid;
				soic_rec.line = GenSoicRef ();
				store [line_cnt].commitRef = soic_rec.line;
				soic_rec.hhbr_hash = store [line_cnt].hhbrHash;
				soic_rec.hhcc_hash = 0L;
				soic_rec.qty = (float) 0.00;
				sprintf (soic_rec.program, "%-20.20s", PNAME);
				sprintf (soic_rec.op_id, "%-14.14s", currentUserName);
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

	if (store [line_cnt].hhbrHash == store [line_cnt].origHhbr)
		updQty = lineQty - store [line_cnt].origOrderQty;
	else
		updQty = lineQty;

	/*
	 * Find the soic record for commitRef.
	 */
	strcpy (soic_rec.status, "A");
	soic_rec.pid = progPid;
	soic_rec.line = store [line_cnt].commitRef;
	cc = find_rec (soic2, &soic_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (soic2);
		return;
	}

	soic_rec.hhcc_hash = store [line_cnt].hhccHash;
	soic_rec.qty = updQty;

	cc = abc_update (soic2, &soic_rec);
	if (cc)
		file_err (cc, soic2, "DBUPDATE");

}
/*
 * Clear soic records for current PID. 
 */
void
ClearSoic (
 void)
{
	strcpy (soic_rec.status, "A");
	soic_rec.pid  = progPid;
	soic_rec.line = 0;
	cc = find_rec (soic2, &soic_rec, GTEQ, "u");
	while (!cc && 
		   soic_rec.status [0] == 'A' &&
		   soic_rec.pid == progPid)
	{
		/*
		 * Delete record. 
		 */
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
/*
 * Recalculate the current available stock. 
 * Ignore the soic_line relating to        
 * commitRef for the current line.        
 */
float 
ReCalcAvail (void)
{
	float	realStock;
	float	realCommitted;

	/*
	 * Look up incc record.
	 */
    incc2_rec.hhcc_hash = SR.hhccHash;
    incc2_rec.hhbr_hash = SR.hhsiHash;
    cc = find_rec (incc, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	/*
	 * Calculate actual committed.     
	 * Ignore record for current line.
	 */
	realCommitted = RealTimeCommitted (SR.hhbrHash,
									   SR.hhccHash,
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

	/*
	 * Add into available any stock that was on line when loaded. 
	 */
	if (SR.hhbrHash == SR.origHhbr)
		realStock += SR.origOrderQty;

	return (realStock);
}
/*
 * Function    : RealTimeCommitted ()                        
 * Description : Calculates the actual real-time committed  
 *               quantity of stock for a warehouse (or all 
 *               warehouses).  Data is taken from the soic
 *               file which is updated real-time by a     
 *               number of programs including so_input etc 
 * Parameters  : hhbrHash - link to inventory master (inmr)
 *               hhccHash - link to warehouse master (ccmr)
 *                        - If hhccHash is 0L then calculate
 *                          for all warehouses.            
 *               ignLine  - line to ignore.               
 * Returns     : A float containing the actual quantity  
 *               committed at this time.                
 */
float 
RealTimeCommitted (
  long	hhbrHash,
  long	hhccHash,
  int	ignLine)
{
	float	commQty;	/* Accumulator of committed quantity */

	/*
	 * Initialise calculated quantity. 
	 */
	commQty = 0.00;

	/*
	 * Read soic records.  If hhccHash is 0L
	 * then read for all warehouses.         
	 */
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

/*
 * Check for cudi records for current debtor. 
 */
int
CheckCudi (void)
{
	open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");
	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	if (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
		return (TRUE);
		
	return (FALSE);
}

/*
 * Load warehouse details into linked list 
 */
void
LoadWarehouseInfo (void)
{
	struct	WH_LIST	*lcl_ptr;

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no,  "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * Store WH details 
		 */
		lcl_ptr = wh_alloc ();

		sprintf (lcl_ptr->br_no,   "%2.2s", ccmr_rec.est_no);
		sprintf (lcl_ptr->wh_no,   "%2.2s", ccmr_rec.cc_no);
		sprintf (lcl_ptr->wh_name, "%-40.40s", ccmr_rec.name);
		lcl_ptr->whPrinterNo = ccmr_rec.lpno;
		lcl_ptr->hhccHash = ccmr_rec.hhcc_hash;
		if (wh_head == WH_NULL)
			wh_head = lcl_ptr;
		else
			wh_tail->next = lcl_ptr;
		wh_tail = lcl_ptr;
	
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	return;
}

int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn != cur_screen || scn == FREIGHT_SCN || scn == MISC_SCN)
		return (FALSE);
	
	if (scn == HEADER_SCN)
	{
		InputSONS (TRUE, 0); 
		restart = FALSE;
		return (TRUE);
	}
	if (store [lin].hhbrHash == 0L)
		return (FALSE);
	
	InputSONS (FALSE, lin); 
	restart = FALSE;
	/*"Customer No : %s (%s)"*/
	print_at (4,0, ML (mlStdMess012), cumr_rec.dbt_no, 
									clip (cumr_rec.dbt_name));
	if (envVar.dbMcurr)
		print_at (4, 65,"    (%-3.3s)", cumr_rec.curr_code);

	return (PSLW);
}

int
win_function2 (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{

	if (scn != ITEM_SCN)
		return (FALSE);

	if (store [lin].hhbrHash == 0L)
		return (FALSE);
	

	/*
	 * Check for contract. 
	 */
	if (store [lin].contractStatus)
	{
		/*
		 * Item has a contract price, negotiation window not available.
		 */
		print_mess (ML (mlSoMess239));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	/*
	 * Disable edit of qty BO field. 
	 */
	negoScn [1].fldEdit = 0;

	/*
	 * Initialise values for negotiation window. 
	 */
	negoRec.qOrd		=	store [lin].qtyOrder;
	negoRec.qBord		=	store [lin].qtyTotal - store [lin].qtyOrder;

	negoRec.regPc			=  	store [lin].regPc;
	negoRec.discArray [0]	=	store [lin].discA;
	negoRec.discArray [1]	=	store [lin].discB;
	negoRec.discArray [2]	=	store [lin].discC;
	negoRec.grossPrice		=	store [lin].gSalePrice;
	negoRec.salePrice		=	store [lin].salePrice;
	negoRec.margCost		= 	store [lin].marginCost;
	negoRec.outer_size		=	store [lin].outerSize;

	NegPrice (2, 8, local_rec.item_no, soln_rec.item_desc, 
				   store [lin].cumulative, scn);

	if (!restart)
	{
		local_rec.qty_ord 		=   negoRec.qOrd;
		local_rec.qty_back 		=   negoRec.qBord;

		store [lin].qtyOrder		=	negoRec.qOrd;
		store [lin].qtyTotal		= 	negoRec.qOrd +
										negoRec.qBord;
		store [lin].regPc		= 	negoRec.regPc;
		store [lin].discA		= 	negoRec.discArray [0];
		store [lin].discB		= 	negoRec.discArray [1];
		store [lin].discC		= 	negoRec.discArray [2];
		store [lin].discPc		=	CalcOneDisc (store [lin].cumulative,
													 negoRec.discArray [0],
													 negoRec.discArray [1],
													 negoRec.discArray [2]);
		store [lin].gSalePrice 	= 	negoRec.grossPrice;
		store [lin].salePrice		=	negoRec.salePrice;
		store [lin].actSalePrice		=	negoRec.salePrice;
		store [lin].marginCost 		=	negoRec.margCost;

		soln_rec.dis_pc  			= 	ScreenDisc (store [lin].discPc);
		soln_rec.sale_price 		= 	store [lin].salePrice;

		if (store [lin].calcSalePrice != soln_rec.sale_price)
			strcpy (store [lin].priceOR, "Y");

		if (store [lin].calcDisc != ScreenDisc (soln_rec.dis_pc))
			strcpy (store [lin].discOR, "Y");

		putval (lin);
	}

	CalculateTotalBox (FALSE, FALSE);
	
	restart = FALSE;
    return (TRUE); /* is this ok? -- vij */
}

int
win_function3 (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	int		sup_frm_alt;
	int		line_sav;
	char	HoldStr [100];

	if (scn != ITEM_SCN)
		return (FALSE);
	
	if (store [lin].hhbrHash == 0L)
		return (FALSE);
	
	line_sav	=	line_cnt;
	line_cnt	=	lin;

	strcpy (HoldStr, temp_str);
	getval (lin);
	sup_frm_alt = ProcessAltSupWh ();
	strcpy (temp_str, HoldStr);

	if (sup_frm_alt)
		putval (lin);
	else
	{	
		line_cnt = line_sav;
		return (FALSE);
	}

	line_cnt = line_sav;
	return (TRUE);
}

/*
 * Process alternate stocking point
 */
int
ProcessAltSupWh (void)
{
	int		i;
	int		wh_fnd;
	int		sup_flg;
	float	wh_avail;
	float	realCommitted;
	struct	WH_LIST	*lcl_ptr;

	putval (line_cnt);

	/*
	 * Open tab_disp table 
	 */
	tab_open ("wh_lst", wh_keys, 8, 1, 7, FALSE);
	tab_add ("wh_lst",
		"# %-5.5s | %-5.5s | %-40.40s | %-11.11s| %-11.11s",
		"BR NO",
		"WH NO",
		"           WAREHOUSE NAME",
		" AVAIL. STK",
		" ON ORDER");

	/*
	 * WARNING only %.2f Available for part # %s. 
	 *  Supply from alternate warehouse ?		 
	 */
	sprintf (err_str, ML (mlSoMess284), 
					SR.qtyAvailable, clip (local_rec.item_no));
	rv_pr (err_str, 2, 19, 1);

	/*
	 * Load tab_disp table 
	 */
	wh_fnd = FALSE;
	lcl_ptr = wh_head;
	while (lcl_ptr != WH_NULL)
	{
		if (PHANTOM)
		{
        	wh_avail	= 	ProcessPhantom
						 	(
								inmr_rec.hhbr_hash,
	    						lcl_ptr->hhccHash
							);
		}
		else
		{
			incc_rec.hhcc_hash = lcl_ptr->hhccHash;
			incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
											  inmr_rec.hhsi_hash);
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
			{
				lcl_ptr = lcl_ptr->next;
				continue;
			}

			realCommitted = RealTimeCommitted 
							(
								inmr_rec.hhbr_hash,
							   	incc_rec.hhcc_hash, 0
							);
			if (envVar.includeForwardStock)
			{
				wh_avail = incc_rec.closing_stock -
						  (incc_rec.committed + realCommitted) -
						   incc_rec.backorder - 
						   incc_rec.forward;
			}
			else
			{
				wh_avail = incc_rec.closing_stock -
						  (incc_rec.committed + realCommitted) -
						   incc_rec.backorder;
			}
			if (envVar.qCApplies && envVar.qCAvailable)
				wh_avail -= incc_rec.qc_qty;
		}
	    if (wh_avail > 0.00 || incc_rec.on_order > 0.00)
	    {
			/*
			 * Don't allow selection of current WH 
			 * as the alternate stocking point.   
			 */
			if (incc_rec.hhcc_hash == local_rec.hhccHash)
			{
				lcl_ptr = lcl_ptr->next;
				continue;
			}
			
		wh_fnd = TRUE;
		tab_add ("wh_lst",
			"   %2.2s  |   %2.2s  | %-40.40s | %9.2f  |  %9.2f           %10ld",
			lcl_ptr->br_no,
			lcl_ptr->wh_no,
			lcl_ptr->wh_name,
			wh_avail,
			incc_rec.on_order,
			lcl_ptr->hhccHash);
	    }

	    lcl_ptr = lcl_ptr->next;
	}

	/*
	 * If there are some valid WHs in    
	 * table then allow user to tag one. 
	 */
	if (wh_fnd == FALSE)
	{
		tab_add ("wh_lst", "%-15.15s%s", " ", ML (mlSoMess387));
		tab_display ("wh_lst", TRUE);
		sleep (sleepTime);
		sprintf (local_rec.sup_br, "%2.2s", alternateBranchWarehouseNo);
		sprintf (local_rec.sup_wh, "%2.2s", alternateBranchWarehouseNo + 2);
		TidyAlternateScreen ();
		/*
		 * WARNING only %.2f Available for part # %s 
		 */
		sprintf (err_str, ML (mlStdMess090), 
							SR.qtyAvailable,
							clip (local_rec.item_no));
		print_mess (err_str);
	}
	else
	{
	    exit_alt = FALSE;
	    while (!exit_alt)
	    {
			alternateHhccHash = 0L;
			tab_scan ("wh_lst");
	
			/*
			 * No alternate stocking point selected 
			 */
			if (alternateHhccHash == 0L)
			{
		    	/*
		    	 * Read the original incc. 
		    	 */
		    	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		    	incc_rec.hhbr_hash = alt_hash 
										(
											inmr_rec.hhbr_hash,
											inmr_rec.hhsi_hash
										);
		    	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		    	if (cc)
					file_err (cc, incc, "DBFIND");

		    	/*
		    	 * Close Table And Redraw screen 
				 */
		    	TidyAlternateScreen ();

		    	return (FALSE);
			}

			if (PHANTOM)
			{
        		wh_avail	= 	ProcessPhantom
							 	(
									inmr_rec.hhbr_hash,
	    							alternateHhccHash
								);
			}
			else
			{
				/*
				 * Process selected WH 
				 */
				incc_rec.hhcc_hash = alternateHhccHash;
				incc_rec.hhbr_hash = alt_hash 
										(
											inmr_rec.hhbr_hash,
											inmr_rec.hhsi_hash
										);
				cc = find_rec (incc, &incc_rec, COMPARISON, "r");
				if (cc)
		    		file_err (cc, incc, "DBFIND");
	
				realCommitted = RealTimeCommitted 
								(
									inmr_rec.hhbr_hash,
									incc_rec.hhcc_hash, 0
								);
				if (envVar.includeForwardStock)
				{
		    		wh_avail = incc_rec.closing_stock -
			       				(incc_rec.committed + realCommitted) -
			       				incc_rec.backorder - 
			       				incc_rec.forward;
				}
				else
				{
		    		wh_avail = incc_rec.closing_stock -
			       				(incc_rec.committed + realCommitted) -
			       				incc_rec.backorder;
				}
				if (envVar.qCApplies && envVar.qCAvailable)
					wh_avail -= incc_rec.qc_qty;
			}
			if (wh_avail < ToStdUom (local_rec.qty_ord))
			{
		    	sup_flg = AlternateInputResponce (wh_avail);
		    	if (sup_flg & CANCEL)
					continue;

		    	if ((sup_flg & REDUCE) || (sup_flg & OVERRIDE))
					break;

		    	if ((sup_flg & F_SUPP) &&
			 		sohr_rec.full_supply [0] == 'N' &&
			 		envVar.fullSupplyOrder &&
					local_rec.schedOrder [0] == 'N')
		    	{
					sprintf (err_str, "%s %s ? ", ML (mlSoMess386), 
												 cumr_rec.dbt_no);
			
					i = prmptmsg (err_str,"YyNn",1,2);
					BusyFunction (FALSE);
					if (i == 'Y' || i == 'y')
			    		strcpy (sohr_rec.full_supply, "Y");
					else
			    		strcpy (sohr_rec.full_supply, "N");
		    	}
		    	break;
			}
			else
		    	break;
	    }

	    /*
	     * Set live level values for selected WH 
	     */
	    local_rec.hhccHash = alternateHhccHash;
	    sprintf (local_rec.sup_br, "%2.2s", alternateBranchWarehouseNo);
	    sprintf (local_rec.sup_wh, "%2.2s", alternateBranchWarehouseNo + 2);
	    SR.hhccHash = alternateHhccHash;
	    SR.qtyAvailable = wh_avail;
	    if (inmr_rec.serial_item [0] == 'Y')
			SR.hhwhHash = incc_rec.hhwh_hash;

		SR.actHhwhHash = incc_rec.hhwh_hash;
	    putval (line_cnt);

	    /*
	     * Close Table And Redraw screen 
	     */
	    TidyAlternateScreen ();

	    return (TRUE);
	}

	/*
	 * Make sure correct incc is read 
	 */
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash 
							(
								inmr_rec.hhbr_hash,
								inmr_rec.hhsi_hash
							);
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	    file_err (cc, incc, "DBFIND");

	local_rec.hhccHash = ccmr_rec.hhcc_hash;
	sprintf (local_rec.sup_br, "%2.2s", ccmr_rec.est_no);
	sprintf (local_rec.sup_wh, "%2.2s", ccmr_rec.cc_no);
	SR.hhccHash = ccmr_rec.hhcc_hash;
	return (FALSE);
}
static	int		
BrDispFunc (
 int c, 
 KEY_TAB *psUnused)
{
	char	get_buf [200];

	tab_get ("wh_lst", get_buf, CURRENT, 0);
	alternateHhccHash = atol (get_buf + 94);

#ifdef GVISION
	DisplayStockWindow (SR.hhsiHash, 0L);
#else
	/*
	 * Display Branch Status 
	 */
	if (!wpipe_open)
	{
		if (OpenStockWindow ())
			return (c);
	}
	fprintf (pout,"%10ld%10ld\n", SR.hhsiHash, 0L);

	fflush (pout);
	IP_READ (namesPipeFileName);
#endif	/* GVISION */

	crsr_off ();
	PauseForKey (23, 50,ML (mlStdMess042), 0);

#ifdef GVISION
	HideStockWindow ();
#else
	ClearWindow ();
#endif	/* GVISION */

	tab_display ("wh_lst", TRUE);
	redraw_keys ("wh_lst");
	alternateHhccHash = 0L;

	return (c);
}

static	int		
WhDispFunc (
 int c, 
 KEY_TAB *psUnused)
{
	char	get_buf [200];

	tab_get ("wh_lst", get_buf, CURRENT, 0);
	alternateHhccHash = atol (get_buf + 94);

#ifdef GVISION
	DisplayStockWindow (SR.hhsiHash, alternateHhccHash);
#else
	/*
	 * Display Warehouse Status 
	 */
	if (!wpipe_open)
	{
		if (OpenStockWindow ())
			return (c);
	}
	fprintf (pout,"%10ld%10ld\n", SR.hhsiHash, alternateHhccHash);

	fflush (pout);
	IP_READ (namesPipeFileName);
#endif	/* GVISION */

	crsr_off ();
	PauseForKey (23, 50, ML (mlStdMess042), 0);

#ifdef GVISION
	HideStockWindow ();
#else
	ClearWindow ();
#endif	/* GVISION */

	tab_display ("wh_lst", TRUE);
	redraw_keys ("wh_lst");

	alternateHhccHash = 0L;
	return (c);
}

static	int		
SelectFunc (
 int c, 
 KEY_TAB *psUnused)
{
	char	get_buf [200];

	tab_get ("wh_lst", get_buf, CURRENT, 0);
	alternateHhccHash = atol (get_buf + 94);
	sprintf (alternateBranchWarehouseNo, "%2.2s%2.2s",get_buf + 3,get_buf + 11);

	return (FN16);
}

static	int		
ExitFunc (
 int c, 
 KEY_TAB *psUnused)
{
	exit_alt = TRUE;
	return (c);
}

/*
 * Tidy up alternate supply screen 
 */
void
TidyAlternateScreen (void)
{
	int		sav_lcount;

	/*
	 * Close Table And Redraw screen 
	 */
	tab_close ("wh_lst", TRUE);
	blank_at (19, 0, 130);
	scn_write (ITEM_SCN);
	sav_lcount = lcount [ITEM_SCN];
	lcount [ITEM_SCN] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [ITEM_SCN];
	scn_display (ITEM_SCN);
	lcount [ITEM_SCN] = sav_lcount;
}

/*
 * Tidy up alternate supply screen 
 */
void
TidySonsScreen (
 void)
{
	int		i;

	for (i = 0; i < 4 ; i++)
		print_at (i+2,0, "%90.90s", ns_space);
	
	/*
	 * NOTE : Active Keys [Window #1], [Window #2] & [Window #3] 
	 */
	print_at (3,0, ML (mlSoMess281));

	print_at (4,0, ML (mlStdMess012),
						cumr_rec.dbt_no,
						clip (cumr_rec.dbt_name));
	if (envVar.dbMcurr)
		print_at (4,65,"    (%-3.3s)", cumr_rec.curr_code);
	
}

/*
 * Input responses to stock quantity on hand less-than input quantity. 
 */
int
AlternateInputResponce (
  float	wh_avail)
{
	int		i;
	int		fs_flag 	= FALSE;
	int		displayed 	= FALSE;
	int		retrn_flg	= FALSE;
	char	val_keys [300];
	char	disp_str [300];

	cc = 0;
	
	if (local_rec.schedOrder [0] == 'Y' && inmr_rec.bo_flag [0] == 'N')
	{
		/*
		 * %s (C)ancel%s  %s (R)educe%s  
		 */
		strcpy (val_keys, "CcRr");
		sprintf (disp_str, ML (mlSoMess379),
				ta [8], ta [9], ta [8], ta [9]);
	}
	else
	{
		/*
		 * %s (O)verride%s  %s (C)ancel%s  %s (R)educe%s  
		 */

		if (envVar.overrideQuantity [0] == 'Y')
		{
			strcpy (val_keys, "OoCcRr");
			sprintf (disp_str, ML (mlSoMess380),ta [8],ta [9],ta [8],ta [9],ta [8],ta [9]);
		}
		else
		{
			strcpy (val_keys, "CcRr");
			sprintf (disp_str, ML (mlSoMess379),ta [8], ta [9], ta [8], ta [9]);

		}
	}
	if (BO_OK)
	{
		if (FULL_BO)
		{
			/*
			 * %s (F)orce b/o%s  
			 */
			sprintf (err_str, ML (mlSoMess381), ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "Ff");
		}
		else
		{
			/*
			 * %s (B)ackorder bal%s  %s (F)orce b/o%s 
			 */
			sprintf (err_str, ML (mlSoMess382), ta [8], ta [9], ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "BbFf");
		}
	}

	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, 2);

		BusyFunction (FALSE);
		switch (i)
		{
		/*
		 * Accept Quantity input. 
		 */
		case	'O':
		case	'o':
			retrn_flg = OVERRIDE;
			break;

		case	'B':
		case	'b':
			local_rec.qty_back 	= local_rec.qty_ord;
			local_rec.qty_ord 	= ToLclUom (wh_avail);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			local_rec.qty_back -= local_rec.qty_ord;
			fs_flag = TRUE;
			retrn_flg = B_ORDER;
			break;

		/*
		 * Cancel Quantity input and check if log to lost sale. 
		 */
		case	'C':
		case	'c':
			retrn_flg = CANCEL;
			break;

		/*
		 * Quantity has been reduced to equal quantity on hand. 
		 */
		case	'R':
		case	'r':
			SR.qtyOrigional = local_rec.qty_ord;
			local_rec.qty_ord = ToLclUom (wh_avail);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			retrn_flg = REDUCE;
			break;

		case	'F':
		case	'f':
			local_rec.qty_back = local_rec.qty_ord;
			local_rec.qty_ord = 0.00;
			fs_flag = TRUE;
			retrn_flg = FORCE_BO;
			break;
		}
		print_at (2,1,"%90.90s"," ");

		if (i != 'D' && i != 'd')
			break;
	}

	if (displayed)
		ClearWindow ();

	if (!envVar.fullSupplyOrder)
		fs_flag = FALSE;

	return (retrn_flg | fs_flag);
}

int
LoadDisplay (
 char	*_run_string)
{
	long	hhcuHash;
	char	orderNo [9];

	hhcuHash = atol (_run_string + 8);
	sprintf (orderNo, "%-8.8s", _run_string + 19);

	abc_selfield (cumr, "cumr_hhcu_hash");

	cumr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		abc_selfield (cumr, (envVar.dbFind) ? "cumr_id_no3" : "cumr_id_no");
		return (EXIT_FAILURE);
	}
	strcpy (local_rec.customerNo, cumr_rec.dbt_no);
	/*
	 * Check if invoice is on file. 
	 */
	sprintf (sohr_rec.co_no, "%-2.2s", _run_string);
	sprintf (sohr_rec.br_no, "%-2.2s", _run_string + 3);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (sohr_rec.order_no, zero_pad (orderNo, 8));
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
	if (cc)
		return (EXIT_FAILURE);

	strcpy (local_rec.schedOrder, sohr_rec.sch_ord);

	if (LoadItemScreen (sohr_rec.hhso_hash))
		return (EXIT_FAILURE);

	strcpy (local_rec.spinst [0],sohr_rec.din_1);
	strcpy (local_rec.spinst [1],sohr_rec.din_2);
	strcpy (local_rec.spinst [2],sohr_rec.din_3);
	newOrder = 0;
	entry_exit = 1;

	SetOrderDefaults (newOrder);

	return (EXIT_SUCCESS);
}
/*
 * Main validation Routine for item number. 
 */
int
ValidateItemNumber (
 int	getField)
{
	int		i;
	int		itemChanged = FALSE;
	long	orig_hhbr_hash;
	long	hhccHash;
	float	realCommitted;
	char	*sptr;
	char	sav_sup_br [3];
	char	sav_sup_wh [3];

	abc_selfield (inmr,"inmr_id_no");

	hhccHash = ccmr_rec.hhcc_hash;
	skip_entry = 0;

	if (prog_status == ENTRY)
		strcpy (local_rec.serialNo, serialSpace);

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.item_no);

	SR.bonusFlag [0] = (CheckBonusItem (inmr_rec.item_no)) ? 'Y' : 'N';
	soln_rec.hide_flag [0] = (CheckHidden (inmr_rec.item_no)) ? 'Y' : 'N';

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
		cc	=	check_indent 
				(
					comm_rec.co_no, 
					comm_rec.est_no,
					hhccHash,
					inmr_rec.item_no
				);
		if (cc)
		{
			/*
			 * Item not found. 
			 */
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
				/*
				 * Item not found. 
				 */
				errmess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			/*
			 * Customer No : %s (%s)
			 */
			print_at (4,0, ML (mlStdMess012),
								cumr_rec.dbt_no,
								clip (cumr_rec.dbt_name));
			if (envVar.dbMcurr)
				print_at (4,65,"    (%-3.3s)", cumr_rec.curr_code);
			BusyFunction (FALSE);
		}
	}
	orig_hhbr_hash	=	inmr_rec.hhbr_hash;

	if (inmr_rec.inmr_class [0] == 'Z' && line_cnt < 1)
	{
		/*
		 * Sorry, You CANNOT begin a Sales order with a comment line!!
		 */
		print_mess (ML (mlSoMess107));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (inmr_rec.inmr_class [0] == 'Z' && line_cnt > 0)
	{
		hhccHash = store [line_cnt - 1].hhccHash;
		putval (-1);
		getval (line_cnt - 1);
		strcpy (sav_sup_br, local_rec.sup_br);
		strcpy (sav_sup_wh, local_rec.sup_wh);
		getval (-1);
	}

	SuperSynonymError ();

	sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);

	strcpy (SR._desc2, inmr_rec.description2);

	/*
	 * Set stocking point to current WH 
	 */
	SR.hhccHash = hhccHash;
	local_rec.hhccHash = hhccHash;
	strcpy (local_rec.sup_br, comm_rec.est_no);
	strcpy (local_rec.sup_wh, comm_rec.cc_no);
	if (inmr_rec.inmr_class [0] == 'Z' && line_cnt > 0)
	{
		strcpy (local_rec.sup_br, sav_sup_br);
		strcpy (local_rec.sup_wh, sav_sup_wh);
	}

	/*
	 * Item changed. 
	 */
	if (prog_status   != ENTRY &&
		SR.hhbrHash != inmr_rec.hhbr_hash &&
		SR.hhbrHash != 0)
	{
		if (inmr_rec.inmr_class [0] == 'K')
		{
			/*
			 * Cannot substitute Kits for standard items. 
			 */
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		itemChanged = TRUE;
		local_rec.qty_ord = 0.00;
		if (strcmp (SR.serialNo, serialSpace) &&
			inmr_rec.serial_item [0] == 'Y')
		{
			cc = UpdateInsf (SR.hhwhHash, SR.hhsiHash, SR.serialNo, "C", "F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
			/*
			 * Because we have freed the insf record we must
			 * blank the serial number field on the soln in 
			 * case of a restart.                           
			 */
			if (SR.hhslHash > 0)
			{
				soln_rec.hhsl_hash	=	SR.hhslHash;
				cc = find_rec (soln2, &soln_rec, EQUAL, "u");
				if (!cc)
				{
					strcpy (soln_rec.serial_no, serialSpace);
					cc = abc_update (soln2, &soln_rec);
					if (cc)
						file_err (cc, soln2, "DBUPDATE");
				}
			}
		}
		strcpy (local_rec.serialNo, serialSpace);
		strcpy (SR.serialNo, local_rec.serialNo);
		strcpy (SR.orgSerialNo, local_rec.serialNo);
	}

	SR.hhbrHash	=	inmr_rec.hhbr_hash;
	SR.hhsiHash 	= 	alt_hash 
						(
							inmr_rec.hhbr_hash, 
							inmr_rec.hhsi_hash
						);
	SR.weight 		= 	inmr_rec.weight;
	SR.outerSize 		= 	inmr_rec.outer_size;
	SR.defaultDisc 	= 	inmr_rec.disc_pc;
	SR.itemClass [0] 	= 	inmr_rec.inmr_class [0];
	strcpy (SR.source, 	inmr_rec.source);
	strcpy (SR.category,	inmr_rec.category);
	strcpy (SR.sellGroup,	inmr_rec.sellgrp);
	strcpy (SR.backorderFlag,		inmr_rec.bo_flag);
	strcpy (SR.releaseFlag,	inmr_rec.bo_release);
	strcpy (SR.packSize,	inmr_rec.pack_size);
	strcpy (local_rec.UOM,	inmr_rec.sale_unit);
	strcpy (SR.costingFlag,	inmr_rec.costing_flag);
	strcpy (SR.priceOR,"N");
	strcpy (SR.discOR,"N");

	/*
	 * Find for UOM GROUP. 
	 */
	strcpy (inum_rec.uom, inmr_rec.sale_unit);
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");
	
	SR.hhumHash 		= inum_rec.hhum_hash;
	SR.cnvFct			= inum_rec.cnv_fct;
	SR.stdCnvFct		= inum_rec.cnv_fct;

	strcpy (SR.uom,		inum_rec.uom);
	strcpy (SR.uomGroup,	inum_rec.uom_group);
	
	/*
	 * Check for Indent items. 
	 */
	if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
		SR.indent = FALSE;
	else
		SR.indent = TRUE;

	if (FLD ("pack_size") != ND)
		FLD ("pack_size") = (!strcmp (inmr_rec.pack_size,"     ")) 
								? YES : NI;

	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,SR.category);
	cc = find_rec (excf,&excf_rec,COMPARISON,"r");
	if (cc)
	{
		/*
		 * Cannot find Category %s on file 
		 */
		print_mess (ML (mlStdMess004));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	SR.minMargin = excf_rec.min_marg;
	SR.exchRate  = excf_rec.ex_rate;

	incc_rec.hhcc_hash 	= 	hhccHash;
	incc_rec.hhbr_hash	= 	alt_hash 
								(
									inmr_rec.hhbr_hash,
									inmr_rec.hhsi_hash
								);

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc) 
	{
		/*
		 * Item %s is not on file at Warehouse - create ? 
		 */
		i = prmptmsg (ML (mlStdMess033),"YyNn",1,2);
		BusyFunction (FALSE);
		if (i == 'n' || i == 'N') 
		{
			skip_entry = -1 ;
			return (EXIT_SUCCESS); 
		}
		else 
		{
			cc = AddIncc (incc_rec.hhcc_hash, incc_rec.hhbr_hash);
			if (cc)
				file_err (cc, incc, "DBADD");

			SR.qtyAvailable = 0.00;
		}
	}
	inei_rec.hhbr_hash =	alt_hash 
							(
								inmr_rec.hhbr_hash,
							  	inmr_rec.hhsi_hash
							);

	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei,&inei_rec,COMPARISON,"r");
	if (cc)
		cc = AddInei ();

	if (cc)
		file_err (cc, inei, "DBFIND");

	SR.marginCost = (inei_rec.avge_cost == 0.00) ? inei_rec.last_cost : inei_rec.avge_cost;

	if (inmr_rec.hhbr_hash != orig_hhbr_hash) 
	{
		BusyFunction (FALSE);
		/*
		 *"Item No. %s Has Been Superceded by Item No. %s %c 
		 */
		sprintf (err_str, ML (mlSoMess234),
			clip (local_rec.supplierPart),clip (local_rec.item_no),BELL);
		print_mess (err_str);
		sleep (sleepTime);
	}
	strcpy (soln_rec.item_desc,inmr_rec.description);

	SR.actHhwhHash = incc_rec.hhwh_hash;

	/*
	 * Item is a serial item. 
	 */
	if (inmr_rec.serial_item [0] == 'Y')
	{
		SR.hhwhHash = incc_rec.hhwh_hash;
		if (FLD ("ser_no") == NA)
			FLD ("ser_no") = YES;
	}
	else
	{
		SR.hhwhHash = -1L;
		if (NEED (label ("ser_no")))
			FLD ("ser_no") = NA;
	}

	/*
	 * Process soic. 
	 */
	ProcSoic (FALSE, line_cnt);

	if (itemChanged)
	{
		local_rec.qty_ord = 0.00;
		SR.qtyOrder = local_rec.qty_ord;
		local_rec.qty_back = 0.00;
		SR.qtyTotal = ToStdUom (local_rec.qty_ord);
		SR.qtyTotal += ToStdUom (local_rec.qty_back);
		DSP_FLD ("qty_ord");
		DSP_FLD ("qty_back");
		PriceProcess (TRUE);
		DiscProcess (TRUE);
		CalculateTotalBox (FALSE, FALSE);
	}

	if (!BONUS)
	{
		SR.taxAmt = inmr_rec.tax_amount;
		soln_rec.tax_pc  = inmr_rec.tax_pc;
		soln_rec.gst_pc  = inmr_rec.gst_pc;
	}
	else
	{
		sprintf (soBonus, "%-2.2s", envVar.soSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no,"%-s%-.*s", sptr,16 - (int) strlen (sptr),soBonus);
		SR.taxAmt = 0.00;
		soln_rec.tax_pc  = 0.00;
		soln_rec.gst_pc  = 0.00;
	}

	SR.taxPc = soln_rec.tax_pc;
	SR.gstPc = soln_rec.gst_pc;
	strcpy (soln_rec.pack_size,inmr_rec.pack_size);

	if (envVar.advertLevy)
	{
		ItemLevy 
		(	
			inmr_rec.hhbr_hash,
			comm_rec.est_no, 
			cumr_rec.curr_code, 
			comm_rec.dbt_date
		);
		LSR.advertLevyAmt = DPP (inal_rec.value * 100);
		SR.advertLevyPc  = (double) inal_rec.percent;
	}
	else
	{
		LSR.advertLevyAmt = 0.00;
		LSR.advertLevyPc  = 0.00;
	}

	DSP_FLD ("item_no");
	DSP_FLD ("descr");
	DSP_FLD ("pack_size");
	DSP_FLD ("UOM");
	if (local_rec.qty_ord + local_rec.qty_back > 0.00)
	{
		DSP_FLD ("sale_price");
		DSP_FLD ("disc");
	}

	if (!NON_STOCK)
	{
		SR.costPrice = 0.00;
		soln_rec.cost_price = 0.00;
		DSP_FLD ("cost_price");
	}

	realCommitted =	RealTimeCommitted 
					(
						inmr_rec.hhbr_hash, 
						hhccHash, 0
					);
	if (envVar.includeForwardStock)
	{
		SR.qtyAvailable = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder - 
						incc_rec.forward;
	}
	else
	{
		SR.qtyAvailable = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder;
	}
	if (envVar.qCApplies && envVar.qCAvailable)
		SR.qtyAvailable -= incc_rec.qc_qty;

	if (itemChanged && getField)
	{
		do
		{
			get_entry (label ("qty_ord"));
			cc = spec_valid (label ("qty_ord"));
		} while (cc && !restart);
		DSP_FLD ("qty_ord");

		/*
		 * Update soic quantity. 
		 */
		UpdSoicQty ();
	}

	if (NO_KEY (label ("descr")))
		skip_entry++;
	DSP_FLD ("hide");

	FLD ("disc") 		= orig_disc;
	FLD ("sale_price") 	= orig_sale;
	if (soln_rec.due_date == 0L)
		soln_rec.due_date = sohr_rec.dt_required;

	return (EXIT_SUCCESS);
}

void
PriceProcess (
	int		displayPrompt)
{
	int		pType;
	float	regPc;
	double	grossPrice;

	SR.pricingCheck	= FALSE;

	if (BONUS)
	{
		soln_rec.sale_price 	= 0.00;
		SR.actSalePrice 			= 0.00;
		SR.calcSalePrice			= 0.00;
		SR.salePrice 			= 0.00;

		soln_rec.dis_pc  		= 0.00;
		SR.discPc 	 			= 0.00;
		SR.calcDisc 			= 0.00;
		if (displayPrompt)
		{
			DSP_FLD ("sale_price");
			DSP_FLD ("disc");
		}
		return;
	}
	pType = atoi (sohr_rec.pri_type);

	itemTotal	=	TotalItemQty
					(
						SR.hhbrHash,
						line_cnt,
						ToStdUom (local_rec.qty_ord + local_rec.qty_back)
					);

	grossPrice = GetCusPrice 
				(	
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					sohr_rec.area_code,
					cumr_rec.class_type,
					SR.sellGroup,
					cumr_rec.curr_code,
					pType,
					cumr_rec.disc_code,
					cnch_rec.exch_type,
					cumr_rec.hhcu_hash,
					SR.hhccHash,
					SR.hhbrHash,
					SR.category,
					cnch_rec.hhch_hash,
					(envVar.useSystemDate) ? local_rec.lsystemDate : comm_rec.dbt_date,
					itemTotal,
					pocrRec.ex1_factor,
					FGN_CURR,
					&regPc
				);

	SR.pricingCheck	= TRUE;
	SR.calcSalePrice = GetCusGprice (grossPrice, regPc) * SR.cnvFct;

	if (SR.priceOR [0] == 'N')
	{
		SR.gSalePrice 	= 	grossPrice * SR.cnvFct;
		SR.salePrice 		=	SR.calcSalePrice;
		SR.regPc 			= 	regPc;
		soln_rec.sale_price = 	SR.calcSalePrice;
		SR.actSalePrice 		= 	SR.calcSalePrice;
	}

	SR.contractPrice 		= (_CON_PRICE) ? TRUE : FALSE;
	SR.contractStatus  	= _cont_status;
	if (displayPrompt)
		DSP_FLD ("sale_price");
}

void
DiscProcess (
	int		displayPrompt)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*
	 * Discount does not apply. 
	 */
	if (SR.contractStatus == 2 || SR.contractPrice || SR.indent)
	{
		soln_rec.dis_pc  	= 0.00;
		SR.discPc 	 		= 0.00;
		SR.calcDisc 		= 0.00;
		SR.discA			= 0.00;
		SR.discB			= 0.00;
		SR.discC			= 0.00;
		if (displayPrompt)
			DSP_FLD ("disc");
		return;
	}

	if (SR.pricingCheck == FALSE)
		PriceProcess (displayPrompt);

	pType = atoi (sohr_rec.pri_type);
	itemTotal	=	TotalItemQty
					(
						SR.hhbrHash,
						line_cnt,
						ToStdUom (local_rec.qty_ord + local_rec.qty_back)
					);
	cumDisc	=	GetCusDisc 
				(
					comm_rec.co_no,
					comm_rec.est_no,
					SR.hhccHash,
					cumr_rec.hhcu_hash,
					cumr_rec.class_type,
					cumr_rec.disc_code,
					SR.hhsiHash,
					SR.category,
					SR.sellGroup,
					pType,
					SR.gSalePrice,
					SR.regPc,
					itemTotal,
					discArray
				);
				
	if (SR.discOR [0] == 'Y')
	{
		if (displayPrompt)
		DSP_FLD ("disc");
		return;
	}
	SR.calcDisc	=	CalcOneDisc 
						(
							cumDisc,
							discArray [0],
							discArray [1],
							discArray [2]
						);
	if (SR.discOR [0] == 'N')
	{
		soln_rec.dis_pc 	=	ScreenDisc (SR.calcDisc);
		SR.discPc			=	SR.calcDisc;

		SR.discA 			= 	discArray [0];
		SR.discB 			= 	discArray [1];
		SR.discC 			= 	discArray [2];
		SR.cumulative 		= 	cumDisc;

		if (SR.defaultDisc > ScreenDisc (soln_rec.dis_pc) && 
			SR.defaultDisc != 0.0)
		{
			soln_rec.dis_pc = 	ScreenDisc (SR.defaultDisc);
			SR.calcDisc	=	SR.defaultDisc;
			SR.discPc		=	SR.defaultDisc;
			SR.discA 		= 	SR.defaultDisc;
			SR.discB 		= 	0.00;
			SR.discC 		= 	0.00;
		}
	}
	if (displayPrompt)
		DSP_FLD ("disc");
}

int
AddInei (
 void)
{
	inei_rec.avge_cost = 0.00;
	inei_rec.last_cost = 0.00;
	strcpy (inei_rec.stat_flag,"0");
	cc = abc_add (inei,&inei_rec);
	if (cc)
		file_err (cc, inei, "DBADD");

	inei_rec.hhbr_hash	=	alt_hash 
								(
									inmr_rec.hhbr_hash,
					  				inmr_rec.hhsi_hash
								);
	strcpy (inei_rec.est_no, comm_rec.est_no);
	return (find_rec (inei,&inei_rec,COMPARISON,"r"));
}

/*
 * Check Whether A Serial Number For This Item Number
 * Has Already Been Used.				            
 * Return 1 if duplicate					       
 */
int
CheckDuplicateSerial (
 char	*serialNo,
 long	hhbrHash,
 int	lineNumber)
{
	int		i;
	int		noItems = (prog_status == ENTRY) ? line_cnt : lcount [ITEM_SCN];

	for (i = 0;i < noItems;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == lineNumber)
			continue;

		/*
		 * cannot duplicate item_no/serial_no unless serial no was not input 
		 */
		if (!strcmp (store [i].serialNo, serialSpace))
			continue;

		/*
		 * Only compare serial numbers for the same item number 
		 */
		if (store [i].hhsiHash == hhbrHash)
		{
			if (!strcmp (store [i].serialNo, serialNo))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Delete line. 
 */
int
DeleteLine (
 int	dispLine)
{
	int		i, j;
	int		this_page;
	int		delta;
	int		is_comment;

	if (prog_status == ENTRY)
	{
		/*
		 * Cannot Delete Lines on Entry 
		 */
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [ITEM_SCN] == 0)
		return (EXIT_SUCCESS);

	LogLostSales (ToStdUom (local_rec.qty_ord));

	/*
	 * Free insf record if a serial item 
	 */
	if (strcmp (SR.serialNo, serialSpace))
	{
		cc = UpdateInsf (SR.hhwhHash, SR.hhsiHash, SR.serialNo, "C", "F");

		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}

	/*
	 * Because we have freed the insf record we must 
	 * blank the serial number field on the soln in 
	 * case of a restart.                          
	 */
	if (SR.hhslHash > 0)
	{
		soln_rec.hhsl_hash	=	SR.hhslHash;
		cc = find_rec (soln2, &soln_rec, EQUAL, "u");
		if (!cc)
		{
			strcpy (soln_rec.serial_no, serialSpace);
			cc = abc_update (soln2, &soln_rec);
			if (cc)
				file_err (cc, soln2, "DBUPDATE");
		}
	}

	this_page = line_cnt / TABLINES;

	is_comment = (SR.itemClass [0] == 'Z') ? TRUE : FALSE;
	delta = 1;
	while (!is_comment && store [line_cnt + delta].itemClass [0] == 'Z' && line_cnt + delta < lcount [ITEM_SCN])
		delta++;

	add_hash_RC (comm_rec.co_no,
				 comm_rec.est_no,
				 SR.hhsiHash,
				 SR.hhccHash,
				 progPid,
				 nextSoicRef);

	/*
	 * Process soic record for line. 
	 */
	ProcSoic (TRUE, line_cnt);

	for (i = line_cnt;line_cnt < lcount [ITEM_SCN] - delta;line_cnt++)
	{
		getval (line_cnt + delta);
		local_rec.line_no	=	line_cnt + 1;
		putval (line_cnt);

		memcpy ((char *) &SR, 
				  (char *) &store [line_cnt + delta],
				   sizeof (struct storeRec));

		/*
		 * Move lot information. 
		 */
		LotMove (line_cnt, line_cnt + 1);

		if (dispLine)
		{
			if (this_page == line_cnt / TABLINES)
				line_display ();
		}
	}

	while (line_cnt < lcount [ITEM_SCN])
	{
		sprintf (local_rec.item_no,"%-16.16s"," ");
		sprintf (soln_rec.item_desc,"%-40.40s"," ");
		sprintf (soln_rec.hide_flag,"%-1.1s"," ");
		local_rec.qty_ord 		= 0.00;
		local_rec.qty_back 		= 0.00;
		soln_rec.cost_price 	= 0.00;
		soln_rec.item_levy 		= 0.00;
		soln_rec.sale_price 	= 0.00;
		soln_rec.dis_pc 		= 0.00;
		soln_rec.tax_pc 		= 0.00;
		soln_rec.due_date 		= 0L;
		local_rec.hhccHash 	= 0L;
		strcpy (local_rec.serialNo, serialSpace);
		strcpy (local_rec.sup_br, "  ");
		strcpy (local_rec.sup_wh, "  ");
		putval (line_cnt);

		if (dispLine)
		{
			if (this_page == line_cnt / TABLINES)
				blank_display ();
		}

		line_cnt++;
	}

	/*
	 * Clear data left at end of tabular screen. 
	 */
	for (j = lcount [ITEM_SCN] - delta; j < lcount [ITEM_SCN]; j++)
	{

		memset ((char *) &store [j], '\0', sizeof (struct storeRec));

		store [j].canEdit	 	= TRUE;

		strcpy (store [j].serialNo, serialSpace);
		strcpy (store [j].orgSerialNo, serialSpace);
		strcpy (store [j]._desc2, ns_space);
		strcpy (store [j].category, "           ");
		strcpy (store [j].status, " ");
		strcpy (store [j].sellGroup, "      ");
		strcpy (store [j].bonusFlag, " ");
		strcpy (store [j].itemClass, " ");
		strcpy (store [j].source, " ");
		strcpy (store [j].backorderFlag, " ");
		strcpy (store [j].releaseFlag, " ");
		strcpy (store [j].packSize, "     ");
		strcpy (store [j].costingFlag, " ");
		strcpy (store [j].priceOR, " ");
		strcpy (store [j].discOR, " ");
		strcpy (store [j].marginFailed, " ");

		/*
		 * Clear out lot information. 
		 */
		LotClear (j);
	}

	if (lcount [ITEM_SCN] > 0)
		lcount [ITEM_SCN] -= delta;

	line_cnt = i;
	getval (line_cnt);
	CalculateTotalBox (FALSE, FALSE);
	return (EXIT_SUCCESS);
}
/*
 * Calculate Defaults for Levy.
 */
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

	sohr_rec.no_kgs		= 0.00;
	est_freight 		= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	for (i = 0;i < lcount [ITEM_SCN];i++)
	{
		weight = (store [i].weight > 0.00) ? store [i].weight 
										     : comr_rec.frt_mweight;
		totalKgs += (weight * store [i].qtyOrder);
	}
	/*
	 * Cost by Kg by Carrier / Zone. 
	 */
	if (envVar.soFreightCharge == 3)
		freightValue = (double) totalKgs * carrierCostKg;

	/*
	 * Cost by Kg by Zone. 
	 */
	if (envVar.soFreightCharge == 2)
		freightValue = (double) totalKgs * zoneCostKg;

	calcMarkup = (double) freightMarkup;
	calcMarkup *= freightValue;
	calcMarkup = DOLLARS (calcMarkup);
	
	freightValue += calcMarkup;
	freightValue = twodec (CENTS (freightValue));

	if (freightValue < comr_rec.frt_min_amt && freightValue > 0.00)
		est_freight = comr_rec.frt_min_amt;
	else
		est_freight = freightValue;
	
	if (FREIGHT_CHG && envVar.automaticFreight)
	{
		if (envVar.soFreightCharge == 1)
		{
			sohr_rec.freight = CENTS (zoneFixedAmount);
			est_freight 	 = CENTS (zoneFixedAmount);
		}
		else
			sohr_rec.freight = est_freight;
	}
	sohr_rec.no_kgs = totalKgs;

	if (inScreenFlag)
		PrintBoxTotals ();

	return;
}

/*
 * Calculate totals. 
 */
void
CalculateTotalBox (
 int	override,
 int	clear_disc)
{
	int		i;
	int		no_lines = 0;
	double	other = 0.00;
	double	wk_gst = 0.00;
	float	gst_pc = 0.00;
	float	gst_div = 0.00;
	double	diff = 0.00;
	double	gst_amount = 0.00;

	no_lines = ((line_cnt > lcount [ITEM_SCN] - 1)) ? line_cnt : lcount [ITEM_SCN];
	inv_tot = 0.00;
	dis_tot = 0.00;
	tax_tot = 0.00;
	gst_tot = 0.00;
	lev_tot = 0.00;
	tot_tot = 0.00;

	if (override)
		scn_set (ITEM_SCN);

	for (i = 0;i <= no_lines;i++) 
	{
		if (override)
		{
			getval (i);
			
			if (clear_disc)
			{
				soln_rec.dis_pc = 0.00;
				store [i].discPc = 0.00;
			}
			else
			{
				soln_rec.dis_pc += ScreenDisc (local_rec.discOverride);
				store [i].discPc = ScreenDisc (soln_rec.dis_pc);
			}
		}
		
		CalcExtendedTotal (i);

		store [i].itemLevy	=	l_levy;

		if (override)
			putval (i);

		inv_tot += l_total;
		dis_tot += l_disc;
		tax_tot += l_tax;
		gst_tot += l_gst;
		lev_tot += l_levy;
	}

	gst_tot = no_dec (gst_tot);

	if (override && clear_disc)	
		sohr_rec.other_cost_1 = 0.00;

	CalculateFreight 
	(
		trcm_rec.markup_pc, 
		trcl_rec.cost_kg,
		trzm_rec.chg_kg,
		trzm_rec.dflt_chg
	);
	other = sohr_rec.freight + 	
			sohr_rec.insurance +
			sohr_rec.other_cost_1 +
			sohr_rec.other_cost_2 + 	
			sohr_rec.other_cost_3 -
			sohr_rec.discount;

	if (noSalesTax)
		wk_gst = 0.00;
	else
		wk_gst = (double) (comm_rec.gst_rate / 100.00);

	wk_gst *= other;

	gst_tot += wk_gst;
	gst_tot = no_dec (gst_tot);

	if (envVar.dbNettUsed)
		tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + other + lev_tot);
	else
		tot_tot = no_dec (inv_tot + tax_tot + gst_tot + other + lev_tot);

	if (override > 1 && !clear_disc)
	{
		if (tot_tot != local_rec.inputTotal)
		{
			diff = no_dec (local_rec.inputTotal - tot_tot);

			if (noSalesTax)
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

			sohr_rec.other_cost_1 = diff;
			sohr_rec.other_cost_1 -= gst_amount;
			gst_tot += gst_amount;
			gst_tot = no_dec (gst_tot);
		}
	}
	if (!override)
		PrintBoxTotals ();

	local_rec.discOverride = 0.00;
}

/*
 * Calculate extended Values. 
 */
void
CalcExtendedTotal (
 int	line_no)
{
	/*
	 * Update soln gross tax and disc for each line. 
	 */
	l_total = (double) store [line_no].qtyOrder;
	l_total *= out_cost (store [line_no].actSalePrice,store [line_no].outerSize);
	l_total = no_dec (l_total);
	
	if (noSalesTax)
		t_total = 0.00;
	else
	{
		t_total = (double) store [line_no].qtyOrder;
		t_total *= out_cost (store [line_no].taxAmt,store [line_no].outerSize);
		t_total = no_dec (t_total);
	
	}
	l_disc = (double) store [line_no].discPc;
	l_disc *= l_total;
	l_disc = DOLLARS (l_disc);
	l_disc = no_dec (l_disc);
	if (envVar.advertLevy)
	{
		lineLevyPc 	= (double) (store [line_no].advertLevyPc);
		lineLevyPc 	= DOLLARS (lineLevyPc);
		lineLevyPc 	*= l_total;
		lineLevyPc 	= no_dec (lineLevyPc);

		lineLevyAmt = (double) store [line_no].qtyOrder;
		lineLevyAmt *= store [line_no].advertLevyAmt;
		lineLevyAmt = no_dec (lineLevyAmt);
		l_levy		=	lineLevyPc	+	lineLevyAmt;

		if (ORD_DISPLAY)
			l_levy	=	store [line_no].itemLevy;
	}
	if (noSalesTax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) store [line_no].taxPc;
		if (sohr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
		{
			if (envVar.dbNettUsed)
				l_tax *= (l_total + l_levy - l_disc);
			else
				l_tax *= l_total + l_levy;
		}
		l_tax = DOLLARS (l_tax);
	}
	l_tax = no_dec (l_tax);

	if (noSalesTax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) store [line_no].gstPc;
		if (envVar.dbNettUsed)
			l_gst *= ((l_total - l_disc) + l_tax + l_levy);
		else
			l_gst *= (l_total + l_tax + l_levy);
		l_gst = DOLLARS (l_gst);
	}
}

/*
 * Draw totals.
 */
void
DrawTotals (void)
{
	box (96,0,35,4);
	/*
	 * Nett      Total:
	 * Fri/Other Total:
	 * Tax       Total:
	 * TOTAL          :
	 */
	print_at (1,98, ML (mlSoMess064));
	print_at (2,98, ML (mlSoMess065));
	print_at (3,98, ML (mlSoMess066), envVar.gstCode);
	print_at (4,98, ML (mlSoMess068));
}

void
PrintBoxTotals (void)
{
	double	f_other = 0.00;
	

	f_other = 	sohr_rec.freight + 	
		  		sohr_rec.insurance +
		  		sohr_rec.other_cost_1 +
		  		sohr_rec.other_cost_2 + 	
		  		sohr_rec.other_cost_3 -
				sohr_rec.discount;

	if (envVar.dbNettUsed)
		tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + f_other + lev_tot);
	else
		tot_tot = no_dec (inv_tot + tax_tot + gst_tot + f_other + lev_tot);

	if (envVar.dbNettUsed)
		print_at (1,115,"%14.2f",DOLLARS (no_dec((inv_tot + lev_tot) - dis_tot)));
	else
		print_at (1,115,"%14.2f",DOLLARS (no_dec(inv_tot + lev_tot)));

	print_at (2,115,"%14.2f",DOLLARS (f_other));
	print_at (3,115,"%14.2f",DOLLARS (gst_tot + tax_tot));
	print_at (4,115,"%14.2f",DOLLARS (tot_tot));
}

/*
 * Routine to load number plate lines. 
 */
int
LoadNumberPlateLines (
	long	hhndHash)
{
	float	noPlateQtyRemain	=	0.00,
			realCommitted		=	0.00,
			std_cnv_fct			=	1.00;

	struct	WH_LIST	*lcl_ptr;

	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (inum, "inum_hhum_hash");

	abc_selfield (sknd, "sknd_id_no");

	sknd_rec.sknh_hash	= sknh_rec.sknh_hash;
	sknd_rec.line_no	= 0;
	cc = find_rec (sknd, &sknd_rec, GTEQ, "r");
	while (!cc && sknd_rec.sknh_hash == sknh_rec.sknh_hash)
	{
		if (sknd_rec.status [0] == 'D')
		{
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
			continue;
		}
		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash	=	sknd_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc) 
			file_err (cc, inmr, "DBFIND");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		std_cnv_fct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	sknd_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		line_cnt	= lcount [ITEM_SCN];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		LSR.cnvFct 		= inum_rec.cnv_fct / std_cnv_fct;
		LSR.stdCnvFct 	= std_cnv_fct;

		strcpy (local_rec.item_no,inmr_rec.item_no);
		incc_rec.hhcc_hash = 	sknd_rec.hhcc_hash;
		incc_rec.hhbr_hash = 	alt_hash 
								(
									inmr_rec.hhbr_hash,
								  	inmr_rec.hhsi_hash
								);

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		inei_rec.hhbr_hash	=	alt_hash 
								(
									inmr_rec.hhbr_hash,
									inmr_rec.hhsi_hash
								);
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei,&inei_rec,COMPARISON,"r");
		if (cc)
			cc = AddInei ();

		if (cc)
			file_err (cc, inei, "DBFIND");

		LSR.marginCost = (inei_rec.avge_cost == 0.00) ? 
                   				inei_rec.last_cost : 
                   				inei_rec.avge_cost;

		LSR.actHhwhHash = incc_rec.hhwh_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			LSR.hhwhHash = incc_rec.hhwh_hash;
			strcpy (local_rec.serialNo, sknd_rec.serial_no);
			strcpy (LSR.serialNo, sknd_rec.serial_no);
			strcpy (LSR.orgSerialNo, sknd_rec.serial_no);
		}
		else
		{
			LSR.hhwhHash = -1L;
			strcpy (local_rec.serialNo, serialSpace);
			strcpy (LSR.serialNo,  serialSpace);
			strcpy (LSR.orgSerialNo, serialSpace);
		}

		strcpy (LSR.backorderFlag, inmr_rec.bo_flag);
		strcpy (LSR.releaseFlag,inmr_rec.bo_release);
		strcpy (LSR.uom, inum_rec.uom);

		/*
		 * if the line has a contract on it then  user  
		 * not allowed to edit price or disc            
		 */
		LSR.contractStatus = 0;

		/*
		 * Check for Indent items. 
		 */
		if (strncmp (inmr_rec.item_no,"INDENT",6) || envVar.discountIndents)
			LSR.indent = FALSE;
		else
			LSR.indent = TRUE;

		/*
		 * Set quantity remaining from number plate detail.
		 */
		noPlateQtyRemain	=	sknd_rec.qty_rec - sknd_rec.qty_return;

		/*
		 * Subtract the amount(s) already issued.
		 */
		skni_rec.sknd_hash = sknd_rec.sknd_hash;
		cc = find_rec (skni, &skni_rec, GTEQ, "r");
		while (!cc && skni_rec.sknd_hash == sknd_rec.sknd_hash)
		{
			noPlateQtyRemain	-=	skni_rec.qty_issued;
			cc = find_rec (skni, &skni_rec, NEXT, "r");
		}
		if (noPlateQtyRemain <= 0.00)
			noPlateQtyRemain = 0.00;

		cc = LoadLocation 
			(
				lcount [ITEM_SCN], 
				sknd_rec.sknd_hash, 
				incc_rec.hhcc_hash,
				inum_rec.uom,
				LSR.cnvFct,
				noPlateQtyRemain
			);
		if (cc)
		{
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
			continue;
		}
		strcpy (local_rec.LL, "Y");

		LSR.qtyOrigional		= ToLclUom (noPlateQtyRemain);
		local_rec.qty_ord 	= ToLclUom (noPlateQtyRemain);
		local_rec.unitQty 	= noPlateQtyRemain;
		local_rec.qty_back 	= 0.00;

		realCommitted	=	RealTimeCommitted 
							(
								inmr_rec.hhbr_hash,
							   	incc_rec.hhcc_hash,
							   	0
							);
		if (envVar.includeForwardStock)
		{
		    LSR.qtyAvailable = incc_rec.closing_stock -
							(incc_rec.committed + realCommitted) -
							 incc_rec.backorder - 
							 incc_rec.forward;
		}
		else
		{
		    LSR.qtyAvailable = incc_rec.closing_stock -
							(incc_rec.committed + realCommitted) -
							 incc_rec.backorder;
		}
		if (envVar.qCApplies && envVar.qCAvailable)
			LSR.qtyAvailable -= incc_rec.qc_qty;

		LSR.qtyAvailable 	+= 	ToLclUom (noPlateQtyRemain);
		LSR.qtyTotal 	=	ToLclUom (noPlateQtyRemain);

		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, excf, "DBFIND");

		LSR.minMargin 		= excf_rec.min_marg;
		LSR.exchRate  		= excf_rec.ex_rate;
		LSR.qtyOrder 		= ToLclUom (noPlateQtyRemain);
		LSR.qtyOrigional	= ToLclUom (noPlateQtyRemain);
		LSR.defaultDisc 	= inmr_rec.disc_pc;
		LSR.taxPc 			= inmr_rec.tax_pc;
		LSR.gstPc 			= inmr_rec.gst_pc;
		LSR.discPc 			= 0.00;
		LSR.taxAmt 		= inmr_rec.tax_amount;
		LSR.weight 		= inmr_rec.weight;
		LSR.outerSize 			= inmr_rec.outer_size;

		if (envVar.advertLevy)
		{
			ItemLevy 
			(	
				inmr_rec.hhbr_hash,
				comm_rec.est_no, 
				cumr_rec.curr_code, 
				comm_rec.dbt_date
			);
			LSR.advertLevyAmt = DPP (inal_rec.value * 100);
			LSR.advertLevyPc  = (double) inal_rec.percent;
		}
		else
		{
			LSR.advertLevyAmt = 0.00;
			LSR.advertLevyPc  = 0.00;
		}
		LSR.costPrice 	= sknd_rec.land_cst;
		LSR.bonusFlag [0] 		= 'N';
		LSR.itemClass [0] 		= inmr_rec.inmr_class [0];
		LSR.regPc 			= 0.00;
		LSR.discA 			= 0.00;
		LSR.discB 			= 0.00;
		LSR.discC 			= 0.00;
		LSR.cumulative 	= 0;
		LSR.hhbrHash 		= sknd_rec.hhbr_hash;
		LSR.hhumHash 		= sknd_rec.hhum_hash;
		LSR.hhsiHash 		= alt_hash 
							(
								inmr_rec.hhbr_hash,
								inmr_rec.hhsi_hash
							 );
		LSR.hhslHash     	= 0L;
		LSR.hhccHash     	= sknd_rec.hhcc_hash;
		local_rec.hhccHash    = sknd_rec.hhcc_hash;
		strcpy (LSR.source, inmr_rec.source);
		strcpy (LSR.category, inmr_rec.category);
		strcpy (LSR.sellGroup, inmr_rec.sellgrp);
		strcpy (LSR.packSize, inmr_rec.pack_size);
		strcpy (local_rec.UOM, inum_rec.uom);
		strcpy (LSR.costingFlag, inmr_rec.costing_flag);
		strcpy (LSR.priceOR, "N");
		strcpy (LSR.discOR, "N");
		strcpy (LSR._desc2, inmr_rec.description2);

		strcpy (soln_rec.item_desc, inmr_rec.description);
		strcpy (soln_rec.sman_code, cumr_rec.sman_code);
		strcpy (soln_rec.hide_flag, "N");
		strcpy (soln_rec.pack_size, LSR.packSize);
		soln_rec.cost_price = 	LSR.costPrice;
		soln_rec.item_levy 	= 	LSR.itemLevy;
		soln_rec.sale_price	=	0.00;
		soln_rec.dis_pc		=	0.00;
		soln_rec.due_date	=	local_rec.lsystemDate;

		lcl_ptr = wh_head;
		while (lcl_ptr != WH_NULL)
		{
			if (lcl_ptr->hhccHash == sknd_rec.hhcc_hash)
			{	
				sprintf (local_rec.sup_br, "%2.2s", lcl_ptr->br_no);
				sprintf (local_rec.sup_wh, "%2.2s", lcl_ptr->wh_no);
				break;
			}
			lcl_ptr = lcl_ptr->next;
		}
		LSR.canEdit = TRUE;
	
		strcpy (LSR.status, createStatusFlag);
		LSR.commitRef 		= -1;
		LSR.origHhbr 		= sknd_rec.hhbr_hash;
		LSR.origOrderQty 	= ToLclUom (noPlateQtyRemain);
		soln_rec.dis_pc 	= 0.00;
		local_rec.line_no 	= lcount [ITEM_SCN] + 1;

		PriceProcess (FALSE);
		DiscProcess (FALSE);
		
		putval (lcount [ITEM_SCN]++);
		if (lcount [ITEM_SCN] >= MAXLINES) 
			break;

		cc = find_rec (sknd, &sknd_rec, NEXT, "r");
	}
	vars [scn_start].row = lcount [ITEM_SCN];

	scn_set (HEADER_SCN);
	if (lcount [ITEM_SCN] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Routine to read all soln records whose hash matches the one on the   
 * sohr record. Stores all non screen relevant details in another       
 * structure. Also gets part number for the part hash. And g/l account  
 * number.                                                              
 */
int
LoadItemScreen (
	long	hhsoHash)
{
	int		onPackingSlip	=	FALSE,
			allPackingSlip	=	TRUE;

	float	realCommitted	= 	0.00,
			stdCnvFct		=	0.00;
	char	*sptr;
	struct	WH_LIST	*lcl_ptr;

	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;

	print_at (2,0, "%s", ML (mlStdMess035));

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (inum,"inum_hhum_hash");

	LoadSONS (TRUE, 0, sohr_rec.hhso_hash);

	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;

	cc = find_rec (soln,&soln_rec,GTEQ,"r");
	while (!cc && hhsoHash == soln_rec.hhso_hash) 
	{
		if (soln_rec.status [0] == 'P')
			onPackingSlip	=	TRUE;
		else
			allPackingSlip	=	FALSE;

		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc) 
			file_err (cc, inmr, "DBFIND");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		stdCnvFct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	soln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		line_cnt	= lcount [ITEM_SCN];

		if (stdCnvFct == 0.00)
			stdCnvFct = 1;

		LSR.cnvFct 			= inum_rec.cnv_fct/stdCnvFct;
		LSR.stdCnvFct 		= stdCnvFct;

		strcpy (LSR.uom, 		inum_rec.uom);
		strcpy (LSR.uomGroup, inum_rec.uom_group);

		soln_rec.sale_price		*= LSR.cnvFct;
		soln_rec.gsale_price	*= LSR.cnvFct;

		if (soln_rec.bonus_flag [0] == 'Y')
		{
			sprintf (soBonus, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), soBonus);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);
	
		if (soln_rec.hide_flag [0] == 'Y')
		{
			sprintf (soHide, "%-2.2s", envVar.soSpecial + 2);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), soHide);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);

		if (local_rec.schedOrder [0] == 'Y' && soln_rec.del_no != 0)
			sprintf (local_rec.deliveryNo, "%05d", soln_rec.del_no);

		incc_rec.hhcc_hash = soln_rec.hhcc_hash;
		incc_rec.hhbr_hash = alt_hash 
								(
									inmr_rec.hhbr_hash,
								  	inmr_rec.hhsi_hash
								);

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		inei_rec.hhbr_hash	=	alt_hash 
									(
										inmr_rec.hhbr_hash,
							 			inmr_rec.hhsi_hash
									);
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei,&inei_rec,COMPARISON,"r");
		if (cc)
			cc = AddInei ();

		if (cc)
			file_err (cc, inei, "DBFIND");

		LSR.marginCost = (inei_rec.avge_cost == 0.00) ? 
                   				inei_rec.last_cost : 
                   				inei_rec.avge_cost;

		LSR.actHhwhHash = incc_rec.hhwh_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			LSR.hhwhHash = incc_rec.hhwh_hash;
			strcpy (local_rec.serialNo, soln_rec.serial_no);
			strcpy (LSR.serialNo, local_rec.serialNo);
			strcpy (LSR.orgSerialNo, local_rec.serialNo);
		}
		else
		{
			LSR.hhwhHash = -1L;
			strcpy (local_rec.serialNo, serialSpace);
			strcpy (LSR.serialNo,   serialSpace);
			strcpy (LSR.orgSerialNo,  serialSpace);
		}

		strcpy (LSR.backorderFlag,inmr_rec.bo_flag);
		strcpy (LSR.releaseFlag,inmr_rec.bo_release);

		/*
		 * if the line has a contract on it then  user  
		 * not allowed to edit price or disc           
		 */
		LSR.contractStatus = soln_rec.cont_status;

		/*
		 * Check for Indent items. 
		 */
		if (strncmp (inmr_rec.item_no,"INDENT",6) || envVar.discountIndents)
			LSR.indent = FALSE;
		else
			LSR.indent = TRUE;

		LSR.qtyOrigional	= ToLclUom (soln_rec.qty_org_ord);
		local_rec.qty_ord 	= ToLclUom (soln_rec.qty_order);
		local_rec.unitQty 	= soln_rec.qty_order;
		local_rec.qty_back 	= 0.00;

		if (soln_rec.qty_bord != 0.00)
		{
			if (inmr_rec.bo_flag [0] == 'Y')
	    		local_rec.qty_back = ToLclUom (soln_rec.qty_bord);
			else
			{
				if (inmr_rec.bo_flag [0] == 'F')
				{
					if (local_rec.qty_ord == 0.00)
	    					local_rec.qty_back = ToLclUom (soln_rec.qty_bord);
					else
					{
						local_rec.qty_back = ToLclUom (soln_rec.qty_bord);
						local_rec.qty_back += local_rec.qty_ord;
						local_rec.qty_ord = 0.00;
					}
				}
				else
					local_rec.qty_back = 0.00;
			}
		}

		realCommitted	=	RealTimeCommitted 
							(
								inmr_rec.hhbr_hash,
							   	incc_rec.hhcc_hash,
							   	0
							);
		if (envVar.includeForwardStock)
		{
		    LSR.qtyAvailable = incc_rec.closing_stock -
							(incc_rec.committed + realCommitted) -
							incc_rec.backorder - 
							incc_rec.forward;
		}
		else
		{
		    LSR.qtyAvailable = incc_rec.closing_stock -
							(incc_rec.committed + realCommitted) -
							incc_rec.backorder;
		}
		if (envVar.qCApplies && envVar.qCAvailable)
			LSR.qtyAvailable -= incc_rec.qc_qty;

		LSR.qtyAvailable += ToLclUom (soln_rec.qty_order + soln_rec.qty_bord);

		LSR.qtyTotal =	ToLclUom (soln_rec.qty_order + soln_rec.qty_bord);

		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, excf, "DBFIND");

		LSR.minMargin 		= excf_rec.min_marg;
		LSR.exchRate  		= excf_rec.ex_rate;
		LSR.qtyOrder 		= ToLclUom (soln_rec.qty_order);
		LSR.qtyOrigional 	= ToLclUom (soln_rec.qty_org_ord);
		LSR.defaultDisc 	= inmr_rec.disc_pc;
		LSR.taxPc 			= inmr_rec.tax_pc;
		LSR.gstPc 			= inmr_rec.gst_pc;
		LSR.discPc 			= soln_rec.dis_pc;
		LSR.taxAmt 			= inmr_rec.tax_amount;
		LSR.weight 			= inmr_rec.weight;
		LSR.outerSize 			= inmr_rec.outer_size;

		if (envVar.advertLevy)
		{
			ItemLevy 
			(	
				inmr_rec.hhbr_hash,
				comm_rec.est_no, 
				cumr_rec.curr_code, 
				comm_rec.dbt_date
			);
			LSR.advertLevyAmt = DPP (inal_rec.value * 100);
			LSR.advertLevyPc  = (double) inal_rec.percent;
		}
		else
		{
			LSR.advertLevyAmt = 0.00;
			LSR.advertLevyPc  = 0.00;
		}
		LSR.costPrice 	= DPP (soln_rec.cost_price);
		LSR.itemLevy 		= DPP (soln_rec.item_levy);
		LSR.actSalePrice 		= DPP (soln_rec.sale_price);
		LSR.salePrice 	= DPP (soln_rec.sale_price);
		LSR.bonusFlag [0] 		= soln_rec.bonus_flag [0];
		LSR.itemClass [0] 		= inmr_rec.inmr_class [0];
		LSR.regPc 		= soln_rec.reg_pc;
		LSR.discA 		= soln_rec.disc_a;
		LSR.discB 		= soln_rec.disc_b;
		LSR.discC 		= soln_rec.disc_c;
		LSR.cumulative 	= soln_rec.cumulative;
		LSR.gSalePrice 	= DPP (soln_rec.gsale_price);
		LSR.hhbrHash 		= soln_rec.hhbr_hash;
		LSR.hhumHash 		= soln_rec.hhum_hash;
		LSR.hhsiHash 		= alt_hash 
							(
								inmr_rec.hhbr_hash,
								inmr_rec.hhsi_hash
							 );
		LSR.hhslHash     = soln_rec.hhsl_hash;
		LSR.hhccHash     = soln_rec.hhcc_hash;
		local_rec.hhccHash    = soln_rec.hhcc_hash;
		strcpy (LSR.source,inmr_rec.source);
		strcpy (LSR.category,inmr_rec.category);
		strcpy (LSR.sellGroup,inmr_rec.sellgrp);
		strcpy (LSR.packSize,inmr_rec.pack_size);
		strcpy (local_rec.UOM,inum_rec.uom);
		strcpy (LSR.costingFlag,inmr_rec.costing_flag);
		strcpy (LSR.priceOR,soln_rec.pri_or);
		strcpy (LSR.discOR,soln_rec.dis_or);
		strcpy (LSR._desc2,inmr_rec.description2);

		lcl_ptr = wh_head;
		while (lcl_ptr != WH_NULL)
		{
			if (lcl_ptr->hhccHash == soln_rec.hhcc_hash)
			{	
				sprintf (local_rec.sup_br, "%2.2s", lcl_ptr->br_no);
				sprintf (local_rec.sup_wh, "%2.2s", lcl_ptr->wh_no);
				break;
			}
			lcl_ptr = lcl_ptr->next;
		}

		LSR.canEdit = TRUE;
		if (ORD_MAINT)
		{
			if (!VALID_MAINT (soln_rec.status))
				LSR.canEdit = FALSE;
		}
		strcpy (LSR.status, soln_rec.status);
		LSR.commitRef 	= -1;
		LSR.origHhbr 	= soln_rec.hhbr_hash;
		LSR.origOrderQty = ToLclUom (soln_rec.qty_order + soln_rec.qty_bord);
		soln_rec.dis_pc = ScreenDisc (soln_rec.dis_pc);

		LoadSONS (FALSE, lcount [ITEM_SCN], soln_rec.hhsl_hash);

		local_rec.line_no 	=	soln_rec.line_no + 1;

		putval (lcount [ITEM_SCN]++);
		if (lcount [ITEM_SCN] >= MAXLINES) 
			break;

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	
	if (sohr_rec.tax_code [0] == 'A' || sohr_rec.tax_code [0] == 'B')
		noSalesTax = TRUE;
	else
		noSalesTax = FALSE;

	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (inum,"inum_uom");

	/*
	 * Don't allow lines to be added if any line contains a 'P' status 
	 */
	if (ORD_DISPLAY || onPackingSlip || DESPATCHED)
		vars [scn_start].row = lcount [ITEM_SCN];

	if (ORD_MAINT && (sohr_rec.status [0] == 'P' || DESPATCHED))
		vars [scn_start].row = lcount [ITEM_SCN];

	if (allPackingSlip == TRUE && ORD_MAINT)
	{
		print_mess (ML ("No lines able to be maintained on packing slip."));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	if (lcount [ITEM_SCN] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * update all relevent records. 
 */
int
Update (void)
{
	int		i				=	0,
			PrntCounter		=	0,
			autoOrderNo		=	FALSE,
			allForward 		= 	0,
			allBackorder  	= 	0,
			forceUpdate 	= 	FALSE,
			backOrderAll 	= 	TRUE,	
			whPrinterNo 	= 	0,
			key				=	0;

	long	orderNo		=	0L,
			hhsoHash		=	0L,
			last_hhcc		=	0L,
			last_hhso		=	0L;

	char	last_status [2],
			lp_str [5],
			so_hash [10];

	float	orderQty	=	0.00;

	struct	WH_LIST	*lcl_ptr;

	overMargin = FALSE;
	autoOrderNo = FALSE;

	clear ();
	scn_set (HEADER_SCN);

	open_rec (sohr2,sohr_list,SOHR_NO_FIELDS,"sohr_id_no2");
	abc_selfield (inmr,"inmr_hhbr_hash");

	if (envVar.skGrinNoPlate)
		abc_selfield (sknd, "sknd_sknd_hash");

	/*
	 * Check which WHs have been supplied from 
	 */
	if (envVar.supplyFromAlternate)
	{
	    lcl_ptr = wh_head;
	    while (lcl_ptr != WH_NULL)
	    {
			for (i = 0; i < lcount [ITEM_SCN]; i++)
			{
				if (store [i].hhccHash == lcl_ptr->hhccHash)
				{
					/*
					 * Do NOT create a separate sales order 
					 * just for a COMM_LINE items.          
					 */
					inmr_rec.hhbr_hash	=	store [i].hhbrHash;
					cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
					if (cc || inmr_rec.inmr_class [0] == 'Z')
						continue;

					strcpy (lcl_ptr->ordered_from, "Y");
					break;
				}
			}

			lcl_ptr = lcl_ptr->next;
	    }
	}

	if (sohr_rec.tax_code [0] == 'A' || sohr_rec.tax_code [0] == 'B')
	       noSalesTax = TRUE;
	else
	       noSalesTax = FALSE;

	if (local_rec.inputTotal != 0.00)
	   	CalcInputTotal ();

	if (highestDiscountPc >= 100.00 || lowestDiscountPc <= -100.00)
	{
		/*
		 * Sales Order Cancelled Due to Input Total Being Different 
		 * from Input Lines By Greater Than 100%%				
		 * Press Any Key to Continue						
		 */
		print_at (PrntCounter++,0, ML (mlSoMess111)); 
		PauseForKey (PrntCounter++, 0, ML (mlStdMess042), 0);
		return (EXIT_FAILURE);
	}

	/*
	 * NB. Logistic Sales Order Entry does not allow the first line 
	 * of an order to be a COMM_LINE item. ie class Z            
	 */
	strcpy (last_status, createStatusFlag);
	last_hhcc = ccmr_rec.hhcc_hash;
	last_hhso = 0L;

	strcpy (sohr_rec.two_step, (TWO_STEP) ? "Y" : "N");
	strcpy (sohr_rec.din_1,local_rec.spinst [0]);
	strcpy (sohr_rec.din_2,local_rec.spinst [1]);
	strcpy (sohr_rec.din_3,local_rec.spinst [2]);
	strcpy (sohr_rec.del_zone, trzm_rec.del_zone);
	strcpy (sohr_rec.carr_code, trcm_rec.carr_code);
	sprintf (sohr_rec.op_id, "%-14.14s", currentUserName);
	strcpy (sohr_rec.time_create, TimeHHMM ());
	sohr_rec.date_create = TodaysDate ();

	if (newOrder && lcount [ITEM_SCN] == 0)
	{
	    for (i = 0; i < 6; i++)
	    {
			/*
			 * Sales Order Not created as no lines are present.
			 */
			rv_pr (ML (mlSoMess121), 0,0,i % 2);
			sleep (sleepTime);
	    }
	    return (EXIT_FAILURE);
	}

	/*
	 * New Order 
	 */
	if (newOrder) 
	{
	    strcpy (sohr_rec.batch_no,"00000");

	    /*
	     * Generate order number 
	     */
	    if (!strcmp (sohr_rec.order_no,"00000000") || 
		    !strcmp (sohr_rec.order_no,"        "))
	    {
			autoOrderNo = TRUE;
			open_rec (esmr,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");

			strcpy (esmr_rec.co_no,comm_rec.co_no);
			strcpy (esmr_rec.est_no,comm_rec.est_no);
			cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, esmr, "DBFIND");

			/*
			 * Check if Order No Already Allocated if it has been then skip
			 */
			while (CheckSohr (++esmr_rec.nx_order_no, comm_rec.est_no) == 0);

			/*
			 * If supplying from alternate WHs
			 * don't update esmr here as order
			 * numbers are generated later.    
			 */
			if (!envVar.supplyFromAlternate)
			{
				cc = abc_update (esmr,&esmr_rec);
				if (cc)
					file_err (cc, esmr, "DBUPDATE");
			}

			sprintf (sohr_rec.order_no,"%08ld",esmr_rec.nx_order_no);
	    }
	    else
			sprintf (local_rec.orderNo, "%-8.8s", sohr_rec.order_no);
	    
	    /*
	     * Display Sales Order Number Created 
	     */
	    if (!envVar.supplyFromAlternate)
	    {
			/*
			 * Sales Order No (%s) Created. 
			 */
			sprintf (err_str, ML (mlSoMess112), sohr_rec.order_no);
			print_at (PrntCounter++,0,"%s...", err_str);
	    }

	    if (!heldOrder)
	    {
			if (CheckCumr (OrderValue ()))
			{
				/*
				 * NOTE : Order %s is placed on Credit Hold as existing
				 * Order (s) plus this order exceeds credit Terms 	  
				 */
				if (!envVar.supplyFromAlternate)
					print_at (PrntCounter++,0,"%s", ML (mlSoMess113));

				heldOrder = TRUE;
			}
			if (!heldOrder && overMargin)
			{
				/*
				 * NOTE : Order %s is placed on Credit Hold as Line (s)
				 * did not obtain minimum margin percent.			 
				 */
				if (!envVar.supplyFromAlternate)
					print_at (PrntCounter++,0,"%s", ML (mlSoMess114));
			}
	    }

	    strcpy (sohr_rec.co_no,comm_rec.co_no);
	    sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

	    strcpy (sohr_rec.inv_no,"        ");

	    strcpy (sohr_rec.stat_flag, createStatusFlag);
	    strcpy (sohr_rec.status, createStatusFlag);

	    if (ORD_INPUT)
	    {
			/*
			 * Process Held first. 
			 */
			if (overMargin)
				strcpy (sohr_rec.status,"O");

			if (heldOrder)
				strcpy (sohr_rec.status,"H");

			sohr_rec.exch_rate = pocrRec.ex1_factor;
	    }
	    /*
	     * Forward Order	Overwrites all others. 
	     */
	    if (sohr_rec.dt_required > local_rec.lsystemDate)
			strcpy (sohr_rec.status,"F");

	    /*
	     * Scheduled Orders overwrite Forward Orders. 
	     */
		strcpy (sohr_rec.sch_ord, local_rec.schedOrder);
	    if (local_rec.schedOrder [0] == 'Y')
		{
			strcpy (sohr_rec.status,    "G");
			strcpy (sohr_rec.stat_flag, "G");
		}

	    /*
	     * Add sohr record (s).
	     */
	    if (envVar.supplyFromAlternate)
	    {
			/*
			 * Potentially more then one sohr record to add. 
			 */
			lcl_ptr = wh_head;
			while (lcl_ptr != WH_NULL)
			{
				if (lcl_ptr->ordered_from [0] == 'N')
				{
					lcl_ptr = lcl_ptr->next;
					continue;
				}

				if (lcl_ptr->hhccHash == ccmr_rec.hhcc_hash &&
					!autoOrderNo)
				{
					sprintf (sohr_rec.order_no, "%-8.8s", local_rec.orderNo);
					strcpy (sohr_rec.br_no, comm_rec.est_no);
				}
				else
				{
					strcpy (esmr_rec.co_no,comm_rec.co_no);
					sprintf (esmr_rec.est_no, "%2.2s", lcl_ptr->br_no);
					cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
					if (cc)
						file_err (cc, esmr, "DBFIND");

					/*
					 * Check if Order No Already Allocated 
					 * If it has been then skip.          
					 */
					while (CheckSohr (++esmr_rec.nx_order_no, lcl_ptr->br_no) == 0);
					cc = abc_update (esmr,&esmr_rec);
					if (cc)
						file_err (cc, esmr, "DBUPDATE");
								
					sprintf (lcl_ptr->orderNo, "%08ld", esmr_rec.nx_order_no);
					sprintf (sohr_rec.order_no, "%08ld", esmr_rec.nx_order_no);
					sprintf (sohr_rec.br_no, "%2.2s", lcl_ptr->br_no);
		    	}

				/*
				 * Display order information 		 
				 * Sales Order No (%-6.6s) Created. 
				 */
				sprintf (err_str, ML (mlSoMess112), sohr_rec.order_no);
				print_at (PrntCounter++,0," %s...", err_str);

				sprintf (err_str, ML ("for Branch Number %s ..."), sohr_rec.br_no);
				print_at (PrntCounter++,0," %s...", err_str);

				if (heldOrder)
					print_at (PrntCounter++,0,"%s", ML (mlSoMess113));
				else
				{
					if (overMargin)
						print_at (PrntCounter++,0,"%s", ML (mlSoMess114));
				}
		    	cc = abc_add (sohr,&sohr_rec);
		    	if (cc) 
					file_err (cc, (char *)sohr, "DBADD");

		    	cc = find_rec (sohr,&sohr_rec,COMPARISON,"u");
		    	if (cc)
					file_err (cc, sohr, "DBFIND");

				/*
				 * Create a log file record for sales Order. 
				 */
				LogCustService 
				(
					0L,
					sohr_rec.hhso_hash,
					cumr_rec.hhcu_hash,
					sohr_rec.cus_ord_ref,
					sohr_rec.cons_no,
					sohr_rec.carr_code,
					sohr_rec.del_zone,
					LOG_CREATE
				);
	
				UpdateSONS (TRUE, 0, sohr_rec.hhso_hash);

		    	lcl_ptr->hhsoHash = sohr_rec.hhso_hash;

		    	/*
		    	 * Freight and Misc Charges only apply to the first sohr created
		    	 */
		    	sohr_rec.freight 		= 0.00;
		    	sohr_rec.deposit 		= 0.00;
		    	sohr_rec.insurance 		= 0.00;
		    	sohr_rec.discount 		= 0.00;
		    	sohr_rec.other_cost_1 	= 0.00;
		    	sohr_rec.other_cost_2 	= 0.00;
		    	sohr_rec.other_cost_3 	= 0.00;

		    	strcpy (sohr_rec.carr_code, "    ");
		    	strcpy (sohr_rec.carr_area, "  ");
		    	sohr_rec.no_cartons 	= 0;
		    	sohr_rec.no_kgs 		= 0;

		    	abc_unlock (esmr);

		    	lcl_ptr = lcl_ptr->next;
			}
	    }
	    else
	    {
			/*
			 * Only one sohr record to add. 
			 */
			strcpy (sohr_rec.br_no,comm_rec.est_no);
			cc = abc_add (sohr,&sohr_rec);
			if (cc) 
				file_err (cc, sohr, "DBADD");
	
			cc = find_rec (sohr,&sohr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, sohr, "DBFIND");
	
			UpdateSONS (TRUE, 0, sohr_rec.hhso_hash);

			abc_unlock (esmr);

		}

		/*
		 * Now Adding Order Detail Lines
		 */
	    print_at (PrntCounter++,0," %s", ML (mlSoMess087));
	}
	else
	{
	    if (!heldOrder)
	    {
			if (CheckCumr (OrderValue ()))
			{
				print_at (PrntCounter++,0,"%s", ML (mlSoMess113));
				heldOrder = TRUE;
			}
			if (!heldOrder && overMargin)
			{
				print_at (PrntCounter++,0,"%s", ML (mlSoMess114));
			}
		}
		/*
		 * Now Updating Order Detail Lines
		 */
	    print_at (PrntCounter++,0,"%s", ML (mlSoMess088));
	}

	abc_fclose (esmr);

	orderNo = atol (sohr_rec.order_no);

	fflush (stdout);
	scn_set (ITEM_SCN);

	if (sohr_rec.full_supply [0] == 'Y' && ORD_INPUT)
	{
	    /*
	     * Check if all lines are backorded. 
	     */
	    for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++) 
	    {
			getval (line_cnt);
			if (local_rec.qty_ord != 0.00)
			{
				local_rec.qty_back += local_rec.qty_ord;
				local_rec.qty_ord = 0.00;
				putval (line_cnt);
			}
		}
	}
	/*
	 * Check if all lines are backorded. 
	 */
	for (line_cnt = 0; line_cnt < lcount [ITEM_SCN]; line_cnt++) 
	{
	    getval (line_cnt);
	    if (local_rec.qty_ord != 0.00 || 
			soln_rec.due_date > local_rec.lsystemDate)
	    {
	    	backOrderAll = FALSE;
	    }
		/*
		 * Split a mixed backorder / order line into two. 
		 */
		if (SPLIT_LINE)
		{
			orderQty			=	local_rec.qty_ord;
			local_rec.qty_ord	=	0.00;
			putval (line_cnt);
			for (i = line_cnt, line_cnt = lcount [ITEM_SCN]; line_cnt > i; line_cnt--)
			{
				memcpy 
				(
					(char *) &SR, 
					(char *) &store [line_cnt - 1], 
					sizeof (struct storeRec)
				);
				getval (line_cnt - 1);
				putval (line_cnt);
			}
			lcount [ITEM_SCN]++;

			line_cnt = i;
			getval (line_cnt);
			local_rec.qty_back 	= 0.00;
			local_rec.qty_ord 	= orderQty;
			putval (line_cnt);
		}
	}
	abc_selfield (inum, "inum_hhum_hash");

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++) 
	{
		int	new_soln = TRUE,
		new_line_no = -line_cnt - 1;

	    getval (line_cnt);

	    if (envVar.supplyFromAlternate)
	    {
			lcl_ptr = wh_head;
			while (lcl_ptr != WH_NULL)
			{
				if (local_rec.hhccHash == lcl_ptr->hhccHash)
				{
					hhsoHash = lcl_ptr->hhsoHash;
					break;
				}

				lcl_ptr = lcl_ptr->next;
			}
	    }
	    else
	    	hhsoHash = sohr_rec.hhso_hash;

		if (store [line_cnt].hhslHash)
		{
			soln_rec.hhsl_hash	=	store [line_cnt].hhslHash;
	    	new_soln = find_rec (soln2, &soln_rec, EQUAL, "u");
		}

	    getval (line_cnt);

	    strcpy (soln_rec.pri_or, SR.priceOR);
	    strcpy (soln_rec.dis_or, SR.discOR);

	    if (SR.hhwhHash > 0L)
			sprintf (soln_rec.serial_no, "%-25.25s", local_rec.serialNo);
		else
			sprintf (soln_rec.serial_no, "%-25.25s", " ");

		inmr_rec.hhbr_hash	=	SR.hhbrHash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	    if (cc)
			file_err (cc, inmr, "DBFIND");

	    incc_rec.hhcc_hash = local_rec.hhccHash;

	    if (inmr_rec.hhbr_hash == soln_rec.hhbr_hash &&
			!envVar.supplyFromAlternate)
	    {
			if (incc_rec.hhcc_hash != soln_rec.hhcc_hash)
		    	incc_rec.hhcc_hash = soln_rec.hhcc_hash;
	    }

		/*
		 *	Update line numbers
		 *	They're initially -ve to avoid duplicate-index
		 *	errors on updates/adds
		 */
	    soln_rec.hhso_hash 	= hhsoHash;
	    soln_rec.line_no 	= new_line_no;
		
	    soln_rec.hhbr_hash 		= inmr_rec.hhbr_hash;
	    soln_rec.hhcc_hash 		= incc_rec.hhcc_hash;
	    soln_rec.hhum_hash 		= SR.hhumHash;  
	    soln_rec.qty_org_ord	= ToStdUom (SR.qtyOrigional);
	    soln_rec.qty_order 		= ToStdUom (local_rec.qty_ord);
	    soln_rec.qty_bord  		= ToStdUom (local_rec.qty_back);
	    soln_rec.cost_price		= SR.costPrice;
	    soln_rec.item_levy		= SR.itemLevy;
		soln_rec.reg_pc			= SR.regPc;
		soln_rec.disc_a			= SR.discA;
		soln_rec.disc_b			= SR.discB;
		soln_rec.disc_c			= SR.discC;
		soln_rec.cumulative		= SR.cumulative;

		if (SR.cnvFct == 0.00)
			SR.cnvFct = 1.00;

		soln_rec.gsale_price	= SR.gSalePrice / SR.cnvFct;
		soln_rec.sale_price		= SR.salePrice  / SR.cnvFct;
		soln_rec.cont_status	= SR.contractStatus;

	    soln_rec.gst_pc  = (float) ((noSalesTax) ? 0.00 : inmr_rec.gst_pc);
	    soln_rec.tax_pc  = (float) ((noSalesTax) ? 0.00 : inmr_rec.tax_pc);

	    soln_rec.o_xrate = SR.exchRate;
	    soln_rec.n_xrate = 0.00;

	    if (SR.bonusFlag [0] == 'Y')
			strcpy (soln_rec.bonus_flag,"Y");
	    else
			strcpy (soln_rec.bonus_flag,"N");

		soln_rec.del_no = atoi (local_rec.deliveryNo);
		
	    if (new_soln)
	    {
			strcpy (soln_rec.stat_flag, createStatusFlag);
			strcpy (soln_rec.status,    createStatusFlag);

			/*
			 * Process Held first. 
			 */
			if (overMargin)
		    	strcpy (soln_rec.status,"O");

			if (heldOrder)
		    	strcpy (soln_rec.status,"H");
	    }
	    /*
	     * Overide if Forward Order. 
	     */
	    if (soln_rec.due_date > local_rec.lsystemDate)
			strcpy (soln_rec.status,"F");

	    /*
	     * Overide if Scheduled Order. 
	     */
	    if (local_rec.schedOrder [0] == 'Y')
		{
			strcpy (soln_rec.status,    "G");
			strcpy (soln_rec.stat_flag, "G");
		}

	    /*
	     * forward OR scheduled so all on order 
	     */
	    if (soln_rec.status [0] == 'F' || soln_rec.status [0] == 'G')
	    {
			soln_rec.qty_order += soln_rec.qty_bord;
			soln_rec.qty_bord = 0.00;
	    }
	    if (COMM_LINE)
	    {
			strcpy (soln_rec.status,last_status);
			if (line_cnt != 0)
			{
		    	soln_rec.hhcc_hash = last_hhcc;
		    	soln_rec.hhso_hash = last_hhso;
			}
	    }

	    if (!new_soln)
	    {
			/*
			 * If all of line placed on b/o then create line as 
			 * as a b/o but only if backOrderAll is set to true
			 */
			if (soln_rec.qty_order == 0.00 && soln_rec.qty_bord != 0.00)
			{
				if (MANF_ITEM && envVar.soWoAllowed)
				{
					strcpy (soln_rec.stat_flag, "W");
					strcpy (soln_rec.status,    "W");
				}
				else if (backOrderAll)
				{
					strcpy (soln_rec.stat_flag, "B");
					strcpy (soln_rec.status,    "B");
				}
			}
			/*
			 * Safer to add this test that to remember why the above exists. 
			 */
			if (SOLN_HELD && soln_rec.qty_order == 0.00 && 
							 soln_rec.qty_bord != 0.00)
			{
				if (MANF_ITEM && envVar.soWoAllowed)
				{
					strcpy (soln_rec.stat_flag, "W");
					strcpy (soln_rec.status,    "W");
				}
				else
				{
					strcpy (soln_rec.stat_flag, "B");
					strcpy (soln_rec.status,    "B");
				}
			}

			if (COMM_LINE)
			{
				strcpy (soln_rec.status,last_status);
				if (line_cnt != 0)
				{
					soln_rec.hhcc_hash = last_hhcc;
					soln_rec.hhso_hash = last_hhso;
				}
			}
			else
			{
				if (soln_rec.qty_order	== 0.00 && soln_rec.qty_bord == 0.00)
				{
					strcpy (soln_rec.stat_flag, "D");
					strcpy (soln_rec.status,    "D");
				}
			}
			soln_rec.dis_pc = ScreenDisc (soln_rec.dis_pc);

			/*
			 * Update existing order. 
			 */
			if ((cc = abc_update (soln2, &soln_rec)))
		    	file_err (cc, soln2, "DBUPDATE");
	    }
	    else 
	    {
			/*
			 * If all of line placed on b/o then create line as 
			 * as a b/o but only if backOrderAll is set to true.
			 */
			if (soln_rec.qty_order == 0.00 && soln_rec.qty_bord != 0.00)
			{
				if (MANF_ITEM && envVar.soWoAllowed)
				{
					strcpy (soln_rec.stat_flag, "W");
					strcpy (soln_rec.status,    "W");
				}
				else if (backOrderAll)
				{
					strcpy (soln_rec.stat_flag, "B");
					strcpy (soln_rec.status,    "B");
				}
			}
			/*
			 * Safer to add this test that to remember why the above exists.
			 */
			if (SOLN_HELD && soln_rec.qty_order == 0.00 && 
							 soln_rec.qty_bord != 0.00)
			{
				if (MANF_ITEM && envVar.soWoAllowed)
				{
					strcpy (soln_rec.stat_flag, "W");
					strcpy (soln_rec.status,    "W");
				}
				else
				{
					strcpy (soln_rec.stat_flag, "B");
					strcpy (soln_rec.status,    "B");
				}
			}
			if (COMM_LINE)
			{
				strcpy (soln_rec.status,last_status);
				if (line_cnt != 0)
				{
					soln_rec.hhcc_hash = last_hhcc;
					soln_rec.hhso_hash = last_hhso;
				}
			}
		 	
			if (soln_rec.qty_order  == 0.00 && 
				soln_rec.qty_bord == 0.00 &&
				ONE_STEP)
		 	{
			 	strcpy (soln_rec.stat_flag, "D");
			 	strcpy (soln_rec.status,    "D");
		 	}
			soln_rec.dis_pc = ScreenDisc (soln_rec.dis_pc);
			cc = abc_add (soln2,&soln_rec);
			if (cc) 
				file_err (cc, soln, "DBADD");

			abc_unlock (soln);

			/*
			 *	Find the added record
			 */
			soln_rec.hhso_hash = hhsoHash;
			soln_rec.line_no = new_line_no;
			if ((cc = find_rec (soln, &soln_rec, EQUAL, "r")))
				file_err (cc, (char *)soln, "DBFIND");
			store [line_cnt].hhslHash = soln_rec.hhsl_hash;

	    }

		UpdateSONS (FALSE, line_cnt, soln_rec.hhsl_hash);

		/*
		 * Add sobg record for Sales Order Analysis. 
		 */
		if (envVar.salesOrderSales && (ONE_STEP || TWO_STEP))
		{
			add_hash 
			(	
				comm_rec.co_no, 
				comm_rec.est_no,
				"AO", 
				0, 
				soln_rec.hhsl_hash, 
				0L,
				0L,
				(double) 0.00
			);
		}
	    strcpy (last_status, soln_rec.status);
	    last_hhcc = soln_rec.hhcc_hash;
	    last_hhso = soln_rec.hhso_hash;

	    if (soln_rec.status [0] == 'F')
			allForward++;

	    if (soln_rec.status [0] == 'B')
			allBackorder++;

		add_hash_RC 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			soln_rec.hhbr_hash,
			soln_rec.hhcc_hash,
			progPid,
			nextSoicRef
		);

		if ((SK_BATCH_CONT || MULT_LOC) && store [line_cnt].canEdit)
		{
			AllocLotLocation
			(
				line_cnt,
				TRUE,
				LL_LOAD_SO,
				soln_rec.hhsl_hash
			);
		}
		/*
		 * Updated link to sales order line and customer if number plate.
		 */
		if (envVar.skGrinNoPlate && sknd_rec.sknd_hash > 0L)
		{
			/*
			 * Find number plate line detail. (sknd_sknd_hash exists in static)
			 */
			cc = find_rec (sknd, &sknd_rec, EQUAL, "u");
			if (cc)
				file_err (cc, (char *)sknd, "DBFIND");

			/*
			 * Update number plate line detail to issued.
			 */
			strcpy (sknd_rec.status, "I");
			cc = abc_update (sknd, &sknd_rec);
			if (cc)
				file_err (cc, sknd, "DBUPDATE");

			/*
			 * Add record for number plate detail issued record.
			 */
			skni_rec.sknd_hash	=	sknd_rec.sknd_hash;
			skni_rec.hhsl_hash	=	soln_rec.hhsl_hash;
			skni_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
			skni_rec.qty_issued	=	ToStdUom (local_rec.qty_ord);
			strcpy (skni_rec.cus_ord_ref, sohr_rec.cus_ord_ref);
			cc = abc_add (skni, &skni_rec);
			if (cc)
				file_err (cc, skni, "DBADD");
		}
		if (WO_LINE && new_soln)
			CreateWorksOrder (soln_rec.hhsl_hash);
	}
	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (inum, "inum_uom");

	/*
	 * if there were no lines then hhso would not have been assigned a value.
	 */
	if (!line_cnt)
		hhsoHash = sohr_rec.hhso_hash;

	/*
	 *	Delete trailing soln records
	 */
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	for (cc = find_rec (soln, &soln_rec, GTEQ, "u");
		!cc && soln_rec.hhso_hash == hhsoHash;
	    cc = find_rec (soln, &soln_rec, NEXT, "u"))
	{
	    cc = abc_delete (soln);
	    if (cc) 
			file_err (cc, soln, "DBDELETE");

		DeleteSONS (FALSE, soln_rec.hhsl_hash);

	    /*
	     * Add sobg record for Sales Order Analysis. 
	     */
	    if (envVar.salesOrderSales && (ONE_STEP || TWO_STEP))
	    {
			add_hash (comm_rec.co_no, 
					  comm_rec.est_no,
					  "AO", 
					  0, 
					  soln_rec.hhsl_hash, 
					  0L,
					  0L,
					(double) 0.00);
	    }
	}
	abc_unlock (soln);

	/*
	 *	Resequence line numbers
	 *
	 *	Flip the numbers back to +ve.
	 */
	for (i = 0; i < line_cnt; i++)
	{
		soln_rec.hhsl_hash	=	store [i].hhslHash;
		cc = find_rec (soln2, &soln_rec, EQUAL, "u");
	    if (cc)
			file_err (cc, soln2, "DBFIND");

		soln_rec.line_no = -soln_rec.line_no - 1;
	    if ((cc = abc_update (soln2, &soln_rec)))
			file_err (cc, soln2, "abc_update");
	}

	/*
	 * Update existing order header. 
	 */
	if (!newOrder) 
	{	
	    /*
	     * Delete cancelled order. 
	     */
	    if (lcount [ITEM_SCN] == 0) 
	    {
			DeleteSONS (TRUE, sohr_rec.hhso_hash);
			/*
			 * Now Deleting Complete Order
			 */
			print_at (PrntCounter++,0,"%s", ML (mlSoMess089));
			cc = abc_delete (sohr);
			if (cc)
				file_err (cc, sohr, "DBDELETE");
	    }
	}
	if (allBackorder == lcount [ITEM_SCN] && lcount [ITEM_SCN] != 0)
	{
	    if (backOrderAll)
	    {
			strcpy (sohr_rec.sohr_new,"N");
			strcpy (sohr_rec.stat_flag, "B");
			strcpy (sohr_rec.status,    "B");
			forceUpdate = TRUE;
			/*
			 * NOTE : ALL LINES ARE BACKORDERED 
			 */
			print_at (PrntCounter++,0,"%s", ML (mlSoMess115));
	    }
	}
	if (allForward == lcount [ITEM_SCN] && lcount [ITEM_SCN] != 0)
	{
	    strcpy (sohr_rec.stat_flag, (ORD_MAINT) ? sohr_rec.stat_flag : "F");
	    strcpy (sohr_rec.status,  (ORD_MAINT) ? sohr_rec.status : "F");
	    forceUpdate = TRUE;
	}

	/*
	 * Header is status F (ie required date > today)  
	 * but no lines are status F (line level required date > todays date.
	 */
	if (ORD_INPUT && sohr_rec.status [0] == 'F' && allForward == 0)
	{
	    strcpy (sohr_rec.status, createStatusFlag);
	
	    /*
	     * Process Held first. 
	     */
	    if (overMargin)
			strcpy (sohr_rec.status,"O");
	    if (heldOrder)
			strcpy (sohr_rec.status,"H");

	    forceUpdate = TRUE;

		/*
		 * NOTE : ORDER NOT CREATED AS A FORWARD ORDER.  
		 *  NO LINES ARE FORWARD DUE.					
		 */
	    print_at (PrntCounter++,0,"%s", ML (mlSoMess116));
	}
	
	/*Scheduled Order created.");*/
	if (local_rec.schedOrder [0] == 'Y')
	    print_at (PrntCounter++,0,"%s", ML (mlSoMess117));

	if ((!newOrder && lcount [ITEM_SCN] != 0) || forceUpdate) 
	{
		UpdateSONS (TRUE, 0, sohr_rec.hhso_hash);
	    /*
	     * Just update stat flag and rewrite. 
	     */
	    cc = abc_update (sohr,&sohr_rec);
	    if (cc) 
			file_err (cc, sohr, "DBUPDATE");
	}
	
	abc_unlock (sohr);

	if (sohr_rec.del_req [0] == 'Y' && newOrder)
	{
		sprintf (err_str,"tr_trsh_mnt O %010ld",sohr_rec.hhso_hash);
		sys_exec (err_str);
	}

	add_hash (comm_rec.co_no, 
			  comm_rec.est_no,
		  	  "RO", 
			  0, 
			  cumr_rec.hhcu_hash, 
			  0L, 
			  0L, 
			(double) 0.00);

	if (lcount [ITEM_SCN] != 0)
	{
	    if (envVar.supplyFromAlternate)
	    {
			lcl_ptr = wh_head;
			while (lcl_ptr != WH_NULL)
			{
				if (lcl_ptr->ordered_from [0] != 'Y')
				{
					lcl_ptr = lcl_ptr->next;
					continue;
				}

				strcpy (sohr2_rec.co_no,comm_rec.co_no);
				strcpy (sohr2_rec.br_no,lcl_ptr->br_no);
				strcpy (sohr2_rec.order_no,lcl_ptr->orderNo);
				cc = find_rec (sohr2,&sohr2_rec,COMPARISON,"r");
				if (cc || sohr2_rec.status [0] != 'R')
				{
					lcl_ptr = lcl_ptr->next;
					continue;
				}

				/*
				 * Use WHs printer if possible 
				 */
				whPrinterNo = (lcl_ptr->whPrinterNo == 0) ? passedPrinterNo : lcl_ptr->whPrinterNo;

				add_hash (comm_rec.co_no, 
						  lcl_ptr->br_no,
						(whPrinterNo) ? "PA" : "PC", 
						  whPrinterNo,
						  lcl_ptr->hhsoHash, 
						  0L, 
						  0L, 
						(double) 0.00);

				firstInput = 0;
				strcpy (local_rec.defaultOrderType, sohr_rec.ord_type);
				if (!NO_KEY (label ("ord_type")))
					FLD ("ord_type") = NA;

				lcl_ptr = lcl_ptr->next;
			}
	    }
	    else
	    {
			if (sohr_rec.status [0] == 'R')
			{
				add_hash (comm_rec.co_no, 
						  comm_rec.est_no,
						(passedPrinterNo) ? "PA" : "PC", 
						  passedPrinterNo,
						  hhsoHash, 
						  0L, 
						  0L, 
						(double) 0.00);

				firstInput = 0;
				strcpy (local_rec.defaultOrderType,sohr_rec.ord_type);
				if (!NO_KEY (label ("ord_type")))
					FLD ("ord_type") = NA;
			}
	    }
	}
	if (!newNumberPlate)
	{
		sknh_rec.iss_date	=	TodaysDate ();
		cc = abc_update (sknh, &sknh_rec);
		if (cc)
			file_err (cc, (char *)sknh, "DBFIND");
	}
	abc_fclose (sohr2);
	recalc_sobg ();

	local_rec.previousPack = orderNo;
	sprintf (local_rec.prev_dbt_no,"%-6.6s",cumr_rec.dbt_no);

	if (!envVar.salesOrderPrint)
		PauseForKey (PrntCounter++, 0, ML (mlStdMess042),0);
	else
	{
		/*
		 * Key [PRINT] to print a sales order Acknowledgement, 
		 * any other key to continue..						  
		 */
		key = PauseForKey (PrntCounter++, 0, ML (mlSoMess282), 0);
		if ((key == 'P' || key == 'p') && !selectedPrinterNo)
			selectedPrinterNo = get_lpno (0);

		if (selectedPrinterNo && (key == 'P' || key == 'p'))
		{
			sprintf (lp_str, "%05d", selectedPrinterNo);
			sprintf (so_hash, "%09ld", hhsoHash);
			* (arg) = clip (envVar.soPrintProgram);
			* (arg + (1)) = lp_str;
			* (arg + (2)) = so_hash;
			* (arg + (3)) = (char *)0;
			shell_prog (2);
		}
	}
 	if (envVar.supplyFromAlternate)
 		InitWarehouse ();

	if (envVar.skGrinNoPlate)
		abc_selfield (sknd, "sknd_id_no");

	return (EXIT_SUCCESS);
}
/*
 * Initialise fields in WH list. 
 */
void
InitWarehouse (
 void)
{
	struct	WH_LIST *lcl_ptr;
 
	lcl_ptr = wh_head;
	while (lcl_ptr != WH_NULL)
 	{
		strcpy (lcl_ptr->orderNo, "        ");
		lcl_ptr->hhsoHash = 0L;
		strcpy (lcl_ptr->ordered_from, "N");
 
		lcl_ptr = lcl_ptr->next;
	}
}

/*
 * Check Value of Order will existing value of orders.
 */
double	
OrderValue (void)
{
	double	order_val = 0.00;
	double	o_total = 0.00;
	double	o_disc = 0.00;
	int		i;

	for (i = 0;i < lcount [ITEM_SCN];i++) 
	{
		/*
		 * Update soln gross tax and disc for each line. 
		 */
		o_total = (double) store [i].qtyOrder;
		o_total *= out_cost (store [i].actSalePrice,store [i].outerSize);

		o_disc = (double) (store [i].discPc / 100.00);
		o_disc *= o_total;

		order_val += o_total - o_disc;

		if (store [i].marginFailed [0] == 'Y')
			overMargin = TRUE;
	}
	return (order_val);
}

/*
 * Validate credit period and credit limit. 
 */
int
CheckCumr (
	double	OrderValue)
{
	double	total_owing = 0.00;

	if (STANDARD || cumr_rec.crd_flag [0] == 'Y')
		return (EXIT_SUCCESS);

	total_owing = cumr_balance [0] + 
				  cumr_balance [1] +
	              cumr_balance [2] + 
				  cumr_balance [3] +
				  cumr_balance [4] +
				  cumr_balance [5] +
				  cumr_rec.ord_value +
				  OrderValue;

	/*
	 * Check if customer is over his credit limit. 
	 */
	if (cumr_rec.credit_limit <= total_owing && cumr_rec.credit_limit != 0.00)
		return (EXIT_FAILURE);

	if (cumr_rec.od_flag)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

int
CheckSohr (
	long	orderNumber,
	char	*branchNumber)
{
	strcpy (sohr2_rec.co_no, comm_rec.co_no);
	strcpy (sohr2_rec.br_no, branchNumber);
	sprintf (sohr2_rec.order_no, "%08ld", orderNumber);
	return (find_rec (sohr2, &sohr2_rec, COMPARISON, "r"));
}

/*
 * Update serial number checking for multiple branch ability to commit. 
 */
int
UpdateInsf (
	long	hhwhHash,
	long	hhbrHash,
	char	*serialNo,
	char	*oldStatus,
	char	*newStatus)
{
	abc_selfield (insf,"insf_id_no");

	insf_rec.hhwh_hash = hhwhHash;
	insf_rec.hhbr_hash = hhbrHash;
	sprintf (insf_rec.status, "%-1.1s", oldStatus);
	sprintf (insf_rec.serial_no, "%-25.25s", serialNo);

	/*
	 * If not found then item is by hhbr_hash and not hhwh_hash.
	 */
	if (find_rec (insf, &insf_rec, COMPARISON, "r"))
	{
		abc_selfield (insf,"insf_hhbr_id");

		insf_rec.hhwh_hash = hhwhHash;
		insf_rec.hhbr_hash = hhbrHash;
		sprintf (insf_rec.status, "%-1.1s", oldStatus);
		sprintf (insf_rec.serial_no, "%-25.25s", serialNo);
		cc = find_rec (insf, &insf_rec, COMPARISON, "r");
		if (cc)
			return (cc);
	}
	strcpy (insf_rec.status,newStatus);
	return (abc_update (insf,&insf_rec));
}

/*
 * Load purchase order non stock lines file. 
 */
void		
LoadSknh (void)
{
	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
	strcpy (sknh_rec.plate_no, local_rec.numberPlate);
	newNumberPlate = find_rec (sknh, &sknh_rec, COMPARISON, "u");
	if (newNumberPlate)
		memset (&sknh_rec , 0, sizeof (sknh_rec));
}

/*
 * Search Number Plate Header. 
 */
void
SrchSknh (
	char    *keyValue)
{
	int		recordFound	=	FALSE;
    _work_open (15,0,40);
    save_rec ("#Number Plate   ", "#Number Plate description.");

	/*
	 * Flush record buffer first 
	 */
	memset (&sknh_rec, 0, sizeof (sknh_rec));

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", keyValue);
    for (cc = find_rec (sknh, &sknh_rec,  GTEQ,"r");
		 !cc && !strcmp (sknh_rec.co_no, comm_rec.co_no)
		  && !strcmp (sknh_rec.br_no, comm_rec.est_no)
		  && !strncmp (sknh_rec.plate_no, keyValue, strlen (clip (keyValue)));
         cc = find_rec (sknh, &sknh_rec,  NEXT, "r"))
    {
		recordFound	=	FALSE;
		sknd_rec.sknh_hash	= sknh_rec.sknh_hash;
		sknd_rec.line_no	= 0;
		cc = find_rec (sknd, &sknd_rec, GTEQ, "r");
		while (!cc && sknd_rec.sknh_hash == sknh_rec.sknh_hash)
		{
			if (sknd_rec.status [0] != 'D')
			{
				recordFound	=	TRUE;
				break;
			}
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
		}
		if (recordFound)	
		{
        	cc = save_rec (sknh_rec.plate_no, sknh_rec.lab_note1);
        	if (cc)
            	break;
		}
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", temp_str);
    cc = find_rec (sknh, &sknh_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, (char *)sknh, "DBFIND");
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
/*
 * Search On Department     
 */
void
SrchCudp (
 char    *keyValue)
{
    _work_open (2,0,40);
    save_rec ("#No", "#Department Name");

	/*
	 * Flush record buffer first 
	 */
	memset (&cudp_rec, 0, sizeof (cudp_rec));

	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
    sprintf (cudp_rec.dp_no, "%2.2s", keyValue);
    for (cc = find_rec (cudp, &cudp_rec,  GTEQ,"r");
		 !cc && !strcmp (cudp_rec.co_no, comm_rec.co_no)
		  && !strcmp (cudp_rec.br_no, comm_rec.est_no)
		  && !strncmp (cudp_rec.dp_no, keyValue, strlen (clip (keyValue)));
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
       file_err (cc, (char *)cudp, "DBFIND");
}
/*
 * Search for Payment Terms. 
 */
void
SrchPay (void)
{
	int		i = 0;

	_work_open (3,0,40);
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

/*
 * Search for Selling Terms. 
 */
void
SrchSell (void)
{
	int		i = 0;

	_work_open (3,0,40);
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

/*
 * Search for Selling Price Types. 
 */
void
SrchPrice (void)
{
	_work_open (2,0,40);
	save_rec ("# ","#Price ");

	cc = save_rec ("1",comm_rec.price1_desc);
	cc = save_rec ("2",comm_rec.price2_desc);
	cc = save_rec ("3",comm_rec.price3_desc);
	cc = save_rec ("4",comm_rec.price4_desc);
	cc = save_rec ("5",comm_rec.price5_desc);
	cc = save_rec ("6",comm_rec.price6_desc);
	cc = save_rec ("7",comm_rec.price7_desc);
	cc = save_rec ("8",comm_rec.price8_desc);
	cc = save_rec ("9",comm_rec.price9_desc);
	cc = disp_srch ();
	work_close ();
}

/*
 * Search routine for Serial Item master file. 
 */
void
SrchInsf (
 char	*keyValue,
 int	line_no)
{
	int		diff_wh = FALSE;
	char	tmpSerialNo [26];

	_work_open (25,0,40);
	save_rec ("#      Serial Item.         ", (search_key == FN9) 
					? "#   Item Number. | Current Wh "
					: "#   Item Number. ");

	if (search_key == FN9)
		abc_selfield (insf, "insf_hhbr_id");

	insf_rec.hhwh_hash = store [line_no].hhwhHash;
	insf_rec.hhbr_hash = store [line_no].hhsiHash;
	strcpy (insf_rec.status,"F");
	sprintf (insf_rec.serial_no,"%-25.25s",keyValue);
	cc = find_rec (insf,&insf_rec,GTEQ,"r");

	while (!cc && insf_rec.status [0] == 'F' && 
		!strncmp (insf_rec.serial_no,keyValue,strlen (keyValue)))
	{
		if (search_key == FN9)
		{
			if (insf_rec.hhbr_hash != store [line_no].hhsiHash)
			{
				cc = find_rec (insf,&insf_rec,NEXT,"r");
				continue;
			}
		}
		else
		{
			if (insf_rec.hhwh_hash != store [line_no].hhwhHash)
			{
				cc = find_rec (insf,&insf_rec,NEXT,"r");
				continue;
			}
		}
		strcpy (tmpSerialNo, insf_rec.serial_no);
		if (!CheckDuplicateSerial (tmpSerialNo, store [line_no].hhsiHash,line_no))
		{
			if (insf_rec.hhwh_hash != store [line_no].hhwhHash)
				diff_wh = TRUE;	
			else
				diff_wh = FALSE;	

			if (search_key == FN9)
			{
				sprintf (err_str, 
						 "%-16.16s | %-12.12s",
					     local_rec.item_no,
			    	    (diff_wh) ? mlSoInput [3] : " ");
			}
			else
				sprintf (err_str,"%-16.16s",local_rec.item_no);

			cc = save_rec (insf_rec.serial_no,err_str);
			if (cc)
				break;
		}
		cc = find_rec (insf,&insf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	insf_rec.hhwh_hash = store [line_no].hhwhHash;
	insf_rec.hhbr_hash = store [line_no].hhsiHash;
	strcpy (insf_rec.status,"F");
	sprintf (insf_rec.serial_no,"%-25.25s",temp_str);
	cc = find_rec (insf,&insf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, insf, "DBFIND");
	
	strcpy (local_rec.serialNo, insf_rec.serial_no);

	abc_selfield (insf, "insf_id_no");
}

/*
 * Search for order number.
 */
void
SrchSohr (
 char	*keyValue)
{
	_work_open (8,0,40);
	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (sohr_rec.order_no,"%-8.8s",keyValue);

	save_rec ("#Order","#Customer Order No.   ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while (!cc && !strncmp (sohr_rec.order_no,keyValue,strlen (keyValue)) && 
		      !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no,comm_rec.est_no))
	{
		if (cumr_rec.hhcu_hash > 0L &&
		     cumr_rec.hhcu_hash != sohr_rec.hhcu_hash)
			break;

		if (cumr_rec.hhcu_hash == sohr_rec.hhcu_hash ||
			  cumr_rec.hhcu_hash == 0L)
		{
			cc = save_rec (sohr_rec.order_no, sohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (sohr_rec.order_no,"%-8.8s",temp_str);
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sohr, "DBFIND");
}

/*
 * Search for salesman.
 */
void
SrchExsf (
 char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",keyValue);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no,keyValue,strlen (keyValue)))
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
		file_err (cc, exsf, "DBFIND");
}
/*
 * Search for Special instructions. 
 */
void
SrchExsi (
	char	*keyValue)
{
	char	wk_code [4];

	_work_open (3,0,40);
	save_rec ("#No","#Instruction description.");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (keyValue);

	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");
	while (!cc && !strcmp (exsi_rec.co_no,comm_rec.co_no))
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
		file_err (cc, exsi, "DBFIND");
}
/*
 * Search for area.
 */
void
SrchExaf (
 char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Area.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",keyValue);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code,keyValue,strlen (keyValue)))
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
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

/*
 * Search for Zome Master.
 */
void
SrchTrzm (
 char *keyValue)
{
	_work_open (6,0,40);

	save_rec ("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", keyValue);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.est_no) &&
				  !strncmp (trzm_rec.del_zone, keyValue, strlen (keyValue)))
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
		file_err (cc, (char *)trzm, "DBFIND");

	return;
}
/*
 * Search for carrier code.
 */
void
SrchTrcm (
 char	*keyValue)
{
	char	key_string [31];
	long 	currentZoneHash	=	trzm_rec.trzm_hash;

	_work_open (20, 11, 50);

	save_rec ("#Carrier","# Rate Kg. | Carrier Name.");
	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", keyValue);
	cc = find_rec (trcm, &trcm_rec,GTEQ,"r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) && 
		      	  !strncmp (trcm_rec.carr_code, keyValue,strlen (keyValue)))
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
		file_err (cc, (char *)trcm, "DBFIND");

	trzm_rec.trzm_hash	=	atol (temp_str + 12);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (!cc)
		sprintf (local_rec.defaultDelZone, "%-6.6s", trzm_rec.del_zone);

}
/*
 * Add warehouse record for current W/H. 
 */
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

/*
 * Input responses to stock quantity on hand less-than input quantity. 
 */
int
InputResponce (
 void)
{
	int		i;
	int		fs_flag = FALSE;
	int		displayed = FALSE;
	float	wk_qty;
	char	val_keys [300];
	char	disp_str [300];

	cc = 0;
	
	if (local_rec.schedOrder [0] == 'Y' && inmr_rec.bo_flag [0] == 'N')
	{
		/*
		 * %s (C)ancel%s  %s (R)educe%s  %sDisp (N) (A)%s  
		 */
		strcpy (val_keys, "CcNnAaRr");
		sprintf (disp_str, ML (mlSoMess383),
				ta [8], ta [9], ta [8], ta [9],
				ta [8], ta [9]);
	}
	else
	{
		/*
		 *%s (O)verride%s  %s (C)ancel%s  %s (R)educe%s  %sDisp (N) (A)%s  
		 */
		if (envVar.overrideQuantity [0] == 'Y')
		{
			strcpy (val_keys, "OoCcNnAaRr");
			sprintf (disp_str, ML (mlSoMess384),
					ta [8], ta [9], ta [8], ta [9],
					ta [8], ta [9], ta [8], ta [9]);
		}
		else
		{
			strcpy (val_keys, "CcNnAaRr");
			sprintf (disp_str, ML (mlSoMess383),ta [8],ta [9],ta [8],ta [9],ta [8],ta [9]);
		}
	}
			
	if (strcmp (inmr_rec.alternate,sixteen_space))
	{
		/*
		 * %s (S)ubstitute%s  
		 */
		sprintf (err_str, ML (mlSoMess385),ta [8], ta [9]);
		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}

	if (BO_OK)
	{
		if (FULL_BO)
		{
			/*
			 * %s (F)orce b/o%s  
			 */
			sprintf (err_str, ML (mlSoMess381), ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "Ff");
		}
		else
		{
			/*
			 * %s (B)ackorder bal%s  %s (F)orce b/o%s 
			 */
			sprintf (err_str, ML (mlSoMess382),
				ta [8], ta [9], ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "BbFf");
		}
	}

	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, 2);
		BusyFunction (FALSE);
		switch (i)
		{
		/*
		 * Accept Quantity input. 
		 */
		case	'O':
		case	'o':
			break;

		case	'B':
		case	'b':
			local_rec.qty_back 	= local_rec.qty_ord;
			local_rec.qty_ord 	= ToLclUom (SR.qtyAvailable);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			local_rec.qty_back -= local_rec.qty_ord;

			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_back");
			fs_flag = TRUE;
			break;

		/*
		 * Cancel Quantity input and check if log to lost sale. 
		 */
		case	'C':
		case	'c':
			LogLostSales (ToStdUom (local_rec.qty_ord));
			local_rec.qty_ord = 0.00;
			sprintf (local_rec.item_no, "%-16.16s", " ");
			skip_entry = goto_field (label ("qty_ord"), label ("item_no"));
			if (prog_status == ENTRY)
				blank_display ();
			break;

		/*
		 * Display Stock Status Window. 
		 */
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
			BusyFunction (TRUE);
			if (!wpipe_open)
			{
				if (OpenStockWindow ())
					break;
			}
			if (i == 'N' || i == 'n')
				fprintf (pout,"%10ld%10ld\n", SR.hhsiHash,
						      ccmr_rec.hhcc_hash);
			else
				fprintf (pout,"%10ld%10ld\n",SR.hhsiHash,0L);

			fflush (pout);
			IP_READ (namesPipeFileName);
			BusyFunction (FALSE);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*
		 * Quantity has been reduced to equal quantity on hand. 
		 */
		case	'R':
		case	'r':
			wk_qty = ToStdUom (local_rec.qty_ord);
			local_rec.qty_ord = ToLclUom (SR.qtyAvailable);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			LogLostSales (wk_qty - ToStdUom (local_rec.qty_ord));
			break;

		/*
		 * Substitute Alternate number. 
		 */
		case	'S':
		case	's':
			sprintf (soBonus, "%-2.2s", envVar.soSpecial);
			sprintf (err_str,"%s%s", clip (inmr_rec.alternate),
						(BONUS) ? soBonus : " ");

			sprintf (local_rec.item_no,"%-16.16s",err_str);
			SR.hhbrHash = 0L; /*.00;*/
			if (ValidateItemNumber (FALSE))
				skip_entry = goto_field (label ("qty_ord"), label ("item_no"));
			else
				skip_entry = -1;

			DSP_FLD ("item_no");
			DSP_FLD ("descr");
			break;

		case	'F':
		case	'f':
			local_rec.qty_back = local_rec.qty_ord;
			local_rec.qty_ord = 0.00;
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_back");
			fs_flag = TRUE;
			break;
		}
		print_at (2,1,"%90.90s"," ");

		if (i != 'D' && i != 'd')
			break;
	}

#ifdef GVISION
	HideStockWindow ();
#else
	if (displayed)
		ClearWindow ();
#endif	/* GVISION */

	return ((envVar.fullSupplyOrder) ? fs_flag : FALSE);
}

#ifndef GVISION
int
OpenStockWindow (
 void)
{
	namesPipeFileName = IP_CREATE (getpid ());
	if (namesPipeFileName < 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	if ((pout = popen ("so_pwindow","w")) == 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	wpipe_open = TRUE;
	fprintf (pout, "%06d\n", getpid ());
	return (EXIT_SUCCESS);
}
#endif	/* GVISION */

/*
 * Log lost sales from stock quantity on hand less-than input quantity. 
 */
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

	/*
	 * Log to lost sales <y/n> ?\007 
	 */
	i = prmptmsg (ML (mlStdMess177),"YyNn",1,2);
	BusyFunction (TRUE);
	if (i == 'N' || i == 'n')
	{
		BusyFunction (FALSE);
		return;
	}

	sprintf (shhbr_hash,"%09ld",alt_hash (inmr_rec.hhbr_hash,
					     inmr_rec.hhsi_hash));
	sprintf (shhcc_hash,"%09ld",incc_rec.hhcc_hash);
	sprintf (shhcu_hash,"%09ld",cumr_rec.hhcu_hash);
	sprintf (wk_qty,"%10.2f",lost_qty);

	/*
	 * If envVar.dbMcurr convert sales value into local currency. 
	 */
	if (envVar.dbMcurr && pocrRec.ex1_factor != 0.00)
	{
		sprintf (wk_value, "%10.2f",
			no_dec (soln_rec.sale_price / pocrRec.ex1_factor));
	}
	else
		sprintf (wk_value,"%10.2f",soln_rec.sale_price);

	* (arg) = "so_lostsale";
	* (arg+ (1)) = shhbr_hash;
	* (arg+ (2)) = shhcc_hash;
	* (arg+ (3)) = shhcu_hash;
	* (arg+ (4)) = sohr_rec.area_code;
	* (arg+ (5)) = sohr_rec.sman_code;
	* (arg+ (6)) = wk_qty;
	* (arg+ (7)) = wk_value;
	* (arg+ (8)) = "F";
	* (arg+ (9)) = (char *)0;
	shell_prog (2);
	ClearWindow ();
	BusyFunction (FALSE);
	return;
}

/*
 * Setup default values for Order.
 */
void
SetOrderDefaults (
 int	newOrder)
{
	int		i;

	if (newOrder)
	{
		/*
		 * set auto flags to advert_levy flag
		 */
		init_vars (FREIGHT_SCN);	
		init_vars (MISC_SCN);	

		sohr_rec.chg_hhcu_hash	=	0L;

		strcpy (sohr_rec.area_code,cumr_rec.area_code);
		strcpy (sohr_rec.sman_code,cumr_rec.sman_code);
		strcpy (sohr_rec.pri_type,cumr_rec.price_type);
		strcpy (local_rec.priceDesc, sohr_rec.pri_type);
		strcpy (local_rec.priceFullDesc, GetPriceDesc (atoi (sohr_rec.pri_type)));
		strcpy (local_rec.schedOrder, "N");

		/*
		 * Get any special instrunctions. 
		 */
		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg1;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [0],"%60.60s", (cc) ? " " : exsi_rec.inst_text);
		
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg2;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [1],"%60.60s", (cc) ? " " : exsi_rec.inst_text);
		
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg3;
		cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
		sprintf (local_rec.spinst [2],"%60.60s", (cc) ? " " : exsi_rec.inst_text);
		
		abc_fclose (exsi);
		strcpy (sohr_rec.del_name,cumr_rec.dbt_name);
		/*
		 * Get charge to address. 
		 */
		strcpy (sohr_rec.del_add1,cumr_rec.dl_adr1);
		strcpy (sohr_rec.del_add2,cumr_rec.dl_adr2);
		strcpy (sohr_rec.del_add3,cumr_rec.dl_adr3);

		strcpy (sohr_rec.cons_no,sixteen_space);
		strcpy (sohr_rec.fix_exch,"N");
		strcpy (sohr_rec.area_code,cumr_rec.area_code);
		strcpy (sohr_rec.sman_code,cumr_rec.sman_code);
		strcpy (sohr_rec.pri_type,cumr_rec.price_type);

		sohr_rec.dt_required = local_rec.lsystemDate;
		strcpy (sohr_rec.sell_terms,"   ");
		sprintf (sohr_rec.pay_term,"%-40.40s",cumr_rec.crd_prd);
		strcpy (sohr_rec.cont_name, cumr_rec.contact_name);
		strcpy (sohr_rec.cont_phone, cumr_rec.phone_no);

		strcpy (sohr_rec.sohr_new,"Y");
		strcpy (sohr_rec.full_supply,"N");
		sprintf (sohr_rec.ins_det,"%-30.30s"," ");

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (sohr_rec.pay_term,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (sohr_rec.pay_term,"%-40.40s",p_terms [i]._pterm);
				break;
			}
		}

		sohr_rec.insurance 	= 0.00;
		sohr_rec.deposit 	= 0.00;
		sohr_rec.discount 	= 0.00;
		sohr_rec.freight 	= 0.00;

		sohr_rec.other_cost_1 = 0.00;
		sohr_rec.other_cost_2 = 0.00;
		sohr_rec.other_cost_3 = 0.00;
	}
	else
	{
		strcpy (local_rec.priceDesc, sohr_rec.pri_type);
		strcpy (local_rec.priceFullDesc, GetPriceDesc (atoi (sohr_rec.pri_type)));
		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,sohr_rec.dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
			sprintf (cudp_rec.dp_name,"%40.40s"," ");

		strcpy (local_rec.ord_desc, (sohr_rec.ord_type [0] == 'D') 
						? mlSoInput [1] : mlSoInput [2]);
		strcpy (local_rec.ordFullDesc, local_rec.ord_desc);

		/*
		 * If maintenance then need to re-read contract 
		 */
		if (strcmp (sohr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
				file_err (cc, (char *)cnch, "DBFIND");

			sprintf (local_rec.cont_desc, "%-30.30s", cnch_rec.desc);
		}
		else
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
		}
	}
	strcpy (local_rec.priceDesc, sohr_rec.pri_type);
	strcpy (local_rec.priceFullDesc, GetPriceDesc (atoi (sohr_rec.pri_type)));
	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		if (!strncmp (sohr_rec.sell_terms,STerms [i]._scode,
						strlen (STerms [i]._scode)))
		{
			sprintf (local_rec.sell_desc,"%-30.30s", STerms [i]._sdesc);
			break;
		}
	}
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,sohr_rec.sman_code);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exsf_rec.salesman,"%40.40s"," ");

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,sohr_rec.area_code);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exaf_rec.area,"%-40.40s"," ");


	if (newOrder)
	{
		strcpy (sohr_rec.frei_req, cumr_rec.freight_chg);
		strcpy (sohr_rec.carr_code, "    ");
	}
	OpenTransportFiles ("trzm_id_no");
		
	strcpy (trzm_rec.co_no,comm_rec.co_no);
	strcpy (trzm_rec.br_no,comm_rec.est_no);
	strcpy (trzm_rec.del_zone, (newOrder) ? 
						cumr_rec.del_zone : sohr_rec.del_zone);

	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (trzm_rec.del_zone, "      ");
		trzm_rec.trzm_hash	=	0L;
		trzm_rec.dflt_chg	=	0.0;
		trzm_rec.chg_kg		=	0.0;
	}
	strcpy (sohr_rec.del_zone,  trzm_rec.del_zone);
	strcpy (trcm_rec.carr_code, sohr_rec.carr_code);
	sprintf (trcm_rec.carr_desc,"%40.40s", " ");
	est_freight 		= 	0.00;
	trcm_rec.trcm_hash	=	0L;

	if (strcmp (sohr_rec.carr_code, "    ") && trzm_rec.trzm_hash > 0L)
	{
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		strcpy (trcm_rec.carr_code, sohr_rec.carr_code);
		cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, (char *)trcm, "DBFIND");
		
		trcl_rec.trcm_hash = trcm_rec.trcm_hash;
		trcl_rec.trzm_hash = trzm_rec.trzm_hash;
		cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, (char *)trcl, "DBFIND");

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
	if (sohr_rec.chg_hhcu_hash != 0L)
	{
		cumr2_rec.hhcu_hash	=	sohr_rec.chg_hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
			sprintf (local_rec.chargeToName, "%-35.35s", cumr2_rec.dbt_name);
			sohr_rec.chg_hhcu_hash	=	cumr2_rec.hhcu_hash;
		}
	}
	strcpy (local_rec.origDelRequired, (newOrder) ? "N" : sohr_rec.del_req);
	scn_set (HEADER_SCN);
}

/*
 * Clear popup window ready for _new item. 
 */
void
ClearWindow (
 void)
{
	int		i;
#ifdef GVISION
	HideStockWindow ();
#else
	for (i = 17;i < 24;i++)
	{
		move (0,i);
		cl_line ();
	}
	pr_box_lines (cur_screen);
#endif	/* GVISION */
	PrintCompanyDetails ();
}

/*
 * Warn user about something. 
 */
int
WarnUser (
 char	*warnMessage,
 int	warnFlipFlop)
{
	int		i;
	
	clear_mess ();
	print_mess (warnMessage);

	if (!warnFlipFlop)
	{
		/*
		 * Enter 'Y' if Credit is Approved / 'N' if not Appoved 
		 * 'M' for more details on credit required. ? 		
		 */
		i = prmptmsg (ML ("Enter 'Y' to continue / 'N' to cancel / 'M' for more information on credit details [Y/N/M]"),"YyNnMm",1,2);
		print_at (2,1,"%-110.110s"," ");
		if (i == 'Y' || i == 'y') 
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			/*
			 * Has Credit been Approved ? <Y/N> 
			 */
			i = prmptmsg (ML ("Do you wish to continue?"), "YyNn",1,2);
			heading (HEADER_SCN);
			scn_display (HEADER_SCN);
			print_at (2,1,"%-110.110s"," ");
			if (i == 'Y' || i == 'y') 
				return (EXIT_SUCCESS);
		}
		return (EXIT_FAILURE);
	}

	if (warnFlipFlop == 9)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

void
BusyFunction (
 int		flip)
{
	print_at (2,1,"%-90.90s"," ");
	if (flip)
		print_at (2,1, "%s\007\007", ML (mlStdMess035));

	fflush (stdout);
}

/*
 * Validate margin percent. 
 */
int
MarginCheckOk (
	double	salesMargin,
	float	discountMargin,
	double	costSaleMargin,
	float	minimumMargin,
	int		field)
{
	float	marg = 0.00;

	if (SR.contractStatus)
		return (EXIT_SUCCESS);

	if (minimumMargin == 0.00)
		return (EXIT_SUCCESS);

	if (BONUS)
		return (EXIT_SUCCESS);

	if (NO_KEY (field))
		return (EXIT_SUCCESS);

	/*
	 * If envVar.dbMcurr convert sales value into local currency. 
	 */
	if (envVar.dbMcurr)
	{
		if (pocrRec.ex1_factor != 0.00)
			salesMargin /= pocrRec.ex1_factor;
	}

	salesMargin = DOLLARS (salesMargin);
	salesMargin -= (salesMargin * ((double) discountMargin / 100));
	
	/*
	 * Calculate margin percent. 
	 */
	marg = (float) salesMargin - (float) costSaleMargin;
	if (salesMargin != 0.00)
		marg /= (float) salesMargin;
	else
		marg = 0.00;
	
	marg *= 100.00;
	
	if (marg < minimumMargin && !MARG_MESS1)
	{
		int		i;
		if (MARG_MESS2)
		{
			/*
			 * Minimum Margin for category was not obtained %c 
			 *- Press Return.", BELL						
			 */
			i = prmptmsg (ML (mlOlMess075), "YyNn", 0, 2);	
		}
		else
		{
			sprintf (err_str, ML (mlOlMess076), minimumMargin, marg);
			i = prmptmsg (err_str, "YyNn", 0, 2);	
		}
		print_at (2,0,"%-94.94s"," ");
		
		if (i == 'y' || i == 'Y')
			return (EXIT_FAILURE);
		else
			return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
CalcInputTotal (
 void)
{
	CalculateTotalBox (2,TRUE);
	if (sohr_rec.tax_code [0] == 'D')
		local_rec.discOverride = CalculateOverideDisc (tot_tot - tax_tot);
	else
		local_rec.discOverride = CalculateOverideDisc (tot_tot);

	if (local_rec.discOverride > highestDiscountPc)
		highestDiscountPc = local_rec.discOverride; 

	if (local_rec.discOverride < lowestDiscountPc)
		lowestDiscountPc = local_rec.discOverride; 

	CalculateTotalBox (2,FALSE);
}

/*
 * Calculate overide amount.
 */
float	
CalculateOverideDisc (
	double	amt)
{
	float	wk_disc = 0.00;

	if (amt == 0.00)
		return (0.00);

	wk_disc = (float) (OVER_AMT (tot_tot, local_rec.inputTotal, amt));
	wk_disc *= 100.00;
	wk_disc = (float) (twodec (wk_disc));

	return (wk_disc);
}

/*
 * Routine to check if hidden flag has been set (this is indicated by a 
 * '/H' on the end of the part number. If hidden flag is set then '/H' 
 * is removed from part number.                                       
 * Returns 0 if hidden flag has not been set, 1 if it has.           
 */
int
CheckHidden (
	char	*itemNo)
{
	char	hidden_item [17];
	char	*sptr;

	sprintf (hidden_item,"%-16.16s",itemNo);
	sptr = clip (hidden_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [2] && * (sptr + 1) == envVar.soSpecial [3])
		{
			*sptr = '\0';
			sprintf (itemNo,"%-16.16s",hidden_item);
			if (sohr_rec.prt_price [0] == 'Y')
			{
				/*
				 * Acknowledge hidden item but invoice is set to print 
				 * prices so no hidden items not allowed.			  
				 */
				print_mess (ML (mlSoMess055)); 
				sleep (sleepTime);
				return (EXIT_SUCCESS);
			}
			else
				return (EXIT_FAILURE);
		}
	}
	sprintf (itemNo,"%-16.16s",hidden_item);
	return (EXIT_SUCCESS);
}
/*
 * Routine to check if bonus flag has been set (this is indicated by a 
 * '/B' on the end of the part number. If bonus flag is set then '/B' 
 * is removed from part number.                                      
 * Returns 0 if bonus flag has not been set, 1 if it has.           
 */
int
CheckBonusItem (
	char	*itemNo)
{
	char	bonus_item [17];
	char	*sptr;

	sprintf (bonus_item,"%-16.16s",itemNo);
	sptr = clip (bonus_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [0]  && * (sptr + 1) == envVar.soSpecial [1])
		{
			*sptr = '\0';
			sprintf (itemNo,"%-16.16s",bonus_item);
			return (EXIT_FAILURE);
		}
	}
	sprintf (itemNo,"%-16.16s",bonus_item);
	return (EXIT_SUCCESS);
}
/*
 * Process customer sales history. 
 */
void
proc_cusa (
	int _key)
{
	char	disp_str [200];
	
	open_rec (cush,cush_list,CUSH_NO_FIELDS,"cush_id_no");

	if (_key == 0)
	{
		cush_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cush_rec.line_no = 0;
		cc = find_rec ("cush", &cush_rec, GTEQ, "r");
		if (!cc && cush_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			/*
			 * NOTE : Active Keys [Window Active 1] & [Window Active 2]
			 * Sales History use [NEXT][PREV]						  
			 */
			print_at (3,1,"%-90.90s"," ");
			print_at (3,1, ML (mlSoMess283));
			fflush (stdout);
		}
		abc_fclose (cush);
		return;
	}

	Dsp_open (0,17,2);
	Dsp_saverec (" #  |     ITEM NO.     |            DESCRIPTION                   | QUANTITY |  EXT PRICE. |  DISCOUNT | SALE DATE. ");
	Dsp_saverec ("");
	Dsp_saverec (" [NEXT SCN][PREV SCN][INPUT/END]");
	
	cush_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cush_rec.line_no = 0;
	cc = find_rec ("cush", &cush_rec, GTEQ, "r");
	while (!cc && cush_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		sprintf (disp_str," %02d ^E %-16.16s ^E %-40.40s ^E %8.2f ^E %11.2f ^E %9.2f ^E %-10.10s ",
				cush_rec.line_no + 1,
				cush_rec.item_no,
				cush_rec.item_desc,
				cush_rec.item_qty,
				DOLLARS (cush_rec.item_price),
				cush_rec.item_disc,
				DateToString (cush_rec.pur_date));

		Dsp_saverec (disp_str);
		cc = find_rec ("cush", &cush_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();

	crsr_off ();
	ClearWindow ();
	crsr_on ();
	abc_fclose (cush);
}

void
PrintCompanyDetails (
 void)
{
	sprintf (err_str, ML (mlStdMess038),
						comm_rec.co_no,comm_rec.co_short);
	print_at (22,0, "%s", err_str);

	sprintf (err_str, ML (mlStdMess039),
						comm_rec.est_no,comm_rec.est_short);
	print_at (22,31, "/%s", err_str);

	sprintf (err_str, ML (mlStdMess099),
						comm_rec.cc_no,comm_rec.cc_short);
	print_at (22,60, "/%s", err_str);

	sprintf (err_str, ML (mlStdMess127),
						cudp_rec.dp_no,cudp_rec.dp_short);
	print_at (22,90, "/%s", err_str);
}

/*
 * Specific code to handle single level Bills.
 */
void
ProcessKitItem (
	long		hhbrHash,
	float		qty)
{
	int		i;
	int		this_page;
	char	*sptr;
	long	hold_date = sohr_rec.dt_required;
	float	lcl_qty	=	0.00;
	
	PrevKit = TRUE;

	this_page = line_cnt / TABLINES;

	cc = open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_id_no");
	if (cc)
		file_err (cc, (char *)sokt, "OPEN_REC");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = hhbrHash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
		   sokt_rec.hhbr_hash == hhbrHash)
	{
		abc_selfield (inmr, "inmr_hhbr_hash");

		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.bonus [0] == 'Y')
		{
			sprintf (soBonus, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), soBonus);
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
			strcpy (local_rec.serialNo, serialSpace);
			strcpy (SR.serialNo, local_rec.serialNo);
			DSP_FLD ("ser_no");
		}

		if (SERIAL)
		{
			lcl_qty = sokt_rec.matl_qty * qty;
			local_rec.qty_ord = 1.00;
			local_rec.qty_back = 0.00;
			local_rec.line_no 	=	line_cnt + 1;
			soln_rec.due_date = sohr_rec.dt_required;
		}
		else
		{
			local_rec.qty_ord = ToLclUom (sokt_rec.matl_qty * qty);
			local_rec.qty_back = 0.00;
			soln_rec.due_date = sohr_rec.dt_required;
			local_rec.line_no 	=	line_cnt + 1;
			/* local_rec.line_no++;	 */
		}

		if (local_rec.qty_ord == 0.00)
			get_entry (label ("qty_ord"));

		DSP_FLD ("qty_ord");	
		/*if (local_rec.qty_ord == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}*/
		if (sokt_rec.due_date == 0L)
			sohr_rec.dt_required = hold_date;
		else
			sohr_rec.dt_required = sokt_rec.due_date;

		if (sohr_rec.dt_required == 0L)
			soln_rec.due_date = local_rec.lsystemDate;
		else
			soln_rec.due_date = sohr_rec.dt_required;

		strcpy(local_rec.LL ,"N");
		DSP_FLD ("LL");

		/*
		 * if serial we need to to load one line per qty ordered.
		 */
		if (SERIAL)
		{
			int	count;
			for (count = 0; count < lcl_qty; count++)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");

				if (sokt_rec.bonus [0] == 'Y')
				{
					sprintf (soBonus, "%-2.2s", envVar.soSpecial);
					sptr = clip (inmr_rec.item_no);
					sprintf (local_rec.item_no, "%-s%-.*s",
						     sptr, 16 - (int) strlen (sptr), soBonus);
				}
				else
					strcpy (local_rec.item_no, inmr_rec.item_no);

				dflt_used = FALSE;

				if (ValidateItemNumber (FALSE))
					break;

				for (i = label ("qty_ord"); i <= label ("ser_no"); i++)
				{
					skip_entry = 0;
					do
					{
						if (SERIAL && i == label ("ser_no"))
							get_entry (i);

						cc = spec_valid (i);
						/*
						 * if spec_valid returns 1, re-enter field 
						 * eg. if kit item has no sale value,     
						 * re-prompt for sal value if required.  
						 */
						if (cc && !(SERIAL && i == label ("ser_no")))
							get_entry (i);

					} while (cc);
					i += skip_entry;
				}
			
				putval (line_cnt);

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					lcount [ITEM_SCN] = line_cnt;
					this_page = line_cnt / TABLINES;
				}
				lcount [ITEM_SCN] = line_cnt;
				
				line_display ();
				line_cnt++;
			}
		}
		else
		{
			for (i = label ("qty_ord"); i <= label ("ser_no"); i++)
			{
				skip_entry = 0;
				do
				{
					if (SERIAL && i == label ("ser_no"))
						get_entry (i);

					cc = spec_valid (i);
					/*
					 * if spec_valid returns 1, re-enter field 
					 * eg. if kit item has no sale value,   
					 * re-prompt for sal value if required.
					 */
					if (cc && !(SERIAL && i == label ("ser_no")))
						get_entry (i);
				} while (cc);
				i += skip_entry;
			}
		
			putval (line_cnt);

			if (this_page != (line_cnt / TABLINES))
			{
				scn_write (cur_screen);
				lcount [ITEM_SCN] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			lcount [ITEM_SCN] = line_cnt;
			
			line_display ();
			line_cnt++;
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [ITEM_SCN] = line_cnt;
	abc_fclose (sokt);
	sohr_rec.dt_required = hold_date;
	PrevKit = FALSE;
}

/*
 * Specific code to handle single level Bills.
 */
float	
ProcessPhantom (
	long	hhbrHash,
	long	hhccHash)
{
	int		ws_first_time = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;
			
	float	realCommitted;

	open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = hhccHash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
			memset (&incc_rec, 0, sizeof (incc_rec));
	
		realCommitted = RealTimeCommitted 
						(
							sokt_rec.hhbr_hash,
							hhccHash,
							0
						);

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
		if (ws_first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		ws_first_time = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

void
tab_other (
 int line_no)
{
	clear_mess ();
	if (local_rec.inputTotal != 0.00)
		FLD ("disc") = NA;
	else
		FLD ("disc") = discountFieldMode;

	crsr_off ();

	if (cur_screen == ITEM_SCN)
	{
		TidySonsScreen ();

		/*
		 * Display supplying br/wh if different from current.    
		 */
		if (store [line_no].hhccHash == ccmr_rec.hhcc_hash ||
	    	line_no >= lcount [ITEM_SCN])
		{
			print_at (5, 0, "%-60.60s", " ");
		}
		else
			print_at (5, 0, ML (mlSoMess118), local_rec.sup_br, local_rec.sup_wh);

		/*
		 * Display the Second Item Description 
		 */
		if (strcmp (store [line_no]._desc2, ns_space))
			print_at (6,0, ML (mlStdMess250), store [line_no]._desc2); 

		/*
		 * Set the required field for ALL screen2 fields to either
		 * what we saved or NA.       
		 */
		if (store [line_no].canEdit)
		{
			set_scn2_vars (HEADER_SCN);	/* Recover */
			if (store [line_no].hhwhHash < 0L)
			{
				if (NEED (label ("ser_no")))
					FLD ("ser_no") = NA;
			}
			else
			{
				if (FLD ("ser_no") == NA)
					FLD ("ser_no") = YES;
			}

			/*
			 * turn off and on editing
			 * of fields depending on
			 * whether contract or not
			 */
			if (store [line_no].contractStatus)
			{
				FLD ("disc") 		= NA;
				FLD ("sale_price") 	= NA;
			}
			else
			{
				FLD ("disc") 		= orig_disc;
				FLD ("sale_price") 	= orig_sale;
			}

			if (local_rec.schedOrder [0] == 'Y')
			{
				if (!ORD_DISPLAY)
				{
					if (cumr_rec.bo_cons [0] != 'Y')
					{
						if (!HIDE (label ("deliveryNo")))
							FLD ("deliveryNo") = NA;
						if (!HIDE (label ("due_date")))
							FLD ("due_date") = NO;
					}
					else
					{
						if (!HIDE (label ("deliveryNo")))
							FLD ("deliveryNo") = NO;
						if (!HIDE (label ("due_date")))
							FLD ("due_date") = NO;
					}
				}
			}
			else
			{
				if (!ORD_DISPLAY)
				{
					if (!HIDE (label ("deliveryNo")))
						FLD ("deliveryNo") = NA;
					if (!HIDE (label ("due_date")))
						FLD ("due_date") = dueDateReqd;
				}
			}
		}
		else
		{
			sprintf 
			(
				err_str,
				ML ("Current line not available for edit - Status [%s]"), 
				store [line_no].status
			);
			/*
			 * Line not available for editing 
			 */
			errmess (err_str);
			sleep (sleepTime);
			set_scn2_vars (ITEM_SCN);	/* Disable */
		}
		DispSONS (line_no); 

		if (store [line_no].hhslHash > 0L && 
		  (sohr_rec.status [0] == 'P' || 
		   	DESPATCHED || store [line_no].status [0] == 'D' ||
			ORD_DISPLAY || !store [line_no].canEdit))
			FLD ("item_no") = NA;
		else
			FLD ("item_no") = YES;
	}
}

/*
 * Allocate memory for warehouse node 
 */
struct	WH_LIST	*
wh_alloc (
 void)
{
	int		i;
	struct 	WH_LIST *lcl_ptr	=	NULL;

	i = 0;
	while (i < 100)
	{
		lcl_ptr = (struct WH_LIST *)malloc (sizeof (struct WH_LIST));
		if (lcl_ptr != WH_NULL)
			break;
		i++;
		sleep (sleepTime);
	}
	if (lcl_ptr == WH_NULL)
		sys_err ("Error in wh_alloc () During (MALLOC)", 12, PNAME);

	strcpy (lcl_ptr->br_no, "  ");
	strcpy (lcl_ptr->wh_no, "  ");
	sprintf (lcl_ptr->wh_name, "%-40.40s", " ");
	lcl_ptr->hhccHash = 0L;
	strcpy (lcl_ptr->orderNo, "        ");
	lcl_ptr->hhsoHash = 0L;
	strcpy (lcl_ptr->ordered_from, "N");
	lcl_ptr->whPrinterNo = 0;
	lcl_ptr->next = WH_NULL;

	return (lcl_ptr);
}

/*
 * Actions include :             
 * 0: Store all screen2 fields.  
 * 1: Recover all screen2 fields.
 * 2: Disable all screen2 fields.
 */
void
set_scn2_vars (
 int action)
{
		int	i;
		int	j;
		int	tmp_scn;

	tmp_scn = cur_screen;
	if (tmp_scn != ITEM_SCN)
		scn_set (ITEM_SCN);
	switch (action)
	{
	case	0: /* Store all */
		for (j = 0, i = label ("item_no"); i <= label ("hhwhHash"); i++, j++)
			saveRequiredFields [j] = vars [i].required;
		break;
	
	case	1: /* Recover all */
		for (j = 0, i = label ("item_no"); i <= label ("hhwhHash"); i++, j++)
			vars [i].required = saveRequiredFields [j];
		break;
	
	case	2: /* Disable all */
		for (i = label ("item_no"); i <= label ("hhwhHash"); i++)
			if (vars [i].required != ND)
				vars [i].required =  NA;
		break;
	}
	if (tmp_scn != ITEM_SCN)
		scn_set (tmp_scn);
}

/*
 * use_window is a procedure called by scrgen
 * when FN14 or FN15 is pressed.				
 * _key is normally the same as last_char		
 * but by passing it as a parameter it allows	
 * the programmer to do some sneaky things		
 */
int
use_window (
 int _key)
{
	static	long	last_hhcu;
	char	comment [132];

	if (cur_screen == HEADER_SCN)
	{
		move (21,3); line (90);
	}
	/*
	 * Only do anything when we are on screen 1 and	we've read a valid cumr
	 */
	if (cumr_rec.hhcu_hash == 0L)
	{
		last_hhcu = 0L;
		return (EXIT_SUCCESS);
	}
	if (cur_screen == ITEM_SCN)
	{
		proc_cusa (_key);
		return (EXIT_SUCCESS);
	}
	if (cur_screen != HEADER_SCN)
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
	us_pr (comment, 21, 3, 1);
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
		/*
		 * Find the NEXT / PREVIOUS record to the current one	
		 */
		cc = find_rec (cucc,&cucc_rec, (_key == FN14) ? GREATER : LT,"r");

		/*
		 * Woops, looks like we need to loop around	
		 */
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);
	}

	/*
	 * Finding Next	
	 */
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

		/*
		 * Probably the last hhcu group in the cucc	
		 * so find the last record in the file.	
		 */
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

/*
 * Search on Contract (cnch)     
 */
void
SrchCnch (
 char    *keyValue)
{
	_work_open (6,0,40);
	save_rec ("#No","#Contract Description");

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, keyValue);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no))
	{                        
		if (strncmp (cnch_rec.cont_no, keyValue, strlen (keyValue)))
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
		file_err (cc, (char *)cnch, "DBFIND");
}

/*
 * Search on UOM (inum)    
 */
void
SrchInum (
 char    *keyValue)
{
	_work_open (4,0,40);
	save_rec ("#UOM ","#Description");

	strcpy (inum2_rec.uom_group, SR.uomGroup);
	strcpy (inum2_rec.uom, 		keyValue);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, SR.uomGroup))
	{                        
		if (strncmp (inum2_rec.uom, keyValue, strlen (keyValue)))
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

	strcpy (inum2_rec.uom_group, SR.uomGroup);
	strcpy (inum2_rec.uom, 		keyValue);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*
 * Check if customer has a contract.
 */
int
CheckContract (void)
{
	abc_selfield (cncl, "cncl_hhcu_hash");

	cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
	if (cc)
	{
		abc_selfield (cncl, "cncl_id_no");
		return (EXIT_SUCCESS);
	}
	abc_selfield (cncl, "cncl_id_no");
	return (EXIT_FAILURE);
}

float	
ScreenDisc (
 float	DiscountPercent)
{
	if (envVar.reverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR.cnvFct;

	return (cnvQty);
}

float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.cnvFct;

	return (cnvQty);
}

/*
 * Input not stock description lines for non-stock products.
 */
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
			sprintf (disp_str, "%-40.40s", headerDesc [i]);
		else
			sprintf (disp_str, "%-40.40s", SR.nsDesc [i]);

		txt_pval (tx_window, disp_str, 0);
	}

	txt_edit (tx_window);

	for (i = 0; i < MAX_SONS ; i++)
	{
		if (Header)
			sprintf (headerDesc [i],"%-40.40s", txt_gval (tx_window, i + 1));
		else
			sprintf (SR.nsDesc [i], "%-40.40s", txt_gval (tx_window, i + 1));
	}

	txt_close (tx_window, FALSE);
}

/*
 * Input not stock description lines for non-stock products.
 */
void	
DispSONS (
 int	line_cnt)
{
	char	disp_str [200];

	int		i;

	if (!strcmp (SR.nsDesc [0], ns_space))
		return;
	
	for (i = 0; i < 4 ; i++)
	{
		if (i)
			sprintf (disp_str, "%38.38s: %-40.40s", ns_space, SR.nsDesc [i]);
		else
			sprintf (disp_str,ML (mlSoMess119),local_rec.item_no, SR.nsDesc [i]);

		print_at (i+2,0, "%R %-82.82s", disp_str);
	}
}

/*
 * Update purchase order non stock lines file.
 */
void	
UpdateSONS (
	int		header,
	int		line_cnt, 
	long	lineHeadHash)
{
	int	i;

	abc_selfield (sons, (header) ? "sons_id_no3" : "sons_id_no");

	for (i = 0; i < MAX_SONS; i++)
	{
		memset (&sons_rec, 0, sizeof (sons_rec));
		if (header)
			sons_rec.hhso_hash 	= lineHeadHash;
		else
			sons_rec.hhsl_hash 	= lineHeadHash;
		sons_rec.line_no 	= i;
		cc = find_rec (sons, &sons_rec, COMPARISON, "u");
		if (cc)
		{
			if (header)
				sprintf (sons_rec.desc, "%-40.40s", headerDesc [i]);
			else
				sprintf (sons_rec.desc, "%-40.40s", SR.nsDesc [i]);

			/*
			 * Add line only if it is not blank. 
			 */
			if (strcmp (sons_rec.desc, ns_space))
			{
				cc = abc_add (sons, &sons_rec);
				if (cc)
					file_err (cc, (char *)sons, "DBADD");
			}
		}
		else
		{
			if (header)
				sprintf (sons_rec.desc, "%-40.40s", headerDesc [i]);
			else
				sprintf (sons_rec.desc, "%-40.40s", SR.nsDesc [i]);

			/*
			 * Update line only if it is not blank. 
			 */
			if (strcmp (sons_rec.desc, ns_space))
			{
				cc = abc_update (sons, &sons_rec);
				if (cc)
					file_err (cc, (char *)sons, "DBUPDATE");
			}
			else
			{
				cc = abc_delete (sons);
				if (cc)
					file_err (cc, (char *)sons, "DBDELETE");
			}
		}
	}
}

/*
 * Load purchase order non stock lines file.
 */
void	
LoadSONS (
	int		header,
	int		line_cnt,
	long	lineHeadHash)
{
	int	i;

	abc_selfield (sons, (header) ? "sons_id_no3" : "sons_id_no");

	for (i = 0; i < MAX_SONS; i++)
	{
		if (header)
			sprintf (headerDesc [i], "%40.40s", " ");
		else
			sprintf (SR.nsDesc [i],  "%40.40s", " ");
	}

	if (header)
	{
		sons_rec.hhso_hash 	= lineHeadHash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhso_hash == lineHeadHash)
		{
			sprintf (headerDesc [sons_rec.line_no], "%40.40s", sons_rec.desc);

			cc = find_rec (sons, &sons_rec, NEXT, "r");
		}
	}
	else
	{
		sons_rec.hhsl_hash 	= lineHeadHash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhsl_hash == lineHeadHash)
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
	int		header,
	long	lineHeadHash)
{
	if (header)
	{
		abc_selfield (sons, "sons_id_no3");
		sons_rec.hhso_hash 	= lineHeadHash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhso_hash == lineHeadHash)
		{
			cc = abc_delete (sons);
			if (cc)
				file_err (cc, (char *)sons, "DBDELETE");

			sons_rec.hhso_hash 	= lineHeadHash;
			sons_rec.line_no 	= 0;
			cc = find_rec (sons, &sons_rec, GTEQ, "r");
		}
	}
	else
	{
		abc_selfield (sons, "sons_id_no");
		sons_rec.hhsl_hash 	= lineHeadHash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
		while (!cc && sons_rec.hhsl_hash == lineHeadHash)
		{
			cc = abc_delete (sons);
			if (cc)
				file_err (cc, (char *)sons, "DBDELETE");

			sons_rec.hhsl_hash 	= lineHeadHash;
			sons_rec.line_no 	= 0;
			cc = find_rec (sons, &sons_rec, GTEQ, "r");
		}
	}
}

/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	/*
	 * Check for Currency Code.  
	 */
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
	 * Get value of override option 
	 */
	sptr	=	chk_env ("SO_OVERRIDE_QTY");
	if (sptr == (char *)0)
		strcpy (envVar.overrideQuantity, "Y");
	else
		sprintf (envVar.overrideQuantity, "%-1.1s", sptr);

	/*
	 * Check if gst applies. 
	 */
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVar.gstApplies = 0;
	else
		envVar.gstApplies = (*sptr == 'Y' || *sptr == 'y');

	/*
	 * Get gst code. 
	 */
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

	/*
	 * Check for sales order print program. 
	 */
	sptr = chk_env ("SO_PRINT");
	if (sptr == (char *)0)
		envVar.salesOrderPrint = FALSE;
	else
	{
		envVar.salesOrderPrint = TRUE;
		sprintf (envVar.soPrintProgram, "%-14.14s", sptr);
	}

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if advertising Levy applies. 
	 */
	sptr = chk_env ("ADVERT_LEVY");
	envVar.advertLevy = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for discounts on Indent items. 
	 */
	sptr = chk_env ("SO_DIS_INDENT");
	envVar.discountIndents = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if lost sales are logged. 
	 */
	sptr = chk_env ("SO_LOST_SALES");
	envVar.lostSales = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if nett pricing is used. 
	 */
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);
	
	/*
	 * Check for Combined P/S Invoice. 
	 */
	sptr = chk_env ("COMB_INV_PAC");
	if (sptr == (char *)0)
		envVar.combineInvoicePack = FALSE;
	else
	{
		if (sptr [0] == 'Y' || sptr [0] == 'y')
			envVar.combineInvoicePack = TRUE;
		else
			envVar.combineInvoicePack = FALSE;
	}

	/*
	 * Check for Automatic freight charge. 
	 */
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for Address prompting. 
	 */
	sptr = chk_env ("DB_ADDR_PRMPT");
	envVar.addressPrompt = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for sales order full supply option.
     */
	sptr = chk_env ("SO_FULL_SUPPLY");
	envVar.fullSupplyOrder = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for sales order analysis. 
	 */
	sptr = chk_env ("SO_SALES");
	envVar.salesOrderSales = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if special codes for bonus and hidden lines are used. 
	 */
	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVar.soSpecial,"/B/H");
	else
		sprintf (envVar.soSpecial,"%-4.4s", sptr);

	/*
	 * Check for sales order margin checks. 
	 */
	sptr = chk_env ("SO_MARGIN");
	sprintf (envVar.salesOrderMargin, "%-2.2s", 
									(sptr == (char *) 0) ? "00" : sptr);

	/*
	 * Check if stock information window is displayed. 
	 */
	sptr = chk_env ("WIN_OK");
	envVar.windowPopupOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if stock information window is loaded at load time. 
	 */
	sptr = chk_env ("SO_PERM_WIN");
	envVar.perminantWindow = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envVar.perminantWindow)
	{
		if (OpenStockWindow ())
			envVar.windowPopupOk = FALSE;
	}
	/*
	 * Check and Get Credit terms. 
	 */
	sptr = get_env ("SO_CRD_TERMS");
	envVar.creditStop 	= (* (sptr + 0) == 'S');
	envVar.creditTerms 	= (* (sptr + 1) == 'S');
	envVar.creditOver 	= (* (sptr + 2) == 'S');

	/*
     * Check and Get Order Date Type. 
     */
	sptr = chk_env ("SO_DOI");
	envVar.useSystemDate = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;

	/*
	 * Check if supply from alternate warehouses is allowed.       
	 */
	sptr = chk_env ("SO_ALT_SUPP");
	if (sptr)
		envVar.supplyFromAlternate = (*sptr == 'Y');

	/* 
	 * QC module is active or not. 
	 */
	envVar.qCApplies = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/* 
	 * Whether to include QC qty in available stock. 
	 */
	envVar.qCAvailable = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*
	 * Validate is serial items allowed. 
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envVar.serialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if available stock is included. 
	 */
	sptr = chk_env ("SO_FWD_AVL");
	envVar.includeForwardStock = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Get descriptions for other costs 1-3. 
	 */
	sptr = chk_env ("SO_OTHER_1");
	sprintf (envVar.other [0],"%-30.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (envVar.other [1],"%-30.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (envVar.other [2],"%-30.30s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	/*
	 * Customer Company Owned.  
	 */
	sptr = chk_env ("DB_CO");
	envVar.dbCo = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*
	 * Customer Find variable for Search. 
	 */
	sptr = chk_env ("DB_FIND");
	envVar.dbFind = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*
	 * How if Freight Charged. 
	 */
	sptr = chk_env ("SO_FREIGHT_CHG");
	envVar.soFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);

	/*
	 * Check for Number plates.   
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envVar.skGrinNoPlate = ((sptr == (char *)0)) ? 0 : atoi (sptr);

	/*
	 * Check for 3pl Environment. 
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVar.threePl = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if works order can be created from backorder line. 
	 */
	sptr = chk_env ("SO_WO_ALLOWED");
	envVar.soWoAllowed = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if backorder lines are split. 
	 */
	sptr = chk_env ("SO_SPLIT_BO");
	envVar.soSplitBo = (sptr == (char *)0) ? 0 : atoi (sptr);
}

/*
 * Open Transport related files.
 */
void
OpenTransportFiles (
	char	*zoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, zoneIndex);
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
}
/*
 * Close Transport related files.
 */
void
CloseTransportFiles (void)
{
	abc_fclose (trzm);
	abc_fclose (trcm);
	abc_fclose (trcl);
}

/*
 * Main heading Section.
 */
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

	switch (programType)
	{
	case	INPUT:
		if (STANDARD)
		{
			/*
			 * Standard Sales Order Entry. 
			 */
		    rv_pr (ML (mlSoMess122), 25,0,1);
		}
		else
		{
			if (createStatusFlag [0] == 'R')
			{
				/*
				 * Sales Order Entry. (ONE STEP)
				 */
				rv_pr (ML (mlSoMess123), 25,0,1);
			}
			else
			{
				/*
				 * Sales Order Entry. (TWO STEP)
				 */
				rv_pr (ML (mlSoMess124), 25,0,1);
			}
		}
		break;

	case	MAINT:
		/*
		 * Sales Order Maintenance. 
		 */
		rv_pr (ML (mlSoMess125), 25,0,1);
		break;

	case	DISPLAY:
		/*
		 * Sales Order Display. 
		 */
		rv_pr (ML (mlSoMess126), 25,0,1);
		break;

	default:
		break;
	}
	print_at (0,65, ML (mlSoMess120),
					local_rec.prev_dbt_no, local_rec.previousPack);

	switch (scn)
	{
	case	HEADER_SCN:
		inScreenFlag = FALSE;
		use_window (0);
		break;

	case	ITEM_SCN:
		inScreenFlag = TRUE;
		print_at (3,1, ML (mlSoMess340));
		print_at (4,0, ML (mlStdMess012), cumr_rec.dbt_no,
					       				clip (cumr_rec.dbt_name));
		if (envVar.dbMcurr)
			print_at (4,65,"   (%-3.3s)", cumr_rec.curr_code);
		DrawTotals ();
		if (local_rec.inputTotal != 0.00 && prog_status != ENTRY)
		{
			CalcInputTotal ();
			PrintBoxTotals ();
		}
		else
			CalculateTotalBox (FALSE, FALSE);

		break;
		
	case FREIGHT_SCN:
	case MISC_SCN:
		inScreenFlag = FALSE;
		print_at (2,1, ML (mlStdMess012), cumr_rec.dbt_no,
					       				clip (cumr_rec.dbt_name));
		if (envVar.dbMcurr)
			print_at (2,65,"   (%-3.3s)", cumr_rec.curr_code);
		break;
	}


	PrintCompanyDetails ();
	/*  reset this variable for _new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*
 * Routine to get price desctiptions from comm record.
 */
const char	*
GetPriceDesc (
	int		priceNo)
{
	static	char	priceDesc [16];

	strcpy (priceDesc, " ");

	switch (priceNo)
	{
		case	1:	
			strcpy (priceDesc,	comm_rec.price1_desc);
			break;
		case	2:	
			strcpy (priceDesc, comm_rec.price2_desc);
			break;
		case	3:	
			strcpy (priceDesc, comm_rec.price3_desc);
			break;
		case	4:	
			strcpy (priceDesc, comm_rec.price4_desc);
			break;
		case	5:	
			strcpy (priceDesc, comm_rec.price5_desc);
			break;
		case	6:	
			strcpy (priceDesc, comm_rec.price6_desc);
			break;
		case	7:	
			strcpy (priceDesc, comm_rec.price7_desc);
			break;
		case	8:	
			strcpy (priceDesc, comm_rec.price8_desc);
			break;
		case	9:	
			strcpy (priceDesc, comm_rec.price9_desc);
			break;
	}
	return (priceDesc);
}

/*
 * Function to call works order creation program.
 */
void
CreateWorksOrder (
	long	hhslHash)
{
	if (!(worksOrderOutputFile = popen (worksOrderProgram, "w")))
	{
		file_err (errno, "POPEN", worksOrderProgram);
	}
			passedPrinterNo 	= 0,
	fprintf (worksOrderOutputFile, "%d\n", passedPrinterNo);
	fprintf (worksOrderOutputFile, "%ld\n", hhslHash);
	pclose (worksOrderOutputFile);
}

float	
TotalItemQty (
	long	hhbrHash,
	int		lineNumber,
	float	lineQty)
{
	int		i;
	int		noItems = (prog_status == ENTRY) ? line_cnt : lcount [ITEM_SCN];
	float	totalQty	=	lineQty;

	for (i = 0;i < noItems;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == lineNumber)
			continue;

		/*
		 * Only compare same items. 
		 */
		if (store [i].hhsiHash == hhbrHash)
		{
			if (F_HIDE (label ("UOM")))
				totalQty	+=	store [i].qtyTotal * store [i].cnvFct;
			else
				totalQty	+=	store [i].qtyTotal;
		}
	}
	return (totalQty);
}

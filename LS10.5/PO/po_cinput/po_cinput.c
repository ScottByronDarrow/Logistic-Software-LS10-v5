/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_cinput.c,v 5.22 2002/07/24 08:39:03 scott Exp $
|  Program Name  : (po_cinput.c     )                                 |
|  Program Desc  : (Purchase Order Input.                         )   |
|---------------------------------------------------------------------|
|  Date Written  : 22/06/90        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: po_cinput.c,v $
| Revision 5.22  2002/07/24 08:39:03  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.21  2002/07/18 07:00:26  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.20  2002/07/17 09:57:33  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.19  2002/06/20 07:22:03  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.18  2002/06/19 07:00:33  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.17  2002/03/05 06:12:32  scott
| Updated to add check on date raised.
|
| Revision 5.16  2002/01/31 07:03:33  cha
| S/C 739. Updated to put validation to terms of order.
|
| Revision 5.15  2002/01/31 06:29:32  cha
| S/C 740. Updated to add validation for Supplier Payment Terms.
|
| Revision 5.14  2002/01/23 06:11:00  scott
| S/C 00697 - When Maintaining a purchase order under the 3PL environment
| and press return on the item number the FOB will appear in error.
|
| Revision 5.13  2001/11/29 01:25:16  scott
| Updated to use new validation for creation of fractions on UOM conversion.
|
| Revision 5.12  2001/11/07 06:04:06  scott
| Updated to ensure discount changed when quantity changed.
|
| Revision 5.11  2001/11/05 01:37:15  scott
| Updated from testing.
|
| Revision 5.10  2001/10/30 02:09:51  cha
| Fix  Issue #00640 and #00641. Changes done by Scott.
|
| Revision 5.9  2001/10/25 01:44:32  scott
| Updated to ensure spec_valid () is called when item changed.
| Updated to ensure weight is re-read when UOM changed.
| Updated as supplier UOM not displayed correctly on edit.
|
| Revision 5.8  2001/10/09 05:13:40  cha
| Updated to apply changes made Robert in
| version 5.5. Changes cusOrdRef to TAB.
|
| Revision 5.7  2001/10/05 02:48:35  cha
| Changes made to cater for goods returns processing.
|
| Revision 5.4  2001/09/26 23:10:01  scott
| Updated from Scotts Machine
|
| Revision 5.3  2001/08/20 23:36:14  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 09:15:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:09  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.9  2001/05/12 03:44:45  scott
| Updated to define DPP (Decimal Place Precission) #define DPP(x) ndec (x,5)
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_cinput.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_cinput/po_cinput.c,v 5.22 2002/07/24 08:39:03 scott Exp $";

#define MAXWIDTH 	400
#define MAXLINES 	2000
#define	TXT_REQD
#define LOCAL       (pohr_rec.sup_type [0] == 'L')
#include 	<pslscr.h>
#include 	<getnum.h>
#include	<twodec.h>
#include	<s_terms.h>
#include	<get_lpno.h>
#include	<inis_update.h>
#include	<ml_po_mess.h>
#include	<ml_std_mess.h>

#define	PO_HEAD		1
#define	PO_COST		2
#define	PO_LINES	3

#define	CASE_USED	 (envVar.poCaseUsed [0] == 'Y')
#define	INIS_REQ 	 (envVar.poInisReq [0] == 'Y')

#define	FGN_CURR	 (strcmp (sumr_rec.curr_code, currencyCode))

#define	SR			store [line_cnt]
#define	LSR			store [lcount [PO_LINES]]

#define	POGD		store2 [line_cnt]

#define NO_KEY(x)	 (vars [x].required == NA || \
			  		 (vars [x].required == NI && prog_status == ENTRY) || \
			  		  vars [x].required == ND)

#define HIDE(x)	 	(vars [x].required == ND)
#define NEED(x)	 	(vars [x].required == YES)

#define	FOB		0
#define	FRT		1
#define	INS		2
#define	INT		3
#define	B_C		4
#define	DTY		5
#define	O_1		6
#define	O_2		7
#define	O_3		8
#define	O_4		9
#define	MAX_POGD	10

#define	DBOX_TOP	9
#define	DBOX_LFT	35
#define	DBOX_WID	68
#define	DBOX_DEP	3

#define	GOODS_VAL	 (store2 [FOB].s_inv_value)
#define	FRT_INS_VAL	 (store2 [FRT].s_inv_value + store2 [INS].s_inv_value)
#define	FIN_CHG_VAL	 (store2 [INT].s_inv_value + store2 [B_C].s_inv_value)
#define	DUTY_VAL	 (store2 [DTY].s_inv_value)
#define	OTHER_VAL	 (store2 [O_1].s_inv_value + store2 [O_2].s_inv_value +\
					  store2 [O_3].s_inv_value + store2 [O_4].s_inv_value)

#define	MAX_Pons	10

extern	int		_win_func;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct exsiRecord	exsi_rec;
struct inisRecord	inis_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct inmrRecord	inmr_rec;
struct inspRecord	insp_rec;
struct inuvRecord	inuv_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct ineiRecord	inei_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct pohrRecord	pohr3_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct poshRecord	posh_rec;
struct poslRecord	posl_rec;
struct posdRecord	posd_rec;
struct pocrRecord	pocr_rec;
struct polhRecord	polh_rec;
struct podtRecord	podt_rec;
struct pocfRecord	pocf_rec;
struct pogdRecord	pogd_rec;
struct sudsRecord	suds_rec;
struct suinRecord	suin_rec;
struct insfRecord	insf_rec;
struct poliRecord	poli_rec;
struct ponsRecord	pons_rec;


int		*comr_po_sic	=	&comr_rec.po_sic1;
int		*sumr_sic		=	&sumr_rec.sic1;

/*=============
| Table names |
=============*/
static char	*data	= "data",
			*pohr2	= "pohr2",
			*pohr3	= "pohr3",
			*poln2	= "poln2",
			*inum2	= "inum2",
			*DBFIND = "DBFIND",
			*fifteenSpaces	=	"               ";

	char	*currentUser;

	int		newPurchaseOrder,
			deleteLine,
			updateSupplierRecord,
			updateShipment		= FALSE,
			calculateDiscount	= FALSE,
			warehouseSelected	= FALSE,
			printerNumber		= 0,
			heldOrder			= FALSE,
			virLineCnt 			= 0;

	char	branchNumber [3],
			argumentFlag [6],
			poPrintProgram [15];

	long 	longSystemDate;

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	char	poCaseUsed [2];
	char	poInisReq [2];
	char	poReorder [27];
	char	supOrdRound [2];
	double	poAppVal;
	int		IkeaPoNumbers;
	int		IkeaSystem;
	int		allowZeroCost;
	int		crCo;
	int		crFind;
	int		poAppFlag;
	int		poConvSupUom;
	int		poCostScreen;
	int		poIndentOk;
	int		poInput;
	int		poMaxItemPo;
	int		poMaxLines;
	int		poNumGen;
	int		poCostCalc;
	int		poOverride;
	int		poPrint;
	int		poUomDefault;
	int		soDiscRev;
	int		threePlSystem;
}	envVar;

/*--------------------------------------------
| Structure used for pop-up discount screen. |
--------------------------------------------*/
typedef struct tagPromptInfo {
  char*     fldPrompt;
  int       xPos;
  char      fldMask [16];
} __PromptInfo;

__PromptInfo discScn[7];

	char	cat_desc [10] [21];
	char	*invoiceCategory [] =
			{
				"Goods (FOB)",
				"O/S Freight",
				"O/S Insurance",
				"O/S Interest",
				"O/S Bank Charges",
				"Duty",
				"Other - 1",
				"Other - 2",
				"Other - 3",
				"Other - 4",
			};

	char	currencyCode [4];
	char	*screens [] =
			{
				" Header Screen ",
				" Costing Details ",
				" Item Details ",
			};

	char	localPrompt 	[21];
	char	freightPrompt 	[21];
	char	cifPrompt 		[21];
	char	dutyPrompt 		[21];
	char	licencePrompt 	[21];

	char	prefixLocPo 	[16],
			prefixFgnPo 	[16];
	int		origDueDate	=	0,
			origUOM		=	0,
			origQty		=	0;


	FILE 	*pout;
	int		running = 0;

	char	*serialSpace 	= "                         ";
	char	*nonStockSpace 	= "                                        ";

	double	totalWeight	=	0.00,
			totalVolume	=	0.00;

struct store2Rec{
	char	s_curr [4];
	double	s_exch;
	double	s_inv_value;		/* invoice value (less gst)		*/
	double	s_item_value;		/* extension of items & fob's	*/
	int		inv_found;			/* Invoice found on suin.		*/
	long	hhsu_hash;			/* supplier hash				*/
	char	s_alloc [2];		/* allocation D / W / V			*/
	char	s_cost_edit [2];	/* Y(es) if cost edited.		*/
} store2 [TABLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct storeRec {
							/*======================================*/
	char	item_desc [41];	/* Item description.					*/
	char	nsDesc [MAX_Pons + 1] [41]; 	/*Non stock description.    */
	char	_class [2];		/* Item Class.							*/
	char	ser_item [2];	/* Serial item.  						*/
	char	ser_no [26];	/* Serial number.						*/
	long	ship_no;		/* Shipment number.						*/
	double	exch_rate;		/* Exchange Rate. 						*/
	double	quantity;		/* quantity received					*/
	double	outer;			/* outer size.       					*/
	double	imp_duty;		/* duty rate from podt					*/
	char	duty_type [2];	/* duty type from podt					*/
	char	lic_cat [2];	/* licence category from polh.  		*/
	char	lic_no [11];	/* licence no from polh.  				*/
	double	lic_hash;		/* licence hhlc_hash from polh			*/
	double	lic_rate;		/* licence rate from polh				*/
	double	grs_fgn;		/* Gross FOB before discounts 			*/
	double	net_fob;		/* Net FOB after discounts 				*/
	double	val_duty;		/* value of duty						*/
	double	val_fi;			/* value of freight + ins.				*/
	float	weight;			/* inis_weight							*/
	float	volume;			/* inis_volume							*/
	float	lead_time;		/* inis_lead_time						*/
	float	sea_time;		/* inis_sea_time						*/
	float	air_time;		/* inis_air_time						*/
	float	lnd_time;		/* inis_lnd_time						*/
	double	land_cost;		/* Landed cost.  						*/
	char	std_uom [5];	/* Standard (Stock) UOM.				*/
	char	uom_group [21];	/* Standard (Stock) UOM group.			*/
	float	StdCnvFct;		/* Standard (Stock) UOM.				*/
	char	sup_uom [5];	/* Supplier UOM.						*/
	char	inp_uom [5];	/* Input UOM.	  						*/
	float	pur_conv;		/* Input    UOM Conversion Fctr			*/
	float	sup_pur_conv;	/* Supplier UOM Conversion Fctr			*/
	long	hhumHash;		/* Input UOM Hash.            			*/
	long	sup_hhum_hash;	/* Supplier UOM Hash.            		*/
	double	cst_price;		/* Cost price.							*/
	double	base_cost;		/* Base cost from inis or inei 			*/
	int		cumulative;		/* Discounts are cumulative ?  			*/
	float	discArray [4];	/* Regulatory and Disc A, B, C percents */
	int		no_inis;		/* No Inventory supplier record (inis)  */
	int		upd_inis;		/* Update inventory supplier record.    */
	int		upd_ship;		/* Update shipment.                     */
	float	min_order,		/* stuff copied from inis_rec			*/
			ord_multiple,   /* Relates to Min and order multiple.   */
			pallet_size;    /* Relates to Max pallet size available */
	long	hhbrHash,		/* hashes remembered 					*/
			hhccHash;      /*======================================*/
} store [MAXLINES];

struct {
							/*======================================*/
	char	dummy [11];		/* Dummy Screen Gen Field.		        */
							/*======================================*/

							/*======================================*/
							/* Header Screen Local field.		    */
							/*======================================*/
	char	prev_po [16];	/* Previous Purchase order No.			*/
	char	prev_crd_no [7]; /* Previous Creditor Number.			*/
	char	systemDate [11];	/* Current Date dd/mm/yy.				*/
	char 	crd_no [7];		/* Current Creditors Number.			*/
	double	exch_rate;		/* Local Exchange Rate.					*/
	char	ship_desc [5];	/* Shipment Method Description.			*/
							/*--------------------------------------*/

							/*======================================*/
							/* Costing Screen Local field.			*/
							/*======================================*/
	char	category [21];	/* Costing Category.					*/
	char	supplier [7];	/* Costing screen Supplier.				*/
	char	std_uom [5];	/* Standard (Stock) UOM.				*/
	char	inp_uom [5];	/* Supplier UOM.						*/
	float	pur_conv;		/* Supplier UOM Conversion Factor		*/
	char	allocation [2];	/* Allocation D(ollar) W(eight) V(olume */
	char	inv_no [16];	/* Invoice Number.						*/
	char	currency [4];	/* Currency Code.						*/
	double	fgn_val;		/* Fgn Value.							*/
	double	lexch_rate;		/* Exchange Rate.						*/
	double	loc_val;		/* Local Value.							*/
							/*--------------------------------------*/

							/*======================================*/
							/* Line Item Screen Local field.		*/
							/*======================================*/
	char	item_no [17];	/* Local Item Number.					*/
	char	lic_no [11];	/* Local Licence number.				*/
	char	lic_cat [3];	/* Local Licence Category.				*/
	float	qty;			/* Local Quantity.						*/
	float	packageQty;		/* Package quantity						*/
	float	totalChargeWgt;	/* Total charge weight					*/
	float	totalGrossWgt;	/* total gross weight					*/
	float	totalCBM;		/* Total Cubic metres.					*/
	double	grs_fgn;		/* Free-on-board Fgn Dollars.			*/
	char	view_disc [2];	/* View discounts ?          			*/
	double	net_fob;		/* Net after discount (Fgn dollars)     */
	double	loc_fi;			/* Over-Seas Freight + Insurance.		*/
	double	cif_loc;		/* Cost/Insurance/Freight Local.		*/
	double	contingency;	/* Add in contingency.					*/
	double	duty_val;		/* Duty Value.							*/
	char	duty_code [3];	/* Duty Code.							*/
	double	lic_val;		/* Licance Value.						*/
	double	other;			/* Other Cost 1-4 + Bank and Interest.  */
	int		case_no;		/* Local Case Number.					*/
	double	land_cst;		/* Landed cost.							*/
	double	Dsp_land_cst;	/* Landed cost.							*/
	double	fob_cost;		/* Free-On-Bourd Cost.					*/
	long	due_date;		/* Due Date at line item Level.			*/
	char	br_no [3];		/* Branch number.						*/
	char	br_name [41];	/* Branch name.							*/
	char	wh_no [3];		/* Warehouse number.					*/
	char	wh_name [41];	/* Warehouse name.						*/
	long	hhcc_hash;		/* Warehouse hhcc_hash.					*/
	char	cusOrdRef [sizeof poln_rec.cus_ord_ref];
	char	previousCusOrdRef [sizeof poln_rec.cus_ord_ref];
							/*--------------------------------------*/
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.prev_crd_no, "Supplier Number      : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{1, LIN, "hhsu_hash",	 0, 4, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&sumr_rec.hhsu_hash},
	{1, LIN, "name",	 3, 67, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name        : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "pur_ord_no",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Purchase Order No    : ", " ",
		 NE, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no},
	{1, LIN, "date_raised",	 4, 67, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date Purchase Raised : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.date_raised},
	{1, LIN, "sup_type",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "L", "Supplier Type        : ", "Supplier Type F(oreign or L(ocal. ",
		 YES, NO,  JUSTLEFT, "FfLp", "", pohr_rec.sup_type},
	{1, LIN, "po_type",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "P/O Type (N/Q).      : ", "Purchase Order type N(ormal) or Q(uick)",
		ND, NO,  JUSTLEFT, "NQ", "", pohr_rec.stat_flag},
	{1, LIN, "ship_method",	 5, 67, CHARTYPE,
		"U", "          ",
		" ", sumr_rec.ship_method, "Shipment Method      : ", "Shipment Method L(and) / S(ea) / A(ir)",
		NO, NO,  JUSTLEFT, "LSA", "", pohr_rec.ship_method},
	{1, LIN, "ship_desc",	 5, 93, CHARTYPE,
		"UUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ship_desc},
	{1, LIN, "con_name",	 6, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", sumr_rec.cont_name, "Contact name         : ", "<RETURN> - standard contact ",
		 NO, NO,  JUSTLEFT, "", "", pohr_rec.contact},
	{1, LIN, "date_conf",	 6, 67, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Date Confirmed       : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.conf_date},
	{1, LIN, "shipmentTerms",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Terms of Order       : ", "[SEARCH] for valid Terms.",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.term_order},
	{1, LIN, "date_reqd",	 7, 67, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00 ", "Date Due Overide     : ", "Note : Due date other than 00/00/00 will overide line item due dates on lines not allocated to a shipment. ",
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.due_date},
	{1, LIN, "btpay",	 9, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Bank Terms.          : ", "",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.bnk_term_pay},
	{1, LIN, "exch_rate",	9, 67, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Exchange Rate        : ", "<RETURN> - default to current",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.exch_rate},
	{1, LIN, "stpay",	10, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", sumr_rec.pay_terms, "Supplier Terms       : ", "",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.sup_term_pay},
	{1, LIN, "pay_date",	10, 67, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Payment Date         : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.pay_date},
	{1, LIN, "req",		12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Person Requesting PO : ", "Details of person requesting purchase order. ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.req_usr},
	{1, LIN, "reason",	12, 67, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Reason for Purchase  : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.reason},
	{1, LIN, "sin1",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr. 1    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin1},
	{1, LIN, "sin2",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr. 2    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin2},
	{1, LIN, "sin3",	16, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr. 3    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin3},
	{1, LIN, "del1",	18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr. 1    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin1},
	{1, LIN, "del2",	19, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr. 2    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin2},
	{1, LIN, "del3",	20, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr. 3    : ", " ",
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin3},
	{2, TAB, "category",	MAX_POGD, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Category      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.category},
	{2, TAB, "supplier",	 0, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Supplier ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.supplier},
	{2, TAB, "hhsu_hash",	 0, 4, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&sumr2_rec.hhsu_hash},
	{2, TAB, "allocation",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", "D", "Spread", " by : D(ollar  W(eight  V(olume ",
		YES, NO,  JUSTLEFT, "DWV", "", local_rec.allocation},
	{2, TAB, "inv_no",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "  Invoice No   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inv_no},
	{2, TAB, "currency",	 0, 4, CHARTYPE,
		"UUU", "          ",
		" ", " ", " Currency. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.currency},
	{2, TAB, "fgn_val",	 0, 4, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", " Foreign Value ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.fgn_val},
	{2, TAB, "lexch_rate",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNN", "          ",
		" ", "0", " Exch. Rate ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lexch_rate},
	{2, TAB, "loc_val",	 0, 0, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "0", localPrompt, " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_val},

	{3, TAB, "br_no",	MAXLINES, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, "BR", "Enter Branch number.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{3, TAB, "br_name",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO, JUSTLEFT, "", "", local_rec.br_name},
	{3, TAB, "wh_no",	 0, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.cc_no, "WH", "Enter Warehouse number.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.wh_no},
	{3, TAB, "wh_name",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO, JUSTLEFT, "", "", local_rec.wh_name},
	{3, TAB, "hhcc_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.hhcc_hash},
	{3, TAB, "item_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{3, TAB, "hhpl_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &poln_rec.hhpl_hash},
	{3, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inp_uom},
	{3, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", " Quantity ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty},
	{3, TAB, "packageQty",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Package Qty", "Input Package quantity.",
		ND, NO, JUSTRIGHT, "1.00", "9999999.99", (char *)&local_rec.packageQty},
	{3, TAB, "totalChargeWgt",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Charge Wgt", "Input total charge weight.",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalChargeWgt},
	{3, TAB, "totalGrossWgt",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Gross Wgt.", " ",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalGrossWgt},
	{3, TAB, "totalCBM",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Total CBM.", " ",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalCBM},
	{3, TAB, "cusOrdRef",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.previousCusOrdRef, " Customer Order Ref ", "",
		ND, NO,  JUSTLEFT, "", "", local_rec.cusOrdRef},
	{3, TAB, "lic_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Lic. No. ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.lic_no},
	{3, TAB, "fob_cst",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NNNN", "          ",
		" ", "0", "  FOB (FGN)  ", "Enter Cost Per Item. (Return for Last Cost).",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.grs_fgn},
	{3, TAB, "view_disc", 0, 0, CHARTYPE,
		"U", "          ",
#ifdef GVISION
		" ", "N", "View Disc", " View and Amend Discounts (Y/N) ",
#else
		" ", "N", "V", " View and Amend Discounts (Y/N) ",
#endif
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.view_disc},
	{3, TAB, "net_fob",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNN", "          ",
		" ", "", "NET FOB(FGN)", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_fob},
	{3, TAB, "loc_fi",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNN", "          ",
		" ", "0", freightPrompt, "<Return> will Calculate Freight.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_fi},
	{3, TAB, "cif_loc",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NNNNNN", "          ",
		" ", "0", cifPrompt, " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.cif_loc},
	{3, TAB, "duty_val",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNN", "          ",
		" ", "0", dutyPrompt, "<Return> will Calculate Duty.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.duty_val},
	{3, TAB, "lic_val",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "0", licencePrompt, " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.lic_val},
	{3, TAB, "other",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNN", "          ",
		" ", "0", "Other Costs", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.other},
	{3, TAB, "land_cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNNN.NNNNNN", "          ",
		" ", "0", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.land_cst},
	{3, TAB, "Dsp_land_cost",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", "0", " Unit Cost ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.Dsp_land_cst},
	{3, TAB, "case_no",	 0, 0, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Case", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.case_no},
	{3, TAB, "due_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", " Due Date ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.due_date},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern	int	TruePosition;
int MessageCnt = 0;

#include	<proc_sobg.h>
#include	<CheckIndent.h>
#include	<ShipTerms.h>
#include	<SupPrice.h>

#include 	<FindSumr.h>

/*=======================
| Function declarations |
=======================*/
char *	GetShipmentNo		(long);
double	DollarTotal			(void);
double	DutyCalculate		(int);
double	DutyTotal			(void);
double	FreightCalculate	(int);
double	PurchaseOrderValue	(void);
double	VolumeTotal			(void);
double	WeightCalc			(int, int, double, double, double, double);
double	WeightTotal			(void);
float	RndMltpl			(float, char *, float, float);
float	ScreenDisc			(float);
float	ToLclUom			(float, float);
float	ToStdUom			(float, float);
int		CalcVirtualLines	(int);
int		CheckInvoice		(char *);
int		CheckPohr			(char *);
int		CheckPohr_c			(char *);
int		CheckReorder		(char *);
int		CheckShipment		(long);
int		CheckShipmentCosting(long);
int		FindInis			(long, long);
int		FindPocf			(char *);
int		FindPocr			(char *);
void	GetInuvWgtVol		(int, long, long, float *, float *);
int		LoadPoln			(long);
int		LoadSupplier		(int);
int		SameMissingItems	(long);
int		heading				(int);
int		spec_valid			(int);
int		win_function		(int, int, int, int);
void	AddIncc				(long, long);
void	CalculateCost		(int);
void	CalculatePosd		(long, long);
void	CheckEnvironment 	(void);
void	CleanUpPrompt		(void);
void	ClearBox			(int, int, int, int);
void 	ClosePrint			(void);
void	CloseDB				(void);
void	DeleteInsf			(void);
void	DeletePons			(long);
void	DispFields			(int);
void	DisplayScreen		(int);
void	DrawDiscScn			(void);
void	FindExsi			(void);
void	GetWarehouse		(long);
void	InitML				(void);
void	InputField			(int);
void	InputPons			(int);
void	LoadCategoryDesc	(void);
void	LoadInvoiceScreen	(void);
void	LoadPons			(int, long);
void	OpenDB				(void);
void	OpenMissingItems	(char  *);
void	PrintCostDesc		(int);
void	PrintOtherStuff		(void);
void	ReadMisc			(void);
void	RecalcCosting		(void);
void	RunningWVTotal		(int);
void	SetupForEdit		(int);
void	ShowMissingItems	(void);
void 	SrchShipTerms 		(void);
void	ShowPay				(void);
void	SrchExsi			(char *);
void	SrchInum			(char *, int);
void	SrchPocr			(char *);
void	SrchPohr			(char *);
void	SrchPohr2			(char *);
void	SrchPolh			(char *);
void	SrchSuin			(char *, long);
void	Update				(void);
void	UpdateInis			(double);
void	UpdateInsf			(void);
void	UpdatePoln			(int, long, 	int);
void	UpdatePons			(int, long);
void	UpdatePosd			(long);
void	UpdatePosl			(long);
void	VertLine			(int, 	int);
void	_discScn			(int, char *, int, char *);
void	_discScnFreePrompt	(int);
void	_discScnSetPrompt	(int, char *);
void	tab_other			(int);

#ifdef GVISION
#include <disc_win.h>
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#else
void ViewDiscounts (void);
#endif	/* GVISION */

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv[])
{
	int		i,
			cnt;
	double	atof (const char *);

	TruePosition	=	TRUE;

	if (argc != 2)
	{
		print_at (0,0, mlPoMess713, argv [0]);
		print_at (1,0, mlPoMess714);
		print_at (2,0, mlPoMess715);
		print_at (3,0, mlPoMess716);
		print_at (4,0, mlPoMess717);
		print_at (5,0, mlPoMess718);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);


	currentUser = getenv ("LOGNAME");

	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	_win_func = TRUE;

	/*---------------------------------------
	| Check if costs are updated real time. |
	---------------------------------------*/
	updateSupplierRecord = chk_inis ();

	/*------------------------------------
	| Set up Input for Licence/Due Date. |
	------------------------------------*/
	sprintf (argumentFlag,"%-5.5s",argv [1]);

	FLD ("lic_no")   = (argumentFlag [0] == 'Y') ? YES : ND;
	FLD ("lic_val")  = (argumentFlag [1] == 'Y') ? YES : ND;
	FLD ("due_date") = (argumentFlag [2] == 'Y') ? YES : ND;
	FLD ("br_no")    = (argumentFlag [3] == 'Y') ? NE  : ND;
	FLD ("wh_no")    = (argumentFlag [3] == 'Y') ? NE  : ND;
	FLD ("case_no")  = (CASE_USED) ? YES : ND;

	if (envVar.threePlSystem)
	{
		FLD ("packageQty")		=	YES;
		FLD ("totalChargeWgt")	=	YES;
		FLD ("totalGrossWgt")	=	YES;
		FLD ("totalCBM")		=	YES;
		FLD ("cusOrdRef")		=	YES;
		FLD ("lic_no")   		= 	ND;
		FLD ("lic_val")  		= 	ND;
		FLD ("case_no")  		= 	ND;
		FLD ("fob_cst")  		= 	ND;
		FLD ("view_disc")  		= 	ND;
		FLD ("net_fob")			=	ND;
		FLD ("loc_fi")			=	ND;
		FLD ("duty_val")		=	ND;
		FLD ("other")			=	ND;
		FLD ("land_cost")		=	ND;
	}

	init_scr ();
	set_tty ();
	_set_masks ("po_cinput.s");
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store2, sizeof (struct store2Rec));
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	init_vars (PO_HEAD);

	if (argumentFlag [4] == 'Y')
		FLD ("po_type") = ND;

	tab_row = 9;

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	longSystemDate = TodaysDate ();

	/*---------------------------------
	| Open main database files & init |
	---------------------------------*/
	OpenDB ();

	ReadMisc ();

	InitML ();

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = screens [i];

	/*------------------------
	| Set up Screen Prompts. |
	------------------------*/
	sprintf (currencyCode,"%-3.3s",get_env ("CURR_CODE"));
	sprintf (localPrompt,"  %-3.3s Value  ",currencyCode);
	sprintf (freightPrompt, " F&I (%-3.3s) ",currencyCode);
	sprintf (cifPrompt," CIF (%-3.3s) ",currencyCode);
	strcpy (dutyPrompt,"Duty/ Unit");
	strcpy (licencePrompt,"Lic / Unit");

	strcpy (branchNumber, envVar.crCo ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	/*-----------------------------
	| Load category descriptions. |
	-----------------------------*/
	LoadCategoryDesc ();

	strcpy (local_rec.prev_po,	  		"000000000000000");
	strcpy (local_rec.previousCusOrdRef,"00000000000000000000");
	strcpy (local_rec.prev_crd_no,		"000000");

	origDueDate	=	FLD ("due_date");
	origUOM		=	FLD ("UOM");
	origQty		=	FLD ("qty");

	while (prog_exit == 0)
	{
		if (restart)
		{
			abc_unlock (pohr);
			abc_unlock (poln);
		}

		for (i = 0;i < MAXLINES;i++)
		{
			if (i < TABLINES)
			{
				strcpy (store2 [i].s_curr,"   ");
				store2 [i].s_exch = 0.00;
				strcpy (store2 [i].s_cost_edit, "N");
				store2 [i].s_inv_value = 0.00;
				store2 [i].s_item_value = 0.00;
				store2 [i].inv_found = FALSE;
				store2 [i].hhsu_hash = 0L;
			}
			store [i].exch_rate 	= 0.00;
			store [i].ship_no 		= 0L;
			store [i].quantity 		= 0.00;
			store [i].outer 		= 0.00;
			strcpy (store [i].duty_type," ");
			store [i].lic_rate 		= 0.00;
			store [i].net_fob 		= 0.00;
			store [i].grs_fgn 		= 0.00;
			store [i].val_duty 		= 0.00;
			store [i].weight 		= 0.00;
			store [i].volume 		= 0.00;
			store [i].lead_time 	= 0.00;
			store [i].sea_time 		= 0.00;
			store [i].air_time 		= 0.00;
			store [i].lnd_time 		= 0.00;
			store [i].cumulative 	= 0;
			store [i].discArray [0] = 0.00;
			store [i].discArray [1] = 0.00;
			store [i].discArray [2] = 0.00;
			store [i].discArray [3] = 0.00;
			store [i].sup_hhum_hash = 0L;
			store [i].hhumHash 	= 0L;
			strcpy (store [i].ser_item, " ");
			strcpy (store [i].ser_no,serialSpace);
			strcpy (store [i].std_uom, "    ");
			strcpy (store [i].uom_group, "                    ");
			strcpy (store [i].inp_uom, "    ");
			strcpy (store [i].sup_uom, "    ");

			for (cnt = 0; cnt < MAX_Pons; cnt++)
				sprintf (store [i].nsDesc [cnt], "%40.40s", " ");
		}
		eoi_ok 				= TRUE;
		search_ok 			= TRUE;
		entry_exit 			= FALSE;
		edit_exit 			= FALSE;
		prog_exit 			= FALSE;
		restart 			= FALSE;
		newPurchaseOrder 	= FALSE;
		init_ok 			= TRUE;
		skip_tab 			= FALSE;
		calculateDiscount 	= FALSE;
		init_vars (PO_HEAD);
		init_vars (PO_COST);
		init_vars (PO_LINES);
		lcount [PO_COST] 	= 0;
		lcount [PO_LINES] 	= 0;


		totalWeight			=	0.00;
		totalVolume			=	0.00;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (PO_HEAD);
		entry (PO_HEAD);

		if (prog_exit || restart)
			continue;

		if (newPurchaseOrder == TRUE)
		{
			/*--------------------
			| Enter item screen. |
			--------------------*/
			scn_set (PO_LINES);
			vars [scn_start].row = envVar.poMaxLines;
			init_vars (PO_LINES);
			lcount [PO_LINES] = 0;
			heading (PO_LINES);
			scn_display (PO_LINES);
			entry (PO_LINES);

			if (restart)
				continue;
		}
		calculateDiscount = TRUE;

		if (!envVar.poCostScreen)
			no_edit (2);

		/*------------------
		| Edit all screens.|
		------------------*/
		edit_all ();

		DisplayScreen (FALSE);

		calculateDiscount = FALSE;

		if (restart)
			continue;

		Update ();
	}

	/*========================
	| Program exit sequence. |
	========================*/
	clear ();
	snorm ();
	ClosePrint();
	CloseDB ();
	FinishProgram ();
    CleanUpPrompt ();

	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (pohr2, pohr);
	abc_alias (pohr3, pohr);
	abc_alias (poln2, poln);
	abc_alias (inum2, inum);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, !envVar.crFind ? "sumr_id_no"
														    : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2,inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (pohr3,pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poln2,poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_hhpo_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no3");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec (pons, pons_list, PONS_NO_FIELDS, "pons_id_no");
    open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
    open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
    open_rec (inuv,inuv_list, INUV_NO_FIELDS, "inuv_id_no");
}

void
ClosePrint (
 void)
{
	if (!running)
		return;

	running = 0;
	fprintf(pout,"0\n");

#ifdef GVISION
	Remote_fflush (pout);
#else
	fflush (pout);
#endif

	pclose(pout);
}
/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (incc);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posl);
	abc_fclose (podt);
	abc_fclose (inei);
	abc_fclose (inis);
	abc_fclose (posd);
	abc_fclose (pogd);
	abc_fclose (pocr);
	abc_fclose (suds);
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (pons);
	abc_fclose (posh);
	abc_fclose (poli);
	abc_fclose (inuv);
	SearchFindClose ();
	abc_dbclose (data);
}

void
ReadMisc (
 void)
{
	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, DBFIND);

	abc_fclose (comr);

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, DBFIND);
}

int
spec_valid (
 int field)
{
	int		line_no = 0;
	int		i;
	int		checkLines;
	int		wrkLine;
	int 	validPTerm = FALSE;
	double	wrk_fob	=	0.00;

	if (LCHECK ("br_no"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		if (NO_KEY (field))
		{
			strcpy (local_rec.br_no, comm_rec.est_no);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || F_NOKEY (field))
			strcpy (local_rec.br_no, comm_rec.est_no);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);

		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			/*------------------
			| Branch not found. |
			-------------------*/
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.br_name, esmr_rec.est_name);
		if (argumentFlag [3] == 'Y')
		{
			sprintf (err_str, ML (mlStdMess039),
							local_rec.br_no,local_rec.br_name);
			print_at (5, 0, "%R %s", err_str);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("wh_no"))
	{
		if (NO_KEY (field))
		{
			strcpy (local_rec.wh_no, comm_rec.cc_no);
			local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			return (EXIT_SUCCESS);
		}
		if (dflt_used || F_NOKEY (field))
			strcpy (local_rec.wh_no, comm_rec.cc_no);

		if (warehouseSelected)
		{
			abc_selfield (ccmr,"ccmr_id_no");
			warehouseSelected = FALSE;
		}
		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,local_rec.br_no);
		strcpy (ccmr_rec.cc_no, local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Warehouse not found. |
			----------------------*/
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.wh_name, ccmr_rec.name);
		if (argumentFlag [3] == 'Y')
		{
			sprintf (err_str, ML (mlStdMess099),
							local_rec.wh_no,local_rec.wh_name);
			print_at (5, 55, "%R %s", err_str);
		}
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supplierNumber"))
	{
		if (envVar.poInput && (dflt_used || F_NOKEY (field)))
		{
			sprintf (local_rec.crd_no, "%-6.6s", " ");
			DSP_FLD ("supplierNumber");
			return (EXIT_SUCCESS);
		}

		abc_selfield (sumr, envVar.crFind ? "sumr_id_no3" : "sumr_id_no");

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			/*---------------------
			| Supplier not found. |
			---------------------*/
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-------------------------------
		| If quick mode, disable sumr	|
		| if it's not a local one.	|
		-------------------------------*/
		if (argumentFlag [4] == 'Y' && strcmp (sumr_rec.curr_code, currencyCode))
		{
			/*------------------------------
			| Quick P/Orders MUST be local! |
			-------------------------------*/
			print_mess (ML (mlPoMess060));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-----------------------------------
		| Find special instructions if any. |
		-----------------------------------*/
		FindExsi ();

		/*--------------------------
		| Find currency code file. |
		--------------------------*/
		if (FindPocr (sumr_rec.curr_code))
			return (EXIT_FAILURE);

		/*--------------------
		| Find freight file. |
		--------------------*/
		if (FindPocf (sumr_rec.ctry_code))
			return (EXIT_FAILURE);

		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("pur_ord_no"))
	{
		if (SRCH_KEY)
		{
			if (strcmp (local_rec.crd_no, "      "))
				SrchPohr (temp_str);
			else
				SrchPohr2 (temp_str);
			return (EXIT_SUCCESS);
		}

		if (argumentFlag [4] == 'Y')
			strcpy (pohr_rec.stat_flag, "Q");

		if ((dflt_used || !strcmp (pohr_rec.pur_ord_no, fifteenSpaces)) &&
			strcmp (local_rec.crd_no, "      "))
		{
			if (!envVar.poInput)
			{
				/*----------------------
				| Manual P/o numbering |
				----------------------*/
				print_mess (ML (mlPoMess009));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
				strcpy (pohr_rec.pur_ord_no, "SYSTEM GEN.    ");

			DSP_FLD ("pur_ord_no");
			newPurchaseOrder = TRUE;
			LoadInvoiceScreen ();
			return (EXIT_SUCCESS);
		}
		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		/*------------------------------
		| if defaulted over crd_no then |
		| we need to use diff index     |
		------------------------------*/
		if (!strcmp (local_rec.crd_no, "      "))
		{
			abc_selfield (pohr,"pohr_id_no2");
			strcpy (sumr_rec.co_no, comm_rec.co_no);
		}
		else
			abc_selfield (pohr,"pohr_id_no");

		strcpy (pohr_rec.co_no,sumr_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type,"O");
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no,15));

		cc = find_rec (pohr,&pohr_rec,COMPARISON,"w");
		if (cc == -1)
		{
			restart = FALSE;
			heading (1);
			return (EXIT_FAILURE);
		}

		/*------------------------
		| Order already on file. |
		------------------------*/
		if (!cc)
		{
			if (pohr_rec.status [0] == 'T')
			{
				/*-----------------------
				| Order has in transit.  |
				------------------------*/
				print_mess (ML (mlPoMess737));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (pohr_rec.status [0] == 'D')
			{
				/*-----------------------
				| Order has been Closed. |
				------------------------*/
				print_mess (ML (mlPoMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] == 'Y')
			{
				/*-------------------------------------
				| DD Orders cannot be maintained here. |
				---------------------------------------*/
				print_mess (ML (mlPoMess061));
				sleep (sleepTime);
				abc_unlock (pohr);
				return (EXIT_FAILURE);
			}

			local_rec.exch_rate = pohr_rec.curr_rate;
			cc = LoadPoln (pohr_rec.hhpo_hash);
			if (cc)
			{
				restart = 1;
				return (EXIT_SUCCESS);
			}
			entry_exit = 1;
			newPurchaseOrder = FALSE;
			if (!CheckShipment (pohr_rec.hhpo_hash))
				updateShipment = TRUE;

			if (argumentFlag [4] == 'Y' && pohr_rec.stat_flag [0] != 'Q')
			{
				/*---------------------------------
				| Quick P/Orders can be maintained |
				-----------------------------------*/
				print_mess (ML (mlPoMess062));
				sleep (sleepTime);
				clear_mess ();
				restart = 1;
				return (EXIT_SUCCESS);
			}

			switch (pohr_rec.ship_method [0])
			{
				case 'L' :	strcpy (local_rec.ship_desc, "Land");
							break;
				case 'S' :	strcpy (local_rec.ship_desc, "Sea ");
							break;
				case 'A' :	strcpy (local_rec.ship_desc, "Air ");
							break;
			}
			FLD ("sup_type") = NA;
			DSP_FLD ("ship_desc");

			LoadInvoiceScreen ();
			if (envVar.poInput && !strcmp (local_rec.crd_no, "      "))
				return (LoadSupplier (field));
			return (EXIT_SUCCESS);

		}
		else
		{
			cc = find_rec (pohr3, &pohr_rec, COMPARISON, "r");
			if (!cc)
			{
				print_mess (ML("Purchase order is on file for another supplier"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (cc && !strcmp (local_rec.crd_no, "      "))
		{
			/*--------------
			| PO Not found. |
			---------------*/
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (argumentFlag [4] == 'Y')
			strcpy (pohr_rec.stat_flag, "Q");
		if (envVar.poInput)
			strcpy (pohr_rec.pur_ord_no, "SYSTEM GEN.    ");

		DSP_FLD ("pur_ord_no");

		pohr_rec.hhpo_hash = 0L;
		abc_unlock (pohr);
		newPurchaseOrder = TRUE;
		LoadInvoiceScreen ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate date raised.
	 */
	if (LCHECK ("date_raised"))
	{
		if (dflt_used)
			pohr_rec.date_raised = longSystemDate;

		if (pohr_rec.date_raised <= 0L)
		{
			print_mess (ML ("Date Raised must be input"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Check Supplier Type	|
	-----------------------*/
	if (LCHECK ("sup_type"))
	{
		if (F_NOKEY (field) || dflt_used)
		{
			strcpy (pohr_rec.sup_type, (FGN_CURR) ? "F" : "L");
			DSP_FLD ("sup_type");
			return (EXIT_SUCCESS);
		}
	}

	/*-----------------------
	| Check type with CRD	|
	-----------------------*/
	if (LCHECK ("po_type"))
	{
		if (pohr_rec.stat_flag [0] == 'Q')
		{
			if (strcmp (sumr_rec.curr_code, currencyCode))
			{
				/*------------------------------
				| Quick P/Orders MUST be local! |
				-------------------------------*/
				print_mess (ML (mlPoMess060));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Order Terms. |
	-----------------------*/
	if (LCHECK ("shipmentTerms"))
	{
		if (SRCH_KEY)
		{
			SrchShipTerms ();
			return (EXIT_SUCCESS);
		}
		sprintf (pohr_rec.term_order, clip(pohr_rec.term_order));

		for (i = 0; strlen(shipTerms [i]._shipTermsCode); i++)
		{
			if (!strncmp (pohr_rec.term_order, shipTerms [i]._shipTermsCode,
					     strlen (pohr_rec.term_order)))
			{
				validPTerm = TRUE;
				break;
			}
		}

		if (!validPTerm)
		{
			print_mess (ML("Invalid Order Terms"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		validPTerm = FALSE;
		DSP_FLD ("shipmentTerms");

		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Due Date Override. |
	-----------------------------*/
	if (LCHECK ("date_reqd"))
	{
		if (dflt_used || NO_KEY (field))
			pohr_rec.due_date = 0L;

		if (prog_status != ENTRY)
		{
			/*----------------------------------------------
			| Set due date at line level to poOverride date. |
			----------------------------------------------*/
			scn_set (PO_LINES);
			for (i = 0; i < lcount [PO_LINES]; i++)
			{
				getval (i);
				if (store [i].ship_no == 0L)
					local_rec.due_date = pohr_rec.due_date;
				putval (i);
			}

			scn_set (PO_HEAD);
		}

		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Default Exchange Rate. |
	------------------------*/
	if (LCHECK ("exch_rate"))
	{
		if (dflt_used || F_NOKEY (field))
		{
			local_rec.exch_rate = pocr_rec.ex1_factor;
			DSP_FLD ("exch_rate");
		}

		if (local_rec.exch_rate == 0.00)
		{
			/*------------------------------
			| Exchange Rate must be > 0.00 |
			------------------------------*/
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		store2 [FOB].s_exch = local_rec.exch_rate;

		if (prog_status != ENTRY)
		{
			scn_set (PO_LINES);
			for (line_no = 0;line_no < lcount [PO_LINES];line_no++)
			{
				getval (line_no);
				if (store [line_no].ship_no == 0L)
					store [line_no].exch_rate = local_rec.exch_rate;
				if (store [line_no].duty_type [0] != 'D')
				{
					local_rec.duty_val = DutyCalculate (line_no);
					store [line_no].val_duty = local_rec.duty_val;
				}
				local_rec.loc_fi = FreightCalculate (line_no);
				SR.val_fi = local_rec.loc_fi;

				CalculateCost (line_no);
				putval (line_no);
			}
			scn_set (PO_HEAD);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
			ilse
	| Validate supplier payment terms. |
	----------------------------------*/
	if (LCHECK ("stpay"))
	{
		if (SRCH_KEY)
		{
			ShowPay ();
			return (EXIT_SUCCESS);
		}
		for (i = 0; strlen (p_terms [i]._pcode); i++)
		{
			if (!strncmp (pohr_rec.sup_term_pay, p_terms [i]._pcode,
					     strlen (p_terms [i]._pcode)))
			{
				sprintf (pohr_rec.sup_term_pay, "%-30.30s",p_terms [i]._pterm);
				validPTerm = TRUE;
				break;
			}
		}

		if (!validPTerm)
		{
    		print_mess (ML("Invalid Supllier Payment Terms"));
    		sleep (sleepTime);
    		return (EXIT_FAILURE);
		}
		validPTerm = FALSE;

		DSP_FLD ("stpay");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Special instructions. |
	--------------------------------*/
	if (LCHECK ("sin1") || LCHECK ("sin2") || LCHECK ("sin3"))
	{
		i = field - label ("sin1");

		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			abc_fclose (exsi);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		if (i == 0)
			exsi_rec.inst_code = atoi (pohr_rec.stdin1);
		else if (i == 1)
			exsi_rec.inst_code = atoi (pohr_rec.stdin2);
		else if (i == 2)
			exsi_rec.inst_code = atoi (pohr_rec.stdin3);

		if (dflt_used && exsi_rec.inst_code == 0)
			exsi_rec.inst_code = comr_po_sic [i];

		/*------------------------------------------------------
		| Changed for ASL to not overwrite changed instructions |
		 ------------------------------------------------------*/
		if (newPurchaseOrder && prog_status == ENTRY)
		{
			if (!find_rec (exsi,&exsi_rec,COMPARISON,"r"))
			{
				if (i == 0)
					sprintf (pohr_rec.stdin1,"%-60.60s", exsi_rec.inst_text);
				else if (i == 1)
					sprintf (pohr_rec.stdin2,"%-60.60s", exsi_rec.inst_text);
				else if (i == 2)
					sprintf (pohr_rec.stdin3,"%-60.60s", exsi_rec.inst_text);
			}
		}

		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Delivery Instructions |
	--------------------------------*/
	if (LCHECK ("del1") || LCHECK ("del2") || LCHECK ("del3"))
	{
		i = field - label ("del1");

		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);

			abc_fclose (exsi);

			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		if (i == 0)
			exsi_rec.inst_code = atoi (pohr_rec.delin1);
		else if (i == 1)
			exsi_rec.inst_code = atoi (pohr_rec.delin2);
		else if (i == 2)
			exsi_rec.inst_code = atoi (pohr_rec.delin3);

		if (dflt_used && exsi_rec.inst_code == 0)
			exsi_rec.inst_code = sumr_sic [i];

		/*------------------------------------------------------
		| Changed for ASL to not overwrite changed instructions |
		 ------------------------------------------------------*/
		if (newPurchaseOrder && prog_status == ENTRY)
		{
			if (!find_rec (exsi,&exsi_rec,COMPARISON,"r"))
			{
				if (i == 0)
					sprintf (pohr_rec.delin1,"%-60.60s", exsi_rec.inst_text);
				else if (i == 1)
					sprintf (pohr_rec.delin2,"%-60.60s", exsi_rec.inst_text);
				else if (i == 2)
					sprintf (pohr_rec.delin3,"%-60.60s", exsi_rec.inst_text);
			}
		}
		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("item_no"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			if (envVar.poIndentOk)
			{
				cc =	check_indent
					 	(
							comm_rec.co_no,
							comm_rec.est_no,
							local_rec.hhcc_hash,
							local_rec.item_no
						);
				cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.co_no, comm_rec.co_no);
					strcpy (inmr_rec.item_no, local_rec.item_no);
					cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				}
				if (!cc)
				{
					PrintOtherStuff ();
					move (0, 4);
					line (132);
				}
			}
			if (!cc)
			{
				/*----------------------------------------
				| Purchase order # %s  / Supplier %s %s " |
				-----------------------------------------*/
				print_at (2,3, ML (mlPoMess064),
						pohr_rec.pur_ord_no,
						sumr_rec.crd_no,
						sumr_rec.crd_name);
			}
			if (cc)
			{
				/*----------------
				| Item not found. |
				-----------------*/
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

		}
		/*------------------------
		| Discontinued Product ? |
		------------------------*/
		if (inmr_rec.active_status [0] == 'D')
		{
			/*---------------------------
			| Item has been Discontinued |
			----------------------------*/
			print_mess (ML (mlPoMess122));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*----------------------------------------------------------
		| Check that there are less than 99 PO lines for this item |
		----------------------------------------------------------*/
		if (prog_status == ENTRY)
			wrkLine = line_cnt;
		else
			wrkLine = lcount [PO_LINES];
		for (i = 0, checkLines = 0;
			 i < wrkLine && checkLines < envVar.poMaxItemPo;
			 i++)
		{
			if (inmr_rec.hhbr_hash == store [i].hhbrHash)
				checkLines++;
		}
		if (checkLines > (envVar.poMaxItemPo - 1))
		{
			sprintf (err_str, "%s %d",
					 ML ("Line count for this item exceeds"),
					 envVar.poMaxItemPo);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhbrHash = inmr_rec.hhbr_hash;
		strcpy (SR._class, inmr_rec.inmr_class);
		if (check_class (inmr_rec.inmr_class))
		{
			InputPons (line_cnt);
			PrintOtherStuff ();
			move (0, 4);
			line (132);
		}

		if (CheckReorder (inmr_rec.inmr_class))
		{
			/*--------------
			| Invalid class |
			---------------*/
			sprintf (err_str,ML (mlPoMess057),inmr_rec.inmr_class,clip (envVar.poReorder));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (!check_class (SR._class))
			strcpy (SR.item_desc, inmr_rec.description);

		DSP_FLD ("item_no");
		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		SR.outer = (double) inmr_rec.outer_size;
		strcpy (SR.ser_item, inmr_rec.serial_item);

		virLineCnt = CalcVirtualLines ((prog_status == ENTRY));
		if (virLineCnt >= MAXLINES)
		{
			print_mess (ML ("Maximum number of purchase order lines exceeded"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*----------------------------------------
		| Find part number for warehouse record. |
		----------------------------------------*/
		incc_rec.hhcc_hash = local_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");

		if (cc)
		{
			i = prmptmsg (ML (mlStdMess033),"YyNn",1,8);
			if (i == 'n' || i == 'N')
			{
				move (0,8);
				cl_line ();
				skip_entry = -1 ;

				return (EXIT_SUCCESS);
			}
			else
			{
				AddIncc (local_rec.hhcc_hash, inmr_rec.hhbr_hash);
			}
			move (0,8);
			cl_line ();
		}
		SR.hhccHash = incc_rec.hhcc_hash;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Item has not UOM. |
			-------------------*/
			print_mess (ML (mlPoMess123));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (SR.std_uom, inum_rec.uom);
		if (prog_status == ENTRY)
		{
			strcpy (SR.inp_uom, inum_rec.uom);
			strcpy (local_rec.inp_uom,inum_rec.uom);
		}
		strcpy (local_rec.std_uom,inum_rec.uom);
		strcpy (SR.uom_group, inum_rec.uom_group);
		SR.StdCnvFct 		= inum_rec.cnv_fct;
		SR.pur_conv  		= 1.00;
		SR.sup_pur_conv 	= 1.00;
		local_rec.pur_conv 	= 1.00;

		if (FindInis (inmr_rec.hhbr_hash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			strcpy (local_rec.duty_code,inmr_rec.duty);
			strcpy (local_rec.lic_cat,inmr_rec.licence);

			SR.no_inis 			= TRUE;
			GetInuvWgtVol
			(
				SR.no_inis,
				SR.hhbrHash,
				inum_rec.hhum_hash,
				&SR.weight,
				&SR.volume
			);
			SR.weight 			= 0.00;
			SR.volume 			= 0.00;
			SR.lead_time 		= 0.00;
			SR.sea_time 		= 0.00;
			SR.air_time 		= 0.00;
			SR.lnd_time 		= 0.00;
			SR.min_order 		= 0;
			SR.ord_multiple 	= 0;
			SR.pallet_size 		= 0;
			local_rec.fob_cost 	= 0.00;
			SR.base_cost 		= 0.00;
			local_rec.pur_conv 	= 1.00;
			SR.pur_conv 		= 1.00;
			SR.sup_pur_conv		= 1.00;
			SR.sup_hhum_hash	= inum_rec.hhum_hash;
			strcpy (SR.inp_uom, SR.std_uom);
			strcpy (SR.sup_uom, SR.std_uom);
			strcpy (local_rec.inp_uom,local_rec.std_uom);
		}
		else
		{
			SR.no_inis 	= FALSE;

			GetInuvWgtVol
			(
				SR.no_inis,
				SR.hhbrHash,
				SR.hhumHash,
				&SR.weight,
				&SR.volume
			);

			SR.sea_time = inis_rec.sea_time;
			SR.air_time = inis_rec.air_time;
			SR.lnd_time = inis_rec.lnd_time;
			switch (pohr_rec.ship_method [0])
			{
			case 'S':
				SR.lead_time = inis_rec.sea_time;
				break;

			case 'A':
				SR.lead_time = inis_rec.air_time;
				break;

			case 'L':
				SR.lead_time = inis_rec.lnd_time;
				break;
			}

			strcpy (local_rec.duty_code,inis_rec.duty);
			strcpy (local_rec.lic_cat,inis_rec.licence);
			strcpy (inmr_rec.duty,inis_rec.duty);
			strcpy (inmr_rec.licence,inis_rec.licence);
			local_rec.fob_cost 	= DPP (inis_rec.fob_cost);
			SR.base_cost 		= DPP (inis_rec.fob_cost);
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (cc)
			{
				inum_rec.hhum_hash	=	inmr_rec.std_uom;
				cc = find_rec (inum, &inum_rec, COMPARISON, "r");
				if (cc)
				{
					/*------------------
					| Item has no UOM. |
					------------------*/
					print_mess (ML (mlPoMess123));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
			strcpy (SR.inp_uom, inum_rec.uom);
			strcpy (SR.sup_uom, inum_rec.uom);
			strcpy (local_rec.inp_uom,inum_rec.uom);
			local_rec.pur_conv	= SR.StdCnvFct / inum_rec.cnv_fct;
			SR.pur_conv 		= local_rec.pur_conv;
			SR.sup_pur_conv 	= local_rec.pur_conv;
			SR.sup_hhum_hash	= inis_rec.sup_uom;

			SR.min_order 		= inis_rec.min_order;
			SR.ord_multiple 	= inis_rec.ord_multiple;
			SR.pallet_size  	= inis_rec.pallet_size;
		}
		if (SR.weight == 0.00)
			SR.weight = inmr_rec.weight;


		if (SR.no_inis && INIS_REQ && !check_class (SR._class))
		{
			/*--------------------------------------
			| No inventory supplier record exists. |
			--------------------------------------*/
			sprintf (err_str, ML (mlStdMess155),inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.fob_cost == 0.00)
		{
			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no,comm_rec.est_no);
			cc = find_rec (inei,&inei_rec,COMPARISON,"r");
			if (cc)
			{
				/*----------------
				| Item not found. |
				-----------------*/
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		/*-------------------
		| Find duty record. |
		-------------------*/
		if (!strcmp (local_rec.duty_code,"  "))
		{
			SR.imp_duty = 0.00;
			SR.val_duty = 0.00;
			strcpy (SR.duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no,comm_rec.co_no);
			strcpy (podt_rec.code,local_rec.duty_code);
			cc = find_rec (podt,&podt_rec,COMPARISON,"r");
			if (cc)
			{
				/*----------------------
				| Duty Code Not found. |
				----------------------*/
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			SR.imp_duty = podt_rec.im_duty;
			strcpy (SR.duty_type, podt_rec.duty_type);
		}

		/*----------------------------
		| Find and Validate Licence. |
		----------------------------*/
		if (!strcmp (local_rec.lic_cat,"  "))
		{
			if (argumentFlag [0] == 'Y' || argumentFlag [0] == 'y')
				FLD ("lic_no") = NA;

			strcpy (SR.lic_cat, "  ");
			strcpy (SR.lic_no, "          ");
			SR.lic_hash = 0L;
			SR.lic_rate = 0.00;
			strcpy (local_rec.lic_no,"          ");
			local_rec.lic_val = 0.00;

			if (argumentFlag [0] == 'Y' || argumentFlag [0] == 'y')
				DSP_FLD ("lic_no");
		}
		else
		{
			if (argumentFlag [0] == 'Y' || argumentFlag [0] == 'y')
				FLD ("lic_no") = YES;

			strcpy (SR.lic_cat, inmr_rec.licence);
		}

		if (prog_status == ENTRY)
		{
			SR.exch_rate = local_rec.exch_rate;
			SR.ship_no = 0L;
		}

		SR.upd_inis = FALSE;
		SR.upd_ship = FALSE;
		if (local_rec.fob_cost != 0.00)
		{
			SR.cst_price = local_rec.fob_cost;
		}
		else
		{
			if (envVar.poCostCalc)
			{
				SR.cst_price 			= inei_rec.last_cost * SR.exch_rate;
				SR.cst_price			/= SR.outer;
				local_rec.contingency	= DOLLARS (comr_rec.contingency);
				local_rec.contingency	*= SR.cst_price;
				SR.cst_price			+= local_rec.contingency;

				SR.base_cost = SR.cst_price;
			}
			else
			{
				SR.cst_price = 0.00;
				SR.base_cost = 0.00;
			}
		}

		if (prog_status != ENTRY)
		{
			/*-------------------
			| Reenter Location. |
			--------------------*/
			if (!HIDE (label ("fob_cst")))
			{
				do
				{
					local_rec.grs_fgn = 0.00;
					get_entry (label ("fob_cst"));
					if (restart)
						break;
					cc = spec_valid (label ("fob_cst"));
				} while (cc && !HIDE (label ("fob_cst")));
			}
		}

		/*---------------
		| Set due date. |
		---------------*/
		if (pohr_rec.due_date != 0L)
			local_rec.due_date = pohr_rec.due_date;

		tab_other (line_cnt);

		strcpy (local_rec.inp_uom, (envVar.poUomDefault) ? SR.sup_uom : SR.std_uom);
		if (prog_status != ENTRY)
		{
			do
			{
				cc = spec_valid (label ("packageQty"));
			} while (cc && !HIDE (label ("packageQty")));
		}
		DSP_FLD ("UOM");
	}

	if (LCHECK ("packageQty"))
	{
		long	hhumHash;

		if (F_NOKEY (label ("UOM")))
			return (EXIT_SUCCESS);

		if (local_rec.packageQty == 0.0)
			return (EXIT_SUCCESS);

		hhumHash	=	GenerateUOM
						(
							SR.uom_group,
							SR.std_uom,
							local_rec.qty / local_rec.packageQty,
							SR.hhbrHash
						);

		if (hhumHash == -1L)
			return (EXIT_FAILURE);

		inum_rec.hhum_hash	=	hhumHash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		strcpy (local_rec.inp_uom, inum_rec.uom);
		do
		{
			cc = spec_valid (label ("UOM"));
		} while (cc && !HIDE (label ("UOM")));

		return (EXIT_SUCCESS);
 	}

	/*--------------------------
	| Validate Unit of Measure |
	--------------------------*/
	if (LCHECK ("UOM"))
	{
		if (F_NOKEY (label ("UOM")) || dflt_used)
			strcpy (local_rec.inp_uom,(envVar.poUomDefault) ? SR.sup_uom : SR.std_uom);

		if (SRCH_KEY)
		{
			SrchInum (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR.uom_group);
		strcpy (inum2_rec.uom, local_rec.inp_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Invalid Unit Measure |
			----------------------*/
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			/*---------------------
			| Invalid Unit Measure |
			----------------------*/
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.inp_uom, inum2_rec.uom);
		SR.hhumHash = inum2_rec.hhum_hash;
		strcpy (SR.inp_uom, inum2_rec.uom);
        if (inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR.pur_conv = SR.StdCnvFct / inum2_rec.cnv_fct;
		strcpy (SR.inp_uom, inum2_rec.uom);

		GetInuvWgtVol
		(
			SR.no_inis,
			SR.hhbrHash,
			SR.hhumHash,
			&SR.weight,
			&SR.volume
		);
		DSP_FLD ("UOM");
		if (prog_status != ENTRY)
		{
			do
			{
				cc = spec_valid (label ("qty"));
			} while (cc && !HIDE (label ("qty")));

			local_rec.grs_fgn	=	SR.cst_price;
			local_rec.grs_fgn	/=	DPP (SR.pur_conv);
			SR.grs_fgn			=	local_rec.grs_fgn;

			CalculateCost (line_cnt);
			local_rec.loc_fi	= FreightCalculate (line_cnt);
			SR.val_fi			= local_rec.loc_fi;
			CalculateCost (line_cnt);
			line_display ();
			DisplayScreen (TRUE);
		}
		RunningWVTotal (TRUE);
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Quantity input. |
	--------------------------*/
	if (LCHECK ("qty"))
	{
		if (dflt_used || F_NOKEY (field))
			local_rec.qty = ToLclUom (SR.min_order, SR.pur_conv);

		if (strcmp (SR.sup_uom, SR.inp_uom) && envVar.poConvSupUom)
		{
			strcpy (SR.inp_uom, SR.sup_uom);
			strcpy (local_rec.inp_uom, SR.sup_uom);
			do
			{
				cc = spec_valid (label ("UOM"));
			} while (cc && !HIDE (label ("UOM")));

			local_rec.qty = ToLclUom (local_rec.qty, SR.pur_conv);
		}

		if (newPurchaseOrder || !envVar.poOverride)
		{
			local_rec.qty = RndMltpl (ToStdUom (local_rec.qty,SR.pur_conv),
									  envVar.supOrdRound,
									  SR.ord_multiple,
									  SR.min_order);
			local_rec.qty = ToLclUom (local_rec.qty, SR.pur_conv);
		}

		if (ToStdUom (local_rec.qty, SR.pur_conv) > SR.pallet_size &&
		    SR.pallet_size > 0.00)
		{
			print_mess (ML (mlPoMess736));
			sleep (sleepTime);
			clear_mess ();
			local_rec.qty 	= ToLclUom (SR.pallet_size, SR.pur_conv);
		}

		SR.cst_price = 	GetSupPrice
						(
							sumr_rec.hhsu_hash,
						   	SR.hhbrHash,
						   	SR.base_cost,
						   	ToStdUom (local_rec.qty, SR.pur_conv)
						);
		local_rec.contingency	= DOLLARS (comr_rec.contingency);
		local_rec.contingency	*= SR.cst_price;
		SR.cst_price			+= local_rec.contingency;

		SR.cumulative = 	GetSupDisc
							(
								sumr_rec.hhsu_hash,
							   	inmr_rec.buygrp,
							   	ToStdUom (local_rec.qty, SR.pur_conv),
							   	SR.discArray
							);

		SR.quantity = ToStdUom (local_rec.qty, SR.pur_conv);
		if (prog_status != ENTRY)
		{
			local_rec.loc_fi = FreightCalculate (line_cnt);
			SR.val_fi = local_rec.loc_fi;
			CalculateCost (line_cnt);
			line_display ();
			DisplayScreen (TRUE);
		}
		virLineCnt = CalcVirtualLines ((prog_status == ENTRY));
		if (virLineCnt >= MAXLINES)
		{
			print_mess (ML ("Maximum number of purchase order lines exceeded"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		RunningWVTotal (FALSE);
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Licence Number. |
	--------------------------*/
	if (LCHECK ("lic_no"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY)
			strcpy (inmr_rec.licence,SR.lic_cat);

		if (!strcmp (inmr_rec.licence,"  "))
		{
			strcpy (SR.lic_cat, "  ");
			strcpy (SR.lic_no, "          ");
			SR.lic_hash = 0L;
			SR.lic_rate = 0.00;
			return (EXIT_SUCCESS);
		}
		open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_id_no");

		if (SRCH_KEY)
		{
			SrchPolh (temp_str);
			abc_fclose (polh);
			return (EXIT_SUCCESS);
		}

		strcpy (polh_rec.co_no,comm_rec.co_no);
		strcpy (polh_rec.est_no,comm_rec.est_no);
		strcpy (polh_rec.lic_cate,SR.lic_cat);
		strcpy (polh_rec.lic_no,local_rec.lic_no);
		cc = find_rec (polh,&polh_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------------
			| Licence Number Not found. |
			---------------------------*/
			print_mess (ML (mlStdMess154));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (polh);
			return (EXIT_FAILURE);
		}

		strcpy (SR.lic_cat, polh_rec.lic_cate);
		strcpy (SR.lic_no, polh_rec.lic_no);
		SR.lic_hash = polh_rec.hhlc_hash;
		SR.lic_rate = polh_rec.ap_lic_rate;
		strcpy (local_rec.lic_no,polh_rec.lic_no);
		local_rec.lic_val = polh_rec.ap_lic_rate;
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			line_display ();
		}
		abc_fclose (polh);
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate FOB (FGN). |
	--------------------*/
	if (LCHECK ("fob_cst"))
	{
		wrk_fob = SR.cst_price;
		wrk_fob /= DPP (SR.pur_conv);

		if (dflt_used || F_NOKEY (field))
			local_rec.grs_fgn = wrk_fob;

		SR.grs_fgn = n_dec (local_rec.grs_fgn,4);
		if (local_rec.grs_fgn == 0.00 && !F_NOKEY (field))
		{
			i = prmptmsg (ML (mlStdMess121),"YyNn",1,23);
			move (1,23);
			cl_line ();
			if (i != 'Y' && i != 'y')
			{
				FLD ("fob_cst")  =   YES;
				skip_entry = goto_field (field, label ("fob_cst"));
				return (EXIT_FAILURE);
			}
		}
		if (SR.no_inis == TRUE && !check_class (SR._class))
		{
			move (0,23);
			cl_line ();
			/*-----------------------
			| NOTE : Item not found. |
			------------------------*/
			rv_pr (ML (mlPoMess230), 0,32,1);
			sleep (sleepTime);
			SR.upd_inis = 0;
			if (prog_status == ENTRY)
				local_rec.due_date = pohr_rec.due_date;
		}
		else
		{
			move (0,23);
			cl_line ();
		}

		if (wrk_fob != local_rec.grs_fgn && !check_class (SR._class))
		{
			SR.upd_ship = TRUE;

			move (0,23);
			cl_line ();
			if (SR.no_inis == FALSE && !F_NOKEY (field))
			{
				/*-----------
				| Prompt	|
				-----------*/
				if (updateSupplierRecord == -1)
				{
					SR.upd_inis = prmpt_inis (0,23);
				}
				else
					SR.upd_inis = updateSupplierRecord;
			}
			if (prog_status != ENTRY && updateShipment)
			{
				move (0,23);
				cl_line ();
				/*----------------------------------------------------
				| Supplier FOB price for shipment (s) will be changed. |
				-----------------------------------------------------*/
				rv_pr (ML (mlPoMess058), 0,23,1);
				sleep (sleepTime);
			}
		}
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);

			if (SR.duty_type [0] != 'D')
			{
				local_rec.duty_val = DutyCalculate (line_cnt);
				SR.val_duty = local_rec.duty_val;
			}

			if (pocf_rec.load_type [0] != 'D')
			{
				local_rec.loc_fi = FreightCalculate (line_cnt);
				SR.val_fi = local_rec.loc_fi;
			}
			local_rec.net_fob =	CalcNet
								(
									local_rec.grs_fgn,
									SR.discArray,
									SR.cumulative
								);
			SR.net_fob = local_rec.net_fob;
			CalculateCost (line_cnt);
			line_display ();
			DisplayScreen (TRUE);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate View Discount. |
	-------------------------*/
	if (LCHECK ("view_disc"))
	{
		if (local_rec.view_disc [0] == 'Y')
		{
			int	tmpLcount;

			tmpLcount = lcount [PO_LINES];

			wrk_fob = SR.cst_price;
			wrk_fob /= DPP (SR.pur_conv);

#ifdef GVISION
			discRec.grossPrice		= local_rec.grs_fgn;
			discRec.discArray [0]	= SR.discArray [0];
			discRec.discArray [1]	= SR.discArray [1];
			discRec.discArray [2]	= SR.discArray [2];
			discRec.discArray [3]	= SR.discArray [3];

			ViewDiscounts (DBOX_LFT, DBOX_TOP, SR.cumulative);

			local_rec.grs_fgn	= discRec.grossPrice;
			SR.discArray [0]	= discRec.discArray [0];
			SR.discArray [1]	= discRec.discArray [1];
			SR.discArray [2]	= discRec.discArray [2];
			SR.discArray [3]	= discRec.discArray [3];
			local_rec.net_fob	= discRec.netPrice;
#else
			ViewDiscounts ();
#endif	/* GVISION */

			/*-----------------
			| Redraw screens. |
			-----------------*/
			putval (line_cnt);
			scn_write (PO_LINES);

			lcount [PO_LINES] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [PO_LINES];
			scn_display (PO_LINES);
			lcount [PO_LINES] = tmpLcount;
			if (wrk_fob != local_rec.grs_fgn)
			{
				SR.upd_ship = TRUE;

				move (0,23);
				cl_line ();
			}
		}
		else
		{
			local_rec.net_fob =	CalcNet
								(
									local_rec.grs_fgn,
									SR.discArray,
									SR.cumulative
								);
			SR.net_fob = local_rec.net_fob;
		}
		DSP_FLD ("net_fob");
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			local_rec.loc_fi = FreightCalculate (line_cnt);
			SR.val_fi = local_rec.loc_fi;
			CalculateCost (line_cnt);
			line_display ();
			DisplayScreen (TRUE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate FOB (LOC). |
	--------------------*/
	if (LCHECK ("loc_fi"))
	{
		if (dflt_used || F_NOKEY (field))
			local_rec.loc_fi = FreightCalculate (line_cnt);

		SR.val_fi = local_rec.loc_fi;
		CalculateCost (line_cnt);
		line_display ();

		if (prog_status != ENTRY)
			DisplayScreen (TRUE);

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Duty      |
	--------------------*/
	if (LCHECK ("duty_val"))
	{
		if (dflt_used || F_NOKEY (field))
			local_rec.duty_val = DutyCalculate (line_cnt);

		SR.val_duty = local_rec.duty_val;
		CalculateCost (line_cnt);
		line_display ();
		if (prog_status != ENTRY)
			DisplayScreen (TRUE);

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate due date at line item level. |
	---------------------------------------*/
	if (LCHECK ("due_date"))
	{
		if (NO_KEY (field) && pohr_rec.due_date != 0L)
			return (EXIT_SUCCESS);

		if (dflt_used || NO_KEY (field))
		{
			if (pohr_rec.due_date == 0L)
			{
				local_rec.due_date = pohr_rec.date_raised;
				local_rec.due_date += (long) SR.lead_time;
			}
			else
				local_rec.due_date = pohr_rec.due_date;

			DSP_FLD ("due_date");
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Supplier (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("supplier"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		sprintf (err_str,"%-6.6s",local_rec.supplier);

		if (prog_status == ENTRY)
			getval (line_cnt);


		if (dflt_used || F_NOKEY (field) || line_cnt == FOB)
		{
			if (!dflt_used)
			{
				/*-------------------------------------
				| Data cannot be input for goods (FOB) |
				--------------------------------------*/
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
				clear_mess ();
				strcpy (local_rec.supplier,"      ");
			}

			strcpy (POGD.s_curr,"   ");
			POGD.hhsu_hash = 0L;
			sumr2_rec.hhsu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr2_rec.co_no,comm_rec.co_no);
		strcpy (sumr2_rec.est_no,branchNumber);
		strcpy (sumr2_rec.crd_no,pad_num (local_rec.supplier));
		cc = find_rec (sumr,&sumr2_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------
			| Supplier not found. |
			---------------------*/
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (POGD.s_curr,sumr2_rec.curr_code);
		strcpy (local_rec.currency,POGD.s_curr);
		POGD.hhsu_hash = sumr2_rec.hhsu_hash;
		DSP_FLD ("currency");

		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code,local_rec.currency);
		cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------------
			| Currency Code not found. |
			--------------------------*/
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.lexch_rate = pocr_rec.ex1_factor;
		DSP_FLD ("lexch_rate");
		DSP_FLD ("fgn_val");

		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val = local_rec.fgn_val/local_rec.lexch_rate;
		DSP_FLD ("loc_val");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Allocation (costing Screen). |
	-----------------------------------------*/
	if (LCHECK ("allocation"))
	{
		if (line_cnt == FOB)
		{
			strcpy (local_rec.allocation,"D");
			DSP_FLD ("allocation");
		}
		strcpy (POGD.s_alloc,local_rec.allocation);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Invoice Number (Costing Screen.) |
	---------------------------------------------*/
	if (LCHECK ("inv_no"))
	{
		if (pohr_rec.stat_flag [0] == 'Q')
		{
			sprintf (local_rec.inv_no, "               ");
			DSP_FLD ("inv_no");
			return (EXIT_SUCCESS);
		}
		if (line_cnt == FOB)
		{
			/*-------------------------------------
			| Data cannot be input for goods (FOB) |
			--------------------------------------*/
			print_mess (ML (mlPoMess059));
			sleep (sleepTime);
			clear_mess ();
			strcpy (local_rec.inv_no,"               ");
			POGD.inv_found = FALSE;
			DSP_FLD ("inv_no");
			return (EXIT_SUCCESS);
		}
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		open_rec (suin,suin_list,SUIN_NO_FIELDS,"suin_id_no2");
		if (SRCH_KEY)
		{
			if (POGD.hhsu_hash == 0L)
			{
				/*-------------------
				| Search not allowed |
				--------------------*/
				print_mess (ML (mlPoMess063));
				abc_fclose (suin);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_SUCCESS);
			}
			SrchSuin (temp_str, POGD.hhsu_hash);
			abc_fclose (suin);
			return (EXIT_SUCCESS);
		}

		if (POGD.hhsu_hash != 0L && strcmp (local_rec.inv_no,"               "))
		{
			suin_rec.hhsu_hash = POGD.hhsu_hash;
			sprintf (suin_rec.inv_no,"%-15.15s",local_rec.inv_no);
			cc = find_rec (suin,&suin_rec,COMPARISON,"r");
			if (cc)
			{
				/*-------------------
				| Invoice not found. |
				--------------------*/
				print_mess (ML (mlStdMess115));
				sleep (sleepTime);
				abc_fclose (suin);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (suin_rec.approved [0] != 'Y')
			{
				/*--------------------
				| Unapproved invoice |
				--------------------*/
				sprintf (err_str, ML (mlPoMess121),local_rec.inv_no);
				print_mess (err_str);
				sleep (sleepTime);
				abc_fclose (suin);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			POGD.inv_found = TRUE;

			local_rec.loc_val = suin_rec.amt - suin_rec.gst;
			local_rec.fgn_val = DOLLARS (local_rec.loc_val);
			if (suin_rec.exch_rate != 0.00)
				local_rec.loc_val /= suin_rec.exch_rate;

			local_rec.loc_val = DOLLARS (local_rec.loc_val);
			POGD.s_exch = suin_rec.exch_rate;
			local_rec.lexch_rate = suin_rec.exch_rate;

			DSP_FLD ("loc_val");
			DSP_FLD ("fgn_val");
			DSP_FLD ("lexch_rate");
		}
		else
			POGD.inv_found = FALSE;

		if (line_cnt == FOB)
			POGD.s_inv_value = local_rec.fgn_val;
		else
			POGD.s_inv_value = local_rec.loc_val;

		SetupForEdit (line_cnt);
		abc_fclose (suin);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Currency (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("currency"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || F_NOKEY (field) || line_cnt == FOB)
			strcpy (local_rec.currency,POGD.s_curr);

		if (strcmp (local_rec.currency,"   "))
		{
			strcpy (pocr_rec.co_no,comm_rec.co_no);
			strcpy (pocr_rec.code,local_rec.currency);
			cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
			if (cc)
			{
				/*--------------------------
				| Currency Code not found. |
				--------------------------*/
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			local_rec.lexch_rate = pocr_rec.ex1_factor;
			POGD.s_exch = pocr_rec.ex1_factor;
		}
		else
		{
			if (line_cnt != FOB)
			{
				local_rec.lexch_rate = 1.00;
				POGD.s_exch = 1.00;
			}
		}

		DSP_FLD ("lexch_rate");

		if (POGD.inv_found)
		{
			local_rec.fgn_val = local_rec.loc_val;
			local_rec.fgn_val *= local_rec.lexch_rate;
			DSP_FLD ("fgn_val");

			skip_entry = 4;
		}
		else
		{
			if (prog_status != ENTRY)
			{
				local_rec.loc_val = local_rec.fgn_val;
				if (local_rec.lexch_rate != 0.00)
					local_rec.loc_val /= local_rec.lexch_rate;

				DSP_FLD ("loc_val");
			}
		}
		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.s_inv_value = local_rec.fgn_val;
		else
			POGD.s_inv_value = local_rec.loc_val;

		SetupForEdit (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate FGN_VAL Costing Screen. |
	----------------------------------*/
	if (LCHECK ("fgn_val"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used || F_NOKEY (field) || line_cnt == FOB)
		{
			RecalcCosting ();
			local_rec.fgn_val = POGD.s_inv_value;
			if (!dflt_used)
			{
				/*-------------------------------------
				| Data cannot be input for goods (FOB) |
				--------------------------------------*/
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
				clear_mess ();
			}
		}
		local_rec.loc_val = local_rec.fgn_val;
		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val /= local_rec.lexch_rate;

		DSP_FLD ("loc_val");

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.s_inv_value = local_rec.fgn_val;
		else
			POGD.s_inv_value = local_rec.loc_val;

		SetupForEdit (line_cnt);
	}

	/*---------------------------------------------
	| Validate Exchange Rate. (Costing Screen.) |
	---------------------------------------------*/
	if (LCHECK ("lexch_rate"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used || F_NOKEY (field) || line_cnt == FOB)
		{
			local_rec.lexch_rate = POGD.s_exch;
			if (!dflt_used)
			{
				/*-------------------------------------
				| Data cannot be input for goods (FOB) |
				--------------------------------------*/
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
				clear_mess ();
			}
		}

		local_rec.loc_val = local_rec.fgn_val;
		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val /= local_rec.lexch_rate;

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.s_inv_value = local_rec.fgn_val;
		else
			POGD.s_inv_value = local_rec.loc_val;

		DSP_FLD ("loc_val");

		if ((dflt_used || F_NOKEY (field)) && prog_status == ENTRY)
			skip_entry++;

		POGD.s_exch = local_rec.lexch_rate;
		SetupForEdit (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Calculate LOC_VAL (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("loc_val"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		RecalcCosting ();
		if (dflt_used || line_cnt == FOB || F_NOKEY (field))
		{
			local_rec.loc_val = local_rec.fgn_val;

			if (local_rec.lexch_rate != 0.00)
				local_rec.loc_val /= local_rec.lexch_rate;

			DSP_FLD ("loc_val");
			if (!dflt_used)
			{
				/*-------------------------------------
				| Data cannot be input for goods (FOB) |
				--------------------------------------*/
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
				clear_mess ();
			}
		}
		else
		{
			local_rec.fgn_val = local_rec.loc_val;
			local_rec.fgn_val *= local_rec.lexch_rate;

			DSP_FLD ("fgn_val");
		}

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.s_inv_value = local_rec.fgn_val;
		else
			POGD.s_inv_value = local_rec.loc_val;

		SetupForEdit (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Shipment Method. |
	---------------------------*/
	if (LCHECK ("ship_method"))
	{
		if (dflt_used || F_NOKEY (field))
			strcpy (pohr_rec.ship_method, sumr_rec.ship_method);

		switch (pohr_rec.ship_method [0])
		{
			case 'L' :	strcpy (local_rec.ship_desc, "Land");
						break;
			case 'S' :	strcpy (local_rec.ship_desc, "Sea ");
						break;
			case 'A' :	strcpy (local_rec.ship_desc, "Air ");
						break;
			default	 :  /*------------------------
						| Invalid Shipment Method |
						-------------------------*/
						print_mess (ML (mlStdMess119));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
		}
		DSP_FLD ("ship_desc");

		/*------------------------------------------------
		| Change due dates based on new shipment method. |
		------------------------------------------------*/
		if (prog_status != ENTRY && pohr_rec.due_date == 0L)
		{
			scn_set (PO_LINES);
			for (i = 0; i < lcount [PO_LINES]; i++)
			{
				getval (i);

				switch (pohr_rec.ship_method [0])
				{
				case 'S':
					store [i].lead_time = store [i].sea_time;
					break;

				case 'A':
					store [i].lead_time = store [i].air_time;
					break;

				case 'L':
					store [i].lead_time = store [i].lnd_time;
					break;

				}

				if (store [i].ship_no == 0L)
				{
					local_rec.due_date = pohr_rec.date_raised;
					local_rec.due_date += (long) store [i].lead_time;
				}

				putval (i);
			}

			scn_set (PO_HEAD);
		}

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("cusOrdRef"))
	{
		if (!dflt_used)
			strcpy (local_rec.previousCusOrdRef, local_rec.cusOrdRef);
		else
			strcpy (local_rec.cusOrdRef, local_rec.previousCusOrdRef);

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("totalCBM") && envVar.threePlSystem)
	{
		if (local_rec.totalCBM <= 0.00)
			return (EXIT_SUCCESS);

		inuv_rec.hhbr_hash  =   SR.hhbrHash;
		inuv_rec.hhum_hash  =   SR.hhumHash;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Invalid Unit Measure
			 */
			abc_unlock (inuv);
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inuv_rec.volume = 	local_rec.totalCBM /
							ToLclUom (SR.quantity, SR.pur_conv);

		cc = abc_update (inuv, &inuv_rec);
		if (cc)
			file_err (cc, inuv, "DBUPDATE");

		GetInuvWgtVol
		(
			SR.no_inis,
			SR.hhbrHash,
			SR.hhumHash,
			&SR.weight,
			&SR.volume
		);
		RunningWVTotal (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("totalGrossWgt") && envVar.threePlSystem)
	{
		if (local_rec.totalGrossWgt <= 0.00)
			return (EXIT_SUCCESS);

		inuv_rec.hhbr_hash  =   SR.hhbrHash;
		inuv_rec.hhum_hash  =   SR.hhumHash;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Invalid Unit Measure
			 */
			abc_unlock (inuv);
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		inuv_rec.weight = 	local_rec.totalGrossWgt /
							ToLclUom (SR.quantity, SR.pur_conv);
		cc = abc_update (inuv, &inuv_rec);
		if (cc)
			file_err (cc, inuv, "DBUPDATE");

		GetInuvWgtVol
		(
			SR.no_inis,
			SR.hhbrHash,
			SR.hhumHash,
			&SR.weight,
			&SR.volume
		);
		RunningWVTotal (TRUE);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===========================================
| Find Special Instructions for a Supplier. |
===========================================*/
void
FindExsi (
 void)
{
	int		i;
	char	wk_fld [5];

	/*-------------------------------------------------
	| Check if Customer Has Any Special Instructions. |
	-------------------------------------------------*/
	sprintf (pohr_rec.delin1, "%60.60s"," ");
	sprintf (pohr_rec.delin2, "%60.60s"," ");
	sprintf (pohr_rec.delin3, "%60.60s"," ");
	sprintf (pohr_rec.stdin1, "%60.60s"," ");
	sprintf (pohr_rec.stdin2, "%60.60s"," ");
	sprintf (pohr_rec.stdin3, "%60.60s"," ");

	open_rec (exsi ,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");
	for (i = 0;i < 3;i++)
	{
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = sumr_sic [i];
		if (!find_rec (exsi ,&exsi_rec,COMPARISON,"r"))
		{
			if (i == 0)
				strcpy (pohr_rec.delin1,exsi_rec.inst_text);
			else if (i == 1)
				strcpy (pohr_rec.delin2,exsi_rec.inst_text);
			else if (i == 2)
				strcpy (pohr_rec.delin3,exsi_rec.inst_text);

			sprintf (wk_fld, "del%1d", i + 1);
			if (FLD (wk_fld) == YES || FLD (wk_fld) == NO)
				FLD (wk_fld) = NI;
		}
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = comr_po_sic [i];
		if (!find_rec (exsi ,&exsi_rec,COMPARISON,"r"))
		{
			if (i == 0)
				strcpy (pohr_rec.stdin1,exsi_rec.inst_text);
			else if (i == 1)
				strcpy (pohr_rec.stdin2,exsi_rec.inst_text);
			else if (i == 2)
				strcpy (pohr_rec.stdin3,exsi_rec.inst_text);

			sprintf (wk_fld, "sin%1d", i + 1);
			if (FLD (wk_fld) == YES || FLD (wk_fld) == NO)
				FLD (wk_fld) = NI;
		}
	}
	DSP_FLD ("del1");
	DSP_FLD ("del2");
	DSP_FLD ("del3");
	DSP_FLD ("sin1");
	DSP_FLD ("sin2");
	DSP_FLD ("sin3");

	abc_fclose (exsi);
	return;
}
/*==============
 Find inis file
===============*/
int
FindInis (
 long	hhbrHash,
 long	hhsuHash)
{
	inis_rec.hhbr_hash = hhbrHash;
	inis_rec.hhsu_hash = hhsuHash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	return (cc);
}

/*=====================
| Find Currency file. |
=====================*/
int
FindPocr (
 char	*code)
{
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s", code);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	if (cc)
	{
		/*--------------------------
		| Currency code not found. |
		--------------------------*/
		print_mess (ML (mlStdMess040));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

/*============================
| Find Country freight file. |
============================*/
int
FindPocf (
 char	*code)
{
	open_rec (pocf,pocf_list,POCF_NO_FIELDS,"pocf_id_no");
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	sprintf (pocf_rec.code,"%-3.3s", code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
	{
		/*------------------------
		| Country Code not found. |
		-------------------------*/
		print_mess (ML (mlStdMess118));
		abc_fclose (pocf);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	abc_fclose (pocf);
	return (EXIT_SUCCESS);
}

/*====================
| Calculate Freight. |
====================*/
double
FreightCalculate (
 int		line_cnt)
{
	double	friValue 	= 0.00;
	double	frtConvert 	= 0.00;

	/*-----------------------
	| Calculate Freight	|
	-----------------------*/
	frtConvert = pocf_rec.freight_load;

	/*--------------------------
	| Freight is a Unit value. |
	--------------------------*/
	if (pocf_rec.load_type [0] == 'U')
		friValue = frtConvert;

	/*--------------------------
	| Freight is a Percentage. |
	--------------------------*/
	if (pocf_rec.load_type [0] == 'P')
	{
		friValue = SR.net_fob;
		friValue *= SR.outer;
		friValue *= DPP (SR.pur_conv);
		friValue *= frtConvert;
		friValue /= 100.00;
	}

	if (SR.exch_rate != 0.00)
		friValue /= SR.exch_rate;

	return (friValue);
}

/*==================================================
| Calculate Duty on total quantity and each basis. |
==================================================*/
double
DutyCalculate (
 int		line_cnt)
{
	double	dutyValue  = 0.00;

	/*-------------------
	| Calculate Duty   	|
	-------------------*/
	if (SR.duty_type [0] == 'D')
		dutyValue = SR.imp_duty;
	else
	{
		switch (store2 [DTY].s_alloc [0])
		{
		case 'V':
			dutyValue = SR.volume * SR.outer;
			dutyValue *= (SR.imp_duty / 100);
			if (SR.exch_rate != 0.00)
				dutyValue /= SR.exch_rate;
			break;

		case 'W':
			dutyValue = SR.weight * SR.outer;
			dutyValue *= (SR.imp_duty / 100);
			if (SR.exch_rate != 0.00)
				dutyValue /= SR.exch_rate;
			break;

		default:
			dutyValue	=	SR.net_fob;
			dutyValue	*=	SR.outer;
			dutyValue	*=	DPP (SR.pur_conv);
			dutyValue	*=	SR.imp_duty;
			dutyValue	/=	100.00;
			if (SR.exch_rate != 0.00)
				dutyValue /= SR.exch_rate;
		}
	}
	return (dutyValue);
}

/*============================
| Calculate total line cost. |
============================*/
void
CalculateCost (
	int		line_cnt)
{
	double	cifCost = 0.00;

	/*-------------------
	| Calculate FOB FGN	|
	-------------------*/
	if (prog_status != ENTRY)
	{
		local_rec.net_fob =	CalcNet
							(
								local_rec.grs_fgn,
								SR.discArray,
								SR.cumulative
							);
		store [line_cnt].net_fob	= local_rec.net_fob;
		/*
		inei_rec.last_cost	= store [line_cnt].cst_price;
		*/
	}

	/*-------------------
	| Calculate CIF FGN	|
	-------------------*/
	cifCost = 	local_rec.net_fob;
	cifCost *= 	store [line_cnt].outer;
	cifCost *= 	DPP (store [line_cnt].pur_conv);

	/*-------------------
	| Calculate CIF LOC	|
	-------------------*/
	if (store [line_cnt].exch_rate != 0.00)
		local_rec.cif_loc = cifCost / store [line_cnt].exch_rate;
	else
		local_rec.cif_loc = 0.00;

	local_rec.cif_loc += local_rec.loc_fi;

	/*-----------------------
	| Calculate Licence LOC	|
	-----------------------*/
	local_rec.lic_val  = (double) store [line_cnt].lic_rate;
	local_rec.lic_val *= local_rec.cif_loc;
	local_rec.lic_val = local_rec.lic_val;

	/*-------------------------------
	| Calculate Landed Cost local	|
	-------------------------------*/
	local_rec.land_cst = local_rec.cif_loc +
			     		 local_rec.duty_val +
			     		 local_rec.lic_val +
			     		 local_rec.other;

	local_rec.Dsp_land_cst 		= local_rec.land_cst;
	store [line_cnt].land_cost 	= local_rec.land_cst;
}


/*=======================================================================
| Routine to read all poln records whose hash matches the one on the    |
| pohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
int
LoadPoln (
 long	hhpoHash)
{
	int		i;

	print_at (2,1, ML (mlStdMess035));
	fflush (stdout);

	/*------------------------
	| Set screen for putval. |
	------------------------*/
	scn_set (PO_LINES);
	lcount [PO_LINES] = 0;

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (poln, "poln_id_no");

	poln_rec.hhpo_hash	= hhpoHash;
	poln_rec.line_no	= 0;

	cc = find_rec (poln,&poln_rec,GTEQ,"r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00 && !envVar.IkeaSystem)
		{
			cc = find_rec (poln,&poln_rec,NEXT,"r");
			continue;
		}
		/*-------------------
		| Too many orders . |
		-------------------*/
		virLineCnt = CalcVirtualLines (FALSE);
		if (virLineCnt >= MAXLINES)
		{
			print_mess (ML ("Maximum number of purchase order lines exceeded"));
			sleep (sleepTime);
			clear_mess ();
			abc_selfield (inmr, "inmr_id_no");
			abc_selfield (polh, "polh_id_no");
			abc_selfield (poln, "poln_hhpl_hash");
			abc_selfield (pogd, "pogd_id_no3");
			scn_set (PO_HEAD);
			return (EXIT_FAILURE);
		}
		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (strcmp (inmr_rec.supercession,"                "))
		{
			abc_selfield (inmr,"inmr_id_no");
			FindSupercession (comm_rec.co_no, inmr_rec.supercession, TRUE);
			abc_selfield (inmr,"inmr_hhbr_hash");
		}
		LSR.hhbrHash = inmr_rec.hhbr_hash;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		strcpy (LSR.std_uom, inum_rec.uom);
		strcpy (LSR.sup_uom, inum_rec.uom);
		strcpy (local_rec.std_uom,inum_rec.uom);
		strcpy (LSR.uom_group, inum_rec.uom_group);
		LSR.StdCnvFct 	= inum_rec.cnv_fct;
		LSR.hhumHash 	= inum_rec.hhum_hash;

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (LSR.std_uom, inum_rec.uom);
			strcpy (local_rec.std_uom,inum_rec.uom);
			LSR.pur_conv		= 1.00;
			LSR.sup_pur_conv	= 1.00;
		}
		else
		{
			if (inum_rec.cnv_fct == 0.00)
				inum_rec.cnv_fct = 1.00;

			strcpy (LSR.inp_uom, inum_rec.uom);
			strcpy (LSR.sup_uom, inum_rec.uom);
			strcpy (local_rec.inp_uom,inum_rec.uom);
			LSR.pur_conv		=	LSR.StdCnvFct / inum_rec.cnv_fct;
			LSR.sup_pur_conv	=	LSR.StdCnvFct / inum_rec.cnv_fct;
			local_rec.pur_conv 	=	LSR.StdCnvFct / inum_rec.cnv_fct;

			LSR.hhumHash = inum_rec.hhum_hash;
		}
		if (cc || CheckShipmentCosting (poln_rec.ship_no))
		{
			cc = find_rec (poln,&poln_rec,NEXT,"r");
			continue;
		}
		strcpy (local_rec.item_no,inmr_rec.item_no);

		GetWarehouse (poln_rec.hhcc_hash);

		add_hash
		(
			comm_rec.co_no,
			local_rec.br_no,
			"RC",
			0,
			poln_rec.hhbr_hash,
			poln_rec.hhcc_hash,
			0L,
			0.00
		);

		LSR.outer = (double) inmr_rec.outer_size;
		strcpy (LSR._class, inmr_rec.inmr_class);

		if (LSR.outer == 0.0)
			LSR.outer = 1.0;

		strcpy (LSR.item_desc, inmr_rec.description);
		strcpy (LSR.ser_item, inmr_rec.serial_item);

		if (FindInis (inmr_rec.hhbr_hash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			LSR.no_inis 	= TRUE;
			GetInuvWgtVol
			(
				LSR.no_inis,
				LSR.hhbrHash,
				LSR.hhumHash,
				&LSR.weight,
				&LSR.volume
			);
			LSR.lead_time 	= 0.00;
			LSR.sea_time 	= 0.00;
			LSR.air_time 	= 0.00;
			LSR.lnd_time 	= 0.00;
			LSR.weight 		= 0.00;
			LSR.volume 		= 0.00;

			strcpy (local_rec.duty_code,inmr_rec.duty);
			strcpy (local_rec.lic_cat,inmr_rec.licence);
			LSR.cst_price		=	local_rec.fob_cost = 0.00;
			LSR.min_order		=	0;
			LSR.ord_multiple	=	0;

			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no,comm_rec.est_no);
			cc = find_rec (inei,&inei_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, inei, DBFIND);

			if (envVar.poCostCalc)
			{
				LSR.cst_price = inei_rec.last_cost * pohr_rec.curr_rate;
				LSR.cst_price /= LSR.outer;
			}
			else
				LSR.cst_price = 0.00;
		}
		else
		{
			LSR.no_inis = FALSE;
			GetInuvWgtVol
			(
				LSR.no_inis,
				LSR.hhbrHash,
				LSR.hhumHash,
				&LSR.weight,
				&LSR.volume
			);

			LSR.sea_time = inis_rec.sea_time;
			LSR.air_time = inis_rec.air_time;
			LSR.lnd_time = inis_rec.lnd_time;
			switch (pohr_rec.ship_method [0])
			{
			case 'S':
				SR.lead_time = inis_rec.sea_time;
				break;

			case 'A':
				SR.lead_time = inis_rec.air_time;
				break;

			case 'L':
				SR.lead_time = inis_rec.lnd_time;
				break;
			}
			LSR.min_order = inis_rec.min_order;
			LSR.ord_multiple = inis_rec.ord_multiple;

			strcpy (local_rec.duty_code,inis_rec.duty);
			strcpy (local_rec.lic_cat,inis_rec.licence);

			strcpy (inmr_rec.duty,local_rec.duty_code);
			strcpy (inmr_rec.licence,local_rec.lic_cat);
			local_rec.fob_cost	= DPP (inis_rec.fob_cost);
			LSR.cst_price		= DPP (inis_rec.fob_cost);

		}

		if (!strcmp (local_rec.duty_code,"  "))
		{
			LSR.imp_duty = 0.00;
			LSR.val_duty = 0.00;
			strcpy (LSR.duty_type," ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no,comm_rec.co_no);
			strcpy (podt_rec.code,local_rec.duty_code);
			cc = find_rec (podt,&podt_rec,COMPARISON,"r");
			if (cc)
			{
				/*---------------------
				| Duty Code Not found. |
				----------------------*/
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			LSR.imp_duty = podt_rec.im_duty;
			strcpy (LSR.duty_type,podt_rec.duty_type);
		}
		/*----------------------
		| Find licence record. |
		----------------------*/
		if (!strcmp (local_rec.lic_cat,"  ") ||
		      poln_rec.hhlc_hash == 0L)
		{
			strcpy (LSR.lic_cat,"  ");
			strcpy (LSR.lic_no,"          ");
			LSR.lic_hash = 0L;
			LSR.lic_rate = 0.00;
			strcpy (local_rec.lic_no,"          ");
			local_rec.lic_val = 0.00;
		}
		else
		{
			open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_hhlc_hash");
			polh_rec.hhlc_hash = poln_rec.hhlc_hash;
			cc = find_rec (polh,&polh_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, polh, DBFIND);

			strcpy (LSR.lic_cat, polh_rec.lic_cate);
			strcpy (LSR.lic_no, polh_rec.lic_no);
			LSR.lic_hash = polh_rec.hhlc_hash;
			LSR.lic_rate = polh_rec.ap_lic_rate;
			strcpy (local_rec.lic_no,polh_rec.lic_no);
			local_rec.lic_val = polh_rec.ap_lic_rate;
			abc_fclose (polh);
		}
		/*---------------------
		| Setup local record. |
		---------------------*/
		local_rec.qty 			= ToLclUom (poln_rec.qty_ord - poln_rec.qty_rec,
								    		LSR.pur_conv);
		LSR.quantity 			= poln_rec.qty_ord - poln_rec.qty_rec,
		local_rec.case_no 		= poln_rec.case_no;
		local_rec.cif_loc 		= poln_rec.fob_nor_cst;
		local_rec.loc_fi 		= poln_rec.frt_ins_cst;
		local_rec.duty_val 		= poln_rec.duty;
		LSR.val_duty 			= poln_rec.duty;
		LSR.val_fi 				= poln_rec.frt_ins_cst;
		local_rec.lic_val 		= poln_rec.licence;
		local_rec.other 		= poln_rec.lcost_load;
		local_rec.land_cst 		= poln_rec.land_cst;
		local_rec.packageQty	= poln_rec.pack_qty;
		local_rec.totalChargeWgt= poln_rec.chg_wgt;
		local_rec.totalGrossWgt	= poln_rec.gross_wgt;
		local_rec.totalCBM		= poln_rec.cu_metre;
		strcpy (local_rec.cusOrdRef, poln_rec.cus_ord_ref);
		local_rec.Dsp_land_cst 	= poln_rec.land_cst;
		LSR.land_cost 			= local_rec.land_cst;
		local_rec.due_date 		= poln_rec.due_date;

		LSR.exch_rate 			= (poln_rec.exch_rate == 0.00) ?
									local_rec.exch_rate : poln_rec.exch_rate;
		LSR.ship_no 			= poln_rec.ship_no;
		if (LSR.outer != 0.00)
		{
			local_rec.grs_fgn = poln_rec.grs_fgn_cst;
			local_rec.grs_fgn /= DPP (LSR.pur_conv);
			local_rec.grs_fgn /= LSR.outer;

			local_rec.net_fob = poln_rec.fob_fgn_cst;
			local_rec.net_fob /= DPP (LSR.pur_conv);
			local_rec.net_fob /= LSR.outer;
		}

		LSR.grs_fgn 		= local_rec.grs_fgn;
		LSR.net_fob 		= local_rec.net_fob;
		LSR.discArray [0] 	= poln_rec.reg_pc;
		LSR.discArray [1] 	= poln_rec.disc_a;
		LSR.discArray [2] 	= poln_rec.disc_b;
		LSR.discArray [3] 	= poln_rec.disc_c;

		strcpy (local_rec.view_disc, (poln_rec.cumulative) ? "Y" : "N");
		for (i = 0; i < 4; i++)
			if (LSR.discArray [i])
				strcpy (local_rec.view_disc, "Y");

		LSR.cumulative   	= poln_rec.cumulative;
		LSR.upd_inis 		= FALSE;
		LSR.upd_ship 		= FALSE;
		strcpy (LSR.ser_no,poln_rec.serial_no);

		totalWeight	+=	LSR.weight * ToLclUom (LSR.quantity, LSR.pur_conv);
		totalVolume	+=	LSR.volume * ToLclUom (LSR.quantity, LSR.pur_conv);

		/*----------------------------------------------------
		| Put this bit in here to handle change of other etc |
		----------------------------------------------------*/
		CalculateCost (lcount [PO_LINES]);
		LoadPons (lcount [PO_LINES], poln_rec.hhpl_hash);

		putval (lcount [PO_LINES]++);

		cc = find_rec (poln,&poln_rec,NEXT,"r");

		if (lcount [PO_LINES] % 25 == 0)
			putchar ('R');

		fflush (stdout);

		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lcount [PO_LINES] > MAXLINES)
			break;
	}
	vars [scn_start].row = envVar.poMaxLines;

	totalWeight	= twodec (totalWeight);
	totalVolume	= twodec (totalVolume);

	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (pogd,"pogd_id_no3");

	scn_set (PO_HEAD);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [PO_LINES] == 0)
	{
		/*--------------------------
		| PO has no lines for edit. |
		---------------------------*/
		print_mess (ML (mlPoMess004));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*-------------------------
	| Normal exit - return 0. |
	-------------------------*/
	return (EXIT_SUCCESS);
}

void
Update (
 void)
{
	int		key				= 0,
			i 				= 0,
			po_line 		= 0,
			open_pipe 		= 0,
			cost_change 	= FALSE,
			cost_edit 		= FALSE,
			workLen			= 0,
			yearPeriod		= 0,
			PoCounter		= 0;
	float	sr_qty = 0.00;
	char	WorkYear [5];
	char	tempNumber [16];
	time_t	tloc	=	time (NULL);

	struct tm	*tmPtr;

	deleteLine = 0;
	MessageCnt	=	0;

	while (lcount [PO_LINES] == 0)
	{
		print_mess (ML (mlPoMess004));
		sleep (sleepTime);
		clear_mess ();
		edit_all ();
		if (restart)
			return;
	}

	for (i = 0; i < MAXLINES; i++)
	{
		if (i < TABLINES)
		{
			if (store2 [i].s_cost_edit [0] == 'Y')
				cost_edit = TRUE;
		}
	}

	if (envVar.poAppFlag)
	{
		if (PurchaseOrderValue () > envVar.poAppVal)
			heldOrder = TRUE;
	}
	else
		heldOrder = FALSE;

	clear ();
	scn_set (PO_HEAD);

	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.est_no,branchNumber);
	strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
	cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sumr, DBFIND);

	workLen	=	(LOCAL) ? strlen (prefixLocPo) : strlen (prefixFgnPo);

	if (newPurchaseOrder && envVar.poInput)
	{
		/*------------------------------------------------------
		| Specific purchase order number generation from Ikea. |
		| Number is last digit of year + number of days since  |
		| Jan 1 + a sequence number i.e no of po's today.	   |
		------------------------------------------------------*/
		if (envVar.IkeaPoNumbers)
		{
			tmPtr = localtime (&tloc);

			DateToDMY (longSystemDate, NULL, NULL, &yearPeriod);
			sprintf (WorkYear,"%04d", yearPeriod);

			open_rec (pohr2,pohr_list,POHR_NO_FIELDS,"pohr_id_no2");

			sprintf (pohr_rec.pur_ord_no, "%-1.1s%03d%03d",
						WorkYear + 3,
						tmPtr->tm_yday,
						++PoCounter);

			while (CheckPohr (pohr_rec.pur_ord_no) == 0)
			{
				sprintf (pohr_rec.pur_ord_no, "%-1.1s%03d%03d",
							WorkYear + 3,
							tmPtr->tm_yday,
							++PoCounter);
			}
			abc_fclose (pohr2);
		}
	    else if (!envVar.poNumGen)
	    {
			open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");

			strcpy (comr_rec.co_no,comm_rec.co_no);
			cc = find_rec (comr,&comr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, comr, DBFIND);

			open_rec (pohr2,pohr_list,POHR_NO_FIELDS,"pohr_id_no3");

			sprintf (tempNumber, "%ld", (LOCAL) ? ++comr_rec.nx_po_no
												: ++comr_rec.nx_po_no_fgn);
			sprintf
			(
				pohr_rec.pur_ord_no,
				"%-*.*s%s",
				workLen, workLen,
				(LOCAL) ? prefixLocPo : prefixFgnPo,
				zero_pad (tempNumber, 15 - workLen)
			);

			while (CheckPohr_c (pohr_rec.pur_ord_no) == 0)
			{
				sprintf (tempNumber, "%ld", (LOCAL) ? ++comr_rec.nx_po_no
													: ++comr_rec.nx_po_no_fgn);
				sprintf
				(
					pohr_rec.pur_ord_no,
					"%-*.*s%s",
					workLen, workLen,
					(LOCAL) ? prefixLocPo : prefixFgnPo,
					zero_pad (tempNumber, 15 - workLen)
				);
			}
			abc_fclose (pohr2);

			cc = abc_update (comr,&comr_rec);
			if (cc)
				file_err (cc, comr, "DBUPDATE");

			abc_selfield(pohr, "pohr_id_no2");
			abc_fclose (comr);
		}
	    else
	    {
			strcpy (esmr_rec.co_no,comm_rec.co_no);
			sprintf (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, esmr, DBFIND);

			sprintf (tempNumber,"%ld",(LOCAL) ? ++esmr_rec.nx_pur_ord_no
											  : ++esmr_rec.nx_pur_fgn);
			/*-------------------------------------
			| Check if Order No Already Allocated |
			| If it has been then skip.           |
			-------------------------------------*/
			open_rec (pohr2,pohr_list,POHR_NO_FIELDS,"pohr_id_no2");
			sprintf
			(
				pohr_rec.pur_ord_no,
				"%-*.*s%s",
				workLen, workLen,
				(LOCAL) ? prefixLocPo : prefixFgnPo,
				zero_pad (tempNumber, 15 - workLen)
			);
			while (CheckPohr (pohr_rec.pur_ord_no) == 0)
			{
				sprintf (tempNumber,"%ld",(LOCAL) ? ++esmr_rec.nx_pur_ord_no
											  	  : ++esmr_rec.nx_pur_fgn);
				sprintf
				(
					pohr_rec.pur_ord_no,
					"%-*.*s%s",
					workLen, workLen,
					(LOCAL) ? prefixLocPo : prefixFgnPo,
					zero_pad (tempNumber, 15 - workLen)
				);
			}
			abc_fclose (pohr2);

			cc = abc_update (esmr,&esmr_rec);
			if (cc)
				file_err (cc, esmr, "DBUPDATE");
	    }
	}
	/*--------------------------------
	| Add new purchase order header. |
	--------------------------------*/
	if (newPurchaseOrder)
	{
		sprintf (err_str, ML (mlPoMess065), pohr_rec.pur_ord_no);
		print_at (MessageCnt++,0, "%s", err_str);
		fflush (stdout);

		strcpy (pohr_rec.type,"O");
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.curr_code,sumr_rec.curr_code);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		pohr_rec.curr_rate = local_rec.exch_rate;
		strcpy (pohr_rec.status, (heldOrder) ? "U" : "O");
		sprintf (pohr_rec.op_id, "%-14.14s", currentUser);
		strcpy (pohr_rec.time_create, TimeHHMM());
		pohr_rec.date_create = TodaysDate ();

		cc = abc_add (pohr,&pohr_rec);
		if (cc)
			file_err (cc, pohr, "DBADD");

		cc = find_rec (pohr,&pohr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, pohr, DBFIND);

		print_at (MessageCnt++,0, "%s", ML (mlPoMess066));
		abc_unlock (pohr);
	}
	else
		print_at (MessageCnt++,0, "%s", ML (mlPoMess066));

	fflush (stdout);

	/*-----------------------------------
	| Process all purchase order lines. |
	-----------------------------------*/
	scn_set (PO_LINES);
	for (line_cnt = 0;line_cnt < lcount [PO_LINES];line_cnt++)
	{
		getval (line_cnt);

		strcpy (inmr_rec.co_no,comm_rec.co_no);
		strcpy (inmr_rec.item_no,local_rec.item_no);
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, inmr, DBFIND);

		if (inmr_rec.serial_item [0] == 'Y' && local_rec.qty > 1.00)
		{
			sr_qty = local_rec.qty;
			for (i = 0;i < sr_qty;i++)
			{
				local_rec.qty = 1.00;
				UpdatePoln (po_line++, (i) ? 0L : poln_rec.hhpl_hash, line_cnt);
			}
		}
		else
			UpdatePoln (po_line++, poln_rec.hhpl_hash, line_cnt);

		if (SR.upd_inis)
		{
			POGD.s_inv_value = local_rec.fgn_val;
			UpdateInis (local_rec.grs_fgn * DPP (SR.pur_conv));
			cost_change = TRUE;
		}
	}

	/*-------------------------------------------------------
	| Lines have been deleted off so update invoice header. |
	-------------------------------------------------------*/
	if (deleteLine || cost_change)
		UpdatePosd (pohr_rec.hhpo_hash);

	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (!newPurchaseOrder)
	{
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (lcount [PO_LINES] == 0)
		{
			print_at (MessageCnt++,0,"%s", ML (mlPoMess067));
			fflush (stdout);
			abc_unlock (pohr);

			if (!CheckShipment (pohr_rec.hhpo_hash))
			{
				print_at (MessageCnt++,0,"%s", ML (mlPoMess068));
				fflush (stdout);
				cc = abc_delete (posd);
			}

			poln_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
			poln_rec.line_no	=	0;
			cc = find_rec (poln, &poln_rec, GTEQ, "r");
			if (cc || poln_rec.hhpo_hash != pohr_rec.hhpo_hash)
			{
				cc = abc_delete (pohr);
			}
		}
		/*------------------------------------
		| Just update stat flag and rewrite. |
		------------------------------------*/
		else
		{
			/*-------------------------
			| Delete cancelled order. |
			-------------------------*/
			if (lcount [PO_LINES] == deleteLine)
			{
				print_at (MessageCnt++,0,"%s", ML (mlPoMess067));
				fflush (stdout);

				if (!CheckShipment (pohr_rec.hhpo_hash))
				{
					print_at (MessageCnt++,0, "%s", ML (mlPoMess068));
					fflush (stdout);
					cc = abc_delete (posd);
				}

			}
			else
			{
				print_at (MessageCnt++,0,"%s", ML (mlPoMess069));
				fflush (stdout);

				if (pohr_rec.status[0] == 'O')
					strcpy (pohr_rec.status, (heldOrder) ? "U" : "O");
				pohr_rec.curr_rate = local_rec.exch_rate;
				cc = abc_update (pohr,&pohr_rec);
				if (cc)
					file_err (cc, pohr, "DBUPDATE");
			}
		}
	}
	if (cost_edit)
	{
	    scn_set (PO_COST);

	    for (line_cnt = 0;line_cnt < MAX_POGD;line_cnt++)
	    {
			getval (line_cnt);

			strcpy (pogd_rec.co_no,comm_rec.co_no);
			pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
			pogd_rec.line_no = line_cnt;
			cc = find_rec (pogd,&pogd_rec,COMPARISON,"u");

			strcpy (pogd_rec.category,local_rec.category);
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				pogd_rec.hhsu_hash = 0L;
				sprintf (pogd_rec.invoice,"%-15.15s"," ");
				strcpy (pogd_rec.currency, currencyCode);
				pogd_rec.foreign = pogd_rec.nz_value;
				pogd_rec.exch_rate = 1.00;
				pogd_rec.allocation [0] = 'D';
			}
			else
			{
				pogd_rec.hhsu_hash = sumr2_rec.hhsu_hash;
				sprintf (pogd_rec.invoice,"%-15.15s",local_rec.inv_no);
				strcpy (pogd_rec.allocation,local_rec.allocation);
				sprintf (pogd_rec.currency,local_rec.currency);
				pogd_rec.foreign = local_rec.fgn_val;
				pogd_rec.exch_rate = local_rec.lexch_rate;
			}
			strcpy (pogd_rec.cost_edit,POGD.s_cost_edit);
			pogd_rec.nz_value = local_rec.loc_val;

			if (!cc)
			{
				putchar ('U');
				fflush (stdout);

				cc = abc_update (pogd,&pogd_rec);
				if (cc)
					file_err (cc, pogd, "DBUPDATE");
			}
			else
			{
				putchar ('A');
				fflush (stdout);

				cc = abc_add (pogd,&pogd_rec);
				if (cc)
					file_err (cc, pogd, "DBADD");
			}
			abc_unlock (pogd);
			fflush (stdout);
	    }
	    scn_set (PO_HEAD);
	}
	strcpy (local_rec.prev_po,pohr_rec.pur_ord_no);
	strcpy (local_rec.prev_crd_no,sumr_rec.crd_no);

	recalc_sobg ();

	if (!envVar.poPrint)
	{
		/*------------------------------------------------------
		| Purchase order no (%s) %s. Press any key to continue. |
		-------------------------------------------------------*/
		sprintf (err_str, (newPurchaseOrder) ? ML (mlPoMess070)
									  : ML (mlPoMess193),
										pohr_rec.pur_ord_no);
		PauseForKey (MessageCnt++,0, err_str, 0);
	}
	else
	{
		/*-----------------------------------------------------
		| Purchase order no (%s) %s. Key F5 to Print any other |
		| key to continue. |
		------------------------------------------------------*/
		sprintf (err_str, (newPurchaseOrder) ? ML (mlPoMess071)
									  : ML (mlPoMess194),
										pohr_rec.pur_ord_no);

		key = PauseForKey (MessageCnt++,0, err_str, 0);
		if ((key == 'P' || key == 'p') && !printerNumber)
			printerNumber = get_lpno (0);

		if (printerNumber && (key == 'P' || key == 'p'))
		{
			if(!running)
				open_pipe = 1;
			if(open_pipe)
			{
				if((pout = popen(poPrintProgram,"w")) == 0)
				{
					sprintf(err_str, "Error in %s during(POPEN)",poPrintProgram);
					sys_err(err_str, errno, PNAME);
				}
				running = 1;

				fprintf(pout,"%d\n",printerNumber);
				fprintf(pout,"S\n");
#ifdef GVISION
				Remote_fflush (pout);
#else
				fflush (pout);
#endif
			}
			if(running)
			{
				fprintf(pout,"%ld\n",pohr_rec.hhpo_hash);
#ifdef GVISION
				Remote_fflush (pout);
#else
				fflush (pout);
#endif
			}
		}
	}

	abc_unlock (pohr);
}

/*=====================================================
| Check Value of Order will existing value of orders. |
=====================================================*/
double
PurchaseOrderValue (
 void)
{
	double	porder_val = 0.00;
	double	o_total = 0.00;
	int		i;

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		/*-----------------------------------------------
		| Update soln gross tax and disc for each line. |
		-----------------------------------------------*/
		o_total = (double) store [i].quantity;
		o_total *= out_cost ((float)(store [i].land_cost), (float)(store [i].outer));

		porder_val += o_total;
	}
	return (porder_val);
}

/*========================================
| Update or Add lines to Purchase order. |
========================================*/
void
UpdatePoln (
 int		line_no,
 long		hhplHash,
 int		store_line)
{
	int 	add_item 	= FALSE;
	float	xx_qty 		= 0.00;

	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln2, &poln_rec, COMPARISON, "u");
	if (cc)
		add_item = TRUE;
	else
		add_item = FALSE;

	poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
	poln_rec.line_no 	= line_no;

	if (add_item)
	{
		poln_rec.qty_ord = ToStdUom (local_rec.qty, store[store_line].pur_conv);
		poln_rec.qty_rec = 0.00;
	}
	else
	{
		xx_qty = ToLclUom (poln_rec.qty_ord - poln_rec.qty_rec, store [store_line].pur_conv);
		xx_qty -= local_rec.qty;
		poln_rec.qty_ord -= ToStdUom (xx_qty ,store [store_line].pur_conv);
	}
	poln_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	poln_rec.hhcc_hash 	= local_rec.hhcc_hash;
	poln_rec.hhum_hash	= store [store_line].hhumHash;
	poln_rec.exch_rate 	= store [store_line].exch_rate;
	poln_rec.ship_no 	= store [store_line].ship_no;
	poln_rec.hhlc_hash 	= (long)(store [store_line].lic_hash);
	poln_rec.case_no 	= local_rec.case_no;
	if (pohr_rec.due_date > 0L && store [line_no].ship_no == 0L)
		poln_rec.due_date = pohr_rec.due_date;
	else
		poln_rec.due_date = local_rec.due_date;

	poln_rec.reg_pc 	= store [store_line].discArray [0];
	poln_rec.disc_a 	= store [store_line].discArray [1];
	poln_rec.disc_b 	= store [store_line].discArray [2];
	poln_rec.disc_c 	= store [store_line].discArray [3];
	poln_rec.cumulative	= store [store_line].cumulative;

	poln_rec.grs_fgn_cst = local_rec.grs_fgn;
	poln_rec.grs_fgn_cst *= store [store_line].outer;
	poln_rec.grs_fgn_cst *= DPP (store [store_line].pur_conv);

	poln_rec.fob_fgn_cst = local_rec.net_fob;
	poln_rec.fob_fgn_cst *= store [store_line].outer;
	poln_rec.fob_fgn_cst *= DPP (store [store_line].pur_conv);

	poln_rec.fob_nor_cst 	= local_rec.cif_loc;
	poln_rec.frt_ins_cst 	= local_rec.loc_fi;
	poln_rec.duty 			= local_rec.duty_val;
	poln_rec.licence 		= local_rec.lic_val;
	poln_rec.lcost_load	 	= local_rec.other;
	poln_rec.land_cst 		= local_rec.land_cst;
	if (envVar.threePlSystem)
	{
		poln_rec.pack_qty 		= local_rec.packageQty;
		poln_rec.chg_wgt 		= local_rec.totalChargeWgt;
		poln_rec.gross_wgt 		= local_rec.totalGrossWgt;
		poln_rec.cu_metre 		= local_rec.totalCBM;
	}
	else
	{
		poln_rec.pack_qty 		= 0.00;
		poln_rec.chg_wgt 		= 0.00;
		poln_rec.gross_wgt 		= 0.00;
		poln_rec.cu_metre 		= 0.00;
	}
	strcpy (poln_rec.cus_ord_ref, local_rec.cusOrdRef);
	strcpy (poln_rec.item_desc,inmr_rec.description);
	strcpy (poln_rec.cat_code,inmr_rec.category);
	strcpy (poln_rec.stat_flag,"B");

	if (add_item)
	{
		if (line_no % 25 == 0)
			putchar ('A');

		fflush (stdout);
		sprintf (poln_rec.serial_no,"%25.25s"," ");
		strcpy (poln_rec.pur_status, (heldOrder) ? "U" : "O");
		poln_rec.qty_rec = 0.00;
		if (poln_rec.qty_ord != 0.00 || envVar.IkeaSystem)
		{
			cc = abc_add (poln,&poln_rec);
			if (cc)
				file_err (cc, poln, "DBUPDATE");

			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			poln_rec.line_no = line_no;
			cc = find_rec (poln,&poln_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, poln, DBFIND);
		}
		UpdatePons (line_no, poln_rec.hhpl_hash);
	}
	else
	{
		if (poln_rec.qty_ord == 0.00 && !envVar.IkeaSystem)
		{
			putchar ('D');
			fflush (stdout);

			/*-----------------------------------------
			| Find and Delete existing shipment line. |
			-----------------------------------------*/
			poli_rec.hhpl_hash	=	poln_rec.hhpl_hash;
			cc = find_rec (poli, &poli_rec, EQUAL,"r");
			if (!cc)
				abc_delete (poli);

			/*-----------------------------------------
			| Find and Delete existing shipment line. |
			-----------------------------------------*/
			posl_rec.hhpl_hash	=	poln_rec.hhpl_hash;
			cc = find_rec (posl,&posl_rec,EQUAL,"r");
			if (!cc)
				abc_delete (posl);

			/*-----------------------------
			| Delete existing order line. |
			-----------------------------*/
			if (inmr_rec.serial_item [0] == 'Y' &&
			     strcmp (poln_rec.serial_no, serialSpace))
			{
				DeleteInsf ();
			}

			DeletePons (poln_rec.hhpl_hash);

			cc = abc_delete (poln2);
			deleteLine++;
		}
		else
		{
			if (line_no % 25 == 0)
				putchar ('U');

			fflush (stdout);
			if (poln_rec.pur_status [0] == 'O')
				strcpy (poln_rec.pur_status, (heldOrder) ? "U" : "O");

			/*------------------------
			| Update existing order. |
			------------------------*/
			cc = abc_update (poln2,&poln_rec);
			if (cc)
				file_err (cc, poln, "DBUPDATE");

			if (updateShipment && SR.upd_ship)
				UpdatePosl (poln_rec.hhpl_hash);

			UpdatePons (line_no, poln_rec.hhpl_hash);
		}
		abc_unlock (poln2);
	}
	if (inmr_rec.serial_item [0] == 'Y' &&
	     strcmp (poln_rec.serial_no,"                         "))
		UpdateInsf ();

	add_hash
	 (
		comm_rec.co_no,
		local_rec.br_no,
		"RC",
		0,
		poln_rec.hhbr_hash,
		poln_rec.hhcc_hash,
		0L,
		0.00
	);
}

/*=================================================
| Update purchase order Shipment line allocation. |
=================================================*/
void
UpdatePosl (
	long	hhplHash)
{
	posl_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (posl,&posl_rec,GTEQ,"u");
	while (!cc && posl_rec.hhpl_hash == hhplHash)
	{
		posl_rec.sup_price = poln_rec.fob_fgn_cst;
		cc = abc_update (posl,&posl_rec);
		if (cc)
			file_err (cc, posl, "DBUPDATE");

		cc = find_rec (posl,&posl_rec,NEXT,"u");
	}
	abc_unlock (posl);
	return;
}

/*======================================================================
| Update Invoice Header as some lines from poln may have been deleted. |
======================================================================*/
void
UpdatePosd (
	long	hhpoHash)
{
	abc_selfield (posl,"posl_id_no_2");

	posd_rec.hhpo_hash = hhpoHash;
	cc = find_rec (posd, &posd_rec, GTEQ, "u");
	while (!cc && posd_rec.hhpo_hash == hhpoHash)
	{
		print_at (MessageCnt++,0,"%s", ML (mlPoMess072));

		CalculatePosd (posd_rec.hhsh_hash, posd_rec.hhpo_hash);

		cc = abc_update (posd,&posd_rec);
		if (cc)
			file_err (cc, posd, "DBUPDATE");

		cc = find_rec (posd,&posd_rec, NEXT,"u");
	}
	abc_unlock (posd);

	abc_selfield (posl,"posl_hhpl_hash");
}

/*========================================================================
| Calculate Invoice value for posd file from all lines on shipment file. |
========================================================================*/
void
CalculatePosd (
 long hhsh_hash,
 long hhpo_hash)
{
	double	extend;

	posd_rec.total = 0.00;

	strcpy (posl_rec.co_no,comm_rec.co_no);
	posl_rec.hhsh_hash = hhsh_hash;
	posl_rec.hhpo_hash = hhpo_hash;
	cc = find_rec (posl,&posl_rec,GTEQ,"r");
	while (!cc && !strcmp (posl_rec.co_no,comm_rec.co_no) &&
			      posl_rec.hhsh_hash == hhsh_hash &&
			      posl_rec.hhpo_hash == hhpo_hash)
	{
		extend = (double) posl_rec.ship_qty;
		extend *= posl_rec.sup_price;

		posd_rec.total += extend;

		cc = find_rec (posl,&posl_rec,NEXT,"r");
	}
}

/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchShipTerms (
 void)
{
	int		i = 0;

	_work_open (3,0,40);
	save_rec ("#ST ","#Shipment Terms ");

	for (i = 0;strlen (shipTerms [i]._shipTermsCode);i++)
	{
		cc = save_rec (shipTerms [i]._shipTermsCode,shipTerms [i]._shipTermsShort);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}
/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char *key_val)
{
	char	wk_code [4];

	_work_open (3,0,60);
	save_rec ("#SI.","#Special Instruction description.");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);

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
		file_err (cc, "exsi", DBFIND);
}
/*===========================
| Search for Payment Terms. |
===========================*/
void
ShowPay (
 void)
{
	int		i = 0;

	work_open ();
	save_rec ("#Cde","#Supplier Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*==========================
| Search for order number. |
==========================*/
void
SrchPohr (
 char *key_val)
{
	int		NonZeroFound	=	FALSE;

	work_open ();
	save_rec ("#P/O Number.   ","#Date Raised");
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type,"O");
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");
	while (!cc &&
		!strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)) &&
		!strcmp (pohr_rec.co_no,comm_rec.co_no) &&
		!strcmp (pohr_rec.br_no,comm_rec.est_no))
	{
		if (pohr_rec.status [0] == 'T' || pohr_rec.drop_ship [0] == 'Y')
        {
             cc = find_rec (pohr, &pohr_rec, NEXT, "r");
             continue;
        }

		NonZeroFound	=	FALSE;
		poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
		poln_rec.line_no 	= 0;
		cc = find_rec (poln, &poln_rec, GTEQ,"r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
			if ((poln_rec.qty_ord - poln_rec.qty_rec) > 0.00 || envVar.IkeaSystem)
			{
				NonZeroFound	=	TRUE;
				break;
			}
			cc = find_rec (poln, &poln_rec, NEXT,"r");
		}

		if ((pohr_rec.hhsu_hash == sumr_rec.hhsu_hash &&
		      pohr_rec.type [0] == 'O' && pohr_rec.status [0] != 'D') &&
			  (NonZeroFound || envVar.IkeaSystem))
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type,"O");
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, pohr, DBFIND);
}

/*==========================
| Search2 for order number. |
==========================*/
void
SrchPohr2 (
 char *key_val)
{
	int		NonZeroFound	=	FALSE;

	abc_selfield (pohr,"pohr_id_no2");
	abc_selfield (sumr,"sumr_hhsu_hash");

	work_open ();
	sprintf (err_str,
		"#%-7.7s%-10.10s%-40.40s",
		"Crd No",
		"Acronym",
		"Supplier Name"
		);

	save_rec ("#P/O Number.    ", err_str);
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");
	while (!cc &&
		!strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)) &&
		!strcmp (pohr_rec.co_no,comm_rec.co_no) &&
		!strcmp (pohr_rec.br_no,comm_rec.est_no))
	{
		if (pohr_rec.status [0] == 'T' || pohr_rec.drop_ship [0] == 'Y')
        {
             cc = find_rec (pohr, &pohr_rec, NEXT, "r");
             continue;
		}

		NonZeroFound	=	FALSE;
		poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
		poln_rec.line_no 	= 0;
		cc = find_rec (poln, &poln_rec, GTEQ,"r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
			if ((poln_rec.qty_ord - poln_rec.qty_rec) > 0.00 || envVar.IkeaSystem)
			{
				NonZeroFound	=	TRUE;
				break;
			}
			cc = find_rec (poln, &poln_rec, NEXT,"r");
		}
		if ((pohr_rec.type [0] == 'O' && pohr_rec.status [0] != 'D') &&
				(NonZeroFound || envVar.IkeaSystem))
		{
			sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (pohr,&pohr_rec,NEXT,"r");
				continue;
			}

			sprintf (err_str, "%-7.7s%-10.10s%-40.40s",
				 sumr_rec.crd_no,
				 sumr_rec.acronym,
				 sumr_rec.crd_name);

			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, pohr, DBFIND);

	abc_selfield (pohr,"pohr_id_no");
	abc_selfield (sumr, envVar.crFind ? "sumr_id_no3" : "sumr_id_no");

#ifndef GVISION
	clear ();
#endif	/* GVISION */
}

/*=========================================
| Search routine for Licence Header file. |
=========================================*/
void
SrchPolh (
 char *key_val)
{
	work_open ();
	save_rec ("#Lic. No.","#Licence Type");
	strcpy (polh_rec.co_no,comm_rec.co_no);
	strcpy (polh_rec.est_no,comm_rec.est_no);
	strcpy (polh_rec.lic_cate,inmr_rec.licence);
	sprintf (polh_rec.lic_no,"%-10.10s",key_val);
	cc = find_rec (polh,&polh_rec,GTEQ,"r");
	while (!cc && !strcmp (polh_rec.co_no,comm_rec.co_no) &&
		      !strcmp (polh_rec.est_no,comm_rec.est_no) &&
		      !strcmp (polh_rec.lic_cate,inmr_rec.licence) &&
		      !strncmp (polh_rec.lic_no,key_val,strlen (key_val)))
	{
		cc = save_rec (polh_rec.lic_no,polh_rec.type);
		if (cc)
			break;
		cc = find_rec (polh,&polh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (polh_rec.co_no,comm_rec.co_no);
	strcpy (polh_rec.est_no,comm_rec.est_no);
	strcpy (polh_rec.lic_cate,inmr_rec.licence);
	sprintf (polh_rec.lic_no,"%-10.10s",temp_str);
	cc = find_rec (polh,&polh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, polh, DBFIND);
}

void
PrintOtherStuff (
 void)
{
	/*--------------------------------------
	| Purchase order # %s  / Supplier %s %s |
	---------------------------------------*/
	print_at (2,3, ML (mlPoMess064),
		pohr_rec.pur_ord_no,
		sumr_rec.crd_no,
		sumr_rec.crd_name);

	/*-------------------------------------------------------
	| Date %s Currency %s %s  P/O Header Exchange Rate %8.4f |
	--------------------------------------------------------*/
	print_at (3,3, ML (mlPoMess073),
		DateToString (pohr_rec.date_raised),
		pocr_rec.code,
		pocr_rec.description,
		local_rec.exch_rate);

	fflush (stdout);
}

int
CheckShipment (
	long	hhpoHash)
{
	posd_rec.hhpo_hash	=	hhpoHash;
	return (find_rec (posd,&posd_rec,COMPARISON,"r"));
}

/*===================================
| Update Inventory Supplier Record. |
===================================*/
void
UpdateInis (
 double upd_value)
{
	/*----------------------------------
	| Find inventory supplier records. |
	----------------------------------*/
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (inis_rec.co_no,comm_rec.co_no);
	strcpy (inis_rec.br_no,comm_rec.est_no);
	strcpy (inis_rec.wh_no,comm_rec.cc_no);
	cc = find_rec (inis,&inis_rec,COMPARISON,"u");
	if (cc)
	{
		abc_unlock (inis);
		strcpy (inis_rec.wh_no,"  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"u");
	}
	if (cc)
	{
		abc_unlock (inis);
		strcpy (inis_rec.br_no,"  ");
		strcpy (inis_rec.wh_no,"  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"u");
	}
	if (!cc)
	{
		inis_rec.fob_cost = upd_value;
		inis_rec.lcost_date = StringToDate (local_rec.systemDate);
		cc = abc_update (inis,&inis_rec);
		if (cc)
			file_err (cc, inis, "DBUPDATE");
	}
	else
		abc_unlock (inis);
}

int
CheckPohr (
	char	*purchaseOrderNumber)
{
	strcpy (pohr2_rec.co_no,comm_rec.co_no);
	strcpy (pohr2_rec.br_no,comm_rec.est_no);
	sprintf (pohr2_rec.pur_ord_no,"%-15.15s", purchaseOrderNumber);
	return (find_rec (pohr2,&pohr2_rec,COMPARISON,"r"));
}

int
CheckPohr_c (
	char	*purchaseOrderNumber)
{
	abc_selfield(pohr, "pohr_id_no3");
	strcpy (pohr2_rec.co_no,comm_rec.co_no);
	sprintf (pohr2_rec.pur_ord_no,"%-15.15s", purchaseOrderNumber);
	return (find_rec (pohr2,&pohr2_rec,COMPARISON,"r"));
}

/*======================
| Load Costing screen. |
======================*/
void
LoadInvoiceScreen (
 void)
{
	int		i;

	scn_set (PO_COST);
	init_vars (PO_COST);

	abc_selfield (sumr,"sumr_hhsu_hash");

	for (i = 0; i < MAX_POGD; i++)
	{
		strcpy (pogd_rec.co_no,comm_rec.co_no);
		pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
		pogd_rec.line_no = i;
		cc = find_rec (pogd,&pogd_rec,COMPARISON,"r");
		if (!cc && pohr_rec.hhpo_hash > 0L && !newPurchaseOrder)
		{
			store2 [i].hhsu_hash = pogd_rec.hhsu_hash;
			sprintf (local_rec.category,"%-20.20s",cat_desc [i]);
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				sprintf (pogd_rec.invoice, "%-15.15s", " ");
				pogd_rec.hhsu_hash = 0L;
				strcpy (pogd_rec.currency, currencyCode);
				pogd_rec.foreign = pogd_rec.nz_value;
				pogd_rec.exch_rate = 1.00;
				pogd_rec.allocation [0] = 'D';
			}
			sprintf (local_rec.inv_no,"%-15.15s",pogd_rec.invoice);
			sumr2_rec.hhsu_hash = pogd_rec.hhsu_hash;
			sprintf (local_rec.allocation,"%-1.1s",pogd_rec.allocation);
			store2 [i].inv_found = (pogd_rec.hhsu_hash != 0L && strcmp (local_rec.inv_no,"               "));

			if (pogd_rec.hhsu_hash != 0L)
			{
				sumr2_rec.hhsu_hash	=	pogd_rec.hhsu_hash;
				cc = find_rec (sumr,&sumr2_rec,COMPARISON,"r");
				if (cc)
					file_err (cc, sumr, DBFIND);

				sprintf (local_rec.supplier,"%-6.6s", sumr2_rec.crd_no);
			}
			else
				sprintf (local_rec.supplier,"%-6.6s"," ");

			strcpy (store2 [i].s_curr,pogd_rec.currency);
			sprintf (local_rec.currency,"%-3.3s",pogd_rec.currency);
			local_rec.fgn_val = pogd_rec.foreign;
			local_rec.lexch_rate = pogd_rec.exch_rate;
			store2 [i].s_exch = pogd_rec.exch_rate;
			local_rec.loc_val = pogd_rec.nz_value;

			/*-----------------------
			| Store fgn if goods	|
			-----------------------*/
			if (i == FOB)
				store2 [i].s_inv_value = local_rec.fgn_val;
			else
				store2 [i].s_inv_value = local_rec.loc_val;

			strcpy (store2 [i].s_alloc,pogd_rec.allocation);
			strcpy (store2 [i].s_cost_edit,pogd_rec.cost_edit);
		}
		else
		{
			store2 [i].inv_found = FALSE;
			store2 [i].s_exch = 1.00;
			local_rec.lexch_rate = 1.00;

			if (i == FOB)
			{
				store2 [i].s_exch = local_rec.exch_rate;
				local_rec.lexch_rate = local_rec.exch_rate;
			}

			strcpy (store2 [i].s_curr,"   ");
			store2 [i].hhsu_hash = 0L;

			sprintf (local_rec.category,"%-20.20s",cat_desc [i]);

			store2 [i].s_inv_value = 0.00;
			local_rec.fgn_val = 0.00;
			local_rec.loc_val = 0.00;

			strcpy (local_rec.allocation,"D");
			strcpy (store2 [i].s_alloc, "D");
			strcpy (store2 [i].s_cost_edit, "N");
		}
		putval (i);
	}
	lcount [PO_COST] = MAX_POGD;
	vars [scn_start].row = lcount [PO_COST];

	abc_selfield (sumr, !envVar.crFind ? "sumr_id_no" : "sumr_id_no3");

	scn_set (PO_HEAD);
	return;
}

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchSuin (
 char *key_val,
 long hhsu_hash)
{
	char	disp_amt [22];
	double	inv_balance;

	work_open ();
	suin_rec.hhsu_hash = hhsu_hash;
	strcpy (suin_rec.inv_no,key_val);
	save_rec ("#Document","#  Base Currency ");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val,strlen (key_val)) &&
		      (suin_rec.hhsu_hash == hhsu_hash))
	{
		if (suin_rec.approved [0] == 'Y')
		{
			inv_balance = suin_rec.amt - suin_rec.gst;
			sprintf (disp_amt, "%-14.2f ",DOLLARS (inv_balance));
			cc = save_rec (suin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	suin_rec.hhsu_hash = hhsu_hash;
	strcpy (suin_rec.inv_no,temp_str);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, DBFIND);
}
/*=======================
| Search for currency	|
=======================*/
void
SrchPocr (
 char *key_val)
{
	work_open ();

	save_rec ("#Cur","#Currency Description");
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",key_val);
	cc = find_rec (pocr,&pocr_rec,GTEQ,"r");
	while (!cc && !strncmp (pocr_rec.code,key_val,strlen (key_val)) &&
		      !strcmp (pocr_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code,pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr,&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, pocr, DBFIND);
}

/*===============================================
| Setup for Missing Weight / Volume Display	|
===============================================*/
void
OpenMissingItems (
 char *desc)
{
	Dsp_open (2,9,6);

	sprintf (err_str,"   Missing %-6.6s Values   ",desc);
	Dsp_saverec (err_str);
	Dsp_saverec (" From Inventory / Supplier ");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END] ");
}

/*=======================================
| Save Supplier / Item No. for display	|
=======================================*/
int
SameMissingItems (
	long	hhbrHash)
{
	abc_selfield (inmr,"inmr_hhbr_hash");

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	abc_selfield (inmr,"inmr_id_no");
	if (!cc)
	{
		sprintf (err_str," %-6.6s ^E %-16.16s ",local_rec.crd_no,inmr_rec.item_no);
		return (Dsp_saverec (err_str));
	}
	return (EXIT_SUCCESS);
}

/*===============
| Display Data	|
===============*/
void
ShowMissingItems (
 void)
{
	Dsp_srch ();
	Dsp_close ();
}

/*===============================
| Calculate Extended FOB total	|
===============================*/
double
DollarTotal (
 void)
{
	int		i;
	double	value;
	double	total = 0.00;

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		value = store [i].net_fob;
		value *= DPP (store [i].pur_conv);
		value *= store [i].quantity;
		total += value;
	}
	total = total;
	return (total);
}

/*===========================================
| Calculate duty allocation per line		|
|											|
| line1 extend_fob / exch_rate * duty% = A	|
| line2 extend_fob / exch_rate * duty% = B	|
| line3 extend_fob / exch_rate * duty% = C	|
|										--	|
|										 D	|
|											|
| line1    A/D*duty_invoice_total/line_qty	|
| line2    B/D*duty_invoice_total/line_qty	|
| line3    C/D*duty_invoice_total/line_qty	|
===========================================*/
double
DutyTotal (
 void)
{
	int		i;
	double	total = 0.00;
	double	value = 0.00;

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		value = out_cost ((float)(DutyCalculate (i)), (float)(store [i].outer));
		store [i].val_duty = value;
		value *= (double) store [i].quantity;
		total += value;
	}
	total = total;
	return (total);
}

/*===============================
| Calc Total Extended Weights	|
| display any items where the	|
| inis_weight is zero		|
===============================*/
double
WeightTotal (
 void)
{
	int		i;
	int missing = 0;
	int		check;		/* check for missing weights	*/
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("W");

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		if (check && store [i].weight <= 0.00)
		{
			if (!missing)
			{
                OpenMissingItems ("Weight");
				missing = 1;
			}
            if (SameMissingItems (store [i].hhbrHash))
                break;
		}
		value = store [i].weight;
		value *= store [i].quantity;
		total += value;
	}

	if (missing)
        ShowMissingItems ();

	return (total);
}

/*===============================
| Calc Total Extended Volumes	|
| display any items where the	|
| inis_volume is zero		|
===============================*/
double
VolumeTotal (
 void)
{
	int		i;
	int		missing = 0;
	int		check;		/* check for missing volumes	*/
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("V");

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		if (check && store [i].volume <= 0.00)
		{
			if (!missing)
			{
                OpenMissingItems ("Volume");
				missing = 1;
			}
            if (SameMissingItems (store [i].hhbrHash))
                break;
		}
		value = store [i].volume;
		value *= store [i].quantity;
		total += value;
	}

	if (missing)
        ShowMissingItems ();

	return (total);
}

/*=======================================
| Check what weightings are being used	|
| Pass Weighting Type - 				|
|	D(ollar,V(olume,W(eight				|
| Return 1 if Weighting type is used	|
| otherwise 0.							|
=======================================*/
int
CheckInvoice (
 char *weight_type)
{
	int		i;

	for (i = FOB;i <= O_4;i++)
		if (store2 [i].s_alloc [0] == weight_type [0])
			return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}


/*===============================================
| Calculate Spreading of Invoices Received		|
| on a per unit basis							|
===============================================*/
double
WeightCalc (
 int		lineIndex,		/* FOB,FRT,INS,...				*/
 int		line_no,	/* line number of current line in screen 3	*/
 double		fobTotal,
 double		weightTotal,
 double		volumeTotal,
 double		dutyTotal)
{
	double inv_value = store2 [lineIndex].s_inv_value;
	double calcValue = 0.0;

	if (lineIndex == DTY)
	{
		calcValue = store [line_no].val_duty * store [line_no].outer;

		if (dutyTotal != 0.00)
			calcValue /= dutyTotal;
		else
			calcValue = 0.00;

		calcValue *= inv_value;
	}
	else
	{
		switch (store2 [lineIndex].s_alloc [0])
		{
		/*---------------
		| Weight        |
		---------------*/
		case 'W':
			calcValue = (double) store [line_no].weight * store [line_no].outer;
			if (weightTotal != 0.00)
				calcValue /= weightTotal;
			else
				calcValue = 0.00;
			calcValue *= inv_value;
			break;

		/*---------------
		| Volume        |
		---------------*/
		case 'V':
			calcValue = (double) store [line_no].volume * store [line_no].outer;
			if (volumeTotal != 0.00)
				calcValue /= volumeTotal;
			else
				calcValue = 0.00;
			calcValue *= inv_value;
			break;

		default:
			switch (lineIndex)
			{
			case FOB:
				calcValue 	= store [line_no].net_fob;
				calcValue	*= DPP (store [line_no].pur_conv);
				break;

			default:
				calcValue 	= store [line_no].net_fob;
				calcValue	*= DPP (store [line_no].pur_conv);
				if (fobTotal != 0.00)
					calcValue /= fobTotal;
				else
					calcValue = 0.00;
				calcValue *= inv_value;
				break;
			}
			calcValue *= store [line_no].outer;
			break;
		}
	}

	return (calcValue);
}

/*===============================================
| Recalculate & Display screen for every change	|
===============================================*/
void
DisplayScreen (
 int		print_flag)
{
	int		i = line_cnt;
	int		line_no;
	int		this_page = line_cnt / TABLINES;
	double	cif_cost;
	double	value [MAX_POGD];
	double	dty_total = 0.00;
	double	fob_total = 0.00;
	double	wgt_total = 0.00;
	double	vol_total = 0.00;
	int		cost_edit = FALSE;

	for (line_no = FOB;line_no <= O_4;line_no++)
		if (store2 [line_no].s_cost_edit [0] == 'Y')
			cost_edit = TRUE;

	if (!cost_edit)
		return;

	dty_total = DutyTotal ();
	fob_total = DollarTotal ();
	wgt_total = WeightTotal ();
    vol_total = VolumeTotal ();

	print_at (0,0, ML (mlPoMess074));

	if (print_flag)
		putval (line_cnt);
	else
		scn_set (PO_LINES);

	if (store2 [DTY].s_inv_value > 0.0 && dty_total <= 0.0)
    {
		/*------------------------
		| Product is not dutiable |
		-------------------------*/
        print_err (ML (mlPoMess094));
        print_err (ML (mlPoMess095));
    }

	for (line_cnt = 0;line_cnt < lcount [PO_LINES];line_cnt++)
	{
		getval (line_cnt);

		/*-----------------------------------------------
		| Calculate default spreading of invoices	|
		-----------------------------------------------*/
		for (line_no = FOB;line_no <= O_4;line_no++)
		{
			if (store2 [line_no].s_cost_edit [0] == 'Y')
			{
				value [line_no] = WeightCalc
								 (
							  		line_no,
							      	line_cnt,
							      	fob_total,
							      	wgt_total,
							      	vol_total,
							      	dty_total
								);
			}
			else
			{
				switch (line_no)
				{
				case	FOB:
					break;

				case	FRT:
				case	INS:
					value [FRT] = local_rec.loc_fi;
					value [INS] = 0.00;
					break;

				case	INT:
				case	B_C:
				case	O_1:
				case	O_2:
				case	O_3:
				case	O_4:
					value [INT] = 0.00;
					value [B_C] = 0.00;
					value [O_1] = local_rec.other;
					value [O_2] = 0.00;
					value [O_3] = 0.00;
					value [O_4] = 0.00;
					break;

				case	DTY:
					value [DTY] = local_rec.duty_val;
					break;

				default:
					break;
				}
			}
		}

		SR.net_fob = local_rec.net_fob;

		cif_cost 	= local_rec.net_fob;
		cif_cost 	*= store [line_cnt].outer;
		cif_cost	*= DPP (store [line_cnt].pur_conv);

		if (SR.exch_rate != 0.00)
			local_rec.cif_loc = cif_cost / SR.exch_rate;
		else
			local_rec.cif_loc = 0.00;

		local_rec.loc_fi 	= value [FRT] + value [INS];
		SR.val_fi     		= value [FRT] + value [INS];

		local_rec.cif_loc 	+= local_rec.loc_fi;

		local_rec.duty_val 	= value [DTY];
		SR.val_duty     	= value [DTY];

		local_rec.lic_val 	= (double) SR.lic_rate;
		local_rec.lic_val 	*= local_rec.cif_loc;

		local_rec.other 	= value [O_1] + value [O_2] +
				  			  value [O_3] + value [O_4] +
				  			  value [INT] + value [B_C];

		/*-------------------------------
		| Calculate Landed Cost local	|
		-------------------------------*/
		local_rec.land_cst = local_rec.cif_loc +
			     	         local_rec.duty_val +
			     	         local_rec.lic_val +
			     	         local_rec.other;

		local_rec.Dsp_land_cst = local_rec.land_cst;

		SR.land_cost = local_rec.land_cst;

		putval (line_cnt);

		if (print_flag && this_page == line_cnt / TABLINES)
				line_display ();
	}
	line_cnt = i;

	getval (line_cnt);

}

/*==============================================================
| Set fields to be edited once line on costing screen changed. |
==============================================================*/
void
SetupForEdit (
 int	line_no)
{
	switch (line_no)
	{
	case	FOB:
		strcpy (store2 [FOB].s_cost_edit, "Y");
		break;

	case	FRT:
	case	INS:
		strcpy (store2 [FRT].s_cost_edit, "Y");
		strcpy (store2 [INS].s_cost_edit, "Y");
		break;

	case	INT:
	case	B_C:
	case	O_1:
	case	O_2:
	case	O_3:
	case	O_4:
		strcpy (store2 [INT].s_cost_edit, "Y");
		strcpy (store2 [B_C].s_cost_edit, "Y");
		strcpy (store2 [O_1].s_cost_edit, "Y");
		strcpy (store2 [O_2].s_cost_edit, "Y");
		strcpy (store2 [O_3].s_cost_edit, "Y");
		strcpy (store2 [O_4].s_cost_edit, "Y");
		break;

	case	DTY:
		strcpy (store2 [DTY].s_cost_edit, "Y");
		break;
	}
}

/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
tab_other (
 int iline)
{
	if (cur_screen == PO_LINES)
	{
		if (envVar.poMaxLines)
		{
			/*---------------------------------
			| Warning, maximum lines exceeded. |
			----------------------------------*/
			if (prog_status == ENTRY && iline >= envVar.poMaxLines)
				centre_at (4, 132, ML ("Maximum lines exceeded"));
			if (prog_status != ENTRY && lcount [PO_LINES] > envVar.poMaxLines)
				centre_at (4, 132, ML ("Maximum lines exceeded"));
		}
		if (store [iline].exch_rate == 0.00)
		{
			move (0, 5); cl_line ();
			move (0, 6); cl_line ();
			move (0, 7); cl_line ();
			clear_mess ();
			return;
		}
		if (argumentFlag [3] == 'Y')
		{
			/*---------------
			| %R Branch :   |
			| %R Warehouse: |
			---------------*/
			sprintf (err_str, ML (mlStdMess039),
							local_rec.br_no,local_rec.br_name);
			print_at (5, 0, "%R %s", err_str);

			sprintf (err_str, ML (mlStdMess099),
							local_rec.wh_no,local_rec.wh_name);
			print_at (5, 53, "%R %s", err_str);
		}
		/*-------------------
		| %R Line : 		|
		| %R Ship No  : 	|
		| %R Exch Rate   : 	|
		| %R Pricing Unit: 	|
		| %R Desc   : 		|
		| %R Std. UOM : 	|
		-------------------*/
		print_at (6,  0, ML (mlPoMess075));
		print_at (6, 53, ML (mlPoMess076));
		print_at (6, 75, ML (mlPoMess077));
		print_at (6,102, ML (mlPoMess078));
		print_at (7,  0, ML (mlPoMess079));
		print_at (7, 53, ML (mlPoMess080));
		print_at (7, 75, ML ("%R Supp. UOM "));
		print_at (7, 102, ML ("%R Supp. Price  "));
		print_at (8, 75, ML ("%R Tot. Weight "));
		print_at (8, 102, ML ("%R Tot. Volume "));
		print_at (8,  88, "%13.2f", totalWeight);
		print_at (8, 115, "%13.2f", totalVolume);

		print_at (6, 11, "%3d", iline + 1);
		if (store [iline].ship_no == 0L)
		{
			/*------
			| N/A   |
			--------*/
			print_at (6, 63, "%-12.12s"," ");
			print_at (6, 63, ML (mlPoMess117));
			FLD ("qty")			= origQty;
			FLD ("due_date")	= origDueDate;
			FLD ("UOM")			= origUOM;
			clear_mess ();
		}
		else
		{
			print_at (6, 63, "%-12.12s", GetShipmentNo (store[iline].ship_no));
			if (!F_HIDE (label ("due_date")))
				FLD ("due_date")	=	NA;

			if (!F_HIDE (label ("UOM")))
				FLD ("UOM")			=	NA;

			if (!F_HIDE (label ("qty")))
				FLD ("qty")			=	NA;

			print_mess (ML ("Note : Quantity and due date must be maintained in shipment"));
		}

		print_at (6, 91, "%10.4f", store [iline].exch_rate);

		print_at (7, 11, "%-40.40s",
			check_class (store [iline]._class) ? store [iline].nsDesc [0]
											: store [iline].item_desc);

		print_at (7, 63, "%-4.4s", store [iline].std_uom);
		print_at (7, 87, "%-4.4s", store [iline].sup_uom);
		print_at (7, 116, "%12.4f", store [iline].cst_price / DPP (store [iline].sup_pur_conv));
		if (store [iline].outer > 0.00)
			print_at (6,120, "%8.2f", store [iline].outer);
		else
			print_at (6,120, "%8.2f", 1.00);

		if (strcmp (store [iline].ser_no, serialSpace))
		{
			/*----------------
			| %R Serial No : |
			----------------*/
			print_at (8, 0, ML (mlPoMess081));
			print_at (8, 15, "%-25.25s", store [iline].ser_no);
		}
		else
		{
			print_at (8, 0, "             ");
			print_at (8, 15, "%-25.25s", serialSpace);
		}

		fflush (stdout);

		if (prog_status != ENTRY)
			strcpy (local_rec.inp_uom, store [iline].inp_uom);
	}
	if (cur_screen == PO_COST)
	{
		print_at (4,1,"%130.130s"," ");
		PrintCostDesc (iline);
	}

	return;
}
/*=============================
| Calculate default costings. |
=============================*/
void
RecalcCosting (
 void)
{
	int		i;

	double	value = 0.00;

	store2 [DTY].s_inv_value = 0.00;
	store2 [FRT].s_inv_value = 0.00;
	store2 [FOB].s_inv_value = 0.00;

	for (i = 0;i < lcount [PO_LINES];i++)
	{
		value 	=	out_cost (store [i].val_duty,  store [i].outer);
		value 	*=  (double) store [i].quantity;
		store2 [DTY].s_inv_value += value;

		value 	=	out_cost (store [i].val_fi,  store [i].outer);
		value 	*=  (double) store [i].quantity;
		store2 [FRT].s_inv_value += value;

		value = store [i].net_fob;
		value *= DPP (store [i].pur_conv);
		value *= store [i].quantity;
		store2 [FOB].s_inv_value += value;
	}
}

/*========================
| Print Costing Details. |
========================*/
void
PrintCostDesc (
 int		line_no)
{

/*--------------------------------------------------------------------
| FOB - Goods (FOB) is always total of line items and is not Spread. |
| FRT - Freight & Insurance will be spread automatically if cost 	 |
| 		input into these fields.									 |
| INT - Interest, Bank & Other 1-4 will be spread into 				 |
| 		'Int/Bank/Other' field on item screen if value input into    |
| 		any on these fields. 										 |
| DTY - Duty will be spread Automatically if value input into duty   |
|       field. 														 |
--------------------------------------------------------------------*/

	switch (line_no)
	{
	case	FOB:
		sprintf (err_str, ML (mlPoMess082));
		rv_pr (err_str, (132 - strlen (err_str)) / 2,4,1);
		break;

	case	FRT:
	case	INS:
		sprintf (err_str, ML (mlPoMess083));
		rv_pr (err_str, (132 - strlen (err_str)) / 2,4,1);
		break;

	case	INT:
	case	B_C:
	case	O_1:
	case	O_2:
	case	O_3:
	case	O_4:
		sprintf (err_str, ML (mlPoMess084));
		rv_pr (err_str, (132 - strlen (err_str)) / 2,4,1);
		break;

	case	DTY:
		sprintf (err_str, ML (mlPoMess085));
		rv_pr (err_str, (132 - strlen (err_str)) / 2,4,1);
		break;

	default:
		break;
	}
	fflush (stdout);
}

/*=======================================
| Check is costing exists for Shipment. |
=======================================*/
int
CheckShipmentCosting (
 long	hhsh_hash)
{
	if (poln_rec.ship_no == 0L)
		return (EXIT_SUCCESS);

	abc_selfield (pogd, "pogd_id_no2");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhsh_hash = hhsh_hash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd,&pogd_rec,GTEQ,"r");
	if  (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) &&
		     pogd_rec.hhsh_hash == hhsh_hash)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
/*=================================
| Update Pre-receipt Serial item. |
=================================*/
void
UpdateInsf (
 void)
{
	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		return;

	open_rec (insf,insf_list,INSF_NO_FIELDS,"insf_id_no2");

	insf_rec.hhwh_hash = incc_rec.hhwh_hash;
	sprintf (insf_rec.serial_no,"%-25.25s",poln_rec.serial_no);
	cc = find_rec (insf,&insf_rec,COMPARISON,"u");
	if (cc)
	{
		abc_fclose (insf);
		return;
	}
	insf_rec.exch_rate   = poln_rec.exch_rate;
	insf_rec.fob_fgn_cst = poln_rec.fob_fgn_cst;
	insf_rec.fob_nor_cst = poln_rec.fob_nor_cst;
	insf_rec.frt_ins_cst = poln_rec.frt_ins_cst;
	insf_rec.duty        = poln_rec.duty;
	insf_rec.licence     = poln_rec.licence;
	insf_rec.lcost_load  = poln_rec.lcost_load;

	insf_rec.land_cst    = poln_rec.fob_nor_cst +
				           poln_rec.lcost_load 	+
		          	       poln_rec.duty 		+
		          	       poln_rec.licence;

	insf_rec.istore_cost = poln_rec.land_cst;
	insf_rec.est_cost    = poln_rec.land_cst;

	cc = abc_update (insf,&insf_rec);
	if (cc)
		file_err (cc, insf, "DBUPDATE");

	abc_fclose (insf);
	return;
}
/*=================================
| Delete Pre-receipt Serial item. |
=================================*/
void
DeleteInsf (
 void)
{
	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		return;

	open_rec (insf,insf_list,INSF_NO_FIELDS,"insf_id_no2");

	insf_rec.hhwh_hash = incc_rec.hhwh_hash;
	sprintf (insf_rec.serial_no,"%-25.25s",poln_rec.serial_no);
	cc = find_rec (insf,&insf_rec,COMPARISON,"u");
	if (cc)
	{
		abc_fclose (insf);
		return;
	}
	cc = abc_delete (insf);
	if (cc)
		file_err (cc, insf, "DBDELETE");

	return;
}

/*=========================================================
| Load category descriptions if defined else use default. |
=========================================================*/
void
LoadCategoryDesc (
 void)
{
	char	*sptr;
	int		i;

	print_at (0,0, ML (mlStdMess035));
	for (i = 0; i < 10; i++)
	{
		putchar ('.');fflush (stdout);
		switch (i)
		{
		case 0:
			sprintf (cat_desc [i], "%-20.20s", invoiceCategory [i]);
			break;

		case 1:
			sptr = chk_env ("PO_OS_1");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;

		case 2:
			sptr = chk_env ("PO_OS_2");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 3:
			sptr = chk_env ("PO_OS_3");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 4:
			sptr = chk_env ("PO_OS_4");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 5:
			sprintf (cat_desc [i], "%-20.20s", invoiceCategory [i]);
			break;
		case 6:
			sptr = chk_env ("PO_OTHER1");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 7:
			sptr = chk_env ("PO_OTHER2");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 8:
			sptr = chk_env ("PO_OTHER3");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;
		case 9:
			sptr = chk_env ("PO_OTHER4");
			sprintf (cat_desc [i],"%-20.20s", (sptr == (char *)0)
						? invoiceCategory [i] : sptr);
			break;

		default:
			break;
		}
	}
}

void
GetWarehouse (
	long	hhccHash)
{
	if (hhccHash == 0L)
	{
		if (warehouseSelected)
		{
			abc_selfield (ccmr,"ccmr_id_no");
			warehouseSelected = FALSE;
		}

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,comm_rec.est_no);
		strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, ccmr, DBFIND);

		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, esmr, DBFIND);
		strcpy (local_rec.br_name, esmr_rec.est_name);
	}
	else
	{
		if (!warehouseSelected)
		{
			abc_selfield (ccmr,"ccmr_hhcc_hash");
			warehouseSelected = TRUE;
		}

		ccmr_rec.hhcc_hash	=	hhccHash;
		if (find_rec (ccmr,&ccmr_rec,COMPARISON,"r"))
		{
			abc_selfield (ccmr,"ccmr_id_no");
			GetWarehouse (0L);
			return;
		}
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, esmr, DBFIND);

		strcpy (local_rec.br_name, esmr_rec.est_name);
	}
	return;
}

int
CheckReorder (
 char	*_class)
{
	if (strchr (envVar.poReorder, _class [0]) == (char *) 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
AddIncc (
	long	hhccHash,
	long	hhbrHash)
{
	char	temp_sort [29];

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;

	sprintf (temp_sort,"%s%11.11s%-16.16s", inmr_rec.inmr_class,
					       		inmr_rec.category,
				               	inmr_rec.item_no);

	strcpy (incc_rec.sort,temp_sort);
	incc_rec.first_stocked = TodaysDate ();
	incc_rec.closing_stock = 0.00;
	incc_rec.qc_time		  =	0.00;

	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.abc_code,  "A");
	strcpy (incc_rec.abc_update,"Y");
	strcpy (incc_rec.stat_flag,"0");
	strcpy (incc_rec.qc_centre, "    ");

	if ((cc = abc_add (incc ,&incc_rec)))
		file_err (cc, incc, "DBADD");

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc , &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, incc, DBFIND);
}

/*===========================================================
| Input not stock description lines for non-stock products. |
===========================================================*/
void
InputPons (
 int	line_cnt)
{
	int 	i,
			tx_window;
	char	disp_str [200];

	sprintf (err_str,"ADDITIONAL DESCRIPTION FOR ITEM %s",inmr_rec.item_no);
	tx_window = txt_open (2, 1, 5, 40, 5, err_str);

	for (i = 0; i < MAX_Pons ; i++)
	{
		sprintf (disp_str, "%-40.40s", SR.nsDesc [i]);
		txt_pval (tx_window, disp_str, 0);
	}

	txt_edit (tx_window);

	for (i = 0; i < 5 ; i++)
		sprintf (SR.nsDesc [i], "%-40.40s", txt_gval (tx_window, i + 1));

	txt_close (tx_window, FALSE);
	ClearBox (0,2,43,7);
}

/*=============================================================
| Clear screen where box to be drawn + if _box then draw box. |
=============================================================*/
void
ClearBox (
 int	x,
 int	y,
 int	h,
 int	v)
{
	int	i,

		j;

	j = v;
	i = y;

	while (j--)
	{
		if (h > 1)
			print_at (i, x, "%*.*s", h, h, " ");
		i++;
	}
}

/*=============================================
| Update purchase order non stock lines file. |
=============================================*/
void
UpdatePons (
 int		line_cnt,
 long	hhpl_hash)
{
	int	i;

	for (i = 0; i < MAX_Pons; i++)
	{
		pons_rec.hhpl_hash 	= hhpl_hash;
		pons_rec.line_no 	= i;
		cc = find_rec (pons, &pons_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (pons_rec.desc, "%-40.40s", SR.nsDesc [i]);
			/*-----------------------------------
			| Add line only if it is not blank. |
			-----------------------------------*/
			if (strcmp (pons_rec.desc, nonStockSpace))
			{
				cc = abc_add (pons, &pons_rec);
				if (cc)
					file_err (cc, pons, "DBADD");
			}
		}
		else
		{
			sprintf (pons_rec.desc, "%-40.40s", SR.nsDesc [i]);
			/*--------------------------------------
			| Update line only if it is not blank. |
			--------------------------------------*/
			if (strcmp (pons_rec.desc, nonStockSpace))
			{
				cc = abc_update (pons, &pons_rec);
				if (cc)
					file_err (cc, pons, "DBUPDATE");
			}
			else
			{
				cc = abc_delete (pons);
				if (cc)
					file_err (cc, pons, "DBDELETE");
			}
		}
	}
}

/*===========================================
| Load purchase order non stock lines file. |
===========================================*/
void
LoadPons (
 int		line_cnt,
 long	hhpl_hash)
{
	int	i;

	for (i = 0; i < MAX_Pons; i++)
		sprintf (SR.nsDesc [i], "%40.40s", " ");

	pons_rec.hhpl_hash 	= hhpl_hash;
	pons_rec.line_no 	= 0;
	cc = find_rec (pons, &pons_rec, GTEQ, "r");
	while (!cc && pons_rec.hhpl_hash == hhpl_hash)
	{
		sprintf (SR.nsDesc [pons_rec.line_no], "%40.40s", pons_rec.desc);

		cc = find_rec (pons, &pons_rec, NEXT, "r");
	}
}
/*=============================================
| Delete purchase order non stock lines file. |
=============================================*/
void
DeletePons (
 long	hhpl_hash)
{
	pons_rec.hhpl_hash 	= hhpl_hash;
	pons_rec.line_no 	= 0;
	cc = find_rec (pons, &pons_rec, GTEQ, "r");
	while (!cc && pons_rec.hhpl_hash == hhpl_hash)
	{
		cc = abc_delete (pons);
		if (cc)
			file_err (cc, pons, "DBDELETE");

		pons_rec.hhpl_hash 	= hhpl_hash;
		pons_rec.line_no 	= 0;
		cc = find_rec (pons, &pons_rec, GTEQ, "r");
	}
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	clear ();
	swide ();
	/*----------------------------
	| Purchase Order Maintenance. |
	-----------------------------*/
	switch (scn)
	{
	case	PO_LINES:
		tab_row = 9;
		tab_col = 0;
		break;

	case	PO_COST:
		tab_row = 6;
		tab_col = 8;
		break;
	default:
		break;
	}

	if (scn != cur_screen)
		scn_set (scn);

	rv_pr (ML (mlPoMess086), 49,0,1);

	/*--------------------
	| Last Order: %6s/%7s |
	---------------------*/
	print_at (0,90, ML (mlPoMess007), local_rec.prev_crd_no,local_rec.prev_po);

	pr_box_lines (scn);

	switch (scn)
	{
	case	PO_HEAD:
		/*--------------------------------------------------
		| P a y m e n t   T e r m s. 						|
		| S t a n d a r d   I n s t r u c t i o n s. 		|
		| S u p p l i e r   S h i p p i n g   D e t a i l s.|
		---------------------------------------------------*/
		us_pr (ML (mlPoMess087),5,8,1);
		us_pr (ML (mlPoMess088),5,13,1);
		us_pr (ML (mlPoMess089),5,17,1);
		break;

	case	PO_LINES:
		line_at (21,0,132);
		if (envVar.poMaxLines)
		{
			/*---------------------------------
			| Warning, maximum lines exceeded |
			---------------------------------*/
			if (prog_status == ENTRY && line_cnt >= envVar.poMaxLines)
				centre_at (4, 132, ML ("Maximum lines exceeded"));

			if (prog_status != ENTRY && lcount [PO_LINES] > envVar.poMaxLines)
				centre_at (4, 132, ML ("Maximum lines exceeded"));
		}
		PrintOtherStuff ();
		if (calculateDiscount)
			DisplayScreen (FALSE);

		break;

	default:
		break;
	}

	print_at (22, 0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (22, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (22, 90, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

float
RndMltpl (
	float	ord_qty,
	char	*rnd_type,
	float	ord_mltpl,
	float	min_qty)
{
	float	wrk_qty;
	float	up_qty;
	float	down_qty;

	if (ord_qty == 0.00)
		return (0.00);

	ord_qty = twodec (ord_qty);

	if (ord_mltpl == 0.00)
		return ((ord_qty < min_qty) ? min_qty : ord_qty);

	ord_qty -= min_qty;
	if (ord_qty < 0.00)
		ord_qty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (float) (ord_qty / ord_mltpl);
	if (ceil (wrk_qty) == wrk_qty)
		return (ord_qty + min_qty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (rnd_type [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (float) (ord_qty / ord_mltpl);
		wrk_qty = (float) ceil (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (float) (ord_qty / ord_mltpl);
		wrk_qty = (float) floor (twodec (wrk_qty));
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty	= (float) ord_qty;
		wrk_qty = (float) (up_qty / (float) ord_mltpl);
		wrk_qty = (float) ceil (wrk_qty);
		up_qty	= (float) (wrk_qty * ord_mltpl);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty 	= (float) ord_qty;
		wrk_qty 	= (float) (down_qty / (float) ord_mltpl);
		wrk_qty		= (float) floor (wrk_qty);
		down_qty	= (float) (wrk_qty * ord_mltpl);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (float) ord_qty) <= ((float) ord_qty - down_qty))
			ord_qty = (float) up_qty;
		else
			ord_qty = (float) down_qty;

		break;

	default:
		break;
	}
	return (min_qty + ord_qty);
}

/*---------------------------------
| Menu called when CTRL W pressed |
---------------------------------*/
int
win_function (
 int fld,
 int lin,
 int scn,
 int mode)
{
	if (scn == PO_LINES)
	{
		InputPons (lin);
		PrintOtherStuff ();
		line_at (4,0,132);
	}

	restart = FALSE;

	return (PSLW);
}

int
LoadSupplier (
 int	field)
{
	abc_selfield (sumr,"sumr_hhsu_hash");
	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sumr, DBFIND);

	sprintf (local_rec.crd_no, "%-6.6s", sumr_rec.crd_no);
	cc = spec_valid (field -1);
	abc_selfield (sumr, envVar.crFind ? "sumr_id_no3" : "sumr_id_no");
	return (cc);
}

#ifndef GVISION
/*---------------------------------------------
| Allow editing of dicounts for current line. |
---------------------------------------------*/
void
ViewDiscounts (
 void)
{
	int		key;
	int		currFld;
	int		tmpLineCnt;
	double	oldFobFgn;
	float	oldDisc [4];

	/*------------------
	| Save old values. |
	------------------*/
	oldFobFgn = local_rec.grs_fgn;
	oldDisc [0] = SR.discArray [0];
	oldDisc [1] = SR.discArray [1];
	oldDisc [2] = SR.discArray [2];
	oldDisc [3] = SR.discArray [3];

	/*----------------------
	| Draw box and fields. |
	----------------------*/
	DrawDiscScn ();

	/*-----------------------------------------------
	| Allow cursor movement and selection for edit. |
	| Exit without change on FN1.                   |
	| Exit saving changes on FN16.                  |
	-----------------------------------------------*/
	crsr_off ();
	currFld = 0;
	restart = FALSE;
	DispFields (currFld);
	while ((key = getkey ()) != FN16)
	{
		switch (key)
		{
		case BS:
		case LEFT_KEY:
		case UP_KEY:
			currFld--;
			if (currFld < 0)
				currFld = 4;
			break;

		case DOWN_KEY:
		case RIGHT_KEY:
		case ' ':
			currFld++;
			if (currFld >= 5)
				currFld = 0;
			break;

		case '\r':
			InputField (currFld);
			break;

		case FN1:
			/*---------------------
			| Restore old values. |
			---------------------*/
			local_rec.grs_fgn = oldFobFgn;
			SR.discArray [0] = oldDisc [0];
			SR.discArray [1] = oldDisc [1];
			SR.discArray [2] = oldDisc [2];
			SR.discArray [3] = oldDisc [3];
			restart = TRUE;
			break;

		case FN3:
			tmpLineCnt = line_cnt;
			heading (PO_LINES);
			line_cnt = tmpLineCnt;
			lcount [PO_LINES] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [PO_LINES];
			scn_display (PO_LINES);
			DrawDiscScn ();
			DispFields (currFld);
			break;
		}

		DispFields (currFld);
		if (restart)
			break;
	}
	restart = FALSE;
}

void
DrawDiscScn (
 void)
{
	int		i;
	int		fldWid;
	int		headXPos;

	/*-----------
	| Draw box. |
	-----------*/
	cl_box (DBOX_LFT, DBOX_TOP, DBOX_WID, DBOX_DEP);

	/*------------------------------
	| Draw middle horizontal line. |
	------------------------------*/
	move (DBOX_LFT + 1, DBOX_TOP + 2);
	line (DBOX_WID - 1);
	move (DBOX_LFT, DBOX_TOP + 2);
	PGCHAR (10);
	move (DBOX_LFT + DBOX_WID - 1, DBOX_TOP + 2);
	PGCHAR (11);

	/*-------------------------------
	| Draw vertical dividing lines. |
	-------------------------------*/
	for (i = 1; i < 6; i++)
		VertLine (DBOX_LFT + discScn [i].xPos, DBOX_TOP);

	/*---------------
	| Draw heading. |
	---------------*/
	sprintf (err_str, (SR.cumulative) ? ML (mlPoMess134) : ML (mlPoMess135));
	headXPos = DBOX_LFT + (DBOX_WID - strlen (err_str)) / 2;
	rv_pr (err_str, headXPos, DBOX_TOP, 1);

	/*---------------
	| Draw prompts. |
	---------------*/
	for (i = 0; i < 6; i++)
	{
		fldWid = strlen (discScn [i].fldPrompt);
		print_at (DBOX_TOP + 1,
				 DBOX_LFT + discScn [i].xPos + 1,
				 " %-*.*s ",
				 fldWid,
				 fldWid,
				 discScn [i].fldPrompt);
	}
}

/*========================================
| Print lines and stuff in Popup window. |
========================================*/
void
VertLine (
 int		xPos,
 int		yPos)
{
	move (xPos, yPos);
	PGCHAR (8);

	move (xPos, yPos + 1);
	PGCHAR (5);

	move (xPos, yPos + 2);
	PGCHAR (7);

	move (xPos, yPos + 3);
	PGCHAR (5);

	move (xPos, yPos + 4);
	PGCHAR (9);
}

/*=======================================
| Input discount stuff in Popup window. |
=======================================*/
void
DispFields (
 int	rvsField)
{
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [0].xPos + 2,
			 		"%13.4f", local_rec.grs_fgn);
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [1].xPos + 2,
			 		"%6.2f", ScreenDisc (SR.discArray [0]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [2].xPos + 2,
			 		"%6.2f", ScreenDisc (SR.discArray [1]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [3].xPos + 2,
			 		"%6.2f", ScreenDisc (SR.discArray [2]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [4].xPos + 2,
			 		"%6.2f", ScreenDisc (SR.discArray [3]));

	local_rec.net_fob = CalcNet (local_rec.grs_fgn, SR.discArray, SR.cumulative);
	SR.net_fob = local_rec.net_fob;
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [5].xPos + 2,
			 "%12.4f",local_rec.net_fob);

	/*--------------------------
	| Print highlighted field. |
	--------------------------*/
	switch (rvsField)
	{
	case 0:
		sprintf (err_str, "%13.4f", local_rec.grs_fgn);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		sprintf (err_str, "%6.2f", ScreenDisc (SR.discArray [rvsField - 1]));
		break;
	}
	rv_pr (err_str, DBOX_LFT + discScn [rvsField].xPos + 2, DBOX_TOP + 3, 1);
}

void
InputField (
 int	fld)
{
	int	fieldOk;

	crsr_on ();

	fieldOk = FALSE;
	while (!fieldOk)
	{
		fieldOk = TRUE;
		switch (fld)
		{
		case 0:
			local_rec.grs_fgn = getdouble (DBOX_LFT + discScn [fld].xPos + 2,
									  	DBOX_TOP + 3,
									  	discScn [fld].fldMask);

			break;

		case 1:
		case 2:
		case 3:
		case 4:
			SR.discArray [fld - 1] = getfloat (DBOX_LFT + discScn [fld].xPos + 2,
										 	DBOX_TOP + 3,
										 	discScn [fld].fldMask);
			if (SR.discArray [fld - 1] > 99.99)
			{
				/*-------------------------
				| Discount may not > 99.99 |
				--------------------------*/
				print_mess (ML (mlStdMess120));
				sleep (sleepTime);
				clear_mess ();
				fieldOk = FALSE;
			}
			SR.discArray [fld - 1] = ScreenDisc (SR.discArray [fld - 1]);
			break;
		}
	}
	crsr_off ();
}
#endif	/* GVISION */

/*==========================
| Reverse Screen Discount. |
==========================*/
float
ScreenDisc (
 float	DiscountPercent)
{
	if (envVar.soDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

/*=======================================================
| Convert value passed (Qty in STD UOM) to Lcl UOM (UOM |
| specified by local_rec.whUom                          |
=======================================================*/
float
ToLclUom (
	float	stdQty,
	float	CnvFct)
{
	float	cnvQty;

	if (CnvFct == 0.00)
		return (stdQty);

	if (envVar.threePlSystem)
		return (stdQty);

	cnvQty = stdQty * CnvFct;

	return ((float)(twodec (cnvQty)));
}

/*==================================================
| Convert value passed (Qty in LCL UOM) to STD UOM |
==================================================*/
float
ToStdUom (
	float	lclQty,
	float	CnvFct)
{
	float	cnvQty;

	if (CnvFct == 0.00)
		return (lclQty);

	if (lclQty == 0.00)
		return (0.00);

	if (envVar.threePlSystem)
		return (lclQty);

	cnvQty = lclQty / CnvFct;

	return ((float) twodec (cnvQty));
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char 	*key_val,
 int		line_cnt)
{
	work_open ();
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, SR.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR.uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, SR.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*======================================================
| Initialisation of strings in array for multilingual. |
======================================================*/
void
InitML (
 void)
{
	/* initialize default values for array */
    _discScn (0, "  FOB (FGN)  ",	0,  "NNNNNNNN.NNNN"),
	_discScn (1, "Reg %", 			15, "NNN.NN"),
	_discScn (2, "Disc A", 			24, "NNN.NN"),
	_discScn (3, "Disc B", 			33, "NNN.NN"),
	_discScn (4, "Disc C", 			42, "NNN.NN"),
	_discScn (5, "NET FOB(FGN)", 	51, "NNNNNNN.NNNN"),
	_discScn (6, "", 			0,  ""),

    _discScnSetPrompt (0, ML (mlPoMess223));
    _discScnSetPrompt (1, ML (mlPoMess224));
    _discScnSetPrompt (2, ML (mlPoMess225));
    _discScnSetPrompt (3, ML (mlPoMess226));
    _discScnSetPrompt (4, ML (mlPoMess227));
    _discScnSetPrompt (5, ML (mlPoMess228));
}

/*=========================================================
| This function frees the field prompt if it's allocated. |
=========================================================*/
void
_discScnFreePrompt (
 int Index)
{
    if (discScn [Index].fldPrompt != NULL)
        free (discScn [Index].fldPrompt);
}

/*====================================================
| This one sets the prompt to a new value, frees the |
| previous string if necessary                       |
====================================================*/
void
_discScnSetPrompt (
 int Index,
 char* FldPrompt)
{
    _discScnFreePrompt (Index);
    discScn [Index].fldPrompt = (char *) malloc (strlen (FldPrompt) + 1);
    strcpy (discScn [Index].fldPrompt, FldPrompt);
}

/*===========================================
| Set values to the fields of the structure |
===========================================*/
void
_discScn (
	int		Index,
	char	*FldPrompt,
	int 	XPos,
	char	*FldMask)
{
    discScn [Index].fldPrompt = NULL;
    _discScnSetPrompt (Index, FldPrompt);
    discScn [Index].xPos = XPos;
    strncpy (discScn [Index].fldMask, FldMask, 16);
}

/*===================================
| Clean up screen discount prompts. |
===================================*/
void
CleanUpPrompt (
 void)
{
    _discScnFreePrompt (0);
    _discScnFreePrompt (1);
    _discScnFreePrompt (2);
    _discScnFreePrompt (3);
    _discScnFreePrompt (4);
    _discScnFreePrompt (5);
    _discScnFreePrompt (6);
}

/*=============================
| See if shipment is on file. |
=============================*/
char*
GetShipmentNo (
	long    hhshHash)
{

	posh_rec.hhsh_hash	=	hhshHash;
	cc = find_rec (posh, &posh_rec, EQUAL, "r");
	if (cc)
		strcpy (err_str, " N/A ");
	else
		sprintf (err_str, "%-12.12s", posh_rec.csm_no);

	return (err_str);
}

/*==============================================
| Calculate number of virtual lines on the PO. |
==============================================*/
int
CalcVirtualLines (int entryMode)
{
	int idx;
	int actLines = 0;
	int virLines = 0;

	if (entryMode)
		actLines = line_cnt + 1;
	else
		actLines = lcount[PO_LINES];
	for (idx = 0; idx < actLines; idx++)
	{
		if (store[idx].ser_item[0] == 'Y' || store[idx].ser_item[0] == 'y')
			virLines += store[idx].quantity;
		else
			virLines++;
	}
	return (virLines);
}

/*==================================================
| Calculate total weight and volume on each entry. |
==================================================*/
void
RunningWVTotal (
 int	editUom)
{

	int i;
	totalWeight = 0.00;
	totalVolume = 0.00;

	for (i = 0; i < ((prog_status == ENTRY) ? line_cnt + 1 : lcount[PO_LINES]); i++)
	{
		totalWeight += 	store[i].weight *
					 	ToLclUom (store[i].quantity, store[i].pur_conv);
		totalVolume += 	store[i].volume *
						ToLclUom (store[i].quantity, store[i].pur_conv);
	}
	totalWeight = twodec(totalWeight);
	totalVolume = twodec(totalVolume);
	print_at (8, 75, ML ("%R Tot. Weight "));
	print_at (8, 102, ML ("%R Tot. Volume "));
	print_at (8,  88, "%13.2f", totalWeight);
	print_at (8, 115, "%13.2f", totalVolume);
}
/*=================================================
| Find Inuv record to retreave weight and volume. |
=================================================*/
void
GetInuvWgtVol(
	int		no_inis,
	long	hhbrHash,
	long	hhumHash,
	float	*weight,
	float	*volume)
{
	inuv_rec.hhbr_hash  =   hhbrHash;
	inuv_rec.hhum_hash  =   hhumHash;
	if (find_rec (inuv, &inuv_rec, COMPARISON, "r"))
	{
		*weight = (no_inis) ? 0.00 : inis_rec.weight;
		*volume = (no_inis) ? 0.00 : inis_rec.volume;
	}
	else
	{
		*weight = inuv_rec.weight;
		*volume = inuv_rec.volume;
	}
}

/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment (
 void)
{
	char	*sptr;

	/*--------------------------
	| Supplier Company Owned.  |
	--------------------------*/
	sptr = chk_env ("CR_CO");
	envVar.crCo = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*------------------------------------
	| Supplier Find variable for Search. |
	------------------------------------*/
	sptr = chk_env ("CR_FIND");
	envVar.crFind = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-------------------------
	| Purchase order overide. |
	-------------------------*/
	sptr = chk_env ("PO_OVERRIDE");
	envVar.poOverride = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	/*-------------------------
	| Ikea special po numbers |
	-------------------------*/
	sptr = chk_env ("IKEA_PO_NUMBERS");
	envVar.IkeaPoNumbers = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-----------------------
	| Ikea system specific. |
	-----------------------*/
	sptr = chk_env ("IKEA_SYSTEM");
	envVar.IkeaSystem = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-----------------------------------------
	| Max number of lines per purchase order. |
	-----------------------------------------*/
	sptr = chk_env ("PO_MAX_LINES");
	envVar.poMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*----------------------------------------------------
	| Max number of times an item can be on the same PO. |
	----------------------------------------------------*/
	sptr = chk_env ("PO_MAX_ITEM_PO");
	envVar.poMaxItemPo = (sptr == (char *) 0) ? 3 : atoi (sptr);

	/*----------------------
	| Show costing screen. |
	----------------------*/
	sptr = chk_env ("PO_COST_SCREEN");
	envVar.poCostScreen = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*------------------------------
	| Check if discounts reversed. |
	------------------------------*/
	sptr = chk_env ("SO_DISC_REV");
	envVar.soDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*---------------------------------------------
	| Convert input UOM to suppliers or Standard. |
	---------------------------------------------*/
	sptr = chk_env ("PO_CONV_SUP_UOM");
	envVar.poConvSupUom = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------
	| Allow zero costed items. |
	--------------------------*/
   	sptr = chk_env ("ALLOW_ZERO_COST");
	envVar.allowZeroCost = (sptr == (char *) 0) ? 0 : atoi(sptr);

	/*--------------------------------------
	| Use base OR Supplier UOM as default. |
	--------------------------------------*/
	sptr = chk_env ("PO_UOM_DEFAULT");
	envVar.poUomDefault = (sptr == (char *) 0) ? 1 : atoi (sptr);

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envVar.supOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envVar.supOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envVar.supOrdRound, "D");
			break;

		default:
			sprintf (envVar.supOrdRound, "B");
			break;
		}
	}

	/*---------------------------------------------------------
	| Purchase Order number is M(anual or S(ystem generated). |
	---------------------------------------------------------*/
	sptr = chk_env ("PO_INPUT");
	envVar.poInput = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;

	/*-------------------------------------------------------
	| Purchase Order number is Company or branch generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PO_NUM_GEN");
	envVar.poNumGen = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-------------------------------------------------------
	| Purchase Order Cost Calc. 1 if Avgerage can be used.  |
	-------------------------------------------------------*/
	sptr = chk_env ("PO_COST_CALC");
	envVar.poCostCalc = (sptr == (char *)0) ? 1 : atoi (sptr);

	/*-----------------------------------------------
	| Case Number Input PO_CASE_USED = 'Y' and 'N'. |
	-----------------------------------------------*/
	sptr = chk_env ("PO_CASE_USED");
	if (sptr == (char *)0)
		strcpy (envVar.poCaseUsed, "N");
	else
		sprintf (envVar.poCaseUsed, "%-1.1s", sptr);

	/*-----------------------------------
	| Get Purchase order print program. |
	-----------------------------------*/
	sptr = chk_env ("PO_PRINT");
	if (sptr == (char *)0)
		envVar.poPrint = FALSE;
	else
	{
		envVar.poPrint = TRUE;
		sprintf (poPrintProgram, "%-14.14s", sptr);
	}

	/*----------------------------------------------------------
	| Check inis record must exist, PO_INIS_REQ = 'Y' and 'N'. |
	----------------------------------------------------------*/
	sptr = chk_env ("PO_INIS_REQ");
	if (sptr == (char *)0)
		strcpy (envVar.poInisReq, "N");
	else
		sprintf (envVar.poInisReq, "%-1.1s", sptr);

	/*------------------------------------------------------
	| Check is class of item can be used using PO_RECEIPT. |
	------------------------------------------------------*/
	sptr = chk_env ("PO_REORDER");
	if (sptr == (char *)0)
		sprintf (envVar.poReorder,"%-26.26s","ABCD");
	else
		sprintf (envVar.poReorder,"%-26.26s", sptr);

	/*----------------------------------------------------
	| Check if Indent items are allowed to be purchased. |
	-----------------------------------------------------*/
	sptr = chk_env ("PO_INDENT_OK");
	envVar.poIndentOk = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*--------------------------------------------
	| Check for purchase order approval details. |
	--------------------------------------------*/
	sptr = chk_env ("PO_APP_FLAG");
	envVar.poAppFlag = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*------------------------------------------
	| Check for purchase order approval value. |
	------------------------------------------*/
	sptr = chk_env ("PO_APP_VAL");
	envVar.poAppVal = (sptr == (char *)0) ? (double) 0.00 : atof (sptr);

	/*-----------------------------------------------
	| Get local and foreign purchases order prefix. |
	-----------------------------------------------*/
	sptr = chk_env ("PO_LOCAL");
	if (sptr == (char *)0)
		strcpy (prefixLocPo, "LP");
	else
		sprintf (prefixLocPo, "%-15.15s", sptr);
	clip (prefixLocPo);

	sptr = chk_env ("PO_FOREIGN");
	if (sptr == (char *)0)
		strcpy (prefixFgnPo, "FP");
	else
		sprintf (prefixFgnPo, "%-15.15s", sptr);
	clip (prefixFgnPo);

	/*----------------------------
	| Check for 3pl Environment. |
	----------------------------*/
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVar.threePlSystem = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envVar.threePlSystem)
	{
		strcpy (envVar.poCaseUsed, "N");
		strcpy (envVar.poInisReq, "N");
	}
}


/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_ccn_inp.c,v 5.11 2002/11/28 04:09:49 scott Exp $
|  Program Name  : (so_ccn_inp.c)
|  Program Desc  : (Sales Order Customer Collection Note Entry)
|---------------------------------------------------------------------|
|  Date Written  : (22/03/89)      | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: so_ccn_inp.c,v $
| Revision 5.11  2002/11/28 04:09:49  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.10  2002/07/24 08:39:21  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.9  2002/06/26 07:45:16  kaarlo
| S/C #3993. Updated to display printed and not yet printed collection notes in SrchCohr.
|
| Revision 5.8  2002/06/20 07:15:57  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.7  2002/06/20 05:48:56  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2002/03/04 08:50:53  scott
| .
|
| Revision 5.5  2002/03/04 07:47:54  scott
| S/C 00814 INMR6-3-Display Collection Notes; the location window opens in second screen.  In a display program, location window should not be available.
|
| Revision 5.4  2001/08/28 06:14:20  scott
| Updated to change " ( to "(
|
| Revision 5.3  2001/08/09 09:20:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:51:02  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:53  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_ccn_inp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ccn_inp/so_ccn_inp.c,v 5.11 2002/11/28 04:09:49 scott Exp $";

#define	ENTER		13
#define	USE_WIN		1
#define MAXSCNS 	4
#define MAXWIDTH 	160
#define MAXLINES	1000
#define	DBOX_TOP	9
#define	DBOX_LFT	35
#define	DBOX_WID	66
#define	DBOX_ROW	4
#define	DBOX_DEP	2 + DBOX_ROW

#define	HEADER_SCN	1
#define	ITEM_SCN	2
#define	FREIGHT_SCN	3
#define	PROOF_SCN	4

#define	SUCCESS		0
#define	RETURN_ERR	1
#define	BUSY_ON		1
#define	BUSY_OFF	0

#define	SLEEP_TIME	2
#define	EXPORT		 (cohr_rec.ord_type [0] == 'E')
#define	BONUS		 (SR.bonusFlag [0] == 'Y')
#define	SERIAL_ITEM	 (inmr_rec.serial_item [0] == 'Y')
#define	SR			 store [line_cnt]
#define	LSR			 store [lcount [ITEM_SCN]]
#define	SERIAL		 (SR.serialFlag [0] == 'Y')
#define	KIT_ITEM	 (inmr_rec.inmr_class [0] == 'K')
#define	OLD_INSF	 (SR.oldSerial [0] == 'Y')
#define	NO_COST		 (SR.itemClass [0] == 'N')
#define	NON_STOCK(x) (store [x].itemClass [0] == 'Z')
#define	MULT_QTY	 (SR.costingFlag [0] != 'S')
#define	APP_INV		 (_hhco_hash != 0L)
#define	DOL_RET		 (coln_rec.crd_type [0] == 'D')
#define AUTO_SK_UP	 (createStatusFlag [0] == envVar.automaticStockUpdate [0])
#define LOT_OK		 (SR.lotFlag [0] == 'Y' && SR.creditType [0] == 'R' && sk_batch_cont)

#define	FREIGHT_CHG	 (cohr_rec.frei_req [0] == 'Y' && envVar.automaticFreight)

extern	int		_win_func;

#define	CCN_DISPLAY	 (prog_type == DISPLAY)
#define	CCN_MAINT	 (prog_type == MAINT)
#define	CCN_INPUT	 (prog_type == INPUT)

#define	RESTOCK_FEE	 (local_rec.restock_fee [0] == 'Y')

#define	FGN_CURR	 (envVar.dbMcurr && strcmp (cumr_rec.curr_code, envVar.currencyCode))

#define		BY_BRANCH	1
#define		BY_DEPART	2

#include <pslscr.h>
#include <getnum.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <ml_tr_mess.h>
#include <Costing.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#endif	/* GVISION */

#define	INPUT	0
#define	MAINT	1

	int		notax, 				/* charge gst & tax		*/
			newCCN, 				/* creating new credit		*/
			ins_flag, 			/* inserting line item		*/
			inp_disc, 	
			inp_sale, 
			inv_proof = 0, 		/* proof total ok		*/
			by_batch, 			/* require proof total		*/
			loaded = 0, 			/* use lcount for calc'ns	*/
			held_order = 0, 
			prog_type, 
			prmpt_type, 
			sk_batch_cont = FALSE;

	int		specialDisplay	=	FALSE;
	int		inScreenFlag;
	int		CCN_CONF		=	FALSE;

	double	lineTaxAmt 		= 0.00, 		/* line item gross for tax Amt  */
			lineGross 		= 0.00, 		/* line item gross				*/
			lineDisc 		= 0.00, 		/* line item discount			*/
			lineTax 		= 0.00, 		/* line item tax				*/
			lineRestock		= 0.00, 		/* line restocking fee 			*/
			lineGst 		= 0.00, 		/* line item gst				*/
			totalRestock 	= 0.00, 		/* restocking total 			*/
			totalCredit 	= 0.00, 		/* credit total					*/
			totalDisc 		= 0.00, 		/* discount total				*/
			totalTax 		= 0.00, 		/* tax total					*/
			totalGst 		= 0.00, 		/* gst total					*/
			totalProof 		= 0.00, 		/* credit proof total			*/
			totalBatch 		= 0.00, 		/* batch total					*/
			crdTotalAmt 	= 0.00, 		/* 								*/
		 	estimFreight	= 0.00;

	char	defaultBranchNo [3], 		/* branch number		*/
			createStatusFlag [2], 		/* create status for credit	*/
			type_flag [2], 				/* I (nvoice or C (redit Note	*/
			recordLockFlag [2], 
			bonusFlag [3], 
			hiddenFlag [3], 
			*currentUser;

	long	_hhco_hash;


#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct cucrRecord	cucr_rec;
struct cuccRecord	cucc_rec;
struct sobgRecord	sobg_rec;
struct cnchRecord	cnch_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct pocrRecord	pocr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct cudpRecord	cudp_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct exsiRecord	exsi_rec;
struct cudiRecord	cudi_rec;
struct cuinRecord	cuin_rec;
struct soktRecord	sokt_rec;
struct trshRecord	trsh_rec;
struct trchRecord	trch_rec;
struct trzmRecord	trzm_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;

	Money	*cumr_bo	=	&cumr_rec.bo_current;
	int		*cumr_inst	=	&cumr_rec.inst_fg1;

	char	*cumr2 	= 	"cumr2", 
			*inum2	=	"inum2", 
			*cohr2 	= 	"cohr2", 
			*data 	= 	"data", 
			*ser_space = "                         ";

/*
 * The structure envVar groups the values of environment settings together
 */
struct tagEnvVar
{
	char	automaticStockUpdate [2];
	char	currencyCode [4];
	char	gstCode [4];
	char	manualPrint [2];
	char	other [3] [31];
	char	normalOther [31];
	char	soSpecial [5];
	int		automaticFreight;
	int		dbNettUsed;
	int		dbCo;
	int		dbFind;
	int		dbMcurr;
	int		discountIndents;
	int		gstApplies;
	int		invoiceNumbers;
	int		reverseDiscount;
	int		useSystemDate;
	int		windowPopupOk;
	int		soFreightCharge;
	int		serialItemsOk;
} envVar;

	struct	storeRec	{
		long 	hhbrHash;			/* inmr_hhbr_hash					*/
		long 	hhsiHash;			/* inmr_hhsi_hash					*/
		long 	hhwhHash;			/* incc_hhwh_hash					*/
		long 	hhumHash;			/* coln_hhum_hash					*/
		long 	hhccHash;			/* incc_hhcc_hash					*/
		char 	lotFlag [2];		/* lot controlled item.				*/
		float 	qtyReturn;			/* local_rec.qtyReturn				*/
		float	gstPc;				/* inmr_gst_pc or 0.00 if notax		*/
		float	taxPc;				/* inmr_gst_pc or 0.00 if notax		*/
		double	taxAmount;			/* inmr_gst_amt 0.00 if notax		*/
		float	defaultDisc;		/* inmr_dis_pc						*/
		float	calcDisc;			/* calculated discount.				*/
		float	discPc;				/* coln_dis_pc						*/
		float	discA_Pc;			/* Discount percent A.				*/
		float	discB_Pc;			/* Discount percent B.				*/
		float	discC_Pc;			/* Discount percent C.				*/
		float	regPc;				/* Regulatory percent.				*/
		float	outerSize;			/* inmr_outer_size					*/
		double	salePrice;			/* coln_sale_price					*/
		double	calcSalePrice;		/* coln_sale_price					*/
		double	defaultPrice;		/* inmr_price [i]					*/
		double	actSalePrice;		/*                  				*/
		double	grossSalePrice;		/*                  				*/
		double	costPrice;			/* coln_cost_price					*/
		char	serialNo [26];		/* serial number for line			*/
		char	serialFlag [2];		/* Serial item.           			*/
		char	category [12];		/* category for line item			*/
		char	sellGroup [7];		/* inmr_sellgrp						*/
		char	bonusFlag [2];		/* bonus item ?						*/
		char	itemClass [2];		/* item's class for line			*/
		char	disOverride [2];	/* Discount overide.       			*/
		char	priOverride [2];	/* Price overide.       			*/
		char	packSize [6];		/* inmr_pack_size					*/
		double	extendedTotal;		/* sum of extended - disc + tax		*/
		char	costingFlag [2];	/* inmr_costing_flag				*/
		char	UOM [5];			/* inum_uom							*/
		char	LL [2];				/* LotLocation flag.				*/
        float   cnvFct;	
		double	weight;
		int		indent;
		char	cusOrdRef [21];
		char	creditType [2];
		char	oldSerial [2];		/* New line to be added to coln		*/
		int		cumulative;
		int		contractPrice;
		int		contractStat;		/* 0 = not contract line			*/
									/* 1 = contract no disc				*/
		int		pricingCheck;		/* Set if pricing has been  		*/
									/* called for line.         		*/
	} store [MAXLINES];

	int		DB_NETT = TRUE;

    typedef struct tagSTerms {
        char    *_scode;
        char    _sterm [32];
    } STerms;

    STerms s_terms [6];

	char	*scn_desc [] = {
		"HEADER SCREEN.", 
		"ITEM SCREEN.", 
		"FREIGHT SCREEN."
	};

	char	*progName;

/*
 * Local & Screen Structures 
 */
struct {
	long	dflt_inv_no;
	char	_date_raised [11];
	char	_date_required [11];
	char	dummy [11];
	char	inv_no [9];
	char	app_inv_no [9];
	char	cont_no [7];
	char	cont_desc [41];
	char	dflt_ord [2];
	char	dflt_batch [6];
	char	cust_no [7];
	char	ho_cust_no [7];
	char	credit_no [7];
	char	pri_desc [16];
	char	pri_fulldesc [16];
	char	ord_desc [10];
	char	ord_fulldesc [10];
	char	dbt_date [11];
	char	systemDate [11];
	long	lsystemDate;
	char	item_no [17];
	char	uom [5];
	char	sup_part [17];
	char	spinst [3] [61];
	char	sell_desc [31];
	float	qty_sup;
	float	qtyReturn;
	char	serial_no [26];
	char	prev_dbt_no [7];
	char	prev_inv_no [9];
	double	extend;
	char	dflt_sale_no [3];
	char	restock_fee [2];
	char	curr_code [6];
	char	QtyOrdPrmpt [11];
	char	QtyRetPrmpt [11];
	char	LL [2];
	char	headOfficeCustomer [7];
	char	chargeToCustomer [7];
	char	chargeToName [36];
	char	defaultDelZone [7];
} local_rec;            

static	struct	var	vars [] =
{
	{HEADER_SCN, LIN, "debtor", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "000000", "Customer Number     : ", " ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.cust_no}, 
	{HEADER_SCN, LIN, "curr_code", 	 4, 35, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.curr_code}, 
	{HEADER_SCN, LIN, "name", 	 4, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Customer Name       : ", " ", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{HEADER_SCN, LIN, "cus_addr1", 	 5, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Charge Address      : ", " ", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.ch_adr1}, 
	{HEADER_SCN, LIN, "cus_addr2", 	6, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------: ", " ", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.ch_adr2}, 
	{HEADER_SCN, LIN, "cus_addr3", 	7, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------: ", " ", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.ch_adr3}, 
	{HEADER_SCN, LIN, "cus_addr4", 	8, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------------: ", " ", 
		 NA, NO, JUSTLEFT, "", "", cumr_rec.ch_adr4}, 
	{HEADER_SCN, LIN, "headOfficeAccount", 	 10, 66, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Head Office Account : ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.ho_cust_no}, 
	{HEADER_SCN, LIN, "ccn_no", 	 5, 2, CHARTYPE, 
		"UNNNNNNN", "          ", 
		"0", "00000000", "Collection Number   : ", " ", 
		 NE, NO, JUSTRIGHT, "", "", local_rec.inv_no}, 
	{HEADER_SCN, LIN, "app_inv_no", 	 6, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "      ", "Invoice             : ", " Invoice to Apply Credit to ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.app_inv_no}, 
	{HEADER_SCN, LIN, "cus_ord_ref", 	 7, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Reason for Credit.  : ", "Search Available ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.cus_ord_ref}, 
	{HEADER_SCN, LIN, "restock_fee", 	 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", cumr_rec.restock_fee, "Re-Stocking Fee.    : ", "Enter Y (es) if re-stocking fee charged. Default = Customer master file default. ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.restock_fee}, 
	{HEADER_SCN, LIN, "dp_no", 	 10, 2, CHARTYPE, 
		"AA", "          ", 
		" ", cumr_rec.department, "Department Number   : ", " ", 
		 NA, NO, JUSTRIGHT, "", "", cohr_rec.dp_no}, 
	{HEADER_SCN, LIN, "dp_name", 	 10, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cudp_rec.dp_short}, 
	{HEADER_SCN, LIN, "cont_no", 	 11, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Contract Number     : ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts", 
		 NA, NO, JUSTLEFT, "", "", cohr_rec.cont_no}, 
	{HEADER_SCN, LIN, "cont_desc", 	 11, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cnch_rec.desc}, 
	{HEADER_SCN, LIN, "chargeToCustomer", 11, 66, CHARTYPE, 
		"UUUUUU", "          ", 

		" ", "0", "Refund To Customer  : ", "Enter Refund to Customer Number, [SEARCH]. ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.chargeToCustomer}, 
	{HEADER_SCN, LIN, "chargeToName", 	 11, 96, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.chargeToName}, 
	{HEADER_SCN, LIN, "chargeToHash", 11, 66, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "0", "", "", 
		 ND, NO, JUSTLEFT, "", "", (char *)&cohr_rec.chg_hhcu_hash}, 
	{HEADER_SCN, LIN, "batch_no", 	13, 2, CHARTYPE, 
		"UUUUU", "        ", 
		"0", local_rec.dflt_batch, "Batch number        : ", " ", 
		YES, NO, JUSTRIGHT, "", "", cohr_rec.batch_no}, 
	{HEADER_SCN, LIN, "ord_type", 	13, 66, CHARTYPE, 
		"U", "          ", 
		" ", local_rec.dflt_ord, "Credit Type.        : ", " D (omestic  E (xport ", 
		NI, NO, JUSTLEFT, "DE", "", local_rec.ord_desc}, 
	{HEADER_SCN, LIN, "ord_type_desc", 	13, 91, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.ord_fulldesc}, 
	{HEADER_SCN, LIN, "pri_type", 	14, 2, CHARTYPE, 
		"N", "        ", 
		" ", cumr_rec.price_type, "Price Type          : ", " ", 
		NI, NO, JUSTLEFT, "123456789", "", local_rec.pri_desc}, 
	{HEADER_SCN, LIN, "pri_type_desc", 	14, 27, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.pri_fulldesc}, 
	{HEADER_SCN, LIN, "date_raised", 	14, 66, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec._date_raised, "Credit date         : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&cohr_rec.date_raised}, 
	{HEADER_SCN, LIN, "fix_exch", 	14, 66, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Fixed Exchange Rate : ", " ", 
		ND, NO, JUSTLEFT, "YN", "", cohr_rec.fix_exch}, 
	{HEADER_SCN, LIN, "tax_code", 	15, 2, CHARTYPE, 
		"U", "        ", 
		" ", cumr_rec.tax_code, "Tax Code            : ", " ", 
		ND, NO, JUSTLEFT, "ABCD", "", cohr_rec.tax_code}, 
	{HEADER_SCN, LIN, "tax_no", 	15, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "        ", 
		" ", cumr_rec.tax_no, "Tax Number          : ", " ", 
		ND, NO, JUSTLEFT, "", "", cohr_rec.tax_no}, 
	{HEADER_SCN, LIN, "sale_code", 	16, 2, CHARTYPE, 
		"UU", "          ", 
		" ", local_rec.dflt_sale_no, "Salesman            : ", " ", 
		YES, NO, JUSTRIGHT, "", "", cohr_rec.sale_code}, 
	{HEADER_SCN, LIN, "sale_desc", 	16, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Salesman Name       : ", " ", 
		 NA, NO, JUSTLEFT, "", "", exsf_rec.salesman}, 
	{HEADER_SCN, LIN, "area_code", 	17, 2, CHARTYPE, 
		"UU", "          ", 
		" ", cumr_rec.area_code, "Area code           : ", " ", 
		YES, NO, JUSTRIGHT, "", "", cohr_rec.area_code}, 
	{HEADER_SCN, LIN, "area_desc", 	17, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Area Name           : ", " ", 
		 NA, NO, JUSTLEFT, "", "", exaf_rec.area}, 
	{HEADER_SCN, LIN, "grn_no", 		19, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Customer GRN No     : ", " ", 
		 YES, NO, JUSTLEFT, "", "", cohr_rec.grn_no}, 
	{HEADER_SCN, LIN, "prt_price", 	19, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Print Price Details : ", "Y (es) print prices on Credit, N (o) Don't print prices on Invoice.", 
		ND, NO, JUSTLEFT, "NY", "", cohr_rec.prt_price}, 
	{ITEM_SCN, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " Default Deletes Line ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{ITEM_SCN, TAB, "hide", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "H", " ", 
		 NA, NO, JUSTLEFT, "", "", coln_rec.hide_flag}, 
	{ITEM_SCN, TAB, "descr", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "    I t e m   D e s c r i p t i o n .   ", " ", 
		YES, NO, JUSTLEFT, "", "", coln_rec.item_desc}, 
	{ITEM_SCN, TAB, "sman_code", 	 0, 1, CHARTYPE, 
		"UU", "          ", 
		" ", cohr_rec.sale_code, "Sale", " Salesman ", 
		 ND, NO, JUSTRIGHT, "", "", coln_rec.sman_code}, 
	{ITEM_SCN, TAB, "pack_size", 	 0, 0, CHARTYPE, 
		"UUUUU", "          ", 
		" ", " ", "Pack ", " ", 
		 ND, NO, JUSTLEFT, "", "", coln_rec.pack_size}, 
	{ITEM_SCN, TAB, "crd_type", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", " ", " R(eturned, F(aulty, D(ollar, I(gnore ", 
		YES, NO, JUSTLEFT, "RFDI", "", coln_rec.crd_type}, 
	{ITEM_SCN, TAB, "uom", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", "UOM", " Enter Unit of Measure ", 
		YES, NO, JUSTLEFT, "", "", local_rec.uom}, 
	{ITEM_SCN, TAB, "qty_sup", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", local_rec.QtyOrdPrmpt, " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_sup}, 
	{ITEM_SCN, TAB, "qtyReturn", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "1.00", local_rec.QtyRetPrmpt, " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qtyReturn}, 
	{ITEM_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM_SCN, TAB, "sale_price", 	 0, 0, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Sal Price", "NOTE: For D (ollar) amount is total amount to be credited. ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&coln_rec.sale_price}, 
	{ITEM_SCN, TAB, "disc", 	 0, 0, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", " Disc  ", " ", 
		 NI, NO, JUSTRIGHT, "-999.99", "999.99", (char *)&coln_rec.disc_pc}, 
	{ITEM_SCN, TAB, "serialNo", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "      Serial Number      ", " ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.serial_no}, 
	{ITEM_SCN, TAB, "extend", 	 0, 0, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0.00", "  Extended  ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extend}, 
	{FREIGHT_SCN, LIN, "carrierCode", 	 4, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Carrier Code.       : ", "Enter carrier code, [SEARCH] available.", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_code}, 
	{FREIGHT_SCN, LIN, "carr_desc", 	 4, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Description : ", " ", 
		 NA, NO, JUSTLEFT, "", "", trcm_rec.carr_desc}, 
	{FREIGHT_SCN, LIN, "deliveryZoneCode", 	 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.defaultDelZone, "Delivery Zone       : ", "Enter Delivery Zone Code [SEARCH]. ", 
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone}, 
	{FREIGHT_SCN, LIN, "deliveryZoneDesc", 	 5, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Delivery Zone Desc  : ", " ", 
		NA, NO, JUSTLEFT, "", "", trzm_rec.desc}, 
	{FREIGHT_SCN, LIN, "deliveryRequired", 	 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Delivery Reqd. (Y/N) : ", "Enter Y (es) for Delivery. <default = N (o)> ", 
		 YES, NO, JUSTLEFT, "YN", "", cohr_rec.del_req}, 
	{FREIGHT_SCN, LIN, "deliveryDate", 	6, 66, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Delivery Date       : ", " ", 
		NA, NO, JUSTLEFT, " ", "", (char *)&cohr_rec.del_date}, 
	{FREIGHT_SCN, LIN, "cons_no", 	 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Consignment no.     : ", " ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.cons_no}, 
	{FREIGHT_SCN, LIN, "no_cartons", 	 7, 66, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Number Cartons.     : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&cohr_rec.no_cartons}, 
	{FREIGHT_SCN, LIN, "sos", 	 7, 98, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Small Ord.Surcharge : ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.sos}, 
	{FREIGHT_SCN, LIN, "est_freight", 	 9, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Est Freight         : ", " ", 
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&estimFreight}, 
	{FREIGHT_SCN, LIN, "tot_kg", 	 9, 66, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", "0.00", "Total Kgs.          : ", " ", 
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.no_kgs}, 
	{FREIGHT_SCN, LIN, "frei_req", 	 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", cumr_rec.freight_chg, "Freight Required.   : ", "Enter Y (es) if freight required Default = Customer master file default. ", 
		YES, NO, JUSTRIGHT, "YN", "", cohr_rec.frei_req}, 
	{FREIGHT_SCN, LIN, "freight", 	 10, 66, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amount.     : ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.freight}, 
	{FREIGHT_SCN, LIN, "shipname", 	12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dbt_name, "Ship to name        : ", " ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.dl_name}, 
	{FREIGHT_SCN, LIN, "shipaddr1", 	13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr1, "Ship to address 1   : ", " ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.dl_add1}, 
	{FREIGHT_SCN, LIN, "shipaddr2", 	14, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr2, "Ship to address 2   : ", " ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.dl_add2}, 
	{FREIGHT_SCN, LIN, "shipaddr3", 	15, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr3, "Ship to address 3   : ", " ", 
		YES, NO, JUSTLEFT, "", "", cohr_rec.dl_add3}, 
	{FREIGHT_SCN, LIN, "ship_method", 	17, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Shipment method     : ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.spinst [0]}, 
	{FREIGHT_SCN, LIN, "spcode1", 	18, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Instruction 1       : ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.spinst [1]}, 
	{FREIGHT_SCN, LIN, "spcode2", 	19, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Instruction 2       : ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.spinst [2]}, 
	{PROOF_SCN, LIN, "proof", 	 4, 20, MONEYTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "", "Proof Total", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&totalProof}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<p_terms.h>
#include	<cus_price.h>
#include	<cus_disc.h>
#include	<neg_win.h>
#include	<FindCumr.h>
#include	<LocHeader.h>

/*
 * Function Declarations 
 */
void 	InitML 					(void);
void 	ignore 					(void);
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReadMisc 				(void);
int  	spec_valid 				(int);
void 	ProcessPhantomItem 		(long, float);
void 	tab_other 				(int);
void 	CreditAll 				(void);
void 	PrintExtend 			(int);
int  	ValidateItemNumber 		(int);
void 	PriceProcess 			(void);
void 	DiscProcess 			(void);
int  	CheckIncc 				(void);
int  	DeleteLine 				(int);
void 	SetCreditNote 			(int);
int  	LoadCreditNote 			(long);
int  	LoadExistingInvoice 	(long, char *);
void 	Update 					(void);
int  	CheckForCredit 			(char *);
int  	CheckForCCN 			(void);
void 	CalculateExtendedTotal 	(int);
void 	CalculateLineTotal 		(int);
void 	FreeInsf 				(int, char *);
int		SrchCudi 				(int);
void 	SrchPaymentTerms 		(void);
void 	SrchSellTerms 			(void);
void 	SrchPrice 				(void);
void 	SrchExsf 				(char *);
void 	SrchExaf 				(char *);
void 	SrchInum 				(char *);
void 	SrchCohr 				(char *);
void 	SrchCohr2 				(char *);
void 	SrchCucr 				(void);
void 	SrchTrzm 				(char *);
void 	SrchTrcm 				(char *);
int  	CheckDuplicateInsf 		(char *, long, int);
int  	CheckHiddenLine 		(char *);
int  	CheckBonusLine 			(char *);
int  	AddIncc 				(void);
void 	BusyFunction 			(int);
void 	PrintCompanyDetails 	(void);
void 	CalculateInputTotal 	(void);
void 	DrawTotals 				(void);
void 	PrintTotalBoxValues 	(void);
int  	AddInsf 				(long, long, char *, long);
void 	RemoveInsf 				(long, char *);
int  	win_function2 			(int, int, int, int);
int  	LoadDisplay 			(char *);
char 	*GetPostingStatus 		(char *);
int  	use_window 				(int);
int  	FindCucc 				(int, long);
void 	AddSobg 				(int, char *, long);
void 	CheckEnvironment 		(void);
float 	ScreenDisc 				(float);
float 	ToStdUom 				(float);
float 	ToLclUom 				(float);
int  	NotAllAlloc 			(void);
void 	CalculateFreight 		(float, double, double, double);
void 	OpenTransportFiles 		(char *);
void 	CloseTransportFiles 	(void);
int  	heading 				(int);
char 	*NewStrStr 				(char *);
void 	SetSTermCode 			(int, char *);
void 	SetSTerm 				(int, char *, char *);
void 	InitSTerms 				(void);
void 	DeleteSTerms 			(void);
char	*GetPriceDesc 			(int);

/*
 * Main Processing Routine
 */
int
main (
 int	argc, 
 char	*argv [])
{
	int		lpno = 0;
	int		i;
	int		first_time = 1;
	int		field;

	TruePosition	=	TRUE;

	_win_func = TRUE;

	currentUser = getenv ("LOGNAME");

	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();

	SETUP_SCR (vars);


	progName = strrchr (argv [0], '/');
	if (progName)
		progName++;
	else
		progName = argv [0];

	if (!strcmp (progName, "so_ccn_conf"))
		CCN_CONF = TRUE;

	if (!strcmp (progName, "so_ccn_inp") || CCN_CONF)
	{
		prog_type = INPUT;
		strcpy (recordLockFlag, "u");
	}
	else
	{
		if (!strcmp (progName, "so_ccn_dsp") ||
			!strcmp (progName, "so_ccn_sdsp"))
		{
			if (!strcmp (progName, "so_ccn_sdsp"))
				specialDisplay = TRUE;
			else
				specialDisplay = FALSE;

			prog_type = DISPLAY;
			strcpy (recordLockFlag, "r");
		}
		else
		{
			prog_type = MAINT;
			strcpy (recordLockFlag, "u");
		}
	}
	if (argc < 4)
	{
		print_at (0, 0, mlSoMess742 , argv [0]);
		print_at (1, 0, mlSoMess743);
		print_at (2, 0, mlSoMess744);

		return (EXIT_FAILURE);
	}

	switch (argv [1] [0])
	{
	case	'Y':
	case	'y':
		by_batch = 1;
		break;

	case	'N':
	case	'n':
		by_batch = 0;
		break;

	default:
		print_at (1, 0, mlSoMess743);
		print_at (2, 0, mlSoMess744);

		return (EXIT_FAILURE);
	}
	sprintf (createStatusFlag, "%1.1s", argv [2]);

	tab_row = 8;

	lpno = (argc == 5) ? atoi (argv [4]) : 0;


	strcpy (local_rec.dflt_ord, "D");
	strcpy (local_rec.dflt_batch, "00000");

	/*
	 * open main database files.
	 */
	OpenDB ();

	if (specialDisplay && argc < 6)
		specialDisplay = FALSE;

	/*
	 * Open Output to Counter Printer. 
	 */
	if (lpno) 
		ip_open (comm_rec.co_no, comm_rec.est_no, lpno);
	init_scr ();
	set_tty (); 
	sprintf (local_rec.QtyOrdPrmpt, "%-10.10s", (CCN_CONF) 	? " Qty Req. "
															: " Inv Qty. ");
	sprintf (local_rec.QtyRetPrmpt, "%-10.10s", (CCN_CONF) 	? " Qty Ret. "
															: " Qty Ret. ");
	_set_masks (argv [3]);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM_SCN, store, sizeof (struct storeRec));
#endif
	if (envVar.gstApplies)
	{
		FLD ("tax_no")   = ND;
		FLD ("tax_code") = ND;
	}

	if (envVar.serialItemsOk)
	{
		FLD ("serialNo")	=	(envVar.serialItemsOk) ? YES : ND;
		FLD ("extend")		=	(envVar.serialItemsOk) ? ND  : NA;
	}

	InitML ();

	if (CCN_DISPLAY)
	{
		for (field = label ("app_inv_no");FIELD.scn != 0;field++)
		{
			if (FIELD.required != ND)
				FIELD.required = NA;
		}
	}

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	inp_disc = 0;
	inp_sale = 0;

	inp_sale = vars [label ("sale_price")].required;
	inp_disc = vars [label ("disc")].required;
	
	no_edit (PROOF_SCN);

	strcpy (local_rec.dbt_date, DateToString (comm_rec.dbt_date));

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	if (envVar.useSystemDate)
		strcpy (local_rec._date_raised, local_rec.systemDate);
	else
		strcpy (local_rec._date_raised, local_rec.dbt_date);

	strcpy (local_rec._date_required, local_rec.systemDate);
	local_rec.lsystemDate = StringToDate (local_rec.systemDate);
	local_rec.dflt_inv_no = 0L;


	strcpy (defaultBranchNo, (envVar.dbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	FLD  ("LL") = ND;
	if  (SK_BATCH_CONT || MULT_LOC)
		FLD  ("LL") = (CCN_DISPLAY) ? NA : YES;

	if (CCN_INPUT && !CCN_CONF)
		FLD  ("LL") = ND;

	/*
	 * Open Pricing and Discounting files. 
	 */
	OpenPrice ();
	OpenDisc ();

	strcpy (local_rec.prev_inv_no, "00000000");
	strcpy (local_rec.prev_dbt_no, "000000");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		vars [label ("item_no")].row = MAXLINES;
		FLD ("item_no") = NI;
		notax = 0;
		lcount [ITEM_SCN] = 0;
		ins_flag	= 0;
		totalCredit	= 0.00;
		totalDisc 	= 0.00;
		totalTax 	= 0.00;
		totalGst 	= 0.00;
		if (restart) 
		{
			if (first_time && CCN_INPUT)
			{
				strcpy (local_rec.dflt_batch, "00000");
				if (envVar.useSystemDate)
					strcpy (local_rec._date_raised, local_rec.systemDate);
				else
					strcpy (local_rec._date_raised, local_rec.dbt_date);

				strcpy (local_rec._date_required, local_rec.systemDate);
				strcpy (local_rec.dflt_ord, "D");
			}
		}

		if (restart)
		{
			/*
			 * Reset any outstanding insfRecords
			 */
			for (i = 0; i < MAXLINES; i++)
			{
				if (strcmp (store [i].serialNo, ser_space))
				{
					/*
					 * If the coln previously existed the reset the insf
					 * status to sold else remove the insf that has been
					 * created.                                         
					 */
 					if (store [i].oldSerial [0] == 'Y')
					{
						cc	=	UpdateInsf 
								(
									store [i].hhwhHash, 
									0L, 
									store [i].serialNo, 
									"C", 
									"S"
								);
						if (cc && cc < 1000)
							file_err (cc, insf, "DBUPDATE");
					}
					else
					{
						RemoveInsf (store [i].hhwhHash, store [i].serialNo);
					}
				}
			}
		}
		for (i = 0; i < MAXLINES; i++)
		{
			store [i].hhbrHash		=	0L;
			store [i].hhsiHash		=	0L;
			store [i].hhwhHash		=	0L;
			store [i].hhccHash		=	0L;
			store [i].qtyReturn			=	0.00;
			store [i].gstPc			=	0.00;
			store [i].taxPc			=	0.00;
			store [i].taxAmount			=	0.00;
			store [i].defaultDisc		=	0.00;
			store [i].calcDisc		=	0.00;
			store [i].discPc			=	0.00;
			store [i].discA_Pc			=	0.00;
			store [i].discB_Pc			=	0.00;
			store [i].discC_Pc			=	0.00;
			store [i].regPc			=	0.00;
			store [i].outerSize			=	0.00;
			store [i].salePrice		=	0.00;
			store [i].calcSalePrice		=	0.00;
			store [i].defaultPrice		=	0.00;
			store [i].actSalePrice		=	0.00;
			store [i].grossSalePrice		=	0.00;
			store [i].costPrice		=	0.00;
			store [i].extendedTotal		=	0.00;
			store [i].weight			=	0.00;
			store [i].indent			=	0;
			store [i].contractPrice		=	0;
			store [i].contractStat		=	0;
			store [i].pricingCheck		=	0;
			strcpy (store [i].serialNo, 	ser_space);
			strcpy (store [i].category, 	"           ");
			strcpy (store [i].sellGroup, 	"      ");
			strcpy (store [i].bonusFlag, 	" ");
			strcpy (store [i].itemClass, 		" ");
			strcpy (store [i].disOverride, 		"N");
			strcpy (store [i].priOverride, 		"N");
			strcpy (store [i].packSize, 	"     ");
			strcpy (store [i].costingFlag, " ");
			strcpy (store [i].cusOrdRef, "                    ");
			strcpy (store [i].creditType, 	 " ");
			strcpy (store [i].oldSerial, 	 "N");
			strcpy (store [i].lotFlag, 	 "N");
			strcpy (store [i].UOM, 		 "    ");
			strcpy (store [i].LL, 		 "N");
		}
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		loaded = 0;
		init_vars (HEADER_SCN);	
		
		init_ok = TRUE;
		eoi_ok = FALSE;

		if (!specialDisplay)
		{
			heading (HEADER_SCN);
			entry (HEADER_SCN);
			if (prog_exit || restart)
				continue;
		}
		else
		{
			if (LoadDisplay (argv [5]))
			{
				clear ();
				box (0, 0, 36, 1);
				print_at (1, 1, "%R %s ", ML (mlStdMess016));
		
				sleep (sleepTime);
				shutdown_prog ();
                return (EXIT_FAILURE);
			}
			heading (HEADER_SCN);
			scn_display (HEADER_SCN);
		}

		if  ((newCCN && APP_INV) || CCN_CONF)
		{
			i = 'Y';
			if (!CCN_MAINT)
			{
				if (CCN_CONF)
					i = prmptmsg (ML (mlSoMess139), "YyNn", 1, 2);
				else
					i = prmptmsg (ML (mlSoMess140), "YyNn", 1, 2);
				print_at (2, 1, "                                                ");
				if (i == 'Y' || i == 'y')
					CreditAll ();
			}
		}
		else
		{
			i = 'N';
			vars [label ("item_no")].row = MAXLINES;
		}

		if ((newCCN || CCN_CONF) && i != 'Y' && i != 'y')
		{
			heading (ITEM_SCN);
			if (!APP_INV && !CCN_CONF)
				vars [scn_start].required = YES;
			else
				scn_display (ITEM_SCN);

			if (APP_INV || CCN_CONF)
			{
				init_ok = FALSE;
				eoi_ok = FALSE;
			}
			entry (ITEM_SCN);

			vars [scn_start].required = NI;
			
			if (restart)
				continue;
		}
		else
			scn_display (HEADER_SCN);

		prog_status = ! (ENTRY);

		FLD ("item_no") = (CCN_DISPLAY) ? NA : YES;

		loaded = 1;
		ignore ();

		if (CCN_CONF)
		{
			while (TRUE)
			{
				edit_all ();
	
				if (!CCN_DISPLAY && !restart && NotAllAlloc ())
					continue;

				break;
			}
		}
		else
			edit_all ();

		if (specialDisplay)
			break;

		if (restart || CCN_DISPLAY)
			continue;

		if (by_batch)
		{
			CalculateExtendedTotal (TRUE);
			heading (PROOF_SCN);
			entry (PROOF_SCN);

			while (inv_proof)
			{
				edit_all ();
				if (restart)
					break;

				heading (PROOF_SCN);
				scn_display (PROOF_SCN);
				entry (PROOF_SCN);
				if (restart)
					break;
			}
		}
		else
			inv_proof = 0;

		if (!CCN_DISPLAY)
		{
			if (inv_proof == 0 && !restart) 
			{
				Update ();

				/*
				 * Open Output to Counter Printer.
				 */
				if (lpno)
					ip_print (cohr_rec.hhco_hash);

				FLD ("batch_no") 		= NI;
				FLD ("date_raised") 	= NI;
				FLD ("ord_type") 		= NI;
				strcpy (local_rec.dflt_batch, cohr_rec.batch_no);
				strcpy (local_rec.dflt_ord, cohr_rec.ord_type);
				strcpy (local_rec._date_raised, DateToString (cohr_rec.date_raised));
				first_time = 0;
			}
		}
	}
	/*
	 * Open Output to Counter Printer. 
	 */
	if (lpno)
		ip_close ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}   

void
InitML (
 void)
{
    InitSTerms ();
	SetSTermCode (0, ML ("Local                 "));
	SetSTermCode (1, ML ("Cost Insurance Freight"));
	SetSTermCode (2, ML ("Cost & Freight"));
	SetSTermCode (3, ML ("Free Into Store"));
	SetSTermCode (4, ML ("Free On Board"));
}

/*
 * Routine to delete/ignore lines.
 */
void
ignore (
 void)
{
	int		_show = (cur_screen == ITEM_SCN);

	scn_set (ITEM_SCN);
	prog_status = !ENTRY;
	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt);
		if (coln_rec.crd_type [0] == 'I')
		{
			DeleteLine (_show);
			line_cnt--;
		}
	}
	scn_set (HEADER_SCN);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (
 void)
{
	clear ();
	if (by_batch)	
	{
		print_at (0, 0, ML (mlSoMess228), DOLLARS (totalBatch));

		if (envVar.dbMcurr)
			print_at (0, 0, "(%-3.3s) ", envVar.currencyCode);

		PauseForKey (1, 0, ML (mlStdMess042), 0);
	}
	CloseDB (); 
	FinishProgram ();
    DeleteSTerms ();
}

/*
 * Open Database Files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    ReadMisc ();

	abc_alias (cohr2, cohr);

	abc_alias (cumr2, cumr);
	abc_alias (inum2, inum);
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVar.dbFind) ? "cumr_id_no" 
							   : "cumr_id_no3");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	OpenInsf ();
	OpenLocation (ccmr_rec.hhcc_hash);
	IgnoreTotal	=	TRUE;
}

/*
 * Close Database Files
 */
void
CloseDB (void)
{
	abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (cohr);
	abc_fclose (cohr2);
	abc_fclose (coln);
	abc_fclose (esmr);
	abc_fclose (cudp);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_fclose (cnch);
	abc_fclose (cucc);
	abc_fclose (cuit);
	CloseLocation ();
	CloseCosting ();

	/*
	 * Close pricing and discounting files. 
	 */
	ClosePrice ();
	CloseDisc ();
	SearchFindClose ();
	abc_dbclose (data);
}

/*
 * Get common info from commom database file.
 */
void
ReadMisc (void)
{

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (comr);
	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	int		i = 0;
	int		this_page;
	char	*sptr;
	int		val_pterms = FALSE;
	int		TempLine;

	/*
	 * Validate Customer Number.
	 */
	if (LCHECK ("debtor"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, defaultBranchNo);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cust_no, 6));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
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

		use_window (FN14);
		strcpy (local_rec.dflt_sale_no, (!strcmp (ccmr_rec.sman_no, "  ")) ? cumr_rec.sman_code : ccmr_rec.sman_no);

		strcpy (local_rec.restock_fee, cumr_rec.restock_fee);
		
		if (RESTOCK_FEE)
			sprintf (envVar.other [2], "%.30s", "Re-stocking Fee.");
		else
			sprintf (envVar.other [2], "%.30s", envVar.normalOther);

		/*
		 * Check customers currency code.
		 */
		if (envVar.dbMcurr)
		{
			open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				abc_fclose (pocr);
				return (RETURN_ERR);
			}
			sprintf (local_rec.curr_code, "(%-3.3s)", cumr_rec.curr_code);	
			DSP_FLD ("curr_code");
			abc_fclose (pocr);
		}
		else
			pocr_rec.ex1_factor = 1.00;

		return (SUCCESS);
	}
	/*
	 * Validate Charge To Number.
	 */
	if (LCHECK ("chargeToCustomer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.chargeToCustomer, "      ");
			strcpy (local_rec.chargeToName, " ");
			cohr_rec.chg_hhcu_hash	=	0L;
			DSP_FLD ("chargeToCustomer");
			DSP_FLD ("chargeToName");
			return (SUCCESS);
		}
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, defaultBranchNo, temp_str);
			cohr_rec.chg_hhcu_hash	=	0L;
			return (SUCCESS);
		}

		strcpy (cumr2_rec.co_no, comm_rec.co_no);
		strcpy (cumr2_rec.est_no, defaultBranchNo);
		strcpy (cumr2_rec.dbt_no, zero_pad (local_rec.chargeToCustomer, 6));
		cc = find_rec (cumr, &cumr2_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Customer not found. 
			 */
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		cohr_rec.chg_hhcu_hash	=	cumr2_rec.hhcu_hash;
		strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
		sprintf (local_rec.chargeToName, "%-35.35s", cumr2_rec.dbt_name);
		DSP_FLD ("chargeToCustomer");
		DSP_FLD ("chargeToName");
		return (SUCCESS);
	}
		
	if (LCHECK ("ccn_no"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (local_rec.inv_no, "00000000");
			DSP_FLD ("ccn_no");
		}

		if (dflt_used && !CCN_CONF)
		{
			sprintf (local_rec.inv_no, "%08ld", local_rec.dflt_inv_no);
			DSP_FLD ("ccn_no");
		}

		/*
		 * Maintaining Sales Orders
		 */
		if (CCN_CONF || 
			CCN_DISPLAY || 
			SRCH_KEY || 
			 (strcmp (local_rec.inv_no, "00000000") && 
			 strcmp (local_rec.inv_no, "        ")))
		{
			if (SRCH_KEY)
			{
				SrchCohr (temp_str);
				return (SUCCESS);
			}

			/*
			 * Check if credit is on file. 
			 */
			strcpy (cohr_rec.co_no, comm_rec.co_no);
			strcpy (cohr_rec.br_no, comm_rec.est_no);
			strcpy (cohr_rec.type, "N");
			strcpy (cohr_rec.inv_no, zero_pad (local_rec.inv_no, 8));

			cc = find_rec (cohr, &cohr_rec, COMPARISON, recordLockFlag);
			if (cc)
			{
				if (CCN_DISPLAY || CCN_CONF)
				{
					print_mess (ML (mlSoMess229));
					sleep (sleepTime);
					clear_mess ();
					return (RETURN_ERR); 
				}
				newCCN = 1;
				SetCreditNote (newCCN);
				return (SUCCESS);
			}

			if (cumr_rec.hhcu_hash != cohr_rec.hhcu_hash)
			{
				sprintf (err_str, ML (mlSoMess230), local_rec.inv_no);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR); 
			}

			if (CCN_INPUT && 
				cohr_rec.stat_flag [0] != createStatusFlag [0])
			{
				sprintf (err_str, ML (mlSoMess231), local_rec.inv_no);
				abc_unlock (cohr);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR); 
			}

			strcpy (local_rec.spinst [0], cohr_rec.din_1);
			strcpy (local_rec.spinst [1], cohr_rec.din_2);
			strcpy (local_rec.spinst [2], cohr_rec.din_3);
			newCCN = 0;
			_hhco_hash = 0L;

			if (LoadCreditNote (cohr_rec.hhco_hash))
				return (RETURN_ERR);

			if (lcount [ITEM_SCN] == 0)
			{
				print_mess (ML (mlSoMess047));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}

			entry_exit = 1;
		}
		else
			newCCN = 1;

		SetCreditNote (newCCN);
		return (SUCCESS);
	}

	/*
	 * Validate Applied to invoice.
	 */
	if (LCHECK ("app_inv_no"))
	{
		if (SRCH_KEY)
		{
			SrchCohr2 (temp_str);
			return (SUCCESS);
		}

		if (!strcmp (local_rec.app_inv_no, "00000000") || 
		    !strcmp (local_rec.app_inv_no, "        "))
		{
			_hhco_hash = 0L;
			return (SUCCESS);
		}

		/*
		 * Check if Invoice has already been credited. 
		 */
		abc_selfield (cohr2, "cohr_app_inv_no");

		strcpy (local_rec.app_inv_no, zero_pad (local_rec.app_inv_no, 8));
		strcpy (cohr2_rec.app_inv_no, local_rec.app_inv_no);
		cc = find_rec (cohr2, &cohr2_rec, GTEQ, "r");
		while (!cc && !strcmp (cohr2_rec.app_inv_no, local_rec.app_inv_no))
		{
			if (!strcmp (cohr2_rec.co_no, comm_rec.co_no)  &&
			     !strcmp (cohr2_rec.br_no, comm_rec.est_no) &&
			     cohr2_rec.type [0] == 'N')
			{
				sprintf (err_str, ML (mlSoMess232) , local_rec.app_inv_no, cohr2_rec.inv_no);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				abc_selfield (cohr2, "cohr_id_no2");
				return (RETURN_ERR);
			}
			cc = find_rec (cohr2, &cohr2_rec, NEXT, "r");
		}
		
		/*
		 * Check if invoice is on file. 
		 */
		abc_selfield (cohr2, "cohr_id_no2");

		strcpy (cohr2_rec.co_no, comm_rec.co_no);
		strcpy (cohr2_rec.br_no, comm_rec.est_no);
		strcpy (cohr2_rec.type, "I");
		strcpy (cohr2_rec.inv_no, zero_pad (local_rec.app_inv_no, 8));

		cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess115));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		/*
		 * Check that Customer is the same.
		 */
		if (cumr_rec.hhcu_hash != cohr2_rec.hhcu_hash)
		{
			print_mess (ML (mlSoMess075));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR); 
		}

		open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.type, "1");
		strcpy (cuin_rec.inv_no, local_rec.app_inv_no);

		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
		{
			cuin_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
			cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*
			 * Allow processing if the cuin has been right thru debtors
			 */
			if (cohr2_rec.stat_flag [0] != '9' && cohr2_rec.stat_flag [0] != 'D')
			{
				/*
				 * Check that Customer is the same. 
				 */
				print_mess (ML (mlSoMess042));
				sleep (sleepTime);
				clear_mess ();
				abc_fclose (cuin);
				return (RETURN_ERR); 
			}
		}
		abc_fclose (cuin);

		_hhco_hash = cohr2_rec.hhco_hash;

		/*
		 * Load existing invoice details. 
		 */
		if (LoadExistingInvoice (_hhco_hash, cohr2_rec.cont_no))
			return (RETURN_ERR);

		/*
		 * Invoice Has no lines.
		 */
		if (lcount [ITEM_SCN] == 0)
		{
			print_mess (ML (mlSoMess048));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		sprintf (cohr2_rec.cus_ord_ref, "%-20.20s", " ");
		strcpy (local_rec.dflt_sale_no, cohr2_rec.sale_code);
		strcpy (cohr_rec.sale_code, cohr2_rec.sale_code);
		strcpy (local_rec.cont_no, cohr2_rec.cont_no);
		if (strcmp (cohr2_rec.cont_no, "      ") == 0)
			sprintf (local_rec.cont_desc, "%40.40s", " ");
		else
			sprintf (local_rec.cont_desc, "Invoiced under contract %6.6s", 
										  cohr2_rec.cont_no);
		DSP_FLD ("cont_desc");
		return (SUCCESS);
	}

	/*
	 * Validate Batch Number.
	 */
	if (LCHECK ("batch_no"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (cohr_rec.batch_no, local_rec.dflt_batch);

		DSP_FLD ("batch_no");
		return (SUCCESS);
	}

	/*
	 * Validate Customer Order Ref. 
	 */
	if (LCHECK ("cus_ord_ref"))
	{
		open_rec (cucr, cucr_list, CUCR_NO_FIELDS, "cucr_id_no");
		if (SRCH_KEY)
		{
			SrchCucr ();
			abc_fclose (cucr);
			return (SUCCESS);
		}
		strcpy (cucr_rec.co_no, comm_rec.co_no);
		sprintf (cucr_rec.code, "%-1.1s", cohr_rec.cus_ord_ref);
		cc = find_rec (cucr, &cucr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (cohr_rec.cus_ord_ref, "%-20.20s", cucr_rec.desc);
			DSP_FLD ("cus_ord_ref");
		}
		strcpy (err_str, cohr_rec.cus_ord_ref);
		sptr = clip (err_str);
		if (strlen (sptr) == 0)
		{
			print_mess (ML (mlSoMess043));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (cucr);
			return (RETURN_ERR);
		}
		abc_fclose (cucr);
		return (SUCCESS);
	}

	/*
	 * Validate department Number and allow search. 
	 */
	if (LCHECK ("dp_no"))
	{
		if (FLD ("dp_no") == NA)
			strcpy (cohr_rec.dp_no, cumr_rec.department);

		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (cohr_rec.dp_no, cumr_rec.department);

		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR); 
		}

		PrintCompanyDetails ();
		DSP_FLD ("dp_no");
		DSP_FLD ("dp_name");
		return (SUCCESS);
	}

	if (LCHECK ("del_name") || LNCHECK ("del_addr", 8))
	{
		if (F_NOKEY (field))
			return (SUCCESS);

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
				return (SUCCESS);

			strcpy (cohr_rec.dl_name, cudi_rec.name);
			strcpy (cohr_rec.dl_add1, cudi_rec.adr1);
			strcpy (cohr_rec.dl_add2, cudi_rec.adr2);
			strcpy (cohr_rec.dl_add3, cudi_rec.adr3);
		}
		skip_entry = goto_field (field, label ("batch_no"));
	}

	/*
	 * Validate Payment Terms. 
	 */
	if (LCHECK ("pay_term"))
	{
		val_pterms = FALSE;

		if (SRCH_KEY)
		{
			SrchPaymentTerms ();
			return (SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms, p_terms [i]._pcode, strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms, "%-40.40s", p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		DSP_FLD ("pay_term");

		return (SUCCESS);
	}

	/*
	 * Validate Tax Code.
	 */
	if (LCHECK ("tax_code"))
	{
		if (F_HIDE (label ("tax_code")))
		{
			strcpy (cohr_rec.tax_code, cumr_rec.tax_code);
			if (cohr_rec.tax_code [0] == 'A' || 
			     cohr_rec.tax_code [0] == 'B')
				notax = 1;
			else
				notax = 0;
			return (SUCCESS);
		}

		if (cohr_rec.tax_code [0] == 'C' || 
		     cohr_rec.tax_code [0] == 'D')
			FLD ("tax_no") = NO;
		else
			FLD ("tax_no") = YES;

		if (cohr_rec.tax_code [0] == 'A' || 
		     cohr_rec.tax_code [0] == 'B')
			notax = 1;
		else
			notax = 0;

		DSP_FLD ("tax_no");
		return (SUCCESS);
	}
	/*
	 * Validate Tax No. 
	 */
	if (LCHECK ("tax_no"))
	{
		if (F_HIDE (label ("tax_no")))
		{
			strcpy (cohr_rec.tax_no, cumr_rec.tax_no);
			return (SUCCESS);
		}

		if (!strcmp (cohr_rec.tax_no, "               ") && 
		     FLD ("tax_no") == YES)
		{
			print_mess (ML (mlStdMess200));
			sleep (sleepTime);
			clear_mess ();

			return (RETURN_ERR);
		}
		return (SUCCESS);
	}

	/*
	 * Validate Selling Terms.
	 */
	if (LCHECK ("sell_terms"))
	{
		if (SRCH_KEY)
		{
			SrchSellTerms ();
			return (SUCCESS);
		}

		for (i = 0;strlen (s_terms [i]._scode);i++)
		{
			if (!strncmp (cohr_rec.sell_terms, s_terms [i]._scode, strlen (s_terms [i]._scode)))
			{
				sprintf (local_rec.sell_desc, "%-30.30s", s_terms [i]._sterm);
				break;
			}
		}

		if (!strlen (s_terms [i]._scode))
		{
			print_mess (ML (mlSoMess246));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
			
		DSP_FLD ("sel_terms");
		DSP_FLD ("sell_desc");

		return (SUCCESS);
	}

	/*
	 * Validate Processed Date.
	 */
	if (LCHECK ("date_raised"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			cohr_rec.date_raised = StringToDate (local_rec._date_raised);

		if (chq_date (cohr_rec.date_raised, comm_rec.dbt_date))
			return (RETURN_ERR);

		return (SUCCESS);
	}

	/*
	 * Validate Price Type.
	 */
	if (LCHECK ("pri_type"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (local_rec.pri_desc, cumr_rec.price_type);

		if (SRCH_KEY)
		{
			SrchPrice ();
			strcpy (local_rec.pri_desc, "               ");
			return (SUCCESS);
		}
		sprintf (cohr_rec.pri_type, "%-1.1s", local_rec.pri_desc);
		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type) - 1));
		DSP_FLD ("pri_type");
		DSP_FLD ("pri_type_desc");
		return (SUCCESS);
	}

	/*
	 * Validate Order Type.
	 */
	if (LCHECK ("ord_type"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (local_rec.ord_desc, local_rec.dflt_ord);

		strcpy (local_rec.ord_fulldesc, (local_rec.ord_desc [0] == 'D') ? "Domestic" : "Export  ");
		sprintf (cohr_rec.ord_type, "%-1.1s", local_rec.ord_desc);
		DSP_FLD ("ord_type");
		DSP_FLD ("ord_type_desc");
	}

	/*
	 * Validate Price Type.
	 */
	if (LCHECK ("prt_price"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.prt_price, "Y");
			return (SUCCESS);
		}
		return (SUCCESS);
	}

	/*
	 * Validate Salesman Code. 
	 */
	if (LCHECK ("sale_code"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.sale_code, local_rec.dflt_sale_no);
			DSP_FLD ("sale_code");
			DSP_FLD ("sale_desc");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, cohr_rec.sale_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		DSP_FLD ("sale_desc");
		return (SUCCESS);
	}

	/*
	 * Validate Area Code. 
	 */
	if (LCHECK ("area_code"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (cohr_rec.area_code, cumr_rec.area_code);
			DSP_FLD ("area_code");
			DSP_FLD ("area_desc");
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (SUCCESS);
		}
		strcpy (exaf_rec.co_no, comm_rec.co_no);
		strcpy (exaf_rec.area_code, cohr_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		DSP_FLD ("area_desc");
		return (SUCCESS);
	}

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
	{
		if (CCN_DISPLAY)
			return (SUCCESS);

		if ((APP_INV && edit_status == FALSE) || CCN_CONF) 
		{
			if (prog_status == ENTRY)
			{
				getval (line_cnt);
				if (CCN_CONF)
					skip_entry = goto_field (field, label ("qty_sup"));
				else
					skip_entry = goto_field (field, label ("sman_code"));
				return (SUCCESS);
			}
			sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
			DSP_FLD ("item_no");
			return (SUCCESS);
		}

		if (SRCH_KEY)
		{
			i = atoi (cohr_rec.pri_type) - 1;
			InmrSearch 
			 (
				comm_rec.co_no, 
				temp_str, 
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
			return (SUCCESS);
		}

		if (dflt_used || last_char == DELLINE)
			return (DeleteLine (TRUE));

		DSP_FLD ("item_no");
		return (ValidateItemNumber (TRUE));
	}

	/*
	 * Validate item Description.
	 */
	if (LCHECK ("descr"))
	{
		if (NON_STOCK (line_cnt))
		{
			skip_entry = goto_field (field, label ("extend"));
			DSP_FLD ("crd_type");
			strcpy (coln_rec.crd_type, "R");
			strcpy (SR.creditType, "R");
		}
		line_display ();
		return (SUCCESS);
	}
	/*
	 * Validate Pack Size.
	 */
	if (LCHECK ("pack_size"))
	{
		if (dflt_used || (prog_status == ENTRY && F_NOKEY (field)))
		{
			strcpy (coln_rec.pack_size, SR.packSize);
			DSP_FLD ("pack_size");
		}
		return (SUCCESS);
	}

	/*
	 * Validate Credit type.
	 */
	if (LCHECK ("crd_type"))
	{
		/*
		 * if user presses down key then we want it to act as
		 * enter else scrgen sets skip_entry to 0      
		 */
		if (last_char == DOWN_KEY)
			last_char = ENTER;

		if (dflt_used)
		{
			if (!strcmp (local_rec.app_inv_no, "00000000") || 
			    !strcmp (local_rec.app_inv_no, "        "))
			{
				strcpy (coln_rec.crd_type, "R");
			}
			else
			if (prog_status == ENTRY)
				strcpy (coln_rec.crd_type, "R");
			else
				strcpy (coln_rec.crd_type, "I");

		}
		switch (coln_rec.crd_type [0])
		{
		case	'R':
		case	'F':
			if (SERIAL)
			{
				if (local_rec.qtyReturn != 1 && prog_status != ENTRY)
				{
					local_rec.qtyReturn = 1.00;
					DSP_FLD ("qtyReturn");
					cc = spec_valid (label ("qtyReturn"));
					while (cc && !restart)
					{
						get_entry (label ("qtyReturn"));
						cc = spec_valid (label ("qtyReturn"));
					}
				}
				SR.qtyReturn = local_rec.qtyReturn;
				DSP_FLD ("qtyReturn");
				if (!strcmp (SR.serialNo, ser_space) && prog_status != ENTRY)
				{
					do
					{
						get_entry (label ("serialNo"));
						cc = spec_valid (label ("serialNo"));
					} while (cc && !restart);
				}
			}
			else
			{
				if (local_rec.qtyReturn == 0 && prog_status != ENTRY)
				{
					do
					{
						get_entry (label ("qtyReturn"));
						cc = spec_valid (label ("qtyReturn"));
					} while (cc && !restart);
				}
				SR.qtyReturn	= local_rec.qtyReturn;
				DSP_FLD ("qtyReturn");
			}
			break;

		case	'D':
			if (SERIAL && strcmp (SR.serialNo, ser_space) && prog_status != ENTRY)
			{
				if (OLD_INSF)
				{
				   	cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");
					if (cc && cc < 1000)
						file_err (cc, insf, "DBUPDATE");
				}
				else
				{
					RemoveInsf (SR.hhwhHash, SR.serialNo);
				}
				strcpy (SR.serialNo, ser_space);
				strcpy (local_rec.serial_no, ser_space);
				DSP_FLD ("serialNo");
			}
			if (SR.contractStat)
			{
				print_mess (ML (mlSoMess044));
				sleep (sleepTime);
				clear_mess ();

				i = prmptmsg (err_str, "YyNn", 1, 23);
				BusyFunction (BUSY_OFF);
				if (i != 'Y' && i != 'y')
					return (RETURN_ERR);
			}
			SR.contractStat = 0;
			SR.qtyReturn = 1.00;
			local_rec.qtyReturn = 0.00;
			SR.discPc = 0.00;
			coln_rec.disc_pc = 0.00;;
			DSP_FLD ("qtyReturn");
			DSP_FLD ("disc");
			break;

		case	'I':
			if (prog_status != ENTRY)
				return (DeleteLine (TRUE));
			
			SR.qtyReturn = 0.00;
			local_rec.qtyReturn = 0.00;
			DSP_FLD ("qtyReturn");
			print_mess (ML (mlSoMess049));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
			break;
		}
		strcpy (SR.creditType, coln_rec.crd_type);
		CalculateInputTotal ();

		if (!strcmp (local_rec.app_inv_no, "00000000") ||
		    !strcmp (local_rec.app_inv_no, "        "))
		{
			strcpy (SR.cusOrdRef, cohr_rec.cus_ord_ref);
		}

		tab_other (line_cnt);
		return (SUCCESS);
	}

	/*
	 * Validate Unit of Measure (UOM).
	 */
	if (LCHECK ("uom"))
	{
		if (CCN_DISPLAY)
			return (SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.uom, inum_rec.uom);
			strcpy (SR.UOM, local_rec.uom);
			SR.hhumHash	=	inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			DSP_FLD ("uom");
			return (SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		strcpy (local_rec.uom, inum2_rec.uom);
		strcpy (SR.UOM, local_rec.uom);
		SR.hhumHash	=	inum2_rec.hhum_hash;
        if (inum_rec.cnv_fct == 0.00)
             inum_rec.cnv_fct = 1.00;
        SR.cnvFct = inum2_rec.cnv_fct/inum_rec.cnv_fct;
		PriceProcess ();
		DiscProcess ();
		DSP_FLD ("uom");
		return (SUCCESS);

	}

	/*
	 * Validate Quantity Returned.
	 */
	if (LCHECK ("qtyReturn"))
	{
		if (dflt_used)
		{
			if ((APP_INV || CCN_CONF) && local_rec.qty_sup > 0.00)
				local_rec.qtyReturn = local_rec.qty_sup;
			else
				local_rec.qtyReturn = 1.00;
		}

		if (KIT_ITEM && !DOL_RET)
		{
			SR.qtyReturn	= local_rec.qtyReturn;
			this_page 	= line_cnt / TABLINES;
			ProcessPhantomItem (inmr_rec.hhbr_hash, local_rec.qtyReturn);
			skip_entry = goto_field (field, label ("item_no"));
			SR.qtyReturn 		= 0.00;
			local_rec.qtyReturn 	= 0.00;
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
					cc = spec_valid (label ("LL"));
				} while (cc && !restart);
			}

			return (SUCCESS);
		}

		if (line_cnt < lcount [ITEM_SCN] && 
			APP_INV && 
			local_rec.qtyReturn > local_rec.qty_sup)
		{
			print_mess (ML (mlSoMess045));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (SERIAL && local_rec.qtyReturn != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (!APP_INV)
		{
			PriceProcess ();
			DiscProcess ();
			SR.actSalePrice = coln_rec.sale_price;
		}

		/*
		 * Lines to be ignored.
		 */
		if (coln_rec.crd_type [0] == 'I' && local_rec.qtyReturn != 0.00)
		{
			SR.qtyReturn = 1.00;
			local_rec.qtyReturn = 0.00;
			DSP_FLD ("qtyReturn");
			print_mess (ML (mlSoMess050));
			sleep (sleepTime);
			clear_mess ();
			return (SUCCESS);
		}
		/*
		 * Lines to be Dollar Credit.
		 */
		if (coln_rec.crd_type [0] == 'D' && local_rec.qtyReturn != 0.00)
		{
			SR.qtyReturn = 1.00;
			local_rec.qtyReturn = 0.00;
			SR.discPc = 0.00;
			coln_rec.disc_pc = 0.00;;
			DSP_FLD ("disc");
			DSP_FLD ("qtyReturn");
			print_mess (ML (mlSoMess051));
			sleep (sleepTime);
			clear_mess ();
		}
		else
			SR.qtyReturn = local_rec.qtyReturn;

		CalculateInputTotal ();
		PrintExtend (line_cnt);

		if (prog_status == ENTRY && NO_COST)
		{
			BusyFunction (BUSY_OFF);
			print_at (2, 1, ML (mlSoMess238));
			SR.costPrice = getmoney (20, 2, "NNNNNNN.NN");
		}
		if (FLD ("LL") == ND)
		{
			if (DOL_RET || (coln_rec.sale_price == 0.00 && !BONUS))
				skip_entry = 0;
			else
			{
				DSP_FLD ("sale_price");
				DSP_FLD ("disc");
				if (SERIAL_ITEM && !strcmp (coln_rec.serial_no, ser_space))
					skip_entry = goto_field (field, label ("serialNo"));
				else
					skip_entry = goto_field (field, label ("extend"));
			}
		}
		else
		{
			if (prog_status != ENTRY)
			{
				DSP_FLD ("qtyReturn");

				/*
				 * Reenter Location.
				 */
				do
				{
					strcpy (local_rec.LL, "N");
					get_entry (label ("LL"));
					cc = spec_valid (label ("LL"));
				} while (cc && !restart);
			}

		}
	
		return (SUCCESS);
	}

	/*
	 * Validate Sale Price.
	 */
	if (LCHECK ("sale_price"))
	{
		if (dflt_used)
		{
			strcpy (SR.priOverride, "N");
			if (!newCCN || APP_INV)
				coln_rec.sale_price = SR.actSalePrice;
			else
			{
				PriceProcess ();
				DiscProcess ();
			}
			DSP_FLD ("sale_price");
		}

		if (BONUS)
		{
			print_mess (ML (mlSoMess233));
			sleep (sleepTime);
			clear_mess ();

			coln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");
			return (SUCCESS);
		}

		if (coln_rec.sale_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess031), "YyNn", 1, 2);
			BusyFunction (BUSY_OFF);
			if (i != 'Y' && i != 'y')
				return (RETURN_ERR);
		}

		SR.salePrice	=	coln_rec.sale_price;

		if (SR.calcSalePrice != coln_rec.sale_price)
			strcpy (SR.priOverride, "Y");

		/*
		 * Calculate new GROSS sale price.
		 */
		SR.grossSalePrice = (SR.salePrice / (1.00 - (SR.regPc / 100.00)));
		SR.salePrice = GetCusGprice (SR.grossSalePrice, SR.regPc);
		coln_rec.sale_price = SR.salePrice;

		DiscProcess ();

		SR.actSalePrice = coln_rec.sale_price;

		if (SR.taxAmount == 0.00)
			SR.taxAmount = SR.salePrice;

		CalculateInputTotal ();
		PrintExtend (line_cnt);

		if (SERIAL_ITEM && !DOL_RET && !strcmp (coln_rec.serial_no, ser_space))
			skip_entry = goto_field (field, label ("serialNo"));
		else
			skip_entry = goto_field (field, label ("extend"));
		return (SUCCESS);
	}

	/*
	 * Validate Discount.
	 */
	if (LCHECK ("disc"))
	{
		if (FLD ("disc") == NI && prog_status == ENTRY)
			return (SUCCESS);

		if (dflt_used)
		{
			strcpy (SR.disOverride, "N");
			DiscProcess ();
		}
		if (SR.contractPrice || SR.contractStat == 2)
		{
			coln_rec.disc_pc 	=	0.00;
			SR.discA_Pc			=	0.00;
			SR.discB_Pc			=	0.00;
			SR.discC_Pc			=	0.00;
			DSP_FLD ("disc");
		}
		SR.discPc = ScreenDisc (coln_rec.disc_pc);

		if (SR.calcDisc != ScreenDisc (coln_rec.disc_pc))
			strcpy (SR.disOverride, "Y");

		/*
		 * Discount has been entered so set disc B & C to zero. 
		 */
		if (!dflt_used)
		{
			SR.discA_Pc = SR.discPc;
			SR.discB_Pc = 0.00;
			SR.discC_Pc = 0.00;
		}
		CalculateInputTotal ();
		PrintExtend (line_cnt);

		if (SERIAL_ITEM && !DOL_RET && !strcmp (coln_rec.serial_no, ser_space))
			skip_entry = goto_field (field, label ("serialNo"));
		else
			skip_entry = goto_field (field, label ("extend"));
	}

	/*
	 * Validate lots and locations.
	 */
	if  (LCHECK ("LL"))
	{
		int	LLReturnValue	=	0;

		if  (FLD ("LL") == ND || DOL_RET)
			return  (SUCCESS);

		TempLine	=	lcount [2];
		LLReturnValue = DisplayLL
			 (									/*----------------------*/
				line_cnt, 						/*	Line number.		*/
				tab_row + 3 +  (line_cnt % TABLINES), /*  Row for window*/
				tab_col + 22, 					/*  Col for window		*/
				4, 								/*  length for window	*/
				SR.hhwhHash, 					/*	Warehouse hash.		*/
				SR.hhumHash, 					/*	UOM hash			*/
				SR.hhccHash, 					/*	CC hash.			*/
				SR.UOM, 						/* UOM					*/
				SR.qtyReturn, 					/* Quantity.			*/
				SR.cnvFct, 						/* Conversion factor.	*/
				TodaysDate (), 					/* Expiry Date.			*/
				FALSE, 							/* Silent mode			*/
				(local_rec.LL [0] == 'Y'), 		/* Input Mode.			*/
				SR.lotFlag						/* Lot controled item. 	*/
												/*----------------------*/
			);

		/*
		 * Redraw screens. 
		 */
		strcpy (local_rec.LL, "Y");
		strcpy (SR.LL, "Y");
		putval (line_cnt);

		lcount [2] =  (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		scn_write  (2);
		scn_display  (2);
		lcount [2] = TempLine;
		PrintCompanyDetails ();

		if (LLReturnValue)
			return (RETURN_ERR);
		
		if (DOL_RET || (coln_rec.sale_price == 0.00 && !BONUS))
			skip_entry = 0;
		else
		{
			DSP_FLD ("sale_price");
			DSP_FLD ("disc");
			if (SERIAL_ITEM && !strcmp (coln_rec.serial_no, ser_space))
				skip_entry = goto_field (field, label ("serialNo"));
			else
				skip_entry = goto_field (field, label ("extend"));

		}
		DSP_FLD ("LL");
		return  (SUCCESS);
	}
	/*
	 * Validate serial number input. 
	 */
	if (LCHECK ("serialNo"))
	{
		if (F_HIDE (field) || FIELD.required == NA)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SearchInsf (SR.hhwhHash, "S", temp_str);
			return (SUCCESS);
		}

		if (restart)
			return (SUCCESS);

		if (dflt_used || !strcmp (local_rec.serial_no, ser_space))
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (!strcmp (local_rec.serial_no, SR.serialNo))
		{
			DSP_FLD ("serialNo");
			return (SUCCESS);
		}

		cc = FindInsf (SR.hhwhHash, 0L, local_rec.serial_no, "S", "r");
		if (cc)
		{
			cc = FindInsf (SR.hhwhHash, 0L, local_rec.serial_no, "C", "r");
			if (!cc)
			{
				print_mess (ML (mlSoMess052));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			cc = FindInsf (SR.hhwhHash, 0L, local_rec.serial_no, "F", "r");
			if (!cc)
			{
				print_mess (ML (mlSoMess053));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
		}
		if (cc)
		{
			i= prmptmsg (ML (mlSoMess057) , "YyNn", 1, 2);
			BusyFunction (BUSY_OFF);
			if (i != 'Y' && i != 'y') 
			{
				print_mess (ML (mlStdMess201));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			else
			{
				if (CheckDuplicateInsf (local_rec.serial_no, SR.hhsiHash, line_cnt))
				{
					print_mess (ML (mlStdMess097));
					sleep (sleepTime);
					clear_mess ();
					return (RETURN_ERR);
				}
				cc = AddInsf 
					 (
						SR.hhwhHash, 
						SR.hhsiHash, 
						local_rec.serial_no, 
						StringToDate (local_rec.dbt_date)
					);
				if (cc)
					file_err (cc, insf, "DBFIND");

				if (strcmp (SR.serialNo, ser_space))
				{
					if (OLD_INSF)
					{
						cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");
						if (cc && cc < 1000)
							file_err (cc, insf, "DBUPDATE");
					}
					else
						RemoveInsf (SR.hhwhHash, SR.serialNo);
				}

				sprintf (SR.serialNo, "%-25.25s", local_rec.serial_no);
				strcpy (SR.oldSerial, "N");
			}
			DSP_FLD ("serialNo");
			return (SUCCESS);
		}

		if (CheckDuplicateInsf (local_rec.serial_no, SR.hhsiHash, line_cnt))
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		/*
		 * Reset status of old serial number
		 */
		if (strcmp (SR.serialNo, ser_space))
		{
			if (OLD_INSF)
			{
				cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}
			else
				RemoveInsf (SR.hhwhHash, SR.serialNo);
		}

		sprintf (SR.serialNo, "%-25.25s", local_rec.serial_no);
		strcpy (SR.oldSerial, "Y");

		cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "S", "C");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");

		DSP_FLD ("serialNo");
		return (SUCCESS);
	}

	/*
	 * Validate Salesman Code At Item Level.
	 */
	if (LCHECK ("sman_code"))
	{
		if (prog_status == ENTRY)
		{
			strcpy (coln_rec.sman_code, cohr_rec.sale_code);

			if (FIELD.required != ND)
				DSP_FLD ("sman_code");
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, coln_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (!F_HIDE (field))
		{
			move (1, 2);
			print_at (2, 1, ML (mlStdMess202), exsf_rec.salesman_no, exsf_rec.salesman);

		}
		return (SUCCESS);
	}

	if (LCHECK ("pack_size"))
	{
		if (dflt_used || (prog_status == ENTRY && F_NOKEY (field)))
		{
			strcpy (coln_rec.pack_size, SR.packSize);
			DSP_FLD ("pack_size");
		}
		return (SUCCESS);
	}
	if (LCHECK ("extend"))
	{
		PrintExtend (line_cnt);
		return (SUCCESS);
	}

	if (LCHECK ("shipname") || LNCHECK ("shipaddr", 8)) 
	{
		if (SRCH_KEY)
		{
			open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");

			i = SrchCudi (field - label ("shipname"));

			abc_fclose (cudi);
			if (i < 0)
				return (SUCCESS);

			strcpy (cohr_rec.dl_name, cudi_rec.name);
			strcpy (cohr_rec.dl_add1, cudi_rec.adr1);
			strcpy (cohr_rec.dl_add2, cudi_rec.adr2);
			strcpy (cohr_rec.dl_add3, cudi_rec.adr3);
		}
		return (SUCCESS);
	}

	/*
	 * Validate Freight Required Flag.
	 */
	if (LCHECK ("freightRequiredFlag"))
	{
		if (F_NOKEY (field))
			return (SUCCESS);

		if (FREIGHT_CHG)
		{
			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg, 
				trzm_rec.chg_kg, 
				trzm_rec.dflt_chg
			);
			return (SUCCESS);
		}
		return (SUCCESS);
	}

	/*
	 * Validate Carrier Code.
	 */
	if (LCHECK ("carrierCode"))
	{
		if (F_NOKEY (field))
			return (SUCCESS);

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
			return (SUCCESS);
		}
			
		OpenTransportFiles ("trzm_trzm_hash");

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			CloseTransportFiles ();
			return (SUCCESS);
		}
			
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
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
				trcm_rec.trcm_hash	=	0L;
				return (RETURN_ERR);
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
			return (SUCCESS);
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		clear_mess ();

		CloseTransportFiles ();
		return (RETURN_ERR);
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
			return (SUCCESS);
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
			return (SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			CloseTransportFiles ();
			return (RETURN_ERR); 
		}
		if (trcm_rec.trcm_hash > 0L)
		{
			strcpy (trcm_rec.co_no, comm_rec.co_no);
			strcpy (trcm_rec.br_no, comm_rec.est_no);
			cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
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
					return (RETURN_ERR);
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
				return (SUCCESS);
			}
		}
		DSP_FLD ("deliveryZoneDesc");
		DSP_FLD ("deliveryZoneCode");

		CloseTransportFiles ();
		return (SUCCESS);
	}
	/*
	 * Validate delivery required.
	 */
	if (LCHECK ("deliveryRequired"))
	{
		if (!newCCN)
		{
			move (0, 2);cl_line ();
			i = prmptmsg (ML (mlTrMess063) , "YyNn", 0, 2);
			BusyFunction (BUSY_OFF);
			if (i == 'N' || i == 'n') 
				return (SUCCESS);

			sprintf (err_str, "tr_trsh_mnt C %010ld LOCK", cohr_rec.hhco_hash);
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
					file_err (cc, "cohr", "DBUPDATE");
			}
			heading (FREIGHT_SCN);
			scn_write (FREIGHT_SCN);
			scn_display (FREIGHT_SCN);
			print_mess (ML (mlTrMess076));
			sleep (sleepTime);
			clear_mess ();
			DSP_FLD ("deliveryDate");
		}
		return (SUCCESS); 
	}
	/*
	 * Validate Proof Total
	 */
	if (LCHECK ("proof"))
	{
		clear_mess ();
		inv_proof = 1;
		if (end_input) 
		{
			entry_exit = 1;
			return (SUCCESS);
		}
		CalculateExtendedTotal (TRUE);
		/*
		 * Reset to proof screen.
		 */
		scn_set (PROOF_SCN);
		totalProof = no_dec (totalProof);
		crdTotalAmt = no_dec (crdTotalAmt);
		if (totalProof < crdTotalAmt - .01 || totalProof > crdTotalAmt + .01)
		{
			print_mess (ML (mlSoMess046));
			sleep (sleepTime);
			clear_mess ();

			inv_proof = 1;
			return (RETURN_ERR); 
		}
		inv_proof = 0;
		if (envVar.dbMcurr)
			crdTotalAmt = (crdTotalAmt / pocr_rec.ex1_factor);
		totalBatch += crdTotalAmt;
		return (SUCCESS);
	}

	return (SUCCESS);
}

void
ProcessPhantomItem (
 long		hhbrHash, 
 float		qty)
{
	int		i;
	int		this_page;
	char	*sptr;
	long	hold_date = cohr_rec.date_required;
	float	lcl_qty = (float) 0;

	this_page = line_cnt / TABLINES;

	cc = open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_id_no");
	if (cc)
		file_err (cc, "sokt", "OPEN_REC");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = hhbrHash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
		   sokt_rec.hhbr_hash == hhbrHash)
	{
		abc_selfield (inmr, "inmr_hhbr_hash");
		inmr_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
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

		DSP_FLD ("item_no");
		if (ValidateItemNumber (FALSE))
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (SR.serialFlag [0] == 'Y')
		{
			sprintf (local_rec.serial_no, "%25.25s", " ");
			strcpy (SR.serialNo, local_rec.serial_no);
			DSP_FLD ("serialNo");
		}

		if (SERIAL_ITEM)
		{
			lcl_qty = sokt_rec.matl_qty * qty;
			local_rec.qtyReturn = 1.00;
			local_rec.qty_sup = 0.00;
		}
		else
		{
			local_rec.qtyReturn = sokt_rec.matl_qty * qty;
			local_rec.qty_sup = 0.00;
		}

		if (local_rec.qtyReturn == 0.00)
			get_entry (label ("qtyReturn"));
		
		if (local_rec.qtyReturn == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.due_date == 0L)
			cohr_rec.date_required = hold_date;
		else
			cohr_rec.date_required = sokt_rec.due_date;

		/*
		 * if serial we need to to load one line per qty ordered
		 */
		if (SERIAL_ITEM)
		{
			int	count;

			abc_selfield (inmr, "inmr_hhbr_hash");
			inmr_rec.hhbr_hash = sokt_rec.mabr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
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
					lcount [ITEM_SCN] = line_cnt;
					this_page = line_cnt / TABLINES;
				}

				for (i = label ("qtyReturn"); i <= label ("extend"); i++)
				{
					skip_entry = 0;
					do
					{
						if (SERIAL_ITEM && i == label ("serialNo"))
							get_entry (i);

						if (restart)
							return;

						cc = spec_valid (i);

						/*
						 * if spec_valid returns 1, re-enter field
						 * eg. if kit item has no sale value,  
						 * re-prompt for sal value if required.
						 */
						if (cc && ! (SERIAL_ITEM && i == label ("serialNo")))
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
					SR.qtyReturn = local_rec.qtyReturn;
	
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
				lcount [ITEM_SCN] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			for (i = label ("qtyReturn"); i <= label ("extend"); i++)
			{
				skip_entry = 0;
				do
				{
					if (SERIAL_ITEM && i == label ("serialNo"))
						get_entry (i);

					if (restart)
						return;

					cc = spec_valid (i);
					/*
					 * if spec_valid returns 1, re-enter field
					 * eg. if kit item has no sale value,  
					 * re-prompt for sal value if required.
					 */
					if (cc && ! (SERIAL_ITEM && i == label ("serialNo")))
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
				SR.qtyReturn = local_rec.qtyReturn;

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
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [ITEM_SCN] = line_cnt;
	abc_fclose (sokt);
	cohr_rec.date_required = hold_date;
}

void
tab_other (
 int line_no)
{
	/*
	 * turn off and on editing of fields depending on whether contract or not.
	 */
	FLD ("disc") 		= inp_disc;
	FLD ("sale_price") 	= inp_sale;

	if (store [line_no].contractStat)
	{
		FLD ("disc") = (store [line_no].contractStat == 1) ? NA : inp_disc;
		FLD ("sale_price") 	= NA;
	}

	/*
	 * turn off and on editing of serial field depending on item.
	 */
	if (store [line_no].serialFlag [0] != 'Y')
	{
	 	if (!F_HIDE (label ("serialNo")))
			FLD ("serialNo") = NA;
	}
	else
	{
		if (FLD ("serialNo") == NA)
			FLD ("serialNo") = YES;
	}
}

void
CreditAll (
 void)
{
	scn_set (ITEM_SCN);

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt);

		if (CCN_CONF)
			local_rec.qty_sup = local_rec.qtyReturn;

		local_rec.qtyReturn = local_rec.qty_sup;
		
		SR.qtyReturn = local_rec.qtyReturn;

		CalculateLineTotal (line_cnt);
		local_rec.extend = store [line_cnt].extendedTotal;
		putval (line_cnt);
	}
	scn_set (HEADER_SCN);
}

void
PrintExtend (
 int	CurLine)
{
	CalculateLineTotal (CurLine);
	local_rec.extend = store [CurLine].extendedTotal;

	DSP_FLD ("extend");
}

int
ValidateItemNumber (
 int	getFields)
{
	int		itemChanged = FALSE;
	char	*sptr;
	long	orighhbrHash;

	abc_selfield (inmr, "inmr_id_no");

	if (prog_status == ENTRY)
		sprintf (coln_rec.serial_no, "%25.25s", " ");

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.item_no);

	SR.bonusFlag [0] = (CheckBonusLine (inmr_rec.item_no)) ? 'Y' : 'N';
	coln_rec.hide_flag [0] = (CheckHiddenLine (inmr_rec.item_no)) ? 'Y' : 'N';


	cc = FindInmr 
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
		errmess (ML (mlStdMess001));
		sleep (sleepTime);
		clear_mess ();
		return (RETURN_ERR);
	}

	orighhbrHash = inmr_rec.hhbr_hash;

	SuperSynonymError ();

	sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
	if (SERIAL_ITEM && F_HIDE (label ("serialNo")))
	{
		print_mess (ML (mlSoMess054));
		sleep (sleepTime);
		clear_mess ();
		return (RETURN_ERR);
	}


	if (prog_status   != ENTRY &&
		SR.hhbrHash != inmr_rec.hhbr_hash &&
		SR.hhbrHash != 0)
	{
		if (inmr_rec.inmr_class [0] == 'K')
		{
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		itemChanged = TRUE;
		local_rec.qty_sup = 0.00;
		if (strcmp (SR.serialNo, ser_space) && SERIAL)
		{
 			if (SR.oldSerial [0] == 'Y')
			{
				cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");

				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}
			else
			{
				RemoveInsf (SR.hhwhHash, SR.serialNo);
			}
		}
		strcpy (local_rec.serial_no, ser_space);
		strcpy (SR.serialNo, local_rec.serial_no);
		DSP_FLD ("serialNo");
	}

	SR.hhbrHash 	= 	inmr_rec.hhbr_hash;
	SR.hhsiHash 	= 	alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	SR.weight 		= 	inmr_rec.weight;
	SR.outerSize 		= 	inmr_rec.outer_size;
	SR.defaultDisc	= 	inmr_rec.disc_pc;
	SR.itemClass [0] 	= 	inmr_rec.inmr_class [0];
	SR.lotFlag [0] = 	inmr_rec.lot_ctrl [0];

	strcpy (SR.category, 	inmr_rec.category);
	strcpy (SR.sellGroup, 	inmr_rec.sellgrp);
	strcpy (SR.costingFlag, 	inmr_rec.costing_flag);
	strcpy (SR.packSize, 	inmr_rec.pack_size);
	strcpy (SR.serialFlag, inmr_rec.serial_item);

	/*
	 * Find for UOM GROUP.
	 */
	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
	
	SR.hhumHash 	= inum_rec.hhum_hash;
	SR.cnvFct		= inum_rec.cnv_fct;

    strcpy (local_rec.uom, inmr_rec.sale_unit);

	/*
	 * Check for Indent items.
	 */
	if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
		SR.indent = FALSE;
	else
		SR.indent = TRUE;

	if (CheckIncc ())
		return (SUCCESS);

	SR.hhwhHash = incc_rec.hhwh_hash;
	SR.hhccHash = incc_rec.hhcc_hash;

	if (inmr_rec.hhbr_hash != orighhbrHash) 
	{
		BusyFunction (BUSY_OFF);
		sprintf (err_str, ML (mlSoMess234) , clip (local_rec.sup_part), clip (local_rec.item_no), BELL);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
	}

	strcpy (coln_rec.item_desc, inmr_rec.description);

	SR.extendedTotal = 0.00;
	
	if (!BONUS)
	{
		coln_rec.tax_pc = inmr_rec.tax_pc;
		coln_rec.gst_pc = inmr_rec.gst_pc;
		SR.taxPc 		= inmr_rec.tax_pc;
		SR.gstPc 		= inmr_rec.gst_pc;
		SR.taxAmount 	= inmr_rec.tax_amount;
	}
	else
	{
		sprintf (bonusFlag, "%-2.2s", envVar.soSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no, "%-s%-.*s", 
			sptr, 16 - (int) strlen (sptr), bonusFlag);

		DSP_FLD ("item_no");

		coln_rec.tax_pc = 0.00;
		coln_rec.gst_pc = 0.00;
		SR.taxPc 		= 0.00;
		SR.gstPc 		= 0.00;
		SR.taxAmount 	= 0.00;
	}

	DSP_FLD ("descr");
	
	/*
	 * Item is a serial item.
	 */
	if (SERIAL_ITEM)
	{
		if (!F_HIDE (label ("serialNo")))
			FLD ("serialNo") = YES;
	}
	else
	{
		if (!F_HIDE (label ("serialNo")))
			FLD ("serialNo") = NA;
	}

	if (itemChanged)
	{
		strcpy (coln_rec.crd_type, "R");
		strcpy (SR.creditType, coln_rec.crd_type);
		DSP_FLD ("crd_type");
		local_rec.qty_sup = 0.00;
		local_rec.qtyReturn = 0.00;
		SR.qtyReturn = local_rec.qtyReturn;
		DSP_FLD ("qty_sup");
		DSP_FLD ("qtyReturn");
		PriceProcess ();
		DiscProcess ();
		CalculateInputTotal ();
	}


	if (itemChanged && getFields)
	{
		if (SERIAL)
		{
			local_rec.qtyReturn = 1.00;
			DSP_FLD ("qtyReturn");
			cc = spec_valid (label ("qtyReturn"));
			while (cc && !restart)
			{
				get_entry (label ("qtyReturn"));
				cc = spec_valid (label ("qtyReturn"));
			}
			SR.qtyReturn = local_rec.qtyReturn;
			do
			{
				get_entry (label ("serialNo"));
				cc = spec_valid (label ("serialNo"));
			} while (cc && !restart);
		}
		else
		{
			do
			{
				get_entry (label ("qtyReturn"));
				cc = spec_valid (label ("qtyReturn"));
			} while (cc && !restart);
			DSP_FLD ("qtyReturn");
		}
	}

	if (NON_STOCK (line_cnt))
		skip_entry = goto_field (label ("item_no"), label ("extend"));
	else
		skip_entry = 2;

	sptr = clip (inmr_rec.description);
	if (strlen (sptr) == 0)
		skip_entry = 1;

	if (!F_HIDE (label ("hide")))
		DSP_FLD ("hide");

	tab_other (line_cnt);
	return (SUCCESS);
}


void
PriceProcess (
 void)
{
	int		pType;
	float	regPc;
	double	grossPrice;

	SR.pricingCheck	= FALSE;

	if (BONUS)
	{
		coln_rec.sale_price 	= 0.00;
		SR.actSalePrice 			= 0.00;
		SR.calcSalePrice			= 0.00;
		SR.salePrice 			= 0.00;
		DSP_FLD ("sale_price");

		coln_rec.disc_pc  		= 0.00;
		SR.discPc 	 			= 0.00;
		SR.calcDisc 			= 0.00;
		DSP_FLD ("disc");
		return;
	}
	pType = atoi (cumr_rec.price_type);
	grossPrice	=	GetCusPrice 
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
						(envVar.useSystemDate) ? TodaysDate () : comm_rec.dbt_date, 
						ToStdUom (local_rec.qtyReturn), 
						pocr_rec.ex1_factor, 
						FGN_CURR, 
						&regPc
					);

	SR.pricingCheck	= TRUE;

	SR.calcSalePrice = GetCusGprice (grossPrice, regPc) * SR.cnvFct;

	if (SR.priOverride [0] == 'N')
	{
		SR.grossSalePrice 	= 	grossPrice * SR.cnvFct;
		SR.salePrice 		=	SR.calcSalePrice;
		SR.regPc 			= 	regPc;
		coln_rec.sale_price = 	SR.calcSalePrice;
		SR.actSalePrice 	=	SR.calcSalePrice;
	}
	SR.contractPrice 		= (_CON_PRICE) ? TRUE : FALSE;
	SR.contractStat  		= _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess (
 void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*
	 * Discount does not apply.
	 */
	if (DOL_RET || SR.contractStat == 2 || SR.contractPrice || SR.indent)
	{
		coln_rec.disc_pc  	= 0.00;
		SR.discPc 	 		= 0.00;
		SR.calcDisc 		= 0.00;
		SR.discA_Pc			= 0.00;
		SR.discB_Pc			= 0.00;
		SR.discC_Pc			= 0.00;
		DSP_FLD ("disc");
		return;
	}

	if (SR.pricingCheck == FALSE)
		PriceProcess ();

	pType = atoi (cumr_rec.price_type);
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
								SR.grossSalePrice, 
								SR.regPc, 
								ToStdUom (local_rec.qtyReturn), 
								discArray);
							
	if (SR.disOverride [0] == 'Y')
	{
		DSP_FLD ("disc");
		return;
	}
	SR.calcDisc		=	CalcOneDisc (cumDisc, 
								 		 discArray [0], 
								 		 discArray [1], 
								 		 discArray [2]);

	if (SR.disOverride [0] == 'N')
	{
		coln_rec.disc_pc 	=	ScreenDisc (SR.calcDisc);
		SR.discPc			=	SR.calcDisc;

		SR.discA_Pc 			= 	discArray [0];
		SR.discB_Pc 			= 	discArray [1];
		SR.discC_Pc 			= 	discArray [2];
		SR.cumulative 		= 	cumDisc;

		if (SR.defaultDisc > ScreenDisc (coln_rec.disc_pc) &&
			SR.defaultDisc != 0.0)
		{
			coln_rec.disc_pc = 	ScreenDisc (SR.defaultDisc);
			SR.calcDisc	=	SR.defaultDisc;
			SR.discPc		=	SR.defaultDisc;
			SR.discA_Pc 		= 	SR.defaultDisc;
			SR.discB_Pc 		= 	0.00;
			SR.discC_Pc 		= 	0.00;
		}
	}
	if (line_cnt / TABLINES != scn_page)
		return;

	DSP_FLD ("disc");
	CalculateInputTotal ();
}

int
CheckIncc (
 void)
{
	int		i;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);

	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc) 
	{
		BusyFunction (BUSY_OFF);
		i = prmptmsg (ML (mlStdMess033), "YyNn", 1, 2);
		if (i == 'n' || i == 'N') 
		{
			skip_entry = -1 ;
			return (RETURN_ERR); 
		}
		else 
		{
			cc = AddIncc ();
			if (cc)
				file_err (cc, incc, "DBADD");
		}
	}
	return (SUCCESS);
}

int
DeleteLine (
 int	_show)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (RETURN_ERR);
	}

	if (lcount [ITEM_SCN] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (RETURN_ERR);
	}

	print_at (2, 0, ML (mlStdMess014));

	if (OLD_INSF)
	{
	   	cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
	else
		RemoveInsf (SR.hhwhHash, SR.serialNo);

	lcount [ITEM_SCN]--;
	vars [scn_start].row = lcount [ITEM_SCN];

	for (i = line_cnt, line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		if (line_cnt >= i)
		{
			memcpy ((char *) &SR, 
					   (char *) &store [line_cnt + 1], 
					   sizeof (struct storeRec));

			/*
			 * Move lot information.
			 */
			LotMove (line_cnt, line_cnt + 1);

			getval (line_cnt + 1);
			
			if (_show && this_page == line_cnt / TABLINES)
				line_display ();
		}
		else
			getval (line_cnt);

		putval (line_cnt);
	}

	if (_show && this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	i = lcount [ITEM_SCN];

	memset ((char *) &store [i], '\0', sizeof (struct storeRec));

	/*
	 * Clear out lot information.
	 */
	LotClear (i);

	strcpy (store [i].serialNo, 	ser_space);
	strcpy (store [i].category, 	"           ");
	strcpy (store [i].sellGroup, 	"      ");
	strcpy (store [i].bonusFlag, 	" ");
	strcpy (store [i].itemClass, 		" ");
	strcpy (store [i].disOverride, 		"N");
	strcpy (store [i].priOverride, 		"N");
	strcpy (store [i].packSize, 	"     ");
	strcpy (store [i].costingFlag, " ");
	strcpy (store [i].cusOrdRef, "                    ");
	strcpy (store [i].creditType, 	 " ");
	strcpy (store [i].oldSerial, 	 "N");
	strcpy (store [i].lotFlag, 	 "N");
	strcpy (store [i].UOM, 	 	 "    ");

	BusyFunction (BUSY_OFF);

	getval (line_cnt);
	CalculateInputTotal ();
	return (SUCCESS);
}

void
SetCreditNote (
 int	new_inv)
{
	int		i;

	if (new_inv)
	{
		init_vars (FREIGHT_SCN);	

		strcpy (cohr_rec.tax_code, cumr_rec.tax_code);
		strcpy (cohr_rec.area_code, cumr_rec.area_code);
		strcpy (cohr_rec.sale_code, local_rec.dflt_sale_no);
		strcpy (cohr_rec.pri_type, cumr_rec.price_type);
		strcpy (local_rec.pri_desc, cohr_rec.pri_type);
		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type) - 1));

		sprintf (cohr_rec.din_1, "%60.60s", " ");
		sprintf (cohr_rec.din_2, "%60.60s", " ");
		sprintf (cohr_rec.din_3, "%60.60s", " ");

		/*
		 * Get any special instrunctions.
		 */
		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

		for (i = 0;i < 3;i++)
		{
			strcpy (exsi_rec.co_no, comm_rec.co_no);
			exsi_rec.inst_code = cumr_inst [i];
			cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
			if (cc)
				sprintf (local_rec.spinst [i], "%60.60s", " ");
			else 
				sprintf (local_rec.spinst [i], "%-60.60s", exsi_rec.inst_text);
		}

		abc_fclose (exsi);
		strcpy (cohr_rec.dl_name, cumr_rec.dbt_name);
		/*
		 * Get charge to address.
		 */
		strcpy (cohr_rec.dl_add1, cumr_rec.dl_adr1);
		strcpy (cohr_rec.dl_add2, cumr_rec.dl_adr2);
		strcpy (cohr_rec.dl_add3, cumr_rec.dl_adr3);

		strcpy (cohr_rec.cons_no, "                ");
		strcpy (cohr_rec.carr_code, "    ");
		strcpy (cohr_rec.fix_exch, "N");
		strcpy (cohr_rec.area_code, cumr_rec.area_code);
		strcpy (cohr_rec.sale_code, local_rec.dflt_sale_no);
		strcpy (cohr_rec.pri_type, cumr_rec.price_type);

		cohr_rec.date_required = StringToDate (local_rec.systemDate);
		strcpy (cohr_rec.sell_terms, "   ");
		sprintf (cohr_rec.pay_terms, "%-40.40s", cumr_rec.crd_prd);

		sprintf (cohr_rec.ins_det, "%-30.30s", " ");

		sprintf (cohr_rec.op_id, "%-14.14s", currentUser);
		strcpy (cohr_rec.time_create, TimeHHMM ());
		cohr_rec.date_create = TodaysDate ();
		
		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms, p_terms [i]._pcode, 
					strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms, "%-40.40s", p_terms [i]._pterm);
				break;
			}
		}
	}
	else
	{
		strcpy (local_rec.app_inv_no, cohr_rec.app_inv_no);
		strcpy (local_rec.pri_desc, cohr_rec.pri_type);
		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type)-1));
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
			sprintf (cudp_rec.dp_name, "%40.40s", " ");

		strcpy (local_rec.ord_desc, (cohr_rec.ord_type [0] == 'D') ? "D" : "E");
		strcpy (local_rec.ord_fulldesc, (cohr_rec.ord_type [0] == 'D')
						? "Domestic" : "Export  ");

		if (RESTOCK_FEE && cohr_rec.other_cost_3 == 0.00)
			strcpy (local_rec.restock_fee, "N");

		/*
		 * If maintenance then need to re-read contract
		 */
		if (strcmp (cohr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cnch, "DBFIND");
		}
	}
	strcpy (local_rec.pri_desc, cohr_rec.pri_type);
	strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type)-1));

	for (i = 0;strlen (s_terms [i]._scode);i++)
	{
		if (!strncmp (cohr_rec.sell_terms, s_terms [i]._scode, 
					strlen (s_terms [i]._scode)))
		{
			sprintf (local_rec.sell_desc, "%-30.30s", s_terms [i]._sterm);
			break;
		}
	}
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, cohr_rec.sale_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman, "%40.40s", " ");

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code, cohr_rec.area_code);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exaf_rec.area_code, "%40.40s", " ");

	if (newCCN)
	{
		strcpy (cohr_rec.frei_req, cumr_rec.freight_chg);
		strcpy (cohr_rec.carr_code, "    ");
	}
	OpenTransportFiles ("trzm_id_no");
		
	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	strcpy (trzm_rec.del_zone, (newCCN) ? 
						cumr_rec.del_zone : cohr_rec.del_zone);

	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (trzm_rec.del_zone, "      ");
		trzm_rec.trzm_hash	=	0L;
		trzm_rec.dflt_chg	=	0.0;
		trzm_rec.chg_kg		=	0.0;
	}
	strcpy (cohr_rec.del_zone, trzm_rec.del_zone);
	strcpy (trcm_rec.carr_code, cohr_rec.carr_code);
	sprintf (trcm_rec.carr_desc, "%40.40s", " ");
	estimFreight 		= 	0.00;
	trcm_rec.trcm_hash	=	0L;

	if (strcmp (cohr_rec.carr_code, "    ") && trzm_rec.trzm_hash > 0L)
	{
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		strcpy (trcm_rec.carr_code, cohr_rec.carr_code);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "trcm", "DBFIND");
		
		trcl_rec.trcm_hash = trcm_rec.trcm_hash;
		trcl_rec.trzm_hash = trzm_rec.trzm_hash;
		cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "trcl", "DBFIND");

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
	scn_set (HEADER_SCN);
}

int
LoadCreditNote (
	long	hhcoHash)
{
	char	*sptr;
    float   stdcnvFct;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;

	if (!specialDisplay)
	{
		BusyFunction (BUSY_ON);
		move (10, 2);
	}

	abc_selfield (inmr, "inmr_hhbr_hash");

	coln_rec.hhco_hash = hhcoHash;
	coln_rec.line_no = 0;

	cc = find_rec (coln, &coln_rec, GTEQ, "r");

	while (!cc && hhcoHash == coln_rec.hhco_hash) 
	{
	   	/*
    	 * Get part number.
    	 */
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
    	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
    	if (cc) 
 			file_err (cc, inmr, "DBFIND");

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		stdcnvFct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash = coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		line_cnt	= lcount [ITEM_SCN];

		if (stdcnvFct == 0.00)
			stdcnvFct = 1;

		LSR.cnvFct 	= 	inum_rec.cnv_fct/stdcnvFct;

		coln_rec.sale_price		*=	LSR.cnvFct;
		coln_rec.gsale_price	*=	LSR.cnvFct;

		LSR.hhumHash = 	coln_rec.hhum_hash;
		strcpy (local_rec.uom, inum_rec.uom);
		strcpy (LSR.UOM, 	 inum_rec.uom);

		if (coln_rec.hide_flag [0] == 'Y')
		{
			sprintf (hiddenFlag, "%-2.2s", envVar.soSpecial + 2);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s", 
				sptr, 16 - (int) strlen (sptr), hiddenFlag);
		}
		else
			strcpy (local_rec.item_no, inmr_rec.item_no);

		if (coln_rec.bonus_flag [0] == 'Y')
		{
			sprintf (bonusFlag, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s", 
				sptr, 16 - (int) strlen (sptr), bonusFlag);
		}
		else
			strcpy (local_rec.item_no, inmr_rec.item_no);


		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);

		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		LSR.hhwhHash = incc_rec.hhwh_hash;
		LSR.hhccHash = incc_rec.hhcc_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			if (FLD ("serialNo") == ND)
			{
				print_mess (ML (mlSoMess054));
				sleep (sleepTime);
				clear_mess ();
				scn_set (HEADER_SCN);
				return (RETURN_ERR);
			}
			sprintf (LSR.serialNo, "%-25.25s", coln_rec.serial_no);
			sprintf (local_rec.serial_no, "%-25.25s", coln_rec.serial_no);
		}
		else
		{
			sprintf (LSR.serialNo, "%-25.25s", " ");
			sprintf (local_rec.serial_no, "%-25.25s", " ");
		}
		strcpy (LSR.serialFlag, inmr_rec.serial_item);


		/*
		 * if the line has a contract on it then  user 
		 * not allowed to edit price or disc          
		 */
		LSR.contractStat = coln_rec.cont_status;

		/*
		 * Check for Indent items.
		 */
		if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
			LSR.indent = FALSE;
		else
			LSR.indent = TRUE;

		LSR.defaultDisc 	= inmr_rec.disc_pc;
		LSR.taxPc 			= coln_rec.tax_pc;
		LSR.gstPc 			= coln_rec.gst_pc;
		LSR.discPc 			= coln_rec.disc_pc;
		LSR.taxAmount 		= inmr_rec.tax_amount;
		LSR.weight 			= inmr_rec.weight;
		LSR.outerSize 		= inmr_rec.outer_size;
		LSR.itemClass [0] 	= inmr_rec.inmr_class [0];
		LSR.bonusFlag [0]	= coln_rec.bonus_flag [0];
		LSR.actSalePrice	= coln_rec.sale_price;
		LSR.regPc 			= coln_rec.reg_pc;
		LSR.discA_Pc 		= coln_rec.disc_a;
		LSR.discB_Pc 		= coln_rec.disc_b;
		LSR.discC_Pc 		= coln_rec.disc_c;
		LSR.cumulative 		= coln_rec.cumulative;
		LSR.grossSalePrice 	= coln_rec.gsale_price;
		LSR.salePrice 		= coln_rec.sale_price;
		LSR.costPrice 		= coln_rec.cost_price;
		LSR.hhbrHash 		= coln_rec.hhbr_hash;
		LSR.hhsiHash 		= alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
		strcpy (LSR.category, inmr_rec.category);
		strcpy (LSR.sellGroup, inmr_rec.sellgrp);
		strcpy (LSR.costingFlag, inmr_rec.costing_flag);
		strcpy (LSR.packSize, inmr_rec.pack_size);
		strcpy (LSR.lotFlag, inmr_rec.lot_ctrl);
		strcpy (LSR.creditType, coln_rec.crd_type);
		strcpy (LSR.oldSerial, "Y");
		strcpy (LSR.priOverride, "N");
		strcpy (LSR.disOverride, "N");

		if (coln_rec.crd_type [0] == 'D')
		{
			local_rec.qty_sup = 0.00;
			local_rec.qtyReturn = 0.00;
		}
		else
		{
			local_rec.qty_sup = ToLclUom (coln_rec.q_order);
			local_rec.qtyReturn = ToLclUom (coln_rec.q_order);
		}

		LSR.qtyReturn = ToLclUom (coln_rec.q_order);

		/*
		 * Calculate Extended Total. 
		 */
		CalculateLineTotal (lcount [ITEM_SCN]);
		local_rec.extend = LSR.extendedTotal;

		coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);

		if (FLD ("LL") != ND)
		{
			cc = Load_LL_Lines
			 (
				lcount [ITEM_SCN], 
				LL_LOAD_CRD, 
				coln_rec.hhcl_hash, 
				LSR.hhccHash, 
				LSR.UOM, 
				LSR.cnvFct, 
				TRUE
			);
			strcpy (local_rec.LL, (cc) ? "N" : "Y");
		}
	    putval (lcount [ITEM_SCN]++);
	    if (lcount [ITEM_SCN] > MAXLINES) 
			break;

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}

	vars [scn_start].row = lcount [ITEM_SCN];

	scn_set (HEADER_SCN);
	return (SUCCESS);
}

int
LoadExistingInvoice (
 long	hhcoHash, 
 char	*contractNumber)
{
	char	*sptr;
    float   stdcnvFct = 0.00;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;

	BusyFunction (BUSY_ON);
	move (10, 2);

	/*
	 * If maintenance then need to re-read contract
	 */
	if (strcmp (contractNumber, "      "))
	{
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		sprintf (cnch_rec.cont_no, "%-6.6s", contractNumber);
		cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cnch, "DBFIND");
	}
	abc_selfield (inmr, "inmr_hhbr_hash");

	coln_rec.hhco_hash = hhcoHash;
	coln_rec.line_no = 0;

	cc = find_rec (coln, &coln_rec, GTEQ, "r");

	while (!cc && hhcoHash == coln_rec.hhco_hash) 
	{
		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc) 
			file_err (cc, inmr, "DBFIND");

		inum_rec.hhum_hash	= inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		stdcnvFct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	= coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		line_cnt	= lcount [2];

		if (stdcnvFct == 0.00)
			stdcnvFct = 1;

		LSR.cnvFct 	= 	inum_rec.cnv_fct / stdcnvFct;
		coln_rec.sale_price		*=	LSR.cnvFct;
		coln_rec.gsale_price	*=	LSR.cnvFct;
		LSR.hhumHash = 	coln_rec.hhum_hash;

		strcpy (local_rec.uom, inum_rec.uom);
		strcpy (LSR.UOM, inum_rec.uom);

		strcpy (local_rec.item_no, inmr_rec.item_no);

		if (coln_rec.hide_flag [0] == 'Y')
		{
			sprintf (hiddenFlag, "%-2.2s", envVar.soSpecial + 2);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s", 
				sptr, 16 - (int) strlen (sptr), hiddenFlag);
		}
		else
			strcpy (local_rec.item_no, inmr_rec.item_no);

		if (coln_rec.bonus_flag [0] == 'Y')
		{
			sprintf (bonusFlag, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s", 
				sptr, 16 - (int) strlen (sptr), bonusFlag);
		}
		else
			strcpy (local_rec.item_no, inmr_rec.item_no);

		/*
		 * Ignore Z class items
		 */
		if (inmr_rec.inmr_class [0] == 'Z')
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);

		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = AddIncc ();
			if (cc)
				file_err (cc, incc, "DBADD");
		}
	
		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		LSR.hhwhHash = incc_rec.hhwh_hash;
		LSR.hhccHash = incc_rec.hhcc_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			if (FLD ("serialNo") == ND)
			{
				scn_set (HEADER_SCN);
				print_mess (ML (mlSoMess054));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			sprintf (LSR.serialNo, "%-25.25s", coln_rec.serial_no);
			sprintf (local_rec.serial_no, "%-25.25s", coln_rec.serial_no);
	    	cc = UpdateInsf (LSR.hhwhHash, 0L, LSR.serialNo, "S", "C");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}
		else
		{
			sprintf (LSR.serialNo, "%-25.25s", " ");
			sprintf (local_rec.serial_no, "%-25.25s", " ");
		}
		strcpy (LSR.serialFlag, inmr_rec.serial_item);


		/*
		 * if the line has a contract on it then  user 
		 * not allowed to edit price or disc          
		 */
		LSR.contractStat = coln_rec.cont_status;

		/*
		 * Check for Indent items.
		 */
		if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
			LSR.indent = FALSE;
		else
			LSR.indent = TRUE;

		LSR.defaultDisc 		= inmr_rec.disc_pc;
		LSR.taxPc 		= coln_rec.tax_pc;
		LSR.gstPc 		= coln_rec.gst_pc;
		LSR.discPc 		= coln_rec.disc_pc;
		LSR.taxAmount 		= inmr_rec.tax_amount;
		LSR.weight 		= inmr_rec.weight;
		LSR.outerSize 			= inmr_rec.outer_size;
		LSR.itemClass [0] 		= inmr_rec.inmr_class [0];
		LSR.bonusFlag [0]		= coln_rec.bonus_flag [0];
		LSR.actSalePrice		= coln_rec.sale_price;
		LSR.regPc 		= coln_rec.reg_pc;
		LSR.discA_Pc 		= coln_rec.disc_a;
		LSR.discB_Pc 		= coln_rec.disc_b;
		LSR.discC_Pc 		= coln_rec.disc_c;
		LSR.cumulative 	= coln_rec.cumulative;
		LSR.grossSalePrice 	= coln_rec.gsale_price;
		LSR.salePrice 	= coln_rec.sale_price;
		LSR.costPrice 	= coln_rec.cost_price;
		LSR.hhbrHash 		= coln_rec.hhbr_hash;
		LSR.hhsiHash 		= alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
		strcpy (LSR.category, inmr_rec.category);
		strcpy (LSR.sellGroup, inmr_rec.sellgrp);
		strcpy (LSR.costingFlag, inmr_rec.costing_flag);
		strcpy (LSR.packSize, inmr_rec.pack_size);
		strcpy (LSR.creditType, coln_rec.crd_type);
		strcpy (LSR.lotFlag, inmr_rec.lot_ctrl);
		strcpy (LSR.oldSerial, "Y");
		strcpy (LSR.priOverride, "N");
		strcpy (LSR.disOverride, "N");
		strcpy (LSR.cusOrdRef, coln_rec.cus_ord_ref);

		local_rec.qty_sup = ToLclUom (coln_rec.q_order);
		local_rec.qtyReturn = 0.00;
		LSR.qtyReturn = 0.00;

		/*
		 * Calculate Extended Total.
		 */
		CalculateLineTotal (lcount [ITEM_SCN]);
		local_rec.extend = LSR.extendedTotal;

		coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
    	putval (lcount [ITEM_SCN]++);
    	if (lcount [ITEM_SCN] > MAXLINES) 
			break;

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}

	vars [scn_start].row = lcount [ITEM_SCN];

	/*
	 * Set other trailer details to zero.
	 */
	cohr_rec.freight 		= cohr2_rec.freight;
	cohr_rec.sos 			= cohr2_rec.sos;
	cohr_rec.insurance 		= cohr2_rec.insurance;
	cohr_rec.deposit 		= cohr2_rec.deposit;
	cohr_rec.ex_disc 		= cohr2_rec.ex_disc;
	cohr_rec.other_cost_1	= cohr2_rec.other_cost_1;
	cohr_rec.other_cost_2	= cohr2_rec.other_cost_2;
	cohr_rec.other_cost_3	= cohr2_rec.other_cost_3;
	strcpy (local_rec.spinst [0], cohr_rec.din_1);
	strcpy (local_rec.spinst [1], cohr_rec.din_2);
	strcpy (local_rec.spinst [2], cohr_rec.din_3);

	scn_set (HEADER_SCN);
	return (SUCCESS);
}

/*
 * Update all files.
 */
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

	clear ();

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;

	strcpy (cohr_rec.din_1, local_rec.spinst [0]);
	strcpy (cohr_rec.din_2, local_rec.spinst [1]);
	strcpy (cohr_rec.din_3, local_rec.spinst [2]);

	crdTotalAmt 	= 0.00;
	cohr_rec.gross 	= 0.00;
	cohr_rec.disc 	= 0.00;
	cohr_rec.tax 	= 0.00;
	cohr_rec.gst 	= 0.00;

	if (newCCN && lcount [ITEM_SCN] == 0)
	{
		for (i = 0; i < 6; i++)
		{
			rv_pr (ML (mlSoMess070) , 0, 0, i % 2);
			sleep (sleepTime);
		}
		return;
	}

	if (newCCN)
	{
		if (!strcmp (local_rec.inv_no, "00000000") || 
			!strcmp (local_rec.inv_no, "        ")) 
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "esmr", "DBFIND");

			/*
			 * Check if Invoice / Credit Note No Already	
			 * Allocated. If it has been then skip	
			 */
			while (CheckForCCN () == 0);

			cc = abc_update (esmr, &esmr_rec);
			if (cc)
				file_err (cc, "esmr", "DBUPDATE");

			inv_no = esmr_rec.nx_ccn_no;

			sprintf (err_str, "N%07ld", inv_no);
			sprintf (cohr_rec.inv_no, "%-8.8s", err_str);
			strcpy (cohr_rec.app_inv_no, local_rec.app_inv_no);
			local_rec.dflt_inv_no = 0L;
		}
		else
		{
			strcpy (cohr_rec.inv_no, local_rec.inv_no);
			strcpy (cohr_rec.app_inv_no, local_rec.app_inv_no);
			inv_no = atol (cohr_rec.inv_no);
			local_rec.dflt_inv_no = inv_no + 1L;
			if (envVar.manualPrint [0] == 'N')
				manual_inv = FALSE;
		}


		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		strcpy (cohr_rec.type, "N");
		strcpy (cohr_rec.inv_no, cohr2_rec.inv_no);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	
		cohr_rec.exch_rate = pocr_rec.ex1_factor;

		strcpy (cohr_rec.ccn_print, "N");
		strcpy (cohr_rec.stat_flag, createStatusFlag);
		strcpy (cohr_rec.cont_no, local_rec.cont_no);
		cc = abc_add (cohr, &cohr_rec);
		if (cc) 
			file_err (cc, "cohr", "DBADD");

		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "cohr", "DBFIND");

		abc_unlock (esmr);

		print_at (0, 0, ML (mlSoMess130));
		sprintf (err_str, ML (mlSoMess131) , cohr_rec.inv_no);
		print_at (0, 0, ML (mlSoMess132));
		PauseForKey (1, 0, err_str, 0);
	}
	else
		print_at (0, 0, ML (mlStdMess035));

	if (CCN_CONF)
	{

		/*
		 * Is invoice number to come from department of branch.
		 */
		if (envVar.invoiceNumbers == BY_DEPART)
		{
			strcpy (cudp_rec.co_no, comm_rec.co_no);
			strcpy (cudp_rec.br_no, comm_rec.est_no);
			strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
			cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "cudp", "DBFIND");

			inv_no	=	cudp_rec.nx_crd_no;
			inv_no++;
		}
		else
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "esmr", "DBFIND");
		
			inv_no	=	esmr_rec.nx_crd_nte_no;
			inv_no++;
		}

		if (envVar.invoiceNumbers == BY_BRANCH)
			strcpy (tmp_prefix, esmr_rec.crd_pref);
		else
			strcpy (tmp_prefix, cudp_rec.crd_pref);

		clip (tmp_prefix);
		len = strlen (tmp_prefix);

		sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
		sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no);

		/*
		 * Check if Invoice / Credit Note No Already
		 * Allocated. If it has been then skip		
		 */
		while (CheckForCredit (tmp_inv_no) == 0)
			sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no++);

		if (envVar.invoiceNumbers == BY_DEPART)
		{
			cudp_rec.nx_crd_no	=	inv_no;

			cc = abc_update (cudp, &cudp_rec);
			if (cc)
				file_err (cc, "cudp", "DBUPDATE");
		}
		else
		{
			esmr_rec.nx_crd_nte_no	=	inv_no;

			cc = abc_update (esmr, &esmr_rec);
			if (cc)
				file_err (cc, "esmr", "DBUPDATE");
		}
		sprintf (cohr_rec.inv_no, "%-8.8s", tmp_inv_no);
		strcpy (cohr_rec.type, "C");
	}
	abc_selfield (inmr, "inmr_hhbr_hash");

	scn_set (ITEM_SCN);
	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++) 
	{
		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		new_coln = find_rec (coln, &coln_rec, COMPARISON, "u");

		getval (line_cnt);
		if (coln_rec.crd_type [0] == 'D')
		{
			SR.qtyReturn = 1.00;
			local_rec.qtyReturn = 1.00;
		}

		CalculateLineTotal (line_cnt);

		cohr_rec.gross 	+= lineGross;
		cohr_rec.disc 	+= lineDisc;
		cohr_rec.tax 	+= lineTax;
		cohr_rec.gst 	+= lineGst;

		if (SR.creditType [0] == 'D')
		{
	    	cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo, "C", "S");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}
		else
			FreeInsf (line_cnt, SR.serialNo);

		inmr_rec.hhbr_hash = SR.hhbrHash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;

		if (inmr_rec.hhbr_hash == coln_rec.hhbr_hash)
			if (incc_rec.hhcc_hash != coln_rec.incc_hash)
				incc_rec.hhcc_hash = coln_rec.incc_hash;

		coln_rec.hhco_hash 	 	= 	cohr_rec.hhco_hash;
		coln_rec.line_no 	 	= 	line_cnt;
		coln_rec.hhbr_hash 	 	= 	SR.hhbrHash;
		coln_rec.incc_hash 	 	= 	incc_rec.hhcc_hash;
	   	coln_rec.q_order  	 	= 	ToStdUom (local_rec.qtyReturn);
	   	coln_rec.q_backorder 	= 	0.00;
		coln_rec.gst_pc 	 	= 	 (float) ((notax) ? 0.00 : SR.gstPc);
		coln_rec.tax_pc		 	= 	 (float) ((notax) ? 0.00 : SR.taxPc);
		coln_rec.cost_price  	= 	SR.costPrice;
		coln_rec.reg_pc			= 	SR.regPc;
		coln_rec.disc_a			= 	SR.discA_Pc;
		coln_rec.disc_b			= 	SR.discB_Pc;
		coln_rec.disc_c			= 	SR.discC_Pc;
		coln_rec.cumulative		= 	SR.cumulative;
		if (SR.cnvFct == 0.00)
			SR.cnvFct = 1.00;
		coln_rec.gsale_price	= 	SR.grossSalePrice / SR.cnvFct;
		coln_rec.sale_price		= 	SR.salePrice / SR.cnvFct;
		coln_rec.cont_status	= 	SR.contractStat;
		coln_rec.gross	 	 	= 	lineGross;
		coln_rec.amt_disc	 	= 	lineDisc;
		coln_rec.amt_tax	 	= 	lineTax;
		coln_rec.amt_gst	 	= 	lineGst;

		coln_rec.due_date	 	= 	 (envVar.useSystemDate) ? local_rec.lsystemDate
											   : comm_rec.dbt_date;

		coln_rec.hhum_hash		=	SR.hhumHash;

		strcpy (coln_rec.serial_no	, 	SR.serialNo);
		strcpy (coln_rec.cus_ord_ref	, 	SR.cusOrdRef);
		strcpy (coln_rec.bonus_flag	, 	 (BONUS) ? "Y" : "N");
		strcpy (coln_rec.stat_flag	, 	createStatusFlag);
		strcpy (coln_rec.status		, 	"C");

		if (!new_coln)
		{
			/*
			 * Update existing order.
			 */
			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			cc = abc_update (coln, &coln_rec);
			if (cc) 
				file_err (cc, "coln", "DBUPDATE");

		}
		else 
		{
			coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
			coln_rec.line_no 	= line_cnt;
			coln_rec.disc_pc 	= ScreenDisc (coln_rec.disc_pc);
			strcpy (coln_rec.item_desc, inmr_rec.description);
			cc = abc_add (coln, &coln_rec);
			if (cc) 
				file_err (cc, "coln", "DBADD");

			coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
			coln_rec.line_no 	= line_cnt;
			cc = find_rec (coln, &coln_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "coln", "DBFIND");

		}
		if ((SK_BATCH_CONT || MULT_LOC) && FLD ("LL") != ND)

		{
			AllocLotLocation 
			 (
				line_cnt, 
				FALSE, 
				LL_LOAD_CRD, 
				coln_rec.hhcl_hash
			);
		}
	}

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = line_cnt;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		FreeInsf (coln_rec.line_no, coln_rec.serial_no);
		cc = abc_delete (coln);
		if (cc) 
			file_err (cc, "coln", "DBDELETE");

		cc = find_rec (coln, &coln_rec, GTEQ, "u");
	}
	abc_unlock (coln);

	/*
	 * Update existing order header.
	 */
	if (!newCCN) 
	{	
		/*
		 * Delete cancelled order.
		 */
		if (lcount [ITEM_SCN] == 0) 
		{
			print_at (0, 0, ML (mlSoMess133));

			cc = abc_delete (cohr);
			if (cc)
				file_err (cc, "cohr", "DBDELETE");
		}
		else
			print_at (0, 0, ML (mlSoMess090));
	}
	else
		print_at (0, 0, ML (mlSoMess063));

	/*
	 * Calc Totals of Gst etc for cohr
	 */
	CalculateExtendedTotal (FALSE);
	cohr_rec.date_required = local_rec.lsystemDate;

	if (lcount [ITEM_SCN] != 0) 
	{
		cc = abc_update (cohr, &cohr_rec);
		if (cc)
			file_err (cc, "cohr", "DBUPDATE");

	}
	abc_unlock (cohr);

	if (cohr_rec.del_req [0] == 'Y' && newCCN)
	{
		sprintf (err_str, "tr_trsh_mnt C %010ld", cohr_rec.hhco_hash);
		sys_exec (err_str);
	}

	if (AUTO_SK_UP)
		AddSobg (0, "SU", cohr_rec.hhco_hash);

	strcpy (local_rec.prev_inv_no, cohr_rec.inv_no);
	strcpy (local_rec.prev_dbt_no, cumr_rec.dbt_no);

	return;
}

int
CheckForCredit (
 char	*inv_no)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type, "C");
	sprintf (cohr2_rec.inv_no, "%-8.8s", inv_no);
	return (find_rec (cohr2, &cohr2_rec, COMPARISON, "r"));
}
int
CheckForCCN (
 void)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type, "C");
	sprintf (err_str, "N%07ld", ++esmr_rec.nx_crd_nte_no);
	sprintf (cohr2_rec.inv_no, "%-8.8s", err_str);
	return (find_rec (cohr2, &cohr2_rec, COMPARISON, "r"));
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
		scn_set (ITEM_SCN);

		crdTotalAmt = 0.00;

		cohr_rec.gross = 0.00;
		cohr_rec.disc = 0.00;
		cohr_rec.tax = 0.00;
		cohr_rec.gst = 0.00;

		for (i = 0;i < lcount [ITEM_SCN];i++)
		{
			getval (i);
			CalculateLineTotal (i);
			cohr_rec.gross 	+= lineGross;
			cohr_rec.disc 	+= lineDisc;
			cohr_rec.tax 	+= lineTax;
			cohr_rec.gst 	+= lineGst;
		}
	}

	if (notax)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	value = cohr_rec.freight + 
			cohr_rec.sos + 
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3;

	wk_value *= value;
	cohr_rec.gst += wk_value;
	
	cohr_rec.gst = (cohr_rec.gst);
	
	if (envVar.dbNettUsed)
	{
		crdTotalAmt = 	cohr_rec.gross + 
						cohr_rec.tax + 
						cohr_rec.freight +
						cohr_rec.sos +
						cohr_rec.gst - 
						cohr_rec.disc - 
						cohr_rec.deposit -
						cohr_rec.ex_disc + 
						cohr_rec.insurance +
						cohr_rec.other_cost_1 + 
						cohr_rec.other_cost_2 +
						cohr_rec.other_cost_3;
 	}
	else
	{
		crdTotalAmt = 	cohr_rec.gross + 
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
	}
}

void
CalculateLineTotal (
 int	line_no)
{
	if (NON_STOCK (line_no))
	{
		store [line_no].qtyReturn = 0.00;
		store [line_no].salePrice = 0.00;
		store [line_no].costPrice = 0.00;
		store [line_no].outerSize = 0.00;
		store [line_no].discPc = 0.00;
		store [line_no].taxPc = 0.00;
		store [line_no].taxAmount = 0.00;
		store [line_no].gstPc = 0.00;
	}
	
	/*
	 * Update coln gross tax and disc for each line. 
	 */
	lineGross = (double) store [line_no].qtyReturn;
	lineGross *= out_cost (store [line_no].actSalePrice, store [line_no].outerSize);
	lineGross = no_dec (lineGross);

	if (notax)
		lineTaxAmt = 0.00;
	else
	{
		lineTaxAmt = (double) store [line_no].qtyReturn;
		lineTaxAmt *= out_cost (store [line_no].taxAmount, store [line_no].outerSize);

		lineTaxAmt = no_dec (lineTaxAmt);
	}

	lineDisc = (double) (store [line_no].discPc);
	lineDisc = DOLLARS (lineDisc);
	lineDisc *= lineGross;
	lineDisc = no_dec (lineDisc);

	if (notax)
		lineTax = 0.00;
	else
	{
		lineTax = (double) (store [line_no].taxPc);
		lineTax = DOLLARS (lineTax);

		if (cumr_rec.tax_code [0] == 'D')
			lineTax *= lineTaxAmt;
		else
		{
			if (envVar.dbNettUsed)
				lineTax *= (lineGross - lineDisc);
			else
				lineTax *= lineGross;
		}
		lineTax = (lineTax);
	}
	if (store [line_no].creditType [0] == 'R' && RESTOCK_FEE)
	{
		lineRestock = (double) comr_rec.restock_pc;
		lineRestock = DOLLARS (lineRestock);
		if (envVar.dbNettUsed)
			lineRestock *= (lineGross - lineDisc);
		else
			lineRestock *= (lineGross);
	}
	else
		lineRestock = 0.00;

	if (notax)
		lineGst = 0.00;
	else
	{
		lineGst = (double) (store [line_no].gstPc);
		if (envVar.dbNettUsed)
			lineGst *= ((lineGross - lineDisc) + lineTax);
		else
			lineGst *= (lineGross + lineTax);
		lineGst = DOLLARS (lineGst);
	}
	
	store [line_no].extendedTotal = lineGross;
	store [line_no].extendedTotal -= lineDisc;
	store [line_no].extendedTotal += lineTax;
}

/*
 *	Free insf record
 */
void
FreeInsf (
 int	line_no, 
 char	*serialNo)
{
	if (!strcmp (serialNo, ser_space))
		return;

	/*
	 * serial_item and serial number input
	 */
	if (SERIAL)
	{

	    cc = UpdateInsf (store [line_no].hhwhHash, 0L, serialNo, "C", "F");
		if (cc)
			cc = UpdateInsf (store [line_no].hhwhHash, 0L, serialNo, "S", "F");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}

void
SrchPaymentTerms (
 void)
{
	int		i = 0;
	_work_open (3, 0, 40);
	save_rec ("#Cde", "#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode, p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

void
SrchSellTerms (
 void)
{
	int		i = 0;
	_work_open (3, 0, 40);
	save_rec ("#Cde", "#Selling Terms ");

	for (i = 0;strlen (s_terms [i]._scode);i++)
	{
		cc = save_rec (s_terms [i]._scode, s_terms [i]._sterm);
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
	_work_open (1, 0, 40);
	save_rec ("# ", "#Price ");

	for (i = 0;i < 9;i++)
	{
		sprintf (err_str, "%d", i + 1);
		cc = save_rec (err_str, GetPriceDesc (i));
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search for salesman.
 */
void
SrchExsf (
 char	*key_val)
{
	_work_open (2, 0, 40);
	save_rec ("#No", "#Salesperson Description.");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*
 * Search for area.
 */
void
SrchExaf (
 char	*key_val)
{
	_work_open (2, 0, 40);
	save_rec ("#No", "#Area Description");
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", key_val);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strcmp (exaf_rec.co_no, comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code, key_val, strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", key_val);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

/*
 * Search on UOM (inum)    
 */
void
SrchInum (
 char    *key_val)
{
	char	uom_group [21];

	_work_open (3, 0, 40);
	save_rec ("#UOM ", "#Description");
	abc_selfield (inum, "inum_hhum_hash");
	inum_rec.hhum_hash = inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
	if (!cc)
	{
		strcpy (uom_group, inum_rec.uom_group);
	}

	abc_selfield (inum2, "inum_id_no2");	
	strcpy (inum2_rec.uom_group, uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{                        
		if (strncmp (inum2_rec.uom_group, key_val, strlen (key_val)))
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

	strcpy (inum2_rec.uom_group, uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, inum2, "DBFIND");
}

void
SrchCohr (
 char	*key_val)
{
	_work_open (8, 0, 40);
	save_rec ("#CCN No", "#App Inv, Reason for Credit");
	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, "N");
	sprintf (cohr_rec.inv_no, "%-8.8s", key_val);

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
	 	      !strcmp (cohr_rec.br_no, comm_rec.est_no) && 
		      cohr_rec.type [0] == 'N' && 
		      !strncmp (cohr_rec.inv_no, key_val, strlen (key_val)))
	{
		//if (cohr_rec.ccn_print [0] == 'N' && 
		if (((cohr_rec.stat_flag [0] == createStatusFlag [0] &&
			CCN_INPUT) || CCN_DISPLAY) && 
			cumr_rec.hhcu_hash == cohr_rec.hhcu_hash)
		{
			sprintf (err_str, "%s  %s", cohr_rec.app_inv_no, cohr_rec.cus_ord_ref);
			cc = save_rec (cohr_rec.inv_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, "N");
	sprintf (cohr_rec.inv_no, "%-8.8s", temp_str);

	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cohr", "DBFIND");
}

void
SrchCohr2 (
 char	*key_val)
{
	_work_open (8, 0, 20);
	save_rec ("#Inv No.", "#Customer Order Ref.");
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type, "I");
	sprintf (cohr2_rec.inv_no, "%-8.8s", key_val);
	cc = find_rec (cohr2, &cohr2_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr2_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cohr2_rec.br_no, comm_rec.est_no) && 
		       cohr2_rec.type [0] == 'I' && 
		      !strncmp (cohr2_rec.inv_no, key_val, strlen (key_val)))
	{
		if (cumr_rec.hhcu_hash == cohr2_rec.hhcu_hash)
		{
			cc = save_rec (cohr2_rec.inv_no, 
				       cohr2_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (cohr2, &cohr2_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type, "I");
	sprintf (cohr2_rec.inv_no, "%-8.8s", temp_str);

	cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cohr2", "DBFIND");
}

void
SrchCucr (
 void)
{
	_work_open (2, 0, 40);
	save_rec ("# ", "#Reason for Credit");
	strcpy (cucr_rec.co_no, comm_rec.co_no);
	strcpy (cucr_rec.code, " ");
	cc = find_rec (cucr, &cucr_rec, GTEQ, "r");

	while (!cc && !strcmp (cucr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cucr_rec.code, cucr_rec.desc);
		if (cc)
			break;
		cc = find_rec (cucr, &cucr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cucr_rec.co_no, comm_rec.co_no);
	sprintf (cucr_rec.code, "%-1.1s", temp_str);
	cc = find_rec (cucr, &cucr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cucr", "DBFIND");
}

/*
 * Search for Zome Master. 
 */
void
SrchTrzm (
 char *key_val)
{
	_work_open (6, 0, 40);

	save_rec ("#Zone. ", "#Zone Description");

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
		file_err (cc, "trzm", "DBFIND");

	return;
}
/*
 * Search for carrier code. 
 */
void
SrchTrcm (
 char	*key_val)
{
	char	key_string [31];
	long 	currentZoneHash	=	trzm_rec.trzm_hash;

	_work_open (20, 11, 50);

	save_rec ("#Carrier", "# Rate Kg. | Carrier Name.");
	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) && 
		      	  !strncmp (trcm_rec.carr_code, key_val, strlen (key_val)))
	{
		trcl_rec.trcm_hash	=	trcm_rec.trcm_hash;
		trcl_rec.trzm_hash	=	0L;
		cc = find_rec (trcl, &trcl_rec, GTEQ, "r");
		while (!cc && trcl_rec.trcm_hash == trcm_rec.trcm_hash)
		{
	
			if (currentZoneHash	> 0L && trcl_rec.trzm_hash != currentZoneHash)
			{
				cc = find_rec (trcl, &trcl_rec, NEXT, "r");
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
			cc = find_rec (trcl, &trcl_rec, NEXT, "r");
		}
		cc = find_rec (trcm, &trcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "trcm", "DBFIND");

	trzm_rec.trzm_hash	=	atol (temp_str + 12);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (!cc)
		sprintf (local_rec.defaultDelZone, "%-6.6s", trzm_rec.del_zone);

}
/*
 * Check Whether A Serial Number For This Item Number
 * Has Already Been Used.							
 * Return 1 if duplicate						
 */
int
CheckDuplicateInsf (
	char	*serial_no, 
	long	hhbrHash, 
	int		line_no)
{
	int		i;

	if (MULT_QTY)
		return (SUCCESS);

	for (i = 0;i < lcount [ITEM_SCN];i++)
	{
		/*
		 * Ignore Current Line
		 */
		if (i == line_no)
			continue;

		/*
		 * Only compare serial numbers for the same item number.
		 */
		if (store [i].hhsiHash == hhbrHash)
		{
			if (!strcmp (store [i].serialNo, serial_no))
				return (RETURN_ERR);
		}
	}
	return (SUCCESS);
}

/*
 * Routine to check if hidden flag has been set (this is indicated by a 
 * '/H' on the end of the part number. If hidden flag is set then '/H' 
 * is removed from part number.                                       
 * Returns 0 if hidden flag has not been set, 1 if it has.           
 */
int
CheckHiddenLine (
	char	*item_no)
{
	char	hidden_item [17];
	char	*sptr;

	sprintf (hidden_item, "%-16.16s", item_no);
	sptr = clip (hidden_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [2] && * (sptr + 1) == envVar.soSpecial [3])
		{
			*sptr = '\0';
			sprintf (item_no, "%-16.16s", hidden_item);
			if (cohr_rec.prt_price [0] == 'Y')
			{
				print_mess (ML (mlSoMess055));
				sleep (sleepTime);
				clear_mess ();
				return (SUCCESS);
			}
			else
				return (RETURN_ERR);
		}
	}
	sprintf (item_no, "%-16.16s", hidden_item);
	return (SUCCESS);
}

/*
 * Routine to check if bonus flag has been set (this is indicated by a 
 * '/B' on the end of the part number. If bonus flag is set then '/B' 
 * is removed from part number.                                      
 * Returns 0 if bonus flag has not been set, 1 if it has.           
 */
int
CheckBonusLine (
	char	*item_no)
{
	char	bonus_item [17];
	char	*sptr;

	sprintf (bonus_item, "%-16.16s", item_no);
	sptr = clip (bonus_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.soSpecial [0]  && * (sptr + 1) == envVar.soSpecial [1])
		{
			*sptr = '\0';
			sprintf (item_no, "%-16.16s", bonus_item);
			return (RETURN_ERR);
		}
	}
	sprintf (item_no, "%-16.16s", bonus_item);
	return (SUCCESS);
}

/*
 * Add Warhouse Record for Current
 */
int
AddIncc (void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	incc_rec.hhwh_hash = 0L;
	sprintf (incc_rec.sort, "%s%11.11s%-16.16s", 
			inmr_rec.inmr_class, 
			inmr_rec.category, 
			inmr_rec.item_no);
				
	incc_rec.closing_stock = 0.00;

	incc_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc_rec.stat_flag, "0");
	
	cc = abc_add (incc, &incc_rec);
	if (cc) 
		return (RETURN_ERR);

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	return (cc);
}

void
BusyFunction (
 int	flip)
{
	print_at (2, 1, "%-60.60s", (flip) ? ML (mlStdMess035) : " ");
}

void
PrintCompanyDetails (
 void)
{
	print_at (22, 0, ML (mlStdMess038), 
						comm_rec.co_no, comm_rec.co_short);
	print_at (22, 30, ML (mlStdMess039), 
						comm_rec.est_no, comm_rec.est_short);
	print_at (22, 60, ML (mlStdMess099), 
						comm_rec.cc_no, comm_rec.cc_name);
	print_at (22, 100, ML (mlStdMess085), 
						cudp_rec.dp_no, cudp_rec.dp_short);

}

void
CalculateInputTotal (
 void)
{
	int		i;
	int		no_lines = (!loaded  && !ins_flag && prog_status == ENTRY) ? 
				line_cnt : lcount [ITEM_SCN] - 1;

	double		other = 0.00;
	double		wk_gst = 0.00;

	totalCredit 	= 0.00;
	totalDisc 		= 0.00;
	totalTax 		= 0.00;
	totalGst 		= 0.00;
	totalRestock 	= 0.00;

	for (i = 0;i <= no_lines;i++) 
	{
		CalculateLineTotal (i);

		totalCredit 	+= lineGross;
		totalDisc 		+= lineDisc;
		totalTax 		+= lineTax;
		totalGst 		+= lineGst;
		totalRestock 	-= lineRestock;
	}
	if (RESTOCK_FEE && newCCN)
		cohr_rec.other_cost_3 = (totalRestock);

	totalGst = (totalGst);

	other = cohr_rec.freight + 	
			cohr_rec.sos +
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 	
			cohr_rec.other_cost_3;

	if (notax)
		wk_gst = 0.00;
	else
		wk_gst = (double) (comm_rec.gst_rate / 100.00);

	wk_gst *= other;
	totalGst += wk_gst;
	totalGst = (totalGst);

	PrintTotalBoxValues ();
}

void
DrawTotals (
 void)
{
	box (96, 1, 35, 5);
	print_at (2, 97, ML (mlSoMess064));
	print_at (3, 97, ML (mlSoMess065));

	if (envVar.gstApplies)
		print_at (4, 97, ML (mlSoMess066), envVar.gstCode);
	else
		print_at (4, 97, ML (mlSoMess066), "Tax");

	line_at (5, 97, 34);
	print_at (6, 97, ML (mlSoMess068));

	PrintTotalBoxValues ();
}

void
PrintTotalBoxValues (
 void)
{
	double	f_other = 0.00;
	double tot_tot = 0.00;
	
	f_other = cohr_rec.freight + 	
			  cohr_rec.sos +
			  cohr_rec.insurance -
			  cohr_rec.ex_disc + 
			  cohr_rec.other_cost_1 +
			  cohr_rec.other_cost_2 + 	
			  cohr_rec.other_cost_3;


	if (envVar.dbNettUsed)
		tot_tot = (totalCredit - totalDisc + totalTax + totalGst + f_other);
	else
		tot_tot = (totalCredit + totalTax + totalGst + f_other);
	
	if (envVar.dbNettUsed)
		print_at (2, 115, "%14.2f", DOLLARS (totalCredit - totalDisc));
	else
		print_at (2, 115, "%14.2f", DOLLARS (totalCredit));

	print_at (3, 115, "%14.2f", DOLLARS (f_other));
	print_at (4, 115, "%14.2f", DOLLARS (totalGst + totalTax));
	print_at (6, 115, "%14.2f", DOLLARS (tot_tot));
}

/*
 * Routine to add item at warehouse level if not already there.
 */
int
AddInsf (
	long	hhwhHash, 
	long	hhbrHash, 
	char	*serialNo, 
	long	dateIn)
{
	double	cost;

	print_at (2, 1, "%-50.50s", " ");
	print_at (2, 1, ML (mlSoMess238));
	cost = getnum (20, 2, MONEYTYPE, "NNNNNNN.NN");
	BusyFunction (BUSY_OFF);
	
	insfRec.hhwh_hash = hhwhHash;
	insfRec.hhbr_hash = hhbrHash;
	strcpy (insfRec.status, "C");
	strcpy (insfRec.receipted, "Y");
	strcpy (insfRec.serial_no, serialNo);
	insfRec.date_in 	= dateIn;
	insfRec.est_cost 	= cost;
	strcpy (insfRec.stat_flag, "E");

	cc = abc_add (insf, &insfRec);
	if (cc) 
		file_err (cc, insf, "DBADD");

    return (EXIT_SUCCESS);
}
									   
						   			  
/*
 * Routine to remove item at warehouse level. 
 */
void
RemoveInsf (
	long	hhwhHash, 
	char	*serialNo)
{
	insfRec.hhwh_hash = hhwhHash;
	strcpy (insfRec.status, "C");
	strcpy (insfRec.serial_no, 	serialNo);

	cc = find_rec (insf, &insfRec, EQUAL, "u");
	if (!cc)
	{
		cc = abc_delete (insf);
		if (cc) 
			file_err (cc, insf, "DBDELETE");
	}
}

int
win_function2 (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	int		i;

	if (scn != 2)
		return (FALSE);

	if (store [lin].hhbrHash == 0L)
		return (FALSE);
	
	if (prog_status == ENTRY)
		return (FALSE);

	/*
	 * Check for contract.
	 */
	if (store [lin].contractStat)
	{
		print_mess (ML (mlSoMess239));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	/*
	 * Disable edit of qty BO field.
	 */
	negoScn [1].fldEdit = 0;
	negoScn [10].fldEdit = 0;
	negoScn [11].fldEdit = 0;

	/*
	 * Initialise values for negotiation window.
	 */
	negoRec.qOrd			=	store [lin].qtyReturn;
	negoRec.qBord			=	0.00;

	negoRec.regPc			=  	store [lin].regPc;
	negoRec.discArray [0]	=	store [lin].discA_Pc;
	negoRec.discArray [1]	=	store [lin].discB_Pc;
	negoRec.discArray [2]	=	store [lin].discC_Pc;
	negoRec.grossPrice		=	store [lin].grossSalePrice;
	negoRec.salePrice		=	store [lin].salePrice;
	negoRec.margCost		=	store [lin].costPrice;
	negoRec.outer_size		=	store [lin].outerSize;

	NegPrice (2, 10, local_rec.item_no, coln_rec.item_desc, 
				   store [lin].cumulative, scn);

	/*
	 * Clear space where negotiation window was.
	 */
	for (i = 10; i < 15; i++)
	{
		move (1, i);
		cl_line ();
	}
	if (!restart)
	{
		local_rec.qtyReturn 			=   negoRec.qOrd;

		store [lin].qtyReturn		=	negoRec.qOrd;
		store [lin].regPc		= 	negoRec.regPc;
		store [lin].discA_Pc		= 	negoRec.discArray [0];
		store [lin].discB_Pc		= 	negoRec.discArray [1];
		store [lin].discC_Pc		= 	negoRec.discArray [2];
		store [lin].discPc		=	CalcOneDisc (store [lin].cumulative, 
													 negoRec.discArray [0], 
													 negoRec.discArray [1], 
													 negoRec.discArray [2]);
		store [lin].grossSalePrice 	= 	negoRec.grossPrice;
		store [lin].salePrice	=	negoRec.salePrice;
		store [lin].actSalePrice		=	negoRec.salePrice;

		coln_rec.disc_pc  			= 	ScreenDisc (store [lin].discPc);
		coln_rec.sale_price 		= 	store [lin].salePrice;

		if (store [lin].calcSalePrice != coln_rec.sale_price)
			strcpy (store [lin].priOverride, "Y");

		if (store [lin].calcDisc != ScreenDisc (coln_rec.disc_pc))
			strcpy (store [lin].disOverride, "Y");

		PrintExtend (lin);

		putval (lin);
	}

	CalculateInputTotal ();
	
	restart = FALSE;

	return (TRUE);
}

int
LoadDisplay (
 char *_run_string)
{
	long	hhchHash;
	char	_inv_no [9];

	hhchHash = atol (_run_string + 8);
	sprintf (_inv_no, "%-8.8s", _run_string + 19);

	abc_selfield (cumr, "cumr_hhcu_hash");

	cumr_rec.hhcu_hash = hhchHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		abc_selfield (cumr, (envVar.dbFind) ? "cumr_id_no3" : "cumr_id_no");
		return (RETURN_ERR);
	}

	strcpy (local_rec.cust_no, cumr_rec.dbt_no);
	/*
	 * Check if invoice is on file.
 	 */
	sprintf (cohr_rec.co_no, "%-2.2s", _run_string);
	sprintf (cohr_rec.br_no, "%-2.2s", _run_string + 3);
	sprintf (cohr_rec.type, "%-1.1s", _run_string + 6);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cohr_rec.inv_no, zero_pad (_inv_no, 8));
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		return (RETURN_ERR);

	if (LoadCreditNote (cohr_rec.hhco_hash))
		return (RETURN_ERR);

	strcpy (local_rec.spinst [0], cohr_rec.din_1);
	strcpy (local_rec.spinst [1], cohr_rec.din_2);
	strcpy (local_rec.spinst [2], cohr_rec.din_3);
	newCCN = 0;
	entry_exit = 1;

	SetCreditNote (newCCN);

	return (SUCCESS);
}
/*
 * Gets posting status description
 */
char *
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
#endif	/* GVISION */

	sprintf (filename, "%s/BIN/Post_Status", (sptr != (char *)0) 
							? sptr : "/usr/LS10.5");

	if ((fin = fopen (filename, "r")) == 0)
	{
		sprintf (description, "%90.90s", " ");
		return (description);
	}

	sptr = fgets (Data, 150, fin);
	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';

		/*
		 * Look for end of status
		 */
		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		if (*tptr)
		{
			*tptr = '\0';

			/*
			 * Grab printer name
			 */
			if (strchr (sptr, stat [0]))
			{
				fclose (fin);
				sprintf (description, "%-90.90s", ++tptr);
				return (description);
			}
		}
		sptr = fgets (Data, 150, fin);
	}
	fclose (fin);
	sprintf (description, "%90.90s", " ");
	return (description);
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

	if (cur_screen == 1)
	{
		move (21, 3); line (90);
	}
	/*
	 * Only do anything when we are on screen 1 and
	 * we've read a valid cumr.					
	 */
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

	if (FindCucc (_key, last_hhcu))
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
 int _key, 
 long last_hhcu)
{
	if (_key == 0)
	{
		cc = find_rec (cucc, &cucc_rec, COMPARISON, "r");
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (SUCCESS);

		return (RETURN_ERR);
	}

	if (last_hhcu != 0L)
	{
		/*
		 * Find the NEXT / PREVIOUS record to the current one	
		 */
		cc = find_rec (cucc, &cucc_rec, (_key == FN14) ? GREATER : LT, "r");

		/*
		 * Woops, looks like we need to loop around
		 */
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (SUCCESS);
	}

	/*
	 * Finding Next
	 */
	if (_key == FN14)
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = 0L;
		cc = find_rec (cucc, &cucc_rec, GTEQ, "r");
	}
	else
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash + 1L;
		cucc_rec.record_no = 0L;

		cc = find_rec (cucc, &cucc_rec, GTEQ, "r");

		/*
		 * Probably the last hhcu group in the cucc
		 * so find the last record in the file.	
		 */
		if (cc)
			cc = find_rec (cucc, &cucc_rec, LAST, "r");
		else
			cc = find_rec (cucc, &cucc_rec, PREVIOUS, "r");
	}

	if (cc || cucc_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (RETURN_ERR);
	else
		return (SUCCESS);
}
/*
 * Add record to background processing file. 
 */
void
AddSobg (
 int _lpno, 
 char *_type_flag, 
 long _hash)
{
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
	
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _type_flag);
	sobg_rec.lpno = _lpno;
	sobg_rec.hash = _hash;

	cc = find_rec ("sobg", &sobg_rec, COMPARISON, "r");
	/*
	 * Add the record if an identical one doesn't already exist    
	 */
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _type_flag);
		sobg_rec.lpno = _lpno;
		sobg_rec.hash = _hash;

		cc = abc_add ("sobg", &sobg_rec);
		if (cc)
			file_err (cc, "sobg", "DBADD");

	}
	abc_fclose (sobg);
}

/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (
 void)
{
	char	*sptr;

	sprintf (envVar.automaticStockUpdate, "%-1.1s", get_env ("AUTO_SK_UP"));

	/*
	 * Check for Currency Code. 
	 */
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
	 * Check for Automatic freight charge.
	 */
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

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

	/*
	 * Get gst code.
	 */
	if (envVar.gstApplies)
		sprintf (envVar.gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));

	sprintf (envVar.manualPrint, "%-1.1s", get_env ("SO_MAN_PRINT"));

	sptr = chk_env ("SO_OTHER_1");
	sprintf (envVar.other [0], "%.30s", 
					 (sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (envVar.other [1], "%.30s", 
					 (sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (envVar.other [2], "%.30s", 
					 (sptr == (char *)0) ? "Other Costs." : sptr);

	sprintf (envVar.normalOther, (sptr == (char *)0) ? "Other Costs." : sptr);

	/*
	 * Check if special codes for bonus and hidden lines are used.
	 */
	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVar.soSpecial, "/B/H");
	else
		sprintf (envVar.soSpecial, "%-4.4s", sptr);

	/*
	 * Check if nett pricing is used.
	 */
	envVar.dbNettUsed = FALSE;

	sptr = chk_env ("CN_NETT_USED");
	if (sptr == (char *)0)
	{
		sptr = chk_env ("DN_NETT_USED");
		envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);
	}
	else
		envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

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
	 * Check for multi-currency.
	 */
	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for discounts on Indent items.
	 */
	sptr = chk_env ("SO_DIS_INDENT");
	envVar.discountIndents = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_NUMBER");
	envVar.invoiceNumbers = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
     * Check and Get Order Date Type.
     */
	sptr = chk_env ("SO_DOI");
	envVar.useSystemDate = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	/*
	 * Check for Automatic freight charge.
	 */
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * How if Freight Charged. 
	 */
	sptr = chk_env ("SO_FREIGHT_CHG");
	envVar.soFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);

	/*
	 * Validate is serial items allowed.
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envVar.serialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);
}

/*
 * Reverse Screen Discount. 
 */
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

	if (F_HIDE (label ("uom")))
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

	if (F_HIDE (label ("uom")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.cnvFct;

	return (cnvQty);
}

int
NotAllAlloc (
 void)
{
	int		i;

	for (i = 0; i < lcount [ITEM_SCN]; i++)
	{
		if (SK_BATCH_CONT || MULT_LOC)
		{
			if (store [i].LL [0] != 'N')
			{
				sprintf (err_str, ML (mlSoMess240) , i + 1);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
		}
	}
	return (SUCCESS);
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

	cohr_rec.no_kgs		= 0.00;
	estimFreight 		= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	for (i = 0;i < lcount [ITEM_SCN];i++)
	{
		weight = (float) ((store [i].weight > 0.00) ? store [i].weight 
						    : comr_rec.frt_mweight);
		totalKgs += (weight * store [i].qtyReturn);
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

/*
 * Open Transport related files. 
 */
void
OpenTransportFiles (
 char	*ZoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, ZoneIndex);
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

	if (CCN_INPUT)
		strcpy (err_str, ML (mlSoMess134));

	if (CCN_MAINT)
		strcpy (err_str, ML (mlSoMess135));

	if (CCN_DISPLAY)
		strcpy (err_str, ML (mlSoMess136));

	if (CCN_CONF)
		strcpy (err_str, ML (mlSoMess137));
	
	rv_pr (err_str, (130 - strlen (err_str)) / 2, 0, 1);

	print_at (0, 100, ML (mlSoMess138) , local_rec.prev_dbt_no, 
					   local_rec.prev_inv_no);
	move (0, 1);
	line (132);

	switch (scn)
	{
	case	HEADER_SCN:
		use_window (0);
		inScreenFlag	=	FALSE;
		break;

	case	ITEM_SCN:
		inScreenFlag	=	TRUE;
		print_at (4, 0, ML (mlStdMess012) , cumr_rec.dbt_no, clip (cumr_rec.dbt_name));
		if (envVar.dbMcurr)
			print_at (4, 0, "     (%-3.3s)", cumr_rec.curr_code);


		DrawTotals ();
		CalculateInputTotal ();
		break;

	case FREIGHT_SCN:
		inScreenFlag	=	FALSE;
		print_at (2, 1, ML (mlStdMess012) , cumr_rec.dbt_no, 
										clip (cumr_rec.dbt_name));
		if (envVar.dbMcurr)
			print_at (2, 1, "     (%-3.3s)", cumr_rec.curr_code);
		break;
	}

	PrintCompanyDetails ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	if (specialDisplay)
		print_at (2, 1, "%R %s", GetPostingStatus (cohr_rec.stat_flag));

    return (EXIT_SUCCESS);
}

char *
NewStrStr (
 char* cStrData)
{
    char * cReturnValue = NULL;

    if (cStrData)
    {
        cReturnValue = (char*) malloc (strlen (cStrData) + 1);
        strcpy (cReturnValue, cStrData);
    }
    return (cReturnValue);
}

void 
SetSTermCode (
 int iPos, 
 char* cSCode)
{
    if (s_terms [iPos]._scode)
    {
        free (s_terms [iPos]._scode);
        s_terms [iPos]._scode = NULL;
    }

    s_terms [iPos]._scode = NewStrStr (cSCode);
}

void 
SetSTerm (
 int iPos, 
 char* cSCode, 
 char* cSTerm)
{
    SetSTermCode (iPos, cSCode);
    strncpy (s_terms [iPos]._sterm, cSTerm, 32);
}

void 
InitSTerms (
 void)
{
    int i;
    for (i = 0; i < 6; i++)
        s_terms [i]._scode = NULL;

	SetSTerm (0, "   ", "Local                 ");
	SetSTerm (1, "CIF", "Cost Insurance Freight");
	SetSTerm (2, "C&F", "Cost & Freight");
	SetSTerm (3, "FIS", "Free Into Store");
	SetSTerm (4, "FOB", "Free On Board");
	SetSTerm (5, "", "");
}

void 
DeleteSTerms (
 void)
{
    int i;
    for (i = 0; i < 6; i++)
    {
        if (s_terms [i]._scode)
        {
            free (s_terms [i]._scode);
            s_terms [i]._scode = NULL;
        }
    }
}

int
SrchCudi (
	int		indx)
{
	char	workString [170];

	_work_open (5, 0, 80);
	save_rec ("#DelNo", "#Delivery Details");
	cudi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{                        
		sprintf 
		(
			workString, "%s, %s, %s, %s", 
			clip (cudi_rec.name), 
			clip (cudi_rec.adr1), 
			clip (cudi_rec.adr2), 
			clip (cudi_rec.adr3)
		);
		sprintf (err_str, "%05d", cudi_rec.del_no);
		cc = save_rec (err_str, workString); 
		if (cc)
			break;

		cc = find_rec (cudi, &cudi_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (-1);

	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= atoi (temp_str);
	cc = find_rec (cudi, &cudi_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cudi, "DBFIND");

	switch (indx)
	{
	case	0:
		sprintf (temp_str, "%-40.40s", cudi_rec.name);
		break;

	case	1:
		sprintf (temp_str, "%-40.40s", cudi_rec.adr1);
		break;

	case	2:
		sprintf (temp_str, "%-40.40s", cudi_rec.adr2);
		break;

	case	3:
		sprintf (temp_str, "%-40.40s", cudi_rec.adr3);
		break;

	default:
		break;
	}
	return (cudi_rec.del_no);
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

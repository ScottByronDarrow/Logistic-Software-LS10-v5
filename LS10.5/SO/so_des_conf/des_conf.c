/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: des_conf.c,v 5.17 2002/11/28 04:09:50 scott Exp $
|  Program Name  : (so_des_conf.c)                                   |
|  Program Desc  : (Despatch Confirmation.                      )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 19/10/88         |
|---------------------------------------------------------------------|
| $Log: des_conf.c,v $
| Revision 5.17  2002/11/28 04:09:50  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.16  2002/07/24 08:39:25  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.15  2002/07/18 07:18:25  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.14  2002/06/20 07:16:00  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.13  2002/06/20 05:48:59  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.12  2002/04/30 07:56:48  scott
| Update for new Archive modifications;
|
| Revision 5.11  2002/04/29 07:47:14  scott
| Update for new Archive modifications;
|
| Revision 5.10  2002/01/23 08:17:06  scott
| S/C 00709 - Updated to fix lineup issue with department no.
|
| Revision 5.9  2002/01/23 06:02:08  scott
| Updated to not prompt at location if Indent, Phantom or non stock
|
| Revision 5.8  2001/10/23 07:16:39  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.7  2001/10/22 10:29:56  robert
| Updated to make rounding of discount consistent with so_input
|
| Revision 5.6  2001/09/27 03:02:45  scott
| Updated to add AllocationRestore () and AllocationComplete ()
| Fixed problem with allocation to lots being lost if restart performed.
| Updated to ensure allocation performed correctly.
|
| Revision 5.5  2001/08/28 06:04:29  scott
| Updated to fix currency code overwriting customer memory space
|
| Revision 5.4  2001/08/09 09:21:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/06 23:51:12  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: des_conf.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_des_conf/des_conf.c,v 5.17 2002/11/28 04:09:50 scott Exp $";

extern	int		SR_X_POS;
extern	int		SR_Y_POS;
extern	int		X_EALL;
extern	int		Y_EALL;

#define	MAIN_SCN	1
#define	ITEM_SCN	2
#define	FRI_SCN		3
#define	COST_SCN	4

#define MAXSCNS 	4
#define MAXWIDTH 	180
#define MAXLINES	500
#define	USE_WIN		1
#define	EXPORT		(cohr_rec.ord_type [0] == 'E')
#define	SR			store [line_cnt]
#define	LSR			store [lcount [ITEM_SCN]]
#define	SERIAL_ITEM	(SR.hhwhHash > 0L)
#define	BONUS		(SR._bonus [0] == 'Y')
#define	NO_COST		(SR._class [0] == 'N')
#define	NON_STOCK	(SR._class [0] == 'Z')
#define	PHANTOM		(SR._class [0] == 'P')
#define	CU_PRINT	(cumr_rec.reprint_inv [0] == 'Y')
#define	TRANSPORT	(cohr_rec.type [0] == 'T')
#define	INDENT		(SR._indent == FALSE)


#define	BO_OK		((SR._border [0] == 'Y' || SR._border [0] == 'F') && \
                          cumr_rec.bo_flag [0] == 'Y')

#define	FULL_BO		(SR._border [0] == 'F' && cumr_rec.bo_flag [0] == 'Y')

#define	SUR_CHARGE	(sohr_rec.sohr_new [0] == 'Y' && \
                       (cumr_rec.sur_flag [0] == 'Y' || \
                           cumr_rec.sur_flag [0] == 'y'))

#define	MULT_QTY	(SR._cost_flag [0] != 'S')
#define INMR_PRICE_SEARCH	1

#define AUTO_SK_UP	(createStatusFlag [0] == envVar.automaticStockUpdate [0])

#define	OVER_AMT(a,b,c)	(( (a)- (b))/ (c))

#define	FREIGHT_CHG	(cohr_rec.frei_req [0] == 'Y')
		
#define	SLEEP_TIME	2

#define	FGN_CURR	(envVar.dbMcurr && strcmp (cumr_rec.curr_code, envVar.currencyCode))

#define	MAX_SONS	10
#define	TXT_REQD

#include <pslscr.h>
#include <twodec.h>
#include <fcntl.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <ml_tr_mess.h>
#include <CustomerService.h>
#include <Archive.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cnchRecord	cnch_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct cudpRecord	cudp_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct insfRecord	insf_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct pocrRecord	pocrRec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln2_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct exsiRecord	exsi_rec;
struct cuccRecord	cucc_rec;
struct inalRecord	inal_rec;
struct cudiRecord	cudi_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;
struct trshRecord	trsh_rec;
struct trzmRecord	trzm_rec;
struct trchRecord	trch_rec;
struct sonsRecord	sons_rec;

/*-------------
| Table Names |
-------------*/
char	*data	= "data",
		*cumr2	= "cumr2",
		*incc2	= "incc2",
		*inum2	= "inum2",
		*soln2	= "soln2";

/*---------
| Globals |
---------*/
	int		FirstInv,					/* first time in main while		*/
			noTaxCharged,				/* notax for this debtor		*/
			numberOldLines		= 0,	/* #existing lines in order		*/
			nextLineNo 			= 0,	/* #existing lines in order		*/
			invoiceChanged 		= FALSE,/* invoice changed				*/
			hiddenLineFound 	= FALSE,
			inScreenFlag 		= FALSE;

	int		maxLinesUsed		=	50;
		
	int		LotSelectFlag;
	int		windowPipeOpen		=	FALSE,
			namedPipeFileNo,
			windowOk,
			screenDisplayed		=	FALSE;

	FILE	*pout;

	double	lineLevyAmt = 0.00,
			lineLevyPc  = 0.00,
			t_total 	= 0.00,
			l_total 	= 0.00,
			l_disc 		= 0.00,
			l_tax 		= 0.00,
			l_levy 		= 0.00,
			l_gst 		= 0.00,
			inv_tot 	= 0.00,
			f_other 	= 0.00,
			dis_tot 	= 0.00,
			tax_tot 	= 0.00,
			tot_tot 	= 0.00,
			levy_tot 	= 0.00,
			gst_tot 	= 0.00,
			est_freight = 0.00;

	float	realCommitted,
			realStock;

	char	branchNo [3],
			createStatusFlag [2],
			findStatusFlag [2];

	char	mlDesConf [50][101];

	long	des_date = 0L;

	char	*ser_space = "                         ",
			*sixteen_space = "                ",
			*ns_space = "                                        ",
			*forty_space = "                                        ";

	char	so_bonus [3];
	char	so_hide [3];
	char	*curr_user;

	char	HeaderDesc [MAX_SONS + 1][41];
	struct	storeRec {
		char	nsDesc [MAX_SONS + 1][41]; 	/*Non stock description.    */
		char	_desc2 [41]; 		/* Second Item Description          */
		long 	hhbrHash;			/* inmr_hhbr_hash					*/
		long 	actHhwhHash;		/* incc_hhwh_hash					*/
		long 	hhwhHash;			/* incc_hhwh_hash 0L for non serial	*/
		long 	hhsiHash;			/* inmr_hhsi_hash					*/
		long 	hhccHash;			/* incc_hhcc_hash					*/
		long 	hhahHash;			/* Link to lot control file.		*/
		long 	hhumHash;			/* Link to lot inum file.			*/
		long 	hhslHash;			/* soln_hhsl_hash					*/
		long 	hhclHash;			/* coln_hhcl_hash					*/
									/*									*/
		float 	_cnv_fct;			/* Conversion factor.				*/
		float 	_std_cnv_fct;		/* Standard Conversion factor.		*/
		float 	_qty_des;			/* Quantity despatched.				*/
		float 	_qty_back;			/* Quantity backordered.			*/
		float	_gst_pc;			/* inmr_gst_pc.						*/
		float	_tax_pc;			/* inmr_tax_pc.						*/
		float	_dis_pc;			/* coln_disc_pc						*/
		float	_dflt_disc;			/* inmr_disc_pc						*/
		float	_reg_pc;			/* Regulatory percent.      		*/
		float	_disc_a;			/* Discount percent A.      		*/
		float	_disc_b;			/* Discount percent A.      		*/
		float	_disc_c;			/* Discount percent A.      		*/
		float	_weight;			/* inmr_weight.						*/
		float	_outer;				/* inmr_outer.						*/
									/*									*/
		double	_tax_amt;			/* inmr_tax_amount.					*/
		double	advertLevyAmt;		/* Levy amount.						*/
		double	advertLevyPc;		/* Levy percent.					*/
		double	_gsale_price;		/* gross sale price					*/
		double	_sale_price;		/* default sale price				*/
		double	_act_sale;			/* Actual sale price.				*/
		double	_marg_cost;			/* Cost price for margins.			*/
		double	itemLevy;			/* Item Levy.                		*/
									/*									*/
		int		_cumulative;		/* Cumulative 1 or 0 				*/
		int		_pricing_chk;		/* Set if pricing has been  		*/
									/* called for line.         		*/
		int		_con_price;			/* Contract price.					*/
									/* 1 = contract no discount.		*/
									/* 2 = contract discount ok.		*/
		int		_indent;			/* Indent.							*/
		int		_cont_status;		/* 0 = not a contact line.			*/
		int		_dec_pt;			/* inmr_dec_pt.						*/
									/*									*/
		char	_lot_ctrl [2];		/* inmr_lot_ctrl.					*/
		char	_sellgrp [7];		/* inmr_sellgrp						*/
		char	_serial [26];		/* Serial number.					*/
		char	_org_ser [26];		/* Origional Serial number.			*/
		char	_category [12];		/* Category.						*/
		char	_bonus [2];			/* Bonus item Y/N					*/
		char	_class [2];			/* inmr_class.						*/
		char	_border [2];			/* inmr_bo_flag.					*/
		char	_release [2];		/* inmr_bo_release.					*/
		char	_pack_size [6];		/* inmr_pack_size.					*/
		char	_cost_flag [2];		/* inmr_costing_flag.				*/
		char	_ser_item [2];		/* inmr_serial_flag					*/
		char	err_fnd [2];			/* Line error status.				*/
		char	_UOM [5];			/* Unit of Measure.  				*/
       	char    _uom_group [21];	/* UOM Group						*/
		char	_item_desc [41];
	} store [MAXLINES];

    typedef struct tagSTerms {
        char    *_scode;
        char    _sterm [32];
    } STerms;

    STerms s_terms [6];

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	struct	{
		char	*_scode;
		char	*_sterm;
	} s_terms [] = {
		{"   ","Local                 "},
		{"CIF","Cost Insurance Freight"},
		{"C&F","Cost & Freight"},
		{"FIS","Free Into Store"},
		{"FOB","Free On Board"},
		{"",""},
	}; */

	char	*scn_desc [] = {
				"HEADER SCREEN.",
				"ITEM SCREEN.",
				"FREIGHT DETAILS SCREEN.",
				"MISCELLANEOUS DETAIL SCREEN.",
			};

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	char	automaticStockUpdate [2];
	char	currencyCode [4];
	char	gstCode [4];
	char	other [3][31];
	char	overrideQuantity [2];
	char	soSpecial [5];
	int		advertLevy;
	int		automaticFreight;
	int		combineInvoicePack;
	int		dbCo;
	int		dbFind;
	int		dbMcurr;
	int		dbNettUsed;
	int		discountIndents;
	int		gstApplies;
	int		includeForwardStock;
	int		lostSales;
	int		printConfirmation;
	int		reverseDiscount;
	int		salesOrderRealTimeDelete;
	int		serialItemsOk;
	int		soFreightCharge;
	int		soDoi;
	int		soFreightBord;
} envVar;

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	LL [2];
	char	UOM [5];
	char	batch_no [6];
	char	chargeToCustomer [7];
	char	curr_code [6];
	char	cust_no [7];
	char	dbt_date [11];
	char	defaultDelZone [7];
	char	dflt_batch [6];
	char	dflt_date [11];
	char	dummy [11];
	char	headOfficeCustomer [7];
	char	item_desc [41];
	char	item_no [17];
	char	lot_ctrl [2];
	char	ord_desc [10];
	char	ord_fulldesc [10];
	char	prev_dbt_no [7];
	char	prev_order [9];
	char	pri_desc [16];
	char	pri_fulldesc [16];
	char	sell_desc [31];
	char	serial_no [26];
	char	spinst [3][61];
	char	sup_part [17];
	char	systemDate [11];
	char	uom [5];
	double	inp_total;
	float	disc_over;
	float	qty_back;
	float	qty_des;
	float	qty_ord;
	int		line_no;
	long	inv_date;
} local_rec;            

static	struct	var	vars [] =
{
	{MAIN_SCN, LIN, "debtor",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.prev_dbt_no, "Customer No.        : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{MAIN_SCN, LIN, "curr_code",	 4, 35, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{MAIN_SCN, LIN, "name",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{MAIN_SCN, LIN, "p_slip_no",	 5, 2, CHARTYPE,
		"UUUUUUUU", "        ",
		" ", local_rec.prev_order, "Packing Slip No     : ", " ",
		 NE, NO, JUSTRIGHT, "", "", cohr_rec.inv_no},
	{MAIN_SCN, LIN, "cus_ord_ref",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Customer Order Ref. : ", " ",
		 NI, NO,  JUSTLEFT, "", "", cohr_rec.cus_ord_ref},
	{MAIN_SCN, LIN, "invoice_no",	 6, 2, CHARTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Invoice No.         : ", " ",
		 NA, NO, JUSTRIGHT, "", "", cohr_rec.inv_no},
	{MAIN_SCN, LIN, "headOfficeCustomer",	 6, 66, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Head Office Account : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.headOfficeCustomer},
	{MAIN_SCN, LIN, "chargeCustomerNo", 6, 102, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Charge Customer No  : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.chargeToCustomer},
	{MAIN_SCN, LIN, "dp_no",	 7, 2, CHARTYPE,
		"AA", "          ",
		" ", cohr_rec.dp_no, "Department No.      : ", " ",
		 NA, NO, JUSTRIGHT, "", "", cohr_rec.dp_no},
	{MAIN_SCN, LIN, "dp_name",	 7, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cudp_rec.dp_short},
    {MAIN_SCN, LIN, "cust_type", 7, 66, CHARTYPE,
        "AAA", "          ",
        " ", "", "Cust Type           : ", " ",
         NA, NO,  JUSTLEFT, "", "", cumr_rec.class_type},
	{MAIN_SCN, LIN, "cont_no",	 8, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract            : ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 NA, NO,  JUSTLEFT, "", "", cohr_rec.cont_no},
	{MAIN_SCN, LIN, "cont_desc",	 8, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Contract Description: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cnch_rec.desc},
	{MAIN_SCN, LIN, "cus_addr1",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Charge Address      : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr1},
	{MAIN_SCN, LIN, "cus_addr2",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr2},
	{MAIN_SCN, LIN, "cus_addr3",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr3},
	{MAIN_SCN, LIN, "cus_addr4",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr4},
	{MAIN_SCN, LIN, "del_name",	10, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Name       : ", " Select Delivery Name and Address. Search available. ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_name},
	{MAIN_SCN, LIN, "del_addr1",	11, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address    : ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{MAIN_SCN, LIN, "del_addr2",	12, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{MAIN_SCN, LIN, "del_addr3",	13, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "--------------------: ", " ",
		 NO, NO,  JUSTLEFT, "", "", cohr_rec.dl_add2},
	{MAIN_SCN, LIN, "batch_no",	15, 2, CHARTYPE,
		"UUUUU", "        ",
		" ", local_rec.dflt_batch, "Batch number        : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.batch_no},
	{MAIN_SCN, LIN, "ord_type",	15, 66, CHARTYPE,
		"U", "          ",
		" ", "D", "Order Type          : ", " D (omestic  E (xport ",
		NA, NO,  JUSTLEFT, "DE", "", local_rec.ord_desc},
	{MAIN_SCN, LIN, "ord_type_desc",	15, 91, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ord_fulldesc},
	{MAIN_SCN, LIN, "pri_type",	16, 2, CHARTYPE,
		"N", "        ",
		" ", cumr_rec.price_type, "Price Type          : ", "Enter price type 1-5 ",
		 NA, NO,  JUSTLEFT, "12345", "", local_rec.pri_desc},
	{MAIN_SCN, LIN, "pri_type_desc",	16, 27, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.pri_fulldesc},
	{MAIN_SCN, LIN, "dt_raised",	16, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.dflt_date, "Invoice Date        : ", " ",
		NI, NO,  JUSTLEFT, " ", "", (char *)&local_rec.inv_date},
	{MAIN_SCN, LIN, "fix_exch",	16, 66, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exchange Rate : ", " ",
		 ND, NO,  JUSTLEFT, "YN", "", cohr_rec.fix_exch},
	{MAIN_SCN, LIN, "tax_code",	17, 2, CHARTYPE,
		"U", "        ",
		" ", cumr_rec.tax_code, "Tax Code            : ", " ",
		 ND, NO,  JUSTLEFT, "ABCD", "", cohr_rec.tax_code},
	{MAIN_SCN, LIN, "tax_no",	17, 66, CHARTYPE,
		"AAAAAAAAAAAAAAA", "        ",
		" ", cumr_rec.tax_no, "Tax Number          : ", " ",
		 ND, NO,  JUSTLEFT, "", "", cohr_rec.tax_no},
	{MAIN_SCN, LIN, "sman_code",	18, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.sman_code, "Salesman            : ", " ",
		NI, NO, JUSTRIGHT, "", "", cohr_rec.sale_code},
	{MAIN_SCN, LIN, "sman_desc",	18, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesman Description: ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{MAIN_SCN, LIN, "area_code",	19, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.area_code, "Area code           : ", " ",
		NI, NO, JUSTRIGHT, "", "", cohr_rec.area_code},
	{MAIN_SCN, LIN, "area_desc",	19, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Area Description    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{MAIN_SCN, LIN, "disc_over",	20, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Discount Overide    : ", " This Discount is Added to Each Invoice Line ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.disc_over},
	{MAIN_SCN, LIN, "prt_price",	20, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print Price Details.: ", "Y(es) print prices on invoice, N(o) Don't print prices on Invoice.",
		NI, NO,  JUSTLEFT, "NY", "", cohr_rec.prt_price},
	{MAIN_SCN, LIN, "inp_total",	20, 66, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.00", "Invoice Total       : ", " Total Value Override for Invoice ",
		 NI, NO, JUSTRIGHT, "", "", (char *)&local_rec.inp_total},
	{ITEM_SCN, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{ITEM_SCN, TAB, "dsp_line_no",	 0, 0, INTTYPE,
		"NNN", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.line_no},
	{ITEM_SCN, TAB, "line_no",	 0, 0, INTTYPE,
		"NNN", "          ",
		" ", " ", " ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&coln_rec.line_no},
	{ITEM_SCN, TAB, "hide",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "H", " ",
		 NA, NO,  JUSTLEFT, "", "", coln_rec.hide_flag},
	{ITEM_SCN, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  I t e m  D e s c r i p t i o n   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{ITEM_SCN, TAB, "sale_code",	 0, 1, CHARTYPE,
		"UU", "          ",
		" ", cohr_rec.sale_code, "Sale", " Salesman ",
		 ND, NO, JUSTRIGHT, "", "", coln_rec.sman_code},
	{ITEM_SCN, TAB, "ord_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cohr_rec.cus_ord_ref, " Cust Order Ref ", "Customer Order Ref.",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.cus_ord_ref},
	{ITEM_SCN, TAB, "pack_size",	 0, 0, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Pack ", " Pack Size ",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.pack_size},
	{ITEM_SCN, TAB, "UOM",	 0, 0, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "UOM ", " Unit of Measure ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.UOM},
	{ITEM_SCN, TAB, "qty_ord",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Qty Order", " ",
		 NI, NO, JUSTRIGHT, "0", "99999.99", (char *)&local_rec.qty_ord},
	{ITEM_SCN, TAB, "qty_des",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Qty  Desp", " ",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *)&local_rec.qty_des},
	{ITEM_SCN, TAB, "qty_back",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Qty  Back", " ",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *)&local_rec.qty_back},
	{ITEM_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM_SCN, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Cost Price", " ",
		 NO, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&coln_rec.cost_price},
	{ITEM_SCN, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Sale Price", " ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&coln_rec.sale_price},
	{ITEM_SCN, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Disc ", " ",
		YES, NO, JUSTRIGHT, "-99.99", "99.99", (char *)&coln_rec.disc_pc},
	{ITEM_SCN, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Serial Number      ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.serial_no},

	{FRI_SCN, LIN, "carrierCode",	 4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Carrier Code.       : ", "Enter carrier code, [SEARCH] available.",
		YES, NO,  JUSTLEFT, "", "", trcm_rec.carr_code},
	{FRI_SCN, LIN, "carr_desc",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Carrier Description : ", " ",
		 NA, NO,  JUSTLEFT, "", "", trcm_rec.carr_desc},
	{FRI_SCN, LIN, "deliveryZoneCode",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultDelZone, "Delivery Zone       : ", "Enter Delivery Zone Code [SEARCH]. ",
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{FRI_SCN, LIN, "deliveryZoneDesc",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Zone Desc  : ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{FRI_SCN, LIN, "deliveryRequired",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Delivery Reqd. (Y/N): ", "Enter Y(es) for Delivery. <default = N(o)> ",
		 YES, NO, JUSTLEFT, "YN", "", cohr_rec.del_req},
	{FRI_SCN, LIN, "deliveryDate",	6, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.dflt_date, "Delivery Date       : ", " ",
		NA, NO,  JUSTLEFT, " ", "", (char *)&cohr_rec.del_date},
	{FRI_SCN, LIN, "cons_no",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Consignment no.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.cons_no},
	{FRI_SCN, LIN, "no_cartons",	 7, 66, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Number Cartons.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cohr_rec.no_cartons},
	{FRI_SCN, LIN, "est_freight",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Est Freight         : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&est_freight},
	{FRI_SCN, LIN, "tot_kg",	 9, 66, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Total Kgs.          : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.no_kgs},
	{FRI_SCN, LIN, "frei_req",	 10, 2, CHARTYPE,
		"U", "          ",
		" ", cumr_rec.freight_chg, "Freight Required.   : ", "Enter Y(es) if freight required Default = Customer master file default. ",
		YES, NO, JUSTRIGHT, "YN", "", cohr_rec.frei_req},
	{FRI_SCN, LIN, "freight",	 10, 66, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.     : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.freight},
	{FRI_SCN, LIN, "shipname",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship to name        : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_name},
	{FRI_SCN, LIN, "shipaddr1",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship to address 1   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{FRI_SCN, LIN, "shipaddr2",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship to address 2   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add2},
	{FRI_SCN, LIN, "shipaddr3",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship to address 3   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add3},
	{FRI_SCN, LIN, "ship_method",	17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipment method     : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{FRI_SCN, LIN, "spcode1",	18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 1       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{FRI_SCN, LIN, "spcode2",	19, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 2       : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{COST_SCN, LIN, "pay_term",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.      : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.pay_terms},
	{COST_SCN, LIN, "sell_terms",	 6, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Sell Terms.         : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.sell_terms},
	{COST_SCN, LIN, "sell_desc",	7 , 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Sell Description.   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sell_desc},
	{COST_SCN, LIN, "insurance",	 9, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Insurance.          : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.insurance},
	{COST_SCN, LIN, "ins_det",	9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Insurance Details   : ", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.ins_det},
	{COST_SCN, LIN, "deposit",	11, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Deposit.            : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.deposit},
	{COST_SCN, LIN, "discount",	12, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Special Discount.   : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.ex_disc},
	{COST_SCN, LIN, "other_1",	14, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [0], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_1},
	{COST_SCN, LIN, "other_2",	15, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [1], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_2},
	{COST_SCN, LIN, "other_3",	16, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", envVar.other [2], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&cohr_rec.other_cost_3},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include	<cus_price.h>
#include	<cus_disc.h>
#include	<neg_win.h>
#include	<p_terms.h>
#include	<proc_sobg.h>
#include	<FindCumr.h>
#include	<LocHeader.h> 
#include	<CheckIndent.h> 
#include	<ItemLevy.h> 

/*=======================
| Function Declarations |
=======================*/
char*   CheckVariable 			(char *, char *);
char	*GetPriceDesc 			(int);
double	GetMargCost 			(int);
float 	CalcOverideValues 		(double);
float 	ScreenDisc 				(float);
float 	ToLclUom 				(float);
float 	ToStdUom 				(float);
int 	LoadItemScreen 			(long);
int 	CheckOk 				(void);
int  	AddIncc 				(long, long);
int  	CheckBonusLine 			(char *);
int  	CheckDuplicateInsf 		(char *, long, int);
int  	CheckForNilBalance 		(void);
int  	CheckHiddenLine 		(char *);
int  	UpdateInsf 				(long, long, char *, char *, char *);
int 	SrchCudi 				(int);
int  	ValidateItemNumber 		(int);
int  	DeleteLine 				(void);
int  	FindCucc 				(int, long);
int  	heading 				(int scn);
int  	spec_valid 				(int);
int  	use_window 				(int);
int  	win_function 			(int, int, int, int);
int  	win_function2 			(int, int, int, int);
void 	AddCarrierDetails 		(void);
void 	BusyFunction 			(int);
void 	CalcInputTotal 			(void);
void 	CalcLineExtended 		(int);
void 	CalculateFreight 		(float, double, double, double);
void 	CalculateTotalBox 		(int, int);
void 	CheckEnvironment 		(void);
void 	ClearWindow 			(void);
void 	CloseDB 				(void);
void 	CloseTransportFiles 	(void);
void 	Confirm 				(void);
void 	DeleteSTerms 			(void);
void 	DeleteSalesOrders 		(long);
void 	DiscProcess 			(void);
void 	DispSONS 				(int);
void 	DrawTotals 				(void);
void 	FreeInsf 				(int, char *);
void 	InitML 					(void);
void 	InitSTerms 				(void);
void 	InputSONS 				(int, int);
void 	LoadSONS 				(int, int, long);
void 	LogLostSales 			(float);
void 	OpenDB 					(void);
void 	OpenTransportFiles 		(char *);
void 	PriceProcess 			(void);
void 	PrintCompanyDetails 	(void);
void 	PrintTotalBoxValues 	(void);
void 	ReadMisc 				(void);
void 	SerialItemMessage 		(int, int);
void 	SetSTerm 				(int, char *, char *);
void 	SetSTermCode 			(int, char *);
void 	SrchCohr 				(char *);
void 	SrchExaf 				(char *);
void 	SrchExsf 				(char *);
void 	SrchExsi 				(char *);
void 	SrchInsf 				(char *, int);
void 	SrchInum 				(char *);
void 	SrchPay 				(void);
void 	SrchSellTerms 			(void);
void 	SrchTrcm 				(char *);
void 	SrchTrzm 				(char *);
void 	TidySonsScreen 			(void);
void 	Update 					(void);
void 	UpdateSONS 				(int, int, long);
void 	tab_other 				(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	int		i;
	int		cnt;
	int		lpno = 0;

	SETUP_SCR (vars);


	curr_user = getenv ("LOGNAME");

	TruePosition	=	TRUE;

	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	X_EALL = 3;
	Y_EALL = 3;

	_win_func = TRUE;

	FirstInv = 1;
	if (argc != 4 && argc != 5)
	{
		print_at (0,0, mlSoMess755 ,argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (createStatusFlag, "%-1.1s", argv [1]);
	sprintf (findStatusFlag,	 "%-1.1s", argv [2]);

	if (argc == 5)
		lpno = atoi (argv [4]);

	init_scr ();
	set_tty (); 
	_set_masks (argv [3]);

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM_SCN, store, sizeof (struct storeRec));
#endif
	if (envVar.gstApplies)
	{
		FLD ("tax_code") = ND;
		FLD ("tax_no")   = ND;
	}

	if (envVar.serialItemsOk)
	{
		FLD ("ser_no") 		= NA;
		FLD ("descr")		= ND;
	}
	else
		FLD ("ser_no") 	= ND;

	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	/*---------------------------------
	| Open Output to Counter Printer. |
	---------------------------------*/
	if (lpno) 
		ip_open (comm_rec.co_no, comm_rec.est_no, lpno);


	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	des_date = TodaysDate ();

	if (envVar.soDoi)
		strcpy (local_rec.dflt_date, local_rec.systemDate);
	else
		strcpy (local_rec.dflt_date, DateToString (comm_rec.dbt_date));
		
	local_rec.inv_date = StringToDate (local_rec.dflt_date);


	strcpy (branchNo, (envVar.dbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	InitML ();

	/*-------------------------------------
	| Open Discounting and Pricing files. |
	-------------------------------------*/
	OpenPrice ();
	OpenDisc ();


	strcpy (local_rec.prev_order,"00000000");
	strcpy (local_rec.prev_dbt_no,"000000");
	strcpy (local_rec.dflt_batch,"00000");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		for (cnt = 0; cnt < MAX_SONS; cnt++)
			sprintf (HeaderDesc [cnt], "%40.40s", " ");

		for (i = 0; i < maxLinesUsed; i++)
		{
			for (cnt = 0; cnt < MAX_SONS; cnt++)
				sprintf (store [i].nsDesc [cnt], "%40.40s", " ");
		}
		if (restart) 
		{
			if (SK_BATCH_CONT || MULT_LOC)
				AllocationRestore ();
			/*---------------------------------------------------
			| if restarted, free all newly keyed serial number, |
         	| re-committ any orginal serial numbers that have   |
          	| changed, if cannot commit then display message    |
			| and set to blank                                  |
			---------------------------------------------------*/
			for (i = 0; i < maxLinesUsed; i++)
			{
				/*  free keyed */

				if (strcmp (store [i]._serial, ser_space))
				{
					cc = UpdateInsf 
						(
							store [i].hhwhHash,
							store [i].hhbrHash,
							store [i]._serial,
							"C",
							"F"
						);
					if (cc)
						SerialItemMessage (i, TRUE);
				}

				/*  commit original */
				if (strcmp (store [i]._org_ser, ser_space))
				{
					cc = UpdateInsf (store [i].hhwhHash,
									store [i].hhbrHash,
									store [i]._org_ser,"F","C");
					if (cc)
						SerialItemMessage (i, FALSE);
				}
			}
		}
		invoiceChanged = FALSE;
		FLD ("item_no") = NA;
		if (!F_HIDE (label ("descr")))
			FLD ("descr") = NA;
		cumr_rec.hhcu_hash = 0L;
		if (restart) 
		{
			abc_unlock (soln);
			abc_unlock (cohr);
			abc_unlock (coln);
			if (FirstInv && FLD ("batch_no") == NA)
				FLD ("batch_no") = YES;
		}

		for (i = 0; i < maxLinesUsed; i++)
		{
			LotClear (i);

			memset ((char *) &store [i], '\0', sizeof (struct storeRec));
		
			strcpy (store [i].	_lot_ctrl,	" ");
			strcpy (store [i].	_sellgrp ,	"      ");
			strcpy (store [i].	_serial ,   ser_space);
			strcpy (store [i].	_org_ser,   ser_space);
			strcpy (store [i].	_desc2,   ns_space);
			strcpy (store [i].	_category,  "           ");
			strcpy (store [i].	_bonus,		" ");
			strcpy (store [i].	_class,		" ");
			strcpy (store [i].	_border,	" ");
			strcpy (store [i].	_release,	" ");
			strcpy (store [i].	_pack_size, "     ");
			strcpy (store [i].	_cost_flag,	" ");
			strcpy (store [i].	_ser_item,	" ");
			strcpy (store [i].	err_fnd,	" ");
		}
		strcpy (local_rec.defaultDelZone, "      ");
		est_freight = 0.00;
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars (MAIN_SCN);	
		init_vars (ITEM_SCN);	
		init_vars (FRI_SCN);
		init_vars (COST_SCN);
		lcount [ITEM_SCN] = 0;

		/*------------------------------
		| Enter screen 1 linear input. |
		| Turn screen initialise on.   |
		------------------------------*/
		init_ok = TRUE;
		eoi_ok = FALSE;
		heading (MAIN_SCN);
		entry (MAIN_SCN);
		if (prog_exit || restart)
			continue;

		last_char = prmptmsg (ML (mlSoMess148),"YyNn",0,2);

		if (last_char == 'N' || last_char == 'n')
		{
			init_ok	= FALSE;
			eoi_ok	= FALSE;
			heading (ITEM_SCN);
			scn_display (ITEM_SCN);
			entry (ITEM_SCN);
			eoi_ok = TRUE;
			if (restart)
				continue;
		}
		else
			Confirm ();

		prog_status = ! (ENTRY);

		FLD ("item_no") = YES;
		if (!F_HIDE (label ("descr")))
			FLD ("descr") = NI;
		vars [label ("item_no")].row = MAXLINES;

		while (TRUE)
		{
			if (CheckForNilBalance ())
			{
				print_mess (ML (mlSoMess241));
				sleep (sleepTime);
				clear_mess ();
			}

			edit_all ();
			if (restart)
				break;

			if (FREIGHT_CHG && cohr_rec.freight == 0.00	)
			{
				i = prmptmsg (ML (mlSoMess237),"YyNn",0,23);
				BusyFunction (0);
				if (i == 'Y' || i == 'y')
					break;
				if (i == 'n' || i == 'N') 
					print_at (23,0, "                                   ");

			}
			else
				break;
		}

		/*-------------------------------
		| Check for blank Serial items. |
		-------------------------------*/
		while (CheckOk ())
		{
			sprintf (err_str, ML (mlSoMess242),CheckOk ());
			print_mess (err_str);
			sleep (SLEEP_TIME);
			clear_mess ();

			if (CheckForNilBalance ())
			{
				print_mess (ML (mlSoMess241));
				sleep (sleepTime);
				clear_mess ();
			}

			edit_all ();
			if (restart)
				break;
		}

		if (restart)
			continue;

		Update ();
		if (SK_BATCH_CONT || MULT_LOC)
			AllocationComplete ();

		if (lpno)
		{
			/*-----------------------------------------------
			| if printing changed invoices & this invoice	|
			| changed  OR printing all (counter!!!)			|
			-----------------------------------------------*/
			if ((envVar.printConfirmation && invoiceChanged) || !envVar.printConfirmation || CU_PRINT)
				ip_print (cohr_rec.hhco_hash);
		}

		FLD ("batch_no") = NA;
		strcpy (local_rec.dflt_batch,local_rec.batch_no);
		FirstInv = 0;

		/*--------------------------
		| Clear old table entries. |
		--------------------------*/
		init_vars (ITEM_SCN);
	}

	/*---------------------------------
	| Open Output to Counter Printer. |
	---------------------------------*/
	if (lpno)
		ip_close ();

	/*=========================
	| Program exit sequence . |
	=========================*/
	CloseDB (); 
	FinishProgram ();
    DeleteSTerms ();
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

	strcpy (mlDesConf [1], ML ("Domestic"));
	strcpy (mlDesConf [2], ML ("Export"));

}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (soln2, soln);
	abc_alias (cumr2, cumr);
	abc_alias (incc2, incc);
	abc_alias (inum2, inum);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	ReadMisc ();

	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2, incc_list, INCC_NO_FIELDS, "incc_hhwh_hash");
	open_rec (incp,  incp_list, incp_no_fields, "incp_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (insf,  insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (soln2, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (sons,  sons_list, SONS_NO_FIELDS, "sons_id_no4");
	if (envVar.advertLevy)
		open_rec (inal,inal_list,INAL_NO_FIELDS,"inal_id_no");

	OpenLocation (ccmr_rec.hhcc_hash);

	LL_EditLoc	=	FALSE;
	LL_EditLot	=	FALSE;
	LL_EditDate	=	FALSE;
	LL_EditSLot	=	FALSE;

	IgnoreAvailChk	=	TRUE;
	if (llctDesConf [0] == 'V')
		LotSelectFlag	=	INP_VIEW;
	if (llctDesConf [0] == 'A')
		LotSelectFlag	=	INP_AUTO;
	if (llctDesConf [0] == 'M')
	{
		strcpy (StockTake, "Y");
		LotSelectFlag	=	INP_VIEW;
	}
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cnch);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cucc);
	abc_fclose (cuit);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (exaf);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (incp);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (insf);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (pocr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_fclose (sons);
	if (envVar.advertLevy)
		abc_fclose (inal);

	ArchiveClose ();
	CloseLocation ();

	/*-----------------------------
	| Close three Discount files. |
	-----------------------------*/
	ClosePrice ();
	CloseDisc ();

	SearchFindClose ();
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr, &comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec,COMPARISON,"r");
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
	int		val_pterms = FALSE;
	int		TempLine;

	if (line_cnt > maxLinesUsed)
		maxLinesUsed	=	line_cnt;

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("debtor")) 
	{
		skip_entry = 0;
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo,temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
		cc = find_rec (cumr, &cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
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
		DSP_FLD ("name");
		DSP_FLD ("headOfficeCustomer");
		use_window (FN14);

		if (envVar.dbMcurr)
		{
			strcpy (pocrRec.co_no, comm_rec.co_no);
			sprintf (pocrRec.code, "%-3.3s", cumr_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");

			sprintf (local_rec.curr_code, "(%-3.3s)", cumr_rec.curr_code);
			DSP_FLD ("curr_code");
		}
		sprintf (local_rec.defaultDelZone, "%-6.6s", cumr_rec.del_zone);

		return (EXIT_SUCCESS);
	}
		
	/*-------------------------------
	| Validate pack Slip Number.	|
	-------------------------------*/
	if (LCHECK ("p_slip_no")) 
	{
		if (SRCH_KEY)
		{
			SrchCohr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		
		strcpy (cohr_rec.co_no,comm_rec.co_no);
		strcpy (cohr_rec.br_no,comm_rec.est_no);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cohr_rec.type,"P");
		strcpy (cohr_rec.inv_no,zero_pad (cohr_rec.inv_no,8));
		cc = find_rec (cohr, &cohr_rec,COMPARISON,"w");
		if (cc == -1)
		{
			abc_unlock (cohr);
			return (EXIT_FAILURE);
		}
		if (cc)
		{
			print_mess (ML (mlSoMess227));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (cohr_rec.ps_print [0] != 'Y')
		{
			print_mess (ML ("Packing Slip has not been printed."));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		sohr_rec.hhso_hash	=	cohr_rec.hhso_hash;
		if (find_rec (sohr, &sohr_rec, EQUAL, "r"))
		{
			abc_unlock (cohr);

			print_mess (ML (mlStdMess102));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*----------------------
		| if contract exists   |
		| read the cnch record |
		-----------------------*/
		if (strcmp (cohr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess075));
				sleep (SLEEP_TIME);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
		}

		open_rec (cudp,cudp_list,CUDP_NO_FIELDS,"cudp_id_no");
		cc = find_rec (cudp, &cudp_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (cudp_rec.co_no,comm_rec.co_no);
			strcpy (cudp_rec.br_no,comm_rec.est_no);
			strcpy (cudp_rec.dp_no,cohr_rec.dp_no);
			cc = find_rec (cudp, &cudp_rec,COMPARISON,"r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		abc_fclose (cudp);

		if (LoadItemScreen (cohr_rec.hhco_hash))
		{
			abc_unlock (cohr);
			return (EXIT_FAILURE);
		}

		if (lcount [ITEM_SCN] == 0)
		{
			abc_unlock (cohr);

			print_mess (ML (mlSoMess141));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ord_desc,
				(cohr_rec.ord_type [0] == 'D') ? "D" : "E");
		strcpy (local_rec.ord_fulldesc,
				(cohr_rec.ord_type [0] == 'D') ? mlDesConf [1] : mlDesConf [2]);

		strcpy (local_rec.spinst [0],cohr_rec.din_1);
		strcpy (local_rec.spinst [1],cohr_rec.din_2);
		strcpy (local_rec.spinst [2],cohr_rec.din_3);
		strcpy (local_rec.pri_desc, cohr_rec.pri_type);

		strcpy (local_rec.pri_fulldesc, GetPriceDesc (atoi (cohr_rec.pri_type) - 1));

		/*-----------------------------------
		| load selling terms description	|
		-----------------------------------*/
		for (i = 0;strlen (s_terms [i]._scode);i++)
		{
			if (!strncmp (cohr_rec.sell_terms, s_terms [i]._scode,
				      strlen (s_terms [i]._scode)))
			{
				sprintf (local_rec.sell_desc,"%-30.30s", s_terms [i]._sterm);
				break;
			}
		}

		open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
		cc = find_rec (exsf, &exsf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, exsf, "DBFIND");

		abc_fclose (exsf);

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,cohr_rec.area_code);
		cc = find_rec (exaf, &exaf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, exaf, "DBFIND");

		OpenTransportFiles ("trzm_id_no");
		
		strcpy (trzm_rec.co_no,comm_rec.co_no);
		strcpy (trzm_rec.br_no,comm_rec.est_no);
		strcpy (trzm_rec.del_zone,cohr_rec.del_zone);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (trzm_rec.del_zone, cumr_rec.del_zone);
			cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (trzm_rec.del_zone, "      ");
				trzm_rec.trzm_hash	=	0L;
				trzm_rec.dflt_chg	=	0.0;
				trzm_rec.chg_kg		=	0.0;
			}
			else
				strcpy (cohr_rec.del_zone, trzm_rec.del_zone);
		}
		strcpy (trcm_rec.carr_code, "    ");
	  	sprintf (trcm_rec.carr_desc,"%40.40s", " ");
		est_freight 		= 	0.00;
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

		strcpy (local_rec.chargeToCustomer, " SAME ");
		if (cohr_rec.chg_hhcu_hash != 0L)
		{
			cumr2_rec.hhcu_hash	=	cohr_rec.chg_hhcu_hash;
		    cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		    if (!cc)
				strcpy (local_rec.chargeToCustomer, cumr2_rec.dbt_no);
		}
		DSP_FLD ("chargeCustomerNo");
		scn_display (MAIN_SCN);
		
		return (EXIT_SUCCESS);
	}
	/*------------------------
	| Validate Batch Number. |
	------------------------*/
	if (LCHECK ("batch_no")) 
	{
		if (F_NOKEY (field))
			strcpy (local_rec.batch_no,pad_batch (local_rec.dflt_batch));
		else
			strcpy (local_rec.batch_no,pad_batch (local_rec.batch_no));

		DSP_FLD ("batch_no");
		DSP_FLD ("ord_type");
		DSP_FLD ("ord_type_desc");
		DSP_FLD ("pri_type");
		DSP_FLD ("pri_type_desc");
		DSP_FLD ("dt_raised");
		DSP_FLD ("tax_code");
		DSP_FLD ("tax_no");
		DSP_FLD ("fix_exch");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------------------------------------
	| Check for misc fields than can be changed without reprint of p/s. |
	-------------------------------------------------------------------*/
	if (LCHECK ("tot_kg") || LCHECK ("cons_no") || LCHECK ("no_cartons"))
		return (EXIT_SUCCESS);

	if (prog_status != ENTRY)
		invoiceChanged = TRUE;

	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dp_no")) 
	{
		if (prog_status == ENTRY)
			DSP_FLD ("dp_no");

		open_rec (cudp,cudp_list,CUDP_NO_FIELDS,"cudp_id_no");
		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,cohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (SLEEP_TIME);
			clear_mess ();
			abc_fclose (cudp);
			return (EXIT_FAILURE); 
		}
		abc_fclose (cudp);

		PrintCompanyDetails ();
		DSP_FLD ("dp_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("inp_total")) 
	{
		if (strcmp (cohr_rec.cont_no, "      ") && local_rec.inp_total)
		{
			print_mess (ML (mlSoMess243));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("disc_over")) 
	{
		if (strcmp (cohr_rec.cont_no, "      ") && local_rec.disc_over)
		{
			print_mess (ML (mlSoMess243));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY)
		{
			CalculateTotalBox (TRUE,FALSE);
			scn_set (MAIN_SCN);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ("invoice_no")) 
	{
		DSP_FLD ("invoice_no");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Tax Code. |
	--------------------*/
	if (LCHECK ("tax_code")) 
	{
		if (dflt_used || cohr_rec.tax_code [0] == ' ')
		{
			strcpy (cohr_rec.tax_code,cumr_rec.tax_code);
			strcpy (cohr_rec.tax_no,cumr_rec.tax_no);
		}
		if (cohr_rec.tax_code [0] == 'A' ||
		     cohr_rec.tax_code [0] == 'B')
			noTaxCharged = TRUE;
		else
			noTaxCharged = FALSE;

		if (F_HIDE (label ("tax_code")))
			return (EXIT_SUCCESS);

		if (cohr_rec.tax_code [0] == 'C' ||
		     cohr_rec.tax_code [0] == 'D')
			FLD ("tax_no") = NI;
		else
			FLD ("tax_no") = YES;

		DSP_FLD ("tax_no");
		return (EXIT_SUCCESS);
	}

	/*------------------
	| Validate Tax No. |
	------------------*/
	if (LCHECK ("tax_no")) 
	{
		if (F_HIDE (label ("tax_no")))
			return (EXIT_SUCCESS);

		if (dflt_used)
			strcpy (cohr_rec.tax_no,cumr_rec.tax_no);

		if (FLD ("tax_no") == NI)
			return (EXIT_SUCCESS);

		if (noTaxCharged && !strcmp (cohr_rec.tax_no,"               "))
		{
			print_mess (ML (mlStdMess200));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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
			SR_Y_POS = vars [label ("pay_term")].row - 1;

			SrchPay ();

			SR_Y_POS = 0;
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms,
				       p_terms [i]._pcode,
				       strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms,"%-40.40s",p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (SLEEP_TIME);
			clear_mess ();
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
			SR_Y_POS = vars [label ("pay_term")].row - 1;
			SrchSellTerms ();
			SR_Y_POS = 0;
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (s_terms [i]._scode);i++)
		{
			if (!strncmp (cohr_rec.sell_terms,s_terms [i]._scode,strlen (s_terms [i]._scode)))
			{
				sprintf (local_rec.sell_desc,"%-30.30s",s_terms [i]._sterm);
				break;
			}
		}

		if (!strlen (s_terms [i]._scode))
		{
			print_mess (ML (mlStdMess203));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		DSP_FLD ("sell_terms");
		DSP_FLD ("sell_desc");

		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Processed Date. |
	--------------------------*/
	if (LCHECK ("dt_raised")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			/*----------------------------------------------------
			| Packing slip/invoice combined so invoice date must |
			| be what was on printed Doco.                       |
			----------------------------------------------------*/
			if (envVar.combineInvoicePack)
				local_rec.inv_date = cohr_rec.date_raised;
			else
			{
				if (envVar.soDoi)
					local_rec.inv_date = TodaysDate ();
				else
					local_rec.inv_date = comm_rec.dbt_date;
			}
		}
		else
			strcpy (local_rec.systemDate,DateToString (local_rec.inv_date));

		if (prog_status == ENTRY)
		{
			DSP_FLD ("dt_raised");
			if (F_NEED (field) && chq_date (local_rec.inv_date,
							comm_rec.dbt_date))
				return (EXIT_FAILURE);
		}
		else
			if (chq_date (local_rec.inv_date,comm_rec.dbt_date))
				return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Salesman Code. |
	-------------------------*/
	if (LCHECK ("sman_code")) 
	{
		open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
		cc = find_rec (exsf, &exsf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (SLEEP_TIME);
			clear_mess ();
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}

		if (F_NOKEY (field))
			DSP_FLD ("sman_code") ;

		/*------------------------------------------
		| If sman changed at header level and line |
		| level salesman can not be changed then   |
		| force all lines the same as the header.  |
		------------------------------------------*/
		if (F_NOKEY (label ("sale_code")) && prog_status != ENTRY)
		{
			scn_set (ITEM_SCN);
			
			for (i = 0; i < lcount [ITEM_SCN]; i++)
			{
				getval (i);
				strcpy (coln_rec.sman_code, cohr_rec.sale_code);
				putval (i);
			}

			scn_set (MAIN_SCN);
		}

		DSP_FLD ("sman_desc");
		abc_fclose (exsf);
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Area Code. |
	---------------------*/
	if (LCHECK ("area_code")) 
	{
		if (prog_status == ENTRY)
			DSP_FLD ("area_code");

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,cohr_rec.area_code);
		cc = find_rec (exaf, &exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("area_code");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. | 
	-----------------------*/
	if (LCHECK ("item_no")) 
	{
		if (line_cnt < numberOldLines)
		{
			if (prog_status == ENTRY)
			{
				getval (line_cnt);
				if (NON_STOCK)
					skip_entry = goto_field (field,label ("ser_no")) + 1;
			}
			else
			{
				sprintf (local_rec.item_no,"%-16.16s",prv_ntry);
				DSP_FLD ("item_no");
			}
			strcpy (store [line_cnt]._lot_ctrl,inmr_rec.lot_ctrl);
			tab_other (line_cnt);
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

		cc = ValidateItemNumber (TRUE);

		if (!cc)
			strcpy (store [line_cnt]._lot_ctrl,inmr_rec.lot_ctrl);

		local_rec.line_no	=	line_cnt + 1;
		DSP_FLD ("dsp_line_no");

		tab_other (line_cnt);

		return (cc);
	}

	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (prog_status == ENTRY && 
			(F_NOKEY (label ("UOM")) && line_cnt < numberOldLines))
			return (EXIT_SUCCESS);

		if (dflt_used)
			strcpy (local_rec.UOM, 	SR._UOM);

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR._uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (SR._UOM, inum2_rec.uom);
		SR.hhumHash 	= inum2_rec.hhum_hash;

		if (SR._std_cnv_fct == 0.00)
			SR._std_cnv_fct = 1.00;

		SR._cnv_fct 	= inum2_rec.cnv_fct/SR._std_cnv_fct;
		if (SR._pricing_chk == FALSE)
		{
			PriceProcess ();
			DiscProcess ();
		}

		DSP_FLD ("UOM");

		if (prog_status != ENTRY)
        {
            /*-------------------
            | Reenter Qty. Des. |
            --------------------*/
            do
            {
                get_entry (label ("qty_des"));
                cc = spec_valid (label ("qty_des"));
            } while (cc);
        }

		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate Line Number. | 
	-----------------------*/
	if (LCHECK ("dsp_line_no"))
	{
		local_rec.line_no 	=	line_cnt + 1;
		DSP_FLD ("dsp_line_no");
		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Item Description. |
	----------------------------*/
	if (LCHECK ("descr"))
	{
		if (NON_STOCK)
			skip_entry = goto_field (field,label ("ser_no")) + 1;
		line_display ();


		sprintf (SR._item_desc,	"%-40.40s",	local_rec.item_desc);

		tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Validate Salesman Code, Line Level. |
	-------------------------------------*/
	if (LCHECK ("sale_code")) 
	{
		open_rec (exsf,exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
		/*---------------
		| Adding Lines	|
		---------------*/
		if (line_cnt >= numberOldLines && prog_status == ENTRY)
			strcpy (coln_rec.sman_code,cohr_rec.sale_code);

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,coln_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec,COMPARISON,"r");
		if (cc)
		{
			if (!F_HIDE (field))
				print_mess (ML (mlStdMess135));
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}

		if (!F_HIDE (field))
		{
			DSP_FLD ("sale_code");
			print_at (2,1, ML (mlStdMess202) ,exsf_rec.salesman_no,exsf_rec.salesman);
		}
		abc_fclose (exsf);
		return (EXIT_SUCCESS);
	}
	/*----------------------------------------------
	| Validate Customer Order Number , Line Level. |
	----------------------------------------------*/
	if (LCHECK ("ord_ref")) 
	{
		/*---------------
		| Adding Lines	|
		---------------*/
		if (line_cnt >= numberOldLines && prog_status == ENTRY)
			strcpy (coln_rec.cus_ord_ref,cohr_rec.cus_ord_ref);

		DSP_FLD ("ord_ref");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("prt_price")) 
	{
		if (prog_status == ENTRY && F_NOKEY (label ("prt_price")))
		{
			strcpy (cohr_rec.prt_price, (hiddenLineFound) ? "N" : "Y");
			return (EXIT_SUCCESS);
		}
		if (hiddenLineFound && cohr_rec.prt_price [0] == 'Y')
		{
			strcpy (cohr_rec.prt_price, "N");
			print_mess (ML (mlSoMess142));
			sleep (SLEEP_TIME);
			clear_mess ();
			DSP_FLD ("prt_price");
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Pack Size , Line Level. |
	----------------------------------*/
	if (LCHECK ("pack_size")) 
	{
		if (dflt_used)
		{
			strcpy (coln_rec.pack_size,SR._pack_size);
			DSP_FLD ("pack_size");
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Quantity Ordered. |
	----------------------------*/
	if (LCHECK ("qty_ord")) 
	{
		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.qty_ord = local_rec.qty_des + local_rec.qty_back;
		
		if (local_rec.qty_ord < local_rec.qty_des + local_rec.qty_back)
		{
			print_mess (ML (mlSoMess294));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("qty_ord");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Quantity Despatched. |
	-------------------------------*/
	if (LCHECK ("qty_des")) 
	{
		if (dflt_used)
			local_rec.qty_des = local_rec.qty_ord - local_rec.qty_back;

		if (line_cnt < numberOldLines && FULL_BO && local_rec.qty_des != 0.00)
		{
			if (local_rec.qty_des != (local_rec.qty_ord - local_rec.qty_back))
			{
				print_mess (ML (mlSoMess336));
				sleep (SLEEP_TIME);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (line_cnt < numberOldLines && local_rec.qty_des > local_rec.qty_ord)
		{
			print_mess (ML (mlSoMess301));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!MULT_QTY && local_rec.qty_des != 0.00 && local_rec.qty_des != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*--------------------
		| if backorder set
		| ord to zero
		------------------------*/
		if (SERIAL_ITEM && local_rec.qty_des == 0.00)
		{
			if (strcmp (SR._serial, ser_space))
			{
				cc = UpdateInsf (SR.hhwhHash,SR.hhbrHash,SR._serial,"C","F");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}

			local_rec.qty_back = 1.00;
			strcpy (SR._serial, ser_space);
			strcpy (local_rec.serial_no, ser_space);
			DSP_FLD ("qty_back");
			DSP_FLD ("ser_no");
		}

		if (SERIAL_ITEM && (local_rec.qty_des + local_rec.qty_back) > 1.00)
		{
			local_rec.qty_back = 0.00;
			DSP_FLD ("qty_back");
		}

		/*---------------------------------
		| Calculate actual committed.     |
		| Ignore record for current line. |
		---------------------------------*/
		incc_rec.hhwh_hash = SR.actHhwhHash;
		cc = find_rec (incc2, &incc_rec, EQUAL, "r");
		if (cc) 
			file_err (cc, incc2, "DBFIND");

		if (envVar.includeForwardStock)
		{
			realStock = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder - 
						incc_rec.forward + 
						local_rec.qty_ord;
		}
		else
		{
			realStock = incc_rec.closing_stock -
						(incc_rec.committed + realCommitted) -
						incc_rec.backorder +
						local_rec.qty_ord;
		}

		if (local_rec.qty_des != local_rec.qty_ord - local_rec.qty_back)
			invoiceChanged = TRUE;

		if (line_cnt >= numberOldLines)
		{
			local_rec.qty_ord = local_rec.qty_des;
			if (SR._pricing_chk == FALSE)
			{
				PriceProcess ();
				DiscProcess ();
			}	
			DSP_FLD ("qty_ord");
		}

		SR._qty_des = local_rec.qty_des;
		if (!BO_OK)
		{
			local_rec.qty_back = 0.00;
			DSP_FLD ("qty_back");
			if (!BONUS)
				skip_entry = goto_field (field, label ("LL"));
			/*---------------------------------------------
			| log to lost sales if qty backorded reduced. |
			---------------------------------------------*/
			if (local_rec.qty_des < local_rec.qty_ord - local_rec.qty_back)
			{
				LogLostSales (local_rec.qty_ord - 
				  	local_rec.qty_des - 
				  	local_rec.qty_back);
			}
		}
		CalculateTotalBox (FALSE, FALSE);
		CalculateFreight 
		(
			trcm_rec.markup_pc, 
			trcl_rec.cost_kg,
			trzm_rec.chg_kg,
			trzm_rec.dflt_chg
		);
	
		if (prog_status != ENTRY)
		{
			DSP_FLD ("qty_des");

			/*-------------------
			| Reenter Location. |
			--------------------*/
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				cc = spec_valid (label ("LL"));
			} while (cc);
		}

		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Quantity Backordered. |
	--------------------------------*/
	if (LCHECK ("qty_back")) 
	{
		if (FLD ("qty_back") == NA)
		{
			local_rec.qty_back = 0.00;
			DSP_FLD ("qty_back");
		}

		if (dflt_used)
		{
			local_rec.qty_back = (float) ((BO_OK) ? local_rec.qty_ord - local_rec.qty_des : 0.00);
			DSP_FLD ("qty_back");
		}


		if (!BO_OK)
		{
			if (local_rec.qty_back != 0.00)
			{
				print_mess (ML (mlStdMess030));
				sleep (SLEEP_TIME);
				clear_mess ();
				local_rec.qty_back = 0.00;
				DSP_FLD ("qty_back");
			}
			return (EXIT_SUCCESS);
		}

		if ((BO_OK && 
			 local_rec.qty_back != local_rec.qty_ord - local_rec.qty_des) || 
			(!BO_OK && local_rec.qty_back != 0.00))
			invoiceChanged = TRUE;

		if (line_cnt >= numberOldLines)
		{
			local_rec.qty_ord = local_rec.qty_des + local_rec.qty_back;
			DSP_FLD ("qty_ord");
		}

		/*---------------------------------------------
		| log to lost sales if qty backorded reduced. |
		---------------------------------------------*/
		if (local_rec.qty_back < local_rec.qty_ord - local_rec.qty_des)
		{
			LogLostSales (local_rec.qty_ord - 
				  local_rec.qty_des - 
				  local_rec.qty_back);
		}

		SR._qty_back = local_rec.qty_back;

		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Price Input.	|
	-----------------------*/
	if (LCHECK ("sale_price")) 
	{
		if (BONUS)
		{
			coln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");

			print_mess (ML (mlSoMess233));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (prog_status != ENTRY)
				coln_rec.sale_price = SR._sale_price;

			if (SR._pricing_chk == FALSE)
			{
				PriceProcess ();
			}
		}

		if (coln_rec.sale_price == 0.0)
		{
			i = prmptmsg (ML (mlStdMess031) ,"YyNn",0,2);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		/*---------------------------------
		| Calculate _new GROSS sale price. |
		---------------------------------*/
		SR._gsale_price = no_dec (coln_rec.sale_price / (1.00 - (SR._reg_pc / 100.00)));
		SR._sale_price = GetCusGprice (SR._gsale_price, SR._reg_pc);
		coln_rec.sale_price = SR._sale_price;
#if 0
		if (line_cnt >= numberOldLines)
		{
			if (SR._pricing_chk == FALSE)
			{	
				PriceProcess ();
				DiscProcess ();
			}
		}
#endif
		SR._act_sale = coln_rec.sale_price;

		if (prog_status != ENTRY)
			CalculateTotalBox (FALSE, FALSE);

		SR._sale_price	=	coln_rec.sale_price;

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Discount Percent. |
	---------------------------*/
	if (LCHECK ("disc"))
	{
		if (dflt_used)
			DiscProcess ();

		if (SR._con_price)
		{
			coln_rec.disc_pc = 0.00;
			DSP_FLD ("disc");
		}

		SR._dis_pc  = ScreenDisc (coln_rec.disc_pc);

		/*------------------------------
		| Discount has been entered so |
		| set disc B & C to zero.      |
		------------------------------*/
		if (!dflt_used)
		{
			SR._disc_a = SR._dis_pc;
			SR._disc_b = 0.00;
			SR._disc_c = 0.00;
		}

		CalculateTotalBox (FALSE, FALSE);

		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate serial number input. |
	-------------------------------*/
	if (LCHECK ("ser_no")) 
	{
		if (!SERIAL_ITEM)
		{
			strcpy (local_rec.serial_no,ser_space);
			strcpy (SR.err_fnd, "N");
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}

		if (end_input)
		{
			/*--------------------------------------
			| If no serial item input and F16 hit. |
			--------------------------------------*/
			skip_entry = label ("qty_ord") - label ("ser_no");
			strcpy (SR.err_fnd, "N");
			return (EXIT_SUCCESS);
		}
	
		abc_selfield (insf,"insf_id_no");

		if (SRCH_KEY)
		{
			SrchInsf (temp_str,line_cnt);
			strcpy (SR.err_fnd, "N");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.serial_no,ser_space) && local_rec.qty_des > 0.00)
		{
			strcpy (store [line_cnt].err_fnd, "Y");

			print_mess (ML (mlStdMess201));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-------------------------------
		| If same as last serial number |
		| then no need to validate.     |
		-------------------------------*/
		if (!strcmp (local_rec.serial_no, SR._serial))
		{
			strcpy (SR.err_fnd, "N");
			return (EXIT_SUCCESS);
		}

		if (!dflt_used && strcmp (local_rec.serial_no, ser_space))
		{
			insf_rec.hhwh_hash = SR.hhwhHash;
			insf_rec.hhbr_hash = SR.hhbrHash;
			strcpy (insf_rec.status,"F");
			strcpy (insf_rec.serial_no,local_rec.serial_no);
			cc = find_rec (insf, &insf_rec,COMPARISON,"r");
			if (cc)
			{
				abc_selfield (insf,"insf_hhbr_id");

				insf_rec.hhwh_hash = SR.hhwhHash;
				insf_rec.hhbr_hash = SR.hhbrHash;
				strcpy (insf_rec.status,"F");
				strcpy (insf_rec.serial_no,local_rec.serial_no);
				cc = find_rec (insf, &insf_rec,COMPARISON,"r");
			}
			if (cc)
			{
				print_mess (ML (mlStdMess201));
				sleep (SLEEP_TIME);
				strcpy (store [lcount [ITEM_SCN]].err_fnd, "Y");
				return (EXIT_FAILURE);
			}

			if (insf_rec.receipted [0] != 'Y')
			{
				print_mess (ML (mlSoMess337));
				sleep (SLEEP_TIME);
				clear_mess ();
				strcpy (store [lcount [ITEM_SCN]].err_fnd, "Y");
				return (EXIT_FAILURE);
			}

			if (CheckDuplicateInsf (local_rec.serial_no,SR.hhbrHash,line_cnt))
			{
				print_mess (ML (mlStdMess097));
				sleep (SLEEP_TIME);
				strcpy (store [lcount [ITEM_SCN]].err_fnd, "Y");
				return (EXIT_FAILURE);
			}
		}

		/*----------------------------------------
		| free old insf and commit _new insf       |
		| no need to free if blank or same number |
		-----------------------------------------*/
		if (strcmp (SR._serial, ser_space) &&
			strcmp (SR._serial, local_rec.serial_no))
		{
			cc = UpdateInsf (SR.hhwhHash,SR.hhbrHash,SR._serial,"C","F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}
			
		/*--------------------------------------------
		| no need to commit if blank or same number   |
		---------------------------------------------*/
		if (strcmp (local_rec.serial_no, SR._serial) &&
		    strcmp (local_rec.serial_no, ser_space))
		{
			cc = UpdateInsf (SR.hhwhHash,
							SR.hhbrHash,
							local_rec.serial_no,"F","C");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}
		strcpy (SR._serial, local_rec.serial_no);
		strcpy (SR.err_fnd, "N");
		DSP_FLD ("ser_no");
		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Cost Price Input. |
	----------------------------*/
	if (LCHECK ("cost_price"))
	{
		if (!NO_COST)
		{
			print_mess (ML (mlSoMess335));
			coln_rec.cost_price = 0.00;
			sleep (SLEEP_TIME);
			clear_mess ();
			DSP_FLD ("cost_price");
		}
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

		if (PHANTOM || INDENT || NON_STOCK)
			return (EXIT_SUCCESS);

		LLInputClear = (line_cnt >= numberOldLines) ? TRUE : FALSE;

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
				SR._UOM,							/* UOM					*/
				SR._qty_des,						/* Quantity.			*/
				SR._cnv_fct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				LotSelectFlag,						/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR._lot_ctrl						/* Lot controled item. 	*/
													/*----------------------*/
		);

		DSP_FLD ("LL");
		/*-----------------
		| Redraw screens. |
		-----------------*/
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

		skip_entry = (local_rec.qty_des != 0.00 && SERIAL_ITEM && !strcmp (SR._serial,ser_space)) ? 3 : 4;
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate freight. |
	-------------------*/
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
		if (cohr_rec.freight == 0.00 && !dflt_used)
			strcpy (cohr_rec.frei_req, "N");

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Shipment Name And Addresses. |
	---------------------------------------*/
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

	/*---------------------------------
	| Validate Freight Required Flag. |
	---------------------------------*/
	if (LCHECK ("frei_req"))
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
			
			DSP_FLD ("freight");
			DSP_FLD ("est_freight");
			DSP_FLD ("tot_kg");
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
				sleep (SLEEP_TIME);
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
		sleep (SLEEP_TIME);
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
			sleep (SLEEP_TIME);
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
					sleep (SLEEP_TIME);
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
		move (0,2);cl_line ();
		i = prmptmsg (ML (mlTrMess063) ,"YyNn",0,2);
		BusyFunction (0);
		if (i == 'y' || i == 'Y') 
		{
			sprintf (err_str, "tr_trsh_mnt P %010ld LOCK", cohr_rec.hhco_hash);
			sys_exec (err_str);
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhco_hash");
			trsh_rec.hhco_hash	=	cohr_rec.hhco_hash;
			cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (cohr_rec.s_timeslot, trsh_rec.sdel_slot);
				strcpy (cohr_rec.e_timeslot, trsh_rec.edel_slot);
				cohr_rec.del_date	=	trsh_rec.del_date;
				DSP_FLD ("deliveryDate");
				heading (FRI_SCN);
				scn_write (FRI_SCN);
				scn_display (FRI_SCN);
				print_mess (ML (mlTrMess076));
				sleep (SLEEP_TIME);
			}
			
			return (EXIT_SUCCESS); 
		}
		DSP_FLD ("deliveryRequired");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
CalcLineExtended (
 int	line_no)
{
	if (store [line_no]._class [0] == 'Z')
	{
		store [line_no]._qty_des 	= 0.00;
		store [line_no]._sale_price = 0.00;
		store [line_no].itemLevy 	= 0.00;
		store [line_no]._act_sale 	= 0.00;
		store [line_no]._marg_cost 	= 0.00;
		store [line_no]._outer 		= 0.00;
		store [line_no]._dis_pc 	= 0.00;
		store [line_no]._tax_pc 	= 0.00;
		store [line_no]._tax_amt 	= 0.00;
		store [line_no]._gst_pc 	= 0.00;
	}
	/*-----------------------------------------------
	| Update coln gross tax and disc for each line. |
	-----------------------------------------------*/
	l_total = (double) store [line_no]._qty_des;
	l_total *= out_cost (store [line_no]._act_sale,store [line_no]._outer);
	l_total = no_dec (l_total);

	if (noTaxCharged)
		t_total = 0.00;
	else
	{
		t_total = (double) store [line_no]._qty_des;
		t_total *= out_cost (store [line_no]._tax_amt,store [line_no]._outer);
		t_total = no_dec (t_total);
	}

	l_disc = (double) (store [line_no]._dis_pc);
	l_disc = DOLLARS (l_disc);
	l_disc *= l_total;
	l_disc = no_dec (l_disc);

	if (envVar.advertLevy)
	{
		lineLevyPc 	= (double) (store [line_no].advertLevyPc);
		lineLevyPc 	= DOLLARS (lineLevyPc);
		lineLevyPc 	*= l_total;
		lineLevyPc 	= no_dec (lineLevyPc);

		lineLevyAmt = store [line_no].advertLevyAmt;
		lineLevyAmt *= (double) store [line_no]._qty_des;
		lineLevyAmt = no_dec (lineLevyAmt);
		l_levy	=	lineLevyAmt	+	lineLevyPc;
	}
	if (noTaxCharged)
		l_tax = 0.00;
	else
	{
		l_tax = (double) (store [line_no]._tax_pc);
		l_tax = DOLLARS (l_tax);

		if (cohr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
		{
			if (envVar.dbNettUsed)
				l_tax *= (l_total - l_disc + l_levy);
			else
				l_tax *= l_total + l_levy;
		}
		l_tax = no_dec (l_tax);
	}
	
	if (noTaxCharged)
		l_gst = 0.00;
	else
	{
		l_gst = (double) (store [line_no]._gst_pc);
		if (envVar.dbNettUsed)
			l_gst *= ((l_total - l_disc) + l_tax + l_levy);
		else
			l_gst *= (l_total + l_tax + l_levy);
		l_gst = DOLLARS (l_gst);
	}
}

int
ValidateItemNumber (
 int	getFields)
{
	int		i;
	int		itemChanged = FALSE;
	char	*sptr;
	long	orig_hhbr_hash;

	abc_selfield (inmr,"inmr_id_no");

	skip_entry = 0;

	if (dflt_used || !strcmp (local_rec.item_no,"                "))
		return (DeleteLine ());

	if (prog_status == ENTRY)
		sprintf (local_rec.serial_no,"%25.25s"," ");

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.item_no);

	SR._bonus [0] = (CheckBonusLine (inmr_rec.item_no)) ? 'Y' : 'N';
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
		cc = 	check_indent 
				(	
					comm_rec.co_no, 
					comm_rec.est_no,
					ccmr_rec.hhcc_hash,
					inmr_rec.item_no 
				);
		if (cc)
		{
			/*-----------------
			| Item not found. |
			-----------------*/
			print_mess (ML (mlStdMess001));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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
			print_mess (ML (mlStdMess001));
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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
			sleep (SLEEP_TIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		itemChanged = TRUE;
		local_rec.qty_ord = 0.00;
		if (strcmp (SR._serial, ser_space) && SR._ser_item [0] == 'Y')
		{
			cc = UpdateInsf (SR.hhwhHash, SR.hhbrHash, SR._serial, "C", "F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}
		strcpy (local_rec.serial_no, ser_space);
		strcpy (SR._serial, local_rec.serial_no);
		strcpy (SR._org_ser, local_rec.serial_no);
		DSP_FLD ("ser_no");
	}

	SR.hhbrHash 	= 	inmr_rec.hhbr_hash;
	SR.hhsiHash 	= 	alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	SR._qty_des 	= 	0.00;
	SR._qty_back 	= 	0.00;
	SR._weight 		= 	inmr_rec.weight;
	SR._outer 		= 	inmr_rec.outer_size;
	SR._gst_pc 		= 	inmr_rec.gst_pc;
	SR._tax_pc 		= 	inmr_rec.tax_pc;
	SR._tax_amt 	= 	inmr_rec.tax_amount;
	SR._dis_pc 		= 	inmr_rec.disc_pc;
	SR._dflt_disc	= 	inmr_rec.disc_pc;
	SR._class [0] 	= 	inmr_rec.inmr_class [0];
	SR._marg_cost 	= 	GetMargCost (FALSE);
	strcpy (SR._category,  inmr_rec.category);
	strcpy (SR._sellgrp,   inmr_rec.sellgrp);
	strcpy (SR._border,    inmr_rec.bo_flag);
	strcpy (SR._release,   inmr_rec.bo_release);
	strcpy (SR._pack_size, inmr_rec.pack_size);
	strcpy (SR._lot_ctrl,  inmr_rec.lot_ctrl);
	strcpy (SR._cost_flag, inmr_rec.costing_flag);
	sprintf (SR._serial,   "%-25.25s", " ");
	sprintf (SR._org_ser,  "%-25.25s", " ");
	strcpy (local_rec.UOM, inmr_rec.sale_unit);

    /*---------------------
    | Find for UOM GROUP. |
    ----------------------*/
    strcpy (inum_rec.uom, inmr_rec.sale_unit);
    cc = find_rec (inum, &inum_rec, EQUAL, "r");
    if (cc)
        file_err (cc, inum, "DBFIND");

    SR.hhumHash   = inum_rec.hhum_hash;
    SR._cnv_fct     = inum_rec.cnv_fct;
    SR._std_cnv_fct = inum_rec.cnv_fct;

	strcpy (SR._UOM,		inum_rec.uom);
	strcpy (SR._uom_group,	inum_rec.uom_group);

	/*-------------------------
	| Check for Indent items. |
	-------------------------*/
	if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVar.discountIndents)
		SR._indent = FALSE;
	else
		SR._indent = TRUE;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					  inmr_rec.hhsi_hash);
	cc = find_rec (incc, &incc_rec,COMPARISON,"r");
	if (cc) 
	{
		i = prmptmsg (ML (mlStdMess033) ,"YyNn",0,2);
		BusyFunction (0);
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
		}
	}

	SR.hhbrHash = inmr_rec.hhbr_hash;
	SR.hhccHash = ccmr_rec.hhcc_hash;

	if (inmr_rec.hhbr_hash != orig_hhbr_hash) 
	{
		BusyFunction (0);
		sprintf (err_str, ML (mlSoMess234) ,clip (local_rec.sup_part),clip (local_rec.item_no),BELL);
		sleep (SLEEP_TIME);
		print_mess (err_str);
	}

	coln_rec.line_no = -1;
	local_rec.qty_ord = 0.00;
	local_rec.qty_des = 0.00;
	local_rec.qty_back = 0.00;
	strcpy (coln_rec.item_desc,inmr_rec.description);
	sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.description);
	sprintf (SR._item_desc, "%-40.40s", inmr_rec.description);

	if (BONUS)
	{
		sprintf (so_bonus, "%-2.2s", envVar.soSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no,"%-s%-.*s", sptr,16 - (int) strlen (sptr),so_bonus);

		coln_rec.tax_pc  = 0.00;
		coln_rec.gst_pc  = 0.00;
	}

	if (SR._tax_amt == 0.00)
		SR._tax_amt = SR._act_sale;

	strcpy (coln_rec.pack_size,inmr_rec.pack_size);
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
		SR.advertLevyPc  = inal_rec.percent;
	}
	else
	{
		SR.advertLevyAmt = 0.00;
		SR.advertLevyPc  = 0.00;
	}
	DSP_FLD ("item_no");
	DSP_FLD ("descr");
	DSP_FLD ("pack_size");

	if (!NO_COST)
	{
		coln_rec.cost_price = 0.00;
		DSP_FLD ("cost_price");
	}
	SR.actHhwhHash = incc_rec.hhwh_hash;
	SR._dec_pt = inmr_rec.dec_pt;

	/*------------------------
	| Item is a serial item. |
	------------------------*/
	if (inmr_rec.serial_item [0] == 'Y')
	{
		SR.hhwhHash = incc_rec.hhwh_hash;
		strcpy (SR._ser_item, "Y");
		if (FLD ("ser_no") == NA)
			FLD ("ser_no") = YES;
		DSP_FLD ("descr");
	}
	else
	{
		SR.hhwhHash = -1L;
		strcpy (SR._ser_item, "N");
		if (FLD ("ser_no") == YES)
			FLD ("ser_no") = NA;
	}
	
	if (itemChanged)
	{
		local_rec.qty_ord  = 0.00;
		local_rec.qty_des  = 0.00;
		local_rec.qty_back = 0.00;
		SR._qty_des  = local_rec.qty_des;
		SR._qty_back = local_rec.qty_back;
		DSP_FLD ("qty_ord");
		DSP_FLD ("qty_des");
		DSP_FLD ("qty_back");
		if (SR._pricing_chk == FALSE)
		{
			PriceProcess ();
			DiscProcess ();
		}
		CalculateTotalBox (FALSE, FALSE);
	}

	if (NON_STOCK)
		skip_entry = goto_field (label ("item_no"), label ("ser_no")) + 1;
	else
		skip_entry = 3;

	sptr = clip (inmr_rec.description);

	if (itemChanged && getFields)
	{
		if (SR._ser_item [0] == 'Y')
		{
			local_rec.qty_ord = 1.00;
			local_rec.qty_des = 1.00;
			DSP_FLD ("qty_des");
			cc = spec_valid (label ("qty_des"));
			while (cc && !restart && !skip_entry)
			{
				get_entry (label ("qty_des"));
				cc = spec_valid (label ("qty_des"));
			}
			SR._qty_des = local_rec.qty_des;
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
				get_entry (label ("qty_des"));
				cc = spec_valid (label ("qty_des"));
			} while (cc && !restart);
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_des");
		}
	}

	if (strlen (sptr) == 0)
		skip_entry = 2;

	if (!F_HIDE (label ("hide")))
		DSP_FLD ("hide");

	tab_other (line_cnt);

	local_rec.line_no = coln_rec.line_no + 1;
	DSP_FLD ("dsp_line_no");

	return (EXIT_SUCCESS);
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
	est_freight 		= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	for (i = 0;i < lcount [ITEM_SCN];i++)
	{
		weight = (store [i]._weight > 0.00) ? store [i]._weight 
										     : comr_rec.frt_mweight;
		totalKgs += (weight * store [i]._qty_des);
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
		est_freight = comr_rec.frt_min_amt;
	else
		est_freight = freightValue;
	
	if (FREIGHT_CHG && envVar.automaticFreight)
	{
		if (envVar.soFreightCharge == 1)
		{
			cohr_rec.freight = CENTS (zoneFixedAmount);
			est_freight 	 = CENTS (zoneFixedAmount);
		}
		else
			cohr_rec.freight = est_freight;
	}
	cohr_rec.no_kgs = totalKgs;

	if (sohr_rec.sohr_new [0] == 'N' && envVar.soFreightBord)
	{
		cohr_rec.freight = 0.00;
		est_freight  	 = 0.00;
	}

	if (inScreenFlag)
		PrintTotalBoxValues ();

	return;
}

void
PriceProcess (
 void)
{
	int		pType;
	float	regPc;

	SR._pricing_chk	= FALSE;

	if (BONUS)
	{
		coln_rec.sale_price 	= 0.00;
		SR._act_sale 			= 0.00;
		SR._sale_price 			= 0.00;
		SR.itemLevy 			= 0.00;
		DSP_FLD ("sale_price");

		coln_rec.disc_pc  		= 0.00;
		SR._dis_pc 	 			= 0.00;
		DSP_FLD ("disc");
		return;
	}
	pType = atoi (cohr_rec.pri_type);

	SR._gsale_price	=	GetCusPrice 
						(
							comm_rec.co_no,
							comm_rec.est_no,
							comm_rec.cc_no,
							cohr_rec.area_code,
							cumr_rec.class_type,
							SR._sellgrp,
							cumr_rec.curr_code,
							pType,
							cumr_rec.disc_code,
							cnch_rec.exch_type,
							cumr_rec.hhcu_hash,
							SR.hhccHash,
							SR.hhbrHash,
							SR._category,
							cnch_rec.hhch_hash,
							(envVar.soDoi) ? des_date : comm_rec.dbt_date,
							ToStdUom (local_rec.qty_des + local_rec.qty_back),
							pocrRec.ex1_factor,
							FGN_CURR,
							&regPc
						);

	SR._pricing_chk	= TRUE;

	SR._sale_price = GetCusGprice (SR._gsale_price, regPc);

	SR._reg_pc = regPc;
	coln_rec.sale_price = SR._sale_price;
	SR._act_sale 		= SR._sale_price;

	SR._con_price 		= (_CON_PRICE) ? TRUE : FALSE;
	SR._cont_status  	= _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess (
 void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (SR._cont_status == 2 || SR._con_price || SR._indent)
	{
		coln_rec.disc_pc  	= 0.00;
		SR._dis_pc 	 		= 0.00;
		SR._disc_a			= 0.00;
		SR._disc_b			= 0.00;
		SR._disc_c			= 0.00;
		DSP_FLD ("disc");
		return;
	}

	if (SR._pricing_chk == FALSE)
		PriceProcess ();

	pType = atoi (cumr_rec.price_type);
	cumDisc		=	GetCusDisc (	comm_rec.co_no,
								comm_rec.est_no,
								SR.hhccHash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								SR.hhsiHash,
								SR._category,
								SR._sellgrp,
								pType,
								SR._gsale_price,
								SR._reg_pc,
								ToStdUom (local_rec.qty_des + local_rec.qty_back),
								discArray);
							
	SR._dis_pc			=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);
	coln_rec.disc_pc 	=	ScreenDisc (SR._dis_pc);

	SR._disc_a 			= discArray [0];
	SR._disc_b 			= discArray [1];
	SR._disc_c 			= discArray [2];
	SR._cumulative 		= cumDisc;

	if (SR._dflt_disc > ScreenDisc (coln_rec.disc_pc) &&
		SR._dflt_disc != 0.0)
	{
		coln_rec.disc_pc = ScreenDisc (SR._dflt_disc);
		SR._disc_a 		= SR._dflt_disc;
		SR._disc_b 		= 0.00;
		SR._disc_c 		= 0.00;
	}
	SR._dis_pc = ScreenDisc (coln_rec.disc_pc);

	DSP_FLD ("disc");
}

void
CalculateTotalBox (
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

	no_lines = (prog_status == ENTRY) ? line_cnt : lcount [ITEM_SCN] - 1;

	inv_tot 	= 0.00;
	dis_tot 	= 0.00;
	tax_tot 	= 0.00;
	tot_tot 	= 0.00;
	levy_tot 	= 0.00;
	gst_tot 	= 0.00;

	if (override)
		scn_set (ITEM_SCN);

	for (i = 0;i <= no_lines;i++) 
	{
		if (override)
		{
			getval (i);
			if (clear_disc)
			{
				coln_rec.disc_pc = 0.00;
				store [i]._dis_pc = 0.00;
			}
			else
			{
				coln_rec.disc_pc += ScreenDisc (local_rec.disc_over);
				store [i]._dis_pc = ScreenDisc (coln_rec.disc_pc);
			}
			putval (i);
		}
		CalcLineExtended (i);

		inv_tot 	+= l_total;
		dis_tot 	+= l_disc;
		tax_tot 	+= l_tax;
		gst_tot 	+= l_gst;
		levy_tot 	+= l_levy;
	}

	gst_tot = no_dec (gst_tot);
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
	gst_tot += wk_gst;
	gst_tot = no_dec (gst_tot);

	if (envVar.dbNettUsed)
		tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + other + levy_tot);
	else
		tot_tot = no_dec (inv_tot + tax_tot + gst_tot + other + levy_tot);

	if (override > 1 && !clear_disc)
	{
		if (tot_tot != local_rec.inp_total)
		{
			diff = no_dec (local_rec.inp_total - tot_tot);

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
			gst_tot += gst_amount;
			gst_tot = no_dec (gst_tot);
		}
	}
	if (!override)
		PrintTotalBoxValues ();

	local_rec.disc_over = 0.00;
}

void
CalcInputTotal (
 void)
{
	CalculateTotalBox (2,TRUE);

	if (cohr_rec.tax_code [0] == 'D')
		local_rec.disc_over = CalcOverideValues (tot_tot - tax_tot);
	else
		local_rec.disc_over = CalcOverideValues (tot_tot);

	CalculateTotalBox (2,FALSE);
}

/*===========================
| Calculate overide amount. |
===========================*/
float	
CalcOverideValues (
 double	amt)
{
	float	wk_disc = 0.00;

	if (amt == 0.00)
		return (0.00);

	wk_disc = (float) (OVER_AMT (tot_tot, local_rec.inp_total, amt));
	wk_disc *= 100;
	wk_disc = (float) (twodec (wk_disc));
	return (wk_disc);
}

void
DrawTotals (
 void)
{
	crsr_off ();
	box (96,0,35,4);
	print_at (1,97, ML (mlSoMess064));
	print_at (2,97, ML (mlSoMess065));
	print_at (3,97, ML (mlSoMess066), envVar.gstCode);
	print_at (4,97, ML (mlSoMess068));

	PrintTotalBoxValues ();
	crsr_on ();
}

void
PrintTotalBoxValues (
 void)
{
	f_other = 	cohr_rec.freight + 	
		  		cohr_rec.sos +
		  		cohr_rec.insurance -
		  		cohr_rec.ex_disc + 
		  		cohr_rec.other_cost_1 +
		  		cohr_rec.other_cost_2 + 	
		  		cohr_rec.other_cost_3;

	if (envVar.dbNettUsed)
		tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + f_other + levy_tot);
	else
		tot_tot = no_dec (inv_tot + tax_tot + gst_tot + f_other + levy_tot);
	
	crsr_off ();
	move (115,1);
	if (envVar.dbNettUsed)
		print_at (1,115,"%14.2f",DOLLARS (inv_tot - dis_tot + levy_tot));
	else
		print_at (1,115,"%14.2f",DOLLARS (inv_tot + levy_tot));
	print_at (2,115,"%14.2f",DOLLARS (f_other));
	print_at (3,115,"%14.2f",DOLLARS (gst_tot + tax_tot));
	print_at (4,115,"%14.2f",DOLLARS (tot_tot));
	crsr_on ();
}
/*=======================================================================
| Routine to read all coln records whoose hash matches the one on the   |
| sohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
int
LoadItemScreen (
 long	HHCO_HASH)
{
	char	*sptr;
	float	std_cnv_fct;

	hiddenLineFound = FALSE;

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] 	= 0;
	vars [scn_start].row = 0;

	print_at (2,0, ML (mlStdMess035));

	numberOldLines = 0;
	nextLineNo = 0;

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (inum,"inum_hhum_hash");

	LoadSONS (TRUE, 0, cohr_rec.hhco_hash);

	coln_rec.hhco_hash 	= HHCO_HASH;
	coln_rec.line_no 	= 0;

	cc = find_rec (coln, &coln_rec,GTEQ,"r");

	while (!cc && HHCO_HASH == coln_rec.hhco_hash) 
	{
		if (coln_rec.line_no % 5 == 0)
		{
			putchar ('R');
			fflush (stdout);
		}
		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec,COMPARISON,"r");
		if (cc) 
			file_err (cc, inmr, "DBFIND");

		sprintf (local_rec.item_desc, "%-40.40s", coln_rec.item_desc);
		sprintf (LSR._item_desc, "%-40.40s", coln_rec.item_desc);

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		std_cnv_fct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		strcpy (local_rec.UOM, inum_rec.uom);
		line_cnt	= lcount [ITEM_SCN];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		LSR._cnv_fct 				= inum_rec.cnv_fct/std_cnv_fct;
		LSR._std_cnv_fct 			= std_cnv_fct;
		LSR.hhumHash 				= inum_rec.hhum_hash;
		coln_rec.sale_price			*=	LSR._cnv_fct;
		coln_rec.gsale_price		*=	LSR._cnv_fct;

		strcpy (local_rec.UOM, 		inum_rec.uom);
		strcpy (LSR._UOM, 			inum_rec.uom);
		strcpy (LSR._uom_group, 	inum_rec.uom_group);

		if (inmr_rec.serial_item [0] == 'Y' && F_HIDE (label ("ser_no")))
		{
			print_mess (ML (mlSoMess245));
			sleep (SLEEP_TIME);
			scn_set (MAIN_SCN);
			return (EXIT_FAILURE);
		}
		if (coln_rec.hide_flag [0] == 'Y')
		{
			sprintf (so_hide,"%-2.2s", envVar.soSpecial + 2);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s", 
								sptr,16 - (int) strlen (sptr),so_hide);

			hiddenLineFound = TRUE;
		}
		else
		{
			strcpy (coln_rec.hide_flag, "N");
			strcpy (local_rec.item_no,inmr_rec.item_no);
		}

		if (coln_rec.bonus_flag [0] == 'Y')
		{
			sprintf (so_bonus, "%-2.2s", envVar.soSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
								sptr,16 - (int) strlen (sptr),so_bonus);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);
	
		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						   				   inmr_rec.hhsi_hash);

		cc = find_rec (incc, &incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = AddIncc (incc_rec.hhcc_hash, incc_rec.hhbr_hash);
			if (cc)
				file_err (cc, incc, "DBADD");
		}


		if (inmr_rec.serial_item [0] == 'Y')
		{
			strcpy (LSR._ser_item, "Y");
			LSR.hhwhHash = incc_rec.hhwh_hash;
			sprintf (local_rec.serial_no,"%-25.25s",coln_rec.serial_no);
			sprintf (LSR._serial,	"%-25.25s",	coln_rec.serial_no);
			sprintf (LSR._org_ser,	"%-25.25s",	coln_rec.serial_no);
		}
		else
		{
			strcpy (LSR._ser_item, "N");
			LSR.hhwhHash = -1L;
			sprintf (local_rec.serial_no,"%-25.25s"," ");
			sprintf (LSR._serial,"%-25.25s"," ");
			sprintf (LSR._org_ser,"%-25.25s"," ");
		}

		LSR.actHhwhHash 	= incc_rec.hhwh_hash;
		LSR._dec_pt			= inmr_rec.dec_pt;
		LSR.hhbrHash 		= coln_rec.hhbr_hash;
		LSR.hhsiHash 		= alt_hash 
							(
								inmr_rec.hhbr_hash, 
								inmr_rec.hhsi_hash
							);
		LSR.hhccHash 		= coln_rec.incc_hash;
		LSR.hhslHash 		= coln_rec.hhsl_hash;
		LSR.hhclHash 		= coln_rec.hhcl_hash;
		LSR._qty_des 		= ToLclUom (coln_rec.q_order);
		LSR._qty_back 		= ToLclUom (coln_rec.q_backorder);
		LSR._gst_pc 		= inmr_rec.gst_pc;
		LSR._tax_pc 		= inmr_rec.tax_pc;
		LSR._tax_amt 		= DPP (inmr_rec.tax_amount);
		LSR._dflt_disc		= inmr_rec.disc_pc;
		LSR._dis_pc 		= coln_rec.disc_pc;
		LSR._act_sale 		= DPP (coln_rec.sale_price);
		LSR.itemLevy 		= DPP (coln_rec.item_levy);
		LSR._reg_pc 		= coln_rec.reg_pc;
		LSR._disc_a 		= coln_rec.disc_a;
		LSR._disc_b 		= coln_rec.disc_b;
		LSR._disc_c 		= coln_rec.disc_c;
		LSR._cumulative 	= coln_rec.cumulative;
		LSR._gsale_price 	= DPP (coln_rec.gsale_price);
		LSR._sale_price 	= DPP (coln_rec.sale_price);
		LSR._pricing_chk	= TRUE;
		LSR._outer 			= inmr_rec.outer_size;
		LSR._weight 		= inmr_rec.weight;
		if (LSR._tax_amt == 0.00)
			LSR._tax_amt = DPP (coln_rec.sale_price);

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
			LSR.advertLevyPc  = inal_rec.percent;
		}
		else
		{
			LSR.advertLevyAmt = 0.00;
			LSR.advertLevyPc  = 0.00;
		}
		LSR._marg_cost 	= GetMargCost (TRUE);

		/*----------------------------------------------
		| if the line has a contract on it then  user  |
		| not allowed to edit price or disc            |
		----------------------------------------------*/
		LSR._cont_status = coln_rec.cont_status;

		/*-------------------------
		| Check for Indent items. |
		-------------------------*/
		if (strncmp (inmr_rec.item_no,"INDENT",6) || envVar.discountIndents)
			LSR._indent = FALSE;
		else
			LSR._indent = TRUE;

		strcpy (LSR._bonus,     coln_rec.bonus_flag);
		strcpy (LSR._border,    inmr_rec.bo_flag);
		strcpy (LSR._release,   inmr_rec.bo_release);
		strcpy (LSR._pack_size, inmr_rec.pack_size);
		strcpy (LSR._sellgrp,   inmr_rec.sellgrp);
		strcpy (LSR._lot_ctrl,  inmr_rec.lot_ctrl);
		strcpy (LSR._cost_flag, inmr_rec.costing_flag);
		strcpy (LSR._class,     inmr_rec.inmr_class);
		strcpy (LSR._category,  inmr_rec.category);
		strcpy (LSR._desc2,  inmr_rec.description2);

		local_rec.qty_ord = ToLclUom (coln_rec.q_order + coln_rec.q_backorder);
		local_rec.qty_des = ToLclUom (coln_rec.q_order);
		if (local_rec.qty_des > local_rec.qty_ord)
			local_rec.qty_des = local_rec.qty_ord;

		local_rec.qty_back = (float) ((inmr_rec.bo_flag [0] == 'Y') ? 
						local_rec.qty_ord - local_rec.qty_des : 0.00);
	
		if (coln_rec.line_no > nextLineNo)
			nextLineNo = coln_rec.line_no;

		if (inmr_rec.serial_item [0] == 'Y')
		{
			if (!strcmp (local_rec.serial_no, ser_space))
				strcpy (LSR.err_fnd, "Y");
			else
			{
				abc_selfield (insf, "insf_id_no");
				insf_rec.hhwh_hash = LSR.hhwhHash;
				sprintf (insf_rec.status, "%-1.1s", "C");
				strcpy (insf_rec.serial_no, local_rec.serial_no);

				/*----------------------------------------------
				| Check if serial item exists,if error then it |
				| will belong to another branch so clear out.  |
				----------------------------------------------*/
				if (find_rec (insf, &insf_rec, COMPARISON, "r"))
				{
					UpdateInsf 
					(
						LSR.hhwhHash,
						LSR.hhbrHash,
						local_rec.serial_no, "C", "F"
					);

					strcpy (LSR.err_fnd, "Y");
					strcpy (local_rec.serial_no,ser_space);
					sprintf (LSR._serial,  "%-25.25s"," ");
					sprintf (LSR._org_ser, "%-25.25s"," ");
				}
				else
					strcpy (LSR.err_fnd, "N");
			}
		}

		coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);

		LoadSONS (FALSE, lcount [2], coln_rec.hhcl_hash);

		local_rec.line_no = coln_rec.line_no + 1;

		if (FLD ("LL") != ND)
		{
			cc = Load_LL_Lines
			(
				lcount [ITEM_SCN],
				LL_LOAD_INV,
				coln_rec.hhcl_hash,
				LSR.hhccHash,
				LSR._UOM,
				LSR._cnv_fct,
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
						LSR.actHhwhHash, 						
						LSR.hhumHash,						
						LSR.hhccHash,						
						LSR._UOM,							
						local_rec.qty_des,
						LSR._cnv_fct,						
						TodaysDate (),
						TRUE,
						FALSE,
						LSR._lot_ctrl						
					);
			}
			strcpy (local_rec.LL, "Y");
		}
		putval (lcount [ITEM_SCN]++);
		if (lcount [ITEM_SCN] > MAXLINES) 
			break;

		cc = find_rec (coln, &coln_rec,NEXT,"r");
	}
		
	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (inum,"inum_uom");
	numberOldLines = lcount [ITEM_SCN];
	nextLineNo++;

	vars [scn_start].row = lcount [ITEM_SCN];
	scn_set (MAIN_SCN);

	return (EXIT_SUCCESS);
}

double	
GetMargCost (
 int chkColn)
{
	double	margCost;

	/*--------------------
	| Check coln price ? |
	--------------------*/
	if (chkColn)
	{
		if (coln_rec.cost_price != (double)0.00)
			return (DOLLARS (coln_rec.cost_price));
	}
	
	/*-------------------------------------
	| Lookup inei record for item/branch. |
	-------------------------------------*/
	inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, 
								   inmr_rec.hhsi_hash);
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
		return ((double)0.00);

	margCost = (inei_rec.avge_cost == 0.00) ? inei_rec.last_cost 
											   : inei_rec.avge_cost;

	return (margCost);
}

void
Confirm (
 void)
{
	scn_set (ITEM_SCN);

	print_at (2,0, ML (mlSoMess143));
	fflush (stdout);

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt);

		putchar ('C');
		fflush (stdout);

		local_rec.qty_ord = local_rec.qty_des + local_rec.qty_back;
		local_rec.qty_des = local_rec.qty_ord - local_rec.qty_back;
		local_rec.qty_back = (float) ((BO_OK) ? local_rec.qty_ord - local_rec.qty_des : 0.00);
		putval (line_cnt);
	}
	move (0,2);cl_line ();
}

/*===================
| Update all files. |
===================*/
void
Update (void)
{
	int		ns_ok 			= FALSE,
			PslipLine 		= 0;

	float	totalQtyOrd		= 0.00;
	double	value 			= 0.00,
			wk_value 		= 0.00,
			invoice_tot 	= 0.00,
			s_total 		= 0.00,
			s_order 		= 0.00;

	char	tot_str [21];

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		noTaxCharged = 1;
	else
		noTaxCharged = 0;

	strcpy (local_rec.prev_order,cohr_rec.inv_no);
	strcpy (local_rec.prev_dbt_no,cumr_rec.dbt_no);
	strcpy (cohr_rec.carr_code, trcm_rec.carr_code);
	strcpy (cohr_rec.del_zone,  trzm_rec.del_zone);

	if (local_rec.inp_total != 0.00)
		CalcInputTotal ();

	cohr_rec.item_levy 	= 0.00;
	cohr_rec.gross 		= 0.00;
	cohr_rec.disc 		= 0.00;
	cohr_rec.tax 		= 0.00;
	cohr_rec.gst 	= 0.00;

	clear ();
	fflush (stdout);

	scn_set (ITEM_SCN);

	print_at (0,0, ML (mlSoMess144));
	fflush (stdout);

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		/*-----------
		| Get data	|
		-----------*/
		getval (line_cnt);

		/*-------------------
		| the orignal lines	|
		-------------------*/
		if (line_cnt < numberOldLines)
		{
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			/*---------------------------
			| Find Original coln record	|
			---------------------------*/
			cc = find_rec (coln, &coln_rec,COMPARISON,"u");
			if (cc)
			{
				abc_unlock (coln);
				continue;
			}
			PslipLine = coln_rec.hhsl_hash;
			/*---------------------------
			| Find Original soln record	|
			---------------------------*/
			soln_rec.hhsl_hash = coln_rec.hhsl_hash;
			cc = find_rec (soln, &soln_rec,EQUAL,"u");
			if (!cc)
			{
				/*---------------------------------------
				| Non Stock Item, But Item it belongs	|
				| to has been deleted.					|
				---------------------------------------*/
				if (NON_STOCK && !ns_ok)
				{
					if (!envVar.salesOrderRealTimeDelete)
					{
						soln_rec.qty_order = 0.00;
						soln_rec.qty_bord  = 0.00;
						strcpy (soln_rec.status, "D");
						strcpy (soln_rec.stat_flag, "D");
						cc = abc_update (soln, &soln_rec);
						if (cc)
							file_err (cc,soln,"DBUPDATE");
					}
					else
					{
						cc = ArchiveSoln (soln_rec.hhsl_hash);
						if (cc)
							file_err (cc, soln, "ARCHIVE");

						cc = abc_delete (soln);
						if (cc)
							file_err (cc,soln,"DBDELETE");
					}
					/*----------------------------------------------
					| Check if header needs to be deleted/updated. |
					----------------------------------------------*/
					DeleteSalesOrders (soln_rec.hhso_hash);
				}
				else
				{
					/*---------------------------------------------
					| Item is non stock and parent placed on B/O. |
					---------------------------------------------*/
					if (NON_STOCK)
					{
						strcpy (soln_rec.stat_flag,"B");
						strcpy (soln_rec.status,"B");
						cc = abc_update (soln, &soln_rec);
						if (cc)
							file_err (cc,soln,"DBUPDATE");
					}
					ns_ok = TRUE;
				}
			}
			add_hash 
			(	
				comm_rec.co_no, 
				comm_rec.est_no, 
				"RC", 
				0,
				SR.hhbrHash, 
				SR.hhccHash, 
				0L, 
				(double) 0.00
			);

			/*-----------
			| Get data	|
			-----------*/
			getval (line_cnt);
		}
		else
		{
			coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
			coln_rec.line_no 	= nextLineNo++;
			coln_rec.hhbr_hash 	= SR.hhbrHash;
			coln_rec.incc_hash 	= SR.hhccHash;
			coln_rec.hhum_hash 	= SR.hhumHash;
			coln_rec.tax_pc 	= (float) ((noTaxCharged) ? 0.00 : SR._tax_pc);
			coln_rec.gst_pc 	= (float) ((noTaxCharged) ? 0.00 : SR._gst_pc);
			coln_rec.due_date 	= 
						(envVar.soDoi) ? TodaysDate () : comm_rec.dbt_date;

			strcpy (coln_rec.bonus_flag,SR._bonus);
		}

		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,SR._category);
		cc = find_rec (excf, &excf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc,excf,"DBFIND");

		soln_rec.n_xrate = excf_rec.ex_rate;
		coln_rec.o_xrate = soln_rec.o_xrate;
		coln_rec.n_xrate = soln_rec.n_xrate;


		if (findStatusFlag [0] == 'I')
		{
			if (coln_rec.status [0] == 'P')
				strcpy (coln_rec.status,"I");
		}
		if (findStatusFlag [0] == 'T')
		{
			if (coln_rec.status [0] == 'P')
				strcpy (coln_rec.status,"T");
		}
		strcpy (coln_rec.stat_flag, (TRANSPORT) ? findStatusFlag : createStatusFlag);

		if (BONUS)
		{	
			coln_rec.tax_pc  	= 0.00;
			coln_rec.gst_pc  	= 0.00;
			coln_rec.sale_price = 0.00;
			coln_rec.item_levy 	= 0.00;
			coln_rec.disc_pc 	= 0.00;
		}

		coln_rec.q_order 		= ToStdUom (local_rec.qty_des);
		coln_rec.q_backorder 	= ToStdUom (local_rec.qty_back);

		coln_rec.reg_pc 		=	SR._reg_pc;
		coln_rec.disc_a 		=	SR._disc_a;
		coln_rec.disc_b 		=	SR._disc_b;
		coln_rec.disc_c 		=	SR._disc_c;
		coln_rec.cumulative 	=	SR._cumulative;
		if (SR._cnv_fct == 0.00)
			SR._cnv_fct = 1.00;
		coln_rec.gsale_price 	=	DPP (SR._gsale_price / SR._cnv_fct);
		coln_rec.sale_price 	=	DPP (SR._sale_price / SR._cnv_fct);
		coln_rec.cont_status 	=	SR._cont_status;
		coln_rec.item_levy	 	=	DPP (SR.itemLevy);

		strcpy (coln_rec.serial_no, SR._serial);

		sprintf (coln_rec.item_desc, "%40.40s", SR._item_desc);

		if (noTaxCharged)
		{
			coln_rec.tax_pc 	= 0.00;
			coln_rec.gst_pc 	= 0.00;
		}

		totalQtyOrd += coln_rec.q_order;
		s_order = (double) local_rec.qty_des + local_rec.qty_back;
		s_order *= out_cost (SR._sale_price,inmr_rec.outer_size);
		s_total += s_order;
		s_total = no_dec (s_total);

		/*-------------------------------
		| Perform Invoice Calculations	|
		-------------------------------*/
		l_total = local_rec.qty_des * out_cost (SR._sale_price, SR._outer);
		l_total = no_dec (l_total);

		if (noTaxCharged)
			t_total = 0.00;
		else
		{
			t_total = (double) local_rec.qty_des;
			t_total *= out_cost (SR._tax_amt, SR._outer);
			t_total = no_dec (t_total);
		}

		l_disc = (double) ScreenDisc (coln_rec.disc_pc);
		l_disc = DOLLARS (l_disc);
		l_disc *= l_total;
		l_disc = no_dec (l_disc);

		if (envVar.advertLevy)
		{
			lineLevyPc 	= (double) (SR.advertLevyPc);
			lineLevyPc 	= DOLLARS (lineLevyPc);
			lineLevyPc 	*= l_total;
			lineLevyPc 	= no_dec (lineLevyPc);

			lineLevyAmt = SR.advertLevyAmt;
			lineLevyAmt *= (double) SR._qty_des;
			lineLevyAmt = no_dec (lineLevyAmt);
			l_levy		= lineLevyPc + lineLevyAmt;
		}

		if (noTaxCharged)
			l_tax = 0.00;
		else
		{
			l_tax = (double) SR._tax_pc;
			l_tax = DOLLARS (l_tax);

			if (cumr_rec.tax_code [0] == 'D')
				l_tax *= t_total;
			else
			{
				if (envVar.dbNettUsed)
					l_tax *= (l_total - l_disc + l_levy);
				else
					l_tax *= l_total + l_levy;
			}
			l_tax = no_dec (l_tax);
		}
		if (noTaxCharged)
			l_gst = 0.00;
		else
		{
			l_gst = (double) SR._gst_pc;
			if (envVar.dbNettUsed)
				l_gst *= ((l_total - l_disc) + l_tax + l_levy);
			else
				l_gst *= (l_total + l_tax + l_levy);
			l_gst = DOLLARS (l_gst);
		}
		coln_rec.gross    	= l_total;
		coln_rec.amt_disc 	= l_disc;
		coln_rec.amt_tax  	= l_tax;
		coln_rec.amt_gst  	= l_gst;
		coln_rec.item_levy  = l_levy;

		coln_rec.hhah_hash = SR.hhahHash;

		cohr_rec.item_levy	+= l_levy;
		cohr_rec.gross 		+= l_total;
		cohr_rec.disc 		+= l_disc;
		cohr_rec.tax   		+= l_tax;
		cohr_rec.gst   		+= l_gst;

		if (line_cnt < numberOldLines)
		{
			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			cc = abc_update (coln, &coln_rec);
			if (cc)
				file_err (cc, "coln", "DBUPDATE");

			UpdateSONS (FALSE, line_cnt, coln_rec.hhcl_hash);

			putchar ('U');
			fflush (stdout);

			if (PslipLine)
			{
				if (!NON_STOCK)
				{
					/*-------------------
					| backorder qty = 0	|
					| so delete soln	|
					-------------------*/
					if (local_rec.qty_back == 0.00)
					{
						if (!envVar.salesOrderRealTimeDelete)
						{
							soln_rec.qty_order = 0.00;
							soln_rec.qty_bord  = 0.00;
							strcpy (soln_rec.status, "D");
							strcpy (soln_rec.stat_flag, "D");
							cc = abc_update (soln, &soln_rec);
							if (cc)
								file_err (cc,soln,"DBUPDATE");
						}
						else
						{
							cc = ArchiveSoln (soln_rec.hhsl_hash);
							if (cc)
								file_err (cc, soln, "ARCHIVE");

							cc = abc_delete (soln);
							if (cc)
								file_err (cc,soln,"DBUPDATE");
						}
						ns_ok = FALSE;
					}
					else
					{
						strcpy (soln_rec.status,"B");
						strcpy (soln_rec.stat_flag,"B");
						soln_rec.qty_bord = ToStdUom (local_rec.qty_back);
						soln_rec.qty_order = 0.00;
						cc = abc_update (soln, &soln_rec);
						if (cc)
							file_err (cc,soln,"DBUPDATE");

						ns_ok = TRUE;
					}
				}
			}
		}
		else
		{
			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			cc = abc_add (coln, &coln_rec);
			if (cc)
				file_err (cc,coln,"DBADD");

			abc_unlock (coln);

			coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
			coln_rec.line_no	=	line_cnt;
			cc = find_rec (coln, &coln_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, coln, "DBFIND");

			putchar ('A');
			fflush (stdout);
		}
		/*----------------------------------------------
		| Check if header needs to be deleted/updated. |
		----------------------------------------------*/
		DeleteSalesOrders (soln_rec.hhso_hash);

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

	if (envVar.dbMcurr && cohr_rec.exch_rate != 0.00)
	{
		comr_rec.sur_cof *= cohr_rec.exch_rate;
		comr_rec.sur_amt *= cohr_rec.exch_rate;
	}
	if (totalQtyOrd > 0.00 && SUR_CHARGE && 
	    	s_total < comr_rec.sur_cof && s_total > 0.00)
	{
		cohr_rec.sos = comr_rec.sur_amt;
	}
	else
		cohr_rec.sos = 0.00;

	if (totalQtyOrd <= 0.00)
	{
		cohr_rec.freight	=	0.00;
		cohr_rec.sos		=	0.00;
		cohr_rec.insurance 	=	0.00;
		cohr_rec.ex_disc 	=	0.00;
		cohr_rec.other_cost_1	=	0.00;
		cohr_rec.other_cost_2	=	0.00;
		cohr_rec.other_cost_3	=	0.00;
	}
	if (noTaxCharged)
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
	wk_value = no_dec (wk_value);

	cohr_rec.gst += wk_value;
	cohr_rec.gst = no_dec (cohr_rec.gst);
	
	strcpy (cohr_rec.din_1,local_rec.spinst [0]);
	strcpy (cohr_rec.din_2,local_rec.spinst [1]);
	strcpy (cohr_rec.din_3,local_rec.spinst [2]);
	strcpy (cohr_rec.batch_no,pad_batch (local_rec.batch_no));
	cohr_rec.date_raised = local_rec.inv_date;
	cohr_rec.date_required = des_date;

	if (findStatusFlag [0] == 'I' && cohr_rec.type [0] == 'P')
		strcpy (cohr_rec.type,"I");

	if (findStatusFlag [0] == 'T' && cohr_rec.type [0] == 'P')
		strcpy (cohr_rec.type,"T");

	/*-----------------------------------------------------------------------
	| Packing slip from so_pscreat + so_ps_cons are now created with status |
	| stat flag of "P". Basically if status is not 'P' then something else  |
	| has updated is leave as is to prevent double posting.                 |
	-----------------------------------------------------------------------*/
	if (cohr_rec.stat_flag [0] == 'P')
	{
		strcpy (cohr_rec.stat_flag,	createStatusFlag);
		strcpy (cohr_rec.status,	createStatusFlag);
	}

	strcpy (cohr_rec.inv_print, (envVar.combineInvoicePack) ? "Y" : "N");

	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBUPDATE");

	/*-------------------------------------------
	| Create a log file record for sales Order. |
	-------------------------------------------*/
	LogCustService
	(
		cohr_rec.hhco_hash,
		cohr_rec.hhso_hash,
		cumr_rec.hhcu_hash,
		cohr_rec.cus_ord_ref,
		cohr_rec.cons_no,
		cohr_rec.carr_code,
		cohr_rec.del_zone,
		LOG_DISPATCH
	);

	if (!TRANSPORT)
	{
		/*-------------------------------------------
		| Create a log file record for sales Order. |
		-------------------------------------------*/
		LogCustService 
		(
			cohr_rec.hhco_hash,
			cohr_rec.hhso_hash,
			cumr_rec.hhcu_hash,
			cohr_rec.cus_ord_ref,
			cohr_rec.cons_no,
			cohr_rec.carr_code,
			cohr_rec.del_zone,
			LOG_DELIVERY
		);
	}

	UpdateSONS (TRUE, 0, cohr_rec.hhco_hash);

	if (envVar.automaticFreight)
		AddCarrierDetails ();

	if (envVar.dbNettUsed)
	{
		invoice_tot =   cohr_rec.gross 			+
						cohr_rec.item_levy		+ 
						cohr_rec.sos 			+ 
						cohr_rec.freight 		+
						cohr_rec.insurance 		+
						cohr_rec.other_cost_1 	+
						cohr_rec.other_cost_2 	+
						cohr_rec.other_cost_3 	+
						cohr_rec.tax 			+ 
						cohr_rec.erate_var 		+ 
						cohr_rec.gst 			- 
						cohr_rec.disc 			- 
						cohr_rec.deposit 		- 
						cohr_rec.ex_disc;
	}
	else
	{
		invoice_tot =   cohr_rec.gross 			+
						cohr_rec.item_levy		+ 
						cohr_rec.sos 			+ 
						cohr_rec.freight 		+
						cohr_rec.insurance 		+
						cohr_rec.other_cost_1 	+
						cohr_rec.other_cost_2 	+
						cohr_rec.other_cost_3 	+
						cohr_rec.tax			+	 
						cohr_rec.erate_var 		+ 
						cohr_rec.gst 			- 
						cohr_rec.deposit 		- 
						cohr_rec.ex_disc;
	}

	if (envVar.dbMcurr)
	{
		sprintf (tot_str, "%.2f %-3.3s)", DOLLARS (invoice_tot), cumr_rec.curr_code);
	}
	else
		sprintf (tot_str, "%.2f)", DOLLARS (invoice_tot));

	if (TRANSPORT)
		sprintf (err_str, ML (mlSoMess145), cohr_rec.inv_no, tot_str);
	else
		sprintf (err_str, ML (mlSoMess146), cohr_rec.inv_no, tot_str);

	PauseForKey (0,0,err_str, 0);

	add_hash (comm_rec.co_no, 
			  comm_rec.est_no, 
			  "RO", 
			  0,
			  cohr_rec.hhcu_hash, 
			  0L, 
			  0L, 
			(double) 0.00);

	if (AUTO_SK_UP)
	{
		add_hash (comm_rec.co_no, 
				  comm_rec.est_no, 
				  "SU", 
				  0,
				  cohr_rec.hhco_hash, 
				  0L, 
				  0L, 
				(double) 0.00);
	}
	recalc_sobg ();
}

/*===============================
| Delete or update sohr record. |
===============================*/
void
DeleteSalesOrders (
 long	HHSO_HASH)
{
	int		lines_found = 0;

	sohr_rec.hhso_hash	=	HHSO_HASH;
	if (find_rec (sohr, &sohr_rec, COMPARISON, "u"))
	{
		abc_unlock (sohr);
		return;
	}
	/*-----------------------------------
	| Check if sohr needs to be deleted	|
	| ie no soln records remaining for	|
	| sohr.					            |
	-----------------------------------*/
	if (!envVar.salesOrderRealTimeDelete)
	{
		lines_found = 0;

		soln2_rec.hhso_hash = HHSO_HASH;
		soln2_rec.line_no = 0;
		cc = find_rec (soln2, &soln2_rec, GTEQ, "r");
		while (!cc && soln2_rec.hhso_hash == HHSO_HASH)
		{
			if (soln2_rec.status [0] != 'D')
				lines_found++;

			cc = find_rec (soln2, &soln2_rec, NEXT, "r");
		}
		if (!lines_found)
		{
			strcpy (sohr_rec.stat_flag, "D");
			strcpy (sohr_rec.status, "D");
			strcpy (sohr_rec.sohr_new,"N");
			cc = abc_update (sohr, &sohr_rec);
			if (cc)
				file_err (cc, sohr, "DBUPDATE");

			abc_unlock (sohr);
			return;
		}
	}
	else
	{
		soln2_rec.hhso_hash = HHSO_HASH;
		soln2_rec.line_no = 0;
		cc = find_rec (soln2, &soln2_rec,GTEQ,"r");
		if (cc || soln2_rec.hhso_hash != HHSO_HASH)
		{
			cc = ArchiveSohr (sohr_rec.hhso_hash);
			if (cc)
				file_err (cc, sohr, "ARCHIVE");
			
			abc_delete (sohr);
			return;
		}
	}
	/*--------------------------------------------------------
	| Check if status and invoice number/packing slip number |
	| is the same as order may be on multiple packing slips. |
	--------------------------------------------------------*/
	if (!strcmp (sohr_rec.inv_no, cohr_rec.inv_no) && 
	      sohr_rec.status [0] == 'P')
	{
		strcpy (sohr_rec.inv_no,"        ");
		strcpy (sohr_rec.stat_flag,"B");
		strcpy (sohr_rec.status,"B");
	}
	strcpy (sohr_rec.sohr_new,"N");
	cc = abc_update (sohr, &sohr_rec);
	if (cc)
		file_err (cc, sohr, "DBUPDATE");
	
	return;
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
	if (SERIAL_ITEM)
	{
		cc = UpdateInsf (store [line_no].hhwhHash,
		 		store [line_no].hhbrHash,
				ser_no, "C", "F");

		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}
/*======================================================================
| Update serial number checking for multiple branch ability to commit. |
======================================================================*/
int
UpdateInsf (
 long	hhwh_hash,
 long	hhbr_hash,
 char	*serial_no,
 char	*old_status,
 char	*new_status)
{
	abc_selfield (insf,"insf_id_no");

	insf_rec.hhwh_hash = hhwh_hash;
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.status, "%-1.1s", old_status);
	sprintf (insf_rec.serial_no, "%-25.25s", serial_no);

	/*-----------------------------------------------------------
	| If not found then item is by hhbr_hash and not hhwh_hash. |
	-----------------------------------------------------------*/
	if (find_rec (insf, &insf_rec, COMPARISON, "u"))
	{
		abc_selfield (insf,"insf_hhbr_id");

		insf_rec.hhwh_hash = hhwh_hash;
		insf_rec.hhbr_hash = hhbr_hash;
		sprintf (insf_rec.status, "%-1.1s", old_status);
		sprintf (insf_rec.serial_no, "%-25.25s", serial_no);
		cc = find_rec (insf, &insf_rec, COMPARISON, "u");
		if (cc)
			return (cc);
	}
	strcpy (insf_rec.status,new_status);
	return (abc_update (insf, &insf_rec));
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
SrchInum (
 char *key_val)
{
    work_open ();
    save_rec ("#UOM ","#Description");

    strcpy (inum2_rec.uom_group, SR._uom_group);
    strcpy (inum2_rec.uom, key_val);
    cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
    while (!cc && !strcmp (inum2_rec.uom_group, SR._uom_group))
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

    strcpy (inum2_rec.uom_group, SR._uom_group);
    strcpy (inum2_rec.uom, 		key_val);
    cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
    if (cc)
		file_err (cc, inum2, "DBFIND");
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
	cc = find_rec (exsf, &exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf, &exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf, &exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

void
SrchPay (
 void)
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
SrchSellTerms (
 void)
{
	int		i = 0;
	work_open ();
	save_rec ("#Cde","#Selling Terms ");

	for (i = 0;strlen (s_terms [i]._scode);i++)
	{
		cc = save_rec (s_terms [i]._scode,s_terms [i]._sterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*==================
| Search for area. |
==================*/
void
SrchExaf (
 char	*key_val)
{
	work_open ();
	save_rec ("#Ar","#Area.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf, &exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf, &exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
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

	insf_rec.hhwh_hash = store [line_no].hhwhHash;
	strcpy (insf_rec.status,"F");
	sprintf (insf_rec.serial_no,"%-25.25s",key_val);
	cc = find_rec (insf, &insf_rec,GTEQ,"r");

	while (!cc && store [line_no].hhwhHash == insf_rec.hhwh_hash && 
			insf_rec.status [0] == 'F' && 
			!strncmp (insf_rec.serial_no,key_val,strlen (key_val)))
	{
		if (!CheckDuplicateInsf (insf_rec.serial_no,
				  store [line_no].hhbrHash,line_no))
		{
			cc = save_rec (insf_rec.serial_no,
					inmr_rec.item_no);
			if (cc)
				break;
		}
		cc = find_rec (insf, &insf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	insf_rec.hhwh_hash = store [line_no].hhwhHash;
	strcpy (insf_rec.status,"F");
	sprintf (insf_rec.serial_no,"%-25.25s",temp_str);
	cc = find_rec (insf, &insf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, insf, "DBFIND");
	
	strcpy (local_rec.serial_no,insf_rec.serial_no);
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
				sleep (SLEEP_TIME);
				return (EXIT_SUCCESS);
			}
			else
				return (EXIT_FAILURE);
		}
	}
	sprintf (item_no,"%-16.16s",hidden_item);
	return (EXIT_SUCCESS);
}
/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.				|
| Return 1 if duplicate					|
=======================================================*/
int
CheckDuplicateInsf (
 char	*serial_no,
 long	hhbr_hash,
 int	line_no)
{
	int		i;

	for (i = 0;i < lcount [2];i++)
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
		if (!strcmp (store [i]._serial,ser_space))
			continue;

		/*---------------------------------------
		| Only compare serial numbers for	|
		| the same item number			|
		---------------------------------------*/
		if (store [i].hhbrHash == hhbr_hash)
		{
			if (!strcmp (store [i]._serial,serial_no))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*==========================
| Search for p_slip number |
==========================*/
void
SrchCohr (
 char	*key_val)
{
	work_open ();
	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cohr_rec.inv_no,"%-8.8s",key_val);
	strcpy (cohr_rec.type,"P");

	save_rec ("#P/Slip","#Customer Order Number.");
	cc = find_rec (cohr, &cohr_rec,GTEQ,"r");
	while (!cc && !strncmp (cohr_rec.inv_no,key_val,strlen (key_val)) && !strcmp (cohr_rec.co_no,comm_rec.co_no) && !strcmp (cohr_rec.br_no,comm_rec.est_no) && cohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if (cohr_rec.type [0] == 'P')
		{
			cc = save_rec (cohr_rec.inv_no,cohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (cohr, &cohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cohr_rec.inv_no,"%-8.8s",temp_str);
	strcpy (cohr_rec.type,"P");
	cc = find_rec (cohr, &cohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "cohr", "DBFIND");
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

	cc = find_rec (exsi, &exsi_rec,GTEQ,"r");
	while (!cc && !strcmp (exsi_rec.co_no,comm_rec.co_no))
	{
		sprintf (wk_code, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wk_code, err_str);
		if (cc)
			break;

		cc = find_rec (exsi, &exsi_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi, &exsi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsi", "DBFIND");
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
	sprintf (incc_rec.sort,"%s%11.11s%-16.16s",
					inmr_rec.inmr_class,
					inmr_rec.category,
					inmr_rec.item_no);
				
	strcpy (incc_rec.stocking_unit, inmr_rec.sale_unit);
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.allow_repl, "E");
	strcpy (incc_rec.abc_code, inmr_rec.abc_code);
	strcpy (incc_rec.abc_update, inmr_rec.abc_update);
	strcpy (incc_rec.stat_flag,"0");
	incc_rec.first_stocked = TodaysDate ();
	strcpy (incc_rec.stat_flag,"0");
	
	cc = abc_add (incc, &incc_rec);
	if (cc) 
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc, &incc_rec,COMPARISON,"r");
	return (cc);
}
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

	BusyFunction (1);
	i = prmptmsg (ML (mlStdMess177) ,"YyNn",0,2);
	if (i == 'N' || i == 'n')
	{
		BusyFunction (0);
		return;
	}

	sprintf (shhbr_hash,"%09ld",SR.hhbrHash);
	sprintf (shhcc_hash,"%09ld",SR.hhccHash);
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
	BusyFunction (0);
	return;
}

/*========================================
| Clear popup window ready for _new item. |
========================================*/
void
ClearWindow (
 void)
{
	int		i;
	for (i = 18; i < 24 ; i++)
	{
		move (0,i); 
		fflush (stdout);
		cl_line ();
	}
	PrintCompanyDetails ();
}
void
BusyFunction (
 int	flip)
{
	print_at (2,1,"%-90.90s"," ");
	if (flip)
	{
		print_at (2,1, ML (mlStdMess035));
	}
	fflush (stdout);
}

int
DeleteLine (
 void)
{
	int		i;
	int		this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	FreeInsf (line_cnt,local_rec.serial_no);

	lcount [ITEM_SCN]--;

	this_page = line_cnt / TABLINES;

	add_hash (comm_rec.co_no, 
			  comm_rec.est_no, 
			  "RC", 
			  0,
			  SR.hhbrHash, 
			  SR.hhccHash, 
			  0L, 
			(double) 0.00);

	for (i = line_cnt;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		memset ((char *) &SR, '\0', sizeof (struct storeRec));

		/*----------------------------
		| Clear out lot information. |
		----------------------------*/
		LotClear (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	sprintf (local_rec.item_no,"%-16.16s"," ");
	sprintf (coln_rec.item_desc,"%-40.40s"," ");
	strcpy (coln_rec.sman_code,"  ");
	strcpy (coln_rec.cus_ord_ref,"                    ");
	strcpy (coln_rec.pack_size,"     ");
	sprintf (local_rec.item_desc,"%-40.40s"," ");
	sprintf (local_rec.UOM,"%-4.4s"," ");
	local_rec.qty_ord 		= 0.00;
	local_rec.qty_des 		= 0.00;
	local_rec.qty_back 		= 0.00;
	coln_rec.cost_price 	= 0.00;
	coln_rec.sale_price 	= 0.00;
	coln_rec.disc_pc 		= 0.00;
	strcpy (coln_rec.hide_flag, "N");
	sprintf (local_rec.serial_no,"%-25.25s"," ");
	putval (line_cnt);

	memset ((char *) &SR, '\0', sizeof (struct storeRec));

	/*----------------------------
	| Clear out lot information. |
	----------------------------*/
	LotClear (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	invoiceChanged = TRUE;
	line_cnt = i;
	getval (line_cnt);
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

int
CheckOk (
 void)
{
	int		i;
	
	for (i = 0; i < maxLinesUsed; i++)
		if (store [i].hhwhHash > 0L && 
		     store [i].err_fnd [0] == 'Y' &&
		     store [i]._qty_des > 0.00)
			return (i + 1);

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
	{
		move (21,3); line (90);
	}
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
 int _key, 
 long last_hhcu)
{
	if (_key == 0)
	{
		cc = find_rec (cucc, &cucc_rec,COMPARISON,"r");
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);

		return (EXIT_FAILURE);
	}

	if (last_hhcu != 0L)
	{
		/*-------------------------------------------------------
		| Find the NEXT / PREVIOUS record to the current one	|
		-------------------------------------------------------*/
		cc = find_rec (cucc, &cucc_rec, (_key == FN14) ? GREATER : LT,"r");

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
		cc = find_rec (cucc, &cucc_rec,GTEQ,"r");
	}
	else
	{
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash + 1L;
		cucc_rec.record_no = 0L;

		cc = find_rec (cucc, &cucc_rec,GTEQ,"r");

		/*-------------------------------------------
		| Probably the last hhcu group in the cucc	|
		| so find the last record in the file.		|
		-------------------------------------------*/
		if (cc)
			cc = find_rec (cucc, &cucc_rec,LAST,"r");
		else
			cc = find_rec (cucc, &cucc_rec,LT,"r");
	}

	if (cc || cucc_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn != cur_screen || cur_screen > 2)
	{
		putchar (BELL);
		return (FALSE);
	}
	if (cur_screen == 1)
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

	/*--------------------
	| Check for contract |
	--------------------*/
	if (store [lin]._cont_status)
	{
		/*print_mess ("\007 Item has a contract price, negotiation window not available. ");*/
		print_mess (ML (mlSoMess239));
		sleep (SLEEP_TIME);
		clear_mess ();
		return (FALSE);
	}

	/*-----------------------
	| Disable qty BO field. |
	-----------------------*/
	negoScn [1].fldEdit = 0;

	/*---------------------------------------
	| Initialise negotiation window values. |
	---------------------------------------*/
	negoRec.qOrd			=	store [lin]._qty_des;
	negoRec.qBord			=	store [lin]._qty_back;

	negoRec.regPc			=  	store [lin]._reg_pc;
	negoRec.discArray [0]	=	store [lin]._disc_a;
	negoRec.discArray [1]	=	store [lin]._disc_b;
	negoRec.discArray [2]	=	store [lin]._disc_c;
	negoRec.grossPrice		=	store [lin]._gsale_price;
	negoRec.salePrice		=	store [lin]._sale_price;
	negoRec.margCost		=	store [lin]._marg_cost;
	negoRec.outer_size 		=	store [lin]._outer;

	NegPrice (2, 7, local_rec.item_no, coln_rec.item_desc, 
				   store [lin]._cumulative, scn);

	if (!restart)
	{
		local_rec.qty_des 		=   negoRec.qOrd;
		local_rec.qty_back 		=   negoRec.qBord;

		store [lin]._qty_des		=	negoRec.qOrd;
		store [lin]._qty_back		= 	negoRec.qBord;
		store [lin]._reg_pc		= 	negoRec.regPc;
		store [lin]._disc_a		= 	negoRec.discArray [0];
		store [lin]._disc_b		= 	negoRec.discArray [1];
		store [lin]._disc_c		= 	negoRec.discArray [2];
		store [lin]._dis_pc		=	CalcOneDisc (store [lin]._cumulative,
													 negoRec.discArray [0],
													 negoRec.discArray [1],
													 negoRec.discArray [2]);
		store [lin]._gsale_price 	= 	negoRec.grossPrice;
		store [lin]._sale_price		=	negoRec.salePrice;
		store [lin]._act_sale		=	negoRec.salePrice;

		coln_rec.disc_pc  			= 	ScreenDisc (store [lin]._dis_pc);
		coln_rec.sale_price 		= 	store [lin]._sale_price;

		putval (lin);
	}
	CalculateTotalBox (FALSE, FALSE);
	
	restart = FALSE;
    return (TRUE); /* is this correct? -- vij */
}
/*==================================
| Add freight history information. |
==================================*/
void
AddCarrierDetails (
 void)
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
	trch_rec.est_frt_cst 	= 	DOLLARS (est_freight);
	trch_rec.act_frt_cst 	= 	DOLLARS (cohr_rec.freight);
	strcpy (trch_rec.cumr_chg,	(cohr_rec.freight > 0.00) ?  "Y" : "N");
	strcpy (trch_rec.stat_flag, "0");

	cc = abc_add (trch, &trch_rec);
	if (cc)
		file_err (cc, trch, "DBADD");
}

void
PrintCompanyDetails (
 void)
{
	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_short);
	print_at (22,0, err_str);
	sprintf (err_str, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22,30, err_str);
	sprintf (err_str, ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);
	print_at (22,60, err_str);
	sprintf (err_str, ML (mlStdMess085), cudp_rec.dp_no, cudp_rec.dp_short);
	print_at (22,100, err_str);
}

void
tab_other (
 int line_no)
{
	static	int		old_back;
	static	int		old_sale;
	static 	int		old_disc;
	static	int		first_time = TRUE;

	if (first_time)
	{
		old_sale = FLD ("sale_price");
		old_disc = FLD ("disc");
		old_back = FLD ("qty_back");
		first_time = FALSE;
	}

	if (line_no < numberOldLines)
	{
		if (!F_HIDE (label ("UOM")))
			FLD ("UOM") 	 = NA;
		FLD ("qty_back") = old_back;
	}
	else
	{
		if (!F_HIDE (label ("UOM")))
			FLD ("UOM") 	 = YES;
		if (FLD ("qty_back") != ND)
			FLD ("qty_back") = NA;
	}

	/*-------------------------------------
	| Turn fields on and off depending on |
	| contract / promotional price.       |
	-------------------------------------*/
	if (store [line_no]._cont_status)
	{
		FLD ("sale_price") = NA;
		FLD ("disc") = NA;
	}
	else
	{
		FLD ("sale_price") = old_sale;
		FLD ("disc") = old_disc;
	}
	if (cur_screen == 2)
	{
		TidySonsScreen ();

		DispSONS (line_no);

		/*------------------------------------
		| Display of Second Item Description |
		------------------------------------*/
		if (F_HIDE (label ("descr")))
			print_at (5,0, ML (mlStdMess247), store [line_no]._item_desc);
		else
			print_at (5,0, ML (mlStdMess250), store [line_no]._desc2);
	
		if (store [line_no]._ser_item [0] == 'Y')
		{
			if (FLD ("ser_no") == NA)
				FLD ("ser_no") = YES;
		}
		else
		{
			if (FLD ("ser_no") == NA)
				FLD ("ser_no") = NA;
		}
	}
}

int
CheckForNilBalance (
 void)
{
	double 	lcl_tot;

	if (envVar.dbNettUsed)
		lcl_tot = no_dec (inv_tot - dis_tot);
	else
		lcl_tot = no_dec (inv_tot);

	if (lcl_tot == 0.00 && f_other != 0.00)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
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
		strcpy (err_str,"Additional Description for Delivery Order");
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

	if (!strcmp (SR.nsDesc [0], ns_space))
		return;
	
	for (i = 0; i < 4 ; i++)
	{
		if (i)
			sprintf (disp_str, "%38.38s: %-40.40s", ns_space, SR.nsDesc [i]);
		else
			sprintf (disp_str, ML (mlSoMess119) , local_rec.item_no, SR.nsDesc [i]);

		print_at (i+2,0, "%R %-82.82s", disp_str);
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

/*---------------------------------
| Tidy up alternate supply screen |
---------------------------------*/
void
TidySonsScreen (
 void)
{
	int		i;

	for (i = 0; i < 4 ; i++)
		print_at (i+2,0, "%90.90s", ns_space);
	
	print_at (3,1, ML (mlSoMess340));

	print_at (4,0, ML (mlStdMess012) ,cumr_rec.dbt_no,
					       			clip (cumr_rec.dbt_name));
	if (envVar.dbMcurr)
		print_at (4,60,"(%-3.3s)", cumr_rec.curr_code);
	
}
		
void
SerialItemMessage (
 int	Line,
 int	Free)
{
	if (Free)
	{
		sprintf (err_str, ML (mlSoMess338) , 
					 clip (store [Line]._serial),  
					 Line + 1);
	}
	else
	{
		sprintf (err_str, ML (mlSoMess339) , 
					 clip (store [Line]._org_ser),
					 Line + 1);
	}
	print_mess (err_str);
	sleep (sleepTime);
	clear_mess ();
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

float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR._cnv_fct;

	return (cnvQty);
}

float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR._cnv_fct;

	return (cnvQty);
}

void
OpenTransportFiles (
 char	*ZoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, ZoneIndex);
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
}
void
CloseTransportFiles (
 void)
{
	abc_fclose (trzm);
	abc_fclose (trcm);
	abc_fclose (trcl);
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

	/*-----------------------------------
	| Validate is serial items allowed. |
	-----------------------------------*/
	sptr = chk_env ("SK_SERIAL_OK");
	envVar.serialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*---------------------------
	| Check for Currency Code.  |
	---------------------------*/
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	/*------------------------------
	| Get value of override option |
	------------------------------*/
	sptr	=	chk_env ("SO_OVERRIDE_QTY");
	if (sptr == (char *)0)
		strcpy (envVar.overrideQuantity, "Y");
	else
		sprintf (envVar.overrideQuantity, "%-1.1s", sptr);

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
	| Customer multi-currency flag.  |
	--------------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	envVar.soDoi = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	/*------------------------------------
	| Check if advertising Levy applies. |
	------------------------------------*/
	sptr = chk_env ("SO_FREIGHT_BORD");
	envVar.soFreightBord = (sptr == (char *)0) ? 1 : atoi (sptr);

	/*------------------------------------
	| Check if advertising Levy applies. |
	------------------------------------*/
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

	/*---------------------------------------
	| Check if available stock is included. |
	---------------------------------------*/
	sptr = chk_env ("SO_FWD_AVL");
	envVar.includeForwardStock = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*----------------------------------------------
	| Check if sales orders are deleted real time. |
	----------------------------------------------*/
	sptr = chk_env ("SO_RT_DELETE");
	envVar.salesOrderRealTimeDelete = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
	| Check if nett pricing is used. |
	--------------------------------*/
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-------------------------------------
	| Check for Automatic freight charge. |
	-------------------------------------*/
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-------------------------------------------------------------
	| Check if special codes for bonus and hidden lines are used. |
	-------------------------------------------------------------*/
	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVar.soSpecial,"/B/H");
	else
		sprintf (envVar.soSpecial,"%-4.4s", sptr);

	/*-----------------------------------------------------------
	| Check status passed equals automatic stock update status. |
	-----------------------------------------------------------*/
	sprintf (envVar.automaticStockUpdate, "%-1.1s",get_env ("AUTO_SK_UP"));

	/*-----------------------------------------
	| Check if print confirmation is printed. |
	-----------------------------------------*/
	sptr = chk_env ("PRT_CONF");
	if (sptr != (char *)0 && (*sptr == 'Y' || *sptr == 'y'))
		envVar.printConfirmation = TRUE;

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

	/*--------------------------------------------------------
	| Check if system has combined packing slip and invoice. |
	--------------------------------------------------------*/
	strcpy (err_str,CheckVariable ("COMB_INV_PAC","N"));
	if (err_str [0] == 'Y' || err_str [0] == 'y')
		envVar.combineInvoicePack = TRUE;
	else
		envVar.printConfirmation = FALSE;

	/*--------------------------
	| How if Freight Charged.  |
	--------------------------*/
	sptr = chk_env ("SO_FREIGHT_CHG");
	envVar.soFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		pr_box_lines (scn);

		rv_pr (ML (mlSoMess129), 45,0,1);

		switch (scn)
		{
		case	MAIN_SCN:
			print_at (0,100, ML (mlSoMess147), local_rec.prev_dbt_no,
					     	      local_rec.prev_order);
			inScreenFlag = FALSE;
			use_window (0);
			break;

		case	ITEM_SCN:
			inScreenFlag = TRUE;
			print_at (4,0, ML (mlStdMess012),cumr_rec.dbt_no,
						       			   clip (cumr_rec.dbt_name));
			if (envVar.dbMcurr)
				print_at (4,60,"(%-3.3s)", cumr_rec.curr_code);

			DrawTotals ();
			if (local_rec.inp_total != 0.00 && 
			     prog_status != ENTRY)
			{
				CalcInputTotal ();
				PrintTotalBoxValues ();
			}
			else
			{
				if (prog_status != ENTRY)
					CalculateTotalBox (FALSE, FALSE);
			}
			fflush (stdout);
			break;

		case FRI_SCN:
		case COST_SCN:
			inScreenFlag = FALSE;
			print_at (2,1, ML (mlStdMess012) ,cumr_rec.dbt_no,
						       				clip (cumr_rec.dbt_name));
			break;
		}

		PrintCompanyDetails ();
		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
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

	SetSTerm (0, "   ","Local                 ");
	SetSTerm (1, "CIF","Cost Insurance Freight");
	SetSTerm (2, "C&F","Cost & Freight");
	SetSTerm (3, "FIS","Free Into Store");
	SetSTerm (4, "FOB","Free On Board");
	SetSTerm (5, "","");
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


/*=======================================================
| check the existence of the environment variable	    |
| first with company & branch, then company, and	    |
| then by itself.					                    |
| if no vble found then return 'prg'			        |
=======================================================*/
char *
CheckVariable (
	char	*_env,
	char	*_prg)
{
	char	*sptr;
	char	runPrint [41];

	/*-------------------------------
	| Check Company & Branch	|
	-------------------------------*/
	sprintf (runPrint,"%s%s%s",_env,comm_rec.co_no, comm_rec.est_no);
	sptr = chk_env (runPrint);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf (runPrint,"%s%s",_env,comm_rec.co_no);
		sptr = chk_env (runPrint);
		if (sptr == (char *)0)
		{
			sprintf (runPrint,"%s",_env);
			sptr = chk_env (runPrint);
			return ((sptr == (char *)0) ? _prg : sptr);
		}
		else
			return (sptr);
	}
	else
    {
		return (sptr);
    }
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

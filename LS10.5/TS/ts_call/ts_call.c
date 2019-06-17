/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ts_call.c,v 5.6 2002/11/28 04:09:51 scott Exp $
|  Program Name  : (ts_call.c)
|  Program Desc  : (Receive/Make calls)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 11/12/91         |
|---------------------------------------------------------------------|
| $Log: ts_call.c,v $
| Revision 5.6  2002/11/28 04:09:51  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.5  2002/07/24 08:39:33  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/17 09:58:14  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/06/26 06:32:32  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:23:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:55:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.6  2001/05/25 07:15:13  scott
| Updated for invalid label
|
| Revision 4.5  2001/05/08 07:14:47  robert
| Updated for the modified LS10-GUI RingMenu
|
| Revision 4.4  2001/04/18 06:01:06  cha
| fixed code for SQL.
|
| Revision 4.3  2001/04/03 10:23:50  scott
| Basically a TOTAL REWRITE to work with LS10-GUI
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_call.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_call/ts_call.c,v 5.6 2002/11/28 04:09:51 scott Exp $";

extern		int	X_EALL;
extern		int	Y_EALL;
extern		int	SR_X_POS;
extern		int	SR_Y_POS;
extern		int	_mail_ok;
extern		int	EnvScreenOK;

#define		TXT_REQD
#define		MAXSCNS		14
#define		MAXLINES	500
#define		TABLINES 	no_tablines

#define		X_OFF	1
#define		Y_OFF	1

#define		NO_MAILER	 (tspm_rec.mail_flag [0] == 'N')

#include 	<ml_std_mess.h>
#include 	<ml_ts_mess.h>
#include 	<pslscr.h>
#include 	<hot_keys.h>
#include 	<minimenu.h>
#include 	<ring_menu.h>
#include 	<getnum.h>
#include 	<twodec.h>
#include 	<fcntl.h>
#include 	<get_lpno.h>
#include    <tabdisp.h>

#ifdef GVISION
#include <StockWindow.h>
#endif

FILE	*pout;
int	wpipe_open = FALSE;
extern	int	tab_max_page;
extern	int	_win_func;

#define	RESTRT		-1
#define	PG_EXIT		99
#define	PLANNED		0
#define	MAILER		1
#define	RANDOM		2
#define	RECEIVE		3
#define	LEAVEOK		4

#define	S_HIST		2	/*----------------------------------------*/
#define	PROMO		3	/* Please Note that these #defines relate */
#define	BRAND		4	/* directly to the entry number of each   */
#define	PARCEL		6	/* option within the opt_menu structure   */
#define	NEGOT		7	/*                                        */
#define	I_HIST		8	/*                                        */
#define	COMP		9	/*----------------------------------------*/

#define	COMPLAINT	'C'
#define	NOTES		'N'
#define	LST_CALL	'L'
#define	NEXT_VISIT	'V'

#define	MTH_SALES	 (tspm_rec.sales_per [0] == 'M')
#define	FOLLOW_UP	 (tslh_rec.lett_type [0] == 'F')
#define	LABEL_DEF	 (tslh_rec.lett_type [0] == 'L')

#define SLEEP_TIME	2

#define	SR			store [line_cnt]		/* Normal Store */
#define	NG 			store [store_line]	/* Store from negotiation */
#define	BONUS		 (SR.bonusFlag [0] == 'Y')
#define	NO_COST		 (SR.itemClass [0] == 'N')
#define	NON_STOCK	 (SR.itemClass [0] == 'Z')
#define	KIT_ITEM	 (SR.itemClass [0] == 'K' && prog_status == ENTRY)
#define	PHANTOM		 (SR.itemClass [0] == 'P' && prog_status == ENTRY)
#define	BO_OK		 ((SR.backOrder [0] == 'Y' || SR.backOrder [0] == 'F') && \
			    	  cumr_rec.bo_flag [0] == 'Y')

#define	FULL_BO		 (SR.backOrder [0] == 'F' && cumr_rec.bo_flag [0] == 'Y')
#define	MULT_QTY	 (SR.costingFlag [0] != 'S')

#define	MARG_HOLD	 (envVarSoMargin [1] == '1')
#define	MARG_MESS1	 (envVarSoMargin [0] == '0')
#define	MARG_MESS2	 (envVarSoMargin [0] == '1')

#define	TWO_STEP	 (createFlag [0] == 'M')
#define	ONE_STEP	 (createFlag [0] == 'R')
#define	STANDARD	 (createFlag [0] == 'S')
#define	MARGIN_OK	 (strcmp (envVarSoMargin, "00"))

#define	EXT_COST	 (CENTS ((double)NG.qtyOrder * NG.margCost))

#define	FGN_CURR	 (envVarDbMcurr && strcmp (cumr_rec.curr_code, envVarCurrCode))

char	*blnk_line = "                                                                                                                                  ";
static char *mnth_name [] = {
		"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
		"JUL", "AUG", "SEP", "OCT", "NOV", "DEC", ""};
static char *day_name [] = {
		"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT", ""};

char	per_hd [11] [4];

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
	int		clear_ok			= 0,
			envVarDBCo 			= 0,
			envVarDbFind 		= 0,
			envVarDbMcurr		= 0,
			envVarGst 			= 1,
			envVarInpDisc 		= 0,
			envVarSoDisIndent 	= TRUE,
			envVarSoDiscRev 	= FALSE,
			envVarSoFullSupply 	= 0,
			envVarSoFwdAvl 		= 0,
			envVarSoLostSales 	= TRUE,
			envVarSoPermWin 	= FALSE,
			envVarSoSales 		= FALSE,
			envVarTsPrclexp 	= FALSE,
			envVarWinOk			= 1,
			firstTime 			= 1,
			held_order 			= 0,
			iscn_flag 			= FALSE,
			notax 				= 0,
			np_fn				= 0,
			over_margin 		= 0,
			printerNumber 		= 0;

	char	mlTsCall  [10] [101];

extern	int	TruePosition;
extern	int	EnvScreenOK;

	double	inv_tot = 0.00, 
			dis_tot = 0.00,
			tax_tot = 0.00, 
			gst_tot = 0.00,
			tot_tot = 0.00, 
			t_total = 0.00,
			l_total = 0.00, 
			l_disc  = 0.00,
			l_gst   = 0.00, 
			l_tax   = 0.00;

	char	createFlag [2];
	char	envVarSoSpecial [5];
	char	envVarCurrCode [4];
	char	envVarGstTaxName [4];
	char	envVarSoMargin [3];
	char	soBonus [3];
	char	*currentUser;
	char	lastItem [17];
	char	taxPrompt [31];
	char	taxNumber [31];

	struct	storeRec {
		long 	hhbrHash;		/* inmr_hhbr_hash				*/
		long	hhumHash;		/* inum_hhum_hash				*/
		long 	hhsiHash;		/* inmr_hhsi_hash				*/
		long 	hhwhHash;		/* > 0L if serial item			*/
		float	cnvFct;			/* inum_cnv_fct					*/
		float 	qtyAvailable;	/* incc_closing_stock			*/
		float 	qtyTotal;		/* total on order for line		*/
		float 	qtyOrder;		/* qty suppliable for line		*/
		float	defaultDisc;	/* inmr_disc_pc					*/
		float	discPc;			/* inmr_disc_pc 				*/
		float	calcDisc;		/* calculated discount PC 		*/
		float	taxPc;			/* inmr_tax_pc  				*/
		float	gstPc;			/* inmr_gst_pc  				*/
		float	weight;			/* inmr_weight					*/
		float	outerSize;		/* inmr_outer_size				*/
		float	minMarg;		/* Min margin for category.     */
		float	regPc;			/* Regulatory percent 			*/
		float	discA;			/* Discount A         			*/
		float	discB;			/* Discount B         			*/
		float	discC;			/* Discount C         			*/
		int		cumulative;		/* Cumulative 1 or 0  			*/
		double	margCost;		/* Cost price for Margins.		*/
		float	exchRate;		/* Exchange rate for Category.  */
		double	taxAmt;			/* inmr_tax_amt 				*/
		double	costPrice;		/* cost price					*/
		double	grossSalePrice;	/* gross sale price				*/
		double	salePrice;		/* default sale price			*/
		double	calcSalePrice;	/* calculated sale price		*/
		int		pricingCheck;	/* Set if pricing has been called */
		                      	/* for line.                    */
		double	actualSale;		/* actual sale price			*/
		char	category [12];	/* inmr_category				*/
		char	sellGroup [7];	/* Selling group				*/
		char	bonusFlag [2];	/* Y iff bonus item				*/
		char	itemClass [2];	/* inmr_class					*/
		char	backOrder [2];	/* inmr_bo_flag					*/
		char	boRelease [2];	/* inmr_bo_release				*/
		char	packSize [6];	/* inmr_pack_size				*/
		char	costingFlag [2];/* inmr_costing_flag			*/
		char	priceOveride [2];/* soln_pri_or					*/
		char	discOveride [2];/* soln_dis_or					*/
		char	marginFailed [2];/* Margin failed. 				*/
		int		contractPrice;	/* Contract Price				*/
		int		indentItem;		/* Indent        				*/
		int		contractStatus;	/* 0 = not contract line
								   1 = contract no disc
								   2 = contract disc ok    		*/
		int		commitRef;		/* soic Committed reference	*/
		int		deCommitRef;	/* soic DeCommitted reference	*/
		long	origHhbrHash;	/* Original hhbr hash 		*/
		long	origOrdQty;		/* Original order Qty 		*/
	} store [MAXLINES];

	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cudpRecord	cudp_rec;
struct tspmRecord	tspm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct ineiRecord	inei_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct tsciRecord	tsci_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	sohr2_rec;
struct solnRecord	soln_rec;
struct soicRecord	soic_rec;
struct soicRecord	soic2_rec;
struct polnRecord	poln_rec;
struct soktRecord	sokt_rec;
struct cusaRecord	cusa_rec;
struct cushRecord	cush_rec;
struct sadfRecord	sadf_rec;
struct tshsRecord	tshs_rec;
struct cuccRecord	cucc_rec;
struct tmopRecord	tmop_rec;
struct tsalRecord	tsal_rec;
struct tsalRecord	tsal2_rec;
struct tslhRecord	tslh_rec;
struct tslsRecord	tsls_rec;
struct tsxdRecord	tsxd_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exsiRecord	exsi_rec;
struct tsbcRecord	tsbc_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;


	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;
	Money	*cusa_balance	=	&cusa_rec.val1;
	float	*sadf_qty		=	&sadf_rec.qty_per1;
	int		*cumr_inst		=	&cumr_rec.inst_fg1;

	char	*data  	= "data",
	    	*cumr2 	= "cumr2",
	    	*inmr2 	= "inmr2",
			*inum2	= "inum2",
	    	*sohr2 	= "sohr2",
	    	*soic2 	= "soic2",
	    	*tsal2 	= "tsal2";
	char	*customerList 	= "customerList",
			*sixteen_space 	= "                ";

	int	upd_type;

	char	hd_scn [3] [30];

	float	sales_period [11];

struct SEL_LIST {
	char	item_no [17];
	long	hhbrHash;
	float	sel_ord;
	char	comment [41];
	struct SEL_LIST *next;
};
#define	SEL_NULL ((struct SEL_LIST *) NULL)
struct SEL_LIST *selhd_ptr = SEL_NULL;
struct SEL_LIST *selcur_ptr = SEL_NULL;

#include <cus_price.h>
#include <cus_disc.h>
#include <neg_win.h>

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	customerNo [7];
	char	customerName [41];
	char	letterCode [11];
	char	letterDesc [41];
	char	text [61];
	char	customerDate [11];
	long	prevPackSlip;
	char	prevCustomerNo [7];
	char	priceDesc [16];
	char	frei_req [5];
	char	spinst [3] [61];
	char	item_no [17];
	char	bonus [2];
	char	UOM [5];
	char	other [3] [31];
	char	sell_desc [31];
	float	qty_ord;
	float	qty_back;
	double	dflt_freight;
	double	disc_over;
	char	area_desc [41];
	char	qty_chk [2];
	char	soh_detail [27];
	char	prcl_item [26];
	float	sel_ord;
	char	brand [17];
	long	n_visit_date;
	char	n_visit_time [6];
	long	n_phone_date;
	char	n_phone_time [6];
	long	last_date;
	int		mailer_lpno;
	char	cont_desc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Customer No    ", "Enter Customer Number To Call.",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{1, LIN, "customerName",	 3, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerName},
	{1, LIN, "cont_no1",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract       ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cont_no},
	{1, LIN, "cont_desc1",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cont_desc},

	{2, LIN, "letterCode",	 3, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Mailer Code    ", "Enter Mailer Code To Follow Up.",
		YES, NO,  JUSTLEFT, "", "", local_rec.letterCode},
	{2, LIN, "letterDesc",	 3, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.letterDesc},

	{3, TXT, "last_call",	 7, 2, CHARTYPE,
		"", "          ",
		" ", "", "LAST CALL NOTES", "",
		4, 60, 16, "", "", local_rec.text},

	{4, TXT, "notes",	 7, 68, CHARTYPE,
		"", "          ",
		" ", "", "NOTES", "",
		4, 60, 16, "", "", local_rec.text},

	{5, TXT, "complaints",	13, 2, CHARTYPE,
		"", "          ",
		" ", "", "COMPLAINTS", "",
		4, 60, 16, "", "", local_rec.text},

	{6, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " Default : Deletes Line ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{6, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "   I t e m   D e s c r i p t i o n .   "," ",
		NI, NO,  JUSTLEFT, "", "", soln_rec.item_desc},
	{6, TAB, "bonus",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "N", "B", " Bonus Item (Y/N) ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.bonus},
	{6, TAB, "UOM",       0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM ", " ",
		ND, NO, JUSTLEFT, "", "", local_rec.UOM},
	{6, TAB, "sale_code",	 0, 1, CHARTYPE,
		"UU", "          ",
		" ", sohr_rec.sman_code, "Sale", " Salesman ",
		 ND, NO, JUSTRIGHT, "", "", soln_rec.sman_code},
	{6, TAB, "ord_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", sohr_rec.cus_ord_ref, " Cust Order Ref. ", "Customer Order Ref.",
		 ND, NO,  JUSTLEFT, "", "", soln_rec.cus_ord_ref},
	{6, TAB, "pack_size",	 0, 0, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Pack ", " Pack Size ",
		 ND, NO,  JUSTLEFT, "", "", soln_rec.pack_size},
	{6, TAB, "qty_ord",	 0, 0, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "1.00", "Qty Ord", " ",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *)&local_rec.qty_ord},
	{6, TAB, "qty_back",	 0, 1, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "0.00", "Qty Back", " ",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *)&local_rec.qty_back},
	{6, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Cst Price", " Cost Price ",
		 ND, NO, JUSTRIGHT, "-999999.99", "999999.99", (char *)&soln_rec.cost_price},
	{6, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Sell Price", " Sell Price ",
		YES, NO, JUSTRIGHT, "-999999.99", "999999.99", (char *)&soln_rec.sale_price},
	{6, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Disc ", " ",
		 NO, NO, JUSTRIGHT, "0.0", "100.0", (char *)&soln_rec.dis_pc},
	{6, TAB, "due_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", "00/00/00", "Due Date", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&soln_rec.due_date},
	{6, TAB, "qty_chk",	 0, 0, CHARTYPE,
		"U", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.qty_chk},
	{6, TAB, "soh_detail",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", " SOH   | DUE DATE | PO QTY", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.soh_detail},
	{7, LIN, "cus_ord_ref",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Customer Order Ref    ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cus_ord_ref},
	{7, LIN, "dt_raised",	 7, 60, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.customerDate, "Order Date            ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&sohr_rec.dt_raised},
	{7, LIN, "dt_required",	 7, 100, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Required Date         ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&sohr_rec.dt_required},
	{7, LIN, "tax_code",	 8, 2, CHARTYPE,
		"U", "        ",
		" ", cumr_rec.tax_code, taxPrompt, " ",
		 NO, NO,  JUSTLEFT, "ABCD", "", sohr_rec.tax_code},
	{7, LIN, "tax_no",	 8, 60, CHARTYPE,
		"AAAAAAAAAAAAAAA", "        ",
		" ", cumr_rec.tax_no, taxNumber, " ",
		 NO, NO,  JUSTLEFT, "", "", sohr_rec.tax_no},
	{7, LIN, "sman_code",	 9, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.sman_code, "Sales Person          ", " ",
		YES, NO, JUSTRIGHT, "", "", sohr_rec.sman_code},
	{7, LIN, "sman_desc",	 9, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{7, LIN, "area_code",	 10, 2, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.area_code, "Area Code             ", " ",
		YES, NO, JUSTRIGHT, "", "", sohr_rec.area_code},
	{7, LIN, "area_desc",	 10, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_desc},
	{7, LIN, "shipname",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship Name             ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_name},
	{7, LIN, "ship_method",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Ship Method           ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{7, LIN, "shipaddr1",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship Address One      ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add1},
	{7, LIN, "shipaddr2",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship Address Two      ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add2},
	{7, LIN, "shipaddr3",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship Address Three    ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.del_add3},
	{7, LIN, "spcode1",	16, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction One       ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{7, LIN, "spcode2",	17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction Two       ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{7, LIN, "sos_ok",	18, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "S.O Surcharge. (Y/N)  ", "Enter N (o) to Overide Small Order Surcharge.",
		YES, NO, JUSTRIGHT, "YN", "", sohr_rec.sohr_new},
	{7, LIN, "pay_term",	18, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms         ", " ",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.pay_term},

	{8, LIN, "brand",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Brand Code    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.brand},

	{9, LIN, "n_vst_dt",	 3, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Next Visit Date    ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.n_visit_date},
	{9, LIN, "n_vst_tm",	 4, 2, CHARTYPE,
		"AA:AA", "          ",
		" ", " ", "Next Visit Time    ", " ",
		 NO, NO,  JUSTLEFT, "02", "", local_rec.n_visit_time},
	{9, LIN, "n_phn_dt",	5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Next Phone Date    ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.n_phone_date},
	{9, LIN, "n_phn_tm",	6, 2, CHARTYPE,
		"AA:AA", "          ",
		" ", " ", "Next Phone Time    ", " ",
		 NO, NO,  JUSTLEFT, "0123456789", "", local_rec.n_phone_time},

	{10, TXT, "nxt_vst_notes",	 7, 41, CHARTYPE,
		"", "          ",
		" ", "", " NEXT VISIT NOTES ", "",
		5, 60, 16, "", "", local_rec.text},

	{11, LIN, "lst_date",	 3, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Include Calls Up To A Next Phone Date Of  ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.last_date},

	{12, LIN, "cont_no2",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract      ", " Enter Contract If Contract Prices Available - Search Available",
		YES, NO,  JUSTLEFT, "", "", sohr_rec.cont_no},
	{12, LIN, "cont_desc2",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cont_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <CheckIndent.h>
#include <proc_sobg.h>
static	KEY_TAB null_keys [] = { END_KEYS };

static	int	select_func (int c, KEY_TAB *psUnused);
static	int	exit_func (int c, KEY_TAB *psUnused);
static	int	dummy_func (int iUnused, KEY_TAB *psUnused);
static	KEY_TAB cust_keys [] =
{
    { NULL,                 '\r', select_func,
	" Select Customer For Call ",					"A" },
    { NULL,                 FN1, dummy_func,
	" ",								"A" },
    { NULL,                 FN16, exit_func,
	" Exit Telesales ",						"A" },
    END_KEYS
};

static	int	prcl_func (int c, KEY_TAB *psUnused);
static	int	prclexp_func (int c, KEY_TAB *psUnused);
static	int	abort_func (int c, KEY_TAB *psUnused);
static	int	ok_func (int c, KEY_TAB *psUnused);

static	KEY_TAB prcl_keys [] =
{
#ifdef	GVISION
    { " EXPLODE KIT ",      'E', prclexp_func,
#else
    { " [E]XPLODE KIT",      'E', prclexp_func,
#endif
	" Show Contents Of Kit ",					"E" },
    { NULL,                 '\r', prcl_func,
	" Select Parcel And Enter Order Quantity ",			"A" },
    { NULL,                 FN1, abort_func,
	" Exit Parcel Selection ",					"A" },
    { NULL,                 FN16, ok_func,
	" Exit Parcel Selection ",					"A" },
    END_KEYS
};
static	int	item_func (int c, KEY_TAB *psUnused);
static	KEY_TAB item_keys [] =
{
    { NULL,                 '\r', item_func,
	" Select Item And Enter Order Quantity ",			"A" },
    { NULL,                 FN1, abort_func,
	" Abort Item Selection ",					"A" },
    { NULL,                 FN16, ok_func,
	" Exit Item Selection ",					"A" },
    END_KEYS
};

static	int	PromoFunc (int c, KEY_TAB *psUnused);
static	KEY_TAB prom_keys [] =
{
    { NULL,                 '\r', PromoFunc,
	" Select Promotional Item And Enter Order Quantity ",		"A" },
    { NULL,                 FN1, abort_func,
	" Exit Promotional Item Selection ",				"A" },
    { NULL,                 FN16, ok_func,
	" Exit Promotional Item Selection ",				"A" },
    END_KEYS
};

static	int	mtag_func (int c, KEY_TAB *psUnused);
static	KEY_TAB mail_keys [] =
{

#ifdef	GVISION
    { " TAG/UNTAG ",        'T', mtag_func,
#else
    { " [T]AG/UNTAG",        'T', mtag_func,
#endif
	" Tag Mailer For Sending ",					"A" },
    { NULL,                 '\r', mtag_func,
	" Tag Mailer For Sending ",					"A" },
    { NULL,                 FN1, abort_func,
	" Exit Mail Selection ",					"A" },
    { NULL,                 FN16, ok_func,
	" Exit Mail Selection ",					"A" },
    END_KEYS
};

MENUTAB call_menu  [] =
{
	{ " 1. PLANNED CALL     ",
	  " Make A Planned Call " },
	{ " 2. MAILER CALL      ",
	  " Make A Call Following On From A Mailer " },
	{ " 3. RANDOM CALL      ",
	  " Make A Random Call " },
	{ " 4. RECEIVE CALL     ",
	  " Receive A Call From A Lead " },
	{ " 5. EXIT TO MENU     ",
	  " Exit Tele-Sales " },
	{ ENDMENU }
};

#ifdef GVISION
MENUTAB notes_menu  [] =
{
	{ " 1. LAST CALL NOTES  ",
	  " Edit Last Call Notes" },
	{ " 2. NOTES            ",
	  " Edit Notes" },
	{ " 3. COMPLAINTS       ",
	  " Edit Complaints" },
	{ ENDMENU }
};

#define	MM_LAST_CALL	0
#define	MM_NOTES		1
#define	MM_COMPLAINTS	2

#endif	/* GVISION */

int RingCallNotes 		(void);
int RingCreditNotes 	(void);
int RingSalesHistory 	(void);
int RingPromoItem 		(void);
int RingBrandPromo 		(void);
int RingTagMailer 		(void);
int RingParcelPromo 	(void);
int RingNegoScreen 		(void);
int RingItemHistory 	(void);
int RingNextVisit 		(void);

#ifndef GVISION
menu_type	opt_menu [] = {
    {"<Call Notes>",            " Maintain Call Notes.  [C]",
	RingCallNotes,   "Cc", 0, ALL },
    {"<Credit Control Notes>",  " Display Credit Control Notes.  [R]",
	RingCreditNotes ,   "Rr", 0, ALL },
    {"<Sales History>",         " Select From Sales History.  [S]",
	RingSalesHistory,   "Ss", 0, ALL },
    {"<Promotional Item>",      " Select Promotional Items.  [O]",
	RingPromoItem,   "Oo", 0, ALL },
    {"<Brand Promotion>",       " Select From Brand Promotions.  [B]",
	RingBrandPromo,   "Bb", 0, ALL },
    {"<Tag Mailers>",           " Tag And Send Mailers.  [T]",
	RingTagMailer,  "Tt", 0, ALL },
    {"<Parcel Promotion>",      " Select Parcel Promotions.  [P]",
	RingParcelPromo,  "Pp", 0, ALL },
    {"<Negotiation Screen>" ,   " Item Price Negotiation.   [N]",
	RingNegoScreen,  "Nn", 0, ALL },
    {"<Item Sales History>" ,   " Display Specific Item History.   [I]",
	RingItemHistory,  "Ii", 0, ALL },
    {"<Arrange Visit>",         " Arrange Next Visit/Call Date/Time.   [A]",
	RingNextVisit, "Aa", 0, ALL },
    {"<EDIT/END>",        " Exit From Option Menu.   [EDIT/END]",
	_no_option, "", FN16, ALL },
    {"",}
};
#else
menu_type	opt_menu [] = {
    {0, "<Call Notes>",            " Maintain Call Notes",
	RingCallNotes, 0, ALL },
    {0, "<Credit Control Notes>",  " Display Credit Control Notes",
	RingCreditNotes , 0, ALL },
    {0, "<Sales History>",         " Select From Sales History",
	RingSalesHistory, 0, ALL },
    {0, "<Promotional Item>",      " Select Promotional Items",
	RingPromoItem, 0, ALL },
    {0, "<Brand Promotion>",       " Select From Brand Promotions",
	RingBrandPromo, 0, ALL },
    {0, "<Tag Mailers>",           " Tag And Send Mailers",
	RingTagMailer, 0, ALL },
    {0, "<Parcel Promotion>",      " Select Parcel Promotions",
	RingParcelPromo, 0, ALL },
    {0, "<Negotiation Screen>" ,   " Item Price Negotiation",
	RingNegoScreen, 0, ALL },
    {0, "<Item Sales History>" ,   " Display Specific Item History",
	RingItemHistory, 0, ALL },
    {0, "<Arrange Visit>",         " Arrange Next Visit/Call Date/Time",
	RingNextVisit, 0, ALL },
    {0, "", }
};
#endif

char	*scn_desc [] = { "", "", "", "", "",
		"ITEM SCREEN",
		"ORDER DETAILS SCREEN", "" };

int		DIFF_OP;
char	log_name [15];

char	headOffice [7];
char	order_ref [17];
char	get_buf [250];
char	upper_op [15];

int		in_edit_all;
int		no_tablines;
int		store_line;
int		cc1;
int		first_notes;
int		more_custs;
int		no_custs;
int		no_items;
int		call_type;
int		exit_call;
int		make_list;
int		list_made;
int		curr_mnth;
int		curr_day;
int		upd_visit;
int		upd_phone;
int		checking_comp;
int		nextSoicRef = 0;
int		envVarSoDoi;
int		envVarQcApply = FALSE,
		envVarSkQcAvl = FALSE;

long	currHhcuHash;
long	mail_hash;
long	po_due_date;
long	progPid;
char	envVarSoOverrideQty [2];

float	po_qty_ord;

struct CUST_LIST {
	long	hhcuHash;
	char	dbt_no [7];
	char	dbt_name [41];
	char	bst_ph_time [6];
	long	nxt_ph_date;
	int		ph_freq;
	char	phone_no [16];
	double	acc_bal;
	double	crd_lmt;
	char	terms [4];
	char	exceed [2];
	struct  CUST_LIST *prev;
	struct  CUST_LIST *next;
};

#define	CUST_NULL ((struct CUST_LIST *) NULL)

struct	CUST_LIST *Header_ptr = CUST_NULL;
struct	CUST_LIST *curr_ptr = CUST_NULL;

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
const	char	 *GetPriceDesc	(int);
double	OrderValue				(void);
float	ProcessPhantom			(long,float);
float	ReCalcAvail				(void);
float	RealTimeCommitted		(long,long,int);
float	ScreenDisc				(float);
float	ToLclUom				(float);
float	ToStdUom				(float);
int		ActivateCumr			(void);
int		AddIncc					(void);
int		AddItem					(long);
int		AllZero					(void);
int		ChampainCall			(void);
int		CheckBonus				(char *);
int		CheckContract			(void);
int		CheckCumr				(double);
int		CheckSohr				(long);
int		ContractDetails			(void);
int		CreateCumr				(void);
int		CreditExceeded			(double);
int		CustomerHeading			(int);
int		Deallocate				(void);
int		DeleteLine				(void);
int		DisplaySales			(void);
int		FindPos					(struct	CUST_LIST *);
int		FindSuper				(char *);
int		GenSoicRef				(void);
int		GetHoCustomer			(void);
int		GetNextPhoneDate		(void);
int		GetOperator				(void);
int		InputRes				(int);
int		LoadList				(void);
int		LoadPrcl				(void);
int		LoadText				(char);
int		LogLostSales			(float);
int		MarginOK				(double, float, double, float);
#ifndef GVISION
int		OpenSkWin				(void);
#endif
int		ProcessCall				(void);
int		ProcessSOH				(void);
int		RandomCall				(void);
int		ScanItemTab				(void);
int		UpdateText				(char);
int		UpdateTsls				(void);
int		ValidItemNo				(int);
int		ValidTime				(char *);
int		WarnUser				(char *, int);
int		heading					(int);
int		spec_valid				(int);
int		win_function			(int, int, int, int);
void	AddList					(char *,long, float, char *);
void	AddNewSoic				(int);
void	CalcSales				(void);
void	CalculateExtend			(int);
void	CalculateTotal			(void);
void	CheckEnvironment		(void);
void	CheckForComp			(void);
void	CheckTabLines			(void);
void	ClearBox				(int, int, int, int);
void	ClearSoic				(void);
void	ClearStore				(void);
void	CloseDB					(void);
void	DiscProcess				(void);
void	DrawTotal				(void);
void	FindPromo				(long, char *,char *, char *,char *);
void	FreeTheList				(void);
void	InitLine				(void);
void	InitML					(void);
void	MonthlySales			(long);
void	OpenDB					(void);
void	OpenItemTab				(void);
void	POrderDate				(long, long, float);
void	PriceProcess			(void);
void	PrintCoStuff			(void);
void	PrintTotal				(void);
void	ProcList				(void);
void	ProcSoic				(int, int);
void	ProcessKitItem			(long, float);
void	ReadCudp				(void);
void	ReadMisc				(void);
void	SetOrder				(void);
void	SetPeriodHeader			(char *);
void	SrchCnch				(char *);
void	SrchExsf				(char *);
void	SrchExaf				(char *);
void	SrchInum				(char *);
void	SrchTsbc				(char *);
void	SrchTslh				(char *);
void	TsCallMenu				(void);
void	UpdSoicQty				(void);
void	Update					(void);
void	shutdown_prog			(void);
void	tab_other				(int);

static int select_func 			(int, KEY_TAB *);
static int exit_func 			(int, KEY_TAB *);
static int PromoFunc 			(int, KEY_TAB *);
static int item_func 			(int, KEY_TAB *);
static int mtag_func 			(int, KEY_TAB *);

static struct CUST_LIST *cust_alloc 	(void);
static struct SEL_LIST *sel_alloc 		(void);
static int dummy_func 			(int, KEY_TAB *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int	i;

	char	*sptr = getenv ("LOGNAME");

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;
	
	_mail_ok = FALSE;
	no_tablines = 10;
	tab_max_page = 200;
	SR_Y_POS = 1;
	X_EALL = 0;
	Y_EALL = 7;

	/*-------------
	| Set LOGNAME |
	-------------*/
	if (!sptr)
	{
		errmess (ML (mlTsMess086));
		sleep (sleepTime);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	currentUser = sptr;
	sprintf (log_name, "%-14.14s", sptr);
	sprintf (upper_op, "%-14.14s", log_name);
	upshift (upper_op);

	progPid = (long) getpid ();

	/*---------------
	| Get gst code. |
	---------------*/
	sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVarGst = 0;
	else
		envVarGst = (*sptr == 'Y' || *sptr == 'y');

	if (envVarGst)
	{
		sprintf (taxPrompt, "%-3.3s Code              ", envVarGstTaxName);
		sprintf (taxNumber, "%-3.3s Number            ", envVarGstTaxName);
	}
	else
	{
		sprintf (taxPrompt, "Tax Code                 ");
		sprintf (taxNumber, "Tax Number               ");
	}

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	envVarSoDoi = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;

	sptr = chk_env ("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);


	/*------------------------------------
	| Get Value to allow override option |
	------------------------------------*/
	sprintf (envVarSoOverrideQty,"%-1.1s",get_env ("SO_OVERRIDE_QTY"));

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 2;
	MENU_COL = 0;
	input_row = 21;

	CheckEnvironment ();

	if (argc != 4 && argc != 5)
	{
		print_at (0,0, mlTsMess712 , argv [0]);
		print_at (1,0, mlTsMess713);
		return (EXIT_FAILURE);
	}

	argv [1] [0] = toupper (argv [1] [0]);

	sprintf (createFlag,"%-1.1s",argv [1]);

	printerNumber = (argc == 5) ? atoi (argv [4]) : 0;

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	abc_dbopen (data);
	ReadMisc ();

	sptr = chk_env ("SO_OTHER_1");
	sprintf (local_rec.other [0],"%.30s", (sptr == (char *)0)
						? "Other Costs." : sptr);
	sptr = chk_env ("SO_OTHER_2");
	sprintf (local_rec.other [1],"%.30s", (sptr == (char *)0)
						? "Other Costs." : sptr);
	sptr = chk_env ("SO_OTHER_3");
	sprintf (local_rec.other [2],"%.30s", (sptr == (char *)0)
						? "Other Costs." : sptr);
	if (STANDARD)
	{
		vars [label ("order_no")].fill = " ";
		vars [label ("order_no")].just = JUSTLEFT;
	}

	SETUP_SCR (vars);


	init_scr ();
	set_tty ();

	/*-------------------------
	| Check department number |
	-------------------------*/
	sprintf (cudp_rec.dp_no, "%2.2s", argv [3]);
	ReadCudp ();

	_set_masks (argv [2]);

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (6, store, sizeof (struct storeRec));
#endif
	for (i = 0;i < 7;i++)
		tab_data [i]._desc = scn_desc [i];

	envVarDBCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	strcpy (branchNumber, (envVarDBCo) ? comm_rec.est_no : " 0");

	sprintf (local_rec.customerDate, "%10.10s", DateToString (comm_rec.dbt_date));
	sprintf (local_rec.systemDate, "%10.10s", DateToString (TodaysDate ()));

	local_rec.lsystemDate = TodaysDate ();
	DateToDMY (local_rec.lsystemDate, NULL, &curr_mnth, NULL);
	curr_day = local_rec.lsystemDate % 7;  

	swide ();
	clear ();

	/*---------------------------
	| Open main database files. |
	---------------------------*/

	print_at (0,0, mlStdMess035);

	fflush (stdout);
	OpenDB ();
	InitML ();
	
	OpenPrice ();
	OpenDisc ();

	local_rec.prevPackSlip = 0L;

	if (!GetOperator ())
	{
		errmess (ML (mlStdMess139));
		sleep (sleepTime);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*--------------------
		| Clear store array. |
		--------------------*/
		ClearStore ();

		clear_ok = TRUE;
		call_type = -1;
		heading (0);

		TsCallMenu ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
InitML (void)
{
	sprintf (hd_scn [0], " %s ",ML (mlTsMess717));
	sprintf (hd_scn [1], " %s ",ML (mlTsMess718));
	sprintf (hd_scn [2], " %s ",ML (mlTsMess719));

	strcpy (mlTsCall  [1], ML ("YOU   DO   NOT   HAVE   ANY   LEADS   ALLOCATED   TO   YOU   FOR   TODAY"));
	strcpy (mlTsCall  [2], ML ("THERE ARE NO PARCEL OFFERINGS"));
}

/*-------------------------------------
| Set up program based on environment |
-------------------------------------*/
void
CheckEnvironment (void)
{
	char	*sptr;

	sptr = chk_env ("SO_DIS_INDENT");
	envVarSoDisIndent = (sptr) ? atoi (sptr) : TRUE;

	sptr = chk_env ("SO_LOST_SALES");
	envVarSoLostSales = (sptr) ? atoi (sptr) : TRUE;

	/*-------------------------------------------
	| Check for sales order full supply option. |
	-------------------------------------------*/
	sptr = chk_env ("SO_FULL_SUPPLY");
	envVarSoFullSupply = (sptr) ? atoi (sptr) : 0;

	/*---------------------------------
	| Check for sales order analysis. |
	---------------------------------*/
	sptr = chk_env ("SO_SALES");
	envVarSoSales = (sptr) ? atoi (sptr) : 0;

	/*-------------------------------
	| Check for bonus/hidden items. |
	-------------------------------*/
	sptr = chk_env ("SO_SPECIAL");
	sprintf (envVarSoSpecial,"%-4.4s", (sptr) ? sptr : "/B/H");

	/*--------------------------------------
	| Check for sales order margin checks. |
	--------------------------------------*/
	sptr = chk_env ("SO_MARGIN");
	sprintf (envVarSoMargin, "%-2.2s", (sptr) ? sptr : "00");

	/*------------------------------------------------------------
	| Check if forward orders are included into available stock. |
	------------------------------------------------------------*/
	sptr = chk_env ("SO_FWD_AVL");
	envVarSoFwdAvl = (sptr) ? atoi (sptr) : TRUE;

	/*-----------------------------
	| Check if discount is input. |
	-----------------------------*/
	sptr = chk_env ("INP_DISC");
	envVarInpDisc = (sptr) ? (*sptr == 'M' || *sptr == 'm') : 0;

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVarWinOk = (sptr) ? atoi (sptr) : 1;

	/*-----------------------------------------------------------
	| Check if stock information window is loaded at load time. |
	-----------------------------------------------------------*/
	sptr = chk_env ("SO_PERM_WIN");
	envVarSoPermWin = (sptr) ? atoi (sptr) : 0;
#ifndef GVISION
	if (envVarSoPermWin)
	{
		if (OpenSkWin ())
			envVarWinOk = FALSE;
	}
#endif

	/*-------------------------------------
	| Check Parcel Explosion Is Automatic |
	-------------------------------------*/
	sptr = chk_env ("TS_PRCLEXP");
	envVarTsPrclexp = (sptr) ? (*sptr == 'Y' || *sptr == 'y') : 0;

	/* QC module is active or not. */
	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	/* Whether to include QC qty in available stock. */
	envVarSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (pout);
		IP_CLOSE (np_fn);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{

	abc_alias (cumr2, cumr);
	abc_alias (inmr2, inmr);
	abc_alias (sohr2, sohr);
	abc_alias (tsal2, tsal);
	abc_alias (soic2, soic);
	abc_alias (inum2, inum);

	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no"
			 				     : "cumr_id_no3");
	open_rec (cumr2,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");

	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (soic,  soic_list, SOIC_NO_FIELDS, "soic_id_no2");
	open_rec (soic2, soic_list, SOIC_NO_FIELDS, "soic_id_no");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (tspm,  tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
	open_rec (tsxd,  tsxd_list, TSXD_NO_FIELDS, "tsxd_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (incp,  incp_list, incp_no_fields, "incp_id_no2");
	open_rec (cncl,	cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec (cnch,	cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd,	cncd_list, cncd_no_fields, "cncd_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (inmr2);
	abc_fclose (inei);
	abc_fclose (incc);
	abc_fclose (incp);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (soic);
	abc_fclose (soic2);
	abc_fclose (poln);
	abc_fclose (tspm);
	abc_fclose (tsxd);
	abc_fclose (exsf);
	abc_fclose (exaf);
	abc_fclose (cncl);
	abc_fclose (cnch);
	abc_fclose (cncd);

	ClosePrice ();
	CloseDisc ();

	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*------------------
| Read cudp record |
------------------*/
void
ReadCudp (void)
{
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");

	strcpy (cudp_rec.co_no, comm_rec.co_no);
	strcpy (cudp_rec.br_no, comm_rec.est_no);
	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
	{
		clear ();
		print_mess (ML (mlStdMess084));
		sleep (sleepTime);
		shutdown_prog ();
	}

	abc_fclose (cudp);
}

void
ClearStore (void)
{
	int		i;

	for (i = 0; i < MAXLINES; i++)
		memset ((char *)&store [i], '\0', sizeof (struct storeRec));
}

int
spec_valid (
 int    field)
{
	int	i;
	int	this_page;
	char	tmp_item [17];
	char	*sptr;

	/*-------------------------
	| validate customer contract
	-------------------------*/
	if (LNCHECK ("cont_no", 7))
	{
		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
			sprintf (local_rec.cont_desc, "%40.40s", " ");
			if (LCHECK ("cont_no1"))
				DSP_FLD ("cont_desc1");
			else
				DSP_FLD ("cont_desc2");
			return (EXIT_SUCCESS);
		}
		
		sprintf (local_rec.cont_desc, "%40.40s", " ");
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*-----------------------------------------
		| now see if contract is still current.    
		------------------------------------------*/
		
		if ((LCHECK ("cont_no2")) && cnch_rec.date_exp < local_rec.lsystemDate)
		{
			sprintf (err_str,ML (mlTsMess061),DateToString (cnch_rec.date_exp));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((LCHECK ("cont_no1")) && (cnch_rec.date_exp < local_rec.lsystemDate))
		{
			sprintf (err_str,ML (mlTsMess061),DateToString (cnch_rec.date_exp));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}


		if ((LCHECK ("cont_no2")) && (cnch_rec.date_wef > local_rec.lsystemDate))
		{
			sprintf (err_str,ML (mlTsMess062),DateToString (cnch_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((LCHECK ("cont_no1")) && (cnch_rec.date_wef > local_rec.lsystemDate))
		{
			sprintf (err_str,ML (mlTsMess062),DateToString (cnch_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*-----------------------------------------
		| now see if contract is assigned to customer
		------------------------------------------*/
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlTsMess063));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.cont_desc, cnch_rec.desc);

		if (LCHECK ("cont_no1"))
			DSP_FLD ("cont_desc1");
		else
			DSP_FLD ("cont_desc2");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.customerNo));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			if (call_type != RECEIVE ||
			     (call_type == RECEIVE && !CreateCumr ()))
			{
				/*print_mess ("\007 Customer Not Found On File ");*/
				print_mess (ML (mlStdMess021));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*----------------------
		| Check currency code. |
		----------------------*/
		if (FGN_CURR)
		{
			print_mess (ML (mlTsMess008));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		tspm_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
		if (cc)
		{
			if (call_type != RECEIVE ||
			    (call_type == RECEIVE && !ActivateCumr ()))
			{
				/*print_mess ("\007 Customer Not Active For Telesales ");*/
				print_mess (ML (mlTsMess002));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (call_type == RANDOM && 
		    strcmp (tspm_rec.op_code, upper_op))
		{
			/*print_mess ("\007 Customer Is Not Allocated To You ");*/
			print_mess (ML (mlTsMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		GetHoCustomer ();

		currHhcuHash = cumr_rec.hhcu_hash;
		sprintf (local_rec.customerName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("customerName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("letterCode"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchTslh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tslh_rec.co_no, comm_rec.co_no);
		sprintf (tslh_rec.let_code, "%-10.10s", local_rec.letterCode);
		cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess ("\007 Letter Does Not Exist On File ");*/
			print_mess (ML (mlStdMess109));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		mail_hash = tslh_rec.hhlh_hash;
		sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
		DSP_FLD ("letterDesc");

		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("item_no"))
	{
		if (last_char == FN16)
		{
			exit_call = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (last_char == DELLINE)
			return (DeleteLine ());

		return (ValidItemNo (TRUE));
	}

	/*----------------------------
	| Validate Item Description. |
	----------------------------*/
	if (LCHECK ("descr"))
	{
		if (NON_STOCK || !strcmp (inmr_rec.item_no,"NS              "))
			skip_entry = goto_field (field, label ("due_date"));

		return (EXIT_SUCCESS);
	}

	/*----------------
	| Validate Bonus |
	----------------*/
	if (LCHECK ("bonus"))
	{
		sprintf (tmp_item, "%-16.16s", local_rec.item_no);
		sptr = clip (tmp_item);
		if (local_rec.bonus [0] == 'Y')
		{
			sprintf (soBonus, "%-2.2s", envVarSoSpecial);
			if (strlen (tmp_item) <= 14)
			{
				strcat (tmp_item, soBonus);
				sprintf (local_rec.item_no, "%-16.16s",tmp_item);

				SR.taxAmt 			= 0.00;
				soln_rec.tax_pc		= 0.00;
				soln_rec.dis_pc		= 0.00;
				soln_rec.gst_pc		= 0.00;
				soln_rec.sale_price = 0.00;
				SR.salePrice 		= soln_rec.sale_price;
				SR.actualSale 		= soln_rec.sale_price;
				SR.discPc 			= ScreenDisc (soln_rec.dis_pc);
				SR.taxPc 			= soln_rec.tax_pc;
				SR.gstPc 			= soln_rec.gst_pc;

				DSP_FLD ("sale_price");
			}
			else
				strcpy (local_rec.bonus, "N");
		}
		else
		{
			if (strlen (sptr) > 2)
			{
				sptr += (strlen (sptr) - 2);
				if (*sptr == envVarSoSpecial [0]  && 
				    * (sptr + 1) == envVarSoSpecial [1])
				{
					*sptr = '\0';
					sprintf (local_rec.item_no, "%-16.16s", tmp_item);
				}
			}
			ValidItemNo (FALSE);
		}

		strcpy (SR.bonusFlag, local_rec.bonus);
		DSP_FLD ("item_no");
		DSP_FLD ("bonus");

		/*-------------------------------
		| Check for complimentary items |
		-------------------------------*/
		CheckForComp ();

		return (EXIT_SUCCESS);
	}

	/*==========================
	| Validate Unit of Measure |
	==========================*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			SR.hhumHash = inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
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

		strcpy (local_rec.UOM, inum2_rec.uom);
		SR.hhumHash = inum2_rec.hhum_hash;
		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;

		SR.cnvFct = inum2_rec.cnv_fct / inum_rec.cnv_fct;

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Salesman Code At Item Level. |
	---------------------------------------*/
	if (LCHECK ("sale_code"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (soln_rec.sman_code,sohr_rec.sman_code);

		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Customer Order Ref At Item Level. |
	--------------------------------------------*/
	if (LCHECK ("ord_ref"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (soln_rec.cus_ord_ref,sohr_rec.cus_ord_ref);

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Pack Size At Item Level. |
	-----------------------------------*/
	if (LCHECK ("pack_size"))
	{
		if (dflt_used || FIELD.required != YES)
		{
			strcpy (soln_rec.pack_size,SR.packSize);
			DSP_FLD ("pack_size");
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Quantity Input. |
	--------------------------*/
	if (LCHECK ("qty_ord"))
	{
		if (NON_STOCK && local_rec.qty_ord != 0.00)
		{
			local_rec.qty_ord = 0.00;
			print_mess (ML (mlTsMess064));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
		SR.qtyTotal = ToStdUom (local_rec.qty_ord);

		if (KIT_ITEM)
		{
			this_page = line_cnt / TABLINES;
			ProcessKitItem (inmr_rec.hhbr_hash, ToStdUom (local_rec.qty_ord));
			skip_entry = goto_field (label ("qty_ord"),
		                 	         label ("item_no"));
			SR.qtyTotal = 0.00;
			local_rec.qty_ord = 0.00;
			if (this_page == (line_cnt / TABLINES))
				blank_display ();
			return (EXIT_SUCCESS);
		}
		if (PHANTOM)
			SR.qtyAvailable = ProcessPhantom (inmr_rec.hhbr_hash,
						      			 local_rec.qty_ord);
		if (prog_status != ENTRY)
			SR.qtyTotal += local_rec.qty_back;

		if (prog_status == ENTRY)
			local_rec.qty_back = 0.00;

		/*-------------------------------------------------------
		| Serial Items Can only have Qty of 0.00 or 1.00	|
		-------------------------------------------------------*/
		if (!MULT_QTY && local_rec.qty_ord != 0.00 &&
                     local_rec.qty_ord != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*-------------------------------------------------
		| Recalculate the actual current available stock. |
		-------------------------------------------------*/
		SR.qtyAvailable = ReCalcAvail ();

		PriceProcess ();
		DiscProcess ();

		strcpy (local_rec.qty_chk, "N");
		sprintf (local_rec.soh_detail, "%-26.26s", " ");
		DSP_FLD ("soh_detail");

		if (!STANDARD && 
			envVarWinOk && 
			 ((SR.qtyAvailable - ToStdUom (local_rec.qty_ord)) < 0.00) && 
			!NO_COST && 
			!NON_STOCK)

		{
			ProcessSOH ();
			DSP_FLD ("soh_detail");

			if (skip_entry != 0)
				return (EXIT_SUCCESS);

			DSP_FLD ("qty_ord");
		}

		if (prog_status == ENTRY && NO_COST && F_HIDE (label ("cost_price")))
		{
			print_at (6, 1, "%-95.95s", " ");
			fflush (stdout);
			print_at (6, 1, ML (mlTsMess075));

			SR.costPrice = getmoney (20,6,"NNNNNNN.NN");
		}
		else
			SR.costPrice = 0.00;

		skip_entry = (!BONUS && soln_rec.sale_price == 0.0) ? 1 : 3;

		if (prog_status == ENTRY)
			DSP_FLD ("qty_back");

		if (BONUS)
		{
			soln_rec.due_date = sohr_rec.dt_required;
			DSP_FLD ("due_date");
			skip_entry = goto_field (field, label ("due_date"));
		}
		else
		{
			if (NO_COST)
				skip_entry = TRUE;
			else
			{
				if (soln_rec.sale_price == 0.00)
					skip_entry = 2;
				else
				{
					if (soln_rec.dis_pc == 0.00 && envVarInpDisc)
						skip_entry = 3;
					else
						skip_entry = 4;
				}
			}
		}
		SR.qtyOrder = local_rec.qty_ord;

		if (prog_status != ENTRY || skip_entry >= 2)
			CalculateTotal ();

		if (MARGIN_OK)
		{
			if (MarginOK (SR.actualSale, SR.discPc,
				       SR.margCost, SR.minMarg))
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
			else
				strcpy (SR.marginFailed, "N");
		}
		else
			strcpy (SR.marginFailed, "N");

		/*----------------------------------------
		| Update the real-time committal record. |
		----------------------------------------*/
		UpdSoicQty ();

		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Quantity Backordered. |
	--------------------------------*/
	if (LCHECK ("qty_back"))
	{
		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);
		if (NON_STOCK && local_rec.qty_back != 0.00)
		{
			local_rec.qty_back = 0.00;
			print_mess (ML (mlTsMess064));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		if (!BO_OK)
		{
			if (local_rec.qty_back != 0.00)
			{
				print_mess (ML (mlStdMess030));
				sleep (sleepTime);
				local_rec.qty_back = 0.00;
				DSP_FLD ("qty_back");
			}
			return (EXIT_SUCCESS);
		}

		if (local_rec.qty_back > SR.qtyTotal)
		{
			sprintf (err_str, ML (mlTsMess065) ,local_rec.qty_back,SR.qtyTotal);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		PriceProcess ();
		DiscProcess ();

		SR.qtyTotal = ToStdUom (local_rec.qty_ord);
		SR.qtyTotal += local_rec.qty_back;
		CalculateTotal ();

		/*----------------------------------------
		| Update the real-time committal record. |
		----------------------------------------*/
		UpdSoicQty ();

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Cost Price Input. |
	----------------------------*/
	if (LCHECK ("cost_price"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			soln_rec.cost_price = 0.00;

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Sale Price Input. |
	----------------------------*/
	if (LCHECK ("sale_price"))
	{
		if (BONUS)
		{
			print_mess (ML (mlTsMess066));
			sleep (sleepTime);
			soln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			PriceProcess ();
			DiscProcess ();
			DSP_FLD ("sale_price");
		}
		if (SR.calcSalePrice != soln_rec.sale_price)
			strcpy (SR.priceOveride,"Y");

		/*---------------------------------
		| Calculate new GROSS sale price. |
		---------------------------------*/
		SR.grossSalePrice = no_dec (soln_rec.sale_price / (1.00 - (SR.regPc / 100.00)));
		SR.salePrice = GetCusGprice (SR.grossSalePrice, SR.regPc);
		soln_rec.sale_price = SR.salePrice;

		DiscProcess ();

		if (soln_rec.dis_pc == 0.00 && envVarInpDisc)
			skip_entry = 0;
		else
			skip_entry = 1;

		SR.actualSale = soln_rec.sale_price;

		CalculateTotal ();

		if (MARGIN_OK)
		{
			if (MarginOK (SR.actualSale, SR.discPc,
				       SR.margCost, SR.minMarg))
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
			else
				strcpy (SR.marginFailed, "N");
		}
		else
			strcpy (SR.marginFailed, "N");

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Date Required. |
	-------------------------*/
	if (LCHECK ("dt_required"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			sohr_rec.dt_required = local_rec.lsystemDate;

		if (sohr_rec.dt_required > local_rec.lsystemDate)
		{
			i = prmptmsg (ML (mlTsMess016),"YyNn",2,5);

			if (i == 'N' || i == 'n')
			{
				sohr_rec.dt_required = local_rec.lsystemDate;
				DSP_FLD ("dt_required");
			}
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Discount Percent. |
	----------------------------*/
	if (LCHECK ("disc"))
	{
		if (dflt_used)
		{
			strcpy (SR.discOveride, "N");
			DiscProcess ();
		}

		if (SR.contractPrice || SR.contractStatus == 2)
		{
			soln_rec.dis_pc = 0.00;
			SR.discA = 0.00;
			SR.discB = 0.00;
			SR.discC = 0.00;
			DSP_FLD ("disc");
		}

		if (SR.calcDisc != ScreenDisc (soln_rec.dis_pc))
			strcpy (SR.discOveride,"Y");

		SR.discPc   = ScreenDisc (soln_rec.dis_pc);

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

		CalculateTotal ();

		if (MARGIN_OK)
		{
			if (MarginOK (SR.actualSale, SR.discPc,
				       SR.margCost, SR.minMarg))
				strcpy (SR.marginFailed, (MARG_HOLD) ? "Y" : "N");
			else
				strcpy (SR.marginFailed, "N");
		}
		else
			strcpy (SR.marginFailed, "N");

		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Due Date , Item Level. |
	---------------------------------*/
	if (LCHECK ("due_date"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			soln_rec.due_date = sohr_rec.dt_required;
			DSP_FLD ("due_date");
		}

		if (soln_rec.due_date > local_rec.lsystemDate &&
		     sohr_rec.dt_required != soln_rec.due_date)
		{
			i = prmptmsg (ML (mlTsMess016) ,"YyNn",2,5);

			if (i == 'N' || i == 'n')
			{
				soln_rec.due_date = local_rec.lsystemDate;
				DSP_FLD ("due_date");
			}
		}

		if (prog_status == ENTRY)
			skip_entry = 1;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sman_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			CustomerHeading (TRUE);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		sprintf (exsf_rec.salesman_no,"%-2.2s", sohr_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("sman_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("area_code"))
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			CustomerHeading (TRUE);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		sprintf (exaf_rec.area_code,"%-2.2s", sohr_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.area_desc, "%-30.30s", exaf_rec.area);
		DSP_FLD ("area_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("brand"))
	{
		if (SRCH_KEY)
		{
			open_rec (tsbc, tsbc_list, TSBC_NO_FIELDS, "tsbc_id_no");

			SrchTsbc (temp_str);

			abc_fclose (tsbc);

			return (EXIT_SUCCESS);
		}

		open_rec (tsbc,  tsbc_list, TSBC_NO_FIELDS, "tsbc_id_no");

		strcpy (tsbc_rec.co_no, comm_rec.co_no);
		sprintf (tsbc_rec.brand, "%-16.16s", local_rec.brand);
		cc = find_rec (tsbc, &tsbc_rec, COMPARISON, "r");
		if (cc)
		{
			if (dflt_used)
				print_mess (ML (mlTsMess067));
			else
				print_mess (ML (mlStdMess073));

			sleep (sleepTime);
			clear_mess ();
			abc_fclose (tsbc);
			return (EXIT_FAILURE);
		}
		abc_fclose (tsbc);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("n_vst_tm"))
	{
		if (!ValidTime (local_rec.n_visit_time))
		{
			print_mess (ML (mlStdMess142));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("n_vst_dt"))
	{
		if (dflt_used)
		{
			local_rec.n_visit_date = 0L;
			return (EXIT_SUCCESS);
		}

		if (local_rec.n_visit_date < local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("n_phn_tm"))
	{
		if (!ValidTime (local_rec.n_phone_time))
		{
			print_mess (ML (mlStdMess142));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("n_phn_dt"))
	{
		if (dflt_used)
		{
			local_rec.n_phone_date = 0L;
			return (EXIT_SUCCESS);
		}

		if (local_rec.n_phone_date < local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Process soic record for current line. |
---------------------------------------*/
void
ProcSoic (
 int        delLine,
 int        procLine)
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
				soic_rec.time_create = atot (TimeHHMM ());
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
					soic_rec.time_create = atot (TimeHHMM ());
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
 int    lineNo)
{
	strcpy (soic_rec.status, "A");
	soic_rec.pid  = progPid;
	soic_rec.line = GenSoicRef ();

	store [lineNo].commitRef = soic_rec.line;
	soic_rec.hhbr_hash = store [lineNo].hhbrHash;
	soic_rec.hhcc_hash = 0L;
	soic_rec.qty = (float) 0.00;
	sprintf (soic_rec.program, "%-20.20s", PNAME);
	sprintf (soic_rec.op_id, "%-14.14s", currentUser);
	soic_rec.time_create = atot (TimeHHMM ());
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
UpdSoicQty (void)
{
	float	updQty;
	float	lineQty;

	lineQty = ToStdUom (local_rec.qty_ord + local_rec.qty_back);
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
				soic_rec.pid = progPid;
				soic_rec.line = GenSoicRef ();
				store [line_cnt].commitRef = soic_rec.line;
				soic_rec.hhbr_hash = store [line_cnt].hhbrHash;
				soic_rec.hhcc_hash = 0L;
				soic_rec.qty = (float) 0.00;
				sprintf (soic_rec.program, "%-20.20s", PNAME);
				sprintf (soic_rec.op_id, "%-14.14s", currentUser);
				soic_rec.time_create = atot (TimeHHMM ());
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
		updQty = lineQty - store [line_cnt].origOrdQty;
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
	float	realStock		=	0.00,
			realCommitted	=	0.00;

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
	if (envVarSoFwdAvl)
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
	if (envVarQcApply && envVarSkQcAvl)
		realStock -= incc2_rec.qc_qty;

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
	int		ignLine)
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
CreateCumr (void)
{
	int	i;

	i = prmptmsg (ML (mlTsMess068) , "YyNn", 45, 5);

	move (0,5);
	cl_line ();
	if (i == 'N' || i == 'n')
		return (FALSE);

	sprintf (err_str, "ts_mr_inpt %s", local_rec.customerNo);

	if (sys_exec (err_str))
		return (FALSE);

	heading (1);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.customerNo));
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	return (TRUE);
}

int
ActivateCumr (void)
{
	int	i;

	i = prmptmsg (ML (mlTsMess017) , "YyNn", 35, 5);

	move (0,5);
	cl_line ();
	if (i == 'N' || i == 'n')
		return (FALSE);

	sprintf (err_str, "ts_active %08ld", cumr_rec.hhcu_hash);
	if (sys_exec (err_str))
		return (FALSE);

	tspm_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	return (TRUE);
}

/*---------------------
| Validate input time |
---------------------*/
int
ValidTime (
 char*  time_str)
{
	char	tmp_time [6];
	char	*tptr;
	int	tmp_hour, tmp_min, i;

	/*---------------------------
	| Replace spaces with zeros |
	---------------------------*/
	sprintf (tmp_time, "%-5.5s", time_str);
	tptr = tmp_time;
	i = 0;
	while (*tptr)
	{
		if (*tptr == ' ' && i != 2)
			*tptr = '0';
		i++;
		tptr++;
	}
	sprintf (time_str, "%-5.5s", tmp_time);
	time_str [2] = ':';

	tmp_hour = atoi (time_str);
	tmp_min = atoi (time_str + 3);

	if (tmp_hour > 23 || tmp_min > 59)
		return (FALSE);

	return (TRUE);
}

/*========================================
| Search routine for Letter master file. |
========================================*/
void
SrchTslh (
 char*      key_val)
{
	_work_open (10,0,40);
	save_rec ("#Letter","#Letter Description");
	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (tslh_rec.co_no, comm_rec.co_no) &&
		   !strncmp (tslh_rec.let_code, key_val, strlen (key_val)))
	{
		cc = save_rec (tslh_rec.let_code, tslh_rec.let_desc);
		if (cc)
			break;

		cc = find_rec (tslh, &tslh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tslh, "DBFIND");

	sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
}

/*========================================
| Search routine for Brand master file. |
========================================*/
void
SrchTsbc (
 char*      key_val)
{
	work_open ();
	save_rec ("#Brand Code","#Brand Description");
	strcpy (tsbc_rec.co_no, comm_rec.co_no);
	sprintf (tsbc_rec.brand, "%-16.16s", key_val);
	cc = find_rec ("tsbc", &tsbc_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (tsbc_rec.co_no, comm_rec.co_no) &&
		   !strncmp (tsbc_rec.brand, key_val, strlen (key_val)))
	{
		cc = save_rec (tsbc_rec.brand, tsbc_rec.brand_desc);
		if (cc)
			break;

		cc = find_rec (tsbc, &tsbc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	return;
}

/*======================
| Search for salesman. |
======================*/
void
SrchExsf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Sm","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && 
		   !strcmp (exsf_rec.co_no,comm_rec.co_no) &&
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

/*======================
| Search for salesman. |
======================*/
void
SrchExaf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Area","#Area Description.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && 
		   !strcmp (exaf_rec.co_no,comm_rec.co_no) &&
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
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

/*-----------------------------------------
| Calculate SOH and replenishment details |
-----------------------------------------*/
int
ProcessSOH (void)
{
	char	s_on_hand [11];

	strcpy (local_rec.qty_chk, "Y");

	sprintf (s_on_hand, "%7.0f", SR.qtyAvailable);

	POrderDate (alt_hash (SR.hhbrHash, SR.hhsiHash), ccmr_rec.hhcc_hash, 
		 SR.qtyAvailable);

	sprintf (local_rec.soh_detail,
		"%-7.7s|%-10.10s| %6.0f", s_on_hand, DateToString (po_due_date), po_qty_ord);

	return (EXIT_SUCCESS);
}

/*===================================================
| the due date from the purchase orders	            |
===================================================*/
void
POrderDate (
	long	hhbrHash,
	long	hhccHash,
	float	qty_avail)
{
	float	qty_left = 0.00;

	qty_left = qty_avail;
	qty_left *= -1.00;

	poln_rec.hhbr_hash = hhbrHash;
	poln_rec.due_date = 0L;

	cc = find_rec ("poln",&poln_rec,GTEQ,"r");
	/*-----------------------------------------------
	| Find poln records for the warehouse item	|
	-----------------------------------------------*/
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec ("poln",&poln_rec,NEXT,"r");
			continue;
		}
		if (poln_rec.hhcc_hash != hhccHash)
		{
			cc = find_rec ("poln",&poln_rec,NEXT,"r");
			continue;
		}
		po_qty_ord =  poln_rec.qty_ord - poln_rec.qty_rec;
		qty_left -= po_qty_ord;

		/*---------------------------------------
		| The P/O which put the qty into -ve	|
		| is the P/O to use.			|
		---------------------------------------*/
		if (qty_left < 0.00)
		{
			qty_left *= -1;
			po_qty_ord = (qty_left > po_qty_ord) ? po_qty_ord : qty_left;
			po_due_date = poln_rec.due_date;
			break;
		}
		cc = find_rec ("poln",&poln_rec,NEXT,"r");
	}

	return;
}

/*=================================
| Setup default values for Order. |
=================================*/
void
SetOrder (void)
{
	int	i;

	init_vars (7);

	strcpy (sohr_rec.order_no,"00000000");
	strcpy (sohr_rec.area_code,cumr_rec.area_code);
	strcpy (sohr_rec.sman_code,cumr_rec.sman_code);
	strcpy (sohr_rec.pri_type,cumr_rec.price_type);
	strcpy (local_rec.priceDesc, GetPriceDesc (atoi(sohr_rec.pri_type)));

	/*--------------------------------
	| Get any special instrunctions. |
	--------------------------------*/
	open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

	for (i = 0; i < 3; i++)
	{
	       strcpy (exsi_rec.co_no,comm_rec.co_no);
	       exsi_rec.inst_code = cumr_inst [i];
	       cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
	       if (cc)
		      sprintf (local_rec.spinst [i],"%60.60s"," ");
	       else
		      sprintf (local_rec.spinst [i],"%-60.60s",exsi_rec.inst_text);
	}
	abc_fclose (exsi);
	strcpy (sohr_rec.del_name,cumr_rec.dbt_name);
	/*------------------------
	| Get charge to address. |
	------------------------*/
	strcpy (sohr_rec.del_add1,cumr_rec.dl_adr1);
	strcpy (sohr_rec.del_add2,cumr_rec.dl_adr2);
	strcpy (sohr_rec.del_add3,cumr_rec.dl_adr3);

	strcpy (sohr_rec.cons_no,     sixteen_space);
	strcpy (sohr_rec.fix_exch,    "N");
	strcpy (sohr_rec.area_code,   cumr_rec.area_code);
	strcpy (sohr_rec.sman_code,   cumr_rec.sman_code);
	strcpy (sohr_rec.pri_type,    cumr_rec.price_type);
	strcpy (sohr_rec.sell_terms,  "   ");
	sprintf (sohr_rec.pay_term,   "%-40.40s", cumr_rec.crd_prd);
	strcpy (sohr_rec.sohr_new,         "Y");
	strcpy (sohr_rec.full_supply, "N");
	sprintf (sohr_rec.ins_det,    "%-30.30s", " ");

	sohr_rec.dt_raised   = comm_rec.dbt_date;
	sohr_rec.dt_required = local_rec.lsystemDate;
	sohr_rec.insurance   = 0.00;
	sohr_rec.deposit     = 0.00;
	sohr_rec.discount    = 0.00;
	sohr_rec.freight     = 0.00;

	sohr_rec.other_cost_1 = 0.00;
	sohr_rec.other_cost_2 = 0.00;
	sohr_rec.other_cost_3 = 0.00;

	strcpy (local_rec.priceDesc, GetPriceDesc (atoi(sohr_rec.pri_type)));
	strcpy (exsf_rec.co_no,  comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,  sohr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman, "%30.30s", " ");

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code,  sohr_rec.area_code);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		sprintf (local_rec.area_desc, "%30.30s", " ");
	else
		sprintf (local_rec.area_desc, "%30.30s", exaf_rec.area);
}

/*============================
| Warn user about something. |
============================*/
int
WarnUser (
 char*  wn_mess,
 int    wn_flip)
{
	int	i;

	clear_mess ();
	print_mess (wn_mess);

	if (!wn_flip)
	{
		i = prmptmsg (ML ("Enter 'Y' to continue / 'N' to cancel / 'M' for more information on credit details  [Y/N/M] ") ,"YyNnMm",2,5);
		print_at (6, 1, "%-95.95s", " ");
		if (i == 'Y' || i == 'y')
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm')
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML ("Do you wish to continue?"), "YyNn",2,5);
			heading (1);
			scn_display (1);
			print_at (6, 1, "%-95.95s", " ");
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

/*==========================
| Validate margin percent. |
==========================*/
int
MarginOK (
 double     sales,
 float      disc,
 double     csale,
 float      min_marg)
{
	float	marg = 0.00;

	if (SR.contractStatus)
		return (EXIT_SUCCESS);

	if (min_marg == 0.00)
		return (EXIT_SUCCESS);

	sales = DOLLARS (sales);
	sales -= (sales * ((double) disc / 100));

	/*---------------------------
	| Calculate margin percent. |
	---------------------------*/
	marg = (float) sales - (float) csale;
	if (sales != 0.00)
		marg /= (float) sales;
	else
		marg = 0.00;

	marg *= 100.00;

	if (marg < min_marg && !MARG_MESS1)
	{
		if (MARG_MESS2)
			PauseForKey (6,2, ML (mlTsMess020), 0);
		else
			PauseForKey (6,2, ML (mlTsMess715), 0);

		print_at (6, 2, "%-95.95s"," ");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*======================================================================
| Routine to check if bonus flag has been set (this is indicated by a  |
| '/B' on the end of the part number. If bonus flag is set then '/B'   |
| is removed from part number.                                         |
| Returns 0 if bonus flag has not been set, 1 if it has.               |
======================================================================*/
int
CheckBonus (
 char*  item_no)
{
	char	bonus_item [17];
	char	*sptr;

	sprintf (bonus_item, "%-16.16s", item_no);
	sptr = clip (bonus_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVarSoSpecial [0]  && * (sptr + 1) == envVarSoSpecial [1])
		{
			*sptr = '\0';
			sprintf (item_no, "%-16.16s", bonus_item);
			strcpy (local_rec.bonus, "Y");
			return (EXIT_FAILURE);
		}
	}
	sprintf (item_no, "%-16.16s", bonus_item);
	return (EXIT_SUCCESS);
}

void
PrintCoStuff (void)
{
	line_at (20,0,132);

	print_at (21,0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (21,45, ML (mlStdMess039),   comm_rec.est_no, comm_rec.est_name);


	print_at (22,0,  ML (mlStdMess099), comm_rec.cc_no,   comm_rec.cc_name);
	print_at (22,45, ML (mlStdMess085), cudp_rec.dp_no, cudp_rec.dp_name);

}
/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
void
ProcessKitItem (
	long	hhbrHash,
	float	qty)
{
	int		i;
	int		this_page;
	long	hold_date = sohr_rec.dt_required;
	char	*sptr;
	float	lcl_qty	=	0.00;

	this_page = line_cnt / TABLINES;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_id_no");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash 	= hhbrHash;
	sokt_rec.line_no 	= 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
	       		sokt_rec.hhbr_hash == hhbrHash)
	{
		inmr_rec.hhbr_hash	= sokt_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (sokt_rec.bonus [0] == 'Y')
		{
			sprintf (soBonus, "%-2.2s", envVarSoSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), soBonus);
		}
		else
			strcpy (local_rec.item_no, inmr_rec.item_no);

		dflt_used = FALSE;

		if (ValidItemNo (FALSE))
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (!MULT_QTY)
		{
			lcl_qty = sokt_rec.matl_qty * qty;
			local_rec.qty_ord = 1.00;
			local_rec.qty_back = 0.00;
		}
		else
		{
			local_rec.qty_ord = sokt_rec.matl_qty * qty;
			local_rec.qty_back = 0.00;
		}

		if (local_rec.qty_ord == 0.00)
			get_entry (label ("qty_ord"));

		if (local_rec.qty_ord == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.due_date == 0L)
			sohr_rec.dt_required = hold_date;
		else
			sohr_rec.dt_required = sokt_rec.due_date;

		/*-----------------------------
		| if serial we need to to load
		| one line per qty ordered.
		------------------------------*/

		if (!MULT_QTY)
		{
			int	count;

			abc_selfield (inmr, "inmr_hhbr_hash");
			inmr_rec.hhbr_hash = sokt_rec.mabr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");

			for (count = 0; count < lcl_qty; count++)
			{

				if (sokt_rec.bonus [0] == 'Y')
				{
					sprintf (soBonus, "%-2.2s", envVarSoSpecial);
					sptr = clip (inmr_rec.item_no);
					sprintf (local_rec.item_no, "%-s%-.*s",
						     sptr, 16 - (int) strlen (sptr), soBonus);
				}
				else
					strcpy (local_rec.item_no, inmr_rec.item_no);

				dflt_used = FALSE;

				if (ValidItemNo (FALSE))
					break;

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					DSP_FLD ("item_no");
					DSP_FLD ("descr");
					lcount [2] = line_cnt;
					this_page = line_cnt / TABLINES;
				}

				for (i = label ("item_no"); i <= label ("soh_detail"); i++)
				{
					skip_entry = 0;
					do
					{
						dflt_used 	=	TRUE;
				/*--------------------------
				| Check Quantity available |
				--------------------------*/
						cc = spec_valid (i);

						if (cc)
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
					SR.qtyOrder = local_rec.qty_ord;
	
					if (this_page != (line_cnt / TABLINES))
					{
						scn_write (cur_screen);
						lcount [2] = line_cnt;
						this_page = line_cnt / TABLINES;
					}
					lcount [2] = line_cnt;
					
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
				lcount [2] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			for (i = label ("item_no"); i <= label ("soh_detail"); i++)
			{
				skip_entry = 0;
				do
				{
					dflt_used 	=	TRUE;
					cc = spec_valid (i);
					/*-----------------------------------------
					| if spec_valid returns 1, re-enter field |
					| eg. if kit item has no sale value,      |
					| re-prompt for sal value if required.    |
					-----------------------------------------*/
					if (cc)
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
				SR.qtyOrder = local_rec.qty_ord;

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					lcount [2] = line_cnt;
					this_page = line_cnt / TABLINES;
				}
				lcount [2] = line_cnt;
			
				line_display ();
				line_cnt++;
			}
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [6] = line_cnt;
	sohr_rec.dt_required = hold_date;

	abc_fclose (sokt);
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
float
ProcessPhantom (
	long	hhbrHash,
	float	qty)
{
	int		firstTime = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;
	float	realCommitted;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");


	sokt_rec.hhbr_hash = hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 
										   incc_rec.hhcc_hash,
										   0);
		if (envVarSoFwdAvl)
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
		if (envVarQcApply && envVarSkQcAvl)
			on_hand -= incc_rec.qc_qty;
		on_hand /= sokt_rec.matl_qty;
		if (firstTime)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		firstTime = FALSE;
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

/*=======================================
| Add warehouse record for current W/H. |
=======================================*/
int
AddIncc (void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	incc_rec.hhwh_hash = 0L;
	sprintf (incc_rec.sort, "%s%11.11s%-16.16s",
		inmr_rec.inmr_class, inmr_rec.category, inmr_rec.item_no);

	incc_rec.first_stocked = local_rec.lsystemDate;
	incc_rec.closing_stock = 0.00;
	incc_rec.committed = 0.00;
	incc_rec.backorder = 0.00;
	strcpy (incc_rec.stat_flag,"0");

	cc = abc_add ("incc",&incc_rec);
	if (cc)
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	return (cc);
}

/*=====================================================================
| Input responses to stock quantity on hand less-than input quantity. |
=====================================================================*/
int
InputRes (
 int    line_no)
{
	int	i;
	int	fs_flag = FALSE;
	int	displayed = FALSE;
	float	wk_qty;
	char	val_keys [150];
	char	disp_str [300];

	cc = 0;

	if (envVarSoOverrideQty [0] == 'Y')
	{
		strcpy (val_keys, "OoCcNnAaRr");
		sprintf (disp_str,
				ML (mlTsMess089),
				ta [8], ta [9], ta [8], ta [9],
				ta [8], ta [9], ta [8], ta [9]);
	}
	else
	{
		strcpy (val_keys, "CcNnAaRr");
		sprintf (disp_str,
				ML ("%s (C)ancel%s %s (R)educe%s %sDisp (N) (A)%s "),
				ta [8], ta [9], 
				ta [8], ta [9], ta [8], ta [9]);
	}

	if (strcmp (inmr_rec.alternate,sixteen_space))
	{
		sprintf (err_str, ML (mlTsMess714), ta [8], ta [9]);
		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}

	if (BO_OK)
	{
		if (FULL_BO)
		{
			sprintf (err_str, ML (mlTsMess090), ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "Ff");
		}
		else
		{
			sprintf (err_str, ML (mlTsMess091), ta [8], ta [9], ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "BbFf");

		}
	}

	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 2, 6);

		switch (i)
		{
		/*------------------------
		| Accept Quantity input. |
		------------------------*/
		case	'O':
		case	'o':
			break;

		case	'B':
		case	'b':
			local_rec.qty_back = ToStdUom (local_rec.qty_ord);
			local_rec.qty_ord = SR.qtyAvailable;
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			local_rec.qty_back -= local_rec.qty_ord;
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_back");
			putval (line_no);
			fs_flag = TRUE;
			break;

		/*------------------------------------------------------
		| Cancel Quantity input and check if log to lost sale. |
		------------------------------------------------------*/
		case	'C':
		case	'c':
			LogLostSales (local_rec.qty_ord);
			local_rec.qty_ord = 0.00;
			DSP_FLD ("qty_ord");
			putval (line_no);
			break;

		/*------------------------------
		| Display Stock Status Window. |
		------------------------------*/
		case	'N':
		case	'n':
		case	'A':
		case	'a':
#ifdef GVISION
			if (i == 'N' || i == 'n')
				DisplayStockWindow (SR.hhsiHash, ccmr_rec.hhcc_hash);
			else
				DisplayStockWindow (SR.hhsiHash, 0L);
#else
			if (!wpipe_open)
			{
				if (OpenSkWin ())
					break;
			}
			if (i == 'N' || i == 'n')
				fprintf (pout,"%10ld%10ld\n", SR.hhsiHash,
						      ccmr_rec.hhcc_hash);
			else
				fprintf (pout,"%10ld%10ld\n",SR.hhsiHash,0L);

			fflush (pout);
			IP_READ (np_fn);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*------------------------------------------------------
		| Quantity has been reduced to equal quantity on hand. |
		------------------------------------------------------*/
		case	'R':
		case	'r':
			wk_qty = local_rec.qty_ord;
			local_rec.qty_ord = ToLclUom (SR.qtyAvailable);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			LogLostSales (wk_qty - local_rec.qty_ord);
			DSP_FLD ("qty_ord");
			putval (line_no);
			break;

		/*------------------------------
		| Substitute Alternate number. |
		------------------------------*/
		case	'S':
		case	's':
			sprintf (soBonus, "%-2.2s", envVarSoSpecial);
			sprintf (err_str,"%s%s", clip (inmr_rec.alternate),
						 (BONUS) ? soBonus : " ");

			sprintf (local_rec.item_no,"%-16.16s",err_str);
			if (ValidItemNo (FALSE))
				skip_entry = goto_field (label ("qty_ord"), label ("item_no"));
			else
				skip_entry = -1;

			DSP_FLD ("item_no");
			DSP_FLD ("descr");
			putval (line_no);
			break;

		case	'F':
		case	'f':
			local_rec.qty_back = local_rec.qty_ord;
			local_rec.qty_ord = 0.00;
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_back");
			putval (line_no);
			fs_flag = TRUE;
			break;
		}
		print_at (6, 1, "%95.95s"," ");

		if (i != 'D' && i != 'd')
			break;
	}
	return ((envVarSoFullSupply) ? fs_flag : FALSE);
}

/*==============
| Delete line. |
==============*/
int
DeleteLine (void)
{
	int		i, j;
	int		this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	this_page = line_cnt / TABLINES;

	add_hash (comm_rec.co_no, 
			  comm_rec.est_no, 
			  "RC", 
			  0,
			  SR.hhsiHash, 
			  ccmr_rec.hhcc_hash, 
			  progPid, 
			  (double) 0.00);

	/*-------------------------------
	| Process soic record for line. |
	-------------------------------*/
	ProcSoic (TRUE, line_cnt);

	for (i = line_cnt; line_cnt < lcount [6] - 1; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		memcpy 
		(
			(char *) &SR, 
			(char *) &store [line_cnt + 1], 
			sizeof (struct storeRec)
		);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	sprintf (local_rec.item_no,"%-16.16s"," ");
	sprintf (soln_rec.item_desc,"%-36.36s"," ");
	local_rec.qty_ord 	= 0.00;
	local_rec.qty_back 	= 0.00;
	soln_rec.cost_price = 0.00;
	soln_rec.sale_price = 0.00;
	soln_rec.dis_pc 	= 0.00;
	soln_rec.tax_pc 	= 0.00;
	soln_rec.due_date 	= 0L;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	for (j = lcount [2] - 1; j < lcount [2]; j++)
		memset ((char *) &store [j], '\0', sizeof (struct storeRec));

	lcount [6]--;

	line_cnt = i;
	getval (line_cnt);
	CalculateTotal ();
	return (EXIT_SUCCESS);
}

/*==========================================
| Main validation Routine for item number. |
==========================================*/
int
ValidItemNo (
 int        getFields)
{
	int		i;
	int		itemChanged = FALSE;
	char	*sptr;
	long	orig_hhbr_hash;
	float	realCommitted;

	strcpy (local_rec.qty_chk, "N");
	sprintf (local_rec.soh_detail, "%-26.26s", " ");

	abc_selfield (inmr, "inmr_id_no");

	skip_entry = 0;

	if (dflt_used)
	{
		if (strlen (clip (lastItem)) == 0 || prog_status != ENTRY)
			return (DeleteLine ());
		else
			sprintf (local_rec.item_no, "%-16.16s", lastItem);
	}

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.item_no);
	SR.bonusFlag [0] = (CheckBonus (inmr_rec.item_no)) ? 'Y' : 'N';

	cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
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
		return (EXIT_FAILURE);
	}

	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		/*------------------
		| Check for indent |
		------------------*/
		cc =	check_indent
				 (
					comm_rec.co_no, 
					comm_rec.est_no,
					ccmr_rec.hhcc_hash,
					inmr_rec.item_no
				);
	}
	if (cc)
	{
		print_mess (ML (mlStdMess001));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	orig_hhbr_hash = inmr_rec.hhbr_hash;

	SuperSynonymError ();

	sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);

	if (SR.hhbrHash != inmr_rec.hhbr_hash &&
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
	}

	SR.hhbrHash 		= 	inmr_rec.hhbr_hash;
	SR.hhsiHash 		= 	alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	SR.weight 			= 	inmr_rec.weight;
	SR.outerSize 		= 	inmr_rec.outer_size;
	SR.defaultDisc 		= 	inmr_rec.disc_pc;
	SR.itemClass [0] 	= 	inmr_rec.inmr_class [0];
	strcpy (SR.category,	inmr_rec.category);
	strcpy (SR.sellGroup,	inmr_rec.sellgrp);
	strcpy (SR.backOrder,	inmr_rec.bo_flag);
	strcpy (SR.boRelease,	inmr_rec.bo_release);
	strcpy (SR.packSize,	inmr_rec.pack_size);
	strcpy (SR.costingFlag,	inmr_rec.costing_flag);
	strcpy (SR.priceOveride,"N");
	strcpy (SR.discOveride,	"N");

	strcpy (inum_rec.uom, inmr_rec.sale_unit);
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	SR.hhumHash = inum_rec.hhum_hash;
	SR.cnvFct = inum_rec.cnv_fct;

	/*-------------------------
	| Check for indent items. |
	-------------------------*/
	if (strncmp (inmr_rec.item_no, "INDENT", 6) || envVarSoDisIndent)
		SR.indentItem = FALSE;
	else
		SR.indentItem = TRUE;

	if (FLD ("pack_size") != ND)
		FLD ("pack_size") = (!strcmp (inmr_rec.pack_size,"     "))
								? YES : NI;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,SR.category);
	cc = find_rec (excf,&excf_rec,COMPARISON,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess004));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	SR.minMarg = excf_rec.min_marg;
	SR.exchRate  = excf_rec.ex_rate;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
	{
		sprintf (err_str, ML (mlStdMess033) ,clip (inmr_rec.item_no));

		i = prmptmsg (err_str,"YyNn",2,5);
		if (i == 'n' || i == 'N')
		{
			skip_entry = -1;
			return (EXIT_SUCCESS);
		}
		else
		{
			cc = AddIncc ();
			if (cc)
				file_err (cc, "incc", "DBADD");
			SR.qtyAvailable = 0.00;
		}
	}
	inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei,&inei_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "inei", "DBFIND");

	SR.margCost = (inei_rec.avge_cost == 0.00) ? inei_rec.last_cost 						        : inei_rec.avge_cost;
	SR.margCost = out_cost (SR.margCost, SR.outerSize);

	strncpy (soln_rec.item_desc,inmr_rec.description,36);

	if (!BONUS)
	{
		SR.taxAmt = inmr_rec.tax_amount;
		soln_rec.tax_pc  = inmr_rec.tax_pc;
		soln_rec.gst_pc  = inmr_rec.gst_pc;
	}
	else
	{
		sprintf (soBonus, "%-2.2s", envVarSoSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no,"%-s%-.*s",
			sptr,16 - (int) strlen (sptr),soBonus);

		SR.taxAmt = 0.00;
		soln_rec.tax_pc  = 0.00;
		soln_rec.gst_pc  = 0.00;
	}

	SR.taxPc = soln_rec.tax_pc;
	SR.gstPc = soln_rec.gst_pc;
	strcpy (soln_rec.pack_size,inmr_rec.pack_size);

	DSP_FLD ("item_no");
	DSP_FLD ("descr");
	DSP_FLD ("pack_size");

	if (!NO_COST)
	{
		SR.costPrice = 0.00;
		soln_rec.cost_price = 0.00;
		DSP_FLD ("cost_price");
	}

	DSP_FLD ("sale_price");

	SR.hhwhHash = -1L;
	FLD ("qty_ord") = YES;

	realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 
									   ccmr_rec.hhcc_hash, 
									   0);
	if (envVarSoFwdAvl)
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
	if (envVarQcApply && envVarSkQcAvl)
		SR.qtyAvailable -= incc_rec.qc_qty;

	/*---------------
	| Process soic. |
	---------------*/
	ProcSoic (FALSE, line_cnt);

	if (itemChanged)
	{
		local_rec.qty_ord = 0.00;
		local_rec.qty_back = 0.00;
		SR.qtyOrder = local_rec.qty_ord;
		DSP_FLD ("qty_ord");
		DSP_FLD ("qty_back");
		PriceProcess ();
		DiscProcess ();
		CalculateTotal ();
	}

	if (itemChanged && getFields)
	{
		do
		{
			get_entry (label ("qty_ord"));
			cc = spec_valid (label ("qty_ord"));
		} while (cc && !restart);
		DSP_FLD ("qty_ord");

		/*----------------------------------------
		| Update the real-time committal record. |
		----------------------------------------*/
		UpdSoicQty ();
	}

	if (NON_STOCK)
		skip_entry = goto_field (label ("item_no"),label ("due_date"));
	else
	{
		if (BONUS)
		{
			skip_entry = 2;
			DSP_FLD ("bonus");
		}
		/*else
		{
			skip_entry = 1;

		}*/
	}

	sptr = clip (inmr_rec.description);
	if (strlen (sptr) == 0)
		skip_entry = 1;
	
	/*------------------------------------------------
	| Store last item (truncate BONUS if it exists). |
	------------------------------------------------*/
	sprintf (lastItem, "%-16.16s", local_rec.item_no);
	sptr = clip (lastItem);
	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVarSoSpecial [0]  && * (sptr + 1) == envVarSoSpecial [1])
		{
			*sptr = '\0';
			sprintf (lastItem, "%-16.16s", lastItem);
		}
	}

	/*-------------------------------
	| Check for complimentary items |
	-------------------------------*/
	if (BONUS)
		CheckForComp ();


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
		soln_rec.sale_price 	= 0.00;
		SR.actualSale 			= 0.00;
		SR.calcSalePrice			= 0.00;
		SR.salePrice 			= 0.00;
		DSP_FLD ("sale_price");

		soln_rec.dis_pc  		= 0.00;
		SR.discPc 	 			= 0.00;
		SR.calcDisc 			= 0.00;
		DSP_FLD ("disc");
		return;
	}
	pType = atoi (cumr_rec.price_type);
	grossPrice = GetCusPrice (comm_rec.co_no,
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
								  ccmr_rec.hhcc_hash,
								  SR.hhbrHash,
								  SR.category,
								  cnch_rec.hhch_hash,
							  	 (envVarSoDoi) ? TodaysDate () : comm_rec.dbt_date,
								  ToStdUom (local_rec.qty_ord + local_rec.qty_back),
								  1.00,
								  FGN_CURR,
								  &regPc);


	SR.pricingCheck	= TRUE;

	SR.calcSalePrice = GetCusGprice (grossPrice, regPc) * SR.cnvFct;

	if (SR.priceOveride [0] == 'N')
	{
		SR.grossSalePrice     = grossPrice * SR.cnvFct;
		SR.regPc          = regPc;
		SR.salePrice      = SR.calcSalePrice;
		soln_rec.sale_price = SR.calcSalePrice;
		SR.actualSale 		= SR.calcSalePrice;
	}

	SR.contractPrice 	= (_CON_PRICE) ? TRUE : FALSE;
	SR.contractStatus  	= _cont_status;

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
	if (SR.contractStatus == 2 || SR.contractPrice || SR.indentItem)
	{
		soln_rec.dis_pc  	= 0.00;
		SR.discPc 	 		= 0.00;
		SR.discA 			= 0.00;
		SR.discB 			= 0.00;
		SR.discC 			= 0.00;
		SR.calcDisc 		= 0.00;
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
								SR.salePrice,
								SR.regPc,
								ToStdUom (local_rec.qty_ord + local_rec.qty_back),
								discArray);

	if (SR.discOveride [0] == 'Y')
	{
		DSP_FLD ("disc");
		return;
	}
							
	SR.discPc			=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);
	soln_rec.dis_pc 	=	ScreenDisc (SR.discPc);

	SR.discA 			= discArray [0];
	SR.discB 			= discArray [1];
	SR.discC 			= discArray [2];
	SR.cumulative 		= cumDisc;

	if (SR.defaultDisc > ScreenDisc (soln_rec.dis_pc) && SR.defaultDisc != 0.0)
	{
		soln_rec.dis_pc = ScreenDisc (SR.defaultDisc);
		SR.discA 		= SR.defaultDisc;
		SR.discB 		= 0.00;
		SR.discC 		= 0.00;
	}

	SR.discPc    = ScreenDisc (soln_rec.dis_pc);
	SR.calcDisc =	SR.discPc;

	DSP_FLD ("disc");
}

/*===============================
| Validate Supercession Number. |
===============================*/
int
FindSuper (
 char*  sitem_no)
{
	/*---------------------------------------
	| At the end of the supercession chain	|
	---------------------------------------*/
	if (!strcmp (sitem_no,sixteen_space))
		return (EXIT_SUCCESS);

	/*-------------------------------
	| Find the superceeding item	|
	-------------------------------*/
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,sitem_no);
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (!cc)
		return (FindSuper (inmr_rec.supercession));
	/*---------------------------------------
	| Couldn't find the superceeding item	|
	---------------------------------------*/
	sprintf (err_str, ML (mlTsMess071) ,local_rec.item_no,sitem_no,sitem_no);
	print_mess (err_str);
	sleep (sleepTime);
	return (cc);
}

/*-------------------------------
| Check for complimentary items |
-------------------------------*/
void
CheckForComp (void)
{
	int	i, tmp_line;
	int	first_item;

	if (list_made != FALSE)
        return; /* FALSE */

	open_rec (tsci,  tsci_list, TSCI_NO_FIELDS, "tsci_id_no");

	tsci_rec.hhbr_hash = inmr_rec.hhbr_hash;
	tsci_rec.line_no = 0;
	cc = find_rec (tsci, &tsci_rec, GTEQ, "r");
	if (cc || (!cc && tsci_rec.hhbr_hash != inmr_rec.hhbr_hash))
	{
		abc_fclose (tsci);
        return; /* FALSE */
	}

/*	sprintf (err_str, "\007Product has complimentary products, examine ? ");
*/
	sprintf (err_str, ML (mlTsMess072));

	crsr_on ();
	i = prmptmsg (err_str, "YyNn", 2, 5);
	rv_pr ("                                                     ", 2, 5, 0);
	crsr_off ();
	if (i == 'N' || i == 'n')
	{
		abc_fclose (tsci);
		return; /* FALSE */
	}

	checking_comp = TRUE;
	putval (line_cnt);
	tmp_line = line_cnt;

	no_items = 0;
	tab_open ("item_lst", item_keys, 7, 1, 8, FALSE);
	tab_add ("item_lst",
		"# %-16.16s  %-40.40s  %-10.10s  %-40.40s ",
		" Item Number", " Item Description", "Order Qty", " Comments ");

	first_item = TRUE;
	while (!cc && tsci_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (first_item)
		{
			inmr2_rec.hhbr_hash = tsci_rec.hhbr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (cc)
			{
				abc_fclose (tsci);
				return; /* (FALSE); */
			}
			tab_add ("item_lst",
				"*%-16.16s  %-40.40s  %-10.10s  %-40.40s%-15.15s         %08ld  ",
				inmr2_rec.item_no, inmr2_rec.description,
				"   0.00", " ", " ", inmr2_rec.hhbr_hash);
			no_items++;
		}
		inmr2_rec.hhbr_hash = tsci_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (tsci, &tsci_rec, NEXT, "r");
			continue;
		}

		tab_add ("item_lst",
			" %-16.16s  %-40.40s  %-10.10s  %-40.40s%-15.15s         %08ld  ",
			inmr2_rec.item_no, inmr2_rec.description,
			"   0.00", tsci_rec.comment,
			" ", inmr2_rec.hhbr_hash);
		no_items++;

		first_item = FALSE;
		cc = find_rec (tsci, &tsci_rec, NEXT, "r");
	}
	abc_fclose (tsci);
	ScanItemTab ();

	scn_set (6);
	scn_write (6);
	line_cnt = tmp_line;
	lcount [6] = (prog_status == ENTRY) ? line_cnt : lcount [6];
	scn_display (6);

	if (selhd_ptr == SEL_NULL)
	{
		getval (line_cnt);
		line_display ();
	}
	else
	{
		list_made = COMP;
		ProcList ();
		list_made = FALSE;
		skip_entry = goto_field (cur_field, label ("item_no"));
	}
}

/*===================
| Assign mini menu. |
===================*/
void
TsCallMenu (void)
{
	crsr_off ();
	for (;;)
	{
	    	mmenu_print ("  TELE-SALES  CALLS  ", call_menu, 0);
	    	switch (mmenu_select (call_menu))
	    	{
		case PLANNED :
			call_type = PLANNED;
			GetNextPhoneDate ();
			ChampainCall ();
			return;

		case MAILER :
			call_type = MAILER;
			local_rec.last_date = 999999L;
			ChampainCall ();
			return;

		case RANDOM :
			call_type = RANDOM;
			RandomCall ();
			return;

		case RECEIVE :
			call_type = RECEIVE;
			RandomCall ();
			return;

		case RESTRT :
			restart = TRUE;
			return;

		case LEAVEOK :
		case PG_EXIT :
			prog_exit = TRUE;
			return;

		default :
			break;
	    	}
	}
}

/*----------------------------------
| Allow user to change upper bound |
| for range of calls to make.      |
----------------------------------*/
int
GetNextPhoneDate (void)
{
	clear_ok = FALSE;

	heading (11);
	local_rec.last_date = local_rec.lsystemDate;
	scn_display (11);
	edit (11);
	if (restart)
		restart = FALSE;

	clear_ok = TRUE;

	return (EXIT_SUCCESS);
}

/*------------------------------
| Process Calls For A Campaign |
------------------------------*/
int
ChampainCall (void)
{
	int	lcl_scn;

	open_rec (tsal,  tsal_list, TSAL_NO_FIELDS, "tsal_id_no");
	open_rec (tsal2, tsal_list, TSAL_NO_FIELDS, "tsal_id_no2");

	if (call_type == MAILER)
	{
		open_rec (tslh,  tslh_list, TSLH_NO_FIELDS, "tslh_id_no");

		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		init_vars (2);
		clear_ok = TRUE;
		heading (2);
		entry (2);
		abc_fclose (tslh);
		if (restart || prog_exit)
		{
			prog_exit = FALSE;
			return (FALSE);
		}
		lcl_scn = 2;
	}
	else
		lcl_scn = 0;

	/*---------------------------
	| Read Lead Allocation File |
	---------------------------*/
	if (!LoadList ())
		return (FALSE);

	more_custs = TRUE;
	while (more_custs)
	{
		heading (lcl_scn);
		tab_open (customerList, cust_keys, 2, 0, 14, FALSE);
		tab_add (customerList,
			"# %-6.6s|%-40.40s|%-7.7s| %-7.7s  |%-4.4s| %-15.15s | %-10.10s | %-10.10s |%-5.5s|%-6.6s",
			"CUST. "    , "          CUSTOMER NAME",
			"BEST PH"   , "NEXT PH",
			"FREQ"      , "PHONE NUMBER",
			" A/C BAL." , "CRD LIMIT",
			"TERMS"     , "EXCEED");

		/*--------------------------------
		| Load tab_disp table with leads |
		--------------------------------*/
		curr_ptr = Header_ptr;
		while (curr_ptr != CUST_NULL)
		{
			/*-----------------------------------------
			| NB hhcu_hash is stored at 141st char in |
			| string so it can be used but not seen   |
			| Remember this if you change the length  |
			| of any of these fields                  |
			-----------------------------------------*/
			tab_add (customerList,
					 " %-6.6s|%-40.40s| %-5.5s |%-10.10s| %2d | %-15.15s |%11.2f |%11.2f | %-3.3s | %-3.3s          %08ld ",

				curr_ptr->dbt_no,
				curr_ptr->dbt_name,
				curr_ptr->bst_ph_time,
				DateToString (curr_ptr->nxt_ph_date),
				curr_ptr->ph_freq,
				curr_ptr->phone_no,
				curr_ptr->acc_bal,
				curr_ptr->crd_lmt,
				curr_ptr->terms,
				 (curr_ptr->exceed [0] == 'Y') ? "Yes" : "No ",
				curr_ptr->hhcuHash);

			curr_ptr = curr_ptr->next;
		}

		if (no_custs == 0)
		{
			tab_add (customerList,
				"                       ***    %-73.73s    *** ",
				mlTsCall  [1]);
			putchar (BELL);
			tab_display (customerList, TRUE);
			sleep (sleepTime);
			tab_close (customerList, TRUE);

			more_custs = FALSE;
		}
		else
		{
			tab_scan (customerList);
			tab_close (customerList, TRUE);

			if (more_custs)
				Deallocate ();
		}
	}

	abc_fclose (tsal);
	abc_fclose (tsal2);

	return (EXIT_SUCCESS);
}

/*--------------------------
| Select a cutomer to call |
--------------------------*/
static int
select_func (
 int        c,
 KEY_TAB*   psUnused)
{
	tab_get (customerList, get_buf, CURRENT, 0);
	currHhcuHash = atol (get_buf + 140);

	if (ProcessCall ())
		return (FN16);
	tab_display (customerList, TRUE);
	return (c);
}

static int
exit_func (
 int        c,
 KEY_TAB*   psUnused)
{
	more_custs = FALSE;
	return (FN16);
}


/*-----------------------------
| Read Lead Allocation File   |
| into an ordered linked list |
-----------------------------*/
int
LoadList (void)
{
	struct	CUST_LIST	*lcl_ptr;

	/*---------------------
	| Clear existing list |
	---------------------*/
	if (Header_ptr != CUST_NULL)
	{
		curr_ptr = Header_ptr;
		while (curr_ptr != CUST_NULL)
		{
			lcl_ptr = curr_ptr;
			curr_ptr = curr_ptr->next;
			free (lcl_ptr);
		}
		Header_ptr = CUST_NULL;
	}
	no_custs = 0;

	tsal_rec.hhop_hash = tmop_rec.hhop_hash;
	tsal_rec.line_no = 0;
	cc = find_rec (tsal, &tsal_rec, GTEQ, "r");
	while (!cc && tsal_rec.hhop_hash == tmop_rec.hhop_hash)
	{
		cumr_rec.hhcu_hash = tsal_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");

		tspm_rec.hhcu_hash = tsal_rec.hhcu_hash;
		cc1 = find_rec (tspm, &tspm_rec, COMPARISON, "r");
		if (cc || cc1)
		{
			cc = find_rec (tsal, &tsal_rec, NEXT, "r");
			continue;
		}
		if ((tspm_rec.n_phone_date > local_rec.last_date) ||
		    (call_type == PLANNED && tsal_rec.hhlh_hash != 0L) ||
		    (call_type == MAILER && tsal_rec.hhlh_hash != mail_hash))
		{
			cc = find_rec (tsal, &tsal_rec, NEXT, "r");
			continue;
		}

		no_custs++;

		/*-------------------------
		| Insert into linked list |
		-------------------------*/
		lcl_ptr = cust_alloc ();
		lcl_ptr->hhcuHash = cumr_rec.hhcu_hash;
		sprintf (lcl_ptr->dbt_no, "%-6.6s", cumr_rec.dbt_no);
		sprintf (lcl_ptr->dbt_name, "%-40.40s", cumr_rec.dbt_name);
		sprintf (lcl_ptr->bst_ph_time, "%-5.5s",tspm_rec.best_ph_time);
		lcl_ptr->nxt_ph_date = tspm_rec.n_phone_date;
		lcl_ptr->ph_freq = tspm_rec.phone_freq;
		sprintf (lcl_ptr->phone_no, "%-15.15s", cumr_rec.phone_no);
		lcl_ptr->acc_bal = DOLLARS (cumr_balance [0] + 
					   				cumr_balance [1] +
		      		   	   			cumr_balance [2] + 
					   				cumr_balance [3] +
		      		   	   			cumr_balance [4] + 
					   				cumr_balance [5]);

		lcl_ptr->crd_lmt = DOLLARS (cumr_rec.credit_limit);
		sprintf (lcl_ptr->terms, "%-3.3s", cumr_rec.crd_prd);
		strcpy (lcl_ptr->exceed,
			 (CreditExceeded (lcl_ptr->acc_bal)) ? "Y" : "N");

		if (Header_ptr == CUST_NULL)
		{
			Header_ptr = lcl_ptr;
			curr_ptr = lcl_ptr;
			lcl_ptr->prev = CUST_NULL;
			lcl_ptr->next = CUST_NULL;
		}
		else
			FindPos (lcl_ptr);
		curr_ptr = lcl_ptr;

		cc = find_rec (tsal, &tsal_rec, NEXT, "r");
	}

	return (TRUE);
}

/*--------------------------
| Find correct position in |
| linked list and insert   |
--------------------------*/
int
FindPos (
 struct CUST_LIST *     lcl_ptr)
{
	struct CUST_LIST *tmp_ptr;

	if (strcmp (lcl_ptr->bst_ph_time, curr_ptr->bst_ph_time) > 0)
		tmp_ptr = curr_ptr;
	else
		tmp_ptr = Header_ptr;
	while (tmp_ptr != CUST_NULL)
	{
		if (strcmp (lcl_ptr->bst_ph_time, tmp_ptr->bst_ph_time) < 0 ||
		    (!strcmp (lcl_ptr->bst_ph_time, tmp_ptr->bst_ph_time) &&
		     lcl_ptr->nxt_ph_date < tmp_ptr->nxt_ph_date))
		{
			if (tmp_ptr == Header_ptr)
			{
				lcl_ptr->next = tmp_ptr;
				Header_ptr->prev = lcl_ptr;
				lcl_ptr->prev = CUST_NULL;
				Header_ptr = lcl_ptr;
			}
			else
			{
				lcl_ptr->next = tmp_ptr;
				tmp_ptr->prev->next = lcl_ptr;
				lcl_ptr->prev = tmp_ptr->prev;
				tmp_ptr->prev = lcl_ptr;
			}
			return (EXIT_SUCCESS);
		}

		/*------------------------
		| Append to tail of list |
		------------------------*/
		if (tmp_ptr->next == CUST_NULL)
		{
			lcl_ptr->next = CUST_NULL;
			lcl_ptr->prev = tmp_ptr;
			tmp_ptr->next = lcl_ptr;
			return (EXIT_SUCCESS);
		}
		tmp_ptr = tmp_ptr->next;
	}

	return (EXIT_SUCCESS);
}

/*---------------------------
| Check if customers credit |
| Limit is exceed           |
---------------------------*/
int
CreditExceeded (
 double     tot_owing)
{
	/*---------------------------------------------
	| Check if customer is over his credit limit. |
	---------------------------------------------*/
	if (cumr_rec.credit_limit <= tot_owing &&
	     cumr_rec.credit_limit != 0.00 &&
	     cumr_rec.crd_flag [0] != 'Y')
	{
		return (TRUE);
	}

	/*-----------------------
	| Check Credit Terms	|
	-----------------------*/
	if (cumr_rec.od_flag && cumr_rec.crd_flag [0] != 'Y')
		return (TRUE);

	return (FALSE);
}

/*--------------------------------
| Update tspm record.            |
| Deallocate cust from operator. |
| Remove from linked list        |
--------------------------------*/
int
Deallocate (void)
{
	struct	CUST_LIST *tmp_ptr;

	tspm_rec.hhcu_hash	=	currHhcuHash;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, tspm, "DBFIND");

	sprintf (tspm_rec.lst_op_code, "%-14.14s", tspm_rec.op_code);
	sprintf (tspm_rec.op_code, "%-14.14s", " ");
	tspm_rec.lst_ord = DOLLARS (tot_tot);

	if (call_type == PLANNED)
		tspm_rec.n_phone_date += (long)tspm_rec.phone_freq;

	if (upd_visit)
	{
		tspm_rec.n_visit_date = local_rec.n_visit_date;
		strcpy (tspm_rec.n_visit_time, local_rec.n_visit_time);
	}

	if (upd_phone)
	{
		tspm_rec.n_phone_date = local_rec.n_phone_date;
		strcpy (tspm_rec.n_phone_time, local_rec.n_phone_time);
	}

	tspm_rec.lphone_date = local_rec.lsystemDate;

	cc = abc_update (tspm, &tspm_rec);
	if (cc)
		file_err (cc, tspm, "DBUPDATE");

	/*----------------------------
	| Update Letter Sent Record. |
	----------------------------*/
	if (call_type == MAILER)
		UpdateTsls ();

	/*--------------------
	| Remove tsal record |
	--------------------*/
	tsal_rec.hhop_hash = tmop_rec.hhop_hash;
	tsal_rec.hhcu_hash = currHhcuHash;
	cc = find_rec (tsal2, &tsal_rec, COMPARISON, "u");
	if (!cc)
	{
		cc = abc_delete (tsal2);
		if (cc)
			file_err (cc, tsal2, "DBDELETE");
	}

	if (call_type != PLANNED && call_type != MAILER)
		return (EXIT_SUCCESS);

	/*-----------------------------
	| Remove entry in linked list |
	-----------------------------*/
	tmp_ptr = Header_ptr;
	while (tmp_ptr != CUST_NULL)
	{
		if (tmp_ptr->hhcuHash == currHhcuHash)
		{
			if (tmp_ptr == Header_ptr)
			{
				Header_ptr = tmp_ptr->next;
				if (tmp_ptr->next != CUST_NULL)
					tmp_ptr->next->prev = CUST_NULL;
				free (tmp_ptr);
				break;
			}

			if (tmp_ptr->next != CUST_NULL)
				tmp_ptr->next->prev = tmp_ptr->prev;

			if (tmp_ptr->prev != CUST_NULL)
				tmp_ptr->prev->next = tmp_ptr->next;

			free (tmp_ptr);

			break;
		}

		tmp_ptr = tmp_ptr->next;
	}
	no_custs--;

	if (no_custs <= 0)
		more_custs = FALSE;

	return (EXIT_SUCCESS);
}

/*----------------------------
| Update Letter Sent Record. |
----------------------------*/
int
UpdateTsls (void)
{
	open_rec (tsls,  tsls_list, TSLS_NO_FIELDS, "tsls_id_no");

	tsls_rec.hhlh_hash = tslh_rec.hhlh_hash;
	tsls_rec.hhcu_hash = currHhcuHash;
	tsls_rec.date_sent = 0L;
	cc = find_rec (tsls, &tsls_rec, GTEQ, "u");
	while (!cc &&
	       tsls_rec.hhlh_hash == tslh_rec.hhlh_hash &&
	       tsls_rec.hhcu_hash == currHhcuHash)
	{
		if (tsls_rec.date_called == 0L)
		{
			tsls_rec.date_called = TodaysDate ();
			strcpy (tsls_rec.time_called, TimeHHMM ());
			cc = abc_update (tsls, &tsls_rec);
			if (cc)
				file_err (cc, tsls, "DBUPDATE");
			break;
		}
		else
			abc_unlock (tsls);

		cc = find_rec (tsls, &tsls_rec, NEXT, "u");
	}
	abc_fclose (tsls);

	return (EXIT_SUCCESS);
}

/*---------------------------------
| Process A Call To A Random Lead |
---------------------------------*/
int
RandomCall (void)
{
	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	search_ok = TRUE;

	open_rec (tsal2,  tsal_list, TSAL_NO_FIELDS, "tsal_id_no");

	init_vars (1);
	clear_ok = TRUE;
	heading (1);
	entry (1);
	if (restart || prog_exit)
	{
		prog_exit = FALSE;
		return (FALSE);
	}

	if (ProcessCall ())
		Deallocate ();

	abc_fclose (tsal2);
	return (EXIT_SUCCESS);
}

/*----------------------
| Lookup operator file |
----------------------*/
int
GetOperator (void)
{
	open_rec (tmop,  tmop_list, TMOP_NO_FIELDS, "tmop_id_no");

	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", upper_op);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	abc_fclose (tmop);
	if (cc)
		return (FALSE);

	return (TRUE);
}

/*-----------------------------------------------------------
| Take Order From Customer                                    |
| Return FALSE if FN1 pressed  and debtor not reached       |
| Return TRUE if FN16 pressed  and call proceeded as normal |
-----------------------------------------------------------*/
int
ProcessCall (void)
{
	exit_call = FALSE;

	if (call_type == PLANNED || call_type == MAILER)
	{
		cumr_rec.hhcu_hash = currHhcuHash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");

		tspm_rec.hhcu_hash = currHhcuHash;
		cc1 = find_rec (tspm, &tspm_rec, COMPARISON, "r");
		if (cc || cc1)
		{
			print_mess (ML (mlTsMess082));
			sleep (sleepTime);
			clear_mess ();
			return (TRUE);
		}
		GetHoCustomer ();
		if (CheckContract ())
			ContractDetails ();
		else
		{
#ifdef GVISION
			scn_hide (6);
#endif	/* GVISION */
		}
	}
	upd_visit = FALSE;
	upd_phone = FALSE;
	local_rec.n_visit_date = tspm_rec.n_visit_date;
	sprintf (local_rec.n_visit_time, "%-5.5s", tspm_rec.n_visit_time);
	local_rec.n_phone_date = tspm_rec.n_phone_date;
	sprintf (local_rec.n_phone_time, "%-5.5s", tspm_rec.n_phone_time);

	/*------------------------
	| Edit last call details |
	------------------------*/
	first_notes = TRUE;
	RingCallNotes ();
	if (restart)
	{
		restart = FALSE;
		return (FALSE);
	}

	SetOrder ();

	sprintf (lastItem, "%-16.16s", " ");
	_win_func = TRUE;
	/*---------------------
	| Order entry section |
	---------------------*/
	while (!exit_call)
	{
		in_edit_all = FALSE;
		notax = 0;
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		tab_row = 7;
		tab_col = 0;
		lcount [6] = 0;
		dis_tot = 0.00;
		inv_tot = 0.00;
		tax_tot = 0.00;
		tot_tot = 0.00;
		gst_tot = 0.00;

		/*--------------------
		| Clear store array. |
		--------------------*/
		ClearStore ();

		/*-------------
		| Enter items |
		-------------*/
		line_cnt = -1;
		clear_ok = FALSE;
		init_vars (6);
		heading (6);
		entry (6);
		if (restart)
		{
			abc_unlock (sohr);
			heading (0);
			restart = FALSE;
			_win_func = FALSE;

			/*---------------------
			| Clear soic records. |
			---------------------*/
			ClearSoic ();

			return (FALSE);
		}

		prog_status = ! (ENTRY);
		in_edit_all = TRUE;

		no_edit (1);
		no_edit (2);
		no_edit (3);
		no_edit (4);
		no_edit (5);
		no_edit (8);
		no_edit (9);
		no_edit (10);
		no_edit (11);
		no_edit (12);
		edit_all ();
		if (restart)
		{
			abc_unlock (sohr);
			heading (0);
			restart = FALSE;
			_win_func = FALSE;

			/*---------------------
			| Clear soic records. |
			---------------------*/
			ClearSoic ();

			return (FALSE);
		}

		scn_set (6);
		scn_write (6);
		scn_display (6);
		CheckTabLines ();

		Update ();
		prog_exit = FALSE;
		exit_call = TRUE;
	}
	_win_func = FALSE;
	return (TRUE);
}

/*---------------------------------------------
| Process each lines for B/O etc if necessary |
---------------------------------------------*/
void
CheckTabLines (void)
{
	int	i, this_page;
	char	tmp_item [17];

	this_page = 0;
	for (line_cnt = 0; line_cnt < lcount [6]; line_cnt++)
	{
		if (envVarSoFullSupply && sohr_rec.full_supply [0] == 'Y')
			break;

		getval (line_cnt);
		if (local_rec.qty_chk [0] != 'Y')
			continue;

		if (this_page != (line_cnt / TABLINES))
		{
			scn_write (6);
			scn_display (6);
			this_page = line_cnt / TABLINES;
		}

		/*---------------------------------
		| Display current item in reverse |
		---------------------------------*/
		sprintf (tmp_item, "%-16.16s", local_rec.item_no);
		rv_pr (tmp_item, 
			tab_col + 1, 
			 (line_cnt % TABLINES) + tab_row + 2, 1);

		sprintf (err_str,
			ML (mlTsMess716),
			SR.qtyAvailable, clip (local_rec.item_no));

		cc = WarnUser (err_str,1);

		if (InputRes (line_cnt) &&
		     sohr_rec.full_supply [0] == 'N' &&
		     envVarSoFullSupply)
		{
			sprintf (err_str, ML (mlTsMess083) , cumr_rec.dbt_no);

			i = prmptmsg (err_str,"YyNn",2,5);
			if (i == 'Y' || i == 'y')
				strcpy (sohr_rec.full_supply, "Y");
			else
				strcpy (sohr_rec.full_supply, "A");
		}

		/*--------------------------------
		| Display current item in normal |
		--------------------------------*/
		rv_pr (tmp_item, tab_col + 1, (line_cnt % TABLINES) + tab_row + 2, 0);
	}
}

/*==============================
| update all relevent records. |
==============================*/
void
Update (void)
{
	int		new_soln;
	long	order_no;
	int		all_forward = 0;
	int		all_border  = 0;
	int		force_update = FALSE;
	int		bord_all = TRUE;	/* Set as if all lines b/ordered. */
	char	last_status [2];
	int		i;

	over_margin = FALSE;

	clear ();
	scn_set (1);

	if (sohr_rec.tax_code [0] == 'A' || sohr_rec.tax_code [0] == 'B')
		notax = TRUE;
	else
		notax = FALSE;

	strcpy (last_status, createFlag);

	strcpy (sohr_rec.din_1,local_rec.spinst [0]);
	strcpy (sohr_rec.din_2,local_rec.spinst [1]);
	strcpy (sohr_rec.din_3,local_rec.spinst [2]);
	sprintf (sohr_rec.frei_req,"%-1.1s",local_rec.frei_req);
	sprintf (sohr_rec.op_id, "%-14.14s", currentUser);
	sohr_rec.date_create = TodaysDate ();
	strcpy (sohr_rec.time_create, TimeHHMM ());

	if (lcount [6] == 0)
	{
		for (i = 0; i < 2; i++)
		{
			rv_pr (ML (mlTsMess031) , 0,0,i % 2);

			sleep (sleepTime);
		}
		move (0,0);
		cl_line ();
		return;
	}

	strcpy (sohr_rec.batch_no,"00000");

	if (!strcmp (sohr_rec.order_no,"00000000") ||
	     !strcmp (sohr_rec.order_no,"        "))
	{
		open_rec (sohr2,sohr_list,SOHR_NO_FIELDS,"sohr_id_no2");
		open_rec (esmr,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");

		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,comm_rec.est_no);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		/*---------------------------------------
		| Check if Order No Already Allocated	|
		| If it has been then skip		|
		---------------------------------------*/
		while (CheckSohr (++esmr_rec.nx_order_no) == 0);

		abc_fclose (sohr2);

		cc = abc_update (esmr,&esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");

		sprintf (sohr_rec.order_no,"%08ld",esmr_rec.nx_order_no);
		abc_fclose (esmr);
	}
	print_at (0,0, ML (mlTsMess092) ,sohr_rec.order_no);

	if (!held_order)
	{
		if (CheckCumr (OrderValue ()))
		{
			print_at (0,0, ML (mlTsMess076) ,sohr_rec.order_no);

			held_order = TRUE;
		}
		if (!held_order && over_margin)
		{
			print_at (0,0, ML (mlTsMess077) ,sohr_rec.order_no);

		}
	}

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	strcpy (sohr_rec.dp_no, cudp_rec.dp_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

	strcpy (sohr_rec.inv_no,"        ");
	strcpy (sohr_rec.ord_type, "D");

	strcpy (sohr_rec.stat_flag, createFlag);
	strcpy (sohr_rec.status, createFlag);

	/*---------------------
	| Process Held first. |
	---------------------*/
	if (over_margin)
		strcpy (sohr_rec.status,"O");

	if (held_order)
		strcpy (sohr_rec.status,"H");

	/*--------------------------------------
	| Forward Order	Overwrites all others. |
	--------------------------------------*/
	if (sohr_rec.dt_required > local_rec.lsystemDate)
		strcpy (sohr_rec.status,"F");

	cc = abc_add (sohr,&sohr_rec);
	if (cc)
		file_err (cc, "sohr", "DBADD");

	cc = find_rec (sohr,&sohr_rec,COMPARISON,"u");
	if (cc)
		file_err (cc, "sohr", "DBFIND");

	abc_unlock (esmr);

	print_at (0,0, ML (mlTsMess021));

	order_no = atol (sohr_rec.order_no);

	abc_selfield (inmr,"inmr_hhbr_hash");
	fflush (stdout);
	scn_set (6);

	if (sohr_rec.full_supply [0] == 'Y')
	{
		/*-----------------------------------
		| Check if all lines are backorded. |
		-----------------------------------*/
		for (line_cnt = 0;line_cnt < lcount [6];line_cnt++)
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
	/*-----------------------------------
	| Check if all lines are backorded. |
	-----------------------------------*/
	for (line_cnt = 0;line_cnt < lcount [6];line_cnt++)
	{
		getval (line_cnt);
		if (local_rec.qty_ord != 0.00 ||
	             soln_rec.due_date > local_rec.lsystemDate)
	    		bord_all = FALSE;
	}

	for (line_cnt = 0;line_cnt < lcount [6];line_cnt++)
	{
		getval (line_cnt);

		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = line_cnt;
		new_soln = find_rec (soln,&soln_rec,COMPARISON,"u");

		getval (line_cnt);

		strcpy (soln_rec.pri_or,SR.priceOveride);
		strcpy (soln_rec.dis_or,SR.discOveride);

		inmr_rec.hhbr_hash	= SR.hhbrHash;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc)
			 file_err (cc, inmr, "DBFIND");

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;

		if (inmr_rec.hhbr_hash == soln_rec.hhbr_hash)
			if (incc_rec.hhcc_hash != soln_rec.hhcc_hash)
				incc_rec.hhcc_hash = soln_rec.hhcc_hash;

		abc_selfield (inum, "inum_hhum_hash");

		inum_rec.hhum_hash = SR.hhumHash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = line_cnt;

		soln_rec.hhbr_hash   = inmr_rec.hhbr_hash;
		soln_rec.hhcc_hash   = incc_rec.hhcc_hash;
		soln_rec.hhum_hash 	 = inum_rec.hhum_hash;
    	soln_rec.qty_order   = ToStdUom (local_rec.qty_ord);
    	soln_rec.qty_bord    = ToStdUom (local_rec.qty_back);
		soln_rec.cost_price  = SR.costPrice;
		soln_rec.reg_pc      = SR.regPc;
		soln_rec.disc_a      = SR.discA;
		soln_rec.disc_b      = SR.discB;
		soln_rec.disc_c      = SR.discC;
		soln_rec.cumulative  = SR.cumulative;
		if (SR.cnvFct == 0.00)
			SR.cnvFct = 1.00;
		soln_rec.sale_price = SR.salePrice / SR.cnvFct;
		soln_rec.gsale_price = SR.grossSalePrice / SR.cnvFct;
		soln_rec.cont_status = SR.contractStatus;

		soln_rec.gst_pc  = (float) ((notax) ? 0.00 : inmr_rec.gst_pc);

		soln_rec.tax_pc  = (float) ((notax) ? 0.00 : inmr_rec.tax_pc);

		soln_rec.o_xrate = SR.exchRate;
		soln_rec.n_xrate = 0.00;

		soln_rec.cont_status = SR.contractStatus;

		if (SR.bonusFlag [0] == 'Y')
			strcpy (soln_rec.bonus_flag,"Y");
		else
			strcpy (soln_rec.bonus_flag,"N");

		if (new_soln)
		{
			strcpy (soln_rec.stat_flag, createFlag);
			strcpy (soln_rec.status,    createFlag);

			/*---------------------
			| Process Held first. |
			---------------------*/
			if (over_margin)
				strcpy (soln_rec.status,"O");

			if (held_order)
				strcpy (soln_rec.status,"H");

		}
		/*---------------------------
		| Overide if Forward Order. |
		---------------------------*/
		if (soln_rec.due_date > local_rec.lsystemDate)
			strcpy (soln_rec.status,"F");

		/*---------------------------------------
		| forward so all on order		|
		---------------------------------------*/
		if (soln_rec.status [0] == 'F')
		{
			soln_rec.qty_order += soln_rec.qty_bord;
			soln_rec.qty_bord = 0.00;
		}
		if (NON_STOCK)
			strcpy (soln_rec.status,last_status);

		if (!new_soln)
		{
			putchar ('U');
			fflush (stdout);

			if (NON_STOCK)
				strcpy (soln_rec.status,last_status);

			/*------------------------
			| Update existing order. |
			------------------------*/
			soln_rec.dis_pc  = ScreenDisc (soln_rec.dis_pc);
			cc = abc_update (soln,&soln_rec);
			if (cc)
				file_err (cc, "soln", "DBUPDATE");

			/*-------------------------------------------
			| Add sobg record for Sales Order Analysis. |
			-------------------------------------------*/
			if (envVarSoSales && (ONE_STEP || TWO_STEP))
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

			abc_unlock (soln);
		}
		else
		{
			putchar ('A');
			fflush (stdout);

			/*--------------------------------------------------
			| If all of line placed on b/o then create line as |
			| as a b/o but only if bord_all is set to true.    |
			--------------------------------------------------*/
	    		if (soln_rec.qty_order == 0.00 && bord_all)
			{
				strcpy (soln_rec.stat_flag, "B");
				strcpy (soln_rec.status, "B");
			}
			if (NON_STOCK)
				strcpy (soln_rec.status,last_status);

			soln_rec.dis_pc  = ScreenDisc (soln_rec.dis_pc);
			cc = abc_add (soln,&soln_rec);
			if (cc)
				file_err (cc, "soln", "DBADD");

			abc_unlock (soln);

			/*-------------------------------------------
			| Add sobg record for Sales Order Analysis. |
			-------------------------------------------*/
			if (envVarSoSales && (ONE_STEP || TWO_STEP))
			{
				soln_rec.hhso_hash = sohr_rec.hhso_hash;
				soln_rec.line_no = line_cnt;
				if (!find_rec (soln,&soln_rec,COMPARISON,"r"))
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
		}
		strcpy (last_status, soln_rec.status);

		if (soln_rec.status [0] == 'F')
			all_forward++;

		if (soln_rec.status [0] == 'B')
			all_border++;

		add_hash (comm_rec.co_no, 
				  comm_rec.est_no, 
				  "RC", 
				  0,
				  soln_rec.hhbr_hash, 
				  soln_rec.hhcc_hash,
				  progPid,
				  (double) 0.00);
	}
	abc_selfield (inmr,"inmr_id_no");

	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = line_cnt;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		putchar ('D');
		fflush (stdout);
		abc_unlock (soln);
		cc = abc_delete (soln);
		if (cc)
			file_err (cc, "soln", "DBDELETE");

		/*-------------------------------------------
		| Add sobg record for Sales Order Analysis. |
		-------------------------------------------*/
		if (envVarSoSales && (ONE_STEP || TWO_STEP))
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
		cc = find_rec (soln,&soln_rec,GTEQ,"u");
	}
	abc_unlock (soln);

	if (all_border == lcount [6] && lcount [6] != 0)
	{
	   	if (bord_all)
	   	{
			strcpy (sohr_rec.sohr_new,"N");
			strcpy (sohr_rec.stat_flag, "B");

			strcpy (sohr_rec.status, "B");

			force_update = TRUE;
			print_at (0,0, ML (mlTsMess022));
		}
	}
	if (all_forward == lcount [6] && lcount [6] != 0)
	{
		strcpy (sohr_rec.stat_flag, "F");
		strcpy (sohr_rec.status, "F");
		force_update = TRUE;
	}

	if (force_update)
	{
		/*------------------------------------
		| Just update stat flag and rewrite. |
		------------------------------------*/
		cc = abc_update (sohr,&sohr_rec);
		if (cc)
			file_err (cc, "sohr", "DBUPDATE");
	}
	abc_unlock (sohr);

	add_hash (comm_rec.co_no, 
			  comm_rec.est_no,
			  "RO", 
			  0, 
			  cumr_rec.hhcu_hash, 
			  0L, 
			  0L, 
			  (double) 0.00);

	if (lcount [6] != 0)
	{
		if (sohr_rec.status [0] == 'R')
		{
			add_hash (comm_rec.co_no, 
					  comm_rec.est_no,
					  (printerNumber) ? "PA" : "PC", 
					  printerNumber,
					  sohr_rec.hhso_hash, 
					  0L, 
					  0L, 
					  (double) 0.00);

			firstTime = 0;
		}
	}
	recalc_sobg ();

	local_rec.prevPackSlip = order_no;
	sprintf (local_rec.prevCustomerNo,"%-6.6s",cumr_rec.dbt_no);

    PauseForKey (0, 0, ML (mlStdMess042), 0);
	return;
}

/*=====================================================
| Check Value of Order will existing value of orders. |
=====================================================*/
double
OrderValue (void)
{
	double	order_val = 0.00;
	double	o_total = 0.00;
	double	o_disc = 0.00;
	int	i;

	for (i = 0;i < lcount [6];i++)
	{
		/*-----------------------------------------------
		| Update soln gross tax and disc for each line. |
		-----------------------------------------------*/
		o_total = (double) store [i].qtyOrder;
		o_total *= out_cost (store [i].actualSale,store [i].outerSize);

		o_disc = (double) (store [i].discPc / 100.00);
		o_disc *= o_total;

		order_val += o_total - o_disc;

		if (store [i].marginFailed [0] == 'Y')
			over_margin = TRUE;
	}
	return (order_val);
}

/*==========================================
| Validate credit period and credit limit. |
==========================================*/
int
CheckCumr (
 double     new_value)
{
	double	total_owing = 0.00;

	if (STANDARD || cumr_rec.crd_flag [0] == 'Y')
		return (EXIT_SUCCESS);

	total_owing = cumr_balance [0] + cumr_balance [1] +
	              cumr_balance [2] + cumr_balance [3] +
	  	      	  cumr_balance [4] + cumr_balance [5] +
		      	  cumr_rec.ord_value + new_value;

	/*---------------------------------------------
	| Check if customer is over his credit limit. |
	---------------------------------------------*/
	if (cumr_rec.credit_limit <= total_owing && cumr_rec.credit_limit != 0.00)
		return (EXIT_FAILURE);

	if (cumr_rec.od_flag)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

int
CheckSohr (
 long int   order_no)
{
	strcpy (sohr2_rec.co_no,comm_rec.co_no);
	strcpy (sohr2_rec.br_no,comm_rec.est_no);
	sprintf (sohr2_rec.order_no,"%08ld",order_no);
	cc = find_rec (sohr2,&sohr2_rec,COMPARISON,"r");
	return (cc);
}

/*===================
| Calculate totals. |
===================*/
void
CalculateTotal (void)
{
	int	i;
	int	no_lines = 0;
	double	other = 0.00;
	double	wk_gst = 0.00;

	no_lines = (prog_status == ENTRY && (lcount [6] - 1 < line_cnt)) ?
		     line_cnt : lcount [6] - 1;

	inv_tot = 0.00;
	dis_tot = 0.00;
	tax_tot = 0.00;
	gst_tot = 0.00;
	tot_tot = 0.00;

	for (i = 0; i <= no_lines; i++)
	{
		CalculateExtend (i);

		inv_tot += l_total;
		dis_tot += l_disc;
		tax_tot += l_tax;
		gst_tot += l_gst;
	}

	gst_tot = no_dec (gst_tot);

	other = sohr_rec.freight + 	sohr_rec.insurance +
			sohr_rec.other_cost_1 + sohr_rec.other_cost_2 +
			sohr_rec.other_cost_3;

	if (notax)
		wk_gst = 0.00;
	else
		wk_gst = (double) (comm_rec.gst_rate / 100.00);

	wk_gst *= other;

	gst_tot += wk_gst;
	gst_tot = no_dec (gst_tot);

	tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + other);

	PrintTotal ();

	local_rec.disc_over = 0.00;
}

/*============================
| Calculate extended Values. |
============================*/
void
CalculateExtend (
 int    line_no)
{
	/*-----------------------------------------------
	| Update soln gross tax and disc for each line. |
	-----------------------------------------------*/
	l_total = (double) store [line_no].qtyOrder;
	l_total *= out_cost (store [line_no].actualSale,store [line_no].outerSize);
	l_total = no_dec (l_total);

	if (notax)
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

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) store [line_no].taxPc;
		if (sohr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
			l_tax *= (l_total - l_disc);

		l_tax = DOLLARS (l_tax);
	}

	l_tax = no_dec (l_tax);

	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) store [line_no].gstPc;
		l_gst *= ((l_total - l_disc) + l_tax);
		l_gst = DOLLARS (l_gst);
	}
}

/*==============
| Draw totals. |
==============*/
void
DrawTotal (void)
{
	ClearBox (96,1,35,4);
	print_at (2,97, ML (mlTsMess023));

	print_at (3,97, ML (mlTsMess024));

	if (envVarGst)
		print_at (4,97, ML (mlTsMess025), envVarGstTaxName);
	else
		print_at (4,97, ML (mlTsMess026));

	print_at (5,97, ML (mlTsMess027));
}

void
PrintTotal (void)
{
	double	f_other = 0.00;

	f_other = sohr_rec.freight +	sohr_rec.insurance +
		  sohr_rec.other_cost_1 + sohr_rec.other_cost_2 +
		  sohr_rec.other_cost_3;

	tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot + f_other);

	print_at (2, 115, "%14.2f", DOLLARS (inv_tot - dis_tot));
	print_at (3, 115, "%14.2f", DOLLARS (f_other));
	print_at (4, 115, "%14.2f", DOLLARS (gst_tot + tax_tot));
	print_at (5, 115, "%14.2f", DOLLARS (tot_tot));
}

#ifndef	GVISION
int
OpenSkWin (void)
{
	np_fn = IP_CREATE (getpid ());
	if (np_fn < 0)
	{
		envVarWinOk = FALSE;
		return (EXIT_FAILURE);
	}
	if ((pout = popen ("so_pwindow","w")) == 0)
	{
		envVarWinOk = FALSE;
		return (EXIT_FAILURE);
	}
	wpipe_open = TRUE;
	fprintf (pout, "%06d\n", getpid ());
	return (EXIT_SUCCESS);
}
#endif	/* GVISION */

/*======================================================================
| Log lost sales from stock quantity on hand less-than input quantity. |
======================================================================*/
int
LogLostSales (
 float      lost_qty)
{
	int	i;
	char	shhbrHash [10];
	char	shhccHash [10];
	char	shhcuHash [10];
	char	wk_qty [11];
	char	wk_value [11];

	if (!envVarSoLostSales)
		return (FALSE);
	i = prmptmsg (ML (mlStdMess177) ,"YyNn",2,5);

	if (i == 'N' || i == 'n')
		return (EXIT_SUCCESS);
	sprintf (shhbrHash,"%09ld",alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash));

	sprintf (shhccHash,"%09ld",incc_rec.hhcc_hash);
	sprintf (shhcuHash,"%09ld",cumr_rec.hhcu_hash);
	sprintf (wk_qty,"%10.2f",lost_qty);
	sprintf (wk_value,"%10.2f",soln_rec.sale_price);

	* (arg) = "so_lostsale";
	* (arg+ (1)) = shhbrHash;
	* (arg+ (2)) = shhccHash;
	* (arg+ (3)) = shhcuHash;
	* (arg+ (4)) = sohr_rec.area_code;
	* (arg+ (5)) = sohr_rec.sman_code;
	* (arg+ (6)) = wk_qty;
	* (arg+ (7)) = wk_value;
	* (arg+ (8)) = "F";
	* (arg+ (9)) = (char *)0;
	shell_prog (2);
	return (EXIT_SUCCESS);
}

/*---------------------------------
| Menu called when CTRL W pressed |
---------------------------------*/
int
win_function (
 int    fld,
 int    lin,
 int    scn,
 int    mode)
{
	int	tmp_line, tmp_p_stat, this_page;
	struct	SEL_LIST *lcl_ptr;
	int	hold_mail = _mail_ok;

	_mail_ok = FALSE;
	_win_func = FALSE;
	
	opt_menu [NEGOT].flag  = SHOW;
	if (cur_field == label ("item_no"))
	{
		opt_menu [S_HIST].flag = ALL;
		opt_menu [PROMO].flag  = ALL;
		opt_menu [BRAND].flag  = ALL;
		opt_menu [PARCEL].flag = ALL;
		opt_menu [I_HIST].flag = SHOW;
	}
	else
	{
		opt_menu [S_HIST].flag = SHOW;
		opt_menu [PROMO].flag  = SHOW;
		opt_menu [BRAND].flag  = SHOW;
		opt_menu [PARCEL].flag = SHOW;
		opt_menu [I_HIST].flag = ALL;
		if (cur_field == label ("qty_ord") && mode != ENTRY)
			opt_menu [NEGOT].flag  = ALL;
	}

	list_made = FALSE;
	tmp_line = line_cnt;
	line_cnt = lin;

	tmp_p_stat = prog_status;
	checking_comp = FALSE;
#ifndef GVISION
	run_menu (opt_menu, "", input_row);
#else
    run_menu (NULL, opt_menu);
#endif

	clear_ok = FALSE;
	init_ok = FALSE;

	/*---------------------------
	| Redraw Order Entry Screen |
	---------------------------*/
	prog_status = tmp_p_stat;

	line_cnt = lin;
	lcount [6] = (prog_status == ENTRY) ? line_cnt : lcount [6];

	/*--------------------------------
	| Process List Of Selected Items |
	--------------------------------*/
	switch (list_made)
	{
	case FALSE:
		break;

	case NEGOT:
		dflt_used = FALSE;

		spec_valid (label ("qty_ord"));
		putval (line_cnt);
		skip_entry = 0;
		break;

	case S_HIST:
	case PROMO:
		ProcList ();
		InitLine ();
		tmp_line = line_cnt;
		break;

	case BRAND:
		ProcList ();
		InitLine ();
		tmp_line = line_cnt;
		/*
		heading (6);
		scn_set (6);
		scn_write (6);
		*/
		break;

	case PARCEL:
		lcl_ptr = selhd_ptr;
		while (lcl_ptr != SEL_NULL)
		{
			this_page = line_cnt / TABLINES;
			ProcessKitItem (lcl_ptr->hhbrHash, lcl_ptr->sel_ord);
			skip_entry = goto_field (label ("qty_ord"),
		                 	         label ("item_no"));
			lcl_ptr = lcl_ptr->next;
			if (this_page != (line_cnt / TABLINES))
			{
				this_page = line_cnt % TABLINES;
				if (!this_page)
					scn_write (6);
				blank_display ();
			}
		}
		tmp_line = line_cnt;
		InitLine ();
		break;
	}

	FreeTheList ();
	crsr_on ();
	init_ok = TRUE;

	line_cnt = tmp_line;

	_mail_ok = hold_mail;
	_win_func = TRUE;

	return (PSLW);
}

void
ProcList (void)
{
	int		i; 
	int		this_page;
	int		fldInvalid;
	long	hold_date = sohr_rec.dt_required;
	struct	SEL_LIST *lcl_ptr;

	this_page = line_cnt / TABLINES;
	lcl_ptr   = selhd_ptr;
	while (lcl_ptr != SEL_NULL)
	{
		sprintf (local_rec.item_no, "%-16.16s", lcl_ptr->item_no);
		dflt_used = FALSE;

		if (ValidItemNo (FALSE))
		{
			lcl_ptr = lcl_ptr->next;
			continue;
		}

		local_rec.qty_ord = lcl_ptr->sel_ord;
		local_rec.qty_back = 0.00;
		if (local_rec.qty_ord == 0.00)
			get_entry (label ("qty_ord"));

		if (local_rec.qty_ord == 0.00)
		{
			lcl_ptr = lcl_ptr->next;
			continue;
		}

		if (inmr_rec.costing_flag [0] == 'S')
			local_rec.qty_ord = 1.00;

		for (i = label ("item_no") ; i <= label ("due_date") ; i++)
		{
			dflt_used 	=	TRUE;
			fldInvalid = spec_valid (i);
			while (fldInvalid)
			{
				if (i == label ("sale_price"))
				{
					get_entry (label ("sale_price"));
				}
				else
				{
					/*-------------------------------
					| No specific actions for field |
					| so move on to next iteration  |
					| of for loop.                  |
					-------------------------------*/
					break;
				}

				fldInvalid = spec_valid (i);
			}
		}

		putval (line_cnt);

		if (this_page != (line_cnt / TABLINES))
		{
			scn_write (cur_screen);
			lcount [6] = line_cnt;
			this_page = line_cnt / TABLINES;
		}
		lcount [6] = line_cnt;

		line_display ();
		line_cnt++;

		lcl_ptr = lcl_ptr->next;
	}
	lcount [6] = line_cnt;
	sohr_rec.dt_required = hold_date;
}

/*-----------------------------
| Initialise contents of line |
-----------------------------*/
void
InitLine (void)
{
	sprintf (local_rec.item_no, "%-16.16s", " ");
	sprintf (soln_rec.item_desc, "%-36.36s", " ");
	sprintf (soln_rec.sman_code, "%2.2s", " ");
	sprintf (soln_rec.cus_ord_ref, "%-20.20s", " ");
	sprintf (soln_rec.pack_size, "%-5.5s", " ");
	local_rec.qty_ord = 0.00;
	local_rec.qty_back = 0.00;
	soln_rec.cost_price = 0.00;
	soln_rec.sale_price = 0.00;
	soln_rec.dis_pc = 0.00;
	soln_rec.due_date = 0L;
	sprintf (local_rec.soh_detail, "%-26.26s", " ");
}

/*-------------------------------------------------
| Maintain LAST CALL NOTES / NOTES and COMPLAINTS |
-------------------------------------------------*/
int
RingCallNotes (void)
{
	int	cur_scn;
	int	chr, ok;
	int	tmp_field;

	tmp_field = cur_field;
	crsr_off ();

	if (!first_notes)
	{
		rv_pr (ML (mlTsMess033), (132 - (int) strlen (ML (mlTsMess033))) / 2, 0, 1);

#ifdef GVISION
		scn_hide (6);
#endif	/* GVISION */

	}
	CustomerHeading (TRUE);

	/*------------------------
	| Load and display the 3 |
	| maintainable screens   |
	------------------------*/
	LoadText (LST_CALL);
	scn_display (3);

	LoadText (NOTES);
	scn_display (4);

	LoadText (COMPLAINT);
	scn_display (5);

	/*----------------------
	| Display sales screen |
	----------------------*/
	crsr_off ();
	DisplaySales ();

	cur_scn = 3;
	ok = TRUE;
	while (ok)
	{
#ifdef GVISION
		crsr_off ();
		for (;;)
		{
	    	mmenu_print ("  MAINTAIN NOTES  ", notes_menu, 0);
	    	switch (mmenu_select (notes_menu))
	    	{
			case MM_LAST_CALL :
				edit (3);
				break;

			case MM_NOTES :
				edit (4);
				break;

			case MM_COMPLAINTS :
				edit (5);
				break;

			case RESTRT :
				restart = TRUE;
				ok = FALSE;
				break;

			case PG_EXIT :
				ok = FALSE;
				break;
	    	}

			if (!ok)
				break;
		}
#else
		crsr_off ();
		rv_pr (hd_scn [0], 24, 7,  (cur_scn == 3));
		rv_pr (hd_scn [1], 95, 7,  (cur_scn == 4));
		rv_pr (hd_scn [2], 27, 13, (cur_scn == 5));

        chr = getkey ();
		switch (chr)
		{
		case LEFT_KEY:
		case '\b':
			if (cur_scn == 4 || cur_scn == 5)
				cur_scn = 3;
			else
				cur_scn = 4;
			break;

		case UP_KEY:
			if (cur_scn == 4 || cur_scn == 5)
				cur_scn = 3;
			else
				cur_scn = 5;
			break;

		case RIGHT_KEY:
			if (cur_scn == 3 || cur_scn == 5)
				cur_scn = 4;
			else
				cur_scn = 3;
			break;

		case DOWN_KEY:
			if (cur_scn == 3 || cur_scn == 4)
				cur_scn = 5;
			else
				cur_scn = 3;
			break;

		case '\r':
			edit (cur_scn);
			break;

		case FN1:
			restart = TRUE;
			ok = FALSE;
			break;

		case FN3:
			scn_display (cur_scn);
			break;

		case FN16:
			ok = FALSE;
			break;

		default:
			putchar (BELL);
			break;
		}
#endif	/* GVISION */
	}
	UpdateText (LST_CALL);
	UpdateText (NOTES);
	UpdateText (COMPLAINT);

#ifdef GVISION
	//-----------------------
	// Hide notes objects
	//
	scn_hide (3);
	scn_hide (4);
	scn_hide (5);
	for (int i = 12; i < 18; i++)
	{
		move (0, i);
		cl_line ();
	}
	move (40, 19);
	cl_line ();

	if (!first_notes)
		scn_display (6);
#endif	/* GVISION */

	first_notes = FALSE;

	cur_field = tmp_field;
	return (EXIT_SUCCESS);
}

int
LoadText (
 char   load_type)
{
	int	lcl_scn;

	/*----------------------------
	| Set screen N - for putval. |
	----------------------------*/
	switch (load_type)
	{
	case LST_CALL:
		lcl_scn = 3;
		break;

	case NOTES:
		lcl_scn = 4;
		break;

	case COMPLAINT:
		lcl_scn = 5;
		break;

	case NEXT_VISIT:
		lcl_scn = 10;
		break;

	default:
		return (FALSE);
	}

	scn_set (lcl_scn);
	lcount [lcl_scn] = 0;

	init_ok = TRUE;
	init_vars (lcl_scn);
	init_ok = FALSE;

	tsxd_rec.hhcu_hash = currHhcuHash;
	sprintf (tsxd_rec.type, "%c", load_type);
	tsxd_rec.line_no = 0;
	cc = find_rec (tsxd, &tsxd_rec, GTEQ, "r");
	while (!cc &&
	       tsxd_rec.hhcu_hash == currHhcuHash &&
	       tsxd_rec.type [0] == load_type)
	{
		sprintf (local_rec.text, "%-60.60s", tsxd_rec.desc);
		putval (lcount [lcl_scn]++);

		cc = find_rec (tsxd, &tsxd_rec, NEXT, "r");
	}
	return (TRUE);
}

int
UpdateText (
 char   load_type)
{
	int	lcl_scn;
	int	txt_cnt;

	/*----------------------------
	| Set screen N - for putval. |
	----------------------------*/
	switch (load_type)
	{
	case LST_CALL:
		lcl_scn = 3;
		break;

	case NOTES:
		lcl_scn = 4;
		break;

	case COMPLAINT:
		lcl_scn = 5;
		break;

	case NEXT_VISIT:
		lcl_scn = 10;
		break;

	default:
		return (FALSE);
	}
	scn_set (lcl_scn);

	for (txt_cnt = 0;txt_cnt < lcount [lcl_scn] ;txt_cnt++)
	{
		getval (txt_cnt);

		tsxd_rec.hhcu_hash = currHhcuHash;
		sprintf (tsxd_rec.type, "%c", load_type);
		tsxd_rec.line_no = txt_cnt;

		cc = find_rec (tsxd,&tsxd_rec,COMPARISON,"u");
		if (cc)
		{
			sprintf (tsxd_rec.desc, "%-60.60s", local_rec.text);

			cc = abc_add (tsxd,&tsxd_rec);
			if (cc)
				file_err (cc, tsxd, "DBADD");
		}
		else
		{
			sprintf (tsxd_rec.desc, "%-60.60s", local_rec.text);

			cc = abc_update (tsxd,&tsxd_rec);
			if (cc)
				file_err (cc, tsxd, "DBUPDATE");
		}
	}
	tsxd_rec.hhcu_hash = currHhcuHash;
	sprintf (tsxd_rec.type, "%c", load_type);
	tsxd_rec.line_no = txt_cnt;

	cc = find_rec (tsxd,&tsxd_rec,GTEQ, "r");
	while (!cc &&
	       tsxd_rec.hhcu_hash == currHhcuHash &&
	       tsxd_rec.type [0] == load_type)
	{
		abc_delete (tsxd);

		tsxd_rec.hhcu_hash = currHhcuHash;
		sprintf (tsxd_rec.type, "%c", load_type);
		tsxd_rec.line_no = txt_cnt;

		cc = find_rec (tsxd, &tsxd_rec, GTEQ, "r");
	}
	return (TRUE);
}

/*=============================
| if no lines enter yet allow
| user to select contract
================================*/
int
ContractDetails (void)
{
	int	old_scr = cur_screen;
	int	old_prog_exit = prog_exit;
	int	old_restart = restart;
	int	old_edit_exit = edit_exit;

	init_vars (12);
	heading (12);

	prog_exit = FALSE;

	while (!prog_exit)
	{
		entry_exit = FALSE;
		edit_exit = FALSE;
		restart = FALSE;
		search_ok = TRUE;

		init_vars (12);
		entry (12);
		if (!strcmp (sohr_rec.cont_no, "      ") || restart)
		{
			strcpy (sohr_rec.cont_no, "      ");
			sprintf (local_rec.cont_desc, "%40.40s", " ");
			scn_display (12);
			break;
		}
		
		scn_display (12);
		edit (12);
		if (restart)
			continue;

		break;
	}

	prog_exit = old_prog_exit;

	restart = old_restart;
	edit_exit = old_edit_exit;
	cur_screen = old_scr;
	heading (cur_screen);
	return (EXIT_SUCCESS);
}

/*-------------------------------
| Display note pad information. |
-------------------------------*/
int
RingCreditNotes (void)
{
	char	disp_str [200];

	CustomerHeading (TRUE);
	Dsp_open (1,7,7);
	Dsp_saverec ("    Contact Name    |  Date  |                                   Comments.                                    ");
	Dsp_saverec ("");
	Dsp_saverec (" [NEXT SCREEN]  [PREV SCREEN]  [EDIT/END]");

	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no");

	cucc_rec.hhcu_hash = currHhcuHash;
	cucc_rec.record_no = 0;
	cc = find_rec ("cucc", &cucc_rec, GTEQ,"r");
	while (!cc && cucc_rec.hhcu_hash == currHhcuHash)
	{
		sprintf (disp_str, "%-20.20s^E%-10.10s^E%-80.80s",
				cucc_rec.con_person,
				DateToString (cucc_rec.cont_date),
				cucc_rec.comment);

		Dsp_saverec (disp_str);
		cc = find_rec ("cucc", &cucc_rec, NEXT,"r");
	}
	abc_fclose (cucc);

	Dsp_srch ();
	Dsp_close ();

	return (EXIT_SUCCESS);
}

/*-------------------------------------------
| Select Items Based On Sales History       |
| The next 160 lines deal with Sales Hist.  |
-------------------------------------------*/
int
RingSalesHistory (void)
{
	CustomerHeading (TRUE);
	rv_pr (ML (mlTsMess035), (132 - (int) strlen (ML (mlTsMess035))) / 2, 0, 1);

	if (MTH_SALES)
		open_rec (sadf,  sadf_list, SADF_NO_FIELDS, "sadf_id_no3");
	else
		open_rec (tshs, tshs_list, TSHS_NO_FIELDS, "tshs_id_no");

	SetPeriodHeader (tspm_rec.sales_per);
	OpenItemTab ();

	CalcSales ();

	ScanItemTab ();

	if (selhd_ptr != SEL_NULL)
		list_made = S_HIST;

	if (MTH_SALES)
		abc_fclose (sadf);
	else
		abc_fclose (tshs);

	return (TRUE);
}

/*-------------------------------------------
| Calculate weekly/daily sales analysis for |
| current hhcu and selected hhbr hash       |
-------------------------------------------*/
void
CalcSales (void)
{
	int	i, j;
	int	data_found = FALSE;
	long	prevHhbrHash = 0L;
	long	perd_date [11], st_date, skip_per;

	for (i = 0; i < 11; i++)
		sales_period [i] = 0.00;
	if (MTH_SALES)
	{
	    sadf_rec.hhcu_hash = currHhcuHash;
	    sadf_rec.hhbr_hash = 0L;
	    cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
	    while (!cc && sadf_rec.hhcu_hash == currHhcuHash)
	    {
		if (sadf_rec.year [0] != 'C')
		{
		    cc = find_rec (sadf, &sadf_rec, NEXT, "r");
		    continue;
		}

		if (!data_found)
	    	    prevHhbrHash = sadf_rec.hhbr_hash;

		if (sadf_rec.hhbr_hash != prevHhbrHash && !AllZero ())
		{
		    AddItem (prevHhbrHash);

		    prevHhbrHash = sadf_rec.hhbr_hash;
		    for (i = 0; i < 11; i++)
			sales_period [i] = 0.00;
		}

		for (i = (curr_mnth -1), j = 0; j < 11; i--, j++)
		{
		    if (i < 0)
			i = 11;

		    sales_period [j] += sadf_qty [i];
		}
		data_found = TRUE;
	        cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	    }
	}
	else
	{
	    if (tspm_rec.sales_per [0] == 'D')
	    {
	    	st_date = local_rec.lsystemDate;
			skip_per = 1L;
	    }
	    else
	    {
	    	st_date = local_rec.lsystemDate - (long)curr_day;
			skip_per = 7L;
	    }
	    for (i = 0; i < 11; i++)
	    {
			perd_date [i] = st_date;
			st_date -= skip_per;
	    }

	    tshs_rec.hhcu_hash = currHhcuHash;
	    tshs_rec.hhbr_hash = 0L;
	    tshs_rec.date = 0L;
	    cc = find_rec (tshs, &tshs_rec, GTEQ, "r");
	    prevHhbrHash = tshs_rec.hhbr_hash;
	    while (!cc && tshs_rec.hhcu_hash == currHhcuHash)
	    {
			if (tshs_rec.hhbr_hash != prevHhbrHash &&
				!AllZero ())
			{
				AddItem (prevHhbrHash);

				prevHhbrHash = tshs_rec.hhbr_hash;
				for (i = 0; i < 11; i++)
					sales_period [i] = 0.00;
			}

			for (i = 0; i < 11; i++)
			{
				if (tshs_rec.date >= perd_date [i] &&
					tshs_rec.date < (perd_date [i] + skip_per))
				{
					sales_period [i] += tshs_rec.qty;
					break;
				}
			}

			data_found = TRUE;
			cc = find_rec (tshs, &tshs_rec, NEXT, "r");
	    }
	}
	if (data_found && !AllZero ())
	    AddItem (prevHhbrHash);
}

int
AllZero (void)
{
	int	i;

	for (i = 0; i < 11; i++)
		if (sales_period [i] != 0.00)
			return (FALSE);
	return (TRUE);
}

int
AddItem (
	long	hhbrHash)
{
	char	tmp_val [11] [5];
	int	i;

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	for (i = 0; i < 11; i++)
	{
		if (sales_period [i] == 0.00)
			strcpy (tmp_val [i], "    ");
		else
			sprintf (tmp_val [i], "%4.0f", sales_period [i]);
	}

	tab_add ("item_lst",
		" %-16.16s| %-40.40s| %-10.10s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s           %08ld  ",
		inmr_rec.item_no, inmr_rec.description, "   0.00",
		tmp_val [0],  tmp_val [1], tmp_val [2], tmp_val [3], tmp_val [4],
		tmp_val [5],  tmp_val [6], tmp_val [7], tmp_val [8], tmp_val [9],
		tmp_val [10], inmr_rec.hhbr_hash);
	no_items++;

	return (TRUE);
}

/*-------------------------------------------
| Select Promotional Items                  |
| The next 150 lines deal with Promotions   |
-------------------------------------------*/
int
RingPromoItem (void)
{
	int		i;
	char	br_no [3];
	char	wh_no [3];
	char	tmp_item [17];

	CustomerHeading (TRUE);
	rv_pr (ML (mlTsMess036), (132 - (int) strlen (ML (mlTsMess036))) / 2, 0, 1);

	FreeTheList ();
	no_items = 0;

	strcpy (br_no, comm_rec.est_no);
	strcpy (wh_no, comm_rec.cc_no);

	FindPromo (currHhcuHash, "  ", "   ", br_no, wh_no);
	FindPromo (currHhcuHash, "  ", "   ", br_no,  "  ");
	FindPromo (currHhcuHash, "  ", "   ",  "  ",  "  ");
	FindPromo (currHhcuHash, sohr_rec.area_code, "   ", br_no, wh_no);
	FindPromo (currHhcuHash, sohr_rec.area_code, "   ", br_no,  "  ");
	FindPromo (currHhcuHash, sohr_rec.area_code, "   ",  "  ",  "  ");
	FindPromo (0L, "  ", cumr_rec.class_type, br_no, wh_no);
	FindPromo (0L, "  ", cumr_rec.class_type, br_no,  "  ");
	FindPromo (0L, "  ", cumr_rec.class_type,  "  ",  "  ");
	FindPromo (0L, "  ", "   ", br_no, wh_no);
	FindPromo (0L, "  ", "   ", br_no,  "  ");
	FindPromo (0L, "  ", "   ",  "  ",  "  ");

	tab_open ("prom_lst", prom_keys, 7, 1, 8, FALSE);
	tab_add ("prom_lst",
		"# %-16.16s  %-40.40s  %-10.10s     %-40.40s ",
		" Item Number", " Item Description",
		" Order Qty",   " Promotional Narrative");

	if (selhd_ptr == SEL_NULL)
	{
		tab_add ("prom_lst", " ** No Promotions For This Customer ** ");
		putchar (BELL);
		tab_display ("prom_lst", TRUE);
		sleep (sleepTime);
		tab_close ("prom_lst", TRUE);
		return (FALSE);
	}
	else
	{
		selcur_ptr = selhd_ptr;
		while (selcur_ptr != SEL_NULL)
		{
			inmr_rec.hhbr_hash = selcur_ptr->hhbrHash;
		    cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		    if (!cc)
		    {
				tab_add ("prom_lst",
			    		" %-16.16s  %-40.40s        0.00     %-40.40s          %08ld  ",
			    		inmr_rec.item_no, inmr_rec.description,
			    		selcur_ptr->comment, inmr_rec.hhbr_hash);
				no_items++;
		    }
		    selcur_ptr = selcur_ptr->next;
		}

		tab_scan ("prom_lst");
		/*------------------------------
		| Build list of selected items |
		------------------------------*/
		FreeTheList ();
		if (make_list)
		{
		    for (i = 0; i < no_items; i++)
		    {
		    	tab_get ("prom_lst", get_buf, EQUAL, i);
		    	if (atof (get_buf + 64) != 0.00)
		    	{
			    	sprintf (tmp_item, "%-16.16s", get_buf + 1);
			    	AddList (tmp_item, atol (get_buf + 121), (float)atof (get_buf + 64), " ");
		    	}
		    }
		}
		tab_close ("prom_lst", TRUE);
	}
	scn_display (6);
	if (selhd_ptr != SEL_NULL)
		list_made = PROMO;
    return (EXIT_SUCCESS);
}

/*----------------------------------------
| Enter quantities for promotional items |
----------------------------------------*/
static int
PromoFunc (
 int        c,
 KEY_TAB *  psUnused)
{
	int	y_pos;

	tab_get ("prom_lst", get_buf, CURRENT, 0);

	y_pos = tab_sline ("prom_lst");
	local_rec.sel_ord = (float)atof (get_buf + 64);
	crsr_on ();
	local_rec.sel_ord = getfloat (66, y_pos, "NNNN.NN");
	crsr_off ();

	tab_update ("prom_lst",
				" %-16.16s  %-40.40s     %7.2f     %-70.70s",
				get_buf + 1,
				get_buf + 19,
				local_rec.sel_ord,
				get_buf + 73);

	return (c);
}

/*------------------------------------------
| Find any items that the current customer |
| gets a discount on                       |
------------------------------------------*/
void
FindPromo (
	long	hhcuHash,
	char	*areaCode,
	char	*cust_type,
	char	*br_no,
	char	*wh_no)
{
	struct	SEL_LIST 	*lcl_ptr;
	int		item_fnd;
	char	incpKey [7];

	sprintf (incpKey, "%2.2s%2.2s%2.2s", comm_rec.co_no, br_no, wh_no);

	strcpy (incp_rec.key, incpKey);
	strcpy (incp_rec.status, "A");
	incp_rec.hhcu_hash = hhcuHash;
	sprintf (incp_rec.curr_code, "   ");
	strcpy (incp_rec.area_code, "  ");
	strcpy (incp_rec.cus_type, "   ");
	incp_rec.hhbr_hash = 0L;
	incp_rec.date_from = 0L;
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (incp_rec.key, incpKey) &&
	       incp_rec.hhcu_hash == hhcuHash)
	{
		if ((!strcmp (cust_type, "   ") &&
		      strcmp (incp_rec.cus_type, "   ")) ||
		     (strcmp (cust_type, "   ") &&
		      strcmp (incp_rec.cus_type, cust_type)))
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}
		if ((!strcmp (areaCode, "   ") &&
		      strcmp (incp_rec.area_code, "   ")) ||
		     (strcmp (areaCode, "   ") &&
		      strcmp (incp_rec.area_code, areaCode)))
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}

		if (comm_rec.dbt_date < incp_rec.date_from ||
		     comm_rec.dbt_date > incp_rec.date_to)
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}

		item_fnd = FALSE;
		lcl_ptr = selhd_ptr;
		while (lcl_ptr != SEL_NULL)
		{
			if (incp_rec.hhbr_hash == lcl_ptr->hhbrHash)
			{
				item_fnd = TRUE;
				break;
			}

			lcl_ptr = lcl_ptr->next;
		}

		if (!item_fnd)
			AddList (" ", incp_rec.hhbr_hash, 0.00, incp_rec.comment);
		
		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
}

/*-----------------------------------------------
| Brand Promotions                              |
| The next 104 lines deal with Brand Promotions |
-----------------------------------------------*/
int
RingBrandPromo (void)
{
	int	tmp_field;

	tmp_field = cur_field;

	abc_selfield (inmr, "inmr_id_no_4");
	SetPeriodHeader ("M");

	init_vars (8);
	heading (8);
	rv_pr (ML (mlTsMess037), (132 - (int) strlen (ML (mlTsMess037))) / 2, 0, 1);

	entry (8);
	if (restart)
	{
		abc_selfield (inmr, "inmr_id_no");
		restart = FALSE;
		cur_field = tmp_field;
		return (FALSE);
	}
	OpenItemTab ();

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.maker_no, "%-16.16s", local_rec.brand);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (inmr_rec.maker_no, local_rec.brand))
	{
		MonthlySales (inmr_rec.hhbr_hash);
		tab_add ("item_lst",
				" %-16.16s  %-40.40s  %-10.10s %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f           %08ld  ",
				inmr_rec.item_no, inmr_rec.description, "   0.00",
				sales_period [0], sales_period [1], sales_period [2],
				sales_period [3], sales_period [4], sales_period [5],
				sales_period [6], sales_period [7], sales_period [8],
				sales_period [9], sales_period [10],
				inmr_rec.hhbr_hash);
		no_items++;

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	ScanItemTab ();
	abc_selfield (inmr, "inmr_id_no");
	if (selhd_ptr != SEL_NULL)
		list_made = BRAND;

	cur_field = tmp_field;
	scn_display (6);
	return (TRUE);
}

void
OpenItemTab (void)
{
	no_items = 0;
	tab_open ("item_lst", item_keys, 7, 1, 8, FALSE);
	tab_add ("item_lst",
			"# %-16.16s| %-40.40s| %-10.10s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s|%-4.4s",
			" Item Number", " Item Description", "Order Qty",
			per_hd [0], per_hd [1], per_hd [2], per_hd [3], per_hd [4],per_hd [5],
			per_hd [6], per_hd [7], per_hd [8], per_hd [9], per_hd [10]);
}

int
ScanItemTab (void)
{
	int	i;
	char	tmp_item [17];

	if (no_items == 0)
	{
		putchar (BELL);
		tab_add ("item_lst", " ** No Valid Items ** ");
		tab_display ("item_lst", TRUE);
		sleep (sleepTime);
		tab_close ("item_lst", TRUE);
		return (FALSE);
	}
	else
	{
		tab_scan ("item_lst");
		/*------------------------------
		| Build list of selected items |
		------------------------------*/
		FreeTheList ();
		if (make_list)
		{
		    for (i = 0; i < no_items; i++)
		    {
				tab_get ("item_lst", get_buf, EQUAL, i);
				if (atof (get_buf + 62) != 0.00)
				{
			    	sprintf (tmp_item, "%-16.16s", get_buf + 1);
			    	AddList (tmp_item, atol (get_buf + 136),
				     	 	  (float)atof (get_buf + 62), " ");
				}
		    }
		}
		tab_close ("item_lst", TRUE);
		return (TRUE);
	}
}

/*--------------------------------------
| Enter quantities for brand promotion |
--------------------------------------*/
static int
item_func (
 int        c,
 KEY_TAB *  psUnused)
{
	int		y_pos;
	int		validQty;

	tab_get ("item_lst", get_buf, CURRENT, 0);

	y_pos = tab_sline ("item_lst");

	/*--------------
	| Lookup item. |
	--------------*/
	inmr2_rec.hhbr_hash	= atol (get_buf + 137);
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr2, "DBFIND");

	validQty = FALSE;
	while (!validQty)
	{
		local_rec.sel_ord = (float)atof (get_buf + 62);
		crsr_on ();
		local_rec.sel_ord = getfloat (63, y_pos, "NNNN.NN");
		crsr_off ();

		if (last_char == FN1)
			break;

		if (inmr2_rec.serial_item [0] == 'Y' &&
			local_rec.sel_ord != 1.00 &&
			local_rec.sel_ord != 0.00)
		{
			putchar (BELL);
			continue;
		}

		validQty = TRUE;
	}

	tab_update ("item_lst",
				"%-17.17s  %-40.40s  %7.2f    %-75.75s",
				get_buf,
				get_buf + 19,
				local_rec.sel_ord,
				get_buf + 72);

	return (c);
}

/*-----------------------------------
| Tag mailer to be sent to customer |
-----------------------------------*/
int
RingTagMailer (void)
{
	int	i, num_mail;
	long	hhlh_hash;
	char	hhcu_str [9];
	char	hhlh_str [9];
	char	lp_str [3];

	num_mail = 0;

	local_rec.mailer_lpno = get_lpno (0);
	for (i = 1; i < 7; i++)
		print_at (i, 1, "%-60.60s", " ");

	CustomerHeading (TRUE);

	strcpy (err_str, ML (mlTsMess032));
	rv_pr (err_str, (132 - (int) strlen (err_str)) / 2, 0, 1);
/*
	DrawTotal ();
	CalculateTotal ();
*/
	
	rv_pr (ML (mlTsMess038), (132 - (int) strlen (ML (mlTsMess038))) / 2, 0, 1);

	tab_open ("mail_tag", mail_keys, 7, 1, 7, FALSE);
	tab_add ("mail_tag",
			"# %-10.10s  %-40.40s ",
			" LETTER CODE ", " LETTER DESCRIPTION ");

	open_rec (tslh, tslh_list, TSLH_NO_FIELDS, "tslh_id_no");
	open_rec (tsls, tsls_list, TSLS_NO_FIELDS, "tsls_id_no");

	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", " ");
	cc = find_rec (tslh, &tslh_rec, GTEQ, "r");
	while (!cc && !strcmp (tslh_rec.co_no, comm_rec.co_no))
	{
	    if (FOLLOW_UP)
	    {
			tsls_rec.hhlh_hash = tslh_rec.hhlh_hash;
			tsls_rec.hhcu_hash = currHhcuHash;
			tsls_rec.date_sent = local_rec.lsystemDate;
			cc = find_rec (tsls, &tsls_rec, COMPARISON, "r");
			if ((!cc && tsls_rec.date_called == 0L) || NO_MAILER)
			{
		    	cc = find_rec (tslh, &tslh_rec, NEXT, "r");
		    	continue;
			}
	    }
	
	    /*------------------------------
	    | Can't send label definitions |
	    ------------------------------*/
	    if (LABEL_DEF)
	    {
			cc = find_rec (tslh, &tslh_rec, NEXT, "r");
			continue;
	    }

	    num_mail++;

	    tab_add ("mail_tag",
		    	" %-10.10s  %-40.40s           %08ld",
		    	tslh_rec.let_code,
		    	tslh_rec.let_desc,
		    	tslh_rec.hhlh_hash);

	    cc = find_rec (tslh, &tslh_rec, NEXT, "r");
	}

	abc_fclose (tslh);
	abc_fclose (tsls);

	if (num_mail == 0)
	{
	    tab_add ("mail_tag",
				"      ***  NO MAILERS TO SEND TO THIS CUSTOMER ***");
	    putchar (BELL);
	    tab_display ("mail_tag", TRUE);
	    sleep (sleepTime);
	    tab_close ("mail_tag", TRUE);
	}
	else
	{
	    tab_scan ("mail_tag");
	    if (!make_list)
	    {
			tab_close ("mail_tag", TRUE);
			return (FALSE);
	    }

	    for (i = 0; i < num_mail; i++)
	    {
			tab_get ("mail_tag", get_buf, EQUAL, i);
			if (tagged (get_buf))
			{
		    	hhlh_hash = atol (get_buf + 63);
		    	sprintf (hhcu_str, "%08ld", cumr_rec.hhcu_hash);
		    	sprintf (hhlh_str, "%08ld", hhlh_hash);
		    	sprintf (lp_str,   "%2d",   local_rec.mailer_lpno);
		    	if (!fork ())
		    	{
					execlp ("ts_prtmail",
							"ts_prtmail",
							hhcu_str,
							hhlh_str,
							lp_str,
							 (char *)0);
		    	}
			}
	    }
	    tab_close ("mail_tag", TRUE);
	}
	return (TRUE);
}

/*------------------------------
| Tag/Untag Mailer For Sending |
------------------------------*/
static	int
mtag_func (
 int        c,
 KEY_TAB *  psUnused)
{
	tag_toggle ("mail_tag");

	return (c);
}

/*-------------------------------------------
| Select Parcel Items                       |
| The next 221 lines deal with Parcel Items |
-------------------------------------------*/
int
RingParcelPromo (void)
{
	char	tmp_item [17];
	int	i, no_in_tab;

	CustomerHeading (TRUE);

	rv_pr (ML (mlTsMess039), (132 - (int) strlen (ML (mlTsMess039))) / 2, 0, 1);

	if (envVarTsPrclexp)
		set_keys (prcl_keys, "E", KEY_PASSIVE);
	else
		set_keys (prcl_keys, "E", KEY_ACTIVE);

	tab_open ("prcl_lst", prcl_keys, 7, 1, 8, FALSE);

	open_rec (sokt,  sokt_list, SOKT_NO_FIELDS, "sokt_id_no");

	tab_add ("prcl_lst",
			"# %-16.16s   %-40.40s   %-10.10s    %-10.10s ",
			" Item Number ",
			" Item Description ",
			"Parcel Qty",
			"Order Qty");

	no_in_tab = LoadPrcl ();
	if (no_in_tab > 0)
	{
		tab_scan ("prcl_lst");
		/*-------------------------------------
		| Build linked list of selected items |
		-------------------------------------*/
		FreeTheList ();
		if (make_list)
		{
		    for (i = 0; i < no_in_tab; i++)
		    {
				tab_get ("prcl_lst", get_buf, EQUAL, i);
				if (tagged (get_buf) && atof (get_buf + 78) != 0.00)
				{
					sprintf (tmp_item, "%-16.16s", get_buf + 2);
					AddList (tmp_item,
						 	  atol (get_buf + 94),
						 	  (float)atof (get_buf + 78),
						 	  " ");
				}
		    }
		}
		tab_close ("prcl_lst", TRUE);
		if (selhd_ptr != SEL_NULL)
			list_made = PARCEL;
		abc_fclose (sokt);
		return (TRUE);
	}
	else
	{
		putchar (BELL);
		tab_add ("prcl_lst", " ** %-29.29s ** ", mlTsCall  [2]);
		sleep (sleepTime);
		tab_display ("prcl_lst", TRUE);
		tab_close ("prcl_lst", TRUE);
		abc_fclose (sokt);
		return (FALSE);
	}
	scn_display (6);
}

/*---------------------------------------
| Load Parcel Items Into Tabular Screen |
---------------------------------------*/
int
LoadPrcl (void)
{
	int	no_in_tab, cc1, kit_head;
	int	first_kit = TRUE;
	long	prevHhbrHash;
	char	*sptr;

	no_in_tab = 0;
	prevHhbrHash = 0L;
	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = 0L;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && !strcmp (sokt_rec.co_no, comm_rec.co_no))
	{
		kit_head = FALSE;
		if (prevHhbrHash != sokt_rec.hhbr_hash)
			kit_head = TRUE;

		if (kit_head)
		{
			inmr2_rec.hhbr_hash	=	sokt_rec.hhbr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		}
		     	       	  
		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc1 = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc || cc1)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (kit_head)
		{
			if (!first_kit && envVarTsPrclexp)
			{
				tab_add ("prcl_lst", "%-84.84s", " ");
				no_in_tab++;
			}

			tab_add ("prcl_lst",
					"* %-16.16s     %-40.40s %-10.10s       0.00          %08ld  ",
					inmr2_rec.item_no,
					inmr2_rec.description,
					" ",
					inmr2_rec.hhbr_hash);
			no_in_tab++;
		}

		if (envVarTsPrclexp)
		{
			sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
			if (sokt_rec.bonus [0] == 'Y')
			{
				sptr = clip (inmr_rec.item_no);
				sprintf (local_rec.item_no, "%-s%-.*s",
					sptr, 16 - (int) strlen (sptr), "/B");
			}

			tab_add ("prcl_lst",
					"     %-16.16s  %-40.40s %10.2f %-10.10s ",
					local_rec.item_no,
					inmr_rec.description,
					sokt_rec.matl_qty,
					" ");
			no_in_tab++;
		}
		first_kit = FALSE;
		prevHhbrHash = sokt_rec.hhbr_hash;
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	return (no_in_tab);
}

/*------------------------------
| Enter quantities for parcels |
------------------------------*/
static int
prcl_func (
 int        c,
 KEY_TAB *  psUnused)
{
	int	y_pos;

	tab_get ("prcl_lst", get_buf, CURRENT, 0);
	if (!tagged (get_buf))
	{
		putchar (BELL);
		return (c);
	}

	y_pos = tab_sline ("prcl_lst");
	local_rec.sel_ord = (float)atof (get_buf + 78);
	crsr_on ();
	local_rec.sel_ord = getfloat (80, y_pos, "NNNN.NN");
	crsr_off ();

	tab_update ("prcl_lst",
				"* %-16.16s     %-40.40s %-10.10s    %7.2f          %08ld  ",
				get_buf + 2,
				get_buf + 23,
				" ",
				local_rec.sel_ord,
				atol (get_buf + 94));

	return (c);
}

static	int
prclexp_func (
 int        c,
 KEY_TAB*   psUnused)
{
	char	*sptr;
	long	tmp_hhbr;

	tab_get ("prcl_lst", get_buf, CURRENT, 0);
	tmp_hhbr = atol (get_buf + 94);

	tab_open ("prcl_exp", null_keys, 8, 2, 5, FALSE);
	tab_add ("prcl_exp",
			"#     %-16.16s  %-40.40s %-10.10s  ",
			"Item Number",
			" Item Description ",
			"Parcel Qty");
	tab_add ("prcl_exp", "* %-16.16s     %-40.40s ", get_buf + 2, get_buf + 23);

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash 	= tmp_hhbr;
	sokt_rec.line_no 	= 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
	       sokt_rec.hhbr_hash == tmp_hhbr)
	{
		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
		if (sokt_rec.bonus [0] == 'Y')
		{
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no, "%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), "/B");
		}

		tab_add ("prcl_exp", "     %-16.16s  %-40.40s %10.2f  ",
				local_rec.item_no, inmr_rec.description,
				sokt_rec.matl_qty);

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	tab_scan ("prcl_exp");
	tab_close ("prcl_exp", TRUE);
	tab_display ("prcl_lst", TRUE);
	redraw_keys ("prcl_lst");

	return (c);
}

/*--------------------------------------------
| Negotiate Item Price, Discount, Markup etc |
--------------------------------------------*/
int
RingNegoScreen (void)
{
	/*---------------------
	| Check for contract. |
	---------------------*/
	if (store [line_cnt].contractStatus)
	{
		print_mess (ML (mlTsMess073));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*-------------------------------
	| Disable edit of qty BO field. |
	-------------------------------*/
	negoScn [1].fldEdit = 0;  

	/*-------------------------------------------
	| Initialise values for negotiation window. |
	-------------------------------------------*/
	negoRec.qOrd	=	store [line_cnt].qtyOrder;
	negoRec.qBord	=	store [line_cnt].qtyTotal - store [line_cnt].qtyOrder;

	negoRec.regPc			=  	store [line_cnt].regPc;
	negoRec.discArray [0]	=	store [line_cnt].discA;
	negoRec.discArray [1]	=	store [line_cnt].discB;
	negoRec.discArray [2]	=	store [line_cnt].discC;
	negoRec.grossPrice		=	store [line_cnt].grossSalePrice;
	negoRec.salePrice		=	store [line_cnt].salePrice;
	negoRec.margCost		= 	store [line_cnt].margCost;
	negoRec.outer_size		= 	store [line_cnt].outerSize;

	NegPrice (2, 7, local_rec.item_no, soln_rec.item_desc, 
				   store [line_cnt].cumulative, 6);

	if (!restart)
	{
		local_rec.qty_ord 		=   negoRec.qOrd;

		store [line_cnt].qtyOrder	=	negoRec.qOrd;
		store [line_cnt].qtyTotal	= 	negoRec.qOrd + negoRec.qBord;
		store [line_cnt].regPc		= 	negoRec.regPc;
		store [line_cnt].discA		= 	negoRec.discArray [0];
		store [line_cnt].discB		= 	negoRec.discArray [1];
		store [line_cnt].discC		= 	negoRec.discArray [2];
		store [line_cnt].discPc		=	CalcOneDisc (store [line_cnt].cumulative,
													negoRec.discArray [0],
													negoRec.discArray [1],
													negoRec.discArray [2]);
		store [line_cnt].grossSalePrice = 	negoRec.grossPrice;
		store [line_cnt].salePrice	 =	negoRec.salePrice;
		store [line_cnt].actualSale	 =	negoRec.salePrice;
		store [line_cnt].margCost 	 =	negoRec.margCost;

		soln_rec.dis_pc  			= 	ScreenDisc (store [line_cnt].discPc);
		soln_rec.sale_price 		= 	store [line_cnt].salePrice;

		if (store [line_cnt].calcSalePrice != soln_rec.sale_price)
			strcpy (store [line_cnt].priceOveride, "Y");

		if (store [line_cnt].calcDisc != ScreenDisc (soln_rec.dis_pc))
			strcpy (store [line_cnt].discOveride, "Y");

		putval (line_cnt);

		CalculateTotal ();
	}
	restart = FALSE;
    return (EXIT_SUCCESS);
}

/*-----------------------------------
| Display Item History For Customer |
-----------------------------------*/
int
RingItemHistory (void)
{
	char	disp_str [200];

	rv_pr (ML (mlTsMess040), (132 - (int) strlen (ML (mlTsMess040))) / 2, 0, 1);

	open_rec (cush,cush_list,CUSH_NO_FIELDS,"cush_id_no");

	Dsp_open (1,7,7);
	Dsp_saverec (" SALE DATE | QUANTITY |  EXT PRICE. |  DISCOUNT ");
	Dsp_saverec ("");
	Dsp_saverec ("  [REDRAW]  [NEXT]  [PREV]  [EDIT/END] ");

	cush_rec.hhcu_hash = currHhcuHash;
	cush_rec.line_no = 0;
	cc = find_rec ("cush", &cush_rec, GTEQ, "r");
	while (!cc && cush_rec.hhcu_hash == currHhcuHash)
	{
		if (strcmp (cush_rec.item_no, local_rec.item_no))
		{
			cc = find_rec ("cush", &cush_rec, NEXT, "r");
			continue;
		}
		sprintf (disp_str," %-10.10s ^E %8.2f ^E %11.2f ^E %9.2f ",
				DateToString (cush_rec.pur_date),
				cush_rec.item_qty,
				DOLLARS (cush_rec.item_price),
				cush_rec.item_disc);

		Dsp_saverec (disp_str);
		cc = find_rec ("cush", &cush_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (cush);

	return (EXIT_SUCCESS);
}

/*------------------------------------
| Arrange next visit/phone date/time |
------------------------------------*/
int
RingNextVisit (void)
{

	int	tmp_field;
	rv_pr (ML (mlTsMess041), (132 - (int) strlen (ML (mlTsMess041))) / 2, 0, 1);

#ifdef GVISION
	scn_hide (6);
#endif	/* GVISION */

	tmp_field = cur_field;
	restart = FALSE;
	init_ok = FALSE;
	init_vars (9);

	/*------------------------------
	| Load next visit notes screen |
	------------------------------*/
	LoadText (NEXT_VISIT);
	scn_display (10);

	heading (9);
	scn_display (9);
	edit (9);
		
	if (restart)
	{
		local_rec.n_visit_date = tspm_rec.n_visit_date;
		sprintf (local_rec.n_visit_time, "%-5.5s", tspm_rec.n_visit_time);
		local_rec.n_phone_date = tspm_rec.n_phone_date;
		sprintf (local_rec.n_phone_time, "%-5.5s", tspm_rec.n_phone_time);
		cur_field = tmp_field;
		restart = FALSE;
#ifdef GVISION
		scn_hide (10);
#endif	/* GVISION */

		return (EXIT_FAILURE);
	}

	if (local_rec.n_visit_date != tspm_rec.n_visit_date ||
	    strcmp (local_rec.n_visit_time, tspm_rec.n_visit_time))
		upd_visit = TRUE;

	if (local_rec.n_phone_date != tspm_rec.n_phone_date ||
	    strcmp (local_rec.n_phone_time, tspm_rec.n_phone_time))
		upd_phone = TRUE;

	/*---------------------------
	| Maintain next visit notes |
	---------------------------*/
	scn_write (9);
	scn_display (10);
	scn_set (10);
	edit (10);
	if (!restart)
		UpdateText (NEXT_VISIT);
	
	restart = FALSE;
	list_made = FALSE;
	cur_field = tmp_field;

#ifdef GVISION
		scn_hide (10);
		for (int i = 7; i < 13; i++)
		{
			move (0, i);
			cl_line ();
		}
#endif	/* GVISION */
	scn_display (6);

	return (EXIT_SUCCESS);
}

/*--------------------------------------
| Calculate monthly sales analysis for |
| current hhcu and selected hhbr hash  |
--------------------------------------*/
void
MonthlySales (
	long	hhbrHash)
{
	int	i, j;

	open_rec (sadf,  sadf_list, SADF_NO_FIELDS, "sadf_id_no3");

	for (i = 0; i < 11; i++)
		sales_period [i] = 0.00;

	sadf_rec.hhcu_hash = currHhcuHash;
	sadf_rec.hhbr_hash = hhbrHash;
	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
	while (!cc &&
	       sadf_rec.hhcu_hash == currHhcuHash &&
	       sadf_rec.hhbr_hash == hhbrHash)
	{
		if (sadf_rec.year [0] != 'C')
		{
			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			continue;
		}

		for (i = (curr_mnth -1), j = 0; j < 11; i--, j++)
		{
			if (i < 0)
				i = 11;

			sales_period [j] += sadf_qty [i];
		}
		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}

	abc_fclose (sadf);
}

/*--------------------------------
| Draw standard customer heading |
--------------------------------*/
int
CustomerHeading (
 int    ALL_DISP)
{
	heading (0);

	if (ALL_DISP)
	{
		box (0, 1, 132, 3);
		print_at (2, 1, ML (mlTsMess078),
				cumr_rec.dbt_no, cumr_rec.dbt_name, headOffice);

		print_at (3, 1, ML (mlTsMess079),
				cumr_rec.phone_no, tspm_rec.cont_name1, tspm_rec.cont_name2);

		print_at (4, 1, ML (mlTsMess028), 
				DateToString (tspm_rec.lphone_date), cumr_rec.sman_code, order_ref);
	}
	else
	{
		box (0, 1, 96, 3);
		print_at (2, 1, ML (mlTsMessb78),
				cumr_rec.dbt_no, cumr_rec.dbt_name);

		print_at (3, 1, ML (mlTsMessb79),
				cumr_rec.phone_no, tspm_rec.cont_name1);

		print_at (4, 1, ML (mlTsMessb28) ,
				DateToString (tspm_rec.lphone_date), cumr_rec.sman_code);
	}
	return (EXIT_SUCCESS);
}

/*------------------------------
| Display Sales Details Screen |
------------------------------*/
int
DisplaySales (void)
{
	int	i;
	double	last_12mth;

	last_12mth = 0.00;

	ClearBox (68, 13, 62, 4);

	rv_pr (ML (mlTsMess042), 90, 13, 0);

	print_at (14,69, ML (mlTsMess087),
			 DOLLARS (cumr_rec.mtd_sales), DOLLARS (cumr_rec.ytd_sales));

	open_rec (cusa,  cusa_list, CUSA_NO_FIELDS, "cusa_id_no");

	cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cusa_rec.year,"C");
	cc = find_rec ("cusa",&cusa_rec,COMPARISON,"r");
	if (!cc)
	{
		for (i = 0; i < 12; i++)
			last_12mth += DOLLARS (cusa_balance [i]);
	}

	abc_fclose (cusa);

	print_at (15,97, ML (mlTsMess029), last_12mth);
	print_at (16,69, ML (mlTsMess030), DateToString (cumr_rec.date_lastinv));
	print_at (17,69, ML (mlTsMess088) , tspm_rec.lst_ord);

	return (EXIT_SUCCESS);
}

int
GetHoCustomer (void)
{
	if (cumr_rec.ho_dbt_hash == 0L)
		strcpy (headOffice, "      ");
	else
	{
		cumr2_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
		if (cc)
			strcpy (headOffice, "      ");
		else
			sprintf (headOffice, "%-6.6s", cumr2_rec.dbt_no);
	}
	return (EXIT_SUCCESS);
}

static int
abort_func (
 int        c,
 KEY_TAB*   psUnused)
{
	make_list = FALSE;
	return (FN16);
}

static int
dummy_func (
 int        iUnused,
 KEY_TAB*   psUnused)
{
	return (EXIT_SUCCESS);
}

static int
ok_func (
	int       c,
	KEY_TAB*  psUnused)
{
	if (checking_comp == TRUE)
	{
		tab_get ("item_lst", get_buf, EQUAL, 0);
		if (atof (get_buf + 62) == 0.00)
		{
			print_mess (ML (mlTsMess074));
			sleep (sleepTime);
			clear_mess ();
			return (' ');
		}
	}

	make_list = TRUE;
	return (FN16);
}

int
heading (
 int    scn)
{
	if (scn != cur_screen && scn != 0)
		scn_set (scn);

	swide ();
	clear ();

	if (scn == 1)
		box (0,2,132,2);

	if (scn != 6)
	{
		line_at (1,0,132);

		if (call_type == PLANNED)
			strcpy (err_str, ML (mlTsMess043));
		else if (call_type == MAILER)
			strcpy (err_str, ML (mlTsMess044));
		else if (call_type == RANDOM)
			strcpy (err_str, ML (mlTsMess045));
		else if (call_type == RECEIVE)
			strcpy (err_str, ML (mlTsMess046));
		else
			strcpy (err_str, ML (mlTsMess047));
	
		PrintCoStuff ();
	}

	if (scn != 12)
		rv_pr (err_str, (132 - (int) strlen (err_str)) / 2, 0, 1);

	if (scn == 2)
	{
		rv_pr (ML (mlTsMess044), (132 - (int) strlen (ML (mlTsMess044))) / 2, 0, 1);

		box (0,2,132,1);
	}

	if (scn == 6)
	{
		CustomerHeading (FALSE);
		strcpy (err_str, ML (mlTsMess032));
		rv_pr (err_str, (132 - (int) strlen (err_str)) / 2, 0, 1);

		DrawTotal ();
		CalculateTotal ();
	}

	if (scn == 7)
	{
		CustomerHeading (TRUE);
		box (0, 6, 132, 12);
	}

	if (scn == 9)
		box (0,2,132,4);

	if (scn == 10)
	{
		rv_pr (ML (mlTsMess048), (132 - (int) strlen (ML (mlTsMess048))) / 2, 0, 1);

		box (1,7,130,3);
	}

	if (scn == 8)
		box (0,2,132,1);
	
	if (scn == 11)
		box (0,2,132,1);

	if (scn == 12)
		box (0,2,132,2);

	/*---------------------
	| Display Date / Time |
	---------------------*/
	if (scn != 12)
	{
		print_at (0, 114, "%s  %-5.5s", local_rec.systemDate, TimeHHMM ());
		PrintCoStuff ();
	}

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	if (scn != 0)
		scn_write (scn);

    return (EXIT_SUCCESS);
}

/*-------------------------------------------
| Set up month array based on current month |
-------------------------------------------*/
void
SetPeriodHeader (
 char*  hd_type)
{
	int	i, j;

	for (i = 0; i < 11; i++)
	{
		switch (hd_type [0])
		{
		case 'W':
			if (i == 0)
				strcpy (per_hd [i], "CUR");
			else if (i >= 10)
				sprintf (per_hd [i], "W%2d", i);
			else
				sprintf (per_hd [i], " W%1d", i);
			break;

		case 'D':
			j = curr_day - (i % 7);
			if (j < 0)
				j += 7;
			sprintf (per_hd [i], "%-3.3s", day_name [j]);
			break;

		case 'M':
		default:
			j = curr_mnth - i - 1;
			if (j < 0)
				j += 12;
			sprintf (per_hd [i], "%-3.3s", mnth_name [j]);
			break;

		}
	}
}

/*-----------------------------
| Allocate a block of memory  |
| for one node of linked list |
-----------------------------*/
static struct CUST_LIST *
cust_alloc (void)
{
	struct CUST_LIST *lcl_ptr;

	lcl_ptr = (struct CUST_LIST *) malloc (sizeof (struct CUST_LIST));
	if (lcl_ptr == CUST_NULL)
		file_err (errno, "malloc", "CUST_LIST");
	return (lcl_ptr);
}

void
FreeTheList (void)
{
	struct SEL_LIST *lcl_ptr;

	if (selhd_ptr == SEL_NULL)
		return;

	selcur_ptr = selhd_ptr;
	while (selcur_ptr != SEL_NULL)
	{
		lcl_ptr = selcur_ptr;
		selcur_ptr = selcur_ptr->next;
		free (lcl_ptr);
	}
	selhd_ptr = SEL_NULL;
	selcur_ptr = SEL_NULL;
}

void
AddList (
	char	*itemNumber,
	long	hhbrHash,
	float	selectOrder,
	char	*lst_text)
{
	struct SEL_LIST *lcl_ptr;

	lcl_ptr = sel_alloc ();
	sprintf (lcl_ptr->item_no, "%-16.16s", itemNumber);
	lcl_ptr->hhbrHash = hhbrHash;
	lcl_ptr->sel_ord = selectOrder;
	sprintf (lcl_ptr->comment, "%-40.40s", lst_text);
	lcl_ptr->next = SEL_NULL;

	if (selhd_ptr == SEL_NULL)
		selhd_ptr = lcl_ptr;
	else
		selcur_ptr->next = lcl_ptr;
	selcur_ptr = lcl_ptr;
}

/*----------------------------------------
| Allocate a block of memory             |
| for one node of item selectlinked list |
----------------------------------------*/
static struct SEL_LIST *
sel_alloc (void)
{
	struct SEL_LIST *lcl_ptr;

	lcl_ptr = (struct SEL_LIST *) malloc (sizeof (struct SEL_LIST));
	if (lcl_ptr == SEL_NULL)
		file_err (errno, "malloc", "SEL_LIST");
	return (lcl_ptr);
}

/*===============================
| Search on Contract (cnch)     |
===============================*/
void
SrchCnch (
 char*  key_val)
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

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	sprintf (cnch_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, cnch, "DBFIND");
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char*  key_val)
{
	work_open ();
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
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

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}
	
void
tab_other (
 int    line_no)
{
	static	int	orig_disc;
	static	int	orig_sale;
	static	int	firstTime = TRUE;

	/*-------------------------
	| turn off and on editing
	| of fields depending on
	| whether contract or not
	------------------------*/
	if (store [line_no].contractStatus)
	{
		if (firstTime)
		{
			orig_disc = FLD ("disc");
			orig_sale = FLD ("sale_price");
		}
		FLD ("disc") = NA;
		FLD ("sale_price") = NA;
		firstTime = FALSE;
	}
	else
	{
		if (!firstTime)
		{
			FLD ("disc") = orig_disc;
			FLD ("sale_price") = orig_sale;
		}
	}
}


/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc (
 float  DiscountPercent)
{
	if (envVarSoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

/*===================
| Convertion of Qty |
===================*/
float
ToStdUom (
 float  lclqty)
{
	float	cnv_qty;

	if (F_HIDE (label ("UOM")))
		return (lclqty);

	if (SR.cnvFct == 0.00 || lclqty == 0.00)
		return (0.00);	

	cnv_qty = lclqty * SR.cnvFct;

	return (cnv_qty);
}


float
ToLclUom (
        float   lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.cnvFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty / SR.cnvFct;

    return (cnvQty);
}

/*=============================================================
| Clear screen where box to be drawn + if _box then draw box. |
=============================================================*/
void
ClearBox (
	int	x, 
	int y, 
	int h, 
	int v)
{
	int	i,
		j;
	box (x,y,h,v);

	if (h > 1)
	{
		j = v ;		
		i = y + 1;
	}
	else
	{
		j = v + 2;
		i = y;
	}
		
	while (j--)
	{
		if (h > 1)
			print_at (i, x + 1 , "%*.*s", h - 2, h - 2, " ");
		i++;
	}
}

/*=====================================================
| Routine to get price desctiptions from comm record. |
=====================================================*/
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
/*==================================r
| Check if customer has a contract. |
===================================*/
int
CheckContract (
 void)
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

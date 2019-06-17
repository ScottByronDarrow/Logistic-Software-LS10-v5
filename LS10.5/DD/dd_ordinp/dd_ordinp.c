/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dd_ordinp.c,v 5.15 2002/12/01 04:48:14 scott Exp $
|  Program Name  : (dd_ordinp.c)
|  Program Desc  : (Direct Delivery Order Input/Maintenance)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 30/05/94         |
|---------------------------------------------------------------------|
| $Log: dd_ordinp.c,v $
| Revision 5.15  2002/12/01 04:48:14  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.14  2002/11/28 04:09:46  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.13  2002/07/26 10:42:41  robert
| S/C 4216 - Fixed memory corruption on LS10-GUI
|
| Revision 5.12  2002/07/24 08:38:50  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.11  2002/07/17 09:57:09  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.10  2002/07/01 05:13:23  robert
| Name structure storeRec (for use with SetSortArray)
|
| Revision 5.9  2002/06/26 04:50:29  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/01/09 00:28:59  scott
| Updated to PO_INPUT not checked as not required.
|
| Revision 5.7  2001/11/08 03:24:40  robert
| Updated to move #include definition to fix compilation error on LS10-GUI
|
| Revision 5.6  2001/11/06 03:04:29  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dd_ordinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_ordinp/dd_ordinp.c,v 5.15 2002/12/01 04:48:14 scott Exp $";

#define	USE_WIN		1
#define MAXSCNS 	6
#define MAXLINES	500
#define TABLINES	10

#define	SR			store [line_cnt]
#define	MARG_MESS1	(envVarSoMargin [0] == '0')
#define	MARG_MESS2	(envVarSoMargin [0] == '1')
#define	DBOX_TOP	9
#define	DBOX_LFT	35
#define	DBOX_WID	68
#define	DBOX_DEP	3

#define	HDRSCN		1
#define	ORDSCN		2
#define	CSTSCN		3
#define	PRISCN		4
#define	TLRSCN		5
#define	SHPSCN		6


#define	PRIORITY	0
#define	PRICE		1

#define	PENDINGFLAG	"P"
#define	ACTIVEFLAG	"A"
#define	DELETEFLAG	"X"

/*
 * On-Cost Category Definitions
 */
#define	FOB			0 /* Goods			*/
#define	FRT			1 /* Freight		*/
#define	INS			2 /* Insurance		*/
#define	INTEREST	3 /* Intrest		*/
#define	CHG			4 /* Bank Charges	*/
#define	DTY			5 /* Duty			*/
#define	OT1			6 /* Other Costs 1	*/
#define	OT2			7 /* Other Costs 2	*/
#define	OT3			8 /* Other Costs 3	*/
#define	OT4			9 /* Other Costs 4	*/

#define 	DDGD_NULL 	((struct DDGD_PTR *) NULL)

#define		TXT_REQD
#include <pslscr.h>
#include <twodec.h>
#include <hot_keys.h>
#include <minimenu.h>
#include <get_lpno.h>
#include <getnum.h>
#include <ml_dd_mess.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <tabdisp.h>
#include <inis_update.h>
#include <proc_sobg.h>

#define	PROMPT	prompts [prmpt_type]._prompt
#define	SRCH	prompts [prmpt_type]._srch

#define	FGN_CURR	(envVarDbMcurr && strcmp (cumr_rec.curr_code, currencyCode))

	struct
    {
		char	*_prompt;
		char	*_srch;
    } prompts [] = { {"Invoice"},{"#Inv No."}};

static	int		SuppSlctFunc 	(int, struct tag_KEY_TAB *);
static	int		ExitFunc 		(int, struct tag_KEY_TAB *);
static	int		RestartFunc 	(int, struct tag_KEY_TAB *);

	static	KEY_TAB supp_keys [] =
	{
   		{ NULL, '\r', SuppSlctFunc,
		"Select Supplier for On-Costs.", "A", 0, 0, 0},
   		{ NULL, FN1, RestartFunc,
		"", "A", 0, 0, 0},
   		{ NULL, FN16, ExitFunc,
		 "Exit from current option.", "A", 0, 0, 0},
   		END_KEYS
	};

	/*
	 * Structure used for pop-up discount screen.
	 */
	typedef struct tagPromptInfo 
	{
  		char*     fldPrompt;
  		int       xPos;
  		char      fldMask [16];
	} __PromptInfo;

	__PromptInfo discScn[7];

	int		notax				= 0,				
			newOrder			= FALSE,
			ins_flag			= 0,	
			envVarDbNettUsed 	= TRUE,
			envVarPoAppFlag 	= FALSE,
			envVarFeInstall 	= FALSE,
			envVarPoPrint 		= FALSE,
			envVarPoMaxLines 	= 0,
			envVarPoInput		= 0,
			envVarPoNumGen 		= 0,
			hhcc_selected 		= FALSE,
			envVarPoOverRide	= 0,
			envVarWinOk			= 0,	
			envVarDbFind 		= 0,	
			envVarSoDoi			= 0,
			envVarCrFind 		= 0,
			envVarDdCrDefault 	= 0,
			envVarStopCrd 		= 1,
			envVarCrdTerm 		= 1,
			envVarCrdOver 		= 1,
			prmpt_type			= 0,
			envVarDbMcurr		= 0,
			envVarSoDiscRev 	= FALSE;

	char	*currentUser;

	char	envVarCrCo [3],	
			envVarDbCo [3],	
			envVarSoMargin [3],
			currencyCode [4],
			envVarPoReorder [27],
			envVarSupOrdRound [2];


	float	vol_tot = 0.00,		/* Volume total for spread*/
	     	wgt_tot = 0.00;		/*  total for spread*/

	double	l_dis 		= 0.00,		/* line item discount	*/
			l_tax 		= 0.00,		/* line item tax		*/
			l_gst 		= 0.00,		/* line item gst		*/
			l_total		= 0.00, 
			t_total		= 0.00,
			ord_total	= 0.00,
			c_left		= 0.00,
			fob_tot 	= 0.00,		/* total cost of goods	*/
			fai_tot 	= 0.00,		/* total cost of goods	*/
			dty_tot 	= 0.00,		/* total cost of goods	*/
			oth_tot 	= 0.00,		/* total cost of goods	*/
			inv_tot 	= 0.00,		/* invoiced total		*/
			tax_tot 	= 0.00,		/* tax total			*/
			tot_tot 	= 0.00,		/* Total all.			*/
			cst_tot 	= 0.00,		/* Cost total for spread*/
			fother 		= 0.00,		/* other total			*/
			envVarPoAppVal = 0.00;

	FILE	*pout;

	char	*cumr2 	= "cumr2",
			*ddhr2 	= "ddhr2",
			*ddln2 	= "ddln2",
			*pohr2 	= "pohr2",
			*poln2 	= "poln2",
			*data 	= "data",
			*sixteen_space = "                ",
			*twenty_spaces = "                    ",
			*forty_spaces  = "                                        ",
			*sixty_spaces  = "                                                            ";

#include	"schema"

struct crbkRecord	crbk_rec;
struct ddhrRecord	ddhr_rec;
struct ddhrRecord	ddhr2_rec;
struct ddlnRecord	ddln_rec;
struct ddshRecord	ddsh_rec;
struct ddgdRecord	ddgd_rec;
struct sudsRecord	suds_rec;
struct inisRecord	inis_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct posoRecord	poso_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct cuccRecord	cucc_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct pocrRecord	pocr_rec;
struct cudpRecord	cudp_rec;
struct pocfRecord	pocf_rec;
struct inspRecord	insp_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct inccRecord	incc_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct exsiRecord	exsi_rec;
struct inumRecord	inum_rec;
struct cudiRecord	cudi_rec;
struct cfhrRecord	cfhr_rec;
struct cflnRecord	cfln_rec;
struct podtRecord	podt_rec;
struct fehrRecord	fehr_rec;
struct fehrRecord	fehr2_rec;
struct felnRecord	feln_rec;

	Money	*cumr_bo		=	&cumr_rec.bo_current;
	
#include <FindCumr.h> /* requires cumr_rec */
#include <FindSumr.h> /* requires comm_rec */

	struct	DDGD_PTR
	{
		struct ddgdRecord ddgd_array [10];
	};

	struct storeRec {
		long 	hhbrHash;			/* inmr_hhbr_hash		*/
		long 	hhplHash;			/* poln_hhpl_hash		*/
		long 	hhdlHash;			/* ddln_hhdl_hash		*/
		long 	hhsuHash;			/* sumr_hhsu_hash		*/
		long 	hhdsHash;			/* ddsh_hhds_hash		*/
		long	hhccHash;			/* hashes remembered 					*/
		float 	 qty;				/* local_rec.qty		*/
		float	_gst_pc;			/* inmr_gst_pc or 0.00 if notax	*/
		float	_tax_pc;			/* inmr_gst_pc or 0.00 if notax	*/
		double	_tax_amt;			/* inmr_gst_amt " 0.00 if notax	*/
		float	_outer;				/* inmr_outer_size		*/
		float	_min_marg;			/* Min margin for category.     */
		double	_cost_price;		/* default sale price */
		char	_category [12];		/* serial number for line	*/
		char	_class [2];			/* item's class for line	*/
		char	_costing_flag [2];	/* inmr_costing_flag		*/
		double	_weight;
		int		_con_price;
		int		_indent;
		double	imp_duty;			/* duty rate from podt				*/
		char	duty_type [2];		/* duty type from podt				*/
		char	item_desc [41];
		double	net_fob;			/* Net FOB after discounts 			*/
		double	amt_fai;			/* value of freight + ins.			*/
		double	amt_dty;			/* value of duty          			*/
		double	amt_oth;			/* value of other costs.  			*/
		float	weight;				/* inis_weight						*/
		float	volume;				/* inis_volume						*/
		double	land_cst;			/* Landed cost.  					*/
		char	std_uom [5];		/* Standard (Stock) UOM.			*/
									/*==================================*/
									/* On-cost Screen Local field.		*/
									/*==================================*/
		char	supp_no [7];		/* Costing screen Supplier.			*/
		char	supp_name [41];		/* Supplier name.					*/
		char	supp_curr [5];		/* Supplier currency.				*/
		char	supp_uom [5];		/* Supplier UOM.					*/
		float	supp_conv;			/* Supplier UOM Conversion Factor	*/
		float	supp_lead;			/* Supplier Lead time.           	*/
		double	supp_exch;			/* PO Exchange Rate.				*/
									/*==================================*/
									/* On-cost Screen Local field.		*/
									/*==================================*/
struct	DDGD_PTR *ddgd_ptr;			/* Pointer to On-Cost details.		*/
									/*==================================*/
									/* Selling Price Calc Fields     	*/
									/*==================================*/
		int		_pricing_chk;		/* Pricing check done				*/
		int		_cumulative;		/* Discounts are cumulative ?  		*/
		char	_dis_or [2];		/* Discount override. 				*/
		char	_sellgrp [7];		/* item selling group.				*/
		float	_uplift;			/* Uplift							*/
		float	_dflt_uplift;		/* Standard Uplift					*/
		float	_dflt_disc;			/* inmr_disc_pc						*/
		float	_dis_pc;			/* ddln_disc_pc						*/
		float	_calc_disc;			/*                       			*/
		float	_reg_pc;			/* Regulatory percent.      		*/
		float	_disc_a;			/* Discount percent A.      		*/
		float	_disc_b;			/* Discount percent A.      		*/
		float	_disc_c;			/* Discount percent A.      		*/
		double	_sale_price;		/* ddln_sale_price					*/
		double	_calc_sprice;		/*                 					*/
		double	_act_sale;			/*                 					*/
		double	_gsale_price;		/* ddln_gsale_price					*/
		double	_dflt_price;		/* Standard Price.					*/
									/*==================================*/
									/*                               	*/
									/*==================================*/
		double	cst_price;			/* Cost price.						*/
		double	base_cost;			/* Base cost from inis or inei 		*/
		double	base_sale;			/* Base sale price from inpr.  		*/
		int		cumulative;			/* Discounts are cumulative ?  		*/
		float	discArray [4];		/* Regulatory and Disc A, B, C pcs */
		int		no_inis;			/* No Inventory supplier record (inis)  */
		int		upd_inis;			/* Update inventory supplier record.    */
		float	min_order,			/* stuff copied from inis_rec			*/
				ord_multiple;   	/* Relates to Min and order multiple.   */
		double	total;
		int		_cont_status;
		int		_cont_cost;			/* TRUE if cost on contract				*/
		int		line_num;
		int		_keyed;				/* 0 = nothing changed
									   1 = uplift changed
									   2 = sale price changed   */
	} store [MAXLINES];

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
	};

	char	cat_desc [10] [21];
	char	*inv_cat [] = {
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

	int		up_inis			= 0,
			heldOrder		= 0,
			printerNumber 	= 0;

	char	loc_prmt 	[21],
			fi_prmt 	[21],
			cif_prmt 	[21],
			dty_prmt 	[21],
			lcl_prmt 	[21],
			sel_prmt 	[21],
			ext_prmt 	[21],
			mar_prmt 	[21],
			poPrintProgram [15];

#include	<p_terms.h>

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	systemDate [11];		/* Current Date dd/mm/yy.			*/
	char	prog_desc [10];		/* HDRSCN description field			*/
	char	exch_desc [10];		/* HDRSCN description field			*/
	double	exch_rate;			/* Local Exchange Rate.				*/
								/*==================================*/
								/* Order Screen Local field.		*/
								/*==================================*/
	char	supp_no [7];		/* Costing screen Supplier.			*/
	char	supp_name [41];		/* Supplier UOM.					*/
	char	supp_curr [5];		/* Supplier UOM.					*/
	char	supp_uom [5];		/* Supplier UOM.					*/
	float	supp_conv;			/* Supplier UOM Conversion Factor	*/
	double	supp_exch;			/* PO Exchange Rate.				*/
								/*==================================*/
								/* Order Screen Local field.		*/
								/*==================================*/
	char	cst_category [21];	/* On-Cost Category.				*/
	char	cst_spread [2];		/* On-Cost Spread Allocation.		*/
	char	cst_curr [4];		/* On-Cost currency.				*/
	double	cst_fgn_val;		/* On-Cost Foreign Value.			*/
	double	cst_exch;			/* On-Cost Exchange Rate.			*/
	double	cst_loc_val;		/* On-Cost Local Value.           	*/
	long	cst_hhds;			/* On-Cost Shipment Hash.			*/
	long	cst_hhpo;			/* On-Cost Purchase Order Hash.		*/
								/*==================================*/
								/*==================================*/
								/* Line Item Screen Local field.	*/
								/*==================================*/
	char	item_no [17];		/* Local Item Number.				*/
	char	description [21];
	char	std_uom [5];		/* Local Standard UOM.				*/
	float	qty;				/* Local Quantity.					*/
	double	grs_fgn;			/* Free-on-board Fgn Dollars.		*/
	char	view_disc [2];		/* View discounts PO Screen 		*/
	double	net_fob;			/* Net after discount (Fgn dollars) */
	double	loc_fi;				/* Over-Seas Freight + Insurance.	*/
	double	cif_loc;			/* Cost/Insurance/Freight Local.	*/
	double	duty_val;			/* Duty Value.						*/
	char	duty_code [3];		/* Duty Code.						*/
	double	oth;				/* Other Cost 1-4 + Bank and Int.   */
	double	land_cst;			/* Landed cost.						*/
	double	fob_cost;			/* Free-On-Bourd Cost.				*/
	long	due_date;			/* Due Date at line item Level.		*/
	char	br_no [3];			/* Branch number.					*/
	char	br_name [41];		/* Branch name.						*/
	char	wh_no [3];			/* Warehouse number.				*/
	char	wh_name [41];		/* Warehouse name.					*/
	long	hhcc_hash;			/* Warehouse hhcc_hash.				*/
								/*==================================*/
	long	dflt_order_no;
	long	date_reqd;
	char	dflt_date_due [11];
	char	dummy [11];
	char	dflt_ord [2];
	char	cust_no [7];
	char	ho_cust_no [7];
	char	pri_desc [16];
	char	ord_desc [10];
	char	dbt_date [11];
	long	lsystemDate;
	char	other [3] [31];
	char	spinst [3] [61];
	char	sell_desc [31];
	float	qty_sup;
	double	extend;
	double	lcl_cst;
	float	margin;
	double	marval;
	float	uplift;
	char	dflt_sale_no [3];
	char	carr_area [3];
	char	cont_desc [41];
	char	req_usr [41];
	char	reason [41];
	int		no_ctn;
	double	wgt_per_ctn;
} local_rec;


struct {
	char	ship_method [2];	/* Shipment Method L)and S)ea A)ir			*/
	char	desc_method [11];	/* Description of shipment method 4 chars	*/
	long	con_no;				/* Customers consignment number				*/
	char	vessel [30];		/* Vessel Name.								*/
	char	space_book [11];	/* Space booked on vessel					*/
	char	carrier [43];		/* Carrier of goods.						*/
	char	book_ref [11];		/* Booking reference     					*/
	char	bol_no [11];		/* B.O.L. Number							*/
	char	airway [21];		/* Airways B.O.L. Number					*/
	long	date_load;			/* Date of loading.							*/
	char	con_rel_no [21];	/* Container release number.				*/
	long	date_depart;		/* Date of departure.						*/
	long	date_arrive;		/* Date of Arrival.							*/
	char	packing [41];		/* Packing details.							*/
	char	port_orig [26];		/* Port of origin/loading					*/
	char	dept_orig [21];		/* Loading depot (at port)					*/
	char	port_dsch [19];		/* Port of discharge.						*/
	char	port_dest [26];		/* Port of destination						*/
	char	marks [5] [34];		/* Shipment marks x 5						*/
} dflt_ship_rec;

static	struct	var	vars [] =
{
	{HDRSCN, LIN, "debtor",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Customer       :", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{HDRSCN, LIN, "name",	 4, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{HDRSCN, LIN, "cus_addr1",	 5, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr1},
	{HDRSCN, LIN, "cus_addr2",	 6, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "        : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr2},
	{HDRSCN, LIN, "cus_addr3",	 7, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "        : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr3},
	{HDRSCN, LIN, "cus_addr4",	 8, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "        : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr4},
	{HDRSCN, LIN, "order_no",	 5, 24, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "00000000", "Order Number   :", " ",
		 NE, NO,  JUSTLEFT, "", "", ddhr_rec.order_no},
	{HDRSCN, LIN, "date_raised",	6, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.dbt_date, "Order Date     :", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&ddhr_rec.dt_raised},
	{HDRSCN, LIN, "date_reqd",	 7, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date Required. :", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.date_reqd},
	{HDRSCN, LIN, "cus_ord_ref",	 8, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",      "Order Ref.     :", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.cus_ord_ref},
	{HDRSCN, LIN, "cont_no",	10, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Contract       :", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 YES, NO, JUSTLEFT, "", "", ddhr_rec.cont_no},
	{HDRSCN, LIN, "dp_no",	10, 56, CHARTYPE,
		"AA", "          ",
		" ", cumr_rec.department, "Dept No.    :", " ",
		YES, NO, JUSTRIGHT, "", "", ddhr_rec.dp_no},
	{HDRSCN, LIN, "fix_exch",	10, 100, CHARTYPE,
		"U", "          ",
		" ", "N",      "Fixed Quote      :", "Yes / No (Default = No)",
		 YES, NO,  JUSTLEFT, "YN", "", ddhr_rec.fix_exch},
	{HDRSCN, LIN, "exch_desc",10, 103, CHARTYPE,
		"AAA", "          ",
		" ", "N", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.exch_desc},
	{HDRSCN, LIN, "sale_code",	11, 24, CHARTYPE,
		"UU", "          ",
		" ", "  ", "Salesman       :", " ",
		YES, NO, JUSTRIGHT, "", "", ddhr_rec.sman_code},
	{HDRSCN, LIN, "pri_type",	11, 56, CHARTYPE,
		"N", "        ",
		" ", cumr_rec.price_type, "Price Type. :", " ",
		NA, NO,  JUSTLEFT, "1", "9", local_rec.pri_desc},
	{HDRSCN, LIN, "exch_rate",	11, 100, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", " ",      "Exchange Rate    :", " ",
		 NI, NO,  JUSTRIGHT, "", "", (char *) &local_rec.exch_rate},
	{HDRSCN, LIN, "terms",	 13, 24, CHARTYPE,
		"UUU", "          ",
		" ", "DIS", "Pricing Terms  :", "FOB,DIS,CIF. Default = DIS",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.sell_terms},
	{HDRSCN, LIN, "fwd_exch",	13, 88, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Forward Exch Contract:", " Enter Forward Exchange Contract - Search Available For This Customers Contracts",
		 ND, NO, JUSTLEFT, "", "", ddhr_rec.fwd_exch},
	{HDRSCN, LIN, "req",		14, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",         "Person Req P/O :", "Details of person requesting purchase order. ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.req_usr},
	{HDRSCN, LIN, "reason",	14, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Reason for Purchase. :", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.reason},
	{HDRSCN, LIN, "prog_inv",	15, 24, CHARTYPE,
		"U", "          ",
		" ", "Y",      "Progressive Inv:", "Yes / No (Default = Yes)",
		 YES, NO,  JUSTLEFT, "YN", "", ddhr_rec.progressive},
	{HDRSCN, LIN, "prog_desc",15, 27, CHARTYPE,
		"AAA", "          ",
		" ", "N", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.prog_desc},

	{ORDSCN, TAB, "br_no",	MAXLINES, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, "BR", "Enter Branch number.",
		 ND, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{ORDSCN, TAB, "wh_no",	 0, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.cc_no, "WH", "Enter Warehouse number.",
		 ND, NO, JUSTRIGHT, "", "", local_rec.wh_no},
	{ORDSCN, TAB, "hhcc_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhcc_hash},
	{ORDSCN, TAB, "item_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{ORDSCN, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Item Number.  ", " ",
		ND, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{ORDSCN, TAB, "supp",	 0, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", " Supplier ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.supp_no},
	{ORDSCN, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "1", " Quantity ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty},
	{ORDSCN, TAB, "cst_per",	 0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", "0", " Per  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{ORDSCN, TAB, "fob_cst",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", " FOB (FGN) ", "Enter Cost Per Item. (Return for Last Cost).",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.grs_fgn},
	{ORDSCN, TAB, "view_disc", 0, 0, CHARTYPE,
		"U", "          ",
		" ", "N", "V", " View and Amend Discounts (Y/N) ",
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.view_disc},
	{ORDSCN, TAB, "net_fob",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "NET FOB(FGN)", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_fob},
	{ORDSCN, TAB, "loc_fi",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", fi_prmt, "<Return> will Calculate Freight.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_fi},
	{ORDSCN, TAB, "cif_loc",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", cif_prmt, " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.cif_loc},
	{ORDSCN, TAB, "duty_val",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", dty_prmt, "<Return> will Calculate Duty.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.duty_val},
	{ORDSCN, TAB, "other",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", "Int/Bank/Other", " ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.oth},
	{ORDSCN, TAB, "land_cost",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", " Unit Cost. ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.land_cst},
	{ORDSCN, TAB, "due_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Due Date", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.due_date},
	{ORDSCN, TAB, "hhdl_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &ddln_rec.hhdl_hash},

	{CSTSCN, TAB, "category",	TABLINES, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Category      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cst_category},
	{CSTSCN, TAB, "spread",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", "D", "Spread", " by : D(ollar  W(eight  V(olume ",
		YES, NO,  JUSTLEFT, "DWV", "", local_rec.cst_spread},
	{CSTSCN, TAB, "currency",	 0, 3, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Curr Code", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.cst_curr},
	{CSTSCN, TAB, "fgn_val",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", " Foreign Value ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cst_fgn_val},
	{CSTSCN, TAB, "cst_exch",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNN", "          ",
		" ", "0", " Exch. Rate ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cst_exch},
	{CSTSCN, TAB, "loc_val",	 0, 0, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", loc_prmt, " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.cst_loc_val},
	{CSTSCN, TAB, "hhds_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.cst_hhds},
	{CSTSCN, TAB, "hhpo_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.cst_hhpo},

	{PRISCN, TAB, "item_no3",	 MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{PRISCN, TAB, "descr3",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Item Description   ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.description},
	{PRISCN, TAB, "qty3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", " Quantity ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_sup},
	{PRISCN, TAB, "lcl_cost",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", lcl_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.lcl_cst},
	{PRISCN, TAB, "uplift",	 0, 0, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", " ", " Uplift ", " ",
		YES, NO, JUSTRIGHT, "-999.99", "9999.99", (char *)&local_rec.uplift},
	{PRISCN, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", sel_prmt, " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&ddln_rec.sale_price},
	{PRISCN, TAB, "sale_disc",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Disc%", " ",
		YES, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&ddln_rec.disc_pc},
	{PRISCN, TAB, "extend",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", ext_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extend},
	{PRISCN, TAB, "margin",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "   GP % ", " ",
		NA, NO, JUSTRIGHT, "-99.99", "99.99", (char *)&local_rec.margin},
	{PRISCN, TAB, "marval",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", mar_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.marval},
	{TLRSCN, LIN, "shipname",	3, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship to name :", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.del_name},
	{TLRSCN, LIN, "shipaddr1",	4, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship to addr 1", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.del_add1},
	{TLRSCN, LIN, "shipaddr2",	5, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship to addr 2", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.del_add2},
	{TLRSCN, LIN, "shipaddr3",	6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship to addr 3", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.del_add3},
	{TLRSCN, LIN, "freight",	 3, 94, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ddhr_rec.freight},
	{TLRSCN, LIN, "other_1",	4, 94, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", local_rec.other [0], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ddhr_rec.other_cost_1},
	{TLRSCN, LIN, "other_2",	5, 94, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", local_rec.other [1], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ddhr_rec.other_cost_2},
	{TLRSCN, LIN, "other_3",	6, 94, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", local_rec.other [2], " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ddhr_rec.other_cost_3},
	{TLRSCN, LIN, "spcode0",	8, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 1", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{TLRSCN, LIN, "spcode1",	9, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 2", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{TLRSCN, LIN, "spcode2",	10, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 3", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{TLRSCN, LIN, "pay_term",	 11, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.pay_term},
	{TLRSCN, LIN, "no_ctn",	8, 103, INTTYPE,
		"NNNNNNNN", "          ",
		" ", " ", "No. of Carton", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.no_ctn},
	{TLRSCN, LIN, "wgt_per",	9, 103, DOUBLETYPE,
		"NNNNNNNN.NNNN", "          ",
		" ", " ",      "Weight/Carton    :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.wgt_per_ctn},
	{TLRSCN, LIN, "sin1",	13, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 1.", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.stdin1},
	{TLRSCN, LIN, "sin2",	14, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 2.", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.stdin2},
	{TLRSCN, LIN, "sin3",	15, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 3.", " ",
		YES, NO,  JUSTLEFT, "", "", ddhr_rec.stdin3},
	{TLRSCN, LIN, "del1",	16, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr 1.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.delin1},
	{TLRSCN, LIN, "del2",	17, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr 2.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.delin2},
	{TLRSCN, LIN, "del3",	18, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Instr 3.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.delin3},

	{SHPSCN, LIN, "ship_method",	3, 21, CHARTYPE,
		"U", "          ",
		" ", "S", "Shipment Method :", "Shipment Method L(and) / S(ea) / A(ir)",
		YES, NO,  JUSTLEFT, "LSA", "", dflt_ship_rec.ship_method},
	{SHPSCN, LIN, "consignment",	3, 88, LONGTYPE,
		"NNN", "          ",
		" ", "0", "Consignment No. :", "Customers Consignment Number",
		YES, NO,  JUSTRIGHT, "", "", (char *) &dflt_ship_rec.con_no},
	{SHPSCN, LIN, "desc_method",	3, 24, CHARTYPE,
		"AAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", dflt_ship_rec.desc_method},
	{SHPSCN, LIN, "vessel",	4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Vessel          :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.vessel},
	{SHPSCN, LIN, "space_book",	4, 88, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Space Booked        :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.space_book},
	{SHPSCN, LIN, "carrier",	5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipping Line   :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.carrier},
	{SHPSCN, LIN, "book_ref",	5, 88, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Booking Reference   :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.book_ref},
	{SHPSCN, LIN, "bol_no",	6, 21, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "B.O.L. No.      :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.bol_no},
	{SHPSCN, LIN, "airway",	6, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Airways B.O.L. No.  :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.airway},
	{SHPSCN, LIN, "date_load", 7, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Loading Date    :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&dflt_ship_rec.date_load},
	{SHPSCN, LIN, "con_rel_no",	7, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Container Release No:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.con_rel_no},
	{SHPSCN, LIN, "packing",	8, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Packing Details :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.packing},
	{SHPSCN, LIN, "port_orig",	10, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Departure Port  :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.port_orig},
	{SHPSCN, LIN, "date_depart", 10, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Departure Date      :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&dflt_ship_rec.date_depart},
	{SHPSCN, LIN, "dept_orig",	11, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Departure depot :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.dept_orig},
	{SHPSCN, LIN, "port_dsch",	12, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Discharge Port  :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.port_dsch},
	{SHPSCN, LIN, "port_dest",	13, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Destination     :", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.port_dest},
	{SHPSCN, LIN, "date_arrive", 13, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Arrival Date        :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&dflt_ship_rec.date_arrive},
	{SHPSCN, LIN, "mark0",	15, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipping Marks 1:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.marks [0]},
	{SHPSCN, LIN, "mark1",	16, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               2:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.marks [1]},
	{SHPSCN, LIN, "mark2",	17, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               3:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.marks [2]},
	{SHPSCN, LIN, "mark3",	18, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               4:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.marks [3]},
	{SHPSCN, LIN, "mark4",	19, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               5:", " ",
		YES, NO,  JUSTLEFT, "", "", dflt_ship_rec.marks [4]},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <cus_price.h>
#include <cus_disc.h>
#include <neg_win.h>
#include <SupPrice.h>
#include <CheckIndent.h>
#include <std_decs.h>

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

/*=====================================================================
| Local Variables
=====================================================================*/
int			PrintOff	=	0;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
double  CheckCumr 				(double, int);
double  DutyCalc 				(int);
double  FreightCalc 			(int);
double  PurchaseOrderValue 		(void);
float   RndMltpl 				(float, char *, float, float);
float   ScreenDisc 				(float);
int     AddIncc 				(void);
int     AddInei 				(void);
int     CheckDdhr 				(void);
int     CheckPohr 				(long);
int     CheckReorder 			(char *);
int     DdgdClear 				(struct DDGD_PTR *);
int     EditMenu 				(void);
int 	FindCucc 				(int, long);
int     FindInis 				(long, long);
int     FindPocf 				(char *);
int     FindPocr 				(char *);
int     GetDefaultSupplier 		(long);
int     IntLoadEnvironment 		(void);
int     LoadItems 				(long);
int     LoadPoln 				(long);
int     ReadSumr 				(long);
int     UpdateMenu 				(void);
int     UpdateOrder 			(char *);
int     WarningUser 			(char *, int, int);
int     heading 				(int);
int 	use_window 				(int);
int     spec_valid 				(int);
struct DDGD_PTR *   ddgd_alloc 	(void);
void    AddDdhrRecord 			(char *);
void    AddDdshRecord 			(long, long, char *, struct DDGD_PTR *);
void    AddPohrRecord 			(long, long, char *, int);
void    AddPolnRecord 			(long, long, int, char *);
void    BusyFunc 				(int);
void    CalExtend 				(int);
void    CalcCostTotals 			(long);
void    CalcCosts 				(int);
void    CalcExtend 				(int);
void    CalcItemTotals 			(long);
void    CalcTotals 				(void);
void    CatIntoPohr 			(void);
void    CloseDB 				(void);
void    DiscProcess 			(int);
void    DispCustSupp 			(int);
void    DspFlds 				(int);
void    DrawDiscScn 			(void);
void    DrawVLine 				(int, int);
void    FindExsi 				(void);
void    GetDdhrOrderNo 			(void);
void    GetOnCost 				(void);
void    GetWarehouse 			(long);
void    InputField 				(int);
void	InitML					(void);
void    LoadCatDesc 			(void);
void    LoadDdgd 				(struct DDGD_PTR *);
void    LoadIntoCstScn 			(void);
void    LoadIntoPriScn 			(void);
void    LoadIntoTlrScn 			(void);
void    MarginOk 				(double, double, float, int, double, float,int);
void    OpenDB 					(void);
void    PriceProcess 			(int);
void    PrintCompanyStuff 		(void);
void    SetOrder 				(int);
void    SpreadCosts 			(long);
void    SrchCnch 				(char *);
void    SrchCudp 				(char *);
int		SrchCudi 				(int);
void    SrchDdhr 				(char *);
void    SrchExsf 				(char *);
void    SrchExsi 				(void);
void    SrchFehr 				(char *);
void    SrchInis 				(void);
void    SrchPayTerms 			(void);
void    SrchPocr 				(char *);
void    SrchPrices 				(void);
void    UpdateFwdExch 			(double, char *);
void    UpdateInis 				(double, float);
void    UpdateLine 				(int);
void    shutdown_prog 			(void);
void    tab_other 				(int);
void    tab_screen2 			(int);
void	VerticalLine			(int, int);
void	_discScn				(int, char *, int, char *);
void	_discScnFreePrompt		(int);
void	_discScnSetPrompt		(int, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int		i;
	int		first_time = 1;
	char	*sptr;

	tab_row = 8;

	if (argc < 2)
	{
		print_at (0,0, ML (mlDdMess704) ,argv [0]);
		printf ("usage error:\n");
        return (EXIT_FAILURE);
	}

	printerNumber = (argc == 3) ? atoi (argv [2]) : 0;

	if (IntLoadEnvironment ())
    {
        return (EXIT_FAILURE);
    }

	vars [label ("order_no")].fill = " ";
	vars [label ("order_no")].lowval = alpha;

	init_scr ();
	_set_masks (argv [1]);

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ORDSCN, store, sizeof (struct storeRec));
#endif

	FLD ("fwd_exch") = (envVarFeInstall) ? NO : ND;

	/*
	 * Set up Screen Prompts.
	 */
	sprintf (loc_prmt,"  %-3.3s Value  ",currencyCode);
	sprintf (fi_prmt," F&I (%-3.3s)",currencyCode);
	sprintf (cif_prmt," CIF(%-3.3s) ",currencyCode);
	strcpy (dty_prmt,"Duty/ Unit");
	sprintf (lcl_prmt," Cost(%-3.3s) ",currencyCode);
	sprintf (mar_prmt," GP (%-3.3s) ",	currencyCode);

	/*
	 * open main database files.
	 */
	OpenDB ();

	InitML ();

	sptr = get_env ("CR_CO");
	strcpy (envVarCrCo, (atoi (sptr)) ? comm_rec.est_no : " 0");
	sptr = get_env ("DB_CO");
	strcpy (envVarDbCo, (atoi (sptr)) ? comm_rec.est_no : " 0");

	FLD ("extend")  = NA;
	FLD ("item_no") = NE;

	strcpy  (local_rec.dbt_date, DateToString (comm_rec.dbt_date));

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();
	local_rec.dflt_order_no = 0L;

	memset (store, 0, sizeof (store));
	for (i = 0; i < MAXLINES; i++)
		store [i].ddgd_ptr = DDGD_NULL;

	swide ();
	clear ();

	/*-----------------------------
	| Open Three Discount files.  |
	-----------------------------*/
	OpenPrice ();
	OpenDisc ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		set_tty (); 

		abc_unlock (ddhr);
		abc_unlock (ddln);
		abc_unlock (ddsh);
		abc_unlock (ddgd);
		abc_unlock (pohr);
		abc_unlock (poln);

		abc_selfield (pohr, "pohr_id_no");

		cumr_rec.hhcu_hash = 0L;
		cnch_rec.hhch_hash = 0L;
		notax = 0;
		lcount [ORDSCN] = 0;
		lcount [PRISCN] = 0;
		ins_flag = 0;
		inv_tot = 0.00;
		tax_tot = 0.00;
		tot_tot = 0.00;
		memset (&inmr_rec, 0, sizeof (inmr_rec));

		/*  I N V O I C E   P A R T  */
		if (restart) 
		{
			if (first_time)
			{
				strcpy (local_rec.dflt_ord,"D");

				if (!F_NOKEY (label ("date_raised")))
					FLD ("date_raised") = YES;

			}
		}

		for (i = 0; i < MAXLINES; i++)
		{
			if (store [i].ddgd_ptr != DDGD_NULL)
				DdgdClear (store [i].ddgd_ptr);
		}
		memset (store, 0, sizeof (store));
		for (i = 0; i < MAXLINES; i++)
			store [i].ddgd_ptr = DDGD_NULL;

		/*  O R D E R    P A R T  */

		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars (HDRSCN);	
		init_vars (ORDSCN);	
		init_vars (CSTSCN);	
		init_vars (PRISCN);	
		init_vars (TLRSCN);	
		init_vars (SHPSCN);	

		heading (HDRSCN);
		entry (HDRSCN);
		if (restart || prog_exit)
			continue;

		if (newOrder)
		{
			heading (ORDSCN);
			entry (ORDSCN);
			if (restart || (entry_exit && lcount [ORDSCN] == 0))
				continue;

			do
			{
			heading 	(CSTSCN);
			GetOnCost 	();
			scn_write 	(CSTSCN);
			if (sumr_rec.hhsu_hash > 0)
			{
				box (0, 2, 132, 2);
				DispCustSupp (-1);
				LoadCatDesc ();
				LoadIntoCstScn ();
				scn_display (CSTSCN);
				edit 		(CSTSCN);
				if (!restart)
					SpreadCosts (sumr_rec.hhsu_hash);
			}
			} while (!restart && sumr_rec.hhsu_hash > 0);

			heading (PRISCN);
			scn_display (PRISCN);
			edit (PRISCN);
			if (restart)
				continue;

			heading (TLRSCN);
			scn_display (TLRSCN);
			edit (TLRSCN);
			if (restart)
				continue;

			heading (SHPSCN);
			scn_display (SHPSCN);
			entry (SHPSCN);
			if (restart)
				continue;

			GetDdhrOrderNo ();
		}
		while (!edit_exit && !restart)
		{
			EditMenu ();

			if (!restart && edit_exit) 
				edit_exit = UpdateMenu ();
		}
		if (!restart)
			first_time = 0;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
	char	*scn_desc [] = 
	{
		"HEADER SCREEN.",
		"ORDER SCREEN.",
		"ON-COST SCREEN.",
		"PRICING SCREEN.",
		"INSTRUCTION SCREEN.",
	};


MENUTAB edit_menu [] =
	{
		{ " 1. HEADER SCREEN       ",
			" Direct Delivery Header Information " },
		{ " 2. ORDER SCREEN        ",
			" Customers Order Entry " },
		{ " 3. ON-COST SCREEN      ",
			" Supplier Costs Entry " },
		{ " 4. PRICING SCREEN      ",
			" Adjust Customer Pricing " },
		{ " 5. INSTRUCTION SCREEN  ",
			" Delivery Instructions " },
		{ " 6. SHIPMENT SCREEN     ",
			" Default Shipment Details " },
		{ ENDMENU }
	};


int
EditMenu (void)
{
	for (;;)
	{
		mmenu_print ("       Edit (3)        ", edit_menu, 0);
		switch (mmenu_select (edit_menu))
		{
			case  0 :
				heading 	(HDRSCN);
				scn_display (HDRSCN);
				edit 		(HDRSCN);
				return (TRUE);

			case  1 :
				heading 	(ORDSCN);
				scn_display (ORDSCN);
				edit 		(ORDSCN);
				return (TRUE);

			case  2 :
				heading 	(CSTSCN);
				GetOnCost 	();
				scn_write 	(CSTSCN);
				if (sumr_rec.hhsu_hash < 0)
					return (TRUE);
				box (0, 2, 132, 2);
				DispCustSupp (-1);
				LoadCatDesc ();
				LoadIntoCstScn ();
				scn_display (CSTSCN);
				edit 		(CSTSCN);
				if (!restart)
					SpreadCosts (sumr_rec.hhsu_hash);
				return (TRUE);

			case  3 :
				heading 	(PRISCN);
				scn_display (PRISCN);
				edit 		(PRISCN);
				return (TRUE);

			case  4 :
				heading 	(TLRSCN);
				scn_display (TLRSCN);
				edit 		(TLRSCN);
				return (TRUE);

			case  5 :
				heading 	(SHPSCN);
				scn_display (SHPSCN);
				edit 		(SHPSCN);
				return (TRUE);

			case -1 :
				edit_exit = TRUE;
				restart = TRUE;
				return (FALSE);
	
			case 99 :
				edit_exit = TRUE;
				return (TRUE);
			default :
				break;
		}
	}
}


MENUTAB upd_menu [] =
	{
		{ " 1. SAVE ORDER          ",
			" Save order to be continued later " },
		{ " 2. CONFIRM ORDER       ",
			" Confirm customers acceptance of order " },
		{ " 3. RE-EDIT ORDER       ",
			" Re-edit current order " },
		{ " 4. DELETE ORDER        ",
			" Delete previously saved order " },
		{ " 5. ABANDON CHANGES     ",
			" Abandon Changes made to order " },
		{ ENDMENU }
	};


int
UpdateMenu (void)
{
	for (;;)
	{
		mmenu_print ("  ORDER OPTIONS   ", upd_menu, 0);
		switch (mmenu_select (upd_menu))
		{
			case  0 :
			case 99 :	/* Save Order */
				cc = UpdateOrder (PENDINGFLAG);
				return (cc);

			case  1 :	/* Confirm Order */
				cc = UpdateOrder (ACTIVEFLAG);
				return (cc);

			case  3 :	/* Delete Order */
				cc = UpdateOrder (DELETEFLAG);
				return (cc);

			case  2 :	/* Re-Edit Order */
				return (FALSE);

			case  4 :
			case -1 :	/* Abandon Changes */
				restart = TRUE;
				return (TRUE);
	
			default :
				break;
		}
	}
}


/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (void)
{
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
		
	open_rec  (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_id_no");
	open_rec  (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
	open_rec  (ddsh, ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no");
	open_rec  (ddgd, ddgd_list, DDGD_NO_FIELDS, "ddgd_id_no");
	open_rec  (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec  (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec  (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec  (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec  (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec  (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec  (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec  (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec  (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec  (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec  (cncd, cncd_list, cncd_no_fields, "cncd_id_no2");
	open_rec  (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec  (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec  (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec  (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec  (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec  (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec  (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec  (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec  (fehr, fehr_list, FEHR_NO_FIELDS, "fehr_id_no");
	open_rec  (feln, feln_list, FELN_NO_FIELDS, "feln_id_no");
	open_rec  (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");

	open_rec  (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no" 
							  							   : "cumr_id_no3");
	open_rec  (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
														   : "sumr_id_no3");

	abc_alias (cumr2, cumr);
	abc_alias (pohr2, pohr);
	abc_alias (poln2, poln);
	abc_alias (ddhr2, ddhr);
	abc_alias (ddln2, ddln);

	open_rec  (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec  (poln2, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec  (ddln2, ddln_list, DDLN_NO_FIELDS, "ddln_hhdl_hash");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (ddgd);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_fclose (cudp);
	abc_fclose (cuit);
	abc_fclose (cucc);
	abc_fclose (exaf);
	abc_fclose (pocr);
	abc_fclose (cncl);
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (poln2);
	abc_fclose (ccmr);
	abc_fclose (sumr);
	abc_fclose (inum);
	abc_fclose (insp);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (podt);
	abc_fclose (inis);
	abc_fclose (suds);
	abc_fclose (fehr);
	abc_fclose (feln);
	abc_fclose (crbk);

	/*------------------------------
	| Close Three Discount files.  |
	------------------------------*/
	CloseDisc ();
	SearchFindClose ();

	abc_dbclose (data);
}

/*============================================
| Get common info from environment variables. |
============================================*/
int
IntLoadEnvironment (void)
{
	char		*sptr;

	currentUser = getenv ("LOGNAME");

	/*------------------------------
	| Get Supplier default search. |
	------------------------------*/
	sptr = chk_env ("DD_CR_DEFAULT");
	envVarDdCrDefault = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-----------------------------------
	| Get Purchase order print program. |
	-----------------------------------*/
	sptr = chk_env ("PO_PRINT");
	if (sptr == (char *)0)
		envVarPoPrint = FALSE;
	else
	{
		envVarPoPrint = TRUE;
		sprintf (poPrintProgram, "%-14.14s", sptr);
	}

	/*--------------------------------------------
	| Check if forward exchange module installed. |
	--------------------------------------------*/
	sptr = chk_env ("FE_INSTALL");
	envVarFeInstall = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*--------------------------------------------
	| Check for purchase order approval details. |
	--------------------------------------------*/
	sptr = chk_env ("PO_APP_FLAG");
	envVarPoAppFlag = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_APP_VAL");
	envVarPoAppVal = CENTS ((sptr == (char *)0) ? (double) 0.00 : atof (sptr));

	/*-------------------------------------------------------
	| Purchase Order number is Company or branch generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PO_NUM_GEN");
	envVarPoNumGen = (sptr == (char *)0) ? 0 : atoi (sptr);

	envVarPoOverRide = (sptr = chk_env ("PO_OVERRIDE")) ? atoi (sptr) : FALSE;

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envVarSupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envVarSupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envVarSupOrdRound, "D");
			break;

		default:
			sprintf (envVarSupOrdRound, "B");
			break;
		}
	}

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);


	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("PO_MAX_LINES");
	envVarPoMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_FIND");
	envVarDbFind = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("CR_FIND");
	envVarCrFind = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SO_DOI");
	envVarSoDoi = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (currencyCode,"%-3.3s",get_env ("CURR_CODE"));

	sptr = chk_env ("SO_MARGIN");
	sprintf (envVarSoMargin, "%-2.2s", (sptr == (char *)0) ? "00" : sptr);

	/*------------------------------------------------------
	| Check is class of item can be used using PO_RECEIPT. |
	------------------------------------------------------*/
	sptr = chk_env ("PO_REORDER");
	if (sptr == (char *)0)
		sprintf (envVarPoReorder,"%-26.26s","ABCD");
	else
		sprintf (envVarPoReorder,"%-26.26s", sptr);

	/*-----------------------------
	| Check and Get Credit terms. |
	-----------------------------*/
	sptr = get_env ("SO_CRD_TERMS");
	envVarStopCrd = (*(sptr + 0) == 'S');
	envVarCrdTerm = (*(sptr + 1) == 'S');
	envVarCrdOver = (*(sptr + 2) == 'S');

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVarWinOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_OTHER_1");
	sprintf (local_rec.other [0],"%.30s",(sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (local_rec.other [1],"%.30s",(sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (local_rec.other [2],"%.30s",(sptr == (char *)0) ? "Other Costs." : sptr);

    return (EXIT_SUCCESS);
}

/*==========================================
| Validate credit period and credit limit. |
==========================================*/
double
CheckCumr (
	double new_value,
	int    inc_orders)
{
	double	credit_over = 0.00;
	double	total_owing = 0.00;

	if (cumr_rec.crd_flag [0] == 'Y')
    {
	    return (EXIT_SUCCESS);
    }

	total_owing = cumr_bo [0] + 
		  	      cumr_bo [1] +
	              cumr_bo [2] + 
	  	          cumr_bo [3] +
	  	          cumr_bo [4] +
	  	          cumr_bo [5] +
		          new_value;

	if (inc_orders)
		total_owing += cumr_rec.ord_value;

	/*---------------------------------------------
	| Check if customer is over his credit limit. |
	---------------------------------------------*/
	if (cumr_rec.credit_limit <= total_owing && cumr_rec.credit_limit != 0.00)
	{
		credit_over = twodec ((total_owing - cumr_rec.credit_limit) / 100);
		return (credit_over);
	}

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int    field)
{
	int		i = 0;
	int		val_pterms = FALSE;
	double	wrk_fob;
	float	base_uplift;
	double  temp_dis = 0.00,
			temp_gross = 0.00;

	if (LCHECK ("uplift"))
	{
		if (SR._pricing_chk == FALSE)
			PriceProcess (line_cnt);

		base_uplift = 0.00;
		if (SR._cost_price)
			base_uplift = (SR._calc_sprice - SR._cost_price) 
						/ SR._cost_price;
		base_uplift *= 100.00;

		if (dflt_used)
		{
			local_rec.uplift = 0.00;
			if (SR._cost_price)
				local_rec.uplift = (ddln_rec.sale_price - SR._cost_price) 
								 / SR._cost_price;
			local_rec.uplift *= 100.00;
			local_rec.uplift = base_uplift;
		}

		if (local_rec.uplift == base_uplift)
			SR._keyed = 0;
		else
			SR._keyed = 1;

		SR._uplift = local_rec.uplift;

		ddln_rec.sale_price = no_dec (SR._cost_price * 
								      (1.00 + (local_rec.uplift / 100.00)));
		
		temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		MarginOk (ddln_rec.sale_price, ScreenDisc (ddln_rec.disc_pc),
				  local_rec.qty_sup, SR._cont_status,
				  local_rec.lcl_cst, SR._min_marg,FALSE);
	
		SR.total = local_rec.extend;
		line_display ();
		CalcTotals ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("terms"))
	{
		if (!strcmp (ddhr_rec.sell_terms, "FOB"))
		{
			scn_set (ORDSCN);
			for (i = 0; i < lcount [ORDSCN]; i++)
			{
				getval (i);

				local_rec.duty_val = 0.00;
				local_rec.loc_fi   = 0.00;
				store [i].amt_fai    = local_rec.loc_fi;
				
				CalcCosts (i);
				putval (i);
			}
			FLD ("duty_val") = NA;
			FLD ("loc_fi")   = NA;
			scn_set (HDRSCN);
			DSP_FLD ("terms");
			return (EXIT_SUCCESS);
		}
		if (!strcmp (ddhr_rec.sell_terms, "CIF"))
		{
			scn_set (ORDSCN);
			for (i = 0; i < lcount [ORDSCN]; i++)
			{
				getval (i);

				local_rec.duty_val = 0.00;
				store [i].imp_duty  = local_rec.duty_val;
				
				CalcCosts (i);
				putval (i);
			}
			FLD ("duty_val") = NA;
			FLD ("loc_fi")   = YES;
			scn_set (HDRSCN);
			DSP_FLD ("terms");
			return (EXIT_SUCCESS);
		}
		if (!strcmp (ddhr_rec.sell_terms, "DIS"))
		{
			FLD ("duty_val") = YES;
			FLD ("loc_fi")   = YES;
			DSP_FLD ("terms");
			return (EXIT_SUCCESS);
		}
		
		return (EXIT_FAILURE);
	}

    if (LCHECK ("fwd_exch"))
	{
		if (dflt_used)
		{
			strcmp (fehr_rec.cont_no, "      ");
			{
				if (FindPocr (cumr_rec.curr_code))
				{
					print_mess (ML (mlStdMess040));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.exch_rate = pocr_rec.ex1_factor;
			}
			strcpy (fehr_rec.cont_no, "      ");
			DSP_FLD ("exch_rate");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchFehr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		
		strcpy (fehr_rec.co_no,   comm_rec.co_no);
		strcpy (fehr_rec.cont_no, ddhr_rec.fwd_exch);
		cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (fehr_rec.curr_code, cumr_rec.curr_code))
		{
			print_mess (ML (mlDdMess080));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (fehr_rec.stat_flag [0] != 'A')
		{
			print_mess (ML (mlDdMess081));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-----------------------------------------
		| now see if contract is still current.    
		------------------------------------------*/
		if (fehr_rec.date_exp < ddhr_rec.dt_raised)
		{
			sprintf (err_str, 
					ML (mlDdMess082), 
					DateToString (fehr_rec.date_exp));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (fehr_rec.date_wef > ddhr_rec.dt_raised)
		{
			sprintf (err_str, 
					ML (mlDdMess083), 
					DateToString (fehr_rec.date_wef));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*------------------------------------- 
		| Load Composite Invoice Exchange Rate |
		--------------------------------------*/

		strcpy (crbk_rec.co_no,   comm_rec.co_no);
		strcpy (crbk_rec.bank_id, fehr_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess043));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (FindPocr (crbk_rec.curr_code))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.exch_rate = fehr_rec.exch_rate * pocr_rec.ex1_factor;
		DSP_FLD ("exch_rate");

		return (EXIT_SUCCESS);
	}


	if (LCHECK ("br_no"))
	{
		if (F_NOKEY (field))
		{
			strcpy (local_rec.br_no, comm_rec.est_no);
			return (EXIT_SUCCESS);
		}
		open_rec (esmr,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");

		if (dflt_used)
			strcpy (local_rec.br_no, comm_rec.est_no);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
	
		cc = find_rec (esmr,&esmr_rec,EQUAL,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			abc_fclose (esmr);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.br_name, esmr_rec.est_name);
		abc_fclose (esmr);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_no"))
	{
		if (F_NOKEY (field))
		{
			strcpy (local_rec.wh_no, comm_rec.cc_no);
			local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.wh_no, comm_rec.cc_no);

		if (hhcc_selected)
		{
			abc_selfield (ccmr,"ccmr_id_no");
			hhcc_selected = FALSE;
		}
		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,local_rec.br_no);
		strcpy (ccmr_rec.cc_no, local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.wh_name, ccmr_rec.name);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fix_exch"))
	{
		if (ddhr_rec.fix_exch [0] == 'Y')
			strcpy (local_rec.exch_desc, ML ("Yes"));
		else
			strcpy (local_rec.exch_desc, ML ("No "));
		
		DSP_FLD ("exch_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prog_inv"))
	{
		if (ddhr_rec.progressive [0] == 'Y')
			strcpy (local_rec.prog_desc, ML ("Yes"));
		else
			strcpy (local_rec.prog_desc, ML ("No "));
		
		DSP_FLD ("prog_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Price Input.	|
	-----------------------*/
	if (LCHECK ("sale_price")) 
	{
		if (dflt_used)
		{
			PriceProcess (line_cnt);
			SR._keyed = 0;
			local_rec.uplift = 0.00;
			if (local_rec.lcl_cst)
				local_rec.uplift = ((ddln_rec.sale_price /
									local_rec.exch_rate) - 
									local_rec.lcl_cst) /
									local_rec.lcl_cst;
			local_rec.uplift *= 100.00;
			SR._uplift = local_rec.uplift;
		}
		else
			SR._keyed = 2;

		if (ddln_rec.sale_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess031),"YyNn",1,2);
			box (0, 2, 132, 2);
			BusyFunc (0);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		if (SR._tax_amt == 0.00)
			store [line_cnt]._tax_amt = SR._cost_price;

		MarginOk (ddln_rec.sale_price, ScreenDisc (ddln_rec.disc_pc),
				  local_rec.qty_sup, SR._cont_status,
				  local_rec.lcl_cst, SR._min_marg,FALSE);

		if (local_rec.lcl_cst)
			local_rec.uplift = ((ddln_rec.sale_price /
								local_rec.exch_rate) - 
								local_rec.lcl_cst) /
								local_rec.lcl_cst;
		else
			local_rec.uplift = 0.00;

		local_rec.uplift *= 100.00;
		SR._uplift = local_rec.uplift;
									  
		temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;						  
								  
		SR.total = local_rec.extend;
		line_display ();
		CalcTotals ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sale_disc")) 
	{
		if (FLD ("sale_disc") == NI && prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (SR._dis_or, "N");
			DiscProcess (line_cnt);
		}
		
		if (SR._con_price || SR._cont_status == 2)
		{
			ddln_rec.disc_pc 	=	0.00;
			SR._disc_a			=	0.00;
			SR._disc_b			=	0.00;
			SR._disc_c			=	0.00;
			DSP_FLD ("sale_disc");
		}
		SR._dis_pc = ScreenDisc (ddln_rec.disc_pc);

		if (SR._calc_disc != ScreenDisc (ddln_rec.disc_pc))
			strcpy (SR._dis_or, "Y");

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
		MarginOk (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   local_rec.qty_sup, 
				   SR._cont_status,
				   local_rec.lcl_cst, 
				   SR._min_marg, 
				   FALSE);
								  
		temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
								 
		SR.total = local_rec.extend;
		line_display ();
		CalcTotals ();
	}

	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ("order_no")) 
	{
		if (!strcmp (local_rec.cust_no, "      "))
			abc_selfield (ddhr, "ddhr_id_no2");
		else
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (ddhr_rec.order_no,"00000000");
			DSP_FLD ("order_no");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (ddhr_rec.order_no,"%08ld", local_rec.dflt_order_no);
			DSP_FLD ("order_no");
		}

		/*-------------------------------
		| Maintaining Sales Orders	|
		-------------------------------*/
		if (SRCH_KEY)
		{
			SrchDdhr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (ddhr_rec.co_no,    comm_rec.co_no);
		strcpy (ddhr_rec.br_no,    comm_rec.est_no);
		strcpy (ddhr_rec.order_no, zero_pad (ddhr_rec.order_no,8));
		ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;

		newOrder = FALSE;
		cc = find_rec (ddhr, &ddhr_rec, EQUAL, "w");
		if (cc)
		{
			if (restart)
				return (EXIT_SUCCESS); 
				
			if (!strcmp (local_rec.cust_no, "      "))
			{
				print_mess (ML (mlStdMess021)); 
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (ddhr);
				return (EXIT_FAILURE); 
			}
			else
			{
				newOrder = 1;
				SetOrder (newOrder);
				return (EXIT_SUCCESS);
			}
		}
		local_rec.no_ctn		=	ddhr_rec.no_ctn;
		local_rec.wgt_per_ctn	=	ddhr_rec.wgt_per_ctn;

		if (!strcmp (local_rec.cust_no, "      "))
		{
			cumr2_rec.hhcu_hash = ddhr_rec.hhcu_hash;
			cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND");

			strcpy (local_rec.cust_no, cumr2_rec.dbt_no);
			DSP_FLD ("debtor");
			cc = spec_valid (label ("debtor"));
			if (cc)
				return (EXIT_FAILURE);
		}
		else
		if (cumr_rec.hhcu_hash != ddhr_rec.hhcu_hash)
		{
			sprintf (err_str, ML (mlDdMess084), ddhr_rec.order_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (ddhr);
			return (EXIT_FAILURE); 
		}

		if (!newOrder && strcmp (ddhr_rec.stat_flag, PENDINGFLAG))
		{
			sprintf (err_str, ML (mlDdMess040), ddhr_rec.order_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (ddhr);
			return (EXIT_FAILURE); 
		}

		if (strcmp (ddhr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no,   comm_rec.co_no);
			strcpy (cnch_rec.cont_no, ddhr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess075));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}


		if (strcmp (ddhr_rec.fwd_exch, "      "))
		{
			strcpy (fehr_rec.co_no,   comm_rec.co_no);
			strcpy (fehr_rec.cont_no, ddhr_rec.fwd_exch);
			cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess075));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			memcpy (&fehr2_rec, &fehr_rec, sizeof (fehr_rec));
		}
		else
			strcpy (fehr2_rec.cont_no, "      ");

		if (!strcmp (local_rec.cust_no, "      "))
			abc_selfield (ddhr, "ddhr_id_no2");

		local_rec.exch_rate = ddhr_rec.exch_rate;

		if (LoadItems (ddhr_rec.hhdd_hash))
		{
			scn_set (HDRSCN);
			restart = TRUE;
			return (EXIT_FAILURE);
		}

		if (ddhr_rec.fix_exch [0] == 'Y')
			strcpy (local_rec.exch_desc, ML ("Yes"));
		else
			strcpy (local_rec.exch_desc, ML ("No "));

		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no,    ddhr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cudp, "DBFIND");

		local_rec.date_reqd = ddhr_rec.dt_required;
		strcpy (local_rec.spinst [0], ddhr_rec.din_1);
		strcpy (local_rec.spinst [1], ddhr_rec.din_2);
		strcpy (local_rec.spinst [2], ddhr_rec.din_3);
		scn_display (HDRSCN);
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
			InmrSearch 
			(
				comm_rec.co_no, 
				temp_str,
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
			return (EXIT_SUCCESS);
		}
		cc	=	FindInmr 
				(
					comm_rec.co_no, 
					local_rec.item_no,
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
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
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			cc	=	FindInmr 
					(
						comm_rec.co_no, 
						local_rec.item_no,
						cumr_rec.hhcu_hash, 
						cumr_rec.item_codes
					);
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.item_no);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
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
			print_mess (ML (mlDdMess033));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (notax)
		{
			SR._tax_pc = 0.00;
			SR._gst_pc = 0.00;
			SR._tax_amt = 0.00;
		}
		else
		{
			SR._tax_pc = inmr_rec.tax_pc;
			SR._gst_pc = inmr_rec.gst_pc;
			SR._tax_amt = inmr_rec.tax_amount;
		}

		SR.hhbrHash = inmr_rec.hhbr_hash;
		SR.hhdlHash = -1;
		SR.hhplHash = -1;
		SR.line_num = -1;

		if (check_class (inmr_rec.inmr_class))
		{
			sprintf (err_str, ML (mlDdMess030) ,inmr_rec.item_no, inmr_rec.inmr_class);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (CheckReorder (inmr_rec.inmr_class))
		{
			sprintf (err_str, ML (mlDdMess029) ,inmr_rec.item_no, inmr_rec.inmr_class, clip (envVarPoReorder));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (SR._sellgrp, inmr_rec.sellgrp);
		DSP_FLD ("item_no");
		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		SR._outer = (double) inmr_rec.outer_size;

		inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
		strcpy (inei_rec.est_no, comm_rec.est_no);
		if (find_rec (inei,&inei_rec,EQUAL,"r"))
		{
			if (AddInei ())
			{
				errmess (ML (mlStdMess178));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		strcpy (SR._category, inmr_rec.category);
		strcpy (SR.item_desc, inmr_rec.description);
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,SR._category);
		cc = find_rec (excf,&excf_rec,EQUAL,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SR._min_marg = twodec (excf_rec.min_marg);

		/*----------------------------------------
		| Find part number for warehouse record. |
		----------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,EQUAL,"r");
		if (cc) 
		{
			sprintf (err_str, ML (mlStdMess033));
			i = prmptmsg (err_str,"YyNn",1,20);
			if (i == 'n' || i == 'N') 
			{
				move (0,20);
				cl_line ();
				line (132);
				skip_entry = -1 ;
				
				return (EXIT_SUCCESS); 
			}
			else 
			{
				cc = AddIncc ();
				if (cc)
					file_err (cc, incc, "DBADD");
				cl_line ();
				line_at (20,0,131);
			}
		}
		SR.hhccHash = incc_rec.hhcc_hash;

		strcpy (SR.supp_no, "      ");
		strcpy (SR.supp_name, forty_spaces);
		strcpy (SR.supp_curr, "   ");
		strcpy (SR.supp_uom, "EACH");
		strcpy (local_rec.supp_uom, "EACH");
		SR.supp_lead = 0.00;
		SR.supp_conv = 1.00;
		local_rec.supp_conv = 1.00;

		SR._cont_cost = FALSE;
		if (!strncmp (inmr_rec.item_no,"INDENT",6))
			SR._indent = TRUE;
		else
			SR._indent = FALSE;

		SR.no_inis = TRUE;
		SR.weight = 0.00;
		SR.volume = 0.00;
		SR.supp_lead = 0.00;
		SR.min_order = 0;
		SR.ord_multiple = 0;
		strcpy (SR._dis_or, "N");

		strcpy (local_rec.duty_code, inmr_rec.duty);
		if (SR._cont_cost == FALSE)
		{
			local_rec.fob_cost = 0.00;
			SR.base_cost = 0.00;
		}
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (SR.supp_uom, inum_rec.uom);
			strcpy (local_rec.supp_uom, inum_rec.uom);
			SR.supp_conv = 1.00;
			local_rec.supp_conv = 1.00;
		}

		if (local_rec.fob_cost == 0.00)
		{
			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no,comm_rec.est_no);
			cc = find_rec (inei,&inei_rec,EQUAL,"r");
			if (cc) 
			{
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		/*-------------------
		| Find duty record. |
		-------------------*/
		if (!strcmp (local_rec.duty_code, "  "))
		{
			SR.imp_duty = 0.00;
			strcpy (SR.duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code,  local_rec.duty_code);
			cc = find_rec (podt, &podt_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			SR.imp_duty = podt_rec.im_duty;
			strcpy (SR.duty_type, podt_rec.duty_type);
		}

		SR.upd_inis = FALSE;
		if (local_rec.fob_cost != 0.00)
			SR.cst_price = local_rec.fob_cost;
		else
		{
			SR.cst_price = inei_rec.last_cost * local_rec.supp_exch;
			SR.base_cost = SR.cst_price;
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		if (find_rec (inum, &inum_rec, EQUAL, "r"))
			strcpy (SR.std_uom, "    ");
		else
			strcpy (SR.std_uom, inum_rec.uom);
		strcpy (local_rec.std_uom, SR.std_uom);

		DSP_FLD ("cst_per");

		tab_other (line_cnt);
	}

	/*--------------------------
	| Validate Quantity input. |
	--------------------------*/
	if (LCHECK ("qty"))
	{
		if (dflt_used)
		{
			local_rec.qty = SR.min_order;
			SR.qty = SR.min_order;
		}

		if (newOrder || !envVarPoOverRide)
		{
			local_rec.qty = RndMltpl (local_rec.qty,
									   envVarSupOrdRound,
									   SR.ord_multiple,
									   SR.min_order);
		}

		if (SR._cont_cost == FALSE)
		{
			SR.cst_price = GetSupPrice (SR.hhsuHash,
									   SR.hhbrHash,
									   SR.base_cost,
									   local_rec.qty);
			SR.cumulative = GetSupDisc (SR.hhsuHash,
									   inmr_rec.buygrp,
									   local_rec.qty,
									   SR.discArray);
		}

		SR.qty = local_rec.qty;

		if (prog_status != ENTRY)
		{
			wrk_fob = SR.cst_price;

			if (SR._outer > 0.00)
				wrk_fob /= SR._outer;

			local_rec.grs_fgn = CENTS (wrk_fob);
			CalcCosts (line_cnt);
			if (SR.ddgd_ptr != DDGD_NULL)
			{
				LoadIntoCstScn ();
				putval (line_cnt);
				SpreadCosts (SR.hhsuHash);
				scn_display (ORDSCN);
			}
			line_display ();
		}

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate FOB (FGN). |
	--------------------*/
	if (LCHECK ("fob_cst"))
	{
		int		sav_line;
		/*------------------------------------------
		| SR.cst_price is in foreign currency and  |
		| standard UOM.                            |
		------------------------------------------*/
		wrk_fob = SR.cst_price;
		if (SR._outer > 0.00)
			wrk_fob /= SR._outer;

		if (SR._cont_cost == TRUE)
		{
			local_rec.grs_fgn = CENTS (wrk_fob);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			local_rec.grs_fgn = CENTS (wrk_fob);
		}

		if (local_rec.grs_fgn == 0.00)
		{
			i = prmptmsg (ML (mlStdMess121),"YyNn",1,2);
			BusyFunc (0);
			box (0, 2, 132, 2);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		move (0,23);
		cl_line ();

		if (wrk_fob != local_rec.grs_fgn)
		{
			move (0,23);
			cl_line ();
			if (SR.no_inis == FALSE)
			{
				/*---------------
				| Prompt		|
				---------------*/
				if (up_inis == -1)
				{
					SR.upd_inis = prmpt_inis (0,23);
				}
				else
					SR.upd_inis = up_inis;
			}
		}

		if (prog_status != ENTRY)
		{
			local_rec.net_fob = CalcNet (local_rec.grs_fgn, 
										 SR.discArray, 
										 SR.cumulative);
			SR.net_fob = local_rec.net_fob;
			CalcCosts (line_cnt);
			sav_line = line_cnt;
			if (SR.ddgd_ptr != DDGD_NULL)
			{
				putval (line_cnt);
				CalcItemTotals (SR.hhsuHash);
				SpreadCosts (SR.hhsuHash);
				line_cnt = sav_line;
				getval (line_cnt);
				scn_display (ORDSCN);
			}
			line_display ();
			DSP_FLD ("net_fob");
		}
		else
			putval (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate View Discount. |
	-------------------------*/
	if (LCHECK ("view_disc"))
	{
		if (SR._cont_cost == TRUE)
		{
			strcpy (local_rec.view_disc, "N");
			DSP_FLD ("view_disc");
			return (EXIT_SUCCESS);
		}

		if (local_rec.view_disc [0] == 'Y')
		{
			int	temp = lcount [ORDSCN];

#ifdef GVISION
			discRec.grossPrice		= DOLLARS (local_rec.grs_fgn);
			discRec.discArray [0]	= SR.discArray [0];
			discRec.discArray [1]	= SR.discArray [1];
			discRec.discArray [2]	= SR.discArray [2];
			discRec.discArray [3]	= SR.discArray [3];

			ViewDiscounts (DBOX_LFT, DBOX_TOP, SR.cumulative);

			local_rec.grs_fgn	= CENTS (discRec.grossPrice);
			SR.discArray [0]	= discRec.discArray [0];
			SR.discArray [1]	= discRec.discArray [1];
			SR.discArray [2]	= discRec.discArray [2];
			SR.discArray [3]	= discRec.discArray [3];
#else
			ViewDiscounts ();
#endif	/* GVISION */

			/*-----------------
			| Redraw screens. |
			-----------------*/
			putval (line_cnt);
			scn_write (ORDSCN);

			lcount [ORDSCN] = (prog_status == ENTRY) ? line_cnt + 1 
													: lcount [ORDSCN];
			scn_display (ORDSCN);
			lcount [ORDSCN] = temp;
		}
		else
		{
			local_rec.net_fob = CalcNet (local_rec.grs_fgn, 
										 SR.discArray, 
										 SR.cumulative);
			SR.net_fob = local_rec.net_fob;
		}
		CalcCosts (line_cnt);
		if (prog_status == ENTRY)
		{
			lcount [ORDSCN]++;
			if (SR.ddgd_ptr != DDGD_NULL)
			{
				LoadIntoCstScn ();
				putval (line_cnt);
				SpreadCosts (SR.hhsuHash);
			}
			LoadIntoPriScn ();
			lcount [ORDSCN]--;
			scn_display (ORDSCN);
		}
		line_display ();
		DSP_FLD ("net_fob");
	}

	/*--------------------
	| Validate F+I (LOC). |
	--------------------*/
	if (LCHECK ("loc_fi"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);
		
		if (dflt_used)
			local_rec.loc_fi = FreightCalc (line_cnt);
		
		SR.amt_fai = local_rec.loc_fi;
		CalcCosts (line_cnt);
		line_display ();

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Duty      |
	--------------------*/
	if (LCHECK ("duty_val"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);
		
		if (dflt_used)
			local_rec.duty_val = DutyCalc (line_cnt);

		CalcCosts (line_cnt);

		SR.amt_dty = local_rec.duty_val;

		/*-----------------
		| recalc dty pc
		-----------------*/
		line_display ();

		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Other Costs|
	---------------------*/
	if (LCHECK ("other"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);
		
		if (dflt_used)
			local_rec.oth = 0.00;

		CalcCosts (line_cnt);

		SR.amt_oth = local_rec.oth;

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("debtor")) 
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, envVarDbCo, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.cust_no, "      "))
		{
			strcpy (local_rec.cust_no, "      ");
			DSP_FLD ("debtor");
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,envVarDbCo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
		cc = find_rec (cumr,&cumr_rec,EQUAL,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		recalc_sobg ();

		/*--------------------------------------
		| Check if customer is on stop credit. |
		--------------------------------------*/
		if (cumr_rec.stop_credit [0] == 'Y')
		{
			if (envVarStopCrd)
			{
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				strcpy (err_str, ML (mlStdMess060));
				cc = WarningUser (err_str,0,2);
				if (cc)
					return (cc);
			}
		}

		/*---------------------------------------------
		| Check if customer is over his credit limit. |
		---------------------------------------------*/
		if (CheckCumr (0.00, FALSE))
		{
			if (envVarCrdOver)
			{
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str, ML (mlStdMess061));
				cc = WarningUser (err_str,0,2);
				if (cc)
					return (EXIT_FAILURE);

			}
		}
		/*-----------------------
		| Check Credit Terms	|
		-----------------------*/
		if (cumr_rec.od_flag)
		{
			if (envVarCrdTerm)
			{
				sprintf (err_str, ML (mlStdMess062) , cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str, ML (mlStdMess062) , cumr_rec.od_flag);
				cc = WarningUser (err_str,0,2);
				if (cc)
					return (EXIT_FAILURE);
			}
			
		}
		strcpy (local_rec.ho_cust_no, "N/A   ");
		if (cumr_rec.ho_dbt_hash != 0L)
		{
			cumr2_rec.hhcu_hash	=	cumr_rec.ho_dbt_hash;
		    cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		    if (!cc)
				strcpy (local_rec.ho_cust_no, cumr2_rec.dbt_no);
		}

		LoadIntoTlrScn ();
		DSP_FLD ("name");
		DSP_FLD ("cus_addr1");
		DSP_FLD ("cus_addr2");
		DSP_FLD ("cus_addr3");
		DSP_FLD ("cus_addr4");

		use_window (FN14);
		if (strlen (clip (ccmr_rec.sman_no)))
			strcpy (local_rec.dflt_sale_no, ccmr_rec.sman_no);
		else
			strcpy (local_rec.dflt_sale_no, cumr_rec.sman_code);
		
		/*-------------------------------
		| if cus_ord_ref must be input	|
		-------------------------------*/
		if (cumr_rec.po_flag [0] == 'Y')
			FLD ("cus_ord_ref") = YES;
		else
			FLD ("cus_ord_ref") = (F_NOKEY (label ("cus_ord_ref"))) ? NA : YES;

		/*------------
		| sort out tax 
		--------------*/
		strcpy (ddhr_rec.tax_code, cumr_rec.tax_code);
		if (ddhr_rec.tax_code [0] == 'A' || 
			 ddhr_rec.tax_code [0] == 'B')
			notax = 1;
		else
			notax = 0;

		/*--------------------------
		| Get customer price type. |
		--------------------------*/
		if (cumr_rec.price_type [0] == '1')
			strcpy (local_rec.pri_desc, comm_rec.price1_desc);
		else if (cumr_rec.price_type [0] == '2')
			strcpy (local_rec.pri_desc, comm_rec.price2_desc);
		else if (cumr_rec.price_type [0] == '3')
			strcpy (local_rec.pri_desc, comm_rec.price3_desc);
		else if (cumr_rec.price_type [0] == '4')
			strcpy (local_rec.pri_desc, comm_rec.price4_desc);
		else if (cumr_rec.price_type [0] == '5')
			strcpy (local_rec.pri_desc, comm_rec.price5_desc);
		else if (cumr_rec.price_type [0] == '6')
			strcpy (local_rec.pri_desc, comm_rec.price6_desc);
		else if (cumr_rec.price_type [0] == '7')
			strcpy (local_rec.pri_desc, comm_rec.price7_desc);
		else if (cumr_rec.price_type [0] == '8')
			strcpy (local_rec.pri_desc, comm_rec.price8_desc);
		else if (cumr_rec.price_type [0] == '9')
			strcpy (local_rec.pri_desc, comm_rec.price9_desc);

		/*--------------------------
		| Get customer price type. |
		--------------------------*/
		DSP_FLD ("pri_type");

		/*----------------------------------
		| Get currency code and exch rate. |
		----------------------------------*/
		if (envVarDbMcurr)
		{
			if (FindPocr (cumr_rec.curr_code))
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sprintf (sel_prmt,"Sell(%-3.3s)",	cumr_rec.curr_code);
			sprintf (ext_prmt," Ext(%-3.3s) ",	cumr_rec.curr_code);
		}
		else
		{
			local_rec.exch_rate = 1.00;
			FLD ("exch_rate") = ND;
		}
		DSP_FLD ("exch_rate");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("exch_rate")) 
	{
		if (dflt_used)
		{
			if (FindPocr (cumr_rec.curr_code))
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			local_rec.exch_rate = pocr_rec.ex1_factor;
		}
		if (prog_status != ENTRY)
		{
			LoadIntoPriScn ();
		}

		DSP_FLD ("exch_rate");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("date_reqd")) 
	{
		strcpy (local_rec.dflt_date_due, DateToString (local_rec.date_reqd));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cus_ord_ref")) 
	{
		if (!strcmp (ddhr_rec.cus_ord_ref,twenty_spaces))
		{
			if (cumr_rec.po_flag [0] == 'Y')
			{
				print_mess (ML (mlDdMess034));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| validate debtors contract
	-------------------------*/
	if (LCHECK ("cont_no"))
	{
		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			return (EXIT_SUCCESS);
		
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, ddhr_rec.cont_no);
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
		if (cnch_rec.date_exp < ddhr_rec.dt_raised)
		{
			sprintf (err_str, 
					ML (mlDdMess082), 
					DateToString (cnch_rec.date_exp));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cnch_rec.date_wef > ddhr_rec.dt_raised)
		{
			sprintf (err_str, 
					ML (mlDdMess083), 
					DateToString (cnch_rec.date_wef));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*-----------------------------------------
		| now see if contract is assigned to debtor
		------------------------------------------*/
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlDdMess085));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.cont_desc, cnch_rec.desc);
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supp"))
	{
		abc_selfield (sumr, (envVarCrFind) ? "sumr_id_no3" : "sumr_id_no");

		if (dflt_used)
		{
			cc = GetDefaultSupplier (SR.hhbrHash);
			if (cc)
			{
				print_mess (ML (mlStdMess007));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
				strcpy (local_rec.supp_no, sumr_rec.crd_no);
		}

		if (SRCH_KEY)
		{
			if (!FindInis (SR.hhbrHash, 0))
				SrchInis ();
			else
				SumrSearch (comm_rec.co_no, envVarCrCo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, envVarCrCo);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supp_no));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
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

		local_rec.supp_exch = pocr_rec.ex1_factor;

		strcpy (SR.supp_no,		sumr_rec.crd_no);
		strcpy (SR.supp_name,	sumr_rec.crd_name);
		strcpy (SR.supp_curr,	sumr_rec.curr_code);

		SR.hhsuHash = sumr_rec.hhsu_hash;
		SR.supp_exch  = pocr_rec.ex1_factor;

		/*--------------------
		| Find freight file. |
		--------------------*/
		if (FindPocf (sumr_rec.ctry_code))
			return (EXIT_FAILURE);


		if (FindInis (SR.hhbrHash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			SR.no_inis = TRUE;
			SR.weight = inmr_rec.weight;
			SR.volume = 0.00;
			SR.supp_lead = 0.00;
			SR.min_order = 0;
			SR.ord_multiple = 0;

			strcpy (local_rec.duty_code, inmr_rec.duty);
				/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no,comm_rec.est_no);
			cc = find_rec (inei,&inei_rec,EQUAL,"r");
			if (cc) 
				file_err (cc, inei,"DBFIND");

			if (SR._cont_cost == FALSE)
			{
				SR.base_cost = inei_rec.last_cost * SR.supp_exch;
				local_rec.fob_cost = SR.base_cost;
			}
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (SR.supp_uom, inum_rec.uom);
				strcpy (local_rec.supp_uom, inum_rec.uom);
				SR.supp_conv = 1.00;
				local_rec.supp_conv = 1.00;
			}
			SR.supp_lead = 0.00;
		}
		else
		{
			SR.no_inis 		= FALSE;
			SR.weight 		= inis_rec.weight;
			SR.volume 		= inis_rec.volume;
			SR.supp_lead 	= inis_rec.lead_time;
			SR.min_order 	= inis_rec.min_order;
			SR.ord_multiple = inis_rec.ord_multiple;

			strcpy (local_rec.duty_code, inis_rec.duty);
			strcpy (inmr_rec.duty, inis_rec.duty);
			if (SR._cont_cost == FALSE)
			{
				local_rec.fob_cost = inis_rec.fob_cost;
				SR.base_cost = inis_rec.fob_cost;
			}
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (SR.supp_uom, inum_rec.uom);
				strcpy (local_rec.supp_uom, inum_rec.uom);
				SR.supp_conv = inis_rec.pur_conv;
				local_rec.supp_conv = inis_rec.pur_conv;
			}
		}

		if (SR.supp_conv == 0.00)
			SR.supp_conv = 1.00;

		if (SR.no_inis == TRUE)
		{
			move (0,23);
			cl_line ();
			sprintf (err_str, ML (mlDdMess086) , local_rec.item_no, local_rec.supp_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			SR.upd_inis = 0;
		}
		else
		{
			move (0,23);
			cl_line ();
		}

		/*------------------------
		| get contract price ???
		------------------------*/
		if (cnch_rec.hhch_hash != 0L)
		{
			/*--------------------------------------------------------
			| Use ContCusPrice to determine if a valid contract line |
			| is available for this line. If so then _cont_status    |
			| will be non-zero and the appropriate cncd record will  |
			| be loaded.                                             |
			--------------------------------------------------------*/
			(void) ContCusPrice (cnch_rec.hhch_hash,
								 SR.hhbrHash,
								 SR.hhccHash,
								 cumr_rec.curr_code,
								 cnch_rec.exch_type,
								 FGN_CURR,
								 (float)local_rec.exch_rate);
			if (_cont_status)
			{
				if (cncd_rec.cost > 0.00 && 
					cncd_rec.hhsu_hash == sumr_rec.hhsu_hash)
				{
					local_rec.fob_cost = DOLLARS (cncd_rec.cost);
					SR.base_cost = local_rec.fob_cost;
					SR.cst_price = SR.base_cost;
					SR._cont_cost = TRUE;
				}
			}
		}

		/*-------------------------------------
		| Find new duty record based of inis. |
		-------------------------------------*/
		if (!strcmp (local_rec.duty_code, "  "))
		{
			SR.imp_duty = 0.00;
			strcpy (SR.duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code,  local_rec.duty_code);
			cc = find_rec (podt, &podt_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			SR.imp_duty = podt_rec.im_duty;
			strcpy (SR.duty_type, podt_rec.duty_type);
		}
		SR.ddgd_ptr = DDGD_NULL;

		DispCustSupp (line_cnt);

		for (i = 0; i < line_cnt; i++)
		{
			if (SR.hhsuHash == store [i].hhsuHash)
			{
				SR.ddgd_ptr = store [i].ddgd_ptr;
				break;
			}
			
		}
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}
		
	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dp_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (ddhr_rec.dp_no, cumr_rec.department);

		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no,    ddhr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		PrintCompanyStuff ();
		DSP_FLD ("dp_no");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Processed Date. |
	--------------------------*/
	if (LCHECK ("date_raised")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			ddhr_rec.dt_raised = StringToDate (local_rec.dbt_date);
			DSP_FLD ("date_raised");
		}

		
		if (chq_date (ddhr_rec.dt_raised,comm_rec.dbt_date))
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sale_code")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy (ddhr_rec.sman_code, local_rec.dflt_sale_no);
			DSP_FLD ("sale_code");
			return (EXIT_SUCCESS);
		}

		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (ddhr_rec.sman_code, local_rec.dflt_sale_no);
			DSP_FLD ("sale_code");
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, ddhr_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}

		abc_fclose (exsf);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pri_type")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			strcpy (local_rec.pri_desc, cumr_rec.price_type);

		if (SRCH_KEY)
		{
			SrchPrices ();
			strcpy (local_rec.pri_desc,"               ");
			return (EXIT_SUCCESS);
		}
		sprintf (ddhr_rec.pri_type,"%-1.1s",local_rec.pri_desc);
		if (ddhr_rec.pri_type [0] == '1')
			strcpy (local_rec.pri_desc, comm_rec.price1_desc);
		else if (ddhr_rec.pri_type [0] == '2')
			strcpy (local_rec.pri_desc, comm_rec.price2_desc);
		else if (ddhr_rec.pri_type [0] == '3')
			strcpy (local_rec.pri_desc, comm_rec.price3_desc);
		else if (ddhr_rec.pri_type [0] == '4')
			strcpy (local_rec.pri_desc, comm_rec.price4_desc);
		else if (ddhr_rec.pri_type [0] == '5')
			strcpy (local_rec.pri_desc, comm_rec.price5_desc);
		else if (ddhr_rec.pri_type [0] == '6')
			strcpy (local_rec.pri_desc, comm_rec.price6_desc);
		else if (ddhr_rec.pri_type [0] == '7')
			strcpy (local_rec.pri_desc, comm_rec.price7_desc);
		else if (ddhr_rec.pri_type [0] == '8')
			strcpy (local_rec.pri_desc, comm_rec.price8_desc);
		else if (ddhr_rec.pri_type [0] == '9')
			strcpy (local_rec.pri_desc, comm_rec.price9_desc);
		
		DSP_FLD ("pri_type");
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
			SrchPayTerms ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (ddhr_rec.pay_term,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (ddhr_rec.pay_term,"%-40.40s",p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_term");

		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Default Exchange Rate. |
	------------------------*/
	if (LCHECK ("po_exch"))
	{
		if (dflt_used)
		{
			local_rec.supp_exch = pocr_rec.ex1_factor;
			DSP_FLD ("po_exch");
		}

		if (local_rec.supp_exch == 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			int	line_no;
			scn_set (ORDSCN);
			for (line_no = 0;line_no < lcount [ORDSCN];line_no++) 
			{
				getval (line_no);
				if (store [line_no].duty_type [0] != 'D')
				{
					local_rec.duty_val = DutyCalc (line_no);
				}
				
				CalcCosts (line_no);
				putval (line_no);
			}
			scn_set (HDRSCN);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Shipment Name And Addresses. |
	---------------------------------------*/
	if (LCHECK ("shipname") || 
	    LNCHECK ("shipaddr",8)) 
	{
		if (SRCH_KEY)
		{
			open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");

			i = SrchCudi (field - label ("shipname"));

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (ddhr_rec.del_name,cudi_rec.name);
			strcpy (ddhr_rec.del_add1, cudi_rec.adr1);
			strcpy (ddhr_rec.del_add2, cudi_rec.adr2);
			strcpy (ddhr_rec.del_add3, cudi_rec.adr3);
		}
		CatIntoPohr ();
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("spcode0") || LCHECK ("spcode1") || LCHECK ("spcode2"))
	{
		i = field - label ("spcode0") ;

		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi ();
			abc_fclose (exsi);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comm_rec.co_no);

		if (i == 0)
		{
			exsi_rec.inst_code = (dflt_used) ? cumr_rec.inst_fg1 
										: atoi (local_rec.spinst [0]);
		}
		if (i == 1)
		{
			exsi_rec.inst_code = (dflt_used) ? cumr_rec.inst_fg2 
										: atoi (local_rec.spinst [1]);
		}
		if (i == 2)
		{
			exsi_rec.inst_code = (dflt_used) ? cumr_rec.inst_fg2 
										: atoi (local_rec.spinst [2]);
		}

		if (!find_rec (exsi,&exsi_rec, EQUAL,"r"))
			sprintf (local_rec.spinst [i],"%-60.60s",exsi_rec.inst_text);
		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate Currency	|
	-----------------------*/
	if (LCHECK ("currency"))
	{
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			strcpy (local_rec.cst_curr, currencyCode);
	
		if (strcmp (local_rec.cst_curr,"   "))
		{
			cc = FindPocr (local_rec.cst_curr);
			if (cc)
			{
				errmess (ML (mlStdMess040));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			local_rec.cst_exch = pocr_rec.ex1_factor;
		}
		else
		{
			local_rec.cst_exch = 1.00;
		}

		if (!strcmp (local_rec.cst_spread, " "))
		{
			strcpy (local_rec.cst_spread, "D");
			DSP_FLD ("spread");
		}

		DSP_FLD ("cst_exch");

		local_rec.cst_loc_val = local_rec.cst_fgn_val;
		if (local_rec.cst_exch != 0.00)
			local_rec.cst_loc_val /= local_rec.cst_exch;

		DSP_FLD ("loc_val");
	
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate FGN_VAL	|
	-----------------------*/
	if (LCHECK ("fgn_val"))
	{
		if (dflt_used)
		{
			if (line_cnt == 0)
			{
				CalcCostTotals (sumr_rec.hhsu_hash);
				local_rec.cst_fgn_val = fob_tot;
			}
		}
		if (!strcmp (local_rec.cst_spread, " "))
		{
			strcpy (local_rec.cst_spread, "D");
			DSP_FLD ("spread");
		}

		if (!strcmp (local_rec.cst_curr, "   "))
		{
			strcpy (local_rec.cst_curr, currencyCode);
			local_rec.cst_exch = 1.00;
			DSP_FLD ("currency");
		}

		local_rec.cst_loc_val = local_rec.cst_fgn_val;
		if (local_rec.cst_exch != 0.00)
			local_rec.cst_loc_val /= local_rec.cst_exch;

		DSP_FLD ("fgn_val");
		DSP_FLD ("loc_val");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Cost Exchange Rate	|
	-------------------------------*/
	if (LCHECK ("cst_exch")) 
	{
		if (dflt_used)
		{
			if (strcmp (local_rec.cst_curr,"   "))
			{
				cc = FindPocr (local_rec.cst_curr);
				if (cc)
				{
					errmess (ML (mlStdMess040));
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}

				local_rec.cst_exch = pocr_rec.ex1_factor;
			}
			else
			{
				strcpy (local_rec.cst_curr, currencyCode);
				local_rec.cst_exch = 1.00;
			}
		}
	
		if (!strcmp (local_rec.cst_spread, " "))
		{
			strcpy (local_rec.cst_spread, "D");
			DSP_FLD ("spread");
		}

		if (!strcmp (local_rec.cst_curr, "   "))
		{
			strcpy (local_rec.cst_curr, currencyCode);
			DSP_FLD ("currency");
		}

		local_rec.cst_loc_val = local_rec.cst_fgn_val;
		if (local_rec.cst_exch != 0.00)
			local_rec.cst_loc_val /= local_rec.cst_exch;

		DSP_FLD ("currency");
		DSP_FLD ("loc_val");

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Shipment Method. |
	---------------------------*/
	if (LCHECK ("ship_method"))
	{
		switch (dflt_ship_rec.ship_method [0])
		{
			case 'L' :	strcpy (dflt_ship_rec.desc_method, ML ("Land"));
						break;
			case 'S' :	strcpy (dflt_ship_rec.desc_method, ML ("Sea "));
						break;
			case 'A' :	strcpy (dflt_ship_rec.desc_method, ML ("Air "));
						break;
			default	 :  print_mess (ML (mlStdMess137));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
		}
		DSP_FLD ("desc_method");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
GetDefaultSupplier (
 long		HHBR_HASH)
{
	long	HHSU_HASH = 0L;
	int		wsPriority	=	0;
	int		lowestPriority = 27;
	double	lowestCost		= 999999999.99;

	abc_selfield (sumr, "sumr_hhsu_hash");

	inis_rec.hhbr_hash = HHBR_HASH;
	inis_rec.hhsu_hash = 0L;
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc =  find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == HHBR_HASH)
	{
		if (envVarDdCrDefault == PRIORITY)
		{
			if (inis_rec.sup_priority [0] == 'C')
				wsPriority = atoi (inis_rec.sup_priority + 1) + 18;
			if (inis_rec.sup_priority [0] == 'B')
				wsPriority = atoi (inis_rec.sup_priority + 1) + 9;
			if (inis_rec.sup_priority [0] == 'W')
				wsPriority = atoi (inis_rec.sup_priority + 1);

			if (wsPriority < lowestPriority)
			{
				HHSU_HASH = inis_rec.hhsu_hash;
				lowestPriority = wsPriority;
			}
		}
		else
		{
			if (inis_rec.fob_cost < lowestCost)
			{
				HHSU_HASH = inis_rec.hhsu_hash;
				lowestCost = inis_rec.fob_cost;
			}
		}
		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}

	if (HHSU_HASH > 0L)
	{
		abc_selfield (sumr, "sumr_hhsu_hash");
		sumr_rec.hhsu_hash	=	HHSU_HASH;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
		if (!cc)
			return (EXIT_SUCCESS);
		else
			return (EXIT_FAILURE);
	}
	return (EXIT_FAILURE);
}

void
SetOrder (
 int    new_ord)
{
	int			i;

	if (ddhr_rec.pri_type [0] == '1')
		strcpy (local_rec.pri_desc, comm_rec.price1_desc);
	else if (ddhr_rec.pri_type [0] == '2')
		strcpy (local_rec.pri_desc, comm_rec.price2_desc);
	else if (ddhr_rec.pri_type [0] == '3')
		strcpy (local_rec.pri_desc, comm_rec.price3_desc);
	else if (ddhr_rec.pri_type [0] == '4')
		strcpy (local_rec.pri_desc, comm_rec.price4_desc);
	else if (ddhr_rec.pri_type [0] == '5')
		strcpy (local_rec.pri_desc, comm_rec.price5_desc);
	else if (ddhr_rec.pri_type [0] == '6')
		strcpy (local_rec.pri_desc, comm_rec.price6_desc);
	else if (ddhr_rec.pri_type [0] == '7')
		strcpy (local_rec.pri_desc, comm_rec.price7_desc);
	else if (ddhr_rec.pri_type [0] == '8')
		strcpy (local_rec.pri_desc, comm_rec.price8_desc);
	else if (ddhr_rec.pri_type [0] == '9')
		strcpy (local_rec.pri_desc, comm_rec.price9_desc);
	
	if (new_ord)
	{
		GetWarehouse (0L);

		init_vars (3);	
		init_vars (4);	

		strcpy (ddhr_rec.tax_code,	cumr_rec.tax_code);
		strcpy (ddhr_rec.tax_no,	cumr_rec.tax_no);
		strcpy (ddhr_rec.area_code,	cumr_rec.area_code);
		strcpy (ddhr_rec.sman_code,	local_rec.dflt_sale_no);
		strcpy (ddhr_rec.pri_type,	cumr_rec.price_type);

		/*--------------------------------
		| Get any special instrunctions. |
		--------------------------------*/
		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg1;
		cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
		sprintf (local_rec.spinst[0],"%60.60s",(cc) ? " " : exsi_rec.inst_text);

		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg2;
		cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
		sprintf (local_rec.spinst[1],"%60.60s",(cc) ? " " : exsi_rec.inst_text);

		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = cumr_rec.inst_fg3;
		cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
		sprintf (local_rec.spinst[2],"%60.60s",(cc) ? " " : exsi_rec.inst_text);

		abc_fclose (exsi);

		/*------------------------
		| Get charge to address. |
		------------------------*/
		strcpy (ddhr_rec.del_add1, cumr_rec.dl_adr1);
		strcpy (ddhr_rec.del_add2, cumr_rec.dl_adr2);
		strcpy (ddhr_rec.del_add3, cumr_rec.dl_adr3);

		strcpy  (ddhr_rec.cus_ord_ref,	twenty_spaces);
		sprintf (ddhr_rec.op_id, 		"%-14.14s", currentUser);
		
		ddhr_rec.date_create = TodaysDate ();
		strcpy (ddhr_rec.time_create, TimeHHMM());

		strcpy  (ddhr_rec.area_code,	cumr_rec.area_code);
		strcpy  (ddhr_rec.sman_code,	local_rec.dflt_sale_no);
		strcpy  (ddhr_rec.pri_type,		cumr_rec.price_type);
		strcpy  (ddhr_rec.sell_terms,	"   ");
		sprintf (ddhr_rec.pay_term,	"%-40.40s",	cumr_rec.crd_prd);

		ddhr_rec.dt_raised   = comm_rec.dbt_date;
		ddhr_rec.dt_required = TodaysDate ();

		for (i = 0; strlen (p_terms [i]._pcode); i++)
		{
			if (!strncmp (ddhr_rec.pay_term, p_terms [i]._pcode,
						  strlen (p_terms [i]._pcode)))
			{
				sprintf (ddhr_rec.pay_term, "%-40.40s", p_terms [i]._pterm);
				break;
			}
		}
	}
	else
	{
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no,    ddhr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, EQUAL,"r");
		if (cc)
			sprintf (cudp_rec.dp_name, "%40.40s", " ");
		strcpy (local_rec.ord_desc, "        ");
	}
	for (i = 0; strlen (s_terms [i]._scode); i++)
	{
		if (!strncmp (ddhr_rec.sell_terms, s_terms [i]._scode,
					  strlen (s_terms [i]._scode)))
		{
			sprintf (local_rec.sell_desc, "%-30.30s", s_terms [i]._sterm);
			break;
		}
	}	
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, ddhr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
	if (cc)
		sprintf (exsf_rec.salesman, "%40.40s", " ");

	abc_fclose (exsf);

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code,  ddhr_rec.area_code);
	cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
	if (cc)
		sprintf (exaf_rec.area_code, "%40.40s", " ");

	strcpy  (cfhr_rec.carr_code, "    ");
	sprintf (cfhr_rec.carr_desc, "%40.40s", " ");
	strcpy  (local_rec.carr_area,   ddhr_rec.carr_area);

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code,  ddhr_rec.area_code);
	if (!find_rec (exaf, &exaf_rec, EQUAL, "r"))
	{
		strcpy (local_rec.carr_area, exaf_rec.area_code);
	}
	else
	{
		strcpy (local_rec.carr_area, "  ");
	}
	strcpy (fehr2_rec.cont_no, "      ");

	scn_set (HDRSCN);
}

/*=======================================================================
| Routine to read all ddln records whose hash matches the one on the ddhr
=======================================================================*/
int
LoadItems (
	long   hhddHash)
{
	int		i;
	int		noShpScn 	= TRUE;
	double	temp_gross 	= 0.00,
			temp_dis 	= 0.00;			

	/*--------------------------
	| Set PRISCN - for putval. |
	--------------------------*/
	scn_set (PRISCN);
	lcount [PRISCN] = 0;
	lcount [ORDSCN] = 0;

	BusyFunc (1);
	move (10, 3);

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (ddsh,"ddsh_hhds_hash");

	ddln_rec.hhdd_hash = hhddHash;
	ddln_rec.line_no = 0;

	if (ddhr_rec.progressive [0] == 'Y')
		strcpy (local_rec.prog_desc, ML ("Yes"));
	else
		strcpy (local_rec.prog_desc, ML ("No "));
	
	cc = find_rec (ddln,&ddln_rec,GTEQ,"r");

	while (!cc && hhddHash == ddln_rec.hhdd_hash) 
	{
		store [lcount [PRISCN]]._cont_status = ddln_rec.cont_status;
		store [lcount [PRISCN]].line_num = ddln_rec.line_no;
		store [lcount [PRISCN]].hhdlHash = ddln_rec.hhdl_hash;
		store [lcount [PRISCN]].hhdsHash = ddln_rec.hhds_hash;
		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	ddln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc) 
			file_err (cc, inmr, "DBFIND");

		/*-------------------------
		| Check for Indent items. |
		-------------------------*/
		if (!strncmp (inmr_rec.item_no,"INDENT",6))
			store [PRISCN]._indent = TRUE;
		else
			store [PRISCN]._indent = FALSE;

		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (store [lcount [PRISCN]]._sellgrp, inmr_rec.sellgrp);
		strcpy (store [lcount [PRISCN]].item_desc, inmr_rec.description);
		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		store [lcount [PRISCN]]._outer = (double) inmr_rec.outer_size;
	
		inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						  				  inmr_rec.hhsi_hash);
		strcpy (inei_rec.est_no, ddhr_rec.br_no);
		cc = find_rec (inei, &inei_rec,  EQUAL, "r");
		if (cc)
			file_err (cc, inei, "DBFIND");

		store [lcount [PRISCN]].hhbrHash = ddln_rec.hhbr_hash;
		store [lcount [PRISCN]].hhsuHash = ddln_rec.hhsu_hash;
		store [lcount [PRISCN]].hhplHash = ddln_rec.hhpl_hash;

		strcpy (excf_rec.co_no,  comm_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
			file_err (cc, excf, "DBFIND");

		store [lcount [PRISCN]]._min_marg = twodec (excf_rec.min_marg);
		store [lcount [PRISCN]]._outer    = inmr_rec.outer_size;
		store [lcount [PRISCN]]._class [0] = inmr_rec.inmr_class [0];
		strcpy (store [lcount [PRISCN]]._category,     inmr_rec.category);
		strcpy (store [lcount [PRISCN]]._costing_flag, inmr_rec.costing_flag);

		store [lcount [PRISCN]]._weight = inmr_rec.weight;
			
		local_rec.qty = local_rec.qty_sup = ddln_rec.q_order;
		store [lcount [PRISCN]].qty  = ddln_rec.q_order;

		store [lcount [PRISCN]]._dis_pc  = ddln_rec.disc_pc;
		store [lcount [PRISCN]]._gst_pc  = ddln_rec.gst_pc;
		store [lcount [PRISCN]]._tax_pc  = ddln_rec.tax_pc;
		store [lcount [PRISCN]]._tax_amt = inmr_rec.tax_amount;

		store [lcount [PRISCN]]._gsale_price = ddln_rec.gsale_price;

		ddln_rec.disc_pc = ScreenDisc (ddln_rec.disc_pc);
		/*----------------------
		| store the information
		| do some other working
		| and then retreive record
		-------------------------*/
		putval (lcount [PRISCN]);

		if (LoadPoln (ddln_rec.hhpl_hash))
		{
			return (TRUE);
		}

		UpdateLine (lcount [PRISCN]);

		/*------------------------------------------
		| Load any on-cost details if new shipment |
		| otherwise set pointer to on-cost struct  |
		------------------------------------------*/
		ddsh_rec.hhds_hash = ddln_rec.hhds_hash;
		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "u");

		if (!cc && noShpScn)
		{
			dflt_ship_rec.date_load =	ddsh_rec.date_load;
			dflt_ship_rec.date_depart =	ddsh_rec.ship_depart;
			dflt_ship_rec.date_arrive =	ddsh_rec.ship_arrive;
			strcpy (dflt_ship_rec.ship_method,	ddsh_rec.ship_method);
			switch (ddsh_rec.ship_method [0])
			{
				case 'L' :	strcpy (dflt_ship_rec.desc_method, ML ("Land"));
							break;
				case 'S' :	strcpy (dflt_ship_rec.desc_method, ML ("Sea "));
							break;
				case 'A' :	strcpy (dflt_ship_rec.desc_method, ML ("Air "));
							break;
				default	 :  strcpy (dflt_ship_rec.desc_method, "    ");
							break;
			}
			dflt_ship_rec.con_no = ddsh_rec.con_no;
			strcpy (dflt_ship_rec.vessel,		ddsh_rec.vessel);
			strcpy (dflt_ship_rec.space_book,	ddsh_rec.space_book);
			strcpy (dflt_ship_rec.carrier,		ddsh_rec.carrier);
			strcpy (dflt_ship_rec.book_ref,		ddsh_rec.book_ref);
			strcpy (dflt_ship_rec.bol_no,		ddsh_rec.bol_no);
			strcpy (dflt_ship_rec.airway,		ddsh_rec.airway);
			strcpy (dflt_ship_rec.con_rel_no,	ddsh_rec.con_rel_no);
			strcpy (dflt_ship_rec.packing,		ddsh_rec.packing);
			strcpy (dflt_ship_rec.port_orig,	ddsh_rec.port_orig);
			strcpy (dflt_ship_rec.dept_orig,	ddsh_rec.dept_orig);
			strcpy (dflt_ship_rec.port_dsch,	ddsh_rec.port_dsch);
			strcpy (dflt_ship_rec.port_dest,	ddsh_rec.port_dest);
			strcpy (dflt_ship_rec.marks [0],	ddsh_rec.mark0);
			strcpy (dflt_ship_rec.marks [1],	ddsh_rec.mark1);
			strcpy (dflt_ship_rec.marks [2],	ddsh_rec.mark2);
			strcpy (dflt_ship_rec.marks [3],	ddsh_rec.mark3);
			strcpy (dflt_ship_rec.marks [4],	ddsh_rec.mark4);
			noShpScn = FALSE;
		}

		if (!cc && ddsh_rec.cost_flag [0] == 'Y')
		{
			i = 0;
			while (i < lcount [PRISCN])
			{
				if (ddln_rec.hhds_hash == store [i].hhdsHash)
					break;
				else
					i++;
			}
			if (i == lcount [PRISCN])
			{
				store [lcount [PRISCN]].ddgd_ptr = ddgd_alloc ();
				LoadDdgd (store [lcount [PRISCN]].ddgd_ptr);
			}
			else
				store [lcount [PRISCN]].ddgd_ptr = store [i].ddgd_ptr;
		}
		else
			store [lcount [PRISCN]].ddgd_ptr = DDGD_NULL;

		scn_set (PRISCN);
		getval (lcount [PRISCN]);

		store [lcount [PRISCN]]._keyed = ddln_rec.keyed;

		if (!ddln_rec.cont_status)
		{
			PriceProcess (lcount [PRISCN]);
	
			if (ddln_rec.sale_price == no_dec (store [lcount [PRISCN]]._calc_sprice))
				store [lcount [PRISCN]]._keyed = 0;
		}

		if (ddhr_rec.exch_rate == 0.00)
			ddhr_rec.exch_rate = 1.00;

		if (store [lcount [PRISCN]]._cost_price)
		{
			local_rec.uplift = ((ddln_rec.sale_price / ddhr_rec.exch_rate) - 
								store [lcount [PRISCN]]._cost_price) / 
								store [lcount [PRISCN]]._cost_price;
		}

		local_rec.uplift *= 100.00;
		store [lcount [PRISCN]]._uplift = local_rec.uplift;
		local_rec.exch_rate = ddhr_rec.exch_rate;
		local_rec.lcl_cst   = local_rec.land_cst;
		store [lcount [PRISCN]]._cost_price = no_dec (local_rec.lcl_cst * 
												 local_rec.exch_rate);
		
		temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
				
		store [lcount [PRISCN]].total = local_rec.extend;

		/* recalculate margin  */
		MarginOk (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   local_rec.qty_sup, 
				   store [lcount [PRISCN]]._cont_status,
				   local_rec.lcl_cst, 
				   store [lcount [PRISCN]]._min_marg, 
				   TRUE);

		putval (lcount [PRISCN]++);

		if (lcount [PRISCN] > MAXLINES) 
			break;

		cc = find_rec (ddln, &ddln_rec, NEXT, "r");
	}

	prog_status = EDIT;
	entry_exit = TRUE;

	if (ddhr_rec.tax_code [0] == 'A' || ddhr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;

	lcount [ORDSCN] = lcount [PRISCN];

	scn_set (HDRSCN);
	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (ddsh,"ddsh_id_no");
	return (EXIT_SUCCESS);
}

void
LoadDdgd (
 struct DDGD_PTR*   lcl_ptr)
{
	int				i;

	for (i = FOB; i <= OT4; i++)
	{
		strcpy (ddgd_rec.co_no,	comm_rec.co_no);
		ddgd_rec.hhds_hash = ddsh_rec.hhds_hash;
		ddgd_rec.line_no = i;
		cc = find_rec (ddgd, &ddgd_rec, EQUAL, "u");
		if (!cc)
		{
			lcl_ptr->ddgd_array [i].line_no = ddgd_rec.line_no;
			sprintf (lcl_ptr->ddgd_array [i].category,
				"%-20.20s", ddgd_rec.category);
			sprintf (lcl_ptr->ddgd_array [i].allocation,
				"%-1.1s", ddgd_rec.allocation);	
			sprintf (lcl_ptr->ddgd_array [i].currency,
				"%-3.3s", ddgd_rec.currency);
			lcl_ptr->ddgd_array [i].fgn_value = ddgd_rec.fgn_value;
			lcl_ptr->ddgd_array [i].exch_rate = ddgd_rec.exch_rate;
			lcl_ptr->ddgd_array [i].loc_value = ddgd_rec.loc_value;
		}
		else
	   	{
			lcl_ptr->ddgd_array [i].line_no = i;
			sprintf (lcl_ptr->ddgd_array [i].category,
				"%-20.20s", cat_desc [i]);
			sprintf (lcl_ptr->ddgd_array [i].allocation,
				"%-1.1s", " ");	
			sprintf (lcl_ptr->ddgd_array [i].currency,
				"%-3.3s", "  ");
			lcl_ptr->ddgd_array [i].fgn_value = 0.00;
			lcl_ptr->ddgd_array [i].exch_rate = 1.00;
			lcl_ptr->ddgd_array [i].loc_value = 0.00;

			if (lcount [CSTSCN] == FOB)
			{
				sprintf (lcl_ptr->ddgd_array [i].allocation, "%-1.1s", "D");	
				sprintf (lcl_ptr->ddgd_array [i].currency, "%-3.3s", 
						 sumr_rec.curr_code);
				lcl_ptr->ddgd_array [i].fgn_value = fob_tot;
				lcl_ptr->ddgd_array [i].exch_rate = pocr_rec.ex1_factor;
				lcl_ptr->ddgd_array [i].loc_value 
					= lcl_ptr->ddgd_array [i].fgn_value 
					/ lcl_ptr->ddgd_array [i].exch_rate;
			}
			if (lcount [CSTSCN] == FRT)
			{
				sprintf (lcl_ptr->ddgd_array [i].allocation, "%-1.1s", "D");	
				sprintf (lcl_ptr->ddgd_array [i].currency, "%-3.3s", currencyCode);
				lcl_ptr->ddgd_array [i].fgn_value = fai_tot;
				lcl_ptr->ddgd_array [i].exch_rate = pocr_rec.ex1_factor;
				lcl_ptr->ddgd_array [i].loc_value 
					= lcl_ptr->ddgd_array [i].fgn_value 
					/ lcl_ptr->ddgd_array [i].exch_rate;
			}
			if (lcount [CSTSCN] == DTY)
			{
				sprintf (lcl_ptr->ddgd_array [i].allocation, "%-1.1s", "D");	
				sprintf (lcl_ptr->ddgd_array [i].currency, "%-3.3s", currencyCode);
				lcl_ptr->ddgd_array [i].fgn_value = dty_tot;
				lcl_ptr->ddgd_array [i].exch_rate = pocr_rec.ex1_factor;
				lcl_ptr->ddgd_array [i].loc_value 
					= lcl_ptr->ddgd_array [i].fgn_value 
					/ lcl_ptr->ddgd_array [i].exch_rate;
			}
			if (lcount [CSTSCN] == OT1)
			{
				sprintf (lcl_ptr->ddgd_array [i].allocation, "%-1.1s", "D");	
				sprintf (lcl_ptr->ddgd_array [i].currency, "%-3.3s", currencyCode);
				lcl_ptr->ddgd_array [i].fgn_value = oth_tot;
				lcl_ptr->ddgd_array [i].exch_rate = pocr_rec.ex1_factor;
				lcl_ptr->ddgd_array [i].loc_value 
					= lcl_ptr->ddgd_array [i].fgn_value 
					/ lcl_ptr->ddgd_array [i].exch_rate;
			}
		}
	}
}

int
CheckDdhr (
 void)
{
	strcpy  (ddhr2_rec.co_no, comm_rec.co_no);
	strcpy  (ddhr2_rec.br_no, comm_rec.est_no);
	sprintf (err_str,"%08ld", ++esmr_rec.nx_dd_order);

	sprintf (ddhr2_rec.order_no, "%-8.8s", err_str);
	return  (find_rec (ddhr2, &ddhr2_rec, EQUAL, "r"));
}

void
CalExtend (
 int    line_no)
{
	/*
	 * Update ddln gross tax for each line.
	 */
		
	l_total = (double) store [line_no].qty;
	l_total *= out_cost (ddln_rec.sale_price, store [line_no]._outer);
	l_total = no_dec (l_total);

	l_dis = (double) ddln_rec.disc_pc;
	l_dis = DOLLARS (l_dis);
	l_dis *= l_total;
	l_dis = no_dec (l_dis);

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) (store [line_no]._tax_pc);
		l_tax = DOLLARS (l_tax);
		l_tax *= (l_total - l_dis);
		l_tax = no_dec (l_tax);
	}
	
	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) (store [line_no]._gst_pc);
		l_gst = DOLLARS (l_gst);
		l_gst *= (l_total - l_dis + l_tax);
	}
	store [line_no].total = l_total + l_tax - l_dis;
}

void
SrchPayTerms (
 void)
{
	int	i = 0;
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
SrchPrices (
 void)
{
	int		count;
	char	*sptr = chk_env ("SK_DBPRINUM");
	if (sptr)
	{
		count = atoi (sptr);
		if (count < 1 || count > 9)
			count = 5;
	}
	else
		count = 5;

	work_open ();
	save_rec ("# ","#Price ");

	save_rec ("1",comm_rec.price1_desc);
	if (count > 1)
		save_rec ("2",comm_rec.price1_desc);
	else if (count > 2)
		save_rec ("3",comm_rec.price1_desc);
	else if (count > 3)
		save_rec ("4",comm_rec.price1_desc);
	else if (count > 4)
		save_rec ("5",comm_rec.price1_desc);
	else if (count > 5)
		save_rec ("6",comm_rec.price1_desc);
	else if (count > 6)
		save_rec ("7",comm_rec.price1_desc);
	else if (count > 7)
		save_rec ("8",comm_rec.price1_desc);
	else if (count > 8)
		save_rec ("9",comm_rec.price1_desc);

	cc = disp_srch ();
	work_close ();
}

/*===================================
| Add Warhouse Record for Current . |
===================================*/
int
AddIncc (
 void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					  inmr_rec.hhsi_hash);
	incc_rec.hhwh_hash = 0L;
	sprintf (incc_rec.sort,"%s%11.11s%-16.16s", inmr_rec.inmr_class,
						      inmr_rec.category,
						      inmr_rec.item_no);
	incc_rec.closing_stock = 0.00;
	incc_rec.committed	  = 0.00;
	incc_rec.backorder	  = 0.00;
	incc_rec.forward	  = 0.00;
	
	incc_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc_rec.stat_flag, "0");
	cc = abc_add (incc, &incc_rec);
	if (cc) 
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					  inmr_rec.hhsi_hash);
	return (find_rec (incc,&incc_rec,EQUAL,"r"));
}

/*============================
| Warn user about something. |
============================*/
int
WarningUser (
 char*  wn_mess,
 int    wn_flip,
 int    mess_len)
{
	int		wn_cnt;	
	int		i;
	
	for (wn_cnt = 1; wn_cnt < mess_len + 1 ; wn_cnt++)
	{
		clear_mess ();
		print_mess (wn_mess);
		sleep (sleepTime);
	}

	if (!wn_flip)
	{
		BusyFunc (0);
		i = prmptmsg (ML (mlDdMess066),"YyNnMm",1,2);
		move (1,2);
		cl_line ();
		BusyFunc (0);
		if (i == 'Y' || i == 'y')
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			/*
			 * Has Credit been Approved ? <Y/N> 
			 */
			i = prmptmsg (ML ("Do you wish to continue?"), "YyNn",1,2);
			heading (HDRSCN);
			scn_display (HDRSCN);
			print_at (2,1,"%-110.110s"," ");
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
BusyFunc (
 int    flip)
{
	print_at (2,1,"%-20.20s",(flip) ? ML (mlStdMess035) : " ");
}

void
PrintCompanyStuff (void)
{
	line_at (20,0,132);
	print_at (21,  0, ML (mlStdMess038) , comm_rec.co_no,comm_rec.co_name);
	print_at (21, 60, ML (mlStdMess039) , comm_rec.est_no,comm_rec.est_name);
	print_at (22,  0, ML (mlStdMess099) , comm_rec.cc_no,comm_rec.cc_name);
	print_at (22, 60, ML (mlStdMess127) , cudp_rec.dp_no,cudp_rec.dp_short);
}

void
SpreadCosts (
	long   hhsuHash)
{
	int		i;
	int		line_sav;
	int		sav_screen;
	double	wsAmt = 0.00;
	double	wsTot = 0.00;
	double	wsFai = 0.00;
	double	wsDty = 0.00;
	double	wsOth = 0.00;

	sav_screen = cur_screen;
	line_sav = line_cnt;
	if (cur_screen != CSTSCN)
	{
		scn_set (cur_screen);
		putval (line_cnt);
		scn_set (CSTSCN);
	}


	/*------------------ 
	| Accumulate Costs |
	------------------*/
	for (i = 0;i < lcount [ORDSCN]; i++) 
	{
		if (store [i].hhsuHash == hhsuHash)
		{
			wsFai = 0.00;
			wsDty = 0.00;
			wsOth = 0.00;
			if (store [i].ddgd_ptr == DDGD_NULL)
				store [i].ddgd_ptr = ddgd_alloc ();

			for (lcount [CSTSCN] = FOB;lcount [CSTSCN] <= OT4; lcount [CSTSCN]++)
			{
	 		   	getval (lcount [CSTSCN]);

				switch (local_rec.cst_spread [0])
				{
					case 'V' : wsAmt = store [i].volume * store [i].qty;
							   wsTot = vol_tot;
							   break;
					case 'W' : wsAmt = store [i].weight * store [i].qty;
							   wsTot = wgt_tot;
							   break;
					default  : wsAmt = DOLLARS (store [i].net_fob * store [i].qty);
							   wsTot = cst_tot;
							   break;
				}

				if (wsTot == 0.00)
					wsTot = 1.00;

	   		 	switch (lcount [CSTSCN])
				{
					case FRT : wsFai += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case INS : wsFai += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case INTEREST :
                               wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case CHG : wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case DTY : wsDty += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case OT1 : wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case OT2 : wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case OT3 : wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
					case OT4 : wsOth += (local_rec.cst_loc_val*(wsAmt / wsTot));
							   break;
				}
				store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].line_no
					= lcount [CSTSCN];
				sprintf (store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].category,
					"%-20.20s", local_rec.cst_category);
				sprintf (store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].allocation,
					"%-1.1s", 
						 local_rec.cst_spread);	
				sprintf (store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].currency,
					"%-3.3s", local_rec.cst_curr);
				store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].fgn_value 
					= local_rec.cst_fgn_val;
				store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].exch_rate 
					= local_rec.cst_exch;
				store [i].ddgd_ptr->ddgd_array [lcount [CSTSCN]].loc_value 
					= local_rec.cst_loc_val;
			}
			store [i].amt_fai = wsFai / store [i].qty;
			store [i].amt_dty = wsDty / store [i].qty;
			store [i].amt_oth = wsOth / store [i].qty;
			scn_set (ORDSCN);
			getval (i);
			local_rec.loc_fi 	= store [i].amt_fai;
			local_rec.duty_val 	= store [i].amt_dty;
			local_rec.oth 		= store [i].amt_oth;
			CalcCosts (i);
			putval (i);
			scn_set (CSTSCN);
		}
	}
	if (cur_screen != sav_screen)
		scn_set (sav_screen);
	
	line_cnt = line_sav;
	getval (line_cnt);
}

void
CalcItemTotals (
	long   hhsuHash)
{
	int		i;
	int		key;
	float	wsFloat;

	cst_tot = 0.00;
	vol_tot = 0.00;
	wgt_tot = 0.00;

	abc_selfield (inmr,"inmr_hhbr_hash");
	for (i = 0;i < lcount [ORDSCN]; i++) 
	{
		if (store [i].hhsuHash == hhsuHash)
		{
			if (store [i].volume == 0.00)
			{
				inmr_rec.hhbr_hash	=	store [i].hhbrHash;
				cc = find_rec (inmr, &inmr_rec, EQUAL,"r");
				if (cc) 
					file_err (cc, inmr, "DBFIND");


				key = prmptmsg (ML (mlStdMess125), "YyNn",1,6);
				if (key != 'Y' && key != 'y')
				{
					print_at (6, 1, "%60.60s", " ");
					sprintf (err_str, ML (mlDdMess073) , clip (inmr_rec.item_no));
					rv_pr (err_str, 1, 6, 1);
					wsFloat = getfloat (57, 6, "NNNNNN.NN");
					if (last_char == '\r')
						store [i].volume = wsFloat;
					else
						store [i].volume = 0.00;
				}
				print_at (6, 1, "%60.60s", " ");
			}

			if (store [i].weight == 0.00)
			{
				inmr_rec.hhbr_hash	=	store [i].hhbrHash;
				cc = find_rec (inmr, &inmr_rec, EQUAL,"r");
				if (cc) 
					file_err (cc, inmr, "DBFIND");

				sprintf (err_str, ML (mlStdMess126));

				key = prmptmsg (err_str, "YyNn",1,6);
				if (key != 'Y' && key != 'y')
				{
					print_at (6, 1, "%60.60s", " ");

					sprintf (err_str, ML (mlDdMess074), clip (inmr_rec.item_no));
					rv_pr (err_str, 1, 6, 1);
					wsFloat = getfloat (57, 6, "NNNNNN.NN");
					if (last_char == '\r')
						store [i].weight = wsFloat;
					else
						store [i].weight = 0.00;
				}
				print_at (6, 1, "%60.60s", " ");
			}
			if (cur_screen == ORDSCN)
				box (0, 2, 132, 2);


			cst_tot += DOLLARS (store [i].net_fob) * store [i].qty;
			vol_tot += store [i].volume * store [i].qty;
			wgt_tot	+= store [i].weight * store [i].qty;
		}
	}
	abc_selfield (inmr, "inmr_id_no");
}

void
CalcCostTotals (
 long   hhsu_hash)
{
	int		i;

	fob_tot = 0.00;
	dty_tot = 0.00;
	fai_tot = 0.00;
	oth_tot = 0.00;

	for (i = 0;i < lcount [ORDSCN]; i++) 
	{
		if (store [i].hhsuHash == hhsu_hash)
		{
			fob_tot += store [i].net_fob * store [i].qty;
			dty_tot += store [i].amt_dty * store [i].qty;
			fai_tot	+= store [i].amt_fai * store [i].qty;
			oth_tot += store [i].amt_oth * store [i].qty;
		}
	}
}

void
CalcTotals (
 void)
{
	int		i;
	double	wk_gst = 0.00;

	inv_tot = 0.00;
	tax_tot = 0.00;
	tot_tot = 0.00;

	for (i = 0;i < lcount [PRISCN]; i++) 
	{
		inv_tot += store [i].total;
		tax_tot += store [i]._tax_amt;
	}

	tax_tot = no_dec (tax_tot);

	fother = 0.00;

	if (notax)
		wk_gst = 0.00;
	else
		wk_gst = (double) (comm_rec.gst_rate / 100.00);

	wk_gst *= fother;
	tax_tot += wk_gst;
	tax_tot = no_dec (tax_tot);

	tot_tot = no_dec (inv_tot + tax_tot + fother);

}




/*==========================
| Validate margin percent. |
==========================*/
void
MarginOk (
 double     sales,
 double     disc,
 float      wsQty,
 int        contSts, 
 double     csale,
 float      min_marg,
 int        loading)
{
	float	marg = 0.00;
	double	wsSales;

	local_rec.margin = 0.00;
	local_rec.marval = 0.00;

	if (local_rec.exch_rate != 0.00)
		sales = no_dec (sales / local_rec.exch_rate);
	wsSales = no_dec (sales - (sales * (disc / 100)));
	wsSales /= 100.00;
	csale /= 100.00;

	/*---------------------------
	| Calculate margin percent. |
	---------------------------*/
	marg = (float) wsSales - (float) csale;
	if (wsSales != 0.00)
		marg /= (float) wsSales;
	else
		marg = 0.00;
	
	marg *= 100.00;
	marg = twodec (marg);
	
	if (marg < min_marg && 
		!MARG_MESS1 && 
		!loading &&
		min_marg != 0.00 &&
		contSts == 0)
	{
		if (MARG_MESS2)
		{
			print_at (2, 0, ML (mlDdMess071));
			print_at (2, 47, ML (mlStdMess042));
		}
		else
		{
			print_at (2, 0, ML (mlDdMess071));
			print_at (2, 75, ML (mlStdMess042));
		}
		box (0, 2, 132, 2);
	}

	local_rec.margin = marg;
	local_rec.marval = CENTS ((wsSales - csale) * wsQty);
	
	return;
}

/*=========================================================
| Load category descriptions if defined else use default. |
=========================================================*/
void
LoadCatDesc (
 void)
{
	char	*sptr;
	int		i;

	for (i = 0; i < 10; i++)
	{
		switch (i)
		{
		case 0:
			sprintf (cat_desc [i], "%-20.20s", inv_cat [i]);
			break;

		case 1:
			sptr = chk_env ("DD_OS_1");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;

		case 2:
			sptr = chk_env ("DD_OS_2");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 3:
			sptr = chk_env ("DD_OS_3");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 4:
			sptr = chk_env ("DD_OS_4");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 5:
			sprintf (cat_desc [i], "%-20.20s", inv_cat [i]);
			break;
		case 6:
			sptr = chk_env ("DD_OTHER1");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 7:
			sptr = chk_env ("DD_OTHER2");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 8:
			sptr = chk_env ("DD_OTHER3");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 9:
			sptr = chk_env ("DD_OTHER4");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;

		default:
			break;
		}
	}
}

void
LoadIntoCstScn (
 void)
{
	int				i;
	int				sav_screen;
	struct DDGD_PTR	*lcl_ptr;

	CalcItemTotals (sumr_rec.hhsu_hash);

	sav_screen = cur_screen;
	init_vars (CSTSCN);	
	scn_set (CSTSCN);

	CalcCostTotals (sumr_rec.hhsu_hash);
	
	lcl_ptr = DDGD_NULL;
	for (i = 0;i < lcount [ORDSCN]; i++) 
	{
		if (store [i].hhsuHash == sumr_rec.hhsu_hash
		&&  store [i].ddgd_ptr != DDGD_NULL)
			lcl_ptr = store [i].ddgd_ptr;
	}

	for (lcount [CSTSCN] = FOB;lcount [CSTSCN] <= OT4; lcount [CSTSCN]++)
	{
		if (lcl_ptr != DDGD_NULL)
	    {

			sprintf (local_rec.cst_category,"%-20.20s", 
					 lcl_ptr->ddgd_array [lcount [CSTSCN]].category);
			sprintf (local_rec.cst_spread,	"%-1.1s", 
			         lcl_ptr->ddgd_array [lcount [CSTSCN]].allocation);
			sprintf (local_rec.cst_curr,	"%-3.3s",
			         lcl_ptr->ddgd_array [lcount [CSTSCN]].currency);
			local_rec.cst_fgn_val 	= 
					 lcl_ptr->ddgd_array [lcount [CSTSCN]].fgn_value;
			local_rec.cst_exch		= 
					 lcl_ptr->ddgd_array [lcount [CSTSCN]].exch_rate;
			local_rec.cst_loc_val 	= 
					 lcl_ptr->ddgd_array [lcount [CSTSCN]].loc_value;

			if (lcount [CSTSCN] == FOB)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", sumr_rec.curr_code);
				local_rec.cst_fgn_val = fob_tot;
				local_rec.cst_exch	  = pocr_rec.ex1_factor;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
	    }
	    else
	    {
			sprintf (local_rec.cst_category,"%-20.20s", 
					 cat_desc [lcount [CSTSCN]]);
			sprintf (local_rec.cst_spread,	"%-1.1s", " ");
			sprintf (local_rec.cst_curr,	"%-3.3s", "   ");
			local_rec.cst_fgn_val 	= 0.00;
			local_rec.cst_exch		= 1.00;
			local_rec.cst_loc_val 	= 0.00;

			if (lcount [CSTSCN] == FOB)
			{
				sprintf (local_rec.cst_category,"%-20.20s", 
						 cat_desc [lcount [CSTSCN]]);
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", sumr_rec.curr_code);
				local_rec.cst_fgn_val = fob_tot;
				local_rec.cst_exch	  = pocr_rec.ex1_factor;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == FRT)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", currencyCode);
				local_rec.cst_fgn_val = fai_tot;
				local_rec.cst_exch	  = 1.000;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == DTY)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", currencyCode);
				local_rec.cst_fgn_val = dty_tot;
				local_rec.cst_exch	  = 1.000;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == OT1)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", currencyCode);
				local_rec.cst_fgn_val = oth_tot;
				local_rec.cst_exch	  = 1.000;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
	    }

	    putval (lcount [CSTSCN]);
	}

	vars [scn_start].row = lcount [CSTSCN];
	cur_screen = sav_screen;
	return;
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	pr_box_lines (scn);
	tab_col = 0;

	rv_pr (ML (mlDdMess056), (132 - strlen (ML (mlDdMess056))) /2, 0, 1);

	switch (scn)
	{
	case	HDRSCN: /*sprintf (err_str, " Header Screen ");*/
			   rv_pr (ML (mlDdMess075), (132 - strlen (ML (mlDdMess075))) /2, 1, 1);
			   break;
	case	ORDSCN: sprintf (err_str, " Order Screen ");
			   rv_pr (ML (mlDdMess076), (132 - strlen (ML (mlDdMess076))) /2, 1, 1);
			   break;
	case	CSTSCN: sprintf (err_str, " On-Cost Screen ");
			   rv_pr (ML (mlDdMess077), (132 - strlen (ML (mlDdMess077))) /2, 1, 1);
			   break;
	case	PRISCN: sprintf (err_str, " Pricing Screen ");
			   rv_pr (ML (mlDdMess078), (132 - strlen (ML (mlDdMess078))) /2, 1, 1);
			   break;
	case	TLRSCN: sprintf (err_str, " Instruction Screen ");
			   rv_pr (ML (mlDdMess079), (132 - strlen (ML (mlDdMess079))) /2, 1, 1);
			   break;
	case	SHPSCN: /*sprintf (err_str, " Shipment Default Screen ");*/
			   rv_pr (ML (mlDdMess087), (132 - strlen (ML (mlDdMess087))) /2, 1, 1);
			   break;
	}

	switch (scn)
	{
	case	HDRSCN:
		use_window (0);
		break;

	case	ORDSCN:
		tab_other (line_cnt);
		box (0, 2, 132, 2);
		DispCustSupp (line_cnt);
		break;
	case	CSTSCN:
		box (0, 2, 132, 2);
		DispCustSupp (-1);
		tab_col = 20;
		break;
	case	PRISCN:
		LoadIntoPriScn ();
		CalcTotals ();
		box (0, 2, 132, 2);
		DispCustSupp (line_cnt);
		break;
	case	TLRSCN:
		LoadIntoTlrScn ();
		/*sprintf (err_str, " Customers Consignment Details ");*/

		us_pr (ML (mlDdMess065), (132 - strlen (ML (mlDdMess065))) /2, 2, 1);
		/*sprintf (err_str, " Supplier Shipping Details. ");*/

		us_pr (ML (mlDdMess002), (132 - strlen (ML (mlDdMess002))) /2, 12, 1);
		break;
	case	SHPSCN:
		break;
	}

	PrintCompanyStuff ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

void
GetOnCost (
 void)
{
	int		count;
	int		i;
	int		maxSupp;
	char	suppList [100] [8];

	for (i = 2; i < 6; i++)
	{
		move (0, i);
		cl_line ();
	}
	tab_open ("supp", supp_keys, 2, 30, 6, FALSE);
	tab_add  ("supp", "# Supplier Supplier Name                           Costed");

	scn_set (ORDSCN);
	maxSupp = 0;
	for (count = 0; count < lcount [ORDSCN]; count++)
	{
		getval (count);
		i = 0;
		while (TRUE)
		{
			if (!strncmp (local_rec.supp_no, suppList [i], 6))
				break;
			if (i == maxSupp)
			{
				strcpy (suppList [i], local_rec.supp_no);
				maxSupp++;
				if (store [count].ddgd_ptr != DDGD_NULL)
					strcat (suppList [i], "Y");
				else
					strcat (suppList [i], "N");
					
				break;
			}
			i++;
		}
	}
	for (count = 0; count < maxSupp; count++)
	{
		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, envVarCrCo);
		strcpy (sumr_rec.crd_no, pad_num (suppList [count]));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (!cc) 
		{
			tab_add  ("supp", "%-6.6s   %-40.40s    %1.1s",
					   suppList [count],
					   sumr_rec.crd_name,
					   &suppList [count] [6]);
		}
	}
	tab_scan ("supp");
	tab_close ("supp", TRUE);
	scn_set (CSTSCN);
}

void
DispCustSupp (
 int lineNo)
{
	box (0, 2, 132, 2);

	sprintf (err_str,  
			ML (mlDdMess041),
			cumr_rec.dbt_no,
			clip (cumr_rec.dbt_name),
			(envVarDbMcurr) ? "(CURR" : "    ", 
			(envVarDbMcurr) ? cumr_rec.curr_code : "   ",
			(envVarDbMcurr) ? ')' : ' ',
			"Pricing Terms : ",
			ddhr_rec.sell_terms,
			"Inv Exch Rate : ",
			local_rec.exch_rate);

	print_at (3, 1, "%s", err_str);

	if (lineNo < 0)
	{
/*		sprintf (err_str,  
		" Supplier No : %6.6s (%-40.40s) %4.4s %-3.3s%c   %s %3.3s  %s %.4f",
				sumr_rec.crd_no,
				clip (sumr_rec.crd_name),
				(envVarDbMcurr) ? "(CURR" : "    ", 
				(envVarDbMcurr) ? sumr_rec.curr_code : "   ",
				(envVarDbMcurr) ? ')' : ' ',
				"Fixed Quote   : ",
				local_rec.exch_desc,
				"P/O Exch Rate : ",
				pocr_rec.ex1_factor);
*/

		sprintf (err_str,  
		ML (mlDdMess042),
				sumr_rec.crd_no,
				clip (sumr_rec.crd_name),
				(envVarDbMcurr) ? "(CURR" : "    ", 
				(envVarDbMcurr) ? sumr_rec.curr_code : "   ",
				(envVarDbMcurr) ? ')' : ' ',
				"Fixed Quote   : ",
				local_rec.exch_desc,
				"P/O Exch Rate : ",
				pocr_rec.ex1_factor);
	}
	else
	{
		sprintf (err_str,  
		ML (mlDdMess042),
				store [lineNo].supp_no,
				clip (store [lineNo].supp_name),
				(envVarDbMcurr) ? "(CURR" : "    ", 
				(envVarDbMcurr) ? store [lineNo].supp_curr : "   ",
				(envVarDbMcurr) ? ')' : ' ',
				"Fixed Quote   : ",
				local_rec.exch_desc,
				"P/O Exch Rate : ",
				store [lineNo].supp_exch);
	}
	print_at (4, 1, "%s", err_str);
}


void
tab_other (
 int    line_no)
{
	static	int	fst_time = TRUE;
	static	int	orig_uplift;
	static	int	orig_sale;
	static	int	orig_fob_cst;
	static	int	orig_net_fob;
	static	int	orig_view_disc;
	static	int	orig_spread;
	static	int	orig_currency;
	static	int	orig_fgn_val;
	static	int	orig_cst_exch;
	static	int	orig_loc_fi;
	static	int	orig_duty_val;
	static	int	orig_other;
	static	int	orig_sale_disc;

	if (fst_time)
	{
		orig_sale      	= FLD ("sale_price");
		orig_uplift    	= FLD ("uplift");
		orig_fob_cst   	= FLD ("fob_cst");
		orig_net_fob   	= FLD ("net_fob");
		orig_view_disc 	= FLD ("view_disc");
		orig_spread 	= FLD ("spread");
		orig_currency 	= FLD ("currency");
		orig_fgn_val 	= FLD ("fgn_val");
		orig_cst_exch 	= FLD ("cst_exch");
		orig_loc_fi 	= FLD ("loc_fi");
		orig_duty_val 	= FLD ("duty_val");
		orig_other 		= FLD ("other");
		orig_sale_disc 	= FLD ("sale_disc");
		fst_time = FALSE;
	}
	else
	{
		/*   reset   */
		FLD ("sale_price") 	= orig_sale;
		FLD ("uplift")     	= orig_uplift;
		FLD ("fob_cst")    	= orig_fob_cst;
		FLD ("net_fob")    	= orig_net_fob;
		FLD ("view_disc")  	= orig_view_disc;
		FLD ("spread")		= orig_spread;
		FLD ("currency")	= orig_currency;
		FLD ("fgn_val")		= orig_fgn_val;
		FLD ("cst_exch")	= orig_cst_exch;
		FLD ("loc_fi")		= orig_loc_fi;
		FLD ("duty_val")	= orig_duty_val;
		FLD ("other")		= orig_other;
		FLD ("sale_disc")  	= orig_sale_disc;
	}

	if (cur_screen == CSTSCN)
	{
		if (line_no == FOB)
		{
			FLD ("spread")		= NA;
			FLD ("currency")	= NA;
			FLD ("fgn_val")		= NA;
			FLD ("cst_exch")	= NA;
		}
		if (!strcmp (ddhr_rec.sell_terms, "FOB") 
		&&  line_no <= OT4)
		{
			FLD ("spread")		= NA;
			FLD ("currency")	= NA;
			FLD ("fgn_val")		= NA;
			FLD ("cst_exch")	= NA;
		}
		if (!strcmp (ddhr_rec.sell_terms, "CIF") 
		&&  line_no != FRT
		&&  line_no != INS)
		{
			FLD ("spread")		= NA;
			FLD ("currency")	= NA;
			FLD ("fgn_val")		= NA;
			FLD ("cst_exch")	= NA;
		}
		return;
	}

	if (cur_screen == ORDSCN)
	{
		DispCustSupp (line_no);
		tab_screen2 (line_no);
		return;
	}


	/*-------------------------
	| turn off and on editing
	| of fields depending on
	| whether contract or not
	| New Line Or field
	| changed
	------------------------*/
	if (cur_screen == PRISCN)
	{
		/*-----------------------------------------------
		| Set the maximum number of lines in the        |
		| TABular screen to the number of lines loaded. |
		-----------------------------------------------*/
		if (prog_status == EDIT)
			vars [scn_start].row = lcount [PRISCN];

		DispCustSupp (line_no);

		if (store [line_no]._cont_status || line_no >= lcount [PRISCN])
		{
			FLD ("sale_price") = NA;
			FLD ("uplift") = NA;
			if (store [line_no]._con_price || store [line_no]._cont_status == 2)
				FLD ("sale_disc") = NA;
			else
				FLD ("sale_disc") = orig_sale_disc;
		}
		else
		{
			/*  if field changed */
			/* see store for doco */
	
			FLD ("sale_disc") = orig_sale_disc;
			if (store [line_no]._keyed == 0)
				return;
	
			if (store [line_no]._keyed == 1)
			{
/*
				FLD ("sale_price") = NA;
*/
				FLD ("uplift") = orig_uplift;
			}
			else
			{
				FLD ("sale_price") = orig_sale;
/*
				FLD ("uplift") = NA;
*/
			}
		}
	}
}

/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
tab_screen2 (
 int iline)
{
	if (!strcmp (ddhr_rec.sell_terms, "CIF"))
	{
		FLD ("loc_fi")		= YES;
		FLD ("duty_val")	= NA;
		FLD ("other")		= NA;
	}
	if (!strcmp (ddhr_rec.sell_terms, "FOB"))
	{
		FLD ("loc_fi")		= NA;
		FLD ("duty_val")	= NA;
		FLD ("other")		= NA;
	}
	if (!strcmp (ddhr_rec.sell_terms, "DIS"))
	{
		FLD ("loc_fi")		= YES;
		FLD ("duty_val")	= YES;
		FLD ("other")		= YES;
	}
	if (store [iline]._cont_cost)
	{
		FLD ("fob_cst")   = NA;
		FLD ("net_fob")   = NA;
		FLD ("view_disc") = NA;
	}
	if (store [iline].ddgd_ptr != DDGD_NULL)
	{
		FLD ("loc_fi")		= NA;
		FLD ("duty_val")	= NA;
		FLD ("other")		= NA;
	}
	if (envVarPoMaxLines)
	{
		if (prog_status == ENTRY && iline >= envVarPoMaxLines)
			/*centre_at (4, 132, "%R Warning, maximum lines exceeded \007");*/
			centre_at (4, 132, ML (mlStdMess158));
		if (prog_status != ENTRY && lcount [ORDSCN] > envVarPoMaxLines)
			/*centre_at (4, 132, "%R Warning, maximum lines exceeded ");*/
			centre_at (4, 132, ML (mlStdMess158));
	}
	if (store [iline].supp_exch == 0.00)
	{
		move (0, 6);
		cl_line ();
		move (0, 7);
		cl_line ();
		return;
	}
	print_at (6,  0, ML (mlDdMess058));
	print_at (6, 55, ML (mlDdMess059));
	print_at (6, 90, ML (mlDdMess060));
	print_at (7,  0, ML (mlDdMess061));
	print_at (7, 55, ML (mlDdMess062));
	print_at (7, 90, ML (mlDdMess063));


	print_at (6, 11, "%3d", iline + 1);
	if (store [iline]._outer > 0.00)
		print_at (6,71, "%-7.1f", store [iline]._outer);
	else
		print_at (6,71, "%-7.1f", 1.00);
	print_at (6,111, "%9.4f", store [iline].supp_conv);

	print_at (7, 11, "%-40.40s", store [iline].item_desc);
	print_at (7, 71, "%-4.4s", store [iline].supp_uom);
	print_at (7,111, "%6.1f", store [iline].supp_lead);

	if (prog_status != ENTRY)
		strcpy (local_rec.supp_uom, store [iline].supp_uom);

	return;
}

/*=======================
| Search for currency	|
=======================*/
void
SrchPocr (char *key_val)
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
		file_err (cc, pocr, "DBFIND");
}



/*==================
| Search for inis. |
==================*/
void
SrchInis (void)
{
	abc_selfield (sumr, "sumr_hhsu_hash");
	work_open ();
	save_rec ("#Number", "#Priority  Supplier Name.");
	inis_rec.hhbr_hash = SR.hhbrHash;
	inis_rec.hhsu_hash = 0L;
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc =  find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == SR.hhbrHash)
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf (err_str, "    %2.2s    %s", 
					 inis_rec.sup_priority,
					 sumr_rec.crd_name);
			cc = save_rec (sumr_rec.crd_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
	abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}


/*======================
| Search for salesman. |
======================*/
void
SrchExsf (
 char*      key_val)
{
	work_open ();
	save_rec ("#Sm", "#Salesman.");
	strcpy  (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp  (exsf_rec.co_no, comm_rec.co_no) && 
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
	strcpy  (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*=================================
| Search on Department (cudp)     |
=================================*/
void
SrchCudp (
 char*      key_val)
{
	_work_open (2,0,40);
	save_rec ("#DN","#Department Description");

	strcpy (cudp_rec.co_no, comm_rec.co_no);
	strcpy (cudp_rec.br_no, comm_rec.est_no);
	strcpy (cudp_rec.dp_no, key_val);
	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (cudp_rec.co_no, comm_rec.co_no) &&
		   !strcmp (cudp_rec.br_no, comm_rec.est_no))
	{                        
		if (strncmp (cudp_rec.dp_no, key_val, strlen (key_val)))
		{
			cc = find_rec (cudp, &cudp_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
		if (cc)
			break;
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (cudp_rec.co_no, comm_rec.co_no);
	strcpy (cudp_rec.br_no, comm_rec.est_no);
	sprintf (cudp_rec.dp_no,    "%-2.2s", temp_str);
	cc = find_rec (cudp, &cudp_rec, EQUAL, "r");
	if (cc)
 	        file_err (cc, cudp, "DBFIND");
}

/*===============================
| Search on Contract (cnch)     |
===============================*/
void
SrchCnch (
 char*      key_val)
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

		/*
		if (cnch_rec.date_wef > TodaysDate ())
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}

		if (cnch_rec.date_exp < TodaysDate ())
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}
		*/

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

	cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
	if (cc)
 	        file_err (cc, cnch, "DBFIND");
}

/*============================
| Calculate total line cost. |
============================*/
void
CalcCosts (
 int    wk_line)
{
	double	cif_cost = 0.00;

	local_rec.net_fob = CalcNet (local_rec.grs_fgn, 
								store [wk_line].discArray, 
								store [wk_line].cumulative);
	local_rec.net_fob = no_dec (local_rec.net_fob);
	store [wk_line].net_fob = local_rec.net_fob;
	if (local_rec.duty_val == 0.00)
		local_rec.duty_val = DutyCalc (wk_line);
	
	if (local_rec.loc_fi == 0.00)
		local_rec.loc_fi = FreightCalc (wk_line);

	store [wk_line].amt_fai = local_rec.loc_fi;

	/*-------------------
	| Calculate CIF FGN	|
	-------------------*/
	cif_cost = local_rec.net_fob * store [wk_line]._outer;

	/*-------------------
	| Calculate CIF NZL	|
	-------------------*/
	if (store [wk_line].supp_exch != 0.00)
		local_rec.cif_loc = cif_cost / store [wk_line].supp_exch;
	else
		local_rec.cif_loc = 0.00;

	local_rec.cif_loc = twodec (local_rec.cif_loc);
	local_rec.cif_loc += local_rec.loc_fi;
	local_rec.cif_loc = twodec (local_rec.cif_loc);
	local_rec.cif_loc = no_dec (local_rec.cif_loc);

	/*-------------------------------
	| Calculate Landed Cost NZL	|
	-------------------------------*/
	local_rec.land_cst = local_rec.cif_loc + 
			     		 local_rec.duty_val + 
			     		 local_rec.oth;

	store [wk_line].land_cst = local_rec.land_cst;
}

float
RndMltpl (
 float  ord_qty,
 char*  rnd_type,
 float  ord_mltpl,
 float min_qty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (ord_qty == 0.00)
		return (0.00);

	if (ord_mltpl == 0.00)
		return ((ord_qty < min_qty) ? min_qty : ord_qty);

	ord_qty -= min_qty;
	if (ord_qty < 0.00)
		ord_qty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (ord_qty / ord_mltpl);
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
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = floor (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double) ord_qty;
		wrk_qty = (up_qty / (double)ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * ord_mltpl);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double) ord_qty;
		wrk_qty = (down_qty / (double) ord_mltpl);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * ord_mltpl);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double) ord_qty) <= ((double) ord_qty - down_qty))
			ord_qty = (float) up_qty;
		else
			ord_qty = (float) down_qty;

		break;

	default:
		break;
	}

	return (min_qty + ord_qty);
}

/*====================
| Calculate Freight. |
====================*/
double
FreightCalc (
 int    wk_line)
{
	double	value = 0.00;
	double	frt_conv = 0.00;


	if (!strcmp (ddhr_rec.sell_terms, "FOB"))
		return (0.00);

	/*-----------------------
	| Calculate Freight	|
	-----------------------*/
	frt_conv = pocf_rec.freight_load;

	/*--------------------------
	| Freight is a Unit value. |
	--------------------------*/
	if (pocf_rec.load_type [0] == 'U')
		value = frt_conv;

	/*--------------------------
	| Freight is a Percentage. |
	--------------------------*/
	if (pocf_rec.load_type [0] == 'P')
	{
		value = (store [wk_line].net_fob * store [wk_line]._outer);
		value = (value * frt_conv) / 100;
	}

	if (local_rec.supp_exch != 0.00)
		value /= local_rec.supp_exch;

	value = no_dec (value);

	return (value);
}

/*==================================================
| Calculate Duty on total quantity and each basis. |
==================================================*/
double
DutyCalc (
 int    wk_line)
{
	double	value  = 0.00;

	if (!strcmp (ddhr_rec.sell_terms, "FOB") ||
	    !strcmp (ddhr_rec.sell_terms, "CIF"))
		return (0.00);

	/*-------------------
	| Calculate Duty   	|
	-------------------*/
	if (store [wk_line].duty_type [0] == 'D')
		value = store [wk_line].imp_duty;
	else
	{
		value = store [wk_line].net_fob * store [wk_line]._outer;

		value *= (store [wk_line].imp_duty / 100);
		if (local_rec.supp_exch != 0.00)
			value /= local_rec.supp_exch;

		value = no_dec (value);
	}
	return (value);
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
	DspFlds (currFld);
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
			heading (ORDSCN);
			line_cnt = tmpLineCnt;
			lcount [ORDSCN] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [ORDSCN];
			scn_display (ORDSCN);
			DrawDiscScn ();
			DspFlds (currFld);
			break;
		}

		DspFlds (currFld);
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
		VerticalLine (DBOX_LFT + discScn [i].xPos, DBOX_TOP);

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
VerticalLine (
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
DspFlds (
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

int
CheckReorder (
 char*  item_class)
{
	if (strchr (envVarPoReorder, item_class [0]) == (char *) 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*==============
 Find inis file
===============*/
int
FindInis (
 long   HHBR_HASH, 
 long   HHSU_HASH)
{
	if (HHSU_HASH > 0L)
	{
		inis_rec.hhbr_hash = HHBR_HASH;
		inis_rec.hhsu_hash = HHSU_HASH;
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
	else
	{
		inis_rec.hhbr_hash = HHBR_HASH;
		inis_rec.hhsu_hash = 0L;
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no,"  ");
		cc =  find_rec (inis, &inis_rec, GTEQ, "r");
		if (!cc && inis_rec.hhbr_hash == HHBR_HASH)
			return (EXIT_SUCCESS);
		else
			return (EXIT_FAILURE);
	}
}


/*=====================
| Find Currency file. |
=====================*/
int
FindPocr (
 char*  code)
{
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s", code);
	cc = find_rec (pocr,&pocr_rec,EQUAL,"r");
	if (cc)
	{
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
FindPocf (char *code)
{
	open_rec (pocf,pocf_list,POCF_NO_FIELDS,"pocf_id_no");
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	sprintf (pocf_rec.code,"%-3.3s", code);
	cc = find_rec (pocf,&pocf_rec,EQUAL,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess118));
		abc_fclose (pocf);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	abc_fclose (pocf);
	return (EXIT_SUCCESS);
}

/*===========================================
| Find Special Instructions for a Supplier. |
===========================================*/
void
FindExsi (
 void)
{
	/*-------------------------------------------------
	| Check if Supplier Has Any Special Instructions. |
	-------------------------------------------------*/
	open_rec (exsi ,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = comr_rec.po_sic1;
	cc = find_rec (exsi ,&exsi_rec,EQUAL,"r");
	sprintf (pohr_rec.stdin1,"%60.60s",(cc) ? " " : exsi_rec.inst_text);

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = comr_rec.po_sic2;
	cc = find_rec (exsi ,&exsi_rec,EQUAL,"r");
	sprintf (pohr_rec.stdin2,"%60.60s",(cc) ? " " : exsi_rec.inst_text);

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = comr_rec.po_sic3;
	cc = find_rec (exsi ,&exsi_rec,EQUAL,"r");
	sprintf (pohr_rec.stdin3,"%60.60s",(cc) ? " " : exsi_rec.inst_text);

	abc_fclose (exsi);
	return;
}

void
GetWarehouse (
 long   hhccHash)
{
	if (hhccHash == 0L)
	{
		if (hhcc_selected)
		{
			abc_selfield (ccmr,"ccmr_id_no");
			hhcc_selected = FALSE;
		}

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,comm_rec.est_no);
		strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
		cc = find_rec (ccmr,&ccmr_rec,EQUAL,"r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");
	
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, esmr, "DBFIND");
		strcpy (local_rec.br_name, esmr_rec.est_name);
		abc_fclose (esmr);
	}
	else
	{
		if (!hhcc_selected)
		{
			abc_selfield (ccmr,"ccmr_hhcc_hash");
			hhcc_selected = TRUE;
		}
		
		ccmr_rec.hhcc_hash	=	hhccHash;
		if (find_rec (ccmr,&ccmr_rec,EQUAL,"r"))
		{
			abc_selfield (ccmr,"ccmr_id_no");
			GetWarehouse (0L);
			return;
		}
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, esmr, "DBFIND");
		strcpy (local_rec.br_name, esmr_rec.est_name);
		abc_fclose (esmr);
	}
	return;
}

void
LoadIntoPriScn (
 void)
{
	int		count;
	int		sav_screen;
	double	temp_gross 	= 0.00,
			temp_dis	= 0.00;

	sav_screen = cur_screen;
	for (count = 0; count < lcount [ORDSCN]; count++)
	{
		scn_set (ORDSCN);
		getval (count);
		CalcCosts (count);
		putval (count);
		scn_set (PRISCN);

		/*-----------------------
		| if line is less than
		| lcount [PRISCN] means old
		| line - retreive it
		------------------------*/
		if (count < lcount [PRISCN])
			getval (count);
		else
			store [count]._keyed = 0;

		/*------------------------
		| work out new information
		-------------------------*/
		local_rec.qty_sup = local_rec.qty;
		sprintf (local_rec.description, "%-20.20s", inmr_rec.description);
		local_rec.lcl_cst = local_rec.land_cst;
		store [count]._cost_price = no_dec (local_rec.lcl_cst * 
											local_rec.exch_rate);

		/*------------------------
		| get contract price ???
		------------------------*/
		if (cnch_rec.hhch_hash != 0L)
		{
			if (store [count]._keyed != 2)
			{
				ddln_rec.sale_price = ContCusPrice (cnch_rec.hhch_hash,
												   store [count].hhbrHash,
												   store [count].hhccHash,
												   cumr_rec.curr_code,
												   cnch_rec.exch_type,
												   FGN_CURR,
												   (float)local_rec.exch_rate);

				if (ddln_rec.sale_price != (double) -1.00)
				{
					if (local_rec.lcl_cst)
						local_rec.uplift = ((ddln_rec.sale_price /
											local_rec.exch_rate) - 
											local_rec.lcl_cst) /
											local_rec.lcl_cst;

					local_rec.uplift *= 100.00;
					store [count]._uplift = local_rec.uplift;

					store [count]._cont_status = TRUE;
					local_rec.extend = no_dec ((ddln_rec.sale_price 
									 - (ddln_rec.sale_price
								  	 * (ScreenDisc (ddln_rec.disc_pc) / 100)))
								  	 * local_rec.qty_sup);
					store [count].total = local_rec.extend;
					CalExtend (count);
				}
			}
		}

		/*----------------------
		| if not contract or
		| contract not found
		----------------------*/
		if (ddln_rec.sale_price == (double) -1.00 || cnch_rec.hhch_hash == 0L)
		{

			PriceProcess (count);
			if (store [count]._keyed == 0)
				local_rec.uplift = 0.00;
			/*-------------------------------------------
			| if _keyed != 2 then means price not edited
			------------------------------------------------*/
			if (store [count]._keyed == 1)
			{
				ddln_rec.sale_price = no_dec (local_rec.lcl_cst * 
											  local_rec.exch_rate);
				ddln_rec.sale_price *= (1.00 + (store [count]._uplift / 100.00));
				ddln_rec.sale_price = no_dec (ddln_rec.sale_price); 

				local_rec.uplift = store [count]._uplift;
				store [count]._cont_status = FALSE;
				
				temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
				temp_gross = no_dec (temp_gross);
				
				temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
				temp_dis = no_dec (temp_dis);
				local_rec.extend = temp_gross - temp_dis;			
				
				store [count].total = local_rec.extend;
				CalExtend (count);
			}
			else
			{
				if (local_rec.lcl_cst)
					local_rec.uplift = ((ddln_rec.sale_price /
										local_rec.exch_rate) - 
										local_rec.lcl_cst) /
										local_rec.lcl_cst;
				local_rec.uplift *= 100.00;
				store [count]._uplift = local_rec.uplift;
			}
		}

		if (store [count]._dis_or [0] != 'Y')
			DiscProcess (count);
		
		MarginOk (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   local_rec.qty_sup,
				   store [count]._cont_status,
				   local_rec.lcl_cst, 
				   store [count]._min_marg, 
				   TRUE);
				   
		temp_gross = ddln_rec.sale_price * local_rec.qty_sup;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		store [count].total = local_rec.extend;
		CalExtend (count);
		putval (count);
	}
	lcount [PRISCN] = count;
	if (sav_screen != cur_screen)
		scn_set (sav_screen);
}

void
LoadIntoTlrScn (
 void)
{
	if (newOrder && !strcmp (ddhr_rec.del_name, forty_spaces))
	{
		strcpy (ddhr_rec.del_name,   cumr_rec.dbt_name);
		strcpy (ddhr_rec.del_add1, cumr_rec.dl_adr1);
		strcpy (ddhr_rec.del_add2, cumr_rec.dl_adr2);
		strcpy (ddhr_rec.del_add3, cumr_rec.dl_adr3);
	}
	CatIntoPohr ();

	if (newOrder)
	{
		open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

		if (!strcmp (local_rec.spinst [0], sixty_spaces))
		{
			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg1;
			if (!find_rec (exsi,&exsi_rec, EQUAL,"r"))
				sprintf (local_rec.spinst [0],"%-60.60s",exsi_rec.inst_text);
		}
		if (!strcmp (local_rec.spinst [1], sixty_spaces))
		{
			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg2;
			if (!find_rec (exsi,&exsi_rec, EQUAL,"r"))
				sprintf (local_rec.spinst [1],"%-60.60s",exsi_rec.inst_text);
		}
		if (!strcmp (local_rec.spinst [2], sixty_spaces))
		{
			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg3;
			if (!find_rec (exsi,&exsi_rec, EQUAL,"r"))
				sprintf (local_rec.spinst [2],"%-60.60s",exsi_rec.inst_text);
		}
		abc_fclose (exsi);
	}
}

void
PriceProcess (int wsLine)
{
	int			pType;
	float		regPc;
	double		grossPrice;

	store [wsLine]._pricing_chk	= FALSE;

	pType = atoi (ddhr_rec.pri_type);
	grossPrice = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  comm_rec.cc_no,
							  cumr_rec.area_code,
							  cumr_rec.class_type,
							  store [wsLine]._sellgrp,
							  cumr_rec.curr_code,
							  pType,
							  cumr_rec.class_type,
							  cnch_rec.exch_type,
							  cumr_rec.hhcu_hash,
							  ccmr_rec.hhcc_hash,
							  store [wsLine].hhbrHash,
							  store [wsLine]._category,
							  cnch_rec.hhch_hash,
							  (envVarSoDoi) ? TodaysDate() : comm_rec.dbt_date,
							  local_rec.qty_sup,
							  pocr_rec.ex1_factor,
							  FGN_CURR,
							  &regPc);
	store [wsLine]._pricing_chk	= TRUE;

	store [wsLine]._calc_sprice = GetCusGprice (grossPrice, regPc);
	if (store [wsLine]._keyed == 0)
	{
		store [wsLine]._gsale_price 	= 	grossPrice;
		store [wsLine]._sale_price 	=	store [wsLine]._calc_sprice;
		store [wsLine]._reg_pc 		= 	regPc;
		ddln_rec.sale_price 		= 	store [wsLine]._calc_sprice;
		store [wsLine]._act_sale 		= 	store [wsLine]._calc_sprice;
	}
	store [wsLine]._con_price 		= (_CON_PRICE) ? TRUE : FALSE;
	store [wsLine]._cont_status  	= _cont_status;
}

void
DiscProcess (int wsLine)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (store [wsLine]._cont_status == 2 
	|| store [wsLine]._con_price 
	|| store [wsLine]._indent)
	{
		ddln_rec.disc_pc	  	= 0.00;
		store [wsLine]._dis_pc 	= 0.00;
		store [wsLine]._calc_disc 	= 0.00;
		store [wsLine]._disc_a		= 0.00;
		store [wsLine]._disc_b		= 0.00;
		store [wsLine]._disc_c		= 0.00;
		return;
	}
/*
	if (store [wsLine]._pricing_chk == FALSE)
*/
	PriceProcess (wsLine);

	pType = atoi (ddhr_rec.pri_type);
	cumDisc		=	GetCusDisc (	comm_rec.co_no,
								comm_rec.est_no,
								ccmr_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								store [wsLine].hhbrHash,
								store [wsLine]._category,
								store [wsLine]._sellgrp,
								pType,
								store [wsLine]._gsale_price,
								store [wsLine]._reg_pc,
								local_rec.qty_sup,
								discArray);
							
	if (store [wsLine]._dis_or [0] == 'Y')
		return;
	
	store [wsLine]._calc_disc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	if (store [wsLine]._dis_or [0] == 'N')
	{
		ddln_rec.disc_pc 			=	ScreenDisc (store [wsLine]._calc_disc);
		store [wsLine]._dis_pc		=	store [wsLine]._calc_disc;

		store [wsLine]._disc_a 		= 	discArray [0];
		store [wsLine]._disc_b 		= 	discArray [1];
		store [wsLine]._disc_c		= 	discArray [2];
		store [wsLine]._cumulative	= 	cumDisc;

		if (store [wsLine]._dflt_disc > ScreenDisc (ddln_rec.disc_pc) &&
			store [wsLine]._dflt_disc != 0.0)
		{
			ddln_rec.disc_pc		= 	ScreenDisc (store [wsLine]._dflt_disc);
			store [wsLine]._calc_disc	=	store [wsLine]._dflt_disc;
			store [wsLine]._dis_pc		=	store [wsLine]._dflt_disc;
			store [wsLine]._disc_a 	= 	store [wsLine]._dflt_disc;
			store [wsLine]._disc_b 	= 	0.00;
			store [wsLine]._disc_c 	= 	0.00;
		}
	}
}

/*=================================
| Add Branch Record for Current . |
=================================*/
int
AddInei (
 void)
{
	inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					  inmr_rec.hhsi_hash);
	strcpy (inei_rec.est_no, comm_rec.est_no);
	inei_rec.avge_cost = 0.00;
	inei_rec.last_cost = 0.00;
	strcpy (inei_rec.stat_flag,"0");
	cc = abc_add (inei,&inei_rec);
	if (cc) 
		return (EXIT_FAILURE);

	return (find_rec (inei,&inei_rec,EQUAL,"r"));
}

int
CheckPohr (
 long   po_no)
{
	strcpy (pohr2_rec.co_no,comm_rec.co_no);
	strcpy (pohr2_rec.br_no,comm_rec.est_no);
	sprintf (err_str, "%ld", po_no);
	sprintf (pohr2_rec.pur_ord_no,"DD%13.13s", zero_pad (err_str, 13));
	return (find_rec (pohr2,&pohr2_rec,EQUAL,"r"));
}

/*===================================
| Update Inventory Supplier Record. |
===================================*/
void
UpdateInis (
	double     upd_value,
	float	    upd_vol)
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
		strcpy (inis_rec.wh_no,"  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"u");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no,"  ");
		strcpy (inis_rec.wh_no,"  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"u");
	}
	if (!cc)
	{
		inis_rec.fob_cost = upd_value;
		inis_rec.lcost_date = TodaysDate ();
		if (inis_rec.volume == 0 && upd_vol != 0)
			inis_rec.volume = upd_vol;

		cc = abc_update (inis,&inis_rec);
		if (cc)
			file_err (cc, inis, "DBUPDATE");
	}
	else
		abc_unlock (inis);
}

int
UpdateOrder (
	char	*updFlag)
{
	int		key;
	int 	i = 0;
	int		new_ddln;
	int		ddLines = 0;
	char	printProgram [41];
	char	updateFlag [2];
	double	credit_over;

	PrintOff	=	0;

	clear ();

	/*------------------------
	| D D H R  D E T A I L S |
	-------------------------*/
	strcpy (updateFlag, updFlag);
	strcpy (ddhr_rec.din_1,	local_rec.spinst[0]);
	strcpy (ddhr_rec.din_2,	local_rec.spinst[1]);
	strcpy (ddhr_rec.din_3,	local_rec.spinst[2]);
	strcpy (ddhr_rec.carr_area, local_rec.carr_area);
	ddhr_rec.no_ctn			=	local_rec.no_ctn;
	ddhr_rec.wgt_per_ctn	=	local_rec.wgt_per_ctn;

	ord_total 		= 0.00;
	ddhr_rec.gross 	= 0.00;
	ddhr_rec.disc 	= 0.00;
	ddhr_rec.tax 	= 0.00;
	ddhr_rec.gst 	= 0.00;

	if (newOrder && !strcmp (updateFlag, DELETEFLAG))
		return (TRUE);

	if (newOrder && lcount [PRISCN] == 0)
	{
		for (i = 0; i < 6; i++)
		{
			rv_pr (ML (mlDdMess043) , 0,0,i % 2);
			sleep (sleepTime);
		}
		return (TRUE);
	}

	/*-------------------------------
	| Check Forward Exchange Amounts |
	--------------------------------*/
	if (envVarFeInstall)
	{
		CalcExtend (TRUE);
		if (!newOrder)
		{
			/* If contract hasn't changed get old feln value for check */

			strcpy (feln_rec.index_by, "D");
			feln_rec.index_hash = ddhr_rec.hhdd_hash;
			cc = find_rec (feln, &feln_rec, EQUAL,"r");
			if (cc)
				feln_rec.value = 0.00;
		}
		else
			feln_rec.value = 0.00;

		if (!strcmp (fehr2_rec.cont_no, fehr_rec.cont_no))
			fehr_rec.val_avail += feln_rec.value;
		else
			fehr2_rec.val_avail += feln_rec.value;

		if (strcmp (ddhr_rec.fwd_exch, "      ")
		&&  strcmp (updateFlag, DELETEFLAG)
		&&  ord_total > fehr_rec.val_avail)
		{
			sprintf (err_str, ML (mlDdMess011), DOLLARS (fehr_rec.val_avail));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			if (!strcmp (fehr2_rec.cont_no, fehr_rec.cont_no))
				fehr_rec.val_avail -= feln_rec.value;
			else
				fehr2_rec.val_avail -= feln_rec.value;
			return (FALSE);
		}
	}

	if (!strcmp (updateFlag, ACTIVEFLAG))
	{
		CalcExtend (TRUE);
		credit_over = CheckCumr (ord_total, TRUE);
		if (credit_over > 0.00)
		{
			print_at (PrintOff++,0, ML (mlDdMess055) ,ddhr_rec.order_no, DOLLARS (credit_over));

			sleep (6);
			strcpy (updateFlag, PENDINGFLAG);
		}
	}

	if (newOrder)
	{
		AddDdhrRecord (updateFlag);
		if (strcmp (ddhr_rec.fwd_exch, "      ") && envVarFeInstall)
		{
			strcpy (feln_rec.index_by, "D");
			feln_rec.hhfe_hash  = fehr_rec.hhfe_hash;
			feln_rec.index_hash = ddhr_rec.hhdd_hash;
			feln_rec.value     = 0.00;
			cc = abc_add (feln, &feln_rec);
			if (cc) 
				file_err (cc, feln, "DBADD");
			strcpy (feln_rec.index_by, "D");
			feln_rec.index_hash = ddhr_rec.hhdd_hash;
			cc = find_rec (feln, &feln_rec, EQUAL,"u");
			if (cc)
				file_err (cc, feln, "DBFIND");
		}
	}
	else
	{
		if (envVarFeInstall
		&&  strcmp (ddhr_rec.fwd_exch, "      "))
		{
			strcpy (feln_rec.index_by, "D");
			feln_rec.index_hash = ddhr_rec.hhdd_hash;
			cc = find_rec (feln, &feln_rec, EQUAL,"u");
			if (cc)
			{
				strcpy (feln_rec.index_by, "D");
				feln_rec.hhfe_hash = fehr_rec.hhfe_hash;
				feln_rec.index_hash = ddhr_rec.hhdd_hash;
				feln_rec.value     = 0.00;
				cc = abc_add (feln, &feln_rec);
				if (cc) 
					file_err (cc, feln, "DBADD");
				strcpy (feln_rec.index_by, "D");
				feln_rec.index_hash = ddhr_rec.hhdd_hash;
				cc = find_rec (feln, &feln_rec, EQUAL,"u");
				if (cc)
					file_err (cc, feln, "DBFIND");
			}
		}
		print_at (PrintOff++,1, ML (mlDdMess019));
	}

	/*--------------
	| Process ddlns |
	---------------*/
	scn_set (PRISCN);
	ddLines = lcount [PRISCN];
	lcount [PRISCN] = 0;

	for (line_cnt = 0; line_cnt < ddLines; line_cnt++) 
	{
		/*--------------
		| new line added
		---------------*/
		if (store [line_cnt].hhdlHash == -1)
		{
			new_ddln = TRUE;
			ddln_rec.line_no = line_cnt;
		}
		else
		{
			ddln_rec.hhdd_hash 	= ddhr_rec.hhdd_hash;
			ddln_rec.line_no 	= line_cnt;
			ddln_rec.hhdl_hash	= store [line_cnt].hhdlHash;
			new_ddln = find_rec (ddln2, &ddln_rec, EQUAL, "r");
			if (!new_ddln)
			{
				ddln_rec.hhdd_hash 	= ddhr_rec.hhdd_hash;
				ddln_rec.line_no 	= line_cnt;
				new_ddln = find_rec (ddln, &ddln_rec, EQUAL, "u"); 
			}
			ddln_rec.line_no = line_cnt;
		}

		scn_set (ORDSCN);

		getval (line_cnt);
	
		strcpy (inmr_rec.co_no,   comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");
	
		AddPolnRecord (SR.hhplHash, SR.hhsuHash, line_cnt, updateFlag);

		if (SR.qty == 0.00 || !strcmp (updateFlag, DELETEFLAG))
		{
			if (!new_ddln)
			{
				cc = abc_delete (ddln);
				if (cc)
					file_err (cc, ddln, "DBDELETE");
			}
			continue;
		}

		if (SR.upd_inis)
		{
			UpdateInis (local_rec.net_fob * SR._outer, SR.volume);
		}
	
		scn_set (PRISCN);

		getval (line_cnt);

		lcount [PRISCN]++;
		CalExtend (line_cnt);

		ddhr_rec.gross 	+= l_total;
		ddhr_rec.disc 	+= l_dis;
		ddhr_rec.tax 	+= l_tax;
		ddhr_rec.gst 	+= l_gst;

		strcpy (ddln_rec.item_desc,  inmr_rec.description);
		strcpy (ddln_rec.sman_code,  ddhr_rec.sman_code);
		ddln_rec.hhbr_hash 	= SR.hhbrHash;
		ddln_rec.hhsu_hash 	= SR.hhsuHash;
		ddln_rec.hhpl_hash 	= SR.hhplHash;
		ddln_rec.hhds_hash  = ddsh_rec.hhds_hash;
		ddln_rec.q_order  	= SR.qty;
		ddln_rec.cost_price = SR.land_cst;
		ddln_rec.on_cost    = SR.amt_fai
							+ SR.amt_dty
							+ SR.amt_oth;

		ddln_rec.reg_pc		= SR._reg_pc;
		ddln_rec.gst_pc 	= (notax) ? 0.00 : SR._gst_pc;
		ddln_rec.tax_pc 	= (notax) ? 0.00 : SR._tax_pc;
		strcpy (ddln_rec.bonus_flag, "N");

		ddln_rec.gsale_price = SR._gsale_price;
		ddln_rec.gross 		= l_total;
		ddln_rec.amt_disc 	= l_dis;
		ddln_rec.amt_tax 	= l_tax;
		ddln_rec.amt_gst 	= l_gst;

		if (local_rec.date_reqd > 0L)
			ddln_rec.due_date = local_rec.date_reqd;
		else
			ddln_rec.due_date = ddhr_rec.dt_raised;
		ddln_rec.req_date = ddln_rec.due_date;

		strcpy (ddln_rec.stat_flag, updateFlag);
		strcpy (ddln_rec.pack_size, inmr_rec.pack_size);
		ddln_rec.keyed = SR._keyed;
		ddln_rec.cont_status = SR._cont_status;

		if (!new_ddln)
		{
			ddln_rec.disc_pc = ScreenDisc (ddln_rec.disc_pc);
			cc = abc_update (ddln, &ddln_rec);
			if (cc)
				file_err (cc, ddln, "DBUPDATE");
		}
		else 
		{
			ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
			ddln_rec.line_no = line_cnt;
			ddln_rec.disc_pc = ScreenDisc (ddln_rec.disc_pc);
			cc = abc_add (ddln, &ddln_rec);
			if (cc) 
				file_err (cc, ddln, "DBADD");
		}
	}

	if (!newOrder && (lcount [PRISCN] > 0 && strcmp (updateFlag,DELETEFLAG)))
	{
		print_at (PrintOff++,0, ML (mlDdMess044));
		abc_selfield (pohr, "pohr_hhdd_hash");
		abc_selfield (poln, "poln_hhpo_hash");
	
		pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
		while (!cc && pohr_rec.hhdd_hash == ddhr_rec.hhdd_hash)
		{
			poln_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
			cc = find_rec (poln, &poln_rec, EQUAL, "r");
			if (cc)
			{
				print_at (PrintOff++,0, ML (mlDdMess017), pohr_rec.pur_ord_no);

				cc = abc_delete (pohr);
				if (cc) 
					file_err (cc, pohr, "DBDELETE");
			}
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
		}
		abc_selfield (pohr, "pohr_id_no");
		abc_selfield (poln, "poln_hhpl_hash");

		print_at (PrintOff++,0, ML (mlDdMess045));

		abc_selfield (ddsh, "ddsh_id_no3");
		abc_selfield (ddln, "ddln_hhds_hash");
		abc_selfield (ddgd, "ddgd_hhds_hash");
	
		ddsh_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		ddsh_rec.hhds_hash = 0L;
		cc = find_rec (ddsh, &ddsh_rec, GTEQ, "u");
		while (!cc && ddsh_rec.hhdd_hash == ddhr_rec.hhdd_hash)
		{
			ddln_rec.hhds_hash	=	ddsh_rec.hhds_hash;
			cc = find_rec (ddln, &ddln_rec, EQUAL, "r");
			if (cc)
			{
				ddgd_rec.hhds_hash = ddsh_rec.hhds_hash;
				cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
				while (!cc && ddgd_rec.hhds_hash == ddsh_rec.hhds_hash)
				{
					cc = abc_delete (ddgd);
					if (cc) 
						file_err (cc, ddgd, "DBDELETE");
					cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
				}
				cc = abc_delete (ddsh);
				if (cc) 
					file_err (cc, ddsh, "DBDELETE");
			}
			cc = find_rec (ddsh, &ddsh_rec, NEXT, "r");
		}
		abc_selfield (ddsh, "ddsh_id_no");
		abc_selfield (ddln, "ddln_id_no");
		abc_selfield (ddgd, "ddgd_id_no");
	}

	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (!newOrder) 
	{	
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (lcount [PRISCN] == 0 || !strcmp (updateFlag, DELETEFLAG)) 
		{
			print_at (PrintOff++,0, ML (mlDdMess046));

			abc_selfield (pohr, "pohr_hhdd_hash");
			pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
			cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
			while (!cc && pohr_rec.hhdd_hash == ddhr_rec.hhdd_hash)
			{
				print_at (PrintOff++,0, ML (mlDdMess017), pohr_rec.pur_ord_no);
				cc = abc_delete (pohr);
				if (cc)
					file_err (cc, pohr, "DBDELETE");
				cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
			}
			abc_unlock (pohr);
			abc_selfield (ddsh, "ddsh_id_no3");
			abc_selfield (ddgd, "ddgd_hhds_hash");
	
			if (envVarFeInstall)
				UpdateFwdExch (ord_total, updateFlag);

			ddsh_rec.hhdd_hash = ddhr_rec.hhdd_hash;
			ddsh_rec.hhds_hash = 0L;
			cc = find_rec (ddsh, &ddsh_rec, GTEQ, "u");
			while (!cc && ddsh_rec.hhdd_hash == ddhr_rec.hhdd_hash)
			{
				ddgd_rec.hhds_hash = ddsh_rec.hhds_hash;
				cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
				while (!cc && ddgd_rec.hhds_hash == ddsh_rec.hhds_hash)
				{
					cc = abc_delete (ddgd);
					if (cc) 
						file_err (cc, ddgd, "DBDELETE");
					cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
				}
				cc = abc_delete (ddsh);
				if (cc) 
					file_err (cc, ddsh, "DBDELETE");
				cc = find_rec (ddsh, &ddsh_rec, NEXT, "u");
			}
			abc_selfield (ddsh, "ddsh_id_no");
			abc_selfield (ddgd, "ddgd_id_no");

			print_at (PrintOff++,0, ML (mlDdMess016), ddhr_rec.order_no);
			cc = abc_delete (ddhr);
			if (cc)
				file_err (cc, ddhr, "DBDELETE");
		}
		else
			print_at (PrintOff++,0, ML (mlDdMess047));
	}
	else
		print_at (PrintOff++,0, ML (mlDdMess048));


	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no,
		"RO", 
		0, 
		cumr_rec.hhcu_hash, 
		0L, 
		0L, 
		(double) 0.00
	);

	recalc_sobg ();
    PauseForKey (PrintOff++, 0, ML (mlStdMess042), 0);

	/*
	 * Calc Totals of Gst etc for ddhr
	 */
	CalcExtend (TRUE);

	ddhr_rec.dt_required = local_rec.lsystemDate;
	ddhr_rec.exch_rate   = local_rec.exch_rate;
	ddhr_rec.dt_required = local_rec.date_reqd;
	strcpy (ddhr_rec.stat_flag, updateFlag);

	if (lcount [PRISCN] != 0) 
	{
		cc = abc_update (ddhr, &ddhr_rec);
	
		if (cc)
			file_err (cc, ddhr, "DBUPDATE");
	}
	else
		abc_unlock (ddhr);


	if (envVarFeInstall)
		UpdateFwdExch (ord_total, updateFlag);

	if (lcount [PRISCN] != 0 && strcmp (updateFlag, DELETEFLAG)) 
	{
        sprintf (err_str,
                 ML (mlDdMess049), 
                 strcmp (updateFlag, PENDINGFLAG) ? "confirmed" : "pro-forma");
        crsr_on ();
        key =  prmptmsg (err_str, "YyNn", 0, PrintOff++);
        crsr_off ();
		if (!printerNumber && (key == 'Y' || key == 'y'))
		{
			printerNumber = get_lpno (0);
			clear ();
    		PrintOff = 0;
		}
	
		if (printerNumber && (key == 'Y' || key == 'y'))
		{
			print_at (PrintOff++,0, ML (mlDdMess050), ddhr_rec.order_no);

			if (!strcmp (updateFlag, ACTIVEFLAG))
				strcpy (printProgram, "dd_ord_conf");
			else
				strcpy (printProgram, "dd_qt_prt");

			if ((pout = popen (printProgram,"w")) == 0)
			{
				sprintf (err_str, "Error in %s during (POPEN)",printProgram);
				sys_err (err_str, errno, PNAME);
			}
			fprintf (pout,"%d\n",printerNumber);
			fprintf (pout,"S\n");
#ifdef GVISION
			Remote_fflush (pout);
#else
			fflush (pout);
#endif
			fprintf (pout,"%ld\n", ddhr_rec.hhdd_hash);
#ifdef GVISION
			Remote_fflush (pout);
#else
			fflush (pout);
#endif
			pclose(pout);
		}
	}

	if (envVarPoPrint && strcmp (updateFlag, DELETEFLAG))
	{
		if (lcount [ORDSCN])
		{
            sprintf (err_str,
                     ML (mlDdMess051), 
                     strcmp (updateFlag, PENDINGFLAG) ? "active" : "pro-forma");
            crsr_on ();
            key =  prmptmsg (err_str, "YyNn", 0, PrintOff++);
			crsr_off ();
			if (!printerNumber && (key == 'Y' || key == 'y'))
				printerNumber = get_lpno (0);
	
			if (printerNumber && (key == 'Y' || key == 'y'))
			{
				abc_selfield (pohr, "pohr_hhdd_hash");
				pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
				cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
				while (!cc && pohr_rec.hhdd_hash == ddhr_rec.hhdd_hash)
				{
					if ((pout = popen (poPrintProgram,"w")) == 0)
					{
						sprintf (err_str, "Error in %s during (POPEN)",poPrintProgram);
						sys_err (err_str, errno, PNAME);
					}
					fprintf (pout,"%d\n",printerNumber);
					fprintf (pout,"S\n");
#ifdef GVISION
					Remote_fflush (pout);
#else
					fflush (pout);
#endif
					fprintf (pout,"%ld\n",pohr_rec.hhpo_hash);
#ifdef GVISION
					Remote_fflush (pout);
#else
					fflush (pout);
#endif
					pclose(pout);

					cc = find_rec (pohr, &pohr_rec, NEXT, "r");
				}
			}
		}
	}
	return (TRUE);
}

void
UpdateFwdExch (
	double   ord_total,
	char	*updateFlag)
{
	if (!strcmp (updateFlag, DELETEFLAG)) /* If delete order */
	{
		cc = abc_delete (feln);
		if (cc)
			file_err (cc, feln, "DBDELETE");

		cc = abc_update (fehr, &fehr_rec);
		if (cc)
			file_err (cc, fehr, "DBUPDATE");
		
		if (!strcmp (fehr2_rec.cont_no, fehr_rec.cont_no)
		&&  !strcmp (fehr2_rec.cont_no, "      "))
		{
			strcpy (fehr_rec.cont_no, fehr2_rec.cont_no);
			cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, fehr, "DBFIND");

			fehr_rec.val_avail = fehr2_rec.val_avail;
			cc = abc_update (fehr, &fehr_rec);
			if (cc)
				file_err (cc, fehr, "DBUPDATE");
		}
	}
	else
	{
		if (strcmp (ddhr_rec.fwd_exch, "      ")) /* FEC Set */
		{
			feln_rec.hhfe_hash = fehr_rec.hhfe_hash;
			feln_rec.value     = ord_total;
			cc = abc_update (feln, &feln_rec);
			if (cc)
				file_err (cc, feln, "DBUPDATE");

			fehr_rec.val_avail -= ord_total;

			cc = abc_update (fehr, &fehr_rec);
			if (cc)
				file_err (cc, fehr, "DBUPDATE");
		}
		else
		{
			cc = abc_delete (feln);
			if (cc)
				file_err (cc, feln, "DBDELETE");
		}


		if (strcmp (fehr2_rec.cont_no, fehr_rec.cont_no)
		&&  strcmp (fehr2_rec.cont_no, "      "))
		{
			strcpy (fehr_rec.cont_no, fehr2_rec.cont_no);
			cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, fehr, "DBFIND");

			fehr_rec.val_avail = fehr2_rec.val_avail;
			cc = abc_update (fehr, &fehr_rec);
			if (cc)
				file_err (cc, fehr, "DBUPDATE");
		}
	}
}

void
GetDdhrOrderNo (void)
{
	long	ord_no;

	if (!strcmp (ddhr_rec.order_no,"00000000") || 
	     !strcmp (ddhr_rec.order_no,"        ") ||
		 !strlen (ddhr_rec.order_no)) 
	{
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no,  comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		/*
		 * Check if Order Already Allocated.	If it has been then skip
		 */
		open_rec (ddhr2, ddhr_list, DDHR_NO_FIELDS, "ddhr_id_no2");

		while (CheckDdhr () == 0);

		abc_fclose (ddhr2);

		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBFIND");

		ord_no =  esmr_rec.nx_dd_order;

		sprintf (err_str, "%08ld",  ord_no);
		sprintf (ddhr_rec.order_no, "%-8.8s", err_str);
		local_rec.dflt_order_no = 0L;
	}
	else
	{
		ord_no = atol (ddhr_rec.order_no);
		local_rec.dflt_order_no = ord_no + 1L;
	}
}

void
AddDdhrRecord (
	char*  updateFlag)
{
	print_at (PrintOff++,0, ML (mlDdMess052), ddhr_rec.order_no);

	strcpy (ddhr_rec.co_no, comm_rec.co_no);
	strcpy (ddhr_rec.br_no, comm_rec.est_no);
	strcpy (ddhr_rec.stat_flag, updateFlag);
	ddhr_rec.hhcu_hash 	  = cumr_rec.hhcu_hash;
	ddhr_rec.exch_rate    = local_rec.exch_rate;
	ddhr_rec.dt_required  = local_rec.date_reqd;

	cc = abc_add (ddhr, &ddhr_rec);
	if (cc) 
	{
		strcpy (ddhr_rec.order_no, "        ");
		GetDdhrOrderNo ();
		cc = abc_add (ddhr, &ddhr_rec);
		if (cc)
			file_err (cc, ddhr, "DBADD");
	}
	cc = find_rec (ddhr, &ddhr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, ddhr, "DBFIND");

	abc_unlock (esmr);
}

void
AddDdshRecord (
	long   hhddHash, 
	long   hhsuHash, 
	char	*updateFlag, 
 	struct DDGD_PTR *lcl_ptr)
{
	int		cat_idx;

	ddsh_rec.hhdd_hash = hhddHash;
	ddsh_rec.hhsu_hash = hhsuHash;
	strcpy (ddsh_rec.ship_no, " 1");
	cc = find_rec (ddsh, &ddsh_rec, EQUAL, "u");
	if (cc)
	{
		if (!strcmp (updateFlag, DELETEFLAG))
			return;

		cc = abc_add (ddsh, &ddsh_rec);
		if (cc) 
			file_err (cc, ddsh, "DBADD");

		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "u");
		if (cc)
			file_err (cc, ddsh, "DBFIND");
	}

	if (lcl_ptr != DDGD_NULL)
	{
		for (cat_idx = FOB; cat_idx <= OT4 ; cat_idx++)
		{
			strcpy (ddgd_rec.co_no,	comm_rec.co_no);
			ddgd_rec.hhds_hash = ddsh_rec.hhds_hash;
			ddgd_rec.line_no = cat_idx;
			cc = find_rec (ddgd, &ddgd_rec, EQUAL, "u");
			if (cc)
			{
				sprintf (ddgd_rec.category,"%-20.20s", 
							 lcl_ptr->ddgd_array [cat_idx].category);
				sprintf (ddgd_rec.allocation,	"%-1.1s", 
					         lcl_ptr->ddgd_array [cat_idx].allocation);
				sprintf (ddgd_rec.currency,	"%-3.3s",
					         lcl_ptr->ddgd_array [cat_idx].currency);
				ddgd_rec.fgn_value 	= 
							 lcl_ptr->ddgd_array [cat_idx].fgn_value;
				ddgd_rec.exch_rate		= 
							 lcl_ptr->ddgd_array [cat_idx].exch_rate;
				ddgd_rec.loc_value 	= 
							 lcl_ptr->ddgd_array [cat_idx].loc_value;
				if (strcmp (updateFlag, DELETEFLAG))
				{
					cc = abc_add (ddgd, &ddgd_rec);
					if (cc) 
						file_err (cc, ddgd, "DBADD");
				}
			}
			else
			{
				sprintf (ddgd_rec.category,"%-20.20s", 
							 lcl_ptr->ddgd_array [cat_idx].category);

				sprintf (ddgd_rec.allocation,	"%-1.1s", 
					         lcl_ptr->ddgd_array [cat_idx].allocation);

				sprintf (ddgd_rec.currency,	"%-3.3s",
					         lcl_ptr->ddgd_array [cat_idx].currency);

				ddgd_rec.fgn_value 	= lcl_ptr->ddgd_array [cat_idx].fgn_value;
				ddgd_rec.exch_rate	= lcl_ptr->ddgd_array [cat_idx].exch_rate;
				ddgd_rec.loc_value 	= lcl_ptr->ddgd_array [cat_idx].loc_value;
				if (strcmp (updateFlag, DELETEFLAG))
				{
					cc = abc_update (ddgd, &ddgd_rec);
					if (cc) 
						file_err (cc, ddgd, "DBUPDATE");
				}
				else
				{
					cc = abc_delete (ddgd);
					if (cc) 
						file_err (cc, ddgd, "DBDELETE");
				}
			}
		}
	}
	ddsh_rec.hhdd_hash = hhddHash;
	ddsh_rec.hhsu_hash = hhsuHash;
	strcpy (ddsh_rec.ship_no, " 1");
	if (lcl_ptr != DDGD_NULL)
		strcpy (ddsh_rec.cost_flag, "Y");
	else
		strcpy (ddsh_rec.cost_flag, "N");

	ddsh_rec.con_no =		dflt_ship_rec.con_no;
	ddsh_rec.due_date  =	0L;
	ddsh_rec.date_load =	dflt_ship_rec.date_load;
	ddsh_rec.ship_depart =	dflt_ship_rec.date_depart;
	ddsh_rec.ship_arrive =	dflt_ship_rec.date_arrive;
	strcpy (ddsh_rec.ship_method,	dflt_ship_rec.ship_method);
	strcpy (ddsh_rec.vessel,		dflt_ship_rec.vessel);
	strcpy (ddsh_rec.space_book,	dflt_ship_rec.space_book);
	strcpy (ddsh_rec.carrier,		dflt_ship_rec.carrier);
	strcpy (ddsh_rec.book_ref,		dflt_ship_rec.book_ref);
	strcpy (ddsh_rec.bol_no,		dflt_ship_rec.bol_no);
	strcpy (ddsh_rec.airway,		dflt_ship_rec.airway);
	strcpy (ddsh_rec.con_rel_no,	dflt_ship_rec.con_rel_no);
	strcpy (ddsh_rec.packing,		dflt_ship_rec.packing);
	strcpy (ddsh_rec.port_orig,		dflt_ship_rec.port_orig);
	strcpy (ddsh_rec.dept_orig,		dflt_ship_rec.dept_orig);
	strcpy (ddsh_rec.port_dsch,		dflt_ship_rec.port_dsch);
	strcpy (ddsh_rec.port_dest,		dflt_ship_rec.port_dest);
	strcpy (ddsh_rec.mark0,			dflt_ship_rec.marks [0]);
	strcpy (ddsh_rec.mark1,			dflt_ship_rec.marks [1]);
	strcpy (ddsh_rec.mark2,			dflt_ship_rec.marks [2]);
	strcpy (ddsh_rec.mark3,			dflt_ship_rec.marks [3]);
	strcpy (ddsh_rec.mark4,			dflt_ship_rec.marks [4]);
	strcpy (ddsh_rec.stat_flag,		updateFlag);

	if (strcmp (updateFlag, DELETEFLAG))
	{
		cc = abc_update (ddsh, &ddsh_rec);
		if (cc) 
			file_err (cc, ddsh, "DBUPDATE");
	}
	else
	{
		cc = abc_delete (ddsh);
		if (cc) 
			file_err (cc, ddsh, "DBDELETE");
	}
}

/*
 * Check Value of Order will existing value of orders.
 */
double
PurchaseOrderValue (void)
{
	double	porder_val = 0.00;
	double	o_total = 0.00;
	int		i;

	for (i = 0;i < lcount [ORDSCN];i++) 
	{
		o_total = (double) store [i].qty;
		o_total *= out_cost (store [i].land_cst, store [i]._outer);

		porder_val += o_total;
	}
	return (porder_val);
}

void
AddPohrRecord (
	long	hhddHash, 
	long	hhsuHash, 
	char	*updateFlag, 
	int		wk_line)
{
	char	*tptr;
	int 	i = 0;

	ReadSumr (hhsuHash);

	if (envVarPoAppFlag)
	{
		if (PurchaseOrderValue () > envVarPoAppVal)
			heldOrder = TRUE;
	}
	else
		heldOrder = FALSE;

	abc_selfield (pohr, "pohr_hhdd_hash");
	pohr_rec.hhdd_hash = hhddHash;
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && pohr_rec.hhdd_hash == hhddHash)
	{
		if (pohr_rec.hhsu_hash == hhsuHash)
		{
			abc_selfield (pohr, "pohr_id_no");
			cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
			if (cc)
				file_err (cc, pohr, "DBFIND");

			print_at (PrintOff++,0, ML (mlDdMess053),
						   pohr_rec.pur_ord_no, clip (sumr_rec.crd_name));

			if (local_rec.date_reqd > 0L)
				pohr_rec.due_date = local_rec.date_reqd;
			else
				pohr_rec.due_date = ddhr_rec.dt_raised;

			strcpy (pohr_rec.req_usr,   ddhr_rec.req_usr);
			strcpy (pohr_rec.reason,    ddhr_rec.reason);
			strcpy (pohr_rec.stdin1, ddhr_rec.stdin1);
			strcpy (pohr_rec.stdin2, ddhr_rec.stdin2);
			strcpy (pohr_rec.stdin3, ddhr_rec.stdin3);
			strcpy (pohr_rec.drop_ship, "Y");
			CatIntoPohr ();
			if (!strcmp (updateFlag, PENDINGFLAG))
			{
				strcpy (pohr_rec.status, PENDINGFLAG);
				cc = abc_update (pohr, &pohr_rec);
				if (cc)
					file_err (cc, pohr, "DBUPDATE");
			}
			if (!strcmp (updateFlag, ACTIVEFLAG))
			{
				strcpy (pohr_rec.status, (heldOrder) ? "U" : "O");
				cc = abc_update (pohr, &pohr_rec);
				if (cc)
					file_err (cc, pohr, "DBUPDATE");
			}
			return;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	abc_selfield (pohr, "pohr_id_no");
	
    if (!envVarPoNumGen)
    {
		open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");

		open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
		while (CheckPohr (++comr_rec.nx_po_no) == 0);
		abc_fclose (pohr2);

		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		sprintf (err_str, "%ld", comr_rec.nx_po_no);
		sprintf (pohr_rec.pur_ord_no, "DD%-13.13s", zero_pad (err_str, 13));

		abc_fclose (comr);
    }
    else
    {
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy  (esmr_rec.co_no,  comm_rec.co_no);
		sprintf (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		/*
		 * Check if Order No Already Allocated if it has been then skip.
		 */
		open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
		while (CheckPohr (++esmr_rec.nx_pur_ord_no) == 0);
		abc_fclose (pohr2);

		cc = abc_update (esmr, &esmr_rec);
		if (cc)
	    	file_err (cc, esmr, "DBUPDATE");
		
		sprintf (err_str, "%ld", esmr_rec.nx_pur_ord_no);
		sprintf (pohr_rec.pur_ord_no, "DD%-13.13s", zero_pad (err_str, 13));

		abc_fclose (esmr);
    }

	print_at (PrintOff++,0, ML (mlDdMess054), 
		   pohr_rec.pur_ord_no, clip (sumr_rec.crd_name));

	strcpy (pohr_rec.type,"O");
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.curr_code,sumr_rec.curr_code);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	pohr_rec.curr_rate = store [wk_line].supp_exch;
	if (!strcmp (updateFlag, PENDINGFLAG))
		strcpy (pohr_rec.status, PENDINGFLAG);
	else
		strcpy (pohr_rec.status, (heldOrder) ? "U" : "O");

	sprintf (pohr_rec.op_id, "%-14.14s", currentUser);

	pohr_rec.date_create = TodaysDate ();
	strcpy (pohr_rec.time_create, TimeHHMM ());
	pohr_rec.date_raised = local_rec.lsystemDate;
	strcpy (pohr_rec.contact, sumr_rec.cont_name);
	strcpy (pohr_rec.term_order, ddhr_rec.sell_terms);
	strcpy (pohr_rec.stat_flag,"N");
	if (ddhr_rec.dt_required > 0L)
		pohr_rec.due_date = ddhr_rec.dt_required;
	else
		pohr_rec.due_date = ddhr_rec.dt_raised;

	/*
	 * default pay terms
	 */
	for (i = 0; strlen (p_terms [i]._pcode); i++)
	{
		if (!strncmp (sumr_rec.pay_terms, p_terms [i]._pcode,
				     strlen (p_terms [i]._pcode)))
		{
			sprintf (pohr_rec.sup_term_pay, "%-30.30s",p_terms [i]._pterm);
			break;
		}
	}

	/*
	 * Shipment Default. A(ir) / L(and) / S(ea) 
	 */
	tptr = chk_env ("PO_SHIP_DEFAULT");
	if (tptr == (char *) 0)
		sprintf (pohr_rec.ship_method, "S");
	else
	{
		switch (*tptr)
		{
		case	'S':
		case	's':
			sprintf (pohr_rec.ship_method, "S");
			break;

		case	'L':
		case	'l':
			sprintf (pohr_rec.ship_method, "L");
			break;

		case	'A':
		case	'a':
			sprintf (pohr_rec.ship_method, "A");

		default:
			sprintf (pohr_rec.ship_method, "S");
			break;
		}
	}
	CatIntoPohr ();
	strcpy (pohr_rec.drop_ship,"Y");
	strcpy (pohr_rec.req_usr,   ddhr_rec.req_usr);
	strcpy (pohr_rec.reason,    ddhr_rec.reason);
	strcpy (pohr_rec.stdin1, ddhr_rec.stdin1);
	strcpy (pohr_rec.stdin2, ddhr_rec.stdin2);
	strcpy (pohr_rec.stdin3, ddhr_rec.stdin3);

	cc = abc_add (pohr,&pohr_rec);
	if (cc) 
		file_err (cc, pohr, "DBADD");

	cc = find_rec (pohr, &pohr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, pohr, "DBFIND");
}



/*========================================
| Update or Add lines to Purchase order. |
========================================*/
void
AddPolnRecord (
	 long   hhplHash, 
	 long   hhsuHash, 
	 int    store_line, 
	 char*  updateFlag)
{
	int 	add_item = FALSE;
	float	xx_qty = 0.00;

	abc_selfield (poln, "poln_hhpl_hash");

	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, EQUAL, "u");
	if (cc)
		add_item = TRUE;
	else
		add_item = FALSE;

	AddDdshRecord (ddhr_rec.hhdd_hash, hhsuHash, updateFlag,
				   store [store_line].ddgd_ptr);
	AddPohrRecord (ddhr_rec.hhdd_hash, hhsuHash, updateFlag, store_line);

	if (add_item)
	{
		poln_rec.qty_ord = local_rec.qty;
		poln_rec.qty_rec = 0.00;
	}
	else
	{
		xx_qty = poln_rec.qty_ord - poln_rec.qty_rec;
		xx_qty -= local_rec.qty;
		poln_rec.qty_ord -= xx_qty;
	}
	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = store_line;
	
	poln_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	poln_rec.hhum_hash 	= inmr_rec.std_uom;
	poln_rec.hhcc_hash 	= local_rec.hhcc_hash;
	poln_rec.exch_rate 	= store [store_line].supp_exch;
	poln_rec.hhlc_hash 	= 0;
	poln_rec.case_no 	= 0;
	if (local_rec.date_reqd > 0L)
		poln_rec.due_date = local_rec.date_reqd;
	else
		poln_rec.due_date = ddhr_rec.dt_raised;

	poln_rec.reg_pc 	= store [store_line].discArray [0];
	poln_rec.disc_a 	= store [store_line].discArray [1];
	poln_rec.disc_b 	= store [store_line].discArray [2];
	poln_rec.disc_c 	= store [store_line].discArray [3];
	poln_rec.cumulative	= store [store_line].cumulative;

	if (store [store_line].supp_conv == 0.00)
		store [store_line].supp_conv = 1.00;

	poln_rec.grs_fgn_cst = DOLLARS (local_rec.grs_fgn * 
									store [store_line]._outer);

	poln_rec.fob_fgn_cst = DOLLARS (local_rec.net_fob * 
									store [store_line]._outer);
	
	poln_rec.fob_nor_cst = DOLLARS (((local_rec.net_fob
								    * store [store_line]._outer)
								    / store [store_line].supp_exch)
									+ store [store_line].amt_fai);
	poln_rec.frt_ins_cst = DOLLARS (store [store_line].amt_fai);
	poln_rec.duty 		 = DOLLARS (store [store_line].amt_dty);
	poln_rec.licence 	 = 0;
	poln_rec.lcost_load  = DOLLARS (store [store_line].amt_oth);
	poln_rec.land_cst 	 = DOLLARS (local_rec.land_cst);
	strcpy (poln_rec.item_desc,inmr_rec.description);
	strcpy (poln_rec.cat_code, store [store_line]._category);
	strcpy (poln_rec.stat_flag,"B");

	if (add_item)
	{
		sprintf (poln_rec.serial_no,"%25.25s"," ");
		strcpy (poln_rec.pur_status, (heldOrder) ? "U" : "O");
		poln_rec.qty_rec = 0.00;
		if (poln_rec.qty_ord != 0.00 && strcmp (updateFlag, DELETEFLAG))
		{
			cc = abc_add (poln,&poln_rec);
			if (cc) 
				file_err (cc, poln, "DBADD");

			abc_selfield (poln, "poln_id_no");
	
			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			poln_rec.line_no = store_line;
			sprintf (err_str, "DBFIND %d", __LINE__);
			cc = find_rec (poln, &poln_rec, EQUAL, "r");
			if (cc) 
				file_err (cc, poln, err_str);
		}
	}
	else
	{
		if (poln_rec.qty_ord == 0.00 || !strcmp (updateFlag, DELETEFLAG))
		{
			/*
			 * delete poln ddln
			 */
			cc = abc_delete (poln);
			if (cc)
				file_err (cc, poln, "DBDELETE");
			return;
		}
		else
		{
			/*
			 * Update existing order.
			 */
			poln_rec.line_no = store_line;
			cc = abc_update (poln, &poln_rec);
			if (cc) 
				file_err (cc, poln, "DBUPDATE");

		}
		abc_unlock (poln);
	}
	store [store_line].hhplHash = poln_rec.hhpl_hash;
}

void
CalcExtend (
	int    cal_line)
{
	int	i;
	double	wk_value = 0.00;
	double	value;

	if (cal_line)
	{
		scn_set (PRISCN);

		ord_total = 0.00;

		ddhr_rec.gross 	= 0.00;
		ddhr_rec.disc 	= 0.00;
		ddhr_rec.tax 	= 0.00;
		ddhr_rec.gst 	= 0.00;

		for (i = 0;i < lcount [PRISCN];i++)
		{
			getval (i);
			CalExtend (i);
			ddhr_rec.gross 	+= l_total;
			ddhr_rec.disc 	+= l_dis;
			ddhr_rec.tax 	+= l_tax;
			ddhr_rec.gst 	+= l_gst;
		}
	}

	if (notax)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	value = 0.00;

	wk_value *= value;
	ddhr_rec.gst += wk_value;
	
	ddhr_rec.gst = no_dec (ddhr_rec.gst);
	
	if (envVarDbNettUsed)
	{
		ord_total = ddhr_rec.gross + ddhr_rec.tax + 
					ddhr_rec.gst - ddhr_rec.disc; 
	}
}

int
LoadPoln (
	long   hhplHash)
{
	int			headerLoaded = FALSE;

	/*
	 * Set screen for putval. 
	 */
	scn_set (ORDSCN);
	
	abc_selfield (poln,"poln_hhpl_hash");

	poln_rec.hhpl_hash = hhplHash;
	cc = find_rec (poln,&poln_rec,EQUAL,"r");
	if (!cc)
	{
		if (headerLoaded == FALSE)
		{
			abc_selfield (pohr, "pohr_hhpo_hash");
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			abc_selfield (pohr, "pohr_id_no");
			if (cc)
				return (TRUE);

			headerLoaded = TRUE;
		}
		local_rec.supp_exch = pohr_rec.curr_rate;
		store [lcount [PRISCN]].supp_exch = pohr_rec.curr_rate;

		if (!ReadSumr (ddln_rec.hhsu_hash))
			return (TRUE);

		strcpy (store [lcount [PRISCN]].std_uom, "    ");
		strcpy (local_rec.std_uom, store [lcount [PRISCN]].std_uom);

		/*
		 * Get part number.
		 */
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			if (strcmp (inmr_rec.supercession, "                "))
			{
				abc_selfield (inmr, "inmr_id_no");
				FindSupercession (comm_rec.co_no, inmr_rec.supercession, TRUE);
				abc_selfield (inmr, "inmr_hhbr_hash");
			}
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			if (!find_rec (inum, &inum_rec, EQUAL, "r"))
			{
				strcpy (store [lcount [PRISCN]].std_uom, inum_rec.uom);
				strcpy (local_rec.std_uom, inum_rec.uom);
			}
		} 
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (local_rec.item_no, inmr_rec.item_no);

		GetWarehouse (0L);

		strcpy (store [lcount [PRISCN]].item_desc, inmr_rec.description);

		/*---------------------
		| Setup local record. |
		---------------------*/
		local_rec.qty 			= poln_rec.qty_ord - poln_rec.qty_rec;
		local_rec.cif_loc 		= CENTS (poln_rec.fob_nor_cst);
		local_rec.loc_fi 		= CENTS (poln_rec.frt_ins_cst);
		store [lcount [PRISCN]].amt_fai = CENTS (poln_rec.frt_ins_cst);
		local_rec.duty_val 		= CENTS (poln_rec.duty);
		store [lcount [PRISCN]].amt_dty = CENTS (poln_rec.duty);
		local_rec.oth 			= CENTS (poln_rec.lcost_load);
		store [lcount [PRISCN]].amt_oth = CENTS (poln_rec.lcost_load);
		local_rec.land_cst 		= CENTS (poln_rec.land_cst);
		store [lcount [PRISCN]]._cost_price 		= local_rec.land_cst;
		local_rec.due_date 		= poln_rec.due_date;

		store [lcount [PRISCN]].hhccHash = poln_rec.hhcc_hash;
		local_rec.hhcc_hash = store [lcount [PRISCN]].hhccHash;

		local_rec.grs_fgn = CENTS (poln_rec.grs_fgn_cst);
		local_rec.net_fob = CENTS (poln_rec.fob_fgn_cst); 

		store [lcount [PRISCN]].base_cost 		= poln_rec.grs_fgn_cst;
		store [lcount [PRISCN]].net_fob 		= local_rec.net_fob;
		store [lcount [PRISCN]].discArray [0] 	= poln_rec.reg_pc;
		store [lcount [PRISCN]].discArray [1] 	= poln_rec.disc_a;
		store [lcount [PRISCN]].discArray [2] 	= poln_rec.disc_b;
		store [lcount [PRISCN]].discArray [3] 	= poln_rec.disc_c;
		store [lcount [PRISCN]].cumulative   	= poln_rec.cumulative;

		strcpy (local_rec.view_disc, "N");
		
		strcpy (store [lcount [PRISCN]].supp_no,	sumr_rec.crd_no);
		strcpy (store [lcount [PRISCN]].supp_name,	sumr_rec.crd_name);
		strcpy (store [lcount [PRISCN]].supp_curr,	sumr_rec.curr_code);
		store [lcount [PRISCN]].supp_exch  = pocr_rec.ex1_factor;

		/*------------------------
		| get contract price ???
		------------------------*/
		if (cnch_rec.hhch_hash != 0L)
		{
			/*--------------------------------------------------------
			| Use ContCusPrice to determine if a valid contract line |
			| is available for this line. If so then _cont_status    |
			| will be non-zero and the appropriate cncd record will  |
			| be loaded.                                             |
			--------------------------------------------------------*/
			(void) ContCusPrice (cnch_rec.hhch_hash,
								 store [lcount [PRISCN]].hhbrHash,
								 store [lcount [PRISCN]].hhccHash,
								 cumr_rec.curr_code,
								 cnch_rec.exch_type,
								 FGN_CURR,
								 (float)local_rec.exch_rate);
			if (_cont_status)
			{
				if (cncd_rec.cost > 0.00 && 
					cncd_rec.hhsu_hash == sumr_rec.hhsu_hash)
				{
					local_rec.fob_cost = DOLLARS (cncd_rec.cost);
					store [lcount [PRISCN]].base_cost = local_rec.fob_cost;
					store [lcount [PRISCN]].cst_price = store [lcount [PRISCN]].base_cost;
					store [lcount [PRISCN]]._cont_cost = TRUE;
				}
			}
		}

		if (FindInis (poln_rec.hhbr_hash, ddln_rec.hhsu_hash))
		{
			/* inis *not* found! */
			store [lcount [PRISCN]].no_inis = TRUE;
			store [lcount [PRISCN]].weight = inmr_rec.weight;
			store [lcount [PRISCN]].volume = 0.00;
			store [lcount [PRISCN]].supp_lead = 0.00;
			store [lcount [PRISCN]].min_order = 0;
			store [lcount [PRISCN]].ord_multiple = 0;

			strcpy (local_rec.duty_code, inmr_rec.duty);
			if (store [lcount [PRISCN]]._cont_cost == FALSE)
			{
				local_rec.fob_cost = 0.00;
				store [lcount [PRISCN]].base_cost = 0.00;
			}
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (store [lcount [PRISCN]].supp_uom, inum_rec.uom);
				strcpy (local_rec.supp_uom, inum_rec.uom);
				store [lcount [PRISCN]].supp_conv = 1.00;
				local_rec.supp_conv = 1.00;
			}
			store [lcount [PRISCN]].supp_lead = 0.00;
		}
		else
		{
			store [lcount [PRISCN]].no_inis = FALSE;
			store [lcount [PRISCN]].weight = inis_rec.weight;
			store [lcount [PRISCN]].volume = inis_rec.volume;
			store [lcount [PRISCN]].supp_lead = inis_rec.lead_time;
			store [lcount [PRISCN]].min_order = inis_rec.min_order;
			store [lcount [PRISCN]].ord_multiple = inis_rec.ord_multiple;

			strcpy (local_rec.duty_code, inis_rec.duty);
			strcpy (inmr_rec.duty, inis_rec.duty);
			if (store [lcount [PRISCN]]._cont_cost == FALSE)
			{
				local_rec.fob_cost = inis_rec.fob_cost;
				store [lcount [PRISCN]].base_cost = inis_rec.fob_cost;
			}
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (store [lcount [PRISCN]].supp_uom, inum_rec.uom);
				strcpy (local_rec.supp_uom, inum_rec.uom);
				store [lcount [PRISCN]].supp_conv = inis_rec.pur_conv;
				local_rec.supp_conv = inis_rec.pur_conv;
			}
		}

		if (store [lcount [PRISCN]].supp_conv == 0.00)
		{
			local_rec.supp_conv = 1.00;
			store [lcount [PRISCN]].supp_conv = 1.00;
		}
		store [lcount [PRISCN]].upd_inis 		= FALSE;
	
		putval (lcount [PRISCN]);

		cc = find_rec (poln,&poln_rec,NEXT,"r");
	}
	else
		file_err (cc, poln, "DBFIND");

	return (FALSE);
}

int
ReadSumr (
	long   hhsuHash)
{
	abc_selfield (sumr, "sumr_hhsu_hash");

	sumr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sumr, "DBFIND");

	/*--------------------------
	| Find currency code file. |
	--------------------------*/
	FindPocr (sumr_rec.curr_code);

	/*--------------------
	| Find freight file. |
	--------------------*/
	FindPocf (sumr_rec.ctry_code);
	strcpy (local_rec.supp_no, sumr_rec.crd_no);

	abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	return (TRUE);
}

void
UpdateLine (
	int    lline)
{
	scn_set (ORDSCN);
	getval (lline);

	/*-------------------
	| Find duty record. |
	-------------------*/
	strcpy (store [lline].supp_uom, "    ");
	if (FindInis (store [lline].hhbrHash, sumr_rec.hhsu_hash))
	{
		/* inis *not* found! */
		store [lline].no_inis 	= TRUE;
		store [lline].supp_lead 	= 0.00;
		store [lline].weight 	= inmr_rec.weight;
		store [lline].volume 	= 0.00;
		store [lline].supp_conv 	= 1.00;

		strcpy (local_rec.duty_code, inmr_rec.duty);
		store [lline].cst_price = local_rec.fob_cost = 0.00;

		store [lline].min_order = 0;
		store [lline].ord_multiple = 0;

		/*-------------------------------------
		| Find part number for branch record. |
		-------------------------------------*/
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no,comm_rec.est_no);
		cc = find_rec (inei,&inei_rec,COMPARISON,"r");
		if (cc) 
			file_err (cc, inei,"DBFIND");

		if (store [lline]._cont_cost == FALSE)
		{
			local_rec.fob_cost = inis_rec.fob_cost;
			store [lline].cst_price = inei_rec.last_cost * pohr_rec.curr_rate;
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		if (!find_rec (inum, &inum_rec, EQUAL, "r"))
		{
			strcpy (store [lline].supp_uom, inum_rec.uom);
			strcpy (local_rec.supp_uom, inum_rec.uom);
			store [lline].supp_conv = 1.00;
			local_rec.supp_conv = 1.00;
		}
		else
		{
			strcpy (store [lline].supp_uom, "EACH");
			strcpy (local_rec.supp_uom, "EACH");
			store [lline].supp_conv = 1.0;
			local_rec.supp_conv = 1.0;
		}
	}
	else
	{
		store [lline].no_inis = FALSE;
		store [lline].weight = inis_rec.weight;
		store [lline].volume = inis_rec.volume;

		store [lline].supp_lead = inis_rec.lead_time;
		store [lline].supp_conv = inis_rec.pur_conv;

		store [lline].min_order = inis_rec.min_order;
		store [lline].ord_multiple = inis_rec.ord_multiple;

		inum_rec.hhum_hash	=	inis_rec.sup_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (store [lline].supp_uom, inum_rec.uom);
			strcpy (local_rec.supp_uom, inum_rec.uom);
		}
		else
		{
			strcpy (store [lline].supp_uom, "EACH");
			strcpy (local_rec.supp_uom, "EACH");
			store [lline].supp_conv = 1.0;
			local_rec.supp_conv = 1.0;
		}

		strcpy (local_rec.duty_code,inis_rec.duty);

		strcpy (inmr_rec.duty,local_rec.duty_code);
		if (store [lline]._cont_cost == FALSE)
		{
			local_rec.fob_cost = inis_rec.fob_cost;
			store [lline].cst_price = inis_rec.fob_cost;
		}
	}

	if (store [lline].supp_conv == 0.00)
		store [lline].supp_conv = 1.00;

	local_rec.supp_exch = pohr_rec.curr_rate;
	putval (lline);
}

void
CatIntoPohr (void)
{
	int		addSize = 0;
	char	address [125];

	sprintf (address, "%124.124s", " ");
	sprintf (pohr_rec.delin1, "SHIP TO : %-40.40s", ddhr_rec.del_name);

	/*
	 * if add 1 + add 2 < 60 put on line 1 and  add 3 on line 2
	 * OR if add 2 + add 3 < 60  put on line 2 and add 1 on line 1
	 */
	addSize = strlen (clip (ddhr_rec.del_add1));
	addSize += strlen (clip (ddhr_rec.del_add2));
	if (addSize < 60)
	{
		strcpy (address, clip (ddhr_rec.del_add1));
		strcat  (address, " ");
		strcat (address, clip (ddhr_rec.del_add2));
		sprintf (pohr_rec.delin2, "%-60.60s", address);
		sprintf (pohr_rec.delin3, "%-60.60s", ddhr_rec.del_add3);
		if (cur_screen == TLRSCN)
		{
			DSP_FLD ("del1");
			DSP_FLD ("del2");
			DSP_FLD ("del3");
		}
		return;
	}

	addSize = strlen (clip (ddhr_rec.del_add2));
	addSize += strlen (clip (ddhr_rec.del_add3));
	if (addSize < 60)
	{
		strcpy (address, clip (ddhr_rec.del_add2));
		strcat  (address, " ");
		strcat (address, clip (ddhr_rec.del_add3));
		sprintf (pohr_rec.delin3, "%-60.60s", address);
		sprintf (pohr_rec.delin2, "%-60.60s", ddhr_rec.del_add1);
		if (cur_screen == TLRSCN)
		{
			DSP_FLD ("del1");
			DSP_FLD ("del2");
			DSP_FLD ("del3");
		}
		return;
	}

	/*-------------------------
	| ELSE 3 fields of 40
	| to fit in 2 fields of 60
	--------------------------*/
	strcpy (address, "");
	strcat (address, clip (ddhr_rec.del_add1)); strcat  (address, " ");
	strcat (address, clip (ddhr_rec.del_add2)); strcat  (address, " ");
	strcat (address, clip (ddhr_rec.del_add3)); strcat  (address, " ");

	sprintf (pohr_rec.delin2, "%-60.60s", address);
	sprintf (pohr_rec.delin3, "%-60.60s", address + 60);

	if (cur_screen == TLRSCN)
	{
		DSP_FLD ("del1");
		DSP_FLD ("del2");
		DSP_FLD ("del3");
	}
}
/*---------------------------------------
| Preliminary check on on-cost supplier |
---------------------------------------*/
static int
SuppSlctFunc (
	int		iUnused,
	struct tag_KEY_TAB*    psUnused)
{
	char	supp_buf [100];
	char	tmp_supp [7];


	tab_get ("supp", supp_buf, CURRENT, 0);
	sprintf (tmp_supp, "%-6.6s", &supp_buf [0]);
	strcpy (sumr_rec.co_no,  comm_rec.co_no);
	strcpy (sumr_rec.est_no, envVarCrCo);
	strcpy (sumr_rec.crd_no, tmp_supp);
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc) 
	{
		print_mess (ML (mlStdMess022));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE); 
	}
	/*--------------------------
	| Find currency code file. |
	--------------------------*/
	FindPocr (sumr_rec.curr_code);

	/* exit_choose = TRUE; */
	return (FN16);
}

static	int
ExitFunc (
 int                    iUnused,
 struct tag_KEY_TAB*    psUnused)
{
	sumr_rec.hhsu_hash = -1;
	return (FN16);
}

static	int
RestartFunc (
 int                    iUnused,
 struct tag_KEY_TAB*    psUnused)
{
	sumr_rec.hhsu_hash = -1;
	restart = TRUE;
	return (FN1);
}

/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (void)
{
	char	wkCode [4];

	_work_open (3,0,60);
	save_rec ("#No.","#Instruction description.");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = 0;

	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");
	while (!cc && !strcmp (exsi_rec.co_no,comm_rec.co_no))
	{
		sprintf (wkCode, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wkCode, err_str);
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
	cc = find_rec (exsi,&exsi_rec,EQUAL,"r");
	if (cc)
		file_err (cc, exsi, "DBFIND");
}

/*
 * Search for DD orders.
 */
void
SrchDdhr (
	char	*keyValue)
{
	abc_selfield (ddhr, "ddhr_id_no");
	_work_open (8,0,16);
	save_rec ("#Order No", "#Cust Order");
	strcpy  (ddhr_rec.co_no,    comm_rec.co_no);
	strcpy  (ddhr_rec.br_no,    comm_rec.est_no);
	sprintf (ddhr_rec.order_no, "%-8.8s", keyValue);
	if (!strcmp (local_rec.cust_no, "      "))
	{
		ddhr_rec.hhcu_hash = 0L;
		abc_selfield (ddhr, "ddhr_id_no2");
		cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");

		while (!cc && 
			   !strcmp  (ddhr_rec.co_no,    comm_rec.co_no) && 
			   !strcmp  (ddhr_rec.br_no,    comm_rec.est_no) && 
			   !strncmp (ddhr_rec.order_no, keyValue, strlen (keyValue)))
		{
			if (!strcmp (ddhr_rec.stat_flag, PENDINGFLAG))
			{
				cc = save_rec (ddhr_rec.order_no, ddhr_rec.cus_ord_ref);
				if (cc)
					break;
			}
			cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
		}
		abc_selfield (ddhr, "ddhr_id_no");
	}
	else
	{
		ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");

		while (!cc && 
			   !strcmp  (ddhr_rec.co_no,    comm_rec.co_no) && 
			   !strcmp  (ddhr_rec.br_no,    comm_rec.est_no) && 
			   !strncmp (ddhr_rec.order_no, keyValue, strlen (keyValue)))
		{
			if (cumr_rec.hhcu_hash > 0L &&
			     cumr_rec.hhcu_hash != ddhr_rec.hhcu_hash)
				break;
			
			if (!strcmp (ddhr_rec.stat_flag, PENDINGFLAG) &&
			    (cumr_rec.hhcu_hash == ddhr_rec.hhcu_hash || 
			     cumr_rec.hhcu_hash == 0L))
			{
				cc = save_rec (ddhr_rec.order_no, ddhr_rec.cus_ord_ref);
				if (cc)
					break;
			}
			cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
		}
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}
/*
 * Search for FEC
 */
void
SrchFehr (
	char	*key_val)
{
    _work_open (5,0,40);
	save_rec ("#Cont No","#Bank  Currency ");                       
	strcpy  (fehr_rec.co_no,   comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-3.3s", key_val);
	cc = find_rec (fehr, &fehr_rec, GTEQ, "r");
    while (!cc && !strcmp (fehr_rec.co_no, comm_rec.co_no) 
			   && !strncmp (fehr_rec.cont_no, key_val, strlen (key_val)))
    {
		if (!strcmp (fehr_rec.curr_code, cumr_rec.curr_code))
		{
			sprintf (err_str, "%-4.4s      ", fehr_rec.bank_id); 
		    cc = save_rec (fehr_rec.cont_no, err_str);                       
			if (cc)
				break;
		}
		cc = find_rec (fehr, &fehr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (fehr_rec.co_no,   comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, fehr, "DBFIND");
}



/*
 * Allocate memory for all_ddgd linked list
 */
struct	DDGD_PTR *
ddgd_alloc (void)
{
	struct	DDGD_PTR	*lcl_ptr;

	lcl_ptr = (struct DDGD_PTR *) malloc (sizeof (struct DDGD_PTR));

	if (lcl_ptr == DDGD_NULL)
       	sys_err ("Error allocating memory for ddgd list During (MALLOC)",errno,PNAME);
	else
		memcpy (lcl_ptr->ddgd_array, " ", sizeof (lcl_ptr->ddgd_array));
		
	return (lcl_ptr);
}

/*------------------
| Clear ddgd list. |
------------------*/
int
DdgdClear (
 struct DDGD_PTR *lcl_ptr)
{
	free (lcl_ptr);
	return (EXIT_SUCCESS);
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

int
FindCucc (
	int    _key,
	long   lastHhcu)
{
	if (_key == 0)
	{
		cc = find_rec (cucc,&cucc_rec,COMPARISON,"r");
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);
		return (EXIT_FAILURE);
	}

	if (lastHhcu != 0L)
	{
		/*-------------------------------------------------------
		| Find the NEXT / PREVIOUS record to the current one	|
		-------------------------------------------------------*/
		cc = find_rec (cucc,&cucc_rec,(_key == FN14) ? GTEQ : LTEQ,"r");
		if (!cc)
			cc = find_rec (cucc,&cucc_rec,(_key == FN14) ? NEXT : PREVIOUS,"r");

		/*-------------------------------------------
		| Woops, looks like we need to loop around	|
		-------------------------------------------*/
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
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = TodaysDate () + 30000L;
		cc = find_rec (cucc,&cucc_rec,LTEQ,"r");
	}

	if (cc || cucc_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
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
 int    _key)
{
	static	long	lastHhcu;
	char	comment [132];

	/*
	 * Only do anything when we are on screen 1 and	we've read a valid cumr.
 	 */	
	if (cur_screen != 1 || cumr_rec.hhcu_hash == 0L)
	{
		lastHhcu = 0L;
		return (EXIT_SUCCESS);
	}

	if (FindCucc (_key,lastHhcu))
		return (EXIT_SUCCESS);

	if (lastHhcu != cumr_rec.hhcu_hash)
		lastHhcu = cumr_rec.hhcu_hash;

	crsr_off ();
	box (0,17,132,1);
	sprintf (comment,"%-25.25s%-80.80s%-25.25s"," ",cucc_rec.comment," ");
	rv_pr (comment,1,18,1);
	crsr_on ();
    return (EXIT_SUCCESS);
}

int
SrchCudi (
	int		indx)
{
	char 	workString [256];

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
		sprintf (err_str, "%5d", cudi_rec.del_no);
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
 * Initialisation of strings in array for multilingual.
 */
void
InitML (void)
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

/*
 * This function frees the field prompt if it's allocated.
 */
void 
_discScnFreePrompt (
	int		Index)
{
    if (discScn [Index].fldPrompt != NULL) 
        free (discScn [Index].fldPrompt);         
}

/*
 * This one sets the prompt to a new value, frees the
 * previous string if necessary                     
 */
void 
_discScnSetPrompt (
	int		Index, 
	char	*FldPrompt)
{
    _discScnFreePrompt (Index); 
    discScn [Index].fldPrompt = (char *) malloc (strlen (FldPrompt) + 1); 
    strcpy (discScn [Index].fldPrompt, FldPrompt);         
}

/*
 * Set values to the fields of the structure
 */
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

/*
 * Clean up screen discount prompts.
 */
void 
CleanUpPrompt (void)
{
    _discScnFreePrompt (0);
    _discScnFreePrompt (1);
    _discScnFreePrompt (2);
    _discScnFreePrompt (3);
    _discScnFreePrompt (4);
    _discScnFreePrompt (5);
    _discScnFreePrompt (6);
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ol_invoice.c,v 5.6 2002/11/28 04:09:47 scott Exp $
|  Program Name  : (ol_invoice.c)
|  Program Desc  : (Generate Invoices / Credit Notes Directly)
|                  (Kitting Version)
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written : 16/03/89          |
|---------------------------------------------------------------------|
| $Log: ol_invoice.c,v $
| Revision 5.6  2002/11/28 04:09:47  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.5  2002/07/24 08:38:55  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/26 05:18:23  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/08/09 09:14:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:32:46  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:17  scott
| Update - LS10.5
|
| Revision 4.5  2001/05/24 02:05:09  scott
| Updated for changes in inal. Program does not require
|
| Revision 4.4  2001/04/05 23:37:40  scott
| Updated to use LTEQ and PREVIOUS calls to work with SQL and CISAM
|
| Revision 4.3  2001/03/27 17:13:18  robert
| updated as OpenSkWin is not defined in LS/10 GUI
|
| Revision 4.2  2001/03/27 06:36:38  scott
| Updated to change arguments passed to DbBalWin to avoid usage of read_comm ()
|
| Revision 4.1  2001/03/27 03:54:04  scott
| Updated to remove program shell of db_balwin and replaced with function DbBalWin
|
| Revision 4.0  2001/03/09 02:31:00  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/03/08 02:12:02  scott
| Updated to increase the delivery address number from 0-999 to 0-32000
| This change did not require a change to the schema
| As a general practice all programs have had app.schema added and been cleaned
|
=====================================================================*/
#define CCMAIN
char    *PNAME = "$RCSfile: ol_invoice.c,v $",
        *PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_invoice/ol_invoice.c,v 5.6 2002/11/28 04:09:47 scott Exp $";

extern	int		_win_func;


#define	ADJ_VAL(x)	 (adj_val (DOLLARS (x)) * 100.00);
#define USE_WIN 	1
#define MAXSCNS 	4
#define MAXWIDTH 	200
#define MAXLINES	70
#define TABLINES	7
#define	EXPORT		 (cohr_rec.ord_type [0] == 'E')

#define	K_NONE		0
#define	K_START		1
#define	K_ITEM		2
#define	K_END		3

#define	PRINT_LINES	1
#define	CALC_LINES	1
#define	SLEEP_TIME	2

#define	SCN_HEAD	1
#define	SCN_LINES	2
#define	SCN_TENDER	3
#define	SCN_PROOF	4
#define	ErrorReturn	1

#define	SERIAL_ITEM		(inmr_rec.serial_item [0] == 'Y')
#define	SR				store [line_cnt]
#define	SERIAL			(SR._hhwh_hash > 0L)
#define	BONUS			(SR._bonus [0] == 'Y')
#define	KIT_START		(SR._kit_flag == K_START)
#define	KIT_END			(SR._kit_flag == K_END)
#define	NO_COST			(SR._class [0] == 'N')
#define	NON_STOCK(x)	(store [x]._class [0] == 'Z')
#define OLD_INSF		(SR._old_insf [0] == 'Y')
#define MARGIN_OK   	(strcmp (envVar.SoMargin, "00"))
#define	MARG_MESS1		(envVar.SoMargin [0] == '0')
#define	MARG_MESS2		(envVar.SoMargin [0] == '1')
#define	MARG_MESS3		(envVar.SoMargin [0] == '2')
#define	KIT_ITEM		(SR._class [0] == 'K' && IS_ENTRY)
#define	PHANTOM			(SR._class [0] == 'P' && IS_ENTRY)
#define	MAX_SCHG		(4)
#define	SU(x)			(SR._schg_flag [0] == x)

#include <pslscr.h>
#include <getnum.h>
#include <twodec.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_ol_mess.h>
#include <ml_so_mess.h>
#include <Costing.h>

#ifdef GVISION
#include <StockWindow.h>
#endif	/* GVISION */

#define	IS_ENTRY		 (prog_status == ENTRY)
#define	NOT_ENTRY		 (prog_status != ENTRY)
#define	INVOICE			 (cohr_rec.type [0] == 'I')
#define	ONLINE_INVOICE	 (type_flag [0] == 'O')
#define	ONLINE_CREDIT	 (type_flag [0] == 'R')
#define	INV_STR			 (ONLINE_INVOICE) ? "Invoice" : "Credit Note"
#define CASH_INVOICE    (cumr_rec.cash_flag [0] == 'Y')
#define CHARGE_INVOICE  (cumr_rec.cash_flag [0] != 'Y')
#define AUTO_SK_UP		 (creat_flag [0] == envVar.AutoStockUpdate [0])

#define	OL_DISPLAY		 (prog_type == DISPLAY)
#define	OL_MAINT		 (prog_type == MAINT)
#define	OL_INPUT		 (prog_type == INPUT)

#define	INPUT	0
#define	MAINT	1

#define	PROMPT	prompts [prmpt_type]._prompt
#define	SRCH	prompts [prmpt_type]._srch

#define	FGN_CURR	 (envVar.MultiCurrency && strcmp (cumr_rec.curr_code, envVar.CurrencyCode))

#define		BY_BRANCH	1
#define		BY_DEPART	2

	struct	{
		char	*_prompt;
		char	*_srch;
	} prompts [] = {
	        {"Invoice",     "#Inv No."},
			{"Credit Note", "#Crd No."},
	};

	struct
	{
		char	dbt_no [7];
		char	tax_code [2];
		char	disc_code [2];
		char	class_type [4];
		char	price_type [2];
	} lastCust;

	int		lpno = 0;
	int		notax;				/* charge gst & tax		*/
	int		envDbCo = 0;		/* debtors company owned	*/
	int		new_invoice;		/* creating new invoice		*/
	int		manualInvoice = FALSE;
	int		credit_loaded = FALSE;	/* Loaded Invoice to credit.	*/
	int		cash_invoice;		/* Cash Invoice        		*/
	int		ins_flag;			/* inserting line item		*/
	int		win_ok;				/* stock check window		*/
	int		inp_disc;			/* input dis_pc if 0.00	*/
	int		so_perm_win;
	int		inp_sale;		
	int		inv_proof = 0;		/* proof total ok		*/
	int		by_batch;			/* require proof total		*/
	int		loaded = 0;			/* use lcount for calc'ns	*/
	int		prog_type;
	int		prmpt_type;
	int		np_fn;
	int		PV_inpstatus = ENTRY;
	int		PV_schg_line = 0;
	int		Recalc_kit	=	FALSE;
	int		Recalc_schg = FALSE;
	int		SO_DIS_INDENT = TRUE;
	int		MCURR;

	long	kit_hash	= 0L;
	double	c_left 		= 0.00,		/* credit remaining				*/
			t_total 	= 0.00,		/* line item gross for tax Amt  */
			l_total 	= 0.00,		/* line item gross				*/
			l_each 		= 0.00,		/* line item value each			*/
			l_disc 		= 0.00,		/* line item discount			*/
			l_tax 		= 0.00,		/* line item tax				*/
			l_gst 		= 0.00,		/* line item gst				*/
			inv_tot 	= 0.00,		/* invoiced total				*/
			dis_tot 	= 0.00,		/* discount total				*/
			tax_tot 	= 0.00,		/* tax total					*/
			gst_tot 	= 0.00,		/* gst total					*/
			proof_total = 0.00,		/* invoice proof total			*/
			amt_tendered = 0.00,	/* Cash given					*/
			amt_change 	= 0.00,		/* Change given					*/
			batch_tot 	= 0.00,		/* batch total					*/
			inv_total 	= 0.00,		/* 								*/
			previousPrice = 0.00;	/* 								*/

	float	previousDiscount = 0.00;

	char	branchNo [3],			/* branch number		*/
			creat_flag [2],		/* create status for invoice	*/
			type_flag [2],		/* I (nvoice or C (redit Note	*/
			read_flag [2],
			nd_flag [7],
			op_id [15],
			print_yn [2],
			ol_invoice [31],
			so_bonus [3],
			*curr_user,
			Loaded_no [9];		/* No. of loaded invoice.	*/

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct sobgRecord	sobg_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct cuccRecord	cucc_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuitRecord	cuit_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct exsiRecord	exsi_rec;
struct cudiRecord	cudi_rec;
struct essrRecord	essr_rec;
struct soktRecord	sokt_rec;
struct cudpRecord	cudp_rec;

	char	
			*cumr2 	= 	"cumr2",
			*inmr2 	= 	"inmr2",
			*cohr2 	= 	"cohr2",
			*data 	= 	"data",
			*ser_space     = "                         ",
			*sixteen_space = "                ",
			*twenty_spaces = "                    ";

	long	last_dockno;	/*	Last docket number   */

	struct	storeRec {
		long 	_hhbr_hash;			/* inmr_hhbr_hash					*/
		long 	_hhum_hash;			/* inmr_hhum_hash					*/
		long 	_hhsi_hash;			/* inmr_hhsi_hash					*/
		long 	_hhwh_hash;			/* incc_hhwh_hash					*/
		long	_origHhbr;			/* Original hhbr hash 				*/
		long	_origOrdQty;		/* Original order Qty 				*/
									/*									*/
		float 	_qty_avail;			/* incc_closing_stock				*/
		float 	_qty_sup;			/* local_rec.qty_sup				*/
		float	_gst_pc;			/* inmr_gst_pc or 0.00 if notax		*/
		float	_tax_pc;			/* inmr_gst_pc or 0.00 if notax		*/
		float	_dflt_disc;			/* inmr_dis_pc						*/
		float	_dis_pc;			/* coln_dis_pc						*/
		float	_calc_disc;			/*              					*/
		float	_outer;				/* inmr_outer_size					*/
		float	_min_marg;			/* Min margin for category.	     	*/
		float	_reg_pc;			/* Regulatory percent.      		*/
		float	_disc_a;			/* Discount percent A.      		*/
		float	_disc_b;			/* Discount percent A.      		*/
		float	_disc_c;			/* Discount percent A.      		*/
									/*									*/
		double	_tax_amt;			/* inmr_gst_amt " 0.00 if notax		*/
		double	_marg_cost;			/* Cost price for Margins.			*/
		double	_sale_price;		/* coln_sale_price					*/
		double	_calc_sprice;		/*                 					*/
		double	_act_sale;			/*                 					*/
		double	_gsale_price;		/*                 					*/
		double	_dflt_price;		/*                					*/
		double	_weight;			/* inmr_weight.						*/
		double	_ValueEach;			/* value each						*/
		double	_ExtendTotal;		/* Extended total.					*/
									/*									*/
		char	_serial [26];		/* serial number for line			*/
		char	_org_ser [26];		/* serial number for line			*/
		char	_category [12];		/* serial number for line			*/
		char	_sellgrp [7];		/* inmr_sellgrp.           			*/
		char	_bonus [2];			/* bonus item ?						*/
		char	_class [2];			/* item's class for line			*/
		char	_pack_size [6];		/* inmr_pack_size					*/
		char	_schg_flag [2];   	/* inmr surcharge flag				*/
		char	_pri_or [2];   		/* Price Overide.     				*/
		char	_dis_or [2];   		/* Discount Overide.  				*/
		char	_old_insf [2];		/* Y = insf record already allocated*/
									/*									*/
		int		_kit_flag;			/* Start End or None.				*/
		int		_pricing_chk;		/* Set if pricing has been  		*/
									/* called for line.         		*/
		int		_cumulative;		/* Cumulative 1 or 0				*/
		int		_con_price;			/* Contract price.					*/
		int		_indent;			/* Indent applies.					*/
		int		_cont_status;		/* 0 = not contract line			*/
									/* 1 = contract no disc				*/
	} store [ MAXLINES ];

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
        int             Pass_Disc = 0;
	char	*scn_desc [] =
		{
		  "HEADER SCREEN.",
		  "ITEM SCREEN.",
		  "AMOUNT TENDERED.",
		  "PROOF SCREEN."
		};

	FILE	*wout,
			*olPout;

	long	progPid;
	long	next_inv_no	=	0L;

char		*norm_invoice = "M0123456789";

#include	<p_terms.h>

/*==========================================================================
| The structure envVar groups the values of environment settings together. |
==========================================================================*/
struct tagEnvVar
{
	char	TaxCode [4];
	char	CurrencyCode [4];
	char	SoSpecial [5];
	char	AutoStockUpdate [2];
	char	SoMargin [3];
	char	Other [3] [31];
	int		GstApplies;
	int		SalesOrderNumber;
	int		MultiCurrency;
	int		ReverseDiscount;
	int		dbNettUsed;
	int		SoDisIndent;
	int		OlBelowSale;
	int		QcApply;
	int		SkQcAvl;
	int		DbStopcrd;
	int		DbCrdterm;
	int		DbCrdover;
	int		SoFwdAvl;
	int		WinOk;
	int		SoPermWin;
	int		DbCoOwned;
	int		KitDiscount;
	int		useSystemDate;
} envVar;

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	long	dflt_inv_no;
	double	dflt_freight;
	char	_date_raised [11];
	char	_date_required [11];
	char	dummy [11];
	char	dflt_ord [2];
	char	dflt_batch [6];
	char	cust_no [7];
	char	cust_name [41];
	char	invoice_no [9];
	char	inv_prmpt [30];
	char	pri_desc [16];
	char	ord_desc [10];
	char	dbt_date [11];
	char	systemDate [11];
	long	lsystemDate;
	char	item_no [17];
	char	sup_part [17];
	char	other [3] [31];
	char	spinst [3] [41];
	char	sell_desc [31];
	float	qty_ord;
	float	qty_sup;
	char	serial_no [26];
	char	prev_dbt_no [7];
	char	prev_inv_no [9];
	double	each;
	double	extend;
	char	dflt_sale_no [3];
	char	cont_desc [41];
	char	dl_name [41];
	char	creditOpId [15];
}
   local_rec, tmp_local;            

char	tmp_addr [3] [41];

static	struct	var	vars [] =
{
	{SCN_HEAD, LIN, "debtor",	4, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", esmr_rec.sales_acc, "Customer No.", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{SCN_HEAD, LIN, "name",	 4, 87, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.dbt_name, "Name", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.dl_name},
	{SCN_HEAD, LIN, "addr1",	 6, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.ch_adr1, "Postal : ", " ",
		YES, NO,  JUSTLEFT, "", "", tmp_addr [0]},
	{SCN_HEAD, LIN, "addr2",	 7, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.ch_adr2, "       : ", " ",
		YES, NO,  JUSTLEFT, "", "", tmp_addr [1]},
	{SCN_HEAD, LIN, "addr3",	 8, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.ch_adr3, "       : ", " ",
		YES, NO,  JUSTLEFT, "", "", tmp_addr [2]},
	{SCN_HEAD, LIN, "shipaddr1",	 6, 87, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.dl_adr1, "Delivery :", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add1},
	{SCN_HEAD, LIN, "shipaddr2",	 7, 87, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.dl_adr2, "         :", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add2},
	{SCN_HEAD, LIN, "shipaddr3",	 8, 87, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.dl_adr3, "         :", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.dl_add3},

	{SCN_HEAD, LIN, "ord_ref",	10, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Order Ref.", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.ord_ref},
	{SCN_HEAD, LIN, "op_id",	10, 87, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", op_id, "Operator ID.", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.op_id},
	{SCN_HEAD, LIN, "invoice_no",	11, 22, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "00000000", local_rec.inv_prmpt, " ",
		 NE, NO,  JUSTLEFT, "", "", cohr_rec.inv_no},
	{SCN_HEAD, LIN, "invoice_app",	11, 50, CHARTYPE,
		"UUNNNNNN", "          ",
		"0", " ", "Credit Invoice.", " ",
		 ND, NO,  JUSTLEFT, "", "", cohr_rec.app_inv_no},
	{SCN_HEAD, LIN, "cus_ord_ref",	11, 87, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Customer Order No.", " ",
		YES, NO,  JUSTLEFT, "", "", cohr_rec.cus_ord_ref},
	{SCN_HEAD, LIN, "dp_no",	13, 22, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Department No.", " ",
		YES, NO, JUSTRIGHT, "", "", cohr_rec.dp_no},
	{SCN_HEAD, LIN, "dp_name",	13, 26, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cudp_rec.dp_name},
	{SCN_HEAD, LIN, "sale_code",	14, 22, CHARTYPE,
		"UU", "          ",
		" ", essr_rec.short_id, "Salesman", " ",
		YES, NO, JUSTRIGHT, "", "", cohr_rec.sale_code},
	{SCN_HEAD, LIN, "sale_desc",	14, 26, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{SCN_HEAD, LIN, "pri_type",	14, 87, CHARTYPE,
		"N", "          ",
		" ", cumr_rec.price_type, "Price Type.", " ",
		YES, NO,  JUSTLEFT, "12345", "", local_rec.pri_desc},
	{SCN_HEAD, LIN, "freight",	16, 22, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Freight.", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *) &cohr_rec.freight},
	{SCN_HEAD, LIN, "cont_no",		17, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract :", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 ND, NO, JUSTLEFT, "", "", cohr_rec.cont_no},
	{SCN_HEAD, LIN, "cont_desc",	17, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTLEFT, "", "", local_rec.cont_desc},
	{SCN_LINES, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " Default Deletes Line ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{SCN_LINES, TAB, "hide",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "H", " ",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.hide_flag},
	{SCN_LINES, TAB, "descr",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "    I t e m   D e s c r i p t i o n .   ", " ",
		YES, NO,  JUSTLEFT, "", "", coln_rec.item_desc},
	{SCN_LINES, TAB, "sman_code",	 0, 1, CHARTYPE,
		"UU", "          ",
		" ", cohr_rec.sale_code, "Sale", " Salesman ",
		 ND, NO, JUSTRIGHT, "", "", coln_rec.sman_code},
	{SCN_LINES, TAB, "ln_ord_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cohr_rec.cus_ord_ref, " Cust Order Ref. ", "Customer Order Ref.",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.cus_ord_ref},
	{SCN_LINES, TAB, "pack_size",	 0, 0, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Pack ", " ",
		 ND, NO,  JUSTLEFT, "", "", coln_rec.pack_size},
	{SCN_LINES, TAB, "qty_ord",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "1.00", " Qty Ord ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_ord},
	{SCN_LINES, TAB, "qty_sup",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "", " Qty Sup ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_sup},
	{SCN_LINES, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Cst Price", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &coln_rec.cost_price},
	{SCN_LINES, TAB, "price_type",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "R", " ", " ",
		YES, NO, JUSTLEFT, "RSN", "", coln_rec.price_type},
	{SCN_LINES, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Sal Price", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&coln_rec.sale_price},
	{SCN_LINES, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "0.00", " Disc  ", " ",
		YES, NO, JUSTRIGHT, "-999.99", "100.00", (char *)&coln_rec.disc_pc},
	{SCN_LINES, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "      Serial Number      ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{SCN_LINES, TAB, "each",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0.00", " Value Each ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.each},
	{SCN_LINES, TAB, "extend",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0.00", "  Extended  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extend},
	{SCN_TENDER, LIN, "tendered",	18, 20, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Amount tendered", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&amt_tendered},
	{SCN_TENDER, LIN, "change",	18, 68, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "      Change given", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *)&amt_change},
	{SCN_PROOF, LIN, "proof",	18, 22, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Proof Total", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&proof_total},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<cus_price.h>
#include	<cus_disc.h>
#include	<neg_win.h>

#include	<FindCumr.h>

char 	*OlCheck 				(char *, char *, char *, char *);
const char	*GetPriceDesc 		(int);
float	ProcessPhantom 			(long);
float	ReCalcAvail 			(void);
float	ScreenDisc 				(float);
int		AddMissingWarehouse 	(struct inmrRecord inmr_rec);
int		AddSerialNo 			(long, long, char *, long);
int		CheckBonus 				(char *);
int		CheckDuplicateSearial 	(char *, long, int);
int		CheckEsmr 				(void);
int		CheckIncc 				(void);
int		CheckInvoiceDups 		(int);
int		CheckMarginOk 			(double, float, double, float);
int		CheckSerialQuantity 	(float	*);
int		DeleteInvoiceLine 		(void);
int		FindCucc 				(int, long);
int		FoundOpenKit 			(void);
int		InsertInvoiceLine 		(void);
int		KitHasNS 				(void);
int		LineInKit 				(int);
int		LoadInvoiceLines 		(long);
#ifndef GVISION
int		OpenSkWin 				(void);
#endif
int		SpecialItem 			(int);
int		SrchCudi 				(int);
int		UpdateMenu 				(int);
int		ValidateItemNumber 		(int);
int		WarnUser 				(char *, int, int);
int		heading 				(int);
int		spec_valid 				(int);
int		use_window 				(int);
int		win_function2 			(int, int, int, int);
static	char *GenInvoiceNo 		(int, long);
void	AddSobg 				(int, char	*, long);
void	AltClear 				(int);
void	Busy 					(int);
void	CalLineExtended 		(int);
void	CalTotalExtended 		(int);
void	CalcInvoiceTotal 		(void);
void	CalcKitLine 			(void);
void	CalcSurcharge 			(void);
void	CheckEnvironment 		(void);
void	CheckKit	 			(int);
void	ClearWindow 			(void);
void	CloseDB 				(void);
void	CommitSerialNo 			(int, char *);
void	DelSline 				(int);
void	DeleteInvoice 			(int);
void	DeleteSurcharge 		(void);
void	DiscProcess 			(int);
void	DrawCustomerInfo 		(void);
void	DrawTotal 				(void);
void	ForceKitCompletion 		(void);
void	FreeSearialNo 			(int, char *);
void	InitML 					(void);
void	InitStore 				(int);
void	InptRes 				(float	*);
void	LogLostSales 			(float);
void	NextInvoiceNo 			(int);
void	OlClose 				(void);
void	OlOpen 					(char *, char *, int);
void	OlPrint 				(long);
void	OpenDB 					(void);
void	PriceProcess 			(int);
void	PrintCoStuff 			(void);
void	PrintCustomer 			(void);
void	PrintExtend 			(int);
void	PrintTotal 				(void);
void	ProcessKitItem 			(long, float);
void	ProcessScreen 			(int, int);
void	ReadMisc 				(void);
void	RecalcSchg 				(void);
void	RemoveSerialNo 			(long, long, char	*);
void	ResetKit 				(int, int);
void	RunningKit 				(int);
void	SearchInvoiceNo 		(char *);
void	SetInvoiceDefaults 		(int);
void	SetSchgCalc 			(void);
void	ShowAppliedInvoiceNo 	(char *);
void	ShowExsf 				(char *);
void	ShowPaymentTerms 		(void);
void	SrchCnch 				(char *);
void	SrchCudp 				(char *);
void	SrchCumr 				(char *);
void	SrchExaf 				(char *);
void	SrchExsi 				(char *);
void	SrchInsf 				(char *, int);
void	SrchPrice				(void);
void	SrchSell				(void);
void	UpdateInvoice 			(int, int);
void	UpdateSerialNo 			(int, char *);
void	tab_other 				(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	int		first_time = 1;
	int		field;
	char	*sptr;


	curr_user = getenv ("LOGNAME");

	progPid = (long) getpid ();

	_win_func = TRUE;

	SETUP_SCR (vars);


	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	strcpy (print_yn, "N");


	if (argc != 7 && argc != 6)
	{
		/*-----------------------------------------------------------
		| Usage : %s <OP_ID> <batch_flag> <creat_flag> <type_flag>  |
		| 		<text_file> - optional <LPNO>						|
		|<batch_flag> - Y (es By Batch 								|
		|             - N (o 										|
		|<type_flag>  - O (n-line Invoice 							|
		|             - I - Manual Online Invoice     				|
		|             - R (efund (On-line Credit Note) 				|
		|<text_file_name> 											|
		-----------------------------------------------------------*/

		print_at (0,0,ML (mlOlMess048), argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	if (!strcmp (argv [0],"ol_invoice"))
	{
		prog_type = INPUT;
		strcpy (read_flag,"u");
	}
	else
	{
		if (!strcmp (argv [0],"ol_invdisp"))
		{
			prog_type = DISPLAY;
			strcpy (read_flag,"r");
		}
		else
		{
			prog_type = MAINT;
			strcpy (read_flag,"u");
		}
	}

	sprintf (op_id, "%-14.14s", argv [1]);

	switch (argv [2] [0])
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
		/*-------------------------------
		| <batch_flag> - Y (es By Batch 	|
		|              - N (o			|
		-------------------------------*/
		print_at (0,0,ML (mlOlMess048), argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (creat_flag,"%-1.1s",argv [3]);

	switch (argv [4] [0])
	{
	case	'O':
	case	'o':
		strcpy (type_flag,"O");
		prmpt_type = 0;
		break;

	case	'I':
	case	'i':
		strcpy (type_flag,"I");
		prmpt_type = 0;
		manualInvoice = TRUE;
		lpno = 0;
		break;

	case	'R':
	case	'r':
		strcpy (type_flag,"R");
		prmpt_type = 1;
		break;

	default:
		/*-----------------------------------------------
		| <type_flag>  - O (n-line Invoice 				|
		|             - R (efund (On-line Credit Note) 	|
		-----------------------------------------------*/
		print_at (0,0,ML (mlOlMess048), argv [0]);
		return (EXIT_FAILURE);
	}

	tab_row = 8;

	sprintf (local_rec.inv_prmpt,"%s No.",PROMPT);

	if (argc == 7)
		lpno = atoi (argv [6]);

#ifndef GVISION
	if (envVar.SoPermWin)
	{
		if (OpenSkWin ())
			win_ok = FALSE;
	}
#endif

	strcpy (local_rec.dflt_ord,"D");
	strcpy (local_rec.dflt_batch,"00000");

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	ReadMisc ();

	strcpy (local_rec.other [0], envVar.Other [0]);
	strcpy (local_rec.other [1], envVar.Other [1]);
	strcpy (local_rec.other [2], envVar.Other [2]);

	init_scr ();

	_set_masks (argv [5]);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_LINES, store, sizeof (struct storeRec));
#endif

	if (OL_DISPLAY)
	{
		for (field = label ("cus_ord_ref");FIELD.scn != 0;field++)
			if (FIELD.required != ND)
				FIELD.required = NA;
	}
	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];

	inp_disc = 0;
	inp_sale = 0;

	inp_sale = FLD ("sale_price");
	inp_disc = FLD ("disc");

	if (!strcmp ("ol_docket", argv [0]))
		strcpy (print_yn, "Y");

	if (OL_DISPLAY)
	{
		for (field = label ("cus_ord_ref");FIELD.scn != 0;field++)
			if (FIELD.required != ND)
				FIELD.required = NA;
	}

	if (ONLINE_INVOICE)
	{
		vars [label ("invoice_no")].fill 	 = " ";
		vars [label ("invoice_no")].lowval = alpha;
	}
	else
		vars [label ("invoice_no")].lowval = norm_invoice;

	init_scr ();
	set_tty (); 

	no_edit (SCN_PROOF);

	strcpy (local_rec.dbt_date,DateToString (comm_rec.dbt_date));
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	strcpy (local_rec._date_raised,   local_rec.systemDate);
	strcpy (local_rec._date_required, local_rec.systemDate);
	local_rec.dflt_inv_no 	=  0L;

	strcpy (branchNo, (envVar.DbCoOwned) ? comm_rec.est_no : " 0");

	if (ONLINE_CREDIT)
		FLD ("invoice_app") = YES;

	/*---------------------------
	| open main database files. |
	---------------------------*/
	InitML ();

	/*--------------------------------
	| Open price and Discount files. |
	--------------------------------*/
	OpenPrice ();
	OpenDisc ();

	strcpy (local_rec.prev_inv_no,"00000000");
	strcpy (local_rec.prev_dbt_no,"000000");

	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,comm_rec.est_no);
	if ((cc = find_rec (esmr,&esmr_rec,COMPARISON,"r")))
		file_err (cc, "esmr", "DBFIND");

	prog_exit = 0;
	while (!prog_exit)
	{
		PV_inpstatus = ENTRY;
		Recalc_schg = FALSE;

		PV_schg_line = lcount [SCN_LINES] = 0;
		ins_flag = 0;

		if (ONLINE_CREDIT)
			no_edit (SCN_TENDER);

		inv_tot = dis_tot = tax_tot = gst_tot = 0.00;
		l_total = l_each = l_disc = l_tax = l_gst = 0.00;

		if (restart) 
		{
			if (first_time && OL_INPUT)
			{
				strcpy (local_rec.dflt_batch,"00000");
				strcpy (local_rec._date_raised,
							local_rec.systemDate);
				strcpy (local_rec._date_required,
							local_rec.systemDate);
				strcpy (local_rec.dflt_ord,"D");
			}
		}
		if (restart)
		{
			/*------------------------------------
			| Reset any outstanding insfRecords |
			------------------------------------*/
			for (i = 0; i < MAXLINES; i++)
			{
				if (strcmp (store [i]._serial, ser_space))
				{
					/*---------------------------------------------------
					| If the coln previously existed the reset the insf |
					| status to sold else remove the insf that has been |
					| created.                                          |
					---------------------------------------------------*/
 					if (store [i]._old_insf [0] == 'Y')
					{
						if (ONLINE_INVOICE)
							cc = UpdateInsf (store [i]._hhwh_hash,0L,
											store [i]._serial,
											"C","F");
						else
							cc = UpdateInsf (store [i]._hhwh_hash,0L,
											store [i]._serial,
											"C","S");

						if (cc && cc < 1000)
							file_err (cc, insf, "DBUPDATE");
					}
					else
					{
						RemoveSerialNo (store [i]._hhwh_hash,
									   store [i]._hhsi_hash,
						   			   store [i]._serial);
					}
				}

				if (strcmp (store [i]._org_ser, ser_space))
				{
					/*--------------------------------------
					| Restore original serial allocations. |
					--------------------------------------*/
					if (ONLINE_INVOICE)
						cc = UpdateInsf (store [i]._hhwh_hash,0L,
										store [i]._org_ser,
										"F","C");
					else
						cc = UpdateInsf (store [i]._hhwh_hash,0L,
										store [i]._org_ser,
										"F","C");
				}
			}
		}

		for (i = 0; i < MAXLINES; i++)
			InitStore (i);

		search_ok = 1;
		entry_exit = edit_exit = prog_exit = restart = 0;
		loaded = 0;
		init_vars (SCN_HEAD);	

		ProcessScreen (SCN_HEAD, FALSE);
		if (prog_exit || restart)
			continue;

		if (new_invoice && !credit_loaded)
		{
			no_edit (SCN_TENDER);
			line_cnt = -1;
			ProcessScreen (2, FALSE);
			if (restart)
				continue;

			CalcSurcharge ();
		}
		else
			scn_display (SCN_HEAD);

		PV_inpstatus = EDIT;
		PV_schg_line = lcount [SCN_LINES];
		loaded = 1;
		while (!edit_exit && !restart)
		{
			edit_all ();
			ForceKitCompletion ();

			if (OL_DISPLAY)
				continue;

			if (by_batch)
			{
				no_edit (SCN_TENDER);
				CalTotalExtended (TRUE);
				ProcessScreen (4, FALSE);

				while (inv_proof)
				{
					edit_all ();
					ForceKitCompletion ();

					if (restart)
						break;

					ProcessScreen (4, TRUE);
					
					if (restart)
						break;
				}
			}
			else
			{
				if (!strcmp (local_rec.cust_no,
						esmr_rec.sales_acc) &&
							!ONLINE_CREDIT && lpno)
				{
					ProcessScreen (3, TRUE);

					while (inv_proof)
					{
						edit_all ();
						ForceKitCompletion ();

						if (restart)
							break;

						ProcessScreen (3, TRUE);
						
						if (restart)
							break;
					}
				}
				inv_proof = 0;
			}

			if (!inv_proof && !restart) 
			{
				if (manualInvoice)
				{
					UpdateInvoice (TRUE, 0);
					edit_exit = TRUE;
				}
				else
				{
					if (! (edit_exit = UpdateMenu (lpno)))
						inv_proof = 1;
				}

				strcpy (local_rec.dflt_ord,cohr_rec.ord_type);
				strcpy (local_rec._date_raised,
							local_rec.systemDate);
				first_time = 0;
			}
		}
		if (restart || OL_DISPLAY)
			continue;
	}
	/*---------------------------------
	| Open Output to Counter Printer. |
	---------------------------------*/
	clear ();
	if (by_batch)	
	{
		/*----------------------
		| Batch Total = %-8.2f |
		| Any key To Continue  |
		----------------------*/
		print_at (0,0,ML (mlOlMess040),DOLLARS (batch_tot));
		PauseForKey (0,0,ML (mlStdMess042),0);
	}
	clear ();

#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (wout);
		IP_CLOSE (np_fn);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
InitML 
 (
	void
)
{
	int		i;

	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		strcpy (err_str, STerms [i]._sdesc);
		strcpy (STerms [i]._sdesc, strdup (err_str));
	}
	prompts [0]._prompt = strdup (ML (mlOlMess069));	
	prompts [1]._prompt = strdup (ML (mlOlMess070));	
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB
 (
	void
)
{
	abc_dbopen (data);

	/*--------------------------------------------------
	|	Set up alias and view values for show_inv. |
	--------------------------------------------------*/
	abc_alias (cohr2,cohr);
	abc_alias (cumr2,cumr);
	abc_alias (inmr2,inmr);

	open_rec (cumr,		cumr_list,	CUMR_NO_FIELDS, "cumr_id_no3");
	open_rec (cumr2,	cumr_list,	CUMR_NO_FIELDS,	"cumr_hhcu_hash");
	open_rec (incc,		incc_list,	INCC_NO_FIELDS,	"incc_id_no");
	open_rec (inmr,		inmr_list,	INMR_NO_FIELDS,	"inmr_id_no");
	open_rec (inmr2,	inmr_list,	INMR_NO_FIELDS,	"inmr_hhbr_hash");
	open_rec (coln,		coln_list,	COLN_NO_FIELDS,	"coln_id_no");
	open_rec (cohr,		cohr_list,	COHR_NO_FIELDS,	"cohr_id_no2");
	open_rec (cohr2,	cohr_list,	COHR_NO_FIELDS,	"cohr_id_no2");
	open_rec (esmr,		esmr_list,	ESMR_NO_FIELDS,	"esmr_id_no");
	open_rec (cudp,		cudp_list,	CUDP_NO_FIELDS,	"cudp_id_no");
	open_rec (exaf,		exaf_list,	EXAF_NO_FIELDS,	"exaf_id_no");
	open_rec (exsf,		exsf_list,	EXSF_NO_FIELDS,	"exsf_id_no");
	open_rec (essr,		essr_list,	ESSR_NO_FIELDS,	"essr_id_no");
	open_rec (cucc,		cucc_list,	CUCC_NO_FIELDS,	"cucc_id_no");
	open_rec (cuit,		cuit_list,	CUIT_NO_FIELDS,	"cuit_id_no");
	open_rec (cncl,		cncl_list,	CNCL_NO_FIELDS,	"cncl_id_no");
	open_rec (cnch,		cnch_list,	CNCH_NO_FIELDS,	"cnch_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB
 (
	void
)
{
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (cohr);
	abc_fclose (cohr2);
	abc_fclose (coln);
	abc_fclose (esmr);
	abc_fclose (cudp);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_fclose (essr);
	abc_fclose (cucc);
	abc_fclose (cuit);
	abc_fclose (cncl);
	abc_fclose (cnch);

	/*--------------------------------------
	| Close pricing and discounting files. |
	--------------------------------------*/
	ClosePrice ();
	CloseDisc ();
	CloseCosting ();

	SearchFindClose ();
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc
 (
	void
)
{

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	if ((cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r")))
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
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
			heading (SCN_LINES);
			firstTime = FALSE;
		}
		tmpLineCnt = line_cnt;
		line_cnt = lcount [SCN_LINES] - 1;
		scn_display (SCN_LINES);
		line_cnt = tmpLineCnt;

		_edit (SCN_LINES, lcount [SCN_LINES] - 1, 0);

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
	for (i = 0; i < lcount [SCN_LINES]; i++)
	{
		if (store [i]._kit_flag == K_END &&
			foundOpenKit)
		{
			foundOpenKit = FALSE;
			continue;
		}
		
		if (store [i]._kit_flag == K_START)
			foundOpenKit = TRUE;
	}

	return (foundOpenKit);
}

int
spec_valid
 (
	int field
)
{
	int		i = 0;
	int		this_page;
	double	total_owing = 0.00;
	char	tmp_inv [9];
	int		cudi_index;
	long	CheckDate = 0L;

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ( "debtor") )
	{
		if (dflt_used)
			sprintf (local_rec.cust_no, esmr_rec.sales_acc);

		if (SRCH_KEY)
		{
			if (IS_ENTRY)
				CumrSearch (comm_rec.co_no, branchNo, temp_str);
			else
				SrchCumr (temp_str);

			cumr_rec.hhcu_hash = 0L;
			PrintCustomer ();
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
		if ((cc = find_rec (cumr,&cumr_rec,COMPARISON,"r")))
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (ErrorReturn);
		}
	
		/*----------------------
		| Check currency code. |
		----------------------*/
		if (FGN_CURR)
		{
			/*---------------------------------------------------
			| You may not sell to a debtor whose currency code 	|
			| is different to native currency.					| 
			---------------------------------------------------*/
			print_mess (ML (mlOlMess005));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}

		if (prog_status != ENTRY && (cumr_rec.cash_credit [0] == ' ' &&
			strcmp (pad_num (cumr_rec.dbt_no), lastCust.dbt_no)))
		{
			print_mess ("You can only change to a credit card or cash customer");
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}

		/*---------------------------
		| User is changing customer |
		---------------------------*/
		if (prog_status != ENTRY && 
			strcmp (pad_num (cumr_rec.dbt_no), lastCust.dbt_no))
		{
			/*-------------------------------------------------
			| The old customer and the new customer must have |
			| certain criteria in common.  Othewise the whole |
			| invoice needs to recalculated (prices, tax etc) |
			| As the functionality is not in here to do the   |
			| recalculation, we disallow changing to a        |
			| customer who does not meet the criteria :       |
			|   Tax Code                                      |
			|   Discount Code                                 |
			|   Customer Type                                 |
			|   Price Type                                    |
			-------------------------------------------------*/
			if (strcmp (cumr_rec.tax_code,   lastCust.tax_code) ||
				strcmp (cumr_rec.disc_code,  lastCust.disc_code) ||
				strcmp (cumr_rec.class_type, lastCust.class_type) ||
				strcmp (cumr_rec.price_type, lastCust.price_type))
			{
				print_mess (ML ("Unable to change customers due to different tax code, discount code, customer type or price type."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status == ENTRY || cumr_rec.cash_credit [0] != 'C')
		{
			/*--------------------------------
			| Don't change details if we are |
			| swapping to a cash or credit   |
			| card customer.                 |
			--------------------------------*/

			sprintf (local_rec.dl_name, "%-40.40s", cumr_rec.dbt_name);
			strcpy (tmp_addr [0], cumr_rec.ch_adr1);
			strcpy (tmp_addr [1], cumr_rec.ch_adr2);
			strcpy (tmp_addr [2], cumr_rec.ch_adr3);
			sprintf (cohr_rec.din_1, "%-60.60s", tmp_addr [0]);
			sprintf (cohr_rec.din_2, "%-60.60s", tmp_addr [1]);
			sprintf (cohr_rec.din_3, "%-60.60s", tmp_addr [2]);
			strcpy (cohr_rec.dl_add1, cumr_rec.dl_adr1);
			strcpy (cohr_rec.dl_add2, cumr_rec.dl_adr2);
			strcpy (cohr_rec.dl_add3, cumr_rec.dl_adr3);
		}

		if (CASH_INVOICE)
			cash_invoice = 1;
		else
		{
			no_edit (SCN_TENDER);
			cash_invoice = 0;
		}

		/*--------------------------------------
		| Check if customer is on stop credit. |
		--------------------------------------*/
		if (ONLINE_INVOICE && cumr_rec.stop_credit [0] == 'Y' && 
 		     !CASH_INVOICE)
		{
			if (envVar.DbStopcrd)
			{
				/*-------------------------------------------------------
				| Customer is on Stop Credit,Cannot Process Any Orders. |
				--------------------------------------------------------*/
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				return (ErrorReturn);
			}
			else
			{
				strcpy (err_str, ML (mlStdMess060));
				if ((cc = WarnUser (err_str,0,2)))
					return (cc);
			}
		}

		total_owing	=	cumr_rec.bo_current + 	
						cumr_rec.bo_per1 +
		              	cumr_rec.bo_per2 + 
						cumr_rec.bo_per3 +
		              	cumr_rec.bo_per4 + 
						cumr_rec.bo_fwd;

		c_left = total_owing - cumr_rec.credit_limit;

		/*---------------------------------------------
		| Check if customer is over his credit limit. |
		---------------------------------------------*/
		if (ONLINE_INVOICE && cumr_rec.credit_limit <= total_owing &&
				cumr_rec.credit_limit != 0.00 && !CASH_INVOICE)
		{
			if (envVar.DbCrdover)
			{
			    /*------------------------------------------------------
				| Customer Over Credit Limit, Can't Process Any Orders |
			    ------------------------------------------------------*/
			    print_mess (ML (mlStdMess061));
				sleep (sleepTime);
			    return (ErrorReturn);
			}
			else
			{
				/*--------------------------------------------
			    | WARNING Customer over Credit Limit by %.2f |
				--------------------------------------------*/
			    if ((cc = WarnUser (ML (mlStdMess061),0,2)))
			    	return (ErrorReturn);
			}
		}
		/*-----------------------
		| Check Credit Terms	|
		-----------------------*/
		if (cumr_rec.od_flag && !CASH_INVOICE)
		{
			/*----------------------------------------------
			| Customer credit terms exceeded by %d periods |
			----------------------------------------------*/
			if (envVar.DbCrdterm)
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				return (ErrorReturn);
			}
			else
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				cc = WarnUser (err_str,0,2);
				if (cc)
					return (ErrorReturn);
			}
			
		}
		PrintCustomer ();

		strcpy (lastCust.dbt_no,     cumr_rec.dbt_no);
		strcpy (lastCust.tax_code,   cumr_rec.tax_code);
		strcpy (lastCust.disc_code,  cumr_rec.disc_code);
		strcpy (lastCust.class_type, cumr_rec.class_type);
		strcpy (lastCust.price_type, cumr_rec.price_type);

/*
		FLD ("pri_type") = (CASH_INVOICE) ? YES : NA;
*/

		if (cumr_rec.tax_code [0] == 'A' || cumr_rec.tax_code [0] == 'B')
			notax = 1;
		else
			notax = 0;

		use_window (FN14);

		strcpy (local_rec.dflt_sale_no, cumr_rec.sman_code);

		/*-------------------------------
		| if cus_ord_ref must be input	|
		-------------------------------*/
		if (!OL_DISPLAY)
		{
			if (cumr_rec.po_flag [0] == 'Y')
				FLD ("cus_ord_ref") = YES;
			else
				FLD ("cus_ord_ref") = ( F_NOKEY (label ("cus_ord_ref") )) ? NA : YES;
		}
		cohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		return (EXIT_SUCCESS);
	}
		
	if (CASH_INVOICE)
	{
		if (LCHECK ( "addr1")) 
			sprintf (cohr_rec.din_1, "%-60.60s", tmp_addr [0]);

		if (LCHECK ( "addr2")) 
			sprintf (cohr_rec.din_2, "%-60.60s", tmp_addr [1]);

		if (LCHECK ( "addr3")) 
			sprintf (cohr_rec.din_3, "%-60.60s", tmp_addr [2]);
	}

	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ( "invoice_no")) 
	{

/*
		manualInvoice = FALSE;
*/

		if (!lpno && !strcmp (cohr_rec.inv_no, "00000000"))
		{
			sprintf (cohr_rec.inv_no, "%08ld", local_rec.dflt_inv_no);
			DSP_FLD ("invoice_no");
			if (!strcmp (cohr_rec.inv_no, "00000000"))
			{
				/*-------------------------------
				| You must enter an invoice no. |
				--------------------------------*/
				strcpy (cohr_rec.inv_no, "");
				print_mess (ML (mlStdMess115));
				sleep (sleepTime);
				return (ErrorReturn);
			}
		}
		if (IS_ENTRY && lpno && F_NOKEY ( label ("invoice_no")))
		{
			strcpy (cohr_rec.inv_no,"00000000");
			DSP_FLD ("invoice_no");
		}

		/*---------------------------
		| Maintaining Sales Orders	|
		---------------------------*/
		if (OL_DISPLAY || SRCH_KEY || strcmp (cohr_rec.inv_no,"00000000"))
		{
			if (SRCH_KEY)
			{
				SearchInvoiceNo (temp_str);
				return (EXIT_SUCCESS);
			}

			/*------------------------------
			| Check if invoice is on file. |
			------------------------------*/
			strcpy (cohr_rec.co_no,  comm_rec.co_no);
			strcpy (cohr_rec.br_no,  comm_rec.est_no);
			strcpy (cohr_rec.type,   type_flag);
			strcpy (cohr_rec.inv_no, zero_pad (cohr_rec.inv_no, 8));

			cc = find_rec (cohr,&cohr_rec,COMPARISON,read_flag);
			if (cc)
			{
				if (OL_DISPLAY || lpno || cohr_rec.inv_no [0] == 'M')
				{
					/*--------------------
					| Invoice not found. |
					--------------------*/
					sprintf (err_str,ML (mlStdMess237),PROMPT,cohr_rec.inv_no, 8);
					print_mess (err_str);
					sleep (sleepTime);
					return (ErrorReturn); 
				}
				new_invoice = 1;
/*
				manualInvoice = TRUE;
*/
				SetInvoiceDefaults (new_invoice);
				if (ONLINE_INVOICE)
				    goto_field (field, label ("cus_ord_ref"));
				return (EXIT_SUCCESS);
			}
			else
			{
				if (manualInvoice)
				{
					print_mess ("\007Invoice already exists");
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			strcpy (Loaded_no, cohr_rec.inv_no);
			if (lpno)
				strcpy (cohr_rec.inv_no, "00000000");

			if (cumr_rec.hhcu_hash != cohr_rec.hhcu_hash)
			{
				/*-----------------------------------------------
				| Invoice No.is on file for a different Customer. |
				-----------------------------------------------*/
				sprintf (err_str,ML (mlOlMess025),PROMPT,cohr_rec.inv_no);
				print_mess (err_str);
				sleep (sleepTime);
				return (ErrorReturn); 
			}

			if (OL_INPUT && cohr_rec.stat_flag [0] != creat_flag [0])
			{
				/*----------------------------
				| Invoice has been processed.|
				----------------------------*/
			   	sprintf (err_str,ML (mlOlMess026), PROMPT,cohr_rec.inv_no);
			   	abc_unlock (cohr);
			   	print_mess (err_str);
				sleep (sleepTime);
			   	return (ErrorReturn); 
			}

			sprintf (local_rec.dl_name, "%40.40s", cohr_rec.dl_name);
			if (LoadInvoiceLines (cohr_rec.hhco_hash))
				return (ErrorReturn);

			new_invoice = 0;
			entry_exit = 1;
		}
		else
			new_invoice = 1;

		SetInvoiceDefaults (new_invoice);

		if (ONLINE_INVOICE)
			goto_field (field, label ("cus_ord_ref"));

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Invoice Number to be Credited. |
	-----------------------------------------*/
	if (LCHECK ( "invoice_app")) 
	{
		if (F_HIDE ( label ( "invoice_app")) ) 
			return (EXIT_SUCCESS);

		strcpy (tmp_inv, cohr_rec.inv_no);

		/*---------------------------
		| Maintaining Sales Orders	|
		---------------------------*/
		if (SRCH_KEY)
		{
			ShowAppliedInvoiceNo (temp_str);
			strcpy (cohr_rec.inv_no, tmp_inv);
			return (EXIT_SUCCESS);
		}

		if (!strcmp ("        ", cohr_rec.app_inv_no) ||
			!strcmp ("00000000", cohr_rec.app_inv_no))
		{
			/*-------------------------------
			| You must enter an Invoice no. |
			-------------------------------*/
			print_mess (ML (mlStdMess115));
			sleep (sleepTime);
			return (ErrorReturn);
		}

		/*------------------------------
		| Check if invoice is on file. |
		------------------------------*/
		strcpy (cohr2_rec.co_no,  comm_rec.co_no);
		strcpy (cohr2_rec.br_no,  comm_rec.est_no);
		strcpy (cohr2_rec.type,   "I");
		strcpy (cohr2_rec.inv_no, zero_pad (cohr_rec.app_inv_no, 8));

		cc = find_rec (cohr,&cohr2_rec,COMPARISON,read_flag);
		if (cc)
		{
			/*--------------------
			| Invoice not found. |
			--------------------*/
			print_mess (ML (mlStdMess115));
			sleep (sleepTime);
		}
		else
		{
			if (cohr_rec.hhcu_hash != cohr2_rec.hhcu_hash)
			{
				/*---------------------------------------------
				| Invoice No. %s belongs to another Customer. |
				---------------------------------------------*/
			    print_mess (ML (mlOlMess058));
			    sleep (sleepTime);
			    return (ErrorReturn);
			}
			memcpy ((char *)&cohr_rec, (char *)&cohr2_rec, sizeof (cohr_rec));

			cohr_rec.date_required = StringToDate (local_rec.systemDate);
			cohr_rec.date_raised 	= cohr_rec.date_required;
			sprintf (cohr_rec.cus_ord_ref, "%-20.20s", " ");
/*
			sprintf (cohr_rec.ord_ref, 	"%-16.16s", " ");
			sprintf (cohr_rec.op_id, 	"%-14.14s",	" ");
*/
			sprintf (cohr_rec.op_id, 	"%-14.14s",	local_rec.creditOpId);
			strcpy  (cohr_rec.inv_no, 	tmp_inv);
			strcpy  (cohr_rec.app_inv_no, temp_str);
			if (LoadInvoiceLines (cohr2_rec.hhco_hash))
				return (ErrorReturn);
			scn_display (cur_screen);

			entry_exit = 1;
			if (CASH_INVOICE)
			{
				strncpy (tmp_addr [0], cohr_rec.din_1, 40);
				tmp_addr [0] [40] = '\0';
				strncpy (tmp_addr [1], cohr_rec.din_2, 40);
				tmp_addr [1] [40] = '\0';
				strncpy (tmp_addr [2], cohr_rec.din_3, 40);
				tmp_addr [2] [40] = '\0';

				DSP_FLD ("addr1");
				DSP_FLD ("addr2");
				DSP_FLD ("addr3");
			}
			SetInvoiceDefaults (0);
			credit_loaded = TRUE;
			return (EXIT_SUCCESS);
		}

		credit_loaded = FALSE;
		goto_field (field, label ("cus_ord_ref"));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ( "cus_ord_ref")) 
	{
		if (!strcmp (cohr_rec.cus_ord_ref,"                    "))
		{
			if (cumr_rec.po_flag [0] == 'Y')
			{
			    /*------------------------------------ 
				| Purchase Order Number Must be Input |
				-------------------------------------*/
			    print_mess (ML (mlSoMess078));
				sleep (sleepTime);
			    return (ErrorReturn);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("name") || !strncmp (FIELD.label,"shipaddr",8)) 
	{
		if (!strncmp ( FIELD.label, "name", 4) )
 			cudi_index = -1;
		else
 			cudi_index = field - label ("shipaddr1");

		if (SRCH_KEY)
		{
			open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");

			i = SrchCudi (cudi_index + 1);

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			sprintf (local_rec.dl_name, "%-40.40s", cudi_rec.name);

			sprintf (cohr_rec.dl_add1, "%-40.40s", cudi_rec.adr1);
			sprintf (cohr_rec.dl_add2, "%-40.40s", cudi_rec.adr2);
			sprintf (cohr_rec.dl_add3, "%-40.40s", cudi_rec.adr3);

			DSP_FLD ("name");
			DSP_FLD ("shipaddr1");
			DSP_FLD ("shipaddr2");
			DSP_FLD ("shipaddr3");
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dp_no")) 
	{
		if (dflt_used || F_NOKEY (field))
			strcpy (cohr_rec.dp_no,cumr_rec.department);


		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
		{
			/*------------------------------
			| Department Number not found. |
			------------------------------*/
			print_mess (ML (mlStdMess084));
			sleep (sleepTime);
			return (ErrorReturn); 
		}
			
		PrintCoStuff ();
		DSP_FLD ("dp_no");
		DSP_FLD ("dp_name");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ( "pri_type")) 
	{
		if (IS_ENTRY &&
			 (FIELD.required == NA || FIELD.required == NI))
		{
			strcpy (local_rec.pri_desc, cumr_rec.price_type);
		}

		if (CASH_INVOICE)
		{
			sprintf (cohr_rec.pri_type,"%-1.1s", local_rec.pri_desc);
			print_at (vars [field].row, vars [field].col+3, "%s          ", 
										cohr_rec.pri_type);
			return (EXIT_SUCCESS);
		}


		if (SRCH_KEY)
		{
			SrchPrice ();
			strcpy (local_rec.pri_desc,"               ");
			return (EXIT_SUCCESS);
		}
		sprintf (cohr_rec.pri_type,"%-1.1s",local_rec.pri_desc);
		strcpy (local_rec.pri_desc, GetPriceDesc (atoi(cohr_rec.pri_type)));

		DSP_FLD ("pri_type");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ( "sale_code")) 
	{
		if (IS_ENTRY &&
				 (FIELD.required == NA || FIELD.required == NI))
		{
			strcpy (cohr_rec.sale_code,local_rec.dflt_sale_no);
			DSP_FLD ("sale_code");
		}

		if (SRCH_KEY)
		{
			ShowExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
		if ((cc = find_rec (exsf,&exsf_rec,COMPARISON,"r")))
		{
			/*--------------------- 
			| Salesman not found. |
			---------------------*/
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (ErrorReturn);
		}

		DSP_FLD ("sale_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("op_id") )
	{
		strcpy (local_rec.creditOpId, "");

		strcpy (essr_rec.co_no,comm_rec.co_no);
		strcpy (essr_rec.est_no,comm_rec.est_no);
		strcpy (essr_rec.op_id,cohr_rec.op_id);
		if ((cc = find_rec (essr,&essr_rec,COMPARISON,"r")))
		{
			/*--------------------- 
			| Operator not found. |
			---------------------*/
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			return (ErrorReturn);
		}

		/*----------------------------------------- 
		| Save the operator ID, in case this is a |
		| credit and we need to use it later.     |
		-----------------------------------------*/
		strcpy (local_rec.creditOpId, cohr_rec.op_id);

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| validate debtors contract |
	---------------------------*/
	if (LCHECK ("cont_no"))
	{
		if (IS_ENTRY && F_NOKEY (field))
		{
			strcpy (local_rec.cont_desc, " ");
			strcpy (cnch_rec.exch_type, " ");
			cnch_rec.hhch_hash	=	0L;
			return (EXIT_SUCCESS);
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
			/*---------------------
			| Contract Not found. |
			---------------------*/
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}
		CheckDate = StringToDate (local_rec.systemDate);

		/*------------------------------------------
		| now see if contract is still current.    |
		------------------------------------------*/
		if (cnch_rec.date_exp < CheckDate)
		{
			sprintf (err_str, 
					/*"Contract No Longer Current - Expired %s",*/ 
					ML (mlOlMess059), 
					DateToString (cnch_rec.date_exp));

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}

		if (cnch_rec.date_wef > CheckDate)
		{
			/*-----------------------------------------
			| Contract Not Yet Current - Effective %s |
			-----------------------------------------*/
			sprintf (err_str, ML (mlOlMess060), DateToString (cnch_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}
		/*-------------------------------------------
		| now see if contract is assigned to debtor |
		-------------------------------------------*/
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			/*----------------------------------------
			| Contract Not Assigned To This Customer |
			-----------------------------------------*/
			print_mess (ML (mlOlMess050));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}
		strcpy (local_rec.cont_desc, cnch_rec.desc);
		DSP_FLD ("cont_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. | 
	-----------------------*/
	if (LCHECK ( "item_no")) 
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
			return (DeleteInvoiceLine ());

		if (last_char == INSLINE)
			return (InsertInvoiceLine ());

		SetSchgCalc ();
		if (ValidateItemNumber (TRUE))
			return (ErrorReturn);

		RunningKit	 (line_cnt);
	
		if (line_cnt < lcount [SCN_LINES])
			RecalcSchg ();

		CalcInvoiceTotal ();

		if (SpecialItem (atoi (cumr_rec.price_type)))
			strcpy (coln_rec.price_type, "S");
		else
			strcpy (coln_rec.price_type, "R");

		if (KIT_START || KIT_END || NON_STOCK (line_cnt))
			strcpy (coln_rec.price_type, " ");

		DSP_FLD ("price_type");

                Pass_Disc = 0;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("descr")) 
	{
		if (NON_STOCK (line_cnt))
			skip_entry = goto_field (field,label ("extend"));

		line_display ();
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Salesman Code At Item Level. |
	---------------------------------------*/
	if (LCHECK ("sman_code")) 
	{
		if (IS_ENTRY)
		{
			strcpy (coln_rec.sman_code,cohr_rec.sale_code);
			DSP_FLD ("sman_code");
		}

		if (SRCH_KEY)
		{
			ShowExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,coln_rec.sman_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------- 
			| Salesman not found. |
			---------------------*/ 
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (ErrorReturn);
		}

		if (FIELD.required != ND)
		{
			/*--------------------
			| Salesman : %s - %s |
			--------------------*/
			print_at (2,1, ML (mlStdMess202),
						exsf_rec.salesman_no,exsf_rec.salesman);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Customer Order Ref At Item Level. |
	--------------------------------------------*/
	if (LCHECK ("ln_ord_ref")) 
	{
		if (IS_ENTRY)
		{
			strcpy (coln_rec.cus_ord_ref,cohr_rec.cus_ord_ref);
			DSP_FLD ("ln_ord_ref");
		}

		if (FIELD.required != ND && NOT_ENTRY)
		{
			if (!strcmp (coln_rec.cus_ord_ref,twenty_spaces))
			{
				if (cumr_rec.po_flag [0] == 'Y')
				{
					/*-------------------------------------
					| Purchase Order Number Must be Input |
					-------------------------------------*/
					print_mess (ML (mlStdMess048));
					sleep (sleepTime);
					return (ErrorReturn);
				}
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pack_size")) 
	{
		if (dflt_used || (IS_ENTRY && F_NOKEY (field)))
		{
			strcpy (coln_rec.pack_size,SR._pack_size);
			DSP_FLD ("pack_size");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qty_ord")) 
	{

		if (NOT_ENTRY && 
				local_rec.qty_ord < local_rec.qty_sup)
		{
		   	print_mess (ML (mlOlMess051));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);

		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Quantity Input. |
	--------------------------*/
	if (LCHECK ("qty_sup")) 
	{
		if (SR._hhbr_hash	==	0L)
		{
			skip_entry	=	goto_field (field, label ("item_no"));
			blank_display ();		
			return (EXIT_SUCCESS);
		}
		/*--------------------------------------------------
		| Reset sale price and discount to default values. |
		--------------------------------------------------*/
		FLD ("sale_price") = inp_sale;
		FLD ("disc") 		= inp_disc;

		if (KIT_ITEM)
		{
			this_page = line_cnt / TABLINES;
			ProcessKitItem (inmr_rec.hhbr_hash,local_rec.qty_sup);
			skip_entry = goto_field (label ("qty_sup"),
		                 	         label ("item_no"));
			SR._qty_sup 		= 0.00;
			local_rec.qty_sup 	= 0.00;
			if (this_page == (line_cnt / TABLINES))
				blank_display ();
			return (EXIT_SUCCESS);
		}
		if (PHANTOM)
			SR._qty_avail = ProcessPhantom (inmr_rec.hhbr_hash);

		if (NON_STOCK (line_cnt))
		{
			local_rec.qty_ord = 0.00;
			local_rec.qty_sup = 0.00;
			DSP_FLD ("qty_ord");
			DSP_FLD ("qty_sup");
			CalcInvoiceTotal ();
			skip_entry = goto_field (field,label ("extend"));
			return (EXIT_SUCCESS);
		}

		if (KIT_START)
		{
			SR._qty_sup			=	0.00;
			SR._sale_price		=	0.00;
			SR._gsale_price		=	0.00;
			local_rec.qty_sup	=	0.00;
			CalcInvoiceTotal ();
			DSP_FLD ("qty_sup");
			return (EXIT_SUCCESS);
		}
		SetSchgCalc ();

		if (local_rec.qty_sup == 0.00)
		{
			/*----------------------------------------
			| Qty Supplied Must be Greater than Zero |
			----------------------------------------*/
		   	print_mess (ML (mlOlMess052));
			sleep (sleepTime);
			clear_mess ();
		   	return (ErrorReturn);
		}
		
		if (!F_HIDE ( label ( "qty_ord") ) && 
			local_rec.qty_sup > local_rec.qty_ord)
		{
				/*--------------------------------------------
				| Qty Supplied Must be Less than Qty Ordered |
				--------------------------------------------*/
		    	print_mess (ML (mlOlMess050));
				sleep (sleepTime);
				clear_mess ();
		    	return (ErrorReturn);
		}

		/*---------------------------------------------------
		| Serial Items Can only have Qty of 0.00 or 1.00	|
		---------------------------------------------------*/
		if (SERIAL_ITEM && local_rec.qty_sup && local_rec.qty_sup != 1.00)
		{
			/*----------------------------------------------------
			| Quantity can only be 0.00 or 1.00 for Serial Items |
			----------------------------------------------------*/
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			return (ErrorReturn);
		}

		/*-------------------------------------------------
		| Recalculate the actual current available stock. |
		-------------------------------------------------*/
		if (!PHANTOM)
			SR._qty_avail = ReCalcAvail ();

		if (!ONLINE_CREDIT)
		{
			PriceProcess (atoi (cohr_rec.pri_type));
			DiscProcess  (atoi (cohr_rec.pri_type));
		}

		/*----------------------------------------------------
		| Validate to see if on hand is less than input qty. |
		----------------------------------------------------*/
		if (ONLINE_INVOICE && 
			!KIT_END &&
			 ((SR._qty_avail - local_rec.qty_sup) < 0.00) &&
				!NO_COST && !NON_STOCK (line_cnt))
		{
			/*-------------------------------------------
			| WARNING only %.2f Available for part # %s |
			|  and you are invoicing %.2f %c			|
			-------------------------------------------*/
			sprintf (err_str, ML (mlOlMess027),
								 SR._qty_avail,
								 clip (local_rec.item_no),
								 local_rec.qty_sup);
			Busy (1);
			cc = WarnUser (err_str, 1, 2);

			InptRes (&local_rec.qty_sup);

			if (skip_entry != 0)
				return (EXIT_SUCCESS);

			if (IS_ENTRY && !ins_flag)
				lcount [SCN_LINES] = line_cnt;

			if (NOT_ENTRY)
				CalcInvoiceTotal ();
		}
		if (IS_ENTRY && NO_COST && F_HIDE (label ("cost_price")))
		{
			/*--------------------
			| Input Cost Price : |
			--------------------*/
			Busy (0);
			print_at (2,1,ML (mlOlMess033));
			coln_rec.cost_price = getmoney (20,2,"NNNNNNN.NN");
		}
			
		if (F_HIDE ( label ( "qty_ord")) )
			local_rec.qty_ord = local_rec.qty_sup;

		SR._qty_sup = local_rec.qty_sup;

		if (SERIAL_ITEM && local_rec.qty_sup == 1.00 && NOT_ENTRY)
		{
			DSP_FLD ("qty_sup");
			do
			{
				get_entry (label ("ser_no"));
				cc = spec_valid (label ("ser_no"));
			} while (cc && !restart);
		}

		if (NOT_ENTRY)
			PrintExtend (line_cnt);

		if (KIT_END && Recalc_kit && NOT_ENTRY)
		{
			CalcKitLine ();
			coln_rec.sale_price		=	SR._ExtendTotal;
			if (local_rec.qty_sup)
				coln_rec.sale_price	/=	local_rec.qty_sup;

			coln_rec.sale_price		=	no_dec (coln_rec.sale_price);
			coln_rec.gsale_price	=	coln_rec.sale_price;
			DSP_FLD ("pack_size");
			SR._sale_price			=	coln_rec.sale_price;
			SR._gsale_price			=	coln_rec.sale_price;
		}
			
		if (!NO_COST && !F_HIDE ( label ( "cost_price") ) )
		{
			coln_rec.cost_price = 0.00;
			DSP_FLD ("cost_price");
		}
			
		if (BONUS || SR._cont_status)
			skip_entry = goto_field (field,label ("extend"));
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

		SR._qty_sup = local_rec.qty_sup;

		RunningKit (line_cnt);

		if (line_cnt < lcount [SCN_LINES])
			RecalcSchg ();

		if ((KIT_END && NOT_ENTRY) || !KIT_END)
			CalcInvoiceTotal ();

		if (MARGIN_OK)
                {
                	if (!CheckMarginOk (SR._act_sale, SR._dis_pc, SR._marg_cost, SR._min_marg))
                        Pass_Disc=1;
                }
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Discount Input. |
	--------------------------*/
	if (LCHECK ( "cost_price") )
	{
		if ((!NO_COST && !F_HIDE ( field)) || KIT_END || KIT_START)
		{
			/*---------------------------------------
			| Cannot Enter Cost Price for this Item |
			---------------------------------------*/
			if (!KIT_START && !KIT_END)
			{
				print_mess (ML (mlOlMess053));
				sleep (sleepTime);
			}
			coln_rec.cost_price = 0.00;
			DSP_FLD ("cost_price");
		}
		else
		{
/*
			if (coln_rec.sale_price == 0.00)
				skip_entry = 0;
			else
				if (coln_rec.disc_pc == 0.00)
					skip_entry = 1;
				else
					skip_entry = 2;
*/
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("price_type"))
	{
		if (dflt_used)
		{
			if (SpecialItem (atoi (cumr_rec.price_type)))
				strcpy (coln_rec.price_type, "S");
			else
				strcpy (coln_rec.price_type, "R");

			if (KIT_END)
				strcpy (coln_rec.price_type, "R");
		}

		if ((coln_rec.price_type [0] == 'S' && 
			!SpecialItem (atoi (cumr_rec.price_type))) || 
			 ((coln_rec.price_type [0] == 'S' && 
			 KIT_END)))
		{	
			print_mess ("Not A Special Item\007");
			sleep (sleepTime);
			return (EXIT_FAILURE);	
		}

		if (coln_rec.price_type [0] == 'N')
		{
			_ignorePromoPrice = 1;
			PriceProcess (atoi (cohr_rec.pri_type));
			_ignorePromoPrice = 0;

			coln_rec.disc_pc  	= 0.00;
			SR._dis_pc 	 	= 0.00;
			SR._calc_disc 		= 0.00;
			SR._disc_a		= 0.00;
			SR._disc_b		= 0.00;
			SR._disc_c		= 0.00;
			DSP_FLD ("disc");

			FLD ("disc") = NA;
		}
		else
			FLD ("disc") = YES;

		if (!KIT_END)
		{
			if (coln_rec.price_type [0] == 'R')
			{
				if (NOT_ENTRY)
				{
					strcpy (SR._pri_or, "N");
					strcpy (SR._dis_or, "N");
				}

				_ignorePromoPrice = 1;
				PriceProcess (atoi (cohr_rec.pri_type));
				_ignorePromoPrice = 0;

				DiscProcess  (atoi (cohr_rec.pri_type));
				DSP_FLD ("sale_price");
				DSP_FLD ("disc");
			}
			else if (coln_rec.price_type [0] == 'S')
			{
				i = (CASH_INVOICE) ? 1 : atoi (cohr_rec.pri_type);
				PriceProcess (i);
				DiscProcess  (i);
				DSP_FLD ("sale_price");
				DSP_FLD ("disc");
			}
			PrintExtend (line_cnt);

			if (NOT_ENTRY)
				CalcInvoiceTotal ();
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Price Input.	|
	-----------------------*/
	if (LCHECK ("sale_price")) 
	{
		previousPrice = SR._sale_price;

		SetSchgCalc ();

		if (dflt_used)
		{
			strcpy (SR._pri_or, "N");

			if (coln_rec.price_type [0] == 'R')
				_ignorePromoPrice = 1;

			PriceProcess (atoi (cohr_rec.pri_type));
			_ignorePromoPrice = 0;

			DiscProcess  (atoi (cohr_rec.pri_type));
			coln_rec.sale_price = SR._sale_price;
			DSP_FLD ("sale_price");
		}

		if (prog_status == ENTRY ||
			 (prog_status != ENTRY && !LineInKit (line_cnt)))
		{
			DiscProcess  (atoi (cohr_rec.pri_type));
		}

		SR._act_sale = coln_rec.sale_price;

		if (SR._tax_amt == 0.00)
			SR._tax_amt = SR._sale_price;

		PrintExtend (line_cnt);

		if (!ONLINE_CREDIT && 
			coln_rec.sale_price < SR._calc_sprice &&
			coln_rec.price_type [0] != 'N')
		{
			/*---------------------------------------
			| Sale Price lower than inventory Price |
			---------------------------------------*/
			print_mess (ML (mlOlMess054));
			sleep (sleepTime);
			if (envVar.OlBelowSale)
				return (ErrorReturn); 
		}
		if (BONUS)
		{
			/*--------------------------------------------------
			| Item is A bonus Item - So it cannot have a Price |
			--------------------------------------------------*/
			print_mess (ML (mlStdMess219));
			sleep (sleepTime);
			clear_mess ();
			coln_rec.sale_price = 0.00;
			DSP_FLD ("sale_price");
			return (EXIT_SUCCESS);
		}

		if (KIT_START)
		{
			coln_rec.sale_price	=	0.00;
			SR._gsale_price		=	0.00;
			SR._sale_price 		=	0.00;
			SR._act_sale		=	0.00;
			DSP_FLD ("sale_price");	
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			if (KIT_END)
			{
				CalcKitLine ();
				coln_rec.sale_price		=	SR._ExtendTotal;
				if (local_rec.qty_sup)
					coln_rec.sale_price	/=	local_rec.qty_sup;

				coln_rec.sale_price		=	no_dec (coln_rec.sale_price);
				coln_rec.gsale_price	=	coln_rec.sale_price;
				SR._act_sale			=	coln_rec.sale_price;
				SR._gsale_price			=	coln_rec.sale_price;
				SR._sale_price 			=	coln_rec.sale_price;
				DSP_FLD ("sale_price");
			}
		}
		if (coln_rec.sale_price == 0.00)
		{
			/*-------------------------------------
			| Is This Really a no Charge Item ? " |
			-------------------------------------*/
			i = prmptmsg (ML (mlStdMess031), "YyNn",1,2);
			Busy (0);
			if (i != 'Y' && i != 'y')
				return (ErrorReturn);
		}

		if (SR._calc_sprice != coln_rec.sale_price)
			strcpy (SR._pri_or, "Y");

		/*---------------------------------	
		| Calculate new GROSS sale price. |
		---------------------------------*/
		SR._gsale_price = no_dec (coln_rec.sale_price / (1.00 - (SR._reg_pc / 100.00)));
		SR._sale_price = GetCusGprice (SR._gsale_price, SR._reg_pc);
				
/*
		skip_entry = (coln_rec.disc_pc == 0.00 && inp_disc) ? 0 : 1;
*/
		skip_entry = (Pass_Disc) ? 0 : 1;

		PrintExtend (line_cnt);
		if (line_cnt >= lcount [SCN_LINES] && skip_entry)
			RecalcSchg ();

		if (IS_ENTRY || edit_status)
		{
			if (!KIT_END)
				DiscProcess  (atoi (cohr_rec.pri_type));
			else
			{
				coln_rec.disc_pc  	= 0.00;
				SR._dis_pc 	 		= 0.00;
				SR._calc_disc 		= 0.00;
				SR._disc_a			= 0.00;
				SR._disc_b			= 0.00;
				SR._disc_c			= 0.00;
				DSP_FLD ("disc");
			}
		}
			
/*
		if (NOT_ENTRY)
		{
*/
			if (LineInKit (line_cnt))
				RunningKit (line_cnt);

			CalcInvoiceTotal ();
/*
		}
*/

		/*--------------------------------------------------------
		| Price Type is forced in the condition to allow checking|
		| of margin since discount is not applicable in the said |
		| price type. (for N & S).    							 |
		--------------------------------------------------------*/
		if (MARGIN_OK && coln_rec.price_type [0] != 'R')
			return (!CheckMarginOk
					 (
						coln_rec.sale_price, 
						SR._dis_pc, 
						SR._marg_cost,
						SR._min_marg
					));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("disc")) 
	{
		previousDiscount = SR._dis_pc;

		if (coln_rec.price_type [0]	==	'N')
		{
			DiscProcess  (atoi (cohr_rec.pri_type));
			PrintExtend (line_cnt);
			return (EXIT_SUCCESS);
		}
		if (LineInKit (line_cnt))
		{
			if (envVar.KitDiscount)
			{
				if (dflt_used)
					DiscProcess  (atoi (cohr_rec.pri_type));
			}
			else
			{
				coln_rec.disc_pc  	= 0.00;
				SR._dis_pc 	 		= 0.00;
				SR._calc_disc 		= 0.00;
				SR._disc_a			= 0.00;
				SR._disc_b			= 0.00;
				SR._disc_c			= 0.00;
			}

        	SR._dis_pc   = ScreenDisc (coln_rec.disc_pc);

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

			DSP_FLD ("disc");
			PrintExtend (line_cnt);

			RunningKit (line_cnt);
			CalcInvoiceTotal ();
			return (!CheckMarginOk
				 	 (
						coln_rec.sale_price, 
						SR._dis_pc, 
						SR._marg_cost,
						SR._min_marg
					));
		}

		if (FLD ( "disc") == NI && IS_ENTRY )
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			if (KIT_END)
			{
				coln_rec.disc_pc 	=	0.00;
				SR._disc_a			=	0.00;
				SR._disc_b			=	0.00;
				SR._disc_c			=	0.00;
			}
			else
			{
				strcpy (SR._dis_or, "N");
				DiscProcess  (atoi (cohr_rec.pri_type));
			}
		}
		if (SR._con_price || SR._cont_status == 2)
		{
			coln_rec.disc_pc 	=	0.00;
			SR._disc_a			=	0.00;
			SR._disc_b			=	0.00;
			SR._disc_c			=	0.00;
			DSP_FLD ("disc");
		}
		SR._dis_pc = ScreenDisc (coln_rec.disc_pc);

		if (SR._calc_disc != ScreenDisc (coln_rec.disc_pc))
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
		PrintExtend (line_cnt);

		if (line_cnt >= lcount [SCN_LINES]) /* If adding item at end of table */
			RecalcSchg ();

		CalcInvoiceTotal ();

		if ( MARGIN_OK /*&& previousDiscount != SR._dis_pc)*/
                 )
                {
			if (!CheckMarginOk
					 (
						SR._act_sale,
						SR._dis_pc, 
						SR._marg_cost,
						SR._min_marg
					))
                           skip_entry = -2;
                }
                return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate serial number input. |
	-------------------------------*/
	if (LCHECK ("ser_no")) 
	{
		if (F_HIDE (field) || FIELD.required == NA || SR._hhwh_hash < 0L)
		{
			strcpy (local_rec.serial_no, ser_space);
			strcpy (SR._serial, local_rec.serial_no);
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}
	
		if (SRCH_KEY)
		{
			SrchInsf (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/*--------------------------
			| Serial No must be input. |
			--------------------------*/
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}

		if (!strcmp (SR._serial, local_rec.serial_no))
			return (EXIT_SUCCESS);

		if (ONLINE_INVOICE)
		{
		   	cc = FindInsf (SR._hhwh_hash, 0L,local_rec.serial_no, "F", "r");
		   	if (cc)
		   	{
		  	 	cc = FindInsf (SR._hhwh_hash, 0L,local_rec.serial_no, "C", "r");
		   		if (!cc)
		   		{
					/*--------------------------------------------
					| Selected serial item is already committed. |
					--------------------------------------------*/
					print_mess (ML (mlOlMess055));
					sleep (sleepTime);
					clear_mess ();
					return (ErrorReturn);
		   		}
		  	 	cc = FindInsf (SR._hhwh_hash, 0L,local_rec.serial_no, "S", "r");
		   		if (!cc)
		   		{
					/*---------------------------------------
					| Selected serial item is already sold. |
					---------------------------------------*/
					print_mess (ML (mlOlMess056));
					sleep (sleepTime);
					clear_mess ();
					return (ErrorReturn);
		   		}
				/*------------------------------
				| No such serial no for item  |
				------------------------------*/
				print_mess (ML (mlStdMess201));
				sleep (sleepTime);
				clear_mess ();
				return (ErrorReturn);
		   	}
		}
		else
		{
		   	cc = FindInsf (SR._hhwh_hash, 0L, local_rec.serial_no, "S", "r");
		   	if (cc)
			{
		   		cc = FindInsf (SR._hhwh_hash, 0L, local_rec.serial_no, "C", "r");
				if (!cc)
				{
					/*----------------------------------------
					| Serial number is on file as committed. |
					| Credit not available.					 |
					----------------------------------------*/
					print_mess (ML (mlOlMess057));
					sleep (sleepTime);
					clear_mess ();
					return (ErrorReturn);
				}
			}
		   	if (cc)
		   	{
				/*-----------------------------------------
				| Do You want to accept this serial item? |
				-----------------------------------------*/
				i= prmptmsg (ML (mlOlMess061), "YyNn", 1, 2);
				Busy (0);
				if (i != 'Y' && i != 'y') 
				{
					/*------------------------
					| Serial Item not found. |
					------------------------*/
					print_mess (ML (mlStdMess201));
					sleep (sleepTime);
					clear_mess ();
					return (ErrorReturn);
				}
				else
				{
					if (CheckDuplicateSearial (local_rec.serial_no,SR._hhsi_hash,line_cnt))
					{
						/*-------------------------
						| Duplicate Serial Number |
						-------------------------*/
						print_mess (ML (mlStdMess097));
						sleep (sleepTime);
						return (ErrorReturn);
					}
					cc = AddSerialNo
						 (
							SR._hhwh_hash,
					 		SR._hhsi_hash,
							local_rec.serial_no,
							StringToDate (local_rec.dbt_date)
						);
					if (cc)
						file_err (cc, insf, "DBFIND");
	
					if (strcmp (SR._serial, ser_space))
					{
						if (OLD_INSF)
						{
							cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "S");
							if (cc && cc < 1000)
								file_err (cc, insf, "DBUPDATE");
						}
						else
							RemoveSerialNo (SR._hhwh_hash,
										   SR._hhsi_hash,
										   SR._serial);
					}
	
					sprintf (SR._serial,"%-25.25s",local_rec.serial_no);
					strcpy (SR._old_insf, "N");
				}
				return (EXIT_SUCCESS);
			}
		}
		if (CheckDuplicateSearial (local_rec.serial_no, SR._hhsi_hash, line_cnt))
		{
			/*-------------------------
			| Duplicate Serial Number |
			-------------------------*/
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}
		/*----------------------------------
		| Reset status of old serial number |
		-----------------------------------*/

		if (strcmp (SR._serial, ser_space))
		{
			if (OLD_INSF)
			{
				if (ONLINE_INVOICE)
					cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "F");
				else
					cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "S");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}
			else
				RemoveSerialNo (SR._hhwh_hash,
							   SR._hhsi_hash,
							   SR._serial);
		}

		sprintf (SR._serial,"%-25.25s",local_rec.serial_no);
		strcpy (SR._old_insf, "Y");

		if (ONLINE_INVOICE)
			cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "F", "C");
		else
			cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "S", "C");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");

		DSP_FLD ("ser_no");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ( "extend")) 
	{
		PrintExtend (line_cnt);
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

	/*----------------------
	| Validate Proof Total |
	----------------------*/
	if (LCHECK ("proof")) 
	{
		clear_mess ();
		inv_proof = 1;
		if (end_input) 
		{
			entry_exit = 1;
			return (EXIT_SUCCESS);
		}
		CalTotalExtended (TRUE);
		/*------------------------
		| Reset to proof screen. |
		------------------------*/
		scn_set (SCN_TENDER);
		proof_total = no_dec (proof_total);
		inv_total = no_dec (inv_total);

		if (CASH_INVOICE)
		{
			cohr_rec.gst += ADJ_VAL (inv_total);
			inv_total += ADJ_VAL (inv_total);
		}

		if (proof_total < inv_total - .01 ||
					proof_total > inv_total + .01)
		{
			if (ONLINE_INVOICE)
				/*sprintf (err_str,"Proof Total %8.2f Not Equal To Invoice Total %8.2f%c",DOLLARS (proof_total),DOLLARS (inv_total),BELL);*/
				sprintf (err_str,ML (mlOlMess006),DOLLARS (proof_total),DOLLARS (inv_total));
			else
				/*sprintf (err_str,"Proof Total %8.2f Not Equal To Credit Total %8.2f%c",DOLLARS (proof_total),DOLLARS (inv_total),BELL);*/
				sprintf (err_str,ML (mlOlMess044),DOLLARS (proof_total),DOLLARS (inv_total));
			print_mess (err_str);
			sleep (sleepTime);
			inv_proof = 1;
			return (ErrorReturn); 
		}
		inv_proof = 0;
		batch_tot += inv_total;
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Amount tendered |
	--------------------------*/
	if (LCHECK ( "tendered")) 
	{
		clear_mess ();
		CalTotalExtended (TRUE);
		/*---------------------------
		| Reset to tendered screen. |
		---------------------------*/
		scn_set (SCN_TENDER);
		amt_tendered = no_dec (amt_tendered);
		inv_total = no_dec (inv_total);
		if (CASH_INVOICE)
		{
			cohr_rec.gst += ADJ_VAL (inv_total);
			inv_total += ADJ_VAL (inv_total);
		}

		if (amt_tendered < inv_total - .01 && ONLINE_INVOICE)
		{
		    /*Amount tendered %8.2f Less than Invoice Total %8.2f%c*/
		    sprintf (err_str, ML (mlOlMess028),
									DOLLARS (amt_tendered),
									DOLLARS (inv_total));
		    print_mess (err_str);
			sleep (sleepTime);
		    inv_proof = 1;
		    return (ErrorReturn); 
		}

		amt_change = amt_tendered - inv_total;
		amt_change = no_dec (amt_change);
		putchar (BELL);
		DSP_FLD ("change");
		inv_proof = 0;
		batch_tot += inv_total;
		if (end_input) 
			entry_exit = 1;
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}
/*------------------------------------------
| Recalculate the current available stock. |
------------------------------------------*/
float 
ReCalcAvail 
 (
	void
)
{
	float	realStock;

	/*----------------------
	| Look up incc record. |
	----------------------*/
    incc2_rec.hhcc_hash = ccmr_rec.hhcc_hash;
    incc2_rec.hhbr_hash = SR._hhbr_hash;
    cc = find_rec (incc, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	if (envVar.SoFwdAvl)
	{
		realStock = incc2_rec.closing_stock -
					incc2_rec.committed -
					incc2_rec.backorder - 
					incc2_rec.forward;
	}
	else
	{
		realStock = incc2_rec.closing_stock -
					incc2_rec.committed -
					incc2_rec.backorder;
	}
	if (envVar.QcApply && envVar.SkQcAvl)
		realStock -= incc2_rec.qc_qty;

	/*------------------------------------------------------------
	| Add into available any stock that was on line when loaded. |
	------------------------------------------------------------*/
	if (SR._hhbr_hash == SR._origHhbr)
		realStock += SR._origOrdQty;

	return (realStock);
}

int
win_function2
 (
	int fld, 
	int lin, 
	int scn, 
	int mode
)
{
	int		i;

	if (scn != 2)
		return (FALSE);

	if (store [ lin ]._hhbr_hash == 0L)
		return (FALSE);
	
	if (IS_ENTRY)
		return (FALSE);

	/*---------------------
	| Check for contract. |
	---------------------*/
	if (store [lin]._cont_status)
	{
		/*Item has a contract price, negotiation window not available. */
		print_mess (ML (mlOlMess007));
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
	negoRec.qOrd			=	store [ lin ]._qty_sup;
	negoRec.qBord			=	0.00;

	negoRec.regPc			=  	store [ lin ]._reg_pc;
	negoRec.discArray [0]	=	store [ lin ]._disc_a;
	negoRec.discArray [1]	=	store [ lin ]._disc_b;
	negoRec.discArray [2]	=	store [ lin ]._disc_c;
	negoRec.grossPrice		=	store [ lin ]._gsale_price;
	negoRec.salePrice		=	store [ lin ]._sale_price;
	negoRec.margCost		= 	store [ lin ]._marg_cost;
	negoRec.outer_size		= 	store [ lin ]._outer;

	NegPrice (2, 10, local_rec.item_no, coln_rec.item_desc, 
				   store [ lin ]._cumulative, scn);

	/*-------------------------------------------
	| Clear space where negotiation window was. |
	-------------------------------------------*/
	for (i = 10; i < 15; i++)
	{
		move (1, i);
		cl_line ();
	}
	if (!restart)
	{
		local_rec.qty_sup 			=   negoRec.qOrd;

		store [ lin ]._qty_sup		=	negoRec.qOrd;
		store [ lin ]._reg_pc		= 	negoRec.regPc;
		store [ lin ]._disc_a		= 	negoRec.discArray [0];
		store [ lin ]._disc_b		= 	negoRec.discArray [1];
		store [ lin ]._disc_c		= 	negoRec.discArray [2];
		store [ lin ]._dis_pc		=	CalcOneDisc (store [ lin ]._cumulative,
													 negoRec.discArray [0],
													 negoRec.discArray [1],
													 negoRec.discArray [2]);
		store [ lin ]._gsale_price 	= 	negoRec.grossPrice;
		store [ lin ]._sale_price	=	negoRec.salePrice;
		store [ lin ]._act_sale		=	negoRec.salePrice;
		store [ lin ]._marg_cost 	=	negoRec.margCost;

		coln_rec.disc_pc  			= 	ScreenDisc (store [ lin ]._dis_pc);
		coln_rec.sale_price 		= 	store [ lin ]._sale_price;

		if (store [ lin ]._calc_sprice != coln_rec.sale_price)
			strcpy (store [ lin ]._pri_or, "Y");

		if (store [ lin ]._calc_disc != ScreenDisc (coln_rec.disc_pc))
			strcpy (store [ lin ]._dis_or, "Y");

		PrintExtend (lin);
		putval (lin);
	}
	CalcInvoiceTotal ();
	
	restart = FALSE;

	return (TRUE);
}

void
PrintCustomer 
 (
	void
)
{
	DSP_FLD ("name");
	DSP_FLD ("addr1");
	DSP_FLD ("addr2");
	DSP_FLD ("addr3");
	DSP_FLD ("shipaddr1");
	DSP_FLD ("shipaddr2");
	DSP_FLD ("shipaddr3");
}

/*===================================
| Print line extend and value each. |
===================================*/
void
PrintExtend 
 (
	int	CurLine
)
{
	CalLineExtended (CurLine);
	local_rec.extend	=	store [ CurLine ]._ExtendTotal;
	local_rec.each 		= 	store [ CurLine ]._ValueEach;

	DSP_FLD ("each");
	DSP_FLD ("extend");
}

/*=======================
| Validate item number. |
=======================*/
int
ValidateItemNumber 
 (
	int		getFields
)
{
	int		itemChanged = FALSE;
	long	orig_hhbr_hash;
	char	*sptr;

	abc_selfield (inmr,"inmr_id_no");

	/*-----------------------
	| Default Is To Delete	|
	-----------------------*/
	if (!strcmp (local_rec.item_no,sixteen_space))
		strcpy (local_rec.item_no,"\\D             ");

	if (IS_ENTRY)
	{
		sprintf (coln_rec.serial_no,"%25.25s"," ");
		coln_rec.disc_pc = 0.00;
	}
	if (IS_ENTRY)
		sprintf (local_rec.serial_no,"%25.25s"," ");

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.item_no);

	SR._bonus [0] = (CheckBonus (inmr_rec.item_no)) ? 'Y' : 'N';

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
		cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		/*-----------------
		| Item not found. |
		-----------------*/
		errmess (ML (mlStdMess001));
		sleep (sleepTime);
		clear_mess ();
		return (ErrorReturn);
	}
	/*-------------------------------
	| First character is a '\'		|
	| \\	- start/end of kit		|
	| \D	- delete current line	|
	| \I	- insert before current	|
	-------------------------------*/
	SR._kit_flag = K_NONE;

	if (local_rec.item_no [0] == 92)	/*	If Backslash.   */
	{
		if (!F_HIDE ( label ( "ser_no") ))
			FLD ("ser_no") = NA;

		if (CheckIncc ())
			return (EXIT_SUCCESS);

		if (local_rec.item_no [1] == 92)
		{
			memset ((char *) &SR, '\0', sizeof (struct storeRec));

			SR._hhbr_hash	=	kit_hash = inmr_rec.hhbr_hash;
			SR._hhum_hash	=	inmr_rec.std_uom;
			SR._hhsi_hash	=	alt_hash
								 (
									inmr_rec.hhbr_hash,
									inmr_rec.hhbr_hash
								);
			SR._hhwh_hash	=	-1L;
			SR._gst_pc		= (float) ((notax) ? 0.00 : inmr_rec.gst_pc);
			sprintf (SR._serial,"%-25.25s"," ");
			strcpy	 (SR._category,"           ");
			SR._bonus [0]	= 'N';
			SR._class [0]	= inmr_rec.inmr_class [0];
			SR._kit_flag	= K_START;
			SR._ExtendTotal	= 0.00;
			strcpy	 (SR._pack_size,		"     ");
			strcpy	 (SR._schg_flag,		inmr_rec.schg_flag);
			SR._weight		=	0.00;
			SR._con_price	=	FALSE;
			SR._indent		=	FALSE;

			local_rec.qty_ord		= 0.00;
			local_rec.qty_sup		= 0.00;
			coln_rec.sale_price		= 0.00;
			coln_rec.gsale_price	= 0.00;
			coln_rec.disc_pc			= 0.00;
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
				skip_entry = 2;
			return (EXIT_SUCCESS);
		}

		if (local_rec.item_no [1] == 'D')
			return (DeleteInvoiceLine ());

		if (local_rec.item_no [1] == 'I')
			return (InsertInvoiceLine ());
	}
	orig_hhbr_hash = inmr_rec.hhbr_hash;

	SuperSynonymError ();

	sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
	if (SERIAL_ITEM && F_HIDE ( label ("ser_no")))
	{
		/*-----------------------------
		| Cannot Invoice Serial Items |
		-----------------------------*/
		print_mess (ML (mlOlMess022));
		sleep (sleepTime);
		return (ErrorReturn);
	}

	if (NOT_ENTRY && SR._hhbr_hash != inmr_rec.hhbr_hash &&
		SR._hhbr_hash != 0)
	{
		if (inmr_rec.inmr_class [0] == 'K')
		{
			/*--------------------------------------------
			| Cannot substitute Kits for standard items. |
			--------------------------------------------*/
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (ErrorReturn);
		}
		itemChanged = TRUE;
		if (strcmp (SR._serial, ser_space) && SERIAL)
		{
			if (OLD_INSF)
			{
				if (ONLINE_INVOICE)
					cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "F");
				else
					cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "S");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}
			else
				RemoveSerialNo (SR._hhwh_hash, SR._hhsi_hash, SR._serial);
		}
		strcpy (local_rec.serial_no, ser_space);
		strcpy (SR._serial, local_rec.serial_no);
		strcpy (SR._org_ser, local_rec.serial_no);
		DSP_FLD ("ser_no");
	}

	SR._hhbr_hash 	= 	inmr_rec.hhbr_hash;
	SR._hhum_hash	=	inmr_rec.std_uom;
	SR._hhsi_hash 	= 	alt_hash (inmr_rec.hhbr_hash, 
								  inmr_rec.hhsi_hash);
	SR._weight 		= 	inmr_rec.weight;
	SR._outer 		= 	inmr_rec.outer_size;

	SR._dflt_disc 	= 	inmr_rec.disc_pc;
	SR._class [0] 	= 	inmr_rec.inmr_class [0];
	SR._outer 		= 	inmr_rec.outer_size;

	strcpy (SR._category		, inmr_rec.category);
	strcpy (SR._sellgrp		, inmr_rec.sellgrp);
	strcpy (SR._schg_flag	, inmr_rec.schg_flag);
	strcpy (SR._pack_size	, inmr_rec.pack_size);

	/*-------------------------
	| Check for Indent items. |
	-------------------------*/
	if (strncmp ( inmr_rec.item_no, "INDENT", 6) || envVar.SoDisIndent )
		SR._indent = FALSE;
	else
		SR._indent = TRUE;

	if (CheckIncc ())
		return (EXIT_SUCCESS);

	if (inmr_rec.hhbr_hash != orig_hhbr_hash) 
	{
		/*Item No. %s Has Been Superceded by Item No. %s %c*/
		Busy (0);
		sprintf (err_str, ML (mlStdMess238),
							clip (local_rec.sup_part),
							clip (local_rec.item_no));
		print_mess (err_str);
		sleep (sleepTime);
	}
	cc	=	FindInei 
			(
				alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash),
				comm_rec.est_no,
				"r"
			);
	if (cc)
		file_err (cc, inei, "DBFIND");

	SR._marg_cost = (ineiRec.avge_cost == 0.00) 
					? twodec (ineiRec.last_cost) 
					: twodec (ineiRec.avge_cost);

	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,SR._category);
	cc = find_rec (excf,&excf_rec,COMPARISON,"r");
	if (cc)
	{
		/*---------------------
		| Category not found. |
		---------------------*/
		print_mess (ML (mlStdMess004));
		sleep (sleepTime);
		return (ErrorReturn);
	}
	SR._min_marg = (float) twodec (excf_rec.ol_min_marg);

	strcpy (coln_rec.item_desc,inmr_rec.description);

	if (!BONUS)
	{
		coln_rec.tax_pc  	= inmr_rec.tax_pc;
		coln_rec.gst_pc  	= inmr_rec.gst_pc;
		SR._tax_pc			= inmr_rec.tax_pc;
		SR._gst_pc			= inmr_rec.gst_pc;
		SR._tax_amt			= inmr_rec.tax_amount;
	}
	else
	{
		sprintf (so_bonus,"%-2.2s", envVar.SoSpecial);
		sptr = clip (inmr_rec.item_no);
		sprintf (local_rec.item_no,"%-s%-.*s", sptr,16 - (int) strlen (sptr),so_bonus);

		DSP_FLD ("item_no");
		coln_rec.tax_pc  = 0.00;
		coln_rec.gst_pc  = 0.00;
		SR._tax_amt			= 0.00;
	}

	DSP_FLD ("item_no");
	DSP_FLD ("descr");

	/*------------------------
	| Item is a serial item. |
	------------------------*/
	if (SERIAL_ITEM)
	{
		SR._hhwh_hash = incc_rec.hhwh_hash;

		if (!F_HIDE ( label ("ser_no")))
			FLD ("ser_no") = YES;
	}
	else
	{
		SR._hhwh_hash = -1L;
		if (!F_HIDE ( label ("ser_no")))
			FLD ("ser_no") = NA;
	}

	if (itemChanged)
	{
		local_rec.qty_ord = 0.00;
		local_rec.qty_sup = 0.00;
		SR._qty_sup = local_rec.qty_sup;
		DSP_FLD ("qty_ord");
		DSP_FLD ("qty_sup");
		PriceProcess (atoi (cohr_rec.pri_type));
		DiscProcess  (atoi (cohr_rec.pri_type));
		CalcInvoiceTotal ();
	}

	if (envVar.SoFwdAvl)
	{
		SR._qty_avail = incc_rec.closing_stock -
						incc_rec.committed -
						incc_rec.backorder - 
						incc_rec.forward;
	}
	else
	{
		SR._qty_avail = incc_rec.closing_stock -
						incc_rec.committed -
						incc_rec.backorder;
	}
	if (envVar.QcApply && envVar.SkQcAvl)
		SR._qty_avail -= incc_rec.qc_qty;

	if (itemChanged && getFields)
	{
		if (SERIAL)
		{
			local_rec.qty_sup = 1.00;
			DSP_FLD ("qty_sup");
			cc = spec_valid (label ("qty_sup"));
			if (skip_entry)
			while (cc && !restart && !skip_entry)
			{
				get_entry (label ("qty_sup"));
				cc = spec_valid (label ("qty_sup"));
			}
			if (skip_entry)
			{
				SR._hhbr_hash = -1;
				return (ErrorReturn);
			}
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
				SR._hhbr_hash = -1;
				return (ErrorReturn);
			}
		}
	}

	if (NOT_ENTRY)
		PrintExtend (line_cnt);
	
	if (NON_STOCK (line_cnt))
		skip_entry = goto_field (label ("item_no"),label ("extend"));
	else
		skip_entry = 2;

	sptr = clip (inmr_rec.description);
	
	if (strlen (sptr) == 0)
		skip_entry = 1;

	if (!F_HIDE ( label ( "hide")) )
		DSP_FLD ("hide");

	tab_other (line_cnt);

	return (EXIT_SUCCESS);
}
/*===================================
| Determine whether the current kit |
| contains an NS item.              |
===================================*/
int
KitHasNS (void)
{
	int		tmpLineCnt = line_cnt;
	int		i;

	/*-------------------------------------
	| Start at the current line and work  |
	| backwards until we get to the start |
	| of the kit                          |
	-------------------------------------*/
	for (i = line_cnt - 1; i >= 0; i--)
	{
		if (store [i]._kit_flag == K_START)
			break;
		
		getval (i);
		if (store [i]._class [0] == 'Z' || 
			!strcmp (local_rec.item_no, "NS              "))
		{
			getval (tmpLineCnt);
			return (TRUE);
		}
	}

	getval (tmpLineCnt);

	return (FALSE);
}

void
PriceProcess (
	int		pType)
{
	float	regPc;
	double	grossPrice;

	SR._pricing_chk	= FALSE;

	if (BONUS)
	{
		coln_rec.sale_price 	= 0.00;
		SR._act_sale 			= 0.00;
		SR._calc_sprice			= 0.00;
		SR._sale_price 			= 0.00;
		DSP_FLD ("sale_price");

		coln_rec.disc_pc  		= 0.00;
		SR._dis_pc 	 			= 0.00;
		SR._calc_disc 			= 0.00;
		DSP_FLD ("disc");
		return;
	}
	grossPrice = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  comm_rec.cc_no,
							  cumr_rec.area_code,
							  cumr_rec.class_type,
							  SR._sellgrp,
							  cumr_rec.curr_code,
					  		  pType,
							  cumr_rec.class_type,
							  cnch_rec.exch_type,
							  cumr_rec.hhcu_hash,
							  ccmr_rec.hhcc_hash,
							  SR._hhbr_hash,
							  SR._category,
							  cnch_rec.hhch_hash,
							  (envVar.useSystemDate) ? TodaysDate (): comm_rec.dbt_date,
							  local_rec.qty_sup,
							  1.0,	/* No exchange rate as local only. */
							  FGN_CURR,
							  &regPc);

	SR._pricing_chk	= TRUE;

	SR._calc_sprice = GetCusGprice (grossPrice, regPc);

	if (SR._pri_or [0] == 'N')
	{
		SR._gsale_price 	= 	grossPrice;
		SR._sale_price 		=	SR._calc_sprice;
		SR._reg_pc 			= 	regPc;
		coln_rec.sale_price = 	SR._calc_sprice;
		SR._act_sale 		= 	SR._calc_sprice;
	}
	SR._con_price 		= (_CON_PRICE) ? TRUE : FALSE;
	SR._cont_status  	= _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess  
 (
	int		pType
)
{
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (SR._cont_status == 2 || SR._con_price || SR._indent || coln_rec.price_type [0] == 'N')
	{
		coln_rec.disc_pc  	= 0.00;
		SR._dis_pc 	 		= 0.00;
		SR._calc_disc 		= 0.00;
		SR._disc_a			= 0.00;
		SR._disc_b			= 0.00;
		SR._disc_c			= 0.00;
		DSP_FLD ("disc");
		return;
	}

	if (SR._pricing_chk == FALSE)
		PriceProcess (atoi (cohr_rec.pri_type));

	cumDisc		=	GetCusDisc (	comm_rec.co_no,
								comm_rec.est_no,
								ccmr_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								SR._hhsi_hash,
								SR._category,
								SR._sellgrp,
								pType,
								SR._gsale_price,
								SR._reg_pc,
								local_rec.qty_sup,
								discArray);
							
	if (SR._dis_or [0] == 'Y')
	{
		DSP_FLD ("disc");
		return;
	}
	SR._calc_disc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	if (SR._dis_or [0] == 'N')
	{
		coln_rec.disc_pc 	=	ScreenDisc (SR._calc_disc);
		SR._dis_pc			=	SR._calc_disc;

		SR._disc_a 			= 	discArray [0];
		SR._disc_b 			= 	discArray [1];
		SR._disc_c 			= 	discArray [2];
		SR._cumulative 		= 	cumDisc;

		if (SR._dflt_disc > ScreenDisc (coln_rec.disc_pc) &&
		    SR._dflt_disc != 0.0)
		{
			coln_rec.disc_pc = 	ScreenDisc (SR._dflt_disc);
			SR._calc_disc	=	SR._dflt_disc;
			SR._dis_pc		=	SR._dflt_disc;
			SR._disc_a 		= 	SR._dflt_disc;
			SR._disc_b 		= 	0.00;
			SR._disc_c 		= 	0.00;
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

	if (!lcount [SCN_LINES])
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
			cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "F");
		else
			cc = UpdateInsf (SR._hhwh_hash, 0L,SR._serial, "C", "S");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
	else
	{
		RemoveSerialNo (SR._hhwh_hash, SR._hhsi_hash, SR._serial);
	}

	lcount [SCN_LINES]--;

	for (i = line_cnt,line_cnt = 0;line_cnt < lcount [SCN_LINES];line_cnt++)
	{
		if (line_cnt >= i)
		{
			memcpy  ((char *) &SR, (char *) &store [line_cnt + 1],
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
	InitStore (lcount [SCN_LINES] + 1);

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
	
	Busy (0);

	PV_schg_line = lcount [SCN_LINES];
	line_cnt = i;
	getval (line_cnt);

	RunningKit (line_cnt);
	CalcInvoiceTotal ();

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

	if (lcount [SCN_LINES] >= vars [label ("item_no")].row)
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

	for (i = line_cnt,line_cnt = lcount [SCN_LINES];line_cnt > i;line_cnt--)
	{
		memcpy  ((char *) &SR, (char *) &store [line_cnt - 1],
						sizeof (struct storeRec));
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [SCN_LINES]++;
	PV_schg_line = lcount [SCN_LINES];
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

	memset ((char *) &store [ line_cnt ], '\0', sizeof (struct storeRec));
	InitStore (line_cnt);

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	Busy (0);

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

void
SetInvoiceDefaults 
 (
	int		new_inv
)
{
	int		i;

	init_vars (SCN_TENDER);	
	init_vars (SCN_PROOF);	

	if (new_inv)
	{
/*
		strcpy (cohr_rec.op_id,op_id);
*/
		strcpy (cohr_rec.tax_code,  cumr_rec.tax_code);
		strcpy (cohr_rec.area_code, cumr_rec.area_code);
		strcpy (cohr_rec.pri_type,  cumr_rec.price_type);
		strcpy (cohr_rec.batch_no,  "00000");
		strcpy (local_rec.pri_desc, GetPriceDesc (atoi(cohr_rec.pri_type)));
		/*--------------------------------
		| Get any special instrunctions. |
		--------------------------------*/
		if (CHARGE_INVOICE)
		{
			open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg1;
			if (find_rec (exsi,&exsi_rec,COMPARISON,"r"))
				sprintf (local_rec.spinst [0], "%60.60s"," ");
			else 
				sprintf (local_rec.spinst [0], "%-60.60s", exsi_rec.inst_text);

			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg2;
			if (find_rec (exsi,&exsi_rec,COMPARISON,"r"))
				sprintf (local_rec.spinst [1], "%60.60s"," ");
			else 
				sprintf (local_rec.spinst [1], "%-60.60s", exsi_rec.inst_text);

			strcpy (exsi_rec.co_no,comm_rec.co_no);
			exsi_rec.inst_code = cumr_rec.inst_fg3;
			if (find_rec (exsi,&exsi_rec,COMPARISON,"r"))
				sprintf (local_rec.spinst [2], "%60.60s"," ");
			else 
				sprintf (local_rec.spinst [2], "%-60.60s", exsi_rec.inst_text);

			abc_fclose (exsi);
		}

		cohr_rec.date_create = TodaysDate ();
		strcpy (cohr_rec.time_create, TimeHHMM ());
		strcpy (cohr_rec.cons_no,"                ");
		strcpy (cohr_rec.fix_exch,"N");
		strcpy (cohr_rec.area_code,cumr_rec.area_code);
		strcpy (cohr_rec.sale_code,local_rec.dflt_sale_no);
		strcpy (cohr_rec.pri_type,cumr_rec.price_type);
		strcpy (cohr_rec.sell_terms,"   ");
		sprintf (cohr_rec.pay_terms, "%-40.40s", cumr_rec.crd_prd);

		sprintf (cohr_rec.ins_det,"%-30.30s"," ");

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cohr_rec.pay_terms,
				p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (cohr_rec.pay_terms,"%-40.40s",
							p_terms [i]._pterm);
				break;
			}
		}
	}
	else
	{
		if (CASH_INVOICE)
		    sprintf (local_rec.pri_desc, "%-1.1s", cohr_rec.pri_type);
		else
		    strcpy (local_rec.pri_desc, GetPriceDesc (atoi(cohr_rec.pri_type)));

		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,cohr_rec.dp_no);
		if ((cc = find_rec (cudp,&cudp_rec,COMPARISON,"r")))
			sprintf (cudp_rec.dp_name,"%40.40s"," ");
		strcpy (local_rec.ord_desc,
				 (cohr_rec.ord_type [0] == 'D') ? "Domestic" :
								   "Export  ");

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
		}
	}

	for (i = 0;strlen (STerms [i]._scode);i++)
	{
		if (!strncmp (cohr_rec.sell_terms,
				STerms [i]._scode,strlen (STerms [i]._scode)))
		{
			sprintf (local_rec.sell_desc,"%-30.30s", STerms [i]._sdesc);
			break;
		}
	}
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
	if ((cc = find_rec (exsf,&exsf_rec,COMPARISON,"r")))
		sprintf (exsf_rec.salesman,"%40.40s"," ");

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,cohr_rec.area_code);
	if ((cc = find_rec (exaf,&exaf_rec,COMPARISON,"r")))
		sprintf (exaf_rec.area_code,"%40.40s"," ");

	scn_set (SCN_HEAD);
}

/*=======================================================================
| Routine to read all coln records whoose hash matches the one on the   |
| cohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
int
LoadInvoiceLines 
 (
	long	HHCO_HASH
)
{
	int		in_kit	=	0;
	char	*sptr;

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (SCN_LINES);
	lcount [SCN_LINES] = 0;

	Busy (1);
	move (10,2);

	abc_selfield (inmr,"inmr_hhbr_hash");

	coln_rec.hhco_hash = HHCO_HASH;
	coln_rec.line_no = 0;

	cc = find_rec (coln,&coln_rec,GTEQ,read_flag);

	while (!cc && HHCO_HASH == coln_rec.hhco_hash) 
	{
		if (coln_rec.line_no < 100)
		{
			putchar ('R');
			fflush (stdout);
		}

	   	/*------------------
	    | Get part number. |
	    ------------------*/
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
	    cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
		    file_err (cc, "inmr", "DBFIND");

		if (coln_rec.bonus_flag [0] == 'Y')
		{
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s", sptr, 16 - (int) strlen (sptr),"/B");
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);

		cc	=	FindInei
				(
					alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash),
					cohr_rec.br_no,
					"r"
				);
		if (cc)
			file_err (cc, inei, "DBFIND");

		store [lcount [SCN_LINES]]._marg_cost = (ineiRec.avge_cost == 0.00) 
										? twodec (ineiRec.last_cost) 
										: twodec (ineiRec.avge_cost);

		incc_rec.hhcc_hash = coln_rec.incc_hash;
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						  				  inmr_rec.hhsi_hash);

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		    file_err (cc, "incc", "DBFIND");

		if (inmr_rec.serial_item [0] == 'Y')
		{
			if (vars [label ("ser_no")].required == ND)
			{
				/*-----------------------------
				| Cannot Invoice Serial Items |
				-----------------------------*/
				print_mess (ML (mlOlMess022));
				sleep (sleepTime);
				scn_set (SCN_LINES);
				return (ErrorReturn);
			}
			store [lcount [SCN_LINES]]._hhwh_hash = incc_rec.hhwh_hash;
			sprintf (local_rec.serial_no, "%-25.25s", coln_rec.serial_no);
			sprintf (store [lcount [SCN_LINES]]._serial,"%-25.25s", local_rec.serial_no);
			sprintf (store [lcount [SCN_LINES]]._org_ser,"%-25.25s", local_rec.serial_no);
			strcpy (store [lcount [SCN_LINES]]._old_insf, "Y");
			if (strcmp (store [lcount [SCN_LINES]]._serial, ser_space))
			{
				if (ONLINE_INVOICE)
					cc = UpdateInsf (store [lcount [SCN_LINES]]._hhwh_hash,0L,
									store [lcount [SCN_LINES]]._serial, 
									"F", "C");
				else
					cc = UpdateInsf (store [lcount [SCN_LINES]]._hhwh_hash,0L,
									store [lcount [SCN_LINES]]._serial, 
									"S", "C");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}
		}
		else
		{
			store [lcount [SCN_LINES]]._hhwh_hash = -1L;
			sprintf (local_rec.serial_no, "%-25.25s", " ");
			sprintf (store [lcount [SCN_LINES]]._serial,"%-25.25s"," ");
			strcpy (store [lcount [SCN_LINES]]._old_insf, "N");
		}

		/*----------------------------------------------
		| if the line has a contract on it then  user  |
		| not allowed to edit price or disc            |
		----------------------------------------------*/
		store [lcount [SCN_LINES]]._cont_status = coln_rec.cont_status;

		/*-------------------------
		| Check for Indent items. |
		-------------------------*/
		if (strncmp (inmr_rec.item_no,"INDENT",6) || envVar.SoDisIndent)
			store [ lcount [SCN_LINES] ]._indent = FALSE;
		else
			store [ lcount [SCN_LINES] ]._indent = TRUE;

		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "excf", "DBFIND");

		store [lcount [SCN_LINES]]._min_marg 		= (float) twodec (excf_rec.ol_min_marg);
		store [lcount [SCN_LINES]]._dflt_disc 	= inmr_rec.disc_pc;
		store [lcount [SCN_LINES]]._tax_pc 		= coln_rec.tax_pc;
		store [lcount [SCN_LINES]]._gst_pc 		= coln_rec.gst_pc;
		store [lcount [SCN_LINES]]._dis_pc 		= coln_rec.disc_pc;
		store [lcount [SCN_LINES]]._tax_amt 		= inmr_rec.tax_amount;
		store [lcount [SCN_LINES]]._weight 		= inmr_rec.weight;
		store [lcount [SCN_LINES]]._outer 		= inmr_rec.outer_size;
		store [lcount [SCN_LINES]]._class [0] 		= inmr_rec.inmr_class [0];
		store [lcount [SCN_LINES]]._bonus [0] 		= coln_rec.bonus_flag [0];
		store [lcount [SCN_LINES]]._act_sale 		= coln_rec.sale_price;
		store [lcount [SCN_LINES]]._reg_pc 		= coln_rec.reg_pc;
		store [lcount [SCN_LINES]]._disc_a 		= coln_rec.disc_a;
		store [lcount [SCN_LINES]]._disc_b 		= coln_rec.disc_b;
		store [lcount [SCN_LINES]]._disc_c 		= coln_rec.disc_c;
		store [lcount [SCN_LINES]]._cumulative 	= coln_rec.cumulative;
		store [lcount [SCN_LINES]]._gsale_price 	= coln_rec.gsale_price;
		store [lcount [SCN_LINES]]._hhbr_hash 	= coln_rec.hhbr_hash;
		store [lcount [SCN_LINES]]._hhum_hash 	= coln_rec.hhum_hash;
		store [lcount [SCN_LINES]]._hhsi_hash 	= alt_hash (inmr_rec.hhbr_hash,
													inmr_rec.hhsi_hash);
		strcpy (store [lcount [SCN_LINES]]._category,inmr_rec.category);
		strcpy (store [lcount [SCN_LINES]]._sellgrp,inmr_rec.sellgrp);
		strcpy (store [lcount [SCN_LINES]]._pack_size,inmr_rec.pack_size);
		strcpy (store [lcount [SCN_LINES]]._pri_or, "N");
		strcpy (store [lcount [SCN_LINES]]._dis_or, "N");
		strcpy (store [lcount [SCN_LINES]]._schg_flag,
		    (ONLINE_CREDIT && inmr_rec.schg_flag [0] == 'S') ?
						"0" : inmr_rec.schg_flag);
			
		local_rec.qty_ord = coln_rec.q_order + coln_rec.q_backorder;
		local_rec.qty_sup = coln_rec.q_order;

		/*---------------------------
		| Start or end of Kit item. |
		---------------------------*/
		if (local_rec.item_no [0] == 92 && local_rec.item_no [1] == 92)
		{
			kit_hash = inmr_rec.hhbr_hash;
			if (in_kit)
			{
				store [lcount [SCN_LINES]]._kit_flag 	 = K_END;
				store [lcount [SCN_LINES]]._qty_sup 	 = coln_rec.q_order;
				store [lcount [SCN_LINES]]._gst_pc 	 = (float) ((notax) 
											? 0.00 : inmr_rec.gst_pc);
				store [lcount [SCN_LINES]]._tax_pc	 = 0.00;
				store [lcount [SCN_LINES]]._sale_price = coln_rec.sale_price;
				store [lcount [SCN_LINES]]._sale_price = coln_rec.gsale_price;
				store [lcount [SCN_LINES]]._dflt_price = 0.00;
				in_kit = 0;
			}
			else
			{
				store [lcount [SCN_LINES]]._kit_flag 		= K_START;
				store [lcount [SCN_LINES]]._qty_sup 		= 0.00;
				store [lcount [SCN_LINES]]._gst_pc 		= 0.00;
				store [lcount [SCN_LINES]]._tax_pc 		= 0.00;
				store [lcount [SCN_LINES]]._tax_amt 		= 0.00;
				store [lcount [SCN_LINES]]._sale_price 	= 0.00;
				store [lcount [SCN_LINES]]._gsale_price 	= 0.00;
				store [lcount [SCN_LINES]]._dflt_price 	= 0.00;
				in_kit = 1;
			
				local_rec.qty_ord		= 0.00;
				local_rec.qty_sup 		= 0.00;
				coln_rec.sale_price 	= 0.00;
				coln_rec.gsale_price 	= 0.00;
				coln_rec.disc_pc 		= 0.00;
				coln_rec.tax_pc 		= 0.00;
				coln_rec.due_date 		= 0L;
				sprintf (coln_rec.serial_no,"%-25.25s"," ");
			}
			store [lcount [SCN_LINES]]._ExtendTotal = 0.00;
			store [lcount [SCN_LINES]]._qty_avail = 0.00;
		}
		else
		{
			store [lcount [SCN_LINES]]._qty_avail = 
									incc_rec.closing_stock -
								  	incc_rec.committed -
									incc_rec.backorder;

			if (envVar.QcApply && envVar.SkQcAvl)
				store [lcount [SCN_LINES]]._qty_avail -= incc_rec.qc_qty;

			store [lcount [SCN_LINES]]._qty_sup = coln_rec.q_order;

			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
		}
		store [lcount [SCN_LINES]]._origHhbr		= 	coln_rec.hhbr_hash;
		store [lcount [SCN_LINES]]._origOrdQty	= 	 (long) (coln_rec.q_order + 
													coln_rec.q_backorder);
		/*---------------------------
		| Calculate Extended Total. |
		---------------------------*/
		CalLineExtended (lcount [SCN_LINES]);

		local_rec.each 		= 	store [lcount [SCN_LINES]]._ValueEach;
		local_rec.extend 	= 	store [lcount [SCN_LINES]]._ExtendTotal;
		if (KIT_END)
			local_rec.extend 	*=	 (double) local_rec.qty_sup;
			
		putval (lcount [SCN_LINES]++);
		if (lcount [SCN_LINES] > MAXLINES) 
			break;

		cc = find_rec (coln,&coln_rec,NEXT,read_flag);
	}
	prog_status = !ENTRY;
	
	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;

	/*-----------------------------------------------
	| Set the maximum number of lines in the        |
	| TABular screen to the number of lines loaded. |
	-----------------------------------------------*/
	if (OL_DISPLAY)
		vars [scn_start].row = lcount [SCN_LINES];

	scn_set (SCN_HEAD);
	return (EXIT_SUCCESS);
}

MENUTAB upd_menu [] =
	{
		{ " 1. CONFIRM INVOICE ",
			" PRINT INVOICE AND TRANSFER TO SALES ORDER SYSTEM " },
		{ " 2. RE-EDIT         ",
			" RE-EDIT CURRENT INVOICE " },
		{ " 3. ABANDON INVOICE ",
			" DELETE PREVIOUSLY SAVED INVOICE " },
		{ " 4. AMOUNT TENDERED ",
			" RE-ENTER AMOUNT TENDERED " },
		{ " 5. ESTIMATE        ",
			" ESTIMATE ONLY INVOICE " },
		{ ENDMENU }
	};

int
UpdateMenu 
 (
	int _lpno
)
{
	for (;;)
	{
		mmenu_print ("  INVOICE OPTIONS   ", upd_menu, 0);
		switch (mmenu_select (upd_menu))
		{
			case 0 :	/* CONFIRM */
				/*CASH_DRAW %s*/
#ifndef GVISION
				sprintf (err_str,ML (mlOlMess008), ttyname (0));
				sys_exec (err_str);
#endif	/* GVISION */
				UpdateInvoice (TRUE, _lpno);
				return (TRUE);

			case 1 :	/* RE-EDIT */
			case -1 :
				return (FALSE);

			case 2 :	/* ABANDON */
				DeleteInvoice (TRUE);
				return (TRUE);

			case 3 :	/* AMT TENDERED */
				ProcessScreen (3, TRUE);
				break;

			case 4 :	/* ESTIMATE */
				/*UpdateInvoice (FALSE, 0);*/
				UpdateInvoice (FALSE, _lpno);
				return (TRUE);

			default :
				break;
		}
	}
}

void
NextInvoiceNo 
 (
	int		conf_flag
)
{
	long	inv_no;

	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	strcpy (cohr_rec.stat_flag,creat_flag);

	if (!strcmp (cohr_rec.inv_no,"00000000") || conf_flag) 
	{
		open_rec (cohr2,cohr_list,COHR_NO_FIELDS,"cohr_id_no2");

		/*------------------------------------------------------
		| Is invoice number to come from department of branch. |
		------------------------------------------------------*/
		if (envVar.SalesOrderNumber == BY_DEPART)
		{
			strcpy (cudp_rec.co_no, comm_rec.co_no);
			strcpy (cudp_rec.br_no, comm_rec.est_no);
			cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "cudp", "DBFIND");
		}
		else
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "esmr", "DBFIND");
		}

		/*-------------------------------------------
		| Check if Invoice / Credit Note No Already	|
		| Allocated. If it has been then skip		|
		-------------------------------------------*/
		while (! (CheckInvoiceDups (conf_flag)));

		strcpy (cohr_rec.inv_no, GenInvoiceNo (conf_flag, 0L));
		if (envVar.SalesOrderNumber == BY_DEPART)
		{
			if ((cc = abc_update (cudp, &cudp_rec)))
				file_err (cc, "cudp", "DBUPDATE");
		}
		else
		{
			if ((cc = abc_update (esmr, &esmr_rec)))
				file_err (cc, "esmr", "DBUPDATE");
		}
		
		local_rec.dflt_inv_no = 0L;
	}
	else
	{
		inv_no = atol (cohr_rec.inv_no);
		local_rec.dflt_inv_no = inv_no + 1L;
	}
}

/*
void
check_invoice
 (
	char *invoice
)
{
	strcpy (cohr2_rec.co_no,  comm_rec.co_no);
	strcpy (cohr2_rec.br_no,  comm_rec.est_no);
	strcpy (cohr2_rec.type,   type_flag);
	strcpy (cohr2_rec.inv_no, invoice);
	cc = find_rec (cohr2,&cohr2_rec,COMPARISON,"r");
	if (!cc)
		
	

}
*/

/*===========================
| Update all on-line files. |
===========================*/
void
UpdateInvoice 
 (
	int		conf_flag,
	int		_lpno
)
{
	int		start_kit	=	-1;
	int		in_kit		=	0;
	double	k_total		=	0.00;
	double	k_disc		=	0.00;
	double	k_tax		=	0.00;
	double	k_gst		=	0.00;

	int		new_coln;

	if (!lcount [SCN_LINES])
	{
		/*---------------------------------------------
		| Can't create Invoice with no line details. |
		---------------------------------------------*/
		print_mess (ML (mlOlMess023));
		sleep (sleepTime);
		return;
	}
	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;
		
	if (CHARGE_INVOICE)
	{
		sprintf (cohr_rec.din_1, "%-60.60s", local_rec.spinst [0]);
		sprintf (cohr_rec.din_2, "%-60.60s", local_rec.spinst [1]);
		sprintf (cohr_rec.din_3, "%-60.60s", local_rec.spinst [2]);
	}

	else 
	if (CASH_INVOICE && ONLINE_INVOICE)
		strcpy (cohr_rec.app_inv_no, cohr_rec.inv_no);

	cohr_rec.gross = cohr_rec.disc = cohr_rec.tax = 0.00;
	cohr_rec.gst = 0.00;
	cohr_rec.exch_rate = 1.00;

	if (conf_flag)
	{
		cohr_rec.type [0] = (ONLINE_CREDIT) ? 'C' :'I';

		/*---------------------------------
		| If we are confirming an invoice |
		| or credit then set the date and |
		| time created to NOW.            |
		---------------------------------*/
		cohr_rec.date_create = TodaysDate ();
		strcpy (cohr_rec.time_create, TimeHHMM ());
	}
	else
		strcpy (cohr_rec.type, type_flag);

	cohr_rec.date_required = StringToDate (local_rec.systemDate);
	cohr_rec.date_raised = cohr_rec.date_required;
	strcpy (cohr_rec.dl_name, local_rec.dl_name);

	if (!new_invoice)
		strcpy  (cohr_rec.inv_no, Loaded_no);

	strcpy (cohr_rec.stat_flag, creat_flag);
	if (!manualInvoice)
		NextInvoiceNo (conf_flag);

	if (new_invoice)
	{
		print_at (2,24,"%s", "A - ");
		strcpy (cohr_rec.inv_print, print_yn);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

		if ((cc = abc_add (cohr,&cohr_rec)))
			file_err (cc, "cohr", "DBADD");

		if ((cc = find_rec (cohr,&cohr_rec,COMPARISON,"u")))
			file_err (cc, "cohr", "DBFIND");

		abc_unlock (esmr);
	}
	else
		print_at (2,24,"%s", "U - ");

	fflush (stdout);
	abc_selfield (inmr,"inmr_hhbr_hash");

	scn_set (SCN_LINES);
	for (line_cnt = 0;line_cnt < lcount [SCN_LINES];line_cnt++) 
	{
		if (KIT_START)
		{
			/*-----------------------------------
			| Zero totals for kit components	|
			-----------------------------------*/
			k_total		=	0.00;
			k_disc		= 	0.00;
			k_tax		= 	0.00;
			k_gst		=	0.00;
			start_kit 	=	line_cnt;
			in_kit 		=	TRUE;
			continue;
		}
		if (KIT_END)
			in_kit	=	FALSE;

		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		new_coln = find_rec (coln,&coln_rec,COMPARISON,"u");

		getval (line_cnt);
		CalLineExtended (line_cnt);

		if (in_kit)
		{
			k_total	+= l_total;
			k_disc 	+= l_disc;
			k_tax 	+= l_tax;
			k_gst 	+= l_gst;
		}
		else
		{
			cohr_rec.gross 	+= l_total;
			cohr_rec.disc 	+= l_disc;
			cohr_rec.tax 	+= l_tax;
			cohr_rec.gst 	+= l_gst;
		}

		if (new_coln)
			coln_rec.incc_hash = ccmr_rec.hhcc_hash;
		coln_rec.hhco_hash 		= 	cohr_rec.hhco_hash;
		coln_rec.line_no 		= 	line_cnt;
		coln_rec.hhbr_hash 		= 	SR._hhbr_hash;

		coln_rec.hhum_hash 		= 	SR._hhum_hash;

	    coln_rec.q_order 	 	= 	local_rec.qty_sup;
	    coln_rec.q_backorder 	= 	local_rec.qty_ord - local_rec.qty_sup;
		coln_rec.gst_pc 		= 	 (float) (( notax) ? 0.00 : SR._gst_pc);
		coln_rec.tax_pc 		= 	 (float) (( notax) ? 0.00 : SR._tax_pc);
		coln_rec.reg_pc			= 	SR._reg_pc;
		coln_rec.disc_a			= 	SR._disc_a;
		coln_rec.disc_b			= 	SR._disc_b;
		coln_rec.disc_c			= 	SR._disc_c;
		coln_rec.cumulative		= 	SR._cumulative;
		coln_rec.gsale_price	= 	SR._gsale_price;
		coln_rec.cont_status	= 	SR._cont_status;
		coln_rec.gross 			= 	l_total;
		coln_rec.amt_disc 		= 	l_disc;
		coln_rec.amt_tax 		= 	l_tax;
		coln_rec.amt_gst 		= 	l_gst;
		coln_rec.due_date 		= 	local_rec.lsystemDate;

		strcpy (coln_rec.serial_no,SR._serial);
		strcpy (coln_rec.bonus_flag, (BONUS) ? "Y" : "N");
		strcpy (coln_rec.hide_flag,"N");

		if (!new_coln)
		{
			putchar ('U');
			strcpy (coln_rec.stat_flag,"B");
			/*------------------------
			| Update existing order. |
			------------------------*/
			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			if ((cc = abc_update (coln,&coln_rec)))
				file_err (cc, "coln", "DBUPDATE");

			abc_unlock (coln);
		}
		else 
		{
			putchar ('A');
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = line_cnt;
			strcpy (coln_rec.stat_flag,"N");

			coln_rec.disc_pc = ScreenDisc (coln_rec.disc_pc);
			if ((cc = abc_add (coln,&coln_rec)))
			    	file_err (cc, "coln", "DBADD");
		}
		fflush (stdout);

		if (KIT_END)
		{
			coln2_rec.hhco_hash 	=	cohr_rec.hhco_hash;
			coln2_rec.line_no 	=	start_kit;

			new_coln = find_rec (coln,&coln2_rec,COMPARISON,"u");
			coln_rec.hhco_hash 		=	cohr_rec.hhco_hash;
			coln_rec.line_no 		=	start_kit;
			coln_rec.sale_price 	=	-coln_rec.sale_price;
			coln_rec.gross 			=	-coln_rec.gross;
			strcpy (coln_rec.bonus_flag, (BONUS) ? "Y" : "N");
			strcpy (coln_rec.hide_flag,	"N");
			if (!new_coln)
			{
				putchar ('U');
				cc = abc_update (coln,&coln_rec);
				if (cc)
					file_err (cc, "coln", "DBUPDATE");
			}
			else 
			{
				putchar ('A');
				cc = abc_add (coln,&coln_rec);
				if (cc)
					file_err (cc, "coln", "DBADD");
			}
			fflush (stdout);
		}
	}

	for (line_cnt = lcount [SCN_LINES];line_cnt < MAXLINES;line_cnt++) 
	{
		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		cc = find_rec (coln,&coln_rec,GTEQ,"u");
		if (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
		{
			putchar ('D');
			fflush (stdout);
			FreeSearialNo (coln_rec.line_no,local_rec.serial_no);
			abc_unlock (coln);
			if ((cc = abc_delete (coln)))
			    	file_err (cc, "coln", "DBDELETE");
		}
		else
		{
			abc_unlock (coln);
			break;
		}
	}

	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (!new_invoice) 
	{	
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (!lcount [SCN_LINES]) 
		{
			abc_unlock (cohr);
			if ((cc = abc_delete (cohr)))
		    	file_err (cc, "cohr", "DBDELETE");
		}
	}

	/*---------------------------------------
	| Calc Totals of Gst etc for cohr	|
	---------------------------------------*/
	CalTotalExtended (FALSE);
	if (CASH_INVOICE)
		cohr_rec.gst += ADJ_VAL (inv_total);

	cohr_rec.date_required = StringToDate (local_rec.systemDate);
	cohr_rec.date_raised = cohr_rec.date_required;

	if (lcount [SCN_LINES]) 
	{
		if ((cc = abc_update (cohr,&cohr_rec)))
			file_err (cc, "cohr", "DBUPDATE");
	}
	abc_unlock (cohr);

	strcpy (local_rec.prev_inv_no,cohr_rec.inv_no);
	strcpy (local_rec.prev_dbt_no,cumr_rec.dbt_no);

	if (_lpno)
	{
    	OlOpen ("OL_CTR_INV", "ol_ctr_inv", lpno);

		if (conf_flag)
			fprintf (olPout,"C\n");
		else
			fprintf (olPout,"E\n");
				
		OlPrint (cohr_rec.hhco_hash);
		strcpy (cohr_rec.inv_print, "Y");

		OlClose ();
	}

	if (AUTO_SK_UP && conf_flag)
		AddSobg (0,"SU",cohr_rec.hhco_hash);

	if (conf_flag)
	{
		if (CASH_INVOICE)
			PauseForKey (23,0,ML (mlStdMess042),0);
	}
	return;
}

/*=====================================
| Delete SAVED/UNCONFIRMED Invoice.   |
=====================================*/
void
DeleteInvoice 
 (
 int		stat_flag
)
{
	if (new_invoice)
	{
		/*-----------------------------------------------
		| Can't abandon Invoice that hasn't been saved. |
		-----------------------------------------------*/
		if (stat_flag)
		{
			print_mess (ML (mlOlMess024));
			sleep (sleepTime);
		}
		return;
	}

	if (stat_flag)
	{
		print_at (2,24,"%s", "D - ");
		fflush (stdout);
	}

	strcpy (cohr_rec.inv_no, Loaded_no);
	strcpy (cohr_rec.type, type_flag);
	if ((cc = find_rec (cohr,&cohr_rec,COMPARISON,"u")))
		file_err (cc, "cohr", "DBFIND");

	scn_set (SCN_LINES);
	for (line_cnt = 0;line_cnt < lcount [SCN_LINES];line_cnt++) 
	{
		coln_rec.hhco_hash = cohr_rec.hhco_hash;
		coln_rec.line_no = line_cnt;
		cc = find_rec (coln,&coln_rec,GTEQ,"u");
		if (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
		{
			if (stat_flag)
				putchar ('D');

			fflush (stdout);

	    	if (ONLINE_INVOICE)
				cc = UpdateInsf (SR._hhwh_hash, 0L, SR._serial,"C","F");
		    else
				cc = UpdateInsf (SR._hhwh_hash, 0L, SR._serial, "C","S");

			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");

			abc_unlock (coln);
			if ((cc = abc_delete (coln)))
				file_err (cc, "coln", "DBDELETE");
		}
		else
		{
			abc_unlock (coln);
			break;
		}
	}
	abc_unlock (cohr);
	if ((cc = abc_delete (cohr)))
		file_err (cc, "cohr", "DBDELETE");

	return;
}

int
CheckInvoiceDups 
 (
	int		conf_flag
)
{
	abc_selfield ("cohr2", "cohr_id_no2");
	strcpy (cohr2_rec.co_no,  comm_rec.co_no);
	strcpy (cohr2_rec.br_no,  comm_rec.est_no);
	strcpy (cohr2_rec.type,   (conf_flag)? (ONLINE_CREDIT)? "C" : "I" : type_flag);
	strcpy (cohr2_rec.inv_no, GenInvoiceNo (conf_flag, 1L));
	return (find_rec (cohr2,&cohr2_rec,EQUAL,"r"));
}

void
CalTotalExtended 
 (
	int		cal_line
)
{
	int		i;
	double	wk_value = 0.00;
	double	value;

	if (cal_line)
	{
		scn_set (SCN_LINES);

		inv_total = 0.00;

		cohr_rec.gross 	= 0.00;
		cohr_rec.disc 	= 0.00;
		cohr_rec.tax 	= 0.00;
		cohr_rec.gst 	= 0.00;

		for (i = 0;i < lcount [SCN_LINES];i++)
		{
			if (LineInKit (i))
				continue;

			getval (i);
			CalLineExtended (i);
			cohr_rec.gross 	+= l_total;
			cohr_rec.disc 	+= l_disc;
			cohr_rec.tax 	+= l_tax;
			cohr_rec.gst 	+= l_gst;
		}
	}

	if (notax)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	value = cohr_rec.freight + 
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3;

	wk_value *= value;
	cohr_rec.gst += wk_value;
	
	cohr_rec.gst = no_dec (cohr_rec.gst);
	
	inv_total = cohr_rec.gross + 
				cohr_rec.tax + 
				cohr_rec.freight +
				cohr_rec.gst - 
				cohr_rec.disc - 
				cohr_rec.deposit -
				cohr_rec.ex_disc + 
				cohr_rec.insurance +
				cohr_rec.other_cost_1 + 
				cohr_rec.other_cost_2 +
		    	cohr_rec.other_cost_3;

/*
	if (envVar.dbNettUsed)
		inv_total -= cohr_rec.disc;
*/
}

void
CalLineExtended 
 (
	int		line_cnt
)
{
	double TmpDiscAmt = 0.00;
	float  TmpDiscPc = 0.00;

	l_each = 0.00;
	if (NON_STOCK (line_cnt))
	{
		SR._qty_sup 	= 0.00;
		SR._sale_price 	= 0.00;
		SR._outer 		= 0.00;
		SR._dis_pc 		= 0.00;
		SR._tax_pc 		= 0.00;
		SR._tax_amt		= 0.00;
		SR._gst_pc 		= 0.00;
	}

	/*-----------------------------------------------
	| Update coln gross tax and disc for each line. |
	-----------------------------------------------*/

	l_total	 = (double) SR._qty_sup;
	l_total *= out_cost (SR._act_sale, SR._outer);
	l_total	 = no_dec (l_total);

	t_total	 = (double) SR._qty_sup;
	t_total *= out_cost (SR._tax_amt,SR._outer);
	t_total	 = no_dec (t_total);

	l_disc 	= (double) (SR._dis_pc);
	l_disc 	= DOLLARS (l_disc);
	l_disc *= l_total;
	l_disc 	=  no_dec (l_disc);

	TmpDiscPc 	= DOLLARS (SR._dis_pc);
	l_each 		= out_cost (SR._act_sale,SR._outer);
	TmpDiscAmt 	= (l_each * TmpDiscPc);
	TmpDiscAmt 	= no_dec (TmpDiscAmt);
	l_each     -= TmpDiscAmt;

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) (SR._tax_pc);
		l_tax = DOLLARS (l_tax);

		if (cohr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
		{
			if (envVar.dbNettUsed)
				l_tax *= (l_total - l_disc);
			else
				l_tax *= l_total;
		}

		l_tax = no_dec (l_tax);
	}
	
	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) (SR._gst_pc);
		l_gst = DOLLARS (l_gst);
		if (envVar.dbNettUsed)
			l_gst *= ((l_total - l_disc) + l_tax);
		else
			l_gst *= (l_total + l_tax);
	}
	SR._ExtendTotal 	= no_dec (l_total - l_disc + l_tax);
	SR._ValueEach 		= no_dec (l_each);
}

/*===============================================
|	free or commit insf records appropriately	|
===============================================*/
void
UpdateSerialNo 
 (
	int		line_no,
	char	*ser_no
)
{
	/*-----------------------
	|	line being deleted	|
	-----------------------*/
	if (!local_rec.qty_sup)
	{
		/*---------------------------
		|	free old serial number	|
		---------------------------*/
		if (strcmp (ser_no,ser_space))
			FreeSearialNo (line_no,ser_no);

		/*---------------------------
		|	free new serial number	|
		---------------------------*/
		if (strcmp (local_rec.serial_no,ser_space))
			FreeSearialNo (line_no,local_rec.serial_no);

		return;
	}

	/*--------------------------------
	|	if serial number has changed |
	--------------------------------*/
	if (strcmp (ser_no,local_rec.serial_no))
	{
		/*---------------------------
		|	free old serial number	|
		---------------------------*/
		if (strcmp (ser_no,ser_space))
			FreeSearialNo (line_no,ser_no);
		/*-------------------------------
		|	commit new serial number	|
		-------------------------------*/
		if (strcmp (local_rec.serial_no,ser_space))
			CommitSerialNo (line_no,local_rec.serial_no);
	}
}

/*=======================
|	Free insf record	|
=======================*/
void
FreeSearialNo 
 (
	int		line_no,
	char	*ser_no
)
{
	if (!strcmp (ser_no,ser_space))
		return;

	/*---------------------------------------
	| serial_item and serial number input	|
	---------------------------------------*/
	if (SERIAL)
	{
	    if (ONLINE_INVOICE)
	    {
		cc = UpdateInsf (store [line_no]._hhwh_hash,0L, ser_no,"C","F");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	    }
	    else
	    {

	    	if ((cc = UpdateInsf (store [line_no]._hhwh_hash,0L, ser_no,"S","F")))
			cc = UpdateInsf (store [line_no]._hhwh_hash,0L,ser_no, "C","F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
	    }
	}
}

/*===============================
|	Commit insf record	|
===============================*/
void
CommitSerialNo 
 (
	int		line_no,
	char	*ser_no
)
{
	if (!strcmp (ser_no,ser_space))
		return;

	/*---------------------------------------
	| serial_item and serial number input	|
	---------------------------------------*/
	if (SERIAL && ONLINE_INVOICE)
	{
		cc = UpdateInsf (store [line_no]._hhwh_hash,0L, ser_no,"F","C");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}

void
ShowPaymentTerms
 (
	void
)
{
	int		i = 0;
	work_open ();
	save_rec ("#Cde","#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		if ((cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm)))
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*===========================
| Search for Selling Terms. |
===========================*/
void
SrchSell (
 void)
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
/*=================================
| Search for Selling Price Types. |
=================================*/
void
SrchPrice (
 void)
{
	work_open ();
	save_rec ("# ","#Price ");

	cc = save_rec (err_str,comm_rec.price1_desc);
	cc = save_rec (err_str,comm_rec.price2_desc);
	cc = save_rec (err_str,comm_rec.price3_desc);
	cc = save_rec (err_str,comm_rec.price4_desc);
	cc = save_rec (err_str,comm_rec.price5_desc);
	cc = save_rec (err_str,comm_rec.price6_desc);
	cc = save_rec (err_str,comm_rec.price7_desc);
	cc = save_rec (err_str,comm_rec.price8_desc);
	cc = save_rec (err_str,comm_rec.price9_desc);
	cc = disp_srch ();
	work_close ();
}

/*======================
| Search for salesman. |
======================*/
void
ShowExsf
 (
	char *key_val
)
{
	work_open ();
	save_rec ("#Sm","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) &&
		!strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		if ((cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman)))
			break;
		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	if ((cc = find_rec (exsf,&exsf_rec,COMPARISON,"r")))
		file_err (cc, "exsf", "DBFIND");
}

/*==================
| Search for area. |
==================*/
void
SrchExaf
 (
	char *key_val
)
{
	work_open ();
	save_rec ("#Ar","#Area.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) &&
			!strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		if ((cc = save_rec (exaf_rec.area_code,exaf_rec.area)))
			break;
		cc = find_rec (exaf,&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	if ((cc = find_rec (exaf,&exaf_rec,COMPARISON,"r")))
		file_err (cc, "exaf", "DBFIND");
}

/*=============================================
| Search routine for Serial Item master file. |
=============================================*/
void
SrchInsf
 (
	char *key_val, 
	int line_no
)
{
	work_open ();
	save_rec ("#      Serial Item.         ","# ");
	if (ONLINE_INVOICE)
	{
		cc = FindInsf (SR._hhwh_hash,0L, "","F","r");
		while (!cc && SR._hhwh_hash == insfRec.hhwh_hash &&
		    strncmp (insfRec.serial_no,key_val,strlen (key_val)) < 0)
			cc = FindInsf (0L,0L, "","F","r");
	}
	else
	{
		cc = FindInsf (SR._hhwh_hash,0L, "","S","r");
		while (!cc && 
                    SR._hhwh_hash == insfRec.hhwh_hash &&
		    strncmp (insfRec.serial_no,key_val,strlen (key_val)) < 0)
			cc = FindInsf (0L,0L, "","S","r");
	}

	while (!cc && SR._hhwh_hash == insfRec.hhwh_hash &&
	!strncmp (insfRec.serial_no,key_val,strlen (key_val)))
	{
		if (!CheckDuplicateSearial (insfRec.serial_no,
					store [line_no]._hhsi_hash,line_no))
		{
			if ((cc = save_rec (insfRec.serial_no, inmr_rec.item_no)))
				break;
		}
		if (ONLINE_INVOICE)
			cc = FindInsf (0L,0L, "","F","r");
		else
			cc = FindInsf (0L,0L, "","S","r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (ONLINE_INVOICE)
		cc = FindInsf (SR._hhwh_hash,0L, temp_str,"F","r");
	else
		cc = FindInsf (SR._hhwh_hash,0L, temp_str,"S","r");

	if (cc && cc != 1)
		file_err (cc, insf, "DBFIND");

	strcpy (local_rec.serial_no,insfRec.serial_no);
}

void
ShowAppliedInvoiceNo 
 (
	char 	*key_val
)
{
	strcpy (type_flag, "I");
	SearchInvoiceNo (key_val);
	strcpy (cohr_rec.type, "R");
	strcpy (type_flag, "R");
}

void
SearchInvoiceNo 
 (
	char	*key_val
)
{
	char 	save_str [128];
	long	tmp_hash;

	work_open ();
	/*---------------------------------------
	|	Store Original cumr hash value. |
	---------------------------------------*/
	tmp_hash = cumr_rec.hhcu_hash;

	save_rec (SRCH,"#Cust Order       Operator ID.    Inv. Type");
	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type,  type_flag);
	sprintf (cohr_rec.inv_no,"%-8.8s",key_val);

	cc = find_rec (cohr,&cohr_rec,GTEQ,"r");

	while (!cc &&	!strcmp (cohr_rec.co_no,comm_rec.co_no) &&
			!strcmp (cohr_rec.br_no,comm_rec.est_no) &&
			cohr_rec.type [0] == type_flag [0] &&
			!strncmp (cohr_rec.inv_no,key_val,strlen (key_val)))
	{
		if (cumr_rec.hhcu_hash == cohr_rec.hhcu_hash)
		{
			/*--------------------------------------------
			|  Load cumr info to print CHARGE or CASH.   |
			--------------------------------------------*/
			if ((cc = find_rec (cumr2,&cumr_rec,COMPARISON,"r")))
				file_err (cc, "cumr2", "DBFIND");

			sprintf (save_str, "%-20.20s   %-14.14s  %-9.9s",
					cohr_rec.cus_ord_ref,
					cohr_rec.op_id,
					 (CASH_INVOICE) ? "CASH" : "CHARGE");
			if ((cc = save_rec (cohr_rec.inv_no, save_str)))
				break;
		}
		cc = find_rec (cohr,&cohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type,  type_flag);
	strcpy (cohr_rec.inv_no,temp_str);


	if ((cc = find_rec (cohr,&cohr_rec,COMPARISON,"r")))
		file_err (cc, "cohr", "DBFIND");

	/*--------------------------------
	|	Reload Original cumr record. |
	---------------------------------*/
	cumr_rec.hhcu_hash = tmp_hash;
	if ((cc = find_rec (cumr2,&cumr_rec,COMPARISON,"r")))
		file_err (cc, "cumr2", "DBFIND");
}

/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.				                |
| Return 1 if duplicate					                |
=======================================================*/
int
CheckDuplicateSearial 
 (
	char	*serial_no,
	long	hhbr_hash,
	int		line_no
)
{
	int		i;

	if (!SERIAL_ITEM)
		return (EXIT_SUCCESS);

	for (i = 0;i < lcount [2];i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*-----------------------------------
		| Only compare serial numbers for	|
		| the same item number			    |
		-----------------------------------*/
		if (store [i]._hhsi_hash == hhbr_hash)
		{
			if (!strcmp (store [i]._serial,serial_no))
				return (ErrorReturn);
		}
	}
	return (EXIT_SUCCESS);
}

/*=====================================================================
| Input responses to stock quantity on hand less-than input quantity. |
=====================================================================*/
void
InptRes 
 (
	float		*lcl_qty
)
{
	int		i;
	int		displayed = FALSE;
	float	wk_qty;
	char	val_keys [150];
	char	disp_str [150];

	cc = 0;
	skip_entry = 0;

	Busy (0);

	if (SERIAL_ITEM)
	{
		sprintf (disp_str, ML (mlOlMess067), ta [8], ta [9], ta [8], ta [9]);
		strcpy (val_keys, "CcNnAa");
	}
	else
	{
		sprintf (disp_str, ML (mlOlMess068),
				ta [8], ta [9], ta [8], ta [9],
				ta [8], ta [9], ta [8], ta [9]);
		strcpy (val_keys, "OoCcNnAaRr");
	}

	if (strcmp (inmr_rec.alternate,sixteen_space))
	{
		sprintf (err_str,ML (mlOlMess009), ta [8], ta [9]);
		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}
	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, 2);
		Busy (0);

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
			LogLostSales (*lcl_qty);
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
			if (i == 'N' || i == 'n')
				DisplayStockWindow (SR._hhsi_hash, ccmr_rec.hhcc_hash);
			else
				DisplayStockWindow (SR._hhsi_hash, 0L);
#else
			Busy (1);
			if (!wpipe_open)
			{
				if (OpenSkWin ())
					break;
			}
			if (i == 'N' || i == 'n')
				fprintf (wout,"%10ld%10ld\n", SR._hhsi_hash,
						      ccmr_rec.hhcc_hash);
			else
				fprintf (wout,"%10ld%10ld\n",SR._hhsi_hash,0L);

			fflush (wout);
			IP_READ (np_fn);
			Busy (0);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*------------------------------------------------------
		| Quantity has been reduced to equal quantity on hand. |
		------------------------------------------------------*/
		case	'R':
		case	'r':
			wk_qty = *lcl_qty;
			*lcl_qty = SR._qty_avail;
			if (*lcl_qty < 0.00)
				*lcl_qty = 0.00;
			LogLostSales (wk_qty - *lcl_qty);
			break;

		/*------------------------------
		| Substitute Alternate number. |
		------------------------------*/
		case	'S':
		case	's':
			sprintf (so_bonus,"%-2.2s", envVar.SoSpecial);
			sprintf (err_str,"%s%s",clip (inmr_rec.alternate),
						 (BONUS) ? so_bonus : " ");

			sprintf (local_rec.item_no,"%-16.16s",err_str);
			if (ValidateItemNumber (FALSE))
			{
				skip_entry = goto_field (label ("qty_sup"),
					         	  label ("item_no"));
			}
			else
			{
				CheckSerialQuantity (lcl_qty);
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
OpenSkWin
 (
	void
)
{
	np_fn = IP_CREATE (getpid ());
	if (np_fn < 0)
	{
		win_ok = FALSE;
		return (ErrorReturn);
	}
	if ((wout = popen ("so_pwindow","w")) == 0)
	{
		win_ok = FALSE;
		return (ErrorReturn);
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
LogLostSales
 (
	float	lost_qty
)
{
	int		i;
	char	shhbr_hash [10];
	char	shhcc_hash [10];
	char	shhcu_hash [10];
	char	wk_qty [11];
	char	wk_value [11];
		
	Busy (1);
	/*---------------------------
	| Log to lost sales <y/n> ? |
	---------------------------*/
	i = prmptmsg (ML (mlStdMess177),"YyNn",1,2);
	if (i == 'N' || i == 'n')
	{
		Busy (0);
		return;
	}

	sprintf (shhbr_hash,"%09ld",alt_hash (inmr_rec.hhbr_hash,
					     inmr_rec.hhsi_hash));
	sprintf (shhcc_hash,"%09ld",incc_rec.hhcc_hash);
	sprintf (shhcu_hash,"%09ld",cumr_rec.hhcu_hash);
	sprintf (wk_qty,"%10.2f",lost_qty);
	sprintf (wk_value,"%10.2f",coln_rec.sale_price);

	* (arg) = "ol_lostsale";
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
	Busy (0);
	return;
}

/*========================================
| Clear popup window ready for new item. |
========================================*/
void
ClearWindow
 (
	void
)
{
	int		i;
	for (i = 17; i < 24 ; i++)
	{
		move (0,i); 
		fflush (stdout);
		cl_line ();
	}
	PrintCoStuff ();
}

/*============================
| Warn user about something. |
============================*/
int
WarnUser
 (
	char	*wn_mess,
	int		wn_flip,
	int		mess_len
)
{
	int		wn_cnt;	
	int		i;
	
	if (OL_DISPLAY)
		return (EXIT_SUCCESS);

	for (wn_cnt = 1; wn_cnt < mess_len + 1 ; wn_cnt++)
	{
		clear_mess ();
		print_mess (wn_mess);
		sleep (sleepTime);
	}

	if (!wn_flip)
	{
		Busy (0);
		/*---------------------------------------------------------
		| Enter 'Y' if Credit is Approved / 'N' if not Appoved / |
		| 'M' for more details on credit required. ? 			 |
		---------------------------------------------------------*/
		i = prmptmsg (ML (mlOlMess062),"YyNnMm",1,2);
		move (1,2);
		cl_line ();
		if (i == 'Y' || i == 'y') 
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML (mlOlMess063), "YyNn",1,2);
			heading (SCN_HEAD);
			scn_display (SCN_HEAD);
			Busy (0);
			if (i == 'Y' || i == 'y') 
				return (EXIT_SUCCESS);
		}
		return (ErrorReturn);
	}

	if (wn_flip == 9)
		return (ErrorReturn);
	else
		return (EXIT_SUCCESS);
}

void
Busy
 (
	int flip
)
{
	print_at (2,1,"%-90.90s", (flip) ? ML (mlStdMess035) : " ");
	fflush (stdout);
}

void
PrintCoStuff
 (
	void
)
{
	move (1,20);
	line (131);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

	strcpy (err_str,ML (mlStdMess039));
	print_at (21,60,err_str,comm_rec.est_no,comm_rec.est_name);

	strcpy (err_str,ML (mlStdMess099));
	print_at (22,0,err_str,comm_rec.cc_no,comm_rec.cc_name);

	strcpy (err_str,ML (mlStdMess127));
	print_at (22,60,err_str,cudp_rec.dp_no,cudp_rec.dp_name);
}

void
CalcInvoiceTotal
 (
	void
)
{
	int		i;
	int		no_lines;

/*
	The following line is different than its equivilent in other programs.
	This is to deal with SURCHRGES.
	Change at your peril.
*/
	no_lines = ((line_cnt > lcount [SCN_LINES] - 1)) ? line_cnt : lcount [SCN_LINES];

	inv_tot = dis_tot = tax_tot = gst_tot = 0.00;

	for (i = 0;i <= no_lines;i++) 
	{
		if (LineInKit (i))
			continue;

		CalLineExtended (i);
		inv_tot += l_total;
		dis_tot += l_disc;
		tax_tot += l_tax;
		gst_tot += l_gst;
	}
	gst_tot = no_dec (gst_tot);

	PrintTotal ();
}

void
DrawCustomerInfo 
 (
	void
)
{
	/*Customer No.        %-6.6s           Name       %s*/
	print_at (3,2,ML (mlStdMess012), cumr_rec.dbt_no, local_rec.dl_name);

	/*Customer Order No.  %-15.15s  Delivery - %-40.40s*/
	print_at (5,2,ML (mlOlMess034), cohr_rec.cus_ord_ref, cohr_rec.dl_add1);

	/*Order Ref           %-15.15s*/
	print_at (6,2,ML (mlOlMess035), cohr_rec.ord_ref);

	print_at (6,50,"%-40.40s", cohr_rec.dl_add2);
	print_at (7,50,"%-40.40s", cohr_rec.dl_add3);
	fflush (stdout);
}

void
DrawTotal
 (
	void
)
{
	box (97,1,34,5);
	/*Net     Total :*/
	print_at (2,98,ML (mlOlMess036), (PROMPT));

	/*Freight Total :*/
	print_at (3,98,ML (mlOlMess037));

	/*Total :*/
	print_at (4,98,ML ("GST :"));

	move (98,5); line (33);

	/*TOTAL         :*/
	print_at (6,98,ML (mlOlMess039));

	PrintTotal ();
}

void
PrintTotal
 (
	void
)
{
	double tot_tot = 0.00, freight_gst;
	
	freight_gst = cohr_rec.freight *
			 (double) (comm_rec.gst_rate / 100.00);

	tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot +
					 cohr_rec.freight + freight_gst);
	if (CASH_INVOICE)
	{
		gst_tot += ADJ_VAL (tot_tot);
		tot_tot += ADJ_VAL (tot_tot);
	}
	print_at (2,115,"%14.2f",DOLLARS (inv_tot - dis_tot));

	print_at (3,115,"%14.2f",DOLLARS (cohr_rec.freight));

	print_at (4,115,"%14.2f",DOLLARS (gst_tot + tax_tot + freight_gst));

	print_at (6,115,"%14.2f",DOLLARS (tot_tot));
}

/*==============================================================
| Routine to add item at warehouse level if not already there. |
==============================================================*/
int
AddSerialNo 
 (
	long	wh_hash,
	long	ser_hash,
	char	*ser_no,
	long	din
)
{
	double	cost;

	cl_line ();

	/*Input Cost Price : */
	print_at (2,1,ML (mlOlMess033));

	cost = getnum (20,2,MONEYTYPE, "NNNNNNN.NN");
	Busy (0);
	
	insfRec.hhwh_hash = wh_hash;
	insfRec.hhbr_hash = ser_hash;
	strcpy (insfRec.status,"C");
	strcpy (insfRec.receipted,"Y");
	strcpy (insfRec.serial_no,ser_no);
	insfRec.date_in = din;
	insfRec.est_cost = cost;
	strcpy (insfRec.stat_flag,"E");

	cc = abc_add (insf,&insfRec);
	if (cc) 
		file_err (cc, insf, "DBADD");

	return (EXIT_SUCCESS);
}

void
SrchCumr 
 (
	char *key_val
)
{
	work_open ();
	save_rec ("#Cust Number", "#Customer Name  ");

	abc_selfield (cumr, "cumr_id_no");
	strcpy  (cumr_rec.co_no,  comm_rec.co_no);
	strcpy  (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%6.6s", key_val);

	for (cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
		 !cc && 
		  !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
		  !strcmp (cumr_rec.est_no, branchNo);
		 cc = find_rec (cumr, &cumr_rec, NEXT, "r"))
	{
		if (!strncmp (cumr_rec.dbt_no, key_val, strlen (key_val)) &&
			strcmp (cumr_rec.cash_credit, " "))
		{
			if (!strcmp (cumr_rec.cash_credit, "C"))
			{
		  		if (CheckEsmr ())
				{
					cc = save_rec (cumr_rec.dbt_no, cumr_rec.dbt_name);
					if (cc)
						break;
				}
			}
			else
			{
				cc = save_rec (cumr_rec.dbt_no, cumr_rec.dbt_name);
				if (cc)
					break;
			}
		}
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&cumr_rec, 0, sizeof (cumr_rec));   /* flush cumr_rec */
		abc_selfield (cumr, "cumr_id_no3");
		return;
	}

	strcpy  (cumr_rec.co_no, comm_rec.co_no);
	strcpy  (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%-6.6s", temp_str);
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	abc_selfield (cumr, "cumr_id_no3");
	return;
}

int 
CheckEsmr 
 (
	void
)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);  
	strcpy (esmr_rec.est_no, comm_rec.est_no);  
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (!cc && CASH_INVOICE)
		return (TRUE);
	else
		return (FALSE);
}

int
heading
 (
	int scn
)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

		
	pr_box_lines (scn);

	if (OL_INPUT)
	{
		switch (type_flag [0])
		{
		case	'O':
			/*Online Invoicing. */
			rv_pr (ML (mlOlMess010),54,0,1);
			break;

		case	'R':
			/*Credit Note Input. */
			rv_pr (ML (mlOlMess011),54,0,1);
			break;

		default:
			break;
		}
	}
	else
	if (OL_MAINT)
	{
		switch (type_flag [0])
		{
		case	'O':
			/*Manual Online Invoicing. */
			rv_pr (ML (mlOlMess012),51,0,1);
			break;

		case	'R':
			/*Manual Credit Note Input.*/
			rv_pr (ML (mlOlMess013),51,0,1);
			break;

		default:
			break;
		}
	}
	else
	{
		switch (type_flag [0])
		{
		case	'O':
			/*Invoice Display. */
			rv_pr (ML (mlOlMess014),54,0,1);
			break;

		case	'R':
			/*Credit Note Display. */
			rv_pr (ML (mlOlMess015),54,0,1);
			break;

		default:
			break;
		}
	}

	/*Last %s: %6s/%s */
	print_at (0,90,ML (mlOlMess041), PROMPT,  local_rec.prev_dbt_no, 
											local_rec.prev_inv_no);
	switch (scn)
	{
	case	1:
		use_window (0);
		break;

	case	2:
		DrawCustomerInfo ();
		DrawTotal ();
		CalcInvoiceTotal ();
		break;

	case	3:
		DrawCustomerInfo ();
		scn_write (SCN_LINES);
		scn_display (SCN_LINES);
		DrawTotal ();
		scn_set (scn);
		break;

	case	4:
		scn_write (SCN_LINES);
		scn_display (SCN_LINES);
		scn_set (scn);
		break;

	default :
		break;
	}

	/*  reset this variable for new screen NOT page	*/
	PrintCoStuff ();
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

static	char *
GenInvoiceNo 
 (
	int		conf_flag,
	long		incr
)
{
	static	char	tmp_invno 	 [9],	
					tmp_prefix 	 [3], 
					tmp_mask 	 [12];
	long	inv_no = 0L;
	int		len;

	len = 8;

	if (envVar.SalesOrderNumber	==	BY_DEPART)
	{
		if (CASH_INVOICE)
			strcpy (tmp_prefix, cudp_rec.csh_pref);
		else
			strcpy (tmp_prefix, cudp_rec.chg_pref);
	}
	else
	{
		if (CASH_INVOICE)
			strcpy (tmp_prefix, esmr_rec.csh_pref);
		else
			strcpy (tmp_prefix, esmr_rec.chg_pref);
	}

	clip (tmp_prefix);
	len = strlen (tmp_prefix);
	sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
	sprintf (tmp_invno, tmp_mask, tmp_prefix, inv_no);

	if (!conf_flag)
	{
		if (envVar.SalesOrderNumber	==	BY_DEPART)
		{
			cudp_rec.nx_sav_no += incr;
			inv_no = cudp_rec.nx_sav_no;
		}
		else
		{
			esmr_rec.nx_sav_inv += incr;
			inv_no = esmr_rec.nx_sav_inv;
		}
		tmp_prefix [0] = '\0';
	}
	else if (ONLINE_INVOICE)
	{
		if (CASH_INVOICE)
		{
			if (envVar.SalesOrderNumber	==	BY_DEPART)
			{
				cudp_rec.nx_csh_no += incr;
				inv_no = cudp_rec.nx_csh_no;
			}
			else
			{
				esmr_rec.nx_csh_inv += incr;
				inv_no = esmr_rec.nx_csh_inv;
			}
		}
		else
		{
			if (envVar.SalesOrderNumber	==	BY_DEPART)
			{
				cudp_rec.nx_chg_no += incr;
				inv_no = cudp_rec.nx_chg_no;
			}
			else
			{
				esmr_rec.nx_inv_no += incr;
				inv_no = esmr_rec.nx_inv_no;
			}
		}
	}
	else
	{
		if (CASH_INVOICE)
		{
			if (envVar.SalesOrderNumber	==	BY_DEPART)
			{
				cudp_rec.nx_crd_no += incr;
				inv_no = cudp_rec.nx_crd_no;
			}
			else
			{
				esmr_rec.nx_csh_crd += incr;
				inv_no = esmr_rec.nx_csh_crd;
			}
		}
		else
		{
			if (envVar.SalesOrderNumber	==	BY_DEPART)
			{
				cudp_rec.nx_crd_no += incr;
				inv_no = cudp_rec.nx_crd_no;
			}
			else
			{
				esmr_rec.nx_crd_nte_no += incr;
				inv_no = esmr_rec.nx_crd_nte_no;
			}
		}
	}

	clip (tmp_prefix);
	len = strlen (tmp_prefix);
	sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
	sprintf (tmp_invno, tmp_mask, tmp_prefix, inv_no);

	return (tmp_invno);
}

void
RecalcSchg
 (
	void
)
{
	int		append_flag = FALSE;

	if (PV_inpstatus == ENTRY || !Recalc_schg)
		return;

	if (line_cnt >= PV_schg_line)
	{
		lcount [SCN_LINES]++;
		append_flag = TRUE;
	}

	putval (line_cnt);

	DeleteSurcharge ();
	CalcSurcharge ();

	if (append_flag)
		lcount [SCN_LINES]--;

	getval (line_cnt);
}

void
CalcSurcharge 
 (
	void
)
{
	float	schg_qtys [MAX_SCHG];
	int		s_cnt, i, tmp_line, this_page;
	char	schg_code [17];

	if (ONLINE_CREDIT)
		return ;

	this_page = line_cnt / TABLINES;
	i = (CASH_INVOICE) ? 0 : atoi (cohr_rec.pri_type) - 1;

	abc_selfield (inmr,"inmr_id_no");

	for (s_cnt = 0; s_cnt < MAX_SCHG; s_cnt++)
		schg_qtys [s_cnt] = 0.0;

	tmp_line = line_cnt;
	for (line_cnt = 0;line_cnt < lcount [SCN_LINES];line_cnt++)
	{
		if (SU ('1'))
			schg_qtys [0] += SR._qty_sup;

		else if (SU ('2'))
			schg_qtys [1] += SR._qty_sup;

		else if (SU ('3'))
			schg_qtys [2] += SR._qty_sup;

		else if (SU ('4'))
			schg_qtys [3] += SR._qty_sup;
	}

	memcpy  ((char *) &inmr2_rec, (char *) &inmr_rec, sizeof (inmr_rec));

	for (s_cnt = 0; s_cnt < MAX_SCHG; s_cnt++)
	{
		if (schg_qtys [s_cnt])
		{
			memset ((char *) &SR, '\0', sizeof (struct storeRec));

			sprintf (schg_code, "SU%-1.1d", s_cnt + 1);
			strcpy (inmr_rec.co_no,comm_rec.co_no);
			sprintf (inmr_rec.item_no,"%-16.16s", schg_code);
			strcpy (local_rec.item_no,inmr_rec.item_no);

			if (ValidateItemNumber (FALSE))
				break;

			local_rec.qty_sup	=	schg_qtys [ s_cnt ];

			for (i = label ( "qty_sup") ; i <= label ( "extend" ) ; i++ )
			{
				spec_valid (i);
				i += skip_entry;
			}
	
			putval (line_cnt);

			if (this_page != (line_cnt / TABLINES))
			{
				scn_write (cur_screen);
				lcount [SCN_LINES] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			lcount [SCN_LINES] = line_cnt;
			
			line_display ();

			lcount [SCN_LINES]++;
			line_cnt++;
		}
	}
	memcpy ((char *) &inmr_rec, (char *) &inmr2_rec, sizeof (inmr_rec));

	line_cnt = tmp_line;
	CalcInvoiceTotal ();
}

void
DeleteSurcharge 
 (
	void
)
{
	register	int		i;

	for (i = 0; i < lcount [SCN_LINES]; i++)
		while (store [i]._schg_flag [0] == 'S' && i < lcount [SCN_LINES])
			DelSline (i);
}

void
DelSline 
 (
	int		line_no
)
{
	int		this_page, tmp_line;

	tmp_line = line_cnt;
	this_page = line_cnt / TABLINES;

	for (line_cnt = line_no ;line_cnt < lcount [SCN_LINES]; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		memcpy  ((char *) &SR, (char *) &store [line_cnt + 1],
						sizeof (struct storeRec));
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	sprintf (local_rec.item_no,"%-16.16s"," ");
	sprintf (coln_rec.item_desc,"%-40.40s"," ");
	sprintf (local_rec.serial_no,"%-25.25s"," ");
	local_rec.qty_ord = local_rec.qty_sup = 0;
	coln_rec.sale_price = 0.00;
	coln_rec.disc_pc = coln_rec.tax_pc = 0.00;
	coln_rec.due_date = 0L;
	memset ((char *) &SR, '\0', sizeof (struct storeRec));
	putval (line_cnt);
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = tmp_line;
	lcount [SCN_LINES]--;
}

void
SetSchgCalc (
	void)
{
	if (SR._schg_flag [0] <= '0' || SR._schg_flag [0] == 'S')
		Recalc_schg = FALSE;
	else
		Recalc_schg = TRUE;
}

void
ProcessScreen
 (
	int		scr_no, 
	int		dsp_flg
)
{
	heading (scr_no);
	if (dsp_flg)
		scn_display (scr_no);
	entry (scr_no);
}

/*==========================
| Validate margin percent. |
==========================*/
int
CheckMarginOk
 (
	double	sales,
	float	disc,
	double	csale,
	float	min_marg
)
{
	float	marg = 0.00;

	if (BONUS || SR._cont_status)
		return (EXIT_FAILURE);

	if (min_marg == 0.00)
		return (EXIT_FAILURE);

	sales /= 100;

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
		int		i;
		move (0,2);

		if (MARG_MESS2)
			i = prmptmsg (ML (mlOlMess075), "YyNn", 0, 2);
		else
		{
			sprintf (err_str, ML (mlOlMess076), min_marg, marg);
			i = prmptmsg (err_str, "YyNn", 0, 2);
		}
		move (0,2);
		printf ("%-94.94s"," ");

		if (i == 'y' || i == 'Y')
			return (EXIT_FAILURE);
		else
			return (EXIT_SUCCESS);
	}
	return (EXIT_FAILURE);
}
/*======================================================================
| Routine to check if bonus flag has been set (this is indicated by a  |
| '/B' on the end of the part number. If bonus flag is set then '/B'   |
| is removed from part number.                                         |
| Returns 0 if bonus flag has not been set, 1 if it has.               |
======================================================================*/
int
CheckBonus
 (
	char	*item_no
)
{
	char	bonus_item [17];
	char	*sptr;

	sprintf (bonus_item,"%-16.16s",item_no);
	sptr = clip (bonus_item);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == envVar.SoSpecial [0]  && * (sptr + 1) == envVar.SoSpecial [1])
		{
			*sptr = '\0';
			sprintf (item_no,"%-16.16s",bonus_item);
			return (ErrorReturn);
		}
	}
	sprintf (item_no,"%-16.16s",bonus_item);
	return (EXIT_SUCCESS);
}

/*===========================================
| Add record to background processing file. |
===========================================*/
void
AddSobg 
 (
	int		_lpno,
	char	*_type_flag,
	long	_hash
)
{
	open_rec (sobg,sobg_list,SOBG_NO_FIELDS,"sobg_id_no");
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _type_flag);
	sobg_rec.lpno = _lpno;
	sobg_rec.hash = _hash;
	sobg_rec.pid = progPid;

	cc = find_rec (sobg,&sobg_rec,COMPARISON,"r");
	/*--------------------------------------------------------------
	| Add the record if an identical one doesn't already exist     |
	--------------------------------------------------------------*/
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _type_flag);
		sobg_rec.lpno = _lpno;
		sobg_rec.hash = _hash;
		sobg_rec.pid = progPid;

		cc = abc_add (sobg,&sobg_rec);
		if (cc)
			file_err (cc, "sobg", "DBADD");

	}
	abc_fclose (sobg);
}

int
CheckSerialQuantity
 (
	float	*ser_qty
)
{
	/*----------------------------------------------------
	| Validate to see if on hand is less than input qty. |
	----------------------------------------------------*/

	if (ONLINE_INVOICE && win_ok &&
		 ((SR._qty_avail - *ser_qty) < 0.00) &&
			!NO_COST && !NON_STOCK (line_cnt))
	{
		sprintf (err_str, ML (mlOlMess027), 	SR._qty_avail, 
											clip (local_rec.item_no),
											*ser_qty,
											BELL);

		Busy (1);
		cc = WarnUser (err_str,1,2);

		InptRes (ser_qty);

		return (skip_entry);
	}
	else
		return (EXIT_SUCCESS);
}
			
/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
void
ProcessKitItem 
 (
	long		hhbr_hash,
	float		qty
)
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
		cc = find_hash (inmr, &inmr_rec,COMPARISON,"r",
							sokt_rec.mabr_hash);
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.bonus [0] == 'Y')
		{
			sprintf (so_bonus, "%-2.2s", envVar.SoSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.item_no,"%-s%-.*s",
				sptr, 16 - (int) strlen (sptr), so_bonus);
		}
		else
			strcpy (local_rec.item_no,inmr_rec.item_no);

		dflt_used = FALSE;

		if (ValidateItemNumber (FALSE))
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (SR._hhwh_hash < 0L)
		{
			sprintf (local_rec.serial_no, "%25.25s", " ");
			strcpy (SR._serial, local_rec.serial_no);
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
			get_entry (label ( "qty_ord"));
		
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
			cc = find_hash (inmr, &inmr_rec, EQUAL, "r", sokt_rec.mabr_hash);


			/*--------------------------
			| Check Quantity available |
			--------------------------*/
			CheckSerialQuantity (&lcl_qty);

			for (count = 0; count < lcl_qty; count++)
			{

				if (sokt_rec.bonus [0] == 'Y')
				{
					sprintf (so_bonus, "%-2.2s", envVar.SoSpecial);
					sptr = clip (inmr_rec.item_no);
					sprintf (local_rec.item_no, "%-s%-.*s",
						     sptr, 16 - (int) strlen (sptr), so_bonus);
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
					lcount [SCN_LINES] = line_cnt;
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
						if (cc && ! (SERIAL && i == label ("ser_no")))
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
					SR._qty_sup = local_rec.qty_sup;
	
					if (this_page != (line_cnt / TABLINES))
					{
						scn_write (cur_screen);
						lcount [SCN_LINES] = line_cnt;
						this_page = line_cnt / TABLINES;
					}
					lcount [SCN_LINES] = line_cnt;
					
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
				lcount [SCN_LINES] = line_cnt;
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
					if (cc && ! (SERIAL && i == label ("ser_no")))
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
				SR._qty_sup = local_rec.qty_sup;

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					lcount [SCN_LINES] = line_cnt;
					this_page = line_cnt / TABLINES;
				}
				lcount [SCN_LINES] = line_cnt;
			
				line_display ();
				line_cnt++;
			}
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [SCN_LINES] = line_cnt;
	abc_fclose (sokt);
	cohr_rec.date_required = hold_date;
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
float
ProcessPhantom
 (
	long	hhbr_hash
)
{
	int		first_time = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;

	open_rec ("sokt", sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");

	cc = find_hash ("sokt", &sokt_rec, GTEQ, "r", hhbr_hash);
	while (!cc && sokt_rec.hhbr_hash == hhbr_hash)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_hash ("sokt",&sokt_rec,NEXT,"r", hhbr_hash);
			continue;
		}
	
		if (envVar.SoFwdAvl)
		{
			on_hand = incc_rec.closing_stock -
					  incc_rec.committed -
					  incc_rec.backorder - 
		   	          incc_rec.forward;
		}
		else
		{
			on_hand = incc_rec.closing_stock -
					  incc_rec.committed -
					  incc_rec.backorder;
		}
		if (envVar.QcApply && envVar.SkQcAvl)
			on_hand -= incc_rec.qc_qty;

		on_hand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_hash ("sokt",&sokt_rec,NEXT,"r", hhbr_hash);
	}
	abc_fclose ("sokt");

	return (min_qty);
}

/*===============================================
| use_window is a procedure called by scrgen	|
| when FN14 or FN15 is pressed.					|
| _key is normally the same as last_char		|
| but by passing it as a parameter it allows	|
| the programmer to do some sneaky things		|
===============================================*/
int
use_window
 (
 int		_key
)
{
	static	long	lastHhcu;
	char	comment [132];

	if (cur_screen == SCN_HEAD)
	{
		move (21,3); line ( 90 );
	}
	/*-----------------------------------------------
	| Only do anything when we are on screen 1 and	|
	| we've read a valid cumr.						|
	-----------------------------------------------*/
	if (cumr_rec.hhcu_hash == 0L)
	{
		lastHhcu = 0L;
		return 0;
	}
	if (cur_screen != SCN_HEAD)
	{
		lastHhcu = 0L;
		return 0;
	}

	if (FindCucc (_key,lastHhcu))
		return 0;

	if (lastHhcu != cumr_rec.hhcu_hash)
		lastHhcu = cumr_rec.hhcu_hash;

	crsr_off ();
	sprintf (comment, " [ %-80.80s ] ", cucc_rec.comment);
	us_pr (comment, 22, 3, 1);
	crsr_on ();
	return 0;
}

void
AltClear
 (
	int		len
)
{
	print_at (0,0,"%-*.*s",len, len, " "); fflush (stdout);
}

int
FindCucc
 (
	int		_key,
	long	lastHhcu
)
{
	if (_key == 0)
	{
		cc = find_rec (cucc,&cucc_rec,COMPARISON,"r");
		if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			return (EXIT_SUCCESS);

		return (ErrorReturn);
	}

	if (lastHhcu != 0L)
	{
		/*-------------------------------------------------------
		| Find the NEXT / PREVIOUS record to the current one	|
		-------------------------------------------------------*/
		cc = find_rec (cucc,&cucc_rec, (_key == FN14) ? NEXT : PREVIOUS,"r");

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
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = TodaysDate () + 30000L;

		cc = find_rec (cucc,&cucc_rec, LTEQ, "r");

		/*-------------------------------------------
		| Probably the last hhcu group in the cucc	|
		| so find the last record in the file.		|
		-------------------------------------------*/
	}

	if (cc || cucc_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (ErrorReturn);
	else
		return (EXIT_SUCCESS);
}
/*===============================
| Search on Contract (cnch)     |
===============================*/
void
SrchCnch
 (
	char    *key_val
)
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

void
tab_other
 (
	int	line_no
)
{
	/*-------------------------------------------------------------------------
	| turn off and on editing of fields depending on whether contract or not. |
	------------------------------------------------------------------------*/
	FLD ("disc") 		= inp_disc;
	FLD ("sale_price") 	= inp_sale;

	if (store [line_no]._cont_status)
	{
		FLD ("disc") = (store [line_no]._cont_status == 1) ? NA : inp_disc;
		FLD ("sale_price") 	= NA;
	}
}

/*============================================
| Routine to remove item at warehouse level. |
============================================*/
void
RemoveSerialNo 
 (
	long	hhwh_hash,
	long	hhbr_hash,
	char	*ser_no
)
{
	insfRec.hhwh_hash 			= hhwh_hash;
	strcpy (insfRec.status,		"C");
	strcpy (insfRec.serial_no,	ser_no);

	cc = find_rec (insf,&insfRec,EQUAL,"u");
	if (!cc)
	{
		cc = abc_delete (insf);
		if (cc) 
			file_err (cc, insf, "DBDELETE");
	}
}

void
InitStore 
 (
 int		line_no
)
{
	memset ((char *) &store [ line_no ], '\0', sizeof (struct storeRec));

	strcpy (store [ line_no ]._serial, 	ser_space);
	strcpy (store [ line_no ]._org_ser, 	ser_space);
	strcpy (store [ line_no ]._category,	"           ");
	strcpy (store [ line_no ]._sellgrp,	"      ");
	strcpy (store [ line_no ]._bonus,  	" ");
	strcpy (store [ line_no ]._class,	" ");
	strcpy (store [ line_no ]._dis_or,	"N");
	strcpy (store [ line_no ]._pri_or,	"N");
	strcpy (store [ line_no ]._pack_size,"     ");
	strcpy (store [ line_no ]._schg_flag," ");
	strcpy (store [ line_no ]._old_insf, "N");
	store [ line_no ]._kit_flag	=	K_NONE;
	store [ line_no ]._ExtendTotal	=	0.00;
	
}

/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc
	 (
		float	DiscountPercent
	)
{
	if (envVar.ReverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment 
 (
	void
)
{
	char	*sptr;

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVar.GstApplies	= 0;
	else
		envVar.GstApplies = (*sptr == 'Y' || *sptr == 'y');

	if (envVar.GstApplies)
		sprintf (envVar.TaxCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVar.TaxCode, "%-3.3s", "Tax");

	sprintf (envVar.CurrencyCode, "%-3.3s", get_env ("CURR_CODE"));

	sptr = chk_env ("SO_NUMBERS");
	envVar.SalesOrderNumber = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	/*-----------------
	| Multi Currency. |
	-----------------*/
	sptr = chk_env ("DB_MCURR");
	envVar.MultiCurrency = (sptr == ( char *)0) ? FALSE : atoi ( sptr );

	sptr = chk_env ("SO_DISC_REV");
	envVar.ReverseDiscount = (sptr == (char *)0) ? FALSE : atoi ( sptr );

	/*----------------------
	| Net or Gross values. |
	----------------------*/
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *) 0) ? TRUE : atoi (sptr);

	sprintf (envVar.AutoStockUpdate, "%-1.1s",get_env ("AUTO_SK_UP"));

	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVar.SoSpecial,"/B/H");
	else
		sprintf (envVar.SoSpecial,"%-4.4s", sptr);

	sptr = chk_env ("SO_MARGIN");
	if (sptr == (char *)0)
		sprintf (envVar.SoMargin, "%-2.2s", "00");
	else
		sprintf (envVar.SoMargin, "%-2.2s", sptr);

	/*--------------------------------------
	| Check for discounts on Indent items. |
	--------------------------------------*/
	sptr = chk_env ("SO_DIS_INDENT");
	envVar.SoDisIndent	= (sptr == (char *)0) ? TRUE : atoi ( sptr );

	sptr = chk_env ("OL_BELOW_SALE");
	envVar.OlBelowSale = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/* QC module is active or not. */
	envVar.QcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/* Whether to include QC qty in available stock. */
	envVar.SkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*-----------------------------
	| Check and Get Credit terms. |
	-----------------------------*/
	sptr = get_env ("SO_CRD_TERMS");
	envVar.DbStopcrd = (* (sptr + 0) == 'S');
	envVar.DbCrdterm = (* (sptr + 1) == 'S');
	envVar.DbCrdover = (* (sptr + 2) == 'S');

	sptr = chk_env ("SO_FWD_AVL");
	envVar.SoFwdAvl = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVar.WinOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-----------------------------------------------------------
	| Check if stock information window is loaded at load time. |
	-----------------------------------------------------------*/
	sptr = chk_env ("SO_PERM_WIN");
	envVar.SoPermWin = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SO_OTHER_1");
	sprintf (envVar.Other [0],"%.30s", (sptr == (char *)0) 
										? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (envVar.Other [1],"%.30s", (sptr == (char *)0) 
										? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (envVar.Other [2],"%.30s", (sptr == (char *)0) 
										? "Other Costs." : sptr);
	
	envVar.DbCoOwned = atoi (get_env ("DB_CO"));

	sptr = chk_env ("KIT_DISC");
	envVar.KitDiscount = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("RECALC_KIT");
	Recalc_kit = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	envVar.useSystemDate = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;
}

void
RunningKit 
 (
	int	curr_line
)
{
	int		tmp_line;

	if (IS_ENTRY || !Recalc_kit ||
	    (!LineInKit (curr_line) && store [curr_line]._kit_flag != K_END))
		return;

	tmp_line = line_cnt;
	putval (line_cnt);
	while (!KIT_END && line_cnt < lcount [SCN_LINES])
		line_cnt++;

	if (line_cnt < lcount [SCN_LINES])
	{
		getval (line_cnt);
		CalcKitLine ();
		coln_rec.sale_price = SR._ExtendTotal;
		if (SR._qty_sup)
			coln_rec.sale_price /= SR._qty_sup;

		coln_rec.sale_price = no_dec (coln_rec.sale_price);
		SR._sale_price = coln_rec.sale_price;
		SR._act_sale = coln_rec.sale_price;
		DSP_FLD ("sale_price");
		PrintExtend (line_cnt);
		putval (line_cnt);
	}
	line_cnt = tmp_line;
	getval (line_cnt);
}

int
LineInKit 
 (
	int	line_no
)
{
	register	int	i;
	int	in_kit;

	for (in_kit = FALSE,i = 0;i <= line_no;i++)
	{
		if (store [i]._kit_flag == K_START)
			in_kit = TRUE;

		if (store [i]._kit_flag == K_END)
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
	register	int	line_no;
	double	k_value = (double) 0;

	for (line_no = 0;line_no <= line_cnt;line_no++)
	{
		switch (store [line_no]._kit_flag)
		{
		case	K_START:
			k_value = 0.00;
			break;

		case	K_END:
			store [line_no]._ExtendTotal = k_value;
			break;

		default:
			k_value += store [line_no]._ExtendTotal;
			break;
		}
	}
}

int
SpecialItem 
 (
	int		pType
)
{
	double		findSpecialPrice	=	0.00;
	findSpecialPrice	=	PromCusPrice 
							 (
								comm_rec.co_no,
								comm_rec.est_no,
								comm_rec.cc_no,
								cumr_rec.curr_code,
								cumr_rec.hhcu_hash,
								cumr_rec.area_code,
								cumr_rec.class_type,
								SR._hhbr_hash,
								comm_rec.dbt_date,
								pType
							);

	if (findSpecialPrice != (double) -1.00)
		return (TRUE);

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
	if (SR._kit_flag != K_NONE)
	{
		SR._outer = 1.00;
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

		SR._kit_flag = k_status;
	}
	else
		CalLineExtended (line_cnt);

	if (print_it && this_page == line_cnt / TABLINES)
		line_display ();
}

int
CheckIncc 
 (
	void
)
{
	int createInccs = FALSE;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
					                  inmr_rec.hhsi_hash);
	if (find_rec (incc, &incc_rec, EQUAL, "r"))
	{
		int c;

		sprintf (err_str,
			     "\007Part %s is not on file at warehouse - create? ",
				 clip (inmr_rec.item_no));
		Busy (0);
		c = prmptmsg (err_str, "YyNn", 1, 2);
		createInccs = (c == 'y' || c == 'Y');
		if (!createInccs)
		{
			skip_entry = -1;
			return (EXIT_FAILURE); 
		}

		createInccs = TRUE;
		AddMissingWarehouse (inmr_rec);
	}

	if (inmr_rec.inmr_class [0] == 'P')
	{
		open_rec ("sokt", sokt_list, SOKT_NO_FIELDS, "sokt_id_no");

		strcpy (sokt_rec.co_no, comm_rec.co_no);
		sokt_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokt_rec.line_no = 0;
		for (cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");
			 !cc &&
			  !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
			  sokt_rec.hhbr_hash == inmr_rec.hhbr_hash;
			 cc = find_rec ("sokt", &sokt_rec, NEXT, "r"))
		{
			if (find_hash (inmr2, &inmr2_rec, EQUAL, "r", sokt_rec.mabr_hash))
				continue;

			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = alt_hash (inmr2_rec.hhbr_hash,
											  inmr2_rec.hhsi_hash);
			if (find_rec (incc, &incc_rec, EQUAL, "r"))
			{
				if (!createInccs)
				{
					int c;

					sprintf (err_str,
							 "\007Phantom kit component %s "
							 "is not on file at warehouse - create? ",
							 clip (inmr2_rec.item_no));
					Busy (0);
					c = prmptmsg (err_str, "YyNn", 1, 2);
					createInccs = (c == 'y' || c == 'Y');
				}
				if (!createInccs)
				{
					skip_entry = -1;
					abc_fclose ("sokt");
					return (EXIT_FAILURE); 
				}

				AddMissingWarehouse (inmr2_rec);
			}
		}
		abc_fclose ("sokt");
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Add Warhouse Record for Current . |
===================================*/
int
AddMissingWarehouse
 (
	struct inmrRecord inmr_rec
)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash
							 (
								inmr_rec.hhbr_hash,
					  			inmr_rec.hhsi_hash 
							);
	incc_rec.hhwh_hash = 0L;
	sprintf (incc_rec.sort,"%s%11.11s%-16.16s", 
										inmr_rec.inmr_class,
										inmr_rec.category,
										inmr_rec.item_no);
	incc_rec.first_stocked = TodaysDate ();
	incc_rec.closing_stock = 0.00;
	strcpy (incc_rec.stat_flag,"0");
	
	if ((cc = abc_add ("incc",&incc_rec)))
		return (ErrorReturn);

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = alt_hash
							 (
								inmr_rec.hhbr_hash,
								inmr_rec.hhsi_hash 
							);
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	return (cc);
}

/*=======================================================
| For every line in 2nd tabular screen					|
| go thru and set kit_flag for start or end of kit		|
| also set part descriptions for start & end of kit		|
| if print_it is true print the lines on the current	|
| screen.												|
=======================================================*/
void
CheckKit 
 (
	int		print_it
)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
	int	high_val = (!ins_flag && IS_ENTRY) ? line_cnt : lcount [2] - 1;

	if (IS_ENTRY)
		scn_page = line_cnt / TABLINES;

	for (i = line_cnt,line_cnt = 0;line_cnt <= high_val;line_cnt++)
	{
		getval (line_cnt);
		CalcInvoiceTotal ();
		ResetKit (this_page,print_it);
		putval (line_cnt);
	}
	line_cnt = i;
	getval (line_cnt);
}
/*=============================
| Invoice Print Open Routine. |
=============================*/
void
OlOpen
 (
    char    *envName,
    char    *progName,
    int     lpno)
{
    char    runPrint [81];

    /*-------------------------------------------
    | first time or Company or Branch Changed   |
    -------------------------------------------*/
    strcpy (runPrint,OlCheck (envName, progName, comm_rec.co_no,comm_rec.est_no));

    /*-------------------------------
    | Require to Open the Pipe  |
    -------------------------------*/
    if ((olPout = popen (runPrint,"w")) == 0)
    {
        sprintf (err_str,"Error in %s during (POPEN)",runPrint);
        sys_err (err_str,errno,PNAME);
    }
    fprintf (olPout,"%d\n",lpno);
    fprintf (olPout,"S\n");
    fflush (olPout);
}
   
/*==============================
| Invoice Print Close Routine. |
==============================*/
void
OlClose
 (
	void
)
{
    fprintf (olPout,"0\n");
    fflush (olPout);
    pclose (olPout);
}

/*========================
| Invoice Print Routine. |
========================*/
void
OlPrint
 (
	long ip_hash
)
{
    fprintf (olPout,"%ld\n", ip_hash);

    fflush (olPout);
}

/*============================
| Check for System Variable. |
============================*/
char    *OlCheck
(
	char *_env, 
	char *_prg, 
	char *_co_no, 
	char *_br_no
)
{
    char    *sptr;
    char    *chk_env (char *);
    char    runPrint [41];

    /*-------------------------------
    | Check Company & Branch    |
    -------------------------------*/
    sprintf (runPrint,"%s%s%s",_env,_co_no,_br_no);
    sptr = chk_env (runPrint);
    if (sptr == (char *)0)
    {
        /*---------------
        | Check Company |
        ---------------*/
        sprintf (runPrint,"%s%s",_env,_co_no);
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
        return (sptr);
}

/*===========================
| Search On Department      |
===========================*/
void
SrchCudp
 (
 char    *key_val
)
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
          && !strcmp (cudp_rec.br_no, comm_rec.est_no)
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
char*   CheckVariable (char*  _env, char*  _prg);

/*=======================================================
| check the existence of the environment variable	    |
| first with company & branch, then company, and	    |
| then by itself.					                    |
| if no vble found then return 'prg'			        |
=======================================================*/
char *
CheckVariable (
 char*  _env,
 char*  _prg)
{
	char	*sptr;
	char	runPrint[41];

	/*-------------------------------
	| Check Company & Branch	|
	-------------------------------*/
	sprintf (runPrint,"%s%s%s",_env, comm_rec.co_no, comm_rec.est_no);
	sptr = chk_env (runPrint);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf (runPrint,"%s%s",_env, comm_rec.co_no);
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
		file_err (cc, "exsi", "DBFIND");
}

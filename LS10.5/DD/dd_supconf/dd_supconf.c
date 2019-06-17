/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: dd_supconf.c,v 5.17 2002/11/28 04:09:46 scott Exp $
|  Program Name  : (dd_supconf.c)                                     |
|  Program Desc  : (Direct Delivery Supplier Confirmation.      )     |
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 13/06/94         |
|---------------------------------------------------------------------|
| $Log: dd_supconf.c,v $
| Revision 5.17  2002/11/28 04:09:46  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.16  2002/08/14 04:28:39  scott
| Updated for Linux error
|
| Revision 5.15  2002/07/24 08:38:50  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.14  2002/07/18 06:31:55  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.13  2002/07/17 09:57:10  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.12  2002/06/26 04:50:30  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.11  2001/11/19 03:22:36  scott
| Updated for printing.
|
| Revision 5.10  2001/11/08 06:54:39  cha
| Updated to make sure that reports are printed in GUI.
|
| Revision 5.9  2001/10/24 10:12:32  cha
| Updated to correct rounding in the
| discount level.
|
| Revision 5.8  2001/10/24 09:28:15  cha
| Updated to round discount correctly.
|
| Revision 5.7  2001/10/23 08:40:49  cha
| Updated to ensure that rounding is
| consistent with other modules.
|
| Revision 5.6  2001/10/22 03:43:20  cha
| Updated to make sure that grn_no is
| genereated correctly.
|
| Revision 5.5  2001/10/17 01:47:20  cha
| Updated to fix some minor errors.
|
| Revision 5.4  2001/10/17 01:41:11  cha
| Updated to fix incorrect prompt_at. Changes made by Scott.
|
| Revision 5.3  2001/08/28 08:45:58  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 08:55:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:24:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/27 06:35:18  scott
| Updated to change arguments passed to DbBalWin to avoid usage of read_comm ()
|
| Revision 4.1  2001/03/27 03:51:41  scott
| Updated to remove program shell of db_balwin and replaced with function DbBalWin
|
| Revision 4.0  2001/03/09 02:26:17  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.4  2001/03/08 02:12:01  scott
| Updated to increase the delivery address number from 0-999 to 0-32000
| This change did not require a change to the schema
| As a general practice all programs have had app.schema added and been cleaned
|
| Revision 3.3  2001/03/06 09:25:47  scott
| Updated to allow -ve discounts to be processed.
| Problem with this was test on inmr_disc_pc should have been applied
| only when inmr_disc_pc had a value.
|
| Revision 3.2  2000/12/21 02:38:10  ramon
| Updated to change LoadEnvironment() to IntLoadEnvironment() as there's already an existing function like this in the LS10-GUI library.
|
| Revision 3.1  2000/11/20 07:39:01  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:14:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/09/07 02:30:53  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.1  2000/09/06 07:49:17  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 08:53:47  gerry
| Forced Revsion No. Start 2.0 Rel-15072000
|
| Revision 1.45  2000/07/13 23:24:26  scott
| Updated to use environment SO_DOI for pricing.
| Environment is used in most so programs and as such all should be consistant.
|
| Revision 1.44  2000/06/26 03:39:49  scott
| Updated to remove unused fields.
|
| Revision 1.43  2000/06/13 05:01:59  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.42  2000/06/01 08:25:58  ana
| SC2754 - 16216 Corrected conversion of qty based on order lot size (std uom) - USL.
|
| Revision 1.41  2000/05/18 03:36:58  nz
| 	Remove the function set_timer ()
|
| Revision 1.40  2000/05/10 07:07:26  nz
| USL16300 It executes/calls po_poprint using a 'pipe' rather then
|          calling the program with parameters.
|
| Revision 1.39  2000/02/18 10:57:12  ramon
| Changed clip () to rtrim () for constant string.
|
| Revision 1.38  2000/01/31 09:48:52  ramon
| For GVision compatibility, I moved the description field 3 chars. to the right. I also changed the data fields from TAB to LIN type in Shipment screen.
|
| Revision 1.37  1999/12/06 01:46:56  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.36  1999/11/17 06:39:58  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.35  1999/11/15 23:47:04  scott
| Updated for incorrect GVISION comment.
|
| Revision 1.34  1999/11/11 00:00:40  cam
| Added conditional compile code for GVision re ViewDiscounts
|
| Revision 1.33  1999/11/04 08:03:55  scott
| Updated due to use of -Wall flag on compiler.
|
| Revision 1.32  1999/10/01 07:48:32  scott
| Updated for standard function calls.
|
| Revision 1.31  1999/09/29 10:10:36  scott
| Updated to be consistant on function names.
|
| Revision 1.30  1999/09/29 01:56:06  scott
| Updated form getkey ();
|
| Revision 1.29  1999/09/17 07:26:23  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.28  1999/09/16 02:21:05  scott
| Updated from Ansi Project
|
| Revision 1.27  1999/07/15 10:24:53  scott
| Updated for abc_delete
|
| Revision 1.26  1999/06/14 23:47:01  scott
| Updated to add log file
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dd_supconf.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_supconf/dd_supconf.c,v 5.17 2002/11/28 04:09:46 scott Exp $";

#define	USE_WIN		1
#define MAXSCNS 	6
#define MAXLINES	500
#define TABLINES	10

#define	SR			store [line_cnt]
#define	SI			store [local_rec.storeIdx]
#define	MARG_MESS1	 (envVarSoMargin [0] == '0')
#define	MARG_MESS2	 (envVarSoMargin [0] == '1')
#define	DBOX_TOP	9
#define	DBOX_LFT	35
#define	DBOX_WID	66
#define	DBOX_DEP	3

#define	HDRSCN		1
#define	ORDSCN		2
#define	CSTSCN		3
#define	PRISCN		4
#define	TLRSCN		5
#define	SHPSCN		6


#define	PRIORITY	0
#define	PRICE		1
#define	FGN_CURR	 (envVarDbMcurr && strcmp (cumr_rec.curr_code, envVarCurrCode))

#define	PENDINGFLAG		"P"
#define	ACTIVEFLAG		"A"
#define	CONFIRMFLAG		"C"
#define	DESPATCHFLAG	"D"
#define	INVOICEFLAG		"I"
#define	DELETEFLAG		"X"

#define 	SHIP_NULL 	 ((struct SHIP_PTR *) NULL)

/*=============================
| On-Cost Category Definitions |
==============================*/
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

#define 	DDGD_NULL 	 ((struct DDGD_PTR *) NULL)

#include <pslscr.h>
#include <twodec.h>
#include <hot_keys.h>
#include <minimenu.h>
#include <getnum.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_dd_mess.h>
#include <inis_update.h>
#include <proc_sobg.h>

	/*--------------------------------------------
	| Structure used for pop-up discount screen. |
	--------------------------------------------*/
	static struct
	{
		char	fldPrompt [14];
		int		xPos;
		char	fldMask [12];
	} discScn [] = 
	{
		{"FOB (FGN)", 		0,  "NNNNNNNN.NN"},
		{"Reg %", 			14, "NNN.NN"},
		{"Disc A", 			23, "NNN.NN"},
		{"Disc B", 			32, "NNN.NN"},
		{"Disc C", 			41, "NNN.NN"},
		{"NET FOB (FGN)", 	50, "NNNNNNNN.NN"},
		{"", 			0,  ""},
	};
    
	int		notax,				/* charge gst & tax		*/
			new_order,		/* creating new invoice		*/
			ins_flag,			/* inserting line item		*/
			np_fn,
			prmpt_type;

	float	vol_tot = 0.00,		/* Volume total for spread*/
	     	wgt_tot = 0.00;		/*  total for spread*/

	int		envVarSoDiscRev 	= FALSE,
			envVarSoDoi			= FALSE,
			envVarWinOk			= FALSE,
			envVarSoPermWin		= FALSE,
			envVarDbFind 		= 0,	
			envVarCrFind 		= 0,
			envVarDdCrDefault 	= 0,	
			envVarFeInstall		= 0,
			envVarDbMcurr		= 0,
			db_stopcrd 			= 1,
			db_crdterm 			= 1,
			db_crdover 			= 1,
			l_lines				= 0;

	double	l_tax = 0.00,		/* line item tax		*/
			l_gst = 0.00,		/* line item gst		*/
			l_dis = 0.00,		/* line item discount	*/
			l_total, 
			t_total,
			ord_total,
			c_left,
			fob_tot = 0.00,		/* total cost of goods	*/
			fai_tot = 0.00,		/* total cost of goods	*/
			dty_tot = 0.00,		/* total cost of goods	*/
			oth_tot = 0.00,		/* total cost of goods	*/
			inv_tot = 0.00,		/* invoiced total		*/
			tax_tot = 0.00,		/* tax total			*/
			tot_tot = 0.00,		/* Total all.			*/
			cst_tot = 0.00,		/* Cost total for spread*/
			fother = 0.00,		/* other total			*/
			temp_gross 	= 0.00, /* temporary gross		*/
			temp_dis 	= 0.00; /* temporary dsicount	*/

	char	cbranchNumber [3],			/* branch number		*/
			dbranchNumber [3],			/* For Customer		*/
			envVarSoMargin [3],
			*envVarCurrentUser,
			envVarCurrCode [4];

#include	"schema"

struct ddhrRecord	ddhr_rec;
struct ddhrRecord	ddhr2_rec;
struct ddlnRecord	ddln_rec;
struct ddlnRecord	ddln2_rec;
struct ddshRecord	ddsh_rec;
struct ddshRecord	ddsh2_rec;
struct ddgdRecord	ddgd_rec;
struct sudsRecord	suds_rec;
struct inisRecord	inis_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct poghRecord	pogh_rec;
struct poghRecord	pogh2_rec;
struct poglRecord	pogl_rec;
struct pohsRecord	pohs_rec;
struct suphRecord	suph_rec;
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
struct felnRecord	feln_rec;

	Money	*cumr_bo	=	&cumr_rec.bo_current;
	int		*cumr_inst	=	&cumr_rec.inst_fg1;

	char	*cumr2 = "cumr2",
			*ddhr2 = "ddhr2",
			*ddln2 = "ddln2",
			*pohr2 = "pohr2",
			*poln2 = "poln2",
			*data = "data",
			*sixteen_space = "                ",
			*twenty_spaces = "                    ",
			*forty_spaces  = "                                        ";

#include <FindCumr.h> /* requires declaration */
#include <FindSumr.h> /* requires comm_rec declaration */    

	struct	storeRec {
									/*==================================*/
									/* ddln related data.         		*/
									/*==================================*/
		int  	line_num;			/* ddln/store line_num         		*/
		long 	hhbr_hash;			/* inmr_hhbr_hash		            */
		long 	hhum_hash;			/* inmr_std_uom. 		            */
		long 	hhpl_hash;			/* poln_hhpl_hash					*/
		long 	hhdl_hash;			/* ddln_hhdl_hash					*/
		long 	hhsu_hash;			/* sumr_hhsu_hash					*/
		long 	hhds_hash;			/* ddsh_hhds_hash					*/
		long 	hhcc_hash;			/* incc_hhcc_hash					*/
		float   qty;				/* local_rec.qty					*/
		float	dis_pc;				/* ddln_disc_pc						*/
		float	gst_pc;				/* inmr_gst_pc or 0.00 if notax		*/
		float	tax_pc;				/* inmr_gst_pc or 0.00 if notax		*/
		double	gsale_price;		/* ddln_gsale_price					*/
		long	due_date;			/*                                  */
									/*==================================*/
									/* inmr related data.         		*/
									/*==================================*/
		char	std_uom [5];			/* Standard (Stock) UOM.			*/
		char	sellgrp [7];		/* item selling group.				*/
		char	item_no [17];		/* item number     					*/
		char	item_desc [41];		/* item description					*/
		double	tax_amt;			/* inmr_gst_amt " 0.00 if notax		*/
		float	outer;				/* inmr_outer_size					*/
		char	category [12];		/* item category for line			*/
		char	item_class [2];		/* item's class for line			*/
		char	cost_flag [2];		/* inmr_costing_flag				*/
		char	duty_code [3];		/* inmr_duty_code/inis_duty_code	*/
		char	duty_type [2];		/* duty type from podt				*/
		double	imp_duty;			/* duty rate from podt				*/
		char	ship_no [3];			/* Shipment Number for line.		*/
									/*==================================*/
									/* excf related data.         		*/
									/*==================================*/
		float	min_marg;			/* Min margin for category.     	*/
									/*==================================*/
									/* inis related data.         		*/
									/*==================================*/
		float	weight;				/* inis_weight						*/
		float	volume;				/* inis_volume						*/
		char	supp_uom [5];		/* Supplier UOM.					*/
		float	supp_conv;			/* Supplier UOM Conversion Factor	*/
		float	supp_lead;			/* Supplier Lead time.           	*/
		double	supp_exch;			/* Supplier Exchange Rate.          */
		int		no_inis;			/* No Inventory supplier record inis*/
		int		upd_inis;			/* Update inventory supplier record.*/
		float	min_order;			/* stuff copied from inis_rec		*/
		float	ord_multiple;   	/* Relates to Min and order multiple*/
		double	base_cst;			/* Base cost from inis or inei 		*/
		double	grs_fgn;			/* Gross foreign cost (init. base) */
		double	land_cst;			/* Cost price.						*/
									/*==================================*/
									/* Priceing Calc Fields 	    	*/
									/*==================================*/
		double	extend;				/* default sale price 				*/
		int		pricing_chk;		/* Pricing check done				*/
		int		cumulative;			/* Discounts are cumulative ?  		*/
		char	dis_or [2];			/* Discount override. 				*/
		float	uplift;				/* Uplift							*/
		float	dflt_uplift;		/* Standard Uplift					*/
		float	dflt_disc;			/* inmr_disc_pc						*/
		float	calc_disc;			/*                       			*/
		float	purchDiscs [4];		/* Purchasing discounts				*/
		float	reg_pc;				/* Regulatory percent.      		*/
		float	disc_a;				/* Discount percent A.      		*/
		float	disc_b;				/* Discount percent A.      		*/
		float	disc_c;				/* Discount percent A.      		*/
		double	sale_price;			/* ddln_sale_price					*/
		double	calc_sprice;		/*                 					*/
		double	act_sale;			/*                 					*/
		double	dflt_price;			/* Standard Price.					*/
		double	net_fob;			/* Net FOB after discounts 			*/
		double	amt_fai;			/* value of freight + ins.			*/
		double	amt_dty;			/* value of duty          			*/
		double	amt_oth;			/* value of other costs.  			*/
		int		cont_status;		/* Contract Status 1, 2, 3			*/
		int		con_price;			/* TRUE if Price on contract		*/
		int		cont_cost;			/* TRUE if cost on contract			*/
		int		keyed;				/* 0 = nothing changed				*/
									/* 1 = uplift changed				*/
									/* 2 = sale price changed   		*/
									/*==================================*/
	} store [ MAXLINES ];

	/*    STORE    */

	struct tag_s_terms
    {
        char	*_scode;
		char	*_sterm;
	} s_terms [] = {
					{"   ",""},
					{"CIF",""},
					{"C&F",""},
					{"FIS",""},
					{"FOB",""},
					{"",""}
                  };

    char* s_terms_const [] = {"Local                 ",
                              "Cost Insurance Freight",
                              "Cost & Freight",
					          "Free Into Store",
					          "Free On Board",
                              ""
                             };
	int	wpipe_open = FALSE;

	char	cat_desc [10] [21];
	char	*inv_cat [] = {"",
                          "",
                          "",
                          "",
                          "",
                          "",
                          "",
                          "",
                          "",
                          ""};
	char*   inv_cat_const [] ={"Goods (FOB)",
		                      "O/S Freight",
                              "O/S Insurance",
                              "O/S Interest",
                              "O/S Bank Charges",
                              "Duty",
                              "Other - 1",
                              "Other - 2",
                              "Other - 3",
                              "Other - 4"
                             };

	struct ddgdRecord 	ddgdArray [10];

	int		storeMax = 0;
	int		envVarPoPrint = FALSE;
	int		envVarPoMaxLines = 0;
	int		envVarPoSuHist = 0;
	int		held_order;
	int		lpno = 0;
	int		updInis;
	int		allLinesConf;
	int		allLinesDesp;
	int		hhcc_selected = FALSE;

	char	progFlag [2];
	char	loc_prmt [15];
	char	fi_prmt [11];
	char	cif_prmt [11];
	char	dty_prmt [11];
	char	lcl_prmt [11];
	char	sel_prmt [11];
	char	ext_prmt [11];
	char	mar_prmt [11];
	char	dflt_due_date [11];
	char	envVarPoReorder [27];
	char	envVarSupOrdRound [2];
	char	poPrintProg [15];
#include	<p_terms.h>

struct	SHIP_PTR
{
	char	ship_no [3];
	long	due_date;
	long	hhds_hash;
	char	ship_meth [2];
	char	vessel [30];
	char	costed [2];
	char	status [2];
	struct	SHIP_PTR	*next;
};


struct	SHIP_PTR	*ship_curr;
struct	SHIP_PTR	*ship_head = SHIP_NULL;

/*===========================
| Local & Screen Structures |
===========================*/
struct {
								/*==================================*/
								/* Header Screen Local field.		*/
								/*==================================*/
	char	cust_no [7];			/* Customer Number                  */
	char	ho_cust_no [7];		/* Head Office Customer No.         */
	char	dd_ord_no [9];		/* Direct Delivery Order No.		*/
	char	po_ord_no [16];		/* Purchase Order No.				*/
	char	ship_no [3];			/* Shipment No.      				*/
	char	desc_method [5];		/* Shipment method description.		*/
								/*==================================*/
	char	systemDate [11];		/* Current Date dd/mm/yy.			*/
	char	prog_desc [4];		/* HDRSCN description field			*/
	char	exch_desc [4];		/* HDRSCN description field			*/
	double	exch_rate;			/* Local Exchange Rate.				*/
								/*==================================*/
								/* Order Screen Local field.		*/
								/*==================================*/
	char	supp_no [7];			/* Costing screen Supplier.			*/
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
								/* Line Item Screen Local field.	*/
								/*==================================*/
	char	item_no [17];		/* Local Item Number.				*/
	char	item_desc [41];      /* Local Item description.          */
	char	short_desc [21];
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
	long	hr_due_date;		/* Due Date at line item Level.		*/
	long	ln_due_date;		/* Due Date at line item Level.		*/
	char	br_no [3];			/* Branch number.					*/
	char	br_name [41];		/* Branch name.						*/
	char	wh_no [3];			/* Warehouse number.				*/
	char	wh_name [41];		/* Warehouse name.					*/
	long	hhcc_hash;			/* Warehouse hhcc_hash.				*/
	char	other [3] [31];
								/*==================================*/
								/* Miscellaneous Information.       */
								/*==================================*/
	int 	storeIdx;			/* Index into store array.     		*/
								/*==================================*/
	long	dflt_order_no;
	long	date_reqd;
	char	dummy [11];
	char	dflt_ord [2];
	char	pri_desc [16];
	char	ord_desc [10];
	long	lsystemDate;
	char	spinst [3] [61];
	char	sell_desc [31];
	double	extend;
	double	lcl_cst;
	float	margin;
	double	marval;
	float	uplift;
	char	dflt_sale_no [3];
	char	carr_area [3];
	char	cont_desc [41];
	char	inv_no [9];
	int		no_ctn;
	double	wgt_per_ctn;
} local_rec;

static	struct	var	vars [] =
{
	{HDRSCN, LIN, "supp_no",	 4, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Supplier           :", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.supp_no},
	{HDRSCN, LIN, "supp_name",	 4, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name               :", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{HDRSCN, LIN, "cust_no",	 5, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Customer           :", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{HDRSCN, LIN, "cust_name",	 5, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name               :", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{HDRSCN, LIN, "cus_addr1",	 6, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",      "Address            :", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr1},
	{HDRSCN, LIN, "cus_addr2",	 7, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                   :", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr2},
	{HDRSCN, LIN, "cus_addr3",	 8, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                   :", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr3},
	{HDRSCN, LIN, "cus_addr4",	 9, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                   :", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr4},
	{HDRSCN, LIN, "dd_ord_no",	 6, 22, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "00000000","DD Order Number    :"," Enter Direct Delivery Order Number ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dd_ord_no},
	{HDRSCN, LIN, "po_ord_no",	 7, 22, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "0", "PO Order Number    :", " Enter Purchase Order Number ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.po_ord_no},
	{HDRSCN, LIN, "date_reqd",	 8, 22, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00 ", "Date Required.     :", " ", 
		NA, NO,  JUSTLEFT, "", "", (char *)&ddhr_rec.dt_required},
	{HDRSCN, LIN, "cus_ord_ref",	 9, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",      "Order Ref.         :", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.cus_ord_ref},
	{HDRSCN, LIN, "db_cont_no",	 10, 22, CHARTYPE,
		"AAAAAA", "          ",
		" ", " ",      "Customer Contract  :", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.cont_no},
	{HDRSCN, LIN, "fe_cont_no",	 10, 88, CHARTYPE,
		"AAAAAA", "          ",
		" ", " ",      "Forward Exchange   :", " ",
		 ND, NO,  JUSTLEFT, "", "", ddhr_rec.fwd_exch},
	{HDRSCN, LIN, "invoice_no",	 11, 22, CHARTYPE,
		"MUUUUUUU", "          ",
		" ", "M0000000", "Invoice Number.    :", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.inv_no},
	{HDRSCN, LIN, "ship_no",	 12, 22, CHARTYPE,
		"NN", "          ",
		" ", " 1", "Shipment No.       :", " ",
		 NE, NO,  JUSTRIGHT, "", "", local_rec.ship_no},
	{HDRSCN, LIN, "hr_due_date",	 12, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", dflt_due_date, "Due Date           :", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.hr_due_date},
	{ORDSCN, TAB, "item_no",	 MAXLINES, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{ORDSCN, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Item Number.  ", " ",
		ND, NO,  JUSTLEFT, "", "", inmr_rec.description},
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
		" ", "", "NET FOB (FGN)", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_fob},
	{ORDSCN, TAB, "loc_fi",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", fi_prmt, "<Return> will Calculate Freight.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_fi},
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
	{ORDSCN, TAB, "ln_due_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Due Date", " ",
		 NI, NO,  JUSTRIGHT, "", "", (char *)&local_rec.ln_due_date},
	{ORDSCN, TAB, "hhdl_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &ddln_rec.hhdl_hash},
	{ORDSCN, TAB, "store_idx2",	 0, 0, INTTYPE,
		"NNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.storeIdx},

	{CSTSCN, TAB, "category",	TABLINES, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Category      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cst_category},
	{CSTSCN, TAB, "spread",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", "D", "Spread", " by : D (ollar  W (eight  V (olume ",
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
		" ", "", " Item Description.  ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.short_desc},
	{PRISCN, TAB, "qty3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", " Quantity ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty},
	{PRISCN, TAB, "lcl_cost",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", lcl_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.land_cst},
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
		" ", "0.00", " Disc ", " ",
		YES, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&ddln_rec.disc_pc},
	{PRISCN, TAB, "extend",	 0, 0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", ext_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extend},
	{PRISCN, TAB, "margin",	 0, 0, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "", "  GP %  ", " ",
		NA, NO, JUSTRIGHT, "-99.99", "99.99", (char *)&local_rec.margin},
	{PRISCN, TAB, "marval",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", mar_prmt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.marval},
	{PRISCN, TAB, "store_idx3",	 0, 0, INTTYPE,
		"NNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.storeIdx},

	{TLRSCN, LIN, "shipname",	3, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dbt_name, "Ship to name :", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.del_name},
	{TLRSCN, LIN, "shipaddr1",	4, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Ship to addr 1", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.del_add1},
	{TLRSCN, LIN, "shipaddr2",	5, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Ship to addr 2", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.del_add2},
	{TLRSCN, LIN, "shipaddr3",	6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Ship to addr 3", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.del_add3},
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
		NA, NO,  JUSTLEFT, "", "", local_rec.spinst [0]},
	{TLRSCN, LIN, "spcode1",	9, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 2", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.spinst [1]},
	{TLRSCN, LIN, "spcode2",	10, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Instruction 3", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.spinst [2]},
	{TLRSCN, LIN, "no_ctn",	8, 103, INTTYPE,
		"NNNNNNNN", "          ",
		" ", " ", "No. of Carton ", " ",
		NA, NO,  JUSTRIGHT, "", "", (char *) &local_rec.no_ctn},
	{TLRSCN, LIN, "wgt_per",	9, 103, DOUBLETYPE,
		"NNNNNNNN.NNNN", "          ",
		" ", " ", "Weight/Carton    :", " ",
		NA, NO,  JUSTRIGHT, "", "", (char *) &local_rec.wgt_per_ctn},
	{TLRSCN, LIN, "pay_term",	 11, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.", " ",
		NA, NO,  JUSTLEFT, "", "", ddhr_rec.pay_term},
	{TLRSCN, LIN, "sin1",	13, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 1.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.stdin1},
	{TLRSCN, LIN, "sin2",	14, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 2.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.stdin2},
	{TLRSCN, LIN, "sin3",	15, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Standard Instr 3.", " ",
		NA, NO,  JUSTLEFT, "", "", pohr_rec.stdin3},
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

	{SHPSCN, LIN, "ship_method",	3, 23, CHARTYPE,
		"U", "          ",
		" ", sumr_rec.ship_method, "Shipment Method :", "Shipment Method L (and) / S (ea) / A (ir)",
		YES, NO,  JUSTLEFT, "LSA", "", ddsh_rec.ship_method},
	{SHPSCN, LIN, "desc_method",	3, 26, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc_method},
	{SHPSCN, LIN, "consignment",	3, 88, LONGTYPE,
		"NNN", "          ",
		" ", "0", "Consignment No. :", "Customers Consignment Number",
		YES, NO,  JUSTRIGHT, "", "", (char *) &ddsh_rec.con_no},
	{SHPSCN, LIN, "vessel",	4, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Vessel          :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.vessel},
	{SHPSCN, LIN, "space_book",	4, 88, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Space Booked        :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.space_book},
	{SHPSCN, LIN, "carrier",	5, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipping Line   :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.carrier},
	{SHPSCN, LIN, "book_ref",	5, 88, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Booking Reference   :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.book_ref},
	{SHPSCN, LIN, "bol_no",	6, 23, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "B.O.L. No.      :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.bol_no},
	{SHPSCN, LIN, "airway",	6, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Airways B.O.L. No.  :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.airway},
	{SHPSCN, LIN, "date_load", 7, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Loading Date    :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&ddsh_rec.date_load},
	{SHPSCN, LIN, "con_rel_no",	7, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Container Release No:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.con_rel_no},
	{SHPSCN, LIN, "packing",	8, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Packing Details :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.packing},
	{SHPSCN, LIN, "port_orig",	10, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Departure Port  :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.port_orig},
	{SHPSCN, LIN, "date_depart", 10, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Departure Date      :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&ddsh_rec.ship_depart},
	{SHPSCN, LIN, "dept_orig",	11, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Departure depot :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.dept_orig},
	{SHPSCN, LIN, "port_dsch",	12, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Discharge Port  :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.port_dsch},
	{SHPSCN, LIN, "port_dest",	13, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Destination     :", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.port_dest},
	{SHPSCN, LIN, "date_arrive", 13, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Arrival Date        :", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&ddsh_rec.ship_arrive},
	{SHPSCN, LIN, "mark0",	15, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Shipping Marks 1:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.mark0},
	{SHPSCN, LIN, "mark1",	16, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               2:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.mark1},
	{SHPSCN, LIN, "mark2",	17, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               3:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.mark2},
	{SHPSCN, LIN, "mark3",	18, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               4:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.mark3},
	{SHPSCN, LIN, "mark4",	19, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "               5:", " ",
		YES, NO,  JUSTLEFT, "", "", ddsh_rec.mark4},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include 	<cus_price.h>
#include 	<cus_disc.h>
#include 	<neg_win.h>
#include 	<SupPrice.h>

int		PrntCnt	= 0;
int		running = 0;

FILE	*pout;

/*=====================================================================
| Loccal Function Prototypes.
=====================================================================*/
double	DtyCalc					(int);
double	FrtCalc					(int);
double	OutCost					(double,float);
float	RndMltpl				(float, char *, float, float);
float	ScreenDisc				(float);
int		AddIncc					(void);
int		AddInei					(void);
int		AddToShipment			(int, float, char *, long);
int		CheckFehrTotals			(char *);
int		CheckReorder			(char *);
int		ClrShipList				(struct	SHIP_PTR	*);
int		ConfirmMenu				(void);
int		DespatchMenu			(void);
int		EditMenu				(void);
int		FindCucc				(int, long);
int		FindInis				(long, long);
int		FindPocf				(char *);
int		FindPocr				(char *);
int		GetNextDdlnLine			(void);
int		GetNextPolnLine			(void);
int		LoadItems				(long, long);
int		LoadPoln				(long);
int		SelectShipment			(char *, long);
int		SrchCudi 				(int);
int		UpdateOrder				(char *);
int		WarnUser				(char *, int, int);
int		heading					(int);
int		spec_valid				(int);
long	GetDueDate				(void);
struct	SHIP_PTR *AddToShipList	(char *, struct	SHIP_PTR *);
struct	SHIP_PTR *ShipAlloc		(void);
void	CalcCost				(int);
void	CalcCostTotals			(long);
void	CalcDdhrTotals			(void);
void	CalcExtend				(int);
void	CalcItemTotals			(long);
void	CalcTotal				(void);
void	CatIntoPohr				(void);
void	CloseDB					(void);
void	Destroy_ML				(void);
void	DiscProcess				(int);
void	DispCustSupp			(int);
void	DispFlds				(int);
void	DrawDiscScn				(void);
void	DrawVLine				(int, int);
void	GenGrnNo				(void);
void	GetNextShipNo			(char *);
void	GetWarehouse			(long);
void	Init_ML					(void);
void	InputField				(int);
void	LoadCatDesc				(void);
void	LoadDdgd				(long);
void	IntLoadEnvironment		(void);
void	LoadIntoCstScn			(void);
void	LoadIntoOrdScn			(void);
void	LoadIntoPriScn			(void);
void	LoadIntoTlrScn			(void);
void	MarginOK				(double, double, float, int, double,float,int);
void	OpenDB					(void);
void	PriceProcess			(int);
void	PrintCoStuff			(void);
void	SillyBusyFunc			(int);
void	SpreadCosts				(long);
void	SrchDdhr				(char *);
void	SrchDdsh				(char *);
void	SrchPocr				(char *);
void	SrchPohr				(char *);
void	TabScreen2				(int);
void	UpdateDdgd				(long, char *);
void	UpdateDdln				(int, char *);
void	UpdateDdsh				(char *, char *);
void	UpdateInis				(double, float);
void	UpdatePogh				(char *);
void	UpdatePogl				(int, char *);
void	UpdatePohr				(void);
void	UpdatePohs				(char *);
void	UpdatePoln				(int, char *);
void	UpdateSuph				(int, char *);
void	shutdown_prog			(void);
void	tab_other				(int);
int		use_window				(int);

#ifdef GVISION
#include <disc_win.h>
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#define	fflush	Remote_fflush
#else
void ViewDiscounts (void);
#endif	/* GVISION */

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	tab_row = 8;

	if (argc < 3)
	{
		print_at (0,0,ML (mlDdMess701),argv [0]);
        return (EXIT_FAILURE);
	}

	sprintf (progFlag, "%1.1s", argv [1]);
	if (strcmp (progFlag, CONFIRMFLAG) &&
	    strcmp (progFlag, DESPATCHFLAG))
	{
		print_at (0,0,ML (mlDdMess701),argv [0]);
		print_at (0,0,ML (mlDdMess702), CONFIRMFLAG);
		print_at (0,0,ML (mlDdMess703), DESPATCHFLAG);
        return (EXIT_FAILURE);
	}

	lpno = (argc == 4) ? atoi (argv [3]) : 0;

	IntLoadEnvironment ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	vars [label ("dd_ord_no")].fill = " ";
	vars [label ("dd_ord_no")].lowval = alpha;
	vars [label ("po_ord_no")].fill = " ";
	vars [label ("po_ord_no")].lowval = alpha;

	sptr = chk_env ("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	init_scr ();
	_set_masks (argv [2]);

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ORDSCN, store, sizeof (struct storeRec));
#endif

	/*------------------------
	| Set up Screen Prompts. |
	------------------------*/
	sprintf (loc_prmt,"  %-3.3s Value  ",envVarCurrCode);
	sprintf (fi_prmt," F&I (%-3.3s)",envVarCurrCode);
	sprintf (cif_prmt," CIF (%-3.3s) ",envVarCurrCode);
	strcpy  (dty_prmt,"Duty/ Unit");
	sprintf (lcl_prmt," Cost (%-3.3s) ",envVarCurrCode);
	sprintf (mar_prmt," GP (%-3.3s) ",	envVarCurrCode);

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();
	Init_ML ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	GetWarehouse (0L);

	sptr = get_env ("CR_CO");
	strcpy (cbranchNumber, (atoi (sptr)) ? comm_rec.est_no : " 0");
	sptr = get_env ("DB_CO");
	strcpy (dbranchNumber, (atoi (sptr)) ? comm_rec.est_no : " 0");

	memset (store, 0, sizeof (store));

	envVarDbFind = atoi (get_env ("DB_FIND"));

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
		cumr_rec.hhcu_hash = 0L;
		cnch_rec.hhch_hash = 0L;
		memset (&inmr_rec, 0, sizeof (inmr_rec));

		abc_unlock (ddhr);
		abc_unlock (ddln);
		abc_unlock (pohr);
		abc_unlock (poln);

		search_ok = TRUE;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		ship_head = SHIP_NULL;
		init_vars (HDRSCN);	
		init_vars (ORDSCN);	
		init_vars (CSTSCN);	
		init_vars (PRISCN);	
		init_vars (TLRSCN);	
		lcount [ORDSCN]	= 0;	
		lcount [CSTSCN]	= 0;
		lcount [PRISCN] = 0;

		heading (HDRSCN);
		entry (HDRSCN);
		if (restart || prog_exit)
			continue;

		while (!edit_exit && !restart)
		{
			EditMenu ();

			if (!restart && edit_exit) 
			{
				if (!strcmp (progFlag, CONFIRMFLAG))
					edit_exit = ConfirmMenu ();
				else
					edit_exit = DespatchMenu ();
			}
		}
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
			" Shipment Details " },
		{ ENDMENU }
	};


int
EditMenu (
 void)
{
	for (;;)
	{
		mmenu_print ("       Edit (3)        ", edit_menu, 0);
		switch (mmenu_select (edit_menu))
		{
			case  0 :
				heading 	 (HDRSCN);
				scn_display (HDRSCN);
				edit 		 (HDRSCN);
				return (TRUE);

			case  1 :
				heading 	 (ORDSCN);
				scn_display (ORDSCN);
				edit 		 (ORDSCN);
				return (TRUE);

			case  2 :
				heading 	 (CSTSCN);
				scn_write 	 (CSTSCN);
				DispCustSupp (-1);
				LoadCatDesc ();
				CalcItemTotals (sumr_rec.hhsu_hash);
				LoadIntoCstScn ();
				scn_display (CSTSCN);
				edit 		 (CSTSCN);
				if (!restart)
					SpreadCosts (sumr_rec.hhsu_hash);
				return (TRUE);

			case  3 :
				heading 	 (PRISCN);
				scn_display (PRISCN);
				edit 		 (PRISCN);
				return (TRUE);

			case  4 :
				heading 	 (TLRSCN);
				scn_display (TLRSCN);
				edit 		 (TLRSCN);
				return (TRUE);

			case  5 :
				heading 	 (SHPSCN);
				scn_display (SHPSCN);
				edit 		 (SHPSCN);
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


MENUTAB desp_menu [] =
	{
		{ " 1. SAVE SHIPMENT       ",
			" Save shipment to be continued later " },
		{ " 2. DESPATCH SHIPMENT   ",
			" Confirm suppliers despatch of shipment " },
		{ " 3. RE-EDIT SHIPMENT    ",
			" Re-edit current shipment " },
		{ " 4. DELETE SHIPMENT     ",
			" Delete this shipment only, Split Lines are still updated " },
		{ " 5. ABANDON SHIPMENT    ",
			" Abandon Changes made to shipment " },
		{ "                        ",
			"                                  " },
		{ ENDMENU }
	};


int
DespatchMenu (void)
{
	for (;;)
	{
		mmenu_print ("  ORDER OPTIONS   ", desp_menu, 0);
		switch (mmenu_select (desp_menu))
		{
			case  0 :
			case 99 :
				cc = !UpdateOrder (CONFIRMFLAG);
				return (cc);

			case  1 :
				cc = !UpdateOrder (DESPATCHFLAG);
				return (cc);

			case  3 :
				cc = !UpdateOrder (DELETEFLAG);
				return (cc);

			case  2 :
				return (FALSE);

			case  4 :
			case -1 :
				restart = TRUE;
				return (TRUE);
	
			default :
				break;
		}
	}
}



MENUTAB conf_menu [] =
	{
		{ " 1. SAVE SHIPMENT       ",
			" Save shipment to be continued later " },
		{ " 2. CONFIRM SHIPMENT    ",
			" Confirm suppliers acceptance of shipment " },
		{ " 3. RE-EDIT SHIPMENT    ",
			" Re-edit current shipment " },
		{ " 4. DELETE SHIPMENT     ",
			" Delete this shipment only, Split Lines are still updated " },
		{ " 5. ABANDON SHIPMENT    ",
			" Abandon Changes made to shipment " },
		{ "                        ",
			"                                  " },
		{ ENDMENU }
	};


int
ConfirmMenu (
 void)
{
	for (;;)
	{
		mmenu_print ("  ORDER OPTIONS   ", conf_menu, 0);
		switch (mmenu_select (conf_menu))
		{
			case  0 :
			case 99 :
				cc = !UpdateOrder (ACTIVEFLAG);
				return (cc);

			case  1 :
				cc = !UpdateOrder (CONFIRMFLAG);
				return (cc);

			case  3 :
				cc = !UpdateOrder (DELETEFLAG);
				return (cc);

			case  2 :
				return (FALSE);

			case  4 :
			case -1 :
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
shutdown_prog (
 void)
{
    Destroy_ML ();
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
		
	open_rec  (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec  (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec  (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec  (cncd, cncd_list, cncd_no_fields, "cncd_id_no2");
	open_rec  (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec  (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec  (ddgd, ddgd_list, DDGD_NO_FIELDS, "ddgd_id_no");
	open_rec  (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_id_no");
	open_rec  (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
	open_rec  (ddsh, ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no");
	open_rec  (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec  (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec  (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec  (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec  (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec  (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec  (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec  (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec  (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec  (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec  (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");

	if (envVarFeInstall)
	{
		open_rec  (fehr, fehr_list, FEHR_NO_FIELDS, "fehr_id_no");
		open_rec  (feln, feln_list, FELN_NO_FIELDS, "feln_id_no");
	}
	open_rec  (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no" 
							  							   : "cumr_id_no3");
	open_rec  (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
														   : "sumr_id_no3");

	abc_alias (cumr2, cumr);
	abc_alias (ddln2, ddln);

	open_rec  (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec  (ddln2, ddln_list, DDLN_NO_FIELDS, "ddln_hhdl_hash");

	if (!strcmp (progFlag, DESPATCHFLAG))
	{
		open_rec  (pogh, pogh_list, POGH_NO_FIELDS, "pogh_id_no");
		open_rec  (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
		open_rec  (pohs, pohs_list, POHS_NO_FIELDS, "pohs_id_no");
		if (envVarPoSuHist)
			open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_id_no");
	}
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (cncl);
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (cucc);
	abc_fclose (cudp);
	abc_fclose (cumr);
	abc_fclose (ddgd);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (insp);
	abc_fclose (inum);
	abc_fclose (pocr);
	abc_fclose (podt);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (suds);
	abc_fclose (sumr);

	if (envVarFeInstall)
	{
		abc_fclose (fehr);
		abc_fclose (feln);
	}

	abc_fclose (cumr2);
	abc_fclose (ddln2);

	if (!strcmp (progFlag, DESPATCHFLAG))
	{
		abc_fclose (pogh);
		abc_fclose (pogl);
		abc_fclose (pohs);
		if (envVarPoSuHist)
			abc_fclose (suph);
	}
	/*------------------------------
	| Close Three Discount files.  |
	------------------------------*/

	SearchFindClose ();
	abc_dbclose (data);
}



/*============================================
| Get common info from environment variables. |
============================================*/
void
IntLoadEnvironment (
 void)
{
	char		*sptr;

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
		sprintf (poPrintProg, "%-14.14s", sptr);
	}

	envVarCurrentUser = getenv ("LOGNAME");

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


	sptr = chk_env ("PO_MAX_LINES");
	envVarPoMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (envVarCurrCode,"%-3.3s",get_env ("CURR_CODE"));


	/*---------------------------
	| Forward Exchange Enabled? |
	---------------------------*/
	sptr = chk_env ("FE_INSTALL");
	envVarFeInstall = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (envVarFeInstall)
		FLD ("fe_cont_no") = NA;

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
	db_stopcrd = (* (sptr + 0) == 'S');
	db_crdterm = (* (sptr + 1) == 'S');
	db_crdover = (* (sptr + 2) == 'S');

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVarWinOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-----------------------------------------------------------
	| Check if stock information window is loaded at load time. |
	-----------------------------------------------------------*/
	sptr = chk_env ("SO_PERM_WIN");
	envVarSoPermWin = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (envVarSoPermWin)
	{
		/***
		if (open_sk_win ())
			envVarWinOk = FALSE;
		***/
	}

	sptr = chk_env ("SO_OTHER_1");
	sprintf (local_rec.other [0],"%.30s", (sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (local_rec.other [1],"%.30s", (sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (local_rec.other [2],"%.30s", (sptr == (char *)0) ? "Other Costs." : sptr);

	sptr = chk_env ("PO_SU_HIST");
	if (sptr == (char *)0)
		envVarPoSuHist = 0;
	else
		envVarPoSuHist = atoi (sptr);

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	envVarSoDoi = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;
}

int
spec_valid (
 int    field)
{
	int			i;
	double		wrk_fob;
	double		custTotalOwing = 0.00;
	static	int newEntry = TRUE;

	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if (LCHECK ("supp_no"))
	{
		abc_selfield (sumr, (envVarCrFind) ? "sumr_id_no3" : "sumr_id_no");

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, cbranchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.supp_no, "      "))
		{
			memset (&sumr_rec, 0, sizeof (sumr_rec));
			strcpy (local_rec.supp_no, "      ");
			DSP_FLD ("supp_no");
			FLD ("cust_no") = NE;
			FLD ("dd_ord_no") = NE;
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, cbranchNumber);
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
		/***
		find_exsi ();
		***/

		/*--------------------------
		| Find currency code file. |
		--------------------------*/
		if (FindPocr (sumr_rec.curr_code))
			return (EXIT_FAILURE);


/***
		local_rec.supp_exch = pocr_rec.ex1_factor;
		strcpy (SR.supp_no,		sumr_rec.crd_no);
		strcpy (SR.supp_name,	sumr_rec.crd_name);
		strcpy (SR.supp_curr,	sumr_rec.curr_code);

		SR._hhsu_hash = sumr_rec.hhsu_hash;
		SR.supp_exch  = pocr_rec.ex1_factor;
***/

		/*--------------------
		| Find freight file. |
		--------------------*/
		if (FindPocf (sumr_rec.ctry_code))
			return (EXIT_FAILURE);

		memset (&cumr_rec, 0, sizeof (cumr_rec));
		strcpy (local_rec.cust_no, "      ");
		FLD ("cust_no") = NA;
		DSP_FLD ("cust_no");

		memset (&ddhr_rec, 0, sizeof (ddhr_rec));
		strcpy (local_rec.dd_ord_no, "        ");
		FLD ("dd_ord_no") = NA;
		DSP_FLD ("dd_ord_no");

		DSP_FLD ("supp_name");
		return (EXIT_SUCCESS);
	}
		
	/*---------------------------
	| Validate Customer Number. |
	---------------------------*/
	if (LCHECK ("cust_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy  (local_rec.dd_ord_no, "        ");
			DSP_FLD ("dd_ord_no");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, dbranchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,  comm_rec.co_no);
		strcpy (cumr_rec.est_no, dbranchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.cust_no));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
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
		if (cumr_rec.stop_credit [0] == 'Y')
		{
			if (db_stopcrd)
			{
				/*Customer is on Stop Credit,Cannot Process Any Orders.*/
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				/*WARNING Customer is on Stop Credit.*/
				cc = WarnUser (ML (mlStdMess060),0,2);
				if (cc)
					return (cc);
			}
		}

		custTotalOwing = cumr_bo [0] + cumr_bo [1] +
		              	 cumr_bo [2] + cumr_bo [3] +
		              	 cumr_bo [4] + cumr_bo [5];

		c_left = custTotalOwing - cumr_rec.credit_limit;

		/*---------------------------------------------
		| Check if customer is over his credit limit. |
		---------------------------------------------*/
		if (cumr_rec.credit_limit <= custTotalOwing && 
			        cumr_rec.credit_limit != 0.00)
		{
			if (db_crdover)
			{
				/*print_mess ("Customer is Over Credit Limit,Cannot Process Any Orders\007");*/
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				/*sprintf (err_str,"WARNING Customer is over Credit Limit by %.2f\007",DOLLARS (c_left));*/
				cc = WarnUser (ML (mlStdMess061),0,2);
				if (cc)
					return (EXIT_FAILURE);

			}
		}
		/*-----------------------
		| Check Credit Terms	|
		-----------------------*/
		if (cumr_rec.od_flag)
		{
			if (db_crdterm)
			{
				/*sprintf (err_str,"Customer credit terms exceeded by %d periods, CANNOT PROCESS ORDER\007", cumr_rec.od_flag);*/
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);    
			}
			else
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				cc = WarnUser (err_str,0,2);
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

		/*
		LoadIntoTlrScn ();
		*/
		DSP_FLD ("cust_no");
		DSP_FLD ("cust_name");
		DSP_FLD ("cus_addr1");
		DSP_FLD ("cus_addr2");
		DSP_FLD ("cus_addr3");
		DSP_FLD ("cus_addr4");

		use_window (FN14);
		strcpy (local_rec.dflt_sale_no, (!strcmp (ccmr_rec.sman_no,"  ")) ? cumr_rec.sman_code : ccmr_rec.sman_no);
		
		/*------------
		| sort out tax 
		--------------*/
		strcpy (ddhr_rec.tax_code, cumr_rec.tax_code);
		if (ddhr_rec.tax_code [0] == 'A' || 
			 ddhr_rec.tax_code [0] == 'B')
			notax = 1;
		else
			notax = 0;

		/*----------------------------------
		| Get currency code and exch rate. |
		----------------------------------*/
		if (envVarDbMcurr)
		{
			if (FindPocr (cumr_rec.curr_code))
			{
				/*print_mess ("\007 Currency Code For Customer Not Found. ");*/
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sprintf (sel_prmt,"Sell (%-3.3s)",	cumr_rec.curr_code);
			sprintf (ext_prmt," Ext (%-3.3s) ",	cumr_rec.curr_code);
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate DD Order Number. |
	---------------------------*/
	if (LCHECK ("dd_ord_no")) 
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			strcpy  (local_rec.dd_ord_no, "        ");
			DSP_FLD ("dd_ord_no");
			return (EXIT_SUCCESS);
		}

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
		strcpy (ddhr_rec.order_no, zero_pad (local_rec.dd_ord_no,8));
		ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;

		cc = find_rec (ddhr, &ddhr_rec, EQUAL, "w");
		if (cc)
		{
			if (restart)
				return (EXIT_SUCCESS);

			/*sprintf (err_str,
					 "Order No. %s is not on file.\007", 
					 ddhr_rec.order_no);*/
			print_mess (ML (mlStdMess122));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (ddhr);
			return (EXIT_FAILURE); 
		}

		if (cumr_rec.hhcu_hash != ddhr_rec.hhcu_hash)
		{
			print_mess (ML (mlStdMess122));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (ddhr);
			return (EXIT_FAILURE); 
		}

		if (!strcmp (progFlag, CONFIRMFLAG) &&
			 strcmp (ddhr_rec.stat_flag, ACTIVEFLAG))
		{
			sprintf (err_str,ML (mlDdMess040), ddhr_rec.order_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (ddhr);
			return (EXIT_FAILURE); 
		}
                
		if (strcmp (ddhr_rec.cont_no, "      "))
		{
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, ddhr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlDdMess068));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (strcmp (ddhr_rec.fwd_exch, "      "))
		{
			strcpy (fehr_rec.co_no, comm_rec.co_no);
			strcpy (fehr_rec.cont_no, ddhr_rec.fwd_exch);
			cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
			if (cc)
			{
				/*sprintf (err_str,
				   "Order No. %s has an invalid Forward Exchange Contract No. %s.\007",
				    ddhr_rec.order_no, ddhr_rec.fwd_exch);*/
				print_mess (ML (mlDdMess069));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (ddhr_rec.fix_exch [0] == 'Y')
			strcpy (local_rec.exch_desc, "Yes");
		else
			strcpy (local_rec.exch_desc, "No ");

		if (envVarDbMcurr)
			local_rec.exch_rate = ddhr_rec.exch_rate;
		else
			local_rec.exch_rate = 1.00;
		
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no,    ddhr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cudp", "DBFIND");

		local_rec.date_reqd = ddhr_rec.dt_required;
		local_rec.hr_due_date = ddhr_rec.dt_required;

		strcpy (dflt_due_date, DateToString (local_rec.date_reqd));

		strcpy (local_rec.spinst [0], ddhr_rec.din_1);
		strcpy (local_rec.spinst [1], ddhr_rec.din_2);
		strcpy (local_rec.spinst [2], ddhr_rec.din_3);
		local_rec.no_ctn		=	ddhr_rec.no_ctn;
		local_rec.wgt_per_ctn	=	ddhr_rec.wgt_per_ctn;

		DSP_FLD ("dd_ord_no");
		DSP_FLD ("date_reqd");
		DSP_FLD ("cus_ord_ref");
		PrintCoStuff ();
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("po_ord_no"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (FLD ("cust_no") == NA)
		{
			abc_selfield (pohr, "pohr_id_no");
			pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		}
		else
		{
			abc_selfield (pohr, "pohr_id_no4");
			pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		}
		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (pohr_rec.co_no,      comm_rec.co_no);
		strcpy (pohr_rec.br_no,      comm_rec.est_no);
		strcpy (pohr_rec.type,       "O");
		strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.po_ord_no, 15));

		cc = find_rec (pohr, &pohr_rec, EQUAL, "w");
		if (cc)
		{
			if (restart)
				return (EXIT_SUCCESS);

			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (pohr_rec.drop_ship [0] != 'Y')
		{
			/*print_mess ("Sorry, Purchase Order does not relate to a direct delivery.");*/
			print_mess (ML (mlDdMess003));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}

		if (pohr_rec.status [0] != 'O' && 
		     pohr_rec.status [0] != 'R' &&
		     pohr_rec.status [0] != 'U' &&
		     pohr_rec.status [0] != 'r' &&
		     pohr_rec.status [0] != 'P' &&
			 pohr_rec.status [0] != 'C')
		{
			
			/*print_mess ("Sorry, Purchase order cannot be confirmed as it does not have correct status");*/
			print_mess (ML (mlDdMess031));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (FLD ("cust_no") == NA)
		{
			abc_selfield (ddhr, "ddhr_hhdd_hash");
			ddhr_rec.hhdd_hash	=	pohr_rec.hhdd_hash;
			cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, ddhr, "DBFIND");
			else
				strcpy (local_rec.dd_ord_no, ddhr_rec.order_no);
			abc_selfield (ddhr, "ddhr_id_no");

			abc_selfield (cumr, "cumr_hhcu_hash");
			cumr_rec.hhcu_hash	=	ddhr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "cumr", "DBFIND");
			else
				strcpy (local_rec.cust_no, cumr_rec.dbt_no);

			abc_selfield (cumr, (!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");

			FLD ("cust_no") = NE;
			DSP_FLD ("cust_no");
			cc = spec_valid (label ("cust_no"));
			if (cc)
			{
				strcpy (local_rec.cust_no,   "      ");
				strcpy (local_rec.dd_ord_no, "        ");
				DSP_FLD ("cust_no");
				DSP_FLD ("dd_ord_no");
				return (EXIT_FAILURE);
			}
			FLD ("cust_no") = NA;

			FLD ("dd_ord_no") = NE;
			DSP_FLD ("dd_ord_no");
			cc = spec_valid (label ("dd_ord_no"));
			if (cc)
			{
				strcpy (local_rec.cust_no,   "      ");
				strcpy (local_rec.dd_ord_no, "        ");
				DSP_FLD ("cust_no");
				DSP_FLD ("dd_ord_no");
				return (EXIT_FAILURE);
			}
			FLD ("dd_ord_no") = NA;

		}

		abc_selfield (sumr, "sumr_hhsu_hash");
		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlDdMess070));
			abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");

		strcpy (local_rec.supp_no, sumr_rec.crd_no);

		DSP_FLD ("supp_no");
		DSP_FLD ("supp_name");

		if (pohr_rec.status [0] == 'U')
		{
			print_mess (ML (mlDdMess032));
			sleep (sleepTime);

			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Shipment Number. |
	---------------------------*/
	if (LCHECK ("ship_no"))
	{
		if (SRCH_KEY)
		{
			SrchDdsh (temp_str);
			return (EXIT_SUCCESS);
		}

		/*-------------------------------
		| Check if shipment is on file. |
		-------------------------------*/
		abc_selfield (ddsh, "ddsh_id_no");
		ddsh_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		ddsh_rec.hhsu_hash = sumr_rec.hhsu_hash;
		sprintf (ddsh_rec.ship_no, "%2.2s", local_rec.ship_no);
		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "r");
		if (cc)
		{
			/*sprintf (err_str,"\007 Shipment %s not on file", ddsh_rec.ship_no);*/
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!strcmp (progFlag, CONFIRMFLAG))
		{
			switch (ddsh_rec.stat_flag [0])
			{
				case 'C' : /*sprintf (err_str,"\007 Shipment %s is already confirmed",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess005));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
				case 'D' : /*sprintf (err_str,"\007 Shipment %s is already despatched",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess006));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
				case 'I' : /*sprintf (err_str,"\007 Shipment %s is already despatched and invoiced",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess007));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
				default	 : break;
			}
		}


		if (!strcmp (progFlag, DESPATCHFLAG))
		{
			switch (ddsh_rec.stat_flag [0])
			{
				case 'C' : break;
				case 'D' : /*sprintf (err_str,"\007 Shipment %s is already despatched",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess006));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
				case 'I' : /*sprintf (err_str,"\007 Shipment %s is already despatched and invoiced",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess007));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
				default	 : /*sprintf (err_str,"\007 Shipment %s is not yet confirmed",
								   ddsh_rec.ship_no);*/
						   print_mess (ML (mlDdMess008));
						   sleep (sleepTime);
						   return (EXIT_FAILURE);
			}
		}

		if (ddsh_rec.cost_flag [0] == 'Y')
		{
			LoadDdgd (ddsh_rec.hhds_hash);
			LoadIntoCstScn ();
		}

		if (LoadItems (ddhr_rec.hhdd_hash, ddsh_rec.hhds_hash))
		{
			scn_set (HDRSCN);
			restart = TRUE;
			return (EXIT_FAILURE);
		}

		if (ddhr_rec.dt_required > 0L)
			strcpy (dflt_due_date, DateToString (ddhr_rec.dt_required));
		else
			strcpy (dflt_due_date, DateToString (ddhr_rec.dt_raised));

		if (ddsh_rec.due_date != 0L)
		{
			if (ddsh_rec.due_date > ddhr_rec.dt_required)
			{
				clear_mess ();
				/*sprintf (err_str,"\007Due Date is past the required date");*/
				print_mess (ML (mlDdMess035));
				sleep (sleepTime);
			}
			local_rec.hr_due_date = ddsh_rec.due_date;
			FLD ("hr_due_date") = NI;
		}

		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Shipment Due Date. |
	-----------------------------*/

	if (LCHECK ("hr_due_date"))
	{
		if (dflt_used)
		{
			local_rec.hr_due_date = ddhr_rec.dt_required;
			DSP_FLD ("hr_due_date");
		}

		if (prog_status != ENTRY &&
			local_rec.hr_due_date > ddhr_rec.dt_required)
		{
			clear_mess ();
			i = prmptmsg (ML (mlDdMess036),"YyNn",1,23);
			clear_mess ();
			if (i == 'n' || i == 'N') 
				return (EXIT_FAILURE);
		}
		for (i = 0; i < storeMax; i++)
		{
			if (!strcmp (ddsh_rec.ship_no, store [i].ship_no))
				store [i].due_date = local_rec.hr_due_date;
		}
		LoadIntoOrdScn ();
		scn_set (HDRSCN);
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}
		
	/*---------------------------
	| Validate Shipment Method. |
	---------------------------*/
	if (LCHECK ("ship_method"))
	{
		switch (ddsh_rec.ship_method [0])
		{
			case 'L' :	strcpy (local_rec.desc_method, "Land");
						break;
			case 'S' :	strcpy (local_rec.desc_method, "Sea ");
						break;
			case 'A' :	strcpy (local_rec.desc_method, "Air ");
						break;
			/*default	 :  print_mess ("\007Invalid Shipment Method");*/
			default	 :  print_mess (ML (mlStdMess119));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
		}
		DSP_FLD ("desc_method");

		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Quantity input. |
	--------------------------*/
	if (LCHECK ("qty"))
	{
		if (dflt_used)
			local_rec.qty = SI.min_order;

		local_rec.qty = RndMltpl (local_rec.qty,
									envVarSupOrdRound,
									SI.ord_multiple,
									SI.min_order);

		local_rec.qty = twodec (local_rec.qty);

		if (prog_status == ENTRY && local_rec.qty == 0.00)
		{
			/*sprintf (err_str,"\007Item cannot have quantity zero");*/
			print_mess (ML (mlStdMess123));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SI.cont_cost == FALSE && prog_status == ENTRY)
		{
			SI.grs_fgn  = GetSupPrice (SI.hhsu_hash,
									  SI.hhbr_hash,
									  SI.base_cst,
									  local_rec.qty);

			SI.cumulative = GetSupDisc (SI.hhsu_hash,
									   inmr_rec.buygrp,
									   local_rec.qty,
									   SI.purchDiscs);
		}

		if (prog_status != ENTRY)
		{
			if (local_rec.qty != SI.qty && SI.keyed == 0)
				SI.keyed = 2;

			if (local_rec.qty < SI.qty)
			{
				i = prmptmsg (ML (mlDdMess004),"YyNn",1,2);
				box (0, 2, 132, 2);
				if (i == 'Y' || i == 'y')
				{
					cc = AddToShipment (local_rec.storeIdx, 
								   local_rec.qty,
								   ddsh_rec.ship_no, 
								   0L);
					if (cc)
						return (EXIT_FAILURE);
				}
			}
			SI.qty = local_rec.qty;
			if (SI.qty == 0.00)
			{
				LoadIntoOrdScn ();
				heading		 (ORDSCN);
				scn_display (ORDSCN);
				if (line_cnt > lcount [ORDSCN])
					line_cnt = lcount [ORDSCN];
			}
			else
				DSP_FLD ("qty");


			/*-------------------------------------
			| Contract costs are always in std uom |
			--------------------------------------*/
			if (SI.cont_cost == FALSE)
				wrk_fob = SI.grs_fgn * SI.supp_conv;
			else
				wrk_fob = SI.grs_fgn;

			if (SI.outer > 0.00)
				wrk_fob /= SI.outer;

			local_rec.grs_fgn = wrk_fob;
			CalcCost (local_rec.storeIdx);

			if (ddsh_rec.cost_flag [0] == 'Y')
			{
				CalcItemTotals (sumr_rec.hhsu_hash);
				LoadIntoCstScn ();
				putval (line_cnt);
				SpreadCosts (sumr_rec.hhsu_hash);
				scn_display (ORDSCN);
			}
			if (lcount [ORDSCN])
				line_display ();
		}
		else
			SI.qty = local_rec.qty;



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
		wrk_fob = SI.base_cst;
		if (SI.outer > 0.00)
			wrk_fob /= SI.outer;

		if (SI.cont_cost == TRUE)
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
			SillyBusyFunc (0);
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
			if (SI.no_inis == FALSE)
			{
				/*---------------
				| Prompt		|
				---------------*/
				if (updInis == -1)
				{
					SI.upd_inis = prmpt_inis (0,23);
				}
				else
					SI.upd_inis = updInis;
			}
		}

		SI.grs_fgn = local_rec.grs_fgn;
		if (prog_status != ENTRY)
		{
			local_rec.net_fob = CalcNet (local_rec.grs_fgn, 
										 SI.purchDiscs, 
										 SI.cumulative);
			SI.net_fob = local_rec.net_fob;
			CalcCost (local_rec.storeIdx);
			local_rec.land_cst = SI.land_cst;
			sav_line = line_cnt;
			if (ddsh_rec.cost_flag [0] == 'Y')
			{
				putval (line_cnt);
				CalcItemTotals (SR.hhsu_hash);
				SpreadCosts (SR.hhsu_hash);
				scn_display (ORDSCN);
			}
			line_cnt = sav_line;
			line_display ();
			DSP_FLD ("net_fob");
		}

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate View Discount. |
	-------------------------*/
	if (LCHECK ("view_disc"))
	{
		if (SI.cont_cost == TRUE)
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
			discRec.discArray [0]	= SR.purchDiscs [0];
			discRec.discArray [1]	= SR.purchDiscs [1];
			discRec.discArray [2]	= SR.purchDiscs [2];
			discRec.discArray [3]	= SR.purchDiscs [3];

			ViewDiscounts (DBOX_LFT, DBOX_TOP, SR.cumulative);

			local_rec.grs_fgn	= CENTS (discRec.grossPrice);
			SR.purchDiscs [0]	= discRec.discArray [0];
			SR.purchDiscs [1]	= discRec.discArray [1];
			SR.purchDiscs [2]	= discRec.discArray [2];
			SR.purchDiscs [3]	= discRec.discArray [3];
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
										 SI.purchDiscs, 
										 SI.cumulative);
			SI.net_fob = local_rec.net_fob;
		}
		CalcCost (local_rec.storeIdx);
		if (prog_status == ENTRY)
		{
			if (ddsh_rec.cost_flag [0] == 'Y')
			{
				LoadIntoOrdScn ();
				CalcItemTotals (sumr_rec.hhsu_hash);
				LoadIntoCstScn ();
				putval (line_cnt);
				SpreadCosts (sumr_rec.hhsu_hash);
			}
			LoadIntoPriScn ();
			scn_display (ORDSCN);
			lcount [ORDSCN]--;
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
			local_rec.loc_fi = FrtCalc (local_rec.storeIdx);
		
		SI.amt_fai = local_rec.loc_fi;
		CalcCost (local_rec.storeIdx);
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
			local_rec.duty_val = DtyCalc (local_rec.storeIdx);

		CalcCost (local_rec.storeIdx);

		SI.amt_dty = local_rec.duty_val;

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

		CalcCost (local_rec.storeIdx);

		SI.amt_oth = local_rec.oth;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ln_due_date"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
		{
			local_rec.ln_due_date = local_rec.hr_due_date;
			DSP_FLD ("ln_due_date");
			SI.due_date = local_rec.ln_due_date;
			return (EXIT_SUCCESS);
		}
		
		if (dflt_used)
		{
			local_rec.ln_due_date = local_rec.hr_due_date;
			DSP_FLD ("ln_due_date");
		}

		if (local_rec.hr_due_date != local_rec.ln_due_date)
		{
			i = prmptmsg (ML (mlDdMess009),"YyNn",1,23);
			clear_mess ();
			if (i == 'n' || i == 'N') 
				return (EXIT_FAILURE);

			cc = AddToShipment (local_rec.storeIdx, 
						   0.00,
						   ddsh_rec.ship_no, 
						   local_rec.ln_due_date);
			if (cc)
				return (EXIT_FAILURE);
			local_rec.qty = SI.qty = 0.00;
			LoadIntoOrdScn ();
			heading		 (ORDSCN);
			scn_display (ORDSCN);
			if (line_cnt > lcount [ORDSCN])
				line_cnt = lcount [ORDSCN];
			DSP_FLD ("qty");
		}
		SI.due_date = local_rec.ln_due_date;
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("uplift"))
	{
		double		base_uplift;

		if (SI.pricing_chk == FALSE)
			PriceProcess (local_rec.storeIdx);

		base_uplift = 0.00;
		if (SI.land_cst)
			base_uplift = ((SI.calc_sprice / ddhr_rec.exch_rate) - SI.land_cst) 
						/ SI.land_cst;
		base_uplift *= 100.00;
		base_uplift = twodec (base_uplift);

		if (dflt_used)
			local_rec.uplift = base_uplift;

		if (twodec (local_rec.uplift) == twodec (base_uplift))
			SI.keyed = 0;
		else
			SI.keyed = 1;

		SI.uplift = local_rec.uplift;

		ddln_rec.sale_price = no_dec ((SI.land_cst * ddhr_rec.exch_rate) *
								      (1.00 + (local_rec.uplift / 100.00)));
		SI.sale_price = ddln_rec.sale_price;

		MarginOK (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   SI.qty,
				   SI.cont_status,
				   SI.land_cst, 
				   SI.min_marg,
				   FALSE);
	
		temp_gross = ddln_rec.sale_price * local_rec.qty;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		SI.extend = local_rec.extend;
		line_display ();
		CalcTotal ();
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Price Input.	|
	-----------------------*/
	if (LCHECK ("sale_price")) 
	{
		if (dflt_used)
		{
			if (SI.pricing_chk == TRUE)
				ddln_rec.sale_price = SI.calc_sprice;
			else
				PriceProcess (local_rec.storeIdx);
			SI.keyed = 0;
			local_rec.uplift = 0.00;
			if (SI.land_cst)
				local_rec.uplift = ((ddln_rec.sale_price
								 / ddhr_rec.exch_rate)
								 - SI.land_cst) 
								 / SI.land_cst;
			local_rec.uplift *= 100.00;
			local_rec.uplift = twodec (local_rec.uplift);
			SI.uplift = local_rec.uplift;
			SI.sale_price = ddln_rec.sale_price;
		}
		else
			SI.keyed = 2;

		if (ddln_rec.sale_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess031),"YyNn",1,2);
			box (0, 2, 132, 2);
			SillyBusyFunc (0);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		if (SI.tax_amt == 0.00)
			SI.tax_amt = SI.land_cst;

		MarginOK (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   SI.qty,
				   SI.cont_status,
				   SI.land_cst, 
				   SI.min_marg,
				   FALSE);


		if (SI.land_cst)
			local_rec.uplift = ((ddln_rec.sale_price
							 / ddhr_rec.exch_rate)
							 - SI.land_cst) 
							 / SI.land_cst;
		else
			local_rec.uplift = 0.00;

		local_rec.uplift *= 100.00;
		local_rec.uplift = twodec (local_rec.uplift);
		SI.uplift = local_rec.uplift;
		
		temp_gross = ddln_rec.sale_price * local_rec.qty;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		SI.extend = local_rec.extend;
		SI.sale_price = ddln_rec.sale_price;
		line_display ();
		CalcTotal ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sale_disc")) 
	{
		if (FLD ("sale_disc") == NI && prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (SI.dis_or, "N");
			DiscProcess (local_rec.storeIdx);
		}
		
		if (SI.con_price || SI.cont_status == 2)
		{
			ddln_rec.disc_pc 	=	0.00;
			SI.disc_a			=	0.00;
			SI.disc_b			=	0.00;
			SI.disc_c			=	0.00;
			DSP_FLD ("sale_disc");
		}
		SI.dis_pc = ScreenDisc (ddln_rec.disc_pc);

		if (SI.calc_disc != ScreenDisc (ddln_rec.disc_pc))
			strcpy (SI.dis_or, "Y");

		/*------------------------------
		| Discount has been entered so |
		| set disc B & C to zero.      |
		------------------------------*/
		if (!dflt_used)
		{
			SI.disc_a = SI.dis_pc;
			SI.disc_b = 0.00;
			SI.disc_c = 0.00;
		}
		MarginOK (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   SI.qty,
				   SI.cont_status,
				   SI.land_cst, 
				   SI.min_marg,
				   FALSE);

		temp_gross = ddln_rec.sale_price * local_rec.qty;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		SI.extend = local_rec.extend;
		line_display ();
		CalcTotal ();
	}


	/*-----------------------
	| Validate Item Number. | 
	-----------------------*/
	if (LCHECK ("item_no"))
	{
		if (newEntry)
		{
			local_rec.storeIdx = storeMax++;
			newEntry = FALSE;
		}

		if (last_char == EOI)
		{
			storeMax--;
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
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");

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
			SI.tax_pc = 0.00;
			SI.gst_pc = 0.00;
			SI.tax_amt = 0.00;
		}
		else
		{
			SI.tax_pc = inmr_rec.tax_pc;
			SI.gst_pc = inmr_rec.gst_pc;
			SI.tax_amt = inmr_rec.tax_amount;
		}

		SI.hhbr_hash = inmr_rec.hhbr_hash;
		SI.hhum_hash = inmr_rec.std_uom;
		SI.hhsu_hash = sumr_rec.hhsu_hash;
		SI.hhcc_hash = ccmr_rec.hhcc_hash;
		SI.hhds_hash = ddsh_rec.hhds_hash;
		SI.hhdl_hash = -1;
		SI.hhpl_hash = -1;
		SI.line_num = -1;
		strcpy (SI.ship_no, ddsh_rec.ship_no);

		if (check_class (inmr_rec.inmr_class))
		{
			sprintf (err_str,ML (mlDdMess030),inmr_rec.item_no, inmr_rec.inmr_class);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (CheckReorder (inmr_rec.inmr_class))
		{
			sprintf (err_str,ML (mlDdMess029),inmr_rec.item_no, inmr_rec.inmr_class, clip (envVarPoReorder));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (SI.item_no, 	inmr_rec.item_no);
		strcpy (SI.item_desc, 	inmr_rec.description);
		strcpy (SI.sellgrp, 	inmr_rec.sellgrp);
		DSP_FLD ("item_no");
		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		SI.outer = (double) inmr_rec.outer_size;

		inei_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						  inmr_rec.hhsi_hash);
		strcpy (inei_rec.est_no, comm_rec.est_no);
		if (find_rec (inei,&inei_rec,EQUAL,"r"))
		{
			if (AddInei ())
			{
				/*errmess ("Error adding branch record.");*/
				errmess (ML (mlDdMess089));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		strcpy (SI.category, inmr_rec.category);
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,SI.category);
		cc = find_rec (excf,&excf_rec,EQUAL,"r");
		if (cc)
		{
			/*sprintf (err_str," Cannot find Category %s on file ",SI.category);*/
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SI.min_marg = twodec (excf_rec.min_marg);

		/*----------------------------------------
		| Find part number for warehouse record. |
		----------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,EQUAL,"r");
		if (cc) 
		{
			i = prmptmsg (ML (mlStdMess033),"YyNn",1,20);
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
					file_err (cc, "incc", "DBADD");
				move (0,20);
				cl_line ();
				line (131);
			}
		}
		SI.hhcc_hash = incc_rec.hhcc_hash;

		/*------------------
		| Get inis details |
		------------------*/

		if (FindInis (SI.hhbr_hash, SI.hhsu_hash))
		{
			/* inis *not* found! */
			SI.no_inis 		= TRUE;
			SI.volume 		= 0.00;
			SI.supp_lead 	= 0.00;
			SI.min_order 	= 0;
			SI.ord_multiple	= 0;

			strcpy (SI.duty_code, inmr_rec.duty);
			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc) 
				file_err (cc, "inei", "DBFIND");

			if (SI.cont_cost == FALSE)
			{
				SI.base_cst = inei_rec.last_cost * SI.supp_exch;
				SI.grs_fgn  = inei_rec.last_cost * SI.supp_exch;
			}
			
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (SI.supp_uom, inum_rec.uom);
				SI.supp_conv = 1.00;
			}
		}
		else
		{
			SI.no_inis 		= FALSE;
			SI.weight 		= inis_rec.weight;
			SI.volume 		= inis_rec.volume;
			SI.supp_lead 	= inis_rec.lead_time;
			SI.min_order 	= inis_rec.min_order;
			SI.ord_multiple	= inis_rec.ord_multiple;

			strcpy (SI.duty_code, inis_rec.duty);
			if (SI.cont_cost == FALSE)
			{
				SI.base_cst = inis_rec.fob_cost;
				SI.grs_fgn  = inis_rec.fob_cost;
			}
			
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (SI.supp_uom, inum_rec.uom);
				SI.supp_conv = inis_rec.pur_conv;
			}
		}
		SI.upd_inis 		= FALSE;
		strcpy (SI.dis_or, "N");

		/*-------------------
		| Find duty record. |
		-------------------*/
		if (!strcmp (SI.duty_code, "  "))
		{
			SI.imp_duty = 0.00;
			strcpy (SI.duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code,  SI.duty_code);
			cc = find_rec (podt, &podt_rec, EQUAL, "r");
			if (cc)
			{
				/*sprintf (err_str," Duty Code %s Not on File ", SI.duty_code);*/
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			SI.imp_duty = podt_rec.im_duty;
			strcpy (SI.duty_type, podt_rec.duty_type);
		}

		SI.supp_exch = pohr_rec.curr_rate;
		SI.upd_inis = FALSE;
		if (local_rec.fob_cost == 0.00)
			local_rec.fob_cost = SI.grs_fgn;
		else
			SI.grs_fgn  = local_rec.fob_cost;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (SI.std_uom, "    ");
		else
			strcpy (SI.std_uom, inum_rec.uom);
		strcpy (local_rec.std_uom, SI.std_uom);

		DSP_FLD ("cst_per");

		tab_other (line_cnt);
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
			strcpy (local_rec.cst_curr, envVarCurrCode);
	
		if (strcmp (local_rec.cst_curr,"   "))
		{
			cc = FindPocr (local_rec.cst_curr);
			if (cc)
			{
				/*sprintf (err_str,"Currency Code %s is not on File.",local_rec.cst_curr);*/
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
			strcpy (local_rec.cst_curr, envVarCurrCode);
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
					/*sprintf (err_str,"Currency Code %s is not on File.",local_rec.cst_curr);*/
					errmess (ML (mlStdMess040));
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}

				local_rec.cst_exch = pocr_rec.ex1_factor;
			}
			else
			{
				strcpy (local_rec.cst_curr, envVarCurrCode);
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
			strcpy (local_rec.cst_curr, envVarCurrCode);
			DSP_FLD ("currency");
		}

		local_rec.cst_loc_val = local_rec.cst_fgn_val;
		if (local_rec.cst_exch != 0.00)
			local_rec.cst_loc_val /= local_rec.cst_exch;

		DSP_FLD ("currency");
		DSP_FLD ("loc_val");

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

			strcpy (ddhr_rec.del_name, cudi_rec.name);
			strcpy (ddhr_rec.del_add1, cudi_rec.adr1);
			strcpy (ddhr_rec.del_add2, cudi_rec.adr2);
			strcpy (ddhr_rec.del_add3, cudi_rec.adr3);
		}
		CatIntoPohr ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("store_idx2")) 
	{
		if (prog_status == ENTRY)
			newEntry = TRUE;
		return 0;
	}
	if (LCHECK ("invoice_no")) 
	{
		if (dflt_used)
			strcpy (local_rec.inv_no, "00000000");

		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}




/*===============================
|								|
|	FILE UPDATE FUNCTIONS		|
|								|
===============================*/
int
SelectShipment (
 char*      thisShip,
 long   due_date)
{
	int				line_sav;
	struct SHIP_PTR	*lcl_ptr;

	line_sav = line_cnt;
	putval (line_cnt);
	if (ship_head == SHIP_NULL 
	|| (!strcmp (ship_head->ship_no, thisShip) && ship_head->next == SHIP_NULL))
	{
		strcpy (temp_str, "  ");
		return (EXIT_SUCCESS);
	}

	lcl_ptr = ship_head;

	work_open ();
	save_rec ("#Ship No", "# Vessel               Method  Due Date  ");
	
	sprintf (err_str, " New Shipment           %1.1s               ",
			 sumr_rec.ship_method);
	cc = save_rec ("  ", err_str);
	if (cc)
		return (cc);
	
	while (lcl_ptr != SHIP_NULL)
	{
		if (strcmp (thisShip, lcl_ptr->ship_no)
		&&  (due_date == 0L || due_date == lcl_ptr->due_date))
		{
			sprintf (err_str, " %-20.20s   %1.1s    %-10.10s ",
					 lcl_ptr->vessel,
					 lcl_ptr->ship_meth,
					 DateToString (lcl_ptr->due_date));
			cc = save_rec (lcl_ptr->ship_no, err_str);
			if (cc)
				break;
		}
		lcl_ptr = lcl_ptr->next;
	}
	cc = disp_srch ();
	work_close ();
	heading (ORDSCN);
	scn_display (ORDSCN);
	line_cnt = line_sav;
	getval (line_cnt);
	return (cc);
}


void
GetNextShipNo (
 char*  shipNo)
{
	int				shipNum;
	struct SHIP_PTR	*lcl_ptr;

	abc_selfield (ddsh, "ddsh_hhds_hash");

	lcl_ptr = ship_head;

	while (lcl_ptr != SHIP_NULL)
	{
		if (strcmp (lcl_ptr->ship_no,shipNo) > 0)
			strcpy (shipNo, lcl_ptr->ship_no);
		cc = find_rec (ddsh, &ddsh2_rec, NEXT, "r");
		lcl_ptr = lcl_ptr->next;
	}
	shipNum = atoi (shipNo);
	shipNum++;
	sprintf (shipNo, "%2d", shipNum);
}



long
GetDueDate (
 void)
{
	char	due_date [11];
	long	ldue_date;

	/*-----------------------------------------------
	| Only the date characters are acceptable here. |
	-----------------------------------------------*/
	while (TRUE)
	{
		/*rv_pr ("Enter due date for new shipment : ", 1, 2);*/
		rv_pr (ML (mlDdMess026), 1, 2, 1);
		if (FullYear ())
			get_date  (36, 2, "DD/DD/DDDD", due_date);
		else
			get_date  (36, 2, "DD/DD/DD", due_date);

		dflt_used = (strlen (due_date) == 0);
		ldue_date = StringToDate (due_date);
		if 	 (ldue_date < 0 && !dflt_used)
		{
			/*print_mess ("Invalid due date\007");*/
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}
	
		if (last_char != '\r')
		{
			/*print_mess ("A valid due date must be entered.\007");*/
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}

		if (dflt_used)
			ldue_date = TodaysDate ();
   	 	
		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (ldue_date < TodaysDate ())
		{
			cc = prmptmsg (ML (mlDdMess072), "YyNn",1,2);
			if (cc != 'Y' && cc != 'y') 
				continue;
			else
				return (ldue_date);
		}
		else
			return (ldue_date);
	}
}

int
AddToShipment (
 int        idx,
 float      qty,
 char*      thisShip,
 long   due_date)
{
	int				key;
	int				gotShip;
	struct SHIP_PTR	*lcl_ptr;

	gotShip = FALSE;

	while (gotShip == FALSE)
	{
		cc = SelectShipment (thisShip, due_date);
		if (cc)
			return (EXIT_FAILURE);
		if (strcmp (temp_str, "  "))
		{
			lcl_ptr = ship_head;
	
			while (lcl_ptr != SHIP_NULL)
			{
				if (!strcmp (lcl_ptr->ship_no, temp_str))
				{
					ship_curr = lcl_ptr;
					break;
				}
				lcl_ptr = lcl_ptr->next;
			}

			if (!strcmp (ship_curr->status, CONFIRMFLAG))
			{
				/*sprintf (err_str,"\007 Shipment %s is already confirmed.",
						clip (ship_curr->ship_no));*/
				print_mess (ML (mlDdMess005));
				sleep (sleepTime);
				clear_mess ();
				continue;
			}

			if (!strcmp (ship_curr->status, DESPATCHFLAG))
			{
				/*sprintf (err_str,"\007 Shipment %s is already despatched.",
						clip (ship_curr->ship_no));*/
				print_mess (ML (mlDdMess006));
				sleep (sleepTime);
				clear_mess ();
				continue;
			}

			if (ship_curr->costed [0] == 'Y')
			{
				sprintf (err_str,ML (mlDdMess010),clip (ship_curr->ship_no));
				key = prmptmsg (err_str, "YyNn",1,2);
				if (key == 'Y' || key == 'y')
					gotShip = TRUE;
			}
			else
				gotShip = TRUE;
		}
		else
		{
			GetNextShipNo (temp_str);
			if (ship_head == SHIP_NULL)
			{
				lcl_ptr = ShipAlloc ();
				lcl_ptr->next = SHIP_NULL;
				ship_head = lcl_ptr;
				ship_curr = ship_head;
				strcpy (ship_curr->ship_no, temp_str);
			}
			else
				ship_curr = AddToShipList (temp_str, ship_curr);
	
			if (due_date == 0L)
				ship_curr->due_date  = GetDueDate ();
			else
				ship_curr->due_date  = due_date;
			ship_curr->hhds_hash = -1;
			strcpy (ship_curr->ship_meth, sumr_rec.ship_method);
			sprintf (ship_curr->vessel, "Shipment %2s                  ",
					 ship_curr->ship_no);
			strcpy (ship_curr->costed, "N");
			strcpy (ship_curr->status, "P");
			gotShip = TRUE;
	
			box (0, 2, 132, 2);

		}
	}
	memcpy (&store [storeMax], &store [idx], sizeof (struct storeRec));
	strcpy (store [storeMax].ship_no, ship_curr->ship_no);
	store [storeMax].hhpl_hash = -1;
	store [storeMax].hhdl_hash = -1;
	store [storeMax].hhds_hash = ship_curr->hhds_hash;
	store [storeMax].qty 	  = store [idx].qty - qty;
	store [storeMax].due_date  = ship_curr->due_date;
	storeMax++;
	return (EXIT_SUCCESS);
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

/*===================================
| Update Inventory Supplier Record. |
===================================*/
void
UpdateInis (
 double     upd_value,
 float      upd_vol)
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
		inis_rec.lcost_date = StringToDate (local_rec.systemDate);
		if (inis_rec.volume == 0 && upd_vol != 0)
			inis_rec.volume = upd_vol;

		cc = abc_update (inis,&inis_rec);
		if (cc)
			file_err (cc, "inis", "DBUPDATE");
	}
	else
		abc_unlock (inis);
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
	cc = abc_add ("inei",&inei_rec);
	if (cc) 
		return (EXIT_FAILURE);

	return (find_rec (inei,&inei_rec,EQUAL,"r"));
}


int
CheckReorder (
 char*  item_class)
{
	if (strchr (envVarPoReorder, item_class [0]) == (char *) 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

int
CheckFehrTotals (
 char*  updateFlag)
{
	int			i;
	double		newTotal = 0.00;
	double		oldTotal = 0.00;
	double		shpTotal = 0.00;

	newTotal = 0.00;
	oldTotal = 0.00;
	shpTotal = 0.00;

	if (!strcmp (ddhr_rec.fwd_exch, "      "))
		return (EXIT_SUCCESS);

	strcpy (fehr_rec.co_no, comm_rec.co_no);
	strcpy (fehr_rec.cont_no, ddhr_rec.fwd_exch);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, fehr, "DBFIND");
			
	strcpy (feln_rec.index_by, "D");
	feln_rec.index_hash = ddhr_rec.hhdd_hash;
	cc = find_rec (feln, &feln_rec, EQUAL,"u");
	if (cc)
		oldTotal = 0.00;
	else
		oldTotal = feln_rec.value;

	ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	ddln_rec.line_no = 0;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
	while (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash)
	{
		if (ddln_rec.stat_flag [0] != 'I'
		&&  ddln_rec.hhds_hash  != ddsh_rec.hhds_hash)
		{
			newTotal += ddln_rec.gross;
		}
		cc = find_rec (ddln, &ddln_rec, NEXT, "r"); 
	}

	for (i = 0; i < storeMax; i++)
	{
		CalcExtend (i);
		newTotal += l_total;
		shpTotal += l_total;
	}

	if (strcmp (updateFlag, DELETEFLAG))
	{
		if (newTotal > (fehr_rec.val_avail + oldTotal))
		{
			/*sprintf (err_str,"\007Forward Exchange Contract amount exceeded. Amount Remaining %f. Order cannot be saved.", DOLLARS (fehr_rec.val_avail + oldTotal));*/
			sprintf (err_str,ML (mlDdMess011), DOLLARS (fehr_rec.val_avail + oldTotal));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		fehr_rec.val_avail += oldTotal;
		fehr_rec.val_avail -= newTotal;
		feln_rec.value      = newTotal;
	}
	else
	{
		fehr_rec.val_avail += oldTotal;
		fehr_rec.val_avail -= (newTotal - shpTotal);
		feln_rec.value      = (newTotal - shpTotal);
	}
	return (EXIT_SUCCESS);
}

int
UpdateOrder (
 char*  updateFlag)
{
	int		key;
	int		count = 0;
	PrntCnt	=	0;

	ord_total 		= 0.00;
	ddhr_rec.gross 	= 0.00;
	ddhr_rec.disc 	= 0.00;
	ddhr_rec.tax 	= 0.00;
	ddhr_rec.gst 	= 0.00;

	if (envVarFeInstall)
	{
		cc = CheckFehrTotals (updateFlag);
		if (cc)
			return (EXIT_FAILURE);
	}

	clear ();
	fflush (stdout);

	/*print_at (0,0," Now Updating Direct Delivery Order %s ...\n\r",ddhr_rec.order_no);*/
	print_at (PrntCnt++,0,ML (mlDdMess019));
	fflush (stdout);

	UpdateDdsh (updateFlag, ddsh_rec.ship_no);

	if (!strcmp (progFlag, DESPATCHFLAG))
	{
		UpdatePogh (updateFlag);
		UpdatePohs (updateFlag);
	}

	/*--------------
	| Process ddlns |
	---------------*/

	/*print_at (0,0,"\n\r Now Updating Direct Delivery Order Lines ... ");*/
	print_at (PrntCnt++,0,ML (mlDdMess020));
	fflush (stdout);

	for (count = 0; count < storeMax; count++) 
	{
	
		UpdatePoln (count, updateFlag);

		UpdateDdln (count, updateFlag);

		if (envVarPoSuHist && !strcmp (progFlag, DESPATCHFLAG))
			UpdateSuph (count, updateFlag);

		if (!strcmp (progFlag, DESPATCHFLAG))
			UpdatePogl (count, updateFlag);
	}

	CalcDdhrTotals ();

	/*-------------------------
	| Delete cancelled order. |
	-------------------------*/
	if (l_lines == 0) 
	{
		/*print_at (0,0,"\n\r\n\r Now Deleting Complete Order ... ");*/
		print_at (PrntCnt++,0,ML (mlDdMess018));
		abc_selfield (pohr, "pohr_hhdd_hash");
		pohr_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
		while (!cc && pohr_rec.hhdd_hash == ddhr_rec.hhdd_hash)
		{
			print_at (PrntCnt++,0,ML (mlDdMess017),pohr_rec.pur_ord_no);
			cc = abc_delete (pohr);
			if (cc)
				file_err (cc, pohr, "DBDELETE");
			cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
		}
		abc_unlock (pohr);

		print_at (PrntCnt++,0,ML (mlDdMess016),ddhr_rec.order_no);
		cc = abc_delete (ddhr);
		if (cc)
			file_err (cc, ddhr, "DBDELETE");

		if (envVarFeInstall && strcmp (ddhr_rec.fwd_exch, "      "))
		{
			cc = abc_update (fehr, &fehr_rec);
			if (cc)
				file_err (cc, fehr, "DBUPDATE");

			cc = abc_delete (feln);
			if (cc)
				file_err (cc, feln, "DBDELETE");
		}

		add_hash (comm_rec.co_no, 
				  comm_rec.est_no,
			  	  "RO", 
				  0, 
				  cumr_rec.hhcu_hash, 
				  0L, 
				  0L, 
				  (double) 0.00);

		recalc_sobg ();
        PauseForKey (PrntCnt++, 0, ML (mlStdMess042), 0);		

	}
	else
	{
		print_at (PrntCnt++,0, "%s" ,ML (mlDdMess012));

		/*---------------------------------------
		| Calc Totals of Gst etc for ddhr		|
		---------------------------------------*/
		cc = find_rec (ddhr, &ddhr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, ddhr, "DBFIND");
	
		ddhr_rec.gross 	= l_total;
		ddhr_rec.disc  	= l_dis;
		ddhr_rec.tax	= l_tax;
		ddhr_rec.gst	= l_gst;
		strcpy (ddhr_rec.inv_no, local_rec.inv_no);
	
		if (allLinesDesp)
			strcpy (ddhr_rec.stat_flag, DESPATCHFLAG);
		else
		if (allLinesConf)
			strcpy (ddhr_rec.stat_flag, CONFIRMFLAG);
		else
			strcpy (ddhr_rec.stat_flag, ACTIVEFLAG);
	
	
		cc = abc_update (ddhr, &ddhr_rec);
		if (cc)
			file_err (cc, ddhr, "DBUPDATE");
	
		if (envVarFeInstall && strcmp (ddhr_rec.fwd_exch, "      "))
		{
			cc = abc_update (fehr, &fehr_rec);
			if (cc)
				file_err (cc, fehr, "DBUPDATE");

			cc = abc_update (feln, &feln_rec);
			if (cc)
				file_err (cc, feln, "DBUPDATE");
		}

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

	
		/*---------------------------------------
		| Does the purchase order have any lines |
		----------------------------------------*/
		abc_selfield (poln, "poln_hhpo_hash");
		poln_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (cc)
		{
			/*-------------------
			| If not delete pohr |
			--------------------*/
			print_at (PrntCnt++,0,ML (mlDdMess017),pohr_rec.pur_ord_no);
			cc = find_rec (pohr, &pohr_rec, EQUAL, "w");
			if (cc) 
				file_err (cc, pohr, "DBFIND");

			cc = abc_delete (pohr);
			if (cc) 
				file_err (cc, pohr, "DBDELETE");

            PauseForKey (PrntCnt++, 0, ML (mlStdMess042), 0);
		}
		else
		{
			if (!strcmp (updateFlag, DESPATCHFLAG))
				UpdatePohr ();

            PauseForKey (PrntCnt++, 0, ML (mlStdMess042), 0);

			if (l_lines > 0 && strcmp (updateFlag, DELETEFLAG))
			{
                key = prmptmsg (ML (mlDdMess013), "YyNn",0, PrntCnt++);
				crsr_off ();
				if (!lpno && (key == 'Y' || key == 'y'))
					lpno = get_lpno (0);
			
				clear ();
				if (lpno && (key == 'Y' || key == 'y'))
				{
					print_at (PrntCnt++,0,ML (mlDdMess015));

					if ((pout = popen ("dd_sup_conf","w")) == 0)
						file_err (errno, "dd_sup_conf", "popen");
					
					fprintf (pout,"%d\n",lpno);
					fprintf (pout,"S\n");
					fprintf (pout,"%010ld", ddsh_rec.hhds_hash);
	
#ifdef GVISION
					Remote_fflush (pout);
#else
					fflush (pout);
#endif
					pclose (pout);
				}
			}
		
		
			if (envVarPoPrint && strcmp (updateFlag, DELETEFLAG))
			{
                char msgBuff [256];
                sprintf (msgBuff, ML (mlDdMess014), pohr_rec.pur_ord_no);
                key = prmptmsg (msgBuff, "YyNn", 0, PrntCnt++);
				crsr_off ();
				if (!lpno && (key == 'Y' || key == 'y'))
					lpno = get_lpno (0);
		
				if (lpno && (key == 'Y' || key == 'y'))
				{
					print_at (PrntCnt++,0,ML (mlDdMess021),pohr_rec.pur_ord_no);

					if ((pout = popen (poPrintProg,"w")) == 0)
						file_err (errno, poPrintProg, "popen");

					fprintf (pout,"%d\n",lpno);
					fprintf (pout,"S\n");
					fprintf (pout,"%ld\n",pohr_rec.hhpo_hash);
#ifdef GVISION
					Remote_fflush (pout);
#else
					fflush (pout);
#endif
					pclose (pout);
					cc = find_rec (pohr, &pohr_rec, NEXT, "r");
				}
			}
		}
	}
	return (EXIT_SUCCESS);
}

void
UpdateDdgd (
 long   hhds_hash,
 char*      updateFlag)
{
	int			cat_idx;

	if (!strcmp (updateFlag, DELETEFLAG))
	{
		strcpy (ddgd_rec.co_no,	comm_rec.co_no);
		ddgd_rec.hhds_hash = hhds_hash;
		ddgd_rec.line_no = 0;
		cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
		while (!cc && ddgd_rec.hhds_hash == hhds_hash)
		{
			cc = abc_delete (ddgd);
			if (cc) 
				file_err (cc, "ddgd", "DBDELETE");
			cc = find_rec (ddgd, &ddgd_rec, GTEQ, "u");
		}
		return;
	}

	for (cat_idx = FOB; cat_idx <= OT4 ; cat_idx++)
	{
		strcpy (ddgd_rec.co_no,	comm_rec.co_no);
		ddgd_rec.hhds_hash = hhds_hash;
		ddgd_rec.line_no = cat_idx;
		cc = find_rec (ddgd, &ddgd_rec, EQUAL, "u");
		if (cc)
		{
			sprintf (ddgd_rec.category,"%-20.20s", 
					 ddgdArray [cat_idx].category);
			sprintf (ddgd_rec.allocation,	"%-1.1s", 
			         ddgdArray [cat_idx].allocation);
			sprintf (ddgd_rec.currency,	"%-3.3s",
			         ddgdArray [cat_idx].currency);
			ddgd_rec.fgn_value 	= 
					 ddgdArray [cat_idx].fgn_value;
			ddgd_rec.exch_rate		= 
					 ddgdArray [cat_idx].exch_rate;
			ddgd_rec.loc_value 	= 
					 ddgdArray [cat_idx].loc_value;
			cc = abc_add (ddgd, &ddgd_rec);
			if (cc) 
				file_err (cc, "ddgd", "DBADD");
		}
		else
		{
			sprintf (ddgd_rec.category,"%-20.20s", ddgdArray [cat_idx].category);
			sprintf (ddgd_rec.allocation,"%-1.1s", ddgdArray [cat_idx].allocation);
			sprintf (ddgd_rec.currency,	"%-3.3s",
			         ddgdArray [cat_idx].currency);
			ddgd_rec.fgn_value 	= 
					 ddgdArray [cat_idx].fgn_value;
			ddgd_rec.exch_rate		= 
					 ddgdArray [cat_idx].exch_rate;
			ddgd_rec.loc_value 	= 
					 ddgdArray [cat_idx].loc_value;
			cc = abc_update (ddgd, &ddgd_rec);
			if (cc) 
				file_err (cc, "ddgd", "DBUPDATE");
		}
	}
}

void
GenGrnNo (
 void)
{
	int		grnExists;

	/*--------------------------
	| Open Branch Master File. |
	--------------------------*/
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	/*---------------------------
	| Read Branch Master Record |
	---------------------------*/
	grnExists = TRUE;
	do
	{
		/*----------------------------
		| Read Branch Master Record. |
		----------------------------*/
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		/*-------------------------------------
		| Increment nx_gr_on and update esmr. |
		-------------------------------------*/
		sprintf (err_str, "%ld", ++esmr_rec.nx_gr_no);
		strcpy (pogh2_rec.gr_no, zero_pad (err_str, 15));
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");

		/*-----------------------------
		| Check for existence of GRN. |
		-----------------------------*/
		strcpy (pogh2_rec.co_no, comm_rec.co_no);
		cc = find_rec (pogh, &pogh2_rec, COMPARISON, "r");
		if (cc)
			grnExists = FALSE;

	} while (grnExists);

	sprintf (err_str, "%ld", esmr_rec.nx_gr_no);
	strcpy (pogh2_rec.gr_no, zero_pad (err_str, 15));
	strcpy (pogh_rec.gr_no, pogh2_rec.gr_no);

	/*---------------------------
	| Close Branch Master File. |
	---------------------------*/
	abc_fclose (esmr);
}

void
UpdatePohs (
 char*  updateFlag)
{
	int				i;
	int				sav_rec = FALSE;
	double			est_cost	=	0;

	if (!strcmp (updateFlag, DESPATCHFLAG))
	{
		i = 0;
		while (TRUE)
		{
			if (i == storeMax)
				break;
			if (!strcmp (ddsh_rec.ship_no, store [i].ship_no)
			&&  store [i].qty > 0.00)
			{
				sav_rec = TRUE;
				est_cost += (store [i].qty * store [i].land_cst);
			}
			i++;
		}
	
		if (i < storeMax)
		{
			strcpy (pohs_rec.co_no, pogh_rec.co_no);
			strcpy (pohs_rec.br_no, pogh_rec.br_no);
			strcpy (pohs_rec.gr_no, pogh_rec.gr_no);
			pohs_rec.hhsu_hash = pogh_rec.hhsu_hash;
			cc = find_rec (pohs, &pohs_rec, EQUAL, "u");
			if (cc)
				pohs_rec.est_cost = 0.00;
			strcpy (pohs_rec.co_no, pogh_rec.co_no);
			strcpy (pohs_rec.br_no, pogh_rec.br_no);
			strcpy (pohs_rec.gr_no, pogh_rec.gr_no);
			pohs_rec.hhsu_hash = pogh_rec.hhsu_hash;
			strcpy (pohs_rec.pur_ord_no, pogh_rec.pur_ord_no);
			pohs_rec.date_receipt = TodaysDate ();
			pohs_rec.date_cost = 0L;
			pohs_rec.est_cost += est_cost;
			pohs_rec.act_cost = 0.00;
			strcpy (pohs_rec.printed, "N");
			strcpy (pohs_rec.stat_flag, "R");
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				if (cc)
				{
					cc = abc_add (pohs, &pohs_rec);
					if (cc)
						file_err (cc, "pohs", "DBADD");
				}
				else
				{
					cc = abc_update (pohs, &pohs_rec);
					if (cc)
						file_err (cc, "pohs", "DBUPDATE");
				}
			}
			else
				abc_unlock (pohs);
		}
	}
}



void
UpdatePogh (
 char*      updateFlag)
{
	int				i;

	if (!strcmp (updateFlag, DESPATCHFLAG))
	{
		i = 0;
		while (TRUE)
		{
			if (i == storeMax)
				break;
			if (!strcmp (ddsh_rec.ship_no, store [i].ship_no)
			&&  store [i].qty > 0.00)
				break;
			else
				i++;
		}

		if (i < storeMax)
		{
			strcpy (pogh_rec.pur_status,"R");
			strcpy (pogh_rec.gl_status, "R");
			pogh_rec.exch_rate = ddhr_rec.exch_rate;
			strcpy (pogh_rec.co_no, comm_rec.co_no);
			strcpy (pogh_rec.br_no, comm_rec.est_no);
			pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
			pogh_rec.hhsh_hash = 0L;
			pogh_rec.hhpo_hash = pohr_rec.hhpo_hash;
			pogh_rec.hhds_hash = ddsh_rec.hhds_hash;
			GenGrnNo ();

			print_at (PrntCnt++,0,ML (mlDdMess022),pogh_rec.gr_no);
			sleep (sleepTime);

			strcpy (pogh_rec.rec_by, "P");
			strcpy (pogh_rec.pur_ord_no, pohr_rec.pur_ord_no);
			pogh_rec.date_raised = comm_rec.inv_date;
			strcpy (pogh_rec.drop_ship, "Y");
		
			cc = abc_add (pogh, &pogh_rec);
			if (cc)
				file_err (cc, "pogh", "DBADD");
			cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "pogh", "DBFIND");
		}
	}
}



void
UpdateDdsh (
 char*      updateFlag,
 char*      thisShip)
{
	int				i;
	int				new_ddsh;
	int				sav_ddsh;
	struct SHIP_PTR	*lcl_ptr;

	abc_selfield (ddsh, "ddsh_hhds_hash");

	lcl_ptr = ship_head;

	while (lcl_ptr != SHIP_NULL)
	{
		ddsh_rec.hhds_hash	=	lcl_ptr->hhds_hash;
		cc = find_rec (ddsh, &ddsh2_rec, EQUAL, "w");
		if (!cc)
			new_ddsh = FALSE;
		else
			new_ddsh = TRUE;

		if (!strcmp (updateFlag, DELETEFLAG)
		&&  !strcmp (thisShip,	 lcl_ptr->ship_no))
		{
			/*print_at (0,0,"\n\r Now Deleting Direct Delivery Shipment %s ... ",
					 lcl_ptr->ship_no);*/
			print_at (PrntCnt++,0,ML (mlDdMess023),lcl_ptr->ship_no);
	
			if (!new_ddsh)
			{
				if (lcl_ptr->costed [0] == 'Y')
					UpdateDdgd (ddsh2_rec.hhds_hash, updateFlag);
				cc = abc_delete (ddsh);
				if (cc)
					file_err (cc, ddsh, "DBDELETE");
			}
			lcl_ptr = lcl_ptr->next;
			continue;
		}

		i = 0;
		sav_ddsh = FALSE;
		while (TRUE)
		{
			if (i == storeMax)
				break;
			if (!strcmp (lcl_ptr->ship_no, store [i].ship_no)
			&&  store [i].qty > 0.00)
			{
				sav_ddsh = TRUE;
				break;
			}
			else
				i++;
		}

		if (i < storeMax)
		{
			if (new_ddsh)
			{
				abc_selfield (ddsh, "ddsh_id_no");

				/*print_at (0,0,"\n\r Now Creating Direct Delivery Shipment %s ... ",
						 lcl_ptr->ship_no);*/
				print_at (PrntCnt++,0,ML (mlDdMess024),lcl_ptr->ship_no);
				fflush  (stdout);
				memcpy (&ddsh2_rec, &ddsh_rec, sizeof (ddsh_rec));
				strcpy (ddsh2_rec.ship_no, lcl_ptr->ship_no);
				ddsh2_rec.due_date = lcl_ptr->due_date;
				ddsh2_rec.date_load   = 0L;
				ddsh2_rec.ship_depart = 0L;
				ddsh2_rec.ship_arrive = 0L;
				strcpy (ddsh2_rec.ship_method, pohr_rec.ship_method);
				strcpy (ddsh2_rec.vessel, 	  lcl_ptr->vessel);
				strcpy (ddsh2_rec.space_book, "          ");
				strcpy (ddsh2_rec.bol_no, 	  "          ");
				strcpy (ddsh2_rec.carrier,
					"                                          ");
				strcpy (ddsh2_rec.airway, twenty_spaces);
				strcpy (ddsh2_rec.con_rel_no, twenty_spaces);
				strcpy (ddsh2_rec.packing,
					"                                         ");
				strcpy (ddsh2_rec.port_orig, "                         ");
				strcpy (ddsh2_rec.dept_orig, twenty_spaces);
				strcpy (ddsh2_rec.port_dsch, "                  ");
				strcpy (ddsh2_rec.port_dest, "                         ");
				strcpy (ddsh2_rec.mark0,"                                 ");
				strcpy (ddsh2_rec.mark1,"                                 ");
				strcpy (ddsh2_rec.mark2,"                                 ");
				strcpy (ddsh2_rec.mark3,"                                 ");
				strcpy (ddsh2_rec.mark4,"                                 ");
				strcpy (ddsh2_rec.cost_flag, "N");
				strcpy (ddsh2_rec.stat_flag, ACTIVEFLAG);
				cc = abc_add (ddsh, &ddsh2_rec);
				if (cc) 
					file_err (cc, ddsh, "DBADD");
		
				cc = find_rec (ddsh, &ddsh2_rec, EQUAL, "r");
				if (cc) 
					file_err (cc, ddsh, err_str);

				abc_selfield (ddsh, "ddsh_hhds_hash");
			}
			else
			{
				/*print_at (0,0,"\n\r Now Updating Direct Delivery Shipment %s ... ",
						 lcl_ptr->ship_no);*/
				print_at (PrntCnt++,0,ML (mlDdMess025),lcl_ptr->ship_no);
				if (!strcmp (lcl_ptr->ship_no, thisShip))
				{
					UpdateDdgd (ddsh_rec.hhds_hash, updateFlag);
					memcpy (&ddsh2_rec, &ddsh_rec, sizeof (ddsh_rec));
					cc = find_rec (ddsh, &ddsh_rec, EQUAL, "w");
					ddsh_rec.due_date = 			local_rec.hr_due_date;
					ddsh2_rec.date_load =			ddsh2_rec.date_load;
					ddsh2_rec.ship_depart =			ddsh2_rec.ship_depart;
					ddsh2_rec.ship_arrive =			ddsh2_rec.ship_arrive;
					strcpy (ddsh_rec.ship_method, 	ddsh2_rec.ship_method);
					strcpy (ddsh_rec.vessel, 		ddsh2_rec.vessel);
					strcpy (ddsh_rec.space_book, 	ddsh2_rec.space_book);
					strcpy (ddsh_rec.bol_no, 		ddsh2_rec.bol_no);
					strcpy (ddsh_rec.carrier, 		ddsh2_rec.carrier);
					strcpy (ddsh_rec.airway, 		ddsh2_rec.airway);
					strcpy (ddsh_rec.con_rel_no, 	ddsh2_rec.con_rel_no);
					strcpy (ddsh_rec.packing, 		ddsh2_rec.packing);
					strcpy (ddsh_rec.port_orig, 	ddsh2_rec.port_orig);
					strcpy (ddsh_rec.dept_orig, 	ddsh2_rec.dept_orig);
					strcpy (ddsh_rec.port_dsch, 	ddsh2_rec.port_dsch);
					strcpy (ddsh_rec.port_dest, 	ddsh2_rec.port_dest);
					strcpy (ddsh_rec.mark0, 		ddsh2_rec.mark0);
					strcpy (ddsh_rec.mark1, 		ddsh2_rec.mark1);
					strcpy (ddsh_rec.mark2, 		ddsh2_rec.mark2);
					strcpy (ddsh_rec.mark3, 		ddsh2_rec.mark3);
					strcpy (ddsh_rec.mark4, 		ddsh2_rec.mark4);
					strcpy (ddsh_rec.cost_flag, 	ddsh2_rec.cost_flag);
					strcpy (ddsh_rec.stat_flag, 	updateFlag);
					cc = abc_update (ddsh, &ddsh_rec);
				}
				else
				{
					cc = abc_update (ddsh, &ddsh2_rec);
				}
				if (cc) 
					file_err (cc, ddsh, "DBUPDATE");
		
			}
			abc_unlock (ddsh);
		}
		else
		if (!new_ddsh &&  !strcmp (thisShip,	 lcl_ptr->ship_no))
		{
			/*print_at (0,0,"\n\r Now Deleting Direct Delivery Shipment %s ... ",
					 lcl_ptr->ship_no);*/
			print_at (PrntCnt++,0,ML (mlDdMess023),lcl_ptr->ship_no);
	
			if (!new_ddsh)
			{
				if (lcl_ptr->costed [0] == 'Y')
					UpdateDdgd (ddsh2_rec.hhds_hash, updateFlag);
				cc = abc_delete (ddsh);
				if (cc)
					file_err (cc, ddsh, "DBDELETE");
			}
			lcl_ptr = lcl_ptr->next;
			continue;
		}
		lcl_ptr = lcl_ptr->next;
	}
	abc_selfield (ddsh, "ddsh_id_no");
}


int
GetNextPolnLine (
 void)
{
	int		lineNo;

	lineNo = 0;

	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = lineNo;
	cc = find_rec (poln, &poln_rec, EQUAL, "r"); 
	while (!cc)
	{
		poln_rec.line_no = ++lineNo;
		cc = find_rec (poln, &poln_rec, EQUAL, "r"); 
	}
	return (lineNo);
}


void
UpdatePohr (
 void)
{
	int		allDespatched = TRUE;

	cc = find_rec (pohr, &pohr_rec, GTEQ, "u");
	if (cc)
		file_err (cc, pohr, "DBFIND");
	abc_selfield (poln, "poln_hhpo_hash");
	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash && 
allDespatched)
	{
		if (strcmp (poln_rec.pur_status, "R"))
			allDespatched = FALSE;
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	abc_selfield (poln, "poln_hhpl_hash");

	if (allDespatched)
	{
		strcpy (pohr_rec.status, "R");
		cc = abc_update (pohr, &pohr_rec);
		if (cc) 
			file_err (cc, pohr, "DBUPDATE");
	}
}

void
UpdatePogl (
 int    count,
 char * updateFlag)
{
	static	int		wsLine	=	0;

	if (!strcmp (ddsh_rec.ship_no, store [count].ship_no) &&  
		store [count].qty > 0.00)
	{
		pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
		pogl_rec.line_no   = wsLine++;
		pogl_rec.hhbr_hash = store [count].hhbr_hash;
		pogl_rec.hhum_hash = store [count].hhum_hash;
		pogl_rec.hhcc_hash = store [count].hhcc_hash;
		pogl_rec.hhpl_hash = store [count].hhpl_hash;
		pogl_rec.hhlc_hash = 0L;
		strcpy (pogl_rec.po_number, pohr_rec.pur_ord_no);
		strcpy (pogl_rec.serial_no, "                         ");
		strcpy (pogl_rec.lot_no, "       ");
		strcpy (pogl_rec.slot_no, "       ");
		pogl_rec.exp_date = 0L;
		strcpy (pogl_rec.location,"          ");
	
		pogl_rec.qty_ord = 0.00;
		pogl_rec.qty_rec = store [count].qty;

		pogl_rec.fob_fgn_cst = DOLLARS (store [count].net_fob 
									  * store [count].outer);
		pogl_rec.frt_ins_cst = DOLLARS (store [count].amt_fai);
	
		pogl_rec.fob_nor_cst = DOLLARS (((store [count].net_fob
									    * store [count].outer)
									    / store [count].supp_exch)
										+ store [count].amt_fai);
		pogl_rec.lcost_load  = DOLLARS (store [count].amt_oth);
		pogl_rec.duty 		 = DOLLARS (store [count].amt_dty);
		pogl_rec.licence 	 = 0;
		pogl_rec.land_cst 	 = DOLLARS (no_dec (store [count].land_cst));

		strcpy (pogl_rec.item_desc,store [count].item_desc);
		strcpy (pogl_rec.cat_code, store [count].category);
	
		pogl_rec.rec_date = comm_rec.inv_date;
		strcpy (pogl_rec.pur_status,"R");
		strcpy (pogl_rec.gl_status, "R");
		strcpy (pogl_rec.stat_flag,"0");

		cc = abc_add (pogl, &pogl_rec);
		if (cc)
			file_err (cc, "pogl", "DBADD");
	}
}

/*===================================
| Updated Supplier History records. |
===================================*/
void
UpdateSuph (
 int    count,
 char*  updateFlag)
{
	if (store [count].qty == 0.00 || !strcmp (updateFlag, DELETEFLAG))
		return;

	if (strcmp (updateFlag, DESPATCHFLAG))
		return;

	strcpy (suph_rec.br_no,	  pohr_rec.br_no);
	suph_rec.hhbr_hash 		= store [count].hhbr_hash;
	suph_rec.hhum_hash 		= store [count].hhum_hash;
	suph_rec.hhcc_hash 		= store [count].hhcc_hash;
	suph_rec.hhsu_hash 		= pohr_rec.hhsu_hash;
	suph_rec.ord_date 		= pohr_rec.date_raised;
	suph_rec.due_date 		= store [count].due_date;
	suph_rec.ord_qty  		= store [count].qty;
	suph_rec.rec_date 		= local_rec.lsystemDate;
	suph_rec.rec_qty  		= store [count].qty;
	suph_rec.net_cost 		= DOLLARS (store [count].net_fob 
							* store [count].outer);
	suph_rec.land_cost 		= DOLLARS (store [count].land_cst);
	strcpy (suph_rec.status, "A");
	strcpy (suph_rec.ship_method, pohr_rec.ship_method);
	suph_rec.ship_no = 0L;

	strcpy (suph_rec.drop_ship, "Y");
	strcpy (suph_rec.grn_no, pogh_rec.gr_no);
	strcpy (suph_rec.po_no , pohr_rec.pur_ord_no);

	cc = abc_add (suph, &suph_rec);
	if (cc)
		file_err (cc, "suph", "DBADD");
}



void
UpdatePoln (
 int    count,
 char*  updateFlag)
{
	int			new_poln;
 
	abc_selfield (poln, "poln_hhpl_hash");

	poln_rec.hhpl_hash	=	store [count].hhpl_hash;
	cc = find_rec (poln, &poln_rec, EQUAL, "u");
	if (cc)
		new_poln = TRUE;
	else
		new_poln = FALSE;

	if (store [count].qty == 0.00 || !strcmp (updateFlag, DELETEFLAG))
	{
		if (!new_poln)
		{
			cc = abc_delete (poln);
			if (cc)
				file_err (cc, "poln", "DBDELETE");
		}
		return;
	}

	if (new_poln)
	{
		abc_selfield (poln, "poln_id_no");
		poln_rec.line_no = GetNextPolnLine ();
		poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	}
	poln_rec.qty_ord = store [count].qty;
	poln_rec.qty_rec = 0.00;

	if (!strcmp (progFlag, DESPATCHFLAG))
		poln_rec.qty_rec = store [count].qty;

	poln_rec.hhbr_hash 	= store [count].hhbr_hash;
	poln_rec.hhum_hash 	= store [count].hhum_hash;
	poln_rec.hhcc_hash 	= store [count].hhcc_hash;
	poln_rec.exch_rate 	= store [count].supp_exch;
	poln_rec.hhlc_hash 	= 0;
	poln_rec.case_no 	= 0;
	poln_rec.due_date 	= store [count].due_date;

	poln_rec.reg_pc 	= store [count].purchDiscs [ 0 ];
	poln_rec.disc_a 	= store [count].purchDiscs [ 1 ];
	poln_rec.disc_b 	= store [count].purchDiscs [ 2 ];
	poln_rec.disc_c 	= store [count].purchDiscs [ 3 ];
	poln_rec.cumulative	= store [count].cumulative;

	if (store [count].supp_conv == 0.00)
		store [count].supp_conv = 1.00;

	poln_rec.grs_fgn_cst = DOLLARS (store [count].grs_fgn * store [count].outer);

	poln_rec.fob_fgn_cst = DOLLARS (store [count].net_fob * store [count].outer);
	
	poln_rec.fob_nor_cst = DOLLARS (((store [count].net_fob
								    * store [count].outer)
								    / store [count].supp_exch)
									+ store [count].amt_fai);
	poln_rec.frt_ins_cst = DOLLARS (store [count].amt_fai);
	poln_rec.duty 		 = DOLLARS (store [count].amt_dty);
	poln_rec.licence 	 = 0;
	poln_rec.lcost_load  = DOLLARS (store [count].amt_oth);
	poln_rec.land_cst 	 = DOLLARS (store [count].land_cst);

	strcpy (poln_rec.item_desc,store [count].item_desc);
	strcpy (poln_rec.cat_code, store [count].category);
	if (!strcmp (updateFlag, DESPATCHFLAG))
		strcpy (poln_rec.pur_status, "R");
	strcpy (poln_rec.stat_flag,"B");

	if (new_poln)
	{
		sprintf (poln_rec.serial_no,  "%25.25s"," ");
		strcpy  (poln_rec.pur_status, "O");
		poln_rec.qty_rec = 0.00;
		cc = abc_add (poln,&poln_rec);
		if (cc) 
			file_err (cc, "poln", "DBADD");

	
		sprintf (err_str, "DBFIND %d", __LINE__);
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (cc) 
			file_err (cc, "poln", err_str);
	}
	else
	{
		/*------------------------
		| Update existing order. |
		------------------------*/
		if (!strcmp (progFlag, DESPATCHFLAG)
		&&  !strcmp (updateFlag, DESPATCHFLAG))
			strcpy  (poln_rec.pur_status, "R");
		poln_rec.line_no = count;
		cc = abc_update (poln, &poln_rec);
		if (cc) 
			file_err (cc, "poln", "DBUPDATE");

	}
	abc_unlock (poln);
	
	store [count].hhpl_hash = poln_rec.hhpl_hash;
}

int
GetNextDdlnLine (
 void)
{
	int		lineNo;

	lineNo = 0;

	ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	ddln_rec.line_no = lineNo;
	cc = find_rec (ddln, &ddln_rec, EQUAL, "r"); 
	while (!cc)
	{
		ddln_rec.line_no = ++lineNo;
		cc = find_rec (ddln, &ddln_rec, EQUAL, "r"); 
	}
	return (lineNo);
}

void
UpdateDdln (
 int    count,
 char*  updateFlag)
{
	int			new_ddln;

	/*--------------
	| new line added
	---------------*/
	if (store [count].hhdl_hash == -1)
	{
		new_ddln = TRUE;
		ddln_rec.line_no = GetNextDdlnLine ();
		ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	}
	else
	{
		ddln_rec.hhdl_hash	=	store [count].hhdl_hash;
		new_ddln = find_rec (ddln2, &ddln_rec, EQUAL, "r");
		if (!new_ddln)
			new_ddln = find_rec (ddln, &ddln_rec, EQUAL, "u"); 

		if (new_ddln)
		{
			ddln_rec.line_no = GetNextDdlnLine ();
			ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		}
	}

	strcpy (inmr_rec.co_no,   comm_rec.co_no);
	strcpy (inmr_rec.item_no, store [count].item_no);
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	if (store [count].qty == 0.00 || !strcmp (updateFlag, DELETEFLAG))
	{
		if (!new_ddln)
		{
			cc = abc_delete (ddln);
			if (cc)
				file_err (cc, "ddln", "DBDELETE");
		}
		return;
	}

	if (store [count].upd_inis)
		UpdateInis (store [count].net_fob * store [count].outer, 
					store [count].volume);
	
	CalcExtend (count);

	strcpy (ddln_rec.item_desc,  store [count].item_desc);
	strcpy (ddln_rec.sman_code,  ddhr_rec.sman_code);
	ddln_rec.hhbr_hash 	= store [count].hhbr_hash;

	ddsh2_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	ddsh2_rec.hhsu_hash = store [count].hhsu_hash;
	strcpy (ddsh2_rec.ship_no, store [count].ship_no);
	cc = find_rec (ddsh, &ddsh2_rec, EQUAL, "w");
	if (cc)
		file_err (cc, ddsh, "DBUPDATE");

	if (store [count].hhds_hash < 0)
		ddln_rec.hhds_hash  = ddsh2_rec.hhds_hash;
	else
		ddln_rec.hhds_hash  = store [count].hhds_hash;

	ddln_rec.hhsu_hash 	= store [count].hhsu_hash;
	ddln_rec.hhpl_hash 	= store [count].hhpl_hash;
	ddln_rec.q_order  	= store [count].qty;
	ddln_rec.cost_price = store [count].land_cst;
	ddln_rec.on_cost	= store [count].amt_fai
						+ store [count].amt_dty
						+ store [count].amt_oth;

	ddln_rec.reg_pc		= store [count].reg_pc;
	ddln_rec.disc_pc	= store [count].dis_pc;
	ddln_rec.gst_pc 	= (notax) ? 0.00 : store [count].gst_pc;
	ddln_rec.tax_pc 	= (float)((notax) ? 0.00 : store [count].tax_pc);
	strcpy (ddln_rec.bonus_flag, "N");

	ddln_rec.gsale_price = store [count].gsale_price;
	ddln_rec.sale_price = store [count].sale_price;
	ddln_rec.gross 		= l_total;
	ddln_rec.amt_disc 	= l_dis;
	ddln_rec.amt_tax 	= l_tax;
	ddln_rec.amt_gst 	= l_gst;

	ddln_rec.due_date = store [count].due_date;
	ddln_rec.req_date = store [count].due_date;

	if (!strcmp (store [count].ship_no, ddsh_rec.ship_no))
		strcpy (ddln_rec.stat_flag, updateFlag);
	else
		strcpy (ddln_rec.stat_flag, ddsh2_rec.stat_flag);

	strcpy (ddln_rec.pack_size, inmr_rec.pack_size);
	ddln_rec.keyed = store [count].keyed;
	ddln_rec.cont_status = store [count].cont_status;

	if (!new_ddln)
	{
		cc = abc_update (ddln, &ddln_rec);
		if (cc)
			file_err (cc, "ddln", "DBUPDATE");
	}
	else 
	{
		ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		cc = abc_add (ddln, &ddln_rec);
		if (cc) 
			file_err (cc, "ddln", "DBADD");
	}
}




/*===============================
|								|
|	PRICING FUNCTIONS			|
|								|
===============================*/
void
PriceProcess (
 int    wsLine)
{
	int			pType;
	float		regPc;
	double		grossPrice;

	store [wsLine].pricing_chk	= FALSE;

	pType = atoi (ddhr_rec.pri_type);
	grossPrice = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  comm_rec.cc_no,
							  cumr_rec.area_code,
							  cumr_rec.class_type,
							  store [wsLine].sellgrp,
							  cumr_rec.curr_code,
							  pType,
							  cumr_rec.disc_code,
							  cnch_rec.exch_type,
							  cumr_rec.hhcu_hash,
							  ccmr_rec.hhcc_hash,
							  store [wsLine].hhbr_hash,
							  store [wsLine].category,
							  cnch_rec.hhch_hash,
							  (envVarSoDoi) ? TodaysDate (): comm_rec.dbt_date,
							  store [wsLine].qty,
							  pocr_rec.ex1_factor,
							  FGN_CURR,
							  &regPc);

	store [wsLine].pricing_chk	= TRUE;

	store [wsLine].calc_sprice = GetCusGprice (grossPrice, regPc);
	if (store [wsLine].keyed == 0)
	{
		store [wsLine].gsale_price 	= 	grossPrice;
		store [wsLine].sale_price 	=	store [wsLine].calc_sprice;
		store [wsLine].reg_pc 		= 	regPc;
		ddln_rec.sale_price 		= 	store [wsLine].calc_sprice;
		store [wsLine].act_sale 		= 	store [wsLine].calc_sprice;
	}
	store [wsLine].con_price 		= (_CON_PRICE) ? TRUE : FALSE;
	store [wsLine].cont_status  		= _cont_status;
}

void
DiscProcess (
 int    wsLine)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (store [wsLine].cont_status == 2 
	|| store [wsLine].con_price)
	{
		ddln_rec.disc_pc		  	= 0.00;
		store [wsLine].dis_pc 		= 0.00;
		store [wsLine].calc_disc 	= 0.00;
		store [wsLine].disc_a		= 0.00;
		store [wsLine].disc_b		= 0.00;
		store [wsLine].disc_c		= 0.00;
		return;
	}

	if (store [wsLine].pricing_chk == FALSE)
		PriceProcess (wsLine);

	pType = atoi (ddhr_rec.pri_type);
	cumDisc		=	GetCusDisc (	comm_rec.co_no,
								comm_rec.est_no,
								ccmr_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								store [wsLine].hhbr_hash,
								store [wsLine].category,
								store [wsLine].sellgrp,
								pType,
								store [wsLine].gsale_price,
								store [wsLine].reg_pc,
								store [wsLine].qty,
								discArray);
							
	if (store [wsLine].dis_or [0] == 'Y')
		return;
	
	store [wsLine].calc_disc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	if (store [wsLine].dis_or [0] == 'N')
	{
		ddln_rec.disc_pc 			=	ScreenDisc (store [wsLine].calc_disc);
		store [wsLine].dis_pc		=	store [wsLine].calc_disc;

		store [wsLine].disc_a 		= 	discArray [0];
		store [wsLine].disc_b 		= 	discArray [1];
		store [wsLine].disc_c		= 	discArray [2];
		store [wsLine].cumulative	= 	cumDisc;

		if (store [wsLine].dflt_disc > ScreenDisc (ddln_rec.disc_pc) &&
		   	store [wsLine].dflt_disc != 0.0)
		{
			ddln_rec.disc_pc 		= 	ScreenDisc (store [wsLine].dflt_disc);
			store [wsLine].calc_disc	=	store [wsLine].dflt_disc;
			store [wsLine].dis_pc	=	store [wsLine].dflt_disc;
			store [wsLine].disc_a 	= 	store [wsLine].dflt_disc;
			store [wsLine].disc_b 	= 	0.00;
			store [wsLine].disc_c 	= 	0.00;
		}
	}
}




/*--------------------------
| Validate margin percent. |
--------------------------*/
void
MarginOK (
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

	if (ddhr_rec.exch_rate != 0.00)
	sales = sales / ddhr_rec.exch_rate;
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
			print_at (2, 0, ML (mlDdMess071));
		else
			print_at (2, 0,ML (mlDdMess071));
		
		PauseForKey (2,75, ML (mlStdMess042), 0);
		box (0, 2, 132, 2);
	}

	local_rec.margin = marg;
	local_rec.marval = CENTS ((wsSales - csale) * wsQty);
	
	return;
}

void
CalcItemTotals (
 long   hhsu_hash)
{
	int		i;
	int		key;
	float	wsFloat;

	cst_tot = 0.00;
	vol_tot = 0.00;
	wgt_tot = 0.00;

	abc_selfield (inmr,"inmr_hhbr_hash");
	for (i = 0; i < storeMax; i++) 
	{
		if (!strcmp (ddsh_rec.ship_no, store [i].ship_no)
		&&  store [i].qty > 0.00)
		{
			if (store [i].volume == 0.00)
			{
				inmr_rec.hhbr_hash	= store [i].hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL,"r");
				if (cc) 
					file_err (cc, inmr, "DBFIND");

				key = prmptmsg (ML (mlStdMess125), "YyNn",1,2);
				if (key != 'Y' && key != 'y')
				{
					print_at (2, 1, "%60.60s", " ");
					sprintf (err_str,ML (mlDdMess073),
							  clip (inmr_rec.item_no));
					rv_pr (err_str, 1, 2, 1);
					wsFloat = getfloat (57, 2, "NNNNNN.NN");
					if (last_char == '\r')
						store [i].volume = wsFloat;
					else
						store [i].volume = 0.00;
				}
				print_at (2, 1, "%60.60s", " ");
			}

			if (store [i].weight == 0.00)
			{
				inmr_rec.hhbr_hash	= store [i].hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL,"r");
				if (cc) 
					file_err (cc, inmr, "DBFIND");

				key = prmptmsg (ML (mlStdMess126), "YyNn",1,2);
				if (key != 'Y' && key != 'y')
				{
					print_at (2, 1, "%60.60s", " ");
					/*sprintf (err_str,"Enter weight for item %s in kilograms ",
							  clip (inmr_rec.item_no));*/
					sprintf (err_str,ML (mlDdMess074),
							  clip (inmr_rec.item_no));
					rv_pr (err_str, 1, 2, 1);
					wsFloat = getfloat (57, 2, "NNNNNN.NN");
					if (last_char == '\r')
						store [i].weight = wsFloat;
					else
						store [i].weight = 0.00;
				}
				print_at (2, 1, "%60.60s", " ");
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

	for (i = 0;i < storeMax; i++) 
	{
		if (!strcmp (ddsh_rec.ship_no, store [i].ship_no)
		&&  store [i].qty > 0.00)
		{
			fob_tot += store [i].net_fob * store [i].qty;
			dty_tot += store [i].amt_dty * store [i].qty;
			fai_tot	+= store [i].amt_fai * store [i].qty;
			oth_tot += store [i].amt_oth * store [i].qty;
		}
	}
}




/*============================
| Calculate total line cost. |
============================*/
void
CalcCost (
 int    wk_line)
{
	double		cif_cost = 0.00;
	double		cif_loc  = 0.00;

	store [wk_line].net_fob = CalcNet (store [wk_line].grs_fgn, 
								store [wk_line].purchDiscs, 
								store [wk_line].cumulative);
	store [wk_line].net_fob = no_dec (store [wk_line].net_fob);
	if (prog_status == ENTRY)
	{
		if (store [wk_line].amt_dty == 0.00)
			store [wk_line].amt_dty = DtyCalc (wk_line);
		
		if (store [wk_line].amt_fai == 0.00)
			store [wk_line].amt_fai = FrtCalc (wk_line);
	}



	/*-------------------
	| Calculate CIF FGN	|
	-------------------*/
	cif_cost = store [wk_line].net_fob * store [wk_line].outer;

	/*-------------------
	| Calculate CIF NZL	|
	-------------------*/
	if (store [wk_line].supp_exch != 0.00)
		cif_loc = cif_cost / store [wk_line].supp_exch;
	else
		cif_loc = cif_cost;

	cif_loc = twodec (cif_loc);
	cif_loc += store [wk_line].amt_fai;
	cif_loc = twodec (cif_loc);
	cif_loc = no_dec (cif_loc);

	/*-------------------------------
	| Calculate Landed Cost NZL	|
	-------------------------------*/
	store [wk_line].land_cst = cif_loc + 
			     		 	  store [wk_line].amt_dty + 
			     		 	  store [wk_line].amt_oth;

	local_rec.land_cst = store [wk_line].land_cst;
	local_rec.duty_val = store [wk_line].amt_dty;
	local_rec.loc_fi   = store [wk_line].amt_fai;
}

void
SpreadCosts (
 long   hhsu_hash)
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
	for (i = 0;i < storeMax; i++) 
	{
		wsFai = 0.00;
		wsDty = 0.00;
		wsOth = 0.00;

		if (!strcmp (ddsh_rec.ship_no, store [i].ship_no)
		&&  store [i].qty > 0.00)
		{
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
					case FRT : wsFai += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case INS : wsFai += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case INTEREST : 
                               wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case CHG : wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case DTY : wsDty += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case OT1 : wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case OT2 : wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case OT3 : wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
					case OT4 : wsOth += (local_rec.cst_loc_val* (wsAmt / wsTot));
							   break;
				}
				ddgdArray [lcount [CSTSCN]].line_no
					= lcount [CSTSCN];
				sprintf (ddgdArray [lcount [CSTSCN]].category,
					"%-20.20s", local_rec.cst_category);
				sprintf (ddgdArray [lcount [CSTSCN]].allocation,
					"%-1.1s", 
						 local_rec.cst_spread);	
				sprintf (ddgdArray [lcount [CSTSCN]].currency,
					"%-3.3s", local_rec.cst_curr);
				ddgdArray [lcount [CSTSCN]].fgn_value 
					= local_rec.cst_fgn_val;
				ddgdArray [lcount [CSTSCN]].exch_rate 
					= local_rec.cst_exch;
				ddgdArray [lcount [CSTSCN]].loc_value 
					= local_rec.cst_loc_val;
			}
			store [i].amt_fai = wsFai / store [i].qty;
			store [i].amt_dty = wsDty / store [i].qty;
			store [i].amt_oth = wsOth / store [i].qty;
			CalcCost (i);
			scn_set (CSTSCN);
		}
	}
	strcpy (ddsh_rec.cost_flag, "Y");
	LoadIntoOrdScn ();

	if (cur_screen != sav_screen)
		scn_set (sav_screen);
	
	line_cnt = line_sav;
	getval (line_cnt);
}



/*====================
| Calculate Freight. |
====================*/
double
FrtCalc (
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
		value = (store [wk_line].net_fob * store [wk_line].outer);
		value = (value * frt_conv) / 100;
	}

	if (local_rec.supp_exch != 0.00)
		value /= store [wk_line].supp_exch;

	value = no_dec (value);

	return (value);
}

/*==================================================
| Calculate Duty on total quantity and each basis. |
==================================================*/
double
DtyCalc (
 int    wk_line)
{
	double	value  = 0.00;

	if (!strcmp (ddhr_rec.sell_terms, "FOB") ||
	    !strcmp (ddhr_rec.sell_terms, "CIF"))
		return (0.00);

	/*-------------------
	| Calculate Duty   	|
	-------------------*/
	if (store [ wk_line ].duty_type [0] == 'D')
		value = store [wk_line].imp_duty;
	else
	{
		value = store [wk_line].net_fob * store [wk_line].outer;

		value *= (store [ wk_line ].imp_duty / 100);
		if (store [ wk_line].supp_exch != 0.00)
			value /= store [ wk_line ].supp_exch;

		value = no_dec (value);
	}
	return (value);
}

void
CalcDdhrTotals (
 void)
{
	abc_selfield (inmr, "inmr_hhbr_hash");

	l_lines = 0.00;
	l_total = 0.00;
	t_total = 0.00;
	l_dis   = 0.00;
	l_tax   = 0.00;
	l_gst   = 0.00;
	allLinesConf	= TRUE;
	allLinesDesp	= TRUE;

	ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	ddln_rec.line_no = 0;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
	while (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash)
	{
		l_lines++;
		l_total += ddln_rec.gross;
		l_dis   += ddln_rec.amt_disc;
		l_tax   += ddln_rec.amt_tax;
		l_gst   += ddln_rec.amt_gst;

		if (strcmp (ddln_rec.stat_flag, CONFIRMFLAG) &&
			strcmp (ddln_rec.stat_flag, DESPATCHFLAG) &&
			strcmp (ddln_rec.stat_flag, INVOICEFLAG))
			allLinesConf = FALSE;

		if (strcmp (ddln_rec.stat_flag, DESPATCHFLAG) &&
			strcmp (ddln_rec.stat_flag, INVOICEFLAG))
			allLinesDesp = FALSE;

		cc = find_rec (ddln, &ddln_rec, NEXT, "r"); 
	}
	abc_selfield (inmr, "inmr_id_no");
}

/*=======================================
| Calculate the unit cost for items	|
| if the outer_size <= 0 then assume	|
| costing is per unit.			|
=======================================*/
double
OutCost (
    double cost,
    float  outer_size)
{
	double		value;

	if (outer_size <= 0.00)
		return (cost);

	value = cost;
	value /= (double) outer_size;
	return (value);
}

void
CalcExtend (
 int    line_no)
{
	
	/*-----------------------------------------------
	| Update ddln gross tax for each line. |
	-----------------------------------------------*/
		
	l_total = (double) store [line_no].qty;
	l_total *= OutCost (store [line_no].sale_price, store [line_no].outer);
	l_total = no_dec (l_total);

	if (notax)
		t_total = 0.00;
	else
	{
		t_total = (double) store [line_no].qty;
		t_total *= OutCost (store [line_no].tax_amt, store [line_no].outer);
		t_total = no_dec (t_total);
	}

	l_dis = l_total * (store [line_no].dis_pc / 100);
	l_dis = no_dec (l_dis);
	
	if (store [line_no].outer > 0.00)
		l_dis = l_dis / store [line_no].outer;

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) (store [line_no].tax_pc);
		l_tax = DOLLARS (l_tax);
		l_tax *= (l_total - l_dis);
		l_tax = no_dec (l_tax);
	}
	
	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) (store [line_no].gst_pc);
		l_gst = DOLLARS (l_gst);
		l_gst *= (l_total - l_dis + l_tax);
	}
	store [line_no].extend = no_dec (l_total - l_dis + l_tax);
}


void
CalcTotal (
 void)
{
	int		i;
	double	wk_gst = 0.00;

	inv_tot = 0.00;
	tax_tot = 0.00;
	tot_tot = 0.00;

	for (i = 0;i < lcount [PRISCN]; i++) 
	{
		inv_tot += store [i].extend;
		tax_tot += store [i].tax_amt;
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

float
RndMltpl (
 float  ord_qty,
 char*  rnd_type,
 float  ord_mltpl,
 float  min_qty)
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


/*===============================
|								|
|	LOAD FUNCTIONS				|
|								|
===============================*/
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
			file_err (cc, "ccmr", "DBFIND");
	}
	else
	{
		if (!hhcc_selected)
		{
			abc_selfield (ccmr,"ccmr_hhcc_hash");
			hhcc_selected = TRUE;
		}
		
		ccmr_rec.hhcc_hash	=	hhccHash;
		cc = find_rec (ccmr,&ccmr_rec,EQUAL,"r");
		if (cc)
		{
			abc_selfield (ccmr,"ccmr_id_no");
			GetWarehouse (0L);
			return;
		}
	}
	return;
}

/*=========================================================================
| Routine to read all ddln records whose hash matches the one on the ddhr |
| and whose shipment number matches that input on header screen.          |
=========================================================================*/
int
LoadItems (
 long   hhdd_hash,
 long   hhds_hash)
{
	struct SHIP_PTR	*lcl_ptr;

	/*--------------------------
	| Set PRISCN - for putval. |
	--------------------------*/
	scn_set (PRISCN);

	storeMax	   = 0;
	lcount [PRISCN] = 0;
	lcount [ORDSCN] = 0;

	SillyBusyFunc (1);
	move (10, 3);

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (ddsh,"ddsh_hhds_hash");

	ddln_rec.hhdd_hash = hhdd_hash;
	ddln_rec.line_no = 0;

	cc = find_rec (ddln,&ddln_rec,GTEQ,"r");

	while (!cc && hhdd_hash == ddln_rec.hhdd_hash) 
	{
		ddsh2_rec.hhds_hash = ddln_rec.hhds_hash;
		cc = find_rec (ddsh, &ddsh2_rec, EQUAL, "u");
		if (!cc)
		{
			if (ddsh2_rec.hhsu_hash == sumr_rec.hhsu_hash)
			{
				/*---------------------------
				| Add shipment to ship list |
				---------------------------*/
				if (ship_head == SHIP_NULL)
				{
					lcl_ptr = ShipAlloc ();
					lcl_ptr->next = SHIP_NULL;
					ship_head = lcl_ptr;
					ship_curr = ship_head;
					strcpy (ship_curr->ship_no, ddsh2_rec.ship_no);
				}
				else
				{
					ship_curr = AddToShipList (ddsh2_rec.ship_no, ship_curr);
				}
	
				ship_curr->due_date  = ddsh2_rec.due_date;
				ship_curr->hhds_hash = ddsh2_rec.hhds_hash;
				strcpy (ship_curr->ship_meth, ddsh2_rec.ship_method);
				strcpy (ship_curr->vessel, ddsh2_rec.vessel);
				strcpy (ship_curr->costed, ddsh2_rec.cost_flag);
				strcpy (ship_curr->status, ddsh2_rec.stat_flag);
			}
		}

		if (ddln_rec.hhds_hash != hhds_hash)
		{
			cc = find_rec (ddln, &ddln_rec, NEXT, "r");
			continue;
		}

		/*------------------
		| Get ddln details |
		------------------*/
		store [storeMax].line_num   = ddln_rec.line_no;
		store [storeMax].hhdl_hash  = ddln_rec.hhdl_hash;
		store [storeMax].hhds_hash  = ddln_rec.hhds_hash;
		store [storeMax].hhbr_hash  = ddln_rec.hhbr_hash;
		store [storeMax].hhsu_hash  = ddln_rec.hhsu_hash;
		store [storeMax].hhpl_hash  = ddln_rec.hhpl_hash;

		store [storeMax].qty          = twodec (ddln_rec.q_order);
		store [storeMax].gsale_price  = ddln_rec.gsale_price;
		store [storeMax].sale_price   = ddln_rec.sale_price;
		store [storeMax].reg_pc       = ddln_rec.reg_pc;
		store [storeMax].disc_a 	  = ddln_rec.disc_a;
		store [storeMax].disc_b 	  = ddln_rec.disc_b;
		store [storeMax].disc_c	 	  = ddln_rec.disc_c;
		store [storeMax].dis_pc       = ddln_rec.disc_pc;
		store [storeMax].gst_pc       = ddln_rec.gst_pc;
		store [storeMax].tax_pc       = ddln_rec.tax_pc;
		store [storeMax].cont_status  = ddln_rec.cont_status;
		store [storeMax].keyed        = ddln_rec.keyed;


		/*------------------
		| Get inmr details |
		------------------*/
		inmr_rec.hhbr_hash	=	ddln_rec.hhbr_hash;
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
				strcpy (store [storeMax].std_uom, inum_rec.uom);
			else
				strcpy (store [storeMax].std_uom, "    ");
		} 
		else
			file_err (cc, inmr, "DBFIND");

		strcpy (store [storeMax].item_no, 	 inmr_rec.item_no);
		strcpy (store [storeMax].sellgrp, 	 inmr_rec.sellgrp);
		strcpy (store [storeMax].item_desc, 	 inmr_rec.description);
		strcpy (store [storeMax].category,     inmr_rec.category);
		strcpy (store [storeMax].cost_flag, 	 inmr_rec.costing_flag);


		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		store [storeMax].outer 	= (double) inmr_rec.outer_size;
		store [storeMax].tax_amt  = inmr_rec.tax_amount;
		store [storeMax].item_class [0] = inmr_rec.inmr_class [0];
		store [storeMax].weight 	= inmr_rec.weight;
		store [storeMax].dflt_disc 	= inmr_rec.disc_pc;
	
		/*------------------
		| Get excf details |
		------------------*/

		strcpy (excf_rec.co_no,  comm_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "excf", "DBFIND");

		store [storeMax].min_marg = twodec (excf_rec.min_marg);
			

		/*------------------
		| Get inis details |
		------------------*/

		if (FindInis (ddln_rec.hhbr_hash, ddln_rec.hhsu_hash))
		{
			/* inis *not* found! */
			store [storeMax].no_inis 		= TRUE;
			store [storeMax].volume 		= 0.00;
			store [storeMax].supp_lead 	= 0.00;
			store [storeMax].min_order 	= 0;
			store [storeMax].ord_multiple	= 0;

			strcpy (store [storeMax].duty_code, inmr_rec.duty);
			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc) 
				file_err (cc, "inei", "DBFIND");

			if (store [storeMax].cont_cost == FALSE)
			{
				store [storeMax].base_cst = inei_rec.last_cost 
											  * store [storeMax].supp_exch;
				store [storeMax].grs_fgn  = inei_rec.last_cost 
											  * store [storeMax].supp_exch;
			}
			
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (store [storeMax].supp_uom, inum_rec.uom);
				store [storeMax].supp_conv = 1.00;
			}
		}
		else
		{
			store [storeMax].no_inis 		= FALSE;
			store [storeMax].weight 		= inis_rec.weight;
			store [storeMax].volume 		= inis_rec.volume;
			store [storeMax].supp_lead 	= inis_rec.lead_time;
			store [storeMax].min_order 	= inis_rec.min_order;
			store [storeMax].ord_multiple	= inis_rec.ord_multiple;

			strcpy (store [storeMax].duty_code, inis_rec.duty);
			if (store [storeMax].cont_cost == FALSE)
			{
				store [storeMax].base_cst = inis_rec.fob_cost;
				store [storeMax].grs_fgn  = inis_rec.fob_cost;
			}
			
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (store [storeMax].supp_uom, inum_rec.uom);
				store [storeMax].supp_conv = inis_rec.pur_conv;
			}
		}
		store [storeMax].upd_inis 		= FALSE;

		/*------------------
		| Get podt details |
		------------------*/

		if (!strlen (clip (store [storeMax].duty_code)))
		{
			store [storeMax].imp_duty = 0.00;
			strcpy (store [storeMax].duty_type, " ");
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code,  store [storeMax].duty_code);
			cc = find_rec (podt, &podt_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "podt", "DBFIND");
	
			store [storeMax].imp_duty = podt_rec.im_duty;
			strcpy (store [storeMax].duty_type, podt_rec.duty_type);
		}

		/*------------------
		| Get poln details |
		------------------*/

		if (LoadPoln (ddln_rec.hhpl_hash))
			return (TRUE);

		CalcCost (storeMax);

		/*------------------------------------------
		| Load any on-cost details if new shipment |
		| otherwise set pointer to on-cost struct  |
		------------------------------------------*/

		ddsh_rec.hhds_hash = ddln_rec.hhds_hash;
		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "u");
		if (!cc)
			strcpy (store [storeMax].ship_no, ddsh_rec.ship_no);



		/*---------------------
		| Get Pricing details |
		---------------------*/

		if (!ddln_rec.cont_status)
		{
			PriceProcess (storeMax);
	
			if (ddln_rec.sale_price == no_dec (store [storeMax].calc_sprice))
				store [storeMax].keyed = 0;
		}

		if (store [storeMax].land_cst)
			store [storeMax].uplift = ((ddln_rec.sale_price
										  /	 ddhr_rec.exch_rate)
										  -	 store [storeMax].land_cst)
										  /  store [storeMax].land_cst;

		store [storeMax].uplift *= 100.00;

		store [storeMax].land_cst = no_dec (local_rec.land_cst);
		
		temp_gross = ddln_rec.sale_price * local_rec.qty;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		
		store [storeMax].extend = local_rec.extend;
		CalcExtend (storeMax);

		/* recalculate margin  */
		MarginOK (ddln_rec.sale_price, 
				   ddln_rec.disc_pc,
				   store [storeMax].qty,
				   store [storeMax].cont_status,
				   store [storeMax].land_cst, 
				   store [storeMax].min_marg, 
				   TRUE);
		/*------------------------------------
		| Load local_rec and save into order |
		| and pricing screens.               |
		------------------------------------*/

		strcpy (local_rec.item_no,   inmr_rec.item_no);
		strcpy (local_rec.item_desc, inmr_rec.item_no);
		sprintf (local_rec.short_desc,"%-20.20s", inmr_rec.item_no);
		strcpy (local_rec.std_uom, 	 store [storeMax].std_uom);
		strcpy (local_rec.view_disc, "N");
		local_rec.qty 			= ddln_rec.q_order;
		local_rec.grs_fgn 		= store [storeMax].grs_fgn;
		local_rec.net_fob 		= store [storeMax].net_fob;
		local_rec.loc_fi 		= store [storeMax].amt_fai;
		local_rec.duty_val 		= store [storeMax].amt_dty;
		local_rec.oth 			= store [storeMax].amt_oth;
		local_rec.land_cst 		= store [storeMax].land_cst;
		local_rec.ln_due_date 	= store [storeMax].due_date;
		local_rec.lcl_cst 		= store [storeMax].land_cst;
		local_rec.uplift 		= store [storeMax].uplift;
		local_rec.extend 		= store [storeMax].extend;
		local_rec.storeIdx 		= storeMax;
	
		ddln_rec.disc_pc = ScreenDisc (ddln_rec.disc_pc);
		scn_set (ORDSCN);
		putval (lcount [PRISCN]);

		scn_set (PRISCN);
		putval (lcount [PRISCN]++);

		storeMax++;

		if (storeMax > MAXLINES) 
			break;

		cc = find_rec (ddln, &ddln_rec, NEXT, "r");
	}

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
 long   hhds_hash)
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
			ddgdArray [i].line_no = ddgd_rec.line_no;
			sprintf (ddgdArray [i].category, "%-20.20s", ddgd_rec.category);
			sprintf (ddgdArray [i].allocation, "%-1.1s", ddgd_rec.allocation);	
			sprintf (ddgdArray [i].currency, "%-3.3s", ddgd_rec.currency);
			ddgdArray [i].fgn_value = ddgd_rec.fgn_value;
			ddgdArray [i].exch_rate = ddgd_rec.exch_rate;
			ddgdArray [i].loc_value = ddgd_rec.loc_value;
		}
		else
	   	{
			ddgdArray [i].line_no = i;
			sprintf (ddgdArray [i].category, "%-20.20s", cat_desc [i]);
			sprintf (ddgdArray [i].allocation, "%-1.1s", " ");	
			sprintf (ddgdArray [i].currency, "%-3.3s", "  ");
			ddgdArray [i].fgn_value = 0.00;
			ddgdArray [i].exch_rate = 1.00;
			ddgdArray [i].loc_value = 0.00;

			if (lcount [CSTSCN] == FOB)
			{
				sprintf (ddgdArray [i].allocation, "%-1.1s", "D");	
				sprintf (ddgdArray [i].currency,"%-3.3s", sumr_rec.curr_code);
				ddgdArray [i].fgn_value = fob_tot;
				ddgdArray [i].exch_rate = pocr_rec.ex1_factor;
				ddgdArray [i].loc_value = ddgdArray [i].fgn_value 
					                    / ddgdArray [i].exch_rate;
			}
			if (lcount [CSTSCN] == FRT)
			{
				sprintf (ddgdArray [i].allocation, "%-1.1s", "D");	
				sprintf (ddgdArray [i].currency, "%-3.3s", envVarCurrCode);
				ddgdArray [i].fgn_value = fai_tot;
				ddgdArray [i].exch_rate = pocr_rec.ex1_factor;
				ddgdArray [i].loc_value = ddgdArray [i].fgn_value 
					                    / ddgdArray [i].exch_rate;
			}
			if (lcount [CSTSCN] == DTY)
			{
				sprintf (ddgdArray [i].allocation, "%-1.1s", "D");	
				sprintf (ddgdArray [i].currency, "%-3.3s", envVarCurrCode);
				ddgdArray [i].fgn_value = dty_tot;
				ddgdArray [i].exch_rate = pocr_rec.ex1_factor;
				ddgdArray [i].loc_value = ddgdArray [i].fgn_value 
					                    / ddgdArray [i].exch_rate;
			}
			if (lcount [CSTSCN] == OT1)
			{
				sprintf (ddgdArray [i].allocation, "%-1.1s", "D");	
				sprintf (ddgdArray [i].currency, "%-3.3s", envVarCurrCode);
				ddgdArray [i].fgn_value = oth_tot;
				ddgdArray [i].exch_rate = pocr_rec.ex1_factor;
				ddgdArray [i].loc_value = ddgdArray [i].fgn_value 
					                    / ddgdArray [i].exch_rate;
			}
		}
	}
}

int
LoadPoln (
 long   hhpl_hash)
{
	int		headerLoaded = FALSE;

	abc_selfield (poln, "poln_hhpl_hash");

	poln_rec.hhpl_hash = hhpl_hash;

	cc = find_rec (poln, &poln_rec, EQUAL, "r");
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

		store [storeMax].supp_exch = pohr_rec.curr_rate;

		store [storeMax].amt_fai 	 	= CENTS (poln_rec.frt_ins_cst);
		store [storeMax].amt_dty 	 	= CENTS (poln_rec.duty);
		store [storeMax].amt_oth 	 	= CENTS (poln_rec.lcost_load);
		store [storeMax].land_cst 		= CENTS (poln_rec.land_cst);
		store [storeMax].hhcc_hash 	= poln_rec.hhcc_hash;
		store [storeMax].grs_fgn	 	= CENTS (poln_rec.grs_fgn_cst);
		store [storeMax].net_fob 		= CENTS (poln_rec.fob_fgn_cst); 
		store [storeMax].purchDiscs [0] = poln_rec.reg_pc;
		store [storeMax].purchDiscs [1] = poln_rec.disc_a;
		store [storeMax].purchDiscs [2] = poln_rec.disc_b;
		store [storeMax].purchDiscs [3]	= poln_rec.disc_c;
		store [storeMax].cumulative   	= poln_rec.cumulative;
		store [storeMax].due_date   	= poln_rec.due_date;

		/*------------------------
		| get contract price 	 |
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
								 store [storeMax].hhbr_hash,
								 store [storeMax].hhcc_hash,
								 cumr_rec.curr_code,
								 cnch_rec.exch_type,
								 FGN_CURR,
								 (float)local_rec.exch_rate);
			if (_cont_status)
			{
				if (cncd_rec.cost > 0.00 && 
					cncd_rec.hhsu_hash == sumr_rec.hhsu_hash)
				{
					store [storeMax].grs_fgn = cncd_rec.cost;
					store [storeMax].base_cst = store [storeMax].grs_fgn;
					store [storeMax].land_cst = store [storeMax].grs_fgn;
					store [storeMax].cont_cost = TRUE;
				}
			}
		}

		cc = find_rec (poln,&poln_rec,NEXT,"r");
	}
	else
		file_err (cc, "poln", "DBFIND");

	return (FALSE);
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
			sprintf (cat_desc [ i ], "%-20.20s", inv_cat [ i ]);
			break;

		case 1:
			sptr = chk_env ("DD_OS_1");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;

		case 2:
			sptr = chk_env ("DD_OS_2");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 3:
			sptr = chk_env ("DD_OS_3");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 4:
			sptr = chk_env ("DD_OS_4");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 5:
			sprintf (cat_desc [ i ], "%-20.20s", inv_cat [ i ]);
			break;
		case 6:
			sptr = chk_env ("DD_OTHER1");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 7:
			sptr = chk_env ("DD_OTHER2");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 8:
			sptr = chk_env ("DD_OTHER3");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;
		case 9:
			sptr = chk_env ("DD_OTHER4");
			if (sptr == (char *)0)
				sprintf (cat_desc [ i ],"%-20.20s",inv_cat [ i ]);
			else
				sprintf (cat_desc [ i ],"%-20.20s",sptr);
			break;

		default:
			break;
		}
	}
}

void
LoadIntoOrdScn (
 void)
{
	int		count;
	int		sav_screen;


	sav_screen = cur_screen;
	if (cur_screen != ORDSCN)
		scn_set (ORDSCN);

	lcount [ORDSCN] = 0;
	for (count = 0; count < storeMax; count++)
	{
		if (store [count].hhds_hash == ddsh_rec.hhds_hash
		&&  store [count].qty > 0)
		{
			strcpy (local_rec.item_no,   store [count].item_no);
			strcpy (local_rec.item_desc, store [count].item_desc);
			sprintf (local_rec.short_desc,"%-20.20s", store [count].item_desc);
			strcpy (local_rec.std_uom, 	 store [count].std_uom);
			strcpy (local_rec.view_disc, "N");
			local_rec.qty 			= store [count].qty;
			local_rec.grs_fgn 		= store [count].grs_fgn;
			local_rec.net_fob 		= store [count].net_fob;
			local_rec.loc_fi 		= store [count].amt_fai;
			local_rec.duty_val 		= store [count].amt_dty;
			local_rec.oth 			= store [count].amt_oth;
			local_rec.land_cst 		= store [count].land_cst;
			local_rec.ln_due_date 	= store [count].due_date;
			local_rec.lcl_cst 		= store [count].land_cst;
			local_rec.uplift 		= store [count].uplift;
			local_rec.extend 		= store [count].extend;
			local_rec.storeIdx 		= count;
			putval (lcount [ORDSCN]++);
		}
	}
	scn_set (sav_screen);
}

void
LoadIntoCstScn (
 void)
{
	int				sav_screen;

	sav_screen = cur_screen;
	init_vars (CSTSCN);	
	scn_set (CSTSCN);

	CalcCostTotals (sumr_rec.hhsu_hash);
	
	for (lcount [CSTSCN] = FOB;lcount [CSTSCN] <= OT4; lcount [CSTSCN]++)
	{
		if (ddsh_rec.cost_flag [0] == 'Y')
	    {
			sprintf (local_rec.cst_category,"%-20.20s", 
					 ddgdArray [lcount [CSTSCN]].category);
			sprintf (local_rec.cst_spread,	"%-1.1s", 
			         ddgdArray [lcount [CSTSCN]].allocation);
			sprintf (local_rec.cst_curr,	"%-3.3s",
			         ddgdArray [lcount [CSTSCN]].currency);
			local_rec.cst_fgn_val 	= 
					 ddgdArray [lcount [CSTSCN]].fgn_value;
			local_rec.cst_exch		= 
					 ddgdArray [lcount [CSTSCN]].exch_rate;
			local_rec.cst_loc_val 	= 
					 ddgdArray [lcount [CSTSCN]].loc_value;
			if (lcount [CSTSCN] == FOB)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", sumr_rec.curr_code);
				local_rec.cst_fgn_val = fob_tot;
				local_rec.cst_exch	  = pohr_rec.curr_rate;
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
				local_rec.cst_exch	  = pohr_rec.curr_rate;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == FRT)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", envVarCurrCode);
				local_rec.cst_fgn_val = fai_tot;
				local_rec.cst_exch	  = 1.000;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == DTY)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", envVarCurrCode);
				local_rec.cst_fgn_val = dty_tot;
				local_rec.cst_exch	  = 1.000;
				local_rec.cst_loc_val = local_rec.cst_fgn_val / local_rec.cst_exch;
			}
			if (lcount [CSTSCN] == OT1)
			{
				sprintf (local_rec.cst_spread,	"%-1.1s", "D");
				sprintf (local_rec.cst_curr,	"%-3.3s", envVarCurrCode);
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

void
LoadIntoPriScn (
 void)
{
	int		idx;
	int		count;


	LoadIntoOrdScn ();

	for (count = 0; count < lcount [ORDSCN]; count++)
	{
		scn_set (ORDSCN);
		getval (count);
		CalcCost (count);
		idx	= local_rec.storeIdx;
		putval (count);
		scn_set (PRISCN);
		getval (count);

		/*------------------------
		| work out new information
		-------------------------*/
		sprintf (local_rec.item_no, 	"%-16.16s", store [idx].item_no);
		sprintf (local_rec.short_desc, 	"%-20.20s", store [idx].item_desc);
		local_rec.qty = store [idx].qty;
		local_rec.land_cst = store [idx].land_cst;
		local_rec.storeIdx = idx;

		/*------------------------
		| get contract price ???
		------------------------*/
		if (cnch_rec.hhch_hash != 0L)
		{
			if (store [idx].keyed != 2)
			{
				ddln_rec.sale_price = ContCusPrice (cnch_rec.hhch_hash,
												   store [idx].hhbr_hash,
												   store [idx].hhcc_hash,
												   cumr_rec.curr_code,
												   cnch_rec.exch_type,
												   FGN_CURR,
												   (float)local_rec.exch_rate);

				if (ddln_rec.sale_price != (double) -1.00)
				{
					if (store [idx].land_cst)
						local_rec.uplift = ((ddln_rec.sale_price /
											ddhr_rec.exch_rate) - 
											store [idx].land_cst) / 
											store [idx].land_cst;

					local_rec.uplift *= 100.00;
					store [idx].uplift = local_rec.uplift;

					store [idx].cont_status = TRUE;
					temp_gross = ddln_rec.sale_price * local_rec.qty;
					temp_gross = no_dec (temp_gross);
					
					temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
					temp_dis = no_dec (temp_dis);
					local_rec.extend = temp_gross - temp_dis;
					store [idx].extend = local_rec.extend;
					CalcExtend (idx);
				}
			}
		}

		/*----------------------
		| if not contract or
		| contract not found
		----------------------*/
		if (ddln_rec.sale_price == (double) -1.00 || cnch_rec.hhch_hash == 0L)
		{
			PriceProcess (idx);
			if (store [idx].keyed == 0)
				local_rec.uplift = 0.00;
			/*-------------------------------------------
			| if _keyed != 2 then means price not edited
			------------------------------------------------*/
			if (store [idx].keyed == 1)
			{
				ddln_rec.sale_price = no_dec (store [idx].land_cst * 
											  ddhr_rec.exch_rate);
				ddln_rec.sale_price *= (1.00 + (store [idx].uplift / 100.00));
				ddln_rec.sale_price = no_dec (ddln_rec.sale_price); 

				local_rec.uplift = store [idx].uplift;
				store [idx].cont_status = FALSE;
				
				temp_gross = ddln_rec.sale_price * local_rec.qty;
				temp_gross = no_dec (temp_gross);
				
				temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
				temp_dis = no_dec (temp_dis);
				local_rec.extend = temp_gross - temp_dis;
				store [idx].extend = local_rec.extend;
				CalcExtend (idx);
			}
			else
			{
				ddln_rec.sale_price = store [idx].sale_price;
				if (store [idx].land_cst)
					local_rec.uplift = ((store [idx].sale_price /
										ddhr_rec.exch_rate) - 
										store [idx].land_cst) / 
										store [idx].land_cst;
				local_rec.uplift *= 100.00;
				store [idx].uplift = local_rec.uplift;
			}
		}

		if (store [idx].dis_or [0] != 'Y')
			DiscProcess (idx);
		
		MarginOK (ddln_rec.sale_price, 
				   ScreenDisc (ddln_rec.disc_pc),
				   store [idx].qty,
				   store [idx].cont_status,
				   store [idx].land_cst, 
				   store [idx].min_marg, 
			   	TRUE);
		temp_gross = ddln_rec.sale_price * local_rec.qty;
		temp_gross = no_dec (temp_gross);
		
		temp_dis = temp_gross * (ScreenDisc (ddln_rec.disc_pc) / 100);
		temp_dis = no_dec (temp_dis);
		local_rec.extend = temp_gross - temp_dis;
		store [idx].extend = local_rec.extend;
		CalcExtend (idx);
		putval (count);
	}
	lcount [PRISCN] = count;
}


void
LoadIntoTlrScn (
 void)
{
	int		i;

	if (new_order && !strcmp (ddhr_rec.del_name, forty_spaces))
	{
		strcpy (ddhr_rec.del_name,   cumr_rec.dbt_name);
		strcpy (ddhr_rec.del_add1, cumr_rec.dl_adr1);
		strcpy (ddhr_rec.del_add2, cumr_rec.dl_adr2);
		strcpy (ddhr_rec.del_add3, cumr_rec.dl_adr3);
	}
	CatIntoPohr ();

	open_rec (exsi,exsi_list,EXSI_NO_FIELDS,"exsi_id_no");

	for (i = 0; i < 3; i++)
	{
		strcpy (exsi_rec.co_no,comm_rec.co_no);
		exsi_rec.inst_code = cumr_inst [i];

		if (!find_rec (exsi,&exsi_rec, EQUAL,"r"))
			sprintf (local_rec.spinst [i],"%-60.60s",exsi_rec.inst_text);
	}
	abc_fclose (exsi);
}



void
CatIntoPohr (
 void)
{
	int		addSize = 0;
	char	address [125];

	sprintf (address, "%124.124s", " ");
	sprintf (pohr_rec.delin1, "SHIP TO : %-40.40s", ddhr_rec.del_name);

	/*------------------------------------------------------------
	| if add 1 + add 2 < 60 put on line 1 and add 3 on line 2    |
	| OR if add 2 + add 3 < 60 put on line 2 and add 1 on line 1 |
	------------------------------------------------------------*/
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

	/*----------------------------------------------
	| ELSE 3 fields of 40 to fit in 2 fields of 60 |
	-----------------------------------------------*/
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




/*===============================
|								|
|	SEARCH AND FIND FUNCTIONS	|
|								|
===============================*/

/*=======================
| Search for currency	|
=======================*/
void
SrchPocr (
 char*  key_val)
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
		file_err (cc, "pocr", "DBFIND");
}



/*==================
| Search for pohr. |
==================*/
void
SrchPohr (
 char*  key_val)
{
	abc_selfield (sumr, "sumr_hhsu_hash");
	abc_selfield (ddhr, "ddhr_hhdd_hash");
	abc_selfield (pohr, "pohr_id_no2");
	work_open ();
	if (FLD ("cust_no") != NA)
	{
		save_rec ("#P/Order Number ", "#Supplier.");
		strcpy (pohr_rec.co_no, comm_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, key_val);
		cc =  find_rec (pohr, &pohr_rec, GTEQ, "r");
		while (!cc && 
			   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
			   !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		   	   !strncmp (pohr_rec.pur_ord_no, key_val, strlen (key_val)))
		{
			if (pohr_rec.hhdd_hash == ddhr_rec.hhdd_hash)
			{
				sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
				if (!cc)
				{
					cc = save_rec (pohr_rec.pur_ord_no, sumr_rec.crd_name);
					if (cc)
						break;
				}
			}
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
		}
	}
	else
	{
		save_rec ("#P/Order Number ", "#D-D Order.");
		strcpy (pohr_rec.co_no, comm_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, key_val);
		cc =  find_rec (pohr, &pohr_rec, GTEQ, "r");
		while (!cc && 
			   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
			   !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		   	   !strncmp (pohr_rec.pur_ord_no, key_val, strlen (key_val)))
		{
			if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash)
			{
				ddhr_rec.hhdd_hash	=	pohr_rec.hhdd_hash;
				cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
				if (!cc)
				{
					if	 (!strcmp (progFlag,	CONFIRMFLAG))
					{
						if (!strcmp (ddhr_rec.stat_flag, ACTIVEFLAG))
						{
							sprintf (err_str, " %-8.8s ", ddhr_rec.order_no);
							cc = save_rec (pohr_rec.pur_ord_no, err_str);
							if (cc)
								break;
						}
					}
					else
					{
						if (!strcmp (ddhr_rec.stat_flag, ACTIVEFLAG) ||
						    !strcmp (ddhr_rec.stat_flag, CONFIRMFLAG))
						{
							sprintf (err_str, " %-8.8s ", ddhr_rec.order_no);
							cc = save_rec (pohr_rec.pur_ord_no, err_str);
							if (cc)
								break;
						}
					}
				}
			}
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
		}
	}
	abc_selfield (pohr, "pohr_id_no");
	abc_selfield (ddhr, "ddhr_id_no");
	abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

/*=======================
| Search for Shipments. |
=======================*/
void
SrchDdsh (
 char*  key_val)
{
	work_open ();
	save_rec ("#Ship No", "#Due Date    ");
	ddsh_rec.hhdd_hash = ddhr_rec.hhdd_hash;
	ddsh_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (ddsh_rec.ship_no, "  ");
	cc = find_rec (ddsh, &ddsh_rec, GTEQ, "r");

	while (!cc && 
		   ddsh_rec.hhdd_hash == ddhr_rec.hhdd_hash &&
		   ddsh_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		strcpy (err_str, DateToString (ddsh_rec.due_date));
		cc = save_rec (ddsh_rec.ship_no, err_str);
		if (cc)
			break;
		cc = find_rec (ddsh, &ddsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}



/*=======================
| Search for DD orders. |
=======================*/
void
SrchDdhr (
 char*  key_val)
{
	int		stat_ok;

	work_open ();
	save_rec ("#Order No", "#Cust Order");
	strcpy  (ddhr_rec.co_no,    comm_rec.co_no);
	strcpy  (ddhr_rec.br_no,    comm_rec.est_no);
	sprintf (ddhr_rec.order_no, "%-8.8s", key_val);
	ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");

	while (!cc && 
		   !strcmp  (ddhr_rec.co_no,    comm_rec.co_no) && 
		   !strcmp  (ddhr_rec.br_no,    comm_rec.est_no) && 
		   !strncmp (ddhr_rec.order_no, key_val, strlen (key_val)))
	{
		if (cumr_rec.hhcu_hash > 0L &&
		     cumr_rec.hhcu_hash != ddhr_rec.hhcu_hash)
			break;
	
		stat_ok = FALSE;

		if (!strcmp (ddhr_rec.stat_flag, ACTIVEFLAG) &&
			!strcmp (progFlag,			 CONFIRMFLAG))
			stat_ok = TRUE;

		if	 (!strcmp (progFlag,			 DESPATCHFLAG) &&
		    (!strcmp (ddhr_rec.stat_flag, CONFIRMFLAG) ||
		     !strcmp (ddhr_rec.stat_flag, ACTIVEFLAG)))
			stat_ok = TRUE;

		if (stat_ok &&
		    (cumr_rec.hhcu_hash == ddhr_rec.hhcu_hash || 
		     cumr_rec.hhcu_hash == 0L))
		{
			cc = save_rec (ddhr_rec.order_no, ddhr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (ddhr_rec.co_no,    comm_rec.co_no);
	strcpy  (ddhr_rec.br_no,    comm_rec.est_no);
	sprintf (ddhr_rec.order_no, "%-8.8s", key_val);
	ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ddhr, "DBFIND");
}


/*==============
 Find inis file
===============*/
int
FindInis (
 long   hhbrHash, 
 long   hhsuHash)
{
	if (hhsuHash > 0L)
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
	else
	{
		inis_rec.hhbr_hash = hhbrHash;
		inis_rec.hhsu_hash = 0L;
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no,"  ");
		cc =  find_rec (inis, &inis_rec, GTEQ, "r");
		if (!cc && inis_rec.hhbr_hash == hhbrHash)
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
FindPocf (
 char*  code)
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

/*===============================
|								|
|	SCREEN DISPLAY FUNCTIONS	|
|								|
===============================*/
int
heading (
 int    scn)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	pr_box_lines (scn);
	tab_col = 0;

	if (!strcmp (progFlag, CONFIRMFLAG))
		rv_pr (ML (mlDdMess027), (132 - strlen (ML (mlDdMess027))) /2, 0, 1);
	else
		rv_pr (ML (mlDdMess028), (132 - strlen (ML (mlDdMess028))) /2, 0, 1);

	switch (scn)
	{
	case	HDRSCN: 
		   rv_pr (ML (mlDdMess075), (132 - strlen (ML (mlDdMess075))) /2, 1, 1);
		   break;
	case	ORDSCN: 
		   rv_pr (ML (mlDdMess076), (132 - strlen (ML (mlDdMess076))) /2, 1, 1);
		   break;
	case	CSTSCN: 
		   rv_pr (ML (mlDdMess077), (132 - strlen (ML (mlDdMess077))) /2, 1, 1);
		   break;
	case	PRISCN:
		   rv_pr (ML (mlDdMess078), (132 - strlen (ML (mlDdMess078))) /2, 1, 1);
		   break;
	case	TLRSCN:
		   rv_pr (ML (mlDdMess079), (132 - strlen (ML (mlDdMess079))) /2, 1, 1);
		   break;
	}

	switch (scn)
	{
	case	HDRSCN:
		use_window (0);
		break;
	case	ORDSCN:
		DispCustSupp (line_cnt);
		break;
	case	CSTSCN:
		DispCustSupp (-1);
		tab_col = 20;
		break;
	case	PRISCN:
		LoadIntoPriScn ();
		CalcTotal ();
		DispCustSupp (line_cnt);
		break;
	case	TLRSCN:
		/*sprintf (err_str, " Customers Consignment Details ");*/
		us_pr (ML (mlDdMess065), (132 - strlen (ML (mlDdMess065))) /2, 2, 1);
		/*sprintf (err_str, " Supplier Shipping Details. ");*/
		us_pr (ML (mlDdMess002), (132 - strlen (ML (mlDdMess002))) /2, 12, 1);
		break;
	case	SHPSCN:
		spec_valid (label ("ship_method"));
		break;
	}

	PrintCoStuff ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
DispCustSupp (
 int    lineNo)
{
	box (0, 2, 132, 2);
	sprintf 
	(
		err_str,  
		ML (mlDdMess041),
		cumr_rec.dbt_no,
		clip (cumr_rec.dbt_name),
		(envVarDbMcurr) ? "(CURR" : "    ", 
		(envVarDbMcurr) ? cumr_rec.curr_code : "   ",
		(envVarDbMcurr) ? ')' : ' ',
		"Pricing Terms : ",
		ddhr_rec.sell_terms,
		"Inv Exch Rate : ",
		local_rec.exch_rate
	);
	print_at (3, 1, "%s", err_str);

	sprintf 
	(
		err_str,  
		ML (mlDdMess042),
		sumr_rec.crd_no,
		clip (sumr_rec.crd_name),
		(envVarDbMcurr) ? "(CURR" : "    ", 
		(envVarDbMcurr) ? sumr_rec.curr_code : "   ",
		(envVarDbMcurr) ? ')' : ' ',
		"Fixed Quote   : ",
		local_rec.exch_desc,
		"P/O Exch Rate : ",
		pohr_rec.curr_rate
	);
	print_at (4, 1, "%s", err_str);
}



/*============================
| Warn user about something. |
============================*/
int
WarnUser (
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
		SillyBusyFunc (0);
		i = prmptmsg (ML (mlDdMess066),"YyNnMm",1,2);
		move (1,2);
		cl_line ();
		SillyBusyFunc (0);
		if (i == 'Y' || i == 'y')
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML (mlDdMess067),"YyNn",1,2);
			heading (HDRSCN);
			scn_display (HDRSCN);
			SillyBusyFunc (0);
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
SillyBusyFunc (
 int    flip)
{
	print_at (2,1,"%-20.20s", (flip) ? ML (mlStdMess035) : " ");
	fflush (stdout);
}

void
PrintCoStuff (void)
{
	line_at (20,0,132);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21,  0, err_str, comm_rec.co_no,comm_rec.co_name);

	strcpy (err_str, ML (mlStdMess039));
	print_at (21, 60, err_str, comm_rec.est_no,comm_rec.est_name);

	strcpy (err_str, ML (mlStdMess099));
	print_at (22, 0, err_str, comm_rec.cc_no,comm_rec.cc_name);

	strcpy (err_str, ML (mlStdMess127));
	print_at (22, 60, err_str,  cudp_rec.dp_no,cudp_rec.dp_short);
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
		&&  line_no != INS
		&&  line_no != FRT)
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
		DispCustSupp (local_rec.storeIdx);
		TabScreen2 (local_rec.storeIdx);
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
		DispCustSupp (line_no);

		if (store [local_rec.storeIdx].cont_status || line_no >= lcount [PRISCN])
		{
			FLD ("sale_price") = NA;
			FLD ("uplift") = NA;
			if (store [local_rec.storeIdx].con_price || store [local_rec.storeIdx].cont_status == 2)
				FLD ("sale_disc") = NA;
			else
				FLD ("sale_disc") = orig_sale_disc;
		}
		else
		{
			/*  if field changed */
			/* see store for doco */
	
			FLD ("sale_disc") = orig_sale_disc;
			if (store [local_rec.storeIdx].keyed == 0)
				return;
	
			if (store [local_rec.storeIdx].keyed == 1)
			{
				FLD ("sale_price") = NA;
				FLD ("uplift") = orig_uplift;
			}
			else
			{
				FLD ("sale_price") = orig_sale;
				FLD ("uplift") = NA;
			}
		}
	}
}




/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
TabScreen2 (
 int    iline)
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
	if (store [iline].cont_cost)
	{
		FLD ("fob_cst")   = NA;
		FLD ("net_fob")   = NA;
		FLD ("view_disc") = NA;
	}
	if (ddsh_rec.cost_flag [0] == 'Y')
	{
		FLD ("loc_fi")		= NA;
		FLD ("duty_val")	= NA;
		FLD ("other")		= NA;
	}
	if (envVarPoMaxLines)
	{
		if (prog_status == ENTRY && iline >= envVarPoMaxLines)
			centre_at (4, 132, ML (mlStdMess158));
		if (prog_status != ENTRY && lcount [ ORDSCN ] > envVarPoMaxLines)
			centre_at (4, 132, ML (mlStdMess158));
	}
	if (store [iline].qty == 0.00)
	{
		move (0, 6);
		cl_line ();
		move (0, 7);
		cl_line ();
		return;
	}
	print_at (6,  0,ML (mlDdMess058));
	print_at (6, 55,ML (mlDdMess059));
	print_at (6, 90,ML (mlDdMess060));
	print_at (7,  0,ML (mlDdMess061));
	print_at (7, 55,ML (mlDdMess062));
	print_at (7, 90,ML (mlDdMess063));

	print_at (6, 11, "%3d", iline + 1);
	if (store [iline].outer > 0.00)
		print_at (6,71, "%-7.1f", store [iline].outer);
	else
		print_at (6,71, "%-7.1f", 1.00);
	print_at (6,111, "%9.4f", store [iline].supp_conv);

	print_at (7, 11, "%-40.40s", store [iline].item_desc);
	print_at (7, 71, "%-4.4s", store [iline].supp_uom);
	print_at (7,111, "%6.1f", store [iline].supp_lead);

	fflush (stdout);

	if (prog_status != ENTRY)
		strcpy (local_rec.supp_uom, store [iline].supp_uom);

	return;
}


/*===============================
|								|
|	VIEW DISCOUNT FUNCTIONS		|
|								|
===============================*/

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
	oldFobFgn =  local_rec.grs_fgn;
	oldDisc [0] = SR.purchDiscs [0];
	oldDisc [1] = SR.purchDiscs [1];
	oldDisc [2] = SR.purchDiscs [2];
	oldDisc [3] = SR.purchDiscs [3];

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
	DispFlds (currFld);

    while ((key = getkey () != FN16))
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
			SR.purchDiscs [0]  = oldDisc [0];
			SR.purchDiscs [1]  = oldDisc [1];
			SR.purchDiscs [2]  = oldDisc [2];
			SR.purchDiscs [3]  = oldDisc [3];
			restart = TRUE;
			break;

		case FN3:
			tmpLineCnt = line_cnt;
			heading (ORDSCN);
			line_cnt = tmpLineCnt;
			lcount [ ORDSCN ] = (prog_status == ENTRY) ? line_cnt + 1 
													  : lcount [ ORDSCN ];
			scn_display (ORDSCN);
			DrawDiscScn ();
			DispFlds (currFld);
			break;
		}

		DispFlds (currFld);
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
	line_at (DBOX_TOP + 2, DBOX_LFT + 1, DBOX_WID - 1);
	move (DBOX_LFT, DBOX_TOP + 2);
	PGCHAR (10);
	move (DBOX_LFT + DBOX_WID - 1, DBOX_TOP + 2);
	PGCHAR (11);

	/*-------------------------------
	| Draw vertical dividing lines. |
	-------------------------------*/
	for (i = 1; i < 6; i++)
		DrawVLine (DBOX_LFT + discScn [i].xPos, DBOX_TOP);

	/*---------------
	| Draw heading. |
	---------------*/
	headXPos = DBOX_LFT + (DBOX_WID - strlen (err_str)) / 2;
	if (SR.cumulative)
		rv_pr (ML (mlDdMess057), headXPos, DBOX_TOP, 1);
	else
		rv_pr (ML (mlDdMess064), headXPos, DBOX_TOP, 1);

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

void
DrawVLine (
 int    xPos,
 int    yPos)
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

void
DispFlds (
 int    rvsField)
{
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [0].xPos + 2,
			 		"%11.2f", DOLLARS (local_rec.grs_fgn));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [1].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.purchDiscs [0]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [2].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.purchDiscs [1]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [3].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.purchDiscs [2]));
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [4].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.purchDiscs [3]));

	local_rec.net_fob = CalcNet (local_rec.grs_fgn, SR.purchDiscs,SR.cumulative);
	SR.net_fob = local_rec.net_fob;
	print_at (DBOX_TOP + 3,DBOX_LFT + discScn [5].xPos + 2,
			 "%11.2f", DOLLARS (local_rec.net_fob));

	/*--------------------------
	| Print highlighted field. |
	--------------------------*/
	switch (rvsField)
	{
	case 0:
		sprintf (err_str, "%11.2f", DOLLARS (local_rec.grs_fgn));
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		sprintf (err_str, "%6.2f", ScreenDisc (SR.purchDiscs [rvsField - 1]));
		break;
	}
	rv_pr (err_str, DBOX_LFT + discScn [rvsField].xPos + 2, DBOX_TOP + 3, 1);
}

void
InputField (
 int    fld)
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
			local_rec.grs_fgn = getmoney (DBOX_LFT + discScn [fld].xPos + 2, 
									  	DBOX_TOP + 3,
									  	discScn [fld].fldMask);

			break;
	
		case 1:
		case 2:
		case 3:
		case 4:
			SR.purchDiscs [fld - 1] = getfloat (DBOX_LFT + discScn [fld].xPos + 2, 
										 	DBOX_TOP + 3,
										 	discScn [fld].fldMask);
			if (SR.purchDiscs [fld - 1] > 99.99)
			{
				/*print_mess ("\007 Discount may not exceed 99.99 ");*/
				print_mess (ML (mlStdMess120));
				sleep (sleepTime);
				clear_mess ();
				fieldOk = FALSE;
			}
			SR.purchDiscs [fld - 1] = ScreenDisc (SR.purchDiscs [fld - 1]);
			break;
		}
	}
	crsr_off ();
}
#endif	/* GVISION */

/*===============================
|								|
|	SHIP LINKED LIST FUNCTIONS	|
|								|
===============================*/


/*------------------------
| Add user to send list. |
------------------------*/
struct SHIP_PTR *
AddToShipList (
 char*              shipNo,
 struct SHIP_PTR*   curr_ptr)
{
	struct	SHIP_PTR	*lcl_ptr;

	/*------------------------
	| Check that user is not |
	| already in the list.   |
	------------------------*/
	lcl_ptr = ship_head;
	while (lcl_ptr != SHIP_NULL)
	{
		if (!strcmp (lcl_ptr->ship_no, shipNo))
			return (curr_ptr);

		lcl_ptr = lcl_ptr->next;
	}

	/*--------------------------------------
	| Allocate memory and set next to null |
	--------------------------------------*/
	lcl_ptr = ShipAlloc ();

	/*------------------------------
	| store user name in send list |
	------------------------------*/
	lcl_ptr->next = curr_ptr->next;
	curr_ptr->next = lcl_ptr;
	curr_ptr = lcl_ptr;
	strcpy (curr_ptr->ship_no, shipNo);
	curr_ptr->due_date = 0L;
	strcpy (curr_ptr->ship_meth, pohr_rec.ship_method);
	strcpy (curr_ptr->vessel, "                             ");

	return (curr_ptr);
}

/*--------------------------------------
| Allocate memory for Ship linked list |
--------------------------------------*/
struct SHIP_PTR*
ShipAlloc (
 void)
{
	struct	SHIP_PTR	*lcl_ptr;

	lcl_ptr = (struct SHIP_PTR *) malloc (sizeof (struct SHIP_PTR));

	if (lcl_ptr == SHIP_NULL)
        	sys_err ("Error allocating memory for ship list During (MALLOC)",errno,PNAME);
		
	return (lcl_ptr);
}

/*------------------
| Clear Ship List. |
------------------*/
int
ClrShipList (
 struct SHIP_PTR*   lcl_head)
{
	struct	SHIP_PTR	*lcl_ptr;
	struct	SHIP_PTR	*tmp_ptr;

	lcl_ptr = lcl_head;
	while (lcl_ptr != SHIP_NULL)
	{
		tmp_ptr = lcl_ptr;
		lcl_ptr = lcl_ptr->next;
		free (tmp_ptr);
	}
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

void
Init_ML (
 void)
{
	strcpy (discScn [0].fldPrompt, ML (mlDdMess101));
	strcpy (discScn [1].fldPrompt, ML (mlDdMess102));
	strcpy (discScn [2].fldPrompt, ML (mlDdMess103));
	strcpy (discScn [3].fldPrompt, ML (mlDdMess104));
	strcpy (discScn [4].fldPrompt, ML (mlDdMess105));
	strcpy (discScn [5].fldPrompt, ML (mlDdMess106));

    s_terms [0]._sterm = strdup (ML (s_terms_const [0]));
    s_terms [1]._sterm = strdup (ML (s_terms_const [1]));
    s_terms [2]._sterm = strdup (ML (s_terms_const [2]));
    s_terms [3]._sterm = strdup (ML (s_terms_const [3]));
    s_terms [4]._sterm = strdup (ML (s_terms_const [4]));

	inv_cat [0] =  strdup (ML (inv_cat_const [0]));
	inv_cat [1] =  strdup (ML (inv_cat_const [1]));
	inv_cat [2] =  strdup (ML (inv_cat_const [2]));
	inv_cat [3] =  strdup (ML (inv_cat_const [3]));
	inv_cat [4] =  strdup (ML (inv_cat_const [4]));
	inv_cat [5] =  strdup (ML (inv_cat_const [5]));
	inv_cat [6] =  strdup (ML (inv_cat_const [6]));
	inv_cat [7] =  strdup (ML (inv_cat_const [7]));
	inv_cat [8] =  strdup (ML (inv_cat_const [8]));
	inv_cat [9] =  strdup (ML (inv_cat_const [9]));
}

void
Destroy_ML (void)
{
    int iIndex;
    
    for (iIndex = 0; iIndex < 5; iIndex++)
    {
        free (s_terms [iIndex]._sterm);
    }

    for (iIndex = 0; iIndex < 10; iIndex++)
    {
        free (inv_cat [iIndex]);
    }
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
		cc = find_rec (cucc,&cucc_rec, (_key == FN14) ? GREATER : LT,"r");

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

/*===============================================
| use_window is a procedure called by scrgen	|
| when FN14 or FN15 is pressed.			        |
| _key is normally the same as last_char	    |
| but by passing it as a parameter it allows	|
| the programmer to do some sneaky things	    |
===============================================*/
int
use_window (
 int    _key)
{
	static	long	lastHhcu;
	char	comment [132];

	/*-----------------------------------------------
	| Only do anything when we are on screen 1 and	|
	| we've read a valid cumr.			|
	-----------------------------------------------*/
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

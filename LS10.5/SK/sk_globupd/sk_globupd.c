/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_globupd.c  )                                  |
|  Program Desc  : ( Global Price Update                          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  inpr, ingp, sumr, inmr, ingu, pocr, esmr, ccmr    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inpr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 20/10/93         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/93)      | Modified  by : Dirk Heinsius.    |
|  Date Modified : (30/09/97)      | Modified  by : Elizabeth D. Paid |
|                :                                                    |
|  (15/11/93)    : HGP 9501 Entry of branch and warehouse numbers     |
|                :          subject to environment (SK_CUSPRI_LVL).   |
|  (30/09/97)    : SEL       - Multilingual Conversion, changed printf|
|                :             to print_at                            |
| $Log: sk_globupd.c,v $
| Revision 5.5  2002/12/01 04:48:15  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.4  2001/11/05 01:40:43  scott
| Updated from Testing.
|
| Revision 5.3  2001/09/24 07:05:33  robert
| updated to make sure dsp_screen_close () is closed when dsp_screen
| is already not needed
|
| Revision 5.2  2001/08/09 09:18:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:05  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:27  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:10:44  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.21  2000/06/15 02:39:04  scott
| Updated to change FindSumr() local routine to IntFindSumr to ensure no
| conflict will exist with new routine FindSumr() that is about to be
| introduced.
|
| Revision 1.20  2000/06/13 05:02:52  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.19  2000/02/01 05:32:21  ramon
| Changed the declaration of cBuffer from cBuffer[256] to cBuffer[512] in SaveAud(). This causes an application error.
|
| Revision 1.18  2000/01/20 18:18:28  cam
| Changes for GVision compatibility.  Made printing of 70 spaces over fields
| conditional code.  Also fixed branch description.
|
| Revision 1.17  1999/12/06 01:30:46  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.16  1999/11/11 05:59:37  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.15  1999/11/03 07:31:59  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.14  1999/10/19 22:13:21  scott
| Updated from ansi testing
|
| Revision 1.13  1999/10/13 02:41:57  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.12  1999/10/12 21:20:32  scott
| Updated by Gerry from ansi project.
|
| Revision 1.11  1999/10/08 05:32:22  scott
| First Pass checkin by Scott.
|
| Revision 1.10  1999/06/20 05:19:58  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
char	*PNAME = "$RCSfile: sk_globupd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_globupd/sk_globupd.c,v 5.5 2002/12/01 04:48:15 scott Exp $";

#define	CCMAIN
#define	QUIT	0
#define	PRINT	1
#define	LIVEUPD	2
#define	CREATE	3
#define	SEL_DELETE	4

#include <pslscr.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	LIVE		(ingu_rec.hhgu_hash == 0L)

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_inv_date"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
	};

	int	comm_no_fields = 15;

	struct	{
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	tcc_no[3];
		long	tinv_date;
		char	price_type[9][16];
	} comm_rec;

	/*======================
	| Inventory Price File |
	======================*/
	struct dbview inpr_list [] =
	{
		{"inpr_hhbr_hash"},
		{"inpr_price_type"},
		{"inpr_br_no"},
		{"inpr_wh_no"},
		{"inpr_curr_code"},
		{"inpr_area_code"},
		{"inpr_cust_type"},
		{"inpr_hhgu_hash"},
		{"inpr_price_by"},
		{"inpr_qty_brk1"},
		{"inpr_qty_brk2"},
		{"inpr_qty_brk3"},
		{"inpr_qty_brk4"},
		{"inpr_qty_brk5"},
		{"inpr_qty_brk6"},
		{"inpr_qty_brk7"},
		{"inpr_qty_brk8"},
		{"inpr_qty_brk9"},
		{"inpr_base"},
		{"inpr_price1"},
		{"inpr_price2"},
		{"inpr_price3"},
		{"inpr_price4"},
		{"inpr_price5"},
		{"inpr_price6"},
		{"inpr_price7"},
		{"inpr_price8"},
		{"inpr_price9"}
	};

	int	inpr_no_fields = 28;

	struct tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	br_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	area_code [3];
		char	cust_type [4];
		long	hhgu_hash;
		char	price_by [2];
		double	qty_brk[9];
		double	price[10];
	} inpr_rec, inpr2_rec;

	/*=====================================
	| Inventory Buying and Selling Groups |
	=====================================*/
	struct dbview ingp_list [] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	int	ingp_no_fields = 5;

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	} ingp_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_name"},
	};

	int	ccmr_no_fields = 4;

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		char	name [41];
	} ccmr_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list [] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	int	esmr_no_fields = 3;

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
	} esmr_rec;

	/*===========================
	| Global Price Update File  |
	===========================*/
	struct dbview ingu_list [] =
	{
		{"ingu_co_no"},
		{"ingu_br_no"},
		{"ingu_wh_no"},
		{"ingu_curr_code"},
		{"ingu_file_code"},
		{"ingu_file_desc"},
		{"ingu_price_type"},
		{"ingu_eff_date"},
		{"ingu_hhgu_hash"},
		{"ingu_apply_to"},
		{"ingu_st_range"},
		{"ingu_end_range"},
		{"ingu_uplift"},
		{"ingu_rounding"},
		{"ingu_price_1"},
		{"ingu_price_2"},
		{"ingu_price_3"},
		{"ingu_price_4"},
		{"ingu_price_5"},
		{"ingu_sig_1"},
		{"ingu_sig_2"},
		{"ingu_sig_3"},
		{"ingu_sig_4"},
		{"ingu_sig_5"},
	};

	int	ingu_no_fields = 24;

	struct tag_inguRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	file_code [7];
		char	file_desc [41];
		int		price_type;
		long	eff_date;
		long	hhgu_hash;
		char	apply_to [2];
		char	st_range [17];
		char	end_range [17];
		float	uplift;
		char	rounding [2];
		double	price [5];
		double	sig [5];
	} ingu_rec;

	/*==================================+
	 | Stock Inventory Supplier Record. |
	 +==================================*/
#define	INIS_NO_FIELDS	26

	struct dbview	inis_list [INIS_NO_FIELDS] =
	{
		{"inis_co_no"},
		{"inis_br_no"},
		{"inis_wh_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_part"},
		{"inis_sup_priority"},
		{"inis_hhis_hash"},
		{"inis_fob_cost"},
		{"inis_lcost_date"},
		{"inis_duty"},
		{"inis_licence"},
		{"inis_sup_uom"},
		{"inis_pur_conv"},
		{"inis_min_order"},
		{"inis_norm_order"},
		{"inis_ord_multiple"},
		{"inis_pallet_size"},
		{"inis_lead_time"},
		{"inis_sea_time"},
		{"inis_air_time"},
		{"inis_lnd_time"},
		{"inis_dflt_lead"},
		{"inis_weight"},
		{"inis_volume"},
		{"inis_stat_flag"}
	};

	struct tag_inisRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_part [17];
		char	sup_priority [3];
		long	hhis_hash;
		double	fob_cost;
		Date	lcost_date;
		char	duty [3];
		char	licence [3];
		long	sup_uom;
		float	pur_conv;
		float	min_order;
		float	norm_order;
		float	ord_multiple;
		float	pallet_size;
		float	lead_time;
		float	sea_time;
		float	air_time;
		float	lnd_time;
		char	dflt_lead [2];
		float	weight;
		float	volume;
		char	stat_flag [2];
	}	inis_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_description"},
		{"inmr_quick_code"},
		{"inmr_buygrp"},
		{"inmr_sellgrp"},
	};

	int	inmr_no_fields = 12;

	struct tag_inmrRecord
	{
		char	mr_co_no [3];
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_alpha_code [17];
		char	mr_super_no [17];
		char	mr_maker_no [17];
		char	mr_alternate [17];
		char	mr_description [41];
		char	mr_quick_code [9];
		char	mr_buygrp[7];
		char	mr_sellgrp[7];
	} inmr_rec;

	/*======================
	| Currency File Record |
	======================*/
	struct dbview pocr_list [] =
	{
		{"pocr_co_no"},
		{"pocr_code"},
		{"pocr_description"},
	};

	int	pocr_no_fields = 3;

	struct tag_pocrRecord
	{
		char	co_no [3];
		char	code [4];
		char	description [41];
	} pocr_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview sumr_list [] =
	{
		{"sumr_hhsu_hash"},
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
	};

	int	sumr_no_fields = 6;

	struct tag_sumrRecord
	{
		long	sm_hhsu_hash;
		char	sm_co_no [3];
		char	sm_est_no [3];
		char	sm_crd_no [7];
		char	sm_name [41];
		char	sm_acronym [10];
	} sumr_rec;

	char	*comm   = "comm",
			*data   = "data",
			*sumr	= "sumr",
			*inmr	= "inmr",
			*inis	= "inis",
			*ccmr	= "ccmr",
			*esmr	= "esmr",
			*pocr	= "pocr",
			*inpr	= "inpr",
			*inpr2	= "inpr2",
			*ingu	= "ingu",
			*ingp   = "ingp";

	char	CURR_CODE[4];
	char	branchNumber[3];

	int		CR_FIND;
	int		C_OWNED;
	int		MCURR = FALSE;
	int		fstTime;
	int		numOfPrices;
	int		numOfBrks;
	int		skPriceLevel;

	FILE	*faud;
	FILE	*fout;
	FILE	*fsort;

	char	QuitStr[] = {"Quit Without Updating "};

MENUTAB file_menu [] =
{
	{ " 1. QUIT                       ", 
	  QuitStr }, 
	{ " 2. PRINT FUTURE PRICES        ", 
	  " Print Future Pricing Structure Based On Stored Information. " }, 
	{ " 3. UPDATE TO LIVE PRICES      ", 
	  " Update Future Prices To Live Prices . " }, 
	{ " 4. UPDATE/CREATE FUTURE PRICES", 
	  " Update/Create Future Prices ." }, 
	{ " 5. DELETE FILE                ", 
	  " Delete Global Pricing File." }, 
	{ ENDMENU }
};

MENUTAB no_file_menu [] =
{
	{ " 1. QUIT                ", 
	  QuitStr }, 
	{ " 2. PRINT LIVE PRICES   ", 
	  " Print Pricing Structure Based On Current Information. " }, 
	{ " 3. UPDATE LIVE PRICES  ", 
	  " Update Live Prices ." }, 
	{ ENDMENU }
};

struct
{
	char	brdesc[44];
	char	whdesc[44];
	char	rangedesc[14];
	char	typedesc[8];
	char	ssellgrp[7];
	char	sselldesc[41];
	char	esellgrp[7];
	char	eselldesc[41];
	char	sbuygrp[7];
	char	sbuydesc[41];
	char	ebuygrp[7];
	char	ebuydesc[41];
	char	sitem[17];
	char	sitemdesc[41];
	char	eitem[17];
	char	eitemdesc[41];
	char	ssupp[7];
	char	ssuppdesc[41];
	char	esupp[7];
	char	esuppdesc[41];
	int		price_type;
	char	price_desc[16];
	int		lpno;
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "price",	3, 24, INTTYPE,
		"N", "          ",
		"", "",  "Price Type           :", "Enter Price Type To Update. ",
		NE, NO, JUSTLEFT, "1", "9", (char *) &local_rec.price_type},
	{1, LIN, "price_desc",3, 34, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.price_desc},
	{1, LIN, "file",	4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "File Name            :", "Enter File Name, Search Available Or Press Enter To Access Live Prices",
		NE, NO, JUSTLEFT, "", "", ingu_rec.file_code},
	{1, LIN, "filedesc",	5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  "File Description     :", "",
		NA, NO, JUSTLEFT, "", "", ingu_rec.file_desc},
	{1, LIN, "curr",	6, 24, CHARTYPE,
		"UUU", "          ",
		" ", " ",  "Currency             :", "Currency For Global Price Update, Default=Lcl Currency, Search Available",
		NA, NO, JUSTLEFT, "", "", ingu_rec.curr_code},
	{1, LIN, "currdesc",	7, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "Currency Description :", "",
		NA, NO, JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "effdate",	8, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ",  "Effective Date       :", "",
		ND, NO, JUSTLEFT, "", "", (char *) &ingu_rec.eff_date},
	{1, LIN, "br",	10, 24, CHARTYPE,
		"NN", "          ",
		" ", " ",  "Branch Number        :", "Default = A)ll, Search Available",
		NA, NO, JUSTRIGHT, "", "", ingu_rec.br_no},
	{1, LIN, "brdesc",	10, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.brdesc},
	{1, LIN, "wh",	11, 24, CHARTYPE,
		"NN", "          ",
		" ", " ",  "Warehouse Number     :", "Default = A)ll, Search Available",
		NA, NO, JUSTRIGHT, "", "", ingu_rec.wh_no},
	{1, LIN, "whdesc",	11, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "Warehouse Number     :", "",
		NA, NO, JUSTLEFT, "", "", local_rec.whdesc},
	{1, LIN, "range",	13, 24, CHARTYPE,
		"U", "          ",
		" ", "S",  "Range Type           :", "Enter Range Type B)uying S)elling P) Supplier I)tem, Default = S)elling",
		NA, NO, JUSTLEFT, "BSPI", "", ingu_rec.apply_to},
	{1, LIN, "rangedesc",	13, 28, CHARTYPE,
		" AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangedesc},
	{1, LIN, "sbuygrp",	14, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Buying Group   :", "Enter Start Buying Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sbuygrp},
	{1, LIN, "sbuydesc",	14, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sbuydesc},
	{1, LIN, "ebuygrp",	15, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Buying Group     :", "Enter End Buying Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ebuygrp},
	{1, LIN, "ebuydesc",	15, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ebuydesc},
	{1, LIN, "ssellgrp",	14, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Selling Group  :", "Enter Start Selling Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssellgrp},
	{1, LIN, "sselldesc",	14, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sselldesc},
	{1, LIN, "esellgrp",	15, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Selling Group    :", "Enter End Selling Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esellgrp},
	{1, LIN, "eselldesc",	15, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eselldesc},
	{1, LIN, "sitem",	14, 24, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ",  "Start Item           :", "Enter Start Item, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sitem},
	{1, LIN, "sitemdesc",	14, 41, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sitemdesc},
	{1, LIN, "eitem",	15, 24, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ",  "End Item             :", "Enter End Item, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.eitem},
	{1, LIN, "eitemdesc",	15, 41, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eitemdesc},
	{1, LIN, "ssupp",	14, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Supplier       :", "Enter Start Supplier, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssupp},
	{1, LIN, "ssuppdesc",	14, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ssuppdesc},
	{1, LIN, "esupp",	15, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Supplier         :", "Enter End Supplier, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esupp},
	{1, LIN, "esuppdesc",	15, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.esuppdesc},
	{1, LIN, "uplift",	 17, 24, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ",  "Uplift         :", "Enter % Uplift ",
		 NA, NO,  JUSTRIGHT, "-99.99", "99.99", (char *) &ingu_rec.uplift},
	{1, LIN, "roundtype",	17, 60, CHARTYPE,
		"U", "          ",
		" ", "U", "Rounding  :", "U)p D)own N)earest, Default = Up",
		 NA, NO,  JUSTLEFT, "UDN", "", ingu_rec.rounding},
	{1, LIN, "typedesc",	17, 60, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.typedesc},
	{2, LIN, "upto1",	4, 24, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Up To Value 1 :", "Enter Value That Rounding Applies To ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price[0]},
	{2, LIN, "sig1",	4, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Rounding Level 1 :", "Enter Rounding Value",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig[0]},
	{2, LIN, "upto2",	5, 24, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            2 :", "Enter Value That Rounding Applies To ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price[1]},
	{2, LIN, "sig2",	5, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               2 :", "Enter Rounding Value",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig[1]},
	{2, LIN, "upto3",	6, 24, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            3 :", "Enter Value That Rounding Applies To ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price[2]},
	{2, LIN, "sig3",	6, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               3 :", "Enter Rounding Value",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig[2]},
	{2, LIN, "upto4",	7, 24, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            4 :", "Enter Value That Rounding Applies To ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price[3]},
	{2, LIN, "sig4",	7, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               4 :", "Enter Rounding Value",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig[3]},
	{2, LIN, "upto5",	8, 24, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            5 :", "Enter Value That Rounding Applies To ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price[4]},
	{2, LIN, "sig5",	8, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               5 :", "Enter Rounding Value",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig[4]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void srch_ingp (char *key_val);
int  heading (int scn);
void Process (void);
void shutdown_prog (void);
void LoadFields (void);
void srch_ingu (char *key_val);
int  ReadPocr (char *rec, int Errors);
int  ReadEsmr (int Errors);
int  ReadCcmr (int Errors);
void SetUpRange (int load);
int  FindGrp (char *rec, int Errors);
int  IntFindSumr (char *rec, int Errors);
void srch_ccmr (char *key_val);
void srch_pocr (char *key_val);
void srch_esmr (char *key_val);
void GetDesc (void);
void update_menu (void);
void CreateInprs (void);
void UpdInprs (void);
int  IsOutOfRange (void);
void PrintInprs (void);
void CalcUplift (void);
void DoRounding (int i);
int  BuyGrpOk (void);
int  SellGrpOk (void);
int  SuppOk (void);
int  ItemOk (void);
double Roundit (char *type, float sig, float value);
void PrintAud (void);
void OpenAud (void);
void OpenPrnt (void);
void SaveAud (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr = get_env ("CURR_CODE");

	sprintf (CURR_CODE, "%-3.3s", sptr);
	CR_FIND = atoi (get_env ("CR_FIND"));
	C_OWNED = atoi (get_env ("CR_CO"));

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr)
	{
		numOfPrices = atoi (sptr);
		if (numOfPrices > 9 || numOfPrices < 1)
			numOfPrices = 9;
	}
	else
		numOfPrices = 5;

	sptr = chk_env ("SK_DBQTYNUM");
	if (sptr)
	{
		numOfBrks = atoi (sptr);
		if (numOfBrks > 9 || numOfBrks < 0)
			numOfBrks = 9;
	}
	else
		numOfBrks = 0;

	sptr = chk_env ("DB_MCURR");
	if (sptr)
		MCURR = atoi (sptr);

	if (argc < 2)
	{
		print_at (0,0, mlStdMess036, argv[0]);
		return (EXIT_FAILURE);
	}

	local_rec.lpno = atoi (argv[1]);

	sptr = chk_env("SK_CUSPRI_LVL");
	if (sptr == (char *)0)
		skPriceLevel = 0;
	else
		skPriceLevel = atoi (sptr);

	SETUP_SCR (vars);

	if (skPriceLevel == 0) /* Company Level Pricing */
	{
		FLD ("br")     = ND;
		FLD ("brdesc") = ND;
		FLD ("wh")     = ND;
		FLD ("whdesc") = ND;
		vars[ label ("range") ].row		=	10;
		vars[ label ("rangedesc") ].row	=	10;
		vars[ label ("sbuygrp") ].row	=	11;
		vars[ label ("sbuydesc") ].row	=	11;
		vars[ label ("ebuygrp") ].row	=	12;
		vars[ label ("ebuydesc") ].row	=	12;
		vars[ label ("ssellgrp") ].row	=	11;
		vars[ label ("sselldesc") ].row	=	11;
		vars[ label ("esellgrp") ].row	=	12;
		vars[ label ("eselldesc") ].row	=	12;
		vars[ label ("sitem") ].row		=	11;
		vars[ label ("sitemdesc") ].row	=	11;
		vars[ label ("eitem") ].row		=	12;
		vars[ label ("eitemdesc") ].row	=	12;
		vars[ label ("ssupp") ].row		=	11;
		vars[ label ("ssuppdesc") ].row	=	11;
		vars[ label ("esupp") ].row		=	12;
		vars[ label ("esuppdesc") ].row	=	12;
		vars[ label ("uplift") ].row	=	14;
		vars[ label ("roundtype") ].row	=	14;
		vars[ label ("typedesc") ].row	=	14;
	}

	if (skPriceLevel == 1) /* Branch Level Pricing */
	{
		FLD ("wh")     = ND;
		FLD ("whdesc") = ND;
		vars[ label ("range") ].row		=	12;
		vars[ label ("rangedesc") ].row	=	12;
		vars[ label ("sbuygrp") ].row	=	13;
		vars[ label ("sbuydesc") ].row	=	13;
		vars[ label ("ebuygrp") ].row	=	14;
		vars[ label ("ebuydesc") ].row	=	14;
		vars[ label ("ssellgrp") ].row	=	13;
		vars[ label ("sselldesc") ].row	=	13;
		vars[ label ("esellgrp") ].row	=	14;
		vars[ label ("eselldesc") ].row	=	14;
		vars[ label ("sitem") ].row		=	13;
		vars[ label ("sitemdesc") ].row	=	13;
		vars[ label ("eitem") ].row		=	14;
		vars[ label ("eitemdesc") ].row	=	14;
		vars[ label ("ssupp") ].row		=	13;
		vars[ label ("ssuppdesc") ].row	=	13;
		vars[ label ("esupp") ].row		=	14;
		vars[ label ("esuppdesc") ].row	=	14;
		vars[ label ("uplift") ].row	=	16;
		vars[ label ("roundtype") ].row	=	16;
		vars[ label ("typedesc") ].row	=	16;
	}

	if (!MCURR)
	{
		sprintf (ingu_rec.curr_code, "%-3.3s", CURR_CODE);
		FLD ("curr") = NA;
		FLD ("currdesc") = ND;
	}

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	
	strcpy (branchNumber, (C_OWNED) ? comm_rec.test_no : " 0");
	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		sprintf (ingu_rec.file_desc, "%-40.40s", " ");
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
		fstTime 	= TRUE;
	
		init_vars (1);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		if (LIVE)
		{
			heading (2);
			entry (2);
			if (prog_exit || restart)
				continue;
		}

		edit_all ();
		if (restart)
			continue;

		Process ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	abc_alias (inpr2, inpr);
	open_rec(sumr,sumr_list,sumr_no_fields,CR_FIND ?"sumr_id_no3":"sumr_id_no");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (ingp, ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec (ingu, ingu_list, ingu_no_fields, "ingu_id_no");
	open_rec (esmr, esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec (pocr, pocr_list, pocr_no_fields, "pocr_id_no");
	open_rec (inpr, inpr_list, inpr_no_fields, "inpr_hhgu_hash");
	open_rec (inpr2,inpr_list, inpr_no_fields, "inpr_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ingp);	
	abc_fclose (sumr);	
	abc_fclose (inmr);	
	abc_fclose (inis);	
	abc_fclose (ingu);	
	abc_fclose (esmr);	
	abc_fclose (ccmr);	
	abc_fclose (pocr);	
	abc_fclose (inpr);	
	abc_fclose (inpr2);	
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (!MCURR && cur_screen == 1)
	{
		sprintf (ingu_rec.curr_code, "%-3.3s", CURR_CODE);
		DSP_FLD ("curr");
	}

	if (LCHECK ("price"))
	{
		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.price_type > numOfPrices)
		{
			sprintf (err_str, ML(mlSkMess218), numOfPrices);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.price_desc, 
				comm_rec.price_type [local_rec.price_type - 1]);
		DSP_FLD ("price_desc");
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("sig", 3))
	{
		int	count = atoi (FIELD.label + 3);
		count --;

		if (ingu_rec.price[count] == 0.00)
		{
			print_mess (ML(mlSkMess219));
			sleep (sleepTime);
			clear_mess ();
			/*----------------
			| force it to be zero
			--------------------*/
			ingu_rec.sig[count] = 0.00;
			return (EXIT_SUCCESS);
		}

		if (ingu_rec.sig[count] >= ingu_rec.price[count]) 
		{
			print_mess (ML(mlSkMess220));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((ingu_rec.sig[count] / ingu_rec.price[count]) > 0.2)
		{
			print_mess (ML(mlSkMess221));
			sleep (sleepTime);
			clear_mess ();

			/*-------------------
			| this is a warning only 
			| therefore no return (EXIT_FAILURE);
			--------------------------*/
		}
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("upto", 4))
	{
		int	count = atoi (FIELD.label + 4);
		count --;

		if (!ingu_rec.price[count])
		{
			int	i;
			for (i = count; i < 5; i++)
			{
				ingu_rec.price[i] = 0.00;
				ingu_rec.sig[i] = 0.00;
			}
			entry_exit = TRUE;
			scn_display (2);
			return (EXIT_SUCCESS);
		}

		/*--------------------------------
		| make sure last entered no == 0.00
		--------------------------------*/
		if (count)
		{
			if (ingu_rec.price[count - 1] == 0.00)
			{
				print_mess (ML(mlSkMess122));
				sleep (sleepTime);
				clear_mess ();
				/*----------------
				| force it to be zero
				--------------------*/
				ingu_rec.price[count] = 0.00;
				return (EXIT_SUCCESS);
			}
		}

		/*--------------------------------
		| make sure larger than last value
		--------------------------------*/
		if (count)
		{
			if (ingu_rec.price[count] <= ingu_rec.price[count - 1])
			{
				print_mess (ML(mlSkMess222));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*--------------------------------
		| make sure less than next value
		| and that value is not zero.    
		--------------------------------*/
		if (prog_status != ENTRY && count != 4)
		{
			if ((ingu_rec.price[count] >= ingu_rec.price[count + 1]) &&
				ingu_rec.price[count + 1] != 0.00)
			{
				print_mess (ML(mlSkMess223));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if ((ingu_rec.sig[count] / ingu_rec.price[count]) > 0.2)
		{
			print_mess (ML(mlSkMess221));
			sleep (sleepTime);
			clear_mess ();

			/*-------------------
			| this is a warning only 
			| therefore no return (EXIT_FAILURE);
			--------------------------*/
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("roundtype"))
	{
		GetDesc ();
		DSP_FLD ("typedesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr"))
	{
			if (FLD ("curr") == NA)
				return (EXIT_SUCCESS);

			if (SRCH_KEY)
			{
				srch_pocr (temp_str);
				return (EXIT_SUCCESS);
			}

			if (dflt_used)
			{
				if (ReadPocr (CURR_CODE, TRUE))
					return (EXIT_FAILURE);
				strcpy (ingu_rec.curr_code, pocr_rec.code);
			}
			else
			{
				if (ReadPocr (ingu_rec.curr_code, TRUE))
					return (EXIT_FAILURE);
			}
		
			DSP_FLD ("currdesc");
			return (EXIT_SUCCESS);
	}

	if (LCHECK ("br"))
	{
			if (skPriceLevel == 0)
			{
				strcpy (ingu_rec.br_no, "  ");
				strcpy (ingu_rec.wh_no, "  ");
				return (EXIT_SUCCESS);
			}

			if (SRCH_KEY)
			{
				srch_esmr (temp_str);
				return (EXIT_SUCCESS);
			}

			if (ReadEsmr (TRUE))
				return (EXIT_FAILURE);

			DSP_FLD ("br");
			DSP_FLD ("brdesc");
			return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh"))
	{
			if (skPriceLevel == 1)
			{
				strcpy (ingu_rec.wh_no, "  ");
				return (EXIT_SUCCESS);
			}

			if (FLD ("wh") == NA)
				return (EXIT_SUCCESS);

			if (SRCH_KEY)
			{
				srch_ccmr (temp_str);
				return (EXIT_SUCCESS);
			}

			if (ReadCcmr (TRUE))
				return (EXIT_FAILURE);
			DSP_FLD ("wh");
			DSP_FLD ("whdesc");
			return (EXIT_SUCCESS);
	}

	if (LCHECK ("file"))
	{
		/*------------------------
		| if you get new file then
		| reset QuitStr to say quit
		| without saving
		----------------------------*/
		strcpy (QuitStr, "Quit Without Updating ");
		
		FLD ("curr")		= NA;
		if (skPriceLevel == 0)
		{
			FLD ("br") 			= ND;
			FLD ("wh") 			= ND;
		}
		else
		if (skPriceLevel == 1)
		{
			FLD ("br") 			= NA;
			FLD ("wh") 			= ND;
		}
		else
		{
			FLD ("br") 			= NA;
			FLD ("wh") 			= NA;
		}
		FLD ("range") 		= NA;
		FLD ("effdate") 	= NA;
		FLD ("uplift") 		= NA;
		FLD ("roundtype") 	= NA;
		FLD ("sbuygrp")   	= ND;
		FLD ("ebuygrp")   	= ND;
		FLD ("sbuydesc")  	= ND;
		FLD ("ebuydesc")  	= ND;
		FLD ("ssellgrp")  	= ND;
		FLD ("esellgrp")  	= ND;
		FLD ("sselldesc") 	= ND;
		FLD ("eselldesc") 	= ND;
		FLD ("sitem")  	  	= ND;
		FLD ("eitem")     	= ND;
		FLD ("sitemdesc") 	= ND;
		FLD ("eitemdesc") 	= ND;
		FLD ("ssupp")     	= ND;
		FLD ("esupp")     	= ND;
		FLD ("ssuppdesc") 	= ND;
		FLD ("esuppdesc") 	= ND;
		FLD ("upto1")		= NA;
		FLD ("upto2")		= NA;
		FLD ("upto3")		= NA;
		FLD ("upto4")		= NA;
		FLD ("upto5")		= NA;
		FLD ("sig1")		= NA;
		FLD ("sig2")		= NA;
		FLD ("sig3")		= NA;
		FLD ("sig4")		= NA;
		FLD ("sig5")		= NA;

		if (SRCH_KEY)
		{
			srch_ingu (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			memset (&ingu_rec, 0, sizeof (ingu_rec));
			strcpy (ingu_rec.co_no, comm_rec.tco_no);
			ingu_rec.price_type = local_rec.price_type;

			if (MCURR)
				FLD ("curr")		= YES;
			if (skPriceLevel == 0)
			{
				FLD ("br") 			= ND;
				FLD ("wh") 			= ND;
			}
			else
			if (skPriceLevel == 1)
			{
				FLD ("br") 			= YES;
				FLD ("wh") 			= ND;
			}
			else
			{
				FLD ("br") 			= YES;
				FLD ("wh") 			= YES;
			}
			FLD ("range") 		= YES;
			FLD ("effdate") 	= ND;
			FLD ("uplift") 		= YES;
			FLD ("roundtype") 	= YES;
			FLD ("upto1")		= YES;
			FLD ("upto2")		= YES;
			FLD ("upto3")		= YES;
			FLD ("upto4")		= YES;
			FLD ("upto5")		= YES;
			FLD ("sig1")		= YES;
			FLD ("sig2")		= YES;
			FLD ("sig3")		= YES;
			FLD ("sig4")		= YES;
			FLD ("sig5")		= YES;

			heading (1);
			sprintf (ingu_rec.file_desc, "%-40.40s", "L I V E   P R I C E S");
			DSP_FLD ("price");
			DSP_FLD ("price_desc");
			DSP_FLD ("filedesc");
			return (EXIT_SUCCESS);
		}

		strcpy (ingu_rec.co_no, comm_rec.tco_no);
		ingu_rec.price_type = local_rec.price_type;
		cc = find_rec (ingu, &ingu_rec, EQUAL, "w");
		if (cc == -1)
			return (EXIT_FAILURE);

		if (cc)
		{
			print_mess (ML(mlSkMess555));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		LoadFields ();
		scn_display (1);
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("sitem"))
	{
		if (FLD ("sitem") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.sitem, "%16.16s", " ");
			sprintf (local_rec.sitemdesc, 
					 "%-34.34s",
					 "Start Of File ");
			DSP_FLD ("sitemdesc");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.sitem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.sitem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.sitem,inmr_rec.mr_item_no);
		sprintf (local_rec.sitemdesc, "%-34.34s", inmr_rec.mr_description);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sitem, local_rec.eitem) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		DSP_FLD ("sitemdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eitem"))
	{
		if (FLD ("eitem") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		if (dflt_used)
		{
			sprintf (local_rec.eitem, "%16.16s", "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.eitemdesc, 
					 "%-34.34s",
					 "End Of File ");
			DSP_FLD ("eitemdesc");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.eitem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.eitem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.eitem,inmr_rec.mr_item_no);
		sprintf (local_rec.eitemdesc, "%-34.34s", inmr_rec.mr_description);

		if (strcmp (local_rec.sitem, local_rec.eitem) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("eitemdesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| supplier Validate Creditor Number. 
	---------------------------*/
	if (LCHECK ("ssupp"))
	{
		if (FLD ("ssupp") == ND || !LIVE)
			return (EXIT_SUCCESS);


		if (dflt_used)
		{
			sprintf (local_rec.ssupp, "%6.6s", " ");
			sprintf (local_rec.ssuppdesc, 
					 "%-40.40s",
					 "Start Of File ");
			DSP_FLD ("ssuppdesc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.ssupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.ssupp, sumr_rec.sm_crd_no);
		strcpy (local_rec.ssuppdesc, sumr_rec.sm_name);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		DSP_FLD ("ssuppdesc");
		return(0);
	}

	if (LCHECK ("esupp"))
	{
		if (FLD ("esupp") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.esupp, "%6.6s", "~~~~~~");
			sprintf (local_rec.esuppdesc, 
					 "%-40.40s",
					 "End Of File ");
			DSP_FLD ("esuppdesc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.esupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.esuppdesc, sumr_rec.sm_name);
		strcpy (local_rec.esupp, sumr_rec.sm_crd_no);

		if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("esuppdesc");
		return(0);
	}

	/*--------------------------
	| if range entered was by
	| buying group
	---------------------------*/
	if (LCHECK ("sbuygrp"))
	{
		if (FLD ("sbuygrp") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.sbuygrp, "      ");
			strcpy (ingp_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (ingp_rec.type, "B");
			if (FindGrp (local_rec.sbuygrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sbuydesc, ingp_rec.desc);
		DSP_FLD ("sbuydesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ebuygrp"))
	{
		if (FLD ("ebuygrp") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.ebuygrp, "~~~~~~");
			strcpy (ingp_rec.desc, "End Of File");
		}
		else
		{
			strcpy (ingp_rec.type, "B");
			if (FindGrp (local_rec.ebuygrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ebuydesc, ingp_rec.desc);
		DSP_FLD ("ebuydesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| selling group
	---------------------------*/
	if (LCHECK ("ssellgrp"))
	{
		if (FLD ("ssellgrp") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.ssellgrp, "      ");
			strcpy (ingp_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (ingp_rec.type, "S");
			if (FindGrp (local_rec.ssellgrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sselldesc, ingp_rec.desc);
		DSP_FLD ("sselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esellgrp"))
	{
		if (FLD ("esellgrp") == ND || !LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.esellgrp, "~~~~~~");
			strcpy (ingp_rec.desc, "End Of File");
		}
		else
		{
			strcpy (ingp_rec.type, "S");
			if (FindGrp (local_rec.esellgrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.eselldesc, ingp_rec.desc);
		DSP_FLD ("eselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("range"))
	{
		static char	last_range[3] = {"  "};
 		if (!LIVE)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY && ingu_rec.apply_to[0] == last_range[0])
			return (EXIT_SUCCESS);

#ifndef GVISION
		print_at (13, 2, "%70.70s", " ");
		print_at (14, 2, "%70.70s", " ");
#endif	/* GVISION */

		SetUpRange (FALSE);

		scn_write (1);
		DSP_FLD ("rangedesc");

		/*----------------------
		| if change the range then 
		| do get_entry's for both
		| new fields
		------------------------*/
		if (prog_status != ENTRY && ingu_rec.apply_to[0] != last_range[0])
		{
			DSP_FLD ("range");
			/*  sellgrp */
			if (ingu_rec.apply_to[0] == 'S')
			{
				do
				{
					get_entry (label ("ssellgrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ssellgrp")));
				DSP_FLD ("ssellgrp");

				do
				{
					get_entry (label ("esellgrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("esellgrp")));
				DSP_FLD ("esellgrp");
			}

			/*  buygrp */
			if (ingu_rec.apply_to[0] == 'B')
			{
				do
				{
					get_entry (label ("sbuygrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("sbuygrp")));
				DSP_FLD ("sbuygrp");

				do
				{
					get_entry (label ("ebuygrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ebuygrp")));
				DSP_FLD ("ebuygrp");
			}

			/*  item */
			if (ingu_rec.apply_to[0] == 'I')
			{
				do
				{
					get_entry (label ("sitem"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("sitem")));
				DSP_FLD ("sitem");

				do
				{
					get_entry (label ("eitem"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("eitem")));
				DSP_FLD ("eitem");
			}

			/*  supplier */
			if (ingu_rec.apply_to[0] == 'P')
			{
				do
				{
					get_entry (label ("ssupp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ssupp")));
				DSP_FLD ("ssupp");

				do
				{
					get_entry (label ("esupp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("esupp")));
				DSP_FLD ("esupp");
			}
		}

		strcpy (last_range, ingu_rec.apply_to);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
srch_ingp (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	if (ingu_rec.apply_to[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type[0] == 'S' && ingu_rec.apply_to[0] == 'S')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (ingp_rec.type[0] == 'B' && ingu_rec.apply_to[0] == 'B')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	if (ingu_rec.apply_to[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

int
heading (
 int scn)
{
	int		ws_lines;

	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	strcpy (err_str, ML (mlSkMess224));
	rv_pr (err_str, (80 - strlen (clip (err_str))) / 2, 0, 1);

	if (skPriceLevel == 0) /* Branch Level Pricing */
		ws_lines = 0;
	else
	if (skPriceLevel == 1) /* Branch Level Pricing */
		ws_lines = 2;
	else
		ws_lines = 3;

	if (scn == 1)
	{
		box (0, 2, 80, 12 + ws_lines);

		if (!MCURR)
		{
			move (1, 7);
			line (79);
		}
		move (1, 9);
		line (79);
		if (skPriceLevel > 0)
		{
			move (1, 9 + ws_lines);
			line (79);
		}
		move (1, 13 + ws_lines);
		line (79);
	}
	else
		box (0, 3, 80, 5);

	move (0, 21);
	line (80);

	print_at (22, 1, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
	move (0,1);
	line (80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
Process (
 void)
{
		/*------------------------
		| we will use the ingu as the basis
		| of update, even on LIVE PRICES
		-----------------------------------*/
		if (LIVE)
		{
			strcpy (ingu_rec.co_no, comm_rec.tco_no);
			
			/*------------------------
			| if ALL wh's or br's then
			| set to blank

			if (!strcmp (ingu_rec.br_no, "AL"))
				strcpy (ingu_rec.br_no, "  ");
			if (!strcmp (ingu_rec.wh_no, "AL"))
				strcpy (ingu_rec.wh_no, "  ");
			---------------------------*/

			/*--------------------
			| copy in the ranges
			----------------------*/
			if (ingu_rec.apply_to[0] == 'B')
			{
				sprintf (ingu_rec.st_range, "%-6.6s", local_rec.sbuygrp);
				sprintf (ingu_rec.end_range, "%-6.6s", local_rec.ebuygrp);
			}

			if (ingu_rec.apply_to[0] == 'S')
			{
				sprintf (ingu_rec.st_range, "%-6.6s", local_rec.ssellgrp);
				sprintf (ingu_rec.end_range, "%-6.6s", local_rec.esellgrp);
			}

			if (ingu_rec.apply_to[0] == 'I')
			{
				sprintf (ingu_rec.st_range, "%-16.16s", local_rec.sitem);
				sprintf (ingu_rec.end_range, "%-16.16s", local_rec.eitem);
			}

			if (ingu_rec.apply_to[0] == 'P')
			{
				sprintf (ingu_rec.st_range, "%-6.6s", local_rec.ssupp);
				sprintf (ingu_rec.end_range, "%-6.6s", local_rec.esupp);
			}
		}

		update_menu ();
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
LoadFields (
 void)
{
	/*-------------------
	| read and load currency
	-------------------------*/
	if (MCURR)
		if (ReadPocr (ingu_rec.curr_code, FALSE))
			return;

	/*-------------------------
	| read and load branch
	-----------------------*/
	if (ReadEsmr (FALSE))
		return;

	/*-------------------------
	| read and load warehouse
	-----------------------*/
	if (ReadCcmr (FALSE))
		return;

	/*---------------------
	| read and load range
	| read different files
	| depending upon type
	---------------------*/
	strcpy (ingu_rec.apply_to, ingu_rec.apply_to);
	SetUpRange (TRUE);

	scn_write (1);
	DSP_FLD ("rangedesc");
	/*------------
	| load rounding
	------------*/
	GetDesc ();
}

void
srch_ingu (
 char *key_val)
{
	work_open ();
	save_rec ("#File", "#Description ");

	strcpy (ingu_rec.co_no, comm_rec.tco_no);
	sprintf (ingu_rec.file_code, "%-6.6s", key_val);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingu_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (ingu_rec.file_code, key_val, strlen (key_val)))
	{
		if (ingu_rec.price_type != local_rec.price_type)
		{
			cc = find_rec (ingu, &ingu_rec, NEXT, "r");
			continue;
		}
		cc = save_rec (ingu_rec.file_code, ingu_rec.file_desc);
		if (cc)
			break;

		cc = find_rec (ingu, &ingu_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingu_rec.co_no, comm_rec.tco_no);
	sprintf (ingu_rec.file_code, "%-6.6s", temp_str);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, EQUAL, "u");
	if (cc)
		file_err (cc, ingu, "DBFIND");
}

int
ReadPocr (
 char *rec, 
 int Errors)
{
	strcpy (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%3.3s", rec);
	cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
	if (cc)
	{
		if (Errors)
		{
			/*("Currency Code Not On File\007");*/

			print_mess (ML(mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
		}
		else
			file_err (cc, pocr, "DBFIND");
	}
	return (cc);
}

int
ReadEsmr (
 int Errors)
{
	if (skPriceLevel == 0)
	{
		strcpy (ingu_rec.br_no, "AL");
		return (EXIT_SUCCESS);
	}

	if (dflt_used || !strcmp (ingu_rec.br_no, "  "))
	{
		cc = FALSE;
		strcpy (ingu_rec.br_no, "AL");
		sprintf (local_rec.brdesc, "%-43.43s", "ALL");
		if (skPriceLevel == 2)
		{
			FLD ("wh") = NA;
			strcpy (ingu_rec.wh_no, "AL");
			sprintf (local_rec.whdesc, "%-43.43s", "ALL");
			DSP_FLD ("wh");
			DSP_FLD ("whdesc");
		}
	}
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.tco_no);
		strcpy (esmr_rec.est_no, ingu_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			if (Errors)
			{
				print_mess (ML(mlStdMess073));
				sleep (sleepTime);
				clear_mess ();
			}
			else
				file_err (cc, esmr, "DBFIND");
		}
		if (LIVE && skPriceLevel == 2)
			FLD ("wh") = YES;

		sprintf (local_rec.brdesc, "%s %s", ingu_rec.br_no, esmr_rec.est_name);
	}
	DSP_FLD ("brdesc");
	return (cc);
}

int
ReadCcmr (
 int Errors)
{
	if (skPriceLevel != 2)
	{
		strcpy (ingu_rec.wh_no, "AL");
		return (EXIT_SUCCESS);
	}

	if (dflt_used || 
		!strcmp (ingu_rec.wh_no, "  ") || 
		!strcmp (ingu_rec.wh_no, "AL"))
	{
		cc = FALSE;
		sprintf (local_rec.whdesc, "%-43.43s", "ALL");
		strcpy (ingu_rec.wh_no, "AL");
	}
	else
	{
		strcpy (ccmr_rec.co_no, comm_rec.tco_no);
		strcpy (ccmr_rec.est_no, ingu_rec.br_no);
		strcpy (ccmr_rec.cc_no, ingu_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			if (Errors)
			{
				/*("Warehouse Not On File ");*/

				print_mess (ML(mlStdMess100));
				sleep (sleepTime);
				clear_mess ();
			}
			else
				file_err (cc, ccmr, "DBFIND");
		}
		sprintf (local_rec.whdesc, "%s %s", ingu_rec.wh_no, ccmr_rec.name);
	}
	DSP_FLD ("whdesc");
	return (cc);
}

/*=======================
| if load is true, copy in
| from ingu else
| blank them, this will also
| mean reading ingp or sumr
| or inmr for descriptions
==========================*/
void
SetUpRange (
 int load)
{
	FLD ("sbuygrp")   = ND;
	FLD ("ebuygrp")   = ND;
	FLD ("sbuydesc")  = ND;
	FLD ("ebuydesc")  = ND;
	FLD ("ssellgrp")  = ND;
	FLD ("esellgrp")  = ND;
	FLD ("sselldesc") = ND;
	FLD ("eselldesc") = ND;
	FLD ("sitem")  	  = ND;
	FLD ("eitem")     = ND;
	FLD ("sitemdesc") = ND;
	FLD ("eitemdesc") = ND;
	FLD ("ssupp")     = ND;
	FLD ("esupp")     = ND;
	FLD ("ssuppdesc") = ND;
	FLD ("esuppdesc") = ND;

	/*-------------------------
	| if choice is buying group
	--------------------------*/
	if (ingu_rec.apply_to[0] == 'B')
	{
		FLD ("sbuygrp")  = (LIVE) ? YES : NA;
		FLD ("ebuygrp")  = (LIVE) ? YES : NA;
		FLD ("sbuydesc") = NA;
		FLD ("ebuydesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", "Buying Group");

		if (load)
		{
			sprintf (local_rec.sbuygrp, "%-6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.sbuygrp, "      "))
			{
				strcpy (ingp_rec.desc, "Start Of File");
			}
			else
			{
				strcpy (ingp_rec.type, "B");
				if (FindGrp (local_rec.sbuygrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.sbuydesc, ingp_rec.desc);

			sprintf (local_rec.ebuygrp, "%-6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.ebuygrp, "~~~~~~"))
			{
				strcpy (ingp_rec.desc, "End Of File");
			}
			else
			{
				strcpy (ingp_rec.type, "B");
				if (FindGrp (local_rec.ebuygrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.ebuydesc, ingp_rec.desc);
		}
		else
		{
			sprintf (local_rec.sbuygrp, "%6.6s", " ");
			sprintf (local_rec.ebuygrp, "%6.6s", "~~~~~~");
			sprintf (local_rec.sbuydesc, "%40.40s", " ");
			sprintf (local_rec.ebuydesc, "%40.40s", " ");
		}
		return;
	}

	/*-------------------------
	| if choice is selling group
	--------------------------*/
	if (ingu_rec.apply_to[0] == 'S')
	{
		FLD ("ssellgrp")  = (LIVE) ? YES : NA;
		FLD ("esellgrp")  = (LIVE) ? YES : NA;
		FLD ("sselldesc") = NA;
		FLD ("eselldesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", "Selling Group");
		if (load)
		{
			sprintf (local_rec.ssellgrp, "%-6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.ssellgrp, "      "))
			{
				strcpy (ingp_rec.desc, "Start Of File");
			}
			else
			{
				strcpy (ingp_rec.type, "S");
				if (FindGrp (local_rec.ssellgrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.sselldesc, ingp_rec.desc);

			sprintf (local_rec.esellgrp, "%-6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.esellgrp, "~~~~~~"))
			{
				strcpy (ingp_rec.desc, "End Of File");
			}
			else
			{
				strcpy (ingp_rec.type, "S");
				if (FindGrp (local_rec.esellgrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.eselldesc, ingp_rec.desc);
		}
		else
		{
			sprintf (local_rec.ssellgrp, "%6.6s", " ");
			sprintf (local_rec.esellgrp, "%6.6s", "~~~~~~");
			sprintf (local_rec.sselldesc, "%40.40s", " ");
			sprintf (local_rec.eselldesc, "%40.40s", " ");
		}
		return;
	}

	/*---------------------
	| if choice is item
	----------------------*/
	if (ingu_rec.apply_to[0] == 'I')
	{
		FLD ("sitem")  = (LIVE) ? YES : NA;
		FLD ("eitem")  = (LIVE) ? YES : NA;
		FLD ("sitemdesc") = NA;
		FLD ("eitemdesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", "Item");

		if (load)
		{
			/* start item */
			sprintf (local_rec.sitem, "%16.16s", ingu_rec.st_range);
			if (!strcmp (local_rec.sitem, "                "))
			{
				sprintf (local_rec.sitemdesc, 
						 "%-34.34s",
					 "Start Of File ");
			}
			else
			{
				cc = FindInmr (comm_rec.tco_no, local_rec.sitem, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
					strcpy (inmr_rec.mr_item_no, local_rec.sitem);
					cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
				}
				if (cc)
					file_err (cc, inmr, "DBFIND");

				SuperSynonymError ();
				strcpy (local_rec.sitem, inmr_rec.mr_item_no);
				sprintf(local_rec.sitemdesc,"%-34.34s",inmr_rec.mr_description);
			}

			/*  end item */
			sprintf (local_rec.eitem, "%16.16s", ingu_rec.end_range);
			if (!strcmp (local_rec.eitem, "~~~~~~~~~~~~~~~~"))
			{
				sprintf (local_rec.eitemdesc, "%-34.34s", "End Of File ");
			}
			else
			{
				cc = FindInmr (comm_rec.tco_no, local_rec.eitem, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
					strcpy (inmr_rec.mr_item_no, local_rec.eitem);
					cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
				}
				SuperSynonymError ();
				strcpy (local_rec.eitem, inmr_rec.mr_item_no);
				sprintf(local_rec.eitemdesc,"%-34.34s",inmr_rec.mr_description);
			}
		}
		else
		{
			sprintf (local_rec.sitem, "%16.16s", " ");
			sprintf (local_rec.eitem, "%16.16s", "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.sitemdesc, "%34.34s", " ");
			sprintf (local_rec.eitemdesc, "%34.34s", " ");
		}
		return;
	}

	/*---------------------
	| if choice is supplier
	----------------------*/
	if (ingu_rec.apply_to[0] == 'P')
	{
		FLD ("ssupp")  = (LIVE) ? YES : NA;
		FLD ("esupp")  = (LIVE) ? YES : NA;
		FLD ("ssuppdesc") = NA;
		FLD ("esuppdesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", "Supplier");

		if (load)
		{
			sprintf (local_rec.ssupp, "%6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.ssupp, "      "))
			{
				sprintf (local_rec.ssuppdesc, 
						 "%40.40s",
						 "Start Of File ");
			}
			else
			{
				cc = IntFindSumr (local_rec.ssupp, TRUE);
				if (cc)
					file_err (cc, sumr, "DBFIND");

				strcpy (local_rec.ssupp, sumr_rec.sm_crd_no);
				strcpy (local_rec.ssuppdesc, sumr_rec.sm_name);
			}

			sprintf (local_rec.esupp, "%6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.esupp, "~~~~~~"))
			{
				sprintf (local_rec.esuppdesc, 
						 "%-40.40s",
						 "End Of File ");
			}
			else
			{
				cc = IntFindSumr (local_rec.esupp, TRUE);
				if (cc)
					file_err (cc, sumr, "DBFIND");

				strcpy (local_rec.esupp, sumr_rec.sm_crd_no);
				strcpy (local_rec.esuppdesc, sumr_rec.sm_name);
			}
		}
		else
		{
			sprintf (local_rec.ssupp, "%6.6s", " ");
			sprintf (local_rec.esupp, "%6.6s", "~~~~~~");
			sprintf (local_rec.ssuppdesc, "%40.40s", " ");
			sprintf (local_rec.esuppdesc, "%40.40s", " ");
		}
		return;
	}
}

int
FindGrp (
 char *rec, 
 int Errors)
{
		sprintf (ingp_rec.code, "%-6.6s", rec);
		strcpy (ingp_rec.co_no, comm_rec.tco_no);
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		
		if (cc && Errors)
		{
			if (ingp_rec.type[0] == 'B')
				print_mess (ML(mlStdMess207));
			else
				print_mess (ML(mlStdMess208));

			sleep (sleepTime);
			clear_mess ();
		}
		return (cc);
}

int
IntFindSumr (
 char *rec, 
 int Errors)
{
		sprintf (sumr_rec.sm_crd_no, "%6.6s", rec);
		strcpy (sumr_rec.sm_co_no, comm_rec.tco_no);
		strcpy (sumr_rec.sm_est_no, branchNumber);
		pad_num (sumr_rec.sm_crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc && Errors) 
		{
			/*sprintf (err_str, "Supplier %s Is Not On File", sumr_rec.sam_crd_no);*/

			print_mess (ML(mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
		}
		return (cc); 
}

void
srch_ccmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, ingu_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp  (ccmr_rec.co_no, comm_rec.tco_no) &&
		   !strcmp	(ccmr_rec.est_no, ingu_rec.br_no) &&
	       !strncmp (ccmr_rec.cc_no, key_val, strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, ingu_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
srch_pocr (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	strcpy (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp  (pocr_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (pocr_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

void
srch_esmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Br", "#Short Name  ");

	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp  (esmr_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
GetDesc (
 void)
{
	if (ingu_rec.rounding[0] == 'U')
		sprintf (local_rec.typedesc, "%-7.7s", "Up");
	if (ingu_rec.rounding[0] == 'D')
		sprintf (local_rec.typedesc, "%-7.7s", "Down");
	if (ingu_rec.rounding[0] == 'N')
		sprintf (local_rec.typedesc, "%-7.7s", "Nearest");
}

/*===================
| Update mini menu. |
===================*/
void
update_menu (
 void)
{
	int	reply;

	for (;;)
	{
		/*-------------------------
		| use different menu based
		| upon whether it is live
		--------------------------*/

	    mmenu_print (" UPDATE SELECTION ",(LIVE) ? no_file_menu : file_menu, 0);
	    switch (mmenu_select ((LIVE) ? no_file_menu : file_menu))
	    {
		case QUIT 	 :
			return;

		case PRINT 	 :
			dsp_screen ("Printing Prices", comm_rec.tco_no, comm_rec.tco_name);
			PrintInprs ();
#ifdef GVISION
			dsp_screen_close ();
#endif
			heading (1);
			scn_display (1);
			break;

		case LIVEUPD :
			dsp_screen ("Updating Live Prices", comm_rec.tco_no, comm_rec.tco_name);
			UpdInprs ();
#ifdef GVISION
			dsp_screen_close ();
#endif
			heading (1);
			scn_display (1);
			break;

		case CREATE :
			dsp_screen ("Creating Future Prices", comm_rec.tco_no, comm_rec.tco_name);
			CreateInprs ();
#ifdef GVISION
			dsp_screen_close ();
#endif
			heading (1);
			scn_display (1);
			break;

		case SEL_DELETE :
			/*------------------------
			|  sets record including
			| index to blanks
			------------------------*/
			print_at (15, 10,ML(mlSkMess566) );

			reply = toupper (prmptmsg ( ML(mlStdMess107), "NnYy", 25,18));
			if (reply == 'N')
				break;

			dsp_screen ("Deleting Future Prices", comm_rec.tco_no, comm_rec.tco_name);
			memset (&inpr_rec, 0, sizeof (inpr));
			inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
			cc = find_rec (inpr, &inpr_rec, GTEQ, "w");

			while (!cc && inpr_rec.hhgu_hash == ingu_rec.hhgu_hash)
			{
				cc = abc_delete (inpr);
				if (cc)
					file_err (cc, inpr, "DBDELETE");

				memset (&inpr_rec, 0, sizeof (inpr));
				inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
				cc = find_rec (inpr, &inpr_rec, GTEQ, "w");
			}
			abc_unlock (inpr);

			cc = abc_delete (ingu);
			if (cc)
				file_err (cc, ingu, "DBDELETE");

#ifdef GVISION
			dsp_screen_close ();
#endif
			return;
	
		default :
			if (last_char == FN16)
				return;
			break;
	    }
	}
}

void
CreateInprs (
 void)
{
		int	reply;
		memset (&inpr_rec, 0, sizeof (inpr));
		inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
		cc = find_rec (inpr, &inpr_rec, GTEQ, "w");
		if (!cc && inpr_rec.hhgu_hash == ingu_rec.hhgu_hash)
		{
			print_at (15, 4, ML(mlSkMess567));
			reply = toupper (prmptmsg (ML(mlStdMess107), "NnYy", 25,18));
			if (reply == 'N')
				return;

			do
			{
				cc = abc_delete (inpr);
				if (cc)
					file_err (cc, inpr, "DBDELETE");

				memset (&inpr_rec, 0, sizeof (inpr));
				inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
				cc = find_rec (inpr, &inpr_rec, GTEQ, "w");
			} while (!cc && inpr_rec.hhgu_hash == ingu_rec.hhgu_hash);
		}
		abc_unlock (inpr);

		strcpy (QuitStr, "Quit                ");
		/*-----------------------
		| now loop thru inprs if
		| in range create new inpr
		| with hhgu_hash
		---------------------------*/
		abc_selfield (inpr, "inpr_hhbr_hash");
		abc_selfield (inmr, "inmr_hhbr_hash");
		abc_selfield (sumr, "sumr_hhsu_hash");

		memset (&inpr_rec, 0, sizeof (inpr));
		cc = find_hash (inpr, &inpr_rec, GTEQ, "r", 0L);


		while (!cc)
		{
			if (inpr_rec.price_type != ingu_rec.price_type ||
				inpr_rec.hhgu_hash != 0L)
			{
				cc = find_hash (inpr, &inpr_rec, NEXT, "r", 0L);
				continue;
			}
			/*-----------------
			| if inpr is in range
			--------------------*/
			if (!IsOutOfRange ())
			{
				CalcUplift ();
				/*--------------------
				| change the hhgu_hash
				| and re-add the record
				-----------------------*/
				inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
				cc = abc_add (inpr, &inpr_rec);
				if (cc)
					file_err (cc, inpr, "DBADD");
			}
			cc = find_hash (inpr, &inpr_rec, NEXT, "r", 0L);
		}

		abc_selfield (inpr, "inpr_hhgu_hash");
		abc_selfield (inmr, "inmr_id_no");
		abc_selfield (sumr, (CR_FIND) ? "sumr_id_no3" : "sumr_id_no");
}

void
UpdInprs (
 void)
{
	int	reply;
	int	updated = FALSE;

	/*---------------------
	| if NOT LIVE then change
	| future prices into
	| live, of course give 
	| warning if no future prices
	| created yet or effective date
	| not yet reached.
	------------------------------*/
	if (!LIVE)
	{
		if (ingu_rec.eff_date > comm_rec.tinv_date)
		{
		/*	cl_box (23, 14, 32, 4);*/
			/*(15, 25, "Can Not Update Live Prices  ");
			(16, 25, "With This Global Price File ");
			(17, 25, "As The Date Is Not Yet      ");
			(18, 25, "Effective. - Press Any Key !");*/

			print_at (15, 4, ML(mlSkMess568));
			PauseForKey (0, 0, "", 0);
			return;
		}

		memset (&inpr_rec, 0, sizeof (inpr));
		inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
		inpr_rec.price_type = ingu_rec.price_type;

		cc = find_rec (inpr, &inpr_rec, GTEQ, "r");
		if (cc || inpr_rec.hhgu_hash != ingu_rec.hhgu_hash)
		{
		/*	cl_box (23, 14, 32, 4);*/
			/*(15, 25, "Can Not Update Live Prices  ");
			(16, 25, "With This Global Price File ");
			(17, 25, "As No Future Prices Have    ");
			(18, 25, "Been Created - Press Any Key");*/

			print_at (15, 4, ML(mlSkMess569));
			PauseForKey (0, 0, "", 0);
			return;
		}
	}

	strcpy (QuitStr, "Quit                ");

	print_at (15, 4, ML(mlSkMess570));

	reply = toupper (prmptmsg (ML(mlStdMess107), "NnYy", 25,17));
	if (reply == 'N')
		return;

	/*-----------------------
	| now loop thru inprs if
	| in range copy across future
	| prices if by file else uplift
	| prices if LIVE.              
	---------------------------*/
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (sumr, "sumr_hhsu_hash");

	cc = find_hash (inpr, &inpr_rec, GTEQ,"w",(LIVE) ? 0L : ingu_rec.hhgu_hash);

	while (!cc)
	{
		if ((LIVE && inpr_rec.hhgu_hash != 0L) || 
			(!LIVE && inpr_rec.hhgu_hash != ingu_rec.hhgu_hash))
		{
			abc_unlock (inpr);
			cc =find_hash(inpr,&inpr_rec,NEXT,"w",(LIVE)?0L:ingu_rec.hhgu_hash);
			continue;
		}

		/*--------------------
		| check price type 
		---------------------*/
		if (inpr_rec.price_type != ingu_rec.price_type)
		{
			abc_unlock (inpr);
			cc =find_hash(inpr,&inpr_rec,NEXT,"w",(LIVE)?0L:ingu_rec.hhgu_hash);
			continue;
		}

		/*-----------------
		| if inpr is in range
		--------------------*/
		if (!IsOutOfRange ())
		{
			/*-------------------
			| take a snapshot of
			| before record
			-------------------*/
			memcpy (&inpr2_rec, &inpr_rec, sizeof (inpr_rec));

			/*----------------------------
			| if !LIVE find LIVE price to
			| delete && print on audit
			-----------------------------*/
			if (!LIVE)
			{
				inpr2_rec.hhgu_hash = 0L;
				cc = find_rec (inpr2, &inpr2_rec, EQUAL, "w");
				if (cc)
					file_err (cc, inpr2, "DBFIND");
			}

			/*--------------------
			| If LIVE Alter the 
			| prices otherwise already
			| done when creating future
			| prices.
			----------------------*/
			if (LIVE)
				CalcUplift ();

			/*-----------
			| Print Audit 
			------------*/
			SaveAud ();
			updated = TRUE;

			/*-------------------
			| if not LIVE delete
			| LIVE record.
			--------------------*/
			if (!LIVE)
			{
				cc = abc_delete (inpr2);
				if (cc)
					file_err (cc, inpr2, "DBDELETE");
			}

			/*--------------------
			| change the hhgu_hash
			| and back to zero, if 
			| LIVE does not hurt anyway
			-----------------------*/

			inpr_rec.hhgu_hash = 0L;
			cc = abc_update (inpr, &inpr_rec);
			if (cc)
				file_err (cc, inpr, "DBUPDATE");
		}
		cc = find_hash(inpr,&inpr_rec, NEXT,"w",(LIVE) ? 0L:ingu_rec.hhgu_hash);
	}

	if (updated)
	{
		fstTime = TRUE;
		fsort = sort_sort (fsort, "price");
		PrintAud ();
		fprintf (faud, ".EOF\n");
		pclose (faud);
	}

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (sumr, (CR_FIND) ? "sumr_id_no3" : "sumr_id_no");
}

int
IsOutOfRange (
 void)
{
	char	br[3];
	char	wh[3];
	/*-----------------------
	| first thing we have to do 
	| is check that inpr is ok
	| regarding curr, branch and wh_no
	---------------------------*/
	if (strcmp (inpr_rec.curr_code, ingu_rec.curr_code))
		return (TRUE);
	
	/*-----------------------
	| check that
	| inpr belongs to this brnch
	| and warehouse
	-----------------------------*/
	if (!strcmp (ingu_rec.br_no, "AL"))
		strcpy (br, "  ");
	else
		strcpy (br, ingu_rec.br_no);
		
	if (!strcmp (ingu_rec.wh_no, "AL"))
		strcpy (wh, "  ");
	else
		strcpy (wh, ingu_rec.wh_no);

	if (strcmp (inpr_rec.br_no, br))
		return (TRUE);
	if (strcmp (inpr_rec.wh_no, wh))
		return (TRUE);

	/*------------------------
	| no matter which option 
	| we need to read the inmr
	--------------------------*/

	cc = find_hash (inmr, &inmr_rec, EQUAL, "r", inpr_rec.hhbr_hash);
	if (cc)
		file_err (cc, inmr, "DBFIND");

	switch (ingu_rec.apply_to[0])
	{
	case 'B'	:
			if (BuyGrpOk ())
				return (EXIT_SUCCESS);
			break;
	case 'S'	:
			if (SellGrpOk ())
				return (EXIT_SUCCESS);
			break;
	case 'P'	:
			/*--------------------
			| find priority one
			| supp record for this
			| item
			-----------------------*/
			if (SuppOk ())
				return (EXIT_SUCCESS);
			break;
	case 'I'	:
			if (ItemOk ())
				return (EXIT_SUCCESS);
			break;
	}

	/*------------------
	| if function does
	| not return FALSE
	| then return TRUE
	-------------------*/
	return (TRUE);
}

void
PrintInprs (
 void)
{
	int		count;
	char	*sptr;
	int		fstTime2 = TRUE;

	memset (&inpr_rec, 0, sizeof (inpr));
	inpr_rec.hhgu_hash = ingu_rec.hhgu_hash;
	cc = find_rec (inpr, &inpr_rec, GTEQ, "r");
	if (cc || inpr_rec.hhgu_hash != ingu_rec.hhgu_hash)
	{
		print_at (15, 4, ML(mlSkMess571));
		PauseForKey (0, 0, "", 0);
		return;
	}
	OpenPrnt ();

	strcpy (QuitStr, "Quit                ");
	/*-----------------------
	| now loop thru inprs
	---------------------------*/
	abc_selfield (inpr, "inpr_hhbr_hash");
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (sumr, "sumr_hhsu_hash");

	memset (&inpr_rec, 0, sizeof (inpr));
	cc = find_hash (inpr, &inpr_rec, GTEQ, "r", 0L);

	while (!cc)
	{
		if (inpr_rec.price_type != ingu_rec.price_type ||
			inpr_rec.hhgu_hash != ingu_rec.hhgu_hash)
		{
			cc = find_hash (inpr, &inpr_rec, NEXT, "r", 0L);
			continue;
		}
		/*-----------------
		| if inpr is in range
		--------------------*/
		if (!IsOutOfRange ())
			SaveAud ();

		cc = find_hash (inpr, &inpr_rec, NEXT, "r", 0L);
	}

	abc_selfield (inpr, "inpr_hhgu_hash");
	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (sumr, (CR_FIND) ? "sumr_id_no3" : "sumr_id_no");

	fsort = sort_sort (fsort, "price");
	sptr = sort_read (fsort);

	while (sptr)
	{
		dsp_process ("Printing ", sptr);
		fprintf (fout, ".LRP5\n");
		if (!fstTime2)
		{
			fprintf (fout, "|-------------------------------------------");
			fprintf (fout, "--------------------------------------------");
			fprintf (fout, "--------------------------------------------");
			fprintf (fout, "-------------------------|\n");
		}
		fstTime2 = FALSE;

		fprintf (fout, "| Item : %16.16s", sptr);	
		fprintf (fout, "- %40.40s", sptr + 16);
		fprintf (fout, "Br :  %2.2s ", sptr + 56);
		fprintf (fout, "Wh : %2.2s ", sptr + 58);
		fprintf (fout, "Cus Type : %3.3s", sptr + 60);
		fprintf (fout, "%59.59s|\n", " ");
		fprintf (fout, "|%156.156s|\n", " ");
		
		strcpy (err_str, "");
		strcat  (err_str, "|  Quantity Breaks -   ");
		strncat (err_str, "           ", 11);

		for (count = 0; count < numOfBrks; count++)	/*  Qty Brk info	*/
			strncat (err_str, sptr + (64 + (count * 11)), 11);
		
		count = 157 - (strlen (err_str));
		fprintf (fout, "%s%*.*s|\n", err_str, count, count, " ");

		strcpy (err_str, "");
		if (LIVE)
			strcat  (err_str, "|  Live Prices     -   ");
		else
			strcat  (err_str, "|  Future Prices   -   ");
		for (count = 0; count <= numOfBrks; count++)
			strncat (err_str, sptr + (273 + (count * 11)), 11);

		count = 157 - (strlen (err_str));
		fprintf (fout, "%s%*.*s|\n", err_str, count, count, " ");

		sptr = sort_read (fsort);
	}
	cc = sort_delete (fsort, "price");
	if (cc)
	{
		sleep (sleepTime);
		/*-------------------
		| try again
		-------------*/
		cc = sort_delete (fsort, "price");
		if (cc)
			sys_err ("Error In Deleting Sort File", errno, PNAME);
	}
	fprintf (fout, ".EOF\n");
	pclose (fout);
	fstTime 	= TRUE;
}

void
CalcUplift (
 void)
{
	int	i;
	for (i = 0; i < 10; i++)
	{
		inpr_rec.price [i] *= (1.00 + (ingu_rec.uplift / 100));
		DoRounding (i);
	}
}

void
DoRounding (
 int i)
{
	int	count;
	/*----------------------------
	| if first price == 0.00 this
	| means no rounding
	----------------------------*/
	if (ingu_rec.price [0] == 0.00)
		return;

	/*----------------------
	| work top down 5 to 1
	-----------------------*/
	for (count = 4; count >= 0; count--)
	{
		if (ingu_rec.price[count] == 0.00)
			continue;
		
		/*-----------------------------
		| if not the first rounding
		| level compare to see if between
		| next lowest and present rounding
		| values, if so roundIT.
		-------------------------------*/
		if (count &&
			inpr_rec.price [i] > ingu_rec.price[count - 1] && 
			inpr_rec.price [i] <= ingu_rec.price[count])
		{
			inpr_rec.price[i] = Roundit(ingu_rec.rounding, 
											 	ingu_rec.sig[count],
											 	inpr_rec.price[i]);
			break;
		}

		/*------------------------------
		| if this is the first rounding value
		| then just check that lower than first
		| rounding level, if so roundIT.
		-------------------------------------*/
		if (!count && inpr_rec.price [i] <= ingu_rec.price[count])
		{
			inpr_rec.price[i] = Roundit(ingu_rec.rounding, 
											 	ingu_rec.sig[count],
											 	inpr_rec.price[i]);
			break;
		}
	}
}

int
BuyGrpOk (
 void)
{
	if (strcmp (inmr_rec.mr_buygrp, local_rec.sbuygrp) >= 0 &&
		strcmp (inmr_rec.mr_buygrp, local_rec.ebuygrp) < 1)
			return (TRUE);
	
	return (FALSE);
}

int
SellGrpOk (
 void)
{
	if (strcmp (inmr_rec.mr_sellgrp, local_rec.ssellgrp) >= 0 &&
		strcmp (inmr_rec.mr_sellgrp, local_rec.esellgrp) < 1)
			return (TRUE);
	
	return (FALSE);
}

int
SuppOk (
 void)
{
	/*---------------------------------------
	| try to find priority one inis record. |
	---------------------------------------*/
	inis_rec.hhbr_hash	=	inmr_rec.mr_hhbr_hash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec.tco_no);
	strcpy (inis_rec.br_no, comm_rec.test_no);
	strcpy (inis_rec.wh_no, comm_rec.tcc_no);
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
	if (cc)
	{
		inis_rec.hhbr_hash	=	inmr_rec.mr_hhbr_hash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec.tco_no);
		strcpy (inis_rec.br_no, comm_rec.test_no);
		strcpy (inis_rec.wh_no, comm_rec.tcc_no);
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		inis_rec.hhbr_hash	=	inmr_rec.mr_hhbr_hash;
		strcpy (inis_rec.sup_priority, "W1");
		strcpy (inis_rec.co_no, comm_rec.tco_no);
		strcpy (inis_rec.br_no, comm_rec.test_no);
		strcpy (inis_rec.wh_no, comm_rec.tcc_no);
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
	}
	if (cc)
		return (FALSE);

	/*----------------------------------------------------
	| next find supplier and see if is in range of ingu. |
	----------------------------------------------------*/
	sumr_rec.sm_hhsu_hash	=	inis_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sumr, "DBFIND");
	
	if (strcmp (sumr_rec.sm_crd_no, local_rec.ssupp) >= 0 &&
		strcmp (sumr_rec.sm_crd_no, local_rec.esupp) < 1)
			return (TRUE);
		
	return (FALSE);
}

int
ItemOk (
 void)
{
	if (strcmp (inmr_rec.mr_item_no, local_rec.sitem) >= 0 &&
		strcmp (inmr_rec.mr_item_no, local_rec.eitem) < 1)
			return (TRUE);

	return (FALSE);
}

/*---------------------
| this is copied from
| rnd_mltpl.h
--------------------*/
double	
Roundit (
 char *type, 
 float sig, 
 float value)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (value == 0.00)
		return(0.00);

	if (sig == 0.00)
		return(value);

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (value / sig);
	if (ceil (wrk_qty) == wrk_qty)
		return(value);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (type[0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double)(value / sig);
		wrk_qty = ceil(wrk_qty);
		value = (double)(wrk_qty * sig);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double)(value / sig);
		wrk_qty = floor(wrk_qty);
		value = (double)(wrk_qty * sig);
		break;

	case 'N':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double)value;
		wrk_qty = (up_qty / (double)sig);
		wrk_qty = ceil(wrk_qty);
		up_qty = (double)(wrk_qty * sig);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double)value;
		wrk_qty = (down_qty / (double)sig);
		wrk_qty = floor(wrk_qty);
		down_qty = (double)(wrk_qty * sig);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double)value) <= ((double)value - down_qty))
			value = (double)up_qty;
		else
			value = (double)down_qty;

		break;

	default:
		return (value);
	}

	return(value);
}

void
PrintAud (
 void)
{
	char	*sptr;
	int		count;

	if (fstTime)
		OpenAud ();


	/*----------------------------
	| read in sorted information
	| for info on offsets see doco
	-----------------------------*/
	sptr = sort_read (fsort);

	while (sptr)
	{
		dsp_process ("Updating ", sptr);
		fprintf (faud, ".LRP5\n");
		if (!fstTime)
		{
			/*
			fprintf (faud, "%40.40s", " ");
			*/
			fprintf (faud, "|-------------------------------------------");
			fprintf (faud, "--------------------------------------------");
			fprintf (faud, "--------------------------------------------");
			fprintf (faud, "-------------------------|\n");
		}
		fstTime = FALSE;

		fprintf (faud, "| Item : %16.16s", sptr);	
		fprintf (faud, "- %40.40s", sptr + 16);
		fprintf (faud, "Br :  %2.2s ", sptr + 56);
		fprintf (faud, "Wh : %2.2s ", sptr + 58);
		fprintf (faud, "Cus Type : %3.3s", sptr + 60);
		fprintf (faud, "%59.59s|\n", " ");
		fprintf (faud, "|%156.156s|\n", " ");
		
		strcpy (err_str, "");
		strcat  (err_str, "|  Quantity Breaks -   ");
		strncat (err_str, "           ", 11);
		for (count = 0; count < numOfBrks; count++)	/*  Qty Brk info	*/
			strncat (err_str, sptr + (64 + (count * 11)), 11);
		
		count = 157 - (strlen (err_str));
		fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

		strcpy (err_str, "");
		strcat  (err_str, "|  Old Prices      -   ");
		for (count = 0; count <= numOfBrks; count++)	/*  Old Prices  	*/
			strncat (err_str, sptr + (163 + (count * 11)), 11);

		count = 157 - (strlen (err_str));
		fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

		strcpy (err_str, "");
		strcat  (err_str, "|  New Prices      -   ");
		for (count = 0; count <= numOfBrks; count++)	/*  New Prices   */
			strncat (err_str, sptr + (273 + (count * 11)), 11);

		count = 157 - (strlen (err_str));
		fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

		sptr = sort_read (fsort);
	}
	cc = sort_delete (fsort, "price");
	if (cc)
	{
		sleep (sleepTime);
		/*-------------------
		| try again
		-------------*/
		cc = sort_delete (fsort, "price");
		if (cc)
			sys_err ("Error In Deleting Sort File", errno, PNAME);
	}
	fstTime 	= TRUE;
}

void
OpenAud (
 void)
{
	if ((faud = popen ("pformat", "w")) == (FILE *) NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (faud, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (faud, ".LP%d\n", local_rec.lpno);

	fprintf (faud, ".17\n");
	fprintf (faud, ".PI12\n");
	fprintf (faud, ".L158\n");

	fprintf(faud,".ECompany %s - %s\n",comm_rec.tco_no,comm_rec.tco_name);
	fprintf (faud, ".EGLOBAL PRICE UPDATE Audit\n");
	fprintf (faud, ".E AS AT %s", SystemTime ());
	fprintf (faud, 
			".EUpdated By File %6.6s - %s\n", 
			ingu_rec.file_code,
			ingu_rec.file_desc);
	fprintf (faud, ".EFor Branch %s\n", local_rec.brdesc);
	fprintf (faud, ".EFor Warehouse %s\n", local_rec.whdesc);
	fprintf (faud, 
			".EFor Currency %s - %s\n",
			ingu_rec.curr_code,
			pocr_rec.description);
	fprintf (faud, 
			".EFor Price Type %d - %s\n",
			local_rec.price_type,
			local_rec.price_desc);

	fprintf (faud, ".C For Range of %ss\n", local_rec.rangedesc);

	switch (ingu_rec.apply_to[0])
	{
	case 'B'	:
		fprintf (faud, 
				".C From %s - %s\n",
				local_rec.sbuygrp, 
				local_rec.sbuydesc);
		fprintf (faud, 
				".C to %s - %s\n",
				local_rec.ebuygrp,
				local_rec.ebuydesc);
		break;
	case 'S'	:
		fprintf (faud, 
				".C From %s - %s\n",
				local_rec.ssellgrp, 
				local_rec.sselldesc);
		fprintf (faud, 
				".C to %s - %s\n",
				local_rec.esellgrp,
				local_rec.eselldesc);
		break;
	case 'P'	:
		fprintf (faud, 
				".C From %s - %s\n",
				local_rec.ssupp, 
				local_rec.ssuppdesc);
		fprintf (faud, 
				".C to %s - %s\n",
				local_rec.esupp,
				local_rec.esuppdesc);
		break;
	case 'I'	:
		fprintf (faud, 
				".C From %s - %s\n",
				local_rec.sitem, 
				local_rec.sitemdesc);
		fprintf (faud, 
				".C to %s - %s\n",
				local_rec.eitem,
				local_rec.eitemdesc);
		break;
	}
	fprintf (faud, ".B1\n");

	fprintf (faud, "=========================================================");
	fprintf (faud, "=========================================================");
	fprintf (faud, "============================================\n");

	fprintf (faud, ".R=======================================================");
	fprintf (faud, "=========================================================");
	fprintf (faud, "==============================================\n");
}

void
OpenPrnt (
 void)
{
	if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".16\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf(fout,".ECompany %s - %s\n",comm_rec.tco_no,comm_rec.tco_name);

	if (LIVE)
	{
		fprintf (fout, ".E LIVE PRICE REPORT \n");
		fprintf (fout, ".B1\n");
	}
	else
	{
		fprintf (fout, ".E FUTURE PRICE REPORT - %s\n", ingu_rec.file_code);
		fprintf (fout, ".E EFFECTIVE DATE %s\n", DateToString (ingu_rec.eff_date));
	}

	fprintf (fout, ".E AS AT %s", SystemTime ());
	fprintf (fout, ".EFor Branch %s\n", local_rec.brdesc);
	fprintf (fout, ".EFor Warehouse %s\n", local_rec.whdesc);
	fprintf (fout, 
			".EFor Currency %s - %s\n",
			ingu_rec.curr_code,
			pocr_rec.description);
	fprintf (fout, 
			".EFor Price Type %d - %s\n",
			local_rec.price_type,
			local_rec.price_desc);

	fprintf (fout, ".C For Range of %ss\n", local_rec.rangedesc);

	switch (ingu_rec.apply_to[0])
	{
	case 'B'	:
		fprintf (fout, 
				".C From %s - %s\n",
				local_rec.sbuygrp, 
				local_rec.sbuydesc);
		fprintf (fout, 
				".C to %s - %s\n",
				local_rec.ebuygrp,
				local_rec.ebuydesc);
		break;
	case 'S'	:
		fprintf (fout, 
				".C From %s - %s\n",
				local_rec.ssellgrp, 
				local_rec.sselldesc);
		fprintf (fout, 
				".C to %s - %s\n",
				local_rec.esellgrp,
				local_rec.eselldesc);
		break;
	case 'P'	:
		fprintf (fout, 
				".C From %s - %s\n",
				local_rec.ssupp, 
				local_rec.ssuppdesc);
		fprintf (fout, 
				".C to %s - %s\n",
				local_rec.esupp,
				local_rec.esuppdesc);
		break;
	case 'I'	:
		fprintf (fout, 
				".C From %s - %s\n",
				local_rec.sitem, 
				local_rec.sitemdesc);
		fprintf (fout, 
				".C to %s - %s\n",
				local_rec.eitem,
				local_rec.eitemdesc);
		break;
	}
	fprintf (fout, ".B1\n");

	fprintf (fout, "=========================================================");
	fprintf (fout, "=========================================================");
	fprintf (fout, "============================================\n");

	fprintf (fout, ".R=======================================================");
	fprintf (fout, "=========================================================");
	fprintf (fout, "==============================================\n");
}

void
SaveAud (
 void)
{
    char cBuffer [512];

	if (fstTime)
	{
		fsort = sort_open ("price");
		if (fsort == (FILE *) NULL)
			sys_err ("Error In Opening Sort File", errno, PNAME);
		fstTime = FALSE;
	}

	sprintf (cBuffer, "%16.16s%40.40s%2.2s%2.2s%3.3s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n",
				inmr_rec.mr_item_no,
				inmr_rec.mr_description,
				inpr_rec.br_no,
				inpr_rec.wh_no,
				inpr_rec.cust_type,
				(inpr_rec.qty_brk[0]),
				(inpr_rec.qty_brk[1]),
				(inpr_rec.qty_brk[2]),
				(inpr_rec.qty_brk[3]),
				(inpr_rec.qty_brk[4]),
				(inpr_rec.qty_brk[5]),
				(inpr_rec.qty_brk[6]),
				(inpr_rec.qty_brk[7]),
				(inpr_rec.qty_brk[8]),
				DOLLARS (inpr2_rec.price[0]),       /*    new prices */
				DOLLARS (inpr2_rec.price[1]),
				DOLLARS (inpr2_rec.price[2]),
				DOLLARS (inpr2_rec.price[3]),
				DOLLARS (inpr2_rec.price[4]),
				DOLLARS (inpr2_rec.price[5]),
				DOLLARS (inpr2_rec.price[6]),
				DOLLARS (inpr2_rec.price[7]),
				DOLLARS (inpr2_rec.price[8]),
				DOLLARS (inpr2_rec.price[9]),
				DOLLARS (inpr_rec.price[0]),		/* old prices */
				DOLLARS (inpr_rec.price[1]),
				DOLLARS (inpr_rec.price[2]),
				DOLLARS (inpr_rec.price[3]),
				DOLLARS (inpr_rec.price[4]),
				DOLLARS (inpr_rec.price[5]),
				DOLLARS (inpr_rec.price[6]),
				DOLLARS (inpr_rec.price[7]),
				DOLLARS (inpr_rec.price[8]),
				DOLLARS (inpr_rec.price[9]));
    sort_save (fsort, cBuffer);
}


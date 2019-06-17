/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_mrmaint.c,v 5.15 2002/07/18 07:15:54 scott Exp $
|  Program Name  : ( sk_mrmaint.c   )                                 |
|  Program Desc  : ( Stock Master Add/ Update.                    )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_mrmaint.c,v $
| Revision 5.15  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.14  2002/04/11 03:46:22  scott
| Updated to add comments to audit files.
|
| Revision 5.13  2002/04/11 01:42:08  scott
| Changes audit file name.
|
| Revision 5.12  2002/02/26 05:45:21  cha
| Cancelled S/C 823. Rolled back to version 5.10.
|
| Revision 5.10  2002/01/31 03:48:14  cha
| S/C 716. Fixed Inex lines not refreshing.
|
| Revision 5.9  2001/11/22 08:41:15  cha
| Updated to fix uninitialized fields in Oracle.
|
| Revision 5.8  2001/11/06 07:10:00  robert
| Updated to fixed addition of extra line in inex table.
|
| Revision 5.7  2001/11/05 01:40:45  scott
| Updated from Testing.
|
| Revision 5.6  2001/10/05 03:00:37  cha
| Added code to produce audit files.
|
| Revision 5.5  2001/08/09 09:19:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.4  2001/08/06 23:45:23  scott
| RELEASE 5.0
|
| Revision 5.3  2001/07/25 02:19:15  scott
| Update - LS10.5
|
| Revision 4.4  2001/05/29 02:34:41  scott
| Updated to ensure blank items cannot be created.
| Updated to ensure first description is not blank.
| Updated prompt help to be more meaningful.
| Updated to improve look of search fields.
| Updated to add descriptions to costing fields.
| Updated to ensure weight and volume conform to standard.
| Updated to ensure inventory default is defined.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_mrmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mrmaint/sk_mrmaint.c,v 5.15 2002/07/18 07:15:54 scott Exp $";

#define	SCN_MAIN		1
#define	SCN_FLAGS		2
#define	SCN_SUPPLIER	3
#define	SCN_EXDESC		4
#define	SCN_BOM			5

#define TXT_REQD
#define MAXSCNS		6
#define MAXWIDTH	200
#include <pslscr.h>
#include <number.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <DBAudit.h>


#define		NON_STOCK	(inmr_rec.inmr_class [0] == 'N' || \
					     inmr_rec.inmr_class [0] == 'Z' || \
					     inmr_rec.inmr_class [0] == 'P' || \
					     inmr_rec.inmr_class [0] == 'K')

#define	SLEEP_TIME	2

#include	"schema"

struct bmmsRecord		bmms_rec;
struct commRecord		comm_rec;
struct ccmrRecord		ccmr_rec;
struct esmrRecord		esmr_rec;
struct excfRecord		excf_rec;
struct inasRecord		inas_rec;
struct inccRecord		incc_rec;
struct inedRecord		ined_rec;
struct ineiRecord		inei_rec;
struct ingdRecord		ingd_rec;
struct inisRecord		inis_rec;
struct inmdRecord		inmd_rec;
struct ingpRecord		ingp_rec;
struct inmrRecord		inmr_rec;
struct inmrRecord		inmr1_rec;
struct inmrRecord		inmr2_rec;
struct inmrRecord		inmr3_rec;
struct inmrRecord		inmr4_rec;
struct inexRecord		inex_rec;
struct inumRecord		inum_rec;
struct inumRecord		inum1_rec;
struct inumRecord		inum2_rec;
struct inumRecord		inum3_rec;
struct inumRecord		inum4_rec;
struct inuvRecord		inuv_rec;
struct podtRecord		podt_rec;
struct polhRecord		polh_rec;
struct sumrRecord		sumr_rec;
struct pocrRecord		pocr_rec;
struct srskRecord		srsk_rec;
struct posdtupRecord	posdtup_rec;

	char	*data	= "data",
	    	*inmr1	= "inmr1",
	    	*inmr2	= "inmr2",
	    	*inmr3	= "inmr3",
	    	*inmr4	= "inmr4",
	    	*inum2	= "inum2",
	    	*inum3	= "inum3";

#define MAX_BR		99
#define	INPUT		(prog_status == ENTRY)

	extern	char	*numeric;

   	int 	newItem 			= 0,
			envVarSkBatchCont 	= FALSE,
			envVarCrCo 			= 0,
			envVarCrFind 		= 0,
			envVarGstInclusive 	= 0,
			envVarSkGrades 		= FALSE,
			envVarQcApply 		= FALSE,
			envVarSoDiscRev		= FALSE,
			changeInccSort 		= 0,
			noSupplier 			= TRUE,
			noMasterWh 			= TRUE,
			max_branches 		= 0,
			manufacturedItem	= FALSE,
			loop_cnt			= 0,
			needToChgItemNo		= 0,
			prevInexLineCnt		= 0;

	char	envVarGst [2],
			envVarCurrCode [4],
			fob_cost_prmpt [31],
			lcl_cost_prmpt [31],
			discont_text [41],
			old_active_status [2],
			branchNumber [3],
			gstPercent [7],
			envVarGstTaxName [4],
			gstTaxPrompt [30],
			temp_sort [29],
			envVarSkQuickCode [9],
			sr_br [MAX_BR] [3];

	char	*scn_desc [] = {
				"Inventory Maintenance Screen 1",
				"Inventory Maintenance Screen 2",
				"Inventory Supplier Screen",
				"Extra Description Maintenance",
				"Manufacturing Screen"
	};

	struct	{
		char	*sourceCode;
		char	*sourceDesc;
	} source [] = {
		{"BM","(Bulk Manufactured)     "},
		{"BP","(Bulk Product)          "},
		{"MC","(Manufactured Component)"},
		{"MP","(Manufactured Product)  "},
		{"PP","(Purchased Product)     "},
		{"RM","(Raw Material)          "},
		{""," "},
	};

	struct	{
		char	*costCode;
		char	*costDesc;
	} costStruct [] = {
		{"F",	"(FIFO)         "},
		{"I",	"(LIFO)         "},
		{"L",	"(Last cost)    "},
		{"A",	"(Average Cost) "},
		{"T",	"(Standard cost)"},
		{"P",	"(Previous cost)"},
		{"S",	"(Serial cost)  "},
		{""," "},
	};
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	src_desc [27];
	char	costDesc [16];
	char	std_uom [5];
	char	alt_uom [5];
	char	pur_uom [5];
	char	st2_uom [5];
	char	lot_ctrl [2];
	char	lprev_item [17];
	char	lsup_item [17];
	char	lsup_desc [41];
	char	ex_desc [41];
	char	sup_part [17];
	double	fob_cost;
	double	std_cost;
	float	volume;
	float	weight;
	float	min_order;
	float	norm_order;
	float	ord_multiple;
	float	pallet_size;
	float	lead_time;
	float	alt_cnv_fact;
	float	pur_cnv_fact;
	float	grade_pc;
	double	exch_rate;
	char	selldesc [41];
	char	buydesc [41];
	char	curr_desc [41];
	double	eoq;
	char	dfltQty [15];
} local_rec;

static	struct	var	vars [] =
{
	{SCN_MAIN, LIN, "itemno",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number      ", "Use search keys to view existing items.",
		 NE, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{SCN_MAIN, LIN, "desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description      ", "Item description should be input.",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{SCN_MAIN, LIN, "desc2",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description      ", "Second line of description not required.",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.description2},
	{SCN_MAIN, LIN, "acronym",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", inmr_rec.item_no, "Alpha Code       ", "Enter item alpha code if available.",
		 NO, NO,  JUSTLEFT, "", "", inmr_rec.alpha_code},
	{SCN_MAIN, LIN, "alternate",	 7, 36, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Alternate Number ", "Enter a valid alternate of <return> for none.",
		 NO, NO,  JUSTLEFT, "", "", inmr_rec.alternate},
	{SCN_MAIN, LIN, "quick_code",	 8, 2, CHARTYPE,
		envVarSkQuickCode, "          ",
		" ", " ", "Short Code       ", "Enter Item short code, duplicates not allowed.",
		NO, NO,  JUSTLEFT, "", "", inmr_rec.quick_code},
	{SCN_MAIN, LIN, "brandNumber",	 8, 36, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Brand Number     ", "Enter item brand Code.",
		 NO, NO,  JUSTLEFT, "", "", inmr_rec.maker_no},
	{SCN_MAIN, LIN, "comm_code",	 9, 2, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", " ", "Commodity Code   ", "Enter item Commodity. <return> for none",
		 NO, NO,  JUSTLEFT, "", "", inmr_rec.commodity},
	{SCN_MAIN, LIN, "source",	10, 2, CHARTYPE,
		"UU", "          ",
		" ", "PP","Source           ", "Use search Key for valid source Codes and descriptions. ",
		NO, NO,  JUSTLEFT, "", "", inmr_rec.source},
	{SCN_MAIN, LIN, "src_desc",	10, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_desc},
	{SCN_MAIN, LIN, "inmr_class",	 12, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Class            ", "Enter Item Class (A-Z)",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", inmr_rec.inmr_class},
	{SCN_MAIN, LIN, "qcReqd", 	 12, 36, CHARTYPE, 
		"U", "          ", 
		" ", "N", "QC Check Reqd.   ", "QC Check Required. Yes or No (defaults to No).", 
		NO, NO, JUSTLEFT, "YN", "", inmr_rec.qc_reqd},
	{SCN_MAIN, LIN, "category",	13, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category         ", "Use Search key for valid categories.",
		NO, NO,  JUSTLEFT, "", "", inmr_rec.category},
	{SCN_MAIN, LIN, "cat_desc",	13, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{SCN_MAIN, LIN, "sellgrp",	15, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Selling Group    ", " ",
		 YES, NO,  JUSTLEFT, "", "", inmr_rec.sellgrp},
	{SCN_MAIN, LIN, "selldesc",	15, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.selldesc},
	{SCN_MAIN, LIN, "buygrp",	16, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Buying Group     ", " ",
		 YES, NO,  JUSTLEFT, "", "", inmr_rec.buygrp},
	{SCN_MAIN, LIN, "buydesc",	16, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.buydesc},
	{SCN_MAIN, LIN, "active_status",	 18, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Active Status    ", "Enter Active Status. Full Search Available",
		NO, NO,  JUSTLEFT, "", "", inmr_rec.active_status},
	{SCN_MAIN, LIN, "act_desc",	 18, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", inas_rec.description},
	{SCN_FLAGS, LIN, "abc_code",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "ABC Code           ", "Enter ABC Code (A-D)",
		NO, NO,  JUSTLEFT, "ABCD", "", inmr_rec.abc_code},
	{SCN_FLAGS, LIN, "abc_update",	 3, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "ABC Update         ", "Automatic Update of ABC Code (Y/N)",
		NO, NO,  JUSTLEFT, "YN", "", inmr_rec.abc_update},
	{SCN_FLAGS, LIN, "bo_flag",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "B/Ord Accepted     ", "Y(es) = B/Orders are allowed, N(o) = not allowed, F(ull) B/Orders Only.",
		NO, NO,  JUSTLEFT, "YNF", "", inmr_rec.bo_flag},
	{SCN_FLAGS, LIN, "bo_release",	 4, 36, CHARTYPE,
		"U", "          ",
		" ", "A", "B/Order Release    ", "Enter M(anual) A(utomatic).",
		NO, NO,  JUSTLEFT, "AM", "", inmr_rec.bo_release},
	{SCN_FLAGS, LIN, "serial",	5, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Serial Item        ", "Y(es) N(o)",
		NO, NO, JUSTRIGHT, "YN", "", inmr_rec.serial_item},
	{SCN_FLAGS, LIN, "costingFlag",	5, 36, CHARTYPE,
		"U", "          ",
		" ", "F", "Costing Type       ", "Use search key for valid costing codes and descriptions",
		YES, NO, JUSTRIGHT, "FILATP", "", inmr_rec.costing_flag},
	{SCN_FLAGS, LIN, "costDesc",	5, 57, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.costDesc},
	{SCN_FLAGS, LIN, "lot_ctrl",	6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Lot Control (Y/N)  ", " Enter Y(es) if Lot Control Required.",
		 NO, NO, JUSTRIGHT, "YN", "", inmr_rec.lot_ctrl},
	{SCN_FLAGS, LIN, "dec_pt",	6, 36, INTTYPE,
		"N", "          ",
		" ", "2", "Decimal Places     ", "(0-6) Number of decimal places for Stock. ",
		NO, NO, JUSTRIGHT, "0", "6", (char *)& inmr_rec.dec_pt},
	{SCN_FLAGS, LIN, "schg",	7, 2, CHARTYPE,
		"U", "          ",
		" ", "0", "Surcharge Flag     ", "0-No Surcharge or 1-4 Use Surcharge item [SU1-4]",
		 NI, NO,  JUSTLEFT, "S01234", "", inmr_rec.schg_flag},
	{SCN_FLAGS, LIN, "grade",	7, 36, CHARTYPE,
		"U", "          ",
		" ", " ", "Inventory Grade    ", "Enter Inventory Grade",
		ND, NO, JUSTLEFT, "", "", inmr_rec.grade},
	{SCN_FLAGS, LIN, "grade_pc",	7, 65, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.grade_pc},
	{SCN_FLAGS, LIN, "std_uom",	 9, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "","Standard  UOM      ", "Use Search key for valid unit of Measures ",
		NO, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{SCN_FLAGS, LIN, "alt_uom",	 9, 36, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Alternate UOM      ", "Use Search key for valid unit of Measures. ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.alt_uom},
	{SCN_FLAGS, LIN, "alt_cnv_fact",	 10, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "0.00", "Conv Factor        ", "Enter Conversion Factor. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.alt_cnv_fact},
	{SCN_FLAGS, LIN, "outer",	 10, 36, FLOATTYPE,
		"NNNNN.N", "          ",
		" ", "1.00", "Pricing Conv       ", " Cost & Sale quantity / price conversion.",
		YES, NO, JUSTRIGHT, "1", "99999.9", (char *)& inmr_rec.outer_size},
	{SCN_FLAGS, LIN, "pack_size",	11, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", local_rec.std_uom, "Pack Size          ", "Note that this is a narrative field only.",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.pack_size},
	{SCN_FLAGS, LIN, "gst",	13, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", gstPercent, gstTaxPrompt, " ",
		NO, NO, JUSTRIGHT, "0", "100.00", (char *)& inmr_rec.gst_pc},
	{SCN_FLAGS, LIN, "taxp",	13, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Tax %              ", " ",
		NO, NO, JUSTRIGHT, "0", "100.00", (char *)& inmr_rec.tax_pc},
	{SCN_FLAGS, LIN, "tax_amt",13, 36, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Last W/sale Price  ", "Input last wholesale price.",
		NO, NO, JUSTRIGHT, "0", "9999999.99", (char *)& inmr_rec.tax_amount},
	{SCN_FLAGS, LIN, "discp", 14, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Discount %         ", " ",
		NO, NO, JUSTRIGHT, "-100.00", "100.00", (char *)& inmr_rec.disc_pc},
	{SCN_FLAGS, LIN, "safety",	14, 36, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Safety Stock       ", "Enter Safety stock in Weeks. ",
		NO, NO, JUSTRIGHT, "0", "99.999", (char *)& inmr_rec.safety_stock},
	{SCN_FLAGS, LIN, "minqty",	15, 2, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Minimum Quantity   ", " ",
		NO, NO, JUSTRIGHT, "0", "999999.99", (char *)& inmr_rec.min_quan},
	{SCN_FLAGS, LIN, "reorder_flag",	 15, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Reorder Item (Y/N) ", "Y(es) = Reorder item, N(o) = Prompt for reorder.",
		NO, NO,  JUSTLEFT, "YN", "", inmr_rec.reorder},
	{SCN_FLAGS, LIN, "ex_code",	17, 2, CHARTYPE,
		"UUU", "          ",
		" ", "", "Extra Desc Code    ", " ",
		 NO, NO,  JUSTLEFT, "", "", inmr_rec.ex_code},
	{SCN_FLAGS, LIN, "ex_desc",	17, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", ined_rec.desc},
	{SCN_FLAGS, LIN, "min_sell_pric",	 18, 2, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Min. Sell Price    ", "Minimum Selling Price",
		NO, NO,  JUSTRIGHT, "0", "99999999.99", (char *)&inmr_rec.min_sell_pric},
	{SCN_FLAGS, LIN, "supitem",	19, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Supercession No    ", " ",
		 NO, NO,  JUSTLEFT, "", "",inmr_rec.supercession},
	{SCN_FLAGS, LIN, "supdesc",	19, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.lsup_desc},
	{SCN_SUPPLIER, LIN, "crd_no",	3,	2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier No        ", " ",
		 NE, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{SCN_SUPPLIER, LIN, "crd_name",	3, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{SCN_SUPPLIER, LIN, "crd_curr_code", 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Currency Code      ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.curr_code},
	{SCN_SUPPLIER, LIN, "crd_curr_desc", 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},
	{SCN_SUPPLIER, LIN, "sup_item",	 6, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ",inmr_rec.item_no, "Supplier Part      ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.sup_part},
	{SCN_SUPPLIER, LIN, "sup_pri",	 6, 42,   CHARTYPE,
		"UN", "          ",
		" ", "C1", "Priority           ", "Default =  C(ompany) Priority 1 ",
		YES, NO,  JUSTLEFT, "CBW123456789", "", inis_rec.sup_priority},
	{SCN_SUPPLIER, LIN, "sup_pur_uom",	 8, 2, CHARTYPE,
		"AAAA", "          ",
		" ", local_rec.std_uom,"Purchase UOM       ", "Use Search key for valid unit of Measures ",
		NE, NO,  JUSTLEFT, "", "", local_rec.pur_uom},
	{SCN_SUPPLIER, LIN, "fob_cost",	 8, 42, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", fob_cost_prmpt, "Enter Latest Price in Suppliers Native Currency. ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.fob_cost},
	{SCN_SUPPLIER, LIN, "sup_std_uom",	 9, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Standard UOM       ", "Use Search key for valid unit of Measures. ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st2_uom},
	{SCN_SUPPLIER, LIN, "std_cost",	 9, 42, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", lcl_cost_prmpt, "",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.std_cost},
	{SCN_SUPPLIER, LIN, "pur_cnv_fact",	 10, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", "Conv Factor        ", "Enter Conversion Factor. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.pur_cnv_fact},
	{SCN_SUPPLIER, LIN, "duty",	 12, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Duty Code          ", "Blank if no duty applies ",
		 NO, NO,  JUSTLEFT, "", "",inmr_rec.duty},
	{SCN_SUPPLIER, LIN, "licence",	 12, 42, CHARTYPE,
		"UU", "          ",
		" ", " ", "Licence Code       ", "Blank if no licence applies",
		 NO, NO,  JUSTLEFT, "", "",inmr_rec.licence},
	{SCN_SUPPLIER, LIN, "dflt_lead",14, 2,  CHARTYPE,
		"U", "          ",
		" ", sumr_rec.ship_method, "Default Ship Meth  ", "Enter default shipment method. A(ir) / S(ea) / L(and).",
		NE, NO, JUSTRIGHT, "LSA", "", inis_rec.dflt_lead},
	{SCN_SUPPLIER, LIN, "lead",	14, 42, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Lead Times         ", "Enter Lead times in days.",
		NO, NO, JUSTRIGHT, "0", "99999.99", (char *) &local_rec.lead_time},
	{SCN_SUPPLIER, LIN, "pallet_size",	15, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Pallet Size        ", " ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.pallet_size},
	{SCN_SUPPLIER, LIN, "min_order",	16, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Min Order Qty      ", " ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.min_order},
	{SCN_SUPPLIER, LIN, "nor_order",	17, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Norm Order Qty     ", " ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.norm_order},
	{SCN_SUPPLIER, LIN, "ord_mult",	18, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Order Lot size     ", " ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.ord_multiple},
	{SCN_SUPPLIER, LIN, "s_weight",	16, 42, FLOATTYPE,
		"NNNNNN.NNNN", "          ",
		" ", "0", "Weight (Supplier)  ", " Weight in Kg ",
		 NO, NO, JUSTRIGHT, "0", "999999.99", (char *)&local_rec.weight},
	{SCN_SUPPLIER, LIN, "weight",	17, 42, FLOATTYPE,
		"NNNNNN.NNNN", "          ",
		" ", "0", "Weight (Master)    ", " Weight in Kg ",
		 NO, NO, JUSTRIGHT, "0", "999999.99", (char *)&inmr_rec.weight},
	{SCN_SUPPLIER, LIN, "volume",	18, 42, FLOATTYPE,
		"NNNNNN.NNN", "          ",
		" ", "0", "Volume             ", "Volume in cubic metres per Unit.",
		NO, NO, JUSTRIGHT, "0", "99999.99", (char *) &local_rec.volume},
	{SCN_EXDESC, TXT, "ex_desc" , 2, 19, 0, "", "          ",
		" ", "", "    EXTRA  DESCRIPTION  MAINTENANCE     ", "",
		17, 40, 100, "", "", local_rec.ex_desc},
	{SCN_BOM, LIN, "dflt_bom", 	3, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Default BOM No   ", "Default BOM number.", 
		NO, NO, JUSTRIGHT, "0", "99999", (char *) &inmr_rec.dflt_bom}, 
	{SCN_BOM, LIN, "dflt_rtg", 	4, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Default RTG No   ", "Default Routing number.", 
		NO, NO, JUSTRIGHT, "0", "99999", (char *) &inmr_rec.dflt_rtg}, 
	{SCN_BOM, LIN, "eoq", 	5, 2, DOUBLETYPE, 
		local_rec.dfltQty, "          ", 
		" ", " ", "Economic Ord Qty ", "Economic Order Quantity.", 
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.eoq}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern	int	TruePosition;
int		envVarPosInstalled 		=	FALSE;
int		envSkMrMaintDup		=	FALSE;

/*=======================
| Function Declarations |
=======================*/
float 	ScreenDisc 				(float);
int  	CalculateConversion 	(int);
int  	GetSupplier 			(void);
int  	Update 					(void);
int  	ValidQuantity 			(double, int);
int  	ValidUOM 				(int);
int  	heading 				(int);
int 	ReadMisc 				(void);
int  	spec_valid 				(int);
void 	AddBranchWarehouse 		(void);
void 	AddInis 				(void);
void 	CheckHiddenFields 		(char *, int);
void 	CheckSourceCode 		(void);
void 	CloseDB 				(void);
void 	GetGrpCodes 			(void);
void	GetCostDesc 			(void);
void 	InccSortChanged 		(void);
void 	OpenDB 					(void);
void 	PosdtupUpdate 			(void);
void 	ReadEsmr 				(void);
void 	ReadInex 				(void);
void	ClearInex				(void);
void 	ReadInas 				(void);
void 	SetfieldValue 			(char *);
void 	SrchExcf 				(char *);
void 	SrchInas 				(char *);
void 	SrchIndg 				(char *);
void 	SrchIned 				(char *);
void 	SrchIngp 				(char *, int);
void 	SrchInum 				(char *);
void 	SrchPodt 				(char *);
void 	SrchPolh 				(char *);
void 	SrchSource 				(void);
void 	SrchCosting				(void);
void 	UpdateInex 				(void);
void 	UpdateInuv 				(long, int);
void 	UpdateSrsk 				(void);
void 	shutdown_prog 			(void);

#include	<FindSumr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv []) 
{
	char	*sptr;
	int		i;

	TruePosition	=	TRUE;

	envVarGstInclusive = atoi (get_env ("GST_INCLUSIVE"));
	sprintf (envVarGst, "%-1.1s", get_env ("GST"));
	sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	sprintf (gstTaxPrompt,"%-3.3s %%              ", envVarGstTaxName);

	sprintf(envVarCurrCode, "%-3.3s", get_env("CURR_CODE"));

	/*--------------------------
	| Set up the Quantity Mask |
	--------------------------*/
	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dfltQty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dfltQty, sptr);

	/*------------------------------
	| Check for POS installation.  |
	------------------------------*/
	sptr = chk_env("POS_INSTALLED");
	envVarPosInstalled = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env("SK_MRMAINT_DUP");
	envSkMrMaintDup = (sptr == (char *)0) ? 0 : atoi (sptr);

	SETUP_SCR (vars);

	sptr = chk_env ("SK_QUICK_CODE");

	if (sptr == (char *)0)
		strcpy (envVarSkQuickCode , "NNNN");
	else
	{
		if (strlen (sptr) > 8)
			* (sptr + 8) = '\0';
		strcpy (envVarSkQuickCode, clip (sptr));
	}
	if (strchr (envVarSkQuickCode, 'U') == (char *) 0 &&
	    strchr (envVarSkQuickCode, 'A') == (char *) 0 &&
	    strchr (envVarSkQuickCode, 'D') == (char *) 0)
		vars [label ("quick_code")].lowval = numeric;

	sptr = chk_env ("SK_GRADES");
	if (sptr && *sptr == 'Y')
		envVarSkGrades = TRUE;
	else
		envVarSkGrades = FALSE;

	sptr = chk_env("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	init_scr ();		
	set_tty (); 	
	_set_masks ("sk_mrmaint.s");	

	sptr = chk_env ("SK_BATCH_CONT");
	envVarSkBatchCont = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	FLD ("qcReqd") = envVarQcApply ? YES : ND;

	for (i = 0; i < 6; i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (SCN_MAIN);			/*  set default values		*/

	/*---------------------------------------
	| Take Out Tax for New-Zealand Clients. |
	---------------------------------------*/
	if (envVarGst [0] == 'Y' || envVarGst [0] == 'y')
	{
		FLD ("taxp") 	= ND;
		FLD ("tax_amt") = ND;
	}
	else
		FLD ("gst") = ND;

	FLD ("grade")    = (envVarSkGrades) 	? YES : ND;
	FLD ("grade_pc") = (envVarSkGrades) 	? NA  : ND;
	FLD ("lot_ctrl") = (envVarSkBatchCont) 	? YES : ND;

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	cc = ReadMisc ();
	if (cc)
	{
		clear();
		print_at (0,0, ML ("No item template has been defined."));
		print_at (1,0, ML ("Templete must be defined to maintain item"));
		sleep (sleepTime);
		shutdown_prog ();
    	return (EXIT_SUCCESS);
	}

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	/*-------------------------
	| Set up Default for Gst. |
	-------------------------*/
	sprintf (gstPercent, "%3.2f", (envVarGstInclusive) ? 0.00 : comm_rec.gst_rate);

	ReadEsmr ();

	SetfieldValue ("inmr_class");
	SetfieldValue ("schg");
	SetfieldValue ("abc_code");
	SetfieldValue ("abc_update");
	SetfieldValue ("serial");
	SetfieldValue ("costingFlag");
	SetfieldValue ("lot_ctrl");
	SetfieldValue ("dec_pt");
	SetfieldValue ("bo_flag");
	SetfieldValue ("bo_release");
	SetfieldValue ("grade");
	SetfieldValue ("std_uom");
	SetfieldValue ("alt_uom");
	SetfieldValue ("sup_pur_uom");
	SetfieldValue ("outer");
	SetfieldValue ("duty");
	SetfieldValue ("licence");
	SetfieldValue ("gst");
	SetfieldValue ("taxp");
	SetfieldValue ("tax_amt");
	SetfieldValue ("minqty");
	SetfieldValue ("safety");
	SetfieldValue ("source");
	SetfieldValue ("min_sell_pric");

	/*-------------------------------------
	| Read inas for code D (Discontinued) |
	-------------------------------------*/
	ReadInas ();

	/*----------------------------------
	| Beginning of input control loop. |
	----------------------------------*/
    prog_exit = 0;
	restart	  =	FALSE;
	while (prog_exit == 0)
	{
		entry_exit			= FALSE;
		edit_exit			= FALSE;
		newItem				= FALSE;
		search_ok			= TRUE;
		init_ok				= (envSkMrMaintDup) ? FALSE : TRUE;
		manufacturedItem	= FALSE;
		restart				= FALSE;

		sprintf (fob_cost_prmpt, "Sup. Cost  (%-3.3s)    ", sumr_rec.curr_code);
		sprintf (lcl_cost_prmpt, "Lcl. Cost  (%-3.3s)    ", envVarCurrCode);

		if (!envSkMrMaintDup)
			init_vars (SCN_MAIN);

		init_vars (SCN_EXDESC);
		lcount [SCN_EXDESC] = 0;

		FLD ("itemno") 	= NE;
		needToChgItemNo = FALSE;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (SCN_MAIN);
		entry (SCN_MAIN);
		if (prog_exit || restart)
			continue;

		if (!newItem)
			ReadInex ();

		if (newItem)
		{
			heading (SCN_FLAGS);
			entry (SCN_FLAGS);
			if (restart)
				continue;

			heading (SCN_EXDESC);
			entry (SCN_EXDESC);
			if (restart)
				continue;

			if (noMasterWh == FALSE && !NON_STOCK)
			{
				heading (SCN_SUPPLIER);
				entry (SCN_SUPPLIER);
				if (restart)
					continue;
			}
			if (manufacturedItem)
			{
				heading (SCN_BOM);
				entry (SCN_BOM);
				if (restart)
					continue;
			}
		}
		else
		{
			strcpy (inmr4_rec.co_no, comm_rec.co_no);
			strcpy (local_rec.lsup_item,inmr_rec.supercession);
			strcpy (inmr4_rec.item_no,inmr_rec.supercession);
			cc = find_rec (inmr4, &inmr4_rec, COMPARISON, "r");
			if (!cc)
				strcpy (local_rec.lsup_desc, inmr4_rec.description);

			strcpy (excf_rec.co_no, comm_rec.co_no);
			strcpy (excf_rec.cat_no,inmr_rec.category);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
	 			file_err (cc, excf, "DBFIND");

			strcpy (ined_rec.co_no, comm_rec.co_no);
			strcpy (ined_rec.code,inmr_rec.ex_code);
			cc = find_rec (ined, &ined_rec, COMPARISON, "r");
			if (cc)
				sprintf (ined_rec.desc, "%-40.40s", " ");

			heading (SCN_MAIN);
			scn_display (SCN_MAIN);
		}
		if (noMasterWh == TRUE || NON_STOCK)
			no_edit (SCN_SUPPLIER);

		if (!manufacturedItem)
			no_edit (SCN_BOM);
		else
			edit_ok (SCN_BOM);

		edit_all ();
		if (restart)
			continue;

		/*-----------------------------
		| Update stock master record. |
		-----------------------------*/
		while (!Update ())
		{
			heading (SCN_MAIN);
			scn_display (SCN_MAIN);
			FLD ("itemno") = YES;
			needToChgItemNo = TRUE;
			edit_all ();
			if (restart)
				break;
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr1, inmr);
	abc_alias (inmr2, inmr);
	abc_alias (inmr3, inmr);
	abc_alias (inmr4, inmr);
	abc_alias (inum2, inum);
	abc_alias (inum3, inum);

	open_rec (inas,  inas_list, INAS_NO_FIELDS, "inas_id_no");
	open_rec (inex,  inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr4, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr1, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_quick_id");
	open_rec (inmr3, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (polh,  polh_list, POLH_NO_FIELDS, "polh_id_no");
	open_rec (ined,  ined_list, INED_NO_FIELDS, "ined_id_no");
	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, (!envVarCrFind)
						? "sumr_id_no" : "sumr_id_no3");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_id_no");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum3, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inuv,  inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_mabr_hash");
	open_rec (ingd,  ingd_list, INGD_NO_FIELDS, "ingd_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");

	if (envVarPosInstalled)
		open_rec (posdtup, posdtup_list, POSDTUP_NO_FIELDS, "posterm_id_no");
	OpenAuditFile ("StockMasterFile.txt");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_unlock (inmr);

	abc_fclose (inas);
	abc_fclose (inmr);
	abc_fclose (inmr4);
	abc_fclose (inmr1);
	abc_fclose (inmr2);
	abc_fclose (inmr3);
	abc_fclose (excf);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (esmr);
	abc_fclose (podt);
	abc_fclose (polh);
	abc_fclose (ined);
	abc_fclose (inis);
	abc_fclose (inex);
	abc_fclose (sumr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (inum3);
	abc_fclose (inuv);
	abc_fclose (bmms);
	abc_fclose (ingd);
	abc_fclose (ingp);
	if (envVarPosInstalled)
		abc_fclose (posdtup);

	SearchFindClose ();
	CloseAuditFile ();
	abc_dbclose (data);
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
int
ReadMisc (void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no2");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.master_wh, "Y");
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	noMasterWh = (cc) ? TRUE : FALSE;

	if (noMasterWh == TRUE)
	{
		abc_selfield (ccmr, "ccmr_id_no");
		
		strcpy (ccmr_rec.co_no,  comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "ccmr", "DBFIND");
	}
	abc_fclose (ccmr);

	open_rec (inmd, inmd_list, INMD_NO_FIELDS, "inmd_co_no");

	strcpy (inmd_rec.co_no, comm_rec.co_no);
	cc = find_rec (inmd, &inmd_rec, COMPARISON, "r");
	if (cc)
	{
		abc_fclose (inmd);
		return (EXIT_FAILURE);
	}
	abc_fclose (inmd);

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int		i;
	int		tmp_scn_no;
	double	save_cnv_fact;


	if (LCHECK ("sellgrp"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str, FALSE);
			return (EXIT_SUCCESS);
		}
		strcpy (ingp_rec.code,inmr_rec.sellgrp);
		strcpy (ingp_rec.type, "S");
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess208));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.selldesc, ingp_rec.desc);
		DSP_FLD ("selldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("buygrp"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str, TRUE);
			return (EXIT_SUCCESS);
		}
		strcpy (ingp_rec.code,inmr_rec.buygrp);
		strcpy (ingp_rec.type, "B");
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess207));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.buydesc, ingp_rec.desc);
		DSP_FLD ("buydesc");
		return (EXIT_SUCCESS);
	}
	/*------------------------
	| Validate Item Number . |
	------------------------*/
	if (LCHECK ("itemno"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (!strlen (clip (inmr_rec.item_no)))
		{
			print_mess (ML(mlSkMess581));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*----------------------------
		| Item number already exists |
		| and we need to change it.  |
		----------------------------*/
		if (needToChgItemNo)
		{
			strcpy (inmr2_rec.co_no,  comm_rec.co_no);
			sprintf (inmr2_rec.item_no, "%-16.16s",inmr_rec.item_no);
			cc = find_rec (inmr3, &inmr2_rec, COMPARISON, "r");
			if (!cc)
			{
				print_mess (ML(mlStdMess204));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			return (EXIT_SUCCESS);
		}

		newItem = FALSE;
		strcpy (inmr_rec.supercession, "                ");
      	strcpy (inmr_rec.co_no, comm_rec.co_no);
      	sprintf (inmr_rec.item_no, "%-16.16s",inmr_rec.item_no);
		if (!F_HIDE (label ("std_uom")))
			FLD ("std_uom")  = NO;

		if (!F_HIDE (label ("alt_uom")))
			FLD ("alt_uom")  = NO;

		if (!F_HIDE (label ("alt_cnv_fact")))
			FLD ("alt_cnv_fact")  = NO;

		if (!F_HIDE (label ("sup_pur_uom")))
			FLD ("sup_pur_uom")  = YES;

		if (!F_HIDE (label ("pur_cnv_fact")))
			FLD ("pur_cnv_fact")  = NO;

		if (!F_HIDE (label ("outer")))
			FLD ("outer")  = YES;
		
		cc = find_rec (inmr1, &inmr_rec, COMPARISON, "w");
		SetAuditOldRec (&inmr_rec, sizeof(inmr_rec));
		if (cc)
		{
			newItem = TRUE;
		}
		else
		{
			if ( inmr_rec.on_hand 	!= 0.00 ||
				 inmr_rec.on_order 	!= 0.00 ||
				 inmr_rec.committed != 0.00 ||
				 inmr_rec.backorder != 0.00 ||
				 inmr_rec.forward 	!= 0.00)
			{
				if (!F_HIDE (label ("outer")))
					FLD ("outer") = NA;
			}

			GetGrpCodes ();

			entry_exit = 1;
			abc_selfield (inis, "inis_hhbr_hash");
			inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			cc = find_rec (inis, &inis_rec, EQUAL, "r");
			if (cc)
			{
				bmms_rec.mabr_hash	=	inmr_rec.hhbr_hash;
				cc = find_rec (bmms, &bmms_rec, EQUAL, "r");
			}
			if (!cc)
			{
				if (!F_HIDE (label ("std_uom")))
					FLD ("std_uom") = NA;

				if (!F_HIDE (label ("alt_uom")))
					FLD ("alt_uom") = NA;

				if (!F_HIDE (label ("alt_cnv_fact")))
					FLD ("alt_cnv_fact") = NA;

				if (!F_HIDE (label ("sup_pur_uom")))
					FLD ("sup_pur_uom") = NO;

				if (!F_HIDE (label ("pur_cnv_fact")))
					FLD ("pur_cnv_fact") = NO;
			}
			abc_selfield (inis, "inis_id_no2");
			inmr_rec.disc_pc = ScreenDisc (inmr_rec.disc_pc);
		}

		sprintf (local_rec.lsup_desc, "%40.40s", " ");
		print_at (22,0,ML(mlSkMess603),inmr_rec.item_no," ");
		if (!strcmp (inmr_rec.supercession, "                "))
			FLD ("supitem") = NA;

		for (i = 0;strlen (source [i].sourceCode);i++)
		{
			if (!strcmp (source [i].sourceCode,inmr_rec.source))
				sprintf (local_rec.src_desc, "%-26.26s", source [i].sourceDesc);
		}
		CheckSourceCode ();

		GetCostDesc ();

		/*------------------------------------
		| Check if item a manufactured item, |
		| if so can display/edit screen 5.   |
		------------------------------------*/
		if (!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP"))
		{
			manufacturedItem = TRUE;
			edit_ok (SCN_BOM);
		}
		else
		{
			manufacturedItem = FALSE;
			no_edit (SCN_BOM);
		}

		inum2_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum3, &inum2_rec, COMPARISON, "r");
		if (cc)
			strcpy (local_rec.std_uom, "    ");
		else
			sprintf (local_rec.std_uom, "%-4.4s", inum2_rec.uom);

		strcpy (local_rec.st2_uom, local_rec.std_uom);

		inum3_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inum3, &inum3_rec, COMPARISON, "r");
		if (cc)
			strcpy (local_rec.alt_uom, "    ");
		else
			sprintf (local_rec.alt_uom, "%-4.4s", inum3_rec.uom);

		if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
			FLD ("alt_cnv_fact") = NA;
		else
		{
			if (FLD ("alt_cnv_fact") != ND)
				FLD ("alt_cnv_fact") = YES;
		}
		local_rec.alt_cnv_fact =inmr_rec.uom_cfactor;
		if (strcmp (local_rec.std_uom, "    "))
			sprintf (lcl_cost_prmpt, "Lcl. Cost/%-4.4s (%-3.3s)", 
					 local_rec.std_uom, envVarCurrCode);

		/*-----------------------------------
		| Lookup grade and store percentage |
		-----------------------------------*/
		if (envVarSkGrades)
		{
			strcpy (ingd_rec.co_no, comm_rec.co_no);
			strcpy (ingd_rec.grade,inmr_rec.grade);
			cc = find_rec (ingd, &ingd_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (inmr_rec.grade, " ");
				local_rec.grade_pc = 0.00;
			}
			else
				local_rec.grade_pc = ingd_rec.writedown;
		}

		if (FLD ("alternate") != ND)
		{
			if (!newItem &&inmr_rec.hhsi_hash != 0L)
				FLD ("alternate") = NA;
			else
				FLD ("alternate") = NO;
		}
		noSupplier = TRUE;
		sumr_rec.hhsu_hash = 0L;

		/*----------------
		| Active Status. |
		----------------*/
		strcpy (inas_rec.co_no, comm_rec.co_no);
		strcpy (inas_rec.act_code,inmr_rec.active_status);
		cc = find_rec (inas, &inas_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inmr_rec.active_status, " ");
 			sprintf (inas_rec.description, "%-40.40s", " ");
		}
		strcpy (old_active_status,inmr_rec.active_status);

		if (!newItem)
			noSupplier = GetSupplier ();

		CheckHiddenFields ("crd_no",   	 (newItem) ? NO : NA);
		CheckHiddenFields ("sup_item", 	 (newItem) ? NO : NA);
		CheckHiddenFields ("fob_cost", 	 (newItem) ? NO : NA);
		CheckHiddenFields ("lead",     	 (newItem) ? NO : NA);
		CheckHiddenFields ("min_order",	 (newItem) ? NO : NA);
		CheckHiddenFields ("nor_order",	 (newItem) ? NO : NA);
		CheckHiddenFields ("ord_mult",	 (newItem) ? NO : NA);
		CheckHiddenFields ("pallet_size",(newItem) ? NO : NA);
		CheckHiddenFields ("volume",  	 (newItem) ? NO : NA);
		CheckHiddenFields ("s_weight",	 (newItem) ? NO : NA);
		CheckHiddenFields ("duty",    	 (newItem) ? NO : NA);
		CheckHiddenFields ("licence",  	 (newItem) ? NO : NA);
		CheckHiddenFields ("sup_pur_uom",(newItem) ? NO : NA);

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Item Description. |
	----------------------------*/
	if (LCHECK ("desc"))
	{
		if (!strlen (clip (inmr_rec.description)))
		{
			print_mess (ML(mlSkMess581));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Item Alpha Code. |
	---------------------------*/
	if (LCHECK ("acronym"))
	{
		if (dflt_used)
		{
			strcpy (inmr_rec.alpha_code, inmr_rec.item_no);
			DSP_FLD ("acronym");
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Alternate Item Number. |
	---------------------------------*/
	if (LCHECK ("alternate"))
	{
		if (dflt_used)
		{
			strcpy (inmr_rec.alternate, "                ");
			DSP_FLD ("alternate");
			return (EXIT_SUCCESS);
		}

		/*-------------------------------
		| Copy current inmr record into |
		| inmr3_rec as stck_search ()   |
		| uses inmr_rec.                |
		-------------------------------*/
		memcpy ((char *)&inmr3_rec, (char *)&inmr_rec, sizeof (struct inmrRecord));

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			strcpy (inmr3_rec.alternate,inmr_rec.item_no);

			/*------------------------------------
			| Copy inmr3_rec back into inmr_rec. |
			------------------------------------*/
			memcpy ((char *)&inmr_rec, 
					   (char *)&inmr3_rec, 
					   sizeof (struct inmrRecord));
	
			return (EXIT_SUCCESS);
		}

		/*-----------------------
		| Validate Item number. |
		-----------------------*/
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", inmr_rec.alternate);
		cc = find_rec (inmr3, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			/*------------------------------------
			| Copy inmr3_rec back into inmr_rec. |
			------------------------------------*/
			memcpy ((char *)&inmr_rec, 
					   (char *)&inmr3_rec, 
					   sizeof (struct inmrRecord));
			return (EXIT_FAILURE);
		}

		if (inmr_rec.inmr_class [0] == 'Z')
		{
			print_mess (ML(mlSkMess592));
			sleep (sleepTime);
			clear_mess ();
			/*------------------------------------
			| Copy inmr3_rec back into inmr_rec. |
			------------------------------------*/
			memcpy ((char *)&inmr_rec, 
					   (char *)&inmr3_rec, 
					   sizeof (struct inmrRecord));
			return (EXIT_FAILURE);
		}

		strcpy (inmr3_rec.alternate,inmr_rec.item_no);
		/*------------------------------------
		| Copy inmr3_rec back into inmr_rec. |
		------------------------------------*/
		memcpy ((char *)&inmr_rec, 
				   (char *)&inmr3_rec, 
				   sizeof (struct inmrRecord));

		if (!strcmp (inmr_rec.item_no, inmr_rec.alternate))
		{
			print_mess (ML(mlSkMess593));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Category. |
	--------------------*/
	if (LCHECK ("category"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (excf_rec.item_alloc [0] == 'N')
		{
			print_mess (ML(mlSkMess594));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		changeInccSort = 1;
		DSP_FLD ("cat_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("quick_code"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (!strncmp (inmr_rec.quick_code, "                ", strlen (vars [field].mask)))
			return (EXIT_SUCCESS);

		strcpy (inmr2_rec.co_no, comm_rec.co_no);
		strcpy (inmr2_rec.quick_code,inmr_rec.quick_code);
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (!cc)
		{
			if (newItem || inmr2_rec.hhbr_hash !=inmr_rec.hhbr_hash)
			{
				/*("Duplicate Short Code ");*/

				print_mess (ML(mlSkMess663));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		return (EXIT_SUCCESS);
	}
	/*-----------------
	| Validate Class. |
	-----------------*/
	if (LCHECK ("inmr_class"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.inmr_class, inmd_rec.inmd_class);
			DSP_FLD ("inmr_class");
			return (EXIT_SUCCESS);
		}
		changeInccSort = 1;
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Supercession Item. |
	-----------------------------*/
	if (LCHECK ("supitem"))
	{
		if (FLD ("supitem") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (inmr_rec.supercession, "                ");
			sprintf (local_rec.lsup_desc, "%-40.40s", " ");
			strcpy (local_rec.lsup_item, "                ");
			DSP_FLD ("supdesc");
			return (EXIT_SUCCESS);
		}
		sprintf (inmr_rec.supercession, "%-16.16s", prv_ntry);
		print_mess (ML(mlSkMess595));
		sleep (sleepTime);
		clear_mess ();
		DSP_FLD ("supdesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Inventory Grade |
	--------------------------*/
	if (LCHECK ("grade"))
	{
		if (F_NOKEY (field) && INPUT)
			return (EXIT_SUCCESS);
	
		if (dflt_used)
		{
			local_rec.grade_pc = 0.00;
			DSP_FLD ("grade_pc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchIndg (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ingd_rec.co_no, comm_rec.co_no);
		sprintf (ingd_rec.grade, "%-1.1s",inmr_rec.grade);
		cc = find_rec (ingd, &ingd_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlSkMess666));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.grade_pc = ingd_rec.writedown;
		DSP_FLD ("grade_pc");

		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Extra Decription Code. |
	---------------------------------*/
	if (LCHECK ("ex_code"))
	{
		if (SRCH_KEY)
		{
			SrchIned (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (inmr_rec.ex_code, "   ");
			sprintf (ined_rec.desc, "%-40.40s", " ");
			DSP_FLD ("ex_desc");
			return (EXIT_SUCCESS);
		}

		strcpy (ined_rec.co_no, comm_rec.co_no);
		strcpy (ined_rec.code,inmr_rec.ex_code);
		cc = find_rec (ined, &ined_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlSkMess665));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("ex_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("min_sell_pric"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			if (inmr_rec.min_sell_pric < 0.00)
			{
				print_mess (ML(mlSkMess597));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}
	}


	/*-----------------------
	| Validate serial item. |
	-----------------------*/
	if (LCHECK ("serial"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.serial_item, inmd_rec.serial_item);
			DSP_FLD ("serial");
			return (EXIT_SUCCESS);
		}
		if (inmr_rec.serial_item [0] == 'Y')
		{
			strcpy (inmr_rec.costing_flag, "S");
			strcpy (inmr_rec.lot_ctrl, "N");
			DSP_FLD ("lot_ctrl");
			FLD ("costingFlag") = NA;
			FLD ("lot_ctrl") = (envVarSkBatchCont) 	? NA : ND;
		}
		else
		{
			if (prog_status != ENTRY &&inmr_rec.costing_flag [0] == 'S')
			{
				strcpy (inmr_rec.costing_flag, "F");
			}
			FLD ("costingFlag") = NO;
			FLD ("lot_ctrl") = (envVarSkBatchCont) 	? NO : ND;
		}
		GetCostDesc ();
		DSP_FLD ("costDesc");
		DSP_FLD ("costingFlag");

		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Costing flag. |
	------------------------*/
	if (LCHECK ("costingFlag"))
	{
		if (SRCH_KEY)
		{
			SrchCosting ();
			return (EXIT_SUCCESS);
		}
		if (F_NOKEY (field) && INPUT)
			strcpy (inmr_rec.costing_flag, inmd_rec.costing_flag);

		if (inmr_rec.costing_flag [0] == 'S' && inmr_rec.serial_item [0] != 'Y')
	    {
			print_mess (ML(mlSkMess542));
			sleep (sleepTime);
			clear_mess ();
	    	strcpy (inmr_rec.serial_item, "Y");
			DSP_FLD ("serial");
			return (EXIT_SUCCESS);
		}
		GetCostDesc ();
		DSP_FLD ("costDesc");
		DSP_FLD ("costingFlag");
		return (EXIT_SUCCESS);
	}

	/*----------------
	| Validate Duty. |
	----------------*/
	if (LCHECK ("duty"))
	{
		if (F_NOKEY (field) && INPUT)
			strcpy (inmr_rec.duty, inmd_rec.duty);

		if (SRCH_KEY)
		{
			SrchPodt (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inmr_rec.duty, "  "))
			return (EXIT_SUCCESS);

		strcpy (podt_rec.co_no, comm_rec.co_no);
		strcpy (podt_rec.code,inmr_rec.duty);
		cc = find_rec (podt, &podt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess124));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate Licence. |
	-------------------*/
	if (LCHECK ("licence"))
	{
		if (F_NOKEY (field) && INPUT)
			strcpy (inmr_rec.licence, inmd_rec.licence);

		if (SRCH_KEY)
		{
			SrchPolh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inmr_rec.licence, "  "))
			return (EXIT_SUCCESS);

		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);
		strcpy (polh_rec.lic_cate,inmr_rec.licence);
		strcpy (polh_rec.lic_no, "          ");
		cc = find_rec (polh, &polh_rec, GTEQ, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess154));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (polh_rec.lic_cate,inmr_rec.licence))
		{
			print_mess (ML(mlStdMess124));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate Licence. |
	-------------------*/
	if (LCHECK ("gst"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		inmr_rec.gst_pc = inmd_rec.gst_pc;
			DSP_FLD ("gst");
		}

		if (FLD ("gst") == ND)
	  	inmr_rec.gst_pc = 0.00;

		if (envVarGstInclusive)
		{
			if (inmr_rec.gst_pc != 0.00)
			{
				sprintf (err_str, ML(mlSkMess598), envVarGstTaxName, envVarGstTaxName);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("abc_code"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.abc_code, inmd_rec.abc_code);
			DSP_FLD ("abc_code");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("abc_update"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.abc_update, inmd_rec.abc_update);
			DSP_FLD ("abc_update");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("bo_flag"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.bo_flag, inmd_rec.bo_flag);
			DSP_FLD ("bo_flag");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("bo_release"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.bo_release, inmd_rec.bo_release);
			DSP_FLD ("bo_release");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("outer"))
	{
		if ((F_NOKEY (field) && INPUT) || dflt_used)
		{
		inmr_rec.outer_size = inmd_rec.outer_size;
			DSP_FLD ("outer");
			return (EXIT_SUCCESS);
		}

		if (inmr_rec.outer_size < 1.00)
		{
			/*("\007 Pricing factor must be greater than or equal to 1. ");*/

			print_mess (ML(mlSkMess599));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	if (LCHECK ("taxp"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		inmr_rec.tax_pc = inmd_rec.tax_pc;
			DSP_FLD ("taxp");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("tax_amt"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		inmr_rec.tax_amount = inmd_rec.tax_amount;
			DSP_FLD ("tax_amt");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("minqty"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		inmr_rec.min_quan = inmd_rec.min_quan;
			DSP_FLD ("minqty");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("safety"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		inmr_rec.safety_stock = inmd_rec.safety_stock;
			DSP_FLD ("safety");
			return (EXIT_SUCCESS);
		}
	}
	if (LCHECK ("schg"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.schg_flag, "0");
			DSP_FLD ("schg");
			return (EXIT_SUCCESS);
		}
		if (!strncmp (inmr_rec.item_no, "SU", 2))
			strcpy (inmr_rec.schg_flag, "S");
		else
		{
			if (inmr_rec.schg_flag [0] == 'S')
				strcpy (inmr_rec.schg_flag, "0");
		}

		DSP_FLD ("schg");
	}

	/*-------------------------
	| Validate active status. |
	-------------------------*/
	if (LCHECK ("active_status"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inas_rec.co_no, comm_rec.co_no);
		sprintf (inas_rec.act_code, "%-1.1s",inmr_rec.active_status);
		cc = find_rec (inas, &inas_rec, COMPARISON, "r");
		if (cc)
		{
			/*("\007Active Status not found on file ");*/

			print_mess (ML(mlSkMess312));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (old_active_status [0] !=inmr_rec.active_status [0])
		{
			tmp_scn_no = cur_screen;
			scn_set (SCN_EXDESC);
			if (inmr_rec.active_status [0] == 'D')
			{
				/*-----------------------------------------------
				| Active status has changed to Discontinued, so |
				| add discontinued description to inex (first   |
				| line of screen SCN_EXDESC).                   |
				-----------------------------------------------*/
				for (i = lcount [SCN_EXDESC]; i >= 0; i--)
				{
					getval (i);
					putval (i + 1);
				}
				lcount [SCN_EXDESC]++;

				sprintf (local_rec.ex_desc, "%-40.40s", discont_text);
				putval (0);

			}
			else
			{
				/*----------------------------------------------
				| Active status has changed to something other |
				| than Discontinued so remove discontinued     |
				| description from inex (iff it is there)      |
				----------------------------------------------*/
				getval (0);
				if (!strcmp (local_rec.ex_desc, discont_text))
				{
					for (i = 0; i < lcount [SCN_EXDESC]; i++) 
					{
						getval (i + 1);
						putval (i);
					}
					lcount [SCN_EXDESC]--;
				}
			}
			scn_set (tmp_scn_no);
		}

		strcpy (old_active_status,inmr_rec.active_status);
		DSP_FLD ("act_desc");

		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Creditors Number Input. |
	----------------------------------*/
	if (LCHECK ("crd_no"))
	{
		/*-----------------------
		| Search for Suppliers. |
		-----------------------*/
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------------
		| Lookup Supplier master file. |
		------------------------------*/
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.crd_no, pad_num (sumr_rec.crd_no));
		strcpy (sumr_rec.est_no, branchNumber);
		cc = find_rec (sumr , &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.pur_uom, "    "))
			sprintf (fob_cost_prmpt, "Sup. Cost/%-4.4s (%-3.3s)", 
					 local_rec.pur_uom, sumr_rec.curr_code);
		else
			sprintf (fob_cost_prmpt, "Sup. Cost  (%-3.3s)    ", sumr_rec.curr_code);
		display_prmpt (label ("fob_cost"));

		open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  sumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
		{
			local_rec.exch_rate = 1.0;
			sprintf (local_rec.curr_desc, "%-40.40s", "Unknown");
		}
		else
		{
			local_rec.exch_rate = pocr_rec.ex1_factor;
			sprintf (local_rec.curr_desc, "%-40.40s", pocr_rec.description);
		}
		abc_fclose (pocr);

		strcpy (local_rec.st2_uom, local_rec.std_uom);

		local_rec.std_cost = ((local_rec.fob_cost 
						   * local_rec.pur_cnv_fact)
						   *inmr_rec.outer_size)
						   / local_rec.exch_rate;
		DSP_FLD ("crd_name");
		DSP_FLD ("crd_curr_code");
		DSP_FLD ("crd_curr_desc");
		DSP_FLD ("sup_std_uom");
		DSP_FLD ("sup_pri");
		DSP_FLD ("std_cost");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("std_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		if ((F_NOKEY (field) && INPUT) || dflt_used)
			strcpy (local_rec.std_uom, inmd_rec.std_duom);

		sprintf (inum2_rec.uom, "%-4.4s", local_rec.std_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			/*("\007UOM Not Found On File ");*/

			print_mess (ML(mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inmr_rec.std_uom = inum2_rec.hhum_hash;

		save_cnv_fact = local_rec.alt_cnv_fact;

		if (prog_status != ENTRY)
		{
			CalculateConversion (TRUE);
			if (FLD ("alt_cnv_fact") == YES)
				local_rec.alt_cnv_fact = (float) (save_cnv_fact);
		}
		if (!ValidUOM (TRUE))
			return (EXIT_FAILURE);

		DSP_FLD ("std_uom");
		if (prog_status != ENTRY)
			DSP_FLD ("alt_cnv_fact");

		strcpy (local_rec.st2_uom, local_rec.std_uom);

		if (strcmp (local_rec.std_uom, "    "))
			sprintf (lcl_cost_prmpt, "Lcl. Cost/%-4.4s (%-3.3s)", 
					 local_rec.std_uom, envVarCurrCode);
		else
			sprintf (lcl_cost_prmpt, "Lcl. Cost  (%-3.3s)   :", envVarCurrCode);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		if ((F_NOKEY (field) && INPUT) || dflt_used)
			strcpy (local_rec.alt_uom, inmd_rec.alt_duom);

		sprintf (inum2_rec.uom, "%-4.4s", local_rec.alt_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	inmr_rec.alt_uom = inum2_rec.hhum_hash;

		save_cnv_fact = local_rec.alt_cnv_fact;

		CalculateConversion (TRUE);
		if (prog_status != ENTRY &&  FLD ("alt_cnv_fact") == YES)
				local_rec.alt_cnv_fact = (float) (save_cnv_fact);
		
		if (!ValidUOM (TRUE))
			return (EXIT_FAILURE);

		DSP_FLD ("alt_uom");
		DSP_FLD ("alt_cnv_fact");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_cnv_fact"))
	{
		if (!F_NOKEY (field) && dflt_used)
			local_rec.alt_cnv_fact = inmd_rec.uom_cfactor;

		if (!ValidUOM (TRUE))
			return (EXIT_FAILURE);

		DSP_FLD ("alt_cnv_fact");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sup_pur_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		if (F_NOKEY (field) && INPUT)
			strcpy (local_rec.pur_uom, local_rec.std_uom);

		sprintf (inum2_rec.uom, "%-4.4s", local_rec.pur_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inis_rec.sup_uom = inum2_rec.hhum_hash;


		CalculateConversion (FALSE);
		if (!ValidUOM (FALSE))
			return (EXIT_FAILURE);

		if (prog_status != ENTRY && local_rec.pur_cnv_fact != 0.00)
			local_rec.fob_cost = ((local_rec.std_cost 
							   / local_rec.pur_cnv_fact)
							   /inmr_rec.outer_size)
							   * local_rec.exch_rate;

		DSP_FLD ("sup_pur_uom");
		DSP_FLD ("fob_cost");
		DSP_FLD ("sup_std_uom");
		DSP_FLD ("std_cost");
		DSP_FLD ("pur_cnv_fact");

		if (strcmp (local_rec.pur_uom, "    "))
			sprintf (fob_cost_prmpt, "Sup. Cost/%-4.4s (%-3.3s)", 
					 local_rec.pur_uom, sumr_rec.curr_code);
		else
			sprintf (fob_cost_prmpt, "Sup. Cost  (%-3.3s)    ", sumr_rec.curr_code);

		display_prmpt (label ("fob_cost"));

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fob_cost"))
	{
		local_rec.std_cost = ((local_rec.fob_cost 
						   * local_rec.pur_cnv_fact)
						   *inmr_rec.outer_size)
						   / local_rec.exch_rate;

		DSP_FLD ("fob_cost");
		DSP_FLD ("sup_std_uom");
		DSP_FLD ("std_cost");

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Conv. Fct |
	--------------------*/
	if (LCHECK ("pur_cnv_fact"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (local_rec.pur_cnv_fact < 0)
		{
			print_mess (ML(mlSkMess561));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!ValidUOM (TRUE))
			return (EXIT_FAILURE);

		local_rec.fob_cost = ((local_rec.std_cost /
		    				  local_rec.pur_cnv_fact) /
		    				 inmr_rec.outer_size) *
							  local_rec.exch_rate;

		DSP_FLD ("fob_cost");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("source"))
	{
		if (SRCH_KEY)
		{
			SrchSource ();
			return (EXIT_SUCCESS);
		}
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (inmr_rec.source, inmd_rec.source);
			DSP_FLD ("source");

			/*------------------------------------
			| Check if item a manufactured item, |
			| if so can display/edit screen 5.   |
			------------------------------------*/
			if (!strcmp (inmr_rec.source, "BP") ||
				!strcmp (inmr_rec.source, "BM") ||
				!strcmp (inmr_rec.source, "MC") ||
				!strcmp (inmr_rec.source, "MP"))
			{
				manufacturedItem = TRUE;
				edit_ok (SCN_BOM);
			}
			else
			{
				manufacturedItem = FALSE;
				no_edit (SCN_BOM);
			}

			if (newItem)
			{
				inmr_rec.dflt_bom = inmd_rec.dflt_bom;
				inmr_rec.dflt_rtg = inmd_rec.dflt_rtg;
			}

			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (source [i].sourceCode);i++)
		{
			if (!strcmp (source [i].sourceCode,inmr_rec.source))
			{
				sprintf (local_rec.src_desc, "%-26.26s", source [i].sourceDesc);
				DSP_FLD ("src_desc");
				CheckSourceCode ();

				/*------------------------------------
				| Check if item a manufactured item, |
				| if so can display/edit screen 5.   |
				------------------------------------*/
				if (!strcmp (inmr_rec.source, "BP") ||
					!strcmp (inmr_rec.source, "BM") ||
					!strcmp (inmr_rec.source, "MC") ||
					!strcmp (inmr_rec.source, "MP"))
				{
					manufacturedItem = TRUE;
					edit_ok (SCN_BOM);
				}
				else
				{
					manufacturedItem = FALSE;
					no_edit (SCN_BOM);
				}

				return (EXIT_SUCCESS);
			}
		}
		print_mess (ML(mlSkMess664));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (LCHECK ("lot_ctrl"))
	{
		if ((F_NOKEY (field) && INPUT) || dflt_used)
		{
			strcpy (inmr_rec.lot_ctrl, inmd_rec.lot_ctrl);
			DSP_FLD ("lot_ctrl");
			return (EXIT_SUCCESS);
		}
		FLD ("costingFlag") = NO;

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("dec_pt"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			inmr_rec.dec_pt = inmd_rec.dec_pt;
			DSP_FLD ("dec_pt");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_bom"))
	{
		if (dflt_used)
		{
			inmr_rec.dflt_bom = inmd_rec.dflt_bom;
			DSP_FLD ("dflt_bom");
		}

		if (inmr_rec.dflt_bom > 32767)
		{
			print_mess (ML(mlSkMess216));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_rtg"))
	{
		if (dflt_used)
		{
			inmr_rec.dflt_rtg = inmd_rec.dflt_rtg;
			DSP_FLD ("dflt_rtg");
		}

		if (inmr_rec.dflt_bom > 32767)
		{
			print_mess (ML(mlSkMess216));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eoq"))
	{
		if (dflt_used)
		{
			local_rec.eoq = inmd_rec.eoq;
			DSP_FLD ("eoq");
		}
		local_rec.eoq = n_dec (local_rec.eoq,inmr_rec.dec_pt);
		if (!ValidQuantity (local_rec.eoq,inmr_rec.dec_pt))
			return (EXIT_FAILURE);
	inmr_rec.eoq = (float) (local_rec.eoq);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int 
CalculateConversion (
 int calc_alt)
{
	number	cnv_fct;
	number	std_cnv_fct;
	number	oth_cnv_fct;

	inum2_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum3, &inum2_rec, EQUAL, "r");
	if (cc)
	{
		if (calc_alt)
			local_rec.alt_cnv_fact = 1.00;
		else
			local_rec.pur_cnv_fact = 1.00;
		return (EXIT_SUCCESS);
	}

	if (calc_alt)
	{
		inum3_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inum3, &inum3_rec, EQUAL, "r");
		if (cc)
		{
			local_rec.alt_cnv_fact = 1.00;
			return (EXIT_SUCCESS);
		}
	}
	else
	{
		inum3_rec.hhum_hash	=	inis_rec.sup_uom;
		cc = find_rec (inum3, &inum3_rec, EQUAL, "r");
		if (cc)
		{
			local_rec.pur_cnv_fact = 1.00;
			return (EXIT_SUCCESS);
		}
	}

	if (strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
	{
		if (calc_alt)
		{
			if (FLD ("alt_cnv_fact") != ND)
				FLD ("alt_cnv_fact") = YES;

			local_rec.alt_cnv_fact = 1.00;
		}
		else
		{
			if (FLD ("pur_cnv_fact") != ND)
				FLD ("pur_cnv_fact") = YES;

			local_rec.pur_cnv_fact = 1.00;
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum2_rec.cnv_fct);
	NumFlt (&oth_cnv_fct, inum3_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| std uom cnv_fct / alt uom cnv_fct = conversion factor    |
	----------------------------------------------------------*/
	NumDiv (&std_cnv_fct, &oth_cnv_fct, &cnv_fct);

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	if (calc_alt)
	{
		local_rec.alt_cnv_fact = NumToFlt (&cnv_fct);

		if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
			FLD ("alt_cnv_fact") = NA;
	}
	else
	{
		local_rec.pur_cnv_fact = NumToFlt (&cnv_fct);

		if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
			FLD ("pur_cnv_fact") = NA;
	}

	return (EXIT_SUCCESS);
}

/*==========================================
| Check whether uom is valid compared with |
| the dec_pt and the conversion factor.    |
| eg. std uom = kg     iss uom = gm        |
|     conv.fact = 1000 dec_pt = 2          |
|     issue 5 gm, converts to 0.005 kg     |
|     round to 2 dec_pt, new qty = 0.01 kg |
|     or 10gm                              |
|This is incorrect and not allowed.        |
==========================================*/
int
ValidUOM (
 int calc_alt)
{
	long	numbers [7];
	float	cnv_fact;

	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (calc_alt)
		cnv_fact = local_rec.alt_cnv_fact;
	else
		cnv_fact = local_rec.pur_cnv_fact;

	if (cnv_fact > numbers [inmr_rec.dec_pt])
	{
		sprintf 
		(
			err_str,
			ML(mlSkMess482),
			(calc_alt) ? "ALT" : "PUR",
			(calc_alt) ? local_rec.alt_uom : local_rec.pur_uom,
			inmr_rec.dec_pt
		);

		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

/*--------------------------------------------
| Checks if the quantity entered by the user |
| valid quantity that can be saved to a      |
| float variable without any problems of     |
| losing figures after the decimal point.    |
| eg. if _dec_pt is 2 then the greatest      |
| quantity the user can enter is 99999.99    |
--------------------------------------------*/
int
ValidQuantity (
 double _qty, 
 int _dec_pt)
{
	/*--------------------------------
	| Quantities to be compared with |
	| with the user has entered.     |
	--------------------------------*/
	double	compare [7];
	
	compare [0] = 9999999.00;
	compare [1] = 999999.90;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, ML(mlSkMess238), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

void
CheckSourceCode (
 void)
{
	int		i;

	for (i = 0;strlen (source [i].sourceCode);i++)
		if (!strcmp (source [i].sourceCode,inmr_rec.source))
			break;
}

void
SrchCosting (
 void)
{
	int		i;
	_work_open (1,0,20);
	save_rec ("#C", "#Costing Description");
	for (i = 0;strlen (costStruct [i].costCode);i++)
	{
		strcpy (err_str, ML (costStruct [i].costDesc));
		cc = save_rec (costStruct [i].costCode, err_str);
		if (cc)
			break;
	}
	disp_srch ();
	work_close ();
}

void
SrchSource (
 void)
{
	int		i;
	_work_open (2,0,40);
	save_rec ("#Sr", "#Source Description");
	for (i = 0;strlen (source [i].sourceCode);i++)
	{
		strcpy (err_str, ML (source [i].sourceDesc));
		cc = save_rec (source [i].sourceCode, err_str);
		if (cc)
			break;
	}
	disp_srch ();
	work_close ();
}

/*=============================
| Update All inventory files. |
=============================*/
int
Update (
 void)
{
	clear ();

	strcpy (inmr_rec.stat_flag, "0");

	inmr_rec.uom_cfactor = local_rec.alt_cnv_fact;
	strcpy (inmr_rec.sale_unit, local_rec.std_uom);
	inmr_rec.disc_pc = ScreenDisc (inmr_rec.disc_pc);
	
	/*-----------------------
	| Discontinued Product. |
	-----------------------*/
	if (inmr_rec.active_status [0] == 'D')
		strcpy (inmr_rec.bo_flag, "N");

	/*-----------------------------
	| Add inventory master record |
	-----------------------------*/
	if (newItem)
	{
		inmr_rec.hhsi_hash 	= 0L;
		inmr_rec.on_hand   	= 0.00;
		inmr_rec.on_order  	= 0.00;
		inmr_rec.committed 	= 0.00;
		inmr_rec.backorder 	= 0.00;
		inmr_rec.forward   	= 0.00;
		inmr_rec.ltd_sales 	= 0.00;

		print_at (0,0,ML(mlStdMess035));
		fflush (stdout);

		sprintf (err_str, "%s : %s (%s)", ML ("Item"), inmr_rec.item_no, inmr_rec.description);
		AuditFileAdd(err_str, &inmr_rec, inmr_list, INMR_NO_FIELDS);
		cc = abc_add (inmr1, &inmr_rec);
		if (cc)
		{
			if (cc == 100)
			{

				print_mess (ML(mlSkMess600));
				sleep (sleepTime);
				clear_mess ();
				return (FALSE);
			}
			else
	 			file_err (cc, "inmr", "DBADD");
		}

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (inmr1, &inmr_rec, COMPARISON, "r");
		if (cc)
	 		file_err (cc, "inmr", "DBFIND");
	
		UpdateSrsk ();
		UpdateInex ();
		UpdateInuv (inmr_rec.std_uom, 1);

		AddBranchWarehouse ();

		/*--------------------------------------------------
		| If noMasterWh equals 0 there is no master record  |
        | for that branch and warehouse.                   |
		--------------------------------------------------*/
		if (noMasterWh == FALSE && !NON_STOCK)
			AddInis ();
	}
	else
	{
		/*--------------------------------
		| update inventory master record |
		--------------------------------*/
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (err_str, "%s : %s (%s)", ML ("Item"), inmr_rec.item_no, inmr_rec.description);
		AuditFileAdd(err_str, &inmr_rec, inmr_list, INMR_NO_FIELDS);
		cc = abc_update (inmr1, &inmr_rec);
		if (cc)
	 		file_err (cc, "inmr", "DBUPDATE");

		UpdateSrsk ();
		UpdateInex ();
		UpdateInuv (inmr_rec.std_uom, 1);

		/*--------------------------------------------------
		| Update inventory branch records if sort changes. |
		--------------------------------------------------*/
		if (changeInccSort)
			InccSortChanged ();

	}
	abc_unlock (inmr);
	strcpy (local_rec.lprev_item,inmr_rec.item_no);

	if (envVarPosInstalled)
		PosdtupUpdate ();

	return (TRUE);
}

/*=====================================
| Add branch/warehouse master record. |
=====================================*/
void
AddBranchWarehouse (
 void)
{
	int		i;

	/*-------------------------------------
	| Add inei Records For Master branch. |
	-------------------------------------*/
	print_at (1,0,ML(mlStdMess035));
	fflush (stdout);

	for (i = 0; i < max_branches; i++)
	{
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, sr_br [i]);
		inei_rec.min_stock = inmr_rec.min_quan;
		inei_rec.safety_stock = inmr_rec.safety_stock;
		inei_rec.std_batch = 1.00;
		strcpy (inei_rec.abc_code, inmr_rec.abc_code);
		strcpy (inei_rec.abc_update, inmr_rec.abc_update);
		strcpy (inei_rec.stat_flag, "0");
		inei_rec.dflt_bom = inmr_rec.dflt_bom;
		inei_rec.dflt_rtg = inmr_rec.dflt_rtg;
		inei_rec.eoq = inmr_rec.eoq;

   		cc = abc_add (inei, &inei_rec);
		if (cc)
	 		file_err (cc, "inei", "DBADD");

		print_at (1,30,"%s, ", inei_rec.est_no);
		fflush (stdout);
	}

	/*------------------------------------
	| Add incc Record For Master Branch. |
	------------------------------------*/
	print_at (2,0,ML(mlStdMess035));
	fflush (stdout);
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (temp_sort, "%s%11.11s%16.16s", inmr_rec.inmr_class, 
					     inmr_rec.category, 
					     inmr_rec.item_no);

	strcpy (incc_rec.sort, temp_sort);
	strcpy (incc_rec.stocking_unit, inmr_rec.sale_unit);
	strcpy (incc_rec.stat_flag, "0");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.abc_code, inmr_rec.abc_code);
	strcpy (incc_rec.abc_update, inmr_rec.abc_update);
	incc_rec.first_stocked = TodaysDate ();
	incc_rec.safety_stock = inmr_rec.safety_stock;
	incc_rec.dflt_bom = inmr_rec.dflt_bom;
	incc_rec.dflt_rtg = inmr_rec.dflt_rtg;
	incc_rec.eoq = inmr_rec.eoq;

	cc = abc_add (incc, &incc_rec);
	if (cc)
	 	file_err (cc, incc, "DBADD");
}
/*================================
| Add inventory supplier record. |
================================*/
void	
AddInis (
 void)
{
	if (sumr_rec.hhsu_hash == 0L || !strcmp (sumr_rec.crd_no, "      "))
		return;

	strcpy (inis_rec.co_no, comm_rec.co_no);
	if (inis_rec.sup_priority [0] == 'C')
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
	}
	if (inis_rec.sup_priority [0] == 'B')
	{
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, "  ");
	}
	if (inis_rec.sup_priority [0] == 'W')
	{
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, comm_rec.cc_no);
	}
	inis_rec.hhbr_hash 	 = inmr_rec.hhbr_hash;
	inis_rec.hhsu_hash 	 = sumr_rec.hhsu_hash;
	strcpy (inis_rec.sup_part, inmr_rec.item_no);
	inis_rec.hhis_hash 	 	= 0L;
	inis_rec.fob_cost 	 	= local_rec.fob_cost * local_rec.pur_cnv_fact;
	inis_rec.lcost_date 	 = TodaysDate ();
	strcpy (inis_rec.duty, inmr_rec.duty);
	strcpy (inis_rec.licence, inmr_rec.licence);
	inis_rec.min_order 	 	= local_rec.min_order;
	inis_rec.norm_order 	= local_rec.norm_order;
	inis_rec.ord_multiple 	= local_rec.ord_multiple;
	inis_rec.pallet_size 	= local_rec.pallet_size;
	inis_rec.lead_time 	 	= local_rec.lead_time;
	inis_rec.sea_time	 	= local_rec.lead_time;
	inis_rec.air_time	 	= local_rec.lead_time;
	inis_rec.lnd_time	 	= local_rec.lead_time;
	inis_rec.weight 	 	= local_rec.weight;
	inis_rec.volume 	 	= local_rec.volume;
	inis_rec.pur_conv 	 	= local_rec.pur_cnv_fact;
	strcpy (inis_rec.sup_part, local_rec.sup_part);
	strcpy (inis_rec.stat_flag, "C");

	sprintf (inum2_rec.uom, "%-4.4s", local_rec.pur_uom);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (!cc)
	{
		inis_rec.sup_uom = inum2_rec.hhum_hash;
		UpdateInuv(inis_rec.sup_uom,0);
	}

	cc = abc_add (inis, &inis_rec);
	if (cc)
	{
		/*("Inventory supplier record was not added due to duplicates");*/

		print_mess (ML(mlSkMess601));
		sleep (sleepTime);
		clear_mess ();
	}

	return;
}

/*============================================
| Find all incc records and Update sort key. |
============================================*/
void
InccSortChanged (void)
{
	incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, GTEQ, "u");
	while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf (temp_sort, "%s%11.11s%16.16s", inmr_rec.inmr_class, 
				   		      inmr_rec.category, 
				   		      inmr_rec.item_no);

		strcpy (incc_rec.sort, temp_sort);
		cc = abc_update (incc, &incc_rec);
		if (cc)
	 		file_err (cc, incc, "DBUPDATE");

		abc_unlock (incc);
		cc = find_rec (incc, &incc_rec, NEXT, "u");
	}
	abc_unlock (incc);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	save_rec ("#Category No", "#Category Description");
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
		      !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, excf, "DBFIND");
}

/*==============================
| Search for Duty master file. |
==============================*/
void
SrchPodt (
 char *key_val)
{
	_work_open (2,0,40);
	strcpy (podt_rec.co_no, comm_rec.co_no);
	sprintf (podt_rec.code, "%-2.2s", key_val);
	save_rec ("#DC", "#Duty Code Description");
	cc = find_rec (podt, &podt_rec, GTEQ, "r");
	while (!cc && !strncmp (podt_rec.code, key_val, strlen (key_val)) &&
		      !strcmp (podt_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (podt_rec.code, podt_rec.description);
		if (cc)
			break;
		cc = find_rec (podt, &podt_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (podt_rec.co_no, comm_rec.co_no);
	sprintf (podt_rec.code, "%-2.2s", temp_str);
	cc = find_rec (podt, &podt_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, podt, "DBFIND");
}

/*=================================
| Search for Licence master file. |
=================================*/
void
SrchPolh (
 char *key_val)
{
	char	lic_cat [3];

	_work_open (2,0,40);
	strcpy (lic_cat, "  ");
	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	sprintf (polh_rec.lic_cate, "%-2.2s", key_val);
	strcpy (polh_rec.lic_no, "          ");
	save_rec ("#LC", "#Licence Code Type");
	cc = find_rec (polh, &polh_rec, GTEQ, "r");
	while (!cc && !strncmp (polh_rec.lic_cate, key_val, strlen (key_val)) &&
		      !strcmp (polh_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (polh_rec.lic_cate, lic_cat))
		{
			cc = save_rec (polh_rec.lic_cate, polh_rec.type);
			if (cc)
				break;
			strcpy (lic_cat, polh_rec.lic_cate);
		}
		cc = find_rec (polh, &polh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	sprintf (polh_rec.lic_cate, "%-2.2s", temp_str);
        strcpy (polh_rec.lic_no, "          ");
	cc = find_rec (polh, &polh_rec, GTEQ, "r");
	if (cc)
	 	file_err (cc, polh, "DBFIND");
}

/*=====================================
| Search for Extra Description file. |
=====================================*/
void
SrchIned (
 char *key_val)
{
	_work_open (3,0,40);
	strcpy (ined_rec.co_no, comm_rec.co_no);
	sprintf (ined_rec.code, "%-3.3s", key_val);
	save_rec ("#EDC", "#Extra Description");
	cc = find_rec (ined, &ined_rec, GTEQ, "r");
	while (!cc && !strncmp (ined_rec.code, key_val, strlen (key_val)) &&
		      !strcmp (ined_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (ined_rec.code, ined_rec.desc);
		if (cc)
			break;
		cc = find_rec (ined, &ined_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ined_rec.co_no, comm_rec.co_no);
	sprintf (ined_rec.code, "%-3.3s", temp_str);
	cc = find_rec (ined, &ined_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, ined, "DBFIND");
}

void
SrchInum (
 char *key_val)
{
	_work_open (4,0,40);
	save_rec ("#UOM", "#UOM Description ");
	sprintf (inum_rec.uom_group, "%-20.20s", " ");
	inum_rec.hhum_hash = 0L;

	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while (!cc)
	{
		cc = save_rec (inum_rec.uom, inum_rec.desc);
		if (cc)
			break;

		cc = find_rec (inum, &inum_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
}

void
SrchIndg (
 char *key_val)
{
	_work_open (1,0,40);
	save_rec ("#G", "#Grade Writedown ");
	strcpy (ingd_rec.co_no, comm_rec.co_no);
	strcpy (ingd_rec.grade, " ");
	cc = find_rec (ingd, &ingd_rec, GTEQ, "r");
	while (!cc)
	{
		sprintf (err_str, "%5.2f", ingd_rec.writedown);
		cc = save_rec (ingd_rec.grade, err_str);
		if (cc)
			break;

		cc = find_rec (ingd, &ingd_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
}

void
SrchInas (
 char *key_val)
{
	_work_open (1,0,40);
	save_rec ("#C", "#Active Status Description");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, " ");
	cc = find_rec (inas, &inas_rec, GTEQ, "r");
	while (!cc && !strcmp (inas_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (inas_rec.act_code, inas_rec.description);
		if (cc)
			break;

		cc = find_rec (inas, &inas_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, temp_str);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inas, "DBFIND");
}

void
ReadEsmr (
 void)
{
	max_branches = 0;

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");

	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (sr_br [max_branches++] , esmr_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	abc_fclose (esmr);
}

void
ReadInas (
 void)
{
	sprintf (discont_text, "%-40.40s", " ");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, "D");
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		sprintf (discont_text, "%-40.40s", " ");
	else
		sprintf (discont_text, "%-40.40s", inas_rec.description);
}

int
GetSupplier (
 void)
{
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inis_rec.sup_priority, "C1");
			strcpy (inis_rec.co_no, comm_rec.co_no);
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
	}
	if (cc)
	{
		sprintf (sumr_rec.crd_no, "%6.6s", " ");
		sprintf (sumr_rec.crd_name,   "%40.40s", " ");
		sprintf (podt_rec.description,   "%20.20s", " ");
		sprintf (polh_rec.lic_no, "%10.10s", " ");
		sprintf (local_rec.sup_part, "%16.16s", " ");
		sprintf (local_rec.pur_uom,  "%4.4s", " ");
		sprintf (inis_rec.dflt_lead,"%1.1s", " ");
		local_rec.fob_cost		= 0.00;
		local_rec.std_cost		= 0.00;
		local_rec.volume 		= 0.00;
		local_rec.min_order 	= 0.00;
		local_rec.norm_order 	= 0.00;
		local_rec.ord_multiple  = 0.00;
		local_rec.pallet_size   = 0.00;
		local_rec.lead_time 	= 0.00;
		local_rec.pur_cnv_fact  = 0.00;

		return (EXIT_FAILURE);
	}

	abc_selfield (sumr, "sumr_hhsu_hash");

	sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	{
		sprintf (sumr_rec.crd_no, "%6.6s", " ");
		sprintf (sumr_rec.crd_name,   "%40.40s", " ");
		sprintf (podt_rec.description,   "%20.20s", " ");
		sprintf (polh_rec.lic_no, "%10.10s", " ");
		sprintf (local_rec.sup_part, "%16.16s", " ");
		sprintf (local_rec.pur_uom,  "%4.4s", " ");
		sprintf (inis_rec.dflt_lead,"%1.1s", " ");
		local_rec.fob_cost		= 0.00;
		local_rec.std_cost		= 0.00;
		local_rec.volume 		= 0.00;
		local_rec.min_order 	= 0.00;
		local_rec.norm_order 	= 0.00;
		local_rec.ord_multiple  = 0.00;
		local_rec.pallet_size  	= 0.00;
		local_rec.lead_time 	= 0.00;
		local_rec.pur_cnv_fact  = 0.00;
		abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
		return (EXIT_FAILURE);
	}

	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code,  sumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
	if (cc)
	{
		local_rec.exch_rate = 1.0;
		sprintf (local_rec.curr_desc, "%-40.40s", "Unknown");
	}
	else
	{
		local_rec.exch_rate = pocr_rec.ex1_factor;
		sprintf (local_rec.curr_desc, "%-40.40s", pocr_rec.description);
	}
	abc_fclose (pocr);

	local_rec.fob_cost		= 	inis_rec.fob_cost;
	local_rec.volume 		= 	inis_rec.volume;
	local_rec.min_order 	= 	inis_rec.min_order;
	local_rec.norm_order 	=  	inis_rec.norm_order;
	local_rec.ord_multiple  = 	inis_rec.ord_multiple;
	local_rec.pallet_size   = 	inis_rec.pallet_size;
	local_rec.lead_time 	= 	inis_rec.lead_time;
	strcpy (local_rec.sup_part,     inis_rec.sup_part);

	inum4_rec.hhum_hash	=	inis_rec.sup_uom;
	cc = find_rec (inum3, &inum4_rec, EQUAL, "r");
	if (cc)
		strcpy (local_rec.pur_uom, "    ");
	else
		sprintf (local_rec.pur_uom, "%-4.4s", inum4_rec.uom);

	if (!strcmp (inum2_rec.uom_group, inum4_rec.uom_group))
		FLD ("pur_cnv_fact") = NA;
	else
	{
		if (FLD ("pur_cnv_fact") != ND)
			FLD ("pur_cnv_fact") = YES;
	}
	local_rec.pur_cnv_fact = inis_rec.pur_conv;
	local_rec.fob_cost = local_rec.fob_cost / local_rec.pur_cnv_fact;
	local_rec.std_cost = ((local_rec.fob_cost 
					   * local_rec.pur_cnv_fact)
					   * inmr_rec.outer_size)
					   / local_rec.exch_rate;

	if (strcmp (local_rec.pur_uom, "    "))
		sprintf (fob_cost_prmpt, "Sup. Cost/%-4.4s (%-3.3s)", 
				 local_rec.pur_uom, sumr_rec.curr_code);

	if (strcmp (inis_rec.duty, "  "))
	{
		strcpy (podt_rec.co_no, comm_rec.co_no);
		strcpy (podt_rec.code, inis_rec.duty);
		cc = find_rec (podt, &podt_rec, COMPARISON, "r");
		if (cc)
			strcpy (podt_rec.code, "  ");
	}
	if (strcmp (inis_rec.licence, "  "))
	{
		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);
		strcpy (polh_rec.lic_cate, inis_rec.licence);
		strcpy (polh_rec.lic_no, "          ");
		cc = find_rec (polh, &polh_rec, GTEQ, "r");
		if (cc)
			strcpy (inis_rec.licence, "  ");

		if (strcmp (polh_rec.lic_cate, inis_rec.licence))
			strcpy (inis_rec.licence, "  ");
	}
	strcpy (inmr_rec.licence, inis_rec.licence);
	strcpy (inmr_rec.duty, inis_rec.duty);
	local_rec.weight = inis_rec.weight;

	abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	return (EXIT_SUCCESS);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input.                        |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML(mlSkMess602), 24, 0, 1);

		print_at (0,52,ML(mlSkMess096), local_rec.lprev_item);

		line_at (1, 0, 80);

		pr_box_lines (scn);

		print_at (21,0,ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,ML(mlSkMess603), inmr_rec.item_no, inmr_rec.description);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else
        abc_unlock (inmr);

    return (EXIT_SUCCESS);
}

/*======================================================
| If field cannot be input then leave, else set to NI. |
======================================================*/
void
SetfieldValue (
 char *fld_name)
{
	if (F_NOKEY (label (fld_name)) || FLD (fld_name) == YES)
		return;

	FLD (fld_name) = NI;

	return;
}

/*======================================================
| If field cannot be input then leave, else set to NI. |
======================================================*/
void
CheckHiddenFields (
 char *fld_name, 
 int key)
{
	if (F_HIDE (label (fld_name)))
		return;

	FLD (fld_name) = key;

	return;
}

void
UpdateInuv (
 long	uom,
 int		alt_uom)
{
	inuv_rec.hhbr_hash	= inmr_rec.hhbr_hash;
	inuv_rec.hhum_hash	=	uom;
	cc = find_rec ("inuv", &inuv_rec, COMPARISON, "r");
	if (cc)
	{
		inuv_rec.hhbr_hash	= inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	uom;
		cc = abc_add ("inuv", &inuv_rec);
		if (cc)
			file_err (cc, "inuv", "DBADD");
	}
	if (alt_uom)
	{
		if (uom == inmr_rec.alt_uom)
			return;

		inuv_rec.hhbr_hash	= inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	= inmr_rec.alt_uom;
		cc = find_rec ("inuv", &inuv_rec, COMPARISON, "r");
		if (cc)
		{
			inuv_rec.hhbr_hash	= inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	= inmr_rec.alt_uom;
			cc = abc_add ("inuv", &inuv_rec);
			if (cc)
				file_err (cc, "inuv", "DBADD");
		}
	}
}
	
void
UpdateInex (
 void)
{
	int		i;

	scn_set (SCN_EXDESC);

	/*-------------------------------------
	| Add first line as description line. |
	-------------------------------------*/
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no = 0;
	cc = find_rec (inex, &inex_rec, EQUAL, "u");
	if (cc)
	{
		sprintf (inex_rec.desc, "%-40.40s", inmr_rec.description2);
		cc = abc_add (inex, &inex_rec);
		if (cc)
			file_err (cc, "inex", "DBADD");
	}
	else
	{
		sprintf (inex_rec.desc, "%-40.40s", inmr_rec.description2);
		cc = abc_update (inex, &inex_rec);
		if (cc)
			file_err (cc, "inex", "DBUPDATE");
	}
	
	if (lcount [SCN_EXDESC] == 0)
	{
		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = 1;
		cc = find_rec (inex, &inex_rec, GTEQ, "u");
		while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			abc_delete (inex);
			inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
			inex_rec.line_no = 1;
			cc = find_rec (inex, &inex_rec, GTEQ, "u");
		}
		abc_unlock (inex);
		return;
	}
	for (i = lcount [SCN_EXDESC] ; i >= 0; i--)
	{
		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = i + 2;
		cc = find_rec (inex, &inex_rec, COMPARISON, "u");
		getval (i);
	  	clip (local_rec.ex_desc);
		if (strlen (local_rec.ex_desc) == 0)
		{
			lcount [SCN_EXDESC]--;
			abc_delete (inex);
		}
		else
		{
			lcount [SCN_EXDESC]++;
			break;
		}
	}
	for (i = 0; i < lcount [SCN_EXDESC]; i++)
	{
		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = i + 1;
		cc = find_rec (inex, &inex_rec, COMPARISON, "u");
		getval (i);
		sprintf (inex_rec.desc, "%-40.40s", local_rec.ex_desc);
		if (cc)
		{
			cc = abc_add (inex, &inex_rec);
			if (cc)
				file_err (cc, "inex", "DBADD");
		}
		else
		{
			cc = abc_update (inex, &inex_rec);
			if (cc)
				file_err (cc, "inex", "DBUPDATE");
		}
	}

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no = lcount [SCN_EXDESC] + 1;
	cc = find_rec (inex, &inex_rec, GTEQ, "u");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		abc_delete (inex);

		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = lcount [SCN_EXDESC] + 1;
		cc = find_rec (inex, &inex_rec, GTEQ, "u");
	}
	abc_unlock (inex);
	return;
}


/*===============================
| Update search file for stock. |
===============================*/
void
UpdateSrsk (
 void)
{
	int		NewSrsk;

	open_rec (srsk, srsk_list, SRSK_NO_FIELDS, "srsk_hhbr_hash");

	srsk_rec.hhbr_hash = inmr_rec.hhbr_hash;
	NewSrsk = find_rec ("srsk", &srsk_rec, EQUAL, "u");

	strcpy (err_str, inmr_rec.description);
	strcpy (srsk_rec.co_no, 	  inmr_rec.co_no);
	strcpy (srsk_rec.item_no, 	  inmr_rec.item_no);
	strcpy (srsk_rec.srsk_class,  inmr_rec.inmr_class);
	strcpy (srsk_rec.active_status,inmr_rec.active_status);
	strcpy (srsk_rec.alpha_code,  inmr_rec.alpha_code);
	strcpy (srsk_rec.alternate,   inmr_rec.alternate);
	strcpy (srsk_rec.barcode, 	  inmr_rec.barcode);
	strcpy (srsk_rec.maker_no,    inmr_rec.maker_no);
	strcpy (srsk_rec.description, upshift (err_str));
	strcpy (srsk_rec.category,    inmr_rec.category);
	strcpy (srsk_rec.source,      inmr_rec.source);
	strcpy (srsk_rec.sellgrp,     inmr_rec.sellgrp);
	strcpy (srsk_rec.buygrp,      inmr_rec.buygrp);
	strcpy (srsk_rec.qc_reqd,     inmr_rec.qc_reqd);
	strcpy (srsk_rec.spare,       " ");
	
	if (NewSrsk)
	{
		cc = abc_add ("srsk", &srsk_rec);
		if (cc)
			file_err (cc, "srsk", "DBADD");
	}
	else
	{
		cc = abc_update ("srsk", &srsk_rec);
		if (cc)
			file_err (cc, "srsk", "DBUPDATE");
	}
	abc_fclose (srsk);
}

void
ReadInex (
 void)
{
	scn_set (SCN_EXDESC);
	ClearInex();
	lcount [SCN_EXDESC] = 0;

	inex_rec.hhbr_hash  = inmr_rec.hhbr_hash;
	inex_rec.line_no    = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (inex_rec.line_no == 0)
			sprintf (inmr_rec.description2, "%-40.40s", inex_rec.desc);
		else
		{
			sprintf (local_rec.ex_desc, "%-40.40s", inex_rec.desc);

			putval (lcount [SCN_EXDESC]++);
		}
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
	prevInexLineCnt = lcount [SCN_EXDESC];
	return;
}

void
SrchIngp (
	char	*key_val, 
	int		isbuy)
{
	_work_open (6,0,40);
	if (isbuy)
		save_rec ("#CODE  ", "#Buying Group Description ");
	else
		save_rec ("#CODE  ", "#Selling Group Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, (isbuy) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if ((isbuy && ingp_rec.type [0] == 'B') ||
			 (!isbuy && ingp_rec.type [0] == 'S'))
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		else
			break;

		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, (isbuy) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

/*=======================
| Validate group codes. |
=======================*/
void
GetGrpCodes (void)
{
	strcpy (ingp_rec.code,inmr_rec.sellgrp);
	strcpy (ingp_rec.type, "S");
	strcpy (ingp_rec.co_no, comm_rec.co_no);
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
		
	strcpy (local_rec.selldesc, ingp_rec.desc);

	strcpy (ingp_rec.code, inmr_rec.buygrp);
	strcpy (ingp_rec.type, "B");
	strcpy (ingp_rec.co_no, comm_rec.co_no);
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
	
	strcpy (local_rec.buydesc, ingp_rec.desc);
}

/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc (
 float	DiscountPercent)
{
	if (envVarSoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}
/*=======================
| Updated pos record(s) |
=======================*/
void 
PosdtupUpdate (void)
{
    posdtup_rec.pos_no = 0;
	strcpy (posdtup_rec.file_name, "inmr");
	posdtup_rec.record_hash = inmr_rec.hhbr_hash;
	cc = find_rec (posdtup, &posdtup_rec, COMPARISON, "r");
	if (cc)
	{
		cc = abc_add (posdtup, &posdtup_rec);
		if (cc) 
			file_err (cc, posdtup, "DBADD");
	}
	else
	{
		cc = abc_update (posdtup, &posdtup_rec);
		if (cc) 
			file_err (cc, posdtup, "DBUPDATE");
	}
}

/*==============================
| Get description for costing. |
==============================*/
void
GetCostDesc (void)
{
	int		i;

	for (i = 0;strlen (costStruct [i].costCode);i++)
	{
		if (costStruct [i].costCode [0] == inmr_rec.costing_flag [0])
			sprintf (local_rec.costDesc, "%-15.15s", costStruct [i].costDesc);
	}
}


void
ClearInex (void)
{
	int ctr = 0;
	lcount [SCN_EXDESC] = 0;
	for (ctr = 0; ctr <= prevInexLineCnt; ctr++)
	{
		sprintf (local_rec.ex_desc, "%40s", " ");
		putval(lcount [SCN_EXDESC]++);
	}
}

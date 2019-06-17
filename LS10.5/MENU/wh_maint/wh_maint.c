/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: wh_maint.c,v 5.9 2002/11/21 06:37:39 keytan Exp $
|  Program Name  : (wh_maint.c)
|  Program Desc  : (Warehouse Master File Maintenence)
|---------------------------------------------------------------------|
|  Date Written  : A long time ago | Author   : Scott B Darrow.       |
|---------------------------------------------------------------------|
| $Log: wh_maint.c,v $
| Revision 5.9  2002/11/21 06:37:39  keytan
| Updated not to allow multiple master warehouse per branch.
|
| Revision 5.8  2002/07/17 09:57:27  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.7  2001/11/29 02:12:15  scott
| Updated to use screen file IF applicable
|
| Revision 5.6  2001/10/20 02:28:28  scott
| Updated to allow default on address lines.
|
| Revision 5.5  2001/10/17 09:50:58  francis
| Updated so that will not proceed when adding new warehouse
| when address 1-4 isnt inputed.
|
| Revision 5.4  2001/10/05 02:50:41  cha
| Updated to handle locations correctly.
|
| Revision 5.2  2001/08/09 05:13:59  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:52  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/05/29 06:02:28  scott
| Updated to add app.schema
| Updated to include new validations for picking locations
|
+====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: wh_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/wh_maint/wh_maint.c,v 5.9 2002/11/21 06:37:39 keytan Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <ml_sk_mess.h>

#define		MAIN_SCN		1
#define		WH_SCN			2
#define		OPTION_SCN		3
#define		FORTY_SPACES    "                                        "

   	int newWarehouse = FALSE;
   	int	newRecord = 0;
	extern	int		TruePosition;

#include	"schema"

struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct lomrRecord	lomr_rec;
struct exsfRecord	exsf_rec;
struct llctRecord	llct_rec;

	char	*data = "data";
	char 	*ccmr2 = "ccmr2";

	char	*curr_user;

int	envVarMultLoc = FALSE;

	int		newControlRecord	=	FALSE;

	char	promptYes [6],
			promptNo  [6],
			promptManual	[21],
			promptView		[21],
			promptAuto		[21],
			promptNoAlloc	[21];


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	mst_wh [2];
	char	mst_wh_desc [6];
	char	sal_ok [2];
	char	sal_ok_desc [6];
	char	pur_ok [2];
	char	pur_ok_desc [6];
	char	trans_ok [2];
	char	trans_ok_desc [6];
	char	iss_ok [2];
	char	iss_ok_desc [6];
	char	rec_ok [2];
	char	rec_ok_desc [6];
	char	rep_ok [2];
	char	rep_ok_desc [6];
	char	lrp_ok [2];
	char	lrp_ok_desc [6];
	char	alt_uom [2];
	char	alt_uom_desc [6];
	char	auto_all [2];
	char	auto_all_desc [6];
	char	exp_items [2];
	char	exp_items_desc [6];
	char	all_locs [2];
	char	all_locs_desc [6];
	char	PickOrder1 [2],
			PickOrder2 [2],
			PickOrder3 [2];
	char	ProgPick [6] [2];
	char	ProgPickDesc [6] [31];
	int	lpno;
} local_rec;

static	struct	var	vars [] =
{
	{MAIN_SCN, LIN, "co_no",	 2, 2, CHARTYPE,
		"NN", "          ",
		" ", "", "Company Number    : ", " ",
		 NE, NO, JUSTRIGHT, "1", "99", ccmr_rec.co_no},
	{MAIN_SCN, LIN, "co_name",	 3, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Company Name      : ", " ",
		 NA, NO,  JUSTLEFT, "", "", comr_rec.co_short_name},
	{MAIN_SCN, LIN, "br_no",	 4, 2, CHARTYPE,
		"NN", "          ",
		" ", "", "Branch Number     : ", " ",
		 NE, NO, JUSTRIGHT, "0", "99", ccmr_rec.est_no},
	{MAIN_SCN, LIN, "br_name",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", esmr_rec.short_name},
	{MAIN_SCN, LIN, "wh_no",	 7, 2, CHARTYPE,
		"NN", "          ",
		" ", "", "Warehouse Number  : ", " ",
		 NE, NO, JUSTRIGHT, "0", "99", ccmr_rec.cc_no},
	{MAIN_SCN, LIN, "wh_name",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Warehouse Name    : ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.name},
	{MAIN_SCN, LIN, "wh_acronym",	 9, 2, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "Warehouse Acronym : ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.acronym},
	{MAIN_SCN, LIN, "wh_add1",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address Part One  : ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.whse_add1},
	{MAIN_SCN, LIN, "wh_add2",  11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address Part Two  : ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.whse_add2},
	{MAIN_SCN, LIN, "wh_add3",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address Part Three: ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.whse_add3},
	{MAIN_SCN, LIN, "wh_add4",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address Part Four : ", " ",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.whse_add4},
	{MAIN_SCN, LIN, "sman",		15, 2, CHARTYPE,
		"UU", "          ",
		" ", " ","Salesman number   : ", "Enter Salesman for Van Stocks Etc.",
		 NO, NO, JUSTRIGHT, "", "", ccmr_rec.sman_no},
	{MAIN_SCN, LIN, "sman_name",	16, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesman name     : ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{WH_SCN, LIN, "pick_ord1",	2, 2, CHARTYPE,
		"U", "          ",
		" ", "L", "1st stock Picking Order  : ", "L(ocation) E(xpiry) F(ifo). ",
		YES, NO,  JUSTLEFT, "LEF", "", local_rec.PickOrder1},
	{WH_SCN, LIN, "pick_flag1",	2, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "LP", "Pick Flags.               : ", "L)oose,B)ulk,D)amaged,P)icking,S)alvage, C)lean, Q)A, O)ther, R)efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSRCOQ", "", llct_rec.pick_flg1},
	{WH_SCN, LIN, "pick_ord2",	3, 2, CHARTYPE,
		"U", "          ",
		" ", "E", "2nd stock Picking Order. : ", "L(ocation) E(xpiry) F(ifo). ",
		YES, NO,  JUSTLEFT, "LEF", "", local_rec.PickOrder2},
	{WH_SCN, LIN, "pick_flag2",	3, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "B", "Pick Flags.               : ", "L(oose,B(ulk,D(amaged,P(icking,S(alvage,C(lean,Q(A,O(ther,R(efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSRCOQ", "", llct_rec.pick_flg2},
	{WH_SCN, LIN, "pick_ord3",	4, 2, CHARTYPE,
		"U", "          ",
		" ", "F", "3rd stock Picking Order. : ", "L(ocation) E(xpiry) F(ifo). ",
		YES, NO,  JUSTLEFT, "LEF", "", local_rec.PickOrder3},
	{WH_SCN, LIN, "pick_flag3",	4, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "DSCQOR", "Pick Flags.               : ", "L(oose,B(ulk,D(amaged,P(icking,S(alvage,C(lean,Q(A,O(ther,R(efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSRCOQ", "", llct_rec.pick_flg3},

	{WH_SCN, LIN, "alt_uom",		6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Allow Alternate UOM      : ", "Y(es) Allow alternate UOM's to be used to fill required quantity. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.alt_uom},
	{WH_SCN, LIN, "alt_uom_desc",		6, 32, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom_desc},
	{WH_SCN, LIN, "auto_all",		6, 40, CHARTYPE,
		"U", "          ",
		" ", "Y", "Allow Auto Allocation     : ", "Y(es) Allow automatic allocation of stock.",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.auto_all},
	{WH_SCN, LIN, "auto_all_desc",		6, 71, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.auto_all_desc},
	{WH_SCN, LIN, "exp_items",		7, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Allow Expired Items      : ", "Y(es) Allow expired items to be picked. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.exp_items},
	{WH_SCN, LIN, "exp_items_desc",		7, 32, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.exp_items_desc},
	{WH_SCN, LIN, "all_locs",		7, 40, CHARTYPE,
		"U", "          ",
		" ", "N", "Display All Locations     : ", "Y(es) display all locations including zero and negative ones.",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.all_locs},
	{WH_SCN, LIN, "all_locs_desc",		7, 71, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.all_locs_desc},

	{WH_SCN, LIN, "only_location",	9, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Use only this location.  : ", "Default <space> if location input this is only location used for WH.",
		YES, NO,  JUSTLEFT, "", "", ccmr_rec.only_loc},
	{WH_SCN, LIN, "dflt_location",	9, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Default location.         : ", "Default location for WH when no valid location found for Item.",
		YES, NO,  JUSTLEFT, "", "", llct_rec.dflt_loc},
	{WH_SCN, LIN, "val_locs",		11, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "LBPRCODQS", "Enter Location types     : ", "L(oose,B(ulk,D(amaged,P(icking,S(alvage,C(lean,Q(A,O(ther,R(efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSRCO", "", llct_rec.val_locs},
	{WH_SCN, LIN, "pick_locs",		12, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "LPBO", "Pick Location types      : ", "L(ocation,B(ulk,D(amaged,P(icking,S(alvage,C(lean,Q(A,O(ther,R(efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSCQOR", "", llct_rec.pick_locs},
	{WH_SCN, LIN, "rept_locs",		13, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "BPLO", "Receipt location types   : ", "L(oose,B(ulk,D(amaged,P(icking,S(alvage,C(lean,Q(A,O(ther,R(efrigerated",
		YES, NO,  JUSTLEFT, "LBDPSCQOR", "", llct_rec.rept_locs},
	{WH_SCN, LIN, "so_invoice",		14, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Direct Invoicing (M/V/A) : ", "M(anually Allocate) V(iew Allocation) A(utomatic Allocation)",
		YES, NO,  JUSTLEFT, "MVA", "", local_rec.ProgPick [0]},
	{WH_SCN, LIN, "so_invoice_desc",		14, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [0]},

	{WH_SCN, LIN, "so_input",		15, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Sales Order    (M/V/A/N) : ", "M(anually Allocate) V(iew Allocation) A(utomatic Alloc.) N(o Allocation)",
		YES, NO,  JUSTLEFT, "MVAN", "", local_rec.ProgPick [1]},
	{WH_SCN, LIN, "so_input_desc",		15, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [1]},

	{WH_SCN, LIN, "so_credit",		16, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Direct Credit (M/V/A)    : ", "M(anually Allocate) V(iew Allocation) A(utomatic Allocation)",
		YES, NO,  JUSTLEFT, "MVA", "", local_rec.ProgPick [2]},
	{WH_SCN, LIN, "so_credit_desc",		16, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [2]},

	{WH_SCN, LIN, "so_des_conf",		17, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Desp. Confirmation(M/V/A): ", "M(anually Allocate) V(iew Allocation) A(utomatic Allocation)",
		YES, NO,  JUSTLEFT, "MVA", "", local_rec.ProgPick [3]},
	{WH_SCN, LIN, "so_des_conf_desc",		17, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [3]},

	{WH_SCN, LIN, "so_ades_conf",		18, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Auto Des. Confirm.(A)    : ", "A(utomatic Allocation)",
		YES, NO,  JUSTLEFT, "A", "", local_rec.ProgPick [4]},
	{WH_SCN, LIN, "so_ades_conf_desc",		18, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [4]},
	{WH_SCN, LIN, "pc_issue",		19, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Issue Materials (M/V/A)  : ", "M(anually Allocate) V(iew Allocation) A(utomatic Allocation)",
		YES, NO,  JUSTLEFT, "MVA", "", local_rec.ProgPick [5]},
	{WH_SCN, LIN, "pc_issue_desc",		19, 33, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ProgPickDesc [5]},

	{OPTION_SCN, LIN, "mst_wh",	2, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Master Warehouse.         : ", "Enter Y if this is the Master Warehouse. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.mst_wh},
	{OPTION_SCN, LIN, "mst_wh_desc",	2, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.mst_wh_desc},

	{OPTION_SCN, LIN, "sal_ok",	3, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sales Allowed.            : ", "Enter Y if Sales are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.sal_ok},
	{OPTION_SCN, LIN, "sal_ok_desc",	3, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.sal_ok_desc},

	{OPTION_SCN, LIN, "pur_ok",	4, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Purchases Allowed         : ", "Enter Y if Purchases are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.pur_ok},
	{OPTION_SCN, LIN, "pur_ok_desc",	4, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.pur_ok_desc},

	{OPTION_SCN, LIN, "trans_ok",	5, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Transfers Allowed         : ", "Enter Y if Transfers (Issues / Receipts) are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.trans_ok},
	{OPTION_SCN, LIN, "trans_ok_desc",	5, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.trans_ok_desc},

	{OPTION_SCN, LIN, "iss_ok",	6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Issues Allowed            : ", "Enter Y if Issues are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.iss_ok},
	{OPTION_SCN, LIN, "iss_ok_desc",	6, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.iss_ok_desc},

	{OPTION_SCN, LIN, "rec_ok",	7, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Receipts Allowed          : ", "Enter Y if Receipts are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rec_ok},
	{OPTION_SCN, LIN, "rec_ok_desc",	7, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.rec_ok_desc},

	{OPTION_SCN, LIN, "rep_ok",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Stock Reports. Allowed    : ", "Enter Y if Stock Reports are Allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_ok},
	{OPTION_SCN, LIN, "rep_ok_desc",	8, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.rep_ok_desc},

	{OPTION_SCN, LIN, "lrp_ok",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "LRP Demand and reorder    : ", "Enter Y if LRP demand and reorder allowed. ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.lrp_ok},
	{OPTION_SCN, LIN, "lrp_ok_desc",	9, 33, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.lrp_ok_desc},
	{OPTION_SCN, LIN, "lpno",		10, 2, INTTYPE,
		"NN", "          ",
		" ", "0", "Warehouse Printer         : ", "Default if Warehouse has no printer. ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*============================
| Local function prototypes  |
============================*/
int		LocChkLocation	 (long, char *);
int		heading			 (int);
int		spec_valid		 (int);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	LoadCcmr		 (void);
void	SrchComr		 (char *);
void	SrchEsmr		 (char *);
void	SrchExsf		 (char *);
void	SrchCcmr		 (char *);
void	Update			 (void);
void	SrchLomr		 (long, char *);
void 	SetupPrompts 	 (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	SETUP_SCR (vars);

	TruePosition = TRUE;

	curr_user = getenv ("LOGNAME");

	sptr = chk_env ("MULT_LOC");
	envVarMultLoc = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	OpenDB ();

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();			/*  get into raw mode		*/
	_set_masks ("wh_maint.s");			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	SetupPrompts ();

	while (prog_exit == 0)
	{
		skip_entry 	= FALSE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newRecord 	= FALSE;
		search_ok 	= TRUE;
		init_vars (MAIN_SCN);
		init_vars (WH_SCN);
		init_vars (OPTION_SCN);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (MAIN_SCN);
		entry (MAIN_SCN);
		if (restart || prog_exit) 
			continue;
				
		if (!newWarehouse)
		{
			heading (MAIN_SCN);
			scn_display (MAIN_SCN);
		}
		else
		{
			heading (WH_SCN);
			entry (WH_SCN);

			heading (OPTION_SCN);
			entry (OPTION_SCN);
		}

		/*------------------------
		| Edit screen 1 linear . |
		------------------------*/
		if (restart) 
			continue;

		edit_all ();
		if (restart)
			continue;

		Update (); 
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
======================= */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	
	abc_alias (ccmr2, ccmr);
	open_rec (llct, llct_list, LLCT_NO_FIELDS, "llct_hhcc_hash");
	open_rec (lomr, lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no2");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (llct);
	abc_fclose (lomr);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (exsf);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		i;	
	char *	sptr;

	if (LCHECK ("co_no"))
	{
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (ccmr_rec.co_no [0] == '0')
		{
			print_mess (ML ("Sorry, Company 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (comr_rec.co_no,ccmr_rec.co_no);
		cc = find_rec (comr,&comr_rec,COMPARISON,"r");
		if (cc)
		{
			/*------------------------
			| Company No. Not found. |
			------------------------*/
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("co_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("br_no"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (ccmr_rec.est_no [0] == '0')
		{
			print_mess (ML ("Sorry, Branch 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (esmr_rec.co_no,ccmr_rec.co_no);
		strcpy (esmr_rec.est_no,ccmr_rec.est_no);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			/*-----------------------
			| Branch No. Not found. |
			-----------------------*/
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("br_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_no"))
	{
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			sprintf (ccmr_rec.name,"%-40.40s"," ");
			sprintf (ccmr_rec.acronym,"%-9.9s"," ");
			DSP_FLD ("wh_name");
			DSP_FLD ("wh_acronym");
			return (EXIT_SUCCESS);
		}
		if (ccmr_rec.cc_no [0] == '0')
		{
			print_mess (ML ("Sorry, Warehouse 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!strcmp (ccmr_rec.cc_no, "  "))
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		newWarehouse = find_rec (ccmr,&ccmr_rec,COMPARISON,"u");
		if (newWarehouse)
		{
			abc_unlock (ccmr);
			newRecord 			= TRUE;
			newControlRecord 	= TRUE;
			ccmr_rec.hhcc_hash	= 0L;
		}
		else
		{
			newRecord			= 0;
			entry_exit 			= 1;
			LoadCcmr ();
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_add1"))
	{
		if (!strcmp(ccmr_rec.whse_add1, FORTY_SPACES))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_add2"))
	{
		if (!strcmp(ccmr_rec.whse_add2, FORTY_SPACES))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_add3"))
	{
		if (!strcmp(ccmr_rec.whse_add3, FORTY_SPACES))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_add4"))
	{
		if (!strcmp(ccmr_rec.whse_add4, FORTY_SPACES))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			sprintf (exsf_rec.salesman, "%-40.40s", " ");
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no,comr_rec.co_no);
		strcpy (exsf_rec.salesman_no,ccmr_rec.sman_no);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Salesman not found. |
			---------------------*/
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("sman_name");
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("mst_wh"))
	{
	 	if (local_rec.mst_wh [0] == 'Y')
		{
			/*Only one master warehouse is allowed per branch*/
			strcpy (ccmr2_rec.co_no, comr_rec.co_no);
			strcpy (ccmr2_rec.est_no, esmr_rec.est_no);
			strcpy (ccmr2_rec.master_wh,"Y");
			cc = find_rec (ccmr2, &ccmr2_rec,EQUAL, "r");
			if (!cc)
			{
				if (strcmp(ccmr_rec.cc_no,ccmr2_rec.cc_no))
				{
					print_mess (ML("Only one master warehouse is allowed per branch"));			
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}
			}
			strcpy (local_rec.mst_wh_desc,promptYes);
		}
		else
			strcpy (local_rec.mst_wh_desc,promptNo);

		DSP_FLD ("mst_wh_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sal_ok"))
	{
		strcpy (local_rec.sal_ok_desc,
			 (local_rec.sal_ok [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("sal_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pur_ok"))
	{
		strcpy (local_rec.pur_ok_desc,
			 (local_rec.pur_ok [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("pur_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("trans_ok"))
	{
		strcpy (local_rec.trans_ok_desc,
			 (local_rec.trans_ok [0] == 'Y') ? promptYes : promptNo);

		if (local_rec.trans_ok [0] == 'Y')
		{
			strcpy (local_rec.iss_ok,"Y");
			strcpy (local_rec.iss_ok_desc, promptYes);
			strcpy (local_rec.rec_ok,"Y");
			strcpy (local_rec.rec_ok_desc,promptYes);

			DSP_FLD ("iss_ok");
			DSP_FLD ("iss_ok_desc");
			DSP_FLD ("rec_ok");
			DSP_FLD ("rec_ok_desc");
			skip_entry += 2;
		}
		DSP_FLD ("trans_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("iss_ok"))
	{
		strcpy (local_rec.iss_ok_desc,
				 (local_rec.iss_ok [0] == 'Y') ? promptYes : promptNo);

		if (local_rec.iss_ok [0] == 'N')
		{
			strcpy (local_rec.trans_ok,"N");
			strcpy (local_rec.trans_ok_desc,promptNo);
			display_field (label ("trans_ok"));
			display_field (label ("trans_ok_desc"));
		}
		DSP_FLD ("iss_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rec_ok"))
	{
		strcpy (local_rec.rec_ok_desc,
				 (local_rec.rec_ok [0] == 'Y') ? promptYes : promptNo);

		if (local_rec.rec_ok [0] == 'N')
		{
			strcpy (local_rec.trans_ok,"N");
			strcpy (local_rec.trans_ok_desc, promptNo);
			DSP_FLD ("trans_ok");
			DSP_FLD ("trans_ok_desc");
		}
		DSP_FLD ("rec_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rep_ok"))
	{
		strcpy (local_rec.rep_ok_desc,
				 (local_rec.rep_ok [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("rep_ok_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lrp_ok"))
	{
		strcpy (local_rec.lrp_ok_desc,
				 (local_rec.lrp_ok [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("lrp_ok_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (dflt_used)
		{
			local_rec.lpno = 0;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{	
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*----------------
			| Invalid Printer |
			----------------*/
			print_mess (ML (mlStdMess020)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	if (LCHECK ("alt_uom"))
	{
		strcpy (local_rec.alt_uom_desc,
					 (local_rec.alt_uom [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("alt_uom_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("auto_all"))
	{
		strcpy (local_rec.auto_all_desc,
					 (local_rec.auto_all [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("auto_all_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("exp_items"))
	{
		strcpy (local_rec.exp_items_desc,
					 (local_rec.exp_items [0] == 'Y') ? promptYes : promptNo);
		DSP_FLD ("exp_items_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("all_locs"))
	{
		strcpy (local_rec.all_locs_desc,
					 (local_rec.all_locs [0] == 'Y') ? promptYes: promptNo);
		DSP_FLD ("all_locs_desc");
		return (EXIT_SUCCESS);
	}
						
	if (LCHECK ("pick_ord1"))
	{
		if (local_rec.PickOrder2 [0] == local_rec.PickOrder1 [0])
		{
			/*------------------------------------------------
			| Pick order 1 cannot be the same as Pick order 2|
			------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 1, 2);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.PickOrder3 [0] == local_rec.PickOrder1 [0])
		{
			/*-------------------------------------------------
			| Pick order 1 cannot be the same as Pick order 3 |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 1, 3);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pick_ord2"))
	{
		if (local_rec.PickOrder1 [0] == local_rec.PickOrder2 [0])
		{
			/*-------------------------------------------------
			| Pick order 2 cannot be the same as Pick order 1 |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 2, 1);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.PickOrder3 [0] == local_rec.PickOrder2 [0])
		{
			/*-------------------------------------------------
			| Pick order 2 cannot be the same as Pick order 3 |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 2, 3);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pick_ord3"))
	{
		if (local_rec.PickOrder1 [0] == local_rec.PickOrder3 [0])
		{
			/*-------------------------------------------------
			| Pick order 3 cannot be the same as Pick order 1 |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 3, 1);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.PickOrder2 [0] == local_rec.PickOrder3 [0])
		{
			/*-------------------------------------------------
			| Pick order 3 cannot be the same as Pick order 2 |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlMenuMess176), 3, 2);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pick_flag1"))
	{
		for (i = 0; i < (int) strlen (clip (llct_rec.pick_flg1)); i++)
		{
			sptr = strchr (llct_rec.pick_flg2, llct_rec.pick_flg1 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 2nd location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "2nd");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sptr = strchr (llct_rec.pick_flg3, llct_rec.pick_flg1 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 3rd location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "3rd");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (envVarMultLoc && !strlen (clip (llct_rec.pick_flg1)))
		{
			print_mess (ML (mlSkMess581));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pick_flag2"))
	{
		for (i = 0; i < (int) strlen (clip (llct_rec.pick_flg2)); i++)
		{
			sptr = strchr (llct_rec.pick_flg1, llct_rec.pick_flg2 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 1st location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "1st");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sptr = strchr (llct_rec.pick_flg3, llct_rec.pick_flg2 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 3rd location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "3rd");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pick_flag3"))
	{
		for (i = 0; i < (int) strlen (clip (llct_rec.pick_flg3)); i++)
		{
			sptr = strchr (llct_rec.pick_flg1, llct_rec.pick_flg3 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 1st location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "1st");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			sptr = strchr (llct_rec.pick_flg2, llct_rec.pick_flg3 [i]);
			if (sptr != (char *)0)
			{
				/*---------------------------------------
				| Pick location is already specified |
				| in 2nd location pick flags.			|
				---------------------------------------*/
				sprintf (err_str, ML (mlMenuMess177), "2nd");
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("so_invoice"))
	{
		if (local_rec.ProgPick [0] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [0], promptManual);

		if (local_rec.ProgPick [0] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [0], promptView);

		if (local_rec.ProgPick [0] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [0], promptAuto);

		DSP_FLD ("so_invoice_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("so_input"))
	{
		if (local_rec.ProgPick [1] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [1],promptManual);

		if (local_rec.ProgPick [1] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [1],promptView);

		if (local_rec.ProgPick [1] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [1],promptAuto);

		if (local_rec.ProgPick [1] [0]	== 'N')
			strcpy (local_rec.ProgPickDesc [1],promptNoAlloc);

		DSP_FLD ("so_input_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("so_credit"))
	{
		if (local_rec.ProgPick [2] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [2],promptManual);

		if (local_rec.ProgPick [2] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [2],promptView);

		if (local_rec.ProgPick [2] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [2],promptAuto);

		DSP_FLD ("so_credit_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("so_des_conf"))
	{
		if (local_rec.ProgPick [3] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [3],promptManual);

		if (local_rec.ProgPick [3] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [3],promptView);

		if (local_rec.ProgPick [3] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [3],promptAuto);

		DSP_FLD ("so_des_conf_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("so_ades_conf"))
	{
		if (local_rec.ProgPick [4] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [4],promptManual);

		if (local_rec.ProgPick [4] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [4],promptView);

		if (local_rec.ProgPick [4] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [4],promptAuto);

		DSP_FLD ("so_ades_conf_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pc_issue"))
	{
		if (local_rec.ProgPick [5] [0]	== 'M')
			strcpy (local_rec.ProgPickDesc [5],promptManual);

		if (local_rec.ProgPick [5] [0]	== 'V')
			strcpy (local_rec.ProgPickDesc [5],promptView);

		if (local_rec.ProgPick [5] [0]	== 'A')
			strcpy (local_rec.ProgPickDesc [5],promptAuto);

		DSP_FLD ("pc_issue_desc");
		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Royalty Code  Search. |
	--------------------------------*/
	if (LCHECK ("only_location"))
	{
		if (dflt_used)
		{
			strcpy (ccmr_rec.only_loc, " ");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchLomr (ccmr_rec.hhcc_hash, temp_str);
			return (EXIT_SUCCESS);
		}
			
		cc = LocChkLocation (ccmr_rec.hhcc_hash, ccmr_rec.only_loc);
		if (cc)
		{
			print_mess (ML (mlStdMess209));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Royalty Code  Search. |
	--------------------------------*/
	if (LCHECK ("dflt_location"))
	{
		if (SRCH_KEY)
		{
			SrchLomr (ccmr_rec.hhcc_hash, temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (llct_rec.dflt_loc, " ");
			return (EXIT_SUCCESS);
		}
		if (envVarMultLoc && !strcmp (llct_rec.dflt_loc, "          "))
		{
			print_mess (ML ("Default location must be defined."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		cc = LocChkLocation (ccmr_rec.hhcc_hash, llct_rec.dflt_loc);
		if (cc)
		{
			print_mess (ML (mlStdMess209));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
LoadCcmr (
 void)
{
	strcpy (local_rec.mst_wh, 
					(ccmr_rec.master_wh [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.mst_wh_desc,
					(ccmr_rec.master_wh [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.sal_ok,
					 (ccmr_rec.sal_ok [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.sal_ok_desc,
					 (ccmr_rec.sal_ok [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.pur_ok, 
					 (ccmr_rec.pur_ok [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.pur_ok_desc, 
					 (ccmr_rec.pur_ok [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.trans_ok, 
					 (ccmr_rec.issues_ok [0] == 'Y' && 
		  	  		 ccmr_rec.receipts [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.trans_ok_desc, 
					 (ccmr_rec.issues_ok [0] == 'Y' && 
		  	  		 ccmr_rec.receipts [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.iss_ok, 
					 (ccmr_rec.issues_ok [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.iss_ok_desc, 
					 (ccmr_rec.issues_ok [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.rec_ok, 
					 (ccmr_rec.receipts [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.rec_ok_desc, 
					 (ccmr_rec.receipts [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.rep_ok, 
					 (ccmr_rec.reports_ok [0] == 'Y') ? "Y" : "N");
	strcpy (local_rec.rep_ok_desc, 
					 (ccmr_rec.reports_ok [0] == 'Y') ? promptYes : promptNo);
	strcpy (local_rec.lrp_ok, 
					 (ccmr_rec.lrp_ok [0] == 'N') ? "N" : "Y");
	strcpy (local_rec.lrp_ok_desc, 
					 (ccmr_rec.lrp_ok [0] == 'N') ? promptNo : promptYes);

	strcpy (exsf_rec.co_no,comr_rec.co_no);
	strcpy (exsf_rec.salesman_no,ccmr_rec.sman_no);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");

	llct_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
	newControlRecord = find_rec (llct, &llct_rec, COMPARISON, "u");
	if (!newControlRecord)
	{
		strcpy (local_rec.alt_uom, (llct_rec.alt_uom [0] == 'Y') ? "Y" : "N");
		strcpy (local_rec.alt_uom_desc,
					 (llct_rec.alt_uom [0] == 'Y') ? promptYes : promptNo);
		strcpy (local_rec.auto_all, (llct_rec.auto_all [0] == 'Y') ? "Y" : "N");
		strcpy (local_rec.auto_all_desc,
					 (llct_rec.auto_all [0] == 'Y') ? promptYes : promptNo);

		strcpy (local_rec.exp_items, (llct_rec.exp_items [0] == 'Y') ? "Y":"N");
		strcpy (local_rec.exp_items_desc,
					 (llct_rec.exp_items [0] == 'Y') ? promptYes : promptNo);

		strcpy (local_rec.all_locs, (llct_rec.all_locs [0] == 'Y') ? "Y":"N");
		strcpy (local_rec.all_locs_desc,
					 (llct_rec.all_locs [0] == 'Y') ? promptYes : promptNo);

		sprintf (local_rec.PickOrder1, "%-1.1s", llct_rec.pick_ord);
		sprintf (local_rec.PickOrder2, "%-1.1s", llct_rec.pick_ord + 1);
		sprintf (local_rec.PickOrder3, "%-1.1s", llct_rec.pick_ord + 2);

		strcpy (local_rec.ProgPick [5],llct_rec.pc_issue);
		switch (llct_rec.invoice [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [0],"M");
			strcpy (local_rec.ProgPickDesc [0],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [0],"V");
			strcpy (local_rec.ProgPickDesc [0],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [0],"A");
			strcpy (local_rec.ProgPickDesc [0],promptAuto);
			break;
		}

		switch (llct_rec.input [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [1],"M");
			strcpy (local_rec.ProgPickDesc [1],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [1],"V");
			strcpy (local_rec.ProgPickDesc [1],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [1],"A");
			strcpy (local_rec.ProgPickDesc [1],promptAuto);
			break;

		case 'N':
			strcpy (local_rec.ProgPick [1],"N");
			strcpy (local_rec.ProgPickDesc [1],"No Allocation");
			break;
		}

		switch (llct_rec.credit [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [2],"M");
			strcpy (local_rec.ProgPickDesc [2],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [2],"V");
			strcpy (local_rec.ProgPickDesc [2],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [2],"A");
			strcpy (local_rec.ProgPickDesc [2],promptAuto);
			break;
		}

		switch (llct_rec.des_conf [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [3],"M");
			strcpy (local_rec.ProgPickDesc [3],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [3],"V");
			strcpy (local_rec.ProgPickDesc [3],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [3],"A");
			strcpy (local_rec.ProgPickDesc [3],promptAuto);
			break;
		}

		switch (llct_rec.ades_conf [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [4],"M");
			strcpy (local_rec.ProgPickDesc [4],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [4],"V");
			strcpy (local_rec.ProgPickDesc [4],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [4],"A");
			strcpy (local_rec.ProgPickDesc [4],promptAuto);
			break;
		}
		switch (llct_rec.pc_issue [0])
		{
		case 'M':
			strcpy (local_rec.ProgPick [5],"M");
			strcpy (local_rec.ProgPickDesc [5],promptManual);
			break;

		case 'V':
			strcpy (local_rec.ProgPick [5],"V");
			strcpy (local_rec.ProgPickDesc [5],promptView);
			break;

		case 'A':
			strcpy (local_rec.ProgPick [5],"A");
			strcpy (local_rec.ProgPickDesc [5],promptAuto);
			break;
		}
	}
}

void
SrchComr (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#Co","#Company Name");
	sprintf (comr_rec.co_no,"%2.2s",key_val);
	cc = find_rec (comr,&comr_rec,GTEQ,"r");
	while (!cc && !strncmp (comr_rec.co_no,key_val,strlen (key_val)))
	{
		cc = save_rec (comr_rec.co_no,comr_rec.co_name);
		if (cc)
			break;
		cc = find_rec (comr,&comr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	sprintf (comr_rec.co_no,"%2.2s",temp_str);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");
}

void
SrchEsmr (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#Br","#Branch Name");
	strcpy (esmr_rec.co_no, ccmr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, ccmr_rec.co_no) && 
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
	sprintf (esmr_rec.co_no, ccmr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}

void
SrchExsf (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SM", "#Salesperson Name");
	strcpy (exsf_rec.co_no, comr_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");

	while (!cc && 
	       !strcmp (exsf_rec.co_no, comr_rec.co_no) && 
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

	strcpy (exsf_rec.co_no, comr_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

void
SrchCcmr (
 char *	key_val)
{
	char	co_no [3];
	char	est_no [3];

	_work_open (2,0,40);
	save_rec ("#Wh", "#Warehouse Name");
	strcpy (co_no, ccmr_rec.co_no);
	strcpy (est_no, ccmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (co_no, ccmr_rec.co_no) && 
	       !strcmp (est_no, ccmr_rec.est_no) && 
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
	sprintf (ccmr_rec.co_no, co_no);
	sprintf (ccmr_rec.est_no, est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
Update (
 void)
{
	clear ();

	sprintf (ccmr_rec.master_wh, "%-1.1s", local_rec.mst_wh);
	if (ccmr_rec.master_wh [0] == 'Y')
		strcpy (ccmr_rec.type, "MR");
	else
		strcpy (ccmr_rec.type, "  ");

	sprintf (ccmr_rec.sal_ok, "%-1.1s", local_rec.sal_ok);
	sprintf (ccmr_rec.pur_ok, "%-1.1s", local_rec.pur_ok);
	sprintf (ccmr_rec.issues_ok, "%-1.1s", local_rec.iss_ok);
	sprintf (ccmr_rec.receipts, "%-1.1s", local_rec.rec_ok);
	sprintf (ccmr_rec.reports_ok, "%-1.1s", local_rec.rep_ok);
	sprintf (ccmr_rec.lrp_ok, "%-1.1s", local_rec.lrp_ok);
	ccmr_rec.lpno = local_rec.lpno;
	strcpy (ccmr_rec.stat_flag, "0");

	sprintf (llct_rec.alt_uom, 	"%-1.1s", local_rec.alt_uom);
	sprintf (llct_rec.auto_all, "%-1.1s", local_rec.auto_all);
	sprintf (llct_rec.exp_items,"%-1.1s", local_rec.exp_items);
	sprintf (llct_rec.all_locs, "%-1.1s", local_rec.all_locs);
	sprintf (llct_rec.pick_ord, "%1.1s%1.1s%1.1s", 
										local_rec.PickOrder1,
										local_rec.PickOrder2,
										local_rec.PickOrder3);

	sprintf (llct_rec.invoice, 	"%-1.1s", local_rec.ProgPick [0]);
	sprintf (llct_rec.input, 	"%-1.1s", local_rec.ProgPick [1]);
	sprintf (llct_rec.credit, 	"%-1.1s", local_rec.ProgPick [2]);
	sprintf (llct_rec.des_conf,	"%-1.1s", local_rec.ProgPick [3]);
	sprintf (llct_rec.ades_conf, "%-1.1s", local_rec.ProgPick [4]);
	sprintf (llct_rec.pc_issue, "%-1.1s", local_rec.ProgPick [5]);
	strcpy (llct_rec.only_loc, ccmr_rec.only_loc);

	if (!newRecord)
	{
		cc = abc_update (ccmr, &ccmr_rec);
		if (cc)
			file_err (cc, ccmr, "DBUPDATE");
	}
	else 
	{
		cc = abc_add (ccmr, &ccmr_rec);
		if (cc)
			file_err (cc, ccmr, "DBADD");

		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	}
	if (newControlRecord)
	{
		llct_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		cc = abc_add (llct, &llct_rec);
		if (cc)
			file_err (cc, llct, "DBADD");
	}
	else
	{
		cc = abc_update (llct, &llct_rec);
		if (cc)
			file_err (cc, llct, "DBUPDATE");
	}
		
	abc_unlock (llct);
	abc_unlock (ccmr);
}

/*=====================================
| Check Location id a valid location. |
=====================================*/
int
LocChkLocation (
	long	hhccHash,
	char *	Loc)
{
	lomr_rec.hhcc_hash 	= hhccHash;
	if (Loc == (char *)0)
	{
		strcpy (lomr_rec.location, "          ");
		return (find_rec (lomr, &lomr_rec, GTEQ , "r"));
	}
	sprintf (lomr_rec.location,"%-10.10s",Loc);
	return (find_rec (lomr, &lomr_rec, COMPARISON, "r"));
}
/*=============================
| Search for Location master. |
=============================*/
void
SrchLomr (
 long	hhccHash,
 char *	KeyValue)
{
	_work_open (10,0,40);

	lomr_rec.hhcc_hash 	= 	hhccHash;
	sprintf (lomr_rec.location,"%-10.10s", KeyValue);
	
	cc = save_rec ("# Location ","# Location Description.");

	cc = find_rec (lomr,&lomr_rec,GTEQ,"r");
	while (!cc && lomr_rec.hhcc_hash == hhccHash &&
				!strncmp (lomr_rec.location,KeyValue,strlen (KeyValue)))
	{
		cc = save_rec (lomr_rec.location, lomr_rec.desc);
		if (cc)
			break;

		cc = find_rec (lomr, &lomr_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	lomr_rec.hhcc_hash 	= 	hhccHash;
	sprintf (lomr_rec.location,"%-10.10s", temp_str);
	cc = find_rec (lomr, &lomr_rec, GTEQ,"r");
	if (cc)
		file_err (cc, lomr, "DBFIND");
}

void
SetupPrompts (void)
{
	strcpy (promptYes, 		ML ("Yes"));
	strcpy (promptNo,  		ML ("No "));
	strcpy (promptManual,	ML ("Manually Allocated"));
	strcpy (promptView,		ML ("View Allocation"));
	strcpy (promptAuto,		ML ("Automatic Allocation"));
	strcpy (promptNoAlloc,	ML ("No allocation."));
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlMenuMess179), 23, 0, 1);

	if (scn == MAIN_SCN)
	{
		box (0, 1, 80, 15);
		line_at (6,1,79);
		line_at (14,1,79);
		line_at (20,0,80);
	}
	if (scn == WH_SCN)
	{
		box (0, 1, 80, 18);
		line_at (5,1,79);
		line_at (8,1,79);
		line_at (10,1,79);
	}
	if (scn == OPTION_SCN)
	{
		box (0, 1, 80, 9);
		line_at (20,0,80);
	}

	sprintf (err_str, ML (mlStdMess038), comr_rec.co_no,comr_rec.co_name);
	print_at (21,1,"%s", err_str);
	print_at (22,1, ML (mlStdMess039), esmr_rec.co_no,esmr_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_mr_inpt.c,v 5.15 2002/11/22 02:15:54 keytan Exp $
|  Program Name  : (db_mr_inpt.c)
|  Program Desc  : (Add / Change Customers)
|---------------------------------------------------------------------|
|  Date Written  : 05/01/87        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: db_mr_inpt.c,v $
| Revision 5.15  2002/11/22 02:15:54  keytan
| Updated to fix royalty class field input.
|
| Revision 5.14  2002/07/23 05:14:34  scott
| .
|
| Revision 5.13  2002/07/17 02:51:29  scott
| Updated from service calls and general maintenance.
|
| Revision 5.12  2002/07/12 05:40:58  robert
| S/C 4116 - Adjust description size
|
| Revision 5.11  2002/07/03 07:00:25  kaarlo
| S/C 4027. Updated to fix searc/input validation for Royalty Class and Discount Code.
|
| Revision 5.10  2002/04/11 03:31:00  scott
| Updated to add comments to audit files.
|
| Revision 5.9  2002/03/01 02:36:23  scott
| Updated for message on country
|
| Revision 5.8  2001/11/08 09:37:19  scott
| Updated to allow special instructions of 0-999 instead of 0-500
|
| Revision 5.7  2001/10/05 02:57:50  cha
| Added code to produce audit files.
|
| Revision 5.6  2001/09/19 00:44:59  scott
| General clean up.
|
| Revision 5.5  2001/08/28 06:16:55  scott
| Updated to change " ( to "(
|
| Revision 5.4  2001/08/24 06:13:12  scott
| Updated from scotts machine
|
| Revision 5.3  2001/08/09 09:03:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:22:19  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:15  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_mr_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_mr_inpt/db_mr_inpt.c,v 5.15 2002/11/22 02:15:54 keytan Exp $";

#define	MAXSCNS		6
#include <ml_db_mess.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <pslscr.h>
#include <GlUtils.h>
#include <DBAudit.h>
#define	INPUT		 (prog_status == ENTRY)

#define	TS_CUST 	 (teleSalesMaint && strlen (clip (local_rec.ts_cust)) != 0)

/*===========================
| Special fields and flags. |
===========================*/
int  	newCustomer = 0;

int		NO_DEFAULT = 0;	

char	branchNumber [3],
		cr_branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct srdbRecord	srdb_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct sumrRecord	sumr_rec;
struct tspmRecord	tspm_rec;
struct tmpfRecord	tmpf_rec;
struct cudpRecord	cudp_rec;
struct exclRecord	excl_rec;
struct exmmRecord	exmm_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct trzmRecord	trzm_rec;
struct exctRecord	exct_rec;
struct exdfRecord	exdf_rec;
struct exsiRecord	exsi_rec;
struct dbryRecord	dbry_rec;
struct cubdRecord	cubd_rec;
struct cumdRecord	cumd_rec;
struct pocfRecord	pocf_rec;
struct posdtupRecord	posdtup_rec;

int		*cumr_inst_fg	=	&cumr_rec.inst_fg1;
Money	*cumr_balance	=	&cumr_rec.bo_current;

/*-------------
| Table Names |
-------------*/
static char
	*data	= "data",
	*cumr2	= "cumr2";

/*=======
 Globals
=========*/
	char	prev_stop [2];

	char	*scn_desc [] = 
			{
				"Maintain Master file information Screen.",
				"Maintain Financial information Screen.",
				"Maintain Customer Control file Setup",
				"Maintain Customer Control flags.",
				"Maintain Telesales Specific Information."
			};

	int		teleSalesMaint;
	int		tsmpFind;
	int		checkError = TRUE;
	char	priceTypeComments [40];

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	int		dbCo;
	int		dbFind;
	int		dbMcurr;
	int		crCo;
	int		crFind;
	int		GlByClass;
	int		PosInstalled;
	int		sk_dbprinum;
	int		creditOveride;
	int		combineInvPack;
	char	currencyCode [4];
	char	countryCode [4];
	char	gst [2];
	char	gstCode [4];
	char	taxComment [70];
	char	taxAllowedCodes [5];
	char	taxCodePrompt [31];
	char	taxNumberPrompt [31];
	int		tsInstalled;
} envVar;

extern	int	TruePosition;

#include	<p_terms.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	ts_cust [7];
	char 	previousCustomer [7];
	char 	cust_no [7];
	char 	supp_no [7];
	int  	a_code;
	char 	typedesc [20];
	char 	pri_show [30];
	char	pri_show_desc [16];
	char	bank [25];
	char	cont_pos [4];
	char	email [2][61];
	char	cont_pos_desc [41];
	char	alt_cont [21];
	char	alt_pos [4];
	char	alt_pos_desc [41];
	char	bst_ph_time [6];
	int		ph_freq;
	long	nxt_ph_date;
	char	nxt_ph_time [6];
	int		vs_freq;
	long	nxt_vs_date;
	char	nxt_vs_time [6];
	char	rec_mailer [4];
	char	rec_mailer_desc [4];
	char	sales_per [8];
	char	sales_per_desc [8];
	char	acct_desc [81];
	char 	bf_dbt [31];
	char 	bf_stm [31];
 	char	mail_lbls [2];
 	char	cust_lett [2];
	char	exmm_name [41];
	char	spindesc [3][61];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "dbtrno",	 2, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " Customer No.       :  ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "estno",	 2, 46, CHARTYPE,
		"NN", "          ",
		" ", branchNumber, " Branch        :  ", "Enter <return> to default to Current branch.",
		 NO, NO, JUSTRIGHT, "0", "99", cumr_rec.est_no},
	{1, LIN, "tsdbtrno",	 2, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " Customer No.       :  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "acronym",	 3, 1, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", " Acronym            :  ", "Enter customers acronym, Duplicates are not allowed. ",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.dbt_acronym},
	{1, LIN, "depart",	 3, 46, CHARTYPE,
		"AA", "          ",
		" ", "1", " Department    :  ", "Enter Department number, [SEARCH]. ",
		YES, NO, JUSTRIGHT, "0", "99", cumr_rec.department},
	{1, LIN, "name",	 4, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Name               :  ", " ",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "chaddr1",	6, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Charge To Address  :  ", " ",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr1},
	{1, LIN, "chaddr2",	7, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr2},
	{1, LIN, "chaddr3",	8, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr3},
	{1, LIN, "chaddr4",	9, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr4},
	{1, LIN, "dladdr1", 10, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Deliver To Address :  ", "Press <Return> if Delivery Add = Charge Add",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr1},
	{1, LIN, "dladdr2",	11, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr2},
	{1, LIN, "dladdr3",	12, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr3},
	{1, LIN, "dladdr4",	13, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                    :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.dl_adr4},
	{1, LIN, "deliveryZone", 15, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", " Delivery Zone Code :  ", "Enter Valid Delivery Zone Code. [SEARCH] <return = Not Applicable.>", 
		NO, NO, JUSTLEFT, "", "", cumr_rec.del_zone}, 
	{1, LIN, "zoneDescription", 15, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", trzm_rec.desc},
	{1, LIN, "post_code",	 17, 1, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Post Code          :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.post_code},
	{1, LIN, "contact",	 18, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Contact name 1     :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.contact_name},
	{1, LIN, "contact2",	 19, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Contact name 2     :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.contact2_name},
	{1, LIN, "contact3",	 20, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Contact name 3     :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.contact3_name},
	{1, LIN, "phoneno",	 18, 46, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Phone No.     :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.phone_no},
	{1, LIN, "Fax",	19, 46, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Fax No.       :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.fax_no},
	{1, LIN, "telex",	20, 46, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", " Telex No.     :  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.telex},
	{2, LIN, "curcode",	 3, 1, CHARTYPE,
		"UUU", "          ",
		" ", envVar.currencyCode, " Currency Code.   : ", "enter currency or use [SEARCH] Key",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.curr_code},
	{2, LIN, "ctrycode",	 3, 40, CHARTYPE,
		"UUU", "          ",
		" ", envVar.countryCode, " Country Code     : ", "enter country or use [SEARCH] Key. ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.ctry_code},
	{2, LIN, "acctype",	 4, 1, CHARTYPE,
		"U", "          ",
		" ", "O", " Account type     : ", "Enter B (alance Forward) O (pen Item)",
		 NO, NO,  JUSTLEFT, "BO", "", cumr_rec.acc_type},
	{2, LIN, "crdexpiry", 4, 40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Credit Expiry    : ", "Enter date when credit terms expire",
		 NO, NO, JUSTRIGHT, "", "", (char *)&cumr_rec.crd_expiry},
	{2, LIN, "stmt_type",	 5, 1, CHARTYPE,
		"U", "          ",
		" ", "O", " Statement type   : ", "Enter B (alance Forward) O (pen Item)",
		 NO, NO,  JUSTLEFT, "BO", "", cumr_rec.stmt_type},
	{2, LIN, "stmnt_flg",	 5, 40, CHARTYPE,
		"U", "          ",
		" ", "Y", " Statement Print  : ", "Enter  to enable statement printing, N (o to disable",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.stmnt_flg},
	{2, LIN, "credref",	 6, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Credit reference : ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.credit_ref},
	{2, LIN, "crd_ext",	 6, 40, INTTYPE,
		"NNN", "          ",
		" ", "0", " Credit Extention : ", "Credit Extention period in days",
		 NO, NO,  JUSTRIGHT, "0", "999", (char *)&cumr_rec.crd_ext},
	{2, LIN, "cash_flag",	 7, 1, CHARTYPE,
		"U", "          ",
		" ", "N", " Cash Customer    : ", " = Customer is cash only. N (o = Charge customer.",
		 YES, NO,  JUSTLEFT, "YyNn", "", cumr_rec.cash_flag},
	{2, LIN, "crd_overide",	 7, 40, CHARTYPE,
		"U", "          ",
		" ", "N", " Omit Credit Check: ", "Ignore credit checking.) / N (o).",
		 ND, NO,  JUSTLEFT, "YyNn", "", cumr_rec.crd_flag},
	{2, LIN, "crd_prd",	 8, 1, CHARTYPE,
		"UUU", "          ",
		" ", "20A", " Credit Terms     : ", "nnA to NNF (20A = 20th next mth); Alt.<nnn> = no.days",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.crd_prd},
	{2, LIN, "credlim",	 8, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", " Credit limit     : ", "Zero credit limit means no limit.",
		 NO, NO,  JUSTLEFT, "0", "99999999.99", (char *)&cumr_rec.credit_limit},
	{2, LIN, "stopcrd",	 9, 1, CHARTYPE,
		"U", "          ",
		" ", "N", " Stop Credit.     : ", " if Customer is on Stop Credit",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.stop_credit},
	{2, LIN, "pay_method",	 9, 40, CHARTYPE,
		"U", "          ",
		" ", "C", " Payment Method   : ", "C = Cheque, D = Direct or Draft payment",
		YES, NO,  JUSTLEFT, "CD", "", cumr_rec.pay_method},
	{2, LIN, "taxcode",	10, 1, CHARTYPE,
		"U", "          ",
		" ", "C", envVar.taxCodePrompt, envVar.taxComment,
		 NO, NO,  JUSTLEFT, envVar.taxAllowedCodes, "", cumr_rec.tax_code},
	{2, LIN, "taxno",	10, 40, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", envVar.taxNumberPrompt, "",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.tax_no},
	{2, LIN, "bank_branch",	12, 1, CHARTYPE,
		"UUU-UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", " Bank-Branch Code.: ", "Enter Valid Bank-Branch Code As AAA-AAAAA..A - Used for Bank Lodgements",
		 NO, NO,  JUSTLEFT, "", "", local_rec.bank},
	{2, LIN, "bank",	13, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Bank             : ", "Bank details for reference ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.bk_name},
	{2, LIN, "bk_branch",	13, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Branch          : ", "Branch details for reference ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.bk_branch},
	{2, LIN, "bk_code",	14, 1, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Bank Code        : ", "Bank Code details for reference ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.bk_code},
	{2, LIN, "bk_acno",	14, 40, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Bank Account No : ", "Bank Account details for reference ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.bk_acct_no},
	{2, LIN, "crd_no",	 16, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  " Supplier No.     : ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.supp_no},
	{2, LIN, "crd_name",	 16, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{2, LIN, "gl_acc",	 18, 1, CHARTYPE,
		GlMask, "          ",
		"0", " ", " Control Account  : ", "Enter account or use [SEARCH] Key.",
		YES, NO,  JUSTLEFT, "0123456789*", "", cumr_rec.gl_ctrl_acct},
	{2, LIN, "gl_acc_desc",	 19, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Account Desc     : ", " ",
		 NA, NO,  JUSTLEFT, "", "", glmrRec.desc},
	{2, LIN, "gl_acc_curr",	 19, 50, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", glmrRec.curr_code},
	{3, LIN, "sman",	 3, 1, CHARTYPE,
		"UU", "          ",
		" ", " 1", " Salesman code         :  ", "Enter Valid Salesman Number or [SEARCH]. ",
		 NO, NO, JUSTRIGHT, "", "", cumr_rec.sman_code},
	{3, LIN, "sal_name",	 4, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",   "                       :  ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{3, LIN, "area",	 5, 1, CHARTYPE,
		"UU", "          ",
		" ", " 1", " Area code.            :  ", "Enter valid area number or [SEARCH].",
		 NO, NO, JUSTRIGHT, "", "", cumr_rec.area_code},
	{3, LIN, "area_name",	 6, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",   "                       :  ", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{3, LIN, "cus_type",	 7, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", " Customer type         :  ", "Enter valid customer type or [SEARCH]. ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.class_type},
	{3, LIN, "type_name",	 8, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                       :  ", " ",
		 NA, NO,  JUSTLEFT, "", "", excl_rec.class_desc},
	{3, LIN, "cus_roy",	 9, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", " Royalty Class         :  ", "Enter Royalty Class or [SEARCH]. ",
		 NO, NO,  JUSTLEFT, "", "", dbry_rec.cr_type},
	{3, LIN, "roy_desc",	 10, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                       :  ", " ",
		 NA, NO,  JUSTLEFT, "", "", dbry_rec.desc},
	{3, LIN, "cont_type",	 11, 1, CHARTYPE,
		"UUU", "          ",
		" ", " ", " Contract Type         :  ", "Enter Contract Type or [SEARCH]. ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.cont_type},
	{3, LIN, "cont_desc",	 12, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                       :  ", " ",
		 NA, NO,  JUSTLEFT, "", "", exct_rec.cont_desc},
	{3, LIN, "merchant", 13, 1, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", " Merchandiser Code.    :  ", "Enter Valid Merchandiser Code. [SEARCH] <return = Not Applicable.>", 
		NO, NO, JUSTLEFT, "", "", (char *)&cumr_rec.merchant}, 
	{3, LIN, "mer_name", 14, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                       :  ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.exmm_name}, 
	{3, LIN, "spinst1",	16, 1, INTTYPE,
		"NNN", "          ",
		" ", "0", " S.Inst. 1: ", "Special instruction 1, Enter (0-999)",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&cumr_rec.inst_fg1},
    {3, LIN, "spindesc1",    16, 18, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [0]},	
	{3, LIN, "spinst2",	17, 1, INTTYPE,
		"NNN", "          ",
		" ", "0", " S.Inst. 2: ", "Special instruction 2, Enter (0-999)",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&cumr_rec.inst_fg2},
	{3, LIN, "spindesc2",    17, 18, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [1]},
	{3, LIN, "spinst3",	18, 1, INTTYPE,
		"NNN", "          ",
		" ", "0", " S.Inst. 3: ", "Special instruction 3, Enter (0-999)",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&cumr_rec.inst_fg3},
    {3, LIN, "spindesc3",    18, 18, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
         " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [2]},	
	{4, LIN, "b/oflag",	 3, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Back Orders Allowed.  :  ", " if Backorders Allowed",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.bo_flag},
	{4, LIN, "b/days",	4, 1, INTTYPE,
		"NNN", "          ",
		" ", "999", " Max Days On B/Order   :  ", "Maximum number of days Backorders Allowed to Stay 0-999",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&cumr_rec.bo_days},
	{4, LIN, "b/cons",	 5, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Consolidate B/Orders  :  ", " if Backorder Consolidations Allowed",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.bo_cons},
	{4, LIN, "p/oflag",	6, 1, CHARTYPE,
		"U", "          ",
		" ", "N",    " P/Order Required      :  ", " if Customers purchase order is required ",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.po_flag},
	{4, LIN, "pri_type",	7, 1, CHARTYPE,
		"N", "          ",
		" ", "1", " Price Type            :  ", priceTypeComments,
		 NO, NO,  JUSTLEFT, "", "", local_rec.pri_show},
	{4, LIN, "pri_type_desc", 7, 30, CHARTYPE,
        "AAAAAAAAAAAAAAA", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.pri_show_desc},
	{4, LIN, "discode",	8, 1, CHARTYPE,
		"U", "          ",
		" ", "", " Discount Code.        :  ", "Input A-Z.",
		 NO, NO,  JUSTLEFT, "A", "Z", cumr_rec.disc_code},
	{4, LIN, "discpc",	8, 30, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "%", "Input A-Z.",
		 NA, NO, JUSTRIGHT, "0", "100", (char *)&exdf_rec.disc_pc},
	{4, LIN, "spec_item_flag",	9, 1, CHARTYPE,
		"U", "          ",
		" ", "N", " Customer Item Codes ? :  ", "Use Customer Specific Item Codes",
		 YES, NO,  JUSTLEFT, "YN", "", cumr_rec.item_codes},
	{4, LIN, "nett_pri",	10, 1, CHARTYPE,
		"U", "          ",
		" ", "N", " Pricing To Show ?     :  ", "Y =  Net price used in Invoices/Credits/P-Slips N = To display Sale Price",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.nett_pri_prt},
	{4, LIN, "reprint",	10, 40, CHARTYPE,
		"U", "          ",
		" ", "N", " Reprint invoice ?     :  ", "Always reprint Combined invoice/p/slip at despatch confirmation.",
		 ND, NO,  JUSTLEFT, "YN", "", cumr_rec.reprint_inv},
	{4, LIN, "sos",	11, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Small Order Surcharge.:  ", " if small order Surcharge Applies to Customer",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.sur_flag},
	{4, LIN, "freight_chg",	12, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Freight Charg. Allowed:  ", " if Customer can be charged freight.",
		 NO, NO,  JUSTLEFT, "YN", "", cumr_rec.freight_chg},
	{4, LIN, "restock_fee",	13, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Re-Stock fee Allowed  :  ", " if Customer can be charged a Re-stocking Fee.",
		 NO , NO,  JUSTLEFT, "YN", "", cumr_rec.restock_fee},
 	{4, LIN, "mail_lbls",	14, 1, CHARTYPE,
 		"U", "          ",
 		" ", "Y", " Print Labels (Y/N) ?  :  ", "Print mailing labels?) / N (o).",
 		 NO, NO,  JUSTLEFT, "YN", "", local_rec.mail_lbls},
 	{4, LIN, "cust_lett",	15, 1, CHARTYPE,
 		"U", "          ",
 		" ", "Y", " Print Letters (Y/N) ? :  ", "Print customer letters?) / N (o).",
 		 NO, NO,  JUSTLEFT, "YN", "", local_rec.cust_lett},
	{4, LIN, "SpecialNotes",	 17, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  " Special Notes         : ", " ",
		 YES, NO,  JUSTLEFT, "", "", cumr_rec.spec_note1},
	{4, LIN, "SpecialNotes",	 18, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  "                       : ", " ",
		 YES, NO,  JUSTLEFT, "", "", cumr_rec.spec_note2},

	{5, LIN, "dt_create",	 3, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Date Created   :  ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&tspm_rec.date_create},
	{5, LIN, "dt_lphone",	 3, 40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Date Last Phoned :  ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&tspm_rec.lphone_date},
	{5, LIN, "curr_op",	 4, 1, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", " Curr. Operator :  ", "",
		 NA, NO,  JUSTLEFT, "", "", tspm_rec.op_code},
	{5, LIN, "last_op",	 4, 40, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", " Last Operator    :  ", "",
		 NA, NO,  JUSTLEFT, "", "", tspm_rec.lst_op_code},
	{5, LIN, "cont_name",	 6, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Contact        :  ", "",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.contact_name},
	{5, LIN, "cont_pos",	 7, 1, CHARTYPE,
		"UUU", "          ",
		" ", "", " Position       :  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.cont_pos},
	{5, LIN, "cont_pos_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cont_pos_desc},
	{5, LIN, "email1",	 		8, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Email          : ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.email [0]},
	{5, LIN, "alt_cont",	 10, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Contact        :  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.alt_cont},
	{5, LIN, "alt_pos",	11, 1, CHARTYPE,
		"UUU", "          ",
		" ", "", " Position       :  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.alt_pos},
	{5, LIN, "alt_pos_desc",	11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.alt_pos_desc},
	{5, LIN, "email1",	 		12, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Email          : ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.email [1]},
	{5, LIN, "bst_ph_time",	14, 1, CHARTYPE,
		"AA:AA", "          ",
		" ", "", " Best Phone Time  :  ", "",
		 NO, NO,  JUSTLEFT, "0123456789", "", local_rec.bst_ph_time},
	{5, LIN, "ph_freq",	15, 1, INTTYPE,
		"NN", "          ",
		" ", "", " Phone Frequency  :  ", "",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.ph_freq},
	{5, LIN, "nxt_ph_date",	16, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Next Phone Date  :  ", "",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.nxt_ph_date},
	{5, LIN, "nxt_ph_time",	16, 40, CHARTYPE,
		"AA:AA", "          ",
		" ", "", " Next Phone Time  :  ", "",
		 NO, NO,  JUSTLEFT, "0123456789", "", local_rec.nxt_ph_time},
	{5, LIN, "vs_freq",	17, 1, INTTYPE,
		"NN", "          ",
		" ", "", " Visit Frequency  :  ", "",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.vs_freq},
	{5, LIN, "nxt_vs_date",	18, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Next Visit Date  :  ", "",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.nxt_vs_date},
	{5, LIN, "nxt_vs_time",	18, 40, CHARTYPE,
		"AA:AA", "          ",
		" ", "", " Next Visit Time  :  ", "",
		 NO, NO,  JUSTLEFT, "0123456789", "", local_rec.nxt_vs_time},
	{5, LIN, "rec_mailer",	20, 1, CHARTYPE,
		"U", "          ",
		" ", "Y", " Follow Up Mailer :  ", " Lead can receive follow up mailers (Y/N) ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.rec_mailer},
	{5, LIN, "rec_mailer_desc", 20, 25, CHARTYPE,
        "AAA", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.rec_mailer_desc},
	{5, LIN, "sales_per",	20, 40, CHARTYPE,
		"U", "          ",
		" ", "", " Sales Analysis   :  ", " D (aily) W (eekly) M (onthly) ",
		 NO, NO,  JUSTLEFT, "DWM", "", local_rec.sales_per},
	{5, LIN, "sales_per_desc", 20, 64, CHARTYPE,
        "AAAAAAA", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.sales_per_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindCumr.h>
#include <FindSumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	AddCubd 			 (void);
int 	Check_class 		 (void);
int 	heading 			 (int);
int 	PriceDesc 			 (void);
int 	spec_valid 			 (int);
int 	ValidateTime 		 (char *);
void 	CheckEnvironment 	 (void);
void 	CloseDB 			 (void);
void 	DisplayBranch 		 (void);
void 	FindMisc 			 (void);
void 	FindTspm 			 (void);
void 	GetAccount 			 (void);
void 	OpenDB 				 (void);
void 	ReadCumd 			 (void);
void 	SET_FLD 			 (char *);
void 	SrchCubd 			 (char *);
void 	SrchCudp 			 (char *);
void 	SrchDbry 			 (char *);
void 	SrchExaf 			 (char *);
void 	SrchExcl 			 (char *);
void 	SrchExct 			 (char *);
void 	SrchExdf 			 (char *);
void 	SrchExmm 			 (char *);
void 	SrchExsf 			 (char *);
void 	SrchExsi 			 (char *);
void 	SrchPay 			 (void);
void 	SrchPocf 			 (char *);
void 	SrchTmpf 			 (char *);
void 	SrchTrzm 			 (char *);
void 	UpdatePosdt 		 (void);
void 	Update_srdb 		 (void);
void 	Update 				 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	int		i;
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "ts_mr_inpt"))
		teleSalesMaint = TRUE;


	if (teleSalesMaint && argc >= 2)
		sprintf (local_rec.ts_cust, "%-6.6s", argv [1]);

	TruePosition	=	TRUE;

	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	if (envVar.tsInstalled)
		teleSalesMaint = TRUE;

	SETUP_SCR (vars);

	if (!envVar.dbMcurr)
	{
		FLD ("curcode")		=	NA;
		FLD ("ctrycode")	=	NA;
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	if (TS_CUST)
	{
		vars [label ("dbtrno")].scn = 6;
		FLD ("tsdbtrno") = NA;
	}

	init_scr ();
	set_tty ();

	_set_masks ( (teleSalesMaint) ? "ts_mr_inpt.s" : "db_mr_inpt.s");

	for (i = 0;i < 5;i++)
	{
		if (TS_CUST)
			tab_data [i + 1]._desc = scn_desc [i];
		else
			tab_data [i]._desc = scn_desc [i];
	}

	FLD ("reprint") = (envVar.combineInvPack) ? NO : ND;

	init_vars (1);

	OpenDB (); 	

	ReadCumd ();

	GL_SetMask (GlFormat);

	if (!NO_DEFAULT)
	{
		SET_FLD ("acctype");
		SET_FLD ("stmt_type");
		SET_FLD ("pri_type");
		SET_FLD ("pri_type_desc");
		SET_FLD ("crd_prd");
		SET_FLD ("stopcrd");
		SET_FLD ("cus_roy");
		SET_FLD ("b/oflag");
		SET_FLD ("b/cons");
		SET_FLD ("b/days");
		SET_FLD ("p/oflag");
		SET_FLD ("sos");
		SET_FLD ("freight_chg");
		SET_FLD ("restock_fee");
		SET_FLD ("nett_pri");
		SET_FLD ("taxcode");
		SET_FLD ("reprint");
 		SET_FLD ("gl_acc");
	}

	strcpy (branchNumber,   (envVar.dbCo) ? comm_rec.est_no : " 0");
	strcpy (cr_branchNumber, (envVar.crCo) ? comm_rec.est_no : " 0");

	FLD ("crd_overide") = (envVar.creditOveride) ? YES : ND;
	
	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newCustomer = FALSE;
		checkError	= TRUE;
		search_ok	= TRUE;
		abc_unlock (cumr);
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		if (TS_CUST)
		{
			sprintf (local_rec.cust_no, "%-6.6s",local_rec.ts_cust);
			init_ok = FALSE;
		}
		heading (1);
		if (TS_CUST)
			DSP_FLD ("tsdbtrno");

		entry (1);
		if (prog_exit || restart)
			continue;
		init_ok = TRUE;

		if (newCustomer == 1) 
		{
			/*-------------------------------
			| Enter screen 2 linear input . |
			-------------------------------*/
			heading (2);
			entry (2);
			if (restart)
				continue;

			/*-------------------------------
			| Enter screen 3 linear input . |
			-------------------------------*/
			heading (3);
			entry (3);
			if (restart)
				continue;

			/*-------------------------------
			| Enter screen 4 linear input . |
			-------------------------------*/
			heading (4);
			entry (4);
			if (restart)
				continue;
   
			if (teleSalesMaint)
			{
				/*-------------------------------
				| Enter screen 5 linear input . |
				-------------------------------*/
				init_vars (5);
				heading (5);
				entry (5);
				if (restart)
					continue;
			}
		}
		else 
		{
			FindMisc ();
			scn_display (1);
		}
		
		if (!teleSalesMaint)
			no_edit (5);			

		edit_all ();
		GetAccount ();
		if (restart)
			continue;

		 /*-------------------------------------------
        | re-edit if  currency of GL Account is incorrect. |
        -------------------------------------------*/
        while (checkError)
        {
            edit_all ();
            if (restart)
                break;

            if (prog_exit)
            {
                prog_exit = 0;
                continue;
            }

           GetAccount ();
        }

		/*-----------------------
		| Update customers record. |
		-----------------------*/
		if (!restart)
			Update ();

		if (TS_CUST)
			break;
	}

	/*=========================
	| Program exit sequence	. |
	=========================*/
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, 
							 (envVar.dbFind) ? "cumr_id_no3" : "cumr_id_no");
	
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, 
							 (envVar.crFind) ? "sumr_id_no" : "sumr_id_no3");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exct, exct_list, EXCT_NO_FIELDS, "exct_id_no");
	open_rec (exmm, exmm_list, EXMM_NO_FIELDS, "exmm_id_no");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (exdf, exdf_list, EXDF_NO_FIELDS, "exdf_id_no");
	open_rec (dbry, dbry_list, DBRY_NO_FIELDS, "dbry_id_no");
	open_rec (cubd, cubd_list, CUBD_NO_FIELDS, "cubd_id_no");
	open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");

	OpenPocr ();
	OpenGlmr ();

	if (envVar.PosInstalled)
		open_rec (posdtup, posdtup_list, POSDTUP_NO_FIELDS, "pos_no1");
	if (teleSalesMaint)
	{
		open_rec (tspm, tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
		open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");
	}
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("CustomerMasterFile.txt");
	
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (dbry);
	abc_fclose (cubd);
	abc_fclose (cudp);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (sumr);
	abc_fclose (esmr);
	abc_fclose (excl);
	abc_fclose (exmm);
	abc_fclose (exsf);
	abc_fclose (exsi);
	abc_fclose (exaf);
	abc_fclose (exct);
	abc_fclose (exdf);
	abc_fclose (pocf);
	abc_fclose (trzm);
	if (envVar.PosInstalled)
		abc_fclose (posdtup);

	GL_Close ();

	if (teleSalesMaint)
	{
		abc_fclose (tspm);
		abc_fclose (tmpf);
	}
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

void
ReadCumd (void)
{
	open_rec (cumd,cumd_list,CUMD_NO_FIELDS,"cumd_id_no");

	strcpy (cumd_rec.co_no,comm_rec.co_no);
	strcpy (cumd_rec.est_no,comm_rec.est_no);
	cc = find_rec (cumd, &cumd_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumd_rec.co_no,comm_rec.co_no);
		strcpy (cumd_rec.est_no, " 0");
		cc = find_rec (cumd, &cumd_rec, COMPARISON, "r");
	}
	NO_DEFAULT = (cc) ? TRUE : FALSE;

	abc_fclose (cumd);
}

int
spec_valid (
 int                field)
{
	int		temp = 0;
	int		i;
	int		val_pterms;
	char	work [10];

	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("dbtrno"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (pad_num (local_rec.cust_no), "000000"))
		{
			print_mess (ML ("000000 is not a valid customer number."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
			
	/*----------------------------------------------------------
	| Validate Establishment number (only if entered) i.e. > 0 |
	----------------------------------------------------------*/
	if (LCHECK ("estno"))
	{
		if (TS_CUST && prog_status == ENTRY && last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (!envVar.dbCo)
		{
			strcpy (cumr_rec.est_no, " 0");
			strcpy (cumr2_rec.est_no, " 0");
			DSP_FLD ("estno");
		}
		else
		{
		    if (atoi (cumr_rec.est_no) != 0) 
		    {
				strcpy (esmr_rec.co_no,comm_rec.co_no);
				strcpy (esmr_rec.est_no,cumr_rec.est_no);
				cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
				if (cc)
				{
					errmess (ML (mlStdMess073));
					sleep (sleepTime);
					return (EXIT_FAILURE); 
				}
				strcpy (cumr_rec.est_no,esmr_rec.est_no);
				strcpy (cumr2_rec.est_no,esmr_rec.est_no);
				strcpy (branchNumber, esmr_rec.est_no);
		    }
		    else 
		    {
				errmess (ML (mlDbMess224));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
		    }
		}

		if (prog_status == ENTRY)
		{
			strcpy (cumr_rec.co_no,comm_rec.co_no);
			strcpy (cumr2_rec.co_no,comm_rec.co_no);
			strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
			newCustomer = find_rec (cumr, &cumr_rec, COMPARISON, "w");
			if (!newCustomer)
			{
				entry_exit = 1;
				strcpy (prev_stop, cumr_rec.stop_credit);
				/*
			     * Save old customer record.
			 	 */
			 	SetAuditOldRec (&cumr_rec, sizeof (cumr_rec));
			}
 			strcpy (local_rec.mail_lbls, cumr_rec.mail_label);
 			strcpy (local_rec.cust_lett, cumr_rec.letter);
	
			strcpy (cudp_rec.co_no,comm_rec.co_no);
			strcpy (cudp_rec.br_no,comm_rec.est_no);
			strcpy (cudp_rec.dp_no,cumr_rec.department);
			cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (cudp_rec.dp_no, "  ");
				sprintf (cudp_rec.dp_name, "%40.40s", " ");
			}
			strcpy (exdf_rec.co_no,comm_rec.co_no);
			strcpy (exdf_rec.code,cumr_rec.disc_code);
			cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");
			if (cc)
	      			exdf_rec.disc_pc = 0.00;

			strcpy (exmm_rec.co_no,comm_rec.co_no);
			exmm_rec.exmm_hash = cumr_rec.merchant;
			cc = find_rec (exmm, &exmm_rec, COMPARISON, "r");
			if (cc)
			{
				exmm_rec.exmm_hash = 0L;
				strcpy (local_rec.exmm_name, " ");
			}
			else
				sprintf (local_rec.exmm_name, "%-40.40s", exmm_rec.name);

			strcpy (trzm_rec.co_no,comm_rec.co_no);
			strcpy (trzm_rec.br_no,cumr_rec.est_no);
			strcpy (trzm_rec.del_zone,cumr_rec.del_zone);
			cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (cumr_rec.del_zone, "      ");
				strcpy (trzm_rec.desc, " ");
			}

			if (cumr_rec.hhsu_hash != 0L)
			{
				abc_selfield (sumr, "sumr_hhsu_hash");
				sumr_rec.hhsu_hash = cumr_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
				if (!cc)
					strcpy (local_rec.supp_no, sumr_rec.crd_no);
				else
				{
					strcpy (local_rec.supp_no, "      ");
					cumr_rec.hhsu_hash = 0L;
				}
				abc_selfield (sumr, (envVar.crFind) ? "sumr_id_no" : "sumr_id_no3");
			}
            for (i = 1; i <= 3; i++)
            {
                strcpy (exsi_rec.co_no, comm_rec.co_no);
                exsi_rec.inst_code = cumr_inst_fg [i - 1];
                cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
                if (cc)
                    strcpy (local_rec.spindesc [i - 1], " ");
                else
                    sprintf (local_rec.spindesc [i - 1], "%-47.47s", exsi_rec.inst_text);	
            }
		}
		DisplayBranch ();
		strcpy (glmrRec.acc_no,cumr_rec.gl_ctrl_acct);
		strcpy (glmrRec.co_no,comm_rec.co_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("depart"))
	{
		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,comm_rec.est_no);
		strcpy (cudp_rec.dp_no,cumr_rec.department);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess084));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DisplayBranch ();
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate customer type and allow search. |
	------------------------------------------*/
	if (LCHECK ("cus_type"))
	{
		if (dflt_used)
		{
			strcpy (excl_rec.co_no,comm_rec.co_no);
			strcpy (excl_rec.class_type,"  ");
			cc = find_rec (excl, &excl_rec, GTEQ, "r");
			if (!cc && !strcmp (excl_rec.co_no,comm_rec.co_no))
			      	strcpy (cumr_rec.class_type,excl_rec.class_type);
		}
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excl_rec.co_no,comm_rec.co_no);
		strcpy (excl_rec.class_type,cumr_rec.class_type);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("type_name");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate stop credit. |
	-----------------------*/
	if (LCHECK ("stopcrd"))
	{
		if (F_NOKEY (field) && INPUT)
			strcpy (cumr_rec.stop_credit,cumd_rec.stop_credit);

		if (cumr_rec.stop_credit [0] == 'Y')
		{
			sprintf (err_str,ML (mlDbMess211), cumr_rec.payment_flag);
			print_mess (err_str);
			sleep (sleepTime);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate price type. |
	----------------------*/
	if (LCHECK ("pri_type"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			sprintf (local_rec.pri_show,"%-29.29s", cumd_rec.price_type);
			DSP_FLD ("pri_type");
		}

		cc = PriceDesc ();
		if (cc)
		{
			print_mess (ML (mlDbMess223));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pri_type_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate specific item codes. |
	-------------------------------*/
	if (LCHECK ("spec_item_flag"))
	{
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate account type. |
	------------------------*/
	if (LCHECK ("acctype"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.acc_type, cumd_rec.acc_type);
			DSP_FLD ("acctype");
		}
		if (cumr_rec.acc_type [0] == 'B')
			strcpy (local_rec.bf_dbt, ML ("(Balance B/F customers)"));
		else
			strcpy (local_rec.bf_dbt, ML ("(Open item customers)"));
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate statment type. |
	-------------------------*/
	if (LCHECK ("stmt_type"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.stmt_type, cumd_rec.stmt_type);
			DSP_FLD ("stmt_type");
		}

		if (cumr_rec.stmt_type [0] == 'B')
			strcpy (local_rec.bf_stm, ML ("(Balance B/F statement)"));
		else
			strcpy (local_rec.bf_stm, ML ("(Open item statement)"));
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate statment flag. |
	-------------------------*/
	if (LCHECK ("stmnt_flg"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.stmnt_flg, cumd_rec.stmnt_flg);
			DSP_FLD ("stmnt_flg");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chaddr2"))
	{
		if (dflt_used && INPUT)
			skip_entry = goto_field (field, label ("dladdr1"));

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dladdr2"))
	{
		if (dflt_used && INPUT)
			skip_entry = goto_field (field, label ("crd_prd"));

		return (EXIT_SUCCESS);
	}

	/*------------------------------------------------------------------
	| Check first line of Delivery address for 'same' (means charge to |
	| address and delivery address are the same)                       |
	------------------------------------------------------------------*/
	if (LCHECK ("dladdr1"))
	{
		if (!strncmp (cumr_rec.dl_adr1,"same",4) || 
		     !strncmp (cumr_rec.dl_adr1,"SAME",4) ||
		     dflt_used) 
		{
			strcpy (cumr_rec.dl_adr1,cumr_rec.ch_adr1);
			strcpy (cumr_rec.dl_adr2,cumr_rec.ch_adr2);
			strcpy (cumr_rec.dl_adr3,cumr_rec.ch_adr3);
			strcpy (cumr_rec.dl_adr4,cumr_rec.ch_adr4);
	
			DSP_FLD ("dladdr1");
			DSP_FLD ("dladdr2");
			DSP_FLD ("dladdr3");
			DSP_FLD ("dladdr4");
			entry_exit = 1;
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate customer acronym. |
	----------------------------*/
	if (LCHECK ("acronym"))
	{
		strcpy (cumr2_rec.dbt_acronym, cumr_rec.dbt_acronym);
		cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
		if (cc == 0 && strcmp (cumr2_rec.dbt_no, cumr_rec.dbt_no))
		{
			errmess (ML (mlStdMess181));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!strcmp (cumr_rec.dbt_acronym,"         "))
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Cash Flag. |
	---------------------*/
	if (LCHECK ("cash_flag"))
	{
		if (cumr_rec.cash_flag [0] == 'Y')
			strcpy (cumr_rec.crd_prd, "0  ");

		DSP_FLD ("crd_prd");
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Credit Period . |
	--------------------------*/
	if (LCHECK ("crd_prd"))
	{
		if (cumr_rec.cash_flag [0] == 'Y')
			strcpy (cumr_rec.crd_prd, "0  ");
		else if (F_NOKEY (field) && INPUT)
		{
		 	strcpy (cumr_rec.crd_prd, cumd_rec.crd_prd);
			DSP_FLD ("crd_prd");
		}
			
		if (SRCH_KEY)
		{
			SrchPay ();
			return (EXIT_SUCCESS);
		}
		val_pterms = FALSE;

		/*-------------------------------------
		| Check for format NNA to NNF input . |
		-------------------------------------*/
		if (cumr_rec.crd_prd [2] >= 'A')
		{
			temp = atoi (cumr_rec.crd_prd);
			if (temp < 1 || temp > 30) 
			{ 
				errmess (ML (mlStdMess189));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else   
			{	
				sprintf (cumr_rec.crd_prd,"%02d%c",temp, cumr_rec.crd_prd [2]);
				return (EXIT_SUCCESS);
			}
		}
		/*------------------------------------
		| Check for straight numeric input . |
		------------------------------------*/
		else 
		{
			temp = atoi (cumr_rec.crd_prd);
			if (temp < 0 || temp > 999) 
			{
				errmess (ML (mlStdMess182));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cumr_rec.crd_prd,
				p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (cumr_rec.crd_prd,"%-3.3s", p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("crd_prd");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate salesman code and allow search. |
	------------------------------------------*/
	if (LCHECK ("sman"))
	{
		if (dflt_used)
		{
			strcpy (exsf_rec.co_no,comm_rec.co_no);
			strcpy (exsf_rec.salesman_no,"  ");
			cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
			if (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no))
			      strcpy (cumr_rec.sman_code,exsf_rec.salesman_no);
		}
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cumr_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("sal_name");
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------
	| Validate area code and allow search. |
	--------------------------------------*/
	if (LCHECK ("area"))
	{
		if (dflt_used)
		{
			strcpy (exaf_rec.co_no,comm_rec.co_no);
			strcpy (exaf_rec.area_code,"  ");
			cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
			if (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no))
			      	strcpy (cumr_rec.area_code,exaf_rec.area_code);
		}
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,cumr_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("area_name");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------
	| Validate Merchandiser code and allow search. |
	----------------------------------------------*/
	if (LCHECK ("merchant"))
	{
		if (dflt_used)
		{
			cumr_rec.merchant = 0L;
			sprintf (local_rec.exmm_name, "%40.40s", " ");
			DSP_FLD ("mer_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExmm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exmm_rec.co_no, comm_rec.co_no);
		exmm_rec.exmm_hash = cumr_rec.merchant;
		cc = find_rec (exmm, &exmm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess ("Merchandiser is not on file.");
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.exmm_name, "%-40.40s", exmm_rec.name);
		DSP_FLD ("mer_name");
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Delivery Zone. |
	-------------------------*/
	if (LCHECK ("deliveryZone"))
	{
		if (dflt_used)
		{
			strcpy (cumr_rec.del_zone, "      ");
			sprintf (trzm_rec.desc, "%40.40s", " ");
			DSP_FLD ("zoneDescription");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, (envVar.dbCo) ? cumr_rec.est_no : comm_rec.est_no);
		strcpy (trzm_rec.del_zone, cumr_rec.del_zone);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("zoneDescription");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate Customer Royalty Class  Search. |
	------------------------------------------*/
	if (LCHECK ("cus_roy"))
	{
		
		if (SRCH_KEY)
		{
			SrchDbry (temp_str);
			return (EXIT_SUCCESS);
		}
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (dbry_rec.cr_type, cumd_rec.roy_type);
		}
		
		strcpy (dbry_rec.co_no,comm_rec.co_no);

		DSP_FLD ("cus_roy");
			
		cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");

		if (cc)
		{
			errmess (ML (mlStdMess187));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("roy_desc");
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Contract Type. |
	-------------------------*/
	if (LCHECK ("cont_type"))
	{
		if (dflt_used)
		{
			strcpy (exct_rec.co_no,comm_rec.co_no);
			strcpy (exct_rec.cont_type,"  ");
			cc = find_rec (exct, &exct_rec, GTEQ, "r");
			if (!cc && !strcmp (exct_rec.co_no,comm_rec.co_no))
			      	strcpy (cumr_rec.cont_type,exct_rec.cont_type);
		}
		if (SRCH_KEY)
		{
			SrchExct (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (exct_rec.co_no,comm_rec.co_no);
		strcpy (exct_rec.cont_type,cumr_rec.cont_type);
		cc = find_rec (exct, &exct_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess227));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("cont_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate Customer Royalty Class  Search. |
	------------------------------------------*/
	if (LCHECK ("bank_branch"))
	{
		if (SRCH_KEY)
		{
			SrchCubd (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (cubd_rec.co_no,comm_rec.co_no);
		sprintf (cubd_rec.bank_code, "%-3.3s", local_rec.bank);
		sprintf (cubd_rec.branch_code,"%-20.20s",local_rec.bank + 4);
		cc = find_rec (cubd, &cubd_rec, COMPARISON, "r");
		if (cc)
		{
			i = prmptmsg (ML (mlStdMess217),"YyNn", 1,20);
		
			move (0,20);
			line (79);
			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);

			if (AddCubd ())
			{
				errmess (ML (mlDbMess213));	
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (cubd_rec.co_no,comm_rec.co_no);
			sprintf (cubd_rec.bank_code,"%-3.3s",local_rec.bank);
			sprintf (cubd_rec.branch_code,"%-20.20s",
							local_rec.bank + 4);
		}
		sprintf (local_rec.bank, "%-3.3s-%-20.20s", 
				cubd_rec.bank_code,
				cubd_rec.branch_code);

		DSP_FLD ("bank_branch");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate Discount code and allow search. |
	------------------------------------------*/
	if (LCHECK ("discode"))
	{
		if (SRCH_KEY)
		{
			SrchExdf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exdf_rec.co_no,comm_rec.co_no);

		if (dflt_used)
		{
			strcpy (cumr_rec.disc_code, cumd_rec.disc_code);		
			strcpy (exdf_rec.code, cumd_rec.disc_code);		
		}
		else
			strcpy (exdf_rec.code, cumr_rec.disc_code);

		cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");

		if (cc)
		{
			errmess (ML (mlStdMess188));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("discpc");
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Credit Overide. |
	--------------------------*/
	if (LCHECK ("crd_override"))
	{
		if (FLD ("crd_overide") == ND)
			strcpy (cumr_rec.crd_flag, "N");

		return (EXIT_SUCCESS);
	}


	if (LCHECK ("cont_pos") || LCHECK ("alt_pos"))
	{
		if (dflt_used)
		{
			if (LCHECK ("alt_pos") && 
			    strlen (clip (local_rec.alt_cont)) == 0)
			{
				sprintf (local_rec.alt_pos, "%-3.3s", " ");
				sprintf (local_rec.alt_pos_desc, "%-40.40s"," ");
				DSP_FLD ("alt_pos_desc");
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmpf_rec.co_no,comm_rec.co_no);
		if (LCHECK ("cont_pos"))
			sprintf (tmpf_rec.pos_code, "%-3.3s", local_rec.cont_pos);
		else
			sprintf (tmpf_rec.pos_code, "%-3.3s", local_rec.alt_pos);
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess166));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("cont_pos"))
		{
			sprintf (local_rec.cont_pos_desc, "%-40.40s", tmpf_rec.pos_desc);	
			DSP_FLD ("cont_pos_desc");
		}
		else
		{
			sprintf (local_rec.alt_pos_desc, "%-40.40s", tmpf_rec.pos_desc);	
			DSP_FLD ("alt_pos_desc");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bst_ph_time"))
	{
		if (!ValidateTime (local_rec.bst_ph_time))
		{
			print_mess (ML (mlStdMess142));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nxt_ph_date"))
	{
		if (dflt_used)
		{
			local_rec.nxt_ph_date = 0L;
			return (EXIT_SUCCESS);
		}

		if (local_rec.nxt_ph_date < local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess229));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nxt_ph_time"))
	{
		if (!ValidateTime (local_rec.nxt_ph_time))
		{
			print_mess ("\007 Invalid Time ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nxt_vs_date"))
	{
		if (dflt_used)
		{
			local_rec.nxt_vs_date = 0L;
			return (EXIT_SUCCESS);
		}

		if (local_rec.nxt_vs_date < local_rec.lsystemDate)
		{
			print_mess ("\007 Date Must Be Greater Than Today ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nxt_vs_time"))
	{
		if (!ValidateTime (local_rec.nxt_vs_time))
		{
			print_mess ("\007 Invalid Time ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rec_mailer"))
	{
		if (local_rec.rec_mailer [0] == 'Y')
			strcpy (local_rec.rec_mailer_desc, "Yes");
		else
			strcpy (local_rec.rec_mailer_desc, "No ");

		DSP_FLD ("rec_mailer_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sales_per"))
	{
		if (local_rec.sales_per [0] == 'D')
			strcpy (local_rec.sales_per_desc, "Daily  ");
		else if (local_rec.sales_per [0] == 'W')
			strcpy (local_rec.sales_per_desc, "Weekly ");
		else
			strcpy (local_rec.sales_per_desc, "Monthly");

		DSP_FLD ("sales_per_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("b/oflag"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.bo_flag, cumd_rec.bo_flag);
			DSP_FLD ("b/oflag");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("b/cons"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.bo_cons, cumd_rec.bo_cons);
			DSP_FLD ("b/cons");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("b/days"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			cumr_rec.bo_days = cumd_rec.bo_days;
			DSP_FLD ("b/days");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("p/oflag"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.po_flag, cumd_rec.po_flag);
			DSP_FLD ("p/oflag");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sos"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.sur_flag, cumd_rec.sur_flag);
			DSP_FLD ("sos");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("freight_chg"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.freight_chg,cumd_rec.freight_chg);
			DSP_FLD ("freight_chg");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("restock_fee"))
	{
		if (F_NOKEY (field) && INPUT)
		{
			strcpy (cumr_rec.restock_fee,cumd_rec.restock_fee);
			DSP_FLD ("restock_fee");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nett_pri"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		      strcpy (cumr_rec.nett_pri_prt,cumd_rec.nett_pri_prt);
		      DSP_FLD ("nett_pri");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("taxcode"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (F_NOKEY (field) && INPUT)
		{
	      	strcpy (cumr_rec.tax_code,cumd_rec.tax_code);
			DSP_FLD ("taxcode");
		}
		return (EXIT_SUCCESS);
	}
   	/*-----------------------------
    | Validate Shipment method (s) |
    -----------------------------*/
    if (LCHECK ("spinst1") || LCHECK ("spinst2") || LCHECK ("spinst3"))
    {
        strcpy (work, FIELD.label);
        work [7] = '\0';

        i = atoi (&work [6]);

        if (dflt_used)
        {
            cumr_inst_fg [i - 1] = 0;
            return (EXIT_SUCCESS);
        }

        if (SRCH_KEY)
        {
            SrchExsi (temp_str);
            return (EXIT_SUCCESS);
        }
        strcpy (exsi_rec.co_no, comm_rec.co_no);
        exsi_rec.inst_code = cumr_inst_fg [i - 1];

        cc = find_rec (exsi, (char *) &exsi_rec, COMPARISON, "r");
        if (cc)
        {
			print_mess (ML (mlStdMess184));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
        sprintf (local_rec.spindesc [i - 1],"%-47.47s", exsi_rec.inst_text);
        DSP_FLD (work);
        sprintf (work, "spindesc%1d", i);
        DSP_FLD (work);
        return (EXIT_SUCCESS);
    }

	if (LCHECK ("reprint"))
	{
		if (F_NOKEY (field) && INPUT)
		{
		      strcpy (cumr_rec.reprint_inv, "N");
		      DSP_FLD ("reprint");
		}
		return (EXIT_SUCCESS);
	}
 	if (LCHECK ("mail_lbls"))
 	{
 		DSP_FLD ("mail_lbls");
 		return (EXIT_SUCCESS);
 	}
 
 	if (LCHECK ("cust_lett"))
 	{
 		DSP_FLD ("cust_lett");
 		return (EXIT_SUCCESS);
 	}

	/*-------------------------------
	| Validate Currency Code Input. |
	-------------------------------*/
	if (LCHECK ("curcode"))
	{
		if (F_NOKEY (field))
		{
			strcpy (cumr_rec.curr_code, envVar.currencyCode);
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}

		cc = FindPocr (comm_rec.co_no, cumr_rec.curr_code, "r");
		if (cc)
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Country Code Input. |
	------------------------------*/
	if (LCHECK ("ctrycode"))
	{
		if (F_NOKEY (field))
		{
			strcpy (cumr_rec.ctry_code, envVar.countryCode);
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchPocf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocf_rec.co_no, comm_rec.co_no);
		strcpy (pocf_rec.code, cumr_rec.ctry_code);
		cc = find_rec (pocf, &pocf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess118));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| Validate Control Account. |
	---------------------------*/
	if (LCHECK ("gl_acc"))
	{
		if (dflt_used)
		{
			/*-------------------------------------------
			| Interface gets passed :					|
			|		Company Number.						|
			|		Branch Number.						|
			|		Warehouse Number.					|
			|		Interface Code. (See gl_ariface C)	|
			|		Customer type						|
			|		Stock category.						|
			-------------------------------------------*/
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"ACCT REC",
				 (envVar.GlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
				" "
			);

			strcpy (cumr_rec.gl_ctrl_acct, GL_Account);
		}
 		if (F_NOKEY (field) && INPUT)
 			strcpy (cumr_rec.gl_ctrl_acct, cumd_rec.gl_ctrl_acct);

		if (SRCH_KEY)
			return SearchGlmr_C (comm_rec.co_no, temp_str, "F*P", cumr_rec.curr_code);

		GL_FormAccNo (cumr_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
		strcpy (glmrRec.co_no,comm_rec.co_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
		    print_err ("Account %s is not on file.", glmrRec.acc_no);
			sleep (sleepTime);
		    return (EXIT_FAILURE);
		}
				
		if (Check_class ())
			return (EXIT_FAILURE);

		if (strncmp (glmrRec.curr_code, cumr_rec.curr_code, 3))
		{
		    print_err ("Account %s currency is not equal to customers currency.", glmrRec.acc_no);

			sleep (sleepTime);
		    return (EXIT_FAILURE);
		}
		DSP_FLD ("gl_acc");
		DSP_FLD ("gl_acc_desc");
		DSP_FLD ("gl_acc_curr");

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("crd_no"))
	{
		if (dflt_used)
		{
			cumr_rec.hhsu_hash = 0L;
			return (EXIT_SUCCESS);
		}
			
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, cr_branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,cr_branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supp_no));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
	
		DSP_FLD ("crd_name");
		cumr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*---------------------
| Validate input time |
---------------------*/
int
ValidateTime (
 char*              time_str)
{
	char	tmp_time [6];
	char	*tptr;
	int		tmp_hour;
	int		tmp_min;
	int		i;

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
/*=======================================================================
| Routine to add Customer description of price type to price type field . |
=======================================================================*/
int
PriceDesc (void)
{
	int		junk;

	junk = atoi (mid (local_rec.pri_show,1,1));

	if (junk < 1 || junk > envVar.sk_dbprinum)
		return (EXIT_FAILURE);

	switch (junk)
	{
		case	1:	
			strcpy (local_rec.pri_show_desc,	comm_rec.price1_desc);
			break;
		case	2:	
			strcpy (local_rec.pri_show_desc, comm_rec.price2_desc);
			break;
		case	3:	
			strcpy (local_rec.pri_show_desc, comm_rec.price3_desc);
			break;
		case	4:	
			strcpy (local_rec.pri_show_desc, comm_rec.price4_desc);
			break;
		case	5:	
			strcpy (local_rec.pri_show_desc, comm_rec.price5_desc);
			break;
		case	6:	
			strcpy (local_rec.pri_show_desc, comm_rec.price6_desc);
			break;
		case	7:	
			strcpy (local_rec.pri_show_desc, comm_rec.price7_desc);
			break;
		case	8:	
			strcpy (local_rec.pri_show_desc, comm_rec.price8_desc);
			break;
		case	9:	
			strcpy (local_rec.pri_show_desc, comm_rec.price9_desc);
			break;
	}
	return (EXIT_SUCCESS);
}

int
AddCubd (void)
{
	strcpy (cubd_rec.co_no,comm_rec.co_no);
	sprintf (cubd_rec.bank_code, "%-3.3s", local_rec.bank);
	sprintf (cubd_rec.branch_code,"%-20.20s",local_rec.bank + 4);
	return (abc_add (cubd, &cubd_rec));
}

/*----------------
| Update Record. |
----------------*/
void
Update (void)
{
	clear ();

	open_rec (srdb, srdb_list, SRDB_NO_FIELDS, "srdb_hhcu_hash");

	strcpy (cumr_rec.stat_flag,   "0");
	strcpy (cumr_rec.department,   cudp_rec.dp_no);
	strcpy (cumr_rec.roy_type,     dbry_rec.cr_type);
	sprintf (cumr_rec.bank_code,   "%-3.3s",   local_rec.bank);
	sprintf (cumr_rec.branch_code, "%-20.20s", local_rec.bank + 4);
 	sprintf (cumr_rec.mail_label,  "%-1.1s",   local_rec.mail_lbls);
 	sprintf (cumr_rec.letter,      "%-1.1s",   local_rec.cust_lett);
 	strcpy (cumr_rec.gl_ctrl_acct, glmrRec.acc_no);  
	if (newCustomer == 1) 
	{
		cumr_rec.payment_flag  = 0;
		cumr_rec.od_flag       = 0;
		cumr_rec.total_days_sc = 0;
		cumr_rec.date_stop     = 0L;
		cumr_rec.date_lastinv  = 0L;
		cumr_rec.date_lastpay  = 0L;
		cumr_rec.ho_dbt_hash	  = 0L;
		cumr_rec.amt_lastpay   = 0.00;
		cumr_rec.mtd_sales     = 0.00;
		cumr_rec.ytd_sales     = 0.00;
		cumr_rec.ord_value     = 0.00;
		cumr_balance [0]         = 0.00;
		cumr_balance [1]         = 0.00;
		cumr_balance [2]         = 0.00;
		cumr_balance [3]         = 0.00;
		cumr_balance [4]         = 0.00;
		cumr_balance [5]         = 0.00;
		strcpy (cumr_rec.ch_to_ho_flg, "N");
		strcpy (cumr_rec.dbt_no,     pad_num (local_rec.cust_no));
		strcpy (cumr_rec.co_no,      comm_rec.co_no);
		strcpy (cumr_rec.price_type, mid (local_rec.pri_show, 1, 1));

		cumr_rec.date_open = local_rec.lsystemDate;

		cc = abc_add (cumr, &cumr_rec);
		if (cc) 
		{
			if (cc == DUPADD) 
			{
				errmess (ML (mlStdMess181));
				sleep (sleepTime);
				return;
			}
			file_err (cc, cumr, "DBADD");
		}
		/*-------------------------------------
		| Read back customer to get hhcu hash |
		-------------------------------------*/
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.cust_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

	}
	else 
	{
		/*--------------------------------------
		| Check if Customer is on stop credit. |
		--------------------------------------*/
		if (cumr_rec.stop_credit [0] == 'N')
		{
			if (cumr_rec.date_stop != 0L)
				cumr_rec.total_days_sc += (int) 
				 (comm_rec.dbt_date - cumr_rec.date_stop);

			cumr_rec.date_stop = 0L;
		}
		else
		{
			/*------------------------------------------------------
			| Customer is being put on stop credit for first time. |
			------------------------------------------------------*/
			if (prev_stop [0] == 'N')
				cumr_rec.date_stop = comm_rec.dbt_date;

			/*-----------------------------------------
			| Stop credit date 0L so needs to be set. |
			-----------------------------------------*/
			if (cumr_rec.date_stop == 0L)
				cumr_rec.date_stop = comm_rec.dbt_date;
			
		}
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %s (%s)", ML ("Customer"), cumr_rec.dbt_no, cumr_rec.dbt_name);
		 AuditFileAdd (err_str, &cumr_rec, cumr_list, CUMR_NO_FIELDS);

		strcpy (cumr_rec.price_type,mid (local_rec.pri_show,1,1));
		cc = abc_update (cumr,&cumr_rec);
		if (cc) 
			file_err (cc, cumr, "DBUPDATE");

	}
	/*-----------------------------------
	| Update search file for customers. |
	-----------------------------------*/
	Update_srdb ();	

	strcpy (local_rec.previousCustomer,local_rec.cust_no);

	if (teleSalesMaint)
	{
		sprintf (tspm_rec.cont_name1, "%-20.20s",cumr_rec.contact_name);
		sprintf (tspm_rec.cont_code1, "%-3.3s", local_rec.cont_pos);
		sprintf (tspm_rec.cont_name2, "%-20.20s",local_rec.alt_cont);
		sprintf (tspm_rec.cont_code2, "%-3.3s", local_rec.alt_pos);
		sprintf (tspm_rec.email1,     "%-60.60s", local_rec.email [0]);
		sprintf (tspm_rec.email2,     "%-60.60s", local_rec.email [1]);
		sprintf (tspm_rec.best_ph_time, "%-5.5s", local_rec.bst_ph_time);
		tspm_rec.phone_freq 	= local_rec.ph_freq;
		tspm_rec.n_phone_date 	= local_rec.nxt_ph_date;
		sprintf (tspm_rec.n_phone_time, "%-5.5s", local_rec.nxt_ph_time);
		tspm_rec.visit_freq 	= local_rec.vs_freq;
		tspm_rec.n_visit_date 	= local_rec.nxt_vs_date;
		sprintf (tspm_rec.n_visit_time, "%-5.5s", local_rec.nxt_vs_time);
		sprintf (tspm_rec.mail_flag, "%-1.1s", local_rec.rec_mailer);
		sprintf (tspm_rec.sales_per, "%-1.1s", local_rec.sales_per);

		if (newCustomer || (!newCustomer && !tsmpFind))
		{
			tspm_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cc = abc_add (tspm, &tspm_rec);
			if (cc)
				file_err (cc, tspm, "DBADD");
		}
		else
		{
			cc = abc_update (tspm, &tspm_rec);
			if (cc)
				file_err (cc, tspm, "DBUPDATE");
		}
	}
	abc_fclose (srdb);
	if (envVar.PosInstalled)
		UpdatePosdt ();	
}

/*==========================
| Search for Country Code. |
==========================*/
void
SrchPocf (
 char*              key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Country Code Description");
	strcpy (pocf_rec.co_no, comm_rec.co_no);
	strcpy (pocf_rec.code, key_val);
	cc = find_rec (pocf, &pocf_rec, GTEQ, "r");
	while (!cc && !strncmp (pocf_rec.code,key_val,strlen (key_val)) && 
		      !strcmp (pocf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocf_rec.code,pocf_rec.description);
		if (cc)
			break;
		cc = find_rec (pocf, &pocf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	strcpy (pocf_rec.code,temp_str);
	cc = find_rec (pocf, &pocf_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, pocf, "DBFIND");
}
/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchPay (void)
{
	int		i = 0;
	_work_open (3,0,50);
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

/*==========================================
| Search routine for Category Master File. |
==========================================*/
void
SrchExcl (
 char*              key_val)
{
	_work_open (3,0,40);
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,key_val);
	save_rec ("#No.","#Customer Type Description");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (excl_rec.class_type, key_val,strlen (key_val)) && 
		!strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}
/*======================================
| Search routine for Bank detail file. |
======================================*/
void
SrchCubd (
 char*              key_val)
{
	_work_open (25,0,2);
	strcpy (cubd_rec.co_no,comm_rec.co_no);
	sprintf (cubd_rec.bank_code,"%-3.3s", key_val);
	sprintf (cubd_rec.branch_code,"%-20.20s", key_val + 4);

	save_rec ("#Bank/Branch Code.","#");
	cc = find_rec (cubd, &cubd_rec, GTEQ, "r");
	while (!cc && !strncmp (cubd_rec.bank_code,key_val,strlen (key_val)) &&
		!strcmp (cubd_rec.co_no, comm_rec.co_no))
	{
		sprintf (err_str, "%-3.3s-%s", 
				cubd_rec.bank_code, cubd_rec.branch_code);
		cc = save_rec (err_str, " ");
		if (cc)
			break;

		cc = find_rec (cubd, &cubd_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cubd_rec.co_no,comm_rec.co_no);
	sprintf (cubd_rec.bank_code,"%-3.3s", temp_str);
	sprintf (cubd_rec.branch_code,"%-20.20s", temp_str + 4);
	cc = find_rec (cubd, &cubd_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cubd, "DBFIND");
}

/*==========================================
| Search routine for Salesman Master File. |
==========================================*/
void
SrchExsf (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,key_val);
	save_rec ("#No.","#Salesman Description");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strncmp (exsf_rec.salesman_no, key_val,strlen (key_val)) && 
		      !strcmp (exsf_rec.co_no, comm_rec.co_no))
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
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*============================================
| Search routine for Department Master File. |
============================================*/
void
SrchCudp (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
	strcpy (cudp_rec.dp_no,key_val);
	save_rec ("#Dp.","#Department Description");
	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && !strncmp (cudp_rec.dp_no, key_val,strlen (key_val)) && 
		      !strcmp (cudp_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cudp_rec.br_no,comm_rec.est_no))
	{
		cc = save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
		if (cc)	
			break;
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
	strcpy (cudp_rec.dp_no,temp_str);
	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cudp, "DBFIND");
}

/*======================================
| Search routine for Area Master File. |
======================================*/
void
SrchExaf (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,key_val);
	save_rec ("#No.","#Area Description");
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strncmp (exaf_rec.area_code,key_val,strlen (key_val)) &&
		      !strcmp (exaf_rec.co_no, comm_rec.co_no))
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

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}
/*======================================
| Search routine for Area Master File. |
======================================*/
void
SrchExct (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (exct_rec.co_no,comm_rec.co_no);
	strcpy (exct_rec.cont_type,key_val);
	save_rec ("#No.","#Area Description");
	cc = find_rec (exct, &exct_rec, GTEQ, "r");
	while (!cc && !strncmp (exct_rec.cont_type,key_val,strlen (key_val)) &&
		      !strcmp (exct_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exct_rec.cont_type, exct_rec.cont_desc);
		if (cc)
			break;
		cc = find_rec (exct, &exct_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exct_rec.co_no,comm_rec.co_no);
	strcpy (exct_rec.cont_type,temp_str);
	cc = find_rec (exct, &exct_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exct, "DBFIND");
}

void
SrchDbry (
 char*              key_val)
{
	_work_open (3,0,40);
	save_rec ("#Roy","#Royalty Description ");
	strcpy (dbry_rec.co_no,comm_rec.co_no);
	strcpy (dbry_rec.cr_type,key_val);
	cc = find_rec (dbry, &dbry_rec, GTEQ, "r");
	while (!cc && !strcmp (dbry_rec.co_no,comm_rec.co_no) && 
		      !strncmp (dbry_rec.cr_type,key_val,strlen (key_val)))
	{
		cc = save_rec (dbry_rec.cr_type,dbry_rec.desc);
		if (cc)
			break;
		cc = find_rec (dbry, &dbry_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (dbry_rec.co_no,comm_rec.co_no);
	sprintf (dbry_rec.cr_type,"%-3.3s",temp_str);
	cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dbry, "DBFIND");
}

/*==========================================
| Search routine for Discount Master File. |
==========================================*/
void
SrchExdf (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (exdf_rec.co_no,comm_rec.co_no);
	strcpy (exdf_rec.code,key_val);
	save_rec ("#No.","#Discount %");
	cc = find_rec (exdf, &exdf_rec, GTEQ, "r");
	while (!cc && !strncmp (exdf_rec.code, key_val,strlen (key_val)) && 
	              !strcmp (exdf_rec.co_no, comm_rec.co_no))
	{
		sprintf (err_str, ML (mlDbMess212), exdf_rec.disc_pc);
		cc = save_rec (exdf_rec.code, err_str);
		if (cc)
			break;
		cc = find_rec (exdf, &exdf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exdf_rec.co_no,comm_rec.co_no);
	strcpy (exdf_rec.code,temp_str);
	cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");
}

/*======================
| Search Merchandiser. |
======================*/
void
SrchExmm (
 char*              key_val)
{
	_work_open (6,0,40);
	strcpy (exmm_rec.co_no,comm_rec.co_no);
	exmm_rec.exmm_hash = atol (key_val);
	save_rec ("#Number ","#Merchandiser Name.");
	cc = find_rec (exmm, &exmm_rec, GTEQ, "r");
	while (!cc && !strcmp (exmm_rec.co_no, comm_rec.co_no))
	{
		sprintf (err_str, "%06ld", exmm_rec.exmm_hash);
		cc = save_rec (err_str, exmm_rec.name);
		if (cc)
			break;
		cc = find_rec (exmm, &exmm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exmm_rec.co_no,comm_rec.co_no);
	exmm_rec.exmm_hash = atol (temp_str);
	cc = find_rec (exmm, &exmm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exmm, "DBFIND");
}

/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char*              key_val)
{
	_work_open (6,0,50);

	save_rec ("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, (envVar.dbCo) ? cumr_rec.est_no : comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, (envVar.dbCo) ? cumr_rec.est_no : comm_rec.est_no) &&
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
	strcpy (trzm_rec.br_no, (envVar.dbCo) ? cumr_rec.est_no : comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");

	return;
}

void
SrchTmpf (
 char*              key_val)
{   
	_work_open (3,0,40);
	save_rec ("#Cde","#Position Description");
	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	sprintf (tmpf_rec.pos_code,"%-3.3s",key_val);
	cc = find_rec (tmpf,&tmpf_rec,GTEQ,"r");

	while (!cc && !strcmp (tmpf_rec.co_no,comm_rec.co_no) &&
		      !strncmp (tmpf_rec.pos_code,key_val,strlen (key_val)))
	{
		cc = save_rec (tmpf_rec.pos_code,tmpf_rec.pos_desc);
		if (cc)
			break;

		cc = find_rec (tmpf,&tmpf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	sprintf (tmpf_rec.pos_code,"%-3.3s",temp_str);
	cc = find_rec (tmpf,&tmpf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "tmpf", "DBFIND");
}

void
FindMisc (void)
{
	strcpy (local_rec.pri_show,cumr_rec.price_type);
	PriceDesc ();

	if (cumr_rec.acc_type [0] == 'B')
		strcpy (local_rec.bf_dbt, "(Balance B/F customers)");
	else
		strcpy (local_rec.bf_dbt, "(Open item customers)");

	if (cumr_rec.stmt_type [0] == 'B')
		strcpy (local_rec.bf_stm, "(Balance B/F statement)");
	else
		strcpy (local_rec.bf_stm, "(Open item statement)");

	if (cumr_rec.item_codes [0] != 'Y' &&
		 cumr_rec.item_codes [0] != 'N')
		strcpy (cumr_rec.item_codes, "N");

	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,cumr_rec.class_type);
	if (find_rec (excl, &excl_rec, COMPARISON, "r"))
		strcpy (excl_rec.class_type, "   ");

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,cumr_rec.sman_code);
	if (find_rec (exsf, &exsf_rec, COMPARISON, "r"))
		strcpy (exsf_rec.salesman_no,"  ");

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,cumr_rec.area_code);
	if (find_rec (exaf, &exaf_rec, COMPARISON, "r"))
		strcpy (exaf_rec.area_code,"  ");

	strcpy (exct_rec.co_no,comm_rec.co_no);
	strcpy (exct_rec.cont_type,cumr_rec.cont_type);
	if (find_rec (exct, &exct_rec, COMPARISON, "r"))
		strcpy (exct_rec.cont_type,"   ");

	strcpy (dbry_rec.co_no,comm_rec.co_no);
	strcpy (dbry_rec.cr_type,cumr_rec.roy_type);
	if (find_rec (dbry, &dbry_rec, COMPARISON, "r"))
		strcpy (dbry_rec.cr_type, "   ");

	strcpy (cubd_rec.co_no,comm_rec.co_no);
	strcpy (cubd_rec.bank_code,cumr_rec.bank_code);
	strcpy (cubd_rec.branch_code,cumr_rec.branch_code);
	if (find_rec (cubd, &cubd_rec, COMPARISON, "r"))
	{
		strcpy (cubd_rec.bank_code,"   ");
		strcpy (cubd_rec.branch_code,"                    ");
	}
	sprintf (local_rec.bank, "%-3.3s-%-20.20s", cubd_rec.bank_code,
						    cubd_rec.branch_code);

	if (teleSalesMaint)
		FindTspm ();
}

/*--------------------------
| Find customers tspm record |
--------------------------*/
void
FindTspm (void)
{
	tspm_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
	if (cc)
	{
		tsmpFind = FALSE;
		/*----------------
		| Setup defaults |
		----------------*/
		tspm_rec.date_create = local_rec.lsystemDate;
		tspm_rec.lphone_date = 0L;
		sprintf (tspm_rec.op_code, "%-14.14s", " ");
		sprintf (tspm_rec.lst_op_code, "%-14.14s", " ");
		sprintf (tspm_rec.cont_name1, "%-20.20s", cumr_rec.contact_name);
		sprintf (local_rec.email [0], "%60.60s", " ");
		sprintf (local_rec.email [1], "%60.60s", " ");
		sprintf (local_rec.cont_pos, "%-3.3s", " ");
		sprintf (local_rec.cont_pos_desc, "%-40.40s", " ");
		sprintf (local_rec.alt_cont, "%-20.20s", " ");
		sprintf (local_rec.alt_pos, "%-3.3s", " ");
		sprintf (local_rec.alt_pos_desc, "%-40.40s", " ");
		strcpy (local_rec.bst_ph_time, "00:00");
		local_rec.ph_freq = 0;
		local_rec.nxt_ph_date = 0L;
		strcpy (local_rec.nxt_ph_time, "00:00");
		local_rec.vs_freq = 0;
		local_rec.nxt_vs_date = 0L;
		strcpy (local_rec.nxt_vs_time, "00:00");
		strcpy (local_rec.rec_mailer, "N");
		strcpy (local_rec.rec_mailer_desc, "No ");
		strcpy (local_rec.sales_per, "M");
		strcpy (local_rec.sales_per_desc, "Monthly");
	}
	else
	{
		tsmpFind = TRUE;

		sprintf (local_rec.cont_pos, "%-3.3s", tspm_rec.cont_code1);
		sprintf (local_rec.alt_cont,"%-20.20s",tspm_rec.cont_name2);
		sprintf (local_rec.alt_pos, "%-3.3s", tspm_rec.cont_code2);
		sprintf (local_rec.email [0], tspm_rec.email1);
		sprintf (local_rec.email [1], tspm_rec.email2);
		sprintf (local_rec.bst_ph_time, "%-5.5s", tspm_rec.best_ph_time);
		local_rec.ph_freq = tspm_rec.phone_freq;
		local_rec.nxt_ph_date = tspm_rec.n_phone_date;
		sprintf (local_rec.nxt_ph_time, "%-5.5s", tspm_rec.n_phone_time);
		local_rec.vs_freq = tspm_rec.visit_freq;
		local_rec.nxt_vs_date = tspm_rec.n_visit_date;
		sprintf (local_rec.nxt_vs_time, "%-5.5s", tspm_rec.n_visit_time);
		strcpy (local_rec.rec_mailer, tspm_rec.mail_flag);
		strcpy (local_rec.rec_mailer_desc, tspm_rec.mail_flag [0] == 'Y' ? "Yes" : "No ");

		switch (tspm_rec.sales_per [0])
		{
		case 'D':
			strcpy (local_rec.sales_per, "D");
			strcpy (local_rec.sales_per_desc, "Daily  ");
			break;

		case 'W':
			strcpy (local_rec.sales_per, "W");
			strcpy (local_rec.sales_per_desc, "Weekly ");
			break;

		case 'M':
		default:
			strcpy (local_rec.sales_per, "M");
			strcpy (local_rec.sales_per_desc, "Monthly");
			break;
		}

		/*----------------------------
		| Lookup position of contact |
		----------------------------*/
		strcpy (tmpf_rec.co_no, comm_rec.co_no);
		sprintf (tmpf_rec.pos_code, "%-3.3s", local_rec.cont_pos);
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
		if (cc)
		{
			if (strlen (clip (local_rec.cont_pos)) == 0)
				sprintf (local_rec.cont_pos_desc,"%-40.40s"," ");
			else
				sprintf (local_rec.cont_pos_desc,"%-40.40s", "Unknown Position");
		}
		else
			sprintf (local_rec.cont_pos_desc, "%-40.40s", tmpf_rec.pos_desc);
		
		/*--------------------------------------
		| Lookup position of alternate contact |
		--------------------------------------*/
		strcpy (tmpf_rec.co_no, comm_rec.co_no);
		sprintf (tmpf_rec.pos_code, "%-3.3s", local_rec.alt_pos);
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
		if (cc)
		{
			if (strlen (clip (local_rec.alt_pos)) == 0)
				sprintf (local_rec.alt_pos_desc, "%-40.40s"," ");
			else
				sprintf (local_rec.alt_pos_desc, "%-40.40s", "Unknown Position");
		}
		else
			sprintf (local_rec.alt_pos_desc, "%-40.40s", tmpf_rec.pos_desc);
	}
}

/*======================================================
| If field cannot be input then leave, else set to NI. |
======================================================*/
void
SET_FLD (
 char*          fld_name)
{
	/*---------------------------------------------------------------
	| If Not applicable (NA) or No Input (NI) or No Display (ND) or |
    | if required (YES) then leave as is.                          |
	---------------------------------------------------------------*/
	if (F_NOKEY (label (fld_name)) || FLD (fld_name) == YES)
		return;

	FLD (fld_name) = NI;
}

int
heading (
 int                scn)
{
	if (restart)
	{
		abc_unlock (cumr);
		return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	pr_box_lines (scn);

	strcpy (err_str,ML ("Customer Master Maintenance"));
	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);
	print_at (0,58,ML (mlDbMess188),local_rec.previousCustomer);

	if (scn != 1)
	{
		move (0,1);
		line (80);
	}

	strcpy (err_str,ML (mlStdMess038));
	print_at (22,0,err_str,comm_rec.co_no,comm_rec.co_name);
	DisplayBranch ();
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	
	if (scn == 5 && newCustomer)
	{
		tspm_rec.date_create = local_rec.lsystemDate;
		tspm_rec.lphone_date = 0L;
		sprintf (tspm_rec.op_code, "%-14.14s", " ");
		sprintf (tspm_rec.lst_op_code,"%-14.14s"," ");

		DSP_FLD ("dt_create");
		DSP_FLD ("cont_name");
	}

    return (EXIT_SUCCESS);
}

void
DisplayBranch (void)
{
	if (!strcmp (branchNumber, " 0"))
	{
		print_at (23,0, "Br = 0 %s / Dp: %s %-30.30s ",
			   ML ("(Company Owned Customer)"),
			   cudp_rec.dp_no,
			   cudp_rec.dp_name);
	
	}
	else
	{
		print_at (23,0, "Br: %s %-30.30s / Dp: %s %-30.30s ",
			   esmr_rec.est_no,
			   esmr_rec.short_name,
			   cudp_rec.dp_no,
			   cudp_rec.dp_name);
	}
}

/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char*              key_val)
{
    char    wk_code [4];

	_work_open (3,0,60);
    save_rec ("#Spec Inst", "#Instruction description.");

    strcpy (exsi_rec.co_no, comm_rec.co_no);
    exsi_rec.inst_code = atoi (key_val);

    cc = find_rec (exsi, &exsi_rec, GTEQ, "r");
    while (!cc && !strcmp (exsi_rec.co_no, comm_rec.co_no))
    {
        sprintf (wk_code, "%03d", exsi_rec.inst_code);
        sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
        cc = save_rec (wk_code, err_str);
        if (cc)
            break;
        cc = find_rec (exsi, &exsi_rec, NEXT, "r");
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

    strcpy (exsi_rec.co_no, comm_rec.co_no);
    exsi_rec.inst_code = atoi (temp_str);
    if ( (cc = find_rec (exsi, &exsi_rec, COMPARISON, "r")))
        file_err (cc, exsi, "DBFIND");
}

int
Check_class (void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
	    print_err ("Account %s is not a posting account.",
						glmrRec.acc_no);
		sleep (sleepTime);
	    return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Update search file for customers. |
===================================*/
void
Update_srdb (void)
{
	int		Newsrdb;
	long	hhcu;

	abc_selfield (srdb,"srdb_hhcu_hash");

	hhcu = cumr_rec.hhcu_hash;
	
	srdb_rec.hhcu_hash = cumr_rec.hhcu_hash;
	Newsrdb = find_rec (srdb, &srdb_rec, 	EQUAL ,"u");

	
	strcpy (err_str, cumr_rec.dbt_name);
	strcpy (srdb_rec.co_no, 		cumr_rec.co_no);
	strcpy (srdb_rec.br_no,			cumr_rec.est_no);
	strcpy (srdb_rec.dbt_no,		cumr_rec.dbt_no);
	srdb_rec.hhcu_hash =			cumr_rec.hhcu_hash;
	strcpy (srdb_rec.acronym,		cumr_rec.dbt_acronym);
	strcpy (srdb_rec.sman_code,		cumr_rec.sman_code);
	strcpy (srdb_rec.contact_name,	cumr_rec.contact_name);
	strcpy (srdb_rec.name,			upshift (err_str));

	if (Newsrdb)
	{
		cc = abc_add (srdb, &srdb_rec);
		if (cc)
			file_err (cc, srdb, "DBADD");
	}
	else
	{
		cc = abc_update (srdb, &srdb_rec);
		if (cc)
			file_err (cc, srdb, "DBUPDATE");
	}
}


/*=======================
| Updated pos record (s) |
=======================*/
void 
UpdatePosdt (void)
{
    posdtup_rec.pos_no = 0;
	strcpy (posdtup_rec.file_name,"cumr");
	posdtup_rec.record_hash = cumr_rec.hhcu_hash;

	abc_add (posdtup,&posdtup_rec);
}

void
GetAccount (void)
{
	GL_FormAccNo (cumr_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
	strcpy (glmrRec.co_no,comm_rec.co_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		print_err ("Account %s is not on file.", glmrRec.acc_no);
		sleep (sleepTime);
		return;
	}

	if (Check_class ())
		return;

	if (strncmp (glmrRec.curr_code, cumr_rec.curr_code, 3))
	{
        print_err ("Account %s currency is not equal to customers currency.",
                    glmrRec.acc_no);
		return;
	}
	checkError = FALSE;
}

/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment (void)
{
	char	*sptr;

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

	/*--------------------------------
	| Customer multi-currency flag.  |
	--------------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*-------------------------
	| Suppier Company Owned.  |
	-------------------------*/
	sptr = chk_env ("CR_CO");
	envVar.crCo = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-----------------------------------
	| Suppier Find variable for Search. |
	-----------------------------------*/
	sptr = chk_env ("CR_FIND");
	envVar.crFind = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*--------------------------------------
	| General Ledger Interface Environment |
	--------------------------------------*/
	sptr = chk_env ("GL_BYCLASS");
	envVar.GlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*------------------------------
	| Check for POS installation.  |
	------------------------------*/
	sptr = chk_env ("POS_INSTALLED");
	envVar.PosInstalled = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*---------------------------
	| Check for Currency Code.  |
	---------------------------*/
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	/*---------------------------
	| Check for Contract Code.  |
	---------------------------*/
	sprintf (envVar.countryCode, "%-3.3s", get_env ("CONT_CODE"));

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envVar.sk_dbprinum = 5;
	else
	{
		envVar.sk_dbprinum = atoi (sptr);
		if (envVar.sk_dbprinum > 9 || envVar.sk_dbprinum < 1)
			envVar.sk_dbprinum = 9;
	}
	sprintf (priceTypeComments,"Enter price type [1 - %1d]",envVar.sk_dbprinum);

	/*-------------------------
	| Credit Overide allowed. |
	-------------------------*/
	sptr = chk_env ("SO_CRD_OVERIDE");
	envVar.creditOveride = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*---------------------------------
	| Check for Combined P/S Invoice. |
	---------------------------------*/
	sptr = chk_env ("COMB_INV_PAC");
	if (sptr == (char *)0)
		envVar.combineInvPack = FALSE;
	else
	{
		if (sptr [0] == 'Y' || sptr [0] == 'y')
			envVar.combineInvPack = TRUE;
		else
			envVar.combineInvPack = FALSE;
	}

	/*-------------------------------
	| Take Out Tax for GST Clients. |
	-------------------------------*/
	sprintf (envVar.gst,"%-1.1s", get_env ("GST"));
	if (envVar.gst [0] == 'Y' || envVar.gst [0] == 'y')
	{
		sprintf (envVar.gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
		sprintf (envVar.taxComment, "A=%-3.3s Exempt, C=%-3.3s applies.", 
				 envVar.gstCode, envVar.gstCode);
		sprintf (envVar.taxAllowedCodes, "AC");
	}
	else
	{
		sprintf (envVar.gstCode, "%-3.3s", "Tax");
		sprintf (envVar.taxComment, 
		"A=Tax Exempt, B=Tax Your Care, C=Taxed. D=Taxed using Tax Amount.");
		sprintf (envVar.taxAllowedCodes, "ABCD");
	}
	sprintf (envVar.taxCodePrompt, " %-3.3s Code.        : ",
								envVar.gstCode);
	sprintf (envVar.taxNumberPrompt, " %-3.3s No.          : ", 
								envVar.gstCode);

	/*------------------------------------
	| Customer has Tele-Sales Installed. |
	------------------------------------*/
	sptr = chk_env ("TS_INSTALLED");
	envVar.tsInstalled = (sptr == (char *)0) ? FALSE : atoi (sptr);
}

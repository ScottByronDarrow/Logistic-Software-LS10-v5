/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crcinp.c,v 5.14 2002/11/22 05:18:01 keytan Exp $
|  Program Name  : (db_crcinp.c) 
|  Program Desc  : (Customer Cash Receipts Input)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 12/10/92         |
|---------------------------------------------------------------------|
| $Log: db_crcinp.c,v $
| Revision 5.14  2002/11/22 05:18:01  keytan
| Fix SrchUnallocated receipts. Make sure that invalid cuhd_index_date is also
| considered.
|
| Revision 5.13  2002/07/24 08:38:47  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.12  2002/07/18 06:24:13  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.11  2002/07/16 01:04:19  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crcinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crcinp/db_crcinp.c,v 5.14 2002/11/22 05:18:01 keytan Exp $";

#define MAXSCNS		3
#define MAXLINES	10000

#define		READ_GLWK

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<twodec.h>
#include 	<arralloc.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include    <errno.h>
#include    <getnum.h>
#include    <search_utils.h>
#include    <ml_db_mess.h>

#define	S_MAIN		1
#define	S_ALLC		2
#define	S_DTLS		3

#define	SLEEP_TIME	2

#define INV_NO_LEN  8

#define	DBT_CO_CLOSE	 (envVar.coClose [0] == '1')
#define	BANK_DRAFT		 (local_rec.rec_type [0] == 'B')
#define	DIRECT_CRD		 (local_rec.rec_type [0] == 'D')
#define	CASH_PYMNT		 (local_rec.rec_type [0] == 'A')
#define	CHQ_PAYMNT		 (local_rec.rec_type [0] == 'C')

#define	SPLIT_PAYMENT	 (receiptNoUsed && newReceipt)

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr2_rec;
struct cumrRecord	cumr3_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct cuinRecord	cuin2_rec;
struct cuhdRecord	cuhd_rec;
struct cuhdRecord	cuhd2_rec;
struct cuhdRecord	cuhd3_rec;
struct cudtRecord	cudt_rec;
struct cuchRecord	cuch_rec;
struct cucdRecord	cucd_rec;
struct esmrRecord	esmr_rec;
struct crbkRecord	crbk_rec;
struct bkcrRecord	bkcr_rec;
struct blhdRecord	blhd_rec;
struct bldtRecord	bldt_rec;
struct cuphRecord	cuph_rec;
struct fehrRecord	fehr_rec;
struct fehrRecord	fehr2_rec;
struct felnRecord	feln_rec;
struct fetrRecord	fetr_rec;
struct fetrRecord	fetr2_rec;
struct sachRecord	sach_rec;
struct saclRecord	sacl_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	/*
	 * Customer Receipts Work File Record. 
	 */
	struct {
		char	co_no [3];
		char	br_no [3];
		char	rec_no [9];
		char	reversal [2];
		char	period [3];
		long	hhcp_hash;
		char	bank_id [6];
		char	bank_name [41];
		char	bk_ccode [4];
		char	bk_cdesc [41];
		double	bk_exch;		/* Origin to bank exchange */
		double	bk_rec_amt;		/* Bank receipt amount */
		double	bk_charge;		/* Bank charges */
		double	bk_clear_fee;	/* Bank clearance fee */
		double	bk_l_exch;		/* Bank to local exch rate */
		char	dbt_no [7];		/* Customer Number */
		char	dbt_name [41];	/* Customer Name */
		long	hhcu_hash;		/* Customer Hash */
		long	rec_date;		/* Receipt Date */
		char	rec_type [2];	/* Receipt Type */
		long	due_date;		/* Due Date For Bank Drafts */
		char	narrative [21];	/* Bank Draft Reference */
		char	invoice [9];	/* Invoice being paid */
		double	inv_exch;		/* Invoice Exchange Rate */
		double	inv_amt;		/* Invoice Amount */
		char	o_curr [4];		/* Transaction, customer currency. */
		double	o_exch;			/* Origin to local exchange */
		Money	o_disc;			
		Money	o_total_amt;	
		Money	o_amt_pay;		
		char	gl_disc [MAXLEVEL + 1];	/* GL discount account */
		Money	l_disc;			
		Money	l_total_amt;	
		Money	l_amt_pay;		
	} cudr_rec;

	struct {
		long	hhcuHash;
	} wkRec;

	char	*data  	= 	"data",
			*bldt2 	= 	"bldt2",
			*cuch2 	= 	"cuch2",
			*cucd2 	= 	"cucd2",
			*cudt2 	= 	"cudt2",
			*cuhd2 	= 	"cuhd2",
			*cuhd3 	= 	"cuhd3",
			*cuin2 	= 	"cuin2",
			*cuin3 	= 	"cuin3",
			*cuin4 	= 	"cuin4",
			*cumr2 	= 	"cumr2",
			*cumr3 	= 	"cumr3",
			*fehr2 	= 	"fehr2";

/*
 * The structure envVar groups the values of environment settings together.
 */
struct tagEnvVar
{
	int		feInstall;
	int		GlByClass;
	int		DaysAgeing;
	int		salesCommission;
	int		salesCommissionPayment;
	int		dbHoOnly;
	int		dbAutoRec;
	int		dbNettUsed;
	int		MultiCurrency;
	int		dbCo;
	int		dbFind;
	char	coClose [6];
	char	currCode [4];
	char	dbLdgPres [2];
	int		true_age;
} envVar;

/*
 * The structure exchRate groups the global exchange rate values together.  
 * RULES FOR USING EXHANGE RATES :                
 * 1) The name of the variable is critcial to the use of the exchange rate.
 *    Variables are named as xxxToYyy where xxx is the source and Yyy is the 
 *    destination.   
 * 2) ALWAYS use multiplication to convert a value from xxx to Yyy. 
 * 3) Conversely, to convert from Yyy to xxx use division.
 *                                                
 * Example.                                       
 * The variable bankToLocal is used to convert from Bank currency to Local 
 * currency.  To convert Bank -> Local use multiplication.      
 * To convert Local -> Bank use division.         
 */
struct tagExchRate
{
	double	bankToCustomer;
	double	bankToLocal;
	double	localToCustomer;
} exchRate;

/*
 * The structure lineTotal groups line-level total variables together 
 */
struct tagLineTotal
{
	char	invNo 	[sizeof cuin_rec.inv_no];
	double	payLocal;
	double	payForeign;
	double	invLocal;
} lineTotal [MAXLINES];

/*
 * The structure 'invDtls' is initialised in function 'GetCheques' 
 * the number of details is stored in external variable 'invCnt'. 
 */
struct	tagInvDtls	{
	long	hhciHash;	/* detail invoice reference.       */
	double	inv_oamt;	/* Invoice overseas amount.        */
	double	inv_lamt;	/* Invoice local amount.           */
	double	exch_var;	/* Exchange variation.             */
	double	exch_rate;	/* Exchange rate.                  */
} *invDtls;
static DArray   invDtls_d;
static int      invCnt;

/*
 * The structure headDetail is used to store information about a receipt that 
 * is being adjusted.  Note that we only store info needed for reversing G/L 
 * postings (by way of cudr records).                         
 */
static struct tagHeadDetail
{
	char	recType [2];

	long	dueDate;
	char	narrative [21];
	char	altDrawer [21];
	char	dbBank [4];
	char	dbBranch [21];

	double	bkExch;
	double	bkLclExch;
	double	bkRecAmt;
	double	bkCharge;
	double	bkClearFee;
	double	oDisc;
	double	oTotalAmt;

	double	lDisc;
	double	lTotalAmt;

	long	hhchHash;		/* Link to Letter of Credit (cuch) */
}   headDetails;

/*
 * The structure lineDetail is used to store information about an invoice 
 * that the cash receipt is linked to.  An array of lineDetails is maintained 
 * for existing receipts that are loaded for adjustment.
 */
static struct tagLineDetail
{
	char	invNo [sizeof cuin_rec.inv_no];
	long	invHhci;			
	char	invType [2];
	long	invHhcu;
	char	invPayTerms [4];
	long	invDueDate;
	double	invAmtFgn;
	char	invCurr [4];
	double	invExchRate;
	char	invFixedExch [2];
	double	invAmtLcl;
	char	invFeContract [7];

	double	recAmtFgn;
	double	recExchRate;
	double	recAmtLcl;
	double	recExchVar;
}   *lineDetails;
static DArray   details_d;
static int      dtlsCnt;

/*
 * Global variables. 
 */
int		newReceipt			= FALSE,
		receiptNoUsed		= FALSE,
		multipleReceipts	= FALSE,
		useAltDate			= FALSE,
		fwdMessage   		= TRUE,
		invoiceAdded 		= FALSE,
		currentMonth 		= 0,
		currentYear  		= 0,
		invoiceMonth 		= 0,
		invoiceYear  		= 0,
		selectedYear 		= 0,
		validLetter  		= FALSE,
		tmpLineCnt   		= 0,
		chkError     		= TRUE,
		processUnallocated	= FALSE,
		processDishonoured	= FALSE,
		perFd				= 0,
		crcFd				= 0,
		journalOnly 		= FALSE;

long	altDate      		= 0L,
		monthEndDate 		= 0L;

double  proofTotal 			= 0.00,
		batchTotal 			= 0.00,
		allocateTotal		= 0.00;

char	branchNo 		[3],
		dfltDeposit 	[sizeof	cuin_rec.inv_no],
		dfltExchRate 	[21],
		splitCurr 		[4];

char	*scnDesc [] = {
		"HEADER SCREEN.",
		"HEADER SCREEN.",
		"ALLOCATION SCREEN.",
		""
};

/*
 * Local & Screen Structures. 
 */
struct {
	/*
	 * Screen 1 (S_MAIN) 
	 */
	char	bk_curr_desc [41];
	char	rec_no [9];
	long	rec_date;
	char	rec_type [2];
	char	rec_type_desc [14];
	long	due_date;
	char	narrative [21];
	char	cheque_no [21];
	char	or_no [11];
	double	bank_chg;
	double	rec_amt;
	char	dbt_no [7];
	char	alt_drawer [21];
	char	fe_cont_no [7];
	char	db_bank [4];
	char	db_branch [21];
	double	rec_oamt;
	double	rec_odis;
	double	dbtToLocalExch;
	double	rec_lamt;
	double	rec_gross;
	double	rec_ldis;
	char	lett_crd [16];
	double	lett_amt;
	char	alloc [2];
	char	alloc_desc [31];
	int		sel_month;

	char	recNoComment [128];
	char	bk_ex_prmt 	 [31];
	char	bk_rec_prmt [31];
	char	oamt_prmt   [31];
	char	odis_prmt   [31];
	char	lamt_prmt   [31];
	char	gross_prmt  [31];
	char	lett_prmt   [31];

	/*
	 * Screen 2 (S_ALLC) 
	 */
	char	depositNo [sizeof cuin_rec.inv_no];
	double	unalloc_amt;
	double	unalloc_loc;

	char	unalloc_prmt [41];
	char	unalloc_lprmt [41];

	/*
	 * Screen 3 (S_DTLS) 
	 */
	long	l_hhcu_hash;
	char	l_inv_no [9];
	char	l_inv_type [5];
	long	l_inv_due;
	char	l_curr [4];
	char	l_fx [2];
	double	l_inv_bal;
	double	l_inv_exch;
	double	l_loc_amt;
	double	l_rec_oamt;
	double	l_rec_exch;
	double	l_rec_lamt;
	char	l_fe_cont [7];

	/*
	 * Miscellaneous. 
	 */
	char	dummy [11];
	char	prev_receipt [9];
	char	prev_dbt_no [7];
	char	systemDate [11];
	long	lsystemDate;
	long	mon_inv;
	double	inv_balance;
	double	bankLocalExch;
	char	searchOnInvoice [9];
} local_rec;

static	struct	var	vars [] =
{
	{S_MAIN, LIN, "bank_id",	 3, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Bank Code           : ", "Enter Bank Code. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{S_MAIN, LIN, "bk_name",	 3, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", "Enter Bank name.",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{S_MAIN, LIN, "bk_curr",	 3, 74, CHARTYPE,
		"AAA", "          ",
		" ", "", " /  ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{S_MAIN, LIN, "bk_curr_desc",	 3, 82, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},
	{S_MAIN, LIN, "receipt",	 5, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Receipt No.         : ", local_rec.recNoComment,
		 NO, NO,  JUSTLEFT, "", "", local_rec.rec_no},
	{S_MAIN, LIN, "rec_type",	5, 60, CHARTYPE,
		"U", "          ",
		" ", "C", "Receipt Type.       : ", "Enter A-Cash, (B)ank Draft, (C)heque, (D)irect Credit.",
		YES, NO,  JUSTLEFT, "CBAD", "", local_rec.rec_type},
	{S_MAIN, LIN, "rec_type_desc",5, 86, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.rec_type_desc},
	{S_MAIN, LIN, "or_no",	 6, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "O/R Number          : ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.or_no},
	{S_MAIN, LIN, "cheque",	 6, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Cheque Number       : ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.cheque_no},
	{S_MAIN, LIN, "rec_date",	 7, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date of Receipt     : ", "Enter Receipt date. < default = today > ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.rec_date},
	{S_MAIN, LIN, "due_date",	7, 60, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Draft Due date.     : ", "Enter Due date.",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.due_date},
	{S_MAIN, LIN, "ref",	 7, 95, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Reference : ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.narrative}, 
	{S_MAIN, LIN, "customer",	 9, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No.        : ", "Enter customer number. <return> to move to Invoice search field.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{S_MAIN, LIN, "invoiceSearch",	 9, 32, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Srch Invoice ", "Invoice No Search.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.searchOnInvoice},
	{S_MAIN, LIN, "name",	 9, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{S_MAIN, LIN, "alt_drawer",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Alternate Drawer    : ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.alt_drawer}, 
	{S_MAIN, LIN, "fe_cont_no",	 10, 60, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "F/E Contract No.    : ", "Enter Forward Exchange Contract Number. Search Available.",
		 ND, NO,  JUSTLEFT, "", "", local_rec.fe_cont_no},
	{S_MAIN, LIN, "bank",	11, 2, CHARTYPE,
		"UUU", "          ",
		" ", "", "Customers Bank Code.: ", " Enter Bank code. ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.db_bank},
	{S_MAIN, LIN, "branch",	11, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Branch Code.        : ", "Enter Branch Code ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.db_branch},
	{S_MAIN, LIN, "bk_exch_rate",	 13, 2, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", local_rec.bk_ex_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "9999.99999999", (char *)&exchRate.bankToCustomer},
	{S_MAIN, LIN, "bank_chg",	 13, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Bank Charges.       : ", "Enter amount of Bank charges. ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.bank_chg},
	{S_MAIN, LIN, "orec_amt",	 15, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.oamt_prmt, " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_oamt},
	{S_MAIN, LIN, "orec_dis",	 15, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.odis_prmt, " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_odis},
	{S_MAIN, LIN, "exch_rate",	 16, 2, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", dfltExchRate, "Exchange rate.      : ", " ",
		 NI, NO,  JUSTRIGHT, "0", "9999", (char *)&exchRate.localToCustomer},
	{S_MAIN, LIN, "rec_amt",	 16, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.bk_rec_prmt, "Enter amount of Receipt. ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_amt},
	{S_MAIN, LIN, "lrec_amt",	 17, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.lamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_lamt},
	{S_MAIN, LIN, "rec_gross",	 17, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.gross_prmt, " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_gross},
	{S_MAIN, LIN, "lrec_dis",	 17, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.rec_ldis},
	{S_MAIN, LIN, "lett_crd",	19, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Letter Of Credit    : ", "Enter Letter Of Credit Number",
		NA, NO,  JUSTLEFT, "", "", local_rec.lett_crd},
	{S_MAIN, LIN, "lett_amt",	19, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", local_rec.lett_prmt, "",
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.lett_amt},
	{S_MAIN, LIN, "alloc",	20, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Allocation Type     : ", "Enter (U)nallocated, (R)everse date order, (S)elective document entry, M(onth), (A)mount receipt allocation.",
		NE, NO,  JUSTLEFT, "URSMA", "", local_rec.alloc},
	{S_MAIN, LIN, "alloc_desc",	20, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.alloc_desc},
	{S_MAIN, LIN, "selmonth",	20, 60, INTTYPE,
		"NN", "          ",
		"0", "", "Month (1-12)        : ", " ",
		 NE, NO,  JUSTLEFT, "1", "12", (char *)&local_rec.sel_month},

	{S_ALLC, LIN, "customer", 	3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No.        : ", "Use Normal Customer Search keys or FN11 to Search on Invoice Number.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{S_ALLC, LIN, "name2", 	3, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{S_ALLC, LIN, "receipt",	4, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Receipt No.         : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.rec_no},
	{S_ALLC, LIN, "depositNo",	5, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Deposit Number.     : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.depositNo},
	{S_ALLC, LIN, "bk_curr2",	7, 2, CHARTYPE,
		"AAA", "          ",
		" ", "", "Currency            : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.curr_code},
	{S_ALLC, LIN, "bk_curr_desc2", 7, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},
	{S_ALLC, LIN, "exch_rate2",	 8, 2, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Exchange rate.      : ", " ",
		 NA, NO,  JUSTRIGHT, "0", "9999", (char *)&exchRate.localToCustomer},
	{S_ALLC, LIN, "fe_cont_no2",	 8, 60, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "F/E Contract No.    : ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.fe_cont_no},
	{S_ALLC, LIN, "orec_amt2",	 9, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.oamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_oamt},
	{S_ALLC, LIN, "unalloc_amt",	 9, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.unalloc_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.unalloc_amt},
	{S_ALLC, LIN, "lrec_amt2",	 10, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.lamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_lamt},
	{S_ALLC, LIN, "unalloc_loc",	 10, 60, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.unalloc_lprmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.unalloc_loc},
	{S_ALLC, LIN, "alloc2",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Allocation Type     : ", "Enter (R)everse date order, (S)elective document entry, M(onth), (A)mount receipt allocation.",
		NE, NO,  JUSTLEFT, "RSMA", "", local_rec.alloc},
	{S_ALLC, LIN, "alloc_desc2",	12, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.alloc_desc},
	{S_ALLC, LIN, "selmonth2",	12, 60, INTTYPE,
		"NN", "          ",
		"0", "", "Month (1-12)        : ", " ",
		 NE, NO,  JUSTLEFT, "1", "12", (char *)&local_rec.sel_month},

	{S_DTLS, TAB, "l_invoice",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", " Invoice", "Enter DEP for Deposit for unallocated cash.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.l_inv_no},
	{S_DTLS, TAB, "l_type",	0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Type", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_inv_type},
	{S_DTLS, TAB, "l_inv_date",	0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", " Due Date ", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_inv_due},
	{S_DTLS, TAB, "l_invamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Invoice Amt (Fgn)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_inv_bal},
	{S_DTLS, TAB, "l_curr",	0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "", "Cur", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_curr},
	{S_DTLS, TAB, "l_inv_exch",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Invoice Exch", " ",
		 NA, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.l_inv_exch},
	{S_DTLS, TAB, "l_fx",	0, 1, CHARTYPE,
		"U", "          ",
		" ", "", "F/X", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_fx},
	{S_DTLS, TAB, "l_locamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Invoice Amt (Loc)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_loc_amt},
	{S_DTLS, TAB, "l_rec_oamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Receipt Amt (Fgn)", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_oamt},
	{S_DTLS, TAB, "l_rec_exch",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Receipt Exch.", " ",
		 NA, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.l_rec_exch},
	{S_DTLS, TAB, "l_rec_lamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Receipt Amt (Loc)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_lamt},
	{S_DTLS, TAB, "l_fe_cont",	 0, 1, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "F/E Cont.", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_fe_cont},
	{S_DTLS, TAB, "hhciHash",	 0, 0, LONGTYPE,
		"NNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&cuin_rec.hhci_hash},
	{S_DTLS, TAB, "l_hhcu_hash",  0, 0, LONGTYPE,
       "NNNNNNNNN", "          ",
       " ", "", "", "",
        ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.l_hhcu_hash},
	{S_DTLS, TAB, "pay_terms",	 0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", cuin_rec.pay_terms},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern	int		TruePosition;

extern	int		NO_SRCH_LINES;
extern	int		SR_X_POS;
extern	int		SR_Y_POS;
int		ORIG_NO_SRCH_LINES;
int		ORIG_SR_X_POS;
int		ORIG_SR_Y_POS;
int     alloc;

#include <FindCumr.h>
/*
 * Local Function Prototypes. 
 */
double	CalcExchVariation 	 (void);
int 	CheckFehr 			 (void);
int 	DeleteLine 			 (int);
int 	DuplicateInvNo 		 (char *);
int 	FindDeposits 		 (int, int);
int 	LoadReceipt 		 (void);
int 	ProofTrans 			 (void);
int 	ValidAlloc 			 (void);
int 	WarnMultiple 		 (void);
int 	heading 			 (int);
int 	spec_valid 			 (int);
long 	ReadBranchDate 		 (char *);
void 	AddCommission 		 (long, long, long, double);
void 	AddCudr 			 (int, int);
void 	AddFetr 			 (void);
void 	AddInvoice 			 (int, char *);
void 	AddPaymentHistory 	 (void);
void 	CalcLetterAmt 		 (void);
void 	CheckEnvironment 	 (void);
void 	ChequeInvBal 		 (void);
void 	CloseDB 			 (void);
void 	CreateDrawOff 		 (double);
void 	DisplayTotals 		 (void);
void 	GenerateReceiptNo 	 (void);
void 	GetCheques 			 (int, long);
void 	LoadDeposit 		 (void);
void 	OpenDB 				 (int);
void 	ReadAmount 			 (int);
void 	ReadInvoices 		 (int, long, long);
void 	ReverseReceipt 		 (void);
void 	SrchCrbk 			 (char *);
void 	SrchCuch 			 (char *);
void 	SrchDeposit 		 (char *);
void 	SrchFehr 			 (char *);
void 	SrchInvoice 		 (char *);
void 	SrchLineInvoice 	 (char *);
void 	SrchReceipt 		 (char *);
void 	SrchUnallocated 	 (char *);
void 	UnlockRecords 		 (void);
void 	Update 				 (void);
void 	UpdatePayeeDetails 	 (void);
void 	shutdown_prog 		 (void);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char	*argv [])
{
	int		i;
	int		pid;
	char	*sptr;

	TruePosition	=	TRUE;

	if (argc < 2)
	{
		print_at (0,0, "Usage %s <PID>\n", argv [0]);
        return (EXIT_FAILURE);
	}
	ORIG_NO_SRCH_LINES	= NO_SRCH_LINES;
	ORIG_SR_X_POS		= SR_X_POS;
	ORIG_SR_Y_POS		= SR_Y_POS;

	/*
	 * Check program name. 
	 */
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

    alloc = FALSE;
    if (!strcmp (sptr, "db_crcallc"))
        alloc = TRUE;

	processUnallocated = FALSE;
	processDishonoured = FALSE;

	if (!strcmp (sptr, "db_crcallc"))
		processUnallocated = TRUE;

	if (!strcmp (sptr, "db_crcdis"))
		processDishonoured = TRUE;

	pid = atoi (argv [1]);

	/*
	 * Will Set journalOnly to true if any argument is passed to Program. 
	 * journalOnly = (argc > 2);
	 */

	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();

    /*
     * Allocate intital detail line array. 
     */
    ArrAlloc (&invDtls_d, &invDtls, sizeof (struct tagInvDtls), 1000);
    ArrAlloc (&details_d, &lineDetails, sizeof (struct tagLineDetail), 1000);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	_set_masks ("db_crcinp.s");
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (S_DTLS, lineTotal, sizeof (struct tagLineTotal));
#endif

	for (i = 0; i < 4; i++)
		tab_data [i]._desc = scnDesc [i];

	if (!envVar.MultiCurrency)
		envVar.feInstall = FALSE;

	if (envVar.feInstall)
	{
		FLD ("fe_cont_no")  = YES;
		FLD ("fe_cont_no2") = NA;
		FLD ("l_fe_cont")   = NA;
		FLD ("l_rec_exch")  = ND;
	}

	/*
	 * Use screen 2 for Allocate Unallocated Cash. Otherwise use screen 1.
	 */
	if (processUnallocated)
		no_edit (S_MAIN);
	else
		no_edit (S_ALLC);

	init_vars (S_MAIN);

	tab_row = 5;

	swide ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB (pid);

	strcpy (local_rec.recNoComment, ML ("Enter Cheque / Receipt Number."));
	if (envVar.dbAutoRec)
	{
		strcat (local_rec.recNoComment, 
				ML (" <Return> for System Generated Number"));
	}

	strcpy (local_rec.oamt_prmt,   	ML ("Receipt Amount        "));
	strcpy (local_rec.odis_prmt,   	ML ("Discount Amount       "));
	strcpy (local_rec.bk_ex_prmt,  	ML ("Bank exch rate        "));
	strcpy (local_rec.bk_rec_prmt, 	ML ("Bank Receipt          "));
	strcpy (local_rec.lett_prmt,   	ML ("Remaining Amount      "));
	strcpy (local_rec.unalloc_prmt,	ML ("Unallocated Amt       "));
	strcpy (local_rec.unalloc_lprmt,ML ("Unallocated Amt       "));
	sprintf (local_rec.lamt_prmt,  		"%s(%-3.3s): ", 
							ML ("Receipt Amount "), envVar.currCode);
	sprintf (local_rec.gross_prmt, 		"%s(%-3.3s): ", 
							ML ("Gross receipt  "), envVar.currCode);
	sprintf (local_rec.unalloc_lprmt,	"%s(%-3.3s): ",
							ML ("Unallocated Amt"), envVar.currCode);
	
	DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, &currentYear);

	monthEndDate = MonthEnd (comm_rec.dbt_date);

	/*
	 * Get control accounts. 
	 */
	GL_SetMask (GlFormat);

	strcpy (branchNo, (envVar.dbCo) ? comm_rec.est_no : " 0");

	/*
	 * Find Company Master file record. 
	 */
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_unlock (comr);

	/*
	 * Beginning of Input Control Loop. 
	 */
	prog_exit = FALSE;
	while (!prog_exit)
	{
		UnlockRecords ();

		edit_exit  = FALSE;
		entry_exit = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;

		init_vars (1);			/*  set default values  */
		lcount [S_DTLS] = 0;

		FLD ("bank_id")   = NE;
		FLD ("receipt")   = NO;
		FLD ("rec_date")  = YES;
		FLD ("rec_type")  = YES;
		FLD ("due_date")  = YES;
		FLD ("ref")       = YES;
		FLD ("customer")    = NE;
		FLD ("exch_rate") = NI;

		/*
		 * When processing dishonoured cheques, we do not allow editing 
		 * of any fields.  We allow entry of Bank, Receipt and Customer.
		 */
		if (processDishonoured)
		{
			/*
			 * S_MAIN 
			 */
			FLD ("bank_id")    = NE;
			FLD ("receipt")    = NE;
			FLD ("customer")   = NE;
			FLD ("rec_date")   = NA;
			FLD ("rec_type")   = NA;
			FLD ("due_date")   = NA;
			FLD ("ref")        = NA;
			FLD ("bank_chg")   = NA;
			FLD ("alt_drawer") = NA;
			FLD ("fe_cont_no") = NA;
			FLD ("bank")       = NA;
			FLD ("branch")     = NA;
			FLD ("orec_amt")   = NA;
			FLD ("orec_dis")   = NA;
			FLD ("exch_rate")  = NA;

			/*
			 * S_DTLS 
			 */
			FLD ("l_invoice")  = NA;
			FLD ("l_rec_oamt") = NA;
		}

		newReceipt       = FALSE;
		receiptNoUsed    = FALSE;
		multipleReceipts = FALSE;
		chkError         = TRUE;
		validLetter      = FALSE;

		strcpy (dfltDeposit, "");

		for (i = 0; i < MAXLINES; i++)
		{
			strcpy (lineTotal [i].invNo, "        ");
			lineTotal [i].payForeign = 0.00;
			lineTotal [i].payLocal = 0.00;
			lineTotal [i].invLocal = 0.00;
		}

		init_vars (S_DTLS);

		/*
		 * Enter screen 1 linear input 
		 */
		heading ( (processUnallocated) ? S_ALLC : S_MAIN);
		entry ( (processUnallocated) ? S_ALLC : S_MAIN);
		if (prog_exit || restart)
			continue;

		/*
		 * Enter / Edit Screen 3 Tabular Input . 
		 */
		if (newReceipt)
		{
			if (local_rec.alloc [0] == 'S')
			{
				heading (S_DTLS);
				entry (S_DTLS);
			}
			else
			{
				prog_status = ! (ENTRY);
				heading (S_DTLS);
				scn_display (S_DTLS);
				DisplayTotals ();
				edit (S_DTLS);
			}

			if (lcount [S_DTLS] == 0)
				restart = TRUE;

			if (restart)
				continue;
		}

		/*
		 * The following line is a kludge.  For some reason 
		 * the first time through edit_all () the program  
		 * still thinks it is in ENTRY mode.              
		 */
		prog_status = EDIT;

		edit_all ();

		if (restart)
			continue;

		ProofTrans ();

		if (restart)
			continue;

		if (envVar.feInstall && !chkError)
			chkError = CheckFehr ();

		/*
		 * re-edit tabular if proof total incorrect. 
		 */
		while (chkError)
		{
			edit_all ();
			if (restart)
				break;

			if (prog_exit)
			{
				prog_exit = FALSE;
				continue;
			}

			ProofTrans ();

			if (envVar.feInstall && !chkError)
				chkError = CheckFehr ();
		}

		if (!restart)
			Update ();
	}

	/*
	 * Tidy up array buffer. 
	 */
    ArrDelete (&invDtls_d);
    ArrDelete (&details_d);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Check environment variables and set values in the envVar structure. 
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	/*
	 * Forward Exchange Enabled? 
	 */
	sptr = chk_env ("FE_INSTALL");
	envVar.feInstall = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	/*
	 * General ledger interface by Customer type or salesman. 
	 */
	sptr = chk_env ("GL_BYCLASS");
	envVar.GlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Days ageing for customers. 
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envVar.DaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Sales commissions applies. 
	 */
	sptr = chk_env ("SA_COMMISSION");
	envVar.salesCommission = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Sales commissions payment by receipt. 
	 */
	sptr = chk_env ("SA_COMM_PAYMENT");
	envVar.salesCommissionPayment = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Multi Currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envVar.MultiCurrency = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Customer head office rules. 
	 */
	sptr = chk_env ("DB_HO_ONLY");
	envVar.dbHoOnly = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	/*
	 * Auto generated receipt numbers. 
	 */
	sptr = chk_env ("DB_AUTO_REC");
	if (sptr)
		envVar.dbAutoRec = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envVar.dbAutoRec = FALSE;

	/*
	 * Net or Gross values. 
	 */
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *) 0) ? TRUE : atoi (sptr);

	/*
	 * True ageing or module ageing. 
	 */
	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *) 0)
		envVar.true_age = FALSE;
	else
		envVar.true_age = (*sptr == 'T' || *sptr == 't');

	/*
	 * Company close. 
	 */
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *) 0)
		sprintf (envVar.coClose, "%-5.5s", "11111");
	else
		sprintf (envVar.coClose, "%-5.5s", sptr);

	/*
	 * Lodgements presented flag. 
	 */
	sptr = chk_env ("DB_LDG_PRES");
	if (sptr == (char *) 0)
		strcpy (envVar.dbLdgPres, "N");
	else
		sprintf (envVar.dbLdgPres, "%-1.1s", sptr);

	/*
	 * Get local currency code. 
	 */
	sprintf (envVar.currCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
	 * Company or Branch owned customer. 
	 */
	envVar.dbCo = atoi (get_env ("DB_CO"));

	/*
	 * Invoice from anywhere ?. 
	 */
	envVar.dbFind = atoi (get_env ("DB_FIND"));
	envVar.dbFind  = atoi (get_env ("DB_FIND"));

	if (!DBT_CO_CLOSE && envVar.dbFind && envVar.dbCo)
		useAltDate = TRUE;
	else
		useAltDate = FALSE;
}

/*
 * Program Exit Sequence. 
 */
void
shutdown_prog (void)
{
	clear ();
	snorm ();
	if (!processUnallocated)
	{
        char    temp_msg [80];
        sprintf (temp_msg,
                 ML ("Batch Total = %-8.2f. Press Any Key To Continue."),
                 DOLLARS (batchTotal));
        PauseForKey (0, 0, temp_msg, 0);
	}
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files. 
 */
void
OpenDB (
	int		pid)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	/*
	 * Open work file used for cash receipts journals etc. 
	 */
	sprintf (filename,
			 "%s/WORK/db_crc%05d",
			 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);
	cc = RF_OPEN (filename, sizeof (cudr_rec), "w", &crcFd);
	if (cc)
        file_err (cc, filename, "RF_OPEN");

	/* 
	 * Open work file used for period calcs. 
	 */
	sprintf (filename,
			 "%s/WORK/db_per%05d",
			 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);
	cc = RF_OPEN (filename, sizeof (wkRec), "w", &perFd);
	if (cc)
		sys_err ("Error in db_per During (WKOPEN)", cc, PNAME);

	/*
	 * Open database files. 
	 */
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (bldt2, bldt);
	abc_alias (cucd2, cucd);
	abc_alias (cuch2, cuch);
	abc_alias (cudt2, cudt);
	abc_alias (cuhd2, cuhd);
	abc_alias (cuhd3, cuhd);
	abc_alias (cuin2, cuin);
	abc_alias (cuin3, cuin);
	abc_alias (cuin4, cuin);
	abc_alias (cumr2, cumr);
	abc_alias (cumr3, cumr);
	abc_alias (fehr2, fehr);

	open_rec (bkcr,  bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
	open_rec (blhd,  blhd_list, BLHD_NO_FIELDS, "blhd_id_no");
	open_rec (bldt,  bldt_list, BLDT_NO_FIELDS, "bldt_id_no");
	open_rec (bldt2, bldt_list, BLDT_NO_FIELDS, "bldt_hhcp_hash");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (crbk,  crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cuch,  cuch_list, CUCH_NO_FIELDS, "cuch_id_no");
	open_rec (cuch2, cuch_list, CUCH_NO_FIELDS, "cuch_hhch_hash");
	open_rec (cucd,  cucd_list, CUCD_NO_FIELDS, "cucd_id_no2");
	open_rec (cucd2, cucd_list, CUCD_NO_FIELDS, "cucd_hhcp_hash");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cudt2, cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cuhd2, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cuhd3, cuhd_list, CUHD_NO_FIELDS, "cuhd_receipt_no");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_cron3");
	open_rec (cuin2, cuin_list, CUIN_NO_FIELDS, "cuin_inv_no");
	open_rec (cuin3, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cuin4, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, 
						(!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumr3, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cuph,  cuph_list, CUPH_NO_FIELDS, "cuph_id_no");

	OpenPocr ();

	if (envVar.feInstall)
	{
		open_rec (fehr,  fehr_list, FEHR_NO_FIELDS, "fehr_id_no");
		open_rec (fehr2, fehr_list, FEHR_NO_FIELDS, "fehr_hhfe_hash");
		open_rec (feln,  feln_list, FELN_NO_FIELDS, "feln_id_no");
		open_rec (fetr,  fetr_list, FETR_NO_FIELDS, "fetr_id_no");
	}
	if (envVar.salesCommission && envVar.salesCommissionPayment)
	{
		open_rec (sach,  sach_list, SACH_NO_FIELDS, "sach_hhci_hash");
		open_rec (sacl,  sacl_list, SACL_NO_FIELDS, "sacl_id_no");
	}
}

/*
 * Close data base files . 
 */
void
CloseDB (void)
{
	abc_fclose (bkcr);
	abc_fclose (blhd);
	abc_fclose (bldt);
	abc_fclose (bldt2);
	abc_fclose (comr);
	abc_fclose (crbk);
	abc_fclose (cucd);
	abc_fclose (cucd2);
	abc_fclose (cuch);
	abc_fclose (cuch2);
	abc_fclose (cudt);
	abc_fclose (cudt2);
	abc_fclose (cuhd);
	abc_fclose (cuhd2);
	abc_fclose (cuhd3);
	abc_fclose (cuin);
	abc_fclose (cuin2);
	abc_fclose (cuin3);
	abc_fclose (cuin4);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cumr3);
	abc_fclose (esmr);
	abc_fclose (cuph);

	GL_Close ();

	if (envVar.feInstall)
	{
		abc_fclose (fehr);
		abc_fclose (fehr2);
		abc_fclose (feln);
		abc_fclose (fetr);
	}
	if (envVar.salesCommission && envVar.salesCommissionPayment)
	{
		abc_fclose (sach);
		abc_fclose (sacl);
	}
	abc_dbclose (data);

	cc = RF_CLOSE (crcFd);
	if (cc)
		file_err (cc, "cudr_no", "WKCLOSE");

	cc = RF_CLOSE (perFd);
	if (cc)
		file_err (cc, "db_per", "WKCLOSE");
}

/*
 * Validate fields 
 */
int
spec_valid (
	int		field)
{
	int i = 0;
	int c;
	int i_del;
	int sav_scn;
	int dep_found;

	long max_fdate = 0L;

    int     no_lines;
    int     DepositFlag = FALSE;

    double  ActTotal = 0.00,
            DepTotal = 0.00;

	if (LCHECK ("fe_cont_no"))
	{
		if (!envVar.feInstall)
		{
			fehr_rec.hhfe_hash = 0L;
			strcpy (fehr_rec.cont_no, "      ");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (strcmp (fehr_rec.cont_no, "      "))
			{
				/*
				 * Read exchange record for Customer currency at current Bank. 
				 */
				strcpy (bkcr_rec.co_no, comm_rec.co_no);
				strcpy (bkcr_rec.bank_id, crbk_rec.bank_id);
				strcpy (bkcr_rec.curr_code, cumr_rec.curr_code);
				cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess040));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				exchRate.bankToCustomer = bkcr_rec.ex1_factor;
			}
			fehr_rec.hhfe_hash = 0L;
			strcpy (fehr_rec.cont_no, "      ");
		}
		else
		{
			if (SRCH_KEY)
			{
				SrchFehr (temp_str);
				return (EXIT_SUCCESS);
			}

			strcpy (fehr_rec.co_no, comm_rec.co_no);
			strcpy (fehr_rec.cont_no, local_rec.fe_cont_no);
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
				print_mess (ML (mlDbMess007));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (strcmp (fehr_rec.bank_id, crbk_rec.bank_id))
			{
				print_mess (ML (mlDbMess008));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (fehr_rec.stat_flag [0] != 'A')
			{
				print_mess (ML (mlDbMess009));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			/*
			 * now see if contract is still current.   
			 */
			if (fehr_rec.date_exp < local_rec.lsystemDate)
			{
				sprintf (err_str, ML (mlDbMess010), DateToString (fehr_rec.date_exp));
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (fehr_rec.date_wef > local_rec.lsystemDate)
			{
				sprintf (err_str, ML (mlDbMess011),DateToString (fehr_rec.date_wef));
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			exchRate.bankToCustomer = fehr_rec.exch_rate;
		}

		cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		exchRate.localToCustomer = exchRate.bankToCustomer / exchRate.bankToLocal;

		if (prog_status != ENTRY)
		{
			local_rec.rec_amt = local_rec.rec_oamt;
			if (exchRate.bankToCustomer != 0.00)
				local_rec.rec_amt = no_dec (local_rec.rec_amt
											/ exchRate.bankToCustomer);

			local_rec.rec_lamt = local_rec.rec_oamt;
			if (exchRate.localToCustomer != 0.00)
				local_rec.rec_lamt = no_dec (local_rec.rec_lamt
											 / exchRate.localToCustomer);

			DSP_FLD ("lrec_amt");

			local_rec.rec_gross = local_rec.rec_odis
				+ local_rec.rec_oamt;
			local_rec.rec_ldis = local_rec.rec_odis;

			if (exchRate.localToCustomer != 0.00)
			{
				local_rec.rec_gross = no_dec (local_rec.rec_gross
											  / exchRate.localToCustomer);
				local_rec.rec_ldis = no_dec (local_rec.rec_ldis
											 / exchRate.localToCustomer);
			}

			DSP_FLD ("rec_gross");

			if (local_rec.alloc [0] == 'A'
				|| local_rec.alloc [0] == 'R')
			{
				rv_pr (ML (mlDbMess012), 45, 21, 45);
				sleep (sleepTime);
				line_at (21,0,132);
				if (local_rec.alloc [0] == 'A')
					ReadAmount (TRUE);
				else
					ReadAmount (FALSE);
			}
			if (local_rec.alloc [0] == 'S' && lcount [S_DTLS] > 0)
			{
				sprintf (err_str, "\007 WARNING - Selected Invoices may have forward exchange contracts overidden ");
				rv_pr (ML (mlDbMess012), 45, 21, 45);
				sleep (sleepTime);
				line_at (21,0,132);
			}
			if (local_rec.alloc [0] == 'M')
			{
				rv_pr (ML (mlDbMess012), 45, 21, 45);
				sleep (sleepTime);
				line_at (21,0,132);

				abc_selfield (cuin, cumr_rec.ho_dbt_hash ? "cuin_cron3" 
															: "cuin_ho_cron3");

				selectedYear = (local_rec.sel_month <= currentMonth) 
											? currentYear : currentYear - 1;

				ReadInvoices (TRUE, 
                              cumr_rec.hhcu_hash,
                              cumr_rec.ho_dbt_hash);
				if (tmpLineCnt >= MAXLINES)
				{
					print_mess (ML (mlDbMess036));
					sleep (sleepTime);
					clear_mess ();
				}
				lcount [S_DTLS] = tmpLineCnt;
				scn_set (S_MAIN);
			}

			sav_scn = cur_screen;
			scn_set (S_DTLS);
			for (i = 0; i < lcount [S_DTLS]; i++)
			{
				getval (i);
				local_rec.l_rec_exch = exchRate.localToCustomer;
				if (local_rec.l_rec_exch != 0.00)
					local_rec.l_rec_lamt = no_dec (local_rec.l_rec_oamt
												   / local_rec.l_rec_exch);
				else
					local_rec.l_rec_lamt = 0.00;

				lineTotal [i].payLocal = local_rec.l_rec_lamt;
				putval (i);
			}
			scn_set (sav_scn);
		}

		DSP_FLD ("exch_rate");
		DSP_FLD ("bk_exch_rate");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("l_inv_date"))
	{
		DSP_FLD ("l_inv_date");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("exch_rate"))
	{
		if (exchRate.localToCustomer <= 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.rec_amt = local_rec.rec_oamt;
		if (exchRate.bankToCustomer != 0.00)
			local_rec.rec_amt = no_dec (local_rec.rec_amt
										/ exchRate.bankToCustomer);

		local_rec.rec_lamt = local_rec.rec_oamt;
		if (exchRate.localToCustomer != 0.00)
			local_rec.rec_lamt = no_dec (local_rec.rec_lamt
										 / exchRate.localToCustomer);

		DSP_FLD ("lrec_amt");

		local_rec.rec_gross = local_rec.rec_odis
			+ local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (exchRate.localToCustomer != 0.00)
		{
			local_rec.rec_gross = no_dec (local_rec.rec_gross
										  / exchRate.localToCustomer);
			local_rec.rec_ldis = no_dec (local_rec.rec_ldis
										 / exchRate.localToCustomer);
		}

		DSP_FLD ("rec_gross");

		if (prog_status != ENTRY)
		{
			sav_scn = cur_screen;
			scn_set (S_DTLS);
			for (i = 0; i < lcount [S_DTLS]; i++)
			{
				getval (i);
				local_rec.l_rec_exch = exchRate.localToCustomer;
				if (local_rec.l_rec_exch != 0.00)
					local_rec.l_rec_lamt = no_dec (local_rec.l_rec_oamt
												   / local_rec.l_rec_exch);
				else
					local_rec.l_rec_lamt = 0.00;

				lineTotal [i].payLocal = local_rec.l_rec_lamt;
				putval (i);
			}
			scn_set (sav_scn);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Creditor Number And Allow Search. 
	 */
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (!cc)
				strcpy (crbk_rec.bank_id, esmr_rec.dflt_bank);
		}

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Read exchange record for Local currency at current Bank. 
		 */
		if (envVar.MultiCurrency)
		{
			strcpy (bkcr_rec.co_no,     comm_rec.co_no);
			strcpy (bkcr_rec.bank_id,   crbk_rec.bank_id);
			strcpy (bkcr_rec.curr_code, envVar.currCode);
			cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (err_str,ML (mlDbMess235),envVar.currCode,bkcr_rec.bank_id);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			exchRate.bankToLocal = bkcr_rec.ex1_factor;
		}

		sprintf (local_rec.bk_rec_prmt, ML ("Bank Receipt (%s)  : "),
				 crbk_rec.curr_code);

		display_prmpt (label ("rec_amt"));

		/*
		 * Find lodgement header. 
		 */
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		cc = find_rec (blhd, &blhd_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess078));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		if (!envVar.MultiCurrency)
			exchRate.bankToLocal = pocrRec.ex1_factor;

		strcpy (local_rec.bk_curr_desc, pocrRec.description);
		DSP_FLD ("bk_name");
		DSP_FLD ("bk_curr_desc");
		DSP_FLD ("bk_curr");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Receipt Number.                              
	 * This field depends on the value of the Logistic       
	 * environment variable DB_AUTO_REC.                     
	 *                                                       
	 * DB_AUTO_REC = M(Manual)                              
	 * - Default is not allowed.  The user must enter a receipt number.  
	 *   It does not matter at this stage whether the receipt exists on 
	 *   file or not.          
	 *                                                       
	 * DB_AUTO_REC = S(System)                              
	 * - Default is allowed - defaults to NEW RCPT.          
	 * - Entry of an existing receipt number is allowed to   
	 *   enable adjustment of existing receipts.             
	 *   Validate against the cuhd table.                    
	 *                                                       
	 * Search is available regardless of the setting of the  
	 * above environment variable.                           
	 */
	if (LCHECK ("receipt"))
	{
		strcpy (splitCurr, "   ");

		/*
		 * Allocation of unallocated receipts. 
		 */
		if (processUnallocated)
			return (ValidAlloc ());

		if (SRCH_KEY)
		{
			SrchReceipt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.rec_no, "NEW RCPT"))
		{
			if (envVar.dbAutoRec && !processDishonoured)
			{
				strcpy (local_rec.rec_no, "NEW RCPT");
				receiptNoUsed    = FALSE;
				DSP_FLD ("receipt");
			}
			else
				return (EXIT_FAILURE);
		}
		else
		{
			/*
			 * Find existing receipt on file. 
			 */
			strcpy (cuhd2_rec.receipt_no, 
					zero_pad (local_rec.rec_no, INV_NO_LEN));
			cc = find_rec (cuhd3, &cuhd2_rec, COMPARISON, "r");
			if (cc)
				receiptNoUsed = FALSE;
			else
			{
				receiptNoUsed = TRUE;

				/*
				 * Find the cumr record to record the currency code.  
				 * It is not allowed to split payments over customers  
				 * with different currencies.         
				 */
				cumr3_rec.hhcu_hash	=	cuhd2_rec.hhcu_hash;
				cc = find_rec (cumr3, &cumr3_rec, EQUAL, "r");
				if (cc)
					file_err (cc, cumr3, "DBFIND");

				strcpy (splitCurr, cumr3_rec.curr_code);

				/*
				 * Check that lodgement for original receipt      
				 * is still open, and was lodged in current bank.
				 */
				
				bldt_rec.hhcp_hash	=	cuhd2_rec.hhcp_hash;
				cc = find_rec (bldt2, &bldt_rec, EQUAL, "r");
				if (cc)
				{
					print_mess (ML (mlDbMess247));
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}

				if (bldt_rec.hhbl_hash != blhd_rec.hhbl_hash)
				{
					sprintf (err_str, ML (mlDbMess017), local_rec.rec_no, 
							 							crbk_rec.bank_id);
					print_mess (err_str);
					sleep (sleepTime);
					return (EXIT_FAILURE);
				}
				else
				{
					if (bldt_rec.lodge_no != blhd_rec.nx_lodge_no)
					{
						if (!processDishonoured)
						{
							if (bldt_rec.reconcile [0] == 'Y')
							{
								print_mess (ML (mlDbMess236));
								sleep (sleepTime);
								clear_mess ();
								return (EXIT_FAILURE);
							}

							if (bldt_rec.presented [0] == 'Y')
							{
								print_mess (ML (mlDbMess237));
								sleep (sleepTime);
							}
						}
					}
				}
			}

			if (envVar.dbAutoRec && !receiptNoUsed)
			{
				print_mess (ML (mlDbMess239));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (processDishonoured)
			{
				/*
				 * Receipt must exist. 
				 */
				if (!receiptNoUsed)
				{
					print_mess (ML (mlDbMess239));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				/*
				 * Cannot dishonour CASH. 
				 */
				if (cuhd2_rec.rec_type [0] == 'A')
				{
					print_mess (ML (mlDbMess240));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (WarnMultiple ())
				return (EXIT_FAILURE);
		}

		if (prog_status == ENTRY)
			lcount [S_DTLS] = 0;

		if (processDishonoured)
			skip_entry = goto_field (field, label ("customer"));

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Department Number. 
	 */
	if (LCHECK ("depositNo"))
	{
		if (SRCH_KEY)
		{
			SrchDeposit (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
	     * Use deposit chosen in search. 
	     */
		if (dflt_used && strlen (dfltDeposit) != 0)
			sprintf (local_rec.depositNo, "%8.8s", dfltDeposit);

		/*
	     * Check deposit exists. 
	     */
		dep_found = FALSE;
		local_rec.unalloc_amt = 0.00;
		local_rec.unalloc_loc = 0.00;
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
		while (!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash)
		{
			cuin_rec.hhci_hash	=	cudt_rec.hhci_hash;
			cc = find_rec (cuin3, &cuin_rec, EQUAL, "r");
			if (!cc && !strcmp (cuin_rec.inv_no, local_rec.depositNo))
			{
				dep_found = TRUE;
				if (envVar.feInstall)
				{
					LoadDeposit ();
					DSP_FLD ("fe_cont_no2");
				}
				else
				{
					strcpy (local_rec.fe_cont_no, "       ");
					strcpy (fehr_rec.cont_no, "       ");
					fehr_rec.hhfe_hash = 0L;
				}
				local_rec.unalloc_amt += cudt_rec.amt_paid_inv;
				local_rec.unalloc_loc += cudt_rec.loc_paid_inv;
			}
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}

		if (!dep_found)
		{
			print_mess (ML (mlDbMess037));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.unalloc_amt == 0.00)
		{
			print_mess (ML (mlDbMess038));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("unalloc_amt");
		DSP_FLD ("unalloc_loc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("invoiceSearch"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			skip_entry = goto_field (field, label ("customer"));
			return (EXIT_SUCCESS);
		}
		abc_selfield (cumr, "cumr_hhcu_hash");
		abc_selfield (cuin2, "cuin_inv_no");
			
		if (SRCH_KEY)
		{
			SrchInvoice (temp_str);
			abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
			return (EXIT_SUCCESS);
		}

		if (cumr_rec.hhcu_hash	==	0L)
			abc_selfield (cuin2, "cuin_inv_no");
		else
			abc_selfield (cuin2, "cuin_id_no2");

		strcpy (local_rec.searchOnInvoice, 
						zero_pad (local_rec.searchOnInvoice,INV_NO_LEN));
		strcpy (cuin2_rec.inv_no,local_rec.searchOnInvoice);
		cuin2_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cuin2, &cuin2_rec, GTEQ, "r");
		if (cc || strcmp (local_rec.searchOnInvoice, cuin2_rec.inv_no) || 
			 (cumr_rec.hhcu_hash > 0L && cuin2_rec.hhcu_hash != cumr_rec.hhcu_hash))
		{
			print_mess (mlStdMess115);
			cumr_rec.hhcu_hash = 0L;
			strcpy (local_rec.searchOnInvoice, "        ");
			abc_selfield (cuin2, "cuin_inv_no");
			sleep (sleepTime);
			abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
			return (EXIT_FAILURE);
		}

		cc = find_rec (cuin2, &cuin2_rec, NEXT, "r");
		if (!cc && !strcmp (local_rec.searchOnInvoice, cuin2_rec.inv_no) &&
			cumr_rec.hhcu_hash == 0L)
		{
			NO_SRCH_LINES	=	2;
			SR_X_POS		=	vars [field].col; 
			SR_Y_POS		=	vars [field].row - 1; 
			abc_selfield (cumr, "cumr_hhcu_hash");
			abc_selfield (cuin2, "cuin_inv_no");

			SrchInvoice (temp_str);
			abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
			cl_box (SR_X_POS, SR_Y_POS, 64,NO_SRCH_LINES + 4);
			erase_box (SR_X_POS, SR_Y_POS, 64,NO_SRCH_LINES + 4);
			pr_box_lines (S_MAIN);
			scn_write (S_MAIN);
			scn_display (S_MAIN);
			NO_SRCH_LINES	= ORIG_NO_SRCH_LINES;
			SR_X_POS		= ORIG_SR_X_POS;
			SR_Y_POS		= ORIG_SR_Y_POS;
		}
        else
        {
            /* 
			 * No more records found so get previous
             * record as we've moved one too far now
			 */
            cc = find_rec (cuin2, &cuin2_rec, PREVIOUS, "r");
            if (cc)
                sys_err ("Error in cuin during DBFIND PREVIOUS.", cc, PNAME);
        }

		cumr_rec.hhcu_hash	=	cuin2_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		
		strcpy (local_rec.dbt_no, cumr_rec.dbt_no);
		strcpy (local_rec.searchOnInvoice, "        ");
		DSP_FLD ("invoiceSearch");		
		cc = spec_valid (label ("customer"));
		if (cc)
			skip_entry = goto_field (field, label ("customer"));
		
		abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Number. 
	 */
	if (LCHECK ("customer"))
	{
		abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");

		skip_entry = 0;
		if (dflt_used)
		{
			cumr_rec.hhcu_hash	=	0L;
			FLD ("invoiceSearch")	=	YES;
			return (EXIT_SUCCESS);
		}
		else
			FLD ("invoiceSearch")	=	NA;

		/*
		 * Allocation of unallocated receipts. 
		 */
		if (processUnallocated && last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);

		/*
		 * Search on customer. 
		 */
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Read cumr record. 
		 */
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, (processUnallocated) ? "r" : "w");
		if (cc == -1)
		{
			print_mess (ML (mlStdMess279));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cc)
		{
			abc_unlock (cumr);
			sprintf (err_str, ML (mlStdMess021));
			if (!envVar.dbFind)
			{
				abc_selfield (cumr, "cumr_id_no3");
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (!cc)
					sprintf (err_str,ML (mlDbMess241), local_rec.dbt_no);
				
				abc_selfield (cumr, "cumr_id_no");
			}
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Head Office processing only ? 
		 */
		if (cumr_rec.ho_dbt_hash > 0L && envVar.dbHoOnly)
		{
			sprintf (err_str, ML (mlDbMess014), local_rec.dbt_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Get currency description. 
		 */
		cc = FindPocr (comm_rec.co_no, cumr_rec.curr_code, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		/*
		 * Allocation of unallocated receipts. 
		 */
		if (processUnallocated)
		{
			/*
			 * Get payment details. 
			 */
			GetCheques (TRUE, cumr_rec.hhcu_hash);
			if (cumr_rec.ho_dbt_hash > 0L)
				GetCheques (FALSE, cumr_rec.ho_dbt_hash);

			cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
			cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
			while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
			{
				GetCheques (FALSE, cumr2_rec.hhcu_hash);
				cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
			}
			/*
			 * Display prompts. 
			 */
			sprintf (local_rec.oamt_prmt, 
					ML ("Receipt Amount (%-3.3s): "),
					 cumr_rec.curr_code);

			sprintf (local_rec.unalloc_prmt, 
					 ML ("Unallocated Amt(%-3.3s): "),
					 cumr_rec.curr_code);

			display_prmpt (label ("orec_amt2"));
			display_prmpt (label ("unalloc_amt"));

			DSP_FLD ("name2");

			sprintf (local_rec.bk_curr_desc, "%-40.40s", pocrRec.description);
			exchRate.localToCustomer = pocrRec.ex1_factor;

			DSP_FLD ("bk_curr2");
			DSP_FLD ("bk_curr_desc2");

			return (EXIT_SUCCESS);
		}

		/*
		 * Read exchange record for Customer currency at current Bank. 
		 */
		if (envVar.MultiCurrency)
		{
			strcpy (bkcr_rec.co_no, comm_rec.co_no);
			strcpy (bkcr_rec.bank_id, crbk_rec.bank_id);
			strcpy (bkcr_rec.curr_code, cumr_rec.curr_code);
			cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			exchRate.bankToCustomer = bkcr_rec.ex1_factor;
		}
		else
			exchRate.bankToCustomer = pocrRec.ex1_factor;

		/*
		 * Set up prompts. 
		 */
		sprintf (local_rec.oamt_prmt,
				 ML ("Receipt Amount (%s): "), 
				 cumr_rec.curr_code);

		sprintf (local_rec.odis_prmt,
				 ML ("Discount Amount (%s): "), 
				 cumr_rec.curr_code);

		sprintf (local_rec.bk_ex_prmt,
				 ML ("Bank exch rate (%s): "), 
				 cumr_rec.curr_code);

		sprintf (local_rec.lett_prmt,
				 ML ("Remaining Amt (%s) : "), 
				 cumr_rec.curr_code);

		display_prmpt (label ("orec_amt"));
		display_prmpt (label ("orec_dis"));
		display_prmpt (label ("bk_exch_rate"));
		display_prmpt (label ("lett_amt"));

		exchRate.localToCustomer = exchRate.bankToCustomer / exchRate.bankToLocal;

		DSP_FLD ("exch_rate");
		DSP_FLD ("bk_exch_rate");

		/*
		 * Allow / disallow entry of exchange rate field.
		 * If the customer currency matches local currency then the user is 
		 * NOT allowed  access to the exchange rate field. The opposite is 
		 * true if the customer currency does not match local currency.
		 */
		sprintf (dfltExchRate, "%.4f", exchRate.localToCustomer);
		FLD ("exch_rate") = NO;
		if (!strcmp (cumr_rec.curr_code, envVar.currCode))
			FLD ("exch_rate") = NA;

		GetCheques (TRUE, cumr_rec.hhcu_hash);
		if (cumr_rec.ho_dbt_hash > 0L)
			GetCheques (FALSE, cumr_rec.ho_dbt_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			GetCheques (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}

		newReceipt = TRUE;

		/*
		 * Existing Receipt. 
		 */
		if (!envVar.dbAutoRec || 
			strcmp (local_rec.rec_no, "NEW RCPT"))
		{
			cc = LoadReceipt ();
			if (cc)
			{
				int		allowProcessing = FALSE;

				switch (cc)
				{
				case 1:
					sprintf (err_str, ML (mlDbMess017),
							 local_rec.rec_no, crbk_rec.bank_id);
					break;

				case 2:
					allowProcessing = TRUE;
					if (!processDishonoured)
					{
						if (bldt_rec.reconcile [0] == 'Y')
						{
							strcpy (err_str,ML (mlDbMess236));
							allowProcessing = FALSE;
						}

						if (bldt_rec.presented [0] == 'Y')
							strcpy (err_str,ML (mlDbMess237));
					}
					break;

				case 3:
					sprintf (err_str,ML (mlDbMess238), local_rec.rec_no);
					break;
				}

				if (!allowProcessing)
				{
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
	
					restart = TRUE;
					return (EXIT_SUCCESS);
				}
			}

			FLD ("receipt")  = NA;
			FLD ("rec_date") = NA;

			if (processDishonoured)
			{
				/*
				 * Receipt must exist for customer. 
				 */
				if (newReceipt)
				{
					print_mess (ML (mlDbMess239));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				/*
				 * Cannot dishonour a Dishonoured Receipt. 
				 */
				if (cuhd_rec.dishonoured [0] == 'Y')
				{
					print_mess (ML (mlDbMess242));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (!newReceipt && !SPLIT_PAYMENT)
			{
				if (multipleReceipts)
				{
					FLD ("rec_type")   = NA;
				}

				scn_display (S_MAIN);
				entry_exit = TRUE;
				return (EXIT_SUCCESS);
			}
		}

		/*
		 * Check currency for new customer against 
		 * currency of customer for existing cuhd.
		 */
		if (SPLIT_PAYMENT)
		{
			if (strcmp (cumr_rec.curr_code, splitCurr))
			{
				print_mess (ML (mlDbMess243));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			/*
			 * Set fields that can not be changed for Split Payments. 
			 */
			FLD ("bank_id")   = NA;
			FLD ("receipt")   = NA;
			FLD ("rec_date")  = NA;
			FLD ("rec_type")  = NA;
			FLD ("due_date")  = NA;
			FLD ("ref")       = NA;
			FLD ("customer")    = NA;
			FLD ("exch_rate") = NA;

			/*
			 * For split payments, load some details from      
			 * existing receipt and goto the Allocation field.
			 */
			strcpy (local_rec.rec_type, cuhd2_rec.rec_type);
			switch (local_rec.rec_type [0])
			{
			case 'A':
				strcpy (local_rec.rec_type_desc, ML ("A-Cash       "));
				break;

			case 'B':
				strcpy (local_rec.rec_type_desc, ML ("Bank Draft   "));
				break;

			case 'C':
				strcpy (local_rec.rec_type_desc, ML ("Cheque       "));
				break;

			case 'D':
				strcpy (local_rec.rec_type_desc, ML ("Direct Credit"));
				break;
			}

			strcpy (local_rec.narrative,   cuhd2_rec.narrative);
			strcpy (local_rec.alt_drawer,  cuhd2_rec.alt_drawer);
			strcpy (local_rec.db_bank,     cuhd2_rec.db_bank);
			strcpy (local_rec.db_branch,   cuhd2_rec.db_branch);

			strcpy (headDetails.altDrawer, cuhd2_rec.alt_drawer);
			strcpy (headDetails.dbBank,    cuhd2_rec.db_bank);
			strcpy (headDetails.dbBranch,  cuhd2_rec.db_branch);

			local_rec.rec_date  = cuhd2_rec.date_payment;
			local_rec.due_date  = cuhd2_rec.due_date;

			exchRate.bankToCustomer  = cuhd2_rec.bank_exch;
			exchRate.bankToLocal   = cuhd2_rec.bank_lcl_exch;
			exchRate.localToCustomer = exchRate.bankToCustomer / exchRate.bankToLocal;

			local_rec.bank_chg  = 0.00;
			local_rec.rec_amt   = 0.00;
			local_rec.rec_oamt  = 0.00;
			local_rec.rec_lamt  = 0.00;
			local_rec.rec_odis  = 0.00;
			local_rec.rec_ldis  = 0.00;
			local_rec.rec_gross = 0.00;

			scn_display (S_MAIN);
			skip_entry = goto_field (field, label ("orec_amt"));
		}
		else
		{
			/*
			 * New receipt AND System Generated Numbers. 
			 */
			if (newReceipt && envVar.dbAutoRec)
			{
				strcpy (local_rec.rec_no, "NEW RCPT");
				DSP_FLD ("receipt");
			}

			if (useAltDate && strcmp (comm_rec.est_no, cumr_rec.est_no))
				altDate = ReadBranchDate (cumr_rec.est_no);
			else
				altDate = comm_rec.dbt_date;

			strcpy (local_rec.db_bank, cumr_rec.bank_code);
			strcpy (local_rec.db_branch, cumr_rec.branch_code);

			DSP_FLD ("name");
			DSP_FLD ("bank");
			DSP_FLD ("branch");
		}
		DSP_FLD ("customer");		

		return (EXIT_SUCCESS);
	}

	/*
	 * Alternate drawer details. 
	 */
	if (LCHECK ("alt_drawer"))
	{
		if (strcmp (local_rec.db_bank, "   "))
			skip_entry = goto_field (field, label ("orec_amt"));

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Overseas receipt amount. 
	 */
	if (LCHECK ("orec_amt"))
	{
		if (FLD ("orec_amt") == NA || cur_screen == 2)
			return (EXIT_SUCCESS);

		local_rec.rec_amt = local_rec.rec_oamt;
		if (exchRate.bankToCustomer != 0.00)
			local_rec.rec_amt /= exchRate.bankToCustomer;

		local_rec.rec_lamt = local_rec.rec_oamt;
		if (exchRate.localToCustomer != 0.00)
			local_rec.rec_lamt = no_dec (local_rec.rec_lamt
										 / exchRate.localToCustomer);

		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (exchRate.localToCustomer != 0.00)
		{
			local_rec.rec_gross = no_dec (local_rec.rec_gross
										  / exchRate.localToCustomer);
			local_rec.rec_ldis = no_dec (local_rec.rec_ldis
										 / exchRate.localToCustomer);
		}

		DSP_FLD ("rec_gross");
		DSP_FLD ("rec_amt");
		DSP_FLD ("lrec_amt");

		if (prog_status != ENTRY)
		{
			if (local_rec.alloc [0] == 'A')
			{
				rv_pr (ML (mlDbMess012), 2, 21, 45);
				line_at (21,0,132);
				sleep (sleepTime);
				clear_mess ();
				if (local_rec.alloc [0] == 'A')
					ReadAmount (TRUE);
				else
					ReadAmount (FALSE);
			}

			/*
			 * Calculate amount remaining on letter of credit.       
			 */
			CalcLetterAmt ();
			DSP_FLD ("lett_amt");
		}
	}

	/*
	 * Validate Customer Number. 
	 */
	if (LCHECK ("orec_dis"))
	{
		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (exchRate.localToCustomer != 0.00)
		{
			local_rec.rec_gross = no_dec (local_rec.rec_gross
										  / exchRate.localToCustomer);
			local_rec.rec_ldis = no_dec (local_rec.rec_ldis
										 / exchRate.localToCustomer);
		}
		DSP_FLD ("rec_gross");
	}

	/*
	 * Validate receipt Date. 
	 */
	if (LCHECK ("rec_date"))
	{
		if (processDishonoured)
			return (EXIT_SUCCESS);

		FLD ("rec_type") = YES;

		if (dflt_used)
			local_rec.rec_date = comm_rec.dbt_date;

		DSP_FLD ("rec_date");

		max_fdate = MonthEnd (comm_rec.dbt_date) + 1L;
		if (local_rec.rec_date > MonthEnd (max_fdate))
			return print_err (ML (mlDbMess020));

		if (local_rec.rec_date < MonthStart (comm_rec.dbt_date))
			return print_err (ML (mlDbMess019));

		if (local_rec.rec_date > MonthEnd (comm_rec.dbt_date) &&
			fwdMessage == TRUE)
		{
			i = prmptmsg (ML (mlDbMess097), "YyNn", 1, 2);
			if (i == 'y' || i == 'Y')
			{
				i = prmptmsg (ML (mlDbMess098), "YyNn", 1, 2);
				fwdMessage = (i == 'y' || i == 'Y') ? FALSE
					: TRUE;
			}
			else
			{
				line_at (2,1,131);
				return (EXIT_FAILURE);
			}
			line_at (2,1,131);
		}

		if (local_rec.rec_date > MonthEnd (comm_rec.dbt_date))
		{
			/*
			 * Only Cheques may be forward dated. 
			 */
			strcpy (local_rec.rec_type, "C");
			strcpy (local_rec.rec_type_desc, "Cheque");
			DSP_FLD ("rec_type");
			DSP_FLD ("rec_type_desc");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate receipt type. 
	 */
	if (LCHECK ("rec_type"))
	{
		FLD ("due_date") = NA;
		FLD ("ref") = NA;
		FLD ("lett_crd") = NA;

		if (local_rec.rec_date > MonthEnd (comm_rec.dbt_date)
				&& !BANK_DRAFT && !CHQ_PAYMNT)
		{
			return print_err (ML ("Only Bank Draft and Cheque Payment are allowed for forward payment."));
		}

		if (FLD ("rec_type") == NA)
			return (EXIT_SUCCESS);

		if (CASH_PYMNT)
			strcpy (local_rec.rec_type_desc, ML ("A-Cash       "));

		if (BANK_DRAFT)
		{
			strcpy (local_rec.rec_type_desc, ML ("Bank Draft   "));
			FLD ("due_date") = YES;
			FLD ("ref") = YES;
			FLD ("lett_crd") = YES;
		}

		if (CHQ_PAYMNT)
			strcpy (local_rec.rec_type_desc, ML ("Cheque       "));

		if (DIRECT_CRD)
		{
			strcpy (local_rec.rec_type_desc, ML ("Direct Credit"));
			FLD ("ref") = YES;
		}

		DSP_FLD ("rec_type_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Due Date. 
	 */
	if (LCHECK ("due_date"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.due_date = local_rec.lsystemDate;

		if (local_rec.due_date < local_rec.lsystemDate)
		{
			print_mess (ML ("Date less than current date"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("due_date");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Allocation type. 
	 */
	if (LCHECK ("lett_crd"))
	{
		validLetter = FALSE;
		if (FLD ("lett_crd") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.lett_crd, "               ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCuch (temp_str);
			return (EXIT_SUCCESS);
		}

		cuch_rec.hhcu_hash = cumr_rec.hhcu_hash;
		sprintf (cuch_rec.letter_no, "%-15.15s", local_rec.lett_crd);
		cc = find_rec (cuch, &cuch_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess079));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Letter must be for the correct bank. 
		 */
		if (strcmp (cuch_rec.bank_id, crbk_rec.bank_id))
		{
			print_mess (ML (mlDbMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check expiry date. 
		 */
		if (cuch_rec.expiry_date < local_rec.rec_date)
		{
			c = prmptmsg (ML (mlDbMess139), "YyNn", 20, 23);
			clear_mess ();
			if (c == 'N' || c == 'n')
				return (EXIT_FAILURE);
		}

		/*
		 * Calculate amount remaining on letter of credit.       
		 */
		CalcLetterAmt ();

		/*
		 * Amount of letter of credit exceeded. 
		 */
		if (local_rec.lett_amt < 0.00)
		{
			sprintf (err_str,ML (mlDbMess022),lclip (comma_fmt (DOLLARS (cuch_rec.limit), "NNN,NNN,NNN.NN")));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
		}

		DSP_FLD ("lett_amt");

		validLetter = TRUE;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Allocation type. 
	 */
	if (LCHECK ("alloc") || LCHECK ("alloc2"))
	{
		FLD ("selmonth") = NA;
		FLD ("selmonth2") = NA;

		if (local_rec.alloc [0] == 'U')
		{
			strcpy (local_rec.alloc_desc, ML ("Unallocated             "));

			/*
			 * Add dummy cuin record. 
			 */
			abc_selfield (cuin, "cuin_id_no2");
			AddInvoice (TRUE, "1");

			/*
			 * Add line to tabular screen. 
			 */
			scn_set (S_DTLS);

			local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;
			strcpy (local_rec.l_inv_no, cuin_rec.inv_no);
			strcpy (local_rec.l_inv_type, "INV");

			local_rec.l_inv_due 	= cuin_rec.due_date;
			local_rec.l_inv_bal 	= 0.00;
			local_rec.l_rec_oamt 	= local_rec.rec_oamt + local_rec.rec_odis;
			local_rec.l_inv_exch 	= cuin_rec.exch_rate;
			strcpy (local_rec.l_curr, cuin_rec.currency);
			strcpy (local_rec.l_fx, cuin_rec.er_fixed);
			local_rec.l_loc_amt 	= 0.00;

			if (exchRate.localToCustomer != 0.00)
				local_rec.l_rec_lamt = local_rec.l_rec_oamt / exchRate.localToCustomer;
			lineTotal [lcount [S_DTLS]].payForeign = local_rec.l_rec_oamt;
			lineTotal [lcount [S_DTLS]].payLocal   = local_rec.l_rec_lamt;
			lineTotal [lcount [S_DTLS]].invLocal   = local_rec.l_loc_amt;

			local_rec.mon_inv = cuin_rec.date_of_inv;
			putval (lcount [S_DTLS]++);

			scn_set (S_MAIN);

			return (EXIT_SUCCESS);
		}

		if (local_rec.alloc [0] == 'A')
		{
			strcpy (local_rec.alloc_desc, ML ("Amount receipt allocation"));
			ReadAmount (TRUE);
		}
		if (local_rec.alloc [0] == 'R')
		{
			strcpy (local_rec.alloc_desc, ML ("Reverse date order.     "));
			ReadAmount (FALSE);
		}
		if (local_rec.alloc [0] == 'S')
			strcpy (local_rec.alloc_desc, ML ("Selective document entry"));

		if (local_rec.alloc [0] == 'M')
		{
			strcpy (local_rec.alloc_desc, ML ("Month selection.        "));
			if (processUnallocated)
				FLD ("selmonth2") = YES;
			else
				FLD ("selmonth") = YES;
		}

		if (processUnallocated)
			DSP_FLD ("alloc_desc2");
		else
			DSP_FLD ("alloc_desc");

		return (EXIT_SUCCESS);
	}
	/*
	 * Load Invoice for Selected Month. 
	 */
	if (LCHECK ("selmonth") || LCHECK ("selmonth2"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		abc_selfield (cuin, cumr_rec.ho_dbt_hash ? "cuin_cron3" 
													: "cuin_ho_cron3");

		selectedYear = (local_rec.sel_month <= currentMonth) ? currentYear : currentYear - 1;

		ReadInvoices (TRUE, 
                      cumr_rec.hhcu_hash,
                      cumr_rec.ho_dbt_hash);
		if (tmpLineCnt >= MAXLINES)
		{
			print_mess (ML (mlDbMess036));
			sleep (sleepTime);
			clear_mess ();
		}
		lcount [S_DTLS] = tmpLineCnt;
		scn_set (S_MAIN);
		if (prog_status == ENTRY)
			skip_entry = 1;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Invoice Number. 
	 */
	if (LCHECK ("l_invoice"))
	{
		abc_selfield (cuin, cumr_rec.ho_dbt_hash ? "cuin_id_no2"
					  : "cuin_ho_id");

		if (!strcmp (local_rec.l_inv_no, "kill    ") ||
			!strcmp (local_rec.l_inv_no, "KILL    "))
		{
			if (lcount [S_DTLS] == 0)
			{
				print_mess (ML (mlStdMess080));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			lcount [S_DTLS] = line_cnt;
			heading (S_DTLS);

			if (lcount [S_DTLS] % TABLINES)
			{
				line_cnt = lcount [S_DTLS] - 1;
				scn_display (S_DTLS);
			}
			else
			{
				line_cnt = lcount [S_DTLS];
				/*blank_display (S_DTLS); */
                blank_display ();
			}
			DisplayTotals ();
			return (EXIT_SUCCESS);
		}

		if (!strncmp (local_rec.l_inv_no, "del", 3) ||
			!strncmp (local_rec.l_inv_no, "DEL", 3))
		{
			if (!strcmp (local_rec.l_inv_no, "del     ") ||
				!strcmp (local_rec.l_inv_no, "DEL     "))
				i_del = 1;
			else
				i_del = atoi (local_rec.l_inv_no + 3);

			for (i = 0; i < i_del; i++)
			{
				if (DeleteLine (i_del - i))
					return (EXIT_FAILURE);
			}

			heading (S_DTLS);
			scn_display (S_DTLS);
			DisplayTotals ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchLineInvoice (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Check that invoice is not already in the tabular screen. 
		 */
		if (DuplicateInvNo (local_rec.l_inv_no))
		{
			sprintf (err_str, ML (mlDbMess244), local_rec.l_inv_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		if (local_rec.sel_month <= currentMonth)
			selectedYear = currentYear;

		if (cumr_rec.ho_dbt_hash)
			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		else
			cuin_rec.ho_hash = cumr_rec.hhcu_hash;

		strcpy (cuin_rec.est, (!envVar.dbCo) ? comm_rec.est_no :
				cumr_rec.est_no);
		strcpy (cuin_rec.inv_no, zero_pad (local_rec.l_inv_no, INV_NO_LEN));

		invoiceAdded = FALSE;

		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
		{
			if (!strcmp (cuin_rec.inv_no, "DEP     ") ||
				!strcmp (cuin_rec.inv_no, "dep     "))
			{
				/*
				 * Allocation of unallocated receipts. 
				 */
				if (processUnallocated)
				{
					print_mess (ML (mlDbMess039));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				AddInvoice (TRUE, "1");
				DSP_FLD ("l_invoice");
   
                /*
                 * Not an allocation so calculate the deposit total for user. 
                 */
                if (!processUnallocated)
                {
                    ActTotal = local_rec.rec_oamt + local_rec.rec_odis;

                    DepTotal = 0.00;
                    no_lines = (prog_status == ENTRY) ? line_cnt :
                                                        lcount [3] - 1;
                    for (i = 0;i <= no_lines; i++)
                       DepTotal += lineTotal [i].payForeign;
                    
                    local_rec.inv_balance = ActTotal - DepTotal;
                    skip_entry = goto_field (field, label ("pay_terms"));
                    DepositFlag = TRUE;
                }			

            }
			else
			{
				sprintf (err_str, ML (mlDbMess023),local_rec.l_inv_no);

				i = prmptmsg (err_str, "YyNn", 1, 2);
				if (i == 'y' || i == 'Y')
				{
					move (1, 2);
					cl_line ();
					i = prmptmsg (ML (mlDbMess024),"IiCc", 1, 2);
					if (i == 'I' || i == 'i')
						AddInvoice (FALSE, "1");
					else
						AddInvoice (FALSE, "2");
				}
				else
				{
					move (1, 2);
					cl_line ();
					return (EXIT_FAILURE);
				}
			}
		}
		local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;

		if (envVar.feInstall)
		{
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			cc = find_rec (feln, &feln_rec, EQUAL, "r");
			if (!cc)
			{
				fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
				cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, fehr, "DBFIND");

				if (feln_rec.hhfe_hash != fehr_rec.hhfe_hash)
				{
					sprintf (err_str, ML (mlDbMess025), fehr2_rec.cont_no);
					i = prmptmsg (err_str, "YyNn", 25, 20);
					line_at (20,0,132);
					if (i != 'Y' && i != 'y')
						return (EXIT_FAILURE);
				}
				strcpy (local_rec.l_fe_cont, fehr2_rec.cont_no);
			}
			else
				strcpy (local_rec.l_fe_cont, "      ");
		}

		local_rec.mon_inv = cuin_rec.date_of_inv;

        if (!DepositFlag) 
		   ChequeInvBal ();
		local_rec.l_inv_due = cuin_rec.due_date;
		local_rec.l_inv_bal = local_rec.inv_balance;
		local_rec.l_rec_oamt = local_rec.inv_balance;
		local_rec.l_inv_exch = cuin_rec.exch_rate;
		local_rec.l_rec_exch = exchRate.localToCustomer;
		strcpy (local_rec.l_curr, cuin_rec.currency);
		strcpy (local_rec.l_fx, cuin_rec.er_fixed);
		if (cuin_rec.exch_rate != 0.00)
		{
			local_rec.l_loc_amt = local_rec.inv_balance;
			local_rec.l_loc_amt /= cuin_rec.exch_rate;
		}
		if (local_rec.l_rec_exch != 0.00)
		{
			local_rec.l_rec_lamt = local_rec.inv_balance;
			local_rec.l_rec_lamt /= local_rec.l_rec_exch;
		}

		strcpy (lineTotal [line_cnt].invNo, cuin_rec.inv_no);
		lineTotal [line_cnt].payForeign = local_rec.l_rec_oamt;
		lineTotal [line_cnt].payLocal   = local_rec.l_rec_lamt;
		lineTotal [line_cnt].invLocal   = local_rec.l_loc_amt;

		if (cuin_rec.type [0] == '1')
			strcpy (local_rec.l_inv_type, "INV");

		if (cuin_rec.type [0] == '2')
			strcpy (local_rec.l_inv_type, "CRD");

		if (cuin_rec.type [0] == '3')
			strcpy (local_rec.l_inv_type, "JNL");

		DSP_FLD ("l_type");
		DSP_FLD ("l_invamt");
		DSP_FLD ("l_inv_exch");
		DSP_FLD ("l_curr");
		DSP_FLD ("l_fx");
		DSP_FLD ("l_locamt");
		DSP_FLD ("l_rec_oamt");
		DSP_FLD ("l_rec_exch");
		DSP_FLD ("l_rec_lamt");
		if (envVar.feInstall)
			DSP_FLD ("l_fe_cont");
		else
			DSP_FLD ("l_rec_exch");

		putval (line_cnt);
		DisplayTotals ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Payment Amount 
	 */
	if (LCHECK ("l_rec_oamt"))
	{
		if (dflt_used)
			local_rec.l_rec_oamt = local_rec.l_inv_bal;

		if (exchRate.localToCustomer != 0.00)
		{
			local_rec.l_rec_lamt = local_rec.l_rec_oamt;
			local_rec.l_rec_lamt = no_dec (local_rec.l_rec_lamt
										   / exchRate.localToCustomer);
		}
		lineTotal [line_cnt].payForeign = local_rec.l_rec_oamt;
		lineTotal [line_cnt].payLocal   = local_rec.l_rec_lamt;
		lineTotal [line_cnt].invLocal   = local_rec.l_loc_amt;

		DSP_FLD ("l_rec_oamt");
		DSP_FLD ("l_rec_lamt");
		putval (line_cnt);
		DisplayTotals ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * VALIDATION SUPPORT ROUTINES.                      
 */

/*
 * Validate unallocated receipt number for allocation.       
 */
int
ValidAlloc (void)
{
	if (SRCH_KEY)
	{
		SrchUnallocated (temp_str);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate that receipt exists. 
	 */
	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, zero_pad (local_rec.rec_no, INV_NO_LEN));
	cuhd_rec.index_date = 0L;
	cc = find_rec (cuhd2, &cuhd_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess081));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (WarnMultiple ())
		return (EXIT_FAILURE);

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", cuhd_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str, ML (mlDbMess017),cuhd_rec.receipt_no,cuhd_rec.bank_id);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Receipt must be linked to at least 1 deposit which is not fully allocated
	 */
	if (!FindDeposits (FALSE, TRUE))
	{
		print_mess (ML (mlDbMess040));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	local_rec.rec_date = cuhd_rec.date_payment;
	exchRate.localToCustomer = cuhd_rec.tot_amt_paid / cuhd_rec.loc_amt_paid;
	local_rec.rec_oamt = cuhd_rec.tot_amt_paid + cuhd_rec.disc_given;
	local_rec.rec_lamt = cuhd_rec.loc_amt_paid + cuhd_rec.loc_disc_give;

	DSP_FLD ("exch_rate2");
	DSP_FLD ("orec_amt2");
	DSP_FLD ("lrec_amt2");
	DSP_FLD ("unalloc_amt");

	return (EXIT_SUCCESS);
}

/*
 * If processing dishonoured cheques        
 * then warn user if there are multiple    
 * payments for the receipt number entered.
 */
int
WarnMultiple (void)
{
	int		numRecs = 0;

	strcpy (cuhd3_rec.receipt_no, local_rec.rec_no);
	for (cc = find_rec (cuhd3, &cuhd3_rec, GTEQ, "r");
		 !cc && !strcmp (cuhd3_rec.receipt_no, local_rec.rec_no);
		 cc = find_rec (cuhd3, &cuhd3_rec, NEXT, "r"))
	{
		/*
		 * Cannot dishonour a Dishonoured Receipt. 
		 */
		if (cuhd3_rec.rec_type [0] == 'I' ||
			 (!processDishonoured && cuhd3_rec.dishonoured [0] == 'Y'))
		{
			print_mess (ML (mlDbMess242));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		numRecs++;
	}

	if (numRecs > 1)
	{
		multipleReceipts = TRUE;
		print_mess (ML (mlDbMess245));
		sleep (sleepTime);
		clear_mess ();
	}

	return (EXIT_SUCCESS);
}

/*
 * Calculate amount remaining on letter of credit.
 */
void
CalcLetterAmt (void)
{
	double	totalDrawOff;

	if (!validLetter)
		return;

	totalDrawOff = 0.00;
	cucd_rec.hhch_hash = cuch_rec.hhch_hash;
	cucd_rec.hhcp_hash = 0L;
	cc = find_rec (cucd, &cucd_rec, GTEQ, "r");
	while (!cc && cucd_rec.hhch_hash == cuch_rec.hhch_hash)
	{
		totalDrawOff += cucd_rec.amount;

		cc = find_rec (cucd, &cucd_rec, NEXT, "r");
	}

	/*
	 * Add in current receipt amount for new receipts. 
	 */
	if (newReceipt)
		totalDrawOff += local_rec.rec_oamt;
	else
		totalDrawOff += (local_rec.rec_oamt - headDetails.bkRecAmt);

	local_rec.lett_amt = cuch_rec.limit - totalDrawOff;
}

/*
 * Delete line from tabular screen. 
 */
int
DeleteLine (
 int	del_no)
{
	int i;
	int this_page = line_cnt / TABLINES;
	int	delta = 1;

	/*
	 * entry									
	 */
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * no lines to delete or excessive lines to delete 
	 */
	if ( (lcount [S_DTLS] <= 0) ||
		 ( (line_cnt + del_no) > lcount [S_DTLS]))
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	print_at (2,0, ML (mlStdMess035));

	for (i = line_cnt; line_cnt < lcount [S_DTLS] - delta; line_cnt++)
	{
		getval (line_cnt + delta);
		putval (line_cnt);

		strcpy (lineTotal [line_cnt].invNo, lineTotal [line_cnt + delta].invNo);
		lineTotal [line_cnt].payForeign = lineTotal [line_cnt + delta].payForeign;
		lineTotal [line_cnt].payLocal   = lineTotal [line_cnt + delta].payLocal;
		lineTotal [line_cnt].invLocal   = lineTotal [line_cnt + delta].invLocal;

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	while (line_cnt < lcount [S_DTLS])
	{
		local_rec.l_hhcu_hash = 0L;
		strcpy (local_rec.l_inv_no, "        ");
		strcpy (local_rec.l_curr, "   ");
		strcpy (local_rec.l_inv_type, "    ");
		strcpy (local_rec.l_fx, " ");
		local_rec.l_inv_bal = 0.00;
		local_rec.l_inv_exch = 0.00;
		local_rec.l_loc_amt = 0.00;
		local_rec.l_rec_oamt = 0.00;
		local_rec.l_rec_lamt = 0.00;

		strcpy (lineTotal [line_cnt].invNo, "        ");
		lineTotal [line_cnt].payForeign = 0.00;
		lineTotal [line_cnt].payLocal   = 0.00;
		lineTotal [line_cnt].invLocal   = 0.00;

		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			blank_display ();

		line_cnt++;
	}

	print_at (2, 0,"                          ");
	fflush (stdout);

	/*
	 * blank last line - if required			
	 */
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	lcount [S_DTLS] -= delta;

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Check for duplicate invoice numbers. 
 */
int
DuplicateInvNo (
	char	*invNo)
{
	int		i;
	int		numLines;

	numLines = (prog_status == ENTRY) ? line_cnt : lcount [S_DTLS] - 1;

	for (i = 0; i <= numLines; i++)
	{
		if (!strcmp (lineTotal [i].invNo, invNo))
			return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
/*
 * SEARCHES                             
 */

/*
 * Search for unallocated receipts for current customer 
 */
void
SrchUnallocated (
	char	*keyVal)
{
	_work_open (20, 0, 30);

	sprintf (err_str, "#Value Unallocated (%s) ", cumr_rec.curr_code);

	save_rec ("#Receipt  / Deposit", err_str);

	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuhd_rec.receipt_no, "%-8.8s", keyVal);
	cuhd_rec.index_date = 0L;
	cc = find_rec (cuhd2, &cuhd_rec, GTEQ, "r");
	while (!cc &&
		   cuhd_rec.hhcu_hash == cumr_rec.hhcu_hash &&
		   !strncmp (cuhd_rec.receipt_no, keyVal, strlen (keyVal)))
	{
		/*
	     * Find deposits for this receipt. 
	     */
		if (cuhd_rec.index_date <= 0L)
			FindDeposits (TRUE, FALSE);

		cc = find_rec (cuhd2, &cuhd_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	/*
	 * Extract default deposit. 
	 */
	sprintf (dfltDeposit, "%-8.8s", temp_str + 11);
	temp_str [8] = '\0';
	return;
}

/*
 * Search for normal receipts for current bank code.     
 */
void
SrchReceipt (
	char	*keyVal)
{
	_work_open (8,0,40);

	save_rec ("#Receipt No", "#Customer");

	sprintf (cuhd_rec.receipt_no, "%-8.8s", keyVal);

	for (cc = find_rec (cuhd3, &cuhd_rec, GTEQ, "r");
		 !cc &&
		  !strncmp (cuhd_rec.receipt_no, keyVal, strlen (keyVal));
		 cc = find_rec (cuhd3, &cuhd_rec, NEXT, "r"))
	{
		/*
		 * Ignore receipts for other banks. 
		 */
		if (strcmp (cuhd_rec.bank_id, crbk_rec.bank_id))
			continue;

		/*
		 * Look up cumr record. 
		 */
		cumr3_rec.hhcu_hash	=	cuhd_rec.hhcu_hash;
		cc = find_rec (cumr3, &cumr3_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cumr3, "DBFIND");

		sprintf (err_str, "%-6.6s  %-40.40s", cumr3_rec.dbt_no,
				 							  cumr3_rec.dbt_name);

		cc = save_rec (cuhd_rec.receipt_no, err_str);
		if (cc)
			break;
	}

	cc = disp_srch ();
	work_close ();
}

/*
 * Search for deposits for a receipt. 
 */
void
SrchDeposit (
	char	*keyVal)
{
	_work_open (8,0,30);
	sprintf (err_str, "#Value Unallocated (%s) ", cumr_rec.curr_code);
	save_rec ("#Deposit", err_str);

	/*
	 * Find deposits for current receipt. 
	 */
	FindDeposits (FALSE, FALSE);

	cc = disp_srch ();
	work_close ();
}

/*
 * Letter of credit search. 
 */
void
SrchCuch (
	char	*keyVal)
{
	_work_open (16,0,0);
	save_rec ("#Letter No ", "# ");

	cuch_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuch_rec.letter_no, "%-15.15s", keyVal);
	cc = find_rec (cuch, &cuch_rec, GTEQ, "r");
	while (!cc &&
		   cuch_rec.hhcu_hash == cumr_rec.hhcu_hash &&
		   !strncmp (cuch_rec.letter_no, keyVal, strlen (keyVal)))
	{
		cc = save_rec (cuch_rec.letter_no, " ");
		if (cc)
			break;
		cc = find_rec (cuch, &cuch_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search routine for Creditors Bank File. 
 */
void
SrchCrbk (
	char	*keyVal)
{
	_work_open (5,0,40);

	save_rec ("#Bank ", "#Bank Name ");

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, keyVal);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (crbk_rec.bank_id, keyVal, strlen (keyVal)) &&
		   !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;

		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc)
		file_err (cc, crbk, "DBFIND");
}

/*
 * Invoice Search 
 */
void
SrchLineInvoice (
	char	*keyVal)
{
	_work_open (8,0,40);
	if (cumr_rec.ho_dbt_hash)
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	else
		cuin_rec.ho_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");
	strcpy (cuin_rec.inv_no, keyVal);
	save_rec ("#Inv No", "#Customer (Acronym)Date Inv / Amount Owing.   ");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc &&
		   !strncmp (cuin_rec.inv_no, keyVal, strlen (keyVal)) &&
		  (cumr_rec.ho_dbt_hash ? cuin_rec.hhcu_hash
			: cuin_rec.ho_hash) == cumr_rec.hhcu_hash)
	{
		ChequeInvBal ();
		if (local_rec.inv_balance != 0.00)
		{
			if (cumr3_rec.hhcu_hash != cuin_rec.hhcu_hash)
			{
				cumr3_rec.hhcu_hash	=	cuin_rec.hhcu_hash;
				cc = find_rec (cumr3, &cumr3_rec, EQUAL, "r");
				if (cc)
					file_err (cc, cumr3, "DBFIND");
			}

			sprintf (err_str, "%s (%-9.9s)%s / %.2f",
					 cumr3_rec.dbt_no,
					 cumr3_rec.dbt_acronym,
					 DateToString (cuin_rec.date_of_inv),
					 DOLLARS (local_rec.inv_balance));

			cc = save_rec (cuin_rec.inv_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (cumr_rec.ho_dbt_hash)
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	else
		cuin_rec.ho_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");
	strcpy (cuin_rec.inv_no, temp_str);
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");

	ChequeInvBal ();
}

/*
 * Invoice Search. 
 */
void
SrchInvoice (
	char	*keyVal)
{
	char 	sel_str [21];
	long 	workHash = 0L;

	_work_open (15,8,48);
	cumr_rec.hhcu_hash = 0L;

	strcpy (cuin2_rec.inv_no, keyVal);
	save_rec ("#Invoice No [Hash key]", "#Customer ");
	cc = find_rec (cuin2, &cuin2_rec, GTEQ, "r");
	while (!cc && !strncmp (cuin2_rec.inv_no, keyVal, strlen (keyVal)))
	{
		cumr_rec.hhcu_hash	=	cuin2_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
		{
			sprintf (sel_str, "%8.8s %10ld",
					 cuin2_rec.inv_no, cumr_rec.hhcu_hash);

			sprintf (err_str, "%-6.6s - %-40.40s",
					 cumr_rec.dbt_no, cumr_rec.dbt_name);

			cc = save_rec (sel_str, err_str);
			if (cc)
				break;
		}
		cc = find_rec (cuin2, &cuin2_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		cumr_rec.hhcu_hash = 0L;
		return;
	}

	workHash = atol (temp_str + 9);

	cumr_rec.hhcu_hash	=	workHash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
}

/*
 * Search for fehr records 
 */
void
SrchFehr (
 char*              keyVal)
{
	_work_open (6,0,40);

	save_rec ("#Cont No", "#Bank           ");

	strcpy (fehr_rec.co_no, comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-3.3s", keyVal);
	cc = find_rec (fehr, &fehr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (fehr_rec.co_no, comm_rec.co_no) && 
		   !strncmp (fehr_rec.cont_no, keyVal, strlen (keyVal)))
	{
		if (!strcmp (fehr_rec.curr_code, cumr_rec.curr_code) &&
			!strcmp (fehr_rec.bank_id, crbk_rec.bank_id))
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

	strcpy (fehr_rec.co_no, comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, fehr, "DBFIND");
}

/*
 * ROUTINES ASSOCIATED WITH SEARCHES                           
 */

/*
 * Find deposits for current receipt. 
 */
int
FindDeposits (
	int		save_rec_no,
	int		chk_exists)
{
	int first_dep;
	int dep_found;
	long prev_hhci = 0L;
	double dep_value;
	char amtLeft [16];

	/*
     * Check for unallocated, linked to a DP invoice. 
     */
	dep_found = FALSE;
	first_dep = TRUE;
	dep_value = 0.00;
	cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;
	cudt_rec.hhci_hash = 0L;
	cc = find_rec (cudt2, &cudt_rec, GTEQ, "r");
	while (!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash)
	{
		/*
		 * Change in hhci_hash. 
		 */
		if (prev_hhci != cudt_rec.hhci_hash || first_dep)
		{
			if (!first_dep)
			{
				cuin_rec.hhci_hash	=	prev_hhci;
				cc = find_rec (cuin3, &cuin_rec, EQUAL, "r");
				if (!cc && !strncmp (cuin_rec.inv_no, "DP", 2) &&
					dep_value != 0.00)
				{
					if (chk_exists)
						return (TRUE);

					sprintf (amtLeft, "%8.2f", DOLLARS (dep_value));

					if (save_rec_no)
					{
						sprintf (err_str, "%8.8s / %8.8s",
								 cuhd_rec.receipt_no, cuin_rec.inv_no);
					}
					else
						sprintf (err_str, "%8.8s", cuin_rec.inv_no);
					cc = save_rec (err_str, amtLeft);
					if (cc)
						break;
				}
			}
			prev_hhci = cudt_rec.hhci_hash;
			dep_value = 0.00;
			first_dep = FALSE;
		}
		dep_value += cudt_rec.amt_paid_inv;

		cc = find_rec (cudt2, &cudt_rec, NEXT, "r");
	}

	if (!first_dep)
	{
		cuin_rec.hhci_hash	=	prev_hhci;
		cc = find_rec (cuin3, &cuin_rec, COMPARISON, "r");
		if (!cc && !strncmp (cuin_rec.inv_no, "DP", 2) && dep_value != 0.00)
		{
			if (chk_exists)
				return (TRUE);

			sprintf (amtLeft, "%8.2f", DOLLARS (dep_value));

			if (save_rec_no)
			{
				sprintf (err_str, "%8.8s / %8.8s", cuhd_rec.receipt_no,
						 cuin_rec.inv_no);
			}
			else
				sprintf (err_str, "%8.8s", cuin_rec.inv_no);

			cc = save_rec (err_str, amtLeft);
		}
	}

	return (FALSE);
}

/*
 * Load deposit ? 
 */
void
LoadDeposit (void)
{
	strcpy (fetr_rec.index_by, "C");
	fetr_rec.index_hash = cuin_rec.hhci_hash;
	fetr_rec.hhcp_hash = cudt_rec.hhcp_hash;
	cc = find_rec (fetr, &fetr_rec, EQUAL, "r");
	if (!cc)
	{
		fehr2_rec.hhfe_hash = fetr_rec.hhfe_hash;
		cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, fehr, "DBFIND");
		strcpy (local_rec.fe_cont_no, fehr2_rec.cont_no);
		memcpy (&fehr_rec, &fehr2_rec, sizeof (fehr_rec));
	}
	else
	{
		strcpy (local_rec.fe_cont_no, "       ");
		strcpy (fehr_rec.cont_no, "       ");
		fehr_rec.hhfe_hash = 0L;
	}
}


/*
 * Read all invoices for current customer hash. 
 */
void
ReadInvoices (
	int		clearTotals,
	long	hhcuHash,
	long	hhhoHash)
{
	int 	i;
	char	fe_override [2];

	strcpy (fe_override, " ");
	if (clearTotals)
	{
		print_at (23, 0, "Reading Invoices : ");
		tmpLineCnt = 0;
		scn_set (S_DTLS);
	}
	if (hhhoHash)
		cuin_rec.hhcu_hash 	= hhcuHash;
	else
		cuin_rec.ho_hash 	= hhcuHash;

	strcpy (cuin_rec.est, "  ");
	strcpy (cuin_rec.inv_no, "      ");
	cuin_rec.date_of_inv = 0L;

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && 
		  (hhhoHash ? cuin_rec.hhcu_hash : cuin_rec.ho_hash) == hhcuHash)
	{
		DateToDMY (cuin_rec.date_of_inv, NULL, &invoiceMonth, &invoiceYear);

		if (invoiceYear < selectedYear || (invoiceMonth <= local_rec.sel_month && invoiceYear <= selectedYear))
		{
			ChequeInvBal ();
			if (twodec (local_rec.inv_balance) != 0.00)
			{
				if (envVar.feInstall && fe_override [0] != 'Y')
				{
					strcpy (feln_rec.index_by, "C");
					feln_rec.index_hash = cuin_rec.hhci_hash;
					cc = find_rec (feln, &feln_rec, EQUAL, "r");
					if (!cc)
					{
						if (fehr_rec.hhfe_hash != feln_rec.hhfe_hash)
						{
							if (fe_override [0] == 'N')
							{
								cc = find_rec (cuin, &cuin_rec, NEXT, "r");
								continue;
							}
							else if (fe_override [0] == ' ')
							{
								i = prmptmsg (ML (mlDbMess027), "YyNn", 25, 21);
								line_at (21,0,132);
								if (i == 'Y' || i == 'y')
								{
									sprintf (err_str, ML (mlDbMess025), fehr_rec.cont_no);
									i = prmptmsg (err_str, "YyNn", 25, 21);
									line_at (21,0,132);
									if (i == 'Y' || i == 'y')
										strcpy (fe_override, "Y");
									else
									{
										strcpy (fe_override, "N");
										cc = find_rec (cuin, &cuin_rec, NEXT, "r");
										continue;
									}
								}
								else
								{
									strcpy (fe_override, "N");
									cc = find_rec (cuin, &cuin_rec, NEXT, "r");
									continue;
								}
							}
						}
					}
				}

				if (envVar.feInstall)
				{
					strcpy (feln_rec.index_by, "C");
					feln_rec.index_hash = cuin_rec.hhci_hash;
					cc = find_rec (feln, &feln_rec, EQUAL, "r");
					if (!cc)
					{
						fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
						cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
						if (cc)
							file_err (cc, fehr, "DBFIND");
						strcpy (local_rec.l_fe_cont, fehr2_rec.cont_no);
					}
					else
						strcpy (local_rec.l_fe_cont, "      ");
				}

				local_rec.l_rec_exch = exchRate.localToCustomer;

				local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;
				strcpy (local_rec.l_inv_no, cuin_rec.inv_no);

				if (cuin_rec.type [0] == '1')
					strcpy (local_rec.l_inv_type, "INV");

				if (cuin_rec.type [0] == '2')
					strcpy (local_rec.l_inv_type, "CRD");

				if (cuin_rec.type [0] == '3')
					strcpy (local_rec.l_inv_type, "JNL");

				local_rec.l_inv_due = cuin_rec.due_date;
				local_rec.l_inv_bal = local_rec.inv_balance;
				local_rec.l_rec_oamt = local_rec.inv_balance;
				local_rec.l_inv_exch = cuin_rec.exch_rate;
				strcpy (local_rec.l_curr, cuin_rec.currency);
				strcpy (local_rec.l_fx, cuin_rec.er_fixed);
				if (cuin_rec.exch_rate != 0.00)
					local_rec.l_loc_amt = local_rec.inv_balance /
						cuin_rec.exch_rate;

				if (local_rec.l_rec_exch != 0.00)
					local_rec.l_rec_lamt = local_rec.inv_balance /
						local_rec.l_rec_exch;

				strcpy (lineTotal [tmpLineCnt].invNo, cuin_rec.inv_no);
				lineTotal [tmpLineCnt].payForeign = local_rec.l_rec_oamt;
				lineTotal [tmpLineCnt].payLocal   = local_rec.l_rec_lamt;
				lineTotal [tmpLineCnt].invLocal   = local_rec.l_loc_amt;

				local_rec.mon_inv = cuin_rec.date_of_inv;
				putval (tmpLineCnt);
				tmpLineCnt++;
			}
		}
		if (tmpLineCnt >= MAXLINES)
			return;

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
}

/*
 * Read invoices to satisfy amount. 
 */
void
ReadAmount (
	int	byAmount)
{
	int i;
	int pay_bal = FALSE;
	double wk_amount = 0.00;
	char fe_override [2];

	strcpy (fe_override, " ");
	/*
	 * Allocation of unallocated receipts. 
	 */
	if (processUnallocated)
		wk_amount = local_rec.unalloc_amt;
	else
		wk_amount = local_rec.rec_oamt + local_rec.rec_odis;

	abc_selfield (cuin, cumr_rec.ho_dbt_hash ? "cuin_cron3" 
												: "cuin_ho_cron3");

	tmpLineCnt = 0;
	scn_set (S_DTLS);
	if (cumr_rec.ho_dbt_hash)
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	else
		cuin_rec.ho_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");
	cuin_rec.date_of_inv = 0L;
	print_at (22,0, ML ("Reading Invoices : "));

	fflush (stdout);

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && 
		  (cumr_rec.ho_dbt_hash ? cuin_rec.hhcu_hash
			: cuin_rec.ho_hash) == cumr_rec.hhcu_hash)
	{
		DateToDMY (cuin_rec.date_of_inv, NULL, &invoiceMonth, &invoiceYear);

		ChequeInvBal ();

		if (no_dec (local_rec.inv_balance) == 0.00)
		{
			cc = find_rec (cuin, &cuin_rec, NEXT, "r");
			continue;
		}

		strcpy (local_rec.l_inv_no, cuin_rec.inv_no);

		if (cuin_rec.type [0] == '1')
			strcpy (local_rec.l_inv_type, "INV");

		if (cuin_rec.type [0] == '2')
			strcpy (local_rec.l_inv_type, "CRD");

		if (cuin_rec.type [0] == '3')
			strcpy (local_rec.l_inv_type, "JNL");

		if (byAmount && wk_amount < local_rec.inv_balance)
			pay_bal = TRUE;

		if (envVar.feInstall && fe_override [0] != 'Y')
		{
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			cc = find_rec (feln, &feln_rec, EQUAL, "r");
			if (!cc)
			{
				if (fehr_rec.hhfe_hash != feln_rec.hhfe_hash)
				{
					if (fe_override [0] == 'N')
					{
						cc = find_rec (cuin, &cuin_rec, NEXT, "r");
						continue;
					}
					else if (fe_override [0] == ' ')
					{
						i = prmptmsg (ML (mlDbMess027), "YyNn", 25, 21);
						line_at (21,0,132);
						if (i == 'Y' || i == 'y')
						{
							sprintf (err_str, ML (mlDbMess025), fehr_rec.cont_no);
							i = prmptmsg (err_str, "YyNn", 25, 21);
							line_at (21,0,132);
							if (i == 'Y' || i == 'y')
								strcpy (fe_override, "Y");
							else
							{
								strcpy (fe_override, "N");
								cc = find_rec (cuin, &cuin_rec, NEXT, "r");
								continue;
							}
						}
						else
						{
							strcpy (fe_override, "N");
							cc = find_rec (cuin, &cuin_rec, NEXT, "r");
							continue;
						}
					}
				}
			}
		}

		local_rec.l_inv_due = cuin_rec.due_date;
		local_rec.l_inv_bal = local_rec.inv_balance;
		local_rec.l_rec_oamt = (pay_bal) ? wk_amount
			: local_rec.inv_balance;
		local_rec.l_inv_exch = cuin_rec.exch_rate;
		strcpy (local_rec.l_curr, cuin_rec.currency);
		strcpy (local_rec.l_fx, cuin_rec.er_fixed);
		if (cuin_rec.exch_rate > 0.00)
		{
			local_rec.l_loc_amt = local_rec.inv_balance;
			local_rec.l_loc_amt /= cuin_rec.exch_rate;
		}

		if (envVar.feInstall)
		{
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			cc = find_rec (feln, &feln_rec, EQUAL, "r");
			if (!cc)
			{
				fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
				cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, fehr, "DBFIND");
				strcpy (local_rec.l_fe_cont, fehr2_rec.cont_no);
			}
			else
				strcpy (local_rec.l_fe_cont, "      ");
		}

		local_rec.l_rec_exch = exchRate.localToCustomer;

		if (local_rec.l_rec_exch > 0.00)
		{
			local_rec.l_rec_lamt = (pay_bal) ? wk_amount
				: local_rec.inv_balance;
			local_rec.l_rec_lamt /= local_rec.l_rec_exch;
		}
		lineTotal [tmpLineCnt].payForeign = local_rec.l_rec_oamt;
		lineTotal [tmpLineCnt].payLocal   = local_rec.l_rec_lamt;
		lineTotal [tmpLineCnt].invLocal   = local_rec.l_loc_amt;

		local_rec.mon_inv = cuin_rec.date_of_inv;
		wk_amount -= local_rec.inv_balance;
		local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;
		putval (tmpLineCnt);
		tmpLineCnt++;

		if (pay_bal)
			break;

		if (tmpLineCnt >= MAXLINES)
			break;

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	if (tmpLineCnt >= MAXLINES)
	{
		print_mess ("\007 Program will Only Allow 10000 Invoices To Be Payed at Once - (CONTINUING)");
		sleep (sleepTime);
		clear_mess ();
	}
	lcount [S_DTLS] = tmpLineCnt;
	scn_set (S_MAIN);
}

/*
 * Total invoice payments and determine balance . 
 */
void
ChequeInvBal (void)
{
	int i;

	/*
	 * For each invoice, print details if dbt - crd <> 0. 
	 */
	if (envVar.dbNettUsed)
		local_rec.inv_balance = no_dec (cuin_rec.amt - cuin_rec.disc);
	else
		local_rec.inv_balance = no_dec (cuin_rec.amt);

	for (i = 0; i < invCnt; i++)
	{
		if (cuin_rec.hhci_hash == invDtls [i].hhciHash)
			local_rec.inv_balance -= invDtls [i].inv_oamt;
	}
}

/*
 * Read existing receipt.             
 *                                    
 * Returns :                          
 *  0 - Successful                    
 *  1 - Receipt was not lodged with current bank code.            
 *  2 - Lodgement for receipt closed  
 *  3 - Prior period adjustment and user chose not to continue.   
 */
int
LoadReceipt (void)
{
	int		rtnVal = 0;

	abc_selfield (cuhd, "cuhd_id_no");

	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, local_rec.rec_no);
	cuhd_rec.index_date	=	0L;
	cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "w");
	if (cc)
	{
		newReceipt = TRUE;
		cuhd_rec.tot_amt_paid  = 0.00;
		cuhd_rec.loc_amt_paid  = 0.00;
		cuhd_rec.disc_given    = 0.00;
		cuhd_rec.loc_disc_give = 0.00;
	}
	else
		newReceipt = FALSE;

	abc_selfield (cuhd, "cuhd_hhcu_hash");
	if (!newReceipt)
	{
		/*
		 * Check lodgement. 
		 */
		bldt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (bldt2, &bldt_rec, EQUAL, "r");
		if (cc)
			file_err (cc, bldt2, "DBFIND");

		if (bldt_rec.hhbl_hash != blhd_rec.hhbl_hash)
			return (EXIT_FAILURE);
		else
		{
			if (bldt_rec.lodge_no != blhd_rec.nx_lodge_no)
				rtnVal = 2;
		}
		/*
		 * Store fields for reversal later on. 
		 */
		strcpy (headDetails.recType,   cuhd_rec.rec_type);
		strcpy (headDetails.narrative, cuhd_rec.narrative);
		strcpy (headDetails.altDrawer, cuhd_rec.alt_drawer);
		strcpy (headDetails.dbBank,    cuhd_rec.db_bank);
		strcpy (headDetails.dbBranch,  cuhd_rec.db_branch);

		headDetails.dueDate    = cuhd_rec.due_date;
		headDetails.bkExch     = cuhd_rec.bank_exch;
		headDetails.bkLclExch  = cuhd_rec.bank_lcl_exch;
		headDetails.bkRecAmt   = cuhd_rec.bank_amt;
		headDetails.bkCharge   = cuhd_rec.bank_chg;
		headDetails.bkClearFee = cuhd_rec.clear_fee;
		headDetails.oDisc      = cuhd_rec.disc_given;
		headDetails.oTotalAmt  = cuhd_rec.tot_amt_paid;
		headDetails.lDisc      = cuhd_rec.loc_disc_give;
		headDetails.lTotalAmt  = cuhd_rec.loc_amt_paid;

		/*
		 * Set fields on main screen. 
		 */
		strcpy (local_rec.rec_type, cuhd_rec.rec_type);

		switch (local_rec.rec_type [0])
		{
		case 'A':
			strcpy (local_rec.rec_type_desc, ML ("A-Cash       "));
			break;

		case 'B':
			strcpy (local_rec.rec_type_desc, ML ("Bank Draft   "));
			break;

		case 'C':
			strcpy (local_rec.rec_type_desc, ML ("Cheque       "));
			break;

		case 'D':
			strcpy (local_rec.rec_type_desc, ML ("Direct Credit"));
			break;
		}

		strcpy (local_rec.narrative,  cuhd_rec.narrative);
		strcpy (local_rec.alt_drawer, cuhd_rec.alt_drawer);
		strcpy (local_rec.db_bank,    cuhd_rec.db_bank);
		strcpy (local_rec.db_branch,  cuhd_rec.db_branch);

		local_rec.rec_date  = cuhd_rec.date_payment;
		local_rec.due_date  = cuhd_rec.due_date;
		local_rec.bank_chg  = cuhd_rec.bank_chg;
		local_rec.rec_amt   = cuhd_rec.bank_amt;

		local_rec.rec_oamt  = cuhd_rec.tot_amt_paid;
		local_rec.rec_lamt  = cuhd_rec.loc_amt_paid;
		local_rec.rec_odis  = cuhd_rec.disc_given;
		local_rec.rec_ldis  = cuhd_rec.loc_disc_give;
		local_rec.rec_gross = local_rec.rec_ldis + local_rec.rec_lamt;

		/*
		 * Load Letter of Credit Details. 
		 */
		headDetails.hhchHash = 0L;
		cucd_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (cucd2, &cucd_rec, COMPARISON, "r");
		if (!cc)
		{
			cuch_rec.hhch_hash	=	cucd_rec.hhch_hash;
			cc = find_rec (cuch2, &cuch_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, cuch2, "DBFIND");

			sprintf (local_rec.lett_crd, "%-15.15s", cuch_rec.letter_no);
			headDetails.hhchHash = cucd_rec.hhch_hash;
			validLetter = TRUE;

			/*
			 * Calculate amount remaining on letter of credit. 
			 */
			CalcLetterAmt ();
		}

		strcpy (local_rec.fe_cont_no, "      ");
		strcpy (fehr_rec.cont_no,     "      ");
		fehr_rec.hhfe_hash = 0L;

		/*
		 * Load all cudt / cuin details into lineDetails.  
		 * This array is used for reversing out the original receipt for 
		 * adjustments. At the same time, load lines into the S_DTLS screen.
		 */
		scn_set (S_DTLS);

		dtlsCnt = 0;
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		for (cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
			 !cc && 
			  cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash;
			 cc = find_rec (cudt, &cudt_rec, NEXT, "r"))
		{
			double	invBalFgn;

			/*
			 * Find the cuin relating to the cudt record. 
			 */
			cuin_rec.hhci_hash	=	cudt_rec.hhci_hash;
			cc = find_rec (cuin3, &cuin_rec, EQUAL, "r");
			if (cc)
			{
				sprintf (err_str,
						 "Error in cuin during DBFIND. hhciHash [%ld]",
						 cudt_rec.hhci_hash);
				sys_err (err_str, cc, PNAME);
			}

			/*
			 * Check array bounds. 
			 */
			if (!ArrChkLimit (&details_d, lineDetails, dtlsCnt))
				sys_err ("ArrChkLimit (lineDetails)", ENOMEM, PNAME);

			strcpy (lineDetails [dtlsCnt].invNo,         cuin_rec.inv_no);
			strcpy (lineDetails [dtlsCnt].invType,       cuin_rec.type);
			strcpy (lineDetails [dtlsCnt].invPayTerms,   cuin_rec.pay_terms);
			strcpy (lineDetails [dtlsCnt].invCurr,       cuin_rec.currency);
			strcpy (lineDetails [dtlsCnt].invFixedExch,  cuin_rec.er_fixed);
			strcpy (lineDetails [dtlsCnt].invFeContract, cuin_rec.currency);

			if (envVar.dbNettUsed)
				invBalFgn = no_dec (cuin_rec.amt - cuin_rec.disc);
			else
				invBalFgn = no_dec (cuin_rec.amt);

			lineDetails [dtlsCnt].invHhci     = cudt_rec.hhci_hash;
			lineDetails [dtlsCnt].invHhcu     = cuin_rec.hhcu_hash;
			lineDetails [dtlsCnt].invDueDate  = cuin_rec.due_date;
			lineDetails [dtlsCnt].invAmtFgn   = invBalFgn;
			lineDetails [dtlsCnt].invExchRate = cuin_rec.exch_rate;
			lineDetails [dtlsCnt].invAmtLcl   = 0.00;			/* NOT USED */

			lineDetails [dtlsCnt].recAmtFgn   = cudt_rec.amt_paid_inv;
			lineDetails [dtlsCnt].recExchRate = cudt_rec.exch_rate;
			lineDetails [dtlsCnt].recAmtLcl   = cudt_rec.loc_paid_inv;
			lineDetails [dtlsCnt].recExchVar  = cudt_rec.exch_variatio;

			dtlsCnt++;

			/*
			 * Put a line into the S_DTLS screen. 
			 */
			strcpy (local_rec.l_inv_no, cuin_rec.inv_no);

			switch (cuin_rec.type [0])
			{
			case '1':
				strcpy (local_rec.l_inv_type, "INV");
				break;

			case '2':
				strcpy (local_rec.l_inv_type, "CRD");
				break;

			case '3':
				strcpy (local_rec.l_inv_type, "JNL");
				break;

			}

			strcpy (local_rec.l_curr, cuin_rec.currency);
			strcpy (local_rec.l_fx,   cuin_rec.er_fixed);

			local_rec.l_inv_due = cuin_rec.due_date;

			/*
			 * Calculate outstanding value of invoice. 
			 */
			ChequeInvBal ();
			local_rec.l_inv_bal = local_rec.inv_balance;

			/*
			 * Add back the value allocated to the invoice 
			 * by the current line of the receipt.        
			 */
			local_rec.l_inv_bal += cudt_rec.amt_paid_inv;

			local_rec.l_inv_exch = cuin_rec.exch_rate;

			if (local_rec.l_inv_exch != 0.00)
			{
				local_rec.l_loc_amt = local_rec.l_inv_bal;
				local_rec.l_loc_amt /= local_rec.l_inv_exch;
			}

			local_rec.l_rec_oamt = cudt_rec.amt_paid_inv;
			local_rec.l_rec_exch = cudt_rec.exch_rate;
			local_rec.l_rec_lamt = cudt_rec.loc_paid_inv;

			local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;

			if (envVar.feInstall)
			{
				strcpy (fetr_rec.index_by, "C");
				fetr_rec.index_hash = cuin_rec.hhci_hash;
				fetr_rec.hhcp_hash  = cuhd_rec.hhcp_hash;
				cc = find_rec (fetr, &fetr_rec, EQUAL, "r");
				if (!cc)
				{
					fehr_rec.hhfe_hash = fetr_rec.hhfe_hash;
					cc = find_rec (fehr2, &fehr_rec, EQUAL, "r");
					if (cc)
						file_err (cc, fehr2, "DBFIND");
					strcpy (local_rec.l_fe_cont, fehr_rec.cont_no);

					strcpy (local_rec.fe_cont_no, fehr_rec.cont_no);
				}
				else
					strcpy (local_rec.l_fe_cont, "      ");
			}

			lineTotal [lcount [S_DTLS]].payForeign = local_rec.l_rec_oamt;
			lineTotal [lcount [S_DTLS]].payLocal   = local_rec.l_rec_lamt;
			lineTotal [lcount [S_DTLS]].invLocal   = local_rec.l_loc_amt;

			putval (lcount [S_DTLS]++);
		}
		scn_set (S_MAIN);
	}
	return (rtnVal);
}
/*
 * Main update routine. 
 */
void
Update (void)
{
	int		periodNum = 0;
	int		receiptPeriod;
	double	batchAmt;
	double	lVar = 0.00;
	double	exchVariation;

	DateToDMY (local_rec.rec_date, NULL, &receiptPeriod, NULL);

	clear ();
	print_at (0,0, ML ("Please wait now Processing Cheque"));

	/*
	 * Get Back Customer master record (cumr) for update. 
	 */
	abc_selfield (cumr, "cumr_hhcu_hash");
	cc = find_rec (cumr, &cumr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	abc_selfield (cuhd, "cuhd_id_no");

	if (envVar.dbHoOnly && cumr_rec.ho_dbt_hash > 0L)
		cuhd_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
	else
		cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;

	/*
	 * Generate receipt number if required. 
	 */
	GenerateReceiptNo ();

	/*
	 * Add or update Receipt Payment Header 
	 */
	print_at (1,0, ML ("Processing Cheque Number %s: "), local_rec.rec_no);

	if (newReceipt)
	{
		cuhd_rec.tot_amt_paid = no_dec (local_rec.rec_oamt);
		cuhd_rec.disc_given   = no_dec (local_rec.rec_odis);

		cuhd_rec.loc_amt_paid  = no_dec (local_rec.rec_lamt);
		cuhd_rec.loc_disc_give = no_dec (local_rec.rec_ldis);
	}
	else
	{
		cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "u");

		/*
		 * Reverse the receipt ready to post adjustment. 
		 */
		if (processDishonoured)
		{
			strcpy (cuhd_rec.dishonoured, "Y");
			cc = abc_update (cuhd, &cuhd_rec);
			if (cc)
				file_err (cc, cuhd, "DBUPDATE");
		}
		else
			ReverseReceipt ();

		if (!processUnallocated)
		{
			cuhd_rec.tot_amt_paid = no_dec (local_rec.rec_oamt);
			cuhd_rec.disc_given   = no_dec (local_rec.rec_odis);

			cuhd_rec.loc_amt_paid  = no_dec (local_rec.rec_lamt);
			cuhd_rec.loc_disc_give = no_dec (local_rec.rec_ldis);
		}
	}

	strcpy (cuhd_rec.cheque_no, local_rec.cheque_no);
	strcpy (cuhd_rec.or_no, local_rec.or_no);
	if (processDishonoured)
	{
		/*
		 * We already have the receipt to be dishonoured in cuhd_rec. 
		 * Reverse the amounts and put the new receipt number in. The
		 * cuhd record can then be used to add the Dishonouring cuhd.
		 */
		strcpy (cuhd_rec.receipt_no,  local_rec.rec_no);
		strcpy (cuhd_rec.rec_type,    "I");
		strcpy (cuhd_rec.dishonoured, "N");

		cuhd_rec.bank_amt      = no_dec (cuhd_rec.bank_amt 		* -1.00);
		cuhd_rec.tot_amt_paid  = no_dec (cuhd_rec.tot_amt_paid 	* -1.00);
		cuhd_rec.loc_amt_paid  = no_dec (cuhd_rec.loc_amt_paid 	* -1.00);
		cuhd_rec.disc_given    = no_dec (cuhd_rec.disc_given 	* -1.00);
		cuhd_rec.loc_disc_give = no_dec (cuhd_rec.loc_disc_give * -1.00);
		cuhd_rec.exch_variance = no_dec (cuhd_rec.exch_variance * -1.00);
		local_rec.bank_chg     = no_dec (0.00);
		cuhd_rec.bank_chg      = no_dec (0.00);
		cuhd_rec.clear_fee     = no_dec (0.00);
	}
	else
	{
		cuhd_rec.clear_fee = 0.00;
		if (!processUnallocated)
		{
			strcpy (cuhd_rec.type,       "1");
			strcpy (cuhd_rec.receipt_no, local_rec.rec_no);
			strcpy (cuhd_rec.bank_id,    crbk_rec.bank_id);
			sprintf (cuhd_rec.rec_type,  "%-1.1s", local_rec.rec_type);
			strcpy (cuhd_rec.alt_drawer, local_rec.alt_drawer);
			strcpy (cuhd_rec.narrative,  local_rec.narrative);
			strcpy (cuhd_rec.lodge_flag, "N");
			strcpy (cuhd_rec.db_bank,    local_rec.db_bank);
			strcpy (cuhd_rec.db_branch,  local_rec.db_branch);

			cuhd_rec.bank_amt      = local_rec.rec_amt;
			cuhd_rec.bank_exch     = exchRate.bankToCustomer;
			cuhd_rec.bank_lcl_exch = exchRate.bankToLocal;
			cuhd_rec.bank_chg      = local_rec.bank_chg;
			cuhd_rec.due_date      = local_rec.due_date;
			cuhd_rec.date_payment  = local_rec.rec_date;
			cuhd_rec.date_posted   = StringToDate (local_rec.systemDate);

			if (CHQ_PAYMNT && !SPLIT_PAYMENT)
				cuhd_rec.clear_fee = crbk_rec.clear_fee;

			/*
			 * If we are adjusting an existing split payment 
			 * then use the cheque fee determined when the  
			 * record was first created.                   
			 */
			if (!newReceipt && multipleReceipts)
				cuhd_rec.clear_fee = headDetails.bkClearFee;
		}

		/*
		 * Calculate exchange variation. 
		 */
		exchVariation = CalcExchVariation ();
		cuhd_rec.exch_variance = exchVariation;
		strcpy (cuhd_rec.dishonoured, "N");
	}

	if (newReceipt || processDishonoured)
	{
		/*
		 * Add new payment header and get hhcp hash ref. 
		 */
		strcpy (cuhd_rec.stat_flag, "0");
		cuhd_rec.index_date	=	0L;
        strcpy (cuhd_rec.cheque_no, local_rec.cheque_no);
        strcpy (cuhd_rec.or_no, local_rec.or_no);
		cc = abc_add (cuhd, &cuhd_rec);
		if (cc)
			file_err (cc, cuhd, "DBADD");

		abc_unlock (cuhd);

		cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, cuhd, "DBFIND");

		AddPaymentHistory ();
	}
	else
	{
        strcpy (cuhd_rec.cheque_no, local_rec.cheque_no);
        strcpy (cuhd_rec.or_no, local_rec.or_no);
		cc = abc_update (cuhd, &cuhd_rec);
		if (cc)
			file_err (cc, cuhd, "DBUPDATE");
	}

	abc_selfield (cuhd, "cuhd_hhcu_hash");

	/*
	 * Add Receipts to cash receipts work file & cudt file. 
	 *
	 * Add cudt to reverse deposit for the value allocated to invoices.
	 */
	if (processUnallocated)
	{
		/*
		 * Find deposit (cuin) record. 
		 */
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.inv_no, local_rec.depositNo);
		cc = find_rec (cuin4, &cuin_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cuin4, "DBFIND");

		memcpy (&cuin2_rec, &cuin_rec, sizeof (cuin_rec));

		local_rec.l_rec_lamt = allocateTotal;
		if (exchRate.localToCustomer != 0.00)
		{
			local_rec.l_rec_lamt = no_dec (local_rec.l_rec_lamt / exchRate.localToCustomer);
		}

		/*
		 * Do not update cudr if an argument is passed to CRCINPUT (Journal).
		 */
		if (!journalOnly)
			AddCudr (receiptPeriod, TRUE);

		/*
		 * Add cudt record. 
		 */
		cudt_rec.hhcp_hash    = cuhd_rec.hhcp_hash;
		cudt_rec.hhci_hash    = cuin_rec.hhci_hash;
		cc = find_rec (cudt2, &cudt_rec, EQUAL, "u");
		if (cc)
		{
			cudt_rec.amt_paid_inv = no_dec (allocateTotal 		 * -1.00);
			cudt_rec.loc_paid_inv = no_dec (local_rec.l_rec_lamt * -1.00);
		}
		else
		{
			cudt_rec.amt_paid_inv += no_dec (allocateTotal * -1.00);
			cudt_rec.loc_paid_inv += no_dec (local_rec.l_rec_lamt * -1.00);
		}
		cudt_rec.exch_rate    = exchRate.localToCustomer;
		cudt_rec.exch_variatio     = 0.00;

		strcpy (cudt_rec.stat_flag, "0");

		cc = cc ? abc_add (cudt2, &cudt_rec)
				: abc_update (cudt2, &cudt_rec);
		if (cc)
			file_err (cc, cudt2, "DBADD/DBUPDATE");

		/*
		 * Add Commission record for receipts made against an invoice. 
		 */
		if (envVar.salesCommission && envVar.salesCommissionPayment)
		{
			AddCommission 
			 (
				cumr_rec.hhcu_hash,
				cuin_rec.hhci_hash,
				cuhd_rec.hhcp_hash,
				cudt_rec.loc_paid_inv
			);
		}
	}

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, comm_rec.est_no);
	cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;

	scn_set (S_DTLS);

	/*
	 * Set up period and selpay as no longer in tabular. 
	 */
	for (tmpLineCnt = 0; tmpLineCnt < lcount [S_DTLS]; tmpLineCnt++)
	{
		getval (tmpLineCnt);

		/*
	     * Do not update cudr if an argument is passed to CRCINPUT (Journal).
	     */
		if (!journalOnly)
			AddCudr (receiptPeriod, FALSE);

		if (local_rec.l_inv_exch != 0.00)
			lVar = local_rec.l_rec_oamt / local_rec.l_inv_exch;

		cudt_rec.hhcp_hash    = cuhd_rec.hhcp_hash;
		cudt_rec.hhci_hash    = cuin_rec.hhci_hash;
		strcpy (cudt_rec.stat_flag, "0");

		if (processDishonoured)
		{
			int	i = tmpLineCnt;

			cudt_rec.amt_paid_inv = no_dec (lineDetails [i].recAmtFgn * -1.00);
			cudt_rec.loc_paid_inv = no_dec (lineDetails [i].recAmtLcl * -1.00);
			cudt_rec.exch_rate    = lineDetails [i].recExchRate;
			cudt_rec.exch_variatio     = no_dec (lineDetails [i].recExchVar * -1.00);
		}
		else
		{
			cudt_rec.amt_paid_inv = no_dec (local_rec.l_rec_oamt);
			cudt_rec.loc_paid_inv = no_dec (local_rec.l_rec_lamt);
			cudt_rec.exch_rate    = exchRate.localToCustomer;
			cudt_rec.exch_variatio     = no_dec (local_rec.l_rec_lamt - lVar);
		}

		if (local_rec.l_hhcu_hash != cumr_rec.hhcu_hash)
		{
			wkRec.hhcuHash = local_rec.l_hhcu_hash;
			cc = RF_ADD (perFd, (char *)&wkRec);
			if (cc)
				file_err (cc, "wkno", "WKADD");
		}

		cc = abc_add (cudt, &cudt_rec);
		if (cc)
			file_err (cc, cudt, "DBADD");

		/*
		 * Add Commission record for receipts made against an invoice. 
		 */
		if (envVar.salesCommission && envVar.salesCommissionPayment)
		{
			AddCommission 
			(
				cumr_rec.hhcu_hash,
				cuin_rec.hhci_hash,
				cuhd_rec.hhcp_hash,
				cudt_rec.loc_paid_inv
			);
		}
		if (cuhd_rec.date_payment <= monthEndDate)
		{
			periodNum = AgePeriod
						 (
							cuin_rec.pay_terms,
						 	local_rec.mon_inv,
							altDate,
							cuin_rec.due_date,
						 	envVar.DaysAgeing,
							envVar.true_age
						);

			if (periodNum == -1)
				cumr_rec.bo_fwd -= no_dec (local_rec.l_rec_oamt);
			else
				cumr_balance [periodNum] -= no_dec (local_rec.l_rec_oamt);
		}

		if (envVar.feInstall)
			AddFetr ();
	}

	/*
	 * Create draw-off record. 
	 */
	CreateDrawOff (cuhd_rec.bank_amt);

	/*
     * update customer aged amount. 
	 */
	cumr_rec.date_lastpay = cuhd_rec.date_payment;
	cumr_rec.amt_lastpay  = no_dec (local_rec.rec_oamt);

	/*
	 * If it's not a new receipt then only add the difference to the
	 * batch total figure.            
	 */
	if (!newReceipt)
		batchAmt = local_rec.rec_oamt - headDetails.oTotalAmt;
	else
		batchAmt = local_rec.rec_oamt;
		
	batchTotal += batchAmt;

	if (!processUnallocated)
	{
		cc = abc_update (cumr, &cumr_rec);
		if (cc)
			file_err (cc, cumr, "DBUPDATE");
	}

	wkRec.hhcuHash = cumr_rec.hhcu_hash;
	cc = RF_ADD (perFd, (char *) &wkRec);
	if (cc)
		file_err (cc, "wkno", "WKADD");

	/*
	 * Find lodgement header. 
	 */
	if (!processUnallocated)
	{
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		cc = find_rec (blhd, &blhd_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, blhd, "DBFIND");
	}

	/*
	 * Add bank lodgement detail record. 
	 */
	if (!processUnallocated)
	{
		if (newReceipt || processDishonoured)
		{
			if (processDishonoured)
				strcpy (bldt_rec.rec_type, "I");
			else
				sprintf (bldt_rec.rec_type, "%-1.1s", local_rec.rec_type);

			bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
			bldt_rec.lodge_no  = blhd_rec.nx_lodge_no;

			if (BANK_DRAFT || DIRECT_CRD)
				bldt_rec.lodge_date = local_rec.rec_date;
			else
				bldt_rec.lodge_date = 0L;
			bldt_rec.lodge_time = 0L;

			bldt_rec.hhcu_hash = cumr_rec.hhcu_hash;
			if (strlen (clip (local_rec.alt_drawer)) != 0)
				strcpy (bldt_rec.dbt_name, local_rec.alt_drawer);
			else
				strcpy (bldt_rec.dbt_name, cumr_rec.dbt_name);
			strcpy (bldt_rec.bank_code, local_rec.db_bank);
			strcpy (bldt_rec.branch_code, local_rec.db_branch);

			bldt_rec.hhcp_hash   = cuhd_rec.hhcp_hash;
			bldt_rec.amount      = cuhd_rec.bank_amt;
			bldt_rec.chq_fees    = cuhd_rec.clear_fee;
			bldt_rec.bank_chg    = cuhd_rec.bank_chg;
			bldt_rec.bk_lcl_exch = cuhd_rec.bank_lcl_exch;

			if (BANK_DRAFT)
				bldt_rec.due_date = local_rec.due_date;
			else
				bldt_rec.due_date = 0L;

			sprintf (bldt_rec.presented, "%-1.1s", envVar.dbLdgPres);
			strcpy (bldt_rec.reconcile,  "N");
			strcpy (bldt_rec.stat_flag,  "0");

			cc = abc_add (bldt, &bldt_rec);
			if (cc)
				file_err (cc, bldt, "DBADD");
		}
		else
		{
			bldt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
			cc = find_rec (bldt2, &bldt_rec, EQUAL, "u");
			if (cc)
				file_err (cc, bldt2, "DBFIND");

			bldt_rec.amount   = local_rec.rec_amt;
			bldt_rec.bank_chg = local_rec.bank_chg;
			bldt_rec.chq_fees = cuhd_rec.clear_fee;

			cc = abc_update (bldt2, &bldt_rec);
			if (cc)
				file_err (cc, bldt2, "DBUPDATE");
		}
	}

	/*
	 * Update Alternate Drawer, Customer Bank and Customer Branch for 
	 * split payments of this receipt number.
	 */
	UpdatePayeeDetails ();

	strcpy (local_rec.prev_receipt, local_rec.rec_no);
	strcpy (local_rec.prev_dbt_no, local_rec.dbt_no);

    PauseForKey (3, 0, ML ("Press Any Key to Continue."), 0);
}

/*
 * Generate New Receipt Number. 
 */
void
GenerateReceiptNo (void)
{
	/*
	 * For Split Payments, use the receipt number entered. 
	 */
	if (SPLIT_PAYMENT)
		return;

	/*
	 * When processing Dishonoured receipts AND the   
	 * environment is set for manual receipt numbers,
	 * we must get the new number from the user.    
	 */
	if (!envVar.dbAutoRec && processDishonoured)
	{
		int		recNoValid;

		print_at (2, 1, ML ("Enter New Receipt Number for Dishonoured Receipt : "));

		recNoValid = FALSE;
		while (!recNoValid)
		{
			getalpha (53, 2, "UUUUUUUU", local_rec.rec_no);

			sprintf (cuhd2_rec.receipt_no, "%-8.8s", local_rec.rec_no);
			cc = find_rec (cuhd3, &cuhd2_rec, COMPARISON, "r");
			if (!cc)
			{
				print_mess (ML (mlDbMess015));
				sleep (sleepTime);
				clear_mess ();
				continue;
			}

			if (!strcmp (local_rec.rec_no, "        ") ||
				!strcmp (local_rec.rec_no, ""))
			{
				print_mess ("\007Receipt number may not be blank.");
				sleep (sleepTime);
				clear_mess ();
				continue;
			}

			recNoValid = TRUE;
		}
	}

	/*
	 * Generate a receipt number if :                     
	 * Environment is set to System Generated numbers AND EITHER we are 
	 * processing a Dishonoured Receipt OR we are NOT Allocating Unallocated 
	 * Cash AND we are processing a New Receipt.                 
	 */
	if (envVar.dbAutoRec && 
		 (processDishonoured ||
		 (!processUnallocated && 
		  newReceipt)))
	{
		int		newRecNoOk = FALSE;
		char	nextRecNo [9];

		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");

		/*
		 * Get next receipt number from comr and validate that a cuhd does 
		 * not already exist for the new receipt number.    
		 */
		while (!newRecNoOk)
		{

			comr_rec.nx_rec_no++;

			sprintf (nextRecNo, "%-ld", comr_rec.nx_rec_no);

			strcpy (cuhd2_rec.receipt_no, zero_pad (nextRecNo, INV_NO_LEN));
			cc = find_rec (cuhd3, &cuhd2_rec, COMPARISON, "r");
			if (cc)
				newRecNoOk = TRUE;
		}

		sprintf (local_rec.rec_no, "%-8.8s", nextRecNo);
		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");
	}
}

/*
 * Reverse the existing receipt by posting reversing (-ve) cudt records for 
 * existing links. Note that for adjustments to receipts posted to prior 
 * periods, we create a new cuhd to link the reversal cudt's to.     
 */
void
ReverseReceipt (void)
{
	int		i;
	int		receiptPeriod;

	DateToDMY (local_rec.rec_date, NULL, &receiptPeriod, NULL);

	if (!processUnallocated)
	{
		/*
		 * Remove all cudt records for this receipt. 
		 * They will be re-added soon.              
		 */
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		for (cc = find_rec (cudt, &cudt_rec, GTEQ, "u");
			 !cc &&
			  cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash;
			 cc = find_rec (cudt, &cudt_rec, GTEQ, "u"))
		{
			cc = abc_delete (cudt);
			if (cc)
				file_err (cc, cudt, "DBDELETE");
		}
		abc_unlock (cudt);
	}

	/*
	 * Reverse Letter of Credit draw-off record. 
	 */
	if (validLetter)
	{
		cucd_rec.hhch_hash = headDetails.hhchHash;
		cucd_rec.hhcp_hash = cuhd_rec.hhcp_hash;
		cc = find_rec (cucd2, &cucd_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, cucd2, "DBFIND");

		cc = abc_delete (cucd2);
		if (cc)
			file_err (cc, cucd2, "DBDELETE");
	}

	/*
	 * Reverse the posting from G/L by adding 
	 * an inverse cudr for each lineDetail.  
	 */
	for (i = 0; i < dtlsCnt; i++)
	{
		strcpy (cudr_rec.co_no,    comm_rec.co_no);
		strcpy (cudr_rec.br_no,    comm_rec.est_no);
		strcpy (cudr_rec.rec_no,   local_rec.rec_no);
		strcpy (cudr_rec.reversal, "Y");
		sprintf (cudr_rec.period,  "%02d", receiptPeriod);
		cudr_rec.hhcp_hash = cuhd_rec.hhcp_hash;

		sprintf (cudr_rec.bank_id,   "%-5.5s",   crbk_rec.bank_id);
		sprintf (cudr_rec.bank_name, "%-40.40s", crbk_rec.bank_name);
		sprintf (cudr_rec.bk_ccode,  "%-3.3s",   crbk_rec.curr_code);
		sprintf (cudr_rec.bk_cdesc,  "%-40.40s", local_rec.bk_curr_desc);

		sprintf (cudr_rec.dbt_no,    "%-6.6s",   local_rec.dbt_no);
		sprintf (cudr_rec.dbt_name,  "%-40.40s", cumr_rec.dbt_name);
		sprintf (cudr_rec.rec_type,  "%-1.1s",   headDetails.recType);

		GL_GLI 
		(
			cudr_rec.co_no,
			cudr_rec.br_no,
			"  ",
			"DISC ALLOW",
 			(envVar.GlByClass) 	? cumr_rec.class_type
								: cumr_rec.sman_code,
			" "
		);
		strcpy (cudr_rec.gl_disc,    glmrRec.acc_no);
		strcpy (cudr_rec.narrative,  headDetails.narrative);

		cudr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cudr_rec.rec_date  = local_rec.rec_date;
		if (cudr_rec.rec_type [0] == 'B')
			cudr_rec.due_date = headDetails.dueDate;
		else
			cudr_rec.due_date = 0L;

		cudr_rec.bk_exch      = headDetails.bkExch;
		cudr_rec.bk_rec_amt   = no_dec (headDetails.bkRecAmt);
		cudr_rec.bk_charge    = no_dec (headDetails.bkCharge);
		cudr_rec.bk_clear_fee = no_dec (headDetails.bkClearFee);
		cudr_rec.bk_l_exch    = headDetails.bkLclExch;

		sprintf (cudr_rec.invoice, "%-8.8s", lineDetails [i].invNo);
		cudr_rec.inv_exch    = lineDetails [i].invExchRate;
		cudr_rec.inv_amt     = no_dec (lineDetails [i].invAmtFgn);
		strcpy (cudr_rec.o_curr      , lineDetails [i].invCurr);
		cudr_rec.o_exch      = lineDetails [i].recExchRate;
		cudr_rec.o_disc      = no_dec (headDetails.oDisc);
		cudr_rec.o_total_amt = no_dec (headDetails.oTotalAmt + headDetails.oDisc);
		cudr_rec.o_amt_pay   = no_dec (lineDetails [i].recAmtFgn);
		cudr_rec.l_disc      = no_dec (headDetails.lDisc);
		cudr_rec.l_total_amt = no_dec (headDetails.lTotalAmt + headDetails.lDisc);
		cudr_rec.l_amt_pay   = no_dec (lineDetails [i].recAmtLcl);

		cc = RF_ADD (crcFd, (char *) &cudr_rec);
		if (cc)
			file_err (cc, "cudr_rec", "WKADD");

		/*
		 * Remove Forward Exchange Transaction records. 
		 * These will be re-added later (if required). 
		 */
		if (envVar.feInstall)
		{
			strcpy (fetr_rec.index_by, "C");
			fetr_rec.index_hash = lineDetails [i].invHhci;
			fetr_rec.hhcp_hash  = cuhd_rec.hhcp_hash;
			cc = find_rec (fetr, &fetr_rec, EQUAL, "u");
			if (!cc)
			{
				cc = abc_delete (fetr);
				if (cc)
					file_err (cc, fetr, "DBDELETE");
			}
		}
	}
}

/*
 * Calculate Exchange Variation. 
 */
double
CalcExchVariation (void)
{
	double	exchVariation;
	double	lVar = (double) 0;

	scn_set (S_DTLS);

	exchVariation = 0.00;
	for (tmpLineCnt = 0; tmpLineCnt < lcount [S_DTLS]; tmpLineCnt++)
	{
		getval (tmpLineCnt);
		if (local_rec.l_inv_exch != 0.00)
			lVar = local_rec.l_rec_oamt / local_rec.l_inv_exch;

		exchVariation += no_dec (local_rec.l_rec_lamt - lVar);
	}

	scn_set (S_MAIN);

	return (exchVariation);
}

/*
 * Create draw-off record for Bank Draft. 
 */
void
CreateDrawOff (
	double	drawOffAmt)
{
	if (BANK_DRAFT && validLetter && !processUnallocated)
	{
		cucd_rec.hhch_hash = cuch_rec.hhch_hash;
		cucd_rec.rec_date  = local_rec.rec_date;
		cucd_rec.hhcp_hash = cuhd_rec.hhcp_hash;

		/*
		 * Bank receipt amount. 
		 */
		cucd_rec.amount = drawOffAmt;
		strcpy (cucd_rec.stat_flag, "0");
		cc = abc_add (cucd, &cucd_rec);
		if (cc)
			file_err (cc, cucd, "DBADD");
	}
}

/*
 * Add Forward Exchange Transaction 
 */
void
AddFetr (void)
{
	if (processUnallocated)
	{
		/*
		 * Reduce deposit fetr by allocated amount 
		 */
		strcpy (fetr_rec.index_by, "C");
		fetr_rec.index_hash = cuin2_rec.hhci_hash;
		fetr_rec.hhcp_hash = cudt_rec.hhcp_hash;
		cc = find_rec (fetr, &fetr_rec, EQUAL, "u");
		if (!cc)
		{
			fetr_rec.value -= no_dec (local_rec.l_rec_oamt);
			if (no_dec (fetr_rec.value) <= 0.00)
			{
				cc = abc_delete (fetr);
				if (cc)
					file_err (cc, fetr, "DBDELETE");
			}
			else
			{
				cc = abc_update (fetr, &fetr_rec);
				if (cc)
					file_err (cc, fetr, "DBDELETE");
			}
		}
	}

	if (fehr_rec.hhfe_hash > 0L)
	{
		strcpy (fehr_rec.co_no, comm_rec.co_no);
		strcpy (fehr_rec.cont_no, local_rec.fe_cont_no);
		cc = find_rec (fehr, &fehr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, fehr, "DBFIND");
	}
	else
		fehr_rec.hhfe_hash = 0L;

	strcpy (feln_rec.index_by, "C");
	feln_rec.index_hash = cuin_rec.hhci_hash;
	cc = find_rec (feln, &feln_rec, EQUAL, "u");
	if (!cc)
	{
		if (fehr_rec.hhfe_hash != feln_rec.hhfe_hash)
		{
			fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
			cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
			if (cc)
				file_err (cc, fehr, "DBFIND");
			fehr2_rec.val_avail += no_dec (local_rec.l_rec_oamt);
			cc = abc_update (fehr2, &fehr2_rec);
			if (cc)
				file_err (cc, fehr, "DBFIND");
			if (fehr_rec.hhfe_hash > 0)
			{
				fehr_rec.val_avail -= no_dec (local_rec.l_rec_oamt);
				strcpy (fetr_rec.index_by, "C");
				fetr_rec.index_hash = cuin_rec.hhci_hash;
				fetr_rec.hhfe_hash = fehr_rec.hhfe_hash;
				fetr_rec.hhcp_hash = cuhd_rec.hhcp_hash;
				fetr_rec.value = no_dec (local_rec.l_rec_oamt);
				cc = abc_add (fetr, &fetr_rec);
				if (cc)
					file_err (cc, fetr, "DBADD");
			}
			feln_rec.value -= no_dec (local_rec.l_rec_oamt);
			if (no_dec (feln_rec.value) <= 0.00)
			{
				cc = abc_delete (feln);
				if (cc)
					file_err (cc, feln, "DBDELETE");
			}
			else
			{
				cc = abc_update (feln, &feln_rec);
				if (cc)
					file_err (cc, feln, "DBDELETE");
			}
		}
		else
		{
			fehr_rec.val_avail -= no_dec (local_rec.l_rec_oamt);
			strcpy (fetr_rec.index_by, "C");
			fetr_rec.index_hash = cuin_rec.hhci_hash;
			fetr_rec.hhfe_hash = fehr_rec.hhfe_hash;
			fetr_rec.hhcp_hash = cuhd_rec.hhcp_hash;
			fetr_rec.value = no_dec (local_rec.l_rec_oamt);
			cc = abc_add (fetr, &fetr_rec);
			if (cc)
				file_err (cc, fetr, "DBADD");
			feln_rec.value -= no_dec (local_rec.l_rec_oamt);
			if (no_dec (feln_rec.value) <= 0.00)
			{
				cc = abc_delete (feln);
				if (cc)
					file_err (cc, feln, "DBDELETE");
			}
			else
			{
				cc = abc_update (feln, &feln_rec);
				if (cc)
					file_err (cc, feln, "DBDELETE");
			}
		}
	}
	else
	{
		if (fehr_rec.hhfe_hash > 0)
		{
			fehr_rec.val_avail -= no_dec (local_rec.l_rec_oamt);
			strcpy (fetr_rec.index_by, "C");
			fetr_rec.index_hash = cuin_rec.hhci_hash;
			fetr_rec.hhfe_hash = fehr_rec.hhfe_hash;
			fetr_rec.hhcp_hash = cuhd_rec.hhcp_hash;
			fetr_rec.value = no_dec (local_rec.l_rec_oamt);
			cc = abc_add (fetr, &fetr_rec);
			if (cc)
				file_err (cc, fetr, "DBADD");
		}
	}
	if (fehr_rec.hhfe_hash > 0 && !processUnallocated)
	{
		cc = abc_update (fehr, &fehr_rec);
		if (cc)
			file_err (cc, fehr, "DBUPDATE");
	}
	else
		abc_unlock (fehr);
}

/*
 * Add cheque history for Customer. 
 */
void
AddPaymentHistory (void)
{
	cuph_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cuph_rec.amt_cheq  = cuhd_rec.tot_amt_paid;
	cuph_rec.date_cheq = cuhd_rec.date_payment;

	strcpy (cuph_rec.cheq_no,     cuhd_rec.receipt_no);
	strcpy (cuph_rec.bank_code,   local_rec.db_bank);
	strcpy (cuph_rec.branch_code, local_rec.db_branch);
	strcpy (cuph_rec.stat_flag,   "0");

	cc = abc_add (cuph, &cuph_rec);
	if (cc)
		file_err (cc, cuph, "DBADD");
}

/*
 * Add cheque history for Customer. 
 */
void
AddCudr (
	int		receiptPeriod,
	int		rvs_dep)
{
	strcpy (cudr_rec.co_no,    comm_rec.co_no);
	strcpy (cudr_rec.br_no,    comm_rec.est_no);
	strcpy (cudr_rec.rec_no,   local_rec.rec_no);
	strcpy (cudr_rec.reversal, "N");
	sprintf (cudr_rec.period,  "%02d", receiptPeriod);
	cudr_rec.hhcp_hash = cuhd_rec.hhcp_hash;

	sprintf (cudr_rec.bank_id,   "%-5.5s",   crbk_rec.bank_id);
	sprintf (cudr_rec.bank_name, "%-40.40s", crbk_rec.bank_name);
	sprintf (cudr_rec.bk_ccode,  "%-3.3s",   crbk_rec.curr_code);
	sprintf (cudr_rec.bk_cdesc,  "%-40.40s", local_rec.bk_curr_desc);
	sprintf (cudr_rec.dbt_no,    "%-6.6s",   local_rec.dbt_no);
	sprintf (cudr_rec.dbt_name,  "%-40.40s", cumr_rec.dbt_name);
	sprintf (cudr_rec.rec_type,  "%-1.1s",   cuhd_rec.rec_type);

	GL_GLI 
	(
		cudr_rec.co_no,
		cudr_rec.br_no,
		"  ",
		"DISC ALLOW",
		 (envVar.GlByClass)	? cumr_rec.class_type
						   	: cumr_rec.sman_code,
		" "
	);
	strcpy (cudr_rec.gl_disc,    glmrRec.acc_no);
	strcpy (cudr_rec.narrative,  local_rec.narrative);

	cudr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cudr_rec.rec_date  = local_rec.rec_date;
	cudr_rec.o_exch    = exchRate.localToCustomer;
	strcpy (cudr_rec.o_curr, local_rec.l_curr);
	if (BANK_DRAFT)
		cudr_rec.due_date = local_rec.due_date;
	else
		cudr_rec.due_date = 0L;

	if (processUnallocated)
	{
		cudr_rec.bk_exch      = 1.00;
		cudr_rec.bk_rec_amt   = 0.00;
		cudr_rec.bk_charge    = 0.00;
		cudr_rec.bk_clear_fee = 0.00;
		cudr_rec.bk_l_exch    = 1.00;
		cudr_rec.o_disc       = 0.00;
		cudr_rec.o_total_amt  = 0.00;
		cudr_rec.l_disc       = 0.00;
		cudr_rec.l_total_amt  = 0.00;

		if (rvs_dep)
		{
			sprintf (cudr_rec.invoice, "%-8.8s", local_rec.depositNo);
			cudr_rec.inv_exch  = exchRate.localToCustomer;
			cudr_rec.inv_amt   = 0.00;
			cudr_rec.o_amt_pay = no_dec (allocateTotal * -1.00);
			cudr_rec.l_amt_pay = no_dec (local_rec.l_rec_lamt * -1.0);
		}
		else
		{
			sprintf (cudr_rec.invoice, "%-8.8s", local_rec.l_inv_no);
			cudr_rec.inv_exch  = local_rec.l_inv_exch;
			cudr_rec.inv_amt   = no_dec (local_rec.l_inv_bal);
			cudr_rec.o_amt_pay = no_dec (local_rec.l_rec_oamt);
			cudr_rec.l_amt_pay = no_dec (local_rec.l_rec_lamt);
		}
	}
	else
	{
		sprintf (cudr_rec.invoice, "%-8.8s", local_rec.l_inv_no);
		cudr_rec.bk_exch      = cuhd_rec.bank_exch;

		cudr_rec.bk_rec_amt   = no_dec (local_rec.rec_amt);
		cudr_rec.bk_charge    = no_dec (local_rec.bank_chg);
		cudr_rec.bk_l_exch    = cuhd_rec.bank_lcl_exch;
		cudr_rec.inv_exch     = local_rec.l_inv_exch;
		cudr_rec.inv_amt      = no_dec (local_rec.l_inv_bal);
		cudr_rec.o_disc       = no_dec (local_rec.rec_odis);
		cudr_rec.o_total_amt  = no_dec (local_rec.rec_oamt + local_rec.rec_odis);
		cudr_rec.o_amt_pay    = no_dec (local_rec.l_rec_oamt);
		cudr_rec.l_disc       = no_dec (local_rec.rec_ldis);
		cudr_rec.l_total_amt  = no_dec (local_rec.rec_lamt + local_rec.rec_ldis);
		cudr_rec.l_amt_pay    = no_dec (local_rec.l_rec_lamt);

		if (processDishonoured)
			cudr_rec.bk_clear_fee = no_dec (cuhd_rec.clear_fee * -1.00);
		else
			cudr_rec.bk_clear_fee = no_dec (cuhd_rec.clear_fee);
	 }

	cc = RF_ADD (crcFd, (char *) &cudr_rec);
	if (cc)
		file_err (cc, "cudr_rec", "WKADD");
}

/*
 * Update Alternate Drawer, Customer Bank and Customer Branch for split 
 * payments of this receipt number.
 */
void
UpdatePayeeDetails (void)
{
	char	updReceipt [9];
	char	altDrawer [21];
	char	dbBank [4];
	char	dbBranch [21];

	/*
	 * If we are processing Dishonoured Receipts OR processing Unallocated 
	 * Cash then no details have changed.
	 */
	if (processUnallocated || processDishonoured)
		return;

	/*
	 * If we are processing a new receipt and it is not a Split Payment then 
	 * there are no other cuhd / bldt records to be updated. 
	 */
	if (newReceipt && !SPLIT_PAYMENT)
		return;

	/*
	 * If we are adjusting an existing receipt or processing a Split Payment 
	 * then the headDetails structure has been loaded with the original Payee 
	 * Details.  Check whether they have changed and if they have, update all 
	 * cuhd's (for this receipt number) and the bldt relating to it.
	 */
	if (!strcmp (headDetails.altDrawer, cuhd_rec.alt_drawer) &&
		!strcmp (headDetails.dbBank,    cuhd_rec.db_bank) &&
		!strcmp (headDetails.dbBranch,  cuhd_rec.db_branch))
	{
		return;
	}

	strcpy (altDrawer, cuhd_rec.alt_drawer);
	strcpy (dbBank,    cuhd_rec.db_bank);
	strcpy (dbBranch,  cuhd_rec.db_branch);

	/*
	 * Find all cuhd records with this receipt number. 
	 */
	strcpy (updReceipt, cuhd_rec.receipt_no);
	for (cc = find_rec (cuhd3, &cuhd_rec, GTEQ, "u");
		 !cc && !strcmp (cuhd_rec.receipt_no, updReceipt);
		 cc = find_rec (cuhd3, &cuhd_rec, NEXT, "u"))
	{
		/*
		 * Update payee details. 
		 */
		strcpy (cuhd_rec.alt_drawer, altDrawer);
		strcpy (cuhd_rec.db_bank,    dbBank);
		strcpy (cuhd_rec.db_branch,  dbBranch);
		cc = abc_update (cuhd3, &cuhd_rec);
		if (cc)
			file_err (cc, cuhd3, "DBUPDATE");

		/*
		 * Find bldt record and update payee details. 
		 */
		bldt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (bldt2, &bldt_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, bldt, "DBFIND");

		if (strlen (clip (altDrawer)) != 0)
			sprintf (bldt_rec.dbt_name, "%-40.40s", altDrawer);

		strcpy (bldt_rec.bank_code,   dbBank);
		strcpy (bldt_rec.branch_code, dbBranch);

		cc = abc_update (bldt2, &bldt_rec);
		if (cc)
			file_err (cc, bldt2, "DBUPDATE");

	}
	abc_unlock (cuhd);
	abc_unlock (bldt);
}

/*
 * Add dummy invoice number. 
 */
void
AddInvoice (
	int		depositFlag,    
	char	*invoiceType)
{
	int 	workInvoiceNumber	=	1;

	move (1, 2); cl_line ();
	print_at (2,1,ML (mlStdMess035));
	fflush (stdout);
	while (1)
	{
		/*
		 * If the cumr has a head office use this hash for cuin head office 
		 * hash otherwise use the same customer hash.                      
		 */
		if (envVar.dbHoOnly && cumr_rec.ho_dbt_hash > 0L)
		{
			cuin_rec.ho_hash   = cumr_rec.ho_dbt_hash;
			cuin_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
		}
		else
		{
			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
			if (cumr_rec.ho_dbt_hash)
				cuin_rec.ho_hash = cumr_rec.ho_dbt_hash;
			else
				cuin_rec.ho_hash = cumr_rec.hhcu_hash;
		}
		sprintf (cuin_rec.type, "%-1.1s", invoiceType);
		strcpy (cuin_rec.co_no, comm_rec.co_no);
		strcpy (cuin_rec.est, (!envVar.dbCo) ? comm_rec.est_no
											 : cumr_rec.est_no);

		strcpy (cuin_rec.dp, cumr_rec.department);
		if (depositFlag)
			sprintf (cuin_rec.inv_no, "DP%06d", workInvoiceNumber);
		else
			sprintf (cuin_rec.inv_no, "%-8.8s",
					 zero_pad (local_rec.l_inv_no, INV_NO_LEN));

		strcpy (cuin_rec.narrative, "                    ");
		strcpy (cuin_rec.pay_terms, "0  ");
		strcpy (cuin_rec.currency,  cumr_rec.curr_code);
		strcpy (cuin_rec.er_fixed,  "N");
		strcpy (cuin_rec.stat_flag, "0");

		cuin_rec.date_of_inv       = local_rec.rec_date;
		cuin_rec.date_posted     = local_rec.rec_date;
		cuin_rec.due_date  = local_rec.rec_date;
		cuin_rec.exch_rate = exchRate.localToCustomer;
		cuin_rec.disc      = 0.00;
		cuin_rec.amt       = 0.00;

		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
			break;

		workInvoiceNumber++;
	}
	local_rec.l_hhcu_hash = cuin_rec.hhcu_hash;
	strcpy (local_rec.l_inv_no, cuin_rec.inv_no);
	if (!depositFlag)
		DSP_FLD ("l_invoice");

	cc = abc_add (cuin, &cuin_rec);
	if (cc)
		file_err (cc, cuin, "DBADD");

	cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");

	move (1, 2);
	cl_line ();
	invoiceAdded = TRUE;
}

/*
 * MISCELLANEOUS SUPPORT ROUTINES.                    
 */

/*
 * Check that forward exchange amount available  
 * is not exceeded by the amount being receipted
 */
int
CheckFehr (void)
{
	double	allocAmount	=	0.00,
			availAmount	=	0.00;
	int		sav_screen	=	0;

	if (fehr_rec.hhfe_hash == 0L)
		return (EXIT_SUCCESS);

	availAmount = fehr_rec.val_avail;
	sav_screen 	= cur_screen;
	scn_set (S_DTLS);
	for (tmpLineCnt = 0; tmpLineCnt < lcount [S_DTLS]; tmpLineCnt++)
	{
		getval (tmpLineCnt);
		strcpy (feln_rec.index_by, "C");
		feln_rec.index_hash = cuin_rec.hhci_hash;
		cc = find_rec (feln, &feln_rec, EQUAL, "r");
		if (!cc)
		{
			if (fehr_rec.hhfe_hash == feln_rec.hhfe_hash)
				availAmount += feln_rec.value;
		}
		allocAmount += no_dec (local_rec.l_rec_oamt);
	}
	scn_set (sav_screen);

	if (allocAmount > availAmount)
	{
		sprintf (err_str, ML (mlDbMess030), 
							DOLLARS (allocAmount), DOLLARS (availAmount));
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check proof totals. 
 */
int
ProofTrans (void)
{
	int i;

	double headTotal 		= 0.00;
	double headLocal 		= 0.00;
	double allocateLocal 	= 0.00;

	/*
	 * Allocation of unallocated receipts. 
	 */
	if (processUnallocated)
		headTotal = local_rec.unalloc_amt;
	else
		headTotal = local_rec.rec_oamt + local_rec.rec_odis;

	allocateTotal = 0.00;
	for (i = 0; i < lcount [S_DTLS]; i++)
		allocateTotal += lineTotal [i].payForeign;

	proofTotal = no_dec (allocateTotal - headTotal);

	if (proofTotal == 0.00)
		chkError = FALSE;
	else
	{
		/*
		 * Allocation of unallocated receipts. 
		 */
		if (processUnallocated && proofTotal < 0.00)
		{
			sprintf (err_str, ML (mlDbMess031), DOLLARS (proofTotal * -1.00));
			i = prmptmsg (err_str, "YyNn", 25, 21);
			move (0, 21);
			cl_line ();
			if (i == 'Y' || i == 'y')
			{
				chkError = FALSE;
				return (EXIT_SUCCESS);
			}
		}

		chkError = TRUE;
		sprintf (err_str, ML (mlDbMess032),
				cumr_rec.curr_code,DOLLARS (headTotal),DOLLARS (allocateTotal));
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
	}

	/*
	 * Check for rounding errors in local values and round header 
	 * local amount accordingly.                                  
	 */
	if (processUnallocated)
		headLocal = local_rec.unalloc_loc;
	else
		headLocal = local_rec.rec_lamt + local_rec.rec_ldis;

	allocateLocal = 0.00;
	for (i = 0; i < lcount [S_DTLS]; i++)
		allocateLocal += lineTotal [i].payLocal;

	proofTotal = no_dec (allocateLocal - headLocal);

	if (proofTotal == 0.00)
		chkError = FALSE;
	else
	{
		if (processUnallocated)
		{
			chkError = TRUE;
			sprintf (err_str, ML (mlDbMess033),
							DOLLARS (headLocal),DOLLARS (allocateLocal));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		else
		{
			chkError = TRUE;
			sprintf (err_str, ML (mlDbMess033),
					 		DOLLARS (headLocal), DOLLARS (allocateLocal));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();

			if (!strcmp (cumr_rec.curr_code, crbk_rec.curr_code))
				return (EXIT_SUCCESS);

			sprintf (err_str, ML (mlDbMess034),
							 DOLLARS (proofTotal), DOLLARS (allocateLocal));
			i = prmptmsg (err_str, "YyNn", 25, 20);
			line_at (20,0,132);
			if (i == 'Y' || i == 'y')
			{
				local_rec.rec_lamt += proofTotal;
				chkError = FALSE;
			}
			else
				return (EXIT_SUCCESS);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Unlock main tables records. 
 */
void
UnlockRecords (void)
{
	abc_unlock (cumr);
	abc_unlock (cuhd);
	abc_unlock (cuin);
	abc_unlock (cudt);
}

/*
 * Routine to get cheque details and hold relevent invoice Against. 
 */
void
GetCheques (
	int		clearTotals,
	long	hhcuHash)
{
	if (clearTotals)
	{
		invCnt = 0;
		abc_selfield (cuhd, "cuhd_hhcu_hash");
	}

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
		while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
		{
			if (!ArrChkLimit (&invDtls_d, invDtls, invCnt))
				sys_err ("ArrChkLimit (invDtls)", ENOMEM, PNAME);

			invDtls [invCnt].hhciHash 	= cudt_rec.hhci_hash;
			invDtls [invCnt].inv_oamt  	= cudt_rec.amt_paid_inv;
			invDtls [invCnt].inv_lamt  	= cudt_rec.loc_paid_inv;
			invDtls [invCnt].exch_var  	= cudt_rec.exch_variatio;
			invDtls [invCnt].exch_rate 	= cudt_rec.exch_rate;
			invCnt++;
			cc = find_rec (cudt, &cudt_rec, NEXT, "r");
		}
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*
 * Print payments total. 
 */
void
DisplayTotals (void)
{
	int i;
	int no_lines = (prog_status == ENTRY) ? line_cnt : lcount [S_DTLS] - 1;
	double rec_lamt = 0.00,
	  rec_oamt = 0.00,
	  all_amt = 0.00,
	  local = 0.00;

	for (i = 0; i <= no_lines; i++)
	{
		rec_lamt += lineTotal [i].payLocal;
		rec_oamt += lineTotal [i].payForeign;
		local    += lineTotal [i].invLocal;
	}

	rec_oamt 	= DOLLARS (rec_oamt);
	rec_lamt 	= DOLLARS (rec_lamt);
	local 		= DOLLARS (local);
	all_amt 	= DOLLARS (local_rec.rec_oamt + local_rec.rec_odis);

	/*
	 * Allocation of unallocated receipts. 
	 */
	if (processUnallocated)
	{
		sprintf (err_str, 
				 "Total Amount Unallocated (%s) %8.2f ",
				 cumr_rec.curr_code,
				 DOLLARS (local_rec.unalloc_amt));
	}
	else
	{
		sprintf (err_str, 
				 "Total Receipt Amount (%s) %8.2f ",
				 cumr_rec.curr_code, all_amt);
	}
	rv_pr (err_str, 2, 18, 1);

	if (envVar.feInstall)
	{
		line_at (17,67,17);
		line_at (17,86,17);
		line_at (17,104,17);

		print_at (18, 75, "%8.2f", local);
		print_at (18, 92, "%R %8.2f", rec_oamt);
		print_at (18, 111, "%8.2f", rec_lamt);

		line_at (19,67,17);
		line_at (19,86,17);
		line_at (19,104,17);
	}
	else
	{
		line_at (17,67,17);
		line_at (17,86,17);
		line_at (17,113,17);

		print_at (18, 75, "%8.2f", local);
		print_at (18, 92, "%R %8.2f", rec_oamt);
		print_at (18, 121, "%8.2f", rec_lamt);

		line_at (19,67,17);
		line_at (19,86,17);
		line_at (19,113,17);
	}
}

/*
 * Read branch date. 
 */
long 
ReadBranchDate (
	char	*brNo)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", brNo);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		return (comm_rec.dbt_date);

	return (esmr_rec.dbt_date);
}

/*
 * Add Commission record for receipts made against an invoice. 
 */
void
AddCommission (
	long	hhcuHash,
	long	hhciHash,
	long	hhcpHash,
	double	pay_amt)
{
	double	PayPercent = 0.00,
			CommAmt    = 0.00;

	sach_rec.hhci_hash = hhciHash;
	cc = find_rec (sach, &sach_rec, GTEQ, "r");
	while (!cc && sach_rec.hhci_hash == hhciHash)
	{
		PayPercent = pay_amt / sach_rec.inv_amt;
		CommAmt    = sach_rec.com_val * PayPercent;
	
		sacl_rec.sach_hash	=	sach_rec.sach_hash;
		sacl_rec.hhcp_hash	=	hhcpHash;
		sacl_rec.rec_amt	=	pay_amt;
		sacl_rec.rec_date	=	local_rec.rec_date;
		sacl_rec.com_amt	=	CommAmt;
		strcpy (sacl_rec.status, "0");

		cc = find_rec (sacl, &sacl_rec, COMPARISON, "r");
		if (cc)
		{
			sacl_rec.rec_amt	=	pay_amt;
			sacl_rec.rec_date	=	local_rec.rec_date;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_add (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBADD");
		}
		else
		{
			sacl_rec.rec_amt	=	pay_amt;
			sacl_rec.rec_date	=	local_rec.rec_date;
			sacl_rec.com_amt	=	CommAmt;
			strcpy (sacl_rec.status, "0");

			cc = abc_update (sacl, &sacl_rec);
			if (cc)
				file_err (cc, sacl, "DBUPDATE");
		}
		cc = find_rec (sach, &sach_rec, NEXT, "r");
	}
}

/*
 * Screen headings and frills. 
 */
int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	clear ();
	if (scn != cur_screen)
		scn_set (scn);

	if (!journalOnly)
		rv_pr (ML (mlDbMess050), 56,0,1);
	else
		rv_pr (ML (mlDbMess051), 42,0,1);

	print_at (0,90, ML (mlDbMess166), local_rec.prev_receipt, 
									  local_rec.prev_dbt_no);

	line_at (1,0,132);

	pr_box_lines (scn);

	/*
	 * Allocation of unallocated receipts. 
	 */
	if (processUnallocated)
		us_pr (ML (mlDbMess140), 48, 1, 1);
	else if (processDishonoured)
		us_pr (ML (mlDbMess246), 50, 1, 1);

	if (scn == S_DTLS)
	{
		box (10, 2, 110, 1);
		print_at (3, 11, ML (mlStdMess082), crbk_rec.bank_id);
		print_at (3, 50, ML (mlStdMess083), crbk_rec.bank_name);

		DisplayTotals ();
	}

	print_at (22,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	if (tabscn) 
		print_at (22, 60, ML (mlDbMess167),cumr_rec.dbt_no,cumr_rec.dbt_name);
	else 
		print_at (22, 60, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);

	/*  reset this variable (line_cnt) for new screen NOT page */
	if (scn != cur_screen)
		line_cnt = 0;

	scn_write (scn);

    return (EXIT_SUCCESS);
}

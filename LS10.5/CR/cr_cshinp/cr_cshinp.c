/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_cshinp.c,v 5.8 2002/07/24 08:38:44 scott Exp $
|  Program Name  : (cr_cshinp.c   )                                  | 
|  Program Desc  : (Supplier Cash Cheque Input                )      |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 22/03/96         |
|---------------------------------------------------------------------|
| $Log: cr_cshinp.c,v $
| Revision 5.8  2002/07/24 08:38:44  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/07/16 06:39:28  scott
| Updated from service calls and general maintenance.
|
| Revision 5.5  2002/06/25 03:17:03  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.4  2002/06/21 04:10:24  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/11/13 01:42:39  scott
| Updated to change invoice mask to "AAAAAAAAAAAAAAA" from "UUUUUUUUUUUUUUU"
|
| Revision 5.2  2001/08/09 08:51:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:22  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_cshinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_cshinp/cr_cshinp.c,v 5.8 2002/07/24 08:38:44 scott Exp $";

#define MAXSCNS		3
#define MAXLINES	5000
#define	DTLS		4000	/* Max No. of details per supplier         */

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>
#include 	<GlUtils.h>
#include 	<twodec.h>
#include 	<arralloc.h>

#define	CO_CRD		 (envCoClose [1] == '1')
#define	MAN_CHQ   	 (local_rec.pay_type [0] == '1')
#define	DRAFT_PAY	 (local_rec.pay_type [0] == '2')
#define	DEP_CHQ		 (local_rec.pay_type [0] == '3')
#define	DEP_DRAFT	 (local_rec.pay_type [0] == '4')
#define	TRANSFER	 (local_rec.pay_type [0] == '5')

#define	MAXEST		100

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct suinRecord	suin_rec;
struct suinRecord	suin2_rec;
struct suhdRecord	suhd_rec;
struct sudtRecord	sudt_rec;
struct suhpRecord	suhp_rec;
struct suhtRecord	suht_rec;
struct esmrRecord	esmr_rec;
struct crbkRecord	crbk_rec;
struct bkcrRecord	bkcr_rec;

	Money	*sumr_per		=	&sumr_rec.bo_per1;


	/*==============================================
	| Supplier/Supplier Cheque Work File Record. |
	==============================================*/
	struct {
		char	cr_co_no [3];
		char	cr_br_no [3];
		char	cr_chq_no [16];
		char	cr_period [3];
		long	cr_hhsp_hash;

		char	cr_bank_id [6];
		char	cr_bank_name [41];
		char	cr_bk_ccode [4];
		char	cr_bk_cdesc [41];
		double	cr_bk_exch;			/* Origin to bank exchange */
		double	cr_bk_rec_amt;		/* Bank receipt amount */
		double	cr_bk_charge;		/* Bank receipt amount */
		double	cr_bk_l_exch;		/* Bank to local exch rate */

		char	cr_crd_no [7];		/* Supplier Number */
		char	cr_crd_name [41];	/* Supplier Name */
		long	cr_hhsu_hash;		/* Supplier Hash */

		long	cr_rec_date;		/* Cheque Date */
		char	cr_pay_type [2];	/* Cheque Type */
		char	cr_narrative [21];	/* Bank Draft Reference */

		char	cr_invoice [16];	/* Invoice being paid */
		double	cr_inv_exch;		/* Invoice Exchange Rate */
		double	cr_inv_amt;			/* Invoice Amount */

		char	cr_o_curr [4];		/* Transaction, customer currency. */
		double	cr_o_exch;			/* Origin to local exchange */
		Money	cr_o_disc;	
		Money	cr_o_total_amt;
		Money	cr_o_amt_pay;	
		char	cr_gl_disc [MAXLEVEL + 1];		/* GL discount account */
		Money	cr_l_disc;
		Money	cr_l_total_amt;
		Money	cr_l_amt_pay;
	} sudr_rec;


	struct {
		long	hhsuHash;
	} wk_rec;

	char	*data  = "data",
			*sudt2 = "sudt2",
			*suhd2 = "suhd2",
			*suin2 = "suin2",
			*suin3 = "suin3",
			*suin4 = "suin4",
			*sumr2 = "sumr2";

/*
 * The structure invDtls' is initialised in function 'GetCheque'   
 * the number of details is stored in external variable 'invCnt'.
 */
struct	tagInvDtls {    /*===================================*/
	long	hhsiHash;	/*| detail invoice reference.       |*/
	double	inv_oamt;	/*| Invoice overseas amount.        |*/
	double	inv_lamt;	/*| Invoice local amount.           |*/
	double	exch_var;	/*| Exchange variation.             |*/
	double	exch_rate;	/*| Exchange rate.                  |*/
} *invDtls;				/*===================================*/
static	DArray	invDtls_d;
static	int		invCnt;

int		newReceipt 			= FALSE,
		useAlternateDate 	= FALSE,
		automaticReceipt 	= FALSE,
		forwardMessage 		= TRUE,
		invoiceAdded 		= FALSE,
		multiCurrency		= FALSE,
		chequeError 		= TRUE,
		allocFlag			= FALSE,
		currentMonth 		= 0,
		currentYear 		= 0,
		invoiceMonth 		= 0,
		invoiceYear 		= 0,
		workLineNo	  		= 0,
		pidNumber			= 0,
		workRecordNumber	= 0,
		selectYear 			= 0,
		sudrNumber			= 0,
		envCoOwned 			= 0,
		envCrFind 			= 0,
		notJournalOnly		= 0;

long	nextChequeNumber 	= 0L, 
		alternateDate 		= 0L;

double  proofTotal 			= 0.00,
		batchTotal 			= 0.00,
		bankLocalExchRate	= 0.00,
		allocationTotal		= 0.00;

struct	storeRec {
	double	amountFgnPayment;
	double	amountLocPayment;
	double	amountLocal;
} store [MAXLINES];

char	branchNumber 	[3],
		envCoClose 		[6],
		currencyCode 	[4],
		dflt_dep 		[16],
		remittDesc 		[61],
		defaultRemit	[2];

char	*scn_desc [] = {
		"HEADER SCREEN.",
		"HEADER SCREEN.",
		"ALLOCATION SCREEN.",
		""
};

/*
 * Total for each branchNumber. 
 */
static double	est_total [MAXEST];
static double	est_local [MAXEST];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	prevCheque [16];
	char	prevSupplierNo [7];
	char	systemDate [11];
	long	lsystemDate;
	long	mon_inv;
	double	inv_balance;
	double	tot_cheque;
	char	bk_curr_desc [41];
	char	chq_no [16];
	char	dep_no [16];
	long	rec_date;
	double	rec_amt;
	double	bank_chg;
	char	crd_no [7];
	double	rec_oamt;
	double	unalloc_amt;
	double	unalloc_loc;
	double	rec_odis;
	double	rec_lamt;
	double	rec_ldis;
	double	rec_gross;
	char	oamt_prmt [31];
	char	unalloc_prmt [31];
	char	unalloc_lprmt [31];
	char	odis_prmt [31];
	char	lamt_prmt [31];
	char	bk_ex_prmt [31];
	char	bk_rec_prmt [31];
	char	gross_prmt [31];
	char	narrative [21];
	char	pay_type [2];
	char	pay_type_desc [14];
	double	bk_exch_rate;
	double	exch_rate;
	char	alloc [2];
	char	alloc_desc [31];
	int		sel_month;
	char	l_inv_no [16];
	char	l_inv_type [5];
	char	l_curr [4];
	char	l_fx [2];
	double	l_inv_bal;
	double	l_inv_exch;
	double	l_loc_amt;
	double	l_rec_oamt;
	double	l_rec_exch;
	double	l_rec_lamt;
	char	prt_remit [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "bank_id",	 3, 26, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Bank Code           : ", "Enter Bank Code. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 3, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Name           : ", "Enter Bank name.",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "bk_curr",	 4, 26, CHARTYPE,
		"AAA", "          ",
		" ", "", "Currency            : ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "bk_curr_desc",	 4, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},
	{1, LIN, "cheque",	 6, 26, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Cheque Number.      : ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.chq_no},
	{1, LIN, "rec_date",	 6, 80, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date of Cheque      : ", "Enter Cheque date. < default = today > ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.rec_date},
	{1, LIN, "pay_type",	7, 26, CHARTYPE,
		"U", "          ",
		" ", "1", "Cheque Type.        : ", "Enter 1=Manual Cheque, 2=Draft Payment, 3=Deposit Cheque, 4=Deposit Draft.",
		YES, NO,  JUSTLEFT, "1234", "", local_rec.pay_type},
	{1, LIN, "pay_type_desc",7, 29, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.pay_type_desc},
	{1, LIN, "ref",	 8, 26, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Voucher number      : ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.narrative}, 
	{1, LIN, "supplier",	 10, 26, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier No.        : ", "Use Normal Supplier Search keys or [Search-3] to Search on Invoice Number.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{1, LIN, "name",	 10, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "bk_exch_rate",	 12, 26, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", local_rec.bk_ex_prmt, " ",
		 YES, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.bk_exch_rate},
	{1, LIN, "bank_chg",	 12, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Bank Charges.       : ", "Enter amount of Bank charges. ",
		 YES, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.bank_chg},
	{1, LIN, "orec_amt",	 14, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.oamt_prmt, " ",
		 YES, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_oamt},
	{1, LIN, "orec_dis",	 14, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.odis_prmt, " ",
		 YES, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_odis},
	{1, LIN, "exch_rate",	 15, 26, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Exchange rate.      : ", " ",
		 NI, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.exch_rate},
	{1, LIN, "rec_amt",	 15, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.bk_rec_prmt, "Enter amount of Cheque. ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_amt},
	{1, LIN, "lrec_amt",	 16, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.lamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_lamt},
	{1, LIN, "rec_gross",	 16, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.gross_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_gross},
	{1, LIN, "lrec_dis",	 16, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "", " ",
		 ND, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_ldis},
	{1, LIN, "alloc",	18, 26, CHARTYPE,
		"U", "          ",
		" ", "A", "Allocation Type     : ", "Enter (U)nallocated, (R)everse date order, (S)elective document entry, M(onth), (A)mount cheque allocation.",
		NE, NO,  JUSTLEFT, "URSMA", "", local_rec.alloc},
	{1, LIN, "alloc_desc",	18, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.alloc_desc},
	{1, LIN, "selmonth",	18, 80, INTTYPE,
		"NN", "          ",
		"0", "", "Month (1-12)        : ", " ",
		 NE, NO,  JUSTLEFT, "1", "12", (char *)&local_rec.sel_month},
	{1, LIN, "prt_remit",	 19, 26, CHARTYPE,
		"U", "          ",
		" ", defaultRemit, "Print Remittances   :", remittDesc,
		NI, NO,  JUSTLEFT, "YN", "", local_rec.prt_remit},
	{2, LIN, "supplier", 	3, 26, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier No.        : ", "Use Normal Supplier Search keys or FN11 to Search on Invoice Number.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{2, LIN, "name2", 	3, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{2, LIN, "cheque",	4, 26, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Cheque Number       : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.chq_no},
	{2, LIN, "dep_no",	5, 26, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Deposit Number.     : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dep_no},
	{2, LIN, "bk_curr2",	7, 26, CHARTYPE,
		"AAA", "          ",
		" ", "", "Currency            : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.curr_code},
	{2, LIN, "bk_curr_desc2", 7, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},
	{2, LIN, "exch_rate2",	 8, 26, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Exchange rate.      : ", " ",
		 NA, NO,  JUSTRIGHT, "0", "9999", (char *)&local_rec.exch_rate},
	{2, LIN, "orec_amt2",	 9, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.oamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_oamt},
	{2, LIN, "unalloc_amt",	 9, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.unalloc_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.unalloc_amt},
	{2, LIN, "lrec_amt2",	 10, 26, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.lamt_prmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.rec_lamt},
	{2, LIN, "unalloc_loc",	 10, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", local_rec.unalloc_lprmt, " ",
		 NA, NO,  JUSTRIGHT, "0", "999999999", (char *)&local_rec.unalloc_loc},
	{2, LIN, "prt_remit2",	 12, 26, CHARTYPE,
		"U", "          ",
		" ", defaultRemit, "Print Remittances   : ", remittDesc,
		NI, NO,  JUSTLEFT, "YN", "", local_rec.prt_remit},
	{2, LIN, "alloc2",	13, 26, CHARTYPE,
		"U", "          ",
		" ", "A", "Allocation Type     : ", "Enter (R)everse date order, (S)elective document entry, M(onth), (A)mount cheque allocation.",
		NE, NO,  JUSTLEFT, "RSMA", "", local_rec.alloc},
	{2, LIN, "alloc_desc2",	13, 30, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.alloc_desc},
	{2, LIN, "selmonth2",	13, 80, INTTYPE,
		"NN", "          ",
		"0", "", "Month (1-12)        : ", " ",
		 NE, NO,  JUSTLEFT, "1", "12", (char *)&local_rec.sel_month},

	{3, TAB, "l_invoice",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Invoice  Number", "Enter DEPOSIT for Deposit for unallocated cash.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.l_inv_no},
	{3, TAB, "l_type",	0, 1, CHARTYPE,
		"AAAA", "          ",
		" ", "", " Type ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_inv_type},
	{3, TAB, "l_invamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Invoice Amt (Fgn)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_inv_bal},
	{3, TAB, "l_curr",	0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "", "Curr", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_curr},
	{3, TAB, "l_inv_exch",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Invoice Exch.", " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.l_inv_exch},
	{3, TAB, "l_fx",	0, 1, CHARTYPE,
		"U", "          ",
		" ", "", "F/X", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.l_fx},
	{3, TAB, "l_locamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Invoice Amt (Loc)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_loc_amt},
	{3, TAB, "l_rec_oamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Cheque Amt (Fgn)", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_oamt},
	{3, TAB, "l_rec_exch",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", "Dep. Exch", " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.l_rec_exch},
	{3, TAB, "l_rec_lamt",	 0, 0, MONEYTYPE,
		"N,NNN,NNN,NNN.NN", "          ",
		" ", "", "Cheque Amt (Loc)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_rec_lamt},
	{3, TAB, "hhsiHash",	 0, 0, LONGTYPE,
		"NNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&suin_rec.hhsi_hash},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
void	ReadInvoice		 (int, long);
int		ValidAllocation	 (void);
void	SrchUnallocated	 (char *);
void	SrchDeposits	 (char *);
int		FindDeposits	 (int, int);
void	ReadChequeAmount (int);
int		DeleteLine		 (int);
int		CalcInvoiceBal	 (void);
int		ReadCheques		 (void);
int		PrintTotal		 (void);
int		Update			 (void);
void	MakeCheqHist	 (void);
void	AddSudr		 	 (int, int);
void	SrchCrbk		 (char *);
int		ProofTrans		 (void);
int		heading			 (int);
int		RecordUnlockFunc (void);
void	AddInvoices		 (int, char *);
void	SrchSuin2		 (char *);
int		GetCheque		 (int, long);
void	SrchSuin		 (char *);
long	ReadbranchDates	 (char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;
	int		i;

	if (argc < 2)
	{
		print_at (0,0,mlStdMess046, argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check program name. 
	 */
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	allocFlag = FALSE;
	if (!strcmp (sptr, "cr_cshallc"))
		allocFlag = TRUE;

	pidNumber  = atoi (argv [1]);

	/*
	 * Will Set notJournalOnly to true if any argument is passed to Program. 
	 */
	notJournalOnly = (argc > 2);

	/*
	 * Multi-currency debtors. 
	 */
	sptr = chk_env ("CR_MCURR");
	if (sptr)
		multiCurrency = (*sptr == 'Y' || *sptr == 'y') ? TRUE : FALSE;
	else
		multiCurrency = FALSE;

	/*
	 * Auto generated cheque numbers. 
	 */
	sptr = chk_env ("CR_AUTO_REC");
	if (sptr)
		automaticReceipt = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		automaticReceipt = FALSE;

	strcpy (remittDesc, "Print Remittance <Y (es) N (o)> <return = N (o)>");
	strcpy (defaultRemit, atoi (get_env ("CR_REMIT")) ? "Y" : "N");

	/*
	 * Company close. 
	 */
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *) 0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	/*
	 * Set up Screen Prompts. 
	 */
	sprintf (currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	envCoOwned = atoi (get_env ("CR_CO"));
	envCrFind  = atoi (get_env ("CR_FIND"));

	if (!CO_CRD && envCrFind && envCoOwned)
		useAlternateDate = TRUE;
	else
		useAlternateDate = FALSE;

    /*
     * Allocate intital detail line array. 
     */
    ArrAlloc (&invDtls_d, &invDtls, sizeof (struct tagInvDtls), 1000);
    
	SETUP_SCR (vars);


	init_scr ();
	set_tty (); 
	if (multiCurrency)
		_set_masks ("cr_cshinpM.s");
	else
		_set_masks ("cr_cshinpS.s");

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	for (i = 0; i < 4; i++)
		tab_data [i]._desc = scn_desc [i];

	/*----------------------------
	| Hide multi-currency fields |
	----------------------------*/
	if (!multiCurrency)
	{
		/*-------------
		| Screen One. |
		-------------*/
		FLD ("bk_curr") 		= ND;
		FLD ("bk_curr_desc") = ND;
		FLD ("bk_exch_rate") = ND;
		FLD ("rec_amt") 		= ND;
		FLD ("exch_rate") 	= ND;
		FLD ("lrec_amt") 	= ND;

		for (i = label ("cheque"); i <= label ("orec_dis"); i++)
			vars [i].row -= 1;

		for (i = label ("rec_gross"); i <= label ("prt_remit"); i++)
			vars [i].row -= 2;

		vars [label ("rec_gross")].col = 26;

		/*-------------
		| Screen Two. |
		-------------*/
		FLD ("bk_curr2") 	 = ND;
		FLD ("bk_curr_desc2") = ND;
		FLD ("exch_rate2") 	 = ND;
		FLD ("lrec_amt2") 	 = ND;
		vars [label ("orec_amt2")].row 	= 7;
		vars [label ("unalloc_amt")].row 	= 7;
		vars [label ("alloc2")].row 		= 9;
		vars [label ("alloc_desc2")].row 	= 9;
		vars [label ("selmonth2")].row 	= 9;

		/*---------------
		| Screen Three. |
		---------------*/
		tab_col = 30;
		FLD ("l_curr") 		= ND;
		FLD ("l_inv_exch") 	= ND;
		FLD ("l_fx") 		= ND;
		FLD ("l_locamt") 	= ND;
		FLD ("l_rec_lamt") 	= ND;
		FLD ("l_rec_exch") 	= ND;
	}

	if (allocFlag)
		no_edit (1);
	else
		no_edit (2);

	init_vars (1);	

	tab_row = 5;

	swide ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();

	GL_SetMask (GlFormat);

	strcpy (local_rec.oamt_prmt,   	ML ("Cheque Amount       : "));
	strcpy (local_rec.odis_prmt,   	ML ("Discount Amount     : "));
	strcpy (local_rec.bk_ex_prmt,  	ML ("Bank exch rate      : "));
	strcpy (local_rec.bk_rec_prmt, 	ML ("Bank Deposit        : "));
	strcpy (local_rec.unalloc_prmt,	ML ("Unallocated Amt     : "));
	strcpy (local_rec.unalloc_lprmt,ML ("Unallocated Amt     : "));
		
	if (multiCurrency)
	{
		strcpy (err_str, ML ("Deposit Amount (%-3.3s): "));
		sprintf (local_rec.lamt_prmt, err_str, currencyCode);

		strcpy (err_str, ML ("Gross cheque (%-3.3s)  : "));
		sprintf (local_rec.gross_prmt, err_str, currencyCode);

		strcpy (err_str, ML ("Unallocated Amt (%-3.3s): "));
		sprintf (local_rec.unalloc_lprmt, err_str, currencyCode);
	}
	else
	{
		strcpy (local_rec.lamt_prmt,  ML ("Deposit Amount      : "));
		strcpy (local_rec.gross_prmt, ML ("Gross cheque        : "));
	}


	strcpy (branchNumber, (envCoOwned) ? comm_rec.est_no : " 0");

	/*----------------------------------
	| Beginning of Input Control Loop. |
	----------------------------------*/
	while (prog_exit == 0) 
	{
		RecordUnlockFunc ();
		edit_exit = 0;
   		entry_exit = 0;
   		prog_exit = 0;
   		restart = 0;
		init_vars (1);	/*  set default values  */
		lcount [3] = 0;
		newReceipt = FALSE;
		search_ok = 1;
		chequeError = TRUE;
		strcpy (dflt_dep, "");
		for (i = 0; i < MAXLINES; i++)
		{
			store [i].amountLocPayment	= 0.00;
			store [i].amountFgnPayment	= 0.00;
			store [i].amountLocal		= 0.00;
		}

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading ( (allocFlag) ? 2 : 1);
		entry ( (allocFlag) ? 2 : 1);
		if (prog_exit || restart) 
			continue;

		init_vars (3);

		/*-------------------------------------
		| Enter/Edit Screen 3 Tabular Input . |
		-------------------------------------*/
		if (local_rec.alloc [0] == 'S')
		{
			heading (3);
	  		entry (3);
		}
		else
		{ 
			prog_status = ! (ENTRY);
			heading (3);
			scn_display (3);
			PrintTotal ();
	  		edit (3);
		}

		if (lcount [3] == 0) 
			restart = 1;

		if (restart) 
			continue;	

		edit_all ();

		if (restart) 
			continue;	

		ProofTrans ();

		if (restart) 
			continue;	

		/*-------------------------------------------
		| re-edit tabular if proof total incorrect. |
		-------------------------------------------*/
		while (chequeError)
		{
			edit_all ();
			if (restart)
				break;

			if (prog_exit)
			{
				prog_exit = 0;
				continue;
			}

			ProofTrans ();
		}

		if (!restart)
			Update ();
	}

	ArrDelete (&invDtls_d);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program Exit Sequence. |
========================*/
void
shutdown_prog (
 void)
{
	clear ();
	snorm ();
	if (!allocFlag)
	{
		print_at (0,0,ML (mlCrMess039),DOLLARS (batchTotal));
		PauseForKey (1, 0, ML (mlStdMess042), 0);
	}
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
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*------------------------------------------------
	| Find first day of current debtors module date. |
	------------------------------------------------*/
	DateToDMY (comm_rec.crd_date, NULL, &currentMonth, &currentYear);

	/*-----------------------------------------------------
	| Open work file used for cash cheque journals etc. |
	-----------------------------------------------------*/
	sprintf (filename,
		"%s/WORK/cr_csh%05d", 
		 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);
	cc = RF_OPEN (filename, sizeof (sudr_rec), "w", &sudrNumber);
	if (cc) 
		sys_err ("Error in work_file During (WKOPEN)",cc, PNAME);

	/*---------------------------------------
	| Open work file used for period calcs. |
	---------------------------------------*/
	sprintf (filename,
		"%s/WORK/cr_per%05d", 
		 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);
	cc = RF_OPEN (filename, sizeof (wk_rec), "w", &workRecordNumber);
	if (cc) 
		sys_err ("Error in cr_per During (WKOPEN)", cc, PNAME);

	/*----------------------
	| Open database files. |
	----------------------*/
	abc_alias (sudt2, sudt);
	abc_alias (suhd2, suhd);
	abc_alias (suin2, suin);
	abc_alias (suin3, suin);
	abc_alias (suin4, suin);
	abc_alias (sumr2, sumr);

	open_rec (bkcr,  bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
	open_rec (crbk,  crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (sudt,  sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (sudt2, sudt_list, SUDT_NO_FIELDS, "sudt_id_no");
	open_rec (suhd,  suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (suhd2, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (suin,  suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (suin2, suin_list, SUIN_NO_FIELDS, "suin_inv_no");
	open_rec (suin3, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (suin4, suin_list, SUIN_NO_FIELDS, "suin_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							     					      : "sumr_id_no3");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (suhp,  suhp_list, SUHP_NO_FIELDS, "suhp_id_no");
	open_rec (suht,  suht_list, SUHT_NO_FIELDS, "suht_id_no");

	OpenPocr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (bkcr);
	abc_fclose (crbk);
	abc_fclose (sudt);
	abc_fclose (sudt2);
	abc_fclose (suhd);
	abc_fclose (suhd2);
	abc_fclose (suin);
	abc_fclose (suin2);
	abc_fclose (suin3);
	abc_fclose (suin4);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (esmr);
	abc_fclose (suhp);
	abc_fclose (suht);
	GL_Close ();

	abc_dbclose (data);

	cc = RF_CLOSE (sudrNumber);
	if (cc) 
		file_err (cc, "sudrNumber", "WKCLOSE");

	cc = RF_CLOSE (workRecordNumber);
	if (cc) 
		file_err (cc, "cr_per", "WKCLOSE");
}
int
spec_valid (
 int field)
{
	int		i = 0;
	int		i_del;
	int		sav_scn;
	int		depositFound;

	long	max_fdate = 0L;
	
	int		no_lines;
	int		DepositFlag = FALSE;
	double	ActTotal = 0.00,
			DepTotal = 0.00;

	if (LCHECK ("exch_rate"))
	{
		if (local_rec.exch_rate <= 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			local_rec.rec_amt  = local_rec.rec_oamt;
			if (local_rec.bk_exch_rate != 0.00)
				local_rec.rec_amt = no_dec (local_rec.rec_amt 
										  / local_rec.bk_exch_rate);

			local_rec.rec_lamt = local_rec.rec_oamt;
			if (local_rec.exch_rate != 0.00)
				local_rec.rec_lamt = no_dec (local_rec.rec_lamt
										   / local_rec.exch_rate);

			DSP_FLD ("lrec_amt");
			
			local_rec.rec_gross = local_rec.rec_odis 
								+ local_rec.rec_oamt;
			local_rec.rec_ldis = local_rec.rec_odis;

			if (local_rec.exch_rate != 0.00)
			{
				local_rec.rec_gross = no_dec (local_rec.rec_gross 
									       /  local_rec.exch_rate);
				local_rec.rec_ldis  = no_dec (local_rec.rec_ldis
										   /  local_rec.exch_rate);
			}

			DSP_FLD ("rec_gross");

		if (prog_status != ENTRY)
		{
			sav_scn = cur_screen;
			scn_set (3);
			for (i = 0; i < lcount [3]; i++)
			{
				getval (i);
				local_rec.l_rec_exch = local_rec.exch_rate;
				if (local_rec.l_rec_exch != 0.00)
					local_rec.l_rec_lamt = no_dec (local_rec.l_rec_oamt 
										         / local_rec.l_rec_exch);
				else
					local_rec.l_rec_lamt = 0.00;
				store [i].amountLocPayment = local_rec.l_rec_lamt;
				putval (i);
			}
			scn_set (sav_scn);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------------
	| Validate Supplier Number And Allow Search. |
	--------------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (esmr_rec.co_no,  comm_rec.co_no);
			strcpy (esmr_rec.est_no,comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (!cc)
				strcpy (crbk_rec.bank_id, esmr_rec.dflt_bank);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (multiCurrency)
		{
			cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			bankLocalExchRate = pocrRec.ex1_factor;
			sprintf (local_rec.bk_rec_prmt, "Bank Deposit (%s)  : ",
					crbk_rec.curr_code);
		}
		else
		{
			strcpy (local_rec.bk_rec_prmt, "Bank Deposit        : ");
			bankLocalExchRate = 1.00;
		}
		display_prmpt (label ("rec_amt"));

		strcpy (local_rec.bk_curr_desc, pocrRec.description);
		DSP_FLD ("bk_name");
		DSP_FLD ("bk_curr_desc");
		DSP_FLD ("bk_curr");

		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Cheque Number. |
	--------------------------*/
	if (LCHECK ("cheque"))
	{
		/*-------------------------------------
		| allocFlag of unallocated cheque. |
		-------------------------------------*/
		if (allocFlag)
			return (ValidAllocation ());

	    if (dflt_used)
	    {
			if (automaticReceipt)
			{
				strcpy (local_rec.chq_no, "NEW CHEQUE NO. ");
				DSP_FLD ("cheque");
			}
			else
			{
				nextChequeNumber = atol (local_rec.prevCheque) + 1;
				sprintf (local_rec.chq_no, "%015ld", nextChequeNumber);
			}
	    }
		if (automaticReceipt && !dflt_used)
		{
		   		strcpy (local_rec.chq_no, "NEW CHEQUE NO. ");
		   		DSP_FLD ("cheque");
		}

	    if (prog_status == ENTRY)
	    	lcount [3] = 0;

	    return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Cheque Number. |
	--------------------------*/
	if (LCHECK ("dep_no"))
	{
	    if (SRCH_KEY)
	    {
			SrchDeposits (temp_str);
			return (EXIT_SUCCESS);
	    }	

	    /*-------------------------------
	    | Use deposit chosen in search. |
	    -------------------------------*/
	    if (dflt_used && strlen (dflt_dep) != 0)
			sprintf (local_rec.dep_no, "%15.15s", dflt_dep);

	    /*-----------------------
	    | Check deposit exists. |
	    -----------------------*/
	    depositFound = FALSE;
	    local_rec.unalloc_amt = 0.00;
	    local_rec.unalloc_loc = 0.00;
		sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
	    cc = find_rec (sudt, &sudt_rec, GTEQ,"r");
	    while (!cc && sudt_rec.hhsp_hash == suhd_rec.hhsp_hash)
	    {
			suin_rec.hhsi_hash	=	sudt_rec.hhsi_hash;
			cc = find_rec (suin3, &suin_rec, EQUAL, "r");
			if (!cc && !strcmp (suin_rec.inv_no, local_rec.dep_no))
			{
	    		depositFound = TRUE;
				local_rec.unalloc_amt += sudt_rec.amt_paid_inv;
				local_rec.unalloc_loc += sudt_rec.loc_paid_inv;
			}
	    	cc = find_rec (sudt, &sudt_rec, NEXT,"r");
	    }

	    if (!depositFound)
	    {
			print_mess (ML (mlCrMess146));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
	    }

	    if (local_rec.unalloc_amt == 0.00)
	    {
			print_mess (ML (mlCrMess147));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
	    }

	    DSP_FLD ("unalloc_amt");
	    DSP_FLD ("unalloc_loc");

	    return (EXIT_SUCCESS);
	}
	
	/*-------------------------
	| Validate Supplier Number. |
	-------------------------*/
	if (LCHECK ("supplier"))
	{
		/*-------------------------------------
		| allocFlag of unallocated cheque. |
		-------------------------------------*/
		if (allocFlag && last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		
		if (SRCH_KEY && search_key == OSEARCH)
		{
			abc_selfield (sumr,  "sumr_hhsu_hash");

			SrchSuin (temp_str);

			abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
		   	SumrSearch (comm_rec.co_no, branchNumber, temp_str);
		   	return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, (allocFlag) ? "r" : "u");
		if (cc)
		{
			abc_unlock (sumr);
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		if (sumr_rec.hold_payment [0] == 'Y')
		{
			abc_unlock (sumr);
			print_mess (ML (mlCrMess046));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		if (sumr_rec.remm_prn [0] == ' ')
			strcpy (local_rec.prt_remit, defaultRemit);
		else
			strcpy (local_rec.prt_remit, sumr_rec.remm_prn);

		if (allocFlag)
			DSP_FLD ("prt_remit2");
		else
			DSP_FLD ("prt_remit");

		if (MAN_CHQ || DRAFT_PAY)
		{
			if (sumr_rec.stat_flag [0] != 'S')
			{
				abc_unlock (sumr);
				print_mess (ML (mlCrMess047));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}

		/*-----------------------------------
		| allocFlag of unallocated cheque. |
		-----------------------------------*/
		if (allocFlag)
		{
			/*----------------------
			| Get payment details. |
			----------------------*/
			GetCheque (TRUE, sumr_rec.hhsu_hash);

			/*------------------
			| Display prompts. |
			------------------*/
			if (multiCurrency)
			{
				sprintf (local_rec.oamt_prmt, "Cheque Amount (%-3.3s): ", 
													sumr_rec.curr_code);

				sprintf (local_rec.unalloc_prmt, "Unallocated Amt (%-3.3s): ", 
													sumr_rec.curr_code);
			}
			else
			{
			       strcpy (local_rec.oamt_prmt, "Cheque Amount      : ");
			       strcpy (local_rec.unalloc_prmt,"Unallocated Amt     : ");
			}
			display_prmpt (label ("orec_amt2"));
			display_prmpt (label ("unalloc_amt"));

			DSP_FLD ("name2");
		
			/*---------------------------
			| Get currency description. |
			---------------------------*/
			if (multiCurrency)
			{
				cc = FindPocr (comm_rec.co_no, sumr_rec.curr_code, "r");
				if (cc)
					strcpy (pocrRec.description, "Unknown Currency");

				sprintf (local_rec.bk_curr_desc, "%-40.40s", pocrRec.description);
				local_rec.exch_rate = pocrRec.ex1_factor;
				DSP_FLD ("bk_curr2");
				DSP_FLD ("bk_curr_desc2");
			}
			else
				local_rec.exch_rate = 1.00;

			return (EXIT_SUCCESS);
		}
		
		/*---------------------------------------
		| Check currency code for current bank. |
		---------------------------------------*/
		if (multiCurrency)
		{
			strcpy (bkcr_rec.co_no,    comm_rec.co_no);
			strcpy (bkcr_rec.bank_id,  crbk_rec.bank_id);
			strcpy (bkcr_rec.curr_code, sumr_rec.curr_code);
			cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			local_rec.bk_exch_rate = bkcr_rec.ex1_factor;
		}
		else
			local_rec.bk_exch_rate = 1.00;

		/*-----------------
		| Set up prompts. |
		-----------------*/
		if (multiCurrency)
		{
			sprintf (err_str,  "%-15.15s (%s)", ML ("Cheque Amount"),
											   bkcr_rec.curr_code);
			strcpy (local_rec.oamt_prmt, err_str);

			sprintf (err_str,  "%-15.15s (%s)", ML ("Discount Amount"),
											   bkcr_rec.curr_code);
			strcpy (local_rec.odis_prmt, err_str);

			sprintf (err_str,  "%-15.15s (%s)", ML ("Bank exch rate"),
											   bkcr_rec.curr_code);
			strcpy (local_rec.bk_ex_prmt, err_str);
		}
		else
		{
			strcpy (local_rec.oamt_prmt, ML ("Cheque Amount       : "));
			strcpy (local_rec.odis_prmt, ML ("Discount Amount     : "));
		}

		display_prmpt (label ("orec_amt"));
		display_prmpt (label ("orec_dis"));
		display_prmpt (label ("bk_exch_rate"));

		/*--------------------------------
		| Check customers currency code. |
		--------------------------------*/
		if (multiCurrency)
		{
			cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");
			
			local_rec.exch_rate = n_dec ( (pocrRec.ex1_factor
								         * local_rec.bk_exch_rate), 8);
			DSP_FLD ("exch_rate");
			DSP_FLD ("bk_exch_rate");
		}
		else
			local_rec.exch_rate = 1.00;

		GetCheque (TRUE, sumr_rec.hhsu_hash);

		newReceipt = TRUE;
		if (! (automaticReceipt && !strcmp (local_rec.chq_no, "NEW CHEQUE NO. ")))
		{
		    cc = ReadCheques ();
		    if (cc)
		    {
				switch (cc)
				{
				case 1:
					print_mess (ML (mlCrMess145));
					break;

				case 2:
					print_mess (ML (mlCrMess048));
					break;
				break;
				}
				restart = TRUE;
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_SUCCESS); 
		    }
		}

		if (newReceipt && automaticReceipt)
		{
			strcpy (local_rec.chq_no, "NEW CHEQUE NO. ");
			DSP_FLD ("cheque");
		}
	
		if (useAlternateDate && strcmp (comm_rec.est_no, sumr_rec.est_no))
			alternateDate = ReadbranchDates (sumr_rec.est_no);
		else
			alternateDate = comm_rec.crd_date;

		DSP_FLD ("name");
			
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bk_exch_rate"))
	{
		/*---------------------------------------
		| Check currency code for current bank. |
		---------------------------------------*/
		if (dflt_used)
		{
			if (multiCurrency)
				local_rec.bk_exch_rate = bkcr_rec.ex1_factor;
			else
				local_rec.bk_exch_rate = 1.00;
		}
		if (prog_status != ENTRY)
		{
			local_rec.rec_amt  = local_rec.rec_oamt;
			if (local_rec.bk_exch_rate != 0.00)
				local_rec.rec_amt = no_dec (local_rec.rec_amt 
										  / local_rec.bk_exch_rate);
		}
		DSP_FLD ("rec_amt");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Overseas cheque amount. |
	-----------------------------------*/
	if (LCHECK ("orec_amt"))
	{
		if (FLD ("orec_amt") == NA || cur_screen == 2)
			return (EXIT_SUCCESS);

		local_rec.rec_amt  = local_rec.rec_oamt;
		if (local_rec.bk_exch_rate != 0.00)
			local_rec.rec_amt /= local_rec.bk_exch_rate;

		local_rec.rec_lamt = local_rec.rec_oamt;
		if (local_rec.exch_rate != 0.00)
			local_rec.rec_lamt = no_dec (local_rec.rec_lamt 
									   / local_rec.exch_rate);

		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (local_rec.exch_rate != 0.00)
		{
			local_rec.rec_gross = no_dec (local_rec.rec_gross
										/ local_rec.exch_rate);
			local_rec.rec_ldis  = no_dec (local_rec.rec_ldis
										/ local_rec.exch_rate);
		}

		DSP_FLD ("rec_gross");
		DSP_FLD ("rec_amt");
		DSP_FLD ("lrec_amt");
		
		if (prog_status != ENTRY)
		{
			if (local_rec.alloc [0] == 'A')
			{
				rv_pr (ML (mlCrMess049),2,21,45);
				move (0, 21);
				line (132);
				sleep (sleepTime);
				clear_mess ();
				if (local_rec.alloc [0] == 'A')
					ReadChequeAmount (TRUE);
				else
					ReadChequeAmount (FALSE);
			}
		}
	}

	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if (LCHECK ("orec_dis"))
	{
		local_rec.rec_gross = local_rec.rec_odis + local_rec.rec_oamt;
		local_rec.rec_ldis = local_rec.rec_odis;

		if (local_rec.exch_rate != 0.00)
		{
			local_rec.rec_gross = no_dec (local_rec.rec_gross
										/ local_rec.exch_rate);
			local_rec.rec_ldis  = no_dec (local_rec.rec_ldis
										/ local_rec.exch_rate);
		}

		DSP_FLD ("rec_gross");
	}

	/*-----------------------
	| Validate cheque Date. |
	-----------------------*/
	if (LCHECK ("rec_date"))
	{
		FLD ("pay_type") = YES;

		if (dflt_used)
			local_rec.rec_date = comm_rec.crd_date;

		DSP_FLD ("rec_date");

		if (local_rec.rec_date > local_rec.lsystemDate)
			return print_err (
			  ML ("Cheque date may NOT be Greater than current System date."));

		max_fdate = MonthEnd (comm_rec.crd_date) + 1L;
		if (local_rec.rec_date > MonthEnd (max_fdate))
			return print_err (
			  ML ("Cheque date cannot be Greater than one month Forward."));

		if (local_rec.rec_date < MonthStart (comm_rec.crd_date)) 
			return print_err (
			  ML ("Cheque date must be Greater than current Supplier month."));

		if (local_rec.rec_date > MonthEnd (comm_rec.crd_date) &&
		     forwardMessage == TRUE)
		{
			i = prmptmsg (ML (mlCrMess068), "YyNn", 1, 2);
			if (i == 'y' || i == 'Y')
			{
				i = prmptmsg (ML (mlCrMess069), "YyNn", 1, 2);
				forwardMessage = (i == 'y' || i == 'Y') ? FALSE
								     : TRUE;
			}
			else
			{
				move (1, 2); 
				line (131);
				return (EXIT_FAILURE);
			}
			move (1, 2); 
			line (131);
		}
	
		/*if (local_rec.rec_date > MonthEnd (comm_rec.crd_date))
		{*/
			/*------------------------------------
			| Only Cheques may be forward dated. |
			------------------------------------*/
			/*strcpy (local_rec.pay_type, "1");
			strcpy (local_rec.pay_type_desc, "1=Manual Chq.");
			FLD ("pay_type") = NA;
			DSP_FLD ("pay_type");
			DSP_FLD ("pay_type_desc");
		}*/
		return (EXIT_SUCCESS);
	}
			
	/*------------------------
	| Validate cheque type. |
	------------------------*/
	if (LCHECK ("pay_type"))
	{
		if (FLD ("pay_type") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			if (sumr_rec.pay_method [0] == 'C')
				strcpy (local_rec.pay_type, "1");

			if (sumr_rec.pay_method [0] == 'D')
				strcpy (local_rec.pay_type, "2");

			if (sumr_rec.pay_method [0] == 'T')
				strcpy (local_rec.pay_type, "5");
		}

		if (MAN_CHQ)
			strcpy (local_rec.pay_type_desc, "1=Manual Chq.");

		if (DEP_DRAFT)
			strcpy (local_rec.pay_type_desc, "4=Dep Draft. ");

		if (TRANSFER)
			strcpy (local_rec.pay_type_desc, "5=Transfer.  ");

		if (DRAFT_PAY)
			strcpy (local_rec.pay_type_desc, "2=Draft Paymt");

		if (DEP_CHQ)
			strcpy (local_rec.pay_type_desc, "3=Dep Cheque.");

		DSP_FLD ("pay_type_desc");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Allocation type. |
	---------------------------*/
	if (LCHECK ("alloc") || LCHECK ("alloc2"))
	{
		FLD ("selmonth") 	= NA;
		FLD ("selmonth2") 	= NA;
		FLD ("prt_remit") 	= (allocFlag) ? ND : NI;
		FLD ("prt_remit2") 	= (allocFlag) ? NI : ND;

		if (local_rec.alloc [0] == 'U')
		{
			strcpy (local_rec.alloc_desc,ML ("Unallocated             "));

			/*------------------------
			| Add dummy suin record. |
			------------------------*/
	    	abc_selfield (suin,"suin_id_no2");
			AddInvoices (TRUE, "1");
		
			/*-----------------------------
			| Add line to tabular screen. |
			-----------------------------*/
			scn_set (3);

			strcpy (local_rec.l_inv_no, suin_rec.inv_no);
			strcpy (local_rec.l_inv_type, "INV");

			local_rec.l_inv_bal = 0.00;
			local_rec.l_rec_oamt = local_rec.rec_oamt + local_rec.rec_odis;
			local_rec.l_inv_exch = suin_rec.exch_rate;
			strcpy (local_rec.l_curr, suin_rec.currency);
			strcpy (local_rec.l_fx, suin_rec.er_fixed);
			local_rec.l_loc_amt = 0.00;

			if (local_rec.exch_rate != 0.00)
				local_rec.l_rec_lamt = no_dec (local_rec.l_rec_oamt / local_rec.exch_rate);
			store [lcount [3]].amountFgnPayment = local_rec.l_rec_oamt;
			store [lcount [3]].amountLocPayment = local_rec.l_rec_lamt;
			store [lcount [3]].amountLocal  	= local_rec.l_loc_amt;
			local_rec.mon_inv = suin_rec.date_of_inv;
			putval (lcount [3]++);

			scn_set (1);

			return (EXIT_SUCCESS);
		}
		if (local_rec.alloc [0] == 'A')
		{
			strcpy (local_rec.alloc_desc,ML ("Amount cheque allocation"));
			ReadChequeAmount (TRUE);
		}
		if (local_rec.alloc [0] == 'R')
		{
			strcpy (local_rec.alloc_desc,ML ("Reverse date order.     "));
			ReadChequeAmount (FALSE);
		}
		if (local_rec.alloc [0] == 'S')
			strcpy (local_rec.alloc_desc,ML ("Selective document entry"));

		if (local_rec.alloc [0] == 'M')
		{
			strcpy (local_rec.alloc_desc,ML ("Month selection.        "));
			if (allocFlag)
				FLD ("selmonth2") = YES;
			else
				FLD ("selmonth") = YES;
		}
		if (allocFlag)
			DSP_FLD ("alloc_desc2");
		else
			DSP_FLD ("alloc_desc");
	
		return (EXIT_SUCCESS);
	}
	/*----------------------------------
	| Load Invoice for Selected Month. |
	----------------------------------*/
	if (LCHECK ("selmonth") || LCHECK ("selmonth2"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		abc_selfield (suin, "suin_cron");

		selectYear = (local_rec.sel_month <= currentMonth) ? currentYear : currentYear - 1;

		ReadInvoice (TRUE, sumr_rec.hhsu_hash);
	    if (workLineNo >= MAXLINES) 
	    {
			move (0,22);
			print_mess (ML (mlCrMess148));
			sleep (sleepTime);
			clear_mess ();
	    }
		lcount [3] = workLineNo;
		scn_set (1);
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ("l_invoice"))
	{
	   	abc_selfield (suin, "suin_id_no2");
	
		if (!strcmp (local_rec.l_inv_no,"kill           ") ||
		     !strcmp (local_rec.l_inv_no,"KILL           ")) 
		{ 
			if (lcount [3] == 0)
			{
				print_mess (ML (mlStdMess080));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			lcount [3] = line_cnt;
			heading (3);

			if (lcount [3] % TABLINES)
			{
				line_cnt = lcount [3] - 1;
				scn_display (3);
			}
			else
			{
				line_cnt = lcount [3];
				blank_display ();
			}
			PrintTotal ();
			return (EXIT_SUCCESS);
		}
	
		if (!strncmp (local_rec.l_inv_no, "del", 3) ||
		     !strncmp (local_rec.l_inv_no, "DEL", 3))
		{
			if (!strcmp (local_rec.l_inv_no,"del            ") ||
		 	     !strcmp (local_rec.l_inv_no,"DEL            "))
				i_del = 1;
			else
		    		i_del = atoi (local_rec.l_inv_no + 3);

		    	for (i = 0;i < i_del; i++)
			{
				if (DeleteLine (i_del - i))
					return (EXIT_FAILURE);
			}

			heading (3);
			scn_display (3);
			PrintTotal ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSuin2 (temp_str);
			return (EXIT_SUCCESS);
	    }

    	if (local_rec.sel_month <= currentMonth) 
			selectYear = currentYear;
	
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.est, (!envCoOwned) ? comm_rec.est_no :
							 			     sumr_rec.est_no);
    	strcpy (suin_rec.inv_no,local_rec.l_inv_no);

		invoiceAdded = FALSE;

    	cc = find_rec ("suin", &suin_rec, COMPARISON, "r");
    	if (cc) 
    	{
			if (!strncmp (suin_rec.inv_no, "DEPOSIT",7) || 
			     !strncmp (suin_rec.inv_no, "deposit",7))
			{
				/*-------------------------------------
				| allocFlag of unallocated cheque. |
				-------------------------------------*/
				if (allocFlag)
				{
					print_mess (ML (mlCrMess149));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				AddInvoices (TRUE, "1");
				DSP_FLD ("l_invoice");

				/*------------------------------------------------------------
				| Not an allocation so calculate the deposit total for user. |
				------------------------------------------------------------*/
				if (!allocFlag)
				{
					ActTotal = local_rec.rec_oamt + local_rec.rec_odis;

					DepTotal = 0.00;
					no_lines = (prog_status == ENTRY) ? line_cnt : 
														lcount [3] - 1;
					for (i = 0;i <= no_lines; i++)
						DepTotal += store [i].amountFgnPayment;
					local_rec.inv_balance = ActTotal - DepTotal;
					skip_entry = goto_field (field, label ("hhsiHash"));
					DepositFlag = TRUE;
				}
			}
			else
			{
				i = prmptmsg (ML (mlCrMess050),"YyNn",1,2);
			
				if (i == 'y' || i == 'Y')
				{
					move (1,2);		
					cl_line ();
					i = prmptmsg (ML (mlCrMess070),"IiCc",1,2);
					if (i == 'I' || i == 'i')
						AddInvoices (FALSE,"1");
					else
						AddInvoices (FALSE,"2");
				}
				else
				{
					move (1,2);
					cl_line ();
		   			return (EXIT_FAILURE);
				}
	       	}
	    }
		

		if (suin_rec.stat_flag [0] != 'S')
		{
			sprintf (err_str,ML (mlCrMess143));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		/*-----------------------
		| Invoice not Allrpved. |
		-----------------------*/
		if (suin_rec.approved [0] == 'N')
		{
			sprintf (err_str, ML (mlCrMess052),suin_rec.inv_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

	    local_rec.mon_inv = suin_rec.date_of_inv;

		if (!DepositFlag)
	    	CalcInvoiceBal ();

		local_rec.l_inv_bal = local_rec.inv_balance;
		local_rec.l_rec_oamt = local_rec.inv_balance;
		local_rec.l_inv_exch = suin_rec.exch_rate;
		local_rec.l_rec_exch = local_rec.exch_rate;
		strcpy (local_rec.l_curr, suin_rec.currency);
		strcpy (local_rec.l_fx, suin_rec.er_fixed);
		if (suin_rec.exch_rate != 0.00)
		{
			local_rec.l_loc_amt  = local_rec.inv_balance;
			local_rec.l_loc_amt /= suin_rec.exch_rate;
			local_rec.l_loc_amt = no_dec (local_rec.l_loc_amt);
		}
		if (local_rec.l_rec_exch != 0.00)
		{
			local_rec.l_rec_lamt = local_rec.inv_balance;
			local_rec.l_rec_lamt /= local_rec.l_rec_exch;
			local_rec.l_rec_lamt = no_dec (local_rec.l_rec_lamt);
		}
		store [line_cnt].amountFgnPayment 	= local_rec.l_rec_oamt;
		store [line_cnt].amountLocPayment  	= local_rec.l_rec_lamt;
		store [line_cnt].amountLocal 		= local_rec.l_loc_amt;

		if (suin_rec.type [0] == '1')
			strcpy (local_rec.l_inv_type, "INV");

		if (suin_rec.type [0] == '2')
			strcpy (local_rec.l_inv_type, "CRD");

		if (suin_rec.type [0] == '3')
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
		DSP_FLD ("l_rec_exch");

		putval (line_cnt);
	    	PrintTotal ();

	    	return (EXIT_SUCCESS);
	}
			
	/*---------------------------
	| Validate Payment Amount . |
	---------------------------*/
	if (LCHECK ("l_rec_oamt"))
	{
		if (dflt_used)
			local_rec.l_rec_oamt = local_rec.l_inv_bal;

		if (local_rec.exch_rate != 0.00)
		{
			local_rec.l_rec_lamt = local_rec.l_rec_oamt;
			local_rec.l_rec_lamt = no_dec (local_rec.l_rec_lamt
										/  local_rec.exch_rate);
		}
		store [line_cnt].amountFgnPayment	= local_rec.l_rec_oamt;
		store [line_cnt].amountLocPayment	= local_rec.l_rec_lamt;
		store [line_cnt].amountLocal		= local_rec.l_loc_amt;

		DSP_FLD ("l_rec_oamt");
		DSP_FLD ("l_rec_lamt");
		putval (line_cnt);
		PrintTotal ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============================================
| Read all invoices for current customer hash. |
==============================================*/
void
ReadInvoice (
 int	clear_tot,
 long	hhsu_hash)
{
	if (clear_tot)
	{
		print_at (23,0,ML (mlStdMess035));
		workLineNo = 0;
		scn_set (3);
	}
	suin_rec.hhsu_hash = hhsu_hash;
	strcpy (suin_rec.est,"  ");
	suin_rec.date_of_inv = 0L;

	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && suin_rec.hhsu_hash == hhsu_hash) 
	{
		/*-----------------------------------
		| Invoice not selected for payment. |
		-----------------------------------*/
		if (suin_rec.stat_flag [0] != 'S')
		{
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		/*-----------------------
		| Invoice not Allrpved. |
		-----------------------*/
		if (suin_rec.approved [0] == 'N')
		{
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		DateToDMY (suin_rec.date_of_inv, NULL, &invoiceMonth, &invoiceYear);
	    if (invoiceYear < selectYear || (invoiceMonth <= local_rec.sel_month && 
									invoiceYear <= selectYear)) 
	    {
		    CalcInvoiceBal ();
		    if (local_rec.inv_balance != 0.00) 
			{
				local_rec.l_rec_exch = local_rec.exch_rate;

				strcpy (local_rec.l_inv_no, suin_rec.inv_no);

				if (suin_rec.type [0] == '1')
					strcpy (local_rec.l_inv_type, "INV");

				if (suin_rec.type [0] == '2')
					strcpy (local_rec.l_inv_type, "CRD");

				if (suin_rec.type [0] == '3')
					strcpy (local_rec.l_inv_type, "JNL");

				local_rec.l_inv_bal = local_rec.inv_balance;
				local_rec.l_rec_oamt = local_rec.inv_balance;
				local_rec.l_inv_exch = suin_rec.exch_rate;
				strcpy (local_rec.l_curr, suin_rec.currency);
				strcpy (local_rec.l_fx, suin_rec.er_fixed);
				if (suin_rec.exch_rate != 0.00)
			    	local_rec.l_loc_amt = no_dec (local_rec.inv_balance / 
									      		  suin_rec.exch_rate);

				if (local_rec.l_rec_exch != 0.00)
					local_rec.l_rec_lamt = no_dec (local_rec.inv_balance / 
										   		   local_rec.l_rec_exch);

		      	store [workLineNo].amountFgnPayment = local_rec.l_rec_oamt;
		      	store [workLineNo].amountLocPayment = local_rec.l_rec_lamt;
		      	store [workLineNo].amountLocal 		= local_rec.l_loc_amt;
		      	local_rec.mon_inv = suin_rec.date_of_inv;
		      	putval (workLineNo);
		      	workLineNo++;
		    }
		}
		if (workLineNo >= MAXLINES)
		 	return;

		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
}

/*------------------------------
| Validate unallocated cheque |
| number for allocation.       |
------------------------------*/
int
ValidAllocation (
 void)
{
	if (SRCH_KEY)
	{
		SrchUnallocated (temp_str);
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate that cheque exists. |
	-------------------------------*/
	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suhd_rec.cheq_no, local_rec.chq_no);
	cc = find_rec (suhd2, &suhd_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlCrMess150));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", suhd_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlCrMess053));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*----------------------------------------
	| Cheque must be linked to at least one |
	| deposit which is not fully allocated.  |
	----------------------------------------*/
	if (!FindDeposits (FALSE, TRUE))
	{
		print_mess (ML (mlCrMess151));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	local_rec.exch_rate = n_dec ( (suhd_rec.tot_amt_paid / suhd_rec.loc_amt_paid), 8);
	local_rec.rec_oamt = suhd_rec.tot_amt_paid + suhd_rec.disc_taken;
	local_rec.rec_lamt = suhd_rec.loc_amt_paid + suhd_rec.loc_disc_take;

	DSP_FLD ("exch_rate2");
	DSP_FLD ("orec_amt2");
	DSP_FLD ("lrec_amt2");
	DSP_FLD ("unalloc_amt");

	return (EXIT_SUCCESS);
}

/*---------------------------------
| Search for unallocated cheque   |
| for current bank code.          |
---------------------------------*/
void
SrchUnallocated (
	char	*keyValue)
{

	_work_open (15,0,40);
	sprintf (err_str, "#Value Unallocated (%s) ", sumr_rec.curr_code);
	save_rec ("#Cheque         | Deposit", err_str);

	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (suhd_rec.cheq_no, "%-15.15s", keyValue);
	cc = find_rec (suhd2, &suhd_rec, GTEQ, "r");
	while (!cc && 
	       suhd_rec.hhsu_hash == sumr_rec.hhsu_hash &&
	       !strncmp (suhd_rec.cheq_no, keyValue, strlen (keyValue)))
	{
	    /*---------------------------------
	    | Find deposits for this receipt. |
	    ---------------------------------*/
	    FindDeposits (TRUE, FALSE);
	
	    cc = find_rec (suhd2, &suhd_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	/*--------------------------
	| Extract default deposit. |
	--------------------------*/
	sprintf (dflt_dep, "%15.15s", temp_str + 16);
	temp_str [17] = '\0';

	return;
}

/*------------------------------------
| Search for deposits for a receipt. |
------------------------------------*/
void
SrchDeposits (
	char	*keyValue)
{
	_work_open (15,0,40);
	sprintf (err_str, "#Value Unallocated (%s) ", sumr_rec.curr_code);
	save_rec ("#Deposit", err_str);

	/*------------------------------------
	| Find deposits for current receipt. |
	------------------------------------*/
	FindDeposits (FALSE, FALSE);

	cc = disp_srch ();
	work_close ();

	return;
}

/*------------------------------------
| Find deposits for current receipt. |
------------------------------------*/
int
FindDeposits (
	int	save_chq_no,
	int	chk_exists)
{
    int		first_dep;
    int		depositFound;
    long	prevHhsi = 0L;
    double	dep_value;
    char	amt_left [16];

    /*------------------------------------------------
    | Check for unallocated, linked to a DP invoice. |
    ------------------------------------------------*/
    depositFound = FALSE;
    first_dep = TRUE;
    dep_value = 0.00;
    sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
    sudt_rec.hhsi_hash = 0L;
    cc = find_rec (sudt2, &sudt_rec, GTEQ, "r");
    while (!cc && sudt_rec.hhsp_hash == suhd_rec.hhsp_hash)
    {
		/*----------------------
		| Change in hhsi_hash. |
		----------------------*/
		if (prevHhsi != sudt_rec.hhsi_hash || first_dep)
		{
	    	if (!first_dep)
	    	{
				suin_rec.hhsi_hash	=	prevHhsi;
				cc = find_rec (suin3, &suin_rec, COMPARISON, "r");
				if (!cc && !strncmp (suin_rec.inv_no, "DEPOSIT ", 8) &&
		    			dep_value != 0.00)
				{
		    		if (chk_exists)
						return (TRUE);

		    		sprintf (amt_left, "%8.2f", DOLLARS (dep_value));

		    		if (save_chq_no)
		    		{
						sprintf (err_str, "%15.15s %15.15s",
										suhd_rec.cheq_no,
										suin_rec.inv_no);
		    		}
		    		else
		    			sprintf (err_str, "%15.15s", suin_rec.inv_no);
		    		cc = save_rec (err_str, amt_left);
		    		if (cc)
						break;
				}
	    	}
	    	prevHhsi = sudt_rec.hhsi_hash;
	    	dep_value = 0.00;
	    	first_dep = FALSE;
		}
		dep_value += sudt_rec.amt_paid_inv;

		cc = find_rec (sudt2, &sudt_rec, NEXT, "r");
    }

    if (!first_dep)
    {
		suin_rec.hhsi_hash	=	prevHhsi;
		cc = find_rec (suin3, &suin_rec, COMPARISON, "r");
		if (!cc && !strncmp (suin_rec.inv_no, "DEPOSIT ", 8) && dep_value != 0.00)
		{
	    	if (chk_exists)
				return (TRUE);

	    	sprintf (amt_left, "%8.2f", DOLLARS (dep_value));

	    	if (save_chq_no)
	    	{
	    		sprintf (err_str, "%15.15s %15.15s", 
							suhd_rec.cheq_no, suin_rec.inv_no);
	    	}
	    	else
	    		sprintf (err_str, "%15.15s", suin_rec.inv_no);

	    	cc = save_rec (err_str, amt_left);
		}
    }

    return (FALSE);
}

void
ReadChequeAmount (
 int by_amount)
{
	int		pay_bal = FALSE;
	double	wk_amount = 0.00;

	/*-------------------------------------
	| allocFlag of unallocated receipts. |
	-------------------------------------*/
	if (allocFlag)
		wk_amount = local_rec.unalloc_amt;
	else
		wk_amount = local_rec.rec_oamt + local_rec.rec_odis;

	abc_selfield (suin,"suin_cron");

	workLineNo = 0;
	scn_set (3);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est,"  ");
	suin_rec.date_of_inv = 0L;
	print_mess ("Reading Invoices");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
        if (suin_rec.stat_flag [0] != 'S')
        {
            cc = find_rec (suin, &suin_rec, NEXT, "r");
            continue;
        }

		DateToDMY (suin_rec.date_of_inv, NULL, &invoiceMonth, &invoiceYear);

		CalcInvoiceBal ();
		
		if (no_dec (local_rec.inv_balance) == 0)  
		{
		   	cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}

		strcpy (local_rec.l_inv_no, suin_rec.inv_no);

		if (suin_rec.type [0] == '1')
			strcpy (local_rec.l_inv_type, "INV");

		if (suin_rec.type [0] == '2')
			strcpy (local_rec.l_inv_type, "CRD");

		if (suin_rec.type [0] == '3')
			strcpy (local_rec.l_inv_type, "JNL");

		if (by_amount && wk_amount < local_rec.inv_balance && local_rec.inv_balance > 0.00)
			pay_bal = TRUE;

		local_rec.l_inv_bal = local_rec.inv_balance;
		local_rec.l_rec_oamt = (pay_bal) ? wk_amount
					         : local_rec.inv_balance;
		local_rec.l_inv_exch = suin_rec.exch_rate;
		strcpy (local_rec.l_curr, suin_rec.currency);
		strcpy (local_rec.l_fx, suin_rec.er_fixed);
		if (suin_rec.exch_rate > 0.00)
		{
			local_rec.l_loc_amt  = local_rec.inv_balance;
			local_rec.l_loc_amt /= suin_rec.exch_rate;
			local_rec.l_loc_amt  = no_dec (local_rec.l_loc_amt);
		}

		local_rec.l_rec_exch = local_rec.exch_rate;

		if (local_rec.l_rec_exch > 0.00)
		{
			local_rec.l_rec_lamt = (pay_bal) ? wk_amount
							: local_rec.inv_balance;
			local_rec.l_rec_lamt /= local_rec.l_rec_exch;
		}
		store [workLineNo].amountLocPayment = local_rec.l_rec_lamt;
		store [workLineNo].amountFgnPayment = local_rec.l_rec_oamt;
		store [workLineNo].amountLocal 		= local_rec.l_loc_amt;

		local_rec.mon_inv = suin_rec.date_of_inv;
		wk_amount -= local_rec.inv_balance;
		putval (workLineNo);
		workLineNo++;
		    	
		if (pay_bal)
			break;

		if (workLineNo >= MAXLINES)
		   	break;

		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	if (workLineNo >= MAXLINES) 
	{
		move (0,22);
		print_mess (ML (mlCrMess152));
		sleep (sleepTime);
		clear_mess ();
	}
	lcount [3] = workLineNo;
	scn_set (1);
}

int
DeleteLine (
 int	del_no)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
	int	delta = 1;

	/*---------------------------------------
	| entry									|
	---------------------------------------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*-------------------------------------------------
	| no lines to delete or excessive lines to delete |
	-------------------------------------------------*/
	if ( (lcount [3] <= 0) ||
		 ( (line_cnt + del_no) > lcount [3]))
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	print_at (2,0,ML (mlStdMess014));
	fflush (stdout);

	for (i = line_cnt; line_cnt < lcount [3] - delta; line_cnt++)
	{
		getval (line_cnt + delta);
		putval (line_cnt);

		store [line_cnt].amountFgnPayment = store [line_cnt + delta].amountFgnPayment;
		store [line_cnt].amountLocPayment = store [line_cnt + delta].amountLocPayment;
		store [line_cnt].amountLocal 	  = store [line_cnt + delta].amountLocal;

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	while (line_cnt < lcount [3])
	{
		strcpy (local_rec.l_inv_no, "               ");
		strcpy (local_rec.l_curr, "   ");
		strcpy (local_rec.l_inv_type, "    ");
		strcpy (local_rec.l_fx, " ");
		local_rec.l_inv_bal = 0.00;
		local_rec.l_inv_exch = 0.00;
		local_rec.l_loc_amt = 0.00;
		local_rec.l_rec_oamt = 0.00;
		local_rec.l_rec_lamt = 0.00;
		store [line_cnt].amountFgnPayment 	= 0.00;
		store [line_cnt].amountLocPayment 	= 0.00;
		store [line_cnt].amountLocal 		= 0.00;
		
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			blank_display ();

		line_cnt++;
	}
	
	print_at (2,0,"                          ");
	fflush (stdout);

	/*---------------------------------------
	| blank last line - if required			|
	---------------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	lcount [3] -= delta;

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}
	
/*================================================
| Total invoice payments and determine balance . |
================================================*/
int
CalcInvoiceBal (
 void)
{
	int 	i;

/*---------------------------------------------------------------------------
|This is modified due to the computed negative invoice balance for non-     |
|negative balance invoices.  If payment is applied to a supplier invoice,   |
|the suin_amt and suin_pay_amt are not modified.  suin_amt_paid is modified.|
|But if a journal is applied to the invoice, suin_pay_amt is modified.      |
|It is modified based on suin_amt - suin_amt_paid.  suin_pay_amt now equals |
|the invoice balance.  But if you will pay this invoice, the balance will be| 
|computed as suin_pay_amt - payments&journals from sudt.  This causes wrong |
|invoice balance.                                                           |
|In amendment, suin_pay_amt can never be greater than suin_amt.  But if     |
|journal is applied, suin_pay_amt is possible to exceed suin_amt.  Thus, if |
|suin_pay_amt is greater than suin_amt, (caused by applied journals), sudts |
|must be deducted from suin_amt and not from suin_pay_amt because           |
|suin_pay_amt is already modified by the applied journal (s).                |
---------------------------------------------------------------------------*/ 

	/*
	 * for each invoice, print details if dbt - crd <> 0. 
	 */
	local_rec.inv_balance = (suin_rec.pay_amt + suin_rec.amt_paid == suin_rec.amt) ? no_dec (suin_rec.amt) : no_dec (suin_rec.pay_amt);

	for (i = 0;i < invCnt;i++)
	{
		if (suin_rec.hhsi_hash == invDtls [i].hhsiHash)
			local_rec.inv_balance -= invDtls [i].inv_oamt;
	}
   	return (EXIT_SUCCESS);
}
	
/*------------------------
| Read existing receipt. |
------------------------*/
int
ReadCheques (
 void)
{
	abc_selfield ("suhd","suhd_id_no");

	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suhd_rec.cheq_no, local_rec.chq_no);
	cc = find_rec ("suhd", &suhd_rec, COMPARISON, "w");
	if (cc) 
	{
		newReceipt = TRUE;
		suhd_rec.tot_amt_paid   = 0.00;
		suhd_rec.loc_amt_paid   = 0.00;
		suhd_rec.disc_taken     = 0.00;
		suhd_rec.loc_disc_take  = 0.00;
	}
	else 
	{
		sprintf (err_str, ML (mlCrMess054), DOLLARS (suhd_rec.tot_amt_paid));
		print_mess (err_str);

		sleep (sleepTime);
		clear_mess ();
		newReceipt = FALSE;
	}	

	abc_selfield ("suhd", "suhd_hhsu_hash");
	if (!newReceipt)
	{
		if (multiCurrency)
			return (EXIT_FAILURE);

		if (suhd_rec.date_payment < MonthStart (comm_rec.crd_date))
			return (2);
	}
	return (EXIT_SUCCESS);
}

/*=======================
| Print payments total. |
=======================*/
int
PrintTotal (
 void)
{
	int 	i; 
	int		no_lines = (prog_status == ENTRY) ? line_cnt : lcount [3] - 1;
	double	rec_lamt = 0.00,
			rec_oamt = 0.00,
			all_amt = 0.00,
			local = 0.00;


	for (i = 0; i <= no_lines; i++) 
	{
		rec_lamt += store [i].amountLocPayment;
		rec_oamt += store [i].amountFgnPayment;
		local    += store [i].amountLocal;
	}

	rec_oamt = DOLLARS (rec_oamt);
	rec_lamt = DOLLARS (rec_lamt);
	local    = DOLLARS (local);
	all_amt  = DOLLARS (local_rec.rec_oamt + local_rec.rec_odis);

	/*-------------------------------------
	| allocFlag of unallocated receipts. |
	-------------------------------------*/
	if (allocFlag)
		sprintf (err_str, ML (mlCrMess055),DOLLARS (local_rec.unalloc_amt));
	else
		sprintf (err_str, ML (mlCrMess056),all_amt);
	
	rv_pr (err_str, 2, 18, 1);

	if (multiCurrency)
	{
		line_at (17,65, 17);
		line_at (17,83, 17);
		line_at (17,114,17);
		print_at (18, 68, "%14.2f", local);
		print_at (18, 84, "%R %14.2f", rec_oamt);
		print_at (18, 116, "%14.2f", rec_lamt);

		line_at (19,65,17);
		line_at (19,83,17);
		line_at (19,114,17);
	}
	else
	{
		line_at (19,56,17);
		line_at (19,75,17);
		line_at (17,56,17);
		line_at (17,75,17);
		print_at (18, 62, "%8.2f", local);
		print_at (18, 80, "%R %8.2f", rec_oamt);
	}

	return (EXIT_SUCCESS);
}

/*======================
| Main update routine. |
======================*/
int
Update (
 void)
{
	int 	per_num = 0;
	int		rcpt_prd;
	double	l_var = 0.00;
	double	exch_variation;

	DateToDMY (local_rec.rec_date, NULL, &rcpt_prd, NULL);

	clear ();
	print_at (1,1,ML (mlStdMess035));

	abc_selfield (suhd,"suhd_id_no");

	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;

	/*--------------------------------------
	| Generate receipt number if required. |
	--------------------------------------*/
	if (automaticReceipt && !allocFlag && newReceipt)
	{
		open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
		strcpy (comr_rec.co_no, comm_rec.co_no);
	    cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	    if (cc)
		{
			abc_unlock (comr);
			file_err (cc, comr, "DBFIND");
		}

		comr_rec.nx_chq_no++;
		sprintf (local_rec.chq_no, "%015ld", comr_rec.nx_chq_no);
		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		abc_fclose (comr);
	}
	
	/*----------------------------------------
	| Add or update Cheque Payment Header . |
	----------------------------------------*/

	if (newReceipt == FALSE) 
	{
		cc = find_rec (suhd, &suhd_rec, COMPARISON , "r");
		if (!allocFlag)
		{
			suhd_rec.tot_amt_paid  += no_dec (local_rec.rec_oamt);
			suhd_rec.disc_taken    += no_dec (local_rec.rec_odis);
	
			suhd_rec.loc_amt_paid  += no_dec (local_rec.rec_lamt);
			suhd_rec.loc_disc_take += no_dec (local_rec.rec_ldis);
		}
	}
	else 
	{
		suhd_rec.tot_amt_paid  = no_dec (local_rec.rec_oamt);
		suhd_rec.disc_taken    = no_dec (local_rec.rec_odis);

		suhd_rec.loc_amt_paid  = no_dec (local_rec.rec_lamt);
		suhd_rec.loc_disc_take = no_dec (local_rec.rec_ldis);
	}

	if (!allocFlag)
	{
        strcpy (suhd_rec.pay_type, "1");
		strcpy (suhd_rec.cheq_no, local_rec.chq_no);
		strcpy (suhd_rec.bank_id,    crbk_rec.bank_id);
		suhd_rec.bank_amt = local_rec.rec_amt;
		suhd_rec.bank_exch = local_rec.bk_exch_rate;
		suhd_rec.bank_chg = local_rec.bank_chg;
		sprintf (suhd_rec.pay_type, "%-1.1s", local_rec.pay_type);
		strcpy (suhd_rec.narrative, local_rec.narrative);
		suhd_rec.date_payment = local_rec.rec_date;
		suhd_rec.date_post = StringToDate (local_rec.systemDate);

		/* This should pick up clear fee from comr maybe. */
		suhd_rec.clear_fee = 0.00;
	}

	/*-------------------------------
	| Calculate exchange variation. |
	-------------------------------*/
	scn_set (3);
	exch_variation = 0.00;
	for (workLineNo = 0; workLineNo < lcount [3]; workLineNo++) 
	{
	    getval (workLineNo);
	    if (local_rec.l_inv_exch != 0.00)
			l_var = local_rec.l_rec_oamt / local_rec.l_inv_exch;
	    exch_variation += no_dec (l_var - local_rec.l_rec_lamt);
	}
	scn_set (1);

	suhd_rec.pid	=	pidNumber;

	if (local_rec.prt_remit [0] == 'Y')
		strcpy (suhd_rec.rem_prt, "R");
	else
		strcpy (suhd_rec.rem_prt, "0");

	if (newReceipt == FALSE) 
	{
		suhd_rec.exch_variance += exch_variation;
		cc = abc_update (suhd, &suhd_rec);
		if (cc) 
			file_err (cc, suhd, "DBUPDATE");
	}
	else 
	{
		/*-----------------------------------------------
		| Add new payment header and get hhcp hash ref. |
		-----------------------------------------------*/
		suhd_rec.exch_variance = exch_variation;
		strcpy (suhd_rec.stat_flag, "0");
		cc = abc_add (suhd, &suhd_rec);
		if (cc) 
			file_err (cc, suhd, "DBADD");

		abc_unlock (suhd);

		cc = find_rec (suhd, &suhd_rec, COMPARISON , "r");
		if (cc) 
			file_err (cc, suhd, "DBFIND");
	}

	abc_selfield (suhd, "suhd_hhsu_hash");

	/*-------------------------------------
	| Add sudt to reverse deposit for the |
	| value allocated to invoices.        |
	-------------------------------------*/
	if (allocFlag)
	{
		/*-----------------------------
		| Find deposit (suin) record. |
		-----------------------------*/
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.est, (!envCoOwned) ? comm_rec.est_no 
						                    : sumr_rec.est_no);
		sprintf (suin_rec.inv_no, "%-15.15s", local_rec.dep_no);
		cc = find_rec (suin4, &suin_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, suin4, "DBFIND");

		memcpy (&suin2_rec, &suin_rec, sizeof (suin_rec));

		local_rec.l_rec_lamt = allocationTotal;
		if (local_rec.exch_rate != 0.00)
			local_rec.l_rec_lamt = no_dec (local_rec.l_rec_lamt
										 / local_rec.exch_rate);

		/*------------------------------------
		| Do not update sudr if an argument  |
		| is passed to CSHINPUT (Journal).   |
		------------------------------------*/
		if (!notJournalOnly) 
			AddSudr (rcpt_prd, TRUE);

		/*------------------
		| Add sudt record. |
		------------------*/
    	sudt_rec.hhsp_hash 	 = suhd_rec.hhsp_hash;
		sudt_rec.hhsi_hash    = suin_rec.hhsi_hash;
		sudt_rec.amt_paid_inv = no_dec (allocationTotal * -1.00);
		sudt_rec.loc_paid_inv = no_dec (local_rec.l_rec_lamt * -1.00);
		sudt_rec.exch_rate    = local_rec.exch_rate;
		sudt_rec.exch_variatio     = 0.00;
		
		strcpy (sudt_rec.stat_flag, "0");
		
		cc = abc_add (sudt, &sudt_rec);
		if (cc) 
			file_err (cc, sudt, "DBADD");
	}

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est, comm_rec.est_no);
    sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
	scn_set (3);
	
	/*---------------------------------------------------
	| Set up period and selpay as no longer in tabular. |
	---------------------------------------------------*/
	for (workLineNo = 0; workLineNo < lcount [3]; workLineNo++) 
	{
	    getval (workLineNo);
	    /*------------------------------------
	    | Do not update sudr if an argument  |
	    | is passed to CRCINPUT (Journal).   |
	    ------------------------------------*/
	    if (!notJournalOnly) 
			AddSudr (rcpt_prd, FALSE);

	    if (local_rec.l_inv_exch != 0.00)
		l_var = local_rec.l_rec_oamt / local_rec.l_inv_exch;

    	sudt_rec.hhsp_hash 	 = suhd_rec.hhsp_hash;
	    sudt_rec.hhsi_hash    = suin_rec.hhsi_hash;
	    sudt_rec.amt_paid_inv = no_dec (local_rec.l_rec_oamt);
	    sudt_rec.loc_paid_inv = no_dec (local_rec.l_rec_lamt);
	    sudt_rec.exch_rate    = local_rec.exch_rate;
	    sudt_rec.exch_variatio     = no_dec (l_var - local_rec.l_rec_lamt);

	    strcpy (sudt_rec.stat_flag, "0");

	    cc = abc_add (sudt, &sudt_rec);
	    if (cc) 
			file_err (cc, sudt, "DBADD");

		/*-------------------------------------------------------
		| Subtract payment from current balance owing etc	|
		-------------------------------------------------------*/
		DateToDMY (suin_rec.date_of_inv, NULL, &per_num, NULL);
		if (per_num > currentMonth)
			per_num -= 12;

		per_num -= currentMonth;
		if (per_num < 0)
			per_num *= -1;

		if (per_num > 3)
			per_num = 3;

		sumr_per [per_num] -= no_dec (local_rec.l_rec_oamt);
	}
	/*------------------------------
    | update customer aged amount. |
	------------------------------*/
	batchTotal += local_rec.rec_oamt;

	if (!allocFlag)
	{
		cc = abc_update (sumr,&sumr_rec);
		if (cc) 
	    		file_err (cc, sumr, "DBUPDATE");
	}

	wk_rec.hhsuHash = sumr_rec.hhsu_hash;
	cc = RF_ADD (workRecordNumber, (char *) &wk_rec);
	if (cc) 
	    file_err (cc, "wkno", "WKADD");

	strcpy (local_rec.prevCheque, local_rec.chq_no);
	strcpy (local_rec.prevSupplierNo,  local_rec.crd_no);

	if (!allocFlag)
		MakeCheqHist ();

	return (EXIT_SUCCESS);
}

/*=======================================================
|	Routine to create suhp, suht records. Creates   	|
|	header then rereads it (for the hash) and			|
|	then creates the suht records.						|
|	Returns: 0 if ok, non-zero otherwise.				|
=======================================================*/
void
MakeCheqHist (
 void)
{
	int		i;
	int		est_no;
	int		NewSuhp;

	/*---------------------------
	| Initialise total array	|
	----------------------------*/
	for (i = 0;i < MAXEST; i++)
	{
		est_total [i] = 0.0;
		est_local [i] = 0.0;
	}

	/*-----------------------------------------------------------------
	|  Sum cheque details into branchNumber And create glwk records |
	-----------------------------------------------------------------*/
	sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
	cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
	{
		suin_rec.hhsi_hash = sudt_rec.hhsi_hash;
		cc = find_rec (suin3, &suin_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (suin_rec.est, "00");
			strcpy (suin_rec.inv_no, "Unknown Invoice");
		}
		est_no = atoi (suin_rec.est);
		est_total [est_no] += no_dec (sudt_rec.amt_paid_inv);
		est_local [est_no] += no_dec (sudt_rec.loc_paid_inv);
		cc = find_rec (sudt, &sudt_rec, NEXT, "r");
	}
	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, suhd_rec.cheq_no);
	NewSuhp = find_rec (suhp, &suhp_rec, COMPARISON, "r");

	strcpy (suhp_rec.payee_name, sumr_rec.crd_name);
	strcpy (suhp_rec.payee_acr, sumr_rec.acronym);
	strcpy (suhp_rec.narrative, suhd_rec.narrative);
	suhp_rec.date_payment 	= suhd_rec.date_payment;
	suhp_rec.date_post 		= local_rec.lsystemDate;
	suhp_rec.tot_amt_paid   = no_dec (suhd_rec.tot_amt_paid);
	suhp_rec.loc_amt_paid   = no_dec (suhd_rec.loc_amt_paid);
	suhp_rec.disc_taken     = no_dec (suhd_rec.disc_taken);
	suhp_rec.loc_disc_take 	= no_dec (suhd_rec.loc_disc_take);
	strcpy (suhp_rec.bank_id, suhd_rec.bank_id);
	strcpy (suhp_rec.pay_type, suhd_rec.pay_type);
	strcpy (suhp_rec.stat_flag, "0");
	if (NewSuhp)
	{
		cc = abc_add (suhp, &suhp_rec);
		if (cc)
			file_err (cc, suhp, "DBADD");

		strcpy (suhp_rec.co_no, comm_rec.co_no);
		strcpy (suhp_rec.cheq_no, suhd_rec.cheq_no);
		cc = find_rec (suhp, &suhp_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, suhp, "DBFIND");
	}
	else
	{
		cc = abc_update (suhp, &suhp_rec);
		if (cc)
			file_err (cc, suhp, "DBUPDATE");
	}
	for (i = 0; i < MAXEST; i++)
	{
		if (allocFlag)
		{
			suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
			sprintf (suht_rec.est_no, "%2d", i);
			cc = find_rec (suht, &suht_rec, COMPARISON, "u");
			if (cc)
				abc_unlock (suht);
			else
			{
				cc = abc_delete (suht);
				if (cc)
					file_err (cc, suht, "DBADD");
			}
		}
		if (est_total [i] != 0.0)
		{
			suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
			sprintf (suht_rec.est_no, "%2d", i);
			suht_rec.est_amt_paid = no_dec (est_total [i]);
			suht_rec.est_loc_amt  = no_dec (est_local [i]);
			strcpy (suht_rec.stat_flag, "0");
			cc = abc_add (suht, &suht_rec);
			if (cc)
				file_err (cc, suht, "DBADD");
		}
	}
}

void
AddSudr (
 int rcpt_prd,
 int rvs_dep)
{
	strcpy (sudr_rec.cr_co_no,   comm_rec.co_no);
	strcpy (sudr_rec.cr_br_no,   comm_rec.est_no);
	strcpy (sudr_rec.cr_chq_no,  local_rec.chq_no);
	sprintf (sudr_rec.cr_period, "%02d", rcpt_prd);
	sudr_rec.cr_hhsp_hash = suhd_rec.hhsp_hash;

	sprintf (sudr_rec.cr_bank_id,   "%-5.5s",   crbk_rec.bank_id);
	sprintf (sudr_rec.cr_bank_name, "%-40.40s", crbk_rec.bank_name);
	sprintf (sudr_rec.cr_bk_ccode,  "%-3.3s",   crbk_rec.curr_code);
	sprintf (sudr_rec.cr_bk_cdesc,  "%-40.40s", local_rec.bk_curr_desc);
	sprintf (sudr_rec.cr_crd_no,    "%-6.6s",   local_rec.crd_no);
	sprintf (sudr_rec.cr_crd_name , "%-40.40s", sumr_rec.crd_name);
	sprintf (sudr_rec.cr_pay_type,  "%-1.1s",   suhd_rec.pay_type);

	GL_GLI 
	(
		sudr_rec.cr_co_no,
		sudr_rec.cr_br_no,
		"  ",
		"DISC ALLOW",
		"   ",
		" "
	);
			  		
	sprintf (sudr_rec.cr_gl_disc,   "%-*.*s", MAXLEVEL,MAXLEVEL, GL_Account);
	strcpy (sudr_rec.cr_narrative,  local_rec.narrative);

	sudr_rec.cr_hhsu_hash = sumr_rec.hhsu_hash;
	sudr_rec.cr_rec_date  = local_rec.rec_date;
	sudr_rec.cr_o_exch    = local_rec.exch_rate;
	strcpy (sudr_rec.cr_o_curr, sumr_rec.curr_code);

	if (allocFlag)
	{
		sudr_rec.cr_bk_exch = 1.00;
		sudr_rec.cr_bk_rec_amt = 0.00;
		sudr_rec.cr_bk_charge = 0.00;
		sudr_rec.cr_bk_l_exch = 1.00;
		sudr_rec.cr_o_disc      = 0.00;
		sudr_rec.cr_o_total_amt = 0.00;
		sudr_rec.cr_l_disc      = 0.00;
		sudr_rec.cr_l_total_amt = 0.00;
		if (rvs_dep)
		{
		    sprintf (sudr_rec.cr_invoice, "%-15.15s", local_rec.dep_no);
		    sudr_rec.cr_inv_exch  = local_rec.exch_rate;
		    sudr_rec.cr_inv_amt   = 0.00;
		    sudr_rec.cr_o_amt_pay = no_dec (allocationTotal * -1.00);
		    sudr_rec.cr_l_amt_pay = no_dec (local_rec.l_rec_lamt * -1.0);
		}
		else
		{
		    sprintf (sudr_rec.cr_invoice, "%-15.15s", local_rec.l_inv_no);
		    sudr_rec.cr_inv_exch  = local_rec.l_inv_exch;
		    sudr_rec.cr_inv_amt   = no_dec (local_rec.l_inv_bal);
		    sudr_rec.cr_o_amt_pay = no_dec (local_rec.l_rec_oamt);
		    sudr_rec.cr_l_amt_pay = no_dec (local_rec.l_rec_lamt);
		}
	}
	else
	{
		sprintf (sudr_rec.cr_invoice, "%-15.15s", local_rec.l_inv_no);
		sudr_rec.cr_bk_exch 	= suhd_rec.bank_exch;
		sudr_rec.cr_bk_rec_amt 	= no_dec (local_rec.rec_amt);
		sudr_rec.cr_bk_charge 	= no_dec (local_rec.bank_chg);
		sudr_rec.cr_bk_l_exch 	= bankLocalExchRate;
		sudr_rec.cr_inv_exch 	= local_rec.l_inv_exch;
		sudr_rec.cr_inv_amt  	= no_dec (local_rec.l_inv_bal);
		sudr_rec.cr_o_disc      = no_dec (local_rec.rec_odis);
		sudr_rec.cr_o_total_amt = no_dec (local_rec.rec_oamt);
		sudr_rec.cr_o_amt_pay   = no_dec (local_rec.l_rec_oamt);
		sudr_rec.cr_l_disc      = no_dec (local_rec.rec_ldis);
		sudr_rec.cr_l_total_amt = no_dec (local_rec.rec_lamt + local_rec.rec_ldis);
		sudr_rec.cr_l_amt_pay   = no_dec (local_rec.l_rec_lamt);
	}

	cc = RF_ADD (sudrNumber, (char *) &sudr_rec);
	if (cc) 
		file_err (cc, "sudr_rec", "WKADD");
}

/*=========================================
| Search routine for Supplier Bank File. |
=========================================*/
void
SrchCrbk (
	char	*keyValue)
{
	_work_open (6,0,40);
	save_rec ("#Bank ","#Bank Name ");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, keyValue);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,keyValue,strlen (keyValue)) && 
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

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND");
}
int
ProofTrans (
 void)
{
	int		i;

	double	head_tot  = 0.00;
	double	head_loc  = 0.00;
	double	alloc_loc = 0.00;

	/*-----------------------------------
	| allocFlag of unallocated cheque. |
	-----------------------------------*/
	if (allocFlag)
		head_tot = local_rec.unalloc_amt;
	else
		head_tot = local_rec.rec_oamt + local_rec.rec_odis;

	allocationTotal = 0.00;
	for (i = 0;i < lcount [3]; i++)
		allocationTotal += store [i].amountFgnPayment;

	proofTotal = no_dec (allocationTotal - head_tot);

	if (proofTotal == 0.00)
		chequeError = FALSE;
	else
	{
		/*-----------------------------------
		| allocFlag of unallocated cheque. |
		-----------------------------------*/
		if (allocFlag && proofTotal < 0.00)
		{
			sprintf (err_str,ML (mlCrMess057), DOLLARS (proofTotal * -1.00));
			i = prmptmsg (err_str, "YyNn", 25, 21);
		
			move (0, 21);
			cl_line ();
			if (i == 'Y' || i == 'y')
			{
				chequeError = FALSE;
				return (EXIT_SUCCESS);
			}
		}

		chequeError = TRUE;
		print_at (23, 0, ML (mlCrMess058), (allocFlag) ? sumr_rec.curr_code : bkcr_rec.curr_code,
			DOLLARS (head_tot), 
			DOLLARS (allocationTotal));
		sleep (sleepTime);
		clear_mess ();
	}

	/*-----------------------------------------------------------
	| Check for rounding errors in local values and round header |
	| local amount accordingly.                                  |
	------------------------------------------------------------*/
	if (allocFlag)
		head_loc = local_rec.unalloc_loc;
	else
		head_loc = local_rec.rec_lamt + local_rec.rec_ldis;

	alloc_loc = 0.00;
	for (i = 0;i < lcount [3]; i++)
		alloc_loc += store [i].amountLocPayment;

	proofTotal = no_dec (alloc_loc - head_loc);

	if (proofTotal == 0.00)
		chequeError = FALSE;
	else
	{
		if (allocFlag)
		{
			chequeError = TRUE;
			sprintf (err_str,ML (mlCrMess059),DOLLARS (head_loc),DOLLARS (alloc_loc));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
		}
		else
		{
			chequeError = TRUE;
			sprintf (err_str,ML (mlCrMess059),DOLLARS (head_loc),DOLLARS (alloc_loc));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			sprintf (err_str,ML (mlCrMess153),DOLLARS (proofTotal),DOLLARS (alloc_loc));
			i = prmptmsg (err_str, "YyNn", 25, 20);
			move (0, 20);
			line (132);
			if (i == 'Y' || i == 'y')
			{
				local_rec.rec_lamt += proofTotal;
				chequeError = FALSE;
			}
		}
	}	
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		clear ();
		if (scn != cur_screen)
			scn_set (scn);

		if (!notJournalOnly) 
			rv_pr (ML (mlCrMess060),42,0,1);
		else
			rv_pr (ML (mlCrMess061),36,0,1);

		print_at (0,82,ML (mlCrMess062),local_rec.prevCheque,local_rec.prevSupplierNo);
		move (0,1);
		line (132);

		pr_box_lines (scn);

		/*-------------------------------------
		| allocFlag of unallocated cheque. |
		-------------------------------------*/
		if (allocFlag)
			us_pr (ML (mlCrMess063), 48, 2, 1);

		if (scn == 3)
		{
			box (10, 2, 110, 1);
			print_at (3, 11, ML (mlCrMess064), crbk_rec.bank_id);
			print_at (3, 50, ML (mlCrMess065), crbk_rec.bank_name);

			PrintTotal ();
		}
		strcpy (err_str,ML (mlStdMess038));
		print_at (22,0,err_str,comm_rec.co_no,comm_rec.co_name);

		if (tabscn) 
		{
			strcpy (err_str,ML (mlCrMess066));
			print_at (22, 60, err_str,sumr_rec.crd_no,sumr_rec.crd_name);
		}
		else 
		{
			strcpy (err_str,ML (mlStdMess039));
			print_at (22, 60, err_str,comm_rec.est_no,comm_rec.est_name);
		}

		/*  reset this variable (line_cnt) for new screen NOT page	*/
		if (scn != cur_screen)
			line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
RecordUnlockFunc (
 void)
{
	abc_unlock (sumr);
	abc_unlock (suhd);
	abc_unlock (suin);
	abc_unlock (sudt);
	return (EXIT_SUCCESS);
}

/*===========================
| Add dummy invoice number. |
===========================*/
void
AddInvoices (
 int	dep,
 char *	type)
{
	int		workInvoice;
	
	workInvoice = 1;

	while (1)
	{
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		sprintf (suin_rec.type,"%-1.1s", type);
		strcpy (suin_rec.est, (!envCoOwned) ? comm_rec.est_no 
						   : sumr_rec.est_no);

		if (dep)
			sprintf (suin_rec.inv_no, "DEPOSIT # %04d", workInvoice);
		else
			sprintf (suin_rec.inv_no, "%-15.15s", local_rec.l_inv_no);

		strcpy (suin_rec.narrative,"                    ");
		suin_rec.date_of_inv = local_rec.rec_date;
		suin_rec.date_posted = local_rec.rec_date;
		suin_rec.exch_rate = local_rec.exch_rate;
		strcpy (suin_rec.currency, bkcr_rec.curr_code);
		strcpy (suin_rec.er_fixed, "N");
		suin_rec.pay_amt = 0.00;
		suin_rec.amt = 0.00;
		strcpy (suin_rec.stat_flag,"S");

        	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
		if (cc)
			break;

		workInvoice++;
	}
	strcpy (local_rec.l_inv_no, suin_rec.inv_no);
	if (!dep)
		DSP_FLD ("l_invoice");

	cc = abc_add (suin,&suin_rec);
	if (cc) 
		file_err (cc, suin, "DBADD");

	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc) 
		file_err (cc, suin, "DBFIND");

	invoiceAdded = TRUE;
}

void
SrchSuin2 (
	char	*keyValue)
{
	_work_open (16,0,40);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est,"  ");
	strcpy (suin_rec.inv_no,keyValue);
	save_rec ("#Invoice No     ","#Supplier (Acronym)Date Inv / Amount Owing.   ");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no,keyValue,strlen (keyValue)) && 
		      suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if (suin_rec.stat_flag [0] != 'S')
		{
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
	   	CalcInvoiceBal ();
	   	if (local_rec.inv_balance != 0.00)
	   	{
			if (sumr2_rec.hhsu_hash != suin_rec.hhsu_hash)
			{
				sumr2_rec.hhsu_hash	=	suin_rec.hhsu_hash;
				cc = find_rec (sumr2, &sumr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, "sumr2", "DBFIND");
			}

			sprintf (err_str, "%s (%-9.9s)%s / %.2f",
								sumr2_rec.crd_no,
								sumr2_rec.acronym,
								DateToString (suin_rec.date_of_inv),
								DOLLARS (local_rec.inv_balance));

			cc = save_rec (suin_rec.inv_no,err_str);
			if (cc)
				break;
	    }
	    cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est,"  ");
	strcpy (suin_rec.inv_no,temp_str);
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");

	CalcInvoiceBal ();
}

/*==================================================================
| Routine to get cheque details and hold relevent invoice Against. |
==================================================================*/
int
GetCheque (
 int	clear_tot,
 long	hhsuHash)
{
	if (clear_tot)
	{
		invCnt = 0;
		abc_selfield (suhd, "suhd_hhsu_hash");
	}

	suhd_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
    while (!cc && suhd_rec.hhsu_hash == hhsuHash)
    {
		sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
	    cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
	    while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
	    {
			if (!ArrChkLimit (&invDtls_d, invDtls, invCnt))
				sys_err ("ArrChkLimit (invDtls)", ENOMEM, PNAME);

	   		invDtls [invCnt].hhsiHash   = sudt_rec.hhsi_hash;
	    	invDtls [invCnt].inv_oamt   = sudt_rec.amt_paid_inv;
	    	invDtls [invCnt].inv_lamt   = sudt_rec.loc_paid_inv;
	    	invDtls [invCnt].exch_var   = sudt_rec.exch_variatio;
	    	invDtls [invCnt].exch_rate  = sudt_rec.exch_rate;
	    	++invCnt;

	    	cc = find_rec (sudt, &sudt_rec, NEXT, "r");
	        if (invCnt + 1 >= DTLS)
	        {
		    	sprintf (err_str, ML (mlCrMess067), invCnt);
		    	print_mess (err_str);
				
		    	sleep (15);
				clear_mess ();
		    	return (EXIT_FAILURE);
	        }
	    }
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

void
SrchSuin (
	char	*keyValue)
{
	char	exString [31];
	long	hhsuHash = 0L;

	_work_open (16,0,40);
	sumr_rec.hhsu_hash = 0L;

	strcpy (suin2_rec.inv_no,keyValue);
	save_rec ("#Invoice Number ","#Customer ");
	cc = find_rec (suin2, &suin2_rec, GTEQ, "r");
	while (!cc && !strncmp (suin2_rec.inv_no,keyValue,strlen (keyValue)))
	{
		sumr_rec.hhsu_hash	=	suin2_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
		{
			sprintf (exString, "%15.15s %010ld", 
						suin2_rec.inv_no,
						sumr_rec.hhsu_hash);

			sprintf (err_str, "%-6.6s - %s", 
						sumr_rec.crd_no, 
						sumr_rec.crd_name);
	
			cc = save_rec (exString, err_str);
			if (cc)
				break;
		}
	    	cc = find_rec (suin2, &suin2_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sumr_rec.hhsu_hash = 0L;
		return;
	}

	hhsuHash = atol (temp_str + 16);

	sumr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sumr, "DBFIND");
	
	strcpy (temp_str, sumr_rec.crd_no);
	strcpy (sumr_rec.crd_name, sumr_rec.crd_name);
}
/*===================
| Read branch date. |
===================*/
long
ReadbranchDates (
 char *	br_no)
{
	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", br_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		return (comm_rec.crd_date);

	return (esmr_rec.crd_date);
}
			

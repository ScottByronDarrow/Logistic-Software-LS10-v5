/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_im_inp.c,v 5.8 2002/07/24 08:38:44 scott Exp $
|  Program Name  : (cr_im_inp.c) 
|  Program Desc  : (Supplier Invoice/Credits Input program for)
|               (Automatic invoice matching)
|---------------------------------------------------------------------|
|  Date Written  : (01/10/93)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: cr_im_inp.c,v $
| Revision 5.8  2002/07/24 08:38:44  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/16 01:02:29  scott
| Updated from service calls and general maintenance.
|
| Revision 5.6  2002/07/08 08:43:01  scott
| S/C 004051
|
| Revision 5.5  2002/06/21 04:10:25  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/10/25 06:25:19  robert
| Updated to adjust cursor position to avoid display overlap in LS10-GUI
|
| Revision 5.3  2001/08/23 11:28:35  scott
| Updated from scotts machine
|
| Revision 5.2  2001/08/09 08:51:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:27  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_im_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_im_inp/cr_im_inp.c,v 5.8 2002/07/24 08:38:44 scott Exp $";

#define 	MAXWIDTH	200
#define 	MAXLINES	1000
#include	<ml_std_mess.h>
#include	<ml_cr_mess.h>
#include	<pslscr.h>
#include	<GlUtils.h>
#include	<getnum.h>

#define	MCURR		 (multiCurrency [0] == 'Y')
#define	GST			 (gstApplies [0] == 'Y')
#define	INVOICE		 (invoiceCreditType [0] == 'I')
#define	CREDIT		 (invoiceCreditType [0] == 'C')
#define	F_INV		 (suin_rec.type [0] == '1')
#define	F_CRD		 (suin_rec.type [0] == '2')
#define	F_JNL		 (suin_rec.type [0] == '3')
#define	APPROVED	 (suin_rec.approved [0] == 'Y')
#define	AUTO_ALOC	 (suin_rec.rec_type [0] != 'N')
#define	ACT_COST	 (poActualCost [0] == 'Y')

#ifdef GVISION
#include "ViewTax.h"
#else
void    ViewTax (void);
#endif  /* GVISION */

#define	DBOX_TOP	10
#define	DBOX_LFT	59
#define	DBOX_WID	69
#define	DBOX_ROW	5
#define	DBOX_DEP	2 + DBOX_ROW

extern	int		_win_func;
char	*fifteenSpaces	=	"               ";

	/*---------------------------------------
	| Structure used for pop-up lot screen. |
	---------------------------------------*/
	static struct
	{
		char	fldPrompt [13];
		int		xPos;
		char	fldMask [13];
	} taxScn [] = {
		{" Tax Code ",		3,  "UU"},
		{"   Amount   ", 	28, "NNNNNNNNN.NN"},
		{"    Tax ", 		50, "NNNNNNNNN.NN"},
		{"", 			0,  ""},
	};

	int		win_ok;				/* stock check window		*/

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct inthRecord	inth_rec;
struct sumrRecord	sumr_rec;
struct suinRecord	suin_rec;
struct suidRecord	suid_rec;
struct popcRecord	popc_rec;
struct pohrRecord	pohr_rec;
struct poghRecord	pogh_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;
struct pogdRecord	pogd_rec;
struct poglRecord	pogl_rec;
struct pohsRecord	pohs_rec;
struct polnRecord	poln_rec;
struct posdRecord	posd_rec;
struct suphRecord	suph_rec;
struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct ddshRecord	ddsh_rec;

	Money	*sumr_balance	=	&sumr_rec.bo_curr;
	Money	*suin_tax_amnt	=	&suin_rec.tax_amnt1;
	Money	*suin_tax_val	=	&suin_rec.tax_val1;

	/*===========================
	| Special fields and flags. |
	===========================*/
	char 	*posh2 = "posh2",
			*data = "data";

   	int		newInvoice 			= FALSE,
			invoiceProofTotal 	= TRUE,
			dataInputFlag		= FALSE,
			invoiceApproved		= TRUE,
			calculateGst		= FALSE,
			setupOptionFlag		= FALSE,
			pidNumber			= 0,
			suwkNumber			= 0,
			workFileNo			= 0,
			workLineCounter		= 0,
			currentMonth		= 0,
			invoiceMonth		= 0,
			priorPosting		= 0,
			envPoSuHist 		= 0,
			envCrInvDateChk 	= 0,
			envCrCo			  	= 0,
			envCrFind			= 0,
			envCrMultTax 		= 0;

	struct	storeRec {
		int		SH_NO;
		int		PO_NO;
		int		GR_NO;
		double	glAllocation;
	} store [MAXLINES];

	char 	gstCode [4],
	    	gstPrompt [25],
	    	multiCurrency [2],
	    	branchNumber [3],
	    	gstApplies [2],
	    	poActualCost [2],
 	     	invoiceCreditType [2],
 	     	envCrNarrative [2];

	char	*currentUser;

	double 	invoiceTotal	= 0.00,
			proofTotal		= 0.00,
			batchTotal		= 0.00;

	double 	oldAmount = 0.00;

	float	envCrTaxTol,
			envCrGstPc = 0.0,
			gstDivide = 0.0;

	char	cat_desc [11][21];
	char	*inv_cat [] = {
			"                    ",
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


	/*======================
	| Supplier Work File. |
	======================*/
	struct {
		char	co_no [3];
		char	est [3];
		char	inv_no [sizeof suin_rec.inv_no];
		char	cus_po_no [sizeof suin_rec.cus_po_no];
		char	type [2];
		char	supplierNo [sizeof sumr_rec.crd_no];
		Date	date_of_inv;
		Date	gl_date;
		double	exch_rate;
		Money	fx_disc;
		Money	loc_disc;
		Money	fx_gst;
		Money	loc_gst;
		Money	tot_fx;
		Money	tot_loc;
		char	gl_acc_no [sizeof glwkRec.acc_no];
		Money	fx_amt;
		Money	loc_amt;
		char	narr [sizeof glwkRec.narrative];
		char	user_ref [sizeof glwkRec.user_ref];
		char	currency [sizeof pocrRec.code];
		char	stat_flag [2];
	}	suwk_rec;

	struct {
		long	wk_hash;
	} wk_rec;

	char	*scn_desc [] = {
		"Invoice/Credit Header Screen.",
		"General Ledger Allocation Screen."
	};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	est [3];
	char 	ic_prompt [25];
	char 	pt_prompt [25];
	char 	supplierNo [7];
	char 	doc_no [16];
	double	net_inv;
	double	net_inv_loc;
	double	gst;
	double	gst_loc;
	double	gross;
	double	gross_loc;
	double	prev_gross;
	double	gl_amt;
	double	gl_amt_loc;
	char 	prevInvoiceNo [sizeof suin_rec.inv_no];
	char 	prevSupplierNo [sizeof sumr_rec.crd_no];
	char 	branchNumber_name [41];
	char 	systemDate [11];
	long	lsystemDate;
	char 	com_date [11];
	char 	dflt_cr_date [11];
	char 	acc_no [MAXLEVEL + 1];
	char 	gl_narr [sizeof glwkRec.narrative];
	char 	gl_user_ref [sizeof glwkRec.user_ref];
	char 	loc_curr [4];
	char	gps_ref [sizeof glwkRec.user_ref];
	char	gps_type [2];
	int		cat_no;
	char	dd_ship_no [3];
	char	cat_desc [21];
	char	h_cat_desc [21];
	char	tax_code [DBOX_ROW][3];  	/* Tax code for invoice		*/
	double	tax_amnt [DBOX_ROW];	  	/* Taxable amount for line  */
	double	tax_val [DBOX_ROW];			/* Tax value of line.       */
	float	tax_rate [DBOX_ROW];		/* Tax rate of line.        */
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplier",	 3, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier No.        : ", "Enter Supplier Number or use [SEARCH KEY'S]",
		 NE, NO,  JUSTLEFT, "", "", local_rec.supplierNo},
	{1, LIN, "name",	 3, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "inv_crd",	 4, 24, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", local_rec.ic_prompt, "Enter existing unapproved document no. [SEARCH KEY] Available.",
		 NE, NO,  JUSTLEFT, "", "", suin_rec.inv_no},
	{1, LIN, "adr1",	 4, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address             : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.adr1},
	{1, LIN, "adr2",	 5, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.adr2},
	{1, LIN, "adr3",	 6, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.adr3},
	{1, LIN, "podel",	 5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Ref.           : ", "Enter User Ref, <return> defaults to Supplier Number.",
		YES, NO,  JUSTLEFT, "", "", suin_rec.cus_po_no},
	{1, LIN, "narr",	 6, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Narrative           : ", "Enter Narrative, <return> defaults to Invoice Number. ",
		YES, NO,  JUSTLEFT, "", "", suin_rec.narrative},
	{1, LIN, "rec_type",	 7, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Receipt Type        : ", "Enter N(ormal), G(oods Receipt), P(urchase order) S(hipment) or D(irect Delivery Shipment).",
		 YES, NO,  JUSTLEFT, "NGPSD", "", suin_rec.rec_type},
	{1, LIN, "doc_no",	 7, 80, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Document no.        : ", "Enter Referance no. [SEARCH KEY] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.doc_no},
	{1, LIN, "h_cat_no",	 8, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Analysis Code.      : ", "Enter costing type 1-10. [SEARCH KEY] available",
		YES, NO,  JUSTLEFT, "1", "10", (char *)&suin_rec.cst_type},
	{1, LIN, "h_cat_desc",	 8, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.h_cat_desc},
	{1, LIN, "destin",	 8, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Approved By         : ", "Enter Approved by, <return> defaults to User-name / Terminal-number ",
		YES, NO,  JUSTLEFT, "", "", suin_rec.destin},
	{1, LIN, "inv_date",	10, 24, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.com_date, "Date of Inv/Credit  : ", "<return> defaults to supplier month end date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&suin_rec.date_of_inv},
	{1, LIN, "gl_date",	10, 80, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", local_rec.dflt_cr_date, "General Ledger Date : ", "<return> defaults to invoice date.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&suin_rec.gl_date},
	{1, LIN, "date_post",	11, 24, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Date Posted.        : ", "<return> defaults to system (todays) date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&suin_rec.date_posted},
	{1, LIN, "date_due",	11, 80, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", "0", "Invoice Due Date.   : ", "<return> defaults to invoice date plus normal terms ",
		YES, NO, JUSTRIGHT, "", "", (char *)&suin_rec.pay_date},
	{1, LIN, "curr",	12, 24, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code       : ", " ",
		 NA, NO,  JUSTLEFT, "", "", suin_rec.currency},
	{1, LIN, "ex_rate",	13, 24, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "Exchange Rate       : ", "<return> defaults to current rate for currency ",
		YES, NO, JUSTRIGHT, "", "", (char *)&suin_rec.exch_rate},
	{1, LIN, "er_fixed",	13, 80, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exchange Rate : ", "N(o) (default) / Y(es)",
		YES, NO,  JUSTLEFT, "YN", "", suin_rec.er_fixed},
	{1, LIN, "net_inv",	16, 24, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Nett Amount         : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_inv},
	{1, LIN, "net_inv_loc",	16, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Nett Amount         : ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_inv_loc},
	{1, LIN, "gst",	17, 24, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", gstPrompt, " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.gst},
	{1, LIN, "gst_loc",	17, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", gstPrompt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.gst_loc},
	{1, LIN, "gross",	18, 24, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Gross Amount.       : ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.gross},
	{1, LIN, "gross_loc",	18, 80, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Gross Amount.       : ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.gross_loc},
	{2, TAB, "glacct",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", " ", GlDesc, "Enter General Ledger Account Number.",
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.acc_no},
	{2, TAB, "gl_amt",	 0, 0, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Base Curr Amt. ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt},
	{2, TAB, "gl_amt_loc",	 0, 0, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Local Curr Amt.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt_loc},
	{2, TAB, "gps_type",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "N", "T", "<return> N(one) , P(urchase order), S(hipment), G(oods receipt No)",
		YES, NO,  JUSTLEFT, "NPSG", "", local_rec.gps_type},
	{2, TAB, "gps_ref",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", " Purchase ref. ", "Enter Ref number. [SEARCH KEY] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.gps_ref},
	{2, TAB, "cat_no",	 0, 0, INTTYPE,
		"NN", "          ",
		" ", " ", "CT", "Enter costing type 1-10. [SEARCH KEY] available",
		YES, NO,  JUSTLEFT, "0", "10", (char *)&local_rec.cat_no},
	{2, TAB, "cat_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Costing  Description", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.cat_desc},
	{2, TAB, "gl_narr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", suin_rec.narrative, "      Narrative.    ", "<return> defaults to screen 1 narrative ",
		YES, NO,  JUSTLEFT, "", "", local_rec.gl_narr},
	{2, TAB, "gl_user_ref",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", suin_rec.cus_po_no, "User Reference", "<return> defaults to screen 1 user ref ",
		YES, NO,  JUSTLEFT, "", "", local_rec.gl_user_ref},
	{2, TAB, "pc_gr_no",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		ND, NO,  JUSTLEFT, "", "", popc_rec.gr_no},
	{2, TAB, "pc_gr_hash",	 0, 0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", " ", "", " ",
		ND, NO,  JUSTLEFT, "", "", (char *)&popc_rec.hhgr_hash},
	{2, TAB, "pc_po_no",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		ND, NO,  JUSTLEFT, "", "", popc_rec.po_no},
	{2, TAB, "pc_po_hash",	 0, 1, LONGTYPE,
		"NNNNNNN", "          ",
		" ", " ", "", " ",
		ND, NO,  JUSTLEFT, "", "", (char *)&popc_rec.hhpo_hash},
	{2, TAB, "pc_sh_no",	 0, 1, LONGTYPE,
		"NNNNNNN", "          ",
		" ", " ", "", " ",
		ND, NO,  JUSTLEFT, "", "", (char *)&popc_rec.hhsh_hash},
	{3, LIN, "app",	 4, 28, CHARTYPE,
		"U", "          ",
		" ", "N", local_rec.pt_prompt, " ",
		YES, NO,  JUSTLEFT, "YN", "", suin_rec.approved},
	{3, LIN, "proof",	 6, 28, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", "Proof Total.        : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&proofTotal},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <twodec.h>
#include <FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
int			shutdown_prog		 (void);
void		OpenDB				 (void);
void		CloseDB				 (void);
int			spec_valid			 (int);
int			ValidDdShipment		 (void);
int			ValidateGlDate  	 (void);
void		SrchPogh			 (char *);
void		SrchGrNo			 (char *);
void		SrchShipmentNo		 (char *);
void		SrchPoNumber		 (char *);
void		SrchSuppCatNo		 (int);
void		RestoreSuin			 (void);
void		ReadInvoice			 (void);
void		LoadSuid			 (void);
void		RecalcExchange		 (void);
void		DisplayExchange		 (void);
void		DisplayAllocate		 (void);
void		Update 				 (void);
void		PaymentDate			 (void);
void		SrchSuin			 (char *);
void		LoadCatDesc			 (void);
void		RecDirectDelivery	 (void);
void		UpdatePoln			 (void);
void		ProcessSuph			 (void);
void		AddPogl				 (void);
void		CreateTableInfo		 (void);
int			heading				 (int);
int			CheckClass			 (void);
int			win_function		 (int, int, int, int);
void		ViewTax				 (void);
void		DrawTaxScn			 (void);
void		DrawVLine			 (int, int);
void		DispFlds			 (int, int);
int			ValidTaxAmnt		 (int);
int			ValidTaxVal			 (int);
int			ValidTaxCode		 (int);
int			ValidateTotal		 (void);
void		InputField			 (int, int);
int			SrchDdsh			 (char *);
int			SrchInth			 (char *);
int			DateWithinMonth 	 (long, long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	char *	sptr;

	_win_func = FALSE;

	if (argc < 3)
	{
		print_at (0, 0, mlCrMess074, argv [0]);
		return (EXIT_FAILURE);
	}
	pidNumber   = atoi (argv [2]);
	sprintf (invoiceCreditType, "%-1.1s",argv [1]);

	SETUP_SCR (vars);


	if (!INVOICE && !CREDIT)
	{
		print_at (0,0,ML (mlCrMess074), argv [0]);
		return (EXIT_FAILURE);
	}

	if (INVOICE)
	{
		strcpy (local_rec.ic_prompt, ML ("Invoice No.         : "));
		strcpy (local_rec.pt_prompt, ML ("Invoice Approved    : "));
	}
	else
	{
		strcpy (local_rec.ic_prompt, ML ("Cr/Note No.         : "));
		strcpy (local_rec.pt_prompt, ML ("Cr/Note Approved    : "));
	}

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	sprintf (envCrNarrative, "%-1.1s", chk_env ("CR_NARRATIVE"));

	sprintf (gstApplies, "%-1.1s", get_env ("GST"));
	if (GST)
	{
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
		sprintf (gstPrompt, "%-3.3s Paid.           : ", gstCode);

		sptr = chk_env ("CR_MULT_TAX");
		if (sptr == (char *)0)
			envCrMultTax = 0;
		else
			envCrMultTax = atoi (sptr);

		sptr = chk_env ("CR_TAX_TOL");
		if (sptr == (char *)0)
			envCrTaxTol = 0;
		else
			envCrTaxTol = (float) atof (sptr);

	}

	currentUser = getenv ("LOGNAME");
	
	sprintf (poActualCost, "%-1.1s", get_env ("PO_ACT_COST"));
	if (!ACT_COST)
	{
		/*---------------------------------------------------------------
		| Auto invoice matching cannot apply unless actual costing is   |
		| used due to a discrepency between where the estimated costs   |
		| are posted to the G/L via the excg and where the auto matched |
		| invoices are posted via the GL interface                      |
		---------------------------------------------------------------*/
		FLD ("rec_type") = NA;
		strcpy (suin_rec.rec_type, "N");
	}
	
	sptr = chk_env ("PO_SU_HIST");
	if (sptr == (char *)0)
		envPoSuHist = 0;
	else
		envPoSuHist = atoi (sptr);

	sptr = chk_env ("CR_INV_DATE_CHK");
	if (sptr == (char *)0)
		envCrInvDateChk = 0;
	else
		envCrInvDateChk = atoi (sptr);

	sptr = chk_env ("PRIOR_POST");
	if (sptr == (char *)0)
		priorPosting = 0;
	else
		priorPosting = atoi (sptr);

	if (!GST) 
	{
		FLD ("net_inv") 		= ND;
		FLD ("net_inv_loc") 	= ND;
		FLD ("gst") 			= ND;
		FLD ("gst_loc") 		= ND;
		vars [label ("gross")].row = 16;
		vars [label ("gross_loc")].row = 16;
	}

	if (!MCURR) 
	{
		FLD ("curr") 			= ND;
		FLD ("ex_rate") 		= ND;
		FLD ("er_fixed") 		= ND;
		FLD ("net_inv_loc") 	= ND;
		FLD ("gst_loc") 		= ND;
		FLD ("gross_loc") 		= ND;
		FLD ("gl_amt_loc") 	= ND;
		if (GST)
		{
			vars [label ("net_inv")].row = 13;
			vars [label ("gst")].row = 14;
			vars [label ("gross")].row = 15;
		}
		else
			vars [label ("gross")].row = 13;
	}

	/*-----------------------------------------------
	| Use 4 parameters for supplier initial setup. |
	-----------------------------------------------*/
	if (argc == 4)
		setupOptionFlag = TRUE;
	else
		setupOptionFlag = FALSE;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();

	swide ();
	clear ();

	LoadCatDesc ();

	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	for (i = 0; i < 2; i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (1);
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	envCrFind	= atoi (get_env ("CR_FIND"));
	envCrCo		= atoi (get_env ("CR_CO"));

	OpenDB ();

	GL_SetMask (GlFormat);

	DateToDMY (comm_rec.gl_date, NULL, &currentMonth, NULL);

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);
	strcpy (local_rec.com_date, DateToString (comm_rec.crd_date));

	envCrGstPc = (float) atof (get_env ("CR_GST_PC"));
	if (envCrGstPc != 0.00)
		gstDivide = ( (100 + envCrGstPc) / envCrGstPc);
	else
		gstDivide = 0.00;

	strcpy (local_rec.prevInvoiceNo,"000000000000000");
	strcpy (local_rec.prevSupplierNo, "000000");

	while (prog_exit == 0) 
	{
		abc_unlock (sumr);
		abc_unlock (suin);
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newInvoice	= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [2]	=	0;
		proofTotal = 0.00;
		edit_ok (2);
		_win_func = FALSE;

		for (i = 0; i < MAXLINES; i++)
		{
			store [i].SH_NO = 0;
			store [i].PO_NO = 0;
			store [i].GR_NO = 0;
			store [i].glAllocation = 0.00;
		}

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		CreateTableInfo ();

		if (newInvoice == TRUE && !AUTO_ALOC)
		{
			/*------------------------------
			| Enter screen 2 linear input. |
			------------------------------*/
			heading (2);
			entry (2);
			if (restart) 
				continue;
		}
		no_edit (3);
		
		/*--------------------------------------------
		| re-edit tabular if proof total incorrect . |	
		--------------------------------------------*/
		do
		{
			edit_all ();

			if (restart) 
				break;

			if (AUTO_ALOC)
			{
				invoiceProofTotal = FALSE;
				invoiceApproved	  = FALSE;
				break;
			}

			heading (3);
			scn_display (3);
			entry (3);
			if (restart) 
				break;

			if (invoiceApproved == TRUE)
				 print_mess (ML (mlCrMess143));

			if (invoiceProofTotal == TRUE)
				 print_mess (ML (mlCrMess144));
			if (invoiceProofTotal == TRUE || invoiceApproved == TRUE)
				sleep (sleepTime);

		} while (invoiceProofTotal == TRUE || invoiceApproved == TRUE);

		if (restart) 
			continue;

		/*----------------------------------
		| Update invoice & detail records. |
		----------------------------------*/
		if (invoiceProofTotal == FALSE && invoiceApproved == FALSE)
			Update ();
	}	
	if (shutdown_prog ())
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
int
shutdown_prog (
 void)
{
	clear ();
	print_at (0, 0, ML (mlCrMess039),DOLLARS (batchTotal));
	PauseForKey (1, 0, ML (mlStdMess042), 0);
	CloseDB (); 
	FinishProgram ();
	if (dataInputFlag == FALSE)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
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


	sprintf (filename,"%s/WORK/cr_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);

	cc = RF_OPEN (filename,sizeof (wk_rec),"w",&workFileNo);
	if (cc) 
		file_err (cc, "cr_per", "WKOPEN");

	sprintf (filename,"%s/WORK/cr_suwk%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);

	cc = RF_OPEN (filename,sizeof (suwk_rec),"w",&suwkNumber);
	if (cc) 
		file_err (cc, "suwk_rec", "WKOPEN");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (posh2, posh);

	open_rec (sumr ,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    					    : "sumr_id_no3");

	open_rec (suin , suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (suid , suid_list, SUID_NO_FIELDS, "suid_id_no");
	open_rec (popc , popc_list, POPC_NO_FIELDS, "popc_id_no");
	open_rec (inth , inth_list, INTH_NO_FIELDS, "inth_id_no");
	open_rec (pohr , pohr_list, POHR_NO_FIELDS, "pohr_id_no3");
	open_rec (poln , poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (posh , posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posh2, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (pogh , pogh_list, POGH_NO_FIELDS, "pogh_po_id");
	open_rec (pogl , pogl_list, POGL_NO_FIELDS, "pogl_id_no2");
	open_rec (pohs , pohs_list, POHS_NO_FIELDS, "pohs_id_no");
	open_rec (suph , suph_list, SUPH_NO_FIELDS, "suph_id_no");
	open_rec (ddhr , ddhr_list, DDHR_NO_FIELDS, "ddhr_hhdd_hash");
	open_rec (ddln , ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
	open_rec (ddsh , ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no");
	OpenGlmr ();
	OpenPocr ();
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suid);
	abc_fclose (inth);
	abc_fclose (poln);
	abc_fclose (popc);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (posh);
	abc_fclose (posh2);
	abc_fclose (pohr);
	abc_fclose (pohs);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (suph);
	GL_Close ();
	abc_dbclose (data);
	cc = RF_CLOSE (workFileNo);
	if (cc != 0 && cc != 9)
		file_err (cc, "cr_per", "WKCLOSE");

	cc = RF_CLOSE (suwkNumber);
	if (cc != 0 && cc != 9)
		file_err (cc, "cr_suwk", "WKCLOSE");
}

int
spec_valid (
 int field)
{
	double  chq_gst = 0.00;
	double  total_gl = 0.00;
	int		this_page = line_cnt / TABLINES;

	/*---------------------------------
	| Validate Supplier Number Input. |
	---------------------------------*/
	if (LCHECK ("supplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supplierNo));
		cc = find_rec (sumr , &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			sprintf (err_str, ML (mlStdMess022), local_rec.supplierNo);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		cc = FindPocr (comm_rec.co_no, sumr_rec.curr_code, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (suin_rec.currency,sumr_rec.curr_code);

		if (!strcmp (sumr_rec.curr_code, local_rec.loc_curr))
			FLD ("ex_rate") = NA;
		else
			FLD ("ex_rate") = NO;

		DSP_FLD ("name");
		DSP_FLD ("curr");
		DSP_FLD ("adr1");
		DSP_FLD ("adr2");
		DSP_FLD ("adr3");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Invoice Number Input. |
	--------------------------------*/
	if (LCHECK ("inv_crd"))
	{
		if (SRCH_KEY)
		{
			SrchSuin (temp_str);
			return (EXIT_SUCCESS);
		}
		if (strcmp (suin_rec.inv_no,fifteenSpaces) == 0)
			return (EXIT_FAILURE);
		ReadInvoice ();

		strcpy (suin_rec.currency, sumr_rec.curr_code);
		
		if (newInvoice == FALSE && APPROVED)
		{
			errmess (ML (mlCrMess075));
			sleep (sleepTime);
			restart = 1;
			return (EXIT_SUCCESS);
		}
		if (newInvoice == FALSE)
		{
		    if (INVOICE && F_CRD)
		    {
				errmess (ML (mlCrMess076));
				sleep (sleepTime);
				restart = 1;
				return (EXIT_SUCCESS);
		    }
		    if (CREDIT && F_INV)
		    {
				errmess (ML (mlCrMess077));
				sleep (sleepTime);
				restart = 1;
				return (EXIT_SUCCESS);
		     }
		}
		
		if (newInvoice == FALSE && F_JNL)
		{
			errmess (ML (mlCrMess078));
			sleep (sleepTime);
			restart = 1;
			return (EXIT_SUCCESS);
		}
		if (newInvoice == FALSE)
		{
			RestoreSuin ();
			LoadSuid ();
			spec_valid (label ("doc_no"));
			entry_exit = 1;
		}
		strcpy (local_rec.cat_desc, cat_desc [local_rec.cat_no]);
		return (EXIT_SUCCESS);
	}


	/*------------------------
	| Validate receipt type. |
	------------------------*/
	if (LCHECK ("rec_type"))
	{
		if (FLD ("rec_type") == NA)
		{
			strcpy (suin_rec.rec_type, "N");
			DSP_FLD ("rec_type");
		}

		if (suin_rec.rec_type [0] == 'N') 
		{
			FLD ("doc_no")   = NA;
			FLD ("h_cat_no") = NA;
			strcpy (local_rec.doc_no, fifteenSpaces);
			suin_rec.cst_type = 0;
			strcpy (local_rec.h_cat_desc, cat_desc [suin_rec.cst_type]);
			DSP_FLD ("h_cat_no");
			DSP_FLD ("h_cat_desc");
			DSP_FLD ("doc_no");
			CreateTableInfo ();
			return (EXIT_SUCCESS);
		}
		else
		{
			if (suin_rec.rec_type [0] != 'D')
				print_at (7, 100, "                ");
			FLD ("doc_no")   = YES;
			FLD ("h_cat_no") = YES;
		}
		if (prog_status != ENTRY)
		{
			CreateTableInfo ();
			/*---------------------------------------
			| Force document number to be re-input. |
			---------------------------------------*/
			do
			{
				get_entry (field + 1);
				if (restart)
				{
					restart = FALSE;
					return (EXIT_FAILURE);
				}
			} while (spec_valid (field + 1));

			/*----------------------------------------
			| Force cost type number to be re-input. |
			----------------------------------------*/
			if (suin_rec.cst_type == 0)
			{
				do
				{
					get_entry (field + 2);
					if (restart)
					{
						restart = FALSE;
						return (EXIT_FAILURE);
					}
				} while (spec_valid (field + 2));
			}
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Check document no. |
	--------------------*/
	if (LCHECK ("doc_no"))
	{
		int		i;
		double	grn_tot = 0.00;
		int		part_costed = FALSE;

		if (suin_rec.rec_type [0] == 'N' || suin_rec.rec_type [0] == ' ') 
		{
			strcpy (suin_rec.rec_type, "N");
			strcpy (local_rec.doc_no, " ");
			return (EXIT_SUCCESS);
		}

		if (suin_rec.rec_type [0] == 'P') 
			abc_selfield (pogh, "pogh_po_id");

		if (suin_rec.rec_type [0] == 'D') 
			abc_selfield (pogh, "pogh_dd_id");

		if (suin_rec.rec_type [0] == 'S') 
			abc_selfield (pogh, "pogh_sh_id");

		if (suin_rec.rec_type [0] == 'G') 
			abc_selfield (pogh, "pogh_id_no2");

		if (SRCH_KEY)
		{
			if (suin_rec.rec_type [0] == 'P' ||
				suin_rec.rec_type [0] == 'D') 
				SrchPoNumber (temp_str);
			else
				SrchPogh (temp_str);

			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.doc_no, fifteenSpaces))
		{
			errmess (ML ("As receipt type is not N(ormal) document number must be input."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pogh_rec.co_no,comm_rec.co_no);
		if (suin_rec.rec_type [0] == 'G') 
			strcpy (pogh_rec.gr_no, local_rec.doc_no);

		if (suin_rec.rec_type [0] == 'P') 
		{
			/*----------------------------
			| Check if order is on file. |
			----------------------------*/
			strcpy (pohr_rec.co_no,comm_rec.co_no);
			sprintf (pohr_rec.pur_ord_no, "%-15.15s", local_rec.doc_no);
			strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));
	
			cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.doc_no, pohr_rec.pur_ord_no);
	
			if (pohr_rec.status [0] == 'U')
			{
				print_mess (ML (mlCrMess079));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (pohr_rec.status [0] != 'O' && 
			     pohr_rec.status [0] != 'P' &&
			     pohr_rec.status [0] != 'R' &&
			     pohr_rec.status [0] != 'r' &&
			     pohr_rec.status [0] != 'C')
			{
				print_mess (ML (mlCrMess080));
				sleep (sleepTime);
		
				return (EXIT_FAILURE);
			}
	
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				print_mess (ML (mlCrMess081));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] == 'Y')
			{
				print_mess (ML (mlCrMess141));
				sleep (sleepTime);
		
				return (EXIT_FAILURE);
			}
			sprintf (pogh_rec.pur_ord_no, "%-15.15s", local_rec.doc_no);
			strcpy (pogh_rec.pur_ord_no, zero_pad (pogh_rec.pur_ord_no, 15));
		}

		if (suin_rec.rec_type [0] == 'D') 
		{
			/*----------------------------
			| Check if order is on file. |
			----------------------------*/
			strcpy (pohr_rec.co_no,comm_rec.co_no);
			sprintf (pogh_rec.pur_ord_no, "%-15.15s", local_rec.doc_no);
			strcpy (pogh_rec.pur_ord_no, zero_pad (pogh_rec.pur_ord_no, 15));


			cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			if (pohr_rec.status [0] == 'U')
			{
				print_mess (ML (mlCrMess079));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (pohr_rec.status [0] != 'O' && 
			     pohr_rec.status [0] != 'P' &&
			     pohr_rec.status [0] != 'R' &&
			     pohr_rec.status [0] != 'r' &&
			     pohr_rec.status [0] != 'C')
			{
				print_mess (ML (mlCrMess080));
				sleep (sleepTime);
		
				return (EXIT_FAILURE);
			}
	
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				print_mess (ML (mlCrMess081));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.status [0] == 'P')
			{
				print_mess (ML (mlCrMess082));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] == 'Y')
			{
				DSP_FLD ("doc_no");
				if (newInvoice == FALSE && prog_status == ENTRY)
				{
					print_at (7, 100, ML ("- Shipment"));
					print_at (7, 112, "%2.2s", local_rec.dd_ship_no);
				}
				else
				{
					do
					{
						print_at (7, 100, ML ("- Shipment"));
						rv_pr (ML (" [Enter D-D Shipment Number. [SEARCH KEY] Available.]"),0,23,1);
						getalpha (112, 7, "NN", local_rec.dd_ship_no);
						cc = ValidDdShipment ();
					} while (cc);
					print_at (7, 104, "%2.2s", local_rec.dd_ship_no);
	
					if (restart)
						return (EXIT_SUCCESS);
	
					if (!strcmp (local_rec.dd_ship_no, ""))
						return (EXIT_FAILURE);
				}
			}
			else
			{
				print_mess (ML (mlCrMess142));
				sleep (sleepTime);
		
				return (EXIT_FAILURE);
			}
			sprintf (pogh_rec.pur_ord_no, "%-15.15s", local_rec.doc_no);
			strcpy (pogh_rec.pur_ord_no, zero_pad (pogh_rec.pur_ord_no, 15));
		}

		if (suin_rec.rec_type [0] == 'D') 
			pogh_rec.hhds_hash = ddsh_rec.hhds_hash;

		if (suin_rec.rec_type [0] == 'S') 
		{
			strcpy (posh_rec.co_no, comm_rec.co_no);
			strcpy (posh_rec.csm_no, zero_pad (local_rec.doc_no,12));
			cc = find_rec (posh, &posh_rec, EQUAL, "r");
			pogh_rec.hhsh_hash = cc ? -1 : posh_rec.hhsh_hash;
			/***
			pogh_rec.hhsh_hash = atol (local_rec.doc_no);
			***/
		}

		cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
		if (cc)
		{
			if (suin_rec.rec_type [0] == 'D' && pohr_rec.drop_ship [0] == 'Y')
			{
				sprintf (err_str,ML (mlCrMess083), pogh_rec.pur_ord_no);
				i = prmptmsg (err_str, "YyNn", 1, 2);
				move (1,2);
				line (128);
				if (i == 'N' || i == 'n')
					return (EXIT_FAILURE);

				RecDirectDelivery ();
			}
			else
			{
				if (suin_rec.rec_type [0] == 'G') 
					strcpy (err_str,ML (mlStdMess049));
				if (suin_rec.rec_type [0] == 'P') 
					strcpy (err_str, ML (mlCrMess084));	
				if (suin_rec.rec_type [0] == 'S') 
					sprintf (err_str,ML (mlCrMess154));
	
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (pogh_rec.drop_ship [0] == 'Y' && suin_rec.rec_type [0] != 'D')
		{
			if (suin_rec.rec_type [0] == 'G') 
				strcpy (err_str, ML (mlCrMess085));
			if (suin_rec.rec_type [0] == 'P') 
				strcpy (err_str, ML (mlCrMess086));

			if (suin_rec.rec_type [0] == 'S') 
				strcpy (err_str, ML (mlCrMess087));
	
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*----------------------------------------------
		| Process all lines on goods receipt to obtain |
		| values required for comparison.              |
		----------------------------------------------*/
		grn_tot = 0.00;
		part_costed = FALSE;
		abc_selfield (pogl, "pogl_id_no");
		pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
		pogl_rec.line_no = 0;
		cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
		while (!cc && pogl_rec.hhgr_hash == pogh_rec.hhgr_hash)
		{
			/*---------------------------------------------------------------
			| Line has already been updated from previous auto cost update. |
			---------------------------------------------------------------*/
			if (pogl_rec.auto_cost)
			{
				part_costed = TRUE;
				cc = find_rec (pogl, &pogl_rec, NEXT, "r");
				continue;
			}

			grn_tot += (pogl_rec.land_cst    * pogl_rec.qty_rec);

			cc = find_rec (pogl, &pogl_rec, NEXT, "r");
		}
		abc_selfield (pogl, "pogl_id_no2");

		if (part_costed)
		{
			if (grn_tot > 0.00)
			{
				sprintf (err_str,ML (mlCrMess088), grn_tot);
				print_mess (err_str);
				
				sleep (sleepTime);
				clear_mess ();
			}
			else
			{
				i = prmptmsg (ML (mlCrMess089), "YyNn", 1, 23);
				move (1,2);
				line (128);
				if (i == 'N' || i == 'n')
					return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
			CreateTableInfo ();

		DSP_FLD ("doc_no");
		
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Check Category No. |
	--------------------*/
	if (LCHECK ("h_cat_no"))
	{
		if (SRCH_KEY)
		{
			SrchSuppCatNo (FALSE);
			return (EXIT_SUCCESS);
		}
		strcpy (local_rec.h_cat_desc, cat_desc [suin_rec.cst_type]);
		DSP_FLD ("h_cat_no");
		DSP_FLD ("h_cat_desc");

		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Check for default used on user ref. |
	-------------------------------------*/
	if (LCHECK ("podel"))
	{
		if (dflt_used)
		{
			strcpy (suin_rec.cus_po_no, local_rec.supplierNo);
			DSP_FLD ("podel");
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------
	| Check for default used on narrative. |
	--------------------------------------*/
	if (LCHECK ("narr"))
	{
		if (dflt_used)
		{
			if (envCrNarrative [0] == 'N')
				sprintf (suin_rec.narrative, "%-20.20s", sumr_rec.crd_name);
			else
				strcpy (suin_rec.narrative, suin_rec.inv_no);
			 DSP_FLD ("narr");
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Check for default Approved by. |
	--------------------------------*/
	if (LCHECK ("destin"))
	{
		if (dflt_used)
		{
			sprintf (suin_rec.destin, "%-13.13s TTY%-3d", currentUser, ttyslt ());
			DSP_FLD ("destin");
		}
		return (EXIT_SUCCESS);
	}
			
	/*-------------------------------
	| Validate Invoice/Credit Date. |	
	-------------------------------*/
	if (LCHECK ("inv_date"))
	{
		if (dflt_used)
		{
			if (AUTO_ALOC)
				suin_rec.date_of_inv = pogh_rec.date_raised;
			else
				suin_rec.date_of_inv = comm_rec.crd_date;
		}
		else
		{
			if (!DateWithinMonth (suin_rec.date_of_inv, comm_rec.crd_date))
				return (EXIT_FAILURE);
		}
		DateToDMY (suin_rec.date_of_inv, NULL, &invoiceMonth, NULL);
		strcpy (local_rec.dflt_cr_date, DateToString (suin_rec.date_of_inv));
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Date Due.  |	
	---------------------*/
	if (LCHECK ("date_due"))
	{
		if (dflt_used)
			PaymentDate ();

		DSP_FLD ("date_due");
		return (EXIT_SUCCESS);
	}
		
	/*------------------------
	| Validate gl_date Input. |
	------------------------*/
	if (LCHECK ("gl_date"))
	{
		if (dflt_used)
		{
			if (AUTO_ALOC)
				suin_rec.gl_date = pogh_rec.date_raised;
			else
				suin_rec.gl_date = suin_rec.date_of_inv;

			if (suin_rec.date_of_inv < comm_rec.gl_date)
				suin_rec.gl_date = comm_rec.gl_date;

			return (EXIT_SUCCESS);
		}
		else
		{
			if (!DateWithinMonth (suin_rec.gl_date, comm_rec.gl_date))
				return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("ex_rate"))
	{
		if (dflt_used)
			suin_rec.exch_rate = pocrRec.ex1_factor;

		RecalcExchange ();
		DisplayExchange ();
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate Nett Invoice Amount. |
	-------------------------------*/
	if (LCHECK ("net_inv"))
	{
		if (dflt_used)
		{
			FLD ("gross") = YES;
			skip_entry = 3;
			calculateGst = TRUE;
			return (EXIT_SUCCESS);
		}

		calculateGst = FALSE;
		local_rec.gross = local_rec.net_inv + local_rec.gst;
		DSP_FLD ("gross");
		if (MCURR)
		{
			RecalcExchange ();
			DisplayExchange ();
		}
		return (EXIT_SUCCESS);
	}
	 
	/*----------------------
	| Validate gst Amount. |
	----------------------*/
	if (LCHECK ("gst"))
	{
		if (!GST)
			return (EXIT_SUCCESS);

		if (envCrMultTax)
		{
			DSP_FLD ("gst");
			local_rec.gross = local_rec.net_inv + local_rec.gst;
			DSP_FLD ("gross");
			_win_func = TRUE;
			win_function (label ("gst"), 0, 1, !ENTRY);
			heading (1);
			scn_display (1);
		}

		local_rec.gross = local_rec.net_inv + local_rec.gst;
		DSP_FLD ("gross");

		if (MCURR)
		{
			RecalcExchange ();
			DisplayExchange ();
		}

		/*-----------------------
		| Supplier Tax Excempt. |
		-----------------------*/
		if (sumr_rec.tax_code [0] != 'A' && 
			sumr_rec.tax_code [0] != 'B')
		{
			if (!envCrMultTax)
			{
				if (envCrGstPc != 0.00)
					chq_gst = local_rec.gross / gstDivide;
				else 
					chq_gst = 0.00;
	
				if (dflt_used)
					local_rec.gst = chq_gst;
					
				if (chq_gst < local_rec.gst - 1 || chq_gst > local_rec.gst + 1)
				{ 
					sprintf (err_str, ML (mlCrMess090), gstCode, DOLLARS (local_rec.gst), envCrGstPc);
					errmess (err_str);
					sleep (sleepTime);
					return (EXIT_SUCCESS);
				}
			}
		}
		calculateGst = FALSE;
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Gross Invoice Amount. |
	--------------------------------*/
	if (LCHECK ("gross"))
	{	
		if (dflt_used)
			local_rec.gross = CENTS (atof (prv_ntry));

		/*----------------------------
		| Automatic GST Calculation. |
		----------------------------*/
		if (GST && calculateGst == TRUE)
		{
			if (sumr_rec.tax_code [0] != 'A' && 
				sumr_rec.tax_code [0] != 'B')
			{
				if (envCrMultTax)
				{
					DSP_FLD ("gross");
					_win_func = TRUE;
					win_function (label ("gst"), 0, 1, !ENTRY);
					heading (1);
					scn_display (1);
				}
				else
				{
					if (envCrGstPc != 0.00)
						local_rec.gst = local_rec.gross / gstDivide;
					else
						local_rec.gst = 0.00;
				}
			}
		}

		local_rec.net_inv = local_rec.gross - local_rec.gst;
		DSP_FLD ("net_inv");
		DSP_FLD ("gst");
		DSP_FLD ("gross");
		if (MCURR)
		{
			RecalcExchange ();
			DisplayExchange ();
		}
		return (EXIT_SUCCESS);
	}
		
	/*-------------------------------------------
	| Validate Local Curr Gross Invoice Amount. |
	-------------------------------------------*/
	if (LCHECK ("gross_loc"))
	{	
		if (dflt_used)
			local_rec.gross_loc = CENTS (atof (prv_ntry));

		if ( (!strcmp (sumr_rec.curr_code, local_rec.loc_curr))
			&& (local_rec.gross_loc != CENTS (atof (prv_ntry))))
		{
			print_mess (ML ("Gross Local Amount Cannot be changed"));
			sleep (sleepTime);
			local_rec.gross_loc = CENTS (atof (prv_ntry));
			return (EXIT_SUCCESS);
		}

		if (local_rec.gross_loc == 0.00 && local_rec.gross > 0.00)
		{
			errmess (ML ("Local gross amount cannot be 0.00 as Base gross amount is > 0.00"));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (local_rec.gross_loc > 0)
		{
			double		tmp_exch	=	0.00;
			int			i;

			if (local_rec.gross_loc != 0.00)
			{
				tmp_exch = local_rec.gross / local_rec.gross_loc;

				if ( (suin_rec.exch_rate != tmp_exch) &&
					 (suin_rec.exch_rate != 0.00))
				{
					i = prmptmsg (ML ("Exchange rate will be recomputed, Continue [Y/N]?"),"YyNn",1,2);
					print_at (2,1,"%-110.110s"," ");
					heading (1);
					scn_display (1);
					if (i == 'N' || i == 'n')
						return (EXIT_FAILURE);
				}
				clear_mess ();
				suin_rec.exch_rate = tmp_exch;
			}
			else
			{
				suin_rec.exch_rate = 1.0;
			}
			RecalcExchange ();
			DisplayExchange ();
		}
		return (EXIT_SUCCESS);
	}
		
	/*-----------------------------------------
	| Validate General Ledger Account Number. |
	-----------------------------------------*/
	if (LCHECK ("glacct"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		/*-----------------------------------------
		| Delete GL Allocn Line If Null Entry.    |
		-----------------------------------------*/
		if (dflt_used && prog_status != ENTRY)
		{
			lcount [2]--;
			for (workLineCounter = line_cnt;line_cnt < lcount [2];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				line_display ();
			}
			sprintf (local_rec.acc_no,"%*.*s",MAXLEVEL, MAXLEVEL," ");
			local_rec.gl_amt = 0.00;
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				blank_display ();

			line_cnt = workLineCounter;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			GL_GLI 
			 (
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"DFT EXPEN.",
				" ", 
				" "
			);
			strcpy (local_rec.acc_no, glmrRec.acc_no);
		}
		strcpy (glmrRec.co_no,comm_rec.co_no);

		GL_FormAccNo (local_rec.acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr , &glmrRec, COMPARISON,"r");
		if (cc) 
		{
			print_err (ML (mlStdMess024));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (CheckClass ())
			return (EXIT_FAILURE);
		
		print_at (4, 2, "%R Account: %-66.66s", glmrRec.desc);
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate General Ledger Allocation Amount. |
	--------------------------------------------*/
	if (LCHECK ("gl_amt")) 
	{
		if (MCURR)
		{
			if (suin_rec.exch_rate == 0.00)
			  	 suin_rec.exch_rate = 1.00;

			local_rec.gl_amt_loc = local_rec.gl_amt / suin_rec.exch_rate; 
			DSP_FLD ("gl_amt_loc");
		}
		store [line_cnt].glAllocation = local_rec.gl_amt;
		DisplayAllocate ();
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Proof Total. |
	-----------------------*/
	if (LCHECK ("proof"))
	{
		invoiceProofTotal = TRUE;
		if (end_input) 
		{
			entry_exit = 1; 
			return (EXIT_SUCCESS);
		}
		invoiceTotal = no_dec (local_rec.gross);
		proofTotal = no_dec (proofTotal);

		if (proofTotal != invoiceTotal)
		{ 
			sprintf (err_str, ML (mlCrMess091),DOLLARS (proofTotal),DOLLARS (invoiceTotal));
			errmess (err_str);
			sleep (sleepTime);
			invoiceProofTotal = TRUE;
			return (EXIT_FAILURE); 
		}
		invoiceProofTotal = FALSE;
		return (EXIT_SUCCESS);
	}
	/*----------------
	| Purchase type. |
	----------------*/
	if (LCHECK ("gps_type"))
	{
		store [line_cnt].SH_NO = FALSE,
		store [line_cnt].PO_NO = FALSE,
		store [line_cnt].GR_NO = FALSE;

		if (dflt_used || local_rec.gps_type [0] == 'N')
		{
			if (prog_status == ENTRY)
				skip_entry = goto_field (field,label ("gl_narr"));
			else
			{
				strcpy (local_rec.gps_ref, fifteenSpaces);
				DSP_FLD ("gps_ref");
			}
			return (EXIT_SUCCESS);
		}
	
		if (local_rec.gps_type [0] == 'P')
			store [line_cnt].PO_NO = TRUE;

		if (local_rec.gps_type [0] == 'S')
			store [line_cnt].SH_NO = TRUE;

		if (local_rec.gps_type [0] == 'G')
			store [line_cnt].GR_NO = TRUE;

		if (prog_status != ENTRY)
		{
			do
			{
				get_entry (field +1);
				if (restart)
					break;
			} while (spec_valid (field +1));
		}
		return (EXIT_SUCCESS);
	}
		
	/*-------------------------
	| Check goods receipt no. |
	-------------------------*/
	if (LCHECK ("gps_ref"))
	{
		if ( (store [line_cnt].SH_NO == FALSE &&
		      store [line_cnt].PO_NO == FALSE &&
		      store [line_cnt].GR_NO == FALSE) || dflt_used)
		{
			strcpy (local_rec.gps_ref, fifteenSpaces);
			strcpy (local_rec.gps_type, "N");
			DSP_FLD ("gps_ref");
			DSP_FLD ("gps_type");

			if (prog_status == ENTRY)
				skip_entry = goto_field (field,label ("gl_narr"));

			return (EXIT_SUCCESS);
		}
		
		if (store [line_cnt].PO_NO)
			abc_selfield (pohr, "pohr_id_no3");

		if (store [line_cnt].GR_NO)
		{
			abc_selfield (pogh, "pogh_id_no2");
			abc_selfield (pohr, "pohr_hhpo_hash");
		}
			
		if (SRCH_KEY)
		{
			if (store [line_cnt].PO_NO)
				SrchPoNumber (temp_str);

			if (store [line_cnt].SH_NO)
				SrchShipmentNo (temp_str);

			if (store [line_cnt].GR_NO)
				SrchGrNo (temp_str);

			return (EXIT_SUCCESS);
		}
		if (store [line_cnt].PO_NO)
		{
			/*----------------------------
			| Check if order is on file. |
			----------------------------*/
			strcpy (pohr_rec.co_no,comm_rec.co_no);
			sprintf (pohr_rec.pur_ord_no, "%-15.15s",local_rec.gps_ref);
			strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no,15));
			strcpy (local_rec.gps_ref, pohr_rec.pur_ord_no); 

			cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] == 'Y')
			{
				print_mess (ML (mlCrMess092));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.stat_flag [0] == 'Q')
			{
				print_mess (ML (mlCrMess093));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			strcpy (popc_rec.gr_no, fifteenSpaces);
			strcpy (popc_rec.po_no, pohr_rec.pur_ord_no);
			popc_rec.hhsh_hash = 0L;
			popc_rec.hhpo_hash = pohr_rec.hhpo_hash;
			popc_rec.hhgr_hash = 0L;
		}
		if (store [line_cnt].SH_NO)
		{
			strcpy (posh_rec.co_no,comm_rec.co_no);
			strcpy (posh_rec.csm_no, zero_pad (local_rec.gps_ref,12));
			cc = find_rec (posh,&posh_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess050));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (popc_rec.gr_no, fifteenSpaces);
			strcpy (popc_rec.po_no, fifteenSpaces);
			popc_rec.hhsh_hash = posh_rec.hhsh_hash;
			popc_rec.hhpo_hash = 0L;
			popc_rec.hhgr_hash = 0L;
		}
		if (store [line_cnt].GR_NO)
		{
			strcpy (pogh_rec.co_no,comm_rec.co_no);
			strcpy (pogh_rec.gr_no, local_rec.gps_ref);
			cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess049));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pogh_rec.drop_ship [0] == 'Y')
			{
				sprintf (err_str,ML (mlCrMess094),pogh_rec.gr_no);
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (popc_rec.gr_no, pogh_rec.gr_no);
			strcpy (popc_rec.po_no, fifteenSpaces);
			popc_rec.hhsh_hash = 0L;
			popc_rec.hhpo_hash = 0L;
			popc_rec.hhgr_hash = pogh_rec.hhgr_hash;
		}
		if (store [line_cnt].SH_NO)
			sprintf (local_rec.gl_narr, "SH # %15.15s", local_rec.gps_ref);
		if (store [line_cnt].PO_NO)
			sprintf (local_rec.gl_narr, "PO # %-15.15s", local_rec.gps_ref);
		if (store [line_cnt].GR_NO)
			sprintf (local_rec.gl_narr, "GR # %-15.15s", local_rec.gps_ref);

		DSP_FLD ("gps_ref");
		DSP_FLD ("gl_narr");

		return (EXIT_SUCCESS);
	}
	/*--------------------
	| Check Category No. |
	--------------------*/
	if (LCHECK ("cat_no"))
	{
		if (SRCH_KEY)
		{
			SrchSuppCatNo (FALSE);
			return (EXIT_SUCCESS);
		}
		if (dflt_used && local_rec.gps_type [0] == 'N')
			return (EXIT_SUCCESS);

		strcpy (local_rec.cat_desc, cat_desc [local_rec.cat_no]);
		DSP_FLD ("cat_desc");

		if (store [line_cnt].SH_NO || store [line_cnt].PO_NO || store [line_cnt].GR_NO)
		{
		    if (prog_status == ENTRY)
				skip_entry = goto_field (field,label ("gl_user_ref"));
		}

		if (local_rec.cat_no < 1)
		{
			errmess (ML ("Category number must be greater-than-zero"));
			sleep (sleepTime);		
			return (EXIT_FAILURE);	
		}
		return (EXIT_SUCCESS);
	}
			
	/*------------------------------
	| Determine Default for Field. |
	------------------------------*/
	if (LCHECK ("gl_narr"))
	{
		if (dflt_used)
		{
			if (envCrNarrative [0] == 'N')
				sprintf (local_rec.gl_narr, "%-20.20s", sumr_rec.crd_name);
			else
				strcpy (local_rec.gl_narr, suin_rec.narrative);

			return (EXIT_SUCCESS);
		}
	}

	/*------------------------------
	| Determine Default for Field. |
	------------------------------*/
	if (LCHECK ("gl_user_ref"))
	{
		if (dflt_used)
		{
			if (envCrNarrative [0] == 'N') 
				strcpy (local_rec.gl_user_ref, local_rec.supplierNo);
			else
				strcpy (local_rec.gl_user_ref, suin_rec.cus_po_no);
			return (EXIT_SUCCESS);
		}
	}

	/*-------------------------
	| Validate Approved Flag. |
	-------------------------*/
	if (LCHECK ("app"))
	{
		invoiceApproved = TRUE;
		if (end_input) 
		{
			entry_exit = 1; 
			return (EXIT_SUCCESS);
		}

		if (ValidateGlDate ())
			return (EXIT_FAILURE);

		scn_set (2);
	
		total_gl = 0.00;
		for (workLineCounter = 0; workLineCounter < lcount [2]; workLineCounter++) 
		{
			getval (workLineCounter);
			total_gl += local_rec.gl_amt;
		}
		scn_set (3);
		total_gl = no_dec (total_gl);
		local_rec.net_inv = no_dec (local_rec.net_inv);
		if (APPROVED && (total_gl < local_rec.net_inv - 1 || 
					       total_gl > local_rec.net_inv + 1))
		{
			sprintf (err_str, ML (mlCrMess095),DOLLARS (total_gl),DOLLARS (local_rec.net_inv));
			errmess (err_str);
			sleep (sleepTime);
			invoiceApproved = TRUE;
			return (EXIT_FAILURE);
		}
		invoiceApproved = FALSE;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ValidDdShipment (
 void)
{
	if (last_char == FN1)
	{
		restart = TRUE;
		return (EXIT_SUCCESS);
	}

	if (last_char == UP_KEY || last_char == LEFT_KEY)
	{
		strcpy (local_rec.dd_ship_no, "");
		return (EXIT_SUCCESS);
	}

	if (SRCH_KEY)
	{
		cc = SrchDdsh (temp_str);
		if (cc)
			strcpy (local_rec.dd_ship_no, "");
		else
			strcpy (local_rec.dd_ship_no, temp_str);
		heading (1);
		scn_display (1);
		return (EXIT_FAILURE);
	}

	if (dflt_used)
		strcpy (local_rec.dd_ship_no, " 1");

	/*-------------------------------
	| Check if shipment is on file. |
	-------------------------------*/
	abc_selfield (ddsh, "ddsh_id_no");
	ddsh_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddsh_rec.hhsu_hash = pohr_rec.hhsu_hash;
	sprintf (ddsh_rec.ship_no, "%2.2s", local_rec.dd_ship_no);
	cc = find_rec (ddsh, &ddsh_rec, EQUAL, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess050));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	switch (ddsh_rec.stat_flag [0])
	{
		case 'A' : sprintf (err_str,ML (mlCrMess096), ddsh_rec.ship_no);
				   print_mess (err_str);
				   sleep (sleepTime);
				   return (EXIT_FAILURE);
		case 'C' : 	print_mess (ML (mlCrMess096));
				   	sleep (sleepTime);
					return (EXIT_SUCCESS);
		default	 : break;
	}
	return (EXIT_SUCCESS);
}

int
ValidateGlDate (
 void)
{
	long low_date;

	low_date	=	MonthStart (comm_rec.gl_date);
	if (suin_rec.gl_date < low_date && !priorPosting)
		return print_err ("Input Date earlier than G/L Date %s.",
						DateToString (low_date));
	return (EXIT_SUCCESS);
}

/*==================================
| Search for goods receipt number. |
==================================*/
void
SrchPogh (
 char *	key_val)
{
	char dsp_str [2][81];

	_work_open (15,0,60);

	strcpy (pogh_rec.co_no,comm_rec.co_no);

	if (suin_rec.rec_type [0] == 'P') 
	{
		save_rec ("#P/O  No.","# System Reference     | Date Raised");
		sprintf (pogh_rec.pur_ord_no,"%-15.15s",key_val);
	}

	if (suin_rec.rec_type [0] == 'S') 
	{
		save_rec ("#Ship No.","# System Reference          | Date Raised");
		pogh_rec.hhsh_hash = 0L;
	}

	if (suin_rec.rec_type [0] == 'G') 
	{
		save_rec ("#GRIN No.","# System Reference          | Date Raised");
		sprintf (pogh_rec.gr_no,"%-15.15s",key_val);
	}

	strcpy (pogh_rec.co_no,comm_rec.co_no);
	cc = find_rec (pogh,&pogh_rec,GTEQ,"r");
	while (!cc && !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		if (suin_rec.rec_type [0] == 'P')
		{
			if (!strcmp (pogh_rec.pur_ord_no, fifteenSpaces))
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			if (strncmp (pogh_rec.pur_ord_no,key_val,strlen (key_val)))
				break;

			sprintf (dsp_str [0], "%-15.15s", pogh_rec.pur_ord_no);
			sprintf (dsp_str [1], "Goods Rec: %s | %s ", pogh_rec.gr_no,
										DateToString (pogh_rec.date_raised));
		}

		if (suin_rec.rec_type [0] == 'G')
		{
			if (!strcmp (pogh_rec.gr_no, fifteenSpaces))
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			if (strncmp (pogh_rec.gr_no,key_val,strlen (key_val)))
				break;

			sprintf (dsp_str [0], "%-15.15s", pogh_rec.gr_no);
			sprintf (dsp_str [1], "P/Order  : %s | %s ", pogh_rec.pur_ord_no,
										DateToString (pogh_rec.date_raised));
		}

		if (suin_rec.rec_type [0] == 'S')
		{
			if (pogh_rec.hhsh_hash == 0L)
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			
			abc_selfield (posh2, "posh_id_no");
			strcpy (posh2_rec.co_no, comm_rec.co_no);
			posh2_rec.hhsh_hash = pogh_rec.hhsh_hash;
			cc = find_rec (posh2, &posh2_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			sprintf (dsp_str [0], "%-12.12s", posh2_rec.csm_no);
			sprintf (dsp_str [1], "Goods Rec: %s | %s ", pogh_rec.gr_no,
										DateToString (pogh_rec.date_raised));
		}
		cc = save_rec (dsp_str [0], dsp_str [1]);
		if (cc)
			break;

		cc = find_rec (pogh,&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pogh_rec.co_no,comm_rec.co_no);

	if (suin_rec.rec_type [0] == 'P') 
		sprintf (pogh_rec.pur_ord_no,"%-15.15s", temp_str);

	if (suin_rec.rec_type [0] == 'S') 
	{
		strcpy (posh_rec.co_no, comm_rec.co_no);
		sprintf (posh_rec.csm_no, "%-12.12s", temp_str);
		cc = find_rec (posh, &posh_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "posh", "DBFIND");
		pogh_rec.hhsh_hash = posh_rec.hhsh_hash;
	}
	if (suin_rec.rec_type [0] == 'G') 
		sprintf (pogh_rec.gr_no,"%-15.15s", temp_str);

	cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");
}

/*==================================
| Search for goods receipt number. |
==================================*/
void
SrchGrNo (
 char *	key_val)
{
	_work_open (15,0,60);
	save_rec ("#GR. No.","# System Reference  | Date Raised");
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",key_val);
	cc = find_rec (pogh,&pogh_rec,GTEQ,"r");
	while (!cc && !strncmp (pogh_rec.gr_no,key_val,strlen (key_val)) &&
		      !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		/*------------------------------
		| check to make sure not quick |
		------------------------------*/
		pohr_rec.hhpo_hash = pogh_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (pogh,&pogh_rec,NEXT,"r");
			continue;
		}
		if (pohr_rec.stat_flag [0] == 'Q')
		{
			cc = find_rec (pogh,&pogh_rec,NEXT,"r");
			continue;
		}
		sprintf (err_str, "P/Order  : %s | %s ", pogh_rec.pur_ord_no,
										DateToString (pogh_rec.date_raised));
		
		cc = save_rec (pogh_rec.gr_no,err_str);
		if (cc)
			break;

		cc = find_rec (pogh,&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",temp_str);
	cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");
}

/*=============================
| Search for shipment number. |
=============================*/
void
SrchShipmentNo (
 char *	key_val)
{
	char	dsp_method [5];

	_work_open (15,0,60);
	save_rec ("#Shipment No.","#Ship Name            |     Departed  from   | Method | Destination       ");
	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no,"%-12.12s",key_val);
	cc = find_rec (posh,&posh_rec,GTEQ,"r");
	while (!cc && !strncmp (posh_rec.csm_no,key_val,strlen (key_val)) &&
		      !strcmp (posh_rec.co_no,comm_rec.co_no))
	{
		if (posh_rec.ship_method [0] == 'A')
			strcpy (dsp_method, "AIR ");
		else if (posh_rec.ship_method [0] == 'S')
			strcpy (dsp_method, "SEA ");
		else if (posh_rec.ship_method [0] == 'L')
			strcpy (dsp_method, "LAND");
		else
			strcpy (dsp_method, "????");

		sprintf (err_str, "%s | %s |  %s  | %s ", 
					posh_rec.vessel,
					posh_rec.port,
					dsp_method,
					posh_rec.destination);
					
		cc = save_rec (posh_rec.csm_no, err_str);
		if (cc)
			break;

		cc = find_rec (posh,&posh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no,"%-12.12s",temp_str);
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "posh", "DBFIND");
}

/*===================================
| Search for purchase order number. |
===================================*/
void
SrchPoNumber (
 char *	key_val)
{
	_work_open (15,0,60);

	save_rec ("#P/O Number.    ","#   Order Terms.     |Date  Raised|                   Contact              ");
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");

	while (!cc &&
	        !strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)) &&
	        !strcmp (pohr_rec.co_no,comm_rec.co_no))
	{
		if (pohr_rec.stat_flag [0] == 'Q' || pohr_rec.status [0] == 'P')
		{
			cc = find_rec (pohr,&pohr_rec,NEXT,"r");
			continue;
		}
		if ( (suin_rec.rec_type [0] == 'P' && pohr_rec.drop_ship [0] == 'Y') ||
		 (suin_rec.rec_type [0] == 'D' && pohr_rec.drop_ship [0] != 'Y'))
		{
			cc = find_rec (pohr,&pohr_rec,NEXT,"r");
			continue;
		}

		sprintf (err_str,"%s| %s | %s",
					pohr_rec.term_order, 
					DateToString (pohr_rec.date_raised),
					pohr_rec.contact);
	
		cc = save_rec (pohr_rec.pur_ord_no,err_str);
		if (cc)
			break;
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}
/*=============================
| Search for category number. |
=============================*/
void
SrchSuppCatNo (
 int zero_allow)
{
	int	i;

	work_open ();
	_work_open (11,0,40);
	save_rec ("#Cat No.","#Category Description");
	for (i = (zero_allow) ? 0 : 1; i < 11; i++)
	{
		sprintf (err_str, "%1d      ", i);
		cc = save_rec (err_str, cat_desc [i]);
		if (cc)
			break;
	}
	disp_srch ();
	work_close ();
}

void
RestoreSuin (
 void)
{
	if (F_INV)
	{
		local_rec.gst     = suin_rec.gst;
		local_rec.net_inv = suin_rec.amt - suin_rec.gst;
		local_rec.gross   = suin_rec.amt; 
	}
	if (F_CRD)
	{
		local_rec.gst     = no_dec (suin_rec.gst * -1);
		local_rec.net_inv = no_dec ( (suin_rec.amt - suin_rec.gst) * -1);
		local_rec.gross   = no_dec (suin_rec.amt * -1);
	}
	if (MCURR)
	{
		if (suin_rec.exch_rate == 0.00)
			suin_rec.exch_rate = 1.00;

		local_rec.net_inv_loc = no_dec (local_rec.net_inv / suin_rec.exch_rate);
		local_rec.gst_loc     = no_dec (local_rec.gst     / suin_rec.exch_rate);
		local_rec.gross_loc   = no_dec (local_rec.gross   / suin_rec.exch_rate);
	}

	if (suin_rec.rec_type [0] == 'D')
	{
		abc_selfield (ddsh, "ddsh_hhds_hash");
		cc = find_hash (ddsh, &ddsh_rec, EQUAL, "r", atol (suin_rec.doc_no));
		if (cc)
			file_err (cc, "ddsh", "DBFIND");

		abc_selfield (pohr, "pohr_hhpo_hash");
		abc_selfield (poln, "poln_hhpl_hash");
		abc_selfield (ddln, "ddln_id_no2");

		ddln_rec.hhdd_hash = ddsh_rec.hhdd_hash;
		ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
		cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
		if (!cc && 
			ddln_rec.hhdd_hash == ddsh_rec.hhdd_hash &&
			ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
		{
			cc = find_hash (poln, &poln_rec, EQUAL, "r", ddln_rec.hhpl_hash);
			if (cc)
				file_err (cc, "poln", "DBFIND");

			cc = find_hash (pohr, &pohr_rec, EQUAL, "r", poln_rec.hhpo_hash);
			if (cc)
				file_err (cc, "pohr", "DBFIND");

			strcpy (local_rec.doc_no,     pohr_rec.pur_ord_no);
			strcpy (local_rec.dd_ship_no, ddsh_rec.ship_no);
		}
		else
			file_err (cc, "ddln", "DBFIND");

		abc_selfield (pohr, "pohr_id_no3");
	}
	else
		strcpy (local_rec.doc_no, suin_rec.doc_no);

	local_rec.prev_gross = local_rec.gross;

	DateToDMY (suin_rec.date_of_inv, NULL, &invoiceMonth, NULL);
	strcpy (local_rec.h_cat_desc, cat_desc [suin_rec.cst_type]);
}

/*---------------------------------------------
| Read Invoice / Credit Header Into Tabular.  |
---------------------------------------------*/		
void
ReadInvoice (
 void)
{
	int		i;
	long	today;

	today = TodaysDate ();

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est, comm_rec.est_no);
	cc = find_rec (suin , &suin_rec, COMPARISON , "w");
	if (cc) 
	{
		newInvoice = TRUE;
		suin_rec.amt = 0;
		suin_rec.gst = 0;
		local_rec.prev_gross = 0;
		for (i = 0; i < DBOX_ROW; i++)
		{
			strcpy (local_rec.tax_code [i], "  ");
			local_rec.tax_amnt 	 [i]  = 0.00;
			local_rec.tax_val 	 [i]  = 0.00;
		}
	}
	else 
	{
		newInvoice = FALSE;
		_win_func = TRUE;
		if (suin_rec.exch_rate == 0.00)
			suin_rec.exch_rate = 1.00;
		if (F_INV)
       			sprintf (err_str, ML (mlCrMess097),DOLLARS (suin_rec.amt));
		if (F_CRD)
       			sprintf (err_str, ML (mlCrMess098),DOLLARS (suin_rec.amt));
		
		print_mess (err_str);

		sleep (sleepTime);

		if (suin_rec.gl_date < comm_rec.gl_date)
			suin_rec.gl_date = comm_rec.gl_date;

		oldAmount = suin_rec.amt;
		for (i = 0; i < DBOX_ROW; i++)
		{
			switch (i)
			{
				case 0:
					strcpy (local_rec.tax_code [i], suin_rec.tax_code1);
				break;

				case 1:
					strcpy (local_rec.tax_code [i], suin_rec.tax_code2);
				break;

				case 2:
					strcpy (local_rec.tax_code [i], suin_rec.tax_code3);
				break;

				case 3:
					strcpy (local_rec.tax_code [i], suin_rec.tax_code4);
				break;

				case 4:
					strcpy (local_rec.tax_code [i], suin_rec.tax_code5);
				break;
			}
			local_rec.tax_amnt [i] 	= suin_tax_amnt	 [i];
			local_rec.tax_val [i]  	= suin_tax_val	 [i];
			if (strlen (clip (local_rec.tax_code [i])))
			{
				sprintf (inth_rec.co_no,    "%-2.2s", comm_rec.co_no);
				sprintf (inth_rec.tax_code, "%-2.2s", local_rec.tax_code [i]);
				cc = find_rec (inth, &inth_rec, EQUAL, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess051));
					sleep (sleepTime);
					clear_mess ();
					local_rec.tax_rate [row] = (float) ( (local_rec.tax_val [i] 
											/  local_rec.tax_amnt [i]) * 100);
					continue;
				}

				if (inth_rec.eff_date > today)
				{
					print_mess (ML (mlStdMess052));
					sleep (sleepTime);
					clear_mess ();
				}
				local_rec.tax_rate [row] = inth_rec.tax_rate;
			}
		}
	}	
	return;
}

/*---------------------------------------------
| Load Invoice / Credit Detail Into Tabular.  |
---------------------------------------------*/		
void
LoadSuid (
 void)
{
	init_vars (2);
	scn_set (2);
	lcount [2] = 0;

	suid_rec.hhsi_hash = suin_rec.hhsi_hash;
	suid_rec.line_no = 0;
	cc = find_rec (suid , &suid_rec, GTEQ, "r");	
	while (!cc && suid_rec.hhsi_hash == suin_rec.hhsi_hash)
	{
		strcpy (local_rec.gps_type, suid_rec.pf_type);
		strcpy (local_rec.gps_ref,  suid_rec.pf_ref);
		strcpy (popc_rec.gr_no,     suid_rec.gr_no);
		strcpy (popc_rec.po_no,     suid_rec.po_no);
		popc_rec.hhsh_hash 		=   suid_rec.hhsh_hash;
		popc_rec.hhpo_hash 		=   suid_rec.hhpo_hash;
		popc_rec.hhgr_hash 		=   suid_rec.hhgr_hash;
		local_rec.cat_no 		=   suid_rec.ct_type;
		strcpy (local_rec.cat_desc, cat_desc [local_rec.cat_no]);

		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,suid_rec.gl_acc_no);
		if (find_rec (glmr , &glmrRec, COMPARISON,"r"))
		{
			cc = find_rec (suid , &suid_rec, NEXT, "r");	
			continue;
		}
	
		sprintf (local_rec.acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,suid_rec.gl_acc_no);
		local_rec.gl_amt = suid_rec.period_amt;
		store [lcount [2]].glAllocation	= local_rec.gl_amt;
		if (suin_rec.exch_rate == 0.00)
			suin_rec.exch_rate = 1.00;

		local_rec.gl_amt_loc = local_rec.gl_amt / suin_rec.exch_rate;

		strcpy (local_rec.gl_narr,suid_rec.narrative);
		strcpy (local_rec.gl_user_ref,suid_rec.user_ref);
		putval (lcount [2]++);
	
		cc = find_rec (suid , &suid_rec, NEXT, "r");	
	}
	scn_set (1);
}

/*-------------------------------
| Recalc Local Currency Values. |
-------------------------------*/
void
RecalcExchange (
 void)
{
	if (!MCURR)
		return;

	if (suin_rec.exch_rate == 0.00)
		suin_rec.exch_rate = 1.00;

	local_rec.gross_loc = no_dec (local_rec.gross / suin_rec.exch_rate); 
	local_rec.net_inv_loc = no_dec (local_rec.net_inv / suin_rec.exch_rate); 
	local_rec.gst_loc = no_dec (local_rec.gst / suin_rec.exch_rate);

	/*-----------------------------------------------------
    | Recalc screen 2 gl allocations with new exch. rate. |
	-----------------------------------------------------*/
	scn_set (2);
	for (workLineCounter = 0;workLineCounter < lcount [2];workLineCounter++)
	{
		getval (workLineCounter);
		local_rec.gl_amt_loc = local_rec.gl_amt / suin_rec.exch_rate;
		putval (workLineCounter);
	}
	scn_set (1);
	return;
}

/*----------------------------------
| Redisplay Local Currency Values. |
----------------------------------*/
void
DisplayExchange (
 void)
{
	if (!MCURR)
		return;

	DSP_FLD ("ex_rate");
	DSP_FLD ("net_inv_loc");
	DSP_FLD ("gst_loc");
	DSP_FLD ("gross_loc");
	return;
}

/*----------------------------------
| Display Allocation Balance.      |
----------------------------------*/
void
DisplayAllocate (
 void)
{
	int		i,
	    	max;
	double  total_alloc = 0.00;
	double  alloc_bal = 0.00;

	max = (line_cnt > lcount [2]) ? line_cnt + 1 : lcount [2] + 1;

	for (i = 0;i < max ;i++)
		total_alloc += store [i].glAllocation;


 	alloc_bal = local_rec.gross - local_rec.gst - total_alloc;
	sprintf (err_str, "Balance To Allocate: %14.2f", DOLLARS (twodec (alloc_bal)));
	print_at (19, 1, err_str);
	fflush (stdout);
}

/*=================
| Update Files.   |
=================*/
void
Update (
 void)
{
	char	inv_no [sizeof suin_rec.inv_no],
			rel_pur_order [sizeof suin_rec.cus_po_no],
			supplierNo [sizeof sumr_rec.crd_no];
	
	int		i;
	int		to_val = 0;
	int		new_popc;

	double  total_gl_alloc = 0.00;
	double	wk_amt = 0.00;

	dataInputFlag = TRUE;
	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	/*------------------------
	| Add or update invoice. |
	------------------------*/
	strcpy (inv_no, suin_rec.inv_no);
	strcpy (rel_pur_order, suin_rec.cus_po_no);
	strcpy (supplierNo, local_rec.supplierNo);

	strcpy (suin_rec.type, (INVOICE) ? "1" : "2");

	local_rec.gross = no_dec (local_rec.gross);
	local_rec.gst = no_dec (local_rec.gst);
	local_rec.prev_gross = no_dec (local_rec.prev_gross);

	strcpy (suin_rec.stat_flag, "0");
	strcpy (local_rec.prevInvoiceNo,suin_rec.inv_no);
	strcpy (local_rec.prevSupplierNo, sumr_rec.crd_no);

	suin_rec.amt = (INVOICE) ? local_rec.gross : local_rec.gross * -1;
	suin_rec.gst = (INVOICE) ? local_rec.gst   : local_rec.gst   * -1;
	for (i = 0; i < DBOX_ROW; i++)
	{
		switch (i)
		{
			case 0:
				strcpy (suin_rec.tax_code1, local_rec.tax_code [i]);
			break;

			case 1:
				strcpy (suin_rec.tax_code2, local_rec.tax_code [i]);
			break;

			case 2:
				strcpy (suin_rec.tax_code3, local_rec.tax_code [i]);
			break;

			case 3:
				strcpy (suin_rec.tax_code4, local_rec.tax_code [i]);
			break;

			case 4:
				strcpy (suin_rec.tax_code5, local_rec.tax_code [i]);
			break;
		}
		suin_tax_amnt [i] = local_rec.tax_amnt [i];
		suin_tax_val [i]  = local_rec.tax_val [i];
	}

	if (setupOptionFlag)
		strcpy (suin_rec.approved, "Y");

	if (AUTO_ALOC)
		strcpy (suin_rec.approved, "N");

	if (newInvoice == FALSE)
	{
		if (suin_rec.rec_type [0] == 'D')
			sprintf (suin_rec.doc_no, "%07ld", ddsh_rec.hhds_hash);
		else
			strcpy (suin_rec.doc_no, local_rec.doc_no);

		if (oldAmount != suin_rec.amt)
			suin_rec.pay_amt = suin_rec.amt;

		cc = abc_update (suin ,&suin_rec);
		if (cc) 
			file_err (cc, "suin", "DBUPDATE");
	}
	else
	{
		if (suin_rec.rec_type [0] == 'D')
			sprintf (suin_rec.doc_no, "%07ld", ddsh_rec.hhds_hash);
		else
			strcpy (suin_rec.doc_no, local_rec.doc_no);
		suin_rec.pay_amt = suin_rec.amt;
		suin_rec.amt_paid = 0;
		strcpy (suin_rec.hold_reason, "   ");
		cc = abc_add (suin ,&suin_rec);
		if (cc) 
			file_err (cc, "suin", "DBADD");
			
		cc = find_rec (suin ,&suin_rec, COMPARISON, "r");	
		if (cc) 
			file_err (cc, "suin", "DBFIND");
	}
	abc_unlock (suin);

	/*------------------------------
    | Update supplier aged amount. |
	------------------------------*/
	wk_amt = (INVOICE) ? local_rec.gross : local_rec.gross * -1;

	batchTotal += (INVOICE) ? local_rec.gross_loc : local_rec.gross_loc * -1;

	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.est_no,branchNumber);
	strcpy (sumr_rec.crd_no,pad_num (local_rec.supplierNo));
	cc = find_rec (sumr , &sumr_rec, COMPARISON, "u");
	if (cc) 
		file_err (cc, "sumr", "DBFIND");

	if (INVOICE)
	{
		sumr_rec.mtd_exp += (wk_amt - local_rec.prev_gross);
		sumr_rec.ytd_exp += (wk_amt - local_rec.prev_gross);
	}	
	if (invoiceMonth > currentMonth)
		invoiceMonth -= 12;

	if (currentMonth - invoiceMonth == 0)
		sumr_balance [0] += (wk_amt - local_rec.prev_gross);

	if (currentMonth - invoiceMonth == 1)
		sumr_balance [1] += (wk_amt - local_rec.prev_gross);

	if (currentMonth - invoiceMonth == 2)
		sumr_balance [2] += (wk_amt - local_rec.prev_gross);

	if (currentMonth - invoiceMonth >= 3)
		sumr_balance [3] += (wk_amt - local_rec.prev_gross);

	cc = abc_update (sumr ,&sumr_rec);
	if (cc)
		file_err (cc, "sumr", "DBUPDATE");

	/*-----------------------------------------------------------------
	| Add supplier hash to period recalc work file (prog cr_percalc). |
	-----------------------------------------------------------------*/
	wk_rec.wk_hash = sumr_rec.hhsu_hash;
	cc = RF_ADD (workFileNo, (char *) &wk_rec);
	if (cc) 
		file_err (cc, "cr_per", "WKADD");

	/*---------------------------------------------------------------------
	| Invoice/Credit Approved - Add to cr_suwk batch, Delete Suid Detail. |
	---------------------------------------------------------------------*/
	if (APPROVED && !setupOptionFlag)
	{
		/*-------------------------------------------------
		| Adjust gl values from foreign to local currency.|
		-------------------------------------------------*/
/*
		if (MCURR)
		{
			local_rec.gst 	= no_dec (local_rec.gst_loc);
			local_rec.gross = no_dec (local_rec.gross_loc);
		}
*/

		/*-----------------------------------------------------
		| Add gl alloc to suwk work file used for gl posting. |
		-----------------------------------------------------*/
		scn_set (2);

		to_val = (lcount [2] == 0) ? 1 : lcount [2];
	
		for (workLineCounter = 0;workLineCounter < to_val;workLineCounter++)
		{
			if (lcount [2] > 0)
			{
				getval (workLineCounter);
			}
			/*-------------------------------------------------
			| This bit of code fixed situation when only GST. |
			-------------------------------------------------*/
			else
			{
				local_rec.gl_amt = 0.00;
				local_rec.gl_amt_loc = 0.00;
				strcpy (local_rec.gl_narr,suin_rec.narrative);

				GL_GLI 
				 (
					comm_rec.co_no,
					comm_rec.est_no,
					"  ",
					"DFT EXPEN.",
					" ", 
					" "
				);

				strcpy (local_rec.acc_no, glmrRec.acc_no);
				strcpy (local_rec.gl_user_ref, suin_rec.cus_po_no);
			}
						  
			strcpy (suwk_rec.co_no, comm_rec.co_no);
			strcpy (suwk_rec.est,   comm_rec.est_no);
			strcpy (suwk_rec.inv_no, inv_no);
			strcpy (suwk_rec.cus_po_no, rel_pur_order);
			strcpy (suwk_rec.supplierNo, supplierNo);
			strcpy (suwk_rec.narr, local_rec.gl_narr);
			strcpy (suwk_rec.user_ref, local_rec.gl_user_ref);
			suwk_rec.date_of_inv = suin_rec.date_of_inv;
			sprintf (suwk_rec.gl_acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,local_rec.acc_no);

			suwk_rec.gl_date = suin_rec.gl_date;
			strcpy (suwk_rec.type, suin_rec.type);
			strcpy (suwk_rec.stat_flag, "0");
			suwk_rec.loc_disc 	= 0.00;
			suwk_rec.fx_disc 	= 0.00;
			/*----------------------------------------------
			| Store Inv/Crd Totals In First Allocn Record. |
			----------------------------------------------*/
			if (workLineCounter == 0)
			{
				suwk_rec.fx_gst 	= local_rec.gst;
				suwk_rec.loc_gst 	= local_rec.gst_loc;
				suwk_rec.tot_fx 	= local_rec.gross; 
				suwk_rec.tot_loc 	= local_rec.gross_loc; 
				if (MCURR)
		       		suwk_rec.exch_rate = suin_rec.exch_rate; 
				else
					suwk_rec.exch_rate = 1.00;
			}
			else
			{
				suwk_rec.fx_gst 	= 0.00;
				suwk_rec.loc_gst 	= 0.00;
				suwk_rec.tot_fx 	= 0.00; 
				suwk_rec.tot_loc 	= 0.00;
			}
			local_rec.gl_amt 		= no_dec (local_rec.gl_amt);
			local_rec.gl_amt_loc 	= no_dec (local_rec.gl_amt_loc);

			/*----------------------------------------------
     		| Adjust Last Allocation For Roundoff Error.   |
			----------------------------------------------*/
			if (workLineCounter == lcount [2] - 1)
				local_rec.gl_amt = local_rec.gross
					 - local_rec.gst - total_gl_alloc;
			else
				total_gl_alloc += local_rec.gl_amt;

			/*--------------------------------------
     		| Set Values Negative If Credit Note.  |
			--------------------------------------*/
			if (INVOICE)
			{
				suwk_rec.fx_amt 	=  local_rec.gl_amt;
				suwk_rec.loc_amt 	=  local_rec.gl_amt_loc;
			}
			else
			{
				suwk_rec.fx_gst 	= no_dec (local_rec.gst 			* -1);
				suwk_rec.loc_gst 	= no_dec (local_rec.gst_loc 		* -1);
				suwk_rec.tot_fx 	= no_dec (local_rec.gross 		* -1);
				suwk_rec.tot_loc 	= no_dec (local_rec.gross_loc	* -1);
				suwk_rec.fx_amt		= no_dec (local_rec.gl_amt 		* -1);
				suwk_rec.loc_amt	= no_dec (local_rec.gl_amt_loc	* -1);
			}
		 	strcpy (suwk_rec.currency, suin_rec.currency);
			cc = RF_ADD (suwkNumber, (char *) &suwk_rec);
			if (cc) 
				file_err (cc, "suwkNumber", "WKADD");

		}
		/*------------------------------------------------
		| Delete suid detail records for invoice/credit. |
		------------------------------------------------*/
		suid_rec.hhsi_hash = suin_rec.hhsi_hash;
		suid_rec.line_no = 0;
		cc = find_rec (suid ,&suid_rec, GTEQ, "u");	
		while (!cc && suid_rec.hhsi_hash == suin_rec.hhsi_hash)
		{
			cc = abc_delete (suid);
			if (cc)
				file_err (cc, suid, "DBDELETE");

			cc = find_rec (suid ,&suid_rec, GTEQ, "u");	
		}
		abc_unlock (suid);
	}
	/*---------------------------------------------------------
	| Invoice/Credit Not Approved - Update I/C Suid Detail.   |
	---------------------------------------------------------*/
	else if (!APPROVED)
	{
		/*------------------------------------------------
		| Update suid detail records for invoice/credit. |
		------------------------------------------------*/
		scn_set (2);
		for (workLineCounter = 0;workLineCounter < lcount [2];workLineCounter++)
		{
			getval (workLineCounter);
	
			suid_rec.hhsi_hash = suin_rec.hhsi_hash;
			suid_rec.line_no = workLineCounter;
			cc = find_rec (suid ,&suid_rec, COMPARISON, "u");	

			strcpy (suid_rec.pf_type, local_rec.gps_type);
			strcpy (suid_rec.pf_ref, local_rec.gps_ref);
			strcpy (suid_rec.gr_no, popc_rec.gr_no);
			strcpy (suid_rec.po_no, popc_rec.po_no);
			suid_rec.hhsh_hash = popc_rec.hhsh_hash;
			suid_rec.hhpo_hash = popc_rec.hhpo_hash;
			suid_rec.hhgr_hash = popc_rec.hhgr_hash;
			suid_rec.ct_type = local_rec.cat_no;
			suid_rec.hhsi_hash = suin_rec.hhsi_hash;
			suid_rec.line_no = workLineCounter;
			strcpy (suid_rec.gl_acc_no, local_rec.acc_no);
			suid_rec.period_amt = local_rec.gl_amt;
			strcpy (suid_rec.narrative, local_rec.gl_narr);
			strcpy (suid_rec.user_ref, local_rec.gl_user_ref);
			strcpy (suid_rec.stat_flag, "0");
			if (cc)
			{
				abc_unlock (suid);
				cc = abc_add (suid ,&suid_rec);
				if (cc)
					file_err (cc, suid, "DBADD");
			}
			else
			{
				cc = abc_update (suid ,&suid_rec);
				if (cc)
					file_err (cc, suid, "DBUPDATE");
			}
		}
		/*-----------------------------------------
		| Remove any deleted suid detail records. |
		-----------------------------------------*/
		suid_rec.hhsi_hash = suin_rec.hhsi_hash;
		suid_rec.line_no = lcount [2];
		cc = find_rec (suid ,&suid_rec, GTEQ, "u");	
		while (!cc && suid_rec.hhsi_hash == suin_rec.hhsi_hash)
		{
			cc = abc_delete (suid);
			if (cc)
				file_err (cc, suid, "DBDELETE");

			cc = find_rec (suid ,&suid_rec, GTEQ, "u");	
		}
		abc_unlock (suid);
	}

	/*-----------------------------------------
	| Remove any deleted popc detail records. |
	-----------------------------------------*/
	popc_rec.hhsi_hash = suin_rec.hhsi_hash;
	popc_rec.line_no = 0;
	cc = find_rec (popc,&popc_rec, GTEQ, "u");	
	while (!cc && popc_rec.hhsi_hash == suin_rec.hhsi_hash)
	{
		cc = abc_delete (popc);
		if (cc)
			file_err (cc, "popc", "DBDELETE");
		cc = find_rec (popc ,&popc_rec, GTEQ, "u");	
	}
	abc_unlock (popc);

	to_val = (lcount [2] == 0) ? 1 : lcount [2];

	for (workLineCounter = 0;workLineCounter < lcount [2]; workLineCounter++)
	{
		getval (workLineCounter);
			
		if (local_rec.gps_type [0] == 'N')
			continue;
		
		popc_rec.hhsi_hash = suin_rec.hhsi_hash;
		popc_rec.line_no = workLineCounter;
		new_popc = find_rec (popc , &popc_rec, COMPARISON, "u");

		getval (workLineCounter);
		strcpy (popc_rec.co_no, comm_rec.co_no);
		strcpy (popc_rec.category, local_rec.cat_desc);
		popc_rec.cat_no = local_rec.cat_no;
		popc_rec.hhsu_hash 	= sumr_rec.hhsu_hash;
		strcpy (popc_rec.invoice, suin_rec.inv_no);
		strcpy (popc_rec.currency, suin_rec.currency);
		popc_rec.fgn_val = DOLLARS (local_rec.gl_amt);
		popc_rec.loc_val = DOLLARS (local_rec.gl_amt_loc);
		popc_rec.exch_rate 	= suin_rec.exch_rate;

		if (CREDIT)
		{
			popc_rec.fgn_val *= -1 ;
			popc_rec.loc_val *= -1 ;
		}

		if (new_popc)
		{
			abc_unlock ("popc");
			cc = abc_add (popc, &popc_rec);
		}
		else
			cc = abc_update (popc, &popc_rec);

		if (cc)
			file_err (cc, "popc", "DBFIND");
		
	}
	return;
}


/*===============================================
| Determine Payment Date From Invoice Date.     |
===============================================*/
void
PaymentDate (
 void)
{
	int		mth_offset,
			days;	/* Month offset for payment          	*/

	int		mth,
			yr;

	days = atoi (sumr_rec.pay_terms);

	if (sumr_rec.pay_terms [2] >= 'A')
	{
		/*-------------------------------------
		| Calc. for format NNA to NNF input . |
		-------------------------------------*/
		mth_offset = sumr_rec.pay_terms [2] - 'A' + 1;
		DateToDMY (suin_rec.date_of_inv, NULL, &mth, &yr);
		mth = mth + mth_offset;
		if (mth > 12)
		{
			mth = mth - 12;
			yr++;
		}
		suin_rec.pay_date	=	DMYToDate (days, mth, yr);
	}
	/*-------------------------------------
	| Calc. for format NN input.          |
	-------------------------------------*/
	else
		suin_rec.pay_date = suin_rec.date_of_inv + days;
}

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchSuin (
 char *	key_val)
{
	char	disp_amt [22];
	double	inv_balance;
	
	_work_open (15,0,40);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (suin_rec.inv_no,"%-15.15s", key_val);
	save_rec ("#Document","#     Amount    | App ");
	cc = find_rec (suin , &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val,strlen (key_val)) && 
			 (suin_rec.hhsu_hash == sumr_rec.hhsu_hash))
	{
		if ( (INVOICE && suin_rec.type [0] == '1') ||
		 (CREDIT  && suin_rec.type [0] == '2'))
		{
		    if (suin_rec.approved [0] != 'Y')
		    {
				inv_balance = suin_rec.amt - suin_rec.amt_paid;
				sprintf (disp_amt, "%-14.2f |  %-1.1s ",
								DOLLARS (inv_balance),
								suin_rec.approved);
				cc = save_rec (suin_rec.inv_no, disp_amt);
				if (cc)
					break;
		    }
		}
		cc = find_rec (suin , &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&suin_rec, 0, sizeof (suin_rec));
		return;
	}

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (suin_rec.inv_no,"%-15.15s", temp_str);
	cc = find_rec (suin , &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "suin", "DBFIND");
}

/*=========================================================
| Load category descriptions if defined else use default. |
=========================================================*/
void
LoadCatDesc (
 void)
{
	char *	sptr;
	int		i;

	for (i = 0; i < 11; i++)
	{
		putchar ('.');fflush (stdout);
		switch (i)
		{
		case 0:
			sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			break;

		case 1:
			sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			break;

		case 2:
			sptr = chk_env ("PO_OS_1");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;

		case 3:
			sptr = chk_env ("PO_OS_2");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 4:
			sptr = chk_env ("PO_OS_3");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 5:
			sptr = chk_env ("PO_OS_4");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 6:
			sprintf (cat_desc [i], "%-20.20s", inv_cat [i]);
			break;
		case 7:
			sptr = chk_env ("PO_OTHER1");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 8:
			sptr = chk_env ("PO_OTHER2");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 9:
			sptr = chk_env ("PO_OTHER3");
			if (sptr == (char *)0)
				sprintf (cat_desc [i],"%-20.20s",inv_cat [i]);
			else
				sprintf (cat_desc [i],"%-20.20s",sptr);
			break;
		case 10:
			sptr = chk_env ("PO_OTHER4");
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

/*===================
| Update all files. |
===================*/
void
RecDirectDelivery (
 void)
{
	int		i;
	int		allLinesConf	= TRUE;
	int		allLinesDesp	= TRUE;

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	cc = 0;
	while (!cc)
	{
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		sprintf (err_str, "%ld", ++esmr_rec.nx_gr_no);
		strcpy (pogh_rec.gr_no, zero_pad (err_str, 15));
		cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
	}
	cc = abc_update (esmr, &esmr_rec);
	if (cc)
		file_err (cc, "esmr", "DBUPDATE");
	abc_fclose (esmr);

	strcpy (pogh_rec.pur_status, "R");
	strcpy (pogh_rec.gl_status,  "R");
	pogh_rec.exch_rate = pohr_rec.curr_rate;

	strcpy (pogh_rec.co_no, comm_rec.co_no);
	strcpy (pogh_rec.br_no, comm_rec.est_no);
	pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
	pogh_rec.hhsh_hash = 0L;
	pogh_rec.hhpo_hash = pohr_rec.hhpo_hash;
	pogh_rec.hhds_hash = ddsh_rec.hhds_hash;
	strcpy (pogh_rec.rec_by, "P");
	strcpy (pogh_rec.pur_ord_no, pohr_rec.pur_ord_no);
	pogh_rec.date_raised = comm_rec.inv_date;
	if (pohr_rec.drop_ship [0] == 'Y')
		strcpy (pogh_rec.drop_ship, "Y");
	else
		strcpy (pogh_rec.drop_ship, "N");
	cc = abc_add (pogh, &pogh_rec);
	if (cc)
		file_err (cc, "pogh", "DBADD");

	strcpy (pogh_rec.co_no, comm_rec.co_no);
	sprintf (err_str, "%ld", esmr_rec.nx_gr_no);
	strcpy (pogh_rec.gr_no, zero_pad (err_str, 15));
	cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");

	/*-------------------------------
	| Update Purchase Order Header	|
	-------------------------------*/
	strcpy (pohr_rec.status, "R");
	cc = abc_update (pohr, &pohr_rec);
	if (cc)
		file_err (cc, "pohr", "DBUPDATE");

	abc_selfield (poln, "poln_hhpl_hash");

	abc_selfield (ddln, "ddln_id_no2");
	ddln_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
	while (!cc && 
		   ddln_rec.hhdd_hash == pohr_rec.hhdd_hash &&
		   ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
	{
		cc = find_hash (poln, &poln_rec, EQUAL, "u", ddln_rec.hhpl_hash);
		if (cc)
			file_err (cc, "poln", "DBFIND");
		if (poln_rec.qty_ord != 0.00)
		{
		    UpdatePoln ();

			AddPogl ();
		}
		cc = find_rec (ddln, &ddln_rec, NEXT, "r"); 
	}

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
	pohs_rec.est_cost += 1.00;
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

	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no3");

	for (i = 0; i < 10; i++)
	{
		strcpy (pogd_rec.co_no, comm_rec.co_no);
		pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
		pogd_rec.line_no = i;

		cc = find_rec (pogd, &pogd_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock ("pogd");
			continue;
		}

		pogd_rec.hhpo_hash = 0L;
		pogd_rec.hhsh_hash = 0L;
		pogd_rec.hhgr_hash = pogh_rec.hhgr_hash;

		cc = abc_update (pogd, &pogd_rec);
		if (cc)
			abc_delete (pogd);
	}
	abc_unlock ("pogd");
	abc_fclose (pogd);

	/*---------------------------------------------
	| Update Shipment (ddsh) status to Despatched |
	---------------------------------------------*/
	ddsh_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddsh_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (ddsh_rec.ship_no, "%2.2s", local_rec.dd_ship_no);
	cc = find_rec (ddsh, &ddsh_rec, EQUAL, "u");
	if (cc)
	{
		sprintf (err_str, "DBFIND hhdd [%ld] hhsu [%ld] ship [%2.2s]",
				 pohr_rec.hhdd_hash, sumr_rec.hhsu_hash, 
				 local_rec.dd_ship_no);
		file_err (cc, "ddsh", err_str);
	}
	strcpy (ddsh_rec.stat_flag, "D");
	cc = abc_update (ddsh, &ddsh_rec);
	if (cc)
		file_err (cc, "ddsh", "DBUPDATE");


	/*----------------------------------------------
	| Update D-D lines (ddln) status to Despatched |
	----------------------------------------------*/
	abc_selfield (ddln, "ddln_id_no2");
	ddln_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
	while (!cc && 
		   ddln_rec.hhdd_hash == pohr_rec.hhdd_hash &&
		   ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
	{
		strcpy (ddln_rec.stat_flag, "D");
		cc = abc_update (ddln, &ddln_rec);
		if (cc)
			file_err (cc, "ddln", "DBUPDATE");
		cc = find_rec (ddln, &ddln_rec, NEXT, "r"); 
	}


	/*----------------------------------------------
	| Check if all D-D lines (ddln) are Despatched |
	| or Confirmed and update D-D header (ddhr)    |
	| status accordingly.                          |
	----------------------------------------------*/
	allLinesConf	= TRUE;
	allLinesDesp	= TRUE;

	abc_selfield (ddln, "ddln_id_no");
	ddln_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddln_rec.line_no = 0;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
	while (!cc && ddln_rec.hhdd_hash == pohr_rec.hhdd_hash)
	{
		if (strcmp (ddln_rec.stat_flag, "C") &&
			strcmp (ddln_rec.stat_flag, "D") &&
			strcmp (ddln_rec.stat_flag, "I"))
			allLinesConf = FALSE;

		if (strcmp (ddln_rec.stat_flag, "D") &&
			strcmp (ddln_rec.stat_flag, "I"))
			allLinesDesp = FALSE;

		cc = find_rec (ddln, &ddln_rec, NEXT, "r"); 
	}
	if (allLinesConf || allLinesDesp)
	{
		cc = find_hash (ddhr, &ddhr_rec, EQUAL, "u", pohr_rec.hhdd_hash);
		if (cc)
			file_err (cc, "ddhr", "DBFIND");
		if (allLinesDesp)
			strcpy (ddhr_rec.stat_flag, "D");
		else
			strcpy (ddhr_rec.stat_flag, "C");
		cc = abc_update (ddhr, &ddhr_rec);
		if (cc)
			file_err (cc, "ddhr", "DBUPDATE");
	}
}

void
UpdatePoln (
 void)
{
	poln_rec.ship_no = 0L;
	poln_rec.case_no = 0;
	strcpy (poln_rec.serial_no, " ");
	poln_rec.qty_rec = poln_rec.qty_ord;
	strcpy (poln_rec.pur_status,"R");

	cc = abc_update (poln, &poln_rec);
	if (cc)
		file_err (cc, "poln", "DBUPDATE");

	abc_unlock (poln);

	if (envPoSuHist)
		ProcessSuph ();
}

/*===================================
| Updated Supplier History records. |
===================================*/
void
ProcessSuph (
 void)
{
	if (pohr_rec.conf_date == 0L)
			pohr_rec.conf_date = pohr_rec.date_raised;

	if (poln_rec.due_date == 0L)
			poln_rec.due_date = pohr_rec.conf_date;

	strcpy (suph_rec.br_no, pohr_rec.br_no);
	suph_rec.hhbr_hash 		= poln_rec.hhbr_hash;
	suph_rec.hhum_hash 		= poln_rec.hhum_hash;
	suph_rec.hhcc_hash 		= poln_rec.hhcc_hash;
	suph_rec.hhsu_hash 		= pohr_rec.hhsu_hash;
	suph_rec.ord_date 		= pohr_rec.date_raised;
	suph_rec.due_date 		= poln_rec.due_date;
	suph_rec.ord_qty  		= poln_rec.qty_ord;
	suph_rec.rec_date 		= local_rec.lsystemDate;
	suph_rec.rec_qty  		= poln_rec.qty_ord;
	suph_rec.net_cost 		= poln_rec.fob_fgn_cst;
	suph_rec.land_cost 		= poln_rec.land_cst;
	strcpy (suph_rec.status, "A");
	strcpy (suph_rec.ship_method, pohr_rec.ship_method);
	suph_rec.ship_no = 0L;

	strcpy (suph_rec.drop_ship, pohr_rec.drop_ship);
	strcpy (suph_rec.grn_no, pogh_rec.gr_no);
	strcpy (suph_rec.po_no , pohr_rec.pur_ord_no);

	cc = abc_add (suph,&suph_rec);
	if (cc)
		file_err (cc, "suph", "DBADD");
}

void
AddPogl (
 void)
{
	pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
	pogl_rec.line_no   = poln_rec.line_no;
	pogl_rec.hhbr_hash = poln_rec.hhbr_hash;
	pogl_rec.hhum_hash = poln_rec.hhum_hash;
	pogl_rec.hhcc_hash = poln_rec.hhcc_hash;
	pogl_rec.hhpl_hash = poln_rec.hhpl_hash;
	pogl_rec.hhlc_hash = poln_rec.hhlc_hash;
	strcpy (pogl_rec.po_number, pohr_rec.pur_ord_no);
	strcpy (pogl_rec.serial_no, poln_rec.serial_no);
	strcpy (pogl_rec.lot_no,    " ");
	strcpy (pogl_rec.slot_no,   " ");
	pogl_rec.exp_date = 0L;

	strcpy (pogl_rec.location, "          ");

	pogl_rec.qty_ord = poln_rec.qty_ord;
	pogl_rec.qty_rec = poln_rec.qty_ord;
	pogl_rec.fob_fgn_cst = poln_rec.fob_fgn_cst;
	pogl_rec.frt_ins_cst = poln_rec.frt_ins_cst;
	pogl_rec.fob_nor_cst = poln_rec.fob_nor_cst;
	pogl_rec.lcost_load  = poln_rec.lcost_load;
	pogl_rec.duty = poln_rec.duty;
	pogl_rec.licence = poln_rec.licence;
	pogl_rec.land_cst = poln_rec.land_cst;

	strcpy (pogl_rec.cat_code,  poln_rec.cat_code);
	strcpy (pogl_rec.item_desc, poln_rec.item_desc);
	pogl_rec.rec_date = comm_rec.inv_date;
	strcpy (pogl_rec.pur_status,"R");
	strcpy (pogl_rec.gl_status, "R");
	strcpy (pogl_rec.stat_flag, "0");

	cc = abc_add (pogl, &pogl_rec);
	if (cc)
		file_err (cc, "pogl", "DBADD");
}

void
CreateTableInfo (
 void)
{
	if (!AUTO_ALOC)
	{
		edit_ok (2);
		return;
	}

	scn_set (2);
	lcount [2] = 0;

	getval (lcount [2]);

	if (pogh_rec.drop_ship [0] == 'Y')
	{
		GL_GLI 
		 (
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"PO CLEAR D",
			" ", 
			" "
		);
		strcpy (local_rec.acc_no, glmrRec.acc_no);
	}
	else
	{
		GL_GLI 
		 (
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"PO CLEAR N",
			" ", 
			" "
		);
		strcpy (local_rec.acc_no, glmrRec.acc_no);
	}
		
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,local_rec.acc_no);
	cc = find_rec (glmr , &glmrRec, COMPARISON,"r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	if (CheckClass ())
		file_err (cc, glmr, "CHECK_CLASS");

	local_rec.gl_amt 	 = local_rec.net_inv;
	local_rec.gl_amt_loc = local_rec.net_inv_loc;
	strcpy (local_rec.gps_type, suin_rec.rec_type);
	strcpy (local_rec.gps_ref,  local_rec.doc_no);
	local_rec.cat_no = suin_rec.cst_type;
	strcpy (local_rec.cat_desc, local_rec.h_cat_desc);
	if (envCrNarrative [0] == 'N')
		strcpy (local_rec.gl_user_ref, local_rec.supplierNo);
	else
		strcpy (local_rec.gl_user_ref, suin_rec.cus_po_no);
	
	if (suin_rec.rec_type [0] == 'S')
		sprintf (local_rec.gl_narr, "SH # %15.15s", local_rec.gps_ref);

	if (suin_rec.rec_type [0] == 'P')
		sprintf (local_rec.gl_narr, "PO # %-15.15s", local_rec.gps_ref);
		
	if (suin_rec.rec_type [0] == 'D')
		sprintf (local_rec.gl_narr, "DD # %-15.15s", local_rec.gps_ref);
		
	if (suin_rec.rec_type [0] == 'G')
		sprintf (local_rec.gl_narr, "GR # %-15.15s", local_rec.gps_ref);
	
	store [lcount [2]].glAllocation = local_rec.gl_amt;

	if (suin_rec.rec_type [0] == 'P'|| suin_rec.rec_type [0] == 'D') 
	{
		strcpy (popc_rec.gr_no, fifteenSpaces);
		strcpy (popc_rec.po_no, pogh_rec.pur_ord_no);
		popc_rec.hhsh_hash = 0L;
		popc_rec.hhpo_hash = pogh_rec.hhpo_hash;
		popc_rec.hhgr_hash = 0L;
	}
	if (suin_rec.rec_type [0] == 'S') 
	{
		strcpy (popc_rec.gr_no, fifteenSpaces);
		strcpy (popc_rec.po_no, fifteenSpaces);
		popc_rec.hhsh_hash = pogh_rec.hhsh_hash;
		popc_rec.hhpo_hash = 0L;
		popc_rec.hhgr_hash = 0L;
	}
	if (suin_rec.rec_type [0] == 'G') 
	{
		strcpy (popc_rec.gr_no, pogh_rec.gr_no);
		strcpy (popc_rec.po_no, fifteenSpaces);
		popc_rec.hhsh_hash = 0L;
		popc_rec.hhpo_hash = 0L;
		popc_rec.hhgr_hash = pogh_rec.hhgr_hash;
	}
	putval (lcount [2]++);
	scn_set (1);

	no_edit (2);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		fflush (stdout);
		if (!setupOptionFlag)
		{
			if (INVOICE)
				rv_pr (ML (mlCrMess099),47,0,1);
			else
				rv_pr (ML (mlCrMess100),47,0,1);
		}
		else
			rv_pr (ML (mlCrMess106),45,0,1);

		print_at (0,75,ML (mlCrMess043),local_rec.prevInvoiceNo);
		fflush (stdout);
		move (0,1);
		line (131);

		move (1,input_row);
		switch (scn)
		{
		case  1 :
			if (MCURR) 
			{
				box (0,2,131, (GST) ? 16 : 14); 
					
				move (1,9);
				line (130);
				move (1,14);
				line (130);
				us_pr (ML ("Base Currency Value"),3,15,1);
				us_pr (ML ("Local Currency Equiv"),60,15,1);
				if (suin_rec.rec_type [0] == 'D')
				{
					print_at (7, 92, ML ("- Shipment"));
					print_at (7, 104, "%2.2s", local_rec.dd_ship_no);
				}
			}
			else       
			{
				box (0,2,131, (GST) ? 13 : 11);
					
				move (1,9);
				line (130);
				move (1,12);
				line (130);
			}
			break;
		
		case  2 :
			print_at (2,1,ML (mlStdMess063), sumr_rec.crd_no,sumr_rec.crd_name);
			fflush (stdout);
			if (MCURR)
				print_at (3,1,ML (mlCrMess107), suin_rec.currency, DOLLARS (local_rec.net_inv), local_rec.loc_curr, DOLLARS (local_rec.net_inv_loc));
			else
				print_at (3,1,ML (mlCrMess107), " ", DOLLARS (local_rec.net_inv), " ", 0.00);

			DisplayAllocate ();
			fflush (stdout);
			break;

		case  3 :
			box (0,3,131,3);
			move (1,5);
			line (130);
			break;
		}
		move (1,20);
		line (131);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
CheckClass (
 void)
{
	if (glmrRec.glmr_class [0][0] != 'F' ||
					glmrRec.glmr_class [2][0] != 'P')
	      return (print_err ("Account %s Is Not A Posting Account.",
							glmrRec.desc));

	return (EXIT_SUCCESS);
}

int
win_function (
 int fld,
 int lin,
 int scn,
 int mode)
{
	if (scn != 1)
		return (FALSE);
		
	/*-------------------------------------------
	| Clear space where negotiation window was. |
	-------------------------------------------*/
#ifdef GVISION

    struct tagTaxVals   taxValues [DBOX_ROW];

    for (int i = 0; i < DBOX_ROW; i++)
    {
        strcpy (taxValues [i].taxCode, local_rec.tax_code [i]);
        taxValues [i].taxAmnt = local_rec.tax_amnt [i];
        taxValues [i].taxVal  = local_rec.tax_val [i];
        taxValues [i].taxRate = local_rec.tax_rate [i];
    }

    ViewTax (DBOX_LFT, DBOX_TOP,
             DBOX_WID, DBOX_DEP,
             DBOX_ROW,
             taxValues,
             local_rec.net_inv,
             local_rec.gross);

    for (i = 0; i < DBOX_ROW; i++)
    {
        strcpy (local_rec.tax_code [i], taxValues [i].taxCode);
        local_rec.tax_amnt [i] = taxValues [i].taxAmnt;
        local_rec.tax_val [i]  = taxValues [i].taxVal;
        local_rec.tax_rate [i] = taxValues [i].taxRate;
    }

#else
    ViewTax ();
#endif  /* GVISION */

	return (TRUE);
}

#ifdef GVISION
BOOL   
FindTaxCode (
 char *     taxCode,
 float &    taxRate,
 long &     taxDate)
{
    sprintf (inth_rec.co_no,    "%-2.2s", comm_rec.co_no);
    sprintf (inth_rec.tax_code, "%-2.2s", taxCode);
    cc = find_rec (inth, &inth_rec, EQUAL, "r");
    if (!cc)
    {
        taxRate = inth_rec.tax_rate;
        taxDate = inth_rec.eff_date;
        return (TRUE);
    }

    return (FALSE);
}

#else
/*--------------------------------------
| Allow editing of multiple tax lines. |
--------------------------------------*/
void
ViewTax (
 void)
{
	int		i;
	int		key;
	int		exit = FALSE;
	int		currRow;
	int		currFld;
	char	oldCode [DBOX_ROW][2];
	double	oldAmnt [DBOX_ROW];
	double	oldVal [DBOX_ROW];

	/*------------------
	| Save old values. |
	------------------*/
	for (i = 0; i < DBOX_ROW; i++)
	{
		strcpy (oldCode [i], local_rec.tax_code [i]);
		oldAmnt [i]	= local_rec.tax_amnt [i];
		oldVal [i] 	= local_rec.tax_val [i];
	}

	/*----------------------
	| Draw box and fields. |
	----------------------*/
	DrawTaxScn ();

	/*-----------------------------------------------
	| Allow cursor movement and selection for edit. |
	| Exit without change on FN1.                   |
	| Exit saving changes on FN16.                  |
	-----------------------------------------------*/
	crsr_off ();
	currFld = 0;
	currRow = 0;
	restart = FALSE;
	DispFlds (currRow, currFld);
	while ( (key = getkey ()))
	{
		switch (key)
		{
		case UP_KEY:
			currRow--;
			if (currRow < 0)
				currRow = DBOX_ROW - 1;
			break;

		case DOWN_KEY:
			currRow++;
			if (currRow >= DBOX_ROW)
				currRow = 0;
			break;

		case BS:
		case LEFT_KEY:
			currFld--;
			if (currFld < 0)
				currFld = 2;
			break;

		case RIGHT_KEY:
		case ' ':
			currFld++;
			if (currFld > 2)
				currFld = 0;
			break;

		case '\r':
			InputField (currRow, currFld);
			break;

		case FN1:
			restart = TRUE;
			break;

		case FN3:
			heading (1);
			scn_display (1);
			DrawTaxScn ();
			DispFlds (currRow, currFld);
			break;

		case FN16:
			if (!ValidateTotal ())
				exit = TRUE;
		}

		DispFlds (currRow, currFld);
		if (exit)
			break;

		if (restart)
		{
			/*---------------------
			| Restore old values. |
			---------------------*/
			for (i = 0; i < DBOX_ROW; i++)
			{
				strcpy (local_rec.tax_code [i], oldCode [i]);
				local_rec.tax_amnt [i] = oldAmnt [i];
				local_rec.tax_val [i] = oldVal [i];
			}
			break;
		}
	}
	restart = FALSE;
}

void
DrawTaxScn (
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
	for (i = 1; i < 3; i++)
		DrawVLine (DBOX_LFT + taxScn [i].xPos, DBOX_TOP);

	/*---------------
	| Draw heading. |
	---------------*/
	headXPos = DBOX_LFT + (DBOX_WID - strlen (err_str)) / 2;
	rv_pr (ML (mlCrMess108), headXPos, DBOX_TOP, 1);

	/*---------------
	| Draw prompts. |
	---------------*/
	for (i = 0; i < 3; i++)
	{
		fldWid = strlen (taxScn [i].fldPrompt);
		print_at (DBOX_TOP + 1,
				 DBOX_LFT + taxScn [i].xPos + 1,
				 " %-*.*s ",
				 fldWid,
				 fldWid,
				 taxScn [i].fldPrompt);
	}
}

void
DrawVLine (
 int xPos,
 int yPos)
{
	int	i;

	move (xPos, yPos);
	PGCHAR (8);

	move (xPos, yPos + 1);
	PGCHAR (5);

	move (xPos, yPos + 2);
	PGCHAR (7);

	for (i = 0; i < DBOX_ROW; i++)
	{
		move (xPos, yPos + i + 3);
		PGCHAR (5);
	}
	move (xPos, yPos + i + 3);
	PGCHAR (9);
}

void
DispFlds (
 int rvsRow,
 int rvsField)
{
	int	i;

	for (i = 0; i < DBOX_ROW; i++)
	{
		print_at (DBOX_TOP + i + 3,DBOX_LFT + taxScn [0].xPos + 2,
				 		"%-2.2s", local_rec.tax_code [i]);
		if (strlen (clip (local_rec.tax_code [i])) != 0)
		{
			print_at (DBOX_TOP + i + 3,DBOX_LFT + taxScn [1].xPos + 2, 
					 		"%12.2f", DOLLARS (local_rec.tax_amnt [i]));
			print_at (DBOX_TOP + i + 3,DBOX_LFT + taxScn [2].xPos + 2, 
					 		"%12.2f", DOLLARS (local_rec.tax_val [i]));
		}
		else
		{
			print_at (DBOX_TOP + i + 3,DBOX_LFT + taxScn [1].xPos + 2, 
					 		"%12.2f", 0.00);
			print_at (DBOX_TOP + i + 3,DBOX_LFT + taxScn [2].xPos + 2, 
					 		"%12.2f", 0.00);
		}
	}

	/*--------------------------
	| Print highlighted field. |
	--------------------------*/
	switch (rvsField)
	{
	case 0:
		sprintf (err_str, "%-2.2s", local_rec.tax_code [rvsRow]);
		break;

	case 1:
		sprintf (err_str, "%12.2f", DOLLARS (local_rec.tax_amnt [rvsRow]));
		break;

	case 2:
		sprintf (err_str, "%12.2f", DOLLARS (local_rec.tax_val [rvsRow]));
		break;
	}
	rv_pr (err_str, DBOX_LFT + taxScn [rvsField].xPos + 2, DBOX_TOP + 3 + rvsRow, 1);
}

/*---------------------------
| Validate Tax Amount Input	|
---------------------------*/
int
ValidTaxAmnt (
 int row)
{
	int		i;
	double	alloc = 0.00;

	if (last_char == FN1)
	{
		restart = TRUE;
		return (EXIT_SUCCESS);
	}

	if (last_char != '\r')
		return (EXIT_FAILURE);

	alloc = 0.00;
	for (i = 0; i < DBOX_ROW; i++)
		if (strlen (clip (local_rec.tax_code [i])) != 0
		&&  i != row)
			alloc += local_rec.tax_amnt [i];

	if (dflt_used)
		local_rec.tax_amnt [row]  = local_rec.net_inv - alloc;

	if (local_rec.tax_amnt [row] > (local_rec.net_inv - alloc))
	{
		print_mess (ML (mlCrMess109));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	local_rec.tax_val [row] = local_rec.tax_amnt [row] 
						   * local_rec.tax_rate [row] / 100;

	return (EXIT_SUCCESS);
}

/*---------------------------
| Validate Tax Value Input	|
---------------------------*/
int
ValidTaxVal (
 int row)
{
	if (last_char == FN1)
	{
		restart = TRUE;
		return (EXIT_SUCCESS);
	}

	if (last_char != '\r')
		return (EXIT_FAILURE);

	if (dflt_used)
	{
		local_rec.tax_val [row] = local_rec.tax_amnt [row] 
							   * local_rec.tax_rate [row] / 100;
	}

	if (local_rec.tax_val [row] > local_rec.tax_amnt [row])
	{
		print_mess (ML (mlCrMess110));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*---------------------------
| Validate Tax Code Input	|
---------------------------*/
int
ValidTaxCode (
 int row)
{
	int		i;
	long	today;
	double	tot_amnt = 0.00;
	double	tot_val  = 0.00;

	today = TodaysDate ();

	if (last_char == FN1)
	{
		restart = TRUE;
		return (EXIT_SUCCESS);
	}

	if (last_char == DELLINE)
	{
		strcpy (local_rec.tax_code [row], "  ");
		local_rec.tax_amnt [row] = 0.00;
		local_rec.tax_val [row]  = 0.00;
		return (EXIT_SUCCESS);
	}

	if (SRCH_KEY)
	{
		cc = SrchInth (local_rec.tax_code [row]);
		if (!cc && last_char != FN1)
		{
			strcpy (local_rec.tax_code [row], inth_rec.tax_code);
			local_rec.tax_amnt [row]  = 0.00;
			local_rec.tax_val [row]   = 0.00;
			local_rec.tax_rate [row]  = inth_rec.tax_rate;
		}
		else
		{
			strcpy (local_rec.tax_code [row], "  ");
			local_rec.tax_amnt [row]  = 0.00;
			local_rec.tax_val [row]   = 0.00;
			local_rec.tax_rate [row]  = 0.00;
		}
		heading (1);
		scn_display (1);
		DrawTaxScn ();
		DispFlds (row, 0);
		last_char = SEARCH;
		return (EXIT_FAILURE);
	}
	else
		if (last_char != '\r')
			return (EXIT_FAILURE);

	if (strlen (clip (local_rec.tax_code [row])) == 0)
	{
		print_mess (ML (mlStdMess051));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	sprintf (inth_rec.co_no,    "%-2.2s", comm_rec.co_no);
	sprintf (inth_rec.tax_code, "%-2.2s", local_rec.tax_code [row]);
	cc = find_rec (inth, &inth_rec, EQUAL, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess051));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (inth_rec.eff_date > today)
	{
		print_mess (ML (mlStdMess052));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	local_rec.tax_rate [row] = inth_rec.tax_rate;
	tot_amnt = 0.00;
	tot_val  = 0.00;
	for (i = 0; i < DBOX_ROW; i++)
	{
		if (strlen (clip (local_rec.tax_code [i])) != 0
		&&  i != row)
		{
			tot_amnt += local_rec.tax_amnt [i];
			tot_val  += local_rec.tax_val [i];
		}
	}

	if (local_rec.net_inv != 0.00)
	{
		local_rec.tax_amnt [row]  = local_rec.net_inv - tot_amnt;
		local_rec.tax_val [row] = local_rec.tax_amnt [row] 
							   * local_rec.tax_rate [row] / 100;
	}
	else
	{
		local_rec.tax_val [row] = local_rec.gross - tot_amnt - tot_val;
		local_rec.tax_amnt [row] = local_rec.tax_val [row]
								/ (1 + (local_rec.tax_rate [row] / 100));
		local_rec.tax_val [row] = local_rec.tax_val [row] 
							   - local_rec.tax_amnt [row];
	}

	local_rec.tax_amnt [row] = twodec (local_rec.tax_amnt [row]);
	local_rec.tax_val [row]  = twodec (local_rec.tax_val [row]);

	return (EXIT_SUCCESS);
}

int
ValidateTotal (
 void)
{
	int		i;
	double	theoreticalTotal;
	double	actualTotal;
	double	difference;

	theoreticalTotal = 0.00;
	actualTotal      = 0.00;

	for (i = 0; i < DBOX_ROW; i++)
	{
		if (strlen (clip (local_rec.tax_code [i])) != 0)
		{
			actualTotal      += local_rec.tax_val [i];
			theoreticalTotal += (local_rec.tax_amnt [i]
							  *  local_rec.tax_rate [i]) / 100;
		}
	}
	if (theoreticalTotal == 0.00)
		difference = 0.00;
	else
	{
		difference = fabs (actualTotal - theoreticalTotal);
		difference = twodec ( (difference / theoreticalTotal) * 100);
	}
	if (difference <= envCrTaxTol)
	{
		local_rec.gst = actualTotal;
		return (EXIT_SUCCESS);
	}
	else
	{
		sprintf (err_str,ML (mlCrMess111),DOLLARS (actualTotal),envCrTaxTol, 
				DOLLARS (theoreticalTotal));
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
}

void
InputField (
 int row,
 int fld)
{
	int		fieldOk;

	crsr_on ();

	fieldOk = FALSE;
	while (!fieldOk)
	{
		crsr_on ();
		fieldOk = TRUE;
		switch (fld)
		{
		case 0:
			getalpha (DBOX_LFT + taxScn [fld].xPos + 2, 
				  	 DBOX_TOP + row + 3,
				  	 taxScn [fld].fldMask,
					 local_rec.tax_code [row]);
			fieldOk = !ValidTaxCode (row);
			break;
	
		case 1:
			if (strlen (clip (local_rec.tax_code [row])) == 0)
				break;
			local_rec.tax_amnt [row] = getmoney (DBOX_LFT + taxScn [fld].xPos + 2, 
								   			   DBOX_TOP + row + 3,
								   			   taxScn [fld].fldMask);
			fieldOk = !ValidTaxAmnt (row);
			break;
	
		case 2:
			if (strlen (clip (local_rec.tax_code [row])) == 0)
				break;
			local_rec.tax_val [row] = getmoney (DBOX_LFT + taxScn [fld].xPos + 2, 
								   			  DBOX_TOP + row + 3,
								   			  taxScn [fld].fldMask);
			fieldOk = !ValidTaxVal (row);
			break;
		}
	}
	crsr_off ();
}
#endif	/* GVISION */

/*=======================
| Search for Shipments. |
=======================*/
int
SrchDdsh (
 char *	key_val)
{
	work_open ();
	save_rec ("#Ship No", "#Due Date    ");
	ddsh_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddsh_rec.hhsu_hash = pohr_rec.hhsu_hash;
	strcpy (ddsh_rec.ship_no, "  ");
	cc = find_rec (ddsh, &ddsh_rec, GTEQ, "r");

	while (!cc && 
		   ddsh_rec.hhdd_hash == pohr_rec.hhdd_hash &&
		   ddsh_rec.hhsu_hash == pohr_rec.hhsu_hash)
	{
		strcpy (err_str, DateToString (ddsh_rec.due_date));
		cc = save_rec (ddsh_rec.ship_no, err_str);
		if (cc)
			break;
		cc = find_rec (ddsh, &ddsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	return (cc);
}

/*------------------
| Tax Code Search. |
------------------*/
int
SrchInth (
 char *	key_val)
{
	char	descStr [100];

	_work_open (2,0,60);
	save_rec ("#Cd", "#             Description                | Tax %");

	strcpy (inth_rec.co_no, comm_rec.co_no);
	sprintf (inth_rec.tax_code, "%-2.2s", key_val);
	cc = find_rec (inth, &inth_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inth_rec.co_no, comm_rec.co_no) &&
	       !strncmp (inth_rec.tax_code, key_val, strlen (key_val)))
	{
		sprintf (descStr,"%-40.40s|%7.2f", inth_rec.tax_desc, inth_rec.tax_rate);
		cc = save_rec (inth_rec.tax_code, descStr);
		if (cc)
			break;

		cc = find_rec (inth, &inth_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	strcpy (inth_rec.co_no, comm_rec.co_no);
	sprintf (inth_rec.tax_code, "%-2.2s", temp_str);
	cc = find_rec (inth, &inth_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inth", "DBFIND");
	return (EXIT_SUCCESS);
}

int
DateWithinMonth (
	long	date1,
	long	date2)
{
	if (!envCrInvDateChk)
		return (TRUE);

	if (date1 < MonthStart (date2) || date1 > MonthEnd (date2))
	{
		cc = prmptmsg 
			 (
				ML ("Date is not within current period, accept? (Y/N) "),
			   "YyNn", 
			   1, 
			   23
		);
		return (cc == 'Y' || cc == 'y');
	}
	return (TRUE);
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_mr_inpt.c,v 5.7 2002/04/11 03:30:38 scott Exp $
|  Program Name  : (cr_mr_inpt.c)
|  Program Desc  : (Add/ Maintain Suppliers Master File)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_mr_inpt.c,v $
| Revision 5.7  2002/04/11 03:30:38  scott
| Updated to add comments to audit files.
|
| Revision 5.6  2001/10/05 02:53:45  cha
| Added code to produce audit files.
|
| Revision 5.5  2001/09/19 00:50:29  scott
| Updated from scotts. machine.
|
| Revision 5.4  2001/08/28 10:11:28  robert
| additional update for LS10.5-GUI
|
| Revision 5.3  2001/08/28 06:19:36  scott
| aUpdated for " (
|
| Revision 5.2  2001/08/09 08:52:06  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:37  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_mr_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_mr_inpt/cr_mr_inpt.c,v 5.7 2002/04/11 03:30:38 scott Exp $";

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off
#define	TXT_REQD

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <p_terms.h>
#include <DBAudit.h>

#define	MCURR	 (multiCurrency [0] == 'Y')

/*========
 Globals
==========*/

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int		newSupplier	= 0,
			envCrCo,
			envCrFind,
			tx_window,
			tx_lines,
			bBlankWin = TRUE,
			workFileNumber;
	extern int lp_x_off,
			lp_y_off;

	char	branchNumber [3],
			multiCurrency [2],
			taxCodePrompt [24],
			taxNumberPrompt [24],
			taxCommand [70],
			taxAllowed [5],
			gstCode [4],
			gst [2];

	char	*currentUser;
	int		QA_Installed = FALSE;

	char	currencyCode [4];
	char	countryCode [4];
	char	contcode [4];
	char	shipDefault [2];
	char	headText [120];

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct exsiRecord	exsi_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct pocfRecord	pocf_rec;
struct poclRecord	pocl_rec;
struct qamrRecord	qamr_rec;
struct qasdRecord	qasd_rec;
struct srcrRecord	srcr_rec;

 	int		*sumr_sic		=	&sumr_rec.sic1;

/*===========
 Table names
============*/
static char		*data	= 	"data",
				*sumr2	=	"sumr2";
	
	extern	int	TruePosition;

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	dummy [11];
	char	prev_item [7];
	char	systemDate [11];
	char	cur_crd [7];
	char	acct_desc [81];
	char	dflt_acct [MAXLEVEL + 1];
	char	dflt_acct_desc [85];
	char	ship_desc [5];
	char	spindesc [3][61];
	char	qa_text_edit [2];
 	char	mail_lbls [2];
 	char	cust_lett [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber", 	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier Number     ", "enter supplier or [SEARCH] ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cur_crd},
	{1, LIN, "estno", 	 3, 43, CHARTYPE,
		"NN", "          ",
		" ", " 0",  "Branch Number       ", "Enter 0 for company owned supplier. ",
		 NO, NO, JUSTRIGHT, "0", "99", sumr_rec.est_no},
	{1, LIN, "acronym", 	 4, 2, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "Supplier acronym    ", "Enter supplier acronym, must be unique ",
		YES, NO,  JUSTLEFT, "", "", sumr_rec.acronym},
	{1, LIN, "acctype", 	 4, 43, CHARTYPE,
		"U", "          ",
		" ", "O", "Account type        ", "Enter account type Open item / Balance b/f ",
		YES, NO,  JUSTLEFT, "BO", "", sumr_rec.acc_type},
	{1, LIN, "name", 	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier name       ", "Enter supplier name ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "adr1", 	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 1      ", "Enter supplier address part 1 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.adr1},
	{1, LIN, "adr2", 	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 2      ", "Enter supplier address part 2 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.adr2},
	{1, LIN, "adr3", 	 9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 3      ", "Enter supplier address part 3 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.adr3},
	{1, LIN, "adr4", 	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 4      ", "Enter supplier address part 4 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.adr4},
	{1, LIN, "contact", 	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name 1      ", "Enter supplier contact #1 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.cont_name},
	{1, LIN, "contact", 	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name 2      ", "Enter supplier contact #2 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.cont2_name},
	{1, LIN, "contact", 	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name 3      ", "Enter supplier contact #3 ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.cont3_name},
	{1, LIN, "email", 	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Email. ", "Enter suppliers email account ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.email},
	{1, LIN, "phoneno", 	12, 43, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Phone Number        ", "Enter suppliers phone number ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.cont_no},
	{1, LIN, "fax_no", 	13, 43, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Fax number          ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.fax_no},
	{1, LIN, "dbtrno", 	14, 43, CHARTYPE,
		"AAAAAAAAAAAA", "          ",
		" ", "", "Our account number  ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.debtor_no},
	{1, LIN, "sup_pri", 	17, 2, INTTYPE,
		"N", "          ",
		" ", "1","Supplier priority   ", "(1-9) 1 = Most important.",
		 NO, NO,  JUSTLEFT, "1", "9", (char *)&sumr_rec.sup_pri},
	{1, LIN, "ship_method",	18, 2, CHARTYPE,
		"U", "          ",
		" ", shipDefault, "Shipment method     ", "Default shipment method L(and) / S(ea) / A(ir) / R(ail)",
		YES, NO,  JUSTLEFT, "LSAR", "", sumr_rec.ship_method},
	{1, LIN, "ship_desc",	18, 26, CHARTYPE,
		"UUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ship_desc},
 	{1, LIN, "mail_lbls",	17, 43, CHARTYPE,
 		"U", "          ",
 		" ", "Y", "Print labels ?      ", "Print mailing labels? Y(es) / N(o).",
 		 NO, NO,  JUSTLEFT, "YN", "", local_rec.mail_lbls},
 	{1, LIN, "cust_lett",	18, 43, CHARTYPE,
 		"U", "          ",
 		" ", "Y", "Print Letters ?     ", "Print customer letters? Y(es) / N(o).",
 		 NO, NO,  JUSTLEFT, "YN", "", local_rec.cust_lett},
	{2, LIN, "curcode", 	3, 2, CHARTYPE,
		"UUU", "          ",
		" ", currencyCode, "Currency code     ", "enter currency or [SEARCH] ",
		YES, NO,  JUSTLEFT, "", "", sumr_rec.curr_code},
	{2, LIN, "curdesc", 	3, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{2, LIN, "ctrycode", 	4, 2, CHARTYPE,
		"UUU", "          ",
		" ", countryCode,  "Country code      ", "enter country or [SEARCH] ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.ctry_code},
	{2, LIN, "ctrydesc", 	4, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocf_rec.description},
	{2, LIN, "payterm", 	 6, 2, CHARTYPE,
		"UUU", "          ",
		" ", "20A", "Payment Terms     ", "nnA to nnF (20A = 20th next mth); Alt.<nnn> = no.days",
		 NO, NO, JUSTLEFT, "0123456789ABCDEF", "", sumr_rec.pay_terms},
	{2, LIN, "pay_method", 	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Payment Method    ", "C)heque, D)irect or Draft payment, T)ransfer",
		YES, NO,  JUSTLEFT, "CDT", "", sumr_rec.pay_method},
	{2, LIN, "hold_pay", 	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Hold Payments     ", "Y)es, N)o [default = N]",
		YES, NO,  JUSTLEFT, "YN", "", sumr_rec.hold_payment},
	{2, LIN, "open_date", 	 9, 2, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.systemDate, "Date acct. opened ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&sumr_rec.date_opened},
	{2, LIN, "discount", 	 6, 38, FLOATTYPE,
		"NN.NN", "          ",
		" ", "", "Settlement disc % ", " ",
		 NO, NO,  JUSTLEFT, "0", "99.99", (char *)&sumr_rec.disc},
 	{2, LIN, "cust_lett",	7, 38, CHARTYPE,
 		"U", "          ",
 		" ", "Y", "Print Remittance? ", "Print customer Remittance? Y(es) / N(o).",
 		 NO, NO,  JUSTLEFT, "YN", "", sumr_rec.remm_prn},
	{2, LIN, "taxcode",	8, 38, CHARTYPE,
		"U", "          ",
		" ", "C", taxCodePrompt, taxCommand,
		 NO, NO,  JUSTLEFT, taxAllowed, "", sumr_rec.tax_code},
	{2, LIN, "taxno",	9, 38, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", taxNumberPrompt, "",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.gst_reg_no},
	{2, LIN, "ctrlacct", 	 11, 2, CHARTYPE,
		GlMask, "          ",
		"0", local_rec.dflt_acct, "Control Account   ", "enter account or [SEARCH] ",
		YES, NO,  JUSTLEFT, "0123456789*", "", sumr_rec.gl_ctrl_acct},
	{2, LIN, "ctrlacctdesc",	 11, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", local_rec.dflt_acct, "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.acct_desc},
	{2, LIN, "bk_code", 	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Code         ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.bank_code},
	{2, LIN, "bk_acno", 	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank account No.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.bank_acct_no},
	{2, LIN, "bank", 	 12, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank              ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.bank},
	{2, LIN, "bk_branch", 	13, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch            ", " ",
		 NO, NO,  JUSTLEFT, "", "", sumr_rec.bank_branch},
	{2, LIN, "type_code", 	14, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier Type     ", "Enter Supplier type code or [ SEARCH ] to display/select.",
		 NO, NO, JUSTLEFT, "", "", sumr_rec.type_code},
	{2, LIN, "type_desc", 	14, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocl_rec.desc},
	{2, LIN, "spinst1", 	16, 2, INTTYPE,
		"NNN", "          ",
		" ", "", "Instr. 1 ", " ",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&sumr_rec.sic1},
	{2, LIN, "spindesc1", 	16, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [0]},
	{2, LIN, "spinst2", 	17, 2, INTTYPE,
		"NNN", "          ",
		" ", "", "Instr. 2 ", " ",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&sumr_rec.sic2},
	{2, LIN, "spindesc2", 	17,15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [1]},
	{2, LIN, "spinst3", 	18, 2, INTTYPE,
		"NNN", "          ",
		" ", "", "Instr. 3 ", " ",
		 NO, NO,  JUSTLEFT, "0", "999", (char *)&sumr_rec.sic3},
	{2, LIN, "spindesc3", 	18, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spindesc [2]},
	{3, LIN, "qa_status",	 3, 2, CHARTYPE,
		"N", "          ",
		" ", "", "QA status code  ", " ",
		YES, NO,  JUSTLEFT, "1", "9", sumr_rec.qa_status},
	{3, LIN, "qa_desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "QA description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", qamr_rec.qa_desc},
	{3, LIN, "qa_expiry", 	 5, 2, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", "", "QA Expiry Date  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&sumr_rec.qa_expiry},
	{3, LIN, "qa_text", 	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y","Edit Text (Y/N) ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.qa_text_edit},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include	<get_lpno.h>
#include	<FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB			 (void);
int		ReadDefault		 (void);
int		spec_valid		 (int);
void	Update			 (void);
void	UpdateSrcr		 (void);
void	SrchQamr		 (char *);
void	SrchPocr		 (char *);
void	SrchPocf		 (char *);
void	SrchPocl		 (char *);
void	SrchEsmr		 (char *);
void	SrchExsi		 (char *);
void	DisplayBranch	 (void);
int		CheckClass		 (void);
void	QasdDisplay		 (void);
void	SrchPay			 (void);
void	QasdMaint		 (void);
int		heading			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	TruePosition	=	TRUE;

	currentUser = getenv ("LOGNAME");
	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	sprintf (currencyCode, "%-3.3s", get_env ("CURR_CODE"));
	sprintf (countryCode, "%-3.3s", get_env ("CONT_CODE"));
	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	sprintf (gst,"%-1.1s", get_env ("GST"));

	/*-------------------------------
	| Take Out Tax for GST Clients. |
	-------------------------------*/
	if (gst [0] == 'Y' || gst [0] == 'y')
	{
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
		sprintf (taxCommand, "A=%-3.3s Exempt, C=%-3.3s applies.", 
				 gstCode, gstCode);
		sprintf (taxAllowed, "AC");
	}
	else
	{
		sprintf (gstCode, "%-3.3s", "Tax");
		sprintf (taxCommand, "A=Tax Exempt, B=Tax Your Care, C=Taxed. D=Taxed using Tax Amount.");
		sprintf (taxAllowed, "ABCD");
	}
	sprintf (taxCodePrompt,   "%-3.3s Code.         ", gstCode);
	sprintf (taxNumberPrompt, "%-3.3s No.           ", gstCode);

	sptr = chk_env ("QA_INSTALLED");
	QA_Installed = (sptr == (char *) 0) ? FALSE : atoi (sptr);
		
	/*------------------------------------------
	| Shipment Default. A(ir) / L(and) / S(ea) |
	------------------------------------------*/
	sptr = chk_env ("PO_SHIP_DEFAULT");
	if (sptr == (char *) 0)
		sprintf (shipDefault, "S");
	else
	{
		switch (*sptr)
		{
		case	'S':
		case	's':
			sprintf (shipDefault, "S");
			break;

		case	'L':
		case	'l':
			sprintf (shipDefault, "L");
			break;

		case	'A':
		case	'a':
			sprintf (shipDefault, "A");
			break;

		default:
			sprintf (shipDefault, "S");
			break;
		}
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	/*=============================
	 Open db & read in setup stuff
	===============================*/
	OpenDB ();

	GL_SetMask (GlFormat);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	_set_masks ("cr_mr_inpt.s");
	init_vars (1);

	GL_SetMask (GlFormat);

	/*--------------------------
	| read all default files . |
	--------------------------*/
	if (!ReadDefault ())
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}


	strcpy (local_rec.prev_item, "000000");

	while (!prog_exit)
	{
       	abc_unlock (sumr);

		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newSupplier	= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		init_vars (2);

		memset (&sumr_rec, 0, sizeof (sumr_rec));

		/*------------------------------ 
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		if (newSupplier)
		{
			/*------------------------------ 
			| Enter screen 2 linear input. |
			------------------------------*/
			heading (2);
			entry (2);
			if (prog_exit || restart) 
				continue;
		}
		else
			scn_display (1);

		if (!QA_Installed)
			no_edit (3);

		edit_all ();
		if (restart) 
			continue;

		/*-------------------------
		| Update supplier record. |
		-------------------------*/
		Update ();
	}

	/*========================
	| Program exit sequence. |
	========================*/
	if (!bBlankWin)
	{
		Dsp_close ();
		bBlankWin = TRUE;
	}

	clear ();
	rset_tty ();
	crsr_on ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (sumr2, sumr);	
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envCrFind) 
						? "sumr_id_no" : "sumr_id_no3");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (sumr2,sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
	open_rec (pocl, pocl_list, POCL_NO_FIELDS, "pocl_id_no");
	open_rec (qamr, qamr_list, QAMR_NO_FIELDS, "qamr_id_no");
	open_rec (qasd, qasd_list, QASD_NO_FIELDS, "qasd_id_no");

	OpenGlmr ();
	OpenPocr ();
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("SupplierMasterFile.txt");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr2);
	abc_fclose (sumr);
	abc_fclose (esmr);
	abc_fclose (exsi);
	abc_fclose (pocf);
	abc_fclose (pocl);
	abc_fclose (qamr);
	abc_fclose (qasd);
	GL_Close ();
	RF_CLOSE (workFileNumber);
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

/*========================================
| Read all necessary files for defaults. |
========================================*/
int
ReadDefault (void)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		"  ",
		"ACCT PAY  ",
		"   ",
		" "
	);
	strcpy (local_rec.dflt_acct_desc, glmrRec.desc);
	strcpy (local_rec.dflt_acct, glmrRec.acc_no);

	return (TRUE);
}

int
spec_valid (
 int field)
{
	int		i;
	int 	temp;
	int 	val_pterms;
	char	work [10];

	/*--------------------------------------------
	| Validate Supplier Number And Allow Search. |
	--------------------------------------------*/
	if (LCHECK ("supplierNumber"))
	{
 		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
	}
			
	/*----------------------------------------------------------
	| Validate Establishment number (only if entered) i.e. > 0 |
	----------------------------------------------------------*/
	if (LCHECK ("estno"))
	{
 		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!envCrCo)
		{
			strcpy (sumr_rec.est_no, " 0");
			strcpy (sumr2_rec.est_no, " 0");
			DSP_FLD ("estno");
		}
		else
		{
		    if (atoi (sumr_rec.est_no) != 0) 
		    {
				strcpy (esmr_rec.co_no, comm_rec.co_no);
				strcpy (esmr_rec.est_no, sumr_rec.est_no);
				cc = find_rec (esmr, (char *) &esmr_rec, COMPARISON, "r");
				if (cc)
				{
					errmess (ML (mlStdMess073));
					return (EXIT_FAILURE); 
				}
				strcpy (sumr_rec.est_no, esmr_rec.est_no);
				strcpy (sumr2_rec.est_no, esmr_rec.est_no);
				strcpy (branchNumber, esmr_rec.est_no);
		    }
		    else 
		    {
				errmess (ML (mlCrMess164));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
		    }
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr2_rec.co_no, comm_rec.co_no); 
		strcpy (sumr_rec.crd_no, pad_num (local_rec.cur_crd));
		newSupplier = find_rec (sumr, (char *) &sumr_rec, COMPARISON, "w");
		if (newSupplier == 0)
		{
			entry_exit = 1;
			strcpy (pocrRec.co_no, comm_rec.co_no);
			strcpy (pocrRec.code, sumr_rec.curr_code);
			cc = find_rec (pocr, (char *) &pocrRec, COMPARISON, "r");
			if (cc)
				sprintf (pocrRec.description, "%40.40s", " ");
	
			else if (MCURR)
				strcpy (local_rec.dflt_acct, pocrRec.gl_ctrl_acct);
			strcpy (pocf_rec.co_no, comm_rec.co_no);
			strcpy (pocf_rec.code, sumr_rec.ctry_code);
			cc = find_rec (pocf, (char *) &pocf_rec, COMPARISON, "r");
			if (cc)
				sprintf (pocf_rec.description, "%20.20s", " ");

			strcpy (pocl_rec.co_no, comm_rec.co_no);
			strcpy (pocl_rec.type,  sumr_rec.type_code);
			cc = find_rec (pocl, (char *) &pocl_rec, COMPARISON, "r");
			if (cc)
				sprintf (pocl_rec.desc, "%40.40s", " ");

			for (i = 1; i <= 3; i++)
			{
 				strcpy (exsi_rec.co_no, comm_rec.co_no);
 				exsi_rec.inst_code = sumr_sic [i - 1];
 
 				cc = find_rec (exsi, (char *) &exsi_rec, EQUAL, "r");
 				if (cc)
 				{
 					strcpy (local_rec.spindesc [i - 1], " ");
 				}
				else
					strcpy (local_rec.spindesc [i - 1], exsi_rec.inst_text);

				switch (sumr_rec.ship_method [0])
				{
					case 'L' :	strcpy (local_rec.ship_desc, ML ("Land"));
								break;
					case 'S' :	strcpy (local_rec.ship_desc, ML ("Sea "));
								break;
					case 'A' :	strcpy (local_rec.ship_desc, ML ("Air "));
								break;
					case 'R' :	strcpy (local_rec.ship_desc, ML ("Rail"));
								break;
				}
				DSP_FLD ("ship_desc"); 
				strcpy (local_rec.mail_lbls, sumr_rec.mail_label);
				strcpy (local_rec.cust_lett, sumr_rec.letter);
				if (!strcmp (local_rec.cust_lett, "S"))
					strcpy (local_rec.cust_lett, " "); 
				DSP_FLD ("mail_lbls");
				DSP_FLD ("cust_lett");
				/*
				 * Save old record.
				 */
				SetAuditOldRec (&sumr_rec, sizeof (sumr_rec));
			}
			strcpy (qamr_rec.co_no, comm_rec.co_no);
			strcpy (qamr_rec.br_no, comm_rec.est_no);
			strcpy (qamr_rec.qa_status, sumr_rec.qa_status);
			cc = find_rec ("qamr", &qamr_rec, COMPARISON, "r");
			if (cc) 
				strcpy (qamr_rec.qa_desc, " ");

			strcpy (glmrRec.acc_no, sumr_rec.gl_ctrl_acct);
			
			GL_FormAccNo (sumr_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
			strcpy (glmrRec.co_no, comm_rec.co_no);
			cc = find_rec (glmr, (char *) &glmrRec, COMPARISON, "r");
			sprintf (local_rec.acct_desc, "%s - (%3.3s)", 
					 glmrRec.desc, glmrRec.curr_code);
		}
		DisplayBranch ();
		DSP_FLD ("supplierNumber");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Suppliers Acronym. |
	-----------------------------*/
	if (LCHECK ("acronym"))
	{
		strcpy (sumr2_rec.acronym, sumr_rec.acronym);
		cc = find_rec (sumr2, (char *) &sumr2_rec, COMPARISON, "r");
		if (cc == 0 && strcmp (sumr2_rec.crd_no, sumr_rec.crd_no) != 0) 
		{
			errmess (ML (mlStdMess181));
			return (EXIT_FAILURE);
		}

		if (strcmp (sumr_rec.acronym, "         ") == 0) 
		{
			errmess (ML (mlCrMess165));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("payterm"))
	{
		if (SRCH_KEY)
		{
			SrchPay ();
			return (EXIT_SUCCESS);
		}
		val_pterms = FALSE;

		/*-------------------------------------
		| Check for format NNA to NNF input . |
		-------------------------------------*/
		if (sumr_rec.pay_terms [2] >= 'A')
		{
			temp = atoi (sumr_rec.pay_terms);
			if (temp < 1 || temp > 30) 
			{ 
				errmess (ML (mlCrMess166));
				return (EXIT_FAILURE);
			}
			else   
				sprintf (sumr_rec.pay_terms, "%02d%c",
					       temp, sumr_rec.pay_terms [2]);
		}
		/*------------------------------------
		| Check for straight numeric input . |
		------------------------------------*/
		else 
		{
			temp = atoi (sumr_rec.pay_terms);
			if (temp < 0 || temp > 999) 
			{
				errmess (ML (mlStdMess182));
				return (EXIT_FAILURE);
			}
		}

        for (i = 0;strlen (p_terms [i]._pcode);i++)
        {
            if (!strncmp (sumr_rec.pay_terms,
				 p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
            {
                sprintf (sumr_rec.pay_terms,"%-3.3s", p_terms [i]._pterm);
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

		DSP_FLD ("payterm");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Currency Code Input. |
	-------------------------------*/
	if (LCHECK ("curcode"))
	{
 		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocrRec.co_no, comm_rec.co_no);
		strcpy (pocrRec.code, sumr_rec.curr_code);
		cc = find_rec (pocr, (char *) &pocrRec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("curdesc");

		if (MCURR)
			strcpy (local_rec.dflt_acct, pocrRec.gl_ctrl_acct);
			
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Country Code Input. |
	------------------------------*/
	if (LCHECK ("ctrycode"))
	{
 		if (SRCH_KEY)
		{
			SrchPocf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocf_rec.co_no, comm_rec.co_no);
		strcpy (pocf_rec.code, sumr_rec.ctry_code);
		cc = find_rec (pocf, (char *) &pocf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("ctrydesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Control Account. |
	---------------------------*/
	if (LCHECK ("ctrlacct"))
	{
 		if (SRCH_KEY)
			return SearchGlmr_C (	comm_rec.co_no, 
								temp_str, 
								"F*P", 
								sumr_rec.curr_code);

		GL_FormAccNo (sumr_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
		
		strcpy (glmrRec.co_no, comm_rec.co_no);
		cc = find_rec (glmr, (char *) &glmrRec, COMPARISON, "r");
		if (cc) 
		{
		    print_err (ML (mlStdMess024));
		    return (EXIT_FAILURE);
		}
				
		if (CheckClass ())
			return (EXIT_FAILURE);

		if (strncmp (glmrRec.curr_code, sumr_rec.curr_code, 3))
		{
		    print_err (ML (mlCrMess168),glmrRec.acc_no);
		    return (EXIT_FAILURE);
		}

		sprintf (local_rec.acct_desc, "%s - (%3.3s)", 
				 glmrRec.desc, glmrRec.curr_code);
		DSP_FLD ("ctrlacctdesc");

		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Supplier type code. |
	------------------------------*/
	if (LCHECK ("type_code"))
	{
		if (SRCH_KEY)
		{
			SrchPocl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocl_rec.co_no, comm_rec.co_no);
		strcpy (pocl_rec.type,  sumr_rec.type_code);
		cc = find_rec (pocl, (char *) &pocl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess183));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("type_desc");
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
 			sumr_sic [i - 1] = 0;
 			return (EXIT_SUCCESS);
 		}
 
 		if (SRCH_KEY)
 		{
 			SrchExsi (temp_str);
 			return (EXIT_SUCCESS);
 		}
 		strcpy (exsi_rec.co_no, comm_rec.co_no);
 		exsi_rec.inst_code = sumr_sic [i - 1];
 
 		cc = find_rec (exsi, (char *) &exsi_rec, COMPARISON, "r");
 		if (cc)
 		{
 			print_mess (ML (mlStdMess184));
 
 			sleep (sleepTime);
 			clear_mess ();
 			return (EXIT_FAILURE);
 		}
		strcpy (local_rec.spindesc [i - 1], exsi_rec.inst_text);
		DSP_FLD (work);
		sprintf (work, "spindesc%1d", i);
		DSP_FLD (work);
 		return (EXIT_SUCCESS);
 	}

	/*-----------------------------------
	| Validate Default Shipment Method. |
	-----------------------------------*/
	if (LCHECK ("ship_method"))
	{
		switch (sumr_rec.ship_method [0])
		{
			case 'L' :	strcpy (local_rec.ship_desc, "Land");
						break;
			case 'S' :	strcpy (local_rec.ship_desc, "Sea ");
						break;
			case 'A' :	strcpy (local_rec.ship_desc, "Air ");
						break;
			case 'R' :	strcpy (local_rec.ship_desc, "Rail");
						break;
		}
		DSP_FLD ("ship_desc");
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("qa_status"))
	{
		if (SRCH_KEY)
		{
			SrchQamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qamr_rec.co_no, comm_rec.co_no);
		strcpy (qamr_rec.br_no, comm_rec.est_no);
		strcpy (qamr_rec.qa_status, sumr_rec.qa_status);
		cc = find_rec ("qamr", &qamr_rec, COMPARISON, "r");
		if (cc) 
 		{
 			print_mess (ML (mlStdMess074));
 
 			sleep (sleepTime);
 			clear_mess ();
 			return (EXIT_FAILURE);
		}
		DSP_FLD ("qa_desc");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qa_text"))
	{
		if (local_rec.qa_text_edit [0] == 'Y')
			QasdMaint ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (
 void)
{
	clear ();
 	sprintf (sumr_rec.mail_label,  "%-1.1s",   local_rec.mail_lbls);
 	sprintf (sumr_rec.letter,      "%-1.1s",   local_rec.cust_lett);

	if (newSupplier)
	{
		strcpy (sumr_rec.stat_flag, "0");
		strcpy (sumr_rec.crd_no, pad_num (local_rec.cur_crd));
		cc = abc_add (sumr, &sumr_rec);
		if (cc) 
			file_err (cc, sumr, "DBADD");

		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, sumr, "DBFIND");

	}
	else 
	{
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %s (%s)", ML ("Supplier"),sumr_rec.crd_no,sumr_rec.crd_name);
		 AuditFileAdd 
		 (
		 	err_str,
		 	&sumr_rec, 
			sumr_list, 
			SUMR_NO_FIELDS
		);
		cc = abc_update (sumr, &sumr_rec);
		if (cc) 
			file_err (cc, sumr, "DBUPDATE");

        	abc_unlock (sumr);
	}
	/*------------------------------
	| Update Supplier Search file. |
	------------------------------*/
	UpdateSrcr ();

	strcpy (local_rec.prev_item, local_rec.cur_crd);
}

/*==================================
| Update search file for Supplier. |
==================================*/
void
UpdateSrcr (
 void)
{
	int	Newsrcr;

	open_rec (srcr, srcr_list, SRCR_NO_FIELDS, "srcr_hhsu_hash");

	srcr_rec.hhsu_hash = sumr_rec.hhsu_hash;

	Newsrcr = find_rec ("srcr", &srcr_rec, EQUAL, "u");

	strcpy (err_str, sumr_rec.crd_name);
	strcpy (srcr_rec.co_no, 		sumr_rec.co_no);
	strcpy (srcr_rec.br_no, 		sumr_rec.est_no);
	strcpy (srcr_rec.crd_no,		sumr_rec.crd_no);
	srcr_rec.hhsu_hash =			sumr_rec.hhsu_hash;
	strcpy (srcr_rec.acronym,		sumr_rec.acronym);
	strcpy (srcr_rec.type_code,		sumr_rec.type_code);
	strcpy (srcr_rec.contact_name,	sumr_rec.cont_name);
	strcpy (srcr_rec.name,			upshift (err_str));

	if (Newsrcr)
	{
		cc = abc_add ("srcr", &srcr_rec);
		if (cc)
			file_err (cc, "srcr", "DBADD");
	}
	else
	{
		cc = abc_update ("srcr", &srcr_rec);
		if (cc)
			file_err (cc, "srcr", "DBUPDATE");
	}
	abc_fclose (srcr);
}

/*====================================
| Search for Quality Assurance code. |
====================================*/
void
SrchQamr (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#QA code description");
	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", key_val);

	cc = find_rec ("qamr", &qamr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (qamr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (qamr_rec.br_no, comm_rec.est_no) &&
		   !strncmp (qamr_rec.qa_status, key_val, strlen (key_val)))
	{
		cc = save_rec (qamr_rec.qa_status, qamr_rec.qa_desc);
		if (cc)
			break;

		cc = find_rec ("qamr", &qamr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", temp_str);
	cc = find_rec ("qamr", &qamr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "qamr", "DBFIND");
}
/*===================================
| Search routine for Currency file. |
===================================*/
void
SrchPocr (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Currency code description.");
	strcpy (pocrRec.co_no, comm_rec.co_no);
	strcpy (pocrRec.code, key_val);
	cc = find_rec (pocr, (char *) &pocrRec, GTEQ, "r");
	while (!cc && !strncmp (pocrRec.code, key_val, strlen (key_val)) && 
		      !strcmp (pocrRec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;
		cc = find_rec (pocr, (char *) &pocrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pocrRec.co_no, comm_rec.co_no);
	strcpy (pocrRec.code, temp_str);
	if ( (cc = find_rec (pocr, (char *) &pocrRec, GTEQ, "r")))
		file_err (cc, pocr, "DBFIND");
}

/*============================================
| Search routine for Country / Freight file. |
============================================*/
void
SrchPocf (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Country code description.");
	strcpy (pocf_rec.co_no, comm_rec.co_no);
	strcpy (pocf_rec.code, key_val);
	cc = find_rec (pocf, (char *) &pocf_rec, GTEQ, "r");
	while (!cc && !strncmp (pocf_rec.code, key_val, strlen (key_val)) && 
		      !strcmp (pocf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocf_rec.code, pocf_rec.description);
		if (cc)
			break;
		cc = find_rec (pocf, (char *) &pocf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pocf_rec.co_no, comm_rec.co_no);
	strcpy (pocf_rec.code, temp_str);
	if ( (cc = find_rec (pocf, (char *) &pocf_rec, GTEQ, "r")))
		file_err (cc, pocf, "DBFIND");
}

/*====================================
| Search routine for Supplier class. |
====================================*/
void
SrchPocl (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#Class", "#Supplier class description.");
	strcpy (pocl_rec.co_no, comm_rec.co_no);
	strcpy (pocl_rec.type, key_val);
	cc = find_rec (pocl, (char *) &pocl_rec, GTEQ, "r");
	while (!cc && !strncmp (pocl_rec.type, key_val, strlen (key_val)) && 
		      !strcmp (pocl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocl_rec.type, pocl_rec.desc);
		if (cc)
			break;
		cc = find_rec (pocl, (char *) &pocl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocl_rec.co_no, comm_rec.co_no);
	strcpy (pocl_rec.type, temp_str);
	if ( (cc = find_rec (pocl, (char *) &pocl_rec, GTEQ, "r")))
		file_err (cc, pocl, "DBFIND");
}

/*===============================================
| Search routine for Establishment Master File. |
===============================================*/
void
SrchEsmr (
 char *	key_val)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, key_val);
	save_rec ("#No", "#Branch code description");
	cc = find_rec (esmr, (char *) &esmr_rec, GTEQ, "r");
	while (!cc && !strncmp (esmr_rec.est_no, key_val, strlen (key_val)) && 
		      !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr, (char *) &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, temp_str);
	if ( (cc = find_rec (esmr, (char *) &esmr_rec, COMPARISON, "r")))
		file_err (cc, esmr, "DBFIND");
}

/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char *	key_val)
{
 	char	wk_code [4];
 
	_work_open (3,0,60);
 	save_rec ("#No.", "#Special Instruction description.");
 
 	strcpy (exsi_rec.co_no, comm_rec.co_no);
 	exsi_rec.inst_code = atoi (key_val);
 
 	cc = find_rec (exsi, (char *) &exsi_rec, GTEQ, "r");
 	while (!cc && !strcmp (exsi_rec.co_no, comm_rec.co_no))
 	{
 		sprintf (wk_code, "%03d", exsi_rec.inst_code);
 		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
 		cc = save_rec (wk_code, err_str);
 		if (cc)
 			break;
 
 		cc = find_rec (exsi, (char *) &exsi_rec, NEXT, "r");
 	}
 	cc = disp_srch ();
 	work_close ();
 	if (cc)
 		return;
 
 	strcpy (exsi_rec.co_no, comm_rec.co_no);
 	exsi_rec.inst_code = atoi (temp_str);
 	if ( (cc = find_rec (exsi, (char *) &exsi_rec, COMPARISON, "r")))
 		file_err (cc, exsi, "DBFIND");
}

void
DisplayBranch (
 void)
{
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
}

int
CheckClass (
 void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
	    print_err (ML (mlStdMess025));
	    return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*===============================
| Quality Assurance Text input. |
===============================*/
void
QasdDisplay (
 void)
{
	char	disp_str [61];

	if (!bBlankWin)
	{
		Dsp_close ();
		bBlankWin = TRUE;
	}

	sprintf (headText, " Supplier : %6.6s (%40.40s)  ",
		sumr_rec.crd_no, 
		sumr_rec.crd_name);

	lp_x_off = 8;
	lp_y_off = 10;
	Dsp_prn_open (10,8,8, headText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					 (char *) 0, (char *) 0);
	Dsp_saverec ("  Q U A L I T Y   A S S U R A N C E   S C O P E   T E X T.  ");

	Dsp_saverec ("");
	Dsp_saverec ("");

	qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	qasd_rec.line_no   = 0;
	cc = find_rec (qasd, &qasd_rec, GTEQ, "r");

	while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sprintf (disp_str, "%-60.60s", qasd_rec.desc);
		Dsp_saverec (disp_str);

		cc = find_rec (qasd, &qasd_rec, NEXT, "r");
	}
	Dsp_srch ();
	//Dsp_close ();
	bBlankWin = FALSE;
}

void
SrchPay (
 void)
{
	int     i = 0;
	_work_open (3,0,40);
    save_rec ("#No.","#Payment terms description");

    for (i = 0;strlen (p_terms [i]._pcode);i++)
    {
        cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
        if (cc)
            break;
    }
    cc = disp_srch ();
    work_close ();
}

/*===============================
| Quality Assurance Text input. |
===============================*/
void
QasdMaint (
 void)
{
	int		i,
			last_line	=	0;

	char	disp_str [61];

	if (!bBlankWin)
	{
		Dsp_close ();
		bBlankWin = TRUE;
	}

tx_window = txt_open (8, 10, 10, 60, 300,
			"  Q U A L I T Y   A S S U R A N C E   S C O P E   T E X T.  ");

	qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	qasd_rec.line_no   = 0;
	cc = find_rec (qasd, &qasd_rec, GTEQ, "r");

	while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sprintf (disp_str, "%-60.60s", qasd_rec.desc);

		txt_pval (tx_window, disp_str, 0);

		cc = find_rec (qasd, &qasd_rec, NEXT, "r");
	}

	tx_lines = txt_edit (tx_window);
	if (!tx_lines)
	{
		txt_close (tx_window, FALSE);
		qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		qasd_rec.line_no   = 0;
		cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
		while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			abc_delete (qasd);
			cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
		}
		return;
	}
	for (i = 1; i <= tx_lines; i++)
	{
		last_line = i;

		qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		qasd_rec.line_no   = i -1;
		cc = find_rec (qasd, &qasd_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (qasd_rec.desc, "%-60.60s", txt_gval (tx_window, i));

			cc = abc_add (qasd, &qasd_rec);
			if (cc)
				file_err (cc, qasd, "DBADD");
		}
		else
		{
			sprintf (qasd_rec.desc, "%-60.60s", txt_gval (tx_window, i));

			cc = abc_update (qasd, &qasd_rec);
			if (cc)
				file_err (cc, qasd, "DBUPDATE");
		}
	}
	txt_close (tx_window, FALSE);

	qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	qasd_rec.line_no = last_line;
	cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
	while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		abc_delete (qasd);
		cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
	}
	QasdDisplay ();
	return;
}

/*====================================================
| Display Screen Headings                            |
====================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCrMess167), 20, 0, 1);

		print_at (0,55,ML (mlCrMess025), local_rec.prev_item);
		move (0, 1);
		line (80);

		if (scn == 1)
		{
			box (0, 2, 80, 16);
			line_at (6, 1, 79);
			line_at (11,1, 79);
			line_at (16,1, 79);
			/*
			move (1, 6);
			line (79);
			move (1, 11);
			line (79);
			move (1, 15);
			line (79);
			*/
		}
		
		if (scn == 2)
		{
			box (0, 2, 80, 16);
			move (1, 5);
			line (79);
			move (1, 10);
			line (79);
			move (1, 15);
			line (79);
		}
		if (scn == 3)
		{
			box (0, 2, 80, 4);
			QasdDisplay ();
		}
		
		move (1, 20);
		line (79);
		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0,err_str);
		DisplayBranch ();
		move (1, 23);
		line (79);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else 
		abc_unlock (sumr);

	return (EXIT_SUCCESS);
}

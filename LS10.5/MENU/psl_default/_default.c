/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _default.c,v 5.10 2002/11/21 10:16:33 kaarlo Exp $
|  Program Name  : (psl_default.c)                                 |
|  Program Desc  : (Master file defaults file.               )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/12/91         |
|---------------------------------------------------------------------|
| $Log: _default.c,v $
| Revision 5.10  2002/11/21 10:16:33  kaarlo
| LS01101 SC4158. Updated to display the Licence Code Description.
|
| Revision 5.9  2002/07/03 06:38:26  kaarlo
| S/C 4027. Updated to correct priceNumber from GetPriceDesc().
|
| Revision 5.8  2002/04/11 03:42:12  scott
| Updated to add comments to audit files.
|
| Revision 5.7  2002/03/01 02:22:47  scott
| S/C 00742 - SKMT7-Maintain Inventory Defaults; the Duty Code description does not display when program is run
|
| Revision 5.6  2002/01/29 11:05:10  robert
| SC 00726 - Updated to add delay for error messages
|
| Revision 5.5  2001/11/28 01:05:55  scott
| Issues List Reference Number = 00672
| Program Description = SKMT7-Maintain Inventory Defaults
| CHAR-BASED-ORACLE-HP
| A change made in any of the field does not save after pressing F12.  This is true only in the char-based of oracle-hp.  But in GUI, the program saves the changes.
|
| Revision 5.4  2001/10/05 02:58:18  cha
| Added code to produce audit files.
|
| Revision 5.3  2001/08/28 08:46:19  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 05:13:42  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:34  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _default.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_default/_default.c,v 5.10 2002/11/21 10:16:33 kaarlo Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <number.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <DBAudit.h>

#define		BY_INMR		(inputType [0] == 'I')
#define		BY_CUMR		(inputType [0] == 'C')
#define		BY_SUMR		(inputType [0] == 'S')


   	int 	newRecord 		= 0, 
			envDbCo			= 0, 
			envDbFind		= 0, 
			envGstInclusive = 0, 
			envGst 			= 1, 
			envSkGrades		= 0;

	char	gstPercent 		[6], 
			envGstTaxName 	[4], 
			gstPrompt 		[20], 
			inputType 		[2], 
			gst 			[2];

#include	"schema"

struct commRecord	comm_rec;
struct inmdRecord	inmd_rec;
struct ingdRecord	ingd_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inumRecord	inum3_rec;
struct cumdRecord	cumd_rec;
struct ccmrRecord	ccmr_rec;
struct podtRecord	podt_rec;
struct polhRecord	polh_rec;
struct esmrRecord	esmr_rec;
struct exdfRecord	exdf_rec;
struct dbryRecord	dbry_rec;

	char	*data  = "data", 
			*inum2 = "inum2", 
			*inum3 = "inum3";

	struct	{
		char	*_source;
		char	*_sdesc;
	} source [] = {
		{"BM", "Bulk Manufactured"}, 
		{"BP", "Bulk Product"}, 
		{"MC", "Manufactured Component"}, 
		{"MP", "Manufactured Product"}, 
		{"PP", "Purchased Product"}, 
		{"RM", "Raw Material"}, 
		{"", " "}, 
	};

	extern	int	TruePosition;
/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy [11];
	char 	typedesc [20];
	char 	pri_show [2];
	char 	pri_type_desc [30];
	char 	bf_dbt [41];
	char 	bf_stm [41];
	char 	stmnt_flg_desc [4];
	float	grade_pc;
	char	src_desc [41];
	char	std_uom [5];
	char	alt_uom [5];
	float	cnv_fact;
	char	dfltQty [15];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "class", 	3, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Class              ", "Enter Item Class (A-Z)", 
		 NO, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", inmd_rec.inmd_class}, 
	{1, LIN, "schg", 	 	3, 40, CHARTYPE, 
		"U", "          ", 
		" ", "0", "Surcharge Flag     ", "0-No Surcharge or 1-4 Use Surcharge item [SU1-4]", 
		 NI, NO,  JUSTLEFT, "S01234", "", inmd_rec.schg_flag}, 
	{1, LIN, "abc_code", 	4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "ABC Code           ", "Enter ABC Code (A-D)", 
		 NO, NO,  JUSTLEFT, "ABCD", "", inmd_rec.abc_code}, 
	{1, LIN, "abc_update", 	4, 40, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "ABC Update         ", "Automatic Update of ABC Code (Y/N)", 
		 NO, NO,  JUSTLEFT, "YN", "", inmd_rec.abc_update}, 

	{1, LIN, "lot_ctrl", 	5, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Lot Control (Y/N)  ", " Enter Y(es) if Lot Control Required.", 
		 NO, NO, JUSTLEFT, "YN", "", inmd_rec.lot_ctrl}, 
	{1, LIN, "dec_pt", 	5, 40, INTTYPE, 
		"N", "          ", 
		" ", "2", "Decimal Places     ", "(0-6) Number of decimal places for Stock. ", 
		YES, NO, JUSTRIGHT, "0", "6", (char *)&inmd_rec.dec_pt}, 
	{1, LIN, "serial", 	6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Serial Item        ", "Y(es) N(o)", 
		NO, NO, JUSTRIGHT, "YN", "", inmd_rec.serial_item}, 
	{1, LIN, "cost_flag", 	6, 40, CHARTYPE, 
		"U", "          ", 
		" ", "F", "Costing Type       ", "L=Last, A=Average, P=Previous, F=FIFO, I=LIFO, T=Standard, S=Serial", 
		NO, NO, JUSTRIGHT, "LAFIPST", "", inmd_rec.costing_flag}, 
	{1, LIN, "bo_flag", 	7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "B/Orders Accepted  ", "Y(es) = B/Orders are allowed, N(o) = not allowed, F(ull) B/Orders Only.", 
		 NO, NO,  JUSTLEFT, "YNF", "", inmd_rec.bo_flag}, 
	{1, LIN, "bo_release", 	7, 40, CHARTYPE, 
		"U", "          ", 
		" ", "A", "B/Order Release    ", "Enter M(anual) A(utomatic).", 
		 NO, NO,  JUSTLEFT, "AM", "", inmd_rec.bo_release}, 
	{1, LIN, "source", 	9, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "PP", "Source             ", "Use search Key for valid source Codes and descriptions. ", 
		YES, NO,  JUSTLEFT, "", "", inmd_rec.source}, 
	{1, LIN, "src_desc", 	9, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_desc}, 
	{1, LIN, "std_uom", 	10, 2, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "Standard UOM       ", "Use Search key for valid unit of Measures ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.std_uom}, 
	{1, LIN, "alt_uom", 	10, 40, CHARTYPE, 
		"AAAA", "          ", 
		" ", local_rec.std_uom, "Alternate UOM      ", "Use Search key for valid unit of Measures. ", 
		 NO, NO,  JUSTLEFT, "", "", local_rec.alt_uom}, 
	{1, LIN, "cnv_fact", 	11, 2, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "", "Conv Factor        ", "Enter Conversion Factor. ", 
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.cnv_fact}, 
	{1, LIN, "outer", 	11, 40, FLOATTYPE, 
		"NNNNN.N", "          ", 
		" ", "1.00", "Pricing Conversion ", " Cost & Sale quantity / price conversion.", 
		 NO, NO, JUSTRIGHT, "0", "99999.9", (char *)&inmd_rec.outer_size}, 
	{1, LIN, "safety", 	12, 2, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Safety Stock       ", " ", 
		 NO, NO, JUSTRIGHT, "0", "999999.99", (char *)&inmd_rec.safety_stock}, 
	{1, LIN, "gst", 		14, 2, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", gstPercent, gstPrompt, " ", 
		 NO, NO, JUSTRIGHT, "0", "99.99", (char *)&inmd_rec.gst_pc}, 
	{1, LIN, "taxp", 		14, 2, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", "0", "Tax %              ", " ", 
		 NO, NO, JUSTRIGHT, "0", "99.99", (char *)&inmd_rec.tax_pc}, 
	{1, LIN, "tax_amt", 	14, 40, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", "Last W/sale Price  ", "Input last wholesale price.", 
		 NO, NO, JUSTRIGHT, "0", "9999999.99", (char *)&inmd_rec.tax_amount}, 
	{1, LIN, "licence", 	 16, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Licence Code       ", "Blank if no licence applies", 
		 NO, NO,  JUSTLEFT, "", "", inmd_rec.licence}, 
	{1, LIN, "l_desc", 	 16, 32, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", polh_rec.type}, 
	{1, LIN, "duty", 	 17, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Duty Code          ", "Blank if no duty applies ", 
		 NO, NO,  JUSTLEFT, "", "", inmd_rec.duty}, 
	{1, LIN, "d_desc", 	 17, 32, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", podt_rec.description}, 
	{1, LIN, "duty_amt", 	18, 2, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", "",  "Duty Amount        ", "Use Search key for valid licence. ", 
		 NO, NO,  JUSTLEFT, "", "", (char *)&inmd_rec.duty_amt}, 
	{1, LIN, "eoq", 	18, 40, FLOATTYPE, 
		local_rec.dfltQty, "          ", 
		" ", "",  "Economic Order Qty ", "Economic Order Quantity.", 
		 NO, NO,  JUSTRIGHT, "0.0", "9999999.999999", (char *)&inmd_rec.eoq}, 
	{1, LIN, "dfltBom", 	19, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "1", "Default BOM No     ", "Default BOM Number.", 
		 NO, NO,  JUSTRIGHT, "1", "32767", (char *)&inmd_rec.dflt_bom}, 
	{1, LIN, "dfltRtg", 	19, 40, INTTYPE, 
		"NNNNN", "          ", 
		" ", "1", "Default RTG No     ", "Default Routing Number.", 
		 NO, NO,  JUSTRIGHT, "1", "32767", (char *)&inmd_rec.dflt_rtg}, 

	{2, LIN, "br_no", 	 3, 2, CHARTYPE, 
		"NN", "          ", 
		" ", " 0", "Branch                  ", "Enter <return> to default to company wide default.", 
		 NO, NO, JUSTRIGHT, "0", "99", cumd_rec.est_no}, 
	{2, LIN, "stmt_flg", 	 3, 35, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Statement flag          ", "Enter Y(es to enable statement printing, N(o to disable", 
		YES, NO,  JUSTLEFT, "YyNn", "", cumd_rec.stmnt_flg}, 
	{2, LIN, "stmnt_flg_desc", 	 3, 70, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.stmnt_flg_desc}, 
	{2, LIN, "acctype", 	 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "O", "Account type            ", "Enter B(alance Forward) O(pen Item)", 
		YES, NO,  JUSTLEFT, "BO", "", cumd_rec.acc_type}, 
	{2, LIN, "acc_desc", 	 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.bf_dbt}, 
	{2, LIN, "stmt_type", 	 5, 2, CHARTYPE, 
		"U", "          ", 
		" ", "O", "Statement type          ", "Enter B(alance Forward) O(pen Item)", 
		YES, NO,  JUSTLEFT, "BO", "", cumd_rec.stmt_type}, 
	{2, LIN, "stm_desc", 	 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.bf_stm}, 
	{2, LIN, "pri_type", 	 7, 2, CHARTYPE, 
		"N", "          ", 
		" ", "1", "Price Type              ", "Enter 1-9", 
		YES, NO,  JUSTLEFT, "123456789", "", local_rec.pri_show}, 
	{2, LIN, "pri_type_desc", 	 7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.pri_type_desc}, 
	{2, LIN, "crd_prd", 	 8, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "20A", "Credit Terms            ", "nnA to NNF (20A = 20th next mth); Alt.<nnn> = no.days", 
		 NO, NO,  JUSTLEFT, "", "", cumd_rec.crd_prd}, 
	{2, LIN, "b/cons", 	8, 35, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Consolidate B/Os        ", "Y(es if Backorder Consolidations Allowed", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.bo_cons}, 
	{2, LIN, "b/oflag", 	9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Back Orders Allowed     ", "Y(es if Backorders Allowed", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.bo_flag}, 
	{2, LIN, "b/days", 	9, 35, INTTYPE, 
		"NNN", "          ", 
		" ", "500", "Max Days On B/O         ", "Maximum number of days Backorders Allowed to Stay 0-500", 
		YES, NO,  JUSTLEFT, "0", "500", (char *)&cumd_rec.bo_days}, 
	{2, LIN, "gl_acc", 	 11, 2, CHARTYPE, 
		GlMask, "          ", 
		"0", " ", "Control Account         ", "Enter account or use [SEARCH] Key.", 
		YES, NO,  JUSTLEFT, "0123456789*", "", cumd_rec.gl_ctrl_acct}, 
	{2, LIN, "gl_acc_desc", 	 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Account Desc            ", " ", 
		 NA, NO,  JUSTLEFT, "", "", glmrRec.desc}, 
	{2, LIN, "p/oflag", 	14, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "P/Order Required        ", "Y(es if Customers purchase order is required ", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.po_flag}, 
	{2, LIN, "freight_chg", 	14, 35, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Freight Charges Allowed ", "Y(es if Customer can be charged freight.", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.freight_chg}, 
	{2, LIN, "restock_fee", 	15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Re-Stocking fee Allowed ", "Y(es if Customer can be charged a Re-stocking Fee.", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.restock_fee}, 
	{2, LIN, "nett_pri", 	15, 35, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Nett pricing used ?     ", "Y(es if Nett pricing used on Invoices/Credits/P-Slips.", 
		YES, NO,  JUSTLEFT, "YN", "", cumd_rec.nett_pri_prt}, 
	{2, LIN, "taxcode", 	17, 2, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Tax Code                ", "A=Tax Exempt, B=Tax Your Care, C=Taxed. D=Taxed using Tax Amount.", 
		 NO, NO,  JUSTLEFT, "ABCD", "", cumd_rec.tax_code}, 
	{2, LIN, "discode", 	18, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Discount Code           ", "Input A-Z.", 
		 NO, NO,  JUSTLEFT, "A", "Z", cumd_rec.disc_code}, 
	{2, LIN, "discpc", 	18, 35, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", "Discount Percent        ", "Input A-Z.", 
		 NA, NO, JUSTRIGHT, "0", "100", (char *)&exdf_rec.disc_pc}, 
	{2, LIN, "roycode", 	20, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Royalty Code            ", "Input Existing Royalty Code.", 
		 NO, NO,  JUSTLEFT, "", "", cumd_rec.roy_type}, 
	{2, LIN, "roydesc", 	20, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTRIGHT, "", "", dbry_rec.desc}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<p_terms.h>

/*============================
| Local function prototypes  |
============================*/
char	*GetPriceDesc 	(int);
int		CalcConversion	(void);
int		Check_class		(void);
int		heading			(int);
int		spec_valid		(int);
int		SrchIngd		(char *);
int		SrchInum		(char *);
int		Update			(void);
int		ValidUOM		(void);
void	CloseDB		 	(void);
void	FindMisc		(void);
void	InmdMisc		(void);
void	OpenDB			(void);
void	PriceDesc		(void);
void	shutdown_prog	(void);
void	SrchDbry		(char *);
void	SrchEsmr		(char *);
void	SrchExdf		(char *);
void	SrchPay		 	(void);
void	SrchPodt		(char *);
void	SrchPolh		(char *);
void	SrchSource		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	int		i;
	char *	sptr;

	TruePosition	=	TRUE;
	envGstInclusive = atoi (get_env ("GST_INCLUSIVE"));
	envDbCo 		= atoi (get_env ("DB_CO"));
	envDbFind 		= atoi (get_env ("DB_FIND"));

	envSkGrades = FALSE;
	sptr = chk_env ("SK_GRADES");
	if (sptr)
		envSkGrades = (*sptr == 'Y') ? TRUE : FALSE;

	if (argc != 2)
	{
		print_at (0, 0, mlMenuMess702, argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Input Type.
	 */
	sprintf (inputType, "%-1.1s", argv [1]);

	if (!BY_INMR && !BY_CUMR && !BY_SUMR)
	{
		print_at (0, 0, mlMenuMess702, argv [0]);
		return (EXIT_FAILURE);
	}

	if (BY_SUMR)
	{
		print_at (0, 0, mlMenuMess057);

		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*--------------------------
	| Set up the Quantity Mask |
	--------------------------*/
	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dfltQty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dfltQty, sptr);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	if (BY_INMR)
	{
		for (i = label ("br_no"); i <= label ("discpc"); i++)
			vars [i].scn = 2;

		for (i = label ("class"); i <= label ("dfltRtg"); i++)
			vars [i].scn = 1;
	}
	if (BY_CUMR)
	{
		for (i = label ("class"); i <= label ("dfltRtg"); i++)
			vars [i].scn = 2;

		for (i = label ("br_no"); i <= label ("roydesc"); i++)
			vars [i].scn = 1;
	}

	init_scr ();		
	set_tty (); 
	_set_masks ("psl_default.s");
	init_vars (1);

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envGst = 0;
	else
		envGst = (*sptr == 'Y' || *sptr == 'y');

	/*---------------------------
	| Adjustments based on GST. |
	---------------------------*/
	if (envGst)
	{
		sprintf (envGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
		sprintf (gstPrompt, "%-3.3s %%           :", envGstTaxName);
		FLD ("taxp") = ND;
		FLD ("tax_amt") = ND;
	}
	else
	{
		sprintf (envGstTaxName, "%-3.3s", "Tax");
		sprintf (gstPrompt, "%-3.3s %%           :", envGstTaxName);
		FLD ("gst") = ND;
	}

	/*-------------------------
	| Set up Default for Gst. |
	-------------------------*/
	sprintf (gstPercent, "%5.2f", (envGstInclusive) ? 0.00 : comm_rec.gst_rate);

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	GL_SetMask (GlFormat);

	/*---------------------------------- 
	| Beginning of input control loop. |
	----------------------------------*/
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	newRecord 	= FALSE;
	search_ok 	= TRUE;
	if (BY_INMR)
	{
		FLD ("cnv_fact") = NO;
		strcpy (inmd_rec.co_no, comm_rec.co_no);
		newRecord = find_rec (inmd, &inmd_rec, COMPARISON, "w");
		if (newRecord)
		{
			/*------------------------------
			| Enter screen 1 linear input. |
			------------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
			{
				shutdown_prog ();
				return (EXIT_SUCCESS);
			}
		}
		else
		{
			if (strcmp (inmd_rec.licence, "  "))			
			{
				strcpy (polh_rec.co_no, comm_rec.co_no);
				strcpy (polh_rec.est_no, comm_rec.est_no);  
				strcpy (polh_rec.lic_cate, inmd_rec.licence);
				strcpy (polh_rec.lic_no, "          ");
				cc = find_rec (polh, &polh_rec, GTEQ, "r");
				if (cc) 
				{
					strcpy (polh_rec.type, " ");
				}
			}
			
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code, inmd_rec.duty);
			cc = find_rec (podt, &podt_rec, COMPARISON, "r");
			if (cc)
		 		strcpy (podt_rec.description, " ");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&inmd_rec, sizeof (inmd_rec));
		}
	}
	else
	{
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}
	}

	if (BY_CUMR && !newRecord)
		FindMisc ();

	if (BY_INMR && !newRecord)
	{
		heading (1);
		InmdMisc ();
	}

	heading (1);
	scn_display (1);
	edit (1); 

	if (!restart)
		Update ();

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

	abc_alias (inum2, inum);
	abc_alias (inum3, inum);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumd,  cumd_list, CUMD_NO_FIELDS, "cumd_id_no");
	open_rec (ingd,  ingd_list, INGD_NO_FIELDS, "ingd_id_no");
	open_rec (inmd,  inmd_list, INMD_NO_FIELDS, "inmd_co_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_id_no");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum3, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (polh,  polh_list, POLH_NO_FIELDS, "polh_id_no");
	open_rec (exdf,  exdf_list, EXDF_NO_FIELDS, "exdf_id_no");
	open_rec (dbry,  dbry_list, DBRY_NO_FIELDS, "dbry_id_no");
	OpenGlmr ();
	/*
	 * Open audit file.
	 */
	if (BY_INMR)
		OpenAuditFile ("InventoryDefaultMaintenance.txt");
	if (BY_CUMR)
		OpenAuditFile ("CustomerDefaultMaintenance.txt");
	if (BY_SUMR)
		OpenAuditFile ("SupplierDefaultMaintenance.txt");

}	

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ingd);
	abc_fclose (inmd);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (inum3);
	abc_fclose (cumd);
	abc_fclose (esmr);
	abc_fclose (podt);
	abc_fclose (polh);
	abc_fclose (exdf);
	abc_fclose (dbry);
	abc_dbclose (data);
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	GL_Close ();
}

int
spec_valid (
 int field)
{
	int	i;
	int	temp = 0;
	int	val_pterms;

	/*-----------------
	| Validate Class. |
	-----------------*/
	if (LCHECK ("class"))
	{
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("source"))
	{
		if (SRCH_KEY)
		{
			SrchSource ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (source [i]._source);i++)
		{
			if (!strcmp (source [i]._source, inmd_rec.source))
			{
				sprintf (local_rec.src_desc, "%-26.26s", source [i]._sdesc);

				DSP_FLD ("src_desc");

				return (EXIT_SUCCESS);
			}
		}
		
		errmess (ML (mlMenuMess212));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*----------------
	| Validate Duty. |
	----------------*/
	if (LCHECK ("duty"))
	{
		if (SRCH_KEY)
		{
			SrchPodt (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inmd_rec.duty, "  "))
			return (EXIT_SUCCESS);

		strcpy (podt_rec.co_no, comm_rec.co_no);
		strcpy (podt_rec.code, inmd_rec.duty);
		cc = find_rec (podt, &podt_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess124));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("d_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Licence. |
	-------------------*/
	if (LCHECK ("licence"))
	{
		if (SRCH_KEY)
		{
			SrchPolh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inmd_rec.licence, "  "))
			return (EXIT_SUCCESS);

		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);  
		strcpy (polh_rec.lic_cate, inmd_rec.licence);
		strcpy (polh_rec.lic_no, "          ");
		cc = find_rec (polh, &polh_rec, GTEQ, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess154));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (polh_rec.lic_cate, inmd_rec.licence))
		{
			errmess (ML (mlStdMess154));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("l_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------
	| Validate GST |
	--------------*/
	if (LCHECK ("gst"))
	{
		if (F_HIDE (label ("gst")))
	  		inmd_rec.gst_pc = 0.00;
	
		if (envGstInclusive)
		{
			if (inmd_rec.gst_pc != 0.00)
			{
				sprintf (err_str, ML (mlMenuMess214), envGstTaxName, envGstTaxName);
				errmess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("std_uom")) 
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}
		sprintf (inum2_rec.uom, "%-4.4s", temp_str);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inmd_rec.std_uom = inum2_rec.hhum_hash;

		CalcConversion ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		DSP_FLD ("std_uom");
		DSP_FLD ("cnv_fact");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		sprintf (inum2_rec.uom, "%-4.4s", temp_str);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inmd_rec.alt_uom = inum2_rec.hhum_hash;

		CalcConversion ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		DSP_FLD ("alt_uom");
		DSP_FLD ("cnv_fact");

		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------------------
	| Validate Establishment number (only if entered) i.e. > 0 |
	----------------------------------------------------------*/
	if (LCHECK ("br_no"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!dflt_used)
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, cumd_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (cc)
			{
				errmess (ML (mlStdMess073));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
			strcpy (cumd_rec.est_no, esmr_rec.est_no);
		}
		strcpy (cumd_rec.co_no, comm_rec.co_no);
		newRecord = find_rec (cumd, &cumd_rec, COMPARISON, "w");
		if (!newRecord)
		{
			entry_exit = 1;
			strcpy (dbry_rec.co_no, comm_rec.co_no);
			strcpy (dbry_rec.cr_type, cumd_rec.roy_type);
			cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
			if (cc)
				sprintf (dbry_rec.desc, "%-26.26s%14.14s", 
						"Royalty Class not on file.", 
						" ");
			DSP_FLD ("roydesc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&cumd_rec, sizeof (cumd_rec));
		}
		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (cumd_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			strcpy (glmrRec.desc, "                         ");

		DSP_FLD ("gl_acc_desc");
		
		return (EXIT_SUCCESS);
	}
			
	/*----------------------
	| Validate price type. |
	----------------------*/
	if (LCHECK ("pri_type"))
	{
		PriceDesc ();
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate account type. |
	------------------------*/
	if (LCHECK ("acctype"))
	{
		if (cumd_rec.acc_type [0] == 'B')
			strcpy (local_rec.bf_dbt, ML ("(Balance B/F customer)"));
		else
			strcpy (local_rec.bf_dbt, ML ("(Open item customer)"));

		DSP_FLD ("acc_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate statment type. |
	-------------------------*/
	if (LCHECK ("stmt_type"))
	{
		if (cumd_rec.stmt_type [0] == 'B')
			strcpy (local_rec.bf_stm, ML ("(Balance B/F statement)"));
		else
			strcpy (local_rec.bf_stm, ML ("(Open item statement)"));

		DSP_FLD ("stm_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate statment flag. |
	-------------------------*/
	if (LCHECK ("stmt_flg"))
	{
		if (cumd_rec.stmnt_flg [0] == 'y')
			strcpy (cumd_rec.stmnt_flg, "Y");
		if (cumd_rec.stmnt_flg [0] == 'n')
			strcpy (cumd_rec.stmnt_flg, "N");

		if (cumd_rec.stmnt_flg [0] == 'Y')
			strcpy (local_rec.stmnt_flg_desc, "Yes");
		else
			strcpy (local_rec.stmnt_flg_desc, "No ");

		DSP_FLD ("stmnt_flg_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Credit Period . |
	--------------------------*/
	if (LCHECK ("crd_prd"))
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
		if (cumd_rec.crd_prd [2] >= 'A')
		{
			temp = atoi (cumd_rec.crd_prd);
			if (temp < 1 || temp > 30) 
			{ 
				/*errmess ("Invalid Day of Month");*/
				errmess (ML (mlStdMess189));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else   
			{	
				sprintf (cumd_rec.crd_prd, "%02d%c", 
						temp, cumd_rec.crd_prd [2]);
				return (EXIT_SUCCESS);
			}
		}
		/*------------------------------------
		| Check for straight numeric input . |
		------------------------------------*/
		else 
		{
			temp = atoi (cumd_rec.crd_prd);
			if (temp < 0 || temp > 999) 
			{
				/*errmess ("Invalid entry Must Be 0 - 999.");*/
				errmess (ML (mlStdMess182));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cumd_rec.crd_prd, 
				p_terms [i]._pcode, strlen (p_terms [i]._pcode)))
			{
				sprintf (cumd_rec.crd_prd, "%-3.3s", 
							p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			/*sprintf (err_str, "%s not valid Payment Terms.", cumd_rec.crd_prd);*/
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Control Account. |
	---------------------------*/
	if (LCHECK ("gl_acc"))
	{
		if (SRCH_KEY) 
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		
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
				" ", 
				" "
			);
			strcpy (cumd_rec.gl_ctrl_acct, glmrRec.acc_no); 
		}


		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (cumd_rec.gl_ctrl_acct, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
		    /*print_err ("Account %s is not on file.", glmrRec.acc_no);*/
		    print_err (ML (mlStdMess186));
		    return (EXIT_FAILURE);
		}
				
		if (Check_class ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_acc_desc");
		
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
		strcpy (exdf_rec.co_no, comm_rec.co_no);
		strcpy (exdf_rec.code, cumd_rec.disc_code);
		cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");
		if (cc)
		{
			/*errmess ("Discount Code is not on file.");*/
			errmess (ML (mlStdMess188));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("discpc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Royalty code and allow search. |
	-----------------------------------------*/
	if (LCHECK ("roycode"))
	{
		if (dflt_used)
		{
			strcpy (dbry_rec.co_no, comm_rec.co_no);
			strcpy (dbry_rec.cr_type, "  ");
			cc = find_rec (dbry, &dbry_rec, GTEQ, "r");
			if (!cc && !strcmp (dbry_rec.co_no, comm_rec.co_no))
			      	strcpy (cumd_rec.roy_type, dbry_rec.cr_type);
		}

		if (SRCH_KEY)
		{
			SrchDbry (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (dbry_rec.co_no, comm_rec.co_no);
		strcpy (dbry_rec.cr_type, cumd_rec.roy_type);
		cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
		if (cc)
		{
			/*errmess ("Royalty Class not on file.");*/
			errmess (ML (mlStdMess187));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("roydesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------------------
| Calcualte or allow entry of conversion factor. |
------------------------------------------------*/
int
CalcConversion (
 void)
{
	number	cnv_fct;
	number	std_cnv_fct;
	number	alt_cnv_fct;

	cc = find_hash (inum3, &inum2_rec, COMPARISON, "r", inmd_rec.std_uom);
	if (cc)
	{
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	cc = find_hash (inum3, &inum3_rec, COMPARISON, "r", inmd_rec.alt_uom);
	if (cc)
	{
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	if (strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
	{
		if (FLD ("cnv_fact") != ND)
			FLD ("cnv_fact") = YES;
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum2_rec.cnv_fct);
	NumFlt (&alt_cnv_fct, inum3_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| std uom cnv_fct / alt uom cnv_fct = conversion factor    |
	----------------------------------------------------------*/
	NumDiv (&std_cnv_fct, &alt_cnv_fct, &cnv_fct);

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	local_rec.cnv_fact = NumToFlt (&cnv_fct);

	if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
		FLD ("cnv_fact") = NA;

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
 void)
{
	long	numbers [7];

	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (local_rec.cnv_fact > numbers [inmd_rec.dec_pt])
	{
		sprintf (err_str, 
			ML (mlMenuMess215), 
			BELL, 
			local_rec.alt_uom, 
			inmd_rec.dec_pt);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

/*=======================================================================
| Routine to add Customer description of price type to price type field |
=======================================================================*/
void
PriceDesc (
 void)
{
	int	junk;

	junk = atoi (local_rec.pri_show);

	sprintf (local_rec.pri_type_desc, "%s", GetPriceDesc (junk));
	DSP_FLD ("pri_type_desc");
}

char	*
GetPriceDesc (
	int		priceNumber)
{

	switch (priceNumber)
	{
		case	1:
			return (comm_rec.price1_desc);
		break;

		case	2:
			return (comm_rec.price2_desc);
		break;

		case	3:
			return (comm_rec.price3_desc);
		break;

		case	4:
			return (comm_rec.price4_desc);
		break;

		case	5:
			return (comm_rec.price5_desc);
		break;

		case	6:
			return (comm_rec.price6_desc);
		break;

		case	7:
			return (comm_rec.price7_desc);
		break;

		case	8:
			return (comm_rec.price8_desc);
		break;

		case	9:
			return (comm_rec.price9_desc);
		break;

		default:
			return ("??????????");
	}
}
/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchPay (
 void)
{
	int	i = 0;
	_work_open (3, 0, 40);
	save_rec ("#Cde", "#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode, p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}
/*==========================================
| Search routine for Discount Master File. |
==========================================*/
void
SrchExdf (
 char *	key_val)
{
	_work_open (2, 0, 40);
	strcpy (exdf_rec.co_no, comm_rec.co_no);
	strcpy (exdf_rec.code, key_val);
	save_rec ("#No.", "#Discount %");
	cc = find_rec (exdf, &exdf_rec, GTEQ, "r");
	while (!cc && !strncmp (exdf_rec.code, key_val, strlen (key_val)) && 
	              !strcmp (exdf_rec.co_no, comm_rec.co_no))
	{
		sprintf (err_str, "Discount Percent %5.2f", exdf_rec.disc_pc);
		cc = save_rec (exdf_rec.code, err_str);
		if (cc)
			break;
		cc = find_rec (exdf, &exdf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exdf_rec.co_no, comm_rec.co_no);
	strcpy (exdf_rec.code, temp_str);
	cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");
}

void
FindMisc (
 void)
{
	strcpy (local_rec.pri_show, cumd_rec.price_type);
	PriceDesc ();

	if (cumd_rec.acc_type [0] == 'B')
		strcpy (local_rec.bf_dbt, "(Balance B/F customer)");
	else
		strcpy (local_rec.bf_dbt, "(Open item customer)");

	if (cumd_rec.stmt_type [0] == 'B')
		strcpy (local_rec.bf_stm, "(Balance B/F statement)");
	else
		strcpy (local_rec.bf_stm, "(Open item statement)");

	if (cumd_rec.stmnt_flg [0] == 'Y' || cumd_rec.stmnt_flg [0] == 'y')
		strcpy (local_rec.stmnt_flg_desc, "Yes");
	else
		strcpy (local_rec.stmnt_flg_desc, "No ");

	strcpy (exdf_rec.co_no, comm_rec.co_no);
	strcpy (exdf_rec.code, cumd_rec.disc_code);
	cc = find_rec (exdf, &exdf_rec, COMPARISON, "r");
	if (cc)
		strcpy (cumd_rec.disc_code, "A");
}

void
InmdMisc (
 void)
{
	int	i;

	/*----------------
	| Look up source |
	----------------*/
	for (i = 0;strlen (source [i]._source);i++)
	{
		if (!strcmp (source [i]._source, inmd_rec.source))
		{
			sprintf (local_rec.src_desc, 
				"%-26.26s", 
		           	source [i]._sdesc);
			break;
		}
	}

	/*-----------------
	| Look up grading |
	-----------------*/
	if (envSkGrades)
	{
		strcpy (ingd_rec.co_no, comm_rec.co_no);
		sprintf (ingd_rec.grade, "%-1.1s", inmd_rec.grade);
		cc = find_rec (ingd, &ingd_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ingd, "DBFIND");

		local_rec.grade_pc = ingd_rec.writedown;
	}

	/*---------------
	| Look up UOM's |
	---------------*/
	cc = find_hash (inum3, &inum2_rec, COMPARISON, "r", inmd_rec.std_uom);
	if (cc)
		strcpy (local_rec.std_uom, "    ");
	else
		sprintf (local_rec.std_uom, "%-4.4s", inum2_rec.uom);

	cc = find_hash (inum3, &inum3_rec, COMPARISON, "r", inmd_rec.alt_uom);
	if (cc)
		strcpy (local_rec.alt_uom, "    ");
	else
		sprintf (local_rec.alt_uom, "%-4.4s", inum3_rec.uom);

	if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
		FLD ("cnv_fact") = NA;
	else
	{
		if (FLD ("cnv_fact") != NA)
			FLD ("cnv_fact") = NI;
	}
	local_rec.cnv_fact = inmd_rec.uom_cfactor;
}

int
Check_class (void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
	    print_err (ML (mlStdMess025));
		sleep (sleepTime);
	    return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update All inventory files.
 */
int
Update (void)
{
	clear ();

	/*
	 * Add inventory default file. 
	 */
	if (BY_INMR)
	{
		strcpy (inmd_rec.std_duom, local_rec.std_uom);
		strcpy (inmd_rec.alt_duom, local_rec.alt_uom);
		inmd_rec.uom_cfactor = local_rec.cnv_fact;
		if (newRecord)
		{
			cc = abc_add (inmd, &inmd_rec);
			if (cc) 
				file_err (cc, inmd, "DBADD"); 
		}
		else 
		{
			/*
			 * Update changes audit record.
			 */
			 AuditFileAdd (ML ("Item Default"), &inmd_rec, inmd_list, INMD_NO_FIELDS);
			cc = abc_update (inmd, &inmd_rec);
			if (cc) 
				file_err (cc, inmd, "DBUPDATE"); 
		}
	}
	/*
	 * Add customer default file.
	 */
	if (BY_CUMR)
	{
		sprintf (cumd_rec.price_type, "%-1.1s", local_rec.pri_show);
		strcpy (cumd_rec.gl_ctrl_acct, glmrRec.acc_no);
		if (newRecord)
		{
			strcpy (cumd_rec.sur_flag, "N");
			strcpy (cumd_rec.stop_credit, "N");
			strcpy (cumd_rec.ch_to_ho_flg, "N");
			strcpy (cumd_rec.stmnt_flg, "Y");
			strcpy (cumd_rec.stat_flag, "0");
			strcpy (cumd_rec.stmnt_flg, "Y");
			cc = abc_add (cumd, &cumd_rec);
			if (cc) 
				file_err (cc, cumd, "DBADD"); 
		}
		else 
		{
			/*
			 * Update changes audit record.
			 */
			 AuditFileAdd (ML ("Customer Default"), &cumd_rec, cumd_list, CUMD_NO_FIELDS);
			cc = abc_update (cumd, &cumd_rec);
			if (cc) 
				file_err (cc, cumd, "DBUPDATE");
		}
	}
	return (EXIT_SUCCESS);
}

int
SrchInum (
	char	*key_val)
{
	_work_open (4, 0, 40);
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

	return (EXIT_SUCCESS);
}

void
SrchSource (void)
{
	int	i;
	_work_open (3, 0, 40);
	save_rec ("#Sr", "#Description");
	for (i = 0; strlen (source [i]._source); i++)
	{
		cc = save_rec (source [i]._source, source [i]._sdesc);
		if (cc)
			break;
	}
	disp_srch ();
	work_close ();
}

int
SrchIngd (
	char	*key_val)
{
	char	tmp_desc [71];

	_work_open (1, 0, 40);
	save_rec ("#N", "#Grade Writedown ");
	strcpy (ingd_rec.co_no, comm_rec.co_no);
	sprintf (ingd_rec.grade, " ");
	cc = find_rec (ingd, &ingd_rec, GTEQ, "r");
	while (!cc && !strcmp (ingd_rec.co_no, comm_rec.co_no))
	{
		sprintf (tmp_desc, "%5.2f", ingd_rec.writedown);
		cc = save_rec (ingd_rec.grade, tmp_desc);
		if (cc)
			break;

		cc = find_rec (ingd, &ingd_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();

	return (EXIT_SUCCESS);
}

/*
 * Search for Duty master file.
 */
void
SrchPodt (
	char	*key_val)
{
	_work_open (2, 0, 40);
	strcpy (podt_rec.co_no, comm_rec.co_no);
	sprintf (podt_rec.code, "%-2.2s", key_val);
	save_rec ("#No", "#Duty Description");
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

/*
 * Search for Licence master file.
 */
void
SrchPolh (
	char	*key_val)
{
	char	lic_cat [3];

	_work_open (10, 0, 40);
	strcpy (lic_cat, "  ");
	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	sprintf (polh_rec.lic_cate, "%-2.2s", key_val);
	strcpy (polh_rec.lic_no, "          ");
	save_rec ("#No", "#Licence Type");
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
	
/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input.                      
 */
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	if (BY_CUMR)
		rv_pr (ML (mlMenuMess058), 24, 0, 1);

	if (BY_INMR)
		rv_pr (ML (mlMenuMess059), 24, 0, 1);

	if (BY_CUMR)
		pr_box_lines (1);
	
	if (BY_INMR)
		pr_box_lines (2);
	
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
SrchDbry (
	char	*key_val)
{
	_work_open (3, 0, 40);
	save_rec ("#No.", "#Royalty Description ");
	strcpy (dbry_rec.co_no, comm_rec.co_no);
	strcpy (dbry_rec.cr_type, key_val);
	cc = find_rec (dbry, &dbry_rec, GTEQ, "r");
	while (!cc && !strcmp (dbry_rec.co_no, comm_rec.co_no) && 
		      !strncmp (dbry_rec.cr_type, key_val, strlen (key_val)))
	{
		cc = save_rec (dbry_rec.cr_type, dbry_rec.desc);
		if (cc)
			break;
		cc = find_rec (dbry, &dbry_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (dbry_rec.co_no, comm_rec.co_no);
	sprintf (dbry_rec.cr_type, "%-3.3s", temp_str);
	cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dbry, "DBFIND");
}

/*
 * Search for esmr.
 */
void
SrchEsmr (
 char *	key_val)
{
	_work_open (2, 0, 40);
	save_rec ("#No.", "#Branch Name.");
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (comm_rec.co_no, esmr_rec.co_no) &&
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

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

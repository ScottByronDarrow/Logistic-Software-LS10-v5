/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_misc_ir.c,v 5.22 2002/07/24 08:39:14 scott Exp $
|  Program Name  : (sk_misc_ir.c   )                                  |
|  Program Desc  : (Stock Receipts/ Purchases and issues.         )   |
|                  (Combination of sk_iss & sk_rec                )   |
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sk_misc_ir.c,v $
| Revision 5.22  2002/07/24 08:39:14  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.21  2002/07/08 08:27:02  scott
| S/C 004049 - See S/C
|
| Revision 5.20  2002/06/20 07:11:03  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.19  2002/06/03 03:50:22  scott
| Updated for questionable if
|
| Revision 5.18  2002/02/27 03:42:25  scott
| Updated as file open for vehicle wrong.
|
| Revision 5.17  2002/02/26 05:28:16  scott
| Updated as money mask wrong
|
| Revision 5.16  2002/02/26 04:29:05  scott
| Updated to fix field offset.
|
| Revision 5.15  2002/02/07 10:32:05  cha
| S/C 795. Updated to display and allow input Vehicle field when
| PO_3PL_SYSTEM is 1 and is in ISSUE mode.
|
| Revision 5.14  2002/01/29 02:57:15  robert
| Updated to fix memory dump on LS10-GUI
|
| Revision 5.13  2002/01/22 08:41:27  scott
| Updated as tab_other in wrong place.
|
| Revision 5.12  2002/01/09 01:16:46  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.11  2001/11/22 01:20:58  scott
| Updated for error
|
| Revision 5.10  2001/11/22 01:14:55  scott
| Updated to re-lineup reports as movement transaction reflected as 10 instead of 15
|
| Revision 5.9  2001/11/09 04:57:31  scott
| Updated to ensure Batch control is set if number plates active.
|
| Revision 5.8  2001/10/09 23:06:51  scott
| Updated for returns
|
| Revision 5.7  2001/09/19 00:43:31  scott
| Updated from scotts machine.
|
| Revision 5.6  2001/08/23 12:08:04  scott
| Updated from scotts machine
|
| Revision 5.5  2001/08/20 23:20:56  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_misc_ir.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_misc_ir/sk_misc_ir.c,v 5.22 2002/07/24 08:39:14 scott Exp $";

#define MAXSCNS  	3 
#define MAXWIDTH 	250
#define MAXLINES 	100
#define	TXT_REQD

#include <pslscr.h>
#include <GlUtils.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <ml_sk_mess.h>

#define	SR		store [line_cnt]
#define	LSR		store [lcount [2]]

#define		UPD_GL 		(GlPostingFlag [0] == 'G')
#define		JOURNAL 	(GlPostingFlag [0] == 'J')
#define 	RECEIPT		(programRunFlag [0] == 'R')
#define 	PURCHASE	(programRunFlag [0] == 'P')
#define 	OL_PUR		(programRunFlag [0] == 'O')
#define 	ADJUST		(programRunFlag [0] == 'A')
#define 	ISSUE		(programRunFlag [0] == 'I')
#define		SERIAL		(SR.serFlag [0] == 'Y') 
#define		ADJ_ISS		(ADJUST && local_rec.workQty < 0.00)
#define		ADJ_REC		(ADJUST && local_rec.workQty > 0.00)
#define		POS_PUR		((OL_PUR || PURCHASE) && local_rec.workQty > 0.00)
#define 	I_VALUE(a)	(store2 [a].values [0])
#define 	D_VALUE(a)	(store2 [a].values [1])
#define 	C_VALUE(a)	(store2 [a].values [2])
#define		PRN_FIELD(x)	(vars [label(x)].prmpt)

#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct esmrRecord	esmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct inwuRecord	inwu_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct qcmrRecord	qcmr_rec;
struct qchrRecord	qchr_rec;
struct inirRecord	inir_rec;
struct ffdmRecord	ffdm_rec;
struct sknhRecord	sknh_rec;
struct sknhRecord	sknh2_rec;
struct skndRecord	sknd_rec;
struct trveRecord	trve_rec;

	/*---------------------------
	| Special fields and flags. |
	---------------------------*/

	FILE	*pp;

	int		printerNumber	= 1, 
			LotSelectFlag	= 0, 
			GLSCREEN		= FALSE, 
			plateCalled		= 0, 
			newNumberPlate	= TRUE;

	char	GlPostingFlag [2], 
			programRunFlag [2], 
			lowerAdjustment [17];

	char	*currentUser;
	char	*serialSpace 	= "                         ";
	char	*fortySpace 	= "                                        ";


	long	issueHhmrHash		= 0L, 
			receiptHhmrHash 	= 0L;
	char	issueAcc		 [MAXLEVEL + 1], 
			receiptAcc		 [MAXLEVEL + 1];

	char	pWorkStr [300];
	char	localCurrency [4];

	double	batchTotal = 0;
	double	JnlTotal = 0;
	float	qtyTotal = 0;

	struct	storeRec {
		char	cost_flag [2];
		char	serFlag [2];
		char	lot_ctrl [2];
		long	hhwhHash;
		long	hhumHash;
		long	hhbrHash;
		long	hhsiHash;
		long	hhccHash;
		char	_UOM [5];
		char	ser_no [26];
		char	itemDesc [41];
		char	dbt_acc [MAXLEVEL + 1];
		long	dbt_hash;
		char	crd_acc [MAXLEVEL + 1];
		long	crd_hash;
		int		_commitRef;
		int		_deCommitRef;
		long	_origHhbr;
		float	_origOrdQty;
		float	convFct;
		double	_cost;
		char	qcCentre [5];
		char	qcReqd [2];
	} store [MAXLINES];

#include	<MoveRec.h>

char 	*data	= "data", 
		*incc2	= "incc2", 
		*inum2	= "inum2";

	char	*scn_desc [] = 
			{
				"HEADER SCREEN.", 
				"ITEM SCREEN.", 
				"GENERAL LEDGER SCREEN."
			};

int		envQcApply 			= FALSE, 
		envSkMiscGlRt 		= FALSE, 
		envAllowZeroCost 	= FALSE, 
		envSkQcAvl 			= FALSE, 
		envSkQcPass			= FALSE, 
		envSkIrTypes 		= FALSE, 
		envVarThreePl 		= FALSE, 
		envSkGrinNoPlate 	= FALSE, 
		envSkSerialOk 		= FALSE, 
		firstTime 			= TRUE, 
		journalProof 		= TRUE;

#include	<Costing.h>

extern	int	TruePosition;

/*
 * Local & Screen Structures
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	previousItem [17];
	char 	journalRef [sizeof glwkRec.user_ref];
	char 	NewRef [sizeof glwkRec.user_ref];
	char 	OldRef [sizeof glwkRec.user_ref];
	char	batchString [8];
	long 	journalDate;
	double	workQty;
	double	workCost;
	char	item_no [17];
	long	hhwhHash;
	long	receiptDate;
	long	expDate;
	char	c_exp_date [11];
	char	defaultRecDate [11];
	char	defaultDate [11];
	char	ser_no [26];
	char	item_desc [41];
	char	lot_ctrl [2];
	int		dec_pt;
	char	dflt_ser_no [26];
	char	dflt_qty [15];
	char	rep_qty [30];
	char	qcCentre [5];
	char	qcReqd [2];
	long	hhbrHash;
	char	UOM [5];

	char	glNarrative [21];
	char	prevGlNarrative [21];
	char	dc2_flag [2];
	char	ca_desc [26];
	char	gl_period [3];
	char	gl_acc_no [MAXLEVEL + 1];
	char	glUserRef [sizeof glwkRec.user_ref];
	char	prevGlUserRef [sizeof glwkRec.user_ref];
	double	loc_gl_amt;
	char	gl_loc_curr [4];
	char	location [11];
	char	LL [2];
	char	cusOrdRef [sizeof sknd_rec.cus_ord_ref];					
	char	previousCusOrdRef [sizeof sknd_rec.cus_ord_ref];
	char	numberPlate [sizeof sknh_rec.plate_no];
	char	quantityPrompt [21];
	float	packageQty;		/* Package quantity						*/
	float	totalChargeWgt;	/* Total charge weight					*/
	float	totalGrossWgt;	/* total gross weight					*/
	float	totalCBM;		/* Total Cubic metres.					*/

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "ref", 	 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Journal reference  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.journalRef}, 
	{1, LIN, "jdate", 	 4, 60, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.defaultRecDate, "Journal date       ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.journalDate}, 
	{1, LIN, "ir_type", 	 5, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Issue/Receipt Type      ", "Enter Type [SEARCH]", 
		 NO, NO, JUSTLEFT, "", "", inir_rec.ir_type}, 
	{1, LIN, "ir_desc", 	 5, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", inir_rec.ir_desc}, 
	{1, LIN, "vehicle", 	 6, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "  ", "Vehicle.           ", "Enter Vehicle No. or [SEARCH]", 
		ND, NO,  JUSTLEFT, "", "", trve_rec.ref}, 
	{1, LIN, "vehicleDesc", 	 6, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "  ", "-", " ", 
		ND, NO,  JUSTLEFT, "", "", trve_rec.desc}, 
	{1, LIN, "numberPlate", 	 7, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Number Plate No.   ", "NOTE Default for inwards goods will use next Goods Receipt number.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.numberPlate}, 
	{2, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "  Item number.  ", " ", 
		NE, NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{2, TAB, "itemDesc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "        Item Description.        ", " ", 
		 ND, NO,  JUSTLEFT, "", "", local_rec.item_desc}, 
	{2, TAB, "qty", 	 0, 0, DOUBLETYPE, 
		local_rec.dflt_qty, "          ", 
		" ", "", local_rec.quantityPrompt, " ", 
		YES, NO, JUSTRIGHT, lowerAdjustment, "9999999.99", (char *) &local_rec.workQty}, 
	{2, TAB, "UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", "UOM.", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.UOM}, 
	{2, TAB, "packageQty", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", " ", "Pack. Qty", "Input Package quantity.", 
		ND, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.packageQty}, 
	{2, TAB, "totalChargeWgt", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Chrge Wgt", "Input total charge weight.", 
		ND, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.totalChargeWgt}, 
	{2, TAB, "totalGrossWgt", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Gross Wgt", " ", 
		ND, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.totalGrossWgt}, 
	{2, TAB, "totalCBM", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Tot. CBM.", " ", 
		ND, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.totalCBM}, 
	{2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{2, TAB, "location", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", " ", " Location ", "Enter the location. [SEARCH] [Any other search key will perform lookup on location master.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.location}, 
	{2, TAB, "cusOrdRef", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", local_rec.previousCusOrdRef, " Customer Order Ref ", "", 
		ND, NO,  JUSTLEFT, "", "", local_rec.cusOrdRef}, 
	{2, TAB, "cost", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", " Est Cost ", " ", 
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *) &local_rec.workCost}, 
	{2, TAB, "receiptDate", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.defaultDate, "   Date.  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.receiptDate}, 
	{2, TAB, "qcCentre", 	 0, 0, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Cntr", " ", 
		 NO, NO,  JUSTLEFT, "", "", local_rec.qcCentre}, 
	{2, TAB, "qcReqd", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "", "", "", 
		 ND, NO,  JUSTLEFT, "", "", local_rec.qcReqd}, 
	{2, TAB, "serialNumber", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "     Serial Number       ", "Please note that only 1st 10 character are stored in stock transaction file. ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.ser_no}, 
	{2, TAB, "dec_pt", 	 0, 0, INTTYPE, 
		"N", "          ", 
		"", "", "", "", 
		 ND, NO,  JUSTRIGHT, "", "", (char *)& local_rec.dec_pt}, 
	{2, TAB, "hhbrHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", "", 
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhbrHash}, 
	{3, TAB, "glacct", 	MAXLINES, 0, CHARTYPE, 
		GlMask, "          ", 
		"0", " ", GlDesc, "Enter account or [SEARCH] ", 
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.gl_acc_no}, 
	{3, TAB, "gl_desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "   Account Description   ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.ca_desc}, 
	{3, TAB, "dc2", 	 0, 1, CHARTYPE, 
		"U", "          ", 
		" ", "", "D/C", "Must Be D(ebit) or C(redit). ", 
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc2_flag}, 
	{3, TAB, "loc_gl_amt", 	 0, 0, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Local Currency", " ", 
		YES, NO, JUSTRIGHT, "0", "999999999", (char *)&local_rec.loc_gl_amt}, 
	{3, TAB, "loc_curr", 	 0, 0, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Cur", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.gl_loc_curr}, 
	{3, TAB, "glNarrative", 	 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", local_rec.prevGlNarrative, "      Narrative         ", " ", 
		 NO, NO,  JUSTLEFT, "", "", local_rec.glNarrative}, 
	{3, TAB, "glUserRef", 	 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", local_rec.journalRef, " User Reference. ", " ", 
		 NO, NO,  JUSTLEFT, "", "", local_rec.glUserRef}, 
	{3, TAB, "hhglHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", "", 
		 ND, NO,  JUSTRIGHT, "", "", (char *) &glmrRec.hhmr_hash}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

long	CurrHHCC	=	0L;


struct store2Rec {
	double  values [3];
} store2 [MAXLINES];

char    tmpStr [2] [30];

#include	<MoveAdd.h>
#include	<ser_value.h>
#include	<LocHeader.h>

/*
 * Function Declarations
 */
double	GetIssueCost		(void);
double	ToLclPrice 			(double);
double	ToStdPrice 			(double);
float	GetBrClosing		(long);
float	ReCalcAvail			(void);
float	ToLclUom 			(float);
float	ToStdUom	 		(float);
int		AddIncc				(long, long);
int		CheckClass			(void);
int		CheckDupInsf		(char *, long, int);
int		FindIncc			(void);
int		GetGlAccounts		(char *, int);
int		MoneyZero			(double);
int		ValidQuantity		(double, int);
int		ValidateDate		(long);
int		heading				(int);
int		spec_valid			(int);
void	AddFfdm				(float, long, long);
void	AddInsf				(void);
void	AddInventoryGlwk	(void);
void	AddSknh				(void);
void	AddStandingGlwk		(void);
void	CloseAudit			(void);
void	CloseDB				(void);
void	GenGrnNo			(void);
void	InputSknh			(void);
void	LoadSknd 			(void);
void	LoadSknh			(void);
void	OpenAudit			(void);
void	OpenDB				(void);
void	PformatClose		(void);
void	PrintJournalTotal	(void);
void	PrintSingleLine 	(char *);
void	ProofTrans			(void);
void	ReadGlDefaults		(char *);
void	SrchInir			(char *);
void	SrchInum			(char *);
void	SrchQcmr			(char *);
void	SrchSknh 		 	(char *);
void	SrchTrve 			(char *);
void	Update				(void);
void	UpdateSknh 			(void);
void	UpdateTab			(int);
void	shutdown_prog		(void);
void	tab_other			(int);
void 	PrintDetailedInfo 	(int);
void 	PrintHeaderInfo 	(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv [])
{
	char	*sptr;
	int		i, 
			before, 
			after;

	if (argc < 4)	
	{
		print_at (0, 0, mlSkMess629, argv [0]);
		return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	GLSCREEN = TRUE;
	if (!strncmp (sptr, "sk_misc_ir", 10))
		GLSCREEN = FALSE;

	currentUser = getenv ("LOGNAME");

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);
	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

    sptr = chk_env ("ALLOW_ZERO_COST");
	envAllowZeroCost = (sptr == (char *) 0) ? 0 : atoi(sptr);

    sptr = chk_env ("SK_MISC_GL_RT");
	envSkMiscGlRt = (sptr == (char *) 0) ? 0 : atoi(sptr);

	/*
	 * QC module is active 
	 */
	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/*
	 * Whether to include QC qty in available stock. 
	 */
	envSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*
	 * Whether to allow PASS on the QC Centre. 
	 */
	envSkQcPass = (sptr = chk_env ("SK_QC_PASS")) ? atoi (sptr) : 0;

	/*
	 * Whether multiple issue / receipt types used.
	 */
	envSkIrTypes = (sptr = chk_env ("SK_IR_TYPES")) ? atoi (sptr) : 0;

	printerNumber = atoi (argv [1]);

	sptr = chk_env ("SK_SERIAL_OK");
	envSkSerialOk = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarThreePl = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for Number plates.  
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = ((sptr == (char *)0)) ? 0 : atoi (sptr);

	switch (argv [2] [0])
	{
	case	'G':
	case	'g':
		strcpy (GlPostingFlag , "G");
		break;

	case	'J':
	case	'j':
		strcpy (GlPostingFlag , "J");
		break;

	default:
		print_at (0, 0, mlSkMess629, argv [0]);
		return (EXIT_FAILURE);
	}
	switch (argv [3] [0])
	{
	case	'R':
	case	'r':
		strcpy (programRunFlag, "R");
		break;

	case	'I':
	case	'i':
		strcpy (programRunFlag, "I");
		locStatusIgnore = TRUE;
		break;

	case	'P':
	case	'p':
		strcpy (programRunFlag, "P");
		break;

	case	'O':
	case	'o':
		strcpy (programRunFlag, "O");
		break;

	case	'A':
	case	'a':
		strcpy (programRunFlag, "A");
		break;

	default:
		print_at (0, 0, mlSkMess629, argv [0]);
		return (EXIT_FAILURE);
	}
	strcpy (lowerAdjustment, "0.00");
	if (ADJUST || OL_PUR || PURCHASE)
		strcpy (lowerAdjustment, "-9999999.99");

	SETUP_SCR (vars);


	init_scr ();
	set_tty (); 
	sprintf (err_str, "%s.s", argv [0]);
	_set_masks (err_str);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
	SetSortArray (3, store2, sizeof (struct store2Rec));
#endif


	if (ISSUE)
	{
		if (FLD ("cost") != ND)
			FLD ("cost") = NA ;
	}
	FLD ("ir_type")	= ND;
	FLD ("ir_desc")	= ND;

	if (envSkIrTypes)
	{
		FLD ("ir_type")	= YES;
		FLD ("ir_desc")	= NA;
	}

	FLD ("serialNumber")	= (envSkSerialOk) ? YES : ND;
	FLD ("itemDesc") 		= (envSkSerialOk) ? ND 	: NA;

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	swide ();
	OpenDB ();

	if (envVarThreePl)
	{
		FLD ("itemDesc") 		= 	ND;
		FLD ("packageQty")		=	YES;
		FLD ("totalChargeWgt")	=	YES;
		FLD ("totalGrossWgt")	=	YES;
		FLD ("totalCBM")		=	YES;
		FLD ("cusOrdRef")		=	YES;
		FLD ("vehicle")			=	(ISSUE) ? YES : ND;
		FLD ("vehicleDesc")		=	NA;
		FLD ("receiptDate")		=	ND;
		FLD ("qcCentre")		=	ND;
		FLD ("cost")			=	NI;
		strcpy (local_rec.quantityPrompt, "Unit Quan.");
	}
	else
	{
		strcpy (local_rec.quantityPrompt, "Quantity. ");
		SCN_ROW ("numberPlate")	=	6;
	}
	FLD ("numberPlate") = (envSkGrinNoPlate) ? YES : ND;

	FLD ("LL") 			=	ND;
	FLD ("location") 	=	ND;

	if (MULT_LOC)
	{
		if (SK_BATCH_CONT || envSkGrinNoPlate)
		{
			SK_BATCH_CONT	=	TRUE;
			FLD ("LL")		=	YES;
		}
		else
			FLD ("location") =	YES;
	}
	if (ADJUST)
		FLD ("numberPlate") 	=	ND;

	FLD ("qcCentre") = envQcApply ? NA : ND;

	GL_SetMask (GlFormat);
		
	strcpy (local_rec.defaultRecDate, DateToString (comm_rec.inv_date));
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	local_rec.lsystemDate = TodaysDate ();
	strcpy (local_rec.OldRef, "000000000000000");
	strcpy (local_rec.NewRef, "000000000000000");
	strcpy (local_rec.batchString, "  N/A  ");
	strcpy (local_rec.previousCusOrdRef, "00000000000000000000");
	strcpy (local_rec.prevGlNarrative, " ");
	strcpy (local_rec.prevGlUserRef, " ");

	tab_row = 7;

	while (!prog_exit) 
	{
		entry_exit 		= FALSE;
		restart 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		search_ok 		= TRUE;
		plateCalled 	= FALSE;
		journalProof	= TRUE;
		newNumberPlate	= TRUE;
		strcpy (local_rec.batchString, "  N/A  ");

		for (i = 0; i < MAXLINES; i++)
		{
			store [i].hhwhHash 		= 0L;
			store [i].hhbrHash 		= 0L;
			store [i].hhsiHash 		= 0L;
			store [i].hhumHash 		= 0L;
			store [i].dbt_hash 		= 0L;
			store [i].crd_hash 		= 0L;
			store [i]._commitRef 	= 0;
			store [i]._deCommitRef 	= 0;
			store [i]._origHhbr 	= 0L;
			store [i]._origOrdQty 	= 0.00;
			sprintf (store [i].cost_flag, 	"%1.1s", 	" ");
			sprintf (store [i].serFlag, 	"%1.1s", 	" ");
			sprintf (store [i].lot_ctrl, 	"%1.1s", 	" ");
			sprintf (store [i].ser_no, 		"%25.25s", 	" ");
			sprintf (store [i].itemDesc, 	"%40.40s", 	" ");
			sprintf (store [i].dbt_acc, 	"%9.9s", 	" ");
			sprintf (store [i].crd_acc, 	"%9.9s", 	" ");

			I_VALUE (i)	=	0.00;
			D_VALUE (i)	=	0.00;
			C_VALUE (i)	=	0.00;
		}

		lcount[2] = 0;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (newNumberPlate)
		{
			heading (2);
			entry (2);
			if (restart)
				continue;
		}

		if (GLSCREEN)
		{
			/*
			 * Enter Screen 3 Tabular Input .
			 */
			heading (3);
	   		PrintJournalTotal ();
			entry (3);

			if (restart) 
				continue;
		}
		else
			no_edit (3);

		/*
		 * re-edit tabular if proof total incorrect.
		 */
		if (GLSCREEN)
		{
			while (journalProof)
			{
	    		PrintJournalTotal ();
				edit_all ();
				if (restart) 
					break;
	
				ProofTrans ();
			}
		}
		else
			edit_all ();

		if (restart)
			continue;

		if (!OL_PUR && firstTime)
			OpenAudit ();

		strcpy (local_rec.NewRef, local_rec.journalRef);

		firstTime = FALSE;

		Update ();

		if (strcmp (local_rec.NewRef, local_rec.OldRef))
		{
			CloseAudit ();
			batchTotal = 0.00;
			qtyTotal  = 0.00;
		}

		strcpy (local_rec.OldRef, local_rec.NewRef);

		abc_unlock (inmr);
		abc_unlock (inei);
		abc_unlock (incc);
	}
	shutdown_prog ();	
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	if (!OL_PUR && !firstTime)
		PformatClose (); 

	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;
	abc_dbopen (data);

	abc_alias (incc2, incc);
	abc_alias (inum2, inum);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenGlmr ();
	OpenGlwk ();

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (qcmr,  qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ffdm,  ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
	if (envSkGrinNoPlate)
	{
		open_rec (sknh,  sknh_list, SKNH_NO_FIELDS, "sknh_id_no");
		open_rec (sknd,  sknd_list, SKND_NO_FIELDS, "sknd_id_no");
		open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_id_no");
	}
	if (envVarThreePl)
		open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlStdMess100));
	   	sleep (sleepTime);
		clear_mess ();
	   	return;
	}

	OpenLocation (ccmr_rec.hhcc_hash);
	CurrHHCC 	=	ccmr_rec.hhcc_hash;
	if (RECEIPT || PURCHASE	|| OL_PUR || ADJUST)
		IgnoreAvailChk	=	TRUE;
	else
		strcpy (StockTake, "Y");

	LotSelectFlag	=	INP_AUTO;

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (ffdm);
	abc_fclose (inwu);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (ccmr);
	abc_fclose (glwk);
	abc_fclose (glmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (qcmr);
	abc_fclose (qchr);
	if (envSkGrinNoPlate)
	{
		abc_fclose (sknd);
		abc_fclose (sknh);
	}
	if (envVarThreePl)
		abc_fclose (trve);
	
	abc_fclose ("move");
	CloseLocation ();
	CloseCosting ();
	GL_CloseBatch (printerNumber);
	GL_Close ();
	SearchFindClose ();
	abc_dbclose (data);
}
/*
 * Read all nessasary files for defaults.
 */
void
ReadGlDefaults (
 char	*categoryNo)
{
	GL_GLI 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		comm_rec.cc_no, 
		"MIS SK REC", 
		" ", 
		categoryNo
	);
	receiptHhmrHash = glmrRec.hhmr_hash;
	strcpy (receiptAcc, glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		comm_rec.cc_no, 
		"MIS SK ISS", 
		" ", 
		categoryNo
	);
	issueHhmrHash = glmrRec.hhmr_hash;
	strcpy (issueAcc, glmrRec.acc_no);
	return;
}

int
spec_valid (
 int field)
{
	int 	i;
	int		tmp_dmy [3];
	int		TempLine;
	long	workInloHash	=	0L;
	char	workLocationType [2];

	/*
	 * Batch number.
	 */
	if (LCHECK ("ref"))
	{
		strcpy (local_rec.journalRef, zero_pad (local_rec.journalRef, 15));
		DSP_FLD ("ref");

		return (EXIT_SUCCESS);
	}

	/*
	 * Journal Date.
	 */
	if (LCHECK ("jdate"))
	{
		if (local_rec.journalDate <= 0L)
		{
		   print_mess (ML ("Zero date is not allowed."));
		   return (EXIT_FAILURE);
		}
		return (ValidateDate (local_rec.journalDate));
	}

	/*
	 * Journal Date.
	 */
	if (LCHECK ("receiptDate"))
	{
    	if (local_rec.receiptDate <= 0L)
		{
		   print_mess (ML ("Zero date is not allowed."));
		   return (EXIT_FAILURE);
	    }
		return (ValidateDate (local_rec.receiptDate));
	}

	/*
	 * Validate issue_rec type   
	 */
	if (LCHECK ("ir_type"))
	{
		if (FLD ("ir_type") == ND)
			return (EXIT_SUCCESS);

		open_rec  (inir, inir_list, INIR_NO_FIELDS, "inir_id_no");

		if (SRCH_KEY)
		{
			SrchInir (temp_str);
			abc_fclose (inir);
			return (EXIT_SUCCESS);
		}

		strcpy (inir_rec.co_no, comm_rec.co_no);
		cc = find_rec (inir, &inir_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML ("Issue/Receipt Type is not on File."));
			sleep (sleepTime);
			abc_fclose (inir);
			return (EXIT_FAILURE);
		}

		abc_selfield (glmr, "glmr_hhmr_hash");

		glmrRec.hhmr_hash	=	inir_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			abc_selfield (glmr, "glmr_id_no");
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [2] [0] != 'P')
		{
			errmess (ML ("Account is not a Posting Level Account."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		issueHhmrHash = glmrRec.hhmr_hash;
		receiptHhmrHash = glmrRec.hhmr_hash;
		sprintf (issueAcc, "%-*.*s", MAXLEVEL, MAXLEVEL, glmrRec.acc_no);
		sprintf (receiptAcc, "%-*.*s", MAXLEVEL, MAXLEVEL, glmrRec.acc_no);

		abc_selfield (glmr, "glmr_id_no");

		DSP_FLD ("ir_type");
		DSP_FLD ("ir_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Goods Receipt Number. 
	 */
	if (LCHECK ("numberPlate"))
	{
		char	*sptr;

		if (!envSkGrinNoPlate || FLD ("numberPlate") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY && ISSUE)
		{
			SrchSknh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (ISSUE && dflt_used)
		{
			strcpy (local_rec.numberPlate, " ");
			DSP_FLD ("numberPlate");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.numberPlate, "NEW NO. PLATE. ");
			DSP_FLD ("numberPlate");
			plateCalled	=	TRUE;
			memset (&sknh_rec , 0, sizeof (sknh_rec));
			InputSknh ();
			entry_exit = TRUE;
			
			return (EXIT_SUCCESS);
		}
		if (ISSUE)
		{
			LoadSknh ();
			if (newNumberPlate)
			{
				print_mess (ML (mlStdMess049));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			plateCalled	= TRUE;
			LoadSknd ();
			DSP_FLD ("vehicle");
			DSP_FLD ("vehicleDesc");
			DSP_FLD ("numberPlate");
			InputSknh ();
			return (EXIT_SUCCESS);
		}
		strcpy (err_str, local_rec.numberPlate);
		sptr = clip (err_str);

		while (strlen (sptr))
		{
			if (*sptr != ' ' && *sptr != '0')
				break;
			sptr++;
		}

		if (!strlen (sptr))
		{
			print_mess (ML (mlPoMess206));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		SuperSynonymError ();

		/*------------------------
		| Discontinued Product ? |
		------------------------*/
		if (inmr_rec.active_status [0] == 'D')
		{
			print_mess (ML (mlSkMess558));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (SR.lot_ctrl [0] == 'Y' && !SK_BATCH_CONT)
		{
			print_mess (ML ("Item lot controled by lot contol turned off?."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("itemDesc");
		SR.hhbrHash = inmr_rec.hhbr_hash;
		strcpy (SR.itemDesc, inmr_rec.description);
		SR.hhsiHash = alt_hash
						(
							inmr_rec.hhbr_hash, 
							inmr_rec.hhsi_hash
						);

		if (!ISSUE)
		{
		    local_rec.expDate = 0L;
		    if (inmr_rec.lot_ctrl [0] == 'Y')
		    {
				cc = FindInei (SR.hhsiHash, comm_rec.est_no, "r");
				if (cc) 
				{
					clear_mess ();
					errmess (ML (mlSkMess126));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				DateToDMY 
				(
					local_rec.lsystemDate, 
					&tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]
				);
				tmp_dmy [1]--;
				tmp_dmy [1] += ineiRec.expiry_prd1;
				tmp_dmy [2] += (tmp_dmy [1] / 12);
				tmp_dmy [1] %= 12;
				tmp_dmy [1]++;
				do
				{
					local_rec.expDate = DMYToDate 
										(
											tmp_dmy [0], 
											tmp_dmy [1], 
											tmp_dmy [2]
										);
					tmp_dmy [0]--;
				} while (local_rec.expDate == -1);
				
				strcpy (local_rec.c_exp_date, DateToString (local_rec.expDate));
		    }
		}

		local_rec.receiptDate = StringToDate (local_rec.defaultDate);

		if (!envSkIrTypes)
			ReadGlDefaults (inmr_rec.category);


		/*
		 * Look up to see if item is on Cost Centre
		 */
		incc_rec.hhcc_hash = CurrHHCC;
		incc_rec.hhbr_hash = alt_hash 
								(
									inmr_rec.hhbr_hash, 
									inmr_rec.hhsi_hash
								);
		if (FindIncc ())
		{
			if (ISSUE)
			{
				clear_mess ();
				errmess (ML (mlSkMess364));
	    		sleep (sleepTime);
			    clear_mess ();
				return (EXIT_FAILURE);
			}
			i = prmptmsg (ML (mlStdMess033), "YyNn", 1, 20);
			move (1, 20);
			cl_line ();
			if (i == 'n' || i == 'N') 
			{
				skip_entry = -1 ;
				return (EXIT_SUCCESS); 
			}
			else 
			{
				cc = AddIncc (incc_rec.hhcc_hash, incc_rec.hhbr_hash);
				if (cc)
					file_err (cc, incc, "DBADD");
			}
		}
		SR.hhwhHash	  	= incc_rec.hhwh_hash;
		SR.hhccHash		= incc_rec.hhcc_hash;
		strncpy (local_rec.item_desc, inmr_rec.description, 33);
		DSP_FLD ("itemDesc");
		strcpy (local_rec.lot_ctrl, inmr_rec.lot_ctrl);
		strcpy (SR.lot_ctrl, inmr_rec.lot_ctrl);
		local_rec.dec_pt = inmr_rec.dec_pt;
		
		strcpy (SR.cost_flag, inmr_rec.costing_flag);
		strcpy (SR.serFlag,   inmr_rec.serial_item);
		strcpy (SR.lot_ctrl,  inmr_rec.lot_ctrl);

		strcpy (local_rec.qcCentre, "    ");
		strcpy (local_rec.qcReqd, inmr_rec.qc_reqd);
		strcpy (SR.qcCentre, "    ");
		strcpy (SR.qcReqd, inmr_rec.qc_reqd);
		local_rec.hhbrHash = 	alt_hash 
								(
									inmr_rec.hhbr_hash, 
									inmr_rec.hhsi_hash
								);

		if (!ISSUE && !ADJ_ISS && envQcApply)
		{
			FLD ("qcCentre") = NA;
			if (inmr_rec.qc_reqd [0] == 'Y')
				FLD ("qcCentre") = NO;
		}
		tab_other (line_cnt);

		if (prog_status != ENTRY)
			UpdateTab (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Unit of Measure
	 */
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			SR.hhumHash = inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (SR._UOM, local_rec.UOM);
		SR.hhumHash = inum2_rec.hhum_hash;
        if (inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;
        SR.convFct = inum2_rec.cnv_fct/inum_rec.cnv_fct;

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*
	 * Check If Serial No already exists
	 */
	if (LCHECK ("serialNumber"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (F_HIDE (label ("serialNumber")))
			return (EXIT_SUCCESS);

		if (last_char == EOI && prog_status == ENTRY && (ISSUE || ADJ_ISS))
		{
			skip_entry = -2;
			return (EXIT_SUCCESS);
		}
		if (SR.serFlag [0] != 'Y')
			return (EXIT_SUCCESS);

		if (ISSUE || ADJ_ISS)
		{
			if (SRCH_KEY)
			{
				SearchInsf (local_rec.hhwhHash, "F", temp_str);
				return (EXIT_SUCCESS);
			}
			if (dflt_used)
			{
				print_mess (ML (mlStdMess201));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			local_rec.workCost	=	FindInsfCost
									(
										SR.hhwhHash, 
										0L,
										local_rec.ser_no, 
										"F"
									);
			if (local_rec.workCost == -1.00)
			{
				errmess (ML (mlStdMess201));
	    	   	sleep (sleepTime);
			   	clear_mess ();
				return (EXIT_FAILURE);
			}
			local_rec.workCost 	= n_dec (ToStdPrice (local_rec.workCost), 5);
			SR._cost 			= n_dec (local_rec.workCost, 5);
			local_rec.workQty 	= 1.00;

			DSP_FLD ("qty");
			DSP_FLD ("cost");

			if (CheckDupInsf (local_rec.ser_no, SR.hhsiHash, line_cnt))
			{
				print_mess (ML (mlSkMess559));
	    		sleep (sleepTime);
			    clear_mess ();
				return (EXIT_FAILURE);
			}

			if (insfRec.receipted [0] == 'N')
			{
				sprintf (err_str, ML (mlSkMess343), clip (local_rec.ser_no));
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (SR.ser_no, local_rec.ser_no);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.ser_no, serialSpace))
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckDupInsf (local_rec.ser_no, SR.hhsiHash, line_cnt))
		{
			print_mess (ML (mlSkMess559));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (SR.ser_no, local_rec.ser_no);

		cc = FindInsf (SR.hhwhHash, 0L, local_rec.ser_no, "F", "r");
		if (!cc)
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cc = FindInsf (SR.hhwhHash, 0L, local_rec.ser_no, "C", "r");
		if (!cc)
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.workQty = 1.00;

		DSP_FLD ("qty");

		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		float	qtyAvailable;

		local_rec.workQty = n_dec (local_rec.workQty, local_rec.dec_pt);
		if (!ValidQuantity (local_rec.workQty, local_rec.dec_pt))
			return (EXIT_FAILURE);

		if (SERIAL)
		{
			if (local_rec.workQty > 1.00)
				local_rec.workQty = 1.00;
		}

		if (GetGlAccounts (inmr_rec.category, line_cnt))
			return (EXIT_FAILURE);
			
		if (last_char == EOI && prog_status == ENTRY) 
		{
			skip_entry = -3;
			return (EXIT_SUCCESS);
		}
		if (FLD ("cost") != ND)
		{
			if (ISSUE)
				FLD ("cost") = NA;
			else if (ADJ_ISS)
				FLD ("cost") = NI;
			else
				FLD ("cost") = (envVarThreePl) ? NI : YES;
		}
		if (!ISSUE && !ADJ_ISS)
			return (EXIT_SUCCESS);

		/*
		 * Replace incc_rec.closing_stock with qtyAvailable.
		 */
		qtyAvailable = ReCalcAvail ();

		if (n_dec (qtyAvailable, inmr_rec.dec_pt) < ToStdUom (local_rec.workQty))
		{
			sprintf (tmpStr [0], local_rec.rep_qty, 
					n_dec (qtyAvailable, local_rec.dec_pt));
			sprintf (tmpStr [1], local_rec.rep_qty, local_rec.workQty);
			sprintf (err_str, ML (mlSkMess125), ToStdUom (local_rec.workQty), 
										n_dec (qtyAvailable, local_rec.dec_pt), 
											clip (local_rec.item_no));
			i = prmptmsg (err_str, "YyNn", 1, 2);
			move (1, 2);
			cl_line ();
			if (i == 'n' || i == 'N') 
				return (EXIT_FAILURE); 
		}

		I_VALUE (line_cnt) = (double) local_rec.workQty * local_rec.workCost;

		local_rec.receiptDate = StringToDate (local_rec.defaultDate);


		if (prog_status != ENTRY)
		{
			/*-------------------
			| Reenter Location. |
			--------------------*/
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				cc = spec_valid (label ("LL"));
			} while (cc);
		}
		return (EXIT_SUCCESS);
	}

    /*--------------------                                                 
    | Validate Location. |                                                 
    --------------------*/                                                
    if (LCHECK ("location")) 
    {                                                                           
		if (FLD ("location") == ND)
			return (EXIT_SUCCESS);

        if (SRCH_KEY)
        {                                                                       
			if (search_key	!=	SEARCH)
				SearchLomr (SR.hhccHash, temp_str);
			else
            	SearchLOC (TRUE, SR.hhwhHash, temp_str);
            return (EXIT_SUCCESS); 
        }                                                                       

		if (dflt_used)
		{
			strcpy (local_rec.location, DefaultLocation(SR.hhwhHash));

			cc = 	FindLocation 
					(
						SR.hhwhHash, 
						SR.hhumHash, 
						local_rec.location, 
						ValidLocations, 
						&workInloHash
					);
			if (cc && (ISSUE || ADJ_ISS))
			{
				print_mess (ML ("Invalid Location."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (!strcmp (local_rec.location, "          "))
		{
			print_mess (ML ("Location cannot be blank"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = 	FindLocation 
				(
					SR.hhwhHash, 
					SR.hhumHash, 
					local_rec.location, 
					ValidLocations, 
					&workInloHash
				);
		if (cc && (ISSUE || ADJ_ISS))
		{
			print_mess (ML ("Item does not exist at this Location."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			cc	=	CheckLocation	 
					(
						SR.hhccHash, 
						local_rec.location, 
						workLocationType
					);
			if (cc)
			{
            	print_mess (ML (mlStdMess209));
            	sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
    }
	/*
	 * Validate lots and locations.
	 */
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		if (!ISSUE && !ADJ_ISS)
		{
			LL_EditLoc	=	TRUE; 
			LL_EditLot	=	(SR.lot_ctrl [0] == 'Y') ? TRUE : FALSE;
			LL_EditDate	=	TRUE;
			LL_EditSLot	=	(SR.lot_ctrl [0] == 'Y') ? TRUE : FALSE;
		
			TempLine	=	lcount [2];
			InputExpiry = TRUE;	
			cc = DisplayLL
				(
					line_cnt, 
					tab_row + 3, 
					tab_col + 3, 
					4, 
					SR.hhwhHash, 
					SR.hhumHash, 
					SR.hhccHash, 
					SR._UOM, 
					local_rec.workQty, 
					SR.convFct, 
					StringToDate (local_rec.c_exp_date), 
					FALSE, 
					TRUE, 
					SR.lot_ctrl
				);
			/*-----------------
			| Redraw screens. |
			-----------------*/
			putval (line_cnt);

			lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
			scn_write (2);
			scn_display (2);
			lcount [2] = TempLine;
			if (cc)
				return (EXIT_FAILURE);
			else
				return (EXIT_SUCCESS);
		}

		if (local_rec.LL[0] == 'N')
			LotSelectFlag	=	INP_AUTO;
		else
			LotSelectFlag	=	INP_VIEW;

		if (prog_status == ENTRY)
			strcpy (local_rec.LL, "N");

		TempLine	=	lcount [2];
		InputExpiry = TRUE;	
		cc = DisplayLL
			(
				line_cnt, 
				tab_row + 3, 
				tab_col + 3, 
				4, 
				SR.hhwhHash, 
				SR.hhumHash, 
				SR.hhccHash, 
				SR._UOM, 
				local_rec.workQty, 
				SR.convFct, 
				StringToDate (local_rec.c_exp_date), 
				FALSE, 
				(local_rec.LL [0] == 'Y'), 
				SR.lot_ctrl
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		scn_write (2);
		scn_display (2);
		lcount [2] = TempLine;
			
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	/*
	 * Cost lookup (if none entered).
	 */
	if (LCHECK ("cost"))
	{
		if (last_char == EOI && prog_status == ENTRY && (ISSUE || ADJUST))
		{
			skip_entry = -4;
			return (EXIT_SUCCESS);
		}

		clear_mess ();

		cc = FindInei (SR.hhsiHash, comm_rec.est_no, "r");
		if (cc) 
		{
			clear_mess ();
			errmess (ML (mlSkMess126));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!ISSUE && !ADJ_ISS && (dflt_used || F_NOKEY (field)))
		{
			strcpy (inmr_rec.costing_flag, SR.cost_flag);

			switch (inmr_rec.costing_flag [0])
			{
			case 'A':
				local_rec.workCost = 	ineiRec.avge_cost;
				break;
			case 'L':
				local_rec.workCost = 	ineiRec.last_cost;
				break;
			case 'P':
				local_rec.workCost = 	ineiRec.prev_cost;
				break;
			case 'T':
				local_rec.workCost = 	ineiRec.std_cost;
				break;

			case 'F':
			case 'I':
			case 'S':
				local_rec.workCost = 	ineiRec.avge_cost;
				break;
			}
			if (FLD ("cost") == ND)
		    	local_rec.workCost = ineiRec.std_cost;

			if (local_rec.workCost <= 0.00)
				local_rec.workCost = ineiRec.last_cost;

			if (local_rec.workCost <= 0.00)
				local_rec.workCost = ineiRec.std_cost;

		}
		if (ISSUE || ADJ_ISS)
		{
			if (F_NOKEY (field) || dflt_used)
				local_rec.workCost	=	GetIssueCost ();
		}

		if (SR.convFct == 0.00)
			SR.convFct = 1.00;

		if (dflt_used || F_NOKEY (field))
			local_rec.workCost	=	n_dec (ToStdPrice (local_rec.workCost), 5);
		
		SR._cost = local_rec.workCost;

		DSP_FLD ("cost");

		I_VALUE (line_cnt) = (double) local_rec.workQty * local_rec.workCost;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qcCentre"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/* Read default from incc record. */
			incc_rec.hhcc_hash = CurrHHCC;
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			if (FindIncc ())
				return (EXIT_FAILURE);

			if (!strcmp (incc_rec.qc_centre, "    "))
				return (EXIT_FAILURE);
			
			strcpy (local_rec.qcCentre, incc_rec.qc_centre);
			DSP_FLD ("qcCentre");
			return (EXIT_SUCCESS);
		}

		if (envSkQcPass && !strcmp (local_rec.qcCentre, "PASS"))
			return (EXIT_SUCCESS);

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		strcpy (qcmr_rec.centre, local_rec.qcCentre);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit/Credit Amount input for General Ledger.
	 */
	if (LCHECK ("loc_gl_amt") || LCHECK ("dc2"))
	{
		if (local_rec.dc2_flag [0] == 'D') 
			D_VALUE (line_cnt) = local_rec.loc_gl_amt;
		else
			C_VALUE (line_cnt) = local_rec.loc_gl_amt;

		PrintJournalTotal ();
		strcpy (local_rec.gl_loc_curr, localCurrency);
		DSP_FLD ("loc_curr");
		PrintJournalTotal ();
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Narrative. |
	---------------------*/
	if (LCHECK ("glNarrative"))
	{
		if (dflt_used)
			strcpy (local_rec.glNarrative, local_rec.prevGlNarrative);
		else
			strcpy (local_rec.prevGlNarrative, local_rec.glNarrative);

		DSP_FLD ("glNarrative");
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate User Reference. |
	--------------------------*/
	if (LCHECK ("glUserRef"))
	{
		if (dflt_used)
			strcpy (local_rec.glUserRef, local_rec.prevGlUserRef);
		else
			strcpy (local_rec.prevGlUserRef, local_rec.glUserRef);

		DSP_FLD ("glNarrative");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate General Ledger Account Input.
	 */
	if (LCHECK ("glacct"))
	{
	    if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (local_rec.gl_acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (CheckClass ())
			return (EXIT_FAILURE);

		strcpy (local_rec.ca_desc, glmrRec.desc);
		DSP_FLD ("gl_desc");
	    return (EXIT_SUCCESS);
	}

	if (LCHECK ("cusOrdRef"))
	{
		if (!dflt_used)
			strcpy (local_rec.previousCusOrdRef, local_rec.cusOrdRef);
		else
			strcpy (local_rec.cusOrdRef, local_rec.previousCusOrdRef);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("packageQty"))
	{
		if (F_NOKEY (label ("packageQty")))
			return (EXIT_SUCCESS);
		
		/*
		 * don't use ToLclUom as it only returns same value when 3pl on.
		 */
		if (dflt_used)
			local_rec.packageQty = local_rec.workQty / SR.convFct;

		DSP_FLD ("packageQty");
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("vehicle"))
	{
		if (F_NOKEY (label ("vehicle")))
			return (EXIT_SUCCESS);

		if (dflt_used) 
		{
			strcpy (trve_rec.ref, "          ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTrve (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trve_rec.co_no, comm_rec.co_no);
		strcpy (trve_rec.br_no, comm_rec.est_no);
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
		{
			/*--------------------
			| Vehicle not found. |
			--------------------*/
			errmess (ML (mlStdMess218));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("cusOrdRef"))
	{
		if (F_NOKEY (label ("cusOrdRef")))
			return (EXIT_SUCCESS);

		if (!dflt_used)
			strcpy (local_rec.previousCusOrdRef, local_rec.cusOrdRef);
		else
			strcpy (local_rec.cusOrdRef, local_rec.previousCusOrdRef);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

double
GetIssueCost (
 void)
{
	float	workQty		=	0.00;
	double	costPrice	=	0.00;

	workQty =	(ISSUE) ? local_rec.workQty : local_rec.workQty * -1;
	
	strcpy (inmr_rec.costing_flag, SR.cost_flag);
	switch (inmr_rec.costing_flag [0])
	{
		case 'A':
			costPrice	=	ineiRec.avge_cost;
			break;

		case 'L':
			costPrice	=	ineiRec.last_cost;
			break;

		case 'F':
			costPrice	= FindIncfCost
				 		  (
							SR.hhwhHash, 
							n_dec (incc_rec.closing_stock, local_rec.dec_pt), 
							(float) ToStdUom (workQty), 
							TRUE, 
							inmr_rec.dec_pt
						  );

			if (costPrice <= 0.00)
				costPrice = ineiRec.last_cost;

			if (costPrice <= 0.00)
				costPrice = ineiRec.std_cost;
			break;

		case 'I':
			costPrice	= FindIncfCost
				 		  (
							SR.hhwhHash, 
							n_dec (incc_rec.closing_stock, local_rec.dec_pt), 
							(float) ToStdUom (workQty), 
							FALSE, 
							inmr_rec.dec_pt
						  );

			if (costPrice <= 0.00)
					costPrice =	ineiRec.last_cost;

			if (costPrice <= 0.00)
				costPrice = ineiRec.std_cost;
			break;

		case 'S':
			costPrice	=	FindInsfCost
							(
								SR.hhwhHash, 
								0L,
								local_rec.ser_no, 
								"F"
							);
			if (costPrice == -1.00)
			{
				costPrice	=	FindInsfValue
								(
									SR.hhwhHash, 
									TRUE
								);
			}
			break;

	}
	return (costPrice);
}

/*
 * Recalculate the current available stock.
 */
float 
ReCalcAvail (
 void)
{
	float	realStock;

	/*
	 * Look up incc record.
	 */
    incc2_rec.hhcc_hash = SR.hhccHash;
    incc2_rec.hhbr_hash = SR.hhsiHash;
    cc = find_rec (incc2, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	realStock = incc2_rec.closing_stock -
				incc2_rec.committed -
				incc2_rec.backorder;

	if (envQcApply && envSkQcAvl)
		realStock -= incc2_rec.qc_qty;

	/*
	 * Add into available any stock that was on line when loaded.
	 */
	if (SR.hhbrHash == SR._origHhbr)
		realStock += SR._origOrdQty;

	return (realStock);
}

/*
 * Checks if the quantity entered by the user
 * valid quantity that can be saved to a    
 * float variable without any problems of  
 * losing figures after the decimal point. 
 * eg. if _dec_pt is 2 then the greatest   
 * quantity the user can enter is 99999.99 
 */
int
ValidQuantity (
 double _qty, 
 int _dec_pt)
{
	/*
	 * Quantities to be compared with with the user has entered.  
	 */
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
		sprintf (tmpStr [0], local_rec.rep_qty, _qty);
		sprintf (tmpStr [1], local_rec.rep_qty, compare [_dec_pt]);
		sprintf (err_str, ML (mlSkMess238), 	_qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

int
ValidateDate (
 long	inp_date)
{

	long	endDate		=	0L;

	if (!JOURNAL)
	{
		endDate =	MonthEnd (comm_rec.inv_date);
		if (MonthEnd (local_rec.lsystemDate) > endDate)
			endDate =	MonthStart (local_rec.lsystemDate);

		if (inp_date < MonthStart (comm_rec.inv_date) || inp_date > endDate)
			return print_err (ML (mlStdMess246));
	}

	sprintf (local_rec.defaultDate, "%-10.10s", DateToString (inp_date));

	return (EXIT_SUCCESS);
}

/*
 * add transaction to incc files
 */
void
Update (void)
{
	char	workReference [16];

	int		NoLots		=	TRUE;
	int		updSknd 	= 	FALSE;
	int		i 			= 0;
	double 	old_qty 	= 0, 
			xx_qty 		= 0, 
			old_cost 	= 0, 
			temp_value 	= 0;

	double	WorkQty 	= 0.00;

	int		tran_type 	= 0;

	long	ff_date 	= 0L;

	float	Weeks 		= 0.00, 
			UnservedQty = 0.00;

	clear ();
	print_at (0, 0, ML (mlStdMess035));

	/*
	 * Routine to create number plate header for all inwards transactions.
	 */
	if (envSkGrinNoPlate && (RECEIPT || PURCHASE || OL_PUR))
		AddSknh ();

	if (envSkGrinNoPlate && ISSUE)
	{
		strcpy (sknh_rec.co_no, comm_rec.co_no);
		strcpy (sknh_rec.br_no, comm_rec.est_no);
		strcpy (sknh_rec.plate_no, local_rec.numberPlate);
		cc = find_rec (sknh, &sknh_rec, EQUAL, "u");
		if (!cc)
			UpdateSknh ();
		else
			abc_unlock (sknh);
	}

	/*
	 * Print header information based on fields seen by user + 
	 * print heading information for lines.					
	 */
	PrintHeaderInfo ();
	PrintDetailedInfo (-1);

	/*
	 * Update all inventory and general ledger records.
	 */
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		old_qty 	= 0.00;
		xx_qty 		= 0.00;
		old_cost	= 0.00;

		getval (line_cnt);

		/*
		 * Find inmr record from item number in structure. 
		 */
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.item_no);
		cc = find_rec (inmr , &inmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "inmr", "DBDFIND");
				
		cc	=	FindInei 
				(
					alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash), 
					comm_rec.est_no, 
					"u"
				);
		if (cc) 
			file_err (cc, inei, "DBDFIND");
	 
		/*
		 * Find warehouse record from master item hash.
		 */
		incc_rec.hhcc_hash = 	CurrHHCC;
		incc_rec.hhbr_hash = 	alt_hash 
								(
									inmr_rec.hhbr_hash, 
									inmr_rec.hhsi_hash
								);
		cc = find_rec (incc , &incc_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, incc, "DBDFIND");

		local_rec.hhwhHash = incc_rec.hhwh_hash;

		if (SR.convFct == 0.00)
			SR.convFct = 1.00;

		if ((ISSUE || ADJ_ISS) && newNumberPlate)
		{
			local_rec.workCost	=	ToStdPrice (GetIssueCost ());
			SR._cost			=	local_rec.workCost;
		}
		temp_value = ToLclPrice (SR._cost);
		temp_value = twodec (temp_value);
		
		if (!ISSUE && !ADJ_ISS)
		{
			/*
			 * If costing type of product is FIFO or LIFO then reduce 	 
			 * the number of records to equal current closing stock of WH.
			 */
			if (SR.cost_flag [0] == 'F' || SR.cost_flag [0] == 'I')
			{
		    	ReduceIncf
				(
					incc_rec.hhwh_hash, 
					n_dec (incc_rec.closing_stock, local_rec.dec_pt), 
					(SR.cost_flag [0] == 'F') ? TRUE : FALSE
				);
			}
		}
		
		if (!ISSUE && !ADJ_ISS)
		{
			/*
			 * Find branch record from master item hash.
			 */
			ineiRec.hhbr_hash = 	alt_hash 
									(
										inmr_rec.hhbr_hash, 
										inmr_rec.hhsi_hash
									);
			strcpy (ineiRec.est_no, comm_rec.est_no);
			cc = find_rec (inei , &ineiRec, COMPARISON, "u");
			if (cc)
				file_err (cc, inei, "DBDFIND");
		}

		/*
		 * update inventory cost centre stock record (file incc)
		 */
		if (RECEIPT)
		{
			incc_rec.receipts     += (float) ToStdUom (local_rec.workQty);
			incc_rec.ytd_receipts += (float) ToStdUom (local_rec.workQty);
			if (envQcApply &&
				(local_rec.qcReqd [0] == 'Y' && (!envSkQcPass ||
				(envSkQcPass && strcmp (local_rec.qcCentre, "PASS")))))
					incc_rec.qc_qty += (float) ToStdUom (local_rec.workQty);
		}
		if (PURCHASE || OL_PUR)
		{
			incc_rec.pur  += (float) ToStdUom (local_rec.workQty);
			incc_rec.ytd_pur += (float) ToStdUom (local_rec.workQty);
			if (envQcApply &&
				(local_rec.qcReqd [0] == 'Y' && (!envSkQcPass ||
				(envSkQcPass && strcmp (local_rec.qcCentre, "PASS")))))
					incc_rec.qc_qty += (float) ToStdUom (local_rec.workQty);
		}
		if (ISSUE)
		{
			incc_rec.issues 	+= (float) ToStdUom (local_rec.workQty);
			incc_rec.ytd_issues += (float) ToStdUom (local_rec.workQty);
		}
		if (ADJUST)
		{
			incc_rec.adj 	+= (float) ToStdUom (local_rec.workQty);
			incc_rec.ytd_adj 	+= (float) ToStdUom (local_rec.workQty);
		}

		WorkQty = GetBrClosing (incc_rec.hhbr_hash);
		old_qty = xx_qty = n_dec (WorkQty, local_rec.dec_pt);
		incc_rec.closing_stock =
						n_dec (incc_rec.opening_stock, 	local_rec.dec_pt) 
					  + n_dec (incc_rec.pur, 			local_rec.dec_pt) 
					  + n_dec (incc_rec.receipts, 		local_rec.dec_pt) 
					  + n_dec (incc_rec.adj, 			local_rec.dec_pt)
					  - n_dec (incc_rec.issues, 			local_rec.dec_pt)
					  - n_dec (incc_rec.sales, 			local_rec.dec_pt);

		/*
		 * Store out-of-stock date
		 */
		if (incc_rec.closing_stock <= 0.00 && ISSUE)
		{
			if (incc_rec.os_date == 0L)
			{
				if (comm_rec.inv_date > local_rec.lsystemDate)
					incc_rec.os_date = comm_rec.inv_date;
				else
					incc_rec.os_date = local_rec.lsystemDate;
			}
		}

		/*
		 * If the product now has stock on hand and the out of stock date >0L
		 * then add a record in the ffdm providing the weeks demand for item 
		 * less than 0.                                                      
		 */
		if (incc_rec.closing_stock > 0.00 && !ISSUE && !ADJ_ISS &&
			incc_rec.os_date != 0L)
		{
			Weeks = (float) ((local_rec.lsystemDate-incc_rec.os_date) / 7.00);
		
			if (incc_rec.wks_demand > 0.00)
			{
				UnservedQty = Weeks * incc_rec.wks_demand;

				AddFfdm 
				(
					UnservedQty, 
					incc_rec.hhbr_hash, 
					incc_rec.hhcc_hash
				);
				incc_rec.os_date		= 0L;
				incc_rec.os_ldate	= 0L;
				if (!strcmp (inmr_rec.ex_code, "o/s"))
					sprintf (inmr_rec.ex_code, "%-3.3s", " ");
			}
		}
		/*
		 * Update warehouse record.
		 */
		cc = abc_update (incc , &incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");

		/*
		 * Find Warehouse unit of measure file.
		 */
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON, "u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = find_rec (inwu, &inwu_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		/*
		 * update inventory cost centre stock record (file incc)
		 */
		if (RECEIPT)
			inwu_rec.receipts  	+= (float) ToStdUom (local_rec.workQty);
		if (PURCHASE || OL_PUR)
			inwu_rec.pur  		+= (float) ToStdUom (local_rec.workQty);
		if (ISSUE)
			inwu_rec.issues 	+= (float) ToStdUom (local_rec.workQty);
		if (ADJUST)
			inwu_rec.adj 		+= (float) ToStdUom (local_rec.workQty);

		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu, &inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");

		/*
		 * Somes details have been printed.
		 */
		PrintDetailedInfo (line_cnt);

		if (!ISSUE && !ADJ_ISS && local_rec.workQty != 0.00)
		{
			if (POS_PUR)
			{
				ineiRec.prev_cost 	= (SR.cost_flag [0] == 'A')
									? ineiRec.avge_cost : ineiRec.last_cost;
				ineiRec.last_cost 		= ToLclPrice (SR._cost);
				if (comm_rec.inv_date > local_rec.lsystemDate)
					ineiRec.date_lcost = comm_rec.inv_date;
				else
					ineiRec.date_lcost = local_rec.lsystemDate;
			}
			old_cost = ineiRec.avge_cost;
	
			if (old_qty < 0.00) 
				xx_qty = 0.00;

			if ((old_qty + (float) local_rec.workQty) == 0.00)
				ineiRec.avge_cost = ToLclPrice (SR._cost);
			else 	
			{
				if ((old_qty + (float) ToStdUom (local_rec.workQty)) < 0.00) 
				{    
					if (envAllowZeroCost)         
						ineiRec.avge_cost = 0.00;
					else
						ineiRec.avge_cost = old_cost;
				}     
				else 
				{
					ineiRec.avge_cost =
							((xx_qty * old_cost) +
							(ToStdUom (local_rec.workQty) * ToLclPrice (SR._cost))) /
							(xx_qty + (float) ToStdUom (local_rec.workQty));
				}
			}
			if (POS_PUR)
				ineiRec.lpur_qty = (float) ToStdUom (local_rec.workQty);

			/*
			 * Update branch records.  
			 */
			cc = abc_update (inei , &ineiRec);
			if (cc) 
				file_err (cc, inei, "DBUPDATE");

			switch (inmr_rec.costing_flag [0])
			{
			case 'F':
			case 'I':
				ff_date = local_rec.receiptDate;

				/*
				 * Product is in stock take mode so control  
				 * record  needs to be checked.             
				 */
				if (incc_rec.stat_flag [0] >= 'A' && 
					 incc_rec.stat_flag [0] <= 'Z')
				{
					while (CheckInsc (incc_rec.hhcc_hash, ff_date, incc_rec.stat_flag))
						ff_date += 1L;
				}
				if (local_rec.workQty > 0.00)
				{
					cc = AddIncf 
						(
							local_rec.hhwhHash,		/* incf_hhwh_hash		*/
							ff_date, 				/* incf_fifo_date		*/
							ToLclPrice (SR._cost), 	/* incf_fifo_cost		*/
							ToLclPrice (SR._cost), 	/* incf_act_cost		*/
							(float) ToStdUom (local_rec.workQty), 
							 " ", 					/* Goods receipt No		*/
							ToLclPrice (SR._cost), 	/* incf_fob_nor_cst		*/
							0.00, 					/* incf_frt_ins_cst		*/
							0.00, 					/* incf_duty			*/
							0.00, 					/* incf_license			*/
							0.00, 					/* incf_lcost_load		*/
							ToLclPrice (SR._cost), 	/* incf_land_cst		*/
							"0"						/* incf_stat_flag		*/
						);
					if (cc)
						file_err (cc, "incf", "DBADD");
				}
				break;
	
			default:
				break;
			}
			if (envSkGrinNoPlate)
			{
				memset (&sknd_rec, 0, sizeof (sknd_rec));
	
				sknd_rec.sknh_hash		=	sknh_rec.sknh_hash;
				sknd_rec.line_no		=	line_cnt;
				updSknd = find_rec (sknd, &sknd_rec, COMPARISON, "u");
	
				sknd_rec.hhbr_hash	=	SR.hhbrHash;
				sknd_rec.hhum_hash	=	SR.hhumHash;
				sknd_rec.hhcc_hash	=	SR.hhccHash;
				sknd_rec.hhsu_hash	=	0L;
				sknd_rec.hhve_hash	=	trve_rec.hhve_hash;
				strcpy (sknd_rec.cus_ord_ref, local_rec.cusOrdRef);
				strcpy (sknd_rec.serial_no, local_rec.ser_no); 
				sknd_rec.qty_rec		=	ToStdUom 	(local_rec.workQty);
				sknd_rec.land_cst		=	ToLclPrice 	(local_rec.workCost);
				strcpy (sknd_rec.status, "R");
				strcpy (sknd_rec.lstat_chg, "0");
				strcpy (sknd_rec.edi, "0");
		
				if (updSknd)
				{
					cc = abc_add (sknd, &sknd_rec);
					if (cc)
						file_err (cc, sknd, "DBADD");

					cc = find_rec (sknd, &sknd_rec, COMPARISON, "u");
					if (cc)
						file_err (cc, sknd, "DBFIND");
				}
				else
				{
					cc = abc_update (sknd, &sknd_rec);
					if (cc)
						file_err (cc, sknd, "DBUPDATE");
				}
			
				/*
				 * The following is required as a number of fields are not 
				 * includes in the lot/location window and as such these 
				 * fields need to be added.								
				 */
				if (MULT_LOC && SK_BATCH_CONT)
				{
					for (i = 0; i < MAX_LOTS; i++)
					{
						if (!LL_Valid (line_cnt, i))
							break;
	
						PutNoPlateData
						(
							line_cnt, 
							i, 
							"A", 
							local_rec.packageQty, 
							local_rec.totalChargeWgt, 
							local_rec.totalGrossWgt, 
							local_rec.totalCBM, 
							sknd_rec.sknd_hash
						);
					}
				}
			}
			if (MULT_LOC && SK_BATCH_CONT)
			{
				UpdateLotLocation 
				(
					line_cnt, 
					(ISSUE) ? TRUE : FALSE
				);

				/*
				 * Need to add a link to the location record (inlo) to have
				 * to read number plate detail back and update with hash.	
				 */
				if (envSkGrinNoPlate)
				{
					sknd_rec.sknh_hash		=	sknh_rec.sknh_hash;
					sknd_rec.line_no		=	line_cnt;
					cc = find_rec (sknd, &sknd_rec, COMPARISON, "r");
					if (cc)
						file_err (cc, sknd, "DBFIND");
				}
			}
			if (MULT_LOC && !SK_BATCH_CONT)
			{
				strcpy (err_str, DateToString (0L));
				InLotLocation 
				(							/*------------------------------*/
					SR.hhwhHash, 			/*	Link to WH record (incc).	*/
					SR.hhccHash, 			/*	Link to WH Master (ccmr).	*/
					SR.hhumHash, 			/* 	Link to UOM file  (inum).	*/
					SR._UOM, 				/*  UOM used for entry.			*/
					local_rec.batchString, 	/*  Batch control is off so N/A */
					local_rec.batchString, 	/*  Batch control is off so N/A	*/
					local_rec.location, 	/*  Location from input screen.	*/
					"A", 					/*  Location type. Will be set.	*/
					err_str, 				/*  Expiry date 0L as batch off */
					(float) ToStdUom (local_rec.workQty), /* qty   	*/
					SR.convFct, 			/* Conversion factor from input.*/
					"A", 
					local_rec.packageQty, 
					local_rec.totalChargeWgt, 
					local_rec.totalGrossWgt, 
					local_rec.totalCBM, 
					sknd_rec.sknd_hash
				);							/*------------------------------*/
				if (envSkGrinNoPlate)
				{
					sknd_rec.sknh_hash		=	sknh_rec.sknh_hash;
					sknd_rec.line_no		=	line_cnt;
					cc = find_rec (sknd, &sknd_rec, COMPARISON, "r");
					if (cc)
						file_err (cc, sknd, "DBFIND");
	
					cc = LoadLocation 
						(
							line_cnt, 
							sknd_rec.sknd_hash, 
							incc_rec.hhcc_hash, 
							SR._UOM, 
							SR.convFct, 
					 		ToStdUom (local_rec.workQty)
						);
					if (cc)
						file_err (cc, "inlo", "LoadLocation");
				}
			}
			if (inmr_rec.serial_item [0] == 'Y')
				AddInsf ();
		}
		if ((ISSUE || ADJ_ISS) && local_rec.workQty != 0.00)
		{
			if (MULT_LOC && SK_BATCH_CONT)
				UpdateLotLocation (line_cnt, (ISSUE) ? TRUE : FALSE);

			if (envSkGrinNoPlate)
			{
				sknd_rec.sknh_hash		=	sknh_rec.sknh_hash;
				sknd_rec.line_no		=	line_cnt;
				cc = find_rec (sknd, &sknd_rec, COMPARISON, "u");
				if (cc)
					abc_unlock (sknd);
				else
				{
					strcpy (sknd_rec.lstat_chg, "1");
					strcpy (sknd_rec.status, "I");
					cc = abc_update (sknd, &sknd_rec);
					if (cc)
						file_err (cc, sknd, "DBUPDATE");
				}
			}

			if (MULT_LOC && !SK_BATCH_CONT && ADJ_ISS)
			{
				strcpy (err_str, DateToString (0L));
				InLotLocation 
				(							/*------------------------------*/
					SR.hhwhHash, 			/*	Link to WH record (incc).	*/
					SR.hhccHash, 			/*	Link to WH Master (ccmr).	*/
					SR.hhumHash, 			/* 	Link to UOM file  (inum).	*/
					SR._UOM, 				/*  UOM used for entry.			*/
					local_rec.batchString, 	/*  Batch control is off so N/A */
					local_rec.batchString, 	/*  Batch control is off so N/A	*/
					local_rec.location, 	/*  Location from input screen.	*/
					" ", 					/*  Location type. Will be set.	*/
					err_str, 				/*  Expiry date 0L as batch off */
					(float) ToStdUom (local_rec.workQty), /* quantity */
					SR.convFct, 			/* Conversion factor from input.*/
					"A", 
					local_rec.packageQty, 
					local_rec.totalChargeWgt, 
					local_rec.totalGrossWgt, 
					local_rec.totalCBM, 
					sknd_rec.sknd_hash
				);							/*------------------------------*/
			}

			if (MULT_LOC && !SK_BATCH_CONT && ISSUE)
			{
				strcpy (err_str, DateToString (0L));
				OutLotLocation 
				(							/*------------------------------*/
					SR.hhwhHash, 			/*	Link to WH record (incc).	*/
					SR.hhccHash, 			/*	Link to WH Master (ccmr).	*/
					SR.hhumHash, 			/* 	Link to UOM file  (inum).	*/
					SR._UOM, 				/*  UOM used for entry.			*/
					local_rec.batchString, 	/*  Batch control is off so N/A */
					local_rec.batchString, 	/*  Batch control is off so N/A	*/
					local_rec.location, 	/*  Location from input screen.	*/
					" ", 					/*  Location type. Will be set.	*/
					err_str, 		   	 	/*  Expiry date 0L as batch off */
					(float) ToStdUom (local_rec.workQty), /* quantity */
					SR.convFct, 			/* Conversion factor from input.*/
					"A", 
					local_rec.packageQty, 
					local_rec.totalChargeWgt, 
					local_rec.totalGrossWgt, 
					local_rec.totalCBM, 
					sknd_rec.sknd_hash
				);							/*------------------------------*/
			}
			if (SERIAL)
			{
				cc = UpdateInsf (SR.hhwhHash, 0L, local_rec.ser_no, "F", "S");
				if (cc) 
					file_err (cc, insf, "DBUPDATE");
			}
		}

		strcpy (inmr_rec.item_no, local_rec.item_no);
		
		if (ISSUE)
			inmr_rec.on_hand -= (float) ToStdUom (local_rec.workQty);
		else
		{
			inmr_rec.on_hand += (float) ToStdUom (local_rec.workQty);
			if (envQcApply &&
				(local_rec.qcReqd [0] == 'Y' && (!envSkQcPass ||
				(envSkQcPass && strcmp (local_rec.qcCentre, "PASS")))))
				inmr_rec.qc_qty += (float) ToStdUom (local_rec.workQty);
		}

		if (inmr_rec.on_hand <= 0.00)
			if (!strcmp (inmr_rec.ex_code, "   "))
				sprintf (inmr_rec.ex_code, "%-3.3s", "o/s");

		/*
		 * Update inventory master records.
		 */
		cc = abc_update (inmr , &inmr_rec);
		if (cc) 
			file_err (cc, "inmr", "DBUPDATE");

		sprintf (workReference, "%-15.15s", local_rec.ser_no);
		
		if (ISSUE)
			tran_type = 3;

		if (RECEIPT)
			tran_type = 2;

		if (PURCHASE || OL_PUR)
			tran_type = 5;

		if (ADJUST)
			tran_type = 4;
	
		NoLots	=	TRUE;
		if (SK_BATCH_CONT && MULT_LOC)
		{
			for (i = 0; i < MAX_LOTS; i++)
			{
				if (!LL_Valid (line_cnt, i))
					break;
	
				NoLots	=	FALSE;
				/*--------------------------
				| Log inventory movements. |
				--------------------------*/
				MoveAdd 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					comm_rec.cc_no, 
					incc_rec.hhbr_hash, 
					incc_rec.hhcc_hash, 
					SR.hhumHash, 
					local_rec.receiptDate, 
					tran_type, 
					GetLotNo (line_cnt, i), 
					inmr_rec.inmr_class, 		
					inmr_rec.category, 
					local_rec.journalRef, 
					workReference, 
					GetBaseQty (line_cnt, i), 
					0.00, 
					CENTS (temp_value) 
				); 

			}
		}
		if (NoLots)
		{
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd 
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				comm_rec.cc_no, 
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash, 
				SR.hhumHash, 
				local_rec.receiptDate, 
				tran_type, 
				(MULT_LOC && envSkGrinNoPlate) ? GetLotNo (line_cnt, 0) : " ", 
				inmr_rec.inmr_class, 		
				inmr_rec.category, 
				local_rec.journalRef, 
				workReference, 
				(float) ToStdUom (local_rec.workQty), 
				0.00, 
				CENTS (temp_value) 
			); 
		}

		/*
		 * Add General Ledger inventory transactions.
		 */
		if (UPD_GL)
			AddInventoryGlwk ();


		if (!ISSUE && envQcApply &&
			local_rec.qcReqd [0] == 'Y' &&
			strcmp (local_rec.qcCentre, "PASS"))
		{
			int		qcDays;

			incc_rec.hhcc_hash = CurrHHCC;
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			if (FindIncc ())
				incc_rec.qc_time = 0.00;
			qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

			strcpy (qchr_rec.co_no, 		comm_rec.co_no);
			strcpy (qchr_rec.br_no, 		comm_rec.est_no);
			strcpy (qchr_rec.wh_no, 		comm_rec.cc_no);
			strcpy (qchr_rec.qc_centre, 	local_rec.qcCentre);
			qchr_rec.hhbr_hash			= local_rec.hhbrHash;
			qchr_rec.origin_qty			= (float) ToStdUom (local_rec.workQty);
			qchr_rec.receipt_dt			= local_rec.journalDate;
			qchr_rec.exp_rel_dt			= local_rec.journalDate + (long) qcDays;
			qchr_rec.rel_qty			= 0.00;
			qchr_rec.rej_qty			= 0.00;
			qchr_rec.inlo_hash			=	GetINLO (line_cnt, 0);
			qchr_rec.hhum_hash			=	SR.hhumHash;
			if (SR.serFlag [0] == 'Y')
				sprintf (qchr_rec.serial_no, "%-25.25s", local_rec.ser_no);
			else
				sprintf (qchr_rec.serial_no, "%-25.25s", " ");
			sprintf (qchr_rec.ref_1, 		"%-10.10s", local_rec.journalRef);
			sprintf (qchr_rec.ref_2, 		"%-10.10s", " ");
			qchr_rec.next_seq				= 0;
			strcpy (qchr_rec.source_type, 	"M");

			if ((cc = abc_add (qchr, &qchr_rec)))
				file_err (cc, "qchr", "DBFIND");
		}
	}

	if (GLSCREEN)
	{
		/*
		 * Update all inventory and general ledger records.
		 */
		scn_set (3);
		for (line_cnt = 0; line_cnt < lcount [3]; line_cnt++) 
		{
			getval (line_cnt);
			AddStandingGlwk ();
		}
	}
	strcpy (local_rec.previousItem, inmr_rec.item_no);
}

/*
 * Add transactions to glwk file.
 */
void
AddInventoryGlwk (void)
{
	int		dmy [3];
	double	wk_value = 0.00;

	wk_value = out_cost (ToLclPrice (SR._cost), inmr_rec.outer_size);

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, "10");
	glwkRec.post_date 	= local_rec.receiptDate;
	glwkRec.tran_date 	= local_rec.receiptDate;

	DateToDMY (local_rec.receiptDate, &dmy [0], &dmy [1], &dmy [2]);

	sprintf (glwkRec.period_no, "%02d", dmy [1]);
	sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
	strcpy (glwkRec.user_ref, local_rec.journalRef);
	strcpy (glwkRec.stat_flag, "2");
	if (RECEIPT)
		sprintf (glwkRec.narrative, "Receipt into W.H. %s", 
				comm_rec.cc_no);
	if (PURCHASE || OL_PUR)
		sprintf (glwkRec.narrative, "Purchase into W.H %s", 
				comm_rec.cc_no);
	if (ISSUE)
		sprintf (glwkRec.narrative, "Issue  from   W.H %s", 
				comm_rec.cc_no);
	if (ADJUST)
		sprintf (glwkRec.narrative, "Adjustment to W.H %s", 
				comm_rec.cc_no);

	sprintf (glwkRec.alt_desc1, "%20.20s", " ");
	sprintf (glwkRec.alt_desc2, "%20.20s", " ");
	sprintf (glwkRec.alt_desc3, "%20.20s", " ");
	sprintf (glwkRec.batch_no, "%10.10s", " ");

	glwkRec.amount = CENTS ((ToStdUom (local_rec.workQty) * wk_value));
	if (ADJ_ISS)
		glwkRec.amount *= -1;

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, SR.dbt_acc);
	glwkRec.hhgl_hash = SR.dbt_hash;

	strcpy (glwkRec.jnl_type, "1");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate	= 1.00;
	strcpy (glwkRec.currency, localCurrency);
	if (envSkMiscGlRt)
		GL_AddBatch ();
	else
	{
		cc = abc_add (glwk , &glwkRec);
		if (cc) 
			file_err (cc, "glwk", "DBADD");
	}

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, SR.crd_acc);
	glwkRec.hhgl_hash = SR.crd_hash;
	strcpy (glwkRec.jnl_type, "2");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate	= 1.00;
	strcpy (glwkRec.currency, localCurrency);
	if (envSkMiscGlRt)
		GL_AddBatch ();
	else
	{
		cc = abc_add (glwk , &glwkRec);
		if (cc) 
			file_err (cc, "glwk", "DBADD");
	}
}

/*
 * Add transactions to glwk file.
 */
void
AddStandingGlwk (void)
{
	int		dmy [3];

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, " 1");
	glwkRec.post_date 	= local_rec.receiptDate;
	glwkRec.tran_date	= local_rec.receiptDate;

	DateToDMY (local_rec.receiptDate, &dmy [0], &dmy [1], &dmy [2]);

	sprintf (glwkRec.period_no, "%02d", dmy [1]);
	sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
	strcpy (glwkRec.user_ref, local_rec.glUserRef);
	strcpy (glwkRec.stat_flag, "2");
	strcpy (glwkRec.narrative, local_rec.glNarrative);
	sprintf (glwkRec.alt_desc1, "%20.20s", " ");
	sprintf (glwkRec.alt_desc2, "%20.20s", " ");
	sprintf (glwkRec.alt_desc3, "%20.20s", " ");
	glwkRec.amount = local_rec.loc_gl_amt;

	strcpy (glwkRec.acc_no, local_rec.gl_acc_no);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	strcpy (glwkRec.jnl_type, (local_rec.dc2_flag [0] == 'D') ? "1" : "2");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate	= 1.00;
	strcpy (glwkRec.currency, localCurrency);
	GL_AddBatch ();
}
/*
 * Print report header information.
 */
void
PrintHeaderInfo (void)
{
	sprintf 
	(
		pWorkStr, 
		"%s - %s  /  %s - %s ", 
		PRN_FIELD ("ref"), local_rec.journalRef, 
		PRN_FIELD ("jdate"), DateToString (local_rec.journalDate)
	);
	PrintSingleLine (pWorkStr);
	
	if (envSkIrTypes)
	{
		sprintf 
		(
			pWorkStr, 
			"%s - %s  /  %s - %s ", 
			PRN_FIELD ("ir_type"), inir_rec.ir_type, 
			PRN_FIELD ("ir_desc"), inir_rec.ir_desc 
		);
		PrintSingleLine (pWorkStr);
	}
	if (envVarThreePl)
	{
		sprintf 
		(
			pWorkStr, 
			"%s - %s  /  %s - %s ", 
			PRN_FIELD ("vehicle"), trve_rec.ref, 
			PRN_FIELD ("vehicleDesc"), trve_rec.desc
		);
		PrintSingleLine (pWorkStr);
	}
	if (envSkGrinNoPlate)
	{
		sprintf 
		(
			pWorkStr, 
			"%s - %s, %s, %s", 
			PRN_FIELD ("numberPlate"), local_rec.numberPlate, 
			clip (sknh_rec.lab_note1), sknh_rec.lab_note2
		);
		PrintSingleLine (pWorkStr);
	}
}
/*
 * Print report detailed information.
 */
void
PrintDetailedInfo (
	int		line_cnt)
{
	int		field;
	int		headingLength;
	double	value = out_cost (ToLclPrice(SR._cost), inmr_rec.outer_size);

	if (line_cnt == -1)
	{
		strcpy (pWorkStr, "");

		for (field = label ("item_no"); field < label ("hhbrHash"); field++)
		{
			if (F_HIDE (field))
				continue;

			strcat (pWorkStr, FIELD.prmpt); 
			strcat (pWorkStr, "|");
		}
		if (UPD_GL)
		{
			strcat (pWorkStr, ML (" Debit Account  "));
			strcat (pWorkStr, "|");
			strcat (pWorkStr, ML (" Credit Account "));
			strcat (pWorkStr, "|");
		}
		strcat (pWorkStr, ML ("Extended Total"));
		strcat (pWorkStr, "|");
		headingLength = (int) strlen (pWorkStr);
		pin_bfill (err_str, '-', headingLength);
		PrintSingleLine (err_str);
		PrintSingleLine (pWorkStr);
		PrintSingleLine (err_str);
		return;
	}

	strcpy (pWorkStr, "");
	for (field = label ("item_no"); field < label ("hhbrHash"); field++)
	{
		if (F_HIDE (field))
			continue;
	
		if (LCHECK ("item_no"))
		{
			sprintf (err_str, "%s|", local_rec.item_no);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("itemDesc"))
		{
			sprintf (err_str, "%s|", local_rec.item_desc);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("qty"))
		{
			sprintf (err_str, "%10.2f|", local_rec.workQty);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("UOM"))
		{
			sprintf (err_str, "%s|", local_rec.UOM);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("LL"))
		{
			sprintf (err_str, "%1.1s|", local_rec.LL);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("packageQty"))
		{
			sprintf (err_str, "%11.2f|", local_rec.packageQty);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("totalChargeWgt"))
		{
			sprintf (err_str, "%10.2f|", local_rec.totalChargeWgt);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("totalGrossWgt"))
		{
			sprintf (err_str, "%10.2f|", local_rec.totalGrossWgt);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("totalCBM"))
		{
			sprintf (err_str, "%10.2f|", local_rec.totalCBM);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("location"))
		{
			sprintf (err_str, "%s|", local_rec.location);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("cusOrdRef"))
		{
			sprintf (err_str, "%s|", local_rec.cusOrdRef);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("cost"))
		{
			sprintf (err_str, "%10.2f|", local_rec.workCost);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("receiptDate"))
		{
			sprintf (err_str, "%10.10s|", DateToString (local_rec.receiptDate));
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("qcCentre"))
		{
			sprintf (err_str, "%s|", local_rec.qcCentre);
			strcat (pWorkStr, err_str);
		}
		if (LCHECK ("serialNumber"))
		{
			sprintf (err_str, "%s|", local_rec.ser_no);
			strcat (pWorkStr, err_str);
		}
	}
	if (UPD_GL)
	{
		sprintf (err_str, "%16.16s|%16.16s|", SR.dbt_acc, SR.crd_acc);
		strcat (pWorkStr, err_str);
	}
	sprintf (err_str, "%13.2f |", value * (double) ToStdUom (local_rec.workQty)) ;
	strcat (pWorkStr, err_str);
	PrintSingleLine (pWorkStr);

	batchTotal 	+= (value * (double) ToStdUom (local_rec.workQty));
	qtyTotal   += (float) ToStdUom (local_rec.workQty);
}

void
PrintSingleLine (
	char	*printString)
{
	fprintf (pp, "|%-154.154s|\n", printString);
}

/*
 * Routine to open output pipe to standard print to provide an audit trail
 * of events. This also sends the output straight to the spooler.        
 */
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat", "w")) == 0) 
		file_err (errno, "pformat", "POPEN");

	if (comm_rec.inv_date > local_rec.lsystemDate)
		fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	else
		fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n", printerNumber);
	fprintf (pp, ".9\n");
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".L156\n");
	
	if (RECEIPT)
		fprintf (pp, ".ESTOCK RECEIPTS\n");

	if (ADJUST)
		fprintf (pp, ".ESTOCK ADJUSTMENTS\n");

	if (PURCHASE || OL_PUR)
		fprintf (pp, ".ESTOCK PURCHASES\n");

	if (ISSUE)
		fprintf (pp, ".ESTOCK ISSUES\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s AS AT %s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp, ".EBRANCH %s : Warehouse %s \n", clip (comm_rec.est_name), clip (comm_rec.cc_name));

	fprintf (pp, ".R=============================================================================================================================================================\n");
	fprintf (pp, "=============================================================================================================================================================\n");
}


/*
 * Search for QC Centre
 */
void
SrchQcmr (
 char	*key_val)
{
	_work_open (4, 0, 40);
	save_rec ("#QC", "#Quanity Control Center Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", key_val);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, key_val, strlen (key_val)))
	{
		if (save_rec (qcmr_rec.centre, qcmr_rec.description))
			break;

		cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	if ((cc = find_rec (qcmr, &qcmr_rec, EQUAL, "r")))
		file_err (cc, "qcmr", "DBFIND");
}

int
AddIncc (
	long	hhccHash, 
	long	hhbrHash)
{
	memset (&incc_rec, 0, sizeof (incc_rec));

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	sprintf 
	(
		incc_rec.sort, "%s%11.11s%-16.16s", 
		inmr_rec.inmr_class, inmr_rec.category, inmr_rec.item_no
	);
	strcpy (incc_rec.stocking_unit, inmr_rec.sale_unit);
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.abc_code, inmr_rec.abc_code);
	strcpy (incc_rec.abc_update, inmr_rec.abc_update);
	strcpy (incc_rec.allow_repl, "E");
	if (comm_rec.inv_date > local_rec.lsystemDate)
		incc_rec.first_stocked = comm_rec.inv_date;
	else
		incc_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc_rec.stat_flag, "0");
	
	cc = abc_add (incc , &incc_rec);
	if (cc) 
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	return (FindIncc ());
}

/*
 * Routine to find incc record. Returns 0 if found ok, 1 if not on file.
 * 999 if a database error occurred.                                    
 */
int
FindIncc (void)
{
	cc = find_rec (incc , &incc_rec, COMPARISON, "r");
	local_rec.hhwhHash = incc_rec.hhwh_hash;
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Routine to add item at warehouse level if not already there.
 */
void
AddInsf (void)
{
	int	rc;

	insfRec.hhwh_hash = local_rec.hhwhHash;
	insfRec.hhbr_hash = alt_hash
						(
							inmr_rec.hhbr_hash, 
							inmr_rec.hhsi_hash
						 );
	if (envQcApply && local_rec.qcReqd [0] == 'Y')
		strcpy (insfRec.status, 	"C");
	else
		strcpy (insfRec.status, 	"F");
	sprintf (insfRec.serial_no, "%-25.25s", local_rec.ser_no);
	sprintf (insfRec.location, "%-10.10s", GetLoc (line_cnt, 0));
	insfRec.date_in 	= local_rec.receiptDate;
	insfRec.est_cost 	= SR._cost;
	strcpy (insfRec.receipted, "Y");
	strcpy (insfRec.stat_flag, "E");
	rc = abc_add (insf , &insfRec);
	if (rc) 
		file_err (rc, insf, "DBADD");
}

/*
 * Routine to close the audit trail output file.
 */
void
CloseAudit (void)
{
	sprintf (err_str, local_rec.rep_qty, qtyTotal);
	sprintf (pWorkStr, "%-30.30s :  %s", ML ("Total quantity of Batch "), err_str);
	PrintSingleLine (pWorkStr);
	sprintf (pWorkStr, "%-30.30s :%13.2f", ML ("Total amount of Batch"), batchTotal);
	PrintSingleLine (pWorkStr);
}

void
PformatClose (void)
{
	fprintf (pp, ".EOF\n");
	pclose (pp);
}

/*
 * Check Whether A Serial Number For This Item Number
 * Has Already Been Used. Return 1 if duplicate		
 */
int
CheckDupInsf (
	char 	*ser, 
	long 	hhbrHash, 
	int 	line_no)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_items;i++)
	{
		/*
		 * Ignore Current Line
		 */
		if (i == line_no)
			continue;

		/*
		 * cannot duplicate item_no/ser_no unless serial no was not input
		 */
		if (!strcmp (store [i].ser_no, serialSpace))
			continue;

		/*
		 * Only compare serial numbers for the same item number.
		 */
		if (store [i].hhbrHash == hhbrHash)
		{
			if (!strcmp (store [i].ser_no, ser))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}
/*
 * Process control Accounts.
 */
int
GetGlAccounts (
 char	*categoryNo, 
 int	linePosition)
{
	if (ISSUE)
	{
		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			comm_rec.cc_no, 
			"COSTSALE C", 
			" ", 
			categoryNo
		);
		sprintf (store [linePosition].dbt_acc, "%-16.16s", issueAcc);
		sprintf (store [linePosition].crd_acc, "%-16.16s", glmrRec.acc_no);
		store [linePosition].dbt_hash = issueHhmrHash;
		store [linePosition].crd_hash = glmrRec.hhmr_hash;
	}
	if (ADJUST)
	{
		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			comm_rec.cc_no, 
			(ADJ_ISS) ? "STK ADJ DB" : "STK ADJ CR", 
			" ", 
			categoryNo
		);
		if (ADJ_ISS)
		{
			strcpy (store [linePosition].dbt_acc, glmrRec.acc_no);
			store [linePosition].dbt_hash = glmrRec.hhmr_hash;
		}
		else
		{
			strcpy (store [linePosition].crd_acc, glmrRec.acc_no);
			store [linePosition].crd_hash = glmrRec.hhmr_hash;
		}
		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			comm_rec.cc_no, 
			"INVENTORY ", 
			" ", 
			categoryNo
		);
		if (ADJ_ISS)
		{
			strcpy (store [linePosition].crd_acc, glmrRec.acc_no);
			store [linePosition].crd_hash = glmrRec.hhmr_hash;
		}
		else
		{
			strcpy (store [linePosition].dbt_acc, glmrRec.acc_no);
			store [linePosition].dbt_hash = glmrRec.hhmr_hash;
		}
	}
	if (RECEIPT	|| PURCHASE || OL_PUR || POS_PUR)
	{
		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			comm_rec.cc_no, 
			"PURCHASE D", 
			" ", 
			categoryNo
		);
		strcpy (store [linePosition].crd_acc, receiptAcc);
		strcpy (store [linePosition].dbt_acc, glmrRec.acc_no);
		store [linePosition].crd_hash = receiptHhmrHash;
		store [linePosition].dbt_hash = glmrRec.hhmr_hash;
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	int		boxLineCnt	=	1;
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	switch (scn)
	{
	case 1 :
	case 2 :
	case 3 :
		if (RECEIPT)
			rv_pr (ML (mlSkMess346), 57, 0, 1);

		if (PURCHASE)
			rv_pr (ML (mlSkMess347), 57, 0, 1);

		if (OL_PUR)
			rv_pr (ML (mlSkMess348), 50, 0, 1);

		if (ISSUE)
			rv_pr (ML (mlSkMess349), 58, 0, 1);

		if (ADJUST)
			rv_pr (ML (mlSkMess350), 52, 0, 1);

		if (scn == 1)
		{
			if (envSkGrinNoPlate)
				boxLineCnt++;

			if (envVarThreePl)
				boxLineCnt++;

			if (!envSkIrTypes && boxLineCnt > 1)
				line_at (5, 1, 130);
			else
				boxLineCnt++;
			box (0, 3, 131, boxLineCnt);
		}
		break;

	}
	if (scn == 3 && GLSCREEN)
	{
	    cl_box (75, 2, 50, 3); 

	    print_at (3, 77, ML (mlSkMess351), 	localCurrency); 
	    print_at (4, 77, ML (mlSkMess352), 	localCurrency);
	    print_at (5, 77, ML (mlSkMess353), 	localCurrency); 
		PrintJournalTotal ();
	}

	print_at (0, 102, ML (mlSkMess089), 	local_rec.previousItem);
	line_at (1, 0, 130);
	line_at (21, 0, 130);

	strcpy (err_str, ML (mlStdMess038));		
	print_at (22, 0, err_str, comm_rec.co_no, clip (comm_rec.co_short));
	strcpy (err_str, ML (mlStdMess039));		
	print_at (22, 40, err_str, comm_rec.est_no, clip (comm_rec.est_short));
	strcpy (err_str, ML (mlStdMess099));		
	print_at (22, 70, err_str, comm_rec.cc_no, clip (comm_rec.cc_short));

	line_cnt = 0;	
	scn_write (scn);

	if (envSkGrinNoPlate && plateCalled && scn == 1)
	{
		scn_display (1);
		InputSknh ();
	}
	
    return (EXIT_SUCCESS);
}

int
CheckClass (void)
{
	if (glmrRec.glmr_class [2] [0] != 'P')
	{
		errmess (ML (mlStdMess025));
		sleep (sleepTime);		
		clear_mess ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
tab_other (
	 int	iline)
{
	if (cur_screen != 2)
		return;

	if (!ISSUE && envQcApply)
	{
		if (store [iline].qcReqd [0] != 'Y')
			FLD ("qcCentre") = NA;
		else
			FLD ("qcCentre") = NO;
	}

	if (!strncmp (store [iline].itemDesc, fortySpace, 25))
	{
		move (0, 3); cl_line ();
		return;
	}
	if (prog_status == ENTRY || iline < lcount [2])
	   	print_at (3, 0, ML (mlSkMess099), iline + 1, store [iline].itemDesc); 

	if (envSkSerialOk) 
	{
		if (store [iline].serFlag [0] == 'Y')
			FLD ("serialNumber")	= YES;
		else
			FLD ("serialNumber")	= NA;
	}
}

void
UpdateTab (
	int	line_no)
{

	/*-----------------
	| Check If Serial |
	-----------------*/
	if (store [line_no--].serFlag [0] == 'Y')
	{
		while (TRUE)
		{
			get_entry  (label ("serialNumber"));
			if (!spec_valid (label ("serialNumber")))
			{
				DSP_FLD ("serialNumber");
				break;
			}
		}
		get_entry  (label ("qty"));
		DSP_FLD ("qty");
	}

	while (TRUE)
	{
		get_entry  (label ("cost"));
		if (!spec_valid (label ("cost")))
		{
			DSP_FLD ("cost");
			break;
		}
	}

	if (!ISSUE && envQcApply)
	{
		if (store [line_no--].qcReqd [0] == 'Y')
		{
			do
			{
				get_entry (label ("qcCentre"));
				cc = spec_valid (label ("qcCentre"));
			} while (cc && !restart);
			DSP_FLD ("qcCentre");
		}
	}
}

/*
 * Get Branch closing Stock.
 */
float	
GetBrClosing (
 long	hhbrHash)
{
	float	ClosingStock = 0.00;

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no) && 
				   !strcmp (ccmr_rec.est_no, comm_rec.est_no))
	{
		incc2_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc2_rec.hhbr_hash = hhbrHash;

		if (!find_rec (incc2 , &incc2_rec, COMPARISON, "r"))
			ClosingStock += incc2_rec.closing_stock;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	return (ClosingStock);
}

/*
 * Search for Issue / Receipt type.
 */
void
SrchInir (
 char	*key_val)
{
	_work_open (2, 0, 40);
	save_rec ("#IR", "#Issue/Receipt Description.");
	strcpy (inir_rec.co_no, comm_rec.co_no);
	sprintf (inir_rec.ir_type, "%-2.2s", key_val);
	cc = find_rec (inir, &inir_rec, GTEQ, "r");
	while (!cc && !strcmp (inir_rec.co_no, comm_rec.co_no) &&
		      !strncmp (inir_rec.ir_type, key_val, strlen (key_val)))
	{
		cc = save_rec (inir_rec.ir_type, inir_rec.ir_desc);
		if (cc)
			break;

		cc = find_rec (inir, &inir_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inir_rec.co_no, comm_rec.co_no);
	sprintf (inir_rec.ir_type, "%-2.2s", temp_str);
	cc = find_rec (inir, &inir_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inir", "DBFIND");
}
/*
 * Search on UOM
 */
void
SrchInum (
 char 	*key_val)
{
	_work_open (4, 0, 40);
	save_rec ("#UOM", "#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inum2", "DBFIND");
}

/*
 * Print Journal proof total.
 */
void
PrintJournalTotal (
 void)
{
	int		i;
	double	ItemTotal = 0.00, 
			DbtTotal  = 0.00, 
			CrdTotal  = 0.00;
	
	for (i = 0; i < MAXLINES; i++)
	{
		ItemTotal += I_VALUE (i);
		DbtTotal  += DOLLARS (D_VALUE (i));
		CrdTotal  += DOLLARS (C_VALUE (i));
	}
	sprintf (err_str, "$%.2f", (ItemTotal));
	print_at (3, 100, "%-12.12s", err_str);

	sprintf (err_str, "$%.2f", DbtTotal);
	print_at (4, 100, "%-12.12s", err_str);

	sprintf (err_str, "$%.2f", CrdTotal);
	print_at (5, 100, "%-12.12s", err_str);

}

/*
 * Validate Proof total.
 */
void
ProofTrans (
 void)
{
	double	ItemCheck 	= 0.00, 
			DbtCheck 	= 0.00, 
			CrdCheck 	= 0.00;

	int		i;

	for (i = 0; i < MAXLINES; i++)
	{
		ItemCheck += I_VALUE (i);
		DbtCheck  += DOLLARS (D_VALUE (i));
		CrdCheck  += DOLLARS (C_VALUE (i));
	}
	if 
	( 
		MoneyZero (ItemCheck - DbtCheck) && 
		MoneyZero (ItemCheck - CrdCheck) && 
		MoneyZero (DbtCheck - CrdCheck)
	 )
		journalProof = 0;
	else
	{
		journalProof = 1;
		errmess (ML (mlSkMess345));
		sleep (sleepTime);
		clear_mess ();
	}	
	return;
}

/*
 * Minor support functions.
 */
int
MoneyZero (
	double	m)
{
	return (fabs (m) < 0.0001);
}

/*
 * Update or Add to ffdm table 
 */
void
AddFfdm (
	float	unsrv, 
	long	hhbrHash, 
	long	hhccHash)
{

	long	processDate	=	0L;

	if (comm_rec.inv_date > local_rec.lsystemDate)
		processDate	=	comm_rec.inv_date;
	else
		processDate	=	local_rec.lsystemDate;

	memset (&ffdm_rec, 0, sizeof (ffdm_rec));

	ffdm_rec.hhbr_hash = hhbrHash;
	ffdm_rec.hhcc_hash = hhccHash;
	ffdm_rec.date 	   = processDate;
	strcpy (ffdm_rec.type, "3");
	cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
	if (!cc)
	{
		ffdm_rec.qty += unsrv;

		cc = abc_update (ffdm, &ffdm_rec);
		if (cc)
            file_err (cc, "ffdm", "DBUPDATE");
    }
    else
    {
		ffdm_rec.qty = unsrv;
        cc = abc_add (ffdm, &ffdm_rec);
        if (cc)
            file_err (cc, "ffdm", "DBADD");
    }
}

/*
 * Input not stock description lines for non-stock products.
 */
void		
InputSknh (void)
{
	int 	tx_window;

	if (!plateCalled)
		return;

	strcpy (err_str, ML ("GOODS RECEIPT NUMBER PLATE DETAILS"));
	tx_window = txt_open (10, 30, 6, 60, 6, err_str);

	txt_pval (tx_window, sknh_rec.lab_note1, 0);
	txt_pval (tx_window, sknh_rec.lab_note2, 0);
	txt_pval (tx_window, sknh_rec.lab_note3, 0);
	txt_pval (tx_window, sknh_rec.lab_note4, 0);
	txt_pval (tx_window, sknh_rec.lab_note5, 0);
	txt_pval (tx_window, sknh_rec.lab_note6, 0);

	txt_edit (tx_window);

	sprintf (sknh_rec.lab_note1, "%-60.60s", txt_gval (tx_window, 1));
	sprintf (sknh_rec.lab_note2, "%-60.60s", txt_gval (tx_window, 2));
	sprintf (sknh_rec.lab_note3, "%-60.60s", txt_gval (tx_window, 3));
	sprintf (sknh_rec.lab_note4, "%-60.60s", txt_gval (tx_window, 4));
	sprintf (sknh_rec.lab_note5, "%-60.60s", txt_gval (tx_window, 5));
	sprintf (sknh_rec.lab_note6, "%-60.60s", txt_gval (tx_window, 6));

	txt_close (tx_window, FALSE);
}
/*
 * Update purchase order non stock lines file.
 */
void
AddSknh (void)
{
	GenGrnNo ();

	strcpy (sknh2_rec.co_no, comm_rec.co_no);
	strcpy (sknh2_rec.br_no, comm_rec.est_no);
	strcpy (sknh2_rec.plate_no, local_rec.numberPlate);

	newNumberPlate = find_rec (sknh, &sknh2_rec, COMPARISON, "u");
	
	strcpy (sknh2_rec.lab_note1, sknh_rec.lab_note1);
	strcpy (sknh2_rec.lab_note2, sknh_rec.lab_note2);
	strcpy (sknh2_rec.lab_note3, sknh_rec.lab_note3);
	strcpy (sknh2_rec.lab_note4, sknh_rec.lab_note4);
	strcpy (sknh2_rec.lab_note5, sknh_rec.lab_note5);
	strcpy (sknh2_rec.lab_note6, sknh_rec.lab_note6);
	strcpy (sknh2_rec.printed, "0");
	strcpy (sknh2_rec.edi, "0");

	if (newNumberPlate)
	{
		sknh2_rec.rec_date	=	TodaysDate ();

		cc = abc_add (sknh, &sknh2_rec);
		if (cc)
			file_err (cc, "sknh", "DBADD");
	}
	else
	{
		cc = abc_update (sknh, &sknh2_rec);
		if (cc)
			file_err (cc, "sknh", "DBUPDATE");
	}
	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
	strcpy (sknh_rec.plate_no, local_rec.numberPlate);
	cc = find_rec (sknh, &sknh_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sknh, "DBFIND");
}
/*
 * Update purchase order non stock lines file.
 */
void
UpdateSknh (void)
{
	sknh_rec.iss_date	=	TodaysDate ();
	cc = abc_update (sknh, &sknh_rec);
	if (cc)
		file_err (cc, "sknh", "DBUPDATE");
}

/*
 * Routine simply gets next GRIN No, Number Plate No is Grin Number Normally.
 */
void
GenGrnNo (void)
{
	/*
	 * Open Branch Master File.
	 */
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	/*
	 * Read Branch Master Record.
	 */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	/*
	 * Increment nx_gr_on and update esmr.
	 */
	sprintf (err_str, "%ld", ++esmr_rec.nx_gr_no);
	sprintf (local_rec.numberPlate, "%-15.15s", zero_pad (err_str, 15));
	cc = abc_update (esmr, &esmr_rec);
	if (cc)
		file_err (cc, "esmr", "DBUPDATE");

	/*
	 * Close Branch Master File. 
	 */
	abc_fclose (esmr);
}
/*
 * Load purchase order non stock lines file.
 */
void		
LoadSknh (void)
{
	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
	strcpy (sknh_rec.plate_no, local_rec.numberPlate);
	newNumberPlate = find_rec (sknh, &sknh_rec, COMPARISON, "u");
	if (newNumberPlate)
	{
		abc_unlock (sknh);
		memset (&sknh_rec , 0, sizeof (sknh_rec));
	}
}

void
LoadSknd (void)
{
	scn_set (2);
	lcount [2]	=	0;

	abc_selfield (inmr,  "inmr_hhbr_hash");
	abc_selfield (trve,  "trve_hhve_hash");
	abc_selfield (inum2, "inum_hhum_hash");

	sknd_rec.sknh_hash	= sknh_rec.sknh_hash;
	sknd_rec.line_no	= 0;
	cc = find_rec (sknd, &sknd_rec, GTEQ, "r");
	while (!cc && sknd_rec.sknh_hash == sknh_rec.sknh_hash)
	{

		if (sknd_rec.status [0] == 'I' || sknd_rec.status [0] == 'D')
		{
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
			continue;
		}
		line_cnt	=	lcount [2];
		inmr_rec.hhbr_hash	=	sknd_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		LSR.hhbrHash	=	sknd_rec.hhbr_hash;

		strcpy (local_rec.item_no, inmr_rec.item_no);
		LSR.hhbrHash = inmr_rec.hhbr_hash;
		LSR.hhsiHash = 	alt_hash
						(
							inmr_rec.hhbr_hash, 
							inmr_rec.hhsi_hash
						);

		if (!envSkIrTypes)
			ReadGlDefaults (inmr_rec.category);

		/*
		 * Look up to see if item is on Cost Centre
		 */
		incc_rec.hhcc_hash = 	sknd_rec.hhcc_hash;
		incc_rec.hhbr_hash = 	alt_hash 
								(
									inmr_rec.hhbr_hash, 
									inmr_rec.hhsi_hash
								);
		cc =	FindIncc ();
		if (cc)
			file_err (cc, incc, "DBFIND");

		LSR.hhwhHash	  	= incc_rec.hhwh_hash;
		LSR.hhccHash		= incc_rec.hhcc_hash;
		strncpy (local_rec.item_desc, inmr_rec.description, 33);
		DSP_FLD ("itemDesc");
		strcpy (local_rec.lot_ctrl, inmr_rec.lot_ctrl);
		strcpy (LSR.lot_ctrl, inmr_rec.lot_ctrl);
		local_rec.dec_pt = inmr_rec.dec_pt;
		
		strcpy (LSR.cost_flag, inmr_rec.costing_flag);
		strcpy (LSR.serFlag,  inmr_rec.serial_item);
		strcpy (LSR.lot_ctrl,  inmr_rec.lot_ctrl);

		strcpy (local_rec.qcCentre, "    ");
		strcpy (local_rec.qcReqd, inmr_rec.qc_reqd);
		strcpy (LSR.qcCentre, "    ");
		strcpy (LSR.qcReqd, inmr_rec.qc_reqd);
		local_rec.hhbrHash = 	alt_hash 
								(
									inmr_rec.hhbr_hash, 
									inmr_rec.hhsi_hash
								);

		trve_rec.hhve_hash	=	sknd_rec.hhve_hash;
		cc = find_rec (trve, &trve_rec, COMPARISON, "r");
		if (cc)
			strcpy (trve_rec.ref, " ");

		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		inum2_rec.hhum_hash	=	sknd_rec.hhum_hash;
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inum2, "DBFIND");
			
		if (inum2_rec.cnv_fct == 0.00)
			inum2_rec.cnv_fct = 1.00;

		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (LSR._UOM, local_rec.UOM);

		LSR.hhumHash	=	inum2_rec.hhum_hash;
		LSR.convFct		=	inum2_rec.cnv_fct / inum_rec.cnv_fct;
		cc = LoadLocation 
			(
				lcount [2], 
				sknd_rec.sknd_hash, 
				incc_rec.hhcc_hash, 
				LSR._UOM, 
				LSR.convFct, 
				sknd_rec.qty_rec - sknd_rec.qty_return

			);
		if (cc)
		{
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
			continue;
		}
		if (MULT_LOC && !SK_BATCH_CONT)
			strcpy (local_rec.location, GetLoc 		(lcount [2], 0));

		local_rec.workQty			=	GetBaseQty	(lcount [2], 0);
		local_rec.packageQty		=	GetPackQty	(lcount [2], 0);
		local_rec.totalChargeWgt	=	GetChgWgt	(lcount [2], 0);
		local_rec.totalGrossWgt		=	GetGrossWgt	(lcount [2], 0);
		local_rec.totalCBM			=	GetCuMetres	(lcount [2], 0);
		local_rec.workCost			=	ToStdPrice (sknd_rec.land_cst);
		LSR._cost 					=   ToStdPrice (sknd_rec.land_cst);

		local_rec.receiptDate		=	sknh_rec.rec_date;
		strcpy (local_rec.ser_no, sknd_rec.serial_no);
		strcpy (local_rec.cusOrdRef, sknd_rec.cus_ord_ref);

	   	putval (lcount [2]++);
	   	if (lcount [2] > MAXLINES) 
			break;

		cc = find_rec (sknd, &sknd_rec, NEXT, "r");
	}
	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (trve,  "trve_id_no");
	abc_selfield (inum2, "inum_id_no2");

	scn_set (1);
}

/*
 * Search for trve.
 */
void
SrchTrve (
	char	*keyValue)
{
	_work_open (10, 0, 40);
	save_rec ("#Vehicle", "#Description");
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", keyValue);
	cc = find_rec (trve, &trve_rec, GTEQ, "r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trve_rec.br_no, comm_rec.est_no) &&
			      !strncmp (trve_rec.ref, keyValue, strlen (keyValue)))
	{
		cc = save_rec (trve_rec.ref, trve_rec.desc);
		if (cc)
			break;
		cc = find_rec (trve, &trve_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", temp_str);
	cc = find_rec (trve, &trve_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trve, "DBFIND");
}

/*
 * Search Number Plate Header.
 */
void
SrchSknh (
	char    *keyValue)
{
	_work_open (15, 0, 40);
    save_rec ("#Number Plate   ", "#Number Plate description.");

	/*
	 * Flush record buffer first
	 */
	memset (&sknh_rec, 0, sizeof (sknh_rec));

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", keyValue);
    for (cc = find_rec (sknh, &sknh_rec,  GTEQ, "r");
		 !cc && !strcmp (sknh_rec.co_no, comm_rec.co_no)
		  && !strcmp (sknh_rec.br_no, comm_rec.est_no)
		  && !strncmp (sknh_rec.plate_no, keyValue, strlen (clip (keyValue)));
         cc = find_rec (sknh, &sknh_rec,  NEXT, "r"))
    {
        cc = save_rec (sknh_rec.plate_no, sknh_rec.lab_note1);
        if (cc)
            break;
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", temp_str);
    cc = find_rec (sknh, &sknh_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, sknh, "DBFIND");
}

/*
 * Convert Quantities to Standard UOM.
 */
float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")) || envVarThreePl)
		return (lclQty);

	if (SR.convFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR.convFct;

	return (cnvQty);
}
/*
 * Convert Quantities to Local UOM.
 */
float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")) || envVarThreePl)
		return (lclQty);

	if (SR.convFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.convFct;

	return (cnvQty);
}

/*
 * Convert Price or Cost to Standard UOM.
 */
double	
ToStdPrice (
	 double	lclPrice)
{
	double	cnvPrice;

	if (F_HIDE (label ("UOM")))
		return (lclPrice);

	if (SR.convFct == 0.00 || lclPrice == 0.00)
		return (0.00);

	cnvPrice = lclPrice * (double) SR.convFct;

	return (cnvPrice);
}
/*
 * Convert Price or Cost to Local UOM.
 */
double	
ToLclPrice (
	double	lclPrice)
{
	double	cnvPrice;

	if (F_HIDE (label ("UOM")))
		return (lclPrice);

	if (SR.convFct == 0.00 || lclPrice == 0.00)
		return (0.00);

	cnvPrice = lclPrice / (double) SR.convFct;

	return (cnvPrice);
}

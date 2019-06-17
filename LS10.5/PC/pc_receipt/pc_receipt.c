/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_receipt.c,v 5.13 2002/07/24 08:38:58 scott Exp $
|  Program Name  : (pc_receipt.c  )                                   |
|  Program Desc  : (Production Control Receipts                 )     |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written : 26/02/92          |
|---------------------------------------------------------------------|
| $Log: pc_receipt.c,v $
| Revision 5.13  2002/07/24 08:38:58  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.12  2002/07/18 06:50:07  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.11  2002/07/08 05:13:37  kaarlo
| LS 00850. Updated to fix UOM standard box and date raised alignment problem.
|
| Revision 5.10  2002/07/03 04:20:11  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.9  2001/09/24 06:55:13  cha
| ERROR-319. Updated to allow user to select or input
| location when receipting from production.
| Added pcms_pcms_hash to app.schema to search
| for the pcms_pcms_hash.
| Requirement for LS10-GUI.
|
| Revision 5.8  2001/09/04 08:17:01  scott
| Updated to stop printing of ruleoff line, caused blank entry in batch file.
|
| Revision 5.7  2001/08/09 09:14:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.6  2001/08/06 23:35:02  scott
| RELEASE 5.0
|
| Revision 5.5  2001/07/25 02:18:23  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_receipt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_receipt/pc_receipt.c,v 5.13 2002/07/24 08:38:58 scott Exp $";

#define MAXSCNS		3
#define MAXWIDTH	160
#define MAXLINES	200
#define TABLINES	4

#define LOT_ROW		10
#define LOT_COL		2

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<number.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<Costing.h>

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcrqRecord	pcrq_rec;
struct pcwcRecord	pcwc_rec;
struct pcwoRecord	pcwo_rec;
struct rgrsRecord	rgrs_rec;
struct pcatRecord	pcat_rec;
struct esmrRecord	esmr_rec;
struct pchsRecord	pchs_rec;
struct exwoRecord	exwo_rec;
struct qcmrRecord	qcmr_rec;
struct qchrRecord	qchr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

INEI_STRUCT	inei2Rec;

	int		*inei_expiry_prd	=	&ineiRec.expiry_prd1;

#include	<MoveRec.h>

	char	*data	= "data",
			*inmr2	= "inmr2",
			*inei2	= "inei2",
			*pcwo2	= "pcwo2";

#define	QCITEM		(inmr_rec.qc_reqd [0] == 'Y')
#define	LOT_CTRL	(inmr_rec.lot_ctrl [0] == 'Y')
#define	UPD_GL		(GLUpdateFlag [0] == 'G')
#define	SERIAL		(inmr_rec.serial_item [0] == 'Y')
#define	JOB_CLOSING	(pcwo_rec.order_status [0] == 'C')

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/
	FILE	*pp;
	int		popeOpen = FALSE;

struct	{
	char	name [10];
	int		no_days;
} mnths [] = {
	{"January",   31},
	{"February",  28},
	{"March",     31},
	{"April",     30},
	{"May",       31},
	{"June",      30},
	{"July",      31},
	{"August",    31},
	{"September", 30},
	{"October",   31},
	{"November",  30},
	{"December",  31},
	{"", 0}
};

	char	GLUpdateFlag [2],
			localCurrency [4],
			*serialSpace = "                         ";

	double	batchTotal 		= 0.00,
			quantityTotal 	= 0.00,
			thisReceipt		= 0.00,
			prevReceipt		= 0.00;

	int		printerNumber 	= 1,
			envVarQcApply 	= FALSE,
			envVarPcGenNum  = 0,
			envVarConOrders = 0,
			envVarSoWoAllowed 	= 0,
			batchFlag,
			periodMonth;

struct	storeRec {
	char	validSerial [26];
} store [MAXLINES];


	/* G/L hashes and account numbers */
	long	matHash [2],				/* 0 - direct	1 - mfg variance */
			dirHash [5],				/* 0 - labour   1 - machine  */
			fixHash [5],				/* 2 - qc-check 3 - special  */
			mfgDHash [5],				/* 4 - other    */
			mfgFHash [5];
	char	matAcc [2] [MAXLEVEL + 1],	/* 0 - direct	1 - mfg variance */
			dirAcc [5] [MAXLEVEL + 1],	/* 0 - labour   1 - machine  */
			fixAcc [5] [MAXLEVEL + 1],	/* 2 - qc-check 3 - special  */
			mfgDAcc [5] [MAXLEVEL + 1],	/* 4 - other    */
			mfgFAcc [5] [MAXLEVEL + 1];

struct {
	double	matCost;			/* Material Costs		 */
	double	dLabour;			/* Direct Labour Costs	 */
	double	dMachine;			/* Direct Machine Costs	 */
	double	dQcCheck;			/* Direct QC-Check Costs */
	double	dSpecial;			/* Direct Special Costs	 */
	double	dOther;				/* Direct Other Costs	 */
	double	fLabour;			/* Fixed Labour Costs	 */
	double	fMachine;			/* Fixed Machine Costs	 */
	double	fQcCheck;			/* Fixed QC-Check Costs	 */
	double	fSpecial;			/* Fixed Special Costs	 */
	double	fOther;				/* Fixed Other Costs	 */
} stdCostRec, actCostRec, recCostRec, thisRec;


/*===========================
| Local & Screen Structures.|
===========================*/
struct
{
	char	systemDate [11];
	long	lsystemDate;
	char	prevWoNo [8];
	double	receiptQty;
	double	standardUnitCost;
	double	costPerUnit;
	double	actCost;
	float	outerSize;
	long	hhwhHash;
	long	receiptDate;
	char	dfltRecDate [11];
	char	reference [26];
	char	serialNo [26];
	char	location [11];

	char	creditAccount [MAXLEVEL + 1];
	long	creditHhmrHash;

/* NEW */
	char	orderNo [8];
	char	batchNo [11];
	char	status [21];
	int		priority;
	long	dateRaised;
	long	hhbrHash;
	char	item_no [17];
	int		bom_alt;
	int		rtg_alt;
	int		bom_alt_old;
	int		rtg_alt_old;
	char	strength [6];
	char	desc [36];
	char	desc2 [41];
	char	stdUom [5];
	long	hhumHash;
	char	altUom [5];
	float	cnvFct;
	double	qty_rqd;
	char	stdBatchString [15];
	float	stdBatch;
	float	minBatch;
	float	maxBatch;
	float	prodMult;
	long	rqd_dte;
	double	previousReceipt;
	double	previousReject;
	double	rejectQty;
	char	reqBrName [16];
	char	reqWhName [10];
	char	recBrName [16];
	char	recWhName [10];
	char	reason [3];
	char	reasonDesc [21];
	char	wofAcc [MAXLEVEL + 1];
	char	qcCentre [5];
	long	currHhcc;
	long	mfgHhwhHash;
	char	oldRecBr [3];
	char	oldRecWh [3];
	char	LL [2];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "orderNo",	 3, 18, CHARTYPE,
		"UUUUUUU", "          ",
		" ", "", "Order Number:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.orderNo},
	{1, LIN, "batchNo",	 3, 48, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Batch Number:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.batchNo},
	{1, LIN, "priority",	 3, 95, INTTYPE,
		"N", "          ",
		" ", "5", "     Priority:", " ",
		NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.priority},
	{1, LIN, "dateRaised",	 3, 116, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "     Date Raised:", " ",
		NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.dateRaised},
	{1, LIN, "item_no",	 5, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "bom_alt",	 5, 55, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "BOM Alt. No. : ", " ",
		NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.bom_alt},
	{1, LIN, "rtg_alt",	 6, 55, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "Routing No.  : ", " ",
		NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.rtg_alt},
	{1, LIN, "strength",	 6, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength   :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "desc",		 8, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "desc2",	9, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "stdUom",	 6, 88, CHARTYPE,
		"AAAA", "          ",
		"", "", "Standard:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.stdUom},
	{1, LIN, "altUom",	 6, 115, CHARTYPE,
		"AAAA", "          ",
		"", "", "  Alternate:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.altUom},
	{1, LIN, "qty_rqd",	11, 18, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", local_rec.stdBatchString, "Quantity Reqd:", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_rqd},
	{1, LIN, "rqd_dte",	11, 88, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Reqd Date:", " ",
		NA, NO, JUSTLEFT, "", "", (char *) &local_rec.rqd_dte},
	{1, LIN, "previousReceipt",	12, 18, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", " ", "Prev. Receipted :", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.previousReceipt},
	{1, LIN, "previousReject",	12, 88, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", " ", "Prev. Rejected  :", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.previousReject},
	{1, LIN, "reqBrNo",	 14, 18, CHARTYPE,
		"AA", "          ",
		" ", " ", "Requesting Br:", "Requesting Branch.",
		NA, NO,  JUSTRIGHT, "", "", pcwo_rec.req_br_no},
	{1, LIN, "reqBrName",	 14, 23, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqBrName},
	{1, LIN, "reqWhNo",	 14, 70, CHARTYPE,
		"AA", "          ",
		" ", " ", "Wh:", "Requesting Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", pcwo_rec.req_wh_no},
	{1, LIN, "reqWhName",	 14, 75, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqWhName},
	{1, LIN, "recBrNo",	 15, 18, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no, "Receiving Br :", "Receiving Branch.",
		YES, NO,  JUSTRIGHT, "", "", pcwo_rec.rec_br_no},
	{1, LIN, "recBrName",	 15, 23, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recBrName},
	{1, LIN, "recWhNo",	 15, 70, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.cc_no, "Wh:", "Receiving Warehouse.",
		YES, NO,  JUSTRIGHT, "", "", pcwo_rec.rec_wh_no},
	{1, LIN, "recWhName",	 15, 75, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recWhName},

	{2, LIN, "receiptDate",	17, 17, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.dfltRecDate, "Receipt Date : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.receiptDate},
	{2, LIN, "qcCentre",	17, 88, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "QC Centre     : ", "QC Centre - Defaults To Warehouse QC Centre",
		YES, NO,  JUSTLEFT, "", "", local_rec.qcCentre},
	{2, LIN, "rejectQty",	18, 17, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "0.00", "Qty Rejected : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.999999", (char *) &local_rec.rejectQty},
	{2, LIN, "qty",		18, 88, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", "Quantity      : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.999999", (char *) &local_rec.receiptQty},
	{2, LIN, "reason",		19, 17, CHARTYPE,
		"UU", "          ",
		" ", "", "Reason       : ", "Reason For Rejected Quantity",
		 NO, NO,  JUSTLEFT, "", "", local_rec.reason},
	{2, LIN, "reasonDesc",		19, 20, CHARTYPE,
		"UU", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.reasonDesc},

	{2, LIN, "LL", 	 19, 88, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Location   :", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{2, LIN, "stdUnitCost",		20, 17, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Std Unit Cost: ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.standardUnitCost},
	{2, LIN, "costPerUnit",		20, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Cost Per Unit: ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.costPerUnit},
	{2, LIN, "price",		20, 110, FLOATTYPE,
		"NNNNNNN.N", "          ",
		" ", " ", "Pricing Conv  : ", " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &local_rec.outerSize},

	{3, TAB, "serialNo",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Serial Number      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.serialNo},
	{3, TAB, "ser_loc",	 0, 0, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Location ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.location},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<LocHeader.h>
#include	<MoveAdd.h>
#include	<SrchPcwo2.h>

/*=====================
| function prototypes |
=====================*/
double 	UpdatePcln 			(int);
double 	UpdatePcms 			(int);
float 	GetSerialNo 		(void);
float 	GetUom 				(long, long);
int 	AddIncc 			(long);
int 	DeleteLine 			(void);
int 	DisplayDetails 		(int);
int 	FindIncc 			(void);
int 	GetAccount 			(char *);
int 	ReadDefault 		(char *);
int 	Update 				(void);
int 	ValidQuantity 		(double, int);
int 	heading 			(int);
int 	spec_valid 			(int);
static 	int ValidateDate 	(long);
void 	AddPcgl 			(long, char *, char *, double, char *, char *);
void 	AddSerial 			(void);
void 	CalcCost 			(void);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintDetails 		(int);
void	ProcessSalesOrder 	(long);
void 	RewriteScreen 		(void);
void 	SrchCcmr 			(char *, char *);
void 	SrchEsmr 			(char *);
void 	SrchExwo 			(char *);
void 	SrchQcmr 			(char *);
void 	TransStock 			(float);
void 	UpdateFifo 			(long, float, float);
void 	UpdateQchr 			(float, long);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int  argc, 
 char *argv[])
{
	char	*sptr;

	if (argc < 3)
	{
		print_at (0,0,"Usage : %s <printerNumber> <G(en Ledger)/J(ournal)>", argv [0]);
		return (EXIT_FAILURE);
	}

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		envVarPcGenNum = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envVarPcGenNum = TRUE;

	printerNumber = atoi (argv [1]);

	switch (argv [2] [0])
	{
	case	'G':
	case	'g':
		strcpy (GLUpdateFlag, "G");
		break;

	case	'J':
	case	'j':
		strcpy (GLUpdateFlag, "J");
		break;

	default:
		print_at (0,0,"Usage : %s <printerNumber> <G(en Ledger)/J(ournal)>", argv [0]);
		return (EXIT_FAILURE);
	}

	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/*------------------------------
	| Check if Works Order Allowed |
	------------------------------*/
	sptr = chk_env ("CON_ORDERS");
	envVarConOrders = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*--------------------------------
	| Check Consolidation of orders. |
	--------------------------------*/
	sptr = chk_env ("SO_WO_ALLOWED");
	envVarSoWoAllowed = (sptr == (char *)0) ? 0 : atoi (sptr);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	GL_SetMask ("XXXXXXXXXXXXXXXX");

	FLD ("qcCentre") = envVarQcApply ? YES : ND;

	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;


	strcpy (local_rec.dfltRecDate,DateToString  (comm_rec.inv_date));
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);

	swide ();

	while (!prog_exit)
	{
		/*  reset control flags  */

		entry_exit = 0;
		restart = 0;
		edit_exit = 0;
		prog_exit = 0;
		search_ok = 1;

		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [3] = 0;

		FLD ("orderNo") = NO;
		FLD ("batchNo") = YES;

		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			abc_unlock (pcwo);
			continue;
		}
		FLD ("orderNo") = NE;
		FLD ("batchNo") = NE;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}
		heading (1);
		scn_display (1);

		heading (2);
		entry (2);
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		heading (2);
		scn_display (2);
		edit (2);
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	if (popeOpen)
		CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);
	abc_alias (inmr2, inmr);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcln, pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_hhwo_hash");
	open_rec (pcrq, pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc, pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exwo, exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec ("move",move_list,MOVE_NO_FIELDS, "move_move_hash");
	if (envVarQcApply)
		open_rec (qcmr, qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------------
	| Read ccmr for current warehouse. |
	----------------------------------*/
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	OpenLocation(ccmr_rec.hhcc_hash);
	IgnoreAvailChk	=	TRUE;
	
	LL_EditLoc	=	TRUE; 
	LL_EditLot	=	TRUE;
	LL_EditDate	=	TRUE;
	LL_EditSLot	=	TRUE;

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
	OpenInsf ();
	OpenIncf ();

	if (envVarSoWoAllowed)
	{
		open_rec (sohr,sohr_list,SOHR_NO_FIELDS, "sohr_hhso_hash");
		open_rec (soln,soln_list,SOLN_NO_FIELDS, "soln_hhsl_hash");
	}
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (glmr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgrs);
	abc_fclose (esmr);
	abc_fclose (exwo);
	abc_fclose ("move");
	CloseLocation();
	CloseCosting();
	if (envVarQcApply)
		abc_fclose (qcmr);
	GL_Close ();
	
	SearchFindClose ();

	if (envVarSoWoAllowed)
	{
		abc_fclose (sohr);
		abc_fclose (soln);
	}
	abc_dbclose (data);
}

/*========================================
| Read all nessasary files for defaults. |
========================================*/
int
ReadDefault (
	char	*category)
{
	abc_selfield (glmr, "glmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.rec_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.rec_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML(mlStdMess100));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	local_rec.currHhcc = ccmr_rec.hhcc_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		category
	);
	matHash [0] = glmrRec.hhmr_hash;
	strcpy (matAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D LABR",
		" ",
		category
	);
	dirHash [0] = glmrRec.hhmr_hash;
	strcpy (dirAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MACH",
		" ",
		category
	);
	dirHash [1] = glmrRec.hhmr_hash;
	strcpy (dirAcc [1], glmrRec.acc_no);

	GL_GLI 
	(	
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D QC  ",
		" ",
		category
	);
	dirHash [2] = glmrRec.hhmr_hash;
	strcpy (dirAcc [2], glmrRec.acc_no);

	GL_GLI
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D SPEC",
		" ",
		category
	);
	dirHash [3] = glmrRec.hhmr_hash;
	strcpy (dirAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D OTH ",
		" ",
		category
	);
	dirHash [4] = glmrRec.hhmr_hash;
	strcpy (dirAcc [4], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F LABR",
		" ",
		category
	);
	fixHash [0] = glmrRec.hhmr_hash;
	strcpy (fixAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F MACH",
		" ",
		category
	);
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,GL_Account);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	fixHash [1] = glmrRec.hhmr_hash;
	strcpy (fixAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F QC  ",
		" ",
		category
	);
	fixHash [2] = glmrRec.hhmr_hash;
	strcpy (fixAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F SPEC",
		" ",
		category
	);
	fixHash [3] = glmrRec.hhmr_hash;
	strcpy (fixAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F OTH ",
		" ",
		category
	);
	fixHash [4] = glmrRec.hhmr_hash;
	strcpy (fixAcc [4], glmrRec.acc_no);

	/* Manufacturing Variance Direct Accounts */
	GL_GLI
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MATL",
		" ",
		category
	);
	matHash [1] = glmrRec.hhmr_hash;
	strcpy (matAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D LABR",
		" ",
		category
	);
	mfgDHash [0] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MACH",
		" ",
		category
	);
	mfgDHash [1] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D QC  ",
		" ",
		category
	);

	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,GL_Account);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	mfgDHash [2] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D SPEC",
		" ",
		category
	);
	mfgDHash [3] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D OTH ",
		" ",
		category
	);
	mfgDHash [4] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [4], glmrRec.acc_no);

	/* Manufacturing Variance Fixed Accounts */
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F LABR",
		" ",
		category
	);
	mfgFHash [0] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F MACH",
		" ",
		category
	);
	mfgFHash [1] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F QC  ",
		" ",
		category
	);
	mfgFHash [2] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F SPEC",
		" ",
		category
	);
	mfgFHash [3] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F OTH ",
		" ",
		category
	);
	mfgFHash [4] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [4], glmrRec.acc_no);

	abc_selfield (glmr, "glmr_hhmr_hash");

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int		i,
			flag;

	if (LCHECK  ("orderNo"))
	{
		if (SRCH_KEY)
		{
			SearchOrder (temp_str, "RC", comm_rec.est_no, comm_rec.cc_no);
			return (EXIT_SUCCESS);
		}
	
		if (dflt_used)
		{
			FLD ("batchNo") = YES;
			return (EXIT_SUCCESS);
		}
		else
			FLD ("batchNo") = NA;

		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		if (envVarPcGenNum)
			strcpy (pcwo_rec.order_no, zero_pad (local_rec.orderNo, 7));
		else
			strcpy (pcwo_rec.order_no, local_rec.orderNo);
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		flag = DisplayDetails (FALSE);
		if (flag)
			sprintf (local_rec.orderNo, "%-7.7s", " ");

		return (flag);
	}

	if (LCHECK  ("batchNo"))
	{
		if (SRCH_KEY)
		{
			SearchBatch (temp_str, "RC", comm_rec.est_no, comm_rec.cc_no);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		strcpy (pcwo_rec.batch_no, local_rec.batchNo);
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.batchNo, "%-10.10s", " ");
			return (EXIT_FAILURE);
		}

		flag = DisplayDetails (TRUE);
		if (flag)
			sprintf (local_rec.batchNo, "%-10.10s", " ");

		return (flag);
	}

	if (LCHECK ("recBrNo"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, pcwo_rec.rec_br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.recBrName, esmr_rec.short_name);
		DSP_FLD ("recBrName");
		DSP_FLD ("recBrNo");
		/*---------------------------
		| prompt user for warehouse |
		---------------------------*/
		do
		{
			if (restart)
				return (EXIT_SUCCESS);
			get_entry (label ("recWhNo"));
			cc = spec_valid (label ("recWhNo"));
		} while (cc && !restart);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("recWhNo"))
	{
		if (restart)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str, pcwo_rec.rec_br_no);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, pcwo_rec.rec_br_no);
		strcpy (ccmr_rec.cc_no, pcwo_rec.rec_wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.recWhName, ccmr_rec.acronym);
		DSP_FLD ("recWhName");
		DSP_FLD ("recWhNo");

		/*--------------------------
		| read all default files . |
		--------------------------*/
		if ((cc = ReadDefault (inmr_rec.category)))
			return (EXIT_FAILURE);

		/*---------------------------
		| Process control Accounts. |
		---------------------------*/
		if (GetAccount (inmr_rec.category))
			return (EXIT_FAILURE);

		incc_rec.hhcc_hash = local_rec.currHhcc;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		if (FindIncc ())
		{
			i = prmptmsg (ML(mlStdMess033),"YyNn",1,20);
			print_at (20, 0, "                                                                ");
			if (i == 'n' || i == 'N')
			{
				skip_entry = -1 ;
				return (EXIT_FAILURE);
			}
			else
			{
				cc = AddIncc (local_rec.currHhcc);
				if (cc)
					file_err (cc, (char *)incc, "DBADD");
			}
			local_rec.hhwhHash = incc_rec.hhwh_hash;
		}

		return (EXIT_SUCCESS);
	}

	/*---------------
	| Journal Date. |
	---------------*/
	if (LCHECK ("receiptDate"))
	{
		local_rec.outerSize = inmr_rec.outer_size;
		CalcCost ();

		local_rec.standardUnitCost /= pcwo_rec.prod_qty;
		local_rec.standardUnitCost *= local_rec.outerSize;
		local_rec.costPerUnit = local_rec.standardUnitCost / local_rec.outerSize;
		DSP_FLD ("stdUnitCost");
		DSP_FLD ("costPerUnit");
		DSP_FLD ("price");

		return (ValidateDate (local_rec.receiptDate));
	}

	/*---------------------------
	| Validate QC Centre input. |
	---------------------------*/
	if (LCHECK ("qcCentre"))
	{
		if (!envVarQcApply ||
			F_NOKEY (label ("qcCentre")))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			/* Read default from incc record. */
			incc_rec.hhcc_hash = local_rec.currHhcc;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			if (find_rec (incc, &incc_rec, EQUAL, "r"))
				return (EXIT_FAILURE);

			if (!strcmp (incc_rec.qc_centre, "    "))
				return (EXIT_FAILURE);
			
			strcpy (local_rec.qcCentre, incc_rec.qc_centre);
			DSP_FLD ("qcCentre");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			RewriteScreen ();
			return (EXIT_SUCCESS);
		}

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		strcpy (qcmr_rec.centre, local_rec.qcCentre);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			print_mess (ML(mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("serialNo"))
	{
		if ((dflt_used || last_char == DELLINE) && prog_status != ENTRY)
			return (DeleteLine ());

		if (!strcmp (local_rec.serialNo, serialSpace))
		{
			print_mess (ML(mlPcMess101));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		for (i = 0; i < lcount [3]; i++)
		{
			if (!strcmp (store [i].validSerial, local_rec.serialNo))
			{
				print_mess (ML(mlPcMess102));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		cc = FindInsf (local_rec.hhwhHash, 0L, local_rec.serialNo,"F","r");
		if (!cc)
		{
			print_mess (ML(mlPcMess103));
			return (EXIT_FAILURE);
		}

		cc = FindInsf (local_rec.hhwhHash, 0L, local_rec.serialNo,"C","r");
		if (!cc)
		{
			print_mess (ML(mlPcMess103));
			return (EXIT_FAILURE);
		}

		sprintf (store [line_cnt].validSerial, "%-25.25s", local_rec.serialNo);

		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Qty rejected validation lookup. |
	---------------------------------*/
	if (LCHECK ("rejectQty"))
	{
		local_rec.rejectQty = n_dec (local_rec.rejectQty, inmr_rec.dec_pt);
		DSP_FLD ("rejectQty");
		if (!ValidQuantity (local_rec.rejectQty, inmr_rec.dec_pt))
			return (EXIT_FAILURE);

		if (local_rec.rejectQty > 0.00)
			FLD ("reason") = YES;
		else
		{
			strcpy (local_rec.reason, " ");
			strcpy (local_rec.reasonDesc, " ");
			FLD ("reason") = NA;
			DSP_FLD ("reason");
			DSP_FLD ("reasonDesc");
		}

		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		local_rec.receiptQty = n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
		DSP_FLD ("qty");
		if (!ValidQuantity (local_rec.receiptQty, inmr_rec.dec_pt))
			return (EXIT_FAILURE);

		if (SERIAL && local_rec.receiptQty != 0.00)
		{
			local_rec.receiptQty = (double) GetSerialNo ();

			heading (2);
			scn_display (2);
			/*------------------------
			| Fix bottom of screen 2 |
			------------------------*/
			line_at (4,1,130);
			if (local_rec.receiptQty <= 0.00)
			{
				if (local_rec.receiptQty == -1.00)
					return (EXIT_FAILURE);

				print_mess (ML(mlPcMess101));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			DSP_FLD ("qty");
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reason"))
	{
		if (F_NOKEY (label ("reason")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExwo (temp_str);
			RewriteScreen ();
			return (EXIT_SUCCESS);
		}

		strcpy (exwo_rec.co_no, comm_rec.co_no);
		strcpy (exwo_rec.code, local_rec.reason);
		if (find_rec (exwo, &exwo_rec, EQUAL, "r"))
		{
			print_mess (ML(mlStdMess163));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.reasonDesc, exwo_rec.description);
		DSP_FLD ("reasonDesc");

		glmrRec.hhmr_hash = exwo_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str, ML(mlPcMess082), exwo_rec.code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.wofAcc, glmrRec.acc_no);

		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK("LL"))
	{
		int		j;
		int		mth_idx;
		long	expiryDate;

		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		/*-----------------------
		| Calculate expiry date |
		-----------------------*/
		expiryDate = comm_rec.inv_date;
		for (j = 0; j < inei_expiry_prd [0]; j++)
		{
			mth_idx = (periodMonth + j) % 12;
			expiryDate += (long) (mnths [mth_idx].no_days);
		}
		
		pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcms_rec.uniq_id = 0;
		cc = find_rec (pcms, &pcms_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)pcms, "DBFIND");			
		
		cc = Load_LL_Lines
			 (
			 	line_cnt,
			 	LL_LOAD_MS,
			 	pcms_rec.pcms_hash,
			 	incc_rec.hhcc_hash,
			 	local_rec.stdUom,
			  	local_rec.cnvFct,
			  	TRUE
			  );
		if (cc)
		{
			cc = DisplayLL
				(										/*----------------------*/
					0,									/*	Line number.		*/
					LOT_ROW + 3 + (line_cnt % TABLINES),/*  Row for window		*/
					LOT_COL + 22,						/*  Col for window		*/
					4,									/*  length for window	*/
					incc_rec.hhwh_hash, 				/*	Warehouse hash.		*/
					local_rec.hhumHash,					/*	UOM hash			*/
					incc_rec.hhcc_hash,					/*	CC hash.			*/
					local_rec.stdUom,					/* UOM					*/
					(float)local_rec.receiptQty,		/* Quantity.			*/
					local_rec.cnvFct,					/* Conversion factor.	*/
					expiryDate,							/* Expiry Date.			*/
					FALSE,								/* Silent mode			*/
					(local_rec.LL[0] == 'Y'),			/* Input Mode.			*/
					inmr_rec.lot_ctrl					/* Lot controled item. 	*/
														/*----------------------*/
				);
		}

		//strcpy (local_rec.LL, "Y");
		/*-----------------
		| Redraw screens. |
		-----------------*/
		heading (1);
		scn_display (1);

		heading (2);
		scn_display (2);
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char *key_val)
{
	_work_open (2,0,60);
	save_rec ("#No", "#Branch Description");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (esmr_rec.co_no, comm_rec.co_no) &&
		!strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		sprintf (err_str,
				"(%-15.15s) %-40.40s",
				esmr_rec.short_name,
				esmr_rec.est_name);
		save_rec (esmr_rec.est_no, err_str);

	    cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)esmr, "DBFIND");
}

void
SrchCcmr (
 char *key_val, 
 char *br_no)
{
	_work_open (2,0,60);
	save_rec ("#No", "#Warehouse Description");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, br_no) &&
		!strncmp (ccmr_rec.cc_no, key_val, strlen (key_val)))
	{
		sprintf (err_str, "(%-9.9s) %-40.40s", ccmr_rec.acronym, ccmr_rec.name);
		save_rec (ccmr_rec.cc_no, err_str);

	    cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");
}

void
SrchExwo (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Description");

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", key_val);
	cc = find_rec (exwo, &exwo_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (exwo_rec.co_no, comm_rec.co_no) &&
		!strncmp (exwo_rec.code, key_val, strlen (key_val)))
	{
		save_rec (exwo_rec.code, exwo_rec.description);

	    cc = find_rec (exwo, &exwo_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", temp_str);
	cc = find_rec (exwo, &exwo_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)exwo, "DBFIND");
}

void
SrchQcmr (
 char *key_val)
{
	_work_open (4,0,40);
	save_rec ("#Cntr", "#QC Centre Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", key_val);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, key_val, strlen (key_val)))
	{
		save_rec (qcmr_rec.centre, qcmr_rec.description);

	    cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	cc = find_rec (qcmr, &qcmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)qcmr, "DBFIND");
}

int
DisplayDetails (
 int flag)
{
	int		i;

	batchFlag = flag;

	switch (pcwo_rec.order_status [0])
	{
	case	'R':
		strcpy (local_rec.status, "STATUS: Released    ");
		break;

	case	'C':
		strcpy (local_rec.status, "STATUS: Closing     ");
		break;

	case	'P':
	case	'F':
	case	'I':
	case	'A':
	case	'D':
		abc_unlock (pcwo);
		print_mess (ML(mlPcMess080));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);

	default:
		abc_unlock (pcwo);
		print_mess (ML(mlPcMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	};

	rv_pr (local_rec.status, 63, 3, FALSE);
	rv_pr (ML(mlPcMess014), 88, 5, FALSE);
	rv_pr (ML(mlPcMess017), 89, 8, FALSE);

	/* find requesting branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, pcwo_rec.req_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)esmr, "DBFIND");
	strcpy (local_rec.reqBrName, esmr_rec.short_name);
	DSP_FLD ("reqBrNo");
	DSP_FLD ("reqBrName");

	/* find receiving branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, pcwo_rec.rec_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)esmr, "DBFIND");
	strcpy (local_rec.recBrName, esmr_rec.short_name);
	DSP_FLD ("recBrNo");
	DSP_FLD ("recBrName");

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");
	strcpy (local_rec.reqWhName, ccmr_rec.acronym);
	DSP_FLD ("reqWhNo");
	DSP_FLD ("reqWhName");

	/* find receiving warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.rec_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.rec_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");
	strcpy (local_rec.recWhName, ccmr_rec.acronym);
	DSP_FLD ("recWhNo");
	DSP_FLD ("recWhName");

	/* find manufacturing warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	/*--------------------------
	| read all default files . |
	--------------------------*/
	if ((cc = ReadDefault (inmr_rec.category)))
		return (EXIT_FAILURE);

	if (flag)
	{
		strcpy (local_rec.orderNo, pcwo_rec.order_no);
		DSP_FLD ("orderNo");
	}
	else
	{
		strcpy (local_rec.batchNo, pcwo_rec.batch_no);
		DSP_FLD ("batchNo");
	}

	local_rec.dateRaised = pcwo_rec.create_date;
	DSP_FLD ("dateRaised");

	local_rec.priority = pcwo_rec.priority;
	DSP_FLD ("priority");

	/*--------------------
	| Lookup inmr record |
	--------------------*/
	abc_selfield (inmr, "inmr_hhbr_hash");
	inmr_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inmr, "DBFIND");

	local_rec.outerSize = inmr_rec.outer_size;
	strcpy (local_rec.item_no, inmr_rec.item_no);
	sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
	sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
	sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
	local_rec.hhbrHash = inmr_rec.hhbr_hash;
	DSP_FLD ("item_no");
	DSP_FLD ("strength");
	DSP_FLD ("desc");
	DSP_FLD ("desc2");
	abc_selfield (inmr, "inmr_id_no");

	if (envVarQcApply)
	{
		if (QCITEM)
			FLD ("qcCentre") = YES;
		else
			FLD ("qcCentre") = NA;
	}
	else
		FLD ("qcCentre") = ND;

	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	
	inum_rec.hhum_hash = inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");
	strcpy (local_rec.stdUom, inum_rec.uom);
	local_rec.hhumHash = inum_rec.hhum_hash;
	local_rec.cnvFct = inum_rec.cnv_fct;

	inum_rec.hhum_hash = inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	strcpy (local_rec.altUom,  (cc) ? "    " : inum_rec.uom);
	DSP_FLD ("stdUom");
	DSP_FLD ("altUom");
	abc_fclose (inum);

	cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
	if (cc)
	{
		ineiRec.std_batch = 1;
		ineiRec.min_batch = 1;
		ineiRec.max_batch = 1;
		ineiRec.prd_multiple = 0;
	}
	if (ineiRec.std_batch == 0.00)
		ineiRec.std_batch = 1.00;
	local_rec.stdBatch	= ineiRec.std_batch;
	local_rec.minBatch	= ineiRec.min_batch;
	local_rec.maxBatch	= ineiRec.max_batch;
	local_rec.prodMult	= ineiRec.prd_multiple;
	sprintf (local_rec.stdBatchString, "%14.6f", ineiRec.std_batch);

	print_at (9,  69, "%s %14.6f", ML(mlPcMess018), local_rec.stdBatch);
	print_at (9,  90, "%s %14.6f", ML(mlPcMess019), local_rec.minBatch);
	print_at (9, 111, "%s %14.6f", ML(mlPcMess020), local_rec.maxBatch);

	/*-------------------------------------------
	| Get hhwh_hash for Manufacturing Warehouse |
	-------------------------------------------*/
	incc_rec.hhcc_hash = pcwo_rec.hhcc_hash;
	incc_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	if (FindIncc ())
		file_err (cc, (char *)incc, "DBFIND");
	local_rec.mfgHhwhHash = incc_rec.hhwh_hash;

	strcpy (local_rec.oldRecBr, pcwo_rec.rec_br_no);
	strcpy (local_rec.oldRecWh, pcwo_rec.rec_wh_no);

	/*---------------------------
	| Process control Accounts. |
	---------------------------*/
	if (GetAccount  (inmr_rec.category))
		return (EXIT_FAILURE);

	/*------------------------------------------
	| Look up to see if item is on Cost Centre |
	------------------------------------------*/
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	if (FindIncc ())
	{
		i = prmptmsg (ML(mlStdMess033),"YyNn",1,20);
		print_at (20, 0, "                                                             ");
		if (i == 'n' || i == 'N')
		{
			skip_entry = -1 ;
			return (EXIT_SUCCESS);
		}
		else
		{
			cc = AddIncc (ccmr_rec.hhcc_hash);
			if (cc)
				file_err (cc, (char *)incc, "DBADD");
		}
	}

	local_rec.bom_alt = pcwo_rec.bom_alt;
	local_rec.bom_alt_old = pcwo_rec.bom_alt;
	DSP_FLD ("bom_alt");

	local_rec.rtg_alt = pcwo_rec.rtg_alt;
	local_rec.rtg_alt_old = pcwo_rec.rtg_alt;
	DSP_FLD ("rtg_alt");

	local_rec.rqd_dte = pcwo_rec.reqd_date;
	DSP_FLD ("rqd_dte");

	local_rec.qty_rqd = n_dec (pcwo_rec.prod_qty, inmr_rec.dec_pt);
	DSP_FLD ("qty_rqd");

	local_rec.previousReceipt = n_dec (pcwo_rec.act_prod_qty, 
		inmr_rec.dec_pt);
	DSP_FLD ("previousReceipt");

	local_rec.previousReject = n_dec (pcwo_rec.act_rej_qty, 
		inmr_rec.dec_pt);
	DSP_FLD ("previousReject");

	/*--------------------------------------------------
	| Look up to see if item is on Receiving Warehouse |
	--------------------------------------------------*/
	incc_rec.hhcc_hash = local_rec.currHhcc;
	incc_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	if (FindIncc ())
	{
		i = prmptmsg (ML(mlStdMess033),"YyNn",1,20);
		move (1, 20);
		cl_line ();
		if (i == 'n' || i == 'N')
		{
			skip_entry = -1 ;
			return (EXIT_SUCCESS);
		}
		else
		{
			cc = AddIncc (local_rec.currHhcc);
			if (cc)
				file_err (cc, (char *)incc, "DBADD");
		}
		local_rec.hhwhHash = incc_rec.hhwh_hash;
	}

	entry_exit = TRUE;
	return (EXIT_SUCCESS);
}

void
RewriteScreen (
 void)
{
	heading (1);
	scn_display (1);
	rv_pr (local_rec.status, 63, 3, FALSE);
	rv_pr (ML(mlPcMess014), 88, 5, FALSE);
	rv_pr (ML(mlPcMess017), 89, 8, FALSE);
	print_at (9, 69, "%s %14.6f", ML(mlPcMess018), local_rec.stdBatch);
	print_at (9, 90, "%s %14.6f", ML(mlPcMess019), local_rec.minBatch);
	print_at (9,111, "%s %14.6f", ML(mlPcMess020), local_rec.maxBatch);
	heading (2);
	scn_display (2);
}

static int
ValidateDate (
	long 	inputDate)
{
	if (inputDate < MonthStart  (comm_rec.inv_date) ||
			inputDate > MonthEnd (comm_rec.inv_date))
	{
		return print_err (ML(mlPcMess078), DateToString (comm_rec.inv_date));
	}
	return (EXIT_SUCCESS);
}

/*------------------------------
| Get serial numbers for items |
| being receipted.             |
------------------------------*/
float	
GetSerialNo (void)
{
	int	i;

	tab_col = 94;
	tab_row = 15;

	if (prog_status == ENTRY)
	{
		init_vars (3);
		lcount [3] = 0;
	}

	scn_set (3);

	heading (3);
	scn_display (3);
	edit (3);

	for (i = 15; i < 23; i++)
	{
		move (90, i);
		cl_line ();
	}

	if (restart)
		return (-1.00);

	return ((float)lcount [3]);
}

/*------------------------------
| Calculate cost of production |
| based on materials and cost  |
| of time through routing.     |
------------------------------*/
void
CalcCost (
 void)
{
	double	stdDir, stdFix,
			actDir, actFix,
			tmp_val = 0.00;
	float	cnvFct	= 0.00;
	long	stdTime, actTime;

	memset (&stdCostRec, 0, sizeof (stdCostRec));
	memset (&actCostRec, 0, sizeof (actCostRec));
	memset (&recCostRec, 0, sizeof (recCostRec));

	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	/*-----------------------
	| Process pcms records. |
	-----------------------*/
	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*-----------------------
		| Find item master file |
		-----------------------*/
		inmr2_rec.hhbr_hash = pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			inmr2_rec.outer_size = 1.00;

		/*----------------------------------------------------
		| Find item branch file at the manufacturing branch. | 
		----------------------------------------------------*/
		inei2Rec.hhbr_hash = pcms_rec.mabr_hash;
		strcpy (inei2Rec.est_no, pcwo_rec.br_no);
		cc = find_rec (inei, &inei2Rec, COMPARISON, "r");
		if (cc)
			inei2Rec.std_cost = 0.00;

		cnvFct = GetUom (pcms_rec.uom, inmr2_rec.alt_uom);
		inei2Rec.std_cost /= cnvFct;
		pcms_rec.matl_wst_pc += 100.00;
		pcms_rec.matl_wst_pc /= 100.00;

		/*-------------------------
		| Standard material cost. |
		-------------------------*/
		tmp_val = out_cost (inei2Rec.std_cost, inmr2_rec.outer_size);
		tmp_val *= (pcms_rec.matl_qty * pcms_rec.matl_wst_pc);
		stdCostRec.matCost += CENTS (tmp_val);

		/*-----------------------
		| Actual material cost. |
		-----------------------*/
		actCostRec.matCost += pcms_rec.amt_issued;

		/*-----------------------------------
		| Receipted material cost - so far. |
		-----------------------------------*/
		recCostRec.matCost += pcms_rec.amt_recptd;

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	/*-----------------------
	| Process pcln records. |
	-----------------------*/
	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no		= 0;
	pcln_rec.line_no	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		stdTime = 0L;
		stdDir = stdFix = 0.00;

		/*----------------
		| Standard time. |
		----------------*/
		stdTime =  pcln_rec.setup;
		stdTime += pcln_rec.run;
		stdTime += pcln_rec.clean;

		if (stdTime)
		{
			/*-----------------------
			| Standard direct cost. |
			-----------------------*/
			stdDir = (double) stdTime * pcln_rec.rate;
			stdDir /= 60.00;
			stdDir *= (double) pcln_rec.qty_rsrc;

			/*-------------------------
			| Standard overhead cost. |
			-------------------------*/
			stdFix = (double) stdTime * pcln_rec.ovhd_var;
			stdFix /= 60.00;
			stdFix += pcln_rec.ovhd_fix;
			stdFix *= (double) pcln_rec.qty_rsrc;
		}

		actTime = 0L;
		pcrq_rec.hhwo_hash = pcln_rec.hhwo_hash;
		pcrq_rec.seq_no = pcln_rec.seq_no;
		pcrq_rec.line_no = pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
		while (!cc &&
			pcrq_rec.hhwo_hash == pcln_rec.hhwo_hash &&
			pcrq_rec.seq_no == pcln_rec.seq_no &&
			pcrq_rec.line_no == pcln_rec.line_no)
		{
			/*--------------
			| Actual time. |
			--------------*/
			actTime += pcrq_rec.act_setup;
			actTime += pcrq_rec.act_run;
			actTime += pcrq_rec.act_clean;

			cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
		}

		/*------------------
		| Actual line cost |
		------------------*/
		actDir = (double) actTime * pcln_rec.rate;
		actDir /= 60.00;
		actDir *= pcln_rec.qty_rsrc;

		/*------------------------
		| Actual overhead costs. |
		------------------------*/
		actFix = (double) actTime * pcln_rec.ovhd_var;
		actFix /= 60.00;
		actFix += pcln_rec.ovhd_fix;
		actFix *= (double) pcln_rec.qty_rsrc;

		/*---------------------------------------------------------
		| Check resource type, and add to appropriate total cost. |
		---------------------------------------------------------*/
		rgrs_rec.hhrs_hash	=	pcln_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (!cc)
		{
			switch (rgrs_rec.type [0])
			{
			case	'L': /* Labour */
				stdCostRec.dLabour += stdDir;
				stdCostRec.fLabour += stdFix;
				actCostRec.dLabour += actDir;
				actCostRec.fLabour += actFix;
				recCostRec.dLabour += (	pcln_rec.amt_recptd -
										pcln_rec.ovh_recptd);
				recCostRec.fLabour += pcln_rec.ovh_recptd;
				break;
			case	'M': /* Machine */
				stdCostRec.dMachine += stdDir;
				stdCostRec.fMachine += stdFix;
				actCostRec.dMachine += actDir;
				actCostRec.fMachine += actFix;
				recCostRec.dMachine += (pcln_rec.amt_recptd -
										pcln_rec.ovh_recptd);
				recCostRec.fMachine += pcln_rec.ovh_recptd;
				break;
			case	'Q': /* QC-Check */
				stdCostRec.dQcCheck += stdDir;
				stdCostRec.fQcCheck += stdFix;
				actCostRec.dQcCheck += actDir;
				actCostRec.fQcCheck += actFix;
				recCostRec.dQcCheck += (pcln_rec.amt_recptd -
										pcln_rec.ovh_recptd);
				recCostRec.fQcCheck += pcln_rec.ovh_recptd;
				break;
			case	'S': /* Special */
				stdCostRec.dSpecial += stdDir;
				stdCostRec.fSpecial += stdFix;
				actCostRec.dSpecial += actDir;
				actCostRec.fSpecial += actFix;
				recCostRec.dSpecial += (pcln_rec.amt_recptd -
										pcln_rec.ovh_recptd);
				recCostRec.fSpecial += pcln_rec.ovh_recptd;
				break;
			case	'O': /* Other */
				stdCostRec.dOther += stdDir;
				stdCostRec.fOther += stdFix;
				actCostRec.dOther += actDir;
				actCostRec.fOther += actFix;
				recCostRec.dOther += (pcln_rec.amt_recptd -
										pcln_rec.ovh_recptd);
				recCostRec.fOther += pcln_rec.ovh_recptd;
				break;
			}
		}
		
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	local_rec.standardUnitCost  = stdCostRec.matCost;
	local_rec.standardUnitCost += (stdCostRec.dLabour   + stdCostRec.fLabour);
	local_rec.standardUnitCost += (stdCostRec.dMachine  + stdCostRec.fMachine);
	local_rec.standardUnitCost += (stdCostRec.dQcCheck + stdCostRec.fQcCheck);
	local_rec.standardUnitCost += (stdCostRec.dSpecial  + stdCostRec.fSpecial);
	local_rec.standardUnitCost += (stdCostRec.dOther    + stdCostRec.fOther);

	local_rec.actCost  = actCostRec.matCost;
	local_rec.actCost += (actCostRec.dLabour   + actCostRec.fLabour);
	local_rec.actCost += (actCostRec.dMachine  + actCostRec.fMachine);
	local_rec.actCost += (actCostRec.dQcCheck + actCostRec.fQcCheck);
	local_rec.actCost += (actCostRec.dSpecial  + actCostRec.fSpecial);
	local_rec.actCost += (actCostRec.dOther    + actCostRec.fOther);
	abc_fclose (inmr2);
}

float	
GetUom (
	long 	hhumHash,
	long 	altUom)
{
	char	std_group [21],
			alt_group [21];
	number	stdCnvFct,
			altCnvFct,
			cnvFct,
			result,
			uom_cfactor;

	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	inum_rec.hhum_hash = altUom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");

	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&altCnvFct, inum_rec.cnv_fct);

	inum_rec.hhum_hash = inmr2_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");

	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&stdCnvFct, inum_rec.cnv_fct);

	inum_rec.hhum_hash = hhumHash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");

	abc_fclose (inum);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnvFct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&stdCnvFct, &cnvFct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr2_rec.uom_cfactor);
		NumDiv (&altCnvFct, &cnvFct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

int
DeleteLine (void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML(mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [3]--;
	for (i = line_cnt; line_cnt < lcount [3]; line_cnt++)
	{
		getval (line_cnt + 1);
		sprintf (store [line_cnt].validSerial, "%-25.25s", store [line_cnt + 1].validSerial);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

/*===============================
| add transaction to incc files |
===============================*/
int
Update (
 void)
{
	int		i;

	double	old_qty = 0,
			xx_qty = 0,
			old_cost = 0,
			tmpCost,
			tot_cost,
			UpdatePcms (int closeJob),
			UpdatePcln (int closeJob);

	int		NoLots		=	TRUE;

	int		closeJob;

	long	ff_date = 0L;

	thisReceipt = local_rec.receiptQty + local_rec.rejectQty;
	prevReceipt = n_dec (pcwo_rec.act_prod_qty, inmr_rec.dec_pt) +
			n_dec (pcwo_rec.act_rej_qty, inmr_rec.dec_pt);

	closeJob = FALSE;
	if ((thisReceipt + prevReceipt) >=
		n_dec (pcwo_rec.prod_qty, inmr_rec.dec_pt))
		closeJob = TRUE;

	if (JOB_CLOSING || closeJob)
	{
		i = prmptmsg (ML(mlPcMess079),"YyNnAa", 40, 2);
		if (i == 'Y' || i == 'y')
		{
			if (closeJob)
			{
				open_rec (pcat, pcat_list, PCAT_NO_FIELDS, "pcat_hhwo_hash");

				pcat_rec.hhwo_hash = pcwo_rec.hhwo_hash;
				cc = find_rec (pcat, &pcat_rec, GTEQ, "r");
				while (!cc && pcat_rec.hhwo_hash == pcwo_rec.hhwo_hash)
				{
					if (pcat_rec.stat_flag [0] != 'U')
					{
						print_mess (ML(mlPcMess081));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
					cc = find_rec (pcat, &pcat_rec, NEXT, "r");
				}
				abc_fclose (pcat);
			}

			/* calculate actual cost */
			if (pcwo_rec.act_prod_qty == 0.00)
				local_rec.actCost /= local_rec.receiptQty;
			else
				local_rec.actCost /= (n_dec (pcwo_rec.act_prod_qty,
						inmr_rec.dec_pt) + local_rec.receiptQty);
			local_rec.actCost *= local_rec.outerSize;

			closeJob = TRUE;
		}
		else
		{
			closeJob = FALSE;
			if (i == 'A' || i == 'a')
				return (EXIT_SUCCESS);
		}
	}

	clear ();

	/*--------------------------------------------------
	| Update all inventory and general ledger records. |
	--------------------------------------------------*/
	old_qty = 0;
	xx_qty = 0;
	old_cost = 0;

	putchar ('.');
	fflush (stdout);

	/*-------------------------------------------------
	| Find inmr record from item number in structure. |
	-------------------------------------------------*/
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,local_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, (char *)inmr, "DBFIND");
	if (inmr_rec.hhsi_hash != 0L)
		inmr_rec.hhbr_hash = inmr_rec.hhsi_hash;

	/*----------------------------------------------
	| Find warehouse record from master item hash. |
	----------------------------------------------*/
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = FindIncc ();
	if (cc)
		file_err (cc, (char *)incc, "DBFIND");
	local_rec.hhwhHash = incc_rec.hhwh_hash;

	/*-------------------------------------------------------
	| update inventory cost centre stock record (file incc)	|
	-------------------------------------------------------*/
	incc_rec.receipts  	+= (float)n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
	incc_rec.ytd_receipts += (float)n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
	if (envVarQcApply && QCITEM)
		incc_rec.qc_qty	+= (float)n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
	old_qty = n_dec (incc_rec.closing_stock, inmr_rec.dec_pt);
	xx_qty = n_dec (incc_rec.closing_stock, inmr_rec.dec_pt);
	incc_rec.closing_stock = (float) (n_dec (incc_rec.opening_stock,
				  inmr_rec.dec_pt)
				  + n_dec (incc_rec.pur, inmr_rec.dec_pt)
				  + n_dec (incc_rec.receipts, inmr_rec.dec_pt)
				  + n_dec (incc_rec.adj, inmr_rec.dec_pt)
				  - n_dec (incc_rec.issues, inmr_rec.dec_pt)
				  - n_dec (incc_rec.sales, inmr_rec.dec_pt));

	/*--------------------------
	| Update warehouse record. |
	--------------------------*/
	cc = abc_update (incc,&incc_rec);
	if (cc)
		file_err (cc, (char *)incc, "DBUPDATE");

	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	/*--------------------------------------
	| Find Warehouse unit of measure file. |
	--------------------------------------*/
	inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
	if (cc)
	{
		memset (&inwu_rec, 0, sizeof (inwu_rec));
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = abc_add (inwu, &inwu_rec);
		if (cc)
			file_err (cc, (char *)inwu, "DBADD");

		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
			file_err (cc, (char *)inwu, "DBFIND");
	}
	inwu_rec.receipts  += (float)n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
	inwu_rec.closing_stock = inwu_rec.opening_stock +
							 inwu_rec.pur +
							 inwu_rec.receipts +
							 inwu_rec.adj -
							 inwu_rec.issues -
							 inwu_rec.sales;

	cc = abc_update(inwu,&inwu_rec);
	if (cc)
		file_err(cc, "inwu", "DBUPDATE");

	abc_fclose (inwu);

	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	PrintDetails (closeJob);

	if (local_rec.receiptQty != 0.00)
	{
		/*-------------------------------------------
		| Find branch record from master item hash. |
		-------------------------------------------*/
		ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (ineiRec.est_no, pcwo_rec.rec_br_no);
		cc = find_rec (inei, &ineiRec, COMPARISON, "u");
		if (cc)
			file_err (cc, (char *)inei, "DBFIND");

		old_cost = ineiRec.avge_cost;

		if (old_qty < 0.00)
			xx_qty = 0.00;

		if (old_qty + local_rec.receiptQty == 0.00)
			ineiRec.avge_cost = twodec (DOLLARS (local_rec.actCost));
		else
		{
			if (old_qty + local_rec.receiptQty < 0.00)
				ineiRec.avge_cost = 0.00;
			else
			{
				ineiRec.avge_cost = twodec (((xx_qty * old_cost) + 
						(local_rec.receiptQty * DOLLARS  (local_rec.actCost))) / 
						(xx_qty + local_rec.receiptQty));
			}
		}
		if (closeJob)
		{
			ineiRec.prev_cost = ineiRec.last_cost;
			ineiRec.last_cost = twodec (DOLLARS (local_rec.actCost));
			ineiRec.date_lcost = comm_rec.inv_date;
		}

		/*--------------------------
		| Update branch records.   |
		--------------------------*/
		cc = abc_update (inei,&ineiRec);
		if (cc)
			file_err (cc, (char *)inei, "DBUPDATE");

		switch (inmr_rec.costing_flag [0])
		{
		case 'F':
		case 'I':
			ff_date = local_rec.receiptDate;

			/*-------------------------------------------
			| Product is in stock take mode so control  |
			| record  needs to be checked.              |
			-------------------------------------------*/
			if (incc_rec.stat_flag [0] >= 'A' && incc_rec.stat_flag [0] <= 'Z')
			{
				while (CheckInsc (incc_rec.hhcc_hash,ff_date,incc_rec.stat_flag))
					ff_date += 1L;
			}
			/*-------------------------------------------------------
			| Add FIFO record for the mfg branch and warehouse.     |
			-------------------------------------------------------*/
			cc	=	AddIncf 
					(
						local_rec.mfgHhwhHash,
						ff_date,
						DOLLARS (local_rec.standardUnitCost),
						DOLLARS (local_rec.standardUnitCost),
						(float) local_rec.receiptQty,
						pcwo_rec.batch_no,
						DOLLARS (local_rec.standardUnitCost),
						0.00,
						0.00,
						0.00,
						0.00,
						DOLLARS (local_rec.standardUnitCost),
						"A"
					);
			if (cc)
				file_err (cc, incf, "DBADD");

			if (closeJob)
			{
				UpdateFifo 
				(
					local_rec.mfgHhwhHash,
					(float) twodec (DOLLARS (local_rec.actCost)),
					(float) twodec (DOLLARS (local_rec.standardUnitCost))
				);
			}
			if (local_rec.mfgHhwhHash != local_rec.hhwhHash)
			{
				cc	=	AddIncf 
						(
							local_rec.hhwhHash,
							ff_date,
							DOLLARS (local_rec.standardUnitCost),
							DOLLARS (local_rec.standardUnitCost),
							(float) local_rec.receiptQty,
							pcwo_rec.batch_no,
							DOLLARS (local_rec.standardUnitCost),
							0.00,
							0.00,
							0.00,
							0.00,
							DOLLARS (local_rec.standardUnitCost),
							"A"
						);
				if (cc)
					file_err (cc, incf, "DBADD");

				if (closeJob)
				{
					UpdateFifo 
					(
						local_rec.hhwhHash,
						(float) twodec (DOLLARS (local_rec.actCost)),
						(float) twodec (DOLLARS (local_rec.standardUnitCost))
					);
				}
			}
			break;

		default:
			break;
		}

		if (inmr_rec.serial_item [0] == 'Y')
		{
			scn_set (3);
			for (i = 0; i < lcount [3]; i++)
			{
				getval (i);
				AddSerial ();
			}
		}
	}

	inmr_rec.on_hand += (float) n_dec (local_rec.receiptQty, inmr_rec.dec_pt);
	if (envVarQcApply && QCITEM)
		inmr_rec.qc_qty += (float) n_dec (local_rec.receiptQty,
				inmr_rec.dec_pt);

	/*----------------------------------
	| Update inventory master records. |
	----------------------------------*/
	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, (char *)inmr, "DBUPDATE");

	NoLots = TRUE;

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (line_cnt, i))
			break;

		UpdateLotLocation (0, FALSE);

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
			inmr_rec.std_uom,
			local_rec.receiptDate,
			10,
			local_rec.orderNo,
			inmr_rec.inmr_class,
			inmr_rec.category,
			local_rec.orderNo,
			local_rec.batchNo,
			GetBaseQty (0, i),
			0.00,
			local_rec.actCost
		);

		if (strcmp (local_rec.oldRecBr, pcwo_rec.rec_br_no) ||
			strcmp (local_rec.oldRecWh, pcwo_rec.rec_wh_no))
			TransStock ((float) local_rec.receiptQty);

		if (envVarQcApply && QCITEM)
			UpdateQchr (GetBaseQty (0, i), GetINLO (0, i));
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
			inmr_rec.std_uom,
			local_rec.receiptDate,
			10,
			local_rec.orderNo,
			inmr_rec.inmr_class,
			inmr_rec.category,
			local_rec.orderNo,
			local_rec.batchNo,
			(float)local_rec.receiptQty,
			0.00,
			local_rec.actCost
		);
		if (strcmp (local_rec.oldRecBr, pcwo_rec.rec_br_no) ||
			strcmp (local_rec.oldRecWh, pcwo_rec.rec_wh_no))
			TransStock ((float)local_rec.receiptQty);

		if (envVarQcApply && QCITEM)
			UpdateQchr ((float)local_rec.receiptQty, 0L);
	}

	tot_cost = UpdatePcms (closeJob);
	tot_cost += UpdatePcln (closeJob);

	/*--------------------------------------------
	| Add General Ledger inventory transactions. |
	--------------------------------------------*/
	if (UPD_GL)
	{
		float	cnvFct;

		open_rec (pcgl, pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
		memset (&thisRec, 0, sizeof (thisRec));

		/* post this receipt cost to Mfg item stock account */
		AddPcgl 
		(
			local_rec.creditHhmrHash,
			local_rec.creditAccount,
			"RECPT ",
			(tot_cost > 0.00) ? tot_cost : -tot_cost,
			(tot_cost > 0.00) ? "1" : "2",
			"19"
		);

		cnvFct = (float)(thisReceipt /
				n_dec (pcwo_rec.prod_qty, inmr_rec.dec_pt));

		/* post std cost of materials to the material account */
		tmpCost = no_dec (stdCostRec.matCost * cnvFct);
		thisRec.matCost = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			matHash [0],
			matAcc [0],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);

		/* post std cost of direct labour to the direct labour account */
		tmpCost = no_dec (stdCostRec.dLabour * cnvFct);
		thisRec.dLabour = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			dirHash [0],
			dirAcc [0],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);
		/* post std cost of fixed labour to the fixed labour account */
		tmpCost = no_dec (stdCostRec.fLabour * cnvFct);
		thisRec.fLabour = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			fixHash [0],
			fixAcc [0],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);

		/* post std cost of direct machine to the direct machine account */
		tmpCost = no_dec (stdCostRec.dMachine * cnvFct);
		thisRec.dMachine = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			dirHash [1],
			dirAcc [1],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);
		/* post std cost of fixed machine to the fixed machine account */
		tmpCost = no_dec (stdCostRec.fMachine * cnvFct);
		thisRec.fMachine = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			fixHash [1],
			fixAcc [1],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);

		/* post std cost of direct qc_check to the direct qc_check account */
		tmpCost = no_dec (stdCostRec.dQcCheck * cnvFct);
		thisRec.dQcCheck = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			dirHash [2],
			dirAcc [2],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);
		/* post std cost of fixed qc_check to the fixed qc_check account */
		tmpCost = no_dec (stdCostRec.fQcCheck * cnvFct);
		thisRec.fQcCheck = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl
(fixHash [2],
			fixAcc [2],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19");

		/* post std cost of direct special to the direct special account */
		tmpCost = no_dec (stdCostRec.dSpecial * cnvFct);
		thisRec.dSpecial = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			dirHash [3],
			dirAcc [3],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);
		/* post std cost of fixed special to the fixed special account */
		tmpCost = no_dec (stdCostRec.fSpecial * cnvFct);
		thisRec.fSpecial = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			fixHash [3],
			fixAcc [3],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);

		/* post std cost of direct other to the direct other account */
		tmpCost = no_dec (stdCostRec.dOther * cnvFct);
		thisRec.dOther = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			dirHash [4],
			dirAcc [4],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);
		/* post std cost of fixed other to the fixed other account */
		tmpCost = no_dec (stdCostRec.fOther * cnvFct);
		thisRec.fOther = tmpCost;
		tot_cost -= tmpCost;
		AddPcgl 
		(
			fixHash [4],
			fixAcc [4],
			pcwc_rec.work_cntr,
			(tmpCost > 0.00) ? tmpCost : -tmpCost,
			(tmpCost > 0.00) ? "2" : "1",
			"19"
		);

		/*------------------------------
		| Write-Off rejected quantity. |
		------------------------------*/
		if (local_rec.rejectQty > 0.00)
		{
			double	value =
					out_cost (local_rec.standardUnitCost, inmr_rec.outer_size);

			strcpy (glmrRec.co_no,	comm_rec.co_no);
			strcpy (glmrRec.acc_no,	local_rec.wofAcc);
			if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
				file_err (cc, glmr, "DBFIND");

			AddPcgl 
			(
				glmrRec.hhmr_hash,
				glmrRec.acc_no,
				pcwc_rec.work_cntr,
				local_rec.rejectQty * value,
				"1",
				"12"
			);
			AddPcgl 
			(
				local_rec.creditHhmrHash,
				local_rec.creditAccount,
				pcwc_rec.work_cntr,
				local_rec.rejectQty * value,
				"2",
				"12"
			);
		}

		/*--------------------------------------------------------------
		| If closeJob, post difference between the std and the actual  |
		| cost to the manufacturing variance accounts.                 |
		| The variance is the actual cost less the already receipted   |
		| costs plus this receipted costs.                             |
		--------------------------------------------------------------*/
		if (closeJob) 
		{
			/* post mfg variance materials to the material account */
			tmpCost = no_dec (thisRec.matCost + recCostRec.matCost);
			tmpCost = no_dec (actCostRec.matCost - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl 
			(
				matHash [1],
				matAcc [1],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post mfg variance direct labour to the direct labour account */
			tmpCost = no_dec (thisRec.dLabour + recCostRec.dLabour);
			tmpCost = no_dec (actCostRec.dLabour - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl 
			(
				mfgDHash [0],
				mfgDAcc [0],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);
			/* post mfg variance fixed labour to the fixed labour account */
			tmpCost = no_dec (thisRec.fLabour + recCostRec.fLabour);
			tmpCost = no_dec (actCostRec.fLabour - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgFHash [0],
				mfgFAcc [0],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post mfg variance direct machine to the direct machine account */
			tmpCost = no_dec (thisRec.dMachine + recCostRec.dMachine);
			tmpCost = no_dec (actCostRec.dMachine - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgDHash [1],
				mfgDAcc [1],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);
			/* post mfg variance fixed machine to the fixed machine account */
			tmpCost = no_dec (thisRec.fMachine + recCostRec.fMachine);
			tmpCost = no_dec (actCostRec.fMachine - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgFHash [1],
				mfgFAcc [1],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post mfg var direct qc_check to the direct qc_check account */
			tmpCost = no_dec (thisRec.dQcCheck + recCostRec.dQcCheck);
			tmpCost = no_dec (actCostRec.dQcCheck - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgDHash [2],
				mfgDAcc [2],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);
			/* post mfg variance fixed qc_check to the fixed qc_check account */
			tmpCost = no_dec (thisRec.fQcCheck + recCostRec.fQcCheck);
			tmpCost = no_dec (actCostRec.fQcCheck - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgFHash [2],
				mfgFAcc [2],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post mfg variance direct special to the direct special account */
			tmpCost = no_dec (thisRec.dSpecial + recCostRec.dSpecial);
			tmpCost = no_dec (actCostRec.dSpecial - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgDHash [3],
				mfgDAcc [3],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);
			/* post mfg variance fixed special to the fixed special account */
			tmpCost = no_dec (thisRec.fSpecial + recCostRec.fSpecial);
			tmpCost = no_dec (actCostRec.fSpecial - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgFHash [3],
				mfgFAcc [3],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post mfg variance direct other to the direct other account */
			tmpCost = no_dec (thisRec.dOther + recCostRec.dOther);
			tmpCost = no_dec (actCostRec.dOther - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgDHash [4],
				mfgDAcc [4],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);
			/* post mfg variance fixed other to the fixed other account */
			tmpCost = no_dec (thisRec.fOther + recCostRec.fOther);
			tmpCost = no_dec (actCostRec.fOther - tmpCost);
			tot_cost -= tmpCost;
			AddPcgl
			(
				mfgFHash [4],
				mfgFAcc [4],
				pcwc_rec.work_cntr,
				(tmpCost > 0) ? tmpCost : -tmpCost,
				(tmpCost > 0) ? "2" : "1",
				"19"
			);

			/* post total act cost */
			AddPcgl
			(
				local_rec.creditHhmrHash,
				local_rec.creditAccount,
				pcwc_rec.work_cntr,
				((local_rec.actCost / inmr_rec.outer_size) *
					(local_rec.receiptQty + pcwo_rec.act_prod_qty)),
				"1",
				"19"
			);
			/* post total std cost */
			AddPcgl
			(
				local_rec.creditHhmrHash,
				local_rec.creditAccount,
				pcwc_rec.work_cntr,
				((local_rec.standardUnitCost / inmr_rec.outer_size) *
					(local_rec.receiptQty + pcwo_rec.act_prod_qty)),
				"2",
				"19"
			);
		}
		abc_fclose (pcgl);
	}

	/*-------------------------------------------
	| Update act_prod_qty & act_rej_qty on pcwo |
	-------------------------------------------*/
	pcwo_rec.act_prod_qty += (float) local_rec.receiptQty;
	pcwo_rec.act_rej_qty += (float) local_rec.rejectQty;

	/*-------------------------------
	| Close job if status is "C"	|
	| AND user has requested close.	|
	-------------------------------*/
	if (closeJob)
		strcpy (pcwo_rec.order_status, "Z");

	if (batchFlag)
		cc = abc_update (pcwo2, &pcwo_rec);
	else
		cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, (char *)pcwo, "DBUPDATE");

	if (closeJob)
	{
		open_rec (pchs, pchs_list, PCHS_NO_FIELDS, "pchs_hhwo_hash");
		strcpy (pchs_rec.co_no,		pcwo_rec.co_no);
		strcpy (pchs_rec.br_no,		pcwo_rec.br_no);
		strcpy (pchs_rec.wh_no,		pcwo_rec.wh_no);
		strcpy (pchs_rec.order_no,	pcwo_rec.order_no);
		strcpy (pchs_rec.batch_no,	pcwo_rec.batch_no);
		pchs_rec.hhwo_hash			= pcwo_rec.hhwo_hash;
		pchs_rec.hhbr_hash			= pcwo_rec.hhbr_hash;
		pchs_rec.prod_qty			= pcwo_rec.prod_qty;
		pchs_rec.act_prod_qty		= pcwo_rec.act_prod_qty;
		pchs_rec.act_rej_qty		= pcwo_rec.act_rej_qty;
		pchs_rec.batch_size			= ineiRec.std_batch;
		pchs_rec.outer_size			= inmr_rec.outer_size;
		pchs_rec.std_cost			= DOLLARS (local_rec.standardUnitCost);
		pchs_rec.act_cost			= DOLLARS (local_rec.actCost);
		pchs_rec.bom_no				= pcwo_rec.bom_alt;
		pchs_rec.rtg_no				= pcwo_rec.rtg_alt;

		cc = abc_add (pchs, &pchs_rec);
		if (cc)
			file_err (cc, (char *)pchs, "DBFIND");
		abc_fclose (pchs);
	}
	if (closeJob && pcwo_rec.hhsl_hash > 0L && envVarSoWoAllowed)
		ProcessSalesOrder (pcwo_rec.hhsl_hash);

	/*-----------------------------------
	| add sobg record for recalculation |
	| of final product on order qty     |
	-----------------------------------*/
	add_hash 
	(
		pcwo_rec.co_no, /* re-calc manufacturing warehouse */
		pcwo_rec.br_no,
		"RP",
		0,
		pcwo_rec.hhbr_hash,
		pcwo_rec.hhcc_hash,
		0L,
		(double) 0
	);
 
	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	add_hash (pcwo_rec.co_no,  /* re-calc requesting warehouse */
			pcwo_rec.br_no,
			"RP",
			0,
			pcwo_rec.hhbr_hash,
			ccmr_rec.hhcc_hash,
			0L,
			(double) 0);  /* decrease on order qty for item produced */

	recalc_sobg ();

	strcpy (local_rec.prevWoNo, pcwo_rec.order_no);
	return (EXIT_SUCCESS);
}

/*===================================================
| Adds a inventory QC purchase items reveival file. |
===================================================*/
void
UpdateQchr (
 float	qcQty,
 long 	inloHash)
{
	float	qcDays;

	open_rec (qchr, qchr_list, QCHR_NO_FIELDS, "qchr_id_no");

	incc_rec.hhcc_hash = local_rec.currHhcc;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	if (find_rec (incc, &incc_rec, EQUAL, "r"))
		incc_rec.qc_time = 0.00;
	qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

	strcpy (qchr_rec.co_no,			comm_rec.co_no);
	strcpy (qchr_rec.br_no,			comm_rec.est_no);
	strcpy (qchr_rec.wh_no,			comm_rec.cc_no);
	strcpy (qchr_rec.qc_centre,		local_rec.qcCentre);
	qchr_rec.hhbr_hash				= inmr_rec.hhbr_hash;
	qchr_rec.origin_qty				= qcQty;
	qchr_rec.receipt_dt				= local_rec.receiptDate;
	qchr_rec.exp_rel_dt				= local_rec.receiptDate + (long) qcDays;
	qchr_rec.rel_qty				= 0.00;
	qchr_rec.rej_qty				= 0.00;

	if (LOT_CTRL)
	{
		int		i, mthIdx;
		long	expiryDate = comm_rec.inv_date;
		/*-----------------------
		| Calculate expiry date |
		-----------------------*/
		for (i = 0; i < inei_expiry_prd [0]; i++)
		{
			mthIdx = (periodMonth + i) % 12;
			expiryDate += (long) (mnths [mthIdx].no_days);
		}
	}

	qchr_rec.inlo_hash = inloHash;
	if (SERIAL)
		sprintf (qchr_rec.serial_no, "%-25.25s", local_rec.serialNo);
	else
		strcpy (qchr_rec.serial_no,	" ");
	strcpy (qchr_rec.ref_1,			pcwo_rec.order_no);
	strcpy (qchr_rec.ref_2,			pcwo_rec.batch_no);
	qchr_rec.next_seq				= 0;
	strcpy (qchr_rec.source_type,	"W");

	if (abc_add (qchr, &qchr_rec))
		file_err (cc, (char *)qchr, "DBFIND");
	abc_fclose (qchr);
}

/*=======================================================
| Update actual costs to all related fifo records.      |
=======================================================*/
void
UpdateFifo (
	long	hhwhHash, 
	float	actCost, 
	float	stdCost)
{
	abc_selfield (incf, "incf_id_no_2");

	incfRec.hhwh_hash = hhwhHash;
	strncpy (incfRec.gr_number, pcwo_rec.batch_no, 10);
	cc = find_rec (incf, &incfRec, GTEQ, "u");
	while (!cc &&
		incfRec.hhwh_hash == hhwhHash &&
		!strncmp (incfRec.gr_number, pcwo_rec.batch_no, 10))
	{
		incfRec.fifo_cost 	= actCost;
		incfRec.act_cost 	= stdCost;

		if ((cc = abc_update (incf, &incfRec)))
			file_err (cc, (char *)incf, "DBUPDATE");

		cc = find_rec (incf, &incfRec, NEXT, "u");
	}

	abc_selfield (incf, "incf_seq_id");
}

/*=======================================
| Log inventory movements.              |
| STK ISS for the manufacturing branch, |
| STK REC for the receiving branch.     |
=======================================*/
void
TransStock (
 float _qty)
{
	/*--------------------------
	| Log inventory movements. |
	--------------------------*/
	MoveAdd 
	(
		comm_rec.co_no,
		pcwo_rec.rec_br_no,
		pcwo_rec.rec_wh_no,
		incc_rec.hhbr_hash,
		incc_rec.hhcc_hash,
		inmr_rec.std_uom,
		local_rec.receiptDate,
		2,						/* stock receipt to the receiving branch */
		pcwo_rec.order_no,
		inmr_rec.inmr_class,
		inmr_rec.category,
		pcwo_rec.order_no,
		pcwo_rec.batch_no,
		_qty,
		0.00,
		local_rec.actCost
	);

	/*--------------------------
	| Log inventory movements. |
	--------------------------*/
	MoveAdd 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		incc_rec.hhbr_hash,
		pcwo_rec.hhcc_hash,
		inmr_rec.std_uom,
		local_rec.receiptDate,
		3,						/* stock issue from the manufacturing brahch */
		pcwo_rec.order_no,
		inmr_rec.inmr_class,
		inmr_rec.category,
		pcwo_rec.order_no,
		pcwo_rec.batch_no,
		_qty,
		0.00,
		local_rec.actCost
	);
}

/*---------------------------------------
| Update $amt recptd on pcms records	|
---------------------------------------*/
double	
UpdatePcms (
 int closeJob)
{
	double	tot_cost = 0.00,
			tmpCost,
			quantity;
	float	cnvFct;

	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");

	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*------------------------
		| Find item master file. |
		------------------------*/
		inmr2_rec.hhbr_hash = pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			inmr2_rec.outer_size = 1.00;

		/*----------------------------------------------------
		| Find item branch file at the manufacturing branch. | 
		----------------------------------------------------*/
		inei2Rec.hhbr_hash = pcms_rec.mabr_hash;
		strcpy (inei2Rec.est_no, pcwo_rec.br_no);
		if (find_rec (inei, &inei2Rec, COMPARISON, "r"))
			inei2Rec.std_cost = 0.00;

		cnvFct = GetUom (pcms_rec.uom, inmr2_rec.alt_uom);
		inei2Rec.std_cost /= cnvFct;
		quantity = pcms_rec.matl_wst_pc;
		quantity += 100.00;
		quantity /= 100.00;
		quantity *= pcms_rec.matl_qty;

		/*-------------------------
		| Standard material cost. |
		-------------------------*/
		tmpCost = out_cost (inei2Rec.std_cost, inmr2_rec.outer_size);
		tmpCost *= quantity;
		tmpCost = CENTS (tmpCost);

		tmpCost *= thisReceipt;
		tmpCost /= n_dec (pcwo_rec.prod_qty, inmr_rec.dec_pt);
		tmpCost = no_dec (tmpCost);

		if (closeJob)
			pcms_rec.amt_recptd = pcms_rec.amt_issued;
		else
			pcms_rec.amt_recptd += tmpCost;

		tot_cost += tmpCost;

		/*-----------------------------------
		| add sobg record for recalculation |
		| of material items committed qty   |
		-----------------------------------*/
		add_hash (pcwo_rec.co_no,  /* re-calc manufacturing warehouse */
				pcwo_rec.br_no,
				"RP",
				0,
				pcms_rec.mabr_hash,
				pcwo_rec.hhcc_hash,
				0L,
				(double) 0);  /* decrease committed qty for item produced */

		cc = abc_update (pcms, &pcms_rec);
		if (cc)
			file_err (cc, (char *)pcms, "DBUPDATE");

		cc = find_rec (pcms, &pcms_rec, NEXT, "u");
	}
	abc_unlock (pcms);

	return (tot_cost);
}

/*---------------------------------------
| Update $amt recptd on pcln records	|
---------------------------------------*/
double	
UpdatePcln (
 int closeJob)
{
	double	tot_cost = 0.00,
			ovh_cost = 0.00,
			tmpCost = 0.00,
			actCost	 = 0.00,
			actOvh   = 0.00;
	long	tmp_time = 0L,
			actTime  = 0L;
	int		TYPE = 0;

	pcln_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcln_rec.seq_no = 0;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		if (closeJob)
		{
			actTime = 0L;
			pcrq_rec.hhwo_hash	= pcln_rec.hhwo_hash;
			pcrq_rec.seq_no		= pcln_rec.seq_no;
			pcrq_rec.line_no	= pcln_rec.line_no;
			cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
			while (!cc && 
				pcrq_rec.hhwo_hash	== pcln_rec.hhwo_hash &&
				pcrq_rec.seq_no		== pcln_rec.seq_no &&
				pcrq_rec.line_no	== pcln_rec.line_no)
			{
				actTime += pcrq_rec.act_setup;
				actTime += pcrq_rec.act_run;
				actTime += pcrq_rec.act_clean;

				cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
			}

			actCost = (pcln_rec.rate + pcln_rec.ovhd_var) * (double) actTime;
			actCost /= 60.0;
			actCost += pcln_rec.ovhd_fix;
			actCost *= pcln_rec.qty_rsrc;
			actOvh  = pcln_rec.ovhd_var * (double) actTime;
			actOvh  /= 60.0;
			actOvh  += pcln_rec.ovhd_fix;
			actOvh  *= pcln_rec.qty_rsrc;
		}
		tmp_time = pcln_rec.setup;
		tmp_time += pcln_rec.run;
		tmp_time += pcln_rec.clean;

		tmpCost = ovh_cost = 0.00;
		if (tmp_time)
		{
			tmpCost = (pcln_rec.rate + pcln_rec.ovhd_var) * (double) tmp_time;
			tmpCost /= 60.0;
			tmpCost += pcln_rec.ovhd_fix;
			tmpCost *= pcln_rec.qty_rsrc;
			ovh_cost = pcln_rec.ovhd_var * (double) tmp_time;
			ovh_cost /= 60.0;
			ovh_cost += pcln_rec.ovhd_fix;
			ovh_cost *= pcln_rec.qty_rsrc;
		}

		tmpCost *= thisReceipt;
		tmpCost /= pcwo_rec.prod_qty;
		ovh_cost *= thisReceipt;
		ovh_cost /= pcwo_rec.prod_qty;

		if (closeJob)
		{
			actCost = no_dec (actCost);
			actOvh  = no_dec (actOvh);
			pcln_rec.amt_recptd = actCost;
			pcln_rec.ovh_recptd = actOvh;
		}
		else
		{
			tmpCost = no_dec (tmpCost);
			ovh_cost = no_dec (ovh_cost);
			pcln_rec.amt_recptd += tmpCost;
			pcln_rec.ovh_recptd += ovh_cost;
		}
		tot_cost += tmpCost;

		/*---------------------------------------------------------
		| Check resource type, and add to appropriate total cost. |
		---------------------------------------------------------*/
		
		rgrs_rec.hhrs_hash = pcln_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (!cc)
		{
			switch (rgrs_rec.type [0])
			{
			case	'L': /* Labour 	 */	TYPE = 0; break;
			case	'M': /* Machine  */	TYPE = 1; break;
			case	'Q': /* QC-Check */	TYPE = 2; break;
			case	'S': /* Special  */	TYPE = 3; break;
			case	'O': /* Other 	 */	TYPE = 4; break;
			} /* END OF SWITCH */

			/*------------------------------------------- 
			| read G/L accounts from the resource file. |
			-------------------------------------------*/
			glmrRec.hhmr_hash = rgrs_rec.dir_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				dirHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (dirAcc [TYPE], glmrRec.acc_no);
			}
			
			glmrRec.hhmr_hash = rgrs_rec.fix_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				fixHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (fixAcc [TYPE], glmrRec.acc_no);
			}
			glmrRec.hhmr_hash = rgrs_rec.mfg_dir_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				mfgDHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (mfgDAcc [TYPE], glmrRec.acc_no);
			}
			glmrRec.hhmr_hash = rgrs_rec.mfg_fix_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				mfgFHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (mfgFAcc [TYPE], glmrRec.acc_no);
			}
		}
	    cc = abc_update (pcln, &pcln_rec);
	    if (cc)
			file_err (cc, (char *)pcln, "DBUPDATE");

	    cc = find_rec (pcln, &pcln_rec, NEXT, "u");
	}
	abc_unlock (pcln);

	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash + 1;
	pcln_rec.seq_no		= 0;
	pcln_rec.line_no	= 0;
	if (find_rec (pcln, &pcln_rec, GTEQ, "r"))
		find_rec (pcln, &pcln_rec, LAST, "r");
	else
		find_rec (pcln, &pcln_rec, PREVIOUS, "r");

	pcwc_rec.hhwc_hash = pcln_rec.hhwc_hash;
	cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)pcwc, "DBFIND");

	return (tot_cost);
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
 int closeJob)
{
	double	value;

	if (closeJob)
		value = out_cost (local_rec.actCost, inmr_rec.outer_size);
	else
		value = out_cost (local_rec.standardUnitCost, inmr_rec.outer_size);

	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	if (!popeOpen)
		OpenAudit ();
	popeOpen = TRUE;
	fprintf (pp, "|%16.16s",	inmr_rec.item_no);
	fprintf (pp, "|%-23.23s",	inmr_rec.description);
	fprintf (pp, "|%25.25s",	local_rec.serialNo);
	fprintf (pp, "|%7.7s",		local_rec.orderNo);
	fprintf (pp, "|%10.10s",	local_rec.batchNo);
	fprintf (pp, "|%10.2f",
			DOLLARS  (closeJob ? local_rec.actCost : local_rec.standardUnitCost));
	fprintf (pp, "|%9.1f",		local_rec.outerSize);
	fprintf (pp, "|%14.6f",		local_rec.receiptQty);
	fprintf (pp, "|%10.10s",	DateToString (local_rec.receiptDate));
	fprintf (pp, "|%-10.10s",	local_rec.location);
	fprintf (pp, "|%11.2f|\n",	DOLLARS  (local_rec.receiptQty * value));
	fflush (pp);

	batchTotal += (double)local_rec.receiptQty * value;
	quantityTotal   += local_rec.receiptQty;
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n",printerNumber);
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".EPRODUCTION RECEIPTS\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp,
			".E MANUFACTURING BRANCH %s : Warehouse %s \n",
			clip (comm_rec.est_name),
			clip (comm_rec.cc_name));

	fprintf (pp, ".R==================");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "==========");
	fprintf (pp, "===============");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "============\n");

	fprintf (pp, "=================");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "==========");
	fprintf (pp, "===============");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "=============\n");

	fprintf (pp, "|  ITEM NUMBER   ");
	fprintf (pp, "|   ITEM  DESCRIPTION   ");
	fprintf (pp, "|       REFERENCE         ");
	fprintf (pp, "| ORDER ");
	fprintf (pp, "|  BATCH   ");
	fprintf (pp, "|   @COST  ");
	fprintf (pp, "| PRICING ");
	fprintf (pp, "|   QUANTITY   ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "| LOCATION ");
	fprintf (pp, "|  EXTENDED |\n");

	fprintf (pp, "|                ");
	fprintf (pp, "|                       ");
	fprintf (pp, "|                         ");
	fprintf (pp, "|NUMBER ");
	fprintf (pp, "|  NUMBER  ");
	fprintf (pp, "| RECEIPT  ");
	fprintf (pp, "|  CONV.  ");
	fprintf (pp, "|              ");
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|  VALUE.   |\n");

	fprintf (pp, "|----------------");
	fprintf (pp, "+-----------------------");
	fprintf (pp, "+-------------------------");
	fprintf (pp, "+-------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+---------");
	fprintf (pp, "+--------------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+-----------|\n");
}

int
AddIncc (
	long	hhccHash)
{
	char	tempSort [29];
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sprintf (tempSort,"%s%11.11s%-16.16s",	inmr_rec.inmr_class,
						inmr_rec.category,
						inmr_rec.item_no);

	strcpy (incc_rec.sort,tempSort);
	strcpy (incc_rec.stocking_unit,inmr_rec.sale_unit);
	incc_rec.opening_stock 	= 0.00;
	incc_rec.receipts 		= 0.00;
	incc_rec.pur 			= 0.00;
	incc_rec.issues 		= 0.00;
	incc_rec.adj 			= 0.00;
	incc_rec.sales 			= 0.00;
	incc_rec.closing_stock 	= 0.00;
	incc_rec.ytd_receipts 	= 0.00;
	incc_rec.ytd_pur 		= 0.00;

	incc_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc_rec.stat_flag,"0");

	cc = abc_add (incc, &incc_rec);
	if (cc)
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = FindIncc ();
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*========================================================================
| Routine to find incc record. Returns 0 if found ok, 1 if not on file . |
| 999 if a database error occurred.                                      |
========================================================================*/
int
FindIncc (
 void)
{
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");

	local_rec.hhwhHash = incc_rec.hhwh_hash;
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*==============================================================
| Routine to add item at warehouse level if not already there. |
==============================================================*/
void
AddSerial (
 void)
{
	int	rc;

	insfRec.hhwh_hash = local_rec.hhwhHash;
	insfRec.hhbr_hash = inmr_rec.hhbr_hash;
	sprintf (insfRec.status, "F");
	sprintf (insfRec.serial_no,"%-25.25s",local_rec.serialNo);
	sprintf (insfRec.location,"%-10.10s",local_rec.location);
	insfRec.date_in = local_rec.receiptDate;
	insfRec.est_cost = twodec (DOLLARS  (local_rec.standardUnitCost));
	strcpy (insfRec.stat_flag,"E");
	rc = abc_add (insf, &insfRec);
	if (rc)
		file_err (cc, insf, "DBADD");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp, "|----------------");
	fprintf (pp, "+-----------------------");
	fprintf (pp, "+-------------------------");
	fprintf (pp, "+-------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+---------");
	fprintf (pp, "+--------------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+-----------|\n");

	fprintf (pp, "| BATCH TOTAL    ");
	fprintf (pp, "|                       ");
	fprintf (pp, "|                         ");
	fprintf (pp, "|       ");
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|         ");
	fprintf (pp, "|%14.6f", quantityTotal);
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|%11.2f|\n", DOLLARS (batchTotal));

	fprintf (pp, ".EOF\n");
	pclose (pp);
}
/*===============================
| Process control Accounts	|
===============================*/
int
GetAccount (
	char	*category)
{
	abc_selfield ("glmr", "glmr_id_no");
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE M",
		" ",
		category
	);
	strcpy (local_rec.creditAccount, glmrRec.acc_no);
	local_rec.creditHhmrHash = glmrRec.hhmr_hash;

	abc_selfield ("glmr", "glmr_hhmr_hash");
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	if (scn == 1)
	{
		swide ();
		clear ();
		rv_pr (ML(mlPcMess084), 52,0,1);

		line_at (1, 0, 132);
		line_at (4, 0, 67);
		line_at (7, 67,64);
		line_at (10,1, 131);
		line_at (13,1, 131);
		line_at (21,1, 131);
		box (0, 2, 132, 13);
		box (66, 4, 65, 5);

		rv_pr (ML(mlPcMess014), 88, 5, FALSE);
		rv_pr (ML(mlPcMess017), 89, 8, FALSE);
		print_at (9, 69, ML(mlPcMess018));
		print_at (9, 90, ML(mlPcMess019));
		print_at (9,111, ML(mlPcMess020));
	}

	if (scn == 2)
	{
		rv_pr (local_rec.status, 63, 3, FALSE);

		print_at (9, 69, "%s %14.6f", ML(mlPcMess018), local_rec.stdBatch);
		print_at (9, 90, "%s %14.6f", ML(mlPcMess019), local_rec.minBatch);
		print_at (9,111, "%s %14.6f", ML(mlPcMess020), local_rec.maxBatch);

		box (0, 16, 132, 4);
		line_at (16,1,131);
	}

	if (scn == 3)
		box (tab_col, 16, 38, 7);

	print_at (22,1,ML(mlStdMess038), comm_rec.co_no, clip (comm_rec.co_short));
	print_at (22,30,ML(mlStdMess039),comm_rec.est_no,clip (comm_rec.est_short));
	print_at (22,60,ML(mlStdMess099),comm_rec.cc_no, clip (comm_rec.cc_short));
	print_at (0,102,ML(mlPcMess121), local_rec.prevWoNo);
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_FAILURE);
}

/*===============================
| Add a trans to the pcgl file.	|
| NB: amount should be in cents	|
===============================*/
void
AddPcgl 
(
	long	hhmrHash,
	char	*accountNo,
	char	*workCenter,
	double	amount,
	char	*type,
	char	*tranType
)
{
	if (amount == 0.00)
		return;

	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, tranType);
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no,"%02d", periodMonth);
	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", workCenter);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 	= amount;
	pcgl_rec.loc_amount = amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);
	strcpy (pcgl_rec.acc_no, accountNo);
	pcgl_rec.hhgl_hash = hhmrHash;

	strcpy (pcgl_rec.jnl_type, type);
	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, (char *)pcgl, "DBADD");
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
 int    _dec_pt)
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
		sprintf (err_str,ML(mlPcMess083), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

/*==========================
| Process the sales order. |
==========================*/
void
ProcessSalesOrder (
	long	hhslHash)
{
	soln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (soln, &soln_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (soln);
		return;
	}
	strcpy (soln_rec.status, 	(envVarConOrders) ? "M" : "R");
	strcpy (soln_rec.stat_flag, (envVarConOrders) ? "M" : "R");
	cc = abc_update (soln, &soln_rec);
	if (cc)
		file_err (cc, soln, "DBUPDATE");

	sohr_rec.hhso_hash = soln_rec.hhso_hash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (sohr);
		return;
	}
	strcpy (sohr_rec.status, 	(envVarConOrders) ? "M" : "R");
	strcpy (sohr_rec.stat_flag, (envVarConOrders) ? "M" : "R");
	cc = abc_update (sohr, &sohr_rec);
	if (cc)
		file_err (cc, sohr, "DBUPDATE");
		
	if (soln_rec.status [0] == 'R')
	{
		add_hash 
		(
			"  ",
			"  ",
			(printerNumber) ? "PA" : "PC",
			printerNumber,
			soln_rec.hhso_hash,
			0L,
			0L,
			(double) 0
		);
	}
}
			

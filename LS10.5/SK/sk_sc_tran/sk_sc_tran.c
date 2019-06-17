/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_sc_tran.c,v 5.8 2002/07/24 08:39:17 scott Exp $
|  Program Name  : (sk_sc_tran.c & sk_mc_tran.c )                     |
|  Program Desc  : (Stock Issue/ Receipts Seperate .             )    |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 30/03/91          |
|---------------------------------------------------------------------|
| $Log: sk_sc_tran.c,v $
| Revision 5.8  2002/07/24 08:39:17  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/06/20 07:11:09  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2002/01/09 01:16:57  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.5  2001/11/22 01:14:56  scott
| Updated to re-lineup reports as movement transaction reflected as 10 instead of 15
|
| Revision 5.4  2001/11/20 03:51:37  scott
| Updated to ensure location changed when direct issue and quantity changed on edit.
|
| Revision 5.3  2001/08/09 09:19:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:46  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:31  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sc_tran.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sc_tran/sk_sc_tran.c,v 5.8 2002/07/24 08:39:17 scott Exp $";

#define	MAXSCNS		3
#define	MAXLINES	1000
#define	SR		store [line_cnt]

#define	TR_I_RMT	 (!strcmp (transferType, "RI")) /* Remote Issue.	  */
#define	TR_I_DIR	 (!strcmp (transferType, "ID")) /* Issue Direct.	  */
#define	TR_I_ONE	 (!strcmp (transferType, "IO")) /* Issue One step.    */
#define	TR_I_TWO	 (!strcmp (transferType, "IT")) /* Issue two step.    */
#define	TR_R_ISS	 (!strcmp (transferType, "IR")) /* Issue Request.     */
#define	TR_R_CON	 (!strcmp (transferType, "RC")) /* Request Confirm    */
#define	TR_I_CON	 (!strcmp (transferType, "IC")) /* Issue Confirmatiom.*/
#define	TR_RECEIPT	 (!strcmp (transferType, "RE")) /* Receipt.           */
#define	TR_DISPLAY	 (!strcmp (transferType, "TD")) /* Transfer Display.  */
#define	TR_ISSUE	 (TR_I_RMT || TR_I_DIR || TR_I_ONE || TR_I_TWO)

#define	NO_COST		 (SR.storeClass [0] == 'N')
#define	NON_STOCK	 (SR.storeClass [0] == 'Z')
#define	BO_OK		 (SR.recBackOrder [0] == 'Y' || SR.recBackOrder [0] == 'F')
#define	FULL_BO		 (SR.recBackOrder [0] == 'F')
#define	SER_COSTING	 (SR.costingFlag [0] == 'S')
#define SERIAL		 (SR.serialItem [0] == 'Y')
#define	BLANK_SER	 (!strcmp (local_rec.serial_no, ser_space))
#define	FIFO		 (inmr_rec.costing_flag [0] == 'F')
#define	LIFO		 (inmr_rec.costing_flag [0] == 'L')
#define	DUTY		 (chargeDuty [0] == 'Y')
#define	MC_TRAN		 (multiCompanyTransfer == TRUE)
#define	KIT_ITEM	 (SR.storeClass [0] == 'K' && prog_status == ENTRY)
#define	PHANTOM		 (SR.storeClass [0] == 'P' && prog_status == ENTRY)
#define	E_PHANTOM	 (SR.storeClass [0] == 'P' && prog_status != ENTRY)

#define	FREIGHT_CHG	 (ithr_rec.frt_req [0] == 'Y' && envVar.automaticFreight)

#define	HEADER_SCN		1
#define	ITEM_SCN		2
#define	FREIGHT_SCN		3

#define	ENTER_DATA	 (prog_status == ENTRY)

#define	MAX_SUPER	500

#define	SLEEP_TIME	2

#define	SUCCESS		0
#define	RETURN_ERR	1

#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <ml_tr_mess.h>
#include <pslscr.h>
#include <twodec.h>
#include <proc_sobg.h>

#ifdef GVISION
#include <StockWindow.h>
#endif

extern	int		SR_Y_POS;
extern	int		SR_X_POS;

extern	int		TruePosition;

	FILE	*pout;
	FILE	*pout2;
	char	pipeName [100],
			formatFile [100];

	char	CURR_CODE [4];
	double  branchMargin	=	0.00;

	int		np_fn;
	int		wpipe_open = FALSE;

	int		in_phantom 				= FALSE,
			phantomOption 			= FALSE,
			multiCompanyTransfer 	= FALSE,
			quarTransfer 			= FALSE,
			pipe_open 				= FALSE,
			printerNumber 			= TRUE,
			transferChanged			= FALSE,
			superCounter 			= FALSE,
			processID,
			superFlag,
			newTransfer				=	FALSE;

	int		IN_special 	= FALSE;

	int		notOrigWh; /* Set to TRUE if recHhccHash different to itln_hhcc */
	long	origHhccHash;

	char	sup_part [17],
			alt_part [17],
			iss_rec [2],
			userReferenceNo [9],
			transferType [3],
			chargeDuty [2];

	char	*currentUser;
	char	*result;

	long	recHhwhHash	= 0L,
			issHhwhHash	= 0L;

	float	issClosing		= 0.00,
			recClosing		= 0.00,
			workQuantity	= 0.00;

	double	batchTotal 		= 0.00,
			quantityTotal 	= 0.00,
			workValue 		= 0.00,
			workDuty 		= 0.00;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct ffdmRecord	ffdm_rec;
struct inwdRecord	inwd_rec;
struct inwuRecord	inwu_rec;
struct esmrRecord	esmr_rec;
struct itglRecord	itgl_rec;
struct ithrRecord	ithr_rec;
struct ithrRecord	ithr2_rec;
struct itlnRecord	itln_rec;
struct trshRecord	trsh_rec;
struct trchRecord	trch_rec;
struct trzmRecord	trzm_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;
struct colnRecord	coln_rec;
struct solnRecord	soln_rec;
struct soktRecord	sokt_rec;

	char 	*data	=	"data",
			*incc2	=	"incc2",
			*inum2	=	"inum2",
			*ithr2	=	"ithr2",
			*ser_space		=	"                         ",
			*sixteen_space	=	"                ";

#include	<MoveRec.h>
#include	<Costing.h>

	struct storeRec {
		char	costingFlag [2];
		char	Demand [2];
		char	_description [41];
		char	errorFound [2];
		char	issBackOrder [2];
		char	issLocation [11];
		char	lot_ctrl [2];
		char	lot_no [8];
		char	oldLotNo [8];
		char	origSerialNo [26];
		char	recBackOrder [2];
		char	recLocation [11];
		char	serial [26];
		char	serialItem [2];
		char	storeClass [2];
		char	UOM [5];
		double	fileCost;
		double	upft_amt;
		float	cnvFct;
		float	issAvailable;
		float	issClosing;
		float	origOrderQty;	
		float	qtyDes;
		float	qtyRec;
		float	upft_pc;
		float	weight;
		int		decPt;
		long	hhbrHash;
		long	hhumHash;
		long	issHhwhHash;
		long	itffHash;
		long	origHhbrHash;		/* Original hhbr hash 		*/
		long	recHhbrHash;
		long	recHhwhHash;
	} store [MAXLINES];

	char	issCo [3],
			issConame [41],
			issCoshort [16],
			issCoAddr [3][41],
			issBr [3],
			issBrAddr [3][41],
			issBrname [41],
			issBrshort [16],
			issWh [3],
			issWhname [41],
			issWhshort [10];

	char	recCo [3],
			recConame [41],
			recCoshort [16],
			recCoAddr [3][41],
			recBr [3],
			recBrAddr [3][41],
			recBrname [41],
			recBrshort [16],
			recWh [3],
			recWhname [41],
			recWhshort [10],
			recArea [3];

	char	lastStatus [2];

	long	issHhccHash		=	0L,
			recHhccHash		=	0L,
			progPid			=	0L;

	char	tmp_str [2][30];

	char	*scn_desc [] = {
			"TRANSFER HEADER SCREEN.",
			"TRANSFER ITEM SCREEN.",
			"TRANSFER FREIGHT SCREEN."
		};

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	int		perminantWindow;
	int		windowPopupOk;
	int		transferConfirm;
	int		transferNumbering;
	int		qcApplied;
	int		qCAvailable;
	int		automaticFreight;
	int		fullSupplyOrder;
	int		soFreightCharge;
	int		zeroCost;
	int		IKHK;
	int		soFwdAvl;
} envVar;

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	carr_adesc [41];
	char	carr_desc [41];
	char	defaultDelZone [7];
	char	dflt_qty [15];
	char	dummy [11];
	char	hr_full_supply [2];
	char	LL [2];
	char	ln_full_supp [2];
	char	location [11];
	char	lot_ctrl [2];
	char	lot_no [8];
	char	norm_area [41];
	char	oldLotNo [8];
	char	previousRef [9];
	char	rep_qty [30];
	char	serial_no [26];
	char	stock [2];
	char	stockDate [11];
	char	systemDate [11];
	char	UOM [5];
	double	efreight;
	double	qty_bord_doub;
	double	qty_ord_doub;
	double	qty_rec_doub;
	double	qty_ship_doub;
	double	workCost;
	double	workDuty;
	float	qty_bord;
	float	qty_ord;
	float	qty_rec;
	float	qty_ship;
	int		line_no;
	int		no_cartons;
	long	tran_no;
	long	transDate;
} local_rec;

static struct	var vars [] =
{
	{HEADER_SCN, LIN, "transferNo",	 4, 2, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Transfer Number     : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.tran_no},
	{HEADER_SCN, LIN, "jnlDate",	 5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.stockDate, "Transfer date       : ", "Return to default to current stock module date. ",
		YES, NO, JUSTRIGHT, "1", "", (char *) &local_rec.transDate},
	{HEADER_SCN, LIN, "freightRequired",	 5, 66, CHARTYPE,
		"U", "          ",
		" ", " ", "Freight Required    : ", "Enter Y (es) or N (o)",
		 YES, NO,  JUSTLEFT, "YN", "", ithr_rec.frt_req},
	{HEADER_SCN, LIN, "transRef",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "NONE", "Transfer Reference  : ", "Enter transfer internal ref.",
		YES, NO,  JUSTLEFT, "", "", ithr_rec.tran_ref},
	{HEADER_SCN, LIN, "tr_full_supply",	 6, 66, CHARTYPE,
		"U", "          ",
		" ", "N", "Full supply transfer: ", "Enter Y (es) or N (o)",
		 NE, NO,  JUSTLEFT, "YN", "", local_rec.hr_full_supply},
	{HEADER_SCN, LIN, "issCoNo",	 8, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Issuing Company     : ", "Enter issuing company number.",
		 NA, NO, JUSTRIGHT, "1", "99", issCo},
	{HEADER_SCN, LIN, "issCoShort",	 8, 66, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Company Name        : ", " ",
		 NA, NO, JUSTRIGHT, "", "", issCoshort},
	{HEADER_SCN, LIN, "issBrNo",	 9, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Issuing Branch      : ", "Enter issuing branch number.",
		 NA, NO, JUSTRIGHT, "1", "99", issBr},
	{HEADER_SCN, LIN, "issBrName",	 9, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name         : ", " ",
		 NA, NO, JUSTRIGHT, "", "", issBrname},
	{HEADER_SCN, LIN, "issWhNo",	10, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Issuing Warehouse   : ", "Enter issuing warehouse number.",
		 NA, NO, JUSTRIGHT, "1", "99", issWh},
	{HEADER_SCN, LIN, "issWhName",	10, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Warehouse Name      : ", " ",
		 NA, NO, JUSTRIGHT, "", "", issWhname},
	{HEADER_SCN, LIN, "recCoNo",	12, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Receiving Company   : ", "Enter receiving company number.",
		 NA, NO, JUSTRIGHT, "1", "99", recCo},
	{HEADER_SCN, LIN, "recCoShort",	12, 66, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Company Name        : ", " ",
		 NA, NO, JUSTRIGHT, "", "", recCoshort},
	{HEADER_SCN, LIN, "recBrNo",	13, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Receiving Branch    : ", "Enter receiving branch number.",
		 NA, NO, JUSTRIGHT, "1", "99", recBr},
	{HEADER_SCN, LIN, "recBrName",	13, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name         : ", " ",
		 NA, NO, JUSTRIGHT, "", "", recBrname},
	{HEADER_SCN, LIN, "recWhNo",	14, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Receiving Warehouse : ", "Enter receiving warehouse number.",
		 NA, NO, JUSTRIGHT, "1", "99", recWh},
	{HEADER_SCN, LIN, "recWhName",	14, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Warehouse Name      : ", " ",
		 NA, NO, JUSTRIGHT, "", "", recWhname},
	{ITEM_SCN, TAB, "itemNumber",		MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "    Item no.    ", "Enter item number. [SEARCH] keys available ",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{ITEM_SCN, TAB, "lineNumber",	 0,  1, INTTYPE,
		"NNNN", "          ",
		" ", " ", "lineNumber", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.line_no},
	{ITEM_SCN, TAB, "itemDescription",	 0,  0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            Item Description.           ", " ",
		 YES, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{ITEM_SCN, TAB, "stock",	 0,  0, CHARTYPE,
		"U", "          ",
		" ", "S", " ", "Enter C (ustomer order) S (tock Order). <Default = S> ",
		 NA, NO,  JUSTLEFT, "CcSs", "", local_rec.stock},
	{ITEM_SCN, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", " Unit of Measure ",
		 NO, NO, JUSTLEFT, "", "", local_rec.UOM},
	{ITEM_SCN, TAB, "qtyShip",	 0,  0, DOUBLETYPE,
		local_rec.dflt_qty, "          ",
		" ", " ", "Ship Qty", "Enter shipment quantity. ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *) &local_rec.qty_ship_doub},
	{ITEM_SCN, TAB, "qtyOrder",	 0,  0, DOUBLETYPE,
		local_rec.dflt_qty, "          ",
		" ", "1.0", "Qty Ord.", "Enter order quantity.",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *) &local_rec.qty_ord_doub},
	{ITEM_SCN, TAB, "qtyReceive",	 0,  0, DOUBLETYPE,
		local_rec.dflt_qty, "          ",
		" ", " ", "Qty Rec.", "Enter quanity received. ",
		 ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *) &local_rec.qty_rec_doub},
	{ITEM_SCN, TAB, "qtyBackorder",	 0,  0, DOUBLETYPE,
		local_rec.dflt_qty, "          ",
		" ", " ", "Qty B/O.", "Enter quantity backordered. ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *) &local_rec.qty_bord_doub},
	{ITEM_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM_SCN, TAB, "cost",		 0,  0, DOUBLETYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", " Item  Cost ", "Enter cost price, <default = calculated cost> ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.workCost},
	{ITEM_SCN, TAB, "duty",		 0,  0, DOUBLETYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Extra Amt", "Enter amount of Duty. ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.workDuty},
	{ITEM_SCN, TAB, "ser_no",	 0,  0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Serial No        ", "Enter Serial Number, [SEARCH] key available. ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{ITEM_SCN, TAB, "ln_status",	 0,  0, CHARTYPE,
		"U", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", itln_rec.status},
	{ITEM_SCN, TAB, "ln_full_supp",	 0,  0, CHARTYPE,
		"U", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ln_full_supp},
	{FREIGHT_SCN, LIN, "carrierCode",	 4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Carrier Code.       : ", "Enter carrier code, [SEARCH] available.",
		YES, NO,  JUSTLEFT, "", "", trcm_rec.carr_code},
	{FREIGHT_SCN, LIN, "carr_desc",	 4, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Carrier Description : ", " ",
		 NA, NO,  JUSTLEFT, "", "", trcm_rec.carr_desc},
	{FREIGHT_SCN, LIN, "deliveryZoneCode",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultDelZone, "Delivery Zone       : ", "Enter Delivery Zone Code [SEARCH]. ",
		 YES, NO, JUSTLEFT, "", " ", trzm_rec.del_zone},
	{FREIGHT_SCN, LIN, "deliveryZoneDesc",	 5, 66, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Zone Desc  : ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{FREIGHT_SCN, LIN, "deliveryRequired",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Delivery Reqd. (Y/N): ", "Enter Y (es) for Delivery. <default = N (o)> ",
		 YES, NO, JUSTLEFT, "YN", "", ithr_rec.del_req},
	{FREIGHT_SCN, LIN, "deliveryDate",	6, 66, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Delivery Date       : ", " ",
		NA, NO,  JUSTLEFT, " ", "", (char *)&ithr_rec.del_date},
	{FREIGHT_SCN, LIN, "cons_no",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Consignment no.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", ithr_rec.cons_no},
	{FREIGHT_SCN, LIN, "no_cartons",	 7, 66, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Number Cartons.     : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&ithr_rec.no_cartons},
	{FREIGHT_SCN, LIN, "est_freight",	 8, 2, DOUBLETYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Est Freight         : ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.efreight},
	{FREIGHT_SCN, LIN, "tot_kg",	 8, 40, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Total Kgs.  ", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ithr_rec.no_kgs},
	{FREIGHT_SCN, LIN, "freight",	 8, 66, DOUBLETYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.     : ", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&ithr_rec.frt_cost},
	{0, LIN, "",		 0,  0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include	<LocHeader.h>
#include    <MoveAdd.h>

/*=======================
| Function Declarations |
=======================*/
float 	GetBrClosing 			(char *, char *, long);
float 	ReCalcAvail 			(void);
float 	ToLclUom 				(float);
float 	ToStdUom 				(float);
float 	phantom_avail 			(long);
int  	AddIncc 				(long, long);
int  	CheckDuplicateInsf 		(char *, long, int);
int  	CheckInsf 				(long, char *);
int  	CheckIthr 				(long);
int  	CheckFreight			(void);
int  	CheckOk 				(int);
int  	FindCost 				(void);
int  	FindSuper 				(char *, char *, int);
int  	InputResponse 			(void);
int  	ItemError 				(char *);
int  	LoadItems 				(void);
int		ValidQuantity 			(double, int);
int		ValidateItemNo 			(void);
int		heading 				(int);
int		spec_valid 				(int);
int		warn_user 				(char *);
int		OpenStockWindow 		(void);
void 	AddCarrierDetails 		(void);
void 	AddFifoRecords			(long, long, float);
void	AddInei 				(long, char	*);
void 	AddItgl 				(int, double, double, double);
void 	ArgumentError 			(char *);
void 	BusyFunction 			(int);
void 	CalculateFreight 		(float, double, double, double);
void 	CheckEnvironment 		(void);
void 	CloseDB 				(void);
void 	CloseTransportFiles 	(void);
void 	CommitInsf 				(int, char *);
void 	Confirm 				(int);
void 	FreeInsf 				(int, char *);
void 	HeadingComment 			(char *, int);
void 	LineFullSupply 			(void);
void 	OpenDB 					(void);
void 	OpenTransportFiles 		(char *);
void 	PrintCoStuff 			(void);
void 	ProcessIssues 			(float, double);
void 	ProcessKitItem 			(long, float);
void 	ProcessPhantom 			(long, float, float);
void 	ProcessReceipts 		(float, double);
void 	SetIssueBranch 			(char *, char *, char *, long, int);
void 	SrchCcmr 				(char *, int);
void 	SrchComr 				(char *);
void 	SrchEsmr 				(char *);
void 	SrchInsf 				(char *);
void 	SrchInum 				(char *);
void 	SrchIthr 				(char *);
void 	SrchTrcm 				(char *);
void 	SrchTrzm 				(char *);
void 	SuperAlternateError 	(void);
void 	TransferInsf 			(int, char *);
void 	Update 					(void);
void 	IntUpdateInsf 			(void);
void 	clear_win 				(void);
void 	shutdown_prog 			(void);
void 	tab_other 				(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	int		field,
			i;

	/*============================
	| Setup required parameters. |
	============================*/
	if (argc < 6)
	{
		ArgumentError (argv [0]);
		return (EXIT_FAILURE);
	}
		
	TruePosition	=	TRUE;

	/*-------------------------------------
	| Check environment variables and     |
	| set values in the envVar structure. |
	-------------------------------------*/
	CheckEnvironment ();

	progPid = (long) getpid ();

	SETUP_SCR (vars);


	printerNumber 	= 	atoi (argv [1]);
	processID 		= 	atoi (argv [2]);
	sprintf (transferType, "%-2.2s", argv [3]);
	strcpy (formatFile, argv [4]);
	sprintf (chargeDuty, "%-1.1s", argv [5]);

	if (!TR_I_RMT && !TR_I_DIR && !TR_I_ONE && !TR_I_TWO && !TR_R_ISS &&
		!TR_R_CON && !TR_I_CON && !TR_RECEIPT && !TR_DISPLAY)
	{
		ArgumentError (argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (CURR_CODE, "%-3.3s", get_env ("CURR_CODE"));
	sptr = chk_env ("ABC_IB_MARG");
	branchMargin = (sptr == (char *)0) ? 20.00 : atof (sptr);

	OpenDB ();

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "sk_mc_tran"))
		multiCompanyTransfer = TRUE;
	else
		multiCompanyTransfer = FALSE;

	if (!strcmp (sptr, "sk_qu_tran"))
		quarTransfer = TRUE;
	else
		quarTransfer = FALSE;

	if (envVar.automaticFreight)
		no_edit (FREIGHT_SCN);

	FLD ("LL") = ND;

	if (SK_BATCH_CONT || MULT_LOC)
	{
		if (TR_I_ONE || TR_I_TWO || TR_I_RMT || TR_R_ISS || TR_R_CON)
			FLD ("LL") = ND;

		if (TR_I_DIR || TR_I_CON)
			FLD ("LL") = YES;

		if (TR_RECEIPT	|| TR_DISPLAY)
			FLD ("LL") = NA;

		if (TR_RECEIPT && quarTransfer)
			FLD ("LL") = NO;
	}

	if (TR_ISSUE || TR_R_ISS || TR_DISPLAY)
	{
		FLD ("transferNo")	= YES;
		FLD ("jnlDate")		= YES;
		FLD ("transRef")	= YES;
		FLD ("issCoNo")		= NA;
		if (MC_TRAN)
		{
		    if (TR_I_RMT)
				FLD ("issCoNo") = NE;
		    else
				FLD ("issCoNo") = (TR_ISSUE) ? NA  : NE;
		}

		FLD ("issCoShort")	= NA;
		if (TR_I_RMT)
		    FLD ("issBrNo")	= NE;
		else
		    FLD ("issBrNo")	= (TR_ISSUE) ? NA  : NE;

		FLD ("issBrName")	= NA;
		if (TR_I_RMT)
		    FLD ("issWhNo")	= NE;
		else
		    FLD ("issWhNo")	= (TR_ISSUE) ? NA  : NE;
		FLD ("issWhName")	= NA;

		FLD ("recCoNo")	= NA;
		if (MC_TRAN)
		{
		    if (TR_I_RMT)
				FLD ("recCoNo") = NA;
		    else
				FLD ("recCoNo") = (TR_ISSUE) ? NE : NA;
		}

		FLD ("recCoShort")	= NA;
		if (TR_I_RMT)
		    FLD ("recBrNo")	= NA;
		else
		    FLD ("recBrNo")	= (TR_ISSUE) ? NE : NA;

		FLD ("recBrName")	= NA;
		if (TR_I_RMT)
		    FLD ("recWhNo")	= NA;
		else
		    FLD ("recWhNo")	= (TR_ISSUE) ? NE : NA;

		FLD ("recWhName")		= NA;
		FLD ("itemNumber")		= YES;
		FLD ("lineNumber")		= ND;
		FLD ("stock")			= YES;
		FLD ("qtyOrder")		= YES;
		FLD ("qtyReceive")		= ND;
		FLD ("qtyShip")			= ND;
		FLD ("qtyBackorder")	= YES;
		FLD ("cost")			= ND;
		FLD ("duty") 			= ND;
		FLD ("ser_no")			= (TR_R_ISS) ? ND : NA;
		FLD ("tr_full_supply")	= (envVar.fullSupplyOrder) ? YES : ND ;
		FLD ("freightRequired")	= YES;
		if (DUTY && TR_ISSUE)
		{
			FLD ("duty") = YES;
			FLD ("cost") = ND;
		}

	}
	if (TR_RECEIPT || TR_R_CON || TR_I_CON)
	{
		FLD ("transferNo")		= NE;
		FLD ("jnlDate")			= NA;
		FLD ("transRef")		= NA;
		FLD ("issCoNo")			= NA;
		FLD ("issCoShort")		= NA;
		FLD ("issBrNo")			= NA;
		FLD ("issBrName")		= NA;
		FLD ("issWhNo")			= NA;
		FLD ("issWhName")		= NA;
		FLD ("recCoNo")			= NA;
		FLD ("recCoShort")		= NA;
		FLD ("recBrNo")			= NA;
		FLD ("recBrName")		= NA;
		FLD ("recWhNo")			= NA;
		FLD ("recWhName")		= NA;
		FLD ("itemNumber")		= NA;
		FLD ("lineNumber")		= ND;
		FLD ("stock")			= NA;
		FLD ("ser_no")			= NA;
		FLD ("qtyOrder")		= (TR_R_CON || TR_I_CON) ? YES : ND;
		FLD ("qtyShip")			= (TR_R_CON || TR_I_CON) ? ND  : NA;
		FLD ("qtyReceive")		= (TR_R_CON || TR_I_CON) ? ND  : YES;
		FLD ("qtyBackorder")	= (TR_R_CON || TR_I_CON) ? YES : ND;
		FLD ("cost")			= ND;
		FLD ("tr_full_supply")	= ND;
		FLD ("freightRequired")	= ND;
		FLD ("duty")			= ND;
		FLD ("LL") 				= (MULT_LOC || SK_BATCH_CONT) ? YES : ND;
		if (DUTY && (TR_R_CON || TR_I_CON))
		{
			FLD ("duty")	= YES;
			FLD ("cost")	= ND;
		}
		if (TR_R_CON)
			FLD ("LL")	=	ND;
	}

	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();			/*  get into raw mode			*/
	set_masks ();		/*  setup print using masks		*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM_SCN, store, sizeof (struct storeRec));
#endif

	if (TR_DISPLAY)
	{
		for (field = label ("jnlDate"); FIELD.scn != 0; field++)
			if (FIELD.required != ND)
				FIELD.required = NA;
	}

	init_vars (HEADER_SCN);			/*  set default values		*/

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	if (!MC_TRAN)
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr,&comr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "comr", "DBFIND");

		strcpy (issCo, comr_rec.co_no);
		strcpy (recCo, comr_rec.co_no);
		strcpy (issConame, comr_rec.co_name);
		strcpy (recConame, comr_rec.co_name);
		strcpy (issCoshort, comr_rec.co_short_name);
		strcpy (recCoshort, comr_rec.co_short_name);
		strcpy (issCoAddr [0], comr_rec.co_adr1);
		strcpy (recCoAddr [0], comr_rec.co_adr1);
		strcpy (issCoAddr [1], comr_rec.co_adr2);
		strcpy (recCoAddr [1], comr_rec.co_adr2);
		strcpy (recCoAddr [2], comr_rec.co_adr3);
		strcpy (issCoAddr [2], comr_rec.co_adr3);
	}
	swide ();

	sprintf (local_rec.stockDate,"%-10.10s", DateToString (comm_rec.inv_date));

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		init_vars (HEADER_SCN);
		lcount [ITEM_SCN] = 0;
		transferChanged = FALSE;
		strcpy (lastStatus, " ");

		for (i = 0; i < MAXLINES; i++)
		{
			memset 	 (store + i, 0, sizeof (struct storeRec));
			sprintf (store [i].serial, "%25.25s", " ");
			sprintf (store [i].origSerialNo, "%25.25s", " ");
			strcpy 	 (store [i].errorFound, "N");
			strcpy 	 (store [i].lot_ctrl,  "N");
		}

		/*------------------------------
		| Enter screen 1 linear input. |
		| Turn screen initialise on.   |
		------------------------------*/
		init_ok	= TRUE;
		eoi_ok	= FALSE;

		heading (HEADER_SCN);
		entry (HEADER_SCN);
		if (prog_exit || restart)
			continue;

		if (TR_RECEIPT || TR_R_CON || TR_I_CON)
		{
			move (1,2); cl_line ();

			heading (HEADER_SCN);
			scn_display (HEADER_SCN);
			last_char = prmptmsg (ML (mlSkMess398),"YyNn",1,2);

			if (last_char == 'N' || last_char == 'n')
			{
				init_ok	= FALSE;
				eoi_ok	= FALSE;
				heading (ITEM_SCN);
				scn_display (ITEM_SCN);
				entry (ITEM_SCN);
				if (restart)
					continue;
			}
			else
				Confirm ((TR_RECEIPT) ? FALSE : TRUE);
		}
		else
		{
			if (!TR_DISPLAY)
			{
				heading (ITEM_SCN);
				scn_display (ITEM_SCN);
				entry (ITEM_SCN);
				if (restart)
					continue;
			}
		}

		if (TR_I_DIR || TR_I_CON)
		{
			if (envVar.automaticFreight && ithr_rec.frt_req [0] == 'Y')
			{
				init_ok = FALSE;
				heading (FREIGHT_SCN);
				scn_display (FREIGHT_SCN);
				entry (FREIGHT_SCN);
				init_ok = TRUE;
				if (restart)
					continue;
			}
		}

		edit_all ();
		if (restart)
			continue;

		if (TR_I_CON || TR_RECEIPT)
		{
			/*----------------------------
			| Check for blank Locations. |
			----------------------------*/
			while (CheckOk (FALSE))
			{
				errmess (ML (mlStdMess209));
				sleep (sleepTime);
				edit_all ();
				if (restart)
					break;
			}
			/*-------------------------------
			| Check for blank Serial items. |
			-------------------------------*/
			while (CheckOk (TRUE))
			{
				errmess (ML (mlStdMess201));
				sleep (sleepTime);
				edit_all ();
				if (restart)
					break;
			}
		}

		while (CheckFreight ())
		{
			 i = prmptmsg (ML ("Freight is Zero, correct [Y/N]?"),"YyNn",0,23);          	 BusyFunction (0); 
			 if (i == 'Y' || i == 'y')
				break;
			if (i == 'n' || i == 'N')
				print_at (23,0, "                                   ");

			edit_all ();
			if (restart)
				break;
		}
		/*----------------------
		| Update stock record. |
		----------------------*/
		if (!TR_DISPLAY)
			Update ();

		if (TR_RECEIPT || TR_R_CON || TR_I_CON)
			init_vars (ITEM_SCN);

#ifdef GVISION
		if (pipe_open)
		{
			pclose (pout);
			pipe_open = FALSE;
		}
#endif	/* GVISION */
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*==================================================
| Confirm Complete order and allow user to Change. |
==================================================*/
void
Confirm (
 int	issConf)
{
	scn_set (ITEM_SCN);

	move (1,2); cl_line ();
	print_at (2,1, ML (mlStdMess035));
	fflush (stdout);

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt);
		local_rec.qty_rec	= (float) local_rec.qty_rec_doub;
		local_rec.qty_ord	= (float) local_rec.qty_ord_doub;
		local_rec.qty_bord	= (float) local_rec.qty_bord_doub;
		local_rec.qty_ship	= (float) local_rec.qty_ship_doub;

		if (! (line_cnt % 20))
		{
			putchar ('C');
			fflush (stdout);
		}

		if (issConf)
		{
			local_rec.qty_ship = (float) local_rec.qty_ship_doub;
			local_rec.qty_ord = local_rec.qty_ship;
			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			SR.qtyDes = local_rec.qty_ord;

			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			if (FindCost ())
				continue;

		}
		else
		{
			local_rec.qty_ship		=	 (float) local_rec.qty_ship_doub;
			local_rec.qty_rec		=	local_rec.qty_ship;
			local_rec.qty_rec_doub	= 	 (double) local_rec.qty_rec;
		}
		if (SR.serialItem [0] == 'Y')
		{
			if (!strcmp (local_rec.serial_no,ser_space))
				strcpy (SR.errorFound, "Y");
			else
				strcpy (SR.errorFound, "N");
		}
		if (MULT_LOC || SK_BATCH_CONT)
		{
			if (!TR_RECEIPT)
			{
				cc = DisplayLL
				 (									/*----------------------*/
					line_cnt,						/*	Line number.		*/
					tab_row + 3 + (line_cnt % TABLINES),/*  Row for window	*/
					tab_col + 22,					/*  Col for window		*/
					4,								/*  length for window	*/
					SR.issHhwhHash, 				/*	Warehouse hash.		*/
					SR.hhumHash,					/*	UOM hash			*/
					issHhccHash,					/*	CC hash.			*/
					SR.UOM,						/* UOM					*/
					SR.qtyDes,					/* Quantity.			*/
					SR.cnvFct,					/* Conversion factor.	*/
					TodaysDate (),					/* Expiry Date.			*/
					TRUE,							/* Silent mode			*/
					FALSE,							/* Input Mode.			*/
					SR.lot_ctrl						/* Lot controled item. 	*/
													/*----------------------*/
				);
			}
			else
			{
				cc = DspLLTrans
				 (									/*----------------------*/
					line_cnt,						/*	Line number.		*/
					tab_row + 3 + (line_cnt % TABLINES),/*  Row for window	*/
					tab_col + 22,					/*  Col for window		*/
					4,								/*  length for window	*/
					SR.itffHash,
					SR.recHhwhHash,
					recHhccHash,					/*	CC hash.			*/
					SR.UOM,						/* UOM					*/
					SR.cnvFct,					/* Conversion factor.	*/
					SR.lot_ctrl,					/* Lot controled item. 	*/
					TRUE
				);
				if (cc)
				{
					cc = DisplayLL
					 (									
						line_cnt,							
						tab_row + 3 + (line_cnt % TABLINES),
						tab_col + 22,						
						4,									
						SR.recHhwhHash, 				
						SR.hhumHash,					
						recHhccHash,				
						SR.UOM,				
						local_rec.qty_rec,
						SR.cnvFct,	
						TodaysDate (),
						TRUE,			
						FALSE,
						SR.lot_ctrl				
					);
				}
			}
		}
		local_rec.qty_rec_doub	= (double) local_rec.qty_rec;
		local_rec.qty_ord_doub	= (double) local_rec.qty_ord;
		local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		local_rec.qty_ship_doub = (double) local_rec.qty_ship;
		SR.qtyRec				= (float)  local_rec.qty_rec;
		putval (line_cnt);
	}
}

int
CheckFreight (
	void)
{
	if (ithr_rec.frt_req [0] == 'Y' && ithr_rec.frt_cost == 0.00)
		return (EXIT_FAILURE); 
	if (ithr_rec.frt_req [0] == 'N')
	{
		ithr_rec.frt_cost = 0.00;
		DSP_FLD ("freight");
	}
	return (EXIT_SUCCESS);
}
	
	
int
CheckOk (
 int	serial)
{
	int		i;
	float	chk_qty;

	scn_set (2);

	for (i = 0; i < MAXLINES; i++)
	{
		getval (i);

		local_rec.qty_ord = (float) local_rec.qty_ord_doub;
		local_rec.qty_rec = (float) local_rec.qty_rec_doub;
		if (store [i].serialItem [0] == 'Y' && serial &&
		     store [i].errorFound [0] == 'Y' &&
		     local_rec.qty_ord > 0.00)
			return (i + 1);

		if (TR_RECEIPT)
			chk_qty = local_rec.qty_rec;
		else
			chk_qty = local_rec.qty_ord;

		if (store [i].serialItem [0] == 'N' && !serial &&
		     store [i].errorFound [0] == 'Y' &&
		     chk_qty > 0.00)
			return (i + 1);
	}

	return (SUCCESS);
}

void
ArgumentError (
 char	*pr_name)
{
	print_at (0,0, "Usage : %s <LPNO> <PID> <type> <formatFile> <duty>\n\r",
								pr_name);

	print_at (1,0, "\t\t<type> - RI (Remote Issue)\n\r");
	print_at (2,0, "\t\t       - ID (Issue Direct)\n\r");
	print_at (3,0, "\t\t       - IO (Issue One step)\n\r");
	print_at (4,0, "\t\t       - IT (Issue Two step)\n\r");
	print_at (5,0, "\t\t       - IR (Issue Request)\n\r");
	print_at (6,0, "\t\t       - RC (Request Confirm)\n\r");
	print_at (7,0, "\t\t       - IC (Issue Confirmation.)\n\r");
	print_at (8,0, "\t\t       - RE (Receipt.)\n\r");
	print_at (9,0, "\t\t       - TD (Transfer Display.)\n\r");
	print_at (10,0,"\t\t<formatFile> format file used by sk_trnprt\n\r");
	print_at (11,0,"\t\t<duty> - Y (es duty included) N (o not relevent.\n\r");

	return;

}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	recalc_sobg ();

#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (pout2);
		IP_CLOSE (np_fn);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	CloseDB (); 
	FinishProgram ();

#ifndef GVISION
	if (pipe_open)
		pclose (pout);
#endif	/* GVISION */

}

/*========================
| Open Data Dase Files . |
========================*/
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (ithr2, ithr);
	abc_alias (inum2, inum);
 
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (itgl, itgl_list, ITGL_NO_FIELDS, "itgl_id_no");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS,
		 (TR_DISPLAY) ? "ithr_id_no4" : "ithr_id_no");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
	open_rec ("ffdm", ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec ("inwd", inwd_list, INWD_NO_FIELDS, "inwd_id_no3");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,	comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);
	strcpy (StockTake, "Y");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (incc);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (itgl);
	abc_fclose (ithr);
	abc_fclose (ithr2);
	abc_fclose (itln);
	abc_fclose (inwu);
    abc_fclose (ffdm);
	abc_fclose (inwd);
	abc_fclose ("move");
	CloseLocation ();
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

/*==========================================
| Primary validation and file access here. |
==========================================*/
int
spec_valid (
 int field)
{
	int		i;
	int		TempLine;
	int		zero_keyed;
	int		this_page;
	int		ErrorWindow;

	/*---------------------------
	| Validate Transfer Number. |
	---------------------------*/
	if (LCHECK ("jnlDate"))
	{
		if (TR_ISSUE || TR_R_ISS)
		{
			if (chq_date (local_rec.transDate,comm_rec.inv_date))
				return (RETURN_ERR);
		}
		return (SUCCESS);
	}
	/*---------------------------
	| Validate Transfer Number. |
	---------------------------*/
	if (LCHECK ("transferNo"))
	{
		strcpy (ithr_rec.full_supply, "N");
		newTransfer	=	TRUE;

		if (last_char == EOI)
		{
			prog_exit = 1;
			return (SUCCESS);
		}
		if (TR_I_DIR || TR_R_ISS || TR_I_ONE || TR_I_RMT)
		{
			if (dflt_used || local_rec.tran_no == 0L)
			{
				if (!envVar.transferNumbering)
				{
					print_mess (ML (mlSkMess399));
					sleep (sleepTime);
					clear_mess ();
					return (RETURN_ERR);
				}
			}

			strcpy (ithr_rec.co_no , (MC_TRAN) ? "  " : comm_rec.co_no);

			ithr_rec.del_no = local_rec.tran_no;
			if (TR_R_ISS)
				strcpy (ithr_rec.type, "R");

			if (TR_I_DIR)
				strcpy (ithr_rec.type, "T");

			if (TR_I_TWO)
				strcpy (ithr_rec.type, "U");

			if (TR_I_ONE || TR_I_RMT)
				strcpy (ithr_rec.type, "M");

			if (ithr_rec.del_no > 0L)
			{
			   if (!find_rec (ithr,&ithr_rec,COMPARISON,"u"))
			   {
					abc_unlock (ithr);
					sprintf (err_str,ML (mlSkMess400), local_rec.tran_no);

					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (RETURN_ERR);
			   }
			}
		}
		if (TR_I_TWO)
		{
			if (dflt_used)
			{
				local_rec.tran_no = 0L;
				DSP_FLD ("transferNo");
			}
			else
			{
				print_mess (ML (mlSkMess401));
				sleep (sleepTime);
				local_rec.tran_no = 0L;
				DSP_FLD ("transferNo");
			}
		}
		if (TR_ISSUE || TR_R_ISS)
		{
			if (TR_ISSUE && !TR_I_RMT)
			{
				SetIssueBranch 
				 (
					comm_rec.co_no, 
					comm_rec.est_no, 
					comm_rec.cc_no,
					0L, 
					TRUE
				);

				DSP_FLD ("issCoNo");
				DSP_FLD ("issCoShort");
				DSP_FLD ("issBrNo");
				DSP_FLD ("issBrName");
				DSP_FLD ("issWhNo");
				DSP_FLD ("issWhName");
				if (!MC_TRAN)
				{
					DSP_FLD ("recCoNo");
					DSP_FLD ("recCoShort");
				}
				return (SUCCESS);
			}
			else
			{
				SetIssueBranch 
				 (
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					0L, 
					FALSE
				);

				DSP_FLD ("recCoNo");
				DSP_FLD ("recCoShort");
				DSP_FLD ("recBrNo");
				DSP_FLD ("recBrName");
				DSP_FLD ("recWhNo");
				DSP_FLD ("recWhName");
				if (!MC_TRAN)
				{
					DSP_FLD ("issCoNo");
					DSP_FLD ("issCoShort");
				}
				return (SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SetIssueBranch 
			 (
				comm_rec.co_no,
				comm_rec.est_no,
				comm_rec.cc_no,
				0L, 
				FALSE 
			);
			SrchIthr (temp_str);
			return (SUCCESS);
		}
		cc = LoadItems ();

		if (cc == 1)
		{
			scn_set (HEADER_SCN);
			print_mess (ML (mlStdMess103));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		if (cc == 2)
		{
			scn_set (HEADER_SCN);
			print_mess (ML (mlSkMess402));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		if (cc == 3)
		{
			scn_set (HEADER_SCN);
			if (MC_TRAN)
				print_mess (ML (mlSkMess403));
			else
				print_mess (ML (mlSkMess404));

			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		if (cc == 9)
		{
			scn_set (HEADER_SCN);
			if (MC_TRAN)
				print_mess (ML (mlSkMess405));
			else
				print_mess (ML (mlSkMess406));

			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		OpenTransportFiles ("trzm_id_no");
		
		strcpy (trzm_rec.co_no, issCo);
		strcpy (trzm_rec.br_no, issBr);
		strcpy (trzm_rec.del_zone, ithr_rec.del_zone);

		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (trzm_rec.del_zone, "      ");
			trzm_rec.trzm_hash	=	0L;
			trzm_rec.dflt_chg	=	0.0;
			trzm_rec.chg_kg		=	0.0;
		}
		strcpy (ithr_rec.del_zone,  trzm_rec.del_zone);
		strcpy (trcm_rec.carr_code, ithr_rec.carr_code);
		sprintf (trcm_rec.carr_desc,"%40.40s", " ");
		local_rec.efreight 	= 	0.00;
		trcm_rec.trcm_hash	=	0L;

		if (strcmp (ithr_rec.carr_code, "    ") && trzm_rec.trzm_hash > 0L)
		{
			strcpy (trcm_rec.co_no, issCo);
			strcpy (trcm_rec.br_no, issBr);
			strcpy (trcm_rec.carr_code, ithr_rec.carr_code);
			cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, "trcm", "DBFIND");
			
			trcl_rec.trcm_hash = trcm_rec.trcm_hash;
			trcl_rec.trzm_hash = trzm_rec.trzm_hash;
			cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "trcl", "DBFIND");

			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
		}
		CloseTransportFiles ();

		DSP_FLD ("issCoNo");
		DSP_FLD ("issCoShort");
		DSP_FLD ("issBrNo");
		DSP_FLD ("issBrName");
		DSP_FLD ("issWhNo");
		DSP_FLD ("issWhName");
		DSP_FLD ("recCoNo");
		DSP_FLD ("recCoShort");
		DSP_FLD ("recBrNo");
		DSP_FLD ("recBrName");
		DSP_FLD ("recWhNo");
		DSP_FLD ("recWhName");
		newTransfer	=	FALSE;

		return (SUCCESS);
	}
	/*----------------------------------
	| Validate Issuing Company number. |
	----------------------------------*/
	if (LCHECK ("issCoNo"))
	{
		if (!MC_TRAN || !TR_R_ISS)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (SUCCESS);
		}
		strcpy (comr_rec.co_no,issCo);
		cc = find_rec (comr,&comr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		strcpy (issCo,comr_rec.co_no);
		strcpy (issCoshort,comr_rec.co_short_name);
		strcpy (issCoAddr [0],comr_rec.co_adr1);
		strcpy (issCoAddr [1],comr_rec.co_adr2);
		strcpy (issCoAddr [2],comr_rec.co_adr3);
		DSP_FLD ("issCoShort");
		return (SUCCESS);
	}
	/*----------------------------------
	| Validate Issuing Company number. |
	----------------------------------*/
	if (LCHECK ("recCoNo"))
	{
		if (!MC_TRAN || !TR_R_ISS)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (SUCCESS);
		}
		strcpy (comr_rec.co_no,recCo);
		cc = find_rec (comr,&comr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		strcpy (recCo,comr_rec.co_no);
		strcpy (recCoshort,comr_rec.co_short_name);
		strcpy (recCoAddr [0],comr_rec.co_adr1);
		strcpy (recCoAddr [1],comr_rec.co_adr2);
		strcpy (recCoAddr [2],comr_rec.co_adr3);
		DSP_FLD ("recCoShort");
		return (SUCCESS);
	}
	/*-----------------------------------
	| Validate Issuing   branch number. |
	-----------------------------------*/
	if (LCHECK ("issBrNo"))
	{
		if (!TR_R_ISS  && !TR_I_RMT)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (SUCCESS);
		}

		strcpy (esmr_rec.co_no,issCo);
		strcpy (esmr_rec.est_no,issBr);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		strcpy (issBr,esmr_rec.est_no);
		strcpy (issBrname,esmr_rec.est_name);
		strcpy (issBrshort,esmr_rec.short_name);
		strcpy (issBrAddr [0],esmr_rec.adr1);
		strcpy (issBrAddr [1],esmr_rec.adr2);
		strcpy (issBrAddr [2],esmr_rec.adr3);
		DSP_FLD ("issBrName");

		return (SUCCESS);
	}

	/*------------------------------------
	| Validate Issuing warehouse number. |
	------------------------------------*/
	if (LCHECK ("issWhNo"))
	{
		if (!TR_R_ISS  && !TR_I_RMT)
			return (SUCCESS);

		if (!strcmp (recCo,issCo) &&
		     !strcmp (recBr,issBr) &&
		     !strcmp (recWh,issWh))
		{
			print_mess (ML (mlStdMess215));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		abc_selfield (ccmr,"ccmr_id_no");
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str, FALSE);
			return (SUCCESS);
		}

		strcpy (ccmr_rec.co_no,issCo);
		strcpy (ccmr_rec.est_no,issBr);
		strcpy (ccmr_rec.cc_no,issWh);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess215));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		issHhccHash = ccmr_rec.hhcc_hash;
		strcpy (issWh,ccmr_rec.cc_no);
		strcpy (issWhname,ccmr_rec.name);
		strcpy (issWhshort,ccmr_rec.acronym);
		ReadLLCT (ccmr_rec.hhcc_hash);
		DSP_FLD ("issWhName");
		return (SUCCESS);
	}
	/*-----------------------------------
	| Validate receiving branch number. |
	-----------------------------------*/
	if (LCHECK ("recBrNo"))
	{
		if (!TR_ISSUE || TR_I_RMT)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (SUCCESS);
		}

		strcpy (esmr_rec.co_no,recCo);
		strcpy (esmr_rec.est_no,recBr);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		strcpy (recBr,esmr_rec.est_no);
		strcpy (recBrname,esmr_rec.est_name);
		strcpy (recBrshort,esmr_rec.short_name);
		strcpy (recBrAddr [0],esmr_rec.adr1);
		strcpy (recBrAddr [1],esmr_rec.adr2);
		strcpy (recBrAddr [2],esmr_rec.adr3);
		strcpy (recArea,esmr_rec.area_code);
		DSP_FLD ("recBrName");
		return (SUCCESS);
	}

	/*--------------------------------------
	| Validate receiving warehouse number. |
	--------------------------------------*/
	if (LCHECK ("recWhNo"))
	{
		if (!TR_ISSUE || TR_I_RMT)
			return (SUCCESS);

		if (!strcmp (recCo,issCo) &&
		     !strcmp (recBr,issBr) &&
		     !strcmp (recWh,issWh))
		{
			print_mess (ML (mlStdMess215));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		abc_selfield (ccmr,"ccmr_id_no");
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str, TRUE);
			return (SUCCESS);
		}

		strcpy (ccmr_rec.co_no,recCo);
		strcpy (ccmr_rec.est_no,recBr);
		strcpy (ccmr_rec.cc_no,recWh);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess216));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		recHhccHash = ccmr_rec.hhcc_hash;
		strcpy (recWh,ccmr_rec.cc_no);
		strcpy (recWhname,ccmr_rec.name);
		strcpy (recWhshort,ccmr_rec.acronym);
		DSP_FLD ("recWhName");
		return (SUCCESS);
	}
	/*-----------------------------
	| Validate item number input. |
	-----------------------------*/
	if (LCHECK ("itemNumber"))
	{
		strcpy (local_rec.UOM, "    ");
		if (last_char == EOI)
		{
			prog_exit = 1;
			return (SUCCESS);
		}
		if (TR_RECEIPT || TR_R_CON || TR_I_CON)
		{
			if (ENTER_DATA)
			{
				getval (line_cnt);
				local_rec.qty_rec	= (float) local_rec.qty_rec_doub;
				local_rec.qty_ord	= (float) local_rec.qty_ord_doub;
				local_rec.qty_bord	= (float) local_rec.qty_bord_doub;
				local_rec.qty_ship	= (float) local_rec.qty_ship_doub;
			}
		}
		/*-------------------------
		| Search for part number. |
		-------------------------*/
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (SUCCESS);
		}
		return (ValidateItemNo ());
	}

	if (LCHECK ("itemDescription"))
	{
		if (NON_STOCK)
		{
			skip_entry = goto_field (field,label ("ln_full_supp"))+ 1;
			strcpy (local_rec.stock, "S");
			strcpy (SR._description, inmr_rec.description);
			DSP_FLD ("stock");
		}

		return (SUCCESS);
	}

	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used || F_NOKEY (field))
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			SR.hhumHash = inum_rec.hhum_hash;
			strcpy (SR.UOM, inum_rec.uom);
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (SUCCESS);
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
		strcpy (SR.UOM, inum2_rec.uom);
		SR.hhumHash 	= inum2_rec.hhum_hash;
		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;
		SR.cnvFct 	= inum2_rec.cnv_fct/inum_rec.cnv_fct;
		DSP_FLD ("UOM");
		return (SUCCESS);
	}

	/*--------------------------
	| Validate Quantity Input. |
	--------------------------*/
	if (LCHECK ("qtyOrder"))
	{
		local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);
		local_rec.qty_ord = (float) local_rec.qty_ord_doub;

		if (!ValidQuantity (local_rec.qty_ord_doub, SR.decPt))
			return (EXIT_FAILURE);

		if (TR_ISSUE || TR_R_CON || TR_I_CON)
		{
			if (ithr_rec.full_supply [0] == 'Y' &&
				local_rec.qty_ord > 0.00 &&	
				local_rec.qty_bord > 0.00)
			
			{
				print_mess (ML (mlSkMess407));
				sleep (sleepTime);
				return (RETURN_ERR);
			}
		}

		if (local_rec.qty_ord != 0.00)
		{
			local_rec.qty_bord = 0.00;
			local_rec.qty_bord_doub = 0.00;
			DSP_FLD ("qtyBackorder");
		}

		if (TR_RECEIPT)
			return (SUCCESS);

		if ((NON_STOCK || E_PHANTOM) && local_rec.qty_ord != 0.00)
		{
			local_rec.qty_ord = 0.00;
			local_rec.qty_ord_doub = 0.00;
			if (NON_STOCK)
				print_mess (ML (mlSkMess408));
			else
				print_mess (ML (mlSkMess409));
			sleep (sleepTime);
			return (SUCCESS);
		}

		local_rec.qty_ship = (float) local_rec.qty_ship_doub;
		if (dflt_used && (TR_R_CON || TR_I_CON))
		{
			local_rec.qty_ord = local_rec.qty_ship;
			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
		}

		if (local_rec.qty_ord != local_rec.qty_ship)
			transferChanged = (envVar.transferConfirm) ? TRUE : FALSE;

		if (local_rec.qty_ord == 0.00)
			zero_keyed = TRUE;
		else
			zero_keyed = FALSE;

		/*-------------------------------------------------------
		| Serial Items Can only have Qty of 0.00 or 1.00	|
		-------------------------------------------------------*/
		if (SER_COSTING  && local_rec.qty_ord != 0.00 &&
				     local_rec.qty_ord != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (SLEEP_TIME) ;
			return (RETURN_ERR);
		}
		/*-------------------------------------------------
		| Recalculate the actual current available stock. |
		-------------------------------------------------*/
		SR.issAvailable = ReCalcAvail ();

		if (envVar.windowPopupOk && ((SR.issAvailable - ToStdUom (local_rec.qty_ord)) < 0.00) &&
		     !NO_COST && !NON_STOCK && !KIT_ITEM &&
		     local_rec.qty_ord > 0.00)
		{
			sprintf (tmp_str [0], local_rec.rep_qty, SR.issAvailable);
			sprintf (err_str, ML ("Only %s available for part# %s"),clip (tmp_str [0]),inmr_rec.item_no);
			cc = warn_user (err_str);
			BusyFunction (1);

			if (InputResponse () && envVar.fullSupplyOrder &&
			     ithr_rec.full_supply [0] == 'N')
			{
				if (local_rec.hr_full_supply [0] == 'N')
				{
					sprintf (err_str, ML (mlSkMess410), recWhshort);
					i = prmptmsg (err_str,"YyNn",1,2);
					BusyFunction (0);
					if (i == 'Y' || i == 'y')
					{
						strcpy (ithr_rec.full_supply, "Y");
						strcpy (local_rec.hr_full_supply,"Y");
					}
					else
					{
						strcpy (ithr_rec.full_supply, "A");
						LineFullSupply ();
					}
				}
				else
					strcpy (ithr_rec.full_supply, "Y");
			}

			if (skip_entry != 0)
			{
				DSP_FLD ("itemNumber");
				DSP_FLD ("itemDescription");
				return (SUCCESS);
			}
			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);

			DSP_FLD ("qtyOrder");

		}
		IN_special = FALSE;

		if (KIT_ITEM || PHANTOM)
		{
			this_page = line_cnt / TABLINES;
			IN_special = TRUE;
			if (KIT_ITEM)
			{
				ProcessKitItem 
				 (
					inmr_rec.hhbr_hash, 
					local_rec.qty_ord
				);
			}
			else
			{
				ProcessPhantom 
				 (
					inmr_rec.hhbr_hash,
					local_rec.qty_ord,
					local_rec.qty_bord
				);
			}

			skip_entry = goto_field (label ("qtyOrder"), label ("itemNumber"));

			local_rec.qty_ord = 0.00;
			local_rec.qty_ord_doub = 0.00;
			if (this_page == (line_cnt / TABLINES))
				blank_display ();

			IN_special = FALSE;

			return (SUCCESS);
		}

		if (ENTER_DATA)
		{
			if (local_rec.qty_ord == 0.00)
			{
				if (SER_COSTING || zero_keyed)
					skip_entry = 1;
				else
					skip_entry = 2;
			}
			else
				skip_entry = 2;

			DSP_FLD ("qtyBackorder");
		}

		if (TR_R_CON || TR_I_CON)
			skip_entry = 1;

		if (local_rec.qty_ord == 0.00 && SER_COSTING &&
		     !BLANK_SER && prog_status != ENTRY)
		{
			sprintf (SR.serial,"%-25.25s"," ");
			sprintf (local_rec.serial_no,"%-25.25s"," ");
			sprintf (local_rec.location,"%-10.10s"," ");
			print_mess (ML (mlSkMess411));

			if (!TR_R_ISS)
				DSP_FLD ("ser_no");
		}
		cc = FindCost ();
		if (cc)
		{
			skip_entry = -2;
			return (SUCCESS);
		}
		if (local_rec.qty_ord == 0.00 && !TR_R_ISS && !F_HIDE (label ("LL")))
			FLD ("LL") = NI;

        if (prog_status != ENTRY && (TR_I_CON || TR_I_DIR))
		{
			DSP_FLD ("qtyOrder");
			SR.qtyDes = local_rec.qty_ord;                  
			/*-------------------
			| Reenter Location. |
			--------------------*/
			do
			{
				strcpy (local_rec.LL, "N");
				if (FLD ("LL") != ND)
				{
					get_entry (label ("LL"));
					cc = spec_valid (label ("LL"));
				}				
			} while (cc && !restart);
		}

		SR.qtyDes = local_rec.qty_ord;
	 	local_rec.qty_ord_doub = (double) local_rec.qty_ord; 

		return (SUCCESS);
	}
	/*-------------------------------
	| Validate Quantity Despatched. |
	-------------------------------*/
	if (LCHECK ("qtyReceive"))
	{
		local_rec.qty_rec_doub	= n_dec (local_rec.qty_rec_doub, SR.decPt);
		local_rec.qty_rec		= (float) local_rec.qty_rec_doub;
		SR.qtyRec				= (float) local_rec.qty_rec_doub;

		if (!ValidQuantity (local_rec.qty_rec_doub, SR.decPt))
			return (EXIT_FAILURE);

		if (!TR_RECEIPT)
			return (SUCCESS);

		local_rec.qty_ship = (float) local_rec.qty_ship_doub;
		if (dflt_used)
		{
			local_rec.qty_rec = local_rec.qty_ship;
			local_rec.qty_rec_doub = (double) local_rec.qty_rec;
		}

		if (local_rec.qty_rec > local_rec.qty_ship)
		{
				
			sprintf (tmp_str [0], local_rec.rep_qty, local_rec.qty_ship);
			sprintf (tmp_str [1], local_rec.rep_qty, local_rec.qty_rec);

			if (MULT_LOC || SK_BATCH_CONT)
			{
				print_mess (ML ("Receipt quantity cannot be greater that issue quantity"));
				sleep (sleepTime);
				return (RETURN_ERR);
			}
			sprintf (err_str, ML (mlSkMess412),	clip (tmp_str [1]), 
												clip (tmp_str [0])); 

			i = prmptmsg (err_str,"YyNn",1,2);
			if (i == 'n' || i == 'N')
				return (RETURN_ERR);
			move (1,2);cl_line ();
		}
		if (SER_COSTING)
		{
			if (local_rec.qty_rec != 0.00 &&
			     local_rec.qty_rec != 1.00)
			{
				print_mess (ML (mlStdMess029));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			if (local_rec.qty_bord == 1.00 &&
			     local_rec.qty_ord != 0.00)
			{
				print_mess (ML (mlSkMess413));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			skip_entry = goto_field (label ("ser_no"),label ("ln_full_supply"));
		}

		if (!TR_R_ISS)
			DSP_FLD ("ser_no");

		if (prog_status != ENTRY)
		{
			DSP_FLD ("qtyReceive");

			/*-------------------
			| Reenter Location. |
			--------------------*/
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				cc = spec_valid (label ("LL"));
			} while (cc && !restart);
		}
		SR.qtyRec				= (float) local_rec.qty_rec_doub;
		return (SUCCESS);
	}

	/*--------------------------------
	| Validate Quantity Backordered. |
	--------------------------------*/
	if (LCHECK ("qtyBackorder"))
	{
		if (TR_RECEIPT)
			return (SUCCESS);

		if ((NON_STOCK || E_PHANTOM) && local_rec.qty_bord_doub != 0.00)
		{
			local_rec.qty_bord_doub = 0.00;
			if (NON_STOCK)
				print_mess (ML (mlSkMess408));
			else
				print_mess (ML (mlSkMess409));
			sleep (sleepTime);
			clear_mess ();
			return (SUCCESS);
		}

		local_rec.qty_rec = (float) local_rec.qty_rec_doub;
		local_rec.qty_ship = (float) local_rec.qty_ship_doub;
		local_rec.qty_ord = (float) local_rec.qty_ord_doub;
		local_rec.qty_bord = (float) local_rec.qty_bord_doub;
		if (dflt_used && (TR_R_CON || TR_I_CON))
		{
			local_rec.qty_bord = local_rec.qty_rec +
				 (local_rec.qty_ship - local_rec.qty_ord);
			if (local_rec.qty_bord < 0.00)
				local_rec.qty_bord = 0.00;
			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		}

		if (local_rec.qty_bord < 0.00)
			local_rec.qty_bord = 0.00;

		if (local_rec.qty_bord != local_rec.qty_rec)
			transferChanged = (envVar.transferConfirm) ? TRUE : FALSE;

		if (FULL_BO)
		{
			if (local_rec.qty_ord != 0.00 &&
		        local_rec.qty_bord != 0.00)
			{
				print_mess (ML (mlSkMess407));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
		}

		if (!BO_OK)
		{
			if (local_rec.qty_bord != 0.00)
			{
				print_mess (ML (mlSkMess407));
				sleep (sleepTime);
				local_rec.qty_bord = 0.00;
				DSP_FLD ("qtyBackorder");
				clear_mess ();
				return (RETURN_ERR);
			}
		}

		if (TR_ISSUE || TR_R_CON || TR_I_CON)
		{
			if (ithr_rec.full_supply [0] == 'Y'
			&&  local_rec.qty_ord  != 0.00 
			&&  local_rec.qty_bord != 0.00)
			{
				print_mess (ML (mlSkMess407));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
		}

		/*-------------------------------------------------------
		| Serial Items Can only have Qty of 0.00 or 1.00	|
		-------------------------------------------------------*/
		if (SER_COSTING)
		{
			if (local_rec.qty_ord == 1.00 && local_rec.qty_bord != 0.00)
			{
				print_mess (ML (mlSkMess414));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			if (local_rec.qty_bord != 0.00 && local_rec.qty_bord != 1.00)
			{
				print_mess (ML (mlStdMess029));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			if (local_rec.qty_ord == 0.00 && local_rec.qty_bord == 0.00)
			{
				print_mess (ML (mlSkMess415));
				sleep (sleepTime);
				clear_mess ();
				return (RETURN_ERR);
			}
			if (!TR_R_ISS)
			{
				if (local_rec.qty_bord == 1.00)
					FLD ("ser_no") = NI;
				else
					FLD ("ser_no") = (BLANK_SER) ? YES : NI;
			}
		}
		if (local_rec.qty_ord == 0.00 && !TR_R_ISS && !F_HIDE (label ("LL")))
			FLD ("LL") = NI;

		return (SUCCESS);
	}
	/*-----------------
	| Validate Cost . |
	-----------------*/
	if (LCHECK ("cost"))
	{
		if (F_HIDE (label ("cost")))
			return (SUCCESS);

		if (dflt_used && FLD ("cost") == YES)
		{
			if (FindCost ())
			{
				print_mess (ML (mlSkMess126));
				sleep (sleepTime);
				skip_entry -= 5;
				return (SUCCESS);
			}
		}
		return (SUCCESS);
	}
	/*-----------------
	| Validate Duty . |
	-----------------*/
	if (LCHECK ("duty"))
	{
		double	UpliftValue	=	0.00;

		if (SR.upft_pc > 0.00)
		{
			UpliftValue	=	 (double) SR.upft_pc;
			UpliftValue	*=	local_rec.workCost;
			UpliftValue	=	DOLLARS (UpliftValue);
			UpliftValue	=	twodec (UpliftValue);
		}
		if (SR.upft_amt > 0.00)
			UpliftValue	+=	DOLLARS (SR.upft_amt);

		if (UpliftValue != 0.00)
			local_rec.workDuty	=	twodec (UpliftValue);

		return (SUCCESS);
	}
	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("ser_no"))
	{
		if (!SERIAL)
			return (SUCCESS);

		if (TR_R_ISS)
			return (SUCCESS);

		if (FLD ("ser_no") == NA)
			return (SUCCESS);

		if (SRCH_KEY)
		{
			SearchInsf (SR.issHhwhHash, "F", temp_str);
			return (SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.serial_no, ser_space))
		{
			if (TR_I_RMT || TR_I_ONE || TR_I_TWO || local_rec.qty_ord == 0.00)
			{
				DSP_FLD ("ser_no");
				return (SUCCESS);
			}
		}

		if (local_rec.qty_ord == 0.00)
		{
			print_mess (ML (mlSkMess416));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		local_rec.workCost	=	FindInsfCost
								(
									SR.issHhwhHash,
									0L,
									local_rec.serial_no,
									"F"
								);

		if (local_rec.workCost == -1.00 &&
		     (CheckInsf (SR.hhbrHash, local_rec.serial_no) ||
		       !strcmp (local_rec.serial_no, SR.origSerialNo)))
		{
			local_rec.workCost	=	FindInsfCost
									(
										SR.issHhwhHash,
										0L,
										local_rec.serial_no,
										"C"
									);
		}
		if (local_rec.workCost == -1.00)
		{
			if (local_rec.qty_ord == 0.00)
				return (SUCCESS);

			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (insfRec.receipted [0] != 'Y')
		{
			sprintf (err_str, ML (mlSkMess343), " ");
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}

		if (CheckDuplicateInsf (local_rec.serial_no,SR.hhbrHash,line_cnt))
		{
			print_mess (ML (mlSkMess559));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		strcpy (local_rec.location, insfRec.location);

		DSP_FLD ("cost");
		DSP_FLD ("LL");
		sprintf (SR.serial,"%-25.25s",local_rec.serial_no);

		strcpy (SR.errorFound, "N");
		DSP_FLD ("ser_no");
		return (SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (SUCCESS);

		if (TR_R_ISS)
			return (SUCCESS);

		if (ENTER_DATA && (FLD ("LL") == NI || FLD ("LL") == NA))
			return (SUCCESS);

		if (!TR_RECEIPT)
		{
			TempLine	=	lcount [ITEM_SCN];
			LL_EditLoc	=	 (MULT_LOC) ? TRUE : FALSE;
			ErrorWindow = DisplayLL
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.issHhwhHash, 					/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				issHhccHash,						/*	CC hash.			*/
				SR.UOM,							/* UOM					*/
				SR.qtyDes,						/* Quantity.			*/
				SR.cnvFct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				INP_VIEW,							/* Silent mode			*/
				 (local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR.lot_ctrl							/* Lot controled item. 	*/
													/*----------------------*/
			);
			/*-----------------
			| Redraw screens. |
			-----------------*/
			strcpy (local_rec.LL, "Y");
			putval (line_cnt);

			lcount [ITEM_SCN] = (line_cnt + 1 > lcount [ITEM_SCN]) ? line_cnt + 1 : lcount [ITEM_SCN];
			scn_write (ITEM_SCN);
			scn_display (ITEM_SCN);
			lcount [ITEM_SCN] = TempLine;
			PrintCoStuff ();
			if (ErrorWindow)
				return (EXIT_FAILURE);
		
			return (SUCCESS);
		}
		else
		{
			LL_EditLoc	=	 (MULT_LOC) ? TRUE : FALSE;
			LL_EditLot	=	 (SR.lot_ctrl [0] == 'Y') ? TRUE : FALSE;
			LL_EditDate	=	TRUE;
			LL_EditSLot	=	 (SR.lot_ctrl [0] == 'Y') ? TRUE : FALSE;
			
			TempLine	=	lcount [ITEM_SCN];
			ErrorWindow = DspLLTrans
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.itffHash,						/*						*/
				SR.recHhwhHash,						/*						*/
				recHhccHash,						/*	CC hash.			*/
				SR.UOM,							/* UOM					*/
				SR.cnvFct,						/* Conversion factor.	*/
				SR.lot_ctrl,						/* Lot controled item. 	*/
				TRUE								/*						*/
													/*----------------------*/
			);
			LLInputClear	=	FALSE;
			IgnoreTotal		=	FALSE;
			strcpy (local_rec.LL, (ErrorWindow) ? "N" : "Y");
			ErrorWindow = DisplayLL
			 (									
				line_cnt,							
				tab_row + 3 + (line_cnt % TABLINES),
				tab_col + 22,						
				4,									
				SR.recHhwhHash, 				
				SR.hhumHash,					
				recHhccHash,				
				SR.UOM,				
				SR.qtyRec,
				SR.cnvFct,	
				TodaysDate (),
				FALSE,			
				 (local_rec.LL [0] == 'Y'),
				SR.lot_ctrl				
			);
			/*-----------------
			| Redraw screens. |
			-----------------*/
			putval (line_cnt);

			lcount [ITEM_SCN] = (line_cnt + 1 > lcount [ITEM_SCN]) ? line_cnt + 1 : lcount [ITEM_SCN];
			scn_write (ITEM_SCN);
			scn_display (ITEM_SCN);
			lcount [ITEM_SCN] = TempLine;
			PrintCoStuff ();
			if (ErrorWindow)
				return (EXIT_FAILURE);
		}
		return (SUCCESS);
	}

	/*---------------------------------
	| Validate Freight Required Flag. |
	---------------------------------*/
	if (LCHECK ("freightRequired"))
	{
		if (F_NOKEY (field))
			return (SUCCESS);

		if (dflt_used)
			strcpy (ithr_rec.frt_req, (envVar.automaticFreight) ? "Y" : "N");

		if (ithr_rec.frt_req [0] == 'N')
			ithr_rec.frt_cost = 0.00;
		
		DSP_FLD ("freightRequired");
		return (SUCCESS);
	}

	/*------------------------
	| Validate Carrier Code. |
	------------------------*/
	if (LCHECK ("carrierCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				ithr_rec.frt_cost = 0.00;

			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			trcm_rec.trcm_hash	=	0L;
			return (SUCCESS);
		}
			
		OpenTransportFiles ("trzm_trzm_hash");

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			CloseTransportFiles ();
			return (SUCCESS);
		}
			
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
		if (!cc)
		{
			trcl_rec.trcm_hash = trcm_rec.trcm_hash;
			trcl_rec.trzm_hash = trzm_rec.trzm_hash;
			cc = find_rec (trcl, &trcl_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess134));
				sleep (sleepTime);
				clear_mess ();

				CloseTransportFiles ();
				trcm_rec.trcm_hash	=	0L;
				return (RETURN_ERR);
			}
			CloseTransportFiles ();

			DSP_FLD ("carr_desc");

			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			return (SUCCESS);
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		clear_mess ();

		CloseTransportFiles ();
		return (RETURN_ERR);
	}

	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("deliveryZoneCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				ithr_rec.frt_cost = 0.00;

			strcpy (trzm_rec.del_zone, "      ");
			strcpy (trzm_rec.desc, "      ");
			trzm_rec.trzm_hash	=	0L;
			trzm_rec.dflt_chg	=	0.0;
			trzm_rec.chg_kg		=	0.0;
			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			DSP_FLD ("deliveryZoneDesc");
			DSP_FLD ("deliveryZoneCode");
			return (SUCCESS);
		}
		OpenTransportFiles ("trzm_id_no");

		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			CloseTransportFiles ();
			return (SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			CloseTransportFiles ();
			return (RETURN_ERR); 
		}
		if (trcm_rec.trcm_hash > 0L)
		{
			strcpy (trcm_rec.co_no, comm_rec.co_no);
			strcpy (trcm_rec.br_no, comm_rec.est_no);
			cc = find_rec (trcm, &trcm_rec, COMPARISON,"r");
			if (!cc)
			{
				trcl_rec.trcm_hash = trcm_rec.trcm_hash;
				trcl_rec.trzm_hash = trzm_rec.trzm_hash;
				cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess134));
					sleep (sleepTime);
					clear_mess ();

					CloseTransportFiles ();
					return (RETURN_ERR);
				}
				CloseTransportFiles ();

				CalculateFreight 
				 (
					trcm_rec.markup_pc, 
					trcl_rec.cost_kg,
					trzm_rec.chg_kg,
					trzm_rec.dflt_chg
				);

				DSP_FLD ("est_freight");
				DSP_FLD ("freight");
				DSP_FLD ("tot_kg");
				return (SUCCESS);
			}
		}
		DSP_FLD ("deliveryZoneDesc");
		DSP_FLD ("deliveryZoneCode");

		CloseTransportFiles ();
		return (SUCCESS);
	}

	/*-------------------
	| Validate freight. |
	-------------------*/
	if (LCHECK ("freight"))
	{
		if (dflt_used && FREIGHT_CHG)
		{
			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg,
				trzm_rec.chg_kg,
				trzm_rec.dflt_chg
			);
				
			DSP_FLD ("freight");
			DSP_FLD ("est_freight");
			DSP_FLD ("tot_kg");
			return (SUCCESS);
		}
		else
		{
			if (ithr_rec.frt_cost == 0.00)
				strcpy (ithr_rec.frt_req, "N");
		}
		return (SUCCESS);
	}
	/*-----------------------------
	| Validate delivery required. |
	-----------------------------*/
	if (LCHECK ("deliveryRequired"))
	{
		if (!newTransfer && ithr_rec.del_req [0] == 'Y')
		{
			move (0,2);cl_line ();
			i = prmptmsg (ML (mlTrMess063) ,"YyNn",0,2);
			BusyFunction (0);
			if (i == 'N' || i == 'n') 
				return (SUCCESS);

			sprintf (err_str,"tr_trsh_mnt T %010ld LOCK",ithr_rec.hhit_hash);
			sys_exec (err_str);
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhit_hash");
			trsh_rec.hhit_hash	=	ithr_rec.hhit_hash;
			cc = find_rec (trsh, &trsh_rec, COMPARISON, "u");
			if (!cc)
			{
				strcpy (ithr_rec.s_timeslot, trsh_rec.sdel_slot);
				strcpy (ithr_rec.e_timeslot, trsh_rec.edel_slot);
				ithr_rec.del_date	=	trsh_rec.del_date;
				cc = abc_update (ithr, &ithr_rec);
				if (cc)
					file_err (cc, "ithr", "DBUPDATE");
			}
			abc_unlock ("ithr");
			heading (FREIGHT_SCN);
			scn_write (FREIGHT_SCN);
			scn_display (FREIGHT_SCN);
			print_mess (ML (mlTrMess076));
			sleep (sleepTime);
			DSP_FLD ("deliveryDate");
		}
		return (SUCCESS); 
	}
	return (SUCCESS);
}

/*------------------------------------------
| Recalculate the current available stock. |
------------------------------------------*/
float 
ReCalcAvail (
 void)
{
	float	realStock;

	/*----------------------
	| Look up incc record. |
	----------------------*/
    incc2_rec.hhcc_hash = issHhccHash;
    incc2_rec.hhbr_hash = SR.hhbrHash;
    cc = find_rec (incc, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	realStock = incc2_rec.closing_stock -
				incc2_rec.committed -
				incc2_rec.backorder - 
				incc2_rec.forward;

	if (envVar.IKHK && !envVar.soFwdAvl)
		realStock = incc2_rec.closing_stock -
					incc2_rec.committed -
					incc2_rec.backorder; 

	if (envVar.qcApplied && envVar.qCAvailable)
		realStock -= incc2_rec.qc_qty;

	/*------------------------------------------------------------
	| Add into available any stock that was on line when loaded. |
	------------------------------------------------------------*/
	if (SR.hhbrHash == SR.origHhbrHash)
		realStock += SR.origOrderQty;

	return (realStock);
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
 double	_qty,
 int	_dec_pt)
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
		sprintf (tmp_str [0], local_rec.rep_qty, _qty);
		sprintf (tmp_str [1], local_rec.rep_qty, compare [_dec_pt]);
		sprintf (err_str, ML (mlSkMess238), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

int
ValidateItemNo (
 void)
{
	int		i;
	char	*sptr;

	abc_selfield (inmr, "inmr_id_no");

	skip_entry = 0;

	cc = FindInmr (issCo, inmr_rec.item_no, 0L, "N");
	if (!cc)
	{
		strcpy (inmr_rec.co_no, issCo);
		strcpy (inmr_rec.item_no, inmr_rec.item_no);
		cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		errmess (ML (mlStdMess001));
		sleep (sleepTime);
		return (RETURN_ERR);
	}
	if (!TR_RECEIPT && !TR_R_CON && !TR_I_CON)
	{
		strcpy (itln_rec.item_desc,inmr_rec.description);
	}

	if (prog_status != ENTRY)
	{
		if (inmr_rec.hhbr_hash == SR.hhbrHash)
			return (SUCCESS);
		if (inmr_rec.inmr_class [0] == 'K')
		{
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	
	SR.hhbrHash = inmr_rec.hhbr_hash;
	strcpy (SR.issBackOrder, inmr_rec.bo_flag);
	SR.weight = inmr_rec.weight;

	SuperSynonymError ();

	if (MC_TRAN)
	{
		cc = FindInmr (recCo, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlSkMess433));
			sleep (sleepTime);
			clear_mess ();
			return (RETURN_ERR);
		}
		if (!TR_RECEIPT && !TR_R_CON && !TR_I_CON)
			strcpy (itln_rec.item_desc,inmr_rec.description);


		SR.recHhbrHash = inmr_rec.hhbr_hash;
		strcpy (SR.recBackOrder, inmr_rec.bo_flag);

		SuperSynonymError ();
	}
	else
	{
		SR.recHhbrHash = inmr_rec.hhbr_hash;
		strcpy (SR.recBackOrder, inmr_rec.bo_flag);
	}

	/*---------------------
	| Find for UOM GROUP. |
	----------------------*/
	strcpy (inum_rec.uom, inmr_rec.sale_unit);
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");
	
	SR.hhumHash 	= inum_rec.hhum_hash;
	SR.cnvFct		= inum_rec.cnv_fct;
	strcpy (SR.UOM, inum_rec.uom);
	strcpy (local_rec.UOM, inum_rec.uom);

	strcpy (local_rec.ln_full_supp, "N");
	strcpy (SR.storeClass, inmr_rec.inmr_class);
	strcpy (SR._description, inmr_rec.description);
	DSP_FLD ("itemNumber");
	DSP_FLD ("itemDescription");
	DSP_FLD ("UOM");

	if (TR_RECEIPT || TR_R_CON || TR_I_CON)
	{
		FLD ("ser_no") = (SERIAL && BLANK_SER) ? YES : NA;
		if (NON_STOCK)
			skip_entry = goto_field (label ("itemNumber"),label ("ln_full_supp")) + 1;
		else
			skip_entry = 2;

		if (TR_RECEIPT)
			return (SUCCESS);
	}

	/*--------------------------------------------------
	| Look up to see if item is on issuing Cost Centre |
	--------------------------------------------------*/
	incc_rec.hhcc_hash = issHhccHash;
	incc_rec.hhbr_hash = SR.hhbrHash;

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
	{
		errmess (ML (mlSkMess434));
		sleep (sleepTime);
		clear_mess ();
		return (RETURN_ERR);
	}
	if (!TR_RECEIPT)
	{
		strcpy (excf_rec.co_no,  inmr_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			excf_rec.hhcf_hash	=	0L;

		/*-------------------------------------------------------------------
		| Check for warehouse supply structure to see if demand is updated. |
		-------------------------------------------------------------------*/
		inwd_rec.hhcc_hash	=	recHhccHash;
		inwd_rec.hhbr_hash	=	SR.recHhbrHash;
		inwd_rec.hhcf_hash	=	0L;
		cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
		if (cc)
		{
			inwd_rec.hhcc_hash	=	recHhccHash;
			inwd_rec.hhbr_hash	=	0L;
			inwd_rec.hhcf_hash	=	excf_rec.hhcf_hash;
			cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
			if (cc)
			{
				inwd_rec.hhcc_hash	=	recHhccHash;
				inwd_rec.hhbr_hash	=	0L;
				inwd_rec.hhcf_hash	=	0L;
				cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			strcpy (SR.Demand, "N");
			SR.upft_pc		=	0.00;
			SR.upft_amt		=	0.00;
		}
		else
		{
			strcpy (SR.Demand, 	inwd_rec.demand);
			SR.upft_pc		=	inwd_rec.upft_pc;
			SR.upft_amt		=	inwd_rec.upft_amt;
		}
	}
	strcpy (SR.lot_ctrl, inmr_rec.lot_ctrl);
	strcpy (local_rec.lot_ctrl, inmr_rec.lot_ctrl);

	if (TR_I_RMT || TR_I_DIR || TR_R_CON || TR_I_CON)
		tab_other (line_cnt);
	
	SR.issHhwhHash = incc_rec.hhwh_hash;
	SR.issClosing   = incc_rec.closing_stock;
	if (!TR_RECEIPT && !TR_R_CON && !TR_I_CON)
	{
		SR.issAvailable = incc_rec.closing_stock -
					 incc_rec.backorder -
					 incc_rec.committed -
					 incc_rec.forward;

		if (envVar.IKHK && !envVar.soFwdAvl)
			SR.issAvailable = incc_rec.closing_stock -
						 incc_rec.backorder -
						 incc_rec.committed;

		if (envVar.qcApplied && envVar.qCAvailable)
			SR.issAvailable = incc_rec.qc_qty;

		if (PHANTOM)
			SR.issAvailable = phantom_avail (inmr_rec.hhbr_hash);
	}

	/*----------------------------------------------------
	| Look up to see if item is on receiving Cost Centre |
	----------------------------------------------------*/
	incc_rec.hhcc_hash = recHhccHash;
	incc_rec.hhbr_hash = SR.recHhbrHash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
	{
		if (IN_special)
		{
			cc = AddIncc (SR.recHhbrHash, recHhccHash);
			if (cc)
				file_err (cc, "incc", "DBADD");
			BusyFunction (0);
		}
		else
		{
			i = prmptmsg (ML (mlSkMess435) , "YyNn",1,2);

			if (i == 'n' || i == 'N')
			{
				skip_entry = -1 ;
				BusyFunction (0);
				return (SUCCESS);
			}
			else
			{
				cc = AddIncc (SR.recHhbrHash, recHhccHash);
				if (cc)
					file_err (cc, "incc", "DBADD");
				BusyFunction (0);
			}
			clear_mess ();
		}
	}
	SR.recHhwhHash = incc_rec.hhwh_hash;

	DSP_FLD ("itemDescription");
	strcpy (SR.costingFlag,inmr_rec.costing_flag);
	strcpy (SR.serialItem, inmr_rec.serial_item);
	SR.decPt = inmr_rec.dec_pt;

	if (!TR_R_ISS)
	{
		FLD ("ser_no") = (SERIAL) ? YES : NA;
		if (!F_HIDE (label ("LL")))
			FLD ("LL")    = (MULT_LOC || SK_BATCH_CONT) ? YES : ND;
	}

	if (!F_HIDE (label ("UOM")))
		FLD ("UOM") = (SERIAL) ? NA : YES;

	FLD ("qtyOrder") = YES;

	if (prog_status != ENTRY)
	{
		SR.qtyDes 			= 0.00;
		local_rec.qty_ord 		= 0.00;
		local_rec.qty_ord_doub 	= 0.00;
		local_rec.qty_bord 		= 0.00;
		local_rec.qty_bord_doub = 0.00;
		local_rec.workCost 		= 0.00;
		sprintf (local_rec.serial_no, "%-25.25s", " ");
		sprintf (SR.serial, "%-25.25s", " ");
		sprintf (SR.origSerialNo, "%-25.25s", " ");
		sprintf (local_rec.location, "%-10.10s", " ");
		sprintf (SR.issLocation, "%-10.10s", " ");
		sprintf (SR.recLocation, "%-10.10s", " ");
		DSP_FLD ("qtyOrder");
		DSP_FLD ("qtyBackorder");
		DSP_FLD ("cost");
		DSP_FLD ("ser_no");
		DSP_FLD ("LL");

		if (SERIAL)
		{
			local_rec.qty_ord 		= 1.00;
			local_rec.qty_ord_doub 	= 1.00;
			DSP_FLD ("qtyOrder");
			cc = spec_valid (label ("qtyOrder"));
			while (cc && !restart && !skip_entry)
			{
				get_entry (label ("qtyOrder"));
				cc = spec_valid (label ("qtyOrder"));
			}
			if (skip_entry)
				return (EXIT_FAILURE);

			do
			{
				get_entry (label ("ser_no"));
				cc = spec_valid (label ("ser_no"));
			} while (cc && !restart);
			DSP_FLD ("ser_no");
		}
		else
		{
			do
			{
				get_entry (label ("qtyOrder"));
				cc = spec_valid (label ("qtyOrder"));
			} while (cc && !restart && !skip_entry);
			DSP_FLD ("qtyOrder");
			if (skip_entry)
				return (EXIT_FAILURE);
		}
	}

	if (TR_R_CON || TR_I_CON)
		return (SUCCESS);

	if (NON_STOCK)
		skip_entry = goto_field (label ("itemNumber"),label ("ln_full_supp"));
	else
		skip_entry = 2;

	sptr = clip (inmr_rec.description);
	if (strlen (sptr) == 0)
		skip_entry = 1;

	return (SUCCESS);
}


/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.							    |
| Return 1 if duplicate								    |
=======================================================*/
int
CheckDuplicateInsf (
 char	*serial_no,
 long	hhbrHash,
 int	line_no)
{
	int		i;
	int		no_lines = 0;

	if (!SER_COSTING)
		return (SUCCESS);

	no_lines = (ENTER_DATA) ? line_cnt : lcount [ITEM_SCN];

	for (i = 0;i < no_lines; i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------------
		| Only compare serial numbers for		|
		| the same item number					|
		---------------------------------------*/
		if (store [i].hhbrHash == hhbrHash)
			if (!strcmp (store [i].serial,serial_no))
				return (RETURN_ERR);
	}
	return (SUCCESS);
}

int
FindCost (
 void)
{
	switch (SR.costingFlag [0])
	{
	case 'A':
	case 'L':
	case 'T':
	case 'P':
		local_rec.workCost	=	FindIneiCosts
								(
									SR.costingFlag,
									issBr,
									SR.hhbrHash
								);
		break;

	case 'F':
		local_rec.workCost	=	FindIncfCost 
								(
									SR.issHhwhHash, 
									SR.issClosing,
									local_rec.qty_ord,
									TRUE,
									SR.decPt
								);
		break;

	case 'I':
		local_rec.workCost	=	FindIncfCost 
								(
									SR.issHhwhHash, 
									SR.issClosing,
									local_rec.qty_ord,
									FALSE,
									SR.decPt
								);
		break;

	case 'S':
		local_rec.workCost = FindInsfValue (SR.issHhwhHash,TRUE);
		break;
	}
	if (local_rec.workCost < 0.00)
		local_rec.workCost	=	FindIneiCosts ("L", issBr, SR.hhbrHash);

	SR.fileCost = twodec (local_rec.workCost);

	return (SUCCESS);
}

/*================================
| Update incc & add transaction. |
================================*/
void
Update (
 void)
{
	int		all_delete = TRUE;
	int		zero_all = TRUE;
	int		bord_all = FALSE;
	int		bord_any = FALSE;
	int		freight_posted = FALSE;
	double	value;
	double	duty;

	if (!lcount [2])
	{
		print_mess (ML (mlSkMess432));
		sleep (sleepTime);
		clear_mess ();
		return;
	}

	clear ();
	print_at (0,0,  ML (mlStdMess035));

	fflush (stdout);

	if (!TR_RECEIPT && !TR_R_CON && !TR_I_CON)
	{
		if (!TR_I_TWO)
		{
		    if (envVar.transferNumbering)
		    {
				open_rec (ithr2, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");

				strcpy (comr_rec.co_no,comm_rec.co_no);
				cc = find_rec (comr,&comr_rec,COMPARISON,"u");
				if (cc)
					file_err (cc, "comr", "DBFIND");

				/*----------------------------------------
				| Check if transfer no hash already been |
  				| allocated, if it has then skip number. |
				----------------------------------------*/
				while (CheckIthr (++comr_rec.nx_del_no) == 0);

				cc = abc_update (comr,&comr_rec);
				if (cc)
					file_err (cc, "comr", "DBUPDATE");

				sprintf (userReferenceNo,"%08ld",comr_rec.nx_del_no);

				abc_fclose (ithr2);
		    }
		    else
		    {
				comr_rec.nx_del_no = local_rec.tran_no;
				sprintf (userReferenceNo,"%08ld",comr_rec.nx_del_no);
		    }
		}
		else
		{
			comr_rec.nx_del_no = 900000L + (long) processID;
			strcpy (userReferenceNo," N/A  ");
		}

		strcpy (ithr_rec.co_no , (MC_TRAN) ? "  " : comm_rec.co_no);
		ithr_rec.del_no = comr_rec.nx_del_no;

		strcpy (ithr_rec.type,"T");
		if (TR_R_ISS)
			strcpy (ithr_rec.type,"R");

		if (TR_I_TWO)
			strcpy (ithr_rec.type,"U");

		if (TR_I_ONE || TR_I_RMT)
			strcpy (ithr_rec.type,"M");

		sprintf (ithr_rec.op_id, "%-14.14s", currentUser);
		strcpy (ithr_rec.time_create, TimeHHMM ());
		ithr_rec.date_create = TodaysDate ();
		ithr_rec.iss_date = local_rec.transDate;
		ithr_rec.iss_sdate = TodaysDate ();
		strcpy (ithr_rec.stat_flag, "0");
		strcpy (ithr_rec.printed, "N");
		strcpy (ithr_rec.del_zone, trzm_rec.del_zone);
		strcpy (ithr_rec.carr_code, trcm_rec.carr_code);

		if (ithr_rec.full_supply [0] != 'Y')
			strcpy (ithr_rec.full_supply, "N");

		if (local_rec.hr_full_supply [0] == 'Y')
			strcpy (ithr_rec.full_supply, "Y");

		cc = abc_add (ithr, &ithr_rec);
		if (cc)
			file_err (cc, "ithr", "DBADD");

	}
	else
	{
		if (!TR_I_TWO)
			sprintf (userReferenceNo,"%08ld", (TR_I_TWO)
						? 0L : local_rec.tran_no);
	}

	cc = find_rec (ithr, &ithr_rec,COMPARISON,"u");
	if (cc)
		file_err (cc, "ithr", "DBFIND");

	strcpy (ithr_rec.del_zone, trzm_rec.del_zone);
	strcpy (ithr_rec.carr_code, trcm_rec.carr_code);

	if (TR_I_TWO)
	{
		ithr_rec.del_no = 0L;

		if (ithr_rec.full_supply [0] != 'Y')
			strcpy (ithr_rec.full_supply, "N");

		cc = abc_update (ithr, &ithr_rec);
		if (cc)
			file_err (cc, "ithr", "DBUPDATE");
	}

	scn_set (ITEM_SCN);

	if (TR_ISSUE || TR_R_CON || TR_I_CON || TR_R_ISS)
	{
		bord_all = TRUE;
		bord_any = FALSE;
		/*---------------------------------------
		| Check if all/any lines are backorded. |
		---------------------------------------*/
		for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
		{
			getval (line_cnt);
			local_rec.qty_rec = (float) local_rec.qty_rec_doub;
			local_rec.qty_ord = (float) local_rec.qty_ord_doub;
			local_rec.qty_bord = (float) local_rec.qty_bord_doub;
			local_rec.qty_ship = (float) local_rec.qty_ship_doub;
			if (local_rec.qty_bord != 0.00 &&
			     ithr_rec.full_supply [0] == 'Y')
			{
				local_rec.qty_bord += local_rec.qty_ord;
				local_rec.qty_ord = 0.00;

				local_rec.qty_rec_doub = (double) local_rec.qty_rec;
				local_rec.qty_ord_doub = (double) local_rec.qty_ord;
				local_rec.qty_bord_doub = (double) local_rec.qty_bord;
				local_rec.qty_ship_doub = (double) local_rec.qty_ship;
				putval (line_cnt);
				bord_any = TRUE;
			}
			if (local_rec.qty_ord != 0.00 || local_rec.qty_bord == 0.00)
				bord_all = FALSE;

			if (local_rec.qty_ord != 0.00)
				zero_all = FALSE;
		}
		/*---------------------------------------
		| If ANY were B/Ordered AND FULL-SUPPLY	|
		| then make ALL B/Ordered.		|
		---------------------------------------*/
		if (bord_any && ithr_rec.full_supply [0] == 'Y')
		{
		    bord_all = TRUE;
		    for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
		    {
				getval (line_cnt);
				local_rec.qty_rec = (float) local_rec.qty_rec_doub;
				local_rec.qty_ord = (float) local_rec.qty_ord_doub;
				local_rec.qty_bord = (float) local_rec.qty_bord_doub;
				local_rec.qty_ship = (float) local_rec.qty_ship_doub;

				local_rec.qty_bord += local_rec.qty_ord;
				local_rec.qty_ord = 0.00;

				local_rec.qty_rec_doub = (double) local_rec.qty_rec;
				local_rec.qty_ord_doub = (double) local_rec.qty_ord;
				local_rec.qty_bord_doub = (double) local_rec.qty_bord;
				local_rec.qty_ship_doub = (double) local_rec.qty_ship;
				putval (line_cnt);
		    }
		}
	}
	/*-------------------------------------------------------
	| Update inventory cost centre stock record (file incc).|
	-------------------------------------------------------*/
	for (line_cnt = 0; line_cnt < lcount [ITEM_SCN]; line_cnt++)
	{
		/*-------------------
		| Get Labular line. |
		-------------------*/
		getval (line_cnt);
		local_rec.qty_rec	= (float) local_rec.qty_rec_doub;
		local_rec.qty_ord	= (float) local_rec.qty_ord_doub;
		local_rec.qty_bord	= (float) local_rec.qty_bord_doub;
		local_rec.qty_ship	= (float) local_rec.qty_ship_doub;

		if (!TR_RECEIPT)
		{
			/*-------------------------------------------------
			| Find inmr record from item number in structure. |
			-------------------------------------------------*/
			strcpy (inmr_rec.co_no,issCo);
			cc = find_rec (inmr,&inmr_rec,COMPARISON,
					 (TR_I_RMT || TR_R_ISS || TR_I_ONE || TR_I_TWO || TR_R_CON) ? "r" : "u");
			if (cc)
				file_err (cc, "inmr", "DBFIND");
		}
		else
		{
			/*-------------------------------------------------
			| Find inmr record from item number in structure. |
P
			-------------------------------------------------*/
			strcpy (inmr_rec.co_no,recCo);
			cc = find_rec (inmr,&inmr_rec,COMPARISON,
					 (TR_I_RMT || TR_R_ISS || TR_I_ONE || TR_I_TWO || TR_R_CON) ? "r" : "u");
			if (cc)
				file_err (cc, "inmr", "DBFIND");
		}

		workQuantity = (TR_RECEIPT) ? ToStdUom (local_rec.qty_rec) : 
							    ToStdUom (local_rec.qty_ord);

		value	= local_rec.workCost;
		duty	= local_rec.workDuty;

		workValue = (float) workQuantity;
		workValue *= value;
		workValue = twodec (out_cost (workValue, inmr_rec.outer_size));

		workDuty = (float) workQuantity;
		workDuty *= duty;
		workDuty = twodec (out_cost (workDuty, inmr_rec.outer_size));

		if (!TR_RECEIPT)
		{
			ProcessIssues (workQuantity , value);
			if (TR_I_DIR || TR_I_CON)
			{
				/*----------------------------------
				| Only add freight for first line. |
				----------------------------------*/
				if (envVar.automaticFreight && line_cnt == 0)
					AddCarrierDetails ();

				if (freight_posted)
					AddItgl (TRUE, workValue, 0.00, 0.00);
				else
				{
					AddItgl (TRUE, workValue, 0.00, ithr_rec.frt_cost);
					freight_posted = TRUE;
				}
			}
		}
		else
		{
			ProcessReceipts (workQuantity , value + duty);
			AddItgl (FALSE, workValue, workDuty, 0.00);
		}

		if (TR_I_DIR || TR_I_CON)
		{
			batchTotal += workValue + workDuty;
			quantityTotal += workQuantity;
		}

		add_hash 
		 (
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0,
			SR.hhbrHash, 
			issHhccHash, 
			0L, 
			 (double) 0.00
		);

		add_hash 
		 (
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0,
			SR.recHhbrHash, 
			recHhccHash, 
			0L, 
			 (double) 0.00
		);

		if (notOrigWh)
		{
			add_hash 
			 (
				comm_rec.co_no, 
				comm_rec.est_no, 
				"RC", 
				0,
				SR.recHhbrHash, 
				origHhccHash, 
				0L, 
				 (double) 0.00
			);
		}
	}

	strcpy (local_rec.previousRef,userReferenceNo);

	all_delete = FALSE;

	if (TR_RECEIPT)
	{
		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = 0;
		cc = find_rec (itln, &itln_rec, GTEQ, "r");
		while (!cc && itln_rec.hhit_hash == ithr_rec.hhit_hash)
		{
			all_delete = TRUE;
			if (itln_rec.status [0] != 'D')
			{
				all_delete = FALSE;
				break;
			}
			cc = find_rec (itln, &itln_rec, NEXT, "r");
		}
	}
	if (TR_R_CON || TR_ISSUE || TR_I_CON)
	{
		sprintf (ithr_rec.op_id, "%-14.14s", currentUser);
		ithr_rec.date_create = TodaysDate ();
		strcpy (ithr_rec.time_create, TimeHHMM ());

		strcpy (ithr_rec.carr_area, recArea);
		ithr_rec.no_cartons = local_rec.no_cartons;
	}
	if (TR_RECEIPT || TR_R_CON || TR_I_CON || TR_ISSUE)
	{
		if (TR_RECEIPT)
			ithr_rec.rec_date = comm_rec.inv_date;

		/*----------------------------
		| Set transfer type to base. |
		----------------------------*/
		strcpy (ithr_rec.type, "T");

		if (TR_R_ISS)
			strcpy (ithr_rec.type,"R");

		if (TR_I_TWO)
			strcpy (ithr_rec.type,"U");

		if (TR_I_RMT || TR_I_ONE || TR_R_CON)
			strcpy (ithr_rec.type,"M");

		if (bord_all)
			strcpy (ithr_rec.type, "B");

		if (all_delete)
			strcpy (ithr_rec.type, "D");

		if (ithr_rec.full_supply [0] != 'Y')
			strcpy (ithr_rec.full_supply, "N");

		cc = abc_update (ithr, &ithr_rec);
		if (cc)
			file_err (cc, "ithr", "DBUPDATE");
	}

	abc_unlock (ithr);

	if (zero_all && printerNumber)
	{
		print_at (4,0, ML (mlSkMess417));
		print_at (5,0, ML (mlStdMess042));
		PauseForKey (6, 0, "", 0);
	}

	if (bord_all && printerNumber)
	{
		print_at (5,0, ML (mlSkMess418));
		print_at (6,0, ML (mlStdMess042));
		PauseForKey (7, 0, "", 0);
	}

	if (printerNumber)
	{
		if (!pipe_open)
		{
			if (envVar.IKHK)
				sprintf (pipeName,"sk_trnprt %d %s %s", printerNumber, formatFile, transferType);
			else
				sprintf (pipeName,"sk_trnprt %d %s", printerNumber, formatFile);
			pout = popen (pipeName, "w");
			if (pout == (FILE *) NULL)
				file_err (errno, "sk_trnprt", "POPEN");
			pipe_open = TRUE;
		}
		/*-------------------------------------------------- 
		|	Added for ASL to make sure that this will only |
		|	print if there are changes during confirmation |
		---------------------------------------------------*/
		if (envVar.transferConfirm)
		{
			if (transferChanged && !bord_all && !zero_all)
			{
				fprintf (pout, "%ld\n", ithr_rec.hhit_hash);
				fflush (pout);
			}
		}
		else
		{			
			if (!bord_all && !zero_all)
			{
				fprintf (pout, "%ld\n", ithr_rec.hhit_hash);
				fflush (pout);
			}
		}
	}
	recalc_sobg ();

	if (!TR_RECEIPT)
	{
		if (ithr_rec.del_req [0] == 'Y' && newTransfer)
		{
			sprintf (err_str,"tr_trsh_mnt T %010ld",ithr_rec.hhit_hash);
			sys_exec (err_str);
		}
	}
	return;
}

/*=======================
| Process Stock Issues. |
=======================*/
void
ProcessIssues (
 float	processQuantity,
 double	value)
{
	char	workRef [16];
	int		NoLots	=	TRUE;
	int		i;

	/*----------------------------
	| Update issuing Cost Centre |
	----------------------------*/
	incc_rec.hhcc_hash = issHhccHash;
	incc_rec.hhbr_hash = SR.hhbrHash;

	cc = find_rec (incc,&incc_rec,COMPARISON,
					 (TR_I_RMT || TR_R_ISS || TR_I_ONE || TR_I_TWO || TR_R_CON) ? "r" : "u");
	if (cc)
		file_err (cc, "incc", "DBFIND");

	issClosing = incc_rec.closing_stock;
	issHhwhHash = incc_rec.hhwh_hash;

	if (!TR_R_ISS && !TR_I_TWO && !TR_I_ONE && !TR_R_CON && !TR_I_RMT)
	{
		incc_rec.issues 	+= processQuantity;
		incc_rec.ytd_issues += processQuantity;
		incc_rec.closing_stock = incc_rec.opening_stock +
				    	    	    incc_rec.pur +
				    	    	    incc_rec.receipts +
				    	    	    incc_rec.adj -
				    	    	    incc_rec.issues -
				    	    	    incc_rec.sales;
	    
		cc = abc_update (incc,&incc_rec);
		if (cc)
			file_err (cc, "incc", "DBUPDATE");

		if (SR.Demand [0] == 'Y')
		{
			ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
			ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
			strcpy (ffdm_rec.type, "4");
			ffdm_rec.date		=	StringToDate (local_rec.systemDate);
			cc = find_rec ("ffdm", &ffdm_rec, COMPARISON, "r");
			if (cc)
			{
				ffdm_rec.qty	=	 (float) processQuantity;
				cc = abc_add (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, "ffdm", "DBADD");
			}
			else
			{
				ffdm_rec.qty	+=	 (float) processQuantity;
				cc = abc_update (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, "ffdm", "DBUPDATE");
			}
		}
		
		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, "inwu", "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, "inwu", "DBFIND");
		}
		inwu_rec.issues	+= processQuantity;
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBUPDATE");
	}

	if (TR_ISSUE || TR_R_ISS)
	{
		itln_rec.hhit_hash 		= ithr_rec.hhit_hash;
		itln_rec.line_no 		= line_cnt;
		itln_rec.hhbr_hash 		= SR.hhbrHash;
		itln_rec.r_hhbr_hash 	= (MC_TRAN) ? SR.recHhbrHash : 0L;
		itln_rec.i_hhcc_hash 	= issHhccHash;
		itln_rec.r_hhcc_hash 	= recHhccHash;
		itln_rec.hhum_hash 		= SR.hhumHash;
		strcpy (itln_rec.serial_no,local_rec.serial_no);
		itln_rec.qty_order  	= (local_rec.qty_ord < 0.00)
						? 0.00 : ToStdUom (local_rec.qty_ord);
		itln_rec.qty_border 	= (local_rec.qty_bord < 0.00)
						? 0.00 : ToStdUom (local_rec.qty_bord);
		itln_rec.qty_rec    	= 0.00;
		itln_rec.cost 		= local_rec.workCost;
		itln_rec.duty 		= local_rec.workDuty;

		if (TR_I_DIR)
		{
			strcpy (itln_rec.status,
				 (itln_rec.qty_order == 0.00) ? "B" : "T");
		}
		if (TR_I_TWO)
		{
			strcpy (itln_rec.status,
				 (itln_rec.qty_order == 0.00) ? "B" : "U");
		}
		if (TR_I_ONE || TR_I_RMT)
		{
			strcpy (itln_rec.status,
				 (itln_rec.qty_order == 0.00) ? "B" : "M");
		}
		if (TR_R_ISS)
			strcpy (itln_rec.status, "R");

		if (itln_rec.status [0] == 'B')
			itln_rec.cost = 0.00;

		if (itln_rec.qty_order == 0.00 &&
		     itln_rec.qty_border == 0.00)
			strcpy (itln_rec.status, "D");

		if (ithr_rec.full_supply [0] == 'Y')
			strcpy (itln_rec.full_supply, "Y");
		else
			sprintf (itln_rec.full_supply, "%-1.1s", local_rec.ln_full_supp);
		strcpy (itln_rec.stock, local_rec.stock);
		strcpy (itln_rec.stat_flag, "0");
		itln_rec.due_date = StringToDate (local_rec.systemDate);
		strcpy (itln_rec.tran_ref, ithr_rec.tran_ref);

		if (NON_STOCK && strcmp (lastStatus, " "))
			strcpy (itln_rec.status, lastStatus);
		else
			strcpy (lastStatus, itln_rec.status);

		if (NON_STOCK)
			strcpy (itln_rec.item_desc, SR._description);
		else
			strcpy (itln_rec.item_desc, inmr_rec.description);

		cc = abc_add (itln, &itln_rec);
		if (cc)
			file_err (cc, "itln", "DBADD");

		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = line_cnt;
		cc = find_rec (itln, &itln_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "itln", "DBFIND");
	}
	if (TR_R_CON || TR_I_CON)
	{
		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = local_rec.line_no;
		cc = find_rec (itln, &itln_rec, COMPARISON,"u");
		if (cc)
			file_err (cc, "itln", "DBFIND");

		itln_rec.i_hhcc_hash = issHhccHash;
		itln_rec.r_hhcc_hash = recHhccHash;
		itln_rec.hhum_hash 	= SR.hhumHash;
		strcpy (itln_rec.serial_no,local_rec.serial_no);
		itln_rec.qty_order  = (local_rec.qty_ord < 0.00)
						? 0.00 : ToStdUom (local_rec.qty_ord);
		itln_rec.qty_border = (local_rec.qty_bord < 0.00)
						? 0.00 : ToStdUom (local_rec.qty_bord);
		itln_rec.qty_rec    = 0.00;
		itln_rec.cost = local_rec.workCost;
		itln_rec.duty = local_rec.workDuty;
		if (TR_R_CON)
			strcpy (itln_rec.status,
				 (itln_rec.qty_order == 0.00) ? "B" : "M");
		else
			strcpy (itln_rec.status,
				 (itln_rec.qty_order == 0.00) ? "B" : "T");
		if (itln_rec.qty_order == 0.00 && itln_rec.qty_border == 0.00)
			strcpy (itln_rec.status, "D");

		if (itln_rec.status [0] == 'B' || itln_rec.status [0] == 'D')
			itln_rec.cost = 0.00;

		if (NON_STOCK && strcmp (lastStatus, " "))
			strcpy (itln_rec.status, lastStatus);
		else
			strcpy (lastStatus, itln_rec.status);

		cc = abc_update (itln,&itln_rec);
		if (cc)
			file_err (cc, "itln", "DBUPDATE");

		abc_unlock (itln);
	}

	if (!TR_R_ISS)
	{
		if (SERIAL && local_rec.qty_ord)
		{
			if (TR_I_RMT || TR_I_ONE || TR_I_TWO || TR_R_CON)
				UpdateInsf (SR.issHhwhHash, 0L, local_rec.serial_no, "F","C");
			else
				TransferInsf (line_cnt, SR.origSerialNo);
		}

		if (!TR_I_TWO && !TR_I_ONE && !TR_R_CON && ! TR_I_RMT)
		{
			inmr_rec.on_hand -= processQuantity;
			cc = abc_update (inmr,&inmr_rec);
			if (cc)
				file_err (cc, "inmr", "DBUPDATE");

			abc_unlock (inmr);
		}
	}
	if (!TR_R_ISS && !TR_I_TWO && !TR_I_ONE && !TR_R_CON && !TR_I_RMT)
	{
		if (MULT_LOC || SK_BATCH_CONT)
		{
			UpdateLotLocation
			(
				line_cnt,
				TRUE
			);
			TransLocation 
			 (	
				line_cnt, 
				itln_rec.itff_hash 
			);
		}
	}
	if (!TR_R_ISS && !TR_I_TWO && !TR_I_ONE && !TR_R_CON && !TR_I_RMT)
	{
		if (FIFO || LIFO)
		{
			TransIncfToItff 
			(
				issHhwhHash, 
				issClosing, 
				processQuantity,
				SR.fileCost,
				local_rec.workCost,
				local_rec.workDuty,
				(FIFO) ? TRUE : FALSE, 
				itln_rec.itff_hash
			);
		}
	}
	if (!TR_R_ISS && !TR_I_TWO && !TR_I_ONE && !TR_R_CON && !TR_I_RMT)
	{
		NoLots	=	TRUE;
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			NoLots	=	FALSE;
			sprintf (workRef, "TR%2.2s/%2.2s/%2.2s%5.5s",recCo,recBr,recWh," ");

			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			if (GetBaseQty (line_cnt, i) != 0.00)
			{
				MoveAdd
			 	 (
					comm_rec.co_no,
					issBr,
					issWh,
					SR.hhbrHash,
					issHhccHash,
					SR.hhumHash,
					local_rec.transDate,
					3,
					GetLotNo (line_cnt, i),
					inmr_rec.inmr_class,
					inmr_rec.category,
					userReferenceNo,  
					workRef,
					GetBaseQty (line_cnt, i),
					0.0,
					CENTS (value)
				);
			}
		}
		if (NoLots)
		{
			sprintf (workRef, "TR%2.2s/%2.2s/%2.2s%5.5s",recCo,recBr,recWh," ");
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			if (processQuantity != 0.00)
			{
				MoveAdd
			 	 (
					comm_rec.co_no,
					issBr,
					issWh,
					SR.hhbrHash,
					issHhccHash,
					SR.hhumHash,
					local_rec.transDate,
					3,
					" ",
					inmr_rec.inmr_class,
					inmr_rec.category,
					userReferenceNo,  
					workRef,
					processQuantity,
					0.0,
					CENTS (value)
				);
			}
		}
	}
	return;
}

/*=========================
| Process Stock Receipts. |
=========================*/
void
ProcessReceipts (
 float	processQuantity,
 double	value)
{
	double	r_avge_cost	= 0.00;
	double	old_qty		= 0;
	double	xx_qty		= 0;
	double	cal1_total	= 0;
	char	workRef [16];
	int		i;
	int		NoLots	=	TRUE;

	itln_rec.hhit_hash = ithr_rec.hhit_hash;
	itln_rec.line_no   = local_rec.line_no;

	cc = find_rec (itln,&itln_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "itln", "DBFIND");

	local_rec.qty_rec = (float) local_rec.qty_rec_doub;
	local_rec.qty_ship = (float) local_rec.qty_ship_doub;
	/*--------------------------------------------
	| Process an issue for extra goods supplied. |
	--------------------------------------------*/
	if (local_rec.qty_rec > local_rec.qty_ship)
		ProcessIssues (ToStdUom (local_rec.qty_rec-local_rec.qty_ship), value);

	itln_rec.hhbr_hash = SR.hhbrHash;
	itln_rec.r_hhbr_hash = (MC_TRAN) ? SR.recHhbrHash : 0L;
	itln_rec.i_hhcc_hash = issHhccHash;
	itln_rec.r_hhcc_hash = recHhccHash;
	itln_rec.hhum_hash 	= SR.hhumHash;
	strcpy (itln_rec.serial_no,local_rec.serial_no);
	itln_rec.qty_order  -= ToStdUom (local_rec.qty_rec);
	if (itln_rec.qty_order < 0.00)
	{
		if (itln_rec.qty_border > 0.00)
			itln_rec.qty_border -= (ToStdUom (local_rec.qty_rec) 
									-  ToStdUom (local_rec.qty_ship));
		if (itln_rec.qty_border < 0.00)
			itln_rec.qty_border = 0.00;

		itln_rec.qty_order = 0.00;
	}

	itln_rec.qty_rec    += ToStdUom (local_rec.qty_rec);
	itln_rec.cost = local_rec.workCost;
	itln_rec.duty = local_rec.workDuty;

	if (itln_rec.qty_order == 0.00 && itln_rec.qty_border == 0.00)
		strcpy (itln_rec.status, "D");

	if (itln_rec.qty_order == 0.00 && itln_rec.qty_border != 0.00)
		strcpy (itln_rec.status, "B");

	if (NON_STOCK && strcmp (lastStatus, " "))
		strcpy (itln_rec.status, lastStatus);
	else
		strcpy (lastStatus, itln_rec.status);

	cc = abc_update (itln,&itln_rec);
	if (cc)
		file_err (cc, "itln", "DBUPDATE");

	xx_qty = old_qty = GetBrClosing (recCo, recBr, SR.recHhbrHash);

	/*------------------------------
	| Update Receiving Cost Centre |
	------------------------------*/
	incc_rec.hhcc_hash = recHhccHash;
	incc_rec.hhbr_hash = SR.recHhbrHash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"u");
	if (cc)
	{
		cc = AddIncc (SR.recHhbrHash, recHhccHash);
		if (cc)
			file_err (cc, "incc", "DBADD");
	}
	recHhwhHash = incc_rec.hhwh_hash;
	recClosing   = incc_rec.opening_stock +
				    incc_rec.pur +
				    incc_rec.receipts +
				    incc_rec.adj -
				    incc_rec.issues -
				    incc_rec.sales;

	incc_rec.receipts     += processQuantity;
	incc_rec.ytd_receipts    += processQuantity;
	incc_rec.closing_stock = incc_rec.opening_stock +
				    		    incc_rec.pur +
				    		    incc_rec.receipts +
				    		    incc_rec.adj -
				    		    incc_rec.issues -
				    		    incc_rec.sales;

	cc = abc_update (incc,&incc_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");

	/*--------------------------------------
	| Find Warehouse unit of measure file. |
	--------------------------------------*/
	inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash	=	SR.hhumHash;
	cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
	if (cc)
	{
		memset (&inwu_rec, 0, sizeof (inwu_rec));
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = abc_add (inwu, &inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBADD");

		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
			file_err (cc, "inwu", "DBFIND");
	}
	inwu_rec.receipts	+= processQuantity;
	inwu_rec.closing_stock = inwu_rec.opening_stock +
							 inwu_rec.pur +
							 inwu_rec.receipts +
							 inwu_rec.adj -
							 inwu_rec.issues -
							 inwu_rec.sales;

	cc = abc_update (inwu,&inwu_rec);
	if (cc)
		file_err (cc, "inwu", "DBUPDATE");

	/*----------------------------
	| Update issuing Cost Centre |
	----------------------------*/
	incc_rec.hhcc_hash = issHhccHash;
	incc_rec.hhbr_hash = SR.hhbrHash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "incc", "DBFIND");

	issClosing = incc_rec.closing_stock;
	issHhwhHash = incc_rec.hhwh_hash;

	/*-----------------------------------------------
	| Reduce the FIFO records for the RECEIPTING	|
	| W/House such that they match the closing stk.	|
	-----------------------------------------------*/
	if (FIFO || LIFO)
	{
	    if (issHhwhHash != recHhwhHash)
			ReduceIncf (recHhwhHash, recClosing, (FIFO) ? TRUE : FALSE);
	}

	/*-----------------------------------------
	| Get inei record to update average cost. |
	-----------------------------------------*/
	cc = FindInei (SR.recHhbrHash, recBr, "r");
	if (cc)
	{
		AddInei (SR.recHhbrHash, recBr);
		cc = FindInei (SR.recHhbrHash, recBr, "r");
	}
	if (cc)
	{
		print_mess (ML (mlSkMess419));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	else
		r_avge_cost = ineiRec.avge_cost;

	if (FIFO || LIFO)
	{
		TransItffToIncf
		(
			recHhwhHash, 
			itln_rec.itff_hash, 
			processQuantity
		);
	}

	if (MULT_LOC || SK_BATCH_CONT)
	{
		UpdateItlo 
		 (		
			line_cnt,
			SR.recHhwhHash,
			itln_rec.itff_hash
		);
	}
	if (SERIAL)
		IntUpdateInsf ();

	if (old_qty < 0.00)
		xx_qty = 0.00;

	if (processQuantity != 0.00)
	{
		if (old_qty + processQuantity == 0.00)
			cal1_total = local_rec.workCost + local_rec.workDuty;
		else
		{
			if (old_qty + processQuantity < 0.00)
			{
				if (envVar.zeroCost)
					cal1_total = 0;
				else
					cal1_total = ineiRec.avge_cost;
			}
			else
				cal1_total = ((xx_qty * r_avge_cost) + (processQuantity * (local_rec.workCost + local_rec.workDuty))) / (xx_qty + processQuantity);
		}

		ineiRec.avge_cost = cal1_total ;
		ineiRec.prev_cost = ineiRec.last_cost;
		ineiRec.last_cost = local_rec.workCost + local_rec.workDuty;
		ineiRec.lpur_qty = processQuantity;
		ineiRec.date_lcost = comm_rec.inv_date;
		cc = abc_update (inei,&ineiRec);
		if (cc)
			file_err (cc, "inei", "DBUPDATE");
	}
	inmr_rec.on_hand += processQuantity;
	cc = abc_update (inmr,&inmr_rec);
	if (cc)
		file_err (cc, "inmr", "DBUPDATE");

	NoLots	=	TRUE;
	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (line_cnt, i))
			break;

		NoLots	=	FALSE;
		sprintf (workRef, "TR%2.2s/%2.2s/%2.2s%5.5s",issCo,issBr,issWh," ");
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		if (GetBaseQty (line_cnt, i) != 0.00)
		{
			MoveAdd
		 	 (
				comm_rec.co_no,
				recBr,
				recWh,
				SR.recHhbrHash,
				recHhccHash,
				SR.hhumHash, 
				local_rec.transDate,
				2,
				GetLotNo (line_cnt, i),
				inmr_rec.inmr_class,
				inmr_rec.category,
				userReferenceNo,  
				workRef,
				GetBaseQty (line_cnt, i),
				0.0,
				CENTS (value)
			);
		}
	}
	if (NoLots)
	{
		sprintf (workRef, "TR%2.2s/%2.2s/%2.2s%5.5s",
										comm_rec.co_no,issBr,issWh, " ");
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		if (processQuantity != 0.00)
		{
			MoveAdd
		 	 (
				comm_rec.co_no,
				recBr,
				recWh,
				SR.recHhbrHash,
				recHhccHash,
				SR.hhumHash, 
				local_rec.transDate,
				2,
				userReferenceNo,
				inmr_rec.inmr_class,
				inmr_rec.category,
				userReferenceNo,  
				workRef,
				processQuantity,
				0.0,
				CENTS (value)
			);
		}
	}
}
/*==========================================
| Transfer serial item to transfer status. |
==========================================*/
void
TransferInsf (
 int	line_cnt,
 char	*serialNo)
{
	/*-------------------------
	| Free old serial Number. |
	-------------------------*/
	if (strcmp (serialNo, local_rec.serial_no))
	{
		if (strcmp (serialNo, ser_space))
			FreeInsf (line_cnt, serialNo);
	}
	/*-----------------------------------------------------
	| Check if serial number is committed to Sales Order. |
	-----------------------------------------------------*/
	if (CheckInsf (SR.hhbrHash, local_rec.serial_no))
		return;

	cc = UpdateInsf (SR.issHhwhHash, 0L, local_rec.serial_no, "F", "T");
	if (cc)
		cc = UpdateInsf (SR.issHhwhHash, 0L, local_rec.serial_no, "C", "T");

	if (cc && cc < 1000)
		file_err (cc, insf, "DBFIND");
}

/*===========================================
| Update insf record to free for new Br/Wh. |
===========================================*/
void
IntUpdateInsf (
 void)
{
	/*-----------------------------------
	| Check for transfered serial item. |
	-----------------------------------*/
	cc = FindInsf (SR.issHhwhHash, 0L, local_rec.serial_no,"T","u");
	if (cc)
	{
		/*----------------------------------------------
		| Check for transfer of committed serial item. |
		----------------------------------------------*/
		cc = FindInsf (SR.issHhwhHash, 0L, local_rec.serial_no,"C","u");
	}
	if (cc)
	{
		/*-------------------------------------------------
		| Check for transfer of ex-committed serial item. |
		-------------------------------------------------*/
		cc = FindInsf (SR.issHhwhHash, 0L, local_rec.serial_no,"F","u");
	}

	if (!cc)
	{
		insfRec.duty        += local_rec.workDuty;
		insfRec.istore_cost += local_rec.workDuty;
		insfRec.est_cost    += local_rec.workDuty;
		insfRec.act_cost    += local_rec.workDuty;

		if (insfRec.status [0] == 'T')
			strcpy (insfRec.status, "F");

		insfRec.hhwh_hash = recHhwhHash;
		cc = abc_update (insf,&insfRec);
		if (cc)
			file_err (cc, insf, "DBUPDATE");
	}
	else
		abc_unlock (insf);
}

/*=======================
|	Free insf record	|
=======================*/
void
FreeInsf (
	int		line_cnt,
	char	*serialNo)
{
	/*-----------------------------------------------------
	| Check if serial number is committed to Sales Order. |
	-----------------------------------------------------*/
	if (CheckInsf (SR.hhbrHash, serialNo))
		return;

	cc = UpdateInsf (SR.issHhwhHash, 0L, serialNo, "C", "F");
	if (cc && cc < 1000)
		file_err (cc, insf, "DBUPDATE");
}

void
AddItgl (
 int	issue,
 double	gl_value,
 double	gl_duty,
 double	gl_freight)
{
	int		monthNum;

	strcpy (itgl_rec.co_no, (issue) ? issCo : recCo);
	strcpy (itgl_rec.br_no, (issue) ? issBr : recBr);
	strcpy (itgl_rec.i_co_no, issCo);
	strcpy (itgl_rec.i_br_no, issBr);
	strcpy (itgl_rec.r_co_no, recCo);
	strcpy (itgl_rec.r_br_no, recBr);
	sprintf (itgl_rec.sort, "%06ld", local_rec.tran_no);
	strcpy (itgl_rec.type, (issue) ? "I" : "R");
	strcpy (itgl_rec.ic_trans, (MC_TRAN) ? "Y" : "N");
	itgl_rec.hhbr_hash = inmr_rec.hhbr_hash;
	itgl_rec.hhcc_hash = (issue) ? issHhccHash : recHhccHash;
	itgl_rec.tran_date = local_rec.transDate;
	itgl_rec.post_date = StringToDate (local_rec.systemDate);

	DateToDMY (local_rec.transDate, NULL, &monthNum, NULL);
	sprintf (itgl_rec.period_no, "%02d", monthNum);
	strcpy (itgl_rec.tran_type, "10");
	sprintf (itgl_rec.sys_ref,"%5.1d",1);
	if (issue)
	{
		sprintf (itgl_rec.narr,  ML (mlSkMess420), issBr,issWh);
		itgl_rec.amount = CENTS (gl_value);
	}
	else
	{
		sprintf (itgl_rec.narr, ML (mlSkMess421), recBr,recWh);
		itgl_rec.amount = CENTS (gl_value + gl_duty);
	}
	strcpy (itgl_rec.user_ref,userReferenceNo);
	strcpy (itgl_rec.jnl_type,"1");
	strcpy (itgl_rec.stat_flag, "0");

	cc = abc_add (itgl,&itgl_rec);
	if (cc)
		file_err (cc, "itgl", "DBADD");

	itgl_rec.amount = CENTS (gl_value);

	strcpy (itgl_rec.jnl_type,"2");
	cc = abc_add (itgl,&itgl_rec);
	if (cc)
		file_err (cc, "itgl", "DBADD");

	if (gl_duty != 0.00)
	{
		sprintf (itgl_rec.narr, ML (mlSkMess422), recBr,recWh);

		itgl_rec.amount = CENTS (gl_duty);
		strcpy (itgl_rec.jnl_type,"4");
		cc = abc_add (itgl,&itgl_rec);
		if (cc)
			file_err (cc, "itgl", "DBADD");
	}
	if (gl_freight != 0.00)
	{
		sprintf (itgl_rec.narr, ML (mlSkMess423) ,recBr,recWh);
		itgl_rec.amount = CENTS (gl_freight);
		strcpy (itgl_rec.jnl_type, "5");
		cc = abc_add (itgl,&itgl_rec);
		if (cc)
			file_err (cc, "itgl", "DBADD");
	}
}

int
AddIncc (
 long	hhbrHash,
 long	hhcc_hash)
{
	char	temp_sort [29];

	memset (&incc_rec, 0, sizeof (incc_rec));

	incc_rec.hhbr_hash = hhbrHash;
	incc_rec.hhcc_hash = hhcc_hash;
	sprintf (temp_sort,"%s%11.11s%-16.16s", inmr_rec.inmr_class,
										   inmr_rec.category,
										   inmr_rec.item_no);

	incc_rec.first_stocked 	= StringToDate (local_rec.systemDate);
	strcpy (incc_rec.sort, temp_sort);
	strcpy (incc_rec.stocking_unit, inmr_rec.sale_unit);
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.allow_repl, "E");
	strcpy (incc_rec.abc_code, inmr_rec.abc_code);
	strcpy (incc_rec.abc_update, inmr_rec.abc_update);
	strcpy (incc_rec.stat_flag,"0");

	cc = abc_add (incc,&incc_rec);
	if (cc)
		return (RETURN_ERR);

	incc_rec.hhbr_hash = hhbrHash;
	incc_rec.hhcc_hash = hhcc_hash;
	return (find_rec (incc,&incc_rec,COMPARISON,"w"));
}

void
AddInei (
	long	hhbrHash,
	char	*branchNo)
{
	memset (&ineiRec, 0, sizeof (ineiRec));
	ineiRec.hhbr_hash	=	hhbrHash;
	sprintf (ineiRec.est_no, "%-2.2s", branchNo);
	strcpy (ineiRec.stat_flag,"0");
	cc = abc_add (inei,&ineiRec);
	if (cc)
		file_err (cc, inei, "DBADD");
}

/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char *key_val)
{
	_work_open (6,0,40);

	save_rec ("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.est_no) &&
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
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "trzm", "DBFIND");

	return;
}

/*==========================
| Search for carrier code. |
==========================*/
void
SrchTrcm (
 char	*key_val)
{
	char	key_string [31];
	long 	currentZoneHash	=	trzm_rec.trzm_hash;

	_work_open (20, 11, 50);

	save_rec ("#Carrier","# Rate Kg. | Carrier Name.");
	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec,GTEQ,"r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) && 
		      	  !strncmp (trcm_rec.carr_code, key_val,strlen (key_val)))
	{
		trcl_rec.trcm_hash	=	trcm_rec.trcm_hash;
		trcl_rec.trzm_hash	=	0L;
		cc = find_rec (trcl, &trcl_rec,GTEQ,"r");
		while (!cc && trcl_rec.trcm_hash == trcm_rec.trcm_hash)
		{
	
			if (currentZoneHash	> 0L && trcl_rec.trzm_hash != currentZoneHash)
			{
				cc = find_rec (trcl, &trcl_rec,NEXT,"r");
				continue;
			}
			trzm_rec.trzm_hash	=	trcl_rec.trzm_hash;
			cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (err_str, " %8.2f | %-40.40s", 
							trcl_rec.cost_kg,
							trzm_rec.desc);

				sprintf (key_string, "%-4.4s-%-6.6s %010ld",
							trcm_rec.carr_code,
							trzm_rec.del_zone,
							trzm_rec.trzm_hash);

				cc = save_rec (key_string, err_str);
				if (cc)
					break;
			}
			cc = find_rec (trcl, &trcl_rec,NEXT,"r");
		}
		cc = find_rec (trcm, &trcm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "trcm", "DBFIND");

	trzm_rec.trzm_hash	=	atol (temp_str + 12);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (!cc)
		sprintf (local_rec.defaultDelZone, "%-6.6s", trzm_rec.del_zone);

}
void
SrchComr (
 char	*key_val)
{
	_work_open (2,0,40);
	sprintf (comr_rec.co_no,"%2.2s",key_val);
	cc = find_rec (comr,&comr_rec,GTEQ,"r");
	cc = save_rec ("#Br ","#Br Name");
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
 char	*key_val)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = find_rec (esmr,&esmr_rec,GTEQ,"r");
	cc = save_rec ("#Br ","#Br Name");
	while (!cc && !strcmp (esmr_rec.co_no,comm_rec.co_no) &&
		      !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no,esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",temp_str);
	cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}

void
SrchCcmr (
 char	*key_val,
 int	recWh)
{
	char	br_no [3];

	strcpy (br_no, (recWh) ? recBr : issBr);

	_work_open (2,0,40);
	cc = save_rec ("#Wh No.","#Wh Name");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",key_val);
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) &&
		      !strcmp (ccmr_rec.est_no,br_no) &&
		      !strncmp (ccmr_rec.cc_no,key_val,strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no,ccmr_rec.name);
		if (cc)
			break;
		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,br_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",temp_str);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
}

void
SrchIthr (
 char	*key_val)
{
	char	del_no [9];
	char	type [2];

	strcpy (type, "R");

	if (TR_RECEIPT)
		strcpy (type, "T");

	if (TR_I_CON)
		strcpy (type, "M");

	_work_open (6,0,40);
	save_rec ("#Tran No","#Comments ");
	strcpy (ithr_rec.co_no , (MC_TRAN) ? "  " : comm_rec.co_no);
	strcpy (ithr_rec.type, type);
	ithr_rec.del_no = atol (key_val);

	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");

	while (!cc && !strcmp (ithr_rec.co_no,
				 (MC_TRAN) ? "  " : comm_rec.co_no))
	{
		if (ithr_rec.type [0] == type [0] || TR_DISPLAY)
		{
			if (TR_RECEIPT)
			{
				itln_rec.hhit_hash = ithr_rec.hhit_hash;
				itln_rec.line_no = 0;
				cc = find_rec (itln,&itln_rec,EQUAL,"r");
				if (cc || recHhccHash != itln_rec.r_hhcc_hash)
				{
					cc = find_rec (ithr,&ithr_rec,NEXT,"r");
					continue;
				}
			}
			sprintf (del_no,"%06ld",ithr_rec.del_no);
			cc = save_rec (del_no,ithr_rec.tran_ref);
			if (cc)
				break;

		}
		cc = find_rec (ithr,&ithr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ithr_rec.co_no , (MC_TRAN) ? "  " : comm_rec.co_no);
	strcpy (ithr_rec.type, type);
	ithr_rec.del_no = atol (temp_str);
	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");
	if (cc)
		file_err (cc, "ithr", "DBFIND");
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
SrchInum (
 char    *key_val)
{
	_work_open (4,0,40);
	save_rec ("#UOM ","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
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

/*==============================
| Calculate Defaults for Levy. |
==============================*/
void
CalculateFreight (
 float	freightMarkup,
 double	carrierCostKg,
 double	zoneCostKg,
 double	zoneFixedAmount)
{
	int		i;
	float	totalKgs		= 0.0;
	double	freightValue	= 0.00;
	float	weight			= 0.00;
	double	calcMarkup		= 0.00;

	ithr_rec.no_kgs 	= 0.00;
	local_rec.efreight 	= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	for (i = 0;i < lcount [ITEM_SCN];i++)
	{
		weight = (store [i].weight > 0.00) ? store [i].weight 
										     : comr_rec.frt_mweight;
		totalKgs += (weight * store [i].qtyDes);
	}
	/*-------------------------------
	| Cost by Kg by Carrier / Zone. |
	-------------------------------*/
	if (envVar.soFreightCharge == 3)
		freightValue = (double) totalKgs * carrierCostKg;

	/*---------------------
	| Cost by Kg by Zone. |
	---------------------*/
	if (envVar.soFreightCharge == 2)
		freightValue = (double) totalKgs * zoneCostKg;

	calcMarkup = (double) freightMarkup;
	calcMarkup *= freightValue;
	calcMarkup = DOLLARS (calcMarkup);
	
	freightValue += calcMarkup;
	freightValue = twodec (freightValue);

	if (freightValue < comr_rec.frt_min_amt && freightValue > 0.00)
		local_rec.efreight = comr_rec.frt_min_amt;
	else
		local_rec.efreight = freightValue;
	
	if (FREIGHT_CHG)
	{
		if (envVar.soFreightCharge == 1)
		{
			ithr_rec.frt_cost 	= zoneFixedAmount;
			local_rec.efreight 	= zoneFixedAmount;
		}
		else
			ithr_rec.frt_cost = local_rec.efreight;
	}
	ithr_rec.no_kgs = totalKgs;

	return;
}
/*===========================================
| Load details from Issue Branch/Warehouse. |
===========================================*/
int
LoadItems (
 void)
{
	int		first_time = TRUE;
	int		i;
	float	std_cnv_fct;
	char	type [2];

	notOrigWh = FALSE;
	origHhccHash = 0L;

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (inum,"inum_hhum_hash");

	strcpy (type, "R");

	if (TR_RECEIPT)
		strcpy (type, "T");

	if (TR_I_CON)
		strcpy (type, "M");

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;
	vars [scn_start].row = 0;

	strcpy (ithr_rec.co_no , (MC_TRAN) ? "  " : comm_rec.co_no);
	ithr_rec.del_no = local_rec.tran_no;
	strcpy (ithr_rec.type, type);

	cc = find_rec (ithr,&ithr_rec,COMPARISON, (TR_DISPLAY) ? "r" : "u");
	if (cc)
	{
		vars [scn_start].row = lcount [ITEM_SCN];
		scn_set (HEADER_SCN);
		abc_selfield (inmr,"inmr_id_no");
		return (RETURN_ERR);
	}
	if (TR_DISPLAY)
		strcpy (type, ithr_rec.type);

	itln_rec.hhit_hash = ithr_rec.hhit_hash;
	itln_rec.line_no = 0;
	cc = find_rec (itln,&itln_rec,GTEQ,"r");
	while (!cc && itln_rec.hhit_hash == ithr_rec.hhit_hash)
	{
		if ((itln_rec.qty_order <= 0.00 && TR_RECEIPT) ||
		      ithr_rec.type [0] != type [0] ||
		      itln_rec.status [0] == 'D')
		{
			abc_unlock (itln);
			cc = find_rec (itln,&itln_rec,NEXT,"r");
			continue;
		}
		if (first_time)
		{
			SetIssueBranch 
			 (
				comm_rec.co_no,
				comm_rec.est_no,
				comm_rec.cc_no,
				itln_rec.i_hhcc_hash, 
				TRUE
			);

			if (TR_RECEIPT)
			{
				SetIssueBranch 
				 (
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					0L, 
					FALSE
				);

				if (recHhccHash != itln_rec.r_hhcc_hash)
				{
					i = prmptmsg (ML (mlSkMess424) ,"YyNn",1,2);
					if (i == 'n' || i == 'N')
						return (3);

					/*----------------------------------------
					| Original CO/BR/WH differnt to current. |
					----------------------------------------*/
					notOrigWh = TRUE;
					origHhccHash = itln_rec.r_hhcc_hash;

					move (0,2);
					cl_line ();
					fflush (stdout);
				}
			}
			else
			{
				SetIssueBranch 
				 (
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					itln_rec.r_hhcc_hash, 
					FALSE
				);

			     if (!strcmp (ccmr_rec.co_no,comm_rec.co_no) &&
			         !strcmp (ccmr_rec.est_no,comm_rec.est_no) &&
			         !strcmp (ccmr_rec.cc_no,comm_rec.cc_no))
			     {
					if (!TR_DISPLAY)
						return (9);
			     }
			}
		}
		first_time = FALSE;

		if (! (lcount [ITEM_SCN] % 20))
		{
			putchar ('R');
			fflush (stdout);
		}

		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	itln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "inmr", "DBFIND");
		
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		std_cnv_fct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		line_cnt	 = lcount [ITEM_SCN];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		store [lcount [ITEM_SCN]].cnvFct = inum_rec.cnv_fct/std_cnv_fct;

		strcpy (local_rec.UOM, inum_rec.uom);
		strcpy (store [lcount [ITEM_SCN]].UOM, inum_rec.uom);

		strcpy (store [lcount [ITEM_SCN]].issBackOrder,inmr_rec.bo_flag);
		store [lcount [ITEM_SCN]].hhbrHash = inmr_rec.hhbr_hash;
		store [lcount [ITEM_SCN]].weight = inmr_rec.weight;
		strcpy (store [lcount [ITEM_SCN]].storeClass, inmr_rec.inmr_class);
		strcpy (store [lcount [ITEM_SCN]].costingFlag,inmr_rec.costing_flag);
		strcpy (store [lcount [ITEM_SCN]].serialItem,inmr_rec.serial_item);

		if (MC_TRAN)
		{
			/*------------------
			| Get part number. |
			------------------*/
			inmr_rec.hhbr_hash	=	itln_rec.r_hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "inmr", "DBFIND");
		}
		else
			itln_rec.r_hhbr_hash = inmr_rec.hhbr_hash;

		strcpy (store [lcount [ITEM_SCN]].recBackOrder,inmr_rec.bo_flag);
		store [lcount [ITEM_SCN]].recHhbrHash = inmr_rec.hhbr_hash;
		store [lcount [ITEM_SCN]].decPt = inmr_rec.dec_pt;

		incc_rec.hhcc_hash 		= itln_rec.i_hhcc_hash;
		incc_rec.hhbr_hash 		= itln_rec.hhbr_hash;
		store [lcount [ITEM_SCN]].itffHash	= itln_rec.itff_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "incc", "DBFIND");

		store [lcount [ITEM_SCN]].issHhwhHash = incc_rec.hhwh_hash;
		store [lcount [ITEM_SCN]].issClosing   = incc_rec.closing_stock;
		store [lcount [ITEM_SCN]].issAvailable     = incc_rec.closing_stock -
					       			   incc_rec.backorder -
					       			   incc_rec.committed -
					       			   incc_rec.forward;
		if (envVar.qcApplied && envVar.qCAvailable)
			store [lcount [ITEM_SCN]].issAvailable = incc_rec.qc_qty;

		if (!TR_RECEIPT)
		{
			store [lcount [ITEM_SCN]].issAvailable += itln_rec.qty_border +
										itln_rec.qty_order;
		}

		strcpy (store [lcount [ITEM_SCN]].lot_ctrl, inmr_rec.lot_ctrl);
		strcpy (local_rec.lot_ctrl, inmr_rec.lot_ctrl);
		strcpy (local_rec.oldLotNo, "       ");
		strcpy (store [lcount [ITEM_SCN]].oldLotNo, "       ");
		strcpy (local_rec.lot_no, "       ");
		strcpy (store [lcount [ITEM_SCN]].lot_no, "       ");

		incc_rec.hhcc_hash = recHhccHash;
		incc_rec.hhbr_hash = itln_rec.r_hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = AddIncc (itln_rec.r_hhbr_hash, recHhccHash);
			if (cc)
				file_err (cc, "incc", "DBADD");
		}

		store [lcount [ITEM_SCN]].recHhwhHash = incc_rec.hhwh_hash;

		strcpy (store [lcount [ITEM_SCN]].serialItem, inmr_rec.serial_item);
		strcpy (store [lcount [ITEM_SCN]].costingFlag,inmr_rec.costing_flag);

		local_rec.qty_ord_doub  = (TR_RECEIPT) ? 0.00 :
				n_dec (ToLclUom (itln_rec.qty_order), 
				store [lcount [ITEM_SCN]].decPt);
		local_rec.qty_ord = (float) local_rec.qty_ord_doub;
		local_rec.qty_bord_doub = (TR_RECEIPT) ? 0.00 :
				n_dec (ToLclUom (itln_rec.qty_border), 
				store [lcount [ITEM_SCN]].decPt);
		local_rec.qty_bord = (float) local_rec.qty_bord_doub;
		local_rec.qty_rec_doub  = (TR_RECEIPT) ? 0.00 :
				n_dec (ToLclUom (itln_rec.qty_border), 
				store [lcount [ITEM_SCN]].decPt);
		local_rec.qty_rec = (float) local_rec.qty_rec_doub;
		local_rec.qty_ship_doub =
				n_dec (ToLclUom (itln_rec.qty_order), 
				store [lcount [ITEM_SCN]].decPt);
		local_rec.qty_ship = (float) local_rec.qty_ship_doub;
		local_rec.line_no  = itln_rec.line_no;

		local_rec.workCost  = itln_rec.cost;
		local_rec.workDuty  = itln_rec.duty;
		local_rec.transDate   = comm_rec.inv_date;
		strcpy (local_rec.serial_no,itln_rec.serial_no);
		strcpy (local_rec.stock, itln_rec.stock);
		strcpy (local_rec.ln_full_supp, itln_rec.full_supply);
		strcpy (inmr_rec.description, itln_rec.item_desc);

		if (inmr_rec.serial_item [0] == 'Y')
		{
			strcpy (store [lcount [ITEM_SCN]].serial,  itln_rec.serial_no);
			strcpy (store [lcount [ITEM_SCN]].origSerialNo, itln_rec.serial_no);
		}
		else
		{
			sprintf (store [lcount [ITEM_SCN]].serial , "%25.25s"," ");
			sprintf (store [lcount [ITEM_SCN]].origSerialNo, "%25.25s"," ");
		}

		local_rec.qty_rec_doub = (double) local_rec.qty_rec;
		local_rec.qty_ord_doub = (double) local_rec.qty_ord;
		local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		local_rec.qty_ship_doub = (double) local_rec.qty_ship;

		store [lcount [ITEM_SCN]].origHhbrHash = itln_rec.hhbr_hash;
		store [lcount [ITEM_SCN]].hhumHash = itln_rec.hhum_hash;
		store [lcount [ITEM_SCN]].origOrderQty = ToLclUom (itln_rec.qty_order + itln_rec.qty_border);

		putval (lcount [ITEM_SCN]++);
		if (lcount [ITEM_SCN] > MAXLINES)
			break;

		abc_unlock (itln);
		cc = find_rec (itln,&itln_rec,NEXT,"r");
	}
	abc_unlock (itln);

	vars [scn_start].row = lcount [ITEM_SCN];
	scn_set (HEADER_SCN);
	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (inum,"inum_uom");
	return ((lcount [ITEM_SCN]) ? 0 : 2);
}

void
SetIssueBranch (
 char	*co_no,
 char	*br_no,
 char	*wh_no,
 long	hhcc_hash,
 int	issue)
{
	if (hhcc_hash == 0L)
	{
		abc_selfield (ccmr, "ccmr_id_no");
		strcpy (ccmr_rec.co_no,  co_no);
		strcpy (ccmr_rec.est_no, br_no);
		strcpy (ccmr_rec.cc_no,  wh_no);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "ccmr", "DBFIND");
	}
	else
	{
		abc_selfield (ccmr, "ccmr_hhcc_hash");
		ccmr_rec.hhcc_hash	=	hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL,"r");
		if (cc)
			file_err (cc, "ccmr", "DBFIND");

		abc_selfield (ccmr, "ccmr_id_no");
	}
	strcpy (esmr_rec.co_no,ccmr_rec.co_no);
	strcpy (esmr_rec.est_no,ccmr_rec.est_no);
	cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	strcpy (comr_rec.co_no,ccmr_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	if (issue)
	{
		if (!MC_TRAN)
		{
			strcpy (recCo,comr_rec.co_no);
			strcpy (recCoshort,comr_rec.co_short_name);
		}
		issHhccHash = ccmr_rec.hhcc_hash;
		strcpy (issWh,ccmr_rec.cc_no);
		strcpy (issWhname,ccmr_rec.name);
		strcpy (issWhshort,ccmr_rec.acronym);
		strcpy (issBr,esmr_rec.est_no);
		strcpy (issBrname,esmr_rec.est_name);
		strcpy (issBrshort,esmr_rec.short_name);
		strcpy (issBrAddr [0],esmr_rec.adr1);
		strcpy (issBrAddr [1],esmr_rec.adr2);
		strcpy (issBrAddr [2],esmr_rec.adr3);
		strcpy (issCo,comr_rec.co_no);
		strcpy (issCoshort,comr_rec.co_short_name);
	}
	else
	{
		if (!MC_TRAN)
		{
			strcpy (issCo,comr_rec.co_no);
			strcpy (issCoshort,comr_rec.co_short_name);
		}
		recHhccHash = ccmr_rec.hhcc_hash;
		strcpy (recWh,ccmr_rec.cc_no);
		strcpy (recWhname,ccmr_rec.name);
		strcpy (recWhshort,ccmr_rec.acronym);
		strcpy (recBr,esmr_rec.est_no);
		strcpy (recBrname,esmr_rec.est_name);
		strcpy (recBrshort,esmr_rec.short_name);
		strcpy (recBrAddr [0],esmr_rec.adr1);
		strcpy (recBrAddr [1],esmr_rec.adr2);
		strcpy (recBrAddr [2],esmr_rec.adr3);
		strcpy (recArea,esmr_rec.area_code);
		strcpy (recCo,comr_rec.co_no);
		strcpy (recCoshort,comr_rec.co_short_name);
	}
}

int
ItemError (
 char	*message)
{
	int		i;

	for (i = 0; i < 5; i++)
	{
		errmess (message);
		sleep (sleepTime);
	}

	return (RETURN_ERR);
}
/*========================================
| Clear popup window ready for new item. |
========================================*/
void
clear_win (
 void)
{
	int		i;
	for (i = 18;i < 24;i++)
	{
		move (0,i);
		cl_line ();
	}
}
/*=====================================================================
| Input responses to stock quantity on hand less-than input quantity. |
=====================================================================*/
int
InputResponse (
 void)
{
	int		i;
	int		fs_flag = FALSE;
	int		displayed = FALSE;
	char	val_keys [21];
	char	disp_str [300];

	cc = 0;

	if (SERIAL)
	{
		strcpy (val_keys, "CcDd");

		sprintf (disp_str,
			ML ("%s (C)ancel%s %s (D)isplay%s"),
			ta [8], ta [9], ta [8], ta [9]);
	}
	else
	{
		strcpy (val_keys, "OoCcRrDd");

		sprintf (disp_str,
			ML ("%s (O)verride%s %s (C)ancel%s %s (R)educe%s %s (D)isplay%s"),
			ta [8], ta [9], ta [8], ta [9],
			ta [8], ta [9], ta [8], ta [9]);
	}


	if (strcmp (inmr_rec.alternate,sixteen_space))
	{
		sprintf (err_str, ML ("%s (S)ubstitute%s"), ta [8], ta [9]);

		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}

	if (BO_OK)
	{
		if (FULL_BO ||
		  ((TR_ISSUE || TR_R_CON || TR_I_CON) &&
			 ithr_rec.full_supply [0] == 'Y'))
		{
			sprintf (err_str, ML ("%s (F)orce b/o%s"), ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "Ff");
		}
		else
		{
			sprintf (err_str, ML ("%s (B)ackorder bal%s %s (F)orce b/o%s"),
				ta [8], ta [9], ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "BbFf");
		}
	}
	while (1)
	{

		if (in_phantom)
			i = phantomOption;
		else
			i = prmptmsg (disp_str, val_keys, 1, 2);

		phantomOption = i;

		BusyFunction (0);
		switch (i)
		{
		/*------------------------
		| Accept Quantity input. |
		------------------------*/
		case	'O':
		case	'o':
			break;

		/*--------------------
		| Backorder Balance. |
		--------------------*/
		case	'B':
		case	'b':
			local_rec.qty_bord = local_rec.qty_ord;
			local_rec.qty_ord = ToLclUom (SR.issAvailable);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;

			local_rec.qty_bord -= local_rec.qty_ord;

			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);
			DSP_FLD ("qtyOrder");

			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
			local_rec.qty_bord_doub = n_dec (local_rec.qty_bord_doub,SR.decPt);
			DSP_FLD ("qtyBackorder");
			fs_flag = TRUE;
			break;

		/*------------------------------------------------------
		| Cancel Quantity input and check if log to lost sale. |
		------------------------------------------------------*/
		case	'C':
		case	'c':
			local_rec.qty_ord = 0.00;
			local_rec.qty_ord_doub = 0.00;
			skip_entry = goto_field (label ("qtyOrder"),label ("itemNumber"));
			if (ENTER_DATA)
				blank_display ();
			break;

		/*----------------------------------------------
		| Display other details by running so_win_dsp. |
		----------------------------------------------*/
		case	'D':
		case	'd':
#ifdef GVISION
			DisplayStockWindow (SR.hhbrHash, issHhccHash);
#else
			BusyFunction (1);
			if (!wpipe_open)
			{
				if (OpenStockWindow ())
					break;
			}
			fprintf (pout2,"%10ld%10ld\n", SR.hhbrHash,
					      	issHhccHash);

			fflush (pout2);
			IP_READ (np_fn);
			BusyFunction (0);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*------------------------------------------------------
		| Quantity has been reduced to equal quantity on hand. |
		------------------------------------------------------*/
		case	'R':
		case	'r':
			local_rec.qty_ord = ToLclUom (SR.issAvailable);
			if (local_rec.qty_ord < 0.00)
				local_rec.qty_ord = 0.00;
			local_rec.qty_bord = 0.00;

			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);
			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
			local_rec.qty_bord_doub = n_dec (local_rec.qty_bord_doub,SR.decPt);
			break;

		/*------------------------------
		| Substitute Alternate number. |
		------------------------------*/
		case	'S':
		case	's':
			sprintf (err_str,"%s",clip (inmr_rec.alternate));
			sprintf (inmr_rec.item_no,"%-16.16s",err_str);

			cc = FindInmr (issCo, inmr_rec.item_no, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, inmr_rec.item_no);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			if (!TR_RECEIPT && !TR_R_CON && !TR_I_CON)
			{
				strcpy (itln_rec.item_desc,inmr_rec.description);
			}
			skip_entry = (cc) ? -4 : -1;
			break;

		/*-----------------------------
		| Force a complete backorder. |
		-----------------------------*/
		case	'F':
		case	'f':
			local_rec.qty_bord = local_rec.qty_ord;
			local_rec.qty_ord = 0.00;

			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);
			DSP_FLD ("qtyOrder");

			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
			local_rec.qty_bord_doub = n_dec (local_rec.qty_bord_doub, SR.decPt);
			DSP_FLD ("qtyBackorder");
			fs_flag = TRUE;
			break;
		}
		print_at (2,1, "%90.90s"," ");

		break;
	}

#ifdef GVISION
	HideStockWindow ();
#else
	if (displayed)
		clear_win ();
#endif  /* GVISION */

	if (ithr_rec.full_supply [0] == 'A' && envVar.fullSupplyOrder && fs_flag)
		LineFullSupply ();

	return ((envVar.fullSupplyOrder) ? fs_flag : FALSE);
}

/*------------------------
| Check If User Wants To |
| Full-Supply The Line.  |
------------------------*/
void
LineFullSupply (
 void)
{
	int		i;

	strcpy (local_rec.ln_full_supp, "N");
	sprintf (err_str, ML (mlSkMess430), recWhshort);
	i = prmptmsg (err_str,"YyNn",1,2);
	BusyFunction (0);
	if (i == 'Y' || i == 'y')
	{
		strcpy (local_rec.ln_full_supp, "L");
		local_rec.qty_bord += local_rec.qty_ord;
		local_rec.qty_ord = 0.00;

		local_rec.qty_ord_doub = (double) local_rec.qty_ord;
		local_rec.qty_ord_doub = n_dec (local_rec.qty_ord_doub, SR.decPt);
		local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		local_rec.qty_bord_doub = n_dec (local_rec.qty_bord_doub, SR.decPt);
	}
	return;
}

/*============================
| Warn user about something. |
============================*/
int
warn_user (
 char *wn_mess)
{
	clear_mess ();
	print_mess (wn_mess);
	return (SUCCESS);
}

void
BusyFunction (
 int		flip)
{
	print_at (2,1, "%-90.90s"," ");
	if (flip)
		print_at (2,1, ML (mlStdMess035));

	fflush (stdout);
}

/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
tab_other (
 int	iline)
{
	char	status_desc [20];
	char	ln_desc [40];
	char	SkWork [20];

	if (cur_screen == 2)
	{
		if (!TR_R_ISS && SERIAL)
			FLD ("ser_no") = YES;

		if (!F_HIDE (label ("UOM")))
			FLD ("UOM") = (SERIAL) ? NA : YES;

		if (prog_status == ENTRY)
			return;

		strcpy (status_desc, "                   ");
		if (itln_rec.status [0] == 'B')
			strcpy (status_desc, ML ("Backorder line     "));

		if (itln_rec.status [0] == 'M')
			strcpy (status_desc, ML ("Manual Release line"));

		if (itln_rec.status [0] == 'U')
			strcpy (status_desc, ML ("Unconfirmed TR line"));

		if (itln_rec.status [0] == 'T')
			strcpy (status_desc, ML ("Transfer line      "));

		strcpy (ln_desc, "                                     ");

		if (local_rec.ln_full_supp [0] == 'Y')
			strcpy (ln_desc, ML (mlSkMess431));

		if (local_rec.ln_full_supp [0] == 'L')
			strcpy (ln_desc, ML (mlSkMess437));

		strcpy (SkWork, (local_rec.stock [0] == 'S') ? ML ("Stock   ")
												    : ML ("Customer"));
		print_at (4,0,ML (mlSkMess438),SkWork, status_desc, ln_desc);
	}
	return;
}

void
HeadingComment (
 char	*message,
 int	pos)
{
	int		mess_len;

	mess_len = (130 - (int) strlen (message)) / 2;

	rv_pr (message,mess_len,pos,!pos);
}

/*==================================
| Add freight history information. |
==================================*/
void
AddCarrierDetails (
 void)
{
	open_rec (trch, trch_list, TRCH_NO_FIELDS, "trch_id_no");

	strcpy (trch_rec.co_no, 	issCo);
	strcpy (trch_rec.br_no, 	issBr);
	strcpy (trch_rec.wh_no, 	issWh);
	strcpy (trch_rec.ref_no, 	userReferenceNo);
	trch_rec.date 			= 	local_rec.transDate;
	trch_rec.hhcu_hash 		= 	0L;
	strcpy (trch_rec.cons_no, 	ithr_rec.cons_no);
	strcpy (trch_rec.carr_code, ithr_rec.carr_code);
	strcpy (trch_rec.del_zone, 	ithr_rec.del_zone);
	trch_rec.no_cartons 	= 	ithr_rec.no_cartons;
	trch_rec.no_kgs 		= 	ithr_rec.no_kgs;
	trch_rec.est_frt_cst 	= 	DOLLARS (local_rec.efreight);
	trch_rec.act_frt_cst 	= 	DOLLARS (ithr_rec.frt_cost);
	strcpy (trch_rec.cumr_chg,	 (ithr_rec.frt_cost > 0.00) ?  "Y" : "N");
	strcpy (trch_rec.stat_flag, "0");

	cc = abc_add (trch, &trch_rec);
	if (cc)
		file_err (cc, "trch", "DBADD");
}

int
CheckInsf (
 long	hhbrHash,
 char	*ser_no)
{
	if (!strcmp (ser_no, ser_space))
		return (FALSE);

	open_rec ("soln", soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");

	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("soln", &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhbr_hash == hhbrHash)
	{
		if (!strcmp (soln_rec.serial_no, ser_no))
		{
			abc_fclose ("soln");
			return (TRUE);
		}
		cc = find_rec ("soln", &soln_rec, NEXT, "r");
	}
	abc_fclose ("soln");

	open_rec ("coln", coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");

	coln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("coln", &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhbr_hash == hhbrHash)
	{
		if (!strcmp (coln_rec.serial_no, ser_no))
		{
			abc_fclose ("coln");
			return (TRUE);
		}
		cc = find_rec ("coln", &coln_rec, NEXT, "r");
	}
	abc_fclose ("coln");

	return (FALSE);
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
void
ProcessKitItem (
 long	hhbrHash,
 float	qty)
{
	int		i;
	int		this_page;
	float	lcl_qty	=	0;

	this_page = line_cnt / TABLINES;

	cc = open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_id_no");
	if (cc)
		file_err (cc, "sokt", "OPEN_REC");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = hhbrHash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
		   !restart &&
		   !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
		   sokt_rec.hhbr_hash == hhbrHash)
	{
		abc_selfield (inmr,"inmr_hhbr_hash");
		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		dflt_used = FALSE;

		if (ValidateItemNo ())
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		if (SR.issHhwhHash < 0L)
		{
			sprintf (soln_rec.serial_no, "%25.25s", " ");
			strcpy (SR.serial, soln_rec.serial_no);
			DSP_FLD ("ser_no");
		}

		if (SERIAL)
		{
			lcl_qty = sokt_rec.matl_qty * qty;
			local_rec.qty_ord = 1.00;
			local_rec.qty_bord = 0.00;
		}
		else
		{
			local_rec.qty_ord = sokt_rec.matl_qty * qty;
			local_rec.qty_bord = 0.00;
		}

		if (local_rec.qty_ord == 0.00)
			get_entry (label ("qtyOrder"));
		
		if (local_rec.qty_ord == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		local_rec.qty_rec_doub = (double) local_rec.qty_rec;
		local_rec.qty_ord_doub = (double) local_rec.qty_ord;
		local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		local_rec.qty_ship_doub = (double) local_rec.qty_ship;
		DSP_FLD ("qtyOrder");

		/*-----------------------------
		| if serial we need to to load
		| one line per qty ordered.
		------------------------------*/

		if (SERIAL)
		{
			int	count;
			for (count = 0; count < lcl_qty; count++)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");

				dflt_used = FALSE;

				if (ValidateItemNo ())
					break;

				DSP_FLD ("qtyOrder");
				strcpy (local_rec.location, "          ");
				strcpy (local_rec.serial_no, ser_space);
				for (i = label ("qtyOrder"); i <= label ("ln_full_supply"); i++)
				{
					skip_entry = 0;
					do
					{
						if (SERIAL && 
							FLD ("ser_no") != NA &&
							FLD ("ser_no") != ND &&
							i == label ("ser_no"))
							get_entry (i);

						if (!restart)
						{
							cc = spec_valid (i);
							/*-----------------------------------------
							| if spec_valid returns 1, re-enter field |
							| eg. if kit item has no sale value,      |
							| re-prompt for sal value if required.    |
							-----------------------------------------*/
							if (cc && ! (SERIAL && i == label ("ser_no")))
								get_entry (i);
						}

					} while (cc && !restart);
					i += skip_entry;
					if (restart)
						break;
				}
			
				local_rec.qty_rec_doub = (double) local_rec.qty_rec;
				local_rec.qty_ord_doub = (double) local_rec.qty_ord;
				local_rec.qty_bord_doub = (double) local_rec.qty_bord;
				local_rec.qty_ship_doub = (double) local_rec.qty_ship;
				putval (line_cnt);

				if (this_page != (line_cnt / TABLINES))
				{
					scn_write (cur_screen);
					lcount [ITEM_SCN] = line_cnt;
					this_page = line_cnt / TABLINES;
				}
				lcount [ITEM_SCN] = line_cnt;
				
				line_display ();
				line_cnt++;
				if (restart)
					break;
			}
		}
		else
		{
			strcpy (local_rec.location, "          ");
			strcpy (local_rec.serial_no, ser_space);
			for (i = label ("qtyOrder"); i <= label ("ln_full_supply"); i++)
			{
				skip_entry = 0;
				do
				{
					if (SERIAL && 
						FLD ("ser_no") != NA &&
						FLD ("ser_no") != ND &&
						i == label ("ser_no"))
						get_entry (i);

					if (restart)
						break;

					if (restart)
						break;

					cc = spec_valid (i);
					/*-----------------------------------------
					| if spec_valid returns 1, re-enter field |
					| eg. if kit item has no sale value,      |
					| re-prompt for sal value if required.    |
					-----------------------------------------*/
				} while (cc && !restart);
				i += skip_entry;
				if (restart)
					break;
			}
		
			local_rec.qty_rec_doub 	= (double) local_rec.qty_rec;
			local_rec.qty_ord_doub 	= (double) local_rec.qty_ord;
			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
			local_rec.qty_ship_doub = (double) local_rec.qty_ship;
			putval (line_cnt);

			if (this_page != (line_cnt / TABLINES))
			{
				scn_write (cur_screen);
				lcount [ITEM_SCN] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			lcount [ITEM_SCN] = line_cnt;
			
			line_display ();
			line_cnt++;
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [ITEM_SCN] = line_cnt;
	abc_fclose (sokt);
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
void
ProcessPhantom (
 long	hhbrHash,
 float	qty,
 float	qty_bord)
{
	int		i;
	int		this_page;
	int		first_time = TRUE;

	this_page = line_cnt / TABLINES;

	open_rec ("sokt", sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");

	
	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		if (first_time)
		{
			local_rec.qty_ord = 0.00;
			local_rec.qty_bord = 0.00;

			local_rec.qty_rec_doub = (double) local_rec.qty_rec;
			local_rec.qty_ord_doub = (double) local_rec.qty_ord;
			local_rec.qty_bord_doub = (double) local_rec.qty_bord;
			local_rec.qty_ship_doub = (double) local_rec.qty_ship;
			putval (line_cnt);

			if (this_page != (line_cnt / TABLINES))
			{
				scn_write (cur_screen);
				lcount [ITEM_SCN] = line_cnt;
				this_page = line_cnt / TABLINES;
			}
			lcount [ITEM_SCN] = line_cnt;

			line_display ();
			line_cnt++;
			first_time = FALSE;
		}
		abc_selfield (inmr,"inmr_hhbr_hash");

		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
			first_time = FALSE;
			continue;
		}

		dflt_used = FALSE;

		if (ValidateItemNo ())
		{
			cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
			first_time = FALSE;
			continue;
		}
		local_rec.qty_ord = sokt_rec.matl_qty * qty;
		local_rec.qty_bord = sokt_rec.matl_qty * qty_bord;
		if (sokt_rec.matl_qty == 0.00)
			get_entry (label ("qtyOrder"));

		if (local_rec.qty_ord == 0.00 && local_rec.qty_bord == 0.00)
		{
			cc = find_rec ("sokt",&sokt_rec,NEXT,"r");
			first_time = FALSE;
			continue;
		}
		dflt_used = TRUE;
		in_phantom = TRUE;
		for (i = label ("qtyOrder"); i <= label ("LL") ; i++)
		{
			strcpy (local_rec.LL, "Y");
			while (spec_valid (i))
				get_entry (i);
		}
		in_phantom = FALSE;

		local_rec.qty_rec_doub 	= (double) local_rec.qty_rec;
		local_rec.qty_ord_doub 	= (double) local_rec.qty_ord;
		local_rec.qty_bord_doub = (double) local_rec.qty_bord;
		local_rec.qty_ship_doub = (double) local_rec.qty_ship;
		putval (line_cnt);

		if (this_page != (line_cnt / TABLINES))
		{
			scn_write (cur_screen);
			lcount [ITEM_SCN] = line_cnt;
			this_page = line_cnt / TABLINES;
		}
		lcount [ITEM_SCN] = line_cnt;

		line_display ();
		line_cnt++;

		cc = find_rec ("sokt",&sokt_rec,NEXT,"r");
	}
	lcount [ITEM_SCN] = line_cnt;
	abc_fclose ("sokt");
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
float	
phantom_avail (
 long	hhbrHash)
{
	int		first_time = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;

	open_rec ("sokt", sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = issHhccHash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
			continue;
		}
		on_hand = incc_rec.closing_stock -
				  incc_rec.committed -
				  incc_rec.backorder - 
		          incc_rec.forward;

		if (envVar.qcApplied && envVar.qCAvailable)
			on_hand = incc_rec.qc_qty;

		on_hand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
	}
	abc_fclose ("sokt");

	return (min_qty);
}

float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR.cnvFct;

	return (cnvQty);
}

float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.cnvFct;

	return (cnvQty);
}

/*===============================
| Open Transport related files. |
===============================*/
void
OpenTransportFiles (
 char	*ZoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, ZoneIndex);
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
}
/*================================
| Close Transport related files. |
================================*/
void
CloseTransportFiles (void)
{
	abc_fclose (trzm);
	abc_fclose (trcm);
	abc_fclose (trcl);
}
/*===================================
| Check if transfer already exists. |
===================================*/
int
CheckIthr (
 long	tr_no)
{
	char	type [2];

	strcpy (type,"T");
	if (TR_R_ISS)
		strcpy (type,"R");

	if (TR_I_TWO)
		strcpy (type,"U");

	if (TR_I_ONE || TR_I_RMT)
		strcpy (type,"M");

	strcpy (ithr2_rec.co_no, comm_rec.co_no);
	strcpy (ithr2_rec.type, type);
	ithr2_rec.del_no = tr_no;
	return (find_rec (ithr2, &ithr2_rec, COMPARISON, "r"));
}

/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment (
 void)
{
	char	*sptr;
	int		after	=	0,
			before	=	0;

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);

	before	= strlen (local_rec.dflt_qty);
	sptr	= strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	currentUser = getenv ("LOGNAME");

	/*-------------------------------------------------
	| Check if stock information window is displayed. |
	-------------------------------------------------*/
	sptr = chk_env ("WIN_OK");
	envVar.windowPopupOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-----------------------------------------------------------
	| Check if stock information window is loaded at load time. |
	-----------------------------------------------------------*/
	sptr = chk_env ("SO_PERM_WIN");
	envVar.perminantWindow = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envVar.perminantWindow)
	{
		if (OpenStockWindow ())
			envVar.windowPopupOk = FALSE;
	}
	sptr = chk_env ("SK_TR_CONF");
	envVar.transferConfirm = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*------------------------------------------------------------
	| Check for Stock transfers M (anual or S (ystem generated). |
	------------------------------------------------------------*/
	sptr = chk_env ("SK_TRANS");
	envVar.transferNumbering = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;

	/* QC module is active or not. */
	envVar.qcApplied = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	/* Whether to include QC qty in available stock. */
	envVar.qCAvailable = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*-------------------------------------
	| Check for Automatic freight charge. |
	-------------------------------------*/
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envVar.automaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-------------------------------------------
	| Check for sales order full supply option. |
	-------------------------------------------*/
	sptr = chk_env ("SO_FULL_SUPPLY");
	envVar.fullSupplyOrder = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*--------------------------
	| How if Freight Charged.  |
	--------------------------*/
	sptr = chk_env ("SO_FREIGHT_CHG");
	envVar.soFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);

    /*-----------------------------------------------------------------------
	| Check if system will zero the inei_avge_cost if qty is less than 0.00  |
	-------------------------------------------------------------------------*/
	sptr = chk_env ("ALLOW_ZERO_COST");
		envVar.zeroCost = (sptr == (char *) 0) ? 0 : atoi (sptr);
	sptr = chk_env ("IKEA_HK");
		envVar.IKHK = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-----------------------------------
	| Check if include forward in avail |
	-----------------------------------*/
	sptr = chk_env ("SO_FWD_AVL");
		envVar.soFwdAvl = (sptr == (char *) 0) ? TRUE : atoi (sptr);

}

#ifndef GVISION
int
OpenStockWindow (
 void)
{
	np_fn = IP_CREATE (getpid ());
	if (np_fn < 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	if ((pout2 = popen ("so_pwindow","w")) == 0)
	{
		envVar.windowPopupOk = FALSE;
		return (EXIT_FAILURE);
	}
	wpipe_open = TRUE;
	fprintf (pout2, "%06d\n", getpid ());
	return (SUCCESS);
}
#endif	/* GVISION */

float	
GetBrClosing (
 char	*coNo,
 char	*brNo,
 long	hhbrHash)
{
	float	ClosingStock = 0.00;

	sprintf (ccmr_rec.co_no,  "%-2.2s", coNo);
	sprintf (ccmr_rec.est_no, "%-2.2s", brNo);
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strncmp (ccmr_rec.co_no, coNo, (int) strlen (coNo)) && 
				   !strncmp (ccmr_rec.est_no, brNo, (int) strlen (brNo)))
	{
		incc2_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc2_rec.hhbr_hash = hhbrHash;

		if (!find_rec (incc , &incc2_rec, COMPARISON, "r"))
			ClosingStock += incc2_rec.closing_stock;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	return (ClosingStock);
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	line_at (1,0,130);
	if (MC_TRAN)
		HeadingComment (ML (mlSkMess652) ,0);

	if (TR_RECEIPT)
		HeadingComment (ML (mlSkMess653),MC_TRAN);

	if (TR_I_RMT)
		HeadingComment (ML (mlSkMess654),MC_TRAN);

	if (TR_I_DIR)
		HeadingComment (ML (mlSkMess655),MC_TRAN);

	if (TR_I_ONE)
		HeadingComment (ML (mlSkMess656),MC_TRAN);

	if (TR_I_TWO)
		HeadingComment (ML (mlSkMess657),MC_TRAN);

	if (TR_R_ISS)
		HeadingComment (ML (mlSkMess658),MC_TRAN);

	if (TR_R_CON)
		HeadingComment (ML (mlSkMess659),MC_TRAN);

	if (TR_I_CON)
		HeadingComment (ML (mlSkMess660),MC_TRAN);

	if (TR_DISPLAY)
		HeadingComment (ML (mlSkMess661),MC_TRAN);

	print_at (0, 100,  ML (mlSkMess439), local_rec.previousRef);

	if (scn == 1)
	{
		line_at (7, 1,130);
		line_at (11,1,130);
		box (0,3,130,11);
	}
	if (scn == 3)
	{
		CalculateFreight 
		 (
			trcm_rec.markup_pc, 
			trcl_rec.cost_kg,
			trzm_rec.chg_kg,
			trzm_rec.dflt_chg
		);
		box (0,3,130,5);
	}
	line_cnt = 0;
	scn_write (scn);
	PrintCoStuff ();
    return (EXIT_SUCCESS);
}

void
PrintCoStuff (
 void)
{
	line_at (20,0,130);

	print_at (21,0,  ML (mlSkMess634),  issCo, issCoshort);
	print_at (21,35, ML (mlStdMess039), issBr, issBrshort);
	print_at (21,70, ML (mlStdMess099), issWh, issWhname);

	print_at (22,0,  ML (mlSkMess635),  recCo, recCoshort);
	print_at (22,35, ML (mlStdMess039), recBr, recBrshort);
	print_at (22,70, ML (mlStdMess099), recWh, recWhname);
}

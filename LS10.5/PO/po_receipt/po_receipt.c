/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_receipt.c,v 5.25 2002/07/24 08:39:06 scott Exp $
|  Program Name  : (po_receipt.c  )                                   |
|  Program Desc  : (Purchase order Receipts.                      )   |
|---------------------------------------------------------------------|
|  Date Written  : (15/03/1989)    | Author       : Scott Darrow      |
|---------------------------------------------------------------------|
| $Log: po_receipt.c,v $
| Revision 5.25  2002/07/24 08:39:06  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.24  2002/06/20 07:22:10  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.23  2002/06/19 07:00:41  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.22  2002/05/02 01:37:01  scott
| Updated to add Archive functions
|
| Revision 5.21  2002/01/07 01:17:36  scott
| Updated for returns
|
| Revision 5.20  2001/12/11 09:11:42  scott
| Updated to ensure ignore location flag is on for returns.
|
| Revision 5.19  2001/12/06 05:22:36  scott
| Updated to allow purchase returns that are allocated to an original purchase order to select lot information. The lot display will only show those Location/Lot records that belong to the original receipt. This caters for both multiple receipts of the same product on the same purchase order AND Locations/Lots being split after receipt.
|
| Revision 5.18  2001/12/03 01:28:28  scott
| Updated to allow containers on shipments.
|
| Revision 5.17  2001/11/12 08:33:15  scott
| Updated to have seperate error messages for returns and po.
|
| Revision 5.16  2001/11/07 05:09:58  robert
| Updated to fix exit problem on LS10-GUI
|
| Revision 5.15  2001/10/30 02:12:39  cha
| Fix     Issue #00645 - Dispatch Goods Returns.
| Changes done by Scott.
|
| Revision 5.14  2001/10/25 02:32:39  scott
| Updated to ensure only open lines displayed.
|
| Revision 5.13  2001/10/22 04:17:34  scott
| Updated from testing
|
| Revision 5.12  2001/10/10 00:31:46  scott
| Updated to change MAXWIDTH
|
| Revision 5.11  2001/10/09 23:02:48  scott
| Lost #define for CCMAIN
|
| Revision 5.10  2001/10/09 22:57:21  scott
| Updated for purchase returns
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_receipt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_receipt/po_receipt.c,v 5.25 2002/07/24 08:39:06 scott Exp $";

#define MAXSCNS 	3
#define MAXWIDTH	280
#define	MAXLINES	1000
#define	TXT_REQD

#define	HEADER_SCN	1
#define	DESC_SCN	2
#define	ITEM_SCN	3

#include <errno.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <pslscr.h>
#include <minimenu.h>
#include <arralloc.h>

#define	SR			store [line_cnt]
#define	SRL			store [lcount [ITEM_SCN]]
#define	SERIAL		(SR.ser_item [0] == 'Y')
#define	INPUT		(prog_status == ENTRY)
#define	CASE_USED	(envPoCaseUsed [0] == 'Y')
#define	CONFIRM		0
#define	SEL_IGNORE	1
#define	CF			comma_fmt

#include	"schema"

typedef	int	BOOL;

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct inccRecord	incc_rec;
struct ineiRecord	inei_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;
struct ponsRecord	pons_rec;
struct insfRecord	insf_rec;
struct inumRecord	inum_rec;
struct pogdRecord	pogd_rec;
struct poghRecord	pogh_rec;
struct poghRecord	pogh2_rec;
struct poglRecord	pogl_rec;
struct poglRecord	pogl2_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct pohsRecord	pohs_rec;
struct polnRecord	poln_rec;
struct posdRecord	posd_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;
struct poceRecord	poce_rec;
struct poslRecord	posl_rec;
struct sumrRecord	sumr_rec;
struct suphRecord	suph_rec;
struct posoRecord	poso_rec;
struct qcmrRecord	qcmr_rec;
struct qchrRecord	qchr_rec;
struct sknhRecord	sknh_rec;
struct skndRecord	sknd_rec;
struct skniRecord	skni_rec;
struct trveRecord	trve_rec;
struct inuvRecord	inuv_rec;
struct skcmRecord	skcm_rec;
	 
	/*
	 * Special fields and flags.
	 */
	int		envVarCrCo 			= 0,
			envVarDbCo 			= 0,
			envVarCrFind		= 0,
			envVarDbFind		= 0,
			envPoSuHist 		= 0,
			envVarPoContainer	= 0,
			envPoChkDfltLoc 	= 0,
			envPoRecSortDes 	= 0,
			envQcApply 			= 0,
			envPoRecShip 		= 0,
			envIkeaSystem 		= 0,
			envSkSerialOk 		= 0,
			envSkGrinNoPlate 	= 0,
			envVarThreePl		= 0,
			PageNumUpdate		= 0,
			newPogh				= 0,
			shipment			= 0,
			printerNumber		= 0,
			dsByWhat			= 0,
			genNewGrnNo			= 0,
			PO_RETURN 			= FALSE,
			plateCalled			= 0;

	char 	*data	= "data",
			*pohr2	= "pohr2",
			*poln2	= "poln2",
			*pogl2	= "pogl2",
			*posh2	= "posh2",
			*twentyfiveSpaces 	= "                         ",
			*fifteenSpaces 		= "               ";

	char	branchSupp [3],
			branchCust [3],
			envPoCaseUsed [2];
	
	double	totalEstimatedCost = 0; 

	char	*scn_desc [] = 
			{
				"Goods receipt Header",
				"Number Plate Details",
				"Item Screen"
			};

/*
 * Local & Screen Structures
 */
struct {
							
	char	dummy [11];				
	char	currSerItem [17];

	char	previousPO [sizeof pohr_rec.pur_ord_no];	
	char	previousSupplier [7];		
	char	systemDate [11];		
	long	lsysDate;		
	char	supplierNumber [7];
	float	qtyOrdered;
	float	qtyReceived;	
	float	actReceipt;	
	float	qtySupplied;	
	float	packageQty;
	float	totalChargeWgt;
	float	totalGrossWgt;
	float	totalCBM;
	char	uom [5];	
	float	CnvFct;		
	double	landCost;					
	char	_location [11]; 		
	char	df_serial [26]; 	
	char	serial [26];    
	char	vessel [21]; 
	char	headerLotNo [8]; 
	char	defaultLotNo [8]; 
	char	itemDesc [41]; 
	int		caseNumber;				
	char	lot_ctrl [2];			
	long	expiryDate;
	long	hhbrHash;	
	long	hhccHash;		
	char	qcReqd [2];			
	char	qcCentre [5];			
	char	OrderPrmpt [21],		
			ReceivePrmpt [21],
			PoPrmpt [21];
	char	cusOrdRef [sizeof pogl_rec.cus_ord_ref];
	char	previousCusOrdRef [sizeof pogl_rec.cus_ord_ref];
	char	LL [2];
	char	labNote [61];
	char	containerNo [16];
} local_rec;

struct storeRec {
							/*-------------------------------------------*/
	long	hhbrHash;       /*| Inventory master file hash.             |*/
	long	hhccHash;       /*| Warehouse master file hash.             |*/
	long	hhwhHash;       /*| Warehouse serial hash.                  |*/
	long	hhplHash;       /*| Purchase order line hash.               |*/
	long	hhplOrig;       /*| Purchase order line hash (Origional)    |*/
	long	hhumHash;       /*| Purchase UOM hash.                      |*/
	char	cost_type [2];  /*| Costing Type.                           |*/
	char	ser_item [2];   /*| Costing Type.                           |*/
	char	lot_ctrl [2];	/*| Is item lot-controlled.		    		|*/
	char	uom [5];		/*| Unit of measure.						|*/
	float	CnvFct;			/*| Conversion factor.						|*/
	long	exp_date;	    /*| Lot Expiry Date.                        |*/
	char	qcLocation [11];/*| QC Location                             |*/
	char	serial_no [26]; /*| Serial Number.                          |*/
	char	alloc_ser [26]; /*| Serial Number.                          |*/
	char	dflt_ser [26];  /*| Default Serial number.                  |*/
	char	err_found [2];	/*| Error on line found, Cannot Confirm.    |*/
	char	itemDesc [41];	/*| Item description.			    		|*/
	char	sup_part [17];  /*| Supplier Part number.                   |*/
	char	br_no [3];		/*| Branch number.			                |*/
	char	wh_no [3];		/*| Warehouse number.			            |*/
	char	acronym [10];	/*| Warehouse acronym.			    		|*/
	char	qcReqd [2];		/*| QC Check Required.			            |*/
	char	qcCentre [5];	/*| QC Centre. 		       		            |*/
	double	landCost;		/*| Land Cost.	        		            |*/
	double	outer;			/*| Outer Size (inmr)      		            |*/
	char	_class [2];		/*| Item Class.        		                |*/
							/*-------------------------------------------*/
} store [MAXLINES];

/*
 *	Structure for dynamic array,  for the shipment lines for qsort
 */
struct Shipment
{
	char	polnItemDesc [41];
	long	polnHash;
	long	poslHash;
	long	pohrHash;
}	*ship;
	DArray shipment_d;
	int	shipCnt = 0;

static	struct	var	vars [] =
{
	{HEADER_SCN, LIN, "ship_no",	 4, 20, CHARTYPE,
		"NNNNNNNNNNNN", "          ",
		" ", " ", "Shipment No.", " ",
		 NE, NO,  JUSTLEFT, "", "", posh_rec.csm_no},
	{HEADER_SCN, LIN, "vessel",	 4, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Vessel ", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.vessel},
	{HEADER_SCN, LIN, "container", 4, 100, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Container No  ", "Enter Container No. [SEARCH]", 
		NE, NO, JUSTLEFT, "", "", local_rec.containerNo}, 
	{HEADER_SCN, LIN, "poNumber",	 5, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", local_rec.PoPrmpt, " ",
		 NE, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no},
	{HEADER_SCN, LIN, "caseNumber",	 5, 60, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Case # ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.caseNumber},
	{HEADER_SCN, LIN, "supplierNumber",	 6, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier No.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierNumber},
	{HEADER_SCN, LIN, "supplierName",	 6, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{HEADER_SCN, LIN, "vehicle",	 7, 20, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "  ", "Vehicle. ", "Enter Vehicle No. or [SEARCH]",
		ND, NO,  JUSTLEFT, "", "", trve_rec.ref},
	{HEADER_SCN, LIN, "vehicleDesc",	 7, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "  ", "-", " ",
		ND, NO,  JUSTLEFT, "", "", trve_rec.desc},
	{HEADER_SCN, LIN, "grinNumber",	 8, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Goods Received No.", " ",
		YES, NO,  JUSTLEFT, "", "", pogh_rec.gr_no},
	{HEADER_SCN, LIN, "headerLotNo",	 8, 60, CHARTYPE,
		"UUUUUUU", "          ",
		" ", " ", " Lot No   :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.headerLotNo},
	{DESC_SCN, TXT, "noPlateText", 10, 30, 0, "","          ", " "," ",
	"             GOODS RECEIPT NUMBER PLATE DETAILS             ", " ",
	6,60,6, "", "", local_rec.labNote},
	{ITEM_SCN, TAB, "item_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{ITEM_SCN, TAB, "itemDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "    I t e m   D e s c r i p t i o n     ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{ITEM_SCN, TAB, "hhbrHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhbrHash},
	{ITEM_SCN, TAB, "hhccHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhccHash},
	{ITEM_SCN, TAB, "hhpoHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpo_hash},
	{ITEM_SCN, TAB, "hhplHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpl_hash},
	{ITEM_SCN, TAB, "hhplOrig",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpl_orig},
	{ITEM_SCN, TAB, "CnvFct",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.CnvFct},
	{ITEM_SCN, TAB, "qtyOrdered",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", local_rec.OrderPrmpt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.qtyOrdered},
	{ITEM_SCN, TAB, "qty_rec",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", "Prev Receipt", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.qtyReceived},
	{ITEM_SCN, TAB, "qty_torec",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", " ", " ",
		 ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.actReceipt},
	{ITEM_SCN, TAB, "qty_sup",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.ReceivePrmpt, " ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.qtySupplied},
	{ITEM_SCN, TAB, "UOM",		 0, 0, CHARTYPE,
		"AAAA", "          ",
		"", "", "UOM.", " ",
		 NA, NO, JUSTRIGHT, "", "", local_rec.uom},
	{ITEM_SCN, TAB, "packageQty",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Package Qty", "Input Package quantity.",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.packageQty},
	{ITEM_SCN, TAB, "totalChargeWgt",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Charge Wgt", "Input total charge weight.",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalChargeWgt},
	{ITEM_SCN, TAB, "totalGrossWgt",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Gross Wgt.", " ",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalGrossWgt},
	{ITEM_SCN, TAB, "totalCBM",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Total CBM.", " ",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.totalCBM},
	{ITEM_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM_SCN, TAB, "cusOrdRef",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.previousCusOrdRef, " Customer Order Ref ", "",
		ND, NO,  JUSTLEFT, "", "", local_rec.cusOrdRef},
	{ITEM_SCN, TAB, "land_cost",	 0, 0, DOUBLETYPE,
		"NNNN,NNN,NNN.NN", "          ",
		" ", "", "Unit Land Cost", " Landed Cost ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.landCost},
	{ITEM_SCN, TAB, "qcReqd",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", " ", "", "",
		ND, NO,  JUSTLEFT, "", "", local_rec.qcReqd},
	{ITEM_SCN, TAB, "qcCentre",	 0, 0, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Cntr", "QC Centre",
		ND, NO,  JUSTLEFT, "", "", local_rec.qcCentre},
	{ITEM_SCN, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Serial Number      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.serial},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


MENUTAB upd_menu [] =
{
	{ " 1. CONFIRM DELIVERY  ", 
	  " Confirm receipt of direct delivery order. " }, 
	{ " 2. IGNORE DELIVERY ", 
	  " Do not receipt this order.                " }, 
	{ ENDMENU }
};

 
#include	<LocHeader.h>
/*
 * Function Declarations
 */
char 	*GetShipmentNo		(long);
int		CheckDupSerial		(char *, long, int);
int		CheckOk				(void);
int		DescriptionSort		(const	void *,	const void *);
int		IntFindInmr			(long);
int		FindPons			(long);
int		IntFindSupercession	(char *);
int		heading				(int);
int		spec_valid			(int);
void	AddPogl				(int);
void	CalcExpiry			(void);
void	CalculatePosd		(long, long);
void	CheckDeleteLine		(void);
void	CheckEnvironment 	(void);
void	CloseDB				(void);
void	ConfirmAll			(void);
void	GenGrnNo			(void);
void	InsertNewLine		(void);
void	LoadLines			(float, float,	float);
void	LoadPoln			(long);
void	LoadPosl			(long);
void	LoadNoPlateNotes 	(int);
void	UpdateSknh 			(void);
void	OpenDB				(void);
void	PrintCoStuff		(void);
void	ProcessSuph			(void);
void	SrchPogh			(char *);
void	SrchPohr			(char *);
void	SrchPosh			(char *);
void	SrchQcmr			(char *);
void	SrchTrve 			(char *);
void 	SrchSkcm 			(char *);
void	Update				(void);
void	UpdatePogl			(void);
void	UpdatePoln			(void);
void	UpdatePosd			(void);
void	UpdatePosl			(void);
void	UpdateQchr			(void);
void	shutdown_prog		(void);
void	tab_other			(int);
static	BOOL	IsSpaces	(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		i;

	if (argc != 1 && argc != 2)
	{
		print_at (0,0,mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strncmp (sptr, "po_issue",8))
	{
		PO_RETURN = TRUE;
		locStatusIgnore = TRUE;
	}

	CheckEnvironment ();

	FLD ("qcCentre") 	= (envQcApply) 		? YES 	: ND;
	FLD ("ser_no") 		= (envSkSerialOk) 	? YES 	: ND;
	FLD ("UOM") 		= (envSkSerialOk) 	? ND 	: NA;
	FLD ("land_cost") 	= (envSkSerialOk) 	? ND 	: NA;
	FLD ("caseNumber") 	= (CASE_USED) 		? NE 	: ND;

	if (envVarThreePl)
	{
		FLD ("packageQty")		=	YES;
		FLD ("totalChargeWgt")	=	YES;
		FLD ("totalGrossWgt")	=	YES;
		FLD ("totalCBM")		=	YES;
		FLD ("qtyOrdered")		=	ND;
		FLD ("qty_rec")			=	ND;
		FLD ("cusOrdRef")		=	NA;
		FLD ("land_cost")		=	ND;
		FLD ("vehicle")			=	YES;
		FLD ("vehicleDesc")		=	NA;
		FLD ("itemDesc")		= 	ND;
	}
		
	/*
	 * Container used?
	 */
	sptr = chk_env ("PO_CONTAINER");
	envVarPoContainer	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	if (envVarPoContainer)
		FLD ("container")	= YES;
	else
		FLD ("container")	= ND;
	
	printerNumber = (argc == 2) ? atoi (argv [1]) : 0;

	/*
	 * setup required parameters.
	 */
	init_scr 	();
	set_tty 	();
	set_masks 	();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM_SCN, store, sizeof (struct storeRec));
#endif
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsysDate = TodaysDate ();
	
	clear ();
	swide ();

	for (i = 0; i < 4; i++)
		tab_data [i]._desc  = scn_desc [i];

	/*
	 * open main database files.
	 */
	OpenDB ();

	strcpy (branchSupp, (!envVarCrCo) ? " 0" : comm_rec.est_no);
	strcpy (branchCust, (!envVarDbCo) ? " 0" : comm_rec.est_no);

	FLD ("headerLotNo") = (SK_BATCH_CONT && !envSkGrinNoPlate) ? YES : ND;

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	if (PO_RETURN)
	{
		FLD ("ship_no")		= ND;
		FLD ("vessel")		= ND;
		FLD ("container")	= ND;
		FLD ("caseNumber")	= ND;
		FLD ("grinNumber")	= ND;
		FLD ("headerLotNo")	= ND;
		FLD ("qtyOrdered")	= NA;
		FLD ("qty_rec")		= ND;
		FLD ("qty_torec")	= ND;
		FLD ("qty_sup")		= YES;
		FLD ("qcCentre") 	= ND;
		envQcApply			= FALSE;

		SCN_ROW ("poNumber")		=	4;
		SCN_ROW ("supplierNumber")	=	5;
		SCN_ROW ("supplierName")	=	5;
	
		strcpy (local_rec.OrderPrmpt,   "Qty To Retn.");
		strcpy (local_rec.ReceivePrmpt, (envVarThreePl) ? "Unit Quan."
														: "Qty Returned");
		strcpy (local_rec.PoPrmpt, 		"Purchase Return No.");
	}
	else
	{
		tab_col = 0;
		strcpy (local_rec.OrderPrmpt,   "Qty Ordered.");
		strcpy (local_rec.ReceivePrmpt, (envVarThreePl) ? "Unit Quan."
														: "This Receipt");
		strcpy (local_rec.PoPrmpt, 		"Purchase Order No.");
	}
	
	strcpy (local_rec.previousPO,	  	"000000000000000");
	strcpy (local_rec.previousCusOrdRef,"00000000000000000000");
	strcpy (local_rec.previousSupplier,	"000000");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		for (i = 0; i < MAXLINES; i++)
		{
			memset (store + i, 0, sizeof (struct storeRec));
			strcpy (store [i].err_found, "N");
			sprintf (store [i].itemDesc,"%-40.40s"," ");
			strcpy (store [i].serial_no , twentyfiveSpaces);
			strcpy (store [i].alloc_ser, twentyfiveSpaces);
			strcpy (store [i].dflt_ser, twentyfiveSpaces);
		}
		if (restart) 
		{
			if (SK_BATCH_CONT || MULT_LOC)
				AllocationRestore ();

			strcpy (local_rec.df_serial, twentyfiveSpaces);
			abc_unlock (pohr);
			abc_unlock (poln);
		}

		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		skip_tab	= FALSE;
		restart		= FALSE;
		plateCalled	= FALSE;
		lcount [DESC_SCN] 	= 0;
		init_vars (DESC_SCN);

		/*
		 * Enter screen 1 linear input. Turn screen initialise on.  
		 */
		init_ok = TRUE;
		heading (HEADER_SCN);
		entry (HEADER_SCN);
		if (prog_exit || restart)
			continue;
	
		scn_write (HEADER_SCN);
		scn_display (HEADER_SCN);
		if (!PO_RETURN)
		{
			scn_display (DESC_SCN);
			if (envSkGrinNoPlate)
				entry (DESC_SCN);
			else
				no_edit (DESC_SCN);
		}
		strcpy (local_rec.df_serial, "");
		strcpy (local_rec.currSerItem, "");
		strcpy (local_rec.defaultLotNo, "");
		if (strcmp (local_rec.headerLotNo, "       "))
			strcpy (local_rec.defaultLotNo, local_rec.headerLotNo);

		if (shipment)
			sprintf (err_str, ML (mlPoMess167));
		else
			sprintf (err_str,(PO_RETURN) ? ML (mlPoMess233) : ML (mlPoMess168));
		last_char = prmptmsg (err_str,"YyNn",1,2);
		move (1, 2); cl_line ();
		
		if (last_char == 'N' || last_char == 'n')
		{
			init_ok = FALSE;
			heading (ITEM_SCN);
			scn_display (ITEM_SCN);
			entry (ITEM_SCN);
			if (restart)
				continue;
		}
		else
			ConfirmAll ();

		if (PO_RETURN)
		{
			no_edit (HEADER_SCN);
			no_edit (DESC_SCN);
		}
		

		edit_all ();
		if (restart)
			continue;

		/*
		 * Check for blank Locations.
		 */
		while (!CheckOk ())
		{
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			edit_all ();
			if (restart)
				break;
		}

		if (restart)
			continue;

		Update ();

		if (SK_BATCH_CONT || MULT_LOC)
			AllocationComplete ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Confirm Complete order and allow user to Change.
 */
void
ConfirmAll (void)
{
	int		errFlag = FALSE;

	scn_set (ITEM_SCN);

	print_at (2,1,ML (mlPoMess175));

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++)
	{
		getval (line_cnt);

		local_rec.qtySupplied = (PO_RETURN) ? local_rec.qtyOrdered 
											: local_rec.actReceipt;
	
		if (local_rec.qtySupplied < 0.00)
			local_rec.qtySupplied = 0.00;

		if (SR.ser_item [0] == 'Y')
		{
			if (!strcmp (local_rec.serial,twentyfiveSpaces))
				strcpy (SR.err_found, "Y");
			else
				strcpy (SR.err_found, "N");
		}
		CalcExpiry ();
		InputExpiry = TRUE;
		
		strcpy (local_rec.LL, "Y");

		if (SR.hhplOrig && envSkGrinNoPlate)
		{
			cc = PoLoad
				(
					line_cnt,
					SR.hhplOrig,
					SR.hhccHash,
					SR.uom,
					SR.CnvFct,
					(envVarThreePl) ? local_rec.qtySupplied / SR.CnvFct
									: local_rec.qtySupplied
				);
			strcpy (local_rec.LL, "Y");
			LLInputClear = FALSE;
		}
		cc = DisplayLL
		(
			line_cnt,
			tab_row + 3,
			tab_col + 3,
			4,
			SR.hhwhHash,
			SR.hhumHash,
			SR.hhccHash,
			SR.uom,
			(envVarThreePl) ? local_rec.qtySupplied / SR.CnvFct 
							: local_rec.qtySupplied,
			SR.CnvFct,
			local_rec.expiryDate,
			TRUE,
			TRUE,
			(envVarThreePl && PO_RETURN) ? "Y" : SR.lot_ctrl
		);
		if (cc)
		{
			strcpy (local_rec.LL, "N");
			strcpy (SR.err_found, "Y");
		}
	
		if (check_class (SR._class))
			strcpy (SR.err_found, "N");

		if (envQcApply && local_rec.qcReqd [0] == 'Y')
		{
			strcpy (local_rec.qcCentre, "    ");
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			incc_rec.hhcc_hash = local_rec.hhccHash;
			if (find_rec (incc, &incc_rec, EQUAL, "r"))
				errFlag = TRUE;
			else
			{
				if (!strcmp (incc_rec.qc_centre, "    "))
					errFlag = TRUE;
				else
					strcpy (local_rec.qcCentre, incc_rec.qc_centre);
			}
			strcpy (SR.qcCentre, local_rec.qcCentre);
		}
		putval (line_cnt);
	}

	if (errFlag)
	{
		sprintf (err_str, ML (mlPoMess176), BELL);
		sleep (sleepTime);
		clear_mess ();
	}
	move (1, 2); cl_line ();
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files .
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	/*
	 * Read common terminal record.
	 */
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (pohr2, pohr);
	abc_alias (poln2, poln);
	abc_alias (posh2, posh);
	abc_alias (pogl2, pogl);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no3");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (insf,  insf_list, INSF_NO_FIELDS, "insf_id_no2");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pogh,  pogh_list, POGH_NO_FIELDS, "pogh_id_no2");
	open_rec (pogl,  pogl_list, POGL_NO_FIELDS, "pogl_id_no2");
	open_rec (pogl2, pogl_list, POGL_NO_FIELDS, "pogl_hhpl_hash");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
	open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (pohs,  pohs_list, POHS_NO_FIELDS, "pohs_id_no");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poln2, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posd,  posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (posh,  posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posh2, posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
	open_rec (posl,  posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (pons,  pons_list, PONS_NO_FIELDS, "pons_id_no");
	open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_id_no");
	open_rec (skcm,  skcm_list, SKCM_NO_FIELDS, "skcm_id_no");

	if (envSkGrinNoPlate)
	{
		open_rec (sknh, sknh_list, SKNH_NO_FIELDS, "sknh_id_no");
		open_rec (sknd, sknd_list, SKND_NO_FIELDS, "sknd_hhpl_hash");
		open_rec (skni, skni_list, SKNI_NO_FIELDS, "skni_sknd_hash");
	}
	if (envIkeaSystem)
		open_rec (poce, poce_list, POCE_NO_FIELDS, "poce_id_no");
	if (envPoSuHist)
		open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_id_no");
	if (envQcApply)
		open_rec (qcmr, qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	if (envVarThreePl)
    	open_rec (inuv,inuv_list, INUV_NO_FIELDS, "inuv_id_no");

	strcpy (ccmr_rec.co_no, 	comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, 	comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);

	IgnoreAvailChk	=	(PO_RETURN) ? FALSE : TRUE;
	LL_EditLoc	=	(MULT_LOC) 	 ? TRUE : FALSE; 

	if (envVarThreePl || envSkGrinNoPlate)
	{
		LL_EditLot	=	 FALSE;
		LL_EditDate	=	 FALSE;
		LL_EditSLot	=	 FALSE;
	}
	else
	{
		LL_EditLot	=	(SK_BATCH_CONT) ? TRUE : FALSE;
		LL_EditDate	=	(SK_BATCH_CONT) ? TRUE : FALSE;
		LL_EditSLot	=	(SK_BATCH_CONT) ? TRUE : FALSE;
	}
	if (PO_RETURN)
		strcpy (StockTake, "Y");
	
	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (insf);
	abc_fclose (inum);
	abc_fclose (pogl);
	abc_fclose (pogl2);
	abc_fclose (pogh);
	abc_fclose (pohr);
	abc_fclose (pohr2);
	abc_fclose (pohs);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (posd);
	abc_fclose (posh);
	abc_fclose (posh2);
	abc_fclose (posl);
	abc_fclose (pons);
	abc_fclose (trve);
	abc_fclose (skcm);

	if (envSkGrinNoPlate)
	{
		abc_fclose (sknh);
		abc_fclose (sknd);
		abc_fclose (skni);
	}
	if (envPoSuHist)
		abc_fclose (suph);
	if (envQcApply)
		abc_fclose (qcmr);
	if (envIkeaSystem)
		abc_fclose (poce);
	if (envVarThreePl)
		abc_fclose (inuv);

	CloseLocation ();

	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i;
	int		TempLine;
	char	*sptr;
	long	serHhbr;
	int		serStatus;
	int		firstSer;
	int		thisPage;
	char	orgSerNo [26],
			tmpSerNo [26];

	/*
	 * Validate Shipping Number. 
	 */
	if (LCHECK ("ship_no"))
	{
		shipment = FALSE;

		if (PO_RETURN)
			return (EXIT_SUCCESS);

		if (dflt_used || F_NOKEY (field))
		{
			sprintf (posh_rec.vessel,"%-20.20s"," ");
			strcpy (local_rec.containerNo, " ");
			display_field (field + 1);
			display_field (field + 2);
			if (!F_HIDE (label ("container")))
				FLD ("container")	=	NA;

			if (!F_HIDE (label ("caseNumber")))
				FLD ("container")	=	NE;

			FLD ("poNumber")		=	NE;

			return (EXIT_SUCCESS);
		}
		if (!F_HIDE (label ("container")))
			FLD ("container")	=	YES;

		if (!F_HIDE (label ("caseNumber")))
			FLD ("container")	=	NE;

		FLD ("poNumber")		=	NA;

		if (SRCH_KEY)
		{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (posh_rec.co_no,comm_rec.co_no);
		strcpy (posh_rec.csm_no, zero_pad (posh_rec.csm_no, 12));
		cc = find_rec (posh,&posh_rec,EQUAL,"w");
		if (cc)
		{
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (posh);
			return (EXIT_FAILURE);
		}

		if (posh_rec.status [0] == 'C')
		{
			print_mess (ML (mlPoMess200));
			abc_unlock (posh);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (posh_rec.status [0] == 'D')
		{
			print_mess (ML (mlPoMess201));
			abc_unlock (posh);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!CASE_USED && !envVarPoContainer)
		{
			LoadPosl (posh_rec.hhsh_hash);
			scn_set (HEADER_SCN);

			if (lcount [ITEM_SCN] <= 0) 
			{
				print_mess (ML (mlPoMess202));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (posh);
				return (EXIT_FAILURE);
			}
		}

		sprintf (local_rec.supplierNumber,"%-6.6s"," ");
		sprintf (sumr_rec.crd_name,"%-40.40s"," ");
		sprintf (pohr_rec.pur_ord_no,"%-15.15s"," ");

		DSP_FLD ("vessel");
		DSP_FLD ("poNumber");
		DSP_FLD ("supplierNumber");
		DSP_FLD ("supplierName");
		shipment = TRUE;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Purchase Order Number.
	 */
	if (LCHECK ("poNumber"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);
		
		if (PO_RETURN && last_char == FN16)
		{
			prog_exit = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Check if order is on file.
		 */
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no,15));

		cc = find_rec (pohr,&pohr_rec,EQUAL,"r");
		if (cc)
		{
			print_mess ((PO_RETURN) ? ML (mlStdMess282) : ML (mlStdMess048));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (pohr_rec.drop_ship [0] == 'Y')
		{
			print_mess (ML (mlPoMess203));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}
	
		if (pohr_rec.status [0] == 'U')
		{
			print_mess (ML (mlPoMess204));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}
		if (pohr_rec.status [0] != 'O' && 
		     pohr_rec.status [0] != 'R' &&
		     pohr_rec.status [0] != 'r' &&
		     pohr_rec.status [0] != 'P' &&
		     pohr_rec.status [0] != 'C' &&
		     pohr_rec.status [0] != 'c')
		{
			print_mess (ML (mlPoMess205));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (sumr);
			return (EXIT_FAILURE);
		}
		abc_fclose (sumr);
		strcpy (local_rec.supplierNumber, sumr_rec.crd_no);

		if (!CASE_USED)
		{
			LoadPoln (pohr_rec.hhpo_hash);
			scn_set (HEADER_SCN);

			if (lcount [ITEM_SCN] <= 0) 
			{
				print_mess (ML (mlPoMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		DSP_FLD ("supplierNumber");
		DSP_FLD ("supplierName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("caseNumber"))
	{
		if (F_HIDE (label ("caseNumber")))
			return (EXIT_SUCCESS);

		if (shipment)
		{
			LoadPosl (posh_rec.hhsh_hash);
			scn_set (HEADER_SCN);

			if (lcount [ITEM_SCN] <= 0) 
			{
				print_mess (ML (mlPoMess202));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (posh);
				skip_entry = -4;
				return (EXIT_SUCCESS);
			}
		}
		else
		{
			LoadPoln (pohr_rec.hhpo_hash);
			scn_set (HEADER_SCN);
	
			if (lcount [ITEM_SCN] <= 0) 
			{
				print_mess (ML (mlPoMess004));
				sleep (sleepTime);
				clear_mess ();
				skip_entry = -2;
				return (EXIT_SUCCESS);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Goods Receipt Number. 
	 */
	if (LCHECK ("grinNumber"))
	{
		genNewGrnNo = FALSE;
		if (SRCH_KEY)
		{
			SrchPogh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || FLD ("grinNumber") == ND)
		{
			genNewGrnNo = TRUE;
			newPogh = TRUE;
			strcpy (pogh_rec.gr_no, "NEW GRIN NUMBER");
			DSP_FLD ("grinNumber");
			return (EXIT_SUCCESS);
		}

		strcpy (err_str,pogh_rec.gr_no);
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

		strcpy (pogh_rec.co_no,comm_rec.co_no);
		cc = find_rec (pogh,&pogh_rec,EQUAL,"w");
		if (cc)
		{
			newPogh = TRUE;
		}
		else
		{
			newPogh = FALSE;
			if (pogh_rec.hhsu_hash != sumr_rec.hhsu_hash &&
			     shipment == FALSE)
			{
				print_mess (ML (mlPoMess177));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pogh);
				return (EXIT_FAILURE);
			}

			if ( pogh_rec.pur_status [0] != 'R' && 
			     pogh_rec.pur_status [0] != 'U' &&
			     pogh_rec.pur_status [0] != 'P')
			{
				print_mess (ML (mlPoMess178));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pogh);
				return (EXIT_FAILURE);
			}
			if (shipment && pogh_rec.hhsh_hash == 0)
			{
				print_mess (ML (mlPoMess179));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pogh);
				return (EXIT_FAILURE);
			}
			if (!shipment && pogh_rec.hhsh_hash != 0)
			{
				print_mess (ML (mlPoMess180));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pogh);
				return (EXIT_FAILURE);
			}
			if (shipment)
				return (EXIT_SUCCESS);

			pohr2_rec.hhpo_hash	=	pogh_rec.hhpo_hash;
			cc = find_rec (pohr2, &pohr2_rec, EQUAL, "r");
			if (!cc && strcmp (pohr_rec.stat_flag, pohr2_rec.stat_flag))
			{
				print_mess (ML (mlPoMess181));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pogh);
				return (EXIT_FAILURE);
			}
			if (envSkGrinNoPlate && !PO_RETURN)
				LoadNoPlateNotes (cur_screen);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("headerLotNo"))
	{
		if (FLD ("headerLotNo") == ND)
			return (EXIT_SUCCESS);

		scn_set (ITEM_SCN);
		for (i = 0; i < lcount [ITEM_SCN]; i++)
		{
			getval (i);

			if (store [i].lot_ctrl [0] == 'Y' &&
				strcmp (GetLotNo (i, 0), "       "))
			{
				long	workInloHash	=	0L;
				cc = FindLotNo 
					(
						store [i].hhwhHash,
						store [i].hhumHash,
						local_rec.headerLotNo,
						&workInloHash
					);
				if (cc)
					strcpy (DfltLotNo, local_rec.headerLotNo);
				else
				{
					if (prog_status == ENTRY)
						strcpy (DfltLotNo, local_rec.headerLotNo);
					else
					{
						sprintf (err_str, ML (mlPoMess169),local_rec.headerLotNo, inmr_rec.item_no);
						cc = prmptmsg (err_str, "YyNn", 1, 23);
						move (0, 23); cl_line ();
						if (cc == 'Y' || cc == 'y')
							strcpy (DfltLotNo, local_rec.headerLotNo);
						else
							strcpy (DfltLotNo, "       ");
					}
				}
			}
		}
		scn_set (HEADER_SCN);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Container Number.
	 */
	if (LCHECK ("container"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);
			
		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (EXIT_SUCCESS);
		}
		if (IsSpaces (local_rec.containerNo))
		{
			LoadPosl (posh_rec.hhsh_hash);
			scn_set (HEADER_SCN);
			if (lcount [ITEM_SCN] <= 0) 
			{
				print_mess (ML (mlPoMess202));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (posh);
				skip_entry = goto_field (field, label ("ship_no"));
				return (EXIT_SUCCESS);
			}
			return (EXIT_SUCCESS);
		}
		
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		strcpy (skcm_rec.container, local_rec.containerNo);
		cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTrMess083));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		LoadPosl (posh_rec.hhsh_hash);
		scn_set (HEADER_SCN);

		if (lcount [ITEM_SCN] <= 0) 
		{
			print_mess (ML (mlPoMess202));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (posh);
			skip_entry = goto_field (field, label ("ship_no"));
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("item_no"))
	{
		if (INPUT)
			getval (line_cnt);

		tab_other (line_cnt);
	}

	if (LCHECK ("qty_sup"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (local_rec.qtySupplied > local_rec.actReceipt)
		{
			if (PO_RETURN)
				sprintf (err_str,ML (mlPoMess232),local_rec.qtyOrdered,local_rec.qtySupplied);
			else
				sprintf (err_str,ML (mlPoMess182),local_rec.qtyOrdered,local_rec.qtySupplied);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
		}

		if (dflt_used)
		{
			local_rec.qtySupplied = local_rec.actReceipt;

			if (local_rec.qtySupplied < 0.00)
				local_rec.qtySupplied = 0.00;
		}
		if (INPUT && local_rec.qtySupplied == 0.00)
		{
			skip_entry = goto_field (field, label ("ser_no")) + 1;
			return (EXIT_SUCCESS);
		}

		/*
		 * Check to see if serial item has only 1 or 0 as quantity.
		 */
		if (SERIAL && local_rec.qtySupplied != 0.00 && 
		     local_rec.qtySupplied != 1.00)
		{
			print_mess (ML (mlStdMess029));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (INPUT && (SK_BATCH_CONT || MULT_LOC) && !PO_RETURN)
		{
		    if (local_rec.qtySupplied < local_rec.actReceipt)
		    {
				i = prmptmsg (ML (mlPoMess170), "YyNn", 1, 23);
				move (0, 23); cl_line ();
				if (i == 'Y' || i == 'y')
			    	InsertNewLine ();
				else
			    	CheckDeleteLine ();
		    }
		    else
				CheckDeleteLine ();
		}

		if (!INPUT && SERIAL)
		{
			if (local_rec.qtySupplied == 0.00)
			{
				if (strcmp (local_rec.serial, twentyfiveSpaces))
				{
					strcpy (local_rec.serial, twentyfiveSpaces);
					strcpy (SR.serial_no,     twentyfiveSpaces);
					strcpy (SR.dflt_ser,     twentyfiveSpaces);
					DSP_FLD ("ser_no");
				}
				return (EXIT_SUCCESS);
			}
			else
			{
				if ((SK_BATCH_CONT || MULT_LOC))
				{
					i = label ("LL");
					do
					{
						get_entry (i);
						cc = spec_valid (i);
						strcpy (local_rec.LL, "N");
					}
					while (cc && !restart);

					if (restart)
						return (EXIT_SUCCESS);

					DSP_FLD ("LL");
				}

				if (!strcmp (local_rec.serial, twentyfiveSpaces))
				{
					i = label ("ser_no");
					do
					{
						get_entry (i);
						cc = spec_valid (i);
					}
					while (cc && !restart);

					if (restart)
						return (EXIT_SUCCESS);

					DSP_FLD ("ser_no");
				}
				return (EXIT_SUCCESS);
			}
		}
		if (prog_status != ENTRY)
		{
			/*
			 * Reenter Location.
			 */
			do
			{
				strcpy (local_rec.LL, "N");
				get_entry (label ("LL"));
				if (restart)
					break;
				cc = spec_valid (label ("LL"));
			} while (cc && !F_HIDE (label ("LL")));
		}
		if (SERIAL)
		{
			if (INPUT && strcmp (local_rec.serial,twentyfiveSpaces))
				skip_entry = goto_field (field, label ("ser_no")) + 1;
		}
		
		DSP_FLD ("qty_sup");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("packageQty"))
	{
		if (F_NOKEY (label ("packageQty")))
			return (EXIT_SUCCESS);
		
		if (dflt_used)
			local_rec.packageQty = local_rec.qtySupplied / local_rec.CnvFct;

		DSP_FLD ("packageQty");
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("vehicle"))
	{
		if (F_NOKEY (label ("vehicle")))
			return (EXIT_SUCCESS);

		if (dflt_used) 
		{
			strcpy (trve_rec.ref, " ");
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
			/*
			 * Vehicle not found.
			 */
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
	/*
	 * Validate serial number input.
	 */
	if (LCHECK ("ser_no"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (F_HIDE (label ("ser_no")))
			return (EXIT_SUCCESS);

		if (SR.ser_item [0] != 'Y')
			return (EXIT_SUCCESS);

		if (local_rec.qtySupplied == 0.00)
		{
			print_mess (ML (mlPoMess208));
			sleep (sleepTime);
			clear_mess ();
			strcpy (local_rec.serial, twentyfiveSpaces);
			strcpy (SR.serial_no, local_rec.serial);
			strcpy (SR.dflt_ser,  local_rec.serial);
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}

		if (strcmp (SR.alloc_ser, twentyfiveSpaces))
		{
			clear_mess ();
			sprintf (err_str,ML (mlStdMess097),clip (SR.serial_no));

			rv_pr (err_str, 0,23,1);
			sleep (sleepTime);
			strcpy (local_rec.serial, SR.alloc_ser);
			strcpy (SR.serial_no,  SR.alloc_ser);
			strcpy (SR.dflt_ser,   SR.alloc_ser);
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}

		/*
		 * Default used.
		 */
		if (dflt_used)
		{
			/*
			 * Default serial number must be non-twentyfiveSpaces 
			 * and must be the same item no as last one 
			 * that had a serial number input.         
			 */
			if (strcmp (local_rec.df_serial, ""))
			{
				if (!strcmp (local_rec.currSerItem, inmr_rec.item_no) &&
					prog_status == ENTRY)
				{
					serStatus = GenNextSerNo (local_rec.df_serial,
										  	  TRUE,
										  	  tmpSerNo);
					if (serStatus != 0)
					{
						print_mess (ML (mlPoMess209));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
					sprintf (local_rec.serial, "%-25.25s", tmpSerNo);
				}
				else
					strcpy (local_rec.serial, SR.dflt_ser);
			}
		}

		if (!strcmp (local_rec.serial, twentyfiveSpaces))
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.df_serial, local_rec.serial);

		if (CheckDupSerial (local_rec.serial, SR.hhwhHash, line_cnt))
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		insf_rec.hhwh_hash = SR.hhwhHash;
		strcpy (insf_rec.serial_no,local_rec.serial);
		cc = find_rec (insf,&insf_rec,COMPARISON,"r");
		if (!cc)
		{
			print_mess (ML (mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (SR.serial_no, local_rec.serial);
		strcpy (SR.dflt_ser,  local_rec.serial);
		strcpy (SR.err_found, "N");

		strcpy (local_rec.currSerItem, inmr_rec.item_no);

		/*
		 * Generate serial numbers for all lines with this item no
	 	 * after the current line.   
		 */
		if (prog_status != ENTRY)
		{
			int		stopGen;
			int		tmpLineCnt;
			int		serNoUsed;

			tmpLineCnt = line_cnt;
			thisPage = line_cnt / TABLINES;
			strcpy (orgSerNo, local_rec.serial);
			putval (line_cnt);
			serHhbr = SR.hhbrHash;
			firstSer = TRUE;

			for (i = (line_cnt + 1); i < lcount [ITEM_SCN]; i++)
			{
				getval (i);
				if (store [i].ser_item [0] == 'N' ||
					(store [i].ser_item [0] == 'Y' && 
					 serHhbr != store [i].hhbrHash) ||
					local_rec.qtySupplied == 0.00)
				{
					continue;
				}

				/*
				 * Generate next serial number.
				 */
				do
				{
					serStatus = GenNextSerNo (orgSerNo, firstSer, tmpSerNo);
					switch (serStatus)
					{
					case 1:
						print_mess (ML (mlPoMess207));
						sleep (sleepTime);
						clear_mess ();
						stopGen = TRUE;
						break;
	
					case 2:
						sprintf (err_str, ML (mlPoMess183), i + 1);
						print_mess (err_str);
						sleep (sleepTime);
						clear_mess ();
						stopGen = TRUE;
						break;
	
					default:
						stopGen = FALSE;
						break;
					}
					firstSer = FALSE;

					sprintf (local_rec.serial, "%-25.25s", tmpSerNo);
	
					if (stopGen)
						break;
	
					/*
					 * Check validity of serial number.
					 */
					if (CheckDupSerial (local_rec.serial, store [i].hhwhHash,i))
					{
						serNoUsed = TRUE;
						continue;
					}
	
					insf_rec.hhwh_hash = store [i].hhwhHash;
					strcpy (insf_rec.serial_no, local_rec.serial);
					cc = find_rec (insf, &insf_rec, COMPARISON, "r");
					if (!cc)
					{
						serNoUsed = TRUE;
						continue;
					}
	
					serNoUsed = FALSE;
				} while (serNoUsed);
		
				if (stopGen)
					break;

				strcpy (store [i].serial_no, local_rec.serial);
				strcpy (store [i].dflt_ser,  local_rec.serial);
				strcpy (store [i].err_found, "N");

				/*
				 * Store line.
				 */
				putval (i);

				/*
				 * Display line.
				 */
				if (thisPage == i / TABLINES)
				{
					line_cnt = i;
					line_display ();
				}
			}

			line_cnt = tmpLineCnt;
			getval (line_cnt);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate QC Centre input.
	 */
	if (LCHECK ("qcCentre"))
	{
		if (!envQcApply ||
			F_NOKEY (label ("qcCentre")))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			/* Read default from incc record. */
			incc_rec.hhcc_hash = local_rec.hhccHash;
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			if (find_rec (incc, &incc_rec, EQUAL, "r"))
				return (EXIT_FAILURE);

			if (!strcmp (incc_rec.qc_centre, "    "))
				return (EXIT_FAILURE);
			
			strcpy (local_rec.qcCentre, incc_rec.qc_centre);
			strcpy (SR.qcCentre, local_rec.qcCentre);
			DSP_FLD ("qcCentre");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

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

		strcpy (SR.qcCentre, local_rec.qcCentre);
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate lots and locations.
	 */
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND || FLD ("LL") == NA)
			return (EXIT_SUCCESS);

		if (!PO_RETURN)
			CalcExpiry ();

		if (SR.hhplOrig && envSkGrinNoPlate && PO_RETURN)
		{
			cc = PoLoad
				(
					line_cnt,
					SR.hhplOrig,
					SR.hhccHash,
					SR.uom,
					SR.CnvFct,
					(envVarThreePl) ? local_rec.qtySupplied / SR.CnvFct
									: local_rec.qtySupplied
				);
			strcpy (local_rec.LL, "Y");
			LLInputClear = FALSE;
		}
		TempLine	= lcount [ITEM_SCN];
		InputExpiry = (PO_RETURN) ? FALSE : TRUE;
		cc = DisplayLL
			(
				line_cnt,
				tab_row + 3,
				tab_col + 3,
				4,
				SR.hhwhHash,
				SR.hhumHash,
				SR.hhccHash,
				SR.uom,
				(envVarThreePl) ? local_rec.qtySupplied / SR.CnvFct
								: local_rec.qtySupplied,
				SR.CnvFct,
				local_rec.expiryDate,
				FALSE,
				(PO_RETURN) ? (local_rec.LL [0] == 'Y') : TRUE,
				SR.lot_ctrl
			);
		/*
		 * Redraw screens.
		 */
		putval (line_cnt);

		lcount [ITEM_SCN] = (line_cnt + 1 > lcount [ITEM_SCN]) ? line_cnt + 1 : lcount [ITEM_SCN];
		scn_write (ITEM_SCN);
		scn_display (ITEM_SCN);
		lcount [ITEM_SCN] = TempLine;
		PrintCoStuff ();
		if (cc)
		{
			strcpy (local_rec.LL, "N");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.LL, "Y");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("totalCBM") && envVarThreePl)
	{
		if (local_rec.totalCBM <= 0.00)
			return (EXIT_SUCCESS);
		
		inuv_rec.hhbr_hash  =   SR.hhbrHash;
		inuv_rec.hhum_hash  =   SR.hhumHash;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Invalid Unit Measure
			 */
			abc_unlock (inuv);
			print_mess (ML (mlStdMess028)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inuv_rec.volume = 	local_rec.totalCBM / local_rec.qtySupplied;
		cc = abc_update (inuv, &inuv_rec);
		if (cc)
			file_err (cc, inuv, "DBUPDATE");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("totalGrossWgt") && envVarThreePl)
	{
		if (local_rec.totalGrossWgt <= 0.00)
			return (EXIT_SUCCESS);
		
		inuv_rec.hhbr_hash  =   SR.hhbrHash;
		inuv_rec.hhum_hash  =   SR.hhumHash;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Invalid Unit Measure
			 */
			abc_unlock (inuv);
			print_mess (ML (mlStdMess028)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inuv_rec.weight = 	local_rec.totalGrossWgt / local_rec.qtySupplied;

		cc = abc_update (inuv, &inuv_rec);
		if (cc)
			file_err (cc, inuv, "DBUPDATE");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
CheckDeleteLine (void)
{
	int		cur_line = line_cnt,
			do_delete = FALSE,
			i;
	long	tmp_hhpl;

	putval (line_cnt);
	tmp_hhpl = poln_rec.hhpl_hash;

	if (line_cnt + 1 < lcount [ITEM_SCN])
	{
		getval (line_cnt + 1);
		if (poln_rec.hhpl_hash == tmp_hhpl)
			do_delete = TRUE;
	}

	if (do_delete)
	{
		for (i = line_cnt + 2; i < lcount [ITEM_SCN]; i++)
		{
			getval (i);
			putval (i - 1);

			memcpy 
			(
				(char *) &store [i - 1], 
				(char *) &store [i],
			   	sizeof (struct storeRec)
			);
		}
		lcount [ITEM_SCN]--;
		vars [scn_start].row = lcount [ITEM_SCN];

		i = line_cnt % TABLINES;
		line_cnt -= i;
		for (i = 0; i < TABLINES; i++, line_cnt++)
		{
			getval (line_cnt);
			if (line_cnt < lcount [ITEM_SCN])
				line_display ();
			else
				blank_display ();
		}
	}
	line_cnt = cur_line;
	getval (line_cnt);
}

void
InsertNewLine (void)
{
	int		do_insert = TRUE,
			i;
	long	tmp_hhpl;

	putval (line_cnt);
	tmp_hhpl = poln_rec.hhpl_hash;

	if (line_cnt + 1 < lcount [ITEM_SCN])
	{
		getval (line_cnt + 1);
		if (poln_rec.hhpl_hash == tmp_hhpl)
			do_insert = FALSE;
	}

	if (do_insert)
	{
		for (i = lcount [ITEM_SCN]; i > line_cnt; i--)
		{
			getval (i);
			putval (i + 1);

			memcpy 
			(
				(char *) &store [i + 1], 
				(char *) &store [i],
			   	sizeof (struct storeRec)
			);
		}
		lcount [ITEM_SCN]++;
		vars [scn_start].row = lcount [ITEM_SCN];
	}
	getval (line_cnt);
	local_rec.actReceipt = local_rec.actReceipt - local_rec.qtySupplied;
	local_rec.qtyReceived += local_rec.qtySupplied;
	local_rec.qtySupplied = local_rec.actReceipt;
	putval (line_cnt + 1);

	memcpy 
	(
		(char *) &store [line_cnt + 1], 
		(char *) &store [line_cnt],
		sizeof (struct storeRec)
	);
	getval (line_cnt);
	scn_display (ITEM_SCN);
}

void
LoadLines (
	float	_qtyOrdered,
	float	_qty_rec,
	float	_act_rec)
{
	int		tmp_dmy [3];
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00;

	if (IntFindInmr (poln_rec.hhbr_hash))
		return;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	local_rec.CnvFct	=	PurCnvFct / StdCnvFct;
	SRL.CnvFct 			= 	local_rec.CnvFct;
	strcpy (local_rec.uom, inum_rec.uom);
	strcpy (SRL.uom, inum_rec.uom);
	
	/*
	 * Find Inventory Supplier Record for Suppliers Part.
	 */
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
		strcpy (inis_rec.sup_part, "                ");

	if (envVarThreePl)
	{
		local_rec.qtyOrdered 	= _qtyOrdered;
		local_rec.qtyReceived 	= _qty_rec;
		local_rec.actReceipt = _act_rec;
		local_rec.qtySupplied 	= _act_rec;
		local_rec.packageQty 	= _act_rec / local_rec.CnvFct;
	}
	else
	{
		local_rec.qtyOrdered 	= _qtyOrdered / local_rec.CnvFct;
		local_rec.qtyReceived 	= _qty_rec / local_rec.CnvFct;
		local_rec.actReceipt 	= _act_rec / local_rec.CnvFct;
		local_rec.qtySupplied 	= _act_rec / local_rec.CnvFct;
		local_rec.packageQty 	= 0.00;
	}
	local_rec.landCost 		= poln_rec.land_cst;
	SRL.landCost 			= local_rec.landCost;
	SRL.outer 				= (double) (inmr_rec.outer_size);

	/*
	 * Store non-screen details for later.
	 */
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, EQUAL, "r");
	if (cc)
	{
		sprintf (err_str, "Error in incc %ld / %ld During (DBFIND)",
				poln_rec.hhcc_hash,
				poln_rec.hhbr_hash);

		sys_err (err_str, cc, PNAME);
	}
	local_rec.packageQty		= poln_rec.pack_qty;
	local_rec.totalChargeWgt	= poln_rec.chg_wgt;
	local_rec.totalGrossWgt 	= poln_rec.gross_wgt;
	local_rec.totalCBM			= poln_rec.cu_metre;
	local_rec.hhbrHash 			= poln_rec.hhbr_hash;
	local_rec.hhccHash 			= poln_rec.hhcc_hash;
	SRL.hhbrHash 				= poln_rec.hhbr_hash;
	SRL.hhccHash 				= poln_rec.hhcc_hash;
	SRL.hhumHash 				= poln_rec.hhum_hash;
	SRL.hhwhHash 				= incc_rec.hhwh_hash;
	SRL.hhplHash 				= poln_rec.hhpl_hash;
	SRL.hhplOrig 				= poln_rec.hhpl_orig;

	strcpy (local_rec.cusOrdRef, poln_rec.cus_ord_ref);
	strcpy (SRL.cost_type, 		inmr_rec.costing_flag);
	strcpy (local_rec.lot_ctrl, inmr_rec.lot_ctrl);
	strcpy (SRL.lot_ctrl,  		inmr_rec.lot_ctrl);
	strcpy (SRL.ser_item,  		inmr_rec.serial_item);
	strcpy (SRL.serial_no, 		poln_rec.serial_no);
	strcpy (SRL.alloc_ser, 		poln_rec.serial_no);
	strcpy (local_rec.serial,   poln_rec.serial_no);
	
	strcpy (SRL.sup_part,  	inis_rec.sup_part);
	strcpy (SRL.itemDesc, 	inmr_rec.description);
	strcpy (SRL._class, 		inmr_rec.inmr_class);
	if (check_class (inmr_rec.inmr_class))
	{
		if (FindPons (poln_rec.hhpl_hash))
			sprintf (SRL.itemDesc, "%40.40s", " ");
		else
			strcpy (SRL.itemDesc, pons_rec.desc);
	}

	ccmr_rec.hhcc_hash		=	poln_rec.hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	strcpy (SRL.br_no,   (cc) ? comm_rec.est_no   : ccmr_rec.est_no);
	strcpy (SRL.wh_no,   (cc) ? comm_rec.cc_no 	  : ccmr_rec.cc_no);
	strcpy (SRL.acronym, (cc) ? comm_rec.cc_short : ccmr_rec.acronym);

	strcpy (local_rec.qcReqd,	inmr_rec.qc_reqd);
	strcpy (local_rec.qcCentre,	incc_rec.qc_centre);
	strcpy (SRL.qcReqd,			inmr_rec.qc_reqd);
	strcpy (SRL.qcCentre,		incc_rec.qc_centre);

    local_rec.expiryDate = 0L;
    if (inmr_rec.lot_ctrl [0] == 'Y')
    {
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, SRL.br_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc) 
		{
			clear_mess ();
			print_mess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return;
		}
		DateToDMY (local_rec.lsysDate,&tmp_dmy [0],&tmp_dmy [1],&tmp_dmy [2]);
		tmp_dmy [1]--;
		tmp_dmy [1] += inei_rec.expiry_prd1;
		tmp_dmy [2] += (tmp_dmy [1] / 12);
		tmp_dmy [1] %= 12;
		tmp_dmy [1]++;
		do
		{
			local_rec.expiryDate = DMYToDate (tmp_dmy [0],tmp_dmy [1],tmp_dmy [2]);
			tmp_dmy [0]--;
		} while (local_rec.expiryDate == -1);
    }
	SRL.exp_date = local_rec.expiryDate;

	/* 
	 * Load locations from number plate links.
	 */
	if (SRL.hhplOrig > 0L && envSkGrinNoPlate)
	{
		cc = PoLoad
			(
				line_cnt,
				SRL.hhplOrig,
				SRL.hhccHash,
				SRL.uom,
				SRL.CnvFct,
				_act_rec
			);
		LLInputClear = FALSE;
		strcpy (local_rec.LL, "Y");
	}
	else
	{
		LLInputClear = TRUE;
		strcpy (local_rec.LL, "N");
	}

	putval (lcount [ITEM_SCN]++);

	return;
}

void
LoadPosl  (
	long	hhshHash)
{
	int	i = 0;

	print_at (2,1,ML (mlStdMess035));

	abc_selfield (pohr, "pohr_hhpo_hash");
	abc_selfield (poln, "poln_hhpl_hash");

	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;
	vars [scn_start].row = 0;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&shipment_d, &ship, sizeof (struct Shipment), 1000);
	shipCnt = 0;

	strcpy (posl_rec.co_no, comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpl_hash = 0L;
	for (cc = find_rec (posl, &posl_rec, GTEQ, "r");
		 !cc && !strcmp (posl_rec.co_no, comm_rec.co_no) && 
		  posl_rec.hhsh_hash == hhshHash;
		 cc = find_rec (posl, &posl_rec, NEXT, "r"))
	{
		/*
		 * Quantity already shipped.
		 */
		if (posl_rec.ship_qty <= 0.00)
			continue;

		/*
		 * Case number included and not sale as input case.
		 */
	    if (local_rec.caseNumber > 0 && 
			 posl_rec.case_no != local_rec.caseNumber)
			continue;

		/*
		 * Container number included and not same as input container.
		 */
		if (!IsSpaces (local_rec.containerNo) && 
			strcmp (posl_rec.container, local_rec.containerNo))
			continue;

		/*
		 * Purchase order line does not belong to shipment. 
		 */
		poln_rec.hhpl_hash	=	posl_rec.hhpl_hash;
		if ((cc = find_rec (poln, &poln_rec, EQUAL, "r")))
			continue;

		/*
		 * Purchase order header could not be found.
		 */
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		if ((cc = find_rec (pohr, &pohr_rec, EQUAL, "r")))
			continue;

		/*
		 * Check the array size before adding new element.
	     */
		if (!ArrChkLimit (&shipment_d, ship, shipCnt))
			sys_err ("ArrChkLimit (ship)", ENOMEM, PNAME);

		/*
		 * Load values into array element shipCnt.
		 */
		sprintf (ship [shipCnt].polnItemDesc, "%-40.40s", poln_rec.item_desc);
		ship [shipCnt]. polnHash = poln_rec.hhpl_hash;
		ship [shipCnt]. poslHash = posl_rec.hhpl_hash;
		ship [shipCnt]. pohrHash = pohr_rec.hhpo_hash;

		/*
		 * Increment array counter.
	   	 */
		shipCnt++;
	}

	/*
	 * Sort the array in item description order.
	 */
	if (envPoRecSortDes)
    	qsort (ship, shipCnt, sizeof (struct Shipment), DescriptionSort);

	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (i = 0; i < shipCnt; i++)
	{
		pohr_rec.hhpo_hash	=	ship [i].pohrHash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)pohr, "DBFIND");

		poln_rec.hhpl_hash	=	ship [i].polnHash;
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)poln, "DBFIND");
	
		strcpy (posl_rec.co_no, comm_rec.co_no);
		posl_rec.hhsh_hash	= 	hhshHash;
		posl_rec.hhpl_hash	=	ship [i].poslHash, 
		cc = find_rec (posl, &posl_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)posl, "DBFIND");
		sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;

		LoadLines (poln_rec.qty_ord,poln_rec.qty_rec,posl_rec.ship_qty);
	}
	move (1,2); cl_line ();
	vars [scn_start].row = lcount [ITEM_SCN];
	
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&shipment_d);

	abc_selfield (pohr, "pohr_id_no2");
	return;
}

void
LoadPoln (
	long	hhpoHash)
{
	int	i = 0;

	print_at (2,1,ML (mlStdMess035));

	abc_selfield (poln,"poln_id_no");
	scn_set (ITEM_SCN);
	lcount [ITEM_SCN] = 0;
	vars [scn_start].row = 0;

	/*
	 * Array the initial array
	 */
	ArrAlloc (&shipment_d, &ship, sizeof (struct Shipment), 1000);
	shipCnt = 0;

	strcpy (posl_rec.co_no, comm_rec.co_no);
	poln_rec.hhpo_hash = hhpoHash;
	poln_rec.line_no = 0;
	for (cc = find_rec (poln, &poln_rec, GTEQ, "r");
		 !cc && poln_rec.hhpo_hash == hhpoHash;
		 cc = find_rec (poln, &poln_rec, NEXT, "r"))
	{
		if (local_rec.caseNumber > 0 && 
			poln_rec.case_no != local_rec.caseNumber)
			continue;

		if (poln_rec.ship_no > 0L)
			continue;

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&shipment_d, ship, shipCnt))
			sys_err ("ArrChkLimit (ship)", ENOMEM, PNAME);

		/*
		 * Load values into array element shipCnt.
		 */
		sprintf (ship [shipCnt].polnItemDesc, "%-40.40s", poln_rec.item_desc);
		ship [shipCnt]. polnHash = poln_rec.hhpl_hash;

		/*
		 * Increment array counter.
		 */
		shipCnt++;
	}
	/*
	 * Sort the array in item description order
	 */
	if (envPoRecSortDes)
		qsort (ship, shipCnt, sizeof (struct Shipment), DescriptionSort);

	/*
	 * Change the index on the poln
	 */
	abc_selfield (poln, "poln_hhpl_hash");

	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (i = 0; i < shipCnt; i++)
	{
		poln_rec.hhpl_hash	=	ship [i].polnHash;
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (cc)
			file_err (cc, poln, "DBFIND");

		if (pohr_rec.type [0] == 'C' && poln_rec.pur_status [0] == 'O')
		{
			LoadLines (poln_rec.qty_ord, 0.00, poln_rec.qty_rec);
		}
		else
		{
			if (poln_rec.qty_ord - poln_rec.qty_rec > 0.00)
			{
				LoadLines 
				(
					poln_rec.qty_ord, 
					poln_rec.qty_rec,
				   	poln_rec.qty_ord - poln_rec.qty_rec
				);
			}
	   }
	}

	/*
	 * Change the poln index back
	 */
	abc_selfield (poln, "poln_id_no");

	/*
	 * Free up the array memory
	 */
	ArrDelete (&shipment_d);

	move (1,2); cl_line ();
	vars [scn_start].row = lcount [ITEM_SCN];
	return;
}

int 
DescriptionSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct Shipment a = * (const struct Shipment *) a1;
	const struct Shipment b = * (const struct Shipment *) b1;

	result = strcmp (a.polnItemDesc, b.polnItemDesc);

	return (result);
}

int
IntFindInmr (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, EQUAL,"r");
	if (cc)
		return (cc);

	abc_selfield (inmr,"inmr_id_no");
	cc = IntFindSupercession (inmr_rec.item_no);
	abc_selfield (inmr,"inmr_hhbr_hash");
	return (cc);
}

int
IntFindSupercession (
	char	*item_no)
{
	if (strcmp (inmr_rec.supercession,"                ") == 0)
		return (EXIT_SUCCESS);

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",item_no);
	cc = find_rec (inmr,&inmr_rec,EQUAL,"r");
	if (!cc)
		cc = IntFindSupercession (inmr_rec.supercession);
	return (cc);
}
/*
 * Update all files.
 */
void
Update (void)
{
	int		new_pogl;
	char	printerString [3];
	char	hhsh_str [10];
	long	hhpoHash = 0L;
	int		i;

	PageNumUpdate	=	0;

	clear ();

	if (lcount [ITEM_SCN] == 0)
		return;

	if (shipment)
	{
		strcpy (posh_rec.status,"R");
		cc = abc_update (posh,&posh_rec);
		if (cc)
			file_err (cc, posh, "DBUPDATE");
	}

	strcpy (pogh_rec.pur_status,"R");
	strcpy (pogh_rec.gl_status, "R");
	pogh_rec.exch_rate = (shipment) 
				? posh_rec.ex_rate : pohr_rec.curr_rate;
	if (newPogh)
	{
		strcpy (pogh_rec.co_no,comm_rec.co_no);
		strcpy (pogh_rec.br_no,comm_rec.est_no);
		pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
		pogh_rec.hhsh_hash = (shipment) ? posh_rec.hhsh_hash : 0L;
		pogh_rec.hhpo_hash = (shipment) ? 0L : pohr_rec.hhpo_hash;
		if (genNewGrnNo)
			GenGrnNo ();

		print_at (PageNumUpdate++,0,ML (mlPoMess184),pogh_rec.gr_no);
		sleep (sleepTime);

		strcpy (pogh_rec.rec_by, (shipment) ? "S" : "P");
		strcpy (pogh_rec.pur_ord_no,(shipment) ? fifteenSpaces : pohr_rec.pur_ord_no);
		if (comm_rec.inv_date > local_rec.lsysDate)
			pogh_rec.date_raised = comm_rec.inv_date;
		else
			pogh_rec.date_raised = local_rec.lsysDate;

		strcpy (pogh_rec.drop_ship, "N");
		cc = abc_add (pogh,&pogh_rec);
		if (cc)
			file_err (cc, pogh, "DBADD");

		cc = find_rec (pogh,&pogh_rec,EQUAL,"r");
	}
	else
	{
		strcpy (pogh_rec.rec_by, (shipment) ? "S" : "P");
		print_at (PageNumUpdate++,0,ML (mlPoMess185),pogh_rec.gr_no);

		cc = abc_update (pogh, &pogh_rec);
		if (cc)
			file_err (cc, pogh, "DBUPDATE");

		abc_unlock (pogh);
	}
	totalEstimatedCost = 0.00;

	abc_selfield (pohr,"pohr_hhpo_hash");
	abc_selfield (poln,"poln_hhpl_hash");

	print_at (PageNumUpdate++,0,ML ("Updating Goods Receipt Details ..."));

	if (envSkGrinNoPlate && !PO_RETURN)
		UpdateSknh ();

	scn_set (ITEM_SCN);

	if (envQcApply)
		open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no");

	for (line_cnt = 0;line_cnt < lcount [ITEM_SCN];line_cnt++) 
	{
		getval (line_cnt);

		if (envVarThreePl)
		{
			local_rec.qtyOrdered 	= local_rec.qtyOrdered;
			local_rec.qtyReceived 	= local_rec.qtyReceived;
			local_rec.actReceipt 	= local_rec.actReceipt;
			local_rec.qtySupplied 	= local_rec.qtySupplied;
		}
		else
		{
			local_rec.qtyOrdered 	= local_rec.qtyOrdered	* local_rec.CnvFct;
			local_rec.qtyReceived 	= local_rec.qtyReceived * local_rec.CnvFct;
			local_rec.actReceipt 	= local_rec.actReceipt  * local_rec.CnvFct;
			local_rec.qtySupplied 	= local_rec.qtySupplied * local_rec.CnvFct;
		}

		/*
		 * Update Purchase Order Header
		 */
		if (hhpoHash != poln_rec.hhpo_hash)
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "u");
			if (cc)
				file_err (cc, pohr, "DBFIND");

			strcpy (pohr_rec.status,"R");
			cc = abc_update (pohr,&pohr_rec);
			if (cc)
				file_err (cc, pohr, "DBUPDATE");

			hhpoHash = pohr_rec.hhpo_hash;
		}

		if (local_rec.qtySupplied != 0.00)
		{
		    UpdatePoln ();

		    if (newPogh)
				new_pogl = TRUE;
		    else
		    {
				pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
				pogl_rec.hhpl_hash = poln_rec.hhpl_hash;
				strcpy (pogl_rec.pur_status, "R");
				new_pogl = FALSE;
				cc = find_rec (pogl,&pogl_rec,EQUAL,"u");
				if (cc)
				    new_pogl = TRUE;
				new_pogl = TRUE;
		    }

		    if (new_pogl)
				AddPogl (line_cnt);
		    else
				UpdatePogl ();

		    totalEstimatedCost += (local_rec.qtySupplied * pogl_rec.land_cst);

		    if (shipment)
				UpdatePosl ();

			if (envQcApply && local_rec.qcReqd [0] == 'Y')
				UpdateQchr ();
		}
		if (!PO_RETURN && shipment && envIkeaSystem)
		{
			strcpy (posl_rec.co_no,comm_rec.co_no);
			posl_rec.hhsh_hash = posh_rec.hhsh_hash;
			posl_rec.hhpl_hash = poln_rec.hhpl_hash;
			cc = find_rec (posl,&posl_rec,EQUAL,"r");
			if (cc)
				file_err (cc, posl, "DBFIND");

			strcpy (poce_rec.bu_code, posh_rec.bucode);
			strcpy (poce_rec.bu_type, posh_rec.butype);
			sprintf (poce_rec.csm_no, "%-12.12s", posh_rec.csm_no);
			sprintf (poce_rec.csml_no, "%4d", posl_rec.csml_no);
			poce_rec.rec_qty = poln_rec.qty_rec;
			poce_rec.exp_qty = poln_rec.qty_ord;
			sprintf (poce_rec.rec_uom, "%-2.2s", local_rec.uom);
			sprintf (poce_rec.rec_date, "%-10.10s", local_rec.systemDate);
			cc = abc_add (poce, &poce_rec);
			if (cc && cc != 100)
				file_err (cc, "poce", "DBADD");
		}
	}
	if (envQcApply)
		abc_fclose (qchr);

	if (!PO_RETURN)
	{
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
		pohs_rec.est_cost += totalEstimatedCost;
		pohs_rec.act_cost = 0.00;
		strcpy (pohs_rec.printed, "N");
		strcpy (pohs_rec.stat_flag, "R");
		if (pohr_rec.stat_flag [0] == 'Q')
		{
			if (cc)
			{
				cc = abc_add (pohs, &pohs_rec);
				if (cc)
					file_err (cc, pohs, "DBADD");
			}
			else
			{
				cc = abc_update (pohs, &pohs_rec);
				if (cc)
					file_err (cc, pohs, "DBUPDATE");
			}
		}
		else
			abc_unlock (pohs);
	}

	if (shipment)
	{
		open_rec (pogd,pogd_list,POGD_NO_FIELDS,"pogd_id_no2");

		for (i = 0; i < 10; i++)
		{
			strcpy (pogd_rec.co_no, comm_rec.co_no);
			pogd_rec.hhsh_hash = posh_rec.hhsh_hash;
			pogd_rec.line_no = i;

			cc = find_rec (pogd,&pogd_rec,EQUAL,"u");
			if (cc)
			{
				abc_unlock (pogd);
				continue;
			}

			pogd_rec.hhpo_hash = 0L;
			pogd_rec.hhgr_hash = pogh_rec.hhgr_hash;

			cc = abc_update (pogd,&pogd_rec);
			if (cc)
				abc_delete (pogd);
		}
		abc_fclose (pogd);
	}
	else
	{
		open_rec (pogd,pogd_list,POGD_NO_FIELDS,"pogd_id_no3");

		for (i = 0; i < 10; i++)
		{
			strcpy (pogd_rec.co_no, comm_rec.co_no);
			pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
			pogd_rec.line_no = i;

			cc = find_rec (pogd,&pogd_rec,EQUAL,"u");
			if (cc)
			{
				abc_unlock (pogd);
				continue;
			}

			pogd_rec.hhsh_hash = 0L;
			pogd_rec.hhgr_hash = pogh_rec.hhgr_hash;

			cc = abc_update (pogd,&pogd_rec);
			if (cc)
				abc_delete (pogd);
		}
		abc_fclose (pogd);
	}
	sprintf (local_rec.previousPO,"%-15.15s",pohr_rec.pur_ord_no);

	abc_selfield (pohr,"pohr_id_no2");

	if (shipment)
		UpdatePosd ();

	PauseForKey (PageNumUpdate++, 0, ML ("Press Any Key To Continue"), 0);

	if (printerNumber == 0)
		return;

	if (shipment)
	{
		if (fork () == 0)
		{
			clear ();
			snorm ();
			sprintf (printerString,"%d",printerNumber);
			sprintf (hhsh_str,"%ld",posh_rec.hhsh_hash);

			execlp 
			(
				"po_reconprt",
				"po_reconprt",
				printerString,
				hhsh_str, (char *)0
			);
            /* modified coe for forcing exit */
            prog_exit = TRUE;
            return;
		}
		else
			wait ((int *)0);
	}
	swide ();
	set_tty ();
}

void
GenGrnNo (void)
{
	int		grnExists;

	/*
	 * Open Branch Master File.
	 */
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	/*
	 * Read Branch Master Record
	 */
	grnExists = TRUE;
	do
	{
		/*
		 * Read Branch Master Record.
		 */
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		/*
		 * Increment nx_gr_on and update esmr.
		 */
		sprintf (err_str, "%ld", ++esmr_rec.nx_gr_no);
		sprintf (pogh2_rec.gr_no, "%-15.15s", zero_pad (err_str, 15));
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");

		/*
		 * Check for existence of GRN.
		 */
		strcpy (pogh2_rec.co_no, comm_rec.co_no);
		cc = find_rec (pogh, &pogh2_rec, COMPARISON, "r");
		if (cc)
			grnExists = FALSE;

	} while (grnExists);

	sprintf (err_str, "%ld", esmr_rec.nx_gr_no);
	sprintf (pogh_rec.gr_no, "%-15.15s", zero_pad (err_str, 15));

	/*
	 * Close Branch Master File.
	 */
	abc_fclose (esmr);
}


void
UpdatePoln (void)
{
	poln_rec.hhpl_hash	=	poln_rec.hhpl_hash;
	cc = find_rec (poln, &poln_rec, EQUAL, "u");
	if (cc)
		file_err (cc, poln, "DBFIND");

	if (local_rec.actReceipt == local_rec.qtySupplied || envPoRecShip)
		poln_rec.ship_no = 0L;

	poln_rec.case_no = 0;
	strcpy (poln_rec.serial_no, local_rec.serial);
	
	if (!PO_RETURN)
		poln_rec.qty_rec += local_rec.qtySupplied;

	if (poln_rec.qty_ord - poln_rec.qty_rec < 0.00)
		strcpy (poln_rec.pur_status,"r");
	else
		strcpy (poln_rec.pur_status,"R");

	cc = abc_update (poln,&poln_rec);
	if (cc)
		file_err (cc, poln, "DBUPDATE");

	abc_unlock (poln);

	if (envPoSuHist)
		ProcessSuph ();
}

/*
 * Updated Supplier History records.
 */
void
ProcessSuph (void)
{
	if (pohr_rec.conf_date <= 0L)
			pohr_rec.conf_date = pohr_rec.date_raised;

	if (poln_rec.due_date <= 0L)
			poln_rec.due_date = pohr_rec.conf_date;

	strcpy (suph_rec.br_no, pohr_rec.br_no);
	suph_rec.hhbr_hash 		= poln_rec.hhbr_hash;
	suph_rec.hhum_hash 		= poln_rec.hhum_hash;
	suph_rec.hhcc_hash 		= poln_rec.hhcc_hash;
	suph_rec.hhsu_hash 		= pohr_rec.hhsu_hash;
	suph_rec.ord_date 		= pohr_rec.date_raised;
	suph_rec.due_date 		= poln_rec.due_date;
	suph_rec.ord_qty  		= (PO_RETURN) ? poln_rec.qty_ord * -1
									   	  : poln_rec.qty_ord;
	if (comm_rec.inv_date > local_rec.lsysDate)
		suph_rec.rec_date 		= comm_rec.inv_date;
	else
		suph_rec.rec_date 		= local_rec.lsysDate;

	suph_rec.rec_qty  		= (PO_RETURN) ? local_rec.qtySupplied * -1
									   	  : local_rec.qtySupplied;
	suph_rec.net_cost 		= poln_rec.fob_fgn_cst;
	suph_rec.land_cost 		= poln_rec.land_cst;
	strcpy (suph_rec.status, "A");
	if (shipment)
	{
		strcpy (suph_rec.ship_method, posh_rec.ship_method);
		suph_rec.ship_no = posh_rec.hhsh_hash;
		strcpy (suph_rec.csm_no, posh_rec.csm_no);
	}
	else
	{
		strcpy (suph_rec.ship_method, pohr_rec.ship_method);
		suph_rec.ship_no = 0L;
		strcpy (suph_rec.csm_no, " ");
	}

	strcpy (suph_rec.drop_ship, pohr_rec.drop_ship);
	strcpy (suph_rec.grn_no, pogh_rec.gr_no);
	strcpy (suph_rec.po_no , pohr_rec.pur_ord_no);

	cc = abc_add (suph,&suph_rec);
	if (cc)
		file_err (cc, "suph", "DBADD");
}

void
UpdatePogl (void)
{
	if (PO_RETURN)
		pogl_rec.qty_rec -= local_rec.qtySupplied;
	else
		pogl_rec.qty_rec += local_rec.qtySupplied;

	cc = abc_update (pogl,&pogl_rec);
	if (cc)
		file_err (cc, pogl, "DBUPDATE");
}

void
UpdatePosl (void)
{
	strcpy (posl_rec.co_no,comm_rec.co_no);
	posl_rec.hhsh_hash = posh_rec.hhsh_hash;
	posl_rec.hhpl_hash = poln_rec.hhpl_hash;
	cc = find_rec (posl,&posl_rec,EQUAL,"u");
	if (cc)
		return;

	posl_rec.rec_qty += local_rec.qtySupplied;
	posl_rec.ship_qty = 0.00;

	cc = abc_update (posl,&posl_rec);
	if (cc)
		file_err (cc, posl, "DBUPDATE");

	return;
}

void
AddPogl (
	int		line_no)
{
	pogl_rec.hhgr_hash 	= pogh_rec.hhgr_hash;
	pogl_rec.line_no 	= line_no;
	pogl_rec.hhbr_hash 	= poln_rec.hhbr_hash;
	pogl_rec.hhcc_hash 	= poln_rec.hhcc_hash;
	pogl_rec.hhum_hash 	= poln_rec.hhum_hash;
	pogl_rec.hhpl_hash 	= poln_rec.hhpl_hash;
	pogl_rec.hhlc_hash 	= poln_rec.hhlc_hash;
	strcpy (pogl_rec.po_number,pohr_rec.pur_ord_no);
	strcpy (pogl_rec.serial_no,poln_rec.serial_no);
	strcpy (pogl_rec.lot_no, 	GetLotNo (line_no, 0));
	strcpy (pogl_rec.slot_no,   GetSLotNo (line_no, 0));
	strcpy (pogl_rec.location,  GetLoc (line_no, 0));
	pogl_rec.exp_date = StringToDate (GetExpiry (line_no, 0));

	if (PO_RETURN)
	{
		pogl_rec.qty_ord = (local_rec.qtyOrdered - local_rec.qtyReceived) * -1;
		pogl_rec.qty_rec = local_rec.qtySupplied * -1;
	}
	else
	{
		pogl_rec.qty_ord 	= local_rec.qtyOrdered - local_rec.qtyReceived;
		pogl_rec.qty_rec 	= local_rec.qtySupplied;
	}
	pogl_rec.fob_fgn_cst 	= poln_rec.fob_fgn_cst;
	pogl_rec.frt_ins_cst 	= poln_rec.frt_ins_cst;
	pogl_rec.fob_nor_cst 	= poln_rec.fob_nor_cst;
	pogl_rec.lcost_load  	= poln_rec.lcost_load;
	pogl_rec.duty 			= poln_rec.duty;
	pogl_rec.licence 		= poln_rec.licence;
	pogl_rec.land_cst 		= poln_rec.land_cst;
	pogl_rec.pack_qty		= local_rec.packageQty;
	pogl_rec.chg_wgt		= local_rec.totalChargeWgt;
	pogl_rec.gross_wgt		= local_rec.totalGrossWgt;
	pogl_rec.cu_metre		= local_rec.totalCBM;
	pogl_rec.hhve_hash		= trve_rec.hhve_hash;

	strcpy (pogl_rec.cus_ord_ref,local_rec.cusOrdRef);
	strcpy (pogl_rec.cat_code,poln_rec.cat_code);
	strcpy (pogl_rec.item_desc,poln_rec.item_desc);
	if (comm_rec.inv_date < local_rec.lsysDate)
		pogl_rec.rec_date 	= comm_rec.inv_date;
	else
		pogl_rec.rec_date 	= local_rec.lsysDate;

	strcpy (pogl_rec.pur_status,"R");
	strcpy (pogl_rec.gl_status, "R");
	strcpy (pogl_rec.stat_flag,"0");

	cc = abc_add (pogl,&pogl_rec);
	if (cc)
		file_err (cc, pogl, "DBADD");

	if ((MULT_LOC || SK_BATCH_CONT) && PO_RETURN)
	{
		cc = find_rec (pogl, &pogl_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pogl, "DBFIND");

		AllocLotLocation
		(
			line_no,
			TRUE,
			LL_LOAD_GL,
			pogl_rec.hhgl_hash
		);
	}
}

void
UpdatePosd (void)
{
	print_at (PageNumUpdate++,0,ML (mlPoMess186));

	abc_selfield (posl,"posl_id_no_2");

	strcpy (posd_rec.co_no,comm_rec.co_no);
	posd_rec.hhsh_hash = posh_rec.hhsh_hash;
	posd_rec.hhpo_hash = 0L;

	cc = find_rec (posd,&posd_rec,GTEQ,"u");
	while (!cc && !strcmp (posd_rec.co_no,comm_rec.co_no) &&
		       posd_rec.hhsh_hash == posh_rec.hhsh_hash)
	{
		CalculatePosd (posh_rec.hhsh_hash,posd_rec.hhpo_hash);

		cc = abc_update (posd,&posd_rec);
		if (cc)
			file_err (cc, posd, "DBUPDATE");

		abc_unlock (posd);

		cc = find_rec (posd,&posd_rec,NEXT,"u");
	}
	abc_unlock (posd);

	abc_selfield (posl,"posl_id_no");
}

void
CalculatePosd (
	long	hhshHash,
	long	hhpoHash)
{
	double	extend;

	posd_rec.total = 0.00;

	strcpy (posl_rec.co_no,comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpo_hash = hhpoHash;
	cc = find_rec (posl,&posl_rec,GTEQ,"r");
	while (!cc && !strcmp (posl_rec.co_no,comm_rec.co_no) &&
		      posl_rec.hhsh_hash == hhshHash &&
		      posl_rec.hhpo_hash == hhpoHash)
	{
		poln_rec.hhpl_hash	=	posl_rec.hhpl_hash;
		cc = find_rec (poln2, &poln_rec, EQUAL, "r");
		if (cc)
			inmr_rec.outer_size = 0.00;
		else
		{
			inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
				inmr_rec.outer_size = 0.00;
		}

		extend = (double) posl_rec.rec_qty;
		extend *= posl_rec.sup_price;
		extend = out_cost (extend, inmr_rec.outer_size);

		posd_rec.total += extend;

		cc = find_rec (posl,&posl_rec,NEXT,"r");
	}
}

/*
 * Adds a inventory QC purchase items reveival file.
 */
void
UpdateQchr (void)
{
	float	qcDays;

	incc_rec.hhbr_hash = local_rec.hhbrHash;
	incc_rec.hhcc_hash = local_rec.hhccHash;
	if (find_rec (incc, &incc_rec, EQUAL, "r"))
		incc_rec.qc_time = 0.00;
	qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

	strcpy (qchr_rec.co_no,			comm_rec.co_no);
	strcpy (qchr_rec.br_no,			comm_rec.est_no);
	strcpy (qchr_rec.wh_no,			comm_rec.cc_no);
	strcpy (qchr_rec.qc_centre,		local_rec.qcCentre);
	qchr_rec.hhbr_hash				= local_rec.hhbrHash;
	qchr_rec.origin_qty				= local_rec.qtySupplied;
	if (comm_rec.inv_date > local_rec.lsysDate)
	{
		qchr_rec.receipt_dt				= comm_rec.inv_date;
		qchr_rec.exp_rel_dt				= comm_rec.inv_date + (long) qcDays;
	}
	else
	{
		qchr_rec.receipt_dt				= local_rec.lsysDate;
		qchr_rec.exp_rel_dt				= local_rec.lsysDate + (long) qcDays;
	}
	qchr_rec.rel_qty				= 0.00;
	qchr_rec.rej_qty				= 0.00;
	qchr_rec.inlo_hash				=	GetINLO	(line_cnt, 0);
	qchr_rec.hhum_hash				=	SR.hhumHash;
	sprintf (qchr_rec.serial_no,	"%-25.25s", local_rec.serial);
	qchr_rec.hhsu_hash				= sumr_rec.hhsu_hash;
	sprintf (qchr_rec.ref_1,		"%-10.10s", pohr_rec.pur_ord_no + 5);
	sprintf (qchr_rec.ref_2,		"%-10.10s", pogh_rec.gr_no + 5);
	qchr_rec.shipment				= posh_rec.hhsh_hash;
	qchr_rec.next_seq				= 0;
	strcpy (qchr_rec.source_type,	"P");

	if (abc_add (qchr, &qchr_rec))
		file_err (cc, "qchr", "DBFIND");
}

/*
 * Search for skcm.
 */
void
SrchSkcm (
 char	*keyValue)
{
	_work_open (15,0,40);
	save_rec ("#Container", "#Description");
	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", keyValue);
	cc = find_rec (skcm, &skcm_rec, GTEQ, "r");
	while (!cc && !strcmp (skcm_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm_rec.container, keyValue, strlen (keyValue)))
	{
		cc = save_rec (skcm_rec.container, skcm_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm, &skcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", temp_str);
	cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, skcm, "DBFIND");
}
/*
 * Search for trve.
 */
void
SrchTrve (
	char	*keyValue)
{
	_work_open (10,0,40);
	save_rec ("#Vehicle","#Description");
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", keyValue);
	cc = find_rec (trve, &trve_rec, GTEQ, "r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trve_rec.br_no, comm_rec.est_no) &&
			      !strncmp (trve_rec.ref,keyValue,strlen (keyValue)))
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
	sprintf (trve_rec.ref,"%-10.10s", temp_str);
	cc = find_rec (trve, &trve_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)trve, "DBFIND");
}
/*
 * Search for QC Centre.
 */
void
SrchQcmr (
	char	*keyValue)
{
	_work_open (4,0,40);
	save_rec ("#Cntr", "#Centre Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", keyValue);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, keyValue, strlen (keyValue)))
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
	if (find_rec (qcmr, &qcmr_rec, NEXT, "r"))
		file_err (cc, "qcmr", "DBFIND");
}

/*
 * Search for goods receipt number.
 */
void
SrchPogh (
	char	*keyValue)
{
	_work_open (15,0,60);
	save_rec ("#GRIN Number    ","# System Reference          | Date Raised");
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",keyValue);
	cc = find_rec (pogh,&pogh_rec,GTEQ,"r");
	while (!cc && !strncmp (pogh_rec.gr_no,keyValue,strlen (keyValue)) &&
		      !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		if (((!shipment &&
			pogh_rec.hhsu_hash == sumr_rec.hhsu_hash) ||
			(shipment &&
			  pogh_rec.hhsh_hash == posh_rec.hhsh_hash)) &&
			pogh_rec.pur_status [0] == 'R')
		{
			if (shipment)
			{
				sprintf (err_str, "%s : %15.15s | %s ",
					ML ("Shipment"),
					GetShipmentNo (posh_rec.hhsh_hash),
					DateToString (pogh_rec.date_raised));
			}
			else
			{
				sprintf (err_str, "%s : %s | %s ",
					ML ("P/Order "),
					pogh_rec.pur_ord_no,
					DateToString (pogh_rec.date_raised));
			}
			cc = save_rec (pogh_rec.gr_no,err_str);
			if (cc)
				break;
		}
		cc = find_rec (pogh,&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",temp_str);
	cc = find_rec (pogh,&pogh_rec,EQUAL,"r");
	if (cc)
		file_err (cc, pogh, "DBFIND");
}

/*
 * Search for shipment number.
 */
void
SrchPosh (
	char	*keyValue)
{
	char	dsp_method [5];

	_work_open (15,0,60);
	save_rec ("#Shipment No.","#Ship Name            |     Departed  from   | Method | Destination       ");
	strcpy  (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no,"%-12.12s", " ");
	cc = find_rec (posh,&posh_rec,GTEQ,"r");
	while (!cc && !strcmp (posh_rec.co_no,comm_rec.co_no))
	{
		if ((posh_rec.status [0] == 'I' ||
		      posh_rec.status [0] == 'R') &&
		      !strncmp (posh_rec.csm_no,keyValue,strlen (keyValue)))
		{
			if (posh_rec.ship_method [0] == 'A')
				strcpy (dsp_method, "AIR ");
			else if (posh_rec.ship_method [0] == 'S')
				strcpy (dsp_method, "SEA ");
			else if (posh_rec.ship_method [0] == 'L')
				strcpy (dsp_method, "LAND");
			else
				strcpy (dsp_method, "????");

			sprintf (err_str, "%s | %s |  %s   | %s ", 
						posh_rec.vessel,
						posh_rec.port,
						dsp_method,
						posh_rec.destination);
					
			cc = save_rec (posh_rec.csm_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (posh,&posh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (posh_rec.co_no,comm_rec.co_no);
	strcpy (posh_rec.csm_no,temp_str);
	cc = find_rec (posh,&posh_rec,EQUAL,"r");
	if (cc)
		file_err (cc, posh, "DBFIND");
}

/*
 * Search for purchase order number.
 */
void
SrchPohr (
	char	*keyValue)
{
	int NonZeroFound;

	_work_open (15,0,60);

	if (PO_RETURN)
		save_rec ("#P/R No.        ", "#Supp. |   Order Terms.     |Date Raised|                   Contact              ");
	else
		save_rec ("#P/O No.        ", "#Supp. |   Order Terms.     |Date Raised|                   Contact              ");

	abc_selfield (poln, "poln_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc &&
		   !strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue)) &&
		   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.drop_ship [0] == 'Y')
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}

		NonZeroFound = FALSE;
		poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
		poln_rec.line_no = 0;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
			if (local_rec.caseNumber > 0 &&
		    	 poln_rec.case_no != local_rec.caseNumber)
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}

			if (poln_rec.ship_no > 0L)
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}
			if (PO_RETURN && poln_rec.pur_status [0] == 'O')
			{
				if (poln_rec.pur_status [0] == 'O')
				{
					NonZeroFound = TRUE;
					break;
				}
			}
			if (!PO_RETURN && (twodec (poln_rec.qty_ord - poln_rec.qty_rec)) > 0.00)
			{
				NonZeroFound = TRUE;
				break;
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");

		}

		if ((PO_RETURN && pohr_rec.type [0] != 'C') ||
			  (!PO_RETURN && pohr_rec.type [0] == 'C'))
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}
		
		if ((pohr_rec.status [0] == 'O' || pohr_rec.status [0] == 'R') 
				&& NonZeroFound == TRUE)
		{
			sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
				strcpy (sumr_rec.crd_no, "NoSupp");

			sprintf (err_str, "%s|%s|%-10.10s | %s",
						 sumr_rec.crd_no,
						 pohr_rec.term_order, 
						 DateToString (pohr_rec.date_raised),
						 pohr_rec.contact);
	
			cc = save_rec (pohr_rec.pur_ord_no ,err_str);
			if (cc)
				break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	abc_fclose (sumr);
	if (cc)
		return;
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%15.15s", temp_str);
	cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pohr, "DBFIND");
}

/*
 * Check Whether A Serial Number For This Item Number
 * Has Already Been Used.  Return 1 if duplicate	
 */
int
CheckDupSerial (
 char	*serial_no,
 long	hhwhHash,
 int		line_no)
{
	int		i;
	int		no_lines;

	no_lines = (INPUT && (lcount [ITEM_SCN] - 1 < line_cnt)) ? line_cnt : lcount [ITEM_SCN] - 1;

	for (i = 0;i < no_lines;i++)
	{
		/*
		 * Ignore Current Line
		 */
		if (i == line_no)
			continue;

		/*
		 * cannot duplicate item_no/serial_no unless serial no was not input	
		 */
		if (!strcmp (store [i].serial_no,twentyfiveSpaces))
			continue;

		/*
		 * Only compare serial numbers for the same item number.
		 */
		if (store [i].hhwhHash == hhwhHash)
		{
			if (!strcmp (store [i].serial_no,serial_no))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

int
CheckOk (
 void)
{
	int		i;
	char	wrk_lot_no [8];
	char	wrk_loc_no [11];

	if (PO_RETURN)
		return (TRUE);

	scn_set (ITEM_SCN);
	for (i = 0; i < MAXLINES && i < lcount [ITEM_SCN]; i++)
	{
	    getval (i);
	    if (store [i].ser_item [0] == 'Y' &&
			store [i].err_found [0] == 'Y' &&
			local_rec.qtySupplied > 0.00)
	    {
			sprintf (err_str, ML (mlPoMess187), i + 1);
			return (FALSE);
	    }

	    if (store [i].ser_item [0] == 'N' &&
			store [i].err_found [0] == 'Y' &&
			local_rec.qtySupplied > 0.00 && !check_class (store [i]._class))
	    {
			sprintf (err_str, ML (mlPoMess188), i + 1);
			return (FALSE);
	    }

	    strcpy (wrk_lot_no, GetLotNo (i, 0));
	    clip (wrk_lot_no);
	    if (store [i].lot_ctrl [0] == 'Y' && strlen (wrk_lot_no) == 0 &&
			local_rec.qtySupplied > 0.00)
	    {
			sprintf (err_str, ML (mlPoMess189), i + 1);
			return (FALSE);
	    }
	    strcpy (wrk_loc_no, GetLoc (i, 0));
	    clip (wrk_loc_no);
	    if (MULT_LOC && strlen (wrk_loc_no) == 0 && local_rec.qtySupplied > 0.00)
	    {
			heading (ITEM_SCN);
			scn_display (ITEM_SCN);
			edit_scn (label ("LL"), i);
			sprintf (err_str,ML (mlPoMess188),i + 1);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			continue;
	    }
	    strcpy (wrk_loc_no, GetLoc (i, 0));
	    clip (wrk_loc_no);
	    if (MULT_LOC && envPoChkDfltLoc && !strcmp (wrk_loc_no, llctDefault) && local_rec.qtySupplied > 0.00)
	    {
			heading (ITEM_SCN);
			scn_display (ITEM_SCN);
			edit_scn (label ("LL"), i);
			sprintf (err_str,ML ("Default Location used on line %d, please check"),i + 1);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			continue;
	    }

		if (envQcApply &&
			store [i].qcReqd [0] == 'Y' &&
			!strcmp (store [i].qcCentre, "    "))
		{
			sprintf (err_str, ML (mlPoMess190), i +1);
			return (FALSE);
		}
	}

	return (TRUE);
}

/*
 * Display Infor for lines while in edit mode.
 */
void
tab_other (
	int		iline)
{
	if (cur_screen == ITEM_SCN)
	{
		if (store [iline].ser_item [0] == 'Y')
		{
			if (FLD ("ser_no") != ND)
				FLD ("ser_no") = YES;
		}
		else
		{
			if (FLD ("ser_no") != ND)
				FLD ("ser_no") = NA;
		}

		if (!strncmp (store [iline].itemDesc,twentyfiveSpaces, 25))
		{
			move (0, 3); cl_line ();
			move (0, 4); cl_line ();
			return;
		}

		print_at (3, 0, ML (mlPoMess075));
		print_at (3,10," %3d", iline + 1);
		print_at (3, 17, ML ("Br : %s"),store [iline].br_no);
		print_at (3, 35, ML (mlStdMess099),
								store [iline].br_no, store [iline].acronym);
		print_at (3, 66, ML (mlPoMess079));
		print_at (3,83," %-40.40s", store [iline].itemDesc);

		if (store [iline].outer == 1.00 ||  store [iline].outer == 0.00)
			sprintf (err_str, "%s %s %-6.6s", ML ("Land Cost "),ML ("UOM  "),store [iline].uom);
		else
			sprintf (err_str, "%s * %s %6.1f", ML ("Land Cost "), ML ("Outer"),store [iline].outer);

		print_at (4, 0, "%R %s: ", err_str);
		print_at (4,26," %14.14s",CF (store [iline].landCost,"NNN,NNN,NNN.NN"));

		print_at (4, 66, ML (mlPoMess165));
		print_at (4, 84," %-16.16s", store [iline].sup_part);

		if (envQcApply)
		{
			if (store [iline].qcReqd [0] == 'Y')
				FLD ("qcCentre") = YES;
			else
				FLD ("qcCentre") = NA;
		}
		else
			FLD ("qcCentre") = ND;

		if (FLD ("ser_no") != ND)
		{
			if (store [iline].ser_item [0] == 'Y')
				FLD ("ser_no") = YES;
			else
				FLD ("ser_no") = NA;
		}
		if (FLD ("LL") != ND)
		{
		/*
			if (store [iline].hhplOrig > 0L)
				FLD ("LL") = NA;
			else
				FLD ("LL") = YES;
				*/
		}
	}
	return;
}

/*
 * Find first line for non stock line.
 */
int
FindPons (
	long	hhplHash)
{
	pons_rec.hhpl_hash 	= hhplHash;
	pons_rec.line_no 	= 0;
	return (find_rec ("pons", &pons_rec, COMPARISON, "r"));
}

void
PrintCoStuff (
 void)
{
	line_at (19,0,132);
	print_at (20,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	print_at (22,0, ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);
}

void
CalcExpiry (
 void)
{
	int		tmp_dmy [3];

	inei_rec.hhbr_hash = SR.hhbrHash;
	strcpy (inei_rec.est_no, SR.br_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc) 
	{
		SR.exp_date = 0L;
		return;
	}
	DateToDMY (local_rec.lsysDate, &tmp_dmy [0],&tmp_dmy [1],&tmp_dmy [2]);
	tmp_dmy [1]--;
	tmp_dmy [1] += inei_rec.expiry_prd1;
	tmp_dmy [2] += (tmp_dmy [1] / 12);
	tmp_dmy [1] %= 12;
	tmp_dmy [1]++;
	do
	{
		local_rec.expiryDate = DMYToDate (tmp_dmy [0],tmp_dmy [1],tmp_dmy [2]);
		tmp_dmy [0]--;
	} while (local_rec.expiryDate == -1);

	if (local_rec.expiryDate < local_rec.lsysDate)
		SR.exp_date = 0L;

	SR.exp_date = local_rec.expiryDate;
	return;
}

/*
 * Input not stock description lines for non-stock products.
 */
void		
LoadNoPlateNotes (
	int		currentScreen)
{
	int		newSknh;

	if (!plateCalled)
		return;

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
	strcpy (sknh_rec.plate_no, pogh_rec.gr_no);
	newSknh = find_rec (sknh, &sknh_rec, COMPARISON, "r");
	if (newSknh)
		memset (&sknh_rec , 0, sizeof (sknh_rec));

	scn_set (DESC_SCN);
	lcount [DESC_SCN] = 0;

	strcpy (local_rec.labNote, sknh_rec.lab_note1);
	putval (lcount [DESC_SCN]++);

	strcpy (local_rec.labNote, sknh_rec.lab_note2);
	putval (lcount [DESC_SCN]++);

	strcpy (local_rec.labNote, sknh_rec.lab_note3);
	putval (lcount [DESC_SCN]++);

	strcpy (local_rec.labNote, sknh_rec.lab_note4);
	putval (lcount [DESC_SCN]++);

	strcpy (local_rec.labNote, sknh_rec.lab_note5);
	putval (lcount [DESC_SCN]++);

	strcpy (local_rec.labNote, sknh_rec.lab_note6);
	putval (lcount [DESC_SCN]++);

	scn_set (currentScreen);
}
/*
 * Update purchase order non stock lines file.
 */
void
UpdateSknh (void)
{
	int		newSknh,
			i;

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
	strcpy (sknh_rec.plate_no, pogh_rec.gr_no);

	newSknh = find_rec (sknh, &sknh_rec, COMPARISON, "u");
	
	scn_set (DESC_SCN);
	for (i = 0; i < lcount [DESC_SCN]; i++)
	{
		getval (i);

		switch (i)
		{
			case	0:
			strcpy (sknh_rec.lab_note1, local_rec.labNote);
			break;

			case	1:
			strcpy (sknh_rec.lab_note2, local_rec.labNote);
			break;

			case	2:
			strcpy (sknh_rec.lab_note3, local_rec.labNote);
			break;

			case	3:
			strcpy (sknh_rec.lab_note4, local_rec.labNote);
			break;

			case	4:
			strcpy (sknh_rec.lab_note5, local_rec.labNote);
			break;

			case	5:
			strcpy (sknh_rec.lab_note6, local_rec.labNote);
			break;
		}
	}
	strcpy (sknh_rec.printed, "0");
	strcpy (sknh_rec.edi, "0");
	strcpy (sknh_rec.pur_ord_no, pohr_rec.pur_ord_no);

	if (newSknh)
	{
		sknh_rec.rec_date	=	TodaysDate ();

		cc = abc_add (sknh, &sknh_rec);
		if (cc)
			file_err (cc, sknh, "DBADD");
	}
	else
	{
		cc = abc_update (sknh, &sknh_rec);
		if (cc)
			file_err (cc, sknh, "DBUPDATE");
	}
}
/*
 * See if shipment is on file.
 */
char*
GetShipmentNo (
	long    hhshHash)
{
	posh2_rec.hhsh_hash	=	hhshHash;
	cc = find_rec (posh2, &posh2_rec, EQUAL, "r");
	if (cc)  
		strcpy (err_str, " N/A ");
	else
		sprintf (err_str, "%-12.12s", posh2_rec.csm_no);

	return (err_str);
}           
void
CheckEnvironment (void)
{
	char	*sptr;

	sptr = chk_env ("SK_SERIAL_OK");
	envSkSerialOk = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_CASE_USED");
	if (sptr == (char *)0)
		strcpy (envPoCaseUsed, "N");
	else
		sprintf (envPoCaseUsed,"%-1.1s", sptr);

	sptr = chk_env ("QC_APPLY");
	envQcApply = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_REC_SHIP");
	envPoRecShip = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_SU_HIST");
	envPoSuHist = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_CHK_DFLT_LOC");
	envPoChkDfltLoc = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("PO_REC_SORT_DES");
	envPoRecSortDes = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("IKEA_SYSTEM");
	envIkeaSystem = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("CR_CO");
	envVarCrCo = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("CR_FIND");
	envVarCrFind = (sptr == (char *)0) ? 0 : atoi (sptr);
	
	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("DB_FIND");
	envVarDbFind = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarThreePl = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = (sptr == (char *)0) ? 0 : atoi (sptr);
}

static BOOL
IsSpaces (
	char	*str)
{ 
	/*
	 * Return TRUE if str contains only white space or nulls
	 */
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
}
/*
 * Print Heading.
 */
int
heading	(
	int		scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		if (PO_RETURN)
			rv_pr (ML (mlPoMess172), 42, 0, 1);
		else
			rv_pr (ML (mlPoMess173), 48, 0, 1);

		print_at (0,96,ML (mlPoMess174), local_rec.previousSupplier, local_rec.previousPO);
		line_at (1,0,131);

		if (scn == HEADER_SCN || scn == DESC_SCN)
		{
			if (PO_RETURN)
			{
				box (0, 3, 132, (envVarThreePl) ? 4 : 2);
				if (envVarThreePl)
					line_at (6,1,131);
			}
			else
			{
				box (0, 3, 132, 5);
				if (!envVarThreePl)
					line_at (7,1,131);
			}
			if (envSkGrinNoPlate && !PO_RETURN)
			{
				if (scn == HEADER_SCN)
				{
					scn_set (DESC_SCN);
					scn_write (DESC_SCN);
					scn_display (DESC_SCN);
				}
				else
				{
					scn_set (HEADER_SCN);
					scn_write (HEADER_SCN);
					scn_display (HEADER_SCN);
				}
			}
		}

		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


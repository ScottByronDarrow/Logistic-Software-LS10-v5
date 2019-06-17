/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_alldisp.c,v 5.25 2002/12/01 04:48:15 scott Exp $
|  Program Name  : (sk_alldisp.c)                                     |
|  Program Desc  : (Full System Stock Display.                    )   |
|---------------------------------------------------------------------|
|  Date Written  : (04/02/88)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sk_alldisp.c,v $
| Revision 5.25  2002/12/01 04:48:15  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.24  2002/11/28 04:09:48  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.23  2002/11/27 02:18:42  robert
| SC0035 - Fixed unable to display location display options on second time
| it was chosen by using cl_box () instead of box ().
|
| Revision 5.22  2002/05/06 01:35:18  scott
| Updated to remove limit of display of purchase history.
|
| Revision 5.21  2002/02/08 05:54:39  cha
| S/C 766. Updated to make sure that stock out quantity are
| displayed as negative not positive.
|
| Revision 5.20  2002/01/29 08:52:42  robert
| SC 00707 - Updated to fix display issue on LS10-GUI
|
| Revision 5.19  2002/01/25 01:41:40  scott
| Updated to fix bell message
|
| Revision 5.18  2002/01/23 08:36:51  scott
| S/C 00478 - Updated as not all fields cleared when switching between <S3> <S4>
|
| Revision 5.17  2002/01/23 07:59:41  scott
| S/C 00704 - Location details no not display after selecting <Serial item>
| display option.
|
| Revision 5.16  2001/12/03 02:12:38  scott
| Updated to allow new dril down to shipments.
|
| Revision 5.15  2001/11/07 08:04:04  scott
| Updated to display smallest quantity for number plates based on sknd and inlo
|
| Revision 5.14  2001/11/05 01:40:30  scott
| Updated from Testing.
|
| Revision 5.13  2001/10/24 11:02:26  scott
| Updated for Direct deliveries
|
| Revision 5.12  2001/10/19 03:11:41  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.11  2001/10/17 09:03:56  cha
| Updated to ensure consistent rounding (ISSUE # 00061).
| Changes made by Scott.
|
| Revision 5.10  2001/10/17 08:31:57  cha
| Update to fix lineup of direct delivery display.
| Changes made by Scott.
|
| Revision 5.9  2001/10/09 23:06:40  scott
| Updated for returns
|
| Revision 5.8  2001/09/24 01:51:18  scott
| Updated for number plate returns
|
| Revision 5.7  2001/08/28 10:12:22  robert
| additional update for LS10.5-GUI
|
| Revision 5.6  2001/08/28 08:46:32  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.5  2001/08/23 12:07:53  scott
| Updated from scotts machine
|
| Revision 5.4  2001/08/20 23:22:00  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 09:18:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:37  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:48  scott
| Update - LS10.5
|
| Revision 4.18  2001/05/22 03:55:29  scott
| Updated to include data from inme (display B/F information)
|
| Revision 4.17  2001/05/08 06:43:09  robert
| Updated for the modified LS10-GUI RingMenu
|
| Revision 4.16  2001/05/07 04:39:17  cha
| Updated to change SQL call on purchase history to LTEQ.
| Updated to display supplier price to be / outer size
|
| Revision 4.15  2001/05/07 02:28:33  scott
| Updated to ensure purchase history is rounded correctly
|
| Revision 4.14  2001/05/06 05:02:16  scott
| Updated to use history uom and not standard uom (error)
|
| Revision 4.13  2001/05/02 01:46:12  cha
| Updated to fix some rounding
|
| Revision 4.12  2001/04/19 10:07:03  cha
| Forced Update
|
| Revision 4.10  2001/04/10 11:01:14  scott
| Updated as LIFO never been implemented correctly.
|
| Revision 4.9  2001/04/05 10:08:17  scott
| Updated to make next / previous work with Oracle.
|
| Revision 4.8  2001/04/05 03:01:06  scott
| Updated to fix POPUP default and usage of SK_ALLDISP_POP
| Updated to add IKEA_SYSTEM to display Maker Number or Brand Number
|
| Revision 4.7  2001/04/04 23:38:14  scott
| Updated to change LT call in intr to LTEQ using SQL logic that works with CISAM
|
| Revision 4.6  2001/03/27 01:32:56  scott
|
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.5  2001/03/22 09:07:23  scott
| Updated to fix default on Date Selection, was allowing non numeric characters
|
| Revision 4.4  2001/03/22 08:53:00  scott
| Updated as stock enquiry selection screen displayed END OF RANGE for both
| start and end of range.
|
| Revision 4.3  2001/03/22 08:32:35  scott
| Updated to adjust screen to look better with LS10-GUI
|
| Revision 4.2  2001/03/22 08:21:50  scott
| Updated to lineup company / branch / warehouse prompts.
| Updated to adjust screen to look better with LS10-GUI
|
| Revision 4.1  2001/03/22 07:33:30  scott
| Updated to fix redraw on printer box.
|
| Revision 4.0  2001/03/09 02:36:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.10  2001/02/23 08:30:09  scott
| Updated to re-name Maker number to Brand Number as it is used as brand.
|
| Revision 3.9  2001/02/12 01:50:04  scott
| Updated to add options ex IKEA HK
| Added showing of promo price and date if overides item price.
| Added showing of all promo prices.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_alldisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_alldisp/sk_alldisp.c,v 5.25 2002/12/01 04:48:15 scott Exp $";

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

#define	TXT_REQD
#include	<pslscr.h>	/*  Gen. C Screen Handler Header          */
#include	<string.h>
#include	<hot_keys.h>
#include	<ring_menu.h>
#include	<graph.h>
#include	<getnum.h>
#include	<twodec.h>
#include	<loc_types.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>

#ifdef GVISION
#include	<pictureAt.h>
#endif

#include	"schema"

#define		NDEC(x)		n_dec (x, inmr_rec.dec_pt)

#define	FIFO		(inmr_rec.costing_flag [0] == 'F')
#define	FIFOLIFO	(inmr_rec.costing_flag [0] == 'F' || \
					  inmr_rec.costing_flag [0] == 'I')

#define	ACT_COST	(envVarPoActCost [0] == 'Y')
#define	FORWARD		(soln_rec.status [0] == 'F')
#define	GST			(envVarGst [0] == 'Y')
#define	MAX_TRANS	200

#define	MAX_MONTHS	36
#define	UD_CHAR		(udih_rec.field_type == 1)
#define	UD_INT		(udih_rec.field_type == 2)
#define	UD_FLOAT	(udih_rec.field_type == 3)
#define	UD_DOUBLE	(udih_rec.field_type == 4)

#define SCREENWIDTH	80
#define WIDESCREEN	132

#define	IN_POP_A 	1
#define	IN_POP_B	2
#define	IN_POP_C	3
#define	IN_POP_D	4
#define	IN_POP_E	5
#define	IN_POP_F	6

#ifdef GVISION
#define IN_POP_G	7
#endif

#define	POP_Y 		4
#define	POP_X		68

#define		CAL(amt, pc)		DPP (amt * DOLLARS (pc))

#define		VAL_ITLN 	(itln_rec.status [0] == 'T' || \
						  itln_rec.status [0] == 'B' || \
						  itln_rec.status [0] == 'M' || \
						  itln_rec.status [0] == 'U')
#define		VAL_TRANSIT (itln_rec.status [0] == 'T')

#define		PHANTOM		(inmr_rec.inmr_class [0] == 'P')
#define		NON_SYN		(inmr_rec.hhsi_hash == 0L)

struct	ccmrRecord	ccmr2_rec;
struct	ccmrRecord	ccmr_rec;
struct	cmhrRecord	cmhr_rec;
struct	cmrdRecord	cmrd_rec;
struct	cmrhRecord	cmrh_rec;
struct	colnRecord	coln_rec;
struct	commRecord	comm_rec;
struct	cumrRecord	cumr_rec;
struct	esmrRecord	esmr_rec;
struct	excfRecord	excf_rec;
struct	ffprRecord	ffpr_rec;
struct	inasRecord	inas_rec;
struct	inbmRecord	sk_inbm_rec;
struct	inccRecord	incc_rec;
struct	incpRecord 	incp_rec;
struct	inedRecord	ined_rec;
struct	inexRecord	inex_rec;
struct	inisRecord	inis_rec;
struct	inlaRecord	inla_rec;
struct	inldRecord	inld_rec;
struct	inloRecord	inlo_rec;
struct	inmbRecord	inmb_rec;
struct	inmeRecord	inme_rec;
struct	inmlRecord	inml_rec;
struct	inmrRecord	inmr2_rec;
struct	inmrRecord	inmr3_rec;
struct	inmrRecord	inmr4_rec;
struct	inmrRecord	inmr_rec;
struct	inndRecord	innd_rec;
struct	innhRecord	innh_rec;
struct	inoiRecord	inoi_rec;
struct	inprRecord	inpr2_rec;
struct	inprRecord	inpr_rec;
struct	intrRecord	intr_rec;
struct	inumRecord	inum_rec;
struct	inwdRecord	inwd_rec;
struct	inwsRecord	inws_rec;
struct	inwuRecord	inwu_rec;
struct	ithrRecord	ithr_rec;
struct	itlnRecord	itln_rec;
struct	iudcRecord	iudc_rec;
struct	iudiRecord	iudi_rec;
struct	iudsRecord	iuds_rec;
struct	llihRecord	llih_rec;
struct	llstRecord 	llst_rec;
struct	lrphRecord	lrph_rec;
struct	pcmsRecord	pcms_rec;
struct	pcwoRecord	pcwo_rec;
struct	pocrRecord	pocr_rec;
struct	pohrRecord	pohr_rec;
struct	poliRecord	poli_rec;
struct	polnRecord	poln_rec;
struct	poshRecord	posh_rec;
struct	poslRecord	posl_rec;
struct	qchrRecord	qchr_rec;
struct	qclnRecord	qcln_rec;
struct	ryhrRecord	ryhr_rec;
struct	rymrRecord	rymr_rec;
struct	skndRecord 	sknd_rec;
struct	sknhRecord 	sknh_rec;
struct	skniRecord 	skni_rec;
struct	sobgRecord	sobg_rec;
struct	sohrRecord	sohr_rec;
struct	soktRecord	sokt_rec;
struct	solnRecord	soln_rec;
struct	sumrRecord	sumr_rec;
struct	suphRecord	suph_rec;
struct	trcgRecord	trcg_rec;
struct	trveRecord 	trve_rec;
struct	udidRecord	udid_rec;
struct	udihRecord	udih_rec;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
	double	*inpr_qty_brk 	=	&inpr_rec.qty_brk1;
	double	*inpr2_qty_brk 	=	&inpr2_rec.qty_brk1;
	Money	*inpr2_price 	=	&inpr2_rec.price1;
	Money	*incc_prf 		=	&incc_rec.c_prf_1;
	Money	*incc_val 		=	&incc_rec.c_val_1;
	Money	*incp_price 	=	&incp_rec.price1;

	char	rangeHeader [12][13];

#define	COST_TYPE	cost_desc [i].c_type
#define	COST_DESC	cost_desc [i].c_desc

	float	qtyReq 				= 0.00,
			totalReq 			= 0.00,
			totalRec 			= 0.00,
			totalRej 			= 0.00,
			safetyStock 		= 0.00,
			qtyAvail 			= 0.00,
			qcQty				= 0.00,
			woQtyAnti			= 0.00,
			onOrder				= 0.00,
			committed			= 0.00,
			backorder			= 0.00,
			forward				= 0.00,
			closingStock		= 0.00,
			bfclosingStock		= 0.00,
			transIn 			= 0.00,
			transOut 			= 0.00,
			minStockWks			= 0.00,
			realCommitted		= 0.00,
			o_tot_qty 			= 0.00,
			qtyOnHand [6] 		= {0.00,0.00,0.00,0.00,0.00,0.00},
			qtyCommit [6]  		= {0.00,0.00,0.00,0.00,0.00,0.00},
			qtyBackOrd [6] 		= {0.00,0.00,0.00,0.00,0.00,0.00},
			qtyForward [6]  	= {0.00,0.00,0.00,0.00,0.00,0.00},
			qtyOrder [6]   		= {0.00,0.00,0.00,0.00,0.00,0.00},
			avail [6]     		= {0.00,0.00,0.00,0.00,0.00,0.00};

	float	q_project [4][6];

	double	o_tot_val 			= 0.00,
			weeksDemand 		= 0.00;

	char	envVarGst 			[2],
			envVarCurrCode 		[4],
			envVarGstTaxName 	[4],
			envVarLrpMethods 	[5],
			headingText 		[180],
			saveKey 			[100],
			head_str 			[200],
			branchNumber 		[3],
			mlStkDisp 			[133][125],
			tempStr 			[8][30];

	int		envVarSkAlldispPop 	= 5,
			envVarCrCo 			= 0,
			envVarSkDbPriNum	= 9,
			envVarSkDbQtyNum	= 9,
			envVarIkeaSystem	= 0,
			envVarSkAlldispLocx = 0,
			envVarthreePlSystem = 0,
			saveIndex			= 0,
			hashPassed 			= FALSE,
			popupSelect			= IN_POP_A,
			mainWindowOpen 		= FALSE,
			displayCostOk 		= TRUE,
			tx_window			= 0,
			tx_lines			= 0,
			useMinStockWks		= FALSE,
			mdy [3]				= {0,0,0};

	char	*blankSerial = "                         ";
	char	*sixtySpace = "                                                            ";

	long	currentHhccHash = 0L,
			startMonth 		= 0L,
			hhbrPassed		= 0L;

	struct	{
		char	*c_type;
		char	*c_desc;
	} cost_desc [] = {
		{"L",	"Last    "},
		{"A",	"Average "},
		{"F",	"FIFO    "},
		{"I",	"LIFO    "},
		{"S",	"Serial  "},
		{"P",	"Previous"},
		{"T",	"Standard"},
		{"",	"Unknown "},
	};

	char	*std_foot = " [PRINT] [NEXT] [PREV] [EDIT/END] ";

/*============================
| Menu Callback Declarations |
============================*/
int AddRecalculate 			(void);
int BackOrderDisplay 		(void);
int BarCodeDisplay 			(void);
int	BatchHistoryDisplay		(void);
int BranchStatusDisplay 	(void);
int ClearRedraw 			(void);
int CompanyDisplay 			(void);
int DisplayPreQc 			(void);
int DisplayRelQc 			(void);
int DropShipmentDisplay 	(void);
int DspShipments 			(void);
int FifoDisplay 			(void);
int ForwardSchedule 		(void);
int GlobalSupplyWarehouse 	(void);
int Graph24Months 			(void);
int GraphSalesProfits 		(void);
int GraphThisLastYear 		(void);
int HeadingPrint 			(void);
int HistoryDisplay 			(void);
int InTransitDisplay 		(void);
int InexDisplay 			(void);
int InexMaint 				(void);
int ItemUserDefined 		(void);
int LocationDisplay 		(void);
int NoPlateDisplay 	 		(void);
int MendMoveDisp 			(void);
int MonthEndDisplay 		(void);
int MovementDisplay 		(void);
int NextRecordDisplay 		(void);
int NotePadDisplay 			(void);
int NotePadMaint 			(void);
int OutletInventory 		(void);
int PrevRecordDisplay 		(void);
int PriceDisplay 			(void);
int ProcessGraph 			(void);
int PurchaseHistoryDisplay 	(void);
int PurchaseOrderDisplay 	(void);
int ReorderReview 			(void);
int RequestForTransfers 	(void);
int RequisitionsBordDisplay (void);
int RequisitionsDisplay 	(void);
int RequisitionsForward 	(void);
int SalesOrderDisplay 		(void);
int SelectPopupA 			(void);
int SelectPopupB 			(void);
int SelectPopupC 			(void);
int SelectPopupD 			(void);
int SelectPopupE 			(void);
int SelectPopupF 			(void);
int SelectPopupG 			(void);
int SerialDisplay 			(void);
int SupplierDisplay 		(void);
int SupplyWarehouseDisplay 	(void);
int TransactionsAdj 		(void);
int TransactionsAll 		(void);
int TransactionsDD 			(void);
int TransactionsIC 			(void);
int TransactionsIR 			(void);
int TransactionsPur 		(void);
int UOM_display 			(void);
int UserDefined 			(void);
int WarehouseDisplay 		(void);
int WorkOrderComponent 		(void);
int WorkOrderManufactured 	(void);
int WorksOrderDisplay 		(void);
int RoyaltyDisplay 			(void);
int	FormulaStatistics		(void);
int	SpecialPrice			(void);

#ifndef GVISION
menu_type	_main_menu [] = {
{" ", "", AddRecalculate, "", CTRL ('A'), SELECT		  },
{"<S1>","Select display of Popup Window one.  [1]",
	SelectPopupA, "1",	  },
{"<S2>","Select display of Popup Window two.  [2]",
	SelectPopupB, "2",	  },
{"<S3>","Select display of Popup Window three.  [3]",
	SelectPopupC, "3",	  },
{"<S4>","Select display of Popup Window four.  [4]",
	SelectPopupD, "4",	  },
{"<S5>","Select Prices of Popup Window five.  [5]",
	SelectPopupE, "5",	  },
{"<S6>","Select Prices of Popup Window six.  [6]",
	SelectPopupF, "6",	  },
{"<Movements>", "Display Inventory Movements.  [M]",
	MovementDisplay, "Mm",	  },
{"<Branch Status>", "Display Branch Status.  [B] or [S]",
	BranchStatusDisplay, "SsBb",	  },
{"<MEND Movements>", "Display Month end Inventory Movements.  [M]",
	MendMoveDisp, "Mm",	  },
{"<UOM SOH>", "Display UOM stock on hand.  [U]",
	UOM_display, "Uu",	  },
{"<Standard Pricing>","Display Item Pricing.  [S] or [P]",   
	PriceDisplay, "SsPp",	  },                            
{"<Special Pricing>","Display Special Pricing.  [L] or [P]",   
	SpecialPrice, "LlPp",	  },                            
{"<Sales Orders>","Display Sales Orders.  [S] or [O]",
	SalesOrderDisplay, "SsOo",	  },
{"<Back Orders>", "Display Backorders.  [B]",
	BackOrderDisplay, "Bb",		  },
{"<Purchase Orders>", "Display Purchase Orders.  [P]",
	PurchaseOrderDisplay, "Pp",	  },
{"<Shipments>", "Display Shipments.  [S]",
	DspShipments, "Ss",	  },
{"<Purchase Orders DD>", "Display Purchase Orders for Direct Deliveries. [P]",
	DropShipmentDisplay, "PpDd",	  },
{"<Purchase History>", "Display Purchase Orders History.  [P]",
	PurchaseHistoryDisplay, "PpHh",	  },
{"<Supplier Records>","Display Supplier Records.  [S]",
	SupplierDisplay, "Ss",   },
{"<Supply Warehouses>","Display Supply Warehouses.  [S] or [W]",
	SupplyWarehouseDisplay, "SsWw",   },
{"<Global Supply Warehouses>","Display Global Warehouses.  [S] or [W]",
	GlobalSupplyWarehouse, "SsWw",   },
{"<Forward Schedule>", "Display Forward Schedule.  [F]",
	ForwardSchedule, "Ff",	  },
{"<Reorder Review>", "Display Reorder Review. [R]",
	ReorderReview, "Rr",	  },
{"<Formula Statistics>","Display Formula Statistics information. [F]",
	FormulaStatistics, "Ff",	  },
{"<Graph Demand>", "Display Reorder Demand. [G]",
	ProcessGraph, "Gg",	  },
{"<Graph-This/last year>","Grahical Display of This years Versus Last years Sales History.  [G]",
	GraphThisLastYear, "Gg",	  },
{"<Graph-24 Months>","Grahical Display of 24 months Sales History.  [G]",
	Graph24Months, "Gg",	  },
{"<Graph-Sales/Profits>","Grahical Display of Sales ver Profits.  [G]",
	GraphSalesProfits, "Gg",	  },
{"<In-Trans Stock>","Display Stock In Transit.  [I]",
	InTransitDisplay, "Ii",	  },
{"<Transfer Requests>","Display Stock Transfer Requests.  [R]",
	RequestForTransfers, "Rr",	  },
{"<FIFO Records>", "Display FIFO Records.  [F]",
	FifoDisplay, "Ff",	  },
{"<Location/Lot Records>", "Display Location/Lot Records.  [L]",
	LocationDisplay, "Ll",	  },
{"<Number Plate>", "Display Number Plate Details. [N]",
	NoPlateDisplay, "Nn",	  },
{"<Batch History>", "Display Lot History Records.  [B]",
	BatchHistoryDisplay, "Bb",	  },
{"<Serial Records>",	"Display Serial Numbers.  [S]",
	SerialDisplay, "Ss",	  },
{"<Item Note Pad>", "Display Inventory Item Internal Note Pad.  [N]",
	NotePadDisplay, "Nn",	  },
{"<Maintain Item Note Pad>", "Maintain Inventory Item Internal Note Pad.  [A]",
	NotePadMaint, "Aa",	  },
{"<Display Item Descriptions>", "Display Inventory Item Descriptions.  [D]",
	InexDisplay, "Dd",	  },
{"<Maintain Item Descriptions>", "Maintain Inventory Item Descriptions.  [D]",
	InexMaint, "Dd",	  },
{"<Display Bar Codes>", "Display Item Bar Code.  [B]",
	BarCodeDisplay, "Bb",	  },
{"<Item User Defined Fields>", "Display Item User Defined information. [I]",
	ItemUserDefined, "Ii",	  },
{"<User Defined Fields>", "Display User Defined information. [U]",
	UserDefined, "Uu",	  },
{"<Outlet Inventory>", "Display Outlet Inventory. [O]",
	OutletInventory, "Oo",	  },
{"<Royalty>", "Display Royalties.  [R]",
	RoyaltyDisplay, "Rr",	  },
{"<Trans - All>","Display All Inventory Transactions.   [T]",
	TransactionsAll,"Tt",	  },
{"<Trans - P/Orders>","Display Inventory transactions for P/orders. [T]",
	TransactionsPur, "Tt",	  },
{"<Trans - Iss/Rec>","Display Inventory transactions for Issues/Receipts. [T]",
	TransactionsIR, "Tt",	  },
{"<Trans - Inv/Crd>","Display Inventory transactions for Invoices/Credits. [T]",
	TransactionsIC, "Tt",	  },
{"<Trans - Stock Adj>","Display Inventory transactions for Stock Adjustments. [T]",
	TransactionsAdj, "Tt",	  },
{"<Trans - Dir-Del>","Display Inventory transactions for Direct Deliveries. [T]",
	TransactionsDD, "Tt",	  },
{"<Month Disp>","Display Inventory transactions by Month. [M]",
	MonthEndDisplay, "Mm",	  },
{"<Requisitions>","Display Requisitions.  [R]",
	RequisitionsDisplay, "Rr",	  },
{"<Backorder Requisitions>","Display Backorder Requisitions.  [B]",
	RequisitionsBordDisplay, "Bb",	  },
{"<Forward Requisitions>","Display Forward Requisitions.  [F]",
	RequisitionsForward, "Ff",	  },
{"<Works Orders>", "Displays Works Orders for this Manufactured Item and Where Used. [W]",
	WorksOrderDisplay, "Ww",	},
{"<W/O Mfg Item>", "Displays Works Orders for this Manufactured Item. [W] or [M]",
	WorkOrderManufactured, "WwMm",		},
{"<W/O Where Used>", "Display Works Orders that use this Component Item. [W] or [U]",
	WorkOrderComponent, "WwUu",		},
{"<Released QC Records>", "Display Released QC Retreival Records.  [Q]",
	DisplayRelQc, "Qq",    },
{"<Pre-Released QC Records>", "Display Pre-Released QC Retreival Records.  [Q]",
	DisplayPreQc, "Qq",    },
{"<Sales History>", "Display Sales History.  [S] or [H]",
	HistoryDisplay, "SsHh",	  },
{"<Company>",	"Display by Company.  [C]",
	CompanyDisplay, "Cc",	  },
{"<Warehouse>","Display by Warehouse.  [W]",
	WarehouseDisplay, "Ww",	  },
{" [REDRAW]","Redraw Display", ClearRedraw, "", FN3,			  },
{" [PRINT]", "Print Screen", HeadingPrint, "", FN5,			  },
{" [NEXT SCN]", "Display Next Item", NextRecordDisplay, "", FN14,		  },
{" [PREV SCN]", "Display Previous Item", PrevRecordDisplay, "", FN15,		  },
{" [EDIT/END]", "Exit Display", _no_option, "", FN16, EXIT | SELECT		  },
{"",									  },
};
#else
menu_group _main_group [] = {
	{1, "<S1> - <S7>"},
	{2, "Movements"},
	{3, "Pricing"},
	{4, "Orders"},
	{5, "Suppliers / Reorder"},
	{6, "Graphs"},
	{7, "Transfers"},
	{8, "FIFO/LIFO"},
	{9, "SERIAL"},
	{10,"Batch/Lots/Locations"},
	{11,"User Defined Information"},
	{12,"Extra Item Discription / Note Pad"},
	{13,"Number Plates"},
	{14,"Bar Codes"},
	{15,"Outlet Inventory"},
	{16,"Royalty"},
	{17, "Transactions"},
	{18, "Manufacturing"},
	{19, "History"},
	{20, "Standard Menus"},
	{0, ""}					// terminator item
};

menu_type	_main_menu [] = {
{0, " ","", AddRecalculate, CTRL ('A'), SELECT		  },
{1, "<S1>", "<S1>",	SelectPopupA },
{1, "<S2>", "<S2>",	SelectPopupB },
{1, "<S3>", "<S3>",	SelectPopupC },
{1, "<S4>", "<S4>",	SelectPopupD },
{1, "<S5>", "<S5>",	SelectPopupE },
{1, "<S6>", "<S6>", SelectPopupF },
{1, "<S7>", "<S7>", SelectPopupG },
{2, "<Movements>", "Inventory Movements", MovementDisplay },
{2, "<Branch Status>", "Branch Status", BranchStatusDisplay },
{2, "<MEND Movements>", "Month End Movements", MendMoveDisp	},
{2, "<UOM SOH>", "UOM Stock on Hand", UOM_display },
{2, "<Month Disp>", "Month End Transactions", MonthEndDisplay },
{3, "<Standard Pricing>", "Standard Pricing", PriceDisplay },  
{3, "<Special Pricing>", "Special Pricing", SpecialPrice },
{4, "<Sales Orders>", "Sales Orders", SalesOrderDisplay },
{4, "<Back Orders>", "Back Orders", BackOrderDisplay },
{4, "<Purchase Orders>", "Purchase Orders", PurchaseOrderDisplay },
{4, "<Purchase Orders DD>", "Direct Delivery Purchase Orders", DropShipmentDisplay },
{4, "<Purchase History>", "Purchase History", PurchaseHistoryDisplay },
{4, "<Shipments>", "Shipments", DspShipments, },
{5, "<Supplier Records>", "Supplier Records", SupplierDisplay },
{5, "<Supply Warehouses>", "Supply Warehouses", SupplyWarehouseDisplay	},
{5, "<Global Supply Warehouses>", "Global Supply Warehouses", GlobalSupplyWarehouse	},
{5, "<Forward Schedule>", "Forward Schedule", ForwardSchedule },
{5, "<Reorder Review>", "Reorder Review", ReorderReview	},
{6, "<Formula Statistics>", "Formula Statistics", FormulaStatistics	},
{6, "<Graph Demand>", "Graph Reorder Demand", ProcessGraph },
{6, "<Graph-This/last year>", "Graph-This/last year", GraphThisLastYear },
{6, "<Graph-24 Months>", "Graph-24 Months",	Graph24Months },
{6, "<Graph-Sales/Profits>", "Graph-Sales/Profits",	GraphSalesProfits },
{7, "<In-Trans Stock>", "In-Trans Stock", InTransitDisplay },
{7, "<Transfer Requests>", "Transfer Requests", RequestForTransfers	},
{8, "<FIFO Records>", "FIFO/LIFO Records", FifoDisplay },
{9, "<Serial Records>", "Serial Records", SerialDisplay },
{10, "<Location/Lot Records>", "Location/Lot Records", LocationDisplay },
{10, "<Batch History>", "Batch History", BatchHistoryDisplay },
{11, "<Item User Defined Fields>", "Item User Defined Fields", ItemUserDefined },
{11, "<User Defined Fields>", "User Defined Fields", UserDefined },
{12, "<Item Note Pad>", "Item Note Pad", NotePadDisplay	},
{12, "<Maintain Item Note Pad>", "Maintain Item Note Pad", NotePadMaint	}, 
{12, "<Display Item Descriptions>", "Display Item Descriptions", InexDisplay },
{12, "<Maintain Item Descriptions>", "Maintain Item Descriptions", InexMaint }, 
{13, "<Number Plate>", "Number Plate", NoPlateDisplay },
{14, "<Display Bar Codes>", "Display Bar Codes", BarCodeDisplay	}, 
{15, "<Outlet Inventory>", "Outlet Inventory", OutletInventory },
{16, "<Royalty>", "Royalty", RoyaltyDisplay	}, 
{17, "<Trans - All>", "All Transactions - All",	TransactionsAll	}, 
{17, "<Trans - P/Orders>", "Purchases",	TransactionsPur	}, 
{17, "<Trans - Iss/Rec>", "Transfers Issues / Receipts", TransactionsIR	},
{17, "<Trans - Inv/Crd>", "Invoices / Credits",	TransactionsIC },
{17, "<Trans - Stock Adj>", "Stock Adjustments", TransactionsAdj },
{17, "<Trans - Dir-Del>", "Direct Deliveries", TransactionsDD },
{18, "<Requisitions>", "Requisitions", RequisitionsDisplay },
{18, "<Backorder Requisitions>", "Backorder Requisitions", RequisitionsBordDisplay },
{18, "<Forward Requisitions>", "Forward Requisitions", RequisitionsForward },
{18, "<Works Orders>", "Works Orders - All", WorksOrderDisplay },
{18, "<W/O Mfg Item>", "Works Orders Manufactured Item", WorkOrderManufactured },
{18, "<W/O Where Used>", "Works Orders Where Used", WorkOrderComponent },
{18, "<Released QC Records>", "Released QC Records", DisplayRelQc },
{18, "<Pre-Released QC Records>", "Pre-Released QC Records", DisplayPreQc },
{19, "<Sales History>", "Sales History", HistoryDisplay },
{20, "<Company>", "Company", CompanyDisplay	},
{20, "<Warehouse>", "Warehouse", WarehouseDisplay },
{20, " [REDRAW]","[REDRAW]", ClearRedraw, FN3 },
{20, " [PRINT]","[PRINT]", HeadingPrint, FN5 },
{20, " [NEXT SCN]","[NEXT SCN]", NextRecordDisplay, FN14 },
{20, " [PREV SCN]","[PREV SCN]", PrevRecordDisplay, FN15 },
{0, "",									  },   // terminator item
};
#endif

/*---------------------------------------------------------------
| The two options above have not been upgraded with the changes |
| required between version 7 and version 9.                     |
| The changes are as follows :                                  |
|		DPL  9670 - added 3 new options to do with the display  |
|			 		of the works orders.                        |
|		DPL  9667 - display of upto 6 decimal places.           |
|		DPL 10100 - Modified forward schedules, to account      |
|					for manufacturing.                          |
|		PSL 10366 - change to wide screen.                      |
---------------------------------------------------------------*/

	char	*month_nm [12];

	extern	int		lp_x_off,
					lp_y_off;

	int		envVarSoFwdAvl	= FALSE;
	int		envVarSkQcAvl 	= FALSE;
	int		envVarQcApply 	= FALSE;
	int		wh_flag 	= TRUE;
	int		ff_flag		= FALSE;
	int		envVarExMatch	= FALSE;
	int		curr_month	= 0;
	int		display_ok 	= TRUE;
	int		clear_ok 	= TRUE;
	int		envVarDbMcurr 		= FALSE;
	int		envVarSkCusPriLvl 	= 0;
	int		SK_DISPSEL 	= FALSE;

	char	disp_str [350];
	char	envVarPoActCost [2];
	float	envVarLrpDfltReview;
	float	tot_qty;
	float	con [12];
	float	lcon [12];
	double	val [12];
	double	prf [12];
	double	tot_amount;
	double	cost_amt;

	float	StdCnvFct	=	1.00,
			PurCnvFct	=	1.00,
			CnvFct		=	1.00;

	static	struct	date_rec 
	{
		long	StartDate;
		long	EndDate;
		float	QtySold;
	} store_dates [37];

#include	<RealCommit.h>
#include	<Costing.h>
#include	<get_lpno.h>
#include 	<ser_value.h>

/*============
 Table names
=============*/
static char
	*data	= "data",
	*ccmr2	= "ccmr2",
	*inmr2	= "inmr2",
	*inmr3	= "inmr3",
	*inmr4	= "inmr4";

#define		BY_CO	0
#define		BY_BR	1
#define		BY_WH	2
#include	<LRPFunctions.h>

struct {
		char 	dummy [11];
		char 	prev_item [17];
		char	cost_type [21];
		char	ex_desc [50];
		double	l_cost;
		double	avge_cost;
		double	last_cost;
		double	prev_cost;
		double	std_cost;
		double	cost_price;
		float	available;
		float	mtd_sales;
		float	ytd_sales;
		float	one_ysales;
		float	two_ysales;
		char	dflt_qty [15];
		char	rep_qty [10];
		char	prev_cat [12];
		char	desc [41];
		char	s_class [2],
				e_class [2],
				s_cat [12],
				s_cat_desc [41],
				e_cat [12],
				e_cat_desc [41];
		char	s_item [17],
				e_item [17],
				s_item_desc [41],
				e_item_desc [41];
		char	s_desc1 [41],
				s_desc2 [41];
		char	sMthYr [11];
		char	eMthYr [11];
		char	priceDesc [9][16];
		long	sDate;
		long	eDate;
		long	st_date;
		float	demand_value [MAX_MONTHS];
} local_rec;

static	struct	var	vars [] =
{
	/*-------------------
	| By Class-Category	|
	-------------------*/
	{1, LIN, "s_class",	 3, 22, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class       :", "Input Start Class A-Z.",
		YES, NO,  JUSTLEFT, "A", "Z", local_rec.s_class},
	{1, LIN, "s_cat",	 4, 22, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category    :", "Input Start Inventory Category. [SEARCH] Available.",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_cat},
	{1, LIN, "s_cat_desc",	 4, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_cat_desc},
	{1, LIN, "e_class",	 5, 22, CHARTYPE,
		"U", "          ",
		" ", "Z", "End Class         :", "Input End Class A-Z.",
		YES, NO,  JUSTLEFT, "A", "Z", local_rec.e_class},
	{1, LIN, "e_cat",	6, 22, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "~", "End Category      :", "Input End Inventory Category. [SEARCH] Available.",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_cat},
	{1, LIN, "e_cat_desc",	6, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_cat_desc},

	/*-------------------
	| By Product        |
	-------------------*/
	{1, LIN, "s_item",	 8, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item        :", "Input Start Item Number. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_item},
	{1, LIN, "s_item_desc",	 8, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_item_desc},
	{1, LIN, "e_item",	 9, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~", "End  Item         :", "Input End Item Number. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_item},
	{1, LIN, "e_item_desc",	 9, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_item_desc},
	{1, LIN, "srch1",	 11, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  "Desc Search #1    :", "Wild card search is available.",
		NO, NO,  JUSTLEFT, "", "", local_rec.s_desc1},
	{1, LIN, "srch2",	 12, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  "Desc Search #2    :", "Wild card search is available.",
		NO, NO,  JUSTLEFT, "", "", local_rec.s_desc2},
	{2, LIN, "sMthYr",	 14, 52, CHARTYPE,
		"UU/UUUU", "          ",
		" ", "0", "Start Month/Year :", "Enter Month and Year for Start of Display- Default is current month ",
		ND, NO,  JUSTLEFT, "0123456789/", "", local_rec.sMthYr},
	{2, LIN, "eMthYr",	 15, 52, CHARTYPE,
		"UU/UUUU", "          ",
		" ", " ", "End   Month/Year :", "Enter Month and Year for End of Display - Default is current Month ",
		ND, NO,  JUSTLEFT, "0123456789/", "", local_rec.eMthYr},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


float	transportDespatch	=	0.00,
		transportReturns	=	0.00;

long	hhsiHash			=	0L;

#include	<FindInmr.h>
#include	<number.h>
#include    <chk_ring_sec.h>

/*=======================
| Function Declarations |
=======================*/
char 	*InternalMonthName 	(int);
char 	*LocStatDisplay  	(char *);
float 	CalcMendStk 		(long);
float 	CalcOrders 			(long);
float 	CalculateYtdValues 	(void);
float 	GetLeadDate		 	(long, long);
float 	GetReviewPeriod 	(long, char *);
float 	GetShipQty 			(void);
float 	GetUom 				(long);
float 	SK_CalcAlloc 		(long);
float 	pwr 				(float, int);
int  	BomDisplay 			(int);
int  	CheckColn 			(long);
int  	DispQtyBreaks 		(char *);
int  	Disp_save 			(char *);
int  	Dsp_heading 		(void);
int  	GetPeriodMonth 		(long);
int  	GetWarehouse 		(long);
int  	Heading 			(int);
int  	InvoiceEnquiry 		(char *);
int  	ShipmentEnquiry 	(char *);
int  	ManufactDisplay 	(int);
int  	ProcessPohr 		(long, float, double, long, int);
int  	PurchaseShipment 	(char *);
int  	SerialEnquiry 		(char *);
int  	StockEnquiry 		(char *);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	AllDisplay 			(void);
void 	AlternateMess 		(char *);
void 	CalcDates 			(long);
void 	ChangeData 			(int);
void 	CloseDB 			(void);
void 	DeleteMonthArray 	(void);
void 	DspStockTag 		(void);
void 	FindCost 			(void);
void 	GetItemNo 			(void);
void 	GetTransfers 		(int);
void 	InitML 				(void);
void 	InitMonthArray 		(void);
void 	InternalFindSuper 	(char *);
void 	LoadCompanyData 	(void);
void 	LoadCompanySerial 	(char *, char *);
void 	LoadData 			(void);
void 	LoadHistory 		(long, long, long, char *);
void 	LoadWarehouseData 	(void);
void 	LoadWarehouseSerial (char *, char *);
void 	NormalRedraw 		(void);
void 	OpenDB 				(void);
void 	OrderDisplay 		(int);
void 	PrintFifo 			(long, double, float, char *, double, double);
void 	PrintPopup 			(int);
void 	PrintPopupStuff 	(void);
void 	ProcessOrderHeader 	(long, float, double, long, int, int);
void 	ProcessPhantom 		(long, int);
void 	ProcessRequisition 	(char *);
void 	ProcessSynonym 		(long, int);
void 	ProcessTransactions (long, int);
void 	ProcessTransferLines(long);
void 	ReadCcmr 			(void);
void 	RunSDisplay 		(char *, char *, char *, long, char *, int);
void 	SetMonthName 		(int, char *);
void 	SillyBusyFunction	(int);
void 	SrchExcf 			(char *);
void 	ProcessInsf			(long, char *, char *);
void 	TransDisplay 		(char *);
void 	Transport 			(long);
void 	WildCardMess 		(void);
void 	WsFindInpr 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		first_time = TRUE,
			before,
			after;
	char	*sptr;

	lp_x_off	= 2;
	lp_y_off	= 0;

	SETUP_SCR (vars);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strncmp (sptr, "sk_dispsel", 10))
		SK_DISPSEL = TRUE;
	else
		SK_DISPSEL = FALSE;
		
	if (!strncmp (sptr, "sk_resdisp", 10))
		displayCostOk = FALSE;
	else
		displayCostOk = TRUE;

	sptr = chk_env ("SK_CUSPRI_LVL");
	if (sptr == (char *)0)
		envVarSkCusPriLvl = 0;
	else
		envVarSkCusPriLvl = atoi (sptr);

	sptr = chk_env ("IKEA_SYSTEM");
	envVarIkeaSystem	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_ALLDISP_POP");
	if (sptr == (char *)0)
		envVarSkAlldispPop = 5;
	else
	{
		envVarSkAlldispPop = atoi (sptr);
		if (envVarSkAlldispPop < 0 || envVarSkAlldispPop > 6)
			envVarSkAlldispPop = 6;
	}
	popupSelect	=	envVarSkAlldispPop;

	sptr = chk_env ("SK_DBQTYNUM");
	if (sptr == (char *)0)
		envVarSkDbQtyNum = 0;
	else
	{
		envVarSkDbQtyNum = atoi (sptr);
		if (envVarSkDbQtyNum > 9 || envVarSkDbQtyNum < 0)
			envVarSkDbQtyNum = 9;
	}

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envVarSkDbPriNum = 5;
	else
	{
		envVarSkDbPriNum = atoi (sptr);
		if (envVarSkDbPriNum > 9 || envVarSkDbPriNum < 1)
			envVarSkDbPriNum = 9;
	}

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*------------------
	| Get G.S.T. Code. |
	------------------*/
	sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));

	sptr = get_env ("LRP_METHODS");
	if (sptr == (char *) NULL)
		strcpy (envVarLrpMethods,"ABCD");
	else
		sprintf (envVarLrpMethods,"%-4.4s", sptr);
	
	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarthreePlSystem = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc >= 2)
		wh_flag = (argv [1][0] == 'C') ? FALSE : TRUE;

	if (argc == 3)
	{
		hashPassed = TRUE;
		hhbrPassed = atol (argv [2]);
	}

	sptr = chk_env ("SO_FWD_AVL");
	envVarSoFwdAvl = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SK_ALLDISP_LOCX");
	envVarSkAlldispLocx = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/* Whether to include QC qty in available stock. */
	envVarSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;
	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	sptr = chk_env ("LRP_DFLT_REVIEW");
	if (sptr == (char *)0)
		envVarLrpDfltReview = 4.00;
	else
		envVarLrpDfltReview = atof (sptr);

	sprintf (envVarGst, "%-1.1s",   get_env ("GST"));
	sprintf (envVarPoActCost, "%1.1s",    get_env ("PO_ACT_COST"));

	sptr = chk_env ("EX_MATCH");
	envVarExMatch = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNN.NNNNN");
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

	input_row = 20,
	error_line = 20;

	tab_row = 5;
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	LSA_open ();
	ReadCcmr ();
	InitML ();

	if (wh_flag)
		ff_flag = setup_LSA (BY_WH, 
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no);
	else
		ff_flag = setup_LSA (BY_CO,
			comm_rec.co_no,
			"  ",
			"  ");

	swide ();
	_chk_ring_sec (_main_menu, "sk_alldisp");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	sprintf (local_rec.prev_item, "%16s", " ");

	if (SK_DISPSEL)
	{
		while (prog_exit == 0)	
		{
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;
			init_vars (1);

			FLD ("s_class") = YES;
			FLD ("s_cat") 	= YES;
			FLD ("e_class") = YES;
			FLD ("e_cat") 	= YES;
			FLD ("srch1") 	= YES;
			FLD ("srch2") 	= YES;


			heading (1);
			entry (1);
				
			if (restart || prog_exit)
				continue;

			heading (1);
			scn_display (1);
			edit (1);

			if (restart)
				continue;
			else
				DspStockTag ();
		}
	}
	else
	{
		while (prog_exit == 0)
		{
			crsr_off ();
			entry_exit = 0;
			prog_exit = 0;
			restart = 0;
			display_ok = FALSE;
			clear_ok = TRUE;
			search_ok = 1;
			wh_flag = TRUE;

			if (first_time)
			{
				Heading (1);
				first_time = FALSE;
			}
			else
			{
				move (0, 20);
				cl_line ();
				move (0, 21);
				cl_line ();
			}

			GetItemNo ();
			if (prog_exit || restart)
				continue;

			display_ok = TRUE;
			clear_ok = FALSE;
			Heading (1);
#ifndef GVISION			
			run_menu (_main_menu, "", input_row);
#else
			run_menu (_main_group, _main_menu);
#endif

			strcpy (local_rec.prev_item, inmr_rec.item_no);
		}
	}

	/*=======================================
	| Program exit sequence.		|
	=======================================*/
	CloseDB (); 
	FinishProgram ();
    DeleteMonthArray ();
	return (EXIT_SUCCESS);
}

void
ReadCcmr (void)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	currentHhccHash = ccmr_rec.hhcc_hash;

	DateToDMY (comm_rec.inv_date, &mdy [0], &mdy [1], &mdy [2]);
	startMonth = mdy [1];
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);
	abc_alias (inmr3, inmr);
	abc_alias (inmr4, inmr);
	abc_alias (ccmr2, ccmr);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_id_no");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr4, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_id_no");
	open_rec (inme, inme_list, INME_NO_FIELDS, "inme_hhwh_hash");

	strcpy (local_rec.priceDesc [0], comm_rec.price1_desc);
	strcpy (local_rec.priceDesc [1], comm_rec.price2_desc);
	strcpy (local_rec.priceDesc [2], comm_rec.price3_desc);
	strcpy (local_rec.priceDesc [3], comm_rec.price4_desc);
	strcpy (local_rec.priceDesc [4], comm_rec.price5_desc);
	strcpy (local_rec.priceDesc [5], comm_rec.price6_desc);
	strcpy (local_rec.priceDesc [6], comm_rec.price7_desc);
	strcpy (local_rec.priceDesc [7], comm_rec.price8_desc);
	strcpy (local_rec.priceDesc [8], comm_rec.price9_desc);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	if (mainWindowOpen)
		Dsp_close ();

	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (inld);
	abc_fclose (inmr);
	abc_fclose (inpr);
	abc_fclose (intr);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (inmr2);
	abc_fclose (soic);
	abc_fclose (inum);
	abc_fclose (inex);
	abc_fclose (inlo);
	abc_fclose (inla);
	abc_fclose (inme);
	LSA_close ();
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		smdy [3],
			emdy [3],
			eMth;
	char	tmpMth [3];

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("s_cat"))
	{
		FLD ("e_class")	= YES;
		FLD ("e_cat") 	= YES;

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.s_cat);

		if (dflt_used)
		{
			sprintf (local_rec.s_cat,"%-11.11s","           ");
			sprintf (local_rec.s_cat_desc,"%-40.40s",mlStkDisp [70]);
			sprintf (local_rec.e_cat,"%-11.11s","~~~~~~~~~~~");
			sprintf (local_rec.e_cat_desc,"%-40.40s",mlStkDisp [26]);
			strcpy (local_rec.e_class, "Z");
			DSP_FLD ("e_class");
			DSP_FLD ("e_cat");
			DSP_FLD ("e_cat_desc");
			DSP_FLD ("s_cat_desc");
			FLD ("e_class") = NI;
			FLD ("e_cat") 	= NI;
			return (EXIT_SUCCESS);
		}
		cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (prog_status != ENTRY && strcmp (local_rec.s_cat,local_rec.e_cat)> 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.s_cat_desc,"%-40.40s",excf_rec.cat_desc);
		DSP_FLD ("s_cat_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("e_cat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.e_cat);
		
		if (dflt_used)
		{
			sprintf (local_rec.e_cat,"%-11.11s","~~~~~~~~~~~");
			sprintf (local_rec.e_cat_desc,"%-40.40s",mlStkDisp [26]);
			DSP_FLD ("e_cat_desc");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.s_cat,local_rec.e_cat) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.e_cat_desc,excf_rec.cat_desc);
		DSP_FLD ("e_cat_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Item Number . |
	------------------------*/
	if (LCHECK ("s_item"))
	{
		FLD ("e_item") = YES;
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.s_item,"%-16.16s","                ");
			sprintf (inmr_rec.description,"%-40.40s",mlStkDisp [70]);
			sprintf (local_rec.s_item_desc,  "%-40.40s",mlStkDisp [70]);
			sprintf (local_rec.e_item,"%-16.16s","~~~~~~~~~~~~~~~~");
			sprintf (inmr_rec.description,"%-40.40s",mlStkDisp [26]);
			sprintf (local_rec.e_item_desc,"%-40.40s",mlStkDisp [26]);
			FLD ("e_item") = NI;
			DSP_FLD ("e_item");
			DSP_FLD ("e_item_desc");
			DSP_FLD ("s_item");
			DSP_FLD ("s_item_desc");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.s_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.s_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		strcpy (local_rec.s_item, inmr_rec.item_no);

		DSP_FLD ("s_item");
		DSP_FLD ("s_item_desc");
		
		if (prog_status != ENTRY && strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.s_item_desc,inmr_rec.description);
		DSP_FLD ("s_item_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			sprintf (local_rec.e_item,"%-16.16s","~~~~~~~~~~~~~~~~");
			sprintf (inmr_rec.description,"%-40.40s",mlStkDisp [26]);
			sprintf (local_rec.e_item_desc,"%-40.40s",mlStkDisp [26]);
			DSP_FLD ("e_item");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.e_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.e_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.e_item, inmr_rec.item_no);
	
		DSP_FLD ("e_item");

		if (strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.e_item_desc,inmr_rec.description);
		DSP_FLD ("e_item_desc");
	
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("srch1"))
	{
		if (dflt_used)
			FLD ("srch2") = NI;
		else
			FLD ("srch2") = YES;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sMthYr"))
	{
		DateToDMY (comm_rec.inv_date, NULL, &mdy [1], &mdy [2]);
		
		if (dflt_used)
			sprintf (local_rec.sMthYr, "%02d/%04d", mdy [1], mdy [2]);

		sprintf (tmpMth, "%2.2s", local_rec.sMthYr);
        smdy [0] = 1;
		smdy [1] = atoi (tmpMth);
		smdy [2] = atoi (local_rec.sMthYr + 3);

		if (smdy [1] < 1 || smdy [1] > 12)
		{
			print_mess (ML ("Start Month Must Be Between 1 and 12"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.sDate = DMYToDate (smdy [0], smdy [1], smdy [2]);

		if (prog_status != ENTRY &&
			local_rec.sDate > local_rec.eDate)
		{
			print_mess ("Start Date Must Be Less Than End Date ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eMthYr"))
	{
		if (dflt_used)
			sprintf (local_rec.eMthYr, "%02d/%04d", mdy [1], mdy [2]);

		sprintf (tmpMth, "%2.2s", local_rec.sMthYr);
		eMth = atoi (tmpMth);
		if (eMth < 1 || eMth > 12)
		{
			print_mess (ML ("End Month Must Be Between 1 and 12"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		emdy [0] = 1;
		emdy [1] = atoi (tmpMth);
		emdy [2] = atoi (local_rec.eMthYr + 3);
		
		local_rec.eDate = DMYToDate (emdy [0], emdy [1], emdy [2]);
		local_rec.eDate = MonthEnd (local_rec.eDate);

		if (local_rec.eDate < local_rec.sDate)
		{
			print_mess (ML ("End Date Must Be Greater Than Start Date"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
WildCardMess (void)
{
	char	*curr_user;

	curr_user = getenv ("LOGNAME");

	box (38,14,54,5);

	print_at (15,39, " %-50.50s", ML (mlSkMess248));
	print_at (16,39, " %-50.50s", ML (mlSkMess249));
	print_at (17,39, " %-50.50s", ML (mlSkMess250));
	print_at (18,39, " %-50.50s", ML (mlSkMess251));
	sprintf (err_str, ML (mlSkMess252), toupper (curr_user [0]),curr_user + 1);
	print_at (19,39, " %-50.50s", err_str);
}

void
GetItemNo (void)
{
	char	wk_part [17];

	last_char = 0;

	while (TRUE)
	{
		if (hashPassed)
		{
			hashPassed = FALSE;
			abc_selfield (inmr, "inmr_hhbr_hash");
			inmr_rec.hhbr_hash = hhbrPassed;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
			{
				crsr_on ();
				print_at (20, 3,ML (mlSkMess253));
				getalpha (20, 20, "UUUUUUUUUUUUUUUU", (char *) wk_part);
			}
			else
				strcpy (wk_part, inmr_rec.item_no);
			abc_selfield (inmr, "inmr_id_no");
		}
		else
		{
			crsr_on ();
			print_at (20, 3,ML (mlSkMess253));
			getalpha (20, 20, "UUUUUUUUUUUUUUUU", (char *) wk_part);
		}
		sprintf (inmr_rec.item_no, "%-16.16s", wk_part);

		crsr_off ();
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return;
		}
		if (last_char == FN1)
		{
			restart = TRUE;
			return;
		}

		if (last_char == FN_HELP || last_char == HELP ||
		     last_char == HELP1 || last_char == EXIT_OUT ||
		     (last_char > FN16 && last_char <= FN32))
		{
			shell_prog (last_char);
            if (prog_exit) break;
			Heading (1);
			continue;
		}
		if (last_char == REDRAW)
		{
			Heading (1);
			continue;
		}
		if (SRCH_KEY)
		{
			/*-------------------
			| by company search	|
			-------------------*/
			strcpy (temp_str, wk_part);
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			strcpy (wk_part, clip (temp_str));
			Heading (1);
			continue;
		}
		/*-----------
		| find item	|
		-----------*/
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (wk_part, inmr_rec.item_no);
		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		/*-------------
		| find failed |
		-------------*/
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			abc_selfield (inmr, "inmr_id_no");
			continue;
		}
		if (strcmp (wk_part, inmr_rec.item_no))
		{
		}

		strcpy (wk_part, inmr_rec.item_no);
		/*-----------------------------
		| load the data appropriately |
		-----------------------------*/
		LoadData ();
		return ;
	}
}

/*===================
| display next item |
===================*/
int
NextRecordDisplay (void)
{
	ChangeData (FN14);
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
ClearRedraw (void)
{
	clear ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
Dsp_heading (void)
{
	int		main_check = mainWindowOpen;

#ifndef GVISION
	mainWindowOpen = FALSE;
	NormalRedraw ();
	if (main_check)
		Dsp_close ();
	mainWindowOpen = main_check;
#endif	/* GVISION */
    return (EXIT_SUCCESS);
}

/*=======================================
| display prevoius item					|
=======================================*/
int
PrevRecordDisplay (void)
{
	ChangeData (FN15);
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

void
ChangeData (
 int key)
{
	char	last_item [17];
	/*---------------------------------------
	| perform find in appropriate direction	|
	---------------------------------------*/
	strcpy (last_item, inmr_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, (key == FN14) ? GTEQ : LTEQ, "r");
	if (!cc)
		cc = find_rec (inmr, &inmr_rec, (key == FN14) ? NEXT : PREVIOUS, "r");
	if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		AlternateMess (ML (mlStdMess245));
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, last_item);
		cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	}
	strcpy (local_rec.prev_item, inmr_rec.item_no);
	LoadData ();
}

/*=======================================
| display data by company		        |
=======================================*/
int
CompanyDisplay (void)
{
	/*---------------------------------------
	| by company already selected		    |
	---------------------------------------*/
	if (wh_flag)
	{
		wh_flag = FALSE;
		ff_flag = setup_LSA (BY_CO,
			comm_rec.co_no,
			"  ",
			"  ");
		LoadData ();
		NormalRedraw ();
	}
    return (EXIT_SUCCESS);
}

/*=======================================
| display data by warehouse		        |
=======================================*/
int
WarehouseDisplay (void)
{
	/*---------------------------------------
	| by warehouse already selected		    |
	---------------------------------------*/
	if (!wh_flag)
	{
		wh_flag = TRUE;
		ff_flag = setup_LSA (BY_WH,
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no);
		LoadData ();
		NormalRedraw ();
	}
    return (EXIT_SUCCESS);
}

/*=========================
| Find pricing structure. |
=========================*/
void
WsFindInpr (
 int price_type)
{
	int		i;

	inpr2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inpr2_rec.price_type = price_type + 1;
	inpr2_rec.hhgu_hash = 0L;
	strcpy (inpr2_rec.curr_code, envVarCurrCode);

	if (!envVarSkCusPriLvl)
	{
		strcpy (inpr2_rec.br_no, "  ");
		strcpy (inpr2_rec.wh_no, "  ");
	}
	else 
	{
		strcpy (inpr2_rec.br_no, comm_rec.est_no);
		strcpy (inpr2_rec.wh_no, (envVarSkCusPriLvl == 2) ? comm_rec.cc_no : "  ");
	}
	
	strcpy (inpr2_rec.area_code, "  ");
	strcpy (inpr2_rec.cust_type, "   ");
	cc = find_rec (inpr, &inpr2_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (inpr2_rec.br_no,comm_rec.est_no);
		strcpy (inpr2_rec.wh_no,"  ");
		cc = find_rec (inpr, &inpr2_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (inpr2_rec.br_no,"  ");
			strcpy (inpr2_rec.wh_no,"  ");
		}
		cc = find_rec (inpr, &inpr2_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (inpr2_rec.br_no, "  ");
			strcpy (inpr2_rec.wh_no, "  ");
			inpr2_rec.base     = 0.00;
			for (i = 0; i < 9; i++)
			{
				inpr2_qty_brk [i] = 0.00;
				inpr2_price [i]   = 0.00;
			}
		}
	}
}
int
DispQtyBreaks (
 char	*price)
{
	char		wsWork [22];
	int			j;
	int			wsDspCol;
	int			wsPrice;

	if (envVarSkDbQtyNum == 0)
		return (EXIT_SUCCESS);

	wsPrice = atoi (price);
	WsFindInpr (wsPrice);


	/* sprintf (err_str, " Price Breaks by %-8.8s   ",
					inpr2_rec.price_by [0] == 'V' ?
					"Value   " : "Quantity"); */

	sprintf (err_str, " %s %-8.8s   ",	mlStkDisp [111],
					inpr2_rec.price_by [0] == 'V' ?	
					mlStkDisp [112] : mlStkDisp [113]);

	for (j = 0; j < envVarSkDbQtyNum; j++)
	{
		sprintf (wsWork, "|  %s %1d ", mlStkDisp [114], j + 1);
		strcat (err_str, wsWork);
	}
	if (envVarSkDbQtyNum > 4)
		wsDspCol = (int) (((WIDESCREEN - (int) strlen (err_str)) / 2) - 1);
	else
		wsDspCol = (int) (((SCREENWIDTH - (int) strlen (err_str)) / 2) - 1);

	Dsp_open (wsDspCol, 5, 1);
	Dsp_saverec (err_str);

	/* sprintf (err_str, " Price Type      |    Base  "); */
	sprintf (err_str, " %-15.15s | %-8.8s ",	mlStkDisp [115], mlStkDisp [116]);
	for (j = 0; j < envVarSkDbQtyNum; j++)
	{
		sprintf (wsWork, "|%10.2f", inpr2_qty_brk [j]);
		strcat (err_str, wsWork);
	}
	Dsp_saverec (err_str);
	Dsp_saverec (" [EDIT / END] ");

	if (wsPrice < envVarSkDbPriNum)
		sprintf (err_str, " %-16.16s^E%10.2f", local_rec.priceDesc [wsPrice],
												DOLLARS (inpr2_rec.base));
	else
		sprintf (err_str, "                 ^E          ");
	for (j = 0; j < envVarSkDbQtyNum; j++)
	{
		if (wsPrice < envVarSkDbPriNum)
			sprintf (wsWork, "^E%10.2f", DOLLARS (inpr2_price [j]));
		else
			sprintf (wsWork, "           ");
		strcat (err_str, wsWork);
	}
	Dsp_saverec (err_str);
	Dsp_srch ();
	Dsp_close ();
	return (EXIT_SUCCESS);
}

int
PriceDisplay (void)
{
	char		wsWork [22];
	int			i, j;
	int			wsDspCol;
	char		price [2];
	char		wsHdr [160];

	sprintf (wsHdr, " %-16.16s | %-8.8s ",	mlStkDisp [115], mlStkDisp [116]);
	for (j = 0; j < envVarSkDbQtyNum; j++)
	{
		sprintf (wsWork, "| %-6.6s %1d ", mlStkDisp [114], j + 1);
		strcat (wsHdr, wsWork);
	}
	if (envVarSkDbQtyNum > 4)
		wsDspCol = (int) (((WIDESCREEN - (int) strlen (wsHdr)) / 2) - 1);
	else
		wsDspCol = (int) (((SCREENWIDTH - (int) strlen (wsHdr)) / 2) - 1);

	Dsp_open (wsDspCol, 4, 9);
	Dsp_saverec (wsHdr);
	Dsp_saverec ("");
	Dsp_saverec (" [EDIT / END] ");

	for (i = 0; i < 9; i++)
	{
		WsFindInpr (i);
		if (i < envVarSkDbPriNum)
			sprintf (err_str, " %-16.16s ^E%10.2f", local_rec.priceDesc [i],
													DOLLARS (inpr2_rec.base));
		else
			sprintf (err_str, "                              ");
		for (j = 0; j < envVarSkDbQtyNum; j++)
		{
			if (i < envVarSkDbPriNum)
				sprintf (wsWork, "^E%10.2f", DOLLARS (inpr2_price [j]));
			else
				sprintf (wsWork, "           ");
			strcat (err_str, wsWork);
		}
		sprintf (price, "%1d", i);
		if (i < envVarSkDbPriNum)
			Dsp_save_fn (err_str, price);
		else
			Dsp_save_fn (err_str, (char *) 0);
	}
	Dsp_srch_fn (DispQtyBreaks);
	Dsp_close ();
	display_ok = TRUE;
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

void
LoadData (void)
{
	int		i;
	/*-------------------------------
	| clear extra description		|
	-------------------------------*/
	move (2, 1);
	cl_line ();
	/*---------------------------
	| find supercession			|
	---------------------------*/
	sprintf (inmr2_rec.item_no, "%-16.16s", " ");
	sprintf (inmr2_rec.description, "%-40.40s", " ");
	InternalFindSuper (inmr_rec.supercession);

	hhsiHash	=	alt_hash
					(
						inmr_rec.hhbr_hash,
						inmr_rec.hhsi_hash
					);
	/*-----------------------------------
	| get costing type description		|
	-----------------------------------*/
	for (i = 0;strlen (COST_TYPE);i++)
	{
		if (inmr_rec.costing_flag [0] == COST_TYPE [0])
		{
			strcpy (local_rec.cost_type, COST_DESC);
			break;
		}
	}
	/*----------------------
	| unknown costing type |
	----------------------*/
	if (!strlen (COST_TYPE))
		strcpy (local_rec.cost_type, COST_DESC);

	/*--------------------
	| Get active status. |
	--------------------*/
	open_rec (inas, inas_list, INAS_NO_FIELDS, "inas_id_no");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	sprintf (inas_rec.act_code, "%-1.1s", inmr_rec.active_status);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		sprintf (inas_rec.description, "%-40.40s", mlStkDisp [4]);
	abc_fclose (inas);
	/*-----------------------------------
	| load company or warehouse details	|
	-----------------------------------*/
	if (wh_flag == FALSE)
		LoadCompanyData ();
	else
		LoadWarehouseData ();
}

void
LoadCompanyData (void)
{
	int		month;
	int		i;

	SillyBusyFunction (1);

	/*-------------------------
	| calculate current month |
	-------------------------*/
	curr_month = mdy [1];
	curr_month--;

	/*---------------------------------
	| Calculate Actual Qty Committed. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 0L);

	/*-------------------------
	| calculate qty available |
	-------------------------*/
	if (envVarSoFwdAvl)
	{
		local_rec.available = NDEC (inmr_rec.on_hand) -
				      		  NDEC (inmr_rec.committed + realCommitted) -
				      		  NDEC (inmr_rec.backorder) -
				      		  NDEC (inmr_rec.forward);
	}
	else
	{
		local_rec.available = NDEC (inmr_rec.on_hand) -
				      		  NDEC (inmr_rec.committed + realCommitted) -
				      		  NDEC (inmr_rec.backorder);
	}
	if (envVarQcApply && envVarSkQcAvl)
		local_rec.available -= NDEC (inmr_rec.qc_qty);

	/*-----------------------------
	| Zero appropriate variables. |
	-----------------------------*/
	local_rec.mtd_sales = 0.00;
	local_rec.ytd_sales = 0.00;
	local_rec.one_ysales = 0.00;
	local_rec.two_ysales = 0.00;
	for (month = 0; month < 12; month++)
	{
		con [month] = 0.00;
		lcon [month] = 0.00;
		val [month] = 0.00;
		prf [month] = 0.00;
	}
	local_rec.st_date =	MonthEnd (comm_rec.inv_date) + 1;

	/*---------------------------------------
	| Load Year To Date Sales from incc	|
	---------------------------------------*/
	weeksDemand = 0.00;
	safetyStock = 0.00;
	abc_selfield (ccmr, "ccmr_id_no");
	abc_selfield (ffdm, "ffdm_id_no2");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			LoadHistory
			(
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash, 
				local_rec.st_date, 
				"1"
			);
			local_rec.mtd_sales += NDEC (local_rec.demand_value [35]);
			local_rec.ytd_sales += CalculateYtdValues ();

			for (i = 24; i < 36 ; i++)
			{
				local_rec.one_ysales += NDEC (local_rec.demand_value [i]);
				local_rec.two_ysales += NDEC (local_rec.demand_value [i]);
			}
			for (i = 12; i < 24 ; i++)
			{
				local_rec.two_ysales += NDEC (local_rec.demand_value [i]);
				lcon [i - 12] += NDEC (local_rec.demand_value [i]);
			}
			weeksDemand += incc_rec.wks_demand;
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	qcQty			= NDEC (inmr_rec.qc_qty);
	woQtyAnti		= NDEC (inmr_rec.wo_qty_anti);
	onOrder			= NDEC (inmr_rec.on_order);
	committed		= NDEC (inmr_rec.committed + realCommitted);
	backorder		= NDEC (inmr_rec.backorder);
	forward			= NDEC (inmr_rec.forward);
	closingStock	= NDEC (inmr_rec.on_hand);
	safetyStock		= NDEC (inmr_rec.safety_stock);
	FindCost ();
	abc_selfield (ffdm, "ffdm_id_no3");
	SillyBusyFunction (0);
}

void
LoadWarehouseData (void)
{
	int		i;
	int		month;
	int		whReturnStatus	=	0;

	SillyBusyFunction (1);

	/*--------------------------
	| Calculate current month. |
	--------------------------*/
	curr_month = mdy [1];
	curr_month--;

	/*-----------------------------
	| Zero appropriate variables. |
	-----------------------------*/
	local_rec.mtd_sales = 0.00;
	local_rec.ytd_sales = 0.00;
	local_rec.one_ysales = 0.00;
	local_rec.two_ysales = 0.00;
	for (month = 0; month < 12; month++)
	{
		con [month] = 0.00;
		lcon [month] = 0.00;
		val [month] = 0.00;
		prf [month] = 0.00;
	}
	abc_selfield (ffdm, "ffdm_id_no2");
	local_rec.st_date =	MonthEnd (comm_rec.inv_date) + 1;

	/*---------------------------------------
	| Load Year To Date Sales from incc	|
	---------------------------------------*/
	incc_rec.hhbr_hash = hhsiHash;
	incc_rec.hhcc_hash = currentHhccHash;
	whReturnStatus = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (!whReturnStatus)
	{
		LoadHistory
		(
			incc_rec.hhbr_hash, 
			incc_rec.hhcc_hash, 
			local_rec.st_date, 
			"1"
		);
		local_rec.mtd_sales += NDEC (local_rec.demand_value [35]);
		local_rec.ytd_sales += CalculateYtdValues ();

		for (i = 24; i < 36 ; i++)
		{
			local_rec.one_ysales += NDEC (local_rec.demand_value [i]);
			local_rec.two_ysales += NDEC (local_rec.demand_value [i]);
		}
		for (i = 12; i < 24 ; i++)
		{
			local_rec.two_ysales += NDEC (local_rec.demand_value [i]);
			lcon [i - 12] 		 += NDEC (local_rec.demand_value [i]);
		}
		weeksDemand = NDEC (incc_rec.wks_demand);
		safetyStock = NDEC (incc_rec.safety_stock);

		inme_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		cc = find_rec (inme, &inme_rec, COMPARISON, "r");
		bfclosingStock	=	(cc) ? 0.00 : inme_rec.closing_stock;
	}
	/*---------------------------------------
	| failed to find warehouse record	|
	---------------------------------------*/
	if (whReturnStatus)
	{
		incc_rec.closing_stock	= 0.00;
		incc_rec.committed		= 0.00;
		incc_rec.backorder		= 0.00;
		incc_rec.forward		= 0.00;
		incc_rec.on_order		= 0.00;
		incc_rec.qc_qty			= 0.00;
		incc_rec.wo_qty_anti	= 0.00;
		incc_rec.hhwh_hash		= 0L;
		bfclosingStock			= 0.00;
	}

	/*---------------------------------
	| Calculate Actual Qty Committed. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, currentHhccHash);

	/*----------------------------
	| Calculate available stock. |
	----------------------------*/
	if (envVarSoFwdAvl)
	{
		local_rec.available = 	NDEC (incc_rec.closing_stock) -
								NDEC (incc_rec.committed + realCommitted) -
								NDEC (incc_rec.backorder) -
								NDEC (incc_rec.forward);
	}
	else
	{
		local_rec.available = 	NDEC (incc_rec.closing_stock) -
								NDEC (incc_rec.committed + realCommitted) -
								NDEC (incc_rec.backorder);
	}
	if (envVarQcApply && envVarSkQcAvl)
		local_rec.available -= NDEC (incc_rec.qc_qty);

	qcQty			= NDEC (incc_rec.qc_qty);
	woQtyAnti		= NDEC (incc_rec.wo_qty_anti);
	onOrder			= NDEC (incc_rec.on_order);
	committed		= NDEC (incc_rec.committed + realCommitted);
	backorder		= NDEC (incc_rec.backorder);
	forward			= NDEC (incc_rec.forward);
	closingStock	= NDEC (incc_rec.closing_stock);
	bfclosingStock	= NDEC (bfclosingStock);
	FindCost ();

	abc_selfield (ffdm, "ffdm_id_no3");
	SillyBusyFunction (0);
}

/*===============================
| Calculate Ytd Sales.			|
===============================*/
float	
CalculateYtdValues (void)
{
	int		i,
			j;
	float	ytd = 0.00;

	/*-------------------------------
	| store consumption values		|
	-------------------------------*/
	for (i = 0;i < 12;i++)
	{
		val [i] += incc_val [i];
		prf [i] += incc_prf [i];
		con [i] += NDEC (local_rec.demand_value [i + 24]);
	}
	/*---------------------------------------
	| no fiscal set up						|
	---------------------------------------*/
	if (comm_rec.fiscal == 0)
	{
		/*---------------------------------------
		| sum to current month (feb == 1)		|
		---------------------------------------*/
		for (i = 0;i <= curr_month;i++)
			ytd += NDEC (local_rec.demand_value [i + 24]);

		return (ytd);
	}

	/*---------------------------------------
	| need to sum from fiscal to dec,		|
	| then jan to current month.			|
	---------------------------------------*/
	j = 0;
	if (curr_month < comm_rec.fiscal)
	{
		for (i = comm_rec.fiscal;i < 12;i++)
			ytd += NDEC (local_rec.demand_value [35 - j++]);

		for (i = 0;i <= curr_month;i++)
			ytd += NDEC (local_rec.demand_value [35 - j++]);

		return (ytd);
	}
	for (i = comm_rec.fiscal;i <= curr_month;i++)
		ytd += NDEC (local_rec.demand_value [35 - j++]);

	return (ytd);
}

void
InternalFindSuper (
 char	*item_no)
{
	/*-------------------------------
	| end of supercession chain		|
	-------------------------------*/
	if (!strcmp (item_no, "                "))
		return;

	/*-------------------------------
	| search next link in chain		|
	-------------------------------*/
	strcpy (inmr2_rec.co_no, comm_rec.co_no);
	strcpy (inmr2_rec.item_no, item_no);
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (!cc)
		InternalFindSuper (inmr2_rec.supercession);
}

void
NormalRedraw (void)
{
	clear_ok = FALSE;
	Heading (1);
}

void
AllDisplay (void)
{
	char	temp_line [120];
	char	base_uom [5],
			alt_uom [5];
	char	item_desc [2][41];
	char	new_desc [101];

	crsr_off ();
	/*---------------------------
	| display misc data			|
	---------------------------*/
	display_ok = 1;
	sprintf (local_rec.ex_desc, "%-40.40s", " ");
	/*-----------------------------------
	| check for alternate stocking item	|
	-----------------------------------*/
	if (inmr_rec.hhsi_hash != 0L)
	{
		strcpy (local_rec.ex_desc, ML ("NOTE : SYNONYM STOCKING ITEM                   "));
	}

	/*-----------------------------------
	| check for extra description		|
	-----------------------------------*/
	if (strcmp (inmr_rec.ex_code, "   "))
	{
		open_rec (ined, ined_list, INED_NO_FIELDS, "ined_id_no");
		strcpy (ined_rec.co_no, comm_rec.co_no);
		strcpy (ined_rec.code, inmr_rec.ex_code);
		cc = find_rec (ined, &ined_rec, COMPARISON, "r");
		if (!cc)
			sprintf (local_rec.ex_desc, "NOTE : %-40.40s",
							ined_rec.desc);

		abc_fclose (ined);
	}
	print_at (1,2,"%s", local_rec.ex_desc);

	sprintf (item_desc [0], "%-40.40s", inmr_rec.description);
	inum_rec.hhum_hash = inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (!cc)
		sprintf (item_desc [1],"%36.36s %-4.4s",mlStkDisp [83],inum_rec.uom);
	else
		sprintf (item_desc [1],"%40.40s",mlStkDisp [82]);

	sprintf (new_desc, "%s %s", clip (item_desc [0]), clip (item_desc [1]));

	/*---------------------------------------
	| print data for item			|
	---------------------------------------*/
	strcpy (err_str, mlStkDisp [34]);
	sprintf (headingText, ".%s %16.16s (%-99.99s)",
		err_str, inmr_rec.item_no, new_desc);

	if (mainWindowOpen)
		Dsp_close ();

	mainWindowOpen = TRUE;

	Dsp_nc_prn_open (0, 2, 14, headingText,
				comm_rec.co_no, comm_rec.co_name,
				comm_rec.est_no, comm_rec.est_name,
				(char *) 0, (char *) 0);

	Dsp_saverec (headingText);
	Dsp_saverec ("");

	Dsp_saverec ("");

	sprintf (temp_line, " %s: %-16.16s%28.28s^E", mlStkDisp [6], inmr_rec.alternate, " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s: %-16.16s%28.28s^E", mlStkDisp [5], inmr_rec.alpha_code, " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s: %-16.16s%28.28s^E", mlStkDisp [41], inmr_rec.maker_no, " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s: See ^1<Display Bar Codes>^6   %18.18s^E", mlStkDisp [11], " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s: %8.8s %35.35s^E", mlStkDisp [81], inmr_rec.quick_code, " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s: %-1.1s (%-20.20s)%20.20s^E", mlStkDisp [3], inmr_rec.active_status,
		 inas_rec.description,
		 " ");
	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s     : %-6.6s     /     %s     : %-6.6s  ^E",
			 mlStkDisp [73], inmr_rec.sellgrp,
			 mlStkDisp [13], inmr_rec.buygrp);
			 
	Dsp_saverec (temp_line);

	/*-------------------------------
	| Find standard Unit of Measure |
	-------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (base_uom, inmr_rec.sale_unit);
		StdCnvFct	=	1.00;
	}
	else
	{
		strcpy (base_uom, inum_rec.uom);
		StdCnvFct 	= inum_rec.cnv_fct;
	}

	/*--------------------------------
	| Find Alternate Unit of Measure |
	--------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		strcpy (alt_uom, inmr_rec.sale_unit);
	else
		strcpy (alt_uom, inum_rec.uom);

	sprintf (temp_line, " %s          : %-4.4s       /     %s    : %-4.4s    ^E",
		mlStkDisp [12], base_uom, mlStkDisp [7],  alt_uom);

	Dsp_saverec (temp_line);

	sprintf (temp_line, " %s      : %-16.16s (%-40.40s)",
		mlStkDisp [0],
		inmr2_rec.item_no,
		inmr2_rec.description);
	Dsp_saverec (temp_line);

	ProcessTransferLines (inmr_rec.hhbr_hash);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (closingStock));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (local_rec.available));
	sprintf (tempStr [2], local_rec.rep_qty, NDEC (bfclosingStock));
	
	if (bfclosingStock != 0.0)
	{
		sprintf (temp_line, " %s     :      %14s    %5.5s %14s ^E %s             :     %14s",
			mlStkDisp [92], tempStr [0],
			ML ("B/F  "), tempStr [2], 
			mlStkDisp [85], tempStr [1]);
	}
	else
	{
		sprintf (temp_line, " %s     :      %14s    %5.5s %14s ^E %s             :     %14s",
			mlStkDisp [92], tempStr [0],
			" ", " ",
			mlStkDisp [85], tempStr [1]);
	}

	Dsp_saverec (temp_line);

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (committed));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (onOrder));

	sprintf (temp_line, " %s   :      %14s  %23.23s^E %s              :     %14s",
		mlStkDisp [87], tempStr [0],
		" ",
		mlStkDisp [89], tempStr [1]);
	Dsp_saverec (temp_line);

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (backorder));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (transIn));
	sprintf (temp_line, " %s   :      %14s  %23.23s^E %s       :     %14s",
		mlStkDisp [86], tempStr [0], " ", mlStkDisp [90], tempStr [1]);

	Dsp_saverec (temp_line);

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (forward));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (transOut));

	sprintf (temp_line, " %s     :      %14s  %23.23s^E %s      :     %14s",
		mlStkDisp [88], tempStr [0], " ", mlStkDisp [91], tempStr [1]);

	Dsp_saverec (temp_line);

	Dsp_srch ();

	PrintPopupStuff ();

	PrintPopup (popupSelect);
}

void
PrintPopupStuff (void)
{
	crsr_off ();
	move (66, 4);
	PGCHAR (8);

	print_at (4, 97, "<S%d>", popupSelect) ;

	move (66, 14);
	PGCHAR (8);

	move (0, 14);
	PGCHAR (10);

	move (131, 14);
	PGCHAR (11);

	move (66, 19);
	PGCHAR (9);
}

int
HeadingPrint (void)
{
	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_print ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
TransactionsAll (void)
{
	TransDisplay ("123456789:;<=");
	return (EXIT_SUCCESS);
}

int
TransactionsIR (void)
{
	TransDisplay ("238:;");
	return (EXIT_SUCCESS);
}

int
TransactionsPur (void)
{
	TransDisplay ("59");
	return (EXIT_SUCCESS);
}

int
TransactionsIC (void)
{
	TransDisplay ("67");
	return (EXIT_SUCCESS);
}

int
TransactionsAdj (void)
{
	TransDisplay ("4");
	return (EXIT_SUCCESS);
}

int
TransactionsDD (void)
{
	TransDisplay ("<=");
	return (EXIT_SUCCESS);
}

/*=======================================
| Display Transactions Information.	    |
=======================================*/
void
TransDisplay (
 char *tran_type)
{
	int		reply;
	int		limit = 0;

	tot_amount = 0.00;
	tot_qty = 0.00;

	SillyBusyFunction (1);
	/*---------------------------------------
	| open display							|
	---------------------------------------*/
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;

	lp_x_off = 0;
	lp_y_off = 4;
	limit = 0;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec (" BR | WH |    REFERENCE   |    REFERENCE   |BATCH/LOT|  TRAN   |   TRAN   |    VALUE    |UOM.|    QUANTITY    |     EXTENDED.    ");
	Dsp_saverec (" NO | NO |     NUMBER 1   |     NUMBER 2   | NUMBER. |  TYPE   |   DATE   |    EACH.    |    |     IN/OUT     |       VALUE.     ");
	Dsp_saverec (std_foot);
	/*---------------------------------------
	| find transactions						|
	---------------------------------------*/
	intr_rec.hhbr_hash	=	hhsiHash;
	intr_rec.date		=	TodaysDate () + 30000L;
	cc = find_rec (intr, &intr_rec, LTEQ, "r");
	while (!cc && intr_rec.hhbr_hash == hhsiHash)
	{
		/*===========================
		| if answered yes to prompt |
		| then will read records    |
		| until limit == 0          |
		===========================*/
		if (limit >= MAX_TRANS)
		{
			sprintf (err_str ,ML (mlSkMess244), MAX_TRANS);
			reply = prmptmsg (err_str , "YNyn", 5, 13);
#ifndef GVISION
			blank_at (13, 5, 65);
#endif	/* GVISION */
			crsr_off ();

			if (reply == 'N' || reply == 'n')
				break;

			limit = 0;
		}
		if (wh_flag == TRUE && intr_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (intr, &intr_rec, PREVIOUS, "r");
			continue;
		}
		ccmr_rec.hhcc_hash	=	intr_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		cc = Disp_save (tran_type);
		if (!cc)
			limit++;
		cc = find_rec (intr, &intr_rec, PREVIOUS, "r");
	}
	strcpy (disp_str, "^^GGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGHGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);
	sprintf (tempStr [0], local_rec.rep_qty, tot_qty);
	sprintf (disp_str, "    ^E    ^E %-15.15s^E                ^E         ^E         ^E          ^E             ^E%s^E %14s ^E %16.16s ",
			mlStkDisp [96],
			inmr_rec.sale_unit,
			tempStr [0],
			comma_fmt (DOLLARS (tot_amount), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (disp_str);
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
}

int
Disp_save (
 char *_tran_type)
{
	char	fm_amt [2][18];

	char	wk_type [2];
	float	CnvFct		= 0.00;
	float	DspQty 		= 0.00;
	double	DspAmt 		= 0.00;
	double	wk_cost 	= 0.00;
	double  sal_cost	= 0.00;
		
	static char *tr_type [] =
	{
		"STK BAL",
		"STK REC",
		"STK ISS",
		"STK ADJ",
		"STK PUR",
		"INVOICE",
		"CREDIT ",
		"PRD ISS",
		"STK TRN",
		"PRD REC",
		"STK W/O",
		"DD PUR ",
		"DD INV "
	};
	if (inum_rec.hhum_hash != intr_rec.hhum_hash)
	{
		inum_rec.hhum_hash = intr_rec.hhum_hash;  
		cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
		/* end add */
	}

	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}
	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;

	strcpy (wk_type, " ");
	wk_type [0] = '0' + intr_rec.type;

	if (strchr (_tran_type, wk_type [0]) == (char *) NULL)
		return (EXIT_FAILURE);

	DspQty	= intr_rec.qty / fourdec (CnvFct);
	DspQty	= NDEC (DspQty);
	wk_cost = out_cost (intr_rec.cost_price, inmr_rec.outer_size);
	wk_cost	*= (double) fourdec (CnvFct);
	wk_cost	=	DPP (wk_cost);

	sal_cost = out_cost (intr_rec.sale_price, inmr_rec.outer_size);
    sal_cost *= (double) fourdec (CnvFct);
    sal_cost =   DPP (sal_cost);

	switch (intr_rec.type)
	{
	case 1:
	case 2:
	case 4:
	case 5:
	case 7:
	case 10:
	case 12:
		DspAmt 		= DPP (wk_cost * (double) DspQty);
		break;

 	case 13: 
		DspAmt      = DPP (sal_cost * (double) DspQty);
        DspQty *= -1;
        intr_rec.qty    *= -1;
        DspAmt *= -1;
        break;			
	case 3:
	case 6:
	case 8:
	case 9:
	case 11:
	//case 13:
		DspAmt 		= DPP (wk_cost * (double) DspQty);
		if (DspQty > 0.00)
			DspQty *= -1;
		if (intr_rec.qty > 0.00)
			intr_rec.qty	*= -1;
		if (DspAmt > 0.00)
			DspAmt *= -1;
		break;

	default:
		DspQty = 0.00;
		DspAmt = 0.00;
		break;
	}

	DspAmt	=	DPP (DspAmt);
	tot_qty 	+= NDEC (intr_rec.qty);
	tot_amount 	+= DspAmt;
	if (intr_rec.type == 13)
		strcpy (fm_amt [0], comma_fmt (DOLLARS (sal_cost), "NNNNNN.NNNN"));
	else
		strcpy (fm_amt [0], comma_fmt (DOLLARS (wk_cost), "NNNNNN.NNNN"));
	strcpy (fm_amt [1], comma_fmt (DOLLARS (DspAmt), "N,NNN,NNN,NNN.NN"));

	sprintf (tempStr [0], local_rec.rep_qty, DspQty);
	sprintf (disp_str, " %2.2s ^E %2.2s ^E %-15.15s^E %-15.15s^E %-7.7s ^E %7.7s ^E%10.10s^E %11.11s ^E%4.4s^E %14s ^E %16.16s ",
		intr_rec.br_no,
		ccmr_rec.cc_no,
		intr_rec.ref1,
		intr_rec.ref2,
		intr_rec.batch_no,
		ML (tr_type [intr_rec.type - 1]),
		DateToString (intr_rec.date),
		fm_amt [0],
		inum_rec.uom,
		tempStr [0],
		fm_amt [1]);

	Dsp_saverec (disp_str);
    return (EXIT_SUCCESS);
}

/*===========================================
| Display Monthly Transactions Information.	|
===========================================*/
int
MonthEndDisplay (void)
{
	int		imdy [3];
	int		dataFnd,
			insfFlag;
	long	sDate	=	0L,
			eDate	=	0L;
	float	totOpenBal,
			qty,
			totQty;


	static char *tr_type [] =
	{
		"STK BAL",
		"STK REC",
		"STK ISS",
		"STK ADJ",
		"STK PUR",
		"INVOICE",
		"CREDIT ",
		"PRD ISS",
		"STK TRN",
		"PRD REC",
		"STK W/O"
	};

	open_rec (inmb, inmb_list, INMB_NO_FIELDS, "inmb_id_no");

	FLD ("sMthYr")  = YES;
	FLD ("eMthYr") = YES;
	entry_exit = 0;
	prog_exit = 0;
	restart = 0;
	display_ok = 1;
	search_ok = 1;

	init_vars (2);
	scn_set (2);

	print_at (13,33, "%31.31s", " ");
	print_at (14,33, "%31.31s", " ");
	print_at (15,33, "%31.31s", " ");
	print_at (16,33, "%31.31s", " ");
	box (32, 13, 32, 2);
	scn_write (2);

	entry (2);
	if (restart)
	{
		restart = FALSE;
		ClearRedraw ();
		return (FALSE);
	}

	scn_write (2);
	scn_display (2);
	edit (2);
	if (restart)
	{
		restart = FALSE;
		ClearRedraw ();
		return (FALSE);
	}

	SillyBusyFunction (1);

	sDate = local_rec.sDate;
	DateToDMY (sDate, &imdy [0], &imdy [1], &imdy [2]);

	/*---------------------------------------
	| open display				|
	---------------------------------------*/
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("                       |BR|WH|   REFERENCE.  |   REFERENCE.  | TRAN  |  TRAN  |QUANTITY");
	Dsp_saverec ("                       |NO|NO|    NUMBER 1   |    NUMBER 2   | TYPE  |  DATE  | IN/OUT ");
	Dsp_saverec (std_foot);
	
	while (sDate <= local_rec.eDate)
	{
		totOpenBal = 0.00;
		insfFlag = FALSE;
		dataFnd = FALSE;

		strcpy (inmb_rec.co_no, comm_rec.co_no);
		inmb_rec.hhbr_hash = hhsiHash;
		inmb_rec.date = sDate;
		if (wh_flag)
			inmb_rec.hhcc_hash = currentHhccHash;
		else
			inmb_rec.hhcc_hash = 0L;
		for (cc = find_rec (inmb, &inmb_rec, GTEQ, "r");
			!cc && inmb_rec.hhbr_hash == hhsiHash && inmb_rec.date == sDate;
			cc = find_rec (inmb, &inmb_rec, NEXT, "r"))
		{
			eDate = MonthEnd (inmb_rec.date);
	
			totOpenBal += inmb_rec.opening_bal;

			if (inmb_rec.insuf_trx [0] == 'Y')
				insfFlag = TRUE;

			dataFnd = TRUE;

			if (wh_flag)
				break;
		}
	
		if (!dataFnd)
		{
			DateToDMY (sDate, &imdy [0], &imdy [1], &imdy [2]);
			imdy [1]++;
			if (imdy [1] > 12)
			{
				imdy [1] = 1;
				imdy [2]++;
			}
			sDate = DMYToDate (imdy [0], imdy [1], imdy [2]);
			continue;
		}

		sprintf (err_str, " %s %02d Opening Balance ",
						month_nm [imdy [1] - 1],
						imdy [2]);
		
		sprintf (disp_str, "%30.30s     %8.2f  %31.31s",
						err_str,
						totOpenBal,
						(insfFlag) ? "** INSUFFICIENT TRANSACTIONS **" :
									"                               ");
		Dsp_saverec (disp_str);
		totQty = totOpenBal;

		/*---------------------------
		| find transactions			|
		---------------------------*/
		intr_rec.hhbr_hash = hhsiHash;
		intr_rec.date = sDate;
		for (cc = find_rec (intr, &intr_rec, GTEQ, "r");
			!cc && intr_rec.hhbr_hash == hhsiHash && intr_rec.date <= eDate;
			cc = find_rec (intr, &intr_rec, NEXT, "r"))
		{
			if (wh_flag == TRUE && intr_rec.hhcc_hash != currentHhccHash)
				continue;
	
			ccmr_rec.hhcc_hash	=	intr_rec.hhcc_hash;
			cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");

			switch (intr_rec.type)
			{
			case 1:
			case 2:
			case 4:
			case 5:
			case 7:
			case 10:
				qty = intr_rec.qty;
				break;
		
			case 3:
			case 6:
			case 8:
			case 9:
			case 11:
				qty = intr_rec.qty;
				qty *= -1;
				break;
		
			default:
				qty = 0.00;
				break;
			}
	
			sprintf (disp_str, "                       ^E%2.2s^E%2.2s^E%-15.15s^E%-15.15s^E%7.7s^E%8.8s^E%8.2f",
						intr_rec.br_no,
						ccmr_rec.cc_no,
						intr_rec.ref1,
						intr_rec.ref2,
						tr_type [intr_rec.type - 1],
						DateToString (intr_rec.date),
						qty);
					
			Dsp_saverec (disp_str);

			totQty += qty;
		}
		strcpy (disp_str, "^^GGGGGGGGGGGGGGGGGGGGGGGHGGHGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGHGGGGGGGGHGGGGGGGG^^");
		Dsp_saverec (disp_str);
		sprintf (disp_str, "                             ^E  TOTALS       ^E               ^E       ^E        ^E%8.2f", totQty);
		Dsp_saverec (disp_str);

		DateToDMY (sDate, &imdy [0], &imdy [1], &imdy [2]);
		imdy [1]++;
		if (imdy [1] > 12)
		{
			imdy [1] = 1;
			imdy [2]++;
		}
		sDate = DMYToDate (imdy [0], imdy [1], imdy [2]);
	}
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();

	abc_fclose (inmb);
	return (EXIT_SUCCESS);
}
/*=======================================
| Display Inventory Supplier Records.   |
=======================================*/
int
SupplierDisplay (void)
{
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	if (displayCostOk)
	{
	    Dsp_saverec ("Pri|Supplier| Br | Wh | Cur | Cty | Supplier Item No |   Supplier     | Last  Cost | Min Order  | Nrm Order  |  Order  |  Lead  ");
    	Dsp_saverec ("No |Supplier| No | No |Code |Code |                  |   FOB (FGN)    |    Date    |    Qty.    |    Qty.    |Multiple |  Time  ");
	}
	else
	{
	    Dsp_saverec ("Pri|Supplier| Br | Wh | Cur | Cty | Supplier Item No |                | Last  Cost | Min Order  | Nrm Order  |  Order  |  Lead  ");
    	Dsp_saverec ("No |Supplier| No | No |Code |Code |                  |                |    Date    |    Qty.    |    Qty.    |Multiple |  Time  ");
	}
	Dsp_saverec (std_foot);

	inis_rec.hhbr_hash = hhsiHash;
	strcpy (inis_rec.sup_priority, "  ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == hhsiHash)
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}	
	    if (inis_rec.lead_time == (float) 0.00)
			inis_rec.lead_time = GetLeadDate (inis_rec.hhis_hash, comm_rec.inv_date);

		sprintf (tempStr [0], local_rec.rep_qty, NDEC (inis_rec.min_order));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (inis_rec.norm_order));

	    if (displayCostOk)
	    {
			sprintf (disp_str, " %-2.2s^E %6.6s ^E %2.2s ^E %2.2s ^E %3.3s ^E %3.3s ^E %-16.16s ^E%15.15s ^E %10.10s ^E%11s ^E%11s ^E %7.2f ^E %6.1f ",
				inis_rec.sup_priority,
				sumr_rec.crd_no,
				inis_rec.br_no,
				inis_rec.wh_no,
				sumr_rec.curr_code,
				sumr_rec.ctry_code,
				inis_rec.sup_part,
				comma_fmt (inis_rec.fob_cost, "NNNN,NNN,NNN.NN"),
				DateToString (inis_rec.lcost_date),
				tempStr [0],
				tempStr [1],
				inis_rec.ord_multiple,
				inis_rec.lead_time);
	    }
	    else
	    {
			sprintf (disp_str, " %-2.2s^E %6.6s ^E %2.2s ^E %2.2s ^E %3.3s ^E %3.3s ^E %-16.16s ^E%-16.16s^E %10.10s ^E%11s ^E%11s ^E %7.2f ^E %6.1f ",
				inis_rec.sup_priority,
				sumr_rec.crd_no,
				inis_rec.br_no,
				inis_rec.wh_no,
				sumr_rec.curr_code,
				sumr_rec.ctry_code,
				inis_rec.sup_part,
				" ",
				DateToString (inis_rec.lcost_date),
				tempStr [0],
				tempStr [1],
				inis_rec.ord_multiple,
				inis_rec.lead_time);
	    }
		Dsp_saverec (disp_str);

		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
	abc_fclose (inis);
	abc_fclose (sumr);
	abc_selfield (ccmr, "ccmr_id_no");

	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

/*=======================================
| Display Movements Information.	    |
=======================================*/
int
UOM_display (void)
{
	float	StdCnvFct	=	0.00,
			CnvFct		=	0.00;

	float	totals [7];

	totals [0] = totals [1] = totals [2] = totals [3] = 0.00;
	totals [4] = totals [5] = totals [6] = 0.00;

	SillyBusyFunction (1);

	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (3, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec (" UOM. |    Opening     |   Purchases    |    Receipt     |     Issues     |     Adjust     |     Sales      |    Closing     ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;

	incc_rec.hhcc_hash	=	currentHhccHash;
	incc_rec.hhbr_hash	=	hhsiHash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "incc", "DBFIND");

	inwu_rec.hhwh_hash 	=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash	=	0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "r");
	while (!cc && inwu_rec.hhwh_hash ==	incc_rec.hhwh_hash)
	{
		inum_rec.hhum_hash = inwu_rec.hhum_hash;  
		cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
		if (cc)
		{
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
			inum_rec.cnv_fct = 1;
		}
		CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
		sprintf (tempStr [0], local_rec.rep_qty, 
				NDEC (inwu_rec.opening_stock / CnvFct));
		sprintf (tempStr [1], local_rec.rep_qty, 
				NDEC (inwu_rec.pur / CnvFct));
		sprintf (tempStr [2], local_rec.rep_qty, 
				NDEC (inwu_rec.receipts / CnvFct));
		sprintf (tempStr [3], local_rec.rep_qty, 
				NDEC (inwu_rec.issues / CnvFct));
		sprintf (tempStr [4], local_rec.rep_qty, 
				NDEC (inwu_rec.adj / CnvFct));
		sprintf (tempStr [5], local_rec.rep_qty, 
				NDEC (inwu_rec.sales / CnvFct));
		sprintf (tempStr [6], local_rec.rep_qty, 
				NDEC (inwu_rec.closing_stock / CnvFct));
		sprintf (disp_str, " %-4.4s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s",
				inum_rec.uom,
				tempStr [0],
				tempStr [1],
				tempStr [2],
				tempStr [3],
				tempStr [4],
				tempStr [5],
				tempStr [6]);

		totals [0] += NDEC (inwu_rec.opening_stock);
		totals [1] += NDEC (inwu_rec.pur);
		totals [2] += NDEC (inwu_rec.receipts);
		totals [3] += NDEC (inwu_rec.issues);
		totals [4] += NDEC (inwu_rec.adj);
		totals [5] += NDEC (inwu_rec.sales);
		totals [6] += NDEC (inwu_rec.closing_stock);

		Dsp_saverec (disp_str);
		cc = find_rec (inwu, &inwu_rec, NEXT, "r");
	}
	strcpy (disp_str, "^^GGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);
	sprintf (tempStr [0], local_rec.rep_qty, totals [0]);
	sprintf (tempStr [1], local_rec.rep_qty, totals [1]);
	sprintf (tempStr [2], local_rec.rep_qty, totals [2]);
	sprintf (tempStr [3], local_rec.rep_qty, totals [3]);
	sprintf (tempStr [4], local_rec.rep_qty, totals [4]);
	sprintf (tempStr [5], local_rec.rep_qty, totals [5]);
	sprintf (tempStr [6], local_rec.rep_qty, totals [6]);
	sprintf (disp_str, "^1 %s ^6^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
		inmr_rec.sale_unit,
		tempStr [0],
		tempStr [1],
		tempStr [2],
		tempStr [3],
		tempStr [4],
		tempStr [5],
		tempStr [6]);
	Dsp_saverec (disp_str);
	strcpy (disp_str, "^^GGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);
	abc_fclose (inwu);

	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}
/*===========================================
| Display Month end Movements Information.	|
===========================================*/
int
MendMoveDisp (void)
{
	SillyBusyFunction (1);


	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("Month End |    Opening     |   Purchases    |    Receipt     |     Issues     |     Adjust     |     Sales      |    Closing     ");
	Dsp_saverec ("   Date   |     Stock      |                |                |                |                |                |     Stock      ");
	Dsp_saverec (std_foot);

	if (wh_flag == FALSE)
	{
		Dsp_saverec (" ");
		Dsp_saverec (" CANNOT DISPLAY MONTH END STOCK LEVELS AT COMPANY ");
		Dsp_saverec (" ");
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}
	open_rec (inml, inml_list, INML_NO_FIELDS, "inml_id_no");

	inml_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inml_rec.mend_date	=	0L;
	cc = find_rec (inml, &inml_rec, GTEQ, "r");
	while (!cc && inml_rec.hhwh_hash ==	incc_rec.hhwh_hash)
	{
		sprintf (tempStr [0], local_rec.rep_qty, NDEC (inml_rec.opening_stock));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (inml_rec.pur));
		sprintf (tempStr [2], local_rec.rep_qty, NDEC (inml_rec.receipts));
		sprintf (tempStr [3], local_rec.rep_qty, NDEC (inml_rec.issues));
		sprintf (tempStr [4], local_rec.rep_qty, NDEC (inml_rec.adj));
		sprintf (tempStr [5], local_rec.rep_qty, NDEC (inml_rec.sales));
		sprintf (tempStr [6], local_rec.rep_qty, NDEC (inml_rec.closing_stock));
		sprintf (disp_str, "%-10.10s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s",
				DateToString (inml_rec.mend_date),
				tempStr [0],
				tempStr [1],
				tempStr [2],
				tempStr [3],
				tempStr [4],
				tempStr [5],
				tempStr [6]);
			Dsp_saverec (disp_str);

		cc = find_rec (inml, &inml_rec, NEXT, "r");
	}
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	if (wh_flag == TRUE)
		LoadWarehouseData ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*===================================
| Display Movements Information.	|
===================================*/
int
MovementDisplay (void)
{
	float	totals [7];

	SillyBusyFunction (1);

	totals [0] = totals [1] = totals [2] = totals [3] = 0.00;
	totals [4] = totals [5] = totals [6] = 0.00;

	abc_selfield (ccmr, "ccmr_id_no");
	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("Warehouse. |    Opening     |   Purchases    |    Receipt     |     Issues     |     Adjust     |     Sales      |    Closing     ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (tempStr [0],local_rec.rep_qty, NDEC (incc_rec.opening_stock));
			sprintf (tempStr [1],local_rec.rep_qty, NDEC (incc_rec.pur));
			sprintf (tempStr [2],local_rec.rep_qty, NDEC (incc_rec.receipts));
			sprintf (tempStr [3],local_rec.rep_qty, NDEC (incc_rec.issues));
			sprintf (tempStr [4],local_rec.rep_qty, NDEC (incc_rec.adj));
			sprintf (tempStr [5],local_rec.rep_qty, NDEC (incc_rec.sales));
			sprintf (tempStr [6],local_rec.rep_qty, NDEC (incc_rec.closing_stock));
			sprintf (disp_str, " %-9.9s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s",
				ccmr_rec.acronym,
				tempStr [0],
				tempStr [1],
				tempStr [2],
				tempStr [3],
				tempStr [4],
				tempStr [5],
				tempStr [6]);
			Dsp_saverec (disp_str);

			totals [0] += NDEC (incc_rec.opening_stock);
			totals [1] += NDEC (incc_rec.pur);
			totals [2] += NDEC (incc_rec.receipts);
			totals [3] += NDEC (incc_rec.issues);
			totals [4] += NDEC (incc_rec.adj);
			totals [5] += NDEC (incc_rec.sales);
			totals [6] += NDEC (incc_rec.closing_stock);

			inme_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			cc = find_rec (inme, &inme_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (tempStr [0],local_rec.rep_qty, NDEC (inme_rec.opening_stock));
				sprintf (tempStr [1],local_rec.rep_qty, NDEC (inme_rec.pur));
				sprintf (tempStr [2],local_rec.rep_qty, NDEC (inme_rec.receipts));
				sprintf (tempStr [3],local_rec.rep_qty, NDEC (inme_rec.issues));
				sprintf (tempStr [4],local_rec.rep_qty, NDEC (inme_rec.adj));
				sprintf (tempStr [5],local_rec.rep_qty, NDEC (inme_rec.sales));
				sprintf (tempStr [6],local_rec.rep_qty, NDEC (inme_rec.closing_stock));
				sprintf (disp_str, "%-11.11s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s",
					ML ("  -> B/F  "),
					tempStr [0],
					tempStr [1],
					tempStr [2],
					tempStr [3],
					tempStr [4],
					tempStr [5],
					tempStr [6]);
				Dsp_saverec (disp_str);
			}
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	strcpy (disp_str, "^^GGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);
	sprintf (tempStr [0], local_rec.rep_qty, totals [0]);
	sprintf (tempStr [1], local_rec.rep_qty, totals [1]);
	sprintf (tempStr [2], local_rec.rep_qty, totals [2]);
	sprintf (tempStr [3], local_rec.rep_qty, totals [3]);
	sprintf (tempStr [4], local_rec.rep_qty, totals [4]);
	sprintf (tempStr [5], local_rec.rep_qty, totals [5]);
	sprintf (tempStr [6], local_rec.rep_qty, totals [6]);
	sprintf (disp_str, " %s    ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
		mlStkDisp [96],
		tempStr [0],
		tempStr [1],
		tempStr [2],
		tempStr [3],
		tempStr [4],
		tempStr [5],
		tempStr [6]);
	Dsp_saverec (disp_str);
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	if (wh_flag == TRUE)
		LoadWarehouseData ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

/*========================
| Display Branch Status. |
========================*/
int
BranchStatusDisplay (void)
{
	float	totals [6];
	float	wk_avail = 0.00;

	SillyBusyFunction (1);

	totals [0] = totals [1] = totals [2] = totals [3] = 0.00;
	totals [4] = totals [5] = 0.00;

	abc_selfield (ccmr, "ccmr_id_no");
	lp_x_off = 1;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("Warehouse. |    On Hand     |   Committed    |   Backorder    |   Forward O.   |    Available   |    On Order.   ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			/*---------------------------------
			| Calculate Actual Qty Committed. |
			---------------------------------*/
			realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 
											   ccmr_rec.hhcc_hash);

			/*---------------------------------------
			| calculate available stock				|
			---------------------------------------*/
			if (envVarSoFwdAvl)
			{
				wk_avail = 
						NDEC (incc_rec.closing_stock) -
						NDEC (incc_rec.committed + realCommitted) -
						NDEC (incc_rec.backorder) -
						NDEC (incc_rec.forward);
			}
			else
			{
				wk_avail = 
						NDEC (incc_rec.closing_stock) -
						NDEC (incc_rec.committed + realCommitted) -
						NDEC (incc_rec.backorder);
			}
			if (envVarQcApply && envVarSkQcAvl)
				wk_avail -= NDEC (incc_rec.qc_qty);

			sprintf (tempStr [0], local_rec.rep_qty, NDEC (incc_rec.closing_stock));
			sprintf (tempStr [1], local_rec.rep_qty, 
					 NDEC (incc_rec.committed + realCommitted));
			sprintf (tempStr [2], local_rec.rep_qty, NDEC (incc_rec.backorder));
			sprintf (tempStr [3], local_rec.rep_qty, NDEC (incc_rec.forward));
			sprintf (tempStr [4], local_rec.rep_qty, NDEC (wk_avail));
			sprintf (tempStr [5], local_rec.rep_qty, NDEC (incc_rec.on_order));
			sprintf (disp_str, " %-9.9s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
				ccmr_rec.acronym,
				tempStr [0],
				tempStr [1],
				tempStr [2],
				tempStr [3],
				tempStr [4],
				tempStr [5]);
			Dsp_saverec (disp_str);

			totals [0] += NDEC (incc_rec.closing_stock);
			totals [1] += NDEC (incc_rec.committed + realCommitted);
			totals [2] += NDEC (incc_rec.backorder);
			totals [3] += NDEC (incc_rec.forward);
			totals [4] += NDEC (wk_avail);
			totals [5] += NDEC (incc_rec.on_order);
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	strcpy (disp_str, "^^GGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, totals [0]);
	sprintf (tempStr [1], local_rec.rep_qty, totals [1]);
	sprintf (tempStr [2], local_rec.rep_qty, totals [2]);
	sprintf (tempStr [3], local_rec.rep_qty, totals [3]);
	sprintf (tempStr [4], local_rec.rep_qty, totals [4]);
	sprintf (tempStr [5], local_rec.rep_qty, totals [5]);
	sprintf (disp_str, " %s    ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
		mlStkDisp [96],
		tempStr [0],
		tempStr [1],
		tempStr [2],
		tempStr [3],
		tempStr [4],
		tempStr [5]);
	Dsp_saverec (disp_str);
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	if (wh_flag == TRUE)
		LoadWarehouseData ();
	NormalRedraw ();
	return	(0);
}


/*===================================
| Display note pad information.		|
===================================*/
int
NotePadDisplay (void)
{
	open_rec (innh, innh_list, INNH_NO_FIELDS, "innh_id_no");
	open_rec (innd, innd_list, INND_NO_FIELDS, "innd_id_no");
	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("                 S T O C K   N O T E   P A D   D E T A I L S.                 ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (innh_rec.co_no, comm_rec.co_no);
	innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (innh, &innh_rec, GTEQ, "r");
	while (!cc && innh_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (strcmp (innh_rec.serial_no, "                         "))
		{
			sprintf (disp_str, "  %s%s : (%-25.25s)%s%-37.37s", ta [8], mlStkDisp [74],innh_rec.serial_no, ta [9], " ");
			Dsp_saverec (disp_str);
		}
		innd_rec.hhnh_hash = innh_rec.hhnh_hash;
		innd_rec.line_no = 0;
		cc = find_rec (innd, &innd_rec, GTEQ, "r");
		while (!cc && innd_rec.hhnh_hash == innh_rec.hhnh_hash)
		{
			sprintf (disp_str, "%-8.8s%-60.60s%-8.8s",
				" ", innd_rec.comments, " ");

			Dsp_saverec (disp_str);
			innd_rec.line_no++;
			cc = find_rec (innd, &innd_rec, NEXT, "r");
		}
		cc = find_rec (innh, &innh_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (innh);
	abc_fclose (innd);
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*===============================
| Add note pad information.		|
===============================*/
int
NotePadMaint (void)
{
	int		i,
			last_line	=	0;

	open_rec (innh, innh_list, INNH_NO_FIELDS, "innh_id_no");
	open_rec (innd, innd_list, INND_NO_FIELDS, "innd_id_no");

	txt_close (tx_window, FALSE);

	tx_window = txt_open (5, 0, 13, 60, 300,
			"S t o c k   N o t e   P a d   M a i n t e n a n c e.");

	strcpy (innh_rec.co_no, comm_rec.co_no);
	innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
	if (find_rec (innh, &innh_rec, COMPARISON, "r"))
	{
		strcpy (innh_rec.co_no, comm_rec.co_no);
		innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
		innh_rec.hhwh_hash = 0L;
		sprintf (innh_rec.serial_no, "%25.25s", " ");
		strcpy (innh_rec.stat_flag, "0");
		cc = abc_add (innh, &innh_rec);
		if (cc)
			file_err (cc, "innh", "DBADD");

		strcpy (innh_rec.co_no, comm_rec.co_no);
		innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
		if (find_rec (innh, &innh_rec, COMPARISON, "r"))
		{
			txt_close (tx_window, FALSE);
			abc_fclose (innh);
			abc_fclose (innd);
			NormalRedraw ();
			return (EXIT_SUCCESS);
		}
	}
	innd_rec.hhnh_hash = innh_rec.hhnh_hash;
	innd_rec.line_no = 0;
	cc = find_rec (innd, &innd_rec, GTEQ, "r");
	while (!cc && innd_rec.hhnh_hash == innh_rec.hhnh_hash)
	{
		sprintf (disp_str, "%-60.60s", innd_rec.comments);

		txt_pval (tx_window, disp_str, 0);

		cc = find_rec (innd, &innd_rec, NEXT, "r");
	}

	tx_lines = txt_edit (tx_window);
	if (!tx_lines)
	{
		txt_close (tx_window, FALSE);
		innd_rec.hhnh_hash = innh_rec.hhnh_hash;
		innd_rec.line_no = 0;
		cc = find_rec (innd, &innd_rec, GTEQ, "r");
		while (!cc && innd_rec.hhnh_hash == innh_rec.hhnh_hash)
		{
			abc_delete (innd);
			cc = find_rec (innd, &innd_rec, GTEQ, "r");
		}
		abc_fclose (innh);
		abc_fclose (innd);
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}
	for (i = 1; i <= tx_lines; i++)
	{
		last_line = i;

		innd_rec.hhnh_hash = innh_rec.hhnh_hash;
		innd_rec.line_no = i - 1;
		cc = find_rec (innd, &innd_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock ("innd");
			sprintf (innd_rec.comments, "%-60.60s", txt_gval (tx_window, i));
			cc = abc_add (innd, &innd_rec);
			if (cc)
				file_err (cc, "innd", "DBADD");
		}
		else
		{
			sprintf (innd_rec.comments, "%-60.60s", txt_gval (tx_window, i));
			cc = abc_update (innd, &innd_rec);
			if (cc)
				file_err (cc, "innd", "DBUPDATE");
		}
	}
	abc_unlock ("innd");
	txt_close (tx_window, FALSE);

	innd_rec.hhnh_hash = innh_rec.hhnh_hash;
	innd_rec.line_no = last_line;
	cc = find_rec (innd, &innd_rec, GTEQ, "u");
	while (!cc && innd_rec.hhnh_hash == innh_rec.hhnh_hash)
	{
		abc_delete (innd);
		cc = find_rec (innd, &innd_rec, GTEQ, "u");
	}
	abc_unlock ("innd");
	abc_fclose (innh);
	abc_fclose (innd);
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*========================
| Display Sales history. |
========================*/
int
HistoryDisplay (void)
{
	char	fm_amt [2][14];

	register	int		i;
	int		j;
	int		cur_mth;
	int		cur_year;

	cur_mth = mdy [1];
	cur_year = mdy [2];
	if (cur_year < 1900)
		cur_year += 1900;

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (1, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	if (displayCostOk)
	{
		Dsp_saverec ("    Month        |            S a l e s .          |    Sales     |    Profit    ");
		Dsp_saverec ("                 |   This  Year   |   Last  Year   |    Value     |              ");
	}
	else
	{
		Dsp_saverec ("    Month        |            S a l e s .          |    Sales     ");
		Dsp_saverec ("                 |   This  Year   |   Last  Year   |    Value     ");
	}

	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT / END] ");
	for (i = 0;i < 12; i++)
	{
		j = (i + cur_mth) % 12;
		sprintf (tempStr [0], local_rec.rep_qty, con [i]);
		sprintf (tempStr [1], local_rec.rep_qty, lcon [i]);
		if (displayCostOk)
		{
			strcpy (fm_amt [0], comma_fmt (DOLLARS (val [j]), "N,NNN,NNN.NN"));
			strcpy (fm_amt [1], comma_fmt (DOLLARS (prf [j]), "N,NNN,NNN.NN"));
			sprintf (disp_str, " %-10.10s %4d ^E %14s ^E %14s ^E %12.12s ^E %12.12s ",
				month_nm [j],
				((j + 1) > cur_mth) ? cur_year - 1 : cur_year,
				tempStr [0], tempStr [1],
				fm_amt [0], fm_amt [1]);
		}
		else
		{
			sprintf (disp_str, " %-10.10s %4d ^E %14s ^E %14s ^E %12.2f ",
				month_nm [j],
				((j + 1) > cur_mth) ? cur_year - 1 : cur_year,
				tempStr [0], tempStr [1],
				DOLLARS (val [j]));
		}
		Dsp_saverec (disp_str);
	}
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

/*=========================================================
| Display Works Orders - Manufacture and Component Items. |
=========================================================*/
int
WorksOrderDisplay (void)
{
	int		disp_flag;
	char	*ruler = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGG";

	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");

	sprintf (headingText, " Item Number  : %16.16s  (%40.40s)  ",
		inmr_rec.item_no,
		inmr_rec.description);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec (" BR | WH | WKS ORD |   BATCH    |STS|PRI|  BOM  | ROUTE |  REQ 'D  | UOM  |     REQ 'D     |     REC 'D     |     REJ 'D     ");
	Dsp_saverec (" NO | NO |   NO    |     NO     |   |   |  NO   |  NO   |   DATE   |      |      QTY       |      QTY       |      QTY       ");
	Dsp_saverec (" [Print]  [Next]  [Previous]  [Input/End] ");

	move (0, 23);
	cl_line ();
	rv_pr (ML (mlSkMess241), 0, 23, 1);

	disp_flag = ManufactDisplay (TRUE);

	if (disp_flag)
		Dsp_saverec (ruler);

	disp_flag = BomDisplay (TRUE);

	if (disp_flag)
		Dsp_saverec (ruler);

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (pcwo);
	abc_fclose (pcms);
	clear_mess ();
	swide ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*-------------------------------------------
| Display Works Orders - Manufacture Items. |
-------------------------------------------*/
int
WorkOrderManufactured (void)
{
	int		disp_flag;
	char	*ruler = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGG";

	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");

	sprintf (headingText, " Item Number  : %16.16s  (%40.40s)  ",
		inmr_rec.item_no,
		inmr_rec.description);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec (" BR | WH | WKS ORD |   BATCH    |STS|PRI|  BOM  | ROUTE |  REQ 'D  |     REQ 'D     |     REC 'D     |     REJ 'D     ");
	Dsp_saverec (" NO | NO |   NO    |     NO     |   |   |  NO   |  NO   |   DATE   |      QTY       |      QTY       |      QTY       ");
	Dsp_saverec (" [Print]  [Next]  [Previous]  [Input/End] ");

	move (0, 23);
	cl_line ();
	rv_pr (mlStkDisp [84], 0, 23, 1);

	disp_flag = ManufactDisplay (FALSE);

	if (disp_flag)
		Dsp_saverec (ruler);

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (pcwo);
	abc_fclose (pcms);
	clear_mess ();
	swide ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*---------------------------------------------
| Display works orders for manufactured item. |
---------------------------------------------*/
int 
ManufactDisplay (
 int	flag)
{
	int		display_flag = FALSE;

	totalReq = 0.00;
	totalRec = 0.00;
	totalRej = 0.00;

	pcwo_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (pcwo_rec.order_status [0] == 'D' ||
			pcwo_rec.order_status [0] == 'Z')
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		ccmr2_rec.hhcc_hash = pcwo_rec.hhcc_hash;
		cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		if (!display_flag)
			display_flag = TRUE;

		sprintf (tempStr [0], local_rec.rep_qty, NDEC (pcwo_rec.prod_qty));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (pcwo_rec.act_prod_qty));
		sprintf (tempStr [2], local_rec.rep_qty, NDEC (pcwo_rec.act_rej_qty));
		if (flag)
		{
			sprintf (disp_str, " %-2.2s ^E %-2.2s ^E %-7.7s ^E %-10.10s ^E %-1.1s ^E %1d ^E %5d ^E %5d ^E%-10.10s^E %-4.4s ^E %-14.14s ^E %-14.14s ^E %-14.14s",
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					pcwo_rec.order_no,
					pcwo_rec.batch_no,
					pcwo_rec.order_status,
					pcwo_rec.priority,
					pcwo_rec.bom_alt,
					pcwo_rec.rtg_alt,
					DateToString (pcwo_rec.reqd_date),
					" ",
					tempStr [0],
					tempStr [1],
					tempStr [2]);
		}
		else
		{
			sprintf (disp_str, " %-2.2s ^E %-2.2s ^E %-7.7s ^E %-10.10s ^E %-1.1s ^E %1d ^E %5d ^E %5d ^E%-10.10s^E %-14.14s ^E %-14.14s ^E %-14.14s",
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					pcwo_rec.order_no,
					pcwo_rec.batch_no,
					pcwo_rec.order_status,
					pcwo_rec.priority,
					pcwo_rec.bom_alt,
					pcwo_rec.rtg_alt,
					DateToString (pcwo_rec.reqd_date),
					tempStr [0],
					tempStr [1],
					tempStr [2]);
		}
		Dsp_saverec (disp_str);

		totalReq += NDEC (pcwo_rec.prod_qty);
		totalRec += NDEC (pcwo_rec.act_prod_qty);
		totalRej += NDEC (pcwo_rec.act_rej_qty);

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
	if (display_flag)
	{
		sprintf (tempStr [0], local_rec.rep_qty, NDEC (totalReq));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (totalRec));
		sprintf (tempStr [2], local_rec.rep_qty, NDEC (totalRej));
		if (flag)
		{
			sprintf (disp_str, " %s                                                                   ^E %-14.14s ^E %-14.14s ^E %-14.14s",
					mlStkDisp [96],
					tempStr [0],
					tempStr [1],
					tempStr [2]);
		}
		else
		{
			sprintf (disp_str, " %s                                                            ^E %-14.14s ^E %-14.14s ^E %-14.14s",
					mlStkDisp [96],
					tempStr [0],
					tempStr [1],
					tempStr [2]);
		}
		Dsp_saverec (disp_str);
	}
	return (display_flag);
}

/*-----------------------------------------
| Display Works Orders - Component Items. |
-----------------------------------------*/
int
WorkOrderComponent (void)
{
	int		disp_flag;
	char	*ruler = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGG";

	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");

	sprintf (headingText, " %s  : %16.16s  (%40.40s)  ",
		mlStkDisp [34],
		inmr_rec.item_no,
		inmr_rec.description);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("<------------------------------------- WORKS ORDERS WHERE USED -------------------------------------->|    REQ 'D   |    ISS 'D   ");
	Dsp_saverec ("BR|WH|WKS ORD| BATCH  # |S|P|    ITEM #      |     ITEM DESCRIPTION       |BOM #|RTG #| REQ DATE |UOM |     QTY     |     QTY     ");
	Dsp_saverec (" [Print]  [Next]  [Previous]  [Input/End] ");

	move (0, 23);
	cl_line ();
	rv_pr (mlStkDisp [84], 0, 23, 1);

	disp_flag = BomDisplay (FALSE);

	if (disp_flag)
		Dsp_saverec (ruler);

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (pcwo);
	abc_fclose (pcms);
	clear_mess ();
	swide ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

int 
BomDisplay (
 int	flag)
{
	float	totalIssue 	= 0.00;
	int		display_flag = FALSE;

	totalReq = 0.00;
	totalIssue = 0.00;

	abc_selfield (pcwo, "pcwo_hhwo_hash");
	pcms_rec.mabr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.mabr_hash == inmr_rec.hhbr_hash)
	{
		pcwo_rec.hhwo_hash = pcms_rec.hhwo_hash;
		cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		if (cc || (pcwo_rec.order_status [0] == 'D' ||
			pcwo_rec.order_status [0] == 'Z'))
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		ccmr2_rec.hhcc_hash = pcwo_rec.hhcc_hash;
		cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		inum_rec.hhum_hash	=	pcms_rec.uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			strcpy (inum_rec.uom, mlStkDisp [99]);

		if (!flag)
		{
			inmr4_rec.hhbr_hash = pcwo_rec.hhbr_hash;
			cc = find_rec (inmr4, &inmr4_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (pcms, &pcms_rec, NEXT, "r");
				continue;
			}
		}

		qtyReq = pcms_rec.matl_qty;
		pcms_rec.matl_wst_pc += 100;
		pcms_rec.matl_wst_pc /= 100;
		qtyReq *= pcms_rec.matl_wst_pc;

		if (!display_flag)
		{
			display_flag = TRUE;
			if (flag)
				Dsp_saverec ("<------------------------ WORKS ORDERS WHERE USED ----------------------->^E   REQ 'D QTY   ^E   ISS 'D QTY   ^E                ");
		}

		sprintf (tempStr [0], local_rec.rep_qty, NDEC (qtyReq));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (pcms_rec.qty_issued));
		if (flag)
		{
			sprintf (disp_str, " %-2.2s ^E %-2.2s ^E %-7.7s ^E %-10.10s ^E %-1.1s ^E %1d ^E %5d ^E %5d ^E%-10.10s^E %-4.4s ^E %-14.14s ^E %-14.14s ^E",
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					pcwo_rec.order_no,
					pcwo_rec.batch_no,
					pcwo_rec.order_status,
					pcwo_rec.priority,
					pcwo_rec.bom_alt,
					pcwo_rec.rtg_alt,
					DateToString (pcwo_rec.reqd_date),
					inum_rec.uom,
					tempStr [0],
					tempStr [1]);
		}
		else
		{
			sprintf (disp_str, "%-2.2s^E%-2.2s^E%-7.7s^E%-10.10s^E%-1.1s^E%1d^E%-16.16s^E%-28.28s^E%5d^E%5d^E%-10.10s^E%-4.4s^E%-13.13s^E%-13.13s",
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					pcwo_rec.order_no,
					pcwo_rec.batch_no,
					pcwo_rec.order_status,
					pcwo_rec.priority,
					inmr4_rec.item_no,
					inmr4_rec.description,
					pcwo_rec.bom_alt,
					pcwo_rec.rtg_alt,
					DateToString (pcwo_rec.reqd_date),
					inum_rec.uom,
					tempStr [0],
					tempStr [1]);
		}
		Dsp_saverec (disp_str);

		totalReq += NDEC (qtyReq);
		totalIssue += NDEC (pcms_rec.qty_issued);

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
	if (display_flag)
	{
		sprintf (tempStr [0], local_rec.rep_qty, NDEC (totalReq));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (totalIssue));
		if (flag)
		{
			sprintf (disp_str, " %s                                                                   ^E %-14.14s ^E %-14.14s ^E",
				mlStkDisp [96],
				tempStr [0],
				tempStr [1]);
		}
		else
		{
			sprintf (disp_str, " %s                                                                                               ^E%-13.13s^E%-13.13s",
				mlStkDisp [96],
				tempStr [0],
				tempStr [1]);
		}
		Dsp_saverec (disp_str);
	}
	abc_selfield (pcwo, "pcwo_hhbr_hash");
	return (display_flag);
}

/*-------------------------------------
| Display Released QC Retrieval File. |
-------------------------------------*/
int
DisplayRelQc (void)
{
	char	tmpDate [11];
	int		printed = FALSE;
	float	relQty = 0.00,
			rejQty = 0.00;

	if (!envVarQcApply)
	{
		print_mess (ML (mlSkMess245));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (wh_flag)
		open_rec (qchr, qchr_list, QCHR_NO_FIELDS, "qchr_id_no2");
	else
		open_rec (qchr, qchr_list, QCHR_NO_FIELDS, "qchr_id_no3");
	open_rec (qcln, qcln_list, QCLN_NO_FIELDS, "qcln_id_no");

	abc_selfield (inlo, "inlo_inlo_hash");

	sprintf (headingText, " %s  : %16.16s  (%40.40s)  ",
		mlStkDisp [34],
		inmr_rec.item_no,
		inmr_rec.description);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("Br|Wh| Release  |Line|Employee| QC | Original  | Released  | Rejected  |RS|  Lot  | Slot  |  Expiry  | Location | Refer |Refer |T");
	Dsp_saverec ("No|No|   Date   | No |  Code  |Cntr|    Qty    |    Qty    |    Qty    |Cd|  No   |  No   |   Date   |          |  One  | Two  | ");
	Dsp_saverec (" [Print]  [Next]  [Previous]  [Input/End] ");

	strcpy (qchr_rec.co_no, comm_rec.co_no);
	if (wh_flag)
	{
		strcpy (qchr_rec.br_no, comm_rec.est_no);
		strcpy (qchr_rec.wh_no, comm_rec.cc_no);
	}
	qchr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (qchr_rec.qc_centre, " ");
	qchr_rec.exp_rel_dt = 0L;
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qchr_rec.co_no, comm_rec.co_no) &&
		((wh_flag && 
		!strcmp (qchr_rec.br_no, comm_rec.est_no) &&
		!strcmp (qchr_rec.wh_no, comm_rec.cc_no)) ||
		!wh_flag) &&
		qchr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (qchr_rec.rel_qty == 0.00 &&
			qchr_rec.rej_qty == 0.00)
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}

		if (!printed)
			printed = TRUE;

		qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
		qcln_rec.line_no	= 0;
		cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
		while (!cc &&
			qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
		{
			sprintf (tempStr [0], local_rec.rep_qty,NDEC (qchr_rec.origin_qty));
			sprintf (tempStr [1], local_rec.rep_qty,NDEC (qcln_rec.rel_qty));
			sprintf (tempStr [2], local_rec.rep_qty, NDEC (qcln_rec.rej_qty));
			strcpy (tmpDate, DateToString (qcln_rec.release_dt));
			
			inlo_rec.inlo_hash	=	qcln_rec.inlo_hash;
			cc 	=	find_rec (inlo, &inlo_rec, COMPARISON, "r");
			if (cc)
				memset (&inlo_rec, 0, sizeof (inlo_rec));
				
			sprintf (disp_str,
				"%-2.2s^E%-2.2s^E%-10.10s^E%4d^E%-8.8s^E%-4.4s^E%-11.11s^E%-11.11s^E%-11.11s^E%-2.2s^E%-7.7s^E%-7.7s^E%-10.10s^E%-10.10s^E%-7.7s^E%-6.6s^E%-1.1s",
					qchr_rec.br_no,
					qchr_rec.wh_no,
					tmpDate,
					qcln_rec.line_no,
					qcln_rec.emp_code,
					qchr_rec.qc_centre,
					tempStr [0],
					tempStr [1],
					tempStr [2],
					qcln_rec.reason,
					inlo_rec.lot_no,
					inlo_rec.slot_no,
					DateToString (inlo_rec.expiry_date),
					inlo_rec.location,
					qchr_rec.ref_1,
					qchr_rec.ref_2,
					qchr_rec.source_type);
			Dsp_saverec (disp_str);

			relQty			+= qcln_rec.rel_qty;
			rejQty			+= qcln_rec.rej_qty;

			cc = find_rec (qcln, &qcln_rec, NEXT, "r");
		}

		cc = find_rec (qchr, &qchr_rec, NEXT, "r");
	}

	if (printed)
	{
		sprintf (tempStr [0], local_rec.rep_qty, NDEC (relQty));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (rejQty));
		Dsp_saverec ("^^GGJGGJGGGGGGGGGGJGGGGJGGGGGGGGJGGGGJGGGGGGGGGGGKGGGGGGGGGGGKGGGGGGGGGGGKGGJGGGGGGGJGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGJGGGGGGJG");
		sprintf (disp_str,
				" %s                                        ^E%-11.11s^E%-11.11s^E",
				mlStkDisp [96],
				tempStr [0],
				tempStr [1]);
		Dsp_saverec (disp_str);
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	}

	abc_fclose (qchr);
	abc_fclose (qcln);
	Dsp_srch ();
	Dsp_close ();
	swide ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*-----------------------------------------
| Display Pre-Released QC Retrieval File. |
-----------------------------------------*/
int
DisplayPreQc (void)
{
	char	tmpDate [11],
			tmpDate2 [11];
	int		printed = FALSE;
	float	originQty = 0.00,
			relQty = 0.00,
			rejQty = 0.00;

	if (!envVarQcApply)
	{
		print_mess (ML (mlSkMess245));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (wh_flag)
		open_rec (qchr, qchr_list, QCHR_NO_FIELDS, "qchr_id_no2");
	else
		open_rec (qchr, qchr_list, QCHR_NO_FIELDS, "qchr_id_no3");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

	abc_selfield (inlo, "inlo_inlo_hash");

	sprintf (headingText, " %s  : %16.16s  (%40.40s)  ",
		mlStkDisp [34],
		inmr_rec.item_no,
		inmr_rec.description);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("Br|Wh| Receipt  | Exp Rel  | QC |Original |Released |Rejected |  Lot  | Slot  |  Expiry  | Location | Supplier  | Refer | Refer |T");
	Dsp_saverec ("No|No|   Date   |   Date   |Cntr|   Qty   |   Qty   |   Qty   |  No   |  No   |   Date   |          |   Name    |  One  |  Two  | ");
	Dsp_saverec (" [Print]  [Next]  [Previous]  [Input/End] ");

	strcpy (qchr_rec.co_no, comm_rec.co_no);
	if (wh_flag)
	{
		strcpy (qchr_rec.br_no, comm_rec.est_no);
		strcpy (qchr_rec.wh_no, comm_rec.cc_no);
	}
	qchr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (qchr_rec.qc_centre, " ");
	qchr_rec.exp_rel_dt = 0L;
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qchr_rec.co_no, comm_rec.co_no) &&
		((wh_flag &&
		!strcmp (qchr_rec.br_no, comm_rec.est_no) &&
		!strcmp (qchr_rec.wh_no, comm_rec.cc_no)) ||
		!wh_flag) &&
		qchr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!printed)
			printed = TRUE;

		if (qchr_rec.source_type [0] == 'P')
		{
			sumr_rec.hhsu_hash	=	qchr_rec.hhsu_hash;
			if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
				strcpy (sumr_rec.crd_name, " ");
		}
		else
			strcpy (sumr_rec.crd_name, " ");

		sprintf (tempStr [0], local_rec.rep_qty, NDEC (qchr_rec.origin_qty));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (qchr_rec.rel_qty));
		sprintf (tempStr [2], local_rec.rep_qty, NDEC (qchr_rec.rej_qty));
		strcpy (tmpDate,  DateToString (qchr_rec.receipt_dt));
		strcpy (tmpDate2, DateToString (qchr_rec.exp_rel_dt));
		inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
		cc 	=	find_rec (inlo, &inlo_rec, COMPARISON, "r");
		if (cc)
			memset (&inlo_rec, 0, sizeof (inlo_rec));
				
		sprintf (disp_str,
				"%-2.2s^E%-2.2s^E%10.10s^E%10.10s^E%-4.4s^E%-9.9s^E%-9.9s^E%-9.9s^E%-7.7s^E%-7.7s^E%-10.10s^E%-10.10s^E%-11.11s^E%-7.7s^E%-7.7s^E%-1.1s",
				qchr_rec.br_no,
				qchr_rec.wh_no,
				tmpDate,
				tmpDate2,
				qchr_rec.qc_centre,
				tempStr [0],
				tempStr [1],
				tempStr [2],
				inlo_rec.lot_no,
				inlo_rec.slot_no,
				DateToString (inlo_rec.expiry_date),
				inlo_rec.location,
				sumr_rec.crd_name,
				qchr_rec.ref_1,
				qchr_rec.ref_2,
				qchr_rec.source_type);
		Dsp_saverec (disp_str);

		originQty	+= qchr_rec.origin_qty;
		relQty		+= qchr_rec.rel_qty;
		rejQty		+= qchr_rec.rej_qty;

		cc = find_rec (qchr, &qchr_rec, NEXT, "r");
	}

	if (printed)
	{
		sprintf (tempStr [0], local_rec.rep_qty, NDEC (originQty));
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (relQty));
		sprintf (tempStr [2], local_rec.rep_qty, NDEC (rejQty));
		Dsp_saverec ("^^GGJGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGJGGGGGGGGGJGGGGGGGGGJGGGGGGGJGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGJGGGGGGGJG");
		sprintf (disp_str,
				" %s                         ^E%-9.9s^E%-9.9s^E%-9.9s^E",
				mlStkDisp [96],
				tempStr [0],
				tempStr [1],
				tempStr [2]);
		Dsp_saverec (disp_str);
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGJGGGGGGGGGJGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	}

	abc_fclose (qchr);
	abc_fclose (sumr);
	Dsp_srch ();
	Dsp_close ();
	swide ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

void
ProcessTransferLines (
	long	hhbrHash)
{
	transIn = 0.00;
	transOut = 0.00;

	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.hhbr_hash = hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == hhbrHash)
	{
		if (!VAL_TRANSIT)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag && (itln_rec.r_hhcc_hash == currentHhccHash ||
		     	          itln_rec.i_hhcc_hash == currentHhccHash))
		{
			if (itln_rec.r_hhcc_hash == currentHhccHash)
				transIn    += itln_rec.qty_order;
			else
				transOut    += itln_rec.qty_order;
		}
		if (!wh_flag)
		{
			if (itln_rec.r_hhbr_hash == 0L)
			{
				transOut += itln_rec.qty_order;
				transIn  += itln_rec.qty_order;
			}
			else
				transOut += itln_rec.qty_order;
		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	abc_selfield (itln, "itln_r_hhbr_hash");

	itln_rec.r_hhbr_hash = hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.r_hhbr_hash == hhbrHash)
	{
		if (!VAL_TRANSIT)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag && (itln_rec.r_hhcc_hash == currentHhccHash ||
		     	          itln_rec.i_hhcc_hash == currentHhccHash))
		{
			if (itln_rec.r_hhcc_hash == currentHhccHash)
				transIn    += itln_rec.qty_order;
			else
				transOut    += itln_rec.qty_order;
		}
		else
			transIn  += itln_rec.qty_order;

		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return;
}

/*================================
| Process request for transfers. |
================================*/
int
RequestForTransfers (void)
{
	GetTransfers (FALSE);
    return (EXIT_SUCCESS);
}

/*====================
| Process transfers. |
====================*/
int
InTransitDisplay (void)
{
	GetTransfers (TRUE);
    return (EXIT_SUCCESS);
}

/*=================================
| Display In Transit Information. |
=================================*/
void
GetTransfers (
 int	transfers)
{
	SillyBusyFunction (1);

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	if (transfers)
	{
		Dsp_saverec (" ISSUING  |RECEIVING | DELVRY | DATE  OF |UOM.|    QUANTITY    |    QUANTITY    |    QUANTITY    |STS");
		Dsp_saverec (" CO/BR/WH | CO/BR/WH | DOCKET | TRANSFER |    |     ORDERED    |   BACKORDER    |    RECEIVED    |   ");
		Dsp_saverec (std_foot);
	}
	else
	{
		Dsp_saverec ("REQ. FROM | REQ. FOR | DELVRY | DATE  OF |UOM.|    QUANTITY    ");
		Dsp_saverec (" CO/BR/WH | CO/BR/WH | DOCKET | REQUEST. |    |   REQUESTED    ");
		Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");
	}

	abc_selfield (itln, "itln_hhbr_hash");

	ProcessTransactions (inmr_rec.hhbr_hash, transfers);

	abc_selfield (itln, "itln_r_hhbr_hash");

	ProcessTransactions (inmr_rec.hhbr_hash, transfers);

	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
}

/*=========================
| Process transfer items. |
=========================*/
void
ProcessTransactions (
	long	hhbrHash, 
	int		transfers)
{
	char	iss_co [3],
			iss_br [3],
			iss_wh [3],
			rec_co [3],
			rec_br [3],
			rec_wh [3];

	itln_rec.hhbr_hash = hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && (itln_rec.hhbr_hash == hhbrHash ||
		        itln_rec.r_hhbr_hash == hhbrHash))
	{
		if (transfers && !VAL_ITLN)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (!transfers && itln_rec.status [0] != 'R')
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (itln_rec.qty_order + itln_rec.qty_border <= 0.00)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag &&
		    (itln_rec.i_hhcc_hash != currentHhccHash &&
		      itln_rec.r_hhcc_hash != currentHhccHash))
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
		cc = find_rec (ithr, &ithr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		cc = GetWarehouse (itln_rec.i_hhcc_hash);
		strcpy (iss_co, (cc) ? "  " : ccmr_rec.co_no);
		strcpy (iss_br, (cc) ? "  " : ccmr_rec.est_no);
		strcpy (iss_wh, (cc) ? "  " : ccmr_rec.cc_no);

		cc = GetWarehouse (itln_rec.r_hhcc_hash);
		strcpy (rec_co, (cc) ? "  " : ccmr_rec.co_no);
		strcpy (rec_br, (cc) ? "  " : ccmr_rec.est_no);
		strcpy (rec_wh, (cc) ? "  " : ccmr_rec.cc_no);

		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		if (transfers)
		{
			sprintf (tempStr [0], local_rec.rep_qty, 
						(itln_rec.qty_order + itln_rec.qty_rec) * CnvFct);
			sprintf (tempStr [1], local_rec.rep_qty, 
						itln_rec.qty_border * CnvFct);
			sprintf (tempStr [2], local_rec.rep_qty, 
						itln_rec.qty_rec * CnvFct);

			sprintf (disp_str, " %-2.2s/%-2.2s/%-2.2s ^E %-2.2s/%-2.2s/%-2.2s ^E %06ld ^E%10.10s^E%s^E %14s ^E %14s ^E %14s ^E %-1.1s",
					iss_co, iss_br, iss_wh,
					rec_co, rec_br, rec_wh,
					ithr_rec.del_no,
					DateToString (ithr_rec.iss_date),
					inum_rec.uom,
					tempStr [0],
					tempStr [1],
					tempStr [2],
					itln_rec.status);
		}
		else
		{
			sprintf (tempStr [0], local_rec.rep_qty,
					(itln_rec.qty_order + itln_rec.qty_border) * CnvFct);
			sprintf (disp_str, " %-2.2s/%-2.2s/%-2.2s ^E %-2.2s/%-2.2s/%-2.2s ^E %06ld ^E%10.10s^E%4.4s^E %14s ",
					iss_co, iss_br, iss_wh,
					rec_co, rec_br, rec_wh,
					ithr_rec.del_no,
					DateToString (ithr_rec.iss_date),
					inum_rec.uom,
				  	tempStr [0]);
		}
		Dsp_saverec (disp_str);

		if (strcmp (itln_rec.serial_no, blankSerial))
		{
			if (transfers)
				sprintf (disp_str, "                            %s : %-25.25s", mlStkDisp [68],itln_rec.serial_no);
			else
				sprintf (disp_str, "             %s : %-25.25s", mlStkDisp [68], itln_rec.serial_no);
			Dsp_saverec (disp_str);
		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
}

int
GetWarehouse (
	long	hhccHash)
{
	ccmr_rec.hhcc_hash	=	hhccHash;
	return (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"));
}

/*=======================
| Display Fifo Records. |
=======================*/
int
FifoDisplay (void)
{
	double	est_cost = 0.00,
			act_cost = 0.00,
			cost_used = 0.00;

	float	qty = NDEC (incc_rec.closing_stock);
	float	fifo_qty = 0.00;

	SillyBusyFunction (1);
	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	if (!FIFOLIFO)
	{
		Dsp_saverec (ML ("NOT A FIFO OR LIFO COSTED ITEM"));
		Dsp_saverec     ("                              ");
		Dsp_saverec (" ");
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}
	if (displayCostOk)
	{
		Dsp_saverec (" FIFO DATE |   FIFO COST  |    FIFO QTY    | GOODS RECEIPT | ACTUAL  FIFO |   ESTIMATED  ");
		Dsp_saverec ("           |     USED     |                |     NUMBER    |    COST      |  FIFO COST   ");
		Dsp_saverec (std_foot);
	}
	else
	{
		Dsp_saverec (" FIFO DATE |    FIFO QTY    | GOODS RECEIPT ");
		Dsp_saverec ("           |                |     NUMBER    ");
		Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");
	}

	if (wh_flag == FALSE)
	{
		Dsp_saverec (" ");
		if (displayCostOk)
			Dsp_saverec (" FIFO RECORDS MUST BE DISPLAYED AT WAREHOUSE LEVEL");
		else
			Dsp_saverec (" CANNOT DISPLAY FIFO AT COMPANY ");
		Dsp_saverec (" ");
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}
	cc = FindIncf (incc_rec.hhwh_hash, (FIFO) ? FALSE : TRUE, "r");
	while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
	{
		cost_used = incfRec.fifo_cost;

		/*---------------------------------------------
		| Receipt Costing not Done, At Estimate Cost. |
		---------------------------------------------*/
		if (incfRec.stat_flag [0] == 'E')
		{
			est_cost  = incfRec.fifo_cost;
			act_cost  = 0.00;
		}
		else
		{
			est_cost = (ACT_COST) ? incfRec.act_cost : incfRec.fifo_cost;
			act_cost = (ACT_COST) ? incfRec.fifo_cost : incfRec.act_cost;
		}


		if (qty < incfRec.fifo_qty)
		{
			if (qty != 0.00)
			{
				fifo_qty = qty;
				PrintFifo (incfRec.fifo_date,
					    cost_used,
					    fifo_qty,
		        		incfRec.gr_number,
					    act_cost,
					    est_cost);
			}

			if (displayCostOk)
			{
				Dsp_saverec ("^^GGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGG");
				Dsp_saverec (mlStkDisp [108]);
			}
			else
			{
				Dsp_saverec ("^^GGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGG");
				Dsp_saverec (mlStkDisp [109]);
			}
			fifo_qty = incfRec.fifo_qty - qty;

			PrintFifo (incfRec.fifo_date,
				    cost_used,
				    fifo_qty,
		        	    incfRec.gr_number,
				    act_cost,
				    est_cost);

			qty = (float) 9999999.99;
		}
		else
		{
			fifo_qty = NDEC (incfRec.fifo_qty);

			PrintFifo 
			(
				incfRec.fifo_date,
				cost_used,
				fifo_qty,
				incfRec.gr_number,
				act_cost,
				est_cost
			);

			qty -= NDEC (incfRec.fifo_qty);
		}
		cc = FindIncf (0L, (FIFO) ? FALSE : TRUE, "r");
	}
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

void
PrintFifo (
 long ff_date, 
 double ff_cost, 
 float ff_qty, 
 char *ff_grin, 
 double ff_act, 
 double ff_est)
{
	sprintf (tempStr [0], local_rec.rep_qty, ff_qty);
	if (displayCostOk)
	{
		sprintf (disp_str,"%10.10s ^E %12.12s ^E %14s ^E%15.15s^E %12.2f ^E %12.2f ",
				DateToString (ff_date),
				comma_fmt (ff_cost, "N,NNN,NNN.NN"),
				tempStr [0],
				ff_grin,
				ff_act,
				ff_est);
	}
	else
	{
		sprintf (disp_str,"%10.10s ^E %14s ^E%15.15s",
				DateToString (ff_date),
				tempStr [0],
		        ff_grin);
	}

	Dsp_saverec (disp_str);
}

/*===========================
| Display Location Records. |
===========================*/
int
LocationDisplay (void)
{
	int		reply;
	float	total_loc = 0.00;
	float	total_aloc = 0.00;
	float	DspQty	  = 0.00;
	char	Expiry [11];
	char	CreateDate [11];

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("   Stock   |Loc.|Lot/Batch|Supplier |   Expiry   |   Create   | S | Date Last  | No  |  No |UOM.|    Location    |    Location    ");
	Dsp_saverec ("  Location |Type| Number  |Lot/Batch|    Date    |   Date.    | C |  Changed   |Hits |Picks|    | Available Qty. |    Quantity.   ");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");

	if (wh_flag == FALSE)
	{
		Dsp_saverec (" ");
		Dsp_saverec (" LOCATIONS/LOTS ONLY DISPLAYED AT W/H");
		Dsp_saverec (" ");
		Dsp_srch ();
		Dsp_close ();
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}

	cl_box (40,10,48,8);
	print_at (11,41, "%-46.46s", " ");
	print_at (11,44, ML (mlSkMess254));
	print_at (12,41, "%-46.46s", ML (mlSkMess255));
	print_at (13,41, "%-46.46s", ML (mlSkMess256));
	print_at (14,41, "%-46.46s", ML (mlSkMess257));
	print_at (15,41, "%-46.46s", ML (mlSkMess258));
	print_at (16,41, "%-46.46s", ML (mlSkMess259));
	print_at (17,41, "%-46.46s", ML (mlSkMess260));
	print_at (18,41, "%-46.46s", " ");
	reply = prmptmsg (ML (mlSkMess611), "123456", 54, 18);
	crsr_off ();

	if (reply == '1')
		abc_selfield (inlo, "inlo_id_loc");
	if (reply == '2')
		abc_selfield (inlo, "inlo_id_lot");
	if (reply == '3')
		abc_selfield (inlo, "inlo_id_exp");
	if (reply == '4')
		abc_selfield (inlo, "inlo_id_fifo");
	if (reply == '5')
		abc_selfield (inlo, "inlo_mst_loc");
	if (reply == '6')
		abc_selfield (inlo, "inlo_mst_lot");

	inlo_rec.hhwh_hash 		= incc_rec.hhwh_hash;
	inlo_rec.hhum_hash 		= 0L;
	inlo_rec.expiry_date	= 0L;
	inlo_rec.date_create	= 0L;
	inlo_rec.loc_type [0] 	= ' ';
	sprintf (inlo_rec.location, "%-10.10s", " ");
	sprintf (inlo_rec.lot_no, 	"%-7.7s", " ");
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
	while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		if (inlo_rec.cnv_fct == 0.00)
			inlo_rec.cnv_fct = 1.00;

		DspQty = inlo_rec.qty / inlo_rec.cnv_fct;

		sprintf (tempStr [0], local_rec.rep_qty, NDEC (DspQty));

		DspQty = inlo_rec.qty  - SK_CalcAlloc (inlo_rec.inlo_hash);
		DspQty /= inlo_rec.cnv_fct;
		sprintf (tempStr [1], local_rec.rep_qty, NDEC (DspQty));
		strcpy (Expiry, DateToString (inlo_rec.expiry_date));
		strcpy (CreateDate, DateToString (inlo_rec.date_create));

		sprintf (disp_str, " %-10.10s^E  %1.1s ^E %7.7s ^E %7.7s ^E %10.10s ^E %-10.10s ^E %1.1s ^E %10.10s ^E%5d^E%5d^E%s^E %14s ^E %14s ",
			inlo_rec.location,
			inlo_rec.loc_type,
			inlo_rec.lot_no,
			inlo_rec.slot_no,
			Expiry,
			CreateDate,
			inlo_rec.loc_status,
			DateToString (inlo_rec.date_upd),
			inlo_rec.no_hits,
			inlo_rec.no_picks,
			inlo_rec.uom,
			tempStr [1],
			tempStr [0]);

		sprintf (saveKey, "%s%s", inlo_rec.loc_type, inlo_rec.loc_status);
		total_loc  += NDEC (inlo_rec.qty);
		total_aloc += NDEC (inlo_rec.qty - SK_CalcAlloc (inlo_rec.inlo_hash));

		Dsp_saverec (disp_str);
		
		if (envVarSkAlldispLocx)
		{
			sprintf (disp_str, "%s", LocStatDisplay (saveKey));
			Dsp_saverec (disp_str);
		}

		cc = find_rec (inlo, &inlo_rec, NEXT, "r");
	}
	Dsp_saverec ("^^GGGGGGGGGGGHGGGGHGGGGGGGGGHGGGGGGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGHGGGHGGGGGGGGGGGGHGGGGGHGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG");
	sprintf (tempStr [0], local_rec.rep_qty, total_loc);
	sprintf (tempStr [1], local_rec.rep_qty, total_aloc);
	sprintf (disp_str, " %s    ^E    ^E         ^E         ^E            ^E            ^E   ^E            ^E     ^E     ^E%4.4s^E %14.14s ^E %14.14s ",
	mlStkDisp [96],
	inmr_rec.sale_unit,tempStr [1],tempStr [0]);
	Dsp_saverec (disp_str);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

/*===============================
| Display Number Plate Details. |
===============================*/
int
NoPlateDisplay (void)
{
	float	noPlateQtyRemain	=	0.0,
			dispQuantity		=	0.0,
			packQuantity		=	0.0;

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open 
	(
		0,
		4, 
		12, 
		headingText, 
		comm_rec.co_no, 
		comm_rec.co_name,
		comm_rec.est_no, 
		comm_rec.est_name,
	   (char *) 0, 
	   (char *) 0
   );

	if (envVarthreePlSystem)
	{
		Dsp_saverec (" Number Plate /|  Stock   |UOM.|    Unit   |    Pack   |   Charge  |Total Gross|   Total   | Location | Customer Order Ref.");
		Dsp_saverec (" Dock Receipt  | Location |    |  Quantity |  Quantity |   Weight  |  Weight   |    CBM    |  Status  |                    ");
	}
	else
	{
		Dsp_saverec (" Number Plate/ |   Stock    |UOM | Package   | Unit Qty. |       Location       |  Customer Order Ref. |Supplier| Purchase order  ");
		Dsp_saverec (" Goods Receipt |  Location  |    | Quantity  | Remaining |        Status        |                      | Number |     Number.     ");
	}
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");

	if (wh_flag == FALSE)
	{
		Dsp_saverec (" ");
		Dsp_saverec (" NUMBER PLATES DETAILS AT W/H ONLY");
		Dsp_saverec (" ");
		Dsp_srch ();
		Dsp_close ();
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}

	open_rec (llst, llst_list, LLST_NO_FIELDS, "llst_id_no");
	open_rec (sknh, sknh_list, SKNH_NO_FIELDS, "sknh_sknh_hash");
	open_rec (sknd, sknd_list, SKND_NO_FIELDS, "sknd_sknd_hash");
	open_rec (skni, skni_list, SKNI_NO_FIELDS, "skni_sknd_hash");
	open_rec (trve, trve_list, TRVE_NO_FIELDS, "trve_hhve_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

	abc_selfield (inlo, "inlo_mst_loc");

	inlo_rec.hhwh_hash 		= incc_rec.hhwh_hash;
	inlo_rec.hhum_hash 		= 0L;
	inlo_rec.expiry_date	= 0L;
	inlo_rec.date_create	= 0L;
	inlo_rec.loc_type [0] 	= ' ';
	sprintf (inlo_rec.location, "%-10.10s", " ");
	sprintf (inlo_rec.lot_no, 	"%-7.7s", " ");
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
	while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		sknd_rec.sknd_hash	=	inlo_rec.sknd_hash;
		cc = find_rec (sknd, &sknd_rec, EQUAL, "r");

		if (cc || sknd_rec.status [0] == 'D')
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}

		sknh_rec.sknh_hash	=	sknd_rec.sknh_hash;
		cc = find_rec (sknh, &sknh_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}

		/*
		 * Set quantity remaining from number plate detail.
		 */
		noPlateQtyRemain	=	sknd_rec.qty_rec - sknd_rec.qty_return;

		/*
		 * Subtract the amount(s) already issued.
		 */
		skni_rec.sknd_hash = sknd_rec.sknd_hash;
		cc = find_rec (skni, &skni_rec, GTEQ, "r");
		while (!cc && skni_rec.sknd_hash == sknd_rec.sknd_hash)
		{
			noPlateQtyRemain	-=	skni_rec.qty_issued;
			cc = find_rec (skni, &skni_rec, NEXT, "r");
		}
		if (noPlateQtyRemain <= 0.00)
			noPlateQtyRemain = 0.00;

		if (inlo_rec.qty <= 0.00)
			inlo_rec.qty = 0.00;

		if (inlo_rec.qty >= noPlateQtyRemain)
		{
			dispQuantity		=	noPlateQtyRemain;
			packQuantity		=	noPlateQtyRemain / inlo_rec.cnv_fct;
		}
		else
		{
			dispQuantity		=	inlo_rec.qty;
			packQuantity		=	inlo_rec.qty / inlo_rec.cnv_fct;
		}
		strcpy (llst_rec.co_no, comm_rec.co_no);
		strcpy (llst_rec.code,  inlo_rec.loc_status);
		cc = find_rec (llst, &llst_rec, EQUAL, "r");
		if (cc)
			strcpy (llst_rec.desc, " ");

		trve_rec.hhve_hash	=	sknd_rec.hhve_hash;
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
			strcpy (trve_rec.ref, " ");

		if (envVarthreePlSystem)
		{
			sprintf 
			(	
				disp_str, 
				"%-15.15s^E%10.10s^E%4.4s^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%-10.10s^E%-20.20s",
				sknh_rec.plate_no,
				inlo_rec.location,
				inlo_rec.uom,
				dispQuantity,
				packQuantity,
				inlo_rec.chg_wgt,
				inlo_rec.gross_wgt,
				inlo_rec.cu_metre,
				llst_rec.desc,
				sknd_rec.cus_ord_ref
			);
		}
		else
		{
			sumr_rec.hhsu_hash	=	sknd_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
			if (cc)
				strcpy (sumr_rec.crd_no, " ");

			strcpy (pohr_rec.pur_ord_no, " ");

			poln_rec.hhpl_hash	=	sknd_rec.hhpl_hash;
			cc = find_rec (poln, &poln_rec, COMPARISON, "r");
			if (!cc)
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
				if (cc)
					strcpy (pohr_rec.pur_ord_no, " ");
			}

			sprintf 
			(	
				disp_str, 
				"%-15.15s^E %10.10s ^E%4.4s^E%10.2f ^E%10.2f ^E %-20.20s ^E %-20.20s ^E %-6.6s ^E %-15.15s ",
				sknh_rec.plate_no,
				inlo_rec.location,
				inlo_rec.uom,
				packQuantity,
				dispQuantity,
				llst_rec.desc,
				sknd_rec.cus_ord_ref,
				sumr_rec.crd_no,
				pohr_rec.pur_ord_no
			);
		}
		Dsp_saverec (disp_str);

		cc = find_rec (inlo, &inlo_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (llst);
	abc_fclose (sknh);
	abc_fclose (sknd);
	abc_fclose (trve);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

int
SalesOrderDisplay (void)
{
	OrderDisplay (FALSE);
	return (EXIT_SUCCESS);
}

int
BackOrderDisplay (void)
{
	OrderDisplay (TRUE);
	return (EXIT_SUCCESS);
}

void
OrderDisplay (
 int	BackOrderSelect)
{
	char	*sptr;
	float	ord_qty 	= 0.00;
	float	bord_qty 	= 0.00;
	float	MendStock	=	0.00;
	double	ord_val 	= 0.00;
	double	bord_val 	= 0.00;
	char	tran_date [11];
	char	val_stat [3];
	int		stat_ok = FALSE;
	char	wk_cst_used [15];

	o_tot_qty = 0.00;
	o_tot_val = 0.00;

	saveIndex = 0;
	SillyBusyFunction (1);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_mabr_hash");

	if (envVarDbMcurr)
		open_rec (pocr,pocr_list,POCR_NO_FIELDS,"pocr_id_no");

	abc_selfield (ccmr, "ccmr_hhcc_hash");
	abc_selfield (itln, "itln_hhbr_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("CUSTOMER|             CUSTOMERS NAME              |BR|WH|  ORDER   |   DUE    |UOM.|     ORDER      | ORDER  |S|     AMOUNT     ");
	Dsp_saverec (" NUMBER |                                         |NO|NO|  DATE    |   DATE   |    |      QTY       | NUMBER | |                ");
	Dsp_saverec (std_foot);

	soln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (BackOrderSelect == TRUE && soln_rec.status [0] == 'B' && (soln_rec.qty_bord + soln_rec.qty_order)  <=  0.00)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		if (BackOrderSelect == TRUE && soln_rec.status [0] != 'B' && soln_rec.qty_bord <= 0.00) 
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		if (BackOrderSelect == FALSE && soln_rec.status [0] == 'B')
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		if (wh_flag == TRUE && soln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		if ((soln_rec.qty_order + soln_rec.qty_bord) > 0.00 ||
		     inmr_rec.inmr_class [0] == 'Z')
		{
			inum_rec.hhum_hash	=	soln_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
			CnvFct	=	StdCnvFct / PurCnvFct;

			ord_qty = soln_rec.qty_order + soln_rec.qty_bord;

			ord_val = out_cost (soln_rec.sale_price, inmr_rec.outer_size);
			ord_val *= ((double) ord_qty/CnvFct)  ;
			ord_val -= CAL (ord_val, soln_rec.dis_pc);

			ord_qty *= CnvFct;

			if (soln_rec.status [0] != 'B' && BackOrderSelect)
			{
				bord_qty = soln_rec.qty_bord;
				bord_val = out_cost (soln_rec.sale_price, inmr_rec.outer_size);
				bord_val *= ((double) bord_qty/CnvFct)  ;
				bord_val -= CAL (bord_val, soln_rec.dis_pc);
				bord_qty *= CnvFct;
			}

			ProcessOrderHeader 
			(
				soln_rec.hhso_hash,
				(soln_rec.status [0] != 'B' && BackOrderSelect)? bord_qty : ord_qty,
				(soln_rec.status [0] != 'B' && BackOrderSelect)? bord_val : ord_val,
				soln_rec.due_date,
				FALSE,
				BackOrderSelect
			);
			ord_val = cost_amt;
			if (soln_rec.status [0] != 'B' && BackOrderSelect)
				bord_val = cost_amt;
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}

	ProcessPhantom (inmr_rec.hhbr_hash, BackOrderSelect);

	if (NON_SYN)
	{
		open_rec (inmr3, inmr_list, INMR_NO_FIELDS, "inmr_hhsi_hash");
		ProcessSynonym (inmr_rec.hhbr_hash, BackOrderSelect);
	}

	abc_selfield (itln, "itln_hhbr_hash");

	if (BackOrderSelect)
		strcpy (val_stat, "BT");
	else
		strcpy (val_stat, "UM");

	itln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sptr = strchr (val_stat, itln_rec.status [0]);
		if (sptr == (char *) 0)
			stat_ok = FALSE;
		else
			stat_ok = TRUE;

		if (!stat_ok || (wh_flag == TRUE &&
				   itln_rec.i_hhcc_hash != currentHhccHash))
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		switch (itln_rec.status [0])
		{
		case	'B':
		case	'T':
			ord_qty = itln_rec.qty_border;
			break;

		case	'U':
		case	'M':
			ord_qty = itln_rec.qty_border + itln_rec.qty_order;
			break;

		default:
			ord_qty = 0.00;
			break;
		}
		if (ord_qty <= 0.00)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		ord_val = ord_qty;
		ord_val *= out_cost (itln_rec.cost, inmr_rec.outer_size);
		ord_qty *= CnvFct;

		strcpy (tran_date, DateToString (itln_rec.due_date));

		ccmr_rec.hhcc_hash	=	itln_rec.i_hhcc_hash;
		if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		{
			strcpy (ccmr_rec.est_no, "??");
			strcpy (ccmr_rec.cc_no, "??");
		}
		ithr_rec.hhit_hash = itln_rec.hhit_hash;
		if (find_rec (ithr, &ithr_rec, EQUAL, "r"))
			ithr_rec.del_no = 0L;

		sprintf (wk_cst_used, "%14.14s", comma_fmt (ord_val,"NNN,NNN,NNN.NN"));

		sprintf (tempStr [0], local_rec.rep_qty, ord_qty);
		sprintf (disp_str, "%s %-40.40s ^E%s^E%s^E%s^E%s^E%4.4s^E %14s ^E %06ld ^E%s^E %s ",

			mlStkDisp [97],
		  	(itln_rec.stock [0] == 'C') ? mlStkDisp [29] : mlStkDisp [30],
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			tran_date,
			tran_date,
			inum_rec.uom,
			tempStr [0],
			ithr_rec.del_no,
			itln_rec.status,
			wk_cst_used);

		Dsp_saverec (disp_str);
		o_tot_qty += (ord_qty/CnvFct);
		o_tot_val += CENTS (ord_val);
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	MendStock	=	CalcMendStk (incc_rec.hhwh_hash);
	if (MendStock)
	{
		sprintf (tempStr [0],local_rec.rep_qty, MendStock);
		sprintf (disp_str, " %s          ^E  ^E  ^E          ^E          ^E%4.4s^E %14s ^E        ^E ^E                ",
		mlStkDisp [32],
		inmr_rec.sale_unit,tempStr [0]);
		Dsp_saverec (disp_str);
		o_tot_qty += MendStock;
	}
	MendStock	=	CalcMendStk (incc_rec.hhwh_hash);
	if (realCommitted != 0.00)
	{
		sprintf (tempStr [0],local_rec.rep_qty, realCommitted);
		sprintf (disp_str, " %-48.48s ^E  ^E  ^E          ^E          ^E%4.4s^E %14s ^E        ^E ^E                ",
		mlStkDisp [123],
		inmr_rec.sale_unit,tempStr [0]);
		Dsp_saverec (disp_str);
		o_tot_qty += realCommitted;
	}

	Dsp_saverec ("^^GGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGHGGHGGGGGGGGGGHGGGGGGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGHGHGGGGGGGGGGGGGGGG^^");

	sprintf (wk_cst_used, "%14.14s", comma_fmt (DOLLARS (o_tot_val) ,"NNN,NNN,NNN.NN"));

	sprintf (tempStr [0], local_rec.rep_qty, o_tot_qty);
	sprintf (disp_str, " %s ^E                                         ^E  ^E  ^E          ^E          ^E    ^E %14s ^E        ^E ^E %14.14s ",
		mlStkDisp [96],
		tempStr [0],
		wk_cst_used);
	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGJGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGJGJGGGGGGGGGGGGGGGG^^");
	SillyBusyFunction (0);
	Dsp_srch_fn (InvoiceEnquiry);
	Dsp_close ();
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (sokt);
	if (envVarDbMcurr)
		abc_fclose (pocr);
	if (NON_SYN)
		abc_fclose (inmr3);
	NormalRedraw ();
}

void
ProcessOrderHeader (
	long	hhsoHash,
	float	quantityOrder,
	double	orderValue,
	long	dueDate,
	int		kitItemLine,
	int		BackOrderSelect)
{
	char	order_date [11];
	char	l_due_date [11];
	char	wk_cst_used [15];

	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (!cc)
	{
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec,COMPARISON, "r");
		if (!cc)
		{
			/*----------------------------
			| Convert to local currency. |
			----------------------------*/
			if (envVarDbMcurr)
			{
				strcpy (pocr_rec.co_no, comm_rec.co_no);
				sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
				cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
				if (cc || pocr_rec.ex1_factor == 0.00)
					pocr_rec.ex1_factor = 1.00;

				orderValue = DPP (orderValue / pocr_rec.ex1_factor);
			}

			sprintf (saveKey, "%04d%2.2s%1.1s%-8.8s%06ld",
						saveIndex++,
						sohr_rec.br_no, 
						soln_rec.status,
						sohr_rec.order_no,
						sohr_rec.hhcu_hash);

			strcpy (order_date, DateToString (sohr_rec.dt_raised));
			strcpy (l_due_date, DateToString (dueDate));

			if (soln_rec.bonus_flag [0] == 'Y')
				sprintf (wk_cst_used, "%14.14s", mlStkDisp [10]);
			else if (kitItemLine)
				sprintf (wk_cst_used, "%14.14s", mlStkDisp [118]);
			else
				sprintf (wk_cst_used, "%14.14s", comma_fmt (DOLLARS (orderValue),"NNN,NNN,NNN.NN"));

			cost_amt = orderValue;

			ccmr_rec.hhcc_hash = soln_rec.hhcc_hash;
			if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
			{
				strcpy (ccmr_rec.est_no, "??");
				strcpy (ccmr_rec.cc_no, "??");
			}
			sprintf (tempStr [0], local_rec.rep_qty, quantityOrder);
			sprintf (disp_str, " %s ^E %s^E%s^E%s^E%s^E%s^E%4.4s^E %14s ^E%-8.8s^E%s^E %s ",
					cumr_rec.dbt_no,
					cumr_rec.dbt_name,
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					order_date,
					l_due_date,
					inum_rec.uom,
					tempStr [0],
					sohr_rec.order_no,
					(soln_rec.status [0] != 'B' && BackOrderSelect)? " " : soln_rec.status,
					wk_cst_used);

			Dsp_save_fn (disp_str, saveKey);
			o_tot_qty += quantityOrder/CnvFct;
			if (soln_rec.bonus_flag [0] != 'Y')
				o_tot_val += orderValue;
		}
	}
}

void
ProcessPhantom (
	long	hhbrHash,
	int	BackOrderSelect)
{
	float	ord_qty = 0.00;
	float	bord_qty = 0.00;

	sokt_rec.mabr_hash = hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.mabr_hash == hhbrHash)
	{
		soln_rec.hhbr_hash = sokt_rec.hhbr_hash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhbr_hash == sokt_rec.hhbr_hash)
		{
			if (BackOrderSelect == TRUE && soln_rec.status [0] == 'B' && (soln_rec.qty_bord + soln_rec.qty_order)  <=  0.00)
			{
				cc = find_rec (soln, &soln_rec,NEXT,"r");
				continue;
			}

			if (BackOrderSelect == TRUE && soln_rec.status [0] != 'B' && soln_rec.qty_bord <= 0.00) 
			{
				cc = find_rec (soln, &soln_rec,NEXT,"r");
				continue;
			}

			if (BackOrderSelect == FALSE && soln_rec.status [0] == 'B')
			{
				cc = find_rec (soln, &soln_rec, NEXT,"r");
				continue;
			}
			if (wh_flag == TRUE && soln_rec.hhcc_hash != currentHhccHash)
			{
				cc = find_rec (soln, &soln_rec, NEXT,"r");
				continue;
			}

			if (soln_rec.qty_order + soln_rec.qty_bord)
			{
				ord_qty = soln_rec.qty_order + soln_rec.qty_bord;

				ord_qty *= sokt_rec.matl_qty;

				if (soln_rec.status [0] != 'B' && BackOrderSelect)
				{
					bord_qty = soln_rec.qty_bord;
					bord_qty *= sokt_rec.matl_qty;
				}

				ProcessOrderHeader 
				(
					soln_rec.hhso_hash,
					(soln_rec.status [0] != 'B' && BackOrderSelect)? bord_qty : ord_qty, 
					0.00,
					soln_rec.due_date,
					TRUE,
					BackOrderSelect
				);
			}
			cc = find_rec (soln, &soln_rec, NEXT,"r");
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
}

void
ProcessSynonym (
	long	hhbrHash,
	int		BackOrderSelect)
{
	float	ord_qty = 0.00;
	double	ord_val = 0.00;
	float	bord_qty = 0.00;
	double	bord_val = 0.00;


	inmr3_rec.hhsi_hash = hhbrHash;
	cc = find_rec (inmr3, &inmr3_rec, GTEQ, "r");
	while (!cc && inmr3_rec.hhsi_hash == hhbrHash)
	{
		soln_rec.hhbr_hash = inmr3_rec.hhbr_hash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhbr_hash == inmr3_rec.hhbr_hash)
		{
			if (BackOrderSelect == TRUE && soln_rec.status [0] == 'B' && (soln_rec.qty_bord + soln_rec.qty_order)  <=  0.00)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			if (BackOrderSelect == TRUE && soln_rec.status [0] != 'B' && soln_rec.qty_bord <= 0.00) 
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			if (BackOrderSelect == FALSE && soln_rec.status [0] == 'B')
			{
				soln_rec.hhbr_hash	=	inmr3_rec.hhbr_hash;
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			if (wh_flag == TRUE && soln_rec.hhcc_hash != currentHhccHash)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			if (soln_rec.qty_order + soln_rec.qty_bord)
			{
				ord_qty = soln_rec.qty_order + soln_rec.qty_bord;
				ord_val = soln_rec.sale_price;
				ord_val *= (double) ord_qty;
				ord_val -= CAL (ord_val, soln_rec.dis_pc);

				if (soln_rec.status [0] != 'B' && BackOrderSelect)
				{
					bord_qty = soln_rec.qty_bord;
					bord_val = soln_rec.sale_price;
					bord_val *= (double) bord_qty;
					bord_val -= CAL (bord_val, soln_rec.dis_pc);
				}

				ProcessOrderHeader 
				(
					soln_rec.hhso_hash, 
					(soln_rec.status [0] != 'B' && BackOrderSelect)? bord_qty : ord_qty, 
					(soln_rec.status [0] != 'B' && BackOrderSelect)? bord_val : ord_val,
					soln_rec.due_date,
					FALSE,
					BackOrderSelect
				);
			}
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		cc = find_rec (inmr3, &inmr3_rec, NEXT, "r");
	}
}

int
SerialEnquiry (
	char	*findKey)
{
	long	hhwhHash;
	char	serialNo [26],
			status [2];

	hhwhHash	=	atol (findKey + 4);
	sprintf (serialNo, "%-25.25s", findKey + 16);
	sprintf (status, "%-1.1s", findKey + 42);
	sprintf (err_str, "sk_serdisp %010ld %-25.25s %-1.1s", hhwhHash,serialNo,status);
	print_mess (ML ("Please wait, loading may take some time."));
	snorm ();
	sys_exec (err_str);
	swide ();
	Dsp_heading ();
	return (EXIT_SUCCESS);
}
/*
 * Load up shipment display.
 */
int
ShipmentEnquiry (
	char	*findKey)
{
	sprintf (err_str, "po_ship_disp \"%s\"", findKey);
	SystemExec (err_str, FALSE);
	swide ();
	Dsp_heading ();
	return (EXIT_SUCCESS);
}
/*===================================
| Load up Invoice || order display. |
===================================*/
int
InvoiceEnquiry (
	char	*findKey)
{
	char	_br_no [3];
	char	tmp_type [2];
	char	_type [2];
	char	_inv_no [9];
	long	_hhcu_hash;

	sprintf (_br_no, 	"%2.2s", findKey + 4);
	sprintf (tmp_type, 	"%1.1s", findKey + 6);
	sprintf (_inv_no, 	"%-8.8s", findKey + 7);
	_hhcu_hash = atol (findKey + 15);

	switch (tmp_type [0])
	{
	case '1':
	case '2':
	case '3':
		strcpy (_type, "I");
		break;

	default:
		strcpy (_type, "O");
		break;
	}

	RunSDisplay (comm_rec.co_no, 
	       	       _br_no, 
	              _type, 
	              _hhcu_hash,
	              _inv_no, (_type [0] == 'O') ? TRUE : FALSE);

	return (EXIT_SUCCESS);
}

/*=====================================================
| Execute invoice display passing relevent arguments. |
=====================================================*/
void
RunSDisplay (
 char	*_co_no,
 char	*_br_no,
 char	*_type,
 long	_hhcu_hash,
 char	*_inv_no,
 int	order_disp)
{
	char	*sptr;
	char	*tptr;
	int		indx = 0;
	char	run_string [110];

	sprintf (run_string, "%-2.2s %-2.2s %-1.1s %010ld %-8.8s",
		_co_no, _br_no, _type, _hhcu_hash, _inv_no);

#ifndef GVISION
	box (20,18,40,1);
	rv_pr (ML (mlStdMess035), 21,19,1);
#endif	/* GVISION */

	/*---------------------------------
	| Shell off and get item details. |
	---------------------------------*/
	arg [0] = (order_disp) ? "so_sdisplay" : "so_sinvdisp" ;

	tptr = (order_disp) ? get_env ("SO_ORDDISP") : get_env ("SO_INVDISP");

	sptr = strchr (tptr,'~');
	while (sptr != (char *)0)
	{
		*sptr = '\0';
		arg [++indx] = tptr;

		tptr = sptr + 1;
		sptr = strchr (tptr,'~');
	}
	arg [++indx] = run_string;
	arg [++indx] = (char *)0;

	shell_prog (2);
	swide ();
	Dsp_heading ();
}

int
PurchaseOrderDisplay (void)
{
	float	pord_qty 	= 0.00,
			total_qty 	= 0.00,
			tr_qty 		= 0.00,
			wo_qty 		= 0.00;

	double	workValue	= 0.00,
			pord_val 	= 0.00,
			tot_val 	= 0.00;

	char	tran_date [11];
	char	tran_value [15];

	SillyBusyFunction (1);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no_2");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");
	open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("  SUPP  |              SUPPLIER             |BR|   DATE   |   DATE   |UOM.|     ORDER      | PURCHASE ORDER | S |     AMOUNT     ");
	Dsp_saverec (" NUMBER |                NAME               |NO| ORDERED  |   DUE.   |    |      QTY       |     NUMBER     |   |                ");
	Dsp_saverec (std_foot);

	/*------------------
	| Purchase Orders. |
	------------------*/
	poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	poln_rec.hhpo_hash = 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag == TRUE && poln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		pord_qty = poln_rec.qty_ord - poln_rec.qty_rec;
		pord_val = (double) pord_qty;
		workValue	= 	twodec (poln_rec.land_cst);
		workValue	= 	out_cost (workValue, inmr_rec.outer_size);
		pord_val *= workValue;
		pord_qty *= CnvFct;
		cc = ProcessPohr 
			(
				poln_rec.hhpo_hash,
				pord_qty,
				pord_val,
				poln_rec.due_date,
				FALSE
			);

		if (!cc)
		{
			total_qty += (pord_qty/CnvFct);
			tot_val += (pord_val/CnvFct);
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	/*------------
	| Transfers. |
	------------*/
	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!VAL_ITLN || (wh_flag == TRUE &&
		       	  	   itln_rec.r_hhcc_hash != currentHhccHash))
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		tr_qty = itln_rec.qty_order + itln_rec.qty_border;

		if (tr_qty <= 0.00)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		pord_qty = tr_qty;

		pord_val = tr_qty;
		pord_val *= out_cost (itln_rec.cost, inmr_rec.outer_size);

		strcpy (tran_date, DateToString (itln_rec.due_date));
		sprintf (tran_value, "%14.14s", comma_fmt (pord_val,"NNN,NNN,NNN.NN"));

		ccmr_rec.hhcc_hash = itln_rec.r_hhcc_hash;
		if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		{
			strcpy (ccmr_rec.est_no, "??");
			strcpy (ccmr_rec.cc_no, "??");
		}
		ithr_rec.hhit_hash = itln_rec.hhit_hash;
		if (find_rec (ithr, &ithr_rec, EQUAL, "r"))
			ithr_rec.del_no = 0L;

		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		pord_qty *= CnvFct;

		sprintf (tempStr [0], local_rec.rep_qty, pord_qty);
		sprintf (disp_str, " %s %-33.33s ^E%s^E  W/H %s  ^E%s^E%4.4s^E %14s ^E    %08ld    ^E %s ^E %s ",
			mlStkDisp [97],
		   	(itln_rec.stock [0] == 'C') ? mlStkDisp [29] : mlStkDisp [30],
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			tran_date,
			inum_rec.uom,
			tempStr [0],
			ithr_rec.del_no,
			itln_rec.status,
			tran_value);

		Dsp_saverec (disp_str);
		total_qty += (pord_qty/CnvFct);
		tot_val += (pord_val/CnvFct);
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	/*---------------
	| Works Orders. |
	---------------*/
	pcwo_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*---------------------------------
		| Ignore anything not for current |
		| WH iff in warehouse mode.       |
		---------------------------------*/
		if (wh_flag == TRUE && pcwo_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		/*-----------------------------------------------
		| Ignore planned, closed, deleted works orders. |
		-----------------------------------------------*/
		if (pcwo_rec.order_status [0] == 'P' ||
		    pcwo_rec.order_status [0] == 'Z' ||
		    pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		wo_qty = NDEC (pcwo_rec.prod_qty) - 
			 	(NDEC (pcwo_rec.act_prod_qty) +
			 	NDEC (pcwo_rec.act_rej_qty));
		if (wo_qty <= 0.00)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		sprintf (tempStr [0], local_rec.rep_qty, wo_qty);
		sprintf (disp_str, 
			" %s                                ^E%s^E  W/H %s  ^E%10.10s^E%4.4s^E %14s ^E   %-9.9s    ^E %s ^E ",
			mlStkDisp [102],
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			DateToString (pcwo_rec.reqd_date),
			inmr_rec.sale_unit,
			tempStr [0],
			pcwo_rec.order_no,
			pcwo_rec.order_status);

		Dsp_saverec (disp_str);
		total_qty += wo_qty;

		cc = find_rec (pcwo,&pcwo_rec, NEXT, "r");
	}

	Dsp_saverec ("^^GGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGHGGGGGGGGGGHGGGGGGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGHGGGGGGGGGGGGGGGG^^");

	sprintf (tempStr [0], local_rec.rep_qty, total_qty);

	sprintf (tran_value, "%14.14s", comma_fmt (tot_val,"NNN,NNN,NNN.NN"));

	sprintf (disp_str, " %s ^E                                   ^E  ^E          ^E          ^E    ^E %14s ^E                ^E   ^E%s ",
		mlStkDisp [96],
		tempStr [0],
		tran_value);
	Dsp_saverec (disp_str);
	Dsp_saverec ("^^GGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGJGGGGGGGGGGGGGGGG^^");

	SillyBusyFunction (0);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poli);
	abc_fclose (pcwo);
	abc_fclose (pcms);
	Dsp_srch_fn (PurchaseShipment);
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}
int	
BatchHistoryDisplay (void)
{
	char	dateExpiry   [11],
			dateDispatch [11];

	SillyBusyFunction (1);
	open_rec (llih, llih_list, LLIH_NO_FIELDS, "llih_id_no2");

	lp_x_off = 1;
	lp_y_off = 4;
	Dsp_prn_open (1, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("CUSTOMER|                 NAME              | INVOICE  |   LOT   |SUPPLIER |  DESPATCH  |  EXPIRY    |   QUANTITY    ");
	Dsp_saverec (" NUMBER |                                   | NUMBER   |  NUMBER | LOT NO. |    DATE    |   DATE     |     SOLD      ");
	Dsp_saverec (std_foot);

	/*---------------------------------------
	| find transactions						|
	---------------------------------------*/
	llih_rec.hhbr_hash	=	hhsiHash;
	llih_rec.des_date	=	TodaysDate () + 30000L;
	cc = find_rec (llih, &llih_rec, LTEQ, "r");
	while (!cc && llih_rec.hhbr_hash == hhsiHash)
	{
		cumr_rec.hhcu_hash	=	llih_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, " ");
			strcpy (cumr_rec.dbt_name, " ");
		}
		strcpy (dateExpiry, 	DateToString (llih_rec.expiry_date));
		strcpy (dateDispatch,	DateToString (llih_rec.des_date)); 
		sprintf 
		(
			disp_str, 
			" %-6.6s |%-35.35s| %s | %s | %s | %s | %s | %10.2f ", 
			cumr_rec.dbt_no,
			cumr_rec.dbt_name,
			llih_rec.inv_no,
			llih_rec.lot_no,
			llih_rec.slot_no,
			dateDispatch,
			dateExpiry,
			llih_rec.qty
		);
		Dsp_saverec (disp_str);
		cc = find_rec (llih, &llih_rec, PREVIOUS, "r");
	}
	SillyBusyFunction (0);
	abc_fclose (llih);
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}
int
DropShipmentDisplay (void)
{
	float	pord_qty = 0.00,
			total_qty = 0.00;

	double	pord_val = 0.00,
			tot_val = 0.00;

	char	tran_value [15];

	SillyBusyFunction (1);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no_2");
	open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("  SUPP  |              SUPPLIER             |BR|   DATE   |   DATE   |UOM.|     ORDER      |      P/O       | S |     AMOUNT    ");
	Dsp_saverec (" NUMBER |                NAME               |NO| ORDERED  |   DUE.   |    |      QTY       |     NUMBER     |   |               ");
	Dsp_saverec (std_foot);

	/*------------------
	| Purchase Orders. |
	------------------*/
	poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	poln_rec.hhpo_hash = 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag == TRUE && poln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		pord_qty = poln_rec.qty_ord - poln_rec.qty_rec;

		pord_val	= 	twodec (poln_rec.land_cst);
		pord_val	= 	out_cost (pord_val, inmr_rec.outer_size);
		pord_val	*= 	(double) pord_qty;
		pord_qty 	*= CnvFct;

		cc = 	ProcessPohr 
			 	(
					poln_rec.hhpo_hash,
					pord_qty,
					pord_val,
					poln_rec.due_date,
					TRUE
				);

		if (!cc)
		{
			total_qty += pord_qty;
			tot_val += pord_val;
		}

		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	strcpy (disp_str, "^^GGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGHGGGGGGGGGGHGGGGGGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGHGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);

	sprintf (tran_value, "%14.14s", comma_fmt (tot_val,"NNN,NNN,NNN.NN"));

	sprintf (tempStr [0], local_rec.rep_qty, total_qty);
	sprintf (disp_str, " %s ^E                                   ^E  ^E          ^E          ^E    ^E %14s ^E                ^E   ^E%s ",
			mlStkDisp [96],
			tempStr [0],
			tran_value);
	Dsp_saverec (disp_str);
	SillyBusyFunction (0);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poli);
	Dsp_srch_fn (PurchaseShipment);
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
ProcessPohr (
 long	hhpo_hash,
 float	pord_qty,
 double	pord_val,
 long	due_date,
 int	dropship_ok)
{
	char	order_date [11];
	char	l_due_date [11];
	char	hhpl_char [9];
	char	tran_value [15];

	pohr_rec.hhpo_hash	=	hhpo_hash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (!cc)
	{
		if (pohr_rec.drop_ship [0] == 'Y' && !dropship_ok)
			return (EXIT_FAILURE);

		if (pohr_rec.drop_ship [0] != 'Y' && dropship_ok)
			return (EXIT_FAILURE);

		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (order_date, DateToString (pohr_rec.date_raised));
			strcpy (l_due_date, DateToString (due_date));
			if (due_date == 0L)
				strcpy (l_due_date, "          ");

			if (order_date == 0L)
				strcpy (order_date, "          ");

			sprintf (tempStr [0], local_rec.rep_qty, pord_qty);

			sprintf (tran_value, "%14.14s", comma_fmt (pord_val,"NNN,NNN,NNN.NN"));
			sprintf (disp_str, " %s ^E%-35.35s^E%s^E%s^E%s^E%4.4s^E %14s ^E%s%c^E %s ^E%s ",
				sumr_rec.crd_no,
				sumr_rec.crd_name,
				pohr_rec.br_no,
				order_date,
				l_due_date,
				inum_rec.uom,
				tempStr [0],
				pohr_rec.pur_ord_no,
				(poln_rec.ship_no) ? '*' : ' ',
				poln_rec.pur_status,
				tran_value);

			sprintf (hhpl_char, "%08ld", poln_rec.hhpl_hash);
			Dsp_save_fn (disp_str, hhpl_char);

			/*--------------------------------------
			| Check if P/O Information record exists
			--------------------------------------*/
			poli_rec.hhpl_hash	=	poln_rec.hhpl_hash;
			cc = find_rec (poli, &poli_rec, EQUAL, "r");
			if (!cc && (poli_rec.cont_date 	> 0L ||
						poli_rec.ship_date 	> 0L ||
						poli_rec.eta_date 	> 0L ||
						strcmp (poli_rec.comment,"                        ")))
			{
				char	cont_date [11];
				char	ship_date [11];
				char	eta_date [11];
				
				strcpy (cont_date, 	(poli_rec.cont_date > 0L) 
						? DateToString (poli_rec.cont_date) : "None.");

				strcpy (ship_date, 	(poli_rec.ship_date > 0L) 
						? DateToString (poli_rec.ship_date) : "None.");

				strcpy (eta_date, 	(poli_rec.eta_date > 0L)
						? DateToString (poli_rec.eta_date) : "None.");
				
				sprintf (disp_str,"        ^E  %10.10s / Comment %s / Shipment Date : %10.10s  ETA Date :%10.10s    ",
									cont_date,
									poli_rec.comment,
									ship_date,
									eta_date);
		
				Dsp_saverec (disp_str);
			}
			return (EXIT_SUCCESS);
		}
	}
	return (EXIT_FAILURE);
}

int
DspShipments (void)
{
	char	wk_date1 [11],
			wk_date2 [11];
	float	pord_qty;
	char	dsp_method [5];

	SillyBusyFunction (1);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no_2");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (3, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec (" SHIPMENT      | SHIP |PURCHASE ORDER |UOM.|  QUANTITY   |  QUANTITY   |    VESSEL  NAME    |   ETD    |   ETA    | SUPPLIER ");
	Dsp_saverec ("  NUMBER       |  BY  |     NUMBER    |    |   TO SHIP   |   ORDERED   |                    |   DATE   |   DATE   |  NUMBER  ");
	Dsp_saverec (std_foot);

	/*------------------
	| Purchase Orders. |
	------------------*/
	poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	poln_rec.hhpo_hash = 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag == TRUE && poln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		pord_qty = poln_rec.qty_ord - poln_rec.qty_rec;

		posl_rec.hhpl_hash = poln_rec.hhpl_hash;
		cc = find_rec (posl, &posl_rec, GTEQ, "r");
		while (!cc && posl_rec.hhpl_hash == poln_rec.hhpl_hash)
		{
			if (posl_rec.ship_qty <= 0.00)
			{
				cc = find_rec (posl, &posl_rec, NEXT, "r");
				continue;
			}
			strcpy (posh_rec.co_no, comm_rec.co_no);
			posh_rec.hhsh_hash = posl_rec.hhsh_hash;
			cc = find_rec (posh, &posh_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (posl, &posl_rec, NEXT, "r");
				continue;
			}
			pohr_rec.hhpo_hash = poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (posl, &posl_rec, NEXT, "r");
				continue;
			}
			sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (!cc)
			{
				if (posh_rec.ship_method [0] == 'A')
					strcpy (dsp_method, mlStkDisp [2]);
				else if (posh_rec.ship_method [0] == 'S')
					strcpy (dsp_method, mlStkDisp [67]);
				else if (posh_rec.ship_method [0] == 'L')
					strcpy (dsp_method, mlStkDisp [35]);
				else 
					strcpy (dsp_method, "????");

				strcpy (wk_date1,DateToString (posh_rec.ship_depart));
				strcpy (wk_date2,DateToString (posh_rec.ship_arrive));
				if (posh_rec.ship_depart == 0L)
					strcpy (wk_date1, "          ");
				if (posh_rec.ship_arrive == 0L)
					strcpy (wk_date2, "          ");

				inum_rec.hhum_hash	=	poln_rec.hhum_hash;
				cc = find_rec (inum, &inum_rec, COMPARISON, "r");
				PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
				CnvFct	=	StdCnvFct / PurCnvFct;

				sprintf 
				(
					disp_str, 
					"  %-12.12s ^E %4.4s ^E%15.15s^E%4.4s^E%12.2f ^E%12.2f ^E%20.20s^E%s^E%s^E %s",
					posh_rec.csm_no,
					dsp_method,
					pohr_rec.pur_ord_no,
					inum_rec.uom,
					posl_rec.ship_qty * CnvFct,
					pord_qty * CnvFct,
					posh_rec.vessel,
					wk_date1,
					wk_date2,
					sumr_rec.crd_no
				);
				sprintf (err_str, "%010ld", posh_rec.hhsh_hash);
				Dsp_save_fn (disp_str, err_str);
			}
			cc = find_rec (posl, &posl_rec, NEXT, "r");
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	SillyBusyFunction (0);
	Dsp_srch_fn (ShipmentEnquiry);
	Dsp_close ();
	NormalRedraw ();
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	abc_fclose (posl);
    return (EXIT_SUCCESS);
}

int
PurchaseShipment (
 char	*hhpl_char)
{
	long	hhplHash;
	char	wk_date1 [11],
			wk_date2 [11];

	char	dsp_method [5];

	hhplHash = atol (hhpl_char);

	SillyBusyFunction (1);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("            PURCHASE ORDER LINE DETAIL INFORMATION SCREEN.                    ");

	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, EQUAL, "r");
	if (cc)
	{
		abc_fclose (sumr);
		abc_fclose (pohr);
		abc_fclose (poln);
		abc_fclose (posh);
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_SUCCESS);
	}
	pohr_rec.hhpo_hash = poln_rec.hhpo_hash;
	cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
	if (cc)
	{
		abc_fclose (sumr);
		abc_fclose (pohr);
		abc_fclose (poln);
		abc_fclose (posh);
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_SUCCESS);
	}
	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	{
		abc_fclose (sumr);
		abc_fclose (pohr);
		abc_fclose (poln);
		abc_fclose (posh);
		SillyBusyFunction (0);
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_SUCCESS);
	}
	sprintf (disp_str, " %s  : %s %5.5s %s: %s ",
						mlStkDisp [53], pohr_rec.pur_ord_no, " ",
						mlStkDisp [16], pohr_rec.contact);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %s : %s - %s ", sumr_rec.crd_no,
				                   mlStkDisp [94],sumr_rec.crd_name);
	Dsp_saverec (disp_str);

	if (strcmp (pohr_rec.delin1, sixtySpace))
	{
		sprintf (disp_str," %s : %s ",mlStkDisp [21],pohr_rec.delin1);
		Dsp_saverec (disp_str);
	}

	if (strcmp (pohr_rec.delin2, sixtySpace))
	{
		sprintf (disp_str," %s : %s ",mlStkDisp [22],pohr_rec.delin2);
		Dsp_saverec (disp_str);
	}

	if (strcmp (pohr_rec.delin3, sixtySpace))
	{
		sprintf (disp_str," %s : %s ",mlStkDisp [23],pohr_rec.delin3);
		Dsp_saverec (disp_str);
	}

	sprintf (disp_str, " %s   : %6.4f - %s", 
							 mlStkDisp [27],
							 poln_rec.exch_rate,
						     pohr_rec.curr_code);
	Dsp_saverec (disp_str);

	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = poln_rec.ship_no;
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (!cc)
	{
		sprintf (disp_str, "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^ ^1 %s ^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG", mlStkDisp [69]);
		Dsp_saverec (disp_str);

		strcpy (wk_date1, DateToString (posh_rec.ship_depart));
		strcpy (wk_date2, DateToString (posh_rec.ship_arrive));

		sprintf (disp_str, " %s : %s ", mlStkDisp [79],posh_rec.csm_no);
		Dsp_saverec (disp_str);

		if (posh_rec.ship_method [0] == 'A')
			strcpy (dsp_method, mlStkDisp [2]);
		else if (posh_rec.ship_method [0] == 'S')
			strcpy (dsp_method, mlStkDisp [67]);
		else if (posh_rec.ship_method [0] == 'L')
			strcpy (dsp_method, mlStkDisp [35]);
		else 
			strcpy (dsp_method, "????");

		sprintf (disp_str, " %s : %s %17.17s %s      : %s",
				mlStkDisp [77], dsp_method, " ",
				mlStkDisp [101], posh_rec.vessel);
		Dsp_saverec (disp_str);

		sprintf (disp_str, " %s : %s %11.11s %s : %s",
				mlStkDisp [76], wk_date1, " ", 
				mlStkDisp [75], wk_date2);
		Dsp_saverec (disp_str);

		sprintf (disp_str, " %s : %s   %s : %s",
				mlStkDisp [54], posh_rec.port,
				mlStkDisp [24], posh_rec.destination);
		Dsp_saverec (disp_str);

		sprintf (disp_str, " %s   : %6.4f - %s",
						mlStkDisp [27],
						posh_rec.ex_rate,
						posh_rec.curr_code);
		Dsp_saverec (disp_str);
	}
	else
	{
		sprintf (disp_str, "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGG^^ ^1 %s ^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGG", mlStkDisp [46]);
		Dsp_saverec (disp_str);
	}
	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	return (EXIT_SUCCESS);
}

int
SerialDisplay (void)
{
	lp_x_off = 0;
	lp_y_off = 4;
	saveIndex = 0;
	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("                        S E R I A L   N U M B E R S   D I S P L A Y                         ");
	if (displayCostOk)
		Dsp_saverec (" BR | WH |       Serial Number.      | Receipt | Status. |   Value.     | Number |  Acronym ");
	else
		Dsp_saverec (" BR | WH |       Serial Number.      | Receipt | Status. |              | Number |  Acronym. ");

	Dsp_saverec (std_foot);

	if (inmr_rec.serial_item [0] != 'Y')
	{
		Dsp_saverec (" ");
		Dsp_saverec (" ITEM IS NOT A SERIAL ITEM ");
		Dsp_saverec (" ");
	}
	else
	{
		if (wh_flag == TRUE)
		{
			LoadWarehouseSerial ("F", "Free");
			LoadWarehouseSerial ("C", "Commit");
			LoadWarehouseSerial ("T", "In Tran");
			LoadWarehouseSerial ("S", "Sold");
		}
		else
		{
			LoadCompanySerial ("F", "Free");
			LoadCompanySerial ("C", "Commit");
			LoadCompanySerial ("T", "In Tran");
			LoadCompanySerial ("S", "Sold");
		}
	}
	Dsp_srch_fn (SerialEnquiry);
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

void
LoadWarehouseSerial (
 char *status, 
 char *stat_desc)
{
	abc_selfield (ccmr, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (!cc)
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
			ProcessInsf (incc_rec.hhwh_hash, status, stat_desc);
	}
}

void
LoadCompanySerial (
 char	*status,
 char	*stat_desc)
{
	abc_selfield (ccmr, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = hhsiHash; 
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
			ProcessInsf (incc_rec.hhwh_hash, status, stat_desc);

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
}

void
ProcessInsf (
	long	hhwhHash,
	char	*status,
	char	*stat_desc)
{
	char	number [7];
	char	acronym [10];
	int		serial_ok;

	cc = FindInsf (hhwhHash, 0L,"", status, "r");
	while (!cc && insfRec.hhwh_hash == hhwhHash &&
		      insfRec.status [0] == status [0])
	{
		if (status [0] == 'S' && insfRec.hhcu_hash != 0L)
		{
			cumr_rec.hhcu_hash	=	insfRec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (cc)
				serial_ok = 0;
			else
			{
				sprintf (number, "%-6.6s", cumr_rec.dbt_no);
				sprintf (acronym, "%-9.9s", cumr_rec.dbt_acronym);
				serial_ok = 1;
			}
		}
		else
		{
			strcpy (number, "  N/A ");
			strcpy (acronym, "   N/A   ");
			serial_ok = 1;
		}
		if (serial_ok)
		{
			if (displayCostOk)
			{
				sprintf (disp_str, " %s ^E %s ^E %s ^E   %-3.3s   ^E %-7.7s ^E %12.2f ^E %s ^E %s ",
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					insfRec.serial_no,
					(insfRec.receipted [0] == 'N') ? mlStkDisp [49] : mlStkDisp [107],
					ML (stat_desc),
					ser_value (insfRec.est_cost, insfRec.act_cost),
					number,
					acronym);
			}
			else
			{
				sprintf (disp_str, " %s ^E %s ^E %s ^E   %-3.3s   ^E %-7.7s ^E              ^E %s ^E %s ",
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					insfRec.serial_no,
					(insfRec.receipted [0] == 'N') ? mlStkDisp [49] : mlStkDisp [107],
					ML (stat_desc),
					number,
					acronym);
			}
			sprintf (saveKey, "%04d %010ld %-25.25s %1.1s",
						saveIndex++,
						insfRec.hhwh_hash, 
						insfRec.serial_no,
						insfRec.status);

			Dsp_save_fn (disp_str, saveKey);
		}
		cc = FindInsf (0L, 0L,"", status, "r");
	}
}

/*------------------------------
| Display normal Requisitions. |
------------------------------*/
int
RequisitionsDisplay (void)
{
	ProcessRequisition ("R");
	return (EXIT_SUCCESS);
}

/*---------------------------------
| Display Backorder Requisitions. |
---------------------------------*/
int
RequisitionsBordDisplay (void)
{
	ProcessRequisition ("B");
	return (EXIT_SUCCESS);

}

/*-------------------------------
| Display Forward Requisitions. |
-------------------------------*/
int
RequisitionsForward (void)
{
	ProcessRequisition ("F");
	return (EXIT_SUCCESS);
}

void
ProcessRequisition (
 char	*req_stat)
{

	float	ord_qty = 0.00;
	double	ord_val = 0.00;
	char	req_date [11];
	char	rqrd_date [11];

	o_tot_qty = 0.00;
	o_tot_val = 0.00;

	SillyBusyFunction (1);

	open_rec (cmrh, cmrh_list, CMRH_NO_FIELDS, "cmrh_hhrq_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_hhbr_hash");

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 11, 
		      headingText, 
		      comm_rec.co_no, comm_rec.co_name,
		      comm_rec.est_no, comm_rec.est_name,
		      (char *) 0, (char *) 0);

	Dsp_saverec (" CONTRACT | BR | WH |REQUISITION|  ORDER   |   DUE    |     ORDER      |     AMOUNT     ");
	Dsp_saverec (" NUMBER   | NO | NO |  NUMBER   |   DATE   |   DATE   |      QTY       |                ");
	Dsp_saverec (std_foot);

	cmrd_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*-------------------
		| Check line status |
		-------------------*/
		if (cmrd_rec.stat_flag [0] != req_stat [0])
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}
		/*-------------------------------
		| If in WH mode check that line |
		| relates to current warehouse. |
		-------------------------------*/
		if (wh_flag == TRUE && cmrd_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		/*-------------------
		| Find ccmr record. |
		-------------------*/
		ccmr_rec.hhcc_hash	= cmrd_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT,"r");
			continue;
		}

		cmrd_rec.qty_order = NDEC (cmrd_rec.qty_order);
		cmrd_rec.qty_border = NDEC (cmrd_rec.qty_border);
		if ((cmrd_rec.qty_order + cmrd_rec.qty_border) > 0.00)
		{
			ord_qty = cmrd_rec.qty_order + cmrd_rec.qty_border;
			ord_val = out_cost (cmrd_rec.sale_price, inmr_rec.outer_size);
			ord_val *= (double) ord_qty;
			ord_val -= CAL (ord_val, cmrd_rec.disc_pc);
		
			/*----------------------------
			| Lookup requisition header. |
			----------------------------*/
			cmrh_rec.hhrq_hash = cmrd_rec.hhrq_hash;
			cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
				continue;
			}

			/*-------------------------
			| Lookup contract header. |
			-------------------------*/
			cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
			cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (cmrd,&cmrd_rec, NEXT,"r");
				continue;
			}

			/*--------------------------
			| Save line to Dsp window. |
			--------------------------*/
			strcpy (req_date, DateToString (cmrh_rec.req_date));
			strcpy (rqrd_date,DateToString (cmrh_rec.rqrd_date));
			sprintf (tempStr [0], local_rec.rep_qty, ord_qty);
			sprintf (disp_str,
				"  %s  ^E %s ^E %s ^E  %06ld   ^E%s^E%s^E %14s ^E %12.2f ",
				cmhr_rec.cont_no,
				ccmr_rec.est_no,
				ccmr_rec.cc_no,
				cmrh_rec.req_no,
				req_date,
				rqrd_date,
				tempStr [0],
				DOLLARS (ord_val));

			Dsp_saverec (disp_str);

			o_tot_qty += ord_qty;
			o_tot_val += ord_val;
		}
		cc = find_rec (cmrd, &cmrd_rec, NEXT,"r");
	}

	strcpy (disp_str, "^^GGGGGGGGGGHGGGGHGGGGHGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);
	sprintf (tempStr [0], local_rec.rep_qty, o_tot_qty);
	sprintf (disp_str,
		" %s   ^E    ^E    ^E           ^E          ^E          ^E %14s ^E %12.2f ",
		mlStkDisp [96],
		tempStr [0],
		DOLLARS (o_tot_val));
	Dsp_saverec (disp_str);

	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (cmhr);
	abc_fclose (cmrh);
	abc_fclose (cmrd);
	NormalRedraw ();
}

int
RoyaltyDisplay (void)
{
	char	basis_desc [17];

	open_rec (rymr, rymr_list, RYMR_NO_FIELDS, "rymr_id_no");
	open_rec (ryhr, ryhr_list, RYHR_NO_FIELDS, "ryhr_hhbr_hash");

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec (" Royalty Code |  Royalty Description                     | Basis            ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	ryhr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (ryhr, &ryhr_rec, GTEQ, "r");
	while (!cc && ryhr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		switch (ryhr_rec.basis [0])
		{
		case	'R':
			strcpy (basis_desc, "Retail");
			break;

		case	'N':
			strcpy (basis_desc, "Nett Price");
			break;

		case	'A':
			strcpy (basis_desc, "Absolute Value");
			break;

		default:
			strcpy (basis_desc, "Obsolete Royalty");
			break;
		}
		strcpy (rymr_rec.co_no, comm_rec.co_no);
		strcpy (rymr_rec.code, ryhr_rec.code);
		cc = find_rec (rymr, &rymr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (disp_str, "   %s  ^E %-40.40s ^E %-16.16s ",
				ryhr_rec.code,
				rymr_rec.desc,
				ML (basis_desc));
			Dsp_saverec (disp_str);
		}
		cc = find_rec (ryhr, &ryhr_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (rymr);
	abc_fclose (ryhr);
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

float	
pwr (
 float a, 
 int x)
{
	float	pwr (float a, int x);

	if (x == 0)
		return (1.0);
	return (pwr (a, x - 1) * a);
}

void
FindCost (void)
{
	if (!wh_flag)
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = currentHhccHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	}

	if (FindInei (hhsiHash, comm_rec.est_no, "r"))
	{
		ineiRec.min_stock = 0.00;
		ineiRec.max_stock = 0.00;
	}
	switch (inmr_rec.costing_flag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		local_rec.l_cost	=	FindIneiCosts
								(
									inmr_rec.costing_flag,
									comm_rec.est_no, 
									hhsiHash
								);
		break;
	case 'F':
		local_rec.l_cost	=	FindIncfValue 
								(
									incc_rec.hhwh_hash,
									NDEC (incc_rec.closing_stock), 
									TRUE, 
									TRUE,
									inmr_rec.dec_pt
								);
		break;

	case 'I':
		local_rec.l_cost	=	FindIncfValue 
								(
									incc_rec.hhwh_hash,
									NDEC (incc_rec.closing_stock), 
									TRUE, 
									FALSE,
									inmr_rec.dec_pt
								);
		break;

	case 'S':
		local_rec.l_cost = FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;
	}

	if (local_rec.l_cost < 0.00)
		local_rec.l_cost = FindIneiCosts ("L", comm_rec.est_no, hhsiHash);

	local_rec.avge_cost = FindIneiCosts ("A", comm_rec.est_no, hhsiHash);
	local_rec.last_cost = FindIneiCosts ("L", comm_rec.est_no, hhsiHash);
	local_rec.prev_cost = FindIneiCosts ("P", comm_rec.est_no, hhsiHash);
	local_rec.std_cost  = FindIneiCosts ("T", comm_rec.est_no, hhsiHash);
}

void
AlternateMess (
 char *str)
{
	move (0, input_row);
	cl_line ();
	rv_pr (str, 1, input_row, 1);
}

/*============================================
| Process forward Schedule of Committed etc. |
============================================*/
int
ForwardSchedule (void)
{
	char	*sptr,
			val_methods [27],
			min_supp [10],
			max_supp [10];

	int		st_mth = 0,
			ed_mth = 0,
			st_yr = 0,
			ed_yr = 0,
			i = 0,
			j = 0,
			k = 0,
			insert_mth = 0,
			first_inis = TRUE,
			best_method;

	long	need_date;

	float	pord_qty = 0.00,
			fwd_qty = 0.00,
			min_lead = 0.00,
			max_lead = 0.00,
			tmp_avail,
			qty,
			cnv_fct;

	if (wh_flag == FALSE && ff_flag != 0)
	{
		print_mess (ML (mlSkMess242));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	SillyBusyFunction (1);

	for (i = 0; i < 6; i++)
	{
		qtyOnHand [i] 		= 0.00;
		qtyCommit [i]  		= 0.00;
		qtyBackOrd [i] 		= 0.00;
		qtyForward [i]   	= 0.00;
		qtyOrder [i]   		= 0.00;
		q_project [0] [i]	= 0.00;
		q_project [1] [i]	= 0.00;
		q_project [2] [i]	= 0.00;
		q_project [3] [i]	= 0.00;
		avail [i]     	= 0.00;
	}
	strcpy (min_supp, mlStkDisp [47]);
	strcpy (max_supp, mlStkDisp [47]);

	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "  ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*---------------------------------------
		| If this is a 'lead-date' supplier,	|
		| calculate the nearest lead-date.	    |
		---------------------------------------*/
		if (inis_rec.lead_time == (float) 0.00)
			inis_rec.lead_time = GetLeadDate (inis_rec.hhis_hash, comm_rec.inv_date);

		sumr_rec.hhsu_hash		=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (sumr_rec.acronym, mlStkDisp [100]);
		
		if (first_inis)
		{
			min_lead = inis_rec.lead_time;
			strcpy (min_supp, sumr_rec.acronym);

			max_lead = inis_rec.lead_time;
			strcpy (max_supp, sumr_rec.acronym);
		}
		first_inis = FALSE;

		if (inis_rec.lead_time < min_lead)
		{
			min_lead = inis_rec.lead_time;
			strcpy (min_supp, sumr_rec.acronym);
		}

		if (inis_rec.lead_time > max_lead)
		{
			max_lead = inis_rec.lead_time;
			strcpy (max_supp, sumr_rec.acronym);
		}

		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}

	abc_fclose (inis);
	abc_fclose (sumr);

	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");

	st_mth = mdy [1];
	st_yr = mdy [2];
	if (st_yr < 1900)
		st_yr += 1900;

	ed_mth = st_mth + 5;
	ed_yr = st_yr;
	if (ed_mth > 12)
	{
		ed_mth -= 12;
		ed_yr++;
	}

	for (i = 0; i < 12;i++)
		sprintf (rangeHeader [i], "%-12.12s", " ");

	if (st_yr != ed_yr)
	{
		for (i = st_mth, j = 0; i <= 12; i++, j++)
			sprintf (rangeHeader [j], " %3.3s  %4d ", InternalMonthName (i-1), st_yr);

		for (i = 1, k = j; i <= ed_mth; i++, k++)
			sprintf (rangeHeader [k], " %3.3s  %4d ", InternalMonthName (i-1), ed_yr);
	}
	else
	{
		for (i = st_mth, j = 0; i <= ed_mth; i++, j++)
			sprintf (rangeHeader [j], " %3.3s  %4d ", InternalMonthName (i-1), st_yr);
	}

	/*-------------------------------
	| Calculate forecasted demand	|
	-------------------------------*/
	sptr = get_env ("LRP_METHODS");
	strcpy (val_methods, (sptr) ? sptr : "ABCD");

	calc_LSA 
	(
		val_methods, 
		hhsiHash,
		comm_rec.inv_date, 
		FALSE,
		36,
		LRP_PASSED_MONTH,
		"1"
	);
	best_method = incc_rec.ff_method [0] - 'A';

	for (i = 0; i < 6; i++)
	{
		q_project [0] [i] = NDEC (LSA_result [1] [i + 35]);
		q_project [1] [i] = NDEC (LSA_result [2] [i + 35]);
		q_project [2] [i] = NDEC (LSA_result [3] [i + 35]);
		q_project [3] [i] = NDEC (LSA_result [4] [i + 35]);
	}

	if (wh_flag)
	{
		incc_rec.hhbr_hash = hhsiHash;
		incc_rec.hhcc_hash = currentHhccHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			qtyOnHand [0] 	= NDEC (incc_rec.closing_stock);
			qtyCommit [0]  	= NDEC (incc_rec.committed);
			qtyBackOrd [0] 	= NDEC (incc_rec.backorder);
			avail [0]     	= NDEC (incc_rec.closing_stock) -
							  NDEC (incc_rec.committed);
			avail [0] -= RealTimeCommitted (inmr_rec.hhbr_hash, currentHhccHash);
		}
	}
	else
	{
		qtyOnHand [0]	= NDEC (inmr_rec.on_hand);
		qtyBackOrd [0]	= NDEC (inmr_rec.backorder);
		avail [0]		= NDEC (inmr_rec.on_hand) -
			       		  NDEC (inmr_rec.committed);
		avail [0] 		-= RealTimeCommitted (inmr_rec.hhbr_hash, 0L);
	}

	abc_selfield (pcwo, "pcwo_hhwo_hash");
	/*-----------------------------------------
	| Process BOM to calculate committed qty. |
	-----------------------------------------*/
	pcms_rec.mabr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.mabr_hash == inmr_rec.hhbr_hash)
	{
		/*---------------------------------------
		| check works order, if deleted, closed |
		| get next pcms record                  |
		---------------------------------------*/
		pcwo_rec.hhwo_hash	= pcms_rec.hhwo_hash;
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "pcwo", "DBFIND");

		if (pcwo_rec.order_status [0] == 'Z' ||
			pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		qty = pcms_rec.matl_qty;			/* qty of source required */
		pcms_rec.matl_wst_pc += 100;		/* calculate wastage % */
		pcms_rec.matl_wst_pc /= 100;
		qty *= pcms_rec.matl_wst_pc;		/* add wastage % */
		qty -= pcms_rec.qty_issued;			/* less actual qty issued */
		qty = NDEC (qty);
		cnv_fct = GetUom (pcms_rec.uom);
		qty /= cnv_fct;

		if (qty <= 0.00 ||
			(currentHhccHash != 0L && pcwo_rec.hhcc_hash != currentHhccHash))
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		insert_mth = GetPeriodMonth (pcwo_rec.reqd_date);
		qtyOrder [insert_mth] += qty;

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if ((wh_flag && poln_rec.hhcc_hash != currentHhccHash) ||
			(pohr_rec.drop_ship [0] == 'Y'))
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		pord_qty = NDEC (poln_rec.qty_ord) - NDEC (poln_rec.qty_rec);
		if (pord_qty > 0.00)
		{
			insert_mth = GetPeriodMonth (poln_rec.due_date);
			qtyOrder [insert_mth] += pord_qty;
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	abc_selfield (pcwo, "pcwo_hhbr_hash");
	/*------------------------------------------------
	| Process works orders to calculate on orer qty. |
	------------------------------------------------*/
	pcwo_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*------------------------------------------------------------
		| check works order, if deleted, closed get next pcms record |
		------------------------------------------------------------*/
		if (pcwo_rec.order_status [0] == 'Z' ||
			pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
			
		qty = pcwo_rec.prod_qty - (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
		qty = NDEC (qty);

		if (qty <= 0.00 ||
			(currentHhccHash != 0L && pcwo_rec.hhcc_hash != currentHhccHash))
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		if (wh_flag)
		{
			need_date = (long) incc_rec.lead_time * 7;
			insert_mth = GetPeriodMonth (pcwo_rec.reqd_date - need_date);
		}
		else
			insert_mth = GetPeriodMonth (pcwo_rec.reqd_date);
		qtyOrder [insert_mth] += qty;

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
	soln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (wh_flag && soln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		fwd_qty = NDEC (soln_rec.qty_order) + NDEC (soln_rec.qty_bord);
		if (FORWARD && fwd_qty > 0.00)
		{
			if (soln_rec.due_date >= startMonth)
				insert_mth = GetPeriodMonth (soln_rec.due_date);
			else
				insert_mth = 0;

			qtyForward [insert_mth] += fwd_qty;
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 9, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	sprintf (disp_str, "     %s    :   %s %6.2f  %s (%9.9s)      /      %s %6.2f  %s (%9.9s)    %-20.20s ",
			mlStkDisp [93],
			mlStkDisp [45],	min_lead,
			mlStkDisp [104], min_supp, 
			mlStkDisp [43],  max_lead, 
			mlStkDisp [104], max_supp, " ");

	Dsp_saverec (disp_str);

	if (wh_flag)
	{
		sprintf (disp_str, "%s %6.2f %s|  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s+  ",
			mlStkDisp [39],
			incc_rec.lead_time,
			mlStkDisp [104],
			rangeHeader [0], rangeHeader [1],
			rangeHeader [2], rangeHeader [3],
			rangeHeader [4], rangeHeader [5]);
	}
	else
	{
		sprintf (disp_str, "%-23.23s|  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s  |  %-12.12s+  ",
			" ",
			rangeHeader [0], rangeHeader [1],
			rangeHeader [2], rangeHeader [3],
			rangeHeader [4], rangeHeader [5]);
	}
	Dsp_saverec (disp_str);
	Dsp_saverec (std_foot);

	/*---------------------------------------
	| Subtract the maximum of actual		|
	| sales OR projected sales				|
	---------------------------------------*/
	if (wh_flag)
		tmp_avail = NDEC (incc_rec.backorder + qtyForward [0]);
	else
		tmp_avail = NDEC (inmr_rec.backorder + qtyForward [0]);
	
	if (tmp_avail < q_project [best_method] [0])
		tmp_avail = q_project [best_method] [0];
	avail [0] += qtyOrder [0] - tmp_avail;

	for (i = 1; i < 6; i++)
	{
		qtyOnHand [i] = avail [i-1];
		avail [i] = avail [i-1] + qtyOrder [i];
		if (qtyForward [i] < q_project [best_method] [i])
			avail [i] -= q_project [best_method] [i];
		else
			avail [i] -= qtyForward [i];
	}

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (qtyOnHand [0]));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (qtyOnHand [1]));
	sprintf (tempStr [2], local_rec.rep_qty, NDEC (qtyOnHand [2]));
	sprintf (tempStr [3], local_rec.rep_qty, NDEC (qtyOnHand [3]));
	sprintf (tempStr [4], local_rec.rep_qty, NDEC (qtyOnHand [4]));
	sprintf (tempStr [5], local_rec.rep_qty, NDEC (qtyOnHand [5]));
	sprintf (disp_str, " %-23.23s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			mlStkDisp [110],
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (qtyCommit [0]));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (qtyCommit [1]));
	sprintf (tempStr [2], local_rec.rep_qty, NDEC (qtyCommit [2]));
	sprintf (tempStr [3], local_rec.rep_qty, NDEC (qtyCommit [3]));
	sprintf (tempStr [4], local_rec.rep_qty, NDEC (qtyCommit [4]));
	sprintf (tempStr [5], local_rec.rep_qty, NDEC (qtyCommit [5]));
	sprintf (disp_str, " %-23.23s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			mlStkDisp [14],
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty,NDEC (qtyBackOrd [0] + qtyForward [0]));
	sprintf (tempStr [1], local_rec.rep_qty,NDEC (qtyBackOrd [1] + qtyForward [1]));
	sprintf (tempStr [2], local_rec.rep_qty,NDEC (qtyBackOrd [2] + qtyForward [2]));
	sprintf (tempStr [3], local_rec.rep_qty,NDEC (qtyBackOrd [3] + qtyForward [3]));
	sprintf (tempStr [4], local_rec.rep_qty,NDEC (qtyBackOrd [4] + qtyForward [4]));
	sprintf (tempStr [5], local_rec.rep_qty,NDEC (qtyBackOrd [5] + qtyForward [5]));
	sprintf (disp_str, " %-23.23s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			mlStkDisp [72],
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
	Dsp_saverec (disp_str);

	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 6; i++)
		{
			sprintf (tempStr [i], local_rec.rep_qty, NDEC (q_project [j] [i]));
		}
		if (j == best_method)
			sprintf (err_str, " ^1%s (%c)^6*", mlStkDisp [56],'A' + j);
		else
			sprintf (err_str, " %s (%c) ", mlStkDisp [56],'A' + j);
		sprintf (disp_str, "%s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			err_str,
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
		Dsp_saverec (disp_str);
	}

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (qtyOrder [0]));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (qtyOrder [1]));
	sprintf (tempStr [2], local_rec.rep_qty, NDEC (qtyOrder [2]));
	sprintf (tempStr [3], local_rec.rep_qty, NDEC (qtyOrder [3]));
	sprintf (tempStr [4], local_rec.rep_qty, NDEC (qtyOrder [4]));
	sprintf (tempStr [5], local_rec.rep_qty, NDEC (qtyOrder [5]));
	sprintf (disp_str, " %-23.23s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			mlStkDisp [51],
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, NDEC (avail [0]));
	sprintf (tempStr [1], local_rec.rep_qty, NDEC (avail [1]));
	sprintf (tempStr [2], local_rec.rep_qty, NDEC (avail [2]));
	sprintf (tempStr [3], local_rec.rep_qty, NDEC (avail [3]));
	sprintf (tempStr [4], local_rec.rep_qty, NDEC (avail [4]));
	sprintf (tempStr [5], local_rec.rep_qty, NDEC (avail [5]));
	sprintf (disp_str, " %-23.23s^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
			mlStkDisp [9],
			tempStr [0], tempStr [1],
			tempStr [2], tempStr [3],
			tempStr [4], tempStr [5]);
	Dsp_saverec (disp_str);

	SillyBusyFunction (0);
	Dsp_srch ();
	Dsp_close ();

	abc_fclose (soln);
	abc_fclose (poln);
	abc_fclose (pohr);
	abc_fclose (pcwo);
	abc_fclose (pcms);
	abc_selfield (ccmr, "ccmr_id_no");

	NormalRedraw ();
	return (EXIT_SUCCESS);
}

float 
GetUom (
 long _hhum_hash)
{
	char	std_group [21],
			alt_group [21];
	number	std_cnv_fct,
			alt_cnv_fct,
			cnv_fct,
			result,
			uom_cfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	_hhum_hash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

int
GetPeriodMonth (
 long poln_date)
{
	int		col_no,
			p_yy,
			p_mm,
			c_yy,
			c_mm;
	int		mdy2 [3];

	c_yy = mdy [2];
	c_mm = mdy [1];

	DateToDMY (poln_date, &mdy2 [0], &mdy2 [1], &mdy2 [2]);
	p_yy = mdy2 [2];
	p_mm = mdy2 [1];

	col_no = (p_yy - c_yy) * 12;
	col_no += p_mm;
	col_no -= c_mm;

	if (col_no < 0)
		return (EXIT_SUCCESS);
	if (col_no > 5)
		return (5);

	return (col_no);
}

char *
InternalMonthName (
 int n)
{
	static char *name [] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
		"***"
	};
	return ((n >= 0 && n <= 11) ? ML (name [n]) : ML (name [12]));
}

/*=========================
| Purchase order history. |
=========================*/
int
PurchaseHistoryDisplay (void)
{
	int		i = 1;
	char	dsp_method [5];
	double	workCost	=	0.00;

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

	lp_x_off = 1;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 10, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	Dsp_saverec ("                       S U P P L I E R    P U R C H A S E   H I S T O R Y                       ");
   	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	suph_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (suph, &suph_rec, LTEQ, "r");
	while (!cc && suph_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (wh_flag && suph_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (suph, &suph_rec,PREVIOUS,"r");
			continue;
		}
		ccmr_rec.hhcc_hash	=	suph_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (suph, &suph_rec,PREVIOUS,"r");
			continue;
		}
		sumr_rec.hhsu_hash	=	suph_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (suph, &suph_rec,PREVIOUS,"r");
			continue;
		}
		inum_rec.hhum_hash	=	suph_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

   		sprintf (disp_str, "  %s    :       %6.6s%12.12s (%-40.40s) ",
					mlStkDisp [95],
					sumr_rec.crd_no,
					" ",
					sumr_rec.crd_name);
		Dsp_saverec (disp_str);

   		sprintf (disp_str, "  %s   :       %2.2s%-16.16s (%-40.40s) ",
					mlStkDisp [103],
					ccmr_rec.cc_no,
					" ",
					ccmr_rec.name);
		Dsp_saverec (disp_str);

   		sprintf (disp_str, "  %s : %15.15s%8.8s ^E    %s : %15.15s  ",
					mlStkDisp [57], suph_rec.po_no,
					" ",
					mlStkDisp [31], suph_rec.grn_no);
		Dsp_saverec (disp_str);

		if (suph_rec.ship_method [0] == 'A')
			strcpy (dsp_method, mlStkDisp [2]);
		else if (suph_rec.ship_method [0] == 'S')
			strcpy (dsp_method, mlStkDisp [67]);
		else if (suph_rec.ship_method [0] == 'L')
			strcpy (dsp_method, mlStkDisp [35]);
		else
			strcpy (dsp_method, "????");

   		sprintf (disp_str, "  %s    :  %12.12s%10.10s ^E    %s      :       %-4.4s       ",
				mlStkDisp [80],
				suph_rec.csm_no,
				" ",
				mlStkDisp [78],
				dsp_method);
		Dsp_saverec (disp_str);

   		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");

		sprintf (err_str, local_rec.rep_qty, suph_rec.ord_qty * CnvFct);
   		sprintf (disp_str, "  %s   :     %10.10s%10.10s^E    %s  : %4.4s  %14s ", 
				mlStkDisp [19],
				DateToString (suph_rec.ord_date),
				" ",
				mlStkDisp [59],
				inum_rec.uom,
				err_str);
		Dsp_saverec (disp_str);

   		sprintf (disp_str, "  %s :     %10.10s%10.10s^E    %s   :              %3.3s ", 
				mlStkDisp [15],
				DateToString (suph_rec.due_date), 
				" ",
				mlStkDisp [25],
				(suph_rec.drop_ship [0] == 'Y') ? mlStkDisp [107] : mlStkDisp [49]);
		Dsp_saverec (disp_str);

		sprintf (err_str, local_rec.rep_qty, suph_rec.rec_qty * CnvFct);
   		sprintf (disp_str, "  %s   :     %10.10s%10.10s^E    %s   : %4.4s  %14s ",
				mlStkDisp [20],
				DateToString (suph_rec.rec_date),
				" ",
				mlStkDisp [60],
				inum_rec.uom,
				err_str);
		Dsp_saverec (disp_str);

		workCost	=	twodec (suph_rec.land_cost);
		if (inmr_rec.outer_size > 1.00)
		{
   			sprintf (disp_str, "  %s     :%15.2f(%4.0f)%4.4s^E    %s        :          %ld %s",
					mlStkDisp [36],
					workCost,
					inmr_rec.outer_size,
					" ",
					mlStkDisp [40],
					suph_rec.due_date - suph_rec.ord_date,
					mlStkDisp [28]);
		}
		else
		{
   			sprintf (disp_str, "  %s     :%15.2f%10.10s^E    %s        :          %ld %s",
					mlStkDisp [36],
					workCost,
					" ",
					mlStkDisp [40],
					suph_rec.due_date - suph_rec.ord_date,
					mlStkDisp [28]);
		}
		Dsp_saverec (disp_str);

		sprintf (disp_str, "                                      ^1 %s %4d ^6", mlStkDisp [37],i++);
		Dsp_saverec (disp_str);

		cc = find_rec (suph, &suph_rec, PREVIOUS, "r");
	}
	abc_fclose (sumr);
	abc_fclose (suph);
	Dsp_srch ();
	Dsp_close ();

	display_ok = TRUE;
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
Heading (
 int scn)
{
	if (!restart)
	{
		if (clear_ok)
			clear ();

		crsr_off ();
		if (wh_flag == FALSE)
			strcpy (err_str,ML (mlSkMess246));
		else
			strcpy (err_str,ML (mlSkMess247));

		rv_pr (err_str, 40, 0, 1);
		print_at (0,100,ML (mlSkMess096), local_rec.prev_item);

		move (0, 20);
		cl_line ();
		move (0, 21);
		cl_line ();
			
		line_at (22,0,132);

		if (wh_flag == FALSE)
		{
			move (0, 23);
			cl_line ();

			print_at (23,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		}
		else
		{
			move (0, 23);
			cl_line ();

			print_at (23,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_short);
			print_at (23,50,ML(mlStdMess039),comm_rec.est_no,comm_rec.est_short);
			print_at (23,100,ML(mlStdMess099),comm_rec.cc_no,comm_rec.cc_short);
		}
		line_cnt = 0;
	}

	if (display_ok)
	{
		AllDisplay ();
		clear_ok = FALSE;
	}
	else
	{
		box (0, 2, 132, 16);
		box (66, 5, 1, 6);
		box (66, 15, 1, 2);

		line_at (4,1,131);
		line_at (14,1,131);
		PrintPopupStuff ();
	}
    return (EXIT_SUCCESS);
}

int
GraphThisLastYear (void)
{
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [12];
	char	gpx_ch_indx [8];
	char	mth_nms [12] [8];
	double	gr_val [2 * 12];
	int		i;
	int		j;
	int		cur_mth;

#ifndef GVISION
	Dsp_open (122, 4, 13);
	Dsp_saverec (mlStkDisp [33]);
	Dsp_saverec ("");
	Dsp_saverec ("");
	Dsp_saverec (" L   T  ");
	Dsp_saverec (" a   h  ");
	Dsp_saverec (" s   i  ");
	Dsp_saverec (" t   s  ");
	Dsp_saverec ("        ");
	Dsp_saverec (" Y   Y  ");
	Dsp_saverec (" e   e  ");
	Dsp_saverec (" a   a  ");
	Dsp_saverec (" r   r  ");
	Dsp_saverec ("        ");
	Dsp_saverec (" ^M^M  ^N^N ");
	Dsp_srch ();
	Dsp_close ();
#else	/* GVISION */
	char *	gr_collabels [2];
	gr_collabels [0] = "Last Year";
	gr_collabels [1] = "This Year";
	GR_SetColumnLabels (2, gr_collabels);
#endif	/* GVISION */

	cur_mth = mdy [1];

	lp_x_off = 0;
	lp_y_off = 4;
	gr_win.x_posn = 0;
	gr_win.y_posn = 4;
	gr_win.x_size = 121;
	gr_win.y_size = 12;

	for (i = 0; i < 12; i++)
	{
		j = (i + cur_mth) % 12;

		gr_val [ (1 * 12) + i] = con [i];
		gr_val [ (0 * 12) + i] = lcon [i];
		sprintf (mth_nms [i], "  %-3.3s", month_nm [j]);

		gr_tits [i] = mth_nms [i];
	}

	gr_nam.pr_head = headingText;
	gr_nam.heading = "S A L E S   H I S T O R Y   D I S P L A Y";
	gr_nam.legends = gr_tits;
	strcpy (gpx_ch_indx, "12");
	gr_nam.gpx_ch_indx = gpx_ch_indx;

	GR_graph (&gr_win, GR_TYPE_DBAR, 12, &gr_nam, gr_val, (struct GR_LIMITS *) 0);

	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
GraphSalesProfits (void)
{
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [12];
	char	gpx_ch_indx [8];
	char	mth_nms [12] [8];
	double	gr_val [2 * 12];
	int		i;
	int		j;
	int		cur_mth;

#ifndef GVISION
	Dsp_open (122, 4, 13);
	Dsp_saverec (mlStkDisp [33]);
	Dsp_saverec ("");
	Dsp_saverec ("");
	Dsp_saverec (" S   P  ");
	Dsp_saverec (" A   R  ");
	Dsp_saverec (" L   O  ");
	Dsp_saverec (" E   F  ");
	Dsp_saverec (" S   I  ");
	Dsp_saverec ("     T  ");
	Dsp_saverec ("     S  ");
	Dsp_saverec ("        ");
	Dsp_saverec (" ^M^M  ^N^N ");
	Dsp_srch ();
	Dsp_close ();
#else	/* GVISION */
	char *	gr_collabels [2];
	gr_collabels [0] = "SALES";
	gr_collabels [1] = "PROFITS";
	GR_SetColumnLabels (2, gr_collabels);
#endif	/* GVISION */

	cur_mth = mdy [1];

	lp_x_off = 0;
	lp_y_off = 4;
	gr_win.x_posn = 0;
	gr_win.y_posn = 4;
	gr_win.x_size = 121;
	gr_win.y_size = 12;

	for (i = 0; i < 12; i++)
	{
		j = (i + cur_mth) % 12;

		gr_val [ (0 * 12) + i] = DOLLARS (val [j]);
		gr_val [ (1 * 12) + i] = DOLLARS (prf [j]);
		sprintf (mth_nms [i], "  %-3.3s", month_nm [j]);

		gr_tits [i] = mth_nms [i];
	}

	gr_nam.pr_head = headingText;
	gr_nam.heading = "S A L E S  /  P R O F I T    D I S P L A Y";
	gr_nam.legends = gr_tits;
	strcpy (gpx_ch_indx, "12");
	gr_nam.gpx_ch_indx = gpx_ch_indx;

	GR_graph (&gr_win, GR_TYPE_DBAR, 12, &gr_nam, gr_val, (struct GR_LIMITS *) 0);

	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
Graph24Months (void)
{
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [24];
	char	gpx_ch_indx [8];
	char	mth_nms [24] [8];
	double	gr_val [24];
	int		i;
	int		j;
	int		cur_mth;

	cur_mth = mdy [1];

	lp_x_off = 0;
	lp_y_off = 4;
	gr_win.x_posn = 0;
	gr_win.y_posn = 4;
	gr_win.x_size = 130;
	gr_win.y_size = 12;

	for (i = 0; i < 12; i++)
	{
		j = (i + cur_mth) % 12;

		gr_val [i + 12] =  con [i];
		gr_val [i]    =  lcon [i];
		sprintf (mth_nms [i + 12], " %-3.3s ", month_nm [j]);
		sprintf (mth_nms [i],     " %-3.3s ", month_nm [j]);
		gr_tits [i]      = mth_nms [i];
		gr_tits [i + 12] = mth_nms [i + 12];
	}

	gr_nam.pr_head = headingText;
	gr_nam.heading = mlStkDisp [1];
	gr_nam.legends = gr_tits;
	strcpy (gpx_ch_indx, "12");
	gr_nam.gpx_ch_indx = gpx_ch_indx;

	GR_graph (&gr_win, GR_TYPE_BAR, 24, &gr_nam, gr_val, (struct GR_LIMITS *) NULL);

	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
SelectPopupA (void)
{
	PrintPopup (IN_POP_A);
	return (EXIT_SUCCESS);
}
int
SelectPopupB (void)
{
	PrintPopup (IN_POP_B);
	return (EXIT_SUCCESS);
}
int
SelectPopupC (void)
{
	PrintPopup (IN_POP_C);
	return (EXIT_SUCCESS);
}
int
SelectPopupD (void)
{
	PrintPopup (IN_POP_D);
	return (EXIT_SUCCESS);
}
int
SelectPopupE (void)
{
	PrintPopup (IN_POP_E);
	return (EXIT_SUCCESS);
}
int
SelectPopupF (void)
{
	PrintPopup (IN_POP_F);
	return (EXIT_SUCCESS);
}

#ifdef GVISION
int
SelectPopupG (void)
{
	PrintPopup (IN_POP_G);
	return (EXIT_SUCCESS);
}
#endif


void
PrintPopup (
 int	type)
{
	char	wk_cst_used [22],
			wk_cst_avge [22],
			wk_cst_last [22],
			wk_cst_std [22],
			wk_cst_prev [22];

	char	gr_str [13];
	char	gr_out [21];

	char	ff_desc [13];

	int		foundOne	=	FALSE,
			i;

	/*--------------------------------------------------------
	| Fill with graphics character and fill blockout string. |
	--------------------------------------------------------*/
	memset (gr_str, ta [12] [13], 12);
	sprintf (gr_out, "%s%-12.12s%s", ta [16], gr_str, ta [17]);

	if (displayCostOk)
	{
		sprintf (wk_cst_used, "%12.12s",
				comma_fmt (local_rec.l_cost, "N,NNN,NNN.NN"));

		sprintf (wk_cst_avge, "%12.12s",
				comma_fmt (local_rec.avge_cost,"N,NNN,NNN.NN"));

		sprintf (wk_cst_last, "%12.12s",
				comma_fmt (local_rec.last_cost,"N,NNN,NNN.NN"));

		sprintf (wk_cst_prev, "%12.12s",
				comma_fmt (local_rec.prev_cost,"N,NNN,NNN.NN"));

		if (wh_flag)
			sprintf (wk_cst_std, "%12.12s",
				comma_fmt (local_rec.std_cost,"N,NNN,NNN.NN"));
		else
			sprintf (wk_cst_std, "%12.12s","N/A");
	}
	else
	{
		strcpy (wk_cst_used, gr_out);
		strcpy (wk_cst_avge, gr_out);
		strcpy (wk_cst_last, gr_out);
		strcpy (wk_cst_prev, gr_out);
		strcpy (wk_cst_std,  gr_out);
	}

	crsr_off ();

	if (type)
		popupSelect = type;

#ifdef GVISION
	clearPicture();
#endif

	switch (popupSelect)
	{
	/*-----------------
	| Popup Window S1 |
	-----------------*/
    case IN_POP_A :
		print_at (POP_Y + 1,
				POP_X,
				ML (mlSkMess261),
				inmr_rec.inmr_class,
				inmr_rec.category,
				" ");

		if (wh_flag)
		{
			print_at (POP_Y + 2,
					POP_X,
					ML (mlSkMess262),
					incc_rec.abc_code,
					(incc_rec.abc_update [0] == 'Y') ? mlStkDisp [107] : mlStkDisp [47],
					" ");
		}
		else
		{
			print_at (POP_Y + 2,
					POP_X,
					ML (mlSkMess262),
					inmr_rec.abc_code,
					"   ",
					" ");
		}
		print_at (POP_Y + 3,
				POP_X,
			    ML (mlSkMess263),
				local_rec.cost_type,
				wk_cst_used,
				" ");

		print_at (POP_Y + 4,
				POP_X,
				ML (mlSkMess264),
				wk_cst_last,
				" ");

		print_at (POP_Y + 5,
				POP_X,
				ML (mlSkMess265),
				comma_fmt (local_rec.mtd_sales, "N,NNN,NNN.NN"),
				" ");

		print_at (POP_Y + 6,
				POP_X,
				ML (mlSkMess266),
				comma_fmt (local_rec.ytd_sales, "N,NNN,NNN.NN"),
				" ");

		print_at (POP_Y + 7,
					POP_X,
					ML (mlSkMess720),
					ineiRec.min_stock);
		print_at (POP_Y + 8,
					POP_X,
					ML (mlSkMess721),
					ineiRec.max_stock);

		popupSelect = IN_POP_A;

		break;

	/*-----------------
	| Popup Window S2 |
	-----------------*/
	case IN_POP_B	:
		print_at (POP_Y + 1,
				POP_X,
				ML (mlSkMess263),
				local_rec.cost_type,
				wk_cst_used,
				" ");

		print_at (POP_Y + 2,
				POP_X,
				ML (mlSkMess264),
				wk_cst_last,
				" ");

		print_at (POP_Y + 3,
				POP_X,
	            ML (mlSkMess271),	
				wk_cst_avge,
				" ");

		print_at (POP_Y + 4,
				POP_X,
				ML (mlSkMess272),
				wk_cst_prev,
				" ");

		print_at (POP_Y + 5,
				POP_X,
				ML (mlSkMess273),
				wk_cst_std,
				" ");

		print_at (POP_Y + 6,
				POP_X,
				ML (mlSkMess274),
				inmr_rec.sale_unit,
				inmr_rec.pack_size,
				" ");

		print_at (POP_Y + 7,
				POP_X,
				ML (mlSkMess275),
				inmr_rec.uom_cfactor,
				" ");

		print_at (POP_Y + 8,
				POP_X,
				ML (mlSkMess596),
				inmr_rec.outer_size,
				" ");

		popupSelect = IN_POP_B;

		break;

	/*-----------------
	| Popup Window S3 |
	-----------------*/
	case IN_POP_C	:
		sprintf (ff_desc, "%-12.12s", mlStkDisp [8]);
		if (incc_rec.ff_option [0] == 'M')
			sprintf (ff_desc, "%-12.12s", mlStkDisp [42]);
		if (incc_rec.ff_option [0] == 'P')
			sprintf (ff_desc, "%-12.12s", mlStkDisp [55]);
		if (wh_flag)
		{
			print_at (POP_Y + 1,
					POP_X,
					ML (mlSkMess276),
					ff_desc,
					" ");

			print_at (POP_Y + 2,
					POP_X,
					ML (mlSkMess277),
					incc_rec.ff_method,
					" ");

			print_at (POP_Y + 3,
					POP_X,
					ML (mlSkMess278),
					incc_rec.safety_stock,
				" ");

			print_at (POP_Y + 4,
					POP_X,
					ML (mlSkMess279),
					incc_rec.wks_demand,
					" ");
		}
		else
		{
			print_at (POP_Y + 1, POP_X, "%56.56s", " ");
			print_at (POP_Y + 2, POP_X, "%56.56s", " ");
			print_at (POP_Y + 3, POP_X, "%56.56s", " ");
			print_at (POP_Y + 4, POP_X, "%56.56s", " ");
		}
		print_at (POP_Y + 5,
				POP_X,
				ML (mlSkMess265),
				comma_fmt (local_rec.mtd_sales, "N,NNN,NNN.NN"),
				" ");

		print_at (POP_Y + 6,
				POP_X,
				ML (mlSkMess266),
				comma_fmt (local_rec.ytd_sales, "N,NNN,NNN.NN"),
				" ");

		print_at (POP_Y + 7,
				POP_X,
				ML (mlSkMess280),
				comma_fmt (local_rec.one_ysales, "N,NNN,NNN.NN"),
				" ");

		print_at (POP_Y + 8,
				POP_X,
				ML (mlSkMess281),
				comma_fmt (local_rec.two_ysales, "N,NNN,NNN.NN"),
				" ");

		popupSelect = IN_POP_C;

		break;


	/*-----------------
	| Popup Window S4 |
	-----------------*/
	case IN_POP_D	:
		print_at (POP_Y + 1,
				POP_X,
				ML (mlSkMess282),
				inmr_rec.quick_code,
				" ");

		print_at (POP_Y + 2,
				POP_X,
				ML (mlSkMess283),
			     inmr_rec.ex_code,
				 " ");

		print_at (POP_Y + 3,
				POP_X,
				ML (mlSkMess284),
				incc_rec.location,
				" ");

		if (inmr_rec.bo_flag [0] == 'Y')
		{
			print_at (POP_Y + 4,
					POP_X,
					ML (mlSkMess285),
					" ");
		}
		if (inmr_rec.bo_flag [0] == 'N')
		{
			print_at (POP_Y + 4,
					POP_X,
					ML (mlSkMess612),
					" ");
		}
		if (inmr_rec.bo_flag [0] == 'F')
		{
			print_at (POP_Y + 4,
					POP_X,
					ML (mlSkMess286),
					" ");
		}

		if (inmr_rec.bo_release [0] == 'A') 
			print_at (POP_Y + 5,
				POP_X,
				ML (mlSkMess287),
			    " ");
		else
			print_at (POP_Y + 5,
				POP_X,
				ML (mlSkMess613),
			    " ");
				

		print_at (POP_Y + 6,
				POP_X,
				ML (mlSkMess288),
				inmr_rec.disc_pc,
				" ");

		print_at (POP_Y + 7,
					POP_X,
					"%-3.3s %%                       :             %6.2f%8.8s",
					envVarGstTaxName,
					(GST) ? inmr_rec.gst_pc : inmr_rec.tax_pc,
					" ");

		print_at (POP_Y + 8,
					POP_X,
					ML (mlSkMess291),
					comma_fmt (DOLLARS (inmr_rec.tax_amount), "NN,NNN.NN"),
					" ");

		popupSelect = IN_POP_D;

		break;

	/*-----------------
	| Popup Window S5 |
	-----------------*/
	case IN_POP_E	:

		for (i = 0; i < 8; ++i)
			print_at (POP_Y + i + 1, POP_X, "%50.50s", " ");

		open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_id_no");

		sprintf (incp_rec.key, "%2.2s    ", comm_rec.co_no);
		strcpy (incp_rec.curr_code, envVarCurrCode);
		strcpy (incp_rec.status, "A");
		incp_rec.hhcu_hash = 0L;
		strcpy (incp_rec.area_code, "  ");
		strcpy (incp_rec.cus_type, "   ");
		incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incp_rec.date_from = 0L;
		cc = find_rec (incp, &incp_rec, GTEQ, "r");
		while (!cc &&
				incp_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (incp_rec.date_from <= TodaysDate () &&
				incp_rec.date_to   >= TodaysDate ())
			{
				for (i = 0; i < 8; i++)
				{
					if (incp_price [i] == 0.00)
					{
						WsFindInpr (i);
						print_at 
						(
							POP_Y + i + 1, 
							POP_X, 
							"%-16.16s : %10.2f",
							local_rec.priceDesc [i],
							DOLLARS(inpr2_rec.base)
						);
					}
					else
					{
						print_at 
						(
							POP_Y + i + 1, 
							POP_X, 
							"%-16.16s : %10.2f (%s)",
							local_rec.priceDesc [i],
							DOLLARS (incp_price [i]),
							DateToString (incp_rec.date_from)
						);
					}
				}
				foundOne = TRUE;
				break;
			}
			else
				cc = find_rec (incp, &incp_rec, NEXT, "r");
		}

		/*----------------------------------------
		|  Display standard prices if not found  |
		----------------------------------------*/
		if (!foundOne)
		{
			for (i = 0; i < 8; i++)
			{
				WsFindInpr (i);
				if (i < envVarSkDbPriNum)
				{
					print_at (POP_Y + i + 1, POP_X, "%-16.16s            :  %17.2f",
					local_rec.priceDesc [i],
					DOLLARS(inpr2_rec.base));
				}
			}
		}
		popupSelect = IN_POP_E;
		abc_fclose (incp);
		
		break;

	/*-----------------
	| Popup Window S6 |
	-----------------*/
	case IN_POP_F	:
		
		Transport (inmr_rec.hhbr_hash);

		print_at (POP_Y + 1,
				POP_X,
				ML (mlSkMess261),
				inmr_rec.inmr_class,
				inmr_rec.category,
				" ");

		if (wh_flag)
		{
			print_at (POP_Y + 2,
					POP_X,
					ML (mlSkMess262),
					incc_rec.abc_code,
					(incc_rec.abc_update [0] == 'Y') ? mlStkDisp [107] : mlStkDisp [47],
					" ");
		}
		else
		{
			print_at (POP_Y + 2,
					POP_X,
					ML (mlSkMess262),
					inmr_rec.abc_code,
					"   ",
					" ");
		}
        if (wh_flag)
        {
		    print_at (POP_Y + 3, POP_X,
		    	"%-22.22s      :         %10.10s %8.8s", 
						mlStkDisp [119], DateToString (incc_rec.freeze_date), " ");

		    print_at (POP_Y + 4, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [120], CalcOrders (inmr_rec.hhbr_hash)," ");

		    print_at (POP_Y + 5, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [121], transportDespatch, " ");

		    print_at (POP_Y + 6, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [122], transportReturns, " ");

        }
        else
        {
			print_at (POP_Y + 3, POP_X, "%56.56s", " ");

		    print_at (POP_Y + 4, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [120], CalcOrders (inmr_rec.hhbr_hash)," ");

		    print_at (POP_Y + 5, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [121], transportDespatch, " ");

		    print_at (POP_Y + 6, POP_X,
		    	"%-22.22s      :            %7.2f %10.10s", 
						mlStkDisp [122], transportReturns, " ");

        }
		sprintf (tempStr [0], local_rec.rep_qty, qcQty);

		if (inmr_rec.qc_reqd [0] == 'Y')
			print_at (POP_Y + 7,
					POP_X,
					ML (mlSkMess267),
					tempStr [0],
					" ");
		else
			print_at (POP_Y + 7,
					POP_X,
					ML (mlSkMess268),
					tempStr [0],
					" ");
			
		sprintf (tempStr [0], local_rec.rep_qty, woQtyAnti);
		print_at (POP_Y + 8,
				POP_X,
				ML (mlSkMess269),
				tempStr [0],
				" ");

		popupSelect = IN_POP_F;
		break;
#ifdef GVISION
	/*-----------------
	| Popup Window S7 |
	-----------------*/
	case IN_POP_G	:

		for (i = 0; i < 8; ++i)
			print_at (POP_Y + i + 1, POP_X, "%50.50s", " ");

		char picture_name [25];
		strcpy (picture_name, inmr_rec.item_no);
		clip (picture_name);
		strcat(picture_name, ".jpg");
		pictureAt (POP_X+13, POP_Y+1, 195, 142, picture_name);
		popupSelect = IN_POP_G;
		break;
#endif
	}
	print_at (4, 97, "<S%d>", popupSelect) ;
}

void
Transport (
	long	hhbrHash)
{
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhcl_hash");
	open_rec (trcg, trcg_list, TRCG_NO_FIELDS, "trcg_hhbr_hash");

	trcg_rec.hhbr_hash = hhbrHash;
	cc = find_rec (trcg, &trcg_rec, GTEQ, "r");
	while (!cc && trcg_rec.hhbr_hash == hhbrHash)
	{
		coln_rec.hhcl_hash	=	trcg_rec.hhcl_hash;
		cc = find_rec (coln, &coln_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (trcg, &trcg_rec, NEXT, "r");
			continue;
		}
		if (wh_flag == TRUE && coln_rec.incc_hash != currentHhccHash)
		{
			cc = find_rec (trcg, &trcg_rec, NEXT, "r");
			continue;
		}
		if (trcg_rec.load_type [0] == 'D')
			transportDespatch	+=	coln_rec.qty_del;

		if (trcg_rec.load_type [0] == 'R')
			transportReturns	+=	coln_rec.qty_ret;

		cc = find_rec (trcg, &trcg_rec, NEXT, "r");
	}
}

/*============================================================
| Process Customer Orders and update B (ackorders) or Others. |
============================================================*/
float
CalcOrders (
	long	hhbrHash)
{
	float	salesOrderAllocation	=	0.00;
	float	qty = 0.00;

	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");

	/*------------------------------
	| Process all customer  lines. |
	------------------------------*/
	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && hhbrHash == soln_rec.hhbr_hash) 
	{
		qty =  soln_rec.qty_order;
		if ((wh_flag == TRUE) && (soln_rec.hhcc_hash != currentHhccHash || qty <= 0.00))
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		if (soln_rec.status [0] == 'B' || soln_rec.status [0] == 'F')
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		if (soln_rec.status [0] == 'P')
		{
			if (CheckColn (soln_rec.hhsl_hash))
				salesOrderAllocation	+=	soln_rec.qty_order;
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (salesOrderAllocation);
}

/*=============================================================================
| Routine ensures packing slips already posted are not included in committed. |
=============================================================================*/
int
CheckColn (
 long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	if (coln_rec.status [0] != 'P' && coln_rec.stat_flag [0] != 'P')
		return (TRUE);

	return (FALSE);
}

/*====================================
| Reorder review (Focus Forecasting) |
====================================*/
int
ReorderReview (void)
{
	double	wks_dmd,
			min_ord_qty,
			nrm_ord_qty,
			ord_mlt,
			qty_rec,
			stk_avl_qty,
			stk_ord_qty,
			tot_cvr_qty,
			cvr_rqd_qty,
			net_req_qty;

	float	rvw_per,
			saf_stk,
			lead_tmw,
			lead_tmd,
			cvr_wks,
			stk_avl_wks,
			stk_ord_wks,
			tot_cvr_wks,
			cvr_rqd_wks,
			net_req_wks,
			GetReviewPeriod (long hhbr_hash, char *br_no),
			GetLeadDate (long hash, long date);

	int		cur_mth;

	cur_mth = mdy [1];

	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (ffpr, ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");

	inis_rec.lead_time 		= 0.00;
	inis_rec.min_order 		= 0.00;
	inis_rec.norm_order 	= 0.00;
	inis_rec.ord_multiple 	= 0.00;
	rvw_per = envVarLrpDfltReview;

	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "C1");
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "  ");
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, GTEQ, "r");
		while (!cc && inis_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			rvw_per = GetReviewPeriod (inmr_rec.hhbr_hash, comm_rec.est_no);
			if (inis_rec.lead_time == (float) 0.00)
				inis_rec.lead_time = GetLeadDate (inis_rec.hhis_hash, comm_rec.inv_date);
			else
				break;
			cc = find_rec (inis, &inis_rec, NEXT, "r");
		}
	}
	else
	{
		rvw_per = GetReviewPeriod (inmr_rec.hhbr_hash, comm_rec.est_no);
		if (inis_rec.lead_time == (float) 0.00)
			inis_rec.lead_time = GetLeadDate (inis_rec.hhis_hash, comm_rec.inv_date);
	}
	abc_fclose (inis);

	wks_dmd = weeksDemand;
	min_ord_qty = NDEC (inis_rec.min_order);
	nrm_ord_qty = NDEC (inis_rec.norm_order);
	ord_mlt = inis_rec.ord_multiple;

	lead_tmd = inis_rec.lead_time;
	lead_tmw = inis_rec.lead_time / 7;

	saf_stk = safetyStock;

	useMinStockWks = FALSE;
	if (wh_flag)
	{
		minStockWks = 0.0;
		cvr_wks = rvw_per + lead_tmw + saf_stk;
	}
	else
	{
		minStockWks = 0.0;
		if (weeksDemand != 0.0)
			minStockWks = inmr_rec.min_quan / (float)weeksDemand;
		if (minStockWks > saf_stk)
		{
			useMinStockWks = TRUE;
			cvr_wks = rvw_per + lead_tmw + minStockWks;
		}
		else
			cvr_wks = rvw_per + lead_tmw + saf_stk;
	}

	stk_avl_qty = (local_rec.available > 0.00) ? local_rec.available : 0.00;
	stk_avl_wks = (wks_dmd == 0.00) ? 0.00 : stk_avl_qty / wks_dmd;
	stk_ord_qty = NDEC (onOrder);
	stk_ord_wks = (wks_dmd == 0.00) ? 0.00 : NDEC (onOrder) / wks_dmd;
	tot_cvr_qty = NDEC (stk_avl_qty) + NDEC (stk_ord_qty);
	tot_cvr_wks = stk_avl_wks + stk_ord_wks;
	cvr_rqd_wks = rvw_per + lead_tmw + saf_stk;
	cvr_rqd_qty = cvr_rqd_wks * wks_dmd;
	net_req_qty = NDEC (cvr_rqd_qty) - NDEC (tot_cvr_qty);
	net_req_wks = cvr_rqd_wks - tot_cvr_wks;
	if (net_req_qty <= 0.00)
	{
		net_req_qty = 0.00;
		net_req_wks = 0.00;
	}

	if (net_req_qty > min_ord_qty)
		if (net_req_qty > nrm_ord_qty)
			qty_rec = net_req_qty;
		else
			qty_rec = nrm_ord_qty;
	else
		if (nrm_ord_qty > min_ord_qty)
			qty_rec = nrm_ord_qty;
		else
			qty_rec = min_ord_qty;
	if (net_req_qty <= 0.00)
		qty_rec = 0.00;

	qty_rec = NDEC (qty_rec);
	if (qty_rec < 0.00)
		qty_rec = 0.00;

	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open (0, 2, 15, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);

	sprintf (err_str, "%30.30s%s%30.30s"," ",
				(wh_flag) ? mlStkDisp [63] : mlStkDisp [62], " ");
	Dsp_saverec (err_str);
	sprintf (err_str, ".      %s :   %16.16s   (%40.40s)    ",
			mlStkDisp [34],
			inmr_rec.item_no, inmr_rec.description);
	Dsp_saverec (err_str);
	Dsp_saverec (" [PRINT] [EDIT / END] ");

	sprintf (disp_str, " %s      :        %12.2f   ^E   %s :        %5.2f %s",
		mlStkDisp [105], wks_dmd,
		mlStkDisp [64], rvw_per, mlStkDisp [106]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, min_ord_qty);
	sprintf (disp_str, " %s     :      %14s   ^E   %s : +      %5.2f %s ",
		mlStkDisp [44], tempStr [0],
		mlStkDisp [40], lead_tmd, mlStkDisp [28]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, nrm_ord_qty);
	if (useMinStockWks)
	{
		sprintf (disp_str, 
				 " %s  :      %14s   ^E   %s    : +      %5.2f %s",
				 mlStkDisp [50], tempStr [0],
				 mlStkDisp [117], minStockWks,
				 mlStkDisp [106]);
	}
	else
	{
		sprintf (disp_str, 
				 " %s  :      %14s   ^E   %s  : +      %5.2f %s",
				 mlStkDisp [50], tempStr [0],
				 mlStkDisp [71], saf_stk,
				 mlStkDisp [106]);
	}

	Dsp_saverec (disp_str);
	sprintf (disp_str, " %s    :        %12.2f   ^E   ==================================",
		mlStkDisp [52],
		ord_mlt);
	Dsp_saverec (disp_str);
	sprintf (tempStr [0], local_rec.rep_qty, qty_rec);
	sprintf (disp_str, " %s.  :      %14s   ^E   %s :        %5.2f %s",
		mlStkDisp [58], tempStr [0],
		mlStkDisp [17], cvr_wks, mlStkDisp [106]);
	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");

	sprintf (err_str, "                         %s       %s              %s", mlStkDisp [106], mlStkDisp [61], mlStkDisp [38]);
	Dsp_saverec (err_str);
	Dsp_saverec ("                         ------   --------------        ------------------------ ");
	sprintf (tempStr [0], local_rec.rep_qty, stk_avl_qty);
	sprintf (tempStr [1], local_rec.rep_qty, con [10]);
	sprintf  (disp_str, " %s   :     %6.2f   %14s        %-3.3s   -   %14s ",
		mlStkDisp [85],
		stk_avl_wks,
		tempStr [0],
		month_nm [ (10 + cur_mth) % 12],
		tempStr [1]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, stk_ord_qty);
	sprintf (tempStr [1], local_rec.rep_qty, con [9]);
	sprintf  (disp_str, " %s    :     %6.2f   %14s        %-3.3s   -   %14s ",
		mlStkDisp [89],
		stk_ord_wks,
		tempStr [0],
		month_nm [ (9 + cur_mth) % 12],
		tempStr [1]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, con [8]);
	sprintf  (disp_str, "                        ------------------------        %-3.3s   -   %14s ",
		month_nm [ (8 + cur_mth) % 12],
		tempStr [0]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, tot_cvr_qty);
	sprintf (tempStr [1], local_rec.rep_qty, con [7]);
	sprintf  (disp_str, " %s       :     %6.2f   %14s        %-3.3s   -   %14s ",
		mlStkDisp [98],
		tot_cvr_wks,
		tempStr [0],
		month_nm [ (7 + cur_mth) % 12],
		tempStr [1]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, con [6]);
	sprintf  (disp_str, "                        ------------------------        %-3.3s   -   %14s ",
		month_nm [ (6 + cur_mth) % 12],
		tempStr [0]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, cvr_rqd_qty);
	sprintf (tempStr [1], local_rec.rep_qty, con [5]);
	sprintf  (disp_str, " %s    :     %6.2f   %14s        %-3.3s   -   %14s ",
		mlStkDisp [18],
		cvr_rqd_wks,
		tempStr [0],
		month_nm [ (5 + cur_mth) % 12],
		tempStr [1]);
	Dsp_saverec (disp_str);

	sprintf (tempStr [0], local_rec.rep_qty, net_req_qty);
	sprintf  (disp_str, " %s   :     %6.2f   %14s",
		mlStkDisp [48],
		net_req_wks,
		tempStr [0]);
	Dsp_saverec (disp_str);

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (ffpr);

	NormalRedraw ();
    return (EXIT_SUCCESS);
}

void
SillyBusyFunction (
 int flip)
{
	move (0, 1);
	cl_line ();
	if (flip)
		rv_pr (ML (mlStdMess035), 0, 1, 1);

	fflush (stdout);
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.		        	|
| Return 0 if none found.			            |
===============================================*/
float	
GetLeadDate (
 long hash, 
 long date)
{
	float	days;

	inld_rec.hhis_hash = hash;
	inld_rec.ord_date = date;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	if (cc || inld_rec.hhis_hash != hash)
		return ((float) 0.00);

	days = (inld_rec.sup_date - date);
	return (days);
}

/*---------------------------------------
| find F/F review period record.	|
---------------------------------------*/
float	
GetReviewPeriod (
 long hhbr_hash, 
 char *br_no)
{
	/*---------------------------------------
	| Find out what the review-period	|
	| is for this product. Firstly, try for	|
	| a match on branch/item. Then try for	|
	| a match on item. Then try for a match	|
	| on branch/category. If this fails,	|
	| then use LRP_DFLT_REVIEW environment-	|
	| value. If not found, dflt to 4 weeks.	|
	---------------------------------------*/
	ffpr_rec.hhbr_hash = hhbr_hash;
	strcpy (ffpr_rec.br_no, br_no);
	cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	if (cc)
	{
	    strcpy (ffpr_rec.br_no, "  ");
	    cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	    if (cc)
	    {
			abc_selfield (ffpr, "ffpr_id_no_1");
			strcpy (ffpr_rec.category, inmr_rec.category);
			strcpy (ffpr_rec.br_no, br_no);
			cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
			if (cc)
			{
				strcpy (ffpr_rec.br_no, "  ");
				cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
				if (cc)
				ffpr_rec.review_prd = envVarLrpDfltReview;
			}
			abc_selfield (ffpr, "ffpr_id_no");
	    }
	}

	return (ffpr_rec.review_prd);
}

int
AddRecalculate (void)
{
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, "RC");
	sobg_rec.lpno = 0;
	sobg_rec.hash = inmr_rec.hhbr_hash;
	sobg_rec.hash2 = 0L;
	if (wh_flag)
		sobg_rec.hash2 = currentHhccHash;
	sobg_rec.value = (double) 0.00;

	cc = abc_add (sobg, &sobg_rec);

	abc_fclose (sobg);
	return (EXIT_SUCCESS);
}

int
BarCodeDisplay (void)
{
	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 10, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec (" BAR CODE NUMBER  | UOM. ");


	Dsp_saverec ("");
	Dsp_saverec ("[NEXT] [PREV] [EDIT/END]");

	abc_alias ("inbm2", "inbm");
	open_rec ("inbm2", inbm_list, INBM_NO_FIELDS, "inbm_id_no2");
	strcpy (sk_inbm_rec.co_no, comm_rec.co_no);
	strcpy (sk_inbm_rec.item_no, inmr_rec.item_no);
	strcpy (sk_inbm_rec.barcode, " ");
	cc = find_rec ("inbm2", &sk_inbm_rec, GTEQ, "r");
	while (!cc && !strcmp (sk_inbm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (sk_inbm_rec.item_no, inmr_rec.item_no))
	{
		sprintf (disp_str, " %-16.16s ^E %-4.4s ", 
						sk_inbm_rec.barcode,sk_inbm_rec.uom);

		Dsp_saverec (disp_str);
		cc = find_rec ("inbm2", &sk_inbm_rec, NEXT, "r");
	}
	abc_fclose ("inbm2");
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

int
InexDisplay (void)
{
	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 4, 12, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("          INVENTORY  ITEM  DISPLAY          ");


	Dsp_saverec ("");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 1;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf (disp_str, "  %-40.40s  ", inex_rec.desc);

		Dsp_saverec (disp_str);
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
    return (EXIT_SUCCESS);
}

int
InexMaint (void)
{
	int		i,
			last_line	=	0;

	txt_close (tx_window, FALSE);

	tx_window = txt_open (5, 0, 13, 40, 300,
			"       INVENTORY ITEM  MAINTENANCE.       ");

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 1;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf (disp_str, "%-40.40s", inex_rec.desc);

		txt_pval (tx_window, disp_str, 0);

		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}

	tx_lines = txt_edit (tx_window);
	if (!tx_lines)
	{
		txt_close (tx_window, FALSE);
		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = 1;
		cc = find_rec (inex, &inex_rec, GTEQ, "r");
		while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			abc_delete (inex);
			cc = find_rec (inex, &inex_rec, GTEQ, "r");
		}
		NormalRedraw ();
		return (EXIT_SUCCESS);
	}
	for (i = 1; i <= tx_lines; i++)
	{
		last_line = i;

		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = i;
		cc = find_rec (inex, &inex_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock ("inex");
			sprintf (inex_rec.desc, "%-40.40s", txt_gval (tx_window, i));

			cc = abc_add (inex, &inex_rec);
			if (cc)
				file_err (cc, "inex", "DBADD");
		}
		else
		{
			sprintf (inex_rec.desc, "%-40.40s", txt_gval (tx_window, i));

			cc = abc_update (inex, &inex_rec);
			if (cc)
				file_err (cc, "inex", "DBUPDATE");
		}
		abc_unlock ("inex");
	}
	txt_close (tx_window, FALSE);

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no = last_line + 1;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		abc_delete (inex);
		cc = find_rec (inex, &inex_rec, GTEQ, "r");
	}
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

void
DspStockTag (void)
{
	int		break_out;
	int		valid1	=	0,
			valid2	=	0;
	char	tem_line [200];

	saveIndex = 0;
	Dsp_open (0, 1, 16);
	sprintf (err_str, "    ITEM NO.    |    I T E M   D E S C R I P T I O N     | LOCATION | ACTIVE | QUANTITY  | QUANTITY |  GIT  |  %16.16s   ",
							local_rec.priceDesc [0]);
	Dsp_saverec (err_str);
	Dsp_saverec ("                |                                        |          | STATUS | AVAILABLE | ON ORDER |  QTY  |                     ");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END] ");  

	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no_2");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");

	if (!strcmp (local_rec.s_cat, "           ") &&
	    !strcmp (local_rec.e_cat, "~~~~~~~~~~~"))
		abc_selfield (inmr, "inmr_id_no");
	else
		abc_selfield (inmr, "inmr_id_no_3");

	if (!strcmp (local_rec.e_cat, "~~~~~~~~~~~"))
		memset ((char *)local_rec.e_cat,0xff,sizeof (local_rec.e_cat));

	if (!strcmp (local_rec.e_item, "~~~~~~~~~~~~~~~~"))
		memset ((char *)local_rec.e_item,0xff,sizeof (local_rec.e_item));

	memset (&inmr_rec, 0, sizeof (inmr_rec));
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.inmr_class,local_rec.s_class);
	strcpy (inmr_rec.category,local_rec.s_cat);
	strcpy (inmr_rec.item_no,local_rec.s_item);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
		       strcmp (inmr_rec.inmr_class,local_rec.s_class) >= 0 && 
		       strcmp (inmr_rec.inmr_class,local_rec.e_class) <= 0 &&
		       strcmp (inmr_rec.category,local_rec.s_cat) >= 0 && 
		       strcmp (inmr_rec.category,local_rec.e_cat) <= 0 &&
		       strcmp (inmr_rec.item_no,local_rec.s_item) >= 0 && 
		       strcmp (inmr_rec.item_no,local_rec.e_item) <= 0)
	{
		if (local_rec.s_desc1 [0] != ' ')
		{
			valid1 = check_search (	inmr_rec.description, 
							  		clip (local_rec.s_desc1), 
							  		&break_out);
		}
		else	
			valid1	=	TRUE;

		if (local_rec.s_desc2 [0] != ' ')
		{

			valid2 = check_search (	inmr_rec.description, 
							  		clip (local_rec.s_desc2), 
							  		&break_out);
		}
		if (valid1 || valid2)
		{
			hhsiHash	=	alt_hash
					 		(
								inmr_rec.hhbr_hash,
								inmr_rec.hhsi_hash
							);
			qtyAvail = 0.00;
			incc_rec.hhcc_hash = currentHhccHash;
			incc_rec.hhbr_hash = hhsiHash;
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			if (!cc)
			{
				/*----------------------------
				| Calculate available stock. |
				----------------------------*/
				if (envVarSoFwdAvl)
				{
					qtyAvail = incc_rec.closing_stock -
								incc_rec.committed - 
								incc_rec.backorder - 
								incc_rec.forward;
				}
				else
				{
					qtyAvail = incc_rec.closing_stock -
								incc_rec.committed - 
								incc_rec.backorder;
				}
				if (envVarQcApply && envVarSkQcAvl)
					qtyAvail -= incc_rec.qc_qty;
			}

			WsFindInpr (0);
			sprintf (tem_line,"%s^E%s^E %s^E   %s    ^E%10.2f ^E%9.2f ^E%6.0f ^E %18.18s",
				inmr_rec.item_no,
				inmr_rec.description,
				ccmr_rec.acronym,
				inmr_rec.active_status,
				qtyAvail,
				incc_rec.on_order,
				GetShipQty (),
				comma_fmt (DOLLARS (inpr2_rec.base),"NNN,NNN,NNN,NNN.NN"));
		
			sprintf (saveKey,"%04d%010ld", saveIndex++,inmr_rec.hhbr_hash);	

			Dsp_save_fn (tem_line, saveKey); 
		}
		cc = find_rec (inmr ,&inmr_rec,NEXT,"r");
	}
	abc_fclose (poln);
	abc_fclose (posl);
	abc_selfield (inmr, "inmr_id_no");

	Dsp_srch_fn (StockEnquiry); 
	Dsp_close ();
	return;
}

/*================
| Print heading. |
================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML (mlSkMess292),48,0,1);
		line_at (1,0, 132);

		box (0, 2, 132, 10);
		line_at (7,1,131);
		line_at (10,1,131);
		line_at (21,0,132);
		if (wh_flag == FALSE)
			print_at (22,0,ML (mlStdMess038),
					comm_rec.co_no, comm_rec.co_name);
		else
		{			
			print_at (22,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_short);
			print_at (22,50,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
			print_at (22,100,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_short);
		}
		line_cnt = 0;
		WildCardMess ();
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
StockEnquiry (
 char	*findKey)
{
	hhbrPassed = atol (findKey + 4);
	hashPassed = TRUE;
	GetItemNo ();

    if (prog_exit) 
        return (EXIT_SUCCESS);

	display_ok = TRUE;
	clear_ok = FALSE;
	Heading (1);
#ifndef GVISION
	run_menu (_main_menu, "", input_row);
#else
	run_menu (_main_group, _main_menu);
	clearPicture();	
#endif
	if (mainWindowOpen)
		Dsp_close ();

	mainWindowOpen = FALSE;
	
	strcpy (local_rec.prev_item, inmr_rec.item_no);
	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char	*key_val)
{
	work_open ();
	save_rec ("#Cat Code","#Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf ,&excf_rec,GTEQ,"r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
			!strcmp (excf_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf ,&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc,  "excf" , "DBFIND");
}

float	
GetShipQty (void)
{
	float	ShipQty = 0;

	/*------------------
	| Purchase Orders. |
	------------------*/
	poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	poln_rec.hhpo_hash = 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (wh_flag == TRUE && poln_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		posl_rec.hhpl_hash = poln_rec.hhpl_hash;
		cc = find_rec (posl, &posl_rec, GTEQ, "r");
		while (!cc && posl_rec.hhpl_hash == poln_rec.hhpl_hash)
		{
			if (posl_rec.ship_qty <= 0.00)
			{
				cc = find_rec (posl, &posl_rec, NEXT, "r");
				continue;
			}
			ShipQty += posl_rec.ship_qty,

			cc = find_rec (posl, &posl_rec, NEXT, "r");
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	return (ShipQty);
}

float
CalcMendStk (
 long	hhwh_hash)
{
	float	AddStock 	= 	0.00;

	inme_rec.hhwh_hash = hhwh_hash;
	cc = find_rec (inme, &inme_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	AddStock = 	inme_rec.sales;

	return (AddStock);
}

float
SK_CalcAlloc (
 long	INLO_HASH)
{
	float	QtyAlloc	= 	0.00;

	inla_rec.inlo_hash	=	INLO_HASH;
	inla_rec.pid		=	0L;
	inla_rec.line_no	=	0;
	cc = find_rec (inla, &inla_rec, GTEQ, "r");

	while (!cc && inla_rec.inlo_hash == INLO_HASH)
	{
		QtyAlloc	+= 	inla_rec.qty_alloc;
		cc = find_rec (inla, &inla_rec, NEXT, "r");
	}
	return (QtyAlloc);
}

int
ItemUserDefined (void)
{
	char	udFieldType [11];
	char	udFieldDesc [41];

	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open (18, 4, 11, 
				  "Item User Defined Prompts and fields", 
				  comm_rec.co_no, comm_rec.co_name,
				  comm_rec.est_no, comm_rec.est_name,
				  (char *) 0, (char *) 0);

	Dsp_saverec ("Code|     User Defined Prompt      |Field Type|       User Defined Field Value.        ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	/*---------------------------------------------
	| Open iuds file for user dcode descriptions. |
	---------------------------------------------*/
	open_rec (udih, udih_list, UDIH_NO_FIELDS, "udih_id_no");
	open_rec (udid, udid_list, UDID_NO_FIELDS, "udid_id_no");

	/*------------------------------------------------------
	| Read inventory User Defined Specification type file. |
	------------------------------------------------------*/
	strcpy (udih_rec.co_no,comm_rec.co_no);
	udih_rec.prompt_no = 0;
	cc = find_rec (udih,&udih_rec,GTEQ,"r");
	while (!cc && !strcmp (udih_rec.co_no,comm_rec.co_no))
	{
		/*-----------------------------------------
		| Read inventory Item User Defined codes. |
		-----------------------------------------*/
		udid_rec.hhbr_hash 	=	inmr_rec.hhbr_hash;
		udid_rec.udih_hash 	=	udih_rec.udih_hash;
		cc = find_rec (udid, &udid_rec, COMPARISON, "r");
		if (cc)
			memset (&udid_rec, 0, sizeof (udid_rec));

		if (UD_CHAR)
		{
			strcpy (udFieldType, "Char");
			strcpy (udFieldDesc, udid_rec.field_chr);
	
		}
		if (UD_INT)
		{
			strcpy (udFieldType, "Integer");
			sprintf (udFieldDesc, "%5d", udid_rec.field_int);
		}
		if (UD_FLOAT)
		{
			strcpy (udFieldType, "Float");
			sprintf (udFieldDesc, "%14.4f", udid_rec.field_flt);
		}
		if (UD_DOUBLE)
		{
			strcpy (udFieldType, "Double");
			sprintf (udFieldDesc, "%14.2f", udid_rec.field_dbl);
		}
		sprintf (disp_str, " %02d ^E%-30.30s^E %-8.8s ^E%-40.40s",
					udih_rec.prompt_no,
					udih_rec.prompt_desc,
					udFieldType,
					udFieldDesc);

		Dsp_saverec (disp_str);
		cc = find_rec (udih,&udih_rec,NEXT,"r");
	}
	Dsp_srch ();
	Dsp_close ();

	NormalRedraw ();

	abc_fclose (udih);
	abc_fclose (udid);
	return (EXIT_SUCCESS);
}

int
UserDefined (void)
{
	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open (18, 4, 11, 
				  "User Defined Prompts and fields", 
				  comm_rec.co_no, comm_rec.co_name,
				  comm_rec.est_no, comm_rec.est_name,
				  (char *) 0, (char *) 0);

	Dsp_saverec (" Category / Item No | No | User Defined Prompt | Code |              Description.                ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	/*---------------------------------------------
	| Open iuds file for user dcode descriptions. |
	---------------------------------------------*/
	open_rec (iudc, iudc_list, IUDC_NO_FIELDS, "iudc_id_no");
	open_rec (iuds, iuds_list, IUDS_NO_FIELDS, "iuds_id_no");
	open_rec (iudi, iudi_list, IUDI_NO_FIELDS, "iudi_id_no");

	/*------------------------------------------------------
	| Read inventory User Defined Specification type file. |
	------------------------------------------------------*/
	strcpy (iuds_rec.co_no,comm_rec.co_no);
	iuds_rec.spec_no = 0;
	cc = find_rec (iuds,&iuds_rec,GTEQ,"r");
	while (!cc && !strcmp (iuds_rec.co_no,comm_rec.co_no))
	{
		/*-----------------------------------------
		| Read inventory Item User Defined codes. |
		-----------------------------------------*/
		iudi_rec.hhbr_hash 	=	inmr_rec.hhbr_hash;
		iudi_rec.hhcf_hash 	=	0L;
		iudi_rec.spec_no	=	iuds_rec.spec_no;
		strcpy (iudi_rec.code,  "  ");
		cc = find_rec (iudi, &iudi_rec, GTEQ, "r");
		while (!cc && iudi_rec.hhbr_hash == inmr_rec.hhbr_hash &&
				  iudi_rec.spec_no == iuds_rec.spec_no)
		{
			/*--------------------------------------------------
			| Read inventory User Defined specification codes. |
			--------------------------------------------------*/
			strcpy (iudc_rec.co_no, comm_rec.co_no);
			iudc_rec.spec_no = iudi_rec.spec_no;
			strcpy (iudc_rec.code, iudi_rec.code);
			cc = find_rec (iudc,&iudc_rec,COMPARISON,"r");
			if (!cc)
			{
				sprintf (disp_str, "(%s) %-16.16s^E %2d ^E   %-15.15s   ^E  %2.2s  ^E %40.40s ",
					 "I",
					 inmr_rec.item_no,
					 iuds_rec.spec_no,
					 iuds_rec.spec_desc,
					 iudc_rec.code,
					 iudc_rec.desc);

				Dsp_saverec (disp_str);
			}
			cc = find_rec (iudi, &iudi_rec, NEXT, "r");
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inmr_rec.category);
		cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (iuds,&iuds_rec,NEXT,"r");
			continue;
		}
		iudi_rec.hhbr_hash 	=	0L;
		iudi_rec.hhcf_hash 	=	excf_rec.hhcf_hash;
		iudi_rec.spec_no	=	iuds_rec.spec_no;
		strcpy (iudi_rec.code, "  ");
		cc = find_rec (iudi, &iudi_rec, GTEQ, "r");
		while (!cc && iudi_rec.hhcf_hash == excf_rec.hhcf_hash &&
					  iudi_rec.spec_no == iuds_rec.spec_no)
		{
			/*--------------------------------------------------
			| Read inventory User Defined specification codes. |
			--------------------------------------------------*/
			strcpy (iudc_rec.co_no, comm_rec.co_no);
			iudc_rec.spec_no = iudi_rec.spec_no;
			strcpy (iudc_rec.code, iudi_rec.code);
			cc = find_rec (iudc,&iudc_rec,COMPARISON,"r");
			if (!cc)
			{
				sprintf (disp_str, "(%s) %-16.16s^E %2d ^E   %-15.15s   ^E  %2.2s  ^E %40.40s ",
					 "C",
					 excf_rec.cat_no,
					 iuds_rec.spec_no,
					 iuds_rec.spec_desc,
					 iudc_rec.code,
					 iudc_rec.desc);

				Dsp_saverec (disp_str);
			}
			cc = find_rec (iudi, &iudi_rec, NEXT, "r");
		}
		cc = find_rec (iuds,&iuds_rec,NEXT,"r");
	}
	Dsp_srch ();
	Dsp_close ();

	NormalRedraw ();

	abc_fclose (iudc);
	abc_fclose (iuds);
	abc_fclose (iudi);
	return (EXIT_SUCCESS);
}

int
OutletInventory (void)
{
	char	wk_date1 [11],
			wk_date2 [11];

	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open (15, 4, 11, 
				  "Outlet Inventory Display", 
				  comm_rec.co_no, comm_rec.co_name,
				  comm_rec.est_no, comm_rec.est_name,
				  (char *) 0, (char *) 0);

	Dsp_saverec ("Customer|              Customer Name               |     Stock On Hand       | Date last  |  Date  of  ");
	Dsp_saverec (" Number |                                          |Display Area|Storage Area| Stock take |   Expiry.  ");
	Dsp_saverec (std_foot);

	/*-----------------------------
	| Open Inventory outlet file. |
	-----------------------------*/
	open_rec ("inoi", inoi_list, INOI_NO_FIELDS, "inoi_id_no2");

	/*------------------------------------------------------
	| Read inventory User Defined Specification type file. |
	------------------------------------------------------*/
	inoi_rec.hhbr_hash 	=	inmr_rec.hhbr_hash;
	inoi_rec.hhcu_hash 	=	0L;
	inoi_rec.stake_date =	0L;
	cc = find_rec (inoi, &inoi_rec, GTEQ, "r");
	while (!cc && inoi_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		cumr_rec.hhcu_hash	=	inoi_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inoi, &inoi_rec, NEXT, "r");
			continue;
		}
		sprintf (wk_date1, DateToString (inoi_rec.stake_date));
		sprintf (wk_date2, DateToString (inoi_rec.item_exp));

		sprintf (disp_str, " %6.6s | %40.40s |%11.2f |%11.2f | %10.10s | %10.10s ",
				 cumr_rec.dbt_no,
				 cumr_rec.dbt_name,
				 inoi_rec.tot_disp,
				 inoi_rec.tot_store,
				 wk_date1, wk_date2);

		Dsp_saverec (disp_str);
		cc = find_rec (inoi,&inoi_rec,NEXT,"r");
	}
	Dsp_srch ();
	Dsp_close ();

	NormalRedraw ();

	abc_fclose ("inoi");
    return (EXIT_SUCCESS);
}

/*=======================================
| Load supplier details into table	|
=======================================*/
int
SupplyWarehouseDisplay (void)
{
	double	UpliftValue	=	0.00;
	float	DefaultLead	=	0.00;
	char	br_no [3],
			wh_no [3],
			acronym [10];

	open_rec (inws, inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd, inwd_list, INWD_NO_FIELDS, "inwd_id_no");

	/*-----------------------------------
	| setup warehouse item display		|
	-----------------------------------*/
	Dsp_open (0, 4, 12);
	Dsp_saverec ("  S u p p l y    |R e c e i v i n g |Pri|  Lead | Cost Price | Uplift | Uplift | Transfer   | Minimum  |  Normal  |  Order  ");
	Dsp_saverec ("Br / Wh  Acronym |Br / Wh   Acronym |No.|  Time |            | Percent| Amount |    Price   | Ord. Qty | Ord. Qty | Multiple");
	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");

	/*-----------------------------------
	| load data from inventory supplier	|
	-----------------------------------*/
	inws_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	inws_rec.hhcf_hash	=	0L;
	strcpy (inws_rec.sup_priority, "1");
	inws_rec.hhcc_hash	=	0L;
	cc = find_rec (inws, &inws_rec, GTEQ, "r");
	while (!cc && inws_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		ccmr2_rec.hhcc_hash	=	inws_rec.hhcc_hash;
		cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inws, &inws_rec, NEXT, "r");
			continue;
		}	
		strcpy (br_no, ccmr2_rec.est_no);
		strcpy (wh_no, ccmr2_rec.cc_no);
		strcpy (acronym, ccmr2_rec.acronym);

		inwd_rec.inws_hash	=	inws_rec.inws_hash;
		inwd_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inwd_rec.hhcf_hash	=	0L;
		inwd_rec.hhcc_hash	=	0L;
		cc = find_rec (inwd, &inwd_rec, GTEQ, "r");
		while (!cc && inwd_rec.inws_hash == inws_rec.inws_hash)
		{
			if (inwd_rec.dflt_lead [0] == 'S')
				DefaultLead	=	inwd_rec.sea_time;
			if (inwd_rec.dflt_lead [0] == 'A')
				DefaultLead	=	inwd_rec.air_time;
			if (inwd_rec.dflt_lead [0] == 'L')
				DefaultLead	=	inwd_rec.lnd_time;

			ccmr2_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
			cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (inwd, &inwd_rec, NEXT, "r");
				continue;
			}		

			FindCost ();

			UpliftValue	=	0.00;
			if (inwd_rec.upft_pc > 0.00)
			{
				UpliftValue	=	(double) inwd_rec.upft_pc;
				UpliftValue	*=	local_rec.l_cost;
				UpliftValue	=	DOLLARS (UpliftValue);
				UpliftValue	=	twodec (UpliftValue);
			}
			if (inwd_rec.upft_amt > 0.00)
				UpliftValue	+=	DOLLARS (inwd_rec.upft_amt);
			
			/*---------------------------
			| store data				|
			---------------------------*/
			if (!displayCostOk)
			{
				sprintf (disp_str, "%s / %s %9.9s^E%s / %s %9.9s ^E %s ^E%6.2f ^E%12.12s^E%8.8s^E%8.8s^E%-12.12s^E%9.2f ^E%9.2f ^E%9.2f ",
					br_no,
					wh_no,
					acronym,
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					ccmr2_rec.acronym,
					inws_rec.sup_priority,
					DefaultLead,
					" ",
					" ",
					" ",
					" ",
					inws_rec.min_order,
					inws_rec.norm_order,
					inws_rec.ord_multiple
				);
			}
			else
			{
				strcpy (err_str, "%s / %s %9.9s^E%s / %s %9.9s ^E %s ^E%6.2f ^E%11.2f ^E%6.2f  ^E%7.2f ^E%11.2f ^E%9.2f ^E%9.2f ^E%9.2f ");

				sprintf 
				(
					disp_str,
					err_str, 
					br_no,
					wh_no,
					acronym,
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					ccmr2_rec.acronym,
					inws_rec.sup_priority,
					DefaultLead,
					local_rec.l_cost,
					inwd_rec.upft_pc,
					DOLLARS (inwd_rec.upft_amt),
					local_rec.l_cost + UpliftValue,
					inws_rec.min_order,
					inws_rec.norm_order,
					inws_rec.ord_multiple
				);
			}
			Dsp_saverec (disp_str);
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
		}
		cc = find_rec (inws, &inws_rec, NEXT, "r");
	}
	abc_fclose (inws);
	abc_fclose (inwd);
	Dsp_srch ();
	Dsp_close ();
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

/*=======================================
| Load supplier details into table	    |
=======================================*/
int
GlobalSupplyWarehouse (void)
{
	double	UpliftValue	=	0.00;
	float	DefaultLead	=	0.00;
	char	br_no [3],
			wh_no [3],
			acronym [10];

	open_rec (inws, inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd, inwd_list, INWD_NO_FIELDS, "inwd_id_no");
	abc_selfield (excf, "excf_hhcf_hash");

	/*-----------------------------------
	| setup warehouse item display		|
	-----------------------------------*/
	Dsp_open (0, 4, 12);
	Dsp_saverec (" Supply |Receive |  Category   |Pri|  Lead | Cost Price | Uplift | Uplift | Transfer   | Minimum  |  Normal  |  Order  ");
	Dsp_saverec ("Br / Wh |Br / Wh |             |No.|  Time |            | Percent| Amount |    Price   | Ord. Qty | Ord. Qty | Multiple");
	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");

	/*-----------------------------------
	| load data from inventory supplier	|
	-----------------------------------*/
	inws_rec.hhbr_hash	=	0L;
	inws_rec.hhcc_hash	=	0L;
	inws_rec.hhcf_hash	=	0L;
	strcpy (inws_rec.sup_priority, " ");
	cc = find_rec (inws, &inws_rec, GTEQ, "r");
	while (!cc && inws_rec.hhbr_hash == 0L)
	{
		ccmr2_rec.hhcc_hash	=	inws_rec.hhcc_hash;
		cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inws, &inws_rec, NEXT, "r");
			continue;
		}	
		strcpy (br_no, ccmr2_rec.est_no);
		strcpy (wh_no, ccmr2_rec.cc_no);
		strcpy (acronym, ccmr2_rec.acronym);

		inwd_rec.inws_hash	=	inws_rec.inws_hash;
		inwd_rec.hhbr_hash	=	0L;
		inwd_rec.hhcc_hash	=	0L;
		inwd_rec.hhcf_hash	=	0L;
		strcpy (inwd_rec.sup_priority, "1");
		cc = find_rec (inwd, &inwd_rec, GTEQ, "r");
		while (!cc && inwd_rec.inws_hash == inws_rec.inws_hash)
		{
			if (inwd_rec.dflt_lead [0] == 'S')
				DefaultLead	=	inwd_rec.sea_time;
			if (inwd_rec.dflt_lead [0] == 'A')
				DefaultLead	=	inwd_rec.air_time;
			if (inwd_rec.dflt_lead [0] == 'L')
				DefaultLead	=	inwd_rec.lnd_time;

			ccmr2_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
			cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (inwd, &inwd_rec, NEXT, "r");
				continue;
			}		

			FindCost ();

			UpliftValue	=	0.00;
			if (inwd_rec.upft_pc > 0.00)
			{
				UpliftValue	=	(double) inwd_rec.upft_pc;
				UpliftValue	*=	local_rec.l_cost;
				UpliftValue	=	DOLLARS (UpliftValue);
				UpliftValue	=	twodec (UpliftValue);
			}
			if (inwd_rec.upft_amt > 0.00)
				UpliftValue	+=	DOLLARS (inwd_rec.upft_amt);
			

			strcpy (excf_rec.cat_no, "           ");
			if (inws_rec.hhcf_hash > 0L)
			{
				excf_rec.hhcf_hash	=	inws_rec.hhcf_hash;
				cc = find_rec (excf, &excf_rec, COMPARISON, "r");
				if (cc)
					strcpy (excf_rec.cat_no, "           ");
			}
			/*---------------------------
			| store data				|
			---------------------------*/
			if (!displayCostOk)
			{
				sprintf 
				(
					disp_str, 
					"%s / %s ^E%s / %s ^E %-11.11s ^E %s ^E%6.2f ^E%12.12s^E%8.8s^E%8.8s^E%-12.12s^E%9.2f ^E%9.2f ^E%9.2f ",
					br_no,
					wh_no,
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					excf_rec.cat_no,
					inws_rec.sup_priority,
					DefaultLead,
					" ",
					" ",
					" ",
					" ",
					inws_rec.min_order,
					inws_rec.norm_order,
					inws_rec.ord_multiple
				);
			}
			else
			{
				strcpy (err_str, "%s / %s ^E%s / %s ^E %-11.11s ^E %s ^E%6.2f ^E%11.2f ^E%6.2f  ^E%7.2f ^E%11.2f ^E%9.2f ^E%9.2f ^E%9.2f ");
				sprintf 
				(
					disp_str, 
					err_str, 
					br_no,
					wh_no,
					ccmr2_rec.est_no,
					ccmr2_rec.cc_no,
					excf_rec.cat_no,
					inws_rec.sup_priority,
					DefaultLead,
					local_rec.l_cost,
					inwd_rec.upft_pc,
					DOLLARS (inwd_rec.upft_amt),
					local_rec.l_cost + UpliftValue,
					inws_rec.min_order,
					inws_rec.norm_order,
					inws_rec.ord_multiple
				);
			}
			Dsp_saverec (disp_str);
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
		}
		cc = find_rec (inws, &inws_rec, NEXT, "r");
	}
	abc_fclose (inws);
	abc_fclose (inwd);
	abc_selfield (excf, "excf_id_no");
	Dsp_srch ();
	Dsp_close ();
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

void
InitML (void)
{
	int		i;

    InitMonthArray ();

	for (i = 0; i < 12; i++)
        SetMonthName (i, ML (month_nm [i]));

	cost_desc [0].c_desc = strdup (ML ("Last    "));
	cost_desc [1].c_desc = strdup (ML ("Average "));
	cost_desc [2].c_desc = strdup (ML ("FIFO    "));
	cost_desc [3].c_desc = strdup (ML ("LIFO    "));
	cost_desc [4].c_desc = strdup (ML ("Serial  "));
	cost_desc [5].c_desc = strdup (ML ("Previous"));
	cost_desc [6].c_desc = strdup (ML ("Standard"));
	cost_desc [7].c_desc = strdup (ML ("Unknown "));

	strcpy (mlStkDisp [0],  ML ("Supercession"));
	strcpy (mlStkDisp [1],  ML ("24  M O N T H S    S A L E S   H I S T O R Y   D I S P L A Y"));
	strcpy (mlStkDisp [2],  ML ("AIR "));
	strcpy (mlStkDisp [3],  ML ("Active Status     "));
	strcpy (mlStkDisp [4],  ML ("Active Status Unknown"));
	strcpy (mlStkDisp [5],  ML ("Alpha Code        "));
	strcpy (mlStkDisp [6],  ML ("Alternate Number  "));
	strcpy (mlStkDisp [7],  ML ("Alternate UOM"));
	strcpy (mlStkDisp [8],  ML ("Automatic."));
	strcpy (mlStkDisp [9],  ML ("Available / On Order"));
	strcpy (mlStkDisp [10], ML ("BONUS ITEM"));
	strcpy (mlStkDisp [11], ML ("Bar Code Number   "));
	strcpy (mlStkDisp [12], ML ("Base UOM"));
	strcpy (mlStkDisp [13], ML ("Buying Group"));
	strcpy (mlStkDisp [14], ML ("Committed"));
	strcpy (mlStkDisp [15], ML ("Confirmed Rec Date"));
	strcpy (mlStkDisp [16], ML ("Contact Name"));
	strcpy (mlStkDisp [17], ML ("Cover        "));
	strcpy (mlStkDisp [18], ML ("Cover Required"));
	strcpy (mlStkDisp [19], ML ("Date Ordered    "));
	strcpy (mlStkDisp [20], ML ("Date Received   "));
	strcpy (mlStkDisp [21], ML ("Del Inst  1"));
	strcpy (mlStkDisp [22], ML ("Del Inst  2"));
	strcpy (mlStkDisp [23], ML ("Del Inst  3"));
	strcpy (mlStkDisp [24], ML ("Destination"));
	strcpy (mlStkDisp [25], ML ("Direct Delivery   "));
	strcpy (mlStkDisp [26], ML ("END OF RANGE"));
	strcpy (mlStkDisp [27], ML ("Exch Rate"));
	strcpy (mlStkDisp [28], ML ("Days"));
	strcpy (mlStkDisp [29], ML ("FOR CUST."));
	strcpy (mlStkDisp [30], ML ("FOR STOCK"));
	strcpy (mlStkDisp [31], ML ("Goods Receipt Number"));
	strcpy (mlStkDisp [32], ML ("HELD OVER STOCK ON HAND FOR NEXT PERIOD"));
	strcpy (mlStkDisp [33], ML ("INDEX. "));
	strcpy (mlStkDisp [34], ML ("Item Number"));
	strcpy (mlStkDisp [35], ML ("LAND"));
	strcpy (mlStkDisp [36], ML ("Landed Cost   "));
	strcpy (mlStkDisp [37], ML ("Last Receipt"));
	strcpy (mlStkDisp [38], ML ("Last Six Months Sales "));
	strcpy (mlStkDisp [39], ML ("Lead Time"));
	strcpy (mlStkDisp [40], ML ("Lead Times   "));
	if (envVarIkeaSystem)
		strcpy (mlStkDisp [41], ML ("Maker Number      "));
	else
		strcpy (mlStkDisp [41], ML ("Brand Number      "));
	strcpy (mlStkDisp [42], ML ("Manual."));
	strcpy (mlStkDisp [43], ML ("Max"));
	strcpy (mlStkDisp [44], ML ("Min Order Qty"));
	strcpy (mlStkDisp [45], ML ("Min"));
	strcpy (mlStkDisp [46], ML ("NO SHIPMENT DETAILS"));
	strcpy (mlStkDisp [47], ML ("NONE.   "));
	strcpy (mlStkDisp [48], ML ("Net Requirement"));
	strcpy (mlStkDisp [49], ML ("NO."));
	strcpy (mlStkDisp [50], ML ("Normal Order Qty"));
	strcpy (mlStkDisp [51], ML ("On Order"));
	strcpy (mlStkDisp [52], ML ("Order Multiple"));
	strcpy (mlStkDisp [53], ML ("P/Order No"));
	strcpy (mlStkDisp [54], ML ("Port Origin"));
	strcpy (mlStkDisp [55], ML ("Pre Defined"));
	strcpy (mlStkDisp [56], ML ("Proj. Sales Method"));
	strcpy (mlStkDisp [57], ML ("Purchase order No."));
	strcpy (mlStkDisp [58], ML ("Qty Recommended"));
	strcpy (mlStkDisp [59], ML ("Quantity Ordered   "));
	strcpy (mlStkDisp [60], ML ("Quantity Received "));
	strcpy (mlStkDisp [61], ML ("Quantity"));
	strcpy (mlStkDisp [62], ML ("REORDER REVIEW BY (Company)  "));
	strcpy (mlStkDisp [63], ML ("REORDER REVIEW BY (Warehouse)"));
	strcpy (mlStkDisp [64], ML ("Review Period"));
	strcpy (mlStkDisp [65], ML ("S A L E S   H I S T O R Y   D I S P L A Y"));
	strcpy (mlStkDisp [66], ML ("S A L E S  /  P R O F I T    D I S P L A Y"));
	strcpy (mlStkDisp [67], ML ("SEA "));
	strcpy (mlStkDisp [68], ML ("SERIAL NUMBER"));
	strcpy (mlStkDisp [69], ML ("SHIPMENT DETAILS"));
	strcpy (mlStkDisp [70], ML ("START  RANGE"));
	strcpy (mlStkDisp [71], ML ("Safety Stock"));
	strcpy (mlStkDisp [72], ML ("Sales Orders"));
	strcpy (mlStkDisp [73], ML ("Selling Group"));
	strcpy (mlStkDisp [74], ML ("Serial No"));
	strcpy (mlStkDisp [75], ML ("Ship Arrive"));
	strcpy (mlStkDisp [76], ML ("Ship Depart"));
	strcpy (mlStkDisp [77], ML ("Ship Method"));
	strcpy (mlStkDisp [78], ML ("Shipment Method"));
	strcpy (mlStkDisp [79], ML ("Shipment No"));
	strcpy (mlStkDisp [80], ML ("Shipment Number"));
	strcpy (mlStkDisp [81], ML ("Short Code Number "));
	strcpy (mlStkDisp [82], ML ("Standard UOM :  UNKNOWN UOM"));
	strcpy (mlStkDisp [83], ML ("Standard UOM : "));
	strcpy (mlStkDisp [84], ML ("Status : P(lanned)); F(irmed)); I(ssuing)); A(llocated)); R(eleased and C(losing"));
	strcpy (mlStkDisp [85], ML ("Stock Available"));
	strcpy (mlStkDisp [86], ML ("Stock Backorder"));
	strcpy (mlStkDisp [87], ML ("Stock Committed"));
	strcpy (mlStkDisp [88], ML ("Stock Forward"));
	strcpy (mlStkDisp [89], ML ("Stock On Order"));
	strcpy (mlStkDisp [90], ML ("Stock in Transit (IN)"));
	strcpy (mlStkDisp [91], ML ("Stock in Transit (OUT)"));
	strcpy (mlStkDisp [92], ML ("Stock on Hand"));
	strcpy (mlStkDisp [93], ML ("Sup. Lead Time"));
	strcpy (mlStkDisp [94], ML ("Supplier No"));
	strcpy (mlStkDisp [95], ML ("Supplier Number"));
	strcpy (mlStkDisp [96], ML ("TOTALS"));
	strcpy (mlStkDisp [97], ML ("TRANSFER"));
	strcpy (mlStkDisp [98], ML ("Total Cover"));
	strcpy (mlStkDisp [99], ML ("UNKN"));
	strcpy (mlStkDisp [100], ML ("Unknown "));
	strcpy (mlStkDisp [101], ML ("Vessel"));
	strcpy (mlStkDisp [102], ML ("WORKS ORDER"));
	strcpy (mlStkDisp [103], ML ("Warehouse Number"));
	strcpy (mlStkDisp [104], ML ("Week (s)"));
	strcpy (mlStkDisp [105], ML ("Weeks Demand"));
	strcpy (mlStkDisp [106], ML ("Weeks"));
	strcpy (mlStkDisp [107], ML ("YES"));
	strcpy (mlStkDisp [108], ML ("^1 NOTE ^6: The following FIFO records are not included in current Stock On Hand."));
	strcpy (mlStkDisp [109], ML ("^1FIFO records not included in SOH ^6"));
	strcpy (mlStkDisp [110], ML ("On Hand"));
	strcpy (mlStkDisp [111], ML ("Price Breaks by"));
	strcpy (mlStkDisp [112], ML ("Value   "));
	strcpy (mlStkDisp [113], ML ("Quantity"));
	strcpy (mlStkDisp [114], ML ("Break"));
	strcpy (mlStkDisp [115], ML ("Price Type"));
	strcpy (mlStkDisp [116], ML ("Base"));
	strcpy (mlStkDisp [117], ML ("Min. Stock"));
	strcpy (mlStkDisp [118], ML ("PHANTOM LINE"));
	strcpy (mlStkDisp [119], ML ("Last Freezing Date"));
	strcpy (mlStkDisp [120], ML ("Committed Sales Orders"));
	strcpy (mlStkDisp [121], ML ("Despatched Order"));
	strcpy (mlStkDisp [122], ML ("Orders for Collection"));
	strcpy (mlStkDisp [123], ML ("REAL TIME COMMITTED STOCK"));
}

/*===============================================
| Read the ffdm record for the appropriate year	|
| if record doesn't exist and year is valid		|
| then get data from incc record				|
===============================================*/
void
LoadHistory (
 long	hhbrHash,
 long	hhccHash,
 long	StartDate,
 char	*type)
{
	int		i;

	CalcDates (StartDate);

	for (i = 0; i < MAX_MONTHS; i++)
	    local_rec.demand_value [i]		= 0.00;

	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	sprintf (ffdm_rec.type, "%1.1s", type);
	ffdm_rec.date	=	store_dates [0].StartDate;
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc && ffdm_rec.hhbr_hash	==	hhbrHash &&
				  ffdm_rec.hhcc_hash	==	hhccHash &&
				  ffdm_rec.type [0] 		==  type [0] &&
				  ffdm_rec.date 		<= store_dates [MAX_MONTHS - 1].EndDate)
	{
		for (i = 0; i < MAX_MONTHS; i++)
		{
			if (ffdm_rec.date >= store_dates [i].StartDate &&
			    ffdm_rec.date <= store_dates [i].EndDate)
			{
				store_dates [i].QtySold	+= twodec (ffdm_rec.qty);
			}
		}
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
	for (i = 0; i < MAX_MONTHS; i++)
		local_rec.demand_value [i] = store_dates [i].QtySold;
}

/*===============================================
| Calculate start and end dates for each month. |
===============================================*/
void
CalcDates (
 long	StartDate)
{
	long	CalcStartDate;
	int		i;
	int		tmp_dmy [3];

	DateToDMY (StartDate, &tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]);
	tmp_dmy [2] -= 3;
	CalcStartDate = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);

	for (i = 0; i < MAX_MONTHS; i++)
	{
		store_dates [i].StartDate	=	MonthStart (CalcStartDate);
		store_dates [i].EndDate		=	MonthEnd (CalcStartDate);
		store_dates [i].QtySold		=	0.00;
		CalcStartDate	=	MonthEnd (CalcStartDate) + 1;
	}
}

void 
InitMonthArray (void)
{
    int iPos;

    for (iPos = 0; iPos < 12; iPos++)
        month_nm [iPos] = NULL;

    SetMonthName (0, "January");
    SetMonthName (1, "February");
    SetMonthName (2, "March");
    SetMonthName (3, "April");
    SetMonthName (4, "May");
    SetMonthName (5, "June");
    SetMonthName (6, "July");
    SetMonthName (7, "August");
    SetMonthName (8, "September");
    SetMonthName (9, "October");
    SetMonthName (10, "November");
    SetMonthName (11, "December");
}

void 
DeleteMonthArray (void)
{
    int iPos;

    for (iPos = 0; iPos < 12; iPos++)
    {
        if (month_nm [iPos] != NULL)
        {
            free (month_nm [iPos]);
            month_nm [iPos] = NULL;
        }
    }
}
void 
SetMonthName (
 int iMonth, 
 char* szMonthName)
{
	char	*pcHoldMonth;

    if ((iMonth >=0) && (iMonth < 12))
    {
		pcHoldMonth = month_nm [iMonth];
        month_nm [iMonth] = NULL;
        month_nm [iMonth] = (char*) malloc (strlen (szMonthName) + 1);
        strcpy (month_nm [iMonth], szMonthName);
        if (pcHoldMonth != NULL)
            free (pcHoldMonth);
    }
}
int
ProcessGraph (void)
{
	char	*headingNames [] =
	{
		"Actual",
		"Linear LSA",
		"Seasonal Demand",
		"Local Linear",
		"Focus Forecast"
	};

	char	*mth_nms [] =
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};
	char	response [6] [11];
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [48],
			gpx_ch_indx [6],
			head_str [130],
			pr_head [130],
			method_chk;
	int		indx [5],
			i,
			j,
			k,
			cnt;			/* No. of values to extrapolate	*/
	double	gr_val [100];		/* Array to store result (s)	*/


	cnt = 	calc_LSA 
		  	(
				envVarLrpMethods, 
				hhsiHash,
				comm_rec.inv_date, 
				FALSE,
				36,
				LRP_PASSED_MONTH, 
				"1"
			);

	for (i = 0; i < 5; i++)
	{
		method_chk = 'A' + i - 1;
		if (i == 0)
			strcpy (response [i], "Yes");
		else
			strcpy (response [i], "No");

		if (i == 1 && incc_rec.ff_method [0] == 'A')
			strcpy (response [i], "Highlight");
		if (i == 2 && incc_rec.ff_method [0] == 'B')
			strcpy (response [i], "Highlight");
		if (i == 3 && incc_rec.ff_method [0] == 'C')
			strcpy (response [i], "Highlight");
		if (i == 4 && incc_rec.ff_method [0] == 'D')
			strcpy (response [i], "Highlight");
	}
	/*----------------------------------
	| Setup graphics window & headings |
	----------------------------------*/
	lp_x_off = 0;
	lp_y_off = 3;
	gr_win.x_posn = 0;
	gr_win.y_posn = 3;
	gr_win.x_size = 130;
	gr_win.y_size = 15;

	/*-------------------------
	| Load data to be graphed |
	-------------------------*/
	j = 0;
	strcpy (head_str, "");
	strcpy (gpx_ch_indx, "");
	for (i = 0; i < 5; i++)
	{
		if (response [i] [0] != 'N')
		{
			indx [j] = i;
			j++;
			if (strlen (head_str))
				strcat (head_str, " vs. ");
			strcat (head_str, headingNames [i]);
			strcat (gpx_ch_indx, (response [i] [0] == 'H') ? "1" : "2");
		}
	}
	gr_nam.pr_head = pr_head;
	gr_nam.heading = head_str;
	gr_nam.gpx_ch_indx = gpx_ch_indx;
	switch (j)
	{
		case	1:
			k = (cnt > 35) ? 48 : cnt + 13;
			break;

		case	2:
			k = (cnt > 17) ? 30 : cnt + 13;
			break;

		case	3:
			k = (cnt > 7) ? 20 : cnt + 13;
			break;

		case	4:
			k = (cnt > 2) ? 15 : cnt + 13;
			break;

		case	5:
			k = 12;
			break;

		default:
			return (EXIT_SUCCESS);
	}

	for (i = 0; i < k; i++)
	{
		gr_tits [i]	= mth_nms [ (37 + 12 + i + LSA_mnth - k) % 12];
		gr_val [i]	= 
			LSA_result [indx [0]] [36 + 12 + i - k];
		if (j > 1)
			gr_val [i+k]	= 
			LSA_result [indx [1]] [36 + 12 + i - k];
		if (j > 2)
			gr_val [i+ (k*2)]	= 
			LSA_result [indx [2]] [36 + 12 + i - k];
		if (j > 3)
			gr_val [i+ (k*3)]	= 
			LSA_result [indx [3]] [36 + 12 + i - k];
		if (j > 4)
			gr_val [i+ (k*4)]	= 
			LSA_result [indx [4]] [36 + 12 + i - k];
	}
	gr_nam.legends = gr_tits;

	switch (j)
	{
		case	1:
			GR_graph (&gr_win, GR_TYPE_1BAR, k, &gr_nam, gr_val, NULL);
			break;

		case	2:
			GR_graph (&gr_win, GR_TYPE_2BAR, k, &gr_nam, gr_val, NULL);
			break;

		case	3:
			GR_graph (&gr_win, GR_TYPE_3BAR, k, &gr_nam, gr_val, NULL);
			break;

		case	4:
			GR_graph (&gr_win, GR_TYPE_4BAR, k, &gr_nam, gr_val, NULL);
			break;

		case	5:
			GR_graph (&gr_win, GR_TYPE_5BAR, k, &gr_nam, gr_val, NULL);
			break;
	}
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

int
FormulaStatistics (void)
{
	char	disp_str [200];
	int		cnt;
	char	*formatMask	= "%s   %s   |  %13.2f |  %13.2f |  %13.2f |  %13.2f |  %13.2f%s| %10.2f | %10.2f ^6";

	cnt = 	calc_LSA 
		  	(
				envVarLrpMethods, 
				hhsiHash,
				comm_rec.inv_date, 
				FALSE,
				36,
				LRP_PASSED_MONTH, 
				"1"
			);

	Dsp_open (4, 9, 5);
	Dsp_saverec (" Method | Actual Average |Forecast Average|   Calculated   |   Last Month   |   Projected    | Calculated | Calculated ");
	Dsp_saverec ("        | Last 3 Months  |  Last 3 Months |  Weeks Demand  |     Sales      |     Sales      |  Deviation |Percent Err.");

	Dsp_saverec (" [EDIT/END]");

	open_rec (lrph, lrph_list, LRPH_NO_FIELDS, "lrph_hhwh_hash");
	lrph_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	cc = find_rec (lrph, &lrph_rec, GTEQ, "r");
	while (!cc && lrph_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		sprintf 
		(
			disp_str, 
			formatMask,
			(incc_rec.ff_method [0] == 'A') ? "^1 " : " ", "A",
			 lrph_rec.a_actual, 	
			 lrph_rec.a_forecast,
			 lrph_rec.a_wks_dem, 	
			 LSA_result [0] [34], 
			 LSA_result [1] [35], 	
			(lrph_rec.a_sqr_err >= 99.00) ? "*" : " ", 	
			 lrph_rec.a_sqr_err, 	
			 lrph_rec.a_pc_err
		);
		Dsp_saverec (disp_str);

		sprintf 
		(
			disp_str, 
			formatMask,
			(incc_rec.ff_method [0] == 'B') ? "^1 " : " ", "B",
			 lrph_rec.b_actual, 	
			 lrph_rec.b_forecast,
			 lrph_rec.b_wks_dem, 	
			 LSA_result [0] [34], 
			 LSA_result [2] [35],
			(lrph_rec.b_sqr_err >= 99.00) ? "*" : " ", 	
			 lrph_rec.b_sqr_err, 	
			 lrph_rec.b_pc_err
		);
		Dsp_saverec (disp_str);

		sprintf 
		(
			disp_str, 
			formatMask,
			(incc_rec.ff_method [0] == 'C') ? "^1 " : " ", "C",
			 lrph_rec.c_actual, 	
			 lrph_rec.c_forecast,
			 lrph_rec.c_wks_dem, 	
			 LSA_result [0] [34], 
			 LSA_result [3] [35],
			(lrph_rec.c_sqr_err >= 99.00) ? "*" : " ", 	
			 lrph_rec.c_sqr_err, 	
			 lrph_rec.c_pc_err
		);
		Dsp_saverec (disp_str);

		sprintf 
		(
			disp_str, 
			formatMask,
			(incc_rec.ff_method [0] == 'D') ? "^1 " : " ", "D",
			 lrph_rec.d_actual, 	
			 lrph_rec.d_forecast,
			 lrph_rec.d_wks_dem, 	
			 LSA_result [0][34], 
			 LSA_result [4][35],
			(lrph_rec.d_sqr_err >= 99.00) ? "*" : " ", 	
			 lrph_rec.d_sqr_err, 	
			 lrph_rec.d_pc_err
		);
		Dsp_saverec (disp_str);
		strcpy (disp_str, ML ("NOTE:^1 * ^6 indicates a rejected line. This may have been caused by sales projection going below zero"));
		Dsp_saverec (disp_str);
		cc = find_rec (lrph, &lrph_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	NormalRedraw ();
	return (EXIT_SUCCESS);
}


int 
SpecialPrice (void)
{
	char	tempDate [11];

	open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_id_no");

	lp_x_off = 0;
	lp_y_off = 4;

	Dsp_prn_open (0, 4, 11, headingText, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	sprintf 
	(
		err_str, 
		"DATE FROM | DATE TO  |%-11.11s|%-11.11s|%-11.11s|%-11.11s|%-11.11s|%-11.11s|%-11.11s|%-11.11s|%-11.11s",
		local_rec.priceDesc [0],
		local_rec.priceDesc [1],
		local_rec.priceDesc [2],
		local_rec.priceDesc [3],
		local_rec.priceDesc [4],
		local_rec.priceDesc [5],
		local_rec.priceDesc [6],
		local_rec.priceDesc [7],
		local_rec.priceDesc [8]
	);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	sprintf (incp_rec.key, "%2.2s    ", comm_rec.co_no);
	strcpy (incp_rec.curr_code, (char *)envVarCurrCode);
	strcpy (incp_rec.status, "H");
	incp_rec.hhcu_hash 	= 0L;
	strcpy (incp_rec.area_code, "  ");
	strcpy (incp_rec.cus_type, "   ");
	incp_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	incp_rec.date_from 	= 0L;
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc &&
			!strncmp (incp_rec.curr_code, (char *) envVarCurrCode, 3) &&
			incp_rec.status [0] == 'H' &&
			incp_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf 
		(
			disp_str, 
			"%-10.10s: %-40.40s     ^E           ^E           ^E           ^E           ^E           ^E           ", 
			ML ("Comment   "),
			clip (incp_rec.comment)
		);
		Dsp_saverec (disp_str);

		sprintf (tempDate, "%-10.10s", DateToString (incp_rec.date_to));
		sprintf 
		(
			disp_str, 
			"%-10.10s^E%s^E%10.0f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f",
			DateToString (incp_rec.date_from),
			tempDate,
			DOLLARS (incp_price [0]),
			DOLLARS (incp_price [1]),
			DOLLARS (incp_price [2]),
			DOLLARS (incp_price [3]),
			DOLLARS (incp_price [4]),
			DOLLARS (incp_price [5]),
			DOLLARS (incp_price [6]),
			DOLLARS (incp_price [7]),
			DOLLARS (incp_price [8])
		);
		Dsp_saverec (disp_str);

		Dsp_saverec ("^^GGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGG");

		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}

	sprintf (incp_rec.key, "%2.2s    ", comm_rec.co_no);
	strcpy (incp_rec.curr_code, (char *)envVarCurrCode);
	strcpy (incp_rec.status, "A");
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.area_code, "  ");
	strcpy (incp_rec.cus_type, "   ");
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = 0L;
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc &&
			!strncmp (incp_rec.curr_code, (char *)envVarCurrCode, 3) &&
			incp_rec.status [0] == 'A' &&
			incp_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf 
		(
			disp_str, 
			"%-10.10s: %-40.40s     ^E           ^E           ^E           ^E           ^E           ^E           ", 
			ML ("Comment   "),
			clip (incp_rec.comment)
		);
		Dsp_saverec (disp_str);

		sprintf (tempDate, "%-10.10s", DateToString (incp_rec.date_to));
		sprintf (disp_str, "%-10.10s^E%s^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f ^E%10.0f",
				DateToString (incp_rec.date_from),
				tempDate,
				DOLLARS (incp_price [0]),
				DOLLARS (incp_price [1]),
				DOLLARS (incp_price [2]),
				DOLLARS (incp_price [3]),
				DOLLARS (incp_price [4]),
				DOLLARS (incp_price [5]),
				DOLLARS (incp_price [6]),
				DOLLARS (incp_price [7]),
				DOLLARS (incp_price [8]));

		Dsp_saverec (disp_str);

		Dsp_saverec ("^^GGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGG");

		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (incp);
	NormalRedraw ();
	return (EXIT_SUCCESS);
}

char *
LocStatDisplay (
	char	*statusString)
{
	int		i;
	char	locCode [2];
	char	locCodeDesc [31];

	open_rec (llst, llst_list, LLST_NO_FIELDS, "llst_id_no");

	strcpy (llst_rec.co_no, comm_rec.co_no);
	sprintf (llst_rec.code, "%-1.1s", statusString + 1);
	cc = find_rec (llst, &llst_rec, EQUAL, "r");
	if (cc)
		strcpy (llst_rec.desc, " ");

	strcpy (locCodeDesc," ");
	strcpy (locCode,	" ");

	for (i = 0;strlen (loc_types[i]._loc_code);i++)
	{
		if (statusString [0] == loc_types[i]._loc_code[0])
		{
			sprintf (locCodeDesc,"%-30.30s",loc_types[i]._loc_desc);
			sprintf (locCode, "%1.1s",loc_types[i]._loc_code);
			break;
		}
	}
	sprintf 
	(
		err_str, 
		"%14.14s^1%c^6%s%20.20s^1%c^6%s", 
		" ",
		locCodeDesc [0],
		locCodeDesc + 1,
		" ",
		llst_rec.desc [0],
		llst_rec.desc + 1
	);
	return (err_str);
}

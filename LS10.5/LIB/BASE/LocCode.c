/*
 *  Copyright (C) 1999 - 2001 LogisticSoftware
 *
 * $Id: LocCode.c,v 5.22 2002/03/12 02:01:42 scott Exp $
 * $Log: LocCode.c,v $
 * Revision 5.22  2002/03/12 02:01:42  scott
 * Updated to use "To Fix" when number plate.
 *
 * Revision 5.21  2001/12/14 06:35:19  scott
 * Updated as total allocated wrong
 *
 * Revision 5.20  2001/12/12 07:02:29  scott
 * Updated to change prompts for GUI
 *
 * Revision 5.19  2001/12/12 06:55:20  scott
 * Updated to remove unused variables.
 *
 * Revision 5.18  2001/12/12 06:53:22  scott
 * Updated as new check did not consider conversion
 *
 * Revision 5.17  2001/12/11 09:17:16  scott
 * Updated to add more checks on location available
 *
 * Revision 5.16  2001/12/06 05:28:12  scott
 * Updated to allow purchase returns that are allocated to an original purchase order to select lot information. The lot display will only show those Location/Lot records that belong to the original receipt. This caters for both multiple receipts of the same product on the same purchase order AND Locations/Lots being split after receipt.
 *
 * Revision 5.15  2001/11/28 03:09:32  scott
 * Updated to ignore available check if quantity input is zero.
 *
 * Revision 5.14  2001/11/09 04:43:24  scott
 * Updated to ensure batch control is set when number plates used.
 *
 * Revision 5.13  2001/11/07 08:13:36  scott
 * Updated to fix problem with split location yet only one number plate line remains.
 *
 * Revision 5.12  2001/10/22 02:29:48  scott
 * Updated to add ArrCheck, missing in some calls, would cause core dumps of MAX_TLINES Exceeded.
 *
 * Revision 5.11  2001/10/10 01:32:03  scott
 * Updated to fix auto allocation
 *
 * Revision 5.10  2001/10/09 23:18:07  scott
 * Updated for returns
 *
 *	
 */

#include	<pslscr.h>
#include	<std_decs.h>
#include	<arralloc.h>

#define		ALT_UOM		 	(llctAltUom 	[0]	== 'Y')
#define		AUTO_ALLOC	 	(llctAutoAll 	[0] == 'Y')
#define		EXP_ITEMS	 	(llctExpItems 	[0] == 'Y')
#define		DSP_ALL		 	(llctAllLocs 	[0]	== 'Y')
#define		STOCK_TAKE	 	(StockTake 	[0]	== 'Y')
#define		LOC_AVAILABLE	(inloRec.loc_status [0] == 'A' || \
							 inloRec.loc_status [0] == ' ' || \
							 !envVarThreePl || locStatusIgnore)

#define		add_list	LLAddList
									/*---------------------------------------*/
#define		MAX_LOTS	200			/*| Define No of lots for each screen.  |*/
									/*|-------------------------------------|*/
#define		offsetLocation	 1		/*|	Offset for location.				|*/
#define		offsetLotNo		 14		/*|	Offset for lot number.				|*/
#define		offsetLocType	 27		/*|	Offset for location type.			|*/
#define		offsetSlotNo	 31		/*|	Offset for supplier lot no.			|*/
#define		offsetExpiry	 41		/*|	Offset for expiry date.				|*/
#define		offsetL_UOM		 54		/*|	Offset for UOM.						|*/
#define		offsetAvailQty 	 61		/*|	Offset for Available quantity.		|*/
#define		offsetI_UOM		 75		/*|	Offset for UOM.						|*/
#define		offsetAllocQty	 82		/*|	Offset for Allocation quantity.		|*/
#define		offsetLCnvFct	 96		/*|	Offset for conversion factor.		|*/
#define		offsetICnvFct	 108	/*|	Offset for conversion factor.		|*/
#define		offsetHhwhHash	 120	/*|	Offset for Warehouse hash link.		|*/
#define		offsetHhumHash	 131	/*|	Offset for UOM link hash.			|*/
#define		offsetHhccHash	 142	/*|	Offset for Warehouse master hash.	|*/
#define		offsetCreateDate 153	/*|	Offset for Warehouse master hash.	|*/
#define		offsetInloHash   164	/*|	Offset for Inlo_hash             	|*/
#define		offsetLocStatus  175	/*|	Offset for location status       	|*/
#define		offsetPackQty    177	/*|	Offset for package quantity      	|*/
#define		offsetChgWgt     189	/*|	Offset for charge weight         	|*/
#define		offsetGrossWgt   201	/*|	Offset for gross weight             |*/
#define		offsetCuMetre    213	/*|	Offset for cubic metres             |*/
#define		offsetSkndHash   224	/*|	Offset for sknh_hash                |*/
									/*|-------------------------------------|*/
#include	<hot_keys.h>			/*| Include hot key routines.			|*/
#include	<getnum.h>				/*| Include entry routines.  			|*/
#include    <tabdisp.h>				/*| Include tabdisp routines            |*/
#include	<sys/types.h>			/*| System variable types.   			|*/
									/*---------------------------------------*/

#define		LL_LOT_CTRL	(LotControl [0] == 'Y' && SK_BATCH_CONT)

#define		INP_VIEW 		0
#define		INP_AUTO		1
#define		INP_MANUAL		2
#define		INP_NONE 		3


#define		LL_LOAD_INV		1
#define		LL_LOAD_CRD		2
#define		LL_LOAD_SO 		3
#define		LL_LOAD_TRN		4
#define		LL_LOAD_CM 		5
#define		LL_LOAD_MS 		6
#define		LL_LOAD_GL 		7

#define		MAX_SCN_LINE	16
#define		MIN_LL_LINES	4

#define		select_inla_hhcl_id		1
#define		select_inla_itff_id		2
#define		select_inla_cmrd_id		3
#define		select_inla_hhsl_id		4
#define		select_inla_pcms_id		5
#define		select_inla_hhgl_id		6
#define		select_inla_pid			7

#define	TRUE	1
#define	FALSE	0

#include	<std_decs.h>

	static const char	*llct		 = "_location_llct";
	static const char	*inlo		 = "_location_inlo";
	static const char	*itlo		 = "_location_itlo";
	static const char	*lomr		 = "_location_lomr";
	static const char	*inla		 = "_location_inla";
	static const char	*inla2		 = "_location_inla2";
	static const char	*incc		 = "_location_incc";
	static const char	*sknd		 = "_location_sknd";
	static const char	*lotNoPad 	 = "       ";
	static const char	*locationPad = "          ";


	/*
	 * Inventory Location File.
	 */
#define	INLO_NO_FIELDS	25

	static	struct dbview	inlo_list [INLO_NO_FIELDS] =
	{
		{"inlo_inlo_hash"},
		{"inlo_hhwh_hash"},
		{"inlo_sknd_hash"},
		{"inlo_hhum_hash"},
		{"inlo_location"},
		{"inlo_loc_type"},
		{"inlo_loc_status"},
		{"inlo_lot_no"},
		{"inlo_slot_no"},
		{"inlo_expiry_date"},
		{"inlo_pack_qty"},
		{"inlo_chg_wgt"},
		{"inlo_gross_wgt"},
		{"inlo_cu_metre"},
		{"inlo_uom"},
		{"inlo_cnv_fct"},
		{"inlo_rec_qty"},
		{"inlo_qty"},
		{"inlo_stake"},
		{"inlo_no_hits"},
		{"inlo_no_picks"},
		{"inlo_op_id"},
		{"inlo_time_create"},
		{"inlo_date_upd"},
		{"inlo_date_create"}
	};

	struct inloRecLoc
	{
		long	inlo_hash;
		long	hhwh_hash;
		long	sknd_hash;
		long	hhum_hash;
		char	location [11];
		char	loc_type [2];
		char	loc_status [2];
		char	lot_no [8];
		char	slot_no [8];
		Date	expiry_date;
		float	pack_qty;
		float	chg_wgt;
		float	gross_wgt;
		float	cu_metre;
		char	uom [5];
		float	cnv_fct;
		float	rec_qty;
		float	qty;
		float	stake;
		int		no_hits;
		int		no_picks;
		char	op_id [15];
		char	time_create [6];
		Date	date_upd;
		Date	date_create;
	};

	/*
	 * Inventory Allocation Detail File. 
	 */
#define	INLA_NO_FIELDS	11

	static	struct dbview	inla_list [INLA_NO_FIELDS] =
	{
		{"inla_inlo_hash"},
		{"inla_pid"},
		{"inla_line_no"},
		{"inla_hhcl_hash"},
		{"inla_hhsl_hash"},
		{"inla_cmrd_hash"},
		{"inla_itff_hash"},
		{"inla_pcms_hash"},
		{"inla_hhgl_hash"},
		{"inla_qty_alloc"},
		{"inla_qty_proc"}
	};

	typedef	struct
	{
		long	inlo_hash;
		long	pid;
		int		line_no;
		long	hhcl_hash;
		long	hhsl_hash;
		long	cmrd_hash;
		long	itff_hash;
		long	pcms_hash;
		long	hhgl_hash;
		float	qty_alloc;
		float	qty_proc;
	}	INLA_STRUCT;

	/*
	 * Inventory warehouse record.
	 */
#define	INCC_NO_FIELDS	3

	static	struct dbview	incc_list [INCC_NO_FIELDS] =
	{
		{"incc_hhwh_hash"},
		{"incc_location"},
		{"incc_qc_location"},
	};

	struct inccRecLoc
	{
		long	hhwh_hash;
		char	location [11];
		char	qc_location [11];
	};

	/*
	 * Inventory Location File for transfers. 
	 */
#define	ITLO_NO_FIELDS	25

	static	struct dbview	itlo_list [ITLO_NO_FIELDS] =
	{
		{"itlo_itff_hash"},
		{"itlo_pr_line"},
		{"itlo_line_no"},
		{"itlo_hhwh_hash"},
		{"itlo_sknd_hash"},
		{"itlo_hhum_hash"},
		{"itlo_hhcc_hash"},
		{"itlo_inlo_hash"},
		{"itlo_location"},
		{"itlo_loc_type"},
		{"itlo_loc_status"},
		{"itlo_lot_no"},
		{"itlo_slot_no"},
		{"itlo_expiry_date"},
		{"itlo_pack_qty"},
		{"itlo_chg_wgt"},
		{"itlo_gross_wgt"},
		{"itlo_cu_metre"},
		{"itlo_date_create"},
		{"itlo_l_uom"},
		{"itlo_i_uom"},
		{"itlo_qty_avail"},
		{"itlo_qty"},
		{"itlo_l_cnv_fct"},
		{"itlo_i_cnv_fct"}
	};

	struct itloRecLoc
	{
		long	itff_hash;
		int		pr_line;
		int		line_no;
		long	hhwh_hash;
		long	sknd_hash;
		long	hhum_hash;
		long	hhcc_hash;
		long	inlo_hash;
		char	location [11];
		char	loc_type [2];
		char	loc_status [2];
		char	lot_no [8];
		char	slot_no [8];
		Date	expiry_date;
		float	pack_qty;
		float	chg_wgt;
		float	gross_wgt;
		float	cu_metre;
		Date	date_create;
		char	l_uom [5];
		char	i_uom [5];
		float	qty_avail;
		float	qty;
		float	l_cnv_fct;
		float	i_cnv_fct;
	};

	/*
	 * Location Master File. 
	 */
#define	LOMR_NO_FIELDS	14

	static	struct dbview	lomr_list [LOMR_NO_FIELDS] =
	{
		{"lomr_hhcc_hash"},
		{"lomr_location"},
		{"lomr_desc"},
		{"lomr_comm1"},
		{"lomr_comm2"},
		{"lomr_comm3"},
		{"lomr_min_wgt"},
		{"lomr_max_wgt"},
		{"lomr_min_vol"},
		{"lomr_max_vol"},
		{"lomr_no_picks"},
		{"lomr_no_hits"},
		{"lomr_loc_type"},
		{"lomr_access"}
	};

	struct lomrRecLoc
	{
		long	hhcc_hash;
		char	location [11];
		char	desc [41];
		char	comm1 [41];
		char	comm2 [41];
		char	comm3 [41];
		float	min_wgt;
		float	max_wgt;
		float	min_vol;
		float	max_vol;
		float	no_picks;
		float	no_hits;
		char	loc_type [2];
		char	access [2];
	};

	/*
	 * System Lot/location control file. 
	 */
#define	LLCT_NO_FIELDS	25

	static	struct dbview	llct_list [LLCT_NO_FIELDS] =
	{
		{"llct_hhcc_hash"},
		{"llct_nx_3pl_no"},
		{"llct_pick_ord"},
		{"llct_pick_flg1"},
		{"llct_pick_flg2"},
		{"llct_pick_flg3"},
		{"llct_alt_uom"},
		{"llct_auto_all"},
		{"llct_exp_items"},
		{"llct_all_locs"},
		{"llct_val_locs"},
		{"llct_pick_locs"},
		{"llct_rept_locs"},
		{"llct_qc_locs"},
		{"llct_invoice"},
		{"llct_input"},
		{"llct_credit"},
		{"llct_ccn_inp"},
		{"llct_des_conf"},
		{"llct_ades_conf"},
		{"llct_pc_issue"},
		{"llct_only_loc"},
		{"llct_dflt_loc"},
		{"llct_dflt_stat"},
		{"llct_stat_flag"}
	};

	struct llctRecLoc
	{
		long	hhcc_hash;
		long	nx_3pl_no;
		char	pick_ord [4];
		char	pick_flg1 [11];
		char	pick_flg2 [11];
		char	pick_flg3 [11];
		char	alt_uom [2];
		char	auto_all [2];
		char	exp_items [2];
		char	all_locs [2];
		char	val_locs [11];
		char	pick_locs [11];
		char	rept_locs [11];
		char	qc_locs [11];
		char	invoice [2];
		char	input [2];
		char	credit [2];
		char	ccn_inp [2];
		char	des_conf [2];
		char	ades_conf [2];
		char	pc_issue [2];
		char	only_loc [11];
		char	dflt_loc [11];
		char	dflt_stat [2];
		char	stat_flag [2];
	};

	/*
	 * Goods receipts number plate Lines.
	 */
#define	SKND_NO_FIELDS	5

	static	struct dbview	sknd_list [SKND_NO_FIELDS] =
	{
		{"sknd_sknd_hash"},
		{"sknd_hhpl_hash"},
		{"sknd_qty_rec"},
		{"sknd_qty_return"},
		{"sknd_status"},
	};

	struct skndRecLoc
	{
		long	sknd_hash;
		long	hhpl_hash;
		float	qty_rec;
		float	qty_return;
		char	status [2];
	};

	struct lomrRecLoc	lomrRec;
	struct inloRecLoc	inloRec;
	struct inccRecLoc	inccRec;
	struct llctRecLoc	llctRec;
	struct llctRecLoc	llct2Rec;
	struct itloRecLoc	itloRec;
	struct llctRecLoc	llctRec;
	struct skndRecLoc	skndRec;

	INLA_STRUCT	inlaRec;
	INLA_STRUCT	inla2Rec;

	INLA_STRUCT	inlaRecLoc;

char	*GetExpiry 	(int,int),	/* Gets expiry date from tab window.*/
		*GetLUOM 	(int,int),	/* Gets UOM from tab window.		*/
		*GetLoc 	(int,int),	/* Gets location from tab window.	*/
		*GetLotNo 	(int,int),	/* Gets lot no from tab window.		*/
		*GetSLotNo 	(int,int),	/* Gets supp. lot no from tab window*/
		*GetUOM 	(int,int),	/* Gets UOM from tab window.		*/
		*GetLocStat  (int,int),	/* Gets Location Status from tab    */
		*LotCurrUser,			/* Lot Current User.				*/
		*TAB_MASK	=	" %-10.10s | %-10.10s | %1.1s | %-7.7s | %-10.10s | %4.4s | %11.11s | %4.4s | %11.11s | %11.6f %11.6f %010ld %010ld %010ld %010ld %010ld %1.1s %11.2f %11.2f %11.2f %11.2f %010ld",
		*DefaultLocation	(long);

char	DfltLotNo [8],			/* Default lot number.				*/
		LLExpiryDate [11],		/* Holds expiry date.				*/
		LLQtyFormat [15],		/* Quantity Format					*/
		LLQtyMask [10],			/* Quantity Mask					*/
		LLWorkQty [2] [12],
		LotControl [2],			/* Holds lot control flag from item	*/
		PickFlag [3] [31],		/* Picking flag for order rules 	*/
		PickLocation [31],		/* Valid Picking locations			*/
		PickOrderRules [4],		/* Order for picking rules.			*/
		ReceiptLocation [31],	/* Valid receipt locations			*/
		StockTake [2],			/* Stock take variable.				*/
		ValidLocations [31],	/* Valid locations overall			*/
		llctAdesConf [2],
		llctAllLocs [2],
		llctAltUom [2],
		llctAutoAll [2],
		llctCcnInp [2],
		llctCredit [2],
		llctDefault [11],
		llctDesConf [2],
		llctExpItems [2],
		llctInput [2],
		llctInvoice [2],
		llctPcIssue [2];

float	CalcAlloc 		(long,int),
		GetAvalQty 	 	(int,int),	/* Gets Available Qty from tab window.	*/
		GetBaseQty 	 	(int,int),	/* Gets Base quantity from tab window.	*/
		GetCnvFct	 	(int,int),	/* Gets Conversion factor from tab win.	*/
		GetQty 		 	(int,int),	/* Gets Input quantity from tab window.	*/
		GetPackQty 	 	(int,int),	/* Gets Pack Quantity from tab window.	*/
		GetChgWgt	 	(int,int),	/* Gets Charge Weight from tab window.	*/
		GetGrossWgt	 	(int,int),	/* Gets Gross Weight from tab window.	*/
		GetCuMetres	 	(int,int),	/* Gets Cubic Meters from tab window.	*/
		errorQuantity     = 0.00,	/* Quantity unallocated/error			*/
		qtyAllocated = 0.00,		/* Quantity allocated.					*/
		qtyRequired  = 0.00;		/* Quantity required.					*/

int		CheckLocation 	(long, char *, char *),
		DisplayLL 		(int,int,int,int,long,long,long,char *,float,float,long,int,int,char *),
		DspLLStake 		(int,int,int,int,long,long,long,float,char *,long,char *,char *, int),
		DspLLTrans 		(int,int,int,int,long,long,long,char *,float, char *,int),
		FindLocation 	(long, long, char *, char *, long *),
		FindLotNo 		(long, long, char *, long *),
		IgnoreAvailChk	=	FALSE,	/* Ignore available checks.		*/
		IgnoreTotal		=	FALSE,	/* Ignore total quantity checks	*/
		InputExpiry		=	FALSE,	/* Input of expiry fields.		*/
		LLInputClear	=	TRUE,	/* True if lots cleared on entry*/
		LLReturns			= 0,	/* Returns.                  	*/
		LL_EditDate		=	FALSE,	/* True if date can be changed.	*/
		LL_EditLoc		=	FALSE,	/* True if loc can be changed.	*/
		LL_EditLot		=	FALSE,	/* True if lot can be changed.	*/
		LL_EditSLot		=	FALSE,	/* True if slot can be changed.	*/
		LL_Valid 		(int, int),
		LoadLL 	 		(int, int, long, long,long, char *,float,float),
		LoadLocation 	(int, long, long, char *, float, float),
		PoLoad 			(int, long, long, char *, float, float),
		LoadStake 		(int,long,long,long,char *,char *,float,int),
		LoadTrans 		(int, long, long, long, char *, float, int),
		Load_LL_Lines 	(int,int, long,long,char *,float,int),
		MULT_LOC		=	FALSE,	/* Standard SEL loc env var. 	*/
		SK_BATCH_CONT	=	FALSE,	/* Standard SEL batch env var. 	*/
		SK_DFLT_LOC		=	FALSE,
		currentInlaIndex	=	-1,
		noLocationsFound	= 	0,	/* No locations found flag.		*/
		locStatusIgnore		=	FALSE;

static	int		envVarThreePl 			= 0,
				envVarSkGrinNoPlate 	= 0,
				allocationCnt		 	= 0,
				poLoadOpen				= 0;

long	GetHHUM 	(int,int),	/* Gets HHUM HASH from tab window.	*/
		GetHHWH 	(int,int),	/* Gets HHWH HASH from tab window.	*/
		GetINLO 	(int,int),	/* Gets HHLO HASH from tab window.	*/
		GetSKND 	(int,int),	/* Gets SKND_HASH from tab window.	*/
		currentInloHash				= 0L;

pid_t	LotPID;						/* Lot Process ID.				*/

static	char	LocBuff [512];		/* Buffer to hold lines			*/
static	int		InternalCol		=	0,	/* Column passed to program		*/
				no_LL			=	0,	/* No of lines in tab window	*/
				allocationOpen	=	0;

		long	NextThreePlNo 	(long);
static	long	dHash	= 	0L;
static	void	Open_LL_Tab 		(int, int, int),
				TableSetup 			(void),
				TableTearDown 		(void),
				scan_LL_tab			(void),
				AllocationStore		(INLA_STRUCT *);


static	int	make_LL_list	=	0;	/* True if list can be made.	*/

void	AddAlloc 			(long, int, float),
		ClearLotsInfo 		(long),
		DeletePidAllocation (pid_t),
		FreeList 			(void),
		InLotLocation 		(long, long, long, char	*, char	*, char	*, char	*,char *,char *,float, float, char *, float, float, float, float, long),
		LLAddList 			(long,long,long,long,char *,char *,char *,char *,char *,char *,float,char *,float,float,float, long, char *, float, float, float, float,long),
		LoadByDates 		(int,int,long, long, long,char *,float,char *, int, int),
		LoadByLocation 		(int,int, long, long, long,char *,float,char *, int),
		LotClear 			(int),
		LotMove 			(int, int),
		OutLotLocation 		(long, long, long, char	*, char	*, char	*, char	*,char *,char *,float, float, char *, float, float, float, float, long),
		ReadLLCT 			(long),
		SearchLomr 			(long, char	*),
		UpdateLotLocation 	(int, int),
		_UpdateLotLocation 	(int, int, long, long),
		AllocLotLocation 	(int,int,int,long),
		ClearAlloc 			(long, int),
		CloseLocation 		(void),
		LoadManualLL 		(int, long, long, long, char *, float, float),
		NewLL 				(int,long,long,long,char *,float,char *,float,float,long, int),
		OpenLocation		(long),
		PostLotLocation 	(long,long,long,char *,char *,char *,char *,char *,char *,float,float, int, char *, float, float, float, float, long),
	
		PutNoPlateData 		(int, int, char *, float,float,float,float,long),
		ProcList 			(int, float),
		SearchLOC 			(int, long, char *),
		SelectIndex 		(int),
		TransLocation 		(int, long),
		UpdateItlo 			(int,long,long),
		SetOpID 			(int),
		AllocationRestore	(void),
		AllocationComplete	(void);

/*
 * Structure holds all the values required for lot and  
 * locations before they are updated.                  
 */
struct	tag_lot {							/*------------------------------*/
	long	_hhwhHash 	 [MAX_LOTS];		/* hhwh from item incc record.	*/
	long	_hhumHash 	 [MAX_LOTS];		/* hhum from UOM control file.	*/
	long	_hhccHash 	 [MAX_LOTS];		/* hhcc from ccmr master file.	*/
	long	_inloHash 	 [MAX_LOTS];		/* hhlo from Lot/location file.	*/
	char	_loc 		 [MAX_LOTS] [11];	/* Location 					*/
	char	_lot 		 [MAX_LOTS] [8];	/* Lot number.					*/
	char	_loc_type 	 [MAX_LOTS] [2];	/* Location type.				*/
	char	_slot 		 [MAX_LOTS] [8];	/* Supplier lot number.			*/
	char	_date 		 [MAX_LOTS] [11];	/* Date of expiry.				*/
	char	_l_uom 		 [MAX_LOTS] [5];	/* Lot UOM.						*/
	char	_i_uom 		 [MAX_LOTS] [5];	/* Input UOM.					*/
	float	_qty_avail 	 [MAX_LOTS];		/* Quantity available for lot	*/
	float	_qty 		 [MAX_LOTS];		/* Quantity selected / input.	*/
	float	_l_cnv_fct 	 [MAX_LOTS];		/* Lot conversion factor.		*/
	float	_i_cnv_fct 	 [MAX_LOTS];		/* Input conversion factor.		*/
	long	_date_create [MAX_LOTS];		/* Lot creation date.			*/
	char	_locationStat [MAX_LOTS] [2];	/* Location Status				*/
	float	_packQuantity[MAX_LOTS];		/* Package Quantity 			*/
	float	_chargeWeight[MAX_LOTS];		/* Charge Weight				*/
	float	_grossWeight [MAX_LOTS];		/* Gross Weight 				*/
	float	_cubicMetre	 [MAX_LOTS];		/* Cubic Meters 				*/
	long	_skndHash	 [MAX_LOTS];		/* sknd_hash from numberplate	*/
} *LOT;

DArray	lots_d;
	
struct	tag_allocation {	
	long	inlo_hash;
	long	pid;
	int		line_no;
	long	hhcl_hash;
	long	hhsl_hash;
	long	cmrd_hash;
	long	itff_hash;
	long	pcms_hash;
	long	hhgl_hash;
	float	qty_alloc;
	float	qty_proc;
} *ALLOCATION;

DArray	allocation_d;

/*
 * Structure holds all the values required for lot and        
 * locations as a link list to be updated when input complete. 
 */
struct SEL_LIST {		
	long	_hhwhHash;	
	long	_hhumHash; 
	long	_hhccHash; 
	long	_inloHash; 
	char	_loc [11]; 	
	char	_lot [8]; 	
	char	_loc_type [2]; 
	char	_slot [8]; 	
	char	_date [11];
	char	_l_uom [5];
	char	_i_uom [5];
	float	_qty_avail;
	float	_qty; 		
	float	_l_cnv_fct; 
	float	_i_cnv_fct; 
	long	_date_create;
	char	_locationStat [2];
	float	_packQuantity;
	float	_chargeWeight;
	float	_grossWeight;
	float	_cubicMetre;
	long	_skndHash;
	struct SEL_LIST *next;	
};

#define	SEL_NULL ((struct SEL_LIST *) NULL)
struct 	SEL_LIST *selhd_ptr 	= SEL_NULL;
struct 	SEL_LIST *selcur_ptr 	= SEL_NULL;
static	int	LL_Qty 		(int, KEY_TAB *);
static	int	LL_Loc 		(int, KEY_TAB *);
static	int	LL_Date	 	(int, KEY_TAB *);
static	int	LL_Lot 		(int, KEY_TAB *);
static	int	LL_SLot	 	(int, KEY_TAB *);
static	int	abort_func 	(int, KEY_TAB *);
static	int	ok_func 	(int, KEY_TAB *);

/*
 * Tab key function for input of location, batch no, 
 * supplier lot no, expiry date and quantity.		
 */

#ifndef GVISION
static	KEY_TAB LL_keys [] =
{
    { NULL,                 '\r', ok_func,
	" Exit Item Selection ",					"A" },
    { "[L]ocation.",                 'L', LL_Loc,
	" Select Location.",			"L" },
    { "[B]atch/Lot",                 'B', LL_Lot,
	" Select Lot/Batch Number.",			"B" },
    { "[S]upp. Lot",                 'S', LL_SLot,
	" Select Supplier Lot number.",			"S" },
    { "[E]xpiry Date",               'E', LL_Date,
	" Select Expiry Date.",			"E" },
    { "[Q]uantity",                 'Q', LL_Qty,
	" Select Item And Enter Order Quantity ",			"Q" },
    { NULL,                 FN1, abort_func,
	" Abort Item Selection ",					"A" },
    { NULL,                 FN16, ok_func,
	" Exit Item Selection ",					"A" },
    END_KEYS
};
#else
static	KEY_TAB LL_keys [] =
{
    { NULL,                 '\r', ok_func,
	" Exit Item Selection ",					"A" },
    { " Location ",                 'L', LL_Loc,
	" Select Location.",			"L" },
    { " Batch/Lot ",                 'B', LL_Lot,
	" Select Lot/Batch Number.",			"B" },
    { " Suppier Lot ",                 'S', LL_SLot,
	" Select Supplier Lot number.",			"S" },
    { " Expiry Date ",               'E', LL_Date,
	" Select Expiry Date.",			"E" },
    { " Quantity ",                 'Q', LL_Qty,
	" Select Item And Enter Order Quantity ",			"Q" },
    { NULL,                 FN1, abort_func,
	" Abort Item Selection ",					"A" },
    { NULL,                 FN16, ok_func,
	" Exit Item Selection ",					"A" },
    END_KEYS
};
#endif

static	struct SEL_LIST *sel_alloc (void);
 
void
ReadLLCT (
	long hhccHash)
{
	int		err;

	llctRec.hhcc_hash	=	hhccHash;
	if ((err = find_rec (llct, &llctRec, COMPARISON, "r")))
	{
		errmess (ML ("Warning : Warehouse not setup with correct picking information"));
		sleep (10);
		memset (&llctRec, 0, sizeof (llctRec));
		return;
	}
	strcpy (ReceiptLocation, 	clip (llctRec.rept_locs));
	strcpy (PickLocation, 		clip (llctRec.pick_locs));
	strcpy (ValidLocations, 	clip (llctRec.val_locs));
	strcpy (PickOrderRules, 	clip (llctRec.pick_ord));
	strcpy (PickFlag [0], 		clip (llctRec.pick_flg1));
	strcpy (PickFlag [1], 		clip (llctRec.pick_flg2));
	strcpy (PickFlag [2], 		clip (llctRec.pick_flg3));
	strcpy (llctDefault, 		llctRec.dflt_loc);
	strcpy (llctAltUom,			llctRec.alt_uom);
	strcpy (llctAutoAll,		llctRec.auto_all);
	strcpy (llctExpItems,		llctRec.exp_items);
	strcpy (llctAllLocs,		llctRec.all_locs);
	strcpy (llctInvoice,		llctRec.invoice);
	strcpy (llctInput,			llctRec.input);
	strcpy (llctCredit,			llctRec.credit);
	strcpy (llctCcnInp,			llctRec.ccn_inp);
	strcpy (llctDesConf,		llctRec.des_conf);
	strcpy (llctAdesConf ,		llctRec.ades_conf);
	strcpy (llctPcIssue,		llctRec.pc_issue);
}

static void
TableSetup (void)
{
	/*
	 *	Open all the necessary tables et al
	 */
	static int	done_this_before = FALSE;

	if (!done_this_before)
	{
		done_this_before = TRUE;
		abc_alias (inlo,  "inlo");
		abc_alias (lomr,  "lomr");
		abc_alias (inla,  "inla");
		abc_alias (inla2, "inla");
		abc_alias (llct,  "llct");
		abc_alias (itlo,  "itlo");
	}

	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (lomr,  lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	open_rec (inla,  inla_list, INLA_NO_FIELDS, "inla_id_no");
	open_rec (inla2, inla_list, INLA_NO_FIELDS, "inla_pid");
	open_rec (llct,  llct_list, LLCT_NO_FIELDS, "llct_hhcc_hash");
	ArrAlloc (&lots_d, &LOT, sizeof (struct tag_lot), 10);
}

static void
TableTearDown (void)
{
	abc_fclose (llct);
	abc_fclose (inlo);
	abc_fclose (lomr);
	abc_fclose (inla);
	abc_fclose (inla2);
	abc_fclose (llct);
	if (SK_DFLT_LOC)
		abc_fclose (incc);

	if (poLoadOpen)
		abc_fclose (sknd);
	
	ArrDelete (&lots_d);
}

/*
 * Process issues from stock for lots and locations.
 */
void
OutLotLocation
 (
	long	hhwhHash,
	long	hhccHash,
	long	hhumHash,
	char	*UOM,
	char	*lotNumber,
	char	*supplierLotNo,
	char	*LOC,
	char	*locationType,
	char	*expiryDate,
	float	quantity,
	float	conversionFactor,
	char	*locationStat,
	float	packQuantity,
	float	chargeWeight,
	float	grossWeight,
	float	cubicMetre,
	long	skndHash
)
{
	PostLotLocation 
	(
		hhwhHash,
		hhccHash,
		hhumHash,
		UOM,
		lotNumber,
		supplierLotNo,
		LOC,
		locationType,
		expiryDate,
		quantity * -1,
		conversionFactor,
		TRUE,
		locationStat,
		packQuantity,
		chargeWeight,
		grossWeight,
		cubicMetre,
		skndHash
	);
}
/*
 * Process Receipt to stock for lots and locations.
 */
void
InLotLocation (
	long	hhwhHash,
	long	hhccHash,
	long	hhumHash,
	char	*UOM,
	char	*lotNumber,
	char	*supplierLotNo,
	char	*locationNo,
	char	*locationType,
	char	*expiryDate,
	float	quantity,
	float	conversionFactor,
	char	*locationStat,
	float	packQuantity,
	float	chargeWeight,
	float	grossWeight,
	float	cubicMetre,
	long	skndHash)
{
	PostLotLocation 
	(
		hhwhHash,
		hhccHash,
		hhumHash,
		UOM,
		lotNumber,
		supplierLotNo,
		locationNo,
		locationType,
		expiryDate,
		quantity,
		conversionFactor,
		FALSE,
		locationStat,
		packQuantity,
		chargeWeight,
		grossWeight,
		cubicMetre,
		skndHash
	);
}
		
/*
 * Routines processes issues and reciepts from lots and locations.
 */
void
PostLotLocation (
	long	hhwhHash,
	long	hhccHash,
	long	hhumHash,
	char	*UOM,
	char	*lotNumber,
	char	*supplierLotNo,
	char	*locationNo,
	char	*locationType,
	char	*expiryDate,
	float	quantity,
	float	conversionFactor,
	int		OUTBOUND,
	char	*locationStat,
	float	packQuantity,
	float	chargeWeight,
	float	grossWeight,
	float	cubicMetre,
	long	skndHash)
{
	abc_selfield (inlo, "inlo_mst_id");

	inloRec.hhwh_hash	=	hhwhHash;
	inloRec.hhum_hash	=	hhumHash;
	sprintf (inloRec.location, "%-10.10s", locationNo);
	sprintf (inloRec.lot_no,   "%-7.7s",   lotNumber);
	if (envVarThreePl || envVarSkGrinNoPlate)
	{
		if (!strcmp (inloRec.lot_no, "       ") || 
			!strcmp (inloRec.lot_no, "  N/A  "))
		{
			sprintf (inloRec.lot_no, "%07ld", NextThreePlNo (hhccHash));
			strcpy (lotNumber,strdup (inloRec.lot_no));
		}
	}
	cc = find_rec (inlo, &inloRec, COMPARISON, "u");
	if (cc)
	{
		sprintf (inloRec.loc_type, "%-1.1s", locationType);
		sprintf (inloRec.slot_no,  "%-7.7s", supplierLotNo);
		sprintf (inloRec.uom,      "%-4.4s", UOM);
		strcpy (inloRec.loc_status, "A");
		inloRec.expiry_date	=	StringToDate (expiryDate);
		inloRec.cnv_fct		=	conversionFactor;
		inloRec.rec_qty		=	quantity;
		inloRec.rec_qty		=	inloRec.rec_qty;
		inloRec.qty			=	quantity;
		inloRec.qty			=	inloRec.qty;
		inloRec.no_hits		=	0;
		inloRec.no_picks	=	0;
		inloRec.stake		=	0.00;
		strcpy (inloRec.loc_status, locationStat);
		inloRec.pack_qty	=	packQuantity;
		inloRec.chg_wgt		=	chargeWeight;
		inloRec.gross_wgt	=	grossWeight;
		inloRec.cu_metre	=	cubicMetre;
		inloRec.sknd_hash	=	skndHash;
		SetOpID (TRUE);
		
		if (!CheckLocation (hhccHash, locationNo, lomrRec.loc_type))
			strcpy (inloRec.loc_type,lomrRec.loc_type);

		abc_unlock (inlo);

		cc = abc_add (inlo,&inloRec);
		if (cc)
			file_err (cc, inlo, "DBADD");

		inloRec.hhwh_hash	=	hhwhHash;
		inloRec.hhum_hash	=	hhumHash;
		sprintf (inloRec.location, "%-10.10s", locationNo);
		sprintf (inloRec.lot_no,   "%-7.7s",   lotNumber);
		cc = find_rec (inlo, &inloRec, COMPARISON, "r");
	}
	else
	{
		inloRec.qty		+=	quantity;
		if (OUTBOUND)
			inloRec.no_picks++;

		inloRec.no_hits++;
		SetOpID (FALSE);
		
		if (!CheckLocation (hhccHash, locationNo, lomrRec.loc_type))
			strcpy (inloRec.loc_type,lomrRec.loc_type);

		cc = abc_update (inlo,&inloRec);
		if (cc)
			file_err (cc, inlo, "DBADD");
	}
	currentInloHash	= inloRec.inlo_hash;
}
	
/*
 * Check Location id a valid location. 
 */
int
CheckLocation
 (
	long	hhccHash,
	char	*Loc,
	char	*locType)
{

	lomrRec.hhcc_hash 	= 	hhccHash;
	if (Loc == (char *)0)
	{
		strcpy (lomrRec.location, locationPad);
		cc = find_rec (lomr, &lomrRec, GTEQ , "r");
		if (cc)
			return (EXIT_FAILURE);
		
		strcpy (locType,strdup (lomrRec.loc_type));
		return (EXIT_SUCCESS);
	}
	sprintf (lomrRec.location,"%-10.10s",Loc);
	cc = find_rec (lomr, &lomrRec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);
	
	strcpy (locType,strdup (lomrRec.loc_type));
	return (EXIT_SUCCESS);
}
/*
 * Find Stock location
 */
int
FindLocation
 (
	long	hhwhHash,
	long	hhumHash,
	char	*locationNo,
	char	*setupFlags,
	long	*inloHash)
{
	int		i;

	abc_selfield (inlo, "inlo_id_loc");

	for (i = 0; i < (int) strlen (setupFlags); i++)
	{
		inloRec.hhwh_hash 		= 	hhwhHash;
		inloRec.loc_type [0]	= 	setupFlags [i];
		sprintf (inloRec.location,"%-10.10s", 
						(locationNo	== (char *)0) ? " " : locationNo);
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		while (!cc && inloRec.hhwh_hash == hhwhHash &&
					  inloRec.loc_type [0] == setupFlags [i])
		{
			if ((locationNo != (char *)0) && 
				strcmp (locationNo, inloRec.location))
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (!LOC_AVAILABLE)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (inloRec.hhum_hash	==	hhumHash || hhumHash == 0L)
			{
				*inloHash	=	inloRec.inlo_hash;
				if ((locationNo != (char *)0))
					strcpy (locationNo,strdup (inloRec.location));
				return (EXIT_SUCCESS);
			}
			
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
	}
	*inloHash	=	0L;
	return (EXIT_FAILURE);
}
/*
 * Find Stock location
 */
int
FindLotNo
 (
	long	hhwhHash,
	long	hhumHash,
	char	*LotNo,
	long	*inloHash)
{
	int		i;

	abc_selfield (inlo, "inlo_id_lot");

	for (i = 0; i < (int) strlen (ValidLocations); i++)
	{
		inloRec.hhwh_hash 		= 	hhwhHash;
		inloRec.loc_type [0]	= 	ValidLocations [i];
		sprintf (inloRec.lot_no,"%-7.7s", (LotNo == (char *)0) ? " " : LotNo);
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		while (!cc && inloRec.hhwh_hash == hhwhHash &&
					  inloRec.loc_type [0] == ValidLocations [i])
		{
			if ((LotNo != (char *)0) && strcmp (LotNo, inloRec.lot_no))
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (inloRec.hhum_hash	==	hhumHash || hhumHash == 0L)
			{
				if (LotNo != (char *) 0)
					strcpy (LotNo,strdup (inloRec.lot_no));
				*inloHash	=	inloRec.inlo_hash;
				return (EXIT_SUCCESS);
			}
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
	}
	return (EXIT_FAILURE);
}
		
/*
 * Open location files and read environments.
 */
void
OpenLocation
(
	long	hhccHash)
{
	char	*sptr;
	int		internalBefore,
			internalAfter;

	TableSetup ();

	ReadLLCT (hhccHash);

	strcpy (StockTake, "N");
	strcpy (DfltLotNo, lotNoPad);

	sptr = chk_env ("SK_BATCH_CONT");
	SK_BATCH_CONT 	= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	/*
	 * Check for Number Plate System.
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envVarSkGrinNoPlate = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (envVarSkGrinNoPlate)
	{
		strcpy (DfltLotNo, "  N/A  ");
		SK_BATCH_CONT 	= TRUE;
	}
	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarThreePl = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_DFLT_LOC");
	SK_DFLT_LOC 	= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("MULT_LOC");
	MULT_LOC 		= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_LOC_IGN_TOT");
	IgnoreAvailChk 	= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (LLQtyFormat, "NNNNNNNN.NN");
	else
		strcpy (LLQtyFormat, sptr);

	internalBefore = strlen (LLQtyFormat);
	sptr = strrchr (LLQtyFormat, '.');
	if (sptr)
		internalAfter = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		internalAfter = 0;

	if (internalAfter == 0)
		sprintf (LLQtyMask, "%%%df", internalBefore);
	else
		sprintf (LLQtyMask, "%%%d.%df", internalBefore, internalAfter);


	if (SK_DFLT_LOC)
	{
		abc_alias (incc, "incc");
		open_rec (incc, incc_list,INCC_NO_FIELDS,"incc_hhwh_hash");
	}

	LotPID		=	getpid ();
	LotCurrUser = getenv ("LOGNAME");

	currentInlaIndex		=	-1;
}

/*
 * Close location files.
 */
void
CloseLocation (void)
{
	DeletePidAllocation (LotPID);

	TableTearDown ();
	if (SK_DFLT_LOC)
		abc_fclose (incc);
}

/*
 * Search for Location master.
 */
void
SearchLomr
 (
	long	hhccHash,
	char	*keyValue)
{
	work_open ();

	lomrRec.hhcc_hash 	= 	hhccHash;
	sprintf (lomrRec.location,"%-10.10s", keyValue);
	
	cc = save_rec ("# Location ","# Location Description.");

	cc = find_rec (lomr,&lomrRec,GTEQ,"r");
	while (!cc && lomrRec.hhcc_hash == hhccHash &&
				!strncmp (lomrRec.location,keyValue,strlen (keyValue)))
	{
		cc = save_rec (lomrRec.location, lomrRec.desc);
		if (cc)
			break;

		cc = find_rec (lomr, &lomrRec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	lomrRec.hhcc_hash 	= 	hhccHash;
	sprintf (lomrRec.location,"%-10.10s", temp_str);
	cc = find_rec (lomr, &lomrRec, GTEQ,"r");
	if (cc)
		file_err (cc, lomr, "DBFIND");

	strcpy (keyValue, strdup (lomrRec.location));
}

/*
 * Search for valid Location. 
 */
void
SearchLOC
 (
	int		locLot,
 	long	hhwhHash,
	char	*keyValue)
{
	char	DispUom [100];
	float	AllocQty;

	abc_selfield (inlo, (locLot) ? "inlo_id_loc" : "inlo_id_lot");
	_work_open (25, (locLot) ? 10 : 7,20);
	inloRec.hhwh_hash 	= 	hhwhHash;
	strcpy (inloRec.loc_type, " ");
	sprintf (inloRec.location,"%-10.10s", locationPad);
	if (locLot)
		cc = save_rec ("#  LOCATION  - LOT NO. ","# UOM. |TYPE| QUANTITY ");
	else
		cc = save_rec ("# LOT NO. -  LOCATION  ","# UOM. |TYPE| QUANTITY ");

	cc = find_rec (inlo, &inloRec, GTEQ,"r");
	while (!cc && (inloRec.hhwh_hash == hhwhHash || hhwhHash == 0L))
	{
		if (locLot)
		{
			if (strncmp (inloRec.location,keyValue,strlen (keyValue)))
			{
				cc = find_rec (inlo, &inloRec, NEXT,"r");
				continue;		
			}
			if (!LOC_AVAILABLE)
			{
				cc = find_rec (inlo, &inloRec, NEXT,"r");
				continue;		
			}
		}
		else
		{
			if (strncmp (inloRec.lot_no,keyValue,strlen (keyValue)))
			{
				cc = find_rec (inlo, &inloRec, NEXT,"r");
				continue;		
			}
		}
		if (inloRec.cnv_fct == 0.00)
			inloRec.cnv_fct	=	1.00;

		if (locLot)
		{
			sprintf 
			(
				DispUom, 
				"%-10.10s %-7.7s %010ld %s",
				inloRec.location,
				inloRec.lot_no,
				inloRec.hhum_hash,
				inloRec.loc_type
			);
		}
		else
		{
			sprintf 
			(
				DispUom, 
				"%-7.7s %-10.10s %010ld %s",
				inloRec.lot_no,
				inloRec.location,
				inloRec.hhum_hash,
				inloRec.loc_type
			);
		}
		AllocQty	=	inloRec.qty - CalcAlloc (inloRec.inlo_hash,line_cnt);

		sprintf (LLWorkQty [0], LLQtyMask, AllocQty / inloRec.cnv_fct);
		sprintf 
		(
			err_str,
			" %s | %s  |%11.11s",	
			inloRec.uom, 
			inloRec.loc_type,
			LLWorkQty [0]
		);
		cc = save_rec (DispUom,err_str);
		if (cc)
			break;

		cc = find_rec (inlo,&inloRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	abc_selfield (inlo, "inlo_mst_id");

	inloRec.hhwh_hash 	= 	hhwhHash;
	inloRec.hhum_hash	=	atol (temp_str + 20);
	sprintf (inloRec.location, "%-10.10s",(locLot) ? temp_str : temp_str + 8);
	sprintf (inloRec.lot_no,   "%-7.7s",  (locLot) ? temp_str + 11 : temp_str);
	sprintf (inloRec.loc_type, "%-1.1s", temp_str + 30);

	cc = find_rec (inlo, &inloRec, COMPARISON, "r");
	if (cc)
		file_err (cc, inlo, "DBFIND");
}

/*
 * Set information about change date etc. 
 */
void
SetOpID (
	int		NewRecord)
{
	sprintf (inloRec.op_id, "%-14.14s", LotCurrUser);
	strcpy (inloRec.time_create, TimeHHMM ());

	if (NewRecord)
		inloRec.date_create = TodaysDate ();

	inloRec.date_upd = TodaysDate ();
}

/*
 * Process new lots and locations. 
 */
void
NewLL
(
	int		lineCnt,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*L_UOM,
	float	L_conversionFactor,
	char	*I_UOM,
	float	I_conversionFactor,
	float	quantity,
	long	expiryDate,
	int		_Silent
)
{
	int		i;

	qtyAllocated	=	quantity;
	qtyRequired		=	quantity;

	if (LOT [lineCnt]._hhwhHash [0] != hhwhHash || 
		(prog_status == ENTRY && LLInputClear))
		ClearLotsInfo (lineCnt);

	if (LOT [lineCnt]._hhwhHash [0] == 0L)
	{
		cc = FindLocation (hhwhHash, hhumHash, NULL, ReceiptLocation, &dHash);
		if (cc)	
		{
			strcpy (inloRec.location, DefaultLocation (hhwhHash));
			inloRec.loc_type [0]	=	ReceiptLocation [0];
		}
		if (_Silent)
		{
			strcpy (LLExpiryDate, DateToString (expiryDate));
			LLAddList
			(
				hhwhHash,
				hhumHash,
				hhccHash,
				0L,
				inloRec.location,
				(LL_LOT_CTRL) ? DfltLotNo : "  N/A  ",
				inloRec.loc_type,
				(LL_LOT_CTRL) ? DfltLotNo : "  N/A  ",
				LLExpiryDate,
				L_UOM,
				0.00,
				I_UOM,
				quantity,
				L_conversionFactor,
				I_conversionFactor,
				TodaysDate (),
				inloRec.loc_status,
				inloRec.pack_qty,
				inloRec.chg_wgt,
				inloRec.gross_wgt,
				inloRec.cu_metre,
				inloRec.sknd_hash
			);
		}
		else
		{
			strcpy (LLExpiryDate, DateToString (expiryDate));
			sprintf (LLWorkQty [0], LLQtyMask, 0.00);
			sprintf (LLWorkQty [1], LLQtyMask, quantity);
			tab_add
			(
				"LL_lst", TAB_MASK,
				inloRec.location,
				(LL_LOT_CTRL) ? DfltLotNo : "  N/A  ",
				inloRec.loc_type,
				(LL_LOT_CTRL) ? DfltLotNo : "  N/A  ",
				LLExpiryDate,
				L_UOM,
				LLWorkQty [0],
				I_UOM,
				LLWorkQty [1],
				L_conversionFactor,
				I_conversionFactor,
				hhwhHash,
				hhumHash,
				hhccHash,
				TodaysDate (),
				0L,
				inloRec.loc_status,
				inloRec.pack_qty,
				inloRec.chg_wgt,
				inloRec.gross_wgt,
				inloRec.cu_metre,
				inloRec.sknd_hash
			);
			no_LL++;
		}
	}
	else
	{
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (LOT [lineCnt]._hhwhHash [i] == 0L)
				break;

			if (_Silent)
			{
				LLAddList
				(
					LOT [lineCnt]._hhwhHash 	[i],
					LOT [lineCnt]._hhumHash 	[i],
					LOT [lineCnt]._hhccHash 	[i],
					LOT [lineCnt]._inloHash 	[i],
					LOT [lineCnt]._loc 			[i],
					LOT [lineCnt]._lot 			[i],
					LOT [lineCnt]._loc_type 	[i],
					LOT [lineCnt]._slot 		[i],
					LOT [lineCnt]._date 		[i],
					LOT [lineCnt]._l_uom 		[i],
					LOT [lineCnt]._qty_avail 	[i],
					LOT [lineCnt]._i_uom 		[i],
					LOT [lineCnt]._qty 			[i],
					LOT [lineCnt]._l_cnv_fct 	[i],
					LOT [lineCnt]._i_cnv_fct 	[i],
					LOT [lineCnt]._date_create 	[i],
					LOT [lineCnt]._locationStat [i],
					LOT [lineCnt]._packQuantity	[i],
					LOT [lineCnt]._chargeWeight	[i],
					LOT [lineCnt]._grossWeight 	[i],
					LOT [lineCnt]._cubicMetre	[i],
					LOT [lineCnt]._skndHash		[i]
				);
			}
			else
			{
				sprintf (LLWorkQty [0], LLQtyMask,LOT [lineCnt]._qty_avail [i]);
				sprintf (LLWorkQty [1], LLQtyMask,LOT [lineCnt]._qty [i]);
				tab_add
				(
					"LL_lst", TAB_MASK,
					LOT [lineCnt]._loc 			[i],
					LOT [lineCnt]._lot 			[i],
					LOT [lineCnt]._loc_type 	[i],
					LOT [lineCnt]._slot 		[i],
					LOT [lineCnt]._date 		[i],
					LOT [lineCnt]._l_uom 		[i],
					LLWorkQty 					[0],
					LOT [lineCnt]._i_uom 		[i],
					LLWorkQty 					[1],
					LOT [lineCnt]._l_cnv_fct 	[i],
					LOT [lineCnt]._i_cnv_fct 	[i],
					LOT [lineCnt]._hhwhHash 	[i],
					LOT [lineCnt]._hhumHash 	[i],
					LOT [lineCnt]._hhccHash 	[i],
					LOT [lineCnt]._date_create 	[i],
					LOT [lineCnt]._inloHash 	[i],
					LOT [lineCnt]._locationStat [i],
					LOT [lineCnt]._packQuantity	[i],
					LOT [lineCnt]._chargeWeight	[i],
					LOT [lineCnt]._grossWeight 	[i],
					LOT [lineCnt]._cubicMetre	[i],
					LOT [lineCnt]._skndHash		[i]
				);
				no_LL++;
			}
		}
	}
}

/*
 * Clear information from lot/location file.
 */
void
ClearLotsInfo (
	long	lineCnt)
{
	if (!ArrChkLimit (&lots_d, LOT, lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	memset ((char *) &LOT [lineCnt], 0, sizeof (struct tag_lot));
}

/*
 * Manually load information into lots. 
 */
void
LoadManualLL
(
	int		lineCnt,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	float	quantity
)
{
	int		OneLocation	=	FALSE;
	int		i;
	int		NoRecordFound	=	TRUE;
	float	processQuantity	=	0.00;
	qtyRequired	=	quantity;

	if (LOT [lineCnt]._hhwhHash [0] != hhwhHash || 
		(prog_status == ENTRY && LLInputClear))
		ClearLotsInfo (lineCnt);

	if (LOT [lineCnt]._hhwhHash [0] == 0L)
	{
		abc_selfield (inlo, "inlo_id_fifo");
	
		for (i = 0; i < (int) strlen (PickLocation); i++)
		{
			inloRec.hhwh_hash		=	hhwhHash;
			inloRec.loc_type [0]	=	PickLocation [i];
			inloRec.expiry_date	=	0L;
			inloRec.date_create	=	0L;
			strcpy (inloRec.location, 	locationPad);
			cc = find_rec (inlo, &inloRec, GTEQ, "r");
			while (!cc && inloRec.hhwh_hash == hhwhHash && 
					  	  inloRec.loc_type [0] == PickLocation [i])
			{
				if ((inloRec.date_create + 90L < TodaysDate ()) &&
				 	inloRec.qty == 0.00) 
				{
					cc = find_rec (inlo, &inloRec, NEXT, "r");
					continue;
				}
				if (!LOC_AVAILABLE)
				{
					cc = find_rec (inlo, &inloRec, NEXT, "r");
					continue;
				}
				NoRecordFound	=	FALSE;

				processQuantity		=	0.00;

				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				sprintf (LLWorkQty[0],LLQtyMask, inloRec.qty / inloRec.cnv_fct);
				sprintf (LLWorkQty[1],LLQtyMask, processQuantity);
				tab_add
				(
					"LL_lst", 
					TAB_MASK,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.date_create,
					inloRec.inlo_hash,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
				no_LL++;
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				sprintf (LLWorkQty [0], LLQtyMask, 0.00);
				sprintf (LLWorkQty [1], LLQtyMask, processQuantity);
				tab_add
				(
					"LL_lst", 
					TAB_MASK,
					" ",
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					UOM,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					conversionFactor,
					conversionFactor,
					inloRec.hhwh_hash,
					hhumHash,
					hhccHash,
					inloRec.date_create,
					0L,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
				no_LL++;
				cc = find_rec (inlo, &inloRec, NEXT, "r");
			}
		}
		if (NoRecordFound)
		{
			if (strcmp (llctRec.only_loc, locationPad))
				OneLocation	=	TRUE;
				
			strcpy (LLExpiryDate, DateToString (0L));
			sprintf (LLWorkQty [0], LLQtyMask, 0.00);
			sprintf (LLWorkQty [1], LLQtyMask, quantity);

			tab_add
			(
				"LL_lst", 
				TAB_MASK,
				(OneLocation) ? llctRec.only_loc : DefaultLocation (hhwhHash),
				(LL_LOT_CTRL || envVarSkGrinNoPlate) ? " ToFix " : "  N/A  ",
				"P",
				(LL_LOT_CTRL || envVarSkGrinNoPlate) ? " ToFix " : "  N/A  ",
				LLExpiryDate,
				UOM,
				LLWorkQty [0],
				UOM,
				LLWorkQty [1],
				conversionFactor,
				conversionFactor,
				hhwhHash,
				hhumHash,
				hhccHash,
				TodaysDate (),
				0L,
				"A",
				0.00,
				0.00,
				0.00,
				0.00,
				0L
			);
			no_LL++;
		}
	}
	else
	{
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (LOT [lineCnt]._hhwhHash [i] == 0L)
				break;

			sprintf (LLWorkQty [0], LLQtyMask, LOT [lineCnt]._qty_avail [i]);
			sprintf (LLWorkQty [1], LLQtyMask, LOT [lineCnt]._qty [i]);
			tab_add
			(
				"LL_lst", 
				TAB_MASK,
				LOT [lineCnt]._loc 			[i],
				LOT [lineCnt]._lot 			[i],
				LOT [lineCnt]._loc_type 	[i],
				LOT [lineCnt]._slot 		[i],
				LOT [lineCnt]._date 		[i],
				LOT [lineCnt]._l_uom 		[i],
				LLWorkQty 					[0],
				LOT [lineCnt]._i_uom 		[i],
				LLWorkQty 					[1],
				LOT [lineCnt]._l_cnv_fct 	[i],
				LOT [lineCnt]._i_cnv_fct 	[i],
				LOT [lineCnt]._hhwhHash 	[i],
				LOT [lineCnt]._hhumHash 	[i],
				LOT [lineCnt]._hhccHash 	[i],
				LOT [lineCnt]._date_create 	[i],
				LOT [lineCnt]._inloHash 	[i],
				LOT [lineCnt]._locationStat [i],
				LOT [lineCnt]._packQuantity	[i],
				LOT [lineCnt]._chargeWeight	[i],
				LOT [lineCnt]._grossWeight 	[i],
				LOT [lineCnt]._cubicMetre	[i],
				LOT [lineCnt]._skndHash	    [i]
			);
			no_LL++;
		}
	}
}
/*
 * Load stock take information. 
 */
int
LoadStake
 (
	int		lineCnt,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*Location,
	char	*UOM,
	float 	conversionFactor,
	int		_Silent
)
{
	int		i;
	int		ByLocation	=	TRUE;

	qtyAllocated	=	0.00;
	qtyRequired	=	0.00;

	if (LOT [lineCnt]._hhwhHash [0] != hhwhHash || 
		(prog_status == ENTRY && LLInputClear))
		ClearLotsInfo (lineCnt);

	if (LOT [lineCnt]._hhwhHash [0] == 0L)
	{
		if (Location	== (char *) 0)
			ByLocation	=	FALSE;

		abc_selfield (inlo, "inlo_mst_id");

		inloRec.hhwh_hash		=	hhwhHash;
		inloRec.hhum_hash		=	hhumHash;
		strcpy (inloRec.location, 	locationPad);
		strcpy (inloRec.lot_no, 	lotNoPad);
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		while (!cc && inloRec.hhwh_hash == hhwhHash)
		{
			if (ByLocation && strcmp (inloRec.location, Location))
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (inloRec.hhum_hash != hhumHash && hhumHash != 0L)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (!LOC_AVAILABLE)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			if (_Silent)
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				LLAddList
				(
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.inlo_hash,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					inloRec.qty / inloRec.cnv_fct,
					UOM,
					inloRec.qty / inloRec.cnv_fct,
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.date_create,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			else
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				sprintf (LLWorkQty [0], LLQtyMask, inloRec.qty / inloRec.cnv_fct);
				sprintf (LLWorkQty [1], LLQtyMask, inloRec.qty / inloRec.cnv_fct);
				tab_add
				(
					"LL_lst", TAB_MASK,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.date_create,
					inloRec.inlo_hash,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			qtyAllocated	+=	inloRec.qty / inloRec.cnv_fct;
			qtyRequired	+=	inloRec.qty / inloRec.cnv_fct;
			no_LL++;
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
	}
	else
	{
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (LOT [lineCnt]._hhwhHash [i] == 0L)
				break;

			sprintf (LLWorkQty [0], LLQtyMask, LOT [lineCnt]._qty_avail [i]);
			sprintf (LLWorkQty [1], LLQtyMask, LOT [lineCnt]._qty [i]);
			tab_add
			(
				"LL_lst", TAB_MASK,
				LOT [lineCnt]._loc 			[i],
				LOT [lineCnt]._lot 			[i],
				LOT [lineCnt]._loc_type 	[i],
				LOT [lineCnt]._slot 		[i],
				LOT [lineCnt]._date 		[i],
				LOT [lineCnt]._l_uom 		[i],
				LLWorkQty 					[0],
				LOT [lineCnt]._i_uom 		[i],
				LLWorkQty 					[1],
				LOT [lineCnt]._l_cnv_fct 	[i],
				LOT [lineCnt]._i_cnv_fct 	[i],
				LOT [lineCnt]._hhwhHash 	[i],
				LOT [lineCnt]._hhumHash 	[i],
				LOT [lineCnt]._hhccHash 	[i],
				LOT [lineCnt]._date_create 	[i],
				LOT [lineCnt]._inloHash 	[i],
				LOT [lineCnt]._locationStat [i],
				LOT [lineCnt]._packQuantity	[i],
				LOT [lineCnt]._chargeWeight	[i],
				LOT [lineCnt]._grossWeight 	[i],
				LOT [lineCnt]._cubicMetre	[i],
				LOT [lineCnt]._skndHash		[i]
			);
			no_LL++;
		}
	}
	return (!no_LL);
}
/*
 * Main processing routine for getting    
 * lot/locations based on picking order (s)
 */
int
LoadLL (
	int		lineCnt,
	int		_Silent,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	float	quantity)
{
	int		i;

	noLocationsFound	=	0;

	qtyAllocated	=	quantity;
	qtyRequired		=	quantity;
	errorQuantity	=	0.00;

	/*
	 * First process using picking order rules as defined on llct file.
	 */
	for (i = 0; i < (int) strlen (PickOrderRules); i++)
	{
		if (qtyAllocated <= 0.00 && !STOCK_TAKE)
			break;

		/*
		 * Pick by location.
		 */
		if (PickOrderRules [i] == 'L')
		{
			LoadByLocation 	
			(
				lineCnt,
				_Silent,
				hhwhHash, 
				hhumHash,
				hhccHash,
				UOM,
				conversionFactor,
				PickFlag [i],
				TRUE
			);
		}
		
		/*
		 * Pick by expiry date.
		 */
		if (PickOrderRules [i] == 'E')
		{
			LoadByDates 	
			(
				lineCnt,
				_Silent,
				hhwhHash, 
				hhumHash,
				hhccHash,
				UOM,
				conversionFactor,
				PickFlag [i],
				TRUE,
				TRUE
			);
		}
		/*
		 * Pick by FIFO date.
		 */
		if (PickOrderRules [i] == 'F')
		{
			LoadByDates 	
			(
				lineCnt,
				_Silent,
				hhwhHash, 
				hhumHash,
				hhccHash,
				UOM,
				conversionFactor,
				PickFlag [i],
				FALSE,
				TRUE
			);
		}
	}
	/*
	 * Now pick using alternate UOM as not enough stock available.
	 */
	if (ALT_UOM)
	{
		for (i = 0; i < (int) strlen (PickOrderRules); i++)
		{
			if (qtyAllocated <= 0.00 && !STOCK_TAKE)
				break;

			/*
			 * Pick by location. 
			 */
			if (PickOrderRules [i] == 'L')
				LoadByLocation 	
				(
					lineCnt,
					_Silent,
					hhwhHash, 
					hhumHash,
					hhccHash,
					UOM,
					conversionFactor,
					PickFlag [i],
					FALSE
				);
			/*
			 * Pick by expiry date.
			 */
			if (PickOrderRules [i] == 'E')
				LoadByDates 	
				(
					lineCnt,
					_Silent,
					hhwhHash, 
					hhumHash,
					hhccHash,
					UOM,
					conversionFactor,
					PickFlag [i],
					TRUE,
					FALSE
			);
			/*
			 * Pick by FIFO date.
			 */
			if (PickOrderRules [i] == 'F')
				LoadByDates 	
				(
					lineCnt,
					_Silent,
					hhwhHash, 
					hhumHash,
					hhccHash,
					UOM,
					conversionFactor,
					PickFlag [i],
					FALSE,
					FALSE
				);
		}
	}
	errorQuantity	=	qtyAllocated;
	return (noLocationsFound);
}

/*
 * Load by location. Process lot/locations by pick type 
 * defines in lot/location control file.               
 */
void
LoadByLocation (
	int		lineCnt,
	int		_Silent,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	char	*setupFlags,
	int		FirstPass)
{
	int		i;
	float	processQuantity	=	0.00;

	abc_selfield (inlo, "inlo_id_loc");

	for (i = 0; i < (int) strlen (setupFlags); i++)
	{
		inloRec.hhwh_hash		=	hhwhHash;
		inloRec.loc_type [0]	=	setupFlags [i];
		strcpy (inloRec.location, 	locationPad);
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		while (!cc && inloRec.hhwh_hash == hhwhHash &&
					  inloRec.loc_type [0] == setupFlags [i])
		{
			if (!LOC_AVAILABLE)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			inloRec.qty = inloRec.qty - 
						   CalcAlloc (inloRec.inlo_hash,lineCnt);

			/*
			 * No more quantity to allocate. 
			 */
			if (qtyAllocated <= 0.00 && !STOCK_TAKE)
				break;

			/* 
			 * Don't display lot/location if DSP_ALL not set and < zero. 
			 */
			if (!DSP_ALL && inloRec.qty <= 0.00)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			/*
			 * If first pass only allow same unit of measures.
			 */
			if (FirstPass && inloRec.hhum_hash != hhumHash)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			/*
			 * If Second pass look for alternate unit of measures.
			 */
			if (!FirstPass && inloRec.hhum_hash == hhumHash)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}

			/*
			 * Ignore expired items if not specifically allowed.
			 */
			if (!EXP_ITEMS && inloRec.expiry_date != 0L && 
				inloRec.expiry_date < TodaysDate ())
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}

			processQuantity		=	0.00;
			if (LLReturns)
			{
				processQuantity		=	qtyAllocated;
				qtyAllocated 	= 	0.00;
			}
			else
			{
				/*
				 * Process amount required ver amount available for lot/loc.
				 */
				if (inloRec.qty > 0.00)
				{
					if ((qtyAllocated * conversionFactor) > inloRec.qty)
					{
						processQuantity	=	inloRec.qty / conversionFactor;
						qtyAllocated -= inloRec.qty / conversionFactor;
					}
					else
					{
						processQuantity		=	qtyAllocated;
						qtyAllocated 	= 	0.00;
					}
				}
			}
			noLocationsFound++;
			/*
			 * Silent mode is used for automatic confirmation.
			 */
			if (_Silent)
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				LLAddList
				(
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.inlo_hash,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					inloRec.qty / inloRec.cnv_fct,
					UOM,
					processQuantity,
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.date_create,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			else
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				sprintf (LLWorkQty [0], LLQtyMask, inloRec.qty / inloRec.cnv_fct);
				sprintf (LLWorkQty [1], LLQtyMask, processQuantity);
				tab_add
				(
					"LL_lst", TAB_MASK,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.date_create,
					inloRec.inlo_hash,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
				no_LL++;
			}
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
	}
}
/*
 * Load by Dates. Process lot/locations by pick type    
 * defines in lot/location control file.               
 */
void
LoadByDates
 (
	int		lineCnt,
	int		_Silent,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	char	*setupFlags,
	int		expiryDate,
	int		FirstPass
)
{
	int		i;
	float	processQuantity	=	0.00;

	if (expiryDate)
		abc_selfield (inlo, "inlo_id_exp");
	else
		abc_selfield (inlo, "inlo_id_fifo");

	for (i = 0; i < (int) strlen (PickLocation); i++)
	{
		inloRec.hhwh_hash		=	hhwhHash;
		inloRec.loc_type [0]	=	setupFlags [i];
		inloRec.expiry_date	=	0L;
		inloRec.date_create	=	0L;
		strcpy (inloRec.location, 	locationPad);
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		while (!cc && inloRec.hhwh_hash == hhwhHash && 
					  inloRec.loc_type [0] == setupFlags [i])
		{
			if (!LOC_AVAILABLE)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			inloRec.qty = inloRec.qty - 
						   CalcAlloc (inloRec.inlo_hash,lineCnt);

			/*
			 * No more quantity to allocate. 
			 */
			if (qtyAllocated <= 0.00 && !STOCK_TAKE)
				break;

			/*
			 * Don't display lot/location if DSP_ALL not set and < zero. 
			 */
			if (!DSP_ALL && inloRec.qty <= 0.00)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			/*
			 * If first pass only allow same unit of measures.
			 */
			if (FirstPass && inloRec.hhum_hash != hhumHash)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}
			/*
			 * If Second pass look for alternate unit of measures.
			 */
		  	if (!FirstPass && inloRec.hhum_hash == hhumHash)
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}

			/*
			 * Ignore expired items if not specifically allowed.
			 */
			if (!EXP_ITEMS && inloRec.expiry_date != 0L && 
				inloRec.expiry_date < TodaysDate ())
			{
				cc = find_rec (inlo, &inloRec, NEXT, "r");
				continue;
			}

			processQuantity		=	0.00;
			if (LLReturns)
			{
				processQuantity		=	qtyAllocated;
				qtyAllocated 	= 	0.00;
			}
			else
			{
				/*
				 * Process amount required ver amount available for lot/loc.
				 */
				if (inloRec.qty > 0.00)
				{
					if ((qtyAllocated * conversionFactor) > inloRec.qty)
					{
						processQuantity	=	inloRec.qty / conversionFactor;
						qtyAllocated -= inloRec.qty / conversionFactor;
					}
					else
					{
						processQuantity		=	qtyAllocated;
						qtyAllocated 	= 	0.00;
					}
				}
			}
			noLocationsFound++;
			/*
			 * Silent mode is used for automatic confirmation. 
			 */
			if (_Silent)
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				LLAddList
				(
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.inlo_hash,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					inloRec.qty / inloRec.cnv_fct,
					UOM,
					processQuantity,
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.date_create,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			else
			{
				strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
				sprintf (LLWorkQty [0], LLQtyMask, inloRec.qty / inloRec.cnv_fct);
				sprintf (LLWorkQty [1], LLQtyMask, processQuantity);
				tab_add
				(
					"LL_lst", TAB_MASK,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.date_create,
					inloRec.inlo_hash,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
				no_LL++;
			}
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
	}
}
/*
 * Enter quantities for selected lot/location.
 */
static	int
LL_Qty (int c, KEY_TAB *psUnused)
{
	int		y_pos;
	int		validQty;
	float	inputQty;
	float	L_CnvFct;
	float	I_CnvFct;
	float	Available;
	float	L_Avail,
			I_Avail;

	tab_get ("LL_lst", LocBuff, CURRENT, 0);

	y_pos = tab_sline ("LL_lst");

	validQty = FALSE;
	while (!validQty)
	{
		inputQty 	= (float) atof (LocBuff + offsetAllocQty);
		Available 	= (float) atof (LocBuff + offsetAvailQty);
		L_CnvFct 	= (float) atof (LocBuff + offsetLCnvFct);
		I_CnvFct 	= (float) atof (LocBuff + offsetICnvFct);
		crsr_on ();
		inputQty	= getfloat (InternalCol + offsetAllocQty + 1, y_pos, LLQtyFormat);

		L_Avail		=	Available * L_CnvFct;
		I_Avail		=	inputQty  * I_CnvFct;
		crsr_off ();

		if (last_char == FN1)
			break;

		if (last_char == FN2)
			inputQty 	= (float) atof (LocBuff + offsetAllocQty);

		if (L_Avail < I_Avail && !IgnoreAvailChk && inputQty > 0.00)
		{
			inputQty 	= (float) atof (LocBuff + offsetAllocQty);
			print_mess (ML ("Quantity cannot be more than available for Location/Lot"));
			sleep (sleepTime);
			continue;
		}

		validQty = TRUE;
	}
	sprintf (LLWorkQty [0], LLQtyMask, atof (LocBuff + offsetAvailQty));
	sprintf (LLWorkQty [1], LLQtyMask, inputQty);
	tab_update
	(
		"LL_lst", TAB_MASK,
		LocBuff + offsetLocation,
		LocBuff + offsetLotNo,
		LocBuff + offsetLocType,
		LocBuff + offsetSlotNo,
		LocBuff + offsetExpiry,
		LocBuff + offsetL_UOM,
		LLWorkQty [0],
		LocBuff + offsetI_UOM,
		LLWorkQty [1],
		atof (LocBuff + offsetLCnvFct),
		atof (LocBuff + offsetICnvFct),
		atol (LocBuff + offsetHhwhHash),
		atol (LocBuff + offsetHhumHash),
		atol (LocBuff + offsetHhccHash),
		atol (LocBuff + offsetCreateDate),
		atol (LocBuff + offsetInloHash),
		LocBuff + offsetLocStatus,
		atof (LocBuff + offsetPackQty),
		atof (LocBuff + offsetChgWgt),
		atof (LocBuff + offsetGrossWgt),
		atof (LocBuff + offsetCuMetre),
		atol (LocBuff + offsetSkndHash)
	);

	return (c);
}
/*
 * Enter Lot details. 
 */
static	int	
LL_Lot (int c, KEY_TAB *psUnused)
{
	int		y_pos;
	int		validLot;
	char	LotNum [8],
			SLotNum [8],
			LocNum [11],
			LocType [2],
			ExpDate [11],
			L_UOM [5];
	float	inputQty;
	long	_hhwhHash,
			_hhccHash;

	tab_get ("LL_lst", LocBuff, CURRENT, 0);

	y_pos = tab_sline ("LL_lst");

	if (c == -1)
	{
		sprintf (LotNum, "%-7.7s", LocBuff + offsetLotNo);
		if (strcmp (LotNum, lotNoPad))
			return (EXIT_SUCCESS);
	}
	if (!LL_LOT_CTRL && !envVarSkGrinNoPlate)
	{
		sprintf (LotNum, "%-7.7s", "  N/A  ");
		strcpy (SLotNum, LotNum);
		validLot = TRUE;
	}
	else
		validLot = FALSE;

	while (!validLot)
	{
		sprintf (LotNum, "%-7.7s", 	LocBuff + offsetLotNo);
		sprintf (SLotNum, "%-7.7s", LocBuff + offsetSlotNo);
		inputQty 	= (float) atof (LocBuff + offsetAllocQty);
		getalpha (InternalCol + offsetLotNo + 1, y_pos, "UUUUUUU", LotNum);
		crsr_off ();
		
		_hhwhHash	=	atol (LocBuff + offsetHhwhHash);
		_hhccHash	=	atol (LocBuff + offsetHhccHash);
		sprintf (LocNum, "%-10.10s",	LocBuff + offsetLocation);
		sprintf (LocType, "%-1.1s", 	LocBuff + offsetLocType);
		sprintf (ExpDate, "%-10.10s", 	LocBuff + offsetExpiry);
		sprintf (L_UOM,	  "%-4.4s",   	LocBuff + offsetL_UOM);

		if (last_char == FN4)
		{
			SearchLOC (FALSE, _hhwhHash, LotNum);
			strcpy (LotNum, inloRec.lot_no);
			strcpy (SLotNum, inloRec.slot_no);
			redraw_table ("LL_lst");
			last_char	= ' ';

		}
		if (last_char == FN2)
			sprintf (LotNum, "%-7.7s", LocBuff + offsetLotNo);

		if (last_char == FN1)
			break;

		if (last_char == FN16)
			continue;

		if (!strlen (clip (LotNum)) && inputQty != 0.00)
		{
			print_mess (ML ("Lot number must be input"));
			sleep (sleepTime);
			continue;
		}
		if (!strlen (clip (SLotNum)))
			strcpy (SLotNum, LotNum);

		validLot = TRUE;
	}
	sprintf (LLWorkQty [0], LLQtyMask, atof (LocBuff + offsetAvailQty));
	sprintf (LLWorkQty [1], LLQtyMask, atof (LocBuff + offsetAllocQty));
	tab_update
	(
		"LL_lst", TAB_MASK,
		LocBuff + offsetLocation,
		LotNum,
		LocBuff + offsetLocType,
		SLotNum,
		LocBuff + offsetExpiry,
		LocBuff + offsetL_UOM,
		LLWorkQty [0],
		LocBuff + offsetI_UOM,
		LLWorkQty [1],
		atof (LocBuff + offsetLCnvFct),
		atof (LocBuff + offsetICnvFct),
		atol (LocBuff + offsetHhwhHash),
		atol (LocBuff + offsetHhumHash),
		atol (LocBuff + offsetHhccHash),
		atol (LocBuff + offsetCreateDate),
		atol (LocBuff + offsetInloHash),
		LocBuff + offsetLocStatus,
		atof (LocBuff + offsetPackQty),
		atof (LocBuff + offsetChgWgt),
		atof (LocBuff + offsetGrossWgt),
		atof (LocBuff + offsetCuMetre),
		atol (LocBuff + offsetSkndHash)
	);

	return (c);
}
/*
 * Enter supplier lot details. 
 */
static	int	
LL_SLot (int c, KEY_TAB *psUnused)
{
	int		y_pos;
	int		validLot;
	char	SLotNum [8];

	tab_get ("LL_lst", LocBuff, CURRENT, 0);

	if (c == -1)
	{
		sprintf (SLotNum, "%-7.7s", LocBuff + offsetSlotNo);
		if (strcmp (SLotNum, lotNoPad))
			return (EXIT_SUCCESS);
	}
	y_pos = tab_sline ("LL_lst");

	validLot = FALSE;
	while (!validLot)
	{
		sprintf (SLotNum, "%-7.7s", LocBuff + offsetSlotNo);
		getalpha (InternalCol + offsetSlotNo + 1, y_pos, "UUUUUUU", SLotNum);
		crsr_off ();

		if (!strlen (clip (SLotNum)))
			sprintf (SLotNum, "%-7.7s", LocBuff + offsetLotNo);
		
		if (last_char == FN1)
			break;

		if (last_char == FN16)
			continue;

		if (last_char == FN2)
			sprintf (SLotNum, "%-7.7s", LocBuff + offsetSlotNo);

		validLot = TRUE;
	}
	sprintf (LLWorkQty [0], LLQtyMask, atof (LocBuff + offsetAvailQty));
	sprintf (LLWorkQty [1], LLQtyMask, atof (LocBuff + offsetAllocQty));
	tab_update
	(
		"LL_lst", TAB_MASK,
		LocBuff + offsetLocation,
		LocBuff + offsetLotNo,
		LocBuff + offsetLocType,
		SLotNum,
		LocBuff + offsetExpiry,
		LocBuff + offsetL_UOM,
		LLWorkQty [0],
		LocBuff + offsetI_UOM,
		LLWorkQty [1],
		atof (LocBuff + offsetLCnvFct),
		atof (LocBuff + offsetICnvFct),
		atol (LocBuff + offsetHhwhHash),
		atol (LocBuff + offsetHhumHash),
		atol (LocBuff + offsetHhccHash),
		atol (LocBuff + offsetCreateDate),
		atol (LocBuff + offsetInloHash),
		LocBuff + offsetLocStatus,
		atof (LocBuff + offsetPackQty),
		atof (LocBuff + offsetChgWgt),
		atof (LocBuff + offsetGrossWgt),
		atof (LocBuff + offsetCuMetre),
		atol (LocBuff + offsetSkndHash)
	);

	return (c);
}
/*
 * Enter location details.
 */
static	int	
LL_Loc (int c, KEY_TAB *psUnused)
{
	int		y_pos;
	int		validLoc;
	float	AllocQty;
	char	LocNum [11];
	long	_hhwhHash,
			_hhumHash,
			_hhccHash,
			_inloHash;

	tab_get ("LL_lst", LocBuff, CURRENT, 0);

	y_pos = tab_sline ("LL_lst");

	if (c == -1)
	{
		sprintf (LocNum, "%-10.10s", LocBuff + offsetLocation);
		if (strcmp (LocNum, locationPad))
			return (EXIT_SUCCESS);
	}
	_inloHash	=	atol (LocBuff + offsetInloHash);
	if (_inloHash)
	{
		sprintf (LocNum, "%-10.10s", LocBuff + offsetLocation);
		sprintf (lomrRec.loc_type, "%-1.1s", LocBuff + offsetLocType);
		validLoc = TRUE;
		print_mess (ML ("Existing Location cannot be changed."));
		sleep (sleepTime);
	}
	else
		validLoc = FALSE;
	while (!validLoc)
	{
		sprintf (LocNum, "%-10.10s", LocBuff + offsetLocation);
		getalpha (InternalCol + offsetLocation + 1, y_pos, "UUUUUUUUUU", LocNum);
		crsr_off ();

		_hhwhHash	=	atol (LocBuff + offsetHhwhHash);
		_hhumHash	=	atol (LocBuff + offsetHhumHash);
		_hhccHash	=	atol (LocBuff + offsetHhccHash);
		_inloHash	=	atol (LocBuff + offsetInloHash);

		if (last_char == FN4)
		{
			if (search_key == SEARCH)
			{
				SearchLOC (TRUE, _hhwhHash, LocNum);
				strcpy (LocNum, inloRec.location);
				_inloHash	=	(_inloHash) ? inloRec.inlo_hash : 0L;
			}
			else
			{
				SearchLomr ((_hhccHash) ? _hhccHash : llctRec.hhcc_hash,LocNum);
				_inloHash	= 0L;
			}
			redraw_table ("LL_lst");
			last_char	= ' ';
		}
		if (last_char == FN2)
			sprintf (LocNum, "%-10.10s", LocBuff + offsetLocation);
	
		if (last_char == FN1)
			break;

		if (!strlen (clip (LocNum)))
		{
			print_mess (ML ("Location must be input"));
			sleep (sleepTime);
			continue;
		}
		if (!_inloHash && !strcmp (LocNum, locationPad))
		{
			cc = FindLocation
			 	(
					_hhwhHash,
					_hhumHash,
					LocNum,
					ValidLocations,
					&dHash
			 	);
		}
		else
		{
			abc_selfield (inlo, "inlo_inlo_hash");
			inloRec.inlo_hash	=	_inloHash;
			cc = find_rec (inlo, &inloRec, COMPARISON, "r");
		}
		cc	=	CheckLocation 
				(
					(_hhccHash) ? _hhccHash : llctRec.hhcc_hash,
					LocNum, 
					lomrRec.loc_type
				);
		if (cc)
		{
			print_mess (ML ("Location does not exist in Location Master."));
			sleep (sleepTime);
			validLoc = FALSE;
		}
		else
			validLoc = TRUE;
	}
	AllocQty	=	inloRec.qty - CalcAlloc (inloRec.inlo_hash,line_cnt);

	if (_inloHash)
	{
		strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
		sprintf (LLWorkQty [0], LLQtyMask, AllocQty / inloRec.cnv_fct);
		sprintf (LLWorkQty [1], LLQtyMask, atof (LocBuff + offsetAllocQty));
		tab_update
		(
			"LL_lst", TAB_MASK,
			LocNum,
			inloRec.lot_no,
			lomrRec.loc_type,
			inloRec.slot_no,
			LLExpiryDate,
			inloRec.uom,
			LLWorkQty [0],
			LocBuff + offsetI_UOM,
			LLWorkQty [1],
			inloRec.cnv_fct,
			atof (LocBuff + offsetLCnvFct),
			inloRec.hhwh_hash,
			inloRec.hhum_hash,
			atol (LocBuff + offsetHhccHash),
			inloRec.date_create,
			inloRec.inlo_hash,
			inloRec.loc_status,
			inloRec.pack_qty,
			inloRec.chg_wgt,
			inloRec.gross_wgt,
			inloRec.cu_metre,
			inloRec.sknd_hash
		);
	}
	else
	{
		sprintf (LLWorkQty [0], LLQtyMask, atof (LocBuff + offsetAvailQty));
		sprintf (LLWorkQty [1], LLQtyMask, atof (LocBuff + offsetAllocQty));
		tab_update
		(
			"LL_lst", TAB_MASK,
			LocNum,
			LocBuff + offsetLotNo,
			lomrRec.loc_type,
			LocBuff + offsetSlotNo,
			LocBuff + offsetExpiry,
			LocBuff + offsetL_UOM,
			LLWorkQty [0],
			LocBuff + offsetI_UOM,
			LLWorkQty [1],
			atof (LocBuff + offsetLCnvFct),
			atof (LocBuff + offsetICnvFct),
			atol (LocBuff + offsetHhwhHash),
			atol (LocBuff + offsetHhumHash),
			atol (LocBuff + offsetHhccHash),
			atol (LocBuff + offsetCreateDate),
			atol (LocBuff + offsetInloHash),
			LocBuff + offsetLocStatus,
			atof (LocBuff + offsetPackQty),
			atof (LocBuff + offsetChgWgt),
			atof (LocBuff + offsetGrossWgt),
			atof (LocBuff + offsetCuMetre),
			atol (LocBuff + offsetSkndHash)
		);
	}
	return (c);
}
/*
 * Enter date of expiry.
 */
static	int	LL_Date (int c, KEY_TAB *psUnused)
{
	int		y_pos;
	int		validDate;
	char	ExpDate [11];

	if (!LL_EditDate)
		return (EXIT_SUCCESS);

	tab_get ("LL_lst", LocBuff, CURRENT, 0);

	y_pos = tab_sline ("LL_lst");

	if (c == -1)
	{
		sprintf (ExpDate, "%-10.10s", LocBuff + offsetExpiry);
		if (strcmp (ExpDate, locationPad))
			return (EXIT_SUCCESS);
	}
	validDate = FALSE;
	while (!validDate)
	{
		sprintf (ExpDate, "%-10.10s", LocBuff + offsetExpiry);
		get_date (InternalCol + offsetExpiry + 1, y_pos, "NN/NN/NNNN", ExpDate);
		if (StringToDate (ExpDate) < 0L)
		{
			print_mess (ML ("Not a Valid Date"));
			sleep (sleepTime);
			continue;
		}
		strcpy (ExpDate, ExpDate);
		crsr_off ();

		if (!strlen (clip (ExpDate)))
		{
			print_mess (ML ("Expiry Date must be input"));
			sleep (sleepTime);
			continue;
		}
		if (last_char == FN1)
			break;

		if (last_char == FN2)
			sprintf (ExpDate, "%-10.10s", LocBuff + offsetExpiry);

		validDate = TRUE;
	}
	sprintf (LLWorkQty [0], LLQtyMask, atof (LocBuff + offsetAvailQty));
	sprintf (LLWorkQty [1], LLQtyMask, atof (LocBuff + offsetAllocQty));
	tab_update
	(
		"LL_lst", TAB_MASK,
		LocBuff + offsetLocation,
		LocBuff + offsetLotNo,
		LocBuff + offsetLocType,
		LocBuff + offsetSlotNo,
		ExpDate,
		LocBuff + offsetL_UOM,
		LLWorkQty [0],
		LocBuff + offsetI_UOM,
		LLWorkQty [1],
		atof (LocBuff + offsetLCnvFct),
		atof (LocBuff + offsetICnvFct),
		atol (LocBuff + offsetHhwhHash),
		atol (LocBuff + offsetHhumHash),
		atol (LocBuff + offsetCreateDate),
		atol (LocBuff + offsetInloHash),
		LocBuff + offsetLocStatus,
		atof (LocBuff + offsetPackQty),
		atof (LocBuff + offsetChgWgt),
		atof (LocBuff + offsetGrossWgt),
		atof (LocBuff + offsetCuMetre),
		atol (LocBuff + offsetSkndHash)
	);

	return (c);
}

static	int	
abort_func
 (
	int	c,
	KEY_TAB	*psUnused
)
{
	make_LL_list = FALSE;
	return (FN16);
}
/*
 * Main processing routine for lots and locations.  
 * Passes a lot of arguments but takes a lot of code 
 * from the program related to lots and locations.  
 */
int
DspLLStake
 (
	int		lineCnt,
	int		ROW,
	int		COL,
	int		numberLines,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	float	conversionFactor,
	char	*UOM,
	long	expiryDate,
	char	*locationNo,
	char	*LotCtrl,
	int		_Silent
)
{
	int		ORIG_SRCH_LINES,
			ORIG_X_POS,
			ORIG_Y_POS;

	extern	int		NO_SRCH_LINES;
	extern	int		SR_X_POS;
	extern	int		SR_Y_POS;
	
	IgnoreTotal		=	TRUE;

	ORIG_SRCH_LINES	=	NO_SRCH_LINES;
	ORIG_X_POS		=	SR_X_POS;
	ORIG_Y_POS		=	SR_Y_POS;

	if (LotCtrl [0] == 'Y' || LotCtrl [0] == 'y')
		strcpy (LotControl, "Y");
	else
		strcpy (LotControl, "N");

	if (ROW > MAX_SCN_LINE)
	{
		numberLines = 	MIN_LL_LINES;
		ROW		-=	(MIN_LL_LINES + 5);
	}

	SR_X_POS		=	COL;
	SR_Y_POS		=	ROW;
	NO_SRCH_LINES	=	numberLines - 2;

	if (!_Silent)
	{
		Open_LL_Tab
		(
			ROW,	
			COL,
			numberLines
		);
	}

	cc = LoadStake 
	(
		lineCnt,
		hhwhHash, 
		hhumHash, 
		hhccHash, 
		locationNo,
		UOM,
		conversionFactor,
		_Silent
	);
	if (!_Silent)
	{
		strcpy (LLExpiryDate, DateToString (expiryDate));
		sprintf (LLWorkQty [0], LLQtyMask, 0.00);
		sprintf (LLWorkQty [1], LLQtyMask, 0.00);
		tab_add
		(
			"LL_lst", TAB_MASK,
			(locationNo == (char *) 0) ? locationPad : locationNo,
			(LL_LOT_CTRL) ? lotNoPad : "  N/A  ",
			" ",
			(LL_LOT_CTRL) ? lotNoPad : "  N/A  ",
			LLExpiryDate,
			UOM,
			LLWorkQty [0],
			UOM,
			LLWorkQty [1],
			conversionFactor,
			conversionFactor,
			hhwhHash,
			hhumHash,
			hhccHash,
			TodaysDate (),
			0L,
			"A",
			0.00,
			0.00,
			0.00,
			0.00,
			0L
		);
		no_LL++;
		scan_LL_tab ();
	}
	ProcList (lineCnt, (float) 0.00);
	
	NO_SRCH_LINES	=	ORIG_SRCH_LINES;
	SR_X_POS		=	ORIG_X_POS;
	SR_Y_POS		=	ORIG_Y_POS;
	if (_Silent)
		return (EXIT_SUCCESS);
	return (!make_LL_list);
}

/*
 * Main processing routine for lots and locations.  
 * Passes a lot of arguments but takes a lot of code
 * from the program related to lots and locations.  
 */
int
DisplayLL (
	int		lineCnt,
	int		ROW,
	int		COL,
	int		numberLines,
	long	hhwhHash,
	long	hhumHash,
	long	hhccHash,
	char	*UOM,
	float	quantity,
	float	conversionFactor,
	long	expiryDate,
	int		_Silent,
	int		_Input,
	char	*LotCtrl)
{
	int		ORIG_SRCH_LINES,
			ORIG_X_POS,
			ORIG_Y_POS;

	extern	int		NO_SRCH_LINES;
	extern	int		SR_X_POS;
	extern	int		SR_Y_POS;
	errorQuantity		=	0.00;
	
	ORIG_SRCH_LINES	=	NO_SRCH_LINES;
	ORIG_X_POS		=	SR_X_POS;
	ORIG_Y_POS		=	SR_Y_POS;

	set_keys (LL_keys, "L", (LL_EditLoc) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "B", (LL_EditLot) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "S", (LL_EditSLot) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "E", (LL_EditDate)	? KEY_ACTIVE	:	KEY_PASSIVE);

	if (LotCtrl [0] == 'Y' || LotCtrl [0] == 'y')
		strcpy (LotControl, "Y");
	else
		strcpy (LotControl, "N");

	if (ROW > MAX_SCN_LINE)
	{
		numberLines = 	MIN_LL_LINES;
		ROW		-=	(MIN_LL_LINES + 5);
	}

	SR_X_POS		=	COL;
	SR_Y_POS		=	ROW;
	NO_SRCH_LINES	=	numberLines - 2;

	/*
	 * Open Lot window.
	 */
	if (!_Silent)
	{
		Open_LL_Tab
		(
			ROW,	
			COL,
			numberLines
		);
	}

	/*
	 * Load records for outgoing lots/locations.
	 */
	if (!_Input)
	{
		cc = !LoadLL 
			(
				lineCnt,
				_Silent, 
				hhwhHash, 
				hhumHash, 
				hhccHash, 
				UOM,
				conversionFactor,
				quantity
			);
		if (cc)
		{
			if (!_Silent)
			{
				LoadManualLL
				(
					lineCnt, 
					hhwhHash, 
					hhumHash, 
					hhccHash,
					UOM, 
					conversionFactor, 
					quantity 
				);
			}
			else
			{
				NO_SRCH_LINES	=	ORIG_SRCH_LINES;
				SR_X_POS		=	ORIG_X_POS;
				SR_Y_POS		=	ORIG_Y_POS;
				return (EXIT_FAILURE);
			}
		}
	}
	else
	{
		NewLL 
		(
			lineCnt, 
			hhwhHash, 
			hhumHash, 
			hhccHash,
			UOM, 
			conversionFactor, 
			UOM, 
			conversionFactor, 
			quantity, 
			expiryDate,
			_Silent
		);
	}
	if (!_Silent)
	{
		errorQuantity	=	0.00;
		scan_LL_tab ();
	}

	ProcList (lineCnt, errorQuantity);

	NO_SRCH_LINES	=	ORIG_SRCH_LINES;
	SR_X_POS		=	ORIG_X_POS;
	SR_Y_POS		=	ORIG_Y_POS;

	if (_Silent)
		return (EXIT_SUCCESS);

	return (!make_LL_list);
}
/*
 * Main processing routine for lots and locations.  
 * Passes a lot of arguments but takes a lot of code
 * from the program related to lots and locations. 
 */
int
DspLLTrans
 (
	int		lineCnt,
	int		ROW,
	int		COL,
	int		numberLines,
	long	itffHash,
	long	hhwhHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	char	*LotCtrl,
	int		_Silent
)
{
	int		ORIG_SRCH_LINES,
			ORIG_X_POS,
			ORIG_Y_POS;

	extern	int		NO_SRCH_LINES;
	extern	int		SR_X_POS;
	extern	int		SR_Y_POS;
	
	IgnoreTotal		=	TRUE;

	ORIG_SRCH_LINES	=	NO_SRCH_LINES;
	ORIG_X_POS		=	SR_X_POS;
	ORIG_Y_POS		=	SR_Y_POS;

	if (LotCtrl [0] == 'Y' || LotCtrl [0] == 'y')
		strcpy (LotControl, "Y");
	else
		strcpy (LotControl, "N");

	if (ROW > MAX_SCN_LINE)
	{
		numberLines = 	MIN_LL_LINES;
		ROW		-=	(MIN_LL_LINES + 5);
	}

	SR_X_POS		=	COL;
	SR_Y_POS		=	ROW;
	NO_SRCH_LINES	=	numberLines - 2;

	if (!_Silent)
	{
		Open_LL_Tab
		(
			ROW,	
			COL,
			numberLines
		);
	}

	cc = LoadTrans 
	(
		lineCnt,
		itffHash, 
		hhwhHash, 
		hhccHash,
		UOM,
		conversionFactor,
		_Silent
	);
	if (!_Silent)
		scan_LL_tab ();

	ProcList (lineCnt, (float) 0.00);
	
	NO_SRCH_LINES	=	ORIG_SRCH_LINES;
	SR_X_POS		=	ORIG_X_POS;
	SR_Y_POS		=	ORIG_Y_POS;
	if (_Silent)
		return (EXIT_SUCCESS);

	return (!make_LL_list);
}
static	void
Open_LL_Tab (
	int		_Row,
	int		_Col,
	int		_Depth)
{
	InternalCol	=	_Col;
	no_LL = 0;
	tab_open ("LL_lst", LL_keys, _Row, _Col, _Depth, FALSE);
	
	set_keys (LL_keys, "L", (LL_EditLoc) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "B", (LL_EditLot) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "S", (LL_EditSLot) 	? KEY_ACTIVE	:	KEY_PASSIVE);
	set_keys (LL_keys, "E", (LL_EditDate)	? KEY_ACTIVE	:	KEY_PASSIVE);

	tab_add
	(
		"LL_lst", 
		"#  Location  | lot Number | T |SupLot No|Expiry Date | UOM. |Qty Available| UOM. |Allocate Qty."
	);
}

/*
 * Scan records added to list. 
 */
static	void
scan_LL_tab (void)
{
	long	tmp_hhwhHash;
	long	tmp_hhumHash;
	long	tmp_hhccHash;
	long	tmp_inloHash;
	char	tmp_loc [11];
	char	tmp_lot [8];
	char	tmp_loc_type [2];
	char	tmp_slot [8];
	char	tmp_date [11];
	char	tmp_l_uom [5];
	char	tmp_i_uom [5];
	float	tmp_qty_avail;
	float	tmp_alloc_qty;
	float	tmp_l_cnv_fct;
	float	tmp_i_cnv_fct;
	long	tmp_date_create;
	char	tmp_locationStat [2];
	float	tmp_packQuantity;
	float	tmp_chargeWeight;
	float	tmp_grossWeight;
	float	tmp_cubicMetre;
	long	tmp_skndHash;
	int		i;

	if (no_LL == 0)
	{
		putchar (BELL);
		tab_add ("LL_lst", " ** No Lots / Locations ** ");
		tab_display ("LL_lst", TRUE);
		sleep (sleepTime);
		tab_close ("LL_lst", TRUE);
		make_LL_list = FALSE;
	}
	else
	{
		tab_scan ("LL_lst");
		/*
		 * Build list of selected items
		 */
		FreeList ();
		if (make_LL_list)
		{
		    for (i = 0; i < no_LL; i++)
		    {
				tab_get ("LL_lst", LocBuff, EQUAL, i);

				sprintf (tmp_loc,  		"%-10.10s",	LocBuff + offsetLocation);
				sprintf (tmp_lot,  		"%-7.7s", 	LocBuff + offsetLotNo);
				sprintf (tmp_loc_type,  "%-1.1s", 	LocBuff + offsetLocType);
				sprintf (tmp_slot, 		"%-7.7s", 	LocBuff + offsetSlotNo);
				sprintf (tmp_date, 		"%-10.10s", LocBuff + offsetExpiry);
				sprintf (tmp_i_uom, 	"%4.4s", 	LocBuff + offsetI_UOM);
				sprintf (tmp_l_uom, 	"%4.4s", 	LocBuff + offsetL_UOM);
				sprintf (tmp_locationStat, "%-1.1s", LocBuff + offsetLocStatus);
				tmp_qty_avail		=	(float) atof (LocBuff + offsetAvailQty);
				tmp_alloc_qty		=	(float) atof (LocBuff + offsetAllocQty);
				tmp_l_cnv_fct		=	(float) atof (LocBuff + offsetLCnvFct);
				tmp_i_cnv_fct		=	(float) atof (LocBuff + offsetICnvFct);
				tmp_hhwhHash		=	atol (LocBuff + offsetHhwhHash);
				tmp_hhumHash		=	atol (LocBuff + offsetHhumHash);
				tmp_hhccHash		=	atol (LocBuff + offsetHhccHash);
				tmp_inloHash		=	atol (LocBuff + offsetInloHash);
				tmp_date_create		=	atol (LocBuff + offsetCreateDate);
				tmp_packQuantity	= 	(float) atof (LocBuff + offsetPackQty);
				tmp_chargeWeight	= 	(float) atof (LocBuff + offsetChgWgt);
				tmp_grossWeight		= 	(float) atof (LocBuff + offsetGrossWgt);
				tmp_cubicMetre		= 	(float) atof (LocBuff + offsetCuMetre);
				tmp_skndHash		=	atol (LocBuff + offsetSkndHash);

				LLAddList
				(
					tmp_hhwhHash,
					tmp_hhumHash,
					tmp_hhccHash,
					tmp_inloHash,
					tmp_loc,
					tmp_lot,
					tmp_loc_type,
					tmp_slot,
					tmp_date,
					tmp_l_uom,
					tmp_qty_avail,
					tmp_i_uom,
					tmp_alloc_qty,
					tmp_l_cnv_fct,
					tmp_i_cnv_fct,
					tmp_date_create,
					tmp_locationStat,
					tmp_packQuantity,
					tmp_chargeWeight,
					tmp_grossWeight,
					tmp_cubicMetre,
					tmp_skndHash
				);
		    }
		}
		tab_close ("LL_lst", TRUE);
	}
	return;
}

/*
 * OK selection function.
 */
static	int	
ok_func (
	int		c,
	KEY_TAB	*psUnused)
{
	int		i;
	float	TotalAlloc	 =	0.00,
			allocatedQty =	0.00,
			availableQty =	0.00;

	char	Loc [11],
			Lot [8],
			SLot [8];

	for (i = 0; i < no_LL; i++)
	{
		cc = tab_get ("LL_lst", LocBuff, EQUAL, i);
		if (!cc)
		{
			allocatedQty	= 	(float) atof (LocBuff + offsetAllocQty);
			TotalAlloc		+=	allocatedQty;
			allocatedQty	*= 	(float) atof (LocBuff + offsetICnvFct);

			availableQty	= 	(float) atof (LocBuff + offsetAvailQty);
			availableQty	*= 	(float) atof (LocBuff + offsetLCnvFct);


			sprintf (Loc, "%-10.10s", 	LocBuff + offsetLocation);
			sprintf (Lot, "%-7.7s", 	LocBuff + offsetLotNo);
			sprintf (SLot,"%-7.7s", 	LocBuff + offsetSlotNo);

			if (!strlen (clip (Loc)) && allocatedQty != 0.00)
			{
				print_mess (ML ("Location number must be input"));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}
			if (availableQty < allocatedQty && 
				!IgnoreAvailChk && 
				allocatedQty > 0.00)
			{
				print_mess (ML ("Quantity cannot be more than available for Location/Lot"));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}
			if (!strlen (clip (Lot)) && allocatedQty != 0.00)
			{
				print_mess (ML ("Lot number must be input"));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}

			if (!strlen (clip (SLot)) && allocatedQty != 0.00)
			{
				print_mess (ML ("Supplier Lot number must be input"));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}
		}
	}
	if ((no_dec (qtyRequired) != no_dec (TotalAlloc)) && !IgnoreTotal)
	{
		sprintf (LLWorkQty [0], LLQtyMask, TotalAlloc);
		sprintf (LLWorkQty [1], LLQtyMask, qtyRequired);
		sprintf (err_str, "%s [%11.11s] [%11.11s]", 
			ML ("Allocation Quantity is not equal to Required Quantity"),
			LLWorkQty [0], LLWorkQty [1]);

		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (' ');
	}
	make_LL_list = TRUE;
	return (FN16);
}
void
FreeList (void)
{
	struct SEL_LIST *lcl_ptr;

	if (selhd_ptr == SEL_NULL)
		return;

	selcur_ptr = selhd_ptr;
	while (selcur_ptr != SEL_NULL)
	{
		lcl_ptr = selcur_ptr;
		selcur_ptr = selcur_ptr->next;
		free (lcl_ptr);
	}
	selhd_ptr = SEL_NULL;
	selcur_ptr = SEL_NULL;
}

/*
 * Add records to list.
 */
void
LLAddList (
	long	_hhwhHash,
	long	_hhumHash,
	long	_hhccHash,
	long	_inloHash,
	char	*_locationNo,
	char	*_LOT,
	char	*_locationType,
	char	*_SLOT,
	char	*_DATE,
	char	*_L_UOM,
	float	_QTY_AVAIL, 
	char	*_I_UOM,
	float	_quantity,
	float	_L_conversionFactor,
	float	_I_conversionFactor,
	long	_CREATE_DATE,
	char	*_locationStat,
	float	_packQuantity,
	float	_chargeWeight,
	float	_grossWeight,
	float	_cubicMetre,
	long	_skndHash)
{
	struct SEL_LIST *lcl_ptr;

	lcl_ptr = sel_alloc ();
	sprintf (lcl_ptr->_loc, 			"%-10.10s", _locationNo);
	sprintf (lcl_ptr->_lot, 			"%-7.7s",   _LOT);
	sprintf (lcl_ptr->_loc_type, 		"%-1.1s",   _locationType);
	sprintf (lcl_ptr->_slot,			"%-7.7s",   _SLOT);
	sprintf (lcl_ptr->_date,			"%-10.10s", _DATE);
	sprintf (lcl_ptr->_l_uom,			"%-4.4s",   _L_UOM);
	sprintf (lcl_ptr->_i_uom, 			"%-4.4s",   _I_UOM);
	sprintf (lcl_ptr->_locationStat,	"%-1.1s",  _locationStat);
	lcl_ptr->_hhwhHash		=	_hhwhHash;
	lcl_ptr->_hhumHash		=	_hhumHash;
	lcl_ptr->_hhccHash		=	_hhccHash;
	lcl_ptr->_inloHash		=	_inloHash;
	lcl_ptr->_qty_avail		=	_QTY_AVAIL;
	lcl_ptr->_qty			=	_quantity;
	lcl_ptr->_l_cnv_fct		=	_L_conversionFactor;
	lcl_ptr->_i_cnv_fct		=	_I_conversionFactor;
	lcl_ptr->_date_create	=	_CREATE_DATE;
	lcl_ptr->_packQuantity	=	_packQuantity;
	lcl_ptr->_chargeWeight	=  	_chargeWeight;
	lcl_ptr->_grossWeight	=  	_grossWeight;
	lcl_ptr->_cubicMetre	=  	_cubicMetre;
	lcl_ptr->_skndHash		=  	_skndHash;

	lcl_ptr->next = SEL_NULL;

	if (selhd_ptr == SEL_NULL)
		selhd_ptr = lcl_ptr;
	else
		selcur_ptr->next = lcl_ptr;
	selcur_ptr = lcl_ptr;
}

/*
 * Allocate a block of memory for one node of item selectlinked list
 */
static	struct SEL_LIST *sel_alloc (void)
{
	struct SEL_LIST *lcl_ptr;

	lcl_ptr = (struct SEL_LIST *) malloc (sizeof (struct SEL_LIST));
	if (lcl_ptr == SEL_NULL)
		sys_err ("Error during (MALLOC)", errno, PNAME);
	return (lcl_ptr);
}
/*
 * Process lot/location list.
 */
void
ProcList (
	int		lineCnt,
	float	errorQuantity)
{
	int		i	=	0;


	struct	SEL_LIST *lcl_ptr;

	if (!ArrChkLimit (&lots_d, LOT, lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	ClearLotsInfo (lineCnt);

	lcl_ptr   = selhd_ptr;
	while (lcl_ptr != SEL_NULL)
	{
		if (errorQuantity)
			lcl_ptr->_qty	+=	errorQuantity;

		errorQuantity	=	0.00;


		strcpy (LOT [lineCnt]._loc   [i],	 lcl_ptr->_loc);
		strcpy (LOT [lineCnt]._lot   [i],	 lcl_ptr->_lot);
		strcpy (LOT [lineCnt]._loc_type [i]	,lcl_ptr->_loc_type);
		strcpy (LOT [lineCnt]._slot  [i]	,lcl_ptr->_slot);
		strcpy (LOT [lineCnt]._date  [i]	,lcl_ptr->_date);
		strcpy (LOT [lineCnt]._l_uom [i]	,lcl_ptr->_l_uom);
		strcpy (LOT [lineCnt]._i_uom [i]	,lcl_ptr->_i_uom);
		strcpy (LOT [lineCnt]._locationStat[i] ,lcl_ptr->_locationStat);
		LOT [lineCnt]._qty_avail    [i]	=	 lcl_ptr->_qty_avail;
		LOT [lineCnt]._qty          [i]	=	 lcl_ptr->_qty;
		LOT [lineCnt]._l_cnv_fct    [i]	=	 lcl_ptr->_l_cnv_fct;
		LOT [lineCnt]._i_cnv_fct    [i]	=	 lcl_ptr->_i_cnv_fct;
		LOT [lineCnt]._hhwhHash    [i]	=	 lcl_ptr->_hhwhHash;
		LOT [lineCnt]._hhumHash    [i]	=	 lcl_ptr->_hhumHash;
		LOT [lineCnt]._hhccHash    [i]	=	 lcl_ptr->_hhccHash;
		LOT [lineCnt]._inloHash    [i]	=	 lcl_ptr->_inloHash;
		LOT [lineCnt]._date_create  [i]	=	 lcl_ptr->_date_create;
		LOT [lineCnt]._packQuantity [i] = 	 lcl_ptr->_packQuantity;
		LOT [lineCnt]._chargeWeight [i] =	 lcl_ptr->_chargeWeight;
		LOT [lineCnt]._grossWeight  [i] =    lcl_ptr->_grossWeight;
		LOT [lineCnt]._cubicMetre   [i] =    lcl_ptr->_cubicMetre;
		LOT [lineCnt]._skndHash     [i] =    lcl_ptr->_skndHash;

		errorQuantity = 0.00;
		i++;
		AddAlloc
		(
			lcl_ptr->_inloHash,
			lineCnt,
			lcl_ptr->_qty * lcl_ptr->_i_cnv_fct
		);
		if (i > MAX_LOTS)
			break;
		lcl_ptr = lcl_ptr->next;
	}
	FreeList ();
}

/*
 * Calculate lot allocation
 */
float
CalcAlloc (
	long	inloHash,
	int		lineCnt)
{
	float	QtyAlloc	= 	0.00;

	if (inloHash	==	0L)
		return ((float) 0.00);

	inlaRec.inlo_hash	=	inloHash;
	inlaRec.pid		=	(pid_t) 0;
	inlaRec.line_no	=	0;
	cc = find_rec (inla, &inlaRec, GTEQ, "r");

	while (!cc && inlaRec.inlo_hash == inloHash)
	{
		if ((inlaRec.pid == (long) LotPID && inlaRec.line_no == lineCnt))
		{
			cc = find_rec (inla, &inlaRec, NEXT, "r");
			continue;
		}
		QtyAlloc	+= 	inlaRec.qty_alloc;
		cc = find_rec (inla, &inlaRec, NEXT, "r");
	}
	return (QtyAlloc);
}
	
/*
 * Add loc/allocation
 */
void
AddAlloc (
	long	inloHash,
	int		lineCnt,
	float	quantity)
{
	int		new_inla;

	memset (&inlaRec, 0, sizeof (inlaRec));

	inlaRec.pid			=	(long)	LotPID;
	inlaRec.inlo_hash	=	inloHash;
	inlaRec.line_no		=	lineCnt;
	new_inla = find_rec (inla, &inlaRec, COMPARISON, "u");
	
	inlaRec.qty_alloc	=	quantity;

	if (new_inla && quantity == 0.00)
		return;

	if (new_inla)
	{
		abc_unlock (inla);
		cc = abc_add (inla, &inlaRec);
		if (cc)
			file_err (cc, inla, "DBADD");
	}
	else
	{
		cc = abc_update (inla, &inlaRec);
		if (cc)
			file_err (cc, inla, "DBUPDATE");
	}
}
/*
 * Clear current allocation records.
 */
void
ClearAlloc (
	long	inloHash,
	int		lineCnt)
{
	inlaRec.pid			=	(long)	LotPID;
	inlaRec.inlo_hash	=	inloHash;
	inlaRec.line_no		=	lineCnt;
	cc = find_rec (inla, &inlaRec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (inla);
		return;
	}
	abc_delete (inla);
}
/*
 * Return quantity to calling program.
 */
float
GetQty (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._qty [_lineNumber]);
}
/*
 * Return pack quantity to calling program.
 */
float
GetPackQty (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._packQuantity [_lineNumber]);
}
/*
 * Return charge weight to calling program.
 */
float
GetChgWgt	
(
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._chargeWeight [_lineNumber]);
}
/*
 * Return gross weight to calling program.
 */
float
GetGrossWgt	 
(
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._grossWeight [_lineNumber]);
}
/*
 * Return cubic meters to calling program.
 */
float
GetCuMetres	 
(
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._cubicMetre [_lineNumber]);
}
/*
 * Return skndHash to calling program.
 */
long
GetSKND	 
(
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._skndHash [_lineNumber]);
}
/*
 * Return available quantity to calling program.
 */
float
GetAvailQty (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._qty_avail [_lineNumber]);
}
/*
 * Return base quantity to calling program.
 */
float
GetBaseQty (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._qty [_lineNumber] * 
		    LOT [_lineCnt]._i_cnv_fct [_lineNumber]);
}
/*
 * Return Conversion factor calling program.
 */
float
GetCnvFct (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._i_cnv_fct [_lineNumber]);
}
/*
 * Return UOM to calling program.
 */
char *
GetUOM (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._i_uom [_lineNumber];
	return (sptr);
}
/*
 * Return local UOM to calling program.
 */
char *
GetLUOM (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._l_uom [_lineNumber];
	return (sptr);
}
/*
 * Return location status to calling program.
 */
char *
GetLocStat (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._locationStat [_lineNumber];
	return (sptr);
}
/*
 * Return inloHash to calling program.
 */
long
GetINLO (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	if (LOT [_lineCnt]._inloHash [_lineNumber])
		return (LOT [_lineCnt]._inloHash [_lineNumber]);

	InLotLocation
	(
		LOT [_lineCnt]._hhwhHash	[_lineNumber],
		LOT [_lineCnt]._hhccHash	[_lineNumber],
		LOT [_lineCnt]._hhumHash	[_lineNumber],
		LOT [_lineCnt]._l_uom		[_lineNumber],
		LOT [_lineCnt]._lot		 	[_lineNumber],
		LOT [_lineCnt]._slot		[_lineNumber],
		LOT [_lineCnt]._loc		 	[_lineNumber],
		LOT [_lineCnt]._loc_type	[_lineNumber],
		LOT [_lineCnt]._date		[_lineNumber],
		0.00,
		LOT [_lineCnt]._l_cnv_fct	[_lineNumber],
		LOT [_lineCnt]._locationStat[_lineNumber],
		LOT [_lineCnt]._packQuantity[_lineNumber],
		LOT [_lineCnt]._chargeWeight[_lineNumber],
		LOT [_lineCnt]._grossWeight [_lineNumber],
		LOT [_lineCnt]._cubicMetre  [_lineNumber],
		LOT [_lineCnt]._skndHash    [_lineNumber]
	);
	return (inloRec.inlo_hash);
}

/*
 * Return HhumHash to calling program.
 */
long
GetHHUM (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._hhumHash [_lineNumber]);
}
/*
 * Return HhwhHash to calling program.
 */
long
GetHHWH (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	return (LOT [_lineCnt]._hhwhHash [_lineNumber]);
}

/*
 * Return lot number to calling program.
 */
char *
GetLotNo (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._lot [_lineNumber];
	return (sptr);
}
/*
 * Return suppliers lot to calling program.
 */
char *
GetSLotNo (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._slot [_lineNumber];
	return (sptr);
}
/*
 * Return location date to calling program.
 */
char *
GetLoc (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._loc [_lineNumber];
	return (sptr);
}
/*
 * Return expiry date to calling program.
 */
char *
GetExpiry (
	int		_lineCnt,
	int		_lineNumber)
{
	char	*sptr;

	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	sptr	=	LOT [_lineCnt]._date [_lineNumber];
	return (sptr);
}

/*
 * Check it lot/location is valid.
 */
int
LL_Valid (
	int		_lineCnt,
	int		_lineNumber)
{
	if (!ArrChkLimit (&lots_d, LOT, _lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	if (LOT [_lineCnt]._hhwhHash [_lineNumber] ==	0L)
		return (EXIT_SUCCESS);

	currentInloHash	= LOT [_lineCnt]._inloHash [_lineNumber];
	return (TRUE);
}

/*
 * Called routine to add / update location records.
 */
void
UpdateLotLocation (
	int		lineCnt,
	int		LL_ISSUE)
{
	_UpdateLotLocation
	(
		lineCnt,
		LL_ISSUE,
		0L,
		0L
	);
}
/*
 * Main routine to add / update location records.
 */
void
_UpdateLotLocation (
	int		lineCnt,
	int		LL_ISSUE,
	long	NEW_hhwhHash,
	long	NEW_hhccHash)
{
	int		i;

	abc_selfield (inlo, "inlo_mst_id");

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (LOT [lineCnt]._hhwhHash [i]	==	0L)
			break;

		if (LOT [lineCnt]._qty [i]	==	0.00)
			continue;

		if (LL_ISSUE)
		{
			OutLotLocation
			(
				(NEW_hhwhHash) ? NEW_hhwhHash : LOT [lineCnt]._hhwhHash [i],
				(NEW_hhccHash) ? NEW_hhccHash : LOT [lineCnt]._hhccHash [i],
				LOT [lineCnt]._hhumHash 		[i],
				LOT [lineCnt]._l_uom 			[i],
				LOT [lineCnt]._lot 				[i],
				LOT [lineCnt]._slot 			[i],
				LOT [lineCnt]._loc 				[i],
				LOT [lineCnt]._loc_type 		[i],
				LOT [lineCnt]._date 			[i],
				(LOT [lineCnt]._qty 	[i] * LOT [lineCnt]._i_cnv_fct [i]),
				LOT [lineCnt]._l_cnv_fct 		[i],
				LOT [lineCnt]._locationStat		[i],
				LOT [lineCnt]._packQuantity		[i],
				LOT [lineCnt]._chargeWeight		[i],
				LOT [lineCnt]._grossWeight 		[i],
				LOT [lineCnt]._cubicMetre  		[i],
				LOT [lineCnt]._skndHash 		[i]
			);
		}
		else
		{
			InLotLocation
			(
				(NEW_hhwhHash) ? NEW_hhwhHash : LOT [lineCnt]._hhwhHash [i],
				(NEW_hhccHash) ? NEW_hhccHash : LOT [lineCnt]._hhccHash [i],
				LOT [lineCnt]._hhumHash 		[i],
				LOT [lineCnt]._l_uom 			[i],
				LOT [lineCnt]._lot 				[i],
				LOT [lineCnt]._slot 			[i],
				LOT [lineCnt]._loc 				[i],
				LOT [lineCnt]._loc_type 		[i],
				LOT [lineCnt]._date 			[i],
				(LOT [lineCnt]._qty 	[i] * LOT [lineCnt]._i_cnv_fct [i]),
				LOT [lineCnt]._l_cnv_fct 		[i],
				LOT [lineCnt]._locationStat		[i],
				LOT [lineCnt]._packQuantity		[i],
				LOT [lineCnt]._chargeWeight		[i],
				LOT [lineCnt]._grossWeight 		[i],
				LOT [lineCnt]._cubicMetre  		[i],
				LOT [lineCnt]._skndHash 		[i]
			);
		}
		strcpy (LOT [lineCnt]._lot 			[i], 	inloRec.lot_no);
		strcpy (LOT [lineCnt]._locationStat [i], 	inloRec.loc_status);
		strcpy (LOT [lineCnt]._loc_type 	[i], 	inloRec.loc_type);
		LOT [lineCnt]._inloHash 			[i]	= 	inloRec.inlo_hash;
		ClearAlloc
		(
			inloRec.inlo_hash,
			lineCnt
		);
	}
}

/*
 * Write Transfer lots and locations to itlo file.
 */
void
TransLocation (
	int		lineCnt,
	long	itffHash)
{
	int		i;

	open_rec (itlo, itlo_list, ITLO_NO_FIELDS, "itlo_id_no");

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (LOT [lineCnt]._hhwhHash [i]	==	0L)
			break;

		if (LOT [lineCnt]._qty [i]	==	0.00)
			continue;

		itloRec.itff_hash	=	itffHash;
		itloRec.pr_line	=	lineCnt;
		itloRec.line_no	=	i;

		cc = find_rec (itlo, &itloRec, COMPARISON, "u");
		if (cc)
		{
			itloRec.itff_hash		=	itffHash,
			itloRec.pr_line			=	lineCnt;
			itloRec.line_no			=	i;
			itloRec.hhwh_hash		=	LOT [lineCnt]._hhwhHash [i];
			itloRec.hhum_hash		=	LOT [lineCnt]._hhumHash [i];
			itloRec.hhcc_hash		=	LOT [lineCnt]._hhccHash [i];
			itloRec.inlo_hash		=	LOT [lineCnt]._inloHash [i];
			itloRec.qty_avail		=	LOT [lineCnt]._qty [i];
			itloRec.qty				=	LOT [lineCnt]._qty [i];
			itloRec.l_cnv_fct		=	LOT [lineCnt]._l_cnv_fct [i];
			itloRec.i_cnv_fct		=	LOT [lineCnt]._i_cnv_fct [i];
			itloRec.expiry_date		=	StringToDate (LOT [lineCnt]._date [i]);
			itloRec.date_create		=	LOT [lineCnt]._date_create [i];
			strcpy (itloRec.location, 	LOT [lineCnt]._loc [i]);
			strcpy (itloRec.lot_no, 	LOT [lineCnt]._lot [i]);
			strcpy (itloRec.loc_type, 	LOT [lineCnt]._loc_type [i]);
			strcpy (itloRec.slot_no, 	LOT [lineCnt]._slot [i]);
			strcpy (itloRec.l_uom, 		LOT [lineCnt]._l_uom [i]);
			strcpy (itloRec.i_uom, 		LOT [lineCnt]._i_uom [i]);
			strcpy (itloRec.loc_status, LOT [lineCnt]._locationStat	[i]);
			itloRec.pack_qty		=	LOT [lineCnt]._packQuantity	[i];
			itloRec.chg_wgt			=	LOT [lineCnt]._chargeWeight	[i];
			itloRec.gross_wgt		=	LOT [lineCnt]._grossWeight 	[i];
			itloRec.cu_metre		=	LOT [lineCnt]._cubicMetre  	[i];
			itloRec.sknd_hash		=	LOT [lineCnt]._skndHash  	[i];

			abc_unlock (itlo);
			cc = abc_add (itlo, &itloRec);
			if (cc)
				file_err (cc, itlo, "DBADD");
		}
		else
		{
			itloRec.itff_hash		=	itffHash,
			itloRec.pr_line			=	lineCnt;
			itloRec.line_no			=	i;
			itloRec.hhwh_hash		=	LOT [lineCnt]._hhwhHash [i];
			itloRec.hhum_hash		=	LOT [lineCnt]._hhumHash [i];
			itloRec.hhcc_hash		=	LOT [lineCnt]._hhccHash [i];
			itloRec.inlo_hash		=	LOT [lineCnt]._inloHash [i];
			itloRec.sknd_hash		=	LOT [lineCnt]._skndHash [i];
			itloRec.qty_avail		=	LOT [lineCnt]._qty_avail [i];
			itloRec.qty				+=	LOT [lineCnt]._qty [i];
			itloRec.l_cnv_fct		=	LOT [lineCnt]._l_cnv_fct [i];
			itloRec.i_cnv_fct		=	LOT [lineCnt]._i_cnv_fct [i];
			itloRec.expiry_date		=	StringToDate (LOT [lineCnt]._date [i]);
			itloRec.date_create		=	LOT [lineCnt]._date_create [i];
			strcpy (itloRec.location, 	LOT [lineCnt]._loc [i]);
			strcpy (itloRec.lot_no, 	LOT [lineCnt]._lot [i]);
			strcpy (itloRec.loc_type, 	LOT [lineCnt]._loc_type [i]);
			strcpy (itloRec.slot_no, 	LOT [lineCnt]._slot [i]);
			strcpy (itloRec.l_uom, 		LOT [lineCnt]._l_uom [i]);
			strcpy (itloRec.i_uom, 		LOT [lineCnt]._i_uom [i]);
			strcpy (itloRec.loc_status, LOT [lineCnt]._locationStat	[i]);
			itloRec.pack_qty		=	LOT [lineCnt]._packQuantity	[i];
			itloRec.chg_wgt			=	LOT [lineCnt]._chargeWeight	[i];
			itloRec.gross_wgt		=	LOT [lineCnt]._grossWeight 	[i];
			itloRec.cu_metre		=	LOT [lineCnt]._cubicMetre  	[i];
			itloRec.sknd_hash		=	LOT [lineCnt]._skndHash  	[i];

			cc = abc_update (itlo, &itloRec);
			if (cc)
				file_err (cc, itlo, "DBADD");
		}
	}
	abc_fclose (itlo);
}
/*
 * Move lot line up by one, used for Deletes.
 */
void
LotMove (
	int		To, 
	int		From)
{
	if (!ArrChkLimit (&lots_d, LOT, (From > To) ? From + 1 : To + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	memcpy ((char *) &LOT [To], (char *) &LOT [From], sizeof (struct tag_lot));
}
/*
 * Clear lot values.
 */
void
LotClear (
	int		lineCnt)
{
	if (!ArrChkLimit (&lots_d, LOT, lineCnt + 1))
		sys_err ("ArrChkLimit (LOTS)", ENOMEM, PNAME);

	memset ((char *) &LOT [lineCnt], 0, sizeof (struct tag_lot));
}

/*
 * Write Transfer lots and locations to itlo file.
 */
void
AllocLotLocation (
	int		lineCnt,
	int		IsInvoice,
	int		transferType,
	long	transferHash)
{
	int		i;
	int		NewInla;
	float	allocQuantity	=	0.00;

	/*
	 * Delete PID allocations for all. 
	 */
	if (!lineCnt)
		DeletePidAllocation (LotPID);

	if (transferHash == 0L)
		return;

	memset (&inla2Rec, 0, sizeof (inla2Rec));

	switch (transferType)
	{
		/*
	 	 * Invoice and Credit note from coln_hhcl_hash
	 	 */
		case	LL_LOAD_INV:
		case	LL_LOAD_CRD:
				SelectIndex (select_inla_hhcl_id);
				inla2Rec.hhcl_hash	=	transferHash;
				break;
		/*
	  	 * Sales order entry from soln_hhsl_hash  
	 	 */
		case	LL_LOAD_SO:
				SelectIndex (select_inla_hhsl_id);
				inla2Rec.hhsl_hash	=	transferHash;
				break;
		/*
	 	 * Stock transfers from itff_itff_hash
	 	 */
		case	LL_LOAD_TRN:
				SelectIndex (select_inla_itff_id);
				inla2Rec.itff_hash	=	transferHash;
				break;
		/*
	  	 * Manufacturing from pcms_pcms_hash
	 	 */
		case	LL_LOAD_MS:
				SelectIndex (select_inla_pcms_id);
				inla2Rec.pcms_hash	=	transferHash;
				break;
		/*
	  	 * Load goods receipt allocated line from pogl_hhgl_hash
	 	 */
		case	LL_LOAD_GL:
				SelectIndex (select_inla_hhgl_id);
				inla2Rec.hhgl_hash	=	transferHash;
				break;
		/*
	  	 * Contract management from cmrd_cmrd_hash
	 	 */
		case	LL_LOAD_CM:
				SelectIndex (select_inla_cmrd_id);
				inla2Rec.cmrd_hash	=	transferHash;
				break;

		default :
			print_mess ("Invalid transfer type passed to AllocLotLocation");
			sleep (10);
			return;
	}
	
	for (i = 0; i < MAX_LOTS; i++)
	{
		if (LOT [lineCnt]._hhwhHash [i]	==	0L)
			break;

		allocQuantity	=	LOT [lineCnt]._qty [i] * 
							LOT [lineCnt]._i_cnv_fct [i];

		LOT [lineCnt]._inloHash [i]	=	GetINLO (lineCnt, i);
		inla2Rec.inlo_hash				=	LOT [lineCnt]._inloHash [i];

		NewInla = find_rec (inla2, &inla2Rec, COMPARISON, "u");

		if (LOT [lineCnt]._qty [i]	==	0.00 && NewInla)
			continue;

		inla2Rec.inlo_hash	=	LOT [lineCnt]._inloHash [i];
		inla2Rec.pid		=	0L;
		inla2Rec.line_no	=	0;
		inla2Rec.qty_alloc	= (IsInvoice) ? allocQuantity : allocQuantity * -1;
		if (NewInla)
		{
			abc_unlock (inla2);
			cc = abc_add (inla2, &inla2Rec);
			if (cc)
				file_err (cc, inla2, "DBADD");
		}
		else
		{
			cc = abc_update (inla2, &inla2Rec);
			if (cc)
				file_err (cc, inla2, "DBADD");
		}
	}
}
/*
 * Delete Allocation by PID. 
 */
void
DeletePidAllocation (
	pid_t	LotPID)
{
	SelectIndex (select_inla_pid);

	inla2Rec.pid		=	(long)	LotPID;
	cc = find_rec (inla2, &inla2Rec, GTEQ, "u");
	while (!cc && inla2Rec.pid == (long)	LotPID)
	{
		abc_delete (inla2);

		inla2Rec.pid		=	(long)	LotPID;
		cc = find_rec (inla2, &inla2Rec, GTEQ, "u");
	}
	abc_unlock (inla2);
}
/*
 * Write Transfer lots and locations to itlo file. 
 */
void
UpdateItlo (
	int		lineCnt,
	long	hhwhHash,
	long	itffHash)
{
	int		i;

	open_rec (itlo, itlo_list, ITLO_NO_FIELDS, "itlo_id_no");

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (LOT [lineCnt]._hhwhHash [i]	==	0L)
			break;

		if (LOT [lineCnt]._qty [i]	==	0.00)
			continue;

		itloRec.itff_hash	=	itffHash;
		itloRec.pr_line		=	lineCnt;
		itloRec.line_no		=	i;

		cc = find_rec (itlo, &itloRec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (itlo);
			continue;
		}

		itloRec.qty		-=		LOT [lineCnt]._qty [i];
		if (itloRec.qty <= 0.00)
		{
			abc_unlock (itlo);
			cc = abc_delete (itlo);
			if (cc)
				file_err (cc, itlo, "DBDELETE");
		}
		else
		{
			cc = abc_update (itlo, &itloRec);
			if (cc)
				file_err (cc, itlo, "DBADD");
		}
		LOT [lineCnt]._hhwhHash [i] = hhwhHash;
	}
	abc_fclose (itlo);
	UpdateLotLocation (lineCnt, FALSE);
}

/*
 * Load transfer lines from itlo file.
 */
int
LoadTrans (
	int		lineCnt,
	long	itffHash,
	long	hhwhHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	int		_Silent)
{
	int		OneLocation	=	FALSE;

	qtyAllocated	=	0.00;
	qtyRequired		=	0.00;

	ClearLotsInfo (lineCnt);

	open_rec (itlo, itlo_list, ITLO_NO_FIELDS, "itlo_id_no");

	/*
	 * Load records from itlo file.
	 */
	itloRec.itff_hash	=	itffHash;
	itloRec.pr_line		=	lineCnt;
	itloRec.line_no		=	0;
	cc = find_rec (itlo, &itloRec, GTEQ, "r");
	while (!cc && itloRec.itff_hash == itffHash && 	itloRec.pr_line == lineCnt)
	{
		/*
		 * If the Receipt hhccHash equals the origional hhcc_hash then it	 
		 * means that the product is being receipted back into the same place.
		 */
		if (hhccHash != itloRec.hhcc_hash)
		{
			cc = FindLocation 
			(
				hhwhHash,
				itloRec.hhum_hash, 
				NULL, 
				ReceiptLocation,
				&dHash
			);
			if (cc)
			{
				cc = FindLocation 
				(
					hhwhHash,
					0L,
					NULL, 
					ReceiptLocation,
					&dHash
				);
			}
			if (!cc)	
			{
				strcpy (itloRec.location, inloRec.location);
				strcpy (itloRec.loc_type, inloRec.loc_type);
			}
		}
		
		/*
		 * Silent mode so add record showing nothing on screen.
		 */
		if (_Silent)
		{
			strcpy (LLExpiryDate, DateToString (itloRec.expiry_date));
			LLAddList
			(
				hhwhHash,
				itloRec.hhum_hash,
				hhccHash,
				inloRec.inlo_hash,
				(OneLocation) ? llctRec.only_loc : itloRec.location,
				itloRec.lot_no,
				itloRec.loc_type,
				itloRec.slot_no,
				LLExpiryDate,
				itloRec.l_uom,
				itloRec.qty_avail,
				itloRec.i_uom,
				itloRec.qty,
				itloRec.l_cnv_fct,
				itloRec.i_cnv_fct,
				itloRec.date_create,
				itloRec.loc_status,
				itloRec.pack_qty,
				itloRec.chg_wgt,
				itloRec.gross_wgt,
				itloRec.cu_metre,
				itloRec.sknd_hash
			);
		}
		else
		{
			strcpy (LLExpiryDate, DateToString (itloRec.expiry_date));
			sprintf (LLWorkQty [0], LLQtyMask, itloRec.qty_avail);
			sprintf (LLWorkQty [1], LLQtyMask, itloRec.qty);
			tab_add
			(
				"LL_lst", TAB_MASK,
				(OneLocation) ? llctRec.only_loc : itloRec.location,
				itloRec.lot_no,
				itloRec.loc_type,
				itloRec.slot_no,
				LLExpiryDate,
				itloRec.l_uom,
				LLWorkQty [0],
				itloRec.i_uom,
				LLWorkQty [1],
				itloRec.l_cnv_fct,
				itloRec.i_cnv_fct,
				hhwhHash,
				itloRec.hhum_hash,
				hhccHash,
				itloRec.date_create,
				itloRec.inlo_hash,
				itloRec.loc_status,
				itloRec.pack_qty,
				itloRec.chg_wgt,
				itloRec.gross_wgt,
				itloRec.cu_metre,
				itloRec.sknd_hash
			);
		}
		qtyAllocated	+=	itloRec.qty / itloRec.i_cnv_fct;
		qtyRequired		+=	itloRec.qty / itloRec.i_cnv_fct;
		no_LL++;
		cc = find_rec (itlo, &itloRec, NEXT, "r");
	}
	return (!no_LL);
}
/*
 * Load allocation from a number plate.
 */
int	
LoadLocation (
	int		lineCnt,
	long	skndHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	float	qtyRemain)
{
	qtyAllocated	=	0.00;
	qtyRequired		=	0.00;

	ClearLotsInfo (lineCnt);

	abc_selfield (inlo, "inlo_sknd_hash");

	inloRec.sknd_hash	=	skndHash;
	cc = find_rec (inlo, &inloRec, GTEQ, "r");
	if (cc || inloRec.sknd_hash != skndHash)
		return (cc);

	while (!cc && inloRec.sknd_hash	== skndHash)
	{
		if (!LOC_AVAILABLE)
		{
			cc = find_rec (inlo, &inloRec, NEXT, "r");
			continue;
		}
		/*
		 * This caters for the problem when a location is split into two
		 * although only one number plate line remains.
		 */
		if (inloRec.qty < qtyRemain)
			qtyRemain = inloRec.qty;

		/*
		 * Silent mode so add record showing nothing on screen.
		 */
		strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
		LLAddList
		(
			inloRec.hhwh_hash,
			inloRec.hhum_hash,
			hhccHash,
			inloRec.inlo_hash,
			inloRec.location,
			inloRec.lot_no,
			inloRec.loc_type,
			inloRec.slot_no,
			LLExpiryDate,
			inloRec.uom,
			inloRec.qty / inloRec.cnv_fct,
			UOM,
			qtyRemain / conversionFactor,
			inloRec.cnv_fct,
			conversionFactor,
			inloRec.date_create,
			inloRec.loc_status,
			inloRec.pack_qty,
			inloRec.chg_wgt,
			inloRec.gross_wgt,
			inloRec.cu_metre,
			inloRec.sknd_hash
		);
		qtyAllocated	+=	qtyRemain / conversionFactor;
		qtyRequired		+=	qtyRemain / conversionFactor;
		no_LL++;
		cc = find_rec (inlo, &inloRec, NEXT, "r");
	}

	abc_selfield (inlo, "inlo_mst_id");
	ProcList (lineCnt, (float) 0.00);

	return (EXIT_SUCCESS);
}
/*
 * Load transfer lines from itlo file. 
 */
int	
Load_LL_Lines (
	int		lineCnt,
	int		transferType,
	long	transferHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	int		_Silent)
{
	int		NoAllocation	=	TRUE;

	qtyAllocated	=	0.00;
	qtyRequired	=	0.00;

	ClearLotsInfo (lineCnt);

	switch (transferType)
	{
		/*
	 	 * Invoice and Credit note from coln_hhcl_hash
	 	 */
		case	LL_LOAD_INV:
		case	LL_LOAD_CRD:
				SelectIndex (select_inla_hhcl_id);
				inla2Rec.hhcl_hash	=	transferHash;
				break;
		/*
	  	 * Sales order entry from soln_hhsl_hash  
	 	 */
		case	LL_LOAD_SO:
				SelectIndex (select_inla_hhsl_id);
				inla2Rec.hhsl_hash	=	transferHash;
				break;
		/*
	 	 * Stock transfers from itff_itff_hash
	 	 */
		case	LL_LOAD_TRN:
				SelectIndex (select_inla_itff_id);
				inla2Rec.itff_hash	=	transferHash;
				break;
		/*
	  	 * Load goods receipt allocated line from pogl_hhgl_hash
	 	 */
		case	LL_LOAD_GL:
				SelectIndex (select_inla_hhgl_id);
				inla2Rec.hhgl_hash	=	transferHash;
				break;
		/*
	  	 * Manufacturing from pcms_pcms_hash
	 	 */
		case	LL_LOAD_MS:
				SelectIndex (select_inla_pcms_id);
				inla2Rec.pcms_hash	=	transferHash;
				break;
		/*
	  	 * Contract management from cmrd_cmrd_hash
	 	 */
		case	LL_LOAD_CM:
				SelectIndex (select_inla_cmrd_id);
				inla2Rec.cmrd_hash	=	transferHash;
				break;

		default :
			print_mess ("Invalid transfer type passed to AllocLotLocation");
			sleep (10);
			return (EXIT_FAILURE);
	}

	abc_selfield (inlo, "inlo_inlo_hash");

	inla2Rec.inlo_hash	=	0L;

	cc = find_rec (inla2, &inla2Rec, GTEQ, "u");
	while (!cc)
	{
		if (transferType == LL_LOAD_INV && inla2Rec.hhcl_hash != transferHash)
			break;

		if (transferType == LL_LOAD_CRD && inla2Rec.hhcl_hash != transferHash)
			break;

		if (transferType == LL_LOAD_SO && inla2Rec.hhsl_hash != transferHash)
			break;

		if (transferType == LL_LOAD_TRN && inla2Rec.itff_hash != transferHash)
			break;

		if (transferType == LL_LOAD_MS && inla2Rec.pcms_hash != transferHash)
			break;

		if (transferType == LL_LOAD_GL && inla2Rec.hhgl_hash != transferHash)
			break;

		if (transferType == LL_LOAD_CM && inla2Rec.cmrd_hash != transferHash)
			break;

		inloRec.inlo_hash	=	inla2Rec.inlo_hash;
		cc = find_rec (inlo, &inloRec, COMPARISON, "r");
		if (!cc)
		{
			NoAllocation	=	FALSE;
			/*
			 * Silent mode so add record showing nothing on screen.
			 */
			if (_Silent)
			{
				strcpy (LLExpiryDate, DateToString (itloRec.expiry_date));
				LLAddList
				(
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash,
					inloRec.inlo_hash,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					inloRec.qty / inloRec.cnv_fct,
					UOM,
					inla2Rec.qty_alloc / conversionFactor,
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.date_create,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			else
			{
				strcpy (LLExpiryDate, DateToString (itloRec.expiry_date));
				sprintf 
				(
					LLWorkQty [0], 
					LLQtyMask, 
					inloRec.qty / inloRec.cnv_fct
				);
				sprintf 
				(
					LLWorkQty [1], 
					LLQtyMask, 
					inla2Rec.qty_alloc / conversionFactor
				);
				tab_add
				(
					"LL_lst", TAB_MASK,
					inloRec.location,
					inloRec.lot_no,
					inloRec.loc_type,
					inloRec.slot_no,
					LLExpiryDate,
					inloRec.uom,
					LLWorkQty [0],
					UOM,
					LLWorkQty [1],
					inloRec.cnv_fct,
					conversionFactor,
					inloRec.hhwh_hash,
					inloRec.hhum_hash,
					hhccHash, 
					inloRec.date_create,
					inloRec.inlo_hash,
					inloRec.loc_status,
					inloRec.pack_qty,
					inloRec.chg_wgt,
					inloRec.gross_wgt,
					inloRec.cu_metre,
					inloRec.sknd_hash
				);
			}
			qtyAllocated	+=	inla2Rec.qty_alloc / conversionFactor;
			qtyRequired	+=	inla2Rec.qty_alloc / conversionFactor;
			no_LL++;
		}

		AllocationStore (&inla2Rec);

		abc_delete (inla2);

		if (transferType ==	LL_LOAD_INV || transferType ==	LL_LOAD_CRD)
			inla2Rec.hhcl_hash	=	transferHash;
		else if (transferType	==	LL_LOAD_TRN)
			inla2Rec.itff_hash	=	transferHash;
		else if (transferType	==	LL_LOAD_CM)
			inla2Rec.cmrd_hash	=	transferHash;
		else if (transferType	==	LL_LOAD_SO)
			inla2Rec.hhsl_hash	=	transferHash;
		else if (transferType	==	LL_LOAD_MS)
			inla2Rec.pcms_hash	=	transferHash;
		else if (transferType	==	LL_LOAD_GL)
			inla2Rec.hhgl_hash	=	transferHash;

		cc = find_rec (inla2, &inla2Rec, GTEQ, "u");
	}
	abc_unlock (inla2);
	if (NoAllocation)
		return (EXIT_FAILURE);

	if (_Silent)
		ProcList (lineCnt, (float) 0.00);

	return (EXIT_SUCCESS);
}
char *
DefaultLocation (
	long	hhwhHash)
{
	if (!SK_DFLT_LOC)
		return (llctRec.dflt_loc);

	inccRec.hhwh_hash	=	hhwhHash;
	cc = find_rec (incc, &inccRec, COMPARISON, "r");
	if (cc)
		return (llctRec.dflt_loc);

	if (!strcmp (inccRec.location, locationPad))
		return (llctRec.dflt_loc);

	return (inccRec.location);
}

/*
 * Select index for allocation file (inla).
 */
void
SelectIndex (
	int		indexNumber)
{
	if (indexNumber == currentInlaIndex)
		return;

	switch (indexNumber)
	{
		/*
		 * Invoice and Credit note from coln_hhcl_hash
		 */
		case	select_inla_hhcl_id:
				abc_selfield (inla2, "inla_hhcl_id");
		break;

		/*
		 * Sales order entry from soln_hhsl_hash  
		 */
		case 	select_inla_hhsl_id:
				abc_selfield (inla2, "inla_hhsl_id");
		break;

		/*
	 	 * Stock transfers from itff_itff_hash
	 	 */
		case	select_inla_itff_id:
				abc_selfield (inla2, "inla_itff_id");
		break;

		/*
	 	 * Contract management from cmrd_cmrd_hash
	 	 */
		case	select_inla_cmrd_id:
				abc_selfield (inla2, "inla_cmrd_id");
		break;

		/*
	  	 * Load goods receipt allocated line from pogl_hhgl_hash
	 	 */
		case	select_inla_hhgl_id:
				abc_selfield (inla2, "inla_hhgl_id");
		break;

		/*
	  	 * Manufacturing from pcms_pcms_hash
	 	 */
		case	select_inla_pcms_id:
				abc_selfield (inla2, "inla_pcms_id");
		break;

		/*
	  	 * Select process ID index.
	 	 */
		case	select_inla_pid:
				abc_selfield (inla2, "inla_pid");
		break;
	}
	currentInlaIndex	=	indexNumber;
}

/*
 * Generate next 3pl number.
 */
long
NextThreePlNo (
	long	hhccHash)
{
	llct2Rec.hhcc_hash	=	hhccHash;
	cc = find_rec (llct, &llct2Rec, COMPARISON, "u");
	if (cc)
		file_err (cc, llct, "DBFIND");

	llct2Rec.nx_3pl_no++;

	cc = abc_update (llct, &llct2Rec);
	if (cc)
		file_err (cc, llct, "DBUPDATE");

	return (llct2Rec.nx_3pl_no);
}

/*
 * Put information that is not in display window.
 */
void	
PutNoPlateData (
	int		_lineCnt,
	int		_lineNumber,
	char	*locationStat,
	float	packQuantity,
	float	chargeWeight,
	float	grossWeight,
	float	cubicMetre,
	long	skndHash)
{
	strcpy (LOT [_lineCnt]._locationStat [_lineNumber], locationStat);
	LOT [_lineCnt]._packQuantity [_lineNumber]	=	packQuantity;
	LOT [_lineCnt]._chargeWeight [_lineNumber]	=	chargeWeight;
	LOT [_lineCnt]._grossWeight  [_lineNumber]	=	grossWeight;
	LOT [_lineCnt]._cubicMetre	 [_lineNumber]	=	cubicMetre;
	LOT [_lineCnt]._skndHash	 [_lineNumber]	=	skndHash;
}

/*
 * Store existing allocation.
 */
static	void
AllocationStore (
	INLA_STRUCT	*inla_obj)
{
	if (!allocationCnt)
	{
		allocationOpen	=	TRUE;
		ArrAlloc (&allocation_d, &ALLOCATION,sizeof (struct tag_allocation),10);
	}

	if (!ArrChkLimit (&allocation_d, ALLOCATION, allocationCnt))
		sys_err ("ArrChkLimit (ALLOCATION)", ENOMEM, PNAME);

	ALLOCATION [allocationCnt].inlo_hash	=	inla_obj->inlo_hash;
	ALLOCATION [allocationCnt].pid			=	inla_obj->pid;
	ALLOCATION [allocationCnt].line_no		=	inla_obj->line_no;
	ALLOCATION [allocationCnt].hhcl_hash	=	inla_obj->hhcl_hash;
	ALLOCATION [allocationCnt].hhsl_hash	=	inla_obj->hhsl_hash;
	ALLOCATION [allocationCnt].cmrd_hash	=	inla_obj->cmrd_hash;
	ALLOCATION [allocationCnt].itff_hash	=	inla_obj->itff_hash;
	ALLOCATION [allocationCnt].pcms_hash	=	inla_obj->pcms_hash;
	ALLOCATION [allocationCnt].hhgl_hash	=	inla_obj->hhgl_hash;
	ALLOCATION [allocationCnt].qty_alloc	=	inla_obj->qty_alloc;
	ALLOCATION [allocationCnt].qty_proc		=	inla_obj->qty_proc;
	allocationCnt++;
}
/*
 * Restore allocation as restart must have been used.
 */
void
AllocationRestore (void)
{
	int		i;
	int		NewInla;

	if (!allocationOpen)
		return;

	for (i = 0; i < allocationCnt; i++)
	{
		inla2Rec.inlo_hash	=	ALLOCATION [i].inlo_hash;
		inla2Rec.pid		=	ALLOCATION [i].pid;
		inla2Rec.line_no	=	ALLOCATION [i].line_no;
		inla2Rec.hhcl_hash	=	ALLOCATION [i].hhcl_hash;
		inla2Rec.hhsl_hash	=	ALLOCATION [i].hhsl_hash;
		inla2Rec.cmrd_hash	=	ALLOCATION [i].cmrd_hash;
		inla2Rec.itff_hash	=	ALLOCATION [i].itff_hash;
		inla2Rec.pcms_hash	=	ALLOCATION [i].pcms_hash;
		inla2Rec.hhgl_hash	=	ALLOCATION [i].hhgl_hash;
		inla2Rec.qty_alloc	=	ALLOCATION [i].qty_alloc;
		inla2Rec.qty_proc	=	ALLOCATION [i].qty_proc;
		NewInla = find_rec (inla2, &inla2Rec, COMPARISON, "u");
		if (NewInla)
		{
			abc_unlock (inla2);
			cc = abc_add (inla2, &inla2Rec);
			if (cc)
				file_err (cc, inla2, "DBADD");
		}
		else
		{
			cc = abc_update (inla2, &inla2Rec);
			if (cc)
				file_err (cc, inla2, "DBADD");
		}
	}
}
/*
 * Allocation complete (restart not used) so delete array.
 */
void
AllocationComplete (void)
{
	if (!allocationOpen)
		return;

	ArrDelete (&allocation_d);
	allocationCnt	=	0;
}

/*
 * Function to load origional details from po using number plates.
 */
int
PoLoad (
	int		lineCnt,
	long	hhplHash,
	long	hhccHash,
	char	*UOM,
	float	conversionFactor,
	float	returnQty)
{
	qtyAllocated	=	0.00;
	qtyRequired		=	returnQty * conversionFactor;

	abc_selfield (inlo, "inlo_sknd_hash");

	ClearLotsInfo (lineCnt);

	/*
	 * Open number plate lines.
	 */
	if (!poLoadOpen)
	{
		abc_alias (sknd, "sknd");
		open_rec (sknd,  sknd_list, SKND_NO_FIELDS, "sknd_hhpl_hash");
		poLoadOpen	=	1;
	}
	/* 
	 * Read number plate for purchase order line.
	 */
	skndRec.hhpl_hash	=	hhplHash;
	cc = find_rec (sknd, &skndRec, GTEQ, "r");
	while (!cc && skndRec.hhpl_hash ==	hhplHash)
	{
		/* 
		 * Check if number plate deleted. 
		 */
		if (skndRec.status [0] == 'D')
		{
			cc = find_rec (sknd, &skndRec, NEXT, "r");
			continue;
		}
		/*
		 * Open number plate lines.
		 */
		inloRec.sknd_hash	=	skndRec.sknd_hash;
		cc = find_rec (inlo, &inloRec, GTEQ, "r");
		if (cc || inloRec.sknd_hash != skndRec.sknd_hash)
		{
			cc = find_rec (sknd, &skndRec, NEXT, "r");
			continue;
		}
		while (!cc && inloRec.sknd_hash	== skndRec.sknd_hash)
		{
			/*
			 * Continue to read if in stock take mode or quantity > 0
			 */
			if (qtyRequired <= 0.00 && !STOCK_TAKE)
				break;

			if (inloRec.qty > qtyRequired)
			{
				qtyAllocated	=	qtyRequired;
				qtyRequired		=	0.00;
			}
			else
			{
				qtyAllocated	 =	inloRec.qty;
				qtyRequired	 	-= inloRec.qty;
			}

			/*
			 * Silent mode so add record showing nothing on screen.
			 */
			strcpy (LLExpiryDate, DateToString (inloRec.expiry_date));
			LLAddList
			(
				inloRec.hhwh_hash,
				inloRec.hhum_hash,
				hhccHash,
				inloRec.inlo_hash,
				inloRec.location,
				inloRec.lot_no,
				inloRec.loc_type,
				inloRec.slot_no,
				LLExpiryDate,
				inloRec.uom,
				inloRec.qty / inloRec.cnv_fct,
				UOM,
				qtyAllocated / conversionFactor,
				inloRec.cnv_fct,
				conversionFactor,
				inloRec.date_create,
				inloRec.loc_status,
				inloRec.pack_qty,
				inloRec.chg_wgt,
				inloRec.gross_wgt,
				inloRec.cu_metre,
				inloRec.sknd_hash
			);
			no_LL++;
			cc = find_rec (inlo, &inloRec, NEXT, "r");
		}
		cc = find_rec (sknd, &skndRec, NEXT, "r");
		continue;
	}
	qtyAllocated	= (returnQty - qtyRequired) / conversionFactor;
	qtyRequired		= returnQty / conversionFactor;

	abc_selfield (inlo, "inlo_mst_id");

	ProcList (lineCnt, (float) 0.00);

	return (EXIT_SUCCESS);
}

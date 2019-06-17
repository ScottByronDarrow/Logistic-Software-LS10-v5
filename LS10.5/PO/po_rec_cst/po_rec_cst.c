/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_rec_cst.c,v 5.7 2002/07/24 08:39:05 scott Exp $
|  Program Name  : (po_rec_cst.c) 
|  Program Desc  : (Purchase Order Receipt Costing)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/09/86         |
|---------------------------------------------------------------------|
| $Log: po_rec_cst.c,v $
| Revision 5.7  2002/07/24 08:39:05  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/18 07:00:28  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/06/20 07:22:09  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/06/19 07:00:41  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_rec_cst.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_rec_cst/po_rec_cst.c,v 5.7 2002/07/24 08:39:05 scott Exp $";

#define	 	MAXWIDTH	300
#define	 	MAXLINES	2000
#define		SLEEP_TIME	2
extern	int	_win_func;
extern	int	_mail_ok;

#include 	<pslscr.h>
#include 	<ml_po_mess.h>
#include 	<ml_std_mess.h>
#include 	<twodec.h>
#include 	<hot_keys.h>
#include 	<inis_update.h>
#include    <tabdisp.h>

#define		SCN_SELECT	1
#define		SCN_INVOICE	2
#define		SCN_ITEMS	3

#define		STORE		store [line_cnt]
#define		STORE2		store2 [line_cnt]
#define		SR2I		store2 [lcount [SCN_ITEMS]]
#define 	CHECKRATES	 (exchangeRateRef != popc_rec.exch_rate)
#define		CHECKCURR	 (strcmp (currencyRef , popc_rec.currency))

	int		envCrCo 		= 0, 
			envCrFind 		= 0, 
			missingInis		= TRUE, 
			SH_NO 			= FALSE, 
			PO_NO 			= FALSE, 
			GR_NO 			= FALSE, 
			updateInisFlag	= 0, 
			numberTabLines  = 0;

	char	branchNumber [3];

	char	*suin2 = "suin2", 
			*pogh2 = "pogh2", 
			*pogl2 = "pogl2", 
			*posh2 = "posh2";
			
#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct inisRecord	inis_rec;
struct ineiRecord	inei_rec;
struct inumRecord	inum_rec;
struct suinRecord	suin_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct pogdRecord	pogd_rec;
struct poghRecord	pogh_rec;
struct poghRecord	pogh2_rec;
struct poglRecord	pogl_rec;
struct podtRecord	podt_rec;
struct polhRecord	polh_rec;
struct posdRecord	posd_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;
struct popcRecord	popc_rec;

	char	*fifteenSpaces	=	"               ";
	
	char	tempAmount [3] [15];

	int	newPogh;
	int	TAG_DONE;

	float	gstDivide = 0.00;

#define	FOB	0
#define	FRT	1
#define	INS	2
#define	INT	3
#define	B_C	4
#define	DTY	5
#define	O_1	6
#define	O_2	7
#define	O_3	8
#define	O_4	9

#define	GOODS_VAL	 (store [FOB].s_inv_value)
#define	FRT_INS_VAL	 (store [FRT].s_inv_value + store [INS].s_inv_value)
#define	FIN_CHG_VAL	 (store [INT].s_inv_value + store [B_C].s_inv_value)
#define	DUTY_VAL	 (store [DTY].s_inv_value)
#define	OTHER_VAL	 (store [O_1].s_inv_value + store [O_2].s_inv_value +\
			  		  store [O_3].s_inv_value + store [O_4].s_inv_value)

#define	FRT_INS_ERR	 ((double) 2.00)
#define	DUTY_ERR	 ((double) 2.00)
#define	GOODS_ERR	 ((double) 0.00)

	char	poCategoryDesc [10] [21];
	char	*invoiceCategory [] = {
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

	char	*screens [] = {
			" Select ", 
			" Invoice Details ", 
			" Item Details ", 
	};

	char	locPrompt [15];
	char	gstPrompt [15];
	char	ofiPrompt [13];
	char	cifPrompt [13];
	char	ocfPrompt [11];
	char	dtyPrompt [11];
	char	licPrompt [11];
	double	TotalQty = 0.00;

static	int	tag_func (int c, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB pc_keys  [] = 
{
    { " TAG/UNTAG ", 	'T', tag_func, 
	"Tag/Untag current line.", 					"A" }, 
    { " ALL TAG/UNTAG ", 	CTRL ('A'), tag_func, 
	"Tag/Untag All Lines.", 						"A" }, 
    END_KEYS
};
#else
static	KEY_TAB pc_keys  [] = 
{
    { "  [T]AG/UNTAG ", 	'T', tag_func, 
	"Tag/Untag current line.", 					"A" }, 
    { "  [^A]ccept All ", 	CTRL ('A'), tag_func, 
	"Tag/Untag All Lines.", 						"A" }, 
    END_KEYS
};

#endif
struct storeRec {
	char	s_curr [4];
	double	s_exch;
	double	s_fgn_value;		/* invoice value Fgn.			*/
	double	s_inv_value;		/* invoice value (less gst)		*/
	double	s_item_value;		/* extension of items & fob's	*/
	double	s_gst_value;		/* gst on invoice				*/
	int		inv_found;
	long	hhsuHash;			/* supplier hash				*/
	char	s_alloc [2];		/* allocation D / W / V			*/
} store [TABLINES];

struct store2Rec {
	char	duty_type [2];		/* duty rate from podt		*/
	char	std_uom [5];		/* Standard Unit of Measure	*/
	char	sup_uom [5];		/* Supplier Unit of Measure	*/
	char	supplier [7];		/* Supplier number. 		*/
	double	duty_rate;			/* duty rate from podt		*/
	double	fob_fgn_cst;
	double	lic_rate;			/* licence rate from polh	*/
	double	outer;				/* outer size.       		*/
	double	quantity;			/* quantity received		*/
	double	s_exch;
	double	val_duty;			/* value of duty			*/
	double	val_fob_fgn;		/* value of goods			*/
	double	val_fob_gross;
	double	val_frt_ins;		/* value of frt & ins		*/
	double	volume;				/* inis_volume				*/
	double	weight;				/* inis_weight				*/
	float	pur_conv;			/* SupUOM to STD_UOM cfactor*/
	int		haveInis;
	int		upd_inis;
	long	hhbrHash;			/* hhbrHash for line		*/
} store2 [MAXLINES];

/*
 * Local & Screen Structures 
 */
struct {
	char	allocation [2];
	char	category [21];
	char	complete [7];
	char	crd_name [41];
	char	crd_no [7];
	char	curr_code [4];
	char	currency [4];
	char	dummy [11];
	char	grinNo 	 [sizeof pogh_rec.gr_no];
	char	inv_no [16];
	char	item_no [17];
	char	pOrderNo [sizeof pogh_rec.pur_ord_no];
	char	prevGrinNo [sizeof pogh_rec.gr_no];
	char	prev_crd_no [7];
	char	ship_no [13];
	char	supplier [7];
	char	systemDate [11];
	double	cif_loc;	/* cost insurance freight in $NZ	*/
	double	curr_rate;
	double	cust_rate;
	double	duty_pc;
	double	duty_val;
	double	exch_rate;
	double	fgn_val;
	double	fob_fgn;
	double	fob_gross;
	double	lic_pc;
	double	lic_val;
	double	loc_gst;
	double	loc_val;
	double	os_fi_loc;	/* overseas freight & insurance in $NZ	*/
	double	os_fin_chg;	/* overseas finance charges		*/
	double	other;
	double	scn_fob_fgn;
	double	scn_fob_gross;
	double	unit_cst;
	float	qty_rec;
	long	hhgrHash;
	long	hhpoHash;
	long	hhshHash;
	long	hhsuHash;
} local_rec;

static	struct	var	vars [] =
{
	{SCN_SELECT, LIN, "ship_no", 	 4, 20, CHARTYPE, 
		"UUUUUUUUUUUU", "          ", 
		" ", "0", "Shipment No.", "<return> if costing is not by shipment. ", 
		 NE,NO,  JUSTLEFT, "", "", local_rec.ship_no}, 
	{SCN_SELECT, LIN, "pOrderNo", 	 5, 20, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "0", "Purchase Order No.", "<return> if costing is not by Purchase order. ", 
		 NE,NO,  JUSTLEFT, "", "", local_rec.pOrderNo}, 
	{SCN_SELECT, LIN, "grinNo", 	 6, 20, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", local_rec.prevGrinNo, "Goods Received No.", " ", 
		 NE,NO,  JUSTLEFT, "", "", local_rec.grinNo}, 
	{SCN_SELECT, LIN, "curr_code", 	 8, 20, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Currency.", " ", 
		 NA,NO,  JUSTLEFT, "", "", local_rec.curr_code}, 
	{SCN_SELECT, LIN, "curr_rate", 	 8, 60, DOUBLETYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "0.00", "Conv. Rate.", " ", 
		YES,NO,  JUSTLEFT, "", "", (char *)&local_rec.curr_rate}, 
	{SCN_SELECT, LIN, "cust_rate", 	 9, 20, DOUBLETYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "0", "Customs Conv. Rate", " ", 
		YES,NO,  JUSTLEFT, "", "", (char *)&local_rec.cust_rate}, 
	{SCN_SELECT, LIN, "complete", 	 10, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Complete Costing", " Does this Complete the costing ", 
		YES,NO,  JUSTLEFT, "YN", "", local_rec.complete}, 
	{SCN_INVOICE, TAB, "category", 	TABLINES, 1, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "        Category      ", " ", 
		 NA,NO,  JUSTLEFT, "", "", local_rec.category}, 
	{SCN_INVOICE, TAB, "supplier", 	 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier", " ", 
		YES,NO,  JUSTLEFT, "", "", local_rec.supplier}, 
	{SCN_INVOICE, TAB, "allocation", 	 0, 3, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Spread", " by : D(ollar  W(eight  V(olume ", 
		YES,NO,  JUSTLEFT, "DWV", "", local_rec.allocation}, 
	{SCN_INVOICE, TAB, "inv_no", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "  Invoice No   ", " ", 
		YES,NO,  JUSTLEFT, "", "", local_rec.inv_no}, 
	{SCN_INVOICE, TAB, "currency", 	 0, 3, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Curr Code", " ", 
		YES,NO,  JUSTLEFT, "", "", local_rec.currency}, 
	{SCN_INVOICE, TAB, "fgn_val", 	 0, 1, DOUBLETYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", " Foreign Value ", " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.fgn_val}, 
	{SCN_INVOICE, TAB, "exch_rate", 	 0, 0, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", "0", " Exch. Rate  ", " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.exch_rate}, 
	{SCN_INVOICE, TAB, "loc_val", 	 0, 0, DOUBLETYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", locPrompt, " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_val}, 
	{SCN_INVOICE, TAB, "loc_gst", 	 0, 0, DOUBLETYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", gstPrompt, " ", 
		ND,NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_gst}, 
	{SCN_INVOICE, TAB, "hhgrHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhgrHash}, 
	{SCN_INVOICE, TAB, "hhshHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhshHash}, 
	{SCN_INVOICE, TAB, "hhpoHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhpoHash}, 
	{SCN_ITEMS, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "  Item  Number  ", " ", 
		 NA,NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{SCN_ITEMS, TAB, "qty_rec", 	 0, 0, FLOATTYPE, 
		"NNNNNNN", "          ", 
		" ", "0", "Qty Rec", " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_rec}, 
	{SCN_ITEMS, TAB, "fob_gross", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNNNN.NNN", "          ", 
		" ", "0", "  FOB GROSS  ", " ", 
		ND,NO, JUSTRIGHT, "", "", (char *)&local_rec.fob_gross}, 
	{SCN_ITEMS, TAB, "scn_fob_gross", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNNNN.NNN", "          ", 
		" ", "0", "SUP FOB GROSS ", " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.scn_fob_gross}, 
	{SCN_ITEMS, TAB, "scn_fob_fgn", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNNNN.NNN", "          ", 
		" ", "0", "  NET FOB FGN ", " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.scn_fob_fgn}, 
	{SCN_ITEMS, TAB, "fob_fgn", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NNN", "          ", 
		" ", "0", "FOB(FOREIGN)", " ", 
		ND,NO, JUSTRIGHT, "", "", (char *)&local_rec.fob_fgn}, 
	{SCN_ITEMS, TAB, "os_fi_loc", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "0", ofiPrompt, " ", 
		NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.os_fi_loc}, 
	{SCN_ITEMS, TAB, "cif_loc", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "", cifPrompt, " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.cif_loc}, 
	{SCN_ITEMS, TAB, "os_fin_chg", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "", ocfPrompt, " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.os_fin_chg}, 
	{SCN_ITEMS, TAB, "duty_pc", 	 0, 0, DOUBLETYPE, 
		"NNNN.N", "          ", 
		" ", "0", "Duty %", " ", 
		YES,NO, JUSTRIGHT, "", "", (char *)&local_rec.duty_pc}, 
	{SCN_ITEMS, TAB, "duty_val", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "", dtyPrompt, " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.duty_val}, 
	{SCN_ITEMS, TAB, "lic_pc", 	 0, 0, DOUBLETYPE, 
		"NNNN.N", "          ", 
		" ", "0", " Lic% ", " ", 
		 ND,NO, JUSTRIGHT, "", "", (char *)&local_rec.lic_pc}, 
	{SCN_ITEMS, TAB, "lic_val", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", licPrompt, " ", 
		 ND,NO, JUSTRIGHT, "", "", (char *)&local_rec.lic_val}, 
	{SCN_ITEMS, TAB, "other", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "", "  Other.  ", " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.other}, 
	{SCN_ITEMS, TAB, "unit_cst", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "", " Unit Cost. ", " ", 
		 NA,NO, JUSTRIGHT, "", "", (char *)&local_rec.unit_cst}, 
	{SCN_ITEMS, TAB, "hhsuHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&local_rec.hhsuHash}, 
	{SCN_ITEMS, TAB, "hhglHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&pogl_rec.hhgl_hash}, 
	{SCN_ITEMS, TAB, "hhgrHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&pogl_rec.hhgr_hash}, 
	{SCN_ITEMS, TAB, "line_no", 	 0, 0, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "", " ", 
		 ND,NO,  JUSTRIGHT, "", "", (char *)&pogl_rec.line_no}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES,NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <FindSumr.h>

/*
 * Function Declarations 
 */
double 	DollarTotal 			 (void);
double 	DutyTotal 				 (void);
double 	FobVariance 			 (void);
double 	FrtVariance 			 (void);
double 	VolumeTotal 			 (void);
double 	WeightCalc 				 (int, int, double, double, double, double);
double 	WeightTotal 			 (void);
int 	FindPocr 				 (char *);
int 	FindPohr 				 (void);
int 	PrintDuty 				 (int);
int 	PrintFreightInsurance 	 (int);
int 	PrintGoods 				 (int);
int 	heading 				 (int);
int 	spec_valid 				 (int);
int 	win_function 			 (int, int, int, int);
char	get_buf[200];
static 	int CheckInvoice 		 (char *);
static 	int ErrorPogh 			 (void);
static 	int SaveMissing 		 (long);
static 	int tag_func 			 (int c, KEY_TAB *);
static 	long hhsuGetSupplier 	 (long);
int     MoneyZero              	 (double);
void 	AutoSpreadLines 		 (void);
void 	CloseDB 				 (void);
void 	DisplayScreenStuff 		 (int);
void 	GoodsReceiptAllocation 	 (long);
void 	LoadCategoryDescription  (void);
void 	LoadGoodsReceipt 		 (long, char *, int, long, long, long, double);
void 	LoadInvoiceScreen 		 (void);
void 	LoadPopc 				 (void);
void 	OpenAllocation 			 (void);
void 	OpenDB 					 (void);
void 	OpenMissing 			 (char *);
void 	PrintCompany 			 (void);
void 	PrintHeading 			 (int);
void 	PrintValue 				 (void);
void 	PurchaseAllocation 		 (long);
void 	ReadMisc 				 (void);
void 	ShipmentAllocation 		 (long);
void 	ShowMissing 			 (void);
void 	SrchPocr 				 (char *);
void 	SrchPogh 				 (char *);
void 	SrchSuin 				 (char *);
void 	StorePopc 				 (void);
void 	Update 					 (void);
void 	UpdateHeader 			 (long);
void 	UpdateInis 				 (double, long);
void 	UpdatePosh 				 (long, int);
void 	UpdateTab 				 (void);
void 	shutdown_prog 			 (void);
void	CheckMultiple			 (void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	int		i;
	int		linesBalance = FALSE;
	char	envGst [2];
	char	envGstTaxName [4];
	char	envCurrCode [4];

	_win_func = TRUE;
	SETUP_SCR (vars);

	
	updateInisFlag = chk_inis ();

	/*
	 * setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_INVOICE, store2, sizeof (struct store2Rec));
	SetSortArray (SCN_ITEMS, store, sizeof (struct storeRec));
#endif
	for (i = 0;i < 3;i++)
		tab_data [i]._desc = screens [i];

	no_edit (SCN_SELECT);

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	/*
	 * open main database files. 
	 */
	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	sprintf (envGst,  		"%-1.1s", get_env ("GST"));
	sprintf (envGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	sprintf (envCurrCode, 	"%-3.3s", get_env ("CURR_CODE"));
	sprintf (locPrompt, 	"  %-3.3s Value  ", envCurrCode);
	sprintf (gstPrompt, 	"  %-3.3s %-3.3s    ", 	envCurrCode, envGstTaxName);
	sprintf (ofiPrompt, 	" OS_FI(%-3.3s)",   	envCurrCode);
	sprintf (cifPrompt, 	" CIF(%-3.3s) ",  		envCurrCode);
	sprintf (ocfPrompt, 	" OS_FC_%-3.3s",    	envCurrCode);
	sprintf (dtyPrompt, 	" Duty %-3.3s ",    	envCurrCode);
	sprintf (licPrompt, 	" Lic. %-3.3s ",    	envCurrCode);

	if (envGst [0] != 'Y')
		FLD ("loc_gst") = ND;

	if (comm_rec.gst_rate != 0.00)	
		gstDivide = ((100 + comm_rec.gst_rate) / comm_rec.gst_rate);
	else
		gstDivide = 0.00;

	clear ();
	swide ();

	/*
	 * Load category descriptions.
	 */
	LoadCategoryDescription ();


	strcpy (local_rec.prevGrinNo, 	"000000000000000");
	strcpy (local_rec.prev_crd_no, 	"000000");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		newPogh = TRUE;
		missingInis = FALSE;

		FLD ("cust_rate") = YES;

		for (i = 0;i < MAXLINES;i++)
		{
			if (i < TABLINES)
			{
				strcpy (store [i].s_curr, "   ");
				store [i].s_exch 		= 0.00;
				store [i].s_inv_value 	= 0.00;
				store [i].s_item_value 	= 0.00;
				store [i].s_gst_value 	= 0.00;
				store [i].inv_found 	= FALSE;
				store [i].hhsuHash 	= 0L;
			}
			store2 [i].quantity 		= 0.00;
			store2 [i].outer 			= 1.00;
			store2 [i].duty_rate 		= 0.00;
			store2 [i].s_exch 			= 0.00;
			strcpy (store2 [i].duty_type, " ");
			store2 [i].lic_rate 		= 0.00;
			store2 [i].fob_fgn_cst 		= 0.00;
			store2 [i].val_fob_fgn 		= 0.00;
			store2 [i].val_frt_ins 		= 0.00;
			store2 [i].val_duty 		= 0.00;
			store2 [i].weight 			= 0.00;
			store2 [i].volume 			= 0.00;
			store2 [i].hhbrHash 		= 0L;
			strcpy (store2 [i].std_uom, "    ");
			strcpy (store2 [i].sup_uom, "    ");
		}
		search_ok	= TRUE;
		init_ok		= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		eoi_ok 		= TRUE;
		init_vars (SCN_SELECT);	
		init_vars (SCN_INVOICE);	
		init_vars (SCN_ITEMS);	
		lcount [SCN_INVOICE] = 0;
		lcount [SCN_ITEMS]   = 0;

		heading (SCN_SELECT);
		entry (SCN_SELECT);
		if (prog_exit || restart)
			continue;

		init_ok = FALSE;
		if (newPogh)
		{
			eoi_ok = 0;
			heading (SCN_INVOICE);
			scn_display (SCN_INVOICE);
			edit (SCN_INVOICE);
			if (restart)
				continue;
			lcount [SCN_INVOICE] = TABLINES;
		}

		do
		{
			edit_all ();
			if (restart)
				break;

			DisplayScreenStuff (FALSE);
			linesBalance = FALSE;

			if (!PrintGoods (FALSE))
			{
				print_mess (ML (mlPoMess196));
				sleep (sleepTime);
				linesBalance = TRUE;
			}

			if (!PrintDuty (FALSE))
			{
				print_mess (ML (mlPoMess197));
				sleep (sleepTime);
				linesBalance = TRUE;
			}

			if (!PrintFreightInsurance (FALSE))
			{
				print_mess (ML (mlPoMess198));
				sleep (sleepTime);
				linesBalance = TRUE;
			}

		} while (missingInis == FALSE && local_rec.complete [0] != 'N' && linesBalance != 0);

		if (missingInis == TRUE || restart)
			continue;

		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	ReadMisc ();

	abc_alias (pogl2, pogl);
	abc_alias (pogh2, pogh);
	abc_alias (suin2, suin);
	abc_alias (posh2, posh);

	open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (suin2, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envCrFind) 
						? "sumr_id_no" : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pogh, pogh_list, POGH_NO_FIELDS, "pogh_id_no2");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (pogl2, pogl_list, POGL_NO_FIELDS, "pogl_hhgl_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (posh2, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_hhlc_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (pogh2, pogh_list, POGH_NO_FIELDS, "pogh_hhgr_hash");
}

/*
 * Close Database Files 
 */
void
CloseDB (void)
{
	abc_fclose (suin);
	abc_fclose (podt);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (pogl);
	abc_fclose (pogh);
	abc_fclose (pocr);
	abc_fclose (pogd);
	abc_fclose (posd);
	abc_fclose (posh);
	abc_fclose (posh2);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (polh);
	abc_fclose (inum);
	abc_fclose (inis);
	abc_fclose (inei);
	abc_fclose (pogh2);
	abc_fclose (suin2);
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");
		
	abc_fclose (comr);
}

int
spec_valid (
 int field)
{
	double	nz_gst = 0.00;

	int	first_flag = TRUE;

	/*
	 * Validate Creditor Number. 
	 */
	if (LCHECK ("crd_no"))
	{
		abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.crd_no, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			
			return (EXIT_FAILURE);
		}

		if (FindPocr (sumr_rec.curr_code))
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		strcpy (local_rec.crd_name, sumr_rec.crd_name);
		strcpy (local_rec.curr_code, pocr_rec.code);
		local_rec.curr_rate = pocr_rec.ex1_factor;
		DSP_FLD ("crd_name");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Shipping Number.  
	 */
	if (LCHECK ("ship_no"))
	{
		SH_NO = FALSE;
		PO_NO = FALSE;
		GR_NO = FALSE;

		if (dflt_used)
			return (EXIT_SUCCESS);

		abc_selfield (pogh, "pogh_sh_id");

		if (SRCH_KEY)
		{
			SH_NO = TRUE;
			SrchPogh (temp_str);
			SH_NO = FALSE;
			return (EXIT_SUCCESS);
		}

		strcpy (posh2_rec.co_no, comm_rec.co_no);
		sprintf (posh2_rec.csm_no, "%-12.12s", zero_pad (local_rec.ship_no, 12));
		cc = find_rec (posh2, &posh2_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsh_hash = posh2_rec.hhsh_hash;
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (ErrorPogh ())
			return (EXIT_FAILURE);

		SH_NO = TRUE;

		abc_selfield (pogd, "pogd_id_no2");

		first_flag = TRUE;

		/*
		 * Process multiple shipments. 
		 */
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsh_hash = posh2_rec.hhsh_hash;
		cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
		while (!cc && !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
				pogh_rec.hhsh_hash == posh2_rec.hhsh_hash)
		{
			LoadGoodsReceipt
			(
				pogh_rec.hhgr_hash, 
				pogh_rec.pur_status, 
				first_flag, 
				pogh_rec.hhsh_hash, 
				pogh_rec.hhsu_hash, 
				pogh_rec.hhpo_hash, 
				pogh_rec.cust_rate 
			);
			
			first_flag = FALSE;
			cc = find_rec (pogh, &pogh_rec,NEXT, "r");
		}
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsh_hash = posh2_rec.hhsh_hash;
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, "pogh_hhsh_hash (%010ld)", pogh_rec.hhsh_hash);
			file_err (cc, err_str, "DBFIND");
		}
		sprintf (local_rec.grinNo, 	"%15.15s", " ");
		sprintf (local_rec.pOrderNo, "%15.15s", " ");

		DSP_FLD ("ship_no");
		DSP_FLD ("pOrderNo");
		DSP_FLD ("grinNo");

		skip_entry = goto_field (field, label ("curr_code"));
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Purchase Order Number.  
	 */
	if (LCHECK ("pOrderNo"))
	{
		SH_NO = FALSE;
		PO_NO = FALSE;
		GR_NO = FALSE;

		if (dflt_used)
			return (EXIT_SUCCESS);

		abc_selfield (pogh, "pogh_po_id");

		if (SRCH_KEY)
		{
			PO_NO = TRUE;
			SrchPogh (temp_str);
			PO_NO = FALSE;
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.pOrderNo, zero_pad (local_rec.pOrderNo, 15));
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		strcpy (pogh_rec.pur_ord_no, local_rec.pOrderNo);
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (ErrorPogh ())
			return (EXIT_FAILURE);

		PO_NO = TRUE;

		abc_selfield (pogd, "pogd_id_no3");

		first_flag = TRUE;

		/*
		 * Process multiple Purchase orders. 
		 */
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		strcpy (pogh_rec.pur_ord_no, local_rec.pOrderNo);
		cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
		while (!cc && !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
			          !strcmp (pogh_rec.pur_ord_no, local_rec.pOrderNo))
		{
			LoadGoodsReceipt
			(
				pogh_rec.hhgr_hash, 
				pogh_rec.pur_status, 
				first_flag, 
				pogh_rec.hhsh_hash, 
				pogh_rec.hhsu_hash, 
				pogh_rec.hhpo_hash, 
				pogh_rec.cust_rate 
			);
			
			first_flag = FALSE;

			cc = find_rec (pogh, &pogh_rec,NEXT, "r");
		}
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		strcpy (pogh_rec.pur_ord_no, local_rec.pOrderNo);
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, "pogh_po_no (%s)", pogh_rec.pur_ord_no);
			file_err (cc, err_str, "DBFIND");
		}

		sprintf (local_rec.grinNo, 	"%15.15s", " ");
		strcpy (local_rec.ship_no, 	" ");

		DSP_FLD ("ship_no");
		DSP_FLD ("pOrderNo");
		DSP_FLD ("grinNo");

		skip_entry = goto_field (field, label ("curr_code"));
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Grin Number.  
	 */
	 
	if (LCHECK ("grinNo"))
	{
		SH_NO = FALSE;
		PO_NO = FALSE;
		GR_NO = FALSE;

		abc_selfield (pogh, "pogh_id_no2");

		if (SRCH_KEY)
		{
			GR_NO = TRUE;
			SrchPogh (temp_str);
			GR_NO = FALSE;
			return (EXIT_SUCCESS);
		}

		strcpy (pogh_rec.co_no, comm_rec.co_no);
		strcpy (pogh_rec.gr_no, local_rec.grinNo);
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess049));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (ErrorPogh ())
			return (EXIT_FAILURE);

		GR_NO = TRUE;

		abc_selfield (pogd, "pogd_id_no");

		LoadGoodsReceipt
		(
			pogh_rec.hhgr_hash, 
			pogh_rec.pur_status, 
			TRUE, 
			pogh_rec.hhsh_hash, 
			pogh_rec.hhsu_hash, 
			pogh_rec.hhpo_hash, 
			pogh_rec.cust_rate 
		);

		sprintf (local_rec.pOrderNo, 	"%15.15s", " ");
		strcpy (local_rec.ship_no, 		" ");

		DSP_FLD ("ship_no");
		DSP_FLD ("pOrderNo");
		DSP_FLD ("grinNo");

		skip_entry = goto_field (field, label ("curr_code"));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr_rate"))
	{
		if (dflt_used)
		{
			local_rec.curr_rate = posh_rec.ex_rate;
			DSP_FLD ("curr_rate");
		}

		if (local_rec.curr_rate == 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("cust_rate"))
	{
		if (dflt_used)
			local_rec.cust_rate = local_rec.curr_rate;

		if (local_rec.cust_rate <= 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Complete Flag	
	 */
	if (LCHECK ("complete"))
	{
		strcpy (local_rec.complete, (local_rec.complete [0] == 'Y') 
						? "Y(es)." : "N(o).");
		DSP_FLD ("complete");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Supplier	
	 */
	if (LCHECK ("supplier"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		sprintf (err_str, "%-6.6s", local_rec.supplier);

		if (prog_status == ENTRY)
			getval (line_cnt);

		sprintf (local_rec.supplier, "%-6.6s", err_str);

		if (dflt_used)
		{
			strcpy (STORE.s_curr, "   ");
			STORE.hhsuHash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.supplier, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (STORE.inv_found)
		{
			suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
			sprintf (suin_rec.inv_no, "%-15.15s", local_rec.inv_no);
			cc = find_rec (suin, &suin_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML ("Cannot change the Supplier."));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
		{
			STORE.hhsuHash = sumr_rec.hhsu_hash;
			strcpy (local_rec.currency, sumr_rec.curr_code);
			DSP_FLD ("currency");

			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, local_rec.currency);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (STORE.s_curr, pocr_rec.code);
			local_rec.exch_rate = pocr_rec.ex1_factor;
			DSP_FLD ("exch_rate");
			DSP_FLD ("fgn_val");

		}
			if (local_rec.exch_rate != 0.00)
				local_rec.loc_val = local_rec.fgn_val / local_rec.exch_rate;
			DSP_FLD ("loc_val");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("allocation"))
	{
		if (line_cnt == FOB)
		{
			strcpy (local_rec.allocation, "D");
			DSP_FLD ("allocation");
		}
		strcpy (STORE.s_alloc, local_rec.allocation);
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate INV_NO	
	 */
	if (LCHECK ("inv_no"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSuin (temp_str);
			return (EXIT_SUCCESS);
		}
			
		if (STORE.hhsuHash != 0L && strcmp (local_rec.inv_no, fifteenSpaces))
		{
			suin_rec.hhsu_hash = STORE.hhsuHash;
			sprintf (suin_rec.inv_no, "%-15.15s", local_rec.inv_no);
			cc = find_rec (suin, &suin_rec, COMPARISON, "r");
			if (cc)
			{
				errmess (ML (mlStdMess115));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (suin_rec.approved [0] != 'Y')
			{
				sprintf (err_str, ML (mlPoMess121), local_rec.inv_no);
				errmess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}		
			STORE.inv_found = TRUE;

			local_rec.loc_val = suin_rec.amt - suin_rec.gst;
			local_rec.fgn_val = suin_rec.amt - suin_rec.gst;
			if (suin_rec.exch_rate != 0)
				local_rec.loc_val /= suin_rec.exch_rate;

			local_rec.loc_val = DOLLARS (local_rec.loc_val);

			local_rec.loc_gst = suin_rec.gst;
			if (suin_rec.exch_rate != 0)
				local_rec.loc_gst /= suin_rec.exch_rate;

			local_rec.loc_gst = DOLLARS (local_rec.loc_gst);

			local_rec.fgn_val = DOLLARS (suin_rec.amt - suin_rec.gst);
			STORE.s_fgn_value = DOLLARS (suin_rec.amt - suin_rec.gst);
			strcpy (local_rec.currency, suin_rec.currency);
			DSP_FLD ("currency");
			DSP_FLD ("fgn_val");
			DSP_FLD ("loc_val");
			DSP_FLD ("loc_gst");
			STORE.s_exch		=	suin_rec.exch_rate;
			local_rec.exch_rate	=	suin_rec.exch_rate;
			DSP_FLD ("exch_rate");
		}
		else
			STORE.inv_found = FALSE;

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Currency	
	 */
	if (LCHECK ("currency"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			strcpy (local_rec.currency, STORE.s_curr);
	
		if (strcmp (local_rec.currency, "   "))
		{
			cc = FindPocr (local_rec.currency);
			if (cc)
			{
				errmess (ML (mlStdMess040));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			local_rec.exch_rate = pocr_rec.ex1_factor;
			STORE.s_exch = pocr_rec.ex1_factor;
		}
		else
		{
			if (!SH_NO || line_cnt != 0)
			{
				local_rec.exch_rate = 1.00;
				STORE.s_exch = 1.00;
			}
		}


		if (STORE.inv_found)
		{
			STORE.s_exch		=	suin_rec.exch_rate;
			local_rec.exch_rate	=	STORE.s_exch;
			DSP_FLD ("exch_rate");

			local_rec.fgn_val = STORE.s_fgn_value;
			DSP_FLD ("exch_rate");
			local_rec.loc_val = local_rec.fgn_val;
			if (STORE.s_exch != 0.00)
				local_rec.loc_val /= STORE.s_exch;
			DSP_FLD ("fgn_val");
			DSP_FLD ("loc_val");

			skip_entry = 4;
		}
		else
		{
			if (prog_status != ENTRY)
			{
				local_rec.loc_val = local_rec.fgn_val;
				local_rec.exch_rate	=	STORE.s_exch;
				if (STORE.s_exch != 0.00)
					local_rec.loc_val /= STORE.s_exch;

				DSP_FLD ("exch_rate");
				DSP_FLD ("loc_val");
			}
		}
		/*
		 * Hold Fgn value for goods	
		 */
		if (line_cnt == 0)
			STORE.s_inv_value = local_rec.fgn_val;
		else
			STORE.s_inv_value = local_rec.loc_val;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate FGN_VAL	
	 */
	if (LCHECK ("fgn_val"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used)
		{
			if (line_cnt == 0)
				local_rec.fgn_val = DollarTotal ();
			if (line_cnt == 5 && store [5].s_alloc [0] == 'D')
				local_rec.fgn_val = DutyTotal ();
		}

		local_rec.loc_val = local_rec.fgn_val;
		if (STORE.s_exch != 0.00)
		{
			local_rec.loc_val /= STORE.s_exch;
			local_rec.exch_rate	=STORE.s_exch;
			DSP_FLD ("exch_rate");
		}

		DSP_FLD ("loc_val");

		/*
		 * Hold Fgn value for goods	
		 */
		if (line_cnt == 0)
			STORE.s_inv_value = local_rec.fgn_val;
		else
			STORE.s_inv_value = local_rec.loc_val;

		if (prog_status != ENTRY && local_rec.loc_gst != 0.00)
		{
			if (gstDivide != 0.00)
				nz_gst = local_rec.loc_val / (double) gstDivide;
			else
				nz_gst = 0.00;

			local_rec.loc_gst = nz_gst;
			DSP_FLD ("loc_gst");
			STORE.s_gst_value = nz_gst;
		}
		DSP_FLD ("fgn_val");
		if (prog_status != ENTRY && STORE.s_exch == 0.00)
        {
            /*
             * Reenter Qty. Ord. 
             */
            do
            {
                get_entry (label ("exch_rate"));
                cc = spec_valid (label ("exch_rate"));
            } while (cc);
			local_rec.loc_val = local_rec.fgn_val;
			if (STORE.s_exch != 0)
				local_rec.loc_val /= STORE.s_exch;
			else
				local_rec.loc_val = 0.00;

			STORE.s_inv_value = local_rec.loc_val;
			if (gstDivide != 0.00)
				nz_gst = local_rec.loc_val / (double) gstDivide;
			else
				nz_gst = 0.00;

			local_rec.loc_gst = nz_gst;
			DSP_FLD ("loc_gst");
			STORE.s_gst_value = nz_gst;
			DSP_FLD ("exch_rate");
        }
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Exchange Rate	
	 */
	if (LCHECK ("exch_rate")) 
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used)
			local_rec.exch_rate = STORE.s_exch;

		if (local_rec.exch_rate == 0.00)
			local_rec.exch_rate = 1.00;
	
		local_rec.loc_val = local_rec.fgn_val;
		if (local_rec.exch_rate != 0.00)
			local_rec.loc_val /= local_rec.exch_rate;

		/*
		 * Hold Fgn value for goods	
		 */
		if (line_cnt == 0)
			STORE.s_inv_value = local_rec.fgn_val;
		else
			STORE.s_inv_value = local_rec.loc_val;

		DSP_FLD ("loc_val");

		if (prog_status != ENTRY && local_rec.loc_gst != 0.00)
		{
			if (gstDivide != 0.00)
				nz_gst = local_rec.loc_val / (double) gstDivide;
			else
				nz_gst = 0.00;

			local_rec.loc_gst = nz_gst;
			DSP_FLD ("loc_gst");
			STORE.s_gst_value = nz_gst;
		}
		if (dflt_used && prog_status == ENTRY)
			skip_entry++;

		STORE.s_exch = local_rec.exch_rate;
		DSP_FLD ("exch_rate");
		return (EXIT_SUCCESS);
	}

	/*
	 * Calculate NZL_VAL	
	 */
	if (LCHECK ("loc_val")) 
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			local_rec.loc_val = local_rec.fgn_val;

			STORE.s_inv_value = local_rec.fgn_val;
			if (STORE.s_exch != 0.00)
				local_rec.loc_val /= STORE.s_exch;

			DSP_FLD ("loc_val");
		}
		else
		{
			local_rec.fgn_val = local_rec.loc_val;
			local_rec.fgn_val *= STORE.s_exch;

			DSP_FLD ("fgn_val");
		}

		/*
		 * Hold Fgn value for goods	
		 */
		if (line_cnt == 0)
			STORE.s_inv_value = local_rec.fgn_val;
		else
			STORE.s_inv_value = local_rec.loc_val;

		if (prog_status != ENTRY && local_rec.loc_gst != 0.00)
		{
			if (gstDivide != 0.00)
				nz_gst = local_rec.loc_val / (double) gstDivide;
			else
				nz_gst = 0.00;

			local_rec.loc_gst = nz_gst;
			DSP_FLD ("loc_gst");
			STORE.s_gst_value = nz_gst;
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Calculate NZL_GST	
	 */
	if (LCHECK ("loc_gst"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used)
		{
			if (gstDivide != 0.00)
				nz_gst = local_rec.loc_val / (double) gstDivide;
			else
				nz_gst = 0.00;

			local_rec.loc_gst = nz_gst;
		}

		if (local_rec.loc_gst != nz_gst && local_rec.loc_gst != 0.00)
		{
			sprintf (err_str, ML (mlPoMess212), comm_rec.gst_rate);
			print_mess (err_str);
			sleep (sleepTime);
		}
		STORE.s_gst_value = nz_gst;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate FOB_GROSS	
	 */
	if (LCHECK ("scn_fob_gross"))
	{
		local_rec.scn_fob_fgn = local_rec.scn_fob_gross;
		local_rec.fob_fgn 	= local_rec.scn_fob_fgn * store2[line_cnt].outer;
		store2[line_cnt].val_fob_fgn 	= local_rec.fob_fgn;
		local_rec.fob_gross = local_rec.scn_fob_gross * store2 [line_cnt].outer;
		STORE2.val_fob_gross = local_rec.fob_gross;

		if (local_rec.fob_gross == 0.00)
		{
			int i;

			i = prmptmsg (ML (mlStdMess031), "YyNn", 1, 23);
			move (1, 23);
			cl_line ();
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}
		if (!STORE2.haveInis)
		{
			move (0, 23);
			cl_line ();
			sprintf (err_str, ML ("NOTE : Item no %s has no inventory supplier record for %s"), local_rec.item_no, STORE2.supplier);
			rv_pr (err_str, 0, 23, 1);
			sleep (sleepTime);
			STORE2.upd_inis = 0;
			move (0, 23);
			cl_line ();
			return (EXIT_SUCCESS);
		}

		if (STORE2.haveInis && STORE2.fob_fgn_cst != STORE2.val_fob_gross)
		{
			if (updateInisFlag == -1)
				STORE2.upd_inis = prmpt_inis (0, 23);
			else
				STORE2.upd_inis = updateInisFlag;
		}
		DisplayScreenStuff (TRUE);
		DSP_FLD ("scn_fob_fgn");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate FOB_FGN	
	 */
	if (LCHECK ("scn_fob_fgn"))
	{
		if (store2  [line_cnt].outer == 0.0)
			store2  [line_cnt].outer = 1.00;

		local_rec.fob_fgn = local_rec.scn_fob_fgn * store2 [line_cnt].outer;
		DisplayScreenStuff (TRUE);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate OS_FI_NZL	
	 */
	if (LCHECK ("os_fi_loc"))
	{
		STORE2.val_frt_ins = local_rec.os_fi_loc;
		DisplayScreenStuff (TRUE);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate DUTY_PC	
	 */
	if (LCHECK ("duty_pc"))
	{
		STORE2.duty_rate = local_rec.duty_pc;
		DisplayScreenStuff (TRUE);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Menu called when CTRL W pressed 
 */
int
win_function (
 int		fld, 
 int		lin, 
 int		scn, 
 int		mode)
{
	int	tmp_line, tmp_p_stat;
	int	hold_mail = _mail_ok;
	char	workTempStr [100];

	if (cur_screen == SCN_ITEMS)
	{
		AutoSpreadLines ();
		DisplayScreenStuff (TRUE);
		return (PSLW);
	}
	if (cur_screen != SCN_INVOICE)
		return (PSLW);

	strcpy	 (workTempStr, temp_str);
	_mail_ok = FALSE;
	_win_func = FALSE;
	
	tmp_line = line_cnt;
	tmp_p_stat = prog_status;

	OpenAllocation (); 
		
	line_cnt = tmp_line;
	strcpy	 (temp_str, workTempStr);
	getval (line_cnt);
	prog_status = tmp_p_stat;

	_mail_ok = hold_mail;
	_win_func = TRUE;

	return (PSLW);
}

/*
 * Routine to display all possible errors on status for goods receipt header. 
 */
static	int
ErrorPogh (void)
{
	if (pogh_rec.drop_ship [0] == 'Y')
	{
		print_mess (ML (mlPoMess199));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	
	if (pogh_rec.pur_status [0] == 'U' || pogh_rec.pur_status [0] == 'P')
			return (EXIT_SUCCESS);

	switch (pogh_rec.pur_status [0])
	{
		case 'C' :
			sprintf (err_str, ML (mlPoMess213), pogh_rec.gr_no);
		break;

		case 'D':
			sprintf (err_str, ML (mlPoMess214), pogh_rec.gr_no);
		break;

		default :
			sprintf (err_str, ML (mlPoMess215), pogh_rec.pur_status);
			break;
	}
	print_mess (err_str);
	sleep (sleepTime);
	return (EXIT_FAILURE);
}

/*
 * Calculate FOB on lines. 
 */
void
AutoSpreadLines (void)
{
	int	i;
	double	OrigFob;
	double	OrigFrt;
	double	PlusFob;
	double	PlusFrt;
	double	FobVar = 0.00, 
			FrtVar = 0.00;
	double	loc_conv;

	TotalQty = 0.00;
	FobVar = FobVariance ();
	FrtVar = FrtVariance ();

	scn_set (SCN_ITEMS);
	/*
	 * Perform a general spread over all the items. 
	 */
	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		getval (i);

		if (store2  [i].outer == 0.0)
			store2  [i].outer = 1.00;

		/*
		 * Calculate spread of FOB based on value of lines. 			 
		 * i.e Larger values get more allocation than smaller values. 
		 */
		OrigFob = out_cost (store2 [i].val_fob_fgn, (float) store2 [i].outer);
		OrigFob *= store2 [i].quantity;

		if (store [0].s_item_value != 0.0)
			PlusFob	= FobVar * (OrigFob / store [FOB].s_item_value);
		else
			PlusFob	= 0.00;
		OrigFob += PlusFob;
		if (store2  [i].quantity != 0.0)
			local_rec.scn_fob_fgn 	= OrigFob / store2 [i].quantity;
		else
			local_rec.scn_fob_fgn 	= 0.00;

		local_rec.fob_fgn 		= local_rec.scn_fob_fgn * store2 [i].outer;
		local_rec.scn_fob_fgn 	= local_rec.fob_fgn / store2 [i].outer;
		store2 [i].val_fob_fgn 	= local_rec.fob_fgn;

		PlusFrt = 0.00;
		if (GOODS_VAL != 0.00)
			PlusFrt = OrigFob / GOODS_VAL;
		PlusFrt *= FRT_INS_VAL;
		PlusFrt = PlusFrt;

		loc_conv = local_rec.fob_fgn;
		if (store [FOB].s_exch != 0.00)
			loc_conv /= store [FOB].s_exch;

		if (store2  [i].quantity != 0.0)
			local_rec.os_fi_loc = PlusFrt / store2 [i].quantity;
		else
			local_rec.os_fi_loc = 0.00;

		store2 [i].val_frt_ins = local_rec.os_fi_loc;
		local_rec.cif_loc = loc_conv + local_rec.os_fi_loc;
	
		putval (i);
	}
	/*
	 * Now try and spread the remaining variance on each line. 
	 */
	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		if (store2 [i].outer == 0.0)
			store2 [i].outer = 1.00;

		getval (i);
		FobVar = FobVariance ();
		FrtVar = FrtVariance ();

		if (fabs (FobVar) == 0.00 && fabs (FrtVar) == 0.00)
			break;
		
		TotalQty = store2 [i].quantity;

		if (fabs (FobVar) != 0.00)
		{
			OrigFob =	out_cost 
						 (
							store2 [i].val_fob_fgn, 
							 (float) store2 [i].outer
						);
			OrigFob *= store2 [i].quantity;
			if (TotalQty != 0.00)
				PlusFob = OrigFob + (FobVar * (store2 [i].quantity / TotalQty));
			else
				PlusFob = 0.0;
			PlusFob = PlusFob;

			if (store2 [i].quantity != 0.00)
				local_rec.scn_fob_fgn 	= PlusFob / store2 [i].quantity;
			else
				local_rec.scn_fob_fgn 	= 0.00;
			local_rec.fob_fgn 		= local_rec.scn_fob_fgn * store2 [i].outer;
			local_rec.fob_fgn 		= local_rec.fob_fgn;
			local_rec.scn_fob_fgn 	= local_rec.fob_fgn / store2 [i].outer;
			local_rec.scn_fob_fgn 	= local_rec.scn_fob_fgn;
			store2 [i].val_fob_fgn 	= local_rec.fob_fgn;
		}
		else
		{
			OrigFrt =	out_cost 
						 (
							store2 [i].val_frt_ins, 
							 (float) store2 [i].outer
						);
			OrigFrt *= store2 [i].quantity;
			if (TotalQty != 0.00)
				PlusFrt = OrigFrt + (FrtVar * (store2 [i].quantity / TotalQty));
			else
				PlusFrt = 0.00;

			loc_conv = local_rec.fob_fgn;
			if (store [FOB].s_exch != 0.00)
				loc_conv /= store [FOB].s_exch;

			if (store2  [i].quantity != 0.00)
				local_rec.os_fi_loc = PlusFrt / store2 [i].quantity;
			else
				local_rec.os_fi_loc = 0.00;
			store2 [i].val_frt_ins = local_rec.os_fi_loc;
			local_rec.cif_loc = loc_conv + local_rec.os_fi_loc;
		}
		putval (i);
	}
	getval (line_cnt);
	return;
}
/*
 * Calculate FOB on lines. 
 */
double 
FobVariance (void)
{
	int	i;
	double	extend, 
			variance = 0.00;

	TotalQty = 0.00;
	for (store [0].s_item_value = 0.00, i = 0;i < lcount [SCN_ITEMS];i++)
	{
		extend = out_cost (store2 [i].val_fob_fgn, (float) store2 [i].outer);
		extend *= store2 [i].quantity;
		store [0].s_item_value += extend;
		TotalQty += store2 [i].quantity;
	}
	extend  = GOODS_VAL - store [0].s_item_value;
	if (GOODS_VAL != 0.00)
	{
		extend /= GOODS_VAL;
		extend *= 100.00;
	}

	variance = GOODS_VAL;
	variance -= store [0].s_item_value;
	return (variance);
}
/*
 * Calculate FRT on lines. 
 */
double 
FrtVariance (void)
{
	int	i;
	double	extend		= 0.00, 
			variance 	= 0.00;

	store [FRT].s_item_value 	=	0.00;

	TotalQty = 0.00;
	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		extend = out_cost (store2 [i].val_frt_ins, store2 [i].outer);
		extend *= store2 [i].quantity;
		store [FRT].s_item_value += extend;
		TotalQty += store2 [i].quantity;
	}
	variance = FRT_INS_VAL;
	variance -= store [FRT].s_item_value;
	return (variance);
}
/*
 * Calculate Extended FOB total	
 */
double	
DollarTotal (void)
{
	int	i;
	double	value;
	double	total = 0.00;

	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		value = out_cost (store2 [i].val_fob_fgn, (float) store2 [i].outer);
		value *= store2 [i].quantity;
		total += value;
	}
	return (total);
}
/*
 * Calculate duty allocation per line		
 *											
 * line1 extend_fob / cust_rate * duty% = A	
 * line2 extend_fob / cust_rate * duty% = B	
 * line3 extend_fob / cust_rate * duty% = C	
 *										 --	
 *										  D	
 * line1    A/D*duty_invoice_total/line_qty	
 * line2    B/D*duty_invoice_total/line_qty	
 * line3    C/D*duty_invoice_total/line_qty	
 */
double	
DutyTotal (
 void)
{
	int	i;
	double	dutyValue;
	double	total = 0.00;

	if (local_rec.cust_rate == 0.00)
		return (0.00);

	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
	    switch (store [DTY].s_alloc [0])
	    {
	    case	'V':
			dutyValue = store2 [i].volume;
			dutyValue /= local_rec.cust_rate;
			dutyValue *= (store2 [i].duty_rate / 100);
			dutyValue *= store2 [i].quantity;			
			store2 [i].val_duty	= dutyValue;
			total += dutyValue;
		break;

	    case	'W':
			dutyValue = store2 [i].weight;
			dutyValue /= local_rec.cust_rate;
			dutyValue *= (store2 [i].duty_rate / 100);
			dutyValue *= store2 [i].quantity;
			store2 [i].val_duty = dutyValue;
			total += dutyValue;
		break;

	    case	'D':
	    default:
			dutyValue = store2 [i].val_fob_fgn;
			dutyValue /= local_rec.cust_rate;
			dutyValue *= (store2 [i].duty_rate / 100);	
			dutyValue = dutyValue;
			dutyValue *= store2 [i].quantity;
			dutyValue = out_cost (dutyValue, (float) store2 [i].outer);
			store2 [i].val_duty = dutyValue;
			total += dutyValue;
		break;
	    }
	}

	return (total);
}

/*
 * Calc Total Extended Weights display any items where the inis_weight is zero
 */
double	
WeightTotal (void)
{
	int	i;
	int	missing = 0;
	int	check;		/* check for missing weights	*/
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("W");

	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		if (check && store2 [i].weight <= 0.00)
		{
			if (!missing)
				OpenMissing ("Weight");
			missing = 1;
			if (SaveMissing (store2 [i].hhbrHash))
				break;
		}
		
		if (!missing)
		{
			value = store2 [i].weight;
			value *= store2 [i].quantity;
			total += value;
		}
	}

	if (missing)
	{
		ShowMissing ();
		missingInis = TRUE;
	}

	return (total);
}

/*
 * Calc Total Extended Volumes display any items where the inis_volume is zero
 */
double	
VolumeTotal (void)
{
	int	i;
	int	check;		/* check for missing volumes	*/
	int	missing = 0;
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("V");

	for (i = 0;i < lcount [SCN_ITEMS];i++)
	{
		if (check && store2 [i].volume <= 0.00)
		{
			if (!missing)
				OpenMissing ("Volume");
			missing = 1;
			if (SaveMissing (store2 [i].hhbrHash))
				break;
		}
		
		if (!missing)
		{
			value = store2 [i].volume;
			value *= store2 [i].quantity;
			total += value;
		}
	}

	if (missing)
	{
		ShowMissing ();
		missingInis = TRUE;
	}

	return (total);
}

/*
 * Check what weightings are being used Pass Weighting Type
 *	D(ollar, V(olume, W(eight		
 * Return 1 if Weighting type is used otherwise 0
 */
static	int
CheckInvoice (
 char	*weight_type)
{
	int	i;

	for (i = FOB;i <= O_4;i++)
		if (store [i].s_alloc [0] == weight_type [0])
			return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Setup for Missing Weight / Volume Display	
 */
void
OpenMissing (
	char	*desc)
{
	Dsp_open (2, 8, 6);

	sprintf (err_str, "   Missing %-6.6s Values   ", desc);
	Dsp_saverec (err_str);
	Dsp_saverec (" From Inventory / Supplier ");
	Dsp_saverec (" [NEXT]   [PREV]   [EDIT/END]");
}

/*
 * Save Supplier / Item No. for display	
 */
static	int
SaveMissing (
 long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (!cc)
	{
		sprintf (err_str, " %-6.6s ^E %-16.16s ", local_rec.crd_no, inmr_rec.item_no);
		return (Dsp_saverec (err_str));
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Data	
 */
void
ShowMissing (void)
{
	Dsp_srch ();
	Dsp_close ();
	edit_exit = 1;
}

double	
WeightCalc (
 int		lineIndex, 		/* FOB, FRT, INS, ...				*/
 int		line_no, 	/* line number of current line in screen 3	*/
 double		fobTotal, 
 double		weightTotal, 
 double		volumeTotal, 
 double		dutyTotal)
{
	double	inv_value = store [lineIndex].s_inv_value;
	double	calcValue;

	if (lineIndex == DTY)
	{
	/*
		calcValue	=	store2 [line_no].val_duty * store2 [line_no].outer;
	*/
		calcValue	=	store2 [line_no].val_duty / store2[line_no].quantity * store2[line_no].outer;
		if (dutyTotal != 0.00)
			calcValue /= dutyTotal;
		else
			calcValue = 0.00;

		calcValue	*=	inv_value;
	}
	else
	{
		switch (store [lineIndex].s_alloc [0])
		{
		/*
		 * Weight	
		 */
		case	'W':
		{
			calcValue = (double) store2 [line_no].weight * 
								 store2 [line_cnt].outer;
			if (weightTotal != 0.00)
				calcValue /= weightTotal;
			else
				calcValue = 0.00;
			calcValue *= inv_value;
		}
		break;

		/*
		 * Volume	
		 */
		case	'V':
		{
			calcValue = store2 [line_no].volume * store2 [line_no].outer;
			if (volumeTotal != 0.00)
				calcValue /= volumeTotal;
			else
				calcValue = 0.00;
			calcValue *= inv_value;
		}
		break;

		default:
			switch (lineIndex)
			{
			case	FOB:
				calcValue =	store2 [line_no].val_fob_fgn; 
				break;
	
			default:
				calcValue	=	store2 [line_no].val_fob_fgn; 
				if (fobTotal != 0.00)
					calcValue /= fobTotal;
				else
					calcValue = 0.00;
				calcValue *= inv_value;
				break;
			}
			/*
			calcValue *= (double) store2 [line_no].outer;
			*/
			break;
		}
	}
	return (calcValue);
}
/*
 * Recalculate & Display screen for every change	
 */
void
DisplayScreenStuff (
 int print_flag)
{
	int	i = line_cnt;
	int	line_no;
	int	this_page = line_cnt / TABLINES;
	double	loc_conv;
	double	value [10];
	double	dty_total;
	double	fob_total;
	double	wgt_total;
	double	vol_total;

	missingInis = FALSE;
	dty_total = DutyTotal ();
	fob_total = DollarTotal ();
	wgt_total = WeightTotal ();
	vol_total = VolumeTotal ();

	if (CheckInvoice ("W") || CheckInvoice ("V"))
		if (cur_screen == SCN_ITEMS)
			scn_write (SCN_ITEMS);

	if (print_flag)
		putval (line_cnt);
	else
		scn_set (SCN_ITEMS);

	for (line_cnt = 0;line_cnt < lcount [SCN_ITEMS];line_cnt++)
	{
		getval (line_cnt);

		/*
		 * Calculate default spreading of invoices	
		 */
		for (line_no = FOB;line_no <= O_4;line_no++)
		{
			value [line_no] =	WeightCalc 
								 (
									line_no,   
									line_cnt, 
									fob_total, 
									wgt_total, 
									vol_total, 
									dty_total
								);
		}
		loc_conv = local_rec.fob_fgn;

		if (store [FOB].s_exch != 0.00)
			loc_conv /= store [FOB].s_exch;

		STORE2.val_fob_fgn		= local_rec.fob_fgn;
		local_rec.os_fi_loc 	= value [FRT] + value [INS];
		STORE2.val_frt_ins		= local_rec.os_fi_loc;
		local_rec.cif_loc		= loc_conv + local_rec.os_fi_loc;

		local_rec.os_fin_chg 	= value [INT] + value [B_C];

		local_rec.duty_val 		= value [DTY];
		local_rec.duty_val 		= local_rec.duty_val;
		STORE2.val_duty 		= local_rec.duty_val;

		local_rec.lic_val 		= loc_conv;
		local_rec.lic_val 		*= STORE2.lic_rate;

		local_rec.other 		= value [O_1] + 
								  value [O_2] + 
				  		  		  value [O_3] + 
								  value [O_4];

		local_rec.unit_cst 		= local_rec.cif_loc + 
								  local_rec.os_fin_chg + 
								  local_rec.duty_val + 
								  local_rec.lic_val + 
								  local_rec.other;

		putval (line_cnt);

		if (print_flag && this_page == line_cnt / TABLINES)
				line_display ();
	}
	line_cnt = i;

	PrintGoods (print_flag);
	PrintFreightInsurance (print_flag);
	PrintDuty (print_flag);

	getval (line_cnt);
}

/*
 * Find Currency	
 */
int
FindPocr (
	char *currCode)
{
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, currCode);
	return (find_rec (pocr, &pocr_rec, COMPARISON, "r"));
}

/*
 * Open up allocation window and call relevent functions to store data. 
 */
void
OpenAllocation (void)
{
	numberTabLines = 0;
	TAG_DONE = FALSE;

	abc_selfield (sumr, "sumr_hhsu_hash");
	open_rec (popc, popc_list, POPC_NO_FIELDS, "popc_sh_id");

	tab_open ("pc_alloc", pc_keys, 6, 0, 10, FALSE);
	tab_add ("pc_alloc", 
		"#  GRIN NUMBER   | SUPP.|INVOICE NUMBER.|SHIPMENT NO.|  P/O. NUMBER  |CATEGORY DESCRIPTION|FOREIGN VALUE.|EX. RATE|  LOCAL VALUE ");

	/*
	 * Load details by Purchase order. 
	 */
	if (PO_NO)
		PurchaseAllocation (pogh_rec.hhpo_hash);

	/*
	 * Load details by Shipment. 
	 */
	if (SH_NO)
	{
		ShipmentAllocation (pogh_rec.hhsh_hash);

		/*
		 * Load details by Purchase order within shipment. 
		 */
		strcpy (posd_rec.co_no, comm_rec.co_no);
		posd_rec.hhsh_hash = pogh_rec.hhsh_hash;
		posd_rec.hhpo_hash = 0L;
		cc = find_rec (posd, &posd_rec, GTEQ, "r");
		while (!cc && !strcmp (posd_rec.co_no, comm_rec.co_no) && 
		       		posd_rec.hhsh_hash == pogh_rec.hhsh_hash)
		{
			PurchaseAllocation (posd_rec.hhpo_hash);

			cc = find_rec (posd, &posd_rec,NEXT, "r");
		}
	}
	/*
	 * Load details by Goods receipt No. 
	 */
	if (GR_NO)
	{
		GoodsReceiptAllocation (pogh_rec.hhgr_hash);

		if (pogh_rec.hhsh_hash > 0L)
			ShipmentAllocation (pogh_rec.hhsh_hash);

		/*
		 * Load details by Purchase order within shipment. 
		 */
		strcpy (posd_rec.co_no, comm_rec.co_no);
		posd_rec.hhsh_hash = pogh_rec.hhsh_hash;
		posd_rec.hhpo_hash = 0L;
		cc = find_rec (posd, &posd_rec, GTEQ, "r");
		while (!cc && !strcmp (posd_rec.co_no, comm_rec.co_no) && 
		       		posd_rec.hhsh_hash == pogh_rec.hhsh_hash)
		{
			PurchaseAllocation (posd_rec.hhpo_hash);

			cc = find_rec (posd, &posd_rec,NEXT, "r");
		}
	}

	LoadPopc ();
}
	
/*
 * Allocate by goods receipt number. 
 */
void
GoodsReceiptAllocation (
	long	hhgrHash)
{
	if (hhgrHash == 0L)
		return;

	abc_selfield (popc, "popc_gr_id");
	strcpy (popc_rec.co_no, comm_rec.co_no);
	popc_rec.hhgr_hash = hhgrHash;
	cc = find_rec (popc, &popc_rec, GTEQ, "r");
	while (!cc && !strcmp (popc_rec.co_no, comm_rec.co_no) &&
		 popc_rec.hhgr_hash == hhgrHash)
	{
		StorePopc ();

		cc = find_rec (popc, &popc_rec,NEXT, "r");
	}
}
/*
 * Allocate by Shipment number. 
 */
void
ShipmentAllocation (
 long	hhshHash)
{
	if (hhshHash == 0L)
		return;

	abc_selfield (popc, "popc_sh_id");
	strcpy (popc_rec.co_no, comm_rec.co_no);
	popc_rec.hhsh_hash = hhshHash;
	cc = find_rec (popc, &popc_rec, GTEQ, "r");
	while (!cc && !strcmp (popc_rec.co_no, comm_rec.co_no) &&
		 popc_rec.hhsh_hash == hhshHash)
	{
		StorePopc ();
		cc = find_rec (popc, &popc_rec,NEXT, "r");
	}
}
/*
 * Allocate by Purchase order number. 
 */
void
PurchaseAllocation (
 long	hhpoHash)
{
	if (hhpoHash == 0L)
		return;

	abc_selfield (popc, "popc_po_id");
	strcpy (popc_rec.co_no, comm_rec.co_no);
	popc_rec.hhpo_hash = hhpoHash;
	cc = find_rec (popc, &popc_rec, GTEQ, "r");
	while (!cc && !strcmp (popc_rec.co_no, comm_rec.co_no) &&
		 popc_rec.hhpo_hash == hhpoHash)
	{
		StorePopc ();
		cc = find_rec (popc, &popc_rec,NEXT, "r");
	}
}
/*
 * Store details of allocation type. 
 */
void
StorePopc (void)
{
	char	csm_no [13];
	
	suin_rec.hhsi_hash	=	popc_rec.hhsi_hash;
	cc = find_rec (suin2, &suin_rec, EQUAL, "r");
	if (cc || suin_rec.approved [0] != 'Y')
		return;

	strcpy (tempAmount [0], comma_fmt (popc_rec.fgn_val,  "NNN,NNN,NNN.NN"));
	strcpy (tempAmount [1], comma_fmt (popc_rec.exch_rate, "NNNN.NNNN"));
	strcpy (tempAmount [2], comma_fmt (popc_rec.loc_val,  "NNN,NNN,NNN.NN"));

	sumr_rec.hhsu_hash	= popc_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		sprintf (sumr_rec.crd_no, "%-6.6s", "      ");
		
	strcpy (posh_rec.co_no, 	comm_rec.co_no);
	posh_rec.hhsh_hash	=	pogh_rec.hhsh_hash;
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	strcpy (csm_no, (cc) ? " " : posh_rec.csm_no);

	tab_add ("pc_alloc", 
			" %15.15s|%6.6s|%15.15s|%-12.12s|%15.15s|%20.20s|%14.14s|%8.8s|%14.14s     %010ld", 
				pogh_rec.gr_no, 
				sumr_rec.crd_no, 
				popc_rec.invoice, 
				csm_no, 
				popc_rec.po_no, 
				popc_rec.category, 
				tempAmount [0], 
				tempAmount [1], 
				tempAmount [2], 
				popc_rec.hhpc_hash
	);
	numberTabLines++;
}

/*
 * Load stored details. 
 */
void
LoadPopc (void)
{
	if (numberTabLines > 0)
	{
		tab_scan ("pc_alloc");
		CheckMultiple ();
		UpdateTab ();
	}
	else
	{
		tab_add ("pc_alloc", "  ************  NO VALID LINES CAN BE LOADED FROM THIS WORK FILE  ************");
		tab_display ("pc_alloc", TRUE);
		sleep (sleepTime);
	}
		
	tab_close ("pc_alloc", TRUE);
	abc_fclose (popc);
}

/*
 * Update tagged lines. 
 */
void
UpdateTab (void)
{
	int		i, 
			InvoiceAlloc [TABLINES];
	char	get_buf [305];
	double	nz_gst = 0.00;

	scn_set (SCN_INVOICE);
	init_vars (SCN_INVOICE);	

	abc_selfield (sumr, "sumr_hhsu_hash");
	abc_selfield (popc, "popc_hhpc_hash");

	/*
	 * Clear out any existing records. 
	 */
	for (i = 0; i < TABLINES; i++) 
	{
		getval (i);
		InvoiceAlloc  [i] = 0;
		strcpy (local_rec.inv_no,   fifteenSpaces);
		strcpy (local_rec.currency, "   ");
		strcpy (local_rec.supplier, "      ");
		strcpy (local_rec.allocation, " ");
		local_rec.fgn_val 	= 0.00;
		local_rec.exch_rate 	= 0.00;
		local_rec.loc_val 	= 0.00;
		putval (i);

		strcpy (store [i].s_curr, "   ");
		store [i].s_exch 	= 0.00;
		store [i].s_inv_value 	= 0.00;
		store [i].s_item_value = 0.00;
		store [i].s_gst_value 	= 0.00;
		store [i].inv_found 	= FALSE;
		store [i].hhsuHash 	= 0L;
	}

	/*
	 * Process all tagged lines 
	 */
	for (i = 0; i < numberTabLines; i++)
	{
		tab_get ("pc_alloc", get_buf, EQUAL, i);

		if (!tagged (get_buf))
			continue;

		popc_rec.hhpc_hash = atol (get_buf + 133);

		if (find_rec (popc, &popc_rec, EQUAL, "r"))
			continue;
		
		if (popc_rec.cat_no == 0)
			continue;

		TAG_DONE = TRUE;

		line_cnt = popc_rec.cat_no - 1;

		getval (line_cnt);

		sumr_rec.hhsu_hash = popc_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
			sprintf (local_rec.supplier, "%-6.6s", "      ");
		else
			sprintf (local_rec.supplier, "%-6.6s", sumr_rec.crd_no);

		strcpy (local_rec.allocation, "D");
	    strcpy (STORE.s_alloc, local_rec.allocation);
		strcpy (local_rec.inv_no, popc_rec.invoice);
		strcpy (local_rec.currency, popc_rec.currency);
		local_rec.fgn_val += popc_rec.fgn_val;
		local_rec.loc_val += popc_rec.loc_val;
		local_rec.exch_rate = popc_rec.exch_rate;

		if (InvoiceAlloc  [line_cnt])
		{
			sprintf (local_rec.supplier, "%6.6s", "  N/A ");
			sprintf (local_rec.inv_no, "%15.15s", "MULT. INVOICES");
			local_rec.exch_rate = 1.00;
			local_rec.fgn_val	= local_rec.loc_val;
			strcpy (local_rec.currency, "N/A");
		}

		/*
		 * Hold Fgn value for goods	
		 */
		if (popc_rec.cat_no == 1)
			STORE.s_inv_value = local_rec.fgn_val;
		else
			STORE.s_inv_value = local_rec.loc_val;

		if (gstDivide != 0.00)
			nz_gst = local_rec.loc_val / (double) gstDivide;
		else
			nz_gst = 0.00;

		local_rec.loc_gst = nz_gst;
		strcpy (STORE.s_curr, popc_rec.currency);
		STORE.hhsuHash	= sumr_rec.hhsu_hash;
		STORE.s_gst_value = nz_gst;
		STORE.inv_found = TRUE;
		STORE.s_exch = popc_rec.exch_rate;
	
		InvoiceAlloc [line_cnt]++;
		putval (line_cnt);
	}
	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
}
/*
 * Allow user to tag lines for release 
 */
static	int 
tag_func (
 int c, 
 KEY_TAB *psUnused)
{
	if (c == 'T')
		tag_toggle ("pc_alloc");
	else
		tag_all ("pc_alloc");

	return (c);
}

int
FindPohr (void)
{
	pohr_rec.hhpo_hash = posd_rec.hhpo_hash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (!cc)
	{
		sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	}
	return (cc);
}

void
LoadInvoiceScreen (void)
{
	init_vars (SCN_INVOICE);	
	scn_set (SCN_INVOICE);
	abc_selfield (sumr, "sumr_hhsu_hash");
	for (lcount [SCN_INVOICE] = 0;lcount [SCN_INVOICE] < TABLINES;lcount [SCN_INVOICE]++)
	{
	    strcpy (pogd_rec.co_no, comm_rec.co_no);
	    pogd_rec.hhgr_hash = pogh_rec.hhgr_hash;
	    pogd_rec.hhsh_hash = pogh_rec.hhsh_hash;
	    pogd_rec.hhpo_hash = pogh_rec.hhpo_hash;
	    pogd_rec.line_no = lcount [SCN_INVOICE];
	    cc = find_rec (pogd, &pogd_rec, COMPARISON, "r");
	    if (!cc)
	    {
			newPogh = FALSE;
			store [lcount [SCN_INVOICE]].hhsuHash = pogd_rec.hhsu_hash;
			sprintf (local_rec.category, "%-20.20s", poCategoryDesc [lcount [SCN_INVOICE]]);
			sprintf (local_rec.inv_no, "%-15.15s", pogd_rec.invoice);
			sprintf (local_rec.allocation, "%-1.1s", pogd_rec.allocation);
			store [lcount [SCN_INVOICE]].inv_found = (pogd_rec.hhsu_hash != 0L && strcmp (local_rec.inv_no, fifteenSpaces));

			if (pogd_rec.hhsu_hash != 0L)
			{
				sumr_rec.hhsu_hash = pogd_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
				if (cc)
				{
					sprintf (err_str, "sumr (%06ld)", pogd_rec.hhsu_hash);
					file_err (cc, err_str, "DBFIND");
				}
				sprintf (local_rec.supplier, "%-6.6s", sumr_rec.crd_no);
			}
			else
				sprintf (local_rec.supplier, "%-6.6s", " ");

			strcpy (store [lcount [SCN_INVOICE]].s_curr, pogd_rec.currency);
			sprintf (local_rec.currency, "%-3.3s", pogd_rec.currency);
			local_rec.fgn_val = pogd_rec.foreign;
			local_rec.exch_rate = pogd_rec.exch_rate;
			store [lcount [SCN_INVOICE]].s_exch = local_rec.exch_rate;
			local_rec.loc_val = pogd_rec.nz_value;
			local_rec.loc_gst = pogd_rec.nz_gst;

			/*
			 * Store fgn iff goods	
			 */
			if (lcount [SCN_INVOICE] == 0)
				store [lcount [SCN_INVOICE]].s_inv_value = local_rec.fgn_val;
			else
				store [lcount [SCN_INVOICE]].s_inv_value = local_rec.loc_val;
			store [lcount [SCN_INVOICE]].s_gst_value = local_rec.loc_gst;
	    	strcpy (store [lcount [SCN_INVOICE]].s_alloc, pogd_rec.allocation);
	    }
	    else
	    {
			store [lcount [SCN_INVOICE]].inv_found = FALSE;
			if (lcount [SCN_INVOICE] < 5)
				store [lcount [SCN_INVOICE]].s_exch = 0.00;
			else
				store [lcount [SCN_INVOICE]].s_exch = 1.00;

			if (lcount [SCN_INVOICE] == 0)
				store [lcount [SCN_INVOICE]].s_exch = posh_rec.ex_rate;

			strcpy (local_rec.allocation, "D");
			strcpy (store [lcount [SCN_INVOICE]].s_alloc, local_rec.allocation);
			strcpy (store [lcount [SCN_INVOICE]].s_curr, "   ");
			store [lcount [SCN_INVOICE]].hhsuHash = pogd_rec.hhsu_hash;
			sprintf (local_rec.category, "%-20.20s", poCategoryDesc [lcount [SCN_INVOICE]]);
	    }
	    local_rec.hhgrHash = pogh_rec.hhgr_hash;
	    local_rec.hhshHash = pogh_rec.hhsh_hash;
	    local_rec.hhpoHash = pogh_rec.hhpo_hash;
    
	    putval (lcount [SCN_INVOICE]);
	}

	vars [scn_start].row = lcount [SCN_INVOICE];
	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
	scn_set (SCN_SELECT);
	return;
}

static long
hhsuGetSupplier (
	long hhplHash)
{
	poln_rec.hhpl_hash = hhplHash;
	cc = find_rec (poln, &poln_rec, EQUAL, "r");
	if (cc)
		return (pogh_rec.hhsu_hash);

	if (pohr_rec.hhpo_hash != poln_rec.hhpo_hash)
	{
		pohr_rec.hhpo_hash = poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc)
			return (pogh_rec.hhsu_hash);
	}
	return (pohr_rec.hhsu_hash);
}

void
LoadGoodsReceipt (
	long	hhgrHash, 
	char	*gr_stat, 
	int		fst_flag, 
	long	hhshHash, 
	long	hhsuHash, 
	long	hhpoHash, 
	double	cust_rate)
{
	int	inis_err;
	int	part; 
	double	loc_conv;

	print_at (0, 0, ML (mlStdMess035));

	if (fst_flag)
	{
		init_vars (SCN_INVOICE);	
		lcount [SCN_ITEMS] = 0;
	}
	scn_set (SCN_ITEMS);

	part = (gr_stat [0] == 'P');

	pogl_rec.hhgr_hash = hhgrHash;
	pogl_rec.line_no = 0;
	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	while (!cc && pogl_rec.hhgr_hash == hhgrHash)
	{
		/*
		 * Fetch supplier of line for inis record
		 */
		long po_hhsuHash = hhsuGetSupplier (pogl_rec.hhpl_hash);

		abc_selfield (sumr, "sumr_hhsu_hash");
		sumr2_rec.hhsu_hash	=	po_hhsuHash;
		cc = find_rec (sumr, &sumr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, sumr, "DBFIND");

		abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
		strcpy (SR2I.supplier, sumr2_rec.crd_no);

		SR2I.haveInis	= !cc;

		inmr_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str, "inmr_hhbr_hash (%010ld)", pogl_rec.hhbr_hash);
			file_err (cc, err_str, "DBFIND");
		}
		SR2I.hhbrHash	= inmr_rec.hhbr_hash;
		SR2I.outer 		= inmr_rec.outer_size;
		if (SR2I.outer == 0.00)
			SR2I.outer = 1.00;

		inis_rec.hhsu_hash = po_hhsuHash;
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, comm_rec.cc_no);
		inis_err = find_rec (inis, &inis_rec, COMPARISON, "r");
		if (inis_err)
		{
			strcpy (inis_rec.wh_no, "  ");
			inis_err = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
		if (inis_err)
		{
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			inis_err = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
		cc = inis_err;
		SR2I.haveInis	= !cc;
		SR2I.weight   	= (!cc) ? inis_rec.weight : 0.00;
		SR2I.volume   	= (!cc) ? inis_rec.volume : 0.00;
		SR2I.pur_conv 	= (float) ((!cc) ? inis_rec.pur_conv : 1.00);

		if (inis_err)
		{
			int dd;

			/*
			 * Find part number for branch record. 
			 */
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			dd = find_rec (inei, &inei_rec, EQUAL, "r");
			if (dd) 
				file_err (dd, inei, "DBFIND");

			local_rec.fob_gross = inei_rec.last_cost * pohr_rec.curr_rate;
		}
		else
			local_rec.fob_gross = inis_rec.fob_cost * SR2I.outer;
		
		SR2I.val_fob_gross = local_rec.fob_gross;

		if (!cc)
		{
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
		}

		strcpy (SR2I.sup_uom, (cc) ? "    " : inum_rec.uom);
			
		SR2I.upd_inis = FALSE;
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		strcpy (SR2I.std_uom, (cc) ? "    " : inum_rec.uom);

		strcpy (local_rec.item_no, inmr_rec.item_no);
		local_rec.qty_rec 		= 	pogl_rec.qty_rec;
		local_rec.scn_fob_gross	=	local_rec.fob_gross  / SR2I.outer;
		local_rec.fob_fgn 		= 	pogl_rec.fob_fgn_cst;
		local_rec.scn_fob_fgn 	= 	local_rec.fob_fgn / SR2I.outer;
		local_rec.os_fi_loc 	= 	 (!part) ? pogl_rec.frt_ins_cst : 0.00;
		local_rec.cif_loc 		= 	 (!part) ? pogl_rec.fob_nor_cst : 0.00;
		local_rec.os_fin_chg 	= 	0.00;
	
		/*
		 * Couldn't find Inis OR Inis_duty is Blank, AND	
		 * inmr_duty is blank						
		 */
		if ((inis_err || !strcmp (inis_rec.duty, "  ")) && 
						 !strcmp (inmr_rec.duty, "  "))
		{
			local_rec.duty_pc = 0.00;
		}
		else
		{

			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code, 
					 (!inis_err && 
					 strcmp (inis_rec.duty, "  ")) 	? inis_rec.duty 
													: inmr_rec.duty);
			cc = find_rec (podt, &podt_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (err_str, "podt (%s)", inmr_rec.duty);
				file_err (cc, err_str, "DBFIND");
			}

			if (local_rec.cust_rate != 0.00)
				loc_conv = local_rec.fob_fgn / local_rec.cust_rate;
			else
				loc_conv = 0.00;

			/*
			 * Duty is Dollar Value	
			 */
			if (podt_rec.duty_type [0] == 'D')
			{
				if (loc_conv != 0.00)
					local_rec.duty_pc = podt_rec.im_duty / loc_conv;
				else
					local_rec.duty_pc = 0.00;
			}
			else
				local_rec.duty_pc = podt_rec.im_duty;
		}

		if (pogl_rec.hhlc_hash == 0L)
			polh_rec.ap_lic_rate = 0.00;
		else
		{
			polh_rec.hhlc_hash	=	pogl_rec.hhlc_hash;
			cc = find_rec (polh, &polh_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (err_str, "polh (%06ld)", pogl_rec.hhlc_hash);
				file_err (cc, err_str, "DBFIND");
			}
		}

		/*
		 * ie duty_pc of 10% is held as 10.00	
		 */
		local_rec.duty_val 	= (!part) ? pogl_rec.duty : 0.00;
		local_rec.lic_pc 	= polh_rec.ap_lic_rate;
		local_rec.lic_val 	= (!part) ? pogl_rec.licence : 0.00;
		local_rec.other 	= (!part) ? pogl_rec.lcost_load : 0.00;
		local_rec.unit_cst 	= (!part) ? pogl_rec.land_cst : 0.00;

		SR2I.quantity		= (double) pogl_rec.qty_rec;
		SR2I.duty_rate		= local_rec.duty_pc;
		SR2I.lic_rate		= polh_rec.ap_lic_rate;
		SR2I.val_fob_fgn 	= local_rec.fob_fgn;
		SR2I.fob_fgn_cst 	= local_rec.fob_fgn;
		SR2I.val_frt_ins 	= local_rec.os_fi_loc;
		SR2I.val_duty 		= local_rec.duty_val;

		strcpy (SR2I.duty_type, podt_rec.duty_type);

		local_rec.hhsuHash = po_hhsuHash;

		putval (lcount [SCN_ITEMS]++);

		cc = find_rec (pogl, &pogl_rec,NEXT, "r");
	}
	if (hhshHash > 0L)
	{
		strcpy (posh_rec.co_no, comm_rec.co_no);
		posh_rec.hhsh_hash = hhshHash;
		cc = find_rec (posh, &posh_rec, COMPARISON, "r");
		if (!cc)
			strcpy (local_rec.curr_code, posh_rec.curr_code);
		else
			strcpy (local_rec.curr_code, "   ");
	}
	else
	{
		abc_selfield (sumr, "sumr_hhsu_hash");
		sumr_rec.hhsu_hash	=	hhsuHash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		strcpy (local_rec.curr_code, (cc) ? "   " : sumr_rec.curr_code);
		posh_rec.ex_rate = ((FindPocr (local_rec.curr_code)) ? 0.00 : pocr_rec.ex1_factor);
	}
	vars [scn_start].row = lcount [SCN_ITEMS];

	if (fst_flag)
		LoadInvoiceScreen ();

	move (0, 2);
	cl_line ();
	fflush (stdout);

	scn_set (SCN_SELECT);

	if (!newPogh)
	{
		FLD ("cust_rate") = NA;
		local_rec.cust_rate = cust_rate;
		DSP_FLD ("cust_rate");
	}
	DSP_FLD ("curr_code");
}

/*
 * Update all files. 
 */
void
Update (void)
{
	long	lastHhgr = 0L;
	int	costed = (local_rec.complete [0] == 'Y');

	clear ();

	strcpy (local_rec.prev_crd_no, local_rec.crd_no);

	print_at (0, 0, ML (mlStdMess035));

	if (SH_NO)
		abc_selfield (pogd, "pogd_id_no2");
	
	if (PO_NO)
		abc_selfield (pogd, "pogd_id_no3");
	
	if (GR_NO)
		abc_selfield (pogd, "pogd_id_no");

	scn_set (SCN_INVOICE);
	for (line_cnt = 0;line_cnt < lcount [SCN_INVOICE];line_cnt++)
	{
		getval (line_cnt);

		strcpy (pogd_rec.co_no, comm_rec.co_no);
		pogd_rec.hhgr_hash = local_rec.hhgrHash;
		pogd_rec.hhsh_hash = local_rec.hhshHash;
		pogd_rec.hhpo_hash = local_rec.hhpoHash;
		pogd_rec.line_no = line_cnt;
		cc = find_rec (pogd, &pogd_rec, COMPARISON, "u");

		strcpy (pogd_rec.category, local_rec.category);
		strcpy (pogd_rec.invoice, local_rec.inv_no);
		strcpy (pogd_rec.allocation, local_rec.allocation);
		pogd_rec.hhsu_hash = STORE.hhsuHash;
		sprintf (pogd_rec.currency, local_rec.currency);
		pogd_rec.foreign = local_rec.fgn_val;
		pogd_rec.exch_rate = local_rec.exch_rate;
		pogd_rec.nz_value = local_rec.loc_val;
		pogd_rec.nz_gst = local_rec.loc_gst;

		if (!cc)
		{
			cc = abc_update (pogd, &pogd_rec);
			if (cc)
				file_err (cc, pogd, "DBUPDATE");
		}
		else
		{
			cc = abc_add (pogd, &pogd_rec);
			if (cc)
				file_err (cc, pogd, "DBADD");
		}
		abc_unlock (pogd);
		fflush (stdout);
	}

	scn_set (SCN_ITEMS);

	print_at (0, 0, ML (mlStdMess035));


	for (line_cnt = 0; line_cnt < lcount [SCN_ITEMS];line_cnt++)
	{
		getval (line_cnt);

		cc = find_rec (pogl2, &pogl_rec, COMPARISON, "u");

		getval (line_cnt);

		/*
		 * Update goods receipt header record. 
		 */
		if (pogl_rec.hhgr_hash != lastHhgr)
			UpdateHeader (pogl_rec.hhgr_hash);

		lastHhgr = pogl_rec.hhgr_hash;

		pogl_rec.act_cst 		= local_rec.unit_cst;
		pogl_rec.fob_fgn_cst 	= local_rec.fob_fgn;
		pogl_rec.fob_nor_cst 	= local_rec.cif_loc;
		pogl_rec.frt_ins_cst 	= local_rec.os_fi_loc;
		pogl_rec.lcost_load		= local_rec.other;
		pogl_rec.duty			= local_rec.duty_val;
		pogl_rec.duty_pc 		= (float) local_rec.duty_pc;
		pogl_rec.licence 		= local_rec.lic_val;
		strcpy (pogl_rec.pur_status, (costed) ? "C" : "P");
		strcpy (pogl_rec.gl_status, (costed) ? "C" : "P");

		cc = abc_update (pogl2, &pogl_rec);
		if (cc)
			file_err (cc, pogl2, "DBUPDATE");

		abc_unlock (pogl);

		if (STORE2.upd_inis)
		{
			UpdateInis 
			 (
				out_cost (local_rec.fob_gross, STORE2.outer), 
				local_rec.hhsuHash
			);
		}
	}
}

/*
 * Update goods receipt header record. 
 */
void
UpdateHeader (
 long hhgrHash)
{
	int	costed = (local_rec.complete [0] == 'Y');

	pogh2_rec.hhgr_hash = hhgrHash;
	cc = find_rec (pogh2, &pogh2_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, pogh2, "DBFIND");

	pogh2_rec.cust_rate = local_rec.cust_rate;

	if (SH_NO)
		strcpy (pogh2_rec.cst_by, "S");

	if (PO_NO)
		strcpy (pogh2_rec.cst_by, "P");

	if (GR_NO)
		strcpy (pogh2_rec.cst_by, "G");
	
	strcpy (pogh2_rec.pur_status, (costed) ? "C" : "P");
	strcpy (pogh2_rec.gl_status, (costed) ? "C" : "P");
	cc = abc_update (pogh2, &pogh2_rec);
	if (cc)
		file_err (cc, pogh2, "DBUPDATE");

	abc_unlock (pogh2);

	if (SH_NO)
		UpdatePosh (pogh2_rec.hhsh_hash, costed);
}

void
UpdatePosh (
 long hhshHash, 
 int costed)
{
	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh, &posh_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, posh, "DBFIND");
	
	strcpy (posh_rec.status, (costed) ? "C" : "P");
	
	cc = abc_update (posh, &posh_rec);
	if (cc)
		file_err (cc, posh, "DBUPDATE");
}

/*
 * Search for goods receipt number. 
 */
void
SrchPogh (
 char	*keyValue)
{
	char	sr_key [21];

	_work_open (20, 0, 60);

	strcpy (pogh_rec.co_no, comm_rec.co_no);

	if (SH_NO)
	{
		save_rec ("#Shipment No    ", "# System Reference          | Date Raised");
		pogh_rec.hhsh_hash = 0L;
	}

	if (PO_NO)
	{
		save_rec ("#P/Order Number ", "# System Reference          | Date Raised");
		sprintf (pogh_rec.pur_ord_no, "%-15.15s", keyValue);
	}

	if (GR_NO)
	{
		save_rec ("#GRIN Number    ", "# System Reference          | Date Raised");
		sprintf (pogh_rec.gr_no, "%-15.15s", keyValue);
	}

	cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
	while (!cc && !strcmp (pogh_rec.co_no, comm_rec.co_no))
	{
		if (SH_NO && pogh_rec.hhsh_hash == 0L)
		{
			cc = find_rec (pogh, &pogh_rec,NEXT, "r");
			continue;
		}

		if (PO_NO && !strcmp (pogh_rec.pur_ord_no, fifteenSpaces))
		{
			cc = find_rec (pogh, &pogh_rec,NEXT, "r");
			continue;
		}
		
		if (GR_NO && !strcmp (pogh_rec.gr_no, fifteenSpaces))
		{
			cc = find_rec (pogh, &pogh_rec,NEXT, "r");
			continue;
		}

		if (PO_NO)
		{
			strcpy (sr_key, pogh_rec.pur_ord_no);
			sprintf (err_str, "GRIN No  : %s | %10.10s  ", 
					pogh_rec.gr_no, 
					DateToString (pogh_rec.date_raised));
		}
		if (SH_NO)
		{
			strcpy (posh_rec.co_no, comm_rec.co_no);
			posh_rec.hhsh_hash	=	pogh_rec.hhsh_hash;
			cc = find_rec (posh, &posh_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (pogh, &pogh_rec,NEXT, "r");
				continue;
			}
			sprintf (sr_key, "%-15.15s", (cc) ? " " : posh_rec.csm_no);

			sprintf (err_str, "GRIN No  : %s | %10.10s  ", 
					pogh_rec.gr_no, 
					DateToString (pogh_rec.date_raised));
		}
		if (GR_NO)
		{
			strcpy (sr_key, pogh_rec.gr_no);
			sprintf (err_str, "P/Order  : %s | %10.10s  ", 
					pogh_rec.pur_ord_no, 
					DateToString (pogh_rec.date_raised));
		}
		cc = save_rec (sr_key, err_str);
		if (cc)
			break;
	
		cc = find_rec (pogh, &pogh_rec,NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (SH_NO)
	{
		strcpy (posh2_rec.co_no, comm_rec.co_no);
		sprintf (posh2_rec.csm_no, "%-12.12s", temp_str);
		cc = find_rec (posh2, &posh2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, posh2, "DBFIND");

		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsh_hash = posh2_rec.hhsh_hash;
	}
	strcpy (pogh_rec.co_no, comm_rec.co_no);
	if (PO_NO)
		sprintf (pogh_rec.pur_ord_no, "%-15.15s", temp_str);
	if (GR_NO)
		sprintf (pogh_rec.gr_no, "%-15.15s", temp_str);

	cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pogh, "DBFIND");
}
/*
 * Search routine for supplier invoice file.     
 */
void
SrchSuin (
 char	*keyValue)
{
	char	disp_amt [22];
	double	inv_balance;
	
	_work_open (16, 0, 40);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no, keyValue);
	save_rec ("#Document", "#  Base Currency ");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, keyValue, strlen (keyValue)) && 
			 (suin_rec.hhsu_hash == sumr_rec.hhsu_hash))
	{
		if (suin_rec.approved [0] == 'Y')
		{
			inv_balance = suin_rec.amt - suin_rec.gst;
			sprintf (disp_amt, "%-14.2f ", DOLLARS (inv_balance));
			cc = save_rec (suin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (suin, &suin_rec,NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no, temp_str);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");
}
/*
 * Search for currency	
 */
void
SrchPocr (
 char	*keyValue)
{
	_work_open (3, 0, 40);
	save_rec ("#No", "#Currency Description");
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", keyValue);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strncmp (pocr_rec.code, keyValue, strlen (keyValue)) && 
		      !strcmp (pocr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocr_rec,NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*
 * Print Costing Details. 
 */
void
PrintHeading (
 int scn)
{
	if (scn == SCN_INVOICE)
		box (0, 1, 132, 2);
	else
	{
		box (0, 1, 132, 5);
		line_at (4,1,131);
	}

	if (SH_NO)
	{
		strcpy (posh_rec.co_no, 	comm_rec.co_no);
		posh_rec.hhsh_hash	=	pogh_rec.hhsh_hash;
		cc = find_rec (posh, &posh_rec, COMPARISON, "r");
		if (!cc)
			print_at (2, 5, ML ("Shipment No %s"), posh_rec.csm_no);
	}
	if (PO_NO)
		print_at (2, 5, ML (mlPoMess154), pogh_rec.pur_ord_no);
	if (GR_NO)
		print_at (2, 5, ML (mlPoMess155), pogh_rec.gr_no);
	
	print_at (3, 5, ML (mlPoMess156), DateToString (pogh_rec.date_raised));
	print_at (3, 26, ML (mlPoMess157), local_rec.curr_code, local_rec.curr_rate);
	print_at (3, 57, ML (mlPoMess158), local_rec.cust_rate);
}

void
PrintValue (
 void)
{
	print_at (5, 2, ML (mlPoMess216), " ", " ", " ");
	print_at (5, 68, ML (mlPoMess217), " ", " ", " ");
	print_at (6, 2, ML (mlPoMess218), " ", " ", " ");

	move (66, 4); PGCHAR (8);
	move (66, 5); PGCHAR (5);
	move (66, 6); PGCHAR (5);
	move (66, 7); PGCHAR (9);
}

int
PrintGoods (
 int print_flag)
{
	int	i;
	double	extend, 
			variance = 0.00;

	for (store [0].s_item_value = 0.00, i = 0;i < lcount [SCN_ITEMS];i++)
	{
		extend = out_cost (store2 [i].val_fob_fgn, (float) store2 [i].outer);
		extend *= store2 [i].quantity;
		store [0].s_item_value += extend;
	}

	extend  = GOODS_VAL - store [0].s_item_value;
	if (GOODS_VAL != 0.00)
	{
		extend /= GOODS_VAL;
		extend *= 100.00;
	}


	variance = twodec (GOODS_VAL);
	variance -= twodec (store [0].s_item_value);
	
	if (print_flag)
	{
		print_at (5, 16, "%11.2f", GOODS_VAL);
		print_at (5, 36, "%11.2f", store [0].s_item_value);
		print_at (5, 54, "%11.2f", variance);
	}

	if (MoneyZero (variance))
		return (TRUE);

	if (fabs (extend) < GOODS_ERR)
		return (TRUE);

	return (FALSE);
}

int
PrintDuty (
 int print_flag)
{
	int	i;
	double	extend, 
			variance = 0.00;

	for (store [5].s_item_value = 0.00, i = 0;i < lcount [SCN_ITEMS];i++)
	{
		extend = out_cost (store2 [i].val_duty, store2 [i].outer);
		extend *= store2 [i].quantity;
		store [5].s_item_value += extend;
	}

	extend = DUTY_VAL - store [5].s_item_value;
	if (DUTY_VAL != 0.00)
	{
		extend /= DUTY_VAL;
		extend *= 100.00;
	}

	variance = no_dec (DUTY_VAL);
	variance -= no_dec (store [5].s_item_value);
	variance = no_dec (variance);
		
	if (print_flag)
	{
		print_at (5, 82, "%11.2f", DUTY_VAL);
		print_at (5, 103, "%11.2f", store [5].s_item_value);
		print_at (5, 120, "%11.2f", variance);
	}

	if (MoneyZero (variance))
		return (TRUE);
	if (fabs (extend) < DUTY_ERR)
		return (TRUE);
	return (FALSE);
}

int
PrintFreightInsurance (
 int print_flag)
{
	int	i;
	double	extend, 
			variance = 0.00;

	for (store [FRT].s_item_value = 0.00, i = 0;i < lcount [SCN_ITEMS];i++)
	{
		extend = out_cost (store2 [i].val_frt_ins, store2 [i].outer);
		extend *= store2 [i].quantity;
		store [FRT].s_item_value += extend;
	}

	extend  = FRT_INS_VAL - store [FRT].s_item_value;
	if (FRT_INS_VAL != 0.00)
	{
		extend /= FRT_INS_VAL;
		extend *= 100.00;
	}

	variance  = threedec (FRT_INS_VAL);
	variance -= threedec (store [FRT].s_item_value);
	variance = threedec (variance);

	store [2].s_item_value = 0.00;

	if (print_flag)
	{
		print_at (6, 16, "%11.2f", FRT_INS_VAL);
		print_at (6, 36, "%11.2f", store [FRT].s_item_value);
		print_at (6, 54, "%11.2f", variance);
	}

	if (MoneyZero (variance))
		return (TRUE);
	if (fabs (extend) < FRT_INS_ERR)
		return (TRUE);
	return (FALSE);
}

/*
 * Update Inventory Supplier Record. 
 */
void
UpdateInis (
 double upd_value, 
 long hhsuHash)
{
	/*
	 * Find inventory supplier records. 
	 */
	inis_rec.hhsu_hash = hhsuHash;
	inis_rec.hhbr_hash = pogl_rec.hhbr_hash;
	strcpy (inis_rec.co_no, 	comm_rec.co_no);
	strcpy (inis_rec.br_no, 	comm_rec.est_no);
	strcpy (inis_rec.wh_no, 	comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	if (cc)
	{
		strcpy (inis_rec.wh_no, 	"  ");
		abc_unlock (inis);
		cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, 	"  ");
		strcpy (inis_rec.wh_no, 	"  ");
		abc_unlock (inis);
		cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	}
	if (!cc)
	{
		inis_rec.fob_cost = upd_value;
		inis_rec.lcost_date = StringToDate (local_rec.systemDate);
		cc = abc_update (inis, &inis_rec);
		if (cc)
			file_err (cc, inis, "DBUPDATE");
	}
	abc_unlock (inis);
}

/*
 * Load category descriptions if defined else use default. 
 */
void
LoadCategoryDescription (void)
{
	char	*sptr;
	int	i;

	for (i = 0; i < 10; i++)
	{
		switch (i)
		{
		case 0:
			sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			break;

		case 1:
			sptr = chk_env ("PO_OS_1");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;

		case 2:
			sptr = chk_env ("PO_OS_2");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 3:
			sptr = chk_env ("PO_OS_3");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 4:
			sptr = chk_env ("PO_OS_4");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 5:
			sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			break;
		case 6:
			sptr = chk_env ("PO_OTHER1");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 7:
			sptr = chk_env ("PO_OTHER2");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 8:
			sptr = chk_env ("PO_OTHER3");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;
		case 9:
			sptr = chk_env ("PO_OTHER4");
			if (sptr == (char *)0)
				sprintf (poCategoryDesc [i], "%-20.20s", invoiceCategory [i]);
			else
				sprintf (poCategoryDesc [i], "%-20.20s", sptr);
			break;

		default:
			break;
		}
	}
}

/*
 * Print Heading. 
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		tab_row = (scn == SCN_ITEMS) ? 8 : 6;
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		print_at (0, 96, ML (mlPoMess007), local_rec.prev_crd_no, local_rec.prevGrinNo);
		switch (scn)
		{
		case	SCN_SELECT:
			rv_pr (ML (mlPoMess159), 50, 0, 1);
			line_at (1,0,132);
			line_at (7,1,131);
			box (0, 3, 132, 7);
			break;

		case	SCN_INVOICE:
			rv_pr (ML (mlPoMess160), 42, 0, 1);
			PrintHeading (scn);
			rv_pr (ML (mlPoMess161), 40, 5, 1);
			break;

		case	SCN_ITEMS:
			rv_pr (ML (mlPoMess162), 40, 0, 1);
			DisplayScreenStuff (FALSE);
			PrintHeading (scn);
			PrintValue ();
			PrintGoods (TRUE);
			PrintDuty (TRUE);
			PrintFreightInsurance (TRUE);
			rv_pr (ML (mlPoMess163), 28, 7, 1);
			break;

		default:
			break;
		}

		line_cnt = 0;
		scn_write (scn);
	}
	PrintCompany ();
    return (EXIT_SUCCESS);
}

void
tab_other (
	int		line)
{
	getval (line);
	if (cur_screen == 3)
	{
		 FLD ("scn_fob_gross") = store2[line].haveInis ? YES : NA;
	}
	if (cur_screen == 2)
	{
		if (store [line].hhsuHash == 0L)
			FLD ("inv_no") = NA;
		else
			FLD ("inv_no") = YES;
	}
}
void
PrintCompany (void)
{
	line_at (20, 0, 132);
	strcpy (err_str, ML (mlStdMess038));
	print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);

	line_at (22, 0, 132);
}

/*      
*  Minor support functions
*/
int
MoneyZero (
   double m)     
{                        
   return (fabs (m) < 0.001);        
}


/*
 * Check for tagged lines. 
 */
void	
CheckMultiple (void)
{
	int  	i				= 0, 
			firstTime		= FALSE, 
  			categoryCnt		= 0, 
  			multipleRate 	= FALSE, 
  			multipleCurr 	= FALSE;
  	char	currencyRef [4];
  	double	exchangeRateRef	=	0.00;
		  
	abc_selfield (popc, "popc_hhpc_hash");
	for (categoryCnt = 0; categoryCnt < TABLINES; categoryCnt++)
	{
		firstTime = TRUE;
		for (i = 0; i < numberTabLines; i++)
		{
			tab_get ("pc_alloc", get_buf, EQUAL, i);
			popc_rec.hhpc_hash = atol (get_buf + 133);
			cc = find_rec (popc, &popc_rec, EQUAL, "r");
			if (cc)
				continue;

			if (!tagged (get_buf) || categoryCnt != popc_rec.cat_no)
			{
			sprintf (err_str, "tagged [%d] [%d][%d]", tagged (get_buf) , categoryCnt , popc_rec.cat_no);
				continue;
			}

			if (CHECKRATES || CHECKCURR)
			{
				if (firstTime)
				{
					if (CHECKRATES)
						exchangeRateRef = popc_rec.exch_rate;
					if (CHECKCURR)
						strcpy (currencyRef , popc_rec.currency);

					firstTime = FALSE;
				}
				if (CHECKRATES && !multipleRate)
				{
					strcpy (err_str, ML ("%R Multiple invoices with different exchange rates involved. Press any key to continue."));

					PauseForKey (23, 1, err_str, 0);
					clear_mess();
					multipleRate = TRUE;
				}
				if (CHECKCURR && !multipleCurr)
				{
					strcpy (err_str, ML ("%R Multiple invoices with different currencies involved. Press any key to continue."));
					PauseForKey (23, 1, err_str, 0);
					multipleCurr = TRUE;
					clear_mess();
				}
				if (multipleRate && multipleCurr)
					return;
			}
		}
	}
}


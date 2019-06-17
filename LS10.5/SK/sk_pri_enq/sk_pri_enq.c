/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_pri_enq.c,v 5.5 2002/12/01 04:48:17 scott Exp $
|  Program Name  : (sk_pri_enq.c)
|  Program Desc  : (Price & Delivery Enquiry)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : (06/10/93)       |
|---------------------------------------------------------------------|
| $Log: sk_pri_enq.c,v $
| Revision 5.5  2002/12/01 04:48:17  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.4  2002/11/28 04:09:48  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_pri_enq.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_pri_enq/sk_pri_enq.c,v 5.5 2002/12/01 04:48:17 scott Exp $";

#define	X_OFF	lpXoff
#define	Y_OFF	lpYoff

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<hot_keys.h>
#include 	<ring_menu.h>
#include 	<twodec.h>
#include 	<getnum.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define	MANUAL			0
#define	BRANCH			1
#define	COMPANY			2

#define	QUANTITIES    	1
#define	CUSTOMERDETAIL 	2
#define	ITEMDETAILS 	3
#define	STOCKSTATUS 	4
#define	PURCHASEORDERS 	5
#define	CONTRACTPRICE 	6
#define	PRICINGSTRUCT 	7
#define	DISCOUNTSTRUCT 	8
#define	DELIVERYDETAILS	9

#define	POP_X	 		1
#define	POP_Y	 		13

#define	CAL(amt, pc)	 (no_dec (amt * DOLLARS (pc)))

#define	KITITEM			 (inmr_rec.inmr_class [0] == 'K')

#define	VAL_ITLN 		 (itln_rec.status [0] == 'T' || \
						  itln_rec.status [0] == 'B' || \
						  itln_rec.status [0] == 'M' || \
						  itln_rec.status [0] == 'U')

#define	SCREENWIDTH		132
#define SLEEPTIME		3

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	char	mlSkDisp [133][125];

char	*std_foot = " [NEXT] [PREV] [EDIT/END] ";

char	*sixty_space = "                                                            ";

char	*underline = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct ccmrRecord	ccmr3_rec;
struct cuitRecord	cuit_rec;
struct cumrRecord	cumr_rec;
struct cudiRecord	cudi_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct inmrRecord	inmr_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct pcwoRecord	pcwo_rec;
struct pocrRecord	pocr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct sumrRecord	sumr_rec;
struct exafRecord	exaf_rec;

	int		envVarQcApply 		= FALSE,
			envVarSkQcAvl 		= FALSE,
			envVarDbCo			= 0,
			envVarSoFwdAvl		= 0,
			envVarDbFind		= 0,
			envVarSkDbprinum	= 0,
			envVarSkDbqtynum	= 0,
			envVarDbMCurr		= 0,
			envVarSoDoi			= 0,
			envVarSoDiscRev 	= FALSE,
			priceBreak 			= 0,
			discBreak 			= 0,
			single_cont 		= FALSE,
			clearOK 			= TRUE,
			mainOpen			= FALSE,
			popup_select 		= CUSTOMERDETAIL,
			lpXoff				= 0,
			lpYoff			= 0,
			custPriceType		= 0,
			cumDisc				= 0;

	FILE	*fout;

	char	branchNumber 	[3],
			envVarSoSpecial [5],
			bonus 			[3],
			envVarCurrCode 	[4],
			Curr_desc 		[41],
			curDiscFor 		[21],
			curDiscLevel 	[21];
	
	char	*data  = "data",
			*sptr;

	double	basePrice		= 0.00,
			stdPrice		= 0.00, 
			curPrice		= 0.00, 
			regPrice		= 0.00, 
			curr_factor		= 0.00;

	float	curDiscs [3]	= {0.00,0.00,0.00},
			totalDisc		= 0.00,
			gwsRegPc		= 0.00,
			prQty 			= 0.00;


#include	<cus_price.h>
#include	<cus_disc.h>

struct	tag_inprRecord	inpr2_rec;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	sort [2];
	char	customerNo [7];
	char	lastCustomerNo [7];
	char	customerCurr [6];
	char	customerName [41];
	char	curr_desc [41];
	char	itemNumber [17];
	char	itemDesc [41];
	char	back [8];
	char	onight [8];
	int		lpno;	
	char	dflt_qty [15];
	char	rep_qty [10];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo",	 3, 12, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Customer    ", "Enter Customer Number, Full Search Available, Default for last customer. ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{1, LIN, "customerCurr",	 3, 20, CHARTYPE,
		"AAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerCurr},
	{1, LIN, "customerName",	 3, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerName},
	{1, LIN, "itemNumber",	4, 12, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Item        ", "Enter Item Number, Full Search Available.",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "itemDesc",	 4, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc},
	{1, LIN, "areaCode",	5, 12, CHARTYPE,
		"UU", "          ",
		" ", " ", " Area Code   ", "Enter Area Number, Full Search Available.",
		YES, NO,  JUSTLEFT, "", "", exaf_rec.area_code},
	{1, LIN, "areaDesc",	 5, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Callback Declarations 
 */
char	*GetPriceDesc 		(int);
int 	GetQuantity 		(void);
int 	CustomerDetails		(void);
int 	ItemDetails 		(void);
int 	StockStatus 		(void);
int 	StatusDisplay 		(void);
int 	ContractPrice 		(void);
int 	PricingStruct 		(void);
int 	DiscountStruct 		(void);
int 	PurchaseOrders 		(void);
int	 	DeliveryDetails		(void);
int 	ClearRedraw 		(void);
void 	SrchExaf 			(char *);

#ifndef GVISION
menu_type	_main_menu [] = {
  {"<Quantity Input>", "Display Price For Quantity.  [ Q ]",
	GetQuantity, "Qq",	  },
  {"<Customer Details>", "Display Customers Details.  [ C ]",
	CustomerDetails, "Cc",	  },
  {"<Item Details>", "Display Item Details.  [ I ]",
	ItemDetails, "Ii",	  },
  {"<Stock Status>", "Display Stock Status For Item.  [ S ]",
	StockStatus, "Ss",	  },
  {"<Branch Status>", "Display Branch Status.  [ B ] or [ S ]",
	StatusDisplay, "Bb",	  },
  {"<Contract Price>", "Display Contract Pricing For Customer and Item.  [ C ]",
	ContractPrice, "Cc",	  },
  {"<Standard Pricing>", "Display Standard Pricing Structure For Item.  [ S ]",
	PricingStruct, "Ss",	  },
  {"<Discount Breaks>","Display Discount Breaks For Item.  [ D ]",
	DiscountStruct, "Dd",	  },
  {"<Purchase Orders>", "Display Purchase Orders For Item.  [ P ]",
	PurchaseOrders, "Pp",	  },
  {"<Delivery Details>", "Display Delivery Details.  [ D ]",
	DeliveryDetails, "Dd",	  },
  {"<FN03>", "Redraw Display", ClearRedraw, "", FN3,			  },
  {"<FN16>", "Exit Display", _no_option, "", FN16, EXIT | SELECT	  },
  {"",									  },
};
#else
menu_type	_main_menu [] = {
  {0, "<Quantity Input>", "",
	GetQuantity, },
  {0, "<Customer Details>", "",
	CustomerDetails, },
  {0, "<Item Details>", "",
	ItemDetails, },
  {0, "<Stock Status>", "",
	StockStatus, },
  {0, "<Branch Status>", "",
	StatusDisplay, },
  {0, "<Contract Price>", "",
	ContractPrice, },
  {0, "<Standard Pricing>", "",
	PricingStruct, },
  {0, "<Discount Breaks>","",
	DiscountStruct, },
  {0, "<Purchase Orders>", "",
	PurchaseOrders, },
  {0, "<Delivery Details>", "",
	DeliveryDetails, },
  {0, "<FN03>", "Redraw Display", ClearRedraw, FN3,			  },
  {0, "",									  },
};
#endif

#include 	<get_lpno.h>
#include 	<FindCumr.h>

/*
 * Function Declarations 
 */
char 	*DiscountTitle 			(void);
char 	*SetDiscFor 			(void);
char 	*SetDiscLvl 			(void);
char 	*SetPrice 				(void);
float 	ScreenDisc 				(float);
int  	CheckBonus 				(char *);
int  	DispQtyBreaks 			(char *);
int  	GetCurDisc 				(void);
int  	GetCurPrice 			(void);
int  	GetStdPrice 			(void);
int  	PoShipment 				(char *);
int  	ProcessPohr 			(long, float, double, long);
int  	heading 				(int);
int  	spec_valid 				(int);
static 	int DisplayTheItem 		(int);
void 	AllDisplay 				(void);
void 	CloseDB 				(void);
void 	ContractLine 			(void);
void 	DisplayData 			(void);
void 	InitML 				 	(void);
void 	OpenDB 					(void);
void 	PrintMissGr 			(void);
void 	PrintPopup 				(int);
void 	TheReDraw 				(void);
void 	WsFindInpr 				(int);
void 	shutdown_prog 			(void);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	int		before,
			after;

	if (argc != 1 && argc != 3)
	{
		print_at (0,0,mlSkMess055, argv [0]);
		return (EXIT_FAILURE);
	}
	
	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind	= atoi (get_env ("DB_FIND"));

	sptr = chk_env ("SK_DBQTYNUM");
	if (sptr == (char *)0)
		envVarSkDbqtynum = 0;
	else
	{
		envVarSkDbqtynum = atoi (sptr);
		if (envVarSkDbqtynum > 9 || envVarSkDbqtynum < 0)
			envVarSkDbqtynum = 9;
	}

	sptr = chk_env ("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envVarSkDbprinum = 5;
	else
	{
		envVarSkDbprinum = atoi (sptr);
		if (envVarSkDbprinum > 9 || envVarSkDbprinum < 1)
			envVarSkDbprinum = 9;
	}

	/*
     * Check and Get Order Date Type. 
     */
	sptr = chk_env ("SO_DOI");
	envVarSoDoi = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;

	/*
	 * Check for multi-currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envVarDbMCurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Get native currency. 
	 */
	sprintf (envVarCurrCode,"%-3.3s",get_env ("CURR_CODE"));

	sptr = chk_env ("SO_FWD_AVL");
	if (sptr == (char *)0)
		envVarSoFwdAvl = TRUE;
	else
		envVarSoFwdAvl = atoi (sptr);

	sptr = chk_env ("SO_SPECIAL");
	if (sptr == (char *)0)
		strcpy (envVarSoSpecial,"/B/H");
	else
		sprintf (envVarSoSpecial,"%-4.4s", sptr);

	/* QC module is active or not. */
	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	/* Whether to include QC qty in available stock. */
	envVarSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	OpenDB ();

	InitML ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*
	 * Setup required variables for NormCusPrice in cus_price.h. 
	 */
	sptr = chk_env ("SK_PROMO_PRICE");
	if (sptr == (char *)0)
		_promoPrice = 0;
	else
		_promoPrice = atoi (sptr);

	sptr = chk_env ("SK_CUSPRI_LVL");
	if (sptr == (char *)0)
		_priceLevel = 0;
	else
		_priceLevel = atoi (sptr);

	sptr = chk_env ("SK_PRI_ORD");
	if (sptr == (char *)0)
		_priceOrder = 2;
	else
		_priceOrder = atoi (sptr);

	_numQtyBrks    = envVarSkDbqtynum;
	_numPriceTypes = envVarSkDbprinum;

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

	/*
	 * Prepare screen. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	input_row = 19;
	strcpy (local_rec.lastCustomerNo, "      ");

	if (argc == 3)
	{
   		entry_exit = TRUE;
   		edit_exit  = FALSE;
   		prog_exit  = FALSE;
   		restart    = FALSE;
		search_ok  = FALSE;
		prQty      = 1;

		init_vars (1);	
		clearOK = TRUE;
		heading (1);
		abc_selfield (cumr, "cumr_hhcu_hash");
		cumr_rec.hhcu_hash = atol (argv [1]);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		custPriceType = atoi (cumr_rec.price_type);

		if (envVarDbMCurr)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, cumr_rec.curr_code);
			find_rec (pocr, &pocr_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (SLEEPTIME);
				clear_mess ();
				strcpy (cumr_rec.curr_code, envVarCurrCode);
				strcpy (pocr_rec.co_no, comm_rec.co_no);
				strcpy (pocr_rec.code, envVarCurrCode);
				find_rec (pocr, &pocr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, "pocr", "DBFIND");
				strcpy (Curr_desc, pocr_rec.description);
				curr_factor = 1.00;
			}
			else
			{
				curr_factor = pocr_rec.ex1_factor;
				strcpy (Curr_desc, pocr_rec.description);
			}
		}
		else
			curr_factor = 1.00;

		sprintf (local_rec.customerNo,   "%-6.6s",   cumr_rec.dbt_no);
		sprintf (local_rec.customerCurr, "(%-3.3s)", cumr_rec.curr_code);
		sprintf (local_rec.customerName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("customerNo");
		DSP_FLD ("customerCurr");
		DSP_FLD ("customerName");
		
		abc_selfield (inmr, "inmr_hhbr_hash");
		inmr_rec.hhbr_hash = atol (argv [2]);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (KITITEM)
		{
			print_mess (ML (mlSkMess388));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (! (CheckBonus (inmr_rec.item_no)))
		{
			cc = GetCurPrice ();
			if (cc)
				restart = TRUE;

			cc = GetStdPrice ();
			if (cc)
				restart = TRUE;

			cc = GetCurDisc ();
			if (cc)
				restart = TRUE;
		}
		else
		{
			sprintf (bonus,"%-2.2s", envVarSoSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.itemNumber,"%-s%-.*s", sptr,16 - (int) strlen (sptr),bonus);
		}
		sprintf (local_rec.itemNumber,   "%-16.16s", inmr_rec.item_no);
		sprintf (local_rec.itemDesc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no, "  ");
	
		cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
		if (cc)
			file_err (cc, "ccmr", "DBFIND");

		/*
		 * Display Pricing.  
		 */
		DisplayTheItem (0);
		shutdown_prog ();   
		return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
   		entry_exit = 0;
   		edit_exit  = 0;
   		prog_exit  = 0;
   		restart    = 0;
		search_ok  = TRUE;
		prQty      = 1;

		init_vars (1);	
		clearOK = TRUE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
		clearOK = FALSE;

		/*
		 * Display Pricing.  
		 */
		DisplayTheItem (0);
		strcpy (local_rec.lastCustomerNo, local_rec.customerNo);
	}
	shutdown_prog ();   
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no"
														   : "cumr_id_no3");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (cudi,  cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_hhch_hash");
	open_rec (cncl,  cncl_list, CNCL_NO_FIELDS, "cncl_id_no2");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ithr,  ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");

	OpenPrice ();
	OpenDisc ();

}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (cuit);
	abc_fclose (cudi);
	abc_fclose (cnch);
	abc_fclose (cncl);
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (pocr);
	abc_fclose (inum);
	abc_fclose (exaf);

	ClosePrice ();
	CloseDisc ();
	SearchFindClose ();
	abc_dbclose (data);
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

int
spec_valid (
	int field)
{
	/*
	 * Screen ONE Validation. 
	 */
	/*
	 * Validate Customer Number.  
	 */
	if (LCHECK ("customerNo"))
	{
		if (prog_status == ENTRY && last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used && strcmp (local_rec.lastCustomerNo, "      ") != 0)
		{
			strcpy (local_rec.customerNo, local_rec.lastCustomerNo);
			sprintf (local_rec.customerCurr, "(%-3.3s)",   cumr_rec.curr_code);
			sprintf (local_rec.customerName, "%-40.40s", cumr_rec.dbt_name);
			DSP_FLD ("customerCurr");
			DSP_FLD ("customerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.customerNo));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		custPriceType = atoi (cumr_rec.price_type);

		if (envVarDbMCurr)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, cumr_rec.curr_code);
			find_rec (pocr, &pocr_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (SLEEPTIME);
				clear_mess ();
				strcpy (cumr_rec.curr_code, envVarCurrCode);
				strcpy (pocr_rec.co_no, comm_rec.co_no);
				strcpy (pocr_rec.code, envVarCurrCode);
				find_rec (pocr, &pocr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, "pocr", "DBFIND");
				strcpy (Curr_desc, pocr_rec.description);
				curr_factor = 1.00;
			}
			else
			{
				curr_factor = pocr_rec.ex1_factor;
				strcpy (Curr_desc, pocr_rec.description);
			}
		}
		else
			curr_factor = 1.00;

		sprintf (local_rec.customerCurr, "(%-3.3s)",   cumr_rec.curr_code);
		sprintf (local_rec.customerName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("customerCurr");
		DSP_FLD ("customerName");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("itemNumber"))
	{
		if (SRCH_KEY)
		{
			InmrSearch 
			 (	
				comm_rec.co_no, 
				temp_str, 
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
			strcpy (inmr_rec.description, 
				   "                                        ");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			return (EXIT_FAILURE);
		
		cc = FindInmr 
			 (
				comm_rec.co_no, 
				local_rec.itemNumber, 
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (KITITEM)
		{
			print_mess (ML (mlSkMess388));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((CheckBonus (inmr_rec.item_no)))
		{
			sprintf (bonus,"%-2.2s", envVarSoSpecial);
			sptr = clip (inmr_rec.item_no);
			sprintf (local_rec.itemNumber,"%-s%-.*s",
				sptr,16 - (int) strlen (sptr),bonus);
		}
	
		SuperSynonymError ();

		sprintf (local_rec.itemDesc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no, "  ");
	
		cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
		if (cc)
			file_err (cc, "ccmr", "DBFIND");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Area. 
	 */
	if (LCHECK ("areaCode"))
	{
		if (F_HIDE (field) || dflt_used)
			strcpy (exaf_rec.area_code, cumr_rec.area_code);

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no, comm_rec.co_no);
		cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("areaDesc");

		if (!(CheckBonus (inmr_rec.item_no)))
		{
			cc = GetCurPrice ();
			if (cc)
				restart = TRUE;

			cc = GetStdPrice ();
			if (cc)
				restart = TRUE;

			cc = GetCurDisc ();
			if (cc)
				restart = TRUE;
		}
	
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Display selected contract 
 */
static int 
DisplayTheItem (
	int c)
{
	/*
	 * Display main screen. 
	 */
	AllDisplay ();

#ifndef GVISION
	run_menu (_main_menu, "", input_row);
#else
    run_menu (NULL, _main_menu);
#endif

	return (c);
}

char *
SetDiscFor (void)
{
	switch (_rtnDType)
	{
	case	D_NODISC		: return ("");
	case	D_CUST_ITEM		: return (ML ("Cust/Item      "));
	case	D_CUST_MINSELL	: return (ML ("Cust/MinSell   "));
	case	D_CUST_MAJSELL	: return (ML ("Cust/MajorSell "));
	case	D_CUST_MINCAT	: return (ML ("Cust/MinCat    "));
	case	D_CUST_MAJCAT	: return (ML ("Cust/MajorCat  "));
	case	D_CTYP_ITEM		: return (ML ("CusType/Item   "));
	case	D_CTYP_MINSELL	: return (ML ("CusType/MinSell"));
	case	D_CTYP_MAJSELL	: return (ML ("CusType/MajSell"));
	case	D_CTYP_MINCAT	: return (ML ("CusType/MinCat "));
	case	D_CTYP_MAJCAT	: return (ML ("CusType/MajCat "));
	case	D_PTYP_ITEM		: return (ML ("PrType/Item    "));
	case	D_PTYP_MINSELL	: return (ML ("PrType/MinSell "));
	case	D_PTYP_MAJSELL	: return (ML ("PrType/MajSell "));
	case	D_PTYP_MINCAT	: return (ML ("PrType/MinCat  "));
	case	D_PTYP_MAJCAT	: return (ML ("PrType/MajorCat"));
	case	D_MAX_CAT		: return (ML ("Customer/MaxCat"));
	case	D_CUSTOMER		: return (ML ("Customer Code  "));
	}
    return (""); /* this ok? */
}

char *
SetDiscLvl (
 void)
{
	switch (_rtnDLvl)
	{
	case 0 : return (ML ("Company"));
	case 1 : return (ML ("Branch"));
	case 2 : return (ML ("Warehouse"));
	}
    return (""); /* is this OK? */
}

char *
SetPrice (
 void)
{
	switch (_rtnPrice)
	{
	case 0 : return (ML ("No Price   "));
	case 1 : return (ML ("Contract   "));
	case 2 : return (ML ("Customer   "));
	case 3 : return (ML ("Standard   "));
	}
    return (""); /* is this OK? */
}

/*
 * Display data on main screen using print_at for greater re-display speed. 
 */
void
DisplayData (void)
{
	char	wsPriceType [21];
	char	wsExpDate [21];
	char	wsPriceLevel [21];
	char	wsPriceFor [21];
	char	wsAllowDisc [4];
	char	wsApplyDisc [21];
	char	wsCurDiscs [3] [21];
	char	wsTotalDisc [21];
	double	discAmt;
	double	custPrice;

	strcpy (wsPriceType, 	ML ("Standard  "));
	strcpy (wsExpDate, 		"  /  /    ");
	strcpy (wsPriceLevel, 	ML ("Warehouse "));
	strcpy (wsAllowDisc, 	ML ("Yes"));
	strcpy (wsApplyDisc, 	ML ("Absolute  "));
	strcpy (wsPriceFor, 	ML ("Item      "));

	if (cumDisc == TRUE)
		strcpy (wsApplyDisc, ML ("Cumulative "));

	strcpy (curDiscFor, SetDiscFor ());
	if (inmr_rec.disc_pc > totalDisc && inmr_rec.disc_pc != 0.0)
	{
		curDiscs [0] = inmr_rec.disc_pc;
		totalDisc   = inmr_rec.disc_pc;
		strcpy (curDiscFor,   ML ("Item Disc"));
		strcpy (curDiscLevel, ML ("Not Applicable  "));
	}
	else
	if (_rtnDType == D_MAX_CAT || _rtnDType == D_CUSTOMER)
		strcpy (curDiscLevel, ML ("Not Applicable  "));
	else
		strcpy (curDiscLevel, SetDiscLvl ());

	sprintf (wsCurDiscs [0], "%7.2f", ScreenDisc (curDiscs [0]));
	sprintf (wsCurDiscs [1], "%7.2f", ScreenDisc (curDiscs [1]));
	sprintf (wsCurDiscs [2], "%7.2f", ScreenDisc (curDiscs [2]));
	sprintf (wsTotalDisc, 	"%7.2f",  ScreenDisc (totalDisc));
	discAmt = regPrice * (totalDisc / 100);
	strcpy (wsPriceType, SetPrice ());

	switch (_rtnPrice)
	{
	case 0 : strcpy (wsExpDate, "  /  /    ");
			 break;
	case 1 : strcpy (wsExpDate, "  /  /    ");
			 break;
	case 2 : strcpy (wsExpDate, DateToString (incp_rec.date_to));
		 	 if (strcmp (&incp_rec.key [2], "    ") == 0)
		 		strcpy (wsPriceLevel, ML ("Company"));
		 	 else
			 {
		 		if (strncmp (&incp_rec.key [4], "  ", 2) == 0)
		 			strcpy (wsPriceLevel, ML ("Branch"));
		 		else
		 			strcpy (wsPriceLevel, ML ("Warehouse"));
			 }
			 if (incp_rec.hhcu_hash != 0L)
				strcpy (wsPriceFor, ML ("Customer "));
			 else if (strcmp (incp_rec.cus_type, "   ") != 0)
			 {
				strcpy (wsPriceFor, ML ("Cust Type"));
			 	if (strcmp (incp_rec.area_code, "  ") != 0)
					strcpy (wsPriceFor, ML ("Cust Type/Area"));
			}

			 if (_CON_PRICE)
			 {
				strcpy (wsAllowDisc, 	"No ");
				strcpy (wsCurDiscs [0], 	"       ");
				strcpy (wsCurDiscs [1], 	"       ");
				strcpy (wsCurDiscs [2], 	"       ");
				strcpy (wsTotalDisc, 	"       ");
				strcpy (wsApplyDisc, 	"            ");
				discAmt = 0.00;
			 }
			 break;

	case 3 : strcpy (wsExpDate, "  /  /    ");
		 	 if (strcmp (inpr_rec.br_no, "  ") == 0)
		 		strcpy (wsPriceLevel, ML ("Company"));
		 	 else
			 {
		 		if (strcmp (inpr_rec.wh_no, "  ") == 0)
		 			strcpy (wsPriceLevel, ML ("Branch"));
		 		else
		 			strcpy (wsPriceLevel, ML ("Warehouse"));
			 }
			 if (strcmp (inpr_rec.cust_type, "   ") != 0)
				strcpy (wsPriceFor, ML ("Cust Type"));
			 if (strcmp (inpr_rec.area_code, "  ") != 0)
				strcpy (wsPriceFor, ML ("Cust Type/Area"));
			 break;
	}
	custPrice = regPrice - discAmt;
	/*
	custPrice = out_cost (custPrice, inmr_rec.outer_size);
	*/

	print_at (6,  28, "%15.2f", 	prQty);
	print_at (6,  69, "%3.3s",  	wsAllowDisc);
	print_at (6, 113, "%-15.15s", 	wsPriceType);

	print_at (7,  28, "%15.1f", 	inmr_rec.outer_size);
	print_at (7,  69, "%-15.15s", 	wsApplyDisc);
	print_at (7, 113, "%-15.15s", 	wsPriceLevel);

	print_at (8,  28, "%15.2f", 	DOLLARS (curPrice));
	print_at (8,  69, "%-7.7s",  	wsCurDiscs [0]);
	print_at (8, 113, "%-15.15s", 	wsPriceFor);

	print_at (9,  13, "%7.2f", 		ScreenDisc (gwsRegPc));
	print_at (9,  28, "%15.2f", 	DOLLARS (regPrice));
	print_at (9,  69, "%-7.7s",  	wsCurDiscs [1]);
	print_at (9, 113, "%-15.15s", 	wsExpDate);

	print_at (10,  13, "%-7.7s", 	wsTotalDisc);
	print_at (10,  28, "%15.2f", 	DOLLARS (regPrice - discAmt));
	print_at (10,  69, "%-7.7s",  	wsCurDiscs [2]);
	print_at (10, 113, "%-15.15s", 	curDiscLevel);

	print_at (11,  28, "%15.2f", 	DOLLARS (custPrice));
	print_at (11,  69, "%-7.7s",  	wsTotalDisc);
	print_at (11, 113, "%-15.15s", 	curDiscFor);
}

/*
 * Display main screen. 
 */
void
AllDisplay (void)
{
	char	wsWork [200];

	if (mainOpen == TRUE)
		Dsp_close ();


	Dsp_open (0, 2, 12);
	sprintf (err_str, ".%-12.12s : %-6.6s  (%-5.5s)  %-97.97s", 
												mlSkDisp [0],
												local_rec.customerNo,
												local_rec.customerCurr,
												local_rec.customerName);
	Dsp_saverec (err_str);
	sprintf (err_str, ".%-12.12s : %-16.16s %-97.97s", 
												mlSkDisp [1],
												local_rec.itemNumber,
												local_rec.itemDesc);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	mainOpen = TRUE;

	sprintf (wsWork, " %-23.23s : %-15.15s ^E", mlSkDisp [2], " ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-15.15s    ^E", mlSkDisp [3], " ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [4], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	sprintf (wsWork, " %-23.23s : %-15.15s ^E", mlSkDisp [15], " ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-15.15s    ^E", mlSkDisp [6], " ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [7], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	sprintf (wsWork, " %-23.23s : %-15.15s ^E", mlSkDisp [5], " ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-7.7s %%          ^E", mlSkDisp [9], " ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [13], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	sprintf (wsWork, " +/- %-4.4s%% (       )     : %-15.15s ^E", mlSkDisp [8]," ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-7.7s %%          ^E", mlSkDisp [10], " ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [14], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	sprintf (wsWork, " +/- %-4.4s%% (       )     : %-15.15s ^E", mlSkDisp [12]," ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-7.7s %%          ^E", mlSkDisp [11]," ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [16], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	sprintf (wsWork, " %-23.23s : %-15.15s ^E", mlSkDisp [17], " ");
	strcpy  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-7.7s %%          ^E", mlSkDisp [18], " ");
	strcat  (err_str, wsWork);
	sprintf (wsWork, " %-20.20s : %-14.14s   ^E", mlSkDisp [19], " ");
	strcat  (err_str, wsWork);
	Dsp_saverec (err_str);

	Dsp_srch ();

	DisplayData ();
	/*
	 * Print missing graphics characters. 
	 */
	PrintMissGr ();

	/*
	 * Print popup window selected. 
	 */
	PrintPopup (popup_select);
}

void
PrintPopup (
	int type)
{
	crsr_off ();
	if (type)
		popup_select = type;

	switch (popup_select)
	{
	case QUANTITIES       :	GetQuantity ();
							break;
	case CUSTOMERDETAIL   :	CustomerDetails ();
							break;
	case ITEMDETAILS 	  :	ItemDetails ();
							break;
	case STOCKSTATUS      :	StockStatus ();
							break;
	case CONTRACTPRICE    :	ContractPrice ();
							break;
	case PRICINGSTRUCT    :	PricingStruct ();
							break;
	case DISCOUNTSTRUCT   :	DiscountStruct ();
							break;
	case DELIVERYDETAILS  :	DeliveryDetails ();
							break;
	case PURCHASEORDERS   :	PurchaseOrders ();
							break;
	}
}

int
CustomerDetails (
 void)
{
	int		i;

	/*
	 * Customers Details. 
	 */
	popup_select = CUSTOMERDETAIL;
	line_at (13, 1, SCREENWIDTH - 2);
	move (63, 13);
	PGCHAR (8);
	print_at (13, 52, "%-17.17s", 	mlSkDisp [68]);
	print_at (14, 3,  mlSkDisp [69], cumr_rec.contact_name, " ");
	print_at (15, 3,  mlSkDisp [70], cumr_rec.ch_adr1);
	print_at (16, 3, "                 : %-40.40s", cumr_rec.ch_adr2);
	print_at (17, 3, "                 : %-40.40s", cumr_rec.ch_adr3);

	for (i = 14; i < 18; i++)
	{
		move (63, i);
		PGCHAR (5);
	}

	print_at (14,66,mlSkDisp [71], GetPriceDesc (custPriceType - 1), " ");
	print_at (15,66,mlSkDisp [72],cumr_rec.class_type, exaf_rec.area_code, " ");
	print_at (16,66,mlSkDisp [73], cumr_rec.item_codes [0] == 'Y' 
										? mlSkDisp [20] : mlSkDisp [21]);
	if (envVarDbMCurr)
		print_at (17,66,mlSkDisp [74], Curr_desc);
	else
		print_at (17,66, "                  : %-40.40s", " ");

    return (EXIT_SUCCESS);
}

int
StockStatus (
 void)
{
	int			i;
	double		br_closing_stock = 0;
	double		br_on_order = 0;
	double		br_committed = 0;
	double		br_backorder = 0;
	double		br_fwdorder = 0;
	double		br_available = 0;
	double		br_qc_qty = 0.00;

	popup_select = STOCKSTATUS;
	strcpy (ccmr2_rec.co_no, comm_rec.co_no);
	strcpy (ccmr2_rec.est_no, comm_rec.est_no);
	strcpy (ccmr2_rec.cc_no, "  ");

	cc = find_rec (ccmr, &ccmr2_rec, GTEQ, "r");

	while (!cc 
	&& !strcmp (ccmr2_rec.co_no, comm_rec.co_no)
	&& !strcmp (ccmr2_rec.est_no, comm_rec.est_no))
	{
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incc_rec.hhcc_hash = ccmr2_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (!cc)
		{
			br_closing_stock += incc_rec.closing_stock;
			br_on_order		 += incc_rec.on_order;
			br_committed	 += incc_rec.committed;
			br_backorder	 += incc_rec.backorder;
			br_fwdorder	 	 += incc_rec.forward;
			br_qc_qty		 += incc_rec.qc_qty;
		}
		cc = find_rec (ccmr, &ccmr2_rec, NEXT, "r");
	}
	/*
	 * calculate available stock		
	 */
	if (envVarSoFwdAvl)
	{
		br_available =  br_closing_stock -
				   		br_committed -
				  		br_backorder -
				   		br_fwdorder;
	}
	else
	{
		br_available =  br_closing_stock -
				   		br_committed -
				   		br_backorder;
	}
	if (envVarQcApply && envVarSkQcAvl)
		br_available -= br_qc_qty;

	print_at (14, 3, mlSkDisp [22], br_closing_stock, 	" ");
	print_at (14,66, mlSkDisp [23], br_available, 		" ");
	print_at (15, 3, mlSkDisp [24], br_committed, 		" ");
	print_at (15,66, mlSkDisp [25], br_on_order, 		" ");
	print_at (16, 3, mlSkDisp [26], br_fwdorder, 		" ");
	print_at (16,66, mlSkDisp [27], br_backorder, 		" ");
	print_at (17, 3,"                            %10.10s %20.20s", " ", " ");
	print_at (17,66,"                            %10.10s %22.22s", " ", " ");
	line_at (13, 1, SCREENWIDTH - 2);
	move (66,13);
	PGCHAR (8);

	for (i = 14; i < 18; i++)
	{
		move (63, i);
		PGCHAR (5);
	}

	print_at (13, 52, mlSkDisp [28]);
	crsr_off ();
    return (EXIT_SUCCESS);
}

int
PurchaseOrders (
 void)
{
	float	pord_qty 	= 0.00;
	float	tot_qty 	= 0.00;
	float	tr_qty 		= 0.00;
	float	wo_qty 		= 0.00;

	double	pord_val	= 0.00;
	double	tot_val 	= 0.00;

	char	order_date[11];
	char	tran_date[11];
	char	tran_value[13];

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no_2");
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	lpXoff = 0;
	lpYoff = 4;
	Dsp_open(0, 5, 9);
	Dsp_saverec ("SUPPLIER|       SUPPLIER NAME               | BR |    DATE    |    DATE    | UOM. |  ORDER   | PURCHASE ORDER | S |   AMOUNT    ");
	Dsp_saverec (" NUMBER |                                   | NO |  ORDERED   |    DUE.    |      | QUANTITY |     NUMBER     |   |             ");
	Dsp_saverec (std_foot);

	/*
	 * Purchase Orders. 
	 */
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

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		pord_qty = (poln_rec.qty_ord - poln_rec.qty_rec);
		pord_val = (double) (poln_rec.qty_ord - poln_rec.qty_rec);
		pord_val *= out_cost (poln_rec.land_cst, inmr_rec.outer_size);
		pord_qty *= CnvFct;

		cc = ProcessPohr 
			(
				poln_rec.hhpo_hash,
				pord_qty,
				pord_val,
				poln_rec.due_date
			);
		if (!cc)
		{
			tot_qty += pord_qty;
			tot_val += pord_val;
		}

		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	/*
	 * Transfers. 
	 */
	abc_selfield (itln, "itln_hhbr_hash");
	itln_rec.hhbr_hash	= inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");

	while (!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		ccmr2_rec.hhcc_hash = itln_rec.r_hhcc_hash;
		if (find_rec (ccmr, &ccmr2_rec, EQUAL, "r"))
		{
			strcpy (ccmr2_rec.co_no, "??");
			strcpy (ccmr2_rec.est_no, "??");
			strcpy (ccmr2_rec.cc_no, "??");
		}

		if (!VAL_ITLN || 
		   (strcmp(ccmr2_rec.est_no, comm_rec.est_no) != 0) ||
		   (strcmp(ccmr2_rec.co_no, comm_rec.co_no) != 0))
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

		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (cc) ? 1.00 : inum_rec.cnv_fct;
		CnvFct	=	StdCnvFct / PurCnvFct;

		pord_qty *= CnvFct;

		strcpy (tran_date, DateToString (itln_rec.due_date));
		sprintf (tran_value, "%12.2f", pord_val);

		ithr_rec.hhit_hash = itln_rec.hhit_hash;
		if (find_rec (ithr, &ithr_rec, EQUAL, "r"))
		{
			ithr_rec.del_no = 0L;
			ithr_rec.iss_date = 0L;
		}
		strcpy (order_date, DateToString (ithr_rec.iss_date));

		ccmr3_rec.hhcc_hash = itln_rec.i_hhcc_hash;
		if (find_rec (ccmr, &ccmr3_rec, EQUAL, "r"))
		{
			strcpy (ccmr3_rec.co_no, "??");
			strcpy (ccmr3_rec.est_no, "??");
			strcpy (ccmr3_rec.cc_no, "??");
		}

		sprintf 
		(	
			err_str, 
			"    Br %s Wh %s TRANSFER %s TO W/H %s^E %s ^E %s ^E %s ^E %4.4s ^E %8.2f ^E %07ld        ^E %s ^E%12.2f ",
			ccmr3_rec.est_no,
			ccmr3_rec.cc_no,
		    (itln_rec.stock[0] == 'C') ? mlSkDisp [62] : mlSkDisp [63],
			ccmr2_rec.cc_no,
			ccmr2_rec.est_no,
			order_date,
			tran_date,
			inum_rec.uom,
			pord_qty,
			ithr_rec.del_no,
			itln_rec.status,
			pord_val
		);

		Dsp_saverec (err_str);
		tot_qty += pord_qty;
		tot_val += pord_val;
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	/*
	 * Works Orders. 
	 */
	pcwo_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec(pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		ccmr2_rec.hhcc_hash = pcwo_rec.hhcc_hash;
		if (find_rec (ccmr, &ccmr2_rec, EQUAL, "r"))
		{
			strcpy (ccmr2_rec.co_no, "??");
			strcpy (ccmr2_rec.est_no, "??");
			strcpy (ccmr2_rec.cc_no, "??");
		}
		/*
		 * Ignore anything not for current branch 
		 */
		if ((strcmp(ccmr2_rec.co_no, comm_rec.co_no) != 0) ||
			(strcmp(ccmr2_rec.est_no, comm_rec.est_no) != 0))
		{
			cc = find_rec(pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		/*
		 * Ignore planned, closed, deleted works orders. 
		 */
		if (pcwo_rec.order_status[0] == 'P' ||
		    pcwo_rec.order_status[0] == 'Z' ||
		    pcwo_rec.order_status[0] == 'D')
		{
			cc = find_rec(pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		wo_qty = pcwo_rec.prod_qty 
			   - (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
		if (wo_qty <= 0.00)
		{
			cc = find_rec(pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		strcpy (order_date, DateToString (pcwo_rec.create_date));

		sprintf (err_str, "   %-20.20s %s                 ^E %s ^E %s ^E %s ^E %4.4s ^E %8.2f ^E %-7.7s        ^E %s ^E             ",
			mlSkDisp [64],
			ccmr2_rec.cc_no,
			ccmr2_rec.est_no,
			order_date,
			DateToString (pcwo_rec.reqd_date),
			inmr_rec.sale_unit,
			wo_qty,
			pcwo_rec.order_no,
			pcwo_rec.order_status);

		Dsp_saverec (err_str);
		tot_qty += wo_qty;

		cc = find_rec(pcwo,&pcwo_rec, NEXT, "r");
	}

	Dsp_saverec ("^^GGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGHGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGHGGGGGGGGGGGGG");
	sprintf (err_str, " %-6.6s ^E                                   ^E    ^E            ^E            ^E      ^E %8.2f ^E                ^E   ^E%12.2f ", mlSkDisp [65], tot_qty, tot_val);
	Dsp_saverec (err_str);
	abc_selfield (ccmr, "ccmr_id_no");
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (pcwo);
	Dsp_srch_fn (PoShipment);
	Dsp_close ();
	TheReDraw ();
    return (EXIT_SUCCESS);
}
int
ContractPrice (
 void)
{
	int		cc1;

	strcpy (ccmr2_rec.co_no, comm_rec.co_no);
	strcpy (ccmr2_rec.est_no, comm_rec.est_no);
	strcpy (ccmr2_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	Dsp_open (0, 5, 10);
	Dsp_saverec (" CONTRACT NO | CONTRACT DESCRIPTION                     | EXPIRY DATE | CONTRACT PRICE | DISCOUNT |  CONTACT NAME                ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cncl_rec.hhch_hash = 0L;
	cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
	while (!cc && cncl_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		cnch_rec.hhch_hash = cncl_rec.hhch_hash;
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cnch", "DBFIND");

		if (cnch_rec.date_wef <= local_rec.lsystemDate &&
			cnch_rec.date_exp >= local_rec.lsystemDate)
		{
			cncd_rec.hhch_hash = cncl_rec.hhch_hash;
			cc1 = find_rec (cncd, &cncd_rec, GTEQ, "r");
			while (!cc1 && cncd_rec.hhch_hash == cncl_rec.hhch_hash)
			{
				if ((cncd_rec.hhbr_hash == inmr_rec.hhbr_hash) &&
				    (cncd_rec.hhcc_hash == 0L || 
					 cncd_rec.hhcc_hash == ccmr2_rec.hhcc_hash))
					ContractLine ();
				cc1 = find_rec (cncd, &cncd_rec, NEXT, "r");
			}
		}
		cc = find_rec (cncl, &cncl_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	TheReDraw ();
    return (EXIT_SUCCESS);
}

void
ContractLine (
 void)
{
	if ((cnch_rec.exch_type [0] == 'V') ||
		 (cnch_rec.exch_type [0] == 'F' && 
		strcmp (cncd_rec.curr_code, cumr_rec.curr_code) == 0))
	{
		if (cnch_rec.exch_type [0] == 'V')
			cncd_rec.price *= curr_factor;

		sprintf (err_str, "   %6.6s    ^E %-40.40s ^E  %10.10s ^E  %12.2f  ^E      %3.3s   ^E  %-20.20s         ",
			cnch_rec.cont_no,
			cnch_rec.desc,
			DateToString (cnch_rec.date_exp),
			DOLLARS (cncd_rec.price),
			cncd_rec.disc_ok [0] == 'Y' ? "Yes" : "No ",
			cnch_rec.contact);

		Dsp_saverec (err_str);
	}
}

/*
 * Find pricing structure. 
 */
void
WsFindInpr (
	int	price_type)
{
	int		i;

	inpr2_rec.hhbr_hash = inpr_rec.hhbr_hash;
	inpr2_rec.price_type = price_type + 1;
	inpr2_rec.hhgu_hash = 0L;
	strcpy (inpr2_rec.curr_code, inpr_rec.curr_code);
	strcpy (inpr2_rec.br_no, inpr_rec.br_no);
	strcpy (inpr2_rec.wh_no, inpr_rec.wh_no);
	strcpy (inpr2_rec.cust_type, inpr_rec.cust_type);
	cc = find_rec (inpr, &inpr2_rec, EQUAL, "r");
	if (cc)
	{
		inpr2_rec.base     = 0.00;
		for (i = 0; i < 9; i++)
		{
			inpr2_rec.qty_brk [i] = 0.00;
			inpr2_rec.price [i]   = 0.00;
		}
	}
}

int
DispQtyBreaks (
 char *price)
{
	char		wsWork [20];
	int			i = 0, j;
	int			wsDspCol;
	int			wsPrice;

	if (envVarSkDbqtynum == 0)
		return (EXIT_SUCCESS);

	wsPrice = atoi (price);
	WsFindInpr (wsPrice);

	sprintf (err_str, " %-10.10s %-4.4s |  %6.6s  ",
					mlSkDisp [66],
					inpr2_rec.price_by [0] == 'V' ?
					"Val." : "Qty.",
					mlSkDisp [67]);
	for (j = 0; j < envVarSkDbqtynum; j++)
	{
		sprintf (wsWork, "|  Break %1d ", j + 1);
		strcat (err_str, wsWork);
	}
	wsDspCol = (int) ((SCREENWIDTH - strlen (err_str)) / 2) - 1;
	Dsp_open (wsDspCol, 8, 1);
	Dsp_saverec (err_str);
	sprintf (err_str, " Pricing Type    |          ");
	for (j = 0; j < envVarSkDbqtynum; j++)
	{
		sprintf (wsWork, "|%10.2f", inpr2_rec.qty_brk [j]);
		strcat (err_str, wsWork);
	}
	Dsp_saverec (err_str);
	Dsp_saverec (" [EDIT / END] ");

	if (wsPrice < envVarSkDbprinum)
	{
		sprintf (err_str, "%-16.16s ^E%10.2f", GetPriceDesc (wsPrice),
												DOLLARS (inpr2_rec.base));
	}
	else
		sprintf (err_str, "                  ^E          ");

	for (j = 0; j < envVarSkDbqtynum; j++)
	{
		if (i < envVarSkDbprinum)
			sprintf (wsWork, "^E%10.2f", DOLLARS (inpr2_rec.price [j]));
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
PricingStruct (
 void)
{
	char		wsWork [20];
	int			i, j;
	int			wsDspCol;
	char		price [2];

	sprintf (err_str, " %-16.16s | %-8.8s ", "Std. Price Type ", "Base");
	for (j = 0; j < envVarSkDbqtynum; j++)
	{
		sprintf (wsWork, "|  Break %1d ", j + 1);
		strcat (err_str, wsWork);
	}
	wsDspCol = (int) ((SCREENWIDTH - strlen (err_str)) / 2) - 1;
	Dsp_open (wsDspCol, 5, 9);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec (" [EDIT / END] ");

	for (i = 0; i < 9; i++)
	{
		WsFindInpr (i);
		if (i < envVarSkDbprinum)
			sprintf (err_str, " %-16.16s ^E%10.2f", GetPriceDesc (i),
													DOLLARS (inpr2_rec.base));
		else
			sprintf (err_str, "                             ");
		for (j = 0; j < envVarSkDbqtynum; j++)
		{
			if (i < envVarSkDbprinum)
				sprintf (wsWork, "^E%10.2f", DOLLARS (inpr2_rec.price [j]));
			else
				sprintf (wsWork, "           ");
			strcat (err_str, wsWork);
		}
		sprintf (price, "%1d", i);
		if (i < envVarSkDbprinum)
			Dsp_save_fn (err_str, price);
		else
			Dsp_save_fn (err_str, (char *) 0);
	}
	if (envVarSkDbqtynum > 0)
		print_at (20, 0, mlSkDisp [29]);
	Dsp_srch_fn (DispQtyBreaks);
	if (envVarSkDbqtynum > 0)
		print_at (20, 0, "                                                            ");
	Dsp_close ();
	TheReDraw ();
    return (EXIT_SUCCESS);
}

char *
DiscountTitle (
 void)
{
	if (inmr_rec.disc_pc > totalDisc && inmr_rec.disc_pc != 0.0)
		return (ML ("Discounting By Item Code"));
	else
	switch (_rtnDType)
	{
	case	D_NODISC		: 
				return (ML ("No Discounting Available"));
	case	D_CUST_ITEM		: 
				return (ML ("Discounts By Customer And Item"));
	case	D_CUST_MINSELL	: 
				return (ML ("Discounts By Customer And Minor Sell Group"));
	case	D_CUST_MAJSELL	: 
				return (ML ("Discounts By Customer And Major Sell Group"));
	case	D_CUST_MINCAT	: 
				return (ML ("Discounts By Customer And Minor Category"));
	case	D_CUST_MAJCAT	: 
				return (ML ("Discounts By Customer And Major Category"));
	case	D_CTYP_ITEM		: 
				return (ML ("Discounts By Customer Type And Item"));
	case	D_CTYP_MINSELL	: 
				return (ML ("Discounts By Customer Type And Minor Sell Group"));
	case	D_CTYP_MAJSELL	: 
				return (ML ("Discounts By Customer Type And Major Sell Group"));
	case	D_CTYP_MINCAT	: 
				return (ML ("Discounts By Customer Type And Minor Category"));
	case	D_CTYP_MAJCAT	: 
				return (ML ("Discounts By Customer Type And Major Category"));
	case	D_PTYP_ITEM		: 
				return (ML ("Discounts By Price Type And Item"));
	case	D_PTYP_MINSELL	: 
				return (ML ("Discounts By Price Type And Minor Sell Group"));
	case	D_PTYP_MAJSELL	: 
				return (ML ("Discounts By Price Type And Major Selling Group"));
	case	D_PTYP_MINCAT	: 
				return (ML ("Discounts By Price Type And Minor Category"));
	case	D_PTYP_MAJCAT	: 
				return (ML ("Discounts By Price Type And Major Category"));
	case	D_MAX_CAT		: 
				return (ML ("Discounting Is Maximum Discount For Category"));
	case	D_CUSTOMER		: 
				return (ML ("Discounting By Customer Code"));
	}
    return (""); /* correct? */
}

int
DiscountStruct (void)
{
	int			i;
	int			wsDspCol;

	if (strcmp (curDiscFor,   "Item Disc") == 0)
	{
		print_mess (mlSkDisp [30]);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (_rtnDType == D_CUSTOMER)
	{
		print_mess (mlSkDisp [31]);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (_rtnDType == D_MAX_CAT)
	{
		print_mess (mlSkDisp [32]);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	sprintf (err_str, "%-50.50s", DiscountTitle ());
	wsDspCol = (int) ((SCREENWIDTH - strlen (err_str)) / 2) - 1;
	Dsp_open (wsDspCol, 5, 9);
	Dsp_saverec (err_str);
	strcpy (err_str, " Qty Break | Discount A | Discount B | Discount C ");
	Dsp_saverec (err_str);
	Dsp_saverec (" [ EDIT / END ] ");
	if (inmr_rec.disc_pc > totalDisc && inmr_rec.disc_pc != 0.0)
	{
		strcpy (err_str, "             Discounting By Item Code             ");
		curDiscs [0] = inmr_rec.disc_pc;
		curDiscs [1] = 0.00;
		curDiscs [2] = 0.00;
		sprintf (err_str, " %9.2f ^E %10.2f ^E %10.2f ^E %10.2f ",
						  0.00,
						  curDiscs [0],
						  curDiscs [1],
						  curDiscs [2]);
		Dsp_saverec (err_str);
	}
	else
	if (_rtnDType == D_MAX_CAT || _rtnDType == D_CUSTOMER)
	{
		sprintf (err_str, " %9.2f ^E %10.2f ^E %10.2f ^E %10.2f ",
						  0.00,
						  curDiscs [0],
						  curDiscs [1],
						  curDiscs [2]);
		Dsp_saverec (err_str);
	}
	else
	for (i = 0; i < 6; i++)
	{
		if (inds_rec.qty_brk [i] > 0.00)
		{
			sprintf (err_str, " %9.2f ^E %10.2f ^E %10.2f ^E %10.2f ",
							  inds_rec.qty_brk [i],
							  inds_rec.disca_pc [i],
							  inds_rec.discb_pc [i],
							  inds_rec.discc_pc [i]);
			Dsp_saverec (err_str);
		}
	}

	Dsp_srch ();
	Dsp_close ();
	TheReDraw ();
    return (EXIT_SUCCESS);
}

int 
GetCurPrice (void)
{
	double	wsRegAmt;

	curPrice	=	GetCusPrice 
					(
						comm_rec.co_no,			/* Company No.			  */
						comm_rec.est_no,		/* Branch No.			  */
						comm_rec.cc_no,			/* Warehouse No.		  */
						exaf_rec.area_code,		/* Customer Area Code	  */
						cumr_rec.class_type,	/* Customer Type		  */
						inmr_rec.sellgrp,		/* Item Selling Group	  */
						cumr_rec.curr_code,		/* Customer Currency	  */
						(int) custPriceType,	/* Customer Price Type	  */
						cumr_rec.disc_code,		/* Customer Discount Code */
						" ",					/* Contract Type F/V	  */
						cumr_rec.hhcu_hash,		/* Customer Hash		  */
						ccmr_rec.hhcc_hash,		/* Warehouse Hash		  */
						inmr_rec.hhbr_hash, 	/* Item Hash			  */
						inmr_rec.category, 		/* Item category		  */
						0L,						/* Contract Hash		  */
						(envVarSoDoi) ? TodaysDate () : comm_rec.dbt_date,
						(float) prQty, 			/* Quantity to price for  */
						curr_factor,			/* Exch Rate for contracts*/
						FALSE,					/* Contract use only	  */
						&gwsRegPc				/* Retnd Regulatory pc    */
					);

	if (_priceError)
	{
		print_mess (mlSkDisp [33]);
		sleep (SLEEPTIME);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	else
	{
		/*
		 * Deduct regulatory percent if non-zero. 
		 */
		if (_rtnPrice == 3 && gwsRegPc != (float)0.00)
		{
			wsRegAmt = curPrice * (double)gwsRegPc;
			wsRegAmt = wsRegAmt / (double) 100.00;
			wsRegAmt = twodec (wsRegAmt);

			regPrice = curPrice - wsRegAmt;
		}
		else
			regPrice = curPrice;
	}
	return (EXIT_SUCCESS);
}

int
GetStdPrice (void)
{
	double	wsStdPrice;

	/*
	 * Find ingp record to get regulatory percent. 
	 */
	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "S");
	sprintf (ingp_rec.code, "%-6.6s", inmr_rec.sellgrp);
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		gwsRegPc = 0.00;
	else
		gwsRegPc = ingp_rec.sell_reg_pc;

	stdPrice  = 0.00;
	basePrice = 0.00;

	wsStdPrice = 	NormCusPrice 
					(
						comm_rec.est_no, 
						comm_rec.cc_no, 
						exaf_rec.area_code, 
						cumr_rec.class_type, 
						inmr_rec.hhbr_hash, 
						cumr_rec.curr_code, 
						(float) prQty, 
						(int) custPriceType
					);
	if (wsStdPrice == (double) -1.00)
		return (EXIT_SUCCESS);
	else
	{
		basePrice = inpr_rec.base;
		stdPrice  = wsStdPrice;
	}

	return (EXIT_SUCCESS);
}

int
GetCurDisc (void)
{
	curDiscs [0] = (float) 0.00;
	curDiscs [1] = (float) 0.00;
	curDiscs [2] = (float) 0.00;

	cumDisc = GetCusDisc 
			  (
					comm_rec.co_no,			/* Company No.			  */
					comm_rec.est_no,		/* Branch No.			  */
					ccmr_rec.hhcc_hash,		/* Warehouse Hash		  */
					cumr_rec.hhcu_hash,		/* Customer Hash		  */
					cumr_rec.class_type,	/* Customer Type		  */
					cumr_rec.disc_code, 	/* Customer Discount code */
					inmr_rec.hhbr_hash, 	/* Item Hash			  */
					inmr_rec.category, 		/* Item category		  */
					inmr_rec.sellgrp,		/* Item Selling Group	  */
					(int) custPriceType,	/* Customer Price Type	  */
					basePrice,				/* Item Base Price        */
					gwsRegPc,				/* Regulatory Percentage  */
					(float) prQty, 			/* Quantity to price for  */
					&curDiscs [0]			/* Ads to return discs.   */
				);

	totalDisc = CalcOneDisc (cumDisc,
							curDiscs [0],
							curDiscs [1],
							curDiscs [2]);
    return (EXIT_SUCCESS);
}

int
GetQuantity (
 void)
{
	int		i;
	double	multi;

	prQty = getfloat (35, 6, "NNNNN.NN");
	if (inpr_rec.price_by [0] == 'V')
		multi = inpr_rec.base;
	else
		multi = 1.00;

	for (i = 0; i < envVarSkDbqtynum; i++)
	{
		if (inpr_rec.qty_brk [i] == 0.00 ||
			inpr_rec.price [i]   == 0.00)
			break;

		if ((prQty * multi) >= inpr_rec.qty_brk [i])
			priceBreak = i + 1;

		if (prQty < inpr_rec.qty_brk [i])
			break;
	}

	 (void) GetCurPrice ();
	 (void) GetStdPrice ();
	 (void) GetCurDisc  ();


	DisplayData ();
    return (EXIT_SUCCESS);
}

/*
 * Clear and redraw the main screen. 
 */
int
ClearRedraw (void)
{
	clear ();
	heading (1);
	AllDisplay ();
    return (EXIT_SUCCESS);
}

/*
 * redraw the main screen without clearing. 
 */
void
TheReDraw (void)
{
	heading (1);

	move (0, 19);
	cl_line ();
	move (0, 20);
	cl_line ();

	AllDisplay ();
}

/*
 * Move to next contract on file. 
 */
int	
ItemDetails (void)
{
	int		i;

	/*
	 * Item Details. 
	 */
	popup_select = ITEMDETAILS;
	line_at (13, 1, SCREENWIDTH - 2);
	move (63, 13);
	PGCHAR (8);
	print_at (13, 52, "    %s    ", mlSkDisp [34]);

	print_at (14,  3, mlSkDisp [35], inmr_rec.inmr_class);
	print_at (15,  3, mlSkDisp [36], inmr_rec.category);
	print_at (16,  3, mlSkDisp [37], inmr_rec.sellgrp);
	print_at (17,  3, mlSkDisp [38], inmr_rec.buygrp);
	for (i = 14; i < 18; i++)
	{
		move (63, i);
		PGCHAR (5);
	}

	print_at (14, 66, mlSkDisp [39], inmr_rec.sale_unit);
	print_at (15, 66, mlSkDisp [40], inmr_rec.uom_cfactor, " ");
	print_at (16, 66, mlSkDisp [41], inmr_rec.outer_size, " ");
	print_at (17, 66, mlSkDisp [42], gwsRegPc, " ");
    return (EXIT_SUCCESS);
}

/*
 * Move to prev contract on file. 
 */
int	
DeliveryDetails (void)
{
	int		wsCol;

	sprintf (err_str, " DELIVERY NAME                            | DELIVERY ADDRESS                         ");
	wsCol = (int) ((SCREENWIDTH - strlen (err_str)) / 2);
	Dsp_open (wsCol, 5, 10);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	sprintf (err_str, " %-40.40s ^E %-40.40s ",
					  ML ("Standard Delivery Address"),
					  cumr_rec.dl_adr1);
	Dsp_saverec (err_str);
	sprintf (err_str, " %-40.40s ^E %-40.40s ",
					  " ",
					  cumr_rec.dl_adr2);
	Dsp_saverec (err_str);
	sprintf (err_str, " %-40.40s ^E %-40.40s ",
					  " ",
					  cumr_rec.dl_adr3);
	Dsp_saverec (err_str);
	sprintf (err_str, " %-40.40s ^E %-40.40s ",
					  " ",
					  cumr_rec.dl_adr4);
	Dsp_saverec (err_str);

	cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cudi_rec.del_no    = 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		sprintf (err_str, " %-40.40s ^E %-40.40s ",
						  cudi_rec.name,
						  cudi_rec.adr1);
		Dsp_saverec (err_str);
		sprintf (err_str, " %-40.40s ^E %-40.40s ",
						  " ",
						  cudi_rec.adr2);
		Dsp_saverec (err_str);
		sprintf (err_str, " %-40.40s ^E %-40.40s ",
						  " ",
						  cudi_rec.adr3);
		Dsp_saverec (err_str);
		sprintf (err_str, " %-40.40s ^E %-40.40s ",
						  " ",
						  cudi_rec.adr4);
		Dsp_saverec (err_str);
		cc = find_rec (cudi, &cudi_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	TheReDraw ();
    return (EXIT_SUCCESS);
}

int
ProcessPohr (
	long	hhpoHash, 
	float 	pOrderQty, 
	double 	pOrderValue, 
	long 	dueDate)
{
	char	order_date[11];
	char	l_due_date[11];
	char	hhpl_char[9];

	pohr_rec.hhpo_hash = hhpoHash;
	cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
	if (!cc)
	{
		if (strcmp(pohr_rec.br_no, comm_rec.est_no) != 0)
			return(1);

		sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (order_date, DateToString (pohr_rec.date_raised));
			strcpy (l_due_date, DateToString (dueDate));

			sprintf (err_str, " %s ^E%-35.35s^E %s ^E %s ^E %s ^E %4.4s ^E%9.2f ^E%s%c^E %s ^E%12.2f ",
				sumr_rec.crd_no,
				sumr_rec.crd_name,
				pohr_rec.br_no,
				order_date,
				l_due_date,
				inum_rec.uom,
				pOrderQty,
				pohr_rec.pur_ord_no,
				(poln_rec.ship_no) ? '*' : ' ',
				poln_rec.pur_status,
				pOrderValue);

			sprintf (hhpl_char, "%08ld", poln_rec.hhpl_hash);
			Dsp_save_fn (err_str, hhpl_char);
			return(0);
		}
		return(1);
	}
	return(1);
}
int
PoShipment (
 char *hhpl_char)
{
	long	hhplHash;
	char	wk_date1 [11],
			wk_date2 [11];

	char	dsp_method [5];

	hhplHash = atol (hhpl_char);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");

	lpXoff = 0;
	lpYoff = 4;
	Dsp_open (0, 5, 10);

	Dsp_saverec ("                                      PURCHASE ORDER LINE DETAIL INFORMATION SCREEN.                                              ");

	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	poln_rec.hhpl_hash = hhplHash;
	cc = find_rec (poln, &poln_rec, EQUAL, "r");
	if (cc)
	{
		abc_fclose (sumr);
		abc_fclose (pohr);
		abc_fclose (poln);
		abc_fclose (posh);
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
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_SUCCESS);
	}
	sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	{
		abc_fclose (sumr);
		abc_fclose (pohr);
		abc_fclose (poln);
		abc_fclose (posh);
		Dsp_srch ();
		Dsp_close ();
		return (EXIT_SUCCESS);
	}

	sprintf 
	(
		err_str, 
		" %26.26s %-11.11s : %s %13.13s Cont Name   : %s ",
		" ", mlSkDisp [45], pohr_rec.pur_ord_no, " ", pohr_rec.contact
	);
	Dsp_saverec (err_str);

	sprintf 
	(
		err_str, 
		" %26.26s %-11.11s : %s - %s ", 
	  	" ", mlSkDisp [46], sumr_rec.crd_no, sumr_rec.crd_name
	);
	Dsp_saverec (err_str);

	if (strcmp (pohr_rec.delin1, sixty_space))
	{
		sprintf (err_str, " %26.26s %-11.11s : %s ", " ", mlSkDisp [47], pohr_rec.delin1);
		Dsp_saverec (err_str);
	}

	if (strcmp (pohr_rec.delin2, sixty_space))
	{
		sprintf (err_str, " %26.26s %-11.11s : %s ", " ", mlSkDisp [48], pohr_rec.delin2);
		Dsp_saverec (err_str);
	}

	if (strcmp (pohr_rec.delin3, sixty_space))
	{
		sprintf (err_str, " %26.26s %-11.11s : %s ", 
		" ", mlSkDisp [49], pohr_rec.delin3);
		Dsp_saverec (err_str);
	}

	sprintf (err_str, " %26.26s %-11.11s : %6.4f - %s", 
				  " ", mlSkDisp [50], poln_rec.exch_rate, pohr_rec.curr_code);
	Dsp_saverec (err_str);

	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = poln_rec.ship_no;
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (err_str, "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^ ^1 SHIPMENT DETAILS ^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		Dsp_saverec (err_str);

		strcpy (wk_date1, DateToString (posh_rec.ship_depart));
		strcpy (wk_date2, DateToString (posh_rec.ship_arrive));

		sprintf (err_str, " %26.26s %-11.11s : %12.12s ", 
						  " ", mlSkDisp [51], posh_rec.csm_no);
		Dsp_saverec (err_str);

		if (posh_rec.ship_method [0] == 'A')
			strcpy (dsp_method, mlSkDisp [58]);
		else if (posh_rec.ship_method [0] == 'S')
			strcpy (dsp_method, mlSkDisp [59]);
		else if (posh_rec.ship_method [0] == 'L')
			strcpy (dsp_method, mlSkDisp [60]);
		else 
			strcpy (dsp_method, "????");

		sprintf (err_str, " %26.26s %-11.11s : %s %17.17s %-11.11s : %s",
		  " ", mlSkDisp [52], dsp_method, " ", mlSkDisp [55], posh_rec.vessel);
		Dsp_saverec (err_str);

		sprintf (err_str, " %26.26s %-11.11s : %s %11.11s %-11.11s : %s",
				  " ", mlSkDisp [53],wk_date1, " ", mlSkDisp [56],wk_date2);
		Dsp_saverec (err_str);

		sprintf (err_str, " %26.26s %-11.11s : %s   %-11.11s : %s",
	  " ", mlSkDisp [54], posh_rec.port, mlSkDisp [57], posh_rec.destination);
		Dsp_saverec (err_str);

		sprintf (err_str, " %26.26s %-11.11s : %6.4f - %s",
					  " ", mlSkDisp [50], posh_rec.ex_rate, posh_rec.curr_code);
		Dsp_saverec (err_str);
	}
	else
	{
		sprintf (err_str, "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^ ^1 %-20.20s^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG", mlSkDisp [61]);
		Dsp_saverec (err_str);
	}
	Dsp_srch ();
	Dsp_close ();
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
    return (EXIT_SUCCESS);
}

/*
 * Routine to check if bonus flag has been set (this is indicated by a  
 * '/B' on the end of the part number. If bonus flag is set then '/B'  
 * is removed from part number.                                       
 * Returns 0 if bonus flag has not been set, 1 if it has.            
 */
int
CheckBonus (
	char	*itemNumber)
{
	char	bonus_item [17];
	char	*wsptr;

	sprintf (bonus_item,"%-16.16s",itemNumber);
	wsptr = clip (bonus_item);

	if (strlen (wsptr) > 2)
	{
		wsptr += (strlen (wsptr) - 2);
		if (*wsptr == envVarSoSpecial [0]  && * (wsptr + 1) == envVarSoSpecial [1])
		{
			*wsptr = '\0';
			sprintf (itemNumber,"%-16.16s",bonus_item);
			return (EXIT_FAILURE);
		}
	}
	sprintf (itemNumber,"%-16.16s",bonus_item);
	return (EXIT_SUCCESS);
}

/*
 * Display Branch Status. 
 */
int
StatusDisplay (void)
{
	char	head_text [120];
	char	disp_str [300];
	char	tmp_str [8] [30];
	float	totals [6];
	float	wk_avail = 0.00;

	totals [0] = totals [1] = totals [2] = totals [3] = 0.00;
	totals [4] = totals [5] = 0.00;

	abc_selfield (ccmr, "ccmr_id_no");
	lpXoff = 1;
	lpYoff = 4;

	sprintf (head_text, " Item Number  : %16.16s     (%40.40s)  ",
		inmr_rec.item_no,
		inmr_rec.description);
	Dsp_prn_open (0, 4, 12, head_text, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec ("Warehouse. |    On Hand     |   Committed    |   Backorder    | Forward Order  |    Available   |    On Order.   ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			/*
			 * calculate available stock.
			 */
			if (envVarSoFwdAvl)
			{
				wk_avail = 
						n_dec (incc_rec.closing_stock, inmr_rec.dec_pt) -
						n_dec (incc_rec.committed, 
							   inmr_rec.dec_pt) -
						n_dec (incc_rec.backorder, inmr_rec.dec_pt) -
						n_dec (incc_rec.forward, inmr_rec.dec_pt);
			}
			else
			{
				wk_avail = 
						n_dec (incc_rec.closing_stock, inmr_rec.dec_pt) -
						n_dec (incc_rec.committed, 
							   inmr_rec.dec_pt) -
						n_dec (incc_rec.backorder, inmr_rec.dec_pt);
			}
			if (envVarQcApply && envVarSkQcAvl)
				wk_avail -= n_dec (incc_rec.qc_qty, inmr_rec.dec_pt);

			sprintf (tmp_str [0], local_rec.rep_qty, 
					n_dec (incc_rec.closing_stock, inmr_rec.dec_pt));
			sprintf (tmp_str [1], 
					 local_rec.rep_qty, 
					 n_dec (incc_rec.committed, 
							inmr_rec.dec_pt));
			sprintf (tmp_str [2], local_rec.rep_qty, 
					n_dec (incc_rec.backorder, inmr_rec.dec_pt));
			sprintf (tmp_str [3], local_rec.rep_qty, 
					n_dec (incc_rec.forward, inmr_rec.dec_pt));
			sprintf (tmp_str [4], local_rec.rep_qty, 
					n_dec (wk_avail, inmr_rec.dec_pt));
			sprintf (tmp_str [5], local_rec.rep_qty, 
					n_dec (incc_rec.on_order, inmr_rec.dec_pt));
			sprintf (disp_str, " %-9.9s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
				ccmr_rec.acronym,
				tmp_str [0],
				tmp_str [1],
				tmp_str [2],
				tmp_str [3],
				tmp_str [4],
				tmp_str [5]);
			Dsp_saverec (disp_str);

			totals [0] += n_dec (incc_rec.closing_stock, inmr_rec.dec_pt);
			totals [1] += n_dec (incc_rec.committed, 
								inmr_rec.dec_pt);
			totals [2] += n_dec (incc_rec.backorder, inmr_rec.dec_pt);
			totals [3] += n_dec (incc_rec.forward, inmr_rec.dec_pt);
			totals [4] += n_dec (wk_avail, inmr_rec.dec_pt);
			totals [5] += n_dec (incc_rec.on_order, inmr_rec.dec_pt);
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	strcpy (disp_str, "^^GGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG^^");
	Dsp_saverec (disp_str);

	sprintf (tmp_str [0], local_rec.rep_qty, totals [0]);
	sprintf (tmp_str [1], local_rec.rep_qty, totals [1]);
	sprintf (tmp_str [2], local_rec.rep_qty, totals [2]);
	sprintf (tmp_str [3], local_rec.rep_qty, totals [3]);
	sprintf (tmp_str [4], local_rec.rep_qty, totals [4]);
	sprintf (tmp_str [5], local_rec.rep_qty, totals [5]);
	sprintf (disp_str, " TOTALS    ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ^E %14s ",
		tmp_str [0],
		tmp_str [1],
		tmp_str [2],
		tmp_str [3],
		tmp_str [4],
		tmp_str [5]);
	Dsp_saverec (disp_str);
	Dsp_srch ();
	Dsp_close ();
/*
	if (wh_flag == TRUE)
		load_wh ();
*/
	TheReDraw ();
    return (EXIT_SUCCESS);
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	if (clearOK)
	{
		swide ();
		clear ();
	}

	print_at (0, SCREENWIDTH - 22 ,mlSkDisp [43], local_rec.lastCustomerNo);

	strcpy (err_str,mlSkDisp [44]);
	rv_pr (err_str, (SCREENWIDTH - 2 - strlen (err_str)) / 2, 0, 1);

	line_at (1,0, SCREENWIDTH);
	box (0, 2, SCREENWIDTH - 1, 15);
	line_at (6,1, SCREENWIDTH - 2);

	line_at (21, 0, SCREENWIDTH);

	print_at (22,0,ML (mlStdMess038), comm_rec.co_no, clip (comm_rec.co_name)); 
	print_at (22,45,ML (mlStdMess039), comm_rec.est_no, clip (comm_rec.est_name));
	scn_write (scn);
    return (EXIT_SUCCESS);
}

/*
 * Print missing graphics characters. 
 */
void
PrintMissGr (void)
{
	crsr_off ();
	move (44,5);
	PGCHAR (8);

	move (88,5);
	PGCHAR (8);

	move (44,13);
	PGCHAR (9);

	move (63,13);
	PGCHAR (8);

	move (0,13);
	PGCHAR (10);

	move (88,13);
	PGCHAR (9);

	move (130,13);
	PGCHAR (11);

	move (63,18);
	PGCHAR (9);
}

/*
 * Reverse Screen Discount. 
 */
float	
ScreenDisc (
	float	DiscountPercent)
{
	if (envVarSoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

void
SrchExaf (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Customer Area Description");
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", keyValue);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (exaf_rec.area_code, keyValue, strlen (keyValue)) &&
		   !strcmp (exaf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", temp_str);
	cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}
char	*
GetPriceDesc (
	int		priceNumber)
{

	switch (priceNumber)
	{
		case	0:
			return (comm_rec.price1_desc);
		break;

		case	1:
			return (comm_rec.price2_desc);
		break;

		case	2:
			return (comm_rec.price3_desc);
		break;

		case	3:
			return (comm_rec.price4_desc);
		break;

		case	4:
			return (comm_rec.price5_desc);
		break;

		case	5:
			return (comm_rec.price6_desc);
		break;

		case	6:
			return (comm_rec.price7_desc);
		break;

		case	7:
			return (comm_rec.price8_desc);
		break;

		case	8:
			return (comm_rec.price9_desc);
		break;

		default:
			return ("??????????");
	}
}

void
InitML (
 void)
{
	strcpy (mlSkDisp [0],  ML ("Customer"));
	strcpy (mlSkDisp [1],  ML ("Item"));
	strcpy (mlSkDisp [2],  ML ("Price For Quantity"));
	strcpy (mlSkDisp [3],  ML ("Discounts Allowed"));
	strcpy (mlSkDisp [4],  ML ("Current Pricing"));
	strcpy (mlSkDisp [5],  ML ("Current Price Value"));
	strcpy (mlSkDisp [6],  ML ("Discounts Applied"));
	strcpy (mlSkDisp [7],  ML ("Current Pricing Lvl"));
	strcpy (mlSkDisp [8],  ML ("Reg."));
	strcpy (mlSkDisp [9],  ML ("Discount A."));
	strcpy (mlSkDisp [10], ML ("Discount B."));
	strcpy (mlSkDisp [11], ML ("Discount C."));
	strcpy (mlSkDisp [12], ML ("Disc"));
	strcpy (mlSkDisp [13], ML ("Current Pricing For"));
	strcpy (mlSkDisp [14], ML ("Current Pricing Exp."));
	strcpy (mlSkDisp [15], ML ("Pricing Conversion"));
	strcpy (mlSkDisp [16], ML ("Current Discount Lvl"));
	strcpy (mlSkDisp [17], ML ("Nett Customer Price"));
	strcpy (mlSkDisp [18], ML ("Discount Total"));
	strcpy (mlSkDisp [19], ML ("Current Discount For"));
	strcpy (mlSkDisp [20], ML ("Yes"));
	strcpy (mlSkDisp [21], ML ("No "));
	strcpy (mlSkDisp [22], ML (mlSkMess063));
	strcpy (mlSkDisp [23], ML (mlSkMess064));
	strcpy (mlSkDisp [24], ML (mlSkMess065));
	strcpy (mlSkDisp [25], ML (mlSkMess066));
	strcpy (mlSkDisp [26], ML (mlSkMess067));
	strcpy (mlSkDisp [27], ML (mlSkMess068));
	strcpy (mlSkDisp [28], ML (mlSkMess069));
	strcpy (mlSkDisp [29], ML (mlSkMess070));
	strcpy (mlSkDisp [30], ML (mlSkMess115));
	strcpy (mlSkDisp [31], ML (mlSkMess116));
	strcpy (mlSkDisp [32], ML (mlSkMess117));
	strcpy (mlSkDisp [33], ML (mlSkMess631));
	strcpy (mlSkDisp [34], ML (mlSkMess071));
	strcpy (mlSkDisp [35], ML (mlSkMess072));
	strcpy (mlSkDisp [36], ML (mlSkMess073));
	strcpy (mlSkDisp [37], ML (mlSkMess074));
	strcpy (mlSkDisp [38], ML (mlSkMess075));
	strcpy (mlSkDisp [39], ML (mlSkMess076));
	strcpy (mlSkDisp [40], ML (mlSkMess077));
	strcpy (mlSkDisp [41], ML (mlSkMess078));
	strcpy (mlSkDisp [42], ML (mlSkMess079));
	strcpy (mlSkDisp [43], ML (mlSkMess080));
	strcpy (mlSkDisp [44], ML (mlSkMess118));

	strcpy (mlSkDisp [45], ML ("P/Order No "));
	strcpy (mlSkDisp [46], ML ("Supplier No"));
	strcpy (mlSkDisp [47], ML ("Del.Inst. 1"));
	strcpy (mlSkDisp [48], ML ("Del.Inst. 2"));
	strcpy (mlSkDisp [49], ML ("Del.Inst. 3"));
	strcpy (mlSkDisp [50], ML ("Exch Rate  "));
	strcpy (mlSkDisp [51], ML ("Shipment   "));
	strcpy (mlSkDisp [52], ML ("Ship Method"));
	strcpy (mlSkDisp [53], ML ("Ship Depart"));
	strcpy (mlSkDisp [54], ML ("Port Origin"));
	strcpy (mlSkDisp [55], ML ("Vessel     "));
	strcpy (mlSkDisp [56], ML ("Ship Arrive"));
	strcpy (mlSkDisp [57], ML ("Destination"));
	strcpy (mlSkDisp [58], ML ("AIR "));
	strcpy (mlSkDisp [59], ML ("SEA "));
	strcpy (mlSkDisp [60], ML ("LAND"));
	strcpy (mlSkDisp [61], ML ("NO SHIPMENT DETAILS"));
	strcpy (mlSkDisp [62], ML ("FOR CUST."));
	strcpy (mlSkDisp [63], ML ("FOR STOCK"));
	strcpy (mlSkDisp [64], ML ("WORKS ORDER FOR W/H "));
	strcpy (mlSkDisp [65], ML ("TOTALS"));
	strcpy (mlSkDisp [66], ML ("Pri Breaks"));
	strcpy (mlSkDisp [67], ML ("Base  "));

	strcpy (mlSkDisp [68], ML (mlSkMess056));
	strcpy (mlSkDisp [69], ML (mlSkMess057));
	strcpy (mlSkDisp [70], ML (mlSkMess058));
	strcpy (mlSkDisp [71], ML (mlSkMess059));
	strcpy (mlSkDisp [72], ML (mlSkMess060));
	strcpy (mlSkDisp [73], ML (mlSkMess061));
	strcpy (mlSkDisp [74], ML (mlSkMess062));
	 
}

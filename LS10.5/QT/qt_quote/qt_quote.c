/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: qt_quote.c,v 5.7 2002/11/28 04:09:48 scott Exp $
|  Program Name  : (qt_quote.c)
|  Program Desc  : (Quotation Input)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/01/1991       |
|---------------------------------------------------------------------|
| $Log: qt_quote.c,v $
| Revision 5.7  2002/11/28 04:09:48  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.6  2002/07/24 08:39:10  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/18 07:04:12  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/03 04:26:48  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qt_quote.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_quote/qt_quote.c,v 5.7 2002/11/28 04:09:48 scott Exp $";

#define MAXWIDTH 	150
#define MAXSCNS 	4
#define TABLINES 	10
#define MAXLINES 	100
#define MAX_PAR 	30
#define	SLEEP_TIME	3

#define SR   		store [line_cnt] 

#define	ADD_QT 		 (local_rec.action [0] == 'A')
#define	MOD_QT 		 (local_rec.action [0] == 'M')
#define	CPY_QT 		 (local_rec.action [0] == 'C')
#define	CNL_QT 		 (local_rec.action [0] == 'L')
#define	SAL_QT 		 (local_rec.action [0] == 'S')

#define	MULT_QTY	 (SR.costingFlag [0] != 'S')

#define NO_KEY(x)	 (vars [x].required == NA || \
			  		  vars [x].required == NI || \
			  		  vars [x].required == ND)

#define HIDE(x)	 	 (vars [x].required == ND)
#define NEED(x)	 	 (vars [x].required == YES)

#define	FGN_CURR	 (envVar.dbMcurr && strcmp (cumr_rec.curr_code, envVar.currencyCode))
#define	FREIGHT_CHG (local_rec.frei_req [0] == 'Y')  
#define	NON_STOCK	 (SR.itemClass [0] == 'Z')

#include <std_decs.h>
#include <ml_qt_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fprintf	Remote_fprintf
#define	fclose	Remote_fclose
#define	fflush	Remote_fflush
#endif	/* GVISION */

	/*
	 * Special fields and flags. 
	 */
	int		newQuote,
			notax 		= 0,
			blankWin 	= TRUE,
			specialNo	= FALSE,
			transferqt	= FALSE,
			val_pterms	= FALSE,
			insideLine = FALSE,
			partPrinted = FALSE,
			prog_type,
			specialDisplay = FALSE,
			oldLines  	= 0;

	int		defaultDiscFlag = NI;

	float	ScreenDisc (float);
	float	ToStdUom (float);
	float	ToLclUom (float);

	double	inv_tot 	= 0.00,
			min_tot		= 0.00,  
			dis_tot 	= 0.00,
			tax_tot 	= 0.00,
			gst_tot  	= 0.00,
			cst_tot  	= 0.00,
			tot_tot  	= 0.00,
			sal_tot		= 0.00,
			l_cost   	= 0.00,
			l_min		= 0.00,
			l_sale	   	= 0.00,  
			t_total  	= 0.00,
			TotalNett	= 0.00,
			TotalGross	= 0.00,
			l_total  	= 0.00,
			l_nett  	= 0.00,
			l_disc   	= 0.00,
			l_gst    	= 0.00,
			l_tax    	= 0.00,
			l_bgp    	= 0.00,
			l_agp 	    = 0.00,
			c_left   	= 0.00;

	FILE	*fout;

	char	*curr_user;

	struct	{
		char	*_scode;
		char	*_sterm;

	} s_terms [] = {
		{"   ","Local                 "},
		{"CIF","Cost Insurance Freight"},
		{"C&F","Cost & Freight"},
		{"FIS","Free Into Store"},
		{"FOB","Free On Board"},
		{"",""},
	};

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exsiRecord	exsi_rec;
struct ccmrRecord	ccmr_rec;
struct esmrRecord	esmr_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct cumrRecord	cumr_rec;
struct cudiRecord	cudi_rec;
struct cfhrRecord	cfhr_rec;
struct cflnRecord	cfln_rec;
struct cuitRecord	cuit_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct qthrRecord	qthr_rec;
struct qtlnRecord	qtln_rec;
struct qtlpRecord	qtlp_rec;
struct qtlhRecord	qtlh_rec;
struct qtldRecord	qtld_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct pocrRecord	pocrRec;
struct pocrRecord	pocr2_rec;
struct sumrRecord	sumr_rec;
struct tmpfRecord	tmpf_rec;

	char	*data = "data",
			*inum2 = "inum2";

	char	branchNumber [3],
			read_flag [2];
	char	*ser_space = "                         ";
	long 	cur_ldate;

static double FindCostValue (int);

	char	*scn_desc [] = 
	{
		"Quotation Header Screen.",
		"Quotation line Item Screen.",
		"Quotation Paragraph Screen.",
		"Quotation Footer Screen."
	};

	/*
	 * Local & Screen Structures. 
	 */
	struct storeRec {
		long	hhbrHash;
		long	hhccHash;
		long	hhwhHash;
		long	hhsiHash;
		long	hhumHash;
		float	quantity;
		float	qtyAvailable;
		float	defaultDisc;
		float	disPc;
		float	regPc;
		float	discA;
		float	discB;
		float	discC;
		int		cumulative;
		float	calcDisc;
		float	taxPc;
		float	gstPc;
		float	outerSize;
		float	weight;
		double	taxAmount;
		double	costPrice;
		double  min_sell_pric;
		double	grossSalePrice;
		double	salePrice;
		double	actSalePrice;
		double	calcSalePrice;
		double  sale;
		char	priceOveride [2];
		char	discOveride [2];
		char	serialNo [26];
		char	origSerialNo [26];
		char	serialFlag [2];
		char	itemClass [2];
		char	costingFlag [2];
		char	category [12];	
		char	sellGroup [7];	
		char 	uom [5];
		int		contractPrice;
		int		contractStatus;
		int		contractCost;
		int		pricingCheck;
		int		origLine;
		char	alternate [2];
		int		indent;
		int		decPt;
		float   cnvFct;
	} store [MAXLINES];

	long	_hhlh_hash [MAX_PAR];
	char	_par_desc [MAX_PAR][41];

/*
 * The structure envVar groups the values of 
 * environment settings together.            
 */
struct tagEnvVar
{
	char	gstCode [4];
	int		creditOver;
	int		creditStop;
	int		creditTerms;
	int		serialItemsOk;
	char	currencyCode [4];
	int		dbMcurr;
	int		qtLmargin;
	char	qtDirectory [51];
	int		inpDisc;
	int		gstApplies;
	int		dbCo;
	int		dbFind;
	int		serialItemOk;
	int		includeForwardStock;
	int		reverseDiscount;
	int		discountIndents;
	int		useSystemDate;
} envVar;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	char 	action [2];
	char	item_no [17];
	long	item_hash;
	char	sup_part [17];
	float	qty;
	float	qty_avail;
	float	weight;
	double	cost_price;
	char	par_code [11];
	char	serial_no [26];
	long	n_quote_no;
	char	cont_no [7];
	char	quote_no [9];
	char	cont_desc [21];
	char	stat_desc [41];
	char	pri_desc [41];
	char	sell_desc [31];
	char	reset_flag [2];
	char	carr_code [5];
	char	carr_area [3];
	char	carr_desc [41];
	char	carr_adesc [41];
	char	co_no [3];
	char	br_no [3];
	char	sell_terms [4];
	char	pay_term [41];
	char	sos [2];
	char	UOM [5];
} local_rec;

float 	dis_dec,
	  	disc_amt,	
	  	net_dis_pc;
double 	net_sal_tot;

static	struct	var	vars [] =
{
	{1, LIN, "action",	 4, 22, CHARTYPE,
		"U", "          ",
		" ", "", "Action (A,C,L,M,S)", "Enter A (dd Quote),  C (opy Quote),  L (Cancel a Quote), M (odify a Quote),  Create a S (ales order) from a Quote.",
		 NE, NO,  JUSTLEFT, "ACLMS", "", local_rec.action},
	{1, LIN, "n_quote",	 5, 22, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Quote Number.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.quote_no},
	{1, LIN, "qt_stat",		6, 75, CHARTYPE,
		"UU", "          ",
		" ", "00", "Quote Status Code.", " Enter Quote Status Code",
		 NE, NO, JUSTLEFT, "", "", qthr_rec.status},
	{1, LIN, "qt_stat_desc",	6, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.stat_desc}, 
	{1, LIN, "cust_no",	 4, 75, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer No.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", cumr_rec.dbt_no},
	{1, LIN, "cust_name",	 4, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", qthr_rec.dbt_name},
	{1, LIN, "ord_no",	 5, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Customer Enq. Ref. ", " ",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.enq_ref},
	{1, LIN, "cont_no",		6, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract. ", " Enter Contract If Contract Prices Available - Search Available For This Customers Contracts",
		 NE, NO, JUSTLEFT, "", "", qthr_rec.cont_no},
	{1, LIN, "cont_desc",	6, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.cont_desc},
	{1, LIN, "con_name",	8, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Contact name.", "Enter Customer Contact Name.",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.cont_name},
	{1, LIN, "pos_code",	9, 22, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Contact Position.", "Enter Position Code.",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.pos_code},
	{1, LIN, "pos_desc",	10, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Contact Position Desc.", "Enter Customer Contact Name.",
		NA, NO,  JUSTLEFT, "", "", tmpf_rec.pos_desc},
	{1, LIN, "sman",	 11, 22, CHARTYPE,
		"UU", "          ",
		" ", cumr_rec.sman_code, "Salesman Code.", "",
		 NO, NO, JUSTRIGHT, "", "", qthr_rec.sman_code},
	{1, LIN, "sman_name",	 12, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Salesman Desc.", "",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "del_name",	 8, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Name  :", " Select Delivery Name and Address. Search available. ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.del_name},
	{1, LIN, "del_addr1",	9, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Address        :", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.del_add1},
	{1, LIN, "del_addr2",	10, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "              :", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.del_add2},
	{1, LIN, "del_addr3",	11, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "              :", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.del_add3},
	{1, LIN, "del_addr4",	12, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "              :", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.del_add4},
	{1, LIN, "dt_fst_call",	14, 22, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "First Call Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.dt_fst_call},
	{1, LIN, "dt_lst_call",	14, 75, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Last Call Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.dt_lst_call},
	{1, LIN, "dt_quote",	14, 115, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Quotation Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.dt_quote},
	{1, LIN, "dt_follow_up",	15, 22, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Next Follow-up Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.dt_follow_up},
	{1, LIN, "exp_date",	15, 75, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Quote Expiry Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.expire_date},
	{1, LIN, "no_calls",	16, 22, INTTYPE,
		"N", "          ",
		" ", "1", "Total number of Calls.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.no_calls},
	{1, LIN, "place_ord",	16, 75, CHARTYPE,
		"U", "          ",
		" ", "N", "Order to Place: ", "Y=Order to Place, N=No order to place <default = N>",
		YES, NO,  JUSTLEFT, "YN", "", qthr_rec.place_ord},
	{1, LIN, "del_date",	16, 115, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Delivery Date.", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.del_date},
	{1, LIN, "comm1",	18, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Comments #1: ", " ",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.comm1},
	{1, LIN, "comm2",	19, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Comments #2: ", " ",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.comm2},
	{1, LIN, "comm3",	20, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Comments #3: ", " ",
		YES, NO,  JUSTLEFT, "", "", qthr_rec.comm3},
	{1, LIN, "salute",	18, 75, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Salutation for Letter.", "Enter Letter Salutation.",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.salute},
	{1, LIN, "pri_type",	19, 75, CHARTYPE,
		"N", "        ",
		" ", cumr_rec.price_type, "Price Type.     ", "Enter Price Type",
		 NA, NO,  JUSTLEFT, "", "", qthr_rec.pri_type},
	{1, LIN, "pri_desc",	19, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.pri_desc},
	{1, LIN, "fix_exch",	20, 75, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exchange Rate  ", " ",
		YES, NO,  JUSTLEFT, "YN", "", qthr_rec.fix_exch},
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " Default : Deletes Line ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "descr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "    I t e m   D e s c r i p t i o n .   ", " ",
		YES, NO,  JUSTLEFT, "", "", qtln_rec.item_desc},
	{2, TAB, "item_hash",	0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "0", "", " ",
		ND, NO,  JUSTRIGHT, "0", "", (char *)&qtln_rec.hhbr_hash},
	{2, TAB, "uom",	0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", "Unit Of Measure ",
		NO, NO,  JUSTLEFT, "", "", local_rec.UOM},
	{2, TAB, "qty_avail",	 0, 0, FLOATTYPE,
		"NNNNNNNNN", "          ",
		" ", "1", "Available", " ",
		NA, NO, JUSTRIGHT, "0.00", "999999999", (char *)&local_rec.qty_avail},
	{2, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Quantity", " ",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *)&local_rec.qty},
	{2, TAB, "cost_price",	 0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Cost Price", " Cost Price ",
		YES, NO, JUSTRIGHT, "-999999.99", "999999.99", (char *)&local_rec.cost_price},
	{2, TAB, "sale_price",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "           ",
		" ", "0.00", "Sale Price.", " Sale Price ",
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *)&qtln_rec.sale_price},
	{2, TAB, "disc",	 0, 0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Disc ", " ",
		 NO, NO, JUSTRIGHT, "-999.99", "999.99", (char *)&qtln_rec.disc_pc},
	{2, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Serial Number      ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{2, TAB, "sub_tot",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", " ", "S", "Sub total Print <default '*'>",
		 NO, NO,  JUSTLEFT, "*", "", qtln_rec.st_flag},

	{3, TAB, "par_code",	MAX_PAR, 3, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Paragraph Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.par_code},

	{4, LIN, "sos_ok",	 2, 18, CHARTYPE,
		"U", "          ",
		" ", "", "S.O Surcharge. (Y/N)", "Enter N(o) to Overide Small Order Surcharge.",
		NO, NO, JUSTRIGHT, "YN", "", qthr_rec.sos},
	{4, LIN, "pay_term",	 3, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.", " ",
		NO, NO,  JUSTLEFT, "", "", qthr_rec.pay_term},
	{4, LIN, "sell_terms",	 4, 18, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Sell Terms.", " ",
		NO, NO,  JUSTLEFT, "", "", qthr_rec.sell_terms},
	{4, LIN, "sell_desc",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Sell Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sell_desc},
	{4, LIN, "carr_code",	 7, 18, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Carrier Code.", "Enter carrier code, [SEARCH] available.",
		NO, NO,  JUSTLEFT, "", "", qthr_rec.carr_code},
	{4, LIN, "carr_desc",	 7, 44, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.carr_desc},
	{4, LIN, "carr_area",	 8, 18, CHARTYPE,
		"UU", "          ",
		" ", "", "Area code.", "Enter area code for carrier, [SEARCH] available ",
		NO, NO, JUSTRIGHT, "", "", qthr_rec.carr_area},
	{4, LIN, "ca_desc",	 8, 44, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.carr_adesc},
	{4, LIN, "freight",	 9, 18, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Freight Amount.", " ",
		NO, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&qthr_rec.freight},
	{4, LIN, "tot_kg",	 10, 18, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Total Kgs.", " ",
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&qthr_rec.no_kgs},
	{4, LIN, "reset_price",	11,18, CHARTYPE,
		"U", "          ",
		" ", "N", "Reset Prices.", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.reset_flag},
	{4, LIN, "misc_charge1",	13, 18, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Misc. Charge.", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&qthr_rec.misc_charge1},
	{4, LIN, "misc_charge2",	14, 18, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Misc. Charge.", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&qthr_rec.misc_charge2},
	{4, LIN, "misc_charge3",	15, 18, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "Misc. Charge.", " ",
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&qthr_rec.misc_charge3},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

double	FindCostValue		 (int);
float	ScreenDisc		 	 (float);
float	ToLclUom		 	 (float);
float	ToStdUom		 	 (float);
int		AddSoln		 		 (int);
int		CheckAlternate	 	 (char *);
int		CheckDuplicateInsf	 (char *, long, int);
int		CheckSohr		 	 (long);
int		CommitInsf		 	 (long, char *);
int		CopyQuote		 	 (long);
int		CreateSord		 	 (long);
int		DeleteItemLine		 (void);
int		DeleteParaLine		 (void);
int		InsertLine		 	 (void);
int		InsertParaLine		 (void);
int		LoadDisplay	 		 (char *);
int		LoadQtln		 	 (long);
int		Read_comm		 	 (void);
int		SpecItem		 	 (void);
int		Update			 	 (void);
int		ValidCmd		 	 (char *);
int		ValidItemNo	 		 (int);
int		WarnUser		 	 (char *, int);
int		main				 (int argc, char * argv []);
int		win_function2	 	 (int, int, int, int);
int 	SrchCudi 		 	 (int);
void	AddSohr		 		 (void);
void	Busy			 	 (int);
void	CalExtend		 	 (int);
void	CalcTotal		 	 (int);
void	CalculateFreight	 (float, double);
void	CheckEnvironment	 (void);
void	CloseDB		 		 (void);
void	DiscProcess		 	 (void);
void	HiLight			 	 (char *);
void	NoCustomer			 (void);
void	OpenDB			 	 (void);
void	Parse			 	 (char *);
void	PriceProcess	 	 (void);
void	PrintAmount		 	 (double, int);
void	PrintCoStuff		 (void);
void	PrintLetter		 	 (void);
void	PrintTotal		 	 (void);
void	SetPriceDesc 		 (void);
void	ShowArea		 	 (char *);
void	ShowCarr		 	 (char *);
void	ShowPay		 		 (void);
void	ShowPrice		 	 (void);
void	ShowSell		 	 (void);
void	SrchCnch		 	 (char *);
void	SrchExsf		 	 (char *);
void	SrchInsf		 	 (char *, int);
void	SrchInum		 	 (char *);
void	SrchQthr		 	 (char *);
void	SrchQtln		 	 (char *);
void	SrchStatus		 	 (void);
void	SrchTmpf		 	 (char *);
void	SubstitudeCommand	 (int);
void	UpdateQtlp		 	 (int);
void	shutdown_prog	 	 (void);
void	tab_other		 	 (int);

#include <twodec.h>
#include <proc_sobg.h>
#include <RealCommit.h>
#include <qt_commands.h>
#include <cus_price.h>
#include <cus_disc.h>
#include <neg_win.h>
#include <qt_status.h>
#include <p_terms.h>
#include <FindCumr.h> 
#include <find_cuit.h>
#include <Costing.h>

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	int		j;
	int		iMainRet = EXIT_SUCCESS;


	if (argc != 2 && argc != 3)
	{
		print_at (0,0, mlStdMess071, argv [0]);
		return (EXIT_SUCCESS);
	}
	specialDisplay = FALSE;
	if (argc == 3)
	{
		if (strcmp (argv [2], " "))
		{
			specialDisplay = TRUE;
			strcpy (read_flag, "r");
		} 
		else
			specialDisplay = FALSE;
	}

	/*
	 * Check environment variables and set values in the envVar structure. 
	 */
	CheckEnvironment ();

	SETUP_SCR (vars);


	tab_row = 7;
	init_scr ();
	set_tty (); 
	_set_masks (argv [1]);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	cur_ldate = TodaysDate ();

	/*
	 * Read common terminal record. 
	 */
	Read_comm ();

	defaultDiscFlag = vars [label ("disc")].required;

	strcpy (branchNumber, (!envVar.dbCo) ? " 0" : comm_rec.est_no);

	clear ();
	swide ();

	FLD ("qty_avail")	=	 (envVar.serialItemOk) ? ND	: NA;
	FLD ("ser_no")		=	 (envVar.serialItemOk) ? YES	: ND;

	/*
	 * Open main database files. 
	 */
	OpenDB ();
	OpenPrice ();
	OpenDisc ();

	_win_func = TRUE;

	while (prog_exit == 0)
	{
		if (restart) 
		{
			abc_unlock (qthr);
			abc_unlock (qtln);
			abc_unlock (qtlp);
			/*
			 * Free any outstanding insfRecords 
			 */

			if (line_cnt > lcount [2])
				lcount [2] = line_cnt;

			for (i = 0; i < lcount [2]; i++)
			{
				if (strcmp (store [i].serialNo, ser_space))
				{
					cc = 	UpdateInsf 
							 (
								store [i].hhwhHash,
								0L,
								store [i].serialNo,
								"C",
								"F"
							);

					if (cc && cc < 1000)
						file_err (cc, insf, "DBUPDATE");
				}
				if (strcmp (store [i].origSerialNo, ser_space))
				{
					cc = 	UpdateInsf 
							 (
								store [i].hhwhHash,
								0L,
								store [i].origSerialNo,
								"F",
								"C"
							);

					if (cc && cc < 1000)
						file_err (cc, insf, "DBUPDATE");
				}
			}
		}
		notax = 0;
		for (i = 0; i < MAX_PAR ; i++)
		{
			_hhlh_hash [i] = 0L;
			sprintf (_par_desc [i], "%-40.40s", " ");
		}
		for (i = 0; i < MAXLINES; i++)
		{
			memset (store + i, 0, sizeof (struct storeRec));
			store [i].origLine   = -1;
			strcpy (store [i].priceOveride,       "N");
			strcpy (store [i].discOveride,       "N");
			strcpy (store [i].serialNo,       ser_space);
			strcpy (store [i].origSerialNo,     ser_space);
		}
		eoi_ok     = TRUE;
		search_ok  = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		skip_entry = FALSE;
		newQuote  = FALSE;
		init_ok    = TRUE;
		skip_tab   = FALSE;
		specialNo = FALSE;
		transferqt = FALSE;
		init_vars (1);	
		init_vars (2);	
		init_vars (3);	
		init_vars (4);	
		lcount [2]	= 0;
		lcount [3]	= 0;

		/*
		 * Enter screen 1 linear input. 
		 */
		if (!specialDisplay)
		{
			heading (1);
			entry (1);
		}
		else
		{
			if (LoadDisplay (argv [2]))
			{
				clear ();
				box (0,0,36,1);
				print_at (1,1,"%R %s ", ML (mlStdMess016));
				sleep (sleepTime);
				iMainRet = -1;
				prog_exit = 1;
				continue;		/* break can be used here */
			}
			heading (1);

			/* for QUOTATION HEADER */
			
#ifdef GVISION
			FLD ("action") 		= NA;
#else
			FLD ("action") 		= ND;		
#endif	/* GVISION */
			FLD ("n_quote") 	= NA;		
			FLD ("cont_no") 	= NA;		
			FLD ("cust_no")		= NA;		
			FLD ("ord_no") 		= NA;		
			FLD ("qt_stat") 	= NA;		
			FLD ("con_name")	= NA;		
			FLD ("pos_code")	= NA;		
			FLD ("sman") 		= NA;		
			FLD ("del_name")	= NA;		
			FLD ("del_addr1") 	= NA;		
			FLD ("del_addr2") 	= NA;		
			FLD ("del_addr3") 	= NA;		
			FLD ("del_addr4") 	= NA;		
			FLD ("dt_fst_call")	= NA;		
			FLD ("dt_lst_call")	= NA;		
			FLD ("dt_quote")	= NA;		
			FLD ("dt_follow_up")= NA;		 
			FLD ("exp_date") 	= NA;		
			FLD ("no_calls") 	= NA;		
			FLD ("place_ord") 	= NA;		
			FLD ("del_date") 	= NA;		
			FLD ("comm1")		= NA;		
			FLD ("comm2")		= NA;		
			FLD ("comm3")		= NA;		
			FLD ("salute") 		= NA;		

			/* for Item Line */
			
			FLD ("item_no")		= NA;
			FLD ("descr")		= NA;		
			FLD ("uom")			= NA;		
			FLD ("qty")			= NA;		
			FLD ("cost_price")	= NA;		
			FLD ("sale_price") 	= NA;		
			FLD ("disc") 		= NA;		
			FLD ("sub_tot") 	= NA;		
			FLD ("par_code") 	= NA;		

			/* for footer screen */

			FLD ("sos_ok")		= NA;		
			FLD ("pay_term")	= NA;		
			FLD ("sell_terms")	= NA;		
			FLD ("carr_code")	= NA;		 
			FLD ("carr_area")	= NA;		 
			FLD ("freight")		= NA;		 
			FLD ("reset_price")	= NA;		 
			FLD ("misc_charge1")= NA;		 
			FLD ("misc_charge2")= NA;		 
			FLD ("misc_charge3")= NA;		 

			scn_display (1);
		}
 
		if (prog_exit || restart)
			continue;

		if (newQuote == 1 && entry_exit == 0)
		{
			/*
			 * Enter screen 2 tabular input. 
			 */
			scn_set (2);
			lcount [2] = 0;
			heading (2);
			entry (2);
	
			if (restart)
				continue;

			/*
			 * Enter screen 3 tabular input. 
			 */
			scn_set (3);
			lcount [3] = 0;
			heading (3);
			entry (3);

			if (restart)
				continue;

			/*
			 * Enter screen 4 footer input. 
			 */
			scn_set (4);
			heading (4);
			scn_display (4);
			DSP_FLD ("reset_price");
			edit (4); 

			if (restart)
				continue;
		}
		else
			scn_display (1);

		edit_all ();
		if (restart)
			continue;

		if (!restart && !specialDisplay)
		{
			if (min_tot <= sal_tot)
			{
				if (strcmp (qthr_rec.status, "20"))
				{
					j = prmptmsg (ML (mlQtMess019), "YyNn",1,23);
					move (1,23); 
					cl_line ();
					if (j != 'Y' && j != 'y')
						strcpy (qthr_rec.status, "00");
					else
					{
						strcpy (qthr_rec.status, "20");
						transferqt = TRUE;
					}
				}
			}
			else
			{
				strcpy (qthr_rec.status, "45");
				print_mess (ML (mlQtMess042));
				sleep (sleepTime);
				clear_mess ();
			}
			Update ();
		}
		if (!restart && specialDisplay)
				prog_exit = 1;
	}

	if (specialDisplay)
	 	argv [2] = " ";
	shutdown_prog ();
	return (iMainRet);
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
 * Open Database Files. 
 */
void
OpenDB (void)
{
    abc_alias (inum2 , inum);

	open_rec (cncd,  cncd_list, cncd_no_fields, "cncd_id_no2");
	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envVar.dbFind) ? "cumr_id_no"
							       						 : "cumr_id_no3");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (qthr,  qthr_list, QTHR_NO_FIELDS, "qthr_id_no");
	open_rec (qtln,  qtln_list, QTLN_NO_FIELDS, "qtln_id_no");
	open_rec (qtld,  qtld_list, QTLD_NO_FIELDS, "qtld_id_no");
	open_rec (qtlh,  qtlh_list, QTLH_NO_FIELDS, "qtlh_id_no");
	open_rec (qtlp,  qtlp_list, QTLP_NO_FIELDS, "qtlp_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no"); 
	open_rec (soic,  soic_list, soic_no_fields, "soic_id_no2"); 
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom"); 
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2"); 
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (cncd);
	abc_fclose (cnch);
	abc_fclose (cuit);
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (qthr);
	abc_fclose (qtld);
	abc_fclose (qtlh);
	abc_fclose (qtln);
	abc_fclose (qtlp);
	abc_fclose (soic); 
	abc_fclose (exaf); 
	abc_fclose (inum); 
	abc_fclose (inum2); 
	abc_fclose (pocr);

	ClosePrice ();
	CloseDisc ();

	SearchFindClose ();
	abc_dbclose (data);
}

/*
 * Get common info from commom database file. 
 */
int
Read_comm (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (ccmr);
	abc_fclose (comr);

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int	field)
{
	int	i = 0;

	double	total_owing  = 0.00;
	double	wk_value 	 = 0.00;
	
	/*
	 * validate debtors contract
	 */
	if (LCHECK ("cont_no"))
	{
		if (SRCH_KEY)
		{
	 		open_rec (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_id_no"); 
			SrchCnch (temp_str);
	 		abc_fclose (cncl); 
			return (EXIT_SUCCESS);
		}
		sprintf (local_rec.cont_desc, "%-20.20s", " ");
		if (dflt_used)
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
			return (EXIT_SUCCESS);
		}
		
		strcpy (cnch_rec.co_no,   comm_rec.co_no);
		strcpy (cnch_rec.cont_no, qthr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * now see if contract is still current.    
		 */
		if (cnch_rec.date_exp < cur_ldate)
		{
			sprintf (err_str,ML (mlQtMess026),DateToString (cnch_rec.date_exp));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cnch_rec.date_wef > cur_ldate)
		{
			sprintf (err_str,ML (mlQtMess027),DateToString (cnch_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * now see if contract is assigned to debtor
		 */
	 	open_rec (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_id_no"); 
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
	 		abc_fclose (cncl); 
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.cont_desc,"%-20.20s", cnch_rec.desc);
		if (FLD ("cont_desc") != ND)
			DSP_FLD ("cont_desc");
	 	abc_fclose (cncl); 
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Contact Name. 
	 */
	if (LCHECK ("con_name"))
	{
		if (dflt_used)
		{
			strcpy (qthr_rec.cont_name, cumr_rec.contact_name);
			DSP_FLD ("con_name");
			return (EXIT_SUCCESS);
		}
		DSP_FLD ("con_name");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Action. 
	 */
	if (LCHECK ("action"))
	{
		strcpy (local_rec.reset_flag, "N");
		if (ADD_QT)
		{
			FLD ("n_quote") = NA;
			newQuote = TRUE;
			strcpy (qthr_rec.status, "00");
			for (i = 0;strlen (q_status [i]._stat);i++)
			{
				if (!strncmp (qthr_rec.status, q_status [i]._stat,strlen (q_status [i]._stat)))
				{
					sprintf (local_rec.stat_desc,"%-40.40s",q_status [i]._desc);
					break;
				}
			}
			DSP_FLD ("qt_stat");
			DSP_FLD ("qt_stat_desc");
			FLD ("qt_stat") = NA;
		}
		if (CPY_QT || SAL_QT || MOD_QT)
			FLD ("n_quote") = YES;
		
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Purchase Order Number. 
	 */
	if (LCHECK ("n_quote"))
	{
		if (ADD_QT)
		{
			strcpy (local_rec.quote_no, "00000000");
			strcpy (qthr_rec.quote_no,local_rec.quote_no);
			strncpy (qthr_rec.pay_term, " ",40); 
			strncpy (qthr_rec.sell_terms, " ",3); 
			DSP_FLD ("n_quote");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchQthr (temp_str);
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("n_quote");
			return (EXIT_SUCCESS);
		}

		strcpy (qthr_rec.quote_no, zero_pad (local_rec.quote_no,8));
		strcpy (local_rec.quote_no, qthr_rec.quote_no); 
		DSP_FLD ("n_quote"); 

		/*
		 * Check if order is on file. 
		 */
		abc_selfield (qthr,"qthr_id_no2");
		strcpy (qthr_rec.co_no, comm_rec.co_no);
		strcpy (qthr_rec.br_no, comm_rec.est_no);
		if (!find_rec (qthr, &qthr_rec, EQUAL, "w"))
		{

			if (strcmp (qthr_rec.cont_no, "      ") != 0)
			{
				strcpy (cnch_rec.co_no, comm_rec.co_no);
				strcpy (cnch_rec.cont_no, qthr_rec.cont_no);
				cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
				if (cc)
					file_err (cc, "cnch", "DBFIND");
	
				sprintf (local_rec.cont_desc,"%-20.20s", cnch_rec.desc);
			}
			else
				strcpy (local_rec.cont_desc, "                    ");
			
			open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");

			strcpy (tmpf_rec.co_no, qthr_rec.co_no);
			strcpy (tmpf_rec.pos_code, qthr_rec.pos_code);
			cc = find_rec (tmpf, &tmpf_rec, EQUAL, "r");
			if (cc)
				sprintf (tmpf_rec.pos_desc,"%-20.20s", "                    ");

			abc_fclose (tmpf);

			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			strcpy (local_rec.sos, qthr_rec.sos);
			local_rec.n_quote_no = qthr_rec.hhqt_hash;

			SetPriceDesc ();

		    /*
		     * Quote is to be copied. 
		     */
			if (CNL_QT)
			{
				if (!strcmp (qthr_rec.status, "80"))
				{
					print_mess (ML (mlQtMess032));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				else
				{
					if (strcmp (qthr_rec.status, "00") && 
						strcmp (qthr_rec.status, "10") && 
						strcmp (qthr_rec.status, "20") && 
						strcmp (qthr_rec.status, "30") &&      
						strcmp (qthr_rec.status, "40") && 
						strcmp (qthr_rec.status, "45"))
					{
						print_mess (ML (mlQtMess033));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
					else
					{
						sprintf (err_str,ML (mlQtMess020), local_rec.quote_no);
						i = prmptmsg (err_str, "YyNn",1,2);
						move (1,2); 
						cl_line ();
						if (i != 'Y' && i != 'y')
						{
							restart = TRUE;
							abc_unlock (qthr);
							return (EXIT_SUCCESS);
						}
						else
						{
							strcpy (qthr_rec.status, "80");
							cc = abc_update (qthr, &qthr_rec);
							if (cc) 
								file_err (cc, "qthr", "DBUPDATE");
							
							abc_unlock (qthr);
							restart = TRUE;
							return (EXIT_SUCCESS);
						}
					}
				}
			}

			if (MOD_QT)
			{
				if (strcmp (qthr_rec.status, "00") && 
					strcmp (qthr_rec.status, "10") && 
					strcmp (qthr_rec.status, "20") && 
					strcmp (qthr_rec.status, "50") && 
					strcmp (qthr_rec.status, "45")) 
				{
					print_mess (ML (mlQtMess034));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
					

				for (i = 0;strlen (q_status [i]._stat);i++)
				{
					if (!strncmp (qthr_rec.status, q_status [i]._stat,strlen (q_status [i]._stat)))
					{

						sprintf (local_rec.stat_desc,"%-40.40s",q_status [i]._desc);
						break;

					}
				}

				DSP_FLD ("qt_stat");
				DSP_FLD ("qt_stat_desc");

				strcpy (local_rec.sos, qthr_rec.sos);

				open_rec (cfhr,cfhr_list,CFHR_NO_FIELDS,"cfhr_id_no");
				open_rec (cfln,cfln_list,CFLN_NO_FIELDS,"cfln_cfhh_hash");

				strcpy (cfhr_rec.co_no, comm_rec.co_no);
				strcpy (cfhr_rec.br_no, comm_rec.est_no);
				strcpy (cfhr_rec.carr_code, qthr_rec.carr_code);
				cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
				if (!cc)
				{
					cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
					cc = find_rec (cfln,&cfln_rec,EQUAL,"r"); 
					if (!cc)
					{
						strcpy (exaf_rec.co_no,comm_rec.co_no);
						strcpy (exaf_rec.area_code,cfln_rec.area_code);
						cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
						if (!cc)
						{
							strcpy (local_rec.carr_adesc, exaf_rec.area);
							strcpy (local_rec.carr_desc, cfhr_rec.carr_desc);
						}
					}
				}

				abc_fclose (cfhr);
				abc_fclose (cfln);

				strcpy (local_rec.carr_code, qthr_rec.carr_code);	
				strcpy (local_rec.carr_area, qthr_rec.carr_area);	
			}

			if (CPY_QT)
			{
				if (!strcmp (qthr_rec.status, "45"))
				{
					print_mess (ML (mlQtMess035));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				sprintf (err_str,ML (mlQtMess021),local_rec.quote_no);

				i = prmptmsg (err_str, "YyNn",1,2);
				move (1,2); 
				cl_line ();
				if (i != 'Y' && i != 'y')
				{
					restart = TRUE;
					abc_unlock (qthr);
					return (EXIT_SUCCESS);
				}
				cc = CopyQuote (local_rec.n_quote_no);
				if (cc)
				{
					print_mess (ML (mlQtMess036));
					sleep (sleepTime);
					clear_mess ();
				}
				abc_unlock (qthr);
				restart = TRUE;
				return (EXIT_SUCCESS);
			}
			if (SAL_QT)
			{
				if (!strcmp (qthr_rec.status, "95"))
				{
					print_mess (ML (mlQtMess037));
					sleep (sleepTime);
					clear_mess ();
					restart = TRUE;
					return (EXIT_SUCCESS);
				}
				if (strcmp (qthr_rec.status, "90"))
				{
					print_mess (ML (mlQtMess038));
					sleep (sleepTime);
					clear_mess ();
					restart = TRUE;
					return (EXIT_SUCCESS);
				}
				
				sprintf (err_str,ML (mlQtMess022),local_rec.quote_no);
				i = prmptmsg (err_str, "YyNn",1,2);
				move (1,2); 
				cl_line ();
				if (i != 'Y' && i != 'y')
				{
					restart = TRUE;
					abc_unlock (qthr);
					return (EXIT_SUCCESS);
				}
				if (qthr_rec.hhcu_hash == 0L)
				{
					do
					{
						get_entry (label ("cust_no"));
						cc = spec_valid (label ("cust_no"));
					} while (cc && !restart);
					DSP_FLD ("cust_no");
					sleep (sleepTime);
					qthr_rec.hhcu_hash = cumr_rec.hhcu_hash;
				}

				open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
				strcpy (exsf_rec.co_no, comm_rec.co_no);
				strcpy (exsf_rec.salesman_no, qthr_rec.sman_code);
				cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlQtMess040));
					sleep (sleepTime);
					clear_mess ();
					restart = TRUE;
					return (EXIT_SUCCESS);
				}
				abc_fclose (exsf);
				cc = CreateSord (local_rec.n_quote_no);
				if (cc)
				{
					print_mess (ML (mlQtMess041));
					sleep (sleepTime);
					clear_mess ();
					restart = TRUE;
					return (EXIT_SUCCESS);
				}
				/*
				 * Flag to show sales order has been created. 
				 */
				strcpy (qthr_rec.status, "95");
				cc = abc_update (qthr, &qthr_rec);
				if (cc) 
					file_err (cc, "qthr", "DBUPDATE");
				
				abc_unlock (qthr);
				restart = TRUE;
				return (EXIT_SUCCESS);
		    }
		    entry_exit = TRUE;
		    newQuote = FALSE;
		    if (LoadQtln (qthr_rec.hhqt_hash))
		    {
				restart = TRUE;
				return (EXIT_SUCCESS);
		    }
			open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
			strcpy (exsf_rec.co_no, comm_rec.co_no);
			strcpy (exsf_rec.salesman_no, qthr_rec.sman_code);
			cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
			if (cc)
				sprintf (exsf_rec.salesman,"%-40.40s", " ");

			abc_fclose (exsf);

			if (qthr_rec.hhcu_hash == 0L)
				strcpy (cumr_rec.dbt_no, "99998");
			else
			{
				abc_selfield (cumr, "cumr_hhcu_hash");
				cumr_rec.hhcu_hash	=	qthr_rec.hhcu_hash;
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (cc)
					NoCustomer ();
				else
				{
					strcpy (qthr_rec.dbt_name, cumr_rec.dbt_name);
					DSP_FLD ("cust_name");
					FLD ("cust_name") = NA;
				}
			}

			/*
			 * Lookup contract record. 
			 */
			if (strcmp (qthr_rec.cont_no, "      "))
			{
				strcpy (cnch_rec.co_no, comm_rec.co_no);
				strcpy (cnch_rec.cont_no, qthr_rec.cont_no);
				cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
				if (cc)
					file_err (cc, "cnch", "DBFIND");
			}
			else
			{
				cnch_rec.hhch_hash = 0L;
				strcpy (cnch_rec.exch_type, " ");
			}
		}
		else
		{
			print_mess (ML (mlStdMess210));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer no 
	 */
	if (LCHECK ("cust_no")) 
	{
		abc_selfield (cumr, (!envVar.dbFind) ? "cumr_id_no" : "cumr_id_no3");
		if (dflt_used)
		{
			print_err (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!strncmp (cumr_rec.dbt_no, "99998", 5))
		{
			DSP_FLD ("cust_no");
			FLD ("pri_type") = NO;
			FLD ("cust_name") = NO;
			if (prog_status == ENTRY || ADD_QT)
				skip_entry = goto_field (field, label ("cust_name"));
			else
			{
				qthr_rec.hhcu_hash = 0L;
			 	get_entry (label ("cust_name"));
				DSP_FLD ("cust_name");
				sprintf (qthr_rec.enq_ref,"%-20.20s"," ");
				sprintf (qthr_rec.cont_name, "%-20.20s", " ");
				sprintf (qthr_rec.pos_code,  "%-3.3s", " ");
				sprintf (tmpf_rec.pos_desc,"%-20.20s", " ");
				sprintf (qthr_rec.del_name,   "%-40.40s", " ");
				sprintf (qthr_rec.del_add1, "%-40.40s", " ");
				sprintf (qthr_rec.del_add2, "%-40.40s", " ");
				sprintf (qthr_rec.del_add3, "%-40.40s", " ");
				sprintf (qthr_rec.del_add4, "%-40.40s", " ");
				FLD ("pri_type") = NO;
				DSP_FLD ("ord_no");
				DSP_FLD ("con_name");
				DSP_FLD ("pos_code");
				DSP_FLD ("pos_desc");
				DSP_FLD ("del_name");
				DSP_FLD ("del_addr1");
				DSP_FLD ("del_addr2");
				DSP_FLD ("del_addr3");
				DSP_FLD ("del_addr4");
			}		
			specialNo = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			strcpy (qthr_rec.dbt_name, cumr_rec.dbt_name);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,  comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (cumr_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (pocrRec.co_no, comm_rec.co_no);
		strcpy (pocrRec.code, cumr_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, EQUAL, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		/*
		 * Find currency record for customers currency.
		 */
		if (envVar.dbMcurr)
		{
			strcpy (pocrRec.co_no, comm_rec.co_no);
			sprintf (pocrRec.code, "%-3.3s", cumr_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		
		if (!envVar.dbMcurr || !pocrRec.ex1_factor)
			pocrRec.ex1_factor = 1.00;

		strcpy (qthr_rec.dbt_name, cumr_rec.dbt_name);
		strcpy (qthr_rec.sos, cumr_rec.sur_flag);
		qthr_rec.hhcu_hash = cumr_rec.hhcu_hash; 
		DSP_FLD ("cust_name");

		/*
		 * Check if customer is on stop credit. 
		 */
		if (ADD_QT && cumr_rec.stop_credit [0] == 'Y')
		{
			if (envVar.creditStop)
			{
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str, "***%-26.26s***",ML (mlStdMess060));
				cc = WarnUser (err_str,0);
				if (cc)
					return (cc);
			}
		}

		total_owing = 	cumr_rec.bo_current +
						cumr_rec.bo_per1 +
						cumr_rec.bo_per2 +
						cumr_rec.bo_per3 +
						cumr_rec.bo_per4 +
						cumr_rec.bo_fwd;

		c_left = total_owing - cumr_rec.credit_limit;

		/*
		 * Check if customer is over his credit limit. 
		 */
		if (ADD_QT && cumr_rec.credit_limit <= total_owing && 
			 cumr_rec.credit_limit != 0.00 && cumr_rec.crd_flag [0] != 'Y')
		{
			if (envVar.creditOver)
			{
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str,"***%-33.33s***",ML (mlStdMess061));
				cc = WarnUser (err_str,0);
				if (cc)
					return (EXIT_FAILURE);
			}
		}
		/*
		 * Check Credit Terms	
		 */
		if (ADD_QT && cumr_rec.od_flag && 
		      cumr_rec.crd_flag [0] != 'Y')
		{
			if (envVar.creditTerms)
			{
				sprintf (err_str, ML (mlStdMess062), cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str, ML (mlStdMess062), cumr_rec.od_flag);
				cc = WarnUser (ML (mlStdMess062),0);
				if (cc)
					return (EXIT_FAILURE);
			}
		}
		if (cumr_rec.tax_code [0] == 'A' || cumr_rec.tax_code [0] == 'B')
			notax = 1;
		else
			notax = 0;

		strcpy (qthr_rec.pri_type, cumr_rec.price_type);
		strcpy (qthr_rec.cont_name, cumr_rec.contact_name);

		FLD ("disc") = defaultDiscFlag;
		FLD ("cust_name") = NA;

		DSP_FLD ("cust_name");
		DSP_FLD ("pri_type");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Cust. Enf. Ref.
	 */
	if (LCHECK ("ord_no"))
	{
		if (dflt_used)
		{
			sprintf (qthr_rec.enq_ref,"%-20.20s","                    ");
			DSP_FLD ("ord_no");
			return (EXIT_SUCCESS);
		}

		DSP_FLD ("ord_no");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Position Code. 
	 */
	if (LCHECK ("pos_code"))
	{
		if (dflt_used)
		{
			strcpy (qthr_rec.pos_code, "   ");
			strcpy (tmpf_rec.pos_desc, "                    ");
			DSP_FLD ("pos_code");
			DSP_FLD ("pos_desc");
			return (EXIT_SUCCESS);
		}
		open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");

		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			abc_fclose (tmpf);
			return (EXIT_SUCCESS);
		}

		strcpy (tmpf_rec.co_no, comm_rec.co_no);
		strcpy (tmpf_rec.pos_code, qthr_rec.pos_code);
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess166));
			sleep (sleepTime);
			abc_fclose (tmpf);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pos_code");
		DSP_FLD ("pos_desc");
		abc_fclose (tmpf);
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Price type. 
	 */
	if (LCHECK ("pri_type"))
	{

		if (prog_status == ENTRY && NO_KEY (field))
			strcpy (qthr_rec.pri_type,cumr_rec.price_type);

		if (SRCH_KEY)
		{
			ShowPrice ();
			DSP_FLD ("pri_type");
			SetPriceDesc ();
			DSP_FLD ("pri_desc");
			return (EXIT_SUCCESS);
		}
		if (strncmp (cumr_rec.dbt_no, "99998", 5))
		{
			sprintf (qthr_rec.pri_type,"%-1.1s", cumr_rec.price_type);
			SetPriceDesc ();
		}

		DSP_FLD ("pri_type");
		DSP_FLD ("pri_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Delivery Details. 
	 */
	if (LCHECK ("del_name") || LNCHECK ("del_addr", 8))
	{
		if (NO_KEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used && !specialNo)
		{
			sprintf (qthr_rec.del_name,   "%-40.40s", cumr_rec.dbt_name);
			sprintf (qthr_rec.del_add1, "%-40.40s", cumr_rec.dl_adr1);
			sprintf (qthr_rec.del_add2, "%-40.40s", cumr_rec.dl_adr2);
			sprintf (qthr_rec.del_add3, "%-40.40s", cumr_rec.dl_adr3);
			sprintf (qthr_rec.del_add4, "%-40.40s", cumr_rec.dl_adr4);
			DSP_FLD ("del_name");
			DSP_FLD ("del_addr1");
			DSP_FLD ("del_addr2");
			DSP_FLD ("del_addr3");
			DSP_FLD ("del_addr4");
		}

		if (SRCH_KEY)
		{
			open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");

			i = SrchCudi (field - label ("del_name"));

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (qthr_rec.del_name, cudi_rec.name);
			strcpy (qthr_rec.del_add1, cudi_rec.adr1);
			strcpy (qthr_rec.del_add2, cudi_rec.adr2);
			strcpy (qthr_rec.del_add3, cudi_rec.adr3);
			strcpy (qthr_rec.del_add4, cudi_rec.adr4);
		}
		if (!specialNo)
			skip_entry = goto_field (field, label ("dt_fst_call"));
	}

	/*
	 * Validate Quote Status. 
	 */
	if (LCHECK ("qt_stat"))
	{
		if (dflt_used)
		{
			strcpy (qthr_rec.status, "00");
			for (i = 0;strlen (q_status [i]._stat);i++)
			{
				if (!strncmp (qthr_rec.status, q_status [i]._stat,strlen (q_status [i]._stat)))
				{ sprintf (local_rec.stat_desc,"%-40.40s",q_status [i]._desc);
					break;
				}
			}
			DSP_FLD ("qt_stat");
			DSP_FLD ("qt_stat_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchStatus ();
			return (EXIT_SUCCESS);
		}

		if (ADD_QT && (strcmp (qthr_rec.status, "00")))
		{
			print_err (ML (mlStdMess205));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}  
		if (MOD_QT && (!strcmp (qthr_rec.status, "45")))
		{
			strcpy (qthr_rec.status, "45"); 
			sprintf (local_rec.stat_desc, "%-40.40s", q_status [i]._desc);
		}

		for (i = 0;strlen (q_status [i]._stat);i++)
		{
			if (!strncmp (qthr_rec.status, q_status [i]._stat,strlen (q_status [i]._stat)))
			{
				sprintf (local_rec.stat_desc,"%-40.40s",q_status [i]._desc);
				break;
			}
		}
		DSP_FLD ("qt_stat");
		DSP_FLD ("qt_stat_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate indent Salesman code. 
	 */
	if (LCHECK ("sman"))
	{
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		if (dflt_used)
		{
			strcpy (qthr_rec.sman_code, cumr_rec.sman_code);
			DSP_FLD ("sman");
		}
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			abc_fclose (exsf);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,    qthr_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			abc_fclose (exsf);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("sman_name");
		abc_fclose (exsf);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Expire Date. 
	 */
	if (LCHECK ("exp_date"))
	{
		if (dflt_used)
		{
			qthr_rec.expire_date = TodaysDate () + 30L;
			DSP_FLD ("exp_date");
			return (EXIT_SUCCESS);
		}

		if (qthr_rec.dt_quote > qthr_rec.expire_date)
		{
			print_mess (ML (mlQtMess052));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
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
			return (EXIT_SUCCESS);
		}
		
		if (strncmp (local_rec.item_no, "9999", 4))
		{
			FLD ("descr") = NA;
			DSP_FLD ("uom");
			if (!F_HIDE (label ("qty_avail")))
				FLD ("qty_avail") = NA;

			cc = ValidItemNo (TRUE);
			if (!cc)
				tab_other (line_cnt);

			DSP_FLD ("item_no");
			DSP_FLD ("descr");
			return (cc);
		}
		else
		{
			DSP_FLD ("item_no");
			FLD ("descr") = NO;
			DSP_FLD ("uom");
			if (!F_HIDE (label ("qty_avail")))
				FLD ("qty_avail") = NO;
			SpecItem (); 
			tab_other (line_cnt);  
			return (EXIT_SUCCESS);
		}
	}

	/*
	 * Validate Item Description. 
	 */
	if (LCHECK ("descr"))
	{
		if (dflt_used)
		{
			strcpy (qtln_rec.item_desc, inmr_rec.description);
			DSP_FLD ("descr");
		}

	    if (NON_STOCK || !strcmp (inmr_rec.item_no,"NS              "))
		    	skip_entry = goto_field (field, label ("sub_tot")); 

		DSP_FLD ("descr");
	   	return (EXIT_SUCCESS);
	}

	/*
	 * Validate Unit Of Measure   
	 */
    if (LCHECK ("uom"))
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
		SR.hhumHash 	= inum2_rec.hhum_hash;
		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;
		SR.cnvFct 	= inum2_rec.cnv_fct/inum_rec.cnv_fct;
		DSP_FLD ("uom");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qty_avail"))
	{
		if (F_NOKEY (label ("qty_avail")))
			return (EXIT_SUCCESS);
		
		if (dflt_used)
		{
			local_rec.qty_avail = 0.00;
			DSP_FLD ("qty_avail");
			return (EXIT_SUCCESS);
		}
		SR.qtyAvailable = local_rec.qty_avail;
		DSP_FLD ("qty_avail");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Paragraph input    
	 */
	if (LCHECK ("par_code"))
	{
		if (SRCH_KEY)
		{
			SrchQtln (temp_str);
			return (EXIT_SUCCESS);
		}
	
		if (last_char == DELLINE)
			return (DeleteParaLine ());

		if (last_char == INSLINE)
			return (InsertParaLine ());

		/*
		 * First character is a '\'	
		 * \D	- delete current line	
		 * \I	- insert before current	
		 */
		if (local_rec.par_code [0] == 92)
		{
		    	if (local_rec.par_code [1] == 'D')
				return (DeleteParaLine ());

		    	if (local_rec.par_code [1] == 'I')
				return (InsertParaLine ());
		}

		strcpy (qtlh_rec.co_no, comm_rec.co_no);
		sprintf (qtlh_rec.par_code, "%-10.10s", local_rec.par_code);
		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess047));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		_hhlh_hash [line_cnt] = qtlh_rec.hhlh_hash;
		strcpy (_par_desc [line_cnt], qtlh_rec.par_desc);

		if (newQuote && prog_status == ENTRY)
			tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate quantity input For item. 
	 */
	if (LCHECK ("qty"))
	{
		if (NON_STOCK && local_rec.qty != 0.00)
		{
			local_rec.qty 	= 0.00;
			print_mess (ML (mlQtMess031));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

	    if (!MULT_QTY && (local_rec.qty != 0.00 && local_rec.qty != 1.00))
	    {
			print_mess (ML (mlStdMess029));
			return (EXIT_FAILURE);
	    }
	    SR.quantity = local_rec.qty;

		if (!MULT_QTY && local_rec.qty == 0.00)
		{
			if (strcmp (SR.serialNo, ser_space))
			{
				cc = UpdateInsf (SR.hhwhHash, 0L, SR.serialNo,"C","F");
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}

			strcpy (local_rec.serial_no, ser_space);
			strcpy (SR.serialNo, ser_space);
			DSP_FLD ("ser_no");
		}

		PriceProcess ();
		DiscProcess ();

		if (prog_status == ENTRY)
		{
		    local_rec.cost_price = FindCostValue (line_cnt) * SR.cnvFct;
	        SR.costPrice       = local_rec.cost_price;
			SR.min_sell_pric     = inmr_rec.min_sell_pric;

			skip_entry = (qtln_rec.sale_price == 0.00) ? 1 : 2;

			if (local_rec.cost_price == 0.00)
				skip_entry = 0;

			DSP_FLD ("cost_price");
		}

	    CalcTotal (TRUE);
	    return (EXIT_SUCCESS);
	}

	/*
	 * Validate FOB (FGN). 
	 */
	if (LCHECK ("cost_price"))
	{
		if (FLD ("cost_price") == NA)
			return (EXIT_SUCCESS);
			
		if (dflt_used)
		   	local_rec.cost_price = FindCostValue (line_cnt) * SR.cnvFct;

		SR.costPrice   = local_rec.cost_price;
		SR.min_sell_pric = inmr_rec.min_sell_pric;

		if (local_rec.cost_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess121),"YyNn",1,2);
			Busy (0);
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}
		CalcTotal (TRUE);
		skip_entry = (qtln_rec.sale_price == 0.00) ? 0 : 1;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Sale Price Input. 
	 */
	if (LCHECK ("sale_price"))
	{
		if (FLD ("sale_price") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (SR.priceOveride, "N");
			PriceProcess ();
			DiscProcess ();
			
			DSP_FLD ("sale_price");
		}

		if (qtln_rec.sale_price == 0.00)
		{
			if (inmr_rec.min_sell_pric == 0.00)
			{
				i = prmptmsg (ML (mlStdMess031),"YyNn",1,2);
				Busy (0);
				if (i != 'Y' && i != 'y')
					return (EXIT_FAILURE);
			}
			else  
			{
				print_mess (ML (mlStdMess230));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}

		if (SR.calcSalePrice != qtln_rec.sale_price)
			strcpy (SR.priceOveride, "Y");

		/*
		 * Calculate new GROSS sale price. 
		 */
		SR.grossSalePrice = no_dec (qtln_rec.sale_price / (1.00 - (SR.regPc / 100.00)));
		SR.salePrice  = GetCusGprice (SR.grossSalePrice, SR.regPc);
		qtln_rec.sale_price = SR.salePrice;

		SR.actSalePrice 	= qtln_rec.sale_price;
		SR.disPc 		= qtln_rec.disc_pc;
		SR.sale    		= qtln_rec.sale_price;

		if (prog_status != ENTRY)
			CalcTotal (TRUE);

		/*
		 * Checks if Sale price entered is w/in Min. Sale price range 
		 */
		if (min_tot <= sal_tot) 
			strcpy (qthr_rec.status, "00");
		else
			strcpy (qthr_rec.status, "45");


		return (EXIT_SUCCESS);
	}
				
	/*
	 * Validate Discount Percent. 
	 */
	if (LCHECK ("disc"))
	{
		if (FLD ("disc") == NA)
			return (EXIT_SUCCESS);

		if (FLD ("disc") == NI && prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (SR.contractStatus || SR.indent)
			return (EXIT_FAILURE);

		if (dflt_used)
		{
			strcpy (SR.discOveride, "N");
		}

		if (SR.contractPrice || SR.contractStatus == 2 || SR.indent)
		{
			qtln_rec.disc_pc = 0.00;
			SR.discA      = 0.00;
			SR.discB      = 0.00;
			SR.discC      = 0.00;
			DSP_FLD ("disc");
		}

		SR.disPc = ScreenDisc (qtln_rec.disc_pc);
		SR.disPc = qtln_rec.disc_pc;

		if (SR.calcDisc != ScreenDisc (qtln_rec.disc_pc))
			strcpy (SR.discOveride, "Y");

		/*
		 * Discount has been entered so set disc B & C to zero.   
		 */
		if (!dflt_used)
		{
			SR.discA = SR.disPc;
			SR.discB = 0.00;
			SR.discC = 0.00;
		}

		CalcTotal (TRUE);

		if (min_tot <= sal_tot)
			strcpy (qthr_rec.status, "00");
		else
			strcpy (qthr_rec.status, "45");


		return (EXIT_SUCCESS);
	}

	/*
	 * Validate serial number input. 
	 */
	if (LCHECK ("ser_no"))
	{
		if (SR.serialFlag [0] != 'Y')
		{
			strcpy (local_rec.serial_no,ser_space);
			strcpy (SR.serialNo,ser_space);
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}

		if (end_input)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchInsf (temp_str,line_cnt);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.serial_no, ser_space))
		{
			/*
			 * Free previous serial number if any 
			 */
			if (strcmp (SR.serialNo, ser_space))
			{
				cc = UpdateInsf (SR.hhwhHash,  0L, SR.serialNo, "C", "F");
	
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}

			strcpy (SR.serialNo, local_rec.serial_no);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.serial_no, SR.serialNo))
			return (EXIT_SUCCESS);

		insfRec.hhwh_hash = SR.hhwhHash;
		insfRec.hhbr_hash = SR.hhsiHash;
		strcpy (insfRec.status,"F");
		sprintf (insfRec.serial_no, "%-25.25s", local_rec.serial_no);
		cc = find_rec (insf,&insfRec,COMPARISON,"r");
		if (cc)
		{
			abc_selfield (insf,"insf_hhbr_id");

			insfRec.hhwh_hash = SR.hhwhHash;
			insfRec.hhbr_hash = SR.hhsiHash;
			strcpy (insfRec.status,"F");
			sprintf (insfRec.serial_no, "%-25.25s", local_rec.serial_no);
			cc = find_rec (insf,&insfRec,COMPARISON,"r");
		}
		if (cc)
		{
			insfRec.hhwh_hash = SR.hhwhHash;
			insfRec.hhbr_hash = SR.hhsiHash;
			strcpy (insfRec.status,"C");
			sprintf (insfRec.serial_no, "%-25.25s", local_rec.serial_no);
			cc = find_rec (insf,&insfRec,COMPARISON,"r");
			if (!cc)
			{
				print_mess (ML (mlStdMess097));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			insfRec.hhwh_hash = SR.hhwhHash;
			insfRec.hhbr_hash = SR.hhsiHash;
			strcpy (insfRec.status,"S");
			sprintf (insfRec.serial_no, "%-25.25s", local_rec.serial_no);
			cc = find_rec (insf,&insfRec,COMPARISON,"r");
			if (!cc)
			{
				print_mess (ML (mlStdMess243));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckDuplicateInsf (local_rec.serial_no, SR.hhsiHash, line_cnt))
		{
			print_mess (ML (mlStdMess223));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Free previous serial number if any 
		 */
		if (strcmp (SR.serialNo, ser_space))
		{
			cc = UpdateInsf (SR.hhwhHash, 0L,  SR.serialNo, "C", "F");

			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
		}

		strcpy (SR.serialNo, local_rec.serial_no);

		cc = UpdateInsf (SR.hhwhHash, 0L,  SR.serialNo, "F", "C");

		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
		
		if (!SR.contractCost)
		{
			wk_value = SerialValue (insfRec.est_cost,insfRec.act_cost);
	
			SR.costPrice = CENTS (wk_value) * SR.cnvFct;
			local_rec.cost_price = CENTS (wk_value) * SR.cnvFct;
			DSP_FLD ("cost_price");
		}
		DSP_FLD ("ser_no");

		CalcTotal (TRUE);
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Subtotal Flag. 
	 */
	if (LCHECK ("sub_tot"))
	{
		if (FLD ("sub_tot") == NA)
		{
			strcpy (qtln_rec.st_flag, " ");
			DSP_FLD ("sub_tot");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (qtln_rec.st_flag," ");
			DSP_FLD ("sub_tot");
			return (EXIT_SUCCESS);
		}
		else
		{
			strcpy (qtln_rec.st_flag,"*");
			DSP_FLD ("sub_tot");
			return (EXIT_SUCCESS);
		}
	}
	/*
	 * Validate Selling Terms. 
	 */
	if (LCHECK ("sell_terms"))
	{
		if (SRCH_KEY)
		{
			ShowSell ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (s_terms [i]._scode);i++)
		{
			if (!strncmp (qthr_rec.sell_terms,s_terms [i]._scode,strlen (s_terms [i]._scode)))
			{
				sprintf (local_rec.sell_desc,"%-30.30s",s_terms [i]._sterm);
				break;
			}
		}

		if (!strlen (s_terms [i]._scode))
		{
			print_mess (ML (mlStdMess208));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strncmp (qthr_rec.sell_terms," ",3) && strncmp (local_rec.sell_desc," ", 30))
			
		DSP_FLD ("sell_terms");
		DSP_FLD ("sell_desc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Payment Terms. 
	 */
	if (LCHECK ("pay_term"))
	{
		val_pterms = FALSE;
		if (SRCH_KEY)
		{
			ShowPay ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (qthr_rec.pay_term,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (qthr_rec.pay_term,"%-40.40s",p_terms [i]._pterm);
				val_pterms = TRUE; 
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_term");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Carrier Code. 
	 */
	if (LCHECK ("carr_code"))
	{
		if (FLD ("carr_code") == ND || FLD ("carr_code") == NA)
			return (EXIT_SUCCESS);

		cfhr_rec.markup_pc = 0.00;
		cfln_rec.cost_kg = 0.00;

		if (dflt_used)
		{
		 	CalculateFreight (cfhr_rec.markup_pc, 
			      cfln_rec.cost_kg); 

			if (!strcmp (qthr_rec.carr_code, "    "))  
			{
				strcpy (qthr_rec.carr_code, "    ");
				sprintf (local_rec.carr_desc, "%-40.40s", " ");
			}
			strcpy (qthr_rec.carr_code, local_rec.carr_code);	
			strcpy (local_rec.carr_desc, cfhr_rec.carr_desc);
			DSP_FLD ("carr_code");
			DSP_FLD ("carr_desc");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		open_rec (cfhr,cfhr_list,CFHR_NO_FIELDS,"cfhr_id_no");
		open_rec (cfln,cfln_list,CFLN_NO_FIELDS,"cfln_cfhh_hash");

		if (SRCH_KEY)
		{
			ShowCarr (temp_str);
			abc_fclose (cfhr);
			abc_fclose (cfln); 
			return (EXIT_SUCCESS);
		}
			
		strcpy (cfhr_rec.co_no, comm_rec.co_no);
		strcpy (cfhr_rec.br_no, comm_rec.est_no);
		strcpy (cfhr_rec.carr_code, qthr_rec.carr_code);
		cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
		if (!cc)
		{
			cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
			cc = find_rec (cfln,&cfln_rec,EQUAL,"r"); 
			if (cc)
			{
				print_mess (ML (mlStdMess134));
				sleep (sleepTime);
				clear_mess ();

				abc_fclose (cfhr);
				abc_fclose (cfln);
				return (EXIT_FAILURE);
			}
			abc_fclose (cfhr);
			abc_fclose (cfln);

			strcpy (exaf_rec.co_no,comm_rec.co_no);
			strcpy (exaf_rec.area_code,cfln_rec.area_code);
			cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess108));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (qthr_rec.carr_area, cfln_rec.area_code);
			strcpy (local_rec.carr_adesc, exaf_rec.area);
			strcpy (local_rec.carr_desc, cfhr_rec.carr_desc);
			strcpy (local_rec.carr_area, qthr_rec.carr_area);

			DSP_FLD ("carr_code");
			DSP_FLD ("carr_desc");
			DSP_FLD ("carr_area");
			DSP_FLD ("ca_desc");

			CalculateFreight (cfhr_rec.markup_pc, 
			      cfln_rec.cost_kg); 

			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		else 
		{
			print_mess (ML (mlStdMess134));
			sleep (sleepTime);
			clear_mess ();

			abc_fclose (cfhr);
			abc_fclose (cfln);
			return (EXIT_FAILURE);
		}
	}
	/*
	 * Validate Carrier Area Code. 
	 */
	if (LCHECK ("carr_area"))
	{
		if (FLD ("carr_code") == ND || FLD ("carr_code") == NA)
			return (EXIT_SUCCESS);

		cfhr_rec.markup_pc = 0.00;
		cfln_rec.cost_kg = 0.00;

		if (dflt_used)
		{
			CalculateFreight (cfhr_rec.markup_pc, 
				      cfln_rec.cost_kg);
			if (!strcmp (qthr_rec.carr_area, "  "))
			{
				strcpy (qthr_rec.carr_area, cumr_rec.area_code);
				strcpy (exaf_rec.co_no,comm_rec.co_no);
			}
			strcpy (qthr_rec.carr_area, local_rec.carr_area);	
			strcpy (exaf_rec.area_code,qthr_rec.carr_area);
			cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
			if (!cc)
				strcpy (local_rec.carr_adesc, exaf_rec.area);

			DSP_FLD ("carr_desc");
			DSP_FLD ("carr_area");
			DSP_FLD ("ca_desc");
  	
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			return (EXIT_SUCCESS);
		}
		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (local_rec.carr_area,cumr_rec.area_code);
			DSP_FLD ("carr_area");
		}

		if (SRCH_KEY)
		{
			ShowArea (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,qthr_rec.carr_area);
		cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.carr_adesc, exaf_rec.area);
		strcpy (local_rec.carr_area,exaf_rec.area_code);

		DSP_FLD ("ca_desc");

		open_rec (cfhr,cfhr_list,CFHR_NO_FIELDS,"cfhr_id_no");
		open_rec (cfln,cfln_list,CFLN_NO_FIELDS,"cfln_cfhh_hash");
			
		strcpy (cfhr_rec.co_no, comm_rec.co_no);
		strcpy (cfhr_rec.br_no, comm_rec.est_no);
		strcpy (cfhr_rec.carr_code, qthr_rec.carr_code);
		cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
		if (!cc)
		{
			cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
			cc = find_rec (cfln,&cfln_rec,EQUAL,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess134));
				sleep (sleepTime);
				clear_mess ();

				abc_fclose (cfhr);
				abc_fclose (cfln);
				return (EXIT_FAILURE);
			}
			abc_fclose (cfhr);
			abc_fclose (cfln);

			CalculateFreight (cfhr_rec.markup_pc,cfln_rec.cost_kg);

			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			return (EXIT_SUCCESS);
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Validate Reset Flag. 
	 */
	if (LCHECK ("reset_price"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.reset_flag, "N");
			DSP_FLD ("reset_price");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.reset_flag, "Y"))
		{
			qthr_rec.expire_date = 0L;
			DSP_FLD ("reset_price");
			return (EXIT_SUCCESS);
		}
		DSP_FLD ("reset_price");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Surcharge. 
	 */
	if (LCHECK ("sos_ok"))
	{
		if (dflt_used)
		{
			if (newQuote)
				strcpy (qthr_rec.sos, cumr_rec.sur_flag);
			else
				strcpy (qthr_rec.sos, local_rec.sos); 
				
			DSP_FLD ("sos_ok");
			return (EXIT_SUCCESS);
		}
		DSP_FLD ("sos_ok");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Freight. 
	 */
	if (LCHECK ("freight"))
	{
		if (qthr_rec.freight == 0.00)
		{
			DSP_FLD ("freight");
			return (EXIT_SUCCESS);
		}
		DSP_FLD ("freight");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate No. of kgs.
	 */
	if (LCHECK ("no_kgs"))
	{
		if (qthr_rec.no_kgs == 0.00)
		{
			DSP_FLD ("no_kgs");
			return (EXIT_SUCCESS);
		}
		DSP_FLD ("no_kgs");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Routine to check if an alternate flag has been set 				
 * (this is indicated by a '/A' on the end of the part number. 	
 * If Alternate flag is set then '/A' is removed from part number.
 * Returns 0 if alternate flag has not been set, 1 if it has.     
 */
int
CheckAlternate (
	char	*itemNo)
{
	char	AlternateItem [17];
	char *	sptr;

	sprintf (AlternateItem,"%-16.16s",itemNo);
	sptr = clip (AlternateItem);

	if (strlen (sptr) > 2)
	{
		sptr += (strlen (sptr) - 2);
		if (*sptr == '/'  && * (sptr + 1) == 'A')
		{
			*sptr = '\0';
			sprintf (itemNo,"%-16.16s",AlternateItem);
			return (EXIT_FAILURE);
		}
	}
	sprintf (itemNo,"%-16.16s",AlternateItem);
	return (EXIT_SUCCESS);
}

int
win_function2 (
 int	fld,
 int	lin,
 int	scn,
 int	mode)
{
	int		i;

	if (scn != 2)
		return (EXIT_SUCCESS);

	if (store [lin].hhbrHash == 0L)
		return (EXIT_SUCCESS);
	
	if (prog_status == ENTRY)
		return (EXIT_SUCCESS);

	/*
	 * Check for contract. 
	 */
	if (store [lin].contractStatus)
	{
		print_mess (ML (mlStdMess231));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Disable edit of qty BO field. 
	 */
	negoScn [1].fldEdit = 0;

	/*
	 * Initialise values for negotiation window. 
	 */
	negoRec.qOrd		=	store [lin].quantity;
	negoRec.qBord		=	0.00;

	negoRec.regPc			=  	store [lin].regPc;
	negoRec.discArray [0]	=	store [lin].discA;
	negoRec.discArray [1]	=	store [lin].discB;
	negoRec.discArray [2]	=	store [lin].discC;
	negoRec.grossPrice		=	store [lin].grossSalePrice;
	negoRec.salePrice		=	store [lin].salePrice;
	negoRec.margCost		= 	DOLLARS (store [lin].costPrice);
	negoRec.outer_size		= 	store [lin].outerSize;

	NegPrice (1, 7, local_rec.item_no, qtln_rec.item_desc, 
				   store [lin].cumulative, scn);

	/*
	 * Clear space where negotiation window was. 
	 */
	for (i = 7; i < 12; i++)
	{
		move (1, i);
		cl_line ();
	}

	if (!restart)
	{
		local_rec.qty 				=   negoRec.qOrd;

		store [lin].quantity			=	negoRec.qOrd;
		store [lin].regPc		= 	negoRec.regPc;
		store [lin].discA		= 	negoRec.discArray [0];
		store [lin].discB		= 	negoRec.discArray [1];
		store [lin].discC		= 	negoRec.discArray [2];
		store [lin].disPc		=	CalcOneDisc (store [lin].cumulative,
													 negoRec.discArray [0],
													 negoRec.discArray [1],
													 negoRec.discArray [2]);
		store [lin].grossSalePrice 	= 	negoRec.grossPrice;
		store [lin].salePrice	=	negoRec.salePrice;
		store [lin].actSalePrice		=	negoRec.salePrice;

		qtln_rec.disc_pc  			= 	ScreenDisc (store [lin].disPc);
		qtln_rec.sale_price 		= 	store [lin].salePrice;

		if (store [lin].calcSalePrice != qtln_rec.sale_price)
			strcpy (SR.priceOveride, "Y");

		if (store [lin].calcDisc != ScreenDisc (qtln_rec.disc_pc))
			strcpy (SR.discOveride, "Y");

		putval (lin);
	}

	CalcTotal (TRUE);
	
	restart = FALSE;
	return (EXIT_SUCCESS);
}

/*
 * Main validation Routine for item number. 
 */
int
ValidItemNo (
 int	getFields)
{
	float	realCommitted;
	int		itemChanged = FALSE;
	int		save_sts = prog_status;
	char	WorkMask [100];
	char *	sptr;

	abc_selfield (inmr, "inmr_id_no");

	skip_entry = 0;
	
	if (last_char == DELLINE)
		return (DeleteItemLine ());

	if (last_char == INSLINE)
		return (InsertLine ()); 

	if (dflt_used || !strcmp (local_rec.item_no, "                "))
		return (DeleteItemLine ());

	if (prog_status == ENTRY)
		sprintf (local_rec.serial_no, "%25.25s", " ");
	
	SR.alternate [0] = (CheckAlternate (local_rec.item_no)) ? 'Y' : 'N';

	cc	=	FindInmr 
			 (
				comm_rec.co_no, 
				local_rec.item_no,
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		errmess (ML (mlStdMess001));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	else
	{
		sprintf (WorkMask, "%s %s", ML (mlStdMess012), cumr_rec.curr_code);
		sprintf (err_str, WorkMask, cumr_rec.dbt_no, clip (cumr_rec.dbt_name));
		print_at (4,0,err_str);
	}

	sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
	local_rec.weight = inmr_rec.weight;

	strcpy (inum_rec.uom, inmr_rec.sale_unit);
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");

	if (FLD ("uom") != NE)
	{
		strcpy (SR.uom, inum_rec.uom);
		strcpy (local_rec.UOM, inum_rec.uom);
	    DSP_FLD ("uom");
	}

	if (prog_status   != ENTRY &&
		SR.hhbrHash != inmr_rec.hhbr_hash &&
		SR.hhbrHash != 0)
	{
		if (inmr_rec.inmr_class [0] == 'K')
		{
			print_mess (ML (mlStdMess174));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		itemChanged = TRUE;
		if (strcmp (SR.serialNo, ser_space) && SR.serialFlag [0] == 'Y')
		{
			cc = UpdateInsf (SR.hhwhHash,  0L, SR.serialNo, "C", "F");
			if (cc && cc < 1000)
				file_err (cc, insf, "DBUPDATE");
			/*
			 * Because we have freed the insf record we must 
			 * blank the serial number field on the qtln in 
			 * case of a restart.                          
			 */
			qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
			qtln_rec.line_no   = SR.origLine;
			cc = find_rec (qtln, &qtln_rec, EQUAL, "u");
			if (!cc)
			{
				strcpy (qtln_rec.serial_no, ser_space);
				abc_update (qtln, &qtln_rec);
			}
		}
		strcpy (local_rec.serial_no, ser_space);
		strcpy (SR.serialNo, local_rec.serial_no);
		strcpy (SR.origSerialNo, local_rec.serial_no);
		DSP_FLD ("ser_no");
	}

	SR.hhbrHash 	= inmr_rec.hhbr_hash;
	SR.hhsiHash 	= alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	SR.outerSize     	= inmr_rec.outer_size;
	SR.weight    	= inmr_rec.weight;
	SR.defaultDisc 	= inmr_rec.disc_pc;
	SR.decPt 		= inmr_rec.dec_pt;
	SR.itemClass [0]  	= inmr_rec.inmr_class [0];
	strcpy (SR.category,     inmr_rec.category);
	strcpy (SR.sellGroup,      inmr_rec.sellgrp);
	strcpy (SR.serialFlag,  inmr_rec.serial_item);
	strcpy (SR.serialFlag,  inmr_rec.serial_item);
	strcpy (SR.costingFlag, inmr_rec.costing_flag);

	/*
	 * Check for Indent items. 
	 */
	if (strncmp (inmr_rec.item_no, "9999", 4) || envVar.discountIndents)
		SR.indent = FALSE;
	else
		SR.indent = TRUE;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc) 
	{
		print_mess (ML (mlStdMess192));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	SR.hhccHash = incc_rec.hhcc_hash;

	SuperSynonymError ();

	DSP_FLD ("item_no");
	strcpy (qtln_rec.item_desc, inmr_rec.description);
	qtln_rec.hhbr_hash = inmr_rec.hhbr_hash;

	SR.taxAmount = inmr_rec.tax_amount;
	SR.taxPc  = inmr_rec.tax_pc;
	SR.gstPc  = inmr_rec.gst_pc;

	DSP_FLD ("item_no");
	DSP_FLD ("descr");

 	SR.costPrice = 0.00;  
	DSP_FLD ("cost_price");
	DSP_FLD ("sale_price");
	DSP_FLD ("disc");

	SR.hhwhHash = incc_rec.hhwh_hash;
	/*
	 * Item is a serial item. 
	 */
	if (inmr_rec.serial_item [0] == 'Y')
	{
		FLD ("qty") = NI;
		if (FLD ("ser_no") == NA)
			FLD ("ser_no") = YES;

		local_rec.qty = 1.00;
		DSP_FLD ("descr");
		DSP_FLD ("qty");
	}
	else
	{
		FLD ("qty") = YES;
		if (NEED (label ("ser_no")))
			FLD ("ser_no") = NA;
	}

	if (itemChanged)
	{
		local_rec.qty = 0.00;
		SR.quantity = local_rec.qty;
		DSP_FLD ("qty");
		PriceProcess ();
		DiscProcess ();
		CalcTotal (FALSE);
	}

	/*
	 * Calculate Actual Qty Committed. 
	 */
	realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,
										incc_rec.hhcc_hash);

	if (envVar.includeForwardStock)
	{
		SR.qtyAvailable = incc_rec.closing_stock -
						incc_rec.committed -
						realCommitted - 
						incc_rec.backorder - 
						incc_rec.forward;
	}
	else
	{
		if (!SR.indent)
		{
			SR.qtyAvailable = incc_rec.closing_stock -
							incc_rec.committed -
							realCommitted - 
							incc_rec.backorder;
		}
		else
			SR.qtyAvailable = 0.00;
	}
	
	local_rec.qty_avail = SR.qtyAvailable;
	DSP_FLD ("qty_avail");

	if (NON_STOCK)
	{
		FLD ("descr") = NO;
		skip_entry = goto_field (label ("item_no"),label ("descr")); 
		local_rec.qty = 0.00;
		DSP_FLD ("qty");
	}
	else
		skip_entry = 2;

	sptr = clip (inmr_rec.description);
	if (strlen (sptr) == 0)
		skip_entry = 1;

	if (itemChanged && getFields)
	{
		prog_status = ENTRY;
		if (SR.serialFlag [0] == 'Y')
		{
			local_rec.qty = 1.00;
			DSP_FLD ("qty");
			cc = spec_valid (label ("qty"));
			while (cc && !restart)
			{
				get_entry (label ("qty"));
				cc = spec_valid (label ("qty"));
			}
			SR.quantity = local_rec.qty;
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
				get_entry (label ("qty"));
				cc = spec_valid (label ("qty"));
			} while (cc && !restart);
			DSP_FLD ("qty");
		}
		prog_status = save_sts;
	}

	sptr = clip (inmr_rec.description);
	if (strlen (sptr) == 0)
		skip_entry = 0;

	tab_other (line_cnt);

	return (EXIT_SUCCESS);
}

void
PriceProcess (void)
{
	int		pType;
	float	regPc;
	double	grossPrice;

	SR.pricingCheck	= FALSE;

	pType 	   = atoi (cumr_rec.price_type);

	grossPrice = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  comm_rec.cc_no,
							  cumr_rec.area_code,
							  cumr_rec.class_type,
							  SR.sellGroup,
							  cumr_rec.curr_code,
							  pType,
							  cumr_rec.disc_code,
							  cnch_rec.exch_type,
							  cumr_rec.hhcu_hash,
							  SR.hhccHash,
							  SR.hhbrHash,
							  SR.category,
							  cnch_rec.hhch_hash,
							 (envVar.useSystemDate) ? TodaysDate () : comm_rec.dbt_date,
							  ToStdUom (local_rec.qty),
							  pocrRec.ex1_factor,
							  FGN_CURR,
							  &regPc);

	SR.pricingCheck	= TRUE;

	SR.calcSalePrice = GetCusGprice (grossPrice, regPc) * SR.cnvFct;

	if (SR.priceOveride [0] == 'N')
	{
		SR.grossSalePrice 	= 	grossPrice * SR.cnvFct;
		SR.salePrice 		=	SR.calcSalePrice;
		SR.regPc 			= 	regPc;
		qtln_rec.sale_price = 	SR.calcSalePrice;
		SR.actSalePrice 		= 	SR.calcSalePrice;
	}
	SR.contractPrice 		= (_CON_PRICE) ? TRUE : FALSE;
	SR.contractStatus  	= _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess (void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*
	 * Discount does not apply. 
	 */
	if (SR.contractStatus == 2 || SR.contractPrice || SR.indent)
	{
		qtln_rec.disc_pc  	= 0.00;
		SR.disPc 	 		= 0.00;
		SR.calcDisc 		= 0.00;
		SR.discA			= 0.00;
		SR.discB			= 0.00;
		SR.discC			= 0.00;
		DSP_FLD ("disc");
		return;
	}

	if (SR.pricingCheck == FALSE)
		PriceProcess ();
		

	pType 	= atoi (cumr_rec.price_type);
	cumDisc	=	GetCusDisc (	comm_rec.co_no,
							comm_rec.est_no,
							SR.hhccHash,
							cumr_rec.hhcu_hash,
							cumr_rec.class_type,
							cumr_rec.disc_code,
							SR.hhsiHash,
							SR.category,
							SR.sellGroup,
							pType,
							SR.grossSalePrice,
							SR.regPc,
							ToStdUom (local_rec.qty),
							discArray);

	if (SR.discOveride [0] == 'Y')
	{
		
		DSP_FLD ("disc");
		return;
	}
							
	SR.calcDisc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	qtln_rec.disc_pc 	=	ScreenDisc (SR.calcDisc);
	SR.disPc			=	SR.calcDisc;

	SR.discA 			= 	discArray [0];
	SR.discB 			= 	discArray [1];
	SR.discC 			= 	discArray [2];
	SR.cumulative 		= 	cumDisc;

	if (SR.defaultDisc > ScreenDisc (qtln_rec.disc_pc) &&
		SR.defaultDisc != 0.0)
	{
		qtln_rec.disc_pc = 	ScreenDisc (SR.defaultDisc);
		SR.calcDisc	=	SR.defaultDisc;
		SR.disPc		=	SR.defaultDisc;
		SR.discA 		= 	SR.defaultDisc;
		SR.discB 		= 	0.00;
		SR.discC 		= 	0.00;
 	}

	DSP_FLD ("disc");
}

/*
 * Init fields when no debtor used. 
 */
void
NoCustomer (void)
{
	strcpy (cumr_rec.dbt_name,        "                                        ");
	strcpy (cumr_rec.dbt_acronym,     "UNKNOWN  ");
	strcpy (cumr_rec.curr_code,   "   ");
	strcpy (cumr_rec.ch_adr1,   "                                        ");
	strcpy (cumr_rec.ch_adr2,   "                                        ");
	strcpy (cumr_rec.ch_adr3,   "                                        ");
	strcpy (cumr_rec.dl_adr1,   "                                        ");
	strcpy (cumr_rec.dl_adr2,   "                                        ");
	strcpy (cumr_rec.dl_adr3,   "                                        ");
	strcpy (cumr_rec.area_code,   "  ");
	strcpy (cumr_rec.tax_no,      "               ");
	cumr_rec.inst_fg1 = 0;
	cumr_rec.inst_fg2 = 0;
	cumr_rec.inst_fg3 = 0;
	strcpy (cumr_rec.stat_flag,   " ");
	strcpy (cumr_rec.price_type,  "1");
	strcpy (cumr_rec.tax_code,  (envVar.gstApplies) ? "C" : "A");
	strcpy (cumr_rec.class_type,  "   ");
	strcpy (cumr_rec.disc_code,   " ");
	strcpy (cumr_rec.sman_code,   "  ");
	strcpy (cumr_rec.contact_name,"                    ");
	cumr_rec.hhcu_hash = 0L;
	strcpy (cumr_rec.dbt_no,      " NONE ");
	strcpy (cumr_rec.item_codes,  "N");
	FLD ("cust_name") = YES;

	if (cumr_rec.tax_code [0] == 'A' || cumr_rec.tax_code [0] == 'B')
		notax = TRUE;
	else
		notax = FALSE;

	FLD ("disc") = NA;
}

/*
 * Delete line. 
 */
int
DeleteParaLine (void)
{
	int	i;
	int	this_page;

	this_page = line_cnt / TABLINES;
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	lcount [3]--;
	for (i = line_cnt;line_cnt < lcount [3];line_cnt++)
	{
		getval (line_cnt + 1);
	    	putval (line_cnt);

		_hhlh_hash [line_cnt] = _hhlh_hash [line_cnt + 1];
		strcpy (_par_desc [line_cnt], _par_desc [line_cnt + 1]);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	_hhlh_hash [line_cnt]  = 0L;
	sprintf (_par_desc [line_cnt], "%-40.40s"," ");
	sprintf (local_rec.par_code, "%-10.10s"," ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Delete line. 
 */
int
DeleteItemLine (void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/* Free insf record if a serial item */
	cc = UpdateInsf (SR.hhwhHash,  0L, SR.serialNo, "C", "F");

	if (cc && cc < 1000)
		file_err (cc, insf, "DBUPDATE");

	/* 
	 * Because we have freed the insf record we must 
	 * blank the serial number field on the qtln in 
	 * case of a restart.                          
	 */

	qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
	qtln_rec.line_no   = SR.origLine;
	cc = find_rec (qtln, &qtln_rec, EQUAL, "u");
	if (!cc)
	{
		strcpy (qtln_rec.serial_no, ser_space);
		abc_update (qtln, &qtln_rec);
	}

	lcount [2]--;

	this_page = line_cnt / TABLINES;
	
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		memcpy 
		 (
			 (char *) &SR, 
			 (char *) &store [line_cnt + 1],
			sizeof (struct storeRec)
		);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	sprintf (local_rec.item_no, "%-16.16s", " ");
	sprintf (qtln_rec.item_desc, "%-40.40s", " ");
	local_rec.qty          	= 0.00;
	local_rec.cost_price   	= 0.00;
	qtln_rec.sale_price 	= 0.00;
	qtln_rec.disc_pc    		= 0.00;
	sprintf (local_rec.serial_no, "%-25.25s", " ");
	putval (line_cnt);

	pin_bfill ((char *) &SR, '\0', sizeof (struct storeRec));
	SR.origLine = -1;
	strcpy (SR.priceOveride, "N");
	strcpy (SR.discOveride, "N");
	strcpy (SR.serialNo, ser_space);
	strcpy (SR.origSerialNo, ser_space);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	CalcTotal (TRUE);
	return (EXIT_SUCCESS);
}

int
InsertParaLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
		    
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [3] >= vars [label ("par_code")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	for (i = line_cnt,line_cnt = lcount [3];line_cnt > i;line_cnt--)
	{
		_hhlh_hash [line_cnt] = _hhlh_hash [line_cnt - 1];
		strcpy (_par_desc [line_cnt], _par_desc [line_cnt - 1]);
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
		 	line_display ();
	}
	lcount [3]++;
	line_cnt = i;

	sprintf (_par_desc [line_cnt], "%-40.40s", " ");
	_hhlh_hash [line_cnt]  = 0L;
	sprintf (local_rec.par_code, "%-10.10s"," ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	if (prog_status == ENTRY)
		prog_status = !prog_status;
	init_ok = 1;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
InsertLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("item_no")].row)
	{
		print_mess (ML (mlStdMess076));
		clear_mess ();
		return (EXIT_FAILURE);
	}

	print_at (0,0,ML (mlStdMess035));

	for (i = line_cnt,line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		memcpy 
		 (
			 (char *) &SR, 
			 (char *) &store [line_cnt - 1],
			sizeof (struct storeRec)
		);
		if (store [line_cnt - 1].min_sell_pric != 0.00)
			SR.sale = store [line_cnt - 1].salePrice;

		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
		 	line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	sprintf (local_rec.item_no, "%-16.16s", " ");
	sprintf (qtln_rec.item_desc, "%-40.40s", " ");
	local_rec.qty          = 0.00;
	local_rec.cost_price   = 0.00;
	qtln_rec.sale_price = 0.00;
	qtln_rec.disc_pc    = 0.00;
	sprintf (local_rec.serial_no, "%-25.25s", " ");

	memset ((char *) &store [line_cnt], '\0', sizeof (struct storeRec));
	SR.contractCost    = -1;
	strcpy (SR.priceOveride, " ");
	strcpy (SR.discOveride, " ");
	strcpy (SR.sellGroup, "      ");
	strcpy (SR.category, "           ");
	strcpy (SR.serialNo,   ser_space);
	strcpy (SR.origSerialNo, ser_space);

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	if (prog_status == ENTRY)
		prog_status = !prog_status;
	init_ok = 1;
	line_cnt = i;
	getval (line_cnt);
	CalcTotal (TRUE);
	return (EXIT_SUCCESS);
}

/*
 * Check Whether A Serial Number For This Item Number 
 * Has Already Been Used.                            
 * Return 1 if duplicate                            
 */
int
CheckDuplicateInsf (
 char *		serial_no,
 long int	hhbr_hash,
 int		line_no)
{
	int		i;
	int		ws_no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < ws_no_items;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == line_no)
			continue;

		/*
		 * cannot duplicate item_no/serial_no unless serial no was not input
		 */
		if (!strcmp (store [i].serialNo,ser_space))
			continue;

		/*
		 * Only compare serial numbers for the same item number	
		 */
		if (store [i].hhbrHash == hhbr_hash)
		{
			if (!strcmp (store [i].serialNo,serial_no))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Find cost routine. 
 */
static double
FindCostValue (
	int	lineNo)
{
	double	wk_cost = 0.00;
	double	cont_fnd = 0;

	store [lineNo].contractCost = FALSE;
	/*
	 * If we have a contract price then check for a matching contract cost.
	 */
	if (store [lineNo].contractStatus)
	{
		/*
		 * Look for contract price. 
		 */
		cont_fnd	=	ContCusPrice 
						 (
							cnch_rec.hhch_hash,
							store [lineNo].hhbrHash,
							store [lineNo].hhccHash,
							cumr_rec.curr_code,
							cnch_rec.exch_type,
							FGN_CURR,
							pocrRec.ex1_factor
						);

		if (cont_fnd != (double)-1.00)
		{
			if (cncd_rec.hhsu_hash > 0.00)
			{
				open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");

				sumr_rec.hhsu_hash	=	cncd_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, sumr, "DBFIND");

				store [lineNo].contractCost = TRUE;
				strcpy (pocr2_rec.co_no, comm_rec.co_no);
				strcpy (pocr2_rec.code, cumr_rec.curr_code);
				cc = find_rec (pocr, &pocr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, pocr, "DBFIND");
				wk_cost = cncd_rec.cost / pocr2_rec.ex1_factor;
				abc_fclose (sumr);
				return (wk_cost);
			}
		}
	}
	switch (store [lineNo].costingFlag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		wk_cost	=	FindIneiCosts
					(
						store [lineNo].costingFlag,
						comm_rec.est_no,
						store [lineNo].hhbrHash
					);
		break;
	case 'F':
		wk_cost	=	FindIncfCost 
					(
						store [lineNo].hhwhHash,
						store [lineNo].qtyAvailable,
						store [lineNo].quantity, 
						TRUE,
						store [lineNo].decPt
					);

		break;

	case 'I':
		wk_cost	=	FindIncfCost 
					(
						store [lineNo].hhwhHash,
						store [lineNo].qtyAvailable,
						store [lineNo].quantity, 
						FALSE,
						store [lineNo].decPt
					);
		break;

	case 'S':
		if (!strcmp (store [lineNo].serialNo, ser_space))
			wk_cost = FindInsfValue (store [lineNo].hhwhHash, TRUE);
		else
		{
			wk_cost	=	FindInsfCost 
						(
							store [lineNo].hhwhHash, 
							0L,
							store [lineNo].serialNo,
				 			"F"
						);

			if (wk_cost == -1.00)
				wk_cost = FindInsfValue (store [lineNo].hhwhHash, TRUE);
		}
	
		break;
	}
	if (wk_cost < 0.00)	
	{
		wk_cost	=	FindIneiCosts
					(
						"L",
						comm_rec.est_no,
						store [lineNo].hhbrHash
					);
	}
	if (wk_cost < 0.00)	
	{
		wk_cost	=	FindIneiCosts
					(
						"T",
						comm_rec.est_no,
						store [lineNo].hhbrHash
					);
	}
	return (CENTS (wk_cost));
}

void
Busy (
	int	flip)
{
	print_at (2, 1, "%-58.58s"," ");
	if (flip)
		print_at (2, 1, ML (mlStdMess035));
}

/*
 * Calculate Defaults for Levy. 
 */
void
CalculateFreight (
 float	_markup,
 double _cost_kg)
{
	int		i;
	float	total_kg = 0.0;
	double	f_value = 0.00;
	float	weight = 0.00;
	double	cal_mup = 0.00;

	qthr_rec.freight = 0.00;
	qthr_rec.no_kgs = 0.00;

	for (i = 0;i < lcount [2];i++)
	{
		weight = (store [i].weight > 0.00) 
			       		  	? store [i].weight 
						: comr_rec.frt_mweight;

		total_kg += (weight * store [i].quantity);
	}
	f_value = (double) total_kg * _cost_kg;

	cal_mup = (double) _markup;
	cal_mup *= f_value;
	cal_mup = DOLLARS (cal_mup);

	f_value += cal_mup;
	f_value = twodec (CENTS (f_value));

	if (f_value < comr_rec.frt_min_amt)
		qthr_rec.freight = comr_rec.frt_min_amt;
	else
		qthr_rec.freight = f_value;

	qthr_rec.no_kgs = total_kg;

	return;
}

/*
 * Calculate totals. 
 */
void
CalcTotal (
 int	draw_total)
{
	int		i;
	int		ws_no_lines = 0;

	ws_no_lines = (prog_status == ENTRY && 
				 (lcount [2] - 1 < line_cnt)) ? 
				  line_cnt : lcount [2] - 1;
	cst_tot = 0.00;
	min_tot = 0.00; 
	inv_tot = 0.00;
	dis_tot = 0.00;
	tax_tot = 0.00;
	gst_tot = 0.00;
	sal_tot = 0.00;
	tot_tot = 0.00;

	for (i = 0; i <= ws_no_lines; i++) 
	{
		if (strcmp (store [i].alternate, "Y"))
		{
			CalExtend (i);

			cst_tot += l_cost;
			inv_tot += l_total;
			dis_tot += l_disc;
			tax_tot += l_tax;
			gst_tot += l_gst;
			min_tot += l_min;
			sal_tot += l_sale;
		}
	}

	gst_tot = no_dec (gst_tot);

	tot_tot = no_dec (inv_tot - dis_tot + tax_tot + gst_tot);

	if (cst_tot == 0.00)
	{
		l_bgp = 0.00;
		l_agp = 0.00;
	}
	else
	{
		if (inv_tot != 0.00)
			l_bgp = ((inv_tot) - cst_tot) * 100 / inv_tot;
		else
			l_bgp = 0.00;
		
		if ((inv_tot - dis_tot) != 0.00)
			l_agp = ((inv_tot - dis_tot) - cst_tot) * 100 / (inv_tot - dis_tot);
		else
			l_agp = 0.00; 
	}

	l_bgp = no_dec (l_bgp);
	l_agp = no_dec (l_agp);

	if (draw_total)
		PrintTotal ();

}

/*
 * Calculate extended Values. 
 */
void
CalExtend (
	int	lineNo)
{
	/*
	 * Update gross tax and disc for each line. 
	 */
	l_cost = store [lineNo].quantity;
	l_cost *= out_cost (store [lineNo].costPrice, store [lineNo].outerSize);
	l_cost = no_dec (l_cost);

	l_total = (double) store [lineNo].quantity;
	l_total *= out_cost (store [lineNo].actSalePrice, store [lineNo].outerSize);
	l_total = no_dec (l_total);

	if (notax)
		t_total = 0.00;
	else
	{
		t_total = (double) (store [lineNo].quantity*store [lineNo].cnvFct);
		t_total *= out_cost (store [lineNo].taxAmount, store [lineNo].outerSize);
		t_total = no_dec (t_total);
	}

	l_disc = (double) store [lineNo].disPc;
	l_disc *= l_total;
	l_disc = DOLLARS (l_disc);
	l_disc = no_dec (l_disc);

	if (l_total != 0.00)
		l_min = store [lineNo].min_sell_pric;
	else
		l_min = 0.00;

	if (l_min > 0.00) 
	{
		if (store [lineNo].cnvFct == 0.00)
			store [lineNo].cnvFct = 1.00;

		if (store [lineNo].disPc != 0.00)
		{
			dis_dec  = store [lineNo].disPc/100;
			disc_amt = (float) (store [lineNo].sale * dis_dec); 
			net_sal_tot   = (store [lineNo].sale - disc_amt)
								/store [lineNo].cnvFct;
			
			if (net_sal_tot >= l_min)
				l_sale = l_min;
			else
				l_sale = net_sal_tot;
		}
		else
		{
			net_sal_tot = store [lineNo].sale/store [lineNo].cnvFct;

			if (net_sal_tot >= l_min)
				l_sale = l_min;
			else
				l_sale = net_sal_tot;
		}
	}
	else
		l_sale = 0.00;	

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) store [lineNo].taxPc;
		if (cumr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
			l_tax *= (l_total - l_disc);

		l_tax = DOLLARS (l_tax);
	}
	
	l_tax = no_dec (l_tax);

	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) store [lineNo].gstPc;
		l_gst *= ((l_total - l_disc) + l_tax);
		l_gst = DOLLARS (l_gst);
	}
}

/*
 * Routine to read all qtln records whose hash matches the one on the    
 * qthr record. Stores all non screen relevant details in another       
 * structure. Also gets part number for the part hash. And G/L account  
 * number.                                                             
 * Also reads all qtlp records ie. Paragraphs for Quotation Letter.   
 */
int
LoadQtln (
	long	hhqtHash)
{
	float	realCommitted;
	float	std_cnv_fct;

	Busy (1);
	
	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (2);
	lcount [2] = 0;

	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (inum,"inum_hhum_hash");

	qtln_rec.hhqt_hash = hhqtHash;
	qtln_rec.line_no = 0; 

	cc = find_rec (qtln, &qtln_rec, GTEQ, "r");
	while (!cc && qtln_rec.hhqt_hash == hhqtHash)
	{
		/*
		 * Get part number. 
		 */
		if (qtln_rec.hhbr_hash != 0L)
		{
			inmr_rec.hhbr_hash	=	qtln_rec.hhbr_hash;
            cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, inmr, "DBFIND");

			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "inum", "DBFIND");

			std_cnv_fct	=	inum_rec.cnv_fct;

			inum_rec.hhum_hash	=	qtln_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "inum", "DBFIND");

			line_cnt	= lcount [2];

			if (std_cnv_fct == 0.00)
				std_cnv_fct = 1;

			store [lcount [2]].cnvFct 	= inum_rec.cnv_fct/std_cnv_fct;
			store [lcount [2]].hhumHash 	= inum_rec.hhum_hash;
			strcpy (local_rec.UOM, qtln_rec.uom); 

			qtln_rec.sale_price				*= SR.cnvFct;	
			qtln_rec.gsale_price			*= SR.cnvFct;	

			if (!strcmp (qtln_rec.alt_flag, "Y"))
			{
				strcpy (local_rec.item_no, clip (inmr_rec.item_no));
				strcat (local_rec.item_no, "/A");
			}
			else
				strcpy (local_rec.item_no,inmr_rec.item_no);
		}
		else
		{
			if (!strcmp (qtln_rec.alt_flag, "Y"))
				strcpy (local_rec.item_no, "9999/A");
			else
			{
				strcpy (inmr_rec.item_no, "9999");
				strcpy (local_rec.item_no, "9999");
			}
		}

		if (strncmp (local_rec.item_no, "9999", 4))
		{
			incc_rec.hhcc_hash = qtln_rec.hhcc_hash;
			incc_rec.hhbr_hash = qtln_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, incc, "DBFIND");
		
			store [lcount [2]].hhwhHash = incc_rec.hhwh_hash;

			if (inmr_rec.serial_item [0] == 'Y')
				sprintf (store [lcount [2]].serialNo,"%-25.25s",qtln_rec.serial_no);
			else
				sprintf (store [lcount [2]].serialNo,"%-25.25s"," ");
			strcpy (local_rec.serial_no, store [lcount [2]].serialNo);
			strcpy (store [lcount [2]].origSerialNo, store [lcount [2]].serialNo);

			strcpy (store [lcount [2]].serialFlag,inmr_rec.serial_item);

			/*
			 * Calculate Actual Qty Committed. 
			 */
			realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,
											incc_rec.hhcc_hash);
			if (envVar.includeForwardStock)
			{
				store [lcount [2]].qtyAvailable	=	incc_rec.closing_stock -
					 	  				  			incc_rec.committed -
										  			realCommitted -
					 	  				  			incc_rec.backorder - 
					 	  				  			incc_rec.forward;
			}
			else
			{
				store [lcount [2]].qtyAvailable = 	incc_rec.closing_stock -
													incc_rec.committed -
													realCommitted -
													incc_rec.backorder;
			}
		}
		else
			store [lcount [2]].qtyAvailable = qtln_rec.qty_avail;
		
		local_rec.qty                 	= ToLclUom (qtln_rec.qty);
		local_rec.qty_avail			  	= store [lcount [2]].qtyAvailable;
		store [lcount [2]].quantity         = ToLclUom (qtln_rec.qty);
		store [lcount [2]].disPc      = qtln_rec.disc_pc;
		store [lcount [2]].regPc      = qtln_rec.reg_pc;
		store [lcount [2]].discA      = qtln_rec.disc_a;
		store [lcount [2]].discB      = qtln_rec.disc_b;
		store [lcount [2]].discC      = qtln_rec.disc_c;
		store [lcount [2]].cumulative  = qtln_rec.cumulative;
		store [lcount [2]].contractStatus = qtln_rec.cont_status;
		store [lcount [2]].contractCost   = FALSE;
		store [lcount [2]].hhbrHash   = qtln_rec.hhbr_hash;
		store [lcount [2]].hhccHash   = qtln_rec.hhcc_hash;

		if (!strcmp (qtln_rec.pri_or, "Y"))
			strcpy (local_rec.reset_flag, "N");

		if (strncmp (local_rec.item_no, "9999", 4))
		{
			strcpy (store [lcount [2]].category,     inmr_rec.category);
			strcpy (store [lcount [2]].sellGroup,      inmr_rec.sellgrp);
			strcpy (store [lcount [2]].costingFlag, inmr_rec.costing_flag);
			store [lcount [2]].defaultDisc 		= inmr_rec.disc_pc;
			store [lcount [2]].min_sell_pric  	= inmr_rec.min_sell_pric;
			store [lcount [2]].taxPc    		= inmr_rec.tax_pc;
			store [lcount [2]].gstPc    		= inmr_rec.gst_pc;
			store [lcount [2]].taxAmount   		= inmr_rec.tax_amount;
			store [lcount [2]].outerSize     		= inmr_rec.outer_size;
			store [lcount [2]].weight    		= inmr_rec.weight;
			store [lcount [2]].hhsiHash 		= 	alt_hash 
													 (
														inmr_rec.hhbr_hash, 
														inmr_rec.hhsi_hash
													);
			store [lcount [2]].costPrice 		= FindCostValue (lcount [2]);
			store [lcount [2]].costPrice 		*= store [lcount [2]].cnvFct;
		}
		else
		{
			strcpy (store [lcount [2]].category,   " ");
			strcpy (store [lcount [2]].sellGroup,    " ");
			strcpy (store [lcount [2]].costingFlag, " ");
			store [lcount [2]].defaultDisc = 0.00;
			store [lcount [2]].taxPc    = 0.00;
			store [lcount [2]].gstPc    = 0.00;
			store [lcount [2]].taxAmount   = 0.00;
			store [lcount [2]].outerSize     = 0.00;
			store [lcount [2]].weight    = 0.00;
			store [lcount [2]].hhsiHash = 0L; 
			store [lcount [2]].costPrice = qtln_rec.cost_price * 
										   store [lcount [2]].cnvFct;
		}
		strcpy (store [lcount [2]].uom,   qtln_rec.uom);
		strcpy (store [lcount [2]].alternate,   qtln_rec.alt_flag);
		store [lcount [2]].grossSalePrice = qtln_rec.gsale_price;
		store [lcount [2]].salePrice  = qtln_rec.sale_price;
		store [lcount [2]].actSalePrice    = qtln_rec.sale_price;

		store [lcount [2]].origLine  = qtln_rec.line_no;
		local_rec.cost_price = store [lcount [2]].costPrice;

		if (store [lcount [2]].min_sell_pric != 0.00)
			store [lcount [2]].sale = qtln_rec.sale_price;
		else
			store [lcount [2]].sale = 0.00;

		/*
		 * Check for Indent items. 
		 */
		if (strncmp (inmr_rec.item_no,"9999",4) || envVar.discountIndents)
			store [lcount [2]].indent = FALSE;
		else
			store [lcount [2]].indent = TRUE;

		qtln_rec.disc_pc = ScreenDisc (qtln_rec.disc_pc);
		putval (lcount [2]++);

		cc = find_rec (qtln, &qtln_rec, NEXT, "r");
		continue;
	}
	
	if (specialDisplay)
		vars [scn_start].row = lcount [2];
		
	abc_selfield (inmr,"inmr_id_no");
	abc_selfield (inum,"inum_uom");
	oldLines	= lcount [2];

	/*
	 * Set screen 3 - for putval. 
	 */
	scn_set (3);
	lcount [3] = 0;

	abc_selfield (qtlh, "qtlh_hhlh_hash");

	qtlp_rec.hhqt_hash = hhqtHash;
	qtlp_rec.line_no = 0; 

	cc = find_rec (qtlp, &qtlp_rec, GTEQ, "r");
	while (!cc && qtlp_rec.hhqt_hash == hhqtHash)
	{
		/*
		 * Get paragraph code 
		 */
		qtlh_rec.hhlh_hash	=	qtlp_rec.hhlh_hash;
		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (local_rec.par_code, qtlh_rec.par_code);
			_hhlh_hash [lcount [3]] = qtlh_rec.hhlh_hash;
			strcpy (_par_desc [lcount [3]], qtlh_rec.par_desc);
			putval (lcount [3]++);
		}
		cc = find_rec (qtlp,&qtlp_rec,NEXT,"r");
	}
	
	abc_selfield (qtlh,"qtlh_id_no");

	scn_set (1);

	/*
	 * No entries to edit. 
	 */
	if (lcount [2] == 0)
		return (EXIT_FAILURE);
	/*
	 * No exit - return 0  
	 */
	return (EXIT_SUCCESS);
}
	
int	
Update (void)
{
	int 	i = 0,
			err_cnt = 0,
			lineNo = 0;

	int	add_item = FALSE;

	clear ();
		
	if (newQuote && lcount [2] == 0)
	{
		char	mesg [1024];

		sprintf (mesg, "%s %s", ML (mlQtMess054), ML (mlStdMess042));
		PauseForKey (1, 0, mesg, 0);
		return (EXIT_SUCCESS);
	}

	strcpy (qthr_rec.carr_area, local_rec.carr_area);
	strcpy (qthr_rec.carr_code, cfhr_rec.carr_code);
	sprintf (qthr_rec.op_id, "%-14.14s", curr_user);
	qthr_rec.date_create = TodaysDate ();
	strcpy (qthr_rec.time_create, TimeHHMM ());
	qthr_rec.exch_rate	=	pocrRec.ex1_factor;

	/*
	 * Add new purchase order header. 
	 */
	if (newQuote) 
	{
		HiLight (ML (mlQtMess064));
		abc_selfield (qthr,"qthr_hhqt_hash");
		strcpy (qthr_rec.co_no, comm_rec.co_no);
		strcpy (qthr_rec.br_no, comm_rec.est_no);
		if (!specialNo)
			qthr_rec.hhcu_hash = cumr_rec.hhcu_hash; 
		else
			qthr_rec.hhcu_hash = 0L;

		CalcTotal (FALSE);
		
		if (inv_tot != 0.00)
		{
			qthr_rec.qt_profit_cur	= (inv_tot - (inv_tot - dis_tot));
			qthr_rec.qt_profit_pc	= (float) (( (inv_tot - dis_tot)/inv_tot)*100.00);
			qthr_rec.qt_value		= (inv_tot - dis_tot);
		}
		else
		{
			qthr_rec.qt_profit_cur	= 0.00;
			qthr_rec.qt_profit_pc	= 0.00;
			qthr_rec.qt_value		= 0.00;
		}

		strcpy (qthr_rec.stat_flag, "0");

		if (transferqt)
			strcpy (qthr_rec.status, "20");
		else
			strcpy (qthr_rec.status, "45");

		cc = abc_add (qthr,&qthr_rec);
		if (cc) 
			file_err (cc, "qthr", "DBADD");

		abc_selfield (qthr, "qthr_hhqt_hash");

		cc = find_rec (qthr, &qthr_rec, LAST, "r");
		if (cc)
			return (EXIT_FAILURE);

		sprintf (qthr_rec.quote_no, "%08ld", qthr_rec.hhqt_hash);
		sprintf (qthr_rec.prt_name, "q.%-8ld", qthr_rec.hhqt_hash);
		cc = abc_update (qthr, &qthr_rec);
		if (cc)
			file_err (cc, "qthr", "DBUPDATE");

		strcpy (local_rec.quote_no, qthr_rec.quote_no);

		abc_selfield (qthr, "qthr_id_no");
		abc_unlock (qthr);

		while (!cc && 
			 (qthr_rec.hhcu_hash != cumr_rec.hhcu_hash || 
				strcmp (qthr_rec.co_no, comm_rec.co_no) || 
				strcmp (qthr_rec.br_no, comm_rec.est_no)))
		{
			err_cnt++;
			cc = find_rec (qthr, &qthr_rec, LT, "r");
		}
		if (err_cnt > 3)
			return (EXIT_FAILURE);
		
		abc_selfield (qthr, "qthr_id_no");
		HiLight (ML (mlQtMess065));
	}
	else
		HiLight (ML (mlQtMess066));

	/*
	 * Process all purchase order lines. 
	 */
	scn_set (2);
	for (lineNo = 0; lineNo < lcount [2]; lineNo++) 
	{
		if (!strcmp (qthr_rec.status, "45"))
			strcpy (qthr_rec.status, "45");

		qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
		qtln_rec.line_no   = lineNo;
		if (find_rec (qtln, &qtln_rec, COMPARISON, "u"))
			add_item = TRUE;
		else
			add_item = FALSE;

		getval (lineNo);

		qtln_rec.hhqt_hash   = qthr_rec.hhqt_hash;
		qtln_rec.line_no     = lineNo;
		qtln_rec.hhbr_hash   = store [lineNo].hhbrHash;
		qtln_rec.hhcc_hash   = store [lineNo].hhccHash;
		qtln_rec.hhum_hash   = store [lineNo].hhumHash;
		if (store [lineNo].cnvFct == 0.00)
			store [lineNo].cnvFct = 1.00;
		qtln_rec.sale_price  = n_dec ((store [lineNo].salePrice
							 / store [lineNo].cnvFct), 5);
		qtln_rec.gsale_price = n_dec ((store [lineNo].grossSalePrice
							 / store [lineNo].cnvFct), 5);
		qtln_rec.reg_pc      = store [lineNo].regPc;
		qtln_rec.disc_a      = store [lineNo].discA;
		qtln_rec.disc_b      = store [lineNo].discB;
		qtln_rec.disc_c      = store [lineNo].discC;
		qtln_rec.cumulative  = store [lineNo].cumulative;
		qtln_rec.cont_status = store [lineNo].contractStatus;
		qtln_rec.qty         = local_rec.qty * store [lineNo].cnvFct;
		qtln_rec.exp_date    = qthr_rec.expire_date;
		if (qtln_rec.hhbr_hash == 0L)
		{
			qtln_rec.qty_avail = store [lineNo].qtyAvailable; 
			qtln_rec.cost_price= store [lineNo].costPrice/
								 store [lineNo].cnvFct; 
			qtln_rec.cont_status = 0;
		}
		strcpy (qtln_rec.uom, local_rec.UOM);
		strcpy (qtln_rec.serial_no, store [lineNo].serialNo);
		strcpy (qtln_rec.stat_flag, "0");
		strcpy (qtln_rec.alt_flag, store [lineNo].alternate);
		if (!strcmp (local_rec.reset_flag, "N"))
			strcpy (qtln_rec.pri_or, "Y");

		if (add_item)
		{
			qtln_rec.disc_pc = ScreenDisc (qtln_rec.disc_pc);
			cc = abc_add (qtln, &qtln_rec);
			if (cc) 
				file_err (cc, "qtln", "DBADD");
		}
		else
		{
			if (qtln_rec.qty == 0.00)
			{
				/*
				 * Delete existing order. 
				 */
				cc = abc_delete (qtln);
				if (cc) 
					file_err (cc, "qtln", "DBDELETE");
			}
			else
			{
				/*
				 * Update existing order. 
				 */
				qtln_rec.disc_pc = ScreenDisc (qtln_rec.disc_pc);
				cc = abc_update (qtln, &qtln_rec);
				if (cc) 
					file_err (cc, "qtln", "DBUPDATE");
			}
			abc_unlock (qtln);
		}
	}

	i = lineNo;
    for (lineNo = i; lineNo < MAXLINES; lineNo++)
    {
		qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
		qtln_rec.line_no   = lineNo;
		if (!find_rec (qtln, &qtln_rec, COMPARISON, "r"))
			abc_delete (qtln);
		else
			break;
	}

	/*
	 * Process all paragraph lines. 
	 */
	scn_set (3);
	for (lineNo = 0;lineNo < lcount [3];lineNo++) 
	{
		getval (lineNo);
		strcpy (qtlh_rec.co_no,comm_rec.co_no);
		strcpy (qtlh_rec.par_code,local_rec.par_code);
		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "qtlh", "DBFIND");
	
		UpdateQtlp (lineNo);
	}

	i = lineNo;
    for (lineNo = i;lineNo < MAXLINES; lineNo++)
    {
		qtlp_rec.hhqt_hash = qthr_rec.hhqt_hash;
		qtlp_rec.line_no = lineNo;
		cc = find_rec (qtlp, &qtlp_rec, COMPARISON, "u");
		if (!cc)
			abc_delete (qtlp);
		else
			break;

	}
	/*
	 * Update existing order header. 
	 */
	if (newQuote)
	{
		char	mesg [1024];

		sprintf (err_str, ML (mlQtMess063), qthr_rec.hhqt_hash);
		sprintf (mesg, "%s %s", err_str, ML (mlStdMess042));
		PauseForKey (0, 0, mesg, 0);
	}
	else
	{	
		/*
		 * Delete cancelled order. 
		 */
		if (lcount [2] == 0) 
		{
			HiLight (ML (mlQtMess067));
			abc_unlock (qthr);
			cc = abc_delete (qthr);
			if (cc)
				file_err (cc, "qthr", "DBDELETE");
		}
		else
		{
			HiLight (ML (mlQtMess068));
			strcpy (qthr_rec.co_no, comm_rec.co_no);
			strcpy (qthr_rec.br_no, comm_rec.est_no);
			strcpy (qthr_rec.quote_no, local_rec.quote_no);
			if (transferqt)
				strcpy (qthr_rec.status, "20");

			CalcTotal (FALSE);
			strcpy (qthr_rec.carr_area, local_rec.carr_area);
			sprintf (qthr_rec.prt_name, "q.%-8ld", qthr_rec.hhqt_hash);
			if (inv_tot != 0.00)
			{
				qthr_rec.qt_profit_cur  = (inv_tot - (inv_tot - dis_tot));
				qthr_rec.qt_profit_pc	= (float) (( (inv_tot - dis_tot)/inv_tot)*100.00);
				qthr_rec.qt_value		= (inv_tot - dis_tot);
			}
			else
			{
				qthr_rec.qt_profit_cur	= 0.00;
				qthr_rec.qt_profit_pc	= 0.00;
				qthr_rec.qt_value		= 0.00;
			}
			cc = abc_update (qthr,&qthr_rec);
			if (cc) 
				file_err (cc, "qthr", "DBUPDATE");

			abc_unlock (qthr);
		}
	}

	if (!strcmp (qthr_rec.status, "20"))
	{
		PrintLetter ();
	}

	return (EXIT_SUCCESS);
}

/*
 * Update quotation paragraph file. 
 */
void
UpdateQtlp (
 int	lineNo)
{
	int 	add_item = FALSE;

	qtlp_rec.hhqt_hash = qthr_rec.hhqt_hash;
	qtlp_rec.line_no = lineNo;
	if (find_rec (qtlp, &qtlp_rec, COMPARISON, "u"))
		add_item = TRUE;
	else
		add_item = FALSE;

	qtlp_rec.hhqt_hash = qthr_rec.hhqt_hash;
	qtlp_rec.line_no   = lineNo;
	qtlp_rec.hhlh_hash = qtlh_rec.hhlh_hash;

	if (add_item)
	{
		cc = abc_add (qtlp, &qtlp_rec);
		if (cc) 
			file_err (cc, "qtlp", "DBADD");
	}
	else
	{
		/*
		 * Update existing letter 
		 */
		cc = abc_update (qtlp, &qtlp_rec);
		if (cc) 
			file_err (cc, "qtlp", "DBUPDATE");
		abc_unlock (qtlp);
	}
}

void
HiLight (
 char *	text)
{
	print_at (0,0,"\n\r%s ", text);
}

/*
 * Search for order number. 
 */
void
SrchQthr (
 char *	key_val)
{
	char	wk_mask [9];

	_work_open (8,0,40);
	save_rec ("#Qt No.","#Quote Date| Contact");
	abc_selfield (qthr,"qthr_id_no2");
	strcpy (qthr_rec.co_no, comm_rec.co_no);
	strcpy (qthr_rec.br_no, comm_rec.est_no);
	sprintf (qthr_rec.quote_no,"%-8.8s",key_val);
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strncmp (qthr_rec.quote_no,key_val,strlen (key_val)) && 
		   !strcmp (qthr_rec.co_no, comm_rec.co_no) && 
		   !strcmp (qthr_rec.br_no, comm_rec.est_no))
	{
		sprintf (err_str, "%-10.10s|%s", DateToString (qthr_rec.dt_quote),
										qthr_rec.cont_name);

		sprintf (wk_mask, "%-8.8s", qthr_rec.quote_no);
		cc = save_rec (wk_mask,err_str);
		if (cc)
				break;

		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qthr_rec.co_no, comm_rec.co_no);
	strcpy (qthr_rec.br_no, comm_rec.est_no);
	sprintf (qthr_rec.quote_no, "%-8.8s", temp_str);
	cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
	{
		file_err (cc, "qthr", "DBFIND");
	}
	abc_selfield (qthr,"qthr_id_no");
}

/*
 * Search routine for Quote Status. 
 */
void
SrchStatus (void)
{
	int 	i = 0;

	_work_open (2,0,40);
	save_rec ("#Cd","#Status Description");
		
	for (i =0;strlen (q_status [i]._stat);i++)
	{
		cc = save_rec (q_status [i]._stat,q_status [i]._desc);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search routine for Position Code. 
 */
void
SrchTmpf (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#No","#Position code description");
	strcpy (tmpf_rec.co_no, comm_rec.co_no);
	sprintf (tmpf_rec.pos_code, "%-3.3s", key_val);
	cc = find_rec (tmpf, &tmpf_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (tmpf_rec.co_no,comm_rec.co_no) && 
		   !strncmp (tmpf_rec.pos_code, key_val, strlen (key_val)))
	{
		cc = save_rec (tmpf_rec.pos_code,tmpf_rec.pos_desc);
		if (cc)
			break;

		cc = find_rec (tmpf, &tmpf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmpf_rec.co_no, comm_rec.co_no);
	sprintf (tmpf_rec.pos_code, "%-3.3s", temp_str);
	cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "tmpf", "DBFIND");
}

/*
 * Search routine for Salesman file. 
 */
void
SrchExsf (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Salesperson code description");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		   !strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

/*
 * Search routine for Paragraph file. 
 */
void
SrchQtln (
	char	*key_val)
{
	_work_open (10,0,40);
	save_rec ("#Code","#Paragraph code description");
	strcpy (qtlh_rec.co_no, comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", key_val);
	cc = find_rec (qtlh, &qtlh_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (qtlh_rec.co_no, comm_rec.co_no) && 
		   !strncmp (qtlh_rec.par_code,key_val,strlen (key_val)))
	{
		cc = save_rec (qtlh_rec.par_code, qtlh_rec.par_desc);
		if (cc)
			break;
		cc = find_rec (qtlh, &qtlh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (qtlh_rec.co_no, comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", temp_str);
	cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "qtlh", "DBFIND");
}


/*
 * Search routine for Serial Item master file. 
 */
void
SrchInsf (
 char *	key_val,
 int	line_no)
{
	_work_open (25,0,20);
	save_rec ("#      Serial Item.         ","# ");

	insfRec.hhwh_hash = store [line_no].hhwhHash;
	strcpy (insfRec.status, "F");
	sprintf (insfRec.serial_no, "%-25.25s", key_val);
	cc = find_rec (insf, &insfRec, GTEQ, "r");

	while (!cc && 
		   store [line_no].hhwhHash == insfRec.hhwh_hash && 
		   insfRec.status [0] == 'F' && 
		   !strncmp (insfRec.serial_no, key_val, strlen (key_val)))
	{
		if (!CheckDuplicateInsf (insfRec.serial_no,
		     			 store [line_no].hhbrHash, line_no))
		{
			cc = save_rec (insfRec.serial_no, inmr_rec.item_no);
			if (cc)
				break;
		}
		cc = find_rec (insf, &insfRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	insfRec.hhwh_hash = store [line_no].hhwhHash;
	strcpy (insfRec.status, "F");
	sprintf (insfRec.serial_no, "%-25.25s", temp_str);
	cc = find_rec (insf, &insfRec, COMPARISON, "r");
	if (cc)
		file_err (cc, insf, "DBFIND");
	
	strcpy (local_rec.serial_no,insfRec.serial_no);
}

void
PrintTotal (void)
{
	crsr_off ();
	print_at (1,62,ML (mlQtMess055),DOLLARS (cst_tot));

	move (62,3);
	print_at (3,62,ML (mlQtMess056),l_bgp);
	move (62,4);
	print_at (4,62,ML (mlQtMess057),l_agp);

	move (100,1);
	print_at (1,100,ML (mlQtMess058),DOLLARS (inv_tot));
	move (100,2);
	print_at (2,100,ML (mlQtMess059),DOLLARS (dis_tot));
	move (100,3);
	if (envVar.gstApplies)
		print_at (3,100,ML (mlQtMess060), envVar.gstCode, DOLLARS (tax_tot + gst_tot));
	else
		print_at (3,100,ML (mlQtMess061), DOLLARS (tax_tot + gst_tot));
	move (100,4);
	print_at (4,100,ML (mlQtMess062),DOLLARS (tot_tot));
	crsr_on ();
}

/*
 * Routine to copy a quote. 
 */
int
CopyQuote (
 long int	quote_no)
{
	int 	i = 0;
	int		lines_added = FALSE;
	char	tempalt_flag [2];

	clear ();
	HiLight (ML (mlStdMess064));

	/*
	 * Add Quotation Header. 
	 */
	qthr_rec.hhqt_hash = 0L; 
	strcpy (qthr_rec.status, "00");
	strcpy (qthr_rec.quote_no, "00000000");
	cc = abc_add (qthr,&qthr_rec);
	if (cc) 
	{
		file_err (cc, "qthr", "DBADD");
	}

	abc_selfield (qthr, "qthr_hhqt_hash");

	cc = find_rec (qthr, &qthr_rec, LAST, "r");
	if (cc)
		return (EXIT_FAILURE);

	sprintf (qthr_rec.quote_no, "%08ld", qthr_rec.hhqt_hash);
	sprintf (qthr_rec.prt_name, "q.%-8ld", qthr_rec.hhqt_hash);
	cc = abc_update (qthr, &qthr_rec);
	if (cc)
		file_err (cc, "qthr", "DBUPDATE");

	abc_selfield (qthr, "qthr_id_no");
	abc_unlock (qthr);

	HiLight (ML (mlQtMess065));

	/*
	 * Add Quotation Lines. 
	 */
	for (i = 0; i < MAXLINES; i++)
	{
		qtln_rec.hhqt_hash = quote_no;
		qtln_rec.line_no = i;
		cc = find_rec (qtln, &qtln_rec, COMPARISON, "r");
		if (cc)
			break;

		strcpy (tempalt_flag, qtln_rec.alt_flag);

		strcpy (qtln_rec.alt_flag, tempalt_flag);
		qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
		strcpy (qtln_rec.serial_no, ser_space);
		cc = abc_add (qtln, &qtln_rec);
		if (cc) 
			file_err (cc, "qtln", "DBADD");

		lines_added = TRUE;
	}
	if (lines_added)
	{
		HiLight (ML (mlQtMess069));

		abc_selfield (qtlh, "qtlh_hhlh_hash");

		/*
		 * Add Paragraph Details. 
		 */
		for (i = 0; i < MAXLINES; i++)
		{
			qtlp_rec.hhqt_hash = quote_no;
			qtlp_rec.line_no = i; 
			cc = find_rec (qtlp, &qtlp_rec, COMPARISON, "r");
			if (cc)
				break;

			qtlp_rec.hhqt_hash = qthr_rec.hhqt_hash;
			cc = abc_add (qtlp, &qtlp_rec);
			if (cc) 
				file_err (cc, "qtlp", "DBADD");

		}
		abc_selfield (qtlh,"qtlh_id_no");
	}
	if (lines_added)
	{
		char	mesg [1024];

		sprintf (err_str, ML (mlQtMess063), qthr_rec.hhqt_hash);
		sprintf (mesg, "%s %s", err_str, ML (mlStdMess042));
		PauseForKey (0, 0, mesg, 0);
	}
	else
	{
		abc_delete (qthr);
		sprintf (err_str, ML (mlQtMess070), qthr_rec.hhqt_hash);
		HiLight (err_str);	
		HiLight (ML (mlStdMess042));

	}
	return (EXIT_SUCCESS);
}

/*
 * Routine to copy a quote. 
 */
int
CreateSord (
 long int	quote_no)
{
	int 	i = 0;
	char	mesg [1024];

	clear ();
	HiLight (ML (mlQtMess071));

	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,    qthr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman,"%-40.40s", " ");
	
	abc_fclose (exsf);

	abc_selfield (cumr, "cumr_hhcu_hash");

	cumr_rec.hhcu_hash	=	qthr_rec.hhcu_hash;
	if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
		return (EXIT_FAILURE);

	/*
	 * New Order 
	 */
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");

	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	/*
	 * Check if Order No Already Allocated if it has been then skip
	 */
	while (CheckSohr (++esmr_rec.nx_order_no) == 0);

	cc = abc_update (esmr,&esmr_rec);
	if (cc)
		file_err (cc, "esmr", "DBUPDATE");

	sprintf (sohr_rec.order_no, "%08ld", esmr_rec.nx_order_no);

	abc_unlock (esmr);
	abc_fclose (esmr);

	AddSohr ();

	abc_fclose (esmr);
	abc_fclose (sohr);

	abc_selfield (inmr, "inmr_hhbr_hash");

	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");

	HiLight (ML (mlQtMess072));
	for (i = 0; i < MAXLINES; i++)
	{
		qtln_rec.hhqt_hash = quote_no;
		qtln_rec.line_no = i;
		cc = find_rec (qtln, &qtln_rec, COMPARISON, "u");
		if (cc)
			break;

		AddSoln (i);

		cc = abc_update (qtln, &qtln_rec);
		if (cc) 
			file_err (cc, "qtln", "DBUPDATE");

	}
	abc_selfield (inmr, "inmr_id_no");

	abc_fclose (soln);

	HiLight (ML (mlQtMess051));

	add_hash (comm_rec.co_no, comm_rec.est_no, "RO", 0,
			 cumr_rec.hhcu_hash, 0L, 0L, 0.00);

	recalc_sobg ();

	sprintf (err_str, ML (mlQtMess073), sohr_rec.order_no);
	sprintf (mesg, "%s %s", err_str, ML (mlStdMess042));
	PauseForKey (0, 0, mesg, 0);

	return (EXIT_SUCCESS);
}

/*
 * Add sales order header. 
 */
void
AddSohr (void)
{
	/*
	 * Get any special instrunctions. 
	 */
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = cumr_rec.inst_fg1;
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc) 
		sprintf (sohr_rec.din_1, "%60.60s", " ");
	else 
		sprintf (sohr_rec.din_1, "%-60.60s", exsi_rec.inst_text);

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = cumr_rec.inst_fg2;
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc) 
		sprintf (sohr_rec.din_2, "%60.60s", " ");
	else 
		sprintf (sohr_rec.din_2, "%-60.60s", exsi_rec.inst_text);

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = cumr_rec.inst_fg3;
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc) 
		sprintf (sohr_rec.din_3, "%60.60s", " ");
	else 
		sprintf (sohr_rec.din_3, "%-60.60s", exsi_rec.inst_text);

	abc_fclose (exsi);

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	strcpy (sohr_rec.dp_no, cumr_rec.department);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sohr_rec.hhso_hash = 0L;
	sprintf (sohr_rec.cus_ord_ref, "%-20.20s", qthr_rec.enq_ref);
	strcpy (sohr_rec.carr_code,      qthr_rec.carr_code);
	strcpy (sohr_rec.carr_area,      qthr_rec.carr_area);
	strcpy (sohr_rec.cont_no,      qthr_rec.cont_no);
	strcpy (sohr_rec.ord_type,     "D");
	strcpy (sohr_rec.pri_type,     cumr_rec.price_type);
	strcpy (sohr_rec.frei_req,     "N");
	sohr_rec.dt_raised   = comm_rec.dbt_date;
	sohr_rec.dt_required = qthr_rec.del_date; 
	sohr_rec.exch_rate	=	pocrRec.ex1_factor;
	strcpy (sohr_rec.tax_code,   cumr_rec.tax_code);
	strcpy (sohr_rec.tax_no,     cumr_rec.tax_no);
	strcpy (sohr_rec.area_code,  cumr_rec.area_code);
	strcpy (sohr_rec.sman_code,  qthr_rec.sman_code);
	strcpy (sohr_rec.sell_terms, qthr_rec.sell_terms);
	strcpy (sohr_rec.pay_term,   qthr_rec.pay_term);
	sohr_rec.freight = qthr_rec.freight;
	strcpy (sohr_rec.fix_exch,   qthr_rec.fix_exch);
	strcpy (sohr_rec.batch_no,   "00000");
	strcpy (sohr_rec.del_name,   qthr_rec.del_name);
	strcpy (sohr_rec.del_add1, qthr_rec.del_add1);
	strcpy (sohr_rec.del_add2, qthr_rec.del_add2);
	strcpy (sohr_rec.del_add3, qthr_rec.del_add3);
	strcpy (sohr_rec.sohr_new, qthr_rec.sos); 
	strcpy (sohr_rec.prt_price,  "Y");
	strcpy (sohr_rec.status,     "M");
	strcpy (sohr_rec.stat_flag,  "M");

	cc = abc_add (sohr,&sohr_rec);
	if (cc) 
		file_err (cc, "sohr", "DBADD");
	
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "sohr", "DBFIND");

	abc_unlock (sohr);
}

/* 
 * Add sales order line items. 
 */
int
AddSoln (
 int	line_add)
{
	inmr_rec.hhbr_hash	=	qtln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	soln_rec.hhso_hash   = sohr_rec.hhso_hash;
	soln_rec.line_no     = line_add;
	soln_rec.hhbr_hash   = qtln_rec.hhbr_hash;
	soln_rec.hhum_hash   = inmr_rec.std_uom;
	soln_rec.hhcc_hash   = qtln_rec.hhcc_hash;
	soln_rec.hhsl_hash   = 0L;
	soln_rec.qty_order   = qtln_rec.qty;
	soln_rec.qty_bord    = 0.00;
	soln_rec.gsale_price = qtln_rec.gsale_price;
	soln_rec.sale_price  = qtln_rec.sale_price;
	soln_rec.cost_price  = 0.00;
	soln_rec.dis_pc      = ScreenDisc (qtln_rec.disc_pc);
	soln_rec.reg_pc      = qtln_rec.reg_pc;
	soln_rec.disc_a      = qtln_rec.disc_a;
	soln_rec.disc_b      = qtln_rec.disc_b;
	soln_rec.disc_c      = qtln_rec.disc_c;
	soln_rec.cumulative  = qtln_rec.cumulative;
	soln_rec.tax_pc      = inmr_rec.tax_pc;
	soln_rec.gst_pc      = inmr_rec.gst_pc;
	soln_rec.due_date    = qtln_rec.exp_date;
	soln_rec.cont_status = qtln_rec.cont_status;
	/*
	 * WARNING NOTE !                                            
	 * The serial field input in so_input has been reduced to 20 
	 * characters due to screen width restraints. Consequently   
	 * we cannot directly copy the qtln serial field to the soln 
	 * serial field else we end up with unterminated strings in  
	 * so_input, and so we have truncated the serial no to 20    
	 * characters.                                               
	 */
	sprintf (soln_rec.serial_no, "%20.20s",    qtln_rec.serial_no);
	strcpy (soln_rec.pack_size,    inmr_rec.pack_size);
	strcpy (soln_rec.sman_code,    qthr_rec.sman_code);
	sprintf (soln_rec.cus_ord_ref, "%-20.20s", qthr_rec.enq_ref);
	if (!strcmp (qtln_rec.pri_or, "Y"))
		strcpy (soln_rec.pri_or, "Y");
	else
		strcpy (soln_rec.pri_or, "N");

	strcpy (soln_rec.dis_or,       "N");
	strcpy (soln_rec.item_desc,    qtln_rec.item_desc);
	strcpy (soln_rec.bonus_flag,   "N");
	strcpy (soln_rec.hide_flag,    "N");
	strcpy (soln_rec.status,       "M");
	strcpy (soln_rec.stat_flag,    "M");

	/*
	 * Commit serial item. 
	 */
	if (inmr_rec.costing_flag [0] == 'S' &&
	     inmr_rec.serial_item [0] == 'Y')
		strcpy (qtln_rec.serial_no, ser_space);
	/***
	{
		incc_rec.hhcc_hash = qtln_rec.hhcc_hash;
		incc_rec.hhbr_hash = qtln_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		if (CommitInsf (incc_rec.hhwh_hash,soln_rec.serial_no))
			strcpy (qtln_rec.serial_no, ser_space);
	}
	***/
	
	cc = abc_add (soln, &soln_rec);
	if (cc) 
		file_err (cc, "soln", "DBADD");

	add_hash (comm_rec.co_no, comm_rec.est_no, "RC", 0,
			 soln_rec.hhbr_hash, soln_rec.hhcc_hash, 0L, 0.00);

	return (EXIT_SUCCESS);
}

int
CheckSohr (
 long int	order_no)
{
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%08ld", order_no);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	return (cc);
}


/*
 * Routine to print Quotation Letter from paragraph codes entered          
 */
void
PrintLetter (void)
{
	char	filename [100];
	char	qt_number [9];

	char	parse_str [201];
	char	parse_str1 [201];

	int		i = 0;
	int		i_cnt;

	sprintf (qt_number, "%-8ld", qthr_rec.hhqt_hash);
	sprintf (filename, 
			"%s%s", 
			 (envVar.qtDirectory == (char *)0) ? "PRT/q." : envVar.qtDirectory, 
			clip (qt_number));

	if ((fout = fopen (filename, "w")) == 0) 
		sys_err ("Error in Letter Output During (FOPEN)", errno, PNAME);

	scn_set (2);
	CalcTotal (FALSE);

	for (i = 0;i < lcount [3];i++)
	{
		qtld_rec.hhlh_hash = _hhlh_hash [i];
		qtld_rec.line_no = 0;
		cc = find_rec (qtld, &qtld_rec, GTEQ, "r");
		while (!cc && qtld_rec.hhlh_hash == _hhlh_hash [i])
		{
			getval (0);
			CalExtend (0);
			sprintf (parse_str,
					"%*.*s%s", 
					envVar.qtLmargin, envVar.qtLmargin, " ",
				    clip (qtld_rec.desc));
						  
			Parse (parse_str);
			if (insideLine)
			{
				for (i_cnt = 1; i_cnt < lcount [2]; i_cnt++)
				{
					getval (i_cnt);
					if (strcmp (store [i_cnt].alternate, "Y"))
					{
						CalExtend (i_cnt);
						l_nett	= (l_total - l_disc); 
					}
					else
					{
						l_nett 	= 0.00; 
						local_rec.qty = 0.00;
						l_disc  = 0.00;
						l_cost 	= 0.00;
						l_tax	= 0.00;
						l_gst 	= 0.00;
					}
					sprintf (parse_str1,
							"%*.*s%s", 
							envVar.qtLmargin, envVar.qtLmargin, " ",
							clip (qtld_rec.desc));
					Parse (parse_str1);
				}
	
				insideLine = FALSE;
			}
			cc = find_rec (qtld, &qtld_rec, NEXT, "r");
		}
		fprintf (fout, " \n");
	}
	fclose (fout);
}

void
Parse (
 char *	wrk_prt)
{
	int		cmd;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = p_strsave (wrk_prt);
	
	partPrinted = TRUE;

	/*
	 *	look for caret command	
	 */
	cptr = strchr (wk_prt, '.');
	dptr = wk_prt;
	while (cptr)
	{
		partPrinted = FALSE;
		/*
		 * print line up to now 
		 */
		*cptr = (char) NULL;

		if (cptr != wk_prt)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", dptr);
		}

		/*
		 *	check if valid .command	
		 */
		cmd = ValidCmd (cptr + 1);
		if (cmd >= DB_ACRO)
		{
			if (cmd >= LN_ITEM && cmd <= LN_SERL)
				insideLine = TRUE;

			SubstitudeCommand (cmd);
			dptr = cptr + 8;
		}
		else
		{
			fprintf (fout, ".");
			partPrinted = TRUE;
			dptr = cptr + 1;
		}

		cptr = strchr (dptr,'.');
	}

	/*
	 *	print rest of line	
	 */
	if (partPrinted)
	{
		if (dptr)
			fprintf (fout, "%s\n", dptr);
		else
			fprintf (fout, "\n");
	}
	free (wk_prt);
}

/*
 * Validate .commands. 
 */
int
ValidCmd (
	char	*workString)
{
	int	i;

	/*
	 * Dot command is last character on line. 
	 */
	if (!strlen (workString))
		return (-1);

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (workString,dot_cmds [i],7))
			return (i);

	return (-1);
}

/*
 * Substitute valid .commands with actual data. 
 */
void
SubstitudeCommand (
 int	cmd)
{
	char *	pr_sptr;

	switch (cmd)
	{

	/*
	 * Company Name. 
	 */
	case	CO_NAME:
		pr_sptr = clip (comr_rec.co_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Company Address 1. 
	 */
	case	CO_ADR1:
		pr_sptr = clip (comr_rec.co_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Company Address 2. 
	 */
	case	CO_ADR2:
		pr_sptr = clip (comr_rec.co_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Company Address 3. 
	 */
	case	CO_ADR3:
		pr_sptr = clip (comr_rec.co_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Customer Acronym. 
	 */
	case	DB_ACRO:
		pr_sptr = clip (cumr_rec.dbt_acronym);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-9.9s", pr_sptr);
		}
		break;

	/*
	 * Customer Number. 
	 */
	case	DB_NUMB:
		pr_sptr = clip (cumr_rec.dbt_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-6.6s", pr_sptr);
		}
		break;

	/*
	 * Customer Name. 
	 */
	case	DB_NAME:
		pr_sptr = clip (cumr_rec.dbt_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Customer Address. 
	 */
	case	DB_ADR1:
		pr_sptr = clip (cumr_rec.ch_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Customer Address. 
	 */
	case	DB_ADR2:
		pr_sptr = clip (cumr_rec.ch_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Customer Address. 
	 */
	case	DB_ADR3:
		pr_sptr = clip (cumr_rec.ch_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Customer Contrct Name. 
	 */
	case	DB_CONT:
		pr_sptr = clip (cumr_rec.contact_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-20.20s", pr_sptr);
		}
		break;

	/*
	 * Salesman No. 
	 */
	case	SM_NUMB:
		pr_sptr = clip (qthr_rec.sman_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-2.2s", pr_sptr);
		}
		break;

	/*
	 * Salesman Name. 
	 */
	case	SM_NAME:
		pr_sptr = clip (exsf_rec.salesman);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Quote number. 
	 */
	case	QT_NUMB:
		partPrinted = TRUE;
		fprintf (fout, "%8ld", local_rec.n_quote_no);
		break;

	/*
	 * Quote Salutation. 
	 */
	case	QT_SALU:
		partPrinted = TRUE;
		fprintf (fout, "%s", qthr_rec.salute);
		break;

	/*
	 * Quote Order Number. 
	 */
	case	QT_ORDE:
		partPrinted = TRUE;
		fprintf (fout, "%s", qthr_rec.enq_ref);
		break;

	/*
	 * Quote Date. 
	 */
	case	QT_QDAT:
		partPrinted = TRUE;
		fprintf (fout,"%-10.10s", (qthr_rec.dt_quote == 0L) ? "          " 
											: DateToString (qthr_rec.dt_quote));
		break;

	/*
	 * Quote Expiry Date. 
	 */
	case	QT_EDAT:
		partPrinted = TRUE;
		fprintf (fout, "%10.10s", (qthr_rec.expire_date == 0L) 
						? "          " : DateToString (qthr_rec.expire_date));
		break;

	/*
	 * Quote Contract name. 
	 */
	case	QT_CNAM:
		pr_sptr = clip (qthr_rec.cont_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", pr_sptr);
		}
		break;

	/*
	 * Quote Comment 1. 
	 */
	case	QT_COM1:
		pr_sptr = clip (qthr_rec.comm1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", pr_sptr);
		}
		break;

	/*
	 * Quote Comment 2. 
	 */
	case	QT_COM2:
		pr_sptr = clip (qthr_rec.comm2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", pr_sptr);
		}
		break;

	/*
	 * Quote Comment 3. 
	 */
	case	QT_COM3:
		pr_sptr = clip (qthr_rec.comm3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", pr_sptr);
		}
		break;

	/*
	 * Item Number. 
	 */
		
	case	LN_ITEM:
		pr_sptr = clip (local_rec.item_no);
		cc = find_cuit (qtln_rec.hhbr_hash, 
					   cumr_rec.hhcu_hash, 
					   cumr_rec.item_codes);
		if (!cc)
			pr_sptr = clip (cuit_rec.item_no);
		
		if (*pr_sptr)
		{
			
			partPrinted = TRUE;
			fprintf (fout, "%-16.16s", pr_sptr);
		}
		break;

	/*
	 * Item Description. 
	 */
	case	LN_DESC:
		pr_sptr = clip (qtln_rec.item_desc);
		cc = find_cuit (qtln_rec.hhbr_hash, 
					   cumr_rec.hhcu_hash, 
					   cumr_rec.item_codes);
		if (!cc)
		{
			cc	=	FindInmr 
					 (
						comm_rec.co_no, 
						local_rec.item_no,
						cumr_rec.hhcu_hash, 
						cumr_rec.item_codes
					);
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.item_no);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (!cc)
			{
				if (strcmp (clip (inmr_rec.description), qtln_rec.item_desc) == 0)
				{
					pr_sptr = clip (cuit_rec.item_desc);
				}
			}
		}
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*
	 * Item Quantity. 
	 */
	case	LN_QUTY:
		PrintAmount (local_rec.qty, FALSE);
		break;

	/*
	 * Item price. 
	 */
	case	LN_GROS:
		PrintAmount (DOLLARS (l_total), TRUE);
		break;

	/*
	 * Item price Subtotal. 
	 */
	case	ST_NETT:
		if (strcmp (qtln_rec.alt_flag, "Y"))
			TotalNett = TotalNett + (l_nett); 

		if (qtln_rec.st_flag [0] == '*')
		{
			PrintAmount (DOLLARS (TotalNett), TRUE);
			TotalNett = 0.00;
		}
		break;

	/*
	 * Item discount. 
	 */
	case	LN_DIS1:
		PrintAmount (qtln_rec.disc_pc , FALSE);
		break;

	/*
	 * Item price. 
	 */
	case	LN_DIS2:
		PrintAmount (DOLLARS (l_disc), TRUE);
		break;

	/*
	 * Item extended. 
	 */
	case	LN_NETT:
		PrintAmount (DOLLARS (l_nett) ,TRUE);
		break;

	/*
	 * Item price Subtotal. 
	 */
	case	ST_GROS:
		if (strcmp (qtln_rec.alt_flag, "Y"))
			TotalGross = TotalGross + l_total; 

		if (qtln_rec.st_flag [0] == '*')
		{
			PrintAmount (DOLLARS (TotalGross), TRUE);
			TotalGross = 0.00;
		}
		break;

	/*
	 * Item serial No. 
	 */
	case	LN_SERL:
		pr_sptr = clip (qtln_rec.serial_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%-25.25s", pr_sptr);
		}
		break;

	/*
	 * Module Date, format dd/mm/yy. 
	 */
	case	MOD_DAT:
		partPrinted = TRUE;
		fprintf (fout, "%-10.10s", DateToString (comm_rec.dbt_date));
		break;

	/*
	 * System Date, format dd/mm/yy. 
 	 */
	case	CUR_DAT:
		partPrinted = TRUE;
		fprintf (fout, "%-10.10s", local_rec.systemDate);
		break;

	/*
	 * Customer Full system date. 
	 */
	case FUL_DAT:
		partPrinted = TRUE;
		fprintf (fout, "%s", DateToFmtString (comm_rec.dbt_date, "%e %B %Y", err_str));
		break;

	/*
	 * Total - Subtotal. 
	 */
	case	TOT_GRO: 			/* TOT_NET: */
		PrintAmount (DOLLARS (inv_tot), TRUE);
		break;

	/*
	 * Total - Discount. 
	 */
	case	TOT_DIS:
		PrintAmount (DOLLARS (dis_tot), TRUE);
		break;

	/*
	 * Total - Tax. 
	 */
	case	TOT_TAX:
		PrintAmount (DOLLARS (tax_tot + gst_tot), TRUE);
		break;

	/*
	 * Total - sale_price - discount. 
	 */
	case	TOT_NET:	       /* case	TOT_GRO: */
		PrintAmount (DOLLARS (inv_tot - dis_tot), TRUE);
		break;

	/*
	 * Total - sale_price - discount + tax. 
	 */
	case	TOT_ALL:
		PrintAmount (DOLLARS (tot_tot), TRUE);
		break;

	default:
		break;
	}
	fflush (fout);
}

/*
 * Print dollar and quantity amounts. 
 */
void
PrintAmount (
	double	amt,
	int		dollar_flag)
{
 	fprintf (fout,"%9.9s", comma_fmt ((double)amt, "NN,NNN.NN"));

	partPrinted = TRUE;
}
/*
 *	Commit insf record	
 */
int
CommitInsf (
 long int	_hhwh_hash,
 char *		ser_no)
{
	if (!strcmp (ser_no,ser_space))
		return (EXIT_SUCCESS);

	/*
	 * serial_item and serial number input	
	 */
	return (UpdateInsf (_hhwh_hash,  0L, ser_no, "F", "C"));
}


/*
 * Display Info for lines while in edit mode. 
 */
void
tab_other (
 int	iline)
{
	char		disp_str [200];
	static	int	orig_disc;
	static	int	orig_sale;
	static	int	orig_cost;
	static	int	orig_serl;
	static	int	first_time = TRUE;

	if (cur_screen == 2)
	{
		if (iline < oldLines) 
			FLD ("uom") = NE;
		else
			FLD ("uom") = YES;
				

		if (first_time)
		{
			orig_disc = FLD ("disc");
			orig_sale = FLD ("sale_price");
			orig_cost = FLD ("cost_price");
			orig_serl = FLD ("ser_no");
			first_time = FALSE;
		}

		sprintf (disp_str,ML (mlQtMess053),local_rec.quote_no, local_rec.weight);
		print_at (3,0,disp_str);

		/*
		 * turn off/on editing of fields depending on whether contract or not
		 */
		if (store [iline].contractStatus)
			FLD ("sale_price") = NA;
		else
			FLD ("sale_price") = orig_sale;

		if (store [iline].contractStatus == 1)
			FLD ("disc") = NA;
		else
			FLD ("disc") = orig_disc;

		if (store [iline].contractCost)
			FLD ("cost_price") = NA;
		else
		{
			if (inmr_rec.inmr_class [0] == 'Z')   /* add */
				FLD ("cost_price") = orig_cost;
		}

		if (store [iline].serialFlag [0] != 'Y')
		{
			if (orig_serl != ND)
				FLD ("ser_no") = NA;
		}
		else
			FLD ("ser_no") = orig_serl;
		return;
	}

	if (cur_screen != 3)
		return;

	if (_hhlh_hash [iline] == 0L && blankWin)
		return;

	crsr_off ();

	if (!blankWin)
		Dsp_close ();

	blankWin = TRUE;

	Dsp_open (22,1,16);
	Dsp_saverec ("                               P A R A G R A P H    D E S C R I P T I O N .                                 ");
	Dsp_saverec ("");
	Dsp_saverec ("");

	if (_hhlh_hash [iline] == 0L)
	{
		Dsp_srch ();
		Dsp_close ();
		blankWin = TRUE;
		return;
	}

	sprintf (err_str, "%s", ML ("Paragraph Description : "));

	sprintf (disp_str,"%s %-40.40s", err_str, _par_desc [iline]);
	Dsp_saverec (disp_str);

	qtld_rec.hhlh_hash = _hhlh_hash [iline];
	qtld_rec.line_no = 0;
	cc = find_rec (qtld, &qtld_rec, GTEQ, "r");
	while (!cc && qtld_rec.hhlh_hash == _hhlh_hash [iline])
	{
		sprintf (disp_str,"%-108.108s", qtld_rec.desc);
		Dsp_saverec (disp_str);
		cc = find_rec (qtld, &qtld_rec, NEXT, "r");
	}
	Dsp_srch ();
	if (prog_status == ENTRY)
		crsr_on ();

	blankWin = FALSE;

	return;
}

/*
 * Warn user about something. 
 */
int
WarnUser (
 char *	wn_mess,
 int	wn_flip)
{
	int		i;
	
	clear_mess ();
	print_mess (wn_mess);

	if (!wn_flip)
	{
		i = prmptmsg (ML (mlQtMess024),"YyNnMm",1,2);
		move (1,2);
		printf ("%-110.110s"," ");
		if (i == 'Y' || i == 'y') 
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML (mlQtMess025), "YyNn",1,2);
			heading (1);
			scn_display (1);
			move (1,2);
			printf ("%-110.110s"," ");
			if (i == 'Y' || i == 'y') 
				return (EXIT_SUCCESS);
		}
		return (EXIT_FAILURE);
	}

	if (wn_flip == 9)
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

int
LoadDisplay (
 char *	_run_string)
{
	char _co_no [3];
	char _br_no [3];
	char _quote_no [9];
	
	int		i;

	sprintf (_co_no, "%-2.2s", _run_string); 
	sprintf (_br_no, "%-2.2s", _run_string + 3);
	sprintf (_quote_no, "%-8.8s", _run_string + 6);  

	abc_selfield (qthr, "qthr_id_no2");
	strcpy (qthr_rec.co_no, _co_no);
	strcpy (qthr_rec.br_no, _br_no);
	strcpy (qthr_rec.quote_no, _quote_no);
	cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
	{
		abc_selfield (qthr, "qthr_id_no");
		return (EXIT_FAILURE);
	}
	if (qthr_rec.hhcu_hash == 0L)
		strcpy (cumr_rec.dbt_no, "99998 ");
	else
	{
		abc_selfield (cumr, "cumr_hhcu_hash");
		cumr_rec.hhcu_hash	=	qthr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		strcpy (qthr_rec.dbt_name, (cc) ? " " : cumr_rec.dbt_name);
	}

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, qthr_rec.cont_no);
	cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
	if (!cc)
		sprintf (local_rec.cont_desc,"%-20.20s", cnch_rec.desc);
	else
		strcpy (local_rec.cont_desc, "                    ");
			
	open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");

	strcpy (tmpf_rec.co_no, qthr_rec.co_no);
	strcpy (tmpf_rec.pos_code, qthr_rec.pos_code);
	cc = find_rec (tmpf, &tmpf_rec, EQUAL, "r");
	if (cc)
		sprintf (tmpf_rec.pos_desc,"%-20.20s", "                    ");

	abc_fclose (tmpf);

	for (i = 0;strlen (q_status [i]._stat);i++)
	{
		if (!strncmp (qthr_rec.status, q_status [i]._stat,strlen (q_status [i]._stat)))
		{
			sprintf (local_rec.stat_desc,"%-40.40s",q_status [i]._desc);
			break;
		}
	}

	for (i = 0;strlen (s_terms [i]._scode);i++)
	{
		if (!strncmp (qthr_rec.sell_terms,s_terms [i]._scode,strlen (s_terms [i]._scode)))
		{
			sprintf (local_rec.sell_desc,"%-30.30s",s_terms [i]._sterm);
			break;
		}
	}
	
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, qthr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman,"%-40.40s", " ");

	abc_fclose (exsf);

	open_rec (cfhr,cfhr_list,CFHR_NO_FIELDS,"cfhr_id_no");
	open_rec (cfln,cfln_list,CFLN_NO_FIELDS,"cfln_id_no");

	strcpy (cfhr_rec.co_no, comm_rec.co_no);
	strcpy (cfhr_rec.br_no, comm_rec.est_no);
	strcpy (cfhr_rec.carr_code, qthr_rec.carr_code);
	cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
	if (!cc)
	{
		cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
		strcpy (cfln_rec.area_code, qthr_rec.carr_area);
		find_rec (cfln,&cfln_rec,COMPARISON,"r");

		abc_fclose (cfhr);
		abc_fclose (cfln);

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,qthr_rec.carr_area);
		find_rec (exaf,&exaf_rec,COMPARISON,"r");
		strcpy (local_rec.carr_adesc, exaf_rec.area);
	}

	strcpy (local_rec.quote_no, qthr_rec.quote_no);
	local_rec.n_quote_no = qthr_rec.hhqt_hash;

	SetPriceDesc ();

	strcpy (local_rec.quote_no, _quote_no);

	local_rec.weight = qthr_rec.no_kgs;

	if (LoadQtln (qthr_rec.hhqt_hash))
	{
		abc_selfield (qthr, "qthr_id_no");
		return (EXIT_FAILURE);
	}

	abc_selfield (qthr, "qthr_id_no");
	return (EXIT_SUCCESS);
}

void
PrintCoStuff (void)
{
	move (0,22);
	print_at (22,0,ML (mlStdMess038),
			comm_rec.co_no,  comm_rec.co_name);
	print_at (22,40,ML (mlStdMess039),
			comm_rec.est_no, comm_rec.est_short);
	print_at (22,60,ML (mlStdMess099),
			comm_rec.cc_no, comm_rec.cc_short);

}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		swide ();

		if (!specialDisplay)
		{
			rv_pr (ML (mlQtMess028), (scn == 2) ? 20 : 48,0,1);
		}
		else
		{
			rv_pr (ML (mlQtMess006), (scn == 2) ? 20 : 48,0,1);
		}
			
		pr_box_lines (scn); 
		if (scn == 2)
			CalcTotal (TRUE);

		move (1,input_row);
		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search on Contract (cnch)     
 */
void
SrchCnch (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Contract code description");

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	sprintf (cnch_rec.cont_no, "%-6.6s", key_val);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no)
			   && !strncmp (cnch_rec.cont_no, key_val, strlen (key_val)))
	{                        
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (cnch_rec.cont_no, cnch_rec.desc);
		if (cc)
				break;
		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	sprintf (cnch_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cnch", "DBFIND");
}

/*
 * Search for Selling Price Types. 
 */
void
ShowPrice (void)
{
	_work_open (1,0,30);
	save_rec ("# ","#Price ");

	cc = save_rec ("1", comm_rec.price1_desc);
	cc = save_rec ("2", comm_rec.price2_desc);
	cc = save_rec ("3", comm_rec.price3_desc);
	cc = save_rec ("4", comm_rec.price4_desc);
	cc = save_rec ("5", comm_rec.price5_desc);
	cc = save_rec ("6", comm_rec.price6_desc);
	cc = save_rec ("7", comm_rec.price7_desc);
	cc = save_rec ("8", comm_rec.price8_desc);
	cc = save_rec ("9", comm_rec.price9_desc);
	cc = disp_srch ();
	work_close ();

}

/*
 * Search for Selling Terms. 
 */
void
ShowSell (void)
{
	int		i = 0;

	_work_open (3,0,40);
	save_rec ("#Cde","#Selling Terms ");

	for (i = 0;strlen (s_terms [i]._scode);i++)
	{
		cc = save_rec (s_terms [i]._scode,s_terms [i]._sterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search for Payment Terms. 
 */
void
ShowPay (void)
{
	int		i = 0;

	_work_open (3,0,40);
	save_rec ("#Cde","#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search for carrier code. 
 */
void
ShowCarr (
 char *	key_val)
{
	_work_open (4,0,50);
	save_rec ("#Carrier ","# Rate Kg. |               Carrier Name.            ");
	strcpy (cfhr_rec.co_no, comm_rec.co_no);
	strcpy (cfhr_rec.br_no, comm_rec.est_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s",key_val);
	cc = find_rec (cfhr,&cfhr_rec,GTEQ,"r");
	while (!cc && !strcmp (cfhr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (cfhr_rec.br_no,comm_rec.est_no) && 
		      !strncmp (cfhr_rec.carr_code,key_val,strlen (key_val)))
	{
		cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
		cc = find_rec (cfln,&cfln_rec,GTEQ,"r");
		while (!cc && cfln_rec.cfhh_hash == cfhr_rec.cfhh_hash)
		{
			strcpy (exaf_rec.co_no,comm_rec.co_no);
			strcpy (exaf_rec.area_code, cfln_rec.area_code);
			if (find_rec (exaf,&exaf_rec,COMPARISON, "r"))
			{
				cc = find_rec (cfln,&cfln_rec,NEXT,"r");
				continue;
			}
			
			sprintf (err_str, "%-2.2s %8.2f | %-40.40s", cfln_rec.area_code
													  , cfln_rec.cost_kg,
												        exaf_rec.area);

			cc = save_rec (cfhr_rec.carr_code, err_str);
			if (cc)
				break;

			cc = find_rec (cfln,&cfln_rec,NEXT,"r");
		}
		cc = find_rec (cfhr,&cfhr_rec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cfhr_rec.co_no,comm_rec.co_no);
	strcpy (cfhr_rec.br_no, comm_rec.est_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s",temp_str);
	cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "cfhr", "DBFIND");

	sprintf (local_rec.carr_area, "%-2.2s", temp_str + 5); 
}

/*
 * Search for area. 
 */
void
ShowArea (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Area description.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf,&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

int
SpecItem (void)
{
	if (last_char == DELLINE)
		return (DeleteItemLine ());

	if (last_char == INSLINE)
		return (InsertLine ()); 

	if (dflt_used || !strcmp (local_rec.item_no, "                "))
		return (DeleteItemLine ());

	if (prog_status == ENTRY)
		sprintf (local_rec.serial_no, "%25.25s", " ");
	
	SR.alternate [0] = (CheckAlternate (local_rec.item_no)) ? 'Y' : 'N';

	DSP_FLD ("item_no");
	DSP_FLD ("descr");
	local_rec.qty_avail = 0.00;
	SR.hhbrHash 		= 0L;
	SR.hhsiHash 		= 0L;
	SR.hhccHash 		= 0L;
	SR.outerSize     		= 0.00;
	SR.weight    		= 0.00;
	SR.defaultDisc 		= 0.00;
	SR.qtyAvailable 		= local_rec.qty_avail;

	SR.contractStatus 	= 0;
	SR.taxAmount 		= 0.00;
	SR.taxPc  		= 0.00;
	SR.gstPc  		= 0.00;
	SR.sale				= 0.00;
	SR.min_sell_pric 	= 0.00;
	SR.costPrice   	= 0.00;

	DSP_FLD ("cost_price");
	DSP_FLD ("sale_price");
	DSP_FLD ("disc");

	local_rec.qty_avail = SR.qtyAvailable;
	DSP_FLD ("qty_avail");

	tab_other (line_cnt);

	return (EXIT_SUCCESS);
}

/*
 * Reverse Screen Discount. 
 */
float	
ScreenDisc (
	float	DiscountPercent)
{
	if (envVar.reverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

float	
ToStdUom (
	float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("uom")))
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

	if (F_HIDE (label ("uom")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.cnvFct;

	return (cnvQty);
}

/*
 * Search on UOM (inum)     
 */
void
SrchInum (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#UOM ","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{                        
		if (strncmp (inum2_rec.uom_group, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom,inum2_rec.desc);
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

int
SrchCudi (
	int		indx)
{
	char	workString [170];

	_work_open (5,0,80);
	save_rec ("#DelNo","#Delivery Details");
	cudi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{                        
		sprintf 
		 (
			workString,"%s, %s, %s, %s",
			clip (cudi_rec.name),
			clip (cudi_rec.adr1),
			clip (cudi_rec.adr2),
			clip (cudi_rec.adr3)
		);
		sprintf (err_str, "%05d", cudi_rec.del_no);
		cc = save_rec (err_str, workString); 
		if (cc)
			break;

		cc = find_rec (cudi, &cudi_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (-1);

	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= atoi (temp_str);
	cc = find_rec (cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cudi, "DBFIND");

	switch (indx)
	{
	case	0:
		sprintf (temp_str,"%-40.40s",cudi_rec.name);
		break;

	case	1:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr1);
		break;

	case	2:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr2);
		break;

	case	3:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr3);
		break;

	default:
		break;
	}
	return (cudi_rec.del_no);
}

void
SetPriceDesc (void)
{
	if (qthr_rec.pri_type [0] == '1')
		strcpy (local_rec.pri_desc, comm_rec.price1_desc);
	else if (qthr_rec.pri_type [0] == '2')
		strcpy (local_rec.pri_desc, comm_rec.price2_desc);
	else if (qthr_rec.pri_type [0] == '3')
		strcpy (local_rec.pri_desc, comm_rec.price3_desc);
	else if (qthr_rec.pri_type [0] == '4')
		strcpy (local_rec.pri_desc, comm_rec.price4_desc);
	else if (qthr_rec.pri_type [0] == '5')
		strcpy (local_rec.pri_desc, comm_rec.price5_desc);
	else if (qthr_rec.pri_type [0] == '6')
		strcpy (local_rec.pri_desc, comm_rec.price6_desc);
	else if (qthr_rec.pri_type [0] == '7')
		strcpy (local_rec.pri_desc, comm_rec.price7_desc);
	else if (qthr_rec.pri_type [0] == '8')
		strcpy (local_rec.pri_desc, comm_rec.price8_desc);
	else if (qthr_rec.pri_type [0] == '9')
		strcpy (local_rec.pri_desc, comm_rec.price9_desc);
}

/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	/*
	 * Check and Get Credit terms. 
	 */
	sptr = get_env ("SO_CRD_TERMS");
	sptr = get_env ("SO_CRD_TERMS");
	envVar.creditStop 	= (* (sptr + 0) == 'S');
	envVar.creditTerms 	= (* (sptr + 1) == 'S');
	envVar.creditOver 	= (* (sptr + 2) == 'S');

	/*
	 * Check for Currency Code.  
	 */
	sprintf (envVar.currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Validate is serial items allowed. 
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envVar.serialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for discounts on Indent items. 
	 */
	sptr = chk_env ("SO_DIS_INDENT");
	envVar.discountIndents = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
     * Check and Get Order Date Type. 
     */
	sptr = chk_env ("SO_DOI");
	envVar.useSystemDate = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;

	/*
	 * Check if available stock is included. 
	 */
	sptr = chk_env ("SO_FWD_AVL");
	envVar.includeForwardStock = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check for defined left margin for quotations print. 
	 */
	sptr = chk_env ("QT_LMARGIN");
	envVar.qtLmargin = (sptr == (char *)0) ? 10 : atoi (sptr);

	/*
	 * Get directory for Quotations. 
	 */
	sptr = chk_env ("QT_DIR");
	if (sptr == (char *)0)
		strcpy (envVar.qtDirectory, "PRT/q.");
	else
		strcpy (envVar.qtDirectory, sptr);

	curr_user = getenv ("LOGNAME");

	/*
	 * Check if discount is input. 
	 */
	sptr = chk_env ("INP_DISC");
	envVar.inpDisc = (sptr == (char *)0) ? 0 : (*sptr == 'M' || *sptr == 'm');

	/*
	 * Customer Company Owned.  
	 */
	sptr = chk_env ("DB_CO");
	envVar.dbCo = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*
	 * Customer Find variable for Search. 
	 */
	sptr = chk_env ("DB_FIND");
	envVar.dbFind = (sptr == (char *) 0) ? 1 : atoi (sptr);

	/*
	 * Check if gst applies. 
	 */
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVar.gstApplies = 0;
	else
		envVar.gstApplies = (*sptr == 'Y' || *sptr == 'y');

	/*
	 * Get gst code. 
	 */
	if (envVar.gstApplies)
		sprintf (envVar.gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVar.gstCode, "%-3.3s", "Tax");
}

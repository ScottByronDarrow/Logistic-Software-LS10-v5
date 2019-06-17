/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_prom_pr.c,v 5.11 2002/12/01 04:48:17 scott Exp $
|  Program Name  : (sk_prom_pr.c) 
|  Program Desc  : (Add / Maintain Pricing Structure)
|                 (Originally so_primaint.c renamed 28/10/93)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 19/11/90         |
|---------------------------------------------------------------------|
| $Log: sk_prom_pr.c,v $
| Revision 5.11  2002/12/01 04:48:17  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.10  2002/11/28 04:09:49  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_prom_pr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_prom_pr/sk_prom_pr.c,v 5.11 2002/12/01 04:48:17 scott Exp $";

#define	TABLINES	5

#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<pslscr.h>
#include 	<minimenu.h>
#include 	<twodec.h>
#include 	<hot_keys.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	CUSTOMER	0
#define	CUST_TYPE	1
#define	ITEM_NO		2
#define	NEW_PRICING	3
#define	PROCESS		4
#define	CONT_TYPE	5
#define	CUST_AREA	6

#define	NORMAL		 (byWhat != NEW_PRICING)

#define	SEL_UPDATE		0
#define	SEL_IGNORE		1
#define	SEL_DELETE		2

#define	KITITEM		 (inmr_rec.inmr_class [0] == 'K')

#define	DESCITEM	 (inmr_rec.inmr_class [0] == 'Z')

#include	<cus_price.h>
#include	<cus_disc.h>

struct	tag_incpRecord	incp2_rec;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct cuitRecord	cuit_rec;
struct esmrRecord	esmr_rec;
struct inmrRecord	inmr_rec;
struct exclRecord	excl_rec;
struct exafRecord	exaf_rec;
struct pocrRecord	pocrRec;

	char	*data  = "data",
			*incp2 = "incp2",
			*incp3 = "incp3";

	extern	int		TruePosition;

	int		promoOverlap 	= TRUE,
			newCode 		= FALSE,
			byWhat			= 0,
			envDbFind		= 0,
			envDbCo			= 0,
			envDbMcurr		= 0,
			findIncpFlag	= 0,
			envSkDbPriNum	= 0,
			envSkCusPriLvl	= 0,
			screenWidth		= 0,
			tab_width		= 0,
			disp_scn2 		= FALSE,
			wk_no 			= 0;

	long	lsystemDate = 0L;

	float	percent = 0.00;

	char	envCurrCode 	[4],
			branchNumber 	[3],
			validStatus 	[2],
			systemDate 		[11],
			*curr_user;

	struct	storeRec {
			char	currCode [4];
	} store [MAXLINES];

	FILE	*pout;

	extern	int		stopDateSearch;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy 		[11];
	char	custLabel 	[17];
	char	custMask 	[7];
	char	custPrompt 	[17];
	char	custValue 	[7];
	char	custDesc 	[41];
	char	comment 	[41];
	char	brNo 		[3];
	char	brName 		[31];
	char	whNo 		[3];
	char	whName 		[31];
	long	dateFrom;
	long	dateTo;
	double	wkPrce;	
	char	pricePrompt [9][18];
	char	currCode 	[4];
	char	discAllow 	[2];
	double	exchRate;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, local_rec.custLabel, 4, 2, CHARTYPE,
		local_rec.custMask, "          ",
		" ", "", local_rec.custPrompt, "Enter Customer Type. Full Search Available.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.custValue},
	{1, LIN, "custDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.custDesc},
	{1, LIN, "itemNumber",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "        ",
		" ", "",   "Item Number.   ", "Enter Item Number. Full Search Available.",
		 NE, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{1, LIN, "itemDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "areaCode",	 6, 2, CHARTYPE,
		"UU", "        ",
		" ", "",   "Area Number    ", "Enter Area Number. Full Search Available.",
		 ND, NO,  JUSTLEFT, "", "", exaf_rec.area_code},
	{1, LIN, "areaDesc", 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 ND, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{1, LIN, "brNo",	6, 2, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Branch         ", "Enter Branch Number. Default For All Branches.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.brNo},
	{1, LIN, "brName",	6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.brName},
	{1, LIN, "whNo",	7, 2, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Warehouse      ", "Enter Warehouse Number. Default For All Warehouses",
		NE, NO,  JUSTRIGHT, "", "", local_rec.whNo},
	{1, LIN, "whName",	7, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.whName},
	{1, LIN, "currCode",	 6, 2, CHARTYPE,
		"UUU", "        ",
		" ", envCurrCode, "Currency.      ", "Enter Curency Code. Full Search Available, Default For Local Currency.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.currCode},
	{1, LIN, "dateFrom",	6, 53, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", systemDate, "Date From      ", " ",
		 NE, NO,  JUSTLEFT, "", "", (char *)&local_rec.dateFrom},
	{1, LIN, "dateTo",	7, 53, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", "00/00/00", "Date To        ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.dateTo},
	{1, LIN, "discAllow",	15, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Allow Disc.    ", "Y(es) to allow discounts,  N(o) for Fixed Price.",
		 ND, NO,  JUSTLEFT, "YN", "", local_rec.discAllow},
	{1, LIN, "wkPrce",	14, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Customer Price ", " ",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&local_rec.wkPrce},
	{1, LIN, "comment",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Comment        ", "Enter any related comments to special price.",
		YES, NO,  JUSTLEFT, "", "", local_rec.comment},
	{1, LIN, "price1",	12, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [0], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [0]},
	{1, LIN, "price2",	13, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [1], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [1]},
	{1, LIN, "price3",	14, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [2], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [2]},
	{1, LIN, "price4",	15, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [3], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [3]},
	{1, LIN, "price5",	16, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [4], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [4]},
	{1, LIN, "price6",	17, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [5], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [5]},
	{1, LIN, "price7",	18, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [6], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [6]},
	{1, LIN, "price8",	19, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [7], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [7]},
	{1, LIN, "price9",	20, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [8], "Return will default to current file price.",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [8]},

	{2, TAB, "l_currCode",	 MAXLINES, 0, CHARTYPE,
		"UUU", "        ",
		" ", " ", "Cur", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.currCode},
	{2, TAB, "tprce1",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [0], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [0]},
	{2, TAB, "tprce2",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [1], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [1]},
	{2, TAB, "tprce3",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [2], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [2]},
	{2, TAB, "tprce4",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [3], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [3]},
	{2, TAB, "tprce5",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [4], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [4]},
	{2, TAB, "tprce6",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [5], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [5]},
	{2, TAB, "tprce7",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [6], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [6]},
	{2, TAB, "tprce8",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [7], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [7]},
	{2, TAB, "tprce9",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.pricePrompt [8], "",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&incp_rec.price [8]},
	{2, TAB, "exchRate",	0, 0, MONEYTYPE,
		"NNNNNNNNNN", "          ",
		" ", "0", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.exchRate},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindCumr.h>

/*
 * Function Declarations 
 */
const 	char	*GetPriceDesc 		(int);
int  	CheckCurrCode 	 (char *);
int  	CheckDates 		 (long, long, long);
int  	CheckOverlap 	 (char *, char *, long, int, char *);
int  	DeleteLine 		 (void);
int  	ValidMcurrItem 	 (void);
int  	heading 		 (int);
int  	spec_valid 		 (int);
void 	CenterText 		 (char *, int);
void 	CloseDB 		 (void);
void 	DeleteOld 		 (void);
void 	OpenDB 			 (void);
void 	PosdtUpdate 	 (void);
void 	ProcessItems 	 (void);
void 	SrchCcmr 		 (char *);
void 	SrchEsmr 		 (char *);
void 	SrchExcl 		 (char *);
void 	SrchIncp 		 (char *);
void 	SrchIncp2 		 (void);
void 	SrchExaf 		 (char *);
void 	SrchPocr 		 (char *);
void 	Update 			 (void);
void 	UpdateMenu 		 (void);
void 	shutdown_prog 	 (void);

/*
 * Main Processing Routine 
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	int		i;
	char	*sptr;
	char	work [10];

	stopDateSearch	=	TRUE;
	TruePosition	=	TRUE;

	/*
	 * Setup local variables for environment variables 
	 */
	curr_user = getenv ("LOGNAME");

	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
	 * Check for POS installation.  
	 */
	sptr = chk_env ("SK_PROMO_OLAP");
	promoOverlap = (sptr == (char *)0) ? 1 : atoi (sptr);
	
	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);


	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind  	= atoi (get_env ("DB_FIND"));

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envSkDbPriNum = 5;
	else
	{
		envSkDbPriNum = atoi (sptr);
		if (envSkDbPriNum > 9 || envSkDbPriNum < 1)
			envSkDbPriNum = 9;
	}

	sptr = chk_env ("SK_CUSPRI_LVL");
	envSkCusPriLvl = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check parameters passed and reset screen layout accordingly.        
	 */

	if (argc < 2)
	{
		print_at (0,0, mlSkMess637, argv [0]);
		return (EXIT_FAILURE);
	}

	switch (argv [1][0])
	{
	/*
	 * Customer by item selection.
	 */
	case	'C':
	case	'c':
	case	'1':
		strcpy (local_rec.custLabel, "cust_no");
		sprintf (local_rec.custMask, "%-6.6s", "UUUUUU");
		strcpy (local_rec.custPrompt, "Customer No.   ");
		byWhat = CUSTOMER;
		strcpy (validStatus, "A");

		FLD ("currCode") = ND;
		FLD ("price1")    = ND;
		FLD ("price2")    = ND;
		FLD ("price3")    = ND;
		FLD ("price4")    = ND;
		FLD ("price5")    = ND;
		FLD ("price6")    = ND;
		FLD ("price7")    = ND;
		FLD ("price8")    = ND;
		FLD ("price9")    = ND;
		FLD ("wkPrce")    = YES;
		FLD ("discAllow") = YES;
		FLD ("areaCode")  = ND;
		FLD ("areaDesc")  = ND;
		if (envSkCusPriLvl < 2)
		{
			FLD ("whNo")   = ND;
			FLD ("whName") = ND;
		}
		if (envSkCusPriLvl == 0)
		{
			FLD ("brNo")   = ND;
			FLD ("brName") = ND;
			SCN_COL ("dateFrom")	= 2;
			SCN_COL ("dateTo")		= 2;
		}
		SCN_ROW ("brNo")		= 7;
		SCN_ROW ("brName")		= 7;
		SCN_ROW ("whNo")		= 8;
		SCN_ROW ("whName")		= 8;
		SCN_ROW ("dateFrom")	= 7;
		SCN_ROW ("dateTo")		= 8;
		SCN_ROW ("discAllow")	= 10;
		SCN_ROW ("comment")		= 12;
		SCN_ROW ("wkPrce")		= 11;

		break;

	/*
	 * Customer by type by item selection.
	 */
	case	'T':
	case	't':
	case	'2':
		strcpy (local_rec.custLabel, "cust_type");
		sprintf (local_rec.custMask, "%-3.3s", "UUU");
		strcpy (local_rec.custPrompt, "Customer Type  ");
		byWhat = CUST_TYPE;
		strcpy (validStatus, "A");
		if (!envDbMcurr)
			FLD ("currCode") = ND;
		for (i = 1; i <= 9; i++)
		{
			sprintf (work, "price%1d", i);
			if (i <= envSkDbPriNum)
				FLD (work) = YES;
			else
				FLD (work) = ND;
		}
		FLD ("wkPrce")    = ND;
		FLD ("discAllow") = YES;
		SCN_ROW ("dateFrom")	= 7;
		SCN_ROW ("dateTo")		= 8;
		SCN_ROW ("currCode")	= 9;
		if (envSkCusPriLvl == 1)
		{
			FLD ("whNo")   = ND;
			FLD ("whName") = ND;
			SCN_ROW ("currCode")	= 8;
		}
		if (envSkCusPriLvl == 0)
		{
			FLD ("brNo")   = ND;
			FLD ("brName") = ND;
			FLD ("whNo")   = ND;
			FLD ("whName") = ND;
			SCN_COL ("dateFrom")	= 2;
			SCN_COL ("dateTo")		= 2;
			SCN_ROW ("dateFrom")	= 8;
			SCN_ROW ("dateTo")		= 9;
			SCN_ROW ("currCode")	= 7;
		}
		FLD ("areaCode")  = ND;
		FLD ("areaDesc")  = ND;
		SCN_ROW ("brNo")		= 7;
		SCN_ROW ("brName")		= 7;
		SCN_ROW ("whNo")		= 8;
		SCN_ROW ("whName")		= 8;
		SCN_ROW ("discAllow")	= 9;
		SCN_COL ("discAllow")	= 53;
		SCN_ROW ("comment")		= 10;

		break;

	/*
	 * Customer by type selection.
	 */
	case	'I':
	case	'i':
	case	'3':
		strcpy (local_rec.custLabel, "cust_no");
		sprintf (local_rec.custMask, "%-6.6s", "UUUUUU");
		strcpy (local_rec.custPrompt, "Customer No.   ");
		byWhat = ITEM_NO;
		strcpy (validStatus, "A");
	
		FLD ("currCode") = ND;
		FLD ("cust_no")   = ND;
		FLD ("custDesc") = ND;
		for (i = 1; i <= 9; i++)
		{
			sprintf (work, "price%1d", i);
			if (i <= envSkDbPriNum && !envDbMcurr)
				FLD (work) = YES;
			else
				FLD (work) = ND;
		}
		FLD ("wkPrce")    = ND;
		FLD ("discAllow") = YES;
		FLD ("areaCode")  = ND;
		FLD ("areaDesc")  = ND;
		if (envSkCusPriLvl < 2)
		{
			FLD ("whNo")   = ND;
			FLD ("whName") = ND;
		}
		if (envSkCusPriLvl == 0)
		{
			FLD ("brNo")   = ND;
			FLD ("brName") = ND;
			SCN_COL ("dateFrom")	= 2;
			SCN_COL ("dateTo")		= 2;
		}
		SCN_ROW ("itemNumber")	= 4;
		SCN_ROW ("itemDesc")	= 4;
		SCN_ROW ("brNo")		= 6;
		SCN_ROW ("brName")		= 6;
		SCN_ROW ("whNo")		= 7;
		SCN_ROW ("whName")		= 7;
		SCN_ROW ("dateFrom")	= 6;
		SCN_ROW ("dateTo")		= 7;
		SCN_ROW ("discAllow")	= 8;
		SCN_ROW ("comment")		= 9;
		SCN_ROW ("price1")		= 11;
		SCN_ROW ("price2")		= 12;
		SCN_ROW ("price3")		= 13;
		SCN_ROW ("price4")		= 14;
		SCN_ROW ("price5")		= 15;
		SCN_ROW ("price6")		= 16;
		SCN_ROW ("price7")		= 17;
		SCN_ROW ("price8")		= 18;
		SCN_ROW ("price9")		= 19;

		for (i = envSkDbPriNum + 1; i <= 9; i++)
		{
			sprintf (work, "tprce%1d", i);
			FLD (work) = ND;
		}

		break;

	/*
	 * Customer by Customer by Item by Area selection.
	 */
	case	'A':
	case	'a':
	case	'4':
		strcpy (local_rec.custLabel, "cust_no");
		sprintf (local_rec.custMask, "%-6.6s", "UUUUUU");
		strcpy (local_rec.custPrompt, "Customer No.   ");
		byWhat = CUST_AREA;
		strcpy (validStatus, "A");

		FLD ("areaCode")  = NE;
		FLD ("areaDesc")  = NA;
		FLD ("currCode")  = ND;
		FLD ("price1")    = ND;
		FLD ("price2")    = ND;
		FLD ("price3")    = ND;
		FLD ("price4")    = ND;
		FLD ("price5")    = ND;
		FLD ("price6")    = ND;
		FLD ("price7")    = ND;
		FLD ("price8")    = ND;
		FLD ("price9")    = ND;
		FLD ("wkPrce")    = YES;
		FLD ("discAllow") = YES;
		if (envSkCusPriLvl < 2)
		{
			FLD ("whNo")   = ND;
			FLD ("whName") = ND;
		}
		if (envSkCusPriLvl == 0)
		{
			FLD ("brNo")   = ND;
			FLD ("brName") = ND;
			SCN_COL ("dateFrom")	= 2;
			SCN_COL ("dateTo")		= 2;
		}
		SCN_ROW ("dateFrom")	= 7;
		SCN_ROW ("dateTo")		= 8;
		SCN_ROW ("brNo")		= 8;
		SCN_ROW ("brName")		= 8;
		SCN_ROW ("whNo")		= 9;
		SCN_ROW ("whName")		= 9;
		SCN_ROW ("dateFrom")	= 8;
		SCN_ROW ("dateTo")		= 9;
		SCN_ROW ("discAllow")	= 11;
		SCN_ROW ("comment")		= 13;
		SCN_ROW ("wkPrce")		= 12;

		break;

	default:
		print_at (0,0, mlSkMess637, argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	if (byWhat == ITEM_NO)
	{
		swide ();
		screenWidth = 132;
	}
	else
		screenWidth = 80;

	tab_width = 5 + (envSkDbPriNum * 14);
	tab_row   = 12;
	tab_col   = (screenWidth - tab_width) / 2;

	OpenDB ();

	for (i = 0; i < 9; i++)
	{
		if (byWhat == ITEM_NO)
			sprintf (local_rec.pricePrompt [i]," %-12.12s",GetPriceDesc (i));
		else
			sprintf (local_rec.pricePrompt [i],"%-12.12s   ",GetPriceDesc (i));
	}

	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.est_no);

	/*
	 * Beginning of input control loop 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		abc_unlock (incp);
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;
		disp_scn2  = FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Choose currency. 
		 */
		if (byWhat == ITEM_NO && envDbMcurr && !findIncpFlag)
		{
			heading (2);
			entry (2);

			if (restart)
				continue;
		}

		/*
		 * Edit screen 1 linear input 
		 */
		disp_scn2 = TRUE;
		if (byWhat == ITEM_NO && envDbMcurr)
			edit_all ();
		else
		{
			heading (1);
			scn_display (1);
			edit (1);
		}

		if (restart)
			continue;

		if (byWhat == ITEM_NO && envDbMcurr)
			ProcessItems ();
		else
		{
			Update ();
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
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
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (incp2, incp);
	abc_alias (incp3, incp);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind) ? "cumr_id_no3" 
			 											 : "cumr_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (excl,  excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (incp,  incp_list, incp_no_fields, "incp_id_no");
	open_rec (incp2, incp_list, incp_no_fields, "incp_id_no");
	open_rec (incp3, incp_list, incp_no_fields, "incp_id_no2");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ingp,  ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec (inpr,  inpr_list, inpr_no_fields, "inpr_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");

	OpenPrice ();
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuit);
	abc_fclose (ccmr);
	abc_fclose (excl);
	abc_fclose (esmr);
	abc_fclose (incp);
	abc_fclose (incp2);
	abc_fclose (incp3);
	abc_fclose (ingp);
	abc_fclose (inpr);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (exaf);

	ClosePrice ();

	RF_CLOSE (wk_no);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i			=	0,
			chkLines	=	0,
			noConflict	=	0;

	long	hhcuHash 	= 	0L;

	char	custType [4],
			areaCode [3],
			wkFrom 	 [11],
			wkTo 	 [11],
			tmpCurr  [4],
			tmpKey   [7];

	/*
	 * Validate Customer Number.
	 */
	if (LCHECK ("cust_no"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Find Customer Master file details. 
		 */
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.custValue));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.custDesc, "%-40.40s", cumr_rec.dbt_name);
		display_field (field + 1);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Type. 
	 */
	if (LCHECK ("cust_type"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excl_rec.co_no, comm_rec.co_no);
		sprintf (excl_rec.class_type, "%-3.3s", local_rec.custValue);
		cc = find_rec (excl, &excl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess170));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.custDesc, excl_rec.class_desc);
		DSP_FLD ("custDesc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Customer Area. 
	 */
	if (LCHECK ("areaCode"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_FAILURE);

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
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("itemNumber")) 
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_FAILURE);

		if (last_char == FN16 && byWhat == ITEM_NO)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			if (byWhat == CUSTOMER)
			{
				InmrSearch 
			 	(
					comm_rec.co_no, 
					temp_str, 
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
			}
			else
			{
				InmrSearch 
			 	(
					comm_rec.co_no, 
					temp_str, 
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
			}
			return (EXIT_SUCCESS);
		}
	
		if (byWhat == CUSTOMER || byWhat == CUST_AREA)
		{
			cc	=	FindInmr 
		 			(
						comm_rec.co_no, 
						inmr_rec.item_no, 
						cumr_rec.hhcu_hash, 
						cumr_rec.item_codes
					);
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
		}
		else
		{
			cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (KITITEM)
		{
			print_mess (ML (mlSkMess315));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (DESCITEM)
		{
			print_mess (ML (mlSkMess316));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("currCode"))
	{
		if (FLD ("currCode") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!CheckCurrCode (local_rec.currCode))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Branch Number. 
	 */
	if (LCHECK ("brNo"))
	{
		if (envSkCusPriLvl == 0)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.brNo, "  ");
			strcpy (local_rec.brName, ML ("All Branches"));
			DSP_FLD ("brName");
			strcpy (local_rec.whNo, "  ");
			if (envSkCusPriLvl == 2)
			{
				FLD ("whNo") = NA;
				DSP_FLD ("whNo");
				strcpy (local_rec.whName, ML ("All Warehouses"));
				DSP_FLD ("whName");
			}
			return (EXIT_SUCCESS);
		}
		if (envSkCusPriLvl == 2)
			FLD ("whNo") = NE;

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%-2.2s", local_rec.brNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.brName, "%-30.30s", esmr_rec.est_name);
		DSP_FLD ("brName");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Warehouse Number. 
	 */
	if (LCHECK ("whNo"))
	{
		if (envSkCusPriLvl == 0 || envSkCusPriLvl == 1)
		{
			strcpy (local_rec.whNo, "  ");
			return (EXIT_SUCCESS);
		}

		if (FLD ("whNo") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.whNo, "  ");
			DSP_FLD ("whNo");
			strcpy (local_rec.whName, ML ("All Warehouses"));
			DSP_FLD ("whName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.brNo);
		sprintf (ccmr_rec.cc_no, "%-2.2s", local_rec.whNo);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.whName, "%-30.30s", ccmr_rec.name);
		DSP_FLD ("whName");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate date from. 
	 */
	if (LCHECK ("dateFrom"))
	{
		if (SRCH_KEY)
		{
			if (envDbMcurr && byWhat == ITEM_NO)
				SrchIncp2 ();
			else
				SrchIncp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && local_rec.dateTo < local_rec.dateFrom)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		newCode = FALSE;

		if (byWhat == ITEM_NO)
		{
			if (envDbMcurr)
				return (ValidMcurrItem ());
			else
			{
				hhcuHash = 0L;
				strcpy (custType, "   ");
				strcpy (areaCode, "  ");
				sprintf (tmpCurr, "%-3.3s", envCurrCode);
			}
		}

		if (byWhat == CUSTOMER || byWhat == CUST_AREA)
		{
			if (envDbMcurr)
				sprintf (tmpCurr, "%-3.3s", cumr_rec.curr_code);
			else
				sprintf (tmpCurr, "%-3.3s", envCurrCode);
			hhcuHash = cumr_rec.hhcu_hash;
			strcpy (custType, "   ");
			if (byWhat == CUST_AREA)
				strcpy (areaCode, exaf_rec.area_code);
			else
				strcpy (areaCode, "  ");
		}
	
		if (byWhat == CUST_TYPE)
		{
			strcpy (areaCode, "  ");
			if (envDbMcurr)
				sprintf (tmpCurr, "%-3.3s",local_rec.currCode);
			else
				sprintf (tmpCurr, "%-3.3s", envCurrCode);
			hhcuHash = 0L;
			sprintf (custType, "%-3.3s", local_rec.custValue);
		}
	

		sprintf (incp_rec.key, "%2.2s%2.2s%2.2s", 
							   comm_rec.co_no,
							   local_rec.brNo,
							   local_rec.whNo);
		sprintf (incp_rec.curr_code, "%-3.3s", tmpCurr);
		incp_rec.hhcu_hash = hhcuHash;
		incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (incp_rec.cus_type, custType);
		strcpy (incp_rec.area_code, areaCode);
		strcpy (incp_rec.status, validStatus);
		incp_rec.date_from = local_rec.dateFrom;

		cc = find_rec (incp, &incp_rec, EQUAL, "u");
		if (cc)
		{
			if (!CheckOverlap (custType, areaCode, hhcuHash, FALSE, tmpCurr) && !promoOverlap)
			{
				sprintf (wkFrom, "%-10.10s",DateToString (incp2_rec.date_from));
				sprintf (wkTo  , "%-10.10s",DateToString (incp2_rec.date_to));
				sprintf (err_str, ML (mlSkMess469), wkFrom, wkTo);

				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			newCode = TRUE;
			incp_rec.price [0] = 0.00;
			incp_rec.price [1] = 0.00;
			incp_rec.price [2] = 0.00;
			incp_rec.price [3] = 0.00;
			incp_rec.price [4] = 0.00;
			incp_rec.price [5] = 0.00;
			incp_rec.price [6] = 0.00;
			incp_rec.price [7] = 0.00;
			incp_rec.price [8] = 0.00;
		}
		else
		{
			if (byWhat == CUSTOMER || byWhat == CUST_AREA)
			{
				i = atoi (cumr_rec.price_type) - 1;
				local_rec.wkPrce = incp_rec.price [i];
			}
			entry_exit = 1;
			strcpy (local_rec.comment, incp_rec.comment);
			strcpy (local_rec.discAllow, incp_rec.dis_allow);
			local_rec.dateTo = incp_rec.date_to;
		}
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate date to. 
	 */
	if (LCHECK ("dateTo"))
	{
		if (!NORMAL)
			return (EXIT_SUCCESS);

		if (local_rec.dateTo < local_rec.dateFrom)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}


	/*
	 * Validate Prices. 
	 */
	if (LNCHECK ("price", 5) || LNCHECK ("tprce", 5) || LCHECK ("wkPrce"))
	{
		int		pr_type;
		char	work [20];
		double	dfltPrce;
		float 	wsRegPc;
		double	wsRegAmt;

		strcpy (work, FIELD.label);
		work [6] = '\0';
		if (FLD (work) == ND)
			return (EXIT_SUCCESS);

		if (strcmp (work, "wkPrce") == 0)
			pr_type = atoi (cumr_rec.price_type);
		else
			pr_type = atoi (FIELD.label + 5);

		if (prog_status == ENTRY &&
			last_char   == FN16 &&
			strncmp (work, "tprce", 5) == 0)
		{
			incp_rec.price [pr_type - 1] = 0.00;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (byWhat == ITEM_NO)
			{
				if (envDbMcurr)
					sprintf (tmpCurr, "%-3.3s",local_rec.currCode);
				else
					sprintf (tmpCurr, "%-3.3s", envCurrCode);
				strcpy (custType, "   ");
				strcpy (areaCode, "  ");
			}

			if (byWhat == CUSTOMER || byWhat == CUST_AREA)
			{
				if (envDbMcurr)
					sprintf (tmpCurr, "%-3.3s", cumr_rec.curr_code);
				else
					sprintf (tmpCurr, "%-3.3s", envCurrCode);
				hhcuHash = cumr_rec.hhcu_hash;
				strcpy (custType, cumr_rec.class_type);

				if (byWhat == CUST_AREA)
					strcpy (areaCode, exaf_rec.area_code);
				else
					strcpy (areaCode, "  ");
			}
			if (byWhat == CUST_TYPE)
			{
				strcpy (areaCode, "  ");
				if (envDbMcurr)
					sprintf (tmpCurr, "%-3.3s",local_rec.currCode);
				else
					sprintf (tmpCurr, "%-3.3s", envCurrCode);
				hhcuHash = 0L;
				sprintf (custType, "%-3.3s", local_rec.custValue);
			}

			/*
			 * Find ingp record to get regulatory percent. 
			 */
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.type, "S");
			sprintf (ingp_rec.code, "%-6.6s", inmr_rec.sellgrp);
			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			if (cc)
				wsRegPc = 0.00;
			else
				wsRegPc = ingp_rec.sell_reg_pc;

			dfltPrce	=	NormCusPrice 
							(
								local_rec.brNo, 
								local_rec.whNo, 
								areaCode, 
								custType, 
								inmr_rec.hhbr_hash, 
								tmpCurr, 
								(float) 1.00, 
								(int) pr_type
							);
			if (dfltPrce == (double) -1.00)
			{
				print_mess (ML (mlSkMess390));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				/*
				 * Deduct regulatory percent if non-zero. 
				 */
				if (dfltPrce != (double)-1.00 && wsRegPc != (float)0.00)
				{
					wsRegAmt = dfltPrce * (double)wsRegPc;
					wsRegAmt = wsRegAmt / (double) 100.00;
					wsRegAmt = twodec (wsRegAmt);

					dfltPrce -= wsRegAmt;
				}
				incp_rec.price [pr_type - 1] = dfltPrce;
			}
		}

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("l_currCode"))
	{
		if (FLD ("l_currCode") == ND)
			return (EXIT_SUCCESS);

		if (prog_status == ENTRY && last_char == FN16)
		{
			blank_display ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used || last_char == DELLINE)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!CheckCurrCode (local_rec.currCode))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Already in table 
		 */
		chkLines = (prog_status == ENTRY) ? line_cnt : lcount [2];
		for (i = 0; i < chkLines; i++)
		{
			if (!strcmp (store [i].currCode, local_rec.currCode)) 
			{
				/*
				 * if currency found on a current line then return as it 
				 * will not be on any other lines and also we do NOT want
				 * to check overlap
				 */
			    if (i == line_cnt)
					return (EXIT_SUCCESS);
				print_mess (ML (mlSkMess391));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*
		 * Check for overlap. 
		 */
		noConflict = TRUE;

		sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
					   comm_rec.co_no,
					   local_rec.brNo,
					   local_rec.whNo);
		strcpy (incp2_rec.key, tmpKey);
		sprintf (incp2_rec.curr_code, local_rec.currCode);
		incp2_rec.hhcu_hash = 0L;
		incp2_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (incp2_rec.cus_type, "   ");
		strcpy (incp2_rec.area_code, "  ");
		strcpy (incp2_rec.status, validStatus);
		incp2_rec.date_from = 0L;
	
		cc = find_rec (incp2, &incp2_rec, GTEQ, "r");
		while (!cc && 
		       !strcmp (incp2_rec.key, tmpKey) &&
		       !strcmp (incp2_rec.curr_code, local_rec.currCode) &&
		       incp2_rec.hhbr_hash == inmr_rec.hhbr_hash && 
		       incp2_rec.hhcu_hash == 0L &&
		       !strcmp (incp2_rec.cus_type, "   ") &&	
		       !strcmp (incp2_rec.area_code, "  ") &&	
		       incp2_rec.status [0] == validStatus [0])
		{
			/*
			 * Check for current record. 
			 */
			if (!newCode &&
			    incp2_rec.date_from == local_rec.dateFrom)
			{
				cc = find_rec (incp2, &incp2_rec, NEXT, "r");
				continue;
			}
	
			if (local_rec.dateFrom >= incp2_rec.date_from &&
			    local_rec.dateFrom <= incp2_rec.date_to)
			{
				noConflict = FALSE;
				break;
			}
			
			if (local_rec.dateTo >= incp2_rec.date_from &&
			    local_rec.dateTo <= incp2_rec.date_to)
			{
				noConflict = FALSE;
				break;
			}
	
			if (incp2_rec.date_from >= local_rec.dateFrom &&
				incp2_rec.date_from <= local_rec.dateTo)
			{
				noConflict = FALSE;
				break;
			}
	
			if (incp2_rec.date_to >= local_rec.dateFrom &&
				incp2_rec.date_to <= local_rec.dateTo)
			{
				noConflict = FALSE;
				break;
			}
	
			cc = find_rec (incp2, &incp2_rec, NEXT, "r");
		}

		if (noConflict == FALSE)
		{
			sprintf (wkFrom, "%-10.10s",	DateToString (incp2_rec.date_from));
			sprintf (wkTo  , "%-10.10s",	DateToString (incp2_rec.date_to));
			sprintf (err_str, ML (mlSkMess469), wkFrom, wkTo);

			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (store [line_cnt].currCode, "%-3.3s", local_rec.currCode);	
	
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Delete line from tabular screen. 
 */
int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		strcpy (store [line_cnt].currCode, store [line_cnt + 1].currCode);

		getval (line_cnt + 1);
		putval (line_cnt);
	}

	strcpy (local_rec.currCode, "   ");
	incp_rec.price [0] = 0.00;
	incp_rec.price [1] = 0.00;
	incp_rec.price [2] = 0.00;
	incp_rec.price [3] = 0.00;
	incp_rec.price [4] = 0.00;
	incp_rec.price [5] = 0.00;
	incp_rec.price [6] = 0.00;
	incp_rec.price [7] = 0.00;
	incp_rec.price [8] = 0.00;
	local_rec.exchRate  = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	scn_display (2);
	return (EXIT_SUCCESS);
}

/*
 * Check validity of data entered for Multi-Currency. 
 */
int
ValidMcurrItem (void)
{
	char		tmpKey [7];
	/*
	 * Load existing incp records into tabular screen.       
	 */
	findIncpFlag = FALSE;
	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
					   comm_rec.co_no,
					   local_rec.brNo,
					   local_rec.whNo);
	strcpy (incp_rec.key, tmpKey);
	strcpy (incp_rec.status, validStatus);
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = local_rec.dateFrom;
	strcpy (incp_rec.curr_code, "   ");
	cc = find_rec (incp3, &incp_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (incp_rec.key, tmpKey) &&
	       !strcmp (incp_rec.status, validStatus) &&
	       incp_rec.hhcu_hash == 0L &&
	       !strcmp (incp_rec.cus_type, "   ") &&
	       !strcmp (incp_rec.area_code, "  ") &&
	       incp_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       incp_rec.date_from == local_rec.dateFrom)
	{
		if (!findIncpFlag)
		{
			scn_set (2);
			lcount [2] = 0;
		}
	
		findIncpFlag = TRUE;

		strcpy (local_rec.comment, incp_rec.comment);
		local_rec.dateTo = incp_rec.date_to;
		strcpy (local_rec.discAllow, incp_rec.dis_allow);

		sprintf (local_rec.currCode, "%-3.3s", incp_rec.curr_code);
		sprintf (store [lcount [2]].currCode, "%-3.3s", incp_rec.curr_code);
		if (!CheckCurrCode (local_rec.currCode))
		{
			cc = find_rec (incp3, &incp_rec, NEXT, "r");
			continue;
		}

		putval (lcount [2]++);

		cc = find_rec (incp3, &incp_rec, NEXT, "r");
	}

	newCode = !findIncpFlag;

	if (findIncpFlag)
	{
		scn_display (2);
		scn_set (1);
		scn_display (1);
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}

	/*
	 * No records exist. Check for overlap. 
	 */
	strcpy (incp_rec.key, tmpKey);
	strcpy (incp_rec.status, validStatus);
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = 0L;
	strcpy (incp_rec.curr_code, "   ");
	cc = find_rec (incp3, &incp_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (incp_rec.key, tmpKey) &&
      	   incp_rec.status [0] == validStatus [0] &&
	       incp_rec.hhbr_hash == inmr_rec.hhbr_hash && 
      	   incp_rec.hhcu_hash == 0L &&
       	   !strcmp (incp_rec.area_code, "  ") &&
       	   !strcmp (incp_rec.cus_type, "   "))
	{
		if (local_rec.dateFrom >= incp_rec.date_from && 
		    local_rec.dateFrom <= incp_rec.date_to)
		{
			print_mess (ML (mlSkMess392));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		
		cc = find_rec (incp3, &incp_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*
 * Check validity of currency code. 
 */
int
CheckCurrCode (
	char	*chk_curr)
{
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", chk_curr);
	cc = find_rec (pocr, &pocrRec, EQUAL, "r");
	if (cc)
		return (FALSE);

	local_rec.exchRate = pocrRec.ex1_factor;

	return (TRUE);
}

/*
 * Check for overlaping records.
 */
int
CheckOverlap (
	char	*custType,
	char	*areaCode,
	long	hhcuHash,
	int		chk_type,
	char 	*chk_curr)
{
	char		tmpKey [7];
	
	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
					   comm_rec.co_no,
					   local_rec.brNo,
					   local_rec.whNo);
	strcpy (incp2_rec.key, tmpKey);
	sprintf (incp2_rec.curr_code, "%-3.3s", chk_curr);
	incp2_rec.hhcu_hash = hhcuHash;
	incp2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (incp2_rec.cus_type, custType);
	strcpy (incp2_rec.area_code, areaCode);
	strcpy (incp2_rec.status, validStatus);
	incp2_rec.date_from = 0L;
	cc = find_rec (incp2, &incp2_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (incp2_rec.key, tmpKey) &&
           !strcmp (incp2_rec.curr_code, chk_curr) &&
	       incp2_rec.hhbr_hash == inmr_rec.hhbr_hash && 
      	   incp2_rec.hhcu_hash == hhcuHash &&
       	   !strcmp (incp2_rec.cus_type, custType) &&	
       	   !strcmp (incp2_rec.area_code, areaCode) &&	
      	   incp2_rec.status [0] == validStatus [0])
	{
		if (!chk_type)
		{
			if (local_rec.dateFrom >= incp2_rec.date_from && 
			    local_rec.dateFrom <= incp2_rec.date_to)
				return (FALSE);
		}
		else
		{
			/*
			 * Check for current record. 
			 */
			if (local_rec.dateFrom == incp2_rec.date_from)
			{
				cc = find_rec (incp2, &incp2_rec, NEXT, "r");
				continue;
			}

			if (local_rec.dateTo >= incp2_rec.date_from && 
			    local_rec.dateTo <= incp2_rec.date_to)
				return (FALSE);
		}
		cc = find_rec (incp2, &incp2_rec, NEXT, "r");
	}

	return (TRUE);
}

int
CheckDates (
	long	hhbrHash, 
	long	dateFrom, 
	long	date_to)
{
	long	hhcuHash = 0;
	char	cust_type 	[4];
	char	areaCode 	[3];
	char	tmpCurr 	[4];
	char	tmpKey 		[7];

	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
					   comm_rec.co_no,
					   local_rec.brNo,
					   local_rec.whNo);

	if (byWhat == CUSTOMER || byWhat == CUST_AREA)
	{
		if (envDbMcurr)
			sprintf (tmpCurr, "%-3.3s", cumr_rec.curr_code);
		else
			sprintf (tmpCurr, "%-3.3s", envCurrCode);

		hhcuHash = cumr_rec.hhcu_hash;
		strcpy (cust_type, "   ");
		if (byWhat == CUST_AREA)
			strcpy (areaCode, exaf_rec.area_code);
		else
			strcpy (areaCode, "  ");
	}

	if (byWhat == CUST_TYPE)
	{
		if (envDbMcurr)
			sprintf (tmpCurr, "%-3.3s", local_rec.currCode);
		else
			sprintf (tmpCurr, "%-3.3s", envCurrCode);
		hhcuHash = 0L;
		sprintf (cust_type,"%-3.3s",local_rec.custValue);
	}

	if (byWhat == ITEM_NO)
	{
		if (envDbMcurr)
			sprintf (tmpCurr, "%-3.3s", local_rec.currCode);
		else
			sprintf (tmpCurr, "%-3.3s", envCurrCode);
		hhcuHash = 0L;
		strcpy (cust_type, "   ");
	}

	strcpy (incp2_rec.key, tmpKey);
	sprintf (incp2_rec.curr_code, "%-3.3s", tmpCurr);
	incp2_rec.hhcu_hash = hhcuHash;
	incp2_rec.hhbr_hash = hhbrHash;
	strcpy (incp2_rec.cus_type, cust_type);
	strcpy (incp2_rec.area_code, areaCode);
	strcpy (incp2_rec.status, validStatus);
	incp2_rec.date_from = 0L;

	cc = find_rec (incp2, &incp2_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (incp2_rec.key, tmpKey) &&
	       !strcmp (incp2_rec.curr_code, tmpCurr) &&
	       incp2_rec.hhbr_hash == hhbrHash && 
	       incp2_rec.hhcu_hash == hhcuHash &&
	       !strcmp (incp2_rec.cus_type, cust_type) &&	
	       !strcmp (incp2_rec.area_code, areaCode) &&	
	       incp2_rec.status [0] == validStatus [0])
	{
		/*
		 * Check for current record. 
		 */
		if (!newCode &&
		    incp2_rec.date_from == local_rec.dateFrom)
		{
			cc = find_rec (incp2, &incp2_rec, NEXT, "r");
			continue;
		}

		if (dateFrom >= incp2_rec.date_from &&
		    dateFrom <= incp2_rec.date_to)
			return (EXIT_FAILURE);
		
		if (date_to >= incp2_rec.date_from &&
		    date_to <= incp2_rec.date_to)
			return (EXIT_FAILURE);

		if (incp2_rec.date_from >= dateFrom &&
			incp2_rec.date_from <= date_to)
			return (EXIT_FAILURE);

		if (incp2_rec.date_to >= dateFrom &&
			incp2_rec.date_to <= date_to)
			return (EXIT_FAILURE);

		cc = find_rec (incp2, &incp2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Update Pricing structure. 
 */
void
Update (void)
{
	int	i;

	strcpy (incp_rec.stat_flag, "0");
	if (byWhat == CUSTOMER || byWhat == CUST_AREA)
	{
		incp_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (incp_rec.cus_type, "   ");
		if (byWhat == CUST_AREA)
			strcpy (incp_rec.area_code, exaf_rec.area_code);
		else
			strcpy (incp_rec.area_code, "  ");
		if (envDbMcurr)
			sprintf (incp_rec.curr_code, "%-3.3s", cumr_rec.curr_code);
		else
			sprintf (incp_rec.curr_code, "%-3.3s", envCurrCode);

		for (i = 0; i < 9; i++)
		{
			if (i < envSkDbPriNum)
				incp_rec.price [i] = local_rec.wkPrce;
			else
				incp_rec.price [i] = 0.00;
		}
	}

	if (byWhat == CUST_TYPE)
	{
		incp_rec.hhcu_hash = 0L;
		sprintf (incp_rec.cus_type, "%-3.3s", local_rec.custValue);
		if (envDbMcurr)
			sprintf (incp_rec.curr_code, "%-3.3s", local_rec.currCode);
		else
			sprintf (incp_rec.curr_code, "%-3.3s", envCurrCode);
	}

	if (byWhat == ITEM_NO)
	{
		incp_rec.hhcu_hash = 0L;
		strcpy (incp_rec.cus_type, "   ");
		sprintf (incp_rec.curr_code, "%-3.3s", envCurrCode);
	}

	sprintf (incp_rec.key, "%2.2s%2.2s%2.2s", 
							comm_rec.co_no,
							local_rec.brNo,
							local_rec.whNo);
	strcpy (incp_rec.status, validStatus);
	strcpy (incp_rec.dis_allow, local_rec.discAllow);
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = local_rec.dateFrom;
	incp_rec.date_to = local_rec.dateTo;
	sprintf (incp_rec.comment, "%-40.40s", local_rec.comment);

	incp_rec.price [0] = no_dec (incp_rec.price [0]);
	incp_rec.price [1] = no_dec (incp_rec.price [1]);
	incp_rec.price [2] = no_dec (incp_rec.price [2]);
	incp_rec.price [3] = no_dec (incp_rec.price [3]);
	incp_rec.price [4] = no_dec (incp_rec.price [4]);
	incp_rec.price [5] = no_dec (incp_rec.price [5]);
	incp_rec.price [6] = no_dec (incp_rec.price [6]);
	incp_rec.price [7] = no_dec (incp_rec.price [7]);
	incp_rec.price [8] = no_dec (incp_rec.price [8]);

	if (newCode)
	{
		cc = abc_add (incp, &incp_rec);
		if (cc)
			file_err (cc, incp, "DBFIND");
	}
	else
		UpdateMenu ();

	return;
}

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD  ",
	  " UPDATE PRICING STRUCTURE FILE WITH CHANGES MAKE. " },
	{ " 2. IGNORE CHANGES ",
	  " IGNORE CHANGES JUST MADE TO PRICING STRUCTURE FILE." },
	{ " 3. DELETE RECORD  ",
	  " DELETE PRICING STRUCTURE RECORD." },
	{ ENDMENU }
};

/*
 * Update mini menu. 
 */
void
UpdateMenu (void)
{
	int	ExitLoop = FALSE;

	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case SEL_UPDATE :
		case 99     :
			cc = abc_update (incp, &incp_rec);
			if (cc)
				file_err (cc, incp, "DBUPDATE");
			ExitLoop = TRUE;
			break;

		case SEL_IGNORE :
		case -1     :
			abc_unlock (incp);
			ExitLoop = TRUE;
			break;

		case SEL_DELETE :
			strcpy (incp_rec.comment, "*** RECORD DELETED ***");
			incp_rec.date_from = lsystemDate;
			incp_rec.date_to   = lsystemDate;
			incp_rec.price [0] = 0.0;
			incp_rec.price [1] = 0.0;
			incp_rec.price [2] = 0.0;
			incp_rec.price [3] = 0.0;
			incp_rec.price [4] = 0.0;
			incp_rec.price [5] = 0.0;
			incp_rec.price [6] = 0.0;
			incp_rec.price [7] = 0.0;
			incp_rec.price [8] = 0.0;
			strcpy (incp_rec.dis_allow, " ");

			cc = abc_delete (incp);
			if (cc)
				file_err (cc, incp, "DBDELETE");
			ExitLoop = TRUE;
			break;
	
		default :
			cc = abc_update (incp, &incp_rec);
			if (cc)
				file_err (cc, incp, "DBUPDATE");
			ExitLoop = TRUE;
			break;
	    }
		if (ExitLoop == TRUE)
			break;
	}
}

/*
 * Search for pocr record. 
 */
void
SrchPocr (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No.","#Current Code Description.");

	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", keyValue);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pocrRec.co_no, comm_rec.co_no) && 
	       !strncmp (pocrRec.code, keyValue, strlen (keyValue)))
	{
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

/*
 * Search for incp record. 
 */
void
SrchIncp (
	char	*keyValue)
{
	char	dateStringOne [11];
	char	tmpKey [7];
	long	hhcuHash = 0L;
	char	tmpCurr [4];
	char	cusType [4];
	char	areaCode [3];

	_work_open (10,0,50);
	save_rec ("#From Date ", "# To Date   | Comments");

	if (byWhat == CUSTOMER || byWhat == CUST_AREA)
	{
		hhcuHash = cumr_rec.hhcu_hash;
		if (byWhat == CUST_AREA)
			strcpy (areaCode, exaf_rec.area_code);
		else
			strcpy (areaCode, "  ");
		
		strcpy (cusType, "   ");
		if (envDbMcurr)
			sprintf (tmpCurr, "%-3.3s", cumr_rec.curr_code);
		else
			sprintf (tmpCurr, "%-3.3s", envCurrCode);
	}

	if (byWhat == CUST_TYPE)
	{
		hhcuHash = 0L;
		strcpy (areaCode, "  ");
		sprintf (cusType, "%-3.3s", local_rec.custValue);
		if (envDbMcurr)
			sprintf (tmpCurr, "%-3.3s", local_rec.currCode);
		else
			sprintf (tmpCurr, "%-3.3s", envCurrCode);
	}

	if (byWhat == ITEM_NO)
	{
		hhcuHash = 0L;
		strcpy (cusType, "   ");
		strcpy (areaCode, "  ");
		sprintf (tmpCurr, "%-3.3s", envCurrCode);
	}

	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
						comm_rec.co_no,
						local_rec.brNo,
						local_rec.whNo);
	strcpy (incp_rec.key, tmpKey);
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.hhcu_hash = hhcuHash;
	strcpy (incp_rec.status, validStatus);
	strcpy (incp_rec.curr_code, tmpCurr);
	strcpy (incp_rec.area_code, areaCode);
	strcpy (incp_rec.cus_type, cusType);
	incp_rec.date_from = atol (keyValue);
	if (incp_rec.date_from == 0L)
		incp_rec.date_from = 1L;

	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (incp_rec.key, tmpKey) &&
	       !strcmp (incp_rec.curr_code, tmpCurr) &&
	       incp_rec.hhbr_hash == inmr_rec.hhbr_hash && 
	       incp_rec.hhcu_hash == hhcuHash &&
	       !strcmp (incp_rec.area_code, areaCode) &&	
	       !strcmp (incp_rec.cus_type, cusType) &&	
	       incp_rec.status [0] == validStatus [0])
	{
		sprintf (err_str, "%s | %s", DateToString (incp_rec.date_to), 
									  incp_rec.comment);
		strcpy (dateStringOne, DateToString (incp_rec.date_from));
		cc = save_rec (dateStringOne, err_str);
		if (cc)
			break;

		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (incp_rec.key, tmpKey);
	sprintf (incp_rec.curr_code, "%-3.3s", tmpCurr);
	incp_rec.hhcu_hash = hhcuHash;
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (incp_rec.area_code, areaCode);
	strcpy (incp_rec.cus_type, cusType);
	strcpy (incp_rec.status, validStatus);
	incp_rec.date_from = StringToDate (temp_str);

	cc = find_rec (incp, &incp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, incp, "DBFIND");
}

/*
 * Search for incp record using id_no2 for Multi-Currency ITEM_NO option. 
 */
void
SrchIncp2 (void)
{
	char	wsCurrCode [4];
	char	tmpKey [7];
	long	wsDateFrom;
	char	dateStringOne [11];

	_work_open (10,0,50);
	save_rec ("#Eff Date", "#Comments");

	wsDateFrom = 0L;
	strcpy (wsCurrCode, "   ");
	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
						comm_rec.co_no,
						local_rec.brNo,
						local_rec.whNo);
	strcpy (incp_rec.key, tmpKey);
	strcpy (incp_rec.status, validStatus);
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = 0L;
	sprintf (incp_rec.curr_code, "%-3.3s", " ");
	cc = find_rec (incp3, &incp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (incp_rec.key, tmpKey) &&
	       incp_rec.status [0] == validStatus [0] &&
	       incp_rec.hhcu_hash == 0L &&
	       !strcmp (incp_rec.cus_type, "   ") &&	
	       !strcmp (incp_rec.area_code, "  ") &&	
	       incp_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (incp_rec.date_from != wsDateFrom)
		{
			strcpy (wsCurrCode, incp_rec.curr_code);
			wsDateFrom = incp_rec.date_from;
			strcpy (dateStringOne, DateToString (incp_rec.date_from));
			cc = save_rec (dateStringOne, incp_rec.comment);
			if (cc)
				break;
		}

		cc = find_rec (incp3, &incp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	return;
}

/*
 * Delete any incp records that overlap current 
 */
void
DeleteOld (void)
{
	long 	tmp_date;
	char	tmpCurr [4];
	char	tmpKey [7];

	if (envDbMcurr)
		sprintf (tmpCurr, "%-3.3s", cumr_rec.curr_code);
	else
		sprintf (tmpCurr, "%-3.3s", envCurrCode);

	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
						comm_rec.co_no,
						local_rec.brNo,
						local_rec.whNo);
	strcpy (incp_rec.key, tmpKey);
	sprintf (incp_rec.curr_code, "%-3.3s", tmpCurr);
	incp_rec.hhcu_hash = cumr_rec.hhcu_hash;
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	strcpy (incp_rec.status, validStatus);
	incp_rec.date_from = 0L;

	cc = find_rec (incp, &incp_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (incp_rec.key, tmpKey) &&
	       !strcmp (incp_rec.curr_code, tmpCurr) &&
	       incp_rec.status [0] == validStatus [0] &&
	       incp_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		tmp_date = incp_rec.date_from;

		if (incp_rec.hhbr_hash != inmr_rec.hhbr_hash || 
	  	   (local_rec.dateTo <= incp_rec.date_from) ||
	  	   (local_rec.dateFrom >= incp_rec.date_to))
		{
			abc_unlock (incp);
			cc = find_rec (incp, &incp_rec, NEXT, "u");
			continue;
		}

		cc = abc_delete (incp);
		if (cc)
			file_err (cc, incp, "DBDELETE");

		strcpy (incp_rec.key, tmpKey);
		sprintf (incp_rec.curr_code, "%-3.3s", tmpCurr);
		incp_rec.hhcu_hash = cumr_rec.hhcu_hash;
		incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (incp_rec.cus_type, "   ");
		strcpy (incp_rec.area_code, "  ");
		strcpy (incp_rec.status, validStatus);
		incp_rec.date_from = tmp_date;
	
		cc = find_rec (incp, &incp_rec, GTEQ, "u");
	}

	return;
}

/*
 * Multi-currency update for 'ITEM_NO' option. 
 */
void
ProcessItems (void)
{
	int		i;
	int		rec_ok;
	char	tmpKey [7];
	char	tmpCurr [4];
	char	tmpAllow [2];
	char	tmp_comments [41];
	long	date_to;

	sprintf (tmpKey, "%2.2s%2.2s%2.2s", 
						comm_rec.co_no,
						local_rec.brNo,
						local_rec.whNo);
	sprintf (tmp_comments, "%-40.40s", local_rec.comment);
	strcpy (tmpAllow, local_rec.discAllow);
	date_to = local_rec.dateTo;

	/*
	 * Delete any records that exist on incp but 
	 * don't have a line in the tabular screen.  
	 */
	strcpy (incp_rec.key, tmpKey);
	strcpy (incp_rec.status, validStatus);
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incp_rec.date_from = local_rec.dateFrom;
	strcpy (incp_rec.curr_code, "   ");
	cc = find_rec (incp3, &incp_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (incp_rec.key, tmpKey) &&
	       incp_rec.status [0] == validStatus [0] &&
	       incp_rec.hhcu_hash == 0L &&
	       !strcmp (incp_rec.cus_type, "   ") &&
	       !strcmp (incp_rec.area_code, "  ") &&
	       incp_rec.hhbr_hash == inmr_rec.hhbr_hash && 
	       incp_rec.date_from == local_rec.dateFrom)
	{
		/*
		 * Check that record exists in tablular screen. 
		 */
		rec_ok = FALSE;
		for (i = 0; i < lcount [2]; i++)
		{
			if (!strcmp (store [i].currCode, incp_rec.curr_code))
			{
				rec_ok = TRUE;
				break;
			}
		}

		if (!rec_ok)
		{
			/*
			 * Delete record. 
			 */
			strcpy (tmpCurr, incp_rec.curr_code);
			cc = abc_delete (incp3);
			if (cc)
				file_err (cc, incp3, "DBDELETE");

			/*
			 * Read next record. 
			 */
			strcpy (incp_rec.key, tmpKey);
			strcpy (incp_rec.status, validStatus);
			incp_rec.hhcu_hash = 0L;
			strcpy (incp_rec.cus_type, "   ");
			strcpy (incp_rec.area_code, "  ");
			incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
			incp_rec.date_from = local_rec.dateFrom;
			strcpy (incp_rec.curr_code, tmpCurr);

			cc = find_rec (incp3, &incp_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (incp3);
			cc = find_rec (incp3, &incp_rec, NEXT, "u");
		}
	}

	/*
	 * Add / Update records in tabular screen. 
	 */
	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		strcpy (incp_rec.key, tmpKey);
		strcpy (incp_rec.status, validStatus);
		incp_rec.hhcu_hash = 0L;
		strcpy (incp_rec.cus_type, "   ");
		strcpy (incp_rec.area_code, "  ");
		incp_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incp_rec.date_from = local_rec.dateFrom;
		strcpy (incp_rec.curr_code, store [i].currCode);
		cc = find_rec (incp3, &incp_rec, EQUAL, "u");
		sprintf (incp_rec.comment, "%-40.40s", local_rec.comment);
		if (cc)
		{
			getval (i);
			incp_rec.price [0] = no_dec (incp_rec.price [0]);
			incp_rec.price [1] = no_dec (incp_rec.price [1]);
			incp_rec.price [2] = no_dec (incp_rec.price [2]);
			incp_rec.price [3] = no_dec (incp_rec.price [3]);
			incp_rec.price [4] = no_dec (incp_rec.price [4]);
			incp_rec.price [5] = no_dec (incp_rec.price [5]);
			incp_rec.price [6] = no_dec (incp_rec.price [6]);
			incp_rec.price [7] = no_dec (incp_rec.price [7]);
			incp_rec.price [8] = no_dec (incp_rec.price [8]);
			strcpy (incp_rec.stat_flag, "0");
			strcpy (incp_rec.comment, tmp_comments);
			strcpy (incp_rec.dis_allow, tmpAllow);
			incp_rec.date_to = date_to;

			cc = abc_add (incp3, &incp_rec);
			if (cc)
				file_err (cc, incp3, "DBADD");
		}
		else
		{
			getval (i);
			strcpy (incp_rec.comment, tmp_comments);
			incp_rec.price [0] = no_dec (incp_rec.price [0]);
			incp_rec.price [1] = no_dec (incp_rec.price [1]);
			incp_rec.price [2] = no_dec (incp_rec.price [2]);
			incp_rec.price [3] = no_dec (incp_rec.price [3]);
			incp_rec.price [4] = no_dec (incp_rec.price [4]);
			incp_rec.price [5] = no_dec (incp_rec.price [5]);
			incp_rec.price [6] = no_dec (incp_rec.price [6]);
			incp_rec.price [7] = no_dec (incp_rec.price [7]);
			incp_rec.price [8] = no_dec (incp_rec.price [8]);
			incp_rec.date_to = date_to;
			strcpy (incp_rec.dis_allow, tmpAllow);
			cc = abc_update (incp3, &incp_rec);
			if (cc)
				file_err (cc, incp3, "DBUPDATE");
		}
	}
	
	return;
}

void
SrchEsmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Branch Description");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", keyValue);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
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
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Warehouse Description ");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.brNo);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		   !strcmp	 (ccmr_rec.est_no, local_rec.brNo) &&
	       !strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.brNo);
	sprintf (ccmr_rec.cc_no, "%-2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
SrchExcl (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Customer Type Description");
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", keyValue);
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (excl_rec.class_type, keyValue, strlen (keyValue)) &&
		   !strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", temp_str);
	cc = find_rec (excl, &excl_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
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

void
CenterText (
	char	*ws_buffer, 
	int 	ws_lin)
{
	int		ws_col;
	int		ws_len;

	strcpy (err_str, ws_buffer);
	ws_len = strlen (err_str);
	ws_col = (int) ((screenWidth - ws_len) / 2);
	rv_pr (err_str, ws_col, ws_lin, 1);
}

/*
 * Routine to get price desctiptions from comm record. 
 */
const char	*
GetPriceDesc (
	int		priceNo)
{
	static	char	priceDesc [16];

	strcpy (priceDesc, " ");

	switch (priceNo)
	{
		case	0:	
			strcpy (priceDesc,	comm_rec.price1_desc);
			break;
		case	1:	
			strcpy (priceDesc, comm_rec.price2_desc);
			break;
		case	2:	
			strcpy (priceDesc, comm_rec.price3_desc);
			break;
		case	3:	
			strcpy (priceDesc, comm_rec.price4_desc);
			break;
		case	4:	
			strcpy (priceDesc, comm_rec.price5_desc);
			break;
		case	5:	
			strcpy (priceDesc, comm_rec.price6_desc);
			break;
		case	6:	
			strcpy (priceDesc, comm_rec.price7_desc);
			break;
		case	7:	
			strcpy (priceDesc, comm_rec.price8_desc);
			break;
		case	8:	
			strcpy (priceDesc, comm_rec.price9_desc);
			break;
	}
	return (priceDesc);
}
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		CenterText (ML (mlSkMess393), 0);

		line_at (1,0,screenWidth);

		switch (byWhat)
		{
		case	CUSTOMER:
			CenterText (ML (mlSkMess394), 2);
			break;

		case	CUST_AREA:
			CenterText (ML (mlSkMess722), 2);
			break;

		case	CUST_TYPE:
			CenterText (ML (mlSkMess395), 2);
			break;

		case	ITEM_NO:
			CenterText (ML (mlSkMess396), 2);
			break;

		default:
			CenterText (ML (mlSkMess397), 2);
			break;

		}

		if (byWhat == CUSTOMER)
		{
			box (0, 3, screenWidth, 9);

			line_at (6,1, screenWidth - 1);
			line_at (9,1, screenWidth - 1);
			line_at (21,0, screenWidth);
		}
		if (byWhat == CUST_AREA)
		{
			box (0, 3, screenWidth, 10);

			line_at (7,1, screenWidth - 1);
			line_at (10,1, screenWidth - 1);
			line_at (21,0, screenWidth);
		}
		if (byWhat == CUST_TYPE)
		{
			box (0, 3, screenWidth, 8 + envSkDbPriNum);
			line_at (6,1, screenWidth - 1);
			line_at (11,1, screenWidth - 1);
			line_at (21,0, screenWidth);
		}
		if (byWhat == ITEM_NO)
		{
			if (envDbMcurr)
			{
				box (0, 3, screenWidth, 6);

				if (scn == 1)
				{
					scn_set (2);
					scn_write (2);
					if (disp_scn2)
						scn_display (2);
					scn_set (1);
				}
				else
				{
					scn_set (1);
					scn_write (1);
					scn_display (1);
					scn_set (2);
				}
			}
			else
			{
				box (0, 3, screenWidth, 7 + envSkDbPriNum);
				line_at (10,1, screenWidth - 1);
			}

			line_at (5,1, screenWidth - 1);
			line_at (21,0, screenWidth);
		}
		print_at (22,0, ML (mlStdMess038) ,comm_rec.co_no,comm_rec.co_name);


		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


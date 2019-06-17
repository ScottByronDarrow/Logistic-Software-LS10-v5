/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _display.c,v 5.6 2002/11/25 03:16:35 scott Exp $
|  Program Name  : (lrp_display.c)
|  Program Desc  : (Display Reorder Review)
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _display.c,v $
| Revision 5.6  2002/11/25 03:16:35  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.5  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.4  2001/08/28 10:11:57  robert
| additional update for LS10.5-GUI
|
| Revision 5.3  2001/08/09 09:29:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:24  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:10  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _display.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_display/_display.c,v 5.6 2002/11/25 03:16:35 scott Exp $";

#define		X_OFF	lp_x_off
#define		Y_OFF	lp_y_off
#include	<pslscr.h>
#include	<ring_menu.h>
#include	<ml_lrp_mess.h>
#include	<ml_std_mess.h>
#include	<getnum.h>
#include	<graph.h>
#include	<twodec.h>
#include	<Costing.h>
#define		MAX_SUPP	99
#define		MAX_WH		99

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ffprRecord	ffpr_rec;
struct inccRecord	incc_rec;
struct inisRecord	inis_rec;
struct inwsRecord	inws_rec;
struct inwdRecord	inwd_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocr_rec;
struct podtRecord	podt_rec;
struct sumrRecord	sumr_rec;
struct inspRecord	insp_rec;
struct sudsRecord	suds_rec;
struct excfRecord	excf_rec;
struct lrphRecord	lrph_rec;


struct	{
	float	quantityRecommend;
	float	weeksDemand;
	float	qtyAvailable;
	float	qtyOnOrder;
	float	qtyTotalCover;
	float	qtyCoverRequired;
	float	qtyNettRequired;
} data [3];

#define	BY_CO		0
#define	BY_BR		1
#define	BY_WH		2
#define	WKS_DEMAND	data [BY_CO].weeksDemand
#define	IN_WEEKS(x)	 ((WKS_DEMAND == 0.00) ? 0.00 : x / WKS_DEMAND)

struct	{
	char	data [201];
} supp_rec [ MAX_SUPP ];

struct	{
	char	data [201];
} wh_rec [ MAX_WH ];

struct	{
	char	data [201];
} gwh_rec [ MAX_WH ];

	double	costingInformation [5][25];

	int		clear_ok	=	TRUE,
			display_ok	=	TRUE;

	int		monthlySelect	=	TRUE;
	extern	int	GraphFooter;

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
char	*fullDayNames [] =
{
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun"
};
char	*weekDayNames [] =
{
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri"
};
char	*GraphPrompts [] =
{
	"actual",
	"linear",
	"snl_trn",
	"loc_lin",
	"focusforecast",
};
char	*GraphDescs [] =
{
	"Actual",
	"Linear",
	"Snl_trn",
	"Loc_lin",
	"FocusForecast",
};

float	GetLeadDate 		(long, long);
float	RoundMultiple 		(float, char *, float, float);
int		CalculateWarehouse 	(long);
int		ClearRedraw 		(void);
int		DisplayGlobalWh 	(void);
int		DisplaySupplier 	(void);
int		DisplayWarehouse 	(void);
int		FormulaStatistics	(void);
int		FindProduct 		(int);
int		GetLeadTimes 		(void);
int		GetReviewPeriod		(void);
int		LoadBranch 			(void);
int		LoadCompany 		(void);
int		LoadStockDisplay 	(void);
int		LoadWarehouse		(void);
int		NextDisplay 		(void);
int		PrevDisplay 		(void);
int		ProcessCostGraph 	(void);
int		MonthlyGraph 		(void);
int		DailyGraph 			(void);
int		ProcessGraph 		(void);
int		heading 			(int);
int		spec_valid 			(int);
void	AllDisplay 			(void);
void	CalculateBranch 	(char *,char *);
void	CheckDupResponce 	(int);
void	CloseDB 			(void);
void	DefaultFreight 		(void);
void	DisplayCosting 		(int, int, char	*);
void	GetSupplierDefaults (void);
void	FindCosts 			(void);
void	LcheckFuture 		(void);
void	LoadDistGroup	 	(void);
void	LoadSupplier 	 	(void);
void	OpenDB 				(void);
void	PrintData 			(void);
void	ProcBranch 			(void);
void	ProcCompany 		(void);
void	ProcWarehouse 		(void);
void	ProcessSuppliers 	(int,int);
void	ReadItem 			(void);
void	Redraw 				(void);
void	SetDefaults 	 	(char *, char *);
void	SumData 			(int);
void	ZeroData 			(int);
void	shutdown_prog 		(void);
void 	Process 			(void);

int		byTemp,
		byWhat 		= BY_WH,
		reviewSet 	= FALSE,
		leadTimeSet = FALSE,
		workMinimum	= 0;
int		useMinStockWeeks = FALSE;
float	minStockWeeks = 0.0;
int		LRP_ShowAllSuppliers = 0;

char	currencyCode [4];
char	formatString [256];

long	hhbrHash = 0L;

float	reviewPeriod 		= 0.0,
		LeadWeeks 			= 0.0,
		leadDays 			= 0.0,
		safetyStock 		= 0.0,
		minimumOrder 		= 0.0,
		normalOrder 		= 0.0,
		orderMultiple 		= 0.0,
		quantityRecommend 	= 0.0,
		defaultLeadWeeks 	= 0.0,
		defaultLeadDays		= 0.0,
		defaultReviewPeriod = 0.0,
		envFfDfltReview 	= 0.0;

double	freight = 0.00;
char	envSupOrdRound [2];

char	*ccmr2 	= "ccmr2";

extern int		lp_x_off,
		lp_y_off;


#include	<get_lpno.h>
#include	<LRPFunctions.h>

char	validMethods [5];
char	*headingNames [] =
{
	"Actual",
	"Linear LSA",
	"Seasonal Demand",
	"Local Linear",
	"Focus Forecast"
};

#ifndef GVISION
menu_type	_main_menu [] = {
{"<Company>","Select display by company mode.  [C]",
	LoadCompany, "Cc",	  },
{"<Branch>","Select display by Branch mode.  [B]",
	LoadBranch, "Bb",	  },
{"<Warehouse>","Select display by Warehouse mode.  [W]",
	LoadWarehouse, "Ww",	  },
{"<Reorder Review>","Modify reorder review period. [R]",
	GetReviewPeriod, "Rr",	  },
{"<Lead Time>","Modify lead time. [L]",
	GetLeadTimes, "Ll",	  },
{"<Suppliers>","Display suppliers. [S]",
	DisplaySupplier, "Ss",	  },
{"<Supply Warehouses>","Display Supply Warehouses. [S] or [W]",
	DisplayWarehouse, "SsWw",	  },
{"<Global Supply Warehouses>","Display Global Warehouses. [S] or [W]",
	DisplayGlobalWh, "SsWw",	  },
{"<Graph Monthly Forecast>","Display Graph of Monthly forecast information. [G] or [M]",
	MonthlyGraph, "GgMm",	  },
{"<Graph Daily Forecast>","Display Graph of Daily forecast information. [G] or [D]",
	DailyGraph, "GgDd",	  },
{"<Graph Costing Information>","Display Graph of Costing information. [C]",
	ProcessCostGraph, "Cc",	  },
{"<Formula Statistics>","Display Formula Statistics information. [F]",
	FormulaStatistics, "Ff",	  },
{"<Stock Display>","Call standard stock display. [D]",
	LoadStockDisplay, "Dd",	  },
{" [REDRAW]","Redraw Display", ClearRedraw, "", FN3,			  },
{" [NEXT SCN]", "Display Next Item", NextDisplay, "", FN14,		  },
{" [PREV SCN]", "Display Previous Item", PrevDisplay, "", FN15,		  },
{" [EDIT/END]", "Exit Display", _no_option, "", FN16, EXIT | SELECT		  },
{"",									  },
};
#else
menu_group _main_group [] = {
	{1, "Display Mode"},
	{2, "Reorder Information"},
	{3, "Graphs"},
	{4,"Miscellaneous"},
	{0, ""}					// terminator item
};
menu_type	_main_menu [] = {
{1, "<Company>","Company",
	LoadCompany, },
{1, "<Branch>","Branch",
	LoadBranch, },
{1, "<Warehouse>","Warehouse",
	LoadWarehouse, },
{2, "<Reorder Review>","Reorder Review",
	GetReviewPeriod, },
{2, "<Lead Time>","Lead Time",
	GetLeadTimes, },
{2, "<Suppliers>","Suppliers",
	DisplaySupplier, },
{2, "<Supply Warehouses>","Supply Warehouses",
	DisplayWarehouse, },
{2, "<Global Supply Warehouses>","Global Supply Warehouses",
	DisplayGlobalWh, },
{2, "<Formula Statistics>","Formula Statistics",
	FormulaStatistics, },
{3, "<Graph Monthly Forecast>","Graph Monthly Forecast",
	MonthlyGraph, },
{3, "<Graph Daily Forecast>","Graph Daily Forecast",
	DailyGraph, },
{3, "<Graph Costing Information>","Graph Costing Information",
	ProcessCostGraph, },
{4, "<Stock Display>","Stock Display",
	LoadStockDisplay, },
{4, "[REDRAW]","", ClearRedraw, FN3,			  },
{4, "[NEXT SCN]", "", NextDisplay, FN14,		  },
{4, "[PREV SCN]", "", PrevDisplay, FN15,		  },
{0, "",									  },
};
#endif

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	response [6][2];
	char	responseDesc [6][11];
	int		future;
	double	l_cost;
} local_rec;

static struct	var	vars [] =
{
	{1, LIN, "actual",	12, 20, CHARTYPE,
		"U", "          ",
		" ", "Y", "Actual Data", "Y(es) N(o) or H(ighlight)",
		YES, NO, JUSTLEFT, "YNH", "", local_rec.response [0]},
	{1, LIN, "Actual",	12, 24, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.responseDesc [0]},
	{1, LIN, "linear",	13, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Linear", "Y(es) N(o) or H(ighlight)",
		YES, NO, JUSTLEFT, "YNH", "", local_rec.response [1]},
	{1, LIN, "Linear",	13, 24, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.responseDesc [1]},
	{1, LIN, "snl_trn",	14, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Seas. Trend", "Y(es) N(o) or H(ighlight)",
		YES, NO, JUSTLEFT, "YNH", "", local_rec.response [2]},
	{1, LIN, "Snl_trn",	14, 24, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.responseDesc [2]},
	{1, LIN, "loc_lin",	15, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Local Lin.", "Y(es) N(o) or H(ighlight)",
		YES, NO, JUSTLEFT, "YNH", "", local_rec.response [3]},
	{1, LIN, "Loc_lin",	15, 24, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.responseDesc [3]},
	{1, LIN, "focusforecast",		16, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Focus Forecast", "Y(es) N(o) or H(ighlight)",
		YES, NO, JUSTLEFT, "YNH", "", local_rec.response [4]},
	{1, LIN, "FocusForecast",		16, 24, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.responseDesc [4]},
	{1, LIN, "future",		17, 20, INTTYPE,
		"NN", "          ",
		" ", "12", "Future Periods", "Number of Future Periods in Graph(Dflt = Max = 12)",
		YES, NO, JUSTRIGHT, "0", "12", (char *)&local_rec.future},
	{0, LIN, "",		 0,  0, CHARTYPE,
		"U", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dummy}
};
#include	<SupPrice.h>

extern void		read_comm ();	/*applib*/

#include	<RealCommit.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	sptr = chk_env ("LRP_METHODS");
	if (sptr == (char *) NULL)
	{
		print_at (0,0,mlLrpMess032);
		exit (0);
	}
	else
	{
		sprintf (validMethods,"%-4.4s", sptr);
	}
	SETUP_SCR (vars);
	_set_masks ("lrp_display.s");

	sptr = chk_env ("LRP_SHOW_ALL_SU");
	if (sptr == (char *) 0)
		LRP_ShowAllSuppliers = 0;
	else
		LRP_ShowAllSuppliers = atoi (sptr);

	/*---------------------------
	| check parameters			|
	---------------------------*/
	if (argc != 1 && argc != 3)
	{
		print_at (0, 0, "Usage %s [Optional hhbrHash bywhat(0=Co,1=Br,2=Wh)]", argv [0]);
		exit (argc);
	}
	/*---------------------------
	| set hhbr_hash				|
	---------------------------*/
	if (argc == 3)
	{
		hhbrHash = atol (argv [1]);
		byWhat	 = atoi (argv [2]);
	}
	else
		hhbrHash = 0L;
	/*-----------------------------------
	| initialise screen & terminal		|
	-----------------------------------*/
	init_scr ();
	set_tty ();
	swide ();

	OpenDB ();

	if (byWhat	==	BY_CO)
		setup_LSA (BY_CO, comm_rec.co_no, "  ", "  ");
	if (byWhat	==	BY_BR)
		setup_LSA (BY_BR, comm_rec.co_no, comm_rec.est_no, "  ");
	if (byWhat	==	BY_WH)
		setup_LSA (BY_WH, comm_rec.co_no, comm_rec.est_no, comm_rec.cc_no);

	search_ok = 1;
	sptr = get_env ("CURR_CODE");
	sprintf (currencyCode, "%-3.3s", sptr);

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envSupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envSupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envSupOrdRound, "D");
			break;

		default:
			sprintf (envSupOrdRound, "B");
			break;
		}
	}

	sptr = chk_env ("LRP_DFLT_REVIEW");
	if (sptr == (char *) 0)
		envFfDfltReview = 4;
	else
		envFfDfltReview = atof (sptr);

	/*---------------------------
	| process item				|
	---------------------------*/
	Process ();
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*===============================
| Open data base files.			|
===============================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);

	abc_alias (ccmr2, ccmr);
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_hhcf_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2,ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ffpr, ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
	open_rec (inws, inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd, inwd_list, INWD_NO_FIELDS, "inwd_id_no");
	LSA_open ();
}

/*===============================
| Close data base files.		|
===============================*/
void
CloseDB (void)
{
	abc_fclose (ccmr2);
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (esmr);
	abc_fclose (ffpr);
	abc_fclose (incc);
	abc_fclose (inis);
	abc_fclose (inld);
	abc_fclose (inmr);
	abc_fclose (pocf);
	abc_fclose (pocr);
	abc_fclose (podt);
	abc_fclose (sumr);
	abc_fclose (insp);
	abc_fclose (suds);
	abc_fclose (soic);
	abc_fclose (inws);
	abc_fclose (inwd);
	CloseCosting ();
	LSA_close ();
	abc_dbclose ("data");
}
/*===========================
| process hhbrHash			|
===========================*/
void
Process (void)
{
	int		firstTime	=	TRUE;

	/*-------------------------------
	| find item if passed			|
	-------------------------------*/
	if (hhbrHash != 0L)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", hhbrHash);
		if (cc)
			return;

		abc_selfield (inmr, "inmr_id_no");
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "inmr", "DBFIND");

		if (byWhat == BY_CO)
			LoadCompany ();

		if (byWhat == BY_BR)
			LoadBranch ();

		if (byWhat == BY_WH)
			LoadWarehouse ();
		
		reviewSet 		= FALSE;
		leadTimeSet 	= FALSE;
		GetSupplierDefaults ();
		hhbrHash 	= inmr_rec.hhbr_hash;
		heading (1);
	}
	else
	{
		abc_selfield (inmr, "inmr_id_no");
		inmr_rec.hhbr_hash = -1L;
	}

	while (prog_exit == FALSE)
	{
		crsr_off ();
		entry_exit 	=	FALSE;
		prog_exit	=	FALSE;
		restart 	=	FALSE;
		clear_ok	=	TRUE;
		display_ok	=	TRUE;
		search_ok 	=	TRUE;

		if (firstTime	==	TRUE)
		{
			heading (1);
			firstTime	=	FALSE;
		}
		/*---------------------------
		| display heading			|
		---------------------------*/
		if (hhbrHash != inmr_rec.hhbr_hash)
		{
			ReadItem ();
			if (prog_exit || restart)
			{
				continue;
			}

			crsr_off ();

			hhbrHash = inmr_rec.hhbr_hash;
			if (byWhat == BY_CO)
				LoadCompany ();

			if (byWhat == BY_BR)
				LoadBranch ();

			if (byWhat == BY_WH)
				LoadWarehouse ();
			
			reviewSet 		= FALSE;
			leadTimeSet 	= FALSE;
			GetSupplierDefaults ();
		}
		if (restart)
			continue;
				
		clear_ok	=	FALSE;
		heading (1);
#ifndef GVISION		
		run_menu (_main_menu, "", 19);
#else
		run_menu (_main_group, _main_menu);
#endif
		hhbrHash	=	0L;
	}
}

int
ClearRedraw (void)
{
	clear ();
	display_ok = TRUE;
	Redraw ();
	return (EXIT_SUCCESS);
}

void
Redraw (void)
{
	clear_ok	=	FALSE;
	heading (1);
}
void
ProcCompany (void)
{
	byTemp = byWhat;
	byWhat = BY_CO;
	GetSupplierDefaults ();
	cc = setup_LSA (BY_CO, comm_rec.co_no, "  ", "  ");
	if (cc)
	{
		print_mess (ML (mlLrpMess058));
		sleep (sleepTime);
		clear_mess ();
		byWhat = byTemp;	
		GetSupplierDefaults ();
		setup_LSA (byWhat, comm_rec.co_no, comm_rec.est_no,
					 (byWhat == BY_BR) ? "  " : comm_rec.cc_no);
	}
	display_ok	=	TRUE;
	Redraw ();
}
void
ProcBranch (void)
{
	byWhat = BY_BR;
	GetSupplierDefaults ();
	setup_LSA
	 (
		BY_BR,
		comm_rec.co_no,
		comm_rec.est_no,
		"  "
	);
	display_ok	=	TRUE;
	Redraw ();
}
void
ProcWarehouse (void)
{
	byWhat = BY_WH;
	GetSupplierDefaults ();
	setup_LSA
	 (
		BY_WH,
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no
	);
	display_ok	=	TRUE;
	Redraw ();
}
int
NextDisplay (void)
{
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	if (!cc && !FindProduct (NEXT))
	{
		hhbrHash = inmr_rec.hhbr_hash;

		if (byWhat == BY_CO)
			LoadCompany ();

		if (byWhat == BY_BR)
			LoadBranch ();

		if (byWhat == BY_WH)
			LoadWarehouse ();

		reviewSet 		= FALSE;
		leadTimeSet 	= FALSE;
		GetSupplierDefaults ();
	}
	Redraw ();
	return (EXIT_SUCCESS);
}
int
PrevDisplay (void)
{
	cc = find_rec (inmr, &inmr_rec, LTEQ, "r");
	if (!cc && !FindProduct (PREVIOUS))
	{
		hhbrHash = inmr_rec.hhbr_hash;

		if (byWhat == BY_CO)
			LoadCompany ();

		if (byWhat == BY_BR)
			LoadBranch ();

		if (byWhat == BY_WH)
			LoadWarehouse ();

		reviewSet 		= FALSE;
		leadTimeSet 	= FALSE;
		GetSupplierDefaults ();
	}
	Redraw ();
	return (EXIT_SUCCESS);
}

void
ReadItem (void)
{
	crsr_on ();
	while (TRUE)
	{
		getalpha (12, 2, "UUUUUUUUUUUUUUUU", temp_str);
		switch (last_char)
		{
		case	FN1:
			hhbrHash	=	0L;
			restart	=	TRUE;
			return;

		case	FN4:
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			clip (temp_str);
			ClearRedraw ();
			heading (1);
			continue;

		case	FN16:
			hhbrHash	=	0L;
			prog_exit	=	TRUE;
			return;

		case	'\r':
			if (dflt_used)
				sprintf (temp_str, "%-16.16s", inmr_rec.item_no);

			cc = FindInmr (comm_rec.co_no, temp_str, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, temp_str);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			/*---------------------------
			| find failed				|
			---------------------------*/
			if (cc)
			{
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				continue;
			}
			SuperSynonymError ();
			print_at (2, 12, inmr_rec.item_no);
			print_at (2, 31, inmr_rec.description);
			print_at (2, 80, inmr_rec.sale_unit);
			return;

		default:
			putchar (BELL);
			break;
		}
	}
}

int
GetReviewPeriod (void)
{
	crsr_on ();
	while (1)
	{
		reviewPeriod = getfloat (60, 4, "NNNNN.NN");
		reviewSet = TRUE;
		switch (last_char)
		{
		case	FN1:
				return (EXIT_SUCCESS);

		case	FN16:
				prog_exit	=	TRUE;
				return (EXIT_SUCCESS);

		case	'\r':
			if (dflt_used)
			{
				reviewPeriod = defaultReviewPeriod;
				reviewSet = FALSE;
			}
			print_at (4,64,"%4.0f", reviewPeriod);
			if (byWhat == BY_CO)
				LoadCompany ();

			if (byWhat == BY_BR)
				LoadBranch ();

			if (byWhat == BY_WH)
				LoadWarehouse ();
			return (EXIT_SUCCESS);

		default:
			putchar (BELL);
			break;
		}
	}
	return (EXIT_SUCCESS);
}

int
GetLeadTimes (void)
{
	crsr_on ();
	while (1)
	{
		leadDays 	= getfloat (60, 5, "NNNNN.NN");
		LeadWeeks 	= leadDays / 7;
		leadTimeSet 	= TRUE;
		switch (last_char)
		{
		case	FN1:
			return (EXIT_SUCCESS);

		case	FN16:
		case	'\r':
			if (dflt_used)
			{
				leadDays 	= defaultLeadDays;
				LeadWeeks 	= defaultLeadWeeks;
				leadTimeSet    = FALSE;
			}
			print_at (5,64,"%4.0f", leadDays);
			if (byWhat == BY_CO)
				LoadCompany ();

			if (byWhat == BY_BR)
				LoadBranch ();

			if (byWhat == BY_WH)
				LoadWarehouse ();
			return (EXIT_SUCCESS);

		default:
			putchar (BELL);
			break;
		}
	}
	return (EXIT_SUCCESS);
}

void
GetSupplierDefaults (void)
{
	/*---------------------------
	| reset default				|
	---------------------------*/
	defaultReviewPeriod = 0.00;
	defaultLeadDays 	= 0.00;
	defaultLeadWeeks 	= 0.00;
	SetDefaults (comm_rec.est_no, comm_rec.cc_no);

	if (!reviewSet)
		reviewPeriod = defaultReviewPeriod;

	if (!leadTimeSet)
	{
		LeadWeeks = defaultLeadWeeks;
		leadDays  = defaultLeadDays;
	}
}

void
SetDefaults (
	char	*br_no,
	char	*wh_no)
{
	float	leadDays = 0.00;
	float	LeadWeeks = 0.00;

	inis_rec.hhbr_hash = hhbrHash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, br_no);
	strcpy (inis_rec.wh_no, wh_no);
	cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.hhbr_hash = hhbrHash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, br_no);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = hhbrHash;
			strcpy (inis_rec.sup_priority, "C1");
			strcpy (inis_rec.co_no, comm_rec.co_no);
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		}
	}
	if (!cc)
	{
		if (inis_rec.lead_time == 0.00)
		{
			leadDays  = GetLeadDate 
						 (
							inis_rec.hhis_hash, 
							comm_rec.inv_date
						);
			LeadWeeks = leadDays / 7;
		}
		else
		{
			leadDays  = inis_rec.lead_time;
			LeadWeeks = leadDays / 7;
		}
	}
	else
	{
		leadDays	=	0;
		LeadWeeks	=	0;
	}

	/*-----------------------------------
	| set default lead time if required	|
	-----------------------------------*/
	if (LeadWeeks > defaultLeadWeeks)
	    defaultLeadWeeks = LeadWeeks;

	/*-----------------------------------
	| set default lead time if required	|
	-----------------------------------*/
	if (leadDays > defaultLeadDays)
	    defaultLeadDays = leadDays;

	/*---------------------------------------
	| Find out what the review-period   	|
	| is for this product. Firstly, try for	|
	| a match on branch/item. Then try for	|
	| a match on item. Then try for a match	|
	| on branch/category. If this fails,	|
	| then use LRP_DFLT_REVIEW environment-	|
	| value. If not found, dflt to 4 weeks.	|
	---------------------------------------*/
	ffpr_rec.hhbr_hash = hhbrHash;
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
					ffpr_rec.review_prd = envFfDfltReview;
			}
			abc_selfield (ffpr, "ffpr_id_no");
	    }
	}

	/*---------------------------------------
	| set default review period if required	|
	---------------------------------------*/
	if (ffpr_rec.review_prd > defaultReviewPeriod)
	    defaultReviewPeriod = ffpr_rec.review_prd;
}

int
FindProduct (
	int		dirn)
{
	char	item_no [17];
	/*-------------------------------
	| save current item number		|
	-------------------------------*/
	strcpy (item_no, inmr_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, dirn, "r");
	if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, item_no);
		return (find_rec (inmr, &inmr_rec, COMPARISON, "r"));
	}
	return (EXIT_SUCCESS);
}

int
DisplaySupplier (void)
{
	int		i;
	char	head_str [200];

	/*-----------------------------------
	| setup supplier item display		|
	-----------------------------------*/
	Dsp_open (0, 10, 5);
	sprintf (head_str, "Br/Wh|Pr|Supplier|    Supplier    |  Lead | Cur | Cty | FOB(%-3.3s)  |  Duty | Freight + |Into Store | Minimum  |  Normal  |  Order  ", currencyCode);
	Dsp_saverec (head_str);
	Dsp_saverec ("No/No|No| Number |   Item Number  |  Time |     |     |   Cost    |Percent|Contingency|   Cost    | Ord. Qty | Ord. Qty | Multiple");
	Dsp_saverec (" [REDRAW][NEXT][PREV][EDIT/END] ");

	/*-----------------------------
	| Print supplier information. |
	-----------------------------*/
	for (i = 0; i < MAX_SUPP; i++)
	{
		if (strncmp (supp_rec [ i ].data + 9, "      ", 6))
			Dsp_saverec (supp_rec [i].data);
	}

	Dsp_srch ();
	Dsp_close ();
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

int
DisplayWarehouse (void)
{
	int		i;
	/*-----------------------------------
	| setup warehouse item display		|
	-----------------------------------*/
	Dsp_open (0, 10, 5);
	Dsp_saverec ("  S u p p l y    |R e c e i v i n g |Pri|  Lead | Cost Price | Uplift | Uplift | Transfer   | Minimum  |  Normal  |  Order  ");
	Dsp_saverec ("Br / Wh  Acronym |Br / Wh   Acronym |No.|  Time |            | Percent| Amount |    Price   | Ord. Qty | Ord. Qty | Multiple");
	Dsp_saverec (" [REDRAW][NEXT][PREV][EDIT/END] ");

	/*------------------------------
	| Print warehouse information. |
	------------------------------*/
	for (i = 0; i < MAX_WH; i++)
	{
		if (strncmp (wh_rec [ i ].data + 6, "      ", 6))
			Dsp_saverec (wh_rec [i].data);
	}

	Dsp_srch ();
	Dsp_close ();
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

int
DisplayGlobalWh (void)
{
	int		i;
	/*-----------------------------------
	| setup warehouse item display		|
	-----------------------------------*/
	Dsp_open (0, 10, 5);
	Dsp_saverec (" Supply |Receive |  Category   |Pri|  Lead | Cost Price | Uplift | Uplift | Transfer   | Minimum  |  Normal  |  Order  ");
	Dsp_saverec ("Br / Wh |Br / Wh |             |No.|  Time |            | Percent| Amount |    Price   | Ord. Qty | Ord. Qty | Multiple");
	Dsp_saverec (" [REDRAW][NEXT][PREV][EDIT/END] ");

	/*------------------------------
	| Print warehouse information. |
	------------------------------*/
	for (i = 0; i < MAX_WH; i++)
	{
		if (strncmp (gwh_rec [ i ].data + 6, "      ", 6))
			Dsp_saverec (gwh_rec [i].data);
	}

	Dsp_srch ();
	Dsp_close ();
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

void
ZeroData (int _byWhat)
{
	data [_byWhat].quantityRecommend	= 0.00;
	data [_byWhat].weeksDemand		= 0.00;
	data [_byWhat].qtyAvailable		= 0.00;
	data [_byWhat].qtyOnOrder		= 0.00;
	data [_byWhat].qtyTotalCover	= 0.00;
	data [_byWhat].qtyCoverRequired	= 0.00;
	data [_byWhat].qtyNettRequired		= 0.00;
}

void
SumData (int _byWhat)
{
	data [_byWhat].quantityRecommend	+= data [_byWhat + 1].quantityRecommend;
	data [_byWhat].weeksDemand		+= data [_byWhat + 1].weeksDemand;
	data [_byWhat].qtyAvailable		+= data [_byWhat + 1].qtyAvailable;
	data [_byWhat].qtyOnOrder		+= data [_byWhat + 1].qtyOnOrder;
	data [_byWhat].qtyTotalCover	+= data [_byWhat + 1].qtyTotalCover;
	data [_byWhat].qtyCoverRequired	+= data [_byWhat + 1].qtyCoverRequired;
	data [_byWhat].qtyNettRequired		+= data [_byWhat + 1].qtyNettRequired;
}

void
PrintData (void)
{
	print_at (2, 12, inmr_rec.item_no);
	print_at (2, 31, inmr_rec.description);
	print_at (2, 80, inmr_rec.sale_unit);

	print_at (4,97,		"%8.2f      ", 	IN_WEEKS (data [BY_CO].qtyAvailable));
	print_at (4,111,	"%10.2f", 		data [BY_CO].qtyAvailable);

	print_at (5,97,		"%8.2f      ", 	IN_WEEKS (data [BY_CO].qtyOnOrder));
	print_at (5,111,	"%10.2f", 		data [BY_CO].qtyOnOrder);

	print_at (6,97,		"%8.2f      ", 	IN_WEEKS (data [BY_CO].qtyTotalCover));
	print_at (6,111,	"%10.2f", 		data [BY_CO].qtyTotalCover);

	print_at (7,97,		"%8.2f      ", 	IN_WEEKS (data [BY_CO].qtyCoverRequired));
	print_at (7,111,	"%10.2f", 		data [BY_CO].qtyCoverRequired);

	print_at (8,97,		"%8.2f      ", 	IN_WEEKS (data [BY_CO].qtyNettRequired));
	print_at (8,111,	"%10.2f", 		data [BY_CO].qtyNettRequired);

	print_at (4,60,		"%8.2f ", 		reviewPeriod);
	print_at (5,60,		"%8.2f ", 		leadDays);

	if (useMinStockWeeks)
	{
		print_at (6,43,	ML (mlLrpMess073));
		print_at (6,60,	"%8.2f ", minStockWeeks);
	}
	else
	{
		print_at (6,43,	ML (mlLrpMess015));
		print_at (6,60,	"%8.2f ", safetyStock);
	}
	if (useMinStockWeeks)
		print_at (8,60, "%8.2f ", minStockWeeks + reviewPeriod + LeadWeeks);
	else
		print_at (8,60, "%8.2f ", safetyStock + reviewPeriod + LeadWeeks);

	fflush (stdout);

	print_at (4,27,"%10.2f", data [BY_CO].weeksDemand);
	print_at (5,27,"%10.2f", minimumOrder);
	print_at (6,27,"%10.2f", normalOrder);
	print_at (7,27,"%10.2f", orderMultiple);

	quantityRecommend = data [BY_CO].quantityRecommend;
	quantityRecommend = RoundMultiple (quantityRecommend, envSupOrdRound, orderMultiple, minimumOrder);

	print_at (8,27,"%10.2f", quantityRecommend);
}


/*=======================================
| Load supplier details into table	|
=======================================*/
void
LoadDistGroup (void)
{
	int		i;
	double	UpliftValue	=	0.00;
	float	DefaultLead	=	0.00;
	char	br_no [3],
			wh_no [3],
			acronym [10];

	for (i = 0; i < MAX_WH; i++)
		sprintf (wh_rec [ i ].data, "%-200.200s", " ");

	for (i = 0; i < MAX_WH; i++)
		sprintf (gwh_rec [ i ].data, "%-200.200s", " ");

	i	=	0;

	FindCosts ();
	
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
		inwd_rec.hhcc_hash	=	0L;
		inwd_rec.hhbr_hash	=	0L;
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

			UpliftValue	=	0.00;
			if (inwd_rec.upft_pc > 0.00)
			{
				UpliftValue	=	 (double) inwd_rec.upft_pc;
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
			strcpy (formatString,"%s / %s ^E%s / %s ^E %-11.11s ^E %s ^E%6.2f ^E%11.2f ^E%6.2f%% ^E%7.2f ^E%11.2f ^E%9.2f ^E%9.2f ^E%9.2f ");

			sprintf 
			(
				gwh_rec [ i++ ].data, formatString, 
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
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
		}
		cc = find_rec (inws, &inws_rec, NEXT, "r");
	}
	i	=	0;

	/*-----------------------------------
	| load data from inventory supplier	|
	-----------------------------------*/
	inws_rec.hhbr_hash	=	hhbrHash;
	inws_rec.hhcc_hash	=	0L;
	inws_rec.hhcf_hash	=	0L;
	strcpy (inws_rec.sup_priority, " ");
	cc = find_rec (inws, &inws_rec, GTEQ, "r");
	while (!cc && inws_rec.hhbr_hash == hhbrHash)
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
		inwd_rec.hhcc_hash	=	0L;
		inwd_rec.hhbr_hash	=	0L;
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

			UpliftValue	=	0.00;
			if (inwd_rec.upft_pc > 0.00)
			{
				UpliftValue	=	 (double) inwd_rec.upft_pc;
				UpliftValue	*=	local_rec.l_cost;
				UpliftValue	=	DOLLARS (UpliftValue);
				UpliftValue	=	twodec (UpliftValue);
			}
			if (inwd_rec.upft_amt > 0.00)
				UpliftValue	+=	DOLLARS (inwd_rec.upft_amt);
			
			/*---------------------------
			| store data				|
			---------------------------*/
			strcpy (formatString, "%s / %s %9.9s^E%s / %s %9.9s ^E %s ^E%6.2f ^E%11.2f ^E%6.2f%% ^E%7.2f ^E%11.2f ^E%9.2f ^E%9.2f ^E%9.2f ");
			sprintf (wh_rec [ i++ ].data, formatString,
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
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
		}
		cc = find_rec (inws, &inws_rec, NEXT, "r");
	}
}

void
FindCosts (void)
{
	switch (inmr_rec.costing_flag [0])
	{
	case 'L':
	case 'A':
	case 'P':
	case 'T':
		local_rec.l_cost	=	FindIneiCosts
								(
									inmr_rec.costing_flag,
									comm_rec.est_no,
									inmr_rec.hhbr_hash
								);
		break;

	case 'F':
		local_rec.l_cost	= 	FindIncfValue 
								(
									incc_rec.hhwh_hash,
									incc_rec.closing_stock, 
									TRUE, 
									TRUE,
									inmr_rec.dec_pt
								);
		break;

	case 'I':
		local_rec.l_cost	= 	FindIncfValue 
								(
									incc_rec.hhwh_hash,
									incc_rec.closing_stock, 
									TRUE, 
									FALSE,
									inmr_rec.dec_pt
								);
		break;

	case 'S':
		local_rec.l_cost	= FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;
	}
	return;
}
/*=======================================
| Load supplier details into table	|
=======================================*/
void
LoadSupplier (void)
{
	int		first_supp 	= TRUE;
	int		i;

	for (i = 0; i < MAX_SUPP; i++)
		sprintf (supp_rec [ i ].data, "%-200.200s", " ");

	i	=	0;

	/*-----------------------------------
	| load data from inventory supplier	|
	-----------------------------------*/
	inis_rec.hhbr_hash = hhbrHash;
	strcpy (inis_rec.sup_priority, "C ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == hhbrHash &&
					inis_rec.sup_priority [0] == 'C')
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec,COMPARISON, "r");
		if (!cc)
		{
			ProcessSuppliers (first_supp, i++);
			first_supp = FALSE;
		}
		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
	inis_rec.hhbr_hash = hhbrHash;
	strcpy (inis_rec.sup_priority, "B ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == hhbrHash &&
					inis_rec.sup_priority [0] == 'B')
	{
		if (!LRP_ShowAllSuppliers && 
			 (strcmp (inis_rec.co_no, comm_rec.co_no) ||
			strcmp (inis_rec.br_no, comm_rec.est_no)))
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}

		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec,COMPARISON, "r");
		if (!cc)
		{
			ProcessSuppliers (first_supp,i++);
			first_supp = FALSE;
		}
		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
	inis_rec.hhbr_hash = hhbrHash;
	strcpy (inis_rec.sup_priority, "W ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhbr_hash == hhbrHash &&
					inis_rec.sup_priority [0] == 'W')
	{
		if (!LRP_ShowAllSuppliers && 
			 (strcmp (inis_rec.co_no, comm_rec.co_no) ||
			 strcmp (inis_rec.br_no, comm_rec.est_no) ||
			 strcmp (inis_rec.wh_no, comm_rec.cc_no)))
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec,COMPARISON, "r");
		if (!cc)
		{
			ProcessSuppliers (first_supp,i++);
			first_supp = FALSE;
		}
		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
}


void
ProcessSuppliers
 (	
	int		first_supp,
	int		RecordNumber
)
{
	float	duty_pc 	= 0.00;
	double	fob_cost 	= 0.00;
	double	cif_cost 	= 0.00;
	double	duty 		= 0.00;
	double	contingency = 0.00;
	float	discArray [4];	/* Regulatory and Disc A, B, C percents */
	int		cumulative;

	if (inis_rec.lead_time == 0.00)
		inis_rec.lead_time	=	GetLeadDate 
								 (
									inis_rec.hhis_hash, 
									comm_rec.inv_date
								);
	if (first_supp)
	{
		minimumOrder  		= inis_rec.min_order;
		normalOrder 		= inis_rec.norm_order;
		orderMultiple 		= inis_rec.ord_multiple;
	}
					
	fob_cost		= 	GetSupPrice
				  		 (
							inis_rec.hhsu_hash,
							inis_rec.hhbr_hash,
							inis_rec.fob_cost,
							quantityRecommend 
						);
					
	cumulative  	= 	GetSupDisc
						 (
							inis_rec.hhsu_hash,
							inmr_rec.buygrp,
							quantityRecommend,
							discArray 
						);

	fob_cost		=	CalcNet
						 (
							fob_cost, 
							discArray, 
							cumulative
						);

	/*-------------------------------
	| find currency code			|
	-------------------------------*/
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,sumr_rec.curr_code);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	if (cc || pocr_rec.ex1_factor == 0.00)
		pocr_rec.ex1_factor = 1.00;

	fob_cost /= pocr_rec.ex1_factor;

	/*---------------------------
	| find duty code			|
	---------------------------*/
	strcpy (podt_rec.co_no,comm_rec.co_no);
	strcpy (podt_rec.code,inis_rec.duty);
	cc = find_rec (podt,&podt_rec,COMPARISON,"r");
	if (!cc)
	{
		/*-------------------------------
		| duty is percentage			|
		-------------------------------*/
		if (podt_rec.duty_type [0] == 'P')
		{
			duty_pc = podt_rec.im_duty;
			duty = DOLLARS (duty_pc) * fob_cost;
		}
		else
		{
			duty = podt_rec.im_duty;
			if (duty + fob_cost != 0.00)
				duty_pc = duty / (duty + fob_cost);
		}
	}
	else
	{
		duty = 0.00;
		duty_pc = 0.00;
	}

	DefaultFreight ();
	
	cif_cost = fob_cost + duty + freight;

	contingency = DOLLARS (comr_rec.contingency);
	contingency *= cif_cost;

	/*---------------------------
	| store data				|
	---------------------------*/
	sprintf (supp_rec [RecordNumber].data, "%s/%s^E%s^E %-6.6s ^E%-16.16s^E%6.2f ^E %-3.3s ^E %-3.3s ^E%11.4f^E %5.1f ^E %9.2f ^E%11.4f^E%9.2f ^E%9.2f ^E%9.2f ",
				inis_rec.br_no,
				inis_rec.wh_no,
				inis_rec.sup_priority,
				sumr_rec.crd_no,
				inis_rec.sup_part,
				inis_rec.lead_time,
				sumr_rec.curr_code,
				sumr_rec.ctry_code,
				fob_cost,
				duty_pc,
				freight + contingency,
				cif_cost + contingency,
				inis_rec.min_order,
				inis_rec.norm_order,
				inis_rec.ord_multiple);
}
void
DefaultFreight (void)
{
	double	frt_conv = 0.00;

	freight = 0.00;

	strcpy (pocf_rec.co_no,comm_rec.co_no);
	strcpy (pocf_rec.code,sumr_rec.ctry_code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
		return;

	/*-----------------------
	| Calculate Freight	|
	-----------------------*/
	frt_conv = pocf_rec.freight_load;

	if (pocf_rec.load_type [0] == 'U')
		freight = frt_conv;

	if (pocf_rec.load_type [0] == 'P')
		freight = (inis_rec.fob_cost * frt_conv) / 100;

	freight /= pocr_rec.ex1_factor;
	return;
}

float	
tri_max (
 float	a,
 float	b,
 float	c)
{
	if (a > b)
		if (a > c)
			return (a);
		else
			return ((c > b) ? c : b);
	else
		if (b > c)
			return (b);
		else
			return ((c > a) ? c : a);
}

int
LoadCompany (void)
{
	float	cover;

	ProcCompany ();

	/*-------------------------------
	| zero totals for company		|
	-------------------------------*/
	ZeroData (BY_CO);

	/*-------------------------------
	| load branch suppliers			|
	-------------------------------*/
	LoadSupplier ();
	LoadDistGroup ();
	
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr,&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp (esmr_rec.co_no,comm_rec.co_no))
	{
		/*-----------------------------------
		| calculate figure for branch		|
		-----------------------------------*/
		CalculateBranch (esmr_rec.co_no, esmr_rec.est_no);

		cc = find_rec (esmr,&esmr_rec,NEXT,"r");
	}
	safetyStock = inmr_rec.safety_stock;

	minStockWeeks = 0.0;
	if (data [BY_CO].weeksDemand != 0.0)
		minStockWeeks	 = inmr_rec.min_quan / data [BY_CO].weeksDemand;

	useMinStockWeeks = FALSE;
	if (minStockWeeks > safetyStock)
	{
		cover	 = reviewPeriod + LeadWeeks + minStockWeeks;
		useMinStockWeeks = TRUE;
	}
	else
		cover	 = reviewPeriod + LeadWeeks + safetyStock;

	/*-----------------------------------
	| cover required & nett requirement	|
	-----------------------------------*/
	data [BY_CO].qtyCoverRequired = cover;
	data [BY_CO].qtyCoverRequired *= data [BY_CO].weeksDemand;
	data [BY_CO].qtyNettRequired = data [BY_CO].qtyCoverRequired -
							   data [BY_CO].qtyTotalCover;

	if (data [BY_CO].qtyNettRequired < 0.00)
		data [BY_CO].qtyNettRequired = 0.00;
	/*-------------------------------
	| reorder qty recommended		|
	-------------------------------*/
	if (data [BY_CO].qtyNettRequired < 0.01)
		data [BY_CO].quantityRecommend = 0.00;
	else
		data [BY_CO].quantityRecommend = tri_max (data [BY_CO].qtyNettRequired, 0.00, 0.00);
	Redraw ();
	return (EXIT_SUCCESS);
}

int
LoadBranch (void)
{
	float	cover;

	ProcBranch ();

	ZeroData (BY_CO);

	/*-------------------------------
	| load branch suppliers			|
	-------------------------------*/
	LoadSupplier ();
	LoadDistGroup ();

	CalculateBranch (comm_rec.co_no,comm_rec.est_no);

	minStockWeeks = 0.0;
	if (data [BY_CO].weeksDemand != 0.0)
		minStockWeeks	 = ineiRec.min_stock / data [BY_CO].weeksDemand;

	useMinStockWeeks = FALSE;
	if (minStockWeeks > safetyStock)
	{
		cover	 = reviewPeriod + LeadWeeks + minStockWeeks;
		useMinStockWeeks = TRUE;
	}
	else
		cover	 = reviewPeriod + LeadWeeks + safetyStock;

	/*---------------------------------------
	| cover required & nett requirement	|
	---------------------------------------*/
	data [BY_CO].qtyCoverRequired = cover;
	data [BY_CO].qtyCoverRequired *= data [BY_CO].weeksDemand;
	data [BY_CO].qtyNettRequired = data [BY_CO].qtyCoverRequired -
					data [BY_CO].qtyTotalCover;
	if (data [BY_CO].qtyNettRequired < 0.00)
		data [BY_CO].qtyNettRequired = 0.00;
	/*---------------------------------------
	| reorder qty recommended		|
	---------------------------------------*/
	if (data [BY_CO].qtyNettRequired < 0.01)
		data [BY_CO].quantityRecommend = 0.00;
	else
		data [BY_CO].quantityRecommend = tri_max (data [BY_CO].qtyNettRequired,
					0.00,
					0.00);
	Redraw ();
	return (EXIT_SUCCESS);
}

int
LoadWarehouse (void)
{
	ProcWarehouse ();

	/*---------------------------
	| zero totals 				|
	---------------------------*/
	ZeroData (BY_CO);
	ZeroData (BY_BR);
	
	/*-----------------------------------
	| read warehouse master record		|
	-----------------------------------*/
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		return (EXIT_SUCCESS);

	if (ccmr_rec.lrp_ok [0] == 'N')
		return (EXIT_SUCCESS);

	/*-----------------------------------
	| calculate figure for warehouse	|
	-----------------------------------*/
	cc = CalculateWarehouse (ccmr_rec.hhcc_hash);
	if (cc)
		return (EXIT_SUCCESS);

	minStockWeeks = 0.0;
	useMinStockWeeks = FALSE;

	/*-------------------------------
	| load branch suppliers			|
	-------------------------------*/
	LoadSupplier ();
	LoadDistGroup ();

	/*---------------------------------------
	| calculate figure for branch & company	|
	---------------------------------------*/
	SumData (BY_BR);
	SumData (BY_CO);
	Redraw ();
	return (EXIT_SUCCESS);
}

void
CalculateBranch (
	char	*co_no, 
	char	*br_no)
{
	/*-------------------------------
	| zero totals for branch		|
	-------------------------------*/
	ZeroData (BY_BR);
	/*-----------------------------------
	| read inventory branch record		|
	-----------------------------------*/
	cc = FindInei (hhbrHash, comm_rec.est_no, "r");
	safetyStock = (!cc) ? ineiRec.safety_stock : 0.0;

	/*-----------------------------------
	| read warehouse master record		|
	-----------------------------------*/
	strcpy (ccmr_rec.co_no,co_no);
	strcpy (ccmr_rec.est_no,br_no);
	strcpy (ccmr_rec.cc_no,"  ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,co_no) && 
		      !strcmp (ccmr_rec.est_no,br_no))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
			continue;
		}
		
		/*-----------------------------------
		| calculate figure for warehouse	|
		-----------------------------------*/
		cc = CalculateWarehouse (ccmr_rec.hhcc_hash);

		/*-----------------------------------
		| add warehouse to current branch	|
		-----------------------------------*/
		if (!cc)
			SumData (BY_BR);

		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	/*-----------------------------------
	| add branch to current company		|
	-----------------------------------*/
	SumData (BY_CO);
}

int
CalculateWarehouse (
	long	hhccHash)
{
	float	cover;
	float	realCommitted;

	/*-------------------------------
	| zero data for warehouse		|
	-------------------------------*/
	ZeroData (BY_WH);
	if (byWhat == BY_CO)
	{
		minimumOrder  = 0.00;
		normalOrder = 0.00;
		orderMultiple = 0.00;
	}
	/*---------------------------
	| read warehouse			|
	---------------------------*/
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		return (cc);

	/*---------------------------
	| set safety stock			|
	---------------------------*/
	if (byWhat == BY_WH)
		safetyStock = incc_rec.safety_stock;
	/*-------------------------------
	| check weeks demand			|
	-------------------------------*/
	data [BY_WH].weeksDemand = twodec (incc_rec.wks_demand);

	/*---------------------------------
	| Calculate Actual Qty Committed. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,
										incc_rec.hhcc_hash);

	/*-----------------------------------
	| cover required & qty available	|
	-----------------------------------*/
	cover = reviewPeriod + LeadWeeks + safetyStock;
	data [BY_WH].qtyAvailable = incc_rec.closing_stock -
					        incc_rec.committed -
							realCommitted - 
					        incc_rec.backorder - 
					        incc_rec.forward;

	/*-----------------------------------
	| on order & cover available		|
	-----------------------------------*/
	data [BY_WH].qtyOnOrder = incc_rec.on_order;
	data [BY_WH].qtyTotalCover = data [BY_WH].qtyAvailable +
					            incc_rec.on_order;

	/*-----------------------------------
	| cover required & nett requirement	|
	-----------------------------------*/
	data [BY_WH].qtyCoverRequired = cover;
	data [BY_WH].qtyCoverRequired *= data [BY_WH].weeksDemand;
	data [BY_WH].qtyNettRequired = data [BY_WH].qtyCoverRequired -
					           	  data [BY_WH].qtyTotalCover;
	if (data [BY_WH].qtyNettRequired < 0.00)
		data [BY_WH].qtyNettRequired = 0.00;
	/*-------------------------------
	| reorder qty recommended		|
	-------------------------------*/
	if (data [BY_WH].qtyNettRequired == 0.00)
		data [BY_WH].quantityRecommend = 0.00;
	else
		data [BY_WH].quantityRecommend = tri_max (data [BY_WH].qtyNettRequired,
					minimumOrder,
					normalOrder);

	quantityRecommend = data [BY_WH].quantityRecommend;

	return (EXIT_SUCCESS);
}

void
AllDisplay (void)
{
	/*-----------------------------------
	| display heading & other prompts	|
	-----------------------------------*/
	print_at (2,4,ML (mlLrpMess060));
	print_at (2,29,"-");

	box (0, 3, 130, 5);

	print_at (3,99,ML (mlLrpMess007));
	print_at (4,80,ML (mlLrpMess008));
	print_at (5,80,ML (mlLrpMess009));
	print_at (6,80,ML (mlLrpMess010));
	print_at (7,80,ML (mlLrpMess011));
	print_at (8,80,ML (mlLrpMess012));

	print_at (4,43,ML (mlLrpMess013));
	print_at (5,43,ML (mlLrpMess033));
	if (useMinStockWeeks)
		print_at (6,43,ML (mlLrpMess073));
	else
		print_at (6,43,ML (mlLrpMess015));

	move (43,7);
	line (33);

	print_at (8,43,ML (mlLrpMess016));
	print_at (4,5,ML (mlLrpMess017));

	print_at (5,5,ML (mlLrpMess018));
	print_at (6,5,ML (mlLrpMess019));

	print_at (7,5,ML (mlLrpMess034));
	print_at (8,5,ML (mlLrpMess020));

	move (0,21);
	line (132);

	move (0,22);
	cl_line ();
	if (byWhat == BY_CO)
	{
		print_at (22,0, ML (mlStdMess038), 
							comm_rec.co_no, comm_rec.co_name);
	}
	else if (byWhat == BY_BR)
	{
		print_at (22,0, ML (mlStdMess038), 
							comm_rec.co_no, comm_rec.co_name);
		print_at (22,40,ML (mlStdMess039), 
							comm_rec.est_no, comm_rec.est_name);
	} 
	else if (byWhat == BY_WH)
	{
		print_at (22,0, ML (mlStdMess038), 
							comm_rec.co_no, comm_rec.co_name);
		print_at (22,40,ML (mlStdMess039), 
							comm_rec.est_no, comm_rec.est_name);
		print_at (22,90,ML (mlStdMess099), 
							comm_rec.cc_no, comm_rec.cc_name);
	}
}
int
MonthlyGraph (void)
{
	monthlySelect	=	TRUE;
	return (ProcessGraph ());
}
int
DailyGraph (void)
{
	monthlySelect	=	FALSE;
	return (ProcessGraph ());
}

int
ProcessGraph (void)
{
	int		LRP_dayOfWeek	=	0;
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
				validMethods, 
				inmr_rec.hhbr_hash, 
				comm_rec.inv_date, 
				FALSE,
				36,
				(monthlySelect) ? LRP_PASSED_MONTH : LRP_PASSED_DAY,
				"14"
			);
	for (i = 0; i < 5; i++)
	{
		method_chk = 'A' + i - 1;
		if (i == 0)
			strcpy (local_rec.response [i], "Y");
		else
			strcpy (local_rec.response [i], "N");

		if (i == 1 && incc_rec.ff_method [0] == 'A')
			strcpy (local_rec.response [i], "H");
		if (i == 2 && incc_rec.ff_method [0] == 'B')
			strcpy (local_rec.response [i], "H");
		if (i == 3 && incc_rec.ff_method [0] == 'C')
			strcpy (local_rec.response [i], "H");
		if (i == 4 && incc_rec.ff_method [0] == 'D')
			strcpy (local_rec.response [i], "H");

		vars [i * 2].required = YES;
		if (i > 0 && strchr (LSA_methods, method_chk) == (char *) 0)
			vars [i * 2].required = NA;
	}
	init_ok = FALSE;

	move (0, 14);
	cl_end ();
	box (0, 11, 132, 6);
	print_at (11, 20, ML (mlLrpMess035));
	heading (1);
	scn_write (1);
	if (byWhat == BY_WH)
	{	
		if (incc_rec.ff_method [0] == 'A')
			print_at (13,5,"*");
		if (incc_rec.ff_method [0] == 'B')
			print_at (14,5,"*");
		if (incc_rec.ff_method [0] == 'C')
			print_at (15,5,"*");
		if (incc_rec.ff_method [0] == 'D')
			print_at (16,5,"*");
	}
	entry (1);

	scn_write (1);
	scn_display (1);
	edit (1);

	if (prog_exit || restart)
	{
		prog_exit	=	FALSE;
		restart		=	FALSE;
		return (ClearRedraw ());
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
	sprintf (pr_head, "Item: %s - %s (%s)",
								inmr_rec.item_no,
								inmr_rec.description,
								inmr_rec.sale_unit);
	strcpy (head_str, "");
	strcpy (gpx_ch_indx, "");
	for (i = 0; i < 5; i++)
	{
		if (local_rec.response [i][0] != 'N')
		{
			indx [j] = i;
			j++;
			if (strlen (head_str))
				strcat (head_str, " vs. ");
			strcat (head_str, headingNames [i]);
			if (local_rec.response [i][0] == 'H')
				strcat (gpx_ch_indx, "1");
			else
				strcat (gpx_ch_indx, "2");
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
	LRP_dayOfWeek	=	DayOfWeek (TodaysDate());
	
	for (i = 0; i < k; i++)
	{
		if (monthlySelect)
			gr_tits [i]	= mth_nms [ (37 + local_rec.future + i + LSA_mnth - k) % 12];
		else
		{
			if (LSA_WeekDay)
				gr_tits [i]	= weekDayNames [ (37 + local_rec.future + i + LRP_dayOfWeek - k) % 5];
			else
				gr_tits [i]	= fullDayNames [ (37 + local_rec.future + i + LRP_dayOfWeek - k) % 7];
		}
		gr_val [i]	= 
			LSA_result [indx [0]][36 + local_rec.future + i - k];
		if (j > 1)
			gr_val [i+k]	= 
			LSA_result [indx [1]][36 + local_rec.future + i - k];
		if (j > 2)
			gr_val [i+ (k*2)]	= 
			LSA_result [indx [2]][36 + local_rec.future + i - k];
		if (j > 3)
			gr_val [i+ (k*3)]	= 
			LSA_result [indx [3]][36 + local_rec.future + i - k];
		if (j > 4)
			gr_val [i+ (k*4)]	= 
			LSA_result [indx [4]][36 + local_rec.future + i - k];
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
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}
int
ProcessCostGraph (void)
{
	int		i;

	GraphFooter	=	0;
	for (i = 0; i < 8; i++)
	{
		costingInformation [0][i]	= 0.00;
		costingInformation [1][i]	= 0.00;
		costingInformation [2][i]	= 0.00;
		costingInformation [3][i]	= 0.00;
	}
	i	=	8;
	cc = FindIncf (incc_rec.hhwh_hash, FALSE, "r");
	while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
	{
		costingInformation [0][i]		= incfRec.fob_nor_cst;
		costingInformation [1][i]		= incfRec.frt_ins_cst;
		costingInformation [2][i]		= incfRec.lcost_load;
		costingInformation [3][i]		= incfRec.land_cst;

		i--;
		if (i < 0)
			break;

		cc = FindIncf (0L, FALSE, "r");
	}
	DisplayCosting (0, 0,  ML("LRP - FOB Cost"));
	DisplayCosting (1, 32, ML("LRP - FRT Cost (s)"));
	DisplayCosting (2, 64, ML("LRP - Other Cost (s)"));
	DisplayCosting (3, 96, ML("LRP - Land Cost."));
#ifndef GVISION
	PauseForKey (21, 52,ML (mlStdMess042),0);
#endif
	GraphFooter	=	1;
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

void
DisplayCosting (int	CostType, int Offset, char	*Header)
{
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [32],
			gpx_ch_indx [8],
			head_str [130],
			pr_head [130];
	int		i;

	double	gr_val [100];		/* Array to store result (s)	*/

	/*----------------------------------
	| Setup graphics window & headings |
	----------------------------------*/
	lp_x_off = 0;
	lp_y_off = 3;
	gr_win.x_posn = Offset;
	gr_win.y_posn = 3;
	gr_win.x_size = 30;
	gr_win.y_size = 15;

	/*-------------------------
	| Load data to be graphed |
	-------------------------*/
	sprintf (pr_head, "Item: %s - %s (%s)",
								inmr_rec.item_no,
								inmr_rec.description,
								inmr_rec.sale_unit);
	strcpy (gpx_ch_indx, "1212");
	gr_nam.pr_head = pr_head;
	gr_nam.heading = head_str;
	gr_nam.gpx_ch_indx = gpx_ch_indx;
	
	strcpy (head_str, Header);
	gr_nam.heading = head_str;

	gr_tits [0] = "1";
	gr_tits [1] = "2";
	gr_tits [2] = "3";
	gr_tits [3] = "4";
	gr_tits [4] = "5";
	gr_tits [5] = "6";
	gr_tits [6] = "7";
	gr_tits [7] = "8";
	for (i = 0; i < 8; i++)
		gr_val [i] = costingInformation [CostType][i];
	
	gr_nam.legends = gr_tits;

	GR_graph (&gr_win, GR_TYPE_BAR, 8, &gr_nam, gr_val, NULL);
}

int
FormulaStatistics (
 void)
{
	char	disp_str [200];
	int		cnt;
	char	*formatMask	= "%s   %s   |  %13.2f |  %13.2f |  %13.2f |  %13.2f |  %13.2f%s| %10.2f | %10.2f ^6";

	cnt = 	calc_LSA 
		  	 (
				validMethods, 
				inmr_rec.hhbr_hash, 
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
			 LSA_result [0][34], 
			 LSA_result [1][35], 	
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
			 LSA_result [0][34], 
			 LSA_result [2][35],
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
			 LSA_result [0][34], 
			 LSA_result [3][35],
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
		strcpy (disp_str, ML("NOTE:^1 * ^6 indicates a rejected line. This may have been caused by sales projection going below zero"));
		Dsp_saverec (disp_str);
		cc = find_rec (lrph, &lrph_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	ClearRedraw ();
	return (EXIT_SUCCESS);
}
int
heading (int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	if (clear_ok)
		clear ();

	move (0,1);
	line (132);

	if (byWhat == BY_CO)
		rv_pr (ML (mlLrpMess061), 46,	0, 1);
	if (byWhat == BY_BR)
		rv_pr (ML (mlLrpMess062), 46,	0, 1);
	if (byWhat == BY_WH)
		rv_pr (ML (mlLrpMess063), 46,	0, 1);

	if (display_ok)
	{
		AllDisplay ();
		display_ok	=	FALSE;
	}
	PrintData ();
	return (EXIT_SUCCESS);
}

int
spec_valid (int	field)
{
	if (LCHECK ("actual"))
	{
		if (local_rec.response [0][0] == 'H')
			strcpy (local_rec.responseDesc [0], "Highlight");
		else
			if (local_rec.response [0][0] == 'Y')
				strcpy (local_rec.responseDesc [0], "Yes");
			else
				strcpy (local_rec.responseDesc [0], "No");

		if (local_rec.response [0][0] != 'N')
			LcheckFuture ();
		display_field (field + 1);

		CheckDupResponce (0);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("linear"))
	{
		if (local_rec.response [1][0] == 'H')
			strcpy (local_rec.responseDesc [1], "Highlight");
		else
			if (local_rec.response [1][0] == 'Y')
				strcpy (local_rec.responseDesc [1], "Yes");
			else
				strcpy (local_rec.responseDesc [1], "No");

		if (local_rec.response [1][0] != 'N')
			LcheckFuture ();
		display_field (field + 1);
		CheckDupResponce (1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("snl_trn"))
	{
		if (local_rec.response [2][0] == 'H')
			strcpy (local_rec.responseDesc [2], "Highlight");
		else
			if (local_rec.response [2][0] == 'Y')
				strcpy (local_rec.responseDesc [2], "Yes");
			else
				strcpy (local_rec.responseDesc [2], "No");

		if (local_rec.response [2][0] != 'N')
			LcheckFuture ();
		CheckDupResponce (2);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("loc_lin"))
	{
		if (local_rec.response [3][0] == 'H')
			strcpy (local_rec.responseDesc [3], "Highlight");
		else
			if (local_rec.response [3][0] == 'Y')
				strcpy (local_rec.responseDesc [3], "Yes");
			else
				strcpy (local_rec.responseDesc [3], "No");

		if (local_rec.response [3][0] != 'N')
			LcheckFuture ();
		
		display_field (field + 1);
		CheckDupResponce (3);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("focusforecast"))
	{
		if (local_rec.response [4][0] == 'H')
			strcpy (local_rec.responseDesc [4], "Highlight");
		else
			if (local_rec.response [4][0] == 'Y')
				strcpy (local_rec.responseDesc [4], "Yes");
			else
				strcpy (local_rec.responseDesc [4], "No");

		if (local_rec.response [4][0] != 'N')
			LcheckFuture ();
		
		CheckDupResponce (4);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("future"))
	{
		if (local_rec.future < workMinimum)
		{
			sprintf (err_str,ML (mlLrpMess059),workMinimum);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*---------------------------
| prepare for future LCHECK |
---------------------------*/
void
LcheckFuture (void)
{
	int  	i,
		cnt,
		j = 0;
	for (i = 0; i < 5; i++)
	{
		if (local_rec.response [i][0] != 'N')
			j++;
	}
	cnt = 	calc_LSA 
			 (
				validMethods, 
				inmr_rec.hhbr_hash, 
				comm_rec.inv_date, 
				FALSE,
				36,
				(monthlySelect) ? LRP_PASSED_MONTH : LRP_PASSED_DAY,
				"1"
			);
	switch (j)
	{
		case	1:
			workMinimum = 12;
			break;
		case	2:
			workMinimum = (cnt > 29) ? 0 : 
				 ((cnt < 18) ? 12 : 30 - cnt);
			break;
		case	3:
			workMinimum = (cnt > 19) ? 0 : 
				 ((cnt < 8) ? 12 : 20 - cnt);
			break;
		case	4:
			workMinimum = (cnt > 14) ? 0 : 
				 ((cnt < 3) ? 12 : 15 - cnt);
			break;
		case	5:
			workMinimum = 0;
			break;
		default:
			return;
	}
	if (workMinimum == 12)
	{
		local_rec.future = 12;
		vars [label ("future")].required = NA;	
	}
	else
		vars [label ("future")].required = YES;	
		
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.			        |
| Return 0 if none found.			            |
===============================================*/
float	
GetLeadDate
 (
	long	HHIS_HASH,
	long	ORD_DATE
)
{
	float	days;

	inld_rec.hhis_hash	=	HHIS_HASH;
	inld_rec.ord_date 	=	ORD_DATE;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	if (cc || inld_rec.hhis_hash != HHIS_HASH)
		return ((float) 0.00);

	days = inld_rec.sup_date - ORD_DATE;
	return (days);
}

float
RoundMultiple 
 (
	float	orderQuantity,
	char	*roundType,
	float	orderMultiple,
	float	minimumQuantity
)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (orderQuantity == 0.00)
		return (0.00);

	if (orderMultiple == 0.00)
		return ((orderQuantity < minimumQuantity) ? minimumQuantity : orderQuantity);

	orderQuantity -= minimumQuantity;
	if (orderQuantity < 0.00)
		orderQuantity = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (orderQuantity / orderMultiple);
	if (ceil (wrk_qty) == wrk_qty)
		return (orderQuantity + minimumQuantity);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (roundType [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double) (orderQuantity / orderMultiple);
		wrk_qty = ceil (wrk_qty);
		orderQuantity = (float) (wrk_qty * orderMultiple);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double) (orderQuantity / orderMultiple);
		wrk_qty = floor (wrk_qty);
		orderQuantity = (float) (wrk_qty * orderMultiple);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double) orderQuantity;
		wrk_qty = (up_qty / (double)orderMultiple);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * orderMultiple);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double) orderQuantity;
		wrk_qty = (down_qty / (double) orderMultiple);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * orderMultiple);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double) orderQuantity) <= ((double) orderQuantity - down_qty))
			orderQuantity = (float) up_qty;
		else
			orderQuantity = (float) down_qty;

		break;

	default:
		break;
	}

	return (minimumQuantity + orderQuantity);
}


void
CheckDupResponce 
 (
	int		ResNo
)
{
	int		i;

	if (local_rec.response [ResNo][0] == 'N')
		return;

	for (i = 0; i < 5; i++)
	{
		if (local_rec.response [i][0] == 'Y' && i != ResNo &&
			local_rec.response [i][0] == local_rec.response [ResNo][0])
		{
			strcpy (local_rec.response [i], "N");
			strcpy (local_rec.responseDesc [i], "No");
			DSP_FLD (GraphPrompts [i]);
			DSP_FLD (GraphDescs [i]);
		}
		if (local_rec.response [i][0] == 'H' && i != ResNo &&
			local_rec.response [i][0] == local_rec.response [ResNo][0])
		{
			strcpy (local_rec.response [i], "N");
			strcpy (local_rec.responseDesc [i], "No");
			DSP_FLD (GraphPrompts [i]);
			DSP_FLD (GraphDescs [i]);
		}
	}
}

int
LoadStockDisplay (void)
{
	sprintf (err_str, "sk_alldisp %s %010ld", (byWhat == BY_CO) ? "C" : "W",
											   inmr_rec.hhbr_hash); 
	sys_exec (err_str);
	swide ();
	display_ok = TRUE;
	ClearRedraw ();
	return (EXIT_SUCCESS);
}

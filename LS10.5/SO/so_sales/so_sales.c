/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sales.c,v 5.4 2002/07/17 09:58:10 scott Exp $
|  Program Name  : (so_sales.c) 
|  Program Desc  : (Print Sales Order Analysis)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 07/11/89         |
|---------------------------------------------------------------------|
| $Log: so_sales.c,v $
| Revision 5.4  2002/07/17 09:58:10  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:21:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:51:58  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:04  scott
| Update - LS10.5
|
| Revision 4.1  2001/03/16 06:37:58  scott
| Updated to fix display prompts for LS10-GUI
| Updated to clean program and convert to use app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sales.c,v $", 
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sales/so_sales.c,v 5.4 2002/07/17 09:58:10 scott Exp $";

#include <time.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pr_format3.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <Costing.h>

#define	MONTH		1
#define	WEEK		2
#define	DAY			3
#define	MOVEMENT	4

#define	CUST		1
#define	ITEM		2
#define	SMAN		3

#define	MAXWEEK		6
#define	BY_BR		 (companyBranch [0] == 'B')
#define	DETAIL		 (detailedSummary [0] == 'D')
#define	BY_ITEM		 (sortType == ITEM)

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct sosaRecord	sosa_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct exsfRecord	exsf_rec;
struct ccmrRecord	ccmr_rec;
struct inccRecord	incc_rec;

	int		envVarDbCo		= 0,
			reportSelType	= 0,
			sortType		= 0,
			printed 		= 0,
			firstTime 		= 1,
			foundData 		= 0,
			weekly 			= 0,
			numberLines		= 1,
			envVarDbMcurr 	= FALSE,
			currentWeek 	= 0;

	char	branchNumber 	[3],
			detailedSummary [2],	
			companyBranch 	[2],	
			moduleDate 		[11],
			currentMonth 	[3],
			currentYear 	[5],
			prevBr 			[3],
			currBr 			[3],
			previousAcronym [10],
			currentAcronym 	[10],
			previousItem 	[17],
			currentItem 	[17],
			dataString 		[256],	
			sortString 		[256],
			noPrompt 		[11],
			yesPrompt 		[11];

	long	startDate			=	0L,
			endDate				=	0L,
			previousDate		=	0L,
			currentDate			=	0L,
			previousHash [2]	=	{0L,0L},
			currentHash [2]		=	{0L,0L};


	double	m_val [5]		=	{0,0,0,0,0},
			m_exval [5]		=	{0,0,0,0,0},
			m_cost [5]		=	{0,0,0,0,0};

	float	m_qty [5]		=	{0,0,0,0,0},
			m_margin		=	0;

	struct {
		long	start;
		long	end;
	} week_tab [MAXWEEK];

	struct {
		char	*rep_by;
		char	*sort_by;
	} rep_desc [4] = {
		{"Month", "Customer"}, 
		{"Week", "Item No"}, 
		{"Day", "Salesman"}, 
		{"Movements", "       "}, 
	};

	FILE	*fin;
	FILE	*fout;
	FILE	*fsort;

	extern	int	TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	reportType [9] [2];
	char 	reportDesc [9] [12];
	int  	printerNo;
	char 	back [2];
	char 	backDesc [6];
	char 	onite [2];
	char 	oniteDesc [6];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "sel1", 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Order Analysis By Month   ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [0]}, 
	{1, LIN, "sel1Desc", 4, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [0]}, 
	{1, LIN, "sel2", 5, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Order Analysis By Week    ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [1]}, 
	{1, LIN, "sel2Desc", 5, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [1]}, 
	{1, LIN, "sel3", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Order Analysis By Day     ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [2]}, 
	{1, LIN, "sel3Desc", 6, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [2]}, 
	{1, LIN, "sel4", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Order Analysis Movements  ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [3]}, 
	{1, LIN, "sel4Desc", 7, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [3]}, 

	{1, LIN, "sel5", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Sort By Customer          ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [4]}, 
	{1, LIN, "sel5Desc", 9, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [4]}, 
	{1, LIN, "sel6", 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Sort By Item No           ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [5]}, 
	{1, LIN, "sel6Desc", 10, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [5]}, 
	{1, LIN, "sel7", 11, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Sort By Salesman          ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.reportType [6]}, 
	{1, LIN, "sel7Desc", 11, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [6]}, 
	{1, LIN, "detailedSummary", 13, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Detailed / Summary        ", "Enter D(etailed) or S(ummary). ", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.reportType [7]}, 
	{1, LIN, "detailedSummaryDesc", 13, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [7]}, 
	{1, LIN, "companyBranch", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "B", "Report By Company/Branch  ", "Enter C(ompany) or B(ranch). ", 
		YES, NO, JUSTLEFT, "CB", "", local_rec.reportType [8]}, 
	{1, LIN, "companyBranchDesc", 15, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportDesc [8]}, 
	{1, LIN, "printerNo", 17, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number            ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 18, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background Y/N            ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 18, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite", 18, 40, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight Y/N             ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 18, 70, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
double 	DoubleValue 		(char *);
double 	FindCost 			(long);
float 	FloatValue 			(char *);
int  	CheckBreakOne 		(void);
int  	CheckBreakTwo 		(void);
int  	GetWarehouse 		(long);
int  	ValidBranch 		(char *);
int  	check_page 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
long 	DateValue 			(char *);
long 	LongValue 			(char *);
void 	CloseDB 			(void);
void 	FindValidDate 		(void);
void 	HeadingOutput 		(void);
void 	InitArray 			(void);
void 	OpenDB 				(void);
void 	PrintLine 			(void);
void 	PrintTotal 			(char *);
void 	PrintWkBegin 		(void);
void 	ProcessCData 		(int, int);
void 	ProcessSorted 		(void);
void 	ReadSosa 			(void);
void 	RunProgram 			(char *);
void 	SetBreak 			(char *);
void 	SetupDefault 		(void);
void 	StoreData 			(void);
void 	SumSales 			(char *, int);
void 	shutdown_prog 		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr = chk_env ("DB_MCURR");

	TruePosition	=	TRUE;
	
	if (sptr)
		envVarDbMcurr = atoi (sptr);

	if (argc != 1 && argc != 6)
	{
		print_at (0, 0, mlSoMess765, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	envVarDbCo = atoi (get_env ("DB_CO"));

	OpenDB ();

	strcpy (branchNumber, (!envVarDbCo) ? " 0" : comm_rec.est_no);

	if (argc == 6)
	{
		reportSelType 			= atoi (argv [1]);
		sortType 				= atoi (argv [2]);
		sprintf (detailedSummary, "%-1.1s", argv [3]);
		sprintf (companyBranch, "%-1.1s", argv [4]);
		local_rec.printerNo 	= atoi (argv [5]);

		dsp_screen ("Processing : Printing Sales Order Analysis.", comm_rec.co_no, comm_rec.co_name);

		ReadSosa ();

		HeadingOutput ();
		if (foundData)
			ProcessSorted ();
		fprintf (fout, ".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*=====================
	| Reset control flags |
	=====================*/
   	search_ok 	= TRUE;
   	entry_exit 	= TRUE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	init_vars (1);	
	SetupDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
	rset_tty ();

	if (!restart) 
		RunProgram (argv [0]);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetupDefault (
 void)
{ 

	local_rec.printerNo = 1;
	strcpy (local_rec.back, 		"N");
	strcpy (local_rec.backDesc, 	noPrompt);
	strcpy (local_rec.onite, 		"N");
	strcpy (local_rec.oniteDesc, 	noPrompt);
	strcpy (local_rec.reportType [0], "Y");
	strcpy (local_rec.reportType [1], "N");
	strcpy (local_rec.reportType [2], "N");
	strcpy (local_rec.reportType [3], "N");
	strcpy (local_rec.reportType [4], "Y");
	strcpy (local_rec.reportType [5], "N");
	strcpy (local_rec.reportType [6], "N");
	strcpy (local_rec.reportType [7], "S");
	strcpy (local_rec.reportType [8], "B");

	strcpy (local_rec.reportDesc [0], yesPrompt);
	strcpy (local_rec.reportDesc [1], noPrompt);
	strcpy (local_rec.reportDesc [2], noPrompt);
	strcpy (local_rec.reportDesc [3], noPrompt);
	strcpy (local_rec.reportDesc [4], yesPrompt);
	strcpy (local_rec.reportDesc [5], noPrompt);
	strcpy (local_rec.reportDesc [6], noPrompt);
	strcpy (local_rec.reportDesc [7], ML ("Summary"));
	strcpy (local_rec.reportDesc [8], ML ("Branch"));
	reportSelType = 1;
	sortType = 1;
}

void
RunProgram (
 char *prog_name)
{
	char	lp_str [3];	
	char	reportSelString [3];	
	char	sortTypeString [3];
	char	detailedSummary [2];	
	char	companyBranch [2];	

	rset_tty ();

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	sprintf (lp_str, "%2d", local_rec.printerNo);
	sprintf (reportSelString, "%2d", reportSelType);
	sprintf (sortTypeString, "%2d", sortType);
	sprintf (detailedSummary, "%-1.1s", local_rec.reportType [7]);
	sprintf (companyBranch, "%-1.1s", local_rec.reportType [8]);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onite [0] == 'Y')
	{ 
		if (fork () == 0)
		{
			execlp 
			(
				"ONIGHT", 
				"ONIGHT", 
				prog_name, 
				reportSelString, 
				sortTypeString, 
				detailedSummary, 
				companyBranch, 
				lp_str, 
				ML (mlSoMess398), (char *)0
			);
		}
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				prog_name, 
				prog_name, 
				reportSelString, 
				sortTypeString, 
				detailedSummary, 
				companyBranch, 
				lp_str, 
				(char *)0
			);
		}
	}
	else 
	{
		execlp 
		(
			prog_name, 
			prog_name, 
			reportSelString, 
			sortTypeString, 
			detailedSummary, 
			companyBranch, 
			lp_str, 
			(char *)0
		);
	}
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

int
spec_valid (
 int field)
{
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("sel1"))
	{
		if (local_rec.reportType [0][0] == 'Y')
			strcpy (local_rec.reportDesc [0], yesPrompt);
		else
			strcpy (local_rec.reportDesc [0], noPrompt);

		strcpy (local_rec.reportType [1], "N");
		strcpy (local_rec.reportDesc [1], noPrompt);
		strcpy (local_rec.reportType [2], "N");
		strcpy (local_rec.reportDesc [2], noPrompt);
		strcpy (local_rec.reportType [3], "N");
		strcpy (local_rec.reportDesc [3], noPrompt);

		DSP_FLD ("sel1");
		DSP_FLD ("sel1Desc");
		DSP_FLD ("sel2");
		DSP_FLD ("sel2Desc");
		DSP_FLD ("sel3");
		DSP_FLD ("sel3Desc");
		DSP_FLD ("sel4");
		DSP_FLD ("sel4Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("sel2"))
	{
		if (local_rec.reportType [1][0] == 'Y')
			strcpy (local_rec.reportDesc [1], yesPrompt);
		else
			strcpy (local_rec.reportDesc [1], noPrompt);

		strcpy (local_rec.reportType [0], "N");
		strcpy (local_rec.reportDesc [0], noPrompt);
		strcpy (local_rec.reportType [2], "N");
		strcpy (local_rec.reportDesc [2], noPrompt);
		strcpy (local_rec.reportType [3], "N");
		strcpy (local_rec.reportDesc [3], noPrompt);

		DSP_FLD ("sel1");
		DSP_FLD ("sel1Desc");
		DSP_FLD ("sel2");
		DSP_FLD ("sel2Desc");
		DSP_FLD ("sel3");
		DSP_FLD ("sel3Desc");
		DSP_FLD ("sel4");
		DSP_FLD ("sel4Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("sel3"))
	{
		if (local_rec.reportType [2][0] == 'Y')
			strcpy (local_rec.reportDesc [2], yesPrompt);
		else
			strcpy (local_rec.reportDesc [2], noPrompt);

		strcpy (local_rec.reportType [0], "N");
		strcpy (local_rec.reportDesc [0], noPrompt);
		strcpy (local_rec.reportType [1], "N");
		strcpy (local_rec.reportDesc [1], noPrompt);
		strcpy (local_rec.reportType [3], "N");
		strcpy (local_rec.reportDesc [3], noPrompt);

		DSP_FLD ("sel1");
		DSP_FLD ("sel1Desc");
		DSP_FLD ("sel2");
		DSP_FLD ("sel2Desc");
		DSP_FLD ("sel3");
		DSP_FLD ("sel3Desc");
		DSP_FLD ("sel4");
		DSP_FLD ("sel4Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("sel4"))
	{
		if (local_rec.reportType [3][0] == 'Y')
			strcpy (local_rec.reportDesc [3], yesPrompt);
		else
			strcpy (local_rec.reportDesc [3], noPrompt);

		strcpy (local_rec.reportType [0], "N");
		strcpy (local_rec.reportDesc [0], noPrompt);
		strcpy (local_rec.reportType [1], "N");
		strcpy (local_rec.reportDesc [1], noPrompt);
		strcpy (local_rec.reportType [2], "N");
		strcpy (local_rec.reportDesc [2], noPrompt);

		DSP_FLD ("sel1");
		DSP_FLD ("sel1Desc");
		DSP_FLD ("sel2");
		DSP_FLD ("sel2Desc");
		DSP_FLD ("sel3");
		DSP_FLD ("sel3Desc");
		DSP_FLD ("sel4");
		DSP_FLD ("sel4Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 5. |
	-----------------------------------*/
	if (LCHECK ("sel5"))
	{
		if (local_rec.reportType [4][0] == 'Y')
			strcpy (local_rec.reportDesc [4], yesPrompt);
		else
			strcpy (local_rec.reportDesc [4], noPrompt);

		strcpy (local_rec.reportType [5], "N");
		strcpy (local_rec.reportDesc [5], noPrompt);
		strcpy (local_rec.reportType [6], "N");
		strcpy (local_rec.reportDesc [6], noPrompt);

		DSP_FLD ("sel5");
		DSP_FLD ("sel5Desc");
		DSP_FLD ("sel6");
		DSP_FLD ("sel6Desc");
		DSP_FLD ("sel7");
		DSP_FLD ("sel7Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 6. |
	-----------------------------------*/
	if (LCHECK ("sel6"))
	{
		if (local_rec.reportType [5][0] == 'Y')
			strcpy (local_rec.reportDesc [5], yesPrompt);
		else
			strcpy (local_rec.reportDesc [5], noPrompt);

		strcpy (local_rec.reportType [4], "N");
		strcpy (local_rec.reportDesc [4], noPrompt);
		strcpy (local_rec.reportType [6], "N");
		strcpy (local_rec.reportDesc [6], noPrompt);

		DSP_FLD ("sel5");
		DSP_FLD ("sel5Desc");
		DSP_FLD ("sel6");
		DSP_FLD ("sel6Desc");
		DSP_FLD ("sel7");
		DSP_FLD ("sel7Desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 7. |
	-----------------------------------*/
	if (LCHECK ("sel7"))
	{
		if (local_rec.reportType [6][0] == 'Y')
			strcpy (local_rec.reportDesc [6], yesPrompt);
		else
			strcpy (local_rec.reportDesc [6], noPrompt);

		strcpy (local_rec.reportType [4], "N");
		strcpy (local_rec.reportDesc [4], noPrompt);
		strcpy (local_rec.reportType [5], "N");
		strcpy (local_rec.reportDesc [5], noPrompt);

		DSP_FLD ("sel5");
		DSP_FLD ("sel5Desc");
		DSP_FLD ("sel6");
		DSP_FLD ("sel6Desc");
		DSP_FLD ("sel7");
		DSP_FLD ("sel7Desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("detailedSummary"))
	{
		if (local_rec.reportType [7][0] == 'S')
			strcpy (local_rec.reportDesc [7], ML ("Summary"));
		else
			strcpy (local_rec.reportDesc [7], ML ("Detailed"));

		DSP_FLD ("detailedSummaryDesc");
	}

	if (LCHECK ("companyBranch"))
	{
		if (local_rec.reportType [8][0] == 'C')
			strcpy (local_rec.reportDesc [8], ML ("Company"));
		else
			strcpy (local_rec.reportDesc [8], ML ("Branch"));

		DSP_FLD ("companyBranchDesc");
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'N')
			strcpy (local_rec.backDesc, noPrompt);
		else
			strcpy (local_rec.backDesc, yesPrompt);

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'N')
			strcpy (local_rec.oniteDesc, noPrompt);
		else
			strcpy (local_rec.oniteDesc, yesPrompt);

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
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
		rv_pr (ML (mlSoMess181), 25, 0, 1);

		line_at (1, 0,80);
		move (1, input_row);
		box (0, 3, 80, 15);
		line_at (8, 1,79);
		line_at (12,1,79);
		line_at (14,1,79);
		line_at (16,1,79);
		line_at (20,0,80);

		us_pr (ML (mlSoMess400), 5, 3, 1);
		us_pr (ML (mlSoMess402), 5, 8, 1);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sosa, sosa_list, SOSA_NO_FIELDS, "sosa_id_no1");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");

	strcpy (noPrompt, ML ("NO"));
	strcpy (yesPrompt, ML ("YES"));
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sosa);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (incc);
	CloseCosting ();
	abc_dbclose ("data");
}

void
FindValidDate (void)
{
	int		numberWeeks;
	int	i;
	int	mon;
	int	yr;
	
	int		s_mth;
	long	wk_date;
	int		dmy [3];
	int	day_of_wk = 0;

	strcpy (moduleDate, DateToString (comm_rec.dbt_date));
		
	DateToDMY (comm_rec.dbt_date, &dmy [0], &dmy [1], &dmy [2]);
	sprintf (currentMonth, "%02d", dmy [1]);
	sprintf (currentYear, "%d", dmy [2]); 

	/*-----------------------
	| Initialise week table	|
	-----------------------*/
	for (i = 0; i < MAXWEEK; i++)
	{
		week_tab [i].start = 0L;
		week_tab [i].end   = 0L;
	}

	switch (reportSelType)
	{
	case	MONTH:
		startDate	=	MonthStart (comm_rec.dbt_date);
		endDate 	= 	comm_rec.dbt_date;

		day_of_wk = (int) (startDate % 7L);
		week_tab [0].start = startDate;
		week_tab [0].end = startDate + (long) (7 - day_of_wk);

		for (i = 0; i < MAXWEEK && (week_tab [i].end < endDate); i++)
		{
			week_tab [i + 1].start = week_tab [i].end + 1L;
			if (week_tab [i + 1].start > endDate)
			{
				week_tab [i+1].start = endDate;
				week_tab [i+1].end = endDate;
				i++; 
				break;
			}

			week_tab [i + 1].end   = week_tab [i + 1].start + 6L;
			if (week_tab [i + 1].end > endDate)
			{
				week_tab [i+1].end = endDate;
				i++; 
				break;
			}
		}

		numberWeeks =  i + 1;
		break;

	case	WEEK:
		day_of_wk = (int) (comm_rec.dbt_date % 7L);

		endDate = StringToDate (moduleDate);
		startDate = StringToDate (moduleDate);

		if (day_of_wk == 0)
			day_of_wk += 7;

		while (day_of_wk > 1)
		{
			day_of_wk--;
			startDate--;
			DateToDMY (startDate, NULL, &s_mth, NULL);
			if (s_mth != atoi (currentMonth))
			{
				startDate++;
				break;
			}
		}
		week_tab [0].start = startDate;
		week_tab [0].end = endDate;
		break;

	case	DAY:
		startDate = comm_rec.dbt_date;
		endDate = comm_rec.dbt_date;
		break;

	case	MOVEMENT:
		mon = atoi (currentMonth);
		yr = atoi (currentYear);
		if (mon < comm_rec.fiscal)
			yr--;

		dmy [0] = 1;
		dmy [2] = yr;

		startDate 	= DMYToDate (dmy [0], dmy [1], dmy [2]);
		endDate 	= comm_rec.dbt_date;

		/*-----------------------------------------------
		| Store the week start-end for current month.	|
		-----------------------------------------------*/
		dmy [0] = 1;
		dmy [1] = atoi (currentMonth);
		dmy [2] = atoi (currentYear);

		wk_date 	= DMYToDate (dmy [0], dmy [1], dmy [2]);

		day_of_wk = (int) (wk_date % 7L);
		week_tab [0].start = wk_date;
		week_tab [0].end = wk_date + (long) (7 - day_of_wk);

		for (i = 0; i < MAXWEEK && week_tab [i].end < endDate; i++)
		{
			week_tab [i + 1].start = week_tab [i].end + 1L;
			if (week_tab [i + 1].start > endDate)
			{
				week_tab [i+1].start = endDate;
				week_tab [i+1].end = endDate;
				i++; 
				break;
			}

			week_tab [i + 1].end   = week_tab [i + 1].start + 6L;
			if (week_tab [i + 1].end > endDate)
			{
				week_tab [i+1].end = endDate;
				i++; 
				break;
			}
		}

		numberWeeks =  i + 1;
		break;
	}
}

void
ReadSosa (void)
{
	int	firstTime = TRUE;

	abc_selfield (sosa, (BY_BR) ? "sosa_id_no1" : "sosa_id_no2");

	FindValidDate ();
	strcpy (sosa_rec.co_no, comm_rec.co_no);
	strcpy (sosa_rec.br_no, (BY_BR) ? branchNumber : "  ");
	sosa_rec.date = startDate;
	
	cc = find_rec (sosa, &sosa_rec, GTEQ, "r");
	while (!cc && !strcmp (sosa_rec.co_no, comm_rec.co_no) && 
				ValidBranch (sosa_rec.br_no) && sosa_rec.date <= endDate)
	{
		if (firstTime)
		{
			fsort = sort_open ("so_sales");
			firstTime = FALSE;
			foundData = TRUE;
		}
		StoreData ();
		cc = find_rec (sosa, &sosa_rec, NEXT, "r");
	}
}

int
ValidBranch (
	char	*branchNo)
{
	if (BY_BR && strncmp (branchNo, branchNumber, 2))
		return (EXIT_SUCCESS);
	return (EXIT_FAILURE);
}

void
StoreData (void)
{
	inmr_rec.hhbr_hash	=	sosa_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return;

	cumr_rec.hhcu_hash	=	sosa_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return;

	switch (sortType)
	{
		case	CUST :
			sprintf 
			(
				dataString, 
				"%-2.2s %10ld %-16.16s %-16.16s %10.2f %10.2f %10.2f %10.2f %06ld %06ld\n", 
				 (BY_BR) ? sosa_rec.br_no : "  ", 
				sosa_rec.date, 
				cumr_rec.dbt_acronym, 
				inmr_rec.item_no, 
				sosa_rec.qty, 
				sosa_rec.value, 
				sosa_rec.cost, 
				sosa_rec.disc, 
				sosa_rec.hhbr_hash, 
				sosa_rec.hhcu_hash);

			break;

		case	ITEM :
			sprintf 
			(
				dataString, 
				"%-2.2s %10ld %-16.16s %-16.16s %10.2f %10.2f %10.2f %10.2f %06ld %06ld\n", 
				 (BY_BR) ? sosa_rec.br_no : "  ", 
				sosa_rec.date, 
				inmr_rec.item_no, 
				cumr_rec.dbt_acronym, 
				sosa_rec.qty, 
				sosa_rec.value, 
				sosa_rec.cost, 
				sosa_rec.disc, 
				sosa_rec.hhbr_hash, 
				sosa_rec.hhcu_hash);
			break;

		case	SMAN :
			sprintf 
			(
				dataString, 
				"%-2.2s %10ld %-16.16s %-16.16s %10.2f %10.2f %10.2f %10.2f %06ld %06ld\n", 
				 (BY_BR) ? sosa_rec.br_no : "  ", 
				sosa_rec.date, 
				sosa_rec.sman_no, 
				cumr_rec.dbt_acronym, 
				sosa_rec.qty, 
				sosa_rec.value, 
				sosa_rec.cost, 
				sosa_rec.disc, 
				sosa_rec.hhbr_hash, 
				sosa_rec.hhcu_hash);
			break;
	}
	sort_save (fsort, dataString);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/
	if ((fin = pr_open ("so_sales.p")) == NULL)
		file_err (errno, "so_sales.p", "pr_open");

	if ((fout = popen ("pformat", "w")) == NULL)
		file_err (errno, "pformat", "popen");

	if (BY_BR)
	{
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, branchNumber);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			sprintf (esmr_rec.est_name, "%-40.40s", " ");
		abc_fclose (esmr);
	}

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	if (BY_BR)
		fprintf (fout, ".EBranch %s %s\n", branchNumber, clip (esmr_rec.est_name));
	else
		fprintf (fout, ".B1\n");
	fprintf (fout, ".EReport Selection by %-s Sorted By %-s\n", rep_desc [reportSelType - 1].rep_by, rep_desc [sortType - 1].sort_by);

	if (envVarDbMcurr)
		fprintf (fout, ".EAll Values Are In Local Currency\n");
	else
		fprintf (fout, ".B1\n");

	if (reportSelType == DAY)
		fprintf (fout, ".EOrders for %s", SystemTime ());
	else
		fprintf (fout, ".B1\n");
	pr_format (fin, fout, (DETAIL ? "LINE0" : "LINE_0S"), 0, 0);
	pr_format (fin, fout, (DETAIL ? "RULEOFF" : "RULEOFF_S"), 0, 0);

	switch (sortType)
	{
	case	CUST:
		pr_format (fin, fout, (DETAIL ? "HEADER1" : "HEADER1_S"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "HEADER2" : "HEADER2_S"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "LINE1" : "LINE_1S"), 0, 0);
		break;

	case	ITEM:
		pr_format (fin, fout, (DETAIL ? "HD_ITEM1" : "HD_ITEM_S1"), 1, "  ITEM");
		pr_format (fin, fout, (DETAIL ? "HD_ITEM1" : "HD_ITEM_S1"), 2, "ITEM");
		pr_format (fin, fout, (DETAIL ? "HD_ITEM2" : "HD_ITEM_S2"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "LINE_ITEM" : "LINE_ITEMS"), 0, 0);
		break;

	case	SMAN:
		pr_format (fin, fout, (DETAIL ? "HD_ITEM1" : "HD_ITEM_S1"), 1, "SALESMAN");
		pr_format (fin, fout, (DETAIL ? "HD_ITEM1" : "HD_ITEM_S1"), 2, "SALESMAN");
		pr_format (fin, fout, (DETAIL ? "HD_ITEM2" : "HD_ITEM_S2"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "LINE_ITEM" : "LINE_ITEMS"), 0, 0);
		break;

	}
	fflush (fout);
}

void
ProcessSorted (
 void)
{
	char	*sptr;
	int	print_type;

	InitArray ();
	printed = FALSE;
	firstTime = TRUE;
	if (reportSelType != DAY)
		currentWeek = 0;

	if (sortType == SMAN)
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	fsort = sort_sort (fsort, "so_sales");
	sptr = sort_read (fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf (sortString, "%-103.103s", sptr);

		SetBreak (sortString);

		switch (sortType)
		{
		case	CUST:
		case SMAN:
			dsp_process ("Customer No :", currentAcronym);
			break;

		case ITEM:
			dsp_process ("Item No :", currentItem);
			break;

		}

		if (firstTime)
		{
			strcpy (prevBr, currBr);
			previousDate = currentDate;

			switch (sortType)
			{
			case CUST:
			case ITEM:
				strcpy (previousAcronym, currentAcronym);
				strcpy (previousItem, currentItem);
				break;

			case SMAN:
				strcpy (previousAcronym, currentAcronym);
				sprintf (previousItem, "%-2.2s", currentItem);
				break;
			}

			previousHash [0] = currentHash [0];
			previousHash [1] = currentHash [1];
		}

		print_type = CheckBreakOne ();

		ProcessCData (print_type, firstTime);

		firstTime = 0;

		if (print_type == 0)
			SumSales (sptr + 46, 1);
		else
			SumSales (sptr + 46, 0);

		strcpy (prevBr, currBr);
		previousDate = currentDate;
		strcpy (previousAcronym, currentAcronym);
		if (sortType == SMAN)
			sprintf (previousItem, "%-2.2s", currentItem);
		else
			strcpy (previousItem, currentItem);
		previousHash [0] = currentHash [0];
		previousHash [1] = currentHash [1];

		sptr = sort_read (fsort);
	}
	if (printed)
	{
		PrintLine ();

		numberLines++;

		switch (sortType)
		{
		case	CUST:
			if (DETAIL)
				PrintTotal ("C");
			break;

		case	ITEM:
			if (DETAIL)
				PrintTotal ("I");
			break;

		case	SMAN:
			if (DETAIL)
				PrintTotal ("S");
			break;

		}
		if (reportSelType != DAY)
		{
			currentWeek++;
			PrintTotal ("W");
		}
	}
	PrintTotal ("E");
	sort_delete (fsort, "so_sales");
	if (sortType == SMAN)
		abc_fclose (exsf);
	abc_fclose (ccmr);
}

void
SetBreak (
 char *sortString)
{
	sprintf (currBr, "%-2.2s", sortString);
	currentDate = DateValue (sortString + 3);

	switch (sortType)
	{
	case	CUST:
		sprintf (currentAcronym, "%-9.9s", sortString + 12);
		sprintf (currentItem, "%-16.16s", sortString + 29);
		break;

	case	ITEM:
		sprintf (currentItem, "%-16.16s", sortString + 12);
		sprintf (currentAcronym, "%-9.9s", sortString + 29);
		break;

	case	SMAN:
		sprintf (currentItem, "%-2.2s", sortString + 12);
		sprintf (currentAcronym, "%-9.9s", sortString + 29);
		break;

	}
	currentHash [0] = LongValue (sortString + 90);
	currentHash [1] = LongValue (sortString + 97);
}

int
CheckBreakOne (void)
{
	char	currentMonth [3];
	char	dbt_mth [3];
	char	prev_mth [3];
	int		a_mth, 
			b_mth, 
			c_mth;

	/*------------------------------
	| Extra check for week / month |
	------------------------------*/
	switch (reportSelType)
	{
	case	MONTH:
		if (currentDate > week_tab [currentWeek].end)
		{
			currentWeek++;
			return (4);
		}
		break;

	case	MOVEMENT:

		DateToDMY (currentDate, NULL, &a_mth, NULL);
		sprintf (currentMonth, "%02d", a_mth);  

		/*---------------------------------------
		| If different month, print total for mth|
		| Otherwise add to monthly totals.      |
		---------------------------------------*/
		DateToDMY (previousDate, NULL, &b_mth, NULL);
		sprintf (prev_mth, "%02d", b_mth);  

		if (atoi (currentMonth) != atoi (prev_mth)) 
		{
			DateToDMY (comm_rec.dbt_date, NULL, &c_mth, NULL);
			sprintf (dbt_mth, "%d", c_mth); 
			/*---------------------------------------
			| Print as per monthly for current month|
			---------------------------------------*/
			if (atoi (currentMonth) == atoi (dbt_mth) && weekly == 0) 
			{
				currentWeek = 0;
				weekly = 1;
			}
			return (6);
		}

		if (atoi (currentMonth) == atoi (dbt_mth) && weekly == 0) 
		{
			currentWeek = 0;
			weekly = 1;
		}

		if (currentDate > week_tab [currentWeek].end)
		{
			currentWeek++;
			return (4);
		}
		break;

	default:
		break;
	}

	cc = CheckBreakTwo ();
	return (cc);
}

int
CheckBreakTwo (void)
{
	switch (sortType)
	{
	case	CUST:
		if (strcmp (currentAcronym, previousAcronym))
			return (2);
		if (DETAIL && strcmp (currentItem, previousItem))
		{
			numberLines++;
			return (EXIT_FAILURE);
		}
		break;

	case	ITEM:
		if (strcmp (currentItem, previousItem))
			return (3);
		if (DETAIL && strcmp (currentAcronym, previousAcronym))
		{
			numberLines++;
			return (EXIT_FAILURE);
		}
		break;

	case	SMAN:
		if (strncmp (currentItem, previousItem, 2))
			return (5);
		if (DETAIL && strcmp (currentAcronym, previousAcronym))
		{
			numberLines++;
			return (EXIT_FAILURE);
		}
		break;

	}
	if (currentDate != previousDate)
	{
		numberLines++;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
ProcessCData (
	int		print_type, 
	int		firstTime)
{
	if (firstTime)
	{
		if (reportSelType != DAY && reportSelType != MOVEMENT)
			PrintWkBegin ();
	}

	if (!firstTime && print_type > 1) 
	{
		PrintLine ();

		switch (print_type)
		{
		case	2:
			if (DETAIL)
			{
				PrintTotal ("C");
				pr_format (fin, fout, ((DETAIL) ? "LINE1" : "LINE_1S"), 0, 0);
			}
			break;

		case	3:
			if (DETAIL)
				PrintTotal ("I");
			break;

		case	4:
			if (DETAIL)
			{
				CheckBreakTwo ();
				if (sortType == CUST) 
					PrintTotal ("C");
				else
				if (sortType == ITEM) 
					PrintTotal ("I");
				else
					PrintTotal ("S");
			}
			PrintTotal ("W");
			fprintf (fout, ".PA\n");
			PrintWkBegin ();
			break;

		case	5:
			if (DETAIL)
				PrintTotal ("S");
			break;

		case	6:
			if (DETAIL)
			{
				CheckBreakTwo ();
				if (sortType == CUST) 
				{
					PrintTotal ("C");
					pr_format (fin, fout, ((DETAIL) ? "LINE1" : "LINE_1S"), 0, 0);
				}
				else
				if (sortType == ITEM) 
					PrintTotal ("I");
				else
					PrintTotal ("S");
			}
			PrintTotal ("M");
			if (sortType == CUST)
				pr_format (fin, fout, ((DETAIL) ? "B_LINE" : "B_LINE_S"), 0, 0);
			else
				pr_format (fin, fout, ((DETAIL) ? "B_ITEM" : "B_ITEM_S"), 0, 0);

			if (weekly != 0)
			{
				PrintWkBegin ();
				weekly = 2;
			}
		}

	}

	if (print_type == 1)
		PrintLine ();
}

void
PrintWkBegin (
 void)
{
	switch (sortType)
	{
	case	CUST :
		pr_format (fin, fout, (DETAIL ? "B_LINE" : "B_LINE_S"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "BEGIN_CUST" : "BEGIN_CS"), 1, week_tab [currentWeek].start); 
		break;

	case	ITEM :
	case	SMAN :
		pr_format (fin, fout, (DETAIL ? "B_ITEM" : "B_ITEM_S"), 0, 0);
		pr_format (fin, fout, (DETAIL ? "BEGIN_ITEM" : "BEGIN_IS"), 1, week_tab [currentWeek].start);
		break;

	}
}

void
PrintLine (void)
{
	double	m_margin = 0.00;
	float	margin = 0.00;

	cumr_rec.hhcu_hash	=	previousHash [1];
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		sprintf (cumr_rec.dbt_name, "%40.40s", " ");

	inmr_rec.hhbr_hash	=	previousHash [0];
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return;

	if (sortType == SMAN)
	{
		sprintf (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman, "%-2.2s", previousItem);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			sprintf (exsf_rec.salesman, "%40.40s", " ");
	}
	

	if (m_cost [0] != 0.00 && m_exval [0] != 0)
	{
		m_margin = (m_exval [0] - m_cost [0]) / m_exval [0];
		margin = (float)m_margin;
		margin *= 100;
	}
	else
		margin = 100.00;

	if (sortType == CUST)
	{
		if (DETAIL)
		{
			pr_format (fin, fout, "ORD_LINE", 1, previousDate);
			pr_format (fin, fout, "ORD_LINE", 2, cumr_rec.dbt_acronym);
			pr_format (fin, fout, "ORD_LINE", 3, cumr_rec.dbt_no);
			pr_format (fin, fout, "ORD_LINE", 4, cumr_rec.dbt_name);
			pr_format (fin, fout, "ORD_LINE", 5, previousItem);
			pr_format (fin, fout, "ORD_LINE", 6, m_qty [0]);
			pr_format (fin, fout, "ORD_LINE", 7, m_val [0]);
			pr_format (fin, fout, "ORD_LINE", 8, m_exval [0]);
			pr_format (fin, fout, "ORD_LINE", 9, margin);
		}
		else
		{
			pr_format (fin, fout, "ORD_LINE_S", 1, previousDate);
			pr_format (fin, fout, "ORD_LINE_S", 2, cumr_rec.dbt_acronym);
			pr_format (fin, fout, "ORD_LINE_S", 3, cumr_rec.dbt_no);
			pr_format (fin, fout, "ORD_LINE_S", 4, cumr_rec.dbt_name);
			pr_format (fin, fout, "ORD_LINE_S", 5, m_exval [0]);
			pr_format (fin, fout, "ORD_LINE_S", 6, margin);
		}
	}
	else
	{
		if (DETAIL)
		{
			pr_format (fin, fout, "ITEM_LINE", 1, previousDate);
			pr_format (fin, fout, "ITEM_LINE", 2, (BY_ITEM) ? inmr_rec.item_no : exsf_rec.salesman);
			pr_format (fin, fout, "ITEM_LINE", 3, (BY_ITEM) ? inmr_rec.description : exsf_rec.salesman);
			pr_format (fin, fout, "ITEM_LINE", 4, previousAcronym);
			pr_format (fin, fout, "ITEM_LINE", 5, m_qty [0]);
			pr_format (fin, fout, "ITEM_LINE", 6, m_val [0]);
			pr_format (fin, fout, "ITEM_LINE", 7, m_exval [0]);
			pr_format (fin, fout, "ITEM_LINE", 8, margin);
		}
		else
		{
			pr_format (fin, fout, "ITEM_LINES", 1, previousDate);
			pr_format (fin, fout, "ITEM_LINES", 2, (BY_ITEM) ? inmr_rec.item_no : exsf_rec.salesman);
			pr_format (fin, fout, "ITEM_LINES", 3, (BY_ITEM) ? inmr_rec.description : exsf_rec.salesman);
			pr_format (fin, fout, "ITEM_LINES", 4, m_exval [0]);
			pr_format (fin, fout, "ITEM_LINES", 5, margin);
		}
	}

	m_qty [1] += m_qty [0];
	m_val [1] += m_val [0];
	m_exval [1] += m_exval [0];
	m_cost [1] += m_cost [0];

	m_qty [0] = 0.00;
	m_val [0] = 0.00;
	m_exval [0] = 0.00;
	m_cost [0] = 0.00;
}

void
SumSales (
 char *data_line, 
 int add)
{
	char	*sptr = data_line;
	double	cost = 0.00;
	float	qty  = 0.00;

	cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", currentHash [0]);
	if (cc)
		return;

	cost  = DoubleValue (sptr + 22);
	if (cost == 0)
		cost = FindCost (currentHash [0]);
	qty = FloatValue (sptr);

	if (add)
	{
		m_qty [0]   += qty;
		m_val [0]   += DoubleValue (sptr + 11) - DoubleValue (sptr + 33);
		m_cost [0]  += cost * (double)qty;
		m_exval [0] += (DoubleValue (sptr + 11) - DoubleValue (sptr + 33)) * (double)qty;
	}
	else
	{
		m_qty [0]   = qty;
		m_val [0]   = DoubleValue (sptr + 11) - DoubleValue (sptr + 33);
		m_cost [0]  = cost * (double)qty;
		m_exval [0] = m_val [0] * (double)qty;
	}
}

long	
LongValue (
 char *str)
{
	char	val [7];

	sprintf (val, "%-6.6s", str);
	return (atol (val));
}

long	
DateValue (
 char *str)
{
	char	val [11];

	sprintf (val, "%-10.10s", str);
	return (atol (val));
}

float	
FloatValue (
 char *str)
{
	char	val [11];

	sprintf (val, "%-10.10s", str);
	return ((float) (atof (val)));
}

double	
DoubleValue (
 char *str)
{
	char	val [11];

	sprintf (val, "%-10.10s", str);
	return (atof (val));
}

int
GetWarehouse (
 long hhbr_hash)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, (BY_BR) ? prevBr : comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (!cc)
	{
		incc_rec.hhbr_hash = hhbr_hash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	}
	return (cc);
}

double	
FindCost (
	long	hhbrHash)
{
	double	cost = 0.00;

	if (GetWarehouse (hhbrHash))
		return (0.00);

	switch (inmr_rec.costing_flag [0])
	{
	case 'L':
	case 'A':
	case 'P':
	case 'T':
		cost	=	FindIneiCosts 
					(
						inmr_rec.costing_flag,
						(BY_BR) ? prevBr : comm_rec.est_no, 
						hhbrHash
					);
		break;

	case 'F':
		cost	=	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_rec.closing_stock, 
						TRUE, 
						TRUE,
						inmr_rec.dec_pt
					);
		break;

	case 'I':
		cost	=	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_rec.closing_stock, 
						TRUE, 
						FALSE,
						inmr_rec.dec_pt
					);
		break;

	case 'S':
		cost	=	FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;
	}
	if (cost <= 0.00)
		cost = FindIneiCosts ("L",(BY_BR) ? prevBr : comm_rec.est_no, hhbrHash);

	return (cost);
}

void
PrintTotal (
 char *tot_type)
{
	double	m_margin = 0.00;
	float	margin = 0.00;
	int	j = 0;
	int	print_ok = TRUE;

	switch (tot_type [0])
	{
	case	'C':
		j = 1;
		sprintf (err_str, "%-s", "* TOTAL FOR CUSTOMER");
		if (numberLines < 2)
			print_ok = FALSE;
		else
			numberLines = 1;
		break;

	case	'I':
		j = 1;
		sprintf (err_str, "%-s", "* TOTAL FOR ITEM");
		if (numberLines < 2)
			print_ok = FALSE;
		else
			numberLines = 1;
		break;

	case	'S':
		j = 1;
		sprintf (err_str, "%-s", "* TOTAL FOR SALESMAN");
		if (numberLines < 2)
			print_ok = FALSE;
		else
			numberLines = 1;
		break;

	case	'W':
		j = (DETAIL) ? 2 : 1;
		if (reportSelType == WEEK)
			currentWeek = 1;

		sprintf (err_str, "TOTAL FOR WEEK ENDING %s", DateToString (week_tab [currentWeek - 1].end));

		break;

	case	'M':
		j = (DETAIL) ? 2 : 1;
		sprintf (err_str, "%-s", "* TOTAL FOR MONTH");
		break;

	case	'E':
		if (reportSelType == DAY)
			j = (DETAIL) ? 2 : 1;
		else
			j = (DETAIL) ? 3 : 2;
		sprintf (err_str, "%-s", " *** TOTAL FOR COMPANY ");

		break;
	}

	m_qty [j + 1] += m_qty [j];
	m_val [j + 1] += m_val [j];
	m_exval [j + 1] += m_exval [j];
	m_cost [j + 1] += m_cost [j];

	if (m_cost [j] != 0.00 && m_exval [j] != 0.00)
	{
		m_margin = (m_exval [j] - m_cost [j]) / m_exval [j];
		margin = (float)m_margin;
		margin *= 100;
	}
	else
		margin = 100.00;

	if (print_ok)
	{
		if (sortType == CUST)
		{
			pr_format (fin, fout, ((DETAIL) ? "LINE1" : "LINE_1S"), 0, 0);
			if (DETAIL)
			{
				pr_format (fin, fout, "TOT_LINE", 1, err_str);
				pr_format (fin, fout, "TOT_LINE", 2, m_qty [j]);
				pr_format (fin, fout, "TOT_LINE", 3, m_exval [j]);
				pr_format (fin, fout, "TOT_LINE", 4, margin);
			}
			else
			{
				pr_format (fin, fout, "TOT_LINE_S", 1, err_str);
				pr_format (fin, fout, "TOT_LINE_S", 2, m_exval [j]);
				pr_format (fin, fout, "TOT_LINE_S", 3, margin);
			}
		}
		else
		{
			pr_format (fin, fout, ((DETAIL) ? "LINE_ITEM" : "LINE_ITEMS"), 0, 0);
			if (DETAIL)
			{
				pr_format (fin, fout, "TOT_ITEM", 1, err_str);
				pr_format (fin, fout, "TOT_ITEM", 2, m_qty [j]);
				pr_format (fin, fout, "TOT_ITEM", 3, m_exval [j]);
				pr_format (fin, fout, "TOT_ITEM", 4, margin);
			}
			else
			{
				pr_format (fin, fout, "TOT_ITEM_S", 1, err_str);
				pr_format (fin, fout, "TOT_ITEM_S", 2, m_exval [j]);
				pr_format (fin, fout, "TOT_ITEM_S", 3, margin);
			}
		}
	}

	m_qty [j] = 0.00;
	m_val [j] = 0.00;
	m_exval [j] = 0.00;
	m_cost [j] = 0.00;
}

void
InitArray (
 void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		m_qty [j] = 0.00;
		m_val [j] = 0.00;
		m_exval [j] = 0.00;
		m_cost [j] = 0.00;
	}
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

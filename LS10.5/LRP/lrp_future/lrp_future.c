/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_future.c,v 5.4 2002/11/25 03:16:37 scott Exp $
|  Program Desc  : (Stock Future stock out Reporting.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darow.     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: lrp_future.c,v $
| Revision 5.4  2002/11/25 03:16:37  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.3  2002/07/17 09:57:22  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:29:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:30  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:38  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.26  2000/07/10 01:52:27  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.25  2000/06/13 05:36:16  scott
| Does not need stddec.h
|
| Revision 1.24  2000/06/13 05:29:05  scott
| S/C LSANZ 16400
| Updated to allow for demand type '6' related to production issues.
| NOTE : Please see release notes on new search.
| sk_mrmaint, sk_delete, psl_sr_gen and sch.srsk must be installed/rebuilt.
|
| Revision 1.23  2000/01/28 09:27:08  ramon
| Added a save_rec () function call for the headings.
|
| Revision 1.22  2000/01/27 13:25:42  ramon
| For GVision compatibility, I added description fields to separate the fields to be inputted from its description.
|
| Revision 1.21  1999/12/06 01:34:17  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.20  1999/11/17 06:40:13  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.19  1999/11/09 10:45:46  scott
| Updated as calculated values not printed by mistake.
| S/C ASL
|
| Revision 1.18  1999/11/04 05:47:47  scott
| Updated to allow a warehouse to be excluded from LRP.
|
| Revision 1.17  1999/11/03 00:22:17  scott
| Updated to change environment FF_ to LRP_
|
| Revision 1.16  1999/10/27 07:32:58  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.15  1999/10/11 22:38:40  scott
| Updated for Date Routines
|
| Revision 1.14  1999/09/29 10:10:48  scott
| Updated to be consistant on function names.
|
| Revision 1.13  1999/09/17 07:26:37  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.12  1999/09/16 23:49:41  scott
| Updated to re-apply changes made before ansi update.
|
| Revision 1.11  1999/09/16 09:20:40  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/28 08:12:05  scott
| Updated as static structure option incorrect.
|
| Revision 1.7  1999/06/22 09:20:06  scott
| Updated to change weeks demand calculation to remove strange code related to lead times and future buckets. Also updated to calculate weeks based on 4.348 not 4.29 days per week.
|
| Revision 1.6  1999/06/15 07:27:03  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_future.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_future/lrp_future.c,v 5.4 2002/11/25 03:16:37 scott Exp $";

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_lrp_mess.h>
#include	<ml_std_mess.h>

#define		MAX_SUPP	9

#define		METH_A		1
#define		METH_B		2
#define		METH_C		3
#define		METH_D		4

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct excfRecord	excf_rec;
struct ffprRecord	ffpr_rec;
struct inccRecord	incc_rec;
struct inisRecord	inis_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;

	char	*ccmr2	= "ccmr2";

	int		historyMonths	=	0;
	char	demandInclude [5];
	char	abcCodes [5];
	char	envSupOrdRound [2];

#define	BY_CO	0
#define	BY_BR	1
#define	BY_WH	2

char	master_br [3],
		master_wh [3];
int		warehouseFlag 	= 	TRUE,
		detailedFlag 	= 	TRUE,
		envLrpPriority 	= 	FALSE;

long	currentHhccHash	=	0L;

char	programName [41],
		envLrpMethods [27];

float	covr_prd			=	0.00,
		lead_time			=	0.00,
		revw_prd			=	0.00,
		sfty_prd			=	0.00,
		wks_demand			=	0.00,
		demand				=	0.00,
		qty_avail			=	0.00,
		sur_qty				=	0.00,
		envLrpDfltReview	=	0.00;

double	qty_hand [13],
		qty_hist [12],
		qty_proj [12],
		qty_ord [12];

int		envLrpDmndNeg;
int		printerOpen = FALSE;

FILE	*fout;

#include	<LRPFunctions.h>


/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	back [6];
	char	backgroundDesc [6];
	char	onight [6];
	char	onightDesc [6];
	char	sCategory [12];
	char	sClass [2];
	char	sCatDesc [41];
	char	eCategory [12];
	char	eClass [2];
	char	eCatDesc [41];
	char	sGroup [13];
	char	eGroup [13];
	char	printerString [3];
	int		printerNumber;
	char	noWeeksString [3];
	int		noWeeks;
	int		historyMonths;
	char	forecastMethods [6];
	char	abcCodes [5];
	char	TransfersOK [10];
	char	TransfersOK_desc [10];
	char	LostSalesOK [10];
	char	LostSalesOK_desc [10];
	char	pcIssues [10];
	char	pcIssuesDesc [10];
	char	numberMonths [3];
	char	demandIncluded [5];
	char	Company [10];
	char	Detailed [10];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_class",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class             ", "Input Start Class A-Z.",
		YES, NO,  JUSTLEFT, "A", "Z", local_rec.sClass},
	{1, LIN, "st_cat",	 4, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category          ", "Input Start Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.sCategory},
	{1, LIN, "desc",	 4, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sCatDesc},
	{1, LIN, "en_class",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "Z", "End Class               ", "Input End Class A-Z.",
		YES, NO,  JUSTLEFT, "A", "Z", local_rec.eClass},
	{1, LIN, "en_cat",	 6, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category            ", "Input End Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.eCategory},
	{1, LIN, "desc",	6, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.eCatDesc},
	{1, LIN, "weeks",	8, 2, INTTYPE,
		"NN", "          ",
		" ", "0", "Contingency Weeks       ", "Enter the number of additional contingency weeks <Default = 0>",
		YES, NO, JUSTRIGHT, "0", "52", (char *) &local_rec.noWeeks},
	{1, LIN, "historyMonths",	8, 42, INTTYPE,
		"NN", "          ",
		" ", "36",   "History Months Used     ", "Enter Number of History Months Used in Calculations <Default = 36>",
		YES, NO, JUSTRIGHT, "1", "36", (char *)&local_rec.historyMonths},
	{1, LIN, "forecastMethods",	9, 2, CHARTYPE,
		"UUUU", "          ",
		" ", envLrpMethods, "Forecast Methods        ", envLrpMethods,
		YES, NO, JUSTLEFT, envLrpMethods, "", local_rec.forecastMethods},
	{1, LIN, "abcCodes",	9, 42, CHARTYPE,
		"UUUU", "          ",
		" ", "ABCD", "ABC Codes               ", " Include Items with these ABC Codes (Choose A/B/C/D) <Default = ABCD>",
		YES, NO, JUSTLEFT, "ABCD", "", local_rec.abcCodes},
	{1, LIN, "Detailed",	10, 2, CHARTYPE,
		"U", "          ",
		" ", "S",    "D(etailed) or S(ummary) ", "Enter D(etailed or S(ummary) for report printing. <Default = S>",
		YES, NO, JUSTRIGHT, "DS", "", local_rec.Detailed},
	{1, LIN, "Company/Warehouse",	10, 42, CHARTYPE,
		"U", "          ",
		" ", "W",    "By C(ompany) or W(arehouse)  ", "Enter C(ompany or W(arehouse) for report printing. <Default = W>",
		YES, NO, JUSTRIGHT, "CW", "", local_rec.Company},
	{1, LIN, "transfersok",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Include Transfers        ", "Enter Y(es to include transfer demand, <Default is N(o)>",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.TransfersOK},
	{1, LIN, "transfersok_desc",	11, 32, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.TransfersOK_desc},
	{1, LIN, "lostsalesok",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Include Lost Sales       ", "Enter Y(es to include lost sales demand, <Default = N(o)",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.LostSalesOK},
	{1, LIN, "lostsalesok_desc",	12, 32, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.LostSalesOK_desc},
	{1, LIN, "pcIssues",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Include Production Iss.  ", "Enter Y(es to include Production issues demand, <Default = N(o)",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.pcIssues},
	{1, LIN, "pcIssuesDesc",	13, 32, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.pcIssuesDesc},

	{1, LIN, "printerNumber",	15, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number          ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "back",	15, 42, CHARTYPE,
		"U", "          ",
		" ", "N", "Background              ", " Y(es) N(o) ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backgroundDesc",	15, 73, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.backgroundDesc},
	{1, LIN, "onight",	16, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight               ", " Y(es) N(o) ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	16, 33, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.onightDesc},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <RealCommit.h>

	struct	fcast_type
	{
		char	f_method [2];
		float	f_actual;
		float	f_forecast;
		float	f_wks_demand;
		float	f_sqr_err;
		float	f_pc_err;
	} store [27];

	int		fore_cast = METH_D,
			max_methods;

	double	err_thrshld;

    int envLprDmndNeg;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
float 	GetLeadDate 		(long, long);
float 	GetLeadTime 		(long, char *, char *);
float 	WarehouseDemand 	(long);
int 	BestMethod 			(int);
int 	CalculateErratic 	(int);
int 	FindBestSupplier 	(long, int);
int 	GetProcessMonth		(long);
int 	heading 			(int);
int 	spec_valid 			(int);
static 	float Rnd_Mltpl 	(float, char *, float, float);
void 	CalculateDemand		(int, int);
void 	CalculateMethods	(void);
void 	ClearMethods 		(void);
void 	CloseDB 			(void);
void 	Method_A 			(int);
void 	Method_B 			(int);
void 	Method_C 			(int);
void 	Method_D 			(int);
void 	OpenDB 				(void);
void 	OpenReport 			(void);
void 	PrintShortfall 		(void);
void 	ProcessGutz 		(void);
void 	ReadMisc 			(void);
void 	SrchExcf 			(char *);
void 	shutdown_prog 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;
	SETUP_SCR (vars);


	sptr = chk_env ("LRP_METHODS");
	sprintf (envLrpMethods, "%-4.4s", (sptr) ? sptr : "ABCD");
	max_methods	=	strlen (envLrpMethods);

	sptr = chk_env ("LRP_PRIORITY");
	envLrpPriority = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("LRP_DMND_NEG");
	envLrpDmndNeg = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("LRP_DFLT_REVIEW");
	envLrpDfltReview = (sptr == (char *)0) ? (float) 4 : atof (sptr);

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
	envLrpDfltReview = (sptr) ? atof (sptr) : 4;

	if (for_chk () == 0)
	{
		init_scr ();
		set_tty ();
	}

	if (argc != 1 && argc != 11)
	{
		clear ();

		print_at (0,0, "Usage :-\n");
		print_at (1,0, "%s [S(ummary)/D(etailed)] [<W(arehouse) C(ompany)>] <LPNO> <# months> <start group> <end_group> <Valid Methods> <abcCodes> <History Months> <Demand type Included>\n", argv [0]);
        return (EXIT_FAILURE);
	}

	OpenDB ();

	ReadMisc ();

	strcpy (programName, ML (" Future Stockout Report "));
	
	if (argc == 11)
	{
		if (for_chk ())
			rset_tty ();

		if (argv [2] [0] == 'W' || argv [2] [0] == 'w')
			warehouseFlag = TRUE;
		else
			warehouseFlag = FALSE;

		if (argv [1] [0] == 'S' || argv [1] [0] == 's')
			detailedFlag = FALSE;

		if (argv [2] [0] == 'W' || argv [2] [0] == 'w')
			setup_LSA
			 (
				BY_WH,
				comm_rec.co_no,
				comm_rec.est_no,
				comm_rec.cc_no
			);
		else
		{
			warehouseFlag = FALSE;
			cc = setup_LSA
			 (
				BY_CO,
				comm_rec.co_no,
				"  ",
				"  "
			);
			if (cc)
			{
				/*--------------------------------------------
				| Branches are in different periods! 		|
				| CANNOT produce company-wide forecast!!	|
				--------------------------------------------*/
				if (argc == 3)
					print_mess (ML (mlLrpMess058));
				sleep (sleepTime);
				shutdown_prog (TRUE);
                return (EXIT_SUCCESS);
			}
		}
		local_rec.printerNumber = atoi (argv [3]);
		if (local_rec.printerNumber == 0)
			local_rec.printerNumber = 1;

		local_rec.noWeeks = atoi (argv [4]);

		strcpy (local_rec.sGroup, argv [5]);
		strcpy (local_rec.eGroup, argv [6]);

		strcpy (envLrpMethods, argv [7]);
		strcpy (abcCodes, 	argv [8]);
		historyMonths	=	atoi (argv [9]);
		strcpy (demandInclude, argv [10]);

		ProcessGutz ();
		if (printerOpen)
			pclose (fout);

		shutdown_prog (TRUE);
        return (EXIT_SUCCESS);
	}

	set_masks ();
	init_vars (1);
	local_rec.printerNumber = 1;
	local_rec.noWeeks = 0;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.backgroundDesc, "No ");
	strcpy (local_rec.onight, "N");
	strcpy (local_rec.onightDesc, "No ");
	strcpy (local_rec.sGroup, "            ");
	strcpy (local_rec.eGroup, "~~~~~~~~~~~~");
	init_ok = 0;

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog (TRUE);
            return (EXIT_SUCCESS);
        }

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
        {
			shutdown_prog (TRUE);
            return (EXIT_SUCCESS);
        }
		prog_exit = TRUE;
	}

	sprintf (local_rec.printerString, "%d", local_rec.printerNumber);
	sprintf (local_rec.noWeeksString, "%d", local_rec.noWeeks);

	sprintf (local_rec.sGroup, "%1.1s%-11.11s", local_rec.sClass, local_rec.sCategory);

	sprintf (local_rec.eGroup, "%1.1s%-11.11s", local_rec.eClass, 
						    local_rec.eCategory);

	strcpy (local_rec.demandIncluded, "1");

	if (local_rec.TransfersOK [0] == 'Y')
		strcat (local_rec.demandIncluded, "2");

	if (local_rec.LostSalesOK [0] == 'Y')
		strcat (local_rec.demandIncluded, "3");

	if (local_rec.pcIssues [0] == 'Y')
		strcat (local_rec.demandIncluded, "4");

	sprintf (local_rec.numberMonths, "%02d", local_rec.historyMonths);

	local_rec.TransfersOK [1]	=	 (char) NULL;
	local_rec.LostSalesOK [1]	=	 (char) NULL;
	local_rec.Detailed [1]		=	 (char) NULL;
	local_rec.Company [1]		=	 (char) NULL;

	CloseDB (); 
	FinishProgram ();

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		execlp ("ONIGHT",
			    "ONIGHT",
			    argv [0],
			    local_rec.Detailed,
			    local_rec.Company,
			    local_rec.printerString,
			    local_rec.noWeeksString,
			    local_rec.sGroup,
			    local_rec.eGroup,
			    local_rec.forecastMethods,
			    local_rec.abcCodes,
			    local_rec.numberMonths,
			    local_rec.demandIncluded,
			    programName,
			    (char *) 0);
	}
	/*====================================
	| Test for foreground or background. |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		if (!fork ())
		{
			execlp (argv [0],
				    argv [0],
				    local_rec.Detailed,
				    local_rec.Company,
				    local_rec.printerString,
				    local_rec.noWeeksString,
				    local_rec.sGroup,
				    local_rec.eGroup,
				    local_rec.forecastMethods,
				    local_rec.abcCodes,
				    local_rec.numberMonths,
				    local_rec.demandIncluded,
				    (char *) 0);
		}
	}
	else
	{
		execlp (argv [0],
			    argv [0],
			    local_rec.Detailed,
			    local_rec.Company,
			    local_rec.printerString,
			    local_rec.noWeeksString,
			    local_rec.sGroup,
			    local_rec.eGroup,
			    local_rec.forecastMethods,
			    local_rec.abcCodes,
			    local_rec.numberMonths,
			    local_rec.demandIncluded,
			    (char *) 0);
	}
	shutdown_prog (FALSE);    
    return (EXIT_SUCCESS);
}

/*=====================================================================
| Program exit sequence.                                              |
=====================================================================*/
void
shutdown_prog (
 int    reason)
{
	if (reason)
	{
		CloseDB (); 
		FinishProgram ();
	}
}

/*===========================
| read comm file			|
===========================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	currentHhccHash = ccmr_rec.hhcc_hash;

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no,  "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		if (ccmr_rec.master_wh [0] == 'Y' ||
		    ccmr_rec.master_wh [0] == 'y')
		{
			strcpy (master_br, ccmr_rec.est_no);
			strcpy (master_wh, ccmr_rec.cc_no);
			break;
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
}

/*===============================
| Open data base files.			|
===============================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	abc_alias (ccmr2, ccmr);

	open_rec (ccmr2,ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ffpr, ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
	LSA_open ();
}

/*===============================
| Close data base files.		|
===============================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (esmr);
	abc_fclose (excf);
	abc_fclose (ffpr);
	abc_fclose (incc);
	abc_fclose (inis);
	abc_fclose (inld);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (soic);
	LSA_close ();
	abc_dbclose ("data");
}

int
spec_valid (
 int    field)
{

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("st_cat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.sCategory);

		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc) 
			{
				/*---------------------
				| Category not found. |
				---------------------*/
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.sCategory,"%-11.11s","           ");
			sprintf (excf_rec.cat_desc,"%-40.40s",ML ("BEGINNING OF RANGE"));
		}
		if (prog_status != ENTRY && strcmp (local_rec.sCategory,local_rec.eCategory) > 0)
		{
			/* ------------------------------
			| Start must be less thant end. |
			 ------------------------------*/
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.sCatDesc,"%-40.40s",excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("en_cat")) 
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.eCategory);
		
		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc) 
			{
				/*---------------------
				| Category not found. |
				---------------------*/
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.eCategory,"%-11.11s","~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc,"%-40.40s", ML ("END OF RANGE"));
		}
		if (strcmp (local_rec.sCategory,local_rec.eCategory) > 0)
		{
			/*------------------------------
			| Start must be less than end. |
			------------------------------*/
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.eCatDesc,excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			/*------------------
			| Invalid printer. |
			------------------*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.backgroundDesc, "Yes");
			strcpy (local_rec.onight, "N");
			strcpy (local_rec.onightDesc, "No ");
			DSP_FLD ("onight");
			DSP_FLD ("onightDesc");
		}
		else
		{
			strcpy (local_rec.backgroundDesc, "No ");
		}
		DSP_FLD ("backgroundDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.onightDesc, "Yes");
			strcpy (local_rec.back, "N");
			strcpy (local_rec.backgroundDesc, "No ");
			DSP_FLD ("back");
			DSP_FLD ("backgroundDesc");
		}
		else
			strcpy (local_rec.onightDesc, "No ");

		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("transfersok"))
	{
		strcpy (local_rec.TransfersOK_desc, (local_rec.TransfersOK [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("transfersok_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lostsalesok"))
	{
		strcpy (local_rec.LostSalesOK_desc, (local_rec.LostSalesOK [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("lostsalesok_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pcIssues"))
	{
		strcpy (local_rec.pcIssuesDesc, (local_rec.pcIssues [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("pcIssuesDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Detailed"))
	{
		strcpy (local_rec.Detailed, (local_rec.Detailed [0] == 'S') ? "S(ummary" : "D(etailed");
		DSP_FLD ("Detailed");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Company"))
	{
		strcpy (local_rec.Company, (local_rec.Company [0] == 'C') ? "C(ompany" : "Warehouse");
		DSP_FLD ("Company");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Hair's da gutz ov da programme sauce koad.	|
===============================================*/
void
ProcessGutz (void)
{
	float	pord_qty;

	int		i	=	0,
			WrkCovr	=	0,
			insert_mth;

	float	realCommitted;

	dsp_screen ("LRP - Future Stock-shortfall Report",
									comm_rec.co_no,
									comm_rec.co_name);

	abc_selfield (incc, (warehouseFlag) ? "incc_id_no" : "incc_hhbr_hash");

	OpenReport ();

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", local_rec.sGroup);
	sprintf (inmr_rec.category, "%-11.11s", local_rec.sGroup + 1);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		/*-------------------------------------------
		| Make sure record is within group bounds	|
		-------------------------------------------*/
		if (strncmp (inmr_rec.inmr_class, local_rec.eGroup, 1) > 0)
			break;

		if (strncmp (inmr_rec.inmr_class, local_rec.eGroup, 1) == 0 &&
			 strncmp (inmr_rec.category,local_rec.eGroup + 1,11) > 0)
		break;

		dsp_process ("Item", inmr_rec.item_no);

		/*---------------------------------------
		| Ignore inmr records if:				|
		|  i) Warehouse mode is selected AND	|
		| ii) There is no matching incc record	|
		---------------------------------------*/
		if (warehouseFlag)
		{
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			incc_rec.hhcc_hash = currentHhccHash;
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			if (cc)
			{
					cc = find_rec (inmr, &inmr_rec, NEXT, "r");
					continue;
			}
			else
				wks_demand = incc_rec.wks_demand;
		}
		else
			wks_demand = WarehouseDemand (inmr_rec.hhbr_hash);

		for (i = 0; i < 12; i++)
			qty_ord [i] = 0.00;

		/*---------------------------
		| Item Exists in Warehouse	|
		---------------------------*/
		lead_time = incc_rec.safety_stock + 
			        GetLeadTime 
					 (
						inmr_rec.hhbr_hash, 
						ccmr_rec.est_no, 
						ccmr_rec.cc_no
					);
		calc_LSA
		 (		
			envLrpMethods,	
			inmr_rec.hhbr_hash,	
			comm_rec.inv_date,
			TRUE,
			historyMonths,
			LRP_PASSED_MONTH,
			demandInclude
		);

		CalculateMethods ();

		if (warehouseFlag)
		{
			if (incc_rec.ff_option [0] == 'M' || 
				 incc_rec.ff_option [0] == 'P')
			{
				if (incc_rec.ff_method [0] >= 'A' && 
					 incc_rec.ff_method [0] <= 'D')
				{
					fore_cast = incc_rec.ff_method [0] - 'A' +1;
				}
			}
		}

		for (i = 0; i < 12; i++)
		{
			qty_hist [i] = LSA_result [0] [i + 23];
			qty_proj [i] = LSA_result [fore_cast] [i + 35];
		}

		/*-------------------------------------------
		| Load in all relevant purchase orders	|
		| pertaining to this product/warehouse	|
		-------------------------------------------*/
		poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
		poln_rec.due_date = 0L;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (poln_rec.hhcc_hash != currentHhccHash)
			{
		    		cc = find_rec (poln, &poln_rec, NEXT, "r");
		    		continue;
			}
			cc = find_hash (pohr, &pohr_rec, EQUAL, "r", 
				poln_rec.hhpo_hash);
			if (pohr_rec.drop_ship [0] != 'Y')
			{
				pord_qty = poln_rec.qty_ord - poln_rec.qty_rec;
				if (pord_qty > 0.00)
				{
		    		insert_mth = GetProcessMonth (poln_rec.due_date);
		    		qty_ord [insert_mth] += pord_qty;
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}

		cc = FindBestSupplier (inmr_rec.hhbr_hash, MAX_SUPP);
		if (cc)
	    {
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
	    {
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		/*---------------------------------
		| Calculate Actual Qty Committed. |
		---------------------------------*/
		realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,
											incc_rec.hhcc_hash);
		if (warehouseFlag)
		{
			qty_hand [0] = incc_rec.closing_stock - 
						  incc_rec.committed -
						  incc_rec.backorder -
						  realCommitted;

			sfty_prd = incc_rec.safety_stock;
		}
		else
		{
			qty_hand [0] = inmr_rec.on_hand - 
						  inmr_rec.committed -
						  inmr_rec.backorder -
						  realCommitted;

			sfty_prd = inmr_rec.safety_stock;
		}
		if (sfty_prd > 12)
			sfty_prd = 12;

		covr_prd = lead_time + sfty_prd +  revw_prd + local_rec.noWeeks;

		for (i = 0; i < 12; i++)
		   qty_hand [i + 1] = qty_hand [i] + qty_ord [i] - qty_proj [i];

		WrkCovr	=	 ((covr_prd / 4.348) - 1);
		if (WrkCovr < 1)
			WrkCovr = 1;

		if (WrkCovr > 12)
			WrkCovr = 12;
				
		for (i = 0; i < WrkCovr; i++)
		{
			if (qty_hand [i + 1] < 0.0)
			{
				PrintShortfall ();
				break;
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

float
WarehouseDemand (
	long   hhbrHash)
{
	float	demand = 0.00;

	incc_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhbr_hash == hhbrHash)
	{
		demand += incc_rec.wks_demand;
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	return (demand);
}
/*===============================
| Decide which Method to Use	|
===============================*/
int
BestMethod (
 int    num_methods)
{
	int		i;
	int		fore_cast = -1;
	double	min_err = -1;

	for (i = 1;i < num_methods;i++)
	{
		if (strchr (LSA_methods, store [i].f_method [0]) == (char *) 0)
			continue;

		if (min_err < 0)
		{
			min_err = store [i].f_sqr_err;
			fore_cast = i;
		}

		/*--------------------------------------------------------------------
		| Standard error for this method must be more than 5 percent better  |
		| than earlier method to replace forecast method. Without this 5     |
		| percent condition random variations may be interpreted as seasonal |
		| or local linear may be incorrectly favoured over seasonal          |
		--------------------------------------------------------------------*/
		if (min_err > store [i].f_sqr_err * 1.05)
		{
			fore_cast = i;
			min_err = store [i].f_sqr_err;
		}
	}
	return (fore_cast);
}

/*=======================
| Straight LSA only.	|
=======================*/
void
Method_A (
 int    indx)
{
	store [indx].f_actual = LSA_last3 [0];
	if (strchr (LSA_methods, 'A') != (char *) 0)
	{
		store [indx].f_forecast = LSA_last3 [METH_A];
		CalculateDemand (indx, 1);
		store [indx].f_sqr_err 	= LSA_error [METH_A] [1];
		store [indx].f_pc_err 	= LSA_error [METH_A] [0];
	}
}

/*===========================
| Seaonal demand on LSA.	|
===========================*/
void
Method_B (
 int    indx)
{
	store [indx].f_actual = LSA_last3 [0];
	if (strchr (LSA_methods, 'B') != (char *) 0)
	{
		store [indx].f_forecast 	= LSA_last3 [METH_B];
		CalculateDemand (indx, 2);
		store [indx].f_sqr_err 	= LSA_error [METH_B] [1];
		store [indx].f_pc_err 	= LSA_error [METH_B] [0];
	}
}

/*=======================
| Seaonal trend on LSA.	|
=======================*/
void
Method_C (
 int    indx)
{
	store [indx].f_actual = LSA_last3 [0];
	if (strchr (LSA_methods, 'C') != (char *) 0)
	{
		store [indx].f_forecast = LSA_last3 [METH_C];
		CalculateDemand (indx, 3);
		store [indx].f_sqr_err 	= LSA_error [METH_C] [1];
		store [indx].f_pc_err 	= LSA_error [METH_C] [0];
	}
}

/*===================
| User 'FF' values.	|
===================*/
void
Method_D (
 int    indx)
{
	store [indx].f_actual = LSA_last3 [0];
	if (strchr (LSA_methods, 'D') != (char *) 0)
	{
		store [indx].f_forecast = LSA_last3 [METH_D];
		CalculateDemand (indx, 4);
		store [indx].f_sqr_err 	= LSA_error [METH_D] [1];
		store [indx].f_pc_err 	= LSA_error [METH_D] [0];
	}
}

void
CalculateMethods (void)
{
	char	*method;
	int		num_methods;
	int		fore_cast = -1;

	ClearMethods ();

	/*---------------------------------------
	| Calculate Forecast for Every Method	|
	---------------------------------------*/
	for (method = envLrpMethods, num_methods = 1; strlen (method); method++)
	{
		/*-------------------------------
		| Set index for method used	|
		-------------------------------*/
		switch (incc_rec.ff_option [0])
		{
		case	'M':
			fore_cast = 0;
			break;

		case	'P':
			if (incc_rec.ff_method [0] == *method)
				fore_cast = num_methods;
			break;

		case	'A':
		default:
			break;
		}

		sprintf (store [num_methods].f_method, "%-1.1s", method);

		switch (*method)
		{
		/*-------------------------------
		| Standard Least Squares linear	|
		-------------------------------*/
		case	'A':
			Method_A (num_methods++);
			break;

		/*-------------------------------
		| Seasonal trend variant to LSA	|
		-------------------------------*/
		case	'B':
			Method_B (num_methods++);
			break;

		/*-------------------------------
		| Local Linear variant of   LSA	|
		-------------------------------*/
		case	'C':
			Method_C (num_methods++);
			break;

		/*--------------------
		| Focus Forecasting. |
		--------------------*/
		case	'D':
			Method_D (num_methods++);
			break;

		default:
			break;
		}
	}
	if (incc_rec.ff_option [0] == 'A')
	{
		/*---------------------------------
		| If demand is erratic, forecast  |
		| method E with no demand		  |
		---------------------------------*/
		if (!LSA_hist || zero_hist)
			fore_cast = METH_D;
		else
		{
			fore_cast = BestMethod (num_methods);
			if (CalculateErratic (fore_cast))
				fore_cast = METH_D;
		}
	}
}

void
ClearMethods (void)
{
	int		i;
	
	for (i = 0;i <= max_methods;i++)
	{
		strcpy (store [i].f_method," ");
		store [i].f_actual 		= 0.00;
		store [i].f_forecast 	= 0.00;
		store [i].f_wks_demand 	= 0.00;
		store [i].f_sqr_err 		= 0.00;
		store [i].f_pc_err 		= 0.00;
	}
}

void
CalculateDemand (
 int    indx,
 int    method)
{
	double	demand	=	0.00;

	demand 	= LSA_result [method] [36]; 
	demand 	/= 4.348;

	store [indx].f_wks_demand = demand;

	/*------------------------------------------
	| Set demand to zero if demand is negative |
	| and negative demand is not allowed.      |
	------------------------------------------*/
	if (!envLprDmndNeg && store [indx].f_wks_demand < 0.00)
		store [indx].f_wks_demand = 0.00;
}

/*----------------------------------
| Determine whether the demand for |
| this product is erratic or not.  |
----------------------------------*/
int
CalculateErratic (
 int    fore_cast)
{
	int		i,
			j = 0;
	double	aver = 0;

	if (LSA_hist < 2)
		return (FALSE);

	/*----------------------
	| Perform calculation. |
	----------------------*/
	for (i = (35 - LSA_hist); i < 35; i++)
	{	
		aver += LSA_result [LSA_ACT] [i];
		if (!LSA_result [LSA_ACT] [i])
			j++;	
	}
	j = j * 100 / LSA_hist;			
	aver /= LSA_hist;

	/*-------------------------------------------------------
	| If standard error (forecast) * LRP_ERRATIC is greater  |
	| than average then demand is considered erratic        |
	-------------------------------------------------------*/
	if ((store [fore_cast].f_sqr_err * err_thrshld) > aver)
		return (TRUE); 

	return (FALSE);
}

int
GetProcessMonth (
 long poln_date)
{
	int		col_no,
			p_yy,
			p_mm,
			c_yy,
			c_mm;

	DateToDMY (comm_rec.inv_date, NULL, &c_mm, &c_yy);
	DateToDMY (poln_date, NULL, &p_mm, &p_yy);

	col_no = (p_yy - c_yy) * 12;
	col_no += p_mm;
	col_no -= c_mm;

	if (col_no < 0)
		return (EXIT_SUCCESS);
	if (col_no > 11)
		return (11);

	return (col_no);
}

void
PrintShortfall (void)
{
	static	char	*name [] =
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
	char	supp [7];
	int		mth;
	int		cvr_sug;
	float	sugg_reord;

	DateToDMY (comm_rec.inv_date, NULL, &mth, NULL);

	cvr_sug = lead_time + sfty_prd +  revw_prd + local_rec.noWeeks;
	cvr_sug	/=	4.348;
	if (cvr_sug > 12)
		cvr_sug	=	12;
		
	sugg_reord = 0 - qty_hand [cvr_sug];
	if (sugg_reord < 0)
	    sugg_reord = 0;

	if (sugg_reord != 0)
	{
	    if (sugg_reord < inis_rec.min_order)
			sugg_reord = inis_rec.min_order;
	}
	sugg_reord = Rnd_Mltpl 
				 (
					sugg_reord, 
					envSupOrdRound, 
					inis_rec.ord_multiple, 
					inis_rec.min_order
				);

	strcpy (supp, sumr_rec.crd_no);

	fprintf (fout, "|%1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);
	fprintf (fout, "|%16.16s", inmr_rec.item_no);
	fprintf (fout, "|%40.40s", inmr_rec.description);
	fprintf (fout, "| %6.6s ", supp);
	fprintf (fout, "| %5.0f  %5.0f  %5.0f  %5.0f ", lead_time, sfty_prd,
							   revw_prd, covr_prd);
	fprintf (fout, "|%10.2f %9.2f ", qty_hand [0], wks_demand);
	fprintf (fout, "|    %16.2f |\n", sugg_reord);

	if (!detailedFlag)
		return;

	fprintf (fout, "|            ");
	fprintf (fout, "                 ");
	fprintf (fout, "                                         ");
	fprintf (fout, "         ");
	fprintf (fout, "                             ");
	fprintf (fout, "                      ");
	fprintf (fout, "                      |\n");

	fprintf (fout, "| %16.16s ", " ");
	fprintf (fout, " %10.10s %10.10s", 	   name [ ((mth - 1) % 12)],
	    				       	   name [ ((mth + 0) % 12)]);
	fprintf (fout, " %10.10s %10.10s", 	   name [ ((mth + 1) % 12)],
	    				       	   name [ ((mth + 2) % 12)]);
	fprintf (fout, " %10.10s %10.10s", 	   name [ ((mth + 3) % 12)],
	    				       	   name [ ((mth + 4) % 12)]);
	fprintf (fout, " %10.10s %10.10s", 	   name [ ((mth + 5) % 12)],
	    				       	   name [ ((mth + 6) % 12)]);
	fprintf (fout, " %10.10s %10.10s", 	   name [ ((mth + 7) % 12)],
	    				       	   name [ ((mth + 8) % 12)]);
	fprintf (fout, " %10.10s %10.10s  |\n", name [ ((mth + 9) % 12)],
	    				           name [ ((mth + 10) % 12)]);

	fprintf (fout, "| %16.16s ", "Sales History   ");
	fprintf (fout, " %10.2f %10.2f",    qty_hist [ 0 ], qty_hist [ 1 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hist [ 2 ], qty_hist [ 3 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hist [ 4 ], qty_hist [ 5 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hist [ 6 ], qty_hist [ 7 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hist [ 8 ], qty_hist [ 9 ]);
	fprintf (fout, " %10.2f %10.2f  |\n",qty_hist [ 10], qty_hist [ 11]);

	fprintf (fout, "| %16.16s ", "Demand          ");
	fprintf (fout, " %10.2f %10.2f",    qty_proj [ 0 ], qty_proj [ 1 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_proj [ 2 ], qty_proj [ 3 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_proj [ 4 ], qty_proj [ 5 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_proj [ 6 ], qty_proj [ 7 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_proj [ 8 ], qty_proj [ 9 ]);
	fprintf (fout, " %10.2f %10.2f  |\n",qty_proj [ 10], qty_proj [ 11]);

	fprintf (fout, "| %16.16s ", "Stock on Order  ");
	fprintf (fout, " %10.2f %10.2f",    qty_ord [ 0 ], qty_ord [ 1 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_ord [ 2 ], qty_ord [ 3 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_ord [ 4 ], qty_ord [ 5 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_ord [ 6 ], qty_ord [ 7 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_ord [ 8 ], qty_ord [ 9 ]);
	fprintf (fout, " %10.2f %10.2f  |\n",qty_ord [ 10], qty_ord [ 11]);

	fprintf (fout, "| %16.16s ", "Stock Available ");
	fprintf (fout, " %10.2f %10.2f",    qty_hand [ 1 ], qty_hand [ 2 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hand [ 3 ], qty_hand [ 4 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hand [ 5 ], qty_hand [ 6 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hand [ 7 ], qty_hand [ 8 ]);
	fprintf (fout, " %10.2f %10.2f",    qty_hand [ 9 ], qty_hand [ 10]);
	fprintf (fout, " %10.2f %10.2f  |\n",qty_hand [ 11], qty_hand [ 12]);

	fprintf (fout, "|------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|---------------------|\n");

	fprintf (fout, ".LRP8\n");
}

void
OpenReport (void)
{
	char	RunDesc [256];
	if (printerOpen)
		return;

	fout = popen ("pformat", "w");
	if (fout == (FILE *) NULL)
		file_err (cc, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".PL60\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EFUTURE STOCK-OUT REPORT\n");

	if (warehouseFlag)
		fprintf (fout, ".EFOR WAREHOUSE: %-2.2s %s\n", 
				comm_rec.cc_no, clip (comm_rec.cc_name));
	else
		fprintf (fout, ".EFOR COMPANY: %-2.2s %s\n", 
				comm_rec.co_no, clip (comm_rec.co_name));

	fprintf (fout,".ESTART GROUP : %-12.12s END GROUP : %-12.12s\n",
					local_rec.sGroup, local_rec.eGroup);

	strcpy (RunDesc, "Sales ");

	if (strchr (demandInclude, PLUS_TRANSFERS) != (char *) 0)
		strcat (RunDesc, " + Transfer ");
	if (strchr (demandInclude, PLUS_LOSTSALES) != (char *) 0)
		strcat (RunDesc, " + Lost Sales ");
	if (strchr (demandInclude, PLUS_PC_ISSUES) != (char *) 0)
		strcat (RunDesc, " + Production Issues");

	fprintf (fout, ".CContingency Period : %d (Weeks) /  Forecast methods (%s) / ABC codes (%s) / No Months History (%d) / %s\n", 
						local_rec.noWeeks,
						envLrpMethods,
						abcCodes,
						historyMonths, 
						RunDesc);

	fprintf (fout, ".R=============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	fprintf (fout, "=============================");
	fprintf (fout, "======================");
	fprintf (fout, "=======================\n");

	fprintf (fout, "=============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	fprintf (fout, "=============================");
	fprintf (fout, "======================");
	fprintf (fout, "=======================\n");

	fprintf (fout, "|   GROUP.   ");
	fprintf (fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|             ITEM DESCRIPTION           ");
	fprintf (fout, "| SUPP.  ");
	fprintf (fout, "| LEAD  SAFETY REVIEW  COVER ");
	fprintf (fout, "| AVAILABLE           ");
	fprintf (fout, "|    R E O R D E R    |\n");
	
	fprintf (fout, "|            ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "| NUMBER ");
	fprintf (fout, "| TIME   STOCK PERIOD PERIOD ");
	fprintf (fout, "| QUANTITY.   DEMAND  ");
	fprintf (fout, "|   Q U A N T I T Y   |\n");

	fprintf (fout, "|------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|---------------------|\n");

	printerOpen = TRUE;
	fflush (fout);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category No.", "#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
		      !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "excf", "DBFIND");
}

int
FindBestSupplier (
 long   hhbrHash,
 int    NoLevels)
{
	int		i;

	for (i = 0; i < NoLevels; i++)
	{
		inis_rec.hhbr_hash = hhbrHash;
		sprintf (inis_rec.sup_priority, "W%1d", i);
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, comm_rec.est_no);
		strcpy (inis_rec.wh_no, comm_rec.cc_no);
		cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = hhbrHash;
			sprintf (inis_rec.sup_priority, "B%1d", i);
			strcpy (inis_rec.co_no, comm_rec.co_no);
			strcpy (inis_rec.br_no, comm_rec.est_no);
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
			if (cc)
			{
				inis_rec.hhbr_hash = hhbrHash;
				sprintf (inis_rec.sup_priority, "C%1d", i);
				strcpy (inis_rec.co_no, comm_rec.co_no);
				strcpy (inis_rec.br_no, "  ");
				strcpy (inis_rec.wh_no, "  ");
				cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
			}
		}
		if (!cc)
			return (EXIT_SUCCESS);
	}
	return (cc);
}

float	
GetLeadTime (
 long   hhbr_hash,
 char*  br_no,
 char*  wh_no)
{
	float	weeks = 0.00;

	inis_rec.hhbr_hash = hhbr_hash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, br_no);
	strcpy (inis_rec.wh_no, wh_no);
	cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.hhbr_hash = hhbr_hash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, br_no);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = hhbr_hash;
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
			weeks = GetLeadDate (inis_rec.hhis_hash, comm_rec.inv_date);
		else
			weeks = inis_rec.lead_time / 7.00;
	}

	/*---------------------------------------
	| Find out what the review-period	    |
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
					ffpr_rec.review_prd = envLrpDfltReview;
			}
			abc_selfield (ffpr, "ffpr_id_no");
	    }
	}
	revw_prd = ffpr_rec.review_prd;
	return (weeks);
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.			        |
| Return 0 if none found.			            |
===============================================*/
float	
GetLeadDate (
 long   hash, 
 long   date)
{
	float	weeks;

	inld_rec.hhis_hash = hash;
	inld_rec.ord_date = date;

	cc = find_rec ("inld", &inld_rec, GTEQ, "r");
	if (cc)
		return ((float) 0.00);

	weeks = (inld_rec.sup_date - date) / 7;
	return (weeks);
}

static 
float
Rnd_Mltpl (
 float  ord_qty,
 char*  rnd_type,
 float  ord_mltpl,
 float  min_qty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (ord_qty == 0.00)
		return (0.00);

	if (ord_mltpl == 0.00)
		return ((ord_qty < min_qty) ? min_qty : ord_qty);

	ord_qty -= min_qty;
	if (ord_qty < 0.00)
		ord_qty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (ord_qty / ord_mltpl);
	if (ceil (wrk_qty) == wrk_qty)
		return (ord_qty + min_qty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (rnd_type [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = floor (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double) ord_qty;
		wrk_qty = (up_qty / (double)ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * ord_mltpl);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double) ord_qty;
		wrk_qty = (down_qty / (double) ord_mltpl);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * ord_mltpl);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double) ord_qty) <= ((double) ord_qty - down_qty))
			ord_qty = (float) up_qty;
		else
			ord_qty = (float) down_qty;

		break;

	default:
		break;
	}

	return (min_qty + ord_qty);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (programName, 25, 0, 1);
	move (0, 1);
	line (80);

	box (0, 2, 80, 14);

	move (1, 7);
	line (79);

	move (1, 14);
	line (79);

	move (0, 19);
	line (80);
	print_at (20,1, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21,1, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	move (0, 22);
	line (80);
	move (1, input_row);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}


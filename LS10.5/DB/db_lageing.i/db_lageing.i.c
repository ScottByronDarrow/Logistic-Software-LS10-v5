/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lageing.i.c,v 5.2 2002/07/17 09:57:07 scott Exp $
|  Program Name  : (db_lageing.i.c)
|  Program Desc  : (Selection for Sumary and Detailed trial Balance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/01/87         |
|---------------------------------------------------------------------|
| $Log: db_lageing.i.c,v $
| Revision 5.2  2002/07/17 09:57:07  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/11/26 03:42:31  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lageing.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lageing.i/db_lageing.i.c,v 5.2 2002/07/17 09:57:07 scott Exp $";

#undef	MAXWIDTH
#define	MAXWIDTH	150

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_db_mess.h>
#include <ml_std_mess.h>

#define	SUMMARY		 (programType [0] == 'S')
#define	EXTRA		 (programType [0] == 'E')
#define	OFFSET	13

extern	int	X_EALL;
extern	int	Y_EALL;

	int		envDbDaysAgeing;

	/*
	 * Special fields and flags.
	 */
	char 	programDesc [81],
			programType [2],	
			department [3],
			reportType [2],
			runType [2],
			balType [2],
			sortType [2],
			salesCustomer [2],
			ageType [2],
			noPrompt	[11],
			yesPrompt	[11];

#include	"schema"

struct commRecord	comm_rec;
struct cudpRecord	cudp_rec;
struct pocrRecord	pocr_rec;

	char	*data = "data";


	int	envDbMcurr;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	back [2];
	char	backDesc [8];
	char	reportType [18][2];
	char	reportTypeDesc [18][8];
	double	reportValue [4];
	char	onight [2];
	char	onightDesc [8];
	char	departmentNo [3];
	int		printerNo;
	char	printerString [3];
	char	currencyDisp [2];
	char	currenctDisplayDesc [10];
	char	currencySort [2];
	char	currencySortDesc [4];
	char	startCurrency [4];
	char	startCurrencyDesc [41];
	char	endCurrency [4];
	char	endCurrencyDesc [41];
	char	DfltAgeDays [3];
	int		DaysAgeing;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "printerNo",	 2, 36, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 3, 36, CHARTYPE,
		"U", "          ",
		" ", "N", "Background", "Enter Y(es) or N(o). ",
		YES, NO, JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 3, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	 3, 100, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 3, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},
	{1, LIN, "sel1",	 5, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Overdue Period 1", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [0]},
	{1, LIN, "sel1",	 6, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Overdue Period 2", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [1]},
	{1, LIN, "sel1",	 7, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Overdue Period 3", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [2]},
	{1, LIN, "sel1",	 8, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Overdue Period 4+", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [3]},
	{1, LIN, "sel1",	 9, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Normal Type.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [4]},
	{1, LIN, "sel1",	 10, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Customer over credit limit.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [5]},
	{1, LIN, "sel1",	 10, 100, CHARTYPE,
		"U", "          ",
		" ", "Y", "Customer on Stop Credit.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [6]},
	{1, LIN, "sel1_desc",	 5, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [0]},
	{1, LIN, "sel1_desc",	 6, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [1]},
	{1, LIN, "sel1_desc",	 7, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [2]},
	{1, LIN, "sel1_desc",	 8, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [3]},
	{1, LIN, "sel1_desc",	 9, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [4]},
	{1, LIN, "sel1_desc",	 10, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [5]},
	{1, LIN, "sel1_desc",	 10, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [6]},
	{1, LIN, "val1",	 5, 100, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Value Period 1", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.reportValue [0]},
	{1, LIN, "val2",	 6, 100, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Value Period 2", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.reportValue [1]},
	{1, LIN, "val3",	 7, 100, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Value Period 3", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.reportValue [2]},
	{1, LIN, "val4",	 8, 100, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Value Period 4+", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.reportValue [3]},
	{1, LIN, "sel2",	12, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Report By Company.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [7]},
	{1, LIN, "sel2",	12, 100, CHARTYPE,
		"U", "          ",
		" ", "Y", "Report By Branch.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [8]},
	{1, LIN, "sel2",	13, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Report By Department.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [9]},
	{1, LIN, "sel2_desc",	 12, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [7]},
	{1, LIN, "sel2_desc",	 12, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [8]},
	{1, LIN, "sel2_desc",	 13, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [9]},
	{1, LIN, "dep",	13, 100, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Enter Department No.", " ",
		YES, NO, JUSTRIGHT, "0123456789", "", local_rec.departmentNo},
	{1, LIN, "sel3",	15, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort By Customer Number.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [10]},
	{1, LIN, "sel3",	15, 100, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort by Customer Acronym.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [11]},
	{1, LIN, "sel3_desc",	 15, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [10]},
	{1, LIN, "sel3_desc",	 15, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [11]},
	{1, LIN, "sel5",	16, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort By Salesman.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [12]},
	{1, LIN, "sel5",	16, 100, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort By Cust Type.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [13]},
	{1, LIN, "sel5_desc",	 16, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [12]},
	{1, LIN, "sel5_desc",	 16, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [13]},
	{1, LIN, "sel4",	18, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Exclude zero balance Customers.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [14]},
	{1, LIN, "sel4",	18, 100, CHARTYPE,
		"U", "          ",
		" ", "Y", "Exclude non-zero balance Customers.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [15]},
	{1, LIN, "sel4",	19, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Include 0 and non-zero Customers.", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [16]},
	{1, LIN, "sel4_desc",	 18, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [14]},
	{1, LIN, "sel4_desc",	 18, 103, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [15]},
	{1, LIN, "sel4_desc",	 19, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [16]},
	{1, LIN, "sel6",	19, 100, CHARTYPE,
		"U", "          ",
		" ", "M", "Report Ageing Selection.", "Enter 'M' to age report by month-end-Date, 'T' to age using True age date.",
		YES, NO,  JUSTLEFT, "CcTt", "", local_rec.reportType [17]},
	{1, LIN, "sel6_desc",	 19, 103, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [17]},
	{1, LIN, "DaysAge",	 20, 36, INTTYPE,
		"NN", "          ",
		" ", local_rec.DfltAgeDays, "Report Age Days ", "Enter days ageing to run report.",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.DaysAgeing},

	{2, LIN, "currencySort",	 3, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Sort By Currency", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.currencySort},
	{2, LIN, "currencySortDesc",	 3, 23, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.currencySortDesc},
	{2, LIN, "currencyDisp",	 5, 20, CHARTYPE,
		"U", "          ",
		" ", "L", "Local / Overseas", "Enter L (ocal) or O (verseas). ",
		YES, NO,  JUSTLEFT, "LlOo", "", local_rec.currencyDisp},
	{2, LIN, "currenctDisplayDesc",	 5, 23, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.currenctDisplayDesc},
	{2, LIN, "startCurrency",	 7, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Start Currency", "Enter Start Currency",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCurrency},
	{2, LIN, "startCurrencyDesc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startCurrencyDesc},
	{2, LIN, "endCurrency",	 8, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "End Currency  ", "Enter End Currency",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCurrency},
	{2, LIN, "endCurrencyDesc", 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endCurrencyDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 	(void);
int 	spec_valid 		(int);
void 	SrchPocr 		(char *);
int 	CloseDB 		(void);
void 	OpenDB 			(void);
void 	SetupDefault 	(void);
void 	RunProgram 		(void);
int 	heading 		(int);
void 	PrCoLine 		(void);

extern	int		EnvScreenOK;

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	if (argc != 2)
	{
		print_at (0,0, mlDbMess059, argv [0]);
		return (EXIT_FAILURE);
	}

	X_EALL = 1;
	Y_EALL = 1;

	/*
	 * Multi-currency customer.
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if ageing is by days overdue or as per standard ageing.
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	sprintf (local_rec.DfltAgeDays, "%02d", envDbDaysAgeing);

	SETUP_SCR (vars);

	EnvScreenOK		=	FALSE;
	switch (argv [1][0])
	{
	case	'S':
	case	's':
		strcpy (programType,"S");
		break;

	case	'E':
	case	'e':
		strcpy (programType,"E");
		break;

	case	'D':
	case	'd':
		strcpy (programType,"D");
		break;

	default:
		print_at (0,0, mlDbMess059, argv [0]);
		return (EXIT_FAILURE);
	}

	if (!SUMMARY)
	{
		FLD ("sel5") = ND;
		vars [label ("sel5") + 1].required = ND;
		FLD ("val1") = ND;
		FLD ("val2") = ND;
		FLD ("val3") = ND;
		FLD ("val4") = ND;
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	swide ();

	OpenDB ();

	strcpy (yesPrompt, ML ("Yes"));
	strcpy (noPrompt,  ML ("No "));

   	entry_exit 	= TRUE;
   	search_ok 	= TRUE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	init_vars (1);	

	SetupDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	if (envDbMcurr)
		edit_all ();
	else
		edit (1);

	if (restart) 
	{
		snorm ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	
	RunProgram ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
	char	valid_inp [2];
	int	found_yes = 0;
	int	i;

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			/*-----------------
			| Invalid Printer |
			-----------------*/
			print_mess (ML (mlStdMess020)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.back);

		if (valid_inp [0] == 'N')
			strcpy (local_rec.backDesc, noPrompt);
		else
		{
			strcpy (local_rec.backDesc, yesPrompt);
			if (local_rec.onight [0] == 'Y')
			{
				strcpy (local_rec.onight, "N");
				strcpy (local_rec.onightDesc, noPrompt);

				DSP_FLD ("onight");
				DSP_FLD ("onightDesc");
			}
		}

		DSP_FLD ("back");
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onight"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.onight);

		if (valid_inp [0] == 'N')
			strcpy (local_rec.onightDesc, noPrompt);
		else
		{
			if (local_rec.back [0] == 'Y')
			{
				strcpy (local_rec.back, "N");
				strcpy (local_rec.backDesc, noPrompt);

				DSP_FLD ("back");
				DSP_FLD ("backDesc");
			}
			strcpy (local_rec.onightDesc, yesPrompt);
		}

		DSP_FLD ("onight");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("sel1"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 5]);

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [0], "N");
			strcpy (local_rec.reportType [1], "N");
			strcpy (local_rec.reportType [2], "N");
			strcpy (local_rec.reportType [3], "N");
			strcpy (local_rec.reportType [4], "N");
			strcpy (local_rec.reportType [5], "N");
			strcpy (local_rec.reportType [6], "N");

			strcpy (local_rec.reportTypeDesc [0], noPrompt);
			strcpy (local_rec.reportTypeDesc [1], noPrompt);
			strcpy (local_rec.reportTypeDesc [2], noPrompt);
			strcpy (local_rec.reportTypeDesc [3], noPrompt);
			strcpy (local_rec.reportTypeDesc [4], noPrompt);
			strcpy (local_rec.reportTypeDesc [5], noPrompt);
			strcpy (local_rec.reportTypeDesc [6], noPrompt);
		}

		strcpy (local_rec.reportType [field - 5],
		      (valid_inp [0] == 'Y') ? "Y" : "N");
		strcpy (local_rec.reportTypeDesc [field - 5],
		      (valid_inp [0] == 'Y') ? yesPrompt : noPrompt);

		for (i = 5; i < 12; i++)
		{
			if (!strcmp (local_rec.reportType [i - 5],"Y"))
			{
				sprintf (reportType, "%d", i - 4);
				found_yes++;
			}
		}

		if (found_yes == 0)
		{
			strcpy (reportType, "5");
			strcpy (local_rec.reportType [4], "Y");
			strcpy (local_rec.reportTypeDesc [4], yesPrompt);
		}

		for (i = 5; i < 12; i++)
		{
			display_field (i);
			display_field (i + 7);
		}
				
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Field Selection group 2. |
	-----------------------------------*/
	if (LCHECK ("sel2"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 16]);

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [7], "N");
			strcpy (local_rec.reportType [8], "N");
			strcpy (local_rec.reportType [9], "N");

			strcpy (local_rec.reportTypeDesc [7], noPrompt);
			strcpy (local_rec.reportTypeDesc [8], noPrompt);
			strcpy (local_rec.reportTypeDesc [9], noPrompt);
		}

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [field - 16], "Y");
			strcpy (local_rec.reportTypeDesc [field - 16], yesPrompt);
		}
		else 
		{
			strcpy (local_rec.reportType [field - 16], "N");
			strcpy (local_rec.reportTypeDesc [field - 16], noPrompt);
		}

		for (i = 23; i < 26; i++)
		{
			if (!strcmp (local_rec.reportType [i - 16],"Y"))
			{
				sprintf (runType,"%d",i - 22);
				found_yes++;
			}
		}

		if (found_yes == 0)
		{
			strcpy (runType,"1");
			strcpy (local_rec.reportType [7],"Y");
			strcpy (local_rec.reportTypeDesc [7],yesPrompt);
		}
		
		for (i = 23; i < 26; i++)
		{
			display_field (i);
			display_field (i + 3);
		}

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Department Number Input. |
	-----------------------------------*/
	if (LCHECK ("dep"))
	{
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, local_rec.departmentNo);

		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------------
			| Department is not on file...|
			-----------------------------*/
			errmess (ML (mlStdMess084));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		PrCoLine ();
		return (EXIT_SUCCESS);
	}
		
	/*-----------------------------------
	| Validate Field Selection group 3. |
	-----------------------------------*/
	if (LCHECK ("sel3"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 20]);

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [10], "N");
			strcpy (local_rec.reportType [11], "N");

			strcpy (local_rec.reportTypeDesc [10], noPrompt);
			strcpy (local_rec.reportTypeDesc [11], noPrompt);
		}

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [field - 20], "Y");
			strcpy (local_rec.reportTypeDesc [field - 20], yesPrompt);
		}
		else 
		{
			strcpy (local_rec.reportType [field - 20], "N");
			strcpy (local_rec.reportTypeDesc [field - 20], noPrompt);
		}

		for (i = 30; i < 32; i++)
		{
			if (!strcmp (local_rec.reportType [i - 20],"Y"))
			{
				sprintf (sortType,"%d",i - 29);
				found_yes++;
			}
		}
		if (found_yes == 0)
		{
			strcpy (sortType,"2");
			strcpy (local_rec.reportType [11],"Y");
			strcpy (local_rec.reportTypeDesc [11],yesPrompt);
		}

		for (i = 30; i < 32; i++)
		{
			display_field (i);
			display_field (i + 2);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sel5"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 22]);

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [12], "N");
			strcpy (local_rec.reportType [13], "N");

			strcpy (local_rec.reportTypeDesc [12], noPrompt);
			strcpy (local_rec.reportTypeDesc [13], noPrompt);
		}

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [field - 22], "Y");
			strcpy (local_rec.reportTypeDesc [field - 22], yesPrompt);
		}
		else 
		{
			strcpy (local_rec.reportType [field - 22], "N");
			strcpy (local_rec.reportTypeDesc [field - 22], noPrompt);
		}

		for (i = 34; i < 36; i++)
		{
			if (!strcmp (local_rec.reportType [i - 22],"Y"))
			{
				sprintf (salesCustomer,"%d",i - 33);
				found_yes++;
			}
		}
		if (found_yes == 0)
		{
			strcpy (salesCustomer,"1");
			strcpy (local_rec.reportType [11],"Y");
			strcpy (local_rec.reportTypeDesc [11],yesPrompt);
		}

		for (i = 34; i < 36; i++)
		{
			display_field (i);
			display_field (i + 2);
		}

		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 4. |
	-----------------------------------*/
	if (LCHECK ("sel4"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 24]);

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [14], "N");
			strcpy (local_rec.reportType [15], "N");
			strcpy (local_rec.reportType [16], "N");

			strcpy (local_rec.reportTypeDesc [14], noPrompt);
			strcpy (local_rec.reportTypeDesc [15], noPrompt);
			strcpy (local_rec.reportTypeDesc [16], noPrompt);
		}

		if (valid_inp [0] == 'Y')
		{
			strcpy (local_rec.reportType [field - 24], "Y");
			strcpy (local_rec.reportTypeDesc [field - 24], yesPrompt);
		}
		else 
		{
			strcpy (local_rec.reportType [field - 24], "N");
			strcpy (local_rec.reportTypeDesc [field - 24], noPrompt);
		}

		for (i = 38; i < 41; i++)
		{
			if (!strcmp (local_rec.reportType [i - 24],"Y"))
			{
				sprintf (balType,"%d",i - 37);
				found_yes++;
			}
		}
		if (found_yes == 0)
		{
			strcpy (local_rec.reportType [14],"Y");
			strcpy (local_rec.reportTypeDesc [14],yesPrompt);
			strcpy (balType,"1");
		}

		for (i = 38; i < 41; i++)
		{
			display_field (i);
			display_field (i + 3);
		}

		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Field Selection group 6. |
	-----------------------------------*/
	if (LCHECK ("sel6"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.reportType [field - 27]);

		if (valid_inp [0] == 'M')
			strcpy (local_rec.reportTypeDesc [17], "M (odule");

		if (valid_inp [0] == 'M')
		{
			strcpy (local_rec.reportType [field - 27], "M");
			strcpy (local_rec.reportTypeDesc [field - 27], "Module");
			strcpy (ageType,"M");
		}
		else 
		{
			strcpy (local_rec.reportType [field - 27], "T");
			strcpy (local_rec.reportTypeDesc [field - 27], "True");
			strcpy (ageType,"T");
		}
		DSP_FLD ("sel6");
		DSP_FLD ("sel6_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("currencyDisp"))
	{
		if (local_rec.currencyDisp [0] == 'L')
			strcpy (local_rec.currenctDisplayDesc, "Local");
		else
			strcpy (local_rec.currenctDisplayDesc, "Overseas");

		DSP_FLD ("currenctDisplayDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("currencySort"))
	{
		if (local_rec.currencySort [0] == 'Y')
		{
			strcpy (local_rec.currencySortDesc, yesPrompt);
			FLD ("currencyDisp") = YES;
			FLD ("startCurrency") = YES;
			FLD ("endCurrency") = YES;
		}
		else
		{
			strcpy (local_rec.currencySortDesc, "No ");
			FLD ("currencyDisp") = NA;
			FLD ("startCurrency") = NA;
			FLD ("endCurrency") = NA;
		
			strcpy (local_rec.currencyDisp, "L");
			strcpy (local_rec.currenctDisplayDesc, "Local");

			strcpy (local_rec.startCurrency, "   ");
			strcpy (local_rec.endCurrency, "~~~");
			sprintf (local_rec.startCurrencyDesc, "%-40.40s", "Start Currency");
			sprintf (local_rec.endCurrencyDesc, "%-40.40s", "End Currency");
			DSP_FLD ("currencyDisp");
			DSP_FLD ("currenctDisplayDesc");
			DSP_FLD ("startCurrency");
			DSP_FLD ("startCurrencyDesc");
			DSP_FLD ("endCurrency");
			DSP_FLD ("endCurrencyDesc");
		}

		DSP_FLD ("currencySortDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCurrency"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCurrency, "   ");
			sprintf (local_rec.startCurrencyDesc, "%-40.40s", "Start Currency");
			DSP_FLD ("startCurrency");
			DSP_FLD ("startCurrencyDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", local_rec.startCurrency);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------------
			| Currency Code Not Found |
			-------------------------*/
			print_mess (ML (mlStdMess040)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startCurrencyDesc, "%-40.40s", pocr_rec.description);
		DSP_FLD ("startCurrencyDesc");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCurrency"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCurrency, "~~~");
			sprintf (local_rec.endCurrencyDesc, "%-40.40s", "End Currency");
			DSP_FLD ("endCurrency");
			DSP_FLD ("endCurrencyDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", local_rec.endCurrency);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------------
			| Currency Code Not Found |
			-------------------------*/
			print_mess (ML (mlStdMess040)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endCurrencyDesc, "%-40.40s", pocr_rec.description);
		DSP_FLD ("endCurrencyDesc");
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchPocr (
 char*              key_val)
{
	work_open ();
	save_rec ("#No.","#Currency Code Description");                       
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
        while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no) && 
		      !strncmp (pocr_rec.code,key_val,strlen (key_val)))
    	{                        
	        cc = save_rec (pocr_rec.code, pocr_rec.description);                       
		if (cc)
		        break;

		cc = find_rec (pocr,&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
}

/*=========================
| Close data base files . |
=========================*/
int
CloseDB (void)
{
	abc_fclose (cudp);
	abc_fclose (pocr);
	abc_dbclose (data);
	return (cc);
}

void
OpenDB (void)
{
	abc_dbopen (data);

    /*============================================ 
    | Get common info from common database file. |
    ============================================*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

void
SetupDefault (void)
{
	local_rec.printerNo = 1;
	strcpy (local_rec.back,"N");
	strcpy (local_rec.backDesc,noPrompt);

	strcpy (local_rec.onight,"N");
	strcpy (local_rec.onightDesc,noPrompt);

	strcpy (local_rec.reportType [0],"N");
	strcpy (local_rec.reportTypeDesc [0],noPrompt);
	strcpy (local_rec.reportType [1],"N");
	strcpy (local_rec.reportTypeDesc [1],noPrompt);
	strcpy (local_rec.reportType [2],"N");
	strcpy (local_rec.reportTypeDesc [2],noPrompt);
	strcpy (local_rec.reportType [3],"N");
	strcpy (local_rec.reportTypeDesc [3],noPrompt);
	strcpy (local_rec.reportType [4],"Y");
	strcpy (local_rec.reportTypeDesc [4],yesPrompt);
	strcpy (local_rec.reportType [5],"N");
	strcpy (local_rec.reportTypeDesc [5],noPrompt);
	strcpy (local_rec.reportType [6],"N");
	strcpy (local_rec.reportTypeDesc [6],noPrompt);
	strcpy (local_rec.reportType [7],"Y");
	strcpy (local_rec.reportTypeDesc [7],yesPrompt);
	strcpy (local_rec.reportType [8],"N");
	strcpy (local_rec.reportTypeDesc [8],noPrompt);
	strcpy (local_rec.reportType [9],"N");
	strcpy (local_rec.reportTypeDesc [9],noPrompt);
	strcpy (local_rec.reportType [10],"N");
	strcpy (local_rec.reportTypeDesc [10],noPrompt);
	strcpy (local_rec.reportType [11],"Y");
	strcpy (local_rec.reportTypeDesc [11],yesPrompt);

	if (SUMMARY)
	{
		strcpy (local_rec.reportType [12],"N");
		strcpy (local_rec.reportTypeDesc [12],noPrompt);
		strcpy (local_rec.reportType [13],"N");
		strcpy (local_rec.reportTypeDesc [13],noPrompt);
	}

	strcpy (local_rec.reportType [14],"Y");
	strcpy (local_rec.reportTypeDesc [14],yesPrompt);
	strcpy (local_rec.reportType [15],"N");
	strcpy (local_rec.reportTypeDesc [15],noPrompt);
	strcpy (local_rec.reportType [16],"N");
	strcpy (local_rec.reportTypeDesc [16],noPrompt);
	strcpy (local_rec.reportType [17],"M");
	strcpy (local_rec.reportTypeDesc [17],"Module");

	strcpy (reportType,       "5");
	strcpy (runType,       "1");
	strcpy (sortType,      "2");
	strcpy (balType,       "1");
	strcpy (cudp_rec.dp_no, "  ");
	strcpy (salesCustomer,      "0");
	strcpy (ageType,       "M");

	strcpy (local_rec.currencyDisp, "L");
	strcpy (local_rec.currenctDisplayDesc, "Local");
	strcpy (local_rec.currencySort, "N");
	strcpy (local_rec.currencySortDesc, noPrompt);

	strcpy (local_rec.startCurrency, "   ");
	sprintf (local_rec.startCurrencyDesc, "%-40.40s", "Start Currency");
	strcpy (local_rec.endCurrency, "~~~");
	sprintf (local_rec.endCurrencyDesc, "%-40.40s", "End Currency");

	local_rec.DaysAgeing = envDbDaysAgeing;
	FLD ("currencyDisp") = NA;
	FLD ("startCurrency") = NA;
	FLD ("endCurrency") = NA;
}

void
RunProgram (void)
{
	char	runProgram	[26];
	char	wk_dept [3];
	char	reportValue [4][13];
	char	AgeDays [3];
	
	clear ();
	snorm ();
	rset_tty ();
	print_at (0,0, ML (mlStdMess035)); 
	fflush (stdout);
	strcpy (wk_dept, "  ");
	sprintf (AgeDays, "%02d", local_rec.DaysAgeing);

	sprintf (reportValue [0], "%12.2f", DOLLARS (local_rec.reportValue [0]));
	sprintf (reportValue [1], "%12.2f", DOLLARS (local_rec.reportValue [1]));
	sprintf (reportValue [2], "%12.2f", DOLLARS (local_rec.reportValue [2]));
	sprintf (reportValue [3], "%12.2f", DOLLARS (local_rec.reportValue [3]));

	if (runType [0] == '3')
	{	
		strcpy (wk_dept, local_rec.departmentNo);
		if (!strcmp (local_rec.departmentNo, "  "))
			strcpy (runType,"1");
	}
	if (programType [0] == 'S')
		strcpy (runProgram, "db_lageing");
	else if (programType [0] == 'E')
		strcpy (runProgram, "db_xlfullpr");
	else if (programType [0] == 'D')
		strcpy (runProgram, "db_lfullpr");

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str,
			"ONIGHT \"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			runProgram,
			local_rec.printerNo,
			reportType,
			runType,
			balType,
			sortType,
			salesCustomer,
			ageType,
			reportValue [0],
			reportValue [1],
			reportValue [2],
			reportValue [3],
			local_rec.currencyDisp,
			local_rec.currencySort,
			local_rec.startCurrency,
			local_rec.endCurrency,
			AgeDays,
			wk_dept,
			programDesc
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str,
			"\"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			runProgram,
			local_rec.printerNo,
			reportType,
			runType,
			balType,
			sortType,
			salesCustomer,
			ageType,
			reportValue [0],
			reportValue [1],
			reportValue [2],
			reportValue [3],
			local_rec.currencyDisp,
			local_rec.currencySort,
			local_rec.startCurrency,
			local_rec.endCurrency,
			AgeDays,
			wk_dept
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*
 * Heading concerns itself with clearing the screen, painting the
 * screen overlay in preparation for input                     
 */
int
heading (
	int		scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	switch (programType [0])
	{
	case	'S':
		rv_pr (ML (mlDbMess057), 48,0,1);
		strcpy (programDesc," Summary Ageing Report Selection. ");
		break;

	case	'E':
		rv_pr (ML (mlDbMess058), 48,0,1);
		strcpy (programDesc," Expanded Detailed Ageing Report Selection ");
		break;

	case	'D':
		rv_pr (ML (mlDbMess060), 48,0,1);
		strcpy (programDesc," Detailed Ageing Report Selection. ");
		break;

	default:
		break;
	}

	if (scn == 1)
	{
		cl_box (0, 1, 132, 19);
	
		line_at (4,1,131);
		line_at (11,1,131);
		line_at (14,1,131);
		line_at (17,1,131);
	}
	
	if (scn == 2)
	{
		cl_box (0, 2, 132, 6);
	
		line_at (4,1,131);
		line_at (6,1,131);
		line_at (21,0,131);
	}

	PrCoLine ();
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
PrCoLine (void)
{
	sprintf (err_str, ML (mlStdMess038), 
						comm_rec.co_no, clip (comm_rec.co_short));
	print_at (22,0, "%s", err_str);
	
	sprintf (err_str, ML (mlStdMess039), 
						comm_rec.est_no,clip (comm_rec.est_name));
	print_at (22,45, "%s", err_str);

	sprintf (err_str, ML (mlStdMess085), 
						cudp_rec.dp_no, clip (cudp_rec.dp_name));
	print_at (22,90, "%s", err_str);
}

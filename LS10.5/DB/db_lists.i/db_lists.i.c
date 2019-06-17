/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lists.i.c,v 5.4 2002/07/17 09:57:08 scott Exp $
|  Program Name  : (db_lists.i.c)
|  Program Desc  : (Selection for customer listings)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/01/87         |
|---------------------------------------------------------------------|
| $Log: db_lists.i.c,v $
| Revision 5.4  2002/07/17 09:57:08  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/12/04 05:08:57  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lists.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lists.i/db_lists.i.c,v 5.4 2002/07/17 09:57:08 scott Exp $";

#include <time.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

#define TOTSCNS		1

	                        /*========================================*/
	int		reportType = 0,	/*| Holds report type.                   |*/
			runType = 0,	/*| Holds run type.                      |*/
			sortType = 0;	/*| Holds sort type.                     |*/
                            /*========================================*/

	char	noPrompt	[11],
			yesPrompt	[11];


	extern	int	TruePosition;
	extern	int	EnvScreenOK;

#include	"schema"

struct commRecord	comm_rec;

	char	*data = "data";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	back [2];
	char 	backDesc [8];
	char 	reportType [15][2];
	char 	reportTypeDesc [15][8];
	char 	onight [2];
	char 	onightDesc [8];
	int  	printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "printerNo",	 2, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number                          ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background Y/N                          ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 3, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "YN", "", local_rec.backDesc},
	{1, LIN, "onight",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight Y/N                           ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc", 4, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "YN", "", local_rec.onightDesc},
	{1, LIN, "sel1",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Full Name and Address Listing.          ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [0]},
	{1, LIN, "sel1Desc", 6, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [0]},
	{1, LIN, "sel2",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Short Name and Address Listing.         ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [1]},
	{1, LIN, "sel2Desc", 7, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [1]},
	{1, LIN, "sel3",	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Full Name, Address and Delivery Listing ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [2]},
	{1, LIN, "sel3Desc", 8, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [2]},
	{1, LIN, "sel4",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Full Master File Listing                ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [3]},
	{1, LIN, "sel4Desc", 9, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [3]},
	{1, LIN, "sel5",	10, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Stop Credit Listing                     ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [4]},
	{1, LIN, "sel5Desc", 10, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [4]},
	{1, LIN, "sel6",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Customer Payment Term Listing           ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [5]},
	{1, LIN, "sel6Desc", 11, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [5]},

	{1, LIN, "sel7",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Report By Company                       ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [6]},
	{1, LIN, "sel7Desc", 13, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [6]},
	{1, LIN, "sel8",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Report By Branch                        ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [7]},
	{1, LIN, "sel8Desc", 14, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [7]},

	{1, LIN, "sel9",	16, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort By Customer Number                 ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [8]},
	{1, LIN, "sel9Desc", 16, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [8]},
	{1, LIN, "sel10",	17, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort by Customer Acronym                ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [9]},
	{1, LIN, "sel10Desc", 17, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [9]},
	{1, LIN, "sel11",	18, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort by Salesman                        ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [10]},
	{1, LIN, "sel11Desc", 18, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [10]},
	{1, LIN, "sel12",	19, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort by Cust. Type/Salesman             ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [11]},
	{1, LIN, "sel12Desc", 19, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [11]},
	{1, LIN, "sel13",	20, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Sort by Salesman/Customer Type.         ", "Enter Y(es) or N(o). ",
		YES, NO,  JUSTLEFT, "YyNn", "", local_rec.reportType [12]},
	{1, LIN, "sel13Desc", 20, 45, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reportTypeDesc [12]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
int 	spec_valid 			(int);
void 	CloseDB 			(void);
void 	SetupDefault		(void);
void 	RunProgram 			(void);
int 	heading 			(int);
void 	CoLine 				(void);
void 	ResetGroupOne 		(void);
void 	ResetGroupTwo 		(void);
void 	ResetGroupThree		(void);
void 	ResetGroupFour 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*==============================
	| Read common terminal record. |
	==============================*/
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*=====================
	| Reset control flags |
	=====================*/
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
	edit (1);
	prog_exit = 1;
	if (restart) 
    {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	strcpy (err_str, ML (mlDbMess230));	
	if (!restart) 
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
	int		field)
{
	char	inputValue [2];
	/*
	 * Validate Field Selection background option.
	 */
	if (LCHECK ("back")) 
	{
		sprintf (inputValue, "%1.1s", local_rec.back);

		if (inputValue [0] == 'N')
			strcpy (local_rec.backDesc, noPrompt);
		else
		{
			strcpy (local_rec.backDesc, yesPrompt);
			if (local_rec.onight [0] == 'Y')
			{
				strcpy (local_rec.onightDesc, noPrompt);
				strcpy (local_rec.onight, 	  "N");
			}
		}

		DSP_FLD ("backDesc");
		DSP_FLD ("onight");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection overnight option.
	 */
	if (LCHECK ("onight")) 
	{
		sprintf (inputValue, "%1.1s", local_rec.onight);

		if (inputValue [0] == 'N')
			strcpy (local_rec.onightDesc, noPrompt);
		else
		{
			if (local_rec.back [0] == 'Y')
			{
				strcpy (local_rec.backDesc, noPrompt);
				strcpy (local_rec.back, 	"N");
			}
			strcpy (local_rec.onightDesc, yesPrompt);
		}

		DSP_FLD ("back");
		DSP_FLD ("backDesc");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
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
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel1")) 
	{
		strcpy (inputValue, local_rec.reportType [0]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [0], inputValue);
		strcpy (local_rec.reportTypeDesc [0], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel1Desc");
		reportType = 1;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel2")) 
	{
		strcpy (inputValue, local_rec.reportType [1]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [1], inputValue);
		strcpy (local_rec.reportTypeDesc [1], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel2Desc");
		reportType = 2;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel3")) 
	{
		strcpy (inputValue, local_rec.reportType [2]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [2], inputValue);
		strcpy (local_rec.reportTypeDesc [2], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel3Desc");
		reportType = 3;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel4")) 
	{
		strcpy (inputValue, local_rec.reportType [3]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [3], inputValue);
		strcpy (local_rec.reportTypeDesc [3], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel4Desc");
		reportType = 4;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel5")) 
	{
		strcpy (inputValue, local_rec.reportType [4]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [4], inputValue);
		strcpy (local_rec.reportTypeDesc [4], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel5Desc");
		reportType = 5;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel6")) 
	{
		strcpy (inputValue, local_rec.reportType [5]);

		ResetGroupOne ();

		strcpy (local_rec.reportType [5], inputValue);
		strcpy (local_rec.reportTypeDesc [5], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel6Desc");
		reportType = 6;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel7")) 
	{
		strcpy (inputValue, local_rec.reportType [6]);

		ResetGroupTwo ();

		strcpy (local_rec.reportType [6], inputValue);
		strcpy (local_rec.reportTypeDesc [6], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel7Desc");
		runType = 1;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel8")) 
	{
		strcpy (inputValue, local_rec.reportType [7]);

		ResetGroupTwo ();

		strcpy (local_rec.reportType [7], inputValue);
		strcpy (local_rec.reportTypeDesc [7], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel8Desc");
		runType = 2;
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("sel9")) 
	{
		strcpy (inputValue, local_rec.reportType [8]);

		ResetGroupThree ();

		strcpy (local_rec.reportType [8], inputValue);
		strcpy (local_rec.reportTypeDesc [8], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		sortType = 1;
		DSP_FLD ("sel9Desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sel10")) 
	{
		strcpy (inputValue, local_rec.reportType [9]);

		ResetGroupThree ();

		strcpy (local_rec.reportType [9], inputValue);
		strcpy (local_rec.reportTypeDesc [9], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel10Desc");
		sortType = 2;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel11")) 
	{
		strcpy (inputValue, local_rec.reportType [10]);

		strcpy (local_rec.reportType [10], inputValue);
		strcpy (local_rec.reportTypeDesc [10], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		if (local_rec.reportType [10][0] == 'Y')
		{
			if (local_rec.reportType [8][0] == 'Y')
				sortType = 3;
			if (local_rec.reportType [9][0] == 'Y')
				sortType = 4;
			if (local_rec.reportType [11][0] == 'Y' ||
			    local_rec.reportType [12][0] == 'Y')
			{
				strcpy (local_rec.reportType [10], "N");
				strcpy (local_rec.reportTypeDesc [10], noPrompt);
			}
			DSP_FLD ("sel11");
			DSP_FLD ("sel11Desc");
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel12")) 
	{
		strcpy (inputValue, local_rec.reportType [11]);

		ResetGroupFour ();

		strcpy (local_rec.reportType [11], inputValue);
		strcpy (local_rec.reportTypeDesc [11], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);
	
		DSP_FLD ("sel12Desc");
		sortType = 5;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("sel13")) 
	{
		strcpy (inputValue, local_rec.reportType [12]);

		ResetGroupFour ();
	
		strcpy (local_rec.reportType [12], inputValue);
		strcpy (local_rec.reportTypeDesc [12], 
				(inputValue [0] == 'Y') ? yesPrompt : noPrompt);

		DSP_FLD ("sel13Desc");
		sortType = 6;
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}
void
ResetGroupOne (void)
{
	int		i;
	char	selPrompt 		[11],
			selPromptDesc	[11];

	strcpy (local_rec.reportType [0], "N");
	strcpy (local_rec.reportType [1], "N");
	strcpy (local_rec.reportType [2], "N");
	strcpy (local_rec.reportType [3], "N");
	strcpy (local_rec.reportType [4], "N");
	strcpy (local_rec.reportType [5], "N");

	strcpy (local_rec.reportTypeDesc [0], noPrompt);
	strcpy (local_rec.reportTypeDesc [1], noPrompt);
	strcpy (local_rec.reportTypeDesc [2], noPrompt);
	strcpy (local_rec.reportTypeDesc [3], noPrompt);
	strcpy (local_rec.reportTypeDesc [4], noPrompt);
	strcpy (local_rec.reportTypeDesc [5], noPrompt);
	for (i = 1; i < 7; i++)
	{
		sprintf (selPrompt, "sel%d", i);
		sprintf (selPromptDesc, "sel%dDesc", i);
		DSP_FLD (selPrompt);
		DSP_FLD (selPromptDesc);
	}
}
void
ResetGroupTwo (void)
{
	int		i;
	char	selPrompt 		[11],
			selPromptDesc	[11];

	strcpy (local_rec.reportType [6], "N");
	strcpy (local_rec.reportType [7], "N");

	strcpy (local_rec.reportTypeDesc [6], noPrompt);
	strcpy (local_rec.reportTypeDesc [7], noPrompt);
	for (i = 7; i < 9; i++)
	{
		sprintf (selPrompt, "sel%d", i);
		sprintf (selPromptDesc, "sel%dDesc", i);
		DSP_FLD (selPrompt);
		DSP_FLD (selPromptDesc);
	}
}
void
ResetGroupThree (void)
{
	int		i;
	char	selPrompt 		[11],
			selPromptDesc	[11];

	strcpy (local_rec.reportType [8], "N");
	strcpy (local_rec.reportType [9], "N");

	strcpy (local_rec.reportTypeDesc [8], noPrompt);
	strcpy (local_rec.reportTypeDesc [9], noPrompt);
	for (i = 9; i < 11; i++)
	{
		sprintf (selPrompt, "sel%d", i);
		sprintf (selPromptDesc, "sel%dDesc", i);
		DSP_FLD (selPrompt);
		DSP_FLD (selPromptDesc);
	}
}
void
ResetGroupFour (void)
{
	int		i;
	char	selPrompt 		[11],
			selPromptDesc	[11];

	strcpy (local_rec.reportType [8], "N");
	strcpy (local_rec.reportType [9], "N");
	strcpy (local_rec.reportType [10], "N");
	strcpy (local_rec.reportType [11], "N");
	strcpy (local_rec.reportType [12], "N");

	strcpy (local_rec.reportTypeDesc [8], noPrompt);
	strcpy (local_rec.reportTypeDesc [9], noPrompt);
	strcpy (local_rec.reportTypeDesc [10], noPrompt);
	strcpy (local_rec.reportTypeDesc [11], noPrompt);
	strcpy (local_rec.reportTypeDesc [12], noPrompt);
	for (i = 9; i < 14; i++)
	{
		sprintf (selPrompt, "sel%d", i);
		sprintf (selPromptDesc, "sel%dDesc", i);
		DSP_FLD (selPrompt);
		DSP_FLD (selPromptDesc);
	}
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_dbclose (data);
}


void
SetupDefault (void)
{
	strcpy (noPrompt, ML ("(NO) "));
	strcpy (yesPrompt, ML ("(YES)"));

	local_rec.printerNo = 1;
	strcpy (local_rec.back,		"N");
	strcpy (local_rec.backDesc,	noPrompt);
	strcpy (local_rec.onight,	"N");
	strcpy (local_rec.onightDesc,noPrompt);
	strcpy (local_rec.reportType [0], "Y");
	strcpy (local_rec.reportType [1], "N");
	strcpy (local_rec.reportType [2], "N");
	strcpy (local_rec.reportType [3], "N");
	strcpy (local_rec.reportType [4], "N");
	strcpy (local_rec.reportType [5], "N");
	strcpy (local_rec.reportType [6], "Y");
	strcpy (local_rec.reportType [7], "N");
	strcpy (local_rec.reportType [8], "N");
	strcpy (local_rec.reportType [9], "Y");
	strcpy (local_rec.reportType [10],"N");
    strcpy (local_rec.reportType [11],"N");
    strcpy (local_rec.reportType [12],"N");
	strcpy (local_rec.reportTypeDesc [0],  yesPrompt);
	strcpy (local_rec.reportTypeDesc [1],  noPrompt);
	strcpy (local_rec.reportTypeDesc [2],  noPrompt);
	strcpy (local_rec.reportTypeDesc [3],  noPrompt);
	strcpy (local_rec.reportTypeDesc [4],  noPrompt);
	strcpy (local_rec.reportTypeDesc [5],  noPrompt);
	strcpy (local_rec.reportTypeDesc [6],  yesPrompt);
	strcpy (local_rec.reportTypeDesc [7],  noPrompt);
	strcpy (local_rec.reportTypeDesc [8],  noPrompt);
	strcpy (local_rec.reportTypeDesc [9],  yesPrompt);
	strcpy (local_rec.reportTypeDesc [10], noPrompt);
    strcpy (local_rec.reportTypeDesc [11], noPrompt);
    strcpy (local_rec.reportTypeDesc [12], noPrompt);
	reportType = 1;
	runType = 1;
	sortType = 2;
}

void
RunProgram (void)
{
	rset_tty ();

	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);
	CloseDB (); 
	FinishProgram ();

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y')
	{ 
		sprintf
		(
			err_str, 
			"ONIGHT \"%s\" \"%02d\" \"%02d\" \"%02d\" \"%02d\" \"%s\"",
			"db_lists",
			local_rec.printerNo,
			reportType,
			runType,
			sortType,
			err_str
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf
		(
			err_str, 
			"\"%s\" \"%02d\" \"%02d\" \"%02d\" \"%02d\"",
			"db_lists",
			local_rec.printerNo,
			reportType,
			runType,
			sortType
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlDbMess129),25,0,1);

		line_at (5,1,79);
		line_at (12,1,79);
		line_at (15,1,79);
		line_at (1,0,80);

		if (scn == 1)
			box (0,1,80,19);

		CoLine ();
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
CoLine (void)
{
	print_at (22,0, ML (mlStdMess038),comm_rec.co_no,  comm_rec.co_short); 
	print_at (23,0, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
}

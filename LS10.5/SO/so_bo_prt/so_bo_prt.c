/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_bo_prt.c,v 5.12 2002/07/17 09:58:02 scott Exp $
|  Program Name  : (so_bo_prt.c)
|  Program Desc  : (Print Backorder Report By Customer OR (By Item)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 29/10/88         |
|---------------------------------------------------------------------|
| $Log: so_bo_prt.c,v $
| Revision 5.12  2002/07/17 09:58:02  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.11  2002/01/21 09:43:19  scott
| ..
|
| Revision 5.10  2001/11/08 08:59:14  cha
| Fixed some header allignment issue.
|
| Revision 5.9  2001/11/08 06:18:50  cha
| Updated to passed value when executing program.
| Previously NULL values is not recognize in GUI.
|
| Revision 5.8  2001/11/06 03:35:58  scott
| Updated for SysExex
|
| Revision 5.7  2001/11/05 06:23:01  scott
| Updated to only show backorder qty if status not "B"
|
| Revision 5.6  2001/11/05 04:32:14  scott
| Fixed group problems.
|
| Revision 5.5  2001/10/24 08:03:33  scott
| Updated to ensure reports are lined up.
|
| Revision 5.4  2001/10/24 06:45:44  scott
| Updated to fix lineup, basically a total rewrite.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_bo_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bo_prt/so_bo_prt.c,v 5.12 2002/07/17 09:58:02 scott Exp $";

#include <pslscr.h>
#include <signal.h>
#include <get_lpno.h>
#include <twodec.h>
#include <assert.h>

#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	DISPLAY_REP	(rep_type [0] == 'D')
#define	CO_REP  	(local_rec.coWh [0] == 'C')

#define	ITEM	0
#define	CUST	1
#define	STK	2

#define	LOWER	label ("lower")	/* field number of lower field		*/
#define	UPPER	label ("upper")	/* field number of upper field		*/

#ifdef PSIZE
#undef PSIZE
#endif
#define	PSIZE	15

#define	RESET_LN_NUM	lineNum = (lineNum >= PSIZE) ? lineNum % PSIZE : lineNum++

#define	SOLN_BACKORDER (soln_rec.status [0] == 'B' || soln_rec.qty_bord > 0.0)

#define	ITLN_BACKORDER (itln_rec.status [0] == 'B' || itln_rec.qty_border > 0.0)

#define	BLNK		0
#define	LNE		1
#define	SUB_TOT		2
#define	BIG_TOT		3
#define	MAXITEMS	200

#define	DESCRIPTIVE	(inmr_rec.inmr_class [0] == 'Z')

char	*UNDERLINE = "========================================================================================================================================";
char	*SEPARATOR = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

char	*SEPARATOR1 = "                         ^E                                                     ^E           ^E             ^E         ^E         ^E         ^E     ";
char	*SEPARATOR2 = "                         ^E                                                     ^E           ^E             ^E         ^E         ^E         ^E     ";
char	*C_HEADER = " WH  CUST.NO/         |    CUSTOMER    |   ORDER  | UOM |  QUANTITY |   UNIT   |  EXTENDED  |    PURCHASE   | DUE DATE |   QTY   ";
char	*C_WHEADER= "  CUSTOMER NO.        |    CUSTOMER    |   ORDER  | UOM |  QUANTITY |   UNIT   |  EXTENDED  |    PURCHASE   | DUE DATE |   QTY   ";
char	*C_HEADER1= "ORDER # / ITEM #      |   ORDER NUMBER |   DATE   |     | BACKORDER |   PRICE  |   VALUE    |     NUMBER    |          |         ";

char	*I_HEADER =  " WH  ITEM NO /        |    CUSTOMER    |   ORDER  | UOM | QUANTITY  |   UNIT   |   EXT LCL  |    PURCHASE   | DUE DATE |   QTY   ";
char	*I_WHEADER = "   ITEM  NO /         |    CUSTOMER    |   ORDER  | UOM | QUANTITY  |   UNIT   |   EXT LCL  |    PURCHASE   | DUE DATE |   QTY   ";
char	*I_HEADER1 = "CUSTOMER/ORDER #      |   ORDER NUMBER |   DATE   |     | BACKORDER |   PRICE  |    VALUE   |     NUMBER    |          |         ";

char	*KEY = " [REDRAW] [NEXT SCREEN] [PREVIOUS SCREEN] [EDIT / END]";

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct posoRecord	poso_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inexRecord	inex_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;

	char	*ccmr2	=	"ccmr2",
			*poln2	=	"poln2",
			*inmr2	=	"inmr2",
			*srt_offset [256];

	int		envVarCrCo			= 0,
			envVarDbFind		= 0,
			envVarDbNettUsed 	= TRUE,
			envVarRepTax 		= 0,
			envVarDbMcurr		= 0,
			lineNum 			= 0,
			firstFlag 			= 0,
			recordFound 		= FALSE,
			printed 			= FALSE,
			reportType			= 0,
			noSortFields		= 0;

	char	transferType,
			branchNo [3],
			systemDate [11],
			dataString [300],
			rep_type [2];

	extern	int	TruePosition;

	float	tot_po [3]	=	{0,0,0};
	float	tot_qty [4]	=	{0,0,0,0};
	double	tot_ext [3] = 	{0,0,0};

	char	*itemMask 		= "UUUUUUUUUUUUUUUU";
	char	*customerMask 	= "UUUUUU";

	FILE	*fout;
	FILE	*fsort;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy 			[11];
	char	custItem 		[2];
	char	custItemDesc 	[21];
	char	startCustomer 	[7];
	char	endCustomer 	[7];
	char	startItem 		[17];
	char	endItem 		[17];
	char	coWh 			[2];
	char	coWhDesc 		[21];
	char	dateRequired 	[11];
	char	transfers 		[2];
	char	transfersDesc 	[21];
	char 	back 			[2];
	char 	backDesc 		[21];
	char	onite 			[2];
	char	oniteDesc 		[21];
	char	reportDesc 		[31];
	char	cuttoffDate 	[11];
	char	variableMask 	[17];
	char	lower 			[17];
	char	lowerDesc 		[41];
	char	upper 			[17];
	char	upperDesc 		[41];

	long	inputDate;
	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "custItem", 	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "I",    "Customer/Item.      ", "I(tem or C(ustomer ",
		 NE, NO,  JUSTLEFT, "CI", "", local_rec.custItem},
	{1, LIN, "custItemDesc", 4, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.custItemDesc},
	{1, LIN, "lower", 	 5, 2, CHARTYPE,
		local_rec.variableMask, "          ",
		" ", " ",        "Lower Limit.        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.lower},
	{1, LIN, "lowerDesc", 	 5, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.lowerDesc},
	{1, LIN, "upper", 	 6, 2, CHARTYPE,
		local_rec.variableMask, "          ",
		" ", " ",        "Upper Limit.        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.upper},
	{1, LIN, "upperDesc", 	 6, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.upperDesc},
	{1, LIN, "coWh", 	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Company/Warehouse   ", "C(ompany or W(arehouse ",
		YES, NO,  JUSTLEFT, "CW", "", local_rec.coWh},
	{1, LIN, "coWhDesc", 7, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.coWhDesc},
	{1, LIN, "transfers", 	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Include Transfers   ", " C(ustomer Transfers  /  S(tock Transfers  / B(oth / N(o",
		 NO, NO, JUSTRIGHT, "CSBN", "", local_rec.transfers},
	{1, LIN, "transfersDesc", 8, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.transfersDesc},
	{1, LIN, "reqd_date", 	 9, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate,   "Required Date       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.inputDate},
	{1, LIN, "printerNo", 	11, 2, INTTYPE,
		"NN", "          ",
		" ", "1",        "Printer Number      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back", 	12, 2, CHARTYPE,
		"U", "          ",
		" ", "N",      "Background          ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 12, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite", 	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N",      "Overnight           ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc", 13, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "", 	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

struct	{
	long	hhplHash;
	char	poNumber [sizeof pohr_rec.pur_ord_no];
	float	qty_rem;
	} po_store [250];

int		numberInPoTable;
extern	int	EnvScreenOK;

#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
char 	*GetPoNumber 		(long);
char 	*_SortRead 			(FILE *);
float 	OutstandingPo 		(float);
int  	FindRange 			(int, char *, char *);
int  	GetHeader 			(long);
int  	GetItemNumber 		(long);
int  	GetIthr 			(void);
int  	GetReceiptWh 		(void);
int  	ProcessTransfers 	(int, int, int);
int  	heading 			(int);
int  	spec_valid 			(int);
void	ProcessSortedItem 	(void);
void 	ChangeMask 			(int);
void 	CloseDB 			(void);
void 	GetSoln 			(int);
void 	HeadingOutput 		(void);
void 	OpenDB 				(void);
void 	PrintCoTotal 		(void);
void 	PrintCustTotal 		(void);
void 	PrintHeader 		(char *, char *, char *);
void 	PrintInex 			(void);
void 	PrintLine 			(int);
void 	PrintSave 			(int, float, long, int, long);
void 	PrintWhTotal 		(void);
void 	ProcessCumr 		(void);
void 	ProcessInmr 		(void);
void 	ProcessSohr 		(long);
void 	ProcessSortedCust 	(void);
void 	ReadCcmr 			(void);
void 	ReadMisc 			(void);
void 	RunProgram 			(char *);
void 	SetDefaults 		(void);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr = chk_env ("DB_MCURR");
	char 	strTemp [100];
	TruePosition	=	TRUE;

	if (sptr)
		envVarDbMcurr = atoi (sptr);
	else
		envVarDbMcurr = FALSE;

	EnvScreenOK	=	FALSE;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc != 2 && argc != 9)
	{
		print_at (0, 0, mlSoMess733, argv [0]);
		print_at (1, 0, mlSoMess734, argv [0]);
		print_at (2, 0, mlSoMess775);
		print_at (3, 0, mlSoMess776);
		print_at (4, 0, mlSoMess777);
		print_at (5, 0, mlSoMess778);
		print_at (6, 0, mlSoMess779);
		print_at (7, 0, mlSoMess739);
		print_at (8, 0, mlSoMess740);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	switch	(argv [1] [0])
	{
	case	'D':
	case	'd':
		strcpy (rep_type, "D");
		if (argc == 2)
		{
			FLD ("printerNo") 		= ND;
			FLD ("back") 		= ND;
			FLD ("backDesc") 	= ND;
			FLD ("onite") 		= ND;
			FLD ("oniteDesc") 	= ND;
		}
		break;

	case	'P':
	case	'p':
		strcpy (rep_type, "P");
		if (argc == 2)
		{
			FLD ("printerNo") 	  = YES;
			FLD ("back") 	  = YES;
			FLD ("backDesc")  = NA;
			FLD ("onite") 	  = YES;
			FLD ("oniteDesc") = NA;
		}
		break;

	default :

		print_at (0, 0, ML (mlSoMess741));
		return (EXIT_FAILURE);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	strcpy (systemDate, DateToString (TodaysDate ()));

	if (argc == 2 || (argc == 9 && DISPLAY_REP))
	{
		init_scr ();		/*  sets terminal from termcap	*/
		set_tty ();			/*  get into raw mode		*/
		set_masks ();		/*  setup print using masks	*/
		init_vars (1);		/*  set default values		*/
	}

	if (argc == 2 || (argc == 9 && DISPLAY_REP))
		swide ();

	envVarCrCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	if (envVarCrCo == 0)
		strcpy (branchNo, " 0");
	else
		strcpy (branchNo, comm_rec.est_no);

	if (argc == 9)
	{
		/*
		 * If child process is backgd, then set to raw mode
		 */
		if (for_chk () != 0)
			signal (SIGINT, SIG_IGN);

		sprintf (local_rec.custItem, "%-1.1s", argv [2]);
		reportType = (local_rec.custItem [0] == 'C') ? CUST : ITEM;
		switch (argv [3] [0])
		{
		case 'C':
		case 'c':
			transferType = 'C';
			break;

		case 'S':
		case 's':
			transferType = 'S';
			break;

		case 'B':
		case 'b':
			transferType = 'B';
			break;

		default:
			transferType = '0';
			break;
		}

		if (strchr (argv [4],'~'))
			strcpy (strTemp," ");
		else
			strcpy (strTemp, argv [4]);

		if (local_rec.custItem [0] == 'C')
		{
			sprintf (local_rec.startCustomer, "%-6.6s", strTemp);
			sprintf (local_rec.endCustomer, "%-6.6s", argv [5]);
			noSortFields	= 17;
		}
		else
		{
			sprintf (local_rec.startItem, "%-16.16s", strTemp);
			sprintf (local_rec.endItem, "%-16.16s", argv [5]);
			noSortFields	= 14;
		}
		sprintf (local_rec.coWh, "%-1.1s", argv [6]);

		strcpy (local_rec.dateRequired, argv [7]);
		local_rec.printerNo = atoi (argv [8]);

		if (local_rec.custItem [0] == 'C')
		{
			if (!DISPLAY_REP)
				dsp_screen ("Processing : Printing Backorders By Customer.", comm_rec.co_no, comm_rec.co_name);
		}
		else
		{
			if (!DISPLAY_REP)
				dsp_screen ("Processing : Printing Backorders By Item.", comm_rec.co_no, comm_rec.co_name);
		}

		HeadingOutput ();
		if (local_rec.custItem [0] == 'C')
		{
			if (CO_REP)
				ReadCcmr ();
			else
				ProcessCumr ();
		}
		else
		{
			if (CO_REP)
				ReadCcmr ();
			else
				ProcessInmr ();
		}

		if (DISPLAY_REP)
		{
			if (printed)
				Dsp_saverec (UNDERLINE);
			Dsp_srch ();
			Dsp_close ();
		}
		else
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		SetDefaults ();
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart		= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Entry screen 1 linear input
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefaults (void)
{
	sprintf (local_rec.startItem, 		"%16.16s", " ");
	sprintf (local_rec.endItem, 		"%16.16s", " ");
	sprintf (local_rec.startCustomer, 	"%6.6s", " ");
	sprintf (local_rec.endCustomer, 	"%6.6s", " ");
	ChangeMask (ITEM);
	sprintf (local_rec.lower, 			"%16.16s", " ");
	strcpy (local_rec.upper, 			"~~~~~~~~~~~~~~~~");
	strcpy (local_rec.custItem, 		"I");
	strcpy (local_rec.custItemDesc, 	ML ("Item"));
	strcpy (local_rec.coWh, 			"C");
	strcpy (local_rec.coWhDesc, 		ML ("Company"));
	local_rec.inputDate = StringToDate (systemDate);
	strcpy (local_rec.back, 			"N");
	strcpy (local_rec.backDesc, 		ML ("No"));
	strcpy (local_rec.onite, 			"N");
	strcpy (local_rec.oniteDesc, 		ML ("No"));
	local_rec.printerNo = 1;
}

void
RunProgram (
	char	*programName)
{
	char tempLower [17];

	if (local_rec.custItem [0] == 'C')
		sprintf (local_rec.reportDesc, "Print Backorders By Customer");
	else
		sprintf (local_rec.reportDesc, "Print Backorders By Item");

	strcpy (local_rec.cuttoffDate, DateToString (local_rec.inputDate));

	if (strlen (local_rec.lower) == 0)
		strcpy (tempLower,"~");
	else
		strcpy (tempLower, local_rec.lower);

	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		sprintf
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"",
			programName,
			rep_type,
			local_rec.custItem,
			local_rec.transfers,
			tempLower,
			local_rec.upper,
			local_rec.coWh,
			local_rec.cuttoffDate,
			local_rec.printerNo,
			local_rec.reportDesc
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			rep_type,
			local_rec.custItem,
			local_rec.transfers,
			tempLower,
			local_rec.upper,
			local_rec.coWh,
			local_rec.cuttoffDate,
			local_rec.printerNo
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
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
	int		field)
{
	if (LCHECK ("custItem"))
	{
		strcpy (local_rec.custItemDesc,
			(local_rec.custItem [0] == 'C') ? ML ("Customer") : ML ("Item"));

		DSP_FLD ("custItemDesc");

		if (local_rec.custItem [0] == 'C')
			ChangeMask (1);
		else
			ChangeMask (0);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lower"))
	{
		if (dflt_used)
		{
			if (local_rec.custItem [0] == 'C')
			{
				sprintf (local_rec.lower, "%6.6s", " ");
				strcpy (local_rec.lowerDesc, ML ("First Customer"));
			}
			else
			{
				sprintf (local_rec.lower, "%16.16s", " ");
				strcpy (local_rec.lowerDesc, ML ("First Item"));
			}
			DSP_FLD ("lowerDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindRange (field, local_rec.lower, local_rec.lowerDesc);

		return (cc);
	}

	if (LCHECK ("upper"))
	{
		if (dflt_used)
		{
			if (local_rec.custItem [0] == 'C')
			{
				strcpy (local_rec.upper, "~~~~~~");
				strcpy (local_rec.upperDesc, ML ("Last Customer"));
			}
			else
			{
				strcpy (local_rec.upper, "~~~~~~~~~~~~~~~~");
				strcpy (local_rec.upperDesc, ML ("Last Item"));
			}
			DSP_FLD ("upperDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindRange (field, local_rec.upper, local_rec.upperDesc);

		return (cc);
	}

	if (LCHECK ("coWh"))
	{
		strcpy (local_rec.coWhDesc,
			(local_rec.coWh [0] == 'C') ? ML ("Company") : ML ("Warehouse"));
		DSP_FLD ("coWhDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("transfers"))
	{
		if (local_rec.transfers [0] == 'N')
		{
			strcpy (local_rec.transfersDesc, ML ("No"));
			transferType = '0';
		}

		if (local_rec.transfers [0] == 'B')
		{
			strcpy (local_rec.transfersDesc, ML ("Both"));
			transferType = 'B';
		}

		if (local_rec.transfers [0] == 'C')
		{
			strcpy (local_rec.transfersDesc, ML ("Customer"));
			transferType = 'C';
		}

		if (local_rec.transfers [0] == 'S')
		{
			strcpy (local_rec.transfersDesc, ML ("Stock"));
			transferType = 'S';
		}

		DSP_FLD ("transfersDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (DISPLAY_REP)
			return (EXIT_SUCCESS);

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

	if (LCHECK ("back"))
	{
		if (DISPLAY_REP)
			return (EXIT_SUCCESS);

		strcpy (local_rec.backDesc,
			(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No"));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		if (DISPLAY_REP)
			return (EXIT_SUCCESS);

		strcpy (local_rec.oniteDesc,
			(local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No"));

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (restart)
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlSoMess212), 45, 0, 1);
	line_at (1, 0, 132);

	if (DISPLAY_REP)
		box (0, 3, 132, 6);
	else
	{
		box (0, 3, 132, 10);
		line_at (10, 1, 132);
	}

	line_at (20, 0, 132);

	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	abc_alias (poln2, poln);
	abc_alias (inmr2, inmr);
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS,(!envVarDbFind) ? "cumr_id_no"
														       : "cumr_id_no3");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex,  inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (poln2, poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poso,  poso_list, POSO_NO_FIELDS, "poso_id_no");
	open_rec (ithr,  ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");

	ReadMisc ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (pohr);
	abc_fclose (poso);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (pocr);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	abc_alias (ccmr2, ccmr);
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

/*
 * Start Out Put To Standard Print.
 */
void
HeadingOutput (void)
{
	char	head [42];

	if (local_rec.custItem [0] == 'C')
	{
		if (CO_REP)
			strcpy (head, ML (mlSoMess213));
		else
			strcpy (head, ML (mlSoMess214));
	}
	else
	{
		if (CO_REP)
			strcpy (head, ML (mlSoMess215));
		else
			strcpy (head, ML (mlSoMess216));
	}
	if (DISPLAY_REP)
	{
		rv_pr (head, 45, 0, 1);
		if (local_rec.custItem [0] == 'C')
		{
			print_at (1, 44, ML (mlSoMess217), local_rec.startCustomer, local_rec.endCustomer);
		}
		else
		{
			print_at (1, 36, ML (mlSoMess040), local_rec.startItem, local_rec.endItem);
		}

		Dsp_open (1, 2, PSIZE);
		if (local_rec.custItem [0] == 'C')
		{
			if (CO_REP)
				Dsp_saverec (C_HEADER);
			else
				Dsp_saverec (C_WHEADER);
			Dsp_saverec (C_HEADER1);
		}
		else
		{
			if (CO_REP)
				Dsp_saverec (I_HEADER);
			else
				Dsp_saverec (I_WHEADER);
			Dsp_saverec (I_HEADER1);
		}
		Dsp_saverec (KEY);
	}
	else
	{
		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.printerNo);

		fprintf (fout, ".16\n");
		fprintf (fout, ".PI16\n");
		fprintf (fout, ".L200\n");
		fprintf (fout, ".B1\n");
		fprintf (fout, ".E%s\n", head);
		fprintf (fout, ".B1\n");
		fprintf (fout, ".ECOMPANY %s : %s\n", comm_rec.co_no, clip (comm_rec.co_name));
		fprintf (fout, ".B1\n");
		fprintf (fout, ".EAS AT %s\n", SystemTime ());
		fprintf (fout, ".B1\n");
		if (local_rec.custItem [0] == 'C')
		{
			fprintf (fout, ".EStart Customer %s    End Customer %s\n",
							local_rec.startCustomer,
							local_rec.endCustomer);
		}
		else
		{
			fprintf (fout, ".EStart Item %s     End Item %s\n",
							local_rec.startItem,
							local_rec.endItem);
		}

		fprintf (fout, ".R====================");
		fprintf (fout, "=========================================");
		fprintf (fout, "=========");
		fprintf (fout, "===================");
		fprintf (fout, "===========");
		fprintf (fout, "=======");
		fprintf (fout, "=============");
		fprintf (fout, "=============");
		fprintf (fout, "================");
		fprintf (fout, "================");
		fprintf (fout, "===========");
		fprintf (fout, "============\n");

		fprintf (fout, "====================");
		fprintf (fout, "=========================================");
		fprintf (fout, "=========");
		fprintf (fout, "===================");
		fprintf (fout, "===========");
		fprintf (fout, "=======");
		fprintf (fout, "=============");
		fprintf (fout, "=============");
		fprintf (fout, "================");
		fprintf (fout, "================");
		fprintf (fout, "===========");
		fprintf (fout, "============\n");

		if (local_rec.custItem [0] == 'C')
		{
			fprintf (fout, "!%2.2s   CUSTOMER NO / ", (CO_REP) ? "WH" : "  ");
			fprintf (fout, "!              CUSTOMER NAME /           ");
			fprintf (fout, "! ORDER  ");
			fprintf (fout, "!     CUSTOMER     ");
			fprintf (fout, "!   ORDER  ");
			fprintf (fout, "!  UOM ");
			fprintf (fout, "!  QUANTITY  ");
			fprintf (fout, "!    UNIT    ");
			fprintf (fout, "!    EXTENDED   ");
			fprintf (fout, "!   PURCHASE    ");
			fprintf (fout, "! DUE DATE ");
			fprintf (fout, "! QUANTITY !\n");

			fprintf (fout, "!      ITEM         ");
			fprintf (fout, "!            ITEM    DESCRIPTION         ");
			fprintf (fout, "!   NO   ");
			fprintf (fout, "!   ORDER NUMBER   ");
			fprintf (fout, "!   DATE   ");
			fprintf (fout, "!      ");
			fprintf (fout, "! BACKORDER  ");
			fprintf (fout, "!    PRICE   ");
			fprintf (fout, "!     VALUE     ");
			fprintf (fout, "!    NUMBER     ");
			fprintf (fout, "!          ");
			fprintf (fout, "!          !\n");
		}
		else
		{
			fprintf (fout, "!%2.2s    ITEM  NO  /  ", (CO_REP) ? "WH" : "  ");
			fprintf (fout, "!          ITEM   DESCRIPTION  /         ");
			fprintf (fout, "! ORDER  ");
			fprintf (fout, "!     CUSTOMER     ");
			fprintf (fout, "!   ORDER  ");
			fprintf (fout, "!  UOM ");
			fprintf (fout, "!  QUANTITY  ");
			fprintf (fout, "!    VALUE   ");
			fprintf (fout, "!    EXT LCL    ");
			fprintf (fout, "!    PURCHASE   ");
			fprintf (fout, "! DUE DATE ");
			fprintf (fout, "! QUANTITY !\n");

			fprintf (fout, "!      CUSTOMER     ");
			fprintf (fout, "!              CUSTOMER NAME             ");
			fprintf (fout, "!   NO   ");
			fprintf (fout, "!   ORDER NUMBER   ");
			fprintf (fout, "!   DATE   ");
			fprintf (fout, "!      ");
			fprintf (fout, "! BACKORDER  ");
			fprintf (fout, "!    PRICE   ");
			fprintf (fout, "!     VALUE     ");
			fprintf (fout, "!     NUMBER    ");
			fprintf (fout, "!          ");
			fprintf (fout, "!          !\n");
		}

		PrintLine (LNE);
		fflush (fout);
	}
	firstFlag = TRUE;
}

void
PrintLine (
 int type)
{
	switch (type)
	{
	case	BLNK:
		fprintf (fout, "!                   ");
		fprintf (fout, "                                         ");
		fprintf (fout, "         ");
		fprintf (fout, "                   ");
		fprintf (fout, "           ");
		fprintf (fout, "       ");
		fprintf (fout, "             ");
		fprintf (fout, "             ");
		fprintf (fout, "                ");
		fprintf (fout, "                ");
		fprintf (fout, "           ");
		fprintf (fout, "           !\n");
		break;

	case	LNE:
		fprintf (fout, "!-------------------");
		fprintf (fout, "!----------------------------------------");
		fprintf (fout, "!--------");
		fprintf (fout, "!------------------");
		fprintf (fout, "!----------");
		fprintf (fout, "!------");
		fprintf (fout, "!------------");
		fprintf (fout, "!------------");
		fprintf (fout, "!---------------");
		fprintf (fout, "!---------------");
		fprintf (fout, "!----------");
		fprintf (fout, "!----------!\n");
		break;

	case	SUB_TOT:
		fprintf (fout, "!                   ");
		fprintf (fout, "!                                        ");
		fprintf (fout, "!        ");
		fprintf (fout, "!                  ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "!------------");
		fprintf (fout, "!            ");
		fprintf (fout, "!---------------");
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		fprintf (fout, "!----------!\n");
		break;

	case	BIG_TOT:
		fprintf (fout, "!                   ");
		fprintf (fout, "!                                        ");
		fprintf (fout, "!        ");
		fprintf (fout, "!                  ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "!============");
		fprintf (fout, "!            ");
		fprintf (fout, "!===============");
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		fprintf (fout, "!==========!\n");
		break;
	}

	fflush (fout);
}

void
ReadCcmr (
 void)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		if (local_rec.custItem [0] == 'C')
			ProcessCumr ();
		else
			ProcessInmr ();

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	if (printed)
		PrintCoTotal ();
}

void
ProcessInmr (void)
{
	abc_selfield (soln, "soln_hhbr_hash");
	abc_selfield (sohr, "sohr_hhso_hash");
	abc_selfield (cumr, "cumr_hhcu_hash");

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItem);

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc &&
	       !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       strcmp (inmr_rec.item_no, local_rec.startItem) >= 0 &&
	       strcmp (inmr_rec.item_no, local_rec.endItem) <= 0)
	{
		if (DESCRIPTIVE)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		numberInPoTable = 0;

		soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (soln_rec.due_date <= StringToDate (local_rec.dateRequired)
			    && SOLN_BACKORDER
			    && soln_rec.hhcc_hash == ccmr_rec.hhcc_hash)
			{
				if (!DISPLAY_REP)
					dsp_process ("Item No. :", inmr_rec.item_no);

				cc = GetHeader (soln_rec.hhso_hash);
				if (cc)
				{
					cc = find_rec (soln, &soln_rec, NEXT, "r");
					continue;
				}
				GetSoln (0);
			}
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		PrintCustTotal ();

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	if (printed)
		PrintWhTotal ();

}

int
GetHeader (
	long	hhsoHash)
{
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	return (find_rec (cumr, &cumr_rec, COMPARISON, "r"));
}

void
ProcessCumr (void)
{
	abc_selfield (inmr, "inmr_hhbr_hash");

	if (CO_REP)
		abc_selfield (sohr, "sohr_hhcu_hash");

	memset (&cumr_rec, 0, sizeof (cumr_rec));

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, "  ");
	strcpy (cumr_rec.dbt_no, local_rec.startCustomer);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (cumr_rec.dbt_no, local_rec.startCustomer) < 0 ||
	            strcmp (cumr_rec.dbt_no, local_rec.endCustomer) > 0)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		numberInPoTable = 0;

		memset (&sohr_rec, 0, sizeof (sohr_rec));

		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (sohr_rec.order_no, "        ");

		cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
		while (!cc && sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			if (!CO_REP)
			{
				if (strcmp (sohr_rec.br_no, ccmr_rec.est_no))
					break;
			}

			ProcessSohr (sohr_rec.hhso_hash);
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
		}
		if (recordFound)
			PrintCustTotal ();

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	if (printed)
		PrintWhTotal ();
}

void
PrintCoTotal (void)
{
	if (DISPLAY_REP)
	{
		Dsp_saverec (SEPARATOR);
		RESET_LN_NUM;
		sprintf (dataString, "  ** COMPANY TOTAL **                                   ^E %10.2f^E          ^E%12.2f^E               ^E          ^E%9.2f",
			tot_qty [2],
			DOLLARS (tot_ext [2]),
			tot_po [2]);
		Dsp_saverec (dataString);
		RESET_LN_NUM;
	}
	else
	{
		fprintf (fout, "!  ** COMPANY TOTAL **                                       ");
		fprintf (fout, "!        ");
		fprintf (fout, "!                  ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "! %10.2f ", tot_qty [2]);
		fprintf (fout, "!            ");
		fprintf (fout, "! %14.2f", DOLLARS (tot_ext [2]));
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		fprintf (fout, "! %9.2f!\n", tot_po [2]);
		fflush (fout);
	}
}

/*------------------------------------------
| On last record, print the total backlog.   |
--------------------------------------------*/
void
PrintWhTotal (void)
{
	if (tot_qty [1] > 0.00 || tot_po [1] > 0)
	{
		if (DISPLAY_REP)
		{
			Dsp_saverec (SEPARATOR);
			RESET_LN_NUM;
			sprintf
			(
				dataString, "  ** SALES ORDER TOTAL FOR BR/WH  : %-2.2s/%-2.2s **            ^E %10.2f^E          ^E%12.2f^E               ^E          ^E%9.2f",
				ccmr_rec.est_no,
				ccmr_rec.cc_no,
				tot_qty [1],
				DOLLARS (tot_ext [1]),
				tot_po [1]
			);
			Dsp_saverec (dataString);
			Dsp_saverec (SEPARATOR);
			RESET_LN_NUM;
		}
		else
		{
			PrintLine (LNE);
			fprintf (fout, "!  ** SALES ORDER TOTAL FOR BR/WH  : %-2.2s/%-2.2s **                ", ccmr_rec.est_no, ccmr_rec.cc_no);
			fprintf (fout, "!        ");
			fprintf (fout, "!                  ");
			fprintf (fout, "!          ");
			fprintf (fout, "!      ");
			fprintf (fout, "! %10.2f ", tot_qty [1]);
			fprintf (fout, "!            ");
			fprintf (fout, "! %14.2f", DOLLARS (tot_ext [1]));
			fprintf (fout, "!               ");
			fprintf (fout, "!          ");
			fprintf (fout, "! %9.2f!\n", tot_po [1]);
			fflush (fout);
			PrintLine (LNE);
		}
	}

	if (transferType != '0' && reportType == CUST && (ProcessTransfers (CUST, FALSE, 0)))
	{
		if (DISPLAY_REP)
		{
			Dsp_saverec (SEPARATOR);
			RESET_LN_NUM;
			sprintf (dataString, "  ** GRAND TOTAL FOR BR/WH  : %-2.2s/%-2.2s^E                ^E %10.2f^E            ^E%12.2f^E               ^E         ^E%9.2f",
				ccmr_rec.est_no,
				ccmr_rec.cc_no,
				tot_qty [1],
				DOLLARS (tot_ext [1]),
				tot_po [1]);
			Dsp_saverec (dataString);
			RESET_LN_NUM;
		}
		else
		{
			PrintLine (LNE);
			fprintf (fout, "!  ** GRAND TOTAL FOR BR/WH  : %-2.2s/%-2.2s            ", ccmr_rec.est_no, ccmr_rec.cc_no);
			fprintf (fout, "!       ");
			fprintf (fout, "!                 ");
			fprintf (fout, "!        ");
			fprintf (fout, "!        ");
			fprintf (fout, "! %10.2f", tot_qty [1]);
			fprintf (fout, "!           ");
			fprintf (fout, "! %12.2f", DOLLARS (tot_ext [1]));
			fprintf (fout, "!               ");
			fprintf (fout, "!          ");
			fprintf (fout, "!%9.2f !\n", tot_po [1]);
			fflush (fout);
		}
	}

	tot_qty [2] += tot_qty [1];
	tot_ext [2] += tot_ext [1];
	tot_po [2]  += tot_po [1];
	tot_qty [1] = 0.00;
	tot_ext [1] = 0.00;
	tot_po [1] = 0.00;
}

/*===========================
| Print customer totals.	|
===========================*/
void
PrintCustTotal (void)
{
	if (local_rec.custItem [0] == 'C')
		ProcessSortedCust ();
	else
		ProcessSortedItem ();

	recordFound = FALSE;
}

void
PrintHeader (
	char 	*item_no,
	char 	*item_desc,
	char 	*curr)
{

	if (!DISPLAY_REP)
		fprintf (fout, ".LRP3\n");

	if (!firstFlag)
	{
		if (DISPLAY_REP)
		{
			Dsp_saverec (SEPARATOR);
			RESET_LN_NUM;
		}
		else
			PrintLine (LNE);
	}
	if (DISPLAY_REP)
	{
		sprintf
		(
			dataString,
			"%2.2s %-16.16s  %-40.40s                                                                             ",
			(CO_REP) ? ccmr_rec.cc_no : " ",
			item_no,
			item_desc
		);
		Dsp_saverec (dataString);
		RESET_LN_NUM;

		if (local_rec.custItem [0] != 'C')
			PrintInex ();

		RESET_LN_NUM;
	}
	else
	{
		if (local_rec.custItem [0] == 'C')
		{
			fprintf
			(
				fout,
				"!%2.2s  %-6.6s         ! %-40.40s",
				(CO_REP) ? ccmr_rec.cc_no : " ",
				item_no,
				item_desc
			);
		}
		else
		{
			fprintf
			(
				fout,
				"!%2.2s %-16.16s! %-40.40s",
				(CO_REP) ? ccmr_rec.cc_no : " ",
				item_no,
				item_desc
			);
		}

		fprintf (fout, "                           ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "!            ");
		fprintf (fout, "!            ");
		fprintf (fout, "!               ");
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		fprintf (fout, "!          !\n");

		if (local_rec.custItem [0] != 'C')
			PrintInex ();

		fprintf (fout, "!                   ");
		fprintf (fout, "! %-12.12s%40.40s",
			(local_rec.custItem [0] == 'C')?"Currency -":" ", curr);
		fprintf (fout, "               ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "!            ");
		fprintf (fout, "!            ");
		fprintf (fout, "!               ");
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		fprintf (fout, "!          !\n");
		fflush (fout);
	}

	firstFlag = FALSE;
}

void
ProcessSohr (
	long	hhsoHash)
{
	int	firstSoln = TRUE;

	memset (&soln_rec, 0, sizeof (soln_rec));

	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");

	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.due_date <= StringToDate (local_rec.dateRequired) &&
			 SOLN_BACKORDER &&
			 soln_rec.hhcc_hash == ccmr_rec.hhcc_hash)
		{
			if (!DISPLAY_REP)
				dsp_process ("Cust No. :", cumr_rec.dbt_no);

			GetSoln (firstSoln);
			firstSoln = FALSE;
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

void
GetSoln (
	int		firstSoln)
{
	long	dueDate = -1L;
	int		i;
	int		saved 	= FALSE;
	int		firstPo = TRUE;
	int		poFound = FALSE;
	char	tmpPoNumber [sizeof pohr_rec.pur_ord_no];
	float	polnQty	= 0.00;

	if (!recordFound)
	{
		fsort = sort_open ("cust");
		recordFound = TRUE;
	}

	printed = TRUE;

	if (local_rec.custItem [0] == 'C')
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			sprintf (inmr_rec.item_no, "%16.16s", " ");
	}

	poso_rec.hhsl_hash = soln_rec.hhsl_hash;
	poso_rec.hhpl_hash = 0L;
	cc = find_rec (poso, &poso_rec, GTEQ, "r");
	while (!cc && poso_rec.hhsl_hash == soln_rec.hhsl_hash)
	{
		poln_rec.hhpl_hash	=	poso_rec.hhpl_hash;
		cc = find_rec (poln, &poln_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		sprintf (tmpPoNumber, "%-15.15s", GetPoNumber (poso_rec.hhpl_hash));

		poFound = FALSE;
		for (i = 0; i < numberInPoTable; i++)
		{
			if (po_store [i].hhplHash == poln_rec.hhpl_hash)
			{
				po_store [i].qty_rem -= poso_rec.qty;
				poFound = TRUE;
			}
		}
		if (!poFound)
		{
			po_store [numberInPoTable].hhplHash = poln_rec.hhpl_hash;
			strcpy (po_store [numberInPoTable].poNumber, tmpPoNumber);
			polnQty = poln_rec.qty_ord - poln_rec.qty_rec;
			if (polnQty < 0.00)
				polnQty = 0.00;

			po_store [numberInPoTable++].qty_rem = polnQty - poso_rec.qty;
		}
		dueDate = poln_rec.due_date;

		PrintSave
		(
			firstSoln,
			poso_rec.qty,
			dueDate,
			firstPo,
			poso_rec.hhpl_hash
		);
		saved 	= TRUE;
		firstPo = FALSE;

		cc = find_rec (poso, &poso_rec, NEXT, "r");
	}
	if (saved == FALSE)
		PrintSave (firstSoln, 0.00, -1L, firstPo, 0L);
}

void
PrintSave (
	int 	firstSoln,
	float 	poQty,
	long 	dueDate,
	int 	firstPo,
	long 	hhplHash)
{
	double	extend 	= 0.00;
	double	lcl_ex_rate;
	double	l_total		= 0.00,
			l_disc		= 0.00,
			l_tax		= 0.00,
			l_gst		= 0.00;
	char	dateRaised [11];
	char	dateRequired [11];
	float	qty 	= 0.00;

	/*--------------------
	| Get exchange rate. |
	--------------------*/
	lcl_ex_rate = (envVarDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00;

	if (soln_rec.status [0] == 'B')
		qty = soln_rec.qty_order + soln_rec.qty_bord;
	else
		qty = soln_rec.qty_bord;

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) qty;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);

		if (envVarDbNettUsed)
			extend	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			extend	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		extend	/=	lcl_ex_rate;
	}
	if (firstPo)
	{
		tot_qty [0] += qty;
		tot_qty [1] += qty;

		tot_ext [0] += extend;
		tot_ext [1] += extend;
	}

	/*---------------------------------
	| get pocr details for lcl_price  |
	| only displayed for ITEM else    |
	| curr code is displayed          |
	---------------------------------*/
	if (envVarDbMcurr)
	{
		strcpy (pocr_rec.co_no, cumr_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			strcpy (pocr_rec.description, ML ("Unknown Currency"));
	}

	if (sohr_rec.dt_raised == 0L)
		sprintf (dateRaised, "%-10.10s", " ");
	else
		strcpy (dateRaised, DateToString (sohr_rec.dt_raised));

	if (sohr_rec.dt_required == 0L)
		sprintf (dateRequired, "%10.10s", " ");
	else
		strcpy (dateRequired, DateToString (sohr_rec.dt_required));

	switch (local_rec.custItem [0])
	{
	case	'C':
		sprintf
		(
			dataString,
			"%s%c%d%c%s%c%s%c%s%c%s%c%s%c%ld%c%ld%c%f%c%f%c%f%c%f%c%d%c%s%c%s%c%s%c\n",
			sohr_rec.order_no, 							 1, /* Offset = 0 	*/
			soln_rec.line_no, 							 1, /* Offset = 1	*/
			inmr_rec.item_no, 							 1, /* Offset = 2	*/
			soln_rec.item_desc, 						 1, /* Offset = 3	*/
			sohr_rec.cus_ord_ref, 						 1, /* Offset = 4	*/
			dateRaised, 								 1, /* Offset = 5	*/
			dateRequired, 								 1, /* Offset = 6	*/
			(dueDate <= -1L) ? 0L : dueDate, 			 1, /* Offset = 7	*/
			(dueDate <= -1L) ? 0L : hhplHash, 			 1, /* Offset = 8	*/
			(dueDate <= -1L) ? 0L : poQty, 				 1, /* Offset = 9	*/
			qty, 										 1, /* Offset = 10	*/
			DOLLARS (extend / qty), 					 1, /* Offset = 11	*/
			DOLLARS (extend), 							 1, /* Offset = 12	*/
			firstPo, 									 1, /* Offset = 13	*/
			inmr_rec.inmr_class, 						 1, /* Offset = 14	*/
			(envVarDbMcurr) ? pocr_rec.description : " ",1, /* Offset = 15	*/
			inmr_rec.sale_unit,					 		 1  /* Offset = 16	*/
		);

		sort_save (fsort, dataString);

		break;

	case	'I':
		/* Store data to be sorted later  */
		sprintf
		(
			dataString,
			"%s%c%s%c%s%c%s%c%s%c%s%c%ld%c%ld%c%f%c%f%c%f%c%f%c%d%c%s%c\n",
			cumr_rec.dbt_no, 							1, /* Offset = 0	*/
			cumr_rec.dbt_name, 							1, /* Offset = 1	*/
			sohr_rec.order_no, 							1, /* Offset = 2	*/
			sohr_rec.cus_ord_ref, 						1, /* Offset = 3	*/
			dateRaised, 								1, /* Offset = 4	*/
			dateRequired, 								1, /* Offset = 5	*/
			(dueDate <= -1L) ? 0L : dueDate, 			1, /* Offset = 6	*/
			(dueDate <= -1L) ? 0L : hhplHash, 			1, /* Offset = 7	*/
			(dueDate <= -1L) ? 0L : poQty, 				1, /* Offset = 8	*/
			qty, 										1, /* Offset = 9	*/
			DOLLARS (extend / qty),						1, /* Offset = 10	*/
			DOLLARS (extend), 							1, /* Offset = 11	*/
			firstPo, 									1, /* Offset = 12	*/
			inmr_rec.sale_unit,							1  /* Offset = 13	*/
		);

		sort_save (fsort, dataString);

		break;
	}
}

int
FindRange (
 int field,
 char *fld_value,
 char *fld_desc)
{
	int	dis_fld = field + 1;

	switch (local_rec.custItem [0])
	{
	case 'C':
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (fld_value));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.lower, "%-6.6s", local_rec.lower);
		sprintf (local_rec.upper, "%-6.6s", local_rec.upper);
		if (prog_status != ENTRY &&
			(strncmp (local_rec.lower, local_rec.upper, 16) > 0))
		{

			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		 if ((field == UPPER) && strncmp (local_rec.lower, local_rec.upper, 16) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf ((field == LOWER) ? local_rec.lower : local_rec.upper, "%-6.6s", cumr_rec.dbt_no);
		sprintf (fld_desc, "%-40.40s", (!cc) ? cumr_rec.dbt_name : " ");
		break;

	case 'I':
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", fld_value);
		sprintf ((field == LOWER) ? local_rec.lower : local_rec.upper, "%-16.16s", inmr_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.lower, "%-16.16s", local_rec.lower);
		sprintf (local_rec.upper, "%-16.16s", local_rec.upper);
		 if (prog_status != ENTRY &&
			(strncmp (local_rec.lower, local_rec.upper, 16) > 0))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		 if ((field == UPPER) && strncmp (local_rec.lower, local_rec.upper, 16) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (fld_desc, "%-40.40s", (!cc) ? inmr_rec.description : " ");
		break;
	}
	display_field (field);
	display_field (dis_fld);
	return (cc);
}

void
ChangeMask (
 int file_id)
{
	char	*new_mask = (char *) 0;

	switch (file_id)
	{
	case ITEM:
		new_mask = itemMask;
		strcpy (local_rec.transfers, " ");
		transferType = '0';
		DSP_FLD ("transfers");
		FLD ("transfers") = NO;
		break;

	case CUST:
		new_mask = customerMask;
		strcpy (local_rec.transfers, "N");
		transferType = '0';
		DSP_FLD ("transfers");
		FLD ("transfers") = NA;
		break;
	}

	strcpy (local_rec.variableMask, new_mask);

	strcpy (local_rec.lower, string (16, " "));
	display_field (LOWER);
	strcpy (local_rec.lower, string (strlen (new_mask), " "));
	display_field (LOWER);

	sprintf (local_rec.lowerDesc, "%40.40s", " ");
	display_field (LOWER + 1);

	strcpy (local_rec.upper, string (16, " "));
	display_field (UPPER);
	sprintf (local_rec.upperDesc, "%40.40s", " ");
	display_field (UPPER - 1);
}

void
ProcessSortedCust (void)
{
	char	*sptr;
	int		printCustomer 	= TRUE;
	char	firstPo [2];
	char	dueDateStr [11];
	int		dueNull = FALSE;
	float	po_qty = 0.00;
	float	c_qty = 0.00;
	double	c_ext_val = 0.00;
	float	c_po_qty = 0.00;
	long	t_val;
	long	hhplHash;
	char	poNumber [sizeof pohr_rec.pur_ord_no];
	char	tmpPoQty [13];

	if (recordFound)
	{
		fsort = sort_sort (fsort, "cust");
		sptr = _SortRead (fsort);
	}
	else
		sptr = (char *) 0;

	while (sptr)
	{
		hhplHash = atol (srt_offset [8]);
		sprintf (poNumber, "%-15.15s", GetPoNumber (hhplHash));

		t_val = atol (srt_offset [8]);
		if (t_val <= 0L)
		{
			dueNull = TRUE;
			strcpy (dueDateStr, "          ");
		}
		else
			strcpy (dueDateStr, DateToString (t_val));

		sprintf (firstPo, "%-1.1s", srt_offset [13]);

		if (printCustomer == TRUE)
		{
			PrintHeader (cumr_rec.dbt_no, cumr_rec.dbt_name, srt_offset [15]);
			printCustomer = FALSE;
		}
		if (dueNull)
			sprintf (tmpPoQty, "%8.8s", " ");
		else
			sprintf (tmpPoQty, "%8.2f", po_qty);

		if (DISPLAY_REP)
		{

			sprintf
			(
				dataString,
				"%8.8s/%-13.13s^E%-16.16s^E%10.10s^E %-4.4s^E %10.2f^E%10.2f^E%12.2f^E%-15.15s^E%10.10s^E %8.8s",
				srt_offset [0],
				srt_offset [2],
				srt_offset [4],
				srt_offset [5],
				srt_offset [16],
				(float) (atof (srt_offset [10])),
				atof (srt_offset [11]),
				atof (srt_offset [12]),
				(dueNull) ? " " : poNumber,
				(dueNull) ? " " : dueDateStr,
				tmpPoQty
			);
			Dsp_saverec (dataString);
		}
		else
		{
			fprintf (fout, "!   %-16.16s", srt_offset [2]);
			fprintf (fout, "! %-38.38s ",  srt_offset [3]);
			fprintf (fout, "!%8.8s", 	   srt_offset [0]);
			fprintf (fout, "! %-16.16s ",  srt_offset [4]);
			fprintf (fout, "!%-10.10s",    srt_offset [5]);
			fprintf (fout, "! %-4.4s ",    srt_offset [16]);
			fprintf (fout, "! %10.2f ",    (float) (atof (srt_offset [10])));
			fprintf (fout, "! %10.2f ",    atof (srt_offset [11]));
			fprintf (fout, "!%15.2f", 	   atof (srt_offset [12]));
			fprintf (fout, "!%-15.15s",    (dueNull) ? " " : poNumber);
			fprintf (fout, "!%10.10s", 	   (dueNull) ? " " : dueDateStr);
			fprintf (fout, "! %8.8s !\n",  tmpPoQty);
			fflush (fout);
		}
		c_qty 		+= (float) (atof (srt_offset [10]));
		c_ext_val 	+= atof (srt_offset [12]);
		c_po_qty 	+= po_qty;
		tot_po [1] = c_po_qty;

		sptr = _SortRead (fsort);
	}

	if (DISPLAY_REP)
	{
		if (local_rec.custItem [0] != 'C')
			Dsp_saverec (SEPARATOR2);
		else
		if (c_po_qty > 0.00)
			sprintf (dataString, "  **  TOTAL  **                                         ^E %10.2f^E          ^E%12.2f^E               ^E           ^E %8.2f",
				c_qty,
				c_ext_val,
				c_po_qty);
		else
			sprintf (dataString, "  **  TOTAL  **                                         ^E %10.2f^E          ^E%12.2f^E               ^E          ^E %8.8s",
				c_qty,
				c_ext_val,
				" ");
		Dsp_saverec (dataString);
		RESET_LN_NUM;
	}
	else
	{
		PrintLine (SUB_TOT);

		fprintf (fout, ".LRP3\n");
		fprintf (fout, "!  **  TOTAL  **    ");
		fprintf (fout, "                                         ");
		fprintf (fout, "!        ");
		fprintf (fout, "!                  ");
		fprintf (fout, "!          ");
		fprintf (fout, "!      ");
		fprintf (fout, "! %10.2f ", c_qty);
		fprintf (fout, "!            ");
		fprintf (fout, "!%15.2f", c_ext_val);
		fprintf (fout, "!               ");
		fprintf (fout, "!          ");
		if (c_po_qty > 0.00)
			fprintf (fout, "! %8.2f !\n", c_po_qty);
		else
			fprintf (fout, "! %8.8s !\n", " ");
	}
	if (recordFound)
		sort_delete (fsort, "cust");
}

void
ProcessSortedItem (void)
{
	char	*sptr;

	char	dueDateStr 	[11],
			poNumber 	[sizeof pohr_rec.pur_ord_no],
			tmpPoQty 	[9];

	int		printItem 	= 0,
			dueNull 	= FALSE;

	float	c_qty 		= 0.00,
			po_qty		= 0.00,
			c_po_qty 	= 0.00;

	double	c_ext_val 	= 0.00;

	long	t_val		= 0L,
			hhplHash	= 0L;

	if (recordFound)
	{
		fsort = sort_sort (fsort, "cust");
		sptr = _SortRead (fsort);
	}
	else
		sptr = (char *) 0;

	while (sptr)
	{
		po_qty 		= (float) (atof (srt_offset [8]));
		hhplHash 	= atol (srt_offset [7]);
		strcpy (poNumber, GetPoNumber (hhplHash));

		t_val = atol (srt_offset [6]);
		if (t_val <= 0L)
		{
			strcpy (dueDateStr, "          ");
			dueNull = TRUE;
		}
		else
			strcpy (dueDateStr, DateToString (t_val));

		if (dueNull)
			sprintf (tmpPoQty, "%8.8s", " ");
		else
			sprintf (tmpPoQty, "%8.2f", po_qty);

		if (!printItem)
		{
			PrintHeader (inmr_rec.item_no, inmr_rec.description, " ");
			printItem = 1;
		}
		if (DISPLAY_REP)
		{
			sprintf
			(
				dataString,
				" %6.6s / %-12.12s^E%-16.16s^E%-10.10s^E %-4.4s^E %10.2f^E%10.2f^E%12.2f^E%-15.15s^E%-10.10s^E%8.8s",
				srt_offset [0],
				srt_offset [2],
				srt_offset [3],
				srt_offset [4],
				srt_offset [13],
				(float) (atof (srt_offset [9])),
				atof (srt_offset [10]),
				atof (srt_offset [11]),
				(dueNull) ? " " : poNumber,
				(dueNull) ? " " : dueDateStr,
				tmpPoQty
			);
			Dsp_saverec (dataString);
		}
		else
		{
			fprintf (fout, "! %18.18s", srt_offset [0]);
			fprintf (fout, "!%40.40s", srt_offset [1]);
			fprintf (fout, "!%8.8s", 	srt_offset [2]);
			fprintf (fout, "! %-16.16s ", 	 srt_offset [3]);
			fprintf (fout, "!%-10.10s", 	 srt_offset [4]);
			fprintf (fout, "!  %-4.4s", 	 srt_offset [13]);
			fprintf (fout, "!  %10.2f", 	(float) (atof (srt_offset [9])));
			fprintf (fout, "! %11.2f", 		atof (srt_offset [10]));
			fprintf (fout, "! %14.2f", 		atof (srt_offset [11]));
			fprintf (fout, "!%15.15s", (dueNull) ? " " : poNumber);
			fprintf (fout, "!%10.10s", (dueNull) ? " " : dueDateStr);
			fprintf (fout, "! %8.8s !\n", tmpPoQty);
			fflush (fout);
		}
		c_qty 		+= (float) (atof (srt_offset [9]));
		c_ext_val 	+= atof (srt_offset [11]);
		c_po_qty 	+= (float) (atof (srt_offset [8]));

		sptr = _SortRead (fsort);
	}

	if (c_qty > 0.00 || c_po_qty > 0.00)
	{
	    if (DISPLAY_REP)
	    {
			if (c_po_qty > 0.00)
			{
				sprintf
				(
					dataString,
					"  **  SALES ORDER TOTAL  **                             ^E %10.2f^E          ^E%12.2f^E               ^E          ^E %8.2f",
					c_qty,
					c_ext_val,
					c_po_qty
				);
			}
			else
			{
				sprintf
				(
					dataString,
					"  **  SALES ORDER TOTAL  **                             ^E %10.2f^E          ^E%12.2f^E               ^E          ^E %8.8s",
					c_qty,
					c_ext_val,
					" "
				);
			}
			Dsp_saverec (dataString);
			RESET_LN_NUM;
	    }
	    else
	    {
			PrintLine (SUB_TOT);

			fprintf (fout, ".LRP3\n");
			fprintf (fout, "!  **  TOTAL  **  ");
			fprintf (fout, "                                           ");
			fprintf (fout, "!        ");
			fprintf (fout, "!                  ");
			fprintf (fout, "!          ");
			fprintf (fout, "!      ");
			fprintf (fout, "!  %10.2f", c_qty);
			fprintf (fout, "!            ");
			fprintf (fout, "!   %12.2f", c_ext_val);
			fprintf (fout, "!               ");
			fprintf (fout, "!          ");
			if (c_po_qty > 0.00)
				fprintf (fout, "! %8.2f !\n", c_po_qty);
			else
				fprintf (fout, "! %8.8s !\n", " ");
			fflush (fout);
	    }
	}

	if (recordFound)
		sort_delete (fsort, "cust");

	tot_qty [3] = 0;

	if (transferType != '0')
	{
		cc = ProcessTransfers (ITEM, FALSE, printItem);
		if (cc || c_qty > 0.00)
		{
			c_po_qty = OutstandingPo (c_po_qty);
			tot_po [0] += c_po_qty;
			tot_po [1] += c_po_qty;

			if (DISPLAY_REP)
			{
				sprintf (dataString, "  **  ITEM TOTAL **                                 ^E          ^E %10.2f^E           ^E %12.2f^E               ^E         ^E%9.2f",
					tot_qty [3] + c_qty,
					c_ext_val,
					c_po_qty);

				Dsp_saverec (SEPARATOR2);
				Dsp_saverec (dataString);
			}
			else
			{
				PrintLine (BIG_TOT);
				fprintf (fout,
					"!  **  ITEM TOTAL  **                                  !       !                 !        !        |     ! %10.2f!        ! %9.2f!               !        !%9.2f !\n",
					tot_qty [3] + c_qty,
					c_ext_val,
					c_po_qty);
				fflush (fout);
			}
		}
	}
	else
		if (c_qty > 0.00)
		{
			/*---------------------------------------
			| Print all outstanding purchase orders |
			---------------------------------------*/
			c_po_qty = OutstandingPo (c_po_qty);
			tot_po [0] += c_po_qty;
			tot_po [1] += c_po_qty;
		}
}

float
OutstandingPo (
 float c_po_qty)
{
	int	firstTime = TRUE;
	int	i;
	int	PO_OK;
	char	tmpPoNumber [sizeof pohr_rec.pur_ord_no];
	char	tmpHeader [41];

	sprintf (tmpHeader, "%-36.36s", "Outstanding Purchase Orders");
	for (i = 0; i < numberInPoTable; i++)
	{
		if (twodec (po_store [i].qty_rem) > 0.00)
		{
			poln_rec.hhpl_hash	=	po_store [i].hhplHash;
			cc = find_rec (poln, &poln_rec, COMPARISON, "r");
			if (cc)
				continue;

			strcpy (tmpPoNumber, GetPoNumber (po_store [i].hhplHash));
			if (!firstTime)
				sprintf (tmpHeader, "%-30.30s", " ");

			if (DISPLAY_REP)
			{
				sprintf
				(
					dataString,
					"%-36.36s                                                        ^E%-15.15s^E%10.10s^E %8.2f",
					tmpHeader,
					tmpPoNumber,
					DateToString (poln_rec.due_date),
					po_store [i].qty_rem
				);
				Dsp_saverec (dataString);
				firstTime = FALSE;
			}
			else
			{
				if (firstTime)
					PrintLine (BLNK);
				fprintf
				(
					fout,
					"!%-30.30s       ",
					tmpHeader
				);
				fprintf (fout, "             ");
				fprintf (fout, "!       ");
				fprintf (fout, "!                 ");
				fprintf (fout, "!        ");
				fprintf (fout, "!     ");
				fprintf (fout, "!           ");
				fprintf (fout, "!             ");
				fprintf (fout, "!%-15.15s", tmpPoNumber);
				fprintf (fout, "!%-10.10s", DateToString (poln_rec.due_date));
				fprintf (fout, "! %8.2f !\n", po_store [i].qty_rem);
				firstTime = FALSE;
			}
			c_po_qty += po_store [i].qty_rem;
		}
	}

	poln2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	poln2_rec.due_date 	= 0L;
	cc = find_rec (poln2, &poln2_rec, GTEQ, "r");
	while (!cc && poln2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		PO_OK = TRUE;
		for (i = 0; i < numberInPoTable; i++)
		{
			if (po_store [i].hhplHash == poln2_rec.hhpl_hash)
			{
				PO_OK = FALSE;
				break;
			}
		}
		if (!PO_OK || (PO_OK &&
			twodec ((poln2_rec.qty_ord - poln2_rec.qty_rec)) <= 0.00)  ||
		          poln2_rec.hhcc_hash != ccmr_rec.hhcc_hash)
		{
			cc = find_rec (poln2, &poln2_rec, NEXT, "r");
			continue;
		}

		c_po_qty += poln2_rec.qty_ord - poln2_rec.qty_rec;

		sprintf (tmpPoNumber, "%-15.15s", GetPoNumber (poln2_rec.hhpl_hash));
		if (firstTime)
		{
			if (DISPLAY_REP)
				Dsp_saverec ("                                                                                                                               ");
			else
				PrintLine (BLNK);
		}
		else
			sprintf (tmpHeader, "%-36.36s", " ");

		if (DISPLAY_REP)
		{
			sprintf
			(
				dataString,
				"%-36.36s                                                        ^E%-15.15s^E%10.10s^E %8.2f",
				tmpHeader,
				tmpPoNumber,
				DateToString (poln_rec.due_date),
				(poln2_rec.qty_ord - poln2_rec.qty_rec)
			);
			Dsp_saverec (dataString);
			firstTime = FALSE;
		}
		else
		{
			fprintf (fout, "!%-30.30s                 ", tmpHeader);
			fprintf (fout, "             ");
			fprintf (fout, "!        ");
			fprintf (fout, "!                  ");
			fprintf (fout, "!          ");
			fprintf (fout, "!      ");
			fprintf (fout, "!            ");
			fprintf (fout, "!            ");
			fprintf (fout, "!               ");
			fprintf (fout, "!%-15.15s", tmpPoNumber);
			fprintf (fout, "!%-10.10s", DateToString (poln_rec.due_date));
			fprintf (fout,
				"! %8.2f !\n",
				(poln2_rec.qty_ord - poln2_rec.qty_rec));
			firstTime = FALSE;
		}

		printed = TRUE;
		cc = find_rec (poln2, &poln2_rec, NEXT, "r");
	}

	return (c_po_qty);
}

int
ProcessTransfers (
	int 	type,
	int 	rule_off,
	int 	printItem)
{
	int		firstTime = TRUE;
	int		transferPrinted = FALSE;
	char	tempIssDate [11];
	char	tempDueDate [11];
	char	head_disp [200];
	char	head_prt [200];
	char	full_supply [11];
	long 	hhbrHash;

	strcpy (head_disp, " RECEIVING WHOUSE  ^EDEL NO ^E     COMMENTS    ^E ISS DATE ^E DUE DATE ^E  QTY B/O  ^ECUS/STCK^EFULL SUPP.^E         ^E         ^E         ");
	strcpy (head_prt, "! NO / NAME OF RECEIVING WAREHOUSE              !DEL NO !    COMMENTS     !ISS DATE!DUE DATE! UOM |  QTY B/O  !CUS/STCK!FULL SUPP.!         !        !          !");

	hhbrHash = (type == ITEM) ? inmr_rec.hhbr_hash : 0L;
	itln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while ((type == ITEM && !cc &&
		    itln_rec.hhbr_hash == inmr_rec.hhbr_hash) ||
		    (type == CUST && !cc))
	{
		if ((itln_rec.stock [0] == transferType || transferType == 'B') &&
		    itln_rec.i_hhcc_hash == ccmr_rec.hhcc_hash &&
		    ITLN_BACKORDER)
		{
		    if (GetIthr ())
		    {
				cc = find_rec (itln, &itln_rec, NEXT, "r");
				continue;
		    }

		    if (itln_rec.qty_border <= 0.00)
		    {
				cc = find_rec (itln, &itln_rec, NEXT, "r");
				continue;
		    }

		    if (firstTime)
		    {
				tot_qty [3] = 0.00;

				if (rule_off)
				{
					PrintHeader (inmr_rec.item_no, inmr_rec.description, " ");
					printItem = 1;
					if (DISPLAY_REP)
						Dsp_saverec (head_disp);
					else
						fprintf (fout, "%s\n", head_prt);
				}
				else
				{
					if (!printItem)
					{
						PrintHeader (inmr_rec.item_no, inmr_rec.description, " ");
						printItem = 1;
					}
					if (DISPLAY_REP)
					{
						Dsp_saverec (SEPARATOR2);
						Dsp_saverec (head_disp);
					}
					else
					{
						PrintLine (BLNK);
						fprintf (fout, "%s\n", head_prt);
					}
				}
				firstTime = FALSE;
		    }
		    GetReceiptWh ();

			strcpy (tempIssDate, DateToString (ithr_rec.iss_date));
			strcpy (tempDueDate, DateToString (itln_rec.due_date));
		    switch (itln_rec.full_supply [0])
		    {
				case 'Y':
				strcpy (full_supply, "F/S ORDER ");
				break;

				case 'L':
				strcpy (full_supply, "F/S LINE  ");
				break;

				case 'N':
				default:
				strcpy (full_supply, "          ");
				break;
		    }
		    if (DISPLAY_REP)
		    {
				sprintf
				(
					err_str,
		    	    "%2.2s   %-9.9s     ^E%06ld ^E%-16.16s ^E %8.8s ^E %8.8s ^E%-4.4s ^E %10.2f^E%-8.8s^E%-10.10s^E%-9.9s^E%-9.9s^E%-9.9s",
			    	ccmr2_rec.cc_no,
			    	ccmr2_rec.acronym,
			    	ithr_rec.del_no,
			    	ithr_rec.tran_ref,
			    	tempIssDate,
			    	tempDueDate,
					inmr_rec.sale_unit,
			    	itln_rec.qty_border,
			    	(itln_rec.stock [0] == 'S') ? " STOCK. " : "CUSTOMER",
			    	full_supply,
			    	" ",
					" ",
					" "
				);
				Dsp_saverec (err_str);
		    }
		    else
		    {
				if (type == CUST)
				{
					GetItemNumber (itln_rec.hhbr_hash);
					fprintf (fout,
					"!%-16.16s %2.2s / ",
					inmr_rec.item_no,
					ccmr2_rec.cc_no);
					fprintf (fout, "%-25.25s", ccmr2_rec.name);
				}
				else
				{
					fprintf (fout, "! %2.2s / ", ccmr2_rec.cc_no);
					fprintf (fout, "%-40.40s ", ccmr2_rec.name);
				}

				fprintf (fout, "!%06ld ", ithr_rec.del_no);
				fprintf (fout, "!%-16.16s ", ithr_rec.tran_ref);
				fprintf (fout, "!%8.8s", tempIssDate);
				fprintf (fout, "!%8.8s", tempDueDate);
				fprintf (fout, "!%-4.4s ", inmr_rec.sale_unit);
				fprintf (fout,
					"! %10.2f!%8.8s!%-10.10s!         !        !          !\n",
					itln_rec.qty_border,
					(itln_rec.stock [0] == 'S') ? " STOCK." : "CUSTOMER",
					full_supply);
				fflush (fout);
			}
			tot_qty [3] += itln_rec.qty_border;
			tot_qty [1] += itln_rec.qty_border;
		    transferPrinted = TRUE;

		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	    if (transferPrinted)
	    {
		if (DISPLAY_REP)
		{
		    sprintf (err_str,
			"%-37.37s%-8.8s^E          ^E          ^E %10.2f^E%-8.8s^E%-10.10s^E%-9.9s^E%-9.9s^E%-9.9s",
			(reportType == CUST) ? "  **  TRANSFER TOTAL FOR WAREHOUSE : " : "  **  TRANSFER TOTAL  ** ",
			(reportType == CUST) ? ccmr_rec.cc_no : " ",
			tot_qty [3],
			" ", " ", " ", " ", " ");

			Dsp_saverec (SEPARATOR2);
			Dsp_saverec (err_str);
		}
		else
		{
		    PrintLine (SUB_TOT);
		    fprintf (fout,
			"!%-37.37s%-8.8s  !       !                 !        !        ! %10.2f!        !          !         !        !          !\n",
			(reportType == CUST) ? "  **  TRANSFER TOTAL FOR WAREHOUSE : " : "  **  TRANSFER TOTAL  **",
			(reportType == CUST) ? ccmr_rec.cc_no : " ",
			tot_qty [3]);
			fflush (fout);
		}
	}
	return (transferPrinted);
}

int
GetIthr (void)
{
	ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
	return (find_rec (ithr, &ithr_rec, COMPARISON, "r"));
}

int
GetItemNumber (
	long	hhbrHash)
{
	inmr2_rec.hhbr_hash	=	hhbrHash;
	return (find_rec (inmr2, &inmr2_rec, COMPARISON, "r"));
}

int
GetReceiptWh (void)
{
	ccmr2_rec.hhcc_hash	=	itln_rec.r_hhcc_hash;
	return (find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r"));
}

char *
GetPoNumber (
	long	hhplHash)
{
	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, COMPARISON, "r");
	if (cc)
		return ("NONE");

	pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		return ("NONE");

	return (pohr_rec.pur_ord_no);
}

void
PrintInex (void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (DISPLAY_REP)
		{
			sprintf
			(
				dataString, "%2.2s %40.40s",
				(CO_REP) ? ccmr_rec.cc_no : " ",
				inex_rec.desc
			);
			Dsp_saverec (dataString);
			RESET_LN_NUM;

		}
		else
		{
			if (local_rec.custItem [0] == 'C')
			{
				fprintf
				(
					fout,
					"!%2.2s  %-6.6s         ! %-40.40s",
					 " ",
					 " ",
					 inex_rec.desc
				 );
		 	}
			else
			{
				fprintf
				(
					fout,
					"!%2.2s %-16.16s! %-40.40s",
					 " ",
					 " ",
					 inex_rec.desc
			 	);
		 	}
			fprintf (fout, "         ");
			fprintf (fout, "         ");
			fprintf (fout, "         ");
			fprintf (fout, "           ");
			fprintf (fout, "       ");
			fprintf (fout, "             ");
			fprintf (fout, "             ");
			fprintf (fout, "                ");
			fprintf (fout, "                ");
			fprintf (fout, "         ");
			fprintf (fout, "             !\n");
		}
		fflush (fout);
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}

/*
 * Save offsets for each numerical field.
 */
char*
_SortRead (
 FILE*              srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset [0] = sptr;

	tptr = sptr;
	while (fld_no < noSortFields)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset [fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}

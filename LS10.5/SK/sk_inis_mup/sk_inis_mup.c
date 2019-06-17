/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inis_mup.c,v 5.3 2002/07/01 03:06:13 cha Exp $
|  Program Name  : (sk_inis_mup.c)
|  Program Desc  : (Stock mass inis update program)
|---------------------------------------------------------------------|
|  Date Written  : (23/08/90)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: sk_inis_mup.c,v $
| Revision 5.3  2002/07/01 03:06:13  cha
| Fixed S/C 779
|
| Revision 5.2  2002/03/01 06:57:30  scott
| Updated to add range checks
|
| Revision 5.1  2002/02/28 09:23:42  scott
| Added app.schema
| Clean code and make it look better.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inis_mup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inis_mup/sk_inis_mup.c,v 5.3 2002/07/01 03:06:13 cha Exp $";

#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<pslscr.h>
#include	<getnum.h>
#include	<hot_keys.h>
#include	<ring_menu.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include    <tabdisp.h>

/*
 * Callback Declarations 
 */
static int 	UpdateReturnFucn	(int, KEY_TAB *);
static int 	FieldReturnFunc 	(int, KEY_TAB *);
static int 	LeadTimeReturnFunc 	(int, KEY_TAB *);
static int 	WarehouseReturnFunc (int, KEY_TAB *);
int  	UpdateSelection 	(void);
int  	FieldSelection 		(void);
int  	WarehouseSelection 	(void);
int  	ProcessFile 		(void);
int  	heading 			(void);


static	KEY_TAB	ups_keys [] =
{
	{
		" [RETURN]",
		'\r',
		UpdateReturnFucn,
		" Select highlighted option ",
	},
	END_KEYS
};

static	KEY_TAB	fls_keys [] =
{
	{
		" [RETURN]",
		'\r',
		FieldReturnFunc,
		" Select highlighted option ",
	},
	END_KEYS
};

static KEY_TAB lt_keys [] =
{
	{
		" [RETURN]",
		'\r',
		LeadTimeReturnFunc,
		" Select highlighted option ",
	},
	END_KEYS
};

static	KEY_TAB	whs_keys [] =
{
	{
		" [RETURN]",
		'\r',
		WarehouseReturnFunc,
		" Select highlighted option ",
	},
	END_KEYS
};

#ifdef	GVISION
menu_type	_main_menu [] =
{
    {0, " UPDATE ORDER ","Select update order method/range", UpdateSelection, },
    {0, " FIELDS ",      "Select which fields to update",    FieldSelection, },
    {0, " WHERE ",       "Select update conditions",         WarehouseSelection, },
    {0, " EXECUTE ",     "Perform the file update",          ProcessFile, 0,    ALL },
    {0, "",								      }
};
#else
menu_type	_main_menu [] =
{
    {"<Update order>"," Select update order method/range ", UpdateSelection,    "Uu", },
    {"<Fields>",      " Select which fields to update ",    FieldSelection,    "Ff", },
    {"<Where>",       " Select update conditions ",         WarehouseSelection,    "Ww", },
    {"<eXecute>",     " Perform the file update ",          ProcessFile,  "Xx", 0,    ALL },
    {"< REDRAW >",    " Redraw Display ",                   heading,    "",   FN3, },
    {"< EDIT / END >"," Exit Program ",                     _no_option, "",   FN16, ALL, },
    {"<FN1>",         " Exit Program ",                     _no_option, "",   FN1,  HIDDEN, },
    {"",								      }
};
#endif

#define	SUPPLIER	1
#define	ITEM		2
#define	CATEGORY	3

int		updateMethod	= 0,
		printerNo 		= 1;

char	*updateTableName = "update_sel";

char	minSupplierNo 	[7],
		maxSupplierNo 	[7],
		minItemNo 		[17],
		maxItemNo 		[17],
		minCategoryNo 	[12],
		maxCategoryNo 	[12],
		fobCalc 		[21],
		moqCalc 		[21],
		noqCalc 		[21],
		itemCalc 		[21];

#define	FOB_COST	1
#define	MIN_ORD_QTY	2
#define	NRM_ORD_QTY	4
#define	LEAD_TIMES	8

char	*fieldTableName = "field_sel";

#define SEA_TIME 	1
#define AIR_TIME 	2
#define LAND_TIME 	3
#define RAIL_TIME 	4

char	leadtimeDesc [6];
char	*leadTableName = "lead_sel";

int		fieldSelection		= 0,
		leadMethod 			= 0,
		leadTimeSelected 	= FALSE;

#define	WHR_SUP		1
#define	WHR_ITM		2
#define	WHR_CAT		4
int		whouseSelection;
char	*whTableName = "where_sel";

int		envVarCrCo		=	0;
char	crBranchNumber [3];

static	int	prt_dun_1st = FALSE;
static	int	printLineNo = 0;
FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;

char 	*data = "data";

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	long	lsysdate;
	char	sysdate [11];
} local_rec;

#include <FindSumr.h>
#include <FindInmr.h>

/*
 * Function Declarations 
 */
int  	GetMaxCategory 		(char *);
int  	GetMaxItem 			(char *);
int  	GetMaxSupplier 		(char *);
int  	GetMinCategory 		(char *);
int  	GetMinItem 			(char *);
int  	GetMinSupplier 		(char *);
int  	spec_valid 			(int);
int  	ValidInis 			(void);
void 	CategoryRange 		(char *);
void 	CloseDB 			(void);
void 	GetCalculation 		(int, char *);
void 	HeadPointer 		(void);
void 	ItemRange 			(char *);
void 	LeadtimeSelect 		(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);
void 	SrchExcf 			(char *);
void 	SupplierRange 		(char *);
void 	UpdateInis 			(void);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	if (argc < 2)
	{
		print_at (0,0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv [1]);

	/*
	 * Get system date. 
	 */
	strcpy (local_rec.sysdate, DateToString (TodaysDate ()));
	local_rec.lsysdate = TodaysDate ();

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty	();
	heading ();

	OpenDB 	();

	sptr = chk_env ("CR_CO");
	envVarCrCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (crBranchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	set_help (FN6, "FN06");

	search_ok = TRUE;

	strcpy (minSupplierNo, "      ");
	strcpy (maxSupplierNo, "~~~~~~");
	strcpy (minItemNo, "                ");
	strcpy (maxItemNo, "~~~~~~~~~~~~~~~~");
	strcpy (minCategoryNo, "           ");
	strcpy (maxCategoryNo, "~~~~~~~~~~~");

#ifdef GVISION
    run_menu (NULL, _main_menu);
#else
	run_menu (_main_menu, "", 22);
#endif

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (excf);
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_dbclose (data);
}

/*
 * Dummy entry points to stop * the 'undefined external' messages while linking
 */
int
heading (void)
{
	int	curLineNo;

	clear ();

	rv_pr (ML (mlSkMess160) , 24, 0, TRUE);

	cl_box (0, 1, 80, 7);
	rv_pr (ML ("Update Statements") , 32, 1, FALSE);

	switch (updateMethod)
	{
	case	SUPPLIER:
		print_at (2,2, ML (mlSkMess161), minSupplierNo, maxSupplierNo);
		break;

	case	ITEM:
		print_at (2,2, ML (mlSkMess162), minItemNo, maxItemNo);
		break;

	case	CATEGORY:
		print_at (2,2, ML (mlSkMess163), minCategoryNo, maxCategoryNo);
		break;

	default:
		print_at (2,2,"%-77.77s", " ");
		break;
	}

	curLineNo = 3;

	if (fieldSelection & FOB_COST)
	{
		if (*fobCalc == '=')
			print_at (curLineNo++, 2, ML (mlSkMess164), &fobCalc [1]);
		else
			print_at (curLineNo++, 2, ML (mlSkMess165), fobCalc [0], &fobCalc [1]);
	}

	if (fieldSelection & MIN_ORD_QTY)
	{
		if (*moqCalc == '=')
			print_at (curLineNo++, 2, ML (mlSkMess166), &moqCalc [1]);
		else
			print_at (curLineNo++, 2, ML (mlSkMess167), moqCalc [0], &moqCalc [1]);
	}

	if (fieldSelection & NRM_ORD_QTY)
	{
		if (*noqCalc == '=')
			print_at (curLineNo++, 2, ML (mlSkMess168), &noqCalc [1]);
		else
			print_at (curLineNo++, 2, ML (mlSkMess169), noqCalc [0], &noqCalc [1]);
	}

	if (fieldSelection & LEAD_TIMES)
	{
		if (*itemCalc == '=')
			print_at (curLineNo++, 2, ML (mlSkMess170), &itemCalc [1]);
		else
			print_at (curLineNo++, 2, ML (mlSkMess171), itemCalc [0], &itemCalc [1]);

		switch (leadMethod)
		{
			case SEA_TIME:
				strcpy (leadtimeDesc, ML ("Sea "));
				break;
			case AIR_TIME:
				strcpy (leadtimeDesc, ML ("Air "));
				break;
			case LAND_TIME:
				strcpy (leadtimeDesc, ML ("Land"));
				break;
			case RAIL_TIME:
				strcpy (leadtimeDesc, ML ("Rail"));
				break;
		}

		if (leadTimeSelected)
			print_at (curLineNo ++, 2, ML (mlSkMess172), leadtimeDesc);
	}

	if (whouseSelection & WHR_SUP)
	{
		print_at (8,2, ML (mlSkMess173), minSupplierNo, maxSupplierNo);
		if (whouseSelection & WHR_ITM)
			print_at (9,2, ML (mlSkMess174), minItemNo, maxItemNo);
		else
			if (whouseSelection & WHR_CAT)
				print_at (9,2, ML (mlSkMess175), minCategoryNo, maxCategoryNo);
	}
	else
		if (whouseSelection & WHR_ITM)
		{
			print_at (8,2, ML (mlSkMess176), minItemNo, maxItemNo);
			if (whouseSelection & WHR_CAT)
				print_at (9,2, ML (mlSkMess177), minCategoryNo, maxCategoryNo);
		}
		else
			if (whouseSelection & WHR_CAT)
				print_at (8,2, ML (mlSkMess178), minCategoryNo, maxCategoryNo);
    return (EXIT_SUCCESS);
}

int
spec_valid (
	int		iUnused)
{
	return (EXIT_SUCCESS);
}

/*
 * The following routine(s) deal with entering in the update ordering method.
 */
int
UpdateSelection (void)
{

	tab_open (updateTableName, ups_keys, 10, 2, 3, TRUE);

	tab_add (updateTableName, "# INVENTORY SUPPLIER UPDATE SELECTION ");

	tab_add (updateTableName, " Update by Supplier(s).");
	tab_add (updateTableName, " Update by Item(s).");
	tab_add (updateTableName, " Update by Category(s).");

	while (tab_scan (updateTableName));

	tab_close (updateTableName, TRUE);

	heading ();
    return (EXIT_SUCCESS);
}

static	int
UpdateReturnFucn (
	int 	c, 
	KEY_TAB *psUnused)
{
	switch (tab_tline (updateTableName))
	{
	case	0:
		updateMethod = SUPPLIER;
		if (whouseSelection & WHR_SUP)
			whouseSelection ^= WHR_SUP;
		SupplierRange (updateTableName);
		break;

	case	1:
		updateMethod = ITEM;
		if (whouseSelection & WHR_ITM)
			whouseSelection ^= WHR_ITM;
		ItemRange (updateTableName);
		break;

	case	2:
		updateMethod = CATEGORY;
		if (whouseSelection & WHR_CAT)
			whouseSelection ^= WHR_CAT;
		CategoryRange (updateTableName);
		break;
	}
	return (c);
}

/*
 * The following routine(s) deal with entering 
 * in the field-calculation criterion.         
 */
int
FieldSelection (void)
{
	tab_open (fieldTableName, fls_keys, 10, 2, 4, TRUE);

	tab_add (fieldTableName, "# INVENTORY SUPPLIER FIELD SELECTION ");
	tab_add (fieldTableName, " Supplier FOB Cost.");
	tab_add (fieldTableName, " Supplier Min Order Qty.");
	tab_add (fieldTableName, " Supplier Normal Order Qty.");
	tab_add (fieldTableName, " Supplier Lead Times.");

	while (tab_scan (fieldTableName));

	tab_close (fieldTableName, TRUE);

	heading ();
    return (EXIT_SUCCESS);
}

static	int
FieldReturnFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	/*
	 * The following switch (re)sets a 'bitwise' value into fieldSelection. 
	 */
	switch (tab_tline (fieldTableName))
	{
	case	0:
		fieldSelection ^= FOB_COST;
		if (fieldSelection & FOB_COST)
			GetCalculation (FOB_COST, ML (mlSkMess179));
		heading ();
		redraw_table (fieldTableName);
		break;

	case	1:
		fieldSelection ^= MIN_ORD_QTY;
		if (fieldSelection & MIN_ORD_QTY)
			GetCalculation (MIN_ORD_QTY, ML (mlSkMess180));
		heading ();
		redraw_table (fieldTableName);
		break;

	case	2:
		fieldSelection ^= NRM_ORD_QTY;
		if (fieldSelection & NRM_ORD_QTY)
			GetCalculation (NRM_ORD_QTY, ML (mlSkMess181));
		heading ();
		redraw_table (fieldTableName);
		break;

	case	3:
		fieldSelection ^= LEAD_TIMES;
		if (fieldSelection & LEAD_TIMES)
			GetCalculation (LEAD_TIMES, ML (mlSkMess182));
		heading ();
		redraw_table (fieldTableName);
		LeadtimeSelect ();
		break;
	}
	return (c);
}

void
GetCalculation (
	int 	field, 
	char 	*prmpt)
{
	char	*validChars = ".0123456789";
	char	*sptr;
	char	*receiveField = (char *) 0;
	int		calculationOK	=	FALSE;

	print_at (19, 2, prmpt);

	switch (field)
	{
	case	FOB_COST:
		receiveField = &fobCalc [0];
		break;

	case	MIN_ORD_QTY:
		receiveField = &moqCalc [0];
		break;

	case	NRM_ORD_QTY:
		receiveField = &noqCalc [0];
		break;

	case	LEAD_TIMES:
		receiveField = &itemCalc [0];
		break;
	}

	print_mess (ML (mlSkMess183));

	do
	{
		strcpy (receiveField, "");
		calculationOK = TRUE;
		while (*receiveField != '=' && 
		       *receiveField != '+' && 
		       *receiveField != '-' && 
		       *receiveField != '*' && 
		       *receiveField != '/')
		{
			getalpha (34, 19, "ANNNNNNNNNNN", receiveField);
			if (last_char == FN1)
			{
				strcpy (receiveField, "");
				break;
			}
		}

		if (last_char == FN1)
			break;

		clip (receiveField);
		sptr = receiveField + 1;
		while (*sptr)
		{
			if (!strchr (validChars, *sptr))
			{
				calculationOK = FALSE;
				break;
			}
			sptr++;
		}

	} while (!calculationOK);
}

/*
 * The following routine deal with entering in the 'lead time-clause' criterion
 */
void
LeadtimeSelect (void)
{
	tab_open (leadTableName, lt_keys, 12, 32, 3, TRUE);

	tab_add (leadTableName, "#   INVENTORY SUPPLIER 'LEAD TIME' SELECTION   ");

	tab_add (leadTableName, " Update Sea lead time for Current Supplier(s).");
	tab_add (leadTableName, " Update Air lead time for Current Supplier(s).");
	tab_add (leadTableName, " Update Land lead time for Current Supplier(s).");

	while (tab_scan (leadTableName));

	tab_close (leadTableName, TRUE);
	redraw_table (fieldTableName);
}

static	int
LeadTimeReturnFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	/*
	 * The following switch (re)sets a 'bitwise' value into ts_seln.   
	 */
	switch (tab_tline (leadTableName))
	{
	case	0:
		leadMethod = SEA_TIME;
		break;
	case	1:
		leadMethod = AIR_TIME;
		break;
	case	2:
		leadMethod = LAND_TIME;
		break;
	}
	
	leadTimeSelected = TRUE;

	heading ();

	redraw_table (fieldTableName);
	redraw_table (leadTableName);

	return (c);
}

/*
 * The following routine(s) deal with entering in the 'where-clause' criterion
 */
int
WarehouseSelection (void)
{
	if (!updateMethod)
	{
		print_mess (ML (mlSkMess184));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	tab_open (whTableName, whs_keys, 10, 2, 3, TRUE);

	tab_add (whTableName, "#   INVENTORY SUPPLIER 'WHERE' SELECTION        ");

	switch (updateMethod)
	{
	case	SUPPLIER:
		tab_add (whTableName, " All records for Current Supplier(s).");
		tab_add (whTableName, " Selected Item(s) for Current Supplier(s).");
		tab_add (whTableName, " Selected Category(s) for Current Supplier(s).");
		break;

	case	ITEM:
		tab_add (whTableName, " All records for Current Item(s).");
		tab_add (whTableName, " Selected Supplier(s) for Current Item(s).");
		tab_add (whTableName, " Selected Category(s) for Current Item(s).");
		break;

	case	CATEGORY:
		tab_add (whTableName, " All records for Current Category(s).");
		tab_add (whTableName, " Selected Supplier(s) for Current Category(s).");
		tab_add (whTableName, " Selected Item(s) for Current Category(s).");
		break;
	}
	while (tab_scan (whTableName));

	tab_close (whTableName, TRUE);

	heading ();
    return (EXIT_SUCCESS);
}

static	int
WarehouseReturnFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	int	newValue = 0;
	/*
	 * The following switch (re)sets a 'bitwise' value into fieldSelection.
	 */
	switch (tab_tline (whTableName))
	{
	case	0:
		newValue = 0;
		whouseSelection = 0;
		break;

	case	1:
		if (updateMethod == SUPPLIER)
			newValue = WHR_ITM;
		else
			newValue = WHR_SUP;
		whouseSelection ^= newValue;
		break;

	case	2:
		if (updateMethod == CATEGORY)
			newValue = WHR_ITM;
		else
			newValue = WHR_CAT;
		whouseSelection ^= newValue;
		break;
	}

	if (whouseSelection & newValue)
	{
		switch (newValue)
		{
		case	WHR_SUP:
			SupplierRange (whTableName);
			break;

		case	WHR_ITM:
			ItemRange (whTableName);
			break;

		case	WHR_CAT:
			CategoryRange (whTableName);
			break;
		}
	}
	else
	{
		switch (newValue)
		{
		case	WHR_SUP:
			strcpy (minSupplierNo, "      ");
			strcpy (maxSupplierNo, "~~~~~~");
			break;

		case	WHR_ITM:
			strcpy (minItemNo, "                ");
			strcpy (maxItemNo, "~~~~~~~~~~~~~~~~");
			break;

		case	WHR_CAT:
			strcpy (minCategoryNo, "           ");
			strcpy (maxCategoryNo, "~~~~~~~~~~~");
			break;
		}
	}

	return (c);
}

/*
 * General lookup (search) routines 
 */
void
SupplierRange (
	char *tableName)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no,crBranchNumber);
	while (GetMinSupplier (tableName));
	while (GetMaxSupplier (tableName));
}

int
GetMinSupplier (
	char	*tableName)
{
	int srchPressed = FALSE;

	print_at (19, 1,"%-78.78s", " ");
	print_at (20, 1,"%-78.78s", " ");
	strcpy (sumr_rec.crd_no, " ");
	print_at (19, 2, ML (mlSkMess185));
	crsr_on ();
	getalpha (32, 19, "UUUUUU", (char *) sumr_rec.crd_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		srchPressed = TRUE;
		strcpy (temp_str, sumr_rec.crd_no);
		SumrSearch (comm_rec.co_no, crBranchNumber, temp_str);
		heading ();
		redraw_table (tableName);
		print_at (19, 2, ML (mlSkMess185));
		strcpy (sumr_rec.crd_no, temp_str);
		crsr_on ();
		getalpha (32, 19, "UUUUUU", (char *) sumr_rec.crd_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (minSupplierNo, "      ");
		return (EXIT_SUCCESS);
	}
	pad_num (sumr_rec.crd_no);
	sprintf (sumr_rec.crd_no, "%-6.6s", sumr_rec.crd_no);
	sprintf (minSupplierNo, "%-6.6s", sumr_rec.crd_no);
	print_at (19, 32, sumr_rec.crd_no);
	return (find_rec (sumr, &sumr_rec, EQUAL, "r"));
}

int
GetMaxSupplier (
	char	*tableName)
{
	print_at (20, 1,"%-78.78s", " ");
	print_at (20, 2, ML (mlSkMess186));
	crsr_on ();
	getalpha (32, 20, "UUUUUU", (char *) sumr_rec.crd_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		strcpy (temp_str, sumr_rec.crd_no);
		SumrSearch (comm_rec.co_no, crBranchNumber, temp_str);
		heading ();
		redraw_table (tableName);
		print_at (19, 1,"%-78.78s", " ");
		print_at (19, 2, ML (mlSkMess185));
		print_at (19, 32, minSupplierNo);
		print_at (20, 2, ML (mlSkMess186));
		strcpy (sumr_rec.crd_no, temp_str);
		crsr_on ();
		getalpha (32, 20, "UUUUUU", (char *) sumr_rec.crd_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (maxSupplierNo, "~~~~~~");
		return (EXIT_SUCCESS);
	}
	pad_num (sumr_rec.crd_no);
	sprintf (maxSupplierNo, "%-6.6s", sumr_rec.crd_no);
	sprintf (sumr_rec.crd_no, "%-6.6s", sumr_rec.crd_no);
	print_at (20, 32, sumr_rec.crd_no);

	if (strcmp (minSupplierNo, maxSupplierNo) > 0)
	{
		errmess (ML (mlStdMess006));
		sleep (sleepTime);
		return (EXIT_FAILURE); 
	}
	return (find_rec (sumr, &sumr_rec, EQUAL, "r"));
}

void
ItemRange (
	char	*tableName)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	while (GetMinItem (tableName));
	while (GetMaxItem (tableName));
}

int
GetMinItem (
	char	*tableName)
{
	print_at (19, 1,"%-78.78s", " ");
	print_at (20, 1,"%-78.78s", " ");
	print_at (19, 2, ML (mlSkMess187));
	crsr_on ();
	getalpha (28, 19, "UUUUUUUUUUUUUUUU", (char *) inmr_rec.item_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		strcpy (temp_str, inmr_rec.item_no);
		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		heading ();
		redraw_table (tableName);
		print_at (19, 2, ML (mlSkMess187));
		strcpy (inmr_rec.item_no, temp_str);
		crsr_on ();
		getalpha (28, 19, "UUUUUUUUUUUUUUUU", (char *) inmr_rec.item_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (minItemNo, "                ");
		return (EXIT_SUCCESS);
	}
	sprintf (minItemNo, "%-16.16s", inmr_rec.item_no);
	strcpy (inmr_rec.item_no, minItemNo);
	print_at (19, 28, inmr_rec.item_no);
	return (find_rec (inmr, &inmr_rec, EQUAL, "r"));
}

int
GetMaxItem (
	char	*tableName)
{
	print_at (20, 1,"%-78.78s", " ");
	print_at (20, 2, ML (mlSkMess188));
	crsr_on ();
	getalpha (28, 20, "UUUUUUUUUUUUUUUU", (char *) inmr_rec.item_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		strcpy (temp_str, inmr_rec.item_no);
		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		heading ();
		redraw_table (tableName);
		print_at (19, 1,"%-78.78s", " ");
		print_at (19, 2, ML (mlSkMess187));
		print_at (19, 28, minItemNo);
		print_at (20, 2, ML (mlSkMess188));
		strcpy (inmr_rec.item_no, temp_str);
		crsr_on ();
		getalpha (28, 20, "UUUUUUUUUUUUUUUU", (char *) inmr_rec.item_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (maxItemNo, "~~~~~~~~~~~~~~~~");
		return (EXIT_SUCCESS);
	}
	sprintf (maxItemNo, "%-16.16s", inmr_rec.item_no);
	if (strcmp (minItemNo, maxItemNo) > 0)
	{
		errmess (ML (mlStdMess006));
		sleep (sleepTime);
		return (EXIT_FAILURE); 
	}
	strcpy (inmr_rec.item_no, maxItemNo);
	print_at (20, 28, inmr_rec.item_no);
	return (find_rec (inmr, &inmr_rec, EQUAL, "r"));
}

void
CategoryRange (
	char	*tableName)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	while (GetMinCategory (tableName));
	while (GetMaxCategory (tableName));
}

int
GetMinCategory (
	char	*tableName)
{
	print_at (19, 1,"%-78.78s", " ");
	print_at (20, 1,"%-78.78s", " ");
	print_at (19, 2, ML (mlSkMess189));
	crsr_on ();
	getalpha (32, 19, "UUUUUUUUUUU", (char *) excf_rec.cat_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		strcpy (temp_str, excf_rec.cat_no);
		SrchExcf (temp_str);
		heading ();
		redraw_table (tableName);
		print_at (19, 2, ML (mlSkMess189));
		strcpy (excf_rec.cat_no, temp_str);
		crsr_on ();
		getalpha (32, 19, "UUUUUUUUUUU", (char *) excf_rec.cat_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (minCategoryNo, "           ");
		return (EXIT_SUCCESS);
	}
	sprintf (minCategoryNo, "%-11.11s", excf_rec.cat_no);
	strcpy (excf_rec.cat_no, minCategoryNo);
	print_at (19, 32, excf_rec.cat_no);
	return (find_rec (excf, &excf_rec, EQUAL, "r"));
}

int
GetMaxCategory (
	char	*tableName)
{
	print_at (20, 1,"%-78.78s", " ");
	print_at (20, 2, ML (mlSkMess190));
	crsr_on ();
	getalpha (32, 20, "UUUUUUUUUUU", (char *) excf_rec.cat_no);
	crsr_off ();
	while (SRCH_KEY)
	{
		strcpy (temp_str, excf_rec.cat_no);
		SrchExcf (temp_str);
		heading ();
		redraw_table (tableName);
		print_at (19, 1,"%-78.78s", " ");
		print_at (19, 2, ML (mlSkMess189));
		print_at (19, 32, minCategoryNo);
		print_at (20, 2, ML (mlSkMess190));
		strcpy (excf_rec.cat_no, temp_str);
		crsr_on ();
		getalpha (32, 20, "UUUUUUUUUUU", (char *) excf_rec.cat_no);
		crsr_off ();
	}
	if (dflt_used)
	{
		strcpy (maxCategoryNo, "~~~~~~~~~~~");
		return (EXIT_SUCCESS);
	}
	sprintf (maxCategoryNo, "%-11.11s", excf_rec.cat_no);
	strcpy (excf_rec.cat_no, maxCategoryNo);
	print_at (20, 32, excf_rec.cat_no);
	if (strcmp (minCategoryNo, maxCategoryNo) > 0)
	{
		errmess (ML (mlStdMess006));
		sleep (sleepTime);
		return (EXIT_FAILURE); 
	}
	return (find_rec (excf, &excf_rec, EQUAL, "r"));
}

/*
 * Here's where the donkey routine(s) live 
 * that do the physical update based on    
 * the user's supplied parameters.        
 */
int
ProcessFile (void)
{
	if (!updateMethod)
	{
		print_mess (ML (mlSkMess184));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (!fieldSelection)
	{
		print_mess (ML (mlSkMess191));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	dsp_screen ("Stock Mass Update", comm_rec.co_no, comm_rec.co_name);

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (sumr, "sumr_hhsu_hash");

	abc_selfield (inis, "inis_id_no3");

	inmr_rec.hhbr_hash = 0L;
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no, "");

	cc = find_rec (inis, &inis_rec, FIRST, "u");
	if (!cc)
	{
		sumr_rec.hhsu_hash = inis_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, GTEQ, "r"))
		{
			sprintf (sumr_rec.crd_no, "      ");
			sprintf (sumr_rec.crd_name, "%-40.40s", "Not found!");
		}
	}

	HeadPointer ();

	while (!cc)
	{
		if (ValidInis ())
			UpdateInis ();
		else
			abc_unlock (inis);
		cc = find_rec (inis, &inis_rec, NEXT, "u");
	}
	abc_unlock (inis);

	if (printLineNo)
		fprintf (fout, "==============================================================================================================================================================\n");
	fprintf (fout, ".EOF\n");
	pclose (fout);
    return (EXIT_SUCCESS);
}

/*
 * Check to see if the inis record  is within the allowable range(s)
 */
int
ValidInis (void)
{
	long	saveHhsu	=	0L;

	if (inis_rec.hhbr_hash != inmr_rec.hhbr_hash)
	{
		inmr_rec.hhbr_hash = inis_rec.hhbr_hash;
		if (find_rec (inmr, &inmr_rec, EQUAL, "r"))
		{
			inmr_rec.hhbr_hash = 0L;
			return (FALSE);
		}
	}

	saveHhsu = sumr_rec.hhsu_hash;
	if (inis_rec.hhsu_hash != sumr_rec.hhsu_hash)
	{
		sumr_rec.hhsu_hash = inis_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
		{
			sumr_rec.hhsu_hash = 0L;
			return (FALSE);
		}
	}

	dsp_process ("Item ", inmr_rec.item_no);

	if (strcmp (sumr_rec.co_no, comm_rec.co_no))
		return (FALSE);

	if (strcmp (sumr_rec.crd_no, minSupplierNo) < 0)
		return (FALSE);

	if (strcmp (sumr_rec.crd_no, maxSupplierNo) > 0)
		return (FALSE);

	if (strcmp (inmr_rec.co_no, comm_rec.co_no))
		return (FALSE);

	if (strcmp (inmr_rec.item_no, minItemNo) < 0)
		return (FALSE);

	if (strcmp (inmr_rec.item_no, maxItemNo) > 0)
		return (FALSE);

	if (strcmp (inmr_rec.category, minCategoryNo) < 0)
		return (FALSE);

	if (strcmp (inmr_rec.category, maxCategoryNo) > 0)
		return (FALSE);

	if (saveHhsu != sumr_rec.hhsu_hash)
	{
		if (prt_dun_1st)
		{
			fprintf (fout, "==============================================================================================================================================================\n");
			fprintf (fout, ".PDSupplier : %-6.6s     Supplier Name : %-40.40s\n", sumr_rec.crd_no, sumr_rec.crd_name);
			fprintf (fout, ".PA\n");
			printLineNo = 0;
		}
		else
			fprintf (fout, ".PDSupplier : %-6.6s     Supplier Name : %-40.40s\n", sumr_rec.crd_no, sumr_rec.crd_name);
	}

	return (TRUE);
}

/*
 * Do the actual update and write an audit line 
 */
void
UpdateInis (void)
{
	float	leadTime = 0;
	if (printLineNo > 48)
	{
		fprintf (fout, "==============================================================================================================================================================\n");
		fprintf (fout, ".PA");
		printLineNo = 0;
	}

	if (! (prt_dun_1st || printLineNo))
	{
		fprintf (fout, "Supplier : %-6.6s     Supplier Name : %-40.40s\n", sumr_rec.crd_no, sumr_rec.crd_name);
		prt_dun_1st = TRUE;
	}

	if (printLineNo == 0)
	{
		fprintf (fout, "==============================================================================================================================================================\n");
		fprintf (fout, "| ITEM CODE        | ITEM DESCRIPTION                        |       FOB COST        | MINIMUM ORDER QUANTITY| NORMAL ORDER QUANTITY |       LEAD TIME       |\n");
		fprintf (fout, "|                  |                                         |       OLD |       NEW |       OLD |       NEW |       OLD |       NEW |       OLD |       NEW |\n");
		fprintf (fout, "|------------------+-----------------------------------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------|\n");
		printLineNo = 4;
	}

	fprintf (fout, "| %-16.16s | %-40.40s|%10.2f |",
		inmr_rec.item_no,
		inmr_rec.description,
		inis_rec.fob_cost);
	if (fieldSelection & FOB_COST)
	{
		switch (fobCalc [0])
		{
		case	'=':
			inis_rec.fob_cost = atof (& (fobCalc [1]));
			break;

		case	'+':
			inis_rec.fob_cost += atof (& (fobCalc [1]));
			break;

		case	'-':
			inis_rec.fob_cost -= atof (& (fobCalc [1]));
			break;

		case	'*':
			inis_rec.fob_cost *= atof (& (fobCalc [1]));
			break;

		case	'/':
			inis_rec.fob_cost /= atof (& (fobCalc [1]));
			break;
		}
	}
	fprintf (fout, "%10.2f |%10.2f |", inis_rec.fob_cost, inis_rec.min_order);

	if (fieldSelection & MIN_ORD_QTY)
	{
		switch (moqCalc [0])
		{
		case	'=':
			inis_rec.min_order = atof (& (moqCalc [1]));
			break;

		case	'+':
			inis_rec.min_order += atof (& (moqCalc [1]));
			break;

		case	'-':
			inis_rec.min_order -= atof (& (moqCalc [1]));
			break;

		case	'*':
			inis_rec.min_order *= atof (& (moqCalc [1]));
			break;

		case	'/':
			inis_rec.min_order /= atof (& (moqCalc [1]));
			break;
		}
	}
	fprintf (fout, "%10.2f |%10.2f |", inis_rec.min_order, inis_rec.norm_order);

	if (fieldSelection & NRM_ORD_QTY)
	{
		switch (noqCalc [0])
		{
		case	'=':
			inis_rec.norm_order = atof (& (noqCalc [1]));
			break;

		case	'+':
			inis_rec.norm_order += atof (& (noqCalc [1]));
			break;

		case	'-':
			inis_rec.norm_order -= atof (& (noqCalc [1]));
			break;

		case	'*':
			inis_rec.norm_order *= atof (& (noqCalc [1]));
			break;

		case	'/':
			inis_rec.norm_order /= atof (& (noqCalc [1]));
			break;
		}
	}

	if (fieldSelection & LEAD_TIMES)
	{
		/*
		 * Calculation of lead times for either sea, air, or land lead times
		 */
		if (leadMethod == SEA_TIME)
			leadTime = inis_rec.sea_time;
		else
		if (leadMethod == AIR_TIME)
			leadTime = inis_rec.air_time;
		else
		if (leadMethod == LAND_TIME)
			leadTime = inis_rec.lnd_time;

		fprintf (fout, "%10.2f |%10.2f |", inis_rec.norm_order, leadTime);

			switch (itemCalc [0])
			{
			case	'=':
				leadTime = atof (& (itemCalc [1]));
				break;

			case	'+':
				leadTime += atof (& (itemCalc [1]));
				break;

			case	'-':
				leadTime -= atof (& (itemCalc [1]));
				break;

			case	'*':
				leadTime *= atof (& (itemCalc [1]));
				break;

			case	'/':
				leadTime /= atof (& (itemCalc [1]));
				break;
			}

		if (leadMethod == SEA_TIME)
		{
			inis_rec.sea_time = leadTime;
			if (inis_rec.dflt_lead [0] == 'S')
				inis_rec.lead_time = inis_rec.sea_time;
		}
		else
		if (leadMethod == AIR_TIME)
		{
			inis_rec.air_time = leadTime;
			if (inis_rec.dflt_lead [0] == 'A')
				inis_rec.lead_time = inis_rec.air_time;
		}
		else
		if (leadMethod == LAND_TIME)
		{
			inis_rec.lnd_time = leadTime;
			if (inis_rec.dflt_lead [0] == 'L')
				inis_rec.lead_time = inis_rec.lnd_time;
		}
	}
	fprintf (fout, "%10.2f |\n", leadTime);

	printLineNo++;

	inis_rec.lcost_date = local_rec.lsysdate;

	cc = abc_update (inis, &inis_rec);
	if (cc)
		file_err (cc, inis, "DBUPDATE");
}

void
HeadPointer (void)
{
	int		headLines = 7;
	char	updatePrompt [11];
	char	*minValue,
			*maxValue;

	switch (updateMethod)
	{
	case	SUPPLIER:
		strcpy (updatePrompt, "SUPPLIER  ");
		minValue = minSupplierNo;
		maxValue = maxSupplierNo;
		break;

	case	ITEM:
		strcpy (updatePrompt, "ITEM      ");
		minValue = minItemNo;
		maxValue = maxItemNo;
		break;

	default:
		strcpy (updatePrompt, "CATEGORY  ");
		minValue = minCategoryNo;
		maxValue = maxCategoryNo;
		break;
	}

	if (fieldSelection & FOB_COST)
		headLines++;
	if (fieldSelection & MIN_ORD_QTY)
		headLines++;
	if (fieldSelection & NRM_ORD_QTY)
		headLines++;
	if (fieldSelection & LEAD_TIMES)
		headLines++;

	if (whouseSelection & WHR_SUP)
		headLines++;
	if (whouseSelection & WHR_ITM)
		headLines++;
	if (whouseSelection & WHR_CAT)
		headLines++;

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNo);
	fprintf (fout, ".PL0\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".%d\n", headLines);
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EINVENTORY SUPPLIER GLOBAL UPDATE\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EUpdated By : %s AS AT : %s\n", getenv ("LOGNAME"), SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, "Update By : %-10.10s from %s to %s\n", updatePrompt, minValue, maxValue);
	if (fieldSelection & FOB_COST)
	{
		if (*fobCalc == '=')
			fprintf (fout, "Set FOB Cost = %s\n", &fobCalc [1]);
		else
			fprintf (fout, "Set FOB Cost = FOB Cost %c %s\n", fobCalc [0], &fobCalc [1]);
	}
	if (fieldSelection & MIN_ORD_QTY)
	{
		if (*moqCalc == '=')
			fprintf (fout, "Set Min. Order Qty = %s\n", &moqCalc [1]);
		else
			fprintf (fout, "Set Min. Order Qty = Min. Order Qty %c %s\n", moqCalc [0], &moqCalc [1]);
	}
	if (fieldSelection & NRM_ORD_QTY)
	{
		if (*noqCalc == '=')
			fprintf (fout, "Set Norm. Order Qty = %s\n", &noqCalc [1]);
		else
			fprintf (fout, "Set Norm. Order Qty = Norm. Order Qty %c %s\n", noqCalc [0], &noqCalc [1]);
	}
	if (fieldSelection & LEAD_TIMES)
	{
		if (*itemCalc == '=')
			fprintf (fout, "Set Lead Time = %s\n", &itemCalc [1]);
		else
			fprintf (fout, "Set Lead Time = Lead Time %c %s\n", itemCalc [0], &itemCalc [1]);
	}

	/*
	 * Print lead time type. 
	 */
	if (leadMethod == SEA_TIME)
		strcpy (leadtimeDesc, ML ("Sea "));
	else if (leadMethod == AIR_TIME)
		strcpy (leadtimeDesc, ML ("Air "));
	else if (leadMethod == LAND_TIME)
		strcpy (leadtimeDesc, ML ("Land"));
	else if (leadMethod == RAIL_TIME)
		strcpy (leadtimeDesc, ML ("Rail"));

	if (leadTimeSelected)
		fprintf (fout, "Lead Time Type : %s\n", leadtimeDesc);

	strcpy (updatePrompt, "Where ");
	if (whouseSelection & WHR_SUP)
	{
		fprintf (fout, "Where SUPPLIER from %-16.16s to %-16.16s\n", minSupplierNo, maxSupplierNo);
		strcpy (updatePrompt, "  and ");
	}
	if (whouseSelection & WHR_ITM)
	{
		fprintf (fout, "%-6.6sITEM     from %-16.16s to %-16.16s\n", updatePrompt, minItemNo, maxItemNo);
		strcpy (updatePrompt, "  and ");
	}
	if (whouseSelection & WHR_CAT)
		fprintf (fout, "%-6.6sCATEGORY from %-16.16s to %-16.16s\n", updatePrompt, minCategoryNo, maxCategoryNo);
	fprintf (fout, ".B1\n");

	printLineNo = 0;
}

/*
 * Search for Category master file. 
 */
void
SrchExcf (
	char	*keyValue)
{
	_work_open (11,0,40);
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", keyValue);
	save_rec ("#Category", "#Category Description");
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, keyValue, strlen (keyValue)) &&
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
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, excf, "DBFIND");
}



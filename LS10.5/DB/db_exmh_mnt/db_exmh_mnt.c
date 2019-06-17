/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: db_exmh_mnt.c,v 5.5 2002/07/24 08:38:48 scott Exp $
|  Program Name  : (db_exmh_mnt)
|  Program Desc  : (Market Maintenance)
|---------------------------------------------------------------------|
|  Author        : Basil Wood      | Date Written  : 04/08/94         |
|---------------------------------------------------------------------|
| $Log: db_exmh_mnt.c,v $
| Revision 5.5  2002/07/24 08:38:48  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/26 04:34:15  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/06/26 04:26:50  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_exmh_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_exmh_mnt/db_exmh_mnt.c,v 5.5 2002/07/24 08:38:48 scott Exp $";

#define MAXLINES	1000
#define TABLINES	7

#define	CCMAIN
#include <pslscr.h>
#include <ml_std_mess.h>

#define DIMOF(array) (sizeof (array) / sizeof (array[0]))

typedef int	BOOL;

#include <minimenu.h>

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

#define LCL_SCR_WIDTH 132

#include	"schema"

struct commRecord	comm_rec;
struct exmhRecord	exmh_rec;
struct exmdRecord	exmd_rec;
struct exmsRecord	exms_rec;
struct exsfRecord	exsf_rec;
struct incsRecord	incs_rec;
struct inmrRecord	inmr_rec;
struct cuitRecord	cuit_rec;
struct cumrRecord	cumr_rec;


static char	*data  = "data",
			*incs2 = "incs2",
			*inmr2 = "inmr2";

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
	{ " 3. DELETE RECORD.                     ",
	  "" },
	{ ENDMENU }
};

/*
 * Local & Screen Structures. 
 */
static	struct
{
	char	customerNo [7];
	char	salesmanNo [3];
	char	itemNo [17];
	char	subs_code [17];
	char	dummy [11];
} local_rec;

struct storeRec
{
    long    hhbrHash;
    long    inscHash;
} store [MAXLINES];

static char *scn_desc[] =
{
	"HEADER SCREEN",
	"ITEM DETAILS SCREEN"
};

static	struct	var	vars[] =
{
	{1, LIN, "customerNo",	2, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer Number    ", "Enter Customer Number, Full Search Available. ",
		NE, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{1, LIN, "cumr_dbt_name",	2, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "salesmanNo",	3, 20, CHARTYPE,
		"UU", "          ",
		" ", "0", "Salesperson Number ", "Enter Market Rep Code. [SEARCH] for valid reps. ",
		NE, NO,  JUSTLEFT, "", "", local_rec.salesmanNo},
	{1, LIN, "exsf_salesman",	3, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "date",	5, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Date ", "Enter date when information was last collected",
		NO, NO,  JUSTLEFT, "", "", (char *)&exmh_rec.date},
	{1, LIN, "remarks",	6, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Remarks ", "Enter any remarks that relate to last visit",
		NO, NO, JUSTLEFT, "", "", exmh_rec.remarks},
	{2, TAB, "itemNo",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Item Number   ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNo},
	{2, TAB, "incs_subs_code",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Substitute Code ", "Enter substitute code. [SEARCH] for valid codes. ",
		NO, NO,  JUSTLEFT, "", "", local_rec.subs_code},
	{2, TAB, "incs_subs_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Substitute Description         ", "",
		NA, NO,  JUSTLEFT, "", "", incs_rec.subs_desc},
	{2, TAB, "supp_stat",	 0, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", " Stat ", "Enter supply status code. [SEARCH] for valid codes. ",
		YES, NO,  JUSTRIGHT, "", "", exmd_rec.supp_stat},
	{2, TAB, "unit_price",	0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Price   ", "Enter Unit Price of item",
		NO, NO,  JUSTRIGHT, "", "", (char *) &exmd_rec.unit_price},
	{2, TAB, "disc",	0, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Discount %", "Enter Discount percent given on item",
		NO, NO,  JUSTRIGHT, "0.00", "100.00", (char *) &exmd_rec.disc},
	{2, TAB, "comments",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Comments       ", "Enter general comment",
		NO, NO,  JUSTLEFT, "", "", exmd_rec.comments},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy} 
};

static char 	branchNo[3];
static BOOL		newRecord = FALSE;

#include <FindCumr.h>
/*
 * Local Function Prototypes.
 */
static 	void shutdown_prog 			(void);
static 	void OpenDB 				(void);
static 	void CloseDB 				(void);
int 	spec_valid 					(int);
static 	void LoadExMarketDetails 	(int);
static 	void UpdateExMarketDetails 	(int);
static 	BOOL IsItemInTable 			(int, long);
static 	void ClearItem 				(int);
static 	void DeleteCurrentLine 		(int);
static 	void AddExMarketHeader 		(void);
static 	void UpdateExMarketHeader 	(void);
static 	void SrchExms 				(char *);
static 	void SrchExsf 				(char *);
static 	void SrchIncs 				(char *, long);
void 	tab_other 					(int);
int 	heading 					(int);
static 	void DrawScreen 			(int);
static 	void DisplayCodeDesc 		(int, char *, char *, char *);
static 	void DisplayItemNumber 		(void);
static 	void DisplayStatusCode 		(void);

/*
 * Main processing routine. 
 */
int
main (
	int		argc,
	char	*argv[])
{
	int	i;
	int envDbCo = atoi (get_env ("DB_CO"));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	for (i = 0; i < DIMOF (scn_desc); i++)
		tab_data[i]._desc = scn_desc[i];

	swide ();

	OpenDB ();
	
	strcpy (branchNo, envDbCo ? comm_rec.est_no : " 0");

	/*
	 * Beginning of input control loop 
	 */
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*
		 * Reset control flags . 
		 */
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		init_vars (2);
    	lcount [2] = 0;

		/*
		 * Enter screen 1 linear input
		 */
		heading (1);
		entry (1);

		if (!prog_exit && !restart)
		{
			/*
			 * Screen 2 tabular input
			 */
			LoadExMarketDetails (2);

			if (newRecord)
				entry (2);
			else
				edit (2);

			if (!prog_exit && !restart)
			{
				edit_all ();

				if (!prog_exit && !restart)
				{
					if (newRecord)
						AddExMarketHeader ();
					else
						UpdateExMarketHeader ();

					UpdateExMarketDetails (2);
				}
			}
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
static void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files . 
 */
static void
OpenDB (void)
{
	int envDbFind = atoi (get_env ("DB_FIND"));

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (exmh, exmh_list, EXMH_NO_FIELDS, "exmh_id_no");
	open_rec (exmd, exmd_list, EXMD_NO_FIELDS, "exmd_id_no");
	open_rec (exms, exms_list, EXMS_NO_FIELDS, "exms_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
						envDbFind ? "cumr_id_no3" : "cumr_id_no");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");

	abc_alias (incs2, incs);
	open_rec (incs,  incs_list, INCS_NO_FIELDS, "incs_id_no");
	open_rec (incs2, incs_list, INCS_NO_FIELDS, "incs_incs_hash");

	abc_alias (inmr2, inmr);
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
    open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close data base files 
 */
static void
CloseDB (void)
{
    abc_fclose (inmr2);
	abc_fclose (inmr);
	abc_fclose (incs2);
	abc_fclose (incs);
	abc_fclose (cuit);
	abc_fclose (cumr);
	abc_fclose (exsf);
	abc_fclose (exms);
	abc_fclose (exmd);
	abc_fclose (exmh);
	SearchFindClose ();

	abc_dbclose (data);
}


int
spec_valid (
	int		field)
{
	/*
	 * Validate Customer No.
	 */
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return 0;
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.customerNo));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		DSP_FLD ("cumr_dbt_name");

		if (prog_status == ENTRY)
		{
			/*
			 * Setup defaults for when F2 is pressed
			 */
			strcpy (local_rec.salesmanNo, cumr_rec.sman_code);
			exmh_rec.date = TodaysDate ();
		}

		return 0;
	}

	/*
	 * Validate Salesman No.
	 */
	if (LCHECK ("salesmanNo"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			exsf_rec.hhsf_hash = 0L;
			return 0;
		}

		if (dflt_used)
			strcpy (local_rec.salesmanNo, cumr_rec.sman_code);

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.salesmanNo);
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		DSP_FLD ("exsf_salesman");

		/*
		 * Load External Market records
		 */
		exmh_rec.hhcu_hash = cumr_rec.hhcu_hash;
		exmh_rec.hhsf_hash = exsf_rec.hhsf_hash;
	    cc = find_rec (exmh, &exmh_rec, EQUAL, "u");	  
		if (cc)
			newRecord = TRUE;
		else
		{
			newRecord = FALSE;
			entry_exit = TRUE;

			DSP_FLD ("date");
			DSP_FLD ("remarks");
		}
		 
		return 0;
	}
	
	/*
	 * Validate Header Date
	 */
	if (LCHECK ("date"))
	{
		if (dflt_used)
			exmh_rec.date = TodaysDate ();
		return 0;
	}

	/*
	 * Validate Item number
	 */
	if (LCHECK ("itemNo"))
	{
		if (last_char == DELLINE)
		{
			if (prog_status == ENTRY)
			{
				print_mess (ML (mlStdMess005));
				sleep (sleepTime);
				clear_mess ();
				return 1;
			}
			DeleteCurrentLine (vars[field].scn);
			return 0;
		}

		if (SRCH_KEY)
		{
			InmrSearch 
			 (
				comm_rec.co_no, 
				temp_str,
				cumr_rec.hhcu_hash,
				cumr_rec.item_codes
			);
			return 0;
	  	}

		cc	=	FindInmr 
				 (
					comm_rec.co_no, 
					local_rec.itemNo,
					cumr_rec.hhcu_hash,
					cumr_rec.item_codes
				);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}
		SuperSynonymError ();

		if (IsItemInTable (vars[field].scn, inmr_rec.hhbr_hash))
		{
			errmess (ML (mlStdMess204));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		if (prog_status == ENTRY ||
			store [line_cnt].hhbrHash != inmr_rec.hhbr_hash)
		{
			/*-----------------------
			| Item added or changed
			-----------------------*/
			ClearItem (line_cnt);
			strcpy (local_rec.itemNo, inmr_rec.item_no);
			store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;
			line_display ();
		}

		DisplayItemNumber ();

		return 0;
	}

	/*
	 * Validate Substitute Code
	 */
	if (LCHECK ("incs_subs_code"))
	{
		if (SRCH_KEY)
		{
			SrchIncs (temp_str, store [line_cnt].hhbrHash);
			incs_rec.incs_hash = 0L;
			return 0;
		}
	
		/*
		 * Find Substitute Code Record
		 */
		strcpy (incs_rec.co_no, comm_rec.co_no);
		incs_rec.hhbr_hash = store [line_cnt].hhbrHash;
		strcpy (incs_rec.subs_code, local_rec.subs_code);
		cc = find_rec (incs, &incs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML ("Substitute code not found."));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		store [line_cnt].inscHash = incs_rec.incs_hash;

		return 0;
	}

	/*
	 * Validate Supply Status Code
	 */
	if (LCHECK ("supp_stat"))
	{
		if (SRCH_KEY)
		{
			SrchExms (temp_str);
			return 0;
		}

		/*
		 * Find Status Code Record
		 */
		strcpy (exms_rec.co_no, comm_rec.co_no);
		strcpy (exms_rec.stat_code, exmd_rec.supp_stat);
		cc = find_rec (exms, &exms_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML ("Supply status code not found."));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}
		
		DisplayStatusCode ();

		return 0;
	}

	return 0;
}

/*
 * Load External Market Details from exmd and display.
 * If more than MAXLINES exist, they are ignored
 */
static void
LoadExMarketDetails (
	int		scn)
{
	print_mess (ML (mlStdMess035));

    /*
     * Set screen for putval
     */
    scn_set (scn);
    init_vars (scn);

	memset (store , 0, sizeof (struct storeRec));
    lcount[scn] = 0;

	if (!newRecord)
	{
		exmd_rec.exmh_hash = exmh_rec.exmh_hash;
		exmd_rec.hhbr_hash = 0L;
		cc = find_rec (exmd, &exmd_rec, GTEQ, "r");
		while (	!cc && exmd_rec.exmh_hash == exmh_rec.exmh_hash)
		{
			/*
			 * Put Value Into Tabular Screen. 
			 */
			store [lcount[scn]].hhbrHash = exmd_rec.hhbr_hash;
			if (!find_hash (inmr2, &inmr_rec, EQUAL, "r", exmd_rec.hhbr_hash))
				strcpy (local_rec.itemNo, inmr_rec.item_no);
			else
				*local_rec.itemNo = 0;

			store [lcount[scn]].inscHash = exmd_rec.insc_hash;
			if (!find_hash (incs2, &incs_rec, EQUAL, "r", exmd_rec.insc_hash))
				strcpy (local_rec.subs_code, incs_rec.subs_code);
			else
				*local_rec.subs_code = 0;

			putval (lcount[scn]++);

			if (lcount[scn] == MAXLINES)
			{
				errmess (ML (mlStdMess158));
				sleep (sleepTime);
				clear_mess ();
				break;
			}

			cc = find_rec (exmd, &exmd_rec, NEXT, "r");
		}
	}

	line_cnt = 0;
	DrawScreen (scn);

	clear_mess ();
}


/*
 * Update detail records in exmd 
 */
static void
UpdateExMarketDetails (
	int		scn)
{
	print_mess (ML (mlStdMess035));

    scn_set (scn);

	/*
	 * Remove existing records
	 */
    exmd_rec.exmh_hash = exmh_rec.exmh_hash;
    exmd_rec.hhbr_hash = 0L;
    cc = find_rec (exmd, &exmd_rec, GTEQ, "r");
    while (	!cc && exmd_rec.exmh_hash == exmh_rec.exmh_hash)
    {
		cc = abc_delete (exmd);
	  	if (cc)
			file_err (cc, "exmd", "DBDELETE");

		cc = find_rec (exmd, &exmd_rec, GTEQ, "r");
	}

	/*
	 * Write new records 
	 */
   	for (line_cnt = 0; line_cnt < lcount[scn]; line_cnt++) 
   	{
    	getval (line_cnt);

    	exmd_rec.exmh_hash = exmh_rec.exmh_hash;
    	exmd_rec.hhbr_hash = store [line_cnt].hhbrHash;
    	exmd_rec.insc_hash = store [line_cnt].inscHash;

		cc = abc_add (exmd, &exmd_rec);
		if (cc)
			file_err (cc, "exmd", "DBADD");
	}

	clear_mess ();
}


/*
 * Return TRUE if item has been entered in table
 */
static BOOL
IsItemInTable (
	int		scn,
	long	hhbrHash)
{
	int i, last;

	last = (prog_status == ENTRY ? line_cnt : lcount[scn]);

	for (i = 0; i < last; i++)
	{
		if (hhbrHash == store [i].hhbrHash && i != line_cnt)
			return TRUE;
	}
	return FALSE;
}

/*
 * Clear tabular line fields 
 */
static void
ClearItem (
	int		lineNo)
{
	getval (lineNo);
	memset (&store [lineNo], 0, sizeof (store [0]));
	*local_rec.itemNo = 0;
	*local_rec.subs_code = 0;
	memset (&exmd_rec, 0, sizeof (exmd_rec));
	memset (&incs_rec, 0, sizeof (incs_rec));
	memset (&exms_rec, 0, sizeof (exms_rec));
	putval (lineNo);
}

/*
 * Delete line
 */
static void
DeleteCurrentLine (
	int		scr)
{
	int	pageNo = line_cnt / TABLINES;
	int lineNo = line_cnt;

	/*
	 * Delete item off display
	 */
	for (line_cnt = lineNo; line_cnt < lcount[scr] - 1; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		memcpy (&store [line_cnt], &store [line_cnt + 1], sizeof (store [0]));

		if (line_cnt / TABLINES == pageNo)
			line_display ();
	}

	/*
	 * Blank the last line 
	 */
	ClearItem (line_cnt);
	if (line_cnt / TABLINES == pageNo)
		blank_display ();

	line_cnt = lineNo;
	getval (line_cnt);
	lcount[scr]--;
}

/*
 * Add header record to exmh 
 */
static void
AddExMarketHeader (void)
{
	exmh_rec.hhcu_hash = cumr_rec.hhcu_hash;
	exmh_rec.hhsf_hash = exsf_rec.hhsf_hash;
	cc = abc_add (exmh, &exmh_rec);
	if (cc)
	{
		file_err (cc, "exmh", "DBADD");
	}
	abc_unlock (exmh); 

	/*
	 * Reread exmh_rec because abc_add () does not update exmh_hash field
	 */
	exmh_rec.hhcu_hash = cumr_rec.hhcu_hash;
	exmh_rec.hhsf_hash = exsf_rec.hhsf_hash;
	cc = find_rec (exmh, &exmh_rec, EQUAL, "r");	  
	if (cc)
		file_err (cc, "exmh", "DBFIND");
}

/*
 * Update header record in exmh 
 */
static void
UpdateExMarketHeader (void)
{
	cc = abc_update (exmh, &exmh_rec);
	if (cc)
	{
		file_err (cc, "exmh", "DBUPDATE");
	}
	abc_unlock (exmh); 
}

/*
 * Search Market Supply Status Code File
 */
static void
SrchExms (
	char	*key_val)
{
	struct exmsRecord exms_bak;

	memcpy (&exms_bak, &exms_rec, sizeof exms_bak);

	_work_open (2,0,40);
	save_rec ("#No","#Code Description");

	strcpy (exms_rec.co_no, comm_rec.co_no);
	strcpy (exms_rec.stat_code, key_val);

	cc = find_rec (exms, &exms_rec, GTEQ, "r");

	while (!cc && !strcmp (exms_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exms_rec.stat_code, exms_rec.desc);
		if (cc)
			break;

		cc = find_rec (exms, &exms_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*
		 * Read selected recor
		 */
		strcpy (exms_rec.co_no, comm_rec.co_no);
		strcpy (exms_rec.stat_code, temp_str);
		cc = find_rec (exms, &exms_rec, COMPARISON, "r");
		if (cc)
		{
			file_err (cc, "exms", "DBFIND");
		}
	}
	if (cc)
		memcpy (&exms_rec, &exms_bak, sizeof exms_rec);
}

/*
 * Search External Salesman Fil
 */
static void
SrchExsf (
	char	*key_val)
{
	struct exsfRecord exsf_bak;

	memcpy (&exsf_bak, &exsf_rec, sizeof exsf_bak);

	_work_open (2,0,40);
	save_rec ("#No", "#Salespersons Code Description.");

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, key_val);

	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");

	while (!cc && 
			!strcmp (exsf_rec.co_no, comm_rec.co_no) &&
			!strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*
		 * Read selected record
		 */
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, temp_str);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, exsf, "DBFIND");
	}

	if (cc)
		memcpy (&exsf_rec, &exsf_bak, sizeof exsf_rec);
}

/*
 * Search Inventory Competitors Substitute file. 
 */
static void
SrchIncs (
	char	*key_val,
	long     hhbrHash)
{
	struct incsRecord incs_bak;

	memcpy (&incs_bak, &incs_rec, sizeof incs_bak);

	_work_open (16,0,40);
	save_rec ("#Code","#Substitute Code Description");

	strcpy (incs_rec.co_no, comm_rec.co_no);
	incs_rec.hhbr_hash = hhbrHash;
	strcpy (incs_rec.subs_code, key_val);

	cc = find_rec (incs, &incs_rec, GTEQ, "r");

	while (	!cc
			&& incs_rec.hhbr_hash == hhbrHash &&
			!strcmp (incs_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (incs_rec.subs_code, incs_rec.subs_desc);
		if (cc)
			break;

		cc = find_rec (incs, &incs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*
		 * Read selected record
		 */
		strcpy (incs_rec.co_no, comm_rec.co_no);
		incs_rec.hhbr_hash = hhbrHash;
		strcpy (incs_rec.subs_code, temp_str);
		cc = find_rec (incs, &incs_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incs, "DBFIND");
	}

	if (cc)
		memcpy (&incs_rec, &incs_bak, sizeof incs_rec);
}

/*
 * edit () callback function - tabular screens only
 */
void
tab_other (
	int		lineNo)
{
	if (strcmp (exms_rec.stat_code, exmd_rec.supp_stat))
	{
		strcpy (exms_rec.co_no, comm_rec.co_no);
		strcpy (exms_rec.stat_code, exmd_rec.supp_stat);
		if (find_rec (exms, &exms_rec, EQUAL, "r"))
			memset (&exms_rec, 0, sizeof exms_rec);
		DisplayStatusCode ();
	}

	if (strcmp (inmr_rec.item_no, local_rec.itemNo))
	{
		cc	=	FindInmr 
				 (
					comm_rec.co_no, 
					local_rec.itemNo,
					cumr_rec.hhcu_hash,
					cumr_rec.item_codes
				);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
			memset (&inmr_rec, 0, sizeof inmr_rec);

		DisplayItemNumber ();
	}
}

/*
 * edit () callback function 
 */
int
heading (
	int		scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	switch (scn)
	{
	case 1:
		clear ();
	
		rv_pr (ML (" Market Maintenance "), LCL_SCR_WIDTH/2 - 10, 0, 1);

		box (0, 1, LCL_SCR_WIDTH, 5);
		line_at (4,1, LCL_SCR_WIDTH - 1);
		line_at (20,0, LCL_SCR_WIDTH);
		line_at (22,0, LCL_SCR_WIDTH);

		print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		break;

	case 2:
		/*
		 * Other screens must be redrawn if this screen is redrawn
		 */
		DrawScreen (1);

		tab_row = 11;
		tab_col = 0;

		getval (line_cnt);
		DisplayStatusCode ();
		DisplayItemNumber ();

		break;
	}

	scn_write (scn);	
    return (EXIT_SUCCESS);
}

static void
DrawScreen (
 int                scn)
{
	heading (scn);
	scn_display (scn);
}

static void
DisplayCodeDesc (
 int                row,
 char*              prompt,
 char*              code,
 char*              desc)
{
	char buf [133];

	sprintf (buf, "  %-24s:  %s - %s", prompt, code, desc);
	
	while (strlen (buf) < sizeof (buf) - 1)
		strcat (buf, " ");
	rv_pr (buf, 0, row, 1);
}

static void
DisplayItemNumber (void)
{
	DisplayCodeDesc (tab_row - 2, ML ("Item Number"),
							inmr_rec.item_no, inmr_rec.description);
}

static void
DisplayStatusCode (void)
{
	DisplayCodeDesc (tab_row - 3, ML ("Supply Status Code"),
							exms_rec.stat_code, exms_rec.desc);
}

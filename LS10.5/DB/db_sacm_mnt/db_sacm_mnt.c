/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_sacm_mnt.c,v 5.5 2002/07/24 08:38:49 scott Exp $
|  Program Name  : (db_sacm_mnt)
|  Program Desc  : (Salesperson Call Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: db_sacm_mnt.c,v $
| Revision 5.5  2002/07/24 08:38:49  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:24:15  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/26 04:34:18  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/06/26 04:26:53  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.1  2001/12/07 04:00:58  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_sacm_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_sacm_mnt/db_sacm_mnt.c,v 5.5 2002/07/24 08:38:49 scott Exp $";

#define MAXLINES	500
#define TABLINES	9

#include <pslscr.h>
#include <ml_std_mess.h>

#define DIMOF(array) (sizeof (array) / sizeof (array [0]))

typedef int	BOOL;

#include <minimenu.h>

#define LCL_SCR_WIDTH	132
#define BOX_WIDTH	132
#define BOX_HEIGHT	3
#define BOX_ROW		2
#define BOX_COL		 ( (LCL_SCR_WIDTH - BOX_WIDTH) / 2)

#include	"schema"

struct commRecord	comm_rec;
struct sacmRecord	sacm_rec;
struct sacdRecord	sacd_rec;
struct sacaRecord	saca_rec;
struct exsfRecord	exsf_rec;
struct cumrRecord	cumr_rec;

static char	*data  = "data",
			*cumr2 = "cumr2";

extern	int	stopDateSearch;

/*============================
| Local & Screen Structures. |
============================*/
static	struct
{
	char	salesmanNo [sizeof exsf_rec.salesman_no];
	char	customerNo [sizeof cumr_rec.dbt_no];
	char	dummy [11];
} local_rec;

struct storeRec
{
	long	hhcuHash;
	char	customerNo [sizeof cumr_rec.dbt_no];
} store [MAXLINES];

static char *scn_desc [] =
{
	"HEADER SCREEN",
	"CALL DETAILS SCREEN"
};

static	struct	var	vars [] =
{
	{1, LIN, "salesmanNo",	BOX_ROW + 1, BOX_COL + 28, CHARTYPE,
		"UU", "          ",
		" ", "0", "Salesperson No. ", "Enter Salesperson No. [SEARCH] for valid codes. ",
		NE, NO,  JUSTRIGHT, "", "", local_rec.salesmanNo},
	{1, LIN, "exsf_salesman",	BOX_ROW + 1, BOX_COL + 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "date",	BOX_ROW + 2, BOX_COL + 28, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Call Date ", "Enter Sales Call Date. [SEARCH] for existing records. ",
		NE, NO,  JUSTLEFT, "", "", (char *)&sacm_rec.date},
	{1, LIN, "st_km",	BOX_ROW + 3, BOX_COL + 28, FLOATTYPE,
		"NNNNNN.N", "          ",
		" ", "0", "Start Speedometer Reading ", "Enter Start Speedometer Reading",
		YES, NO,  JUSTLEFT, "", "", (char *) &sacm_rec.st_km},
	{1, LIN, "ed_km",	BOX_ROW + 3, BOX_COL + 80, FLOATTYPE,
		"NNNNNN.N", "          ",
		" ", "0", "End Speedometer Reading ", "Enter End Speedometer Reading",
		YES, NO,  JUSTLEFT, "", "", (char *) &sacm_rec.ed_km},

	{2, TAB, "customerNo",    MAXLINES, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer", "Enter Customer Number, Full Search Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{2, TAB, "cumr_dbt_name",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Customer Name            ", " ",
		NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{2, TAB, "call_in",  0, 2, TIMETYPE,
		"HH:MM", "          ",
		" ", "00:00", " Time In ", "Enter time call started at client\'s site.",
		YES, NO,  JUSTLEFT, "", "", (char *)&sacd_rec.call_in},
	{2, TAB, "call_out",  0, 2, TIMETYPE,
		"HH:MM", "          ",
		" ", "00:00", " Time Out ", "Enter time call ended at client\'s site.",
		YES, NO,  JUSTLEFT, "", "", (char *)&sacd_rec.call_out},
	{2, TAB, "act_code",  0, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", " Code ", "Enter Sales Call Activity Code. [SEARCH] for valid codes.",
		YES, NO,  JUSTLEFT, "", "", sacd_rec.act_code},
	{2, TAB, "remarks",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Call Remarks           ", "Enter remarks related to call.",
		NO, NO,  JUSTLEFT, "", "", sacd_rec.remarks},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};

static char     branchNo [3];
static BOOL		newRecord = FALSE;

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static void 	shutdown_prog 			 (void);
static void 	OpenDB 					 (void);
static void 	CloseDB 				 (void);
static void 	LoadSalesCallDetails 	 (int);
static void 	UpdateSalesCallDetails 	 (int);
static void 	ClearLine 				 (int);
static void 	DeleteCurrentLine 		 (int);
static void 	AddSalesCallHeader 		 (void);
static void 	UpdateSalesCallHeader 	 (void);
static void 	SrchExsf 				 (char *);
static void 	SrchSacm 				 (long);
static void 	SrchSaca 				 (char *);
static void 	DrawScreen 				 (int);
static void 	DisplayCodeDesc 		 (int, char *, char *, char *);
static void 	DisplayActivityCode 	 (void);
void 			tab_other 				 (int);
int 			spec_valid 				 (int);
int 			heading 				 (int);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	int envDbCo = atoi (get_env ("DB_CO"));
	int	i;

	stopDateSearch	=	TRUE;
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

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	strcpy (branchNo, envDbCo ? comm_rec.est_no : " 0");


	for (i = 0; i < DIMOF (scn_desc); i++)
		tab_data [i]._desc = scn_desc [i];

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		lcount [2]	=	0;

		/*-------------------------------
		| Enter screen 1 linear input
		-------------------------------*/
		init_vars (1);
		init_vars (2);
		heading (1);
		entry (1);

		if (!prog_exit && !restart)
		{
			/*------------------------
			| Screen 2 tabular input
			------------------------*/
			LoadSalesCallDetails (2);
			DisplayActivityCode ();

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
						AddSalesCallHeader ();
					else
						UpdateSalesCallHeader ();

					UpdateSalesCallDetails (2);
				}
			}
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
static void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static void
OpenDB (void)
{
	int envDbFind = atoi (get_env ("DB_FIND"));

	abc_dbopen (data);

	open_rec (sacm, sacm_list, SACM_NO_FIELDS, "sacm_id_no");
	open_rec (sacd, sacd_list, SACD_NO_FIELDS, "sacd_id_no");
	open_rec (saca, saca_list, SACA_NO_FIELDS, "saca_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");

	abc_alias (cumr2, cumr);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
					envDbFind ? "cumr_id_no3" : "cumr_id_no");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
    abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (exsf);
	abc_fclose (saca);
	abc_fclose (sacd);
	abc_fclose (sacm);
	abc_dbclose (data);
}


int
spec_valid (
	int		field)
{
	/*-------------------------
	| Validate Salesman No.
	-------------------------*/
	if (LCHECK ("salesmanNo"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			exsf_rec.hhsf_hash = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.salesmanNo);
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("exsf_salesman");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Call Date
	-------------------------*/
	if (LCHECK ("date"))
	{
		if (SRCH_KEY)
		{
			SrchSacm (exsf_rec.hhsf_hash);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			sacm_rec.date = TodaysDate ();

		/*------------------------------
		| Load Salesperson Call record
		------------------------------*/
		strcpy (sacm_rec.co_no, comm_rec.co_no);
		sacm_rec.hhsf_hash = exsf_rec.hhsf_hash;
	    cc = find_rec (sacm, &sacm_rec, EQUAL, "u");	  
		if (cc)
			newRecord = TRUE;
		else
		{
			newRecord = FALSE;
			entry_exit = TRUE;

			DSP_FLD ("st_km");
			DSP_FLD ("ed_km");
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Item number
	-------------------------*/
	if (LCHECK ("customerNo"))
	{
		if (last_char == DELLINE)
		{
			if (prog_status != ENTRY)
				DeleteCurrentLine (vars [field].scn);
			else
			{
				print_mess (ML (mlStdMess005));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
        {
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
            cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
        }

		strcpy (cumr_rec.co_no, comm_rec.co_no);
        strcpy (cumr_rec.est_no, branchNo);
        strcpy (cumr_rec.dbt_no, pad_num (local_rec.customerNo));
        cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
        if (cc)
        {
            print_mess (ML (mlStdMess021));
            sleep (sleepTime);
			return (EXIT_FAILURE);
        }

		store [line_cnt].hhcuHash = cumr_rec.hhcu_hash;
		strcpy (store [line_cnt].customerNo, cumr_rec.dbt_no);
		strcpy (local_rec.customerNo, cumr_rec.dbt_no);

		DSP_FLD ("customerNo");
		DSP_FLD ("cumr_dbt_name");

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Activity Code
	-------------------------*/
	if (LCHECK ("act_code"))
	{
		if (SRCH_KEY)
		{
			SrchSaca (temp_str);
			return (EXIT_SUCCESS);
	  	}

		strcpy (saca_rec.co_no, comm_rec.co_no);
		strcpy (saca_rec.code, sacd_rec.act_code);
		cc = find_rec (saca, &saca_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML ("Activity code not on file"));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		DisplayActivityCode ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Load Sales Call details from sacd
| and display.
| If more than MAXLINES exist, they are ignored
===============================================*/
static void
LoadSalesCallDetails (
 int                scn)
{
    /*-----------------------
    | Set screen for putval
    -----------------------*/
    scn_set (scn);
    init_vars (scn);

	memset (store, 0, sizeof store);
    lcount [scn] = 0;

	if (!newRecord)
	{
		sacd_rec.sacm_hash = sacm_rec.sacm_hash;
		sacd_rec.seq_no = 0;
		cc = find_rec (sacd, &sacd_rec, GTEQ, "r");
		while (	!cc && sacd_rec.sacm_hash == sacm_rec.sacm_hash)
		{
			/*--------------------------------
			| Put Value Into Tabular Screen. |
			--------------------------------*/
			if (!find_hash (cumr2, &cumr_rec, EQUAL, "r", sacd_rec.hhcu_hash))
				strcpy (local_rec.customerNo, cumr_rec.dbt_no);
			else
				*local_rec.customerNo = 0;

			store [lcount [scn]].hhcuHash = cumr_rec.hhcu_hash;
			strcpy (store [lcount [scn]].customerNo, local_rec.customerNo);

			putval (lcount [scn]++);

			if (lcount [scn] == MAXLINES)
			{
				sprintf (err_str,
						"MAXLINES (%d) Exceeded - Remaining Detail Lines Ignored",
						MAXLINES);
				errmess (err_str);
				sleep (sleepTime);
				break;
			}

			cc = find_rec (sacd, &sacd_rec, NEXT, "r");
		}
	}

	line_cnt = 0;
	DrawScreen (scn);

	clear_mess ();
}


/*==============================
| Update detail records in sacd 
==============================*/
static void
UpdateSalesCallDetails (
 int                scn)
{
    scn_set (scn);

	/*-----------------------
	| Remove existing records
	-----------------------*/
    sacd_rec.sacm_hash = sacm_rec.sacm_hash;
    sacd_rec.seq_no = 0;
    cc = find_rec (sacd, &sacd_rec, GTEQ, "r");
    while (	!cc && sacd_rec.sacm_hash == sacm_rec.sacm_hash)
    {
		cc = abc_delete (sacd);
	  	if (cc)
			file_err (cc, sacd, "DBDELETE");

		cc = find_rec (sacd, &sacd_rec, GTEQ, "r");
	}

	/*-----------------------
	| Write new records 
	-----------------------*/
   	for (line_cnt = 0; line_cnt < lcount [scn]; line_cnt++) 
   	{
    	getval (line_cnt);

    	sacd_rec.sacm_hash = sacm_rec.sacm_hash;
    	sacd_rec.seq_no = line_cnt + 1;
    	sacd_rec.hhcu_hash = store [line_cnt].hhcuHash;

		cc = abc_add (sacd, &sacd_rec);
		if (cc)
			file_err (cc, sacd, "DBADD");
	}

	clear_mess ();
}


/*===========================
| Clear tabular line fields 
===========================*/
static void
ClearLine (
 int                lineNo)
{
	getval (lineNo);
	memset (&store [lineNo], 0, sizeof (store [0]));
	memset (&sacd_rec, 0, sizeof sacd_rec);
	putval (lineNo);
}

/*==============
| Delete line
==============*/
static void
DeleteCurrentLine (
 int                scr)
{
	int	pageNo = line_cnt / TABLINES;
	int lineNo = line_cnt;

	/*---------------------------
	| Delete item off display
	---------------------------*/
	for (line_cnt = lineNo; line_cnt < lcount [scr] - 1; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		memcpy (&store [line_cnt], &store [line_cnt + 1],
					sizeof store [0]);
		if (line_cnt / TABLINES == pageNo)
			line_display ();
	}

	/*-----------------------
	| Blank the last line 
	-----------------------*/
	ClearLine (line_cnt);
	if (line_cnt / TABLINES == pageNo)
		blank_display ();

	line_cnt = lineNo;
	getval (line_cnt);
	lcount [scr]--;
}


/*===========================
| Add header record to sacm 
===========================*/
static void
AddSalesCallHeader (void)
{
	strcpy (sacm_rec.co_no, comm_rec.co_no);
	sacm_rec.hhsf_hash = exsf_rec.hhsf_hash;
	cc = abc_add (sacm, &sacm_rec);
	if (cc)
		file_err (cc, sacm, "DBADD");
	abc_unlock (sacm); 

	/*---------------------------------
	| Reread sacm_rec because abc_add ()
	| does not update sacm_hash field
	---------------------------------*/
	strcpy (sacm_rec.co_no, comm_rec.co_no);
	sacm_rec.hhsf_hash = exsf_rec.hhsf_hash;
	cc = find_rec (sacm, &sacm_rec, EQUAL, "r");	  
	if (cc)
		file_err (cc, sacm, "DBFIND");
}

/*==============================
| Update header record in sacm 
==============================*/
static void
UpdateSalesCallHeader (void)
{
	cc = abc_update (sacm, &sacm_rec);
	if (cc)
		file_err (cc, sacm, "DBUPDATE");
	abc_unlock (sacm); 
}


/*===============================
| Search External Salesman File
===============================*/
static void
SrchExsf (
 char*              key_val)
{
	struct exsfRecord exsf_bak;

	memcpy (&exsf_bak, &exsf_rec, sizeof exsf_bak);

	_work_open (2,0,40);
	save_rec ("#No", "#Salesperson code description.");

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
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, temp_str);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, exsf, "DBFIND");
	}

	if (cc)
		memcpy (&exsf_rec, &exsf_bak, sizeof exsf_rec);
}

/*===============================
| Search Sales Call Activity File
===============================*/
static void
SrchSacm (
 long               hhsf_hash)
{
	struct sacmRecord sacm_bak;

	memcpy (&sacm_bak, &sacm_rec, sizeof sacm_bak);

	_work_open (2,0,40);
	save_rec ("#Date", "#Speedometer Readings");

	strcpy (sacm_rec.co_no, comm_rec.co_no);
	sacm_rec.hhsf_hash = hhsf_hash;

	cc = find_rec (sacm, &sacm_rec, GTEQ, "r");

	while (!cc && 
			!strcmp (sacm_rec.co_no, comm_rec.co_no) &&
			sacm_rec.hhsf_hash == hhsf_hash)
	{
		char workDate 	[11];
		char speedo 	[30];

		strcpy (workDate, DateToString (sacm_rec.date));
		sprintf (speedo, "%8.1f - %8.1f", sacm_rec.st_km, sacm_rec.ed_km);

		cc = save_rec (workDate, speedo);
		if (cc)
			break;

		cc = find_rec (sacm, &sacm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (sacm_rec.co_no, comm_rec.co_no);
		sacm_rec.hhsf_hash = hhsf_hash;
		sacm_rec.date = StringToDate (temp_str);
		cc = find_rec (sacm, &sacm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, sacm, "DBFIND");
	}
	if (cc)
		memcpy (&sacm_rec, &sacm_bak, sizeof sacm_rec);
}


/*===============================
| Search Sales Call Activity File
===============================*/
static void
SrchSaca (
 char*              key_val)
{
	struct sacaRecord saca_bak;

	memcpy (&saca_bak, &saca_rec, sizeof saca_bak);

	_work_open (2,0,40);
	save_rec ("#No", "#Sales call activity description");

	strcpy (saca_rec.co_no, comm_rec.co_no);
	strcpy (saca_rec.code, key_val);

	cc = find_rec (saca, &saca_rec, GTEQ, "r");

	while (!cc && 
			!strcmp (saca_rec.co_no, comm_rec.co_no) &&
			!strncmp (saca_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (saca_rec.code, saca_rec.desc);
		if (cc)
			break;

		cc = find_rec (saca, &saca_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (saca_rec.co_no, comm_rec.co_no);
		strcpy (saca_rec.code, temp_str);
		cc = find_rec (saca, &saca_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, saca, "DBFIND");
	}

	if (cc)
		memcpy (&saca_rec, &saca_bak, sizeof saca_rec);
}


/*===========================
| edit () callback function
| - tabular screens only
===========================*/
void
tab_other (
 int                lineNo)
{
	if (strcmp (saca_rec.code, sacd_rec.act_code))
    {
        strcpy (saca_rec.co_no, comm_rec.co_no);
        strcpy (saca_rec.code, sacd_rec.act_code);
        if (find_rec (saca, &saca_rec, EQUAL, "r"))
            memset (&saca_rec, 0, sizeof saca_rec);

        DisplayActivityCode ();
    }
	return;
}


/*===========================
| edit () callback function |
===========================*/
int
heading (
 int                scn)
{

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		switch (scn)
		{
		case 1:
			clear ();

			strcpy (err_str, ML (" SALES CALL MAINTENANCE "));
			rv_pr (err_str, (LCL_SCR_WIDTH - strlen (err_str)) / 2, 0, 1);
			line_at (1,0, LCL_SCR_WIDTH);

			box (BOX_COL, BOX_ROW, BOX_WIDTH, BOX_HEIGHT);

			line_at (20,0, LCL_SCR_WIDTH);
			print_at (21,0, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
			line_at (22,0, LCL_SCR_WIDTH);
			break;
		
		case 2:
			/*-------------------------------
			| Other screens must be redrawn
			| if this screen is redrawn
			-------------------------------*/
			DrawScreen (1);

			tab_row = 18 - TABLINES;
			tab_col = 5;

			break;
		}

		scn_write (scn);	/* Draw prompts only */
	}
    return (EXIT_SUCCESS);
}

static void
DrawScreen (
 int scn)
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
	char buf [LCL_SCR_WIDTH + 1];

	sprintf (buf, "  %-24s:  %s - %s", prompt, code, desc);
	
	while (strlen (buf) < sizeof (buf) - 1)
		strcat (buf, " ");
	rv_pr (buf, 0, row, 1);
}

static void
DisplayActivityCode (void)
{
	DisplayCodeDesc (tab_row - 2, "Call Activity Code",
									saca_rec.code, saca_rec.desc);
}

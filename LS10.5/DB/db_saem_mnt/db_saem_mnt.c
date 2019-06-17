/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_saem_mnt.c,v 5.5 2002/07/24 08:38:49 scott Exp $
|  Program Name  : (db_saem_mnt) 
|  Program Desc  : (Salesperson Expense Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (07/06/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: db_saem_mnt.c,v $
| Revision 5.5  2002/07/24 08:38:49  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:24:15  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/26 04:34:19  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/06/26 04:26:54  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.1  2001/12/07 04:03:39  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_saem_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_saem_mnt/db_saem_mnt.c,v 5.5 2002/07/24 08:38:49 scott Exp $";

#define MAXLINES	500
#define TABLINES	10

#define	CCMAIN
#include <pslscr.h>

#define DIMOF(array) (sizeof (array) / sizeof (array [0]))

typedef int	BOOL;
#include <minimenu.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define SEL_UPDATE	0
#define SEL_IGNORE	1
#define SEL_DELETE	2
#define SEL_DEFAULT	99

#define LCL_SCR_WIDTH	132
#define BOX_WIDTH	130
#define BOX_HEIGHT	3
#define BOX_ROW		2
#define BOX_COL		 ( (LCL_SCR_WIDTH - BOX_WIDTH) / 2)

#include	"schema"

struct commRecord	comm_rec;
struct saemRecord	saem_rec;
struct saedRecord	saed_rec;
struct exsfRecord	exsf_rec;
struct cuinRecord	cuin_rec;

static char	*data  = "data",
			*glmr2  = "glmr2";

struct storeRec
{
    char	dkt_no [sizeof saed_rec.dkt_no];
	double	amt;
	double	refunded;
	char	gl_posted [sizeof saed_rec.gl_posted];
	double	gl_amt;
} store [MAXLINES];

static char *scn_desc [] =
{
	"HEADER SCREEN",
	"ITEM DETAILS SCREEN"
};

/*============================
| Local & Screen Structures. |
============================*/
static	struct
{
	char	dummy 		 [11];
	char	salesman_no [3];
	char	acc_no 		 [MAXLEVEL + 1];
	char	trip_no 	 [21];
	double	refunded;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "salesman_no",	3, 4, CHARTYPE,
		"UU", "          ",
		" ", " ", "Salesperson No.       ", 
		"Enter Salesperson No. [SEARCH] for valid codes.",
		NE, NO,  JUSTRIGHT, "", "", local_rec.salesman_no},
	{1, LIN, "exsf_salesman",	3, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "acc_no",	5, 4, CHARTYPE,
		GlMask, "                          ",
		" ", " ", "Sales GL Expense Code ", " ",
		YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.acc_no},
	{1, LIN, "acc_desc",	5, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", glmrRec.desc},
	{2, TAB, "dkt_no", MAXLINES, 1, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Document No", "Enter Docket Number",
		YES, NO,  JUSTLEFT, "", "", saed_rec.dkt_no},
	{2, TAB, "dkt_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
		"          ", " ", " ", " Sales Expense Description                  ", 
		"Enter Docket Description",
		YES, NO,  JUSTLEFT, "", "", saed_rec.dkt_desc},
	{2, TAB, "date",	0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "  Date  ", "Enter Expense Date",
		YES, NO,  JUSTLEFT, "", "", (char *)&saed_rec.date},
	{2, TAB, "amt",	0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Amount    ", "Enter Expense Amount",
		YES, NO,  JUSTRIGHT, "", "", (char *) &saed_rec.amt},
	{2, TAB, "refunded",	0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Refunded  ", "Enter Amount Refunded through Deduction from Trip Remittance",
		NO, NO,  JUSTRIGHT, "", "", (char *) &local_rec.refunded},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};


static BOOL		newRecord = FALSE;

static int		old_lines = 0;

static char		sman_code 	 [3]		=	" ";
extern	int	TruePosition;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
int spec_valid (int field);
static void LoadSalesExpDetails (int scn);
static void UpdateSalesExpDetails (int scn);
static BOOL IsItemInTable (int scn, char *dkt_no);
static void ClearLine (int lineNo);
static void DeleteCurrentLine (int scr);
static void AddSalesExpHeader (void);
static void UpdateSalesExpHeader (void);
static void SrchExsf (char *key_val);
static void SrchCuin (char *key_val);
void tab_other (int lineNo);
int heading (int scn);
static void DrawScreen (int scn);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	int	i;

	TruePosition	=	TRUE;

	sprintf (GlMask,   "%-*.*s", MAXLEVEL, MAXLEVEL, "AAAAAAAAAAAAAAAA");
	sprintf (GlDesc,   "%-*.*s", MAXLEVEL, MAXLEVEL, " Account Number ");
	sprintf (GlfDesc,   "%-*.*s",FORM_LEN, FORM_LEN, " Account Number ");
	sprintf (GlFormat, "%-*.*s", MAXLEVEL, MAXLEVEL, "XXXXXXXXXXXXXXXX");
	if (argc > 1)
	{
		if (argc != 3)
		{
			print_at (0,0,"Usage : %s Optional <Salesman Code>", argv [0]);
            return (EXIT_FAILURE);
		}
		sprintf (sman_code, "%-2.2s", 	argv [1]);
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR 	 (vars);

	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	OpenDB		 ();
	
	GL_SetMask 	 (GlFormat);

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
		entry_exit 	= 	FALSE; 
		edit_exit 	= 	FALSE;
		restart 	= 	FALSE;
		search_ok 	= 	TRUE;
		lcount [2]	=	0;

		memset (store, 0, sizeof store);
		for (i = 0; i < MAXLINES; i++)
    		strcpy (store [i].gl_posted, "N");

		/*-------------------------------
		| Enter screen 1 linear input
		-------------------------------*/
		init_vars 	 (1);
		init_vars 	 (2);
		heading 	 (1);
		entry 		 (1);
		if (!prog_exit && !restart)
		{
			/*------------------------
			| Screen 2 tabular input
			------------------------*/
			LoadSalesExpDetails (2);

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
						AddSalesExpHeader ();
					else
						UpdateSalesExpHeader ();

					UpdateSalesExpDetails (2);
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (saem, saem_list, SAEM_NO_FIELDS, "saem_id_no");
	open_rec (saed, saed_list, SAED_NO_FIELDS, "saed_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_inv_no");

	abc_alias (glmr2, glmr);
	OpenGlmr ();
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (glmr2);
	abc_fclose (cuin);
	abc_fclose (exsf);
	abc_fclose (saed);
	abc_fclose (saem);
	GL_Close ();
	abc_dbclose (data);
}


int
spec_valid (
 int                field)
{
	/*-------------------------
	| Validate Salesman No.
	-------------------------*/
	if (LCHECK ("salesman_no"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			exsf_rec.hhsf_hash = 0L;
			return 0;
		}
		
		if (dflt_used)
			sprintf (local_rec.salesman_no, "%-6.6s", sman_code);

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.salesman_no);
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str, "Salesperson %s is not on file.\007\007",
						local_rec.salesman_no);
			print_mess (err_str);
			sleep (sleepTime);
			return 1;
		}

		DSP_FLD ("exsf_salesman");
		DSP_FLD ("salesman_no");
		/*------------------------------
		| Load Salesperson Expense record
		------------------------------*/
		strcpy (saem_rec.co_no, comm_rec.co_no);
		saem_rec.hhsf_hash = exsf_rec.hhsf_hash;
	    cc = find_rec (saem, &saem_rec, EQUAL, "u");	  
		if (cc)
			newRecord = TRUE;
		else
		{
			newRecord = FALSE;
			entry_exit = TRUE;

			if (!find_hash (glmr2, &glmrRec, EQUAL, "r", saem_rec.hhmr_hash))
				strcpy (local_rec.acc_no, glmrRec.acc_no);
			else
				*local_rec.acc_no = 0;

			DSP_FLD ("acc_no");
			DSP_FLD ("acc_desc");
		}
		return 0;
	}

	if (LCHECK ("acc_no")) 
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (local_rec.acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'F' ||
		    glmrRec.glmr_class [2][0] != 'P')
		{
			print_mess (ML (mlStdMess025));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("acc_desc");
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("amt"))
	{
		local_rec.refunded = saed_rec.amt;
		DSP_FLD	 ("refunded");
		return 0;
	}
	
	/*----------------------
	| Validate Item number |
	----------------------*/
	if (LCHECK ("dkt_no"))
	{
		if (last_char == DELLINE)
		{
			if (prog_status != ENTRY)
				DeleteCurrentLine (vars [field].scn);
			else
			{
				print_mess (" Cannot Delete Lines on Entry ");
				return 1;
			}
			return 0;
		}

		if (SRCH_KEY)
		{
			SrchCuin (temp_str);
			return 0;
	  	}

		if (IsItemInTable (vars [field].scn, saed_rec.dkt_no))
		{
			sprintf (err_str, "Document Number %s already entered",
									saed_rec.dkt_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		if (prog_status == ENTRY ||
			strcmp (store [line_cnt].dkt_no, saed_rec.dkt_no))
		{
			/*-----------------------
			| Item added or changed
			-----------------------*/
			strcpy (store [line_cnt].dkt_no, saed_rec.dkt_no);
		}

		return 0;
	}

	/*-----------------------
	| Validate Expense Date |
	-----------------------*/
	if (LCHECK ("date"))
	{
		if (dflt_used)
			saed_rec.date = TodaysDate ();

		if (saed_rec.date > TodaysDate ())
		{
			print_mess (ML ("Date cannot be greater than date today"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return 0;
	}

	/*--------------------
	| Validate Refunded  |
	--------------------*/
	if (LCHECK ("refunded"))
	{
		if (local_rec.refunded > saed_rec.amt)
		{
			sprintf (err_str, ML ("Amount refunded greater than Expense amount"));
			print_mess (err_str);
			sleep (sleepTime);
			return 1;
		}
		if (dflt_used)
		{
			local_rec.refunded = saed_rec.amt;
			store [line_cnt].refunded = local_rec.refunded;
			DSP_FLD ("refunded");
			return 0;
		}
		store [line_cnt].refunded = local_rec.refunded;
		return 0;
	}
	return 0;

}

/*===============================================
| Load Sales Expense details from saed
| and display.
| If more than MAXLINES exist, they are ignored
===============================================*/
static void
LoadSalesExpDetails (
 int                scn)
{
    /*-----------------------
    | Set screen for putval
    -----------------------*/
    scn_set (scn);
    init_vars (scn);

    lcount [scn] = 0;

	if (!newRecord)
	{
		saed_rec.saem_hash = saem_rec.saem_hash;
		saed_rec.seq_no = 0;
		cc = find_rec (saed, &saed_rec, GTEQ, "r");
		while (	!cc && saed_rec.saem_hash == saem_rec.saem_hash)
		{
			/*--------------------------------
			| Put Value Into Tabular Screen. |
			--------------------------------*/
			strcpy (store [ lcount [scn] ].dkt_no, saed_rec.dkt_no);
			store [lcount [scn]].refunded 	= 	saed_rec.refunded;
			store [lcount [scn]].amt 		= 	saed_rec.amt;
			store [lcount [scn]].gl_amt		=	saed_rec.gl_amt;
			local_rec.refunded 					= 	saed_rec.refunded;
			strcpy (store [ lcount [scn] ].gl_posted, saed_rec.gl_posted);

			putval (lcount [scn]++);

			if (lcount [scn] == MAXLINES)
			{
				print_mess (ML (mlStdMess008));
				sleep (sleepTime);
				break;
			}

			cc = find_rec (saed, &saed_rec, NEXT, "r");
		}
	}
	old_lines = lcount [scn];
	line_cnt = 0;
	DrawScreen (scn);

	clear_mess ();
}


/*==============================
| Update detail records in saed 
==============================*/
static void
UpdateSalesExpDetails (
 int                scn)
{

	print_mess (ML (mlStdMess035));

    scn_set (scn);

	/*-----------------------
	| Remove existing records
	-----------------------*/
    saed_rec.saem_hash = saem_rec.saem_hash;
    saed_rec.seq_no = 0;
    cc = find_rec (saed, &saed_rec, GTEQ, "r");
    while (	!cc && saed_rec.saem_hash == saem_rec.saem_hash)
    {
		cc = abc_delete (saed);
	  	if (cc)
			file_err (cc, saed, "DBDELETE");

		cc = find_rec (saed, &saed_rec, GTEQ, "r");
	}

	/*-----------------------
	| Write new records 
	-----------------------*/
   	for (line_cnt = 0; line_cnt < lcount [scn]; line_cnt++) 
   	{
    	getval (line_cnt);

    	saed_rec.saem_hash 	= saem_rec.saem_hash;
    	saed_rec.seq_no 	= line_cnt + 1;
    	saed_rec.refunded 	= store [line_cnt].refunded;
    	saed_rec.gl_amt 	= store [line_cnt].gl_amt;
    	strcpy (saed_rec.dkt_no, 	store [line_cnt].dkt_no);
    	strcpy (saed_rec.gl_posted, store [line_cnt].gl_posted);
    	strcpy (saed_rec.stat_flag, "0");
		cc = abc_add (saed, &saed_rec);
		if (cc)
			file_err (cc, saed, "DBADD");
	}
	abc_selfield (saed, "saed_id_no");

	clear_mess ();
}


/*===========================
| Return TRUE if item
| has been entered in table
===========================*/
static BOOL
IsItemInTable (
 int                scn,
 char*              dkt_no)
{
	int i, last;

	last = (prog_status == ENTRY ? line_cnt : lcount [scn]);

	for (i = 0; i < last; i++)
	{
		if (!strcmp (dkt_no, store [i].dkt_no) && i != line_cnt)
			return TRUE;
	}
	return FALSE;
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
	memset (&saed_rec, 0, sizeof saed_rec);
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
| Add header record to saem 
===========================*/
static void
AddSalesExpHeader (void)
{
	strcpy (saem_rec.co_no, comm_rec.co_no);
	saem_rec.hhsf_hash = exsf_rec.hhsf_hash;
	saem_rec.hhmr_hash = glmrRec.hhmr_hash;
	cc = abc_add (saem, &saem_rec);
	if (cc)

		file_err (cc, saem, "DBADD");
	abc_unlock (saem); 

	/*---------------------------------
	| Reread saem_rec because abc_add ()
	| does not update saem_hash field
	---------------------------------*/
	strcpy (saem_rec.co_no, comm_rec.co_no);
	saem_rec.hhsf_hash = exsf_rec.hhsf_hash;
	cc = find_rec (saem, &saem_rec, EQUAL, "r");	  
	if (cc)
		file_err (cc, saem, "DBFIND");
}

/*==============================
| Update header record in saem 
==============================*/
static void
UpdateSalesExpHeader (void)
{
	saem_rec.hhmr_hash = glmrRec.hhmr_hash;
	cc = abc_update (saem, &saem_rec);
	if (cc)
		file_err (cc, saem, "DBUPDATE");
	abc_unlock (saem); 
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
| Search Customer Invoice File
===============================*/
static void
SrchCuin (
 char*              key_val)
{
	struct cuinRecord cuin_bak;

	memcpy (&cuin_bak, &cuin_rec, sizeof cuin_bak);

	_work_open (8,0,20);
	save_rec ("#Invoice", "#Inv. Date");

	strcpy (cuin_rec.inv_no, key_val);

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");

	while (!cc && !strncmp (cuin_rec.inv_no, key_val, strlen (key_val)))
	{
		strcpy (err_str, DateToString (cuin_rec.date_of_inv));
		cc = save_rec (cuin_rec.inv_no, err_str);
		if (cc)
			break;

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (cuin_rec.inv_no, temp_str);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cuin, "DBFIND");
	}

	if (cc)
		memcpy (&cuin_rec, &cuin_bak, sizeof cuin_rec);
}


/*==================================================
| edit () callback function - tabular screens only |
==================================================*/
void
tab_other (
 int                lineNo)
{
	FLD ("dkt_no")		=	YES;
	FLD ("dkt_desc")	=	YES;
	FLD ("date")		=	YES;
	FLD ("amt")			=	YES;
	FLD ("refunded")	=	NO;

    if (store [lineNo].gl_posted [0] != 'N')
	{
		print_mess (ML ("Line cannot be changed as it has been posted to GL."));

		FLD ("dkt_no")		=	NA;
		FLD ("dkt_desc")	=	NA;
		FLD ("date")		=	NA;
		FLD ("amt")			=	NA;
		FLD ("refunded")	=	NA;
	}
	else
		clear_mess ();
	return;
}

/*===========================
| edit () callback function |
===========================*/
int 
heading (
 int                scn)
{
	char	title [41];

	strcpy (title, ML (" SALESPERSON EXPENSE MAINTENANCE "));
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		switch (scn)
		{
		case 1:
			clear ();

			rv_pr (title, (LCL_SCR_WIDTH - strlen (title)) / 2, 0, 1);
			line_at (1,0, LCL_SCR_WIDTH);

			box (BOX_COL, BOX_ROW, BOX_WIDTH, BOX_HEIGHT);
			line_at (BOX_ROW + 2, BOX_COL + 1, BOX_WIDTH - 1);

			line_at (21,0,LCL_SCR_WIDTH);
			print_at (22,0, ML (mlStdMess038), 
						comm_rec.co_no, comm_rec.co_name);
			line_at (23,0,LCL_SCR_WIDTH);
			break;
		
		case 2:
			/*-------------------------------
			| Other screens must be redrawn
			| if this screen is redrawn
			-------------------------------*/
			DrawScreen (1);

			tab_row = 18 - TABLINES;
			tab_col = 1;

			break;
		}

		scn_write (scn);	/* Draw prompts only */
	}
    return (EXIT_SUCCESS);
}

static void
DrawScreen (
 int                scn)
{
	heading (scn);
	scn_display (scn);
}

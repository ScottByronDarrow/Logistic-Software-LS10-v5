/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: para_inp.c,v 5.3 2002/07/18 07:04:12 scott Exp $
|  Program Name  : (qt_para_inp.c )                                   |
|  Program Desc  : (Inquiry Note Pad Input Program.             )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/01/1991       |
|---------------------------------------------------------------------|
| $Log: para_inp.c,v $
| Revision 5.3  2002/07/18 07:04:12  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 08:44:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:49  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/05 06:07:30  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: para_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_para_inp/para_inp.c,v 5.3 2002/07/18 07:04:12 scott Exp $";

#define MAXSCNS 	2
#define MAXLINES	500

#define	TXT_REQD

#define	X_OFF		0
#define	Y_OFF		0

#define	TXT_X		4
#define	TXT_Y		8

#include <std_decs.h>
#include <pslscr.h>
#include <hot_keys.h>
#include <ml_qt_mess.h>
#include <ml_std_mess.h>
#include <tabdisp.h>

	/*==========================
	| Special fields and flags. |
	===========================*/
	int		newNote = 0;
	static 	int		line_no;


#include	"schema"

struct commRecord	comm_rec;
struct qtlhRecord	qtlh_rec;
struct qtldRecord	qtld_rec;

#include	<qt_commands.h>

	char	dot_desc [200] [N_CMDS];

int		clearScreenTwo		= 0,
		clearOK				= 0,
		firstTime 			= TRUE;

char	*data	= "data";

/*===========================
| Local & Screen Structures |
===========================*/
struct {					/*---------------------------------------*/
	char	dummy [11];		/*| Dummy Used In Screen Generator.		|*/
	char	comments [121]; /*| Holds Comments for each line.       |*/
	char	par_code [11];  /*| Holds Paragraph Number              |*/
	char	par_desc [41];  /*| Holds Paragraph Description         |*/
	char	code_from [11]; /*| Holds Paragraph Number to be copied |*/
							/*|_____________________________________|*/
} local_rec;            

	char	LineBlank [121];  

static struct	var vars [] =
{
	{1, LIN, "code", 3, 22, CHARTYPE,
	   "UUUUUUUUUU", "          ", 
	   " ", "", "Paragraph Code", " ",
	   NE, NO, JUSTLEFT, "", "", local_rec.par_code},
	{1, LIN, "code_from", 3, 100, CHARTYPE,
	   "UUUUUUUUUU","          ",
	   " ", " ", "Copy from Paragraph", " ",
	   NO, NO, JUSTLEFT, "", "", local_rec.code_from},
	{1, LIN, "desc", 4, 22, CHARTYPE,
	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
	   " ", qtlh_rec.par_desc, "Description ", " ",
	   YES, NO, JUSTLEFT, "", "", local_rec.par_desc},

	{2, TXT, "comm", TXT_Y, TXT_X, 0,
	   "", "          ",
	   " ", " ", ".....T.........T.........T.........T.........T.........T.........T.........R.........T.........T.........T.......", "",
	   10, 120, MAXLINES, "", "", local_rec.comments},

	{0, LIN, "", 0, 0, INTTYPE,
	   "A", "          ",
	   " ", "",  "dummy", " ",
	   YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

static  int InsertFunc 		(int, KEY_TAB *);
static  int OverwriteFunc 	(int, KEY_TAB *);
static  int RestartFunc 	(int, KEY_TAB *);
static  int ExitFunc 		(int, KEY_TAB *);

#define RETURN_KEY		13
#define INSERT_KEY		'I'
#define OVERWRITE_KEY	'O'

#ifdef	GVISION
static  KEY_TAB listKeys [] =
{
   { NULL,		RETURN_KEY,			InsertFunc,
    "Insert Dot Command",	    						"A" },
   { " INSERT ",		INSERT_KEY,			InsertFunc,
    "Insert Dot Command",	    						"A" },
   { " OVERWRITE ",		OVERWRITE_KEY,			OverwriteFunc,
    "Overwrite with Dot Command",	    						"A" },
   { NULL,				FN1,				RestartFunc,
    "Exit without update.",						"A" },
   { NULL,				FN16,				ExitFunc,
    "Exit and update the database.",			"A" },
   END_KEYS
};
#else
static  KEY_TAB listKeys [] =
{
   { NULL,		RETURN_KEY,			InsertFunc,
    "Insert Dot Command",	    						"A" },
   { " [I]nsert ",		INSERT_KEY,			InsertFunc,
    "Insert Dot Command",	    						"A" },
   { " [O]verwrite ",		OVERWRITE_KEY,			OverwriteFunc,
    "Overwrite with Dot Command",	    						"A" },
   { NULL,				FN1,				RestartFunc,
    "Exit without update.",						"A" },
   { NULL,				FN16,				ExitFunc,
    "Exit and update the database.",			"A" },
   END_KEYS
};
#endif
/*==========================
| Function prototypes.     |
==========================*/
int	  	heading 	 (int);
void	OpenDB		 (void);
void	CloseDB		 (void);
int		LoadDots	 (void);
int		ShowDots	 (void);
void	ReadQtld	 (long);
void	SrchQtlh	 (char *);
int		UpdateData	 (void);
void	PrintCoStuff (void);
void	InitML		 (void);

static int InsertFunc		 (int, KEY_TAB *);
static int OverwriteFunc	 (int, KEY_TAB *);
static int RestartFunc		 (int, KEY_TAB *);
static int ExitFunc			 (int, KEY_TAB *);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (LineBlank, "%120.120s", " ");

	InitML ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		clearOK 	= TRUE;
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	
		init_vars (2);	
		lcount [2] = 0;

		clearScreenTwo = TRUE;
		heading (1);
		clearScreenTwo = FALSE;

		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (2);
		scn_display (2);

		if (newNote == 1 && lcount [2] == 0)
			entry (2);
		else
			edit (2);

		edit_all ();

		if (restart)
			continue;

		UpdateData ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}


/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (qtld, qtld_list, QTLD_NO_FIELDS, "qtld_id_no");
	open_rec (qtlh, qtlh_list, QTLH_NO_FIELDS, "qtlh_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose  (qtld);
	abc_fclose  (qtlh);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*-----------------------
	| Validate Para Code    |
	-----------------------*/
	if (LCHECK ("code"))
	{
    	if (SRCH_KEY)
    	{
			SrchQtlh (temp_str);
			sprintf (local_rec.par_desc,"%-40.40s",qtlh_rec.par_desc);
			return (EXIT_SUCCESS);
    	}

    	strcpy (qtlh_rec.co_no, comm_rec.co_no);
    	strcpy (qtlh_rec.par_code, local_rec.par_code);
    	if (!find_rec (qtlh, &qtlh_rec, COMPARISON, "u"))
    	{
    		strcpy (local_rec.par_desc, qtlh_rec.par_desc);
			DSP_FLD ("desc");
			vars [label ("code_from")].required = NA;
			ReadQtld (qtlh_rec.hhlh_hash);

			entry_exit = 1;
			newNote = 0;
    	}
		else
		{
			abc_unlock ("qtlh");
			vars [label ("code_from")].required = YES;
			newNote = 1;
		}

    	return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Para Code_From. |
	--------------------------*/
	if (LCHECK ("code_from"))
	{
    	if (SRCH_KEY)
    	{
			SrchQtlh (temp_str);
			return (EXIT_SUCCESS);
    	}
	
    	if (!strcmp (local_rec.code_from,"          "))
			return (EXIT_SUCCESS);
	
    	strcpy (qtlh_rec.co_no, comm_rec.co_no);
    	strcpy (qtlh_rec.par_code, local_rec.code_from);
    	if (!find_rec (qtlh, &qtlh_rec, COMPARISON, "r"))
			ReadQtld (qtlh_rec.hhlh_hash);
    	else
    	{
			/*----------------------
			| Paragraph not found. |
			----------------------*/
			errmess (ML (mlStdMess047));
			return (EXIT_FAILURE);
    	}
    	return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Comment Line. |
	------------------------*/
	if (LCHECK ("comm"))
    {
		if (SRCH_KEY)
		{
			ShowDots ();
			clearOK = FALSE;
			heading (2);
			clearOK = TRUE;
			crsr_on ();
			scn_set (2);

			return (EXIT_SUCCESS);
		}
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Load dot commands and descriptions  |
-------------------------------------*/
int
LoadDots (void)
{
	int		i;
	char	disp_str [200];
	
	tab_open ("dot_tab", listKeys, 1, 77, 18, FALSE);
	tab_add ("dot_tab", 
		"#.                   Dot Commands                    ");

	for (i = 0; i < N_CMDS; i++)
	{
		sprintf (disp_str," .%-7.7s  %-40.40s ",dot_cmds [i], dot_desc [i]);
		tab_add ("dot_tab", disp_str);
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Show dot commands and descriptions  |
-------------------------------------*/
int
ShowDots (void)
{
	int		dotCmd;

	line_no = 0;
	if (firstTime)
	{
		LoadDots ();
		firstTime = FALSE;
	}

	tab_display ("dot_tab", TRUE);
	dotCmd = tab_scan ("dot_tab");
	tab_clear ("dot_tab");

	return (line_no);
}

static int
InsertFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	int		xPos = col - TXT_X - 1;
	int		yPos = row - TXT_Y - 1;
	char	buffer1 [241];
	char	buffer2 [121];

	getval (yPos);
	sprintf (buffer1, "%120.120s", local_rec.comments);
	sprintf (buffer2, "%120.120s", &local_rec.comments [xPos]);
	line_no = tab_tline ("dot_tab");
	buffer1 [xPos] = '.';
	xPos++;
	memcpy (&buffer1 [xPos],&dot_cmds [line_no][0],strlen (dot_cmds [line_no]));
	xPos = xPos + strlen (dot_cmds [line_no]);
	memcpy (&buffer1 [xPos], &buffer2 [0], strlen (buffer2));
	sprintf (local_rec.comments, "%-120.120s", buffer1);
	putval (yPos);
	restart = FALSE;
	return (FN16);
}

static int
OverwriteFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	int	xPos = col - TXT_X - 1;
	int	yPos = row - TXT_Y - 1;

	getval (yPos);
	line_no = tab_tline ("dot_tab");
	local_rec.comments [xPos] = '.';
	memcpy (&local_rec.comments [xPos + 1], 
			&dot_cmds [line_no] [0], 
			strlen (dot_cmds [line_no]));
	putval (yPos);
	restart = FALSE;
	return (FN16);
}

static int
RestartFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	restart = FALSE;
	return key;
}

static int
ExitFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	return key;
}

/*=======================
| Read paragraph lines. |
=======================*/
void
ReadQtld (
 long	shash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	qtld_rec.hhlh_hash = shash;
	qtld_rec.line_no = 0; 

	cc = find_rec (qtld, &qtld_rec, GTEQ, "r");
	while (!cc && qtld_rec.hhlh_hash == shash)
	{
		strcpy (local_rec.comments, qtld_rec.desc);
		putval (lcount [2]++);
		cc = find_rec (qtld, &qtld_rec, NEXT, "r");
	}
	scn_set (1);
}

/*=============================================
| Search routine for Paragraph master file. |
=============================================*/
void
SrchQtlh (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code","#Description");
	strcpy (qtlh_rec.co_no,comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", key_val);
	cc = find_rec (qtlh, &qtlh_rec, GTEQ, "r");
	while (!cc && !strcmp (qtlh_rec.co_no,comm_rec.co_no) &&
	          !strncmp (qtlh_rec.par_code,key_val,strlen (key_val)))
	{
		cc = save_rec (qtlh_rec.par_code,qtlh_rec.par_desc);
		if (cc)
			break;

		cc = find_rec (qtlh, &qtlh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qtlh_rec.co_no,comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", key_val);
	cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qtld, "DBFIND");
}

/*===================
| Update all files. |
===================*/
int
UpdateData (void)
{
	int 	add_item = FALSE;
	int		wk_line;
	int		i;

	clear ();
		
	if (newNote == 1) 
	{
		/*----------------------------
		| Creating Paragraph Record. |
		----------------------------*/
		rv_pr (ML (mlQtMess013) , 2, 0, 1);
		strcpy (qtlh_rec.co_no,    comm_rec.co_no);
		strcpy (qtlh_rec.par_code, local_rec.par_code);
		strcpy (qtlh_rec.par_desc, local_rec.par_desc);
		qtlh_rec.hhlh_hash = 0L;
		cc = abc_add (qtlh, &qtlh_rec);
		if (cc) 
			file_err (cc, qtlh, "DBADD");

		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (cc)
			return (EXIT_FAILURE);
	}
	else
	{
		/*----------------------------
		| Updating Paragraph Record. |
		----------------------------*/
		rv_pr (ML (mlQtMess014), 2, 2, 1);
		strcpy (qtlh_rec.par_desc, local_rec.par_desc);
		cc = abc_update (qtlh, &qtlh_rec);
		if (cc) 
			file_err (cc, qtlh, "DBUPDATE");
	}

	scn_set (2);
	/*-------------------------------
   	| Remove blank lines at bottom. |
   	--------------------------------*/
	for (i = lcount [2] - 1; i > 0; i--)
	{
		getval (i);
		if (!strcmp (local_rec.comments, LineBlank))
			lcount [2]--;
		else
			break;
	}
	
	for (wk_line = 0; wk_line < lcount [2]; wk_line++) 
	{
	   	getval (wk_line);

	   	qtld_rec.hhlh_hash = qtlh_rec.hhlh_hash;
	   	qtld_rec.line_no   = wk_line;
	   	cc = find_rec (qtld, &qtld_rec, COMPARISON, "u");
	   	if (cc)
	   		add_item = TRUE;
	   	else
	   		add_item = FALSE;

	   	strcpy (qtld_rec.desc, local_rec.comments);
	   	if (add_item)
	   	{
			cc = abc_add (qtld, &qtld_rec);
			if (cc) 
				file_err (cc, qtld, "DBADD");
	   	}
	   	else
	   	{
	    	/*------------------------
	    	| Update existing order. |
	    	------------------------*/
	    	cc = abc_update (qtld, &qtld_rec);
	    	if (cc) 
				file_err (cc, qtld, "DBUPDATE");
	   	}
		abc_unlock (qtld);
	}

    for (wk_line = lcount [2];wk_line < MAXLINES; wk_line++)
    {
	   	qtld_rec.hhlh_hash = qtlh_rec.hhlh_hash;
	   	qtld_rec.line_no = wk_line;
	   	cc = find_rec (qtld, &qtld_rec, COMPARISON, "r");
	   	if (!cc)
			abc_delete (qtld);
	   	else
		 	break;
	}
	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (newNote == 0) 
	{	
	   	/*-------------------------
	   	| Delete cancelled order. |
	   	-------------------------*/
	   	if (lcount [2] == 0) 
	   	{
			/*----------------------------
			| Deleting Paragraph Record. |
			----------------------------*/
			rv_pr (ML (mlQtMess015), 2,2,1);
			abc_unlock (qtlh);
			cc = abc_delete (qtlh);
			if (cc)
				file_err (cc, qtld, "DBDELETE");
	   	}
	   	abc_unlock (qtlh);
	    
	}
	return (EXIT_SUCCESS);
}

/*========================
| Print Company Details. |
========================*/
void
PrintCoStuff (void)
{
	line_at (20,0,132);
	print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	line_at (22,0,132);
}

/*================
| Print Heading. |
================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		if (clearOK)
			clear ();
		
		/*--------------------------------
		| P a r a g r a p h   I n p u t. |
		--------------------------------*/
		sprintf (err_str, " %s ", ML (mlQtMess016));
		rv_pr (err_str, 48, 0, 1);
		
		line_at (1,0,130);

		box (0, 2, 130, 2);
		if (scn == 1)
		{
			if (clearScreenTwo)
				init_vars (2);	

			if (clearOK)
			{
				scn_set (2);
				scn_display (2);
			}
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}
		scn_set (scn);

		PrintCoStuff ();
		/*--------------------------------------
		| Use [SEARCH] to display dot commands |
		--------------------------------------*/
		if (scn == 2)
			rv_pr (ML (mlStdMess206), 0, 23, 1);
		else
			clear_mess ();

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
InitML (void)
{
	strcpy (dot_desc [0], ML ("Customer Acronym"));
	strcpy (dot_desc [1], ML ("Customer Number"));
	strcpy (dot_desc [2], ML ("Customer Name"));
	strcpy (dot_desc [3], ML ("Customer Address Part 1"));
	strcpy (dot_desc [4], ML ("Customer Address Part 2"));
	strcpy (dot_desc [5], ML ("Customer Address Part 3"));
	strcpy (dot_desc [6], ML ("Customer Contact name."));
	strcpy (dot_desc [7], ML ("Salesman number."));
	strcpy (dot_desc [8], ML ("Salesman name."));
	strcpy (dot_desc [9], ML ("Quotation number."));
	strcpy (dot_desc [10], ML ("Quotation Salutation."));
	strcpy (dot_desc [11], ML ("Quotation Customer order ref."));
	strcpy (dot_desc [12], ML ("Quotation date."));
	strcpy (dot_desc [13], ML ("Quotation Expiry date."));
	strcpy (dot_desc [14], ML ("Quotation Contact name"));
	strcpy (dot_desc [15], ML ("Quotation comment line one."));
	strcpy (dot_desc [16], ML ("Quotation comment line two."));
	strcpy (dot_desc [17], ML ("Quotation comment line three."));
	strcpy (dot_desc [18], ML ("Line item no."));
	strcpy (dot_desc [19], ML ("Line Description"));
	strcpy (dot_desc [20], ML ("Line Quantity."));
	strcpy (dot_desc [21], ML ("Line Nett price."));
	strcpy (dot_desc [22], ML ("Line gross price."));
	strcpy (dot_desc [23], ML ("Line discount percent."));
	strcpy (dot_desc [24], ML ("Line discount amount."));
	strcpy (dot_desc [25], ML ("Line serial number."));
	strcpy (dot_desc [26], ML ("Total quotation discount."));
	strcpy (dot_desc [27], ML ("Total quotation Tax."));
	strcpy (dot_desc [28], ML ("Total quotation Nett."));
	strcpy (dot_desc [29], ML ("Total quotation gross."));
	strcpy (dot_desc [30], ML ("Total quotation gross + Tax."));
	strcpy (dot_desc [31], ML ("Current module Date, Format DD/MM/YY"));
	strcpy (dot_desc [32], ML ("Full Sys. Date, Format 1st January 1990")); 
	strcpy (dot_desc [33], ML ("Current Date, Format DD/DD/DD"));
	strcpy (dot_desc [34], ML ("Company Name."));
	strcpy (dot_desc [35], ML ("Company Address part one."));
	strcpy (dot_desc [36], ML ("Company Address part two."));
	strcpy (dot_desc [37], ML ("Company Address part three."));
	strcpy (dot_desc [38], ML ("Nett Sub-total"));
	strcpy (dot_desc [39], ML ("Gross Sub-total"));
}

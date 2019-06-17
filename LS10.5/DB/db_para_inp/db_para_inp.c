/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: db_para_inp.c,v 5.3 2002/11/20 07:28:39 kaarlo Exp $
|  Program Name  : (db_para_inp.c)
|  Program Desc  : (Inquiry Note Pad Input Program)
|---------------------------------------------------------------------|
|  Author        : Rommel Maldia.   | Date Written  : 02/13/96        |
|---------------------------------------------------------------------|
| $Log: db_para_inp.c,v $
| Revision 5.3  2002/11/20 07:28:39  kaarlo
| LS 01158 SC 4214. Updated to fix init_vars ().
|
| Revision 5.2  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.1  2001/12/07 03:57:32  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_para_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_para_inp/db_para_inp.c,v 5.3 2002/11/20 07:28:39 kaarlo Exp $";

#define MAXSCNS 	2
#define MAXLINES	500 

#define	TXT_REQD

#define	X_OFF		0
#define	Y_OFF		0

#define	TXT_X		4
#define	TXT_Y		8 

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_db_mess.h>
#include <ml_std_mess.h>
#include <tabdisp.h>

	/*==========================
	| Special fields and flags. |
	===========================*/
	int	new_note = 0;
	static int		line_no;

#include	"schema"

struct commRecord	comm_rec;
struct dblhRecord	dblh_rec;
struct dbldRecord	dbld_rec;

char	*dot_desc [] = 
{
	"Current Date, Format dd/mm/yyyy",
	"Current Date, Format 1st January 2000 ",
	"Company Name.",
	"Company Address part one.",
	"Company Address part two.",
	"Company Address part three.",
	"Customer Number",
	"Customer Acronym",
	"Customer Name",
	"Customer Address Part 1",
	"Customer Address Part 2",
	"Customer Address Part 3",
	"Customer Contact Name.",
	"Customer Contact Position.",
	"Customer Alternate Contact Name.",
	"Customer Alternate Contact Position.",
	"Salesman Number.",
	"Salesman Name.",
	"Area Number.",
	"Area Name.",
	"Customer Contract Type.",
	"Customer Price Type.",
	"Customer Bank Code.",
	"Customer Bank Branch.",
	"Customer Discount Code.",
	"Customer Tax Code.",
	"Customer Tax Number.",
	"Date Of Last Invoice.",
	"Date Of Last Payment.",
	"Amount Of Last Payment.",
	"Month To Date Sales.",
	"Year To Date Sales.",
	"Value Of Current Orders.",
	"Customer Post Code.",
	"Customer Business Sector.",
	"Customer Phone Number.",
	"Customer Fax Number.",
	""
};


#include	<db_commands.h>

int		clr_scn2;
int		clear_ok;
int		first_time = TRUE;

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
	   "UUUUUUUU", "          ", 
	   " ", "", "Paragraph Code", " ",
	   NE, NO, JUSTLEFT, "", "", local_rec.par_code},
	{1, LIN, "code_from", 3, 100, CHARTYPE,
	   "UUUUUUUUUU","          ",
	   " ", " ", "Copy from Paragraph", " ",
	   NO, NO, JUSTLEFT, "", "", local_rec.code_from},
	{1, LIN, "desc", 4, 22, CHARTYPE,
	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
	   " ", dblh_rec.par_desc, "Description ", " ",
	   YES, NO, JUSTLEFT, "", "", local_rec.par_desc},

	{2, TXT, "comm", TXT_Y, TXT_X, 0,
	   "", "          ",
	   " ", " ", ".....10........20........30........40........50........60........70........80........90.......100.......110......", "",
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
   { " Insert ",		INSERT_KEY,			InsertFunc,
    "Insert Dot Command",	    						"A" },
   { " Overwrite ",		OVERWRITE_KEY,			OverwriteFunc,
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

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	LoadDots 		(void);
int 	ShowDots 		(void);
int 	ReadDbld 		(long);
void 	SrchDblh 		(char *);
int 	UpdateData 		(void);
void 	PrintCoStuff 	(void);
int 	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (LineBlank, "%120.120s", " ");

	prog_exit = 0;
	while (prog_exit == 0)
	{
		clear_ok = TRUE;
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars (1);	
//		init_vars (2);	
		lcount [2] = 0;

		clr_scn2 = TRUE; 
		heading (1);
		clr_scn2 = FALSE; 

		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (2);
		scn_display (2);

		if (new_note == 1 && lcount [2] == 0)
		{
			entry (2);
			if (restart)
				continue; 
		}
		else
		{
			edit (2);
			if (restart)
				continue; 
		}
	
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

	open_rec (dbld, dbld_list, DBLD_NO_FIELDS, "dbld_id_no");
	open_rec (dblh, dblh_list, DBLH_NO_FIELDS, "dblh_id_no");
}



/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose  (dbld);
	abc_fclose  (dblh);
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*-----------------------
	| Validate Para Code    |
	-----------------------*/
	if ( LCHECK ("code") )
	{
    	if (SRCH_KEY)
    	{
			SrchDblh (temp_str);
			return (EXIT_SUCCESS);
    	}

    	strcpy (dblh_rec.co_no, comm_rec.co_no);
    	sprintf (dblh_rec.par_code, "C-%-8.8s", local_rec.par_code);
    	if ( !find_rec (dblh, &dblh_rec, COMPARISON, "r") )
    	{
    		strcpy (local_rec.par_desc, dblh_rec.par_desc);
			display_field (label ("desc"));
			FLD ("code_from")	= NA;
			if ( ReadDbld ( dblh_rec.dblh_hash ) )
				return (EXIT_FAILURE);

			entry_exit = 1;
			new_note = 0;
    	}
		else
		{
			FLD ("code_from") = YES;
			new_note = 1;
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
			SrchDblh (temp_str);
			return (EXIT_SUCCESS);
    	}
	
    	if (!strcmp (local_rec.code_from,"          "))
			return (EXIT_SUCCESS);
	
    	strcpy (dblh_rec.co_no, comm_rec.co_no);
    	sprintf (dblh_rec.par_code, "C-%-8.8s", local_rec.code_from);
    	if ( !find_rec (dblh, &dblh_rec, COMPARISON, "r"))
    	{
    		strcpy (local_rec.par_desc, dblh_rec.par_desc);
			display_field (label ("desc")); 
			FLD ("code_from")	= NE;
			if ( ReadDbld (dblh_rec.dblh_hash) )
				return (EXIT_FAILURE);
    	}
    	else
    	{
			/*----------------------
			| Paragraph not found. |
			----------------------*/
			errmess ( ML (mlStdMess047) );
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
			clear_ok = FALSE;
			heading (2);
			clear_ok = TRUE;  
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
void
LoadDots (void)
{
	int		i;
	char	disp_str [200];

	tab_open ("dot_tab", listKeys, 1, 77, 18, FALSE);
	tab_add ("dot_tab", 
		"#                    Dot Commands                    ");

	for ( i = 0; i < N_CMDS; i++ )
	{
		sprintf (disp_str, " .%-7.7s  %-40.40s ", 
				 dot_cmds [i], ML (dot_desc [i]));

		tab_add ("dot_tab", disp_str);
	}
}

/*-------------------------------------
| Show dot commands and descriptions  |
-------------------------------------*/
int
ShowDots (void)
{
	int		dotCmd;

	line_no = 0;
	if (first_time)
	{
		LoadDots ();
		first_time = FALSE;
	}

	tab_display ("dot_tab", TRUE);
	/* dotCmd = tab_scan_line ("dot_tab", line_no); */
	dotCmd = tab_scan ("dot_tab");
	tab_clear ("dot_tab"); 

	return (line_no);
}

static int
InsertFunc (
 int                key,
 KEY_TAB*           psUnused)
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
	memcpy (&buffer1 [xPos], 
			&dot_cmds [line_no] [0], 
			strlen (dot_cmds [line_no]));
	xPos = xPos + strlen (dot_cmds [line_no]);
	memcpy (&buffer1 [xPos], &buffer2 [0], strlen (buffer2));
	sprintf (local_rec.comments, "%-120.120s", buffer1);
	putval (yPos);
	restart = FALSE;
	return (FN16);
}

static int
OverwriteFunc (
 int                key,
 KEY_TAB*           psUnused)
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
 int                key,
 KEY_TAB*           psUnused)
{
	restart = FALSE;
	return key;
}

static int
ExitFunc (
 int                key,
 KEY_TAB*           psUnused)
{   
	return key;
}

/*=======================================================================
=======================================================================*/
int
ReadDbld (
	long	dblhHash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	dbld_rec.dblh_hash 	= dblhHash;
	dbld_rec.line_no 	= 0; 

	cc = find_rec (dbld, &dbld_rec, GTEQ, "r");
	while (!cc && dbld_rec.dblh_hash == dblhHash)
	{
		strcpy (local_rec.comments, dbld_rec.desc);
		putval (lcount [2]++);
		cc = find_rec (dbld, &dbld_rec, NEXT, "r");
	}
	scn_set (1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
/*=============================================
| Search routine for Paragraph master file. |
=============================================*/
void
SrchDblh (
 char*              key_val)
{
	work_open ();
	save_rec ("#Code","#Description");
	strcpy (dblh_rec.co_no,comm_rec.co_no);
	sprintf (dblh_rec.par_code, "C-%-8.8s", key_val);
	cc = find_rec (dblh, &dblh_rec, GTEQ, "r");
	while (!cc && !strcmp (dblh_rec.co_no,comm_rec.co_no) &&
	          !strncmp (dblh_rec.par_code + 2,key_val,strlen (key_val)))
	{
		if (!strncmp (dblh_rec.par_code, "C-", 2))
		{
			cc = save_rec (dblh_rec.par_code + 2,dblh_rec.par_desc);
			if (cc)
				break;
		}

		cc = find_rec (dblh, &dblh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (dblh_rec.co_no,comm_rec.co_no);
	sprintf (dblh_rec.par_code, "C-%-8.8s", key_val);
	cc = find_rec (dblh, &dblh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dblh, "DBFIND");

	sprintf (local_rec.par_desc,"%-40.40s",dblh_rec.par_desc);
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
		
	if (new_note == 1) 
	{
		/*---"Now Creating New Paragraph Record."---*/
		rv_pr ( ML (mlDbMess130), 2, 0, 1);
		strcpy (dblh_rec.co_no,    comm_rec.co_no);
		sprintf (dblh_rec.par_code, "C-%-8.8s", local_rec.par_code);
		strcpy (dblh_rec.par_desc, local_rec.par_desc);
		dblh_rec.dblh_hash = 0L;
		cc = abc_add (dblh, &dblh_rec);
		if (cc) 
			file_err (cc, dblh, "DBADD");

		cc = find_rec (dblh, &dblh_rec, COMPARISON, "r");
		if (cc)
			return (EXIT_FAILURE);

	}
	else
	{
		/*---"Now Updating Paragraph Record."---*/
		rv_pr ( ML (mlDbMess131), 2, 2, 1);
		strcpy (dblh_rec.par_desc, local_rec.par_desc);
		cc = abc_update (dblh, &dblh_rec);
		if (cc) 
			file_err (cc, dblh, "DBUPDATE");
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

	   	dbld_rec.dblh_hash = dblh_rec.dblh_hash;
	   	dbld_rec.line_no   = wk_line;
	   	cc = find_rec (dbld, &dbld_rec, EQUAL, "u");
	   	if (cc)
	   		add_item = TRUE;
	   	else
	   		add_item = FALSE;

	   	strcpy (dbld_rec.desc, local_rec.comments);
	   	if (add_item)
	   	{
			cc = abc_add (dbld, &dbld_rec);
			if (cc) 
				file_err (cc, dbld, "DBADD");
	   	}
	   	else
	   	{
	    	/*-----------------------
	    	| Update existing file. |
	    	-----------------------*/
	    	cc = abc_update (dbld, &dbld_rec);
	    	if (cc) 
				file_err (cc, dbld, "DBUPDATE");
	   	}
		abc_unlock (dbld);
	}
   
    for (wk_line = lcount [2];wk_line < MAXLINES; wk_line++)
    {
	   	dbld_rec.dblh_hash = dblh_rec.dblh_hash;
	   	dbld_rec.line_no = wk_line;
	   	if ( !find_rec (dbld, &dbld_rec, COMPARISON, "r"))
			abc_delete (dbld);
	   	else
		 	break;
	}
	
	/*-----------------------------------
	| Update existing paragraph header. |
	-----------------------------------*/
	if (new_note == 0) 
	{	
	   	/*--------------------------
	   	| Delete paragraph header. |
	   	--------------------------*/
	   	if (lcount [2] == 0) 
	   	{
			rv_pr ( ML (mlDbMess132), 2,2,1);
			abc_unlock (dblh);
			cc = abc_delete (dblh);
			if (cc)
				file_err (cc, dblh, "DBDELETE");
	   	}
	   	abc_unlock (dblh);
	    
	}
	return (EXIT_SUCCESS);
}

/*========================
| Print Company Details. |
========================*/
void
PrintCoStuff (void)
{
	move (0,20);
	line (132);
	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (21,0, "%s", err_str);
	move (0,22);
	line (132);
}



/*================
| Print Heading. |
================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		if (clear_ok)
			clear ();
		
		rv_pr ( ML (mlDbMess062), 48, 0, 1);
		
		move (0, 1);
		line (130);

		box (0, 2, 130, 2);
		if (scn == 1) 
		{
			if (clr_scn2)
				init_vars (2);	

			if (clear_ok)
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
		if (scn == 2)
		{
			rv_pr ( ML (mlDbMess063), 0, 23, 1);
		}
		else
			clear_mess ();

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0; 
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

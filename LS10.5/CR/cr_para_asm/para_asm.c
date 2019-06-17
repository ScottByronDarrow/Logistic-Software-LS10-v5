/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: para_asm.c,v 5.3 2002/07/18 06:17:36 scott Exp $
|  Program Name  : (cr_para_asm.c )                                   |
|  Program Desc  : (Paragraph Assembling Program.             )       |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel.    | Date Written : 18/03/96         |
|---------------------------------------------------------------------|
| $Log: para_asm.c,v $
| Revision 5.3  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 08:52:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:38  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:03:31  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:23:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/23 01:12:53  scott
| Update to ensure these programs will work without change with LS10-GUI.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: para_asm.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_para_asm/para_asm.c,v 5.3 2002/07/18 06:17:36 scott Exp $";

#define MAXWIDTH 	150
#define MAXSCNS 	2
#define TABLINES 	10
#define MAXLINES 	100
#define	MAX_PAR 	30
#define SLEEP_TIME 	3

#define SR 		store  [line_cnt]

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

	/*===========================
	| Special fields and flags. |
	===========================*/
	int 	newNotes,
			envCrFind	= 0,
			envCrCo		= 0,
			blank_win	= TRUE,
			clr_scn2,  
			clearOk;

#include	"schema"

struct commRecord	comm_rec;
struct dblhRecord	dblh_rec;
struct dbldRecord	dbld_rec;
struct dbphRecord	dbph_rec;
struct dbpdRecord	dbpd_rec;

	char	*data = "data";
	
	char	branchNumber [3];

	/*============================
	| Local & Screen Structures. |
	============================*/
	struct store_rec {
		char	_lett_code [9];
		char	_lett_desc [41];
		char	_par_code [9];
		char	_par_desc [41];
		int	    _int_line_no;
		char	_line_desc [121];
	} store [MAXLINES];

	long	_dblh_hash [MAX_PAR];
	char	_par_desc [MAX_PAR] [41];

	/*============================
	| Local & Screen Structures. |
	============================*/
	struct {
		char	dummy  [11];
		char	par_code  [9];
		char	par_desc  [41];
		char	lett_code  [9];
		char	lett_desc  [41];
	} local_rec;

static struct	var vars [] =
{
	{1, LIN, "lett_code",	 3, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Letter Code :", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.lett_code},
	{1, LIN, "lett_desc",	3, 70, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description :", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.lett_desc}, 

	{2, TAB, "par_code",	MAX_PAR, 3, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " Paragraph Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.par_code},

	{0, LIN, "", 0, 0, INTTYPE,
	   "A", "          ",
	   " ", "",  "dummy", " ",
	   YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
int		spec_valid		 (int);
int		DeleteLine		 (void);
int		InsertLine		 (void);
int		LoadDbpd		 (long);
int		UpdateData		 (void);
void	SrchDblh		 (char *);
void	SrchDbph		 (char *);
void	tab_other		 (int);
void	PrintCompany	 (void);
int		heading			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv  [])
{
	int	i;
	
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));
	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	/*===========================
	| Open main database files. |
	===========================*/
	OpenDB ();

	prog_exit = 0;
	tab_row = 7;
	tab_col = 2;
	while (prog_exit == 0)
	{
		for (i = 0; i < MAX_PAR ; i++)
		{
			_dblh_hash [i] = 0L;
			sprintf (_par_desc [i], "%-40.40s", " ");
		}
		for (i = 0; i < MAXLINES; i++)
		{
			sprintf (store  [i]._lett_code, "%-8.8s", " ");
			sprintf (store  [i]._lett_desc, "%-40.40s", " ");
			sprintf (store  [i]._par_code, "%-8.8s", " ");
			sprintf (store  [i]._par_desc, "%-40.40s", " ");
			store  [i]._int_line_no = 0L;
			sprintf (store  [i]._line_desc, "%-120.120s", " ");
		}

		eoi_ok 		= TRUE;
		clearOk 	= TRUE;
		clr_scn2	= TRUE;
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		newNotes 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		init_vars (1);	
		lcount [2] = 0; 

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		clearOk = FALSE;  
		heading (1);

		scn_display (1); 
		entry (1);

		if (prog_exit || restart)
			continue;

	/*** Load the paragraphs if exists else entry ***/	
		if (newNotes == 1 && entry_exit == 0)
		{
			/*-------------------------------
			| Enter screen 2 tabular input. |
			-------------------------------*/
			scn_set (2);
			lcount [2] = 0; 
			clearOk	= FALSE;
			heading (2);
			clearOk = TRUE;
			entry (2);

			if (restart)
				continue;
		}
		else 
		{
			scn_display (1);
			clearOk = FALSE;
			heading (2);
			clearOk = TRUE; 
			scn_display (2);
			edit (2);

			if (restart)
				continue; 
		}

		edit_all ();  
		if (restart)
			continue;

		UpdateData (); 
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (dbld, dbld_list, DBLD_NO_FIELDS, "dbld_id_no");
	open_rec (dblh, dblh_list, DBLH_NO_FIELDS, "dblh_id_no");
	open_rec (dbph, dbph_list, DBPH_NO_FIELDS, "dbph_id_no");
	open_rec (dbpd, dbpd_list, DBPD_NO_FIELDS, "dbpd_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose  (dbld);
	abc_fclose  (dblh);
	abc_fclose  (dbph);
	abc_fclose  (dbpd);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------
	| Validate Letter Code |
	----------------------*/
	if (LCHECK ("lett_code"))
	{
		if (SRCH_KEY)
		{
			SrchDbph (temp_str);
			return (EXIT_SUCCESS);
		}

    	strcpy (dbph_rec.co_no, comm_rec.co_no);
    	sprintf (dbph_rec.letter_code, "S-%-8.8s", local_rec.lett_code);
    	if (!find_rec (dbph, &dbph_rec, COMPARISON, "r"))
    	{
    		strcpy (local_rec.lett_desc, dbph_rec.letter_desc);
			DSP_FLD ("lett_desc");

			if (LoadDbpd (dbph_rec.dbph_hash))
				return (EXIT_FAILURE);  

			entry_exit	= TRUE;
			newNotes	= FALSE;
    	}
		else
			newNotes	= TRUE;

		return (EXIT_SUCCESS);
	}
	
	/*------------------------------
	| Validate Letter Description. |
	------------------------------*/
	if (LCHECK ("lett_desc"))
	{
		if (dflt_used && prog_status != ENTRY)
		{
			strcpy (local_rec.lett_desc, dbph_rec.letter_desc);
			DSP_FLD ("lett_desc");
			return (EXIT_SUCCESS);
		}
		else
		{
			if (!strcmp (local_rec.lett_desc, "                                        "))
				return (EXIT_FAILURE);
		}
		DSP_FLD ("lett_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Paragraph input    |
	-----------------------------*/
	if (LCHECK ("par_code"))
	{
		if (SRCH_KEY)
		{
			SrchDblh (temp_str);
			return (EXIT_SUCCESS);
		}
	
		if (last_char == DELLINE)
			return (DeleteLine ());

		if (last_char == INSLINE)
			return (InsertLine ());

		/*-------------------------------
		| First character is a '\'	    |
		| \D	- delete current line	|
		| \I	- insert before current	|
		-------------------------------*/
		if (local_rec.par_code [0] == 92)
		{
			if (local_rec.par_code [1] == 'D')
				return (DeleteLine ());

			if (local_rec.par_code [1] == 'I')
				return (InsertLine ());
		}

		strcpy (dblh_rec.co_no, comm_rec.co_no);
		sprintf (dblh_rec.par_code, "S-%-8.8s", local_rec.par_code);
		cc = find_rec (dblh, &dblh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess047));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		_dblh_hash [line_cnt] = dblh_rec.dblh_hash;
		strcpy (_par_desc [line_cnt], dblh_rec.par_desc);

		if (newNotes && prog_status == ENTRY)
			tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============
| Delete line. |
==============*/
int
DeleteLine (
 void)
{
	int	i;
	int	this_page;

	this_page = line_cnt / TABLINES;
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
	   	putval (line_cnt);

		_dblh_hash [line_cnt] = _dblh_hash [line_cnt + 1];
		strcpy (_par_desc [line_cnt], _par_desc [line_cnt + 1]);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	_dblh_hash [line_cnt]  = 0L;
	sprintf (_par_desc [line_cnt], "%-40.40s"," ");
	sprintf (local_rec.par_code, "%-8.8s"," ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*==============
| Insert line. |
==============*/
int
InsertLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
		    
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("par_code")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	for (i = line_cnt,line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		_dblh_hash [line_cnt] = _dblh_hash [line_cnt - 1];
		strcpy (_par_desc [line_cnt], _par_desc [line_cnt - 1]);
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
		 	line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	sprintf (_par_desc [line_cnt], "%-40.40s", " ");
	_dblh_hash [line_cnt]  = 0L;
	sprintf (local_rec.par_code, "%-8.8s"," ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	if (prog_status == ENTRY)
		prog_status = !prog_status;
	init_ok = 1;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
LoadDbpd (
 long shash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;	

	abc_selfield (dblh, "dblh_dblh_hash"); 

	dbpd_rec.dbph_hash = shash;
	dbpd_rec.line_no = 0;  

	cc = find_rec (dbpd, &dbpd_rec, GTEQ, "r");
	while (!cc && dbpd_rec.dbph_hash == shash)
	{
		fflush (stdout);

		/*---------------------
		| Get paragraph code. |
		---------------------*/
		cc = find_hash (dblh, &dblh_rec, COMPARISON, "r", dbpd_rec.dblh_hash);
		if (!cc)
		{
			sprintf (local_rec.par_code, "%-8.8s", dblh_rec.par_code + 2);
			_dblh_hash [lcount [2]] = dblh_rec.dblh_hash;
			strcpy (_par_desc [lcount [2]], dblh_rec.par_desc);
			putval (lcount [2]++);
		}
		cc = find_rec (dbpd, &dbpd_rec, NEXT, "r");
	}
	abc_selfield (dblh, "dblh_id_no"); 
	scn_set (1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	/*---------------------
	| No exit - return 0. |
	---------------------*/
	return (EXIT_SUCCESS); 
}

/*===================
| Update all files. |
===================*/
int
UpdateData (
 void)
{
	int add_item = FALSE;
	int	wk_line;
	int	i;

	clear ();
	fflush (stdout);
		
	/*---------------------------
   	| Add new paragraph header. |
   	---------------------------*/
	if (newNotes == 1) 
	{
		rv_pr (ML (mlStdMess035), 2, 0, 1);
		strcpy (dbph_rec.co_no,    comm_rec.co_no);
		sprintf (dbph_rec.letter_code, "S-%-8.8s", local_rec.lett_code);
		strcpy (dbph_rec.letter_desc, local_rec.lett_desc);
		dbph_rec.dbph_hash = 0L;
		cc = abc_add (dbph, &dbph_rec);
		if (cc) 
			sys_err ("Error in dbph During (DBADD)", cc, PNAME);

		cc = find_rec (dbph, &dbph_rec, COMPARISON, "r");
		if (cc)
			return (EXIT_FAILURE);
	}
	else
	{
		rv_pr (ML (mlStdMess035), 2, 2, 1);
		strcpy (dbph_rec.letter_desc, local_rec.lett_desc);
		cc = abc_update (dbph, &dbph_rec);
		if (cc) 
			sys_err ("Error in dbph During (DBUPDATE)", cc, PNAME);
	}

	/*------------------------------
   	| Process all paragraph lines. |
   	------------------------------*/
	scn_set (2);
	for (wk_line = 0; wk_line < lcount [2]; wk_line++) 
	{
	   	getval (wk_line);
	   	dbpd_rec.dbph_hash = dbph_rec.dbph_hash;
	   	dbpd_rec.line_no   = wk_line;
	   	cc = find_rec (dbpd, &dbpd_rec, COMPARISON, "u");
	   	if (cc)
	   		add_item = TRUE;
	   	else
	   		add_item = FALSE;
		 	
		strcpy (dblh_rec.co_no, comm_rec.co_no);
		sprintf (dblh_rec.par_code, "S-%-8.8s", local_rec.par_code);
		cc = find_rec (dblh, &dblh_rec, COMPARISON, "r");
		if (!cc && !strcmp (dblh_rec.co_no, comm_rec.co_no) &&
					!strncmp (dblh_rec.par_code + 2, local_rec.par_code,8))
			dbpd_rec.dblh_hash = dblh_rec.dblh_hash;

	   	if (add_item)
	   	{
	   		putchar ('A'); 
	   		fflush (stdout);

			cc = abc_add (dbpd, &dbpd_rec);
			if (cc) 
	      		sys_err ("Error in dbpd During (DBUPDATE)", cc, PNAME);
	   	}
	   	else
	   	{
	    	/*-------------------------
	    	| Update existing detail. |
	    	-------------------------*/
	    	cc = abc_update (dbpd, &dbpd_rec);
	    	if (cc) 
	     	  	sys_err ("Error in dbpd During (DBUPDATE)", cc, PNAME);
	   	}
		abc_unlock (dbpd);
	}

	i = wk_line;
    for (wk_line = i; wk_line < MAXLINES; wk_line++)
    {
	   	dbpd_rec.dbph_hash = dbph_rec.dbph_hash;
	   	dbpd_rec.line_no   = wk_line;
	   	cc = find_rec (dbpd, &dbpd_rec, COMPARISON, "r");
	   	if (!cc)
		{
	   		putchar ('D'); 
	   		fflush (stdout);
			abc_delete (dbpd);
		}
	   	else
		 	break;
	}

	/*-----------------------------------
	| Update existing paragraph header. |
	-----------------------------------*/
	if (newNotes == 0) 
	{	
	   	/*------------------------
	   	| Delete cancelled file. |
	   	------------------------*/
	   	if (lcount [2] == 0) 
	   	{
			rv_pr (ML (mlCrMess132) ,2,2,1);
			abc_unlock (dbph);
			cc = abc_delete (dbph);
			if (cc)
				sys_err ("Error in dbph During (DBDELETE)",cc,PNAME);
	   	}
	   	abc_unlock (dbph);
	    
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Search routine for Paragraph file. |
===================================*/
void
SrchDblh (
 char *	key_val)
{
	work_open ();
	save_rec ("# Code.  ","#Paragraph Description");
	strcpy (dblh_rec.co_no, comm_rec.co_no);
	sprintf (dblh_rec.par_code, "S-%-8.8s", key_val);
	cc = find_rec (dblh, &dblh_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (dblh_rec.co_no, comm_rec.co_no) && 
		   !strncmp (dblh_rec.par_code + 2,key_val,strlen (key_val)))
	{
		if (!strncmp (dblh_rec.par_code, "S-", 2))
		{
			cc = save_rec (dblh_rec.par_code + 2, dblh_rec.par_desc);
			if (cc)
				break;
		}
		cc = find_rec (dblh, &dblh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (dblh_rec.co_no, comm_rec.co_no);
	sprintf (dblh_rec.par_code, "S-%-8.8s", temp_str);
	cc = find_rec (dblh, &dblh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dblh, "DBFIND");
}

/*=================================
| Search routine for Letter file. |
=================================*/
void
SrchDbph (
 char *	key_val)
{
	work_open ();
	save_rec ("#  Code. ","#Letter Description");
	strcpy (dbph_rec.co_no, comm_rec.co_no);
	sprintf (dbph_rec.letter_code, "S-%-8.8s", key_val);
	cc = find_rec (dbph, &dbph_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (dbph_rec.co_no, comm_rec.co_no) && 
		   !strncmp (dbph_rec.letter_code + 2,key_val,strlen (key_val)))
	{
		if (!strncmp (dbph_rec.letter_code, "S-", 2))
		{
			cc = save_rec (dbph_rec.letter_code + 2, dbph_rec.letter_desc);
			if (cc)
				break;
		}
		cc = find_rec (dbph, &dbph_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (dbph_rec.co_no, comm_rec.co_no);
	sprintf (dbph_rec.letter_code, "S-%-8.8s", temp_str);
	cc = find_rec (dbph, &dbph_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dbph, "DBFIND");
}

/*============================================
| Display Info for lines while in edit mode. |
============================================*/
void
tab_other (
 int iline)
{
	char	disp_str  [200];
	
	if (cur_screen != 2)
		return;

	if (_dblh_hash [iline] == 0L && blank_win)
		return;

	crsr_off ();

	if (!blank_win)
		Dsp_close ();

	Dsp_open (22,5,12); 
	Dsp_saverec ("                               P A R A G R A P H    D E S C R I P T I O N .                                 ");
	Dsp_saverec ("");
	Dsp_saverec ("");

	if (_dblh_hash [iline] == 0L)
	{
		Dsp_srch ();
		Dsp_close ();
		blank_win = TRUE;
		return;
	}

	sprintf (disp_str,"Paragraph Description : %-40.40s",_par_desc [iline]);
	Dsp_saverec (disp_str);

	dbld_rec.dblh_hash = _dblh_hash [iline];
	dbld_rec.line_no = 0;
	cc = find_rec (dbld, &dbld_rec, GTEQ, "r");
	while (!cc && dbld_rec.dblh_hash == _dblh_hash [iline])
	{
		sprintf (disp_str,"%-108.108s", dbld_rec.desc);
		Dsp_saverec (disp_str);
		cc = find_rec (dbld, &dbld_rec, NEXT, "r");
	}
	Dsp_srch ();
	if (prog_status == ENTRY)
		crsr_on ();

	blank_win = FALSE;

	return;
}

/*========================
| Print Company Details. |
========================*/
void
PrintCompany (
 void)
{
	move (0,20);
	line (132);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0, err_str, comm_rec.co_no,comm_rec.co_name);
	move (0,22);
	line (132);
}

/*================
| Print Heading. |
================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		swide ();

		centre_at (0, 132,ML (mlCrMess133));

		pr_box_lines (scn); 
		move (0, 1);
		line (130);
		box (0, 2, 130, 1);

		move (1, input_row);
		PrintCompany (); 
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;

		if (scn == 1) 
		{
			if (clr_scn2)
				init_vars (2);	

			if (clearOk)
			{
				heading (2); 
				scn_set (2);
				scn_display (2);
				tab_other (line_cnt); 
			}
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}	
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


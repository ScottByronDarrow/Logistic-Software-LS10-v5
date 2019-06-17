/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lett_mnt.c,v 5.4 2002/07/18 07:17:21 scott Exp $
|  Program Name  : (tm_lett_mnt.c)
|  Program Desc  : (Telemarketing Letters Maintenance)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 17/07/91         |
|---------------------------------------------------------------------|
| $Log: lett_mnt.c,v $
| Revision 5.4  2002/07/18 07:17:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/11/14 07:13:31  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lett_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_lett_mnt/lett_mnt.c,v 5.4 2002/07/18 07:17:21 scott Exp $";

#define MAXSCNS 	3
#define MAXLINES	500

#define	X_OFF		0
#define	Y_OFF		0

#define	TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>
#include <tabdisp.h>
#include <hot_keys.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	newNotes = 0;

#include	"schema"

struct commRecord	comm_rec;
struct tmlhRecord	tmlh_rec;
struct tmlnRecord	tmln_rec;
struct tmcfRecord	tmcf_rec;

char	*dot_desc [] = {
	"Current Date, Format dd/mm/yy",
	"Current Date, Format 1st January 1990 ",
	"Company Name.",
	"Company Address part one.",
	"Company Address part two.",
	"Company Address part three.",
	"Prospect Number",
	"Prospect Acronym",
	"Prospect Name",
	"Prospect Address Part 1",
	"Prospect Address Part 2",
	"Prospect Address Part 3",
	"Prospect Contact name.",
	"Prospect Alternate Contact name.",
	"Salesman number.",
	"Salesman name.",
	"Area number.",
	"Area name.",
	"Prospect Post Code.",
	"Prospect Business Sector.",
	"Prospect Phone Number.",
	"Prospect Fax Number.",
	"Next Phone Date.",
	"Next Phone Time.",
	"Next Visit Date.",
	"Next Visit Time.",
	"Visit Frequency.",
	"Current Operator.",
	"Last Operator.",
	"Call Back Date.",
	"Call Back Time.",
	"Last Phone Date.",
	"Prospect Origin.",
	"Best Phone Time.",
	"User Defined Number One.",
	"User Defined Number Two.",
	"User Defined Number Three.",
	"User Defined Number Four.",
	"User Defined Number Five.",
	"User Defined Number Six.",
	"User Defined Number Seven.",
	"User Defined Number Eight.",
	"User Defined Number Nine.",
	"User Defined Number Ten.",
	"User Defined Number Eleven.",
	"User Defined Number Twelve.",
	""
};
#include	<tm_commands.h>

int	clr_scn3 = FALSE;
int	clear_ok;
int	first_time = TRUE;

extern	int		TruePosition;
/*===========================
| Local & Screen Structures |
===========================*/
struct {			 	 	/*---------------------------------------*/
	char	dummy [11];	 	/*| Dummy Used In Screen Generator.  	|*/
	char	comments [79];  /*| Holds Comments for each line.       |*/
	char	let_code [11];  /*| Holds Letter Number                 |*/
	char	let_desc [41];  /*| Holds Letter Description            |*/
	char	code_from [11]; /*| Holds Letter Number to be copied    |*/
				 			/*|_____________________________________|*/
} local_rec;            

static	struct	var	vars []	={	

	{1, LIN, "campaign_no", 3, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number  ", " Enter Campaign No. Default For Global Letter. ", 
		NO, NO, JUSTLEFT, "", "", (char *)&tmcf_rec.campaign_no}, 
	{2, LIN, "code", 3, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Letter Code      ", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.let_code}, 
	{2, LIN, "code_from", 3, 40, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ","Copy from Letter ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.code_from}, 
	{2, LIN, "desc", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", tmlh_rec.let_desc, "Description      ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.let_desc}, 
	{3, TXT, "comm", 8, 0, 0, 
		"", "          ", 
		" ", " ", ".....T.........T.........T.........T.........T.........T.........T........", "", 
		10, 78, MAXLINES, "", "", local_rec.comments}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	LoadDotCommands 	(void);
void 	ShowDotCommands 	(void);
void 	SrchTmcf 			(char *);
void 	SrchTmlh 			(char *);
void 	PrintCo			 	(void);
int 	heading 			(int scn);
int 	Update 				(void);
int 	ReadTmln 			(long);
int 	spec_valid 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		clear_ok 	= TRUE;
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	
		init_vars (2);	
		init_vars (3);	
		lcount [3] = 0;

		heading (1);

		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		clr_scn3 = TRUE;
		heading (2);
		clr_scn3 = FALSE;

		scn_display (2);
		entry (2);
		if (prog_exit || restart)
			continue;

		scn_write (2);
		scn_display (2);
		scn_display (3);

		if (newNotes == 1 && lcount [3] == 0)
			entry (3);
		else
			edit (3);

		if (restart)
			continue;
	
		no_edit (1);
		edit_all ();

		if (restart)
			continue;

		Update ();

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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmln, tmln_list, TMLN_NO_FIELDS, "tmln_id_no");
	open_rec (tmlh, tmlh_list, TMLH_NO_FIELDS, "tmlh_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose (tmln);
	abc_fclose (tmlh);
	abc_fclose (tmcf);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campaign_no"))
	{
		if (dflt_used)
		{
			tmcf_rec.campaign_no = 0;
			tmcf_rec.hhcf_hash = 0L;
			sprintf (tmcf_rec.c_name1, "%-40.40s", " Global Letter Covering Any Campaign ");
			sprintf (tmcf_rec.c_name2, "%-40.40s", " ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
			scn_display (1);

		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Letter Code  |
	-----------------------*/
	if (LCHECK ("code"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmlh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmlh_rec.co_no, comm_rec.co_no);
		tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		sprintf (tmlh_rec.let_code, "%-10.10s", local_rec.let_code);
		if (!find_rec (tmlh, &tmlh_rec, COMPARISON, "r"))
		{
			sprintf (local_rec.let_desc, "%-40.40s", tmlh_rec.let_desc);
			DSP_FLD ("desc");
			FLD ("code_from") = NA;
			if (ReadTmln (tmlh_rec.hhlh_hash))
				return (EXIT_FAILURE);

			entry_exit	= TRUE;
			newNotes	= FALSE;
		}
		else
		{
			FLD ("code_from") = YES;
			newNotes = TRUE;
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
			SrchTmlh (temp_str);
			return (EXIT_SUCCESS);
		}
	
		if (!strcmp (local_rec.code_from,"          "))
			return (EXIT_SUCCESS);
	
		strcpy (tmlh_rec.co_no, comm_rec.co_no);
		strcpy (tmlh_rec.let_code, local_rec.code_from);
		if (!find_rec (tmlh, &tmlh_rec, COMPARISON, "r"))
		{
			strcpy (local_rec.let_desc, tmlh_rec.let_desc);
			DSP_FLD ("desc");
			entry_exit = TRUE;
			FLD ("code_from")	= NE;
			if (ReadTmln (tmlh_rec.hhlh_hash))
				return (EXIT_FAILURE);
		}
		else
		{
			errmess (ML (mlStdMess109));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Comment Line. |
	------------------------*/
	if (LCHECK (comm))
	{
		if (SRCH_KEY)
		{
			ShowDotCommands ();

			clear_ok = FALSE;
			heading (2);
			scn_display (2);
			clear_ok = TRUE;
			crsr_on ();
			return (EXIT_SUCCESS);
		}
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Load dot commands and descriptions  |
-------------------------------------*/
void
LoadDotCommands (void)
{
	int	i;
	char	disp_str [200];
	
	tab_open ("dot_tab", (KEY_TAB *)0, 3, 0, 4, FALSE);
	tab_add ("dot_tab", 
		"#                    Dot Commands                    ");

	for (i = 0; i < N_CMDS; i++)
	{
		sprintf (disp_str," .%-7.7s  %-40.40s ", dot_cmds [i], dot_desc [i]);

		tab_add ("dot_tab", disp_str);
	}

	return;
}

/*-------------------------------------
| Show dot commands and descriptions  |
-------------------------------------*/
void
ShowDotCommands (void)
{
	if (first_time)
	{
		LoadDotCommands ();
		first_time = FALSE;
	}

	tab_display ("dot_tab", TRUE);
	tab_scan ("dot_tab");

	tab_clear ("dot_tab");

	return;
}

/*==========================
| Read Letter Detail Lines |
==========================*/
int
ReadTmln (
	long	hhlhHash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (3);
	lcount [3] = 0;

	tmln_rec.hhlh_hash 	= hhlhHash;
	tmln_rec.line_no 	= 0; 

	cc = find_rec (tmln, &tmln_rec, GTEQ, "r");
	while (!cc && tmln_rec.hhlh_hash ==hhlhHash)
	{
		strcpy (local_rec.comments, tmln_rec.desc);
		putval (lcount [3]++);
		cc = find_rec (tmln, &tmln_rec, NEXT, "r");
	}
	scn_set (2);

	return (EXIT_SUCCESS);
}

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmcf (
	char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.","#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%04d", tmcf_rec.campaign_no);
		cc = save_rec (err_str, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmcf, "DBFIND");
}

/*=============================================
| Search routine for Letter master file. |
=============================================*/
void
SrchTmlh (
	char *key_val)
{
	_work_open (10,0,40);
	save_rec ("#Code","#Description");
	strcpy (tmlh_rec.co_no,comm_rec.co_no);
	tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	sprintf (tmlh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tmlh, &tmlh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp (tmlh_rec.co_no,comm_rec.co_no)     &&
		  tmlh_rec.hhcf_hash == tmcf_rec.hhcf_hash &&
	          !strncmp (tmlh_rec.let_code,key_val,strlen (key_val)))
	{
		cc = save_rec (tmlh_rec.let_code,tmlh_rec.let_desc);
		if (cc)
			break;

		cc = find_rec (tmlh, &tmlh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmlh_rec.co_no,comm_rec.co_no);
	tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	sprintf (tmlh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tmlh, &tmlh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmlh, "DBFIND");

	sprintf (local_rec.let_desc,"%-40.40s",tmlh_rec.let_desc);
}

/*===================
| Update all files. |
===================*/
int
Update (void)
{
	int 	add_item = FALSE;
	int		lineNo;

	clear ();
		
	if (newNotes == TRUE) 
	{
		rv_pr (ML (mlStdMess035), 2,0,1);
		strcpy (tmlh_rec.co_no, comm_rec.co_no);
		tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		strcpy (tmlh_rec.let_code, local_rec.let_code);
		strcpy (tmlh_rec.let_desc, local_rec.let_desc);
		tmlh_rec.hhlh_hash = 0L;
		cc = abc_add (tmlh,&tmlh_rec);
		if (cc) 
			file_err (cc, tmlh, "DBADD");

		cc = find_rec (tmlh, &tmlh_rec,COMPARISON, "r");
		if (cc)
			return (EXIT_FAILURE);

	}
	else
	{
		rv_pr (ML (mlStdMess035), 2,2,1);
		strcpy (tmlh_rec.let_desc, local_rec.let_desc);
		cc = abc_update (tmlh,&tmlh_rec);
		if (cc) 
			file_err (cc, tmlh, "DBUPDATE");
	}
	
	scn_set (3);
	for (lineNo = 0;lineNo < lcount [3];lineNo++) 
	{
		getval (lineNo);

		tmln_rec.hhlh_hash 	= tmlh_rec.hhlh_hash;
		tmln_rec.line_no 	= lineNo;
		cc = find_rec (tmln, &tmln_rec, COMPARISON, "u");
		if (cc)
		   	add_item = TRUE;
		else
		   	add_item = FALSE;

		strcpy (tmln_rec.desc, local_rec.comments);
		if (add_item)
		{
			cc = abc_add (tmln,&tmln_rec);
			if (cc) 
				file_err (cc, tmln, "DBADD");
		}
		else
		{
			/*------------------------
			| Update existing order. |
			------------------------*/
			cc = abc_update (tmln,&tmln_rec);
			if (cc) 
				file_err (cc, tmln, "DBUPDATE");
		}
		abc_unlock (tmln);
	}

	for (lineNo = lcount [3];lineNo < MAXLINES; lineNo++)
	{
		tmln_rec.hhlh_hash = tmlh_rec.hhlh_hash;
		tmln_rec.line_no = lineNo;
		cc = find_rec (tmln, &tmln_rec, COMPARISON, "r");
		if (!cc)
		{
			putchar ('D');
			fflush (stdout);
			abc_delete (tmln);
		}
		else
		 	break;
	}

	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (newNotes == 0) 
	{	
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (lcount [3] == 0) 
		{
			rv_pr (ML (mlStdMess014),2,2,1);
			abc_unlock (tmlh);
			cc = abc_delete (tmlh);
			if (cc)
				file_err (cc, tmlh, "DBDELETE");
		}
		abc_unlock (tmlh);
	    
	}
	return (EXIT_SUCCESS);
}

/*========================
| Print Company Details. |
========================*/
void
PrintCo (void)
{
	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	line_at (22,0,80);
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

		if (clear_ok)
			clear ();
		
		rv_pr (ML (mlTmMess007),24,0,1);
		
		line_at (1,0,80);

		if (scn == 1)
			box (0,2,80,1);

		if (scn == 2)
		{
			box (0,2,80,2);

			if (clr_scn3)
				init_vars (3);	

			if (clear_ok)
			{
				scn_set (3);
				scn_display (3);
			}

			if (tmcf_rec.campaign_no == 0)
			{
				sprintf (err_str, ML (mlTmMess008),
					tmcf_rec.c_name1, tmcf_rec.c_name2);
			}
			else
			{
				sprintf (err_str, ML (mlTmMess037),
					tmcf_rec.campaign_no,
					tmcf_rec.c_name1, tmcf_rec.c_name2);
			}
	

			rv_pr (err_str,5,2,1);
		}

		if (scn == 3)
		{
			scn_set (2);
			scn_write (2);
			scn_display (2);
		}

		scn_set (scn);

		PrintCo ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

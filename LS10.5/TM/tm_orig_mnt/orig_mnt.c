/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: orig_mnt.c,v 5.4 2002/07/25 11:17:38 scott Exp $
|  Program Name  : (tm_orig_mnt.c)
|  Program Desc  : (Maintain Telemarketing Origin File)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 26/07/91         |
|---------------------------------------------------------------------|
| $Log: orig_mnt.c,v $
| Revision 5.4  2002/07/25 11:17:38  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2001/11/14 06:07:15  scott
| Updated to convert to app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: orig_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_orig_mnt/orig_mnt.c,v 5.4 2002/07/25 11:17:38 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

#define	POS_FILE	 (local_rec.type [0] == 'P' || local_rec.type [0] == 'p')

	/*
	 * Special fields and flags.
	 */
   	int	newCode = 0;

	extern	int		TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct tmofRecord	tmof_rec;
struct tmpfRecord	tmpf_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	type [2];
	char	prev_code [4];
	int	prev_no;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "orig", 4, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Origin Code           ", "", 
		NE, NO, JUSTRIGHT, "", "", tmof_rec.o_code}, 
	{1, LIN, "orig_desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Origin Description    ", " ", 
		YES, NO, JUSTLEFT, "", "", tmof_rec.o_desc}, 
	{2, LIN, "pos", 4, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Position Code         ", " ", 
		NE, NO, JUSTRIGHT, "", "", tmpf_rec.pos_code}, 
	{2, LIN, "pos_desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Position Description  ", " ", 
		YES, NO, JUSTLEFT, "", "", tmpf_rec.pos_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int 	spec_valid 		 (int);
void 	Update 			 (void);
void 	SrchTmof 		 (char *);
void 	SrchTmpf 		 (char *);
int 	heading 		 (int);

int
main (
	int argc,
	char *argv [])
{
	if (argc != 2)
	{
		print_at (0,0,mlTmMess702, argv [0]);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;
	
	sprintf (local_rec.type, "%-1.1s", argv [1]);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	if (POS_FILE)
	{
		vars [label ("orig")].scn 		= 2;
		vars [label ("orig_desc")].scn 	= 2;
		vars [label ("pos")].scn 		= 1;
		vars [label ("pos_desc")].scn 	= 1;
	}

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	if (POS_FILE)
		open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");
	else
		open_rec (tmof, tmof_list, TMOF_NO_FIELDS, "tmof_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	if (POS_FILE)
		abc_fclose (tmpf);
	else
		abc_fclose (tmof);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	if (LCHECK ("orig"))
	{
		if (SRCH_KEY)
		{
			SrchTmof (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmof_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmof, &tmof_rec, COMPARISON, "u");
		if (cc) 
			newCode = 1;
		else    
		{
			newCode = 0;
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pos"))
	{
		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmpf_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "u");
		if (cc) 
			newCode = 1;
		else    
		{
			newCode = 0;
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (
	void)
{
	if (POS_FILE)
	{
		/*=============================
		| Add or update tmpf record . |
		=============================*/
		strcpy (tmpf_rec.co_no,comm_rec.co_no);
		strcpy (tmpf_rec.stat_flag,"0");
		if (newCode)
		{
			cc = abc_add (tmpf,&tmpf_rec);
			if (cc) 
				file_err (cc, tmpf, "DBADD");
		}
		else
		{
			cc = abc_update (tmpf,&tmpf_rec);
			if (cc) 
				file_err (cc, tmpf, "DBUPDATE");
		}
		abc_unlock (tmpf);
		sprintf (local_rec.prev_code, "%-3.3s", tmpf_rec.pos_code);
	}
	else
	{
		/*=============================
		| Add or update tmof record . |
		=============================*/
		strcpy (tmof_rec.co_no,comm_rec.co_no);
		strcpy (tmof_rec.stat_flag,"0");
		if (newCode)
		{
			cc = abc_add (tmof,&tmof_rec);
			if (cc) 
				file_err (cc, tmof, "DBADD");
		}
		else
		{
			cc = abc_update (tmof,&tmof_rec);
			if (cc) 
				file_err (cc, tmof, "DBUPDATE");
		}
		abc_unlock (tmof);
		sprintf (local_rec.prev_code, "%-3.3s", tmof_rec.o_code);
	}
}

void
SrchTmof (
	char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No.","#Origin Description");
	strcpy (tmof_rec.co_no,comm_rec.co_no);
	sprintf (tmof_rec.o_code,"%-3.3s",key_val);
	cc = find_rec (tmof,&tmof_rec,GTEQ,"r");

	while (!cc && !strcmp (tmof_rec.co_no,comm_rec.co_no) &&
		      !strncmp (tmof_rec.o_code,key_val,strlen (key_val)))
	{
		cc = save_rec (tmof_rec.o_code,tmof_rec.o_desc);
		if (cc)
			break;

		cc = find_rec (tmof,&tmof_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmof_rec.co_no,comm_rec.co_no);
	sprintf (tmof_rec.o_code,"%-3.3s",temp_str);
	cc = find_rec (tmof,&tmof_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, tmof, "DBFIND");
}

void
SrchTmpf (
	char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.","#Position Description");
	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	sprintf (tmpf_rec.pos_code,"%-3.3s",key_val);
	cc = find_rec (tmpf,&tmpf_rec,GTEQ,"r");

	while (!cc && !strcmp (tmpf_rec.co_no,comm_rec.co_no) &&
		      !strncmp (tmpf_rec.pos_code,key_val,strlen (key_val)))
	{
		cc = save_rec (tmpf_rec.pos_code,tmpf_rec.pos_desc);
		if (cc)
			break;

		cc = find_rec (tmpf,&tmpf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	sprintf (tmpf_rec.pos_code,"%-3.3s",temp_str);
	cc = find_rec (tmpf,&tmpf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, tmpf, "DBFIND");
}

int
heading (
	int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		if (POS_FILE)
			rv_pr (ML (mlTmMess032) , 25,0,1);
		else
			rv_pr (ML (mlTmMess033) , 25,0,1);

		/*sprintf (err_str, 
			"Last %s Code: %s", 
			 (POS_FILE) ? "Position" : "Origin", local_rec.prev_code);*/

		if (POS_FILE)	
			sprintf (err_str, ML (mlTmMess034), local_rec.prev_code);
		else
			sprintf (err_str, ML (mlTmMess035), local_rec.prev_code);

		rv_pr (err_str, 50,0,0);

		move (0,1);
		line (80);

		box (0,3,80,2);

		move (0,20);
		line (80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		move (0,22);
		line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


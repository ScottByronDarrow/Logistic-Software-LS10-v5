/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: srmaint.c,v 5.2 2001/08/09 09:14:27 scott Exp $
|  Program Name  : (essrmaint.c  )                                 |
|  Program Desc  : (Maintain Branch Security Record.           )   |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth.| Date Written  : 10/05/86        |
|---------------------------------------------------------------------|
| $Log: srmaint.c,v $
| Revision 5.2  2001/08/09 09:14:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:49  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 00:23:18  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: srmaint.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/srmaint/srmaint.c,v 5.2 2001/08/09 09:14:27 scott Exp $";

#define TOTSCNS		1
#include <pslscr.h>

extern	int	TruePosition;
extern	int	EnvScreenOK;

#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_ol_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int	newCode = 0;

#include	"schema"

struct commRecord	comm_rec;
struct essrRecord	essr_rec;
struct essrRecord	pass_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
char	dummy [16];

static	struct	var	vars []	={	

	{1, LIN, "name", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Operator/Program ID     ", " ", 
		NE, NO, JUSTLEFT, "", "", essr_rec.op_id}, 
	{1, LIN, "desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Operator name           ", " ", 
		NO, NO, JUSTLEFT, "", "", essr_rec.op_name}, 
	{1, LIN, "passwd", 7, 2, CHARTYPE, 
		"________", "          ", 
		" ", "", "Password                ", " ", 
		NO, NO, JUSTLEFT, "", "", essr_rec.op_passwd}, 
	{1, LIN, "short", 8, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Short ID                ", " ", 
		NO, NO, JUSTRIGHT, "", "", essr_rec.short_id}, 
	{1, LIN, "level", 9, 2, INTTYPE, 
		"NN", "          ", 
		" ", "", "Security Level          ", " ", 
		NO, NO, JUSTLEFT, "", "", (char *) &essr_rec.sec_level}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

extern	int			EnvScreenOK;

MENUTAB upd_menu [] = {
			{ " 1. UPDATE SECURITY RECORD    " },
			{ " 2. DELETE SECURITY RECORD    " },
			{ " 3. ABANDON SECURITY RECORD   " },
			{ ENDMENU }
		};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
int 	update 			(void);
void 	UpdateMenu 		(void);
void 	UpdateEssr 		(void);
void 	DeleteEssr 		(void);
void 	SrchEssr 		(char *);
int 	heading 		(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int argc,
	char *argv [])
{
	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	SETUP_SCR 	(vars);
	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB ();

	strcpy (essr_rec.co_no,comm_rec.co_no);
	strcpy (essr_rec.est_no,comm_rec.est_no);

	strcpy (pass_rec.co_no,comm_rec.co_no);
	strcpy (pass_rec.est_no,comm_rec.est_no);

	while (!prog_exit)
	{
		entry_exit	= 	FALSE;
		edit_exit 	= 	FALSE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		search_ok 	= 	TRUE;
		newCode 	= 	FALSE;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit)
			break;
				
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
	void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias ("pessr", essr);
	open_rec (essr, essr_list, ESSR_NO_FIELDS, "essr_id_no");
	open_rec ("pessr", essr_list, ESSR_NO_FIELDS, "essr_pass_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
	void)
{
	abc_fclose (essr);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*---------------------
	| validate Tax Code . |
	---------------------*/
	if (LCHECK ("name"))
	{
		if (SRCH_KEY)
		{
			SrchEssr (temp_str);
			return (EXIT_SUCCESS);
		}

		newCode = 0;
		if ((cc = find_rec (essr, &essr_rec, COMPARISON, "w")))
			newCode = 1;
		else    
			entry_exit = 1;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("passwd"))
	{
		upshift (essr_rec.op_passwd);
		return (EXIT_SUCCESS);
	}
			
	return (EXIT_SUCCESS);
}

int
update (
	void)
{
	int invalid = 0;

	/*--------------------------------------------------
	| Add or update product group description record . |
	--------------------------------------------------*/
	if (newCode == 1)
	{
		if ((cc = abc_add (essr,&essr_rec)))
			file_err (cc, essr, "DBADD");
	}
	else
		UpdateMenu ();

        abc_unlock (essr);

	return (invalid);
}

void
UpdateMenu (
	void)
{
	mmenu_print (" SECURITY MAINTENANCE OPTIONS ", upd_menu, 0);

	for (;;)
	{
		switch (mmenu_select (upd_menu))
		{
			case 0 :
				UpdateEssr ();
				return ;

			case 1 :
				DeleteEssr ();
				return ;

			case 2 :
			case -1 :
				return ;
	
			default :
				break;
		}
	}
}

void
UpdateEssr (
	void)
{
	if ((cc = abc_update (essr,&essr_rec)))
		file_err (cc, essr, "DBUPDATE");
}

void
DeleteEssr (
	void)
{
	if ((cc = abc_delete (essr)))
		file_err (cc, essr, "DBDELETE");
}

void
SrchEssr (
	char *key_val)
{
	work_open ();
	save_rec ("#Name","#Description");
	strcpy (essr_rec.co_no,comm_rec.co_no);
	strcpy (essr_rec.est_no,comm_rec.est_no);
	sprintf (essr_rec.op_id,"%-8.8s",key_val);

	cc = find_rec (essr,&essr_rec,GTEQ,"r");

	while (!cc && !strcmp (essr_rec.co_no,comm_rec.co_no) &&
			!strcmp (essr_rec.est_no,comm_rec.est_no) &&
		!strncmp (essr_rec.op_id, key_val, strlen (key_val)))
	{
		if (! (cc = save_rec (essr_rec.op_id,essr_rec.op_name)))
			cc = find_rec (essr,&essr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (essr_rec.co_no,comm_rec.co_no);
	strcpy (essr_rec.est_no,comm_rec.est_no);
	sprintf (essr_rec.op_id,"%-14.14s",temp_str);

	if ((cc = find_rec (essr,&essr_rec,COMPARISON,"r")))
		file_err (cc, essr, "DBFIND");
}

int
heading (
	int scn)
{
	int	s_size = 80;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlOlMess021), 20,0,1);
		line_at (1,0, s_size);

		move (1,input_row);
		if (scn == 1)
		{
			line_at (6,1, s_size);
			box (0,3,80,6);
		}
		line_at (20,0, s_size);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


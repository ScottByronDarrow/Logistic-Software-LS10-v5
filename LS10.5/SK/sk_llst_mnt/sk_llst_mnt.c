/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_llst_mnt.c,v 5.3 2002/07/25 11:17:36 scott Exp $
|  Program Name  : (sk_llst_mnt.c )                                   |
|  Program Desc  : (Location Status Maintenance.                )     |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : (9th Oct 2000)   |
|---------------------------------------------------------------------|
| $Log: sk_llst_mnt.c,v $
| Revision 5.3  2002/07/25 11:17:36  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:18:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:11  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:14  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.2  2000/12/20 05:24:47  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 1.1  2000/11/22 00:25:43  scott
| Added new program
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_llst_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_llst_mnt/sk_llst_mnt.c,v 5.3 2002/07/25 11:17:36 scott Exp $";

#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct llstRecord	llst_rec;

	char	*data = "data";

   	int		newCode	= FALSE;
	extern	int		TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	previousCode [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "locationStatusCode",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Location status code : ", "Enter Location status code.",
		 NE, NO, JUSTLEFT, "", "", llst_rec.code},
	{1, LIN, "statusDesc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description          : ", "Enter description for Location Status code ",
		YES, NO,  JUSTLEFT, "", "", llst_rec.desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Update 				(void);
void 	SrchLlst 			(char *);
int  	spec_valid 			(int);
int  	heading 			(int);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	TruePosition	=	TRUE;
	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = FALSE; 
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;

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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (llst, llst_list, LLST_NO_FIELDS, "llst_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (llst);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------------
	| Validate Location status code. |
	--------------------------------*/
	if (LCHECK ("locationStatusCode"))
	{
		if (SRCH_KEY)
		{
			SrchLlst (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (llst_rec.co_no, comm_rec.co_no);
		cc = find_rec (llst, &llst_rec, COMPARISON, "w");
		if (cc) 
			newCode 	= TRUE;
		else    
		{
			newCode 	= FALSE;
			entry_exit 	= TRUE;
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================
| Updated records. |
==================*/
void
Update (
 void)
{
	strcpy (llst_rec.co_no, comm_rec.co_no);
	if (newCode)
	{
		cc = abc_add (llst, &llst_rec);
		if (cc) 
			file_err (cc, (char *)llst, "DBADD");
	}
	else
	{
		cc = abc_update (llst, &llst_rec);
		if (cc) 
			file_err (cc, (char *)llst, "DBUPDATE");
	}

	abc_unlock (llst);
	strcpy (local_rec.previousCode, llst_rec.code);
}

void
SrchLlst (
 char *key_val)
{
	_work_open (1,1,20);
	save_rec ("#C", "#Code Description");
	strcpy (llst_rec.co_no, comm_rec.co_no);
	sprintf (llst_rec.code, "%-1.1s", key_val);

	cc = find_rec (llst, &llst_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (llst_rec.co_no, comm_rec.co_no) &&
		   !strncmp (llst_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (llst_rec.code, llst_rec.desc);
		if (cc)
			break;

		cc = find_rec (llst, &llst_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (llst_rec.desc, "%-20.20s", " ");
		return;
	}

	strcpy (llst_rec.co_no, comm_rec.co_no);
	sprintf (llst_rec.code, "%-1.1s", temp_str);
	cc = find_rec (llst, &llst_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)llst, "DBFIND");
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

		strcpy (err_str, ML ("Location Status Code Maintenance"));
		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

		print_at (0,65, ML (mlSkMess084), local_rec.previousCode);

		move (0,1);
		line (80);

		box (0, 3, 80, 2);

		move (0, 20);
		line (80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

		move (0, 22);
		line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

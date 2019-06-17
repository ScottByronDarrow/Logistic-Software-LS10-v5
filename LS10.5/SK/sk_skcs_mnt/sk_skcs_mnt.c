/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_skcs_mnt.c,v 5.3 2002/07/25 11:17:37 scott Exp $
|  Program Name  : (sk_skcs_mnt.c )                                   |
|  Program Desc  : (Container Status Maintenance.               )     |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : (12th Dec 2000)  |
|---------------------------------------------------------------------|
| $Log: sk_skcs_mnt.c,v $
| Revision 5.3  2002/07/25 11:17:37  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:19:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:53  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:43  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.3  2000/12/20 05:19:50  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 1.2  2000/12/12 07:46:19  scott
| Updated after testing + includes update and delete ring menu options.
|
| Revision 1.1  2000/12/12 02:51:45  scott
| New Program
|
| Revision 1.1  2000/12/12 02:49:36  scott
| New Program to maintain container status codes.
|
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_skcs_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_skcs_mnt/sk_skcs_mnt.c,v 5.3 2002/07/25 11:17:37 scott Exp $";

#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <ml_tr_mess.h>
#include <pslscr.h>
#include <minimenu.h>

#define	UPDATE		0
#define	LSL_IGNORE	1
#define	LSL_DELETE	2
#define	DEFAULT		99

#include	"schema"

struct commRecord	comm_rec;
struct skcsRecord	skcs_rec;
struct skcmRecord	skcm_rec;

	char	*data = "data";

   	int		newCode	= FALSE;
	extern	int		TruePosition;

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

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	previousCode [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "containerStatusCode",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Container status code : ", "Enter Container status code.",
		 NE, NO, JUSTLEFT, "", "", skcs_rec.code},
	{1, LIN, "statusDesc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description           : ", "Enter description for Container Status code ",
		YES, NO,  JUSTLEFT, "", "", skcs_rec.desc},

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
void 	SrchSkcs 			(char *);
int  	spec_valid 			(int);
int  	heading 			(int);
int		SkcsDelOk 			(void);

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
	open_rec (skcs, skcs_list, SKCS_NO_FIELDS, "skcs_id_no");
	open_rec (skcm, skcm_list, SKCM_NO_FIELDS, "skcm_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (skcs);
	abc_fclose (skcm);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Container status code. |
	---------------------------------*/
	if (LCHECK ("containerStatusCode"))
	{
		if (SRCH_KEY)
		{
			SrchSkcs (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (skcs_rec.co_no, comm_rec.co_no);
		cc = find_rec (skcs, &skcs_rec, COMPARISON, "w");
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
	int		exitLoop;

	if (newCode)
	{
		strcpy (skcs_rec.co_no,comm_rec.co_no);
		cc = abc_add (skcs, &skcs_rec);
		if (cc)
			file_err (cc, (char *)skcs, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATE :
				cc = abc_update (skcs,&skcs_rec);
				if (cc)
					file_err (cc, (char *)skcs, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case LSL_IGNORE :
				abc_unlock (skcs);
				exitLoop = TRUE;
				break;
	
			case LSL_DELETE :
				if (SkcsDelOk ())
				{
					clear_mess ();
					cc = abc_delete (skcs);
					if (cc)
						file_err (cc, (char *)skcs, "DBUPDATE");
				}
				else
				{
					print_mess (ML (mlTrMess082));
					sleep (sleepTime);
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (skcs);
	strcpy (local_rec.previousCode, skcs_rec.code);
}

/*===========================
| Check whether it is OK to |
| delete the iuds record.   |
| Files checked are :       |
===========================*/
int
SkcsDelOk (
 void)
{
	strcpy (skcm_rec.co_no, skcs_rec.co_no);
	strcpy (skcm_rec.stat_code, skcs_rec.code);
	return (find_rec (skcm, &skcm_rec, COMPARISON, "r"));
}
void
SrchSkcs (
 char *key_val)
{
	_work_open (1,2,20);
	save_rec ("#CD", "#Code Description");
	strcpy (skcs_rec.co_no, comm_rec.co_no);
	sprintf (skcs_rec.code, "%-2.2s", key_val);

	cc = find_rec (skcs, &skcs_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (skcs_rec.co_no, comm_rec.co_no) &&
		   !strncmp (skcs_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (skcs_rec.code, skcs_rec.desc);
		if (cc)
			break;

		cc = find_rec (skcs, &skcs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (skcs_rec.desc, "%-40.40s", " ");
		return;
	}

	strcpy (skcs_rec.co_no, comm_rec.co_no);
	sprintf (skcs_rec.code, "%-2.2s", temp_str);
	cc = find_rec (skcs, &skcs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)skcs, "DBFIND");
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

		strcpy (err_str, ML ("Container Status Code Maintenance"));
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


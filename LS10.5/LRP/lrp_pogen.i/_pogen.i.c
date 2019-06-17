/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _pogen.i.c,v 5.4 2002/07/17 09:57:22 scott Exp $
|  Program Name  : (lrp_pogen.i.c  )                                  |
|  Program Desc  : (Input lpno & database filename for          )     |
|                  (Purchase Order Generation.                  )     |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: _pogen.i.c,v $
| Revision 5.4  2002/07/17 09:57:22  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/11 02:17:32  cha
| SE-229. Updated to put delays in error messages.
|
| Revision 5.2  2001/08/09 09:29:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:39  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:31  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/24 08:47:53  scott
| Updated to use file_err instead of sys_err.
|
| Revision 3.0  2000/10/10 12:15:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:43  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.13  2000/07/10 01:52:29  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.12  2000/07/06 08:37:56  scott
| Updated as general maintenance to add app.schema
|
| Revision 1.11  2000/07/03 20:54:03  johno
| Modified to allow for execution of lrp_wogen as well as lrp_pogen.
| Modified Makefile to create link to lrp_wogen.i
|
| Revision 1.10  1999/12/06 01:34:19  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/17 06:40:14  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/10/27 07:33:01  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.7  1999/10/11 21:45:30  cam
| Fixed prototypes for heading ()
|
| Revision 1.6  1999/09/29 10:10:50  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:26:41  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 09:20:44  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 07:27:04  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _pogen.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_pogen.i/_pogen.i.c,v 5.4 2002/07/17 09:57:22 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_lrp_mess.h>
#include <ml_std_mess.h>

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct ffwkRecord	ffwk_rec;

	char	*programDesc;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	int		printerNumber;
	char	filename [15];
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "printerNumber",		 4, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "filename",	 6, 18, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", "Filename ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{0, LIN, "",		 0,  0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 	 (void);
int 	spec_valid 		 (int);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ReadMisc 		 (void);
void 	SrchFfwk 		 (char	*);
int 	heading 		 (int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int        argc,
 char*      argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

  	ReadMisc ();

	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		heading (1);
		entry (1);
		if (restart)
			continue;

		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
		prog_exit = TRUE;
	}
	shutdown_prog ();

	if (!strncmp (argv [0], "lrp_pogen", 9))
	{
		sprintf (err_str, 
			"lrp_pogen %d %s", local_rec.printerNumber,local_rec.filename);
	}
	else
	{
		sprintf (err_str, 
			"lrp_wogen %d %s", local_rec.printerNumber,local_rec.filename);
	}
	sys_exec (err_str);
	return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
    CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int        field)
{
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			/*----------------------------------------------------
			| The Number of Printers Available is %d ",no_lps ())|
			----------------------------------------------------*/
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("filename"))
	{
		if (SRCH_KEY)
		{
			SrchFfwk (temp_str);
			return (EXIT_SUCCESS);
		}

		ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		sprintf (ffwk_rec.filename,"%-14.14s",local_rec.filename);
		cc = find_rec (ffwk,&ffwk_rec,COMPARISON,"r");
		if (cc)
		{
			/*-----------------------
			| File_name Not on File |
			-----------------------*/
			print_mess (ML (mlLrpMess039)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

    open_rec (ffwk, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffwk);
	abc_dbclose ("data");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*===============================
| Search for Database filename	|
===============================*/
void
SrchFfwk (
 char*          key_val)
{
	char	lastName [15];

	work_open ();
	save_rec ("#Filename","# ");
	ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (ffwk_rec.filename,"%-14.14s",key_val);
	cc = find_rec ("ffwk",&ffwk_rec,GTEQ,"r");
	strcpy (lastName,"              ");
	while (!cc && !strncmp (ffwk_rec.filename,key_val,strlen (key_val)) && 
			ffwk_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strcmp (lastName,ffwk_rec.filename))
		{
			cc = save_rec (ffwk_rec.filename," ");
			if (cc)
				break;
		}

		strcpy (lastName,ffwk_rec.filename);
		cc = find_rec ("ffwk",&ffwk_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int        scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	
	/*--------------------------
	| Generate Purchase Orders |
	--------------------------*/
	sprintf (err_str, " %s ", ML (mlLrpMess001));
	rv_pr (err_str,27,0,1);
	move (0,1);
	line (80);

	box (0,3,80,3);

	move (1,5);
	line (79);

	move (0,18);
	line (80);
	print_at (19,1, mlStdMess038, comm_rec.co_no,comm_rec.co_name);
	print_at (20,1, mlStdMess039, comm_rec.est_no,comm_rec.est_name);
	print_at (21,1, mlStdMess099, comm_rec.cc_no,comm_rec.cc_name);
	move (0,22);
	line (80);
	move (1,input_row);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}

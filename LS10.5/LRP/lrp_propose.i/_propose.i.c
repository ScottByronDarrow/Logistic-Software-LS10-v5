/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _propose.i.c,v 5.3 2002/07/17 09:57:22 scott Exp $
|  Program Name  : ( lrp_propose.i.c )                                |
|  Program Desc  : ( Input lpno & database filename for           )   |
|                  ( Proposed Purchase Order Print.               )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _propose.i.c,v $
| Revision 5.3  2002/07/17 09:57:22  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:29:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:41  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/26 08:57:28  scott
| Updated to add app.schema
|
| Revision 3.1  2000/11/23 03:56:35  scott
| Updated during general testing - changed to allow restart.
|
| Revision 3.0  2000/10/10 12:15:34  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:44  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.9  2000/04/14 05:49:37  ramon
| Added description fields for background and overnight fields.
|
| Revision 1.8  1999/12/06 01:34:20  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/17 06:40:15  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/10/27 07:33:02  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.5  1999/09/29 10:10:51  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:41  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 09:20:45  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 07:27:05  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _propose.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_propose.i/_propose.i.c,v 5.3 2002/07/17 09:57:22 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_lrp_mess.h>

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct ffwkRecord	ffwk_rec;

	char	*programDescription;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	int		printerNumber;
	char	printerString [3];
	char	back [6];
	char	back_desc [6];
	char	onight [6];
	char	onight_desc [6];
	char	filename [15];
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "printerNumber",		4, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "back",		5, 18, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "back_desc",	5, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",	5, 60, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc",	5, 63, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc},
	{1, LIN, "filename",	7, 18, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", "Filename ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{0, LIN, "",		0,  0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	spec_valid 			(int);
int 	heading 			(int);
void 	shutdown_prog 		(void);
void 	LoadDefaults 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	SrchFfwk 			(char *);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int        argc,
 char*      argv [])
{
	if (argc != 2) 
	{
		print_at (0,0,mlStdMess244,argv [0]);
        return (EXIT_FAILURE);
	}

	programDescription = argv [1];

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

    OpenDB ();

    ReadMisc ();

	/*=====================
	| Reset control flags |
	=====================*/
	init_vars (1);
	init_ok = 0;

	LoadDefaults ();

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
		{
			shutdown_prog ();
    		return (EXIT_SUCCESS);
		}
	
		ReadMisc ();

		if (!strcmp (local_rec.filename,"              "))
		{
			print_mess (ML (mlLrpMess038));
			sleep (sleepTime);
			continue;
		}
		prog_exit = 1;
	}
	shutdown_prog ();
	sprintf (local_rec.printerString,"%d",local_rec.printerNumber);

	clear ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight [0] == 'Y') 
	{
		print_at (0,0,mlStdMess035);
		fflush (stdout);
		execlp 
		(
			"ONIGHT",
			"ONIGHT",
			"lrp_reorder",
			local_rec.printerString,
			local_rec.filename,
			argv [1], 
			(char *) 0
		);
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () != 0)
		{
			clear ();
			print_at (0,0,mlStdMess035);
			fflush (stdout);
		}
		else
        {
			execlp 
			(
				"lrp_reorder",
				"lrp_reorder",
				local_rec.printerString,
				local_rec.filename,
				(char *) 0
			);
        }   
    }
	else 
	{
		clear ();
		print_at (0,0,mlStdMess035);
		fflush (stdout);

		execlp 
		(
			"lrp_reorder",
			"lrp_reorder",
			local_rec.printerString,
			local_rec.filename,
			(char *) 0
		);
	}
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

void
LoadDefaults (void)
{
	scn_set (1);

	local_rec.printerNumber = 1;
	strcpy (local_rec.back,"N");
	strcpy (local_rec.back_desc,"No ");
	strcpy (local_rec.onight,"N");
	strcpy (local_rec.onight_desc,"No ");
	sprintf (local_rec.filename,"%-14.14s"," ");
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
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.back_desc,"Yes");
			strcpy (local_rec.onight,"N");
			strcpy (local_rec.onight_desc,"No ");
			DSP_FLD ("onight");
			DSP_FLD ("onight_desc");
		}
		else
			strcpy (local_rec.back_desc,"No ");
		display_field (field+1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.onight_desc,"Yes");
			strcpy (local_rec.back,"N");
			strcpy (local_rec.back_desc,"No ");
			DSP_FLD ("back");
			DSP_FLD ("back_desc");
		}
		else
			strcpy (local_rec.onight_desc,"No ");
		display_field (field+1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("filename"))
	{
		if (SRCH_KEY)
			SrchFfwk (temp_str);

		ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		sprintf (ffwk_rec.filename,"%-14.14s",local_rec.filename);
		cc = find_rec (ffwk,&ffwk_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlLrpMess039));
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

	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc, PNAME);
	abc_fclose (ccmr);
}

/*===============================
| Search for Database filename	|
===============================*/
void
SrchFfwk (
 char*      key_val)
{
	char	last_name [15];

	work_open ();
	save_rec ("#Filename","# ");
	ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (ffwk_rec.filename,"%-14.14s",key_val);
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"r");
	strcpy (last_name,"              ");
	while (!cc && !strncmp (ffwk_rec.filename,key_val,strlen (key_val)) && 
					        ffwk_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strcmp (last_name,ffwk_rec.filename))
		{
			cc = save_rec (ffwk_rec.filename," ");
			if (cc)
				break;
		}

		strcpy (last_name,ffwk_rec.filename);
		cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
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
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		
		sprintf (err_str," %s ",programDescription);
		rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
		move (0,1);
		line (80);

		box (0,3,80,4);

		move (1,6);
		line (79);

		move (0,18);
		line (80);
		print_at (19,1,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (20,1,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		print_at (21,1,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_name);
		move (0,22);
		line (80);
		move (1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

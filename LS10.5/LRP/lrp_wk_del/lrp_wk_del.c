/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_wk_del.c,v 5.2 2001/08/09 09:30:10 scott Exp $
|  Program Name  : (ff_wk_del.c)                                      |
|  Program Desc  : (Deletion of specified ffwk reorder files    )     |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: lrp_wk_del.c,v $
| Revision 5.2  2001/08/09 09:30:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:28:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:00  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/29 08:54:32  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:15:45  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:53  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:34:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/10/27 07:33:04  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.6  1999/09/29 10:10:54  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:26:47  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 09:20:50  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/07/16 01:00:52  scott
| Updated for abc_delete
|
| Revision 1.2  1999/06/15 07:27:07  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_wk_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_wk_del/lrp_wk_del.c,v 5.2 2001/08/09 09:30:10 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_lrp_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ffwkRecord	ffwk_rec;

	char 	*data = "data";

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	confirm [2];
	char	filename [15];
	char	dummy [11];
} local_rec;

static struct	var	vars [] =
{
	{1, LIN, "filename",	4, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "Reorder File Name ", "Enter Name of Reorder File ",
		NE, YES, JUSTLEFT, "", "", local_rec.filename},
	{1, LIN, "confirm",	5, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Delete This File ? ", "Confirm That You Want to Delete This File ",
		NA, NO, JUSTLEFT, "NY", "", local_rec.confirm},
	{0, LIN, "",		 0,  0, CHARTYPE,
		"U", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
int 	spec_valid 			 (int);
int 	heading 			 (int);
void 	DeleteFfwk 			 (void);
void 	ReadCcmr 			 (void);
void	SrchFfwk 			 (char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int	count = argc;

	OpenDB (); 	
	ReadCcmr ();

	if (argc > 1)
	{
		while (count-- > 1)
		{
			sprintf (local_rec.filename, "%-14.14s", argv [count]);
			DeleteFfwk ();
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}


	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	set_masks ();

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);

		FLD ("confirm") = NA;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		DeleteFfwk ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}
/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ffwk, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffwk);
	abc_fclose (ccmr);
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("filename"))
	{
		if (SRCH_KEY)
		{
			SrchFfwk (temp_str);
			sprintf (local_rec.filename , "%-14.14s", ffwk_rec.filename);
		}
		ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (ffwk_rec.filename,local_rec.filename);
	
		cc = find_rec (ffwk,&ffwk_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess ("Invalid Filename ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		FLD ("confirm") = YES;
		DSP_FLD ("confirm");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("confirm"))
	{
		if (local_rec.confirm [0] == 'N')
			restart = 1;
	}

	return (EXIT_SUCCESS);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlLrpMess075), 29,0,1);
	
		move (0,1);
		line (80);

		box (0, 3, 80, 2);

		move (0,21);
		line (80);
		move (0,22);
		print_at (22,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
DeleteFfwk (void)
{
    ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
    strcpy (ffwk_rec.filename,local_rec.filename);

    cc = find_rec (ffwk, &ffwk_rec, GTEQ, "u");

    /*---------------------------
    | Delete Existing Records	|
    ---------------------------*/
    while (!cc &&
           ffwk_rec.hhcc_hash == ccmr_rec.hhcc_hash &&
           !strncmp (local_rec.filename,ffwk_rec.filename,14))
    {
	    abc_delete (ffwk);
	    
	    cc = find_rec (ffwk, &ffwk_rec, GTEQ, "u");
    }
    abc_unlock (ffwk);
}

void
ReadCcmr (void)
{
	strcpy (ccmr_rec.co_no , comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);

	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");

	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
SrchFfwk (
 char*  key_val)
{
	char	lastFile [15];

	work_open ();
	save_rec ("#Filename ", "# ");

	ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (ffwk_rec.filename, "%-14.14s", key_val);
	sprintf (lastFile, "%-14.14s", ffwk_rec.filename);
	
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"r");

	while (!cc && 
	       ffwk_rec.hhcc_hash ==  ccmr_rec.hhcc_hash  &&
	       !strncmp (ffwk_rec.filename, key_val, (strlen (key_val))))
	{
		if (strcmp (lastFile, ffwk_rec.filename))
		{
			sprintf (lastFile, "%-14.14s", ffwk_rec.filename);
			cc = save_rec (ffwk_rec.filename, " ");
			if (cc)
				break;
		}

		cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (ffwk_rec.filename, "%-14.14s", temp_str);
	
	cc = find_rec (ffwk,&ffwk_rec,COMPARISON,"r");

	if (cc)
		file_err (cc, "ffwk", "DBFIND");
}


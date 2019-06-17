/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_nvardel.c,v 5.2 2001/08/09 09:15:54 scott Exp $
|  Program Name  : ( po_nvardel.c   )                                 |
|  Program Desc  : ( Purchase Order NEW variance deletion.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohs,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pohs,     ,     ,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (19/03/93)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified  by : Ana Marie Tario   |
|                                                                     |
|  Comments      :                                                    |
|  (19/03/93)    : EGC 8303                                           |
|  (12/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                                                                     |
| $Log: po_nvardel.c,v $
| Revision 5.2  2001/08/09 09:15:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:33  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/11 06:43:15  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.6  1999/11/05 05:17:13  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.5  1999/09/29 10:12:04  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/21 04:38:05  scott
| Updated from Ansi project
|
| Revision 1.3  1999/06/17 10:06:30  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_nvardel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_nvardel/po_nvardel.c,v 5.2 2001/08/09 09:15:54 scott Exp $";

#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<pslscr.h>
#include	<dsp_screen.h>

#include	"schema"

struct commRecord	comm_rec;
struct pohsRecord	pohs_rec;
struct pohsRecord	pohs2_rec;

	char	*data	= "data",
			*pohs2	= "pohs2";

	double	est_tot = 0.00,
			act_tot = 0.00,
			qvr_tot = 0.00,
			pvr_tot = 0.00;

	long	fst_rec_dte	= 0L,
			lst_rec_dte	= 0L,
			fst_cst_dte	= 0L,
			lst_cst_dte	= 0L;

static	struct	var	vars [] =
{
	{1, LIN, "fst_rec_dte",	 3, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Receipt Date", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &fst_rec_dte},
	{1, LIN, "lst_rec_dte",	 4, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "31/12/99", "End Receipt Date  ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &lst_rec_dte},
	{1, LIN, "fst_cst_dte",	 5, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Costing Date", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &fst_cst_dte},
	{1, LIN, "lst_cst_dte",	 6, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "31/12/99", "End Costing Date  ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &lst_cst_dte},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"", "          ",
		"", "", "", " ",
		YES, NO, JUSTRIGHT, "", "", err_str},
};


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);
void 	Process 		(void);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);

	/*----------------------------
	| setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	while (!prog_exit)
	{
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Process ();
        break;
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
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
	abc_alias (pohs2, pohs);

	open_rec (pohs,  pohs_list, POHS_NO_FIELDS, "pohs_id_no2");
	open_rec (pohs2, pohs_list, POHS_NO_FIELDS, "pohs_id_no");
}

/*=======================
| Close Database Files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (pohs);
	abc_fclose (pohs2);
	abc_dbclose (data);
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
		centre_at (0, 80, ML(mlPoMess149));
		move (0, 1);
		line (81);
		box (0, 2, 80, 4);

		line_cnt = 0;
		scn_write (scn);
	}
	move (0, 21);
	line (81);
	strcpy(err_str,ML(mlStdMess038));
	print_at (22, 0, err_str, comm_rec.co_no, comm_rec.co_name);

    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	dsp_screen ("Processing: ", comm_rec.co_no, comm_rec.co_name);

	strcpy (pohs_rec.co_no, comm_rec.co_no);
	strcpy (pohs_rec.br_no, comm_rec.est_no);
	strcpy (pohs_rec.stat_flag, "C");
	cc = find_rec (pohs, &pohs_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (pohs_rec.co_no, comm_rec.co_no) &&
		!strcmp (pohs_rec.br_no, comm_rec.est_no) &&
		!strcmp (pohs_rec.stat_flag, "C")
	)
	{
		if
		(
			pohs_rec.printed [0] != 'Y' ||
			pohs_rec.date_receipt < fst_rec_dte ||
			pohs_rec.date_receipt > lst_rec_dte ||
			pohs_rec.date_cost < fst_cst_dte ||
			pohs_rec.date_cost > lst_cst_dte
		)
		{
			cc = find_rec (pohs, &pohs_rec, NEXT, "r");
			continue;
		}

		cc = find_rec (pohs2, &pohs_rec, EQUAL, "u");
		if (cc)
			file_err (cc, "pohs2", "DBFIND");

		cc = abc_delete (pohs2);
		if (cc)
			file_err (cc, "pohs2", "DBDELETE");

		cc = find_rec (pohs, &pohs_rec, NEXT, "r");
	}
}

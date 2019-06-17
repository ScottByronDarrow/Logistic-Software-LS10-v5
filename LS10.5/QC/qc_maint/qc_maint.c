/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( qc_maint.c     )                                 |
|  Program Desc  : ( QC Centre Maintenacne                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, qcmr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  qcmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (24/08/94)      | Author       : Aroha Merrilees.  |
|---------------------------------------------------------------------|
|  Date Modified : (31/10/94)      | Modified by  : Aroha Merrilees.  |
|  Date Modified : (03/11/94)      | Modified by  : Aroha Merrilees.  |
|  Date Modified : (04/09/1997)    | Modified by  : Jiggs A Veloz.    |
|  Date Modified : (17/09/1999)    | Modified by  : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (31/10/94)    : PSL 11299 - upgrade to ver 9 - mfg cutover - on    |
|                : code changes                                       |
|  (03/11/94)    : PSL 11299 - check QC_APPLY                         |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (17/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
| $Log: qc_maint.c,v $
| Revision 5.3  2002/07/25 11:17:31  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:16:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:33  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:08:48  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/17 06:40:33  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.7  1999/11/04 08:44:10  scott
| Updated to fix warnings due to -Wall flag.
|
| Revision 1.6  1999/09/29 10:12:23  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/21 05:26:34  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/18 05:55:42  scott
| Updated to add log file for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_maint/qc_maint.c,v 5.3 2002/07/25 11:17:31 scott Exp $";

#include	<pslscr.h>
#include	<minimenu.h>
#include	<ml_qc_mess.h>
#include	<ml_std_mess.h>

#define		UPDATEREC	0
#define		IGNOREREC	1
#define		DELETEREC	2
#define		DEFAULT		99

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
	} comm_rec;

	/*=======================
	| QC Centre Master File |
	=======================*/
	struct dbview qcmr_list [] =
	{
		{"qcmr_co_no"},
		{"qcmr_br_no"},
		{"qcmr_centre"},
		{"qcmr_description"}
	};

	int	qcmr_no_fields = 4;

	struct tag_qcmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	centre [5];
		char	description [41];
	} qcmr_rec;

	char	*data	= "data",
			*comm	= "comm",
			*qcmr	= "qcmr"; 

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

	int		newCentre;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy[11];
	char	prevCentre [5];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "centre",	 4, 18, CHARTYPE,
		"UUUU", "          ",
		" ", "", "Centre Code     :", "QC Centre Code",
		 NE, NO, JUSTLEFT, "", "", qcmr_rec.centre},
	{1, LIN, "description",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description     :", "QC Centre Description",
		YES, NO,  JUSTLEFT, "", "", qcmr_rec.description},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		(void);
void	CloseDB	(void);
int		Update		(void);
void	SrchQcmr	(char *);
int		heading (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		QC_APPLY;
	char *	sptr;

	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
	{
		/*-------------------
		| QC_APPLY not set. |
		-------------------*/
		print_at (0, 0, "\n\n%s", ML(mlQcMess001));
		return (EXIT_FAILURE);
	}

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	init_vars (1);
	strcpy (local_rec.prevCentre, "  ");

	while (prog_exit == 0)
	{
		prog_exit	= FALSE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

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
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (qcmr, qcmr_list, qcmr_no_fields, "qcmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (qcmr);

	abc_dbclose (data);
}

int
heading (
 int	scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	/*-----------------------
	| QC Centre Maintenace |
	-----------------------*/
	sprintf (err_str, " %s ", ML (mlQcMess006));
	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

	/*--------------------
	| Last Centre: %4.4s |
	--------------------*/
	print_at (0, 60, ML (mlQcMess005), local_rec.prevCentre);
	move (0, 1); line (80);

	box (0, 3, 80, 2);

	move (0, 21); line (80);
	sprintf (err_str, ML (mlStdMess038), 
			 clip (comm_rec.tco_no), clip (comm_rec.tco_name));
	print_at (22, 0, "%s", err_str);
	print_at (22, 45, ML (mlStdMess039), clip (comm_rec.test_no), clip (comm_rec.test_name));

	scn_write (scn);
	line_cnt = 0;
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int	field)
{
	/*---------------------
	| Validate QC Centre. |
	---------------------*/
	if (LCHECK ("centre"))
	{
		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		newCentre = TRUE;
		strcpy (qcmr_rec.co_no, comm_rec.tco_no);
		strcpy (qcmr_rec.br_no, comm_rec.test_no);
		if (!find_rec (qcmr, &qcmr_rec, EQUAL, "w"))
		{
			newCentre = FALSE;
			entry_exit = TRUE;
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
Update (
 void)
{
	if (newCentre)
	{
		/*----------------------------
		| Update new centre record . |
		----------------------------*/
		strcpy (qcmr_rec.co_no, comm_rec.tco_no);
		strcpy (qcmr_rec.br_no, comm_rec.test_no);
		if ((cc = abc_add (qcmr, &qcmr_rec)))
			file_err (cc, qcmr, "DBADD");
	}
	else
	{
		int		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATEREC :
				/*---------------------------------
				| Update existing centre record . |
				---------------------------------*/
				cc = abc_update (qcmr, &qcmr_rec);
				if (cc) 
					file_err (cc, qcmr, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case IGNOREREC :
				abc_unlock (qcmr);
				exitLoop = TRUE;
				break;
	
			case DELETEREC :
				cc = abc_delete (qcmr);
				if (cc)
					file_err (cc, qcmr, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	strcpy (local_rec.prevCentre, qcmr_rec.centre);

	return (EXIT_SUCCESS);
}

void
SrchQcmr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Cntr", "#Centre Description");

	strcpy (qcmr_rec.co_no, comm_rec.tco_no);
	strcpy (qcmr_rec.br_no, comm_rec.test_no);
	sprintf (qcmr_rec.centre, "%-4.4s", key_val);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc && !strncmp (qcmr_rec.centre, key_val, strlen (key_val)))
	{
		if (save_rec (qcmr_rec.centre, qcmr_rec.description))
			break;

		cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qcmr_rec.co_no, comm_rec.tco_no);
	strcpy (qcmr_rec.br_no, comm_rec.test_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	if ((cc = find_rec (qcmr, &qcmr_rec, COMPARISON, "r")))
		file_err (cc, qcmr, "DBFIND");
}


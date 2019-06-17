/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_spec_inp  )                                   |
|  Program Desc  : ( Asset Specification Maintenance            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, assp,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  assp,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 08/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (06/02/98)      | Modified by : Elena B Cuaresma   |
|                                                                     |
|  Comments      :                                                    |
|  (06/02/98)    : 9.10  New asset module for the standard version 9. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: spec_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_spec_inp/spec_inp.c,v 5.3 2002/07/25 11:17:25 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#if 0
#define	SEL_DELETE 2
#endif
#define	SEL_DEFAULT	99

#define SLEEP_TIME 3

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int	comm_no_fields = 3;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
	} comm_rec;

	/*============================+
	 | Asset Specification File |
	 +============================*/

#define	ASSP_NO_FIELDS	3

	struct dbview	assp_list [ASSP_NO_FIELDS] =
	{
		{"assp_co_no"},
		{"assp_spec_code"},
		{"assp_desc"}
	};

	struct tag_asspRecord
	{
		char	co_no [3];
		char	spec_code [9];
		char	desc [41];
	}	assp_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*assp  = "assp";


MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
#ifdef SEL_DELETE
	{ " 3. DELETE RECORD.                     ",
	  "" },
#endif
	{ ENDMENU }
};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars[] =
{
	{ 1, LIN, "spec_code",	 4, 30, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Asset Specification Code   : ", "Enter Asset Specification Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", assp_rec.spec_code },
	{ 1, LIN, "desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Specification Desc.  : ", "Enter Asset Specification Description",
		YES, NO, JUSTLEFT, "", "", assp_rec.desc },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};

static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
static void SrchAssp (char *key_val);
static void update (void);

int spec_valid (int field);
int heading (int scn);

#ifdef SEL_DELETE
static BOOL	FrspDelOk (char *badFileName);
#endif

static BOOL	newCode = FALSE;

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

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
		edit_exit = FALSE;
		restart = FALSE;
		search_ok = TRUE;
		init_vars (1);

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

		if (restart)
			continue;

		update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
static void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (assp, assp_list, ASSP_NO_FIELDS, "assp_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (assp);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("spec_code"))
	{
		if (SRCH_KEY)
		{
			SrchAssp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (assp_rec.spec_code, "    "))
			return(1);

		strcpy (assp_rec.co_no, comm_rec.tco_no);
		cc = find_rec (assp, &assp_rec, EQUAL, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
		}
		return(0);
	}

	return(0);
}

/*=============================================
| Search Asset System Specification Code File
===============================================*/
static void
SrchAssp (
 char*              key_val)
{
	struct tag_asspRecord assp_bak;

	memcpy (&assp_bak, &assp_rec, sizeof assp_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (assp_rec.co_no, comm_rec.tco_no);
	strcpy (assp_rec.spec_code, key_val);

	cc = find_rec (assp, &assp_rec, GTEQ, "r");

	while (!cc &&
			 	!strcmp (assp_rec.co_no, comm_rec.tco_no) &&
				!strncmp (assp_rec.spec_code, key_val, strlen(key_val)))
	{
		cc = save_rec (assp_rec.spec_code, assp_rec.desc);
		if (cc)
			break;

		cc = find_rec (assp, &assp_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (assp_rec.co_no, comm_rec.tco_no);
		strcpy (assp_rec.spec_code, temp_str);
		cc = find_rec (assp, &assp_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, assp, "DBFIND");
	}

	if (cc)
		memcpy (&assp_rec, &assp_bak, sizeof assp_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (assp_rec.co_no, comm_rec.tco_no);
	if (newCode)
	{
		cc = abc_add (assp, &assp_rec);
		if (cc) 
			file_err(cc, assp, "DBADD");
	}
	else
	{
		BOOL exitLoop = FALSE;

		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (assp, &assp_rec);
				if (cc) 
					file_err (cc, assp, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (assp);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrspDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (assp);
					if (cc)
						file_err (cc, assp, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,
						ML("Matching Document Records Found in %-4.4s, Document Record Not Deleted"),
						badFileName);

					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}

				exitLoop = TRUE;
				break;
			}
#endif

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (assp);
}

/*===========================
| Check whether it is OK to |
| delete the assp record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
static BOOL
FrspDelOk (
 char*              badFileName)
{
	
	return (TRUE);
}
#endif

/*===========================
| edit () callback function |
===========================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		centre_at (0,80, ML("%R Asset Specification Maintenance "));
		move (0, 1); line (80);

		box (0, 3, 80, 2);

		move (0, 20); line(80);
		print_at(21,0, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
		move (0, 22); line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

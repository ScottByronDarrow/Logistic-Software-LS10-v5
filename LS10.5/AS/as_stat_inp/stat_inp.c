/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_stat_inp  )                                   |
|  Program Desc  : ( Asset Status Code Input                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, assc,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  assc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 12/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (06/02/98)      | Modified by : Elena B Cuaresma.  |
|                                                                     |
|  Comments      :                                                    |
|  (06/02/98)    : 9.10  New asset module for the standard version 9. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: stat_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_stat_inp/stat_inp.c,v 5.4 2002/07/25 11:17:25 scott Exp $";

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

	/*==========================+
	 | Asset Status Code File  |
	 +==========================*/

#define	ASSC_NO_FIELDS	3

	struct dbview	assc_list [ASSC_NO_FIELDS] =
	{
		{"assc_co_no"},
		{"assc_stat_code"},
		{"assc_stat_desc"}
	};

	struct tag_asscRecord
	{
		char	co_no [3];
		char	stat_code [3];
		char	stat_desc [41];
	}	assc_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*assc  = "assc";


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
	{ 1, LIN, "stat_code",	 4, 30, CHARTYPE,
		"UU", "          ",
		" ", "", "Asset Status Code          : ", "Enter Asset Status Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", assc_rec.stat_code },
	{ 1, LIN, "stat_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Status Description   : ", "Enter Asset Status Description",
		YES, NO, JUSTLEFT, "", "", assc_rec.stat_desc },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
static void SrchAssc (char *key_val);
static void update (void);

int spec_valid (int field);
int heading (int scn);

#ifdef SEL_DELETE
static BOOL	FrscDelOk (char *badFileName);
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

	open_rec (assc, assc_list, ASSC_NO_FIELDS, "assc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (assc);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("stat_code"))
	{
		if (SRCH_KEY)
		{
			SrchAssc (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (assc_rec.stat_code, "  "))
			return(1);

		strcpy (assc_rec.co_no, comm_rec.tco_no);
		cc = find_rec (assc, &assc_rec, EQUAL, "u");
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
SrchAssc (
 char*              key_val)
{
	struct tag_asscRecord assc_bak;

	memcpy (&assc_bak, &assc_rec, sizeof assc_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (assc_rec.co_no, comm_rec.tco_no);
	strcpy (assc_rec.stat_code, key_val);

	cc = find_rec (assc, &assc_rec, GTEQ, "r");

	while (!cc &&
				!strcmp (assc_rec.co_no, comm_rec.tco_no) &&
				!strncmp (assc_rec.stat_code, key_val, strlen(key_val)))
	{
		cc = save_rec (assc_rec.stat_code, assc_rec.stat_desc);
		if (cc)
			break;

		cc = find_rec (assc, &assc_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (assc_rec.co_no, comm_rec.tco_no);
		strcpy (assc_rec.stat_code, temp_str);
		cc = find_rec (assc, &assc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, assc, "DBFIND");
	}

	if (cc)
		memcpy (&assc_rec, &assc_bak, sizeof assc_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (assc_rec.co_no, comm_rec.tco_no);
	if (newCode)
	{
		cc = abc_add (assc, &assc_rec);
		if (cc) 
			file_err(cc, assc, "DBADD");
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
				cc = abc_update (assc, &assc_rec);
				if (cc) 
					file_err (cc, assc, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (assc);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrscDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (assc);
					if (cc)
						file_err (cc, assc, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,
						ML("Matching Document Records Found in %-4.4s, Document Record Not Deleted"), badFileName);

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
	abc_unlock (assc);
}

/*===========================
| Check whether it is OK to |
| delete the assc record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
static BOOL
FrscDelOk (
char *badFileName)
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

		centre_at (0,80,ML("%R Asset Status Maintenance "));
		move (0, 1); line (80);

		box (0, 3, 80, 2);

		move (0, 20); line(80);
		move (0, 21);
		print_at (21,0, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
		move (0, 22); line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( as_type_inp  )                                   |
|  Program Desc  : ( Asset Type Maintenance                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asty,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  asty,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 13/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (03/03/95)      | Modified by : Ross S. Baquillos  |
|  Date Modified : (06/02/98)      | Modified by : Elena B Cuaresma.  |
|                                                                     |
|  Comments      : SMF 00130 - Change search heading of type code.	  |
|  (06/02/98)    : 9.10  New asset module for the standard version 9. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: type_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_type_inp/type_inp.c,v 5.3 2002/07/25 11:17:26 scott Exp $";

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

	/*===================+
	 | Asset Type File |
	 +===================*/

#define	ASTY_NO_FIELDS	3

	struct dbview	asty_list [ASTY_NO_FIELDS] =
	{
		{"asty_co_no"},
		{"asty_type_code"},
		{"asty_type_desc"}
	};

	struct tag_astyRecord
	{
		char	co_no [3];
		char	type_code [4];
		char	type_desc [41];
	}	asty_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*asty  = "asty";


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
	{ 1, LIN, "type_code",	 4, 30, CHARTYPE,
		"UUU", "          ",
		" ", "", "Asset Type Code            : ", "Enter Asset Type Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", asty_rec.type_code },
	{ 1, LIN, "type_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Type Description     : ", "Enter Asset Type Description",
		YES, NO, JUSTLEFT, "", "", asty_rec.type_desc },
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
static void SrchAsty (char *key_val);
static void update (void);

int spec_valid (int field);
int heading (int scn);

#ifdef SEL_DELETE
static BOOL	FrtyDelOk (char *badFileName);
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

	open_rec (asty, asty_list, ASTY_NO_FIELDS, "asty_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (asty);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("type_code"))
	{
		if (SRCH_KEY)
		{
			SrchAsty (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strlen(clip(asty_rec.type_code)))
			return(1);

		strcpy (asty_rec.co_no, comm_rec.tco_no);
		cc = find_rec (asty, &asty_rec, EQUAL, "u");
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
| Search Asset Type File
===============================================*/
static void
SrchAsty (
 char*              key_val)
{
	char	temp_code [6];
	struct tag_astyRecord asty_bak;

	memcpy (&asty_bak, &asty_rec, sizeof asty_bak);

	work_open();
	save_rec("#Code ","#Description");

	strcpy (asty_rec.co_no, comm_rec.tco_no);
	strcpy (asty_rec.type_code, key_val);

	cc = find_rec (asty, &asty_rec, GTEQ, "r");

	while (!cc &&
				!strcmp (asty_rec.co_no, comm_rec.tco_no) &&
				!strncmp (asty_rec.type_code, key_val, strlen(key_val)))
	{
		sprintf (temp_code,"%-4.4s", asty_rec.type_code);
		cc = save_rec (temp_code, asty_rec.type_desc);
		if (cc)
			break;

		cc = find_rec (asty, &asty_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (asty_rec.co_no, comm_rec.tco_no);
		strcpy (asty_rec.type_code, temp_str);
		cc = find_rec (asty, &asty_rec, COMPARISON, "r");
		if (cc)
		{
			
			file_err (cc, asty, "DBFIND");
		}
	}

	if (cc)
		memcpy (&asty_rec, &asty_bak, sizeof asty_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (asty_rec.co_no, comm_rec.tco_no);
	if (newCode)
	{
		cc = abc_add (asty, &asty_rec);
		if (cc) 
			file_err(cc, asty, "DBADD");
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
				cc = abc_update (asty, &asty_rec);
				if (cc) 
					file_err (cc, asty, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (asty);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrtyDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (asty);
					if (cc)
						file_err (cc, asty, "DBUPDATE");
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
	abc_unlock (asty);
}

/*===========================
| Check whether it is OK to |
| delete the asty record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
static BOOL
FrtyDelOk (
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

		centre_at (0,80,ML("%R Asset Type Maintenance "));
		move (0, 1); line (80);

		box (0, 3, 80, 2);

		move (0, 20); line(80);
		print_at (21,0, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
		move (0, 22); line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

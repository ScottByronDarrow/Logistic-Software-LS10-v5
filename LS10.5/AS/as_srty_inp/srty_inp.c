/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_srty_inp    )                                 |
|  Program Desc  : ( Asset Service Type Input                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asst,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  asst,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 12/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (06/02/98)      | Modified by : Elena B Cuaresma   |
|                                                                     |
|  Comments      :                                                    |
|  (06/02/98)    :  9.10  New asset module for the standard version 9.|
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: srty_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_srty_inp/srty_inp.c,v 5.3 2002/07/25 11:17:25 scott Exp $";

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

	/*===========================+
	 | Asset Service Type File |
	 +===========================*/
#define	ASST_NO_FIELDS	3

	struct dbview	asst_list [ASST_NO_FIELDS] =
	{
		{"asst_co_no"},
		{"asst_ser_code"},
		{"asst_ser_desc"}
	};

	struct tag_asstRecord
	{
		char	co_no [3];
		char	ser_code [3];
		char	ser_desc [41];
	}	asst_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*asst  = "asst";


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
	{ 1, LIN, "ser_code",	 4, 30, CHARTYPE,
		"UU", "          ",
		" ", "", "Asset Service Type Code    : ", "Enter Asset Service Type Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", asst_rec.ser_code },
	{ 1, LIN, "ser_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Service Type Desc.   : ", "Enter Asset Service Type Description",
		YES, NO, JUSTLEFT, "", "", asst_rec.ser_desc },
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
static void SrchAsst (char *key_val);
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

	open_rec (asst, asst_list, ASST_NO_FIELDS, "asst_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (asst);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("ser_code"))
	{
		if (SRCH_KEY)
		{
			SrchAsst (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (asst_rec.ser_code, "  "))
			return(1);

		strcpy (asst_rec.co_no, comm_rec.tco_no);
		cc = find_rec (asst, &asst_rec, EQUAL, "u");
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
SrchAsst (
 char*              key_val)
{
	struct tag_asstRecord asst_bak;

	memcpy (&asst_bak, &asst_rec, sizeof asst_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (asst_rec.co_no, comm_rec.tco_no);
	strcpy (asst_rec.ser_code, key_val);

	cc = find_rec (asst, &asst_rec, GTEQ, "r");

	while (!cc &&
				!strcmp (asst_rec.co_no, comm_rec.tco_no) &&
				!strncmp (asst_rec.ser_code, key_val, strlen(key_val)))
	{
		cc = save_rec (asst_rec.ser_code, asst_rec.ser_desc);
		if (cc)
			break;

		cc = find_rec (asst, &asst_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (asst_rec.co_no, comm_rec.tco_no);
		strcpy (asst_rec.ser_code, temp_str);
		cc = find_rec (asst, &asst_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, asst, "DBFIND");
	}

	if (cc)
		memcpy (&asst_rec, &asst_bak, sizeof asst_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (asst_rec.co_no, comm_rec.tco_no);
	if (newCode)
	{
		cc = abc_add (asst, &asst_rec);
		if (cc) 
			file_err(cc, asst, "DBADD");
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
				cc = abc_update (asst, &asst_rec);
				if (cc) 
					file_err (cc, asst, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (asst);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrtyDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (asst);
					if (cc)
						file_err (cc, asst, "DBUPDATE");
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
	abc_unlock (asst);
}

/*===========================
| Check whether it is OK to |
| delete the asst record.   |
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

		centre_at (0,80,ML("%R Asset Service Type Maintenance "));
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

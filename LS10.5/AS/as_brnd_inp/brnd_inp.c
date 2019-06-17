/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_brnd_inp  )                                   |
|  Program Desc  : ( Asset Brand Maintenance                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asbr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  asbr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 13/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (xx/xx/xx)      | Modified by :                    |
|                                                                     |
|  Comments      :                                                    |
|  (xx/xx/xx)    :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/

char	*PNAME = "$RCSfile: brnd_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_brnd_inp/brnd_inp.c,v 5.4 2002/07/25 11:17:23 scott Exp $";

#define	CCMAIN

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#if 0
#define	SEL_DELETE 2
#endif
#define	SEL_DEFAULT	99

#define SLEEP_TIME 3

#include	"schema"

struct commRecord	comm_rec;
struct asbrRecord	asbr_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	
	char	*data  = "data";

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
	{ 1, LIN, "brand_code",	 4, 30, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Asset Brand Code           : ", "Enter Asset Brand Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", asbr_rec.brand_code },
	{ 1, LIN, "brand_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Brand Description    : ", "Enter Asset Brand Description",
		YES, NO, JUSTLEFT, "", "", asbr_rec.brand_desc },
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
static void srch_asbr (char *);
static void update (void);
int spec_valid (int);
int heading (int);

#ifdef SEL_DELETE
static BOOL	FrbrDelOk (char *badFileName);
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE; 
		edit_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
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

	open_rec (asbr, asbr_list, ASBR_NO_FIELDS, "asbr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (asbr);

	abc_dbclose (data);
}

int spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("brand_code"))
	{
		if (SRCH_KEY)
		{
			srch_asbr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (asbr_rec.brand_code, "        "))
			return(1);

		strcpy (asbr_rec.co_no, comm_rec.co_no);
		cc = find_rec (asbr, &asbr_rec, EQUAL, "u");
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
srch_asbr (
 char*              key_val)
{
	struct asbrRecord asbr_bak;

	memcpy (&asbr_bak, &asbr_rec, sizeof asbr_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (asbr_rec.co_no, comm_rec.co_no);
	strcpy (asbr_rec.brand_code, key_val);

	cc = find_rec (asbr, &asbr_rec, GTEQ, "r");

	while (!cc && 
				!strcmp (asbr_rec.co_no, comm_rec.co_no) &&
				!strncmp (asbr_rec.brand_code, key_val, strlen(key_val)))
	{
		cc = save_rec (asbr_rec.brand_code, asbr_rec.brand_desc);
		if (cc)
			break;

		cc = find_rec (asbr, &asbr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (asbr_rec.co_no, comm_rec.co_no);
		strcpy (asbr_rec.brand_code, temp_str);
		cc = find_rec (asbr, &asbr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "asbr", "DBFIND");
	}

	if (cc)
		memcpy (&asbr_rec, &asbr_bak, sizeof asbr_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (asbr_rec.co_no, comm_rec.co_no);
	if (newCode)
	{
		cc = abc_add (asbr, &asbr_rec);
		if (cc) 
			file_err(cc, "asbr", "DBADD");
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
				cc = abc_update (asbr, &asbr_rec);
				if (cc) 
					file_err (cc, "asbr", "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (asbr);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrbrDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (asbr);
					if (cc)
						file_err (cc, "asbr", "DBUPDATE");
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
	abc_unlock (asbr);
}

/*===========================
| Check whether it is OK to |
| delete the asbr record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
static BOOL
FrbrDelOk (
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
		/* printf ("<%s>", PNAME); */

		centre_at (0,80, ML("%R Asset Brand Maintenance "));
		move (0, 1); line (80);

		box (0, 3, 80, 2);

		move (0, 20); line(80);
		print_at (21,0, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		move (0, 22); line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

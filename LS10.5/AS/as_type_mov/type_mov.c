/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_type_mov  )                                   |
|  Program Desc  : ( Asset Movement Type Maintenance              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asmt,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  asmt,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 01/03/95         |
|---------------------------------------------------------------------|
|  Date Modified : (06/02/98)      | Modified by : Elena B Cuaresma.  |
|                                                                     |
|  Comments      :                                                    |
|  (06/02/98)    : 9.20  New asset module for the standard version 9. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: type_mov.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_type_mov/type_mov.c,v 5.3 2002/07/25 11:17:26 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE 2
#define	SEL_DEFAULT	99

#define SLEEP_TIME 3

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_est_no"},
		{"comm_co_name"},
	};

	int	comm_no_fields = 4;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char    test_no [3];
		char	tco_name [41];
	} comm_rec;

	 /*===========================+
	 | Asset Movement Type File |
	 +===========================*/
#define	ASMT_NO_FIELDS	3

	struct dbview	asmt_list [ASMT_NO_FIELDS] =
	{
		{"asmt_co_no"},
		{"asmt_type_code"},
		{"asmt_desc"}
	};

	struct tag_asmtRecord
	{
		char	co_no [3];
		char	type_code [5];
		char	desc [41];
	}	asmt_rec;


	/*=====================+
	 | Asset Master File |
	 +=====================*/
#define	ASMR_NO_FIELDS	7 

	struct dbview	asmr_list [ASMR_NO_FIELDS] =
	{
		{"asmr_co_no"},
		{"asmr_br_no"},
		{"asmr_ass_group"},
		{"asmr_ass_no"},
		{"asmr_serial_no"},
		{"asmr_hhas_hash"},
		{"asmr_hham_hash"}
	};

	struct tag_asmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	ass_group[6];
		char	ass_no[6];
		char	serial_no [26];
		long	hhas_hash;
		long	hham_hash;
	}	asmr_rec;


	/*=======================+
	 | Asset Movement File |
	 +=======================*/
#define	ASMV_NO_FIELDS	5

	struct dbview	asmv_list [ASMV_NO_FIELDS] =
	{
		{"asmv_hhar_hash"},
		{"asmv_line_no"},
		{"asmv_move_code"},
		{"asmv_move_desc"},
		{"asmv_serial_no"}
	};

	struct tag_asmvRecord
	{
		long	hhas_hash;
		int		line_no;
		char	move_code [5];
		char	move_desc [41];
		char	serial_no [26];
	}	asmv_rec;


	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*asmt  = "asmt";
	char	*asmr  = "asmr";
	char	*asmv  = "asmv";


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

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars[] =
{
	{ 1, LIN, "type_code",	 4, 35, CHARTYPE,
		"UUUU", "          ",
		" ", "", "Asset Movement Type Code       : ", "Enter Asset Movement Type Code  [SEARCH] for valid codes",
		NE, NO,  JUSTLEFT, "", "", asmt_rec.type_code },
	{ 1, LIN, "type_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Movement Description     : ", "Enter Asset Movement Type Description",
		YES, NO, JUSTLEFT, "", "", asmt_rec.desc },
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
static void srch_asmt (char *key_val);
static void update (void);

int spec_valid (int field);
int heading (int scn);

/*#ifdef SEL_DELETE*/
static BOOL	FrmtDelOk (char *badFileName);
/*#endif*/

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

	open_rec (asmt, asmt_list, ASMT_NO_FIELDS, "asmt_id_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (asmv, asmv_list, ASMV_NO_FIELDS, "asmv_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose  (asmt);
	abc_fclose  (asmr);
	abc_fclose  (asmv);
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
			srch_asmt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strlen(clip(asmt_rec.type_code)))
			return(1);

		strcpy (asmt_rec.co_no, comm_rec.tco_no);
		cc = find_rec (asmt, &asmt_rec, EQUAL, "u");
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

/*----------------------------------+
| Search Asset Movement Type File |
+----------------------------------*/
static void
srch_asmt (
 char*              key_val)
{
	struct tag_asmtRecord asmt_bak;

	memcpy (&asmt_bak, &asmt_rec, sizeof asmt_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (asmt_rec.co_no, comm_rec.tco_no);
	strcpy (asmt_rec.type_code, key_val);

	cc = find_rec (asmt, &asmt_rec, GTEQ, "r");

	while (!cc &&
		   !strcmp (asmt_rec.co_no, comm_rec.tco_no) &&
		   !strncmp (asmt_rec.type_code, key_val, strlen(key_val)))
	{
		cc = save_rec (asmt_rec.type_code, asmt_rec.desc);
		if (cc)
			break;

		cc = find_rec (asmt, &asmt_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*---------------------+
		| Read selected record |
		+---------------------*/
		strcpy (asmt_rec.co_no, comm_rec.tco_no);
		strcpy (asmt_rec.type_code, temp_str);
		cc = find_rec (asmt, &asmt_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, asmt, "DBFIND");
	}

	if (cc)
		memcpy (&asmt_rec, &asmt_bak, sizeof asmt_rec);
}

/*==================
| Updated records. |
==================*/
static void
update (void)
{
	strcpy (asmt_rec.co_no, comm_rec.tco_no);
	if (newCode)
	{
		cc = abc_add (asmt, &asmt_rec);
		if (cc) 
			file_err(cc, asmt, "DBADD");
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
				cc = abc_update (asmt, &asmt_rec);
				if (cc) 
					file_err (cc, asmt, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (asmt);
				exitLoop = TRUE;
				break;

			case SEL_DELETE :
			{
				char	badFileName[7];

				if (FrmtDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (asmt);
					if (cc)
						file_err (cc, asmt, "DBUPDATE");
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

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (asmt);
}

/*===========================
| Check whether it is OK to |
| delete the asmt record.   |
| Files checked are : asmv  |
|                           |
===========================*/
#ifdef SEL_DELETE
static BOOL
FrmtDelOk (
 char*              badFileName)
{
	memset (&asmr_rec, 0, sizeof asmr_rec);
	strcpy (asmr_rec.co_no, comm_rec.tco_no);	
	strcpy (asmr_rec.br_no, comm_rec.test_no);
	cc = find_rec (asmr, &asmr_rec, GTEQ, "r");
    while (!cc && !strcmp (asmr_rec.co_no, comm_rec.tco_no)
			   && !strcmp (asmr_rec.br_no, comm_rec.test_no))
	{
		memset (&asmv_rec, 0, sizeof asmv_rec);
		asmv_rec.hhas_hash = asmr_rec.hhas_hash;
		cc = find_rec (asmv, &asmv_rec, GTEQ, "r");
		while (!cc && (asmv_rec.hhas_hash == asmr_rec.hhas_hash)) 
		{
			if (!cc) 
				if (!strcmp(asmt_rec.type_code,asmv_rec.move_code))
				{
					strcpy (badFileName, "asmv");
					return (EXIT_SUCCESS);
				}
			cc = find_rec (asmv, &asmv_rec, NEXT, "r");
		}
		cc = find_rec (asmr, &asmr_rec, NEXT, "r");
		
	}
	return (EXIT_FAILURE);
	
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

		centre_at (0,80, ML("%R Asset Movement Type Maintenance "));
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

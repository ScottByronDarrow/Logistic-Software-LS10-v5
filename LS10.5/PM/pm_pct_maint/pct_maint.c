/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( prpt_inp.c )                                     |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmpr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 16/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (27/12/95)      | Modified by : Jojo M. Gatchalian |
|  Date Modified : (03/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (08/09/97)      | Modified by : Marnie I. Organo   |
|  Date Modified : (16/10/97)      | Modified by : Elena B. Cuaresma. |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                	  |
|  (27/12/95)    : 				   - Schema Changes.                  |
|  (03/01/96)    : 				   - Allow to Delete a Record.        |
|  (08/09/97)    : Modified for Multilingual Conversion.              |
|  (17/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: pct_maint.c,v $
| Revision 5.3  2002/07/25 11:17:29  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:00  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/16 02:56:13  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.10  1999/10/01 07:49:01  scott
| Updated for standard function calls.
|
| Revision 1.9  1999/09/29 10:11:46  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 04:23:19  scott
| Updated from Ansi Project.
|
| Revision 1.7  1999/06/17 07:54:51  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pct_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_pct_maint/pct_maint.c,v 5.3 2002/07/25 11:17:29 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_pm_mess.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

#define SLEEP_TIME	3

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

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	4

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_logname"},
		{"exsf_salesman"},
	};

	struct tag_exsfRecord
	{
		char	co_no [3];
		char	sman_no [3];
		char	logname [15];
		char	sman_name [41];
	} exsf_rec;

	/*===================================+
	 | Project Monitoring Prospects File |
	 +===================================*/
#define	PMPR_NO_FIELDS	8

	struct dbview	pmpr_list [PMPR_NO_FIELDS] =
	{
		{"pmpr_prospect_no"},
		{"pmpr_name"},
		{"pmpr_adr1"},
		{"pmpr_adr2"},
		{"pmpr_adr3"},
		{"pmpr_phone_no"},
		{"pmpr_fax_no"},
		{"pmpr_sman_code"}
	};

	struct tag_pmprRecord
	{
		char	prospect_no [7];
		char	name [41];
		char	adr1 [41];
		char	adr2 [41];
		char	adr3 [41];
		char	phone_no [16];
		char	fax_no [16];
		char	sman_code [3];
	}	pmpr_rec;

	char	*data  = "data",
			*exsf  = "exsf",
			*pmpr  = "pmpr";


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

static	struct	var	vars [] =
{
	{ 1, LIN, "prospect_number",	 3, 23, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "  Prospect Number   : ", 
		"Enter Prospect Number [SEARCH] for references",
		NE, NO,  JUSTLEFT, "", "", pmpr_rec.prospect_no },
	{ 1, LIN, "prospect_name",	 4, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Prospect Name     : ", "Enter Prospect Name",
		YES, NO, JUSTLEFT, "", "", pmpr_rec.name },
	{ 1, LIN, "prospect_addr1",	 5, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#1         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpr_rec.adr1 },
	{ 1, LIN, "prospect_addr2",	 6, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#2         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpr_rec.adr2 },
	{ 1, LIN, "prospect_addr3",	 7, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#3         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpr_rec.adr3 },
	{ 1, LIN, "prospect_phone",	 8, 23, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "  Tel. nos.         : ", "Enter Telephone Numbers",
		NO, NO, JUSTLEFT, "", "", pmpr_rec.phone_no },
	{ 1, LIN, "prospect_fax",	 9, 23, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "  Fax no.           : ", "Enter Fax Number",
		NO, NO, JUSTLEFT, "", "", pmpr_rec.fax_no },
	{ 1, LIN, "prospect_sman",	 10, 23, CHARTYPE,
		"AA", "          ",
		" ", "", "  Salesman Number   : ", "Enter Salesman Number[SEARCH]",
		YES, NO, JUSTLEFT, "", "", pmpr_rec.sman_code },
	{ 1, LIN, "prospect_sman_name",	 10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", exsf_rec.sman_name },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};


static BOOL	newCode = FALSE;

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	SrchPmpr		(char *);
void	update			(void);
BOOL	PrptDelOk		(char *);
int		heading			(int);
void	SrchExsf		(char *);


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
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
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpr);
	abc_fclose (exsf);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("prospect_number"))
	{
		if (SRCH_KEY)
		{
			SrchPmpr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strlen(clip(pmpr_rec.prospect_no)))
			return(1);

		cc = find_rec (pmpr, &pmpr_rec, EQUAL, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;

			strcpy (exsf_rec.co_no, comm_rec.tco_no);
			sprintf (exsf_rec.sman_no, "%-2.2s", pmpr_rec.sman_code);
			cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
			if (cc)
				memset (&exsf_rec, 0, sizeof exsf_rec);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Prospect Address1. |
	-----------------------------*/
	if (LCHECK ("prospect_addr1"))
    {
		if (dflt_used && prog_status == ENTRY)	
			skip_entry = goto_field (field, label ("prospect_phone"));
    }

	/*-----------------------------
	| Validate Prospect Address2. |
	-----------------------------*/
	if (LCHECK ("prospect_addr2"))
    {
		if (dflt_used && prog_status == ENTRY)	
			skip_entry = goto_field (field, label ("prospect_phone"));
    }

	/*-----------------------------
	| Validate Prospect Salesman. |
	-----------------------------*/
	if (LCHECK("prospect_sman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (pmpr_rec.sman_code, "  "))
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);	
			return (EXIT_FAILURE);
		}
	
		strcpy (exsf_rec.co_no,comm_rec.tco_no);
		strcpy (exsf_rec.sman_no, pmpr_rec.sman_code);
		cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpr (
 char *	key_val)
{
	work_open();
	save_rec("#Code   ","#Description");

	strcpy (pmpr_rec.prospect_no, key_val);

	cc = find_rec (pmpr, &pmpr_rec, GTEQ, "r");

	while (!cc && !strncmp (pmpr_rec.prospect_no, key_val, strlen(key_val)))
	{
		cc = save_rec (pmpr_rec.prospect_no, pmpr_rec.name);
		if (cc)
			break;

		cc = find_rec (pmpr, &pmpr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpr_rec.prospect_no, temp_str);
		cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpr, "DBFIND");
	}
}

/*==================
| Updated records. |
==================*/
void
update (
 void)
{
	if (newCode)
	{
		strcpy(err_str, ML(mlStdMess035));
		print_mess (err_str); sleep(2);
		cc = abc_add (pmpr, &pmpr_rec);
		if (cc) 
			file_err(cc, pmpr, "DBADD");
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
				strcpy(err_str, ML(mlStdMess035));
				print_mess (err_str); sleep(2);
				cc = abc_update (pmpr, &pmpr_rec);
				if (cc) 
					file_err (cc, pmpr, "DBUPDATE");
				abc_unlock (pmpr);
				exitLoop = TRUE;
				clear_mess ();
				break;
	
			case SEL_IGNORE :
				abc_unlock (pmpr);
				exitLoop = TRUE;
				break;

			case SEL_DELETE :
			{
				char badFileName [7];

				if (PrptDelOk (badFileName))
				{
					print_mess (ML(mlStdMess014)); sleep(2);
					clear_mess ();
					cc = abc_delete (pmpr);
					if (cc)
						file_err (cc, pmpr, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,ML(mlPmMess007),
						badFileName);

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
	abc_unlock (pmpr);
}

/*===========================
| Check whether it is OK to |
| delete the pmpr record.   |
| Files checked are :       |
|                           |
===========================*/
BOOL
PrptDelOk (
 char *	badFileName)
{
	return (TRUE);
}

/*===========================
| edit () callback function |
===========================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (mlPmMess014), 30, 0,1);
		move (0, 1); line (80);

		box (0, 2, 80, 8);

		move (0, 1); line (79);
		move (1, 22); line(79);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*============================+
| Search Salesman Master File |
=============================*/
void
SrchExsf (
 char *	key_val)
{
	work_open ();
	save_rec ("#Salesman", "#Salesman's Name");
	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	sprintf (exsf_rec.sman_no, "%-2.2s", key_val);
	cc = find_rec ("exsf", &exsf_rec, GTEQ, "r");

	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.tco_no) &&
				  !strncmp (exsf_rec.sman_no,key_val, strlen(key_val)))
	{
		cc = save_rec (exsf_rec.sman_no, exsf_rec.sman_name);
		if (cc)
			break;

		cc = find_rec("exsf", &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	sprintf (exsf_rec.sman_no, "%-2.2s", temp_str);
	sprintf (pmpr_rec.sman_code, "%-2.2s", temp_str);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, "exsf", "DBFIND");
}


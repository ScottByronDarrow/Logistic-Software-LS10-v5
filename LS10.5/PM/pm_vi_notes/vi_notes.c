/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( vi_notes.c  )                                    |
|  Program Desc  : ( Project Note Pad Input Program.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmnt,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmnt,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 29/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (04/01/96)      | Modified  by  : Joy G. Medel     |
|  Date Modified : (19/01/96)      | Modified  by  : Joy G. Medel     |
|  Date Modified : (12/09/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (14/10/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (16/10/97)      | Modified  by  : Rowena Maandig   |
|  Date Modified : (17/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (04/01/96) -      Rebuild structure of pmnt.       |
|  Comments      : (19/01/96) -      Fixed the filename format.       |
|  Comments      : (12/09/97) -      Modified for Multilingual        | 
|                                    Conversion.                      |
|  Comments      : (14/10/97) -      Change error message to ML#.     | 
|  Comments      : (16/10/97) -      Updated proj_no/inv_no from 6 to |
|                                     8 chars.                        | 
|  Comments      : (17/09/1999) -    Ported to ANSI standards.        |
|                                                                     |
| $Log: vi_notes.c,v $
| Revision 5.3  2002/07/25 11:17:30  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:57  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:07  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:58  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/16 02:56:14  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.11  1999/10/01 07:49:02  scott
| Updated for standard function calls.
|
| Revision 1.10  1999/09/29 10:11:50  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 04:23:22  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/06/17 07:54:54  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: vi_notes.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_vi_notes/vi_notes.c,v 5.3 2002/07/25 11:17:30 scott Exp $";

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_pm_mess.h>

	/*==========================
	| Special fields and flags. |
	===========================*/
	int	new_note = 0;
	int part_printed = FALSE;
	int pm_margin = 10;

	FILE *	fout;

	char 	pm_dir [51];


	/*=========================
	| Common Record Structure |
	=========================*/
#define COMM_NO_FIELDS	5

	struct dbview comm_list [COMM_NO_FIELDS] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	struct {
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tes_no [3];
		char	tes_name [41];
	} comm_rec;

	/*===================================+
	 | Project Monitoring Project Notes. |
	 +===================================*/
#define	PMNT_NO_FIELDS	3

	struct dbview	pmnt_list [PMNT_NO_FIELDS] =
	{
		{"pmnt_proj_no"},
		{"pmnt_cont_no"},
		{"pmnt_filename"}
	};

	struct tag_pmntRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	filename [13];
	}	pmnt_rec;

	/*=========================================+
	 | Project Monitoring Project Master file. |
	 +=========================================*/
#define	PMPM_NO_FIELDS	2

	struct dbview	pmpm_list [PMPM_NO_FIELDS] =
	{
		{"pmpm_proj_no"},
		{"pmpm_title"}
	};

	struct tag_pmpmRecord
	{
		char	proj_no [9];
		char	title [41];
	}	pmpm_rec;

	/*===================================+
	 | Project Monitoring Prospects File |
	 +===================================*/
#define	PMPR_NO_FIELDS	2

	struct dbview	pmpr_list [PMPR_NO_FIELDS] =
	{
		{"pmpr_prospect_no"},
		{"pmpr_name"}
	};

	struct tag_pmprRecord
	{
		char	prospect_no [7];
		char	name [41];
	}	pmpr_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	4

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"}
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		char	dbt_name [41];
	}	cumr_rec;

	/*=========================+
	 | Project-Contractor File |
	 +=========================*/
#define	PMPC_NO_FIELDS	6

	struct dbview	pmpc_list [PMPC_NO_FIELDS] =
	{
		{"pmpc_proj_no"},
		{"pmpc_cont_no"},
		{"pmpc_type"},
		{"pmpc_parent_no"},
		{"pmpc_area"},
		{"pmpc_sman_code"}
	};

	struct tag_pmpcRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	type [2];
		char	parent_no [9];
		char	area [7];
		char	sman_code [3];
	}	pmpc_rec;

int		clr_scn2;
int		clear_ok;
int		first_time = TRUE;

char	filename [13];

char	*data	= "data";
char	*pmnt	= "pmnt";
char	*cumr	= "cumr";
char	*pmpm	= "pmpm";
char	*pmpr	= "pmpr";
char	*pmpc	= "pmpc";

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	dummy [11];
	char	comments [121];
	char	proj_no [9];
	char	cust_no [7];
	char	cust_name [41];
	char	title [41];
} local_rec;

	char	LineBlank [121];

		
static struct	var vars [] =
{
	{1, LIN, "proj_ref",	 3, 19, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Project Code      :", "Enter Project Code. [SEARCH]",
		NE, NO,  JUSTLEFT, "", "", local_rec.proj_no},
	{1, LIN, "proj_title",	 3, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.title},
	{1, LIN, "cust_no", 4, 19, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Contractor Number :", "Enter Contractor Number. [SEARCH]",
		NO, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "cust_name", 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.cust_name},

	{0, LIN, "", 0, 0, INTTYPE,
	   "A", "          ",
	   " ", "",  "dummy", " ",
	   YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		(void);
void	CloseDB	(void);
int		spec_valid	(int);
void	vi_file		(void);
void	SrchPmpm	(char *);
void	SrchPmpc	(char *);
int		UpdateData	(void);
void	prn_co		(void);
int		heading		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	prog_exit = 0;
	while (prog_exit == 0)
	{
		clear_ok = TRUE;
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars (1);

		clr_scn2 = TRUE;
		heading (1);
		clr_scn2 = FALSE;

		scn_display (1);
		entry (1);
		if (restart)
			continue;

		vi_file ();

		if (restart)
			continue;

	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}


/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (pmnt, pmnt_list, PMNT_NO_FIELDS, "pmnt_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose  (pmnt);
	abc_fclose  (cumr);
	abc_fclose  (pmpm);
	abc_fclose  (pmpr);
	abc_fclose  (pmpc);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------
	| Validate Project Code    |
	--------------------------*/
	if (LCHECK ("proj_ref"))
	{
		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pmpm_rec.proj_no, local_rec.proj_no); 
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (!cc)
			strcpy (local_rec.title, pmpm_rec.title); 
		else
		{
			/*Project Reference Not On File.*/
			print_mess (ML(mlPmMess008));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("proj_title");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Project Contractor |
	-----------------------------*/
	if (LCHECK ("cust_no"))
	{
		if (SRCH_KEY)
		{
			SrchPmpc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
		strcpy (pmpc_rec.cont_no, local_rec.cust_no); 
		cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
		if (!cc)
		{
			if (!strcmp (pmpc_rec.type, "C"))
			{
				strcpy (cumr_rec.co_no,  comm_rec.tco_no);
				strcpy (cumr_rec.est_no, comm_rec.tes_no);
				strcpy (cumr_rec.dbt_no, pmpc_rec.cont_no);
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (!cc)
					strcpy (local_rec.cust_name, cumr_rec.dbt_name);
			}
			else
			{
				strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no); 
				cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
				if (!cc)
					strcpy (local_rec.cust_name, pmpr_rec.name);
			}
		}
		else
		{
			/*Contractor Number Not On File*/
			print_mess (ML (mlPmMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pmnt_rec.proj_no, local_rec.proj_no);
		strcpy (pmnt_rec.cont_no, local_rec.cust_no);
		cc = find_rec (pmnt, &pmnt_rec, EQUAL, "r");
		if (cc)
			new_note = 1;
		else
			new_note = 0;
	}
	return (EXIT_SUCCESS);
}

void
vi_file (
 void)
{
	char	cmd [100];
	char	directory [101];
	int		day,
			month,	
			year;
	long	currentDateLong;
	char	hoursMinutes [6];
	char *	sptr;

	sptr = getenv ("PROG_PATH");

	crsr_on ();
	sprintf (directory, "%s/BIN/PNT", (sptr == (char *)0) ? "/usr/ver9" : sptr);

	if (new_note)
	{
		/*----------------------------------
		| Format for file is YYYYMMDDHHMM. |
		----------------------------------*/
		currentDateLong	=	TodaysDate ();
		DateToDMY (currentDateLong, &day, &month, &year);
		strcpy (hoursMinutes, TimeHHMM());
		sprintf (filename, "%d%d%d%-2.2s%-2.2s", 
					year, 
					month, 
					day,
					hoursMinutes,
					hoursMinutes + 3);
	}
	else
	{
		strcpy (pmnt_rec.proj_no, local_rec.proj_no);
		strcpy (pmnt_rec.cont_no, local_rec.cust_no);
		cc = find_rec (pmnt, &pmnt_rec, EQUAL, "r");
		if (!cc)
			strcpy (filename, pmnt_rec.filename);
	}

	sys_exec ("ON");
	sprintf (cmd, "vi -w20 %s/%-12.12s", directory, clip (filename));
	cc = sys_exec (cmd);
	if (!cc)
		UpdateData ();
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpm (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code   ", "#Description");
	strcpy (pmpm_rec.proj_no, key_val); 
	cc = find_rec (pmpm, &pmpm_rec, GTEQ, "r");
	while (!cc && !strncmp (pmpm_rec.proj_no, key_val, strlen (key_val)))
	{
		cc = save_rec (pmpm_rec.proj_no, pmpm_rec.title);
		if (cc)
			break;

		cc = find_rec (pmpm, &pmpm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpm_rec.proj_no, temp_str);
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpm, "DBFIND");
	}
}

/*==============================+
| Search Contractor Master File |
===============================*/
void
SrchPmpc (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code   ","#Description");
	strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
	strcpy (pmpc_rec.cont_no, key_val); 
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc && !strcmp (pmpc_rec.proj_no, local_rec.proj_no) &&
			!strncmp (pmpc_rec.proj_no, key_val, strlen(key_val)))
	{
		if (!strcmp (pmpc_rec.type, "C"))
		{
			strcpy (cumr_rec.co_no,  comm_rec.tco_no);
			strcpy (cumr_rec.est_no, comm_rec.tes_no);
			strcpy (cumr_rec.dbt_no, pmpc_rec.cont_no);
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (!cc)
				cc = save_rec (cumr_rec.dbt_no, cumr_rec.dbt_name);
		}
		else
		{
			strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no); 
			cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
			if (!cc)
				cc = save_rec (pmpr_rec.prospect_no, pmpr_rec.name);
		}
		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
		strcpy (pmpc_rec.cont_no, key_val); 
		cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpc, "DBFIND");
	}
}

/*===================
| Update all files. |
===================*/
int
UpdateData (
 void)
{
	clear ();

	if (new_note == 1) 
	{
		rv_pr (ML (mlPmMess023), 2, 2, 1); sleep(1);
		strcpy (pmnt_rec.proj_no, local_rec.proj_no);
		strcpy (pmnt_rec.cont_no, local_rec.cust_no);
		strcpy (pmnt_rec.filename, filename);
		cc = abc_add (pmnt, &pmnt_rec);
		if (cc) 
			sys_err ("Error in pmnt During (DBADD)", cc, PNAME);

		cc = find_rec (pmnt, &pmnt_rec, COMPARISON, "r");
		if (cc)
			return (EXIT_FAILURE);
	}
	else
	{
		rv_pr (ML (mlStdMess035), 2, 2, 1); sleep (sleepTime);
		strcpy (pmnt_rec.proj_no, local_rec.proj_no);
		strcpy (pmnt_rec.cont_no, local_rec.cust_no);
		strcpy (pmnt_rec.filename, filename);
		cc = abc_update (pmnt, &pmnt_rec);
		if (cc)
			file_err (cc, pmnt, "DBUPDATE");
	}
	strcpy (filename, "            ");
	return (EXIT_SUCCESS);
}

/*========================
| Print Company Details. |
========================*/
void
prn_co (
 void)
{
	move (0, 20);
	line (80);
	strcpy (err_str, ML (mlStdMess038));
	print_at (21, 0, err_str, comm_rec.tco_no, comm_rec.tco_name);
	move (0, 22);
	line (80);
}



/*================
| Print Heading. |
================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		/* swide (); */
		if (clear_ok)
			clear ();
		
		rv_pr (ML (mlPmMess023), 22, 0, 1);
		
		move (0, 1);
		line (80);

		box (0, 2, 80, 2);
		if (scn == 1)
		{
			if (clr_scn2)
				init_vars (2);	

			if (clear_ok)
			{
				scn_set (2);
				scn_display (2);
			}
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}
		scn_set (scn);

		prn_co ();
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

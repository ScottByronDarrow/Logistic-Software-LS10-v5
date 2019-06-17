/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pmpm_inp.c )                                     |
|  Program Desc  : ( Project Monitoring Program.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpm,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmpm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 16/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (27/12/95)      | Modified by : Jojo M. Gatchalian |
|  Date Modified : (28/12/95)      | Modified by : Joy G. Medel       |
|  Date Modified : (16/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (16/10/97)      | Modified by : Roanna Marcelino.  |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                	  |
|  (27/12/95)    : 				   - Schema Changes.                  |
|  (28/12/95)    :                 - Estimated costs must display     |
|                :                   comma separator for thousands    |
|                :                   and have two decimal places.     |
|                :                 - Salesman Number Search.          |
|  (16/01/96)    :                 - Added new INACTIVE status.       |
|  (16/10/97)    :                 - Updated to change pmpm_proj_no   |
|                                    from 6 to 8 chars.               |
|  (17/09/1999)  :                 - Ported to ANSI standards.        |
|                                                                     |
| $Log: prjt_inp.c,v $
| Revision 5.3  2002/07/25 11:17:30  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:54  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:05  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:57  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/16 02:56:14  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.11  1999/10/01 07:49:01  scott
| Updated for standard function calls.
|
| Revision 1.10  1999/09/29 10:11:50  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 04:23:21  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/06/17 07:54:53  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: prjt_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_prjt_inp/prjt_inp.c,v 5.3 2002/07/25 11:17:30 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#if 0
#define	SEL_DELETE	2
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

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	5

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_logname"},
		{"exsf_salesman"},
		{"exsf_area_code"},
	};

	struct tag_exsfRecord
	{
		char	co_no [3];
		char	sman_no [3];
		char	logname [15];
		char	sman_name [41];
		char	area_code [3];
	} exsf_rec;


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

	/*==========================================+
	 | Project Monitoring Prospect Master file. |
	 +==========================================*/
#define	PMPM_NO_FIELDS	14

	struct dbview	pmpm_list [PMPM_NO_FIELDS] =
	{
		{"pmpm_proj_no"},
		{"pmpm_title"},
		{"pmpm_date_entrd"},
		{"pmpm_date_start"},
		{"pmpm_date_end"},
		{"pmpm_consultant"},
		{"pmpm_contact_name"},
		{"pmpm_dl_adr1"},
		{"pmpm_dl_adr2"},
		{"pmpm_dl_adr3"},
		{"pmpm_phone_no"},
		{"pmpm_fax_no"},
		{"pmpm_mtrl_cost"},
		{"pmpm_status"}
	};

	struct tag_pmpmRecord
	{
		char	proj_no [9];
		char	title [41];
		long	date_entrd;
		long	date_start;
		long	date_end;
		char	consultant [41];
		char	contact_name [21];
		char	dl_adr1 [41];
		char	dl_adr2 [41];
		char	dl_adr3 [41];
		char	phone_no [16];
		char	fax_no [16];
		double	mtrl_cost;
		char	status [2];
	}	pmpm_rec;

	char	*data  = "data",
			*exsf  = "exsf",
			*pmpm  = "pmpm";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
} local_rec;


static	struct	var	vars [] =
{
	{ 1, LIN, "proj_ref",	 3, 23, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "  Project Code      : ", "Enter Project Code [SEARCH] for references",
		NE, NO,  JUSTLEFT, "", "", pmpm_rec.proj_no },
	{ 1, LIN, "proj_title",	 4, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Project Title     : ", "Enter Project Title",
		YES, NO, JUSTLEFT, "", "", pmpm_rec.title },
	{ 1, LIN, "proj_date_entrd", 6, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "  Date Entered      : ", "Enter Start Date",
		YES, NO, JUSTLEFT, "", "", (char *) &pmpm_rec.date_entrd },
	{ 1, LIN, "proj_sdate",	 7, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "  Start Date        : ", "Enter Start Date",
		YES, NO, JUSTLEFT, "", "", (char *) &pmpm_rec.date_start },
	{ 1, LIN, "proj_edate",	 8, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "  End Date          : ", "Enter End Date",
		YES, NO, JUSTLEFT, "", "", (char *) &pmpm_rec.date_end },
	{ 1, LIN, "proj_consultant", 10, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Main Consultant   : ", "Enter Project Consultant",
		YES, NO, JUSTLEFT, "", "", pmpm_rec.consultant },
	{ 1, LIN, "proj_contact",	 11, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Contact Name      : ", "Enter Contact Name",
		YES, NO, JUSTLEFT, "", "", pmpm_rec.contact_name },
	{ 1, LIN, "proj_addr1",	 12, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#1         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpm_rec.dl_adr1 },
	{ 1, LIN, "proj_addr2",	 13, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#2         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpm_rec.dl_adr2 },
	{ 1, LIN, "proj_addr3",	 14, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  Address#3         : ", "Enter Address",
		NO, NO, JUSTLEFT, "", "", pmpm_rec.dl_adr3 },
	{ 1, LIN, "proj_phone",	 15, 23, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "  Tel. nos.         : ", "Enter Telephone Numbers",
		NO, NO, JUSTLEFT, "", "", pmpm_rec.phone_no },
	{ 1, LIN, "proj_fax",	 16, 23, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "  Fax no.           : ", "Enter Fax Number",
		NO, NO, JUSTLEFT, "", "", pmpm_rec.fax_no },
	{ 1, LIN, "proj_est_cost",	 18, 23, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "  Estimated Cost    : ", "Enter Estimated Value of Materials",
		NO, NO, JUSTRIGHT, "0.00", "999999999.99", (char *) &pmpm_rec.mtrl_cost },
	{ 1, LIN, "proj_status",19, 23, CHARTYPE,
		"U", "          ",
		" ", "", "  Project Status    : ", "Enter Project Status: (P)ending, (A)ctive or (I)nactive, (W)on or (L)ost",
		YES, NO, JUSTLEFT, "PAIWL", "", pmpm_rec.status },
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
void	SrchPmpm		(char *);
void	update			(void);
int		heading			(int);
int		SrchExsf		(char *);

#ifdef SEL_DELETE
BOOL	FrtyDelOk		(char *);
BOOL	PrjtDelOk		(char *);
#endif



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

	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpm);
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
	if (LCHECK ("proj_ref"))
	{
		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strlen (clip (pmpm_rec.proj_no)))
			return (EXIT_FAILURE);

		cc = find_rec (pmpm, &pmpm_rec, EQUAL, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Project Address1. |
	----------------------------*/
	if (LCHECK ("proj_addr1"))
    {
		if (dflt_used && prog_status == ENTRY)	
			skip_entry = goto_field (field, label ("proj_phone"));
    }

	/*----------------------------
	| Validate Project Address2. |
	----------------------------*/
	if (LCHECK ("proj_addr2"))
    {
		if (dflt_used && prog_status == ENTRY)	
			skip_entry = goto_field (field, label ("proj_phone"));
    }
	return (EXIT_SUCCESS);
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

	while (!cc && !strncmp (pmpm_rec.proj_no, key_val, strlen(key_val)))
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
		{
			
			file_err (cc, pmpm, "DBFIND");
		}
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
		cc = abc_add (pmpm, &pmpm_rec);
		if (cc) 
			file_err (cc, pmpm, "DBADD");
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
				cc = abc_update (pmpm, &pmpm_rec);
				if (cc) 
					file_err (cc, pmpm, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (pmpm);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char badFileName [7];

				if (PrjtDelOk (badFileName))
				{
					clear_mess ();
					cc = abc_delete (pmpm);
					if (cc)
						file_err (cc, pmpm, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,
							 "Matching Document Records Found in %-4.4s, Document Record Not Deleted",
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
	abc_unlock (pmpm);
}

/*===========================
| Check whether it is OK to |
| delete the pmpm record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
BOOL
FrtyDelOk (
 char *	badFileName)
{
	return (TRUE);
}

BOOL
PrjtDelOk (
 char *	badFileName)
{
	return (TRUE);
}
#endif

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
		centre_at (0, 80, "%R Projects Maintenance ");
		move (0, 1); line (80);

		box (0, 2, 80, 17);

		move (0, 1); line (79);
		move (1, 5); line (79);
		move (1, 9); line (79);
		move (1, 17); line (79);
		move (1, 22); line(79);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
SrchExsf (
 char *	key_val)
{
	work_open ();
	save_rec ("#CD", "#Salesman's Name");
	strcpy (exsf_rec.co_no,comm_rec.tco_no);
	sprintf (exsf_rec.sman_no, "%-2.2s", key_val);
	cc = find_rec ("exsf", &exsf_rec, GTEQ, "r");

	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.tco_no) &&
				  !strncmp (exsf_rec.sman_no, key_val, strlen (key_val)))
	{
		cc = save_rec (exsf_rec.sman_no, exsf_rec.sman_name);
		if (cc)
			break;

		cc = find_rec ("exsf", &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	sprintf (exsf_rec.sman_no, "%-2.2s", temp_str);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");

	return (EXIT_SUCCESS);
}


/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_act_maint.c )                                 |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpm, pmat, pmpr, cumr,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmat,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy Medel       | Date Written  : 23/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (02/01/95)      | Modified By   : Rommel S. Maldia |
|  Date Modified : (04/09/97)      | Modified By   : Jiggs A Veloz.   |
|  Date Modified : (14/10/97)      | Modified By   : Rowena S Maandig |
|  Date Modified : (17/09/1999)    | Modified by   : Ramon A. Pacheco |
|  Comments      :                                                	  |
|  (02/01/95)    :                 - Bug Fixes.                       |
|                :                                                    |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 11.      |
|  (16/10/97)    : SEL Updated proj_no/inv_no from 6 to 8 char.       |
|  (17/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
| $Log: act_maint.c,v $
| Revision 5.3  2002/07/25 11:17:29  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:01  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:53  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/11/16 02:56:13  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.12  1999/10/20 02:06:55  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/09/29 10:11:45  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/24 04:23:19  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/06/17 07:54:51  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: act_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_act_maint/act_maint.c,v 5.3 2002/07/25 11:17:29 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_pm_mess.h>
#include <ml_std_mess.h>

typedef int BOOL;

#define	SEL_UPDATE		0
#define	SEL_IGNORE		1
#define	SEL_DELETE		2
#define	SEL_DEFAULT		99
#define SLEEP_TIME		3

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"} 
	};

	int	comm_no_fields = 8;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char 	tco_short [16];
		char	test_no [3];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_short [10];
	} comm_rec;

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

	/*==================================+
	 | Project Monitoring Action Table. |
	 +==================================*/
#define	PMAT_NO_FIELDS	5

	struct dbview	pmat_list [PMAT_NO_FIELDS] =
	{
		{"pmat_proj_no"},
		{"pmat_cont_no"},
		{"pmat_act_date"},
		{"pmat_act_details"},
		{"pmat_next_act_date"}
	};

	struct tag_pmatRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		long	act_date;
		char	act_details [81];
		long	next_act_date;
	}	pmat_rec;

	/*==========================================+
	 | Project Monitoring Prospect Master file. |
	 +==========================================*/
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
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	5

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"}
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		long	hhcu_hash;
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
		char	parent_no [7];
		char	area [3];
		char	sman_code [3];
	}	pmpc_rec;

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

	char	*data  = "data",
			*pmat  = "pmat",
			*pmpm  = "pmpm",
			*cumr  = "cumr",
			*pmpc  = "pmpc",
			*pmpr  = "pmpr";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	title [41];
	char 	cust_no [7];
	char	cust_name [41];
	long  	act_date;
	char  	systemDate [11];
	char	act_details [81];
	long	next_act_date;
	char	dummy [11];
} local_rec;


static	struct	var	vars [] =
{
	{1, LIN, "proj_ref",	 4, 24, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", " Project Code      : ", "Enter Project Code. [SEARCH]",
		NE, NO,  JUSTLEFT, "", "", pmat_rec.proj_no},
	{1, LIN, "proj_title",	 4, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.title},
	{1, LIN, "cust_no", 5, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " Contractor Number : ", "Enter Contractor Number. [SEARCH]",
		NO, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "cust_name", 5, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.cust_name},
	{1, LIN, "act_date", 7, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " Date              : ", "Enter Date of Action",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.act_date},
	{1, LIN, "act_details", 8, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Action Details    : ", "Enter Details of Action",
		YES, NO, JUSTLEFT, "", "", local_rec.act_details},
	{1, LIN, "next_act_date",	 9, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " Next Action Date  : ", "Enter Next Action Date",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.next_act_date},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
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
void	SrchPmpc		(char *);
void	update			(void);
int		heading			(int);


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
	clear ();
	swide ();
	set_masks ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
		
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
		init_ok		= TRUE;
		init_vars (1);
		clear();

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
	open_rec (pmat, pmat_list, PMAT_NO_FIELDS, "pmat_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpm);
	abc_fclose (pmat);
	abc_fclose (cumr);
	abc_fclose (pmpc);
	abc_fclose (pmpr);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-----------------------------
	| Validate Project Reference. |
	-----------------------------*/
	if (LCHECK ("proj_ref"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				strcpy (pmat_rec.proj_no, pmpm_rec.proj_no);
				DSP_FLD("proj_ref");
				return(0);
			}
		}

		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pmpm_rec.proj_no, pmat_rec.proj_no);
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------------------------
			| Reference Number %s is not on file. |
			-------------------------------------*/
			print_mess( ML(mlStdMess191) );
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.title, pmpm_rec.title);
		DSP_FLD ("proj_title");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Customer Number. |
	---------------------------*/
	if (LCHECK ("cust_no"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				strcpy (local_rec.cust_no, pmat_rec.cont_no);
				DSP_FLD ("cust_no");
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SrchPmpc (temp_str);  
			return (EXIT_SUCCESS);
		}
	
		strcpy (pmpc_rec.proj_no, pmat_rec.proj_no); 
		strcpy (pmpc_rec.cont_no, local_rec.cust_no); 
		cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------------------------------------
			| Contractor does not exist in project-contractor table !|
			--------------------------------------------------------*/
			errmess (ML (mlPmMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy (cumr_rec.est_no, comm_rec.test_no);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cust_no, 6));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (pmpr_rec.prospect_no, local_rec.cust_no);
			cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
			if (cc)
			{
				/*-----------------------------
				| Customer %s is not on file. |
				-----------------------------*/
				print_mess (ML (mlStdMess021));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				strcpy (local_rec.cust_name, pmpr_rec.name);
				DSP_FLD ("cust_no");
				DSP_FLD ("cust_name");  
			}
		}
		else
		{
			strcpy (local_rec.cust_name, cumr_rec.dbt_name);
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
		}
		return (EXIT_SUCCESS);
	}

	/*----------------
	| Validate Date. |
	----------------*/
	if (LCHECK ("act_date"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				local_rec.act_date = pmat_rec.act_date;
				DSP_FLD("act_date");
				return(0);
			}
			local_rec.act_date = StringToDate (local_rec.systemDate);
			DSP_FLD ("act_date"); 
		}

		strcpy (pmat_rec.proj_no, pmpm_rec.proj_no);
		strcpy (pmat_rec.cont_no, local_rec.cust_no);
		pmat_rec.act_date = local_rec.act_date;
		cc = find_rec (pmat, &pmat_rec, EQUAL, "u");
		if (cc)
		{
			strcpy (pmat_rec.cont_no, "      ");
			pmat_rec.act_date = 0L;
			strcpy (pmat_rec.act_details, "      ");
			pmat_rec.next_act_date = 0L;
			newCode = TRUE;
		}
		else
		{
			strcpy (cumr_rec.co_no, comm_rec.tco_no);
			strcpy (cumr_rec.est_no, comm_rec.test_no);
			strcpy (cumr_rec.dbt_no, zero_pad (pmat_rec.cont_no,6));
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (local_rec.cust_no, cumr_rec.dbt_no);
				strcpy (local_rec.cust_name, cumr_rec.dbt_name);
			}
			else
			{
				strcpy (pmpr_rec.prospect_no, pmat_rec.cont_no);
				cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
				if (!cc)
				{
					strcpy (local_rec.cust_no, pmpr_rec.prospect_no);
					strcpy (local_rec.cust_name, pmpr_rec.name);
				}
			}
			strcpy (local_rec.act_details, pmat_rec.act_details);
			DSP_FLD ("act_details");
			local_rec.act_date = pmat_rec.act_date;
			DSP_FLD ("act_date");
			local_rec.next_act_date = pmat_rec.next_act_date;
			DSP_FLD ("next_act_date");
			
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
			newCode = FALSE;
			entry_exit = TRUE;  
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Action Details. |
	--------------------------*/
	if (LCHECK ("act_details"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				strcpy (local_rec.act_details, pmat_rec.act_details);
				DSP_FLD ("act_details");
				return (EXIT_SUCCESS);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Next Date. |
	---------------------*/
	if (LCHECK ("next_act_date"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				local_rec.next_act_date = pmat_rec.next_act_date;
				DSP_FLD ("next_act_date");
				return (EXIT_SUCCESS);
			}
			local_rec.next_act_date = StringToDate(local_rec.systemDate);
			DSP_FLD ("next_act_date"); 
		}

		if (local_rec.act_date > local_rec.next_act_date || 
				local_rec.next_act_date == 0L)
		{
			/*--------------------------------------------------------------
			| Next Action Date should not be less than Previous Action Date|
			--------------------------------------------------------------*/
	        print_mess( ML(mlPmMess006) ); 
			sleep(2);
			clear_mess();
			return(1);
		}
		return(0);
	}
	return(0); 
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpm (
 char *	key_val)
{
	work_open();
	save_rec("#Code   ","#Description");
	strcpy (pmpm_rec.proj_no, key_val); 
	cc = find_rec (pmpm, &pmpm_rec, GTEQ, "r");
	while (!cc && !strncmp (pmpm_rec.proj_no, key_val, strlen(key_val)))
	{
		cc = save_rec (pmpm_rec.proj_no, pmpm_rec.title);
		if (cc)
			break;

		cc = find_rec (pmpm, &pmpm_rec, NEXT, "r");
	}
	cc = disp_srch();
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
	work_open();
	save_rec("#Code   ","#Description");
	strcpy (pmpc_rec.proj_no, pmat_rec.proj_no); 
	strcpy (pmpc_rec.cont_no, key_val); 
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc && !strcmp (pmpc_rec.proj_no, pmat_rec.proj_no) &&
			!strncmp (pmpc_rec.proj_no, key_val, strlen(key_val)))
	{
		if (!strcmp(pmpc_rec.type, "C"))
		{
			strcpy(cumr_rec.co_no,  comm_rec.tco_no);
			strcpy(cumr_rec.est_no, comm_rec.test_no);
			strcpy(cumr_rec.dbt_no, zero_pad (pmpc_rec.cont_no,6));
			cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
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
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpc_rec.proj_no, pmat_rec.proj_no); 
		strcpy (pmpc_rec.cont_no, key_val); 
		cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpc, "DBFIND");
	}
}

/*==================
| Updated records. |
==================*/
void
update (
 void)
{
	pmat_rec.act_date = local_rec.act_date;
	strcpy (pmat_rec.act_details, local_rec.act_details); 
	pmat_rec.next_act_date = local_rec.next_act_date;
	if (newCode)
	{
		print_mess ( ML(mlStdMess035) );
		sleep(2);
		strcpy (pmat_rec.cont_no, local_rec.cust_no);
		cc = abc_add (pmat, &pmat_rec);
		if (cc) 
			file_err(cc, pmat, "DBADD");
	}
	else
	{
		mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
		switch (mmenu_select (upd_menu))
		{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				print_mess ( ML(mlStdMess035) ); 
				sleep(2);
				strcpy (pmat_rec.cont_no, local_rec.cust_no);
				cc = abc_update (pmat, &pmat_rec);
				if (cc) 
					file_err (cc, pmat, "DBUPDATE");
				abc_unlock (pmat);
				clear_mess ();
				break;

			case SEL_IGNORE :
				abc_unlock (pmat);
				break;

			case SEL_DELETE :
			{
				print_mess (ML (mlStdMess035));
				sleep (sleepTime);
				clear_mess ();
				strcpy (pmat_rec.cont_no, local_rec.cust_no);
				cc = abc_delete (pmat);
				if (cc)
					file_err (cc, pmat, "DBDELETE");
				clear_mess ();
				break;
			}
			default:
				break;
		} 
	}
	clear_mess ();
	abc_unlock (pmat);
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
		swide ();

		/*-----------------------
		| %R Action Maintenance |
		-----------------------*/
		centre_at (0, 132, ML(mlPmMess002) );
		move (0, 1); line (132);

		box (0, 3, 132, 6);

		move (1, 6);
		line (131);
        move(0, 20);       
        line(132);
		print_at (21, 1, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_short);
		print_at (21, 55, ML(mlStdMess039), comm_rec.test_no,comm_rec.test_short);
		print_at (21, 105, ML(mlStdMess099), comm_rec.tcc_no,comm_rec.tcc_short);
        move (0, 22);
		line (132);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

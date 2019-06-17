/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_pq_maint.c )                                  |
|  Program Desc  : ( Project Quotes Program.                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpm, pmpq, pmpr, cumr, qthr,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmpq,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy Medel       | Date Written  : 26/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (18/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (12/09/97)      | Modified by : Marnie Organo      |
|  Date Modified : (15/10/1997)    | Modified by : Jiggs A Veloz.     |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      : (18/01/96) - Fixed Bugs.                       	  |
|                : (12/09/97) - Updated for Multilingual Conversion   |
|  (15/10/1997)  : SEL Updated proj_no and quote_no fields 			  |
|				 :     from char6 to char8.	  						  |
|  (17/09/1999)  : Ported to ANSI standards.                          
|                                                                     |
| $Log: pq_maint.c,v $
| Revision 5.3  2002/07/25 11:17:30  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:01  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:55  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/11/16 02:56:13  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.12  1999/10/20 02:06:55  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/09/29 10:11:46  scott
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
char	*PNAME = "$RCSfile: pq_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_pq_maint/pq_maint.c,v 5.3 2002/07/25 11:17:30 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_pm_mess.h>
typedef  int BOOL;

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

	/*========================================+
	 | Project Monitoring Project Quote File. |
	 +========================================*/
#define	PMPQ_NO_FIELDS	3

	struct dbview	pmpq_list [PMPQ_NO_FIELDS] =
	{
		{"pmpq_proj_no"},
		{"pmpq_cont_no"},
		{"pmpq_quote_no"}
	};

	struct tag_pmpqRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	quote_no [9];
	}	pmpq_rec;

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
#define	PMPC_NO_FIELDS	10

	struct dbview	pmpc_list [PMPC_NO_FIELDS] =
	{
		{"pmpc_proj_no"},
		{"pmpc_cont_no"},
		{"pmpc_type"},
		{"pmpc_parent_no"},
		{"pmpc_area"},
		{"pmpc_mtrl_cost"},
		{"pmpc_loyal_rat"},
		{"pmpc_ord_prob"},
		{"pmpc_sman_code"},
		{"pmpc_status"}
	};

	struct tag_pmpcRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	type [2];
		char	parent_no [7];
		char	area [3];
		Money	mtrl_cost;
		float	loyal_rat;
		float	ord_prob;
		char	sman_code [3];
		char	status [2];
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

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	6

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_cont_no"},
		{"qthr_enq_ref"},
		{"qthr_dt_quote"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		char	cont_no [7];
		char	enq_ref [21];
		long	dt_quote;
	}	qthr_rec;

	char	*data  = "data",
			*pmpq  = "pmpq",
			*pmpm  = "pmpm",
			*cumr  = "cumr", 
			*qthr  = "qthr",
			*pmpc  = "pmpc",
			*pmpr  = "pmpr";

	char	i,
			orig_cust [7];

	int 	old_rec = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	proj_no [9];
	char	title [41];
	char 	cust_no [7];
	char	cust_name [41];
	char 	quote_no [9];
	char	dummy [11];
} local_rec;


static	struct	var	vars [] =
{
	{1, LIN, "proj_ref",	 4, 19, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Project Code      :", "Enter Project Code. [SEARCH]",
		NE, NO,  JUSTLEFT, "", "", local_rec.proj_no},
	{1, LIN, "proj_title",	 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.title},
	{1, LIN, "cust_no", 5, 19, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Contractor Number :", "Enter Contractor Number. [SEARCH]",
		NO, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "cust_name", 5, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.cust_name},
	{1, LIN, "quote_no", 6, 19, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Quotation Number  :", "Enter Quotation Number. [SEARCH]",
		NO, NO,  JUSTLEFT, "", "", local_rec.quote_no},
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
void	SrchQthr		(char *);
void	Update			(void);
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
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
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

		Update ();

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
	open_rec (pmpq, pmpq_list, PMPQ_NO_FIELDS, "pmpq_quote_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpm);
	abc_fclose (pmpq);
	abc_fclose (cumr);
	abc_fclose (qthr);
	abc_fclose (pmpr);
	abc_fclose (pmpc);

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
		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		sprintf (pmpm_rec.proj_no, "%-8.8s", local_rec.proj_no);
		cc = find_rec(pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------------
			| Reference Number not found. |
			-----------------------------*/
			print_mess(ML(mlStdMess191));
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.title, pmpm_rec.title);
		DSP_FLD ("proj_ref");
		DSP_FLD ("proj_title");
		return(0);  
	}

	/*-----------------------------
	| Validate Contractor Number. |
	-----------------------------*/
	if (LCHECK ("cust_no"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				strcpy (local_rec.cust_no, orig_cust);
				DSP_FLD ("cust_no");
				return (EXIT_SUCCESS);
			}
		}	

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
			if (!strcmp(pmpc_rec.type, "C"))
			{
				strcpy(cumr_rec.co_no,  comm_rec.tco_no);
				strcpy(cumr_rec.est_no, comm_rec.test_no);
				strcpy(cumr_rec.dbt_no, zero_pad (local_rec.cust_no,6));
				cc = find_rec(cumr, &cumr_rec, EQUAL, "r");
				if (!cc)
				{
					strcpy (local_rec.cust_no, cumr_rec.dbt_no);
					strcpy (local_rec.cust_name, cumr_rec.dbt_name);
					DSP_FLD("cust_no");
					DSP_FLD("cust_name");  
					return(0);
				}
			}
			else
			{
				strcpy (pmpr_rec.prospect_no, local_rec.cust_no);
				cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
				if (!cc)
				{
					strcpy (local_rec.cust_no,   pmpr_rec.prospect_no);
					strcpy (local_rec.cust_name, pmpr_rec.name);
					DSP_FLD ("cust_no");
					DSP_FLD ("cust_name");  
					return (EXIT_SUCCESS);
				}
			}
		}
		else
		{
			/*------------------------------
			| Contractor Number not found. |
			------------------------------*/
			print_mess (ML (mlPmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Quotation Number. |
	----------------------------*/
	if (LCHECK ("quote_no"))
	{
		if (dflt_used)
		{
			if (prog_status != ENTRY)
			{
				strcpy (local_rec.quote_no, pmpq_rec.quote_no);
				DSP_FLD ("quote_no");
				return (EXIT_SUCCESS);
			}
		}
		if (SRCH_KEY)
		{
			SrchQthr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qthr_rec.co_no, comm_rec.tco_no);
		strcpy (qthr_rec.br_no, comm_rec.test_no);
		strcpy (qthr_rec.quote_no, zero_pad(local_rec.quote_no,8));
		cc = find_rec ("qthr", &qthr_rec, EQUAL, "r");
		if (!cc) 
		{
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("quote_no");
		}	
		else
		{
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("quote_no");

			/*-------------------------------------
			| Quotation Number %s is not on file. |
			-------------------------------------*/
			print_mess (ML (mlStdMess152));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (pmpq_rec.quote_no, zero_pad(local_rec.quote_no,8));
		cc = find_rec ("pmpq", &pmpq_rec, EQUAL, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			/*--------------------------------------------
			| Quotation Number has already been assigned |
			| to another Project/Contractor.			 |
			--------------------------------------------*/
			print_mess (ML (mlPmMess015));
			sleep (sleepTime);
			clear_mess ();
			old_rec = FALSE;
			return (EXIT_FAILURE);
		}
		strcpy (orig_cust, local_rec.cust_no);      
		return (EXIT_SUCCESS);
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
	sprintf (pmpm_rec.proj_no, "%-8.8s", key_val); 
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
		sprintf (pmpm_rec.proj_no, "%-8.8s", temp_str);
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
	char prev_cont [7];

	work_open ();
	save_rec("#Code   ", "#Description");

	sprintf(pmpc_rec.proj_no, "%-8.8s", local_rec.proj_no); 
	strcpy (pmpc_rec.cont_no, key_val); 
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc && !strcmp (pmpc_rec.proj_no, local_rec.proj_no) &&
			!strncmp (pmpc_rec.cont_no, key_val, strlen(key_val)))
	{
		if (!strcmp (prev_cont, pmpc_rec.cont_no))
		{
			cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
			continue;
		}

		if (!strcmp (pmpc_rec.type, "C"))
		{
			strcpy (cumr_rec.co_no,  comm_rec.tco_no);
			strcpy (cumr_rec.est_no, comm_rec.test_no);
			strcpy (cumr_rec.dbt_no, zero_pad (pmpc_rec.cont_no, 6));
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
		strcpy (prev_cont, pmpc_rec.cont_no);
		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}
	cc = disp_srch();
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

/*=============================+
| Search Quotation Header File |
==============================*/
void
SrchQthr (
 char *	key_val)
{
	work_open();
	save_rec("#Code   ","#Quote Description");
	strcpy (qthr_rec.co_no, comm_rec.tco_no);
	strcpy (qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no, "%-8.8s", key_val); 
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strncmp (qthr_rec.quote_no, key_val, strlen(key_val)))
	{
		sprintf (err_str, "%-10.10s -%-15.15s", DateToString(qthr_rec.dt_quote),
												qthr_rec.enq_ref);
		cc = save_rec (qthr_rec.quote_no, err_str);
		if (cc)
			break;

		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (qthr_rec.co_no, comm_rec.tco_no);
		strcpy (qthr_rec.br_no, comm_rec.test_no);
		sprintf(qthr_rec.quote_no, "%-8.8s", temp_str);
		cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, qthr, "DBFIND");
	}
}

/*==================
| Updated records. |
==================*/
void
Update (
 void)
{
	sprintf (pmpq_rec.proj_no,  "%-8.8s", local_rec.proj_no); 
	strcpy (pmpq_rec.cont_no,  local_rec.cust_no); 
	sprintf (pmpq_rec.quote_no, "%-8.8s", local_rec.quote_no); 
	if (newCode)
	{
		strcpy (err_str, ML (mlStdMess035));
		print_mess (err_str); 
		sleep (sleepTime);

		cc = abc_add (pmpq, &pmpq_rec);
		if (cc) 
			file_err (cc, pmpq, "DBADD"); 
	}
	else
	{
		mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
		switch (mmenu_select (upd_menu))
		{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				strcpy(err_str, ML(mlStdMess035));
				print_mess (err_str);
				 sleep(2);

				cc = abc_update (pmpq, &pmpq_rec);
				if (cc) 
					file_err (cc, pmpq, "DBUPDATE");  
				abc_unlock (pmpq);
				clear_mess ();
				break;

			case SEL_IGNORE :
				abc_unlock (pmpq);
				break;
			case SEL_DELETE :
			{
				clear_mess ();
				print_mess (ML (mlStdMess014)); 
				sleep (sleepTime);

				cc = abc_delete (pmpq);
				if (cc)
					file_err (cc, pmpq, "DBDELETE"); 
				clear_mess ();
				break;
			}
			default:
				break;
		} 
	}
	clear_mess ();
	abc_unlock (pmpq);
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
		/*---------------------------
		| Project Quote Maintenance |
		---------------------------*/
		centre_at (0, 80, ML (mlPmMess016));
		move (0, 1); 
		line (80);

		box (0, 3, 80, 3);

        move (0, 20);
        line (80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 1, err_str,comm_rec.tco_no, comm_rec.tco_short);
		strcpy (err_str, ML (mlStdMess039));
		print_at (21, 27, err_str,comm_rec.test_no, comm_rec.test_short);
		strcpy (err_str, ML (mlStdMess099));
		print_at (21, 57, err_str, comm_rec.tcc_no, comm_rec.tcc_short);
        move (0, 22);
		line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


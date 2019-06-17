/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_prj_dtl.c )                                   |
|  Program Desc  : ( Project Monitoring Project Inquiry.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpr, pmpc ,pmpm, pmpq, pmat, qthr,         |
|                   cumr, exsf,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 23/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (27/12/95)      | Modified by : Jojo M. Gatchalian |
|  Date Modified : (19/01/96)      | Modified by : Joy  G. Medel      |
|  Date Modified : (12/09/97)      | Modified by : Roanna Marcelino   |
|  Date Modified : (16/10/97)      | Modified by : Marnie Organo      |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                	  |
|  (27/12/95)    : 				   - Schema Changes.                  |
|                : 				   - Created the functions that are   |
|                : 				     called by the ring menu.         |
|                : 				   - Assorted Fixes.                  |
|  (19/01/96)    : 				   - Display error message if there's |
|                                    no existing vi file.             |
|  (12/09/97)    : 				   - Modified for Multilingual        |
|                :                   Conversion.                      |
|  (16/10/97)    : 				   - Changed length of fields :       |
|                :                   pmpc(proj_no and parent_no) ,    |
|                :                   pmpq(quote_no and proj_no) ,     |
|                :                   pmat_proj_no  and pmnt_proj_no   |
|                :                   from 6 to 8.                     |      
|  (17/09/1999)  :                 - Ported to ANSI standards.        |  
|                                                                     |
| $Log: pm_prj_dtl.c,v $
| Revision 5.3  2002/07/25 11:17:30  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:52  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:25  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:03:55  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/16 02:56:14  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.10  1999/09/29 10:11:46  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 04:23:20  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/06/17 07:54:52  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pm_prj_dtl.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_prj_dtl/pm_prj_dtl.c,v 5.3 2002/07/25 11:17:30 scott Exp $";

#include <pslscr.h>
#include <ring_menu.h>
#include <chk_ring_sec.h>
#include <ml_std_mess.h>
#include <ml_pm_mess.h>

/*==============================================
| Local function prototypes used by _main_menu |
==============================================*/
int		Notes			(void);
int		Actions			(void);
int		Quotations		(void);
int		Subcons			(void);
int		inv_enquiry		(char *);


FILE *	fin;
char directory [100] = "";

menu_type _main_menu [] = {
	{ "<N>otes",				"Display Project Notes",
		Notes,					"Nn", },
	{ "<A>ctions",				"Display Project Developments",
		Actions,				"Aa", },
	{ "<Q>uotations",			"Display Contractor Quotations",
		Quotations,				"Qq", },
	{ "<S>ubcontractors",		"Display Subcontractors",
		Subcons,				"Ss", },
	{ "<E>xit",				"Exit Display",
		_no_option,				"Ee", FN16, EXIT | SELECT | SHOW },
	{ "", },
};

	/*====================
	| System Common File |
	====================*/
#define	COMM_NO_FIELDS	3

	struct dbview comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_est_no"}
	};

	struct tag_commRecord
	{
		int		termno;
		char	tco_no		[3];
		char	test_no		[3];
	}	comm_rec;

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
		char	proj_no 	[9];
		char	title 		[41];
	}	pmpm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
#define	CUMR_NO_FIELDS  4

	struct dbview 	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
	};

	struct tag_cumrRecord
	{
		char	cm_co_no	[3];
		char	cm_est_no	[3];
		char	cm_dbt_no	[7];
		char	cm_name		[41];
	}	cumr_rec;

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	3

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"}
	};

	struct tag_exsfRecord
	{
		char	co_no 		[3];
		char	sman_no 	[3];
		char	sman_name 	[41];
	} exsf_rec;

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
		char	name 		[41];
	}	pmpr_rec;

	/*=========================+
	 | Project-Contractor File |
	 +=========================*/
#define	PMPC_NO_FIELDS	5

	struct dbview	pmpc_list [PMPC_NO_FIELDS] =
	{
		{"pmpc_proj_no"},
		{"pmpc_cont_no"},
		{"pmpc_type"},
		{"pmpc_parent_no"},
		{"pmpc_sman_code"}
	};

	struct tag_pmpcRecord
	{
		char	proj_no 	[9];
		char	cont_no 	[7];
		char	type 		[2];
		char	parent_no 	[7];
		char	sman_code 	[3];
	}	pmpc_rec;

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
		char	proj_no 	[9];
		char	cont_no 	[7];
		Date	act_date;
		char	act_details [81];
		Date	next_act_date;
	}	pmat_rec;

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
		char	proj_no 	[9];
		char	cont_no 	[7];
		char	quote_no 	[9];
	}	pmpq_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	8

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_enq_ref"},
		{"qthr_dt_quote"},
		{"qthr_place_ord"},
		{"qthr_qt_value"},
		{"qthr_qt_profit_cur"},
		{"qthr_qt_profit_pc"}
	};

	struct tag_qthrRecord
	{
		char	br_no 		[3];
		char	quote_no 	[9];
		char	enq_ref 	[21];
		long	dt_quote;
		char	place_ord 	[2];
		double	qt_value;
		double	qt_profit_cur;
		float	qt_profit_pc;
	}	qthr_rec;

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
		char	proj_no 	[9];
		char	cont_no 	[7];
		char	filename	[13];
	}	pmnt_rec;

	char	*data  = "data",
			*pmpm  = "pmpm",
			*pmpc  = "pmpc",
			*pmpq  = "pmpq",
			*pmat  = "pmat",
			*pmpr  = "pmpr",
			*pmnt  = "pmnt",
			*qthr  = "qthr",
			*exsf  = "exsf",
			*cumr  = "cumr";

/*============================
| Local & Screen Structures. |
============================*/
	struct 
	{
		char	name	[41];
		char	cont_no	[7];
		char	text	[133];
		char	dummy 	[11];
	}	local_rec;

static	struct	var	vars [] =
{
	{ 1, LIN, "code",	 2, 25, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Project Code        :", 
		"Enter Project Code [SEARCH] for codes",
		NE, NO,  JUSTLEFT, "", "", pmpm_rec.proj_no },
	{ 1, LIN, "project_title",	 2, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", pmpm_rec.title },
	{ 1, LIN, "cont_no",	 3, 25, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Contractor Number   :", 
		"Enter Contractor Number [SEARCH] for references",
		NO, NO,  JUSTLEFT, "", "", local_rec.cont_no}, 
	{ 1, LIN, "cont_name",	 3, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.name },
	{ 1, LIN, "sman",	 4, 25, CHARTYPE,
		"AA", "          ",
		" ", exsf_rec.sman_no, "Salesman Number     :", "",
		NA, NO, JUSTLEFT, "", "", pmpc_rec.sman_code },
	{ 1, LIN, "proj_sman_name",	 4, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", exsf_rec.sman_name },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};

int		envDbFind  	= 0,
		envDbCo 	= 0,
		line_no 	= 0,
		clear_ok 	= TRUE;

char	branchNo [3]	= "";

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
int		heading			(int);
void	SrchPmpm		(char *);
void	SrchPmpc		(char *);
void	run_sdisplay	(char *, char *, char *);
void	show_mess		(int);
void	cls_scr			(void);


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (branchNo, (envDbCo) ? comm_rec.test_no : " 0");

	sptr = getenv ("PROG_PATH");
	sprintf (directory, "%s/BIN/PNT", (sptr == (char *)0) ? "/usr/ver9" : sptr);

	clear ();
	swide ();
	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
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
		clear_ok 	= TRUE;

		init_vars (1);
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		print_mess (ML (mlPmMess009));
		edit (1);

		if (restart)
			continue;

		run_menu (_main_menu, "", 22);
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
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_quote_no");
	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (pmpq, pmpq_list, PMPQ_NO_FIELDS, "pmpq_id_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
	open_rec (pmat, pmat_list, PMAT_NO_FIELDS, "pmat_id_no");
	open_rec (pmnt, pmnt_list, PMNT_NO_FIELDS, "pmnt_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, 
			 (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpr);
	abc_fclose (pmpm);
	abc_fclose (pmpc);
	abc_fclose (pmpq);
	abc_fclose (pmat);
	abc_fclose (pmnt);
	abc_fclose (qthr);
	abc_fclose (exsf);
	abc_fclose (cumr);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*------------------------
	| Validate Project Code. |
	------------------------*/
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (pmpm_rec.proj_no, "        "))
			return (EXIT_FAILURE);

		cc = find_rec (pmpm, &pmpm_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlPmMess010));
			sleep (sleepTime);	
			return (EXIT_FAILURE);
		}
		else
			DSP_FLD ("project_title");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Contractor Number. |
	-----------------------------*/
	if (LCHECK ("cont_no"))
	{
		char message [61];

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPmpc (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.cont_no, "      "))
			return (EXIT_FAILURE);

		strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
		strcpy (pmpc_rec.cont_no, local_rec.cont_no);
		cc = find_rec (pmpc, &pmpc_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (message,ML(mlPmMess017), local_rec.cont_no);
			print_mess (message);
			sleep (sleepTime);	
			return (EXIT_FAILURE);
		}

		if (pmpc_rec.type [0] == 'C')
		{
			strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
			strcpy (cumr_rec.cm_est_no,branchNo);
			strcpy (cumr_rec.cm_dbt_no, zero_pad (pmpc_rec.cont_no,6));
			cc = find_rec (cumr,&cumr_rec,EQUAL,"r");
			if (!cc)
				strcpy (local_rec.name, cumr_rec.cm_name);
			else
				sprintf(local_rec.name, "%-40.40s", "Not Found");
		}
		else
		{
			strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no);
			cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
			if (!cc)
				strcpy (local_rec.name, pmpr_rec.name);
			else
				sprintf(local_rec.name, "%-40.40s", "Not Found");
		}

		DSP_FLD ("cont_name");

		strcpy (exsf_rec.co_no,comm_rec.tco_no);
		strcpy (exsf_rec.sman_no, pmpc_rec.sman_code);
		cc = find_rec (exsf,&exsf_rec,EQUAL,"r");
		if (!cc)
		{
			DSP_FLD ("sman");
			DSP_FLD ("proj_sman_name");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
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

		if (clear_ok)
			clear ();
		centre_at (0, 132, ML (mlPmMess018));
		box (0, 1, 132, 3);
		box (0, 6, 132, 14);
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
Notes (
 void)
{
	show_mess (0);
	Dsp_open (0, 6, 12);
	Dsp_saverec ("                                 P  r  o  j  e  c  t     D  e  t  a  i  l     N  o  t  e  p  a  d                                 ");
	Dsp_saverec ("");
	Dsp_saverec	(" [RECALL]  [PREV]  [NEXT]  [END] ");

	memset (&pmnt_rec, 0, sizeof (pmnt_rec));
	strcpy (pmnt_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmnt_rec.cont_no, local_rec.cont_no);
	cc = find_rec (pmnt, &pmnt_rec, EQUAL, "r");
	if (!cc)
	{
		char	filename[50]	=	"",
				dsp_str [133] 	= 	"",
				*sptr,
				*tptr;

		sprintf (filename, "%s/%s", directory, pmnt_rec.filename);
		clip (filename);

		if (access (filename, 0) == 0)
		{
			fin = fopen (filename, "r");
			sptr = fgets (dsp_str, 132, fin);
			while (sptr)
			{
				*(sptr + strlen(sptr) - 1) = '\0';

				sprintf (local_rec.text, "%-132.132s", sptr); 

				/*------------------------
				| Replace occurrences of |
				| ESC with a space       |
				------------------------*/
				tptr = strchr (local_rec.text, '\033');
				while (tptr)
				{
					*tptr = ' ';
					tptr = strchr (local_rec.text, '\033');
				}

				Dsp_saverec (local_rec.text);
			
				sptr = fgets (dsp_str, 132, fin);
			} 
			fclose (fin);
		}
	}
	Dsp_srch ();
	Dsp_close ();
	cls_scr	();
	return (EXIT_SUCCESS);
}

int
Actions (
 void)
{
	char date		[11]	=	"",
		 dsp_str	[133]	=	"",
		 next_date	[11]	=	"";

	show_mess (0);
	Dsp_open(0, 6, 12);
	Dsp_saverec ("   Date           |                                  Action Details                                   | Next Action Date          ");
	Dsp_saverec ("");
	Dsp_saverec	(" [RECALL]  [PREV]  [NEXT]  [EDIT/END] ");

	memset (&pmat_rec, 0, sizeof (pmat_rec));
	strcpy (pmat_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmat_rec.cont_no, local_rec.cont_no);
	cc = find_rec (pmat, &pmat_rec, GTEQ, "r");
	while (!cc
	&&	   !strcmp (pmat_rec.proj_no, pmpm_rec.proj_no)
	&&	   !strcmp (pmat_rec.cont_no, local_rec.cont_no))
	{
		sprintf(date, 		"%-10.10s", DateToString (pmat_rec.act_date));
		sprintf(next_date,  "%-10.10s", DateToString (pmat_rec.next_act_date));

		sprintf(dsp_str, "   %s     ^E %-80.80s  ^E %s                ",
				date,
				pmat_rec.act_details,
				next_date);
		Dsp_saverec(dsp_str);

		cc = find_rec (pmat, &pmat_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	cls_scr	();
	return (EXIT_SUCCESS);
}

int
Quotations (
 void)
{
	char	dsp_str		[133]	=	"",
			save_key	[21]	=	"",
			date		[11]	=	"",
			amount		[15]	=	"",
			profitpc	[10]	=	"",
			profit		[15]	=	"";

	line_no = 0;
	show_mess (1);
	Dsp_open (0, 6, 11);
	Dsp_saverec ("  Quote Date   |   Quote No   |   Quote Reference           | Order |   Quote Amount       |   Profit Amount      |   Profit (%)  ");
	Dsp_saverec ("");
	Dsp_saverec	(" [RECALL]  [PREV]  [NEXT]  [END] ");

	memset (&pmpq_rec, 0, sizeof (pmpq_rec));
	strcpy (pmpq_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmpq_rec.cont_no, local_rec.cont_no);
	cc = find_rec (pmpq, &pmpq_rec, GTEQ, "r");
	while (!cc
	&&	   !strcmp (pmpq_rec.proj_no, pmpm_rec.proj_no)
	&&	   !strcmp (pmpq_rec.cont_no, local_rec.cont_no))
	{
		strcpy (qthr_rec.quote_no, pmpq_rec.quote_no);
		cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf(date,    "%-10.10s", DateToString (qthr_rec.dt_quote));
			sprintf(amount,  "%-14.14s", 
					comma_fmt (DOLLARS (qthr_rec.qt_value),
							  "NNN,NNN,NNN.NN"));
			sprintf(profit,  "%-14.14s", 
					comma_fmt (DOLLARS (qthr_rec.qt_profit_cur),
							  "NNN,NNN,NNN.NN"));
			sprintf(profitpc,"%6.2f  %%", qthr_rec.qt_profit_pc);

			sprintf (dsp_str, "  %s     ^E   %s     ^E   %s      ^E   %s   ^E     %s   ^E     %s   ^E   %s   ", 
					 date,
					 qthr_rec.quote_no,
					 qthr_rec.enq_ref,
					 qthr_rec.place_ord,
					 amount,
					 profit,
					 profitpc);

			sprintf (save_key, "%04d%-2.2s%-8.8s",
					 line_no++, 
					 qthr_rec.br_no,
					 qthr_rec.quote_no);

			Dsp_save_fn(dsp_str, save_key); 
		}
		cc = find_rec (pmpq, &pmpq_rec, NEXT, "r");
	}
	Dsp_srch_fn	(inv_enquiry);
	Dsp_close ();
	cls_scr	();
	return (EXIT_SUCCESS);
}

int
Subcons (
 void)
{
	char dsp_str [133] = "";

	show_mess (0);
	Dsp_open (0, 6, 12);
	Dsp_saverec ("                                        Subcontractor Code   |   Subcontractor Name                                               ");
	Dsp_saverec ("");
	Dsp_saverec	(" [RECALL]  [PREV]  [NEXT]  [EDIT/END] ");

	memset (&pmpc_rec, 0, sizeof (pmpc_rec));
	abc_selfield (pmpc, "pmpc_id_no3");
	strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmpc_rec.cont_no, "      ");
	strcpy (pmpc_rec.parent_no, local_rec.cont_no);
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc
		&& !strcmp (pmpc_rec.proj_no, pmpm_rec.proj_no))
	{
		if (!strcmp (pmpc_rec.parent_no, local_rec.cont_no))
		{
			if (pmpc_rec.type [0] == 'C')
			{
				strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
				strcpy (cumr_rec.cm_est_no,branchNo);
				strcpy (cumr_rec.cm_dbt_no, zero_pad (pmpc_rec.cont_no,6));
				cc = find_rec (cumr,&cumr_rec,EQUAL,"r");
				if (!cc)
				{
					sprintf (dsp_str, "                                        %-18.18s   ^E   %-40.40s                           ",
							 cumr_rec.cm_dbt_no,
							 cumr_rec.cm_name);
					Dsp_saverec (dsp_str);
				}
			}
			else
			{
				strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no);
				cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
				if (!cc)
				{
					sprintf(dsp_str, "                                        %-20.20s   ^E   %-40.40s                           ",
							pmpr_rec.prospect_no,
							pmpr_rec.name);
					Dsp_saverec (dsp_str);
				}
			}
		}
		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	cls_scr	();
	abc_selfield (pmpc, "pmpc_id_no");
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
	save_rec ("#Code   ","#Description");

	sprintf(pmpm_rec.proj_no, "%-8.8s", key_val);
	cc = find_rec (pmpm, &pmpm_rec, GTEQ, "r");
	while (!cc 
	&& 	   !strncmp (pmpm_rec.proj_no, key_val, strlen(key_val)))
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
		strcpy (pmpm_rec.proj_no, temp_str);
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpm, "DBFIND");
	}
	else
		memset (&pmpm_rec, 0, sizeof (pmpm_rec));
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpc (
 char *	key_val)
{
	char name [41] = "";

	work_open ();
	save_rec ("#Cont No","#Description");

	strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
	sprintf (pmpc_rec.cont_no, "%-6.6s", key_val);
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc 
		&& !strcmp (pmpc_rec.proj_no, pmpm_rec.proj_no)
		&& !strncmp (pmpc_rec.cont_no, key_val, strlen (key_val)))
	{
		sprintf (name, "%-40.40s",  "Not Found");

		if (pmpc_rec.type [0] == 'C')
		{
			strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
			strcpy (cumr_rec.cm_est_no,branchNo);
			strcpy (cumr_rec.cm_dbt_no, zero_pad (pmpc_rec.cont_no,6));
			cc = find_rec (cumr,&cumr_rec,EQUAL,"r");
			if (!cc)
				sprintf (name, "%-40.40s", cumr_rec.cm_name);
		}
		else
		{
			strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no);
			cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
			if (!cc)
				sprintf (name, "%-40.40s", pmpr_rec.name);
		}
		cc = save_rec (pmpc_rec.cont_no, name);
		if (cc)
			break;

		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
		strcpy (pmpc_rec.cont_no, temp_str);
		cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpc, "DBFIND");
	}
	else
		memset (&pmpc_rec, 0, sizeof (pmpc_rec));
}

int
inv_enquiry (
 char *	find_key)
{
	char _br_no [3],
		 _quote_no [9];

	sprintf (_br_no, "%-2.2s", find_key + 4);
	sprintf (_quote_no, "%-8.8s", find_key + 6);
	run_sdisplay (comm_rec.tco_no, _br_no, _quote_no); 
	return (EXIT_SUCCESS);
}

/*=======================================================
| Execute quotation display passing relevent arguments. |
=======================================================*/
void
run_sdisplay (
 char *	_co_no,
 char *	_br_no,
 char *	_quote_no)
{
	char	run_string [100],
			run_string1 [100];
	int		indx = 1,
			i;

 	sprintf (run_string1, "qt_quote.s");
 	sprintf (run_string, "%-2.2s %-2.2s %-8.8s", _co_no, _br_no, _quote_no); 

	for (i = 0; i < 4; i++)
	{
		move (44, i + 13);
		printf ("                                             ");
	}
	box (47, 13, 39, 1);
	rv_pr (ML (mlStdMess035), 48, 14, 1); 
	print_at (0, 0, "                                         ");

	arg [0] = "qt_quote";
	arg [1] = run_string1;
	arg [++indx] = run_string;
	arg [++indx] = (char *)0;

	shell_prog (2);
	clear ();
	swide ();
	clear_ok = TRUE;
	heading	(1);
	scn_display	(1); 
	show_mess (1);
	Dsp_heading	();
}

void
show_mess (
 int flag)
{
	move (0,22);
	print_at (0, 0,"                                                              \n");
	print_at (0, 0,"                                                              ");
	if (flag)
		/* Display Quotation [ENTER] */
		print_mess (ML (mlPmMess011));
}

void
cls_scr (
 void)
{
	int	i;

	for (i = 0; i < 16; i++)
	{
		move (1, i + 7);
		print_at (0, 0,"                                                                                                                                  ");
	}
	clear_ok = FALSE;
	heading	(1);
	box (0, 6, 132, 14);
}

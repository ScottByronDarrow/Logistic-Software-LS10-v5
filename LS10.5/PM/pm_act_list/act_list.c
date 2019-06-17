/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_act_list.c )                                  |
|  Program Desc  : ( Actions Report Listing.                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  pmpm, pmpc, cumr, pmpr, pmat, comm, exsf, 		  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 10/01/96         |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified by : Marnie Organo      |
|  Date Modified : (16/10/97)      | Modified by : Elizabeth D. Paid  |
|  Date Modified : (15/05/1998)    | Modified by : Leah Manibog.      |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|   Comment      :                                                    |
|  (16/10/97)    :  SEL - change the length of pmpm_proj_no, pmpc_proj|
|                :        _no, pmat_proj_no , pmpr_prospect_no, pmpc_p|
|                :        arent_no from 6 to 8                        |
|  (15/05/1998)  :  SEL - Fixed bug regarding the report printing of  | 
|                :        date and Contractor field.                  |
|  (17/09/1999)  : Ported to ANSI standards.                          |
| $Log: act_list.c,v $
| Revision 5.3  2002/07/17 09:57:31  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:15:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:33:58  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:52  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:36:33  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 02:56:10  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.11  1999/09/29 10:11:45  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/24 04:23:16  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/06/17 07:54:46  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: act_list.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_act_list/act_list.c,v 5.3 2002/07/17 09:57:31 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_pm_mess.h>

int 	first_time	= 1,
		counter		= 0,
		pass_data	= 0;

char 	*comm = "comm",
     	*pmpm = "pmpm",
		*pmpc = "pmpc",
     	*cumr = "cumr",
		*pmpr = "pmpr",
		*exsf = "exsf",
		*pmat = "pmat",
		*data = "data",
		*sptr;

char	proj_no	[9],
		proj_desc [41],
		cont_no [7],
		cont_name [41],
		sman_code [3],
		sman_name [41],
		type [2],
		next_date [11];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list [] ={
		{"comm_term"},
		{"comm_co_no"},
        {"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"}
	};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
        char    tco_short [16];
		char	test_no [3];
		char  	test_name [41];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_short [10];
	} comm_rec;

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
		{"pmpc_sman_code"},
		{"pmpc_status"}
	};

	struct tag_pmpcRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	type [2];
		char	parent_no [7];
		char	sman_code [3];
		char	status [2];
	}	pmpc_rec;

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
		char	co_no [3];
		char	salesman_no [3];
		char	salesman [41];
	}	exsf_rec;

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

	char 	systemDate [11];
	char	s_date [11];
	char	e_date [11];
	long 	lsystemDate;

	FILE	*fout,
			*fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	proj_no [9];
	char	proj_desc [41];
	char	sman_code [3];
	char	sman_name [41];
	long	s_date;
	long	e_date;
    int     prno;
	} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "proj_no",	 3, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Project Code       :", "Enter Project Code or [SEARCH].  Default is ALL.",
		NO, NO, JUSTLEFT, "", "", local_rec.proj_no},
	{1, LIN, "proj_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.proj_desc},
	{1, LIN, "sman_code",	 4, 20, CHARTYPE,
		"UU", "          ",
		" ", " ", "Salesman           :", "Enter Salesman Code or [SEARCH].  Default is ALL.",
		NO, NO, JUSTLEFT, "", "", local_rec.sman_code},
	{1, LIN, "sman_name",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.sman_name},
	{1, LIN, "s_date",	 7, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "     Begin         :", "Enter Beginning Date.",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.s_date},
	{1, LIN, "e_date",	 8, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "systemDate", "     End           :", "Enter Ending Date.",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.e_date},
	{1, LIN, "prno", 	10, 20, INTTYPE, 
		"NN", "          ", 
		" ", " ", "Printer Number     :", " Default is 1.", 
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.prno}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	head_output		(void);
void	process_file	(void);
void	store_data		(void);
void	display_report	(void);
int		heading			(int);
void	head_display	(void);
void	print_detail	(void);
void	SrchPmpm		(char *);
void	SrchPmpc		(char *);
void	SrchExsf		(char *);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR 	(vars);
	OpenDB	();

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();

	/* ============================
	| Set up required parameters  |
	=============================*/
	
	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1);

	while (prog_exit == 0)
	{
		/*===========================	
		|    Reset control flags    |
		===========================*/
		entry_exit	= 0;
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		init_ok		= 1;
		search_ok	= 1;
		init_vars (1);

		/*=============================
		| Entry screen 1 linear input |
		=============================*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*============================	
		| Edit screen 1 linear input |
		============================*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
		if (!restart)
		{
			dsp_screen ("Printing of Actions Listing ...", 
						comm_rec.tco_no, comm_rec.tco_name);
            process_file ();
			head_output ();
			if (pass_data)  
				display_report (); 
			fprintf (fout, "===========");
			fprintf (fout, "============================================");
			fprintf (fout, "============================================");
			fprintf (fout, "===============\n");
			fprintf (fout, ".B2\n"); 
			fprintf (fout, ".E****** E N D    O F    R E P O R T ******\n");
			fflush (fout); 
			break;
		}
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (
 void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (pmat, pmat_list, PMAT_NO_FIELDS, "pmat_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpm);
	abc_fclose (pmpc);
	abc_fclose (pmpr);
	abc_fclose (cumr);
	abc_fclose (exsf);
	abc_fclose (pmat);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-----------------------------------------
	| Validate Project Code and Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("proj_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.proj_no, "~~~~~~~~");
			sprintf (local_rec.proj_desc, "%-40.40s", "All Projects.");
			DSP_FLD ("proj_no");
			DSP_FLD ("proj_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pmpm_rec.proj_no, local_rec.proj_no);
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPmMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.proj_desc, pmpm_rec.title);
		DSP_FLD ("proj_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate Salesman Code and Allow Search. |
	------------------------------------------*/
	if (LCHECK ("sman_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.sman_code, "~~");
			sprintf (local_rec.sman_name, "%-40.40s", "All Salesman.");
			DSP_FLD ("sman_code");
			DSP_FLD ("sman_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.tco_no);
		strcpy (exsf_rec.salesman_no, local_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.sman_name, exsf_rec.salesman);
		DSP_FLD ("sman_name");
		return (EXIT_SUCCESS);
	}

    /*-----------------------
    | Validate Ending Date. |
    -----------------------*/
	if ( LCHECK ("e_date"))
	{
		if (dflt_used)
		{
			local_rec.e_date = StringToDate (systemDate);
			DSP_FLD("e_date"); 
			if (local_rec.s_date > local_rec.e_date)
			{
				print_mess(ML(mlStdMess019));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}
	}

    /*-------------------------
    | Validate Printer Number |
    -------------------------*/
	if (LCHECK("prno"))
	{
		if (dflt_used)
		{
			local_rec.prno = 1;
			DSP_FLD ("prno");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			local_rec.prno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.prno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("prno");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	/*------------------
	| Open format file |
	------------------*/  
	if ((fout = popen ("pformat","w")) == 0)
		file_err (errno,"pformat","POPEN");

	sprintf (s_date, "%-10.10s", DateToString(local_rec.s_date));
	sprintf (e_date, "%-10.10s", DateToString(local_rec.e_date));

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.prno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L115\n");
	fprintf (fout, ".7\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EActions to be taken\n");
	fprintf (fout, ".Ebetween %s and %s\n", s_date, e_date);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECOMPANY %s\n", clip(comm_rec.tco_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".B1\n");
	fflush 	(fout); 
}

void
process_file (
 void)
{
	char	prev_actdate [11],
			prev_actproj [9],
			prev_actcont [7];

	fsort = sort_open ("pm_actlist");

	if (!strcmp (local_rec.proj_no, "~~~~~~~~"))
		strcpy (local_rec.proj_no, "        ");
	if (!strcmp (local_rec.sman_code, "~~"))
		strcpy (local_rec.sman_code, "  ");

	memset (&pmpc_rec, 0, sizeof (pmpc_rec));
	strcpy (pmpc_rec.proj_no, local_rec.proj_no);
	strcpy (pmpc_rec.cont_no, "      ");
	cc = find_rec (pmpc,&pmpc_rec, GTEQ, "r");
	while (!cc &&
			(!strcmp (pmpc_rec.proj_no, local_rec.proj_no) ||
			!strcmp (local_rec.proj_no, "        "))) 
	{
		if (!strcmp (pmpc_rec.sman_code, local_rec.sman_code) ||
			!strcmp (local_rec.sman_code, "  "))
		{ 
			memset (&pmat_rec, 0, sizeof (pmat_rec));
			strcpy (pmat_rec.proj_no, pmpc_rec.proj_no);
			strcpy (pmat_rec.cont_no, pmpc_rec.cont_no);
			pmat_rec.act_date = 0L;
			cc = find_rec (pmat,&pmat_rec, GTEQ, "r");
			while (!cc &&
					!strcmp(pmat_rec.proj_no, pmpc_rec.proj_no) &&
					!strcmp(pmat_rec.cont_no, pmpc_rec.cont_no)) 
			{
				if ((pmat_rec.next_act_date >= local_rec.s_date) &&
				   (pmat_rec.next_act_date <= local_rec.e_date)) 
				{ 
					if (strcmp (prev_actdate, DateToString(pmat_rec.next_act_date)))
						store_data ();
				} 
				if (strcmp(prev_actproj, pmat_rec.proj_no) &&
					strcmp(prev_actcont, pmat_rec.cont_no))
					strcpy (prev_actdate, DateToString(pmat_rec.next_act_date));

				strcpy (prev_actproj, pmat_rec.proj_no);
				strcpy (prev_actcont, pmat_rec.cont_no);
				cc = find_rec (pmat,&pmat_rec, NEXT, "r");
			}
		}
		cc = find_rec (pmpc,&pmpc_rec, NEXT, "r");
	}
}

void			
store_data (
 void)
{
 	char data_string [100];
	
	pass_data = TRUE;
    
    sprintf (data_string,"%-2.2s %-8.8s %-6.6s %-1.1s %-10.10s\n",
				pmpc_rec.sman_code,
				pmat_rec.proj_no,
				pmat_rec.cont_no,
				pmpc_rec.type,
				DateToString (pmat_rec.next_act_date));
	sort_save (fsort, data_string);
}

void
display_report (
 void)
{
    int 	first_time = TRUE;
	char	prev_sman [7];

	fsort = sort_sort(fsort, "pm_actlist");
	sptr =  sort_read(fsort);
	while (sptr != (char *)0)
	{
		sprintf (sman_code,	"%-2.2s",	sptr);
		sprintf (proj_no, 	"%-8.8s", sptr + 3);
		sprintf (cont_no, 	"%-6.6s", sptr + 12);
		sprintf (type,	 	"%-1.1s", sptr + 19);
		sprintf (next_date,	"%-10.10s", sptr + 21);
		if (first_time)
		{
			/* head_output(); */
			head_display (); 
			first_time = FALSE; 
		}
		else
		{
		 	if (strcmp(prev_sman, sman_code))
			{
				fprintf (fout, "===========");
				fprintf (fout, "============================================");
				fprintf (fout, "============================================");
				fprintf (fout, "===============\n"); 
				fprintf (fout, ".B2\n");
				head_display ();
				fprintf (fout, ".PA\n"); 
			}
		}
		strcpy (prev_sman, sman_code);
		print_detail ();  
		sptr	=	sort_read (fsort);
	}
	fflush (fout);
	sort_delete (fsort, "pm_actlist");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);       /* Set a screen ready for manipulation */

		clear();
		rv_pr(ML(mlPmMess012), (80 - strlen (ML(mlPmMess012)))/2,0,1);  /* Reverse video print */
        move(0,1);
        line(81);

		box(0,2,80,8);
		move(1,5);
		line(79); 
		print_at(6,1, ML(mlPmMess013));
		move(1,9);
		line(79); 
 
        move(0,20);       
        line(80);
		strcpy(err_str, ML(mlStdMess038));
		print_at(21,1,err_str,comm_rec.tco_no,comm_rec.tco_short);
		strcpy(err_str, ML(mlStdMess039));
		print_at(21,30,err_str,comm_rec.test_no,comm_rec.test_short);
		strcpy(err_str, ML(mlStdMess099));
		print_at(21,60,err_str,comm_rec.tcc_no,comm_rec.tcc_short);
        move(0,22);
        line(80);
		scn_write(scn);   /* Display all screen prompts */
	}
	return (EXIT_SUCCESS);
}

void
head_display (
 void)
{
	counter		= 0;

    if (first_time)
		first_time = 0;
	else
		fprintf (fout, ".DS5\n");   

	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	strcpy (exsf_rec.salesman_no, sman_code);
	cc = find_rec (exsf, &exsf_rec, EQUAL, "r");

	fprintf (fout, ".ESalesman :  %s - %s\n", 
							exsf_rec.salesman_no,
							exsf_rec.salesman);
	fprintf (fout, ".B1\n"); 
	fprintf (fout, ".C-----------");
	fprintf (fout, "--------------------------------------------");
	fprintf (fout, "--------------------------------------------");
	fprintf (fout, "---------------\n");
	fprintf (fout, ".C| Project  ");
	fprintf (fout, "|                Description                ");
	fprintf (fout, "|                Contractor                 ");
	fprintf (fout, "| Action Date |\n");
	fprintf (fout, ".C-----------");
	fprintf (fout, "+-------------------------------------------");
	fprintf (fout, "+-------------------------------------------");
	fprintf (fout, "+--------------\n");

	fflush(fout);  
	counter += 12;
}

void
print_detail (
 void)
{
	if (counter > 48)
	{
		fprintf (fout, ".C===========");
		fprintf (fout, "============================================");
		fprintf (fout, "============================================");
		fprintf (fout, "===============\n");
		head_display ();
		fprintf (fout, ".PA\n");
	}		

	strcpy (pmpm_rec.proj_no, proj_no);
	cc = find_rec (pmpm, &pmpm_rec, EQUAL, "r");
	if (!cc &&
		(!strcmp(pmpm_rec.proj_no, proj_no)))
		strcpy (proj_desc, pmpm_rec.title);

	if (!strcmp(type, "C"))
	{
		strcpy (cumr_rec.co_no, comm_rec.tco_no);
		strcpy (cumr_rec.est_no, comm_rec.test_no);
		strcpy (cumr_rec.dbt_no, zero_pad (cont_no,6));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp(cumr_rec.co_no, comm_rec.tco_no) &&
			!strcmp(cumr_rec.est_no, comm_rec.test_no) &&
			!strcmp(cumr_rec.dbt_no, zero_pad (cont_no,6))))
			strcpy (cont_name, cumr_rec.dbt_name);
		else
			strcpy (cont_name, "                                        ");
	}
	else
	{
		strcpy (pmpr_rec.prospect_no, cont_no);
		cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp(pmpr_rec.prospect_no, cont_no)))
			strcpy (cont_name, pmpr_rec.name);
		else
			strcpy (cont_name, "                                        "); 
	}

	fprintf (fout, ".C| %-8.8s ", proj_no);
	fprintf (fout, "| %-40.40s  ", proj_desc);
	fprintf (fout, "| %-40.40s  ", cont_name); 
	fprintf (fout, "| %-10.10s  |\n", next_date);
	counter ++;
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

/*=====================+
| Search Salesman File |
======================*/
void
SrchExsf (
 char *	key_val)
{
	work_open();
	save_rec("#Code   ","#Description");
	strcpy (exsf_rec.co_no, comm_rec.tco_no); 
	strcpy (exsf_rec.salesman_no, key_val); 
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp(exsf_rec.co_no, comm_rec.tco_no) &&
				!strncmp (exsf_rec.salesman_no, key_val, strlen(key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (exsf_rec.co_no, comm_rec.tco_no);
		strcpy (exsf_rec.salesman_no, temp_str);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, exsf, "DBFIND");
	}
}

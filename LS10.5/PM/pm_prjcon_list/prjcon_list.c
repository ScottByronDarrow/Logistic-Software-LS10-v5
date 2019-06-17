/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_prjcon_list.c )                               |
|  Program Desc  : ( Project-Contractor Report Listing.           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  pmpm, pmpc, pmpr, cumr, comm,     ,     , 		  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 05/01/96         |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified by : Ana Marie Tario.   |
|  Date Modified : (16/10/97)      | Modified by : Ana Marie Tario.   |
|  Date Modified : (15/05/1998)    | Modified by : Leah Manibog.      |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
| (12/09/97)   : - Incorporated multilingual conversion and DMY4 date.|
| (16/10/97)   : - Changed invoice length from 6 to 8.                |
| (15/05/1998) : - SEL - Fixed bug regarding the report printing of   |
|              :   Contractor, Parent and Salesman field.             |
| (17/09/1999) : - Ported to ANSI standards.                          |
|                                                                     |
| $Log: prjcon_list.c,v $
| Revision 5.3  2002/07/17 09:57:32  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:15:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:53  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:04  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:36:35  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/16 02:56:14  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.10  1999/09/29 10:11:48  scott
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
char	*PNAME = "$RCSfile: prjcon_list.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_prjcon_list/prjcon_list.c,v 5.3 2002/07/17 09:57:32 scott Exp $";

#include <ml_pm_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#define ACT_ST		(local_rec.proj_status[0] == 'A')
#define WON_ST		(local_rec.proj_status[0] == 'W')
#define LST_ST		(local_rec.proj_status[0] == 'L')
#define CON_ST		(local_rec.proj_status[0] == 'N')
#define COS_ST		(local_rec.proj_status[0] == 'S')

int 	first_time	= 1,
		counter		= 0,
		pass_data	= 0;

int		first_proj	= 1;

char 	*comm = "comm",
     	*pmpm = "pmpm",
		*pmpc = "pmpc",
     	*cumr = "cumr",
		*pmpr = "pmpr",
		*exsf = "exsf",
		*data = "data",
		*sptr;

char	proj_no	[9],
		prev_proj_no [9],
		proj_desc [41],
		prev_proj_desc [41],
		cont_no [7],
		cont_name [41],
		type [2],
		parent_no [7],
		parent_name [41],
		sman_code [3],
		sman_name [41],
		status [2],
		status_desc [21];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list [] = {
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
		char	tco_no[3];
		char	tco_name[41];
        char    tco_short[16];
		char	test_no[3];
		char  	test_name[41];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
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

	char 	systemDate [11];
	long 	lsystemDate;

	FILE	*fout,
			*fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	proj_status [2];
	char	status_desc [21];
    int     prno;
	} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "proj_status",	 4, 20, CHARTYPE,
		"U", "          ",
		" ", "W", "Project Status     :", "Status: (A)ctive, Order (W)on or (L)ost, Contractor wo(N) or lo(S)t.",
		YES, NO, JUSTLEFT, "AWLNS", "", local_rec.proj_status},
	{1, LIN, "status_desc",	 4, 37, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.status_desc},
	{1, LIN, "prno", 	5, 20, INTTYPE, 
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


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);
	OpenDB	();

	strcpy (systemDate, DateToString (TodaysDate()));

	/*=============================
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

		/*=================================
		|     Entry screen 1 linear input |
		=================================*/
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
			dsp_screen("Printing Project-Contractor Listing", 
						comm_rec.tco_no, comm_rec.tco_name);
            process_file ();
			if (pass_data)
				display_report ();
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

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------
	| Validate Project Status. |
	--------------------------*/
	if (LCHECK("proj_status"))
	{
		if (dflt_used)
		{ 
			strcpy  (local_rec.proj_status, "~");
 			sprintf (local_rec.status_desc, "%-20.20s", "All Project Status.");
			DSP_FLD ("proj_status");
			DSP_FLD ("status_desc");
   			return (EXIT_SUCCESS);
		}
		if (ACT_ST)
		{
			strcpy (local_rec.proj_status, "A");
 			sprintf (local_rec.status_desc, "%-20.20s", "Active");
		}
		if (WON_ST)
		{
			strcpy (local_rec.proj_status, "W");
 			sprintf (local_rec.status_desc, "%-20.20s", "Order Won");
		}
		if (LST_ST)
		{
			strcpy (local_rec.proj_status, "L");
 			sprintf (local_rec.status_desc, "%-20.20s", "Order Lost");
		}
		if (CON_ST)
		{
			strcpy (local_rec.proj_status, "N");
 			sprintf (local_rec.status_desc, "%-20.20s", "Contractor Won");
		}
		if (COS_ST)
		{
			strcpy (local_rec.proj_status, "S");
 			sprintf (local_rec.status_desc, "%-20.20s", "Contractor Lost");
		}
		DSP_FLD ("proj_status");
		DSP_FLD ("status_desc");
   		return (EXIT_SUCCESS);
	}

    /*-------------------------
    | Validate Printer Number |
    -------------------------*/
	if (LCHECK ("prno"))
	{
		if (dflt_used)
		{
			local_rec.prno = 1;
			DSP_FLD ("prno");
			return (EXIT_SUCCESS);
		}

		if (last_char == SEARCH)
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
	if ((fout = popen ("pformat", "w")) == 0)
		file_err (errno,"pformat", "POPEN");

	sprintf (err_str, "%s <%s>", systemDate, PNAME);
	fprintf (fout, ".START%s\n", clip (err_str));
	fprintf (fout, ".LP%d\n", local_rec.prno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L180\n");
	fprintf (fout, ".6\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EPROJECT-CONTRACTOR LISTING\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECOMPANY %s\n", clip (comm_rec.tco_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fflush (fout);
}

void
process_file (
 void)
{
	char blank_status [2];

	fsort = sort_open ("pm_prconlist");

	if (!strcmp (local_rec.proj_status, "~"))
		strcpy (local_rec.proj_status, "  ");

	sprintf (blank_status ,"%-1.1s",local_rec.proj_status);

	memset (&pmpc_rec, 0, sizeof (pmpc_rec));
	strcpy (pmpc_rec.proj_no, "        ");
	strcpy (pmpc_rec.cont_no, "      ");
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (pmpc_rec.status, local_rec.proj_status) ||
		   !strcmp (blank_status," "))
			store_data ();
		cc = find_rec (pmpc,&pmpc_rec, NEXT, "r");
	}
}

void
store_data (
 void)
{
 	char data_string [150];
	
	pass_data = TRUE;
    
    sprintf (data_string,
			 "%-8.8s %-6.6s %-1.1s %-6.6s %-2.2s %-1.1s\n",
			 pmpc_rec.proj_no,
			 pmpc_rec.cont_no,
			 pmpc_rec.type,
			 pmpc_rec.parent_no,
			 pmpc_rec.sman_code,        
			 local_rec.proj_status);
	sort_save (fsort, data_string);
}

void
display_report (
 void)
{
    int first_time = TRUE;

	fsort = sort_sort (fsort, "pm_prconlist");
	sptr =  sort_read (fsort);
	while (sptr != (char *)0)
	{
		sprintf (proj_no,	"%-8.8s", sptr);
		sprintf (cont_no, 	"%-6.6s", sptr + 9);
		sprintf (type,	 	"%-1.1s", sptr + 16);
		sprintf (parent_no,	"%-6.6s", sptr + 18);
		sprintf (sman_code,	"%-2.2s", sptr + 25);
		sprintf (status,	"%-1.1s", sptr + 28);

		if (first_time)
		{
			head_output ();
			head_display (); 
			first_time = FALSE;
		}
		print_detail ();
		sptr = sort_read (fsort);
	}
	fprintf (fout, "===============");
	fprintf (fout, "==========================================");
	fprintf (fout, "==========================================");
	fprintf (fout, "============================================");
	fprintf (fout, "=================================\n");
	fprintf (fout, ".B2\n"); 
	fprintf (fout, ".E****** E N D    O F    R E P O R T ******\n");
	fflush (fout);
	sort_delete (fsort, "pm_prconlist");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_FAILURE);
	
	if (scn != cur_screen)
		scn_set(scn);       /* Set a screen ready for manipulation */

	strcpy (err_str, ML (mlPmMess022));

	clear ();
	rv_pr (err_str, (80 - strlen (err_str))/2, 0, 1);  /* Reverse video print */
	move (0, 1);
	line (81);
	move (0, 1);
	line (81);

	box (0, 3, 80, 2);
 
	move (0, 20);
	line (80);
    strcpy (err_str, ML (mlStdMess038));
    print_at (21, 1, err_str,comm_rec.tco_no, comm_rec.tco_short);
    strcpy (err_str, ML (mlStdMess039));
    print_at (21, 30, err_str,comm_rec.test_no, comm_rec.test_short);
 	strcpy (err_str, ML (mlStdMess099));
    print_at (21, 60, err_str,comm_rec.tcc_no, comm_rec.tcc_short);
    move (0, 22);
    line (80);

	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
head_display (
 void)
{
	counter	= 0;

    if (first_time)
		first_time = 0;
	else
		fprintf (fout, ".DS5\n");   

	if (!strcmp (status, " "))
		strcpy (status_desc, "ALL");
	if (!strcmp (status, "A"))
		strcpy (status_desc, "Active");
	if (!strcmp (status, "W"))
		strcpy (status_desc, "Order Won");
	if (!strcmp (status, "L"))
		strcpy (status_desc, "Order Lost");
	if (!strcmp (status, "N"))
		strcpy (status_desc, "Contractor Won");
	if (!strcmp (status, "S"))
		strcpy (status_desc, "Contractor Lost");

	fprintf (fout, ".EFor Project Status :  %s\n", clip (status_desc));
	fprintf (fout, ".B2\n"); 
	fprintf (fout, "---------------");
	fprintf (fout, "------------------------------------------");
	fprintf (fout, "------------------------------------------");
	fprintf (fout, "--------------------------------------------");
	fprintf (fout, "---------------------------------\n");
	fprintf (fout, "| Project Code ");
	fprintf (fout, "|               Description               ");
	fprintf (fout, "|               Contractor                ");
	fprintf (fout, "|                  Parent                   ");
	fprintf (fout, "|           Salesman            |\n");
	fprintf (fout, "---------------");
	fprintf (fout, "+-----------------------------------------");
	fprintf (fout, "+-----------------------------------------");
	fprintf (fout, "+-------------------------------------------");
	fprintf (fout, "+--------------------------------\n");

	fflush (fout);  
	counter += 11;
}

void
print_detail (
 void)
{
	if (counter > 48)
	{
		fprintf (fout, "===============");
		fprintf (fout, "==========================================");
		fprintf (fout, "==========================================");
		fprintf (fout, "============================================");
		fprintf (fout, "=================================\n");
		head_display ();
		fprintf (fout, ".PA\n");
	}		

	strcpy (pmpm_rec.proj_no, proj_no);
	cc = find_rec (pmpm, &pmpm_rec, EQUAL, "r");
	if (!cc &&
		(!strcmp (pmpm_rec.proj_no, proj_no)))
		strcpy (proj_desc, pmpm_rec.title);

	if (!strcmp (type, "C"))
	{
		strcpy (cumr_rec.co_no, comm_rec.tco_no);
		strcpy (cumr_rec.est_no, comm_rec.test_no);
		strcpy (cumr_rec.dbt_no, cont_no);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp (cumr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (cumr_rec.est_no, comm_rec.test_no) &&
			!strcmp (cumr_rec.dbt_no, cont_no)))
			strcpy (cont_name, cumr_rec.dbt_name);
		else
			strcpy (cont_name, "                                        ");

		strcpy (cumr_rec.co_no, comm_rec.tco_no);
		strcpy (cumr_rec.est_no, comm_rec.test_no);
		strcpy (cumr_rec.dbt_no, parent_no);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp (cumr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (cumr_rec.est_no, comm_rec.test_no) &&
			!strcmp (cumr_rec.dbt_no, parent_no)))
			strcpy (parent_name, cumr_rec.dbt_name);
		else
		{
			if (!strcmp (parent_no, "0     ")) 
				strcpy (parent_name, "Main Contractor");
			else
				strcpy (parent_name, "               ");
		}
	}
	else
	{
		strcpy (pmpr_rec.prospect_no, cont_no);
		cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp (pmpr_rec.prospect_no, cont_no)))
			strcpy (cont_name, pmpr_rec.name);
		else
			strcpy (cont_name, "                                        "); 

		strcpy (pmpr_rec.prospect_no, parent_no);
		cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
		if (!cc &&
			(!strcmp (pmpr_rec.prospect_no, parent_no)))
			strcpy (parent_name, pmpr_rec.name);
		else
		{
			if (!strcmp (parent_no, "0     ")) 
				strcpy (parent_name, "Main Contractor");
			else
				strcpy (parent_name, "               "); 
		}
	}

	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	strcpy (exsf_rec.salesman_no, sman_code);
	cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
	if (!cc &&
		(!strcmp (exsf_rec.co_no, comm_rec.tco_no) &&
		!strcmp (exsf_rec.salesman_no, sman_code)))
		strcpy (sman_name, exsf_rec.salesman);

	if (strcmp (prev_proj_no, proj_no))
	{
		if (!first_proj)
		{
			fprintf (fout, "|              ");
			fprintf (fout, "|                                         ");
			fprintf (fout, "|                                         ");
			fprintf (fout, "|                                           ");
			fprintf (fout, "|                               |\n");
			counter++;
		}
		fprintf (fout, "| %-8.8s     ", proj_no);
		first_proj = FALSE;
	}
	else
		fprintf (fout, "|              ");
	
	if (strcmp (prev_proj_desc, proj_desc))
		fprintf (fout, "| %-40.40s", proj_desc);
	else
		fprintf (fout, "|                                         ");

	fprintf (fout, "| %-40.40s", cont_name); 
	fprintf (fout, "| %-40.40s  ", parent_name);
	fprintf (fout, "| %-29.29s |\n", sman_name);
	counter++;

	strcpy (prev_proj_no, proj_no);
	strcpy (prev_proj_desc, proj_desc);
}


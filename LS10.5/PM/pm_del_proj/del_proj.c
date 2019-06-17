/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( del_proj.c )  	                                  |
|  Program Desc  : ( Inactive Projects Deletion Program.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  pmpm, pmpc, pmpq, pmat, pmnt, comm,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 16/01/96         |
|---------------------------------------------------------------------|
|  Date Modified : 12/09/97        | Modified  by : Ana Marie Tario.  |
|  Date Modified : 16/10/97        | Modified  by : Leah Manibog.     |
|  Date Modified : 17/09/1999      | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : 12/09/97 - Incorporated multilingual conversion and|
|                :            DMY4 date.							  |
|  16/10/97 	 : Changed length of project no. from 6 to 8 char.	  |
|  17/09/1999    : Ported to ANSI standards.                          |
|                                                                     |
| $Log: del_proj.c,v $
| Revision 5.2  2001/08/09 09:15:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:01  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/11/16 02:56:13  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.9  1999/09/29 10:11:45  scott
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
char	*PNAME = "$RCSfile: del_proj.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_del_proj/del_proj.c,v 5.2 2001/08/09 09:15:02 scott Exp $";

#include	<ml_pm_mess.h>	
#include	<ml_std_mess.h>	
#include	<pslscr.h>	
#include	<get_lpno.h>
#include 	<qt_status.h>

	char	*data = "data",
			*comm = "comm",	
			*pmpm = "pmpm",	
			*pmpc = "pmpc",	
	    	*pmpq = "pmpq",	
	    	*pmat = "pmat",	
	    	*pmnt = "pmnt";	

	char	branchNo [3];
	int		envDbCo = 0,
			envDbFind	 = 0;


	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_cc_no"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 7;
	
	struct {
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		char	cc_no [3];
		long	t_dbt_date;
	} comm_rec;

	/*=========================================+
	 | Project Monitoring Project Master file. |
	 +=========================================*/
#define	PMPM_NO_FIELDS	3

	struct dbview	pmpm_list [PMPM_NO_FIELDS] =
	{
		{"pmpm_proj_no"},
		{"pmpm_title"},
		{"pmpm_status"}
	};

	struct tag_pmpmRecord
	{
		char	proj_no [9];
		char	title [41];
		char	status [2];
	}	pmpm_rec;

	/*=========================+
	 | Project-Contractor File |
	 +=========================*/
#define	PMPC_NO_FIELDS	4

	struct dbview	pmpc_list [PMPC_NO_FIELDS] =
	{
		{"pmpc_proj_no"},
		{"pmpc_cont_no"},
		{"pmpc_type"},
		{"pmpc_parent_no"}
	};

	struct tag_pmpcRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	type [2];
		char	parent_no [7];
	}	pmpc_rec;

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
		Date	act_date;
		char	act_details [81];
		Date	next_act_date;
	}	pmat_rec;

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


/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	resp [2];
	int	lpno;
} local_rec;


static struct	var vars [] = {

	{1, LIN, "resp",	10, 50, CHARTYPE,
		"U", "          ",
		" ", "N", "Do you wish to continue? (Y/N)", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.resp},
	{0, LIN, "", 0, 0, INTTYPE,
		"A","          ",
		" ","", "dummy"," ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB			(void);
int		spec_valid		(int);
void	process			(void);
void	delete_file		(void);
int		heading			(int);
void	FlashMess		(int, char *, int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	clear ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	envDbCo  = atoi (get_env ("DB_CO"));
	envDbFind	  = atoi (get_env ("DB_FIND"));
	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

	OpenDB ();
	
	while (prog_exit == 0)
	{
		search_ok	= 1;
		entry_exit	= 1;
		restart		= 0;
		prog_exit	= 0; 
		init_vars (1); 
		heading (1);
		entry (1);
		while (prog_exit == 0 && restart == 0)
		{
			if (entry_exit)
			{
				heading (1);
				scn_display (1);
				if (!restart)
				{
					if (!strcmp (local_rec.resp, "Y")) 
						process (); 
					else
					{
						shutdown_prog ();
						return (EXIT_SUCCESS);
					}
				}
				restart = 1; 
			}
		}
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

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
	open_rec (pmpq, pmpq_list, PMPQ_NO_FIELDS, "pmpq_id_no");
	open_rec (pmat, pmat_list, PMAT_NO_FIELDS, "pmat_id_no");
	open_rec (pmnt, pmnt_list, PMNT_NO_FIELDS, "pmnt_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose (pmpm);
	abc_fclose (pmpc);
	abc_fclose (pmpq);
	abc_fclose (pmat);
	abc_fclose (pmnt);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{

	if (LCHECK("resp"))
	{
		if (prog_status == ENTRY && FLD ("resp") == NA)
				return (EXIT_SUCCESS);

		if (strcmp (local_rec.resp,"Y") && strcmp(local_rec.resp, "N"))
		{
			print_err (ML (mlStdMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("resp");
		entry_exit = TRUE; 
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
process (
 void)
{

	int	del_found = FALSE;

	memset (&pmpm_rec, 0, sizeof (pmpm_rec));
	strcpy (pmpm_rec.proj_no, "        ");
	cc = find_rec (pmpm, &pmpm_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (pmpm_rec.status, "I"))
		{
			FlashMess (21, ML (mlStdMess014), 4);
			del_found = TRUE;
			delete_file ();
		}

		cc = find_rec (pmpm, &pmpm_rec, NEXT, "r");
		continue;
	}
	if (!del_found)
	{
		FlashMess (21, ML (mlStdMess015), 4);
		del_found = FALSE;
	}
}

void
delete_file (
 void)
{
	char	cmd [100];
	char	directory [101];
	char *	sptr;

	sptr	= getenv ("PROG_PATH");

	sprintf( directory, "%s/BIN/PNT", (sptr == (char *)0) ? "/usr/ver9" : sptr);

	/*--------------
	| Delete File. |
	--------------*/
	strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmpc_rec.cont_no, "      ");
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc && !strcmp(pmpc_rec.proj_no, pmpm_rec.proj_no))
	{
		cc = abc_delete (pmpc);
		if (cc)
			file_err (cc, pmpc, "DBDELETE"); 
							
		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}

	strcpy (pmpq_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmpq_rec.cont_no, "      ");
	cc = find_rec (pmpq, &pmpq_rec, GTEQ, "r");
	while (!cc && !strcmp(pmpq_rec.proj_no, pmpm_rec.proj_no))
	{
		cc = abc_delete (pmpq);
		if (cc)
			file_err (cc, pmpq, "DBDELETE"); 
					
		cc = find_rec (pmpq, &pmpq_rec, NEXT, "r");
	}

	strcpy (pmat_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmat_rec.cont_no, "      ");
	cc = find_rec (pmat, &pmat_rec, GTEQ, "r");
	while (!cc && !strcmp(pmat_rec.proj_no, pmpm_rec.proj_no))
	{
		cc = abc_delete (pmat);
		if (cc)
			file_err (cc, pmat, "DBDELETE"); 
					
		cc = find_rec (pmat, &pmat_rec, NEXT, "r");
	}

	strcpy (pmnt_rec.proj_no, pmpm_rec.proj_no);
	strcpy (pmnt_rec.cont_no, "      ");
	cc = find_rec (pmnt, &pmnt_rec, GTEQ, "r");
	while (!cc && !strcmp(pmnt_rec.proj_no, pmpm_rec.proj_no))
	{
		sprintf(cmd, "rm %s/%-12.12s", directory, pmnt_rec.filename);
		sys_exec (cmd);
		
		cc = abc_delete (pmnt);
		if (cc)
			file_err (cc, pmnt, "DBDELETE"); 
				
		cc = find_rec (pmnt, &pmnt_rec, NEXT, "r");
	}
 	cc = abc_delete (pmpm);
	if (cc)
		file_err (cc, pmpm, "DBDELETE"); 
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		move (0, 1);
		line (80);

		rv_pr (ML (mlPmMess021), 28, 0, 1);

		box (15, 5, 50, 6);
		
		print_at (7, 21, ML (mlPmMess019));
		print_at (8, 21, ML (mlPmMess020));
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
FlashMess (
 int	x_pos,
 char *	mess,
 int	MessDelay)
{
	int	i;
	
	for (i = 0;i < MessDelay; i++)
	{
		move (0, 23);
		rv_pr (err_str, (80 - strlen (mess)) /2, x_pos, (i % 2));
		sleep (sleepTime);
	}
	move (0, x_pos); cl_line();
}	

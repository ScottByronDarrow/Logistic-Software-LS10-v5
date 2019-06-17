/*====================================================================|
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( essr_lists.c     )                               |
|  Program Desc  : ( Security file print program.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, essr,     ,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth | Date Written : 16/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (16/03/89)       | Modified  by : Huon Butterworth |
|  Date Modified : (24/09/97)       | Modified  by : Ana Marie Tario  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: srlist.c,v $
| Revision 5.2  2001/08/09 09:14:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:54  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:28  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.11  1999/12/10 04:25:02  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.10  1999/12/06 01:47:32  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/09/29 10:11:30  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/20 05:51:25  scott
| Updated from Ansi Project.
|
| Revision 1.7  1999/09/10 02:13:17  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.6  1999/06/15 09:39:19  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: srlist.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/srlist/srlist.c,v 5.2 2001/08/09 09:14:26 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		};

	int comm_no_fields = 6;

	struct {
		int 	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	test_no[3];
		char 	test_name[41];
		long 	tdbt_date;
		} comm_rec;

	/*================================
	| Branch Security File Record.   |
	================================*/
	struct dbview essr_list[] ={
		{"essr_co_no"},
		{"essr_est_no"},
		{"essr_op_id"},
		{"essr_op_name"},
		{"essr_sec_level"}
		};

	int essr_no_fields = 5;

	struct {
		char	sr_co_no   [3],
			sr_est_no  [3],
			sr_op_id   [15],
			sr_op_name [41];
		int	sr_sec_level;
		}
		    essr_rec;

	double	rep_total[4];
	double	grand_tot;

	int	lp_no = 1;

	FILE	*pp;
	
	static char *rep_desc = "BRANCH SECURITY FILE REPORT.";

#include	<std_decs.h>
void OpenDB (void);
void CloseDB (void);
void start_report (int prnt_no);
void proc_file (void);
void end_report (void);

int
main (
	int argc, 
	char *argv [])
{

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036, argv[0]);
	    return (EXIT_FAILURE);
	}

	lp_no = atoi (argv[1]);

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sprintf (err_str,"PRINTING %s",rep_desc);
	dsp_screen (err_str,comm_rec.tco_no,comm_rec.tco_name);
	start_report (lp_no);

	proc_file ();
	
	end_report ();
	pclose (pp);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
	void)
{
	abc_dbopen ("data");
	open_rec ("essr", essr_list, essr_no_fields, "essr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
	void)
{
	abc_fclose ("essr");
	abc_dbclose ("data");
}

void
start_report (
	int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ( (pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	fprintf (pp,".13\n");
	fprintf (pp,".L80\n");
	fprintf (pp,".E%s\n",rep_desc);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",comm_rec.tco_name);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",comm_rec.test_name);
	fprintf (pp,".B1\n");
	fprintf (pp,".B1\n.EAS AT : %s.B1\n", SystemTime ());

	fprintf (pp, ".R          =================");
	fprintf (pp, "==========================================\n");

	fprintf (pp, "          =================");
	fprintf (pp, "==========================================\n");

	fprintf (pp, "          |  OPERATOR ID.  ");
	fprintf (pp, "|              OPERATOR NAME             |\n");

	fprintf (pp, "          |----------------");
	fprintf (pp, "|----------------------------------------|\n");

	fprintf (pp,".PI12\n");
}

/*===========================
| Validate and print lines. |
===========================*/
void
proc_file (
	void)
{
	int	find_type = GTEQ;

	strcpy (essr_rec.sr_co_no, comm_rec.tco_no);
	strcpy (essr_rec.sr_est_no, comm_rec.test_no);
	strcpy (essr_rec.sr_op_id, "              ");

	while (1)
	{
		cc = find_rec ("essr", &essr_rec, find_type, "r");
		if (cc || ( strcmp (essr_rec.sr_co_no, comm_rec.tco_no) ||
				strcmp (essr_rec.sr_est_no, comm_rec.test_no)))
			break;

		fprintf (pp,"          ");
		fprintf (pp,"| %14.14s ",essr_rec.sr_op_id);
		fprintf (pp,"|%40.40s|\n",essr_rec.sr_op_name);

		find_type = NEXT;
		dsp_process ("Operator : ", essr_rec.sr_op_id);
	}
}
		
/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (
	void)
{
	fprintf (pp,".EOF\n");
}


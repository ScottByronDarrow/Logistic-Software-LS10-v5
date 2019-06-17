/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ex_lists.c     )                                 |
|  Program Desc  : ( External file print program.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, exsf, exaf, excf,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/01/87         |
|---------------------------------------------------------------------|
|  Date Modified : (27/01/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (03/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (30/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      : (03/09/97) : Incorporated multilingual conversion  |
|                :            : and DMY4 date.                        | 
|                :                                                    |
|                : (30/08/1999) : Converted to ANSI format.           |
|                :                                                    |
|                                                                     |
| $Log: ex_lists.c,v $
| Revision 5.2  2001/08/09 05:13:26  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:59  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:11  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:12  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/09/29 10:11:05  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:26:55  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/06/15 02:35:59  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ex_lists.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ex_lists/ex_lists.c,v 5.2 2001/08/09 05:13:26 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_menu_mess.h>
#include	<ml_std_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>

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
		int termno;
		char tco_no[3];
		char tco_name[41];
		char test_no[3];
		char test_name[41];
		long tdbt_date;
		} comm_rec;

	/*=====================
	| External Area file. |
	=====================*/
	struct dbview exaf_list[] ={
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
		{"exaf_stat_flag"}
		};

	int exaf_no_fields = 4;

	struct {
		char af_co_no[3];
		char af_area_code[3];
		char af_desc[41];
		char af_stat_flag[2];
		} exaf_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
		{"excf_stat_flag"}
		};

	int excf_no_fields = 4;

	struct {
		char cf_co_no[3];
		char cf_categ_no[12];
		char cf_description[41];
		char cf_stat_flag[2];
		} excf_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_stat_flag"}
		};

	int exsf_no_fields = 4;

	struct {
		char sf_co_no[3];
		char sf_salesman_no[3];
		char sf_desc[41];
		char sf_stat_flag[2];
		} exsf_rec;

	double	rep_total[4];
	double	grand_tot;

	int	rep_type = 1,
		lp_no = 1;

	int	find_type = GTEQ;

	FILE	*pp;
	
	static char *rep_desc[] = {
 		"SALESMAN MASTER FILE REPORT.",
 		"STOCK CATEGORY MASTER FILE REPORT.",
 		"AREA MASTER FILE REPORT."
	};

/*=========================
| Function prototypes     |
=========================*/
int		main		(int argc, char * argv []);
void	OpenDB		(void);
void	CloseDB	(void);
void	start_report(int prnt_no);
void	proc_file	(void);
void	prnt_sman	(void);
void	prnt_cat	(void);
void	prnt_area	(void);
void	end_report	(void);

int
main (
 int	argc,
 char *	argv [])
{

	if (argc != 3)
	{
		print_at(0,0,mlMenuMess706,argv[0]);
	    return (EXIT_FAILURE);
	}

	rep_type = atoi(argv[2]);
	lp_no = atoi(argv[1]);

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sprintf(err_str,"PRINTING %s",rep_desc[rep_type - 1]);
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
	start_report(lp_no);

	proc_file();
	
	end_report();
	pclose(pp);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	open_rec("exaf", exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec("excf", excf_list, excf_no_fields, "excf_id_no");
	open_rec("exsf", exsf_list, exsf_no_fields, "exsf_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose("exsf");
	abc_fclose("exaf");
	abc_fclose("excf");
	abc_dbclose("data");
}

void
start_report (
 int	prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf(pp,".START%s<%s>\n", DateToString(comm_rec.tdbt_date), PNAME);
	fprintf(pp,".LP%d\n",prnt_no);
	fprintf(pp,".13\n");
	fprintf(pp,".L80\n");
	fprintf(pp,".E%s\n",rep_desc[rep_type - 1]);
	fprintf(pp,".B1\n");
	fprintf(pp,".E%s\n",comm_rec.tco_name);
	fprintf(pp,".B1\n");
	fprintf(pp,".B1\n.EAS AT : %s.B1\n", SystemTime ());

	fprintf(pp, ".R          =================");
	fprintf(pp, "==========================================\n");

	fprintf(pp, "          =================");
	fprintf(pp, "==========================================\n");

	switch (rep_type)	
	{
	   case 1:
		fprintf(pp, "          |SALESMAN  NUMBER");
		fprintf(pp, "|           SALESMAN DESCRIPTION         |\n");
	   break;

	   case 2:
		fprintf(pp, "          |CATEGORY  NUMBER");
		fprintf(pp, "|           CATEGORY DESCRIPTION         |\n");
	   break;

	   case 3:
		fprintf(pp, "          |   AREA NUMBER  ");
		fprintf(pp, "|            AREA  DESCRIPTION           |\n");
	   break;
	}

	fprintf(pp, "          |----------------");
	fprintf(pp, "|----------------------------------------|\n");

	fprintf(pp,".PI12\n");
}


/*===========================
| Validate and print lines. |
===========================*/
void
proc_file (void)
{

	strcpy(exaf_rec.af_co_no, comm_rec.tco_no);
	strcpy(exaf_rec.af_area_code, "  ");

	strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
	strcpy(exsf_rec.sf_salesman_no, "  ");

	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	strcpy(excf_rec.cf_categ_no, "           ");

	find_type = GTEQ;

	switch (rep_type)	
	{
	    case 1:
			prnt_sman();
			break;
	    case 2:
			prnt_cat();
			break;
	    case 3:
			prnt_area();
			break;
	}
}

void
prnt_sman(void)
{
	while(1)
	{
		cc = find_rec("exsf", &exsf_rec, find_type, "r");
		if (cc || strcmp(exsf_rec.sf_co_no, comm_rec.tco_no) != 0)
			break;

		fprintf(pp,"          ");
		fprintf(pp,"|      %2.2s        ",exsf_rec.sf_salesman_no);
		fprintf(pp,"|%40.40s|\n",exsf_rec.sf_desc);

		find_type = NEXT;
		dsp_process("Salesman : ",exsf_rec.sf_salesman_no);
	}
}

void
prnt_cat(void)
{
	while(1)
	{
		cc = find_rec("excf", &excf_rec, find_type, "r");
		if (cc || strcmp(excf_rec.cf_co_no, comm_rec.tco_no) != 0)
			break;

		fprintf(pp,"          ");
		fprintf(pp,"|  %11.11s   ",excf_rec.cf_categ_no);
		fprintf(pp,"|%40.40s|\n",excf_rec.cf_description);

		find_type = NEXT;
		dsp_process("Category : ",excf_rec.cf_categ_no);
	}
}

void
prnt_area (void)
{
	while(1)
	{
		cc = find_rec("exaf", &exaf_rec, find_type, "r");
		if (cc || strcmp(exaf_rec.af_co_no, comm_rec.tco_no) != 0)
			break;

		fprintf(pp,"          ");
		fprintf(pp,"|      %2.2s        ",exaf_rec.af_area_code);
		fprintf(pp,"|%40.40s|\n",exaf_rec.af_desc);

		find_type = NEXT;
		dsp_process("Area : ",exaf_rec.af_area_code);
	}
}
		
/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (void)
{
	fprintf(pp,".EOF\n");
}

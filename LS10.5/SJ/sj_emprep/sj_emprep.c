/*====================================================================r
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_emprep.c    )                                 |
|  Program Desc  : ( Detailed Job Analysis Report.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjld, sjsr, sjlr, cudp,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 04/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (05/12/88)      | Modified  by  :B.C.Lim.          |
|  Date Modified : (27/11/89)      | Modified  by  :Fui Choo Yap.     |
|  Date Modified : (19/09/97)      | Modified  by  :Rowena S Maandig  |
|  Date Modified : (03/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|                :                                                    |
|     (27/11/89) : Change sjld_km to floattype.                       |
|     (19/09/97) : Updated to incorporate multilingual conversion.    |
|                :                                                    |
|  (03/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                                                                     |
| $Log: sj_emprep.c,v $
| Revision 5.2  2001/08/09 09:17:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:07  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:11  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:14  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:43  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:34:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 05:58:30  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.8  1999/09/29 10:12:55  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/24 05:06:30  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:26  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/

#define	CCMAIN
char	*PNAME = "$RCSfile: sj_emprep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_emprep/sj_emprep.c,v 5.2 2001/08/09 09:17:22 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#define	NO_SCRGEN

#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>	
#include <ml_sj_mess.h>	

/*===================================
|   Constants, defines and stuff    |
===================================*/

#define	E_TIME		emp_total[0]
#define	E_TIME_HALF	emp_total[1]
#define	E_TIME_DOUBLE	emp_total[2]
#define	E_DM		emp_total[3]
#define	E_HM		emp_total[4]
#define	E_MM		emp_total[5]
#define	E_KK		emp_total[6]
#define	E_KM		emp_total[7]
#define	E_PROD		emp_total[8]

#define	R_TIME		rep_total[0]
#define	R_TIME_HALF	rep_total[1]
#define	R_TIME_DOUBLE	rep_total[2]
#define	R_DM		rep_total[3]
#define	R_HM		rep_total[4]
#define	R_MM		rep_total[5]
#define	R_KK		rep_total[6]
#define	R_KM		rep_total[7]
#define	R_PROD		rep_total[8]

/*  NOTES
    these should be declared as const char*
    to minimize potential problems.
*/
char *sjld = "sjld";
char *sjlr = "sjlr";
char *sjsr = "sjsr";
char *data = "data";

    /*=====================
    |   Local variables   |
    =====================*/
	int		lpno = 1;

	long	fr_date;
	long	to_date;

	FILE	*fout;

	double	emp_total[9],
			rep_total[9];

	char	prog_desc[51],
			s_fr_date[11],
			s_to_date[11];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] =
    {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
	};

	const int comm_no_fields = 7;

	struct 
    {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
	} comm_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list[] =
    {
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
	};

	const int cudp_no_fields = 4;

	struct 
    {
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_dp_name[41];
	} cudp_rec;

	/*====================
	| Labour detail file |
	====================*/
	struct dbview sjld_list[] =
    {
		{"sjld_co_no"},
		{"sjld_est_no"},
		{"sjld_dp_no"},
		{"sjld_order_no"},
		{"sjld_emp_code"},
		{"sjld_date"},
		{"sjld_km"},
		{"sjld_dm"},
		{"sjld_hm"},
		{"sjld_mm"},
		{"sjld_kk"},
		{"sjld_time"},
		{"sjld_time_half"},
		{"sjld_time_double"},
	};

	const int sjld_no_fields = 14;

	struct 
    {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
		float	ld_km;
		float	ld_dm;
		float	ld_hm;
		float	ld_mm;
		float	ld_kk;
		float	ld_time;
		float	ld_time_half;
		float	ld_time_double;
	} sjld_rec;

	/*====================
	| Serviceperson file |
	====================*/
	struct dbview sjsr_list[] =
    {
		{"sjsr_co_no"},
		{"sjsr_est_no"},
		{"sjsr_dp_no"},
		{"sjsr_code"},
		{"sjsr_name"},
	};

	const int sjsr_no_fields = 5;

	struct 
    {
		char	sr_co_no[3];
		char	sr_est_no[3];
		char	sr_dp_no[3];
		char	sr_code[11];
		char	sr_name[26];
	} sjsr_rec;

	/*===================
	| Labour rates file |
	===================*/
	struct dbview sjlr_list[] =
    {
		{"sjlr_co_no"},
		{"sjlr_est_no"},
		{"sjlr_dp_no"},
		{"sjlr_code"},
		{"sjlr_cost_hr"},
	};

	const int sjlr_no_fields = 5;

	struct 
    {
		char	lr_co_no[3];
		char	lr_est_no[3];
		char	lr_dp_no[3];
		char	lr_code[3];
		double	lr_cost_hr;
	} sjlr_rec;


/*===============================
|   Local function prototypes   |
===============================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void process_data (void);
void print_tot (char *tot_desc, double *tot_val);
void read_sjsr (void);
float  percent (double num, double den);
double get_rate (char *rate_code);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv[])
{
	if (argc != 5)
	{
		print_at(0,0,mlSjMess705,PNAME);

        return (EXIT_FAILURE);
		/* QUERY this was originally exit(argc); */
	}
	
	sprintf (prog_desc, "%-50.50s", argv[1]);
	lpno = atoi (argv[2]);
	fr_date = StringToDate (argv[3]);

	if (fr_date < 0L)
	{
		print_at (1,0, ML(mlSjMess045),argv[3]);
        return (EXIT_FAILURE);
		/* QUERY thus was originally exit(1); */
	}

	to_date = StringToDate (argv[4]);

	if (to_date < 0L)
	{
		print_at (2,0, ML (mlSjMess046), argv[4]);
		return (EXIT_FAILURE);
		/* QUERY thus was originally exit(1); */
	}
	sprintf (s_fr_date,"%-10.10s",argv[3]);
	sprintf (s_to_date,"%-10.10s",argv[4]);

	OpenDB ();
	ReadMisc ();

	init_scr ();

	dsp_screen ("Processing : Employee Analysis Report", 
                 comm_rec.tco_no, 
                 comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
    fout = popen("pformat","w");
	if (fout == 0)
    {
		sys_err("Error in pformat During (POPEN)", errno, PNAME);
    }

	head_output ();

	process_data ();

	fprintf (fout,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (fout);

	shutdown_prog ();

    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",lpno);
	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n",clip(prog_desc));
	fprintf (fout, 
             ".ECompany    : %s - %s\n",
             comm_rec.tco_no,
             clip (comm_rec.tco_name));
	fprintf (fout, 
             ".EBranch     : %s - %s\n",
             comm_rec.test_no,
             clip (comm_rec.test_name));
	fprintf (fout, 
             ".EDepartment : %s - %s\n",
             comm_rec.tdp_no,
             clip (comm_rec.tdp_name));
	fprintf (fout, ".EAS AT : %-24.24s\n", SystemTime ());
	fprintf (fout, ".EFROM  : %s  TO  : %s\n", s_fr_date, s_to_date);
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R==============================");
	fprintf (fout, "===========");
	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "==============================");
	fprintf (fout, "===========");
	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "|  E M P L O Y E E   N A M E  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "| JOB NO  ");
	fprintf (fout, "| ORD TIME ");
	fprintf (fout, "| TIME&1/2 ");
	fprintf (fout, "| DBL TIME ");
	fprintf (fout, "|   DIRT   ");
	fprintf (fout, "|  HEIGHT  ");
	fprintf (fout, "|   MEAL   ");
	fprintf (fout, "| KERRICK  ");
	fprintf (fout, "| KMETERS  |\n");

	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");

	fflush (fout);
}

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

    open_rec (sjld, sjld_list, sjld_no_fields, "sjld_id_no_2");
	open_rec (sjlr, sjlr_list, sjlr_no_fields, "sjlr_id_no");
	open_rec (sjsr, sjsr_list, sjsr_no_fields, "sjsr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sjld);
	abc_fclose (sjlr);
	abc_fclose (sjsr);
	abc_dbclose (data);
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void 
ReadMisc (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec ("cudp", cudp_list, cudp_no_fields, "cudp_id_no");
	strcpy (cudp_rec.dp_co_no, comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no, comm_rec.test_no);
	strcpy (cudp_rec.dp_dp_no, comm_rec.tdp_no);

	cc = find_rec ("cudp", &cudp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cudp", "DBFIND");

	abc_fclose ("cudp");
}

void
process_data (
 void)
{
	int	cnt = 0;
	int	i   = 0;
	long	old_order_no = 0L;

	for (i = 0;i < 9;i++)
    {
		emp_total[i] = 0.00;
    }

	for (i = 0;i < 9;i++)
    {
		rep_total[i] = 0.00;
    }

	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.test_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sprintf (sjld_rec.ld_emp_code,"%-10.10s"," ");
	sjld_rec.ld_date = fr_date;
	sjld_rec.ld_order_no = 0L;

	cc = find_rec (sjld,&sjld_rec,GTEQ,"r");

	old_order_no = sjld_rec.ld_order_no;

	while (!cc && 
           !strcmp(sjld_rec.ld_co_no,comm_rec.tco_no) && 
           !strcmp(sjld_rec.ld_est_no,comm_rec.test_no) && 
           !strcmp(sjld_rec.ld_dp_no,comm_rec.tdp_no))
	{
		if ((sjld_rec.ld_date >= fr_date) && 
            (sjld_rec.ld_date <= to_date))
		{
			read_sjsr ();

			if (old_order_no != sjld_rec.ld_order_no)
			{
				if (cnt)
                {
					print_tot("      EMPLOYEE TOTALS       ",emp_total);
                }
				old_order_no = sjld_rec.ld_order_no;
/*
its probably about here that we put the break down of costs
*/
				for (i = 0;i < 9;i++)
                {
					emp_total[i] = 0.00;
                }
				cnt = 0;
			}

			dsp_process ("Employee.",sjsr_rec.sr_name);

			cnt++;

			fprintf (fout, "| %-28.28s",	sjsr_rec.sr_name);
			fprintf (fout, "|%-10.10s",	DateToString(sjld_rec.ld_date));
			fprintf (fout, "|%8ld ",		sjld_rec.ld_order_no);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_time);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_time_half);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_time_double);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_dm);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_hm);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_mm);
			fprintf (fout, "|%9.2f ",	sjld_rec.ld_kk);
			fprintf (fout, "|%9.2f |\n",	sjld_rec.ld_km);

			if ((9999L < sjld_rec.ld_order_no) && 
                (sjld_rec.ld_order_no <= 99999L))
			{
				E_PROD += (double) sjld_rec.ld_time;
				E_PROD += (double) sjld_rec.ld_time_half;
				E_PROD += (double) sjld_rec.ld_time_double;
				R_PROD += (double) sjld_rec.ld_time;
				R_PROD += (double) sjld_rec.ld_time_half;
				R_PROD += (double) sjld_rec.ld_time_double;
			}

			E_TIME 			+= (double) sjld_rec.ld_time;
			E_TIME_HALF 	+= (double) sjld_rec.ld_time_half;
			E_TIME_DOUBLE 	+= (double) sjld_rec.ld_time_double;
			E_DM 			+= (double) sjld_rec.ld_dm;
			E_HM 			+= (double) sjld_rec.ld_hm;
			E_MM 			+= (double) sjld_rec.ld_mm;
			E_KK 			+= (double) sjld_rec.ld_kk;
			E_KM 			+= (double) sjld_rec.ld_km;

			R_TIME 			+= (double) sjld_rec.ld_time;
			R_TIME_HALF 	+= (double) sjld_rec.ld_time_half;
			R_TIME_DOUBLE 	+= (double) sjld_rec.ld_time_double;
			R_DM 			+= (double) sjld_rec.ld_dm;
			R_HM 			+= (double) sjld_rec.ld_hm;
			R_MM 			+= (double) sjld_rec.ld_mm;
			R_KK 			+= (double) sjld_rec.ld_kk;
			R_KM 			+= (double) sjld_rec.ld_km;

			fflush (fout);
		}
		cc = find_rec (sjld,&sjld_rec,NEXT,"r");
	}
	print_tot ("      EMPLOYEE TOTALS       ", emp_total);
	print_tot ("        GRAND TOTALS        ", rep_total);
}

void
print_tot (
 char *tot_desc, 
 double *tot_val)
{
	int	i;
	double	total = 0.00;

	fprintf (fout, ".LRP6\n");

	fprintf (fout, "| %-28.28s",tot_desc);
	fprintf (fout, " %9.9s "," ");
	fprintf (fout, " %9.9s"," ");

	for (i = 0;i < 8;i++)
    {
		fprintf(fout, (i == 7) ? "|%9.0f " : "|%9.2f ",tot_val[i]);
    }
	fprintf (fout, "|\n");

	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");

	fprintf (fout, "|%48.48s","TOTAL HOURS RECOVERED  ");
	fprintf (fout, "%10.2f",tot_val[8]);
	fprintf (fout, "%80.80s|\n"," ");

	total = tot_val[0] + tot_val[1] + tot_val[2];

	fprintf (fout, "|%48.48s","TOTAL HOURS WORKED     ");
	fprintf (fout, "%10.2f",total);
	fprintf (fout, "%25.25s","PERCENTAGE RECOVERED : ");
	fprintf (fout, "%9.2f%%",percent(tot_val[8],total));
	fprintf (fout, "%45.45s|\n"," ");

	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");
	
	fflush (fout);
}

/*=======================
| calculate percentage	|
=======================*/
float 
percent (
 double num, 
 double den)
{
    double result;

	if (den == 0.00)
    {
		return (0.00);
    }
    result = (num / den) * 100.00;
	
    return ((float)result);
}

double 
get_rate (
 char *rate_code)
{
	strcpy (sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.test_no);
	strcpy (sjlr_rec.lr_dp_no,sjld_rec.ld_dp_no);
	sprintf (sjlr_rec.lr_code,"%-2.2s",rate_code);

	cc = find_rec (sjlr,&sjlr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "sjlr", "DBFIND");

	return (sjlr_rec.lr_cost_hr);
}

void
read_sjsr (
 void)
{
	strcpy (sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy (sjsr_rec.sr_est_no,comm_rec.test_no);
	strcpy (sjsr_rec.sr_dp_no,sjld_rec.ld_dp_no);
	strcpy (sjsr_rec.sr_code,sjld_rec.ld_emp_code);

	cc = find_rec (sjsr,&sjsr_rec,COMPARISON,"r");
	if (cc)
    {
		sprintf (sjsr_rec.sr_name,"%-25.25s","Unknown Employee");
    }
}

/* [ end of file ] */

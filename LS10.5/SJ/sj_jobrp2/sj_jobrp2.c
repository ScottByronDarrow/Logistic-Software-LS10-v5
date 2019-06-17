/*====================================================================r
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_jobrp2.c    )                                 |
|  Program Desc  : ( Summarised Labour Analysis By Job Report.    )   |
|                  ( By dept by job between given dates           )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjld, sjsr, sjlr, cudp,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (06/12/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (27/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (02/08/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|    (27/11/89)  : Change sjld_km to type float.                      |
|                :                                                    |
|                                                                     |
| $Log: sj_jobrp2.c,v $
| Revision 5.2  2001/08/09 09:17:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:36  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:34:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/09/29 10:13:00  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/24 05:06:36  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:32  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_jobrp2.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_jobrp2/sj_jobrp2.c,v 5.2 2001/08/09 09:17:35 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>	
#include <ml_std_mess.h>	
#include <ml_sj_mess.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
	} comm_rec;

	/*====================
	| Labour detail file |
	====================*/
	struct dbview sjld_list[] ={
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

	int sjld_no_fields = 14;

	struct {
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
	struct dbview sjsr_list[] ={
		{"sjsr_co_no"},
		{"sjsr_est_no"},
		{"sjsr_dp_no"},
		{"sjsr_code"},
		{"sjsr_name"},
	};

	int sjsr_no_fields = 5;

	struct {
		char	sr_co_no[3];
		char	sr_est_no[3];
		char	sr_dp_no[3];
		char	sr_code[11];
		char	sr_name[26];
	} sjsr_rec;

	long	fr_date = 0L;
	long	to_date = 0L;

	FILE	*fout;  

	double	job_total[9],
			rep_total[9];

	int		lpno = 1;

    float   percent(double num, double den);

	char	s_fr_date[11],
			s_to_date[11],
			prog_desc[51];

#define	E_TIME		job_total[0]
#define	E_TIME_HALF	job_total[1]
#define	E_TIME_DOUBLE	job_total[2]
#define	E_DM		job_total[3]
#define	E_HM		job_total[4]
#define	E_MM		job_total[5]
#define	E_KK		job_total[6]
#define	E_KM		job_total[7]
#define	E_PROD		job_total[8]

#define	R_TIME		rep_total[0]
#define	R_TIME_HALF	rep_total[1]
#define	R_TIME_DOUBLE	rep_total[2]
#define	R_DM		rep_total[3]
#define	R_HM		rep_total[4]
#define	R_MM		rep_total[5]
#define	R_KK		rep_total[6]
#define	R_KM		rep_total[7]
#define	R_PROD		rep_total[8]

/*=====================
| Function Prototypes |
======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void process_data (void);
void print_tot (char *tot_desc, double *tot_val);
void read_sjsr (void);

/*=========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char *argv[])
{
	if (argc != 5)
	{
		print_at (0,0, mlSjMess705,argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf (prog_desc,"%-50.50s",argv[1]);
	lpno = atoi (argv[2]);
	fr_date = StringToDate (argv[3]);

	if (fr_date < 0L)
	{
		print_at (0,0,ML (mlSjMess045), argv[3]);
	    return (EXIT_FAILURE);
	}

	to_date = StringToDate (argv[4]);

	if (to_date < 0L)
	{
		print_at (0,0,ML (mlSjMess046),argv[4]);
	    return (EXIT_FAILURE);
	}
	sprintf (s_fr_date,"%-10.10s",argv[3]);
	sprintf (s_to_date,"%-10.10s",argv[4]);

	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	init_scr ();

	dsp_screen ("Processing : Employee Analysis Report", comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	head_output ();

	process_data ();

	fprintf (fout,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (fout);

	shutdown_prog();
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
	fprintf (fout, ".ECompany    : %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf (fout, ".EBranch     : %s - %s\n",comm_rec.test_no,clip(comm_rec.test_name));
	fprintf (fout, ".EDepartment : %s - %s\n",comm_rec.tdp_no,clip(comm_rec.tdp_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %-24.24s\n",SystemTime());
	fprintf (fout, ".E FROM  : %s  TO  : %s\n",s_fr_date,s_to_date);

	fprintf (fout, ".R==========");
	fprintf (fout, "==============================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "==========");
	fprintf (fout, "==============================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "| JOB NO  ");
	fprintf (fout, "|  E M P L O Y E E   N A M E  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "| ORD TIME ");
	fprintf (fout, "| TIME&1/2 ");
	fprintf (fout, "| DBL TIME ");
	fprintf (fout, "|   DIRT   ");
	fprintf (fout, "|  HEIGHT  ");
	fprintf (fout, "|   MEAL   ");
	fprintf (fout, "| KERRICK  ");
	fprintf (fout, "| KMETERS  |\n");

	fprintf (fout, "|---------");
	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
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
	abc_dbopen("data");
	open_rec ("sjld", sjld_list, sjld_no_fields, "sjld_id_no");
	open_rec ("sjsr", sjsr_list, sjsr_no_fields, "sjsr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjld");
	abc_fclose ("sjsr");
	abc_dbclose ("data");
}

void
process_data (
 void)
{
	int		cnt = 0;
	int		i = 0;
	long	old_order_no = 0L;
	char	ord_no[9];

	for (i = 0;i < 9;i++)
		job_total[i] = 0.00;

	for (i = 0;i < 9;i++)
		rep_total[i] = 0.00;

	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.test_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = 0L;
	sjld_rec.ld_date = fr_date;

	cc = find_rec ("sjld",&sjld_rec,GTEQ,"r");

	old_order_no = sjld_rec.ld_order_no;

	while (!cc && !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) && 	
				  !strcmp (sjld_rec.ld_est_no,comm_rec.test_no) && 	
				  !strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no))
	{
		if (sjld_rec.ld_date >= fr_date && sjld_rec.ld_date <= to_date)
		{
			read_sjsr ();
			if (old_order_no != sjld_rec.ld_order_no)
			{
				if (cnt)
					print_tot ("        JOB TOTALS         ",job_total);
				old_order_no = sjld_rec.ld_order_no;
				for (i = 0;i < 9;i++)
					job_total[i] = 0.00;
				cnt = 0;
			}
			sprintf (ord_no,"%8ld",sjld_rec.ld_order_no);
			dsp_process ("Job No : ",ord_no);

			cnt++;

			fprintf (fout, "|%-8ld ",sjld_rec.ld_order_no);
			fprintf (fout, "| %-28.28s",sjsr_rec.sr_name);
			fprintf (fout, "|%-10.10s",DateToString(sjld_rec.ld_date));
			fprintf (fout, "|%9.2f ",sjld_rec.ld_time);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_time_half);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_time_double);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_dm);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_hm);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_mm);
			fprintf (fout, "|%9.2f ",sjld_rec.ld_kk);
			fprintf (fout, "|%9.2f |\n",sjld_rec.ld_km);

			if (9999L < sjld_rec.ld_order_no && sjld_rec.ld_order_no <= 99999L)
			{
				E_PROD += (double) sjld_rec.ld_time;
				E_PROD += (double) sjld_rec.ld_time_half;
				E_PROD += (double) sjld_rec.ld_time_double;
				R_PROD += (double) sjld_rec.ld_time;
				R_PROD += (double) sjld_rec.ld_time_half;
				R_PROD += (double) sjld_rec.ld_time_double;
			}

			E_TIME += (double) sjld_rec.ld_time;
			E_TIME_HALF += (double) sjld_rec.ld_time_half;
			E_TIME_DOUBLE += (double) sjld_rec.ld_time_double;
			E_DM += (double) sjld_rec.ld_dm;
			E_HM += (double) sjld_rec.ld_hm;
			E_MM += (double) sjld_rec.ld_mm;
			E_KK += (double) sjld_rec.ld_kk;
			E_KM += (double) sjld_rec.ld_km;

			R_TIME += (double) sjld_rec.ld_time;
			R_TIME_HALF += (double) sjld_rec.ld_time_half;
			R_TIME_DOUBLE += (double) sjld_rec.ld_time_double;
			R_DM += (double) sjld_rec.ld_dm;
			R_HM += (double) sjld_rec.ld_hm;
			R_MM += (double) sjld_rec.ld_mm;
			R_KK += (double) sjld_rec.ld_kk;
			R_KM += (double) sjld_rec.ld_km;

			fflush (fout);
		}
		cc = find_rec ("sjld",&sjld_rec,NEXT,"r");
	}
	print_tot ("        JOB TOTALS         ",job_total);
	print_tot ("       GRAND TOTALS        ",rep_total);
}

void
print_tot (
 char *tot_desc, 
 double *tot_val)
{
	int	i;
	double	total;

	fprintf (fout, ".LRP6\n");

	fprintf (fout, "| %-27.27s",tot_desc);
	fprintf (fout, " %9.9s "," ");
	fprintf (fout, " %9.9s "," ");

	for (i = 0;i < 8;i++)
		fprintf (fout, (i == 7) ? "|%9.0f " : "|%9.2f ",tot_val[i]);
	fprintf (fout, "|\n");

	fprintf (fout, "|---------");
	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");

	fprintf (fout, "|%50.50s","TOTAL HOURS RECOVERED  ");
	fprintf (fout, "%10.2f",tot_val[8]);
	fprintf (fout, "%78.78s|\n"," ");

	total = tot_val[0] + tot_val[1] + tot_val[2];

	fprintf (fout, "|%50.50s","TOTAL HOURS WORKED     ");
	fprintf (fout, "%10.2f",total);
	fprintf (fout, " %23.23s","PERCENTAGE RECOVERED :");
	fprintf (fout, "%9.2f%%",percent(tot_val[8],total));
	fprintf (fout, "%45.45s|\n"," ");

	fprintf (fout, "|---------");
	fprintf (fout, "|-----------------------------");
	fprintf (fout, "|----------");
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
	if (den == 0.00)
		return(0.00);

	return ((float) ((num / den) * 100.00));
}

void
read_sjsr (
 void)
{
	strcpy (sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy (sjsr_rec.sr_est_no,comm_rec.test_no);
	strcpy (sjsr_rec.sr_dp_no,sjld_rec.ld_dp_no);
	strcpy (sjsr_rec.sr_code,sjld_rec.ld_emp_code);
	cc = find_rec ("sjsr",&sjsr_rec,COMPARISON,"r");
	if (cc)
		sprintf (sjsr_rec.sr_name,"%-25.25s","Unknown Employee");
}

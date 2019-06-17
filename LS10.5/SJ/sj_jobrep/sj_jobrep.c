/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_jobrep.c    )                                 |
|  Program Desc  : ( Detailed Labour Analysis By Job Report.      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjld, sjsr, sjhr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 04/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (04/04/87)      | Modified  by  : Lance Whitford.  |
|  Date Modified : (06/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (27/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (26/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (02/08/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      :  based on la_jobrep.c                              |
|                :  Tidy up program.                                  |
|                :                                                    |
|    (29/11/89)  :  Change moneytype fields to floattype.             |
|    (26/09/97)  :  Incorporated multilingual conversion.             |
|                :                                                    |
|                                                                     |
| $Log: sj_jobrep.c,v $
| Revision 5.2  2001/08/09 09:17:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:35  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:55  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:34:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/16 05:58:33  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.10  1999/10/25 06:17:16  scott
| Updated for pjulmdy
|
| Revision 1.9  1999/10/17 20:34:58  nz
| Updated for pjulmdy and pmdyjul conversion
|
| Revision 1.8  1999/09/29 10:13:00  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/24 05:06:36  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:31  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_jobrep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_jobrep/sj_jobrep.c,v 5.2 2001/08/09 09:17:34 scott Exp $";

#define	NO_SCRGEN
#include <ml_sj_mess.h>	
#include <ml_std_mess.h>	
#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>

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
		{"comm_dbt_date"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
		long	tdbt_date;
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
		{"sjld_km_rate"},
		{"sjld_dm"},
		{"sjld_hm"},
		{"sjld_mm"},
		{"sjld_kk"},
		{"sjld_time"},
		{"sjld_time_half"},
		{"sjld_time_double"},
		{"sjld_tm_rate"},
		{"sjld_oh_rate"},
		{"sjld_pr_rate"}
	};

	int sjld_no_fields = 18;

	struct {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
		float	ld_km;
		double	ld_km_rate;
		float	ld_dm;
		float	ld_hm;
		float	ld_mm;
		float	ld_kk;
		float	ld_time;
		float	ld_time_half;
		float	ld_time_double;
		double	ld_tm_rate;
		double	ld_oh_rate;
		double	ld_pr_rate;
	} sjld_rec;

	/*=====================
	| Serviceperson file  |
	=====================*/
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

	/*=================
	| Job header file |
	=================*/
	struct dbview sjhr_list[] ={
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_comp_date"}
	};

	int sjhr_no_fields = 5;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_comp_date;
	} sjhr_rec;

	int		lpno = 1;

    FILE	*fout;  

	double	job_total[12],
			rep_total[12];

	char	today[11],
			prog_desc[51];

#define	J_TIME		job_total[0]
#define	J_TIME_HALF	job_total[1]
#define	J_TIME_DOUBLE	job_total[2]
#define	J_KM		job_total[3]
#define	J_DM		job_total[4]
#define	J_HM		job_total[5]
#define	J_MM		job_total[6]
#define	J_KK		job_total[7]
#define	J_KM_VAL	job_total[8]
#define	J_LABOUR	job_total[9]
#define	J_OHEAD		job_total[10]
#define	J_PROFIT	job_total[11]

#define	R_TIME		rep_total[0]
#define	R_TIME_HALF	rep_total[1]
#define	R_TIME_DOUBLE	rep_total[2]
#define	R_KM		rep_total[3]
#define	R_DM		rep_total[4]
#define	R_HM		rep_total[5]
#define	R_MM		rep_total[6]
#define	R_KK		rep_total[7]
#define	R_KM_VAL	rep_total[8]
#define	R_LABOUR	rep_total[9]
#define	R_OHEAD		rep_total[10]
#define	R_PROFIT	rep_total[11]

/*=====================
| Function Prototypes |
======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void process_data (void);
void print_tot (char *tot_desc, double *tot_val);
int read_sjsr (void);
int read_sjhr (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv[])
{
	if (argc != 3)
	{
		print_at (0,0,mlSjMess712,argv[0]);
		return (EXIT_FAILURE);
	}
	
	sprintf (prog_desc,"%-50.50s",argv[1]);
	lpno = atoi (argv[2]);

	/*======================
	| Open database files. |
	======================*/
	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	strcpy (today, DateToString (TodaysDate()));

	init_scr ();

	dsp_screen ("Processing : Detailed Job Analysis Report", comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen ("pformat","w")) == NULL)
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
/*==============
| End of main .|
==============*/

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",lpno);
	fprintf (fout, ".13\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n",clip(prog_desc));
	fprintf (fout, ".ECompany    : %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf (fout, ".EBranch     : %s - %s\n",comm_rec.test_no,clip(comm_rec.test_name));
	fprintf (fout, ".EDepartment : %s - %s\n",comm_rec.tdp_no,clip(comm_rec.tdp_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %-24.24s\n",SystemTime());

	fprintf (fout, ".R==========");
	fprintf (fout, "=============================");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=======");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "===========\n");

	fprintf (fout, "==========");
	fprintf (fout, "=============================");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=======");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "===========\n");

	fprintf (fout, "| JOB NO  ");
	fprintf (fout, "| E M P L O Y E E   N A M E  ");
	fprintf (fout, "|  DATE  ");
	fprintf (fout, "|ORD TIME");
	fprintf (fout, "|TIME&1/2");
	fprintf (fout, "|DBL TIME");
	fprintf (fout, "|  KM  ");
	fprintf (fout, "|  DIRT  ");
	fprintf (fout, "| HEIGHT ");
	fprintf (fout, "|  MEAL  ");
	fprintf (fout, "| KERRICK");
	fprintf (fout, "| KMETERS");
	fprintf (fout, "| LABOUR  ");
	fprintf (fout, "|OVERHEADS");
	fprintf (fout, "| PROFIT  |\n");

	fprintf (fout, "|---------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------|\n");
	
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
	open_rec ("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
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
	abc_fclose ("sjhr");
	abc_fclose ("sjsr");
	abc_dbclose ("data");
}

void
process_data (
 void)
{
	int		dmy[3];
	long	TmpYear	=	0L;
	int		i = 0;
	int		cnt = 0;
	char	order[9];
	long	old_job_no = 0L;
	double	km_value = 0.00;
	double	tm_value = 0.00;
	double	tm_half_value = 0.00;
	double	tm_double_value = 0.00;
	double	labour = 0.00;
	double	overhead = 0.00;
	double	profit = 0.00;

	for (i = 0;i < 12;i++)
		job_total[i] = 0.00;

	for (i = 0; i < 12; i++)
		rep_total[i] = 0.00;

	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.test_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = 0L;
	sjld_rec.ld_date = 0L;

	cc = find_rec ("sjld",&sjld_rec,GTEQ,"r");

	old_job_no = sjld_rec.ld_order_no;

	while (!cc && !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) 
		   && !strcmp (sjld_rec.ld_est_no,comm_rec.test_no) 
		   && !strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no))
	{
		cc = read_sjsr ();
		
		DateToDMY (StringToDate (today), &dmy[0], &dmy[1],&dmy[2]);
		TmpYear	=	dmy[2];

		DateToDMY (sjhr_rec.hr_comp_date, &dmy[0], &dmy[1],&dmy[2]);

		if ((!cc && (dmy[1] == 0 && dmy[2] == 0)) || dmy[2] == TmpYear)
		{
			if (old_job_no != sjld_rec.ld_order_no)
			{
				if (cnt != 0)
					print_tot ("         JOB TOTALS         ",job_total);
				old_job_no = sjld_rec.ld_order_no;
				cc = read_sjhr ();
				for (i = 0;i < 12;i++)
					job_total[i] = 0.00;
				cnt = 0;
			}

			sprintf (order,"%8ld",sjld_rec.ld_order_no);
			dsp_process ("Job No :",order);

			km_value = (double) sjld_rec.ld_km;
			km_value *= sjld_rec.ld_km_rate;

			tm_value = (double) sjld_rec.ld_time;
			tm_value *= sjld_rec.ld_tm_rate;

			tm_half_value = (double) sjld_rec.ld_time_half;
			tm_half_value *= sjld_rec.ld_tm_rate;
			tm_half_value *= 1.50;

			tm_double_value = (double) sjld_rec.ld_time_double;
			tm_double_value *= sjld_rec.ld_tm_rate;
			tm_double_value *= 2.00;

			labour = tm_value + tm_half_value + tm_double_value;

			overhead = (double) sjld_rec.ld_time + sjld_rec.ld_time_half + sjld_rec.ld_time_double;
			profit = overhead;
			overhead *= sjld_rec.ld_oh_rate;
			profit *= sjld_rec.ld_pr_rate;
			
			cnt++;

			fprintf (fout, "|%-8ld ",sjld_rec.ld_order_no);
			fprintf (fout, "|%-28.28s",sjsr_rec.sr_name);
			fprintf (fout, "|%8.8s",DateToString(sjld_rec.ld_date));
			fprintf (fout, "|%8.2f",sjld_rec.ld_time);
			fprintf (fout, "|%8.2f",sjld_rec.ld_time_half);
			fprintf (fout, "|%8.2f",sjld_rec.ld_time_double);
			fprintf (fout, "|%6.1f",sjld_rec.ld_km);
			fprintf (fout, "|%8.2f",sjld_rec.ld_dm);
			fprintf (fout, "|%8.2f",sjld_rec.ld_hm);
			fprintf (fout, "|%8.2f",sjld_rec.ld_mm);
			fprintf (fout, "|%8.2f",sjld_rec.ld_kk);
			fprintf (fout, "|%8.2f",km_value);
			fprintf (fout, "|%9.2f",labour);
			fprintf (fout, "|%9.2f",overhead);
			fprintf (fout, "|%9.2f|\n",profit);
			fflush  (fout);

			J_TIME += (double) sjld_rec.ld_time;
			J_TIME_HALF += (double) sjld_rec.ld_time_half;
			J_TIME_DOUBLE += (double) sjld_rec.ld_time_double;
			J_KM += (double) sjld_rec.ld_km;
			J_DM += (double) sjld_rec.ld_dm;
			J_HM += (double) sjld_rec.ld_hm;
			J_MM += (double) sjld_rec.ld_mm;
			J_KK += (double) sjld_rec.ld_kk;
			J_KM_VAL += km_value;
			J_LABOUR += labour;
			J_OHEAD += overhead;
			J_PROFIT += profit;

			R_TIME += (double) sjld_rec.ld_time;
			R_TIME_HALF += (double) sjld_rec.ld_time_half;
			R_TIME_DOUBLE += (double) sjld_rec.ld_time_double;
			R_KM += (double) sjld_rec.ld_km;
			R_DM += (double) sjld_rec.ld_dm;
			R_HM += (double) sjld_rec.ld_hm;
			R_MM += (double) sjld_rec.ld_mm;
			R_KK += (double) sjld_rec.ld_kk;
			R_KM_VAL += km_value;
			R_LABOUR += labour;
			R_OHEAD += overhead;
			R_PROFIT += profit;
		}
		cc = find_rec ("sjld",&sjld_rec,NEXT,"r");
	}
	print_tot ("         JOB TOTALS         ",job_total);
	print_tot ("        GRAND TOTALS        ",rep_total);
}

void
print_tot (
 char *tot_desc,
 double *tot_val)
{
	int	i;

	fprintf (fout, "|         ");
	fprintf (fout, "                             ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "       ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "         ");
	fprintf (fout, "          ");
	fprintf (fout, "          ");
	fprintf (fout, "          |\n");

	fprintf (fout, "|%9.9s"," ");
	fprintf (fout, " %-28.28s",tot_desc);
	fprintf (fout, " %8.8s "," ");

	for (i = 0;i < 8;i++)
		fprintf (fout, (i == 3) ? "%6.0f|" : "%8.2f|",tot_val[i]);
	fprintf (fout, "%8.2f|",tot_val[8]);
	for (i = 9;i < 12;i++)
		fprintf (fout,"%9.2f|",tot_val[i]);
	fprintf (fout, "\n");

	fprintf (fout, "|---------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------|\n");
	
	fflush (fout);
}

int
read_sjsr (
 void)
{
	strcpy (sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy (sjsr_rec.sr_est_no,comm_rec.test_no);
	strcpy (sjsr_rec.sr_dp_no,sjld_rec.ld_dp_no);
	strcpy (sjsr_rec.sr_code,sjld_rec.ld_emp_code);
	cc = find_rec ("sjsr",&sjsr_rec,COMPARISON,"r");
	return (cc);
}

int
read_sjhr (
 void)
{
	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy (sjhr_rec.hr_dp_no,sjld_rec.ld_dp_no);
	sjhr_rec.hr_order_no = sjld_rec.ld_order_no;
	cc = find_rec ("sjhr",&sjhr_rec,COMPARISON,"r");
	if (cc)
		sjhr_rec.hr_comp_date = 0L;
	return (cc);
}

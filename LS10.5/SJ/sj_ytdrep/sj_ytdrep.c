/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_ytdrep.c    )                                 |
|  Program Desc  : ( Year to Date Summary  Report.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ladt, esmr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 07/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (18/11/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Program tidy up                                    |
|  (16/09/97)    : Incorporate multilingual conversion.               |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                :                                                    |
| $Log: sj_ytdrep.c,v $
| Revision 5.2  2001/08/09 09:17:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:09  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:34:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/16 05:58:38  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.10  1999/10/17 20:34:58  nz
| Updated for pjulmdy and pmdyjul conversion
|
| Revision 1.9  1999/09/29 10:13:11  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 05:13:10  scott
| Updated for Ansi project
|
| Revision 1.7  1999/09/24 05:06:44  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:39  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ytdrep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ytdrep/sj_ytdrep.c,v 5.2 2001/08/09 09:17:52 scott Exp $";

#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>	
#include <ml_sj_mess.h>	

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_short_name"},
	};

	int esmr_no_fields = 3;

	struct {
		char	es_co_no[3];
		char	es_est_no[3];
		char	es_name[16];
	} esmr_rec;


	/*=====================
	| Labour details file |
	=====================*/
	struct dbview sjld_list[] ={
		{"sjld_co_no"},
		{"sjld_est_no"},
		{"sjld_dp_no"},
		{"sjld_order_no"},
		{"sjld_emp_code"},
		{"sjld_date"},
		{"sjld_veh_code"},
		{"sjld_km"},
		{"sjld_km_rate"},
		{"sjld_dm"},
		{"sjld_dm_rate"},
		{"sjld_hm"},
		{"sjld_hm_rate"},
		{"sjld_mm"},
		{"sjld_mm_rate"},
		{"sjld_kk"},
		{"sjld_kk_rate"},
		{"sjld_time"},
		{"sjld_time_half"},
		{"sjld_time_double"},
		{"sjld_tm_rate"},
		{"sjld_oh_rate"},
		{"sjld_pr_rate"}
	};

	int sjld_no_fields = 23;

	struct {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
		char	ld_veh_code[4];
		float	ld_km;
		double	ld_km_rate;		/*  Money field  */
		float	ld_dm;
		double	ld_dm_rate;
		float	ld_hm;
		double	ld_hm_rate;
		float	ld_mm;
		double	ld_mm_rate;
		float	ld_kk;
		double	ld_kk_rate;
		float	ld_time;
		float	ld_time_half;
		float	ld_time_double;
		double	ld_tm_rate;		/*  Money field  */
		double	ld_oh_rate;
		double	ld_pr_rate;
	} sjld_rec;

	int	lpno = 1;

	long	wk_start;	/* date of monday of current week	*/
	long	wk_end;		/* date of sunday of current week	*/
	long	lsystemDate;	/* system date				*/

	FILE	*fout;

	struct	tm	*t;

typedef struct accum {
	int	in_use;
	float	pd_hrs;
	float	n_pr_hrs;
	float	km;
	double	km_val;
	float	ytd_pd_hrs;
	float	ytd_n_pr_hrs;
	float	ytd_km;
	double	ytd_km_val;
} accum;

accum	br_accum[100];	/* accumulator for branches ytd			*/
char	old_est_no[3];


/*=======================
| Function Declarations |
=======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void process_data (void);
int process_week (void);
int in_week (void);
void print_week (void);
void print_tot (int br_no, int first_br);
double percent (double num, double den);
void set_week (int first);
void init_ytd (int init_all);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{

	if (argc != 2)
	{
		print_at(0,0,mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}
	
	lpno = atoi(argv[1]);

	/*======================
	| Open database files. |
	======================*/
	OpenDB();

	/*===================================================
	| Read common terminal record & cost centre master. |
	===================================================*/
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	lsystemDate = TodaysDate ();
	
	init_scr();

	dsp_screen("Processing : Weekly Analysis Report", comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	  Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen("pformat","w")) == 0)
	{
		print_at(0,0,"Error in pformat During (POPEN)", cc, PNAME);
		shutdown_prog();
        return (EXIT_FAILURE);
	}

	set_week(TRUE);

	head_output();

	process_data();

    fprintf(fout,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose(fout);

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
	fprintf(fout, ".LP%d\n",lpno);
	fprintf(fout, ".10\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, ".L158\n");

	fprintf(fout, ".B1\n");
	fprintf(fout, ".ESUMMARY OF HOURS REPORT\n");
	fprintf(fout, ".E%s \n",clip(comm_rec.tco_name));
	fprintf(fout, ".E AS AT : %-24.24s\n",SystemTime());

	fprintf(fout, ".R===========");
	fprintf(fout, "==================");
	fprintf(fout, "=============");
	fprintf(fout, "============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "==============\n");

	fprintf(fout, "===========");
	fprintf(fout, "==================");
	fprintf(fout, "=============");
	fprintf(fout, "============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "=============");
	fprintf(fout, "==============\n");

	fprintf(fout, "|   WEEK   ");
	fprintf(fout, "|                 ");
	fprintf(fout, "|            ");
	fprintf(fout, "|           ");
	fprintf(fout, "|            ");
	fprintf(fout, "|       Y E A R   T O   D A T E       ");
	fprintf(fout, "|            ");
	fprintf(fout, "|            ");
	fprintf(fout, "| Y E A R   T O   D A T E |\n");

	fprintf(fout, "|  ENDING  ");
	fprintf(fout, "| BRANCH          ");
	fprintf(fout, "| HOURS PAID ");
	fprintf(fout, "|  NP HOURS ");
	fprintf(fout, "| NP PERCENT ");
	fprintf(fout, "| HOURS PAID ");
	fprintf(fout, "|  NP HOURS ");
	fprintf(fout, "| NP PERCENT ");
	fprintf(fout, "| KILOMETERS ");
	fprintf(fout, "| KM VALUE $ ");
	fprintf(fout, "| KILOMETERS ");
	fprintf(fout, "| KM VALUE $ |\n");

	fflush(fout);
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

	open_rec("sjld", sjld_list, sjld_no_fields, "sjld_id_no_3");
	open_rec("esmr", esmr_list, esmr_no_fields, "esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjld");
	abc_fclose("esmr");
	abc_dbclose("data");
}

void
process_data (
 void)
{
	init_ytd(TRUE);

	strcpy(sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy(sjld_rec.ld_est_no,comm_rec.test_no);
	sjld_rec.ld_date = 0L;

	cc = find_rec("sjld",&sjld_rec,GTEQ,"r");

	/*---------------------------------------
	| Print only for current company	|
	---------------------------------------*/
	while (!cc && !strcmp(sjld_rec.ld_co_no,comm_rec.tco_no) && wk_start <= lsystemDate)
	{
		strcpy(old_est_no,sjld_rec.ld_est_no);
		strcpy (err_str, DateToString(wk_end));
		dsp_process("Week Ending.",err_str);
		/*---------------------------------------
		| Accumulate data for current week	|
		---------------------------------------*/

		switch (in_week())
		{
		case -1:
			cc = find_rec("sjld",&sjld_rec,NEXT,"r");
			break;

		case 0:
			while (!cc && !in_week())
				cc = process_week();

		case 1:
			print_week();
			init_ytd(FALSE);
			set_week(FALSE);
			break;
		}
	}
}

int
process_week (
 void)
{
	int	br_no;
	float	tot_hrs;
	double	km_value;

	/*-------------------------------
	| Get numeric value for est_no	|
	-------------------------------*/
	br_no = atoi(sjld_rec.ld_est_no);
	if (br_no == 0)
	{
		cc = find_rec("sjld",&sjld_rec,NEXT,"r");
		return(cc);
	}

	/*---------------------------------------
	| Accumulate data for current branch	|
	---------------------------------------*/
	while (!cc && !in_week() && !strcmp(sjld_rec.ld_est_no,old_est_no))
	{
		tot_hrs = sjld_rec.ld_time + sjld_rec.ld_time_half + sjld_rec.ld_time_double;
		br_accum[br_no].in_use = TRUE;
		br_accum[br_no].pd_hrs += tot_hrs;

		km_value = (double) sjld_rec.ld_km;
		km_value *= sjld_rec.ld_km_rate;
		km_value = twodec(km_value);
		/*-----------------------
		| Are hrs productive ?	|
		-----------------------*/
		if (!(9999 < sjld_rec.ld_order_no && sjld_rec.ld_order_no <= 99999))
			br_accum[br_no].n_pr_hrs += tot_hrs;

		br_accum[br_no].km += sjld_rec.ld_km;
		br_accum[br_no].km_val += km_value;

		cc = find_rec("sjld",&sjld_rec,NEXT,"r");
	}
	/*-------------------------------
	| Accumulate totals for branch	|
	-------------------------------*/
	br_accum[br_no].ytd_pd_hrs += br_accum[br_no].pd_hrs;
	br_accum[br_no].ytd_n_pr_hrs += br_accum[br_no].n_pr_hrs;
	br_accum[br_no].ytd_km += br_accum[br_no].km;
	br_accum[br_no].ytd_km_val += br_accum[br_no].km_val;
	/*-------------------------------
	| Accumulate grand totals	|
	-------------------------------*/
	br_accum[0].pd_hrs	+= br_accum[br_no].pd_hrs;
	br_accum[0].n_pr_hrs	+= br_accum[br_no].n_pr_hrs;
	br_accum[0].km		+= br_accum[br_no].km;
	br_accum[0].km_val	+= br_accum[br_no].km_val;
	br_accum[0].ytd_pd_hrs	+= br_accum[br_no].pd_hrs;
	br_accum[0].ytd_n_pr_hrs += br_accum[br_no].n_pr_hrs;
	br_accum[0].ytd_km	+= br_accum[br_no].km;
	br_accum[0].ytd_km_val	+= br_accum[br_no].km_val;
	strcpy(old_est_no,sjld_rec.ld_est_no);
	return(cc);
}

int
in_week (
 void)
{
	if (wk_start > sjld_rec.ld_date)
		return(-1);
	if (wk_start <= sjld_rec.ld_date && sjld_rec.ld_date < wk_end)
		return(0);
	if (sjld_rec.ld_date >= wk_end)
		return(1);

    /* added default return value */
    return (EXIT_SUCCESS);
}

void
print_week (
 void)
{
	int	br_no;
	int	first_br;
	
	for (first_br = TRUE,br_no = 1;br_no < 100;br_no++)
	{
		if (br_accum[br_no].in_use)
		{
			print_tot(br_no,first_br);
			first_br = FALSE;
		}
	}
	print_tot(0,first_br);
}

void
print_tot (
 int br_no, 
 int first_br)
{
	if (br_no != 0)
	{
		strcpy(esmr_rec.es_co_no,comm_rec.tco_no);
		sprintf(esmr_rec.es_est_no,"%2d",br_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
		if (cc)
			sprintf(esmr_rec.es_name,"Branch - [%2d]",br_no);
	}
	else
		strcpy(esmr_rec.es_name,"TOTAL");
	
	if (first_br)
	{
		fprintf(fout, ".LRP4\n");
		fprintf(fout, "|----------");
		fprintf(fout, "|-----------------");
		fprintf(fout, "|------------");
		fprintf(fout, "|-----------");
		fprintf(fout, "|------------");
		fprintf(fout, "|------------");
		fprintf(fout, "|-----------");
		fprintf(fout, "|------------");
		fprintf(fout, "|------------");
		fprintf(fout, "|------------");
		fprintf(fout, "|------------");
		fprintf(fout, "|------------|\n");
	}
		
	fprintf(fout, "|%10.10s",(first_br) ? DateToString(wk_end) : " ");
	fprintf(fout, "| %-15.15s ",esmr_rec.es_name);
	fprintf(fout, "| %10.2f ",br_accum[br_no].pd_hrs);
	fprintf(fout, "| %9.2f ",br_accum[br_no].n_pr_hrs);
	fprintf(fout, "| %9.2f%% ",percent(br_accum[br_no].n_pr_hrs,br_accum[br_no].pd_hrs));
	fprintf(fout, "| %10.2f ",br_accum[br_no].ytd_pd_hrs);
	fprintf(fout, "| %9.2f ",br_accum[br_no].ytd_n_pr_hrs);
	fprintf(fout, "| %9.2f%% ",percent(br_accum[br_no].ytd_n_pr_hrs,br_accum[br_no].ytd_pd_hrs));
	fprintf(fout, "| %10f ",br_accum[br_no].km);
	sprintf(err_str,"$%0.2f",br_accum[br_no].km_val);
	fprintf(fout, "| %10.10s ",err_str);
	fprintf(fout, "| %10f ",br_accum[br_no].ytd_km);
	sprintf(err_str,"$%0.2f",br_accum[br_no].ytd_km_val);
	fprintf(fout, "| %10.10s |\n",err_str);

	fflush(fout);
}

/*=======================
| calculate percentage	|
=======================*/
double 
percent (
 double num, 
 double den)
{
	if (den == 0.00)
		return(0.00);

	return((100.00 - ((den - num) / den) * 100.00));
}

void
set_week (
 int first)
{
	int	offset;
	long	yr_start;
	long	seconds;

	int	mon;
	int	yr;
	int	dmy [3];

	if (first)
	{
		DateToDMY (comm_rec.tdbt_date, &dmy[0],&dmy[1],&dmy[2]);

		mon = dmy [1];
		yr = dmy [2];
		if (mon < comm_rec.tfiscal)
			yr--;

		dmy [1] = comm_rec.tfiscal + 1;
		dmy [2] = yr;

		yr_start = DMYToDate (dmy[0], dmy[1], dmy[2]);

		seconds = (yr_start - 25567L) * 86400L;
		t = localtime(&seconds);
		wk_start = yr_start;
		wk_end = wk_start;
		offset = 7 - t->tm_wday;
		wk_end += (long) offset;
	}
	else
	{
		wk_start = wk_end;
		wk_end += 7L;
	}
}

void
init_ytd (
 int init_all)
{
	int	i;

	for (i = 0; i < 100;i++)
	{
		br_accum[i].in_use = FALSE;
		br_accum[i].pd_hrs = 0.00;
		br_accum[i].n_pr_hrs = 0.00;
		br_accum[i].km = 0.00;
		br_accum[i].km_val = 0.00;

		if (init_all)
		{
			br_accum[i].ytd_pd_hrs = 0.00;
			br_accum[i].ytd_n_pr_hrs = 0.00;
			br_accum[i].ytd_km = 0.00;
			br_accum[i].ytd_km_val = 0.00;
		}
	}
}

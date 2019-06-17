/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_allprep.c   )                                 |
|  Program Desc  : ( Sales Analysis Report showing profit.        )   |
|                  ( Prints Several different Reports.            )   |
|                  ( (1) Sales Analysis by Salesman.              )   |
|                  ( (2) Sales Analysis by Category.              )   |
|                  ( (3) Sales Analysis by Customer by Category.  )   |
|                  ( (4) Sales Analysis by Area Code.             )   |
|                  ( (5) Sales Analysis by Customer Type.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, exaf, excf, excl, sale, exsf, cudp    |
|  Database      : (data) sabg,                                       |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 19/05/87         |
|---------------------------------------------------------------------|
|  Date Modified : (19/05/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (14/03/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (30/07/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (06/10/92)      | Modified  by  : Anneliese Allen  |
|  Date Modified : (27/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (10/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : (14/03/91)  Freight now included conditional upon  |
|                : the environment variable SA_FREIGHT being set to 1.|
|                :                                                    |
|  (30/07/91)    : Added time into heading.                           |
|                :                                                    |
|  (06/10/92)    : Removed Last Yr column from output (reported by HEL|
|                : that it was redundant HEL 7532).                   |
|                :                                                    |
|  (27/05/96)    : Updated to fix problems in DateToString. 				  |
|  (10/09/97)    : Modified for Multilingual Conversion and 	      |
|                                                                     |
| $Log: sa_allprep.c,v $
| Revision 5.2  2001/08/09 09:16:36  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:05:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:07  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:23  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:45  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:11  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/02/18 02:35:15  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.13  1999/12/06 01:35:17  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 04:55:29  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.11  1999/10/20 02:07:02  nz
| Updated for final changes on date routines.
|
| Revision 1.10  1999/10/16 01:11:20  nz
| Updated for pjulmdy routines
|
| Revision 1.9  1999/09/29 10:12:39  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:22  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 02:01:50  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:18  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_allprep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_allprep/sa_allprep.c,v 5.2 2001/08/09 09:16:36 scott Exp $";

#define  NO_SCRGEN
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sa_mess.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 12;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_name[41];
		char	tcc_short[10];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_stat_flag"}
	};

	int cumr_no_fields = 9;

	struct {
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_department[3];
		char	mr_dbt_no[7];
		char	mr_dbt_name[41];
		char	mr_dbt_acronym[10];
		char	mr_area_code[3];
		char	mr_sman_code[3];
		char	mr_stat_flag[2];
	} cumr_rec;

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
		char	af_co_no[3];
		char	af_area_code[3];
		char	af_area[41];
		char	af_stat_flag[2];
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
		char	cf_co_no[3];
		char	cf_categ_no[12];
		char	cf_categ_description[41];
		char	cf_stat_flag[2];
	} excf_rec;

	/*=================================
	| Customer Class Type Master File. |
	=================================*/
	struct dbview excl_list[] ={
		{"excl_co_no"},
		{"excl_class_type"},
		{"excl_class_desc"},
		{"excl_stat_flag"}
	};

	int excl_no_fields = 4;

	struct {
		char	cl_co_no[3];
		char	cl_class_type[4];
		char	cl_class_description[41];
		char	cl_stat_flag[2];
	} excl_rec;

	/*===============================================
	| Sales Analysis By Customer/Category/Salesman. |
	===============================================*/
	struct dbview sale_list[] ={
		{"sale_key"},
		{"sale_category"},
		{"sale_sman"},
		{"sale_area"},
		{"sale_ctype"},
		{"sale_dbt_no"},
		{"sale_year_flag"},
		{"sale_period"},
		{"sale_units"},
		{"sale_gross"},
		{"sale_cost_sale"},
		{"sale_disc"}
	};

	int sale_no_fields = 12;

	struct {
		char	le_key[9];
		char	le_category[12];
		char	le_sman[3];
		char	le_area[3];
		char	le_ctype[4];
		char	le_dbt_no[7];
		char	le_year[2];
		char	le_period[3];
		double	le_units;
		double	le_gross;		/*  Money field  */
		double	le_cost_sale;		/*  Money field  */
		double	le_disc;		/*  Money field  */
	} sale_rec;

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
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
		char	sf_stat_flag[2];
	} exsf_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list[] ={
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
		{"cudp_stat_flag"}
	};

	int cudp_no_fields = 5;

	struct {
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_dp_name[41];
		char	dp_stat_flag[2];
	} cudp_rec;

	/*======================
	| Sales / Budget file. |
	======================*/
	struct dbview sabg_list[] ={
		{"sabg_co_no"},
		{"sabg_area_code"},
		{"sabg_sman_code"},
		{"sabg_ctype_code"},
		{"sabg_category"},
		{"sabg_year_flag"},
		{"sabg_bg1_sales"},
		{"sabg_bg2_sales"},
		{"sabg_bg3_sales"},
		{"sabg_bg4_sales"},
		{"sabg_bg5_sales"},
		{"sabg_bg6_sales"},
		{"sabg_bg7_sales"},
		{"sabg_bg8_sales"},
		{"sabg_bg9_sales"},
		{"sabg_bg10_sales"},
		{"sabg_bg11_sales"},
		{"sabg_bg12_sales"},
		{"sabg_bg1_profit"},
		{"sabg_bg2_profit"},
		{"sabg_bg3_profit"},
		{"sabg_bg4_profit"},
		{"sabg_bg5_profit"},
		{"sabg_bg6_profit"},
		{"sabg_bg7_profit"},
		{"sabg_bg8_profit"},
		{"sabg_bg9_profit"},
		{"sabg_bg10_profit"},
		{"sabg_bg11_profit"},
		{"sabg_bg12_profit"}
	};

	int sabg_no_fields = 30;

	struct {
		char	bg_co_no[3];
		char	bg_area_code[3];
		char	bg_sman_code[3];
		char	bg_ctype_code[4];
		char	bg_category[12];
		char	bg_year_flag[2];
		double	bg_sales[12];
		double	bg_profit[12];
	} lsabg_rec, csabg_rec;

	struct {
		char	*heading;
		char	*report;
		char	*desc;
	} head_list[5] = {
		{"SALESMAN","SALESPERSON  ","UNITS         "},
		{"CATEGORY","CATEGORY   ","CAT NUMBER ..... UNITS"},
		{"CUSTOMER BY CATEGORY"," "," "},
		{"AREA CODE","AREA CODES  ","UNITS         "},
		{"CUSTOMER TYPE","CUSTOMER TYPES","CODES ..... UNITS   "},
	};

#define	SALES	0
#define	CATG	1
#define	C_CATG	2
#define	A_CODE	3
#define	C_TYPE	4

#define	BY_CO	0
#define	BY_BR	1
#define	BY_DP	2
#define	BY_WH	3

int	sa_detail;
int	sa_freight;
int	cnt;
int	first_rec = TRUE;
int	first_line = TRUE;
int	low_len;
int	lpno = 1;
int	rep_type;
int	lsabg_found = FALSE;
int	csabg_found = FALSE;
int	sabg_found = FALSE;
int	analysis;
int	cal_fiscal;

FILE	*fout;

char	t_date[26];
char	dp_wh[3];
char	lower[12];
char	upper[12];
char	mask[10];
char	low_value[12];
char	cur_month[3];
char	fiscal[3];

struct {
	char	code[12];
	char	description[41];
} local_rec;

double	lst_yr_sale[3];
double	sales_tot[8];
double	grp_total[8];
double	grand_tot[8];

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void process_data (void);
int check_c_catg (int last_group);
int check_sales (int last_group);
int check_catg (int last_group);
int check_a_code (int last_group);
int check_c_ctype (int last_group);
int delta_code (int file_id);
void prt_sale (int first_line);
int in_range (void);
void init_sale (void);
void init_cust (void);
void init_total (void);
void prt_total (char *tot_desc, char *unit_desc, double *total_line, double yr_total);
void sum_sale (void);
void print_sale (int first_line);
double percent (double l_profit, double l_sale);
int in_fiscal (char *year);
double sum_ytd (int sales, int to_mnth, char *year);
int valid_record (void);
int read_detail (int file_id);
int read_sale (int stype);

/*=====================================================================
| Main Processing Routine
=====================================================================*/
int
main (
 int    argc,
 char*  argv[])
{
	
	char	*sptr;
	int		currentMonth;

	/*---------------------------------------
	| check parameter count			|
	---------------------------------------*/
	if (argc != 8)
	{
		print_at (0,0,mlSaMess700,argv[0]);
        return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| check sales analysis detail flag	|
	---------------------------------------*/
	sa_detail = atoi(get_env("SA_EXP"));

	if (sa_detail < 0)
		sa_detail = 0;
	if (sa_detail > 10)
		sa_detail = 10;

	sa_freight = TRUE;
	if (chk_env("SA_FREIGHT"))
		sa_freight = atoi(get_env("SA_FREIGHT"));

	/*---------------------------------------
	| check report type			|
	---------------------------------------*/
	if (argv[4][0] < '0' || argv[4][0] > '4')
	{
		print_at(1,0,mlSaMess701);
		print_at(2,0,mlSaMess702);
		print_at(3,0,mlSaMess703);
		print_at(4,0,mlSaMess704);
		print_at(5,0,mlSaMess705);
        return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| check analysis			|
	---------------------------------------*/
	if (argv[5][0] < '0' || argv[5][0] > '3')
	{
		/*print_at(6,0,"<analysis> 0 - by company\007\n");
		print_at(7,0,"           1 - by branch\n");
		print_at(8,0,"           2 - by warehouse\n");
		print_at(9,0,"           3 - by department\n");*/
		print_at(6,0,ML(mlSaMess706));
		print_at(7,0,ML(mlSaMess707));
		print_at(8,0,ML(mlSaMess708));
		print_at(9,0,ML(mlSaMess709));
        return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| initialise variables from parameters	|
	---------------------------------------*/
	lpno = atoi(argv[1]);
	sprintf(lower,"%-.11s",clip(argv[2]));
	rep_type = atoi(argv[4]);
	analysis = atoi(argv[5]);
	sprintf(dp_wh,"%-2.2s",argv[6]);
	/*---------------------------------------
	| set upper & check length		|
	---------------------------------------*/
	switch (rep_type)
	{
	case	SALES:
	case	A_CODE:
		sprintf(upper,"%-2.2s",clip(argv[3]));
		low_len = 2;
		break;

	case	CATG:
		sprintf(upper,"%-11.11s",clip(argv[3]));
		low_len = strlen(lower);
		if (low_len == 0 || sa_detail)
			low_len = 11;
		break;

	case	C_CATG:
		sprintf(upper,"%-6.6s",clip(argv[3]));
		low_len = 6;
		break;

	case	C_TYPE:
		sprintf(upper,"%-3.3s",clip(argv[3]));
		low_len = 3;
		break;
	}
	sprintf(mask,"%%-%d.%ds",low_len,low_len);

	OpenDB();

	ReadMisc();

	init_scr();

	DateToDMY (comm_rec.tdbt_date, NULL, &currentMonth, NULL);
	sprintf (cur_month, "%02d", currentMonth);

	sptr = chk_env("SA_YEND");
	cal_fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( cal_fiscal < 1 || cal_fiscal > 12 )
		cal_fiscal = comm_rec.tfiscal;

	sprintf(fiscal,"%02d", cal_fiscal );

	sprintf(err_str,"Processing : SALES ANALYSIS BY %s",head_list[rep_type].heading);
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
	/*--------------------------------------
	| open output				|
 	---------------------------------------*/
	if ((fout = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	head_output();

	process_data();

	/*--------------------------------------- 
	| Program exit sequence.		|
	---------------------------------------*/
    fprintf(fout,".EOF\n");
	pclose(fout);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=====================================================================
| Start Out Put To Standard Print.
=====================================================================*/
void
head_output (void)
{
	int	head_lines = 0;
	/*---------------------------------------
	| set number of lines in heading	|
	---------------------------------------*/
	switch(analysis)
	{
	case	BY_CO:
		head_lines = 14;
		break;

	case	BY_BR:
		head_lines = 15;
		break;

	case	BY_DP:
	case	BY_WH:
		head_lines = 16;
		break;
	}
	fprintf(fout,".START%s<%s>\n",DateToString (TodaysDate ()), PNAME);
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".%d\n",head_lines);
	fprintf(fout,".PI12\n");
	fprintf(fout,".L158\n");

	fprintf(fout,".B1\n");
	fprintf(fout,".ESALES ANALYSIS BY %s\n",head_list[rep_type].heading);
	fprintf(fout,".E%s \n",clip(comm_rec.tco_name));
	
	if (analysis != BY_CO)
		fprintf(fout,".EBranch: %s \n",clip(comm_rec.test_name));

	if (analysis == BY_DP)
		fprintf(fout,".EDepartment: %s \n",clip(cudp_rec.dp_dp_name));

	if (analysis == BY_WH)
		fprintf(fout,".EWarehouse: %s \n",clip(comm_rec.tcc_name));

	fprintf(fout,".EFor Month %s\n",cur_month);
	fprintf(fout,".E AS AT : %-24.24s\n",SystemTime());

	fprintf(fout,".R=============================");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"===========\n");

	fprintf(fout,"=============================");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"===========\n");

	fprintf(fout,"|%28.28s"," ");
	fprintf(fout,"|%26.26sMONTH%26.26s"," "," ");
	fprintf(fout,"|%24.24sYEAR TO DATE%23.23s|\n"," "," ");

	fprintf(fout,"|       %14.14s       ",head_list[rep_type].report);
	fprintf(fout,"|---------");
	fprintf(fout,"----------");
	fprintf(fout,"---------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"---------");
	fprintf(fout,"|---------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"----------|\n");

	if (rep_type == C_CATG)
		fprintf(fout,"|----------------------------");
	else
		fprintf(fout,"|            AND             ");

	fprintf(fout,"|%12.12sSALES%11.11s"," "," ");
	fprintf(fout,"|%11.11sPROFIT%11.11s"," "," ");
	fprintf(fout,"|%12.12sSALES%12.12s"," "," ");
	fprintf(fout,"|%12.12sPROFIT%11.11s|\n"," "," ");

	if (rep_type == C_CATG)
		fprintf(fout,"| CUST |CUSTOMER | CATEGORY  ");
	else
		fprintf(fout,"|   %22.22s   ",head_list[rep_type].desc);

	fprintf(fout,"|---------");
	fprintf(fout,"----------");
	fprintf(fout,"---------");
	fprintf(fout,"|---------");
	fprintf(fout,"----------");
	fprintf(fout,"---------");
	fprintf(fout,"|---------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"----------");
	fprintf(fout,"|---------");
	fprintf(fout,"----------|\n");

	if (rep_type == C_CATG)
		fprintf(fout,"| CODE | ACRONYM |           ");
	else
		fprintf(fout,"|                            ");

	fprintf(fout,"|  ACTUAL ");
	fprintf(fout,"|  BUDGET ");
	fprintf(fout,"|  DIFF  ");
	fprintf(fout,"|  ACTUAL ");
	fprintf(fout,"|  BUDGET ");
	fprintf(fout,"|  DIFF  ");
	fprintf(fout,"|  ACTUAL ");
	fprintf(fout,"|  BUDGET ");
	fprintf(fout,"|   DIFF  ");
	fprintf(fout,"|  ACTUAL ");
	fprintf(fout,"|  BUDGET ");
	fprintf(fout,"|   DIFF  |\n");

	fflush(fout);
}

void
shutdown_prog (void)
{
	CloseDB ();
	FinishProgram ();
}

/*=====================================================================
| Open data base files.
=====================================================================*/
void
OpenDB (void)
{
	abc_dbopen("data");

	switch (rep_type)
	{
	case	SALES:
		open_rec("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
		open_rec("sale",sale_list,sale_no_fields,"sale_sman");
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no2");
		break;

	case	CATG:
		open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
		open_rec("sale",sale_list,sale_no_fields,"sale_category");
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no4");
		break;

	case	C_CATG:
		open_rec("cumr",cumr_list,cumr_no_fields,"cumr_id_no");
		open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
		open_rec("sale",sale_list,sale_no_fields,"sale_id_no_2");
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no4");
		break;

	case	A_CODE:
		open_rec("exaf",exaf_list,exaf_no_fields,"exaf_id_no");
		open_rec("sale",sale_list,sale_no_fields,"sale_area");
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no1");
		break;

	case	C_TYPE:
		open_rec("excl",excl_list,excl_no_fields,"excl_id_no");
		open_rec("sale",sale_list,sale_no_fields,"sale_ctype");
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no3");
		break;
	}
}

/*=====================================================================
| Close data base files.
=====================================================================*/
void
CloseDB (void)
{
	abc_fclose("cumr");
	abc_fclose("exaf");
	abc_fclose("excf");
	abc_fclose("excl");
	abc_fclose("sale");
	abc_fclose("sabg");
	abc_fclose("exsf");
	abc_dbclose("data");
}

/*=======================================
| Get info from commom database file	|
=======================================*/
void
ReadMisc (void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	if (analysis == BY_DP)
	{
		open_rec("cudp",cudp_list,cudp_no_fields,"cudp_id_no");
		strcpy(cudp_rec.dp_co_no,comm_rec.tco_no);
		strcpy(cudp_rec.dp_br_no,comm_rec.test_no);
		strcpy(cudp_rec.dp_dp_no,dp_wh);

		cc = find_rec("cudp",&cudp_rec,COMPARISON,"r");
		if (cc)
			sys_err("Error in cudp During (DBFIND)",cc,PNAME);
		abc_fclose("cudp");
	}
}

void
process_data (void)
{
	/*---------------------------------------
	| initialise totals			|
	---------------------------------------*/
	init_sale();
	init_cust();
	init_total();
	cnt = 0;
	/*---------------------------------------
	| Find First Valid Record		|
	---------------------------------------*/
	if (read_sale(GTEQ))
		return;
		
	cc = read_detail(rep_type);
	if (cc)
		return;

	if (rep_type == C_CATG)
		cc = read_detail(CATG);

	/*-------------------------------------------------------
	| The next few lines were added to try to		|
	| fix a problem with the sabg records missing for	|
	| the first sale record.				|
	-------------------------------------------------------*/
	while (cc || !in_range())
	{
		cc = read_sale(NEXT);
		if (cc)
			return;
		
		cc = read_detail(rep_type);
		if (cc)
			return;

		if (rep_type == C_CATG)
			cc = read_detail(CATG);
	}

	/*-----------------------
	| valid sale records	|
	-----------------------*/
	while (!cc && in_range())
	{
		if (!sa_freight && !strncmp(sale_rec.le_category,"FR+INS+OTHE",11))
		{
			cc = read_sale(NEXT);
			continue;
		}

		cc = check_c_catg(FALSE);
		if (!cc)
		{
			cc = check_sales(FALSE);
			if (!cc)
			{
				cc = check_catg(FALSE);
				if (!cc)
				{
					cc = check_c_ctype(FALSE);
					if (!cc)
					{
						cc = check_a_code(FALSE);
						if (!cc)
						{
							if (valid_record())
								sum_sale();
						}
					}
				}
			}
		}
		cc = read_sale(NEXT);
	}
	check_c_catg(TRUE);
	check_sales(TRUE);
	check_catg(TRUE);
	check_c_ctype(TRUE);
	check_a_code(TRUE);
	if (rep_type == C_CATG)
		prt_total("CUSTOMER TOTAL","CUSTOMER UNITS",grp_total,lst_yr_sale[1]);
	prt_total("GRAND TOTAL","GRAND UNITS",grand_tot,lst_yr_sale[2]);
}

int
check_c_catg (
 int    last_group)
{
	if (rep_type == C_CATG && delta_code(C_CATG))
	{
		prt_sale(first_line);
		if (cnt != 0)
			first_rec = FALSE;

		cc = read_detail(C_CATG);
		if (cc)
			return(cc);
		cc = read_detail(CATG);
		if (cc)
			return(cc);
		prt_total("CUSTOMER TOTAL","CUSTOMER UNITS",grp_total,lst_yr_sale[1]);
		init_sale();
		init_cust();
	}
	return(0);
}

int
check_sales (
 int    last_group)
{
	if (rep_type == SALES && (delta_code(SALES) || last_group))
	{
		prt_sale(first_line);
		if (cnt != 0)
			first_rec = FALSE;

		cc = read_detail(SALES);
		if (cc)
			return(cc);
		init_sale();
	}
	return(0);
}

int
check_catg (
 int    last_group)
{
	if ((rep_type == CATG || rep_type == C_CATG) && (delta_code(CATG) || last_group))
	{
		prt_sale(first_line);
		if (cnt != 0)
			first_rec = FALSE;

		cc = read_detail(CATG);
		if (cc)
			return(cc);
		init_sale();
	}
	return (EXIT_SUCCESS);
}

int
check_a_code (
 int    last_group)
{
	if (rep_type == A_CODE && (delta_code(A_CODE) || last_group))
	{
		prt_sale(first_line);
		if (cnt != 0)
			first_rec = FALSE;

		cc = read_detail(A_CODE);
		if (cc)
			return(cc);
		init_sale();
	}
	return(0);
}

int
check_c_ctype (
 int    last_group)
{
	if (rep_type == C_TYPE && (delta_code(C_TYPE) || last_group))
	{
		prt_sale(first_line);
		if (cnt != 0)
			first_rec = FALSE;

		cc = read_detail(C_TYPE);
		if (cc)
			return(cc);
		init_sale();
	}
	return(0);
}

/*=======================================================
| Check if the code that sale is read by has changed	|
=======================================================*/
int
delta_code (
 int    file_id)
{
	switch (file_id)
	{
	case	SALES:
		return(strcmp(sale_rec.le_sman,exsf_rec.sf_salesman_no));
		break;

	case	CATG:
		return(strncmp(sale_rec.le_category,low_value,strlen(low_value)));
		break;

	case	C_CATG:
		return(strcmp(sale_rec.le_dbt_no,cumr_rec.mr_dbt_no));
		break;

	case	A_CODE:
		return(strcmp(sale_rec.le_area,exaf_rec.af_area_code));
		break;

	case	C_TYPE:
		return(strcmp(sale_rec.le_ctype,excl_rec.cl_class_type));
		break;

	default:
		return(1);
		break;
	}
}

/*===============================================================
| print sale line iff some valid sale records have been read	|
===============================================================*/
void
prt_sale (
 int    first_line)
{
	if (cnt != 0)
	{
		dsp_process(" : ",local_rec.description);
		print_sale(first_line);
	}
	init_sale();
	cnt = 0;
}

/*=====================================================================
| Checks whether appropriate code is in subrange input.
=====================================================================*/
int
in_range (void)
{
	int	valid = 0;

	switch (rep_type)
	{
	case	SALES:
		valid = strncmp(sale_rec.le_sman,upper,2);
		break;

	case	CATG:
		valid = strncmp(sale_rec.le_category,upper,11);
		break;

	case	C_CATG:
		valid = strncmp(sale_rec.le_dbt_no,upper,6);
		break;

	case	A_CODE:
		valid = strncmp(sale_rec.le_area,upper,2);
		break;

	case	C_TYPE:
		valid = strncmp(sale_rec.le_ctype,upper,3);
		break;
	}
	return ((valid <= 0));
}

/*=====================================================================
| initialise salesman/category/customer type/area totals
=====================================================================*/
void
init_sale (void)
{
	int	i;

	cnt = 0;
	first_line = FALSE;
	for (i = 0;i < 8;i++)
		sales_tot[i] = 0.00;

	lst_yr_sale[0] = 0.00;
}

/*===============================
| initialise customer totals	|
===============================*/
void
init_cust (void)
{
	int	i;

	first_line = TRUE;
	for (i = 0;i < 8;i++)
		grp_total[i] = 0.00;

	lst_yr_sale[1] = 0.00;
}

/*=====================================================================
| initialise grand totals.
=====================================================================*/
void
init_total (void)
{
	int	i;

	for (i = 0;i < 8;i++)
		grand_tot[i] = 0.00;

	lst_yr_sale[2] = 0.00;
}

/*=======================================================
| print total lines iwth appropriate descriptions.	|
=======================================================*/
void
prt_total (
 char*      tot_desc,
 char*      unit_desc,
 double*    total_line,
 double     yr_total)
{
	double	percent(double l_profit, double l_sale);

	fprintf(fout,".LRP4\n");

	fprintf(fout,"|============================");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========");
	fprintf(fout,"|=========|\n");

	fprintf(fout,"| %-26.26s ",tot_desc);
	fprintf(fout,"|%9.0f",DOLLARS(total_line[0]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[1]));
	fprintf(fout,"|%8.0f",DOLLARS(total_line[0] - total_line[1]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[2]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[3]));
	fprintf(fout,"|%8.0f",DOLLARS(total_line[2] - total_line[3]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[4]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[5]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[4] - total_line[5]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[6]));
	fprintf(fout,"|%9.0f",DOLLARS(total_line[7]));
	fprintf(fout,"|%9.0f|\n",DOLLARS(total_line[6] - total_line[7]));

	fprintf(fout,"| %-26.26s ",unit_desc);
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%8.8s"," ");
	fprintf(fout,"|%8.2f%%",percent(total_line[2],total_line[0]));
	fprintf(fout,"|%8.2f%%",percent(total_line[3],total_line[1]));
	fprintf(fout,"|%7.2f%%",percent(total_line[2] - total_line[3],total_line[0] - total_line[1]));
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%8.2f%%",percent(total_line[6],total_line[4]));
	fprintf(fout,"|%8.2f%%",percent(total_line[7],total_line[5]));
	fprintf(fout,"|%8.2f%%|\n",percent(total_line[6] - total_line[7],total_line[4] - total_line[5]));
}

/*=======================================================
| sum all the appropriate sales records for the master	|
=======================================================*/
void
sum_sale (void)
{
	int	found = FALSE;
	int	mnth = atoi(cur_month) - 1;
	double	sales = (sale_rec.le_gross - sale_rec.le_disc);
	double	profit = sales - sale_rec.le_cost_sale;
	double	y_sales;
	double	y_profit;
	double	sum_ytd(int sales, int to_mnth, char *year);
	/*---------------------------------------
	| monthly comparison			|
	---------------------------------------*/
	if (sale_rec.le_year[0] == 'C')
	{
		/*---------------------------------------
		| actuals				|
		---------------------------------------*/
		if (!strcmp(sale_rec.le_period,cur_month))
		{
			/*---------------------------------------
			| sales					|
			---------------------------------------*/
			sales_tot[0]	+= sales;
			grp_total[0]	+= sales;
			grand_tot[0]	+= sales;
			/*---------------------------------------
			| profit				|
			---------------------------------------*/
			sales_tot[2]	+= profit;
			grp_total[2]	+= profit;
			grand_tot[2]	+= profit;
			found = TRUE;
		}

		if (in_fiscal(sale_rec.le_year))
		{
			/*---------------------------------------
			| sales					|
			---------------------------------------*/
			sales_tot[4]	+= sales;
			grp_total[4]	+= sales;
			grand_tot[4]	+= sales;
			/*---------------------------------------
			| profit				|
			---------------------------------------*/
			sales_tot[6]	+= profit;
			grp_total[6]	+= profit;
			grand_tot[6]	+= profit;
			found = TRUE;
		}
	}
	if (found && sabg_found)
	{
		if (csabg_found)
		{
			sales_tot[1]	+= CENTS(csabg_rec.bg_sales[mnth]);
			grp_total[1]	+= CENTS(csabg_rec.bg_sales[mnth]);
			grand_tot[1]	+= CENTS(csabg_rec.bg_sales[mnth]);
			sales_tot[3]	+= CENTS(csabg_rec.bg_profit[mnth]);
			grp_total[3]	+= CENTS(csabg_rec.bg_profit[mnth]);
			grand_tot[3]	+= CENTS(csabg_rec.bg_profit[mnth]);
		}
		y_sales = sum_ytd(TRUE,mnth,"C");
		sales_tot[5]	+= y_sales;
		grp_total[5]	+= y_sales;
		grand_tot[5]	+= y_sales;

		y_profit = sum_ytd(FALSE,mnth,"C");
		sales_tot[7]	+= y_profit;
		grp_total[7]	+= y_profit;
		grand_tot[7]	+= y_profit;

		lst_yr_sale[0] = sum_ytd(TRUE,mnth,"L");
		lst_yr_sale[1] += lst_yr_sale[0];
		lst_yr_sale[2] += lst_yr_sale[0];

		lsabg_found = FALSE;
		csabg_found = FALSE;
		sabg_found = FALSE;
	}
	if (found)
		cnt++;
}

/*=====================================================================
| print the sales analysis data                     
=====================================================================*/
void
print_sale (
 int    first_line)
{
	double	percent(double l_profit, double l_sale);

	fprintf(fout,".LRP4\n");

	fprintf(fout,"|----------------------------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|--------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|--------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------");
	fprintf(fout,"|---------|\n");

	if (rep_type == C_CATG)
	{
		if (first_line)
		{
			fprintf(fout,"|%-6.6s",cumr_rec.mr_dbt_no);
			fprintf(fout,"|%-9.9s",cumr_rec.mr_dbt_acronym);
		}
		else
		{
			fprintf(fout,"|%-6.6s"," ");
			fprintf(fout,"|%-9.9s"," ");
		}
		fprintf(fout,"|%-11.11s",local_rec.code);
	}
	else
		fprintf(fout,"| %-11.11s                ",local_rec.code);
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[0]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[1]));
	fprintf(fout,"|%8.0f",DOLLARS(sales_tot[0] - sales_tot[1]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[2]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[3]));
	fprintf(fout,"|%8.0f",DOLLARS(sales_tot[2] - sales_tot[3]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[4]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[5]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[4] - sales_tot[5]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[6]));
	fprintf(fout,"|%9.0f",DOLLARS(sales_tot[7]));
	fprintf(fout,"|%9.0f|\n",DOLLARS(sales_tot[6] - sales_tot[7]));

	fprintf(fout,"| %-26.26s ",(rep_type == C_CATG) ? " " : local_rec.description);
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%8.8s"," ");
	fprintf(fout,"|%8.2f%%",percent(sales_tot[2],sales_tot[0]));
	fprintf(fout,"|%8.2f%%",percent(sales_tot[3],sales_tot[1]));
	fprintf(fout,"|%7.2f%%",percent(sales_tot[2] - sales_tot[3],sales_tot[0] - sales_tot[1]));
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%9.9s"," ");
	fprintf(fout,"|%8.2f%%",percent(sales_tot[6],sales_tot[4]));
	fprintf(fout,"|%8.2f%%",percent(sales_tot[7],sales_tot[5]));
	fprintf(fout,"|%8.2f%%|\n",percent(sales_tot[6] - sales_tot[7],sales_tot[4] - sales_tot[5]));
}

/*=======================
| calculate percentage	|
=======================*/
double
percent (
 double l_profit,
 double l_sale)
{
	if (l_sale == 0.00)
		return(0.00);

	return((l_profit / l_sale) * 100.00);
}

/*=======================================
| TRUE iff comm_rec.tfiscal == 0L	|
| or sale_period in fiscal year		|
=======================================*/
int
in_fiscal (
 char*  year)
{
	/*---------------------------------------
	| valid if fiscal = 0			|
	---------------------------------------*/
	if (!strcmp(fiscal,"00"))
		return (TRUE);

	if (strcmp (fiscal,cur_month) < 0)
		return ((strcmp (fiscal,sale_rec.le_period) < 0 && 
                 strcmp (sale_rec.le_period,cur_month) <= 0));
	else
		return ((strcmp (fiscal,sale_rec.le_period) < 0 ||
                 strcmp (sale_rec.le_period,cur_month) <= 0));
}

/*=======================================
| Return ytd figures			|
=======================================*/
double
sum_ytd (
 int    sales,
 int    to_mnth,
 char*  year)
{
	int	i;
	double	ytd_data = (double) 0;

	if (to_mnth < cal_fiscal)
	{
		for (i = cal_fiscal; i < 12 ; i++)
		{
			if (csabg_found && year[0] == 'C')
			{
				if (sales)
					ytd_data += CENTS(csabg_rec.bg_sales[i]);
				else
					ytd_data += CENTS(csabg_rec.bg_profit[i]);
			}

			if (lsabg_found && year[0] == 'L')
				ytd_data += CENTS(lsabg_rec.bg_sales[i]);
		}
		for (i = 0; i <= to_mnth; i++)
		{
			if (csabg_found && year[0] == 'C')
			{
				if (sales)
					ytd_data += CENTS(csabg_rec.bg_sales[i]);
				else
					ytd_data += CENTS(csabg_rec.bg_profit[i]);
			}

			if (lsabg_found && year[0] == 'L')
				ytd_data += CENTS(lsabg_rec.bg_sales[i]);
		}
		return(ytd_data);
	}
	for (i = cal_fiscal; i <= to_mnth; i++)
	{
		if (csabg_found && year[0] == 'C')
		{
			if (sales)
				ytd_data += CENTS(csabg_rec.bg_sales[i]);
			else
				ytd_data += CENTS(csabg_rec.bg_profit[i]);
		}

		if (lsabg_found && year[0] == 'L')
			ytd_data += CENTS(lsabg_rec.bg_sales[i]);
	}
	return(ytd_data);
}

/*===============================================
| return TRUE iff the sale record is valid	|
| for the circumstance.				|
===============================================*/
int
valid_record (void)
{
	int	valid = 0;

	switch (analysis)
	{
	case	BY_CO:
		valid = !strncmp(comm_rec.tco_no,sale_rec.le_key,2);
		break;
		
	case	BY_BR:
		valid = (!strncmp(comm_rec.tco_no,sale_rec.le_key,2) &&
			!strncmp(comm_rec.test_no,sale_rec.le_key + 2,2));
		break;

	case	BY_DP:
		valid = (!strncmp(comm_rec.tco_no,sale_rec.le_key,2) &&
			!strncmp(comm_rec.test_no,sale_rec.le_key + 2,2) &&
			!strncmp(cudp_rec.dp_dp_no,sale_rec.le_key + 4,2));
		break;

	case	BY_WH:
		valid = (!strncmp(comm_rec.tco_no,sale_rec.le_key,2) &&
			!strncmp(comm_rec.test_no,sale_rec.le_key + 2,2) &&
			!strncmp(comm_rec.tcc_no,sale_rec.le_key + 6,2));
		break;
	}
	return(valid);
}

/*=======================================
| read the appropriate detail file	|
| ie exsf, excf, cumr, exaf, excl.	|
=======================================*/
int
read_detail (
 int    file_id)
{
	char	p_mask[12];

	switch (file_id)
	{
	case	SALES:
		strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(lsabg_rec.bg_sman_code,sale_rec.le_sman);
		strcpy(lsabg_rec.bg_year_flag,"L");
		lsabg_found = !find_rec("sabg",&lsabg_rec,COMPARISON,"r");

		strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(csabg_rec.bg_sman_code,sale_rec.le_sman);
		strcpy(csabg_rec.bg_year_flag,"C");
		csabg_found = !find_rec("sabg",&csabg_rec,COMPARISON,"r");

		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		strcpy(exsf_rec.sf_salesman_no,sale_rec.le_sman);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy(exsf_rec.sf_salesman_no,sale_rec.le_sman);
			sprintf(exsf_rec.sf_salesman,"%-40.40s","Salesman description not found");
		}

		strcpy(local_rec.code,exsf_rec.sf_salesman_no);
		strcpy(local_rec.description,exsf_rec.sf_salesman);
		break;

	case	CATG:
		strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		if (rep_type == C_CATG)
		{
			strcpy(lsabg_rec.bg_category,sale_rec.le_category);
			strcpy(csabg_rec.bg_category,sale_rec.le_category);
			strcpy(excf_rec.cf_categ_no,sale_rec.le_category);
		}
		else
		{
			sprintf(p_mask,mask,sale_rec.le_category);
			sprintf(lsabg_rec.bg_category,"%-11.11s",p_mask);
			sprintf(csabg_rec.bg_category,"%-11.11s",p_mask);
			sprintf(excf_rec.cf_categ_no,"%-11.11s",p_mask);
		}
		cc = find_rec("excf",&excf_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf(excf_rec.cf_categ_no,"%-11.11s",p_mask);
			sprintf(excf_rec.cf_categ_description,"%-40.40s","Category description not found");
		}

		strcpy(lsabg_rec.bg_year_flag,"L");
		lsabg_found = !find_rec("sabg",&lsabg_rec,COMPARISON,"r");

		strcpy(csabg_rec.bg_year_flag,"C");
		csabg_found = !find_rec("sabg",&csabg_rec,COMPARISON,"r");

		strcpy(local_rec.code,excf_rec.cf_categ_no);
		strcpy(local_rec.description,excf_rec.cf_categ_description);
		break;

	case	C_CATG:
		strcpy(cumr_rec.mr_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.mr_est_no,comm_rec.test_no);
		strcpy(cumr_rec.mr_dbt_no,sale_rec.le_dbt_no);
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy(cumr_rec.mr_co_no,comm_rec.tco_no);
			strcpy(cumr_rec.mr_est_no," 0");
			strcpy(cumr_rec.mr_dbt_no,sale_rec.le_dbt_no);
			cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");
		}
		strcpy(local_rec.code,cumr_rec.mr_dbt_no);
		strcpy(local_rec.description,cumr_rec.mr_dbt_acronym);
		lsabg_found = 0;
		csabg_found = 0;
		break;

	case	A_CODE:
		strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(lsabg_rec.bg_area_code,sale_rec.le_area);
		strcpy(lsabg_rec.bg_year_flag,"L");
		lsabg_found = !find_rec("sabg",&lsabg_rec,COMPARISON,"r");

		strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(csabg_rec.bg_area_code,sale_rec.le_area);
		strcpy(csabg_rec.bg_year_flag,"C");
		csabg_found = !find_rec("sabg",&csabg_rec,COMPARISON,"r");

		strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
		strcpy(exaf_rec.af_area_code,sale_rec.le_area);
		cc = find_rec("exaf",&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy(exaf_rec.af_area_code,sale_rec.le_area);
			sprintf(exaf_rec.af_area,"%-40.40s","Area description not found");
		}

		strcpy(local_rec.code,exaf_rec.af_area_code);
		strcpy(local_rec.description,exaf_rec.af_area);
		break;

	case	C_TYPE:
		strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(lsabg_rec.bg_ctype_code,sale_rec.le_ctype);
		strcpy(lsabg_rec.bg_year_flag,"L");
		lsabg_found = !find_rec("sabg",&lsabg_rec,COMPARISON,"r");

		strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(csabg_rec.bg_ctype_code,sale_rec.le_ctype);
		strcpy(csabg_rec.bg_year_flag,"C");
		csabg_found = !find_rec("sabg",&csabg_rec,COMPARISON,"r");

		strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
		strcpy(excl_rec.cl_class_type,sale_rec.le_ctype);
		cc = find_rec("excl",&excl_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy(excl_rec.cl_class_type,sale_rec.le_ctype);
			sprintf(excl_rec.cl_class_description,"%-40.40s","Class description not found");
		}

		strcpy(local_rec.code,excl_rec.cl_class_type);
		strcpy(local_rec.description,excl_rec.cl_class_description);
		break;
	}
	if (lsabg_found || csabg_found)
		sabg_found = TRUE;

	if (file_id == CATG && rep_type == C_CATG)
		strcpy(low_value,local_rec.code);
	else
		sprintf(low_value,mask,local_rec.code);
	
	if (cc)
		strcpy(local_rec.description," No Description found ");
	return(cc);
}

/*===============================================
| read the sale file on the appropriate key	|
===============================================*/
int
read_sale (
 int    stype)
{
	if (stype != NEXT)
	{
		switch (rep_type)
		{
		case	SALES:
			sprintf(sale_rec.le_sman,"%-2.2s",lower);
			break;

		case	CATG:
			sprintf(sale_rec.le_category,"%-11.11s",lower);
			break;

		case	C_CATG:
			sprintf(sale_rec.le_dbt_no,"%-6.6s",lower);
			sprintf(sale_rec.le_category,"%-11.11s"," ");
			break;

		case	A_CODE:
			sprintf(sale_rec.le_area,"%-2.2s",lower);
			break;

		case	C_TYPE:
			sprintf(sale_rec.le_ctype,"%-3.3s",lower);
			break;
		}
	}
	return(find_rec("sale",&sale_rec,stype,"r"));
}

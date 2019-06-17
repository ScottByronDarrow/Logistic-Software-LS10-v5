/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_smancat.c  )                                  |
|  Program Desc  : ( Print Sales by Salesman/Category/Item           )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sadf, sabg                            |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 07/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : 13/02/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 02/03/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 08/03/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 07/04/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 06/11/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 09/11/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 18/10/92        | Modified  by  : Anneliese Allen. |
|  Date Modified : 27/05/96        | Modified  by  : Jiggs Veloz.     |
|  Date Modified : 10/09/97        | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : Added salesman code to sapc, so don't have to      |
|                : look at cust. master to get the salesman no.       |
|                : Added option to either print with Cost-Margin% cols|
|                : or not.                                            |
|                : Print comm_dbt_date at top left of report          |
|                : Print "For The Month of ...." as rept. heading.    |
|                :                                                    |
|  (06/11/91)    : Converted program to use sadf rather than sapc.    |
|                :                                                    |
|  (09/11/91)    : Fixed branch headings.                             |
|                :                                                    |
|  (18/10/92)    : Now totals at major category headings PSM 7377.    |
|                :                                                    |
|  (09/12/92)    : Includes budget and forecast totals for salesman,  |
|                : Branch and company. PSM 8132			      		  |
|                :                                                    |
|  (27/05/96)    : Fixed problems in DateToString. 					      |
|  (10/09/97)    : Modified for Multilingual Conversion and           |
|                                                                     |
| $Log: sa_smancat.c,v $
| Revision 5.2  2001/08/09 09:17:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:53  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:03  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:34  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/02/18 02:35:26  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.15  1999/12/06 01:35:32  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/16 04:55:35  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.13  1999/10/20 02:07:02  nz
| Updated for final changes on date routines.
|
| Revision 1.12  1999/10/16 01:11:22  nz
| Updated for pjulmdy routines
|
| Revision 1.11  1999/09/29 10:12:51  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:27:37  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/06/18 09:39:22  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_smancat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_smancat/sa_smancat.c,v 5.2 2001/08/09 09:17:13 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_sa_mess.h>
#include	<pr_format3.h>

#define		DETAIL		(type[0] == 'D')
#define		SUMMARY		(type[0] == 'S')
#define		CO	0
#define		BR	1
#define		COST_MGN	(cost_mgn[0] == 'Y')

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
		long	tfiscal;
	} comm_rec;

	/*=============================================
	| Sales Analysis Detail file By Item/Customer |
	=============================================*/
	struct dbview sadf_list[] ={
		{"sadf_co_no"},
		{"sadf_br_no"},
		{"sadf_year"},
		{"sadf_hhbr_hash"},
		{"sadf_hhcu_hash"},
		{"sadf_qty_per1"},
		{"sadf_qty_per2"},
		{"sadf_qty_per3"},
		{"sadf_qty_per4"},
		{"sadf_qty_per5"},
		{"sadf_qty_per6"},
		{"sadf_qty_per7"},
		{"sadf_qty_per8"},
		{"sadf_qty_per9"},
		{"sadf_qty_per10"},
		{"sadf_qty_per11"},
		{"sadf_qty_per12"},
		{"sadf_sal_per1"},
		{"sadf_sal_per2"},
		{"sadf_sal_per3"},
		{"sadf_sal_per4"},
		{"sadf_sal_per5"},
		{"sadf_sal_per6"},
		{"sadf_sal_per7"},
		{"sadf_sal_per8"},
		{"sadf_sal_per9"},
		{"sadf_sal_per10"},
		{"sadf_sal_per11"},
		{"sadf_sal_per12"},
		{"sadf_cst_per1"},
		{"sadf_cst_per2"},
		{"sadf_cst_per3"},
		{"sadf_cst_per4"},
		{"sadf_cst_per5"},
		{"sadf_cst_per6"},
		{"sadf_cst_per7"},
		{"sadf_cst_per8"},
		{"sadf_cst_per9"},
		{"sadf_cst_per10"},
		{"sadf_cst_per11"},
		{"sadf_cst_per12"},
		{"sadf_sman"},
		{"sadf_area"},
	};

	int	sadf_no_fields = 43;

	struct	{
		char	df_co_no[3];
		char	df_br_no[3];
		char	df_year[2];
		long	df_hhbr_hash;
		long	df_hhcu_hash;
		float	df_qty_per[12];
		double	df_sal_per[12];
		double	df_cst_per[12];
		char	df_sman[3];
		char	df_area[3];
	} sadf_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int exsf_no_fields = 3;

	struct {
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
	} exsf_rec;

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
		char	cf_cat_no[12];
		char	cf_cat_desc[41];
		char	cf_stat_flag[2];
	} excf_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
	};

	int inmr_no_fields = 6;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
	} inmr_rec;

	/*=====================
	| Sales / Budget file |
	=====================*/
	struct dbview sabg_list[] ={
		{"sabg_co_no"},
		{"sabg_sman_code"},
		{"sabg_category"},
		{"sabg_hhcu_hash"},
		{"sabg_year_flag"},
		{"sabg_bg1_qty"},
		{"sabg_bg2_qty"},
		{"sabg_bg3_qty"},
		{"sabg_bg4_qty"},
		{"sabg_bg5_qty"},
		{"sabg_bg6_qty"},
		{"sabg_bg7_qty"},
		{"sabg_bg8_qty"},
		{"sabg_bg9_qty"},
		{"sabg_bg10_qty"},
		{"sabg_bg11_qty"},
		{"sabg_bg12_qty"},
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
		{"sabg_bg12_profit"},
	};

	int	sabg_no_fields = 29;

	struct	{
		char	bg_co_no[3];
		char	bg_sman_code[3];
		char	bg_category[12];
		long	bg_hhcu_hash;
		char	bg_year_flag[2];
		double	bg_quantity[12];
		double	bg_sales[12];
		double	bg_profit[12];
	} sabg_rec;

FILE	*fin;
FILE	*fout;
FILE	*fsort;

char	sort_str[111];
char	data_str[111];
char	branchNo[3];
char	br_no[3];
char	year_flag[2];

char	prev_br[3];
char	curr_br[3];
char	prev_cat[12];
char	curr_cat[12];
char	prev_prod[17];
char	curr_prod[17];
char	prev_sman[3];
char	curr_sman[3];
char	prev_group[3];
char	curr_group[3];

int	curr_mnth;
int	sub_catg;
int	fiscal;
int	found_data = 0;
int	first_time = TRUE;
int	printed = FALSE;
int	srt_all = FALSE;
int	end_all = FALSE;
int	printerNumber = 1;

long	hhbr_hash = 0L;
long	hhcu_hash = 0L;
long	prev_hash = 0L;
long	curr_hash = 0L;

float	m_qty[5];
float	y_qty[5];
double	m_sales[5];
double	m_csale[5];
double	y_sales[5];
double	y_csale[5];
float	sman_m_qty;
double	sman_m_sales;
double	sman_m_profit;
float	sman_y_qty[2];
double	sman_y_sales[2];
double	sman_y_profit[2];
double	sabg_m_qty[2][2];
double	sabg_m_sal[2][2];
double	sabg_y_qty[2][2];
double	sabg_y_sal[2][2];

float	tot_m_qty[5];
float	tot_y_qty[5];
double	tot_m_sales[5];
double	tot_m_csale[5];
double	tot_y_sales[5];
double	tot_y_csale[5];
char	s_sman[3];
char	e_sman[3];
char	type[2];
char	cost_mgn[2];

char	*comm = "comm",
	*data = "data",
	*sadf = "sadf",
	*inmr = "inmr",
	*exsf = "exsf",
	*excf = "excf",
	*sabg = "sabg";

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void proc_sadf (void);
void store_data (void);
int calc_ytd (void);
void init_array (void);
void proc_sorted (void);
void set_break (char *sort_str);
int check_break (void);
void proc_data (int first_time, int print_type);
void sum_sales (char *data_line, int add);
void print_maj_cat_tot (void);
void print_line (void);
void print_total (char* tot_type);
int print_sman_data (char *year_flag);
int print_data (int tot_type, char *year_flag);
void init_output (void);
int check_page (void);
void print_header (int first_time, char *sman_cat);
char *month_name (int n);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	strcpy(s_sman,"  ");
	strcpy(e_sman,"~~");

	if (argc != 7)
	{
		/*printf("Usage : %s <Report_type> <lower> <upper> <printerNumber> <Br_no> <With Cost-Margin [Y|N]> \007\n\r",argv[0]);*/
		print_at(0,0, ML(mlSaMess717), argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf(type,"%-1.1s",argv[1]);
	sprintf(s_sman,"%-2.2s",argv[2]);
	sprintf(e_sman,"%-2.2s",argv[3]);
	if (!strcmp(s_sman,"  "))
		srt_all = TRUE;
	if (!strcmp(e_sman,"~~"))
		end_all = TRUE;

	printerNumber = atoi(argv[4]);

	if (!strncmp(argv[5],"All",3))
		strcpy(br_no,"  ");
	else
		sprintf(br_no,"%-2.2s",argv[5]);
	sprintf(cost_mgn,"%-1.1s",argv[6]);

	OpenDB();

	DateToDMY (comm_rec.tdbt_date, NULL, &curr_mnth, NULL);

	sptr = chk_env("DIS_FIND");
	sub_catg = ( sptr == (char *)0 ) ? 11 : atoi( sptr );

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;

	if (COST_MGN)
	{
		if ((fin = pr_open("sa_smancat.p")) == NULL)
			sys_err("Error in opening sa_smancat.p during (FOPEN)",errno,PNAME);
	}
	else
	{
		if ((fin = pr_open("sa_smcat.p")) == NULL)
			sys_err("Error in opening sa_smcat.p during (FOPEN)",errno,PNAME);
	}

	init_output();

	proc_sadf();

	/*---------------------------
	| Process data in sort file |
	---------------------------*/
	proc_sorted();

	fprintf(fout,".EOF\n");
	pclose(fout);

	shutdown_prog  ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec(sadf,sadf_list,sadf_no_fields,"sadf_id_no");
	open_rec(inmr,inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec(exsf,exsf_list,exsf_no_fields,"exsf_id_no");
	open_rec(excf,excf_list,excf_no_fields,"excf_id_no");
	open_rec(sabg,sabg_list,sabg_no_fields,"sabg_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(sadf);
	abc_fclose(exsf);
	abc_fclose(excf);
	abc_fclose(inmr);
	abc_fclose(sabg);
	abc_dbclose(data);
}

/*========================================================
| Read data into sort file,then process the information |
========================================================*/
void
proc_sadf (void)
{
	int	store = TRUE;

	strcpy(sadf_rec.df_co_no, comm_rec.tco_no);
	strcpy(sadf_rec.df_br_no, br_no);
	strcpy(sadf_rec.df_year, "C");
	sadf_rec.df_hhbr_hash = 0L;
	sadf_rec.df_hhcu_hash = 0L;
	strcpy(sadf_rec.df_sman, "  ");
	strcpy(sadf_rec.df_area, "  ");
	
	cc = find_rec(sadf, &sadf_rec, GTEQ, "r");
	while (!cc && !strcmp(sadf_rec.df_co_no, comm_rec.tco_no) &&
	       !strcmp(sadf_rec.df_year, "C") )
	{
		store = TRUE;

		/*--------------------------------------------------
		| If different branch & it is not for All Branches |
		| break else continue.				   |
		--------------------------------------------------*/
		if (strcmp(sadf_rec.df_br_no,br_no) && strcmp(br_no,"  "))
			break;

		if (strcmp(sadf_rec.df_sman, s_sman) < 0 || 
	  	    strcmp(sadf_rec.df_sman, e_sman) > 0)
			store = FALSE;

		if (store)
			store_data();

		cc = find_rec(sadf, &sadf_rec, NEXT, "r");
	}
}

void
store_data (void)
{
	dsp_process("Salesman No : ",sadf_rec.df_sman);
	
	cc = find_hash(inmr,&inmr_rec,COMPARISON,"r",sadf_rec.df_hhbr_hash);
	if (cc)
	{
		sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");
		sprintf(inmr_rec.mr_category,"%-11.11s"," ");
		inmr_rec.mr_hhbr_hash = 0L;
	}

	calc_ytd();

	sprintf(data_str,
		"%-2.2s %-2.2s %-11.11s %-16.16s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %6ld\n",
		sadf_rec.df_br_no,
		sadf_rec.df_sman,
		inmr_rec.mr_category,
		inmr_rec.mr_item_no,
		sadf_rec.df_qty_per[curr_mnth - 1],
		sadf_rec.df_sal_per[curr_mnth - 1],
		sadf_rec.df_cst_per[curr_mnth - 1],
		y_qty[0],
		y_sales[0],
		y_csale[0],
		inmr_rec.mr_hhbr_hash);

	sort_save (fsort,data_str);
}

int
calc_ytd (void)
{
	int i;

	y_qty[0] = 0.00;
	y_sales[0] = 0.00;
	y_csale[0] = 0.00;

	if (curr_mnth <= fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			y_qty[0] += sadf_rec.df_qty_per[i];
			y_sales[0] += sadf_rec.df_sal_per[i];
			y_csale[0] += sadf_rec.df_cst_per[i];
		}

		for (i = 0; i < curr_mnth; i++)
		{
			y_qty[0] += sadf_rec.df_qty_per[i];
			y_sales[0] += sadf_rec.df_sal_per[i];
			y_csale[0] += sadf_rec.df_cst_per[i];
		}

	}
	else
	{
		for (i = fiscal; i < curr_mnth; i++)
		{
			y_qty[0] += sadf_rec.df_qty_per[i];
			y_sales[0] += sadf_rec.df_sal_per[i];
			y_csale[0] += sadf_rec.df_cst_per[i];
		}
	}

	return (EXIT_SUCCESS);
}

void
init_array (void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		m_qty[j] = 0.00;
		y_qty[j] = 0.00;
		m_sales[j] = 0.00;
		m_csale[j] = 0.00;
		y_sales[j] = 0.00;
		y_csale[j] = 0.00;
	}
}

void
proc_sorted (void)
{
	char	*sptr;
	int	print_type;

	init_array();
	printed = FALSE;
	first_time = TRUE;

	fsort = sort_sort(fsort,"sman");
	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(sort_str,"%-110.110s",sptr);
		dsp_process("Salesman No :",curr_sman);

		set_break(sort_str);

		if (first_time)
		{
			strcpy(prev_br,curr_br);
			strcpy(prev_sman,curr_sman);
			strcpy(prev_cat,curr_cat);
			strcpy(prev_prod,curr_prod);
			strcpy(prev_group,curr_group);
			prev_hash = curr_hash;
		}

		print_type = check_break();

		proc_data(first_time,print_type);
		first_time = 0;

		if (print_type == 0)
			sum_sales(sptr + 35,1);
		else
			sum_sales(sptr + 35,0);

		strcpy(prev_br,curr_br);
		strcpy(prev_sman,curr_sman);
		strcpy(prev_cat,curr_cat);
		strcpy(prev_prod,curr_prod);
		strcpy(prev_group,curr_group);
		prev_hash = curr_hash;

		sptr = sort_read(fsort);
	}
	if (printed)
	{
		print_line();
		if (DETAIL)
			print_total("C");
		print_maj_cat_tot();
		print_total("S");
		print_sman_data("C");
		print_sman_data("N");
		print_total("B");
		print_data(BR,"C");
		print_data(BR,"N");
		print_total("E");
		print_data(CO,"C");
		print_data(CO,"N");
	}
	sort_delete(fsort,"sman");
}

void
set_break (
 char*  sort_str)
{
	sprintf(curr_br,"%-2.2s",sort_str);
	sprintf(curr_sman,"%-2.2s",sort_str + 3);
	sprintf(curr_cat,"%-11.11s",sort_str + 6);
	sprintf(curr_prod,"%-16.16s",sort_str + 18);
	sprintf(curr_group, "%-*.*s", sub_catg, sub_catg, sort_str + 6);
	curr_hash = atol(sort_str + 101);
}

int
check_break (void)
{
	if (strcmp(curr_br,prev_br))
		return(1);

	if (strcmp(curr_sman,prev_sman))
		return(2);

	if (strcmp(curr_cat,prev_cat))
		return(DETAIL ? 3 : 4);

	if (DETAIL && strcmp(curr_prod,prev_prod))
		return(4);

	return (EXIT_SUCCESS);
}

void
proc_data (
 int    first_time,
 int    print_type)
{

	if (first_time)
	{
		print_header(first_time,"S");
		if (DETAIL)
			print_header(first_time,"C");
		return;
	}

	if (!first_time && (print_type == 1 || print_type == 2 || print_type == 3))
	{
		print_line();

		switch (print_type)
		{
		case	1:
			if (DETAIL)
				print_total("C");
			print_maj_cat_tot();
			print_total("S");
			print_sman_data("C");
			print_sman_data("N");
			print_total("B");
			print_data(BR,"C");
			print_data(BR,"N");
			print_header(first_time,"S");
			print_header(first_time,"C");
			break;

		case	2:
			if (DETAIL)
				print_total("C");
			print_maj_cat_tot();
			print_total("S");
			print_sman_data("C");
			print_sman_data("N");
			print_header(first_time,"S");
			if (DETAIL)
				print_header(first_time,"C");
			break;

		case	3:
			print_total("C");

			if (strcmp(prev_group, curr_group) != 0)
				print_maj_cat_tot();
			print_header(first_time,"C");
			break;
		}
	}

	if (print_type == 4)
	{
		print_line();
		if (strcmp(prev_group, curr_group) != 0)
			print_maj_cat_tot();
	}
}

void
sum_sales (
 char*  data_line,
 int    add)
{
	char	*sptr = data_line;
	register	int	i;

	i = ((DETAIL) ? 0 : 1);
	if (add)
	{
		m_qty[i]   += atof(sptr);
		m_sales[i] += atof(sptr + 11);
		m_csale[i] += atof(sptr + 22);
		y_qty[i]   += atof(sptr + 33);
		y_sales[i] += atof(sptr + 44);
		y_csale[i] += atof(sptr + 55);
	}
	else
	{
		m_qty[i]   = atof(sptr);
		m_sales[i] = atof(sptr + 11);
		m_csale[i] = atof(sptr + 22);
		y_qty[i]   = atof(sptr + 33);
		y_sales[i] = atof(sptr + 44);
		y_csale[i] = atof(sptr + 55);
	}
}

void
print_maj_cat_tot (void)
{
	register	int	i;
	float	tot_m_margin = 0.00;
	float	tot_y_margin = 0.00;
	char	err_str2[26];

	i = ((DETAIL) ? 0 : 1);

	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no,"%-2.2s%-9.9s", prev_group, " ");
	cc = find_rec(excf, &excf_rec, COMPARISON, "r");
	if (cc)
		strcpy(err_str2, "Unknown Category         ");
	else
		sprintf(err_str2,"%-25.25s",excf_rec.cf_cat_desc);

	sprintf(err_str,"%-s","Total For Category");
	
	if (COST_MGN)
	{
		if (tot_m_sales[i] > 0.00)
			tot_m_margin = (tot_m_sales[i] - tot_m_csale[i]) / tot_m_sales[i] * 100.00;
		if (tot_y_sales[i] > 0.00)
			tot_y_margin = (tot_y_sales[i] - tot_y_csale[i]) / tot_y_sales[i] * 100.00;

		pr_format(fin,fout,"MAJ_TOT",1,err_str);
		pr_format(fin,fout,"MAJ_TOT",2,prev_group);
		pr_format(fin,fout,"MAJ_TOT",3,err_str2);
		pr_format(fin,fout,"MAJ_TOT",4,tot_m_qty[i]);
		pr_format(fin,fout,"MAJ_TOT",5,tot_m_sales[i]);
		pr_format(fin,fout,"MAJ_TOT",6,tot_m_csale[i]);
		pr_format(fin,fout,"MAJ_TOT",7,tot_m_margin);
		pr_format(fin,fout,"MAJ_TOT",8,tot_y_qty[i]);
		pr_format(fin,fout,"MAJ_TOT",9,tot_y_sales[i]);
		pr_format(fin,fout,"MAJ_TOT",10,tot_y_csale[i]);
		pr_format(fin,fout,"MAJ_TOT",11,tot_y_margin);
		if (SUMMARY)
			pr_format(fin,fout,"SEPARATOR",0,0);
		if (DETAIL)
			pr_format(fin,fout,"DBL_LINE",0,0);
	}
	else
	{
		pr_format(fin,fout,"MAJ_TOT",1,err_str);
		pr_format(fin,fout,"MAJ_TOT",2,tot_m_qty[i]);
		pr_format(fin,fout,"MAJ_TOT",3,err_str2);
		pr_format(fin,fout,"MAJ_TOT",4,tot_m_sales[i]);
		pr_format(fin,fout,"MAJ_TOT",5,tot_y_qty[i]);
		pr_format(fin,fout,"MAJ_TOT",6,tot_y_sales[i]);
		if (SUMMARY)
			pr_format(fin,fout,"SEPARATOR",0,0);
		if (DETAIL)
			pr_format(fin,fout,"DBL_LINE",0,0);
	}	

	tot_m_qty[i] = 0.00;
	tot_m_sales[i] = 0.00;
	tot_m_csale[i] = 0.00;
	tot_y_qty[i] = 0.00;
	tot_y_sales[i] = 0.00;
	tot_y_csale[i] = 0.00;
}

void
print_line (void)
{
	register	int	i;
	float	m_margin = 0.00;
	float	y_margin = 0.00;

	i = ((DETAIL) ? 0 : 1);

	if (DETAIL)
	{
		cc = find_hash(inmr,&inmr_rec,COMPARISON,"r",prev_hash);
		if (cc)
			strcpy(inmr_rec.mr_description,"No Description found");
	}
	else
	{
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_cat_no,"%-11.11s",prev_cat);
		cc = find_rec(excf, &excf_rec, COMPARISON, "r");
		if (cc)
			strcpy(excf_rec.cf_cat_desc,"No Description found");
	}
	
	if (COST_MGN)
	{
		if (m_sales[i] > 0.00)
			m_margin = (m_sales[i] - m_csale[i]) / m_sales[i] * 100.00;
		if (y_sales[i] > 0.00)
			y_margin = (y_sales[i] - y_csale[i]) / y_sales[i] * 100.00;

		pr_format(fin,fout,"PROD_DET",1,((DETAIL) ? inmr_rec.mr_item_no : excf_rec.cf_cat_no));
		pr_format(fin,fout,"PROD_DET",2,((DETAIL) ? inmr_rec.mr_description : excf_rec.cf_cat_desc));
		pr_format(fin,fout,"PROD_DET",3,m_qty[i]);
		pr_format(fin,fout,"PROD_DET",4,m_sales[i]);
		pr_format(fin,fout,"PROD_DET",5,m_csale[i]);
		pr_format(fin,fout,"PROD_DET",6,m_margin);
		pr_format(fin,fout,"PROD_DET",7,y_qty[i]);
		pr_format(fin,fout,"PROD_DET",8,y_sales[i]);
		pr_format(fin,fout,"PROD_DET",9,y_csale[i]);
		pr_format(fin,fout,"PROD_DET",10,y_margin);
	}
	else
	{
		pr_format(fin,fout,"PROD_DET",1,((DETAIL) ? inmr_rec.mr_item_no : excf_rec.cf_cat_no));
		pr_format(fin,fout,"PROD_DET",2,((DETAIL) ? inmr_rec.mr_description : excf_rec.cf_cat_desc));
		pr_format(fin,fout,"PROD_DET",3,m_qty[i]);
		pr_format(fin,fout,"PROD_DET",4,m_sales[i]);
		pr_format(fin,fout,"PROD_DET",5,y_qty[i]);
		pr_format(fin,fout,"PROD_DET",6,y_sales[i]);
	}

	m_qty[i+1] += m_qty[i];
	m_sales[i+1] += m_sales[i];
	m_csale[i+1] += m_csale[i];
	y_qty[i+1] += y_qty[i];
	y_sales[i+1] += y_sales[i];
	y_csale[i+1] += y_csale[i];

	tot_m_qty[i] += m_qty[i];
	tot_m_sales[i] += m_sales[i];
	tot_m_csale[i] += m_csale[i];
	tot_y_qty[i] += y_qty[i];
	tot_y_sales[i] += y_sales[i];
	tot_y_csale[i] += y_csale[i];

	m_qty[i] = 0.00;
	m_sales[i] = 0.00;
	m_csale[i] = 0.00;
	y_qty[i] = 0.00;
	y_sales[i] = 0.00;
	y_csale[i] = 0.00;
}

void
print_total (
 char*  tot_type)
{
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	int	j = 0;

	switch (tot_type[0])
	{
	case	'C':
		j = 1;
		sprintf(err_str,"%-s","Total For Category");
		break;
	case	'S':
		j = 2;
		pr_format(fin,fout,"SEPARATOR",0,0);
		sprintf(err_str,"%-18.18s %-3.3s","Total For Salesman", prev_sman);
		break;
	case	'B':
		j = 3;
		pr_format(fin,fout,"SEPARATOR",0,0);
		sprintf(err_str,"%-s","Total For Branch  ");
		break;
	case	'E':
		j = 4;
		pr_format(fin,fout,"SEPARATOR",0,0);
		sprintf(err_str,"%-s","Total For Company ");
		break;
	}

	if (COST_MGN)
	{
		if (m_sales[j] > 0.00)
			m_margin = (m_sales[j] - m_csale[j]) / m_sales[j] * 100.00;
		if (y_sales[j] > 0.00)
			y_margin = (y_sales[j] - y_csale[j]) / y_sales[j] * 100.00;

		pr_format(fin,fout,"SUB_TOT",1,err_str);
		pr_format(fin,fout,"SUB_TOT",2,m_qty[j]);
		pr_format(fin,fout,"SUB_TOT",3,m_sales[j]);
		pr_format(fin,fout,"SUB_TOT",4,m_csale[j]);
		pr_format(fin,fout,"SUB_TOT",5,m_margin);
		pr_format(fin,fout,"SUB_TOT",6,y_qty[j]);
		pr_format(fin,fout,"SUB_TOT",7,y_sales[j]);
		pr_format(fin,fout,"SUB_TOT",8,y_csale[j]);
		pr_format(fin,fout,"SUB_TOT",9,y_margin);
	}
	else
	{
		pr_format(fin,fout,"SUB_TOT",1,err_str);
		pr_format(fin,fout,"SUB_TOT",2,m_qty[j]);
		pr_format(fin,fout,"SUB_TOT",3,m_sales[j]);
		pr_format(fin,fout,"SUB_TOT",4,y_qty[j]);
		pr_format(fin,fout,"SUB_TOT",5,y_sales[j]);
	}


	m_qty[j+1] += m_qty[j];
	m_sales[j+1] += m_sales[j];
	m_csale[j+1] += m_csale[j];
	y_qty[j+1] += y_qty[j];
	y_sales[j+1] += y_sales[j];
	y_csale[j+1] += y_csale[j];

	tot_m_qty[j] += m_qty[j];
	tot_m_sales[j] += m_sales[j];
	tot_m_csale[j] += m_csale[j];
	tot_y_qty[j] += y_qty[j];
	tot_y_sales[j] += y_sales[j];
	tot_y_csale[j] += y_csale[j];

	m_qty[j] = 0.00;
	m_sales[j] = 0.00;
	m_csale[j] = 0.00;
	y_qty[j] = 0.00;
	y_sales[j] = 0.00;
	y_csale[j] = 0.00;
}

int
print_sman_data (
 char*  year_flag)
{
	int	i;
	int	j;
	int	year = 0;

	sman_m_qty = 0.00;
	sman_m_sales = 0.00;
	sman_m_profit = 0.00;

	for (i = 0; i < 2; i++)
	{
		sman_y_qty[i]   = 0.00;
		sman_y_sales[i] = 0.00;
		sman_y_profit[i] = 0.00;
	}

	strcpy(sabg_rec.bg_co_no,comm_rec.tco_no);
	sprintf(sabg_rec.bg_sman_code,"%-2.2s",prev_sman);
	strcpy(sabg_rec.bg_year_flag,year_flag);

	cc = find_rec(sabg, &sabg_rec, COMPARISON, "r");
	if(cc)
	{
		for(i = 0; i < 12; i++)
		{
			sabg_rec.bg_quantity[i] = 0.00;
			sabg_rec.bg_sales[i] = 0.00;
			sabg_rec.bg_profit[i] = 0.00;
		}			
	}

	sman_m_qty = sabg_rec.bg_quantity[curr_mnth - 1];
	sman_m_sales = sabg_rec.bg_sales[curr_mnth - 1];
	sman_m_profit = sabg_rec.bg_profit[curr_mnth - 1];

	if(year_flag[0] == 'C')
		year = 0;
	if(year_flag[0] == 'N')
		year = 1;

	if (curr_mnth > fiscal)
	{
		for (j = fiscal; j < curr_mnth; j++)
		{
			sman_y_qty[year]   += sabg_rec.bg_quantity[j];
			sman_y_sales[year] += sabg_rec.bg_sales[j];
			sman_y_profit[year] += sabg_rec.bg_profit[j];
		}
	}
	else
	{
		for (j = fiscal; j < 12; j++)
		{
			sman_y_qty[year]   += sabg_rec.bg_quantity[j];
			sman_y_sales[year] += sabg_rec.bg_sales[j];
			sman_y_profit[year] += sabg_rec.bg_profit[j];
		}

		for (j = 0; j < curr_mnth; j++)
		{
			sman_y_qty[year]   += sabg_rec.bg_quantity[j];
			sman_y_sales[year] += sabg_rec.bg_sales[j];
			sman_y_profit[year] += sabg_rec.bg_profit[j];
		}
	}

	if(year_flag[0] == 'C')
		sprintf(err_str,"%-s","Budgets");
	if(year_flag[0] == 'N')
		sprintf(err_str,"%-s","Forecasts");

	pr_format(fin,fout,"SMAN_DATA",1,err_str);
	pr_format(fin,fout,"SMAN_DATA",2,sman_m_qty);
	pr_format(fin,fout,"SMAN_DATA",3,sman_m_sales);
	pr_format(fin,fout,"SMAN_DATA",4,sman_y_qty[year]);
	pr_format(fin,fout,"SMAN_DATA",5,sman_y_sales[year]);

	sabg_m_qty[BR][year] += sman_m_qty;
	sabg_m_sal[BR][year] += sman_m_sales;
	sabg_y_qty[BR][year] += sman_y_qty[year];
	sabg_y_sal[BR][year] += sman_y_sales[year];

	sabg_m_qty[CO][year] += sman_m_qty;
	sabg_m_sal[CO][year] += sman_m_sales;
	sabg_y_qty[CO][year] += sman_y_qty[year];
	sabg_y_sal[CO][year] += sman_y_sales[year];

	return(0);
}

int		
print_data (
 int    tot_type,
 char*  year_flag)
{
	int	year = 0;

	if(year_flag[0] == 'C')
	{
		year = 0;
		sprintf(err_str,"%-s","Budgets");
	}
	if(year_flag[0] == 'N')
	{
		year = 1;
		sprintf(err_str,"%-s","Forecasts");
	}

	pr_format(fin,fout,"SABG_DATA",1,err_str);
	pr_format(fin,fout,"SABG_DATA",2,sabg_m_qty[tot_type][year]);
	pr_format(fin,fout,"SABG_DATA",3,sabg_m_sal[tot_type][year]);
	pr_format(fin,fout,"SABG_DATA",4,sabg_y_qty[tot_type][year]);
	pr_format(fin,fout,"SABG_DATA",5,sabg_y_sal[tot_type][year]);

	/*if(year_flag[0] == 'N')
		pr_format(fin,fout,"SEPARATOR",0,0);*/

	sabg_m_qty[tot_type][year] = 0.00;
	sabg_m_sal[tot_type][year] = 0.00;
	sabg_y_qty[tot_type][year] = 0.00;
	sabg_y_sal[tot_type][year] = 0.00;

	return(0);
}	

void		
init_output (void)
{
	char	*month_name(int n);
	int	st_mth;

	DateToDMY (comm_rec.tdbt_date, NULL, &st_mth, NULL);

	fsort = sort_open("sman");

	/*-----------------------
	| Open pipe to pformat  |
	-----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);
		
	fprintf(fout,".START%s<%s>\n",DateToString (comm_rec.tdbt_date), PNAME);
	fprintf(fout,".LP%d\n",printerNumber);
	fprintf(fout,".PI12\n");
	fprintf(fout,".12\n");
	fprintf(fout,".L%d\n",(COST_MGN) ? 158 : 116);

	if (DETAIL)
	{
		dsp_screen(" Printing Salesmen' Sales By Category By Item ",comm_rec.tco_no,comm_rec.tco_name);
		fprintf(fout,".ESALESMAN SALES BY CATEGORY BY ITEM REPORT\n");
	}
	else
	{
		dsp_screen(" Printing Salesman Sales By Category Report ",comm_rec.tco_no,comm_rec.tco_name);
		fprintf(fout,".ESALESMAN SALES BY CATEGORY REPORT\n");
	}
	if (!srt_all)
	{
		if (!end_all)
			sprintf(err_str," FROM SALESMAN  %-2.2s  TO SALESMAN  %-2.2s ",s_sman,e_sman);
		else
			sprintf(err_str," FROM SALESMAN  %-2.2s  TO ALL SALESMEN ",s_sman);
	}
	else
		strcpy(err_str," FOR ALL SALESMEN ");
		
	fprintf(fout,".ECOMPANY : %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));

	fprintf(fout,".EFOR THE MONTH OF %-9.9s\n",month_name(st_mth - 1));

	if (!strcmp(br_no,"  "))
		fprintf(fout,".EFOR ALL BRANCHES\n");
	else
		fprintf(fout,".EFOR BRANCH %-2.2s\n",br_no);

	fprintf(fout,".E%s\n",err_str);
	pr_format(fin,fout,"HEADER",0,0);
	pr_format(fin,fout,"HEADER1",0,0);

	if (DETAIL)
		pr_format(fin,fout,"HEADER3",0,0);
	else
		pr_format(fin,fout,"HEADER2",0,0);

	pr_format(fin,fout,"SEPARATOR",0,0);
	pr_format(fin,fout,"RULEOFF",0,0);
}

int
check_page (void)
{
	return(0);
}

void
print_header (
 int    first_time,
 char*  sman_cat)
{
	switch (sman_cat[0])
	{
	case	'S':
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",curr_sman);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			strcpy(exsf_rec.sf_salesman,"No Salesman Found");

		expand(err_str,exsf_rec.sf_salesman);

		if (COST_MGN)
			fprintf(fout,".PD| %-2.2s  %-146.146s|\n",curr_sman,err_str);
		else
			fprintf(fout,".PD| %-2.2s  %-104.104s|\n",curr_sman,err_str);
		
		if (!first_time)
			fprintf(fout,".PA\n");

		break;

	case	'C':
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_cat_no,"%-11.11s",curr_cat);
		cc = find_rec(excf, &excf_rec, COMPARISON, "r");
		if (cc)
			strcpy(excf_rec.cf_cat_desc,"No Description Found");
		expand(err_str,excf_rec.cf_cat_desc);

		if (!first_time)
			pr_format(fin,fout,"BLANKLINE",0,0);
		if (COST_MGN)
			fprintf(fout,"| %-11.11s  %-137.137s|\n",curr_cat,err_str);
		else
			fprintf(fout,"| %-11.11s  %-95.95s|\n",curr_cat,err_str);
		break;
	}
}

char*
month_name (
 int    n)
{
	static char *name[] = {
		"JANUARY  ",
		"FEBRUARY ",
		"MARCH    ",
		"APRIL    ",
		"MAY      ",
		"JUNE     ",
		"JULY     ",
		"AUGUST   ",
		"SEPTEMBER",
		"OCTOBER  ",
		"NOVEMBER ",
		"DECEMBER ",
		"*********"
	};
	return((n >= 0 && n <= 11) ? name[n] : name[12]);
}

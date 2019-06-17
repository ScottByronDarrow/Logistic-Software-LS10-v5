/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_dfcstprd.c )                                  |
|  Program Desc  : ( Print Last 12 mnths sales by cust/item          )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sadf,                                 |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 05/06/91         |
|---------------------------------------------------------------------|
|  Date Modified : (24/07/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (23/08/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (06/09/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (06/05/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (06/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (27/05/96)      | Modified  by  : Jiggs Veloz      |
|  Date Modified : (10/09/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (14/10/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (04/11/1997)    | Modified  by  : Jiggs Veloz.     |
|                                                                     |
|  Comments      : (24/07/91) - Added time into heading. Removed      |
|                : breakdown by branch in company report.             |
|                : (23/08/91) - Updated to remove checking of br_no   |
|                :              on sadf.                              |
|  (06/09/91)    : SC 5770 DFH.                                       |
|  (06/05/92)    : Fix cust description on total line. SC 6973 HEL.   |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (06/04/94)    : PSL 10673 - find_hash wrond index                  |
|  (27/05/96)    : Updated to fix problems in DateToString.                |
|  (14/10/97)    : Added ML().                                        |
|  (04/11/1997)  : Changed no_lps() to valid_lp().                    |
|                :                                                    |
|                                                                     |
| $Log: dfcstprd.c,v $
| Revision 5.3  2002/07/17 09:57:46  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:16:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:36  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:27  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
|
| Revision 1.17  2000/01/20 23:24:11  cam
| Changes for GVision compatibility.  Separate description fields from input
| fields.  Remove call to fflush () for the sort file.
|
| Revision 1.16  1999/12/06 01:35:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.15  1999/11/16 04:55:33  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.14  1999/10/20 02:07:02  nz
| Updated for final changes on date routines.
|
| Revision 1.13  1999/10/16 01:11:21  nz
| Updated for pjulmdy routines
|
| Revision 1.12  1999/09/29 10:12:50  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/17 07:27:34  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.10  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/06/18 09:39:21  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dfcstprd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_dfcstprd/dfcstprd.c,v 5.3 2002/07/17 09:57:46 scott Exp $";

#define	MOD	5

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_sa_mess.h>

#define	NO_SRT_FLDS	41

#define	LINE	0
#define	BLNK	1

#define	QTY	0
#define	SAL	1
#define	CST	2

#define	ITEM	0
#define	CAT		1
#define	CUST	2
#define	GND		3

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
	} sadf_rec, wk_sadf_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	int	cumr_no_fields = 7;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
	} cumr_rec, wk_cumr_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
	};

	int inmr_no_fields = 9;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_alpha_code[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
	} inmr_rec, wk_inmr_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	int excf_no_fields = 3;

	struct {
		char	cf_co_no[3];
		char	cf_cat_no[12];
		char	cf_cat_desc[41];
	} excf_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_short_name"},
	};

	int	esmr_no_fields = 3;

	struct	{
		char	esmr_co_no[3];
		char	esmr_est_no[3];
		char	esmr_short_name[16];
	} esmr_rec;

FILE	*fout;
FILE	*fsort;

int	curr_mnth;
int	curr_year;
int	fiscal;
int	printed = FALSE;
int	first_cust = TRUE;
int	first_time = TRUE;
int	BY_CO = FALSE;
int	BY_BR = FALSE;
int	DETAILED = FALSE;
int	SUMMARY = FALSE;
int	data_found;
int	cust_found;

char	*srt_offset[128];
char	br_no[3];
char	prev_br[3];
char	curr_br[3];
char	prev_item[17];
char	curr_item[17];
char	prev_cat[12];
char	curr_cat[12];
char	curr_cust[7];
char	old_cust_name[41];

double	item_tot[3][12];
double	cat_tot[3][12];
double	cust_tot[3][12];
double	grand_tot[3][12];
double	ytd[3][4];
double	lst_ytd[3][4];
double	pft[14];
float	pc_pft[14];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char 	back[2];
	char 	back_desc[4];
	char	onight[2];
	char	onight_desc[4];
	char	lp_str[3];
	int	lpno;
	char	st_cust[7];
	char	st_cust_desc[41];
	char	end_cust[7];
	char	end_cust_desc[41];
	char	rep_by[2];
	char	rep_by_desc[8];
	char	sum_det[2];
	char	sum_det_desc[9];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "rep_by", 4, 14, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Report By:", " ", 
		NO, NO, JUSTLEFT, "BC", "", local_rec.rep_by}, 
	{1, LIN, "rep_by_desc", 4, 17, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_by_desc}, 
	{1, LIN, "st_cust", 6, 14, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Start Customer:", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.st_cust}, 
	{1, LIN, "st_cust_desc", 6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_cust_desc}, 
	{1, LIN, "end_cust", 7, 14, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Customer:", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.end_cust}, 
	{1, LIN, "end_cust_desc", 7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_cust_desc}, 
	{1, LIN, "sum_det", 9, 20, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Summary or Detailed:", "", 
		NO, NO, JUSTLEFT, "SD", "", local_rec.sum_det}, 
	{1, LIN, "sum_det_desc", 9, 23, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.sum_det_desc}, 
	{1, LIN, "lpno", 11, 14, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number:", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 12, 14, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background    :", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "back_desc", 12, 17, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc}, 
	{1, LIN, "onight", 13, 14, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight     :", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{1, LIN, "onight_desc", 13, 17, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int run_prog (char *prog_name, char *prog_desc);
int proc_cust (char *brnch_no);
int read_sadf (char *brnch_no);
int store_data (long int hhbr_hash, char *brnch_no);
int proc_sort (void);
int print_line (int line_type);
int print_tot (char *tot_type);
int get_categ (void);
int print_gr_tot (void);
int calc_profit (int p_type);
int calc_ytd (char *year);
void head_output (void);
int cust_heading (char *brnch_no);
int init_array (void);
char *month_name (int n);
char *_sort_read (FILE *srt_fil);
int spec_valid (int field);
int heading (int scn);

	int		envVarDbCo		=	0;
	char	branchNumber[3];

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc != 2 && argc != 6)
	{
		print_at(0,0,ML(mlSaMess711),argv[0]);
		print_at(1,0,ML(mlSaMess712), argv[0]);
		print_at(2,0,ML(mlSaMess713));
		print_at(3,0,ML(mlSaMess714));
		print_at(4,0,ML(mlSaMess715));
		print_at(5,0,ML(mlSaMess716));
        return (EXIT_FAILURE);
	}

	SETUP_SCR( vars );

	if (argc == 6)
	{
		if (argv[4][0] == 'C')
			BY_CO = TRUE;
		else
			BY_BR = TRUE;

		if (argv[5][0] == 'D')
			DETAILED = TRUE;
		else
			SUMMARY = TRUE;

		local_rec.lpno = atoi(argv[1]);

		sprintf(local_rec.st_cust,"%-6.6s",argv[2]);
		sprintf(local_rec.end_cust,"%-6.6s",argv[3]);
	}
	else
	{
		init_scr();
		set_tty();
		set_masks();
		init_vars(1);
	}

	DateToDMY (comm_rec.tdbt_date, NULL, &curr_mnth, &curr_year);

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;

	OpenDB();


	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	if (argc == 6)
	{
		sprintf(err_str,"Reading : Sales by Customer (%s)", (DETAILED) ? "Detailed" :"Summary");
		dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);

		init_array();
		if (BY_CO)
			strcpy(br_no, "  ");
		else
			strcpy(br_no, comm_rec.test_no);

		head_output();
		proc_cust(br_no);
		print_gr_tot();

		sort_delete(fsort,"sadf");
		fprintf(fout,".EOF\n");
		pclose(fout);
	
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
	{
		prog_exit = FALSE;
		while (!prog_exit)
		{
			/*---------------------
			| Reset control flags |
			---------------------*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;
			init_vars(1);		/*  set default values	*/
	
			/*----------------------------
			| Entry screen 1 linear input |
			----------------------------*/
			heading(1);
			entry(1);
			if (restart || prog_exit)
				continue;
	
			/*----------------------------
			| Edit screen 1 linear input |
			----------------------------*/
			heading(1);
			scn_display(1);
			edit(1);
			if (restart)
				continue;
	
			if (run_prog (argv[0], argv[1]) == 1)
			{
				return (EXIT_SUCCESS);
			}
			prog_exit = TRUE;
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
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
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	abc_alias("wk_sadf", "sadf");
	open_rec("sadf",sadf_list,sadf_no_fields,"sadf_id_no3");
	open_rec("wk_sadf",sadf_list,sadf_no_fields,"sadf_id_no3");

	abc_alias("wk_inmr", "inmr");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec("wk_inmr",inmr_list,inmr_no_fields,"inmr_id_no");

	abc_alias("wk_cumr", "cumr");
	open_rec("cumr",cumr_list,cumr_no_fields,"cumr_id_no3");
	open_rec("wk_cumr",cumr_list,cumr_no_fields,"cumr_id_no3");

	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("sadf");
	abc_fclose("wk_sadf");
	abc_fclose("inmr");
	abc_fclose("wk_inmr");
	abc_fclose("cumr");
	abc_fclose("excf");
	abc_fclose("esmr");
	abc_dbclose("data");
}

int
run_prog (
 char*  prog_name,
 char*  prog_desc)
{
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	
	shutdown_prog ();

	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				local_rec.st_cust,
				local_rec.end_cust,
				local_rec.rep_by,
				local_rec.sum_det,
				prog_desc,(char *)0);
		}
	}
    else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.st_cust,
				local_rec.end_cust,
				local_rec.rep_by,
				local_rec.sum_det, (char *)0);
		}
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.st_cust,
			local_rec.end_cust,
			local_rec.rep_by,
			local_rec.sum_det, (char *)0);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
proc_cust (
 char*  brnch_no)
{
	int	first_time = TRUE;

	fsort = sort_open("sadf");
	
	cust_found = FALSE;

	strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", local_rec.st_cust);
	/*------------------------------------
	| Process all cumr records that fall |
	| into the range selected by the user|
	------------------------------------*/
	cc = find_rec("cumr", &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp(cumr_rec.cm_co_no, comm_rec.tco_no) &&
	       (strcmp(cumr_rec.cm_dbt_no, local_rec.end_cust) <= 0) )
	{
	       	if (strcmp(cumr_rec.cm_est_no, brnch_no) && 
		    strcmp(brnch_no,"  "))
		{
			cc = find_rec("cumr", &cumr_rec, NEXT, "r");
			continue;
		}

		if (first_time)
		{
			sprintf(curr_cust, "%-6.6s", cumr_rec.cm_dbt_no);
			sprintf(old_cust_name, "%-40.40s", cumr_rec.cm_name);
			first_time = FALSE;
		}

		if (strcmp(cumr_rec.cm_dbt_no, curr_cust))
		{
			if (cust_found)
			{
				cust_heading(brnch_no);
				if (!first_cust)
					fprintf(fout, ".PA\n");

				proc_sort();
			}

			sprintf(curr_cust, "%-6.6s", cumr_rec.cm_dbt_no);
			sprintf(old_cust_name, "%-40.40s", cumr_rec.cm_name);

			cust_found = FALSE;
		}

		read_sadf(brnch_no);
			
		cc = find_rec("cumr", &cumr_rec, NEXT, "r");
	}

	if (cust_found)
	{
		cust_heading(brnch_no);
		if (!first_cust)
			fprintf(fout, ".PA\n");

		proc_sort();

		first_cust = FALSE;
	}

	return (EXIT_SUCCESS);
}

int
read_sadf (
 char*  brnch_no)
{
	int	i;
	int	first_time;
	int	lcl_cc;
	long	curr_hhbr = 0L;

	/*---------------------------
	| Read all sadf records for |
	| current hhcu hash and year|
	| equal to "C"		    |
	---------------------------*/
	sadf_rec.df_hhcu_hash = cumr_rec.cm_hhcu_hash;
	sadf_rec.df_hhbr_hash = 0L;
	first_time = TRUE;
	data_found = FALSE;

	lcl_cc = find_rec("sadf", &sadf_rec, GTEQ, "r");
	while (!lcl_cc && (sadf_rec.df_hhcu_hash == cumr_rec.cm_hhcu_hash))
	{
		if ( strcmp(sadf_rec.df_year, "C") )
		{
			lcl_cc = find_rec("sadf", &sadf_rec, NEXT, "r");
			continue;
		}

		if (sadf_rec.df_hhbr_hash != curr_hhbr)
		{
			if (find_hash("inmr", &inmr_rec, COMPARISON, "r", sadf_rec.df_hhbr_hash))
			{
				lcl_cc = find_rec("sadf", &sadf_rec, NEXT, "r");
				continue;
			}
		}

		data_found = TRUE;

		if (first_time)
		{
			curr_hhbr = sadf_rec.df_hhbr_hash;
			first_time = FALSE;
		}

		if (sadf_rec.df_hhbr_hash != curr_hhbr)
		{
			store_data(curr_hhbr, brnch_no);
			curr_hhbr = sadf_rec.df_hhbr_hash;
		}

		/*--------------------------------
		| Add to totals for current hhcu |
		| current hhbr			 |
		--------------------------------*/
		for (i = 0; i < 12; i++)
		{
			item_tot[QTY][i] += sadf_rec.df_qty_per[i];
			item_tot[SAL][i] += sadf_rec.df_sal_per[i];
			item_tot[CST][i] += sadf_rec.df_cst_per[i];
		}
		
		lcl_cc = find_rec("sadf", &sadf_rec, NEXT, "r");
	}

	if (data_found)
		store_data(curr_hhbr, brnch_no);

	dsp_process("Customer :",cumr_rec.cm_dbt_no);

	return (EXIT_SUCCESS);
}

int
store_data (
 long int   hhbr_hash,
 char*      brnch_no)
{
	int	i;
	int	lcl_cc;
	char	data_str[1000];


	lst_ytd[QTY][CUST] = 0.00;
	lst_ytd[SAL][CUST] = 0.00;
	lst_ytd[CST][CUST] = 0.00;

	cc = find_hash("inmr", &inmr_rec, COMPARISON, "r", hhbr_hash);
	if (cc)
		sys_err("Error in inmr during (DBFIND)",cc,PNAME);

	/*---------------------------
	| Read all sadf records for |
	| current hhcu hash and year|
	| equal to "L"		    |
	---------------------------*/
	wk_sadf_rec.df_hhcu_hash = cumr_rec.cm_hhcu_hash;
	wk_sadf_rec.df_hhbr_hash = hhbr_hash;

	lcl_cc = find_rec("wk_sadf", &wk_sadf_rec, GTEQ, "r");
	while (!lcl_cc && 
	       wk_sadf_rec.df_hhcu_hash == cumr_rec.cm_hhcu_hash &&
	       wk_sadf_rec.df_hhbr_hash == hhbr_hash)
	{
		if ( strcmp(wk_sadf_rec.df_year, "L") )
		{
			lcl_cc = find_rec("wk_sadf", &wk_sadf_rec, NEXT, "r");
			continue;
		}

		calc_ytd("L");
		lcl_cc = find_rec("wk_sadf", &wk_sadf_rec, NEXT, "r");
	}

	sprintf(data_str, 
		"%-11.11s%c%-16.16s%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f\n",
		inmr_rec.mr_category, 1,
		inmr_rec.mr_item_no, 1,
		item_tot[QTY][0],   1,
		item_tot[QTY][1],   1,
		item_tot[QTY][2],   1,
		item_tot[QTY][3],   1,
		item_tot[QTY][4],   1,
		item_tot[QTY][5],   1,
		item_tot[QTY][6],   1,
		item_tot[QTY][7],   1,
		item_tot[QTY][8],   1,
		item_tot[QTY][9],   1,
		item_tot[QTY][10],  1,
		item_tot[QTY][11],  1,
		item_tot[SAL][0],   1,
		item_tot[SAL][1],   1,
		item_tot[SAL][2],   1,
		item_tot[SAL][3],   1,
		item_tot[SAL][4],   1,
		item_tot[SAL][5],   1,
		item_tot[SAL][6],   1,
		item_tot[SAL][7],   1,
		item_tot[SAL][8],   1,
		item_tot[SAL][9],   1,
		item_tot[SAL][10],  1,
		item_tot[SAL][11],  1,
		item_tot[CST][0],   1,
		item_tot[CST][1],   1,
		item_tot[CST][2],   1,
		item_tot[CST][3],   1,
		item_tot[CST][4],   1,
		item_tot[CST][5],   1,
		item_tot[CST][6],   1,
		item_tot[CST][7],   1,
		item_tot[CST][8],   1,
		item_tot[CST][9],   1,
		item_tot[CST][10],  1,
		item_tot[CST][11],  1,
		lst_ytd[QTY][ITEM], 1,
		lst_ytd[SAL][ITEM], 1,
		lst_ytd[CST][ITEM]);
	sort_save(fsort, data_str);

	for (i = 0; i < 12; i++)
	{
		item_tot[QTY][i] = 0.00;
		item_tot[SAL][i] = 0.00;
		item_tot[CST][i] = 0.00;
	}

	lst_ytd[QTY][ITEM] = 0.00;
	lst_ytd[SAL][ITEM] = 0.00;
	lst_ytd[CST][ITEM] = 0.00;

	cust_found = TRUE;
	return (TRUE);
}

int
proc_sort (void)
{
	char	*sptr;
	int	i;

	printed = FALSE;
	first_time = TRUE;

	fsort = sort_sort(fsort,"sadf");
	sptr = _sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(curr_item,"%-16.16s",srt_offset[1]);
		sprintf(curr_cat, "%-11.11s", srt_offset[0]);

		if (first_time)
		{
			first_time = FALSE;
			strcpy(prev_item,curr_item);
			strcpy(prev_cat,curr_cat);
		}

		if (strcmp(curr_cat,prev_cat))
			print_tot("CAT");
		else
		{
			if (strcmp(curr_item,prev_item))
				print_tot("ITEM");
		}

		for (i = 0; i < 12; i++)
		{
			item_tot[QTY][i] += atof(srt_offset[i + 2]);
			item_tot[SAL][i] += atof(srt_offset[i + 14]);
			item_tot[CST][i] += atof(srt_offset[i + 26]);
		}

		lst_ytd[QTY][ITEM] += atof(srt_offset[38]);
		lst_ytd[SAL][ITEM] += atof(srt_offset[39]);
		lst_ytd[CST][ITEM] += atof(srt_offset[40]);

		strcpy(prev_item,curr_item);
		strcpy(prev_cat,curr_cat);

		sptr = _sort_read(fsort);
	}

	if (printed)
		print_tot("CUST");

	sort_delete(fsort,"sadf");
	fsort = sort_open("sadf");

	return (EXIT_SUCCESS);
}

int
print_line (
 int    line_type)
{
	if (line_type == LINE)
	{
		fprintf(fout, "!---------------!---------!---------!---------");
		fprintf(fout, "!---------!---------!---------!---------");
		fprintf(fout, "!---------!---------!---------!---------");
		fprintf(fout, "!----------!----------!\n");
	}
	else
	{
		fprintf(fout, "!               !         !         !         ");
		fprintf(fout, "!         !         !         !         ");
		fprintf(fout, "!         !         !         !         ");
		fprintf(fout, "!          !          !\n");
	}
	return (EXIT_SUCCESS);
}

int
print_tot (
 char*  tot_type)
{
	int	i;
	int	j;
	int	lcl_cc;
	char	tmp_desc[81];

	first_cust = FALSE;
	/*------------------
	| Print Item Total |
	------------------*/
	if (!strcmp(tot_type, "ITEM") ||
	    !strcmp(tot_type, "CAT")  ||
	    !strcmp(tot_type, "CUST"))
	{
		calc_ytd("C");

		/*-------------
		| Get Item No |
		-------------*/
		strcpy(wk_inmr_rec.mr_co_no, comm_rec.tco_no);
		sprintf(wk_inmr_rec.mr_item_no, "%-16.16s", prev_item);
		lcl_cc = find_rec("wk_inmr", &wk_inmr_rec, COMPARISON, "r");
		if (lcl_cc)
			sprintf(wk_inmr_rec.mr_description, "%-40.40s", "Unknown Item");

		expand(tmp_desc, wk_inmr_rec.mr_description);
		if (DETAILED)
			fprintf(fout, ".LRP7\n");
		else
			fprintf(fout, ".LRP5\n");

		fprintf(fout, 
			"! %-16.16s  %-80.80s      !         !         !          !          !\n", 
			prev_item, 
			tmp_desc);

		for (j = QTY; j <= CST; j++)
		{
			switch (j)
			{
			case QTY:
				fprintf(fout, "! QTY  ");
				break;
			case SAL:
				fprintf(fout, "! VALUE");
				break;
			case CST:
				fprintf(fout, "! COST ");
				break;
			}

			for (i = curr_mnth; i < 12; i++)
				fprintf(fout, " %8.0f!", item_tot[j][i]);

			for (i = 0; i < curr_mnth; i++)
				fprintf(fout, " %8.0f!", item_tot[j][i]);

			fprintf( fout, 
				" %9.0f! %9.0f!\n",
				ytd[j][ITEM],
				lst_ytd[j][ITEM]);
		}

		calc_profit(ITEM);

		print_line(LINE);

		for (i = 0; i < 12; i++)
		{
			cat_tot[QTY][i] += item_tot[QTY][i];
			cat_tot[SAL][i] += item_tot[SAL][i];
			cat_tot[CST][i] += item_tot[CST][i];
			item_tot[QTY][i] = 0.00;
			item_tot[SAL][i] = 0.00;
			item_tot[CST][i] = 0.00;
		}

		ytd[QTY][CAT] += ytd[QTY][ITEM];
		ytd[SAL][CAT] += ytd[SAL][ITEM];
		ytd[CST][CAT] += ytd[CST][ITEM];

		lst_ytd[QTY][CAT] += lst_ytd[QTY][ITEM];
		lst_ytd[SAL][CAT] += lst_ytd[SAL][ITEM];
		lst_ytd[CST][CAT] += lst_ytd[CST][ITEM];

		lst_ytd[QTY][ITEM] = 0.00;
		lst_ytd[SAL][ITEM] = 0.00;
		lst_ytd[CST][ITEM] = 0.00;
	}

	/*----------------------
	| Print Category Total |
	----------------------*/
	if (!strcmp(tot_type, "CAT") ||
	    !strcmp(tot_type, "CUST"))
	{

		/*-----------------
		| Lookup Category |
		-----------------*/
		get_categ();

		if (DETAILED)
			fprintf(fout, ".LRP8\n");
		else
			fprintf(fout, ".LRP6\n");

		expand(tmp_desc, excf_rec.cf_cat_desc);
		fprintf(fout, 
			"! TOTAL FOR CATEGORY %-11.11s  %-80.80s!           !          !          !\n", 
			prev_cat, 
			tmp_desc);

		for (j = QTY; j <=CST; j++)
		{
			switch (j)
			{
			case QTY:
				fprintf(fout, "! QTY  ");
				break;
			case SAL:
				fprintf(fout, "! VALUE");
				break;
			case CST:
				fprintf(fout, "! COST ");
				break;
			}

			for (i = curr_mnth; i < 12; i++)
				fprintf(fout, " %8.0f!", cat_tot[j][i]);

			for (i = 0; i < curr_mnth; i++)
				fprintf(fout, " %8.0f!", cat_tot[j][i]);

			fprintf( fout, 
				" %9.0f! %9.0f!\n",
				ytd[j][CAT],
				lst_ytd[j][CAT]);
		}

		calc_profit(CAT);

		print_line(BLNK);
		print_line(LINE);

		for (i = 0; i < 12; i++)
		{
			cust_tot[QTY][i] += cat_tot[QTY][i];
			cust_tot[SAL][i] += cat_tot[SAL][i];
			cust_tot[CST][i] += cat_tot[CST][i];

			cat_tot[QTY][i] = 0.00;
			cat_tot[SAL][i] = 0.00;
			cat_tot[CST][i] = 0.00;
		}

		ytd[QTY][CUST] += ytd[QTY][CAT];
		ytd[SAL][CUST] += ytd[SAL][CAT];
		ytd[CST][CUST] += ytd[CST][CAT];

		lst_ytd[QTY][CUST] += lst_ytd[QTY][CAT];
		lst_ytd[SAL][CUST] += lst_ytd[SAL][CAT];
		lst_ytd[CST][CUST] += lst_ytd[CST][CAT];

		ytd[QTY][CAT] = 0.00;
		ytd[SAL][CAT] = 0.00;
		ytd[CST][CAT] = 0.00;

		lst_ytd[QTY][CAT] = 0.00;
		lst_ytd[SAL][CAT] = 0.00;
		lst_ytd[CST][CAT] = 0.00;
	}

	/*----------------------
	| Print Customer Total |
	----------------------*/
	if (!strcmp(tot_type, "CUST"))
	{
		if (DETAILED)
			fprintf(fout, ".LRP7\n");
		else
			fprintf(fout, ".LRP5\n");

		expand(tmp_desc, old_cust_name);
		fprintf(fout, 
			"! TOTAL FOR CUSTOMER %-11.11s  %-80.80s!           !          !          !\n", 
			curr_cust, 
			tmp_desc);

		for (j = QTY; j <=CST; j++)
		{
			switch (j)
			{
			case QTY:
				fprintf(fout, "! QTY  ");
				break;
			case SAL:
				fprintf(fout, "! VALUE");
				break;
			case CST:
				fprintf(fout, "! COST ");
				break;
			}

			for (i = curr_mnth; i < 12; i++)
				fprintf(fout, " %8.0f!", cust_tot[j][i]);

			for (i = 0; i < curr_mnth; i++)
				fprintf(fout, " %8.0f!", cust_tot[j][i]);

			fprintf( fout, 
				" %9.0f! %9.0f!\n",
				ytd[j][CUST],
				lst_ytd[j][CUST]);
		}

		calc_profit(CUST);

		print_line(LINE);

		for (i = 0; i < 12; i++)
		{
			grand_tot[QTY][i] += cust_tot[QTY][i];
			grand_tot[SAL][i] += cust_tot[SAL][i];
			grand_tot[CST][i] += cust_tot[CST][i];

			cust_tot[QTY][i] = 0.00;
			cust_tot[SAL][i] = 0.00;
			cust_tot[CST][i] = 0.00;
		}

		ytd[QTY][GND] += ytd[QTY][CUST];
		ytd[SAL][GND] += ytd[SAL][CUST];
		ytd[CST][GND] += ytd[CST][CUST];

		lst_ytd[QTY][GND] += lst_ytd[QTY][CUST];
		lst_ytd[SAL][GND] += lst_ytd[SAL][CUST];
		lst_ytd[CST][GND] += lst_ytd[CST][CUST];

		ytd[QTY][CUST] = 0.00;
		ytd[SAL][CUST] = 0.00;
		ytd[CST][CUST] = 0.00;

		lst_ytd[QTY][CUST] = 0.00;
		lst_ytd[SAL][CUST] = 0.00;
		lst_ytd[CST][CUST] = 0.00;
	}
	return(0);
}

int
get_categ (void)
{
	int	lcl_cc;

	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no, "%-11.11s", prev_cat);
	lcl_cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (lcl_cc)
		sprintf(excf_rec.cf_cat_desc, "%-40.40s", "Unknown Category");

	return(0);
}

int
print_gr_tot (void)
{
	int	i;
	int	j;

	/*-------------------
	| Print Grand Total |
	-------------------*/
	if (DETAILED)
		fprintf(fout, ".LRP7\n");
	else
		fprintf(fout, ".LRP5\n");

	fprintf(fout, "! GRAND TOTAL%-135.135s!\n", " ");

	for (j = QTY; j <= CST; j++)
	{
		switch (j)
		{
		case QTY:
			fprintf(fout, "! QTY  ");
			break;
		case SAL:
			fprintf(fout, "! VALUE");
			break;
		case CST:
			fprintf(fout, "! COST ");
			break;
		}

		for (i = curr_mnth; i < 12; i++)
			fprintf(fout, "%9.0f!", grand_tot[j][i]);

		for (i = 0; i < curr_mnth; i++)
			fprintf(fout, "%9.0f!", grand_tot[j][i]);

		fprintf( fout, 
			"%10.0f!%10.0f!\n",
			ytd[j][GND],
			lst_ytd[j][GND]);
	}
	calc_profit(GND);

	print_line(LINE);
	return(0);
}

/*----------------------------
| Calculate and print profit |
| information if detailed    |
| report.                    |
----------------------------*/
int
calc_profit (
 int    p_type)
{
	int	i;

	if (SUMMARY)
		return(0);

	for (i = 0; i < 12; i++)
	{
		switch(p_type)
		{
		case ITEM:
			pft[i] = item_tot[SAL][i] - item_tot[CST][i];
			if (item_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / item_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;

		case CAT:
			pft[i] = cat_tot[SAL][i] - cat_tot[CST][i];
			if (cat_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / cat_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;

		case CUST:
			pft[i] = cust_tot[SAL][i] - cust_tot[CST][i];
			if (cust_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / cust_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;

		case GND:
			pft[i] = grand_tot[SAL][i] - grand_tot[CST][i];
			if (grand_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / grand_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;
		}
	}

	pft[12] = ytd[SAL][p_type] - ytd[CST][p_type];
	if (ytd[SAL][p_type] != 0.00)
		pc_pft[12] = (pft[12] / ytd[SAL][p_type] * 100);
	else
		pc_pft[12] = -99.00;

	pft[13] = lst_ytd[SAL][p_type] - lst_ytd[CST][p_type];
	if (lst_ytd[SAL][p_type] != 0.00)
		pc_pft[13] = (pft[13] / lst_ytd[SAL][p_type] * 100);
	else
		pc_pft[13] = -99.00;

	/*--------------------
	| Print profit value |
	--------------------*/
	fprintf(fout, "! PFT  ");
	for (i = curr_mnth; i < 12; i++)
		fprintf(fout, " %8.0f!", pft[i]);

	for (i = 0; i < curr_mnth; i++)
		fprintf(fout, " %8.0f!", pft[i]);

	fprintf( fout, 
		" %9.0f! %9.0f!\n",
		pft[12],
		pft[13]);

	/*----------------------
	| Print profit percent |
	----------------------*/
	fprintf(fout, "! %% PFT");
	for (i = curr_mnth; i < 12; i++)
		fprintf(fout, " %8.2f!", pc_pft[i]);

	for (i = 0; i < curr_mnth; i++)
		fprintf(fout, " %8.2f!", pc_pft[i]);

	fprintf( fout, 
		" %9.2f! %9.2f!\n",
		pc_pft[12],
		pc_pft[13]);	

	return(0);
}

int
calc_ytd (
 char*  year)
{
	int	i;

	if (year[0] == 'C')
	{
		ytd[QTY][ITEM] = 0.00;
		ytd[SAL][ITEM] = 0.00;
		ytd[CST][ITEM] = 0.00;
	}

	if (curr_mnth <= fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][ITEM] += item_tot[QTY][i];
				ytd[SAL][ITEM] += item_tot[SAL][i];
				ytd[CST][ITEM] += item_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][ITEM] += wk_sadf_rec.df_qty_per[i];
				lst_ytd[SAL][ITEM] += wk_sadf_rec.df_sal_per[i];
				lst_ytd[CST][ITEM] += wk_sadf_rec.df_cst_per[i];
			}
		}

		for (i = 0; i < curr_mnth; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][ITEM] += item_tot[QTY][i];
				ytd[SAL][ITEM] += item_tot[SAL][i];
				ytd[CST][ITEM] += item_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][ITEM] += wk_sadf_rec.df_qty_per[i];
				lst_ytd[SAL][ITEM] += wk_sadf_rec.df_sal_per[i];
				lst_ytd[CST][ITEM] += wk_sadf_rec.df_cst_per[i];
			}
		}

	}
	else
	{
		for (i = fiscal; i < curr_mnth; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][ITEM] += item_tot[QTY][i];
				ytd[SAL][ITEM] += item_tot[SAL][i];
				ytd[CST][ITEM] += item_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][ITEM] += wk_sadf_rec.df_qty_per[i];
				lst_ytd[SAL][ITEM] += wk_sadf_rec.df_sal_per[i];
				lst_ytd[CST][ITEM] += wk_sadf_rec.df_cst_per[i];
			}
		}
	}

	return (EXIT_SUCCESS);
}

void
head_output (void)
{
	char	*month_name(int n);
	int	i;

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".10\n");
	fprintf(fout,".L158\n");
	
	fprintf(fout,
		".E%s Sales Analysis By Customer For Last 12 Months\n",
		(DETAILED) ? "Detailed" : "Summary");

	fprintf(fout,
		".ECOMPANY : %s - %s\n",
		comm_rec.tco_no,
		clip(comm_rec.tco_name));

	if (BY_CO)
		fprintf(fout,".EALL BRANCHES\n");
	else
	{
		strcpy(esmr_rec.esmr_co_no, comm_rec.tco_no);
		strcpy(esmr_rec.esmr_est_no, comm_rec.test_no);
		cc = find_rec("esmr", &esmr_rec, COMPARISON, "r");
		if (cc)
			sys_err("Error in esmr during (DBFIND)",cc,PNAME);

		fprintf(fout,
			".EBRANCH : %-2.2s - %-15.15s\n",
			comm_rec.test_no,
			comm_rec.test_name);
	}
		
	fprintf(fout,".EFOR THE MONTH OF %-9.9s\n",month_name(curr_mnth - 1));

	fprintf(fout,".B1\n");
	sprintf(err_str, 
		"For Customers  %-6.6s to %-6.6s", 
		local_rec.st_cust, 
		local_rec.end_cust);

	fprintf(fout,".E%s\n",err_str);

	fprintf(fout,"==================================================");
	fprintf(fout,"==================================================");
	fprintf(fout,"=================================================\n");

	fprintf(fout,"!      ");
	for (i = curr_mnth; i < 12; i ++)
		fprintf(fout, "%-3.3s  %4d!", month_name(i), curr_year - 1);
	for (i = 0; i < curr_mnth - 1; i ++)
		fprintf(fout, "%-3.3s  %4d!", month_name(i), curr_year);

	fprintf(fout, "%-3.3s  %4d!   YTD    !   LYTD   !\n", month_name(curr_mnth - 1), curr_year);

	fprintf(fout,"!---------------!---------!---------!---------");
	fprintf(fout,"!---------!---------!---------!---------");
	fprintf(fout,"!---------!---------!---------!---------");
	fprintf(fout,"!----------!----------!\n");

	fprintf(fout,".R=================================================");
	fprintf(fout,"===================================================");
	fprintf(fout,"=================================================\n");

}

int
cust_heading (
 char*  brnch_no)
{
	char	tmp_desc[81];

	expand(tmp_desc, old_cust_name);
	fprintf(fout, 
		".PD! %-6.6s  %-80.80s      !         !         !         !          !          !\n", 
		curr_cust, 
		tmp_desc);

	return(0);
}

int
init_array (void)
{
	int	i;
	int	j;

	for (i = 0; i < 12; i++)
	{
		item_tot[QTY][i] = 0.00;
		item_tot[SAL][i] = 0.00;
		item_tot[CST][i] = 0.00;

		cust_tot[QTY][i] = 0.00;
		cust_tot[SAL][i] = 0.00;
		cust_tot[CST][i] = 0.00;

		grand_tot[QTY][i] = 0.00;
		grand_tot[SAL][i] = 0.00;
		grand_tot[CST][i] = 0.00;
	}
	
	for (j = ITEM; j < GND; j++)
	{
		ytd[QTY][j] = 0.00;
		ytd[SAL][j] = 0.00;
		ytd[CST][j] = 0.00;
		lst_ytd[QTY][j] = 0.00;
		lst_ytd[SAL][j] = 0.00;
		lst_ytd[CST][j] = 0.00;
	}

	return (EXIT_SUCCESS);
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

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_sort_read (
 FILE*  srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset[0] = sptr;

	tptr = sptr;
	while (fld_no < NO_SRT_FLDS)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;
		srt_offset[fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}

int
spec_valid (
 int    field)
{
	if (LCHECK("rep_by"))
	{
		if (local_rec.rep_by[0] == 'C')
		{
			strcpy(local_rec.rep_by_desc, "Company");
		}
		else
		{
			strcpy(local_rec.rep_by_desc, "Branch ");
		}

		DSP_FLD("rep_by_desc");

		return(0);
	}

	if (LCHECK("sum_det"))
	{
		if (local_rec.sum_det[0] == 'D')
		{
			strcpy(local_rec.sum_det_desc, "Detailed");
		}
		else
		{
			strcpy(local_rec.sum_det_desc, "Summary ");
		}

		DSP_FLD("sum_det_desc");

		return(0);
	}

	if (LCHECK("st_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_cust, "      ");
			sprintf(local_rec.st_cust_desc, "%-40.40s", "First Customer");
			DSP_FLD("st_cust");
			DSP_FLD("st_cust_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		strcpy(wk_cumr_rec.cm_co_no, comm_rec.tco_no);
		sprintf(wk_cumr_rec.cm_dbt_no, "%-6.6s", pad_num(temp_str));
		cc = find_rec("wk_cumr", &wk_cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.st_cust, "%-6.6s",wk_cumr_rec.cm_dbt_no);
			sprintf(local_rec.st_cust_desc, "%-40.40s", wk_cumr_rec.cm_name);
			DSP_FLD("st_cust");
			DSP_FLD("st_cust_desc");
			return(0);
		}
	}

	if (LCHECK("end_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_cust, "~~~~~~");
			sprintf(local_rec.end_cust_desc, "%-40.40s", "Last Customer");
			DSP_FLD("end_cust");
			DSP_FLD("end_cust_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		strcpy(wk_cumr_rec.cm_co_no, comm_rec.tco_no);
		sprintf(wk_cumr_rec.cm_dbt_no, "%-6.6s", pad_num(temp_str));
		cc = find_rec("wk_cumr", &wk_cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Customer Not Found On File ");*/
			print_mess(ML(mlStdMess021));
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.end_cust,"%-6.6s",wk_cumr_rec.cm_dbt_no);
			sprintf(local_rec.end_cust_desc, "%-40.40s", wk_cumr_rec.cm_name);
			DSP_FLD("end_cust");
			DSP_FLD("end_cust_desc");
			return(0);
		}
	}

	if (strcmp(FIELD.label,"lpno") == 0)
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if ( !valid_lp(local_rec.lpno) )
		{
			print_mess(ML(mlStdMess020));
			return(1);
		}

		return(0);
	}

	if (strcmp(FIELD.label,"back") == 0)
	{
		strcpy(local_rec.back_desc,(local_rec.back[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("back_desc");
		return(0);
	}

	if (strcmp(FIELD.label,"onight") == 0)
	{
		strcpy(local_rec.onight_desc,(local_rec.onight[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("onight_desc");
		return(0);
	}
	return(0);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlSaMess005),15,0,1);

		move(0,1);
		line(80);

		box(0,3,80,10);
		move(1,5);
		line(79);

		move(1,8);
		line(79);

		move(1,10);
		line(79);

		move(0,20);
		line(80);
		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no, comm_rec.test_name );
		strcpy(err_str, ML(mlStdMess039));
		print_at(21,45, err_str, comm_rec.test_no,comm_rec.test_name);
		move(0,22);
		line(80);
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

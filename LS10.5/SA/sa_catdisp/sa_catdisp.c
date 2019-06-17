/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_catdisp.c )                                   |
|  Program Desc  : ( Display Product Sales By Category.              )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, excf, sale                                  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : 06/12/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : (24/04/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (10/09/97)      | Modified  by : Marnie I Organo.  |
|                                                                     |
|  Comments      : Modified version of sa_psbycust.c                  |
|                : Modified to use new screen generator.              |
|                : (24/04/91) - Updated for new Dsp_                  |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (09/10/97)    : Modified for Multilingual Conversion.              |
|                :                                                    |
|                                                                     |
| $Log: sa_catdisp.c,v $
| Revision 5.2  2001/08/09 09:16:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:13  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:22  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:35  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:20  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:35:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 04:55:31  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.11  1999/10/16 01:11:20  nz
| Updated for pjulmdy routines
|
| Revision 1.10  1999/10/13 01:34:02  cam
| Fix for GVision compatibility
|
| Revision 1.9  1999/09/29 10:12:44  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:30  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 02:01:51  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:19  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_catdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_catdisp/sa_catdisp.c,v 5.2 2001/08/09 09:16:47 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sa_mess.h>

#define	MTH_TO_DATE	1
#define	YR_TO_DATE	2
#undef  PSIZE
#define	PSIZE		17
#define HEADER		Dsp_saverec("  CATEGORY   |            DESCRIPTION              |             MONTH TO DATE            |              YEAR TO DATE            ")
#define HEADER1		Dsp_saverec("             |                                     |    VALUE     |% MARGIN |   MARGIN    |    VALUE     |% MARGIN |    MARGIN   ")
#define UNDERLINE 	Dsp_saverec(" ================================================================================================================================ ")
#define CHEADER		Dsp_saverec("  CUST NO.   |  CUST ACRONYM   |     CATEGORY      |             MONTH TO DATE            |              YEAR TO DATE            ")
#define CHEADER1	Dsp_saverec("             |                 |                   |    VALUE     |% MARGIN |   MARGIN    |    VALUE     |% MARGIN |    MARGIN   ")

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 8;

	struct {
		int	term;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_short[16];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

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
		char	sl_key[9];
		char	sl_category[12];
		char	sl_sman[3];
		char	sl_area[3];
		char	sl_ctype[4];
		char	sl_dbt_no[7];
		char	sl_year_flag[2];
		char	sl_period[3];
		double	sl_units;
		double	sl_gross;		/*  Money field  */
		double	sl_cost_sale;		/*  Money field  */
		double	sl_disc;		/*  Money field  */
	} sale_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	int cumr_no_fields = 6;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
	} cumr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
	};

	int esmr_no_fields = 4;

	struct {
		char	em_co_no[3];
		char	em_est_no[3];
		char	em_est_name[41];
		char	em_short_name[16];
	} esmr_rec;

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
		char	cf_categ_no[12];
		char	cf_categ_description[41];
	} excf_rec;

char	dbt_date[11];
char	prev_cat[12];
char	curr_cat[12];
char	prev_per[3];
char	curr_per[3];
char	prev_cust[7];
char	curr_cust[7];
char	branchNo[3];

int	line_num = 1;
int	found_data = 0;
int	first_time = 1;
int	by_category = 1;
int	printed = 0;
int	start_mth = 0;
int	end_mth = 0;
int	envDbCo = 0;
int	envDbFind = 0;
int	fiscal = 0;

double	prev_gross = 0.0;
double	prev_disc = 0.0;
double	prev_csale = 0.0;
double	curr_gross = 0.0;
double	curr_disc = 0.0;
double	curr_csale = 0.0;
double	detail_mgross = 0.0;
double	detail_mdisc = 0.0;
double	detail_mcsale = 0.0;
double	detail_ygross = 0.0;
double	detail_ydisc = 0.0;
double	detail_ycsale = 0.0;
double	total_mgross = 0.0;
double	total_mdisc = 0.0;
double	total_mcsale = 0.0;
double	total_ygross = 0.0;
double	total_ydisc = 0.0;
double	total_ycsale = 0.0;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	value[12];
	char	desc[41];
	char	br_no[4];
	char	br_name[41];
	char	det_summ[9];
} local_rec;

char	*cust_label	= "cust_no";
char	*cust_mask	= "UUUUUU";
char	*cust_prompt	= "Customer No ";

static	struct	var	vars[]	={	

	{1, LIN, "br_no", 4, 20, CHARTYPE, 
		"UU", "          ", 
		" ", "ALL", "Branch No ", " Default is ALL ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no}, 
	{1, LIN, "br_name", 4, 60, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.br_name}, 
	{1, LIN, "cat_no", 5, 20, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "ALL        ", "Category No ", " Default is ALL ", 
		YES, NO, JUSTLEFT, "", "", local_rec.value}, 
	{1, LIN, "desc", 5, 60, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.desc}, 
	{1, LIN, "det_summ", 6, 20, CHARTYPE, 
		"U", "          ", 
		" ", "D(etail ", "Report Type ", " D(etail or S(ummary ", 
		YES, NO, JUSTLEFT, "", "", local_rec.det_summ}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void set_custlabel (void);
void set_dflts (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void show_esmr (char *key_val);
void show_excf (char *key_val);
int heading (int scn);
void init_output (void);
void proc_data (void);
void process_data (void);
int check_range (char *rec_period);
void add_total (int mth_yr, double gross, double csale, double disc);
void print_detail (void);
void print_total (void);
int check_page (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc != 2)
	{
		/*printf("Usage : %s <(C)ustomer | C(A)tegory> \n",argv[0]);*/
		print_at(0,0,ML(mlSaMess710),argv[0]);
        return (EXIT_SUCCESS);
	}

	by_category = !strcmp(argv[1],"A");

	if (!by_category)
		set_custlabel();

	SETUP_SCR( vars );

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr();
	set_tty();
	set_masks();
               
	abc_dbopen("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));
	
	strcpy(branchNo, ( envDbCo ) ? comm_rec.test_no : " 0" );

	swide();
	clear();

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;

	OpenDB();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		search_ok = 1;
		init_vars(1);
		crsr_on();
		set_dflts();

		/*-------------------------------
	        | Entry screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		scn_display(1);
		entry(1);

		if (restart || prog_exit)
			continue;

		/*------------------------------
	        | Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		clear();
		crsr_off();
		fflush(stdout);

		init_output();
		proc_data();
	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
set_custlabel (void)
{
	vars[2].label	= cust_label;
	vars[2].mask	= cust_mask;
	vars[2].prmpt	= cust_prompt;
}

void
set_dflts (void)
{
	strcpy(local_rec.value,"ALL        ");
	if (by_category)
		sprintf(local_rec.desc,"%-40.40s","All Caterories");
	else
		sprintf(local_rec.desc,"%-40.40s","All Customers");

	strcpy(local_rec.br_no,"ALL");
	sprintf(local_rec.br_name,"%-40.40s","All Branches");
	strcpy(local_rec.det_summ,"D(etail ");
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
	open_rec("sale",sale_list,sale_no_fields,(by_category) ? "sale_category"
							       : "sale_id_no_2");
	open_rec("cumr",cumr_list,cumr_no_fields,(envDbFind ) ? "cumr_id_no3" 
							    : "cumr_id_no");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("esmr");
	abc_fclose("excf");
	abc_fclose("cumr");
	abc_fclose("sale");
	abc_dbclose("data");
}

int
spec_valid(int field)
{
        if ( LCHECK( "br_no" ) )
        {
		if (last_char == SEARCH)
		{
                       show_esmr(temp_str);
		       return(0);
		}

		if ( !strcmp(local_rec.br_no,"ALL") )
		{
			sprintf(local_rec.br_name,"%-40.40s","All Branches");
			display_field(label("br_name"));
			strcpy(comm_rec.test_no,"  ");
			strcpy(branchNo,"  ");
			return(0);
		}
			
		strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
		sprintf(esmr_rec.em_est_no,"%2.2s",local_rec.br_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");	
		if (cc)
		{
		/*	sprintf(err_str,"Branch No %s Is Not on File ",local_rec.br_no);*/
			print_mess(ML(mlStdMess073));
			sleep(3);
			return(1);
		}
		strcpy(comm_rec.test_no,local_rec.br_no);
		strcpy(branchNo,local_rec.br_no);
		sprintf(local_rec.br_no,"%2.2s ",local_rec.br_no);
		strcpy(local_rec.br_name,esmr_rec.em_est_name);
		display_field(label("br_no"));
		display_field(label("br_name"));
                return(0);
	}

        if ( LCHECK("cat_no") )
        {
		if (last_char == SEARCH)
		{
                       show_excf(temp_str);
		       return(0);
		}

		if ( !strcmp(local_rec.value,"ALL        "))
		{
			sprintf(local_rec.desc,"%-40.40s","All Categories");
			display_field(label("desc"));
			return(0);
		}
			
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_categ_no,"%-11.11s",local_rec.value);
		cc = find_rec("excf",&excf_rec,COMPARISON,"r");	
		if (cc)
		{
			/*sprintf(err_str,"Category %s Is Not on File ",local_rec.value);*/
			print_mess(ML(mlStdMess004));
			sleep(3);
			return(1);
		}
		strcpy(local_rec.desc,excf_rec.cf_categ_description);
		display_field(label("desc"));
                return(0);
	}

        if ( LCHECK("cust_no") )
        {
			if (last_char == SEARCH)
			{
				CumrSearch (comm_rec.tco_no, branchNo, temp_str);
				return(0);
			}

			if ( !strcmp(local_rec.value,"ALL        ") )
			{
				sprintf(local_rec.desc,"%-40.40s","All Customers");
				display_field(label("desc"));
				return(0);
			}

		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNo);
		sprintf(cumr_rec.cm_dbt_no,"%-6.6s",pad_num(local_rec.value));
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");	

		if (cc)
		{
		/*	sprintf(err_str,"Customer %s Is Not on File ",local_rec.value);*/
			print_mess(ML(mlStdMess021));
			sleep(3);
			return(1);
		}
		strcpy(local_rec.desc,cumr_rec.cm_name);
		display_field(label("desc"));
                return(0);
	}

        if ( LCHECK("det_summ") )
        {
		strcpy(local_rec.det_summ,(local_rec.det_summ[0] == 'D') ? "D(etail " : "S(ummary");
		display_field(label("det_summ"));
		return(0);
	}

	return(0);
}

void
show_esmr (
 char*  key_val)
{
        work_open();
	save_rec("#Br No.    ","#Br Name           ");
	strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
	sprintf(esmr_rec.em_est_no,"%2.2s",key_val);
	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");
        while (!cc && !strcmp(esmr_rec.em_co_no,comm_rec.tco_no) && 
		      !strncmp(esmr_rec.em_est_no,key_val,strlen(key_val)))
    	{
		cc = save_rec(esmr_rec.em_est_no,esmr_rec.em_est_name);
		if (cc)
			break;
		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
	sprintf(esmr_rec.em_est_no,"%2.2s",temp_str);
	cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in esmr During (DBFIND)",cc,PNAME);
}

void
show_excf (
 char*  key_val)
{
        work_open();
	save_rec("#Category No.    ","#Category Description    ");
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no,"%-11.11s",key_val);
	cc = find_rec("excf",&excf_rec,GTEQ,"r");
        while (!cc && !strcmp(excf_rec.cf_co_no,comm_rec.tco_no) && 
		      !strncmp(excf_rec.cf_categ_no,key_val,strlen(key_val)))
    	{
		cc = save_rec( excf_rec.cf_categ_no,
			       excf_rec.cf_categ_description);
		if (cc)
			break;
		cc = find_rec("excf",&excf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no,"%-11.11s",temp_str);
	cc = find_rec("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in excf During (DBFIND)",cc,PNAME);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		swide();
		clear();

		if (by_category)
			rv_pr(ML(mlSaMess001),47,0,1);
			/*rv_pr(" Display Sales Analysis By Category ",47,0,1);*/
		else
			rv_pr(ML(mlSaMess002),47,0,1);
			/*rv_pr(" Display Sales Analysis By Customer/Category ",42,0,1);*/
		
		move(0,1);
		line(132);

		box(0,3,132,3);

		move(0,20);
		line(132);
		print_at(21,0,ML(mlStdMess038),
				comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(132);
		/* Reset this variable for new screen NOT page */
		line_cnt = 0; 
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

void
init_output (void)
{
	/*--------------------------------------------
	| Heading and Page Format for Screen Output  |
	| Display.				     |
	--------------------------------------------*/
	Dsp_open(0,1,PSIZE);
	if (by_category)
	{
		/*rv_pr(" SALES ANALYSIS BY CATEGORY REPORT ",47,0,1);*/
		rv_pr(ML(mlSaMess003),47,0,1);
		HEADER;
		HEADER1;
	}
	else
	{
		/*rv_pr(" SALES ANALYSIS BY CUSTOMER/CATEGORY REPORT ",42,0,1);*/
		rv_pr(ML(mlSaMess004),47,0,1);
		CHEADER;
		CHEADER1;
	}

	Dsp_saverec("                            [FN03] [FN14] [FN15] [FN16]                                      ");  
}

/*=====================
| Process information |
=====================*/
void
proc_data (void)
{
	char	valid = FALSE;
	char	co[3];
	char	rec_br[3];
	int	ret_val = 0;

	first_time = 1;
	line_num = 1;
	printed = 0;
	prev_gross = 0.0;
	prev_csale = 0.0;
	prev_disc = 0.0;
	curr_gross = 0.0;
	curr_csale = 0.0;
	curr_disc = 0.0;
	detail_mgross = 0.0;
	detail_mcsale = 0.0;
	detail_mdisc = 0.0;
	detail_ygross = 0.0;
	detail_ycsale = 0.0;
	detail_ydisc = 0.0;
	total_mgross = 0.0;
	total_mcsale = 0.0;
	total_mdisc = 0.0;
	total_ygross = 0.0;
	total_ycsale = 0.0;
	total_ydisc = 0.0;

	if (by_category)
	{
		if ( !strcmp(local_rec.value,"ALL        "))
			strcpy(sale_rec.sl_category,"           ");
		else
			sprintf(sale_rec.sl_category,"%-11.11s",local_rec.value);
	}
	else
	{
		if ( !strcmp(local_rec.value,"ALL        "))
			strcpy(sale_rec.sl_dbt_no,"      ");
		else
			sprintf(sale_rec.sl_dbt_no,"%-6.6s",local_rec.value);
		strcpy(sale_rec.sl_category,"           ");
	}

	cc = find_rec("sale",&sale_rec,GTEQ,"r");

	sprintf(co,"%2.2s",sale_rec.sl_key);
	sprintf(rec_br,"%2.2s",sale_rec.sl_key+2);
	while (!cc)
	{
	    if (!strcmp(co,comm_rec.tco_no))
	    {
		valid = FALSE;

		/*-----------------------------------------------
		| If display / report is to be produced by all	|
		| category or display / report is for a		|
		| specific category and this is that		|
		| category.					|
		-----------------------------------------------*/
		if (by_category)
		{
			if ( !strcmp(sale_rec.sl_category,local_rec.value) || 
			     !strcmp(local_rec.value,"ALL        "))
			{
				if (strcmp(sale_rec.sl_category,local_rec.value)				 && strcmp(local_rec.value,"ALL        ") )
					break;

				/*-----------------------------------------
				| If for ALL branches or the right branch |
				| then store data else don't		  |
				------------------------------------------*/
				if ( !strcmp(rec_br,local_rec.br_no) || 
				     !strcmp(local_rec.br_no,"ALL"))
					valid = TRUE;
			}
		}
		else
		{
			if ( !strncmp(sale_rec.sl_dbt_no,local_rec.value,6) || 
			     !strcmp(local_rec.value,"ALL        ") )
			{
				if (strncmp(sale_rec.sl_dbt_no,local_rec.value,6) && strcmp(local_rec.value,"ALL        ") != 0)
					break;
				/*------------------------------------
				| If for ALL branches or the right   |
				| branch,then store data else don't  |
				-------------------------------------*/
				if ( !strcmp(rec_br,local_rec.br_no) || 
				     !strcmp(local_rec.br_no,"ALL"))
					valid = TRUE;
			}
		}

		/*-----------------------------------------------
		| sale record is a valid record to process	|
		| for the branch_no & category 	keyed in the 	|
		| input part of the program.			|
		-----------------------------------------------*/
		ret_val = check_range(sale_rec.sl_period);
		if (ret_val != 0 && valid && sale_rec.sl_year_flag[0] == 'C')
			process_data();
	    }
	    cc = find_rec("sale",&sale_rec,NEXT,"r");

	    sprintf(co,"%2.2s",sale_rec.sl_key);
	    sprintf(rec_br,"%2.2s",sale_rec.sl_key+2);
	}
	if (!printed)
	{
		Dsp_saverec("      ");
		Dsp_saverec("      ");
		if (by_category)
			Dsp_saverec("                            No Category in given data span.          ");
		else
			Dsp_saverec("                            No Customer in given data span.          ");
		Dsp_saverec("      ");
		Dsp_saverec("                            Press [FN16] key to continue             ");
	}
	else
	{
		/*------------------------------
		| Print the last rec and total |
		------------------------------*/
		ret_val = check_range(curr_per);
		add_total(ret_val,curr_gross,curr_csale,curr_disc);
		if (local_rec.det_summ[0] == 'D')
			print_detail();
			
		UNDERLINE;
		line_num++;
		print_total();
		UNDERLINE;
		line_num++;
	}
	Dsp_srch();
	Dsp_close();
}

void
process_data (void)
{
	int	ret_val = 0;

	printed = 1;
	strcpy(curr_cat,sale_rec.sl_category);
	strcpy(curr_cust,sale_rec.sl_dbt_no);
	strcpy(curr_cust,sale_rec.sl_dbt_no);
	strcpy(curr_per,sale_rec.sl_period);
	curr_gross = DOLLARS(sale_rec.sl_gross);	
	curr_csale = DOLLARS(sale_rec.sl_cost_sale);	
	curr_disc = DOLLARS(sale_rec.sl_disc);	

	if (first_time)
	{
		strcpy(prev_cat,curr_cat);
		strcpy(prev_cust,curr_cust);
		strcpy(prev_per,curr_per);
		prev_gross = curr_gross;
		prev_csale = curr_csale;
		prev_disc = curr_disc;
	}

	if (line_num >= PSIZE)
		line_num = line_num % PSIZE;

	if (!first_time)
	{
		ret_val = check_range(prev_per);

		add_total(ret_val,prev_gross,prev_csale,prev_disc);
	}

	if (local_rec.det_summ[0] == 'D')
	{
		if (by_category)
		{
			if (!first_time && strcmp(prev_cat,curr_cat) != 0)
			{
				print_detail();
				detail_mgross = 0.0;
				detail_mcsale = 0.0;
				detail_mdisc = 0.0;
				detail_ygross = 0.0;
				detail_ycsale = 0.0;
				detail_ydisc = 0.0;
			}
		}
		else
		{
			if (!first_time)
			{
			 	if (( !strcmp(prev_cust,curr_cust) && 
				      strcmp(prev_cat,curr_cat)) || 
				      strcmp(prev_cust,curr_cust))
				{
					print_detail();
					detail_mgross = 0.0;
					detail_mcsale = 0.0;
					detail_mdisc = 0.0;
					detail_ygross = 0.0;
					detail_ycsale = 0.0;
					detail_ydisc = 0.0;
				}
			}
		}
	}

	first_time = 0;
	strcpy(prev_cat,curr_cat);
	strcpy(prev_cust,curr_cust);
	strcpy(prev_cust,curr_cust);
	strcpy(prev_per,curr_per);
	prev_gross = curr_gross;
	prev_disc = curr_disc;
	prev_csale = curr_csale;
}

int
check_range (
 char*  rec_period)
{

	int	i = 0;
	int	ok = FALSE;
	int	ret_val = 0;
	int	period = 0;
	int	sale_per = 0;

	sale_per = atoi(rec_period);

	start_mth = fiscal + 1;
	DateToDMY (comm_rec.tdbt_date, NULL, &end_mth, NULL);

	if (end_mth < start_mth)
		end_mth += 12;

	for (i = start_mth; i <= end_mth && ok == FALSE; i++)
	{
		period = i % 12;
		if (period == 0)
			period = 12;
		
		if (sale_per == period)
		{
			ok = TRUE;
			if (sale_per == end_mth)
				ret_val = MTH_TO_DATE;
			else
				ret_val = YR_TO_DATE;
		}
	}

	return (ret_val);
}

void
add_total (
 int    mth_yr,
 double gross,
 double csale,
 double disc)
{
	if (mth_yr == MTH_TO_DATE)
	{
		detail_mgross += gross;
		detail_mcsale += csale;
		detail_mdisc += disc;
		total_mgross += gross; 
		total_mcsale += csale; 
		total_mdisc += disc; 
	}

	if (mth_yr != 0)
	{
		detail_ygross += gross;
		detail_ydisc += disc;
		detail_ycsale += csale;
		total_ygross += gross; 
		total_ycsale += csale; 
		total_ydisc += disc; 
	}
}

void
print_detail (void)
{
	char	env_line[132];
	char	cat_desc[36];
	char	cust_acro[17];
	double	mmarg_pc = 0.0;
	double	mmarg_val = 0.0;
	double	mvalue = 0.0;
	double	ymarg_pc = 0.0;
	double	ymarg_val = 0.0;
	double	yvalue = 0.0;

	if (by_category)
	{
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_categ_no,"%-11.11s",prev_cat);
		cc = find_rec("excf",&excf_rec,COMPARISON,"r");	

		if (cc)
			strcpy(cat_desc,"Unknown category");
		else
			sprintf(cat_desc,"%-35.35s",excf_rec.cf_categ_description);
	}

	mvalue      = detail_mgross - detail_mdisc;
	if (mvalue < -99999999999.0)
		mvalue = -99999999999.0;

	mmarg_val = mvalue - detail_mcsale;
	if (mmarg_val < -9999999999.0)
		mmarg_val = -9999999999.0;

	if (mvalue > 0)
	{
		mmarg_pc  = mmarg_val / mvalue * 100;
		if (mmarg_pc > 9999.99)
			mmarg_pc = 9999.99;

		if (mmarg_pc < -999.99)
			mmarg_pc = -999.99;
	}
	else
		mmarg_pc  = 0.0;
		
	yvalue      = detail_ygross - detail_ydisc;
	if (yvalue < -99999999999.0)
		yvalue = -99999999999.0;

	ymarg_val = yvalue - detail_ycsale;
	if (ymarg_val < -9999999999.0)
		ymarg_val = -9999999999.0;

	if (yvalue > 0)
	{
		ymarg_pc  = ymarg_val / yvalue * 100;
		if (ymarg_pc > 9999.99)
			ymarg_pc = 9999.99;

		if (ymarg_pc < -999.99)
			ymarg_pc = -999.99;
	}
	else
		ymarg_pc  = 0.0;

	if (by_category)
		sprintf(env_line," %-11.11s | %-35.35s | %12.0f | %7.2f | %11.0f | %12.0f | %7.2f | %11.0f  ",
				prev_cat,
				cat_desc,
				mvalue,
				mmarg_pc,
				mmarg_val,
				yvalue,
				ymarg_pc,
				ymarg_val);
	else
	{
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		sprintf(cumr_rec.cm_est_no,"%2.2s",sale_rec.sl_key+2);
		strcpy(cumr_rec.cm_dbt_no,prev_cust);
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");	
		if (cc)
			strcpy(cust_acro,"Unknown Customer");
		else
			sprintf(cust_acro,"%9.9s",cumr_rec.cm_acronym);
		
		sprintf(env_line," %-6.6s      | %-16.16s| %-11.11s       | %12.0f | %7.2f | %11.0f | %12.0f | %7.2f | %11.0f  ",
				prev_cust,
				cust_acro,
				prev_cat,
				mvalue,
				mmarg_pc,
				mmarg_val,
				yvalue,
				ymarg_pc,
				ymarg_val);
	}

	Dsp_saverec(env_line);
	line_num++;
}

void
print_total (void)
{
	char	env_line[132];
	char	name[16];
	char	string[23];
	double	mmarg_pc = 0.0;
	double	mmarg_val = 0.0;
	double	mvalue = 0.0;
	double	ymarg_pc = 0.0;
	double	ymarg_val = 0.0;
	double	yvalue = 0.0;

	if ( !strcmp(local_rec.br_no,"ALL") )
	{
		/*-------------------------
		| Print Company Total     |
		-------------------------*/
		sprintf(string,"TOTAL FOR COMPANY %-2.2s -",comm_rec.tco_no);
		sprintf(name,"%-15.15s",comm_rec.tco_short);
	}
	else
	{
		/*------------------------
		| Print Branch Total     |
		------------------------*/
		strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
		sprintf(esmr_rec.em_est_no,"%2.2s",local_rec.br_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");	
		if (cc)
		{
			/*sprintf(err_str," Branch %s Not on File ",local_rec.br_no);*/
			print_mess(ML(mlStdMess073));
			return ;
		}
		sprintf(string,"TOTAL FOR BRANCH  %-2.2s -",esmr_rec.em_est_no);
		sprintf(name,"%-15.15s",esmr_rec.em_short_name);
	}
		
	mvalue      = total_mgross - total_mdisc;
	if (mvalue < -99999999999.0)
		mvalue = -99999999999.0;

	mmarg_val = mvalue - total_mcsale;
	if (mmarg_val < -9999999999.0)
		mmarg_val = -9999999999.0;

	if (mvalue > 0)
	{
		mmarg_pc  = mmarg_val / mvalue * 100;
		if (mmarg_pc > 9999.99)
			mmarg_pc = 9999.99;

		if (mmarg_pc < -999.99)
			mmarg_pc = -999.99;
	}
	else
		mmarg_pc  = 0.0;

	yvalue      = total_ygross - total_ydisc;
	if (yvalue < -99999999999.0)
		yvalue = -99999999999.0;

	ymarg_val = yvalue - total_ycsale;
	if (ymarg_val < -9999999999.0)
		ymarg_val = -9999999999.0;

	if (yvalue > 0)
	{
		ymarg_pc  = ymarg_val / yvalue * 100;
		if (ymarg_pc > 9999.99)
			ymarg_pc = 9999.99;

		if (ymarg_pc < -999.99)
			ymarg_pc = -999.99;
	}
	else
		ymarg_pc  = 0.0;
	sprintf(env_line,"  %-22.22s %-15.15s           | %12.0f | %7.2f | %11.0f | %12.0f | %7.2f | %11.0f  ",
			string,
			name,
			mvalue,
			mmarg_pc,
			mmarg_val,
			yvalue,
			ymarg_pc,
			ymarg_val);
	Dsp_saverec(env_line);
	line_num++;
}

int
check_page (void)
{
	return(0);
}


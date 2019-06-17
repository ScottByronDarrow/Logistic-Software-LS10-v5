/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_sadf_prt.c )                                  |
|  Program Desc  : ( Print Item Sales By Customer.                   )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, cumr, sadf                            |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 26/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : 28/05/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 22/07/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 24/07/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 08/08/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 12/12/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 12/02/92        | Modified  by  : Campbell Mander. |
|  Date Modified : 16/03/92        | Modified  by  : Campbell Mander. |
|  Date Modified : 13/05/93        | Modified  by  : Scott B Darrow.  |
|  Date Modified : 27/10/94        | Modified  by  : Lawrence Barnes. |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (27/05/96)      | Modified  by : JIggs A Veloz.    |
|  Date Modified : (16/09/97)      | Modified  by : Rowena S Maandig  |
|                                                                     |
|  Comments      :                                                    |
|    (28/05/91)  : Separated into two programs.                       |
|                : Program now uses sadf file as opposed to sapc.     |
|                :                                                    |
|    (22/07/91)  : Fixed ytd calculation.                             |
|                :                                                    |
|    (24/07/91)  : Added time into heading. Changed company report so |
|                : that break down by branch does not now appear.     |
|                :                                                    |
|    (08/08/91)  : Changed for new indices (id_no3 & 4)               |
|                :                                                    |
|    (12/12/91)  : Added optional category selection for item by cust |
|                : report.                                            |
|                :                                                    |
|    (12/02/92)  : Print module date after month name in heading.     |
|                : Date at top right of heading is now system date.   |
|                :                                                    |
|    (16/03/92)  : Fixed All branches option. SC 6656 DPL.            |
|                :                                                    |
|    (13/05/93)  : Fixed S/C DFT-8866 ( We hope. )                    |
|                :                                                    |
|    (27/10/94)  : FCS 11398 Fixed problem of same customer appearing |
|                :           on first line of every page.             |
|                :                                                    |
|    (01/09/95)  : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|                :                                                    |
|    (27/05/96)  : Updated to fix problems in DateToString. 				  |
|    (16/09/97)  : Updated to incorporate multilingual conversion.    |
|                                                                     |
| $Log: sadf_prt.c,v $
| Revision 5.3  2001/08/28 08:46:29  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 09:17:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:51  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:31  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.21  2000/04/25 09:01:19  marnie
| SC#2811 - LSANZ16251 - Modified to correct the printing of Item Sales by Customer Report.
|
| Revision 1.20  2000/02/18 02:35:25  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.19  1999/12/06 01:35:31  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.18  1999/11/16 04:55:34  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.17  1999/10/16 01:11:21  nz
| Updated for pjulmdy routines
|
| Revision 1.16  1999/09/29 10:12:51  scott
| Updated to be consistant on function names.
|
| Revision 1.15  1999/09/17 07:27:36  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.14  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.13  1999/06/18 09:39:22  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sadf_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf_prt/sadf_prt.c,v 5.3 2001/08/28 08:46:29 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<pr_format3.h>
#include	<ml_std_mess.h>
#include	<ml_sa_mess.h>

#undef      PSIZE
#define		PSIZE	14
#define		CATG_SEL	( sel_type[0] == 'C' )
#define		RESET_LINE_NUM	ln_num = (ln_num >= PSIZE) ? ln_num % PSIZE : ln_num

#define		REP_CUST	( cust_type[0] == 'C' )
#define		REP_TYPE	( cust_type[0] == 'T' )
#define		REP_SMAN	( cust_type[0] == 'S' )

#define		BY_CUST		( cust_prod[0] == 'C' )		
#define		DETAIL		( sum_det[0] == 'D' )		
#define		BY_CAT		( type[0] == 'A'  || type[0] == 'I'	)		
#define		BY_SMAN		( type[0] == 'S' )		
#define		COST_MGN	( costmgn[0] == 'Y' )		

char		*HEADER = "                |                                        | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*AHEADER = " CATEGORY NO /  |         CATEGORY DESCRIPTION /         | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*THEADER = "  CUST TYPE /   |        CUST TYPE DESCRIPTION /         | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*SHEADER = " SALESMAN NO /  |            SALESMAN NAME /             | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*ITEM_HEAD = "ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*ITEM_HEAD1 = "  ITEM NUMBER   |         ITEM     DESCRIPTION           |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*CUST_HEAD = "CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*CUST_HEAD1 = "    CUSTOMER    |            CUSTOMER     NAME           |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*SEPARATION = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char		*UNDERLINE = "===============================================================================================================================";

char		*XHEADER =    "                |                                        |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XAHEADER =   " CATEGORY NO /  |         CATEGORY DESCRIPTION /         |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XTHEADER =   "  CUST TYPE /   |        CUST TYPE DESCRIPTION /         |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XSHEADER =   " SALESMAN NO /  |            SALESMAN NAME /             |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XITEM_HEAD = "ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |   QTY  | SALES  |   QTY  | SALES  ";
char		*XITEM_HEAD1 ="  ITEM NUMBER   |         ITEM     DESCRIPTION           |   QTY  | SALES  |   QTY  | SALES  ";
char		*XCUST_HEAD = "CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |   QTY  | SALES  |   QTY  | SALES  ";
char		*XCUST_HEAD1 ="    CUSTOMER    |            CUSTOMER     NAME           |   QTY  | SALES  |   QTY  | SALES  ";
char		*XSEPARATION = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char		*XUNDERLINE = "=============================================================================================";

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
	} sadf_rec;

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
		{"cumr_class_type"}
	};

	int cumr_no_fields = 7;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
	} cumr_rec;

	/*============================================
	| Alias Of Customer Master File Base Record. |
	============================================*/
	struct dbview wkmr_list[] ={
		{"cumr_hhcu_hash"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
	};

	int wkmr_no_fields = 3;

	struct {
		long	wk_hhcu_hash;
		char	wk_dbt_no[7];
		char	wk_name[41];
	} wkmr_rec;

	/*=================================
	| Customer Class Type Master File. |
	=================================*/
	struct dbview excl_list[] ={
		{"excl_co_no"},
		{"excl_class_type"},
		{"excl_class_desc"},
	};

	int excl_no_fields = 3;

	struct {
		char	cl_co_no[3];
		char	cl_class_type[4];
		char	cl_class_desc[41];
	} excl_rec;

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
	} inmr_rec;

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

	FILE	*fin,
		    *fout,
    		*fsort;

	char	dsp_str[200],
		cust_type[2],
		sel_type[2],
		s_class[2],
		e_class[2],
		s_cat[12],
		e_cat[12],
		s_type[4],
		e_type[4],
		s_man[3],
		e_man[3],
		s_cust[7],
		e_cust[7],
		lower[17],
		upper[17],
		s_group[13],
		e_group[13];
	
	int	ln_num = 0,
		column = 0,
		fiscal,
		envDbCo = 0,
		envDbFind = 0;

	char	sort_str[150],
		data_str[150],
		data_str1[150],
		branchNo[3],
		br_no[3],
		curr_sman[3],
		prev_br[3],
		curr_br[3],
		prev_cat[12],
		curr_cat[12],
		prev_prod[17],
		curr_prod[17],
		prev_item_cat[28],
		curr_item_cat[28],
		prev_dbt[7],
		curr_dbt[7],
		prev_type[4],
		curr_type[4],
		cust_prod[2],
		prt_dsp[2],
		sum_det[2],
		type[2],
		costmgn[2];

	int	found_data = 0,
		first_time = TRUE,
		first_cat = TRUE,
		printed = FALSE,
		srt_all = FALSE,
		end_all = FALSE,
		lpno = 1,
		ptr_offset[8],
		curr_mnth,
		BY_CO;

	int	READ_MASTER = FALSE;

	long	prev_hash[2];
	long	curr_hash[2];

	float	m_qty[5];
	float	y_qty[5];
	double	m_sales[5];
	double	m_csale[5];
	double	y_sales[5];
	double	y_csale[5];

	struct {
		char	*heading;
		char	*by_what;
	} head_list[6] = {
		{"Customer"                  , "By Item   "},
		{"Cust Type"                 , "By Customer"},
		{"Salesman"                  , "By Customer"},
		{"Category"                  , "By Item   "},
		{"Item  "                    , "By Customer"},
		{"Item (Category Selection) ", "By Customer"},
	};

/*=====================================================================
| Local Functions Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void proc_cumr (void);
int valid_cust (void);
void process_cust (void);
void proc_inmr (void);
int valid_item (void);
void process_prod (void);
void store_pdata (void);
void store_cdata (void);
void process_data (void);
char* _sort_read (FILE *srt_fil);
void set_break (char *_sort_str, char *sptr);
int check_break (void);
void cust_header (void);
void type_header (void);
void salesman_header (void);
void category_header (void);
void item_header (void);
int check_page (void);
void proc_data (int first_time, int print_type);
void sum_sales (char *data_line, int add);
void print_line (void);
void draw_line (void);
void print_total (char *tot_type);
void init_output (void);
void init_array (void);
void print_header (int first_time, char *head_type, int prt_head);
char *month_name (int n);
int calc_mtd (void);
int calc_ytd (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	int	REPORT_BY = 0;
	char	*sptr;

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	strcpy(s_class,"A");
	strcpy(e_class,"Z");
	strcpy(s_cat,"           ");
	strcpy(e_cat,"~~~~~~~~~~~");
	strcpy(s_type,"   ");
	strcpy(e_type,"~~~");
	strcpy(s_man,"  ");
	strcpy(e_man,"~~");
	strcpy(s_cust,"      ");
	strcpy(e_cust,"~~~~~~");
	strcpy(lower,"                ");
	strcpy(upper,"~~~~~~~~~~~~~~~~");

	if (argc != 10)
	{
		/*printf("Usage : %s <by_what>\n", argv[0]);
		printf("           <Report_type>\n");
		printf("           <type>\n");
		printf("           <selection [I/C]>\n");
		printf("           <lower>\n");
		printf("           <upper>\n");
		printf("           <lpno - if Printing>\n");
		printf("           <Br_no>\n");
		printf("           <Print Cost-Margin>\n");
		*/
		print_at(0,0,ML(mlSaMess728), argv[0]);
		print_at(1,0,ML(mlSaMess729));
		print_at(2,0,ML(mlSaMess730));
		print_at(3,0,ML(mlSaMess731));
		print_at(4,0,ML(mlSaMess732));
		print_at(5,0,ML(mlSaMess733));
		print_at(6,0,ML(mlSaMess734));
		print_at(7,0,ML(mlSaMess735));
		print_at(8,0,ML(mlSaMess736));
		return (EXIT_FAILURE);
	}

	sprintf( cust_prod, "%1.1s",  argv[1]);
	sprintf( sum_det,   "%1.1s",  argv[2]);
	sprintf( type,      "%1.1s",  argv[3]);
	sprintf( sel_type,  "%1.1s",  argv[4]);
	sprintf( costmgn,   "%-1.1s", argv[9]);

	sptr = chk_env((BY_CUST) ? "SA_BYCUS" : "SA_BYPRD");
	READ_MASTER = ( sptr == (char *)0 ) ? FALSE : atoi(sptr);

	if (BY_CUST)
	{
		switch (type[0])
		{
		case	'C' :
			/*--------------------
			| Sales By Customer. |
			--------------------*/
			REPORT_BY = 0;
			strcpy(cust_type,"C");
			sprintf(s_cust,"%-6.6s",argv[5]);
			sprintf(e_cust,"%-6.6s",argv[6]);
			if (!strcmp(s_cust,"      "))
				srt_all = TRUE;

			if (!strcmp(e_cust,"~~~~~~"))
				end_all = TRUE;
			break;

		case	'T' :
			/*-------------------------
			| Sales By Customer Type. |
			-------------------------*/
			REPORT_BY = 1;
			strcpy(cust_type,"T");
			sprintf(s_type,"%-3.3s",argv[5]);
			sprintf(e_type,"%-3.3s",argv[6]);
			if (!strcmp(s_type,"   "))
				srt_all = TRUE;

			if (!strcmp(e_type,"~~~"))
				end_all = TRUE;
			break;

		case	'S' :
			/*--------------------
			| Sales By Salesman. |
			--------------------*/
			REPORT_BY = 2;
			strcpy(cust_type,"S");
			sprintf(s_man,"%-2.2s",argv[5]);
			sprintf(e_man,"%-2.2s",argv[6]);
			if (!strcmp(s_man,"  "))
				srt_all = TRUE;

			if (!strcmp(e_man,"~~"))
				end_all = TRUE;
			break;
		}
	}
	else 
	{
		if ( BY_CAT || (!BY_CAT && CATG_SEL) )
		{
			if (BY_CAT)
				REPORT_BY = 3;
			else
				REPORT_BY = 5;

			sprintf(s_group, "%-12.12s", argv[5]);
			sprintf(e_group, "%-12.12s", argv[6]);
			sprintf(s_class, "%1.1s",    s_group);
			sprintf(s_cat,   "%-11.11s", s_group + 1);
			sprintf(e_class, "%1.1s",    e_group);
			sprintf(e_cat,   "%-11.11s", e_group + 1);
			if (!strcmp(s_cat,"           "))
				srt_all = TRUE;

			if (!strcmp(e_cat,"~~~~~~~~~~~"))
				end_all = TRUE;
		}
		else	/* By Item */ 
		{
			REPORT_BY = 4;
			sprintf(lower,"%-16.16s",argv[5]);
			sprintf(upper,"%-16.16s",argv[6]);
			if (!strcmp(lower,"                "))
				srt_all = TRUE;

			if (!strcmp(upper,"~~~~~~~~~~~~~~~~"))
				end_all = TRUE;
		}
	}

	lpno = atoi(argv[7]);

	if (!strncmp(argv[8],"All",3) || !strncmp(argv[8],"ALL",3))
	{
		BY_CO = TRUE;
		strcpy(br_no,"  ");
	}
	else
	{
		BY_CO = FALSE;
		sprintf(br_no,"%-2.2s",argv[8]);
	}

	OpenDB();

	DateToDMY (comm_rec.tdbt_date, NULL, &curr_mnth, NULL);

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;


	if ((fin = pr_open("sa_sadf.p")) == NULL)
		sys_err("Error in opening sa_sadf.p during (FOPEN)",errno,PNAME);

	sprintf(err_str,"Processing : %s Sales %s %s",
			head_list[ REPORT_BY ].heading,
			( !REPORT_BY ) ? " " : head_list[ REPORT_BY ].by_what,
			(DETAIL) ? "(Detailed)" : "(Summary)");

	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
  
	if (BY_CUST)
	{
		if ( envDbCo && envDbFind )
			READ_MASTER = TRUE;

		if (READ_MASTER)
			proc_cumr();
		else
			process_cust();
	}
	else
	{
		if (READ_MASTER)
			proc_inmr();
		else
			process_prod();
	}


	init_output();

	/*---------------------------
	| Process data in sort file |
	---------------------------*/
	process_data();

	fprintf(fout,".EOF\n");
	pclose(fout);

	shutdown_prog();
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
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sadf",sadf_list,sadf_no_fields,( BY_CUST ) ? "sadf_id_no2" 
							     : "sadf_id_no");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec("cumr",cumr_list,cumr_no_fields,"cumr_hhcu_hash");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	if (BY_CUST)
	{
		if (REP_TYPE)
			open_rec("excl",excl_list,excl_no_fields,"excl_id_no");

		if (REP_SMAN)
			open_rec("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
	}
	else
		open_rec("excf",excf_list,excf_no_fields,"excf_id_no");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("sadf");
	if (BY_CUST)
	{
		if ( REP_TYPE )
			abc_fclose("excl");

		if ( REP_SMAN )
			abc_fclose("exsf");
		
		if ( REP_CUST )
			abc_fclose("cumr");
	}
	else
		abc_fclose("excf");

	abc_fclose("inmr");
	abc_fclose("cumr");
	abc_fclose("esmr");
	abc_dbclose("data");
}

/*=======================================================
| Read data into sort file,then process the information |
| Read whole cumr file.                                 |
=======================================================*/
void
proc_cumr (void)
{
	char	save_br[3];

	abc_selfield("cumr", "cumr_id_no");
	abc_selfield("sadf", "sadf_id_no3");

	fsort = sort_open("sale");

	strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
	strcpy(cumr_rec.cm_est_no,br_no);
	if (!strcmp (br_no, "  "))
	{
		strcpy(cumr_rec.cm_dbt_no,"      ");
		cc = find_rec ( "cumr" , &cumr_rec, GTEQ, "r");
		if (cc)
			return;
	}

	if ( REP_CUST )
		strcpy(cumr_rec.cm_dbt_no,s_cust);
	else
		strcpy(cumr_rec.cm_dbt_no,"      ");

	cc = find_rec("cumr",&cumr_rec,GTEQ,"r");

	while (!cc && !strcmp(cumr_rec.cm_co_no,comm_rec.tco_no))
	{
		if ( strcmp(cumr_rec.cm_est_no,br_no) && strcmp(br_no,"  ") )
			break;
		
		if ( !valid_cust() )
		{
			if ( REP_CUST )
			{
			    if ( strcmp( br_no, "  " ) )
				break;

			    strcpy( cumr_rec.cm_dbt_no, "~~~~~~" );
			    cc = find_rec("cumr",&cumr_rec,GTEQ,"r");
			    if (!cc)
			    {
				strcpy (save_br, cumr_rec.cm_est_no);
				strcpy(cumr_rec.cm_dbt_no,s_cust);
				cc = find_rec("cumr",&cumr_rec,GTEQ,"r");
				if (!cc && strcmp (cumr_rec.cm_est_no, save_br))
					{
					    cc = find_rec ("cumr", &cumr_rec, LT, "r");
					    cc = find_rec ("cumr", &cumr_rec, GTEQ, "r");
					}
			    }
			    continue;
			}
			else
			{
				cc = find_rec("cumr",&cumr_rec,NEXT,"r");
				continue;
			}
		}
		sadf_rec.df_hhcu_hash = cumr_rec.cm_hhcu_hash;
		sadf_rec.df_hhbr_hash = 0L;
		cc = find_rec("sadf",&sadf_rec,GTEQ,"r");
		while (!cc && sadf_rec.df_hhcu_hash == cumr_rec.cm_hhcu_hash)
		{
			if (sadf_rec.df_year[0] != 'C')
			{
				cc = find_rec("sadf",&sadf_rec,NEXT,"r");
				continue;
			}

			if ( BY_SMAN )
			{
				if ( strcmp(sadf_rec.df_sman,s_man) < 0 || 
				     strcmp(sadf_rec.df_sman,e_man) > 0 )
				{
					cc = find_rec("sadf",&sadf_rec,NEXT,"r");
					continue;
				}
			}
			calc_mtd();
			calc_ytd();

			/*-----------------------------------------------
			| Store if sales & cost of sales <> 0.00	|
			-----------------------------------------------*/
			if ( m_qty[0]   != 0 ||
			     m_sales[0] != 0 ||
			     m_csale[0] != 0 ||
			     y_qty[0]   != 0 ||
			     y_sales[0] != 0 ||
			     y_csale[0] != 0  )
			{
					store_cdata();
			}

			cc = find_rec("sadf",&sadf_rec,NEXT,"r");
		}
		cc = find_rec("cumr",&cumr_rec,NEXT,"r");
	}
	abc_selfield("cumr", "cumr_hhcu_hash");
}

int
valid_cust (void)
{
	int	store_ok = TRUE;

	switch (type[0])
	{
	case	'C':
		if ( strcmp(cumr_rec.cm_dbt_no,s_cust) < 0 || 
		     strcmp(cumr_rec.cm_dbt_no,e_cust) > 0)
			store_ok = FALSE;
		break;

	case	'T':
		if ( strcmp(cumr_rec.cm_class_type,s_type) < 0 || 
		     strcmp(cumr_rec.cm_class_type,e_type) > 0)
			store_ok = FALSE;
		break;

	}
	return(store_ok);
}

/*=======================================================
| Read data into sort file,then process the information |
| Read whole sadf file.                                 |
=======================================================*/
void
process_cust (void)
{
	char	rec_br[3];

	fsort = sort_open("sale");

	strcpy(sadf_rec.df_co_no,comm_rec.tco_no);
	strcpy(sadf_rec.df_br_no,br_no);
	strcpy(sadf_rec.df_year, "C");
	strcpy(rec_br,br_no);
	sadf_rec.df_hhcu_hash = 0L;
	sadf_rec.df_hhbr_hash = 0L;
	strcpy(sadf_rec.df_sman,"  ");

	cc = find_rec("sadf",&sadf_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp(sadf_rec.df_co_no,comm_rec.tco_no) && 
	       (!strcmp(sadf_rec.df_br_no,br_no) || !strcmp(rec_br,"  ")))
	{
	       	if ( sadf_rec.df_year[0] != 'C' )
		{
			cc = find_rec("sadf",&sadf_rec,NEXT,"r");
			continue;
		}

		if ( BY_SMAN && ( strcmp(sadf_rec.df_sman,s_man) < 0 || 
				  strcmp(sadf_rec.df_sman,e_man) > 0))
		{
			cc = find_rec("sadf",&sadf_rec,NEXT,"r");
			continue;
		}
		cc = find_hash("cumr",&cumr_rec,EQUAL,"r",
							sadf_rec.df_hhcu_hash);
		if (cc)
		{
			sprintf(cumr_rec.cm_name,"%-40.40s","Unknown Customer");
			sprintf(cumr_rec.cm_dbt_no,"%-6.6s","DELETE");
			sprintf(cumr_rec.cm_class_type,"%-3.3s","DEL");
		}

		calc_mtd();
		calc_ytd();

		/*-----------------------------------------------
		| Store if sales & cost of sales <> 0.00	|
		-----------------------------------------------*/
		if (valid_cust() && (m_qty[0] != 0 ||
		     m_sales[0]  != 0 ||
		     m_csale[0]  != 0 ||
		     y_qty[0]    != 0 ||
		     y_sales[0]  != 0 ||
		     y_csale[0]  != 0) )
				store_cdata();

		cc = find_rec("sadf",&sadf_rec,NEXT,"r");
	}
}

/*=======================================================
| Read data into sort file,then process the information |
| Read whole inmr file.                                 |
=======================================================*/
void
proc_inmr (void)
{
	char	rec_br[3];

	if ( BY_CAT || (!BY_CAT && CATG_SEL) )
		abc_selfield("inmr","inmr_id_no_3");
	else
		abc_selfield("inmr","inmr_id_no");

	abc_selfield("sadf", "sadf_id_no4");

	fsort = sort_open("sale");

	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	strcpy(rec_br, br_no);
	if ( BY_CAT || (!BY_CAT && CATG_SEL) )
	{
		strcpy(inmr_rec.mr_class,s_class);
		strcpy(inmr_rec.mr_category,s_cat);
		sprintf(inmr_rec.mr_item_no,"%16.16s"," ");
	}
	else
	{
		strcpy(inmr_rec.mr_class," ");
		sprintf(inmr_rec.mr_category,"%11.11s"," ");
		strcpy(inmr_rec.mr_item_no,lower);
	}
	cc = find_rec("inmr",&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp(inmr_rec.mr_co_no,comm_rec.tco_no))
	{
		if ( !valid_item() )
		{
			if (!BY_CAT && !CATG_SEL)
				break;

			cc = find_rec("inmr",&inmr_rec,NEXT,"r");
			continue;
		}
			
		sadf_rec.df_hhbr_hash = inmr_rec.mr_hhbr_hash;
		sadf_rec.df_hhcu_hash = 0L;
		cc = find_rec("sadf",&sadf_rec,GTEQ,"r");
		while (!cc && sadf_rec.df_hhbr_hash == inmr_rec.mr_hhbr_hash)
		{
			if (strcmp(sadf_rec.df_year, "C"))
			{
				cc = find_rec("sadf",&sadf_rec,NEXT,"r");
				continue;
			}

			if (strcmp(sadf_rec.df_co_no,comm_rec.tco_no) || 
			    (strcmp(sadf_rec.df_br_no,br_no) 
			    && strcmp(rec_br,"  ")))
			{
				cc = find_rec("sadf",&sadf_rec,NEXT,"r");
				continue;
			}

			calc_mtd();
			calc_ytd();

			/*-----------------------------------------------
			| Store if sales & cost of sales <> 0.00	|
			-----------------------------------------------*/
			if ( m_qty[0]   != 0 ||
			     m_sales[0] != 0 ||
			     m_csale[0] != 0 ||
			     y_qty[0]   != 0 ||
			     y_sales[0] != 0 ||
			     y_csale[0] != 0  )
					store_pdata();

			cc = find_rec("sadf",&sadf_rec,NEXT,"r");
		}
		cc = find_rec("inmr",&inmr_rec,NEXT,"r");
	}
	abc_selfield("inmr", "inmr_hhbr_hash");
}

int
valid_item (void)
{
	char	item_gp[13];
	int	store_ok = TRUE;

	switch (type[0])
	{
	case	'I':
	case	'C':
		if (CATG_SEL)
		{
			sprintf(item_gp,"%-1.1s%-11.11s", inmr_rec.mr_class,
						  inmr_rec.mr_category);

			if (strcmp(item_gp,s_group) < 0 || 
			    strcmp(item_gp,e_group) > 0)
				store_ok = FALSE;
		}
		else
		{
			if ( strcmp(inmr_rec.mr_item_no,lower) < 0 || 
		     	     strcmp(inmr_rec.mr_item_no,upper) > 0)
				store_ok = FALSE;
		}
		
		break;

	case	'A':
		sprintf(item_gp,"%-1.1s%-11.11s", inmr_rec.mr_class,
						  inmr_rec.mr_category);

		if (strcmp(item_gp,s_group) < 0 || strcmp(item_gp,e_group) > 0)
			store_ok = FALSE;
		break;

	}
	return(store_ok);
}

/*=======================================================
| Read data into sort file,then process the information |
| Read whole sadf file.                                 |
=======================================================*/
void
process_prod (void)
{
	char	rec_br[3];

	fsort = sort_open("sale");

	strcpy(sadf_rec.df_co_no,comm_rec.tco_no);
	strcpy(sadf_rec.df_br_no,br_no);
	strcpy(sadf_rec.df_year,"C");
	strcpy(rec_br,br_no);
	sadf_rec.df_hhbr_hash = 0L;
	sadf_rec.df_hhcu_hash = 0L;
	strcpy(sadf_rec.df_sman,"  ");

	cc = find_rec("sadf",&sadf_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp(sadf_rec.df_co_no,comm_rec.tco_no) && 
	       (!strcmp(sadf_rec.df_br_no,br_no) || !strcmp(rec_br,"  ")))
	{
	       	if (strcmp(sadf_rec.df_year, "C"))
		{
			cc = find_rec("sadf",&sadf_rec,NEXT,"r");
			continue;
		}

		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",sadf_rec.df_hhbr_hash);
		if (cc)
		{
			strcpy(inmr_rec.mr_item_no, "DELETED ITEM    ");
			sprintf(inmr_rec.mr_description, "%-40.40s","DELETED ITEM");
			sprintf(inmr_rec.mr_category, "%-11.11s", "DELETED    ");
		}

		calc_mtd();
		calc_ytd();

		/*-----------------------------------------------
		| Store if sales & cost of sales <> 0.00	|
		-----------------------------------------------*/
		if (valid_item() && (m_qty[0] != 0 ||
		     m_sales[0] != 0 ||
		     m_csale[0] != 0 ||
		     y_qty[0]   != 0 ||
		     y_sales[0] != 0 ||
		     y_csale[0] != 0) )
				store_pdata();

		cc = find_rec("sadf",&sadf_rec,NEXT,"r");
	}
}

/*---------------------------------------
| Store product based data in sort file |
---------------------------------------*/
void
store_pdata (void)
{
	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",sadf_rec.df_hhcu_hash);
	if (cc)
	{
		sprintf(cumr_rec.cm_name,"%-40.40s","Unknown Customer");
		sprintf(cumr_rec.cm_dbt_no,"%-6.6s","DELETE");
		sprintf(cumr_rec.cm_class_type,"%-3.3s","DEL");
	}

	dsp_process("Reading Cust No : ",cumr_rec.cm_dbt_no);

	sprintf (data_str,"%-2.2s%-11.11s%-16.16s%-6.6s",
		(BY_CO) ? "  " : sadf_rec.df_br_no,
		(BY_CAT || (type[0] == 'C' && (sel_type[0] == 'I' || sel_type[0] == 'C') )) ? inmr_rec.mr_category : " ",
		inmr_rec.mr_item_no,
		cumr_rec.cm_dbt_no);
	clip (data_str);
	sprintf (data_str1, "%c%ld%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%ld\n",
		1, sadf_rec.df_hhcu_hash,
		1, m_qty[0],
		1, m_sales[0],
		1, m_csale[0],
		1, y_qty[0],
		1, y_sales[0],
		1, y_csale[0],
		1, sadf_rec.df_hhbr_hash);
	strcat (data_str, data_str1);
	sort_save (fsort, data_str);
}

/*----------------------------------------
| Store customer based data in sort file |
----------------------------------------*/
void
store_cdata (void)
{
	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",sadf_rec.df_hhbr_hash);
	if (cc)
	{
		strcpy(inmr_rec.mr_item_no, "DELETED ITEM    ");
		sprintf(inmr_rec.mr_description, "%-40.40s","DELETED ITEM");
		sprintf(inmr_rec.mr_category, "%-11.11s", "DELETED    ");
	}

	dsp_process("Reading Item : ",inmr_rec.mr_item_no);
	if ( BY_CO )
		sprintf(data_str,"%-2.2s", "  ");
	else
		sprintf(data_str,"%-2.2s", ( envDbCo ) ? 
					cumr_rec.cm_est_no : sadf_rec.df_br_no);
	if ( REP_CUST )
		strcpy(data_str + 2,"   ");
	else if ( REP_TYPE )
		sprintf(data_str + 2,"%-3.3s", cumr_rec.cm_class_type);
	else if ( REP_SMAN )
		sprintf(data_str + 2,"%-2.2s ", sadf_rec.df_sman);
	
	sprintf(data_str + 5,"%-6.6s%-16.16s",
		cumr_rec.cm_dbt_no,
		inmr_rec.mr_item_no);
	clip (data_str);

	sprintf(data_str1, "%c%ld%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%ld\n",
		1, sadf_rec.df_hhcu_hash,
		1, m_qty[0],
		1, m_sales[0],
		1, m_csale[0],
		1, y_qty[0],
		1, y_sales[0],
		1, y_csale[0],
		1, sadf_rec.df_hhbr_hash);
	strcat (data_str, data_str1);
	sort_save(fsort,data_str);
}

void
process_data (void)
{
	char	*_sort_read(FILE *srt_fil);
	char	*sptr;
	int	print_type;

	init_array();
	printed = FALSE;
	first_time = TRUE;
	first_cat = TRUE;

	fsort = sort_sort(fsort,"sale");
	sptr = _sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(sort_str,"%-130.130s",sptr);

		set_break(sort_str,sptr);

		if (BY_CUST)
			dsp_process("Customer No :",curr_dbt);
		else
			dsp_process("Item No :",curr_prod);

		if (first_time)
		{
			strcpy(prev_br,curr_br);
			strcpy(prev_cat,curr_cat);
			strcpy(prev_prod,curr_prod);
			strcpy(prev_dbt,curr_dbt);
			strcpy(prev_type,curr_type);
			prev_hash[0] = curr_hash[0];
			prev_hash[1] = curr_hash[1];
		}

		if (first_cat &&
			type[0] == 'C' && sel_type[0] == 'C')
		{
			strcpy(prev_cat,curr_cat);
			print_header(first_time,"A",FALSE);
		}

					
		print_type = check_break();
		proc_data(first_time,print_type);

		/*
		if (strcmp(curr_cat,prev_cat))
		{
			printf("printing category \n", curr_cat,prev_cat);
			print_header(first_time,"A",FALSE);
		}
		*/

		first_time = 0;
		first_cat = 0;


		if (print_type == 0)
			sum_sales(sptr,1);
		else
			sum_sales(sptr,0);

		strcpy(prev_br,curr_br);
		strcpy(prev_cat,curr_cat);
		strcpy(prev_prod,curr_prod);
		strcpy(prev_dbt,curr_dbt);
		strcpy(prev_type,curr_type);
		prev_hash[0] = curr_hash[0];
		prev_hash[1] = curr_hash[1];

		sptr = _sort_read(fsort);
	}
	if (printed)
	{
		print_line();
		if (BY_CUST)
		{
			if (DETAIL)
			{
				print_total("C");
				if ( REP_SMAN || REP_TYPE )
					print_total(cust_type);
			}
			else
			{
				if ( REP_SMAN || REP_TYPE )
					print_total(cust_type);
			}
		}
		else
		{
			if (DETAIL)
				print_total((BY_CAT) ? "A" : "I");
		}
		print_total("B");
		print_total("E");
	}
	sort_delete(fsort,"sale");
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_sort_read (
 FILE*  srt_fil)
{
	char*   sptr;
	char*   tptr;
	int	fld_no = 0;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	tptr = sptr;
	while (fld_no < 8)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;
		ptr_offset[fld_no++] = tptr - sptr;
	}

	return (sptr);
}

void
set_break (
 char*  _sort_str,
 char*  sptr)
{
	sprintf(curr_br,"%-2.2s",_sort_str);
	if (BY_CUST)
	{
		sprintf(curr_type,"%-3.3s",_sort_str + 2);
		sprintf(curr_dbt,"%-6.6s",_sort_str + 5);
		sprintf(curr_prod,"%-16.16s",_sort_str + 11);
		curr_hash[0] = atol(sptr + ptr_offset[0]);
		curr_hash[1] = atol(sptr + ptr_offset[7]);
	}
	else
	{
		sprintf(curr_cat,"%-11.11s",_sort_str + 2);
		sprintf(curr_prod,"%-16.16s",_sort_str + 13);
		sprintf(curr_dbt,"%-6.6s",_sort_str + 29);
		curr_hash[0] = atol(sptr + ptr_offset[0]);
		curr_hash[1] = atol(sptr + ptr_offset[7]);
	}
}

int
check_break (void)
{
	if (strcmp(curr_br,prev_br))
		return (EXIT_FAILURE);

	if (BY_CUST)
	{
		if ( REP_TYPE || REP_SMAN )
			if (strcmp(curr_type,prev_type))
				return(DETAIL ? 2 : 3);

		if (curr_hash[0] != prev_hash[0])
			return(DETAIL ? 3 : 4);
		if (DETAIL && curr_hash[1] != prev_hash[1])
			return(4);
	}
	else
	{
		if (BY_CAT)
		{
			if (strcmp(curr_cat,prev_cat))
				return(DETAIL ? 3 : 4);
			if (DETAIL && curr_hash[1] != prev_hash[1])
				return(4);
		}
		else
		{
			if (curr_hash[1] != prev_hash[1])
				return(DETAIL ? 3 : 4);

			if (curr_hash[1] != prev_hash[1])
				return(DETAIL ? 3 : 4);
			if (DETAIL && curr_hash[0] != prev_hash[0])
				return(4);
		}
	}
	return(0);
}

void
cust_header (void)
{
	if (DETAIL)
		fprintf(fout,".ECUSTOMER SALES BY ITEM REPORT\n");
	else
		fprintf(fout,".ECUSTOMER SALES REPORT\n");

	if (!srt_all)
	{
		column = 45;
		if (!end_all)
			sprintf(err_str," FROM CUSTOMER %-6.6s TO CUSTOMER %-6.6s ",s_cust,e_cust);
		else
			sprintf(err_str," FROM CUSTOMER %-6.6s TO ALL CUSTOMERS ",s_cust);
	}
	else
	{
		column = 56;
		strcpy(err_str," FOR ALL CUSTOMERS ");
	}
}

void
type_header (void)
{
	if (DETAIL)
		fprintf(fout,".ECUST TYPE SALES BY CUSTOMER BY ITEM REPORT\n");
	else
		fprintf(fout,".ECUST TYPE SALES BY CUSTOMER REPORT\n");

	if (!srt_all)
	{
		column = 47;
		if (!end_all)
			sprintf(err_str," FROM CUST TYPE %-3.3s TO CUST TYPE %-3.3s ",s_type,e_type);
		else
			sprintf(err_str," FROM CUST TYPE %-3.3s TO ALL CUST TYPES ",s_type);
	}
	else
	{
		column = 54;
		strcpy(err_str," FOR ALL CUSTOMER TYPES ");
	}
}

void
salesman_header (void)
{
	if (DETAIL)
		fprintf(fout,".ESALESMAN' SALES BY CUSTOMER BY ITEM REPORT\n");
	else
		fprintf(fout,".ESALESMAN' SALES BY CUSTOMER REPORT\n");

	if (!srt_all)
	{
		column = 48;
		if (!end_all)
			sprintf(err_str," FROM SALESMAN  %-2.2s  TO SALESMAN  %-2.2s ",s_man,e_man);
		else
			sprintf(err_str," FROM SALESMAN  %-2.2s  TO ALL SALESMEN ",s_man);
	}
	else
	{
		column = 57;
		strcpy(err_str," FOR ALL SALESMEN ");
	}
}


void
category_header (void)
{
	if (DETAIL)
		fprintf(fout,".ECATEGORY SALES BY ITEM REPORT\n");
	else
		fprintf(fout,".ECATEGORY SALES REPORT\n");

	if (!srt_all)
	{
		if (!end_all)
		{
			column = 40;
			sprintf(err_str," FROM CATEGORY %-11.11s TO CATEGORY %-11.11s ",s_cat,e_cat);
		}
		else
		{
			column = 43;
			sprintf(err_str," FROM CATEGORY %-11.11s TO ALL CATEGORIES ",s_cat);
		}
	}
	else 
	{
		column = 56;
		strcpy(err_str," FOR ALL CATEGORIES ");
	}
}

void
item_header (void)
{
	if (DETAIL)
	{
		fprintf(fout,
			".EITEM SALES%s BY CUSTOMER REPORT\n",
			(CATG_SEL) ? "(CATEGORY SELECTION)" : "");
	}
	else
	{
		fprintf(fout,
			".EITEM SALES%s REPORT\n",
			(CATG_SEL) ? "(CATEGORY SELECTION)" : "");
	}

	if (!srt_all)
	{
		if (!end_all)
		{
			column = 39;
			if (CATG_SEL)
			{
				sprintf(err_str,
					" FROM GROUP %-1.1s %-11.11s TO GROUP %-1.1s %-11.11s ", 
					s_class, s_cat, 
					e_class, e_cat);
			}
			else
				sprintf(err_str," FROM ITEM %16.16s TO ITEM %16.16s ",lower,upper);
		}
		else
		{
			column = 45;
			if (CATG_SEL)
			{
				sprintf(err_str,
					" FROM GROUP %-1.1s %-11.11s TO LAST GROUP ", 
					s_class, s_cat);
			}
			else
				sprintf(err_str," FROM ITEM %16.16s TO ALL ITEMS ",lower);
		}
	}
	else 
	{
		column = 58;
		if (CATG_SEL)
			strcpy(err_str," FOR ALL GROUPS ");
		else
			strcpy(err_str," FOR ALL ITEMS ");
	}
}

int
check_page (void)
{
	return(0);
}

void
proc_data (
 int    first_time,
 int    print_type)
{
	if (first_time)
	{
		if (DETAIL)
		{
			if (BY_CUST)
				print_header(first_time,cust_type,TRUE);
			else
			if (BY_CAT)
				print_header(first_time,"A",FALSE);
			else
				print_header(first_time,"I",FALSE);

			if ( REP_TYPE || REP_SMAN )
				print_header(first_time,"C",FALSE);
		}
		else
		{
			if (BY_CUST && ( REP_TYPE || REP_SMAN ))
				print_header(first_time,cust_type,FALSE);
		}
	}

	if (!first_time && (print_type == 1 || print_type == 2 || print_type == 3))
	{
		print_line();

		switch (print_type)
		{
		case	1:
			if (BY_CUST)
			{
				if (DETAIL)
					print_total("C");

				if ( !REP_CUST )
					print_total(cust_type);

				print_total("B");
				if ( !REP_CUST )
				{
					print_header(first_time,cust_type,TRUE);
					if (DETAIL)
						print_header(first_time,"C",FALSE);
				}
			}
			else
			{
				if (BY_CAT && DETAIL)
					print_total("A");
				print_total("B");
				if (BY_CAT && DETAIL)
					print_header(first_time,"A",FALSE);
			}
			break;

		case	2:
			print_total("C");
			print_total(cust_type);
			print_header(first_time,cust_type,TRUE);
			print_header(first_time,"C",FALSE);
			break;

		case	3:
			if (BY_CUST)
			{
				if (DETAIL)
				{
					print_total("C");
					print_header(first_time,"C",FALSE);
				}
				else
				{
					print_total(cust_type);
					print_header(first_time,cust_type,TRUE);
				}
			}
			else
			{
				print_total((BY_CAT) ? "A" : "I");
				if (BY_CAT)
					print_header(first_time,"A",FALSE);
				else
				{
					if (strcmp(curr_cat,prev_cat) &&
						type[0] == 'C' && sel_type[0] == 'C')
						print_header(first_time,"A",FALSE);
					print_header(first_time,"I",FALSE);
				}
			}
			break;

		default	:
			break;
		}
	}

	if (print_type == 4)
	{
		print_line();
		if (strcmp(curr_cat,prev_cat) &&
			type[0] == 'C' && sel_type[0] == 'C')
		{
			print_header(first_time,"A",FALSE);
		}
	}
}

void
sum_sales (
 char*  data_line,
 int    add)
{
	char	*sptr = data_line;

	if (add)
	{
		m_qty[0]   += atof(sptr + ptr_offset[1]);
		m_sales[0] += atof(sptr + ptr_offset[2]);
		m_csale[0] += atof(sptr + ptr_offset[3]);
		y_qty[0]   += atof(sptr + ptr_offset[4]);
		y_sales[0] += atof(sptr + ptr_offset[5]);
		y_csale[0] += atof(sptr + ptr_offset[6]);
	}
	else
	{
		m_qty[0]   = atof(sptr + ptr_offset[1]);
		m_sales[0] = atof(sptr + ptr_offset[2]);
		m_csale[0] = atof(sptr + ptr_offset[3]);
		y_qty[0]   = atof(sptr + ptr_offset[4]);
		y_sales[0] = atof(sptr + ptr_offset[5]);
		y_csale[0] = atof(sptr + ptr_offset[6]);
	}
}

void
print_line (void)
{
	register	int	i;
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	char	var_item[17];
	char	var_desc[41];
	int	m_mar_exceed = FALSE;
	int	y_mar_exceed = FALSE;
	char	margin_exceed[8];
	char	mnth_margin[8];
	char	year_margin[8];

	/*-----------------------------------------------
	| If sales & cost of sales  = 0.00, don't print	|
	-----------------------------------------------*/
	if (m_qty[0] == 0.00 && m_sales[0] == 0.00 &&
		m_csale[0] == 0.00 && y_qty[0] == 0.00 &&
		y_sales[0] == 0.00 && y_csale[0] == 0.00)
			return;

	i = (BY_CAT || DETAIL)  ? 1 : 2;
	if ( REP_SMAN || REP_TYPE || (BY_CAT && !DETAIL))
		i  = 1;

	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",prev_hash[0]);
	if (cc)
		sprintf(cumr_rec.cm_name,"%40.40s"," ");

	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",prev_hash[1]);
	if (cc)
	{
		strcpy(inmr_rec.mr_item_no, "DELETED ITEM    ");
		sprintf(inmr_rec.mr_description, "%-40.40s","DELETED ITEM");
		sprintf(inmr_rec.mr_category, "%-11.11s", "DELETED    ");
	}
	
	if (COST_MGN)
	{
		if (m_sales[0] != 0.00)
		{
			/*---------------------------------
			| Check if margin is out of range |
			---------------------------------*/
			m_margin = (m_sales[0] - m_csale[0]) / m_sales[0] * 100.00;
			if (m_margin >= 100000)
			{
				strcpy(margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (m_margin <= -10000)
			{
				strcpy(margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}

		if (y_sales[0] != 0.00)
		{
			y_margin = (y_sales[0] - y_csale[0]) / y_sales[0] * 100.00;
			/*---------------------------------
			| Check if margin is out of range |
			---------------------------------*/
			if (y_margin >= 100000)
			{
				strcpy(margin_exceed, "+******");
				y_mar_exceed = TRUE;
			}

			if (y_margin <= -10000)
			{
				strcpy(margin_exceed, "-******");
				y_mar_exceed = TRUE;
			}
		}
	}

	if (BY_CUST)
	{
		sprintf(var_item,"%-16.16s",(DETAIL) ? prev_prod : prev_dbt);
		strcpy(var_desc,(DETAIL) ? inmr_rec.mr_description : cumr_rec.cm_name);
	}
	else
	{
		if (BY_CAT)
		{
			strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
			sprintf(excf_rec.cf_cat_no,"%-11.11s",prev_cat);
			cc = find_rec("excf", &excf_rec, COMPARISON, "r");
			if (cc)
				strcpy(excf_rec.cf_cat_desc,"No Description Found");
			sprintf(var_item,"%-16.16s",(DETAIL) ? prev_prod : prev_cat);
			strcpy(var_desc,(DETAIL) ? inmr_rec.mr_description : excf_rec.cf_cat_desc);
		}
		else
		{
			sprintf(var_item,"%-16.16s",(DETAIL) ? prev_dbt : prev_prod);
			strcpy(var_desc,(DETAIL) ? cumr_rec.cm_name : inmr_rec.mr_description);
		}
	}

	if (COST_MGN)
	{
		if (m_mar_exceed)
			sprintf(mnth_margin, "%-7.7s", margin_exceed);
		else
			sprintf(mnth_margin, "%7.1f", m_margin);

		if (y_mar_exceed)
			sprintf(year_margin, "%-7.7s", margin_exceed);
		else
			sprintf(year_margin, "%7.1f", y_margin);
	}

	if (COST_MGN)
	{
		pr_format(fin,fout,"PROD_DET",1,var_item);
		pr_format(fin,fout,"PROD_DET",2,var_desc);
		pr_format(fin,fout,"PROD_DET",3,m_qty[0]);
		pr_format(fin,fout,"PROD_DET",4,m_sales[0]);
		pr_format(fin,fout,"PROD_DET",5,m_csale[0]);
		pr_format(fin,fout,"PROD_DET",6,mnth_margin);
		pr_format(fin,fout,"PROD_DET",7,y_qty[0]);
		pr_format(fin,fout,"PROD_DET",8,y_sales[0]);
		pr_format(fin,fout,"PROD_DET",9,y_csale[0]);
		pr_format(fin,fout,"PROD_DET",10,year_margin);
	}
	else
	{
		pr_format(fin,fout,"XPROD_DET",1,var_item);
		pr_format(fin,fout,"XPROD_DET",2,var_desc);
		pr_format(fin,fout,"XPROD_DET",3,m_qty[0]);
		pr_format(fin,fout,"XPROD_DET",4,m_sales[0]);
		pr_format(fin,fout,"XPROD_DET",5,y_qty[0]);
		pr_format(fin,fout,"XPROD_DET",6,y_sales[0]);
	}


	m_qty[i] += m_qty[0];
	m_sales[i] += m_sales[0];
	m_csale[i] += m_csale[0];
	y_qty[i] += y_qty[0];
	y_sales[i] += y_sales[0];
	y_csale[i] += y_csale[0];

	m_qty[0] = 0.00;
	m_sales[0] = 0.00;
	m_csale[0] = 0.00;
	y_qty[0] = 0.00;
	y_sales[0] = 0.00;
	y_csale[0] = 0.00;
}

void
draw_line (void)
{
	pr_format(fin,fout,((COST_MGN) ? "SEPARATOR" : "XSEPARATOR") ,0,0);
}

void
print_total (
 char*  tot_type)
{
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	int	j = 0;
	int	m_mar_exceed = FALSE;
	int	y_mar_exceed = FALSE;
	char	margin_exceed[8];
	char	mnth_margin[8];
	char	year_margin[8];

	switch (tot_type[0])
	{
	case	'C':
		j = 1;
		sprintf(err_str,"%-s %-6.6s %-19.19s","Total For Customer",prev_dbt, cumr_rec.cm_acronym);
		break;

	case	'S':
		j = (DETAIL) ? 2 : 1;
		sprintf(err_str,"%-s %-2.2s %-23.23s","Total For Salesman", curr_sman,exsf_rec.sf_salesman);
		break;

	case	'T':
		j = (DETAIL) ? 2 : 1;
		sprintf(err_str,"%-s %-25.25s","Total For Cust Type", prev_type);
		break;

	case	'I':
		j = 1;
		sprintf(err_str,"%-s %-30.30s","Total For Item", prev_prod);
		break;

	case	'A':
		j = 1;
		sprintf(err_str,"%-s  %-26.26s","Total For Category",prev_cat);
		break;

	case	'B':
		j = (DETAIL && ( REP_SMAN || REP_TYPE )) ? 3 : 2;
		if (BY_CAT && !DETAIL)
			j = 1;

		if (BY_CO)
		{
			m_qty  [j+1] += m_qty[j];
			m_sales[j+1] += m_sales[j];
			m_csale[j+1] += m_csale[j];
			y_qty  [j+1] += y_qty[j];
			y_sales[j+1] += y_sales[j];
			y_csale[j+1] += y_csale[j];
			return;
		}

		/*-----------------
		| Get branch name |
		-----------------*/
		strcpy(esmr_rec.esmr_co_no, comm_rec.tco_no);
		strcpy(esmr_rec.esmr_est_no, prev_br);
		cc = find_rec("esmr", &esmr_rec, COMPARISON, "r");
		if (cc)
			sys_err("Error in esmr during (DBFIND)",cc,PNAME);

		sprintf(err_str,"%-s %-2.2s %-25.25s","Total For Branch", prev_br, esmr_rec.esmr_short_name);
		break;

	case	'E':
		if (!BY_CO)
			return;

		j = (DETAIL && ( REP_SMAN || REP_TYPE )) ? 4 : 3;
		if (BY_CAT && !DETAIL)
			j = 2;
		sprintf(err_str,"%-s","Total For Company ");
		break;
	}

	if (COST_MGN)
	{
		if (m_sales[j] != 0.00)
		{
			m_margin = (m_sales[j] - m_csale[j]) / m_sales[j] * 100.00;
			/*---------------------------------
			| Check if margin is out of range |
			---------------------------------*/
			if (m_margin >= 100000)
			{
				strcpy(margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (m_margin <= -10000)
			{
				strcpy(margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}

		if (y_sales[j] != 0.00)
		{
			y_margin = (y_sales[j] - y_csale[j]) / y_sales[j] * 100.00;
			/*---------------------------------
			| Check if margin is out of range |
			---------------------------------*/
			if (y_margin >= 100000)
			{
				strcpy(margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (y_margin <= -10000)
			{
				strcpy(margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}
	}

/*
	if (!DETAIL || (tot_type[0] != 'B' && tot_type[0] != 'E'))
		draw_line();
*/

	if (COST_MGN)
	{
		if (m_mar_exceed)
			sprintf(mnth_margin, "%-7.7s", margin_exceed);
		else
			sprintf(mnth_margin, "%7.1f", m_margin);

		if (y_mar_exceed)
			sprintf(year_margin, "%-7.7s", margin_exceed);
		else
			sprintf(year_margin, "%7.1f", y_margin);
	}

	fprintf(fout,".LRP3\n");
	if (COST_MGN)
	{
		pr_format(fin,fout,"SUB_TOT",1,err_str);
		pr_format(fin,fout,"SUB_TOT",2,m_qty[j]);
		pr_format(fin,fout,"SUB_TOT",3,m_sales[j]);
		pr_format(fin,fout,"SUB_TOT",4,m_csale[j]);
		pr_format(fin,fout,"SUB_TOT",5,mnth_margin);
		pr_format(fin,fout,"SUB_TOT",6,y_qty[j]);
		pr_format(fin,fout,"SUB_TOT",7,y_sales[j]);
		pr_format(fin,fout,"SUB_TOT",8,y_csale[j]);
		pr_format(fin,fout,"SUB_TOT",9,year_margin);
	}
	else
	{
		pr_format(fin,fout,"XSUB_TOT",1,err_str);
		pr_format(fin,fout,"XSUB_TOT",2,m_qty[j]);
		pr_format(fin,fout,"XSUB_TOT",3,m_sales[j]);
		pr_format(fin,fout,"XSUB_TOT",4,y_qty[j]);
		pr_format(fin,fout,"XSUB_TOT",5,y_sales[j]);
	}

	draw_line();

	if (tot_type[0] != 'E')
	{
		m_qty  [ j + 1 ] += m_qty[ j ];
		m_sales[ j + 1 ] += m_sales[ j ];
		m_csale[ j + 1 ] += m_csale[ j ];
		y_qty  [ j + 1 ] += y_qty[ j ];
		y_sales[ j + 1 ] += y_sales[ j ];
		y_csale[ j + 1 ] += y_csale[ j ];
	}

	m_qty[ j ]   = 0.00;
	m_sales[ j ] = 0.00;
	m_csale[ j ] = 0.00;
	y_qty[ j ]   = 0.00;
	y_sales[ j ] = 0.00;
	y_csale[ j ] = 0.00;
}

void
init_output (void)
{
	char	*month_name(int n);


	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".11\n");
	fprintf(fout,".L158\n");
	

	if (BY_CUST)
	{
		if ( REP_CUST )
			cust_header();

		if ( REP_TYPE )
			type_header();

		if ( REP_SMAN )
			salesman_header();
	}
	else
	{
		if (BY_CAT)
			category_header();
		else
			item_header(); 
	}
		
	fprintf(fout,".ECOMPANY : %s - %s\n", comm_rec.tco_no,
					      clip(comm_rec.tco_name));

	if (!strncmp(br_no,"  ",2))
		fprintf(fout,".EALL BRANCHES\n");
	else
	{
		strcpy(esmr_rec.esmr_co_no, comm_rec.tco_no);
		strcpy(esmr_rec.esmr_est_no, br_no);
		cc = find_rec("esmr", &esmr_rec, COMPARISON, "r");
		if (cc)
			sys_err("Error in esmr during (DBFIND)",cc,PNAME);

		fprintf(fout,".EBRANCH : %-2.2s - %-15.15s\n",
					br_no,esmr_rec.esmr_short_name);
	}
		
	fprintf(fout,
		".EFOR THE MONTH OF %-9.9s (%-10.10s)\n",
		month_name(curr_mnth - 1),
		DateToString(comm_rec.tdbt_date));

	fprintf(fout,".E%s\n",err_str);
	pr_format(fin,fout,((COST_MGN) ? "HEADER" : "XHEADER") ,0,0);

	if (BY_CUST)
	{
		if ( REP_CUST )
			pr_format(fin,fout,((COST_MGN) ? "HEADER1" : "XHEADER1") ,0,0);

		if ( REP_TYPE )
			pr_format(fin,fout,((COST_MGN) ? "THEADER" : "XTHEADER") ,0,0);

		if ( REP_SMAN )
			pr_format(fin,fout,((COST_MGN) ? "SHEADER" : "XSHEADER") ,0,0);

		if (DETAIL)
			pr_format(fin,fout,((COST_MGN) ? "HEADER5" : "XHEADER5") ,0,0);
		else
			pr_format(fin,fout,((COST_MGN) ? "HEADER3" : "XHEADER3") ,0,0);
	}
	else
	{
		if (BY_CAT)
			pr_format(fin,fout,((COST_MGN) ? "AHEADER" : "XAHEADER"),0,0);
		else
			pr_format(fin,fout,((COST_MGN) ? "HEADER1" : "XHEADER1"),0,0);

		if (DETAIL)
			pr_format(fin,fout,((COST_MGN) ? "HEADER4" : "XHEADER4"),0,0);
		else
			pr_format(fin,fout,((COST_MGN) ? "HEADER2" : "XHEADER2"),0,0);

	}

	pr_format(fin,fout,((COST_MGN) ? "SEPARATOR" : "XSEPARATOR") ,0,0);
	pr_format(fin,fout,((COST_MGN) ? "RULEOFF" : "XRULEOFF") ,0,0);
}

void
init_array (void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		m_qty[ j ]   = 0.00;
		y_qty[ j ]   = 0.00;
		m_sales[ j ] = 0.00;
		m_csale[ j ] = 0.00;
		y_sales[ j ] = 0.00;
		y_csale[ j ] = 0.00;
	}
}

void
print_header (
 int    first_time,
 char*  head_type,
 int    prt_head)
{
	char	var_code[17];
	char	var_desc[41];
	char	var_item[9];

	switch (head_type[0])
	{
	case	'C':
		cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",curr_hash[0]);
		if (cc)
		{
			sprintf(cumr_rec.cm_name,"%-40.40s","Unknown Customer");
			sprintf(cumr_rec.cm_dbt_no,"%-6.6s","DELETE");
			sprintf(cumr_rec.cm_class_type,"%-3.3s","DEL");
		}
		strcpy(var_code,cumr_rec.cm_dbt_no);
		strcpy(var_desc,cumr_rec.cm_name);
		strcpy(var_item,"CUSTOMER");
		break;

	case	'T':
		strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
		sprintf(excl_rec.cl_class_type,"%-3.3s",curr_type);
		cc = find_rec("excl", &excl_rec, COMPARISON, "r");
		if (cc)
			strcpy(excl_rec.cl_class_desc,"No Description Found");

		strcpy(var_code,excl_rec.cl_class_type);
		strcpy(var_desc,excl_rec.cl_class_desc);
		strcpy(var_item," CLASS  ");
		break;

	case	'S':
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",curr_type);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
			strcpy(exsf_rec.sf_salesman,"No Salesman Found");
		strcpy(var_code,exsf_rec.sf_salesman_no);
		strcpy(curr_sman,exsf_rec.sf_salesman_no);
		strcpy(var_desc,exsf_rec.sf_salesman);
		strcpy(var_item,"SALESMAN");
		break;

	case	'A':
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_cat_no,"%-11.11s",curr_cat);
		cc = find_rec("excf", &excf_rec, COMPARISON, "r");
		if (cc)
			strcpy(excf_rec.cf_cat_desc,"No Description Found");
		strcpy(var_code,excf_rec.cf_cat_no);
		strcpy(var_desc,excf_rec.cf_cat_desc);
		strcpy(var_item,"CATEGORY");
		break;

	case	'I':
		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",curr_hash[1]);
		if (cc)
		{
			strcpy(inmr_rec.mr_item_no, "DELETED ITEM    ");
			sprintf(inmr_rec.mr_description, "%-40.40s","DELETED ITEM");
			sprintf(inmr_rec.mr_category, "%-11.11s", "DELETED    ");
		}
			
		strcpy(var_code,inmr_rec.mr_item_no);
		strcpy(var_desc,inmr_rec.mr_description);
		strcpy(var_item,"ITEM    ");
		break;
	}

	expand(err_str,var_desc);

	if (prt_head)
	{
		if (COST_MGN)
		{
			if ( !first_time )
				fprintf(fout,".PA\n");
		}
		else
		{
			if ( !first_time )
				fprintf(fout,".PA\n");
		}
	}
	else
	{
		if (COST_MGN)
		{
			fprintf(fout,"| %-16.16s  %-132.132s|\n",var_code,err_str);
		}
		else
		{
			fprintf(fout,"| %-16.16s  %-88.88s  |\n",var_code,err_str);
		}
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

int
calc_mtd (void)
{
	m_qty[0]   = sadf_rec.df_qty_per[ curr_mnth - 1 ];
	m_sales[0] = sadf_rec.df_sal_per[ curr_mnth - 1 ];
	m_csale[0] = sadf_rec.df_cst_per[ curr_mnth - 1 ];
	
	return(0);
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

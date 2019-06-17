/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_dfadj.c    )                                  |
|  Program Desc  : ( Enter sales analysis figures.                   )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, sadf, sale,                                 |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 06/06/91         |
|---------------------------------------------------------------------|
|  Date Modified : 20/06/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 20/08/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 10/04/92        | Modified  by  : Campbell Mander. |
|  Date Modified : 27/05/96        | Modified  by  : JIggs Veloz.     |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : esmr_est_name was incorrect length. Could have     |
|                : caused core dump under certain circumstances.      |
|                :                                                    |
|  (20/08/91)    : Fixed Month Name in audit trail and added sman and |
|                : area to audit trail.                               |
|                :                                                    |
|  (10/04/92)    : Fix entry of customer numbers. Also fix searches.  |
|                : NOTE. This program does not use the standard cumr  |
|                : search as it is a requirement for ALL debtors to   |
|                : appear in every search.                            |
|                :                                                    |
|  (27/05/96)    : Fixed problems in DateToString. 						  |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|                :                                                    |
|                :                                                    |
| $Log: sa_dfadj.c,v $
| Revision 5.2  2001/08/09 09:16:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:33  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:55  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:25  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/06/13 05:02:35  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.13  1999/12/06 01:35:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/17 06:40:38  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.11  1999/11/16 04:55:32  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.10  1999/10/20 02:07:02  nz
| Updated for final changes on date routines.
|
| Revision 1.9  1999/10/16 01:11:21  nz
| Updated for pjulmdy routines
|
| Revision 1.8  1999/10/01 07:49:13  scott
| Updated for standard function calls.
|
| Revision 1.7  1999/09/29 10:12:49  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:33  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 02:01:51  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/06/18 09:39:20  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_dfadj.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_dfadj/sa_dfadj.c,v 5.2 2001/08/09 09:16:54 scott Exp $";

#include <pslscr.h>
#include <ml_sa_mess.h>
#include <ml_std_mess.h>

#define	SALE	0
#define	SADF	1

#define	ADD	0
#define	UPDATE	1

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_dp_no"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int	comm_no_fields = 10;

	struct	{
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	test_short[16];
		char	tcc_no[3];
		char	tdp_no[3];
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

	struct	SADF_REC
	{
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

	/*==============================================
	| Sales Analysis By Customer/Category/Salesman |
	==============================================*/
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
		{"sale_disc"},
	};

	int	sale_no_fields = 12;

	struct	SALE_REC
	{
		char	le_key[9];
		char	le_category[12];
		char	le_sman[3];
		char	le_area[3];
		char	le_ctype[4];
		char	le_dbt_no[7];
		char	le_year_flag[2];
		char	le_period[3];
		double	le_units;
		double	le_gross;	/* money */
		double	le_cost_sale;	/* money */
		double	le_disc;	/* money */
	} sale_rec, wk_sale_rec;

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
		{"cumr_class_type"},
	};

	int	cumr_no_fields = 8;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
	} cumr_rec; 

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_quick_code"},
	};

	int inmr_no_fields = 12;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_alpha_code[17];
		char	mr_super_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		char	mr_quick_code[9];
	} inmr_rec, wk_inmr_rec;

	/*=================================
	| Company Master File Base Record |
	=================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
	};

	int	comr_no_fields = 2;

	struct	{
		char	mr_co_no[3];
		char	mr_co_name[41];
	} comr_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	int	esmr_no_fields = 3;

	struct	{
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_est_name[41];
	} esmr_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_name"},
	};

	int	ccmr_no_fields = 4;

	struct	{
		char	cc_co_no[3];
		char	cc_est_no[3];
		char	cc_cc_no[3];
		char	cc_name[41];
	} ccmr_rec;

	/*========================
	| Department Master File |
	========================*/
	struct dbview cudp_list[] ={
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
	};

	int	cudp_no_fields = 4;

	struct	{
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_name[41];
	} cudp_rec;

	/*========================
	| External Salesman File |
	========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int	exsf_no_fields = 3;

	struct	{
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
	} exsf_rec;

	/*====================
	| External Area file |
	====================*/
	struct dbview exaf_list[] ={
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
	};

	int	exaf_no_fields = 3;

	struct	{
		char	af_co_no[3];
		char	af_area_code[3];
		char	af_area[41];
	} exaf_rec;

FILE	*fout;
FILE	*popen(const char *, const char *);

long	tloc = -1L;

char	*month_name(int n);

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	int	lpno;
	char	time[9];
	char	log_name[15];
	char	period[3];
	char	period_desc[10];
	char	co_no[3];
	char	co_name[41];
	char	br_no[3];
	char	br_name[41];
	char	wh_no[3];
	char	wh_name[41];
	char	dp_no[3];
	char	dp_name[41];
	char	cust_type[4];
	char	cust[7];
	char	cust_name[41];
	long	hhcu_hash;
	char	sman[3];
	char	sman_name[41];
	char	area[3];
	char	area_name[41];
	char	category[12];
	char	item[17];
	char	item_desc[41];
	long	hhbr_hash;
	float	qty;
	double	value;
	double	disc;
	double	cost;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "period",	 3, 14, CHARTYPE,
		"AA", "          ",
		" ", "", "Month      :", "01 = January , 12 = December.",
		YES, NO, JUSTRIGHT, "", "", local_rec.period},
	{1, LIN, "period_desc",	 3, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.period_desc},

	{1, LIN, "co_no",	 5, 14, CHARTYPE,
		"UU", "          ",
		" ", comm_rec.tco_no, "Company    :", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.co_no},
	{1, LIN, "co_name",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.co_name},
	{1, LIN, "br_no",	 6, 14, CHARTYPE,
		"UU", "          ",
		" ", comm_rec.test_no, "Branch     :", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_name",	 6, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_name},
	{1, LIN, "wh_no",	 7, 14, CHARTYPE,
		"UU", "          ",
		" ", comm_rec.tcc_no, "Warehouse  :", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.wh_no},
	{1, LIN, "wh_name",	 7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.wh_name},
	{1, LIN, "dp_no",	 8, 14, CHARTYPE,
		"UU", "          ",
		" ", comm_rec.tdp_no, "Department :", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dp_no},
	{1, LIN, "dp_name",	 8, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dp_name},

	{1, LIN, "cust",	10, 14, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer   :", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.cust},
	{1, LIN, "cust_name",	10, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cust_name},
	{1, LIN, "sman",	11, 14, CHARTYPE,
		"UU", "          ",
		" ", "", "Salesman   :", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.sman},
	{1, LIN, "sman_name",	11, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sman_name},
	{1, LIN, "area",	12, 14, CHARTYPE,
		"UU", "          ",
		" ", "", "Sales Area :", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.area},
	{1, LIN, "area_name",	12, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_name},
	{1, LIN, "item",	13, 14, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item       :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item},
	{1, LIN, "item_desc",	13, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{1, LIN, "qty",	15, 15, FLOATTYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "N", "Sales Quantity :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty},
	{1, LIN, "value",	16, 15, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "N", "Sales Value    :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.value},
	{1, LIN, "disc",	17, 15, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "N", "Sales Discount :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.disc},
	{1, LIN, "cost",	18, 15, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "N", "Sales Cost     :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cost},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.                                          |
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int process (void);
int zero_tot (void);
void add_sale (void);
int update_sale (void);
void add_sadf (void);
void update_sadf (void);
int open_audit (void);
int print_audit (int status, int file_ref);
void close_audit (void);
int spec_valid (int field);
char *month_name (int n);
void srch_co (char *key_val);
void srch_br (char *key_val);
void srch_wh (char *key_val);
void srch_dp (char *key_val);
void srch_cust (char *key_val);
void srch_sman (char *key_val);
void SrchExaf (char *key_val);
int heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	if (argc != 2)
	{
		/* Usage: %s <LPNO>\n"*/
		print_at(0,0, ML(mlStdMess036), argv[0]);
        return (EXIT_FAILURE);
	}

	local_rec.lpno = atoi(argv[1]);

	SETUP_SCR( vars );

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	sprintf(local_rec.log_name, "%-14.14s", getenv("LOGNAME"));

	OpenDB ();

	open_audit ();

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
		init_vars (1);		/*  set default values	*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
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

		process();
	}

	close_audit ();
	shutdown_prog ();
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
	open_rec("comr", comr_list, comr_no_fields, "comr_co_no");
	open_rec("esmr", esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec("ccmr", ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec("cudp", cudp_list, cudp_no_fields, "cudp_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_id_no3");
	open_rec("exsf", exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec("exaf", exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec("sale", sale_list, sale_no_fields, "sale_id_no");
	open_rec("sadf", sadf_list, sadf_no_fields, "sadf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("comr");
	abc_fclose("esmr");
	abc_fclose("ccmr");
	abc_fclose("cudp");
	abc_fclose("cumr");
	abc_fclose("exsf");
	abc_fclose("exaf");
	abc_fclose("inmr");
	abc_fclose("sale");
	abc_fclose("sadf");

	SearchFindClose ();
	abc_dbclose("data");
}

int
process (void)
{
	/*--------------------------
	| Get time for audit trail |
	--------------------------*/
	strcpy (local_rec.time, TimeHHMM());

	/*------------------
	| Find sale record |
	------------------*/
	sprintf (sale_rec.le_key, 
		"%2.2s%2.2s%2.2s%2.2s", 
		local_rec.co_no,
		local_rec.br_no,
		local_rec.wh_no,
		local_rec.dp_no);

	sprintf(sale_rec.le_category, "%-11.11s", local_rec.category);
	sprintf(sale_rec.le_sman,     "%2.2s", local_rec.sman);
	sprintf(sale_rec.le_area,     "%2.2s", local_rec.area);
	sprintf(sale_rec.le_ctype,    "%-3.3s", local_rec.cust_type);
	sprintf(sale_rec.le_dbt_no,   "%-6.6s", local_rec.cust);
	strcpy(sale_rec.le_year_flag, "C");
	sprintf(sale_rec.le_period,   "0%1d", atoi(local_rec.period));

	cc = find_rec("sale", &sale_rec, COMPARISON, "w");
	if (cc)
	{
		if (!zero_tot())
		{
			add_sale();
			print_audit(ADD, SALE);
		}
	}
	else
	{
		if (!zero_tot())
		{
			memcpy ((char *)&wk_sale_rec, (char *)&sale_rec, sizeof (struct SALE_REC));
			update_sale();
			print_audit(UPDATE,SALE);
		}
	}
		
	/*------------------
	| Find sadf record |
	------------------*/
	sprintf(sadf_rec.df_co_no, "%2.2s", local_rec.co_no);
	sprintf(sadf_rec.df_br_no, "%2.2s", local_rec.br_no);
	strcpy(sadf_rec.df_year, "C");
	sadf_rec.df_hhcu_hash = local_rec.hhcu_hash;
	sadf_rec.df_hhbr_hash = local_rec.hhbr_hash;
	sprintf(sadf_rec.df_sman, "%2.2s", local_rec.sman);
	sprintf(sadf_rec.df_area, "%2.2s", local_rec.area);
	cc = find_rec("sadf", &sadf_rec, COMPARISON, "w");
	if (cc)
	{
		if (!zero_tot())
		{
			add_sadf();
			print_audit(ADD,SADF);
		}
	}
	else
	{
		if (!zero_tot())
		{
			memcpy ((char *)&wk_sadf_rec, (char *)&sadf_rec, sizeof (struct SADF_REC));
			update_sadf();
			print_audit(UPDATE,SADF);
		}
	}

	return(0);
}

/*----------------------------
| Check if any of the values |
| are non-zero		     |
----------------------------*/
int
zero_tot (void)
{
	if (local_rec.qty != 0.00)
		return(FALSE);

	if (local_rec.value != 0.00)
		return(FALSE);

	if (local_rec.disc != 0.00)
		return(FALSE);

	if (local_rec.cost != 0.00)
		return(FALSE);

	return(TRUE);
}

void
add_sale(void)
{
	sprintf(sale_rec.le_key,
		"%2.2s%2.2s%2.2s%2.2s", 
		local_rec.co_no,
		local_rec.br_no,
		local_rec.wh_no,
		local_rec.dp_no);
	sprintf(sale_rec.le_category, "%-11.11s", local_rec.category);
	sprintf(sale_rec.le_sman, "%2.2s", local_rec.sman);
	sprintf(sale_rec.le_area, "%2.2s", local_rec.area);
	sprintf(sale_rec.le_ctype, "%-3.3s", local_rec.cust_type);
	sprintf(sale_rec.le_dbt_no, "%-6.6s", local_rec.cust);
	strcpy(sale_rec.le_year_flag, "C");
	sprintf(sale_rec.le_period,   "0%1d", atoi(local_rec.period));
	sale_rec.le_units = local_rec.qty;
	sale_rec.le_gross = CENTS(local_rec.value);
	sale_rec.le_cost_sale = CENTS(local_rec.cost);
	sale_rec.le_disc = CENTS(local_rec.disc);

	cc = abc_add("sale", &sale_rec);
	if (cc)
		sys_err("Error in sale During (DBADD)",cc,PNAME);
}

int
update_sale (void)
{
	sale_rec.le_units += local_rec.qty;
	sale_rec.le_gross += CENTS(local_rec.value);
	sale_rec.le_cost_sale += CENTS(local_rec.cost);
	sale_rec.le_disc += CENTS(local_rec.disc);

	cc = abc_update("sale", &sale_rec);
	if (cc)
		sys_err("Error in sale During (DBUPDATE)",cc,PNAME);

	return(0);
}

void
add_sadf (void)
{
	int	i;

	sprintf(sadf_rec.df_co_no, "%2.2s", local_rec.co_no);
	sprintf(sadf_rec.df_br_no, "%2.2s", local_rec.br_no);
	strcpy(sadf_rec.df_year, "C");
	sadf_rec.df_hhbr_hash = local_rec.hhbr_hash;
	sadf_rec.df_hhcu_hash = local_rec.hhcu_hash;
	
	for (i = 0; i < 12; i++)
	{
		sadf_rec.df_qty_per[i] = 0.00;
		sadf_rec.df_sal_per[i] = 0.00;
		sadf_rec.df_cst_per[i] = 0.00;
	}

	sadf_rec.df_qty_per[atoi(local_rec.period) - 1] = local_rec.qty;
	sadf_rec.df_sal_per[atoi(local_rec.period) - 1] = local_rec.value - local_rec.disc;
	sadf_rec.df_cst_per[atoi(local_rec.period) - 1] = local_rec.cost;

	sprintf(sadf_rec.df_sman, "%2.2s", local_rec.sman);
	sprintf(sadf_rec.df_area, "%2.2s", local_rec.area);

	cc = abc_add("sadf", &sadf_rec);
	if (cc)
		sys_err("Error in sadf During (DBADD)",cc,PNAME);

}

void
update_sadf (void)
{
	sadf_rec.df_qty_per[atoi(local_rec.period) - 1] += local_rec.qty;
	sadf_rec.df_sal_per[atoi(local_rec.period) - 1] += (local_rec.value - local_rec.disc);
	sadf_rec.df_cst_per[atoi(local_rec.period) - 1] += local_rec.cost;
	
	cc = abc_update("sadf", &sadf_rec);
	if (cc)
		sys_err("Error in sadf during (DBUPDATE)",errno,PNAME);

}

/*===============================================================
|	Routine to open output pipe to standard print to	|
|	provide an audit trail of events.			|
|	This also sends the output straight to the spooler.	|
===============================================================*/
int
open_audit (void)
{
	if ((fout = popen("pformat","w")) == NULL) 
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	fprintf(fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".SO\n");
	fprintf(fout,".7\n");
	fprintf(fout,".L132\n");
	fprintf(fout,".PI10\n");
	fprintf(fout,
		".ESALES ANALYSIS ENTRY AUDIT TRAIL FOR %s\n", 
		clip(comm_rec.test_short));

	fprintf(fout, ".CSTARTED %s BY %-8.8s\n", SystemTime (), local_rec.log_name);
	fprintf(fout,".B1\n");

	fprintf(fout,".R=================");
	fprintf(fout,"=========================================");
	fprintf(fout,"======");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");

	fprintf(fout,"=================");
	fprintf(fout,"=========================================");
	fprintf(fout,"======");
	fprintf(fout,"==========");
	fprintf(fout,"=========");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"============");
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");

	return(0);
}

int
print_audit (
 int    status,
 int    file_ref)
{
	fprintf(fout,
		"!%-48.48s %-4.4s record  %-7.7s at %-8.8s %-48.48s!\n",
		" ",
		(file_ref == SALE) ? "sale" : "sadf",
		(status == ADD) ? "ADDED" : "UPDATED",
		local_rec.time,
		" ");

	fprintf(fout,
		"! Company: %2.2s    Branch: %2.2s    Warehouse: %2.2s    Department: %2.2s      MONTH:    %02d   %-9.9s%-39.39s!\n",
		local_rec.co_no,
		local_rec.br_no,
		local_rec.wh_no,
		local_rec.dp_no,
		atoi(local_rec.period),
		month_name(atoi(local_rec.period) - 1),
		" ");

	fprintf(fout,
	       "! Customer: %-6.6s  %-40.40s    Item:  %-16.16s  %-40.40s  !\n",
	       local_rec.cust,
	       local_rec.cust_name,
	       local_rec.item,
	       local_rec.item_desc);

	fprintf(fout,
	       "! Salesman: %2.2s   %-40.40s       Area: %2.2s   %-40.40s                !\n",
	       local_rec.sman,
	       local_rec.sman_name,
	       local_rec.area,
	       local_rec.area_name);

	fprintf(fout, "!%-130.130s!\n", " ");

	if (status == UPDATE)
	{
		if (file_ref == SALE)
		{
			fprintf(fout,
				"! OLD VALUES        Quantity: %12.2f      Value: %12.2f      Disc: %12.2f      Cost: %12.2f %-15.15s!\n",
				wk_sale_rec.le_units,
				DOLLARS(wk_sale_rec.le_gross),
				DOLLARS(wk_sale_rec.le_disc),
				DOLLARS(wk_sale_rec.le_cost_sale),
				" ");
		}
		else
		{
			fprintf(fout,
				"! OLD VALUES        Quantity: %12.2f      Value: %12.2f     Cost: %12.2f %-40.40s!\n",
				wk_sadf_rec.df_qty_per[atoi(local_rec.period) -1],
				wk_sadf_rec.df_sal_per[atoi(local_rec.period) -1],
				wk_sadf_rec.df_cst_per[atoi(local_rec.period) -1],
				" ");
		}
	}

	if (file_ref == SALE)
	{
		fprintf(fout,
			"! NEW VALUES        Quantity: %12.2f      Value: %12.2f      Disc: %12.2f      Cost: %12.2f                !\n",
			sale_rec.le_units,
			DOLLARS(sale_rec.le_gross),
			DOLLARS(sale_rec.le_disc),
			DOLLARS(sale_rec.le_cost_sale));
	}
	else
	{
		fprintf(fout,
			"! NEW VALUES        Quantity: %12.2f      Value: %12.2f     Cost: %12.2f %-40.40s!\n",
			sadf_rec.df_qty_per[atoi(local_rec.period) -1],
			sadf_rec.df_sal_per[atoi(local_rec.period) -1],
			sadf_rec.df_cst_per[atoi(local_rec.period) -1],
			" ");
	}

	fprintf(fout, "!--------------------------------------------");
	fprintf(fout, "--------------------------------------------");
	fprintf(fout, "------------------------------------------!\n");

	return(0);
}

/*=======================================================
|	Routine to close the audit trail output file.	|
=======================================================*/
void
close_audit (void)
{
	fprintf(fout,".EOF\n");
	pclose(fout);
}

int
spec_valid (
 int    field)
{
	int	tmp_month,
		sys_month;

	DateToDMY (comm_rec.tdbt_date, NULL, &sys_month, NULL);

	if (LCHECK("period"))
	{
		tmp_month = atoi(temp_str);

		if (tmp_month < 1 || tmp_month > 12)
		{
			/*----------------------------
			| Month must be from 1 to 12 |
			----------------------------*/
			print_mess( ML(mlSaMess021) );
			sleep(2);
			clear_mess();
			return(1);
		}

		strcpy(local_rec.period_desc, month_name(atoi(temp_str) - 1));
		DSP_FLD("period_desc");
		return(0);

/*
================================================================================
NOTE... At some time, we must remember to validate the period/month to make
sure that no stupid fucken users try to adjust forward figures!!
================================================================================
		print_mess( ML(mlSaMess022) );

		sleep(2);
		clear_mess();
		return(1);
*/
	}

	if (LCHECK("co_no"))
	{
		if (SRCH_KEY)
		{
			srch_co(temp_str);
			return(0);
		}

		sprintf(comr_rec.mr_co_no, "%2.2s", local_rec.co_no);
		cc = find_rec("comr", &comr_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Company Not On File |
			---------------------*/
			print_mess( ML(mlStdMess130) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.co_name, "%-40.40s", comr_rec.mr_co_name);
			DSP_FLD("co_name");
			return(0);
		}
	}

	if (LCHECK("br_no"))
	{
		if (SRCH_KEY)
		{
			srch_br(temp_str);
			return(0);
		}

		sprintf(esmr_rec.mr_co_no, "%2.2s", local_rec.co_no);
		sprintf(esmr_rec.mr_est_no, "%2.2s", local_rec.br_no);
		cc = find_rec("esmr", &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------
			| Branch Not On File |
			--------------------*/
			print_mess( ML(mlStdMess073) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.br_name, "%-40.40s", esmr_rec.mr_est_name);
			DSP_FLD("br_name");
			return(0);
		}
	}

	if (LCHECK("wh_no"))
	{
		if (SRCH_KEY)
		{
			srch_wh(temp_str);
			return(0);
		}

		sprintf(ccmr_rec.cc_co_no, "%2.2s", local_rec.co_no);
		sprintf(ccmr_rec.cc_est_no, "%2.2s", local_rec.br_no);
		sprintf(ccmr_rec.cc_cc_no, "%2.2s", local_rec.wh_no);
		cc = find_rec("ccmr", &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------
			| Warehouse Not On File |
			-----------------------*/
			print_mess( ML(mlStdMess100) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.wh_name, "%-40.40s", ccmr_rec.cc_name);
			DSP_FLD("wh_name");
			return(0);
		}
	}

	if (LCHECK("dp_no"))
	{
		if (SRCH_KEY)
		{
			srch_dp(temp_str);
			return(0);
		}

		sprintf(cudp_rec.dp_co_no, "%2.2s", local_rec.co_no);
		sprintf(cudp_rec.dp_br_no, "%2.2s", local_rec.br_no);
		sprintf(cudp_rec.dp_dp_no, "%2.2s", local_rec.dp_no);
		cc = find_rec("cudp", &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------
			| Department Not found. |
			-----------------------*/
			print_mess( ML(mlStdMess084) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.dp_name,"%-40.40s", cudp_rec.dp_name);
			DSP_FLD("dp_name");
			return(0);
		}
	}

	if (LCHECK("cust"))
	{
		if (SRCH_KEY)
		{
			srch_cust(temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no, local_rec.co_no);
		sprintf(cumr_rec.cm_dbt_no, "%-6.6s", pad_num(temp_str));
		cc = find_rec("cumr", &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Customer Not Found. |
			---------------------*/
			print_mess( ML(mlStdMess021) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.cust_type, "%-3.3s",cumr_rec.cm_class_type);
			local_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;

			sprintf(local_rec.cust, "%-6.6s",cumr_rec.cm_dbt_no);
			sprintf(local_rec.cust_name, "%-40.40s", cumr_rec.cm_name);
			DSP_FLD("cust");
			DSP_FLD("cust_name");
			return(0);
		}
	}

	if (LCHECK("sman"))
	{
		if (SRCH_KEY)
		{
			srch_sman(temp_str);
			return(0);
		}

		sprintf(exsf_rec.sf_co_no, "%2.2s", local_rec.co_no);
		sprintf(exsf_rec.sf_salesman_no, "%2.2s", local_rec.sman);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Salesman Not found. |
			---------------------*/
			print_mess( ML(mlStdMess135) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.sman_name, "%-40.40s", exsf_rec.sf_salesman);
			DSP_FLD("sman_name");
			return(0);
		}
	}

	if (LCHECK("area"))
	{
		if (SRCH_KEY)
		{
			SrchExaf(temp_str);
			return(0);
		}

		sprintf(exaf_rec.af_co_no, "%2.2s", local_rec.co_no);
		sprintf(exaf_rec.af_area_code, "%2.2s", local_rec.area);
		cc = find_rec("exaf", &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			/*----------------- 
			| Area Not found. |
			----------------- */
			print_mess( ML(mlStdMess108) );
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.area_name, "%-40.40s", exaf_rec.af_area);
			DSP_FLD("area_name");
			return(0);
		}
	}

	if (LCHECK("item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*----------------
			| Item Not Found |
			----------------*/
			print_mess( ML(mlStdMess001) );
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		strcpy(local_rec.item, inmr_rec.mr_item_no);
		sprintf(local_rec.category, "%-11.11s", inmr_rec.mr_category);
		local_rec.hhbr_hash = inmr_rec.mr_hhbr_hash;
		sprintf(local_rec.item_desc,"%-40.40s",inmr_rec.mr_description);
		DSP_FLD("item_desc");
		return(0);
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

void
srch_co (
 char*  key_val)
{
        work_open();
	save_rec("#Co", "#Company");
	sprintf(comr_rec.mr_co_no, "%2.2s", key_val);
	cc = find_rec("comr", &comr_rec, GTEQ, "r");
        while (!cc && !strncmp(comr_rec.mr_co_no, key_val, strlen(key_val)))
    	{                        
		cc = save_rec(comr_rec.mr_co_no,comr_rec.mr_co_name);
		if (cc)
			break;

		cc = find_rec("comr",&comr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	sprintf(comr_rec.mr_co_no, "%2.2s", temp_str);
	cc = find_rec("comr", &comr_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in comr During (DBFIND)",cc,PNAME);
    }
}

void
srch_br (
 char*  key_val)
{
        work_open();
	save_rec("#Br", "#Branch");
	strcpy(esmr_rec.mr_co_no, comr_rec.mr_co_no);
	sprintf(esmr_rec.mr_est_no, "%2.2s", key_val);
	cc = find_rec("esmr", &esmr_rec, GTEQ, "r");
        while (!cc && !strcmp(esmr_rec.mr_co_no, comr_rec.mr_co_no) && 
		      !strncmp(esmr_rec.mr_est_no, key_val, strlen(key_val)))
    	{                        
		cc = save_rec(esmr_rec.mr_est_no, esmr_rec.mr_est_name);
		if (cc)
			break;

		cc = find_rec("esmr", &esmr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(esmr_rec.mr_co_no, comr_rec.mr_co_no);
	sprintf(esmr_rec.mr_est_no, "%2.2s", temp_str);
	cc = find_rec("esmr", &esmr_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in esmr During (DBFIND)",cc,PNAME);
    }
}

void
srch_wh (
 char*  key_val)
{
        work_open();
	save_rec("#Wh", "#Warehouse");
	strcpy(ccmr_rec.cc_co_no, comr_rec.mr_co_no);
	strcpy(ccmr_rec.cc_est_no, esmr_rec.mr_est_no);
	sprintf(ccmr_rec.cc_cc_no, "%2.2s", key_val);
	cc = find_rec("ccmr", &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp(ccmr_rec.cc_co_no, comr_rec.mr_co_no) && 
		      !strcmp(ccmr_rec.cc_est_no, esmr_rec.mr_est_no) && 
		      !strncmp(ccmr_rec.cc_cc_no, key_val, strlen(key_val)))
    	{                        
	        cc = save_rec(ccmr_rec.cc_cc_no, ccmr_rec.cc_name); 
		if (cc)
		        break;

		cc = find_rec("ccmr",&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(ccmr_rec.cc_co_no, comr_rec.mr_co_no);
	strcpy(ccmr_rec.cc_est_no, esmr_rec.mr_est_no);
	sprintf(ccmr_rec.cc_cc_no, "%2.2s", temp_str);
	cc = find_rec("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in ccmr During (DBFIND)",cc,PNAME);
    }
}

void
srch_dp (
 char*  key_val)
{
        work_open();
	save_rec("#Dp", "#Department");
	strcpy(cudp_rec.dp_co_no, comr_rec.mr_co_no);
	strcpy(cudp_rec.dp_br_no, esmr_rec.mr_est_no);
	sprintf(cudp_rec.dp_dp_no, "%2.2s", key_val);
	cc = find_rec("cudp", &cudp_rec, GTEQ, "r");
	while (!cc && !strcmp(cudp_rec.dp_co_no, comr_rec.mr_co_no) && 
		      !strcmp(cudp_rec.dp_br_no, esmr_rec.mr_est_no) && 
		      !strncmp(cudp_rec.dp_dp_no, key_val, strlen(key_val)))
    	{                        
	        cc = save_rec(cudp_rec.dp_dp_no, cudp_rec.dp_name); 
		if (cc)
		        break;

		cc = find_rec("cudp",&cudp_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(cudp_rec.dp_co_no, comr_rec.mr_co_no);
	strcpy(cudp_rec.dp_br_no, esmr_rec.mr_est_no);
	sprintf(cudp_rec.dp_dp_no, "%2.2s", temp_str);
	cc = find_rec("cudp", &cudp_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in cudp During (DBFIND)",cc,PNAME);
    }
}

void
srch_cust (
 char*  key_val)
{
        work_open();
	save_rec("#Number", "#Customer Name");
	strcpy(cumr_rec.cm_co_no, comr_rec.mr_co_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", key_val);
	cc = find_rec("cumr", &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp(cumr_rec.cm_co_no, comr_rec.mr_co_no) && 
		      !strncmp(cumr_rec.cm_dbt_no, key_val, strlen(key_val)))
    	{                        
	        cc = save_rec(cumr_rec.cm_dbt_no, cumr_rec.cm_name); 
		if (cc)
		        break;

		cc = find_rec("cumr", &cumr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(cumr_rec.cm_co_no, comr_rec.mr_co_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", temp_str);
	cc = find_rec("cumr", &cumr_rec, COMPARISON, "r");
	if (cc)
 	        sys_err("Error in cumr During (DBFIND)",cc,PNAME);
}

void
srch_sman (
 char*  key_val)
{
        work_open();
	save_rec("#Sman", "#Salesman Name");
	strcpy(exsf_rec.sf_co_no, comr_rec.mr_co_no);
	sprintf(exsf_rec.sf_salesman_no, "%2.2s", key_val);
	cc = find_rec("exsf", &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp(exsf_rec.sf_co_no, comr_rec.mr_co_no) && 
		      !strncmp(exsf_rec.sf_salesman_no,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(exsf_rec.sf_salesman_no, exsf_rec.sf_salesman); 
		if (cc)
		        break;

		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(exsf_rec.sf_co_no, comr_rec.mr_co_no);
	sprintf(exsf_rec.sf_salesman_no, "%2.2s", temp_str);
	cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in exsf During (DBFIND)",cc,PNAME);
    }
}

void
SrchExaf (
 char*  key_val)
{
    work_open();
	save_rec("#Area", "#Area Name");
	strcpy(exaf_rec.af_co_no, comr_rec.mr_co_no);
	sprintf(exaf_rec.af_area_code, "%2.2s", key_val);
	cc = find_rec("exaf", &exaf_rec, GTEQ, "r");
	while (!cc && !strcmp(exaf_rec.af_co_no, comr_rec.mr_co_no) && 
		      !strncmp(exaf_rec.af_area_code,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(exaf_rec.af_area_code, exaf_rec.af_area); 
		if (cc)
		        break;

		cc = find_rec("exaf",&exaf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(exaf_rec.af_co_no, comr_rec.mr_co_no);
	sprintf(exaf_rec.af_area_code, "%2.2s", temp_str);
	cc = find_rec("exaf", &exaf_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err("Error in exaf During (DBFIND)",cc,PNAME);
    }
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

		/*----------------------
		| Sales Analysis Entry |
		----------------------*/
		sprintf (err_str, " %s ", ML(mlSaMess020) );
		rv_pr(err_str,28,0,1);

		move(0,1);
		line(80);

		box(0,2,80,16);
		move(1,4);
		line(79);

		move(1,9);
		line(79);

		move(1,14);
		line(79);

		move(0,20);
		line(80);
		sprintf (err_str, ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at(21,0, "%s", err_str);
		print_at(21,48,ML(mlStdMess039),comm_rec.test_no,comm_rec.test_name);
		move(0,22);
		line(80);
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

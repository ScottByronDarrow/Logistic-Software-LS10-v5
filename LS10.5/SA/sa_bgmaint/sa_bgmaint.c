/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_bgmaint.c   )                                 |
|  Program Desc  : ( Sales Analysis Budget Maintenance.           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, exaf, exsf, excl, excf, sabg,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sabg,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 14/05/87         |
|---------------------------------------------------------------------|
|  Date Modified : (14/05/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : (05/10/90) - General Update for New Scrgen. S.B.D. |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: sa_bgmaint.c,v $
| Revision 5.3  2002/07/18 07:07:26  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:16:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:14  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:35:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 04:55:30  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.6  1999/09/29 10:12:41  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:27:24  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 02:01:50  scott
| Updated from Ansi Project.
|
| Revision 1.3  1999/06/18 09:39:18  scott
| Updated for read_comm(), log for cvs, compile errors.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_bgmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_bgmaint/sa_bgmaint.c,v 5.3 2002/07/18 07:07:26 scott Exp $";

#include <pslscr.h>
#include <ml_sa_mess.h>
#include <ml_std_mess.h>
#undef	TABLINES
#define	TABLINES	12

#define	SALES	0
#define	CATG	1
#define	A_CODE	2
#define	C_TYPE	3

int	new_lsabg;
int	new_csabg;
int	new_sabg;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
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
	};
	
	int comm_no_fields = 10;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_name[41];
		char	tes_short[16];
		char	tcc_no[3];
		char	tcc_name[41];
		char	tcc_short[10];
	} comm_rec;

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
	} lsabg_rec,csabg_rec;

char	*months[] = {
	"January",
	"Febuary",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

int	rep_type;

/*=========================== 
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy[11];
	char	l_mask[12];
	char	l_prompt[12];
	char	l_code[12];
	char	l_desc[41];
	char	l_month[11];
	char	l_file[5];
	double	c_sales;
	double	c_profit;
	double	l_sales;
	double	l_profit;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "l_code", 4, 15, CHARTYPE, 
		local_rec.l_mask, "          ", 
		" ", "", local_rec.l_prompt, " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.l_code}, 
	{1, LIN, " ", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.l_desc}, 
	{2, TAB, "months", 12, 1, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "   Month   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.l_month}, 
	{2, TAB, "lsales", 0, 1, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", " Last Sales  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_sales}, 
	{2, TAB, "lprofit", 0, 1, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", " Last Profit ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_profit}, 
	{2, TAB, "csales", 0, 2, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", " Current Sales  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.c_sales}, 
	{2, TAB, "cprofit", 0, 3, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", " Current Profit ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.c_profit}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================================================================
| Local Function Prototypes
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int check_input (void);
void load_month (void);
void load_scn (void);
void update (void);
void show_area (char *key_val);
void show_sale (char *key_val);
void show_class (char *key_val);
void show_category (char *key_val);
int heading (int scn);

/*=====================================================================
| Main Processing Routine.                                            |
=====================================================================*/
int
main (
 int    argc,
 char*  argv[])
{
	/*---------------------------------------
	| check parameter count			|
	---------------------------------------*/
	if (argc != 2)
	{
		/*--------------------------------
		| Usage : %s <sales_budget_type> |
		--------------------------------*/
		print_at(0,0, ML(mlSaMess737), argv[0]);
        return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);

	/*---------------------------------------
	| check rep type			|
	---------------------------------------*/
	rep_type = atoi(argv[1]);
	if (rep_type < 0 || rep_type > 3)
	{
		print_at(0,0, ML(mlSaMess738) );
		print_at(0,0, ML(mlSaMess739) );
		print_at(0,0, ML(mlSaMess740) );
		print_at(0,0, ML(mlSaMess741) );
		return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| modify screen definition		
	---------------------------------------*/
	switch (rep_type)
	{
	case SALES:
		strcpy(local_rec.l_prompt,"Salesman ");
		strcpy(local_rec.l_mask,"UU");
		strcpy(local_rec.l_file,"exsf");
		vars[0].just = JUSTRIGHT;
		break;

	case CATG:
		strcpy(local_rec.l_prompt,"Category ");
		strcpy(local_rec.l_mask,"UUUUUUUUUUU");
		strcpy(local_rec.l_file,"excf");
		vars[0].just = JUSTLEFT;
		break;

	case A_CODE:
		strcpy(local_rec.l_prompt,"Area ");
		strcpy(local_rec.l_mask,"UU");
		strcpy(local_rec.l_file,"exaf");
		vars[0].just = JUSTRIGHT;
		break;

	case C_TYPE:
		strcpy(local_rec.l_prompt,"Class Type ");
		strcpy(local_rec.l_mask,"UUU");
		strcpy(local_rec.l_file,"excl");
		vars[0].just = JUSTLEFT;
		break;
	}

	init_scr();
	set_tty(); 
	set_masks();

	OpenDB();

	while (prog_exit == 0) 
	{
		new_lsabg = TRUE;
		new_csabg = TRUE;
		new_sabg = TRUE;
		entry_exit = 0;
		restart = 0;
		edit_exit = 0;
		prog_exit = 0;
		eoi_ok = 1;
		search_ok = 1;
		init_ok = 1;

		init_vars(1);	
		init_vars(2);	
		lcount[2] = 0;

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		if (new_sabg)
		{
			init_ok = 0;
			heading(2);
			scn_display(2);
			entry(2);
			if (prog_exit || restart)
				continue;
		}

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		heading(2);
		scn_display(2);
		edit(2);
		if (prog_exit || restart)
			continue;
		update();
	}
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
	open_rec("exaf",exaf_list,exaf_no_fields,"exaf_id_no");
	open_rec("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
	open_rec("excl",excl_list,excl_no_fields,"excl_id_no");
	open_rec("excf",excf_list,excf_no_fields,"excf_id_no");

	switch (rep_type)
	{
	case SALES:
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no2");
		abc_alias("csabg","sabg");
		open_rec("csabg",sabg_list,sabg_no_fields,"sabg_id_no2");
		break;

	case CATG:
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no4");
		abc_alias("csabg","sabg");
		open_rec("csabg",sabg_list,sabg_no_fields,"sabg_id_no4");
		break;

	case A_CODE:
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no1");
		abc_alias("csabg","sabg");
		open_rec("csabg",sabg_list,sabg_no_fields,"sabg_id_no1");
		break;

	case C_TYPE:
		open_rec("sabg",sabg_list,sabg_no_fields,"sabg_id_no3");
		abc_alias("csabg","sabg");
		open_rec("csabg",sabg_list,sabg_no_fields,"sabg_id_no3");
		break;
	}
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose("exaf");
	abc_fclose("exsf");
	abc_fclose("excl");
	abc_fclose("excf");
	abc_fclose("sabg");
	abc_fclose("csabg");
	abc_dbclose("data");
}

int
spec_valid (
 int    field)
{
	if (cur_screen == 2)
		strcpy(local_rec.l_month,months[line_cnt]);

	if (LCHECK("l_code"))
	{
		if (SRCH_KEY)
		{
			switch (rep_type)
			{
			case SALES:
				show_sale(temp_str);
				break;

			case CATG:
				show_category(temp_str);
				break;

			case A_CODE:
				show_area(temp_str);
				break;

			case C_TYPE:
				show_class(temp_str);
				break;
			}
			return(0);
		}
		return(check_input());
	}

	if (LCHECK("lsales"))
	{
		if (last_char == EOI)
		{
			entry_exit = 1;
			edit_exit = 1;
			if (prog_status != ENTRY || local_rec.l_sales != 0.00)
			{
				if (prog_status == ENTRY)
				{
					local_rec.l_sales = 0.00;
					local_rec.l_profit = 0.00;
					local_rec.c_sales = 0.00;
					local_rec.c_profit = 0.00;
				}
				putval(line_cnt);
			}
			lcount[2] = 12;
			line_cnt = 12;
		}
		return(0);
	}

	if (LCHECK("lprofit"))
	{
		if (last_char == EOI)
		{
			entry_exit = 1;
			edit_exit = 1;
			putval(line_cnt);
			lcount[2] = 12;
			line_cnt = 12;
		}
		return(0);
	}

	if (LCHECK("csales"))
	{
		if (last_char == EOI)
		{
			entry_exit = 1;
			edit_exit = 1;
			putval(line_cnt);
			lcount[2] = 12;
			line_cnt = 12;
		}
		return(0);
	}

	if (LCHECK("cprofit"))
	{
		if (last_char == EOI)
		{
			entry_exit = 1;
			edit_exit = 1;
			putval(line_cnt);
			lcount[2] = 12;
			line_cnt = 12;
		}
		return(0);
	}
    return (EXIT_SUCCESS);
}

int
check_input (void)
{
	char	error_str	[26];

	switch (rep_type)
	{
	case SALES:
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%2.2s",local_rec.l_code);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
		strcpy(lsabg_rec.bg_sman_code,exsf_rec.sf_salesman_no);
		strcpy(csabg_rec.bg_sman_code,exsf_rec.sf_salesman_no);
		strcpy(local_rec.l_desc,exsf_rec.sf_salesman);

		if ( cc )
			strcpy(error_str, ML(mlStdMess135) );
		break;

	case CATG:
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		sprintf(excf_rec.cf_categ_no,"%-11.11s",local_rec.l_code);
		cc = find_rec("excf",&excf_rec,COMPARISON,"r");
		strcpy(lsabg_rec.bg_category,excf_rec.cf_categ_no);
		strcpy(csabg_rec.bg_category,excf_rec.cf_categ_no);
		strcpy(local_rec.l_desc,excf_rec.cf_categ_description);

		if ( cc )
			strcpy(error_str, ML(mlStdMess004) );
		break;

	case A_CODE:
		strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
		sprintf(exaf_rec.af_area_code,"%2.2s",local_rec.l_code);
		cc = find_rec("exaf",&exaf_rec,COMPARISON,"r");
		strcpy(lsabg_rec.bg_area_code,exaf_rec.af_area_code);
		strcpy(csabg_rec.bg_area_code,exaf_rec.af_area_code);
		strcpy(local_rec.l_desc,exaf_rec.af_area);

		if ( cc )
			strcpy(error_str, ML(mlStdMess108) );
		break;

	case C_TYPE:
		strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
		sprintf(excl_rec.cl_class_type,"%-3.3s",local_rec.l_code);
		cc = find_rec("excl",&excl_rec,COMPARISON,"r");
		strcpy(lsabg_rec.bg_ctype_code,excl_rec.cl_class_type);
		strcpy(csabg_rec.bg_ctype_code,excl_rec.cl_class_type);
		strcpy(local_rec.l_desc,excl_rec.cl_class_description);

		if ( cc )
			strcpy(error_str, ML(mlSaMess039) );
		break;
	}

	if (cc == 999)
	{
		sprintf(err_str,"Error in %s During (DBFIND)",local_rec.l_file);
		sys_err(err_str,cc,PNAME);
	}

	if (cc)
	{
/*		sprintf(err_str," %s Not on File ",local_rec.l_code);*/
		print_mess(error_str);
		return(1);
	}

	if (prog_status != ENTRY)
		abc_unlock("sabg");
	strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
	strcpy(lsabg_rec.bg_year_flag,"L");
	new_lsabg = find_rec("sabg",&lsabg_rec,COMPARISON,"u");
	strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
	strcpy(csabg_rec.bg_year_flag,"C");
	new_csabg = find_rec("csabg",&csabg_rec,COMPARISON,"u");
	new_sabg = new_lsabg & new_csabg;
	if (new_sabg)
		load_month();
	else
		load_scn();
	return(0);
}

void
load_month (void)
{
	int	i;

	init_vars(2);
	init_ok = FALSE;

	for (i = 0;i < 12;i++,lcount[2]++)
	{
		strcpy(local_rec.l_month,months[i]);
		putval(i);
	}
	scn_set(1);
}

void
load_scn (void)
{
	int	i;

	scn_set(2);
	lcount[2] = 0;

	for (i = 0;i < 12;i++,lcount[2]++)
	{
		strcpy(local_rec.l_month,months[i]);
		local_rec.l_sales = (new_lsabg) ? 0.0 : lsabg_rec.bg_sales[i];
		local_rec.l_profit = (new_lsabg) ? 0.0 : lsabg_rec.bg_profit[i];
		local_rec.c_sales = (new_csabg) ? 0.0 : csabg_rec.bg_sales[i];
		local_rec.c_profit = (new_csabg) ? 0.0 : csabg_rec.bg_profit[i];
		putval(i);
	}
	scn_set(1);
}

void
update (void)
{
	int	i;

	for (i = 0;i < 12;i++)
	{
		getval(i);
		lsabg_rec.bg_sales[i]	= local_rec.l_sales;
		lsabg_rec.bg_profit[i]	= local_rec.l_profit;
		csabg_rec.bg_sales[i]	= local_rec.c_sales;
		csabg_rec.bg_profit[i]	= local_rec.c_profit;
	}

	if (new_lsabg)
	{
		switch (rep_type)
		{
		case SALES:
			strcpy(lsabg_rec.bg_area_code,"  ");
			sprintf(lsabg_rec.bg_sman_code,"%-2.2s",local_rec.l_code);
			strcpy(lsabg_rec.bg_ctype_code,"   ");
			strcpy(lsabg_rec.bg_category,"           ");
			break;

		case CATG:
			strcpy(lsabg_rec.bg_area_code,"  ");
			strcpy(lsabg_rec.bg_sman_code,"  ");
			strcpy(lsabg_rec.bg_ctype_code,"   ");
			sprintf(lsabg_rec.bg_category,"%-11.11s",local_rec.l_code);
			break;

		case A_CODE:
			sprintf(lsabg_rec.bg_area_code,"%-2.2s",local_rec.l_code);
			strcpy(lsabg_rec.bg_sman_code,"  ");
			strcpy(lsabg_rec.bg_ctype_code,"   ");
			strcpy(lsabg_rec.bg_category,"           ");
			break;

		case C_TYPE:
			strcpy(lsabg_rec.bg_area_code,"  ");
			strcpy(lsabg_rec.bg_sman_code,"  ");
			sprintf(lsabg_rec.bg_ctype_code,"%-3.3s",local_rec.l_code);
			strcpy(lsabg_rec.bg_category,"           ");
			break;

		}
		strcpy(lsabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(lsabg_rec.bg_year_flag,"L");
		cc = abc_add("sabg",&lsabg_rec);
		if (cc)
			sys_err("Error in lsabg During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update("sabg",&lsabg_rec);
		if (cc)
			sys_err("Error in lsabg During (DBUPDATE)",cc,PNAME);
		abc_unlock("sabg");
	}

	if (new_csabg)
	{
		switch (rep_type)
		{
		case SALES:
			strcpy(csabg_rec.bg_area_code,"  ");
			sprintf(csabg_rec.bg_sman_code,"%-2.2s",local_rec.l_code);
			strcpy(csabg_rec.bg_ctype_code,"   ");
			strcpy(csabg_rec.bg_category,"           ");
			break;

		case CATG:
			strcpy(csabg_rec.bg_area_code,"  ");
			strcpy(csabg_rec.bg_sman_code,"  ");
			strcpy(csabg_rec.bg_ctype_code,"   ");
			sprintf(csabg_rec.bg_category,"%-11.11s",local_rec.l_code);
			break;

		case A_CODE:
			sprintf(csabg_rec.bg_area_code,"%-2.2s",local_rec.l_code);
			strcpy(csabg_rec.bg_sman_code,"  ");
			strcpy(csabg_rec.bg_ctype_code,"   ");
			strcpy(csabg_rec.bg_category,"           ");
			break;

		case C_TYPE:
			strcpy(csabg_rec.bg_area_code,"  ");
			strcpy(csabg_rec.bg_sman_code,"  ");
			sprintf(csabg_rec.bg_ctype_code,"%-3.3s",local_rec.l_code);
			strcpy(csabg_rec.bg_category,"           ");
			break;

		}
		strcpy(csabg_rec.bg_co_no,comm_rec.tco_no);
		strcpy(csabg_rec.bg_year_flag,"C");
		cc = abc_add("csabg",&csabg_rec);
		if (cc)
			sys_err("Error in csabg During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update("csabg",&csabg_rec);
		if (cc)
			sys_err("Error in csabg During (DBUPDATE)",cc,PNAME);
		abc_unlock("csabg");
	}
}

/*=======================
| Search for area code	|
=======================*/
void
show_area (
 char*  key_val)
{
	work_open();
	save_rec("#Ar","#Area Description");
	strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
	sprintf(exaf_rec.af_area_code,"%-2.2s",key_val);
	cc = find_rec("exaf",&exaf_rec,GTEQ,"r");
	while (!cc && !strncmp(exaf_rec.af_area_code,key_val,strlen(key_val)) &&
		      !strcmp(exaf_rec.af_co_no,comm_rec.tco_no))
	{
		cc = save_rec(exaf_rec.af_area_code,exaf_rec.af_area);
		if (cc)
			break;
		cc = find_rec("exaf",&exaf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
	sprintf(exaf_rec.af_area_code,"%-2.2s",temp_str);
	cc = find_rec("exaf",&exaf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in exaf During (DBFIND)",cc,PNAME);
	strcpy(local_rec.l_desc,exaf_rec.af_area);
}

/*===============================
| Search for salesman code	|
===============================*/
void
show_sale (
 char*  key_val)
{
	work_open();
	save_rec("#Sm","#Salesman Description");
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");
	while (!cc && 
		!strncmp(exsf_rec.sf_salesman_no,key_val,strlen(key_val)) && 
		!strcmp(exsf_rec.sf_co_no,comm_rec.tco_no))
	{
		cc = save_rec(exsf_rec.sf_salesman_no,exsf_rec.sf_salesman);
		if (cc)
			break;
		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in exsf During (DBFIND)",cc,PNAME);
	strcpy(local_rec.l_desc,exsf_rec.sf_salesman);
}

/*=======================
| Search for class type	|
=======================*/
void
show_class (
 char*  key_val)
{
	work_open();
	save_rec("#Cla","#Class Description");
	strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
	sprintf(excl_rec.cl_class_type,"%-3.3s",key_val);
	cc = find_rec("excl",&excl_rec,GTEQ,"r");
	while (!cc && 
		!strncmp(excl_rec.cl_class_type,key_val,strlen(key_val)) && 
	        !strcmp(excl_rec.cl_co_no,comm_rec.tco_no))
	{
		cc = save_rec(excl_rec.cl_class_type,excl_rec.cl_class_description);
		if (cc)
			break;
		cc = find_rec("excl",&excl_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
	sprintf(excl_rec.cl_class_type,"%-3.3s",temp_str);
	cc = find_rec("excl",&excl_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in excl During (DBFIND)",cc,PNAME);
	strcpy(local_rec.l_desc,excl_rec.cl_class_description);
}

/*=======================
| Search for category	|
=======================*/
void
show_category (
 char*  key_val)
{
	work_open();
	save_rec("#Category","#Category Description");
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no,"%-16.16s",key_val);
	cc = find_rec("excf",&excf_rec,GTEQ,"r");
	while (!cc && !strncmp(excf_rec.cf_categ_no,key_val,strlen(key_val)) &&
		      !strcmp(excf_rec.cf_co_no,comm_rec.tco_no))
	{
		cc = save_rec(excf_rec.cf_categ_no,excf_rec.cf_categ_description);
		if (cc)
			break;
		cc = find_rec("excf",&excf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no,"%-16.16s",temp_str);
	cc = find_rec("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in excf During (DBFIND)",cc,PNAME);
	strcpy(local_rec.l_desc,excf_rec.cf_categ_description);
}

int
heading (
 int    scn)
{
	if (scn != cur_screen)
		scn_set(scn);

	clear();
	switch ( rep_type )
	{
		case	SALES :
			sprintf (err_str, " %s ", ML(mlSaMess023) );
			break;
		case 	CATG :
			sprintf (err_str, " %s ", ML(mlSaMess024) );
			break;
		case 	A_CODE:
			sprintf (err_str, " %s ", ML(mlSaMess025) );
			break;
		case C_TYPE:
			sprintf (err_str, " %s ", ML(mlSaMess026) );
			break;
		default :
			break;
	}

	rv_pr(err_str,20,0,1);

	if (scn == 1)
		box(0,3,80,1);
	else
	{
		print_at(2,20,"%s %s",local_rec.l_prompt,local_rec.l_desc);
		/*"Last Years Budget"*/
		us_pr(ML(mlSaMess027), 20,4,1);
		/*"Current Years Budget"*/
		us_pr(ML(mlSaMess028), 50,4,1);
	}
	move(0,1);
	line(80);
	move(0,21); 
	line(80);
	sprintf (err_str, ML(mlStdMess038),comm_rec.tco_no,clip(comm_rec.tco_short));
	print_at(22,0, "%s", err_str);
	print_at(22,30,ML(mlStdMess039),comm_rec.tes_no,clip(comm_rec.tes_short));
	print_at(22,58,ML(mlStdMess099),comm_rec.tcc_no,clip(comm_rec.tcc_short));
	scn_write(scn);
    return (EXIT_SUCCESS);
}

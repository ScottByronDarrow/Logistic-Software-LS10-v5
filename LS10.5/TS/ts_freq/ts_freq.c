/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_freq.c      )                                 |
|  Program Desc  : ( Determine phone frequency for active         )   |
|                  ( telesales debtors.                           )   |
|---------------------------------------------------------------------|
|  Access files  :  tspm, cumr, exsf, exaf, excl,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  tspm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 04/12/91         |
|---------------------------------------------------------------------|
|  Date Modified : (06/09/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                :                                                    |
|                :                                                    |
|                                                                     |
|  Comments      : (  /  /  ) -                                       |
|   (06/09/93)   : HGP 9745. Updated for post_code.                   |
|   (04/09/97)   : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_freq.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_freq/ts_freq.c,v 5.4 2002/03/01 03:50:15 scott Exp $";

#include <ml_ts_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	envDbCo = 0,
			wk_no,
			envDbFind = 0;

	char	branchNo[3];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"}
	};

	int comm_no_fields = 11;
	
	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	test_no[3];
		char 	test_name[41];
		long 	t_dbt_date;
		char 	price_desc[5][16];
	} comm_rec;
		
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
		{"cumr_acc_type"},
		{"cumr_stmt_type"},
		{"cumr_class_type"},
		{"cumr_cont_type"},
		{"cumr_price_type"},
		{"cumr_payment_flag"},
		{"cumr_int_flag"},
		{"cumr_bo_flag"},
		{"cumr_bo_cons"},
		{"cumr_bo_days"},
		{"cumr_po_flag"},
		{"cumr_sur_flag"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_dl_adr1"},
		{"cumr_dl_adr2"},
		{"cumr_dl_adr3"},
		{"cumr_contact_name"},
		{"cumr_telex"},
		{"cumr_stop_credit"},
		{"cumr_date_stop"},
		{"cumr_total_days_sc"},
		{"cumr_credit_limit"},
		{"cumr_crd_prd"},
		{"cumr_credit_ref"},
		{"cumr_bank_code"},
		{"cumr_branch_code"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_roy_type"},
		{"cumr_disc_code"},
		{"cumr_tax_code"},
		{"cumr_tax_no"},
		{"cumr_ch_to_ho_flg"},
		{"cumr_ho_dbt_hash"},
		{"cumr_cat_sa_flag"},
		{"cumr_stmnt_flg"},
		{"cumr_freight_chg"},
		{"cumr_restock_fee"},
		{"cumr_nett_pri_prt"},
		{"cumr_inst_fg1"},
		{"cumr_inst_fg2"},
		{"cumr_inst_fg3"},
		{"cumr_date_lastinv"},
		{"cumr_date_lastpay"},
		{"cumr_amt_lastpay"},
		{"cumr_mtd_sales"},
		{"cumr_ytd_sales"},
		{"cumr_ord_value"},
		{"cumr_bo_current"},
		{"cumr_bo_per1"},
		{"cumr_bo_per2"},
		{"cumr_bo_per3"},
		{"cumr_bo_per4"},
		{"cumr_bo_fwd"},
		{"cumr_od_flag"},
		{"cumr_stat_flag"},
	};

	int		cumr_no_fields = 65;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_acc_type[2];
		char	cm_stmt_type[2];
		char	cm_class_type[4];
		char	cm_cont_type[4];
		char	cm_price_type[2];
		int		cm_payment_flag;
		char	cm_int_flag[2];
		char	cm_bo_flag[2];
		char	cm_bo_cons[2];
		int		cm_bo_days;
		char	cm_po_flag[2];
		char	cm_sur_flag[2];
		char	cm_ch_adr[3][41];
		char	cm_dl_adr[3][41];
		char	cm_contact_name[21];
		char	cm_telex[11];
		char	cm_stop_credit[2];
		long	cm_date_stop;
		int		cm_total_days_sc;
		double	cm_credit_limit;	/* money */
		char	cm_crd_prd[4];
		char	cm_credit_ref[21];
		char	cm_bank_code[4];
		char	cm_branch_code[21];
		char	cm_area_code[3];
		char	cm_sman_code[3];
		char	cm_roy_type[4];
		char	cm_disc_code[2];
		char	cm_tax_code[2];
		char	cm_tax_no[16];
		char	cm_ch_to_ho_flg[2];
		long	cm_ho_dbt_hash;
		char	cm_cat_sa_flag[2];
		char	cm_stmnt_flg[2];
		char	cm_freight_chg[2];
		char	cm_restock_fee[2];
		char	cm_nett_pri_prt[2];
		int		cm_inst_fg1;
		int		cm_inst_fg2;
		int		cm_inst_fg3;
		long	cm_date_lastinv;
		long	cm_date_lastpay;
		double	cm_amt_lastpay;	/* money */
		double	cm_mtd_sales;	/* money */
		double	cm_ytd_sales;	/* money */
		double	cm_ord_value;	/* money */
		double	cm_bo_current;	/* money */
		double	cm_bo_per1;	/* money */
		double	cm_bo_per2;	/* money */
		double	cm_bo_per3;	/* money */
		double	cm_bo_per4;	/* money */
		double	cm_bo_fwd;	/* money */
		int		cm_od_flag;
		char	cm_stat_flag[2];
	} cumr_rec;

	/*=================================
	| Tele-Sales Prospect Master file |
	=================================*/
	struct dbview tspm_list[] ={
		{"tspm_hhcu_hash"},
		{"tspm_phone_freq"},
	};

	int		tspm_no_fields = 2;

	struct	{
		long	pm_hhcu_hash;
		int		pm_phone_freq;
	} tspm_rec;

	/*================================
	| Debtors Class Type Master File |
	================================*/
	struct dbview excl_list[] ={
		{"excl_co_no"},
		{"excl_class_type"},
		{"excl_class_desc"},
		{"excl_stat_flag"},
	};

	int		excl_no_fields = 4;

	struct	{
		char	cl_co_no[3];
		char	cl_class[4];
		char	cl_class_desc[41];
		char	cl_stat_flag[2];
	} excl_rec;

	/*========================
	| External Salesman File |
	========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int		exsf_no_fields = 3;

	struct	{
		char	sf_co_no[3];
		char	sf_sman[3];
		char	sf_sman_name[41];
	} exsf_rec;

	/*====================
	| External Area file |
	====================*/
	struct dbview exaf_list[] ={
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
	};

	int		exaf_no_fields = 3;

	struct	{
		char	af_co_no[3];
		char	af_area_code[3];
		char	af_area[41];
	} exaf_rec;

	char	*comm = "comm",
		*tspm = "tspm",
		*cumr = "cumr",
		*exaf = "exaf",
		*exsf = "exsf",
		*excl = "excl";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	char 	st_cust[7];
	char 	st_name[41];
	char 	end_cust[7];
	char 	end_name[41];
	char 	st_type[4];
	char 	st_type_name[41];
	char 	end_type[4];
	char 	end_type_name[41];
	char 	st_area[3];
	char 	st_area_name[41];
	char 	end_area[3];
	char 	end_area_name[41];
	char 	st_sman[3];
	char 	st_sman_name[41];
	char 	end_sman[3];
	char 	end_sman_name[41];
	long	n_date;
	char	n_time[6];
	int		freq;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_cust",	 4, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer  ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.st_cust},
	{1, LIN, "st_name",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_name},
	{1, LIN, "end_cust",	 5, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End Customer    ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.end_cust},
	{1, LIN, "end_name",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_name},

	{1, LIN, "st_type",	 7, 15, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Start Type      ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.st_type},
	{1, LIN, "st_type_name",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_type_name},
	{1, LIN, "end_type",	 8, 15, CHARTYPE,
		"UUU", "          ",
		" ", " ", "End Type        ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.end_type},
	{1, LIN, "end_type_name",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_type_name},

	{1, LIN, "st_area",	 10, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Area      ", " ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.st_area},
	{1, LIN, "st_area_name",	 10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_area_name},
	{1, LIN, "end_area",	 11, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "End Area        ", " ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.end_area},
	{1, LIN, "end_area_name", 11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_area_name},

	{1, LIN, "st_sman",	 13, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Salesman  ", " ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.st_sman},
	{1, LIN, "st_sman_name",	 13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_sman_name},
	{1, LIN, "end_sman",	 14, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "End Salesman    ", " ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.end_sman},
	{1, LIN, "end_sman_name", 14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_sman_name},

	{1, LIN, "freq",	 16, 15, INTTYPE,
		"NN", "          ",
		" ", "7", "Phone Frequency "," Enter Phone Frequency In Days ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&local_rec.freq},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindCumr.h>
#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void SrchExaf (char *key_val);
void srch_type (char *key_val);
void srch_sman (char *key_val);
int set_freq (void);
int valid_cust (void);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv[])
{
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();
	
	OpenDB(); 	

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	init_vars(1);

	strcpy(branchNo, (envDbCo) ? comm_rec.test_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit(1);
		if (restart)
			continue;

		set_freq();
		prog_exit = TRUE;
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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

	open_rec(tspm, tspm_list, tspm_no_fields, "tspm_hhcu_hash");
	open_rec(cumr, cumr_list, cumr_no_fields, (envDbFind) ? "cumr_id_no3" 
							      				        : "cumr_id_no");
	open_rec(exaf, exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec(exsf, exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec(excl, excl_list, excl_no_fields, "excl_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(tspm);
	abc_fclose(cumr);
	abc_fclose(exaf);
	abc_fclose(exsf);
	abc_fclose(excl);
	abc_dbclose("data");
}

int
spec_valid (
 int    field)
{
	if (LCHECK("st_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_cust, "      ");
			sprintf(local_rec.st_name, "%-40.40s","First Customer");
			DSP_FLD("st_name");
			return(0);
		}
	
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return(0);
		}
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNo);
		sprintf(cumr_rec.cm_dbt_no,"%-6.6s",pad_num(local_rec.st_cust));
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(2);	
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_name, "%-40.40s", cumr_rec.cm_name);
		DSP_FLD("st_name");

		return(0);
	}

	if (LCHECK("end_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_cust, "~~~~~~");
			sprintf(local_rec.end_name, "%-40.40s","Last Customer");
			DSP_FLD("end_name");
			return(0);
		}
	
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNo);
		sprintf(cumr_rec.cm_dbt_no, "%-6.6s",pad_num(local_rec.end_cust));
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(2);	
			clear_mess();
			return(1);
		}
		if (strcmp (local_rec.st_cust,local_rec.end_cust) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf(local_rec.end_name, "%-40.40s", cumr_rec.cm_name);
		DSP_FLD("end_name");

		return(0);
	}

	if (LCHECK("st_type"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_type, "   ");
			sprintf(local_rec.st_type_name, "%-40.40s","First Customer Type ");
			DSP_FLD("st_type_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_type(temp_str);
			return(0);
		}

		strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
		sprintf(excl_rec.cl_class, "%3.3s", local_rec.st_type);
		cc = find_rec("excl", &excl_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess(ML(mlStdMess227));
			sleep(2);	
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_type_name, "%-40.40s", excl_rec.cl_class_desc);
		DSP_FLD("st_type_name");

		return(0);
	}

	if (LCHECK("end_type"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_type, "~~~");
			sprintf(local_rec.end_type_name, "%-40.40s","Last Customer Type ");
			DSP_FLD("end_type_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_type(temp_str);
			return(0);
		}

		strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
		sprintf(excl_rec.cl_class, "%3.3s", local_rec.end_type);
		cc = find_rec("excl", &excl_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess(ML ("Type Does Not Exist On File "));
			sleep(2);	
			clear_mess();
			return(1);
		}

		if (strcmp (local_rec.st_type,local_rec.end_type) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf(local_rec.end_type_name, "%-40.40s", excl_rec.cl_class_desc);
		DSP_FLD("end_type_name");

		return(0);
	}

	if (LCHECK("st_area"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_area, "  ");
			sprintf(local_rec.st_area_name, "%-40.40s", "First Area");
			DSP_FLD("st_area_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExaf(temp_str);
			return(0);
		}

		strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
		sprintf(exaf_rec.af_area_code, "%2.2s", local_rec.st_area);
		cc = find_rec("exaf", &exaf_rec, COMPARISON, "r");
		if (cc) 
		{
			/*print_mess("\007 Area Does Not Exist On File ");*/
			print_mess(ML(mlStdMess108));
			sleep(2);	
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_area_name, "%-40.40s", exaf_rec.af_area);
		DSP_FLD("st_area_name");

		return(0);
	}

	if (LCHECK("end_area"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_area, "~~");
			sprintf(local_rec.end_area_name, "%-40.40s", "Last Area");
			DSP_FLD("end_area_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExaf(temp_str);
			return(0);
		}

		strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
		sprintf(exaf_rec.af_area_code, "%2.2s", local_rec.end_area);
		cc = find_rec("exaf", &exaf_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess(ML(mlStdMess108));
			sleep(2);	
			clear_mess();
			return(1);
		}

		if (strcmp (local_rec.st_area,local_rec.end_area) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf(local_rec.end_area_name, "%-40.40s", exaf_rec.af_area);
		DSP_FLD("end_area_name");

		return(0);
	}

	if (LCHECK("st_sman"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_sman, "  ");
			sprintf(local_rec.st_sman_name, "%-40.40s", "First Salesman");
			DSP_FLD("st_sman_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_sman(temp_str);
			return(0);
		}
	
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_sman, "%2.2s", local_rec.st_sman);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess(ML(mlStdMess135));
			sleep(2);	
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_sman_name, "%-40.40s", exsf_rec.sf_sman_name);
		DSP_FLD("st_sman_name");

		return(0);
	}

	if (LCHECK("end_sman"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_sman, "~~");
			sprintf(local_rec.end_sman_name, "%-40.40s", "Last Salesman");
			DSP_FLD("end_sman_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_sman(temp_str);
			return(0);
		}
	
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_sman, "%2.2s", local_rec.end_sman);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess(ML(mlStdMess135));
			sleep(2);	
			clear_mess();
			return(1);
		}

		if (strcmp (local_rec.st_sman,local_rec.end_sman) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf(local_rec.end_sman_name, "%-40.40s", exsf_rec.sf_sman_name);
		DSP_FLD("end_sman_name");

		return(0);
	}

	return(0);
}	

void
SrchExaf (
 char*  key_val)
{
	_work_open (2,0,40);

	save_rec("#No","#Area Description");
	strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
	sprintf(exaf_rec.af_area_code,"%-2.2s",key_val);
	cc = find_rec("exaf",&exaf_rec,GTEQ,"r");

	while (!cc && !strcmp(exaf_rec.af_co_no,comm_rec.tco_no) &&
		      !strncmp(exaf_rec.af_area_code,key_val,strlen(key_val)))
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
		sys_err("Error in exaf during (DBFIND)",cc,PNAME);
}

void
srch_type (
 char*  key_val)
{
	_work_open (3,0,40);
	save_rec("#No","#Class Description");
	strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
	sprintf(excl_rec.cl_class, "%-3.3s", key_val);
	cc = find_rec("excl",&excl_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp(excl_rec.cl_co_no,comm_rec.tco_no) &&
	       !strncmp(excl_rec.cl_class, key_val, strlen(key_val)))
	{
		cc = save_rec(excl_rec.cl_class, excl_rec.cl_class_desc);
		if (cc)
			break;

		cc = find_rec("excl",&excl_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
	sprintf(excl_rec.cl_class, "%-3.3s", temp_str);
	cc = find_rec("excl",&excl_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in excl during (DBFIND)",cc,PNAME);
}

void
srch_sman (
 char*  key_val)
{
	_work_open (2,0,40);
	save_rec("#No","#Salesman's Name");
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_sman,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp(exsf_rec.sf_co_no,comm_rec.tco_no) && 
	       !strncmp(exsf_rec.sf_sman,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_sman,exsf_rec.sf_sman_name);
		if (cc)
			break;

		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_sman,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in exsf during (DBFIND)",cc,PNAME);
}

/*----------------------
| Create tspm records. |
----------------------*/
int
set_freq (void)
{
	clear();

	abc_selfield(cumr, "cumr_id_no3");

	dsp_screen("Activating Customer", comm_rec.tco_no, comm_rec.tco_name);

	strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", local_rec.st_cust);
	cc = find_rec(cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp(cumr_rec.cm_co_no, comm_rec.tco_no) &&
	               strcmp(cumr_rec.cm_dbt_no, local_rec.end_cust) <= 0)
	{
		if (!valid_cust())
		{
			cc = find_rec(cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		dsp_process("Customer :", cumr_rec.cm_dbt_no);
		cc = find_hash(tspm, &tspm_rec, COMPARISON, "u", 
			       cumr_rec.cm_hhcu_hash);
		if (cc)
		{
			cc = find_rec(cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		tspm_rec.pm_phone_freq = local_rec.freq;

		cc = abc_update(tspm, &tspm_rec);
		if (cc)
			file_err( cc, "tspm", "DBUPDATE" );

		cc = find_rec(cumr, &cumr_rec, NEXT, "r");
	}


	return(0);
}

int
valid_cust (void)
{
	if (strcmp(cumr_rec.cm_area_code, local_rec.st_area) < 0 ||
	    strcmp(cumr_rec.cm_area_code, local_rec.end_area) > 0)
		return(FALSE);

	if (strcmp(cumr_rec.cm_class_type, local_rec.st_type) < 0 ||
	    strcmp(cumr_rec.cm_class_type, local_rec.end_type) > 0)
		return(FALSE);

	if (strcmp(cumr_rec.cm_sman_code, local_rec.st_sman) < 0 ||
	    strcmp(cumr_rec.cm_sman_code, local_rec.end_sman) > 0)
		return(FALSE);

	return(TRUE);
}

int
heading(int scn)
{
	if ( restart )
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set(scn);

	clear();

	box (0,3,80,13);
	move(1,6);
	line(79);

	move(1,9);
	line(79);

	move(1,12);
	line(79);

	move(1,15);
	line(79);

	rv_pr(ML(mlTsMess056),20,0,1);

	move(0,1);
	line(80);

	move(0,20);
	line(80);
	strcpy(err_str,ML(mlStdMess038));
	print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write(scn);
    return (EXIT_SUCCESS);
}


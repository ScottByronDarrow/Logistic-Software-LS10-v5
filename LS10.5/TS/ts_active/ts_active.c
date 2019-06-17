/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_active.c    )                                 |
|  Program Desc  : ( Activate a debtor for telesales by creating  )   |
|                  ( a tspm record.                               )   |
|---------------------------------------------------------------------|
|  Access files  :  tmpm, cumr, cumd,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  tmpm, cumr, cumd,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 07/08/91         |
|---------------------------------------------------------------------|
|  Date Modified : (30/12/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (23/09/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (02/10/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (03/08/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (30/08/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : (30/12/91) - Add code to allow passing of hhcu_hash|
|                : to program.                                        |
|                :                                                    |
|  (23/09/92)    : Change Next Phone Time to Best Phone Time.         |
|                : SC 7639 DPL.                                       |
|                :                                                    |
|  (02/10/92)    : Fix for SC 7514 DPL. Set tspm_sales_per flag.      |
|                :                                                    |
|  (03/08/93)    : HGP 9745. Updated for post code.                   |
|                :                                                    |
|  (30/08/94)    : PSL 10975 - Prevent activation of fgn curr debtors |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_active.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_active/ts_active.c,v 5.2 2001/08/09 09:23:18 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_ts_mess.h>

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int  	envDbCo = 0,
			wk_no,
			envDbFind = 0;

	int		MCURR;

	char	branchNo[3];
	char	Curr_code[5];

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
		{"cumr_curr_code"},
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
		{"cumr_post_code"},
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

	int		cumr_no_fields = 67;

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
		char	cm_curr_code[4];
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
		char	cm_post_code[11];
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
		{"tspm_cont_name1"},
		{"tspm_cont_name2"},
		{"tspm_cont_code1"},
		{"tspm_cont_code2"},
		{"tspm_phone_freq"},
		{"tspm_n_phone_date"},
		{"tspm_n_phone_time"},
		{"tspm_visit_freq"},
		{"tspm_n_visit_date"},
		{"tspm_n_visit_time"},
		{"tspm_mail_flag"},
		{"tspm_op_code"},
		{"tspm_lst_op_code"},
		{"tspm_lphone_date"},
		{"tspm_date_create"},
		{"tspm_best_ph_time"},
		{"tspm_delete_flag"},
		{"tspm_sales_per"},
		{"tspm_stat_flag"},
	};

	int		tspm_no_fields = 20;

	struct	{
		long	pm_hhcu_hash;
		char	pm_cont_name[2][31];
		char	pm_cont_code[2][4];
		int		pm_phone_freq;
		long	pm_n_phone_date;
		char	pm_n_phone_time[6];
		int		pm_visit_freq;
		long	pm_n_visit_date;
		char	pm_n_visit_time[6];
		char	pm_mail_flag[2];
		char	pm_op_code[15];
		char	pm_lst_op_code[15];
		long	pm_lphone_date;
		long	pm_date_create;
		char	pm_best_ph_time[6];
		char	pm_delete_flag[2];
		char	pm_sales_per[2];
		char	pm_stat_flag[2];
	} tspm_rec;

	char	*comm  = "comm",
		*data  = "data",
		*tspm  = "tspm",
		*cumr  = "cumr",
		*cumr2 = "cumr2";

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
	long	n_date;
	char	b_time[6];
	int		freq;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_cust",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer :", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.st_cust},
	{1, LIN, "st_name",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_name},
	{1, LIN, "end_cust",	 5, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End Customer   :", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.end_cust},
	{1, LIN, "end_name",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_name},

	{1, LIN, "freq",	 7, 20, INTTYPE,
		"NN", "          ",
		" ", "7", "Phone Frequency :"," Enter Phone Frequency In Days ",
		 NO, NO,  JUSTRIGHT, "0", "99", (char *)&local_rec.freq},
	{1, LIN, "n_date", 8, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Next Phone Date :", " ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&local_rec.n_date},
	{1, LIN, "b_time", 9, 20, CHARTYPE,
		"AA:AA", "          ",
		" ", " ", "Best Phone Time :", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.b_time},

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
void    shutdown_prog (void);
void    OpenDB (void);
void    CloseDB (void);
int     spec_valid (int field);
int     proc_range (void);
void    activate (void);
int     heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	sprintf(Curr_code, "%-3.3s", get_env("CURR_CODE"));

	/*-------------------------
	| Multi-currency debtors. |
	-------------------------*/
	sptr = chk_env( "DB_MCURR" );
	MCURR = ( sptr == ( char *)0 ) ? FALSE : atoi( sptr );

	OpenDB(); 	
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec); 	

	if (argc == 2)
	{
		cc = find_hash(cumr2, &cumr_rec, COMPARISON,"r", atol(argv[1]));
		if (!cc)
			activate();
	}
	else
	{
		SETUP_SCR (vars);

		init_scr();
		set_tty();
		set_masks();
	
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
			heading (1);
			entry(1);
			if (prog_exit || restart)
				continue;
	
			heading (1);
			scn_display (1);
			edit(1);
			if (restart)
				continue;
	
			proc_range();
			prog_exit = TRUE;
		}
	}
	shutdown_prog ();
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
	abc_dbopen(data);

	abc_alias(cumr2, cumr);

	open_rec(tspm, tspm_list, tspm_no_fields, "tspm_hhcu_hash");
	open_rec(cumr, cumr_list, cumr_no_fields, (envDbFind) ? "cumr_id_no3" 
							      : "cumr_id_no");
	open_rec(cumr2, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(tspm);
	abc_fclose(cumr);
	abc_fclose(cumr2);
	abc_dbclose(data);
}

int
spec_valid (
 int    field)
{
	int		i;
	int		tmp_hour, 
			tmp_min;
	char	tmp_time[6];
	char	*tptr;

	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
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

		if (strcmp (cumr_rec.cm_curr_code, Curr_code))
		{
			print_mess(ML(mlTsMess008));
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

		if (strcmp (cumr_rec.cm_curr_code, Curr_code))
		{
			print_mess(ML(mlTsMess008));
			sleep(2);	
			clear_mess();
			return(1);
		}

		sprintf(local_rec.end_name, "%-40.40s", cumr_rec.cm_name);
		DSP_FLD("end_name");

		return(0);
	}

	if (LCHECK("n_date"))
	{
		if (dflt_used)
		{
			local_rec.n_date = 0L;
			return(0);
		}

		if (local_rec.n_date < local_rec.lsystemDate)
		{
			/*print_mess("\007 Date Must Be Greater Than Today ");*/
			print_mess(ML(mlTsMess009));
			sleep(2);
			clear_mess();
			return(1);
		}

		return(0);
	}

	if (LCHECK("b_time"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.b_time, "00:00");
			DSP_FLD("b_time");
			return(0);
		}

		/*---------------------------
		| Replace spaces with zeros |
		---------------------------*/
		sprintf(tmp_time, "%-5.5s", local_rec.b_time);
		tptr = tmp_time;
		i = 0;
		while (*tptr)
		{
			if (*tptr == ' ' && i != 2)
				*tptr = '0';

			i++;
			tptr++;
		}
		sprintf(local_rec.b_time, "%-5.5s", tmp_time);
		local_rec.b_time[2] = ':';

		tmp_hour = atoi(local_rec.b_time);
		tmp_min = atoi(local_rec.b_time + 3);

		if (tmp_hour > 23 || tmp_min > 59)
		{
			/*print_mess("\007 Invalid Time ");*/
			print_mess(ML(mlStdMess142));
			sleep(2);
			clear_mess();

			return(1);
		}

		DSP_FLD("b_time");
		return(0);
	}

	return(0);
}	

/*----------------------
| Create tspm records. |
----------------------*/
int
proc_range (void)
{
	clear();

	abc_selfield(cumr, "cumr_id_no3");

	dsp_screen("Activating Customers", comm_rec.tco_no, comm_rec.tco_name);

	strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", local_rec.st_cust);
	cc = find_rec(cumr, &cumr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cumr_rec.cm_co_no, comm_rec.tco_no) &&
	       strcmp(cumr_rec.cm_dbt_no, local_rec.end_cust) <= 0)
	{
		dsp_process("Customer :", cumr_rec.cm_dbt_no);
		cc = find_hash(tspm, &tspm_rec, COMPARISON, "r", 
			       cumr_rec.cm_hhcu_hash);
		if (!cc)
		{
			cc = find_rec(cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		activate();

		cc = find_rec(cumr, &cumr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------------------
| Activate debtor by adding a tspm record |
-----------------------------------------*/
void
activate (void)
{
	if (strcmp (cumr_rec.cm_curr_code, Curr_code))
		return;

	tspm_rec.pm_hhcu_hash = cumr_rec.cm_hhcu_hash;
	sprintf(tspm_rec.pm_cont_name[0], 
		"%-20.20s", 
		cumr_rec.cm_contact_name);
	sprintf(tspm_rec.pm_cont_name[1], "%-20.20s", " ");
	strcpy(tspm_rec.pm_cont_code[0], "   ");
	tspm_rec.pm_phone_freq = local_rec.freq;
	tspm_rec.pm_n_phone_date = local_rec.n_date;
	strcpy(tspm_rec.pm_n_phone_time, "12:00");
	tspm_rec.pm_visit_freq = 0;
	tspm_rec.pm_n_visit_date = 0L;
	strcpy(tspm_rec.pm_n_visit_time, "00:00");
	strcpy(tspm_rec.pm_mail_flag, "N");
	sprintf(tspm_rec.pm_op_code, "%-14.14s", " ");
	sprintf(tspm_rec.pm_lst_op_code, "%-14.14s", " ");
	tspm_rec.pm_lphone_date = 0L;
	tspm_rec.pm_date_create = local_rec.lsystemDate;
	sprintf(tspm_rec.pm_best_ph_time, "%-5.5s", local_rec.b_time);
	strcpy(tspm_rec.pm_delete_flag, "N");
	strcpy(tspm_rec.pm_stat_flag, "0");
	if (tspm_rec.pm_phone_freq == 1)
		strcpy(tspm_rec.pm_sales_per, "D");
	else
	{
		if (tspm_rec.pm_phone_freq > 1 && tspm_rec.pm_phone_freq <= 13)
			strcpy(tspm_rec.pm_sales_per, "W");
		else
			strcpy(tspm_rec.pm_sales_per, "M");
	}

	cc = abc_add(tspm, &tspm_rec);
	if (cc)
		file_err( cc, "tspm", "DBADD" );
}

int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set(scn);

	clear();

	box (0,3,80,6);
	move(1,6);
	line(79);

	rv_pr(ML(mlTsMess001),25,0,1);

	move(0,1);
	line(80);

	move(0,20);
	line(80);
	print_at(21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write(scn);
    return (EXIT_SUCCESS);
}

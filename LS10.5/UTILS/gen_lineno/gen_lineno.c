/*=====================================================================
|  Copyright (C) 1988 Logistic Software Limited.                      |
|=====================================================================|  
|  Program Name  : ( gen_lineno.c  )                                  |
|  Program Desc  : ( Data Conversion Utility                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : arln , coln, poln, soln, arhr, cohr, pohr, sohr    |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Updates Files : arln , coln, poln, soln,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Date Written  : (24/04/2000)    | Author      : Jinno Crisostomo   |
|---------------------------------------------------------------------|
|                                                                     |
|  Date Modified : (DD/MM/YYYY)    | Modified  by :                   |
|   Comments     :                                                    |
|                :                                                    |
=====================================================================*/

#define CCMAIN
char    *PNAME = "$RCSfile: gen_lineno.c,v $";
char    *PROG_VERSION = "@(#) - $Header: /usr/ver9.10/REPOSITORY/STD/UTILS/gen_l
ineno/gen_lineno.c,v 1.1 2000/04/24 23:51:18 jinno Exp $";

#include	<pslscr.h>
#include	<stdio.h>

char    *arln   = "arln",
        *coln   = "coln",
        *poln   = "poln",
        *soln   = "soln",
        *arhr   = "arhr",
        *cohr   = "cohr",
        *pohr   = "pohr",
        *sohr   = "sohr",
        *data   = "data";


	/*======+
	 | comm |
	 +======*/
#define	COMM_NO_FIELDS	33

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
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
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dp_short"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_payroll_date"},
		{"comm_gl_date"},
		{"comm_closed_period"},
		{"comm_fiscal"},
		{"comm_gst_rate"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
		{"comm_pay_terms"},
		{"comm_env_name"},
		{"comm_stat_flag"}
	};

	struct tag_commRecord
	{
		int		term;
		char	co_no [3];
		char	co_name [41];
		char	co_short [16];
		char	est_no [3];
		char	est_name [41];
		char	est_short [16];
		char	cc_no [3];
		char	cc_name [41];
		char	cc_short [10];
		char	dp_no [3];
		char	dp_name [41];
		char	dp_short [16];
		Date	dbt_date;
		Date	crd_date;
		Date	inv_date;
		Date	payroll_date;
		Date	gl_date;
		int		closed_period;
		int		fiscal;
		float	gst_rate;
		char	price1_desc [16];
		char	price2_desc [16];
		char	price3_desc [16];
		char	price4_desc [16];
		char	price5_desc [16];
		char	price6_desc [16];
		char	price7_desc [16];
		char	price8_desc [16];
		char	price9_desc [16];
		int		pay_terms;
		char	env_name [61];
		char	stat_flag [2];
	}	comm_rec;

	/*======+
	 | pohr |
	 +======*/
#define	POHR_NO_FIELDS	40

	struct dbview	pohr_list [POHR_NO_FIELDS] =
	{
		{"pohr_co_no"},
		{"pohr_br_no"},
		{"pohr_type"},
		{"pohr_hhsu_hash"},
		{"pohr_pur_ord_no"},
		{"pohr_hhpo_hash"},
		{"pohr_hhsh_hash"},
		{"pohr_hhdd_hash"},
		{"pohr_date_raised"},
		{"pohr_due_date"},
		{"pohr_conf_date"},
		{"pohr_contact"},
		{"pohr_app_code"},
		{"pohr_op_id"},
		{"pohr_time_create"},
		{"pohr_date_create"},
		{"pohr_req_usr"},
		{"pohr_reason"},
		{"pohr_stdin1"},
		{"pohr_stdin2"},
		{"pohr_stdin3"},
		{"pohr_delin1"},
		{"pohr_delin2"},
		{"pohr_delin3"},
		{"pohr_ship1_no"},
		{"pohr_ship2_no"},
		{"pohr_ship3_no"},
		{"pohr_curr_code"},
		{"pohr_curr_rate"},
		{"pohr_term_order"},
		{"pohr_sup_term_pay"},
		{"pohr_bnk_term_pay"},
		{"pohr_pay_date"},
		{"pohr_fgn_total"},
		{"pohr_fgn_outstand"},
		{"pohr_ship_method"},
		{"pohr_drop_ship"},
		{"pohr_status"},
		{"pohr_stat_flag"},
		{"pohr_sup_type"}
	};

	struct tag_pohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	type [2];
		long	hhsu_hash;
		char	pur_ord_no [8];
		long	hhpo_hash;
		long	hhsh_hash;
		long	hhdd_hash;
		Date	date_raised;
		Date	due_date;
		Date	conf_date;
		char	contact [41];
		char	app_code [16];
		char	op_id [15];
		char	time_create [6];
		Date	date_create;
		char	req_usr [41];
		char	reason [41];
		char	stdin1 [61];
		char	stdin2 [61];
		char	stdin3 [61];
		char	delin1 [61];
		char	delin2 [61];
		char	delin3 [61];
		long	ship1_no;
		long	ship2_no;
		long	ship3_no;
		char	curr_code [4];
		double	curr_rate;
		char	term_order [21];
		char	sup_term_pay [31];
		char	bnk_term_pay [31];
		Date	pay_date;
		double	fgn_total;
		double	fgn_outstand;
		char	ship_method [2];
		char	drop_ship [2];
		char	status [2];
		char	stat_flag [2];
		char	sup_type [2];
	}	pohr_rec;

	/*======+
	 | poln |
	 +======*/
#define	POLN_NO_FIELDS	34

	struct dbview	poln_list [POLN_NO_FIELDS] =
	{
		{"poln_hhpo_hash"},
		{"poln_line_no"},
		{"poln_hhbr_hash"},
		{"poln_hhum_hash"},
		{"poln_hhcc_hash"},
		{"poln_hhlc_hash"},
		{"poln_hhpl_hash"},
		{"poln_exch_rate"},
		{"poln_serial_no"},
		{"poln_qty_ord"},
		{"poln_qty_rec"},
		{"poln_reg_pc"},
		{"poln_disc_a"},
		{"poln_disc_b"},
		{"poln_disc_c"},
		{"poln_cumulative"},
		{"poln_grs_fgn_cst"},
		{"poln_fob_fgn_cst"},
		{"poln_fob_nor_cst"},
		{"poln_frt_ins_cst"},
		{"poln_duty"},
		{"poln_licence"},
		{"poln_lcost_load"},
		{"poln_land_cst"},
		{"poln_cat_code"},
		{"poln_item_desc"},
		{"poln_ship_no"},
		{"poln_case_no"},
		{"poln_hhso_hash"},
		{"poln_due_date"},
		{"poln_fwd_date"},
		{"poln_pur_status"},
		{"poln_status"},
		{"poln_stat_flag"}
	};

	struct tag_polnRecord
	{
		long	hhpo_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhum_hash;
		long	hhcc_hash;
		long	hhlc_hash;
		long	hhpl_hash;
		double	exch_rate;
		char	serial_no [26];
		float	qty_ord;
		float	qty_rec;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		int		cumulative;
		double	grs_fgn_cst;
		double	fob_fgn_cst;
		double	fob_nor_cst;
		double	frt_ins_cst;
		double	duty;
		double	licence;
		double	lcost_load;
		double	land_cst;
		char	cat_code [12];
		char	item_desc [41];
		long	ship_no;
		int		case_no;
		long	hhso_hash;
		Date	due_date;
		Date	fwd_date;
		char	pur_status [2];
		char	status [2];
		char	stat_flag [2];
	}	poln_rec;

	/*======+
	 | cohr |
	 +======*/
#define	COHR_NO_FIELDS	84

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_app_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_chg_hhcu_hash"},
		{"cohr_type"},
		{"cohr_cont_no"},
		{"cohr_drop_ship"},
		{"cohr_hhds_hash"},
		{"cohr_cus_ord_ref"},
		{"cohr_chg_ord_ref"},
		{"cohr_ord_ref"},
		{"cohr_grn_no"},
		{"cohr_cons_no"},
		{"cohr_del_zone"},
		{"cohr_del_req"},
		{"cohr_del_date"},
		{"cohr_asm_req"},
		{"cohr_asm_date"},
		{"cohr_asm_hash"},
		{"cohr_s_timeslot"},
		{"cohr_e_timeslot"},
		{"cohr_carr_code"},
		{"cohr_carr_area"},
		{"cohr_no_cartons"},
		{"cohr_wgt_per_ctn"},
		{"cohr_no_kgs"},
		{"cohr_hhso_hash"},
		{"cohr_hhco_hash"},
		{"cohr_frei_req"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
		{"cohr_tax_code"},
		{"cohr_tax_no"},
		{"cohr_area_code"},
		{"cohr_sale_code"},
		{"cohr_op_id"},
		{"cohr_time_create"},
		{"cohr_date_create"},
		{"cohr_gross"},
		{"cohr_freight"},
		{"cohr_insurance"},
		{"cohr_other_cost_1"},
		{"cohr_other_cost_2"},
		{"cohr_other_cost_3"},
		{"cohr_tax"},
		{"cohr_gst"},
		{"cohr_disc"},
		{"cohr_deposit"},
		{"cohr_ex_disc"},
		{"cohr_erate_var"},
		{"cohr_sos"},
		{"cohr_exch_rate"},
		{"cohr_fix_exch"},
		{"cohr_batch_no"},
		{"cohr_dl_name"},
		{"cohr_dl_add1"},
		{"cohr_dl_add2"},
		{"cohr_dl_add3"},
		{"cohr_din_1"},
		{"cohr_din_2"},
		{"cohr_din_3"},
		{"cohr_pay_type"},
		{"cohr_pay_terms"},
		{"cohr_sell_terms"},
		{"cohr_ins_det"},
		{"cohr_pri_type"},
		{"cohr_pri_break"},
		{"cohr_ord_type"},
		{"cohr_prt_price"},
		{"cohr_status"},
		{"cohr_stat_flag"},
		{"cohr_ps_print"},
		{"cohr_ps_print_no"},
		{"cohr_inv_print"},
		{"cohr_ccn_print"},
		{"cohr_printing"},
		{"cohr_hhtr_hash"},
		{"cohr_load_flag"},
		{"cohr_wrmr_hash"},
		{"cohr_pos_inv_no"},
		{"cohr_pos_tran_type"}
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	inv_no [9];
		char	app_inv_no [9];
		long	hhcu_hash;
		long	chg_hhcu_hash;
		char	type [2];
		char	cont_no [7];
		char	drop_ship [2];
		long	hhds_hash;
		char	cus_ord_ref [21];
		char	chg_ord_ref [21];
		char	ord_ref [17];
		char	grn_no [21];
		char	cons_no [17];
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	asm_req [2];
		Date	asm_date;
		long	asm_hash;
		char	s_timeslot [2];
		char	e_timeslot [2];
		char	carr_code [5];
		char	carr_area [3];
		int		no_cartons;
		double	wgt_per_ctn;
		float	no_kgs;
		long	hhso_hash;
		long	hhco_hash;
		char	frei_req [2];
		Date	date_raised;
		Date	date_required;
		char	tax_code [2];
		char	tax_no [16];
		char	area_code [3];
		char	sale_code [3];
		char	op_id [15];
		char	time_create [6];
		Date	date_create;
		Money	gross;
		Money	freight;
		Money	insurance;
		Money	other_cost_1;
		Money	other_cost_2;
		Money	other_cost_3;
		Money	tax;
		Money	gst;
		Money	disc;
		Money	deposit;
		Money	ex_disc;
		Money	erate_var;
		Money	sos;
		double	exch_rate;
		char	fix_exch [2];
		char	batch_no [6];
		char	dl_name [41];
		char	dl_add1 [41];
		char	dl_add2 [41];
		char	dl_add3 [41];
		char	din_1 [61];
		char	din_2 [61];
		char	din_3 [61];
		char	pay_type [2];
		char	pay_terms [41];
		char	sell_terms [4];
		char	ins_det [31];
		char	pri_type [2];
		char	pri_break [2];
		char	ord_type [2];
		char	prt_price [2];
		char	status [2];
		char	stat_flag [2];
		char	ps_print [2];
		int		ps_print_no;
		char	inv_print [2];
		char	ccn_print [2];
		char	printing [2];
		long	hhtr_hash;
		char	load_flag [2];
		long	wrmr_hash;
		char	pos_inv_no [11];
		int		pos_tran_type;
	}	cohr_rec;

	/*======+
	 | coln |
	 +======*/
#define	COLN_NO_FIELDS	48

	struct dbview	coln_list [COLN_NO_FIELDS] =
	{
		{"coln_hhcl_hash"},
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_incc_hash"},
		{"coln_hhum_hash"},
		{"coln_hhsl_hash"},
		{"coln_hhdl_hash"},
		{"coln_crd_type"},
		{"coln_serial_no"},
		{"coln_cont_status"},
		{"coln_qty_org_ord"},
		{"coln_q_order"},
		{"coln_qty_del"},
		{"coln_qty_ret"},
		{"coln_q_backorder"},
		{"coln_gsale_price"},
		{"coln_sale_price"},
		{"coln_cost_price"},
		{"coln_disc_pc"},
		{"coln_reg_pc"},
		{"coln_disc_a"},
		{"coln_disc_b"},
		{"coln_disc_c"},
		{"coln_cumulative"},
		{"coln_tax_pc"},
		{"coln_gst_pc"},
		{"coln_gross"},
		{"coln_freight"},
		{"coln_on_cost"},
		{"coln_amt_disc"},
		{"coln_amt_tax"},
		{"coln_amt_gst"},
		{"coln_erate_var"},
		{"coln_pack_size"},
		{"coln_sman_code"},
		{"coln_cus_ord_ref"},
		{"coln_org_ord_ref"},
		{"coln_o_xrate"},
		{"coln_n_xrate"},
		{"coln_item_desc"},
		{"coln_due_date"},
		{"coln_status"},
		{"coln_bonus_flag"},
		{"coln_hide_flag"},
		{"coln_hhah_hash"},
		{"coln_price_type"},
		{"coln_stat_flag"}
	};

	struct tag_colnRecord
	{
		long	hhcl_hash;
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	incc_hash;
		long	hhum_hash;
		long	hhsl_hash;
		long	hhdl_hash;
		char	crd_type [2];
		char	serial_no [26];
		int		cont_status;
		float	qty_org_ord;
		float	q_order;
		float	qty_del;
		float	qty_ret;
		float	q_backorder;
		Money	gsale_price;
		Money	sale_price;
		Money	cost_price;
		float	disc_pc;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		int		cumulative;
		float	tax_pc;
		float	gst_pc;
		Money	gross;
		Money	freight;
		Money	on_cost;
		Money	amt_disc;
		Money	amt_tax;
		Money	amt_gst;
		Money	erate_var;
		char	pack_size [6];
		char	sman_code [3];
		char	cus_ord_ref [21];
		char	org_ord_ref [21];
		float	o_xrate;
		float	n_xrate;
		char	item_desc [41];
		Date	due_date;
		char	status [2];
		char	bonus_flag [2];
		char	hide_flag [2];
		long	hhah_hash;
		char	price_type [2];
		char	stat_flag [2];
	}	coln_rec;

	/*======+
	 | arhr |
	 +======*/
#define	ARHR_NO_FIELDS	84

	struct dbview	arhr_list [ARHR_NO_FIELDS] =
	{
		{"arhr_co_no"},
		{"arhr_br_no"},
		{"arhr_dp_no"},
		{"arhr_inv_no"},
		{"arhr_app_inv_no"},
		{"arhr_hhcu_hash"},
		{"arhr_chg_hhcu_hash"},
		{"arhr_type"},
		{"arhr_cont_no"},
		{"arhr_drop_ship"},
		{"arhr_hhds_hash"},
		{"arhr_cus_ord_ref"},
		{"arhr_chg_ord_ref"},
		{"arhr_ord_ref"},
		{"arhr_grn_no"},
		{"arhr_cons_no"},
		{"arhr_del_zone"},
		{"arhr_del_req"},
		{"arhr_del_date"},
		{"arhr_asm_req"},
		{"arhr_asm_date"},
		{"arhr_asm_hash"},
		{"arhr_s_timeslot"},
		{"arhr_e_timeslot"},
		{"arhr_carr_code"},
		{"arhr_carr_area"},
		{"arhr_no_cartons"},
		{"arhr_wgt_per_ctn"},
		{"arhr_no_kgs"},
		{"arhr_hhso_hash"},
		{"arhr_hhco_hash"},
		{"arhr_frei_req"},
		{"arhr_date_raised"},
		{"arhr_date_required"},
		{"arhr_tax_code"},
		{"arhr_tax_no"},
		{"arhr_area_code"},
		{"arhr_sale_code"},
		{"arhr_op_id"},
		{"arhr_time_create"},
		{"arhr_date_create"},
		{"arhr_gross"},
		{"arhr_freight"},
		{"arhr_insurance"},
		{"arhr_other_cost_1"},
		{"arhr_other_cost_2"},
		{"arhr_other_cost_3"},
		{"arhr_tax"},
		{"arhr_gst"},
		{"arhr_disc"},
		{"arhr_deposit"},
		{"arhr_ex_disc"},
		{"arhr_erate_var"},
		{"arhr_sos"},
		{"arhr_exch_rate"},
		{"arhr_fix_exch"},
		{"arhr_batch_no"},
		{"arhr_dl_name"},
		{"arhr_dl_add1"},
		{"arhr_dl_add2"},
		{"arhr_dl_add3"},
		{"arhr_din_1"},
		{"arhr_din_2"},
		{"arhr_din_3"},
		{"arhr_pay_type"},
		{"arhr_pay_terms"},
		{"arhr_sell_terms"},
		{"arhr_ins_det"},
		{"arhr_pri_type"},
		{"arhr_pri_break"},
		{"arhr_ord_type"},
		{"arhr_prt_price"},
		{"arhr_status"},
		{"arhr_stat_flag"},
		{"arhr_ps_print"},
		{"arhr_ps_print_no"},
		{"arhr_inv_print"},
		{"arhr_ccn_print"},
		{"arhr_printing"},
		{"arhr_hhtr_hash"},
		{"arhr_load_flag"},
		{"arhr_wrmr_hash"},
		{"arhr_pos_inv_no"},
		{"arhr_pos_tran_type"}
	};

	struct tag_arhrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	inv_no [9];
		char	app_inv_no [9];
		long	hhcu_hash;
		long	chg_hhcu_hash;
		char	type [2];
		char	cont_no [7];
		char	drop_ship [2];
		long	hhds_hash;
		char	cus_ord_ref [21];
		char	chg_ord_ref [21];
		char	ord_ref [17];
		char	grn_no [21];
		char	cons_no [17];
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	asm_req [2];
		Date	asm_date;
		long	asm_hash;
		char	s_timeslot [2];
		char	e_timeslot [2];
		char	carr_code [5];
		char	carr_area [3];
		int		no_cartons;
		double	wgt_per_ctn;
		float	no_kgs;
		long	hhso_hash;
		long	hhco_hash;
		char	frei_req [2];
		Date	date_raised;
		Date	date_required;
		char	tax_code [2];
		char	tax_no [16];
		char	area_code [3];
		char	sale_code [3];
		char	op_id [15];
		char	time_create [6];
		Date	date_create;
		Money	gross;
		Money	freight;
		Money	insurance;
		Money	other_cost_1;
		Money	other_cost_2;
		Money	other_cost_3;
		Money	tax;
		Money	gst;
		Money	disc;
		Money	deposit;
		Money	ex_disc;
		Money	erate_var;
		Money	sos;
		double	exch_rate;
		char	fix_exch [2];
		char	batch_no [6];
		char	dl_name [41];
		char	dl_add1 [41];
		char	dl_add2 [41];
		char	dl_add3 [41];
		char	din_1 [61];
		char	din_2 [61];
		char	din_3 [61];
		char	pay_type [2];
		char	pay_terms [41];
		char	sell_terms [4];
		char	ins_det [31];
		char	pri_type [2];
		char	pri_break [2];
		char	ord_type [2];
		char	prt_price [2];
		char	status [2];
		char	stat_flag [2];
		char	ps_print [2];
		int		ps_print_no;
		char	inv_print [2];
		char	ccn_print [2];
		char	printing [2];
		long	hhtr_hash;
		char	load_flag [2];
		long	wrmr_hash;
		char	pos_inv_no [11];
		int		pos_tran_type;
	}	arhr_rec;

	/*======+
	 | arln |
	 +======*/
#define	ARLN_NO_FIELDS	48

	struct dbview	arln_list [ARLN_NO_FIELDS] =
	{
		{"arln_hhcl_hash"},
		{"arln_hhco_hash"},
		{"arln_line_no"},
		{"arln_hhbr_hash"},
		{"arln_incc_hash"},
		{"arln_hhum_hash"},
		{"arln_hhsl_hash"},
		{"arln_hhdl_hash"},
		{"arln_crd_type"},
		{"arln_serial_no"},
		{"arln_cont_status"},
		{"arln_qty_org_ord"},
		{"arln_q_order"},
		{"arln_qty_del"},
		{"arln_qty_ret"},
		{"arln_q_backorder"},
		{"arln_gsale_price"},
		{"arln_sale_price"},
		{"arln_cost_price"},
		{"arln_disc_pc"},
		{"arln_reg_pc"},
		{"arln_disc_a"},
		{"arln_disc_b"},
		{"arln_disc_c"},
		{"arln_cumulative"},
		{"arln_tax_pc"},
		{"arln_gst_pc"},
		{"arln_gross"},
		{"arln_freight"},
		{"arln_on_cost"},
		{"arln_amt_disc"},
		{"arln_amt_tax"},
		{"arln_amt_gst"},
		{"arln_erate_var"},
		{"arln_pack_size"},
		{"arln_sman_code"},
		{"arln_cus_ord_ref"},
		{"arln_org_ord_ref"},
		{"arln_o_xrate"},
		{"arln_n_xrate"},
		{"arln_item_desc"},
		{"arln_due_date"},
		{"arln_status"},
		{"arln_bonus_flag"},
		{"arln_hide_flag"},
		{"arln_hhah_hash"},
		{"arln_price_type"},
		{"arln_stat_flag"}
	};

	struct tag_arlnRecord
	{
		long	hhcl_hash;
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	incc_hash;
		long	hhum_hash;
		long	hhsl_hash;
		long	hhdl_hash;
		char	crd_type [2];
		char	serial_no [26];
		int		cont_status;
		float	qty_org_ord;
		float	q_order;
		float	qty_del;
		float	qty_ret;
		float	q_backorder;
		Money	gsale_price;
		Money	sale_price;
		Money	cost_price;
		float	disc_pc;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		int		cumulative;
		float	tax_pc;
		float	gst_pc;
		Money	gross;
		Money	freight;
		Money	on_cost;
		Money	amt_disc;
		Money	amt_tax;
		Money	amt_gst;
		Money	erate_var;
		char	pack_size [6];
		char	sman_code [3];
		char	cus_ord_ref [21];
		char	org_ord_ref [21];
		float	o_xrate;
		float	n_xrate;
		char	item_desc [41];
		Date	due_date;
		char	status [2];
		char	bonus_flag [2];
		char	hide_flag [2];
		long	hhah_hash;
		char	price_type [2];
		char	stat_flag [2];
	}	arln_rec;

	/*======+
	 | sohr |
	 +======*/
#define	SOHR_NO_FIELDS	64

	struct dbview	sohr_list [SOHR_NO_FIELDS] =
	{
		{"sohr_co_no"},
		{"sohr_br_no"},
		{"sohr_dp_no"},
		{"sohr_order_no"},
		{"sohr_cont_no"},
		{"sohr_hhcu_hash"},
		{"sohr_chg_hhcu_hash"},
		{"sohr_hhso_hash"},
		{"sohr_inv_no"},
		{"sohr_cus_ord_ref"},
		{"sohr_chg_ord_ref"},
		{"sohr_op_id"},
		{"sohr_time_create"},
		{"sohr_date_create"},
		{"sohr_cons_no"},
		{"sohr_del_zone"},
		{"sohr_del_req"},
		{"sohr_del_date"},
		{"sohr_asm_req"},
		{"sohr_asm_date"},
		{"sohr_s_timeslot"},
		{"sohr_e_timeslot"},
		{"sohr_carr_code"},
		{"sohr_carr_area"},
		{"sohr_no_cartons"},
		{"sohr_no_kgs"},
		{"sohr_sch_ord"},
		{"sohr_ord_type"},
		{"sohr_pri_type"},
		{"sohr_frei_req"},
		{"sohr_dt_raised"},
		{"sohr_dt_required"},
		{"sohr_tax_code"},
		{"sohr_tax_no"},
		{"sohr_area_code"},
		{"sohr_sman_code"},
		{"sohr_sell_terms"},
		{"sohr_pay_term"},
		{"sohr_freight"},
		{"sohr_insurance"},
		{"sohr_ins_det"},
		{"sohr_other_cost_1"},
		{"sohr_other_cost_2"},
		{"sohr_other_cost_3"},
		{"sohr_deposit"},
		{"sohr_discount"},
		{"sohr_exch_rate"},
		{"sohr_fix_exch"},
		{"sohr_batch_no"},
		{"sohr_cont_name"},
		{"sohr_cont_phone"},
		{"sohr_del_name"},
		{"sohr_del_add1"},
		{"sohr_del_add2"},
		{"sohr_del_add3"},
		{"sohr_din_1"},
		{"sohr_din_2"},
		{"sohr_din_3"},
		{"sohr_new"},
		{"sohr_prt_price"},
		{"sohr_full_supply"},
		{"sohr_two_step"},
		{"sohr_status"},
		{"sohr_stat_flag"}
	};

	struct tag_sohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	order_no [9];
		char	cont_no [7];
		long	hhcu_hash;
		long	chg_hhcu_hash;
		long	hhso_hash;
		char	inv_no [9];
		char	cus_ord_ref [21];
		char	chg_ord_ref [21];
		char	op_id [15];
		char	time_create [6];
		Date	date_create;
		char	cons_no [17];
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	asm_req [2];
		Date	asm_date;
		char	s_timeslot [2];
		char	e_timeslot [2];
		char	carr_code [5];
		char	carr_area [3];
		int		no_cartons;
		float	no_kgs;
		char	sch_ord [2];
		char	ord_type [2];
		char	pri_type [2];
		char	frei_req [2];
		Date	dt_raised;
		Date	dt_required;
		char	tax_code [2];
		char	tax_no [16];
		char	area_code [3];
		char	sman_code [3];
		char	sell_terms [4];
		char	pay_term [41];
		Money	freight;
		Money	insurance;
		char	ins_det [31];
		Money	other_cost_1;
		Money	other_cost_2;
		Money	other_cost_3;
		Money	deposit;
		Money	discount;
		double	exch_rate;
		char	fix_exch [2];
		char	batch_no [6];
		char	cont_name [21];
		char	cont_phone [16];
		char	del_name [41];
		char	del_add1 [41];
		char	del_add2 [41];
		char	del_add3 [41];
		char	din_1 [61];
		char	din_2 [61];
		char	din_3 [61];
		char	new [2];
		char	prt_price [2];
		char	full_supply [2];
		char	two_step [2];
		char	status [2];
		char	stat_flag [2];
	}	sohr_rec;

	/*======+
	 | soln |
	 +======*/
#define	SOLN_NO_FIELDS	2

	struct dbview	soln_list [SOLN_NO_FIELDS] =
	{
		{"soln_hhso_hash"},
		{"soln_line_no"}
	};

	struct tag_solnRecord
	{
		long	hhso_hash;
		int	line_no;
	}	soln_rec;

void OpenDB	(void);
void CloseDB	(void);
void GenLine	(char*);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
        int     i;

        if (argc == 1)
        {
		printf ("Usage : %s <table_name> ...\n", argv[0]);
                return (EXIT_FAILURE);
        }

	OpenDB();
        read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);              

        for (i=1; i<argc; i++)
        {
		if (!strcmp ("arln", argv[i]) || !strcmp ("coln", argv[i]) || 
		    !strcmp ("poln", argv[i]) || !strcmp ("soln", argv[i])) 
			GenLine (argv[i]);
		else
			printf ("Invalid table: %s\n", argv[i]);
        }

	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

void
OpenDB ()
{
	abc_dbopen (data);

        open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
        open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
        open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
        open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
        open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_id_no");
        open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
        open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
        open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
} 

void
CloseDB ()
{
	abc_fclose (arln);
	abc_fclose (coln);
	abc_fclose (poln);
	abc_fclose (soln);
	abc_fclose (arhr);
	abc_fclose (cohr);
	abc_fclose (pohr);
	abc_fclose (sohr);

	abc_dbclose (data);
}

void
GenLine ( char* TableName )
{
	int ctr;

	if (!strcmp ("arln", TableName))
	{
		printf("Updating %s.... \n", TableName);
	        strcpy (arhr_rec.co_no, comm_rec.co_no);
	        strcpy (arhr_rec.br_no, comm_rec.est_no);
                cc = find_rec ("arhr", &arhr_rec, GTEQ, "r");
                while (!cc && !strcmp (arhr_rec.co_no, comm_rec.co_no) &&
			!strcmp (arhr_rec.br_no, comm_rec.est_no))
		{
			ctr = 0;
			arln_rec.hhco_hash = arhr_rec.hhco_hash;
			arln_rec.line_no = 0l;
			cc = find_rec ("arln", &arln_rec, GTEQ, "u");
			while (!cc && arln_rec.hhco_hash == arhr_rec.hhco_hash)
			{
                                arln_rec.line_no = ctr;
				cc = abc_update (arln, &arln_rec);
				if (cc)
					file_err (cc, "arln", "DBUPDATE");

				ctr ++;
				cc = find_rec ("arln", &arln_rec, NEXT, "u");
			}
			cc = find_rec ("arhr", &arhr_rec, NEXT, "r");
		}
	}
	
	if (!strcmp ("coln", TableName))
	{
		printf("Updating %s.... \n", TableName);
	        strcpy (cohr_rec.co_no, comm_rec.co_no);
	        strcpy (cohr_rec.br_no, comm_rec.est_no);
                cc = find_rec ("cohr", &cohr_rec, GTEQ, "r");
                while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
			!strcmp (cohr_rec.br_no, comm_rec.est_no))
		{
			ctr = 0;
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = 0l;
			cc = find_rec ("coln", &coln_rec, GTEQ, "u");
			while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
			{
                                coln_rec.line_no = ctr;
				cc = abc_update (coln, &coln_rec);
				if (cc)
					file_err (cc, "coln", "DBUPDATE");

				ctr ++;
				cc = find_rec ("coln", &coln_rec, NEXT, "u");
			}
			cc = find_rec ("cohr", &cohr_rec, NEXT, "r");
		}
	}

	if (!strcmp ("poln", TableName))
	{
		printf("Updating %s.... \n", TableName);
	        strcpy (pohr_rec.co_no, comm_rec.co_no);
	        strcpy (pohr_rec.br_no, comm_rec.est_no);
                cc = find_rec ("pohr", &pohr_rec, GTEQ, "r");
                while (!cc && !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
			!strcmp (pohr_rec.br_no, comm_rec.est_no))
		{
			ctr = 0;
			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			cc = find_rec ("poln", &poln_rec, GTEQ, "u");
			while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
			{
                                poln_rec.line_no = ctr;
				cc = abc_update (poln, &poln_rec);
				if (cc)
					file_err (cc, "poln", "DBUPDATE");

				ctr ++;
				cc = find_rec ("poln", &poln_rec, NEXT, "u");
			}
			cc = find_rec ("pohr", &pohr_rec, NEXT, "r");
		}
	}

	if (!strcmp ("soln", TableName))
	{
		printf("Updating %s.... \n", TableName);
	        strcpy (sohr_rec.co_no, comm_rec.co_no);
	        strcpy (sohr_rec.br_no, comm_rec.est_no);
                cc = find_rec ("sohr", &sohr_rec, GTEQ, "r");
                while (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
			!strcmp (sohr_rec.br_no, comm_rec.est_no))
		{
			ctr = 0;
			soln_rec.hhso_hash = sohr_rec.hhso_hash;
			soln_rec.line_no = 0l;
			cc = find_rec ("soln", &soln_rec, GTEQ, "u");
			while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
			{
                                soln_rec.line_no = ctr;
				cc = abc_update (soln, &soln_rec);
				if (cc)
					file_err (cc, "soln", "DBUPDATE");

				ctr ++;
				cc = find_rec ("soln", &soln_rec, NEXT, "u");
			}
			cc = find_rec ("sohr", &sohr_rec, NEXT, "r");
		}
	}
}

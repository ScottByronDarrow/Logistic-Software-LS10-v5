#ifndef	_ArchiveTables_h
#define	_ArchiveTables_h
/*	$Id: ArchiveTables.h,v 5.0 2002/05/08 01:40:39 scott Exp $
 *******************************************************************************
 *	$Log: ArchiveTables.h,v $
 *	Revision 5.0  2002/05/08 01:40:39  scott
 *	CVS administration
 *	
 *	Revision 1.3  2002/05/07 02:50:01  scott
 *	Updated for new archiving system
 *	
 *	Revision 1.2  2002/05/02 03:47:08  scott
 *	Updated from testing
 *	
 *	Revision 1.1  2002/04/30 03:19:00  scott
 *	Update for new Archive modifications;
 *	
 *	New Archive Functions - Table Headers
 *	
 */

/*
 * Archive copy of Production Control Works Order File 
 */
#define	ARPCWO_NO_FIELDS	27

	static	struct dbview	arpcwo_list [ARPCWO_NO_FIELDS] =
	{
		{"arpcwo_co_no"},
		{"arpcwo_br_no"},
		{"arpcwo_wh_no"},
		{"arpcwo_order_no"},
		{"arpcwo_req_br_no"},
		{"arpcwo_req_wh_no"},
		{"arpcwo_rec_br_no"},
		{"arpcwo_rec_wh_no"},
		{"arpcwo_hhwo_hash"},
		{"arpcwo_reqd_date"},
		{"arpcwo_rtg_seq"},
		{"arpcwo_priority"},
		{"arpcwo_op_id"},
		{"arpcwo_create_time"},
		{"arpcwo_create_date"},
		{"arpcwo_mfg_date"},
		{"arpcwo_hhbr_hash"},
		{"arpcwo_bom_alt"},
		{"arpcwo_rtg_alt"},
		{"arpcwo_hhcc_hash"},
		{"arpcwo_prod_qty"},
		{"arpcwo_act_prod_q"},
		{"arpcwo_act_rej_q"},
		{"arpcwo_order_stat"},
		{"arpcwo_batch_no"},
		{"arpcwo_hhsl_hash"},
		{"arpcwo_stat_flag"}
	};

	struct arpcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	order_no [8];
		char	req_br_no [3];
		char	req_wh_no [3];
		char	rec_br_no [3];
		char	rec_wh_no [3];
		long	hhwo_hash;
		Date	reqd_date;
		int		rtg_seq;
		int		priority;
		char	op_id [15];
		char	create_time [6];
		Date	create_date;
		Date	mfg_date;
		long	hhbr_hash;
		int		bom_alt;
		int		rtg_alt;
		long	hhcc_hash;
		float	prod_qty;
		float	act_prod_q;
		float	act_rej_q;
		char	order_stat [2];
		char	batch_no [11];
		long	hhsl_hash;
		char	stat_flag [2];
	};

/*
 * Archive Copy - Sales Order Header File
 */
#define	ARSOHR_NO_FIELDS	64

	static	struct dbview	arsohr_list [ARSOHR_NO_FIELDS] =
	{
		{"arsohr_co_no"},
		{"arsohr_br_no"},
		{"arsohr_dp_no"},
		{"arsohr_order_no"},
		{"arsohr_cont_no"},
		{"arsohr_hhcu_hash"},
		{"arsohr_chg_hhcu"},
		{"arsohr_hhso_hash"},
		{"arsohr_inv_no"},
		{"arsohr_cus_ord_ref"},
		{"arsohr_chg_ord_ref"},
		{"arsohr_op_id"},
		{"arsohr_time_create"},
		{"arsohr_date_create"},
		{"arsohr_cons_no"},
		{"arsohr_del_zone"},
		{"arsohr_del_req"},
		{"arsohr_del_date"},
		{"arsohr_asm_req"},
		{"arsohr_asm_date"},
		{"arsohr_s_timeslot"},
		{"arsohr_e_timeslot"},
		{"arsohr_carr_code"},
		{"arsohr_carr_area"},
		{"arsohr_no_cartons"},
		{"arsohr_no_kgs"},
		{"arsohr_sch_ord"},
		{"arsohr_ord_type"},
		{"arsohr_pri_type"},
		{"arsohr_frei_req"},
		{"arsohr_dt_raised"},
		{"arsohr_dt_required"},
		{"arsohr_tax_code"},
		{"arsohr_tax_no"},
		{"arsohr_area_code"},
		{"arsohr_sman_code"},
		{"arsohr_sell_terms"},
		{"arsohr_pay_term"},
		{"arsohr_freight"},
		{"arsohr_insurance"},
		{"arsohr_ins_det"},
		{"arsohr_o_cost_1"},
		{"arsohr_o_cost_2"},
		{"arsohr_o_cost_3"},
		{"arsohr_deposit"},
		{"arsohr_discount"},
		{"arsohr_exch_rate"},
		{"arsohr_fix_exch"},
		{"arsohr_batch_no"},
		{"arsohr_cont_name"},
		{"arsohr_cont_phone"},
		{"arsohr_del_name"},
		{"arsohr_del_add1"},
		{"arsohr_del_add2"},
		{"arsohr_del_add3"},
		{"arsohr_din_1"},
		{"arsohr_din_2"},
		{"arsohr_din_3"},
		{"arsohr_new"},
		{"arsohr_prt_price"},
		{"arsohr_full_supply"},
		{"arsohr_two_step"},
		{"arsohr_status"},
		{"arsohr_stat_flag"}
	};

	struct arsohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	order_no [9];
		char	cont_no [7];
		long	hhcu_hash;
		long	chg_hhcu;
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
		Money	o_cost_1;
		Money	o_cost_2;
		Money	o_cost_3;
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
	};

/*
 * Archive Copy - Sales Order Detail Lines File 
 */
#define	ARSOLN_NO_FIELDS	38

	static	struct dbview	arsoln_list [ARSOLN_NO_FIELDS] =
	{
		{"arsoln_hhso_hash"},
		{"arsoln_line_no"},
		{"arsoln_hhbr_hash"},
		{"arsoln_hhcc_hash"},
		{"arsoln_hhum_hash"},
		{"arsoln_hhsl_hash"},
		{"arsoln_serial_no"},
		{"arsoln_cont_status"},
		{"arsoln_qty_order"},
		{"arsoln_qty_bord"},
		{"arsoln_qty_org_ord"},
		{"arsoln_gsale_price"},
		{"arsoln_sale_price"},
		{"arsoln_cost_price"},
		{"arsoln_item_levy"},
		{"arsoln_dis_pc"},
		{"arsoln_reg_pc"},
		{"arsoln_disc_a"},
		{"arsoln_disc_b"},
		{"arsoln_disc_c"},
		{"arsoln_cumulative"},
		{"arsoln_tax_pc"},
		{"arsoln_gst_pc"},
		{"arsoln_o_xrate"},
		{"arsoln_n_xrate"},
		{"arsoln_pack_size"},
		{"arsoln_sman_code"},
		{"arsoln_cus_ord_ref"},
		{"arsoln_pri_or"},
		{"arsoln_dis_or"},
		{"arsoln_item_desc"},
		{"arsoln_due_date"},
		{"arsoln_del_no"},
		{"arsoln_bonus_flag"},
		{"arsoln_hide_flag"},
		{"arsoln_hhah_hash"},
		{"arsoln_status"},
		{"arsoln_stat_flag"}
	};

	struct arsolnRecord
	{
		long	hhso_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhum_hash;
		long	hhsl_hash;
		char	serial_no [26];
		int		cont_status;
		float	qty_order;
		float	qty_bord;
		float	qty_org_ord;
		Money	gsale_price;
		Money	sale_price;
		Money	cost_price;
		Money	item_levy;
		float	dis_pc;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		int		cumulative;
		float	tax_pc;
		float	gst_pc;
		float	o_xrate;
		float	n_xrate;
		char	pack_size [6];
		char	sman_code [3];
		char	cus_ord_ref [21];
		char	pri_or [2];
		char	dis_or [2];
		char	item_desc [41];
		Date	due_date;
		int		del_no;
		char	bonus_flag [2];
		char	hide_flag [2];
		long	hhah_hash;
		char	status [2];
		char	stat_flag [2];
	};

	/*
	 * Production Control Works Order File. 
	 */
#define	PCWO_NO_FIELDS	27

	static	struct dbview	pcwo_list [PCWO_NO_FIELDS] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_req_br_no"},
		{"pcwo_req_wh_no"},
		{"pcwo_rec_br_no"},
		{"pcwo_rec_wh_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_reqd_date"},
		{"pcwo_rtg_seq"},
		{"pcwo_priority"},
		{"pcwo_op_id"},
		{"pcwo_create_time"},
		{"pcwo_create_date"},
		{"pcwo_mfg_date"},
		{"pcwo_hhbr_hash"},
		{"pcwo_bom_alt"},
		{"pcwo_rtg_alt"},
		{"pcwo_hhcc_hash"},
		{"pcwo_prod_qty"},
		{"pcwo_act_prod_qty"},
		{"pcwo_act_rej_qty"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
		{"pcwo_hhsl_hash"},
		{"pcwo_stat_flag"}
	};

	struct pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	order_no [8];
		char	req_br_no [3];
		char	req_wh_no [3];
		char	rec_br_no [3];
		char	rec_wh_no [3];
		long	hhwo_hash;
		Date	reqd_date;
		int		rtg_seq;
		int		priority;
		char	op_id [15];
		char	create_time [6];
		Date	create_date;
		Date	mfg_date;
		long	hhbr_hash;
		int		bom_alt;
		int		rtg_alt;
		long	hhcc_hash;
		float	prod_qty;
		float	act_prod_qty;
		float	act_rej_qty;
		char	order_status [2];
		char	batch_no [11];
		long	hhsl_hash;
		char	stat_flag [2];
	};

	/*
	 * Sales Order Header File.
	 */
#define	SOHR_NO_FIELDS	64

	static	struct dbview	sohr_list [SOHR_NO_FIELDS] =
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

	struct sohrRecord
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
	};

	/*
	 * Sales Order Detail Lines File.
	 */
#define	SOLN_NO_FIELDS	38

	static	struct dbview	soln_list [SOLN_NO_FIELDS] =
	{
		{"soln_hhso_hash"},
		{"soln_line_no"},
		{"soln_hhbr_hash"},
		{"soln_hhcc_hash"},
		{"soln_hhum_hash"},
		{"soln_hhsl_hash"},
		{"soln_serial_no"},
		{"soln_cont_status"},
		{"soln_qty_order"},
		{"soln_qty_bord"},
		{"soln_qty_org_ord"},
		{"soln_gsale_price"},
		{"soln_sale_price"},
		{"soln_cost_price"},
		{"soln_item_levy"},
		{"soln_dis_pc"},
		{"soln_reg_pc"},
		{"soln_disc_a"},
		{"soln_disc_b"},
		{"soln_disc_c"},
		{"soln_cumulative"},
		{"soln_tax_pc"},
		{"soln_gst_pc"},
		{"soln_o_xrate"},
		{"soln_n_xrate"},
		{"soln_pack_size"},
		{"soln_sman_code"},
		{"soln_cus_ord_ref"},
		{"soln_pri_or"},
		{"soln_dis_or"},
		{"soln_item_desc"},
		{"soln_due_date"},
		{"soln_del_no"},
		{"soln_bonus_flag"},
		{"soln_hide_flag"},
		{"soln_hhah_hash"},
		{"soln_status"},
		{"soln_stat_flag"}
	};

	struct solnRecord
	{
		long	hhso_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhum_hash;
		long	hhsl_hash;
		char	serial_no [26];
		int		cont_status;
		float	qty_order;
		float	qty_bord;
		float	qty_org_ord;
		Money	gsale_price;
		Money	sale_price;
		Money	cost_price;
		Money	item_levy;
		float	dis_pc;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		int		cumulative;
		float	tax_pc;
		float	gst_pc;
		float	o_xrate;
		float	n_xrate;
		char	pack_size [6];
		char	sman_code [3];
		char	cus_ord_ref [21];
		char	pri_or [2];
		char	dis_or [2];
		char	item_desc [41];
		Date	due_date;
		int		del_no;
		char	bonus_flag [2];
		char	hide_flag [2];
		long	hhah_hash;
		char	status [2];
		char	stat_flag [2];
	};

	/*
	 * Archive Copy - Purchase order header file 
	 */
#define	ARPOHR_NO_FIELDS	40

	static	struct dbview	arpohr_list [ARPOHR_NO_FIELDS] =
	{
		{"arpohr_co_no"},
		{"arpohr_br_no"},
		{"arpohr_type"},
		{"arpohr_hhsu_hash"},
		{"arpohr_pur_ord_no"},
		{"arpohr_hhpo_hash"},
		{"arpohr_hhsh_hash"},
		{"arpohr_hhdd_hash"},
		{"arpohr_date_raised"},
		{"arpohr_due_date"},
		{"arpohr_conf_date"},
		{"arpohr_contact"},
		{"arpohr_app_code"},
		{"arpohr_op_id"},
		{"arpohr_time_create"},
		{"arpohr_date_create"},
		{"arpohr_req_usr"},
		{"arpohr_reason"},
		{"arpohr_stdin1"},
		{"arpohr_stdin2"},
		{"arpohr_stdin3"},
		{"arpohr_delin1"},
		{"arpohr_delin2"},
		{"arpohr_delin3"},
		{"arpohr_ship1_no"},
		{"arpohr_ship2_no"},
		{"arpohr_ship3_no"},
		{"arpohr_curr_code"},
		{"arpohr_curr_rate"},
		{"arpohr_term_order"},
		{"arpohr_sup_trm_pay"},
		{"arpohr_bnk_trm_pay"},
		{"arpohr_pay_date"},
		{"arpohr_fgn_total"},
		{"arpohr_fgn_ostand"},
		{"arpohr_ship_method"},
		{"arpohr_drop_ship"},
		{"arpohr_status"},
		{"arpohr_stat_flag"},
		{"arpohr_sup_type"}
	};

	struct arpohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	type [2];
		long	hhsu_hash;
		char	pur_ord_no [16];
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
		char	sup_trm_pay [31];
		char	bnk_trm_pay [31];
		Date	pay_date;
		double	fgn_total;
		double	fgn_ostand;
		char	ship_method [2];
		char	drop_ship [2];
		char	status [2];
		char	stat_flag [2];
		char	sup_type [2];
	};

	/*
	 * Archiving Copy - Purchase order detail lines 
	 */
#define	ARPOLN_NO_FIELDS	41

	static	struct dbview	arpoln_list [ARPOLN_NO_FIELDS] =
	{
		{"arpoln_hhpo_hash"},
		{"arpoln_line_no"},
		{"arpoln_hhbr_hash"},
		{"arpoln_hhum_hash"},
		{"arpoln_hhcc_hash"},
		{"arpoln_hhlc_hash"},
		{"arpoln_hhpl_hash"},
		{"arpoln_hhpl_orig"},
		{"arpoln_exch_rate"},
		{"arpoln_serial_no"},
		{"arpoln_container"},
		{"arpoln_cus_ord_ref"},
		{"arpoln_qty_ord"},
		{"arpoln_qty_rec"},
		{"arpoln_pack_qty"},
		{"arpoln_chg_wgt"},
		{"arpoln_gross_wgt"},
		{"arpoln_cu_metre"},
		{"arpoln_reg_pc"},
		{"arpoln_disc_a"},
		{"arpoln_disc_b"},
		{"arpoln_disc_c"},
		{"arpoln_cumulative"},
		{"arpoln_grs_fgn_cst"},
		{"arpoln_fob_fgn_cst"},
		{"arpoln_fob_nor_cst"},
		{"arpoln_frt_ins_cst"},
		{"arpoln_duty"},
		{"arpoln_licence"},
		{"arpoln_lcost_load"},
		{"arpoln_land_cst"},
		{"arpoln_cat_code"},
		{"arpoln_item_desc"},
		{"arpoln_ship_no"},
		{"arpoln_case_no"},
		{"arpoln_hhso_hash"},
		{"arpoln_due_date"},
		{"arpoln_fwd_date"},
		{"arpoln_pur_status"},
		{"arpoln_status"},
		{"arpoln_stat_flag"}
	};

	struct arpolnRecord
	{
		long	hhpo_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhum_hash;
		long	hhcc_hash;
		long	hhlc_hash;
		long	hhpl_hash;
		long	hhpl_orig;
		double	exch_rate;
		char	serial_no [26];
		char	container [16];
		char	cus_ord_ref [21];
		float	qty_ord;
		float	qty_rec;
		float	pack_qty;
		float	chg_wgt;
		float	gross_wgt;
		float	cu_metre;
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
	};

	/*
	 * Purchase order header file. 
	 */
#define	POHR_NO_FIELDS	40

	static	struct dbview	pohr_list [POHR_NO_FIELDS] =
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

	struct pohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	type [2];
		long	hhsu_hash;
		char	pur_ord_no [16];
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
	};

	/*
	 * Purchase order detail lines. 
	 */
#define	POLN_NO_FIELDS	41

	static	struct dbview	poln_list [POLN_NO_FIELDS] =
	{
		{"poln_hhpo_hash"},
		{"poln_line_no"},
		{"poln_hhbr_hash"},
		{"poln_hhum_hash"},
		{"poln_hhcc_hash"},
		{"poln_hhlc_hash"},
		{"poln_hhpl_hash"},
		{"poln_hhpl_orig"},
		{"poln_exch_rate"},
		{"poln_serial_no"},
		{"poln_container"},
		{"poln_cus_ord_ref"},
		{"poln_qty_ord"},
		{"poln_qty_rec"},
		{"poln_pack_qty"},
		{"poln_chg_wgt"},
		{"poln_gross_wgt"},
		{"poln_cu_metre"},
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

	struct polnRecord
	{
		long	hhpo_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhum_hash;
		long	hhcc_hash;
		long	hhlc_hash;
		long	hhpl_hash;
		long	hhpl_orig;
		double	exch_rate;
		char	serial_no [26];
		char	container [16];
		char	cus_ord_ref [21];
		float	qty_ord;
		float	qty_rec;
		float	pack_qty;
		float	chg_wgt;
		float	gross_wgt;
		float	cu_metre;
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
	};

	/*
	 * See file cohr - Exact copy for Archive 
	 */
#define	ARHR_NO_FIELDS	85

	static	struct dbview	arhr_list [ARHR_NO_FIELDS] =
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
		{"arhr_item_levy"},
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

	struct arhrRecord
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
		Money	item_levy;
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
	};

	/*
	 * See file coln - this is the archive version. 
	 */
#define	ARLN_NO_FIELDS	51

	static	struct dbview	arln_list [ARLN_NO_FIELDS] =
	{
		{"arln_hhcl_hash"},
		{"arln_hhco_hash"},
		{"arln_line_no"},
		{"arln_hhbr_hash"},
		{"arln_incc_hash"},
		{"arln_hhum_hash"},
		{"arln_hhsl_hash"},
		{"arln_order_no"},
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
		{"arln_item_levy"},
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
		{"arln_stat_flag"},
		{"arln_hhwo_hash"}
	};

	struct arlnRecord
	{
		long	hhcl_hash;
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	incc_hash;
		long	hhum_hash;
		long	hhsl_hash;
		char	order_no [9];
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
		Money	item_levy;
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
		long	hhwo_hash;
	};

#endif	/* _ArchiveTables_h */

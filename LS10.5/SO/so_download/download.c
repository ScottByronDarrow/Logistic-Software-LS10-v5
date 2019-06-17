/*
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=|
|  Program Name  : ( so_download.c  )                                 |
|  Program Desc  : ( Program to download invoice information for    ) |
|                  ( sealed air                                     ) |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files : See /usr/ver(x)/DOCS/Programs                      |
|---------------------------------------------------------------------|
|  Date Written  : (17/02/1997)    | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
|  Date Modified : (dd/mm/yyyy)    | Modified  by : Scott B Darrow.   |
|                : (01/10/1997)    | Modified  by : Marnie Organo.    | 
|  Date Modified : (25/10/1997)    | Modified by : Campbell Mander.   |
|   Comments     :                                                    |
|   (01/10/1997) : Updated for Multilingual Conversion .              |
|  (25/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                :                                                    |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*/

#define	CCMAIN
char	*PNAME = "$RCSfile: download.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_download/download.c,v 5.2 2001/08/09 09:21:13 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>

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
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
		int		tfiscal;
	} comm_rec;

	/*======+
	 | cohr |
	 +======*/
#define	COHR_NO_FIELDS	65

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_cont_no"},
		{"cohr_drop_ship"},
		{"cohr_hhds_hash"},
		{"cohr_cus_ord_ref"},
		{"cohr_ord_ref"},
		{"cohr_cons_no"},
		{"cohr_carr_code"},
		{"cohr_carr_area"},
		{"cohr_no_cartons"},
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
		{"cohr_inv_print"},
		{"cohr_ccn_print"},
		{"cohr_printing"},
		{"cohr_hhtr_hash"}
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	inv_no [9];
		long	hhcu_hash;
		char	type [2];
		char	cont_no [7];
		char	drop_ship [2];
		long	hhds_hash;
		char	cus_ord_ref [21];
		char	ord_ref [17];
		char	cons_no [17];
		char	carr_code [5];
		char	carr_area [3];
		int		no_cartons;
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
		Money	other_cost[3];
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
		char	inv_print [2];
		char	ccn_print [2];
		char	printing [2];
		long	hhtr_hash;
	}	cohr_rec;

	/*============================================+
	 | Customer Order/Invoice/Credit Detail File. |
	 +============================================*/
#define	COLN_NO_FIELDS	44

	struct dbview	coln_list [COLN_NO_FIELDS] =
	{
		{"coln_hhcl_hash"},
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_incc_hash"},
		{"coln_hhsl_hash"},
		{"coln_hhdl_hash"},
		{"coln_crd_type"},
		{"coln_serial_no"},
		{"coln_cont_status"},
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
		{"coln_o_xrate"},
		{"coln_n_xrate"},
		{"coln_item_desc"},
		{"coln_due_date"},
		{"coln_status"},
		{"coln_bonus_flag"},
		{"coln_hide_flag"},
		{"coln_hhah_hash"},
		{"coln_stat_flag"}
	};

	struct tag_colnRecord
	{
		long	hhcl_hash;
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	incc_hash;
		long	hhsl_hash;
		long	hhdl_hash;
		char	crd_type [2];
		char	serial_no [26];
		int		cont_status;
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
		float	o_xrate;
		float	n_xrate;
		char	item_desc [41];
		Date	due_date;
		char	status [2];
		char	bonus_flag [2];
		char	hide_flag [2];
		long	hhah_hash;
		char	stat_flag [2];
	}	coln_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	62

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_barcode"},
		{"inmr_commodity"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_description2"},
		{"inmr_category"},
		{"inmr_quick_code"},
		{"inmr_abc_code"},
		{"inmr_abc_update"},
		{"inmr_serial_item"},
		{"inmr_lot_ctrl"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_pack_size"},
		{"inmr_weight"},
		{"inmr_on_hand"},
		{"inmr_on_order"},
		{"inmr_committed"},
		{"inmr_backorder"},
		{"inmr_forward"},
		{"inmr_wo_qty_anti"},
		{"inmr_qc_qty"},
		{"inmr_source"},
		{"inmr_dec_pt"},
		{"inmr_ex_code"},
		{"inmr_bo_flag"},
		{"inmr_bo_release"},
		{"inmr_sellgrp"},
		{"inmr_buygrp"},
		{"inmr_disc_pc"},
		{"inmr_gst_pc"},
		{"inmr_min_quan"},
		{"inmr_max_quan"},
		{"inmr_safety_stock"},
		{"inmr_licence"},
		{"inmr_duty"},
		{"inmr_duty_amt"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
		{"inmr_outer_uom"},
		{"inmr_outer_size"},
		{"inmr_pc_off_trade"},
		{"inmr_scrap_pc"},
		{"inmr_tax_pc"},
		{"inmr_tax_amount"},
		{"inmr_ltd_sales"},
		{"inmr_active_status"},
		{"inmr_schg_flag"},
		{"inmr_dflt_bom"},
		{"inmr_dflt_rtg"},
		{"inmr_eoq"},
		{"inmr_qc_reqd"},
		{"inmr_stat_flag"}
	};

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		long	hhsi_hash;
		char	alpha_code [17];
		char	supercession [17];
		char	maker_no [17];
		char	alternate [17];
		char	barcode [17];
		char	commodity [15];
		char	_class [2];
		char	description [41];
		char	description2 [41];
		char	category [12];
		char	quick_code [9];
		char	abc_code [2];
		char	abc_update [2];
		char	serial_item [2];
		char	lot_ctrl [2];
		char	costing_flag [2];
		char	sale_unit [5];
		char	pack_size [6];
		float	weight;
		float	on_hand;
		float	on_order;
		float	committed;
		float	backorder;
		float	forward;
		float	wo_qty_anti;
		float	qc_qty;
		char	source [3];
		int		dec_pt;
		char	ex_code [4];
		char	bo_flag [2];
		char	bo_release [2];
		char	sellgrp [7];
		char	buygrp [7];
		float	disc_pc;
		float	gst_pc;
		float	min_quan;
		float	max_quan;
		float	safety_stock;
		char	licence [3];
		char	duty [3];
		float	duty_amt;
		long	std_uom;
		long	alt_uom;
		float	uom_cfactor;
		long	outer_uom;
		float	outer_size;
		float	pc_off_trade;
		float	scrap_pc;
		float	tax_pc;
		Money	tax_amount;
		float	ltd_sales;
		char	active_status [2];
		char	schg_flag [2];
		int		dflt_bom;
		int		dflt_rtg;
		float	eoq;
		char	qc_reqd [2];
		char	stat_flag [2];
	}	inmr_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	92

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
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
		{"cumr_buy_grp"},
		{"cumr_curr_code"},
		{"cumr_ctry_code"},
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
		{"cumr_ch_adr4"},
		{"cumr_dl_adr1"},
		{"cumr_dl_adr2"},
		{"cumr_dl_adr3"},
		{"cumr_dl_adr4"},
		{"cumr_contact_name"},
		{"cumr_contact2_name"},
		{"cumr_contact3_name"},
		{"cumr_spec_note1"},
		{"cumr_spec_note2"},
		{"cumr_phone_no"},
		{"cumr_fax_no"},
		{"cumr_telex"},
		{"cumr_post_code"},
		{"cumr_stop_credit"},
		{"cumr_date_stop"},
		{"cumr_total_days_sc"},
		{"cumr_credit_limit"},
		{"cumr_crd_prd"},
		{"cumr_chq_prd"},
		{"cumr_crd_flag"},
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
		{"cumr_hhsu_hash"},
		{"cumr_cat_sa_flag"},
		{"cumr_stmnt_flg"},
		{"cumr_freight_chg"},
		{"cumr_restock_fee"},
		{"cumr_nett_pri_prt"},
		{"cumr_reprint_inv"},
		{"cumr_cus_gl_type"},
		{"cumr_inst_fg1"},
		{"cumr_inst_fg2"},
		{"cumr_inst_fg3"},
		{"cumr_gl_ctrl_acct"},
		{"cumr_pay_method"},
		{"cumr_bk_name"},
		{"cumr_bk_branch"},
		{"cumr_bk_code"},
		{"cumr_bk_acct_no"},
		{"cumr_date_open"},
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
		{"cumr_item_codes"},
		{"cumr_mail_label"},
		{"cumr_letter"}
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	department [3];
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_name [41];
		char	dbt_acronym [10];
		char	acc_type [2];
		char	stmt_type [2];
		char	class_type [4];
		char	cont_type [4];
		char	buy_grp [4];
		char	curr_code [4];
		char	ctry_code [4];
		char	price_type [2];
		int		payment_flag;
		char	int_flag [2];
		char	bo_flag [2];
		char	bo_cons [2];
		int		bo_days;
		char	po_flag [2];
		char	sur_flag [2];
		char	ch_adr1 [41];
		char	ch_adr2 [41];
		char	ch_adr3 [41];
		char	ch_adr4 [41];
		char	dl_adr1 [41];
		char	dl_adr2 [41];
		char	dl_adr3 [41];
		char	dl_adr4 [41];
		char	contact_name [21];
		char	contact2_name [21];
		char	contact3_name [21];
		char	spec_note1 [41];
		char	spec_note2 [41];
		char	phone_no [16];
		char	fax_no [16];
		char	telex [11];
		char	post_code [11];
		char	stop_credit [2];
		Date	date_stop;
		int		total_days_sc;
		Money	credit_limit;
		char	crd_prd [4];
		char	chq_prd [3];
		char	crd_flag [2];
		char	credit_ref [21];
		char	bank_code [4];
		char	branch_code [21];
		char	area_code [3];
		char	sman_code [3];
		char	roy_type [4];
		char	disc_code [2];
		char	tax_code [2];
		char	tax_no [16];
		char	ch_to_ho_flg [2];
		long	ho_dbt_hash;
		long	hhsu_hash;
		char	cat_sa_flag [2];
		char	stmnt_flg [2];
		char	freight_chg [2];
		char	restock_fee [2];
		char	nett_pri_prt [2];
		char	reprint_inv [2];
		int		cus_gl_type;
		int		inst_fg1;
		int		inst_fg2;
		int		inst_fg3;
		char	gl_ctrl_acct [17];
		char	pay_method [2];
		char	bk_name [21];
		char	bk_branch [21];
		char	bk_code [16];
		char	bk_acct_no [16];
		Date	date_open;
		Date	date_lastinv;
		Date	date_lastpay;
		Money	amt_lastpay;
		Money	mtd_sales;
		Money	ytd_sales;
		Money	ord_value;
		Money	bo_current;
		Money	bo_per1;
		Money	bo_per2;
		Money	bo_per3;
		Money	bo_per4;
		Money	bo_fwd;
		int		od_flag;
		char	stat_flag [2];
		char	item_codes [2];
		char	mail_label [2];
		char	letter [2];
	}	cumr_rec;


FILE	*fout,
		*fsort;

char	*data = "data",
		*comm = "comm",
		*inmr = "inmr",
		*cohr = "cohr",
		*coln = "coln",
		*cumr = "cumr";

	char	*srt_offset[256];

	char	mlDownload [10][101];

#define	ASCII_PRINT (local_rec.ascii_print[0] == 'A' )
#define	CUST_SORT (local_rec.cust_print[0] == 'Y' )
#define SERIAL_NON (local_rec.serial_or_non[0] == 'Y')
#define SER (inmr_rec.serial_item[0] == 'Y')
#define	INVOICE	(cohr_rec.type[0] == 'I')

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	long	start_date;
	char	systemDate[11];
	long	lsystemDate;
	long	end_date;
 	char	OutputFile[15];
} local_rec;

static struct	var vars[] ={
	{1, LIN, "start_date", 5, 21, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Start Date       :", "Default = <today> ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start_date},
	{1, LIN, "end_date", 6, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date         :", "Default = <today> ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_date}, 
	{1, LIN, "OutputFile", 7, 21, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		"", "", "Output file name :", "Output file will be placed in /tmp ",
		YES, NO, JUSTLEFT, "", "", local_rec.OutputFile}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

char	data_str[400];

/*=======================
| Function Declarations |
=======================*/
void InitML (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void proc_file (void);
int  heading (int scn);
int  spec_valid (int field);
int  IntFindInmr (long hhbr_hash);
int  IntFindCumr (long hhcu_hash);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	OpenDB();

	InitML ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	crsr_on();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == FALSE)
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

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_display (1);
		edit (1);
		if (restart)
			continue;

		proc_file ();

	}	/* end of input control loop	*/

	shutdown_prog();
    return (EXIT_SUCCESS);
}

void
InitML (
 void)
{
	strcpy (mlDownload [1], ML ("UNLOAD"));
	strcpy (mlDownload [2], ML ("Unable to locate product in master file"));
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no5");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inmr);
	abc_dbclose (data);
}

void
proc_file (
 void)
{
	char	OutputFile[100];
	double	SalePrice;
	double	TotalInvoice;
	double	LTotalInvoice;


	dsp_screen ("Writing transactions.", comm_rec.tco_no, 
												comm_rec.tco_name);

	sprintf(OutputFile, "/tmp/%s", clip(local_rec.OutputFile));
	if ((fout = fopen (OutputFile, "w+")) == NULL)
			file_err (errno, OutputFile, "fopen");

	strcpy (cohr_rec.co_no, comm_rec.tco_no);
	strcpy (cohr_rec.br_no, comm_rec.test_no);
	strcpy (cohr_rec.type,  "I");
	cohr_rec.date_raised	=	local_rec.start_date;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.tco_no) &&
				  !strcmp (cohr_rec.br_no, comm_rec.test_no) &&
					cohr_rec.type[0] == 'I' &&
					(cohr_rec.date_raised <= local_rec.end_date))
	{
		dsp_process ("Transaction : ", cohr_rec.inv_no);

		cc = IntFindCumr (cohr_rec.hhcu_hash);
		if (cc)
		{
			sprintf (cumr_rec.dbt_no, "??????");
			sprintf (cumr_rec.dbt_name, 
							"Unable to locate customer in master file");
		}
	
		coln_rec.hhco_hash 	=	cohr_rec.hhco_hash;
		coln_rec.line_no	=	0;
		cc = find_rec (coln, &coln_rec, GTEQ, "r");
		while (!cc && cohr_rec.hhco_hash == coln_rec.hhco_hash)
		{
			cc = IntFindInmr (coln_rec.hhbr_hash);
			if (cc)
			{
				sprintf (inmr_rec.item_no, "****UNKNOWN****");
				sprintf (inmr_rec. description, 
								"Unable to locate product in master file.");
			}
			if (inmr_rec._class[0] == 'N' || inmr_rec._class[0] == 'Z' ||
				coln_rec.q_order == 0.00)
			{
				cc = find_rec (coln, &coln_rec, NEXT, "r");
				continue;
			}
			SalePrice = (coln_rec.gross - coln_rec.amt_disc) / coln_rec.q_order;
	
			TotalInvoice	=	cohr_rec.gross + 
								cohr_rec.tax +
								cohr_rec.gst + 
								cohr_rec.insurance +
								cohr_rec.freight +
								cohr_rec.other_cost[0] +
								cohr_rec.other_cost[1] +
								cohr_rec.other_cost[2] +
								cohr_rec.sos;
			TotalInvoice	-=	cohr_rec.disc;

			if (cohr_rec.exch_rate == 0.00)
				cohr_rec.exch_rate = 1.00;

			LTotalInvoice	=	TotalInvoice / cohr_rec.exch_rate;
								
			fprintf (fout, "%s|%s|%s|%s|%s|%-3.3s|%s|%s|%10.2f|%s|%10.2f|%s|%10.4f|%14.2f|%14.2f|\n", 
						DateToString(cohr_rec.date_raised),
						cohr_rec.inv_no,
						cumr_rec.dbt_no,
						cumr_rec.dbt_name,
						coln_rec.sman_code,
						cohr_rec.pay_terms,
						cohr_rec.area_code,
						inmr_rec.item_no,
						coln_rec.q_order,
						inmr_rec.sale_unit,
						DOLLARS (SalePrice),
						cumr_rec.curr_code,
						cohr_rec.exch_rate,
						DOLLARS (TotalInvoice),
						DOLLARS (LTotalInvoice));

			cc = find_rec (coln, &coln_rec, NEXT, "r");
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	strcpy (cohr_rec.co_no, comm_rec.tco_no);
	strcpy (cohr_rec.br_no, comm_rec.test_no);
	strcpy (cohr_rec.type,  "C");
	cohr_rec.date_raised	=	local_rec.start_date;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp( cohr_rec.co_no, comm_rec.tco_no) &&
				  !strcmp(cohr_rec.br_no, comm_rec.test_no) &&
					cohr_rec.type[0] == 'C' &&
					(cohr_rec.date_raised <= local_rec.end_date))
	{
		dsp_process ("Transaction : ", cohr_rec.inv_no);

		cc = IntFindCumr (cohr_rec.hhcu_hash);
		if (cc)
		{
			sprintf (cumr_rec.dbt_no, "??????");
			sprintf (cumr_rec.dbt_name, 
							"Unable to locate customer in master file");
		}
	
		coln_rec.hhco_hash 	=	cohr_rec.hhco_hash;
		coln_rec.line_no	=	0;
		cc = find_rec (coln, &coln_rec, GTEQ, "r");
		while (!cc && cohr_rec.hhco_hash == coln_rec.hhco_hash)
		{
			cc = IntFindInmr (coln_rec.hhbr_hash);
			if (cc)
			{
				sprintf (inmr_rec.item_no, "****%-7.7s****", mlDownload [1]);
				strcpy (inmr_rec.description, mlDownload [2]);
			}
			if (inmr_rec._class[0] == 'N' || inmr_rec._class[0] == 'Z' ||
				coln_rec.q_order == 0.00)
			{
				cc = find_rec (coln, &coln_rec, NEXT, "r");
				continue;
			}
			SalePrice = (coln_rec.gross - coln_rec.amt_disc) / coln_rec.q_order;

			TotalInvoice	=	cohr_rec.gross + 
								cohr_rec.tax +
								cohr_rec.gst + 
								cohr_rec.insurance +
								cohr_rec.freight +
								cohr_rec.other_cost[0] +
								cohr_rec.other_cost[1] +
								cohr_rec.other_cost[2] +
								cohr_rec.sos;
			TotalInvoice	-=	cohr_rec.disc;
			if (cohr_rec.exch_rate == 0.00)
				cohr_rec.exch_rate = 1.00;

			LTotalInvoice	=	TotalInvoice / cohr_rec.exch_rate;
								
			fprintf (fout, "%s|%s|%s|%s|%s|%-3.3s|%s|%s|%10.2f|%s|%10.2f|%s|%10.4f|%14.2f|%14.2f|\n", 
						DateToString(cohr_rec.date_raised),
						cohr_rec.inv_no,
						cumr_rec.dbt_no,
						cumr_rec.dbt_name,
						coln_rec.sman_code,
						cohr_rec.pay_terms,
						cohr_rec.area_code,
						inmr_rec.item_no,
						coln_rec.q_order,
						inmr_rec.sale_unit,
						DOLLARS (SalePrice),
						cumr_rec.curr_code,
						cohr_rec.exch_rate,
						DOLLARS (TotalInvoice),
						DOLLARS (LTotalInvoice));

			cc = find_rec (coln, &coln_rec, NEXT, "r");
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	fclose (fout);
}

int	
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		
		clear ();
		rv_pr (ML(mlSoMess226), 29, 0, 1);
		
		move (0,1);
		line (80);

		box (0, 4, 80, 3);

		move (0, 20);
		line (78);
		print_at (21,0,ML(mlStdMess038),  comm_rec.tco_no, comm_rec.tco_name);

		/* Reset this variable for new screen NOT page */
		line_cnt = 0; 
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("start_date"))
	{
		if (dflt_used)
			local_rec.start_date = local_rec.lsystemDate;
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("end_date"))
	{
		if (dflt_used)
			local_rec.end_date = local_rec.lsystemDate;

		if (local_rec.end_date < local_rec.start_date)
		{
			/*print_mess ("\007End date is before start date ");*/
			print_mess (ML(mlStdMess026));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}
		else
			return (EXIT_SUCCESS);
	}	
	return (EXIT_SUCCESS);
}

int
IntFindInmr (
 long	hhbr_hash)
{
	if (inmr_rec.hhbr_hash == hhbr_hash)
		return (EXIT_SUCCESS);

	inmr_rec.hhbr_hash = hhbr_hash;
	return (find_rec (inmr, &inmr_rec, EQUAL, "r"));
}

int
IntFindCumr (
 long	hhcu_hash)
{
	cumr_rec.hhcu_hash = hhcu_hash;
	return (find_rec ("cumr", &cumr_rec, EQUAL, "r"));
}

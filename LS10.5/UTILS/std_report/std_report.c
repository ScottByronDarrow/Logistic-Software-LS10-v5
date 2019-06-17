/*=====================================================================
|  Copyright (C) 1988 - 1995 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( std_report.c   )                                 |
|  Program Desc  : ( Standard Report Generator.                   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sumr, cumr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 20/12/88         |
|---------------------------------------------------------------------|
|  Date Modified : (20/12/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (06/12/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (04/11/92)      | Modified  by  : Anneliese Allen. |
|  Date Modified : (21/09/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (07/03/95)	   | Modified  by  : Ronnel L Amanca  |
|  Date Modified : (23/08/95)	   | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (23/02/96)	   | Modified  by  : Scott B Darrow.  |
|  Date Modified : (08/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (05/06/1998)    | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : (06/12/91) - Update for cumr phone_no length.      |
|                : SC 6297 PSL.                                       |
|                :                                                    |
|  (04/11/92)    : Allow for printing of currency code PSL 7896.      |
|                :                                                    |
|  (21/09/93)    : HGP 9864. Increase sumr_cont_no to 15 chars.       |
|                :                                                    |
|  (07/03/95)    : SMF     . Remove inmr_price 1 -5.				  |
|                :                                                    |
|  (07/03/95)    : SMF 12405 Complete clean up and test of prog.      |
|                :                                                    |
|  (23/02/96)    : PDL Updated to fix bugs with double/float.         |
|  (08/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                                      |
|  (05/06/1998)  : BFS SC#124 - Fixed Bug regarding the salesman      |
|                : field and included the latest schema of cumr,inmr  |
|                : and sumr in the program.                           |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: std_report.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/std_report/std_report.c,v 5.4 2002/07/18 07:18:30 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_utils_mess.h>
#include <ml_std_mess.h>

#define	VIEW		std_list[local_rec.std_offset]
#define	VIEW_SIZE	256

#define	PRINT_WIDTH	158
#define	min(a,b)	((a) < (b)) ? (a) : (b)
#define	CUSTOMER	( rep_type[0] == 'C' )
#define	SUPPLIER	( rep_type[0] == 'S' )
#define	ITEM		( rep_type[0] == 'I' )

	int 	valid_sman = 0;
	int		add_line = 0;
	int		envDbCo = 1;
	int		envDbFind = 1;
	int		cr_find = 1;
	int		no_fields;
	int		report_width = 1;
	char	rep_type[2];
	char	branchNumber[3];
	FILE	*popen(const char *, const char *);
	FILE	*fout;

	struct	dbview	std_list[VIEW_SIZE];
	struct	_fields	
	{
		char	*_descr;
		char	*_field;
	};

	/*=====================
	| System Common File. |
	=====================*/
#define	COMM_NO_FIELDS	10

	struct dbview comm_list[COMM_NO_FIELDS] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
	};

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tprice[5][16];
	} comm_rec;



	/*======+
	 | inmr |
	 +======*/
#define	INMR_NO_FIELDS	64

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
		{"inmr_reorder"},
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
		{"inmr_stat_flag"},
		{"inmr_min_sell_pric"}
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
		char	class [2];
		char	description [41];
		char	description2 [41];
		char	category [12];
		char	quick_code [9];
		char	abc_code [2];
		char	reorder [2];
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
		Money	min_sell_pric;
	}	inmr_rec;


	struct	_fields	inmr_fields[] = {
		{"Company No       ", "inmr_co_no"},
		{"Item Number.     ", "inmr_item_no"},
		{"Item Hash.       ", "inmr_hhbr_hash"},
		{"Alternate Hash   ", "inmr_hhsi_hash"},
		{"Alpha Code.      ", "inmr_alpha_code"},
		{"Supercession No  ", "inmr_supercession"},
		{"Maker Number.    ", "inmr_maker_no"},
		{"Alternate No.    ", "inmr_alternate"},
		{"Bar Code No.     ", "inmr_barcode"},
		{"Commodity Code   ", "inmr_commodity"},
		{"Class            ", "inmr_class"},
		{"Description      ", "inmr_description"},
		{"Description2     ", "inmr_description2"},
		{"Category         ", "inmr_category"},
		{"Quick Code.      ", "inmr_quick_code"},
		{"Abc Code.        ", "inmr_abc_code"},
		{"Reorder          ", "inmr_reorder"},
		{"ABC update Y/N   ", "inmr_abc_update"},
		{"Serial item Y/N  ", "inmr_serial_item"},
		{"Lot controled Y/N", "inmr_lot_ctrl"},
		{"Costing Flag     ", "inmr_costing_flag"},
		{"Sale Unit.       ", "inmr_sale_unit"},
		{"Pack Size        ", "inmr_pack_size"},
		{"Weight           ", "inmr_weight"},
		{"On Hand          ", "inmr_on_hand"},
		{"On Order         ", "inmr_on_order"},
		{"On Committed     ", "inmr_committed"},
		{"On Backorder     ", "inmr_backorder"},
		{"On Forward order ", "inmr_forward"},
		{"W/Order Qty Anti.", "inmr_wo_qty_anti"},
		{"On QC.           ", "inmr_qc_qty"},
		{"Source.          ", "inmr_source"},
		{"Decimal Places   ", "inmr_dec_pt"},
		{"Extra Desc. Code ", "inmr_ex_code"},
		{"Backorders Y/N   ", "inmr_bo_flag"},
		{"Backorder release", "inmr_bo_release"},
		{"Selling group    ", "inmr_sellgrp"},
		{"Buying Group     ", "inmr_buygrp"},
		{"Discount %       ", "inmr_disc_pc"},
		{"GST %            ", "inmr_gst_pc"},
		{"Minimum Qty      ", "inmr_min_quan"},
		{"Maximum Qty      ", "inmr_max_quan"},
		{"Safety Stock     ", "inmr_safety_stock"},
		{"Licence          ", "inmr_licence"},
		{"Duty             ", "inmr_duty"},
		{"Duty Amount      ", "inmr_duty_amt"},
		{"UOM - STD Hash   ", "inmr_std_uom"},
		{"UOM - ALT Hash   ", "inmr_alt_uom"},
		{"UOM conversion   ", "inmr_uom_cfactor"},
		{"UOM - Outer.     ", "inmr_outer_uom"},
		{"UOM - Outer Conv.", "inmr_outer_size"},
		{"Pc Off Trade     ", "inmr_pc_off_trade"},
		{"Scrap Percent    ", "inmr_scrap_pc"},
		{"Tax %            ", "inmr_tax_pc"},
		{"Tax Amount.      ", "inmr_tax_amount"},
		{"YTD Sales.       ", "inmr_ltd_sales"},
		{"Active Status    ", "inmr_active_status"},
		{"Surcharge Flag   ", "inmr_schg_flag"},
		{"Default UOM      ", "inmr_dflt_bom"},
		{"Default Rtg.     ", "inmr_dflt_rtg"},
		{"Eco. Order Qty   ", "inmr_eoq"},
		{"QC Required Y/N  ", "inmr_qc_reqd"},
		{"Status Flag      ", "inmr_stat_flag"},
		{"Min.Selling Price", "inmr_min_sell_pric"},
	};


	/*======+
	 | cumr |
	 +======*/
#define	CUMR_NO_FIELDS	93

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
		{"cumr_letter"},
		{"cumr_cash_flag"}
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
		char	cash_flag [2];
	}	cumr_rec;


	struct	_fields	cumr_fields[] = {
		{"Company Number.  ", "cumr_co_no"},
		{"Branch Number.   ", "cumr_est_no"},
		{"Department No.   ", "cumr_department"},
		{"Customer No.     ", "cumr_dbt_no"},
		{"Customer Hash    ", "cumr_hhcu_hash"},
		{"Customer Name    ", "cumr_dbt_name"},
		{"Customer Acronym ", "cumr_dbt_acronym"},
		{"Account Type     ", "cumr_acc_type"},
		{"Statement Type.  ", "cumr_stmt_type"},
		{"Class Type       ", "cumr_class_type"},
		{"Contract Type    ", "cumr_cont_type"},
		{"Buying Group     ", "cumr_buy_grp"},
		{"Currency Code    ", "cumr_curr_code"},
		{"Country Code     ", "cumr_ctry_code"},
		{"Price Type       ", "cumr_price_type"},
		{"Payment Flag     ", "cumr_payment_flag"},
		{"Interest Flag    ", "cumr_int_flag"},
		{"B/O Allowed      ", "cumr_bo_flag"},
		{"Consolidation bo.", "cumr_bo_cons"},
		{"No Days of BO.   ", "cumr_bo_days"},
		{"P/O Required     ", "cumr_po_flag"},
		{"Surcharge flag.  ", "cumr_sur_flag"},
		{"Charge Addr. 1   ", "cumr_ch_adr1"},
		{"Charge Addr. 2   ", "cumr_ch_adr2"},
		{"Charge Addr. 3   ", "cumr_ch_adr3"},
		{"Charge Addr. 4   ", "cumr_ch_adr4"},
		{"Delivery Addr. 1 ", "cumr_dl_adr1"},
		{"Delivery Addr. 2 ", "cumr_dl_adr2"},
		{"Delivery Addr. 3 ", "cumr_dl_adr3"},
		{"Delivery Addr. 4 ", "cumr_dl_adr4"},
		{"Contact Name     ", "cumr_contact_name"},
		{"Contact2 Name    ", "cumr_contact2_name"},
		{"Contact3 Name    ", "cumr_contact3_name"},
		{"Spec. Note 1     ", "cumr_spec_note1"},
		{"Spec. Note 2     ", "cumr_spec_note2"},
		{"Phone Number     ", "cumr_phone_no"},
		{"Fax Number       ", "cumr_fax_no"},
		{"Telex Number     ", "cumr_telex"},
		{"Postal Code      ", "cumr_post_code"},
		{"Stop Credit Flag ", "cumr_stop_credit"},
		{"Date Stop Credit ", "cumr_date_stop"},
		{"Total Days SC    ", "cumr_total_days_sc"},
		{"Credit Limit     ", "cumr_credit_limit"},
		{"Credit Period    ", "cumr_crd_prd"},
		{"Cheque Period    ", "cumr_chq_prd"},
		{"Credit Flag      ", "cumr_crd_flag"},
		{"Credit Ref.      ", "cumr_credit_ref"},
		{"Bank Code        ", "cumr_bank_code"},
		{"Branch Code.     ", "cumr_branch_code"},
		{"Area Code        ", "cumr_area_code"},
		{"Salesman Code    ", "cumr_sman_code"},
		{"Royalty Type     ", "cumr_roy_type"},
		{"Discount Code    ", "cumr_disc_code"},
		{"Tax Code         ", "cumr_tax_code"},
		{"Tax Number       ", "cumr_tax_no"},
		{"Chrg.to Head Off.", "cumr_ch_to_ho_flg"},
		{"Dbt.Head Off.Hash", "cumr_ho_dbt_hash"},
		{"Suppliers Hash   ", "cumr_hhsu_hash"},
		{"Cat. SA Flag     ", "cumr_cat_sa_flag"},
		{"Statement Flag   ", "cumr_stmnt_flg"},
		{"Freight Charge   ", "cumr_freight_chg"},
		{"Restocking Fee   ", "cumr_restock_fee"},
		{"Nett Price Used  ", "cumr_nett_pri_prt"},
		{"Reprint Invoice  ", "cumr_reprint_inv"},
		{"Customer GL Type ", "cumr_cus_gl_type"},
		{"Instruction Fg1  ", "cumr_inst_fg1"},
		{"Instruction Fg2  ", "cumr_inst_fg2"},
		{"Instruction Fg3  ", "cumr_inst_fg3"},
		{"GL Control Acct. ", "cumr_gl_ctrl_acct"},
		{"Payment Method   ", "cumr_pay_method"},
		{"Bank Name .      ", "cumr_bk_name"},
		{"Bank Branch      ", "cumr_bk_branch"},
		{"Bank Code        ", "cumr_bk_code"},
		{"Bank Acct. No.   ", "cumr_bk_acct_no"},
		{"Date Open        ", "cumr_date_open"},
		{"Date Last Sale   ", "cumr_date_lastinv"},
		{"Date Last Pay    ", "cumr_date_lastpay"},
		{"Amount Last Pay  ", "cumr_amt_lastpay"},
		{"MTD Sales Value  ", "cumr_mtd_sales"},
		{"YTD Sales Value  ", "cumr_ytd_sales"},
		{"AR Order Value   ", "cumr_ord_value"},
		{"AR Current       ", "cumr_bo_current"},
		{"AR Overdue 1     ", "cumr_bo_per1"},
		{"AR Overdue 2     ", "cumr_bo_per2"},
		{"AR Overdue 3     ", "cumr_bo_per3"},
		{"AR Overdue 4     ", "cumr_bo_per4"},
		{"AR Forward       ", "cumr_bo_fwd"},
		{"AR Overdue Flag  ", "cumr_od_flag"},
		{"Status Flag      ", "cumr_stat_flag"},
		{"Item Codes       ", "cumr_item_codes"},
		{"Mail Label       ", "cumr_mail_label"},
		{"Letter           ", "cumr_letter"},
		{"Cash Flag        ", "cumr_cash_flag"},
	};


	/*======+
	 | sumr |
	 +======*/
#define	SUMR_NO_FIELDS	50

	struct dbview	sumr_list [SUMR_NO_FIELDS] =
	{
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
		{"sumr_acc_type"},
		{"sumr_debtor_no"},
		{"sumr_adr1"},
		{"sumr_adr2"},
		{"sumr_adr3"},
		{"sumr_adr4"},
		{"sumr_cont_name"},
		{"sumr_cont2_name"},
		{"sumr_cont3_name"},
		{"sumr_cont_no"},
		{"sumr_curr_code"},
		{"sumr_ctry_code"},
		{"sumr_pay_terms"},
		{"sumr_disc"},
		{"sumr_sic1"},
		{"sumr_sic2"},
		{"sumr_sic3"},
		{"sumr_gl_ctrl_acct"},
		{"sumr_hold_payment"},
		{"sumr_fax_no"},
		{"sumr_pay_method"},
		{"sumr_gst_reg_no"},
		{"sumr_bank"},
		{"sumr_bank_branch"},
		{"sumr_bank_code"},
		{"sumr_bank_acct_no"},
		{"sumr_date_opened"},
		{"sumr_sup_pri"},
		{"sumr_type_code"},
		{"sumr_ame"},
		{"sumr_mtd_exp"},
		{"sumr_ytd_exp"},
		{"sumr_bo_curr"},
		{"sumr_bo_per1"},
		{"sumr_bo_per2"},
		{"sumr_bo_per3"},
		{"sumr_qa_status"},
		{"sumr_qa_expiry"},
		{"sumr_mail_label"},
		{"sumr_letter"},
		{"sumr_tax_code"},
		{"sumr_remm_prn"},
		{"sumr_stat_flag"},
		{"sumr_ship_method"}
	};

	struct tag_sumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	crd_no [7];
		long	hhsu_hash;
		char	crd_name [41];
		char	acronym [10];
		char	acc_type [2];
		char	debtor_no [13];
		char	adr1 [41];
		char	adr2 [41];
		char	adr3 [41];
		char	adr4 [41];
		char	cont_name [21];
		char	cont2_name [21];
		char	cont3_name [21];
		char	cont_no [16];
		char	curr_code [4];
		char	ctry_code [4];
		char	pay_terms [4];
		float	disc;
		int		sic1;
		int		sic2;
		int		sic3;
		char	gl_ctrl_acct [17];
		char	hold_payment [2];
		char	fax_no [15];
		char	pay_method [2];
		char	gst_reg_no [16];
		char	bank [21];
		char	bank_branch [21];
		char	bank_code [16];
		char	bank_acct_no [16];
		Date	date_opened;
		int		sup_pri;
		char	type_code [7];
		Money	ame;
		Money	mtd_exp;
		Money	ytd_exp;
		Money	bo_curr;
		Money	bo_per1;
		Money	bo_per2;
		Money	bo_per3;
		char	qa_status [2];
		Date	qa_expiry;
		char	mail_label [2];
		char	letter [2];
		char	tax_code [2];
		char	remm_prn [2];
		char	stat_flag [2];
		char	ship_method [2];
	}	sumr_rec;


	struct	_fields	sumr_fields[] = {
		{"Company No       ", "sumr_co_no"},
		{"Branch No.       ", "sumr_est_no"},
		{"Supplier No.     ", "sumr_crd_no"},
		{"Supplier Hash    ", "sumr_hhsu_hash"},
		{"Supplier Name    ", "sumr_crd_name"},
		{"Supplier Acronym ", "sumr_acronym"},
		{"Account Type     ", "sumr_acc_type"},
		{"Customer Number. ", "sumr_debtor_no"},
		{"Supplier Addr. #1", "sumr_adr1"},
		{"Supplier Addr. #2", "sumr_adr2"},
		{"Supplier Addr. #3", "sumr_adr3"},
		{"Supplier Addr. #4", "sumr_adr4"},
		{"Contact Name     ", "sumr_cont_name"},
		{"Contact2 Name    ", "sumr_cont2_name"},
		{"Contact3 Name	  ", "sumr_cont3_name"}, 
		{"Contact Number.  ", "sumr_cont_no"},
		{"Currency Code.   ", "sumr_curr_code"},
		{"Country Code.    ", "sumr_ctry_code"},
		{"Payment Terms    ", "sumr_pay_terms"},
		{"Discount Code.   ", "sumr_disc"},
		{"Special inst #1  ", "sumr_sic1"},
		{"Special inst #2  ", "sumr_sic2"},
		{"Special inst #3  ", "sumr_sic3"},
		{"G/L Control Acct.", "sumr_gl_ctrl_acct"},
		{"Hold payment flag", "sumr_hold_payment"},
		{"Fax Number.      ", "sumr_fax_no"},
		{"Payment Method   ", "sumr_pay_method"},
		{"Tax Regist. No.  ", "sumr_gst_reg_no"},
		{"Bank Number.     ", "sumr_bank"},
		{"Branch Code.     ", "sumr_bank_branch"},
		{"Bank Code        ", "sumr_bank_code"},
		{"Bank Account No. ", "sumr_bank_acct_no"},
		{"Opening Date.    ", "sumr_date_opened"},
		{"Supplier Priority", "sumr_sup_pri"},
		{"Supplier type    ", "sumr_type_code"},
		{"Amount owing     ", "sumr_ame"},
		{"MTD expenses     ", "sumr_mtd_exp"},
		{"YTD expenses     ", "sumr_ytd_exp"},
		{"AP - Current     ", "sumr_bo_curr"},
		{"AP - Overdue 1   ", "sumr_bo_per1"},
		{"AP - Overdue 2   ", "sumr_bo_per2"},
		{"AP - Overdue 3   ", "sumr_bo_per3"},
		{"QA status        ", "sumr_qa_status"},
		{"QA Expiry        ", "sumr_qa_expiry"},
		{"Mail Label       ", "sumr_mail_label"},
		{"Letter           ", "sumr_letter"},
		{"Tax Code         ", "sumr_tax_code"},
		{"Remmittance Prn. ", "sumr_prn"},
		{"Status Flag      ", "sumr_stat_flag"},
		{"Shipment Method  ", "sumr_ship_method"}
	};

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

	char	*data	= "data",
			*comm	= "comm",
			*cumr	= "cumr",
			*exsf	= "exsf",
			*inmr	= "inmr",
			*sumr	= "sumr";

	union	_type_var
	{
		char	*cptr;
		int		*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} vble;

	char	*single_bar = "----------------------------------------------------------------------------------------------------------------------------------------------------------------";
	char	*rule_off = "================================================================================================================================================================";
	char	*over_flow = "****************************************";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	int		lpno;
	int		ncopies;
	char 	item_no[2][17];
	char 	dbt_no[2][7];
	char 	crd_no[2][7];
	char 	description[2][41];
	char	rep_heading[61];
	char	std_descr[18];
	char	std_heading[41];
	int		std_width;
	char	std_field[21];
	int		std_offset;
	char	systemDate[11];
	long	lsystemDate;
	char	s_sman[3];
	char	e_sman[3];
	char	s_name[41];
	char	e_name[41];
} local_rec;


static	struct	var	vars[] =
{
	{1, LIN, "lpno",	 4, 16, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No. ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "ncopies",	 4, 55, INTTYPE,
		"NN", "          ",
		" ", "1", "# Copies ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ncopies},
	{1, LIN, "start_dbt_no",	 6, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dbt_no[0]},
	{1, LIN, "start_item_no",	 6, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "                ", "Start Item", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.item_no[0]},
	{1, LIN, "start_crd_no",	 6, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.crd_no[0]},
	{1, LIN, "start_description",	 7, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[0]},
	{1, LIN, "end_dbt_no",	 8, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Customer", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dbt_no[1]},
	{1, LIN, "end_item_no",	 8, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", "End Item", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.item_no[1]},
	{1, LIN, "end_crd_no",	 8, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.crd_no[1]},
	{1, LIN, "end_description",	 9, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[1]},
	{1, LIN, "s_sman",	 11, 16, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Salesman ", " Default is All ",
		ND, NO, JUSTLEFT, "", "", local_rec.s_sman},
	{1, LIN, "s_name",	 12, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_name},
	{1, LIN, "e_sman",	 13, 16, CHARTYPE,
		"UU", "          ",
		" ", "~~", "End Salesman   ", " Default is All ",
		ND, NO, JUSTLEFT, "", "", local_rec.e_sman},
	{1, LIN, "e_name",	14, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Name ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_name},
	{1, LIN, "rep_heading",	11, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Report Heading", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.rep_heading},

	{2, TAB, "std_descr",	MAXLINES, 0, CHARTYPE,
		"LLLLLLLLLLLLLLLLL", "          ",
		" ", " ", "   Description   ", " <DEL-LINE> - Delete Line, <INS-LINE> - Insert Line ",
		YES, NO,  JUSTLEFT, "", "", local_rec.std_descr},
	{2, TAB, "std_heading",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "  C o l u m n    D e s c r i p t i o n  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.std_heading},
	{2, TAB, "std_width",	 0, 0, INTTYPE,
		"NNN", "          ",
		" ", "", "   ", " ",
		 NI, NO, JUSTRIGHT, "1", "999", (char *)&local_rec.std_width},
	{2, TAB, "std_field",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.std_field},
	{2, TAB, "std_offset",	 0, 0, INTTYPE,
		"NNNN", "          ",
		" ", "", "   ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.std_offset},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindCumr.h>
#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  DeleteLine (void);
int  InsertLine (void);
void print_cols (void);
void down_shift (char *str);
void srch_field (char *key_val);
void Process (void);
void ProcCumr (void);
void ProcInmr (void);
void ProcSumr (void);
void PrintField (char *data_str);
void sman_srch (char *key_val);
int  heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;

	if (argc != 2)
	{
		/*-------------------------------------------------------
		| Usage : %-20.20s <report_type>						|
		|        %-20.20s <report_type> - C(ustomer Master File |
		|        %-20.20s               - I(nventory Master File|
		|        %-20.20s               - S(upplier Master File |
		-------------------------------------------------------*/
		print_at (0,0, ML(mlUtilsMess715), argv[0]);
		print_at (0,0, ML(mlUtilsMess716), " ");
		print_at (0,0, ML(mlUtilsMess717), " ");
		print_at (0,0, ML(mlUtilsMess718), " ");
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = StringToDate (local_rec.systemDate);

	SETUP_SCR (vars);

	switch (argv [1][0])
	{
	case	'C':
	case	'c':
		strcpy (rep_type, "C");
		FLD ("start_dbt_no")	=	YES;
		FLD ("end_dbt_no")		= 	YES;
		FLD ("s_sman")			=	YES;
		FLD ("e_sman")			=	YES;
		FLD ("s_name")			=	NA;
		FLD ("e_name")			=	NA;
		vars[label ( "rep_heading" )].row = 16;
		envDbCo = atoi (get_env ("DB_CO"));
		envDbFind  = atoi (get_env ("DB_FIND"));
		no_fields = CUMR_NO_FIELDS;
		break;

	case	'I':
	case	'i':
	    valid_sman = 1;	
		strcpy (rep_type, "I");
		FLD ("start_item_no")	= 	YES;
		FLD ("end_item_no")		= 	YES;
		FLD ("s_sman")			=	ND;
		FLD ("e_sman")			=	ND;
		FLD ("s_name")			=	ND;
		FLD ("e_name")			=	ND;
		no_fields = INMR_NO_FIELDS;
		break;

	case	'S':
	case	's':
	    valid_sman = 1;	
		strcpy (rep_type, "S");
		FLD ("start_crd_no")	= 	YES;
		FLD ("end_crd_no")		= 	YES;
		FLD ("s_sman")			=	ND;
		FLD ("e_sman")			=	ND;
		FLD ("s_name")			=	ND;
		FLD ("e_name")			=	ND;
		envDbCo = atoi (get_env ("CR_CO"));
		cr_find  = atoi (get_env ("CR_FIND"));
		no_fields = SUMR_NO_FIELDS;
		break;

	default:
		/*-------------------------------------------	
		|<report_type> - C(ustomer Master File		|
		|              - I(nventory Master File		|
		|              - S(upplier Master File		|
		-------------------------------------------	*/
		print_at(0,0, ML(mlUtilsMess716) );
		print_at(0,0, ML(mlUtilsMess717) );
		print_at(0,0, ML(mlUtilsMess718) );
		return (EXIT_FAILURE);
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty (); 			/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNumber, (envDbCo == 0) ? " 0" : comm_rec.test_no);

	switch (rep_type[0])
	{
	case	'C':
		for (i = 0;i < CUMR_NO_FIELDS;i++)
			std_list [i] = cumr_list [i];
		break;

	case	'I':
		for (i = 0;i < INMR_NO_FIELDS;i++)
			std_list [i] = inmr_list [i];
		break;

	case	'S':
		for (i = 0;i < SUMR_NO_FIELDS;i++)
			std_list [i] = sumr_list [i];
		break;

	default:
		break;
	}

	prog_exit 	= 0;
	while (!prog_exit)
	{
		entry_exit 	 = 0;
		edit_exit 	 = 0;
		prog_exit 	 = 0;
		restart 	 = 0;
		search_ok 	 = 1;
		report_width = 1;

		init_vars (1);
		init_vars (2);
		lcount[2] = 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
	
		heading (2);
		entry (2);
		if (restart)
			continue;
	
		prog_status = !(ENTRY);
		edit_all ();
		if (restart)
			continue;
	
		if (lcount[2] == 0)
		{
			clear ();
			/*----------------------------------------
			| Fields must be selected to run report. |
			----------------------------------------*/
			rv_pr ( ML(mlUtilsMess086),0,0,1);
			sleep (10);
		}
		else
			Process ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}



/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind == 0) ? "cumr_id_no" 
															  : "cumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (cr_find == 0) ? "sumr_id_no" 
															  : "sumr_id_no3");
	open_rec (exsf, exsf_list, exsf_no_fields, "exsf_id_no");
}	



/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (exsf);
	abc_dbclose (data);
}



int
spec_valid (
 int field)
{
	int		i;
	char	buffer[21];

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*------------------
			| Invalid Printer. |
			------------------*/
			print_mess ( ML(mlStdMess020) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_dbt_no"))
	{
		if (FLD ("start_dbt_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.description[0], "%-40.40s","** First Customer **");
			DSP_FLD ("start_description");
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.dbt_no [0]));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			/*---------------------
			| Customer Not found. |
			---------------------*/
			print_mess ( ML(mlStdMess021) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.description [0], cumr_rec.dbt_name);
		DSP_FLD ("start_description");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_item_no"))
	{
		if (FLD ("start_item_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			sprintf (local_rec.description[0], "%-40.40s", "** First Item **");
			DSP_FLD ("start_description");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		cc = FindInmr (comm_rec.tco_no, local_rec.item_no [0], 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.tco_no);
			strcpy (inmr_rec.item_no, local_rec.item_no [0]);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.description [0], inmr_rec.description);
		DSP_FLD ("start_description");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_crd_no"))
	{
		if (FLD ("start_crd_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.description[0], "%-40.40s",
					 "** First Supplier **");
			DSP_FLD ("start_description");
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,  comm_rec.tco_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.crd_no[0]));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			/*---------------------
			| Supplier Not found. |
			---------------------*/
			print_mess ( ML(mlStdMess022) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.description [0], sumr_rec.crd_name);
		DSP_FLD ("start_description");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_dbt_no"))
	{
		if (FLD ("end_dbt_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.description[1], "%-40.40s","** Last Customer **");
			DSP_FLD ("end_description");
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.dbt_no[1]));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess ( ML(mlStdMess021) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.description [1], cumr_rec.dbt_name);
		DSP_FLD ("end_description");
		return (EXIT_SUCCESS);
	}
			
	if (LCHECK ("end_item_no"))
	{
		if (FLD ("end_item_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		if (dflt_used)
		{
			sprintf (local_rec.description [1], "%-40.40s", "** Last Item **");
			DSP_FLD ("end_description");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.item_no [1], 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.tco_no);
			strcpy (inmr_rec.item_no, local_rec.item_no [1]);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.description [1], inmr_rec.description);
		DSP_FLD ("end_description");
		return (EXIT_SUCCESS);
	}
			
	if (LCHECK ("end_crd_no"))
	{
		if (FLD ("end_crd_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.description[1], "%-40.40s",
					 "** Last Supplier **");
			DSP_FLD ("end_description");
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,  comm_rec.tco_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, local_rec.crd_no[1]);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess ( ML(mlStdMess022) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.description [1], sumr_rec.crd_name);
		DSP_FLD ("end_description");
		return (EXIT_SUCCESS);
	}
			
	if (LCHECK ("std_descr"))
	{
		int		tmp_width;

		if (SRCH_KEY)
		{
			add_line = 0;
			srch_field (temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return (DeleteLine ());

		if (last_char == INSLINE)
			return (InsertLine ());

		add_line = 1;

		strcpy (buffer, local_rec.std_descr);
		down_shift (buffer);

		for (i = 0;i < no_fields;i++)
		{
			switch (rep_type [0])
			{
			case	'C':
				strcpy (err_str, cumr_fields[i]._descr);
				break;

			case	'I':
				strcpy (err_str, inmr_fields[i]._descr);
				break;

			case	'S':
				strcpy (err_str, sumr_fields[i]._descr);
				break;
			}
			down_shift (err_str);

			if (!strcmp (err_str, buffer))
			{
				switch (rep_type [0])
				{
				case	'C':
					sprintf (local_rec.std_descr, "%-17.17s",
							 cumr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 cumr_fields[i]._field);
					break;

				case	'I':
					sprintf (local_rec.std_descr, "%-17.17s",
							 inmr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 inmr_fields[i]._field);
					break;

				case	'S':
					sprintf (local_rec.std_descr, "%-17.17s",
							 sumr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 sumr_fields[i]._field);
					break;
				}

				local_rec.std_offset = i;
				tmp_width = (std_list[i].vwtype == CHARTYPE) ? std_list[i].vwlen : 10;
				if (prog_status == ENTRY)
					strcpy (local_rec.std_heading, "");
				if (line_cnt < lcount[2])
					report_width -= (local_rec.std_width + 1);

				if (tmp_width >= strlen (clip (local_rec.std_heading)))
					local_rec.std_width = tmp_width;
				else
					local_rec.std_width = strlen (clip (local_rec.std_heading));


				DSP_FLD ("std_descr");
				DSP_FLD ("std_field");
				DSP_FLD ("std_width");
				report_width += (local_rec.std_width + 1);
				if (report_width > PRINT_WIDTH)
				{
					/*---------------------------------------
					| Specified report data exceeds maximum |
					| report width of %d chars				|
					---------------------------------------*/
					sprintf (err_str, ML(mlUtilsMess082), PRINT_WIDTH);
					print_mess ( err_str );
					sleep (sleepTime);
				}
				return (EXIT_SUCCESS);
			}
		}

		/*----------------
		| Invalid Field. |
		----------------*/
		print_mess ( ML(mlUtilsMess083) );
		sleep (sleepTime);
		clear_mess ();
		
		return (EXIT_FAILURE);
	}

	if (LCHECK ("std_heading"))
	{
		int		tmp_width;

		if (dflt_used)
		{
			sprintf (local_rec.std_heading, "%-40.40s", local_rec.std_descr);
			display_field (field);
		}
		strcpy (buffer, local_rec.std_descr);
		down_shift (buffer);

		for (i = 0;i < no_fields;i++)
		{
			switch (rep_type [0])
			{
			case	'C':
				strcpy (err_str, cumr_fields[i]._descr);
				break;

			case	'I':
				strcpy (err_str, inmr_fields[i]._descr);
				break;

			case	'S':
				strcpy (err_str, sumr_fields[i]._descr);
				break;
			}
			down_shift (err_str);

			if (!strcmp (err_str, buffer))
			{
				/***
				switch (rep_type [0])
				{
				case	'C':
					sprintf (local_rec.std_descr, "%-17.17s",
							 cumr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 cumr_fields[i]._field);
					break;

				case	'I':
					sprintf (local_rec.std_descr, "%-17.17s",
							 inmr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 inmr_fields[i]._field);
					break;

				case	'S':
					sprintf (local_rec.std_descr, "%-17.17s",
							 sumr_fields[i]._descr);
					sprintf (local_rec.std_field, "%-20.20s",
							 sumr_fields[i]._field);
					break;
				}
				***/
				report_width -= (local_rec.std_width + 1);

				tmp_width = (std_list[i].vwtype == CHARTYPE) ? std_list[i].vwlen : 10;
				if (tmp_width >= strlen (clip (local_rec.std_heading)))
					local_rec.std_width = tmp_width;
				else
					local_rec.std_width = strlen (clip (local_rec.std_heading));

				report_width += (local_rec.std_width + 1);
				if (report_width > PRINT_WIDTH)
				{
					/*---------------------------------------
					| Specified report data exceeds maximum |
					| report width of %d chars				|
					---------------------------------------*/
					sprintf (err_str, ML(mlUtilsMess082), PRINT_WIDTH);
					print_mess (err_str);
					sleep (sleepTime);
				}
				DSP_FLD ("std_width");

				putval (line_cnt);

				print_cols ();

				getval (line_cnt);

				return (EXIT_SUCCESS);
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("std_width"))
	{
		putval (line_cnt);

		print_cols ();

		getval (line_cnt);

		return (EXIT_SUCCESS);
	}



	if (LCHECK ("s_sman") && !valid_sman)
	{

		if (dflt_used)
		{
			sprintf(local_rec.s_sman,"%-2.2s","  ");
			sprintf(local_rec.s_name,"%-40.40s","Start Salesman");

			DSP_FLD("s_sman");
			DSP_FLD("s_name");
			return(0);
		}

		if (prog_status != ENTRY && strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
			print_mess( ML(mlStdMess018) );
			sleep(2);
			return(1);
		}

		if (SRCH_KEY)
		{
			sman_srch(temp_str);
			return(0);
		}
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.s_sman);
		cc = find_rec(exsf,&exsf_rec,EQUAL,"r");	
		if (cc)
		{
			/*--------------------
			| Salesman Not found. |
			--------------------*/
			print_mess( ML(mlStdMess135) );
			sleep(2);
			return(1);
		}
		sprintf(local_rec.s_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.s_name,exsf_rec.sf_salesman);

		DSP_FLD("s_sman");
		DSP_FLD("s_name");
		return(0);
	}

	if (LCHECK ("e_sman") && !valid_sman)
	{
		if (dflt_used)
		{
			sprintf(local_rec.e_sman,"%-2.2s","~~");
			sprintf(local_rec.e_name,"%-40.40s","End   Salesman");
			DSP_FLD("e_sman");
			DSP_FLD("e_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			sman_srch(temp_str);
			return(0);
		}

		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.e_sman);
		cc = find_rec(exsf,&exsf_rec,EQUAL,"r");	
		if (cc)
		{
			print_mess( ML(mlStdMess135) );
			sleep(2);
			return(1);
		}

		if (strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
			/*------------------------------------------
			| End Salesman may not be less than start. |
			------------------------------------------*/
			errmess( ML(mlStdMess018) );
			sleep(2);
			return(1);
		}
		sprintf(local_rec.e_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.e_name,exsf_rec.sf_salesman);
		DSP_FLD("e_sman");
		DSP_FLD("e_name");
		return(0);
	}
	return(0);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess ( ML(mlStdMess005) );
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount[2] == 0)
	{
		/*-----------------------------------------
		| Cannot Delete Line - No Lines to Delete |
		-----------------------------------------*/
		print_mess ( ML(mlStdMess032) );
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*-------------------
	| Deleting Line ... |
	-------------------*/
	print_at (2,0, "%s", ML(mlUtilsMess084) );
	fflush (stdout);

	lcount [2]--;

	report_width -= (local_rec.std_width + 1);
	for (i = line_cnt; line_cnt < lcount[2]; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	print_cols ();

	line_cnt = i;
	getval (line_cnt);

	move (0, 2);
	cl_line ();
	fflush (stdout);
	return (EXIT_SUCCESS);
}


int
InsertLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess( ML(mlStdMess005) );
		sleep(2);
		return(1);
	}

	/*--------------------
	| Inserting Line ... |
	--------------------*/
	print_at(2,0, "%s", ML(mlUtilsMess085) );
	fflush(stdout);

	report_width += (local_rec.std_width + 1);
	for (i = line_cnt,line_cnt = lcount[2];line_cnt > i;line_cnt--)
	{
		getval(line_cnt - 1);
		putval(line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display();
	}
	lcount[2]++;
	line_cnt = i;

	if (this_page == line_cnt / TABLINES)
		blank_display();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry(cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;

	print_cols();

	line_cnt = i;
	getval(line_cnt);

	move(0,2);
	cl_line();
	fflush(stdout);
	return(0);
}

void
print_cols (void)
{
	int	i;
	int	x = 0;
	int	y = 3;
	int	len;
	int	width = 1;
	int	ok = TRUE;
	int	high_val = (prog_status == ENTRY) ? line_cnt + add_line: lcount[2];

	/*"Report Width : %3d"*/
	print_at ((y - 1), x,  ML(mlUtilsMess109), report_width);
	for (i = y;i < y + 2;i++)
	{
		move(x,i);
		cl_line();
	}

	if (high_val <= 0)
		return;

	print_at(y,x++,"|");

	for (i = 0;i < high_val;i++)
	{
		getval(i);

		len = min(79 - x,local_rec.std_width);

		if (PRINT_WIDTH - width < local_rec.std_width)
		{
			len = PRINT_WIDTH - width;
			ok = FALSE;
		}

		sprintf(err_str,"%-*.*s",len,len,local_rec.std_heading);
		rv_pr(err_str,x,y,1);

		if (!ok)
		{
			printf("|");
			break;
		}

		/*---------------------------------------
		| Break this description across line	|
		---------------------------------------*/
		if (len < local_rec.std_width)
		{
			y++;
			x = 0;
			sprintf(err_str,"%-*.*s",local_rec.std_width - len,local_rec.std_width - len,local_rec.std_heading + len);
			rv_pr(err_str,x,y,1);

			width += len;
			len = local_rec.std_width - len;
		}
		printf("|");
		width += len;
		width++;
		x += len;
		x++;
	}
}

void
down_shift (
 char *str)
{
	char	*sptr = str;

	while (*sptr)
	{
		*sptr = tolower(*sptr);
		sptr++;
	}
}

void
srch_field (
 char *key_val)
{
	char	FldDspStr[21];
	int	i;

	work_open();
	
	save_rec("#Description","#Database Field");

	if (CUSTOMER)
	{
		for (i = 0;i < CUMR_NO_FIELDS;i++)
		{
			if (!strncmp(cumr_fields[i]._descr, key_val,strlen(key_val)))
			{
				sprintf (FldDspStr, "%-20.20s", cumr_fields[i]._field);
				cc = save_rec(cumr_fields[i]._descr, FldDspStr);
				if (cc)
					break;
			}
		}
	}
	if (SUPPLIER)
	{
		for (i = 0;i < SUMR_NO_FIELDS;i++)
		{
			if (!strncmp(sumr_fields[i]._descr, key_val,strlen(key_val)))
			{
				sprintf (FldDspStr, "%-20.20s", sumr_fields[i]._field);
				cc = save_rec(sumr_fields[i]._descr, FldDspStr);
				if (cc)
					break;
			}
		}
	}
	if (ITEM)
	{
		for (i = 0;i < INMR_NO_FIELDS;i++)
		{
			if (!strncmp(inmr_fields[i]._descr, key_val,strlen(key_val)))
			{
				sprintf (FldDspStr, "%-20.20s", inmr_fields[i]._field);
				cc = save_rec(inmr_fields[i]._descr, FldDspStr);
				if (cc)
					break;
			}
		}
	}
	cc = disp_srch();
	work_close();
}


void
Process (void)
{
	int		min_width = 1;
	char	heading[133];
	char	date_str[133];
	time_t	tloc	=	time (NULL);

	report_width = 1;
	switch (rep_type[0])
	{
	case	'C':
		dsp_screen ("Printing Customer Report",
					 comm_rec.tco_no,
					 comm_rec.tco_name);
		break;

	case	'I':
		dsp_screen ("Printing Inventory Report",
					 comm_rec.tco_no,
					 comm_rec.tco_name);
		break;

	case	'S':
		dsp_screen ("Printing Supplier Report",
					 comm_rec.tco_no,
					 comm_rec.tco_name);
		break;

	default:
		return;
	}
		
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
	{
		getval (line_cnt);

		report_width += local_rec.std_width;
		report_width++;
	}

	if (strlen (clip (local_rec.rep_heading)) == 0)
		strcpy (heading, "User's Specified Report");
	else
		strcpy (heading, clip (local_rec.rep_heading));

	min_width = strlen (heading); 
	if (min_width < strlen (clip (comm_rec.tco_name)))
		min_width = strlen (clip (comm_rec.tco_name));

	sprintf (date_str, "As At %-24.24s", ctime (&tloc));
	if (min_width < strlen (clip (date_str)))
		min_width = strlen (clip (date_str));

	min_width *= 2;
	if (report_width < min_width)
		report_width = min_width;


	sprintf (err_str,"%s <%s>", local_rec.systemDate, PNAME);
	fprintf (fout,".START%s\n",clip(err_str));
	fprintf (fout, ".NC%d\n", local_rec.ncopies);
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L%d\n", min (report_width, PRINT_WIDTH));
	fprintf (fout, ".E%s\n", heading);
	fprintf (fout, ".E%s\n", clip (comm_rec.tco_name));
	fprintf (fout, ".EAs At %-24.24s\n", ctime (&tloc));
	if (CUSTOMER)
	{
		fprintf (fout, ".C From Customer %s TO Customer %s   /   From Salesman %s TO Salesman %s\n", 
			local_rec.dbt_no[0],
			local_rec.dbt_no[1],
			local_rec.s_sman,
			local_rec.e_sman);
	}
	if (SUPPLIER)
	{
		fprintf (fout, ".C From Supplier %s TO Supplier %s\n", 
			local_rec.crd_no[0],
			local_rec.crd_no[1]);
	}
	if (ITEM)
	{
		fprintf (fout, ".C From Item %s TO Item %s\n", 
			local_rec.item_no[0],
			local_rec.item_no[1]);
	}

	fprintf (fout, ".R%s\n", rule_off);
	fprintf (fout, "%s\n",   rule_off);

	for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
	{
		getval (line_cnt);

		fprintf (fout, "|%-*.*s",
				 local_rec.std_width,
				 local_rec.std_width,
				 local_rec.std_heading);
	}

	fprintf (fout, "|\n");

	for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
	{
		getval (line_cnt);

		fprintf (fout, "|%-*.*s",
				 local_rec.std_width,
				 local_rec.std_width,
				 single_bar);
	}
	fprintf (fout, "|\n");
	fflush (fout);

	switch  (rep_type[0])
	{
	case	'C':
		ProcCumr ();
		break;

	case	'I':
		ProcInmr ();
		break;

	case	'S':
		ProcSumr ();
		break;
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
ProcCumr (void)
{
	strcpy  (cumr_rec.co_no,  comm_rec.tco_no);
	strcpy  (cumr_rec.est_no, branchNumber);
	sprintf (cumr_rec.dbt_no, "%-6.6s", local_rec.dbt_no[0]);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");


	while (!cc 
	&&     !strcmp (cumr_rec.co_no, comm_rec.tco_no) 
	&&      strcmp (cumr_rec.dbt_no, local_rec.dbt_no[1]) <= 0)
	{
		if (envDbCo != 0 && strcmp (branchNumber, cumr_rec.est_no))
			break;  
	
		if ( strcmp ( cumr_rec.sman_code, local_rec.s_sman) < 0 ||
			 strcmp ( cumr_rec.sman_code, local_rec.e_sman) > 0)
		{
			cc = find_rec ( cumr, &cumr_rec, NEXT, "r" );
			continue;
		}
		dsp_process ("Customer No : ", cumr_rec.dbt_no);
		for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
		{
			getval (line_cnt);
			PrintField (cumr_rec.co_no);
		}
		fprintf (fout, "|\n");
		cc = find_rec ("cumr", &cumr_rec, NEXT, "r");
	}
}

void
ProcInmr (void)
{
	strcpy  (inmr_rec.co_no,   comm_rec.tco_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.item_no[0]);
	cc = find_rec ("inmr", &inmr_rec, GTEQ, "r");

	while (!cc 
	&&     !strcmp (inmr_rec.co_no, comm_rec.tco_no) 
	&&      strcmp (inmr_rec.item_no, local_rec.item_no[1]) <= 0)
	{
		dsp_process ("Item No : ", inmr_rec.item_no);
		for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
		{
			getval (line_cnt);
			PrintField (inmr_rec.co_no);
		}
		fprintf (fout, "|\n");
		cc = find_rec ("inmr", &inmr_rec, NEXT, "r");
	}
}

void
ProcSumr (void)
{
	strcpy  (sumr_rec.co_no,  comm_rec.tco_no);
	strcpy  (sumr_rec.est_no, branchNumber);
	sprintf (sumr_rec.crd_no, "%-6.6s", local_rec.crd_no[0]);
	cc = find_rec ("sumr", &sumr_rec, GTEQ, "r");

	while (!cc 
	&&     !strcmp (sumr_rec.co_no, comm_rec.tco_no) 
	&&      strcmp (sumr_rec.crd_no, local_rec.crd_no[1]) <= 0)
	{
		if (envDbCo != 0 && strcmp (branchNumber, sumr_rec.est_no))
			break;
	
		dsp_process ("Supplier No : ", sumr_rec.crd_no);
		for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
		{
			getval (line_cnt);
			PrintField (sumr_rec.co_no);
		}
		fprintf (fout, "|\n");
		cc = find_rec ("sumr", &sumr_rec, NEXT, "r");
	}
}
void
PrintField (
 char *data_str)
{
	char	numeric_str[81];

	union	_type_var
	{
		char	*cptr;
		int		*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} rec_ptr;

	rec_ptr.cptr = (char *) data_str + VIEW.vwstart;
	switch (VIEW.vwtype)
	{
	    case	CHARTYPE:
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,rec_ptr.cptr);
		break;

	    case	INTTYPE:
		sprintf(numeric_str,"%*d",local_rec.std_width, *(rec_ptr.iptr));
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,(strlen(numeric_str) > local_rec.std_width) ? over_flow : numeric_str);
		break;

		case	SERIALTYPE:
		case	LONGTYPE:
		sprintf(numeric_str,"%*ld",local_rec.std_width,*(rec_ptr.lptr));
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,(strlen(numeric_str) > local_rec.std_width) ? over_flow : numeric_str);
		break;

	    case	DOUBLETYPE:
		sprintf(numeric_str,"%*.2f",local_rec.std_width,*(rec_ptr.dptr));
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,(strlen(numeric_str) > local_rec.std_width) ? over_flow : numeric_str);
		break;

	    case	FLOATTYPE:
		sprintf(numeric_str,"%*.2f",local_rec.std_width,*(rec_ptr.fptr));
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,(strlen(numeric_str) > local_rec.std_width) ? over_flow : numeric_str);
		break;

	    case	DATETYPE:
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width, DateToString(*(rec_ptr.lptr)));
		break;

	    case	MONEYTYPE:
		sprintf(numeric_str,"%*.2f",local_rec.std_width,DOLLARS(*(rec_ptr.dptr)));
		fprintf(fout,"|%-*.*s",local_rec.std_width,local_rec.std_width,(strlen(numeric_str) > local_rec.std_width) ? over_flow : numeric_str);
		break;
	}
}

void
sman_srch (
 char *key_val)
{
	work_open();
	save_rec("#Sm","#Salesman Name");
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",key_val);
	cc = find_rec(exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp(exsf_rec.sf_co_no,comm_rec.tco_no) && 
			      !strncmp(exsf_rec.sf_salesman_no,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_salesman_no,exsf_rec.sf_salesman);
		if (cc)
			break;
		cc = find_rec(exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",temp_str);
	cc = find_rec(exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the   |
| screen overlay in preparation for input.                        |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		switch (rep_type[0])
		{
		case	'C':
			/*" Customer User Report "*/
			rv_pr ( ML(mlUtilsMess087), 28, 0, 1);
			break;

		case	'I':
			/*" Inventory User Report ", */
			rv_pr ( ML(mlUtilsMess088), 28, 0, 1);
			break;

		case	'S':
			/*" Supplier User Report ",*/
			rv_pr ( ML(mlUtilsMess089), 28, 0, 1);
			break;
		}

		move (0, 1);
		line (80);

		switch (scn)
		{
		case	1:
			box  (0, 3, 80, (CUSTOMER) ? 13: 8);
			move (1, 5);
			line (79);
			move (1, 10);
			line (79);
			if (CUSTOMER)
			{
				move (1, 15);
				line (79);
			}
			break;

		case	2:
			print_cols ();
			break;

		default:
			break;
		}

		move (0, 20);
		line (80);
		print_at (21,0, ML(mlStdMess038),comm_rec.tco_no, comm_rec.tco_name);
		if (rep_type[0] != 'I')
		{
			print_at (22,0, ML(mlStdMess039),comm_rec.test_no, comm_rec.test_name);
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*============================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
==============================================================================
| Header file for POS routines, both server and client components            |
|----------------------------------------------------------------------------|
| Date written     :  August 31, 1998    : Author  :  Primo O. Esteria       |
|----------------------------------------------------------------------------|
============================================================================*/
#ifndef	__MESSAGES__
#define	__MESSAGES__

#define	POS_INIT_COMMS		"001"
#define	POS_TRANS_HEADER	"002"
#define	POS_TRANS_LINE		"003"
#define	POS_TRANS_NOTES		"004"
#define	POS_LOST_SALES_LINE	"005"

#define	POS_CUMR		"012"
#define	POS_INMR		"013"
#define	POS_INPR		"014"
#define	POS_INCP		"015"
#define	POS_INDS		"016"
#define	POS_EXCF		"017"
#define	POS_INUM		"018"
#define	POS_INSF		"019"
#define	POS_SOKT		"020"
#define	POS_KITS_DESC		"021"
#define	POS_POCR		"022"					
#define	POS_REAL_INMR		"051"
#define	POS_REAL_CUMR		"052"

#define	POS_SEND_ALL		"088"
#define	POS_GEN_STATUS		"099"

#define	POS_SERV_EXIT		"999"

#define	SERV_INIT_COMMS		"101"
#define	SERV_CUMR		"102"
#define	SERV_INMR		"103"
#define	SERV_INPR		"104"
#define	SERV_INCP		"105"
#define	SERV_INDS		"106"
#define	SERV_EXCF		"107"
#define	SERV_INUM		"108"
#define	SERV_INSF		"109"
#define	SERV_SOKT		"110"
#define	SERV_KITS_DESC		"111"
#define	SERV_POCR		"112"
#define	SERV_CNCH		"113"
#define	SERV_CNCD		"114"
#define	SERV_INBM		"115"
#define	SERV_GEN_STATUS		"199"
#define	SERV_ALA		"200"
#define	NUM_STATUS		100
#define	STAT_NOT_RUNNING	0
#define	STAT_IDLE		1
#define	STAT_INITIALIZING	2
#define	STAT_SEND		3
#define	STAT_RECV		4
#define	STAT_POOLING		5
#define	STAT_CONNECT		6
#define	STAT_WAIT		7
#define	STAT_ERROR		8
#define	STAT_SEND_ERROR		9
#define	STAT_RECV_ERROR		10
#define	STAT_CONNECT_ERROR	11
#define	STAT_HOST_NOT_FOUND	12
#define	STAT_BIND_ERROR		13
#define	STAT_BIND_INUSE		14

#define	MAX_TERMINALS		10
/*
 * Structures
 */

#pragma	pack(2)

struct	msg_header
{
	char	type [8];
	char	len [2];
	char	message [4];
};

struct	msg_init
{
	char	terminal [5];
	char	ipaddress [25];
	char	user [15];
};

struct	msg_real
{
	char	code [16];
	long	hash;
};

/*
 * POS to server structures
 */
struct	TransHeader
{
	char	detNum [15];
	char	hhco_hash [15];
	char	hhcu_hash [15];
	char	inv_no [11];
	char	cus_ord_ref [21];
	char	date_raised [9];
	char	date_required [9];

	char	op_id [15];
	char	time_create [6];
	char	date_create [9];
	char	gross [15];
	char	tax [15];
	char	gst [15];
	char	disc [15];
	char	pri_type [2];
	char	tran_type [15];
	char	sale_code [3];
	char	area_code [3];
};

struct	TransLine
{
	char	hhco_hash [15];
	char	line_no [15];
	char	hhcl_hash [15];
	char	tranType [2];
	char	hhbr_hash [15];
	char	hhum_hash [15];
	char	crd_type [2];
	char	serial_no [26];
	char	qty_org_ord [15];
	char	q_order [15];
	char	gsale_price [15];
	char	sale_price [15];
	char	disc_pc [15];
	char	reg_pc [15];
	char	disc_a [15];
	char	disc_b [15];
	char	disc_c [15];
	char	cumulative [15];
	char	tax_pc [15];
	char	gst_pc [15];
	char	gross [15];
	char	amt_disc [15];
	char	amt_tax [15];
	char	amt_gst [15];
	char	cus_ord_ref [21];
	char	item_desc [41];
	char	status [2];
	char	bonus_flag [2];
	char	stat_flag [2];
};

struct	TransNotes
{
	char	hhco_hash [15];
	char	hhcl_hash [15];
	char	line_no [15];
	char	desc [41];
};

struct	TransLostSales
{
	char	date [15];
	char	hhbr_hash [15];
	char	hhcu_hash [15];
	char	qty [15];
	char	value [15];
	char	res_code [3];
	char	res_desc [61];
	char	status;
};

/*
 * Server to pos data structures
 */
struct	cumr_record
{
	char	dbt_no [7];
	char	hhcu_hash [15];
	char	dbt_name [41];
	char	class_type [4];
	char	price_type [2];
	char	ch_adr1 [41];
	char	ch_adr2 [41];
	char	ch_adr3 [41];
	char	ch_adr4 [41];
	char	contact_name [21];
	char	phone_no [16];
	char	ord_value [15];
	char	bo_current [15];
	char	bo_per1 [15];
	char	bo_per2 [15];
	char	bo_per3 [15];
	char	bo_per4 [15];
	char	bo_fwd [15];
	char	od_flag [15];
};

struct	inmr_record
{
	char	item_no [17];
	char	hhbr_hash [15];
	char	alpha_code [17];
	char	maker_no [17];
	char	alternate [17];
	char	barcode [17];
	char	_class [2];
	char	category [12];
	char	description [41];
	char	quick_code [9];
	char	sellgrp [7];
	char	buygrp [7];
	char	disc_pc [15];
	char	gst_pc [15];
	char	tax_pc [15];
	char	std_uom [15];
	char	alt_uom [15];
	char	uom_cfactor [15];
	char	outer_uom [15];
	char	outer_size [15];
	char	min_sell_price [15];
};


struct	inpr_record
{
	char	priceLevel [2];
	char	hhbr_hash [15];
	char	price_type [15];
	char	curr_code [4];
	char	cust_type [4];
	char	hhgu_hash [15];
	char	price_by [2];
	char	qty_brk1 [15];
	char	qty_brk2 [15];
	char	qty_brk3 [15];
	char	qty_brk4 [15];
	char	qty_brk5 [15];
	char	qty_brk6 [15];
	char	qty_brk7 [15];
	char	qty_brk8 [15];
	char	qty_brk9 [15];
	char	base [15];
	char	price1 [15];
	char	price2 [15];
	char	price3 [15];
	char	price4 [15];
	char	price5 [15];
	char	price6 [15];
	char	price7 [15];
	char	price8 [15];
	char	price9 [15];
};

struct	incp_record
{
	char	priceLevel [2];
	char	hhcu_hash [15];
	char	area_code [3];
	char	cus_type [4];
	char	hhbr_hash [15];
	char	curr_code [4];
	char	status [2];
	char	date_from [15];
	char	date_to [15];
	char	price1 [15];
	char	price2 [15];
	char	price3 [15];
	char	price4 [15];
	char	price5 [15];
	char	price6 [15];
	char	price7 [15];
	char	price8 [15];
	char	price9 [15];
	char	comment [41];
	char	dis_allow [2];
	char	stat_flag [2];
};

struct	inds_record
{
	char	discLevel [2];
	char	hhcu_hash [15];
	char	price_type [15];
	char	category [12];
	char	sel_group [7];
	char	cust_type [4];
	char	hhbr_hash [15];
	char	disc_by [2];
	char	qty_brk1 [15];
	char	qty_brk2 [15];
	char	qty_brk3 [15];
	char	qty_brk4 [15];
	char	qty_brk5 [15];
	char	qty_brk6 [15];
	char	disca_pc1 [15];
	char	disca_pc2 [15];
	char	disca_pc3 [15];
	char	disca_pc4 [15];
	char	disca_pc5 [15];
	char	disca_pc6 [15];
	char	discb_pc1 [15];
	char	discb_pc2 [15];
	char	discb_pc3 [15];
	char	discb_pc4 [15];
	char	discb_pc5 [15];
	char	discb_pc6 [15];
	char	discc_pc1 [15];
	char	discc_pc2 [15];
	char	discc_pc3 [15];
	char	discc_pc4 [15];
	char	discc_pc5 [15];
	char	discc_pc6 [15];
	char	cum_disc [2];
};

struct	excf_record
{
	char	cat_no [12];
	char	cat_desc [41];
	char	max_disc [15];
	char	min_marg [15];
};

struct	inum_record
{
	char	uom_group [21];
	char	hhum_hash [15];
	char	uom [5];
	char	desc [41];
	char	cnv_fct [15];
};

struct	insf_record
{
	char	hhbr_hash [15];
	char	serial_no [26];
};

struct	sokt_record
{
	char	hhbr_hash [15];
	char	line_no [15];
	char	mabr_hash [15];
	char	matl_qty [15];
	char	due_date [15];
	char	bonus [2];
};

/*
 * pocr
 */

struct	cnch_record
{
	char	cont_no [7];
	char	hhch_hash [15];
	char	desc [41];
	char	contact [21];
	char	date_wef [15];
	char	date_rev [15];
	char	date_ren [15];
	char	exch_type [2];
};

struct	cncd_record
{
	char	hhch_hash [15];
	char	line_no [15];
	char	hhbr_hash [15];
	char	hhcc_hash [15];
	char	hhsu_hash [15];
	char	hhcl_hash [15];
	char	price [15];
	char	curr_code [4];
	char	disc_ok [2];
	char	cost [15];
};

struct	inbm_record
{
	char	co_no [3];
	char	barcode [17];
	char	item_no [17];
	char	uom [5];
};

#define	MESSAGE_LEN	sizeof (struct msg_header)

#pragma	pack()

#endif

#ifndef	GLSTRUCTS_H
#define	GLSTRUCTS_H

#define	FORM_LEN	31
#define	LABEL_LEN	17
#define	MAXLEVEL	16	

/*
 * General ledger Budget header. 
 */
typedef	struct 
{
	char	co_no [3];
	int		budg_no;
	char	desc [41];
	int		status;
	Date	mod_date;
	char	stat_flag [2];
}	GLBD_STRUCT;

/*
 * General Ledger batch control file header.
 */
typedef	struct 
{
	char	co_no [3];
	char	br_no [3];
	char	jnl_type [3];
	char	batch_no [11];
	long	hhbh_hash;
	char	user [9];
	Date	glbh_date;
	char	glbh_time [6];
	char	stat_flag [2];
	int		mth;
	int		other_module;
}	GLBH_STRUCT;

/*
 * General Ledger batch control line item file.
 */
typedef	struct 
{
	long	hhbh_hash;
	int		line_no;
	char	acc_no [17];
	char	acronym [10];
	char	name [31];
	char	chq_inv_no [16];
	Money	ci_amt;
	Money	o1_amt;
	Money	o2_amt;
	Money	o3_amt;
	Money	o4_amt;
	long	hhgl_hash;
	char	tran_type [3];
	char	sys_ref [11];
	char	batch_no [11];
	Date	tran_date;
	char	period_no [3];
	char	narrative [21];
	char	alt_desc1 [21];
	char	alt_desc2 [21];
	char	alt_desc3 [21];
	char	user_ref [16];
	Money	fx_amt;
	Money	local_amt;
	char	dc_flag [2];
	char	currency [4];
	double	exch_rate;
	char	stat_flag [2];
}	GLBL_STRUCT;

/*
 * General ledger chart of accounts record. 
 */
typedef	struct 
{
	char	co_no [3];
	int		level_no;
	long	acc_no;
	long	hhca_hash;
	char	acc_desc [26];
	Date	mod_date;
	char	stat_flag [2];
}	GLCA_STRUCT;

/*
 * General ledger control record. 
 */
typedef	struct 
{
	char	format [32];
	int		fiscal;
	int		history;
	int		link_max;
	char	ptype [2];
	int		max_budg;
	int		nxt_budg;
	Date	mod_date;
	char	stat_flag [2];
}	GLCT_STRUCT;

/*
 * General Ledger Journal Control Record. 
 */
typedef	struct 
{
	char	co_no [3];
	char	journ_type [3];
	long	run_no;
	char	jnl_desc [16];
	long	nxt_pge_no;
	Money	tot_1;
	Money	tot_2;
	Money	tot_3;
	Money	tot_4;
	Money	tot_5;
	Money	tot_6;
	char	stat_flag [2];
	char	rep_prog1 [41];
	char	rep_prog2 [41];
}	GLJC_STRUCT;

/*
 * General Ledger Interface Detail File.
 */
typedef	struct 
{
	long	hhih_hash;
	char	br_no [3];
	char	wh_no [3];
	char	acct_no [17];
}	GLID_STRUCT;

/*
 * General Ledger Interface Header File.
 */
typedef	struct 
{
	char	co_no [3];
	long	hhih_hash;
	char	int_code [11];
	char	int_desc [41];
	char	cat_no [12];
	char	class_type [4];
	char	stat_flag [2];
}	GLIH_STRUCT;

/*
 * General Ledger Log file. 
 */
typedef	struct 
{
	char	co_no [3];
	char	est_no [3];
	char	jnl_type [3];
	int		pid;
	char	desc [41];
	Date	mod_date;
	char	stat_flag [2];
}	GLLG_STRUCT;

/*
 * Link between parent/child accounts. 
 */
typedef	struct 
{
	long	parent_hash;
	long	child_hash;
}	GLLN_STRUCT;

/*
 * General Ledger Master File - Account Header 
 */
typedef	struct 
{
	char	co_no [3];
	char	acc_no [17];
	char	curr_code [4];
	char	desc [26];
	char	glmr_class[3] [2];
	long	hhca_hash;
	long	hhmr_hash;
	int		parent_cnt;
	int		child_cnt;
	Date	mod_date;
	char	system_acc [2];
	char	stat_flag [2];
}	GLMR_STRUCT;

/*
 * General ledger narrative.
 */
typedef	struct 
{
	long	gltr_hash;
	int		line_no;
	char	narrative [21];
}	GLNA_STRUCT;

/*
 * General Ledger Period Balance file. 
 */
typedef	struct 
{
	long	hhmr_hash;
	int		prd_no;
	int		year;
	int		budg_no;
	long	mod_time;
	char	user_id [9];
	Money	balance;
	Money	fx_balance;
	long	hhgp_hash;
}	GLPD_STRUCT;


/*
 * General Ledger Transaction Consolidation File. 
 */
typedef	struct 
{
	long	gltr_hash;
	char	tran_type [3];
	char	sys_ref [11];
	char	batch_no [11];
	Date	tran_date;
	Date	post_date;
	char	narrative [21];
	char	user_ref [16];
	Money	amount;
	char	stat_flag [2];
	Money	amt_origin;
	char	currency [4];
	double	exch_rate;
}	GLTC_STRUCT;

/*
 * General Ledger Transaction File. 
 */
typedef	struct 
{
	long	gltr_hash;
	long	hhmr_hash;
	char	tran_type [3];
	char	sys_ref [11];
	char	batch_no [11];
	Date	tran_date;
	Date	post_date;
	char	narrative [21];
	char	user_ref [16];
	Money	amount;
	char	stat_flag [2];
	Money	amt_origin;
	char	currency [4];
	double	exch_rate;
}	GLTR_STRUCT;

/*
 * General ledger user security record.
 */
typedef	struct 
{
	char	co_no [3];
	char	user_name [15];
	char	acc_hdr_code [32];
	int		super_user;
}	GLUS_STRUCT;

/*
 * General Ledger Transactions Work File. 
 */
typedef	struct 
{
	char	acc_no [17];
	char	co_no [3];
	char	est_no [3];
	char	acronym [10];
	char	name [31];
	char	chq_inv_no [16];
	Money	ci_amt;
	Money	o1_amt;
	Money	o2_amt;
	Money	o3_amt;
	Money	o4_amt;
	long	hhgl_hash;
	char	tran_type [3];
	char	sys_ref [11];
	char	batch_no [11];
	Date	tran_date;
	char	period_no [3];
	Date	post_date;
	char	narrative [21];
	char	alt_desc1 [21];
	char	alt_desc2 [21];
	char	alt_desc3 [21];
	char	user_ref [16];
	Money	amount;
	Money	loc_amount;
	char	jnl_type [2];
	char	currency [4];
	double	exch_rate;
	char	stat_flag [2];
	char	run_no [11];
}	GLWK_STRUCT;

/*
 * Currency File Record.
 */
typedef	struct 
{
	char	co_no [3];
	char	code [4];
	char	description [41];
	char	prime_unit [16];
	char	sub_unit [16];
	double	ex1_factor;
	double	ex2_factor;
	double	ex3_factor;
	double	ex4_factor;
	double	ex5_factor;
	double	ex6_factor;
	double	ex7_factor;
	Date	ldate_up;
	char	gl_ctrl_acct [17];
	char	gl_exch_var [17];
	char	stat_flag [2];
	char	pocr_operator [2];
}	POCR_STRUCT;

#endif	/* GLSTRUCTS_H */

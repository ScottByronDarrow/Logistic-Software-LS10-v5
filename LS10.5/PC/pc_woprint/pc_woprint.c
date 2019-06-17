/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_woprint.c   )                                 |
|  Program Desc  : ( Works Order Print.                           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmin, comm, inei, inmr, inum, pcbp, pcid,     ,   |
|                :  pcln, pcms, pcoi, pcrq, pcwc, pcwo, rgrs,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (06/03/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
|  Date Modified : (17/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (04/05/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/07/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (04/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (16/09/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (18/01/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (18/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (31/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (10/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (16/09/97)      | Modified  by : Rowena S Maandig  |
|                                                                     |
|  Comments      : (17/03/92) - Headings now include WO no on each    |
|                : page (correctly)                                   |
|  (04/05/92)    : Changes to allow for ovhd_var and ovhd_fix on pcln |
|                : Also, re-format the output.                        |
|  (30/07/92)    : Incorporate wastage %age into the schema of things |
|                : S/C KIL 7481.                                      |
|  (04/08/92)    : Changes for BOM Instructions. S/C KIL 7480.        |
|  (16/09/92)    : Add in pcrq processing. (and therefore, the ACTUAL)|
|                : Fixed totals @ TOF. S/C DPL 7776.                  |
|  (18/01/94)    : DPL 9673 - can enter w/o no or batch no, to bring  |
|                : up order details.                                  |
|  (18/03/94)    : DPL 10482 - implementing system generated works    |
|                : order numbering.                                   |
|  (31/03/94)    : HGP 10469. Removal of $ signs. Fix core dump       |
|                : introduced in last version.                        |
|  (10/10/94)    : PSL 11299 - mfg cutover - print mfg, req, rec      |
|                : branch and warehouse.                              |
|  (16/09/97)    : To incorporate multilingual conversion.            |
|                                                                     |
| $Log: pc_woprint.c,v $
| Revision 5.3  2002/07/17 09:57:31  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:16  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:25  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.13  2000/03/07 07:22:23  ramon
| For GVision compatibility, I changed the start and end strength fields from NA to ND to correct the overlap problem.
|
| Revision 1.12  1999/11/12 10:37:50  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.11  1999/09/29 10:11:44  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 08:26:29  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.9  1999/09/13 07:03:23  marlene
| *** empty log message ***
|
| Revision 1.8  1999/09/09 06:12:40  marlene
| *** empty log message ***
|
| Revision 1.7  1999/06/17 07:40:53  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_woprint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_woprint/pc_woprint.c,v 5.3 2002/07/17 09:57:31 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>

	/*===============================
	| BoM Material INstruction File |
	===============================*/
	struct dbview bmin_list [] =
	{
		{"bmin_co_no"}, 
		{"bmin_hhbr_hash"}, 
		{"bmin_alt_no"}, 
		{"bmin_line_no"}, 
		{"bmin_tline"}, 
		{"bmin_text"}, 
	};

	int	bmin_no_fields = 6;

	struct
	{
		char	in_co_no [3];
		long	in_hhbr_hash;
		int	in_alt_no;
		int	in_line_no;
		int	in_tline;
		char	in_text [41];
	} bmin_rec;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] =
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
	};

	int	comm_no_fields = 10;

	struct
	{
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tco_short [16];
		char	test_no [3];
		char	test_name [41];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_name [41];
		char	tcc_short [10];
	} comm_rec;

	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list [] =
	{
		{"inei_hhbr_hash"}, 
		{"inei_est_no"}, 
		{"inei_std_batch"}, 
		{"inei_min_batch"}, 
		{"inei_max_batch"}, 
		{"inei_hzrd_class"}, 
	};

	int	inei_no_fields = 6;

	struct
	{
		long	ei_hhbr_hash;
		char	ei_est_no [3];
		float	ei_std_batch;
		float	ei_min_batch;
		float	ei_max_batch;
		char	ei_hzrd_class [5];
	} inei_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"}, 
		{"inmr_hhbr_hash"}, 
		{"inmr_description"}, 
		{"inmr_description2"}, 
		{"inmr_std_uom"}, 
		{"inmr_alt_uom"}, 
	};

	int	inmr_no_fields = 6;

	struct
	{
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		char	mr_description [41];
		char	mr_description2 [41];
		long	mr_std_uom;
		long	mr_alt_uom;
	} inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list [] =
	{
		{"inum_hhum_hash"}, 
		{"inum_uom"}, 
	};

	int	inum_no_fields = 2;

	struct
	{
		long	um_hhum_hash;
		char	um_uom [5];
	} inum_rec;

	/*=========================
	| Routing By-Product File |
	=========================*/
	struct dbview pcbp_list [] =
	{
		{"pcbp_hhgr_hash"}, 
		{"pcbp_seq_no"}, 
		{"pcbp_hhbr_hash"}, 
		{"pcbp_qty"}, 
		{"pcbp_hhwo_hash"}, 
	};

	int	pcbp_no_fields = 5;

	struct
	{
		long	bp_hhgr_hash;
		int	bp_seq_no;
		long	bp_hhbr_hash;
		float	bp_qty;
		long	bp_hhwo_hash;
	} pcbp_rec;

	/*=========================================
	| Process Control Instruction Detail File |
	=========================================*/
	struct dbview pcid_list [] =
	{
		{"pcid_co_no"}, 
		{"pcid_hhbr_hash"}, 
		{"pcid_hhwc_hash"}, 
		{"pcid_instr_no"}, 
		{"pcid_version"}, 
		{"pcid_line_no"}, 
		{"pcid_text"}, 
	};

	int	pcid_no_fields = 7;

	struct
	{
		char	id_co_no [3];
		long	id_hhbr_hash;
		long	id_hhwc_hash;
		int	id_instr_no;
		int	id_version;
		int	id_line_no;
		char	id_text [61];
	} pcid_rec;

	/*==========================
	| Routing Line detail File |
	==========================*/
	struct dbview pcln_list [] =
	{
		{"pcln_hhgr_hash"}, 
		{"pcln_seq_no"}, 
		{"pcln_hhwc_hash"}, 
		{"pcln_hhrs_hash"}, 
		{"pcln_rate"}, 
		{"pcln_ovhd_var"}, 
		{"pcln_ovhd_fix"}, 
		{"pcln_setup"}, 
		{"pcln_run"}, 
		{"pcln_clean"}, 
		{"pcln_qty_rsrc"}, 
		{"pcln_instr_no"}, 
		{"pcln_yld_clc"}, 
		{"pcln_can_split"}, 
		{"pcln_line_no"}, 
		{"pcln_hhwo_hash"}, 
	};

	int	pcln_no_fields = 16;

	struct
	{
		long	ln_hhgr_hash;
		int	ln_seq_no;
		long	ln_hhwc_hash;
		long	ln_hhrs_hash;
		double	ln_rate;
		double	ln_ovhd_var;
		double	ln_ovhd_fix;
		long	ln_setup;
		long	ln_run;
		long	ln_clean;
		int	ln_qty_rsrc;
		int	ln_instr_no;
		char	ln_yld_clc [5];
		char	ln_can_split [2];
		int	ln_line_no;
		long	ln_hhwo_hash;
	} pcln_rec;

	/*=================================
	| BoM Material Specification File |
	=================================*/
	struct dbview pcms_list [] =
	{
		{"pcms_co_no"}, 
		{"pcms_hhbr_hash"}, 
		{"pcms_alt_no"}, 
		{"pcms_line_no"}, 
		{"pcms_cons"}, 
		{"pcms_mabr_hash"}, 
		{"pcms_uom"}, 
		{"pcms_matl_qty"}, 
		{"pcms_matl_wst_pc"}, 
		{"pcms_instr_no"}, 
		{"pcms_iss_seq"}, 
		{"pcms_uniq_id"}, 
		{"pcms_amt_issued"}, 
		{"pcms_amt_recptd"}, 
		{"pcms_qty_issued"}, 
		{"pcms_act_qty_in"}, 
		{"pcms_hhwo_hash"}, 
	};

	int	pcms_no_fields = 17;

	struct
	{
		char	ms_co_no [3];
		long	ms_hhbr_hash;
		int	ms_alt_no;
		int	ms_line_no;
		char	ms_cons [2];
		long	ms_mabr_hash;
		long	ms_uom;
		float	ms_matl_qty;
		float	ms_matl_wst_pc;
		int	ms_instr_no;
		int	ms_iss_seq;
		int	ms_uniq_id;
		double	ms_amt_issued;	/* money */
		double	ms_amt_recptd;	/* money */
		float	ms_qty_issued;
		char	ms_act_qty_in [2];
		long	ms_hhwo_hash;
	} pcms_rec;

	/*================
	| Order (Special |
	================*/
	struct dbview pcoi_list [] =
	{
		{"pcoi_hhwo_hash"}, 
		{"pcoi_line_no"}, 
		{"pcoi_text"}, 
	};

	int	pcoi_no_fields = 3;

	struct
	{
		long	oi_hhwo_hash;
		int	oi_line_no;
		char	oi_text [61];
	} pcoi_rec;

	/*===================================
	| Production Control Resource Queue |
	===================================*/
	struct dbview pcrq_list [] =
	{
		{"pcrq_hhrs_hash"}, 
		{"pcrq_qty_rsrc"}, 
		{"pcrq_hhwo_hash"}, 
		{"pcrq_prod_class"}, 
		{"pcrq_priority"}, 
		{"pcrq_seq_no"}, 
		{"pcrq_line_no"}, 
		{"pcrq_last_date"}, 
		{"pcrq_last_time"}, 
		{"pcrq_est_date"}, 
		{"pcrq_est_time"}, 
		{"pcrq_est_setup"}, 
		{"pcrq_est_run"}, 
		{"pcrq_est_clean"}, 
		{"pcrq_act_date"}, 
		{"pcrq_act_time"}, 
		{"pcrq_act_setup"}, 
		{"pcrq_act_run"}, 
		{"pcrq_act_clean"}, 
		{"pcrq_can_split"}, 
		{"pcrq_firm_sched"}, 
		{"pcrq_stat_flag"}, 
	};

	int	pcrq_no_fields = 22;

	struct
	{
		long	rq_hhrs_hash;
		int	rq_qty_rsrc;
		long	rq_hhwo_hash;
		char	rq_prod_class [5];
		int	rq_priority;
		int	rq_seq_no;
		int	rq_line_no;
		long	rq_last_date;
		long	rq_last_time;
		long	rq_est_date;
		long	rq_est_time;
		long	rq_est_setup;
		long	rq_est_run;
		long	rq_est_clean;
		long	rq_act_date;
		long	rq_act_time;
		long	rq_act_setup;
		long	rq_act_run;
		long	rq_act_clean;
		char	rq_can_split [2];
		char	rq_firm_sched [2];
		char	rq_stat_flag [2];
	} pcrq_rec;

	/*=======================
	| Work Centre Code file |
	=======================*/
	struct dbview pcwc_list [] =
	{
		{"pcwc_hhwc_hash"}, 
		{"pcwc_co_no"}, 
		{"pcwc_br_no"}, 
		{"pcwc_work_cntr"}, 
		{"pcwc_name"}, 
	};

	int	pcwc_no_fields = 5;

	struct
	{
		long	wc_hhwc_hash;
		char	wc_co_no [3];
		char	wc_br_no [3];
		char	wc_work_cntr [9];
		char	wc_name [41];
	} pcwc_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"}, 
		{"pcwo_br_no"}, 
		{"pcwo_wh_no"}, 
		{"pcwo_req_br_no"}, 
		{"pcwo_req_wh_no"}, 
		{"pcwo_rec_br_no"}, 
		{"pcwo_rec_wh_no"}, 
		{"pcwo_order_no"}, 
		{"pcwo_hhwo_hash"}, 
		{"pcwo_reqd_date"}, 
		{"pcwo_rtg_seq"}, 
		{"pcwo_priority"}, 
		{"pcwo_create_date"}, 
		{"pcwo_hhbr_hash"}, 
		{"pcwo_bom_alt"}, 
		{"pcwo_rtg_alt"}, 
		{"pcwo_prod_qty"}, 
		{"pcwo_order_status"}, 
		{"pcwo_batch_no"}, 
	};

	int	pcwo_no_fields = 19;

	struct
	{
		char	wo_co_no [3];
		char	wo_br_no [3];
		char	wo_wh_no [3];
		char	wo_req_br_no [3];
		char	wo_req_wh_no [3];
		char	wo_rec_br_no [3];
		char	wo_rec_wh_no [3];
		char	wo_order_no [8];
		long	wo_hhwo_hash;
		long	wo_reqd_date;
		int		wo_rtg_seq;
		int		wo_priority;
		long	wo_create_date;
		long	wo_hhbr_hash;
		int		wo_bom_alt;
		int		wo_rtg_alt;
		float	wo_prod_qty;
		char	wo_order_status [2];
		char	wo_batch_no [11];
	} pcwo_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list [] =
	{
		{"rgrs_hhrs_hash"}, 
		{"rgrs_code"}, 
		{"rgrs_desc"}, 
		{"rgrs_type"}, 
	};

	int	rgrs_no_fields = 4;

	struct
	{
		long	rs_hhrs_hash;
		char	rs_code [9];
		char	rs_desc [41];
		char	rs_type [2];
	} rgrs_rec;

	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	3

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
	}	esmr_rec;

	/*===========================================+
	 | Cost Centre/Warehouse Master File Record. |
	 +===========================================*/
#define	CCMR_NO_FIELDS	4

	struct dbview	ccmr_list [CCMR_NO_FIELDS] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_name"},
	};

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		char	name [41];
	}	ccmr_rec;

	char	*bmin	= "bmin", 
			*comm	= "comm", 
			*data	= "data", 
			*inei	= "inei", 
			*inmr	= "inmr", 
			*inum	= "inum", 
			*pcbp	= "pcbp", 
			*pcid	= "pcid", 
			*pcln	= "pcln", 
			*pcms	= "pcms", 
			*pcoi	= "pcoi", 
			*pcrq	= "pcrq", 
			*pcwc	= "pcwc", 
			*pcwo	= "pcwo", 
			*pcwo2	= "pcwo2",
			*rgrs	= "rgrs",
			*esmr	= "esmr",
			*ccmr	= "ccmr";

	FILE	*fout;

	int		pformat_open = FALSE;

	int		PC_GEN;

struct
{
	int	seq_no;
	long	hhwc_hash;
	double	rate;
	double	ovhd_var;
	double	ovhd_fix;
	long	time;
	int	qty_rsrc;
	char	type [2];
	int	instr_no;
} store [MAXLINES];
int	no_stored;

struct
{
	char	status;
	char	desc [13];
} wo_stat [] = {
	{'P', "Planned"}, 
	{'F', "Firm Planned"}, 
	{'I', "Issuing"}, 
	{'A', "Allocated"}, 
	{'R', "Released"}, 
	{'C', "Closing"}, 
	{'Z', "Closed"}, 
	{'D', "Deleted"}, 
	{0, ""}, 
};
int	no_stat = 9;

struct
{
	char	type;
	char	desc [9];
} res_type [] = {
	{'L', "Labour"}, 
	{'M', "Machine"}, 
	{'Q', "QC Check"}, 
	{'O', "Other"}, 
	{'S', "Special"}, 
	{0, ""}, 
};
int	no_res = 5;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	systemDate [11];
	long	lsystemDate;

	char	st_ord_no [8];
	char	st_batch_no [11];
	char	st_item [17];
	char	st_desc [41];
	char	st_strgth [6];
	char	end_ord_no [8];
	char	end_batch_no [11];
	char	end_item [17];
	char	end_desc [41];
	char	end_strgth [6];
	int		lpno;
	char	back [4];
	char	onight [4];

	char	item [17];
	char	item_desc [41];
	char	wo_status [19];
	char	res_desc [9];
	char	std_uom [5];
	char	alt_uom [5];
	char	mtl_uom [5];
	float	std_batch;
	float	min_batch;
	float	max_batch;
	double	tot_cost;
	long	tot_time;

	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_ord_no", 	 4, 14, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", " ", "Start Order :", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.st_ord_no}, 
	{1, LIN, "st_batch_no", 	 4, 38, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Start Batch :", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.st_batch_no}, 
	{1, LIN, "st_item", 	 4, 70, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number:", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.st_item}, 
	{1, LIN, "st_desc", 	 4, 90, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.st_desc}, 
	{1, LIN, "st_strgth", 	 4, 123, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Strength:", " ", 
		ND, NO,  JUSTLEFT, "", "", local_rec.st_strgth}, 

	{1, LIN, "end_ord_no", 	 5, 14, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", " ", "End Order   :", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.end_ord_no}, 
	{1, LIN, "end_batch_no",  5, 38, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "End Batch   :", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.end_batch_no}, 
	{1, LIN, "end_item", 	 5, 70, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number:", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.end_item}, 
	{1, LIN, "end_desc", 	 5, 90, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.end_desc}, 
	{1, LIN, "end_strgth", 	 5, 123, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Strength:", " ", 
		ND, NO,  JUSTLEFT, "", "", local_rec.end_strgth}, 

	{1, LIN, "lpno", 	 7, 14, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No : ", " ", 
		YES, NO,  JUSTLEFT, "", "",  (char *)&local_rec.lpno}, 
	{1, LIN, "back", 	 8, 14, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Background : ", " ", 
		YES, NO,  JUSTLEFT, "NY", "", local_rec.back}, 
	{1, LIN, "onight", 9, 14, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight  : ", " ", 
		YES, NO,  JUSTLEFT, "NY", "", local_rec.onight}, 

	{0, LIN, "", 		 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <srch_pcwo2.h>
/*==========================
| function prototypes |
=======================*/

void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void run_prog (char *prog_name, char *prog_desc);
int heading (int scn);
int spec_valid (int field);
void process (void);
void head_output (void);
void print_wo (void);
void prt_spec_inst (void);
void print_bom (void);
void proc_pcms (void);
void proc_bmin (void);
void print_rtg (void);
void proc_lines (void);
void calc_vals (void);
int proc_byprd (void);
void proc_instr (int brk_rqd);
int get_inst_ver (int store_line);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv[])
{
	char 	*sptr;

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		PC_GEN = TRUE;

	SETUP_SCR (vars);

	if (argc != 2 && argc != 4)
	{
		print_at (0,0,mlPcMess712, argv [0]);
		print_at (1,0,mlPcMess713, argv [0]);
		exit (argc);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 2)
	{
		/*-------------------------------
		| Setup required parameters	|
		-------------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();
	
		/*=======================================
		| Beginning of input control loop	|
		=======================================*/
		while (prog_exit == 0)
		{
			/*-----------------------
			| Reset control flags	|
			-----------------------*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;
	
			init_vars (1);
			/*-------------------------------
			| Enter screen 1 linear input	|
			-------------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;
	
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
	
			run_prog (argv [0], argv [1]);
		}
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	local_rec.lpno = atoi (argv [1]);
	sprintf (local_rec.st_ord_no, "%-7.7s", argv [2]);
	sprintf (local_rec.end_ord_no, "%-7.7s", argv [3]);

	process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);

	open_rec (bmin,  bmin_list, bmin_no_fields, "bmin_id_no");
	open_rec (inei,  inei_list, inei_no_fields, "inei_id_no");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, inum_no_fields, "inum_hhum_hash");
	open_rec (pcbp,  pcbp_list, pcbp_no_fields, "pcbp_id_no");
	open_rec (pcid,  pcid_list, pcid_no_fields, "pcid_id_no");
	open_rec (pcln,  pcln_list, pcln_no_fields, "pcln_id_no");
	open_rec (pcms,  pcms_list, pcms_no_fields, "pcms_id_no");
	open_rec (pcoi,  pcoi_list, pcoi_no_fields, "pcoi_id_no");
	open_rec (pcrq,  pcrq_list, pcrq_no_fields, "pcrq_id_no2");
	open_rec (pcwc,  pcwc_list, pcwc_no_fields, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo2,  pcwo_list, pcwo_no_fields, "pcwo_id_no3");
	open_rec (rgrs,  rgrs_list, rgrs_no_fields, "rgrs_hhrs_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (bmin);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcbp);
	abc_fclose (pcid);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcoi);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgrs);
	abc_fclose (esmr);
	abc_fclose (ccmr);

	abc_dbclose (data);
}

void
run_prog (
 char *prog_name, 
 char *prog_desc)
{
	char	lp_str [3];

	sprintf (lp_str, "%2d", local_rec.lpno);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT", 
				"ONIGHT", 
				prog_name, 
				lp_str, 
				local_rec.st_ord_no, 
				local_rec.end_ord_no, 
				prog_desc,  (char *)0);
		}
		else
			return;
	}

	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name, 
				prog_name, 
				lp_str, 
				local_rec.st_ord_no, 
				local_rec.end_ord_no,  (char *)0);
		else
			return;
	}
	else 
	{
		execlp (prog_name, 
			prog_name, 
			lp_str, 
			local_rec.st_ord_no, 
			local_rec.end_ord_no,  (char *)0);
	}
}

/*=======================
| Display heading scrn	|
=======================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		swide ();
		clear ();
		rv_pr (ML(mlPcMess086), 53, 0, 1);

		box (0, 3, 132, 6);
		move (0, 1);
		line (132);

		move (1, 6);
		line (130);

		move (0, 19);
		line (132);
		/*print_at (22,
			0,
			" Co. : %s %s Br. : %s %s Wh. : %s %s",
			comm_rec.tco_no, comm_rec.tco_short,
			comm_rec.test_no, comm_rec.test_short,
			comm_rec.tcc_no, comm_rec.tcc_short);*/
		print_at (20, 0, ML(mlStdMess038),
			comm_rec.tco_no, comm_rec.tco_short);
		print_at (21, 0, ML(mlStdMess039),
			comm_rec.test_no, comm_rec.test_short);
		print_at (22, 0, ML(mlStdMess099),
			comm_rec.tcc_no, comm_rec.tcc_short);

		scn_set (scn);

		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;

		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*===============================
| Validate entered field (s)	|
===============================*/
int
spec_valid (
 int field)
{
	if (LCHECK ("st_ord_no") || LCHECK ("st_batch_no"))
	{
		if (LCHECK ("st_batch_no"))
			if (F_NOKEY (label ("st_batch_no")))
				return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			if (LCHECK ("st_ord_no"))
				srch_order (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
			else
			{
				srch_batch (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
				abc_selfield (pcwo, "pcwo_id_no");
			}
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.st_ord_no, "       ");
			sprintf (local_rec.st_batch_no, "%-10.10s", " ");
			sprintf (local_rec.st_item, "%-16.16s", " ");
			sprintf (local_rec.st_desc, "%-35.35s", "First Works Order");
			sprintf (local_rec.st_strgth, "%-5.5s", " ");
			DSP_FLD ("st_ord_no");
			DSP_FLD ("st_batch_no");
			DSP_FLD ("st_item");
			DSP_FLD ("st_desc");
			DSP_FLD ("st_strgth");
			if (LCHECK ("st_ord_no"))
				FLD ("st_batch_no") = NO;
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("st_ord_no"))
			FLD ("st_batch_no") = NA;

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("st_ord_no"))
		{
			if (PC_GEN)
				sprintf (pcwo_rec.wo_order_no,
						"%-7.7s",
						zero_pad (local_rec.st_ord_no, 7));
			else
				sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.st_ord_no);
			cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		}
		else
		{
			sprintf (pcwo_rec.wo_batch_no, "%-10.10s", local_rec.st_batch_no);
			cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		}
		if (cc)
		{	
			if (LCHECK ("st_ord_no"))
			/*	sprintf (err_str, 
					" Works Order Not On File - Order No [%s] ", 
					pcwo_rec.wo_order_no);
			else
				sprintf (err_str, 
					" Works Order Not On File - Batch No [%s] ", 
					pcwo_rec.wo_batch_no);*/
				print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.st_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		DSP_FLD ("st_ord_no");

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_ord_no, local_rec.end_ord_no) > 0)
		{
			sprintf (err_str, 
				"Start Works Order %s Greater Than End Works Order %s", 
				local_rec.st_ord_no, 
				local_rec.end_ord_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_ord_no, "%-7.7s", " ");
			DSP_FLD ("st_ord_no");
			return (EXIT_FAILURE);
		}

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			/*print_mess ("\007 Works Order References An Unknown Item ");*/
			print_mess (ML(mlPcMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.st_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		sprintf (local_rec.st_batch_no, "%-10.10s", pcwo_rec.wo_batch_no);
		sprintf (local_rec.st_item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (local_rec.st_desc, "%-35.35s", inmr_rec.mr_description);
		sprintf (local_rec.st_strgth, "%-5.5s", &inmr_rec.mr_description [35]);
		DSP_FLD ("st_ord_no");
		DSP_FLD ("st_batch_no");
		DSP_FLD ("st_item");
		DSP_FLD ("st_desc");
		DSP_FLD ("st_strgth");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_ord_no") || LCHECK ("end_batch_no"))
	{
		if (LCHECK ("end_batch_no"))
			if (F_NOKEY (label ("end_batch_no")))
				return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			if (LCHECK ("end_ord_no"))
				srch_order (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
			else
			{
				srch_batch (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
				abc_selfield (pcwo, "pcwo_id_no");
			}
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.end_ord_no, "~~~~~~~");
			sprintf (local_rec.end_batch_no, "%-10.10s", " ");
			sprintf (local_rec.end_item, "%-16.16s", " ");
			sprintf (local_rec.end_desc, "%-35.35s", "Last Works Order");
			sprintf (local_rec.end_strgth, "%-5.5s", " ");
			DSP_FLD ("end_ord_no");
			DSP_FLD ("end_batch_no");
			DSP_FLD ("end_item");
			DSP_FLD ("end_desc");
			DSP_FLD ("end_strgth");
			if (LCHECK ("end_ord_no"))
				FLD ("end_batch_no") = NO;
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("end_ord_no"))
			FLD ("end_batch_no") = NA;

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("end_ord_no"))
		{
			if (PC_GEN)
				sprintf (pcwo_rec.wo_order_no,
						"%-7.7s",
						zero_pad (local_rec.end_ord_no, 7));
			else
				sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.end_ord_no);
			cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		}
		else
		{
			sprintf (pcwo_rec.wo_batch_no, "%-10.10s", local_rec.end_batch_no);
			cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		}
		if (cc)
		{	
			if (LCHECK ("end_ord_no"))
			/*	sprintf (err_str, 
					" Works Order Not On File - Order No [%s] ", 
					pcwo_rec.wo_order_no);
			else
				sprintf (err_str, 
					" Works Order Not On File - Batch No [%s] ", 
					pcwo_rec.wo_batch_no);*/
				print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.end_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		DSP_FLD ("end_ord_no");

		if (strcmp (local_rec.st_ord_no, local_rec.end_ord_no) > 0)
		{
			/*sprintf (err_str, 
				"Start Works Order %s Greater Than End Works Order %s", 
				local_rec.st_ord_no, 
				local_rec.end_ord_no);*/
			print_mess (ML(mlPcMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_ord_no, "%-7.7s", " ");
			DSP_FLD ("end_ord_no");
			return (EXIT_FAILURE);
		}

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			/*print_mess ("\007 Works Order References An Unknown Item ");*/
			print_mess (ML(mlPcMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.end_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		sprintf (local_rec.end_batch_no, "%-10.10s", pcwo_rec.wo_batch_no);
		sprintf (local_rec.end_item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (local_rec.end_desc, "%-35.35s", inmr_rec.mr_description);
		sprintf (local_rec.end_strgth, "%-5.5s", &inmr_rec.mr_description [35]);
		DSP_FLD ("end_ord_no");
		DSP_FLD ("end_batch_no");
		DSP_FLD ("end_item");
		DSP_FLD ("end_desc");
		DSP_FLD ("end_strgth");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}
	
		if (!valid_lp (local_rec.lpno))
		{
			/*print_mess ("\007 Invalid Printer NUmber ");*/
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.back, "Yes");
		else
			strcpy (local_rec.back, "No ");

		DSP_FLD ("back");
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onight, "Yes");
		else
			strcpy (local_rec.onight, "No ");

		DSP_FLD ("onight");
	
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Print W/O, BOM and Routing for all  |
| works orders in the specified range |
-------------------------------------*/
void
process (
 void)
{
	int	first_time;

	first_time = TRUE;
	dsp_screen ("Printing Works Order", comm_rec.tco_no, comm_rec.tco_name);

	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
	strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
	sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.st_ord_no);
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (pcwo_rec.wo_co_no,   comm_rec.tco_no) &&
	       !strcmp (pcwo_rec.wo_br_no,   comm_rec.test_no) &&
		   !strcmp (pcwo_rec.wo_wh_no, comm_rec.tcc_no) &&
	       strcmp (pcwo_rec.wo_order_no, local_rec.end_ord_no) <= 0)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", 
			       pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.mr_description);

		inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy (inei_rec.ei_est_no, comm_rec.test_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			inei_rec.ei_std_batch = 0.00;
			inei_rec.ei_min_batch = 0.00;
			inei_rec.ei_max_batch = 0.00;
		}

		local_rec.std_batch = inei_rec.ei_std_batch;
		local_rec.min_batch = inei_rec.ei_min_batch;
		local_rec.max_batch = inei_rec.ei_max_batch;

		if (first_time)
			head_output ();
		else
		{
			fprintf (fout, ".DS3\n");
			fprintf (fout, 
				".EWORKS ORDER : %s\n", 
				clip (pcwo_rec.wo_order_no));
			fprintf (fout,
				".EBATCH NUMBER : %s\n",
				clip (pcwo_rec.wo_batch_no));
			fprintf (fout, ".B1\n");
			fprintf (fout, ".EWORKS ORDER DETAILS\n");
			fprintf (fout, ".PA\n");
		}

		dsp_process ("Works Order", pcwo_rec.wo_order_no);

		/*-------------------
		| Print Works Order |
		-------------------*/
		print_wo ();
		print_bom ();
		print_rtg ();

		first_time = FALSE;
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	if (pformat_open)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
}

/*---------------------
| Prepare Output Pipe |
---------------------*/
void
head_output (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	pformat_open = TRUE;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".8\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".ECOMPANY %s : %s\n", 
		comm_rec.tco_no, clip (comm_rec.tco_name));

	fprintf (fout, ".EBRANCH %s : %s\n", 
		comm_rec.test_no, clip (comm_rec.test_name));

	fprintf (fout, ".EWAREHOUSE %s : %s\n", 
		comm_rec.tcc_no, clip (comm_rec.tcc_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, ".DS3\n");
	fprintf (fout, ".EWORKS ORDER : %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".EBATCH NUMBER : %s\n", clip (pcwo_rec.wo_batch_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWORKS ORDER DETAILS\n");

	fprintf (fout, ".R============================================");
	fprintf (fout, "==============================================");
	fprintf (fout, "=============================================\n");
}

/*-------------------
| Print W/O Details |
-------------------*/
void
print_wo (
 void)
{
	int	i;

	for (i = 0; i < no_stat; i++)
	{
		if (pcwo_rec.wo_order_status [0] == wo_stat [i].status)
		{
			if (pcwo_rec.wo_order_status [0] == 'R')
			{
				sprintf (local_rec.wo_status, 
					"%-12.12s(%3d)", 
					wo_stat [i].desc, 
					pcwo_rec.wo_rtg_seq);
			}
			else
			{
				sprintf (local_rec.wo_status, 
					"%-15.15s", 
					wo_stat [i].desc);
			}
			break;
		}
	}

	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");

	fprintf (fout, "| WORKS ORDER NUMBER:");
	fprintf (fout, " %-7.7s%-11.11s", pcwo_rec.wo_order_no, " ");
	fprintf (fout, "STATUS            :");
	fprintf (fout, " %-17.17s%-2.2s", local_rec.wo_status, " ");
	fprintf (fout, "PRIORITY          :");
	fprintf (fout, " %1d%-56.56s|\n", pcwo_rec.wo_priority, " ");

	fprintf (fout, "| BATCH NUMBER      :");
	fprintf (fout, " %-10.10s%-8.8s", pcwo_rec.wo_batch_no, " ");
	fprintf (fout, "QUANTITY REQUIRED :");
	fprintf (fout, " %14.6f%-5.5s", pcwo_rec.wo_prod_qty, " ");
	fprintf (fout, "REQUIRED DATE     :");
	fprintf (fout, " %-10.10s%-9.9s", DateToString (pcwo_rec.wo_reqd_date), " ");
	fprintf (fout, "DATE RAISED       :");
	fprintf (fout, " %-10.10s%-8.8s|\n",DateToString (pcwo_rec.wo_create_date), " ");

	fprintf (fout, "|%-155.155s|\n", " ");

	fprintf (fout, "| ITEM NUMBER       :");
	fprintf (fout, " %-16.16s%-41.41s", inmr_rec.mr_item_no, " ");
	fprintf (fout, "STRENGTH          :");
	fprintf (fout, " %-5.5s%-52.52s|\n", &inmr_rec.mr_description [35], " ");

	fprintf (fout, "| DESCRIPTION       :");
	fprintf (fout, " %-40.40s%-17.17s", inmr_rec.mr_description, " ");
	fprintf (fout, "BOM ALTERNATE     :");
	fprintf (fout, " %2d%-55.55s|\n", pcwo_rec.wo_bom_alt, " ");

	fprintf (fout, "|                   :");
	fprintf (fout, " %-40.40s%-17.17s", inmr_rec.mr_description2, " ");
	fprintf (fout, "ROUTING ALTERNATE :");
	fprintf (fout, " %2d%-55.55s|\n", pcwo_rec.wo_rtg_alt, " ");

	fprintf (fout, "|%-155.155s|\n", " ");

	/*----------------
	| Lookup std uom |
	----------------*/
	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.mr_std_uom);
	if (cc)
		strcpy (inum_rec.um_uom, "UNKN");
	sprintf (local_rec.std_uom, "%-4.4s", inum_rec.um_uom);
	fprintf (fout, "| STD UOM           :");
	fprintf (fout, " %-4.4s%-53.53s", inum_rec.um_uom, " ");

	/*----------------
	| Lookup alt uom |
	----------------*/
	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.mr_alt_uom);
	if (cc)
		strcpy (inum_rec.um_uom, "UNKN");
	sprintf (local_rec.alt_uom, "%-4.4s", inum_rec.um_uom);
	fprintf (fout, "ALT UOM            : ");
	fprintf (fout, " %-4.4s%-53.53s|\n", inum_rec.um_uom, " ");

	fprintf (fout, "|%-155.155s|\n", " ");

	/* read for requesting branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_req_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Requesting Branch Name Not Found");
	fprintf (fout, "| REQUESTING BRANCH :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_req_br_no,
			esmr_rec.est_name,
			" ");
	/* read for requesting warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Requesting Warehouse Name Not Found");
	fprintf (fout, "REQUESTING W/H    :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_req_wh_no,
			ccmr_rec.name,
			" ");

	/* read for manufacturing branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Manufacturing Branch Name Not Found");
	fprintf (fout, "| MANUFACTURE BRANCH:");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_br_no,
			esmr_rec.est_name,
			" ");
	/* read for manufacturing warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Manufacturing Warehouse Name Not Found");
	fprintf (fout, "MANUFACTURE W/H   :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_wh_no,
			ccmr_rec.name,
			" ");

	/* read for receiving branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_rec_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Receiving Branch Name Not Found");
	fprintf (fout, "| RECEIVING BRANCH  :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_rec_br_no,
			esmr_rec.est_name,
			" ");
	/* read for receiving warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_rec_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_rec_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Receiving Warehouse Name Not Found");
	fprintf (fout, "RECEIVING W/H     :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_rec_wh_no,
			ccmr_rec.name,
			" ");

	fprintf (fout, "|%-155.155s|\n", " ");

	fprintf (fout, "|  STANDARD BATCH : ");
	fprintf (fout, "%14.6f%-30.30s", local_rec.std_batch, " ");
	fprintf (fout, "MINIMUM BATCH : %14.6f%30.30s", local_rec.min_batch, " ");
	fprintf (fout, "MAXIMUM BATCH : %14.6f  |\n", local_rec.max_batch);

	prt_spec_inst ();
}
/*----------------------------
| Print Special Instructions |
----------------------------*/
void
prt_spec_inst (
 void)
{
	int	first_inst;

	first_inst = TRUE;
	pcoi_rec.oi_hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcoi_rec.oi_line_no = 0;
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	while (!cc && pcoi_rec.oi_hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		if (first_inst)
		{
			fprintf (fout, "|---------------------------------------");
			fprintf (fout, "----------------------------------------");
			fprintf (fout, "----------------------------------------");
			fprintf (fout, "------------------------------------|\n");

			fprintf (fout, 
				"|%-65.65s  SPECIAL  INSTRUCTIONS  %-65.65s|\n", 
				" ", " ");

			fprintf (fout, "|%-155.155s|\n", " ");

			first_inst = FALSE;
		}

		fprintf (fout, 
			"|%-45.45s  %-61.61s  %-45.45s|\n", 
			" ", pcoi_rec.oi_text, " ");

		cc = find_rec (pcoi, &pcoi_rec, NEXT, "r");
	}

	return;
}

/*-------------------------
| Print Bill Of Materials |
-------------------------*/
void
print_bom (
 void)
{
	fprintf (fout, ".DS9\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".EBATCH NUMBER : %s\n", clip (pcwo_rec.wo_batch_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EBILL OF MATERIALS\n");
	fprintf (fout, 
		".CITEM: %-16.16s ALT NO: %5d   STRENGTH: %-5.5s DESCRIPTION: %-35.35s \n", 
		local_rec.item, 
		pcwo_rec.wo_bom_alt, 
		&local_rec.item_desc [35], 
		local_rec.item_desc);

	fprintf (fout, 
		".CUOM Std: %-4.4s  Alt: %-4.4s       BATCH SIZES  Std: %14.6f  Min: %14.6f  Max: %14.6f\n", 
		local_rec.std_uom, 
		local_rec.alt_uom, 
		inei_rec.ei_std_batch, 
		inei_rec.ei_min_batch, 
		inei_rec.ei_max_batch);

	fprintf (fout, "====================================================");
	fprintf (fout, "====================================================");
	fprintf (fout, "====================================================\n");

	fprintf (fout, "|     MATERIAL     ");
	fprintf (fout, "|         D E S C R I P T I O N      ");
	fprintf (fout, "|STRENGTH");
	fprintf (fout, "|HZD CLASS");
	fprintf (fout, "| QUANTITY REQD ");
	fprintf (fout, "| UOM  ");
	fprintf (fout, "|ISSUE SEQ");
	fprintf (fout, "| QUANTITY ISSUED");
	fprintf (fout, "|ANALYTICAL NO.");
	fprintf (fout, "|ISSUE COMPLETE|\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "+------------------------------------");
	fprintf (fout, "+--------");
	fprintf (fout, "+---------");
	fprintf (fout, "+---------------");
	fprintf (fout, "+------");
	fprintf (fout, "+---------");
	fprintf (fout, "+----------------");
	fprintf (fout, "+--------------");
	fprintf (fout, "+--------------|\n");

	fprintf (fout, ".R===================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "====================================================\n");

	fprintf (fout, ".PA\n");

	proc_pcms ();
	return;
}
/*----------------------
| Process pcms records |
----------------------*/
void
proc_pcms (
 void)
{
	pcms_rec.ms_hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcms_rec.ms_uniq_id   = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.ms_hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcms_rec.ms_mabr_hash);
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy (inei_rec.ei_est_no, comm_rec.test_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
			strcpy (inei_rec.ei_hzrd_class, "UNKN");

		cc = find_hash (inum, &inum_rec, EQUAL, "r", pcms_rec.ms_uom);
		strcpy (local_rec.mtl_uom,  (cc) ? "    " : inum_rec.um_uom);

		pcms_rec.ms_matl_wst_pc += 100.00;
		pcms_rec.ms_matl_wst_pc /= 100.00;

		fprintf (fout, "| %-16.16s ",    inmr_rec.mr_item_no);
		fprintf (fout, "| %-35.35s",     inmr_rec.mr_description);
		fprintf (fout, "| %-5.5s  ",     &inmr_rec.mr_description [35]);
		fprintf (fout, "|  %-4.4s   ",   inei_rec.ei_hzrd_class);
		fprintf (fout, "| %14.6f",       pcms_rec.ms_matl_qty * pcms_rec.ms_matl_wst_pc);
		fprintf (fout, "| %-4.4s ",      local_rec.mtl_uom);
		fprintf (fout, "|   %3d   ",     pcms_rec.ms_iss_seq);
		fprintf (fout, "| %14.6f ",      pcms_rec.ms_qty_issued);
		fprintf (fout, "|%-14.14s",      " ");
		fprintf (fout, "|%-14.14s|\n",   " ");

		proc_bmin ();

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	fflush (stdout);
	return;
}

/*-------------------------------
| Process BOM Instruction (s).	|
-------------------------------*/
void
proc_bmin (
 void)
{
	strcpy (bmin_rec.in_co_no, pcms_rec.ms_co_no);
	bmin_rec.in_hhbr_hash = pcms_rec.ms_hhbr_hash;
	bmin_rec.in_alt_no = pcms_rec.ms_alt_no;
	bmin_rec.in_line_no = pcms_rec.ms_line_no;
	bmin_rec.in_tline = 0;
	cc = find_rec (bmin, &bmin_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (bmin_rec.in_co_no, pcms_rec.ms_co_no) &&
		bmin_rec.in_hhbr_hash == pcms_rec.ms_hhbr_hash &&
		bmin_rec.in_alt_no == pcms_rec.ms_alt_no &&
		bmin_rec.in_line_no == pcms_rec.ms_line_no
	)
	{
		fprintf (fout, "|%-18.18s", 	" ");
		fprintf (fout, "|%-45.45s", 	bmin_rec.in_text);
		fprintf (fout, "|%-9.9s", 	" ");
		fprintf (fout, "|%-15.15s", 	" ");
		fprintf (fout, "|%-6.6s", 	" ");
		fprintf (fout, "|%-9.9s", 	" ");
		fprintf (fout, "|%-16.16s", 	" ");
		fprintf (fout, "|%-14.14s", 	" ");
		fprintf (fout, "|%-14.14s|\n", 	" ");
		cc = find_rec (bmin, &bmin_rec, NEXT, "r");
	}
}

/*---------------
| Print Routing |
---------------*/
void
print_rtg (
 void)
{
	calc_vals ();

	fprintf (fout, ".DS8\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".EBATCH NUMBER : %s\n", clip (pcwo_rec.wo_batch_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EROUTING DETAILS\n");
	fprintf (fout, "     Product Code: %-16.16s  %-35.35s  ", 
		local_rec.item, local_rec.item_desc);

	fprintf (fout, "Alternate No : %d     ", pcwo_rec.wo_rtg_alt);
	fprintf (fout, "Elapsed Time: %s          ", 
		ttoa (local_rec.tot_time, "NNNNNN:NN"));

	fprintf (fout, "Running Costs :  %9.2f\n", DOLLARS (local_rec.tot_cost));

	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");

	fprintf (fout, "|SEQ|");
	fprintf (fout, "WRK CNTR|");
	fprintf (fout, "RESOURCE|");
	fprintf (fout, "          RESOURCE DESCRIPTION          |");
	fprintf (fout, "   TYPE  |");
	fprintf (fout, "OPS|");
	fprintf (fout, " SETUP |");
	fprintf (fout, "  RUN  |");
	fprintf (fout, " CLEAN |");
	fprintf (fout, "RATE/HOUR|");
	fprintf (fout, "VAR. OVHD|");
	fprintf (fout, "FIX. OVHD|");
	fprintf (fout, "   COST  |");
	fprintf (fout, "SPL.|");
	fprintf (fout, "YLD.|");
	fprintf (fout, "INST|\n");

	fprintf (fout, "|---+");
	fprintf (fout, "--------+");
	fprintf (fout, "--------+");
	fprintf (fout, "----------------------------------------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---+");
	fprintf (fout, "-------+");
	fprintf (fout, "-------+");
	fprintf (fout, "-------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "----+");
	fprintf (fout, "----+");
	fprintf (fout, "----|\n");

	fprintf (fout, ".PA\n");
	fflush (fout);

	proc_lines ();
	cc = proc_byprd ();
	proc_instr (cc);

	return;
}
/*--------------------
| Process pcln lines |
--------------------*/
void
proc_lines (
 void)
{
	int	i;
	long	tmp_time;
	double	line_cost;

	pcln_rec.ln_hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.ln_seq_no = 0;
	pcln_rec.ln_line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.ln_hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		/*-----------------------------------------------
		| If the sequence has finished, use ACTUAL time	|
		| elapsed as held on pcrq. Otherwise, use the	|
		| ESTIMATED time from pcln.			|
		-----------------------------------------------*/
		if ((pcwo_rec.wo_rtg_seq > pcln_rec.ln_seq_no &&
			(pcwo_rec.wo_order_status [0] == 'R' ||
			pcwo_rec.wo_order_status [0] == 'C' ||
			pcwo_rec.wo_order_status [0] == 'Z')) ||
		    (pcwo_rec.wo_rtg_seq == pcln_rec.ln_seq_no &&
			(pcwo_rec.wo_order_status [0] == 'C' ||
			pcwo_rec.wo_order_status [0] == 'Z')))
		{
			pcrq_rec.rq_hhwo_hash = pcwo_rec.wo_hhwo_hash;
			pcrq_rec.rq_seq_no = pcln_rec.ln_seq_no;
			pcrq_rec.rq_line_no = pcln_rec.ln_line_no;
			cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pcrq, "DBFIND");
			pcln_rec.ln_setup = pcrq_rec.rq_act_setup;
			pcln_rec.ln_run = pcrq_rec.rq_act_run;
			pcln_rec.ln_clean = pcrq_rec.rq_act_clean;
		}
		cc = find_hash (pcwc, &pcwc_rec, EQUAL, "r", pcln_rec.ln_hhwc_hash);
		if (cc)
			strcpy (pcwc_rec.wc_work_cntr, "NOT FND.");

		cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", pcln_rec.ln_hhrs_hash);
		if (cc)
		{
			strcpy (rgrs_rec.rs_code, "NOT FND.");
			strcpy (rgrs_rec.rs_desc, "NOT FND.");
			strcpy (rgrs_rec.rs_type, " ");
		}

		strcpy (local_rec.res_desc, "NOT FND ");
		for (i = 0; i < no_res; i++)
		{
			if (rgrs_rec.rs_type [0] == res_type [i].type)
			{
				sprintf (local_rec.res_desc, 
					"%-8.8s", 
					res_type [i].desc);
				break;
			}
		}

		tmp_time =  pcln_rec.ln_setup;
		tmp_time += pcln_rec.ln_run;
		tmp_time += pcln_rec.ln_clean;

		line_cost = (double) tmp_time * (pcln_rec.ln_rate + pcln_rec.ln_ovhd_var);
		line_cost /= 60.00;
		line_cost += pcln_rec.ln_ovhd_fix;
		line_cost *= (double) pcln_rec.ln_qty_rsrc;

		fprintf (fout, ".LRP2\n");

		fprintf (fout, "|%3d|", 		pcln_rec.ln_seq_no);
		fprintf (fout, "%-8.8s|", 		pcwc_rec.wc_work_cntr);
		fprintf (fout, "%-8.8s|", 		rgrs_rec.rs_code);
		fprintf (fout, "%-40.40s|", 	rgrs_rec.rs_desc);
		fprintf (fout, " %-8.8s|", 	local_rec.res_desc);
		fprintf (fout, "%3d|", 		pcln_rec.ln_qty_rsrc);
		fprintf (fout, "%s|", 		ttoa (pcln_rec.ln_setup, "NNNN:NN"));
		fprintf (fout, "%s|", 		ttoa (pcln_rec.ln_run,   "NNNN:NN"));
		fprintf (fout, "%s|", 		ttoa (pcln_rec.ln_clean, "NNNN:NN"));
		fprintf (fout, "%9.2f|", 		DOLLARS (pcln_rec.ln_rate));
		fprintf (fout, "%9.2f|", 		DOLLARS (pcln_rec.ln_ovhd_var));
		fprintf (fout, "%9.2f|", 		DOLLARS (pcln_rec.ln_ovhd_fix));
		fprintf (fout, "%9.2f|", 		DOLLARS (line_cost));
		if (pcln_rec.ln_can_split [0] == 'Y')
			fprintf (fout, "Yes |");
		else
			if (pcln_rec.ln_can_split [0] == 'N')
				fprintf (fout, "No  |");
			else
				fprintf (fout, "Same|");
		fprintf (fout, "%-4.4s|", 		pcln_rec.ln_yld_clc);
		fprintf (fout, " %2d |", 		pcln_rec.ln_instr_no);
		if (pcwo_rec.wo_order_status [0] == 'R')
			if (pcwo_rec.wo_rtg_seq <= pcln_rec.ln_seq_no)
				fprintf (fout, "*");
		fprintf (fout, "\n");

		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");

	scn_set (1);
}
/*----------------------------------------
| Calculate running cost/time of routing |
----------------------------------------*/
void
calc_vals (
 void)
{
	double	cur_cost;
	long	cur_time, 
		tmp_time = 0L;
	int	i;

	no_stored = 0;
	pcln_rec.ln_hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.ln_seq_no = 0;
	pcln_rec.ln_line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.ln_hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		/*-----------------------------------------------
		| If the sequence has finished, use ACTUAL time	|
		| elapsed as held on pcrq. Otherwise, use the	|
		| ESTIMATED time from pcln.			|
		-----------------------------------------------*/
		if ((pcwo_rec.wo_rtg_seq > pcln_rec.ln_seq_no &&
			(pcwo_rec.wo_order_status [0] == 'R' ||
			pcwo_rec.wo_order_status [0] == 'C' ||
			pcwo_rec.wo_order_status [0] == 'Z')) ||
		    (pcwo_rec.wo_rtg_seq == pcln_rec.ln_seq_no &&
			(pcwo_rec.wo_order_status [0] == 'C' ||
			pcwo_rec.wo_order_status [0] == 'Z')))
		{
			pcrq_rec.rq_hhwo_hash = pcwo_rec.wo_hhwo_hash;
			pcrq_rec.rq_seq_no = pcln_rec.ln_seq_no;
			pcrq_rec.rq_line_no = pcln_rec.ln_line_no;
			cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pcrq, "DBFIND");
			pcln_rec.ln_setup = pcrq_rec.rq_act_setup;
			pcln_rec.ln_run = pcrq_rec.rq_act_run;
			pcln_rec.ln_clean = pcrq_rec.rq_act_clean;
		}

		store [no_stored].seq_no		= pcln_rec.ln_seq_no;
		store [no_stored].hhwc_hash	= pcln_rec.ln_hhwc_hash;
		store [no_stored].rate		= pcln_rec.ln_rate;
		store [no_stored].ovhd_var	= pcln_rec.ln_ovhd_var;
		store [no_stored].ovhd_fix	= pcln_rec.ln_ovhd_fix;
		store [no_stored].time		= pcln_rec.ln_setup +
						  pcln_rec.ln_run +
						  pcln_rec.ln_clean;
		store [no_stored].qty_rsrc	= pcln_rec.ln_qty_rsrc;
		cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", pcln_rec.ln_hhrs_hash);
		if (cc)
			strcpy (store [no_stored].type, "*");
		else
			strcpy (store [no_stored].type, rgrs_rec.rs_type);
		store [no_stored++].instr_no = pcln_rec.ln_instr_no;
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	local_rec.tot_time = 0L;
	local_rec.tot_cost = 0.00;

	for (tmp_time = 0L, i = 0; i < no_stored; i++)
	{
		cur_time = store [i].time;
		if (cur_time > tmp_time)
			tmp_time = cur_time;

		if (i + 1 == no_stored || store [i].seq_no != store [i+1].seq_no)
		{
			local_rec.tot_time += tmp_time;
			tmp_time = 0L;
		}

		cur_cost = (store [i].rate + store [i].ovhd_var) * (double) cur_time;
		cur_cost /= 60.00;
		cur_cost += store [i].ovhd_fix;
		cur_cost *= store [i].qty_rsrc;

		local_rec.tot_cost += cur_cost;
	}
}
/*---------------------------------------
| Print any by-products for the routing |
---------------------------------------*/
int
proc_byprd (
 void)
{
	int	i, 
		curr_seq, 
		new_seq, 
		first_byprd;

	fprintf (fout, ".R \n");

	fprintf (fout, ".DS8\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EROUTING DETAILS\n");
	fprintf (fout, "     Product Code: %-16.16s  %-35.35s  ", 
		inmr_rec.mr_item_no, inmr_rec.mr_description);
	
	fprintf (fout, "Alternate No : %d     ", pcwo_rec.wo_rtg_alt);
	fprintf (fout, "Elapsed Time: %s          ", 
		ttoa (local_rec.tot_time, "NNNNNN:NN"));
	
	fprintf (fout, "Running Costs :  %9.2f\n", DOLLARS (local_rec.tot_cost));
	fprintf (fout, ".B1\n");
	fprintf (fout, "%-32.32sBY PRODUCTS cont..\n", " ");
	fprintf (fout, "%-32.32s==================\n", " ");
	fprintf (fout, "%-22.22sQUANTITY  ITEM NUMBER     DESCRIPTION\n", " ");

	first_byprd = TRUE;
	curr_seq = -1;
	for (i = 0; i < no_stored; i++)
	{
		if (store [i].seq_no == curr_seq)
			continue;

		curr_seq = store [i].seq_no;

		new_seq = TRUE;

		pcbp_rec.bp_hhwo_hash	= pcwo_rec.wo_hhwo_hash;
		pcbp_rec.bp_seq_no	= store [i].seq_no;
		pcbp_rec.bp_hhbr_hash	= 0L;
		cc = find_rec (pcbp, &pcbp_rec, GTEQ, "r");
		while (!cc &&
			pcbp_rec.bp_hhwo_hash	== pcwo_rec.wo_hhwo_hash &&
			pcbp_rec.bp_seq_no	== store [i].seq_no)
		{
			if (first_byprd)
			{
				fprintf (fout, ".LRP7\n");
				fprintf (fout, ".B1\n");
				fprintf (fout, "%-32.32sBY PRODUCTS\n", " ");
				fprintf (fout, "%-32.32s===========\n", " ");
				fprintf (fout, "%-22.22sQUANTITY  ITEM NUMBER     DESCRIPTION\n", " ");
			}

			if (new_seq)
			{
				fprintf (fout, ".B1\n");
				fprintf (fout, " SEQ NO %6d", store [i].seq_no);
			}
			else
				fprintf (fout, "%-14.14s", " ");

			cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcbp_rec.bp_hhbr_hash);
			if (cc)
			{
				sprintf (inmr_rec.mr_item_no, "%-16.16s", "Not On File");
				sprintf (inmr_rec.mr_description, "%-40.40s", " ");
			}
			fprintf (fout, "  %14.6f", pcbp_rec.bp_qty);
			fprintf (fout, "  %-16.16s", inmr_rec.mr_item_no);
			fprintf (fout, "  %-40.40s\n", inmr_rec.mr_description);

			new_seq = FALSE;
			first_byprd = FALSE;
			cc = find_rec (pcbp, &pcbp_rec, NEXT, "r");
		}
	}

	return (first_byprd == FALSE);
}
/*--------------------------------
| Print any routing instructions |
--------------------------------*/
void
proc_instr (
 int brk_rqd)
{
	int	i, 
		new_seq, 
		inst_ver, 
		first_instr;

	fprintf (fout, ".R \n");

	if (brk_rqd)
	{
		fprintf (fout, "----------------------------------------");
		fprintf (fout, "----------------------------------------");
		fprintf (fout, "----------------------------------------\n");
	}
	fprintf (fout, ".DS7\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EROUTING DETAILS\n");
	fprintf (fout, "     Product Code: %-16.16s  %-35.35s  ", 
		inmr_rec.mr_item_no, inmr_rec.mr_description);
	
	fprintf (fout, "Alternate No : %d     ", pcwo_rec.wo_rtg_alt);
	fprintf (fout, "Elapsed Time: %s          ", 
		ttoa (local_rec.tot_time, "NNNNNN:NN"));
	
	fprintf (fout, "Running Costs :  %9.2f\n", DOLLARS (local_rec.tot_cost));

	fprintf (fout, ".B1\n");
	fprintf (fout, "%-16.16sINSTRUCTIONS cont..\n", " ");
	fprintf (fout, "%-16.16s===================\n", " ");

	first_instr = TRUE;
	for (i = 0; i < no_stored; i++)
	{
		new_seq = TRUE;

		inst_ver = get_inst_ver (i);
		strcpy (pcid_rec.id_co_no, comm_rec.tco_no);
		pcid_rec.id_hhbr_hash = pcwo_rec.wo_hhbr_hash;
		pcid_rec.id_hhwc_hash = store [i].hhwc_hash;
		pcid_rec.id_instr_no = store [i].instr_no;
		pcid_rec.id_version = inst_ver;
		pcid_rec.id_line_no = 0;
		cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
		while (!cc &&
			!strcmp (pcid_rec.id_co_no, comm_rec.tco_no) &&
			pcid_rec.id_hhbr_hash == pcwo_rec.wo_hhbr_hash &&
			pcid_rec.id_hhwc_hash == store [i].hhwc_hash &&
			pcid_rec.id_instr_no == store [i].instr_no &&
			pcid_rec.id_version == inst_ver)
		{
			if (first_instr)
			{
				fprintf (fout, ".LRP5\n");
				fprintf (fout, ".B1\n");
				fprintf (fout, "%-16.16sINSTRUCTIONS\n", " ");
				fprintf (fout, "%-16.16s============\n", " ");
			}

			if (new_seq)
			{
				fprintf (fout, ".B1\n");
				fprintf (fout, " SEQ NO %6d", store [i].seq_no);
			}
			else
				fprintf (fout, "%-14.14s", " ");

			fprintf (fout, "  %-60.60s\n", pcid_rec.id_text);

			new_seq = FALSE;
			first_instr = FALSE;
			cc = find_rec (pcid, &pcid_rec, NEXT, "r");
		}
	}

	return;
}
/*-------------------------------------
| Get the lastest instruction version |
-------------------------------------*/
int
get_inst_ver (
 int store_line)
{
	int	tmp_ver;

	tmp_ver = 0;

	strcpy (pcid_rec.id_co_no, comm_rec.tco_no);
	pcid_rec.id_hhbr_hash = pcwo_rec.wo_hhbr_hash;
	pcid_rec.id_hhwc_hash = store [store_line].hhwc_hash;
	pcid_rec.id_instr_no = store [store_line].instr_no;
	pcid_rec.id_version = 0;
	pcid_rec.id_line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pcid_rec.id_co_no, comm_rec.tco_no) &&
		pcid_rec.id_hhbr_hash == pcwo_rec.wo_hhbr_hash &&
		pcid_rec.id_hhwc_hash == store [store_line].hhwc_hash &&
		pcid_rec.id_instr_no == store [store_line].instr_no)
	{
		tmp_ver = pcid_rec.id_version;

		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}

	return (tmp_ver);
}



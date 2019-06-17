/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_drmaint.c   )                                 |
|  Program Desc  : ( Machine History Detail Record Maintenance.   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mhdr, mhsd, inmr, incc, insf, ccmr, cumr,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  mhdr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 16/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/10/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (29/06/89)      | Modified  by  : Rog Gibbison.    |
|  Date Modified : (08/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (14/01/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/09/93)      | Modified  by  : Aroha Merrilees. |
|  Date Modified : (11/09/97)      | Modified  by  : Roanna Marcelino |
|  Date Modified : (31/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Removed return from read_comm().                   |
|                : Add new search & screen generator.                 |
|                : (08/08/90) - General Update.                       |
|                : (14/01/91) - Added insf_chasis_no -> mhdr_chasis_no|
|  (16/09/93)    : HGP 9503 - increased cus_ord_ref to 20 chars.      |
|  (11/09/97)    : Modified for Multilingual Conversion.              |
|  (31/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                :                                                    |
| $Log: mh_drmaint.c,v $
| Revision 5.4  2002/07/18 06:48:17  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/17 09:57:28  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:22  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:21  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:01:18  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/01/20 22:02:15  cam
| Changes for GVision compatibility.  Fixed print_mess () calls.
|
| Revision 1.11  1999/11/17 06:40:20  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/08 08:09:28  scott
| Updated for fix warnings due to usage of -Wall flag.
|
| Revision 1.9  1999/09/29 10:11:22  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 09:23:24  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/15 03:03:05  scott
| Update for log and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_drmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_drmaint/mh_drmaint.c,v 5.4 2002/07/18 06:48:17 scott Exp $";

#define	PSL_PRINT
#define	X_OFF	0
#define	Y_OFF	1

#define	MAXLINES	2
#define	TABLINES	2
#define	MAXDESC		7
#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_mh_mess.h>
int	print_ok = FALSE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

	/*=====================================
	| Machine History Detail Record File. |
	=====================================*/
	struct dbview mhdr_list[] ={
		{"mhdr_co_no"},
		{"mhdr_hhcc_hash"},
		{"mhdr_hhbr_hash"},
		{"mhdr_serial_no"},
		{"mhdr_model_code"},
		{"mhdr_prod_gp"},
		{"mhdr_chasis_no"},
		{"mhdr_mfg_pur_date"},
		{"mhdr_spec1"},
		{"mhdr_spec2"},
		{"mhdr_spec3"},
		{"mhdr_spec4"},
		{"mhdr_spec5"},
		{"mhdr_spec6"},
		{"mhdr_spec7"},
		{"mhdr_spec_det_1"},
		{"mhdr_spec_det_2"},
		{"mhdr_order_no"},
		{"mhdr_order_date"},
		{"mhdr_sell_date"},
		{"mhdr_hhcu_hash"},
		{"mhdr_cust_type"},
		{"mhdr_cust_area"},
		{"mhdr_rep_no"},
		{"mhdr_inv_no"},
		{"mhdr_cost_nzd"},
		{"mhdr_val_nzd"},
		{"mhdr_war_no"},
		{"mhdr_war_exp"},
		{"mhdr_war_cost"},
		{"mhdr_ex_war_cost"},
		{"mhdr_lst_ser_date"}
	};

	int mhdr_no_fields = 32;

	struct {
		char	dr_co_no[3];
		long	dr_hhcc_hash;
		long	dr_hhbr_hash;
		char	dr_serial_no[26];
		char	dr_model_code[7];
		char	dr_prod_gp[13];
		char	dr_chasis_no[21];
		long	dr_mfg_pur_date;
		char	dr_spec[7][5];
		char	dr_spec_det[2][61];
		char	dr_order_no[17];
		long	dr_order_date;
		long	dr_sell_date;
		long	dr_hhcu_hash;
		char	dr_cust_type[4];
		char	dr_cust_area[3];
		char	dr_rep_no[3];
		char	dr_inv_no[9];
		Money	dr_cost_nzd;	
		Money	dr_val_nzd;	
		char	dr_war_no[7];
		long	dr_war_exp;
		Money	dr_war_cost;		
		Money	dr_ex_war_cost;	
		long	dr_last_serv_date;
	} mhdr_rec;

	/*==================================
	| Spec_type and Code Control File. |
	==================================*/
	struct dbview mhsd_list[] ={
		{"mhsd_co_no"},
		{"mhsd_spec_type"},
		{"mhsd_code"},
		{"mhsd_desc"}
	};

	int mhsd_no_fields = 4;

	struct {
		char	sd_co_no[3];
		char	sd_spec_type[2];
		char	sd_code[5];
		char	sd_desc[41];
	} mhsd_rec;

	/*===============================
	| Machine History Control File. |
	===============================*/
	struct dbview mccf_list[] ={
		{"mccf_co_no"},
		{"mccf_spec_type"},
		{"mccf_spec_desc"}
	};

	int mccf_no_fields = 3;

	struct {
		char	cf_co_no[3];
		char	cf_spec_type[2];
		char	cf_spec_desc[16];
	} mccf_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
	};

	int inmr_no_fields = 12;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_alpha_code[17];
		char	mr_supercession_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		char	mr_serial_item[2];
		char	mr_costing_flag[2];
	} inmr_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_acc_type"},
		{"cumr_class_type"},
		{"cumr_price_type"},
		{"cumr_payment_flag"},
		{"cumr_po_flag"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
	};

	int cumr_no_fields = 14;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_acc_type[2];
		char	cm_class_type[4];
		char	cm_price_type[2];
		int		cm_payment_flag;
		char	cm_po_flag[2];
		char	cm_area_code[3];
		char	cm_sman_code[3];
	} cumr_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_master_wh"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_type"},
		{"ccmr_sal_ok"},
		{"ccmr_pur_ok"},
		{"ccmr_issues_ok"},
		{"ccmr_receipts"},
		{"ccmr_reports_ok"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 14;

	struct {
		char	cc_co_no[3];
		char	cc_est_no[3];
		char	cc_cc_no[3];
		long	cc_hhcc_hash;
		char	cc_master_wh[2];
		char	cc_name[41];
		char	cc_acronym[10];
		char	cc_type[3];
		char	cc_sal_ok[2];
		char	cc_pur_ok[2];
		char	cc_issues_ok[2];
		char	cc_receipts[2];
		char	cc_reports_ok[2];
		char	cc_stat_flag[2];
	} ccmr_rec;

	/*====================================
	| Inventory Cost centre Base Record. |
	====================================*/
	struct dbview incc_list[] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_stat_flag"}
	};

	int incc_no_fields = 4;

	struct {
		long	ic_hhcc_hash;
		long	ic_hhbr_hash;
		long	ic_hhwh_hash;
		char	ic_stat_flag[2];
	} incc_rec;

	/*===============================
	| Inventory Serial Number file. |
	===============================*/
	struct dbview insf_list[] ={
		{"insf_hhsf_hash"},
		{"insf_hhwh_hash"},
		{"insf_status"},
		{"insf_serial_no"},
		{"insf_chasis_no"},
		{"insf_date_in"},
		{"insf_est_cost"},
		{"insf_act_cost"},
		{"insf_stat_flag"}
	};

	int insf_no_fields = 9;

	struct {
		long	sf_hhsf_hash;
		long	sf_hhwh_hash;
		char	sf_status[2];
		char	sf_serial_no[26];
		char	sf_chasis_no[21];
		long	sf_date_in;
		double	sf_est_cost;
		double	sf_act_cost;
		char	sf_stat_flag[2];
	} insf_rec;

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
		{"exsf_com_type"},
		{"exsf_com_pc"},
		{"exsf_com_min"},
		{"exsf_stat_flag"}
	};

	int exsf_no_fields = 7;

	struct {
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
		char	sf_com_type[2];
		float	sf_com_pc;
		Money	sf_com_min;
		char	sf_stat_flag[2];
	} exsf_rec;

	/*============================================
	| Customer Order/Invoice/Credit Header File. |
	============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_cus_ord_ref"},
		{"cohr_cons_no"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
		{"cohr_area_code"},
		{"cohr_sale_code"},
		{"cohr_gross"},
		{"cohr_freight"},
		{"cohr_sos"},
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
		{"cohr_stat_flag"},
	};

	int cohr_no_fields = 26;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		char	hr_cus_ord_ref[21];
		char	hr_cons_no[17];
		long	hr_hhco_hash;
		long	hr_date_raised;
		long	hr_date_required;
		char	hr_area_code[3];
		char	hr_sale_code[3];
		Money	hr_gross;
		Money	hr_freight;	
		Money	hr_sos;	
		Money	hr_insurance;
		Money	hr_o_cost[3];
		Money	hr_tax;	
		Money	hr_gst;	
		Money	hr_disc;
		Money	hr_deposit;
		Money	hr_ex_disc;
		Money	hr_erate_var;		
		char	hr_stat_flag[2];
	} cohr_rec;

	char	*scn_desc[] = {
		"Machine Specification.",
		"Specification free-text.",
		"Machine Sales Details.",
		"Machine Warranty/Service Details."
	};

	int		valid_serial = 0;
   	int 	new_item = 0;
	int		prog_type;
	int		envDbCo = 0,
			envDbFind = 0;

	char	spec[7][16];
	char	branchNo[3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	_class[2];
	char	cat[12];
	char	spec_desc[7][41];
	char	details[61];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "item_no",	 4, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.", " ",
		 NE, NO,  JUSTLEFT, "", "", inmr_rec.mr_item_no},
	{1, LIN, "item_desc",	 5, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.mr_description},
	{1, LIN, "serial_no",	 6, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Serial No.", " ",
		 NE, NO,  JUSTLEFT, "", "", mhdr_rec.dr_serial_no},
	{1, LIN, "model_code",	 7, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Model Code.", " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_model_code},
	{1, LIN, "dummy",	 8, 16, CHARTYPE,
		"U", "          ",
		" ", " ", "Class.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec._class},
	{1, LIN, "dummy",	 8, 60, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cat},
	{1, LIN, "chasis_no",	 9, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Chasis No.", " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_chasis_no},
	{1, LIN, "mfg_pur_date",	10, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Mfg/Purch. Date", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_mfg_pur_date},
	{1, LIN, "cost_nzd",	10, 60, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Cost (NZD).", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_cost_nzd},
	{1, LIN, "spec1",	12, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[0], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[0]},
	{1, LIN, "desc1",	12, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[0]},
	{1, LIN, "spec2",	13, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[1], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[1]},
	{1, LIN, "desc2",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[1]},
	{1, LIN, "spec3",	14, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[2], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[2]},
	{1, LIN, "desc3",	14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[2]},
	{1, LIN, "spec4",	15, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[3], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[3]},
	{1, LIN, "desc4",	15, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[3]},
	{1, LIN, "spec5",	16, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[4], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[4]},
	{1, LIN, "desc5",	16, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[4]},
	{1, LIN, "spec6",	17, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[5], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[5]},
	{1, LIN, "desc6",	17, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[5]},
	{1, LIN, "spec7",	18, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", spec[6], " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_spec[6]},
	{1, LIN, "desc7",	18, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.spec_desc[6]},
	{2, TAB, "detail_spec",	MAXLINES, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                    Specification Details                     ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.details},
	{3, LIN, "order_no",	 4, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Order No.", " ",
		YES, NO,  JUSTLEFT, "", "", mhdr_rec.dr_order_no},
	{3, LIN, "order_date",	 5, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Order Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_order_date},
	{3, LIN, "sell_date",	 6, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Sell Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_sell_date},
	{3, LIN, "cust_no",	 8, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer Code.", " ",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.cm_dbt_no},
	{3, LIN, "cust_name",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.cm_name},
	{3, LIN, "cust_type",	 9, 16, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Class Type.", " ",
		 NA, NO,  JUSTLEFT, "", "", mhdr_rec.dr_cust_type},
	{3, LIN, "class_desc",	 9, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", excl_rec.cl_class_description},
	{3, LIN, "cust_area",	10, 16, CHARTYPE,
		"UU", "          ",
		" ", " ", "Area Code.", " ",
		 NA, NO,  JUSTLEFT, "", "", mhdr_rec.dr_cust_area},
	{3, LIN, "area_name",	10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.af_area},
	{3, LIN, "rep_no",	11, 16, CHARTYPE,
		"UU", "          ",
		" ", " ", "Salesman Code.", " ",
		 NA, NO,  JUSTLEFT, "", "", mhdr_rec.dr_rep_no},
	{3, LIN, "sman_name",	11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.sf_salesman},
	{3, LIN, "inv_no",	13, 16, CHARTYPE,
		"UUUUUUUU", "          ",
		"0", "0", "Invoice No.", " ",
		YES, NO, JUSTRIGHT, "", "", mhdr_rec.dr_inv_no},
	{3, LIN, "value_nzd",	14, 16, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Value (NZD).", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_val_nzd},
	{4, LIN, "war_no",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Warranty No.", " ",
		YES, NO, JUSTRIGHT, "", "", mhdr_rec.dr_war_no},
	{4, LIN, "war_exp",	 5, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Expiry Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_war_exp},
	{4, LIN, "war_cost",	 6, 20, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Costs.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_war_cost},
	{4, LIN, "ex_war_cost",	 7, 20, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Ex-Costs.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_ex_war_cost},
	{4, LIN, "last_serv",	 8, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Last Service Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&mhdr_rec.dr_last_serv_date},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindCumr.h>
#include	<inmr_csearch.h>
#include	<ser_value.h>

/*=====================
| function prototypes |
=====================*/
#include	<std_decs.h>
void OpenDB (void);
void CloseDB (void);
int ReadMisc (void);
void get_mccf (void);
int spec_valid (int field);
void load_tabular (void);
void find_excl (char *class_type);
void find_exaf (char *area_code);
void find_exsf (char *sman_code);
void load_scn_3 (void);
int Get_desc (int spec);
void update (void);
int show_imhdr (char *key_val);
int show_smhdr (char *key_val);
int show_insf (char *key_val);
int show_spec (char *key_val, int spec);
int show_inv (char *key_val);
int heading (int scn);
void psl_print (void);
void shutdown_prog (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main(
 int  argc, 
 char *argv[])
{
	int	field;
	int	i;

	if (argc != 2)
	{
		/*printf("Usage : %s <prog_type>\007\n\r",argv[0]);
		printf("<prog_type> - M(aintenance\n\r");
		printf("            - D(isplay\n\r");*/
		print_at(0,0,ML(mlMhMess007),argv[0]);

		return (EXIT_FAILURE);
	}


	SETUP_SCR (vars);

	switch (argv[1][0])
	{
	case	'M':
	case	'm':
		prog_type = ENTRY;
		break;

	case	'D':
	case	'd':
		prog_type = DISPLAY;
		break;

	default:
		/*printf("<prog_type> - M(aintenance\n\r");
		printf("            - D(isplay\n\r");*/
		print_at(0,0,ML(mlMhMess007),argv[0]);

		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	for (field = 3;prog_type == DISPLAY && FIELD.scn > 0;field++)
		FIELD.required = NA;


	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr();
	set_tty();
	set_masks();

	for (i = 0;i < 4;i++)
		tab_data[i]._desc = scn_desc[i];

	init_vars(1);

	envDbFind = atoi(get_env("DB_FIND"));
	envDbCo = atoi(get_env("DB_CO"));

	OpenDB();
	ReadMisc();

	strcpy( branchNo, (envDbCo) ? comm_rec.test_no : " 0" );

	get_mccf();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		/*-------------------------------
		| Enter screen 2 tabular input. |	
		-------------------------------*/
		if (new_item)
		{
			heading(2);
			entry(2);
			if (restart)
				continue;

			/*-------------------------------
			| Enter screen 3 linear input . |	
			-------------------------------*/
			heading(3);
			entry(3);
			if (restart)
				continue;

			/*-------------------------------
			| Enter screen 4 linear input . |	
			-------------------------------*/
			heading(4);
			entry(4);
			if (restart)
				continue;
		}

		print_ok = TRUE;
		edit_all();
		print_ok = FALSE;
		if (restart || prog_type == DISPLAY)
			continue;

		update();
	}	/* end of input control loop	*/

	shutdown_prog();
	return (EXIT_SUCCESS);
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
OpenDB(
 void)
{
	abc_dbopen("data");
	open_rec("cohr",cohr_list,cohr_no_fields,"cohr_id_no");
	open_rec("mhdr",mhdr_list,mhdr_no_fields,"mhdr_serial_id");
	open_rec("mccf",mccf_list,mccf_no_fields,"mccf_id_no");
	open_rec("mhsd",mhsd_list,mhsd_no_fields,"mhsd_id_no");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec("incc",incc_list,incc_no_fields,"incc_id_no");
	open_rec("insf",insf_list,insf_no_fields,"insf_id_no2");
	open_rec("cumr",cumr_list,cumr_no_fields,"cumr_id_no");
	open_rec("excl",excl_list,excl_no_fields,"excl_id_no");
	open_rec("exaf",exaf_list,exaf_no_fields,"exaf_id_no");
	open_rec("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB(
 void)
{
	abc_fclose("cohr");
	abc_fclose("mhdr");
	abc_fclose("mccf");
	abc_fclose("mhsd");
	abc_fclose("inmr");
	abc_fclose("incc");
	abc_fclose("insf");
	abc_fclose("cumr");
	abc_fclose("exsf");
	abc_fclose("exaf");
	abc_fclose("excl");
	abc_dbclose("data");
}

/*============================================
| Get common info from common database file. |
============================================*/
int
ReadMisc(
 void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");

	strcpy(ccmr_rec.cc_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.cc_est_no,comm_rec.test_no);
	strcpy(ccmr_rec.cc_cc_no,comm_rec.tcc_no);
	cc = find_rec("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose("ccmr");
	return (EXIT_FAILURE);
}

void
get_mccf(
 void)
{
	int	i;

	strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
	strcpy(mccf_rec.cf_spec_type," ");
	cc = find_rec("mccf",&mccf_rec,GTEQ,"r");
	while (!cc && !strcmp(mccf_rec.cf_co_no,comm_rec.tco_no))
	{
		i = atoi(mccf_rec.cf_spec_type);

		if (strcmp(mccf_rec.cf_spec_desc,"               ") != 0)
			strcpy(spec[i - 1],mccf_rec.cf_spec_desc);
		else
			strcpy(spec[i - 1]," ");

		cc = find_rec("mccf",&mccf_rec,NEXT,"r");
	}
}

int
spec_valid(
 int field)
{
	int	i;
	/*-----------------------
	| Validate product code |
	-----------------------*/
	if (LCHECK("item_no"))
	{
		if (SRCH_KEY)
		{
			if (prog_type == ENTRY)
				stck_search(temp_str,"Y ");
			else
				show_imhdr(temp_str);
			return(0);
		}
		strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
		cc = find_rec("inmr",&inmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (inmr_rec.mr_serial_item[0] != 'Y')
		{
			print_mess(ML(mlStdMess236));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		incc_rec.ic_hhcc_hash = ccmr_rec.cc_hhcc_hash;
		incc_rec.ic_hhbr_hash = inmr_rec.mr_hhbr_hash;
		cc = find_rec("incc",&incc_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (prog_type == DISPLAY)
		{
			strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
			mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
			mhdr_rec.dr_hhbr_hash = inmr_rec.mr_hhbr_hash;
			sprintf(mhdr_rec.dr_serial_no,"%-25.25s"," ");
			cc = find_rec("mhdr",&mhdr_rec,GTEQ,"r");
			if (cc || strcmp(mhdr_rec.dr_co_no,comm_rec.tco_no) || 
			     mhdr_rec.dr_hhcc_hash != ccmr_rec.cc_hhcc_hash || 
			     mhdr_rec.dr_hhbr_hash != inmr_rec.mr_hhbr_hash)
			{
				print_mess(ML(mlMhMess013));
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
		}
		/*-----------------------------------------------
		| Format the item class & category for display. |
		-----------------------------------------------*/
		sprintf(local_rec._class,"%-1.1s",inmr_rec.mr_class);
		sprintf(local_rec.cat,"%-11.11s",inmr_rec.mr_category);
		display_field(label("item_desc"));
		return(0);
	}

	/*-----------------------
	| Validate serial no.   |
	-----------------------*/
	if (LCHECK("serial_no"))
	{
		if (SRCH_KEY)
		{
			if (prog_type == ENTRY)
				show_insf(temp_str);
			else
				show_smhdr(temp_str);
			return(0);
		}

		if (!strcmp(mhdr_rec.dr_serial_no,"                         "))
		{
			print_mess(ML(mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
	
		insf_rec.sf_hhwh_hash = incc_rec.ic_hhwh_hash;
		strcpy(insf_rec.sf_serial_no,mhdr_rec.dr_serial_no);
		cc = find_rec("insf",&insf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess201));
			i = prmptmsg(ML(mlMhMess023),"YyNn",1,2);
			clear_mess ();
			if (i != 'Y' && i != 'y')
				return(1);

			valid_serial = FALSE;
		}
		else
			valid_serial = TRUE;

		strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
		mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
		mhdr_rec.dr_hhbr_hash = inmr_rec.mr_hhbr_hash;
		cc = find_rec("mhdr",&mhdr_rec,COMPARISON,"u");
		if (cc)
		{
			if (prog_type == DISPLAY)
			{
				print_mess(ML(mlMhMess014));
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
			new_item = TRUE;
			sprintf(mhdr_rec.dr_prod_gp,"%-1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);
		}
		else
		{
			new_item = FALSE;
			load_tabular();
			load_scn_3();
			entry_exit = TRUE;
		}

		for (i = 0;i < 7;i++)
		{
			Get_desc(i + 1);
			display_field(field + i);
			display_field(field + i + 1);
		}

		if (valid_serial)
		{
			vars[label("cost_nzd")].required = NA;
			vars[label("value_nzd")].required = NA;

	   		mhdr_rec.dr_cost_nzd = ser_value( insf_rec.sf_est_cost,
						  	  insf_rec.sf_act_cost);

			mhdr_rec.dr_cost_nzd = CENTS(mhdr_rec.dr_cost_nzd);
			display_field(label("cost_nzd"));
		}
		else
		{
			vars[label("cost_nzd")].required = NO;
			vars[label("value_nzd")].required = NO;
		}
	   	strcpy(mhdr_rec.dr_chasis_no, insf_rec.sf_chasis_no);
		if ( strcmp(mhdr_rec.dr_chasis_no,"                    "))
		{
			vars[label("chasis_no")].required = NI;
			display_field(label("chasis_no"));
		}

		return(0);
	}

	if (strncmp(FIELD.label,"spec",4) == 0)
	{
		i = atoi(FIELD.label + 4);

		if (SRCH_KEY)
		{
			show_spec(temp_str,i);
			return(0);
		}
		if ( !strcmp(mhdr_rec.dr_spec[i - 1],"    "))
			return(0);

		if ( Get_desc(i) )
		{
			print_mess(ML(mlMhMess015));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		display_field(field);
		display_field(field + 1);
	}

	/*--------------------
	| Validate cust code |
	--------------------*/
	if (LCHECK("cust_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return(0);
		}
		strcpy(cumr_rec.cm_co_no,  comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no, branchNo);
		strcpy(cumr_rec.cm_dbt_no, pad_num(cumr_rec.cm_dbt_no));
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		mhdr_rec.dr_hhcu_hash = cumr_rec.cm_hhcu_hash;
		strcpy(mhdr_rec.dr_cust_type,cumr_rec.cm_class_type);
		strcpy(mhdr_rec.dr_cust_area,cumr_rec.cm_area_code);
		strcpy(mhdr_rec.dr_rep_no,cumr_rec.cm_sman_code);
		/*-------------------------------
		| Display the debtor details.	|
		-------------------------------*/
		DSP_FLD ("cust_name");
		DSP_FLD ("cust_type");
		find_excl(mhdr_rec.dr_cust_type);
		DSP_FLD ("class_desc");
		DSP_FLD ("cust_area");
		find_exaf(mhdr_rec.dr_cust_area);
		DSP_FLD ("area_name");
		DSP_FLD ("rep_no");
		find_exsf(mhdr_rec.dr_rep_no);
		DSP_FLD ("sman_name");
		return(0);
	}

	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if ( LCHECK("inv_no") ) 
	{
		if (SRCH_KEY)
		{
			show_inv(temp_str);
			return(0);
		}

		/*------------------------------
		| Check if invoice is on file. |
		------------------------------*/
		strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
		strcpy(cohr_rec.hr_type,"I");
		cohr_rec.hr_hhcu_hash = cumr_rec.cm_hhcu_hash;
		strcpy(cohr_rec.hr_inv_no,zero_pad(cohr_rec.hr_inv_no, 8));

		cc = find_rec("cohr",&cohr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess115));
			i = prmptmsg(ML(mlStdMess107),"YyNn",1,2);
			clear_mess ();
			if (i != 'Y' && i != 'y')
				return(1);
		}
		else
		{
	   		mhdr_rec.dr_val_nzd = (( cohr_rec.hr_gross + 
				         	 cohr_rec.hr_tax + 
				         	 cohr_rec.hr_gst + 
				         	 cohr_rec.hr_freight +
				         	 cohr_rec.hr_insurance +
				         	 cohr_rec.hr_o_cost[0] +
				         	 cohr_rec.hr_o_cost[1] +
				         	 cohr_rec.hr_o_cost[2] +
				         	 cohr_rec.hr_sos +
				         	 cohr_rec.hr_erate_var +
				         	 cohr_rec.hr_sos) -
				        	 (cohr_rec.hr_disc + 
					 	 cohr_rec.hr_ex_disc));
 	
			display_field(label("value_nzd"));
		}
		return(0);
	}
	return(0);             
}

/*===============
| Load scn 2	|
===============*/
void
load_tabular(
 void)
{
	scn_set(2);
	for (line_cnt = 0;line_cnt < MAXLINES;line_cnt++)
	{
		strcpy(local_rec.details,mhdr_rec.dr_spec_det[line_cnt]);
		putval(line_cnt);
	}
	lcount[2] = MAXLINES;
	scn_set(1);
}

/*===============================
| Find the class description.	|
===============================*/
void
find_excl(
 char *class_type)
{
	strcpy(excl_rec.cl_co_no,comm_rec.tco_no);
	strcpy(excl_rec.cl_class_type,class_type);
	cc = find_rec("excl",&excl_rec,COMPARISON,"r");
	if (cc)
	      strcpy(excl_rec.cl_class_description,"No Class description found.");
}

/*===============================
| Find the area description.	|
===============================*/
void
find_exaf(
 char *area_code)
{
	strcpy(exaf_rec.af_co_no,comm_rec.tco_no);
	strcpy(exaf_rec.af_area_code,area_code);
	cc = find_rec("exaf",&exaf_rec,COMPARISON,"r");
	if (cc)
	      strcpy(exaf_rec.af_area,"No Area description found.");
}

/*===============================
| Find the salesman's name.	|
===============================*/
void
find_exsf(
 char *sman_code)
{
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	strcpy(exsf_rec.sf_salesman_no,sman_code);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
	      strcpy(exsf_rec.sf_salesman,"No salesman name found.");
}

void
load_scn_3(
 void)
{
	scn_set(3);

	abc_selfield("cumr","cumr_hhcu_hash");
	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",mhdr_rec.dr_hhcu_hash);
	if (cc)
	{
		/*-------------------------------
		| Initialise the cumr fields.	|
		-------------------------------*/
		cumr_rec.cm_hhcu_hash = 0L;
		strcpy(cumr_rec.cm_dbt_no,"      ");
		sprintf(cumr_rec.cm_name,"%-40.40s"," ");
		strcpy(cumr_rec.cm_class_type,"   ");
		strcpy(cumr_rec.cm_area_code,"  ");
		strcpy(cumr_rec.cm_sman_code,"  ");
		sprintf(excl_rec.cl_class_description,"%-40.40s"," ");
		sprintf(exaf_rec.af_area,"%-40.40s"," ");
		sprintf(exsf_rec.sf_salesman,"%-40.40s"," ");
	}
	else
	{
		find_excl(cumr_rec.cm_class_type);
		find_exaf(cumr_rec.cm_area_code);
		find_exsf(cumr_rec.cm_sman_code);
	}
	abc_selfield("cumr","cumr_id_no");
	scn_set(1);
}

/*===============================================
| Determine the appropriate spec descriptions.	|
===============================================*/
int
Get_desc(
 int spec)
{
/*	int	ofset;*/

	strcpy(mhsd_rec.sd_co_no,comm_rec.tco_no);
	sprintf(mhsd_rec.sd_spec_type,"%d",spec);
	sprintf(mhsd_rec.sd_code,"%-4.4s",mhdr_rec.dr_spec[spec - 1]);
	if (find_rec("mhsd",&mhsd_rec,COMPARISON,"r"))
		return(1);

	strcpy(local_rec.spec_desc[spec - 1],mhsd_rec.sd_desc);
	return(0);
}

void
update(
 void)
{
	strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
	mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
	mhdr_rec.dr_hhbr_hash = inmr_rec.mr_hhbr_hash;
	if (valid_serial)
		strcpy(mhdr_rec.dr_serial_no,insf_rec.sf_serial_no);

	scn_set(2);
	for (line_cnt = 0;line_cnt < MAXLINES;line_cnt++)
	{
		getval(line_cnt);
		strcpy(mhdr_rec.dr_spec_det[line_cnt],local_rec.details);
	}

	if (new_item)
        {
                cc = abc_add("mhdr",&mhdr_rec);
                if (cc)
                       sys_err("Error in mhdr During (DBADD)",cc,PNAME);
        }
        else
        {
                cc = abc_update("mhdr",&mhdr_rec);
                if (cc)
                       sys_err("Error in mhdr During (DBUPDATE)",cc,PNAME);
        }
	abc_unlock("mhdr");
}

/*====================
| Search for item_no |
====================*/
int
show_imhdr(
 char *key_val)
{
	int	i;

        work_open();
	abc_selfield("mhdr","mhdr_hhbr_hash");
	save_rec("#Product Code    ","#Product Description");
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",key_val);
	cc = find_rec("inmr", &inmr_rec, GTEQ, "r");
        while (!cc && !strcmp(inmr_rec.mr_co_no,comm_rec.tco_no) && 
			!strncmp(inmr_rec.mr_item_no,key_val,strlen(key_val)))
    	{                        
		cc = find_hash("mhdr",&mhdr_rec,COMPARISON,"r",inmr_rec.mr_hhbr_hash);
		if (!cc)
		{
			cc = save_rec(inmr_rec.mr_item_no,inmr_rec.mr_description);
			if (cc)
				break;
		}
		cc = find_rec("inmr",&inmr_rec,NEXT,"r");
	}
	abc_selfield("mhdr","mhdr_serial_id");
	cc = disp_srch();
	work_close();
	if (cc)
		return (EXIT_SUCCESS);

	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",temp_str);
	cc = find_rec("inmr", &inmr_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in inmr During (DBFIND)", cc, PNAME);

	sprintf(mhdr_rec.dr_serial_no,"%25.25s"," ");
	sprintf(mhdr_rec.dr_model_code,"%6.6s"," ");
	sprintf(mhdr_rec.dr_prod_gp,"%12.12s"," ");
	sprintf(mhdr_rec.dr_chasis_no,"%20.20s"," ");
	mhdr_rec.dr_mfg_pur_date = 0L;
	for (i = 0; i < 7;i++)
		strcpy(mhdr_rec.dr_spec[i],"    ");
	return (EXIT_FAILURE);
}

/*=======================
| Search for serial no.	|
=======================*/
int
show_smhdr(
 char *key_val)
{
        work_open();
	save_rec("#Serial Number            ","#Product Description");
	strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
	mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
	mhdr_rec.dr_hhbr_hash = inmr_rec.mr_hhbr_hash;
	sprintf(mhdr_rec.dr_serial_no,"%-25.25s",key_val);
	cc = find_rec("mhdr", &mhdr_rec, GTEQ, "r");
        while (!cc && !strcmp(mhdr_rec.dr_co_no,comm_rec.tco_no) && 
			mhdr_rec.dr_hhcc_hash == ccmr_rec.cc_hhcc_hash && 
			mhdr_rec.dr_hhbr_hash == inmr_rec.mr_hhbr_hash && 
			!strncmp(mhdr_rec.dr_serial_no,key_val,strlen(key_val)))
    	{                        
	       	cc = save_rec(mhdr_rec.dr_serial_no,inmr_rec.mr_description);
		if (cc)
		       	break;
		cc = find_rec("mhdr",&mhdr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return (EXIT_SUCCESS);
	strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
	mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
	mhdr_rec.dr_hhbr_hash = inmr_rec.mr_hhbr_hash;
	sprintf(mhdr_rec.dr_serial_no,"%-25.25s",temp_str);
	cc = find_rec("mhdr", &mhdr_rec, COMPARISON, "r");
	if (cc)
 	         sys_err("Error in mhdr During (DBFIND)", cc, PNAME);
	return (EXIT_FAILURE);
}

/*=======================
| Search for serial no.	|
=======================*/
int
show_insf(
 char *key_val)
{
        work_open();
	save_rec("#Serial Number            ","#");
	insf_rec.sf_hhwh_hash = incc_rec.ic_hhwh_hash;
	sprintf(insf_rec.sf_serial_no,"%-25.25s",key_val);
	cc = find_rec("insf",&insf_rec,GTEQ,"r");
        while (!cc && insf_rec.sf_hhwh_hash == incc_rec.ic_hhwh_hash && 
			!strncmp(insf_rec.sf_serial_no,key_val,strlen(key_val)))
    	{                        
	       	cc = save_rec(insf_rec.sf_serial_no," ");
		if (cc)
		       	break;
		cc = find_rec("insf",&insf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return (EXIT_SUCCESS);
	insf_rec.sf_hhwh_hash = incc_rec.ic_hhwh_hash;
	sprintf(insf_rec.sf_serial_no,"%-25.25s",temp_str);
	cc = find_rec("insf",&insf_rec,COMPARISON,"r");
	if (cc)
 	         sys_err("Error in insf During (DBFIND)",cc,PNAME);
	return (EXIT_FAILURE);
}

/*===============================
| Search for spec description.	|
===============================*/
int
show_spec(
 char *key_val, 
 int  spec)
{
	char	spec_type[2];

	sprintf(spec_type,"%d",spec);
        work_open();
	save_rec("#Code","#Description");
	strcpy(mhsd_rec.sd_co_no,comm_rec.tco_no);
	sprintf(mhsd_rec.sd_spec_type,"%d",spec);
	sprintf(mhsd_rec.sd_code,"%-4.4s",key_val);
	cc = find_rec("mhsd",&mhsd_rec,GTEQ,"r");
        while (!cc && !strcmp(mhsd_rec.sd_co_no,comm_rec.tco_no) && 
			mhsd_rec.sd_spec_type[0] == spec_type[0] && 
			!strncmp(mhsd_rec.sd_code,key_val,strlen(key_val)))
    	{                        
	       	cc = save_rec(mhsd_rec.sd_code,mhsd_rec.sd_desc);
		if (cc)
		       	break;
		cc = find_rec("mhsd",&mhsd_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return (EXIT_SUCCESS);
	strcpy(mhsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy(mhsd_rec.sd_spec_type,spec_type);
	sprintf(mhsd_rec.sd_code,"%-4.4s",temp_str);
	cc = find_rec("mhsd",&mhsd_rec,COMPARISON,"r");
	if (cc)
 	         sys_err("Error in mhsd During (DBFIND)",cc,PNAME);
	return (EXIT_FAILURE);
}

int
show_inv(
 char *key_val)
{
	work_open();
	save_rec("#Invoice No","#Cust Order");
	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,"I");
	sprintf(cohr_rec.hr_inv_no,"%-8.8s",key_val);
	cohr_rec.hr_hhcu_hash = cumr_rec.cm_hhcu_hash;
	cc = find_rec("cohr",&cohr_rec,GTEQ,"r");

	while (!cc && !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
		      !strcmp(cohr_rec.hr_br_no,comm_rec.test_no) && 
		      !strncmp(cohr_rec.hr_inv_no,key_val,strlen(key_val)) &&
		      cohr_rec.hr_hhcu_hash == cumr_rec.cm_hhcu_hash) 
	{
		cc = save_rec(cohr_rec.hr_inv_no,cohr_rec.hr_cus_ord_ref);
		if (cc)
			break;

		cc = find_rec("cohr",&cohr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return (EXIT_SUCCESS);

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,"I");
	cohr_rec.hr_hhcu_hash = cumr_rec.cm_hhcu_hash;
	sprintf(cohr_rec.hr_inv_no,"%-8.8s",temp_str);

	cc = find_rec("cohr",&cohr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in cohr during (DBFIND)",cc,PNAME);
	return (EXIT_FAILURE);
}

int
heading(
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlMhMess008),26,0,1);
		move(0,1);
		line(80);

		if (scn == 1)
		{
			box(0,3,80,15);
			/*Machine Specification Detail*/
			us_pr(ML(mlMhMess009),5,3,1);
			move(1,11);
			line(79);
			/* Specifications ",5,11,1*/
			us_pr(ML(mlMhMess010),5,11,1);
		}

		if (scn == 3)
		{
			box(0,3,80,11);
			/* Sales Detail ",5,3,1*/
			us_pr(ML(mlMhMess011),5,3,1);
			move(1,7);
			line(79);
			move(1,12);
			line(79);
		}

		if (scn == 4)
		{
			box(0,3,80,5);
			/* Warranty / Service Detail */
			us_pr(ML(mlMhMess012),5,3,1);
		}

		strcpy(err_str,ML(mlStdMess038));
		print_at(20,0,err_str,comm_rec.tco_no,comm_rec.tco_name);

		strcpy(err_str,ML(mlStdMess039));
		print_at(21,0,err_str,comm_rec.test_no,comm_rec.test_name);

		strcpy(err_str,ML(mlStdMess099));
		print_at(22,0,err_str,comm_rec.tcc_no,comm_rec.tcc_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_FAILURE);
}

void
psl_print (void)
{
	char	tmp_date[11];
	FILE	*fout;
	int	loop;

	if (!print_ok)
		return;

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", get_lpno (0));
	fprintf (fout, ".5\n");
	fprintf (fout, ".L132\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.tco_name));
	fprintf (fout, ".EMACHINE HISTORY DETAIL\n\n");
	fprintf (fout, ".ESPECIFICATION DETAIL\n");
	fprintf (fout, "====================================================================================================================================\n");
	fprintf (fout, "PRODUCT     %-16.16s     %-40.40s\n",
		inmr_rec.mr_item_no,
		inmr_rec.mr_description);
	fprintf (fout, "SERIAL NO.  %-25.25s\n",
		mhdr_rec.dr_serial_no);
	fprintf (fout, "MODEL  %-6.6s   PROD. CLASS  %-1.1s  CATEG. %-11.11s   CHASSIS %-20.20s\n",
		mhdr_rec.dr_model_code,
		local_rec._class,
		local_rec.cat,
		mhdr_rec.dr_chasis_no);

	fprintf (fout, "MFG/PUR DATE    %-10.10s\n",DateToString (mhdr_rec.dr_mfg_pur_date));
	for (loop = 0; loop < 7; loop++)
		fprintf (fout, "%-15.15s %-4.4s  %-40.40s\n",
			spec[loop],
			mhdr_rec.dr_spec[loop],
			local_rec.spec_desc[loop]);
	fprintf (fout, "SPECIF. DETAIL  %-60.60s\n",
		mhdr_rec.dr_spec_det[0]);
	fprintf (fout, "                %-60.60s\n",
		mhdr_rec.dr_spec_det[1]);
	fprintf (fout, "\n");
	fprintf (fout, ".ESALES DETAIL\n");
	fprintf (fout, "====================================================================================================================================\n");
	sprintf (tmp_date, DateToString (mhdr_rec.dr_order_date));

	fprintf (fout, "SALES ORDER NO.  %-16.16s      ORDER DATE  %-10.10s  DELIV/SOLD DATE  %-10.10s\n",
			mhdr_rec.dr_order_no,
			tmp_date,
			DateToString (mhdr_rec.dr_sell_date));
	fprintf (fout, "CUSTOMER    %-6.6s   %-40.40s   TYPE %-3.3s%-40.40s\n",
		cumr_rec.cm_dbt_no,
		cumr_rec.cm_name,
		mhdr_rec.dr_cust_type,
		excl_rec.cl_class_description);
	fprintf (fout, "SALES AREA  %-2.2s  %-40.40s        REP  %-2.2s  %-40.40s\n",
		mhdr_rec.dr_cust_area,
		exaf_rec.af_area,
		mhdr_rec.dr_rep_no,
		exsf_rec.sf_salesman);
	fprintf (fout, "INVOICE    %-8.8s          COST   %10.2f        PRICE   %10.2f\n",
		mhdr_rec.dr_inv_no,
		DOLLARS (mhdr_rec.dr_cost_nzd),
		DOLLARS (mhdr_rec.dr_val_nzd));
	fprintf (fout, "\n");
	fprintf (fout, ".EWARRANTY/SERVICE DETAIL\n");
	fprintf (fout, "====================================================================================================================================\n");
	sprintf (tmp_date, DateToString (mhdr_rec.dr_war_exp));
	fprintf (fout, "WARRANTY NO.   %-6.6s          EXPIRY DATE  %-10.10s  LAST SERVICE  %-10.10s\n",
			mhdr_rec.dr_war_no,
			tmp_date,
			DateToString (mhdr_rec.dr_last_serv_date));

	fprintf (fout, "WARRANTY COSTS   %10.2f    EX WARR. SERVICE   %10.2f    TOTAL  %10.2f\n",
		DOLLARS (mhdr_rec.dr_war_cost),
		DOLLARS (mhdr_rec.dr_ex_war_cost),
		DOLLARS (mhdr_rec.dr_war_cost + mhdr_rec.dr_ex_war_cost));
	fprintf (fout, "====================================================================================================================================\n");
	fprintf (fout, ".EOF\n");

	pclose (fout);

	heading (cur_screen);
	scn_display (cur_screen);
}

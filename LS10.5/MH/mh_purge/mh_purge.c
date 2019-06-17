/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_purge.c )                                     |
|  Program Desc  : ( Print & Purge Machine History Records        )   |
|                  ( MH02                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  mhdr, mhsd, comm, cumr, ccmr, inmr, incc,         |
|		 :  insf, excl, exaf, exsf                            |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 01/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (30/09/87)      | Modified by : Fui Choo Yap.      |
|  Date Modified : (30/06/89)      | Modified by : Rog Gibbison.      |
|  Date Modified : (18/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (10/04/94)      | Modified by : Roel Michels       |
|  Date Modified : (12.10.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (12.09.97)      | Modified by : Rowena S Maandig   |
|  Date Modified : (31/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (13/08/1999)    | Modified by : Mars dela Cruz.    |
|                                                                     |
|  Comments      : (30/06/89) General Tidy Up & Debug                 |
|     (18/08/93) : HGP 9649 Fix to compile on SVR4                    |
|     (10/04/94) : PSL 10673 - Online conversion                      |
|     (12.10.94) : PSL 11417 Minor cleanup to remove complaints       |
|     (12.09.97) : Incorporated multilingual conversion.              |
|  (31/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|  (13/08/1999)  : Convert all character based source for ANSI -      |
|                : compliant.                                         |         |                :                                                    |
| $Log: mh_purge.c,v $
| Revision 5.2  2001/08/09 09:14:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:34  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:01:20  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/02/18 01:58:28  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.14  1999/09/29 10:11:22  scott
| Updated to be consistant on function names.
|
| Revision 1.13  1999/09/17 09:23:25  scott
| Updated from Ansi
|
| Revision 1.12  1999/07/16 00:05:09  scott
| Updated for abc_delete
|
| Revision 1.11  1999/06/15 03:05:26  scott
| Update for log + database name.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_purge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_purge/mh_purge.c,v 5.2 2001/08/09 09:14:11 scott Exp $";

#define		MAXDESC		7

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<pr_format3.h>
#include	<ml_std_mess.h>
#include	<ml_mh_mess.h>
#include    <std_decs.h>
	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
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
		char	tes_no[3];
		char	tes_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

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
		{"cumr_bank_code"},
		{"cumr_branch_code"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_disc_code"},
		{"cumr_tax_code"},
	};

	int cumr_no_fields = 17;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_dbt_acronym[10];
		char	cm_acc_type[2];
		char	cm_class_type[4];
		char	cm_price_type[2];
		int	cm_payment_flag;
		char	cm_bank_code[4];
		char	cm_branch_code[21];
		char	cm_area_code[3];
		char	cm_sman_code[3];
		char	cm_disc_code[2];
		char	cm_tax_code[2];
	} cumr_rec;

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
		char	dr_spec_det_1[61];
		char	dr_spec_det_2[61];
		char	dr_order_no[17];
		long	dr_order_date;
		long	dr_sell_date;
		long	dr_hhcu_hash;
		char	dr_cust_type[4];
		char	dr_cust_area[3];
		char	dr_rep_no[3];
		char	dr_inv_no[9];
		double	dr_cost_nzd;		/*  Money field  */
		double	dr_val_nzd;		/*  Money field  */
		char	dr_war_no[7];
		long	dr_war_exp;
		double	dr_war_cost;		/*  Money field  */
		double	dr_ex_war_cost;		/*  Money field  */
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
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
	};

	int inmr_no_fields = 7;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		char	mr_serial_item[2];
	} inmr_rec;

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
		double	sf_com_min;		/*  Money field  */
		char	sf_stat_flag[2];
	} exsf_rec;

	int	rc,
		lpno = 1,
		option;

	char	systemDate[11];
	char	spec_desc[MAXDESC][16];

	long	purge_date;

	FILE	*fout, *fin;

/*======================
| Function Prototypes  |
=======================*/
void OpenDB(void);
void CloseDB(void);
void ReadMisc(void);
void head_output(void);
void get_mccf(void);
void print_mhdr(void);
void get_spec_desc(int, char *);
void find_excl(char *);
void find_exaf(char *);
void find_exsf(char *);

int get_mhdr(void);
int valid_sell_date(void);
int expired_warranty(void);

/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int  argc, 
 char *argv[])
{
	if (argc != 3)
	{
		/*Usage : %s <lpno> <purge_date>,argv[0]*/
		print_at (0,0,ML(mlMhMess016),argv[0]);
		return (argc);
	}

	/*-----------------------
	| Printer Number	|
	-----------------------*/
	lpno = atoi (argv[1]);

	/*------------------------
	| Purge recs cutoff date |
	------------------------*/
	purge_date = StringToDate (argv[2]);

	strcpy (systemDate, DateToString (TodaysDate()));

	init_scr();

	OpenDB ();
	ReadMisc ();

	

	dsp_screen("Processing : Printing Machine History Record To be Purged.", comm_rec.tco_no, comm_rec.tco_name);

	if ((fin = pr_open ("mh_purge.p")) == NULL)
		sys_err("Error in opening mh_purge.p during (FOPEN)",errno,PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	get_mccf ();
	head_output();

	get_mhdr ();

	pr_format (fin,fout,"END_FILE",0,0);
	pclose (fout);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void 
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("mhdr",mhdr_list,mhdr_no_fields,"mhdr_serial_id");
	open_rec ("mccf",mccf_list,mccf_no_fields,"mccf_id_no");
	open_rec ("mhsd",mhsd_list,mhsd_no_fields,"mhsd_id_no");
	open_rec ("inmr",inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec ("cumr",cumr_list,cumr_no_fields,"cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose ("mhdr");
	abc_fclose ("mhsd");
	abc_fclose ("inmr");
	abc_fclose ("cumr");
	abc_dbclose ("data");
}

/*============================================
| Get common info from common database file. |
============================================*/
void 
ReadMisc (
 void)
{
	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");

	strcpy (ccmr_rec.cc_co_no,comm_rec.tco_no);
	strcpy (ccmr_rec.cc_est_no,comm_rec.tes_no);
	strcpy (ccmr_rec.cc_cc_no,comm_rec.tcc_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose ("ccmr");
}

void 
get_mccf (
 void)
{
	int	i = 1;

	strcpy (mccf_rec.cf_co_no,comm_rec.tco_no);
	strcpy (mccf_rec.cf_spec_type,"1");
	cc = find_rec ("mccf",&mccf_rec,GTEQ,"r");
	while ((!cc && !strcmp (mccf_rec.cf_co_no,comm_rec.tco_no)) || i <= MAXDESC)
	{
		sprintf (spec_desc[i - 1],"%-15.15s",mccf_rec.cf_spec_desc);
		i++;
		cc = find_rec ("mccf",&mccf_rec,NEXT,"r");
	}
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void 
head_output (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",lpno);

	fprintf (fout,".5\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L145\n");
	fprintf (fout,".E%s\n",clip(comm_rec.tco_name));
	fprintf (fout,".EMACHINE HISTORY DETAIL\n");
	fprintf (fout,".B1\n");

	fflush (fout);
}

int 
get_mhdr (
 void)
{
	/*-----------------------------------
	| Initialize the mhdr record fields |
	-----------------------------------*/
	strcpy (mhdr_rec.dr_co_no,comm_rec.tco_no);
	mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
	mhdr_rec.dr_hhbr_hash = 0L;
	strcpy (mhdr_rec.dr_serial_no,"                         ");

	cc = find_rec ("mhdr",&mhdr_rec,GTEQ,"u");

	while (!cc && !strcmp(mhdr_rec.dr_co_no,comm_rec.tco_no) && mhdr_rec.dr_hhcc_hash == ccmr_rec.cc_hhcc_hash)
	{
		/*-----------------------------------------------------------
		| Print and then purge records with sold date older than    |	
		| purge_date & with an expired warranty                     |
		-----------------------------------------------------------*/
		if (valid_sell_date() && expired_warranty())
		{
			print_mhdr ();
			abc_unlock ("mhdr");
			cc = abc_delete ("mhdr");
			if (cc)
				sys_err ("Error in mhdr During (DBDELETE)",cc,PNAME);
			cc = find_rec ("mhdr",&mhdr_rec,GTEQ,"u");
		}
		else
			cc = find_rec ("mhdr",&mhdr_rec,NEXT,"r");
	}
	abc_unlock ("mhdr");
	return (EXIT_SUCCESS);
}

int 
valid_sell_date (
 void)
{
	if (mhdr_rec.dr_sell_date < purge_date)
		return (TRUE);
	else
		return (FALSE);
}

int 
expired_warranty (
 void)
{
	if (mhdr_rec.dr_war_exp <= StringToDate(systemDate))
		return (TRUE);
	else
		return (FALSE);
}

void 
print_mhdr (
 void)
{
	int	i;
	char	item_class[2];
	char	categ[12];
	double  total_war_cost;

	cc = find_hash ("inmr",&inmr_rec,COMPARISON,"r",mhdr_rec.dr_hhbr_hash);
	if (cc)
		return;

	cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",mhdr_rec.dr_hhcu_hash);
	if (cc)
		return;

	strcpy (item_class,inmr_rec.mr_class);
	strcpy (categ,inmr_rec.mr_category);

	dsp_process (" Serial No : ",mhdr_rec.dr_serial_no);
 
	find_excl (mhdr_rec.dr_cust_type);
	find_exaf (mhdr_rec.dr_cust_area);
	find_exsf (mhdr_rec.dr_rep_no);

	pr_format (fin,fout,"CHECK_PAGE",0,0);
	pr_format (fin,fout,"HEADING0",0,0);
	pr_format (fin,fout,"RULER",0,0);
	pr_format (fin,fout,"PRODUCT",1,inmr_rec.mr_item_no);
	pr_format (fin,fout,"PRODUCT",2,inmr_rec.mr_description);
	pr_format (fin,fout,"SERIAL_NO",1,mhdr_rec.dr_serial_no);
	pr_format (fin,fout,"MODEL",1,mhdr_rec.dr_model_code);
	pr_format (fin,fout,"MODEL",2,item_class);
	pr_format (fin,fout,"MODEL",3,categ);
	pr_format (fin,fout,"MODEL",4,mhdr_rec.dr_chasis_no);
	pr_format (fin,fout,"DATES",1,mhdr_rec.dr_mfg_pur_date);

/*
	pr_format(fin,fout,"MAST_CODE",1,mhdr_rec.dr_mast_spec);
	get_spec_desc(1,mhdr_rec.dr_mast_spec);
	pr_format(fin,fout,"MAST_CODE",2,mhsd_rec.sd_desc);

	pr_format(fin,fout,"FUEL_SPEC",1,mhdr_rec.dr_fuel_spec);
	get_spec_desc(2,mhdr_rec.dr_fuel_spec);
	pr_format(fin,fout,"FUEL_SPEC",2,mhsd_rec.sd_desc);

	pr_format(fin,fout,"FUEL_SPEC",3,mhdr_rec.dr_att_spec);
	get_spec_desc(3,mhdr_rec.dr_att_spec);
	pr_format(fin,fout,"FUEL_SPEC",4,mhsd_rec.sd_desc);

	pr_format(fin,fout,"TYRE_SPEC",1,mhdr_rec.dr_tyre_spec);
	get_spec_desc(4,mhdr_rec.dr_tyre_spec);
	pr_format(fin,fout,"TYRE_SPEC",2,mhsd_rec.sd_desc);

	pr_format(fin,fout,"TYRE_SPEC",3,mhdr_rec.dr_wheel_spec);
	get_spec_desc(5,mhdr_rec.dr_wheel_spec);
	pr_format(fin,fout,"TYRE_SPEC",4,mhsd_rec.sd_desc);

	pr_format(fin,fout,"AIR_SPEC",1,mhdr_rec.dr_airfilt_spec);
	get_spec_desc(6,mhdr_rec.dr_airfilt_spec);
	pr_format(fin,fout,"AIR_SPEC",2,mhsd_rec.sd_desc);

	pr_format(fin,fout,"AIR_SPEC",3,mhdr_rec.dr_other_spec);
	get_spec_desc(7,mhdr_rec.dr_other_spec);
	pr_format(fin,fout,"AIR_SPEC",4,mhsd_rec.sd_desc);
*/
	for (i = 0;i < MAXDESC; i++)
	{
		if (strcmp (spec_desc [i], "               ") != 0)
		{
			pr_format (fin,fout,"SPEC",1,spec_desc[i]);
			pr_format (fin,fout,"SPEC",2,mhdr_rec.dr_spec[i]);
			get_spec_desc (i+1,mhdr_rec.dr_spec[i]);
			pr_format (fin,fout,"SPEC",3,mhsd_rec.sd_desc);
		}
	}

	pr_format (fin,fout,"SPEC_DET0",1,mhdr_rec.dr_spec_det_1);
	pr_format (fin,fout,"SPEC_DET1",1,mhdr_rec.dr_spec_det_2);
	pr_format (fin,fout,"BLANK0",0,0);
	pr_format (fin,fout,"RULER",0,0);

	pr_format (fin,fout,"HEADING1",0,0);
	pr_format (fin,fout,"SALES_ORD",1,mhdr_rec.dr_order_no);
	pr_format (fin,fout,"SALES_ORD",2,mhdr_rec.dr_order_date);
	pr_format (fin,fout,"SALES_ORD",3,mhdr_rec.dr_sell_date);
	pr_format (fin,fout,"CUSTOMER",1,cumr_rec.cm_dbt_no);
	pr_format (fin,fout,"CUSTOMER",2,cumr_rec.cm_dbt_name);
	pr_format (fin,fout,"CUSTOMER",3,mhdr_rec.dr_cust_type);
	pr_format (fin,fout,"CUSTOMER",4,excl_rec.cl_class_description);
	pr_format (fin,fout,"SALES_AREA",1,mhdr_rec.dr_cust_area);
	pr_format (fin,fout,"SALES_AREA",2,exaf_rec.af_area);
	pr_format (fin,fout,"SALES_AREA",3,mhdr_rec.dr_rep_no);
	pr_format (fin,fout,"SALES_AREA",4,exsf_rec.sf_salesman);
	pr_format (fin,fout,"INVOICE",1,mhdr_rec.dr_inv_no);
	pr_format (fin,fout,"INVOICE",2,mhdr_rec.dr_cost_nzd);
	pr_format (fin,fout,"INVOICE",3,mhdr_rec.dr_val_nzd);
	pr_format (fin,fout,"BLANK1",0,0);
	pr_format (fin,fout,"RULER",0,0);

	pr_format (fin,fout,"WARRANTY0",0,0);
	pr_format (fin,fout,"WARRANTY1",1,mhdr_rec.dr_war_no);
	pr_format (fin,fout,"WARRANTY1",2,mhdr_rec.dr_war_exp);
	pr_format (fin,fout,"WARRANTY1",3,mhdr_rec.dr_last_serv_date);
	pr_format (fin,fout,"WARRANTY2",1,mhdr_rec.dr_war_cost);
	pr_format (fin,fout,"WARRANTY2",2,mhdr_rec.dr_ex_war_cost);

	total_war_cost = mhdr_rec.dr_war_cost + mhdr_rec.dr_ex_war_cost;
	pr_format (fin,fout,"WARRANTY2",3,total_war_cost);
	pr_format (fin,fout,"RULER",0,0);

	fflush (fout);
}

void 
get_spec_desc (
 int	spec, 
 char	*mhdr_code)
{
	strcpy (mhsd_rec.sd_co_no,comm_rec.tco_no);
	sprintf (mhsd_rec.sd_spec_type,"%d",spec);
	sprintf (mhsd_rec.sd_code,"%-4.4s",mhdr_code);
	cc = find_rec ("mhsd",&mhsd_rec,COMPARISON,"r");
	if (cc)
		sprintf (mhsd_rec.sd_desc,"%-40.40s"," ");
}

/*===============================
| Find the class description.	|
===============================*/
void 
find_excl (
 char *class_type)
{
	open_rec ("excl",excl_list,excl_no_fields,"excl_id_no");
	strcpy (excl_rec.cl_co_no,comm_rec.tco_no);
	strcpy (excl_rec.cl_class_type,class_type);
	cc = find_rec ("excl",&excl_rec,COMPARISON,"r");
	if (cc)
	      strcpy (excl_rec.cl_class_description,"No Class description found.");
	abc_fclose ("excl");
}

/*===============================
| Find the area description.	|
===============================*/
void 
find_exaf (
 char *area_code)
{
	open_rec ("exaf",exaf_list,exaf_no_fields,"exaf_id_no");
	strcpy (exaf_rec.af_co_no,comm_rec.tco_no);
	strcpy (exaf_rec.af_area_code,area_code);
	cc = find_rec ("exaf",&exaf_rec,COMPARISON,"r");
	if (cc)
	      strcpy (exaf_rec.af_area,"No Area description found.");
	abc_fclose ("exaf");
}

/*===============================
| Find the salesman's name.	|
===============================*/
void 
find_exsf (
 char *sman_code)
{
	open_rec ("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
	strcpy (exsf_rec.sf_co_no,comm_rec.tco_no);
	strcpy (exsf_rec.sf_salesman_no,sman_code);
	cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
	      strcpy (exsf_rec.sf_salesman,"No salesman name found.");
	abc_fclose ("exsf");
}

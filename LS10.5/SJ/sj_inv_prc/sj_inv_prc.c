/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_inv_prc.c   )                                 |
|  Program Desc  : ( Create & Price Service Invoices              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjis, sjhr, cumr, sjjd, sjld, sjsp,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 08/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (09/04/87)      | Modified  by  : Lance Whitford   |
|  Date Modified : (05/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (22/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (30/10/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (29/12/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (16/10/1997)    | Modified  by  : Jiggs A Veloz.   |
|  Date Modified : (02/09/1999)    | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|                :                                                    |
|     (22/11/89) : Fix bug in reading sjhr.                           |
|                :                                                    |
|     (30/10/92) : Fix date on invoice creation. SC 8034 PSL.         |
|     (29/12/92) : removal of esmr_tavern.  PSL 8312                  |
|     (16/09/97) : Convert date in fullYear()                         |
|  (16/10/1997)	 : SEL - Changed inv_no from char6 to char8.		  |
|                :                                                    |
|                                                                     |
| $Log: sj_inv_prc.c,v $
| Revision 5.2  2001/08/09 09:17:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:51  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/09/29 10:12:57  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 05:06:33  scott
| Updated from Ansi
|
| Revision 1.8  1999/09/14 01:40:39  marlyn
| Ported to ANSI standards.
|
| Revision 1.7  1999/06/20 02:30:28  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_inv_prc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_inv_prc/sj_inv_prc.c,v 5.2 2001/08/09 09:17:28 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dbt_date"},
		{"comm_gst_rate"},
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
		long	tdbt_date;
		float	tgst_rate;
	} comm_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
		{"esmr_adr1"},
		{"esmr_adr2"},
		{"esmr_adr3"},
		{"esmr_nx_inv_no"},
		{"esmr_nx_crd_nte_no"},
		{"esmr_nx_pur_ord_no"},
		{"esmr_nx_del_dck_no"},
		{"esmr_nx_cheq_no"},
		{"esmr_ls_cheq_no"},
		{"esmr_nx_job_no"},
		{"esmr_stat_flag"}
	};

	int esmr_no_fields = 15;

	struct {
		char	es_co_no[3];
		char	es_est_no[3];
		char	es_est_name[41];
		char	es_short_name[16];
		char	es_adr1[41];
		char	es_adr2[41];
		char	es_adr3[41];
		long	es_nx_inv_no;
		long	es_nx_crd_nte_no;
		long	es_nx_pur_ord_no;
		long	es_nx_del_dck_no;
		long	es_nx_cheq_no;
		long	es_ls_cheq_no;
		long	es_nx_job_no;
		char	es_stat_flag[2];
	} esmr_rec;

	/*=================================
	| Service Job Header Record File. |
	=================================*/
	struct dbview sjhr_list[] ={
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_status"},
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_cust_ord_no"},
		{"sjhr_cost_estim"},
		{"sjhr_estim_type"},
		{"sjhr_prebill_amt"},
		{"sjhr_fixed_labour"},
		{"sjhr_oh_cost"},
		{"sjhr_lb_cost"},
		{"sjhr_lb_chg"},
		{"sjhr_lb_hrs"},
		{"sjhr_mt_cost"},
		{"sjhr_mt_chg"},
		{"sjhr_km"},
		{"sjhr_km_chg"},
	};

	int sjhr_no_fields = 20;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_ord_no[11];
		double	hr_cost_estim;
		char	hr_estim_type[2];
		double	hr_prebill_amt;
		double	hr_fixed_labour;
		double	hr_oh_cost;
		double	hr_lb_cost;
		double	hr_lb_chg;
		float	hr_lb_hrs;
		double	hr_mt_cost;
		double	hr_mt_chg;
		float	hr_km;	
		double	hr_km_chg;
	} sjhr_rec;

	/*====================================
	| Service Job Invoice Summary  File. |
	====================================*/
	struct dbview sjis_list[] ={
		{"sjis_co_no"},
		{"sjis_est_no"},
		{"sjis_dp_no"},
		{"sjis_invno"},
		{"sjis_order_no"},
		{"sjis_date"},
		{"sjis_chg_client"},
		{"sjis_end_client"},
		{"sjis_cust_ord_no"},
		{"sjis_cost_estim"},
		{"sjis_invoice_cost"},
		{"sjis_invoice_chg"},
		{"sjis_prebill_amt"},
		{"sjis_status"},
		{"sjis_type"},
		{"sjis_prt_stat"}
	};

	int sjis_no_fields = 16;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
		long	is_chg_client;
		long	is_end_client;
		char	is_cust_ord_no[11];
		double	is_cost_estim;
		double	is_invoice_cost;
		double	is_invoice_chg;
		double	is_prebill_amt;
		char	is_status[2];
		char	is_type[2];
		char	is_prt_stat[2];
	} ajis_rec, sjis_rec;

	/*======================
	| Labour details file  |
	======================*/
	struct dbview sjld_list[] ={
		{"sjld_co_no"},
		{"sjld_est_no"},
		{"sjld_dp_no"},
		{"sjld_order_no"},
		{"sjld_emp_code"},
		{"sjld_date"},
		{"sjld_km"},
		{"sjld_km_rate"},
		{"sjld_time"},
		{"sjld_time_half"},
		{"sjld_time_double"},
		{"sjld_tm_rate"},
		{"sjld_oh_rate"},
		{"sjld_pr_rate"}
	};

	int sjld_no_fields = 14;

	struct {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
		float	ld_km;
		double	ld_km_rate;
		float	ld_time;
		float	ld_time_half;
		float	ld_time_double;
		double	ld_tm_rate;
		double	ld_oh_rate;
		double	ld_pr_rate;
	} sjld_rec;

	/*=============================
	| Spare parts/cost items used |
	==============================*/
	struct dbview sjsp_list[] ={
		{"sjsp_co_no"},
		{"sjsp_est_no"},
		{"sjsp_dp_no"},
		{"sjsp_order_no"},
		{"sjsp_partno"},
		{"sjsp_qty"},
		{"sjsp_u_cost"},
		{"sjsp_u_sell"},
	};

	int sjsp_no_fields = 8;

	struct {
		char	sp_co_no[3];
		char	sp_est_no[3];
		char	sp_dp_no[3];
		long	sp_order_no;
		char	sp_partno[17];
		float	sp_qty;
		double	sp_u_cost;
		double	sp_u_sell;
	} sjsp_rec;

	/*=============================
	| Service Disbursements file  |
	=============================*/
	struct dbview sjdc_list[] ={
		{"sjdc_co_no"},
		{"sjdc_est_no"},
		{"sjdc_dp_no"},
		{"sjdc_order_no"},
		{"sjdc_po_no"},
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"}
	};

	int sjdc_no_fields = 8;

	struct {
		char	dc_co_no[3];
		char	dc_est_no[3];
		char	dc_dp_no[3];
		long	dc_order_no;
		char	dc_po_no[9];
		double	dc_est_cost;
		double	dc_act_cost;
		double	dc_chg_cost;
	} sjdc_rec;

	long	lsystemDate;

	char	ord_number[9];	

	double  mt_cost_tot = 0.00,
			mt_chg_tot 	= 0.00,
			lb_cost_tot = 0.00,
			lb_chg_tot 	= 0.00,
			km_chg_tot 	= 0.00,
			oh_cost_tot = 0.00,
			inv_tot 	= 0.00,
			cost_tot 	= 0.00;

	double	extend(float qty, double rate);

	float	lb_hrs_tot 	= 0.0,
			km_tot 		= 0.0;

/*=======================
| Function Prototypes   |
=======================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void proc_file (void);
void proc_sp (void);
void proc_ld (void);
void proc_dc (void);
void create_sjis (void);
void end_proc (void);
int findsjhr (int stype);
int check_sjis (long inv_no);
double extend (float qty, double rate);

/*=========================
| Main Processing Routine |
==========================*/
int
main (
 int argc,
 char *argv[])
{
	char 	systemDate[11];

	dsp_screen ("Pricing Service Invoices ",comm_rec.tco_no,comm_rec.tco_name);

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();

	OpenDB ();
	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );

	proc_file ();

	end_proc ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("esmr", esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec ("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec ("sjld", sjld_list, sjld_no_fields, "sjld_id_no");
	open_rec ("sjdc", sjdc_list, sjdc_no_fields, "sjdc_id_no");
	open_rec ("sjsp", sjsp_list, sjsp_no_fields, "sjsp_id_no");

	abc_alias ("ajis","sjis");
	open_rec ("sjis", sjis_list, sjis_no_fields, "sjis_id_no");
	open_rec ("ajis", sjis_list, sjis_no_fields, "sjis_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("esmr");
	abc_fclose ("sjhr");
	abc_fclose ("sjld");
	abc_fclose ("sjdc");
	abc_fclose ("sjsp");
	abc_fclose ("sjis");
	abc_fclose ("ajis");
	abc_dbclose ("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*===========================
| Validate and print lines. |
===========================*/
void
proc_file (
 void)
{
	int loop_flag = 1;
	int find_type = GTEQ;

	strcpy (sjhr_rec.hr_co_no,  comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no, comm_rec.tes_no);
	strcpy (sjhr_rec.hr_dp_no,  comm_rec.tdp_no);
	strcpy (sjhr_rec.hr_status, "G");
	sjhr_rec.hr_order_no = 0L;

	while (loop_flag) 
	{
		mt_cost_tot = 0.00;
		mt_chg_tot  = 0.00;
		lb_cost_tot = 0.00;
		lb_hrs_tot  = 0.00;
		lb_chg_tot  = 0.00;
		km_tot 	    = 0.00;
		km_chg_tot  = 0.00;
		oh_cost_tot = 0.00;

		cc = findsjhr (find_type);
		if (cc)
		{
			loop_flag = 0;
			continue;
		}
		find_type = NEXT;

		/*-------------------------------------------
		| Check if order belongs to correct company |
		-------------------------------------------*/
		if (strcmp (sjhr_rec.hr_co_no,comm_rec.tco_no))
		{
			abc_unlock ("sjhr");
			loop_flag = 0;
			continue;
		}

	    /*------------------------------------------------- 
		| Check if order belongs to correct branchNolishment |
	    -------------------------------------------------*/ 
		if (strcmp (sjhr_rec.hr_est_no,comm_rec.tes_no))
		{
			abc_unlock ("sjhr");
			loop_flag = 0;
			continue;
		}

	    /*------------------------------------------------- 
		| Check if order belongs to correct department.   |
	    -------------------------------------------------*/ 
		if (strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
		{
			abc_unlock ("sjhr");
			loop_flag = 0;
			continue;
		}

		/*-----------------------------------
		| Check if order has correct status	|
		------------------------------------*/
		if (sjhr_rec.hr_status[0] != 'G')
		{
			abc_unlock ("sjhr");
			continue;
		}

		if (sjhr_rec.hr_status[0] == 'G')
		{
			sprintf (ord_number,"%8ld",sjhr_rec.hr_order_no);
			dsp_process ("Order No : ",ord_number);
			
			proc_sp ();

			proc_ld ();

			proc_dc ();

			if (sjhr_rec.hr_estim_type[0] == 'F')
				inv_tot = sjhr_rec.hr_cost_estim;
			else
			{
				if (sjhr_rec.hr_fixed_labour > 0.00)
					inv_tot = sjhr_rec.hr_fixed_labour;
				else
					inv_tot = lb_cost_tot
						+ lb_chg_tot
						+ oh_cost_tot
						+ km_chg_tot;

				inv_tot += mt_chg_tot;
			}
			cost_tot = lb_cost_tot + oh_cost_tot + km_chg_tot + mt_cost_tot;

			create_sjis ();

			strcpy (sjhr_rec.hr_status,"I");

			cc = abc_update ("sjhr", &sjhr_rec);
			if (cc)
				sys_err ("Error in sjhr During (DBUPDATE)",cc, PNAME);
		}
		abc_unlock ("sjhr");
	}
}

int
findsjhr (
 int stype)
{
	cc = find_rec ("sjhr",&sjhr_rec,stype,"r");
	return (cc);
}

/*===================================
| Process Materials/service charges |
| from sjsp file.                   |
===================================*/
void
proc_sp (
 void)
{
	strcpy (sjsp_rec.sp_co_no,  comm_rec.tco_no);
	strcpy (sjsp_rec.sp_est_no, comm_rec.tes_no);
	strcpy (sjsp_rec.sp_dp_no,  comm_rec.tdp_no);
	sjsp_rec.sp_order_no = sjhr_rec.hr_order_no;
	sprintf (sjsp_rec.sp_partno, "%-16.16s"," ");
	cc = find_rec ("sjsp",&sjsp_rec,GTEQ,"r");

	while (!cc && !strcmp (sjsp_rec.sp_co_no,comm_rec.tco_no) && 
		   !strcmp (sjsp_rec.sp_est_no,comm_rec.tes_no) && 
		   !strcmp (sjsp_rec.sp_dp_no,comm_rec.tdp_no) && 
		   sjsp_rec.sp_order_no == sjhr_rec.hr_order_no)
	{
		mt_cost_tot += extend (sjsp_rec.sp_qty,sjsp_rec.sp_u_cost);
		mt_chg_tot  += extend (sjsp_rec.sp_qty,sjsp_rec.sp_u_sell);

		cc = find_rec ("sjsp",&sjsp_rec,NEXT,"r");
	}
	sjhr_rec.hr_mt_cost = mt_cost_tot;
	sjhr_rec.hr_mt_chg  = mt_chg_tot;
}


/*=================================
| Process Labour and Km  charges  |
| from sjld (labour details) file |
=================================*/
void
proc_ld (
 void)
{
	float 	hrs = 0.0;

	strcpy (sjld_rec.ld_co_no,  comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no, comm_rec.tes_no);
	strcpy (sjld_rec.ld_dp_no, comm_rec.tdp_no);
	sjld_rec.ld_order_no = sjhr_rec.hr_order_no;
	sjld_rec.ld_date 	 = 0L;
	cc = find_rec ("sjld",&sjld_rec,GTEQ,"r");

	while (!cc && !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) && 
			!strcmp (sjld_rec.ld_est_no,comm_rec.tes_no) && 
			!strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no) && 
			sjld_rec.ld_order_no == sjhr_rec.hr_order_no)
	{
		hrs 	=   sjld_rec.ld_time
			+ ( float ) (sjld_rec.ld_time_half * 1.5)
			+ (float ) (sjld_rec.ld_time_double * 2.0);
			
		lb_hrs_tot  += hrs;
		lb_cost_tot += extend (hrs,sjld_rec.ld_tm_rate);
		lb_chg_tot  += extend (hrs,sjld_rec.ld_pr_rate);

		oh_cost_tot += extend (hrs,sjld_rec.ld_oh_rate);
		km_tot      += sjld_rec.ld_km;
		km_chg_tot  += extend (sjld_rec.ld_km,sjld_rec.ld_km_rate);

		cc = find_rec ("sjld",&sjld_rec,NEXT,"r");
	}
	sjhr_rec.hr_lb_cost = lb_cost_tot;
	sjhr_rec.hr_lb_chg  = lb_chg_tot;
	sjhr_rec.hr_lb_hrs  = lb_hrs_tot;
	sjhr_rec.hr_oh_cost = oh_cost_tot;
	sjhr_rec.hr_km      = km_tot;
	sjhr_rec.hr_km_chg  = km_chg_tot;
}

/*===========================
| Process Outside purchases |
| from sjdc file            |
===========================*/
void
proc_dc (
 void)
{
	strcpy (sjdc_rec.dc_co_no,  comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no, comm_rec.tes_no);
	strcpy (sjdc_rec.dc_dp_no, comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf (sjdc_rec.dc_po_no,"%-8.8s"," ");
	cc = find_rec ("sjdc",&sjdc_rec,GTEQ,"r");

	while (!cc && !strcmp (sjdc_rec.dc_co_no,comm_rec.tco_no) && 
			!strcmp (sjdc_rec.dc_est_no,comm_rec.tes_no) && 
			!strcmp (sjdc_rec.dc_dp_no,comm_rec.tdp_no) && 
			sjdc_rec.dc_order_no == sjhr_rec.hr_order_no)
	{
		if (sjdc_rec.dc_act_cost != 0)
			mt_cost_tot += sjdc_rec.dc_act_cost;
		else
			mt_cost_tot += sjdc_rec.dc_est_cost;
		mt_chg_tot += sjdc_rec.dc_chg_cost;

		cc = find_rec ("sjdc",&sjdc_rec,NEXT,"r");
	}
	sjhr_rec.hr_mt_cost = mt_cost_tot; 
	sjhr_rec.hr_mt_chg  = mt_chg_tot; 
}

void
create_sjis (
 void)
{
	/*==========================
    |   update invoice summary |
	==========================*/
	strcpy (sjis_rec.is_co_no,  comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no, comm_rec.tes_no);
	strcpy (sjis_rec.is_dp_no,  comm_rec.tdp_no);

	sjis_rec.is_order_no 	= sjhr_rec.hr_order_no;
	sjis_rec.is_date 		= lsystemDate;
	sjis_rec.is_chg_client 	= sjhr_rec.hr_chg_client;
	sjis_rec.is_end_client 	= sjhr_rec.hr_end_client;

	strcpy (sjis_rec.is_cust_ord_no,sjhr_rec.hr_cust_ord_no);
	strcpy (sjis_rec.is_type,	 "I");
	strcpy (sjis_rec.is_status,	 "G");
	strcpy (sjis_rec.is_prt_stat,"G");

	sjis_rec.is_cost_estim 	 = sjhr_rec.hr_cost_estim;
	sjis_rec.is_invoice_cost = cost_tot;
 	sjis_rec.is_invoice_chg  = inv_tot - sjhr_rec.hr_prebill_amt;
	sjis_rec.is_prebill_amt  = sjhr_rec.hr_prebill_amt ;

	/*=========================
	|   assign invoice number |
	==========================*/
	strcpy (esmr_rec.es_co_no,  comm_rec.tco_no);
	strcpy (esmr_rec.es_est_no, comm_rec.tes_no);
	cc = find_rec ("esmr", &esmr_rec, COMPARISON, "u");
	if (cc)
		sys_err ("Error in esmr during (DBFIND)",cc,PNAME);

	/*---------------------------------------
	| Check if Inv.  No Already Allocated	|
	| If it has been then skip				|
	---------------------------------------*/
	while (check_sjis (++esmr_rec.es_nx_inv_no) == 0);

	cc = abc_update ("esmr",&esmr_rec);
	if (cc)
		sys_err ("Error in esmr during (DBUPDATE)",cc,PNAME);

	sprintf (sjis_rec.is_invno,"%08ld",esmr_rec.es_nx_inv_no);

	cc = abc_add ("sjis",&sjis_rec);
	if (cc)
		sys_err ("Error in sjis During (DBADD)",cc,PNAME);

}

double 
extend (
 float qty,
 double rate)
{
	    
	double	extn = 0.00;
	
	extn = (double) qty;
	extn *= rate;
	return (extn);
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_proc (
 void)
{
}

int
check_sjis (
long inv_no)
{
	strcpy (ajis_rec.is_co_no,  comm_rec.tco_no);
	strcpy (ajis_rec.is_est_no, comm_rec.tes_no);
	sprintf (ajis_rec.is_invno,  "%08ld", inv_no);
	cc = find_rec ("ajis",&ajis_rec,COMPARISON,"r");

	return (cc);
}

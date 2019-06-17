/*====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_invprt.c                                      |
|  Program Desc  : ( Print Service Invoices                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr, sjjd, sjsp, sjdc, sjis,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 05/10/87         |
|---------------------------------------------------------------------|
|  Date Modified : 29/11/88        | Modified By   : B.C.Lim.	      |
|  Date Modified : (18/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (29/10/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|                : (18/09/90) - General Update for New Scrgen. S.B.D. |
|                :                                                    |
|  (29/10/92)    : Change Unit Cost to Unit Price. Fix totals.        |
|                : SC 8034 PSL                                        |
|  (16/09/97)    : Updated to incorporate multilingual conversion.    |
|                :                                                    |
| $Log: sj_invprt.c,v $
| Revision 5.2  2001/08/09 09:17:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:31  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:51  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:34:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 05:58:32  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.11  1999/10/14 00:55:17  cam
| Remove cc
|
| Revision 1.10  1999/09/29 10:12:57  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 05:06:33  scott
| Updated from Ansi
|
| Revision 1.8  1999/09/14 01:41:21  marlyn
| Ported to ANSI standards.
|
| Revision 1.7  1999/06/20 02:30:29  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_invprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_invprt/sj_invprt.c,v 5.2 2001/08/09 09:17:29 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*=================================
	| Service Job Header Record File. |
	=================================*/
	struct dbview sjhr_list[] ={
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_cust_ord_no"},
		{"sjhr_estim_type"},
		{"sjhr_lb_chg"},
		{"sjhr_lb_hrs"},
		{"sjhr_km"},
		{"sjhr_km_chg"},
	};

	int sjhr_no_fields = 12;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_ord_no[11];
		char	hr_estim_type[2];
		double	hr_lb_chg;
		float	hr_lb_hrs;
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
		{"sjis_invoice_chg"},
		{"sjis_prebill_amt"},
		{"sjis_gst_pc"},
	};

	int sjis_no_fields = 9;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
		double	is_invoice_chg;
		double	is_prebill_amt;
		float	is_gst_pc;
	} sjis_rec;

	/*==========================
	| Service Job detail file  |
	==========================*/
	struct dbview sjjd_list[] ={
		{"sjjd_co_no"},
		{"sjjd_est_no"},
		{"sjjd_dp_no"},
		{"sjjd_order_no"},
		{"sjjd_line_no"},
		{"sjjd_detail"}
	};

	int sjjd_no_fields = 6;

	struct {
		char	jd_co_no[3];
		char	jd_est_no[3];
		char	jd_dp_no[3];
		long	jd_order_no;
		int	jd_line_no;
		char	jd_detail[71];
	} sjjd_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_class_type"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
	};

	int cumr_no_fields = 9;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_class_type[4];
		char	cm_ch_adr1[41];
		char	cm_ch_adr2[41];
		char	cm_ch_adr3[41];
	} cumr_rec;

	/*======================
	|  Service detail file |
	======================*/
	struct dbview sjsd_list[] ={
		{"sjsd_co_no"},
		{"sjsd_est_no"},
		{"sjsd_dp_no"},
		{"sjsd_order_no"},
		{"sjsd_line_no"},
		{"sjsd_detail"}
	};

	int sjsd_no_fields = 6;

	struct {
		char	sd_co_no[3];
		char	sd_est_no[3];
		char	sd_dp_no[3];
		long	sd_order_no;
		int	sd_line_no;
		char	sd_detail[71];
	} sjsd_rec;

	/*====================
	| Spare parts usage  |
	====================*/
	struct dbview sjsp_list[] ={
		{"sjsp_co_no"},
		{"sjsp_est_no"},
		{"sjsp_dp_no"},
		{"sjsp_order_no"},
		{"sjsp_partno"},
		{"sjsp_part_desc"},
		{"sjsp_qty"},
		{"sjsp_uom"},
		{"sjsp_u_cost"},
		{"sjsp_u_sell"},
	};

	int sjsp_no_fields = 10;

	struct {
		char	sp_co_no[3];
		char	sp_est_no[3];
		char	sp_dp_no[3];
		long	sp_order_no;
		char	sp_partno[17];
		char	sp_part_desc[41];
		float	sp_qty;
		char	sp_uom[4];
		double	sp_u_cost;
		double	sp_u_sell;
	} sjsp_rec;

	/*===================
	| Outside purchases |
	===================*/
	struct dbview sjdc_list[] ={
		{"sjdc_co_no"},
		{"sjdc_est_no"},
		{"sjdc_dp_no"},
		{"sjdc_order_no"},
		{"sjdc_po_no"},
		{"sjdc_chg_cost"}
	};

	int sjdc_no_fields = 6;

	struct {
		char	dc_co_no[3];
		char	dc_est_no[3];
		char	dc_dp_no[3];
		long	dc_order_no;
		char	dc_po_no[9];
		double	dc_chg_cost;
	} sjdc_rec;

	int	lp_no;

	double	gst_amt = 0.00,
		inv_total = 0.00;

	FILE	*popen(const char *, const char *),
		*fopen(const char *, const char *),
		*fin, 
		*fout;

/*=======================
| Function Prototypes   |
=======================*/
void OpenDB (void);
void CloseDB (void);
void head_output (void);
void proc_sjis (void);
void proc_sjhr (void);
void proc_sjjd (void);
void proc_sjsd (void);
void proc_sjsp (void);
void print_detail (void);
void proc_sjdc (void);

/*=========================
| Main Processing Routine |
==========================*/
int
main (
 int argc, 
 char *argv[])
{
	if (argc != 3)
	{
		print_at (0,0,mlSjMess706,argv[0]);
		return (EXIT_FAILURE);
	}

	lp_no = atoi(argv[1]);

	sprintf (sjis_rec.is_invno,"%-8.8s",argv[2]);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	OpenDB ();
	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );

	if ((fin = pr_open ("sj_invprt.p")) == NULL)
		sys_err ("Error in opening sj_invprt.p during (FOPEN)",errno,PNAME);

	head_output ();  /* initialise report            */
	proc_sjis (); 	/* get invoice            	*/
	proc_sjhr (); 	/* print job header 		*/
	proc_sjjd (); 	/* print job details		*/
	proc_sjsd ();	/* print service details        */
	proc_sjsp ();	/* print spare parts/charges    */

	print_detail ();

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
	open_rec ("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec ("sjjd", sjjd_list, sjjd_no_fields, "sjjd_id_no");
	open_rec ("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec ("sjsd", sjsd_list, sjsd_no_fields, "sjsd_id_no");
	open_rec ("sjsp", sjsp_list, sjsp_no_fields, "sjsp_id_no");
	open_rec ("sjdc", sjdc_list, sjdc_no_fields, "sjdc_id_no");
	open_rec ("sjis", sjis_list, sjis_no_fields, "sjis_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjhr");
	abc_fclose ("cumr");
	abc_fclose ("sjjd");
	abc_fclose ("sjsd");
	abc_fclose ("sjdc");
	abc_fclose ("sjsp");
	abc_fclose ("sjis");
	abc_dbclose ("data");
}

void
head_output (
 void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",lp_no);
	fprintf (fout,".OP\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".6\n");
	fprintf (fout,".L132\n");
	fprintf (fout,".ECOMPANY %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf (fout,"\n");
	fprintf (fout,".ETAX INVOICE\n");
	fprintf (fout,"\n");
}

void
proc_sjis (
 void)
{
	strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no,comm_rec.tes_no);
	strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);

	cc = find_rec ("sjis",&sjis_rec,COMPARISON,"r");
	if (cc)
	{
	 	sprintf (err_str,"Invoice %s is not on file.",sjis_rec.is_invno);
		sys_err (err_str,cc,PNAME);
	}
}

void
proc_sjhr (
 void)
{
	char	chg_dbt[8];
	char	chg_name[41];
	char	chg_adr[3][41];

	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.tes_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = sjis_rec.is_order_no;

	cc = find_rec ("sjhr",&sjhr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in sjhr During (DBFIND)", cc, PNAME);

	cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_chg_client);
	if (cc)
	{
	 	strcpy (chg_dbt,"Unknown");
	 	sprintf (chg_name,"%-40.40s","Unknown Customer");
	 	sprintf (chg_adr[0],"%-40.40s"," ");
	 	sprintf (chg_adr[1],"%-40.40s"," ");
	 	sprintf (chg_adr[2],"%-40.40s"," ");
	}
	else
	{
	 	sprintf (chg_dbt,"%-6.6s ",cumr_rec.cm_dbt_no);
	 	strcpy (chg_name,cumr_rec.cm_dbt_name);
	 	strcpy (chg_adr[0],cumr_rec.cm_ch_adr1);
	 	strcpy (chg_adr[1],cumr_rec.cm_ch_adr2);
	 	strcpy (chg_adr[2],cumr_rec.cm_ch_adr3);
	}

	/*================
	|  Print heading |
	================*/
	pr_format (fin,fout,"NAME",1,chg_name);
	pr_format (fin,fout,"NAME",2,DateToString(sjis_rec.is_date));

	pr_format (fin,fout,"ADR1",1,chg_adr[0]);
	pr_format (fin,fout,"ADR1",2,sjis_rec.is_invno);

	pr_format (fin,fout,"ADR2",1,chg_adr[1]);
	pr_format (fin,fout,"ADR2",2,sjhr_rec.hr_cust_ord_no);

	pr_format (fin,fout,"ADR3",1,chg_adr[2]);
	pr_format (fin,fout,"ADR3",2,sjhr_rec.hr_order_no);

	pr_format (fin,fout,"CLIENT_NO",1,chg_dbt);

	pr_format (fin,fout,"SKIP",1,1);

	if (!cc && strcmp (cumr_rec.cm_class_type,"INT") == 0 )
	{
		cumr_rec.cm_hhcu_hash = sjhr_rec.hr_end_client;
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (chg_dbt,"Unknown");
			sprintf (chg_name,"%-40.40s","Unknown Customer");
			sprintf (chg_adr[0],"%-40.40s"," ");
			sprintf (chg_adr[1],"%-40.40s"," ");
			sprintf (chg_adr[2],"%-40.40s"," ");
		}
		else
		{
			sprintf (chg_dbt,"%-6.6s ",cumr_rec.cm_dbt_no);
			strcpy (chg_name,cumr_rec.cm_dbt_name);
			strcpy (chg_adr[0],cumr_rec.cm_ch_adr1);
			strcpy (chg_adr[1],cumr_rec.cm_ch_adr2);
			strcpy (chg_adr[2],cumr_rec.cm_ch_adr3);
		}

		pr_format (fin,fout,"CUST_NO",1,chg_dbt);
		pr_format (fin,fout,"CUST_NO",2,chg_name);

		pr_format (fin,fout,"CUST_ADDR",1,chg_adr[0]);

		pr_format (fin,fout,"CUST_ADDR",1,chg_adr[1]);

		pr_format (fin,fout,"CUST_ADDR",1,chg_adr[2]);

		pr_format (fin,fout,"SKIP",1,1);
	}
	pr_format (fin,fout,"ULINE",0,0);
}

void
proc_sjjd (
 void)
{
	/*===================
	|  read job details |
	===================*/
	strcpy (sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy (sjjd_rec.jd_est_no,comm_rec.tes_no);
	strcpy (sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;
	cc = find_rec ("sjjd",&sjjd_rec,GTEQ,"r");

	pr_format (fin,fout,"JOB_TITLE",0,0);

	while (!cc && !strcmp (sjjd_rec.jd_co_no,comm_rec.tco_no) && !strcmp (sjjd_rec.jd_est_no,comm_rec.tes_no) && !strcmp(sjjd_rec.jd_dp_no,comm_rec.tdp_no) && sjjd_rec.jd_order_no == sjhr_rec.hr_order_no)
	{
		/*=======================
		| get description lines |
		========================*/
		pr_format (fin,fout,"JOB_LINE",1,sjjd_rec.jd_detail);

		cc = find_rec ("sjjd",&sjjd_rec,NEXT,"r");
	}
	
	pr_format (fin,fout,"SPACES",0,0);
	pr_format (fin,fout,"ULINE",0,0);
}

void
proc_sjsd (
 void)
{
	/*=======================
	|  read service details |
	=======================*/
	strcpy (sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy (sjsd_rec.sd_est_no,comm_rec.tes_no);
	strcpy (sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = 0;
	cc = find_rec ("sjsd",&sjsd_rec,GTEQ,"r");

	pr_format (fin,fout,"JOB_DET",0,0);

	while (!cc && !strcmp (sjsd_rec.sd_co_no,comm_rec.tco_no) && !strcmp (sjsd_rec.sd_est_no,comm_rec.tes_no) && !strcmp(sjsd_rec.sd_dp_no,comm_rec.tdp_no) && sjsd_rec.sd_order_no == sjhr_rec.hr_order_no)
	{
		/*=======================
		| get description lines |
		========================*/
		pr_format (fin,fout,"JOB_DETAIL",1,sjsd_rec.sd_detail);

		cc = find_rec ("sjsd",&sjsd_rec,NEXT,"r");
	}
	pr_format (fin,fout,"SPACES",0,0);
	pr_format (fin,fout,"ULINE",0,0);
}

void
proc_sjsp (
 void)
{
	double	lin_cst = 0.00,
		lin_chg = 0.00;

	/*==============================
	| Print nothing if fixed price |
	==============================*/
	if (sjhr_rec.hr_estim_type[0] == 'F')
		return;

	/*===========================
	|  read Spare parts details |
	===========================*/
	pr_format (fin,fout,"HEADSP1",0,0);
	pr_format (fin,fout,"HEADSP2",0,0);
	pr_format (fin,fout,"SEPARATOR",0,0);
	
	strcpy (sjsp_rec.sp_co_no,comm_rec.tco_no);
	strcpy (sjsp_rec.sp_est_no,comm_rec.tes_no);
	strcpy (sjsp_rec.sp_dp_no,comm_rec.tdp_no);
	sjsp_rec.sp_order_no = sjhr_rec.hr_order_no;
	sprintf (sjsp_rec.sp_partno,"%-16.16s"," ");
	cc = find_rec ("sjsp",&sjsp_rec,GTEQ,"r");

	while (!cc && !strcmp (sjsp_rec.sp_co_no,comm_rec.tco_no) && !strcmp (sjsp_rec.sp_est_no,comm_rec.tes_no) && !strcmp(sjsp_rec.sp_dp_no,comm_rec.tdp_no) && sjsp_rec.sp_order_no == sjhr_rec.hr_order_no)
	{
		lin_cst = (double) sjsp_rec.sp_qty * sjsp_rec.sp_u_cost;
		lin_chg = (double) sjsp_rec.sp_qty * sjsp_rec.sp_u_sell;

	  	pr_format (fin,fout,"PART",1,sjsp_rec.sp_partno);
	  	pr_format (fin,fout,"PART",2,sjsp_rec.sp_part_desc);
	  	pr_format (fin,fout,"PART",3,sjsp_rec.sp_qty);
	  	pr_format (fin,fout,"PART",4,sjsp_rec.sp_uom);
	  	pr_format (fin,fout,"PART",5,sjsp_rec.sp_u_sell);
	  	pr_format (fin,fout,"PART",6,lin_chg);

		cc = find_rec ("sjsp",&sjsp_rec,NEXT,"r");
	}
	
	pr_format (fin,fout,"ULINE",0,0);
}

void
print_detail (
 void)
{
	if (sjhr_rec.hr_estim_type[0] != 'F')
	{
		pr_format (fin,fout,"TOT1",1,"Labour");
		pr_format (fin,fout,"TOT1",2,sjhr_rec.hr_lb_hrs);
		pr_format (fin,fout,"TOT1",3,"hrs");
		pr_format (fin,fout,"TOT1",4,sjhr_rec.hr_lb_chg);
	
		pr_format (fin,fout,"TOT1",1,"Mileage");
		pr_format (fin,fout,"TOT1",2,sjhr_rec.hr_km);
		pr_format (fin,fout,"TOT1",3,"km");
		pr_format (fin,fout,"TOT1",4,sjhr_rec.hr_km_chg);
	}

	proc_sjdc ();	/* print outside purchs details */

	/*=================
	| Print job totals |
	==================*/

	if (sjis_rec.is_prebill_amt > 0.00)
		pr_format (fin,fout,"TOT3",1,sjis_rec.is_prebill_amt);

	pr_format (fin,fout,"ULINE",0,0);

	pr_format (fin,fout,"TOT4",1,"Sub Total");
	pr_format (fin,fout,"TOT4",2,sjis_rec.is_invoice_chg);

	if (sjis_rec.is_invoice_chg != 0.00)
		gst_amt = (double) (sjis_rec.is_gst_pc / 100.00) * sjis_rec.is_invoice_chg;
	else
		gst_amt = 0.00;

	gst_amt = twodec (gst_amt);

	pr_format (fin,fout,"TOT4",1,"G.S.T");
	pr_format (fin,fout,"TOT4",2,gst_amt);

	inv_total = sjis_rec.is_invoice_chg + gst_amt;
	inv_total = twodec (inv_total);
	pr_format (fin,fout,"TOT4",1,"Invoice Total");
	pr_format (fin,fout,"TOT4",2,inv_total);

	pr_format (fin,fout,"ULINE",0,0);
	fprintf (fout,".EOF\n");
}

void
proc_sjdc (
 void)
{
	double 	tot_chg = 0.00;

	/*======================
	|  read outside purchs |
	======================*/
	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.tes_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf (sjdc_rec.dc_po_no,"%-8.8s"," ");
	cc = find_rec ("sjdc",&sjdc_rec,GTEQ,"r");

	while (!cc && !strcmp (sjdc_rec.dc_co_no,comm_rec.tco_no) && !strcmp(sjdc_rec.dc_est_no,comm_rec.tes_no) && !strcmp(sjdc_rec.dc_dp_no,comm_rec.tdp_no) && sjdc_rec.dc_order_no == sjhr_rec.hr_order_no)
	{
		tot_chg += sjdc_rec.dc_chg_cost;

		cc = find_rec ("sjdc",&sjdc_rec,NEXT,"r");
	}
	
  	pr_format (fin,fout,"TOT2",1,tot_chg);

}


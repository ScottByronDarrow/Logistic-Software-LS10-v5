/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_purge.c   )                                   |
|  Program Desc  : ( Purge service jobs from system               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr, sjjd, sjsd, sjsp,               |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (29/11/88)      | Modified  by  : B.C. Lim.        |
|  Date Modified : (27/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|   (27/11/89)   : Change sjsp_qty to floattype.                      |
|   (12/09/97)   : Updated for Multilingual Conversion.               |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
| $Log: sj_purge.c,v $
| Revision 5.2  2001/08/09 09:17:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:42  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:02  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/30 03:29:59  cam
| Fix bugs found during GVision testing
|
| Revision 1.7  1999/09/29 10:13:06  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:06:40  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/20 02:30:35  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_purge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_purge/sj_purge.c,v 5.2 2001/08/09 09:17:44 scott Exp $";

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
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
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
		{"sjhr_status"},
		{"sjhr_issue_date"},
	};

	int sjhr_no_fields = 6;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_issue_date;
	} sjhr_rec;

	/*=============================
	| Spare parts/cost items used |
	==============================*/
	struct dbview sjsp_list[] ={
		{"sjsp_co_no"},
		{"sjsp_est_no"},
		{"sjsp_dp_no"},
		{"sjsp_order_no"},
		{"sjsp_partno"},
		{"sjsp_part_desc"},
		{"sjsp_date"},
		{"sjsp_qty"},
		{"sjsp_uom"},
		{"sjsp_u_cost"},
		{"sjsp_u_sell"},
		{"sjsp_porder_no"}
	};

	int sjsp_no_fields = 12;

	struct {
		char	sp_co_no[3];
		char	sp_est_no[3];
		char	sp_dp_no[3];
		long	sp_order_no;
		char	sp_partno[17];
		char	sp_part_desc[41];
		long	sp_date;
		float	sp_qty;
		char	sp_uom[4];
		double	sp_u_cost;
		double	sp_u_sell;
		char	sp_porder_no[7];
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
		{"sjdc_desc"},
		{"sjdc_hhsu_hash"},
		{"sjdc_invoice_no"},
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"}
	};

	int sjdc_no_fields = 11;

	struct {
		char	dc_co_no[3];
		char	dc_est_no[3];
		char	dc_dp_no[3];
		long	dc_order_no;
		char	dc_po_no[9];
		char	dc_desc[31];
		long	dc_hhsu_hash;
		char	dc_invoice_no[9];
		double	dc_est_cost;
		double	dc_act_cost;
		double	dc_chg_cost;
	} sjdc_rec;

	/*======================
	| service details file |
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


	/*==================
	| Job details file |
	==================*/
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

	int		cr_owned = 0,
			cr_find  = 0;

	char  	branchNo[3],
			ord_number[9];	

	long	purge_date;


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void proc_file (void);
void proc_sp (void);
void proc_dc (void);
void proc_sd (void);
void proc_jd (void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 2)
	{
		print_at(0,0,mlSjMess707,argv[0]);
		return (EXIT_FAILURE);
	}

	init_scr ();

	cr_owned = atoi(get_env("CR_CO"));
	cr_find = atoi(get_env("CR_FIND"));

	OpenDB();

	strcpy (branchNo, (!cr_owned) ? " 0" : comm_rec.test_no);

	purge_date = StringToDate(argv[1]);

	if (purge_date < 0L)
	{
		print_at(0,0, mlStdMess111);
        return (EXIT_FAILURE);
	}

	dsp_screen(" Purging Service Job Invoices ",comm_rec.tco_no,comm_rec.tco_name);

	proc_file();

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no2");
	open_rec("sjjd", sjjd_list, sjjd_no_fields, "sjjd_id_no");
	open_rec("sjdc", sjdc_list, sjdc_no_fields, "sjdc_id_no");
	open_rec("sjsp", sjsp_list, sjsp_no_fields, "sjsp_id_no");
	open_rec("sjsd", sjsd_list, sjsd_no_fields, "sjsd_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("sjjd");
	abc_fclose("sjdc");
	abc_fclose("sjsp");
	abc_fclose("sjsd");
	abc_dbclose("data");
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
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	strcpy(sjhr_rec.hr_status,"I");
	sjhr_rec.hr_order_no = 0L;
	cc = find_rec("sjhr", &sjhr_rec, GTEQ, "r");

	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no) && sjhr_rec.hr_status[0] == 'I')
	{
		if (sjhr_rec.hr_issue_date <= purge_date)
		{
			sprintf(ord_number,"%8ld",sjhr_rec.hr_order_no);
			dsp_process("Order No : ",ord_number);
			
			proc_sp(); /*  purge parts usage  */

			proc_dc(); /*  purge outside purchs  */

			proc_sd(); /*  purge service details */

			proc_jd(); /*  purge job details */

			cc = abc_delete("sjhr");
			if (cc)
				sys_err("Error in sjhr During (DBDELETE)",cc, PNAME);
		}
		cc = find_rec("sjhr", &sjhr_rec, NEXT, "r");
	}
}

void
proc_sp (
 void)
{
	strcpy(sjsp_rec.sp_co_no,comm_rec.tco_no);
	strcpy(sjsp_rec.sp_est_no,comm_rec.test_no);
	strcpy(sjsp_rec.sp_dp_no,comm_rec.tdp_no);
	sjsp_rec.sp_order_no = sjhr_rec.hr_order_no;
	sprintf(sjsp_rec.sp_partno,"%-16.16s"," ");
	cc = find_rec("sjsp",&sjsp_rec,GTEQ,"r");

	while (!cc && !strcmp(sjsp_rec.sp_co_no,comm_rec.tco_no) && !strcmp(sjsp_rec.sp_est_no,comm_rec.test_no) && !strcmp(sjsp_rec.sp_dp_no,comm_rec.tdp_no) && sjsp_rec.sp_order_no == sjhr_rec.hr_order_no)
	{
		cc = abc_delete("sjsp");
		if (cc)
			sys_err("Error in sjsp During (DBDELETE)",cc, PNAME);

		cc = find_rec("sjsp",&sjsp_rec,NEXT,"r");
	}
}

/*===========================
| Process Outside purchases |
| from sjdc file            |
===========================*/
void
proc_dc (
 void)
{
	strcpy(sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy(sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy(sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf(sjdc_rec.dc_po_no,"%-8.8s"," ");
	cc = find_rec("sjdc",&sjdc_rec,GTEQ,"r");

	while (!cc && !strcmp(sjdc_rec.dc_co_no,comm_rec.tco_no) && !strcmp(sjdc_rec.dc_est_no,comm_rec.test_no) && !strcmp(sjdc_rec.dc_dp_no,comm_rec.tdp_no) && sjdc_rec.dc_order_no == sjhr_rec.hr_order_no)
	{
		cc = abc_delete("sjdc");
		if (cc)
			sys_err("Error in sjdc During (DBDELETE)",cc, PNAME);

		cc = find_rec("sjdc",&sjdc_rec,NEXT,"r");
	}
}

void
proc_sd (
 void)
{
	strcpy(sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy(sjsd_rec.sd_est_no,comm_rec.test_no);
	strcpy(sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = 0;
	cc = find_rec("sjsd",&sjsd_rec,GTEQ,"r");

	while (!cc && !strcmp(sjsd_rec.sd_co_no,comm_rec.tco_no) && !strcmp(sjsd_rec.sd_est_no,comm_rec.test_no) && !strcmp(sjsd_rec.sd_dp_no,comm_rec.tdp_no) && sjsd_rec.sd_order_no == sjhr_rec.hr_order_no)
	{
		cc = abc_delete("sjsd");
		if (cc)
			sys_err("Error in sjsd During (DBDELETE)",cc, PNAME);

		cc = find_rec("sjsd",&sjsd_rec,NEXT,"r");
	}
}

/*=====================
| Process Job details |
=====================*/
void
proc_jd (
 void)
{
	strcpy(sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy(sjjd_rec.jd_est_no,comm_rec.test_no);
	strcpy(sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;
	cc = find_rec("sjjd",&sjjd_rec,GTEQ,"r");

	while (!cc && !strcmp(sjjd_rec.jd_co_no,comm_rec.tco_no) && !strcmp(sjjd_rec.jd_est_no,comm_rec.test_no) && !strcmp(sjjd_rec.jd_dp_no,comm_rec.tdp_no) && sjjd_rec.jd_order_no == sjhr_rec.hr_order_no)
	{
		cc = abc_delete("sjjd");
		if (cc)
			sys_err("Error in sjjd During (DBDELETE)",cc, PNAME);

		cc = find_rec("sjjd",&sjjd_rec,NEXT,"r");
	}
}

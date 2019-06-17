/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_ma_rep.c  )                                   |
|  Program Desc  : ( Service Job Costing Margin Analysis Report   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr,     ,     ,     ,     ,               |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/05/88         |
|---------------------------------------------------------------------|
|  Date Modified : (31/05/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (07/12/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (30/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|                :                                                    |
|    (30/11/89)  : Change sjhr_km to float.                           |
|                :                                                    |
|                :                                                    |
| $Log: sj_ma_rep.c,v $
| Revision 5.1  2001/08/09 09:17:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:14:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:00  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:34:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 05:58:34  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.9  1999/10/20 02:07:04  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/09/29 10:13:05  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/24 05:06:39  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:34  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ma_rep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ma_rep/sj_ma_rep.c,v 5.1 2001/08/09 09:17:42 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <get_lpno.h>
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
		{"comm_inv_date"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		long	tinv_date;
	} comm_rec;

	/*=================================
	| Service Job Header Record File. |
	=================================*/
	struct dbview sjhr_list[] ={
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_invoice_no"},
		{"sjhr_inv_amt"},
		{"sjhr_comp_date"},
		{"sjhr_oh_cost"},
		{"sjhr_lb_cost"},
		{"sjhr_lb_chg"},
		{"sjhr_mt_cost"},
		{"sjhr_mt_chg"},
		{"sjhr_km"},
		{"sjhr_km_chg"},
	};

	int sjhr_no_fields = 14;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_invoice_no[9];
		double	hr_inv_amt;
		long	hr_comp_date;
		double	hr_oh_cost;
		double	hr_lb_cost;
		double	hr_lb_chg;
		double	hr_mt_cost;
		double	hr_mt_chg;
		float	hr_km;	
		double	hr_km_chg;
	} sjhr_rec;

	/*===================
	| Outside purchases |
	===================*/
	struct dbview sjdc_list[] ={
		{"sjdc_co_no"},
		{"sjdc_est_no"},
		{"sjdc_dp_no"},
		{"sjdc_order_no"},
		{"sjdc_po_no"},
		{"sjdc_invoice_no"},
		{"sjdc_date"},
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"}
	};

	int sjdc_no_fields = 10;

	struct {
		char	dc_co_no[3];
		char	dc_est_no[3];
		char	dc_dp_no[3];
		long	dc_order_no;
		char	dc_po_no[9];
		char	dc_invoice_no[9];
		long	dc_date;
		double	dc_est_cost;
		double	dc_act_cost;
		double	dc_chg_cost;
	} sjdc_rec;

	long	start_date = 0L,
			end_date = 0L;

	int		lpno = 1;

	double	dc_cst = 0.00;
	double	dc_chg = 0.00;

	char	prog_desc[51];

	FILE	*fout;


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void head_output (void);
void process (void);
void print_line (void);
void proc_sjdc (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 5)
	{
		print_at(0,0,mlSjMess705,argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf(prog_desc,"%-50.50s",argv[1]);
	lpno = atoi(argv[2]);
	start_date = StringToDate(argv[3]);
	end_date = StringToDate(argv[4]);

	if (start_date < 0L)
	{
		print_at(0,0,mlStdMess111);
		return (EXIT_FAILURE);
	}

	if (end_date < 0L)
	{
		print_at(0,0,mlStdMess111);
		return (EXIT_FAILURE);
	}

	OpenDB();

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	head_output();

	process();

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
	fprintf(fout, ".EOF\n");
	pclose(fout);

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
	abc_dbopen("data");

	open_rec("sjhr",sjhr_list,sjhr_no_fields,"sjhr_id_no");
	open_rec("sjdc",sjdc_list,sjdc_no_fields,"sjdc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("sjdc");
	abc_dbclose("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{

	dsp_screen("Printing Service Job Margin Analysis Report",comm_rec.tco_no,comm_rec.tco_name);

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in opening pformat During (POPEN)",errno,PNAME);
	
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(fout, ".LP%d\n",lpno);

	fprintf(fout, ".15\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, ".L158\n");
	fprintf(fout, ".ESERVICE JOB MARGIN ANALYSIS REPORT\n");
	fprintf(fout, ".E%s\n",clip(prog_desc));
	fprintf(fout, ".B1\n");
	fprintf(fout, ".ECompany : %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf(fout, ".EBranch  : %s - %s\n",comm_rec.test_no,clip(comm_rec.test_name));
	fprintf(fout, ".EFrom %-10.10s ",DateToString(start_date));
	fprintf(fout, "To %-10.10s\n",DateToString(end_date));
	fprintf(fout, ".B1\n");
	fprintf(fout, ".EAS AT %-24.24s\n",SystemTime());
	fprintf(fout, ".B1\n");

	fprintf(fout, ".R=========");
	fprintf(fout, "==========");
	fprintf(fout, "=======");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "========\n");

	fprintf(fout, "=========");
	fprintf(fout, "==========");
	fprintf(fout, "=======");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "========\n");

	fprintf(fout, "| Service");
	fprintf(fout, " Invoice  ");
	fprintf(fout, " Inv.  ");
	fprintf(fout, " <-------- ");
	fprintf(fout, "  Labour   ");
	fprintf(fout, " --------> ");
	fprintf(fout, " Service / ");
	fprintf(fout, " Materials ");
	fprintf(fout, "  Vehicle  ");
	fprintf(fout, "  Outside  ");
	fprintf(fout, " Purchase  ");
	fprintf(fout, "   Cost     ");
	fprintf(fout, "  Invoice   ");
	fprintf(fout, " <-- Margin ");
	fprintf(fout, " -->   |\n");

	fprintf(fout, "|  Order ");
	fprintf(fout, "  Number  ");
	fprintf(fout, " Date  ");
	fprintf(fout, "   Cost    ");
	fprintf(fout, "  Overhead ");
	fprintf(fout, "   Profit  ");
	fprintf(fout, "   Cost    ");
	fprintf(fout, "  Charge   ");
	fprintf(fout, "  Charge   ");
	fprintf(fout, "   Cost    ");
	fprintf(fout, "  Charge   ");
	fprintf(fout, "   Total    ");
	fprintf(fout, "   Total    ");
	fprintf(fout, "   Total    ");
	fprintf(fout, "   %%   |\n");

	fprintf(fout, "|--------");
	fprintf(fout, "----------");
	fprintf(fout, "-------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "-----------");
	fprintf(fout, "------------");
	fprintf(fout, "------------");
	fprintf(fout, "------------");
	fprintf(fout, "-------|\n");

	fflush(fout);
}

void
process (
 void)
{
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = 0L;

	cc = find_rec("sjhr",&sjhr_rec,GTEQ,"r");
	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{
		if (sjhr_rec.hr_comp_date >= start_date && sjhr_rec.hr_comp_date <= end_date)
			print_line();
		cc = find_rec("sjhr",&sjhr_rec,NEXT,"r");
	}
}

void
print_line (
 void)
{
	double	profit = 0.00;
	double	total_cost = 0.00;
	double	margin_total = 0.00;
	float	margin_pc = 0.00;

	profit = sjhr_rec.hr_lb_chg - sjhr_rec.hr_lb_cost - sjhr_rec.hr_oh_cost;
	total_cost = sjhr_rec.hr_lb_cost + sjhr_rec.hr_oh_cost + 
		sjhr_rec.hr_mt_cost + sjhr_rec.hr_km_chg;
	margin_total = sjhr_rec.hr_inv_amt - total_cost;
	if (sjhr_rec.hr_inv_amt != 0.00)
		margin_pc = (float) ((margin_total / sjhr_rec.hr_inv_amt) * 100.00);
	proc_sjdc();

	fprintf(fout,"|%06ld",sjhr_rec.hr_order_no);
	fprintf(fout," %-8.8s ",sjhr_rec.hr_invoice_no);
	fprintf(fout," %-5.5s ",DateToString(sjhr_rec.hr_comp_date));
	fprintf(fout," %9.2f ",sjhr_rec.hr_lb_cost);
	fprintf(fout," %9.2f ",sjhr_rec.hr_oh_cost);
	fprintf(fout," %9.2f ",profit);
	fprintf(fout," %9.2f ",sjhr_rec.hr_mt_cost);
	fprintf(fout," %9.2f ",sjhr_rec.hr_mt_chg);
	fprintf(fout," %9.2f ",sjhr_rec.hr_km_chg);
	fprintf(fout," %9.2f ",dc_cst);
	fprintf(fout," %9.2f ",dc_chg);
	fprintf(fout," %10.2f ",total_cost);
	fprintf(fout," %10.2f ",sjhr_rec.hr_inv_amt);
	fprintf(fout," %10.2f ",margin_total);
	fprintf(fout," %6.2f|\n",margin_pc);
	fflush(fout);
}

void
proc_sjdc (
 void)
{
	dc_cst = 0.00;
	dc_chg = 0.00;

	strcpy(sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy(sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy(sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf(sjdc_rec.dc_po_no,"%8.8s"," ");

	cc = find_rec("sjdc",&sjdc_rec,GTEQ,"r");

	while (!cc && !strcmp(sjdc_rec.dc_co_no,comm_rec.tco_no) && !strcmp(sjdc_rec.dc_est_no,comm_rec.test_no) && !strcmp(sjdc_rec.dc_dp_no,comm_rec.tdp_no) && sjdc_rec.dc_order_no == sjhr_rec.hr_order_no)
	{
		if (sjdc_rec.dc_act_cost == 0.00)
			sjdc_rec.dc_act_cost = sjdc_rec.dc_est_cost;

		dc_cst += sjdc_rec.dc_act_cost;
		dc_chg += sjdc_rec.dc_chg_cost;

		cc = find_rec("sjdc",&sjdc_rec,NEXT,"r");
	}
}

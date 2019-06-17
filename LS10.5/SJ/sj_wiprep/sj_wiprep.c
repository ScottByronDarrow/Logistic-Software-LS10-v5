/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_wiprep.c                                      |
|  Program Desc  : ( Print/Display Service Job Costing Report.    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : 23/11/88        | Modified By : B.C.Lim.           |
|  Date Modified : 27/11/89        | Modified By : Fui Choo Yap.      |
|  Date Modified : (13/09/97)      | Modified By : Leah Manibog.      |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator, com-  |
|                : bine the printing & display programs i.e sj_wiprep |
|                : & sj_wipdisp into one program.                     |
|                :                                                    |
|    (27/11/89)  : Change money fields to double.                     |
|                : Combined sj_statrep and sj_wiprep into one program.|
|    (13/09/97)  : Updated for Multilingual Conversion.               |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
| $Log: sj_wiprep.c,v $
| Revision 5.2  2001/08/09 09:17:51  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:08  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/04/11 13:21:34  ramon
| Changed the coordinates of prmptmsg() call in halt() to see it in GVision.
|
| Revision 1.13  1999/12/06 01:34:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 05:58:37  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.11  1999/10/14 00:57:27  cam
| Remove cc
|
| Revision 1.10  1999/09/29 10:13:09  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/29 01:53:12  scott
| Updated for getkey + ansi
|
| Revision 1.8  1999/09/24 05:13:09  scott
| Updated for Ansi project
|
| Revision 1.7  1999/09/24 05:06:44  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:39  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_wiprep/sj_wiprep.c,v 5.2 2001/08/09 09:17:51 scott Exp $";
char	*PNAME = "$RCSfile: sj_wiprep.c,v $";

#include <pslscr.h>
#include <getnum.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

#define		P_SIZE	15
#define		PRINTER		(output_to[0] == 'P')
#define		RESET_LN	(ln_num >= P_SIZE) ? (ln_num = ln_num % P_SIZE) : ln_num++;
#define		WIP 		0
#define		STAT		1

char	*UNDERLINE = "==================================================================================================================";

char	*HEADING1 = " Order Number | Client | Name                                     |  Labour   | Overheads | Materials |  Mileage  ";
char	*HEADING2 = " Order Number | Client | Name                                     | Cust Ord.  | Issued   |       Status          ";

char	*LINE1 = "^^GGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGG";
char	*LINE2 = "^^GGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGG";

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
		{"comm_dbt_date"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		long	tdbt_date;
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
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_cust_ord_no"},
		{"sjhr_issue_date"},
		{"sjhr_oh_cost"},
		{"sjhr_lb_cost"},
		{"sjhr_lb_chg"},
		{"sjhr_lb_hrs"},
		{"sjhr_mt_cost"},
		{"sjhr_mt_chg"},
		{"sjhr_km"},
		{"sjhr_km_chg"},
	};

	int sjhr_no_fields = 17;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_ord_no[11];
		long	hr_issue_date;
		double	hr_oh_cost;
		double	hr_lb_cost;
		double	hr_lb_chg;
		float	hr_lb_hrs;
		double	hr_mt_cost;
		double	hr_mt_chg;
		float	hr_km;	
		double	hr_km_chg;
	} sjhr_rec;

	/*=====================================
	| Service Job Spare Parts Usage File. |
	=====================================*/
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

	/*==============================
	| Labour analysis detail file. |
	==============================*/
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

	/*================================
	| Service Job Disbursement Cost. |
	================================*/
	struct dbview sjdc_list[] ={
		{"sjdc_co_no"},
		{"sjdc_est_no"},
		{"sjdc_dp_no"},
		{"sjdc_order_no"},
		{"sjdc_po_no"},
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"},
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

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
	};

	int cumr_no_fields = 5;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
	} cumr_rec;

	int	envDbCo = 0,
		envDbFind  = 0,
		lp_no = 0,
		ln_num = 0,
		all_cust = FALSE;
	int	rep_type;


	char	branchNo[3],
			output_to[2],
			dummy[11],
			env_line[250],
			cust_no[7];
	char	req_stat[2];
	char	stat_desc[16];

	double  mt_cost_tot = 0.0,
			mt_chg_tot = 0.0,
			lb_cost_tot = 0.0,
			lb_chg_tot = 0.0,
			km_chg_tot = 0.0,
			oh_cost_tot = 0.0,
			tot_lb = 0.0,
			tot_oh = 0.0,
			tot_mt = 0.0,
			tot_km = 0.0;

	float	lb_hrs_tot = 0.0,
			km_tot = 0.0;

	FILE	*fout;

	struct {
		char	*dsp_wds;
		char	*rep_name;
		char	*title;
		char	*line;
	} rep_desc[2] = {
		{"Costing", "COST SUMMARY", "|  Labour   | Overheads | Materials |  Mileage  |", "|--------------|--------|------------------------------------------|-----------|-----------|-----------|-----------|\n"},
		{"Status ", " STATUS ", "| Cust Ord.  | Issued   | Status                |", "|--------------|--------|------------------------------------------|------------|----------|-----------------------|\n"},
	};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void head_output (void);
void proc_sjhr (void);
void get_stat_desc (void);
void cost_sjhr (void);
void proc_sp (void);
void proc_ld (void);
void proc_dc (void);
double extend (float qty, double rate);
int heading (void);
void proc_cost (char *mode);
long select_ord (void);
void order_srch (char *key_val);
int sjhr_disp (void);
int halt (void);
void print_key (void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 5)
	{
		print_at(0,0, mlSjMess708 ,argv[0]);
		return (EXIT_FAILURE);
	}

	if (strncmp(argv[0],"sj_statrep",10) == 0)
		rep_type = STAT;
	else
	if (strncmp(argv[0],"sj_wiprep",9) == 0)
		rep_type = WIP;

	lp_no = atoi(argv[4]);

	switch (argv[1][0])
	{
	case 'D':
	case 'd':
		strcpy(output_to,"D");
		break;

	case 'P':
	case 'p':
		strcpy(output_to,"P");
		break;

	default :
		print_at(0,0, mlSjMess710);
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[2],"ALL   ") == 0)
		all_cust = TRUE;
	sprintf(cust_no,"%-6.6s",argv[2]);

	sprintf(req_stat,"%-1.1s",argv[3]);

	if (req_stat[0] != 'O' && req_stat[0] != 'C' && req_stat[0] != 'I')
	{
		print_at(0,0, mlSjMess708 ,argv[0]);

		return (EXIT_FAILURE);
	}

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	OpenDB();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

	init_scr();
	set_tty();

	if (PRINTER)
	{
		sprintf(err_str,"Printing Service Job %-s Report",rep_desc[rep_type].dsp_wds);
		dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);

		head_output();
		proc_sjhr();
	}
	else
		while (!sjhr_disp()) ;

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
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no3");
	open_rec("sjsp", sjsp_list, sjsp_no_fields, "sjsp_id_no");
	open_rec("sjld", sjld_list, sjld_no_fields, "sjld_id_no");
	open_rec("sjdc", sjdc_list, sjdc_no_fields, "sjdc_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields, (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("sjsp");
	abc_fclose("sjld");
	abc_fclose("sjdc");
	abc_fclose("cumr");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	return(0);
}

void
head_output (
 void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.tdbt_date), PNAME);
	fprintf(fout,".LP%d\n",lp_no);
	fprintf(fout,".11\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L158\n");
	fprintf(fout,".ECOMPANY %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf(fout,".ESERVICE JOB %-s REPORT\n",rep_desc[rep_type].rep_name);
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime());
	if (all_cust)
		fprintf(fout,".EFor All Customers\n");
	else
		fprintf(fout,".ECustomer Selected : %-6.6s\n",cust_no);

	fprintf(fout,"\n");

	fprintf(fout,".R|==================================================================================================================|\n");
	fprintf(fout,"|==================================================================================================================|\n");
	fprintf(fout,"| Order Number | Client | Name                                     ");
	fprintf(fout,"%-s\n",rep_desc[rep_type].title);
	fprintf(fout,rep_desc[rep_type].line);

	fflush(fout);
}

void
proc_sjhr (
 void)
{
	int	first_time = TRUE;
	int	printed = FALSE;
	long	hhcu_hash = 0L;
	long	prev_hhcu = 0L;
	char	ord[9];

	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	if (all_cust)
		sjhr_rec.hr_chg_client = 0L;
	else
	{
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNo);
		strcpy(cumr_rec.cm_dbt_no,cust_no);
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");
		sjhr_rec.hr_chg_client = cumr_rec.cm_hhcu_hash;
		hhcu_hash = cumr_rec.cm_hhcu_hash;
	}

	sprintf(sjhr_rec.hr_status,"%1.1s"," ");
	sjhr_rec.hr_order_no = 0L;
	cc = find_rec("sjhr",&sjhr_rec,GTEQ,"r");
	abc_selfield("cumr","cumr_hhcu_hash");

	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{
		if (!all_cust && sjhr_rec.hr_chg_client != hhcu_hash)
			break;

		if (sjhr_rec.hr_status[0] == req_stat[0])
		{
			printed = TRUE;

			if (rep_type == STAT)
			{
				sprintf(ord,"%8ld",sjhr_rec.hr_order_no);
				if (PRINTER)
					dsp_process("Order No. : ",ord);
				get_stat_desc();
			}

			if (prev_hhcu != sjhr_rec.hr_chg_client)
			{
				cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_chg_client);
				if (cc)
				{
					strcpy(cumr_rec.cm_dbt_no,"      ");
					strcpy(cumr_rec.cm_name,"**** Customer Unknown ****");
				}
				prev_hhcu = sjhr_rec.hr_chg_client;

				if (!first_time)
				{
					if (PRINTER)
						fprintf(fout,rep_desc[rep_type].line);
					else
						Dsp_saverec((rep_type == WIP) ? LINE1 : LINE2);
					RESET_LN;
				}
				first_time = FALSE;
			}

			if (rep_type == WIP)
				cost_sjhr();

			if (PRINTER)
			{
				switch (rep_type)
				{
				case	WIP:
					fprintf(fout,"|   %8ld   | %-6.6s | %-40.40s | %9.2f | %9.2f | %9.2f | %9.2f |\n",
						sjhr_rec.hr_order_no,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						sjhr_rec.hr_lb_cost,
						sjhr_rec.hr_oh_cost,
						sjhr_rec.hr_mt_cost,
						sjhr_rec.hr_km_chg);
					break;

				case	STAT:
					fprintf(fout,"| %8ld     | %-6.6s | %-40.40s | %-10.10s | %-8.8s | %1.1s - %-15.15s   |\n",
						sjhr_rec.hr_order_no,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						sjhr_rec.hr_cust_ord_no,
						DateToString(sjhr_rec.hr_issue_date),
						sjhr_rec.hr_status,
						stat_desc);
					break;

				}
			}
			else
			{
				switch (rep_type)
				{
				case	WIP:
					sprintf(env_line,"   %8ld   ^E %-6.6s ^E %-40.40s ^E %9.2f ^E %9.2f ^E %9.2f ^E %9.2f ",
						sjhr_rec.hr_order_no,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						sjhr_rec.hr_lb_cost,
						sjhr_rec.hr_oh_cost,
						sjhr_rec.hr_mt_cost,
						sjhr_rec.hr_km_chg);
					break;

				case	STAT:
					sprintf(env_line," %8ld     ^E %-6.6s ^E %-40.40s ^E %-10.10s ^E %-8.8s ^E %1.1s - %-15.15s   ",
						sjhr_rec.hr_order_no,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						sjhr_rec.hr_cust_ord_no,
						DateToString(sjhr_rec.hr_issue_date),
						sjhr_rec.hr_status,
						stat_desc);
					break;

				}
				Dsp_saverec(env_line);
				RESET_LN;
			}
		}
		cc = find_rec("sjhr",&sjhr_rec,NEXT,"r");
	}
	if (PRINTER)
	{
		if (rep_type == WIP)
		{
			fprintf(fout,rep_desc[rep_type].line);
			fprintf(fout,"|              |        |                                   TOTALS | %9.2f | %9.2f | %9.2f | %9.2f |\n",
				tot_lb,
				tot_oh,
				tot_mt,
				tot_km);
		}
		fprintf(fout,".EOF\n");
	}
	else
	{
		if (printed)
		{
			if (rep_type == WIP)
			{
				Dsp_saverec(LINE1);
				RESET_LN;
				sprintf(env_line,"              ^E        ^E                                   TOTALS ^E %9.2f ^E %9.2f ^E %9.2f ^E %9.2f ",
					tot_lb,
					tot_oh,
					tot_mt,
					tot_km);
				Dsp_saverec(env_line);
				RESET_LN;
			}
			Dsp_saverec(UNDERLINE);
		}

		if (rep_type == WIP)
			RESET_LN;

		Dsp_srch();
		Dsp_close();
	}

	abc_selfield("cumr",(envDbFind) ? "cumr_id_no" : "cumr_id_no3");
}

void
get_stat_desc (
 void)
{
	switch(sjhr_rec.hr_status[0])
	{
        case 'O':
		strcpy(stat_desc,"Order Opened");
		break;
        case 'C':
		strcpy(stat_desc,"Order Closed");
		break;
        case 'I':
		strcpy(stat_desc,"Order Invoiced");
		break;
	default:
		strcpy(stat_desc,"Unknown Status");
		break;
	}
}

/*======================
| Job costing routines |
======================*/
void
cost_sjhr (
 void)
{
	mt_cost_tot = 0;
	mt_chg_tot = 0;
	lb_cost_tot = 0;
	lb_hrs_tot = 0;
	lb_chg_tot = 0;
	km_tot = 0;
	km_chg_tot = 0;
	oh_cost_tot = 0;
	
	proc_sp();

	proc_ld();

	proc_dc();

	tot_lb += sjhr_rec.hr_lb_cost;
	tot_oh += sjhr_rec.hr_oh_cost;
	tot_mt += sjhr_rec.hr_mt_cost;
	tot_km += sjhr_rec.hr_km_chg;
}

/*===================================
| Process Materials/service charges |
| from sjsp file.                   |
===================================*/
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
		mt_cost_tot += extend(sjsp_rec.sp_qty,sjsp_rec.sp_u_cost);
		mt_chg_tot  += extend(sjsp_rec.sp_qty,sjsp_rec.sp_u_sell);
		cc = find_rec("sjsp",&sjsp_rec,NEXT,"r");
		
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

	strcpy(sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy(sjld_rec.ld_est_no,comm_rec.test_no);
	strcpy(sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = sjhr_rec.hr_order_no;
	sjld_rec.ld_date = 0L;
	sprintf(sjld_rec.ld_emp_code,"%-10.10s"," ");
	cc = find_rec("sjld",&sjld_rec,GTEQ,"r");

	while (!cc && !strcmp(sjld_rec.ld_co_no,comm_rec.tco_no) && !strcmp(sjld_rec.ld_est_no,comm_rec.test_no) && !strcmp(sjld_rec.ld_dp_no,comm_rec.tdp_no) && sjld_rec.ld_order_no == sjhr_rec.hr_order_no)
	{
		hrs 	= (float) (sjld_rec.ld_time +
			  sjld_rec.ld_time_half * 1.5 +
			  sjld_rec.ld_time_double * 2.0);
			
		lb_hrs_tot  += hrs;
		lb_cost_tot += extend(hrs,sjld_rec.ld_tm_rate);
		lb_chg_tot  += extend(hrs,sjld_rec.ld_pr_rate);

		oh_cost_tot += extend(hrs,sjld_rec.ld_oh_rate);
		km_tot      += sjld_rec.ld_km;
		km_chg_tot  += extend(sjld_rec.ld_km,sjld_rec.ld_km_rate);
		cc = find_rec("sjld",&sjld_rec,NEXT,"r");
		
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
	strcpy(sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy(sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy(sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf(sjdc_rec.dc_po_no,"%-8.8s"," ");
	cc = find_rec("sjdc",&sjdc_rec,GTEQ,"r");

	while (!cc && !strcmp(sjdc_rec.dc_co_no,comm_rec.tco_no) && !strcmp(sjdc_rec.dc_est_no,comm_rec.test_no) && !strcmp(sjdc_rec.dc_dp_no,comm_rec.tdp_no) && sjdc_rec.dc_order_no == sjhr_rec.hr_order_no)
	{
		if (sjdc_rec.dc_act_cost != 0)
			mt_cost_tot += sjdc_rec.dc_act_cost;
		else
			mt_cost_tot += sjdc_rec.dc_est_cost;
		mt_chg_tot += sjdc_rec.dc_chg_cost;
		cc = find_rec("sjdc",&sjdc_rec,NEXT,"r");
	}
	sjhr_rec.hr_mt_cost = mt_cost_tot; 
	sjhr_rec.hr_mt_chg  = mt_chg_tot; 
}

double 
extend (
 float qty, 
 double rate)
{
	double	extn = 0.0;
	
	extn = (double) qty;
	extn *= rate;
	return(extn);
}

int
heading (
 void)
{
	int	y;
	char	exp_head[100];

	sprintf(err_str," SERVICE JOB %-s DISPLAY ",rep_desc[rep_type].rep_name);
	expand(exp_head,err_str);
	y = (116 - strlen(clip(err_str))) / 2;
/*
	rv_pr(exp_head,y,0,0);
*/
	print_at(0,y,exp_head);

	Dsp_open(0,1,P_SIZE);
	if (all_cust)
		sprintf(env_line,"                                                FOR ALL CUSTOMERS                                                 ");
	else
		sprintf(env_line,"                                            CUSTOMER SELECTED : %-6.6s                                            ",cust_no);
	Dsp_saverec(env_line);

	Dsp_saverec((rep_type == WIP) ? HEADING1 : HEADING2);

	Dsp_saverec("  [Next Screen]    [Previous Screen]    [Input/End.]  ");

    return (EXIT_SUCCESS);
}

void
proc_cost (
 char *mode)
{
	long	ord_no = 0L;
	char	order_no[9];
	char	printer_no[7];

	ord_no = select_ord();
	if (ord_no == 0L)
		return;

	clear();

	sprintf(order_no,"%8ld",ord_no);
	sprintf(printer_no,"%2d",lp_no);

	/*==================================
	| Suspend display, process costing |
	| then restart at current page     |
	==================================*/
	rv_pr(ML(mlStdMess035) ,10,10,1);

	*(arg) = "sj_cstrep";
	*(arg+(1)) = mode;
	*(arg+(2)) = order_no;
	*(arg+(3)) = printer_no;
	*(arg+(4)) = (char *)0;
	shell_prog(4);
}

long 
select_ord (
 void)
{
	long ord_no = 0L;

	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);

	do
	{
		ord_no = 0L;
		last_char = 0;

		rv_pr(ML(mlSjMess016) ,10,10,1);

		ord_no = getlong(55,10,"NNNNNN");
		if (ord_no == 0L)
			sprintf(temp_str,"%80.80s"," ");
		else
			sprintf(temp_str,"%8ld%72.72s",ord_no," ");

		switch (last_char)
		{
		case	SEARCH :
			order_srch(temp_str);
			ord_no = atol(temp_str);
			return(ord_no);

		case	REDRAW :
			break;

		case	EOI:
		case	RESTART:
			return(0L);
		
		default :
			break;
		}

		if (dflt_used)
			return(0L);

		abc_selfield("sjhr","sjhr_id_no");
		sjhr_rec.hr_order_no = ord_no;
		cc = find_rec("sjhr",&sjhr_rec,GTEQ,"r");
		abc_selfield("sjhr","sjhr_id_no3");
		if (cc)
		{
			errmess(ML(mlStdMess122));
			sleep(3);
			break;
		}
		
	} while (last_char != ENDINPUT) ;

	return(ord_no);
}

void
order_srch (
 char *key_val)
{
	char	order[9];

	work_open();
	abc_selfield("sjhr","sjhr_id_no");
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol(key_val);
	save_rec("#Order","#Issued On");
	cc = find_rec("sjhr",&sjhr_rec,GTEQ,"r");

	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{
		sprintf(order,"%8ld",sjhr_rec.hr_order_no);
		if (strlen(clip(key_val)) == 0 || strncmp(order,key_val,strlen(key_val)) >= 0)
		{
			strcpy (err_str, DateToString(sjhr_rec.hr_issue_date));
			cc = save_rec(order, err_str);
			if (cc)
				break;
		}
		cc = find_rec("sjhr",&sjhr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol(temp_str);
	cc = find_rec("sjhr",&sjhr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in sjhr During (DBFIND)", cc, PNAME);
	abc_selfield("sjhr","sjhr_id_no3");
}

int
sjhr_disp (
 void)
{
	crsr_off();
	clear();
	swide();
     	move(0,21);	
     	line(80);

     	print_at(22,0,  ML(mlStdMess038) ,clip(comm_rec.tco_no), clip(comm_rec.tco_name));
     	print_at(22,45, ML(mlStdMess039) ,clip(comm_rec.test_no), clip(comm_rec.test_name));

	if (halt())
		return(1);
	return(0);
}

int
halt (
 void)
{
	int  ans;

	tot_lb = 0.0; 
	tot_oh = 0.0;
	tot_mt = 0.0;
	tot_km = 0.0;

	print_key();

	ans = prmptmsg (ML (mlSjMess020), "SsPpDdEe", 15, 20);
	switch (ans)
	{
	case	'S':
	case	's':
		heading();
		proc_sjhr();
		break;

	case	'P':
	case	'p':
		proc_cost("P");
		break;

	case	'D':
	case	'd':
		proc_cost("D");
		break;

	case	'E':
	case	'e':
		return(1);
	}
	return(0);
}

void
print_key (
 void)
{
	sprintf(err_str, ML(mlSjMess018) ,rep_desc[rep_type].dsp_wds);
	us_pr(ML(mlSjMess017) ,30,10,1);
	us_pr(err_str,25,12,1);
	us_pr(ML(mlSjMess019) ,25,14,1);

}

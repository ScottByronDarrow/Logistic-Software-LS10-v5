/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_pbcr_prt.c   )                                |
|  Program Desc  : ( Print Service credits & prebill invoices     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjis, sjhr, cumr, sjjd,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 08/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (09/04/87)      | Modified  by  : Lance Whitford   |
|  Date Modified : (29/11/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (24/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (16/10/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      :                                                    |
|     (24/11/89) : Change moneytype fields to doubletype.             |
|     (16/10/97) : Change inv_no from 6 to 8 char.                    |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
| $Log: pbcr_prt.c,v $
| Revision 5.2  2001/08/09 09:17:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:33  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:26  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:01  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:34:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/10/20 02:07:04  nz
| Updated for final changes on date routines.
|
| Revision 1.9  1999/09/29 10:13:05  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 05:06:39  scott
| Updated from Ansi
|
| Revision 1.7  1999/06/20 02:30:34  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pbcr_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_pbcr_prt/pbcr_prt.c,v 5.2 2001/08/09 09:17:42 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
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
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
	};

	int sjhr_no_fields = 6;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_chg_client;
		long	hr_end_client;
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
		{"sjis_end_client"},
		{"sjis_cust_ord_no"},
		{"sjis_invoice_chg"},
		{"sjis_status"},
		{"sjis_gst_pc"},
		{"sjis_type"},
		{"sjis_prt_stat"}
	};

	int sjis_no_fields = 13;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
		long	is_end_client;
		char	is_cust_ord_no[11];
		double	is_invoice_chg;
		char	is_status[2];
		float	is_gst_pc;
		char	is_type[2];
		char	is_prt_stat[2];
	} sjis_rec;

	/*=========================
	| Service job detail file |
	=========================*/
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
		char	cm_name[41];
		char	cm_class_type[4];
		char	cm_ch_adr1[41];
		char	cm_ch_adr2[41];
		char	cm_ch_adr3[41];
	} cumr_rec;

	int		trantyp = 0,
			prnttyp = 0,
			prn_end_client = FALSE,
			lp_no = 1;

	char	dbt_no[8],
			dbt_name[41],
			dbt_adr[3][41];	

	static char  *prtype[] ={
			"**** PREBILL INVOICE ****",
			"**** CREDIT NOTE ****",
			" ",
			"**** reprint ****"
		};
	
	FILE *fout;


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void start_report (int prnt_no);
void get_inv_dets (void);
void find_end_client (void);
void prnt_sjis (void);
void proc_sjjd (void);
void end_report (void);

    
/*========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 3)
	{
		print_at(0,0,mlSjMess706,argv[0]);
	   	return (EXIT_FAILURE);
	}

	lp_no = atoi(argv[1]);
	sprintf(sjis_rec.is_invno,"%-8.8s",argv[2]);

	OpenDB();

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	get_inv_dets();

	start_report(lp_no);

	prnt_sjis();

	end_report();
	pclose(fout);
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

	open_rec("sjis", sjis_list, sjis_no_fields, "sjis_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec("sjjd", sjjd_list, sjjd_no_fields, "sjjd_id_no");
}

/*=======================
| Close database Files. |

=======================*/
void
CloseDB (
 void)
{
	abc_fclose("sjis");
	abc_fclose("cumr");
	abc_fclose("sjhr");
	abc_fclose("sjjd");
	abc_dbclose("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
start_report (
 int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf(fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".OP\n");
	fprintf(fout,".LP%d\n",prnt_no);
	fprintf(fout,".PI12\n");
	fprintf(fout,".2\n");
	fprintf(fout,".L134\n");
	fprintf(fout,".B1\n");

}

/*===========================
| Validate and print lines. |
===========================*/
void
get_inv_dets (
 void)
{
	/*=============================
	| read invoice summary record |
	=============================*/
	strcpy(sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy(sjis_rec.is_est_no,comm_rec.test_no);
	strcpy(sjis_rec.is_dp_no,comm_rec.tdp_no);
	cc = find_rec("sjis", &sjis_rec, COMPARISON, "u");
	if (cc)
		sys_err("Error in sjis During (DBFIND)",cc, PNAME);

	/*===========================
	| test for prebill or credit |
	============================*/
	if (strcmp(sjis_rec.is_type,"C"))
		trantyp = 0;
	else
		trantyp = 1;

	/*====================
	|  test for reprint  |
	====================*/
	if(strcmp(sjis_rec.is_prt_stat,"R"))
		prnttyp = 2;
	else
		prnttyp = 3;

	/*=========================
	| Read order header record |
	==========================*/
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = sjis_rec.is_order_no;
	cc = find_rec("sjhr", &sjhr_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in sjhr During (DBFIND)",cc, PNAME);

	/*========================
	| Read charge to customer |
	=========================*/
	cc = find_hash("cumr",&cumr_rec,COMPARISON, "r",sjhr_rec.hr_chg_client);
	if (cc)
	{
		strcpy(dbt_no,"Unknown");
		sprintf(dbt_name,"%-40.40s","Unknown Customer");
		sprintf(dbt_adr[0],"%-40.40s"," ");
		sprintf(dbt_adr[1],"%-40.40s"," ");
		sprintf(dbt_adr[2],"%-40.40s"," ");
	}
	else
	{
		sprintf(dbt_no,"%-6.6s ",cumr_rec.cm_dbt_no);
		sprintf(dbt_name,"%-40.40s",cumr_rec.cm_name);
		sprintf(dbt_adr[0],"%-40.40s",cumr_rec.cm_ch_adr1);
		sprintf(dbt_adr[1],"%-40.40s",cumr_rec.cm_ch_adr2);
		sprintf(dbt_adr[2],"%-40.40s",cumr_rec.cm_ch_adr3);
	}

	/*==================================
	| Read End customer (if specified) |
	===================================*/
	if (!cc && !strcmp(cumr_rec.cm_class_type,"INT")) 
		prn_end_client = TRUE;
}

void
find_end_client (
 void)
{
	cc = find_hash("cumr",&cumr_rec,COMPARISON, "r",sjhr_rec.hr_end_client);
	if (cc)
	{
		strcpy(dbt_no,"Unknown");
		sprintf(dbt_name,"%-40.40s","Unknown Customer");
		sprintf(dbt_adr[0],"%-40.40s"," ");
		sprintf(dbt_adr[1],"%-40.40s"," ");
		sprintf(dbt_adr[2],"%-40.40s"," ");
	}
	else
	{
		sprintf(dbt_no,"%-6.6s ",cumr_rec.cm_dbt_no);
		strcpy(dbt_name,cumr_rec.cm_name);
		strcpy(dbt_adr[0],cumr_rec.cm_ch_adr1);
		strcpy(dbt_adr[1],cumr_rec.cm_ch_adr2);
		strcpy(dbt_adr[2],cumr_rec.cm_ch_adr3);
	}
}

void
prnt_sjis (
 void)
{
	/*================
	|  Print heading |
	================*/
	fprintf(fout,".B2\n");
	fprintf(fout,".ETAX INVOICE\n");
	fprintf(fout,".B1\n");

	fprintf(fout,".E%s\n",prtype[trantyp]);
	fprintf(fout,".E%s\n",prtype[prnttyp]);
	fprintf(fout,".B1\n");

	fprintf(fout,".ECOMPANY %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf(fout,".B1\n");

	fprintf(fout,"          Charge To : %-7.7s\n",dbt_no);
	fprintf(fout,"                      %-40.40s              Date :%-10.10s\n",dbt_name,DateToString(sjis_rec.is_date));
 
	fprintf(fout,"                      %-40.40s        Invoice No :%-8.8s\n",dbt_adr[0],sjis_rec.is_invno);
	fprintf(fout,"                      %-40.40s       Cust Ord No : %-10.10s \n",dbt_adr[1],sjis_rec.is_cust_ord_no);
	fprintf(fout,"                      %-40.40s    Service Ord No : %6ld \n",dbt_adr[2],sjis_rec.is_order_no);

	if (prn_end_client) 
	{
		find_end_client();

		fprintf(fout,"\n");
		fprintf(fout,"          For customer  : %-7.7s  %-40.40s \n",dbt_no,dbt_name);
		fprintf(fout,"                                   %-40.40s \n",dbt_adr[0]);
		fprintf(fout,"                                   %-40.40s \n",dbt_adr[1]);
		fprintf(fout,"                                   %-40.40s \n",dbt_adr[2]);
		fprintf(fout,"\n");
	}
	fprintf(fout,"\n");
	fprintf(fout,"          |--------------------------------------------------------------------------------------------|\n");

	proc_sjjd();
}

void
proc_sjjd (
 void)
{
	int	printed = FALSE;
	double	gst_amt = 0.00;
	double	nett_amt = 0.00;

	/*=======================
	| Process order details |
	=======================*/
	strcpy(sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy(sjjd_rec.jd_est_no,comm_rec.test_no);
	strcpy(sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;
	cc = find_rec("sjjd",&sjjd_rec,GTEQ,"r");

	fprintf(fout,"          |  Job Description :                                                                         |\n");
	while (!cc && !strcmp(sjjd_rec.jd_co_no,comm_rec.tco_no) && !strcmp(sjjd_rec.jd_est_no,comm_rec.test_no) && !strcmp(sjjd_rec.jd_dp_no,comm_rec.tdp_no) && sjjd_rec.jd_order_no == sjhr_rec.hr_order_no)
	{
		printed = TRUE;
		/*=======================
		| get description lines |
		========================*/
		fprintf(fout,"          |                    %-70.70s  |\n",sjjd_rec.jd_detail);

		cc = find_rec("sjjd",&sjjd_rec,NEXT,"r");
	}

	if (printed)
		fprintf(fout,"          |--------------------------------------------------------------------------------------------|\n");
	fprintf(fout,"          |                                                                   Amount %11.2f       |\n",sjis_rec.is_invoice_chg);

	fprintf(fout,"          |--------------------------------------------------------------------------------------------|\n");

	gst_amt = sjis_rec.is_invoice_chg * (double) sjis_rec.is_gst_pc / 100.00;
	fprintf(fout,"          |                                                                   G.S.T. %11.2f       |\n",gst_amt);
	fprintf(fout,"          |--------------------------------------------------------------------------------------------|\n");

	nett_amt = sjis_rec.is_invoice_chg + gst_amt;
	fprintf(fout,"          |                                                                    Total %11.2f       |\n",nett_amt);
	fprintf(fout,"          |--------------------------------------------------------------------------------------------|\n");
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (
 void)
{
	fprintf(fout,".EOF\n");
	if(strcmp(sjis_rec.is_status,"G") != 0)
		strcpy(sjis_rec.is_status,"I");
	strcpy(sjis_rec.is_prt_stat,"P");
	cc = abc_update("sjis",&sjis_rec);
	if (cc )
		sys_err("Error in sjis During (DBUPDATE)",cc, PNAME);
	abc_unlock("sjis");
}

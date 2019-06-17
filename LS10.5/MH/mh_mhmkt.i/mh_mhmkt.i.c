/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_mhmkt.i.c     )                               |
|  Program Desc  : ( Input Program for Machine Market Analysis Report)|
|                  ( MH11                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  mhdr, mhsd, comm, inmr, incc, insf, ccmr, cumr,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 02/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (02/07/87)      | Modified  by  : Roger Gibbison.  |
|                  (23/09/87)      | Modified  by  : Fui Choo Yap.    |
|                  (04/10/88)      | Modified  by  : Bee Chwee Lim.   |
|                  (30/06/89)      | Modified  by  : Roger Gibbison.  |
|                  (22/04/92)      | Modified  by  : Mike Davy.       |
|                  (10/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (04/09/97)      | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (15/10/1997)    | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (31/10/1997)    | Modified by : Campbell Mander.   |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|  Comments      : Change to use the display utility and add new      |
|                : search & new screen generator.                     |
|                :                                                    |
|                : (30/06/89) - General Tidy Up & Debug               |
|                :                                                    |
|  (22/04/92)    : Change index on inmr to accept a valid item no     |
|                : first time. SC 6854 CSP                            |
|                :                                                    |
|  (10/04/94)    : PSL 10673 - Online conversion                      |
|                :                                                    |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|  (15/10/1997)  : SEL Updated strings in execlp for multilingual.    |
|  (31/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
| $Log: mh_mhmkt.i.c,v $
| Revision 5.3  2002/07/17 09:57:28  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:24  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:33  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 04:51:31  gerry
| Fixed erroneous get mccf routine
|
| Revision 2.0  2000/07/15 09:01:19  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/01/20 21:48:41  cam
| Changes for GVision compatibility.  Separated descriptions from input fields.
|
| Revision 1.16  2000/01/20 21:33:09  cam
| Changes for GVision compatibility.  shutdown_prog () was being called
| indiscriminately and far too many times.
|
| Revision 1.15  2000/01/20 21:14:06  cam
| Changes for GVision compatibility.  Fix Item Description prompt, and also fix
| all print_mess () calls.
|
| Revision 1.14  1999/11/17 06:40:21  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/08 08:09:28  scott
| Updated for fix warnings due to usage of -Wall flag.
|
| Revision 1.12  1999/10/20 02:06:51  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/09/29 10:11:22  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 09:23:24  scott
| Updated from Ansi
|
| Revision 1.8  1999/06/15 03:03:05  scott
| Update for log and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_mhmkt.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_mhmkt.i/mh_mhmkt.i.c,v 5.3 2002/07/17 09:57:28 scott Exp $";

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<pr_format3.h>
#include	<ml_mh_mess.h>
#include	<ml_std_mess.h>
#define		_PSIZE	14
#define		MAXDESC		7

#define	DSP		(dp_flag == 1)
#define	PRINT	(dp_flag == 2)

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_cc_no"},
	};

	int comm_no_fields = 5;

	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char test_no[3];
		char tcc_no[3];
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
		char	cm_name[41];
		char	cm_acronym[10];
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

FILE	*fopen(const char *, const char *);
FILE	*popen(const char *, const char *);
FILE	*fin;
FILE	*fout;
int	option;
char	spec_desc[MAXDESC][16];

char	mlMhMkt [20][101];

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	systemDate[11];
	char	back[5];
	char	onight[5];
	int	lpno;
	char	device[2];
	char	deviceDesc[10];
	char	lp_str[3];
	char	model[7];
	char	item_no[17];
	char	select[2];
	char	selectDesc[23];
	char	option[2];
	long	start_date;
	long	end_date;
	char	start[11];
	char	end[11];
	char	hhbr_str[7];
	long	hhbr_hash;
	char	spec_desc[MAXDESC][41];
	char	dummy[11];
} local_rec;

int	dp_flag = 0;
int	programRun = FALSE;

static	struct	var	vars[]	={	

	{1, LIN, "device", 4, 15, CHARTYPE, 
		"U", "          ", 
		" ", "D", " Output to. ", " P(rinter D(isplay ", 
		YES, NO, JUSTLEFT, "PD", "", local_rec.device}, 
	{1, LIN, "device_desc", 4, 18, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.deviceDesc}, 
	{1, LIN, "lpno", 4, 80, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 5, 15, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 80, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "model", 7, 15, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " Model. ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.model}, 
	{1, LIN, "item_no", 8, 15, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Item Number. ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "dummy", 8, 40, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.mr_description}, 
	{1, LIN, "select", 9, 15, CHARTYPE, 
		"U", "          ", 
		" ", " ", " Selection. ", "U(nsold Machines only  Default - ALL Machines", 
		YES, NO, JUSTLEFT, "U ", "", local_rec.select}, 
	{1, LIN, "select_desc", 9, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.selectDesc}, 
	{1, LIN, "start_date", 11, 15, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, " Start Date. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start_date}, 
	{1, LIN, "end_date", 11, 80, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "31/12/99", " End Date. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================
| function prototypes |
======================*/
void shutdown_prog (void);
void InitML (void);
int run_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
int spec_valid (int field);
void display (char **argv);
void get_mhdr (void);
int valid_mhdr (void);
int dis_mhdr (void);
void sub_head (void);
void init_head (void);
int show_mhdr (char *key_val);
int heading (int scn);
void get_mccf (void);
void printout (char **argv);
void head_output (void);
int check_page (void);
int print_mhdr (void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main(
 int argc, 
 char *argv[])
{
	if (!strcmp (argv[0], "mh_dspmkt"))
		dp_flag = 1;

	if (!strcmp (argv[0], "mh_prtmkt"))
		dp_flag = 2;

	if (DSP && argc != 6)
	{
		/*-------------------------------------------------------------------
		| Usage : %s <model> <hhbr_hash> <selection> <start_date> <end_date>|
		-------------------------------------------------------------------*/
		print_at(0,0, ML(mlMhMess017), argv[0]);
		return (EXIT_FAILURE);
	}

	if (PRINT && argc != 7)
	{
		/*---------------------------------------------------------------
		|Usage : %s <lpno> <model> <hhbr_hash> <selection> <start_date>	|
		| <end_date>\007\n\r											|
		---------------------------------------------------------------*/
		print_at(0,0, ML(mlMhMess018), argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB();
	ReadMisc();

	InitML ();

	init_scr();

	set_tty();

	get_mccf();

	if (DSP)
	{
		display(argv);

		shutdown_prog();
		return (EXIT_FAILURE);
	}

	if (PRINT)
	{
		printout(argv);

		shutdown_prog();
		return (EXIT_FAILURE);
	}

	set_masks();
	init_vars(1);

	strcpy(local_rec.model, "      ");
	strcpy(local_rec.item_no,"                ");
	strcpy(local_rec.hhbr_str,"0");

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	while (prog_exit == 0)
	{
		strcpy(local_rec.lp_str,"1");
   		entry_exit = 0;
   		prog_exit = 0;
   		restart = 0;
   		search_ok = 1;
	
		init_vars(1);
		heading(1);
		entry(1);

		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		run_prog();
		prog_exit = TRUE;
	}

	if (!programRun)
		shutdown_prog ();

	return (EXIT_SUCCESS);
}

void
InitML (
 void)
{
	strcpy (mlMhMkt [1], ML ("Product"));
	strcpy (mlMhMkt [2], ML ("Serial"));
	strcpy (mlMhMkt [3], ML ("Chassis"));
	strcpy (mlMhMkt [4], ML ("Customer"));
	strcpy (mlMhMkt [5], ML ("Order"));
	strcpy (mlMhMkt [6], ML ("Date"));
	strcpy (mlMhMkt [7], ML ("Delivery"));
	strcpy (mlMhMkt [8], ML ("Value"));
	strcpy (mlMhMkt [9], ML ("SPECIFICATIONS"));
}

int
run_prog(
 void)
{
	programRun = TRUE;

	clear();
	print_at(0,0, ML(mlStdMess035) );
	fflush(stdout);

	if (!strcmp(local_rec.model,"ALL   "))
		strcpy(local_rec.model,"      ");

	if (!strcmp(local_rec.item_no,"ALL             "))
		strcpy(local_rec.hhbr_str,"0");

	if (local_rec.select [0] == ' ')
	{
		strcpy(local_rec.option," ");
		strcpy(local_rec.start,DateToString(local_rec.start_date));
		strcpy(local_rec.end,DateToString(local_rec.end_date));
	}
	else
	{
		strcpy(local_rec.option,"U");
		strcpy(local_rec.start,"00/00/00");
		strcpy(local_rec.end,"00/00/00");
	}

	shutdown_prog ();
	
	if (local_rec.device[0] == 'D')
	{
		execlp ("mh_dspmkt",
			"mh_dspmkt",
			local_rec.model,
			local_rec.hhbr_str,
			local_rec.option,
			local_rec.start,
			local_rec.end, (char *) 0);
		return (EXIT_FAILURE);
	}
	snorm();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork() == 0)
		{
			/*Machine Market Analysis Report*/ 
			execlp ("ONIGHT",
				"ONIGHT",
				"mh_prtmkt",
				local_rec.lp_str,
				local_rec.model,
				local_rec.hhbr_str,
				local_rec.option,
				local_rec.start,
				local_rec.end,
				ML(mlMhMess024), (char *) 0);
		return (EXIT_FAILURE);
		}
		return (EXIT_FAILURE);
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp ("mh_prtmkt",
				"mh_prtmkt",
				local_rec.lp_str,
				local_rec.model,
				local_rec.hhbr_str,
				local_rec.option,
				local_rec.start,
				local_rec.end, (char *) 0);
		return (EXIT_FAILURE);
	}
	execlp ("mh_prtmkt",
		"mh_prtmkt",
		local_rec.lp_str,
		local_rec.model,
		local_rec.hhbr_str,
		local_rec.option,
		local_rec.start,
		local_rec.end, (char *) 0);
	return (EXIT_FAILURE);
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

void
OpenDB(
 void)
{
	abc_dbopen("data");
	abc_alias("inmr2","inmr");

	open_rec("mhdr",mhdr_list,mhdr_no_fields,"mhdr_serial_id");
	open_rec("mhsd",mhsd_list,mhsd_no_fields,"mhsd_id_no");
	open_rec("mccf",mccf_list,mccf_no_fields,"mccf_id_no");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec("inmr2",inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec("cumr",cumr_list,cumr_no_fields,"cumr_hhcu_hash");
}

void
CloseDB(
 void)
{
	abc_fclose("mhdr");
	abc_fclose("mhsd");
	abc_fclose("mccf");
	abc_fclose("inmr");
	abc_fclose("inmr2");
	abc_fclose("cumr");
	abc_dbclose("data");
}

/*============================================ 
| Get common info from common database file. |
============================================*/
void
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
}

int
spec_valid(
 int field)
{
	if (LCHECK ("device"))
	{
		strcpy(local_rec.deviceDesc,(local_rec.device[0] == 'P') ? 
				"Printer" : "Display");

		DSP_FLD ("device_desc");
		if (prog_status == ENTRY && local_rec.device[0] == 'D')
			skip_entry = 3;
		else
			skip_entry = 0;
			
		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}
		
		if (!valid_lp (local_rec.lpno))
		{
			/*-------------------
			|  Invalid Printers |
			-------------------*/
			print_mess( ML(mlStdMess020) );
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		sprintf(local_rec.lp_str,"%d",local_rec.lpno);
		return(0);
	}

	if (LCHECK("model"))
	{
		if (strcmp(local_rec.model,"      ") == 0)
			strcpy(local_rec.model,"ALL");
		return(0);
	}

	if (LCHECK("item_no"))
	{
		if (SRCH_KEY)
		{
			show_mhdr(temp_str);
			return(0);
		}

		if (strcmp(local_rec.item_no,"                ") == 0)
		{
			strcpy(local_rec.item_no,"ALL");
			sprintf(inmr_rec.mr_description,"%40.40s"," ");
			strcpy(local_rec.hhbr_str,"0");
			display_field(field + 1);
			return(0);
		}
		else
		{
			strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
			strcpy(inmr_rec.mr_item_no,local_rec.item_no);
			cc = find_rec("inmr2",&inmr_rec,COMPARISON,"r");
			if (cc)
			{
				/*-----------------------
				| No such Item on File. |
				-----------------------*/
				print_mess( ML(mlStdMess001) );
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
		}	
		sprintf(local_rec.hhbr_str,"%ld",inmr_rec.mr_hhbr_hash);
		display_field(field + 1);
		return(0);
	}

	if (LCHECK("select"))
	{
		if  (local_rec.select[0] == 'U')
		{
			strcpy(local_rec.selectDesc,"U(nsold machines only)");
			DSP_FLD("select_desc");
			FLD("start_date")	= NA;
			FLD("end_date")		= NA;
			local_rec.start_date = 0L;
			local_rec.end_date = 0L;
			DSP_FLD("start_date");
			DSP_FLD("end_date");
		}
		else
		{
			strcpy(local_rec.selectDesc,"A(ll machines)        ");
			DSP_FLD ("select_desc");
			FLD("start_date")	= YES;
			FLD("end_date")		= YES;
		}
		return(0);
	}

	if (LCHECK ("start_date"))
	{
		if (FIELD.required == NA)
			return(0);

		if (prog_status == ENTRY)
			return(0);

		if (local_rec.start_date > local_rec.end_date)
		{
			/*------------------------------
			|Error : Start Date <= End Date|
			------------------------------*/
			print_mess( ML(mlStdMess019) );
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);
	}

	if (LCHECK ("end_date"))
	{
		if (FIELD.required == NA)
			return(0);

		if (local_rec.start_date > local_rec.end_date)
		{
			/*--------------------------------
			| Error : Start Date <= End Date |
			--------------------------------*/
			print_mess( ML(mlStdMess019) );
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);
	}
	return(0);
}

void
display(
 char **argv)
{
	/*---------------
	| Model code	|
	---------------*/
	sprintf(local_rec.model,"%-6.6s",argv[1]);

	/*---------------
	| Product hash	|
	---------------*/
	local_rec.hhbr_hash = atol(argv[2]);

	if (!strcmp(local_rec.model,"ALL   "))
		strcpy(local_rec.model,"      ");

	sprintf(local_rec.select,"%-1.1s",argv[3]);
	if (local_rec.select[0] == ' ')
	{
		local_rec.start_date = StringToDate(argv[4]);
		if (local_rec.start_date < 0L)
		{
			/*---------------------
			| Error in start_date |
			---------------------*/
			print_at(0,0, "%s\n\r", ML(mlStdMess111) ); 
			shutdown_prog();
		}
		local_rec.end_date   = StringToDate(argv[5]);
		if (local_rec.end_date < 0L)
		{
			print_at(0,0, "%s\n\r",ML(mlStdMess111) ); 
			shutdown_prog();
		}
	}
	
	/*----------------------------------------------------
	| Determine the required model and item_no selection |
	----------------------------------------------------*/
	if (strcmp(local_rec.model,"      "))
		option = (local_rec.hhbr_hash != 0L) ? 1 : 3;
	else
		option = (local_rec.hhbr_hash != 0L) ? 2 : 4;

	swide();

	get_mhdr();
}

void
get_mhdr(void)
{
	if (DSP)
		init_head();

	/*-----------------------------------
	| Initialize the mhdr record fields |
	-----------------------------------*/
	strcpy(mhdr_rec.dr_co_no,comm_rec.tco_no);
	mhdr_rec.dr_hhcc_hash = ccmr_rec.cc_hhcc_hash;
	mhdr_rec.dr_hhbr_hash = local_rec.hhbr_hash;
	strcpy(mhdr_rec.dr_serial_no,"                         ");
	cc = find_rec("mhdr",&mhdr_rec,GTEQ,"r");
	/*-------------------------------------------------------------------
	| The options determine which conditions are applicable in find_rec |
	-------------------------------------------------------------------*/
	while (!cc && valid_mhdr())
	{
		/*-----------------------------------------------------------
		| If print ALL machines,include machines with start_date <=|
		| sell_date <= end_date ;otherwise include those with       |
		| NULL sell_date					    |
		-----------------------------------------------------------*/
		if (( local_rec.select[0] == 'U' && 
		      mhdr_rec.dr_sell_date == 0L) || 
		      (local_rec.select[0] == ' ' && 
		      (mhdr_rec.dr_sell_date >= local_rec.start_date && 
		      mhdr_rec.dr_sell_date <= local_rec.end_date)))
		{
			if (DSP)
				dis_mhdr ();
			else
				print_mhdr ();
		}

		cc = find_rec("mhdr",&mhdr_rec,NEXT,"r");
	}

	if (DSP)
	{
		Dsp_srch();
		Dsp_close();
	}
}

int
valid_mhdr(void)
{
	/*---------------
	| Wrong Company	|
	---------------*/
	if (strcmp(mhdr_rec.dr_co_no,comm_rec.tco_no))
		return(0);

	/*-----------------------
	| Wrong Warehouse	|
	-----------------------*/
	if (mhdr_rec.dr_hhcc_hash != ccmr_rec.cc_hhcc_hash)
		return(0);

	switch (option)
	{
	/*-------------------------------
	| Selected Model & Product	|
	-------------------------------*/
	case	1:
		if (strcmp(mhdr_rec.dr_model_code,local_rec.model))
			return(0);

		if (mhdr_rec.dr_hhbr_hash != local_rec.hhbr_hash)
			return(0);
		break;

	/*-------------------------------
	| All Models & Selected Product	|
	-------------------------------*/
	case	2:
		if (mhdr_rec.dr_hhbr_hash != local_rec.hhbr_hash)
			return(0);
		break;

	/*-------------------------------
	| Selected Model & All Products	|
	-------------------------------*/
	case	3:
		if (strcmp(mhdr_rec.dr_model_code,local_rec.model))
			return(0);
		break;

	/*-------------------------------
	| All Models & All Products	|
	-------------------------------*/
	default:
		break;
	}
	return(1);
}

int
dis_mhdr(void)
{
	char	sell_date[11];
	char	order_date[11];
	char	env_line[150];
	int	loop;

	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",mhdr_rec.dr_hhbr_hash);
	if (cc)
		return (EXIT_SUCCESS);
	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",mhdr_rec.dr_hhcu_hash);
	if (cc)
		return (EXIT_SUCCESS);

	strcpy(sell_date,DateToString(mhdr_rec.dr_sell_date));
	strcpy(order_date,DateToString(mhdr_rec.dr_order_date));

	for (loop = 0; loop < 7; loop++)
	{
		strcpy (mhsd_rec.sd_co_no, comm_rec.tco_no);
		sprintf (mhsd_rec.sd_spec_type, "%d", loop + 1);
		sprintf (mhsd_rec.sd_code, "%-4.4s", mhdr_rec.dr_spec[loop]);
		cc = find_rec ("mhsd", &mhsd_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.spec_desc[loop], "");
		else
			strcpy (local_rec.spec_desc[loop], mhsd_rec.sd_desc);
	}

	sprintf (env_line, 
			 "%-6.6s ^E ^1 %-7.7s:  ^6 %-16.16s   %-40.40s",
			 mhdr_rec.dr_model_code,
			 mlMhMkt [1],
			 inmr_rec.mr_item_no,
			 inmr_rec.mr_description);
	Dsp_saverec (env_line);

	sprintf (env_line, 
			 "       ^E ^1 %-6.6s:   ^6 %-25.25s",
			 mlMhMkt [2],
			 mhdr_rec.dr_serial_no);
	Dsp_saverec (env_line);

	sprintf (env_line, 
			 "       ^E ^1 %-7.7s:  ^6 %-20.20s",
			 mlMhMkt [3],
			 mhdr_rec.dr_chasis_no);
	Dsp_saverec (env_line);

	sprintf (env_line, 
			 "       ^E ^1 %-8.8s: ^6 %-6.6s   %-40.40s",
			 mlMhMkt [4],
			 cumr_rec.cm_dbt_no,
			 cumr_rec.cm_name);
	Dsp_saverec (env_line);

	sprintf (env_line, 
			 "       ^E ^1 %-5.5s:    ^6   %-6.6s    ^1 %-4.4s:     ^6 %-10.10s  ^1 %-8.8s: ^6 %-10.10s  ^1 %-5.5s:    ^6 %10.2f",
			 mlMhMkt [5],
			 mhdr_rec.dr_order_no,
			 mlMhMkt [6],
			 order_date,
			 mlMhMkt [7],
			 sell_date,
			 mlMhMkt [8],
			 DOLLARS (mhdr_rec.dr_val_nzd));
	Dsp_saverec(env_line);

	sprintf (env_line,
			 "       ^E ^1 %-14.14s ^6",
			 mlMhMkt [9]);
	Dsp_saverec (env_line);

	for (loop = 0; loop < 7; loop++)
	{
		sprintf (env_line, "       ^E    %-16.16s:   %-4.4s   %-40.40s",
			spec_desc[loop],
			mhdr_rec.dr_spec[loop],
			local_rec.spec_desc[loop]);
		Dsp_saverec (env_line);
	}
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");
	return (EXIT_FAILURE);
}

void
sub_head(void)
{
	char	today[11];

	clear();
	/*--------------------------------
	| MACHINE MARKET ANALYSIS DISPLAY |
	--------------------------------*/
	sprintf (err_str, " %s ", ML(mlMhMess003) );
	rv_pr( err_str, 50,0,1);

	/*--------------- 
	| Date: %10.10s |
	--------------- */
	print_at(0,116, ML(mlMhMess001), DateToString (TodaysDate ()));

	move(0,1);
	line(132);

	sprintf(err_str," %s ",clip(comm_rec.tco_name));
	rv_pr(err_str,(132 - strlen(err_str)) / 2,2,1);
	/*-----------------
	| UNSOLD MACHINES |
	-----------------*/
	if (local_rec.select[0] == 'U')
		rv_pr( ML(mlMhMess004), 58,3,1);
	else
	{
		/*---------------------------- 
		| ALL MACHINES FROM %s TO %s |
		----------------------------*/
		strcpy(today,DateToString(local_rec.start_date));
		sprintf(err_str, ML(mlMhMess005),today,DateToString(local_rec.end_date));
		rv_pr(err_str,(132 - strlen(err_str)) / 2,3,1);
	}
}

void
init_head(void)
{

	sub_head ();
	Dsp_open (0, 4, _PSIZE);
	Dsp_saverec ("MODEL  |                                                                                                                         ");
	Dsp_saverec ("");
	Dsp_saverec("    [Redraw]  [Next Screen]  [Previous Screen]  [Input/End]    ");
}

int
show_mhdr(
 char *key_val)
{
    work_open();
	save_rec("#Product Code","#Product Description");
	abc_selfield("inmr","inmr_id_no");
	abc_selfield("mhdr","mhdr_hhbr_hash");
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",key_val);
	cc = find_rec("inmr2",&inmr_rec,GTEQ,"r");
        while (!cc && !strcmp(inmr_rec.mr_co_no,comm_rec.tco_no) && !strncmp(inmr_rec.mr_item_no,key_val,strlen(key_val)))
    	{
		cc = find_hash("mhdr",&mhdr_rec,COMPARISON,"r",inmr_rec.mr_hhbr_hash);
		if (!cc)
		{
			cc = save_rec(inmr_rec.mr_item_no,inmr_rec.mr_description);
			if (cc)
				break;
		}
		cc = find_rec("inmr2",&inmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return (EXIT_SUCCESS);
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",temp_str);
	cc = find_rec("inmr2",&inmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in inmr During (DBFIND)",cc,PNAME);

	abc_selfield("inmr","inmr_hhbr_hash");
	abc_selfield("mhdr","mhdr_serial_id");
	return (EXIT_FAILURE);
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading(
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		swide();
		clear();

		/*----------------------------------------
		|  Machine Selection For Market Analysis |
		----------------------------------------*/
		sprintf (err_str, " %s ", ML(mlMhMess006) );
		rv_pr( err_str, 45,0,1);
		move(0,1);
		line(132);

		box(0,3,132,8);

		move(1,6);
		line(131);

		move(1,10);
		line(131);

		move(0,20);
		line(132);
		print_at(21,0, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_FAILURE);
}

void
get_mccf(
 void)
{
	int	i = 1;

	strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
	strcpy(mccf_rec.cf_spec_type,"1");
	cc = find_rec("mccf",&mccf_rec,GTEQ,"r");
	while ((!cc && !strcmp(mccf_rec.cf_co_no,comm_rec.tco_no)) && i <= MAXDESC)
	{
		sprintf(spec_desc[i - 1],"%-15.15s",mccf_rec.cf_spec_desc);
		i++;
		cc = find_rec("mccf",&mccf_rec,NEXT,"r");
	}
}

void
printout(
 char **argv)
{
	/*---------------
	| Printer No.	|
	---------------*/
	local_rec.lpno = atoi (argv[1]);

	/*---------------
	| Model code	|
	---------------*/
	sprintf (local_rec.model, "%-6.6s", argv[2]);

	/*---------------
	| Product hash	|
	---------------*/
	local_rec.hhbr_hash = atol (argv[3]);

	if (!strcmp (local_rec.model, "ALL   "))
		strcpy (local_rec.model, "      ");

	sprintf (local_rec.select, "%-1.1s", argv[4]);
	if (local_rec.select[0] == ' ')
	{
		local_rec.start_date = StringToDate (argv[5]);
		if (local_rec.start_date < 0L)
		{
			print_at (0,0, "%s\n\r", ML(mlStdMess111) );
			shutdown_prog();
		}
		local_rec.end_date = StringToDate (argv[6]);
		if (local_rec.end_date < 0L)
		{
			print_at (0,0, "%s\n\r", ML(mlStdMess111) );
			shutdown_prog();
		}
	}
	
	/*----------------------------------------------------
	| Determine the required model and item_no selection |
	----------------------------------------------------*/
	if (strcmp (local_rec.model, "      "))
		option = (local_rec.hhbr_hash != 0L) ? 1 : 3;
	else
		option = (local_rec.hhbr_hash != 0L) ? 2 : 4;

	swide ();

	dsp_screen ("Processing : Printing Machine Market Analysis.", comm_rec.tco_no, comm_rec.tco_name);

	if ((fin = pr_open ("mh_mktrep.p")) == NULL)
		sys_err ("Error in opening mh_mktrep.p during (FOPEN)", errno, PNAME);

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)", errno, PNAME);

	head_output ();

	get_mhdr ();

	pr_format (fin, fout, "END_FILE", 0, 0);
	pclose (fout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output(
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".16\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L160\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.tco_name));
	fprintf (fout, ".EMACHINE MARKET ANALYSIS\n");
	if (local_rec.select[0] == 'U')
		fprintf (fout, ".EUNSOLD MACHINES\n");
	else
	{
		fprintf (fout, ".EALL MACHINES PERIOD FROM %s ", DateToString (local_rec.start_date));
		fprintf (fout, "TO %s\n", DateToString (local_rec.end_date));
	}
	fprintf (fout, ".B1\n");

	fprintf (fout, "NOTE: 1 = %s ;", spec_desc[0]);
	fprintf (fout, "2 = %s ;", spec_desc[1]);
	fprintf (fout, "3 = %s ;", spec_desc[2]);
	fprintf (fout, "4 = %s ;\n", spec_desc[3]);
	fprintf (fout, "5 = %s ;", spec_desc[4]);
	fprintf (fout, "6 = %s ;", spec_desc[5]);
	fprintf (fout, "7 = %s \n", spec_desc[6]);

	fprintf (fout, "SC = sales category of customer\n");
	fprintf (fout, "SA = sales area of customer\n");
	fprintf (fout, "Rep = salesperson (representative)\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".R=======");
	fprintf (fout, "=================");
	fprintf (fout, "==========================");
	fprintf (fout, "=======");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "====");
	fprintf (fout, "===");
	fprintf (fout, "=====\n");

	fprintf (fout, "=======");
	fprintf (fout, "=================");
	fprintf (fout, "==========================");
	fprintf (fout, "=======");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "====");
	fprintf (fout, "===");
	fprintf (fout, "=====\n");

	fprintf (fout, "|MODEL ");
	fprintf (fout, "| PRODUCT        ");
	fprintf (fout, "| SERIAL                  ");
	fprintf (fout, "|CHASIS");
	fprintf (fout, "|MF/PU");
	fprintf (fout, "| 1  ");
	fprintf (fout, "| 2  ");
	fprintf (fout, "| 3  ");
	fprintf (fout, "| 4  ");
	fprintf (fout, "| 5  ");
	fprintf (fout, "| 6  ");
	fprintf (fout, "| 7  ");
	fprintf (fout, "|CUST. ");
	fprintf (fout, "| DELIVERY ");
	fprintf (fout, "| ORDER");
	fprintf (fout, "|O/DATE");
	fprintf (fout, "| VALUE    ");
	fprintf (fout, "|INV NO");
	fprintf (fout, "|SC ");
	fprintf (fout, "|SA");
	fprintf (fout, "|REP|\n");

	fprintf (fout, "|------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|-------------------------");
	fprintf (fout, "|------");
	fprintf (fout, "|-----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------");
	fprintf (fout, "|------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------");
	fprintf (fout, "|---");
	fprintf (fout, "|--");
	fprintf (fout, "|---|\n");

	fflush (fout);
}

int
check_page(
 void)
{
	return(0);
}

int
print_mhdr(
 void)
{
	char	mfg_date[11];
	char	ord_date[11];

	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",mhdr_rec.dr_hhbr_hash);
	if (cc)
		return (EXIT_SUCCESS);

	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",mhdr_rec.dr_hhcu_hash);
	if (cc)
		return (EXIT_SUCCESS);

	dsp_process(" Serial No: ",mhdr_rec.dr_serial_no);

	pr_format(fin,fout,"CHECK_PAGE",0,0);
	pr_format(fin,fout,"LINE_ONE",1,mhdr_rec.dr_model_code);
	pr_format(fin,fout,"LINE_ONE",2,inmr_rec.mr_item_no);
	pr_format(fin,fout,"LINE_ONE",3,mhdr_rec.dr_serial_no);
	pr_format(fin,fout,"LINE_ONE",4,mhdr_rec.dr_chasis_no);
	strcpy(ord_date,DateToString(mhdr_rec.dr_mfg_pur_date));
	sprintf(mfg_date,"%5.5s",ord_date + 3);
	pr_format(fin,fout,"LINE_ONE",5,mfg_date);
	pr_format(fin,fout,"LINE_ONE",6,mhdr_rec.dr_spec[0]);
	pr_format(fin,fout,"LINE_ONE",7,mhdr_rec.dr_spec[1]);
	pr_format(fin,fout,"LINE_ONE",8,mhdr_rec.dr_spec[2]);
	pr_format(fin,fout,"LINE_ONE",9,mhdr_rec.dr_spec[3]);
	pr_format(fin,fout,"LINE_ONE",10,mhdr_rec.dr_spec[4]);
	pr_format(fin,fout,"LINE_ONE",11,mhdr_rec.dr_spec[5]);
	pr_format(fin,fout,"LINE_ONE",12,mhdr_rec.dr_spec[6]);
	pr_format(fin,fout,"LINE_ONE",13,cumr_rec.cm_dbt_no);
	pr_format(fin,fout,"LINE_ONE",14,DateToString(mhdr_rec.dr_sell_date));
	pr_format(fin,fout,"LINE_ONE",15,mhdr_rec.dr_order_no);
	sprintf (ord_date, "%-10.10s", DateToString(mhdr_rec.dr_order_date));
	pr_format(fin,fout,"LINE_ONE",16,ord_date);
	pr_format(fin,fout,"LINE_ONE",17,mhdr_rec.dr_val_nzd);
	pr_format(fin,fout,"LINE_ONE",18,mhdr_rec.dr_inv_no);
	pr_format(fin,fout,"LINE_ONE",19,mhdr_rec.dr_cust_type);
	pr_format(fin,fout,"LINE_ONE",20,mhdr_rec.dr_cust_area);
	pr_format(fin,fout,"LINE_ONE",21,mhdr_rec.dr_rep_no);

	pr_format(fin,fout,"LINE_TWO",1,inmr_rec.mr_description);
	pr_format(fin,fout,"LINE_TWO",2,cumr_rec.cm_name);

	fflush(fout);
	return (EXIT_FAILURE);
}

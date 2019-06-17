/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_ivrpr.i.c                                     |
|  Program Desc  : ( Reprint Service Invoices/Prebills/Credits    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,    ,     ,     ,     ,     ,          |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 02/10/87         |
|---------------------------------------------------------------------|
|  Date Modified : 29/11/88        | Modified By : B.C.Lim.           |
|  Date Modified : (18/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/10/97)      | Modified  by  : Elizabeth D. Paid|
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|                : (18/09/90) - General Update for New Scrgen. S.B.D. |
|  (16/10/97)    :    SEL - change the length of field sjis_invno,    |
|                :          sjig_invno from 6 to 8.                   |
|                                                                     |
| $Log: sj_ivrpr.i.c,v $
| Revision 5.3  2002/07/17 09:57:50  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:33  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:22  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:43  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/17 06:40:48  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/16 05:58:32  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.7  1999/09/29 10:12:59  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:06:35  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/20 02:30:30  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ivrpr.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ivrpr.i/sj_ivrpr.i.c,v 5.3 2002/07/17 09:57:50 scott Exp $";

#include <pslscr.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>
#include <get_lpno.h>

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
		int		termno;
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
		{"sjhr_cust_type"},
		{"sjhr_issue_date"},
	};

	int sjhr_no_fields = 8;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_type[2];
		long	hr_issue_date;
	} sjhr_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_class_type"},
	};

	int cumr_no_fields = 7;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
	} cumr_rec;

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
	};

	int sjis_no_fields = 6;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
	} sjis_rec;

	char	systemDate[11],
			branchNo[3],
			prog_title[41];

	int		envDbCo = 0,
			envDbFind = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11],
			invno[9],
			c_client[7],
  	        e_client[7],
			chg_client[41],
  	        end_client[41],
			order_no[9],
			prtr_no[3],
			stat_desc[21],
	        job_detail[7][71];
	int	lp_no;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "invoice_no", 4, 18, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", " ", "Invoice No.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.invno}, 
	{1, LIN, "inv_date", 5, 18, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Invoice date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjis_rec.is_date}, 
	{1, LIN, "order_no", 6, 18, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "Order no ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "charge_to", 8, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge to ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 9, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 10, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End client ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 11, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{1, LIN, "prtr_no", 13, 18, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer no ", " ", 
		YES, NO, JUSTLEFT, "1234567890", "", (char *)&local_rec.lp_no}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include <FindCumr.h>

/*=====================
| Function Prototypes |
======================*/
void  shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void inv_srch (char *key_val);
int heading (int scn);
int spec_valid (int field);
int get_chg_client (void);
int get_end_client (void);
int reprint_sjis (char *prog_name);

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
		print_at (0,0, mlStdMess037,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (prog_title,"%-40.40s",argv[2]);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);


	/*===================
	|  get branchNolishment |
	====================*/

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));


	OpenDB ();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);
	strcpy (systemDate, DateToString (TodaysDate()));

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

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		if (reprint_sjis (argv[1]))
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}

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
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec("sjis", sjis_list, sjis_no_fields, "sjis_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields,(envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("cumr");
	abc_fclose("sjis");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("invoice_no"))
	{
		if (SRCH_KEY)
		{
			inv_srch (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
		strcpy (sjis_rec.is_est_no,comm_rec.test_no);
		strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);
		strcpy (sjis_rec.is_invno,zero_pad(local_rec.invno, 8));
		cc = find_rec ("sjis",&sjis_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess115));
			return (EXIT_FAILURE);
		}
		strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		sjhr_rec.hr_order_no = sjis_rec.is_order_no;
		cc = find_rec ("sjhr",&sjhr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess122));
			return (EXIT_FAILURE);
		}
		abc_selfield ("cumr","cumr_hhcu_hash");
		get_chg_client ();
		get_end_client ();
		abc_selfield ("cumr",(envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");

		scn_display (1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK("end_client"))
	{
		if (FLD ("end_client") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no,branchNo);
		strcpy (cumr_rec.cm_dbt_no,pad_num(local_rec.e_client));
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.end_client,cumr_rec.cm_name);
		DSP_FLD ("end_client");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prtr_no"))
	{
		if (SRCH_KEY)
		{
			local_rec.lp_no = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lp_no))
		{
			errmess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
get_chg_client (
 void)
{
	cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_chg_client);
	if (cc)
	{
		errmess (ML(mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.cm_name);
	if (strcmp (cumr_rec.cm_class_type,"INT") !=  0 )
	{
		strcpy (sjhr_rec.hr_cust_type,"E");
		FLD ("end_client")	= NA;
	}
	else
	{
		strcpy (sjhr_rec.hr_cust_type ,"I");
		FLD ("end_client")	= YES;
	}
	return (EXIT_SUCCESS);
}

int
get_end_client (
 void)
{
	if (sjhr_rec.hr_end_client == 0L)
		return (EXIT_SUCCESS);

	cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_end_client);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.e_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.end_client,cumr_rec.cm_name);
	return (EXIT_SUCCESS);
}

int
reprint_sjis (
char *prog_name)
{
	sprintf (local_rec.prtr_no,"%2d",local_rec.lp_no);

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	*(arg) = prog_name;
	*(arg+(1)) = local_rec.prtr_no;
	*(arg+(2)) = sjis_rec.is_invno;
	*(arg+(3)) = (char *)0;
	shell_prog (3);
	return(0);
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
inv_srch (
 char *key_val)
{
	work_open ();
	strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no,comm_rec.test_no);
	strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);
	sprintf (sjis_rec.is_invno,"%-8.8s",key_val);
	save_rec ("#Invoice.","#Date");
	cc = find_rec ("sjis",&sjis_rec,GTEQ,"r");

	while (!cc && !strcmp (sjis_rec.is_co_no,comm_rec.tco_no) && 
				  !strcmp (sjis_rec.is_est_no,comm_rec.test_no) && 
				  !strcmp (sjis_rec.is_dp_no,comm_rec.tdp_no) && 
				  strncmp (sjis_rec.is_invno,key_val,strlen(key_val)) == 0)
	{ 
		strcpy (err_str, DateToString(sjis_rec.is_date));
		cc = save_rec (sjis_rec.is_invno, err_str);
		if (cc)
			break;

		cc = find_rec ("sjis", &sjis_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no,comm_rec.test_no);
	strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);
	strcpy (sjis_rec.is_invno,temp_str);
	cc = find_rec ("sjis",&sjis_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in sjis During (DBFIND)", cc, PNAME);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (prog_title, (80 - strlen(clip(prog_title))) / 2,0,1);
		move (0,1);
		line (80);

		if (scn == 1)
			box (0,3,80,10);

		move (1,7);
		line (79);
		move (1,12);
		line (79);

		move (0,20);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.test_no,comm_rec.test_name);
		
		move (0,23);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


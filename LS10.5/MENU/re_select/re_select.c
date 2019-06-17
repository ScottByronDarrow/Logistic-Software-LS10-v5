/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (re_select.c  )                                    |
|  Program Desc  : (Company / Branch Select.                    )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, esmr, comr, ccmr,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/04/88         |
|---------------------------------------------------------------------|
|  Date Modified : (07/04/88)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (14/06/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/05/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (21/08/92)      | Modified  by  : Trevor van Bremen|
|  Date Modified : (06/11/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (19/10/93)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (19/10/93)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (24/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (12/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (14/06/89) - Changed to update comm for terminal   |
|                :              zero if console changed.              |
|                : (16/05/91) - Updated to clean up + new co/br close |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|  (06/11/92)    : PSL 8060 copy comr_env_name to comm_env_name       |
|  (18/10/93)    : HGP 9501 Allow for up to 9 price descriptions.     |
|  (24/05/96)    : Updated to fix problems in DateToString. 		  |
|  (12/09/97)    : Incorporate multilingual conversion.      	      |
|  (03/09/1999)  : Ported to ANSI standards.                	      |
|                                                                     |
| $Log: re_select.c,v $
| Revision 5.2  2001/08/09 05:13:52  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:02  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:34  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/06/05 22:37:42  scott
| Updated to more read_comm as not in correct place.
|
| Revision 1.16  2000/05/16 04:26:59  scott
| S/C LSDI-2904 / ASL-16320
| Updated to master branch to company master file.
| Currently master branch is held in an environment variable CO_MST_BR and has to be changed if multiple companies exist with different master branches.
| Change applies to "all_select.c" "re_select.c" "co_maint.c" "modsel.c" and
| effects the schema file "sch.comr".
|
| Revision 1.15  1999/12/10 04:10:58  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.14  1999/12/06 01:47:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/10/26 05:55:18  scott
| Updated from ansi testing
|
| Revision 1.12  1999/10/16 04:56:36  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.11  1999/09/29 10:11:20  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:27:16  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/15 02:36:56  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: re_select.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/re_select/re_select.c,v 5.2 2001/08/09 05:13:52 scott Exp $";

#include 	<pslscr.h>
#include	<license2.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

#define	MASTER		 (!strcmp (masterBranch, esmr_rec.br_no))

#define	CO_DBT		 (companyClose[0] == '1')
#define	CO_CRD		 (companyClose[1] == '1')
#define	CO_INV		 (companyClose[2] == '1')
#define	CO_PAY		 (companyClose[3] == '1')
#define	CO_GEN		 (companyClose[4] == '1')
#define	ALL_COMPANY	 (CO_DBT && CO_CRD && CO_INV && CO_PAY && CO_GEN)
#define	LOCK_COMR 	 (ALL_COMPANY || MASTER)
#define LOCK_ESMR	 (!ALL_COMPANY)

	char	companyClose [6];
	char	masterBranch [3];
	long	workDates [5];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dp_short"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_payroll_date"},
		{"comm_gl_date"},
		{"comm_closed_period"},
		{"comm_fiscal"},
		{"comm_gst_rate"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
		{"comm_pay_terms"},
		{"comm_env_name"},
		{"comm_stat_flag"}
	};

	int comm_no_fields = 33;

	struct {
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tco_short [16];
		char	tes_no [3];
		char	tes_name [41];
		char	tes_short [16];
		char	tcc_no [3];
		char	tcc_name [41];
		char	tcc_short [10];
		char	tdp_no [3];
		char	tdp_name [41];
		char	tdp_short [16];
		long	tdbt_date;
		long	tcrd_date;
		long	tinv_date;
		long	tpay_date;
		long	tgl_date;
		int		tclosed_period;
		int		tfiscal;
		float	tgst_rate;
		char	tprice [9][16];
		int		tpay_terms;
		char	env_name [61];
		char	tstat_flag [2];
	} comm_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list [] = {
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_short_name"},
		{"comr_master_br"},
		{"comr_fiscal"},
		{"comr_dbt_date"},
		{"comr_crd_date"},
		{"comr_inv_date"},
		{"comr_payroll_date"},
		{"comr_gl_date"},
		{"comr_closed_period"},
		{"comr_gst_rate"},
		{"comr_int_rate"},
		{"comr_price1_desc"},
		{"comr_price2_desc"},
		{"comr_price3_desc"},
		{"comr_price4_desc"},
		{"comr_price5_desc"},
		{"comr_price6_desc"},
		{"comr_price7_desc"},
		{"comr_price8_desc"},
		{"comr_price9_desc"},
		{"comr_pay_terms"},
		{"comr_env_name"},
		{"comr_stat_flag"}
	};

	int comr_no_fields = 25;

	struct {
		char	co_no [3];
		char	co_name [41];
		char	co_short [16];
		char	master_br [3];
		int		fiscal;
		long	dbt_date;
		long	crd_date;
		long	inv_date;
		long	pay_date;
		long	gl_date;
		int		closed_period;
		float	gst_rate;
		float	int_rate;
		char	price [9][16];
		int		pay_terms;
		char	env_name [61];
		char	stat_flag [2];
	} comr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list [] = {
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
		{"esmr_dbt_date"},
		{"esmr_crd_date"},
		{"esmr_inv_date"},
		{"esmr_pay_date"},
		{"esmr_gl_date"},
		{"esmr_stat_flag"}
	};

	int esmr_no_fields = 10;

	struct {
		char	co_no [3];
		char	br_no [3];
		char	br_name [41];
		char	short_name [16];
		long	dbt_date;
		long	crd_date;
		long	inv_date;
		long	pay_date;
		long	gl_date;
		char	stat_flag [2];
	} esmr_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list [] = {
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 6;

	struct {
		char	cm_co_no [3];
		char	cm_br_no [3];
		char	cm_wh_no [3];
		char	cm_name [41];
		char	cm_acronym [10];
		char	cm_stat_flag [2];
	} ccmr_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list [] = {
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
		{"cudp_dp_short"},
		{"cudp_location"},
		{"cudp_stat_flag"}
	};

	int cudp_no_fields = 7;

	struct {
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	dp_name [41];
		char	dp_short [16];
		char	location [41];
		char	stat_flag [2];
	} cudp_rec;

	char	systemDate [11];

	char	*data	= "data";
	char	*comm	= "comm";
	char	*comr	= "comr";
	char	*esmr	= "esmr";
	char	*ccmr	= "ccmr";
	char	*cudp	= "cudp";

/*============================
| Local function prototypes  |
============================*/
void	OpenDB		 (void);
void	CloseDB	 	 (void);
void	Update		 (char *, char *, char *, char *);
void	SetupComm	 (void);
void	WriteComm	 (int);
int		OpenComm	 (void);
long	ChangeDate	 (long);

#include 	<write_comm.h>

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;
	char	co_no [3];
	char	br_no [3];
	char	cc_no [3];
	char	dp_no [3];

	if (argc != 1 && argc != 4 && argc != 5)
	{
		print_at (0,0,ML (mlMenuMess091),argv[0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (companyClose,"%-5.5s","11111");
	else
		sprintf (companyClose,"%-5.5s",sptr);

	sptr = chk_env ("CO_MST_BR");
	if (sptr == (char *)0)
		strcpy (masterBranch, " 1");
	else
		sprintf (masterBranch, "%-2.2s", sptr);

	sprintf (dp_no,"%2.2s", (argc == 5) ? argv[4] : " ");

	strcpy (systemDate, DateToString (TodaysDate ()));

	OpenDB ();

	if (argc == 1)
	{
		read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
		ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, FALSE);
		strcpy (co_no,comm_rec.tco_no);
		strcpy (br_no,comm_rec.tes_no);
		strcpy (cc_no,comm_rec.tcc_no);
	}
	else
	{
		sprintf (co_no,"%2.2s",argv[1]);
		sprintf (br_no,"%2.2s",argv[2]);
		sprintf (cc_no,"%2.2s",argv[3]);
	}
	strcpy (comr_rec.co_no, co_no);
	cc = find_rec ("comr", &comr_rec, COMPARISON, "r");
	if (cc)
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}
	if (strcmp (comr_rec.master_br, "  "))
		sprintf (masterBranch, comr_rec.master_br);

	Update (co_no,br_no,cc_no,dp_no);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open Data Base Files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (comm,comm_list,comm_no_fields,"comm_term");
	open_rec (comr,comr_list,comr_no_fields,"comr_co_no");
	open_rec (esmr,esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec ("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");
	open_rec (cudp,cudp_list,cudp_no_fields,"cudp_id_no");
}

/*========================
| Close Data Base Files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (comm);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose ("ccmr");
	abc_fclose (cudp);
	abc_dbclose (data);
}

void
Update (
 char *	co_no,
 char *	br_no,
 char *	wh_no,
 char *	dp_no)
{
	sprintf (comr_rec.co_no,"%2.2s",co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON, (LOCK_COMR) ? "u" : "r");
	if (cc)
	{
		sprintf (err_str,"Company %2.2s not on file",co_no);
		sys_err (err_str,cc,PNAME);
	}

	sprintf (esmr_rec.co_no,"%2.2s",co_no);
	sprintf (esmr_rec.br_no,"%2.2s",br_no);
	cc = find_rec (esmr,&esmr_rec,COMPARISON, (LOCK_ESMR) ? "u" : "r");
	if (cc)
	{
		sprintf (err_str,"Company / Branch %2.2s/%2.2s not on file",co_no,br_no);
		sys_err (err_str,cc,PNAME);
	}

	sprintf (ccmr_rec.cm_co_no,"%2.2s",co_no);
	sprintf (ccmr_rec.cm_br_no,"%2.2s",br_no);
	sprintf (ccmr_rec.cm_wh_no,"%2.2s",wh_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
	{
		sprintf (err_str,"Company / Branch / Warehouse %2.2s/%2.2s/%2.2s not on file",co_no,br_no,wh_no);
		sys_err (err_str,cc,PNAME);
	}

	/*-------------------------------
	| Department is not blanks	|
	-------------------------------*/
	if (strcmp (dp_no,"  "))
	{
		sprintf (cudp_rec.co_no,"%2.2s",co_no);
		sprintf (cudp_rec.br_no,"%2.2s",br_no);
		sprintf (cudp_rec.dp_no,"%2.2s",dp_no);
		cc = find_rec (cudp,&cudp_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,"Company / Branch / Department %2.2s/%2.2s/%2.2s not on file",co_no,br_no,dp_no);
			sys_err (err_str,cc,PNAME);
		}
	}
	else
	{
		sprintf (cudp_rec.dp_no,"%-2.2s"," ");
		sprintf (cudp_rec.dp_name,"%-40.40s"," ");
		sprintf (cudp_rec.dp_short,"%-15.15s"," ");
	}

	workDates[0] = (CO_DBT) ? ChangeDate (comr_rec.dbt_date) :
					   		 ChangeDate (esmr_rec.dbt_date);

	workDates[1] = (CO_CRD) ? ChangeDate (comr_rec.crd_date) :
					   		 ChangeDate (esmr_rec.crd_date);
			
	workDates[2] = (CO_INV) ? ChangeDate (comr_rec.inv_date) :
					   		 ChangeDate (esmr_rec.inv_date);
			
	workDates[3] = (CO_GEN) ? ChangeDate (comr_rec.gl_date) :
					   	 	 ChangeDate (esmr_rec.gl_date);

	workDates[4] = (CO_PAY) ? ChangeDate (comr_rec.pay_date) :
						     ChangeDate (esmr_rec.pay_date);
	comm_rec.termno = ttyslt ();

    cc = find_rec (comm, &comm_rec, COMPARISON, "u");

	SetupComm ();

	if (cc)
	{
		comm_rec.termno = ttyslt ();
		cc = abc_add (comm,&comm_rec);
		if (cc) 
			file_err (cc, "comm", "DBADD");
	}
	else
	{
		cc = abc_update (comm,&comm_rec);
		if (cc) 
			file_err (cc, "comm", "DBUPDATE");
	}
	abc_unlock ("comm");

	WriteComm (comm_rec.termno);

	if (comm_rec.termno == 1)
	{
		comm_rec.termno = 0;
		cc = find_rec ("comm",&comm_rec, COMPARISON, "u");
		SetupComm ();
		if (cc)
			cc = abc_add ("comm",&comm_rec);
		else
			cc = abc_update ("comm",&comm_rec);

		if (cc)
			file_err (cc, "comm", "DBADD/UPDATE");

		WriteComm (comm_rec.termno);
	}

	if (LOCK_COMR)
	{
		cc = abc_update ("comr",&comr_rec);
		if (cc) 
			file_err (cc, "comr", "DBUPDATE");
	}

	if (LOCK_ESMR)
	{
		cc = abc_update ("esmr",&esmr_rec);
		if (cc) 
			file_err (cc, "esmr", "DBUPDATE");
	}
	else
		abc_unlock ("esmr");
}

void
SetupComm (
 void)
{
	comr_rec.dbt_date = workDates [0];
	comr_rec.crd_date = workDates [1];
	comr_rec.inv_date = workDates [2];
	comr_rec.gl_date  = workDates [3];
	comr_rec.pay_date = workDates [4];

	esmr_rec.dbt_date = workDates [0];
	esmr_rec.crd_date = workDates [1];
	esmr_rec.inv_date = workDates [2];
	esmr_rec.gl_date  = workDates [3];
	esmr_rec.pay_date = workDates [4];

	strcpy (comm_rec.tco_no,comr_rec.co_no);
	strcpy (comm_rec.tco_name,comr_rec.co_name);
	strcpy (comm_rec.tco_short,comr_rec.co_short);

	if (strcmp (comr_rec.env_name, "                                                            "))
		strcpy (comm_rec.env_name,comr_rec.env_name);
	else
		sprintf (comm_rec.env_name, "%s/BIN/LOGISTIC", getenv ("PROG_PATH"));

	strcpy (comm_rec.tes_no,esmr_rec.br_no);
	strcpy (comm_rec.tes_name,esmr_rec.br_name);
	strcpy (comm_rec.tes_short,esmr_rec.short_name);
	strcpy (comm_rec.tcc_no,ccmr_rec.cm_wh_no);
	strcpy (comm_rec.tcc_name,ccmr_rec.cm_name);
	strcpy (comm_rec.tcc_short,ccmr_rec.cm_acronym);

	comm_rec.tdbt_date = (CO_DBT) ? comr_rec.dbt_date :
					esmr_rec.dbt_date;

	comm_rec.tcrd_date = (CO_CRD) ? comr_rec.crd_date :
					esmr_rec.crd_date;

	comm_rec.tinv_date = (CO_INV) ? comr_rec.inv_date :
					esmr_rec.inv_date;

	comm_rec.tpay_date = (CO_PAY) ? 0L : 0L;

	comm_rec.tgl_date =  (CO_GEN) ? comr_rec.gl_date :
					esmr_rec.gl_date;

	comm_rec.tclosed_period = comr_rec.closed_period;
	comm_rec.tfiscal = comr_rec.fiscal;
	comm_rec.tgst_rate = comr_rec.gst_rate;
	strcpy (comm_rec.tprice[0],comr_rec.price[0]);
	strcpy (comm_rec.tprice[1],comr_rec.price[1]);
	strcpy (comm_rec.tprice[2],comr_rec.price[2]);
	strcpy (comm_rec.tprice[3],comr_rec.price[3]);
	strcpy (comm_rec.tprice[4],comr_rec.price[4]);
	strcpy (comm_rec.tprice[5],comr_rec.price[5]);
	strcpy (comm_rec.tprice[6],comr_rec.price[6]);
	strcpy (comm_rec.tprice[7],comr_rec.price[7]);
	strcpy (comm_rec.tprice[8],comr_rec.price[8]);
	comm_rec.tpay_terms = comr_rec.pay_terms;
	strcpy (comm_rec.tstat_flag,"0");
}

/*=======================================================================
|									|
=======================================================================*/
long
ChangeDate (
 long cur_date)
{
	int		s_month;
	int		c_month;
	int		s_year;
	int		c_year;
	long	t_dte;

	t_dte = StringToDate (systemDate);
	DateToDMY (t_dte, NULL, &s_month, &s_year);
	DateToDMY (cur_date, NULL, &c_month, &c_year);
	
	if (s_month != c_month || s_year != c_year)
		return (cur_date);

	if (cur_date < StringToDate (systemDate))
		return (StringToDate (systemDate));

	return (cur_date);
}

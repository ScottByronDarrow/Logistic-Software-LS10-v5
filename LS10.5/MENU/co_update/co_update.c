/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (co_update.c  )                                    |
|  Program Desc  : (Maintain Company Details in comm.           )     |
|                 (                                            )      |
|---------------------------------------------------------------------|
|  Access files  :  comm, comr,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (28/02/89)      | Modified  by  : Roger Gibbison   |
|  Date Modified : (28/08/1999)    | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      :                                                    |
| (28/08/1999)  : Ported to ANSI format.                              |
|                :                                                    |
| $Log: co_update.c,v $
| Revision 5.1  2001/08/09 05:13:21  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:08:08  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:53  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:06  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/10 04:10:53  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.7  1999/12/06 01:47:10  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/10/26 05:43:54  scott
| Updated from ansi project
|
| Revision 1.5  1999/09/29 10:11:02  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:53  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: co_update.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/co_update/co_update.c,v 5.1 2001/08/09 05:13:21 scott Exp $";

#include 	<pslscr.h>

#define	CO_DBT		 (envCoClose[0] == '1')
#define	CO_CRD		 (envCoClose[1] == '1')
#define	CO_INV		 (envCoClose[2] == '1')
#define	CO_PAY		 (envCoClose[3] == '1')
#define	CO_GEN		 (envCoClose[4] == '1')
#define	ALL_COMPANY	 (CO_DBT && CO_CRD && CO_INV && CO_PAY && CO_GEN)

	char	envCoClose[ 6 ];
	char	companyNumber[3],
			branchNumber[3],
			warehouseNumber[3];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
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
		{"comm_pay_terms"},
		{"comm_stat_flag"}
	};

	int comm_no_fields = 28;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_name[41];
		char	tes_short[16];
		char	tcc_no[3];
		char	tcc_name[41];
		char	tcc_short[10];
		char	tdp_no[3];
		char	tdp_name[41];
		char	tdp_short[16];
		long	tdbt_date;
		long	tcrd_date;
		long	tinv_date;
		long	tpay_date;
		long	tgl_date;
		int		tclosed_period;
		int		tfiscal;
		float	tgst_rate;
		char	tprice[5][16];
		int		tpay_terms;
		char	tstat_flag[2];
	} comm_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_short_name"},
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
		{"comr_pay_terms"},
		{"comr_stat_flag"}
	};

	int comr_no_fields = 19;

	struct {
		char	mr_co_no[3];
		char	mr_co_name[41];
		char	mr_co_short[16];
		int		mr_fiscal;
		long	mr_dbt_date;
		long	mr_crd_date;
		long	mr_inv_date;
		long	mr_pay_date;
		long	mr_gl_date;
		int		mr_closed_period;
		float	mr_gst_rate;
		float	mr_int_rate;
		char	mr_price[5][16];
		int		mr_pay_terms;
		char	mr_stat_flag[2];
	} comr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
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
		char	es_co_no[3];
		char	es_br_no[3];
		char	es_br_name[41];
		char	es_short_name[16];
		long	es_dbt_date;
		long	es_crd_date;
		long	es_inv_date;
		long	es_pay_date;
		long	es_gl_date;
		char	es_stat_flag[2];
	} esmr_rec;

	long	workDates[5];

/*========================== 
| Function prototypes.     |
==========================*/
void	shutdown_prog	 (void);
int		OpenComm		 (void);
void	WriteComm		 (int);
int		ReadMisc		 (int);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	update			 (void);
void	SetCommValues	 (void);


#include 	<write_comm.h>

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	OpenDB ();

	update ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===========================
| Shutdown program routine. |
===========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
int
ReadMisc (
 int	ttyno)
{
	comm_rec.termno	=	ttyno;
	return (find_rec ("comm", &comm_rec, COMPARISON, "u"));
}

/*=======================
| Open Data Base Files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec ("comm",comm_list,comm_no_fields,"comm_term");
	open_rec ("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec ("comr",comr_list,comr_no_fields,"comr_co_no");
}

/*========================
| Close Data Base Files. |
========================*/
void
CloseDB (void)
{
	abc_fclose ("comm");
	abc_fclose ("comr");
	abc_fclose ("esmr");
	abc_dbclose ("data");
}

void
update (void)
{
	int	ttyno = ttyslt ();

	cc = ReadMisc (ttyno);
	if (cc)
		file_err (cc, "comm", "DBFIND");

	abc_unlock ("comm");

	strcpy (companyNumber,comm_rec.tco_no);
	strcpy (branchNumber,comm_rec.tes_no);
	strcpy (warehouseNumber,comm_rec.tcc_no);

	sprintf (comr_rec.mr_co_no,"%2.2s",companyNumber);
	cc = find_rec ("comr",&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	sprintf (esmr_rec.es_co_no,"%2.2s",companyNumber);
	sprintf (esmr_rec.es_br_no,"%2.2s",branchNumber);
	cc = find_rec ("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	ttyno = 0;

	cc = ReadMisc (ttyno++);

	while (!cc)
	{
		if (!strcmp (comm_rec.tco_no,companyNumber) && 
		     !strcmp (comm_rec.tes_no,branchNumber))
		{
			SetCommValues ();

			cc = abc_update ("comm",&comm_rec);
			if (cc) 
				file_err (cc, "comm", "DBUPDATE");

			WriteComm (comm_rec.termno);
		}

		abc_unlock ("comm");

		cc = ReadMisc (ttyno++);
	}
}

void
SetCommValues (void)
{

	workDates[0] = (CO_DBT) ? comr_rec.mr_dbt_date : esmr_rec.es_dbt_date;
	workDates[1] = (CO_CRD) ? comr_rec.mr_crd_date : esmr_rec.es_crd_date;
	workDates[2] = (CO_INV) ? comr_rec.mr_inv_date : esmr_rec.es_inv_date;
	workDates[3] = (CO_GEN) ? comr_rec.mr_gl_date  : esmr_rec.es_gl_date;
	workDates[4] = (CO_PAY) ? comr_rec.mr_pay_date : esmr_rec.es_pay_date;

	comr_rec.mr_dbt_date = workDates[0];
	comr_rec.mr_crd_date = workDates[1];
	comr_rec.mr_inv_date = workDates[2];
	comr_rec.mr_gl_date  = workDates[3];
	comr_rec.mr_pay_date = workDates[4];

	esmr_rec.es_dbt_date = workDates[0];
	esmr_rec.es_crd_date = workDates[1];
	esmr_rec.es_inv_date = workDates[2];
	esmr_rec.es_gl_date  = workDates[3];
	esmr_rec.es_pay_date = workDates[4];

	strcpy (comm_rec.tco_no,comr_rec.mr_co_no);
	strcpy (comm_rec.tco_name,comr_rec.mr_co_name);
	strcpy (comm_rec.tco_short,comr_rec.mr_co_short);
	strcpy (comm_rec.tes_no,esmr_rec.es_br_no);
	strcpy (comm_rec.tes_name,esmr_rec.es_br_name);
	strcpy (comm_rec.tes_short,esmr_rec.es_short_name);

	comm_rec.tdbt_date = (CO_DBT) ? comr_rec.mr_dbt_date : esmr_rec.es_dbt_date;
	comm_rec.tcrd_date = (CO_CRD) ? comr_rec.mr_crd_date : esmr_rec.es_crd_date;
	comm_rec.tinv_date = (CO_INV) ? comr_rec.mr_inv_date : esmr_rec.es_inv_date;
	comm_rec.tpay_date = (CO_PAY) ? 0L : 0L;
	comm_rec.tgl_date = (CO_GEN) ? comr_rec.mr_gl_date : esmr_rec.es_gl_date;

	comm_rec.tclosed_period = comr_rec.mr_closed_period;
	comm_rec.tfiscal = comr_rec.mr_fiscal;
	comm_rec.tgst_rate = comr_rec.mr_gst_rate;
	strcpy (comm_rec.tprice[0],comr_rec.mr_price[0]);
	strcpy (comm_rec.tprice[1],comr_rec.mr_price[1]);
	strcpy (comm_rec.tprice[2],comr_rec.mr_price[2]);
	strcpy (comm_rec.tprice[3],comr_rec.mr_price[3]);
	strcpy (comm_rec.tprice[4],comr_rec.mr_price[4]);
	comm_rec.tpay_terms = comr_rec.mr_pay_terms;
	strcpy (comm_rec.tstat_flag,"0");
}

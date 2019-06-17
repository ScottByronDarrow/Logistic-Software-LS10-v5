/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_tranwin.c   )                                 |
|  Program Desc  : ( Display Stock transactions Display.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr,     ,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (28/12/87)      | Modified by : Scott B. Darrow.   |
|  Date Modified : (14/11/88)      | Modified by : B.C.Lim.           |
|  Date Modified : (16.06.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (11/09/97)      | Modified by : Roanna Marcelino   |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|       16.06.94 : Changed to use pslscr                              |
|                :                                                    |
|                                                                     |
| $Log: sk_tranwin.c,v $
| Revision 5.1  2001/08/09 09:20:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:18:14  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:33  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:12:19  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/10/13 02:42:20  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.6  1999/10/12 21:20:47  scott
| Updated by Gerry from ansi project.
|
| Revision 1.5  1999/10/08 05:33:01  scott
| First Pass checkin by Scott.
|
| Revision 1.4  1999/06/20 05:20:57  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_tranwin.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_tranwin/sk_tranwin.c,v 5.1 2001/08/09 09:20:23 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>

	char	wk_line[81];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
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
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_payroll_date"},
		{"comm_gl_date"}
	};

	int comm_no_fields = 15;
	
	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char tco_short[16];
		char tes_no[3];
		char tes_name[41];
		char tes_short[16];
		char tcc_no[3];
		char tcc_name[41];
		char tcc_short[10];
		long t_dbt_date;
		long t_crd_date;
		long t_inv_date;
		long t_pay_date;
		long t_gl_date;
	} comm_rec;

	/*==============================
	| Inventory Transactions File. |
	==============================*/
	struct dbview intr_list[] ={
		{"intr_co_no"},
		{"intr_br_no"},
		{"intr_hhbr_hash"},
		{"intr_hhcc_hash"},
		{"intr_type"},
		{"intr_date"},
		{"intr_ref1"},
		{"intr_ref2"},
		{"intr_qty"},
		{"intr_cost_price"},
		{"intr_sale_price"},
		{"intr_stat_flag"}
	};

	int intr_no_fields = 12;

	struct {
		char	tr_co_no[3];
		char	tr_br_no[3];
		long	tr_hhbr_hash;
		long	tr_hhcc_hash;
		int	tr_type;
		long	tr_date;
		char	tr_ref1[11];
		char	tr_ref2[11];
		float	tr_qty;
		double	tr_cost_price;		/*  Money field  */
		double	tr_sale_price;		/*  Money field  */
		char	tr_stat_flag[2];
	} intr_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 5;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_cc_no[3];
		long	cm_hhcc_hash;
		char	cm_stat_flag[2];
	} ccmr_rec;

	long	wk_hash;

	char	wk_str[131];

	float	tot_qty = 0.00;
	double	tot_amount = 0.00;


/*=======================
| Function Declarations |
=======================*/
void proc_file (void);
void Dsp_save (void);
void OpenDB (void);
void CloseDB (void);


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
		print_at(0,0,mlSkMess572, argv[0]);
		return (EXIT_FAILURE);
	}

	wk_hash = atol(argv[1]);
	OpenDB();
	init_scr();
	crsr_off();
	set_tty();
	proc_file();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
proc_file (
 void)
{
	Dsp_open(0,8,8);

	Dsp_saverec("BR|WH|   REF1   |   REF2   |TRAN TYPE|   DATE   |  QTY.  |   VALUE.  ");
	Dsp_saverec("");
	Dsp_saverec("[F14-Next Screen] [F15-Previous Screen] [F16-Input/End]");
	cc = find_hash("intr", &intr_rec, GTEQ, "r", wk_hash);
	while(!cc && intr_rec.tr_hhbr_hash == wk_hash)
	{
	    cc = find_hash("ccmr",&ccmr_rec,EQUAL,"r",intr_rec.tr_hhcc_hash);
	    Dsp_save();
	    cc = find_hash("intr", &intr_rec, NEXT, "r", wk_hash);
	}
	sprintf(wk_str,"           ****** T O T A L S ******            ^E%8.2f^E%11.2f",
		tot_qty,
		DOLLARS(tot_amount));

	Dsp_saverec(wk_str);
	Dsp_srch();
	Dsp_close();
}

void
Dsp_save (
 void)
{

	float	qty = 0.00;
	double	amount = 0.00;

	static char *tr_type[] = {
		"SUPP BACK","STK RECPT"," STK ISS "," STK BAL ",
		" STK PUR "," INVOICE "," CREDIT  ","?????????"
	};

	switch (intr_rec.tr_type)
	{
	    case 1:
	    case 2:
	    case 4:
	    case 5:
	    case 7:
	     	qty = intr_rec.tr_qty;
	     	amount = (intr_rec.tr_cost_price * intr_rec.tr_qty);
	     	break;

	    case 3:
	    case 6:
	     	qty = intr_rec.tr_qty;
	     	amount = (intr_rec.tr_cost_price * intr_rec.tr_qty);
		qty *= -1;
		amount *= -1;
	     	break;
	}
	tot_qty += qty;
	tot_amount += amount;

	
	sprintf(wk_str,"%2.2s^E%2.2s^E%10.10s^E%10.10s^E%9.9s^E%10.10s^E%8.2f^E%11.2f",
			intr_rec.tr_br_no,
			ccmr_rec.cm_cc_no,
			intr_rec.tr_ref1,
			intr_rec.tr_ref2,
			tr_type[intr_rec.tr_type - 1],
			DateToString(intr_rec.tr_date),
			qty,
			DOLLARS(amount));

	Dsp_saverec(wk_str);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec("intr", intr_list, intr_no_fields, "intr_hhbr_hash");
	open_rec("ccmr", ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("intr");
	abc_fclose("ccmr");
	abc_dbclose ("data");
}

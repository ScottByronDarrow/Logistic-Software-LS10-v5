/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( qt_enqy_prt.c )                                  |
|  Program Desc  : ( Print Enquiry List by Customer.) 				  |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, qthr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Carlos Escarrilla.| Date Written: 15/06/1995       |
|---------------------------------------------------------------------|
|  Date Modified : 27/12/1995 		 | Modified By : Elena B. Cuaresma|
|  Date Modified : 05/09/1997	 	 | Modified By : Jiggs A Veloz.   |
|  Date Modified : 29/10/1997 		 | Modified By : Elena B. Cuaresma|
|  Date Modified : 20/08/1999 		 | Modified By : Alvin Misalucha  |
|  Date Modified : 21/01/2000        | Modified By : Vij A. Blones, Jr|
|                :                                                    |
|  Comments      : 27/12/95 - Updated to add date options in printing.|
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|  (29/10/1997)  : SEL Changed quote number length from 6 to 8.       |
|                :     Code checked and tested the program.           |
|                :                                                    |
|  (20/08/1999)  : Ported to ANSI standard.                           |
|  (21/01/2000)  : Fixed problem related to calling edit() without    |
|                : first calling heading()/scn_display()              |
|                :                                                    |
|  (03/02/2000)  : Fixed sort file problem.  added calls to cleanup   |
|                : the sort file properly after use.                  |
|                :                                                    |
| $Log: enqy_prt.c,v $
| Revision 5.4  2002/07/17 09:57:42  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/26 08:09:03  robert
| updated to make default value for start range an empty string
|
| Revision 5.2  2001/08/09 08:44:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:29  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:08:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/02/07 05:11:49  scott
| Updated to move sort_delete as in wrong place. Found by Trevor during Linux port
|
| Revision 1.13  2000/02/03 03:30:32  vij
| added code to cleanup the sort file.
|
| Revision 1.12  2000/01/21 05:21:44  vij
| added calls to heading()/scn_display() before edit()
|
| Revision 1.11  1999/11/16 03:29:18  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.10  1999/10/20 02:07:01  nz
| Updated for final changes on date routines.
|
| Revision 1.9  1999/09/29 10:12:30  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/22 05:18:25  scott
| Updated from Ansi project.
|
| Revision 1.7  1999/09/13 09:20:21  alvin
| Converted to ANSI format.
|
| Revision 1.6  1999/06/18 06:12:22  scott
| Updated to add log for cvs and remove old style read_comm()
|
|				 :													  |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: enqy_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_enqy_prt/enqy_prt.c,v 5.4 2002/07/17 09:57:42 scott Exp $";

#include <pslscr.h>
#include <std_decs.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <qt_status.h>
#include <twodec.h>
#include <ml_qt_mess.h>
#include <ml_std_mess.h>


#define		BLANK_DATE    (local_rec.st_enqy_date == 0L && local_rec.end_enqy_date == 0L)
#define		BLANK_QTNO    (!strcmp(local_rec.st_quote_no, "        ") && !strcmp(local_rec.end_quote_no, "        ")) 	

#define		BLANK_QTSTAT  (!strcmp(local_rec.st_quote_stat, "  ") && !strcmp(local_rec.end_quote_stat, "  ")) 

#define		BLANK_SMAN    (!strcmp(local_rec.st_sman_no, "  ") && !strcmp(local_rec.end_sman_no, "  "))  	

#define		BY_SALESMAN		(local_rec.sort_type [0] == 'S') 

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int  	envDbFind = 0;
	char	branchNo[3];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	int comm_no_fields = 5;
	
	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	test_no[3];
		char 	test_name[41];
	} comm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_sman_code"},
	};

	int	cumr_no_fields = 7;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_sman[7];
	} cumr_rec;

	/*========================
	| External Salesman File |
	========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int	exsf_no_fields = 3;

	struct	{
		char	co_no[3];
		char	salesman_no[3];
		char	salesman[41];
	} exsf_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	52

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_quote_src"},
		{"qthr_cont_no"},
		{"qthr_hhcu_hash"},
		{"qthr_hhqt_hash"},
		{"qthr_enq_ref"},
		{"qthr_op_id"},
		{"qthr_time_create"},
		{"qthr_date_create"},
		{"qthr_expire_date"},
		{"qthr_dt_fst_call"},
		{"qthr_dt_lst_call"},
		{"qthr_dt_follow_up"},
		{"qthr_dt_quote"},
		{"qthr_no_calls"},
		{"qthr_carr_code"},
		{"qthr_carr_area"},
		{"qthr_no_kgs"},
		{"qthr_pri_type"},
		{"qthr_sman_code"},
		{"qthr_sell_terms"},
		{"qthr_pay_term"},
		{"qthr_freight"},
		{"qthr_sos"},
		{"qthr_exch_rate"},
		{"qthr_fix_exch"},
		{"qthr_cont_name"},
		{"qthr_cont_phone"},
		{"qthr_pos_code"},
		{"qthr_del_name"},
		{"qthr_del_add1"},
		{"qthr_del_add2"},
		{"qthr_del_add3"},
		{"qthr_del_add4"},
		{"qthr_comm1"},
		{"qthr_comm2"},
		{"qthr_comm3"},
		{"qthr_salute"},
		{"qthr_status"},
		{"qthr_stat_flag"},
		{"qthr_place_ord"},
		{"qthr_reas_code"},
		{"qthr_reas_desc"},
		{"qthr_comp_code"},
		{"qthr_comp_name"},
		{"qthr_dbt_name"},
		{"qthr_del_date"},
		{"qthr_qt_value"},
		{"qthr_qt_profit_cur"},
		{"qthr_qt_profit_pc"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		char	quote_src [3];
		char	cont_no [7];
		long	hhcu_hash;
		long	hhqt_hash;
		char	enq_ref [21];
		char	op_id [15];
		char	time_create [6];
		long	date_create;
		long	expire_date;
		long	dt_fst_call;
		long	dt_lst_call;
		long	dt_follow_up;
		long	dt_quote;
		int		no_calls;
		char	carr_code [5];
		char	carr_area [3];
		float	no_kgs;
		char	pri_type [2];
		char	sman_code [3];
		char	sell_terms [4];
		char	pay_term [41];
		double	freight;
		char	sos [2];
		double	exch_rate;
		char	fix_exch [2];
		char	cont_name [21];
		char	cont_phone [16];
		char	pos_code [4];
		char	del_name [41];
		char	del_add1 [41];
		char	del_add2 [41];
		char	del_add3 [41];
		char	del_add4 [41];
		char	comm1 [21];
		char	comm2 [21];
		char	comm3 [21];
		char	salute [41];
		char	status [3];
		char	stat_flag [2];
		char	place_ord [2];
		char	reas_code [4];
		char	reas_desc [31];
		char	comp_code [4];
		char	comp_name [31];
		char	dbt_name [41];
		long	del_date;
		double	qt_value;
		double	qt_profit_cur;
		float	qt_profit_pc;
	}	qthr_rec;


	/*=========================+
	| Quotation detail lines. |
	+=========================*/
	#define	QTLN_NO_FIELDS	25

	struct dbview	qtln_list [QTLN_NO_FIELDS] =
	{
		{"qtln_hhqt_hash"},
		{"qtln_line_no"},
		{"qtln_hhbr_hash"},
		{"qtln_hhcc_hash"},
		{"qtln_serial_no"},
		{"qtln_qty"},
		{"qtln_gsale_price"},
		{"qtln_sale_price"},
		{"qtln_cost_price"},
		{"qtln_disc_pc"},
		{"qtln_reg_pc"},
		{"qtln_disc_a"},
		{"qtln_disc_b"},
		{"qtln_disc_c"},
		{"qtln_tax_pc"},
		{"qtln_gst_pc"},
		{"qtln_cumulative"},
		{"qtln_pri_or"},
		{"qtln_dis_or"},
		{"qtln_item_desc"},
		{"qtln_exp_date"},
		{"qtln_stat_flag"},
		{"qtln_cont_status"},
		{"qtln_st_flag"},
		{"qtln_alt_flag"},
	};

	struct tag_qtlnRecord
	{
		long	hhqt_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		char	serial_no [26];
		float	qty;
		double	gsale_price;		/* money */
		double	sale_price;		/* money */
		double	cost_price;		/* money */
		float	dis_pc;
		float	reg_pc;
		float	disc_a;
		float	disc_b;
		float	disc_c;
		float	tax_pc;
		float	gst_pc;
		int		cumulative;
		char	pri_or [2];
		char	dis_or [2];
		char	item_desc [41];
		long	exp_date;
		char	stat_flag [2];
		int		cont_status;
		char	st_flag [2];
		char	alt_flag [2];
	}	qtln_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	2

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_hhbr_hash"},
		{"inmr_outer_size"}
	};

	struct tag_inmrRecord
	{
		long	hhbr_hash;
		float	outer_size;
	}	inmr_rec;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	long	st_enqy_date;
	long	end_enqy_date;
	char	st_date[11];
	char	end_date[11];
	char	st_quote_no[9];
	char	end_quote_no[9];
	char	st_quote_stat[3];
	char	end_quote_stat[3];
	char	st_qstat_name[41];
	char	end_qstat_name[41];
	char	st_sman_no[3];
	char	end_sman_no[3];
	char	st_sman_name[41];
	char	end_sman_name[41];
	char	sort_type[2];
	char	back[4];
	char	onight[4];
	int		lpno;
	char	lp_str[3];
	char 	dummy[11];
	char	date_type[2];
	char	st_cust_no[7];
	char	end_cust_no[7];
	char	st_cust_name[41];
	char	end_cust_name[41];
	char	sort_desc[21];
} local_rec;

static struct	var vars[] ={
	{1, LIN, "st_enqy_date",	4, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", " Start Date            :", "Enter starting date.",
		YES, NO, JUSTLEFT, "", "", (char  *)&local_rec.st_enqy_date},
	{1, LIN, "end_enqy_date",5, 25, EDATETYPE,
		"DD/DD/DD","          ",
		" ", " ", " End   Date            :", "Enter ending date.",
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.end_enqy_date},
	{1, LIN, "date_type",	  4, 75, CHARTYPE,
		"U", "          ",
		" ", " ", " Date Selection Type (Q,E,F,X) :", "Q-uote Date, E-nquiry Date, F-ollow-up Date, X-Expiry Date.",
		 YES, NO,  JUSTLEFT, "QEFX", "", local_rec.date_type},
	{1, LIN, "st_quote_no",	7, 25, CHARTYPE,
		"UUUUUUUU","          ",
		" ", "        ", " Start quote number    :", "Enter starting quote number.",
		NO, NO, JUSTLEFT, "", "", local_rec.st_quote_no},
	{1, LIN, "end_quote_no", 8, 25, CHARTYPE,
		"UUUUUUUU","          ",
		" ", "~~~~~~~~", " End   quote number    :", "Enter ending quote number.",	
		NO, NO, JUSTLEFT, "", "", local_rec.end_quote_no},
	{1, LIN, "st_quote_stat", 7,75, CHARTYPE,
		"NN", "          ",
		" ", "00", " Start quote status            :", "Enter starting quote status.",
		NO, NO, JUSTLEFT, "", "", local_rec.st_quote_stat},
	{1, LIN, "st_qstat_name", 7,85, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.st_qstat_name},
	{1, LIN, "end_quote_stat", 8,75, CHARTYPE,
		"NN", "          ",
		" ", "99", " End   quote status            :", "Enter ending quote status.",
		NO, NO, JUSTLEFT, "", "", local_rec.end_quote_stat},
	{1, LIN, "end_qstat_name", 8,85, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.end_qstat_name}, 
	{1, LIN, "st_cust_no",	10, 25, CHARTYPE,
		"UUUUUU","          ",
		" ", "      ", " Start Customer Number :", "Enter starting customer number.",
		NO, NO, JUSTLEFT, "", "", local_rec.st_cust_no},
	{1, LIN, "st_cust_name", 10,42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.st_cust_name}, 
	{1, LIN, "end_cust_no", 11, 25, CHARTYPE,
		"UUUUUU","          ",
		" ", "~~~~~~", " End   Customer Number :", "Enter ending customer number.",	
		NO, NO, JUSTLEFT, "", "", local_rec.end_cust_no},
	{1, LIN, "end_cust_name", 11,42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.end_cust_name}, 
	{1, LIN, "st_sman_no",	13, 25, CHARTYPE,
		"AA", "          ",
		" ", " ", " Start Salesman        :", "Enter starting salesman.",
		NO, NO, JUSTLEFT, "", "", local_rec.st_sman_no},
	{1, LIN, "st_sman_name", 13, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.st_sman_name}, 
	{1, LIN, "end_sman_no",	14, 25, CHARTYPE,
		"AA", "          ",
		" ", "~~", " End   Salesman        :", "Enter ending salesman.",
		NO, NO, JUSTLEFT, "", "", local_rec.end_sman_no},
	{1, LIN, "end_sman_name",14, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.end_sman_name}, 

	{1, LIN, "sort_type",	16, 25, CHARTYPE,
		"U","          ",
		" ", "S", " Sort type             :", "S=Salesman, T=Status",
		YES, NO, JUSTLEFT, "", "", local_rec.sort_type}, 
	{1, LIN, "sort_desc",16, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA","          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.sort_desc}, 

	{1, LIN, "lpno",			17, 25, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No            :", "",
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",			17, 25 ,CHARTYPE,
		"U", "          ",
		" ", "", " Background            :", "",
		ND, NO, JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "onight",		17, 75, CHARTYPE,
		"U", "          ",
		" ", "", " Overnight                     :", "",
		ND, NO, JUSTLEFT, "", "", local_rec.onight},

	{0, LIN, "",				0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};


FILE	*fout,
		*fsort;

char	*data    = "data",
    	*comm    = "comm",
    	*cumr    = "cumr",
    	*qthr    = "qthr",
    	*qtln    = "qtln",
    	*exsf    = "exsf",
    	*inmr    = "inmr",
		*sptr;

char	prev_sman [3],
		curr_sman [3],
		prev_status [3],
		curr_status [3],
		desc [41];

int		first_time = TRUE;

/*==========================
| Function prototypes      |
==========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	run_prog		(char *, char *);
int		process			(void);
void	process_fsort	(void);
void	proc_bysman		(void);
void	proc_bystatus	(void);
void	find_status		(char *);
int		head_output		(void);
int		heading			(int);
void	quote_search	(char *);
void	status_search	(char *);
void	sman_search		(char *);

#include <FindCumr.h>

	int		envVarDbCo		=	0;
	char	branchNumber[3];
/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;

	SETUP_SCR (vars);
	init_scr();
	set_tty();
	clear();
	swide();
	set_masks();
	init_vars(1);
	OpenDB();

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit= 0;
		edit_exit = 0;
		prog_exit = 0;
		search_ok = 1;
		init_ok	  = 1;
		restart = 0;
		init_vars(1);

		local_rec.lpno = 1;
		strcpy(local_rec.back, "No ");
		strcpy(local_rec.onight, "No ");
		strcpy(local_rec.sort_type, "S");

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		while (!restart && !prog_exit)
		{
			init_vars(1);
			heading(1);
			scn_display(1);
			entry(1);
			if (!restart && !prog_exit)
			{
				heading (1);
				scn_display (1);
				edit(1); 
				if (!restart)
				{
					snorm();
					dsp_screen("Processing Enquiry Report List", comm_rec.tco_no, comm_rec.tco_name);
					process();
					swide();
				}
			}
			if (restart)
				restart = 0;
		}

		rset_tty();

		prog_exit = 1;

	/* 	run_prog(argv[0], argv[1]);  */

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog  (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec(qtln, qtln_list, QTLN_NO_FIELDS, "qtln_id_no");
	open_rec(inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec(cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec(exsf, exsf_list, exsf_no_fields, "exsf_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_dbclose(data);
	abc_fclose(qthr);
	abc_fclose(qtln);
	abc_fclose(inmr);
	abc_fclose(cumr);
	abc_fclose(exsf);
}

int
spec_valid (
 int	field)
{
	int i;
	int found = FALSE;

	if (LCHECK("st_enqy_date"))
	{
		if (dflt_used)
			return(0);

		if (local_rec.st_enqy_date > TodaysDate ())
		{
			/*----------------------------------------------------------
			| Start enquiry date must not be GREATER THAN today's date |
			----------------------------------------------------------*/
			print_mess( ML(mlQtMess009) );
			sleep(1);
			clear_mess();
			return(1);
		}
		if (prog_status != ENTRY &&
			local_rec.st_enqy_date > local_rec.end_enqy_date)
		{
			/*---------------------------------------------------------------
			| Start enquiry date must not be GREATER THAN last enquiry date |
			---------------------------------------------------------------*/
			print_mess( ML(mlQtMess010) );
			sleep(1);
			clear_mess();
			return(1);
		}

		DSP_FLD("st_enqy_date");
		return(0);
	}
					
	if (LCHECK("end_enqy_date"))
	{
		if (dflt_used)
		{
			local_rec.end_enqy_date = TodaysDate ();
			DSP_FLD("end_enqy_date");
			return(0);
		} 

		if (local_rec.end_enqy_date < local_rec.st_enqy_date)
		{
			/*----------------------------------------------------------
			| End enquiry date must no be LESS THAN first enquiry date |
			----------------------------------------------------------*/
			print_mess( ML(mlQtMess011) );
			sleep(1);
			clear_mess();
			return(1);
		}
		DSP_FLD("end_enqy_date");
		return(0);
	}
	/*---------------------
	| Validate Date Type. |
	----------------------*/
	if (LCHECK("date_type"))
	{
		if (!strcmp(local_rec.date_type, " "))
		{
			/*--------------------------------
			| Invalid Date Selection Type ...|
			--------------------------------*/
			print_err ( ML(mlQtMess017) );
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("date_type");
		return(0);
	}

	if (LCHECK("st_quote_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.st_quote_no, "        ");
			DSP_FLD("st_quote_no");
			return(0);
		}

		if (SRCH_KEY)
		{
			quote_search(temp_str);
			return(0);
		}

		if (prog_status != ENTRY && 
			strcmp(local_rec.st_quote_no, local_rec.end_quote_no) > 0)
 
		{
			/*-----------------------------------------------------------
			| Start quote number must NOT be GREATER THAN last quote no |
			-----------------------------------------------------------*/
			print_mess( ML(mlStdMess017) );
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(qthr_rec.co_no, comm_rec.tco_no);
		strcpy(qthr_rec.br_no, comm_rec.test_no);
		strcpy(qthr_rec.quote_no, zero_pad(local_rec.st_quote_no,8));
		cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
		if (cc)
		{
			/*------------------------
			| Quote Number Not Found |
			------------------------*/
			print_mess( ML(mlStdMess152) );
			sleep(2);
			clear_mess();
			return(1);
		}
		DSP_FLD("st_quote_no");
		return(0);
	}

	if (LCHECK("end_quote_no"))
	{
		if (dflt_used)
			return(0);

		if (SRCH_KEY)
		{
			quote_search(temp_str);
			return(0);
		}
		if (strcmp(local_rec.st_quote_no, local_rec.end_quote_no) > 0)
 
		{
			/*-------------------------------------------------------
			| End quote number must NOT be LESS THAN first quote no |
			-------------------------------------------------------*/
			print_mess( ML(mlStdMess018) );
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(qthr_rec.co_no, comm_rec.tco_no);
		strcpy(qthr_rec.br_no, comm_rec.test_no);
		strcpy(qthr_rec.quote_no, zero_pad(local_rec.end_quote_no,8));
		cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
		if (cc)
		{
			/*------------------------
			| Quote Number Not Found |
			------------------------*/
			print_mess( ML(mlStdMess152) );
			sleep(2);
			clear_mess();
			return(1);
		}
		DSP_FLD("end_quote_no");
		return(0);
	}

	/*-------------------------------
	| Validate Start Customer Code. |
	--------------------------------*/
	if (LCHECK("st_cust_no")) 
	{
		if (dflt_used)
		{
			sprintf(local_rec.st_cust_no,"%-6.6s","      ");
			sprintf(local_rec.st_cust_name, "%-40.40s", "Customer");

			FLD("end_cust_no") = NA;

			DSP_FLD("st_cust_no");
			DSP_FLD("end_cust_no");
			DSP_FLD("st_cust_name");
			DSP_FLD("end_cust_name");
			return(0);
		}
		FLD("end_cust_no") = YES;
		
		if (prog_status != ENTRY && 
		    strncmp(local_rec.end_cust_no,"      ",6) && 
		    strcmp(local_rec.st_cust_no,local_rec.end_cust_no) > 0)
		{
			/*---------------------------------
			| End must be greater than start. |
			---------------------------------*/
			print_err( ML(mlStdMess018) );
			sleep(2);
			return(1);
		}


		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		abc_selfield (cumr, "cumr_id_no");
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNumber);
		strcpy(cumr_rec.cm_dbt_no,local_rec.st_cust_no);
		cc = find_rec(cumr, &cumr_rec,COMPARISON,"r");	
		if (cc)
		{
			/*-------------------------
			| Customer No. not found. |
			-------------------------*/
			print_err( ML(mlStdMess021) );
			sleep(2);
			abc_selfield (cumr, "cumr_hhcu_hash");
			return(1);
		}
		strcpy(local_rec.st_cust_no, cumr_rec.cm_dbt_no);
		strcpy(local_rec.st_cust_name, cumr_rec.cm_name );
		DSP_FLD("st_cust_name");
		DSP_FLD("st_cust_no");
		abc_selfield (cumr, "cumr_hhcu_hash");
		return(0);
	}


 	/*========================= 
  	| Validate Customer code  | 
  	==========================*/
	if (LCHECK("end_cust_no"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.end_cust_no,"%-6.6s","~~~~~~");
			sprintf(local_rec.end_cust_name, "%-40.40s", " ");
			DSP_FLD("end_cust_no");
			DSP_FLD("end_cust_name");
			return(0);
		}

		if (last_char == SEARCH)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		abc_selfield (cumr, "cumr_id_no");
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNumber);
		strcpy(cumr_rec.cm_dbt_no,local_rec.end_cust_no);
		cc = find_rec( cumr, &cumr_rec,COMPARISON,"r");	
		if (cc)
		{
			/*-------------------------
			| Customer No. not found. |
			-------------------------*/
			print_err( ML(mlStdMess021) );
			sleep(2);
			abc_selfield (cumr, "cumr_hhcu_hash");
			return(1);
		}
		strcpy(local_rec.end_cust_no, cumr_rec.cm_dbt_no);
		strcpy(local_rec.end_cust_name, cumr_rec.cm_name);

		if (strcmp(local_rec.st_cust_no,local_rec.end_cust_no) > 0)
		{
			/*-------------------------------------------
			| End Customer must not be less than start. |
			-------------------------------------------*/
			errmess( ML(mlStdMess018) );
			sleep(2);
			abc_selfield (cumr, "cumr_hhcu_hash");
			return(1);
		}
		DSP_FLD("end_cust_name");
		DSP_FLD("end_cust_no");
		abc_selfield (cumr, "cumr_hhcu_hash");
		return(0);
	}

	if (LCHECK("st_quote_stat"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_qstat_name, "First quote status");
			DSP_FLD("st_qstat_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			status_search(temp_str);
			return(0);
		}
			
		if (prog_status != ENTRY &&
			strcmp(local_rec.st_quote_stat, local_rec.end_quote_stat) > 0)
 
		{
			/*---------------------------------------------------------------
			| Start quote status must NOT be GREATER THAN last quote status |
			---------------------------------------------------------------*/
			print_mess( ML(mlStdMess017) );
			sleep(1);
			clear_mess();
			return(1);
		}
		for (i=0; i<12; i++)
		{
			if (!strcmp(local_rec.st_quote_stat, q_status[i]._stat))
			{
				found = TRUE;
				strcpy(local_rec.st_qstat_name, q_status[i]._desc);
				break;
			}
		}
		if (!found)
		{
			/*------------------
			| Status not found |
			------------------*/
			found = FALSE;
			print_mess( ML(mlStdMess205) );
			sleep(1);
			clear_mess();
			return(1);
		}

		DSP_FLD("st_quote_stat");
		DSP_FLD("st_qstat_name");
		return(0);
	}

	if (LCHECK("end_quote_stat"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_qstat_name, "Last quote status");
			DSP_FLD("end_qstat_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			status_search(temp_str);
			return(0);
		}

		if (strcmp(local_rec.st_quote_stat,  local_rec.end_quote_stat) > 0)
 
		{
			/*-----------------------------------------------------------
			| End quote status must NOT be LESS THAN first quote status |
			-----------------------------------------------------------*/
			print_mess( ML(mlStdMess018) );
			sleep(1);
			clear_mess();
			return(1);
		}
		for (i=0; i<12; i++)
		{
			if (!strcmp(local_rec.end_quote_stat, q_status[i]._stat))
			{
				found = TRUE;
				strcpy(local_rec.end_qstat_name, q_status[i]._desc);
				break;
			}
		}

		if (!found)
		{
			/*------------------
			| Status not found |
			------------------*/
			found = FALSE;
			print_mess( ML(mlStdMess205) ); 
			sleep(1);
			clear_mess();
			return(1);
		}

		DSP_FLD("end_quote_stat");
		DSP_FLD("end_qstat_name");
		return(0);
	}

	if (LCHECK("st_sman_no"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_sman_name, "First salesman");
			DSP_FLD("st_sman_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			sman_search(temp_str);
			return(0);
		}

		if (prog_status != ENTRY &&
			strcmp(local_rec.st_sman_no, local_rec.end_sman_no) > 0)
 
		{
			/*----------------------------------------------------------------
			| Start salesman number must NOT be GREATER THAN last salesman no |
			----------------------------------------------------------------*/
			print_mess( ML(mlStdMess017) );
			sleep(1);
			clear_mess();
			return(1);
		}

		strcpy(exsf_rec.co_no, comm_rec.tco_no);
		strcpy(exsf_rec.salesman_no, local_rec.st_sman_no);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------------
			| Salesman number Not Found |
			---------------------------*/
			print_mess( ML(mlStdMess135) );
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.st_sman_name, exsf_rec.salesman);
		DSP_FLD("st_sman_no");
		DSP_FLD("st_sman_name");
		return(0);
	}

	if (LCHECK("end_sman_no"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_sman_name, "Last salesman");
			DSP_FLD("end_sman_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			sman_search(temp_str);
			return(0);
		}

		if (strcmp(local_rec.st_sman_no, local_rec.end_sman_no) > 0)
 
		{
			/*------------------------------------------------------------
			| End saleman number must NOT be LESS THAN first salesman no |
			------------------------------------------------------------*/
			print_mess( ML(mlStdMess018) );
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(exsf_rec.co_no, comm_rec.tco_no);
		strcpy(exsf_rec.salesman_no, local_rec.end_sman_no);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------------
			| Salesman Number Not Found |
			---------------------------*/
			print_mess( ML(mlStdMess135) );
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.end_sman_name, exsf_rec.salesman);
		DSP_FLD("end_sman_no");
		DSP_FLD("end_sman_name");
		return(0);
	}

	if (LCHECK("sort_type"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.sort_desc, "Sort By Salesman");
			DSP_FLD("sort_desc");
			return (EXIT_SUCCESS);
		}

		if (local_rec.sort_type[0] == 'S')
			strcpy(local_rec.sort_type, "S");
		else if (local_rec.sort_type[0] == 'T')
		{
			strcpy(local_rec.sort_type, "T");
			strcpy (local_rec.sort_desc, "Sort By Status");
		}
		else
			return(1);

		DSP_FLD("sort_type");
		DSP_FLD("sort_desc");
		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			/*------------------ 
			| Invalid Printer |
			------------------ */
			print_mess( ML(mlStdMess020) );
			sleep(2);
			clear_mess();
		}
		return(0);
	}

	if (LCHECK("back"))
	{
		if (local_rec.back[0] == 'Y')
			strcpy(local_rec.back, "Yes");
		else
			strcpy(local_rec.back, "No ");
	
		DSP_FLD("back");
		return(0);
	}

	if (LCHECK("onight"))
	{
		if (local_rec.onight[0] == 'Y')
			strcpy(local_rec.onight, "Yes");
		else
			strcpy(local_rec.onight, "No ");
	
		DSP_FLD("onight");
		return(0);
	}
	return(0);
}

void
run_prog (
 char *	prog_name,
 char *	prog_desc)
{
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	sprintf(local_rec.st_date,"%ld",local_rec.st_enqy_date);
	sprintf(local_rec.end_date,"%ld",local_rec.end_enqy_date);
	
	shutdown_prog ();

	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.st_date,
				local_rec.end_date, 
				local_rec.st_quote_no,
				local_rec.end_quote_no, 
				local_rec.st_quote_stat,
				local_rec.end_quote_stat, 
				local_rec.st_sman_no,
				local_rec.end_sman_no, 
				local_rec.sort_type, 
				local_rec.lp_str,
				prog_desc, (char *)0);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.st_date,
				local_rec.end_date, 
				local_rec.st_quote_no,
				local_rec.end_quote_no, 
				local_rec.st_quote_stat,
				local_rec.end_quote_stat, 
				local_rec.st_sman_no,
				local_rec.end_sman_no, 
				local_rec.sort_type, 
				local_rec.lp_str, (char *)0);

	}
	else 
	{
		execlp(prog_name,
			prog_name,
				local_rec.st_date,
				local_rec.end_date, 
				local_rec.st_quote_no,
				local_rec.end_quote_no, 
				local_rec.st_quote_stat,
				local_rec.end_quote_stat, 
				local_rec.st_sman_no,
				local_rec.end_sman_no, 
				local_rec.sort_type, 
				local_rec.lp_str, (char *)0);
	}
}

int
process (void)
{
	fsort = sort_open("enqy");

	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no, "%-8.8s", local_rec.st_quote_no);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp(qthr_rec.co_no, comm_rec.tco_no) &&
	       !strcmp(qthr_rec.br_no, comm_rec.test_no)) 
	{
		if (!strcmp(local_rec.date_type, "Q"))
		{
			if (qthr_rec.dt_quote < local_rec.st_enqy_date ||
				qthr_rec.dt_quote > local_rec.end_enqy_date) 
			{
				cc = find_rec(qthr, &qthr_rec, NEXT, "r");
				continue;
			}
		}

		if (!strcmp(local_rec.date_type, "E"))
		{
			if (qthr_rec.date_create < local_rec.st_enqy_date ||
				qthr_rec.date_create > local_rec.end_enqy_date) 
			{
				cc = find_rec(qthr, &qthr_rec, NEXT, "r");
				continue;
			}
		}

		if (!strcmp(local_rec.date_type, "F"))
		{
			if (qthr_rec.dt_follow_up < local_rec.st_enqy_date ||
				qthr_rec.dt_follow_up > local_rec.end_enqy_date) 
			{
				cc = find_rec(qthr, &qthr_rec, NEXT, "r");
				continue;
			}
		}

		if (!strcmp(local_rec.date_type, "X"))
		{
			if (qthr_rec.expire_date < local_rec.st_enqy_date ||
				qthr_rec.expire_date > local_rec.end_enqy_date) 
			{
				cc = find_rec(qthr, &qthr_rec, NEXT, "r");
				continue;
			}
		}

		if (strcmp(qthr_rec.quote_no, local_rec.st_quote_no) < 0 ||
			strcmp(qthr_rec.quote_no, local_rec.end_quote_no) > 0)
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}

		if (strcmp(qthr_rec.status, local_rec.st_quote_stat) < 0 ||
			strcmp(qthr_rec.status, local_rec.end_quote_stat) > 0)
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}
		
		cumr_rec.cm_hhcu_hash = qthr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (strcmp(cumr_rec.cm_dbt_no, local_rec.st_cust_no) < 0 ||
			strcmp(cumr_rec.cm_dbt_no, local_rec.end_cust_no) > 0)
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}

		if (strcmp(qthr_rec.sman_code, local_rec.st_sman_no) < 0 ||
			strcmp(qthr_rec.sman_code, local_rec.end_sman_no) > 0)
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}


		if (local_rec.sort_type [0] == 'S')
			sprintf(err_str, "%-2.2s|%-2.2s|%-8.8s\n", 
					qthr_rec.sman_code,
					qthr_rec.status,
					qthr_rec.quote_no);
		if (local_rec.sort_type [0] == 'T')
			sprintf(err_str, "%-2.2s|%-2.2s|%-8.8s\n", 
					qthr_rec.status,
					qthr_rec.sman_code,
					qthr_rec.quote_no);
	
			sort_save(fsort, err_str);
		
		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
	}
	process_fsort();
	return(0);
}

void
process_fsort (void)
{
	fsort = sort_sort(fsort, "enqy");
	sptr = sort_read(fsort);

	head_output();

	while (sptr != (char *)0)
	{
		if (BY_SALESMAN)
			proc_bysman ();
		else 
			proc_bystatus ();

		first_time = FALSE;
		sptr = sort_read(fsort);
	}
	sort_delete (fsort, "enqy");
	fprintf(fout, ".B2\n");
	fprintf(fout,".C*****  Nothing  Follows  *****\n");
}

void
proc_bysman (void)
{
	sprintf(curr_sman, "%-2.2s", sptr);
	sprintf(curr_status, "%-2.2s", sptr + 3);
	sprintf(qthr_rec.quote_no, "%-8.8s", sptr + 6);
	
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qthr, "DBFIND");

	if (qthr_rec.hhcu_hash != 0L)
	{
		cumr_rec.cm_hhcu_hash = qthr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			memset(&cumr_rec, 0, sizeof(cumr_rec));
	}
	else
	{
		strcpy (cumr_rec.cm_dbt_no, "99998 ");
		sprintf(cumr_rec.cm_name, qthr_rec.dbt_name);
	}

	strcpy(exsf_rec.co_no, comm_rec.tco_no);
	strcpy(exsf_rec.salesman_no, curr_sman);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		memset(&exsf_rec, 0, sizeof(exsf_rec));
	
	find_status(curr_status);	

	if (strcmp(curr_sman, prev_sman))
	{
		if (!first_time)
			fprintf(fout, ".PA\n");
		fprintf(fout, "Salesman  %-2.2s %-40.40s  %-40.40s\n",
						exsf_rec.salesman_no,
						exsf_rec.salesman, " ");
	}
	
	if (strcmp(curr_status, prev_status) || strcmp(curr_sman, prev_sman))
	{
		fprintf(fout, ".B1\n");
		fprintf(fout, "Quote status  %-2.2s %-40.40s %40.40s\n",
						curr_status, desc, " ");
		fprintf(fout, ".B1\n");
	}

	fprintf(fout, "%-8.8s %-10.10s %-2.2s  %-28.28s %-6.6s %-40.40s        %-2.2s %11.2f  %11.2f\n",
						qthr_rec.quote_no,
						DateToString(qthr_rec.date_create),
						curr_status,
						desc,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						exsf_rec.salesman_no,
						DOLLARS(qthr_rec.qt_value),
						DOLLARS(qthr_rec.qt_profit_cur));
		
	strcpy(prev_sman, curr_sman);		
	strcpy(prev_status, curr_status);		
}
			
void		
proc_bystatus (void)
{
	sprintf(curr_status, "%-2.2s", sptr);
	sprintf(curr_sman, "%-2.2s", sptr + 3);
	sprintf(qthr_rec.quote_no, "%-8.8s", sptr + 6);
	
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	cc = find_rec (qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qthr, "DBFIND");

	if (qthr_rec.hhcu_hash != 0L)
	{
		cumr_rec.cm_hhcu_hash = qthr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			memset(&cumr_rec, 0, sizeof(cumr_rec));
	}
	else
	{
		strcpy (cumr_rec.cm_dbt_no, "99998 ");
		sprintf(cumr_rec.cm_name, qthr_rec.dbt_name);
	}

	strcpy(exsf_rec.co_no, comm_rec.tco_no);
	strcpy(exsf_rec.salesman_no, curr_sman);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
 	if (cc)
		memset(&exsf_rec, 0, sizeof(exsf_rec));
	
	find_status(curr_status);	

	if (strcmp(curr_status, prev_status))
	{
		if (!first_time)
			fprintf(fout, ".PA\n");
		fprintf(fout, "Quote status  %-2.2s %-40.40s %40.40s\n",
						curr_status, desc, " ");
	}
	
	if (strcmp(curr_sman, prev_sman) || strcmp(curr_status, prev_status))
	{
		fprintf(fout, ".B1\n");
		fprintf(fout, "Salesman  %-2.2s %-40.40s  %-40.40s\n",
						exsf_rec.salesman_no,
						exsf_rec.salesman, " ");
		fprintf(fout, ".B1\n");
	}

	fprintf(fout, "%-8.8s %-10.10s %-2.2s  %-28.28s %-6.6s %-40.40s        %-2.2s %11.2f  %11.2f\n",
						qthr_rec.quote_no,
						DateToString(qthr_rec.date_create),
						curr_status,
						desc,
						cumr_rec.cm_dbt_no,
						cumr_rec.cm_name,
						exsf_rec.salesman_no,
						DOLLARS(qthr_rec.qt_value),
						DOLLARS(qthr_rec.qt_profit_cur));
		
	strcpy(prev_sman, curr_sman);		
	strcpy(prev_status, curr_status);		
}

void
find_status (
 char *	stat)
{
	int i;

	for (i = 0; i < 12; i++)
	{
		if (!strcmp(q_status[i]._stat, curr_status))
		{
			strcpy(desc, q_status[i]._desc);
			break;
		}
	}
}

int
head_output (void)
{
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".L135\n");

	fprintf(fout,".4\n");
	fprintf(fout,".EENQUIRY LIST\n");

	fprintf(fout,".B2\n");
		
	fprintf(fout, "Quote no  Enq date  ");
	fprintf(fout, "Quote status                     ");   
	fprintf(fout, "               Customer                          ");
	fprintf(fout, "Salesman       Value       Profit\n"); 

	fprintf(fout,".B1\n");

	return(0);
}

int
heading (
 int	scn)
{
	if (scn != cur_screen)
		scn_set(scn);

	clear();

	box (0, 3, 131, 14);
	move(1, 6);
	line(130);
	move(1, 9);
	line(130);
	move(1, 12);
	line(130);
	move(1, 15);
	line(130);

	/*--------------------------------
	| Print Enquiry list by Customer |
	--------------------------------*/
	sprintf (err_str, " %s ", ML(mlQtMess012) );
	rv_pr(err_str, 50, 0, 1);
		
	move(0,1);
	line(131);

	move(0,20);
	line(131);
	sprintf (err_str, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
	print_at(21,0, "%s", err_str);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write(scn);
	return (EXIT_SUCCESS);
}

void
quote_search (
 char *	key_val)
{
	work_open();
	save_rec("#Quote no.", "# ");
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	strcpy(qthr_rec.quote_no, key_val);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");

	while (!cc && !strcmp(qthr_rec.co_no, comm_rec.tco_no) &&
		   !strcmp(qthr_rec.br_no, comm_rec.test_no) &&
		   !strncmp(qthr_rec.quote_no, key_val, strlen(key_val)))
	{
		cc = save_rec(qthr_rec.quote_no, " ");
		if (cc)
			break;

		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no, "%-8.8s", key_val);
	cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qthr, "DBFIND");
}

void
status_search (
 char *	key_val)
{
	int i;

	work_open();
	save_rec("#Status", "#Description");

	for (i=0; i<12; i++) 
		save_rec (q_status [i]._stat, q_status[i]._desc);

	cc = disp_srch();
	work_close();
	if (cc)
		return;
	for (i=0; i<12; i++)
	{
		if (!strcmp(key_val, q_status[i]._stat))
			break;
	}
}

void
sman_search (
 char *	key_val)
{
	work_open();
	save_rec("#No.", "#Description");
	strcpy(exsf_rec.co_no, comm_rec.tco_no);
	strcpy(exsf_rec.salesman_no, key_val);
	cc = find_rec(exsf, &exsf_rec, GTEQ, "r");

	while (!cc && !strcmp(exsf_rec.co_no, comm_rec.tco_no) &&
		   !strncmp(exsf_rec.salesman_no, key_val, strlen(key_val)))
	{
		cc = save_rec(exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.co_no, comm_rec.tco_no);
	sprintf(exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}



/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( statmaint.c     )                                |
|  Program Desc  : ( Quotation Status Maintenance.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : ccmr, cumr, comm, comr, esmr, exsf, qthr,          |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : qthr,                                              |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Elena Cuaresma. | Date Written  : 12/10/95         |
|---------------------------------------------------------------------|
|  Date Modified : 06/11/95 		 Modified By   : Elena Cuaresma   |
|  Date Modified : 28/12/95 		 Modified By   : Elena Cuaresma   |
|  Date Modified : 08/05/96 		 Modified By   : Jiggs Veloz      |
|  Date Modified : 11/09/97 		 Modified By   : Roanna Marcelino |
|  Date Modified : 23/08/1999 		 Modified By   : Alvin Misalucha  |
|                :                        							  | 
|  Comments      : HOSKYNS 007 Fix bug in entering reason code.       |
|  (28/12/95)    : HOSKYNS 007 Does not allow manual entry of quote   |
|                :             status.                                |
|                :             Add an action for Quote Status 40 -    |
|                :             Reported Back.                         |
|                :                        							  | 
|  (08/05/96)    : Update to add Quote Status 45 - ON HOLD            |
|  (11/09/97)    : Modified for Multilingual Conversion.              |
|  (23/08/1999)  : Converted to ANSI convention.                      |
|                :             										  |
| $Log: statmaint.c,v $
| Revision 5.2  2001/08/09 08:44:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:38  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:32  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:03  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/11/16 03:29:21  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.13  1999/09/29 10:12:34  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/22 05:18:29  scott
| Updated from Ansi project.
|
| Revision 1.11  1999/09/14 04:05:39  scott
| Updated for Ansi.
|
| Revision 1.10  1999/06/18 06:12:28  scott
| Updated to add log for cvs and remove old style read_comm()
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: statmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_statmaint/statmaint.c,v 5.2 2001/08/09 08:44:47 scott Exp $";

#define MAXWIDTH 	150
#define MAXSCNS 	2
#define	SLEEP_TIME	3
#define	FUD_QT 		(local_rec.action[0] == 'D')
#define	REQ_QT 		(local_rec.action[0] == 'R')
#define	WON_QT 		(local_rec.action[0] == 'W')
#define	LST_QT 		(local_rec.action[0] == 'L')
#define	ENQ_QT 		(local_rec.action[0] == 'N')
#define	BCK_QT 		(local_rec.action[0] == 'B')

#define NO_KEY(x)	( vars[ x ].required == NA || \
			  		  vars[ x ].required == NI || \
			  		  vars[ x ].required == ND )

#define HIDE(x)		( vars[ x ].required == ND )
#define NEED(x)		( vars[ x ].required == YES )

#include <std_decs.h>
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_qt_mess.h>

	/*=========================== 
	| Special fields and flags. |
	===========================*/
	int		envDbFind = 0,
			envDbCo = 0,
			changes_made = FALSE,
			first_time 	 = TRUE;

	char	*data = "data",
			*cumr = "cumr",
			*comm = "comm",
			*comr = "comr",
			*ccmr = "ccmr",
			*esmr = "esmr",
			*exsf = "exsf",
			*qthr = "qthr",
			*exlc = "exlc",
			*exlq = "exlq";

	char	branchNo[3];

	char	*ser_space = "                         ";
	long 	cur_ldate;

	
	/*==========================
	| Common Record Structure. |	
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
		{"comm_dbt_date"}
	};
	
	int comm_no_fields = 8;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	co_name[41];
		char	test_no[3];
		char	est_short[16];
		char	cc_no[3];
		char	cc_short[10];
		long	dbt_date;
	} comm_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_adr1"},
		{"comr_co_adr2"},
		{"comr_co_adr3"},
		{"comr_stat_flag"}
	};

	int comr_no_fields = 6;

	struct {
		char	co_no[3];
		char	co_name[41];
		char	co_adr[3][41];
		char	stat_flag[2];
	} comr_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_stat_flag"}
	};

	int exsf_no_fields = 4;

	struct {
		char	co_no[3];
		char	no[3];
		char	desc[41];
		char	stat_flag[2];
	} exsf_rec;

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
		char	ccmr_co_no[3];
		char	ccmr_est_no[3];
		char	ccmr_cc_no[3];
		long	ccmr_hhcc_hash;
		char	ccmr_stat_flag[2];
	} ccmr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_nx_order_no"},
	};

	int esmr_no_fields = 3;

	struct {
		char	esmr_co_no[3];
		char	esmr_est_no[3];
		long	esmr_nx_order_no;
	} esmr_rec;

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
		{"cumr_class_type"},
		{"cumr_curr_code"},
		{"cumr_price_type"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_dl_adr1"},
		{"cumr_dl_adr2"},
		{"cumr_dl_adr3"},
		{"cumr_contact_name"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_disc_code"},
		{"cumr_tax_code"},
		{"cumr_tax_no"},
		{"cumr_inst_fg1"},
		{"cumr_inst_fg2"},
		{"cumr_inst_fg3"},
		{"cumr_stat_flag"},
		{"cumr_item_codes"}
	};

	int cumr_no_fields = 27;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dp_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
		char	cm_curr_code[4];
		char	cm_price_type[2];
		char	cm_ch_adr[3][41];
		char	cm_dl_adr[3][41];
		char	cm_contact_name[21];
		char	cm_area_code[3];
		char	cm_sman_code[3];
		char	cm_disc_code[2];
		char	cm_tax_code[2];
		char	cm_tax_no[16];
		int		cm_inst[3];
		char	cm_stat_flag[2];
		char	cm_item_codes[2];
	} cumr_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	14

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_hhcu_hash"},
		{"qthr_dt_follow_up"},
		{"qthr_dt_quote"},
		{"qthr_sman_code"},
		{"qthr_cont_name"},
		{"qthr_status"},
		{"qthr_place_ord"},
		{"qthr_reas_code"},
		{"qthr_reas_desc"},
		{"qthr_comp_code"},
		{"qthr_comp_name"},
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		long	hhcu_hash;
		long	dt_follow_up;
		long	dt_quote;
		char	sman_code [3];
		char	cont_name [21];
		char	status [3];
		char	place_ord [2];
		char	reas_code [4];
		char	reas_desc [31];
		char	comp_code [4];
		char	comp_name [31];
	}	qthr_rec;

	/*===========================
	| External Competitor File. |
	============================*/
	struct dbview exlc_list[] ={
		{"exlc_co_no"},
		{"exlc_code"},
		{"exlc_name"}
	};

	int exlc_no_fields = 3;

	struct {
		char	co_no[3];
		char	code[4];
		char	name[31];
	} exlc_rec;

	/*============================
	| External Lost Reason File. |
	============================*/
	struct dbview exlq_list[] ={
		{"exlq_co_no"},
		{"exlq_code"},
		{"exlq_description"}
	};

	int exlq_no_fields = 3;

	struct {
		char	co_no[3];
		char	code[4];
		char	description[31];
	} exlq_rec;

#include	<qt_commands.h>
#include	<qt_status.h>
#include	<time.h>

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	systemDate[11];
	char 	action[2];
	char	item_no[17];
	long	item_hash;
	char	sup_part[17];
	float	qty;
	double	cost_price;
	char	par_code[11];
	char	serial_no[26];
	long	n_quote_no;
	long	n_pros_no;
	char	cont_no[7];
	char	cont_desc[41];
	char	quote_no[9];
	char	stat_desc[41];
	char	status [3];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "n_quote",	 4, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Quote Number         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.quote_no},
	{1, LIN, "cust_no",	 5, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No          ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.cm_dbt_no},
	{1, LIN, "cust_name",	 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name         ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.cm_name},
	{1, LIN, "sman_desc",	 7, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Salesman             ", "",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.desc},
	{1, LIN, "qt_stat",		8, 20, CHARTYPE,
		"UU", "          ",
		" ", " ", "Quote Status         ", " Enter Quote Status Code",
		 NA, NO, JUSTLEFT, "", "", qthr_rec.status},
	{1, LIN, "qt_stat_desc",	8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.stat_desc}, 
	{1, LIN, "dt_follow_up",	9, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Next Follow up Date  ", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&qthr_rec.dt_follow_up},
	{1, LIN, "action",	  10, 20, CHARTYPE,
		"U", "          ",
		" ", " ", "Action (D,R,W,L,N,B) ", "D-Follow-up Date, R-requote, W-won, L-Lost, N-no order, B-Reporting Back.",
		 YES, NO,  JUSTLEFT, "DRWLNB", "", local_rec.action},

	{2, LIN, "reas_code",	14, 30, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Reason Code        ", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.reas_code},
	{2, LIN, "reas_desc",	15, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description         ", " ",
		 NA, NO,  JUSTLEFT, "", "", qthr_rec.reas_desc},
	{2, LIN, "comp_code",	17, 30, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Competitor's Code  ", " ",
		 NO, NO,  JUSTLEFT, "", "", qthr_rec.comp_code},
	{2, LIN, "comp_name",	18, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Competitor's Name  ", "",
		 NA, NO,  JUSTLEFT, "", "", qthr_rec.comp_name},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		main			(int argc, char * argv []);
int		heading			(int);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		ReadComm		(void);
int		spec_valid		(int field);
int		update			(void);
void	hlight			(char * text);
void	prn_co			(void);
void	show_qthr		(char * key_val);
void	show_stat		(void);
void	show_exlq		(char * key_val);
void	show_exlc		(char * key_val);

#include <FindCumr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 2)
	{
		print_at(0,0,ML(mlStdMess071), argv[0]);
		return (EXIT_SUCCESS);
	} 
	SETUP_SCR (vars);
	init_scr();
	set_tty();
	_set_masks( argv[1] );
	init_vars(1);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	ReadComm();

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));
	strcpy(branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB();

	while (prog_exit == 0)
	{
		abc_unlock(qthr);
		FLD("action") = YES;
		FLD("n_quote")= YES;
		FLD("dt_follow_up")= NA;
		FLD("qt_stat") = NA;
		set_tty();
		eoi_ok     = 1;
		search_ok  = 1;
		entry_exit = 0;
		edit_exit  = 0;
		prog_exit  = 0;
		restart    = 0;
		skip_entry = 0;
		init_ok    = 1;
		skip_tab   = 0;
		changes_made= FALSE;
		init_vars(1);	

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading(1);
		entry(1);
		FLD("n_quote")= NE;
		if (restart == 0 && prog_exit == 0)
		{
			if (LST_QT && (!strcmp(qthr_rec.status, "91")))
			{
				heading(2);
				entry(2);
				if (restart == 0 && prog_exit == 0)
					edit(2);
			} 
			heading(1);
			scn_display(1);
			edit (1);
		}
		if (changes_made && restart == 0)
			update();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	open_rec(cumr, cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no"
							       						 : "cumr_id_no3");
	open_rec(qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec(exlc, exlc_list, exlc_no_fields, "exlc_id_no");
	open_rec(exlq, exlq_list, exlq_no_fields, "exlq_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose(cumr);
	abc_fclose(qthr);
	abc_fclose(exlc);
	abc_fclose(exlq);
	abc_dbclose(data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
int
ReadComm (void)
{
	abc_dbopen(data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec(ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec(comr, comr_list, comr_no_fields, "comr_co_no");

	strcpy(ccmr_rec.ccmr_co_no,  comm_rec.tco_no);
	strcpy(ccmr_rec.ccmr_est_no, comm_rec.test_no);
	strcpy(ccmr_rec.ccmr_cc_no,  comm_rec.cc_no);
	cc = find_rec(ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, ccmr, "DBFIND");

	strcpy(comr_rec.co_no, comm_rec.tco_no);
	cc = find_rec(comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, comr, "DBFIND");

	abc_fclose(ccmr);
	abc_fclose(comr);

	return(0);
}

int
spec_valid (
 int	field)
{
	int	i;

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK("action"))
	{
		if (!strcmp(local_rec.action, " "))
		{
			print_err (ML(mlStdMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("action");
		if (!strcmp (qthr_rec.status, "45") )
		{
			print_err (ML(mlQtMess044));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (FUD_QT)
		{
			FLD ("dt_follow_up") = NO;
			FLD ("n_quote") = NA;
			changes_made = TRUE;
			do
			{
				get_entry (label ("dt_follow_up"));
				cc = spec_valid (label ("dt_follow_up"));
			} while (cc && !restart);
			DSP_FLD ("dt_follow_up");
		}
		if (REQ_QT || WON_QT || LST_QT || ENQ_QT || BCK_QT)

		{			
			FLD ("n_quote") = NA;
			FLD ("qt_stat") = NA;
			if (REQ_QT)
				strcpy (qthr_rec.status, "50");

			if (WON_QT)
				strcpy (qthr_rec.status, "90");

			if (LST_QT)
				strcpy (qthr_rec.status, "91");

			if (BCK_QT)
			{
				if (!strcmp(qthr_rec.status, "30"))
					strcpy (qthr_rec.status, "40");
				else
				{
					print_err(ML(mlQtMess046));
					sleep(2);
					return(1);
				}
			}

			if (ENQ_QT)
			{
				if (!strcmp(qthr_rec.place_ord, "Y") 	&& 
					!strcmp(qthr_rec.status, "92"))
				{
					print_err(ML(mlQtMess045));
					sleep(2);
					return(1);
				}
				else
				{
					strcpy (qthr_rec.status, "92");
					strcpy (qthr_rec.place_ord, "Y");
				}
			}

			for (i = 0;strlen(q_status[i]._stat);i++)
			{
				if (!strncmp(qthr_rec.status, q_status[i]._stat,strlen(q_status[i]._stat)))
				{
					sprintf(local_rec.stat_desc,"%-40.40s",q_status[i]._desc);
					break;
				}
			}
			changes_made = TRUE;
		}
		else
			FLD ("qt_stat") = NA;

		DSP_FLD ("qt_stat");
		DSP_FLD("qt_stat_desc");
		if (LST_QT && (!strcmp(qthr_rec.status, "91")) && prog_status != ENTRY)
		{
			heading(2);
			entry(2);
			if (restart == 0 && prog_exit == 0)
				edit(2);
		}
		return(0);
	}

	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK("n_quote"))
	{
		if (SRCH_KEY)
		{
			show_qthr(temp_str);
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("n_quote");
			return(0);
		}

		if (!strcmp(local_rec.quote_no, "        "))
		{
			print_err (ML(mlStdMess210)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (qthr_rec.quote_no, zero_pad(local_rec.quote_no,8));
		strcpy (local_rec.quote_no, qthr_rec.quote_no); 

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		abc_selfield(qthr,"qthr_id_no2");
		strcpy(qthr_rec.co_no, comm_rec.tco_no);
		strcpy(qthr_rec.br_no, comm_rec.test_no);
		if ( !find_rec(qthr, &qthr_rec, EQUAL, "w"))
		{
			open_rec(exsf, exsf_list, exsf_no_fields, "exsf_id_no");
			strcpy(exsf_rec.co_no, comm_rec.tco_no);
			strcpy(exsf_rec.no, qthr_rec.sman_code);
			cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
			if (cc)
				sprintf(exsf_rec.desc,"%-40.40s", " ");

			abc_fclose(exsf);

		    abc_selfield(cumr, "cumr_hhcu_hash");
			if (qthr_rec.hhcu_hash != 0L)
			{
				if (!find_hash(cumr, &cumr_rec, COMPARISON, "r", 
								qthr_rec.hhcu_hash))
				{
					DSP_FLD ("n_quote");
					DSP_FLD ("cust_no");
					DSP_FLD ("cust_name");
					DSP_FLD ("sman_desc");
					DSP_FLD ("qt_stat");
					DSP_FLD ("dt_follow_up");
				}
			}
			else
			{
				strcpy (cumr_rec.cm_dbt_no, "99998 ");
				DSP_FLD ("n_quote");
				DSP_FLD ("cust_no");
				DSP_FLD ("cust_name");
				DSP_FLD ("sman_desc");
				DSP_FLD ("qt_stat");
				DSP_FLD ("dt_follow_up");
			}
		}
		else
		{
			print_mess(ML(mlStdMess210));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		return(0);
	}
	/*------------------------
	| Validate Quote Status. |
	-------------------------*/
	if (LCHECK("qt_stat"))
	{
		if (last_char == SEARCH)
		{
			show_stat();
			return(0);
		}
		for (i = 0;strlen(q_status[i]._stat);i++)
		{
			if (!strncmp(qthr_rec.status, q_status[i]._stat,strlen(q_status[i]._stat)))
			{
				sprintf(local_rec.stat_desc,"%-40.40s",q_status[i]._desc);
				break;
			}
		}
		DSP_FLD("qt_stat");
		DSP_FLD("qt_stat_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Reason Code. |
	------------------------*/
	if (LCHECK("reas_code"))
	{
		if (dflt_used)
		{
			strcpy(qthr_rec.reas_code, exlq_rec.code);
			DSP_FLD ("reas_code");
			return(0);
		}
		if (last_char == SEARCH) 
		{
			show_exlq(temp_str);
			heading(1);
			scn_display(1);
			heading(2);
			scn_display(2);
			return(0);
		}
		strcpy(exlq_rec.co_no, comm_rec.tco_no);
		strcpy(exlq_rec.code, qthr_rec.reas_code);
		if ( find_rec(exlq, &exlq_rec, EQUAL, "r"))
		{
			print_mess(ML(mlStdMess210));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		strcpy(qthr_rec.reas_desc, exlq_rec.description);
		DSP_FLD("reas_code");
		DSP_FLD("reas_desc");
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| Validate Competitor Code. |
	----------------------------*/
	if (LCHECK("comp_code"))
	{
		if (dflt_used)
		{
			strcpy(qthr_rec.comp_code, exlc_rec.code);
			DSP_FLD ("comp_code");
			return(0);
		}

		if (last_char == SEARCH) 
		{
			show_exlc(temp_str);
			heading(1);
			scn_display(1);
			heading(2);
			scn_display(2);
			return(0);
		}
		strcpy(exlc_rec.co_no, comm_rec.tco_no);
		strcpy(exlc_rec.code, qthr_rec.comp_code);
		if ( find_rec(exlc, &exlc_rec, EQUAL, "r"))
		{
			print_mess(ML(mlStdMess210));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		strcpy(qthr_rec.comp_name, exlc_rec.name);
		DSP_FLD("comp_code");
		DSP_FLD("comp_name");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Reason Description. |
	-------------------------------*/
	if (LCHECK("reas_desc"))
	{
		if (dflt_used)
		{
			strcpy(qthr_rec.reas_desc, exlq_rec.description);
			DSP_FLD ("reas_desc");
			return(0);
		}
		DSP_FLD("reas_desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Competitor's Name. |
	------------------------------*/
	if (LCHECK("comp_name"))
	{
		if (dflt_used)
		{
			strcpy(qthr_rec.comp_name, exlc_rec.name);
			DSP_FLD ("comp_name");
			return(0);
		}

		DSP_FLD("comp_name");
		return (EXIT_SUCCESS);
	}
	return(0);
}

int
update (void)
{
	clear();
	fflush(stdout);
		
	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (!LST_QT)
	{
		strcpy(qthr_rec.reas_code, "   ");
		strcpy(qthr_rec.reas_desc, "                              ");
		strcpy(qthr_rec.comp_code, "   ");
		strcpy(qthr_rec.comp_name, "                              ");
	}

	sprintf (err_str, ML("Now Updating Existing Quote..."));

	hlight(err_str);
	cc = abc_update(qthr, &qthr_rec);
	if (cc) 
		file_err(cc, qthr, "DBUPDATE");

	sleep (sleepTime);
	abc_unlock(qthr);
	changes_made = FALSE;
	return(0);
}

void
hlight (
 char *	text)
{
	print_at (0,0,"\n\r%s ", text);
	fflush(stdout);
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn == 1)
		{
			scn_set (scn);
			clear();
			rv_pr(ML(mlQtMess029), 30,0,1);
			pr_box_lines( scn );
		}

		if (scn == 2)
		{
			scn_set (scn);
			cl_box(10,11,60,7);
			rv_pr(ML(mlQtMess030), 30,12,1);
			move(11,13);
			line(59);
		}

		prn_co();
		scn_write(scn); 
	}
	return (EXIT_SUCCESS);
}

void
prn_co (void)
{
	move(0,21);
	line(79); 

	strcpy(err_str,ML(mlStdMess038)); 
	print_at(22,0,err_str, comm_rec.tco_no, clip(comm_rec.co_name)); 

	strcpy(err_str,ML(mlStdMess039));
	print_at(22,30,err_str, comm_rec.test_no, clip(comm_rec.est_short));

	strcpy(err_str,ML(mlStdMess099));
	print_at(22,50,err_str, comm_rec.cc_no, clip(comm_rec.cc_short));

}

/*==========================
| Search for quote number. |
==========================*/
void
show_qthr (
 char *	key_val)
{
	char	wk_mask[9];

	work_open();
	save_rec("#Qt No.","#Quote Date. | Contact");
	abc_selfield(qthr,"qthr_id_no2");
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no,"%-8.8s",key_val);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strncmp(qthr_rec.quote_no,key_val,strlen(key_val)) && 
		   !strcmp(qthr_rec.co_no, comm_rec.tco_no) && 
		   !strcmp(qthr_rec.br_no, comm_rec.test_no))
	{
		sprintf(err_str, " %10.10s |%s", DateToString(qthr_rec.dt_quote),
						qthr_rec.cont_name);

		sprintf(wk_mask, "%-8.8s", qthr_rec.quote_no);
		cc = save_rec(wk_mask,err_str);
		if (cc)
				break;

		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no, "%-8.8s", temp_str);
	cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
	{
		file_err(cc, qthr, "DBFIND");
	}
	abc_selfield(qthr,"qthr_id_no");
}

/*==================================
| Search routine for Quote Status. |
==================================*/
void
show_stat (void)
{
	int 	i = 0;

	work_open();
	save_rec("#Cd","#Status Description");
	
	for (i =0;strlen(q_status[i]._stat);i++)
	{
		cc = save_rec(q_status[i]._stat,q_status[i]._desc);
		if (cc)
			break;
	}
	cc = disp_srch();
	work_close();
}

/*=========================
| Search for reason code. |
=========================*/
void
show_exlq (
 char *	key_val)
{
	work_open(); 
	save_rec("#Lq","#Description");
	strcpy(exlq_rec.co_no, comm_rec.tco_no);
	sprintf(exlq_rec.code,"%-3.3s",key_val);
	cc = find_rec(exlq, &exlq_rec, GTEQ, "r");
	while (!cc && !strncmp(exlq_rec.code,key_val,strlen(key_val)) && 
		   !strcmp(exlq_rec.co_no, comm_rec.tco_no)) 
	{
		cc = save_rec(exlq_rec.code,exlq_rec.description);
		if (cc)
				break;

		cc = find_rec(exlq, &exlq_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close(); 
	if (cc)
		return;  

	strcpy(exlq_rec.co_no, comm_rec.tco_no);
	sprintf(exlq_rec.code, "%-3.3s", temp_str);
	cc = find_rec(exlq, &exlq_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, exlq, "DBFIND");

	strcpy(qthr_rec.reas_code, exlq_rec.code);
	strcpy(qthr_rec.reas_desc, exlq_rec.description);
}

/*=============================
| Search for competitor code. |
==============================*/
void
show_exlc (
 char *	key_val)
{
	work_open();
	save_rec("#Cd","#Competitor's Name");
	strcpy(exlc_rec.co_no, comm_rec.tco_no);
	sprintf(exlc_rec.code,"%-3.3s",key_val);
	cc = find_rec(exlc, &exlc_rec, GTEQ, "r");
	while (!cc && !strncmp(exlc_rec.code,key_val,strlen(key_val)) && 
		   !strcmp(exlc_rec.co_no, comm_rec.tco_no)) 
	{
		cc = save_rec(exlc_rec.code,exlc_rec.name);
		if (cc)
				break;

		cc = find_rec(exlc, &exlc_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return; 

	strcpy(exlc_rec.co_no, comm_rec.tco_no);
	sprintf(exlc_rec.code, "%-3.3s", temp_str);
	cc = find_rec(exlc, &exlc_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, exlc, "DBFIND");

	strcpy(qthr_rec.comp_code, exlc_rec.code);
	strcpy(qthr_rec.comp_name, exlc_rec.name);
}

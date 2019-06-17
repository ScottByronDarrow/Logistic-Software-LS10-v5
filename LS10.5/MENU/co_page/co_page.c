/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( co_page.c      )                                 |
|  Program Desc  : ( Company / Branch page number Maintenence.    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : comr ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (31/03/88)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (25/11/96)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (04/09/97)      | Modified  by  :Jiggs A Veloz.    |
|  Date Modified : (08/10/97)      | Modified  by  :Ana Marie Tario.  |
|  Date Modified : (28/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (15/06/1998)    | Modified by : Scott B Darrow.    |
|  Date Modified : (27/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      :                                                    |
|  (04/09/97)    : SEL-Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                                      |
|  (08/10/97)    : SEL-Added last foreign po#'s in 1st & 2nd screens. |
|  (28/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|  (27/08/1999)  : Ported to ANSI format.                             |
|                :                                                    |
|                                                                     |
| $Log: co_page.c,v $
| Revision 5.4  2002/07/29 05:23:15  scott
| .
|
| Revision 5.3  2002/07/19 05:01:09  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 05:13:20  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:11  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:07  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:23  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:53  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/12/22 00:32:27  cam
| Fixes for GVision compatibility.  Changed search to do a work_close () after
| disp_srch ().
|
| Revision 1.13  1999/12/06 01:47:09  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 09:41:54  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.11  1999/09/29 10:11:01  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:26:52  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/15 02:35:57  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: co_page.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/co_page/co_page.c,v 5.4 2002/07/29 05:23:15 scott Exp $";

#define	MAXWIDTH	150
#define	MAXLINES	100
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int	new_comp = 0;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_nx_po_no"},
		{"comr_nx_po_no_fgn"},
		{"comr_nx_gr_no"},
		{"comr_nx_pac_no"},
		{"comr_nx_del_no"},
		{"comr_nx_chq_no"},
		{"comr_ls_chq_no"},
		{"comr_nx_job_no"},
		{"comr_nx_rec_no"},
		{"comr_stat_flag"}
	};

	int comr_no_fields = 12;

	struct {
		char	mr_co_no[3];
		char	mr_co_name[41];
		long	mr_nx_pur_no;
		long	mr_nx_pur_no_fgn;
		long	mr_nx_gr_no;
		long	mr_nx_pac_no;
		long	mr_nx_del_no;
		long	mr_nx_chq_no;
		long	mr_ls_chq_no;
		long	mr_nx_job_no;
		long	mr_nx_rec_no;
		char	mr_stat_flag[2];
	} comr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_nx_pur_ord_no"},
		{"esmr_nx_pur_fgn"},
		{"esmr_nx_gr_no"},
		{"esmr_nx_del_dck_no"},
		{"esmr_nx_cheq_no"},
		{"esmr_ls_cheq_no"},
		{"esmr_nx_job_no"},
		{"esmr_stat_flag"},
	};

	int esmr_no_fields = 11;

	struct {
		char	es_co_no[3];
		char	es_br_no[3];
		char	es_est_name[41];
		long	es_num[7];
		char	es_stat_flag[2];
	} esmr_rec;

	long	num[7];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	cur_co[3];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "compno",	 4, 30, CHARTYPE,
		"AA", "          ",
		" ", "", "Company No.", " ",
		 NE, NO, JUSTRIGHT, "1", "99", comr_rec.mr_co_no},
	{1, LIN, "coname",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Company name", " ",
		 NA, NO,  JUSTLEFT, "          ", " ", comr_rec.mr_co_name},
	{1, LIN, "next1",	 7, 30, LONGTYPE,
		"NNNNNNN", "          ",
		"0", "0", "Last Local Purchase Order # ", "Input Last Local Purchase Order #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_pur_no},
	{1, LIN, "next2",	8, 30, LONGTYPE,
		"NNNNNNN", "          ",
		"0", "0", "Last Foreign Purchase Order # ", "Input Last Foreign Purchase Order #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_pur_no_fgn},
	{1, LIN, "next3",	9, 30, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Last Goods Receipt # ", "Input Last Goods Receipt #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_gr_no},
	{1, LIN, "next4",	10, 30, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Last Delivery Docket # ", "Input Last Delivery Docket #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_del_no},
	{1, LIN, "next5",	11, 30, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Last Cheque # ", "Input Last Cheque #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_chq_no},
	{1, LIN, "next6",	12, 30, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Next Cheque # ", "Input Next Cheque #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_ls_chq_no},
	{1, LIN, "next7",	13, 30, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Last Job # ", "Input Last Job #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_job_no},
	{1, LIN, "next8",	14, 30, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Last Cash Receipt # ", "Input Last Cash Receipt #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.mr_nx_rec_no},
	{2, TAB, "br",	MAXLINES, 0, CHARTYPE,
		"AA", "          ",
		"0", "0", "BR", "",
		 NA, NO, JUSTRIGHT, "", "", esmr_rec.es_br_no},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", "0", "Local P/Order No", "Input Last P/O Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[0]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", "0", "Foreign P/Order No.", "Input Last P/O Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[1]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Goods Receipt No", "Input Last GR Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[2]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Delivery Doc No", "Input Last Delivery Docket Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[3]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Next Cheque No", "Input Next Cheque Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[4]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Last Cheque No", "Input Last Cheque Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[5]},
	{2, TAB, "next1",	 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Next Job No", "Input Last Job Number.",
		YES, NO, JUSTRIGHT, "", "", (char *)&num[6]},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		main (int argc, char * argv []);
void	shutdown_prog (void);
void	OpenDB (void);
void	CloseDB (void);
int		spec_valid (int field);
int		get_branches (void);
int		update (void);
void	show_comr (char *key_val);
int		heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();                      /*  get into raw mode		*/
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

	OpenDB();

	while (prog_exit == 0) 
	{
		entry_exit = 0;
		restart = 0;
		edit_exit = 0;
		prog_exit = 0;
		search_ok = 1;
		new_comp = 0;
		init_vars (1);
		init_vars (2);
		lcount[2] = 0;

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		if (new_comp == 1)
		{
			heading(2);
			entry(2);
			if (prog_exit || restart)
				continue;
		}

		edit_exit = 0;

		edit_all();

		if (!restart)
			update();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
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
	abc_dbopen("data");
	open_rec("comr", comr_list, comr_no_fields, "comr_co_no");
	open_rec("esmr", esmr_list, esmr_no_fields, "esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("comr");
	abc_fclose("esmr");
	abc_dbclose("data");
}

int
spec_valid (
 int	field)
{
	/*--------------------------
	| Validate Company Number. |
	--------------------------*/
	if ( LCHECK( "compno" ) )
	{
		if (SRCH_KEY)
		{
			show_comr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy(local_rec.cur_co,comr_rec.mr_co_no);
		cc = find_rec("comr", &comr_rec, COMPARISON, "w");
		if (cc)
		{
			/*--------------------
			| Company not found. |
			--------------------*/
			errmess( ML(mlStdMess130) );
			sleep(3);
			return(1);
		}
		entry_exit = 1;

		DSP_FLD( "coname" );
		DSP_FLD( "next1" );
		DSP_FLD( "next2" );
		DSP_FLD( "next3" );
		DSP_FLD( "next4" );
		DSP_FLD( "next5" );
		DSP_FLD( "next6" );
		DSP_FLD( "next7" );
		DSP_FLD( "next8" );

		get_branches();
		return(0);
	}
	return(0);
}

int
get_branches (void)
{
	scn_set(2);
	lcount[2] = 0;

	strcpy(esmr_rec.es_co_no, comr_rec.mr_co_no);
	strcpy(esmr_rec.es_br_no, "  ");

	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp(esmr_rec.es_co_no,comr_rec.mr_co_no))
	{
		num[ 0 ] = esmr_rec.es_num[ 0 ];
		num[ 1 ] = esmr_rec.es_num[ 1 ];
		num[ 2 ] = esmr_rec.es_num[ 2 ];
		num[ 3 ] = esmr_rec.es_num[ 3 ];
		num[ 4 ] = esmr_rec.es_num[ 4 ];
		num[ 5 ] = esmr_rec.es_num[ 5 ];
		num[ 6 ] = esmr_rec.es_num[ 6 ];
		num[ 7 ] = esmr_rec.es_num[ 7 ];

		putval( lcount[2]++ );

		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lcount[2] > MAXLINES) 
			break;
		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
		
	scn_set(1);

	if ( !lcount[2] )
		return(1);
	
	return(0);
}

int
update (void)
{
	int 	wk_line;

	cc = abc_update("comr",&comr_rec);
	if (cc) 
		sys_err("Error in comr During (DBUPDATE)", cc, PNAME);
			
	/*---------------------------------------------------------
	| Set to Tabular screen(s) to update coln (Order details) |
	---------------------------------------------------------*/
	scn_set(2);

	/*---------------------------
	| Now generate order lines. |
	---------------------------*/
	print_at(0,0, ML(mlStdMess035) );
	fflush(stdout);
	for (wk_line = 0;wk_line < lcount[2];wk_line++) 
	{
		getval(wk_line);

		strcpy(esmr_rec.es_co_no,comr_rec.mr_co_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"u");
		if (cc)
			continue;

		esmr_rec.es_num[ 0 ]  = num[ 0 ];
		esmr_rec.es_num[ 1 ]  = num[ 1 ];
		esmr_rec.es_num[ 2 ]  = num[ 2 ];
		esmr_rec.es_num[ 3 ]  = num[ 3 ];
		esmr_rec.es_num[ 4 ]  = num[ 4 ];
		esmr_rec.es_num[ 5 ]  = num[ 5 ];
		esmr_rec.es_num[ 6 ]  = num[ 6 ];
		esmr_rec.es_num[ 7 ]  = num[ 7 ];

		cc = abc_update("esmr",&esmr_rec);
		if (cc) 
			sys_err("Error in esmr During (DBUPDATE)", cc, PNAME);

	}
	abc_unlock("esmr");
	scn_set(1);
	return(0);
}

void
show_comr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Co No", "#Company Name");

	sprintf (comr_rec.mr_co_no, "%-2.2s", key_val);
	cc = find_rec ("comr", &comr_rec, GTEQ, "r");
	while (!cc && !strncmp (comr_rec.mr_co_no, key_val, strlen (key_val)))
	{
		cc = save_rec (comr_rec.mr_co_no, comr_rec.mr_co_name);
		if (cc)
			break;

		cc = find_rec ("comr", &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.mr_co_no, "%-2.2s", temp_str);
	cc = find_rec ("comr", &comr_rec, COMPARISON, "r");
}

int
heading(
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		swide();
		clear();

		/*---------------------------------
		| Company Page Number Maintenence |
		---------------------------------*/
		sprintf (err_str, " %s ", ML(mlMenuMess145) );
		rv_pr( err_str, 50,0,1);

		move(0,1);
		line(132);

		if (scn == 1)
		{
			box(0,3,132,11);
			move(1,6);
			line(131);
		}

		move(0,22);
		line(132);
		scn_write(scn);
		line_cnt = 0;
		move(0,0);
	}
	return (EXIT_SUCCESS);
}

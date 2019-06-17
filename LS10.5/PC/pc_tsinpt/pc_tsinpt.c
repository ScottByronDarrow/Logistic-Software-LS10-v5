/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_tsinpt.c    )                                 |
|  Program Desc  : ( Production Control Time Entry.               )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, pcat, pcln, pcwc,     ,     ,     ,   |
|                :  pcwo, prmr, prvr, rgrs,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :  pcat,     ,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : 27/02/92        | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
|  Date Modified : (04/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (13/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (02/04/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (20/04/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (29/07/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/09/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (17/11/93)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (21/01/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (17/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (18/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (18/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (31/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (04/09/97)      | Modified  by : Ana Marie Tario.  |
|                                                                     |
|  Comments      : (04/03/92) - Updated for TIMETYPE and changes to   |
|                : MFG schemas.                                       |
|  (09/03/92)    : Remove reference to pcln_hhgr_hash.                |
|  (13/03/92)    : Add run and cleanup times to pcat.                 |
|  (02/04/92)    : Fix defaults on fields.                            |
|  (20/04/92)    : Disallow edit of times for Closed Works Orders.    |
|  (29/07/92)    : Enable entry on a per-employee basis.              |
|                : S/C DPL 7478                                       |
|  (30/09/92)    : Finish of per-employee data entry. DPL 7867.       |
|  (17/11/93)    : DPL 9889 - Ignores pcat records if stat_flag = 'U'.|
|  (21/01/94)    : DPL 9673 - can enter w/o no or batch no, to bring  |
|                : up order details.                                  |
|  (17/03/94)    : DPL 10621 - exit at batch no, display item on scn. |
|  (18/03/94)    : DPL 10621 - Not displaying item on entry.          |
|  (18/03/94)    : DPL 10482 - implementing system generated works    |
|                : order numbering.                                   |
|  (31/03/94)    : DPL 10482. Fix core dump.                          |
|  (05/10/94)    : PSL 11299 - mfg cutover - changes to srch_pcwo2.h  |
|                : display item details.                              |
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                                                                     |
| $Log: pc_tsinpt.c,v $
| Revision 5.5  2002/07/24 08:39:00  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:50:07  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/03 04:20:13  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:14:51  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:08  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:37  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:18  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.14  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.13  1999/10/01 07:48:58  scott
| Updated for standard function calls.
|
| Revision 1.12  1999/09/29 10:11:40  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/17 08:26:26  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.10  1999/09/13 07:03:19  marlene
| *** empty log message ***
|
| Revision 1.9  1999/09/09 06:12:36  marlene
| *** empty log message ***
|
| Revision 1.8  1999/06/17 07:40:49  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_tsinpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_tsinpt/pc_tsinpt.c,v 5.5 2002/07/24 08:39:00 scott Exp $";

#define	MAXLINES	50
#define MAXWIDTH	205
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>
#include <twodec.h>

#define	DAY_MINS	1440

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] =
	{
		{"comm_term"}, 
		{"comm_co_no"}, 
		{"comm_co_short"}, 
		{"comm_est_no"}, 
		{"comm_est_short"}, 
		{"comm_cc_no"}, 
		{"comm_cc_short"}, 
	};

	int	comm_no_fields = 7;

	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_short [16];
		char	test_no [3];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_short [10];
	} comm_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"}, 
		{"inmr_hhbr_hash"}, 
		{"inmr_description"}, 
	};

	int	inmr_no_fields = 3;

	struct
	{
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		char	mr_description [41];
	} inmr_rec;

	/*======================================
	| Production Control Actual Times File |
	======================================*/
	struct dbview pcat_list [] =
	{
		{"pcat_date"}, 
		{"pcat_hhmr_hash"}, 
		{"pcat_hhrs_hash"}, 
		{"pcat_hhwo_hash"}, 
		{"pcat_seq_no"}, 
		{"pcat_hhwc_hash"}, 
		{"pcat_start_time"}, 
		{"pcat_setup"}, 
		{"pcat_run"}, 
		{"pcat_clean"}, 
		{"pcat_comment"}, 
		{"pcat_stat_flag"}, 
	};

	int	pcat_no_fields = 12;

	struct
	{
		long	at_date;
		long	at_hhmr_hash;
		long	at_hhrs_hash;
		long	at_hhwo_hash;
		int		at_seq_no;
		long	at_hhwc_hash;
		long	at_start_time;
		long	at_setup;
		long	at_run;
		long	at_clean;
		char	at_comment [41];
		char	at_stat_flag [2];
	} pcat_rec;

	/*==========================
	| Routing Line detail File |
	==========================*/
	struct dbview pcln_list [] =
	{
		{"pcln_seq_no"}, 
		{"pcln_hhwc_hash"}, 
		{"pcln_hhrs_hash"}, 
		{"pcln_line_no"}, 
		{"pcln_hhwo_hash"}, 
	};

	int	pcln_no_fields = 5;

	struct
	{
		int		ln_seq_no;
		long	ln_hhwc_hash;
		long	ln_hhrs_hash;
		int		ln_line_no;
		long	ln_hhwo_hash;
	} pcln_rec;

	/*=======================
	| Work Centre Code file |
	=======================*/
	struct dbview pcwc_list [] =
	{
		{"pcwc_hhwc_hash"}, 
		{"pcwc_co_no"}, 
		{"pcwc_br_no"}, 
		{"pcwc_work_cntr"}, 
		{"pcwc_name"}, 
	};

	int	pcwc_no_fields = 5;

	struct
	{
		long	wc_hhwc_hash;
		char	wc_co_no [3];
		char	wc_br_no [3];
		char	wc_work_cntr [9];
		char	wc_name [41];
	} pcwc_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"}, 
		{"pcwo_br_no"}, 
		{"pcwo_wh_no"}, 
		{"pcwo_order_no"}, 
		{"pcwo_hhwo_hash"}, 
		{"pcwo_hhbr_hash"}, 
		{"pcwo_rtg_seq"}, 
		{"pcwo_order_status"}, 
		{"pcwo_batch_no"}, 
	};

	int	pcwo_no_fields = 9;

	struct
	{
		char	wo_co_no [3];
		char	wo_br_no [3];
		char	wo_wh_no [3];
		char	wo_order_no [8];
		long	wo_hhwo_hash;
		long	wo_hhbr_hash;
		int		wo_rtg_seq;
		char	wo_order_status [2];
		char	wo_batch_no [11];
	} pcwo_rec;

	/*==============================
	| PayRoll employee Master file |
	==============================*/
	struct dbview prmr_list [] =
	{
		{"prmr_hhmr_hash"}, 
		{"prmr_co_no"}, 
		{"prmr_br_no"}, 
		{"prmr_code"}, 
		{"prmr_hhrs_hash"}, 
		{"prmr_name"}, 
	};

	int	prmr_no_fields = 6;

	struct
	{
		long	mr_hhmr_hash;
		char	mr_co_no [3];
		char	mr_br_no [3];
		char	mr_code [9];
		long	mr_hhrs_hash;
		char	mr_name [41];
	} prmr_rec;

	/*==============================
	| PayRoll Valid Resources file |
	==============================*/
	struct dbview prvr_list [] =
	{
		{"prvr_hhmr_hash"}, 
		{"prvr_hhrs_hash"}, 
	};

	int	prvr_no_fields = 2;

	struct
	{
		long	vr_hhmr_hash;
		long	vr_hhrs_hash;
	} prvr_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list [] =
	{
		{"rgrs_hhrs_hash"}, 
		{"rgrs_co_no"}, 
		{"rgrs_br_no"}, 
		{"rgrs_code"}, 
		{"rgrs_desc"}, 
	};

	int	rgrs_no_fields = 5;

	struct
	{
		long	rs_hhrs_hash;
		char	rs_co_no [3];
		char	rs_br_no [3];
		char	rs_code [9];
		char	rs_desc [41];
	} rgrs_rec;

	char	*comm	= "comm", 
			*data	= "data", 
			*inmr	= "inmr", 
			*pcat	= "pcat", 
			*pcln	= "pcln", 
			*pcwc	= "pcwc", 
			*pcwo	= "pcwo", 
			*pcwo2	= "pcwo2", 
			*pcwo3	= "pcwo3", 
			*prmr	= "prmr", 
			*prvr	= "prvr", 
			*rgrs	= "rgrs", 
			*rgrs2	= "rgrs2";

   	int  	clear_ok;
	long	time_res;
	int		order_flag;
	int		batch_exit;
	int		PC_GEN;

struct	storeRec {
	long	setup;
	long	run;
	long	clean;
	char	item_no [17];
	char	item_desc [41];
} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];

	long	date;
	long	elpsd_time;
	long	curr_time;
	long	dflt_rsrc_hash;
	char	dflt_rsrc [9];

	char	wo_number [8];
	char	batch_no [11];
	char	item_no [17];
	char	item_desc [41];
	long	hhwo_hash;
	int		seq_no;
	char	wrk_cntr [9];
	long	hhwc_hash;
	char	wcntr_desc [26];
	long	st_time;
	long	setup;
	long	run;
	long	clean;
	char	comment [41];
	int		edit_ok;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "date_entry", 	 2, 15, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, " Date         :", " ", 
		 NE, NO, JUSTRIGHT, "", "",  (char *)&local_rec.date}, 
	{1, LIN, "prmr", 		 3, 15, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", " ", " Employee     :", " ", 
		 NE, NO,  JUSTLEFT, "", "", prmr_rec.mr_code}, 
	{1, LIN, "prmr_name", 	 3, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", prmr_rec.mr_name}, 
	{1, LIN, "rsrc", 		 4, 15, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", local_rec.dflt_rsrc, " Resource     :", " ", 
		 NE, NO,  JUSTLEFT, "", "", rgrs_rec.rs_code}, 
	{1, LIN, "rsrc_desc", 	 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", rgrs_rec.rs_desc}, 
	{1, LIN, "elapsed_time", 	 5, 15, TIMETYPE, 
		"NNN:NN", "          ", 
		" ", " ", " Elapsed Time :", "", 
		 NA, NO,  JUSTLEFT, "", "",  (char *)&local_rec.elpsd_time}, 

	{2, TAB, "wo_number", 	 MAXLINES, 0, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", " ", "W/O No.", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.wo_number}, 
	{2, TAB, "batch_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Batch  No.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.batch_no}, 
	{2, TAB, "item_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{2, TAB, "item_desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{2, TAB, "hhwo_hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTRIGHT, "", "",  (char *)&local_rec.hhwo_hash}, 
	{2, TAB, "seq_no", 	 0, 0, INTTYPE, 
		"NNN", "          ", 
		" ", "", "Seq", " ", 
		YES, NO, JUSTRIGHT, "", "",  (char *)&local_rec.seq_no}, 
	{2, TAB, "wrk_cntr", 	 0, 0, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "", "Work Ctr", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.wrk_cntr}, 
	{2, TAB, "hhwc_hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTRIGHT, "", "",  (char *)&local_rec.hhwc_hash}, 
	{2, TAB, "wcntr_desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "    Work  Center  Name   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.wcntr_desc}, 
	{2, TAB, "st_time", 	 0, 0, TIMETYPE, 
		"NN:NN", "          ", 
		" ", " ", "St Time", " ", 
		YES, NO,  JUSTLEFT, "0", "1439",  (char *)&local_rec.st_time}, 
	{2, TAB, "setup", 	 0, 0, TIMETYPE, 
		"NNN:NN", "          ", 
		" ", "0", " Setup ", " ", 
		YES, NO,  JUSTLEFT, "0", "10080",  (char *)&local_rec.setup}, 
	{2, TAB, "run", 		 0, 0, TIMETYPE, 
		"NNN:NN", "          ", 
		" ", "0", "  Run  ", " ", 
		YES, NO,  JUSTLEFT, "0", "10080",  (char *)&local_rec.run}, 
	{2, TAB, "clean", 	 0, 0, TIMETYPE, 
		"NNN:NN", "          ", 
		" ", "0", " Clean ", " ", 
		YES, NO,  JUSTLEFT, "0", "10080",  (char *)&local_rec.clean}, 
	{2, TAB, "comment", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                 Comment                ", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.comment}, 
	{2, TAB, "edit_ok", 	 0, 0, INTTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", " ", 
		ND, NO,  JUSTRIGHT, "", "",  (char *)&local_rec.edit_ok}, 

	{0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <srch_pcwo2.h>
/*========================
| function prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int heading (int scn);
int spec_valid (int field);
int delete_line (void);
void tab_other (int line_no);
void calc_time (int uppr_bnd);
int load_pcat (void);
void update (void);
void SrchPrmr (char *key_val);
void SrchRgrs (char *key_val);
void srch_seq (char *key_val);
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc,
 char *argv[])
{
	int 	i;
	char	*sptr,
			*chk_env (char *);
			
	sptr = get_env ("PC_TIME_RES");
	time_res = (sptr) ? atol (sptr) : 5;

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		PC_GEN = TRUE;

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
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
		batch_exit = TRUE;

		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		clear_ok = TRUE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		for (i = 0; i < MAXLINES; i ++)
		{
			store [i].setup = 0L;
			store [i].run = 0L;
			store [i].clean = 0L;
			sprintf (store [i].item_no, "%16.16s", " ");
			sprintf (store [i].item_desc, "%-40.40s", " ");
		}
		load_pcat ();
		clear_ok = FALSE;
		heading (2);
		clear_ok = TRUE;
		scn_display (2);
		edit (2);
		if (batch_exit)
			prog_exit = FALSE;
		if (restart)
			continue;

		update ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);
	abc_alias (pcwo3, pcwo);
	abc_alias (rgrs2, rgrs);

	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (pcat,  pcat_list, pcat_no_fields, "pcat_id_no");
	open_rec (pcln,  pcln_list, pcln_no_fields, "pcln_id_no");
	open_rec (pcwc,  pcwc_list, pcwc_no_fields, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, pcwo_no_fields, "pcwo_hhwo_hash");
	open_rec (pcwo3, pcwo_list, pcwo_no_fields, "pcwo_id_no3");
	open_rec (prmr,  prmr_list, prmr_no_fields, "prmr_id_no");
	open_rec (prvr,  prvr_list, prvr_no_fields, "prvr_id_no");
	open_rec (rgrs,  rgrs_list, rgrs_no_fields, "rgrs_id_no");
	open_rec (rgrs2, rgrs_list, rgrs_no_fields, "rgrs_hhrs_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (pcat);
	abc_fclose (pcln);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (prmr);
	abc_fclose (prvr);
	abc_fclose (rgrs);
	abc_fclose (rgrs2);

	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		if (clear_ok)
		{
			swide ();
			clear ();
		}

		rv_pr (ML(mlPcMess028), 40, 0, 1);
		move (0, 1);
		line (132);

		if (scn == 1)
		{
			move (0, 1);
			line (130);

			box (0, 1, 132, 4);
		}

		if (scn == 2)
		{
			heading (1);
			scn_display (1);

			tab_col = 0;
			tab_row = 7;
			box (tab_col, tab_row - 1, 132, 12);
			move (0, 6); PGCHAR (10);
			move (131, 6); PGCHAR (11);
		}

		move (0, 21);
		line (132);
/*
		print_at (22, 0, 
			"Co. : %2.2s %-15.15s  Br. : %2.2s %-15.15s  Wh. : %2.2s %-9.9s", 
			comm_rec.tco_no, 
			comm_rec.tco_short, 
			comm_rec.test_no, 
			comm_rec.test_short, 
			comm_rec.tcc_no, 
			comm_rec.tcc_short);*/
		print_at (22, 0, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_short );
		print_at (22,40, ML(mlStdMess039),comm_rec.test_no,comm_rec.test_short);
		print_at (22,60, ML(mlStdMess099),comm_rec.tcc_no,comm_rec.tcc_short);
	

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("prmr"))
	{
		if (SRCH_KEY)
		{
			SrchPrmr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (prmr_rec.mr_co_no, comm_rec.tco_no);
		strcpy (prmr_rec.mr_br_no, comm_rec.test_no);
		cc = find_rec (prmr, &prmr_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, 
				"\007 Employee %s is NOT on file!! ", 
				prmr_rec.mr_code);
			print_mess (err_str);*/
			print_mess(ML(mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("prmr_name");

		cc = find_hash (rgrs2, &rgrs_rec, EQUAL, "r", prmr_rec.mr_hhrs_hash);
		if (cc)
		{
			strcpy (local_rec.dflt_rsrc, "        ");
			local_rec.dflt_rsrc_hash = 0L;
		}
		else
		{
			strcpy (local_rec.dflt_rsrc, rgrs_rec.rs_code);
			local_rec.dflt_rsrc_hash = rgrs_rec.rs_hhrs_hash;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rsrc"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (rgrs_rec.rs_co_no, comm_rec.tco_no);
		strcpy (rgrs_rec.rs_br_no, comm_rec.test_no);
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, 
				"\007 Resource %s is NOT on file!! ", 
				rgrs_rec.rs_code);
			print_mess (err_str);*/
			print_mess(ML(mlPcMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		prvr_rec.vr_hhmr_hash = prmr_rec.mr_hhmr_hash;
		prvr_rec.vr_hhrs_hash = rgrs_rec.rs_hhrs_hash;
		cc = find_rec (prvr, &prvr_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, 
				"\007 Resource %s is NOT valid for this employee!! ", 
				rgrs_rec.rs_code);
			print_mess (err_str);*/
			print_mess(ML(mlPcMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("rsrc_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wo_number"))
	{
		if ((dflt_used || last_char == DELLINE) && prog_status != ENTRY)
			return (delete_line ());

		order_flag = TRUE;
		if (dflt_used && prog_status == ENTRY)
		{
			order_flag = FALSE;
			FLD ("batch_no") = YES;
			return (EXIT_SUCCESS);
		}
		else
			FLD ("batch_no") = NA;

		if (SRCH_KEY)
		{
			srch_order (temp_str, "RC", comm_rec.test_no, comm_rec.tcc_no);
			return (EXIT_SUCCESS);
		}

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (PC_GEN)
			sprintf (pcwo_rec.wo_order_no,
				"%-7.7s",
				zero_pad (local_rec.wo_number, 7));
		else
			sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.wo_number);
		cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		if (cc)
		{
/*
			sprintf (err_str, 
					" Works Order Does Not Exist - Order No [%s]", 
					pcwo_rec.wo_order_no);
			print_mess (err_str);*/
			print_mess(ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		switch (pcwo_rec.wo_order_status [0])
		{
		case	'R':
		case	'C':
			break;

		default:
/*
			print_mess ("\007 Sorry, you may only choose R (eleased) or C (losing) jobs! ");*/
			print_mess(ML(mlPcMess133));
			sleep (sleepTime);
			return (EXIT_FAILURE);
			break;
		}

		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");

		local_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
		strcpy (local_rec.batch_no, pcwo_rec.wo_batch_no);
		DSP_FLD ("batch_no");
		strcpy (store [line_cnt].item_no, inmr_rec.mr_item_no);
		strcpy (store [line_cnt].item_desc, inmr_rec.mr_description);
/*
		print_at (19, 20,
				"%R Mfg Item : %-16.16s   Description : %-40.40s ",
				store [line_cnt].item_no,
				store [line_cnt].item_desc);*/
		print_at (19, 20,ML(mlPcMess027),store [line_cnt].item_no,
				store [line_cnt].item_desc);

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("batch_no"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			batch_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (order_flag)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_batch (temp_str, "RC", comm_rec.test_no, comm_rec.tcc_no);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.batch_no, "          "))
		{
/*
			print_mess (" Must Enter A Works Order Number Or A Batch Number ");*/
			print_mess(ML(mlPcMess134));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		strcpy (pcwo_rec.wo_batch_no, local_rec.batch_no);
		cc = find_rec (pcwo3, &pcwo_rec, COMPARISON, "r");
		if (cc)
		{
/*
			sprintf (err_str, 
					" Works Order Does Not Exist - Batch No [%s] ", 
					pcwo_rec.wo_batch_no);
			print_mess (err_str);*/
			print_mess(ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.batch_no, "%-10.10s", " ");
			return (EXIT_FAILURE);
		}

		switch (pcwo_rec.wo_order_status [0])
		{
		case	'R':
		case	'C':
			break;

		default:
/*
			print_mess ("\007 Sorry, you may only choose R (eleased) or C (losing) jobs! ");*/
			print_mess(ML(mlPcMess133));
			sleep (sleepTime);
			return (EXIT_FAILURE);
			break;
		}

		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");

		local_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
		strcpy (local_rec.wo_number, pcwo_rec.wo_order_no);
		DSP_FLD ("wo_number");
		strcpy (store [line_cnt].item_no, inmr_rec.mr_item_no);
		strcpy (store [line_cnt].item_desc, inmr_rec.mr_description);
		print_at (19, 20,ML(mlPcMess027),store [line_cnt].item_no,
				store [line_cnt].item_desc);

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("seq_no"))
	{
		if (SRCH_KEY)
		{
			srch_seq (temp_str);
			return (EXIT_SUCCESS);
		}

		pcln_rec.ln_hhwo_hash = pcwo_rec.wo_hhwo_hash;
		pcln_rec.ln_seq_no = local_rec.seq_no;
		pcln_rec.ln_line_no = 0;
		cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
		if (cc || 
		     pcln_rec.ln_hhwo_hash != pcwo_rec.wo_hhwo_hash ||
		     pcln_rec.ln_seq_no != local_rec.seq_no)
		{
/*
			print_mess ("\007 Routing Sequence Does Not Exist ");*/
			print_mess(ML(mlPcMess129));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = find_hash (pcwc, &pcwc_rec, COMPARISON, "r", pcln_rec.ln_hhwc_hash);
		if (cc)
		{
			strcpy (pcwc_rec.wc_work_cntr, "UNKNOWN ");
			sprintf (pcwc_rec.wc_name, "%-40.40s", "UNKNOWN");
		}

		local_rec.hhwc_hash = pcwc_rec.wc_hhwc_hash;
		sprintf (local_rec.wrk_cntr, "%-8.8s", pcwc_rec.wc_work_cntr);
		sprintf (local_rec.wcntr_desc, "%-25.25s", pcwc_rec.wc_name);
		DSP_FLD ("wrk_cntr");
		DSP_FLD ("wcntr_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_time"))
	{
		if (dflt_used)
		{
			local_rec.st_time = rnd_time (atot (TimeHHMM()), time_res);
			return (EXIT_SUCCESS);
		}
	
		local_rec.st_time = rnd_time (local_rec.st_time, time_res);
		if (local_rec.st_time >= DAY_MINS)
			local_rec.st_time = 0L;

		DSP_FLD ("st_time");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("setup"))
	{
		local_rec.setup = rnd_time (local_rec.setup, time_res);
		store [line_cnt].setup = local_rec.setup;

		calc_time (line_cnt + 1);
		scn_set (2);

		DSP_FLD ("setup");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("run"))
	{
		local_rec.run = rnd_time (local_rec.run, time_res);
		store [line_cnt].run = local_rec.run;

		calc_time (line_cnt + 1);
		scn_set (2);

		DSP_FLD ("run");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("clean"))
	{
		local_rec.clean = rnd_time (local_rec.clean, time_res);
		store [line_cnt].clean = local_rec.clean;

		calc_time (line_cnt + 1);
		scn_set (2);

		DSP_FLD ("clean");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);             
}

int
delete_line (
 void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*print_mess (" Cannot Delete Lines on Entry ");*/
		print_mess(ML(mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		store [line_cnt].setup = store [line_cnt + 1].setup;
		store [line_cnt].run   = store [line_cnt + 1].run;
		store [line_cnt].clean = store [line_cnt + 1].clean;
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	calc_time (lcount [2]);
	scn_set (2);

	store [lcount [2]].setup = 0L;
	store [lcount [2]].run   = 0L;
	store [lcount [2]].clean = 0L;

	return (EXIT_SUCCESS);
}

void
tab_other (
 int line_no)
{
	if (local_rec.edit_ok || line_no >= lcount [2])
	{
		FLD ("wo_number") = YES;
		if (prog_status != ENTRY)
			FLD ("batch_no")  = YES;
		FLD ("seq_no")    = YES;
		FLD ("st_time")   = YES;
		FLD ("setup")     = YES;
		FLD ("run")       = YES;
		FLD ("clean")     = YES;
		FLD ("comment")   = NO;
	}
	else
	{
		FLD ("wo_number") = NA;
		FLD ("batch_no")  = NA;
		FLD ("seq_no")    = NA;
		FLD ("st_time")   = NA;
		FLD ("setup")     = NA;
		FLD ("run")       = NA;
		FLD ("clean")     = NA;
		FLD ("comment")   = NA;
	}

	if (line_no <= lcount [2])
	{
		print_at (19, 20,ML(mlPcMess027),store [line_cnt].item_no,
				store [line_cnt].item_desc);
	}
}

/*------------------------
| Calculate elapsed time |
| based on store array   |
------------------------*/
void
calc_time (
 int uppr_bnd)
{
	int		i;

	local_rec.elpsd_time = 0L;

	for (i = 0; i < uppr_bnd; i++)
	{
		local_rec.elpsd_time += store [i].setup;
		local_rec.elpsd_time += store [i].run;
		local_rec.elpsd_time += store [i].clean;
	}

	scn_set (1);
	DSP_FLD ("elapsed_time");
}

/*-----------------------------------
| Load pcat records into TAB screen |
-----------------------------------*/
int
load_pcat (
 void)
{
	scn_set (2);
	lcount [2] = 0;
	
	pcat_rec.at_date      = local_rec.date;
	pcat_rec.at_hhmr_hash = prmr_rec.mr_hhmr_hash;
	pcat_rec.at_hhrs_hash = rgrs_rec.rs_hhrs_hash;
	pcat_rec.at_hhwo_hash = 0L;
	pcat_rec.at_seq_no    = 0;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "r");
	while
	 (
		!cc &&
		pcat_rec.at_date	== local_rec.date &&
		pcat_rec.at_hhmr_hash	== prmr_rec.mr_hhmr_hash &&
		pcat_rec.at_hhrs_hash	== rgrs_rec.rs_hhrs_hash
	)
	{
		if (pcat_rec.at_stat_flag [0] == 'U')
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}

		cc = find_hash (pcwo2, &pcwo_rec, COMPARISON, "r", 
			       pcat_rec.at_hhwo_hash);
		if (cc)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.wo_number, "%-7.7s", pcwo_rec.wo_order_no);
		strcpy (local_rec.batch_no, pcwo_rec.wo_batch_no);
		local_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
		local_rec.seq_no    = pcat_rec.at_seq_no;
		if (pcwo_rec.wo_order_status [0] == 'C' ||
		    pcwo_rec.wo_order_status [0] == 'R')
		{
			local_rec.edit_ok = TRUE;
		}
		else
			local_rec.edit_ok = FALSE;

		cc = find_hash (pcwc, &pcwc_rec, COMPARISON, "r", pcat_rec.at_hhwc_hash);
		if (cc)
		{
			strcpy (pcwc_rec.wc_work_cntr, "UNKNOWN ");
			sprintf (pcwc_rec.wc_name, "%-40.40s", "UNKNOWN");
			local_rec.hhwc_hash = 0L;
		}

		sprintf (local_rec.wrk_cntr, "%-8.8s", pcwc_rec.wc_work_cntr);
		sprintf (local_rec.wcntr_desc, "%-25.25s", pcwc_rec.wc_name);
		local_rec.hhwc_hash = pcwc_rec.wc_hhwc_hash;

		/*-------------------
		| Time and Duration |
		-------------------*/
		local_rec.st_time  = pcat_rec.at_start_time;
		local_rec.setup = pcat_rec.at_setup;
		local_rec.run   = pcat_rec.at_run;
		local_rec.clean = pcat_rec.at_clean;
		store [lcount [2]].setup   = pcat_rec.at_setup;
		store [lcount [2]].run     = pcat_rec.at_run;
		store [lcount [2]].clean   = pcat_rec.at_clean;

		sprintf (local_rec.comment, "%-40.40s", pcat_rec.at_comment);

		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (store [lcount [2]].item_no, inmr_rec.mr_item_no);
		strcpy (store [lcount [2]].item_desc, inmr_rec.mr_description);

		putval (lcount [2]++);

		cc = find_rec (pcat, &pcat_rec, NEXT, "r");
	}

	calc_time (lcount [2]);

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Update pcat records from TAB screen |
-------------------------------------*/
void
update (
 void)
{
	int		i;

	/*--------------------
	| Delete Old Records |
	--------------------*/
	pcat_rec.at_date      = local_rec.date;
	pcat_rec.at_hhmr_hash = prmr_rec.mr_hhmr_hash;
	pcat_rec.at_hhrs_hash = rgrs_rec.rs_hhrs_hash;
	pcat_rec.at_hhwo_hash = 0L;
	pcat_rec.at_seq_no    = 0;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	while
	 (
		!cc &&
		pcat_rec.at_date	== local_rec.date &&
		pcat_rec.at_hhmr_hash	== prmr_rec.mr_hhmr_hash &&
		pcat_rec.at_hhrs_hash	== rgrs_rec.rs_hhrs_hash
	)
	{
		cc = abc_delete (pcat);
		if (cc)
			file_err (cc, pcat, "DBDELETE");

		pcat_rec.at_date      = local_rec.date;
		pcat_rec.at_hhmr_hash = prmr_rec.mr_hhmr_hash;
		pcat_rec.at_hhrs_hash = rgrs_rec.rs_hhrs_hash;
		pcat_rec.at_hhwo_hash = 0L;
		pcat_rec.at_seq_no    = 0;
		cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	}
	abc_unlock (pcat);

	/*--------------------
	| Create New Records |
	--------------------*/
	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);

		pcat_rec.at_date       = local_rec.date;
		pcat_rec.at_hhmr_hash  = prmr_rec.mr_hhmr_hash;
		pcat_rec.at_hhrs_hash  = rgrs_rec.rs_hhrs_hash;
		pcat_rec.at_hhwo_hash  = local_rec.hhwo_hash;
		pcat_rec.at_seq_no     = local_rec.seq_no;
		pcat_rec.at_hhwc_hash  = local_rec.hhwc_hash;
		pcat_rec.at_start_time = local_rec.st_time;
		pcat_rec.at_setup      = local_rec.setup;
		pcat_rec.at_run        = local_rec.run;
		pcat_rec.at_clean      = local_rec.clean;
		sprintf (pcat_rec.at_comment, "%-40.40s", local_rec.comment);
		strcpy (pcat_rec.at_stat_flag, "0");

		cc = abc_add (pcat, &pcat_rec);
		if (cc)
			file_err (cc, pcat, "DBADD");
	}
}

/*=======================
| Search for Employee	|
=======================*/
void
SrchPrmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Name");
	strcpy (prmr_rec.mr_co_no, comm_rec.tco_no);
	strcpy (prmr_rec.mr_br_no, comm_rec.test_no);
	sprintf (prmr_rec.mr_code, "%-8.8s", key_val);
	cc = find_rec (prmr, &prmr_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (prmr_rec.mr_co_no, comm_rec.tco_no) &&
		!strcmp (prmr_rec.mr_br_no, comm_rec.test_no) &&
		!strncmp (prmr_rec.mr_code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (prmr_rec.mr_code, prmr_rec.mr_name);
		if (cc)
			break;
		cc = find_rec (prmr, &prmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (prmr_rec.mr_co_no, comm_rec.tco_no);
	strcpy (prmr_rec.mr_br_no, comm_rec.test_no);
	sprintf (prmr_rec.mr_code, "%-8.8s", temp_str);
	cc = find_rec (prmr, &prmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, prmr, "DBFIND");
}

/*=======================
| Search for Resource	|
=======================*/
void
SrchRgrs (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.rs_co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.rs_br_no, comm_rec.test_no);
	sprintf (rgrs_rec.rs_code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (rgrs_rec.rs_co_no, comm_rec.tco_no) &&
		!strcmp (rgrs_rec.rs_br_no, comm_rec.test_no) &&
		!strncmp (rgrs_rec.rs_code, key_val, strlen (key_val)))
	{
		prvr_rec.vr_hhmr_hash = prmr_rec.mr_hhmr_hash;
		prvr_rec.vr_hhrs_hash = rgrs_rec.rs_hhrs_hash;
		cc = find_rec (prvr, &prvr_rec, EQUAL, "r");
		if (!cc)
		{
			cc = save_rec (rgrs_rec.rs_code, rgrs_rec.rs_desc);
			if (cc)
				break;
		}
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.rs_co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.rs_br_no, comm_rec.test_no);
	sprintf (rgrs_rec.rs_code, "%-8.8s", temp_str);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");
}

/*---------------------------
| Search for valid sequence |
---------------------------*/
void
srch_seq (
 char *key_val)
{
	char	seq_str [10];
	int		seq_no;

	work_open ();
	save_rec ("#Seq.", "#Work Center");
	pcln_rec.ln_hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.ln_seq_no = 0;
	pcln_rec.ln_line_no = 0;

	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	seq_no = -1;
	while (!cc &&
	        pcln_rec.ln_hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		if (pcln_rec.ln_seq_no != seq_no)
		{
			sprintf (seq_str, "%2d ", pcln_rec.ln_seq_no);

			cc = find_hash (pcwc, &pcwc_rec, COMPARISON, "r", 
			               pcln_rec.ln_hhwc_hash);
			if (cc)
				strcpy (pcwc_rec.wc_work_cntr, "UNKNOWN ");

			cc = save_rec (seq_str, pcwc_rec.wc_work_cntr);
			if (cc)
				break;

			seq_no = pcln_rec.ln_seq_no;
		}
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}
	
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	local_rec.seq_no = atoi (temp_str);
}


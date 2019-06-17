/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_recprt.c    )                                 |
|  Program Desc  : ( Works Order Costing Report.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inei, inmr, inum, pcln,     ,     ,     ,   |
|                :  pcms, pcrq, pcwc, pcwo, rgrs,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (06/05/92)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (08/06/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (03/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (25/03/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (11/05/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (09/06/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (08/11/93)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (26/11/93)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (21/01/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (18/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (31/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (08/09/97)      | Modified  by : Marnie Organo.    |
|                                                                     |
|  Comments      :                                                    |
|  (08/06/92)    : Fixed divide by zero error by pcms_rec.qty_issued  |
|  (03/08/92)    : Multiply matl_qty by matl_wst_pc. S/C KIL 7481     |
|  (25/03/93)    : DPL 8454. Changed such that ACTUAL qty used IF the |
|                : Works Order has been closed (order_status=Z)       |
|  (11/05/93)    : DPL 8454. Changed such that use_star is ignored if |
|                : Works Order has been closed (order_status=Z)       |
|  (09/06/93)    : DPL 8454. To take into account outer_size for costs|
|  (08/11/93)    : DPL 10030 - Cost of unit by pricing / cost         |
|                : conversion factor. Global Mods.                    |
|  (26/11/93)    : DPL 10030 - Displays 6 decimal places for Cost     |
|                : per Unit. Divids by the conversion factor to       |
|                : calculate the std cost. (as stated in inum schema) |
|  (21/01/94)    : DPL 9673 - Can enter w/o no or batch no, to access |
|                : details for the report. Will print batch no on rpt.|
|  (18/03/94)    : DPL 10482 - implementing system generated works    |
|                : order numbering.                                   |
|  (31/03/94)    : HGP 10469. Removal of $ signs. Fix core dump       |
|                : introduced in last version.                        |
|  (05/10/94)    : PSL 11299 - Upgrade to ver 9 - mfg cutover - print |
|                : req & rec branch and warehouse.                    |
|  (08/09/97)    : Modified for Multilingual Conversion.              |
|                :                                                    |
|                                                                     |
| $Log: pc_recprt.c,v $
| Revision 5.3  2002/07/17 09:57:29  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:03  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:12  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/12 10:37:47  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.11  1999/09/29 10:11:36  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 08:26:24  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.9  1999/09/13 07:03:16  marlene
| *** empty log message ***
|
| Revision 1.8  1999/09/09 06:12:31  marlene
| *** empty log message ***
|
| Revision 1.7  1999/06/17 07:40:45  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_recprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_recprt/pc_recprt.c,v 5.3 2002/07/17 09:57:29 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<get_lpno.h>
#include	<number.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>

FILE	*fout;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] =
	{
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
	};

	int	comm_no_fields = 10;

	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tco_short [16];
		char	test_no [3];
		char	test_name [41];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_name [41];
		char	tcc_short [10];
	} comm_rec;

	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list [] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_std_cost"},
		{"inei_std_batch"},
		{"inei_min_batch"},
		{"inei_max_batch"},
		{"inei_hzrd_class"},
	};

	int	inei_no_fields = 7;

	struct
	{
		long	hhbr_hash;
		char	est_no [3];
		double	std_cost;
		float	std_batch;
		float	min_batch;
		float	max_batch;
		char	hzrd_class [5];
	} inei_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_description2"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
		{"inmr_outer_size"},
	};

	int	inmr_no_fields = 8;

	struct
	{
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		char	mr_description [41];
		char	mr_description2 [41];
		long	mr_std_uom;
		long	mr_alt_uom;
		float	mr_uom_cfactor;
		float	mr_outer_size;
	} inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list [] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_cnv_fct"},
	};

	int	inum_no_fields = 4;

	struct
	{
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		float	cnv_fct;
	} inum_rec;

	/*==========================
	| Routing Line detail File |
	==========================*/
	struct dbview pcln_list [] =
	{
		{"pcln_hhgr_hash"},
		{"pcln_seq_no"},
		{"pcln_hhwc_hash"},
		{"pcln_hhrs_hash"},
		{"pcln_rate"},
		{"pcln_ovhd_var"},
		{"pcln_ovhd_fix"},
		{"pcln_setup"},
		{"pcln_run"},
		{"pcln_clean"},
		{"pcln_qty_rsrc"},
		{"pcln_instr_no"},
		{"pcln_yld_clc"},
		{"pcln_can_split"},
		{"pcln_line_no"},
		{"pcln_hhwo_hash"},
	};

	int	pcln_no_fields = 16;

	struct
	{
		long	hhgr_hash;
		int		seq_no;
		long	hhwc_hash;
		long	hhrs_hash;
		double	rate;
		double	ovhd_var;
		double	ovhd_fix;
		long	setup;
		long	run;
		long	clean;
		int		qty_rsrc;
		int		instr_no;
		char	yld_clc [5];
		char	can_split [2];
		int		line_no;
		long	hhwo_hash;
	} pcln_rec;

	/*=================================
	| BoM Material Specification File |
	=================================*/
	struct dbview pcms_list [] =
	{
		{"pcms_mabr_hash"},
		{"pcms_uom"},
		{"pcms_matl_qty"},
		{"pcms_matl_wst_pc"},
		{"pcms_iss_seq"},
		{"pcms_uniq_id"},
		{"pcms_amt_issued"},
		{"pcms_qty_issued"},
		{"pcms_act_qty_in"},
		{"pcms_hhwo_hash"},
	};

	int	pcms_no_fields = 10;

	struct
	{
		long	mabr_hash;
		long	uom;
		float	matl_qty;
		float	matl_wst_pc;
		int		iss_seq;
		int		uniq_id;
		double	amt_issued;	/* money field */
		float	qty_issued;
		char	act_qty_in [2];
		long	hhwo_hash;
	} pcms_rec;

	/*===================================
	| Production Control Resource Queue |
	===================================*/
	struct dbview pcrq_list [] =
	{
		{"pcrq_hhrs_hash"},
		{"pcrq_qty_rsrc"},
		{"pcrq_hhwo_hash"},
		{"pcrq_seq_no"},
		{"pcrq_line_no"},
		{"pcrq_act_setup"},
		{"pcrq_act_run"},
		{"pcrq_act_clean"},
	};

	int	pcrq_no_fields = 8;

	struct
	{
		long	hhrs_hash;
		int		qty_rsrc;
		long	hhwo_hash;
		int		seq_no;
		int		line_no;
		long	act_setup;
		long	act_run;
		long	act_clean;
	} pcrq_rec;

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
		long	hhwc_hash;
		char	co_no [3];
		char	br_no [3];
		char	work_cntr [9];
		char	name [41];
	} pcwc_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_req_br_no"},
		{"pcwo_req_wh_no"},
		{"pcwo_rec_br_no"},
		{"pcwo_rec_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_reqd_date"},
		{"pcwo_rtg_seq"},
		{"pcwo_priority"},
		{"pcwo_create_date"},
		{"pcwo_hhbr_hash"},
		{"pcwo_bom_alt"},
		{"pcwo_rtg_alt"},
		{"pcwo_prod_qty"},
		{"pcwo_act_prod_qty"},
		{"pcwo_act_rej_qty"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
	};

	int	pcwo_no_fields = 21;

	struct
	{
		char	wo_co_no [3];
		char	wo_br_no [3];
		char	wo_wh_no [3];
		char	wo_req_br_no [3];
		char	wo_req_wh_no [3];
		char	wo_rec_br_no [3];
		char	wo_rec_wh_no [3];
		char	wo_order_no [8];
		long	wo_hhwo_hash;
		long	wo_reqd_date;
		int		wo_rtg_seq;
		int		wo_priority;
		long	wo_create_date;
		long	wo_hhbr_hash;
		int		wo_bom_alt;
		int		wo_rtg_alt;
		float	wo_prod_qty;
		float	wo_act_prod_qty;
		float	wo_act_rej_qty;
		char	wo_order_status [2];
		char	wo_batch_no [11];
	} pcwo_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list [] =
	{
		{"rgrs_hhrs_hash"},
		{"rgrs_code"},
		{"rgrs_desc"},
		{"rgrs_type"},
	};

	int	rgrs_no_fields = 4;

	struct
	{
		long	hhrs_hash;
		char	code [9];
		char	desc [41];
		char	type [2];
	} rgrs_rec;

	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	3

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
	}	esmr_rec;

	/*===========================================+
	 | Cost Centre/Warehouse Master File Record. |
	 +===========================================*/
#define	CCMR_NO_FIELDS	4

	struct dbview	ccmr_list [CCMR_NO_FIELDS] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_name"},
	};

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		char	name [41];
	}	ccmr_rec;

	char	*comm	= "comm",
			*data	= "data",
			*inei	= "inei",
			*inmr	= "inmr",
			*inum	= "inum",
			*pcln	= "pcln",
			*pcms	= "pcms",
			*pcrq	= "pcrq",
			*pcwc	= "pcwc",
			*pcwo	= "pcwo",
			*pcwo2	= "pcwo2",
			*rgrs	= "rgrs",
			*esmr	= "esmr",
			*ccmr	= "ccmr";

	int		pformat_open = FALSE;
	char	*ttoa (long int, char *);
	int		PC_GEN;

struct
{
	int		seq_no;
	long	hhwc_hash;
	double	rate;
	double	ovhd_var;
	double	ovhd_fix;
	long	time;
	int		qty_rsrc;
	char	type [2];
	int		instr_no;
} store [MAXLINES];
int		no_stored;

struct
{
	char	status;
	char	desc [13];
} wo_stat [] = {
	{'P', "Planned"},
	{'F', "Firm Planned"},
	{'I', "Issuing"},
	{'A', "Allocated"},
	{'R', "Released"},
	{'C', "Closing"},
	{'Z', "Closed"},
	{'D', "Deleted"},
	{0, ""},
};
int		no_stat = 9;

struct
{
	char	type;
	char	desc [9];
} res_type [] = {
	{'L', "Labour"},
	{'M', "Machine"},
	{'Q', "QC Check"},
	{'O', "Other"},
	{'S', "Special"},
	{0, ""},
};
int		no_res = 5;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	systemDate [11];
	long	lsystemDate;

	char	st_ord_no [8];
	char	st_batch_no [11];
	char	st_item [17];
	char	st_desc [41];
	char	end_ord_no [8];
	char	end_batch_no [11];
	char	end_item [17];
	char	end_desc [41];
	int		lpno;
	char	back [4];
	char	onight [4];

	char	item [17];
	char	item_desc [41];
	char	wo_status [19];
	char	res_desc [9];
	char	std_uom [5];
	char	alt_uom [5];
	char	mtl_uom [5];
	float	std_batch;
	float	min_batch;
	float	max_batch;
	double	tot_cost;
	long	tot_time;

	double	mat_tot_std;
	double	rtg_res_std;
	double	rtg_ovh_std;
	double	bch_tot_std;
	double	mat_tot_act;
	double	rtg_res_act;
	double	rtg_ovh_act;
	double	bch_tot_act;

	float	outer_size;
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_ord_no",	 4, 14, CHARTYPE,
		"UUUUUUU", "          ",
		" ", " ", "Start Order :", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.st_ord_no},
	{1, LIN, "st_batch_no",	 4, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Start Batch :", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.st_batch_no},
	{1, LIN, "st_item",	 4, 70, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_desc",	 4, 90, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "end_ord_no",	 5, 14, CHARTYPE,
		"UUUUUUU", "          ",
		" ", " ", "End Order   :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_ord_no},
	{1, LIN, "end_batch_no",	 5, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "End Batch   :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_batch_no},
	{1, LIN, "end_item",	 5, 70, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_desc",	 5, 90, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_desc},
	{1, LIN, "lpno",		 7, 14, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No : ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",		 8, 14, CHARTYPE,
		"U", "          ",
		" ", "N", " Background : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "onight",	 9, 14, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight  : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.onight},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<srch_pcwo2.h>
/*=====================
| function prototypes |
=====================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void run_prog (char *prog_name, char *prog_desc);
int heading (int scn);
int spec_valid (int field);
void process (void);
void head_output (void);
void print_wo (void);
void print_bom (void);
void proc_pcms (void);
void print_rtg (void);
void proc_lines (void);
void calc_vals (void);
float get_uom (long int hhum_hash);
void print_summary (void);
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv[])
{
	char 	*sptr;

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		PC_GEN = TRUE;

	SETUP_SCR (vars);

	if (argc != 2 && argc != 4)
	{
		print_at (0,0,mlPcMess712, argv [0]);
		print_at (1,0,mlPcMess713, argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 2)
	{
		/*-------------------------------
		| Setup required parameters	|
		-------------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();

		/*=======================================
		| Beginning of input control loop	|
		=======================================*/
		while (prog_exit == 0)
		{
			/*-----------------------
			| Reset control flags	|
			-----------------------*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;

			init_vars (1);
			/*-------------------------------
			| Enter screen 1 linear input	|
			-------------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;

			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			run_prog (argv [0], argv [1]);
		}
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	local_rec.lpno = atoi (argv [1]);
	sprintf (local_rec.st_ord_no, "%-7.7s", argv [2]);
	sprintf (local_rec.end_ord_no, "%-7.7s", argv [3]);

	process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);

	open_rec (inei, inei_list, inei_no_fields, "inei_id_no");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum, inum_list, inum_no_fields, "inum_hhum_hash");
	open_rec (pcln, pcln_list, pcln_no_fields, "pcln_id_no");
	open_rec (pcms, pcms_list, pcms_no_fields, "pcms_id_no");
	open_rec (pcrq, pcrq_list, pcrq_no_fields, "pcrq_id_no2");
	open_rec (pcwc, pcwc_list, pcwc_no_fields, "pcwc_hhwc_hash");
	open_rec (pcwo, pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, pcwo_no_fields, "pcwo_id_no3");
	open_rec (rgrs, rgrs_list, rgrs_no_fields, "rgrs_hhrs_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgrs);
	abc_fclose (esmr);
	abc_fclose (ccmr);

	abc_dbclose (data);
}

void
run_prog (
 char *prog_name, 
 char *prog_desc)
{
	char	lp_str [3];

	sprintf (lp_str, "%2d", local_rec.lpno);

	clear ();
	snorm ();
	CloseDB (); FinishProgram ();;
	rset_tty ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				lp_str,
				local_rec.st_ord_no,
				local_rec.end_ord_no,
				prog_desc, (char *)0);
		}
		else
			return;
	}

	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				lp_str,
				local_rec.st_ord_no,
				local_rec.end_ord_no, (char *)0);
		else
			return;
	}
	else
	{
		execlp (prog_name,
			prog_name,
			lp_str,
			local_rec.st_ord_no,
			local_rec.end_ord_no, (char *)0);
	}
}

/*=======================
| Display heading scrn	|
=======================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		swide ();
		clear ();
		rv_pr (ML(mlPcMess069),45,0,1);

		box (0, 3, 132, 6);
		move (0, 1);
		line (132);

		move (1, 6);
		line (131);

		move (0, 21);
		line (132);
		strcpy(err_str, ML(mlStdMess038));
		print_at (22, 0,err_str, comm_rec.tco_no, comm_rec.tco_short);
		strcpy(err_str, ML(mlStdMess039));
		print_at (22, 35,err_str, comm_rec.test_no, comm_rec.test_short);
		strcpy(err_str, ML(mlStdMess099));
		print_at (22, 65, err_str, comm_rec.tcc_no, comm_rec.tcc_short);

		scn_set (scn);

		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;

		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*===============================
| Validate entered field(s)	|
===============================*/
int
spec_valid (
 int field)
{
	if (LCHECK ("st_ord_no") || LCHECK ("st_batch_no"))
	{
		if (LCHECK ("st_batch_no"))
			if (F_NOKEY (label ("st_batch_no")))
				return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			/* all works order, expect deleted ones */
			if (LCHECK ("st_ord_no"))
				srch_order (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
			else
			{
				srch_batch (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
				abc_selfield (pcwo, "pcwo_id_no");
			}
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.st_ord_no, "       ");
			sprintf (local_rec.st_batch_no, "%-10.10s", " ");
			sprintf (local_rec.st_item, "%-16.16s", " ");
			sprintf (local_rec.st_desc, "%-35.35s", "First Works Order");
			DSP_FLD ("st_ord_no");
			DSP_FLD ("st_batch_no");
			DSP_FLD ("st_item");
			DSP_FLD ("st_desc");
			if (LCHECK ("st_ord_no"))
				FLD ("st_batch_no") = NO;
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("st_ord_no"))
			FLD ("st_batch_no") = NA;

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("st_ord_no"))
		{
			if (PC_GEN)
				sprintf (pcwo_rec.wo_order_no,
						"%-7.7s",
						zero_pad (local_rec.st_ord_no, 7));
			else
				sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.st_ord_no);
			cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		}
		else
		{
			sprintf (pcwo_rec.wo_batch_no, "%-10.10s", local_rec.st_batch_no);
			cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		}
		if (cc)
		{
/*
Note: Error Message : Works Order Not on File.
			if (LCHECK ("st_batch_no"))
				sprintf (err_str,
					" Works Order Not On File - Batch No [%s] ",
					pcwo_rec.wo_batch_no);
			else
				sprintf (err_str,
					" Works Order Not On File - Order No [%s] ",
					pcwo_rec.wo_order_no);
*/
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.st_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		DSP_FLD ("st_ord_no");

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_ord_no, local_rec.end_ord_no) > 0)
		{
			/*sprintf (err_str,
				"Start Works Order %s Greater Than End Works Order %s",
				local_rec.st_ord_no,
				local_rec.end_ord_no);*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_ord_no, "%-7.7s", " ");
			DSP_FLD ("st_ord_no");
			return (EXIT_FAILURE);
		}

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			/*print_mess ("\007 Works Order References An Unknown Item ");*/
			print_mess (ML(mlPcMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.st_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		sprintf (local_rec.st_batch_no, "%-10.10s", pcwo_rec.wo_batch_no);
		sprintf (local_rec.st_item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf(local_rec.st_desc, "%-35.35s", inmr_rec.mr_description);
		DSP_FLD ("st_ord_no");
		DSP_FLD ("st_batch_no");
		DSP_FLD ("st_item");
		DSP_FLD ("st_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_ord_no") || LCHECK ("end_batch_no"))
	{
		if (LCHECK ("end_batch_no"))
			if (F_NOKEY (label ("end_batch_no")))
				return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			/* all works order, expect deleted ones */
			if (LCHECK ("end_ord_no"))
				srch_order (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
			else
			{
				srch_batch (temp_str,
						"PFIARCZ",
						comm_rec.test_no,
						comm_rec.tcc_no);
				abc_selfield (pcwo, "pcwo_id_no");
			}
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.end_ord_no, "~~~~~~~");
			sprintf (local_rec.end_batch_no, "%10.10s", " ");
			sprintf (local_rec.end_item, "%-16.16s", " ");
			sprintf (local_rec.end_desc, "%-35.35s", "Last Works Order");
			DSP_FLD ("end_ord_no");
			DSP_FLD ("end_batch_no");
			DSP_FLD ("end_item");
			DSP_FLD ("end_desc");
			if (LCHECK ("end_ord_no"))
				FLD ("end_batch_no") = NO;
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("end_ord_no"))
			FLD ("end_batch_no") = NA;

		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("end_ord_no"))
		{
			if (PC_GEN)
				sprintf (pcwo_rec.wo_order_no,
						"%-7.7s",
						zero_pad (local_rec.end_ord_no, 7));
			else
			sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.end_ord_no);
			cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		}
		else
		{
			sprintf (pcwo_rec.wo_batch_no, "%-10.10s", local_rec.end_batch_no);
			cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		}
		if (cc)
		{
/*
Note: Error Message : Works Order Not on File.
			if (LCHECK ("end_batch_no"))
				sprintf (err_str,
					" Works Order Not On File - Batch No [%s] ",
					pcwo_rec.wo_batch_no);
			else
				sprintf (err_str,
					" Works Order Not On File - Order No [%s] ",
					pcwo_rec.wo_order_no);
*/
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.end_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		DSP_FLD ("end_ord_no");

		if (strcmp (local_rec.st_ord_no, local_rec.end_ord_no) > 0)
		{
/*
			sprintf (err_str,
				"Start Works Order %s Greater Than End Works Order %s",
				local_rec.st_ord_no,
				local_rec.end_ord_no);
*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_ord_no, "%-7.7s", " ");
			DSP_FLD ("end_ord_no");
			return (EXIT_FAILURE);
		}

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			/*print_mess ("\007 Works Order References An Unknown Item ");*/
			print_mess (ML(mlPcMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.end_ord_no, "%-7.7s", pcwo_rec.wo_order_no);
		sprintf (local_rec.end_batch_no, "%-10.10s", pcwo_rec.wo_batch_no);
		sprintf (local_rec.end_item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (local_rec.end_desc, "%-35.35s", inmr_rec.mr_description);
		DSP_FLD ("end_ord_no");
		DSP_FLD ("end_batch_no");
		DSP_FLD ("end_item");
		DSP_FLD ("end_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*print_mess ("\007 Invalid Printer Number ");*/
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.back, "Yes");
		else
			strcpy (local_rec.back, "No ");

		DSP_FLD ("back");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onight, "Yes");
		else
			strcpy (local_rec.onight, "No ");

		DSP_FLD ("onight");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Print W/O, BOM and Routing for all  |
| works orders in the specified range |
-------------------------------------*/
void
process (
 void)
{
	int	first_time;

	first_time = TRUE;
	dsp_screen ("Printing Works Order", comm_rec.tco_no, comm_rec.tco_name);

	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
	strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
	sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.st_ord_no);
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (pcwo_rec.wo_co_no,   comm_rec.tco_no) &&
		!strcmp (pcwo_rec.wo_br_no,   comm_rec.test_no) &&
		!strcmp (pcwo_rec.wo_wh_no,   comm_rec.tcc_no) &&
		strcmp (pcwo_rec.wo_order_no, local_rec.end_ord_no) <= 0
	)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		local_rec.outer_size = inmr_rec.mr_outer_size;
		sprintf (local_rec.item, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.mr_description);

		inei_rec.hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.test_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			inei_rec.std_batch = 0.00;
			inei_rec.min_batch = 0.00;
			inei_rec.max_batch = 0.00;
		}

		local_rec.std_batch = inei_rec.std_batch;
		local_rec.min_batch = inei_rec.min_batch;
		local_rec.max_batch = inei_rec.max_batch;

		if (first_time)
			head_output ();
		else
		{
			fprintf (fout, ".DS1\n");
			fprintf (fout,
				".EWORKS ORDER %s\n",
				clip (pcwo_rec.wo_order_no));
			fprintf (fout, ".PA\n");
		}

		dsp_process ("Works Order", pcwo_rec.wo_order_no);

		/*-------------------
		| Print Works Order |
		-------------------*/
		print_wo ();
		print_bom ();
		print_rtg ();
		print_summary ();

		first_time = FALSE;
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	if (pformat_open)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
}

/*---------------------
| Prepare Output Pipe |
---------------------*/
void
head_output (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	pformat_open = TRUE;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".7\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".ECOMPANY %s : %s\n",
		comm_rec.tco_no, clip (comm_rec.tco_name));

	fprintf (fout, ".EBRANCH %s : %s\n",
		comm_rec.test_no, clip (comm_rec.test_name));

	fprintf (fout, ".EWAREHOUSE %s : %s\n",
		comm_rec.tcc_no, clip (comm_rec.tcc_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, ".DS3\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWORKS ORDER DETAILS\n");

	fprintf (fout, ".R=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");
}

/*-------------------
| Print W/O Details |
-------------------*/
void
print_wo (
 void)
{
	int	i;

	for (i = 0; i < no_stat; i++)
	{
		if (pcwo_rec.wo_order_status [0] == wo_stat [i].status)
		{
			if (pcwo_rec.wo_order_status [0] == 'R')
			{
				sprintf (local_rec.wo_status,
					"%-12.12s(%3d)",
					wo_stat [i].desc,
					pcwo_rec.wo_rtg_seq);
			}
			else
			{
				sprintf (local_rec.wo_status,
					"%-18.18s",
					wo_stat [i].desc);
			}
			break;
		}
	}

	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");

	fprintf (fout, "| WORKS ORDER NUMBER:");
	fprintf (fout, " %-7.7s%-11.11s", pcwo_rec.wo_order_no, " ");
	fprintf (fout, "STATUS            :");
	fprintf (fout, " %-17.17s%-2.2s", local_rec.wo_status, " ");
	fprintf (fout, "PRIORITY          :");
	fprintf (fout, " %1d%-18.18s", pcwo_rec.wo_priority, " ");
	fprintf (fout, "DATE RAISED       :");
	fprintf (fout, " %-10.10s%-8.8s|\n", DateToString(pcwo_rec.wo_create_date), " ");

	fprintf (fout, "| BATCH NUMBER      :");
	fprintf (fout, " %-10.10s%-8.8s", pcwo_rec.wo_batch_no, " ");
	fprintf (fout, "QUANTITY REQUIRED :");
	fprintf (fout, " %14.6f%-5.5s", pcwo_rec.wo_prod_qty, " ");
	fprintf (fout, "QUANTITY PRODUCED :");
	fprintf (fout, " %14.6f%-5.5s", pcwo_rec.wo_act_prod_qty, " ");
	fprintf (fout, "REQ'D DATE        :");
	fprintf (fout, " %-10.10s%-8.8s|\n", DateToString(pcwo_rec.wo_reqd_date), " ");

	fprintf (fout, "|%-78.78s", " ");
	fprintf (fout, "QUANTITY REJECTED :");
	fprintf (fout, "%14.6f", pcwo_rec.wo_act_rej_qty);
	fprintf (fout, "%-43.43s|\n", " ");

	fprintf (fout, "| ITEM NUMBER       :");
	fprintf (fout, " %-16.16s%-41.41s", inmr_rec.mr_item_no, " ");
	fprintf (fout, "STRENGTH          :");
	fprintf (fout, " %-5.5s%-52.52s|\n", &inmr_rec.mr_description [35], " ");
	fprintf (fout, "| DESCRIPTION       :");
	fprintf (fout, " %-40.40s%-17.17s", inmr_rec.mr_description, " ");
	fprintf (fout, "BOM ALTERNATE     :");
	fprintf (fout, " %2d%-55.55s|\n", pcwo_rec.wo_bom_alt, " ");

	fprintf (fout, "|                   :");
	fprintf (fout, " %-40.40s%-17.17s", inmr_rec.mr_description2, " ");
	fprintf (fout, "ROUTING ALTERNATE :");
	fprintf (fout, " %2d%-55.55s|\n", pcwo_rec.wo_rtg_alt, " ");

	/*----------------
	| Lookup std uom |
	----------------*/
	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.mr_std_uom);
	if (cc)
		strcpy (inum_rec.uom, "UNKN");
	sprintf (local_rec.std_uom, "%-4.4s", inum_rec.uom);
	fprintf (fout, "| STD UOM           :");
	fprintf (fout, " %-4.4s%-53.53s", inum_rec.uom, " ");
	fprintf (fout, "PRICING CONVERSION:");
	fprintf (fout, " %6.1f%-51.51s|\n", inmr_rec.mr_outer_size, " ");

	/*----------------
	| Lookup alt uom |
	----------------*/
	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.mr_alt_uom);
	if (cc)
		strcpy (inum_rec.uom, "UNKN");
	sprintf (local_rec.alt_uom, "%-4.4s", inum_rec.uom);
	fprintf (fout, "| ALT UOM           :");
	fprintf (fout, " %-4.4s%-130.130s|\n", inum_rec.uom, " ");

	fprintf (fout, "|%-155.155s|\n", " ");

	/* read for requesting branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_req_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Requesting Branch Name Not Found");
	fprintf (fout, "| REQUESTING BRANCH :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_req_br_no,
			esmr_rec.est_name,
			" ");
	/* read for requesting warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Requesting Warehouse Name Not Found");
	fprintf (fout, "REQUESTING W/H    :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_req_wh_no,
			ccmr_rec.name,
			" ");

	/* read for manufacturing branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Manufacturing Branch Name Not Found");
	fprintf (fout, "| MANUFACTURE BRANCH:");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_br_no,
			esmr_rec.est_name,
			" ");
	/* read for manufacturing warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Manufacturing Warehouse Name Not Found");
	fprintf (fout, "MANUFACTURE W/H   :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_wh_no,
			ccmr_rec.name,
			" ");

	/* read for receiving branch details */
	strcpy (esmr_rec.co_no, comm_rec.tco_no);
	strcpy (esmr_rec.est_no, pcwo_rec.wo_rec_br_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		strcpy (esmr_rec.est_name, "Receiving Branch Name Not Found");
	fprintf (fout, "| RECEIVING BRANCH  :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s",
			pcwo_rec.wo_rec_br_no,
			esmr_rec.est_name,
			" ");
	/* read for receiving warehouse details */
	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.wo_rec_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.wo_rec_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		strcpy (ccmr_rec.name, "Receiving Warehouse Name Not Found");
	fprintf (fout, "RECEIVING W/H     :");
	fprintf (fout,
			" %-2.2s %-40.40s%14s|\n",
			pcwo_rec.wo_rec_wh_no,
			ccmr_rec.name,
			" ");

	fprintf (fout, "|%-155.155s|\n", " ");

	fprintf (fout, "| STANDARD BATCH : ");
	fprintf (fout, "%14.6f%-30.30s", local_rec.std_batch, " ");
	fprintf (fout,"MINIMUM BATCH : %14.6f%30.30s", local_rec.min_batch," ");
	fprintf (fout, "MAXIMUM BATCH : %14.6f   |\n", local_rec.max_batch);

	fprintf (fout, "|----------------------------------------------------");
	fprintf (fout, "-----------------------------------------------------");
	fprintf (fout, "--------------------------------------------------|\n");
}

/*-------------------------
| Print Bill Of Materials |
-------------------------*/
void
print_bom (
 void)
{
	fprintf (fout, ".DS8\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip(pcwo_rec.wo_order_no));
	fprintf (fout,".B1\n");
	fprintf (fout, ".EBILL OF MATERIALS\n");
	fprintf (fout,
		".CITEM: %-16.16s ALT NO: %5d   STRENGTH: %-5.5s DESCRIPTION: %-35.35s \n",
		local_rec.item,
		pcwo_rec.wo_bom_alt,
		&local_rec.item_desc [35],
		local_rec.item_desc);

	fprintf (fout,
		".CUOM Std: %-4.4s  Alt: %-4.4s       BATCH SIZES  Std: %14.6f  Min: %14.6f  Max: %14.6f\n",
		local_rec.std_uom,
		local_rec.alt_uom,
		inei_rec.std_batch,
		inei_rec.min_batch,
		inei_rec.max_batch);

	fprintf (fout, "====================");
	fprintf (fout, "======================================");
	fprintf (fout, "========");
	fprintf (fout, "=======");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "==============");
	fprintf (fout, "==============");
	fprintf (fout, "===========");
	fprintf (fout, "===========\n");

	fprintf (fout, "|     MATERIAL     |");
	fprintf (fout, "        D E S C R I P T I O N        |");
	fprintf (fout, "STRNGTH|");
	fprintf (fout, " UOM  |");
	fprintf (fout, "  STANDARD QTY. |");
	fprintf (fout, "    ACTUAL QTY. |");
	fprintf (fout, "STD UNIT COST|");
	fprintf (fout, "ACT UNIT COST|");
	fprintf (fout, " EXT. STD |");
	fprintf (fout, " EXT. ACT |\n");

	fprintf (fout, "|------------------+");
	fprintf (fout, "-------------------------------------+");
	fprintf (fout, "-------+");
	fprintf (fout, "------+");
	fprintf (fout, "----------------+");
	fprintf (fout, "----------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "----------+");
	fprintf (fout, "----------|\n");

	fprintf (fout, "|     MATERIAL     |");
	fprintf (fout, "        D E S C R I P T I O N        |");
	fprintf (fout, "STRNGTH|");
	fprintf (fout, " UOM  |");
	fprintf (fout, "  STANDARD QTY. |");
	fprintf (fout, "    ACTUAL QTY. |");
	fprintf (fout, "STD UNIT COST|");
	fprintf (fout, "ACT UNIT COST|");
	fprintf (fout, " EXT. STD |");
	fprintf (fout, " EXT. ACT |\n");

	fprintf (fout, "|------------------+");
	fprintf (fout, "-------------------------------------+");
	fprintf (fout, "-------+");
	fprintf (fout, "------+");
	fprintf (fout, "----------------+");
	fprintf (fout, "----------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "----------+");
	fprintf (fout, "----------|\n");

	proc_pcms ();

	return;
}

/*----------------------
| Process pcms records |
----------------------*/
void
proc_pcms (
 void)
{
	double	tot_std = 0.00,
			tot_act = 0.00,
			tmp_val; 
	float	get_uom (long int hhum_hash),
			cnv_fct;

	pcms_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcms_rec.uniq_id   = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcms_rec.mabr_hash);
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		inei_rec.hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.test_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inei_rec.hzrd_class, "UNKN");
			inei_rec.std_cost = (double) 0.00;
		}

		cnv_fct = get_uom (pcms_rec.uom);
		cc = find_hash (inum, &inum_rec, EQUAL, "r", pcms_rec.uom);
		strcpy (local_rec.mtl_uom, (cc) ? "    " : inum_rec.uom);

		fprintf (fout, ".LRP3\n");

		pcms_rec.matl_wst_pc += 100.00;
		pcms_rec.matl_wst_pc /= 100.00;

		fprintf (fout, "| %-16.16s ",	inmr_rec.mr_item_no);
		fprintf (fout, "| %-35.35s ",	inmr_rec.mr_description);
		fprintf (fout, "| %-5.5s ",	&inmr_rec.mr_description [35]);
		fprintf (fout, "| %-4.4s ",	local_rec.mtl_uom);
		fprintf (fout, "| %14.6f ",	pcms_rec.matl_qty * pcms_rec.matl_wst_pc);
		fprintf (fout, "| %14.6f ",	pcms_rec.qty_issued);
		fprintf (fout, "|  %9.2f  ",	inei_rec.std_cost);

		if (pcms_rec.qty_issued == 0.00)
			fprintf (fout, "|  %9.2f  ", 0.00);
		else
		{
			fprintf (fout, "|  %9.2f  ", inmr_rec.mr_outer_size * 
						  DOLLARS (pcms_rec.amt_issued / pcms_rec.qty_issued));
		}

		tmp_val = out_cost (inei_rec.std_cost / cnv_fct,
				inmr_rec.mr_outer_size);
		tmp_val *= ( pcms_rec.matl_qty * pcms_rec.matl_wst_pc );
		tot_std += tmp_val;

		fprintf (fout, "|%10.2f",	tmp_val);
		fprintf (fout, "|%10.2f",	DOLLARS (pcms_rec.amt_issued));
		fprintf (fout, "|%-1.1s\n",
			(pcms_rec.act_qty_in [0] != 'Y') ? "*" : " ");
		tot_act += DOLLARS (pcms_rec.amt_issued);

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	local_rec.mat_tot_std = CENTS (tot_std);
	local_rec.mat_tot_act = CENTS (tot_act);

	fprintf (fout, "|----------------------------------------------------");
	fprintf (fout, "-----------------------------------------------------");
	fprintf (fout, "--------------------------------------------------|\n");

	fflush (fout);
	return;
}

/*---------------
| Print Routing |
---------------*/
void
print_rtg (
 void)
{
	calc_vals ();

	fprintf (fout, ".DS8\n");
	fprintf (fout, ".EWORKS ORDER %s\n", clip (pcwo_rec.wo_order_no));
	fprintf (fout,".B1\n");
	fprintf (fout, ".EROUTING DETAILS\n");
	fprintf (fout, "     Product Code: %-16.16s  %-35.35s  ",
		local_rec.item, local_rec.item_desc);

	fprintf (fout, "Alternate No : %d     ", pcwo_rec.wo_rtg_alt);
	fprintf (fout, "Elapsed Time: %s          ",
		ttoa (local_rec.tot_time, "NNNNNN:NN"));

	fprintf (fout, "Running Costs :  %9.2f\n", DOLLARS (local_rec.tot_cost));

	fprintf (fout, "========================================");
	fprintf (fout, "========================================");
	fprintf (fout, "========================================");
	fprintf (fout, "======================================\n");

	fprintf (fout, "|SEQ");
	fprintf (fout, "|  WORK  ");
	fprintf (fout, "|RESOURCE");
	fprintf (fout, "|        RESOURCE        ");
	fprintf (fout, "| RESOURCE ");
	fprintf (fout, "|NO ");
	fprintf (fout, "|   RATE  ");
	fprintf (fout, "|  FIXED  ");
	fprintf (fout, "|    S T A N D A R D    ");
	fprintf (fout, "|      A C T U A L      ");
	fprintf (fout, "|   STANDARD ");
	fprintf (fout, "|    ACTUAL  |\n");

	fprintf (fout, "|NO ");
	fprintf (fout, "| CENTRE ");
	fprintf (fout, "|        ");
	fprintf (fout, "|      DESCRIPTION       ");
	fprintf (fout, "|   TYPE   ");
	fprintf (fout, "|OPS");
	fprintf (fout, "| PER HOUR");
	fprintf (fout, "| OVERHEAD");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "|     COST   ");
	fprintf (fout, "|     COST   |\n");

	fprintf (fout, "|---");
	fprintf (fout, "+--------");
	fprintf (fout, "+--------");
	fprintf (fout, "+------------------------");
	fprintf (fout, "+----------");
	fprintf (fout, "+---");
	fprintf (fout, "+---------");
	fprintf (fout, "+---------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+------------");
	fprintf (fout, "+------------|\n");

	fprintf (fout, "|SEQ");
	fprintf (fout, "|  WORK  ");
	fprintf (fout, "|RESOURCE");
	fprintf (fout, "|        RESOURCE        ");
	fprintf (fout, "| RESOURCE ");
	fprintf (fout, "|NO ");
	fprintf (fout, "|   RATE  ");
	fprintf (fout, "|  FIXED  ");
	fprintf (fout, "|    S T A N D A R D    ");
	fprintf (fout, "|      A C T U A L      ");
	fprintf (fout, "|   STANDARD ");
	fprintf (fout, "|    ACTUAL  |\n");

	fprintf (fout, "|NO ");
	fprintf (fout, "| CENTRE ");
	fprintf (fout, "|        ");
	fprintf (fout, "|      DESCRIPTION       ");
	fprintf (fout, "|   TYPE   ");
	fprintf (fout, "|OPS");
	fprintf (fout, "| PER HOUR");
	fprintf (fout, "| OVERHEAD");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "|     COST   ");
	fprintf (fout, "|     COST   |\n");

	fprintf (fout, "|---");
	fprintf (fout, "+--------");
	fprintf (fout, "+--------");
	fprintf (fout, "+------------------------");
	fprintf (fout, "+----------");
	fprintf (fout, "+---");
	fprintf (fout, "+---------");
	fprintf (fout, "+---------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+------------");
	fprintf (fout, "+------------|\n");

	proc_lines ();

	return;
}

/*--------------------
| Process pcln lines |
--------------------*/
void
proc_lines (
 void)
{
	int		i,
			use_star;
	long	std_tmp_time,
			act_tmp_time;
	double	std_line_cost,
			std_ovhd_cost,
			act_line_cost,
			act_ovhd_cost;

	local_rec.rtg_res_std = 0.00;
	local_rec.rtg_res_act = 0.00;
	local_rec.rtg_ovh_std = 0.00;
	local_rec.rtg_ovh_act = 0.00;

	pcln_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.seq_no = 0;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		use_star = FALSE;
		pcrq_rec.hhwo_hash = pcln_rec.hhwo_hash;
		pcrq_rec.seq_no = pcln_rec.seq_no;
		pcrq_rec.line_no = pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
		if ((cc || pcln_rec.seq_no >= pcwo_rec.wo_rtg_seq) &&
			pcwo_rec.wo_order_status [0] != 'Z' )
			use_star = TRUE;
		if (cc)
		{
			pcrq_rec.act_setup	= 0L;
			pcrq_rec.act_run	= 0L;
			pcrq_rec.act_clean	= 0L;
		}

		cc = find_hash(pcwc,&pcwc_rec,EQUAL, "r",pcln_rec.hhwc_hash);
		if (cc)
			strcpy (pcwc_rec.work_cntr, "NOT FND.");

		cc = find_hash(rgrs,&rgrs_rec,EQUAL, "r",pcln_rec.hhrs_hash);
		if (cc)
		{
			strcpy (rgrs_rec.code, "NOT FND.");
			strcpy (rgrs_rec.desc, "NOT FND.");
			strcpy (rgrs_rec.type, " ");
		}

		strcpy (local_rec.res_desc, "NOT FND ");
		for (i = 0; i < no_res; i++)
		{
			if (rgrs_rec.type [0] == res_type [i].type)
			{
				sprintf (local_rec.res_desc,
					"%-8.8s",
					res_type [i].desc);
				break;
			}
		}

		std_tmp_time =  pcln_rec.setup;
		std_tmp_time += pcln_rec.run;
		std_tmp_time += pcln_rec.clean;

		if (std_tmp_time != 0)
		{
			std_line_cost = (double) std_tmp_time * pcln_rec.rate;
			std_line_cost /= 60.00;
			std_line_cost *= (double) pcln_rec.qty_rsrc;

			std_ovhd_cost = (double) std_tmp_time * pcln_rec.ovhd_var;
			std_ovhd_cost /= 60.00;
			std_ovhd_cost += pcln_rec.ovhd_fix;
			std_ovhd_cost *= (double) pcln_rec.qty_rsrc;
		}
		else
		{
			std_line_cost = 0.00;
			std_ovhd_cost = 0.00;
		}

		act_tmp_time =  pcrq_rec.act_setup;
		act_tmp_time += pcrq_rec.act_run;
		act_tmp_time += pcrq_rec.act_clean;

		act_line_cost = (double) act_tmp_time * pcln_rec.rate;
		act_line_cost /= 60.00;
		act_line_cost *= (double) pcln_rec.qty_rsrc;

		act_ovhd_cost = (double) act_tmp_time * pcln_rec.ovhd_var;
		act_ovhd_cost /= 60.00;
		act_ovhd_cost += pcln_rec.ovhd_fix;
		act_ovhd_cost *= (double) pcln_rec.qty_rsrc;

		fprintf (fout, ".LRP2\n");

		fprintf (fout, "|%3d",		pcln_rec.seq_no);
		fprintf (fout, "|%-8.8s",	pcwc_rec.work_cntr);
		fprintf (fout, "|%-8.8s",	rgrs_rec.code);
		fprintf (fout, "|%-24.24s",	rgrs_rec.desc);
		fprintf (fout, "| %-8.8s ",	local_rec.res_desc);
		fprintf (fout, "|%3d",		pcln_rec.qty_rsrc);
		fprintf (fout, "|%9.2f",	DOLLARS (pcln_rec.rate));
		fprintf (fout, "|         ");
		fprintf (fout, "|%s",		ttoa (pcln_rec.setup, "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (pcln_rec.run,   "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (pcln_rec.clean, "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (pcrq_rec.act_setup, "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (pcrq_rec.act_run,   "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (pcrq_rec.act_clean, "NNNN:NN"));
		fprintf (fout, "| %10.2f ",	DOLLARS (std_line_cost));
		fprintf (fout, "| %10.2f ",	DOLLARS (act_line_cost));
		fprintf (fout, "|%s\n",		(use_star) ? "*" : " ");
		local_rec.rtg_res_std += std_line_cost;
		local_rec.rtg_res_act += act_line_cost;

		fprintf (fout, "|   |        |        |Overhead Content        |          |   ");
		fprintf (fout, "|%9.2f",	DOLLARS (pcln_rec.ovhd_var));
		fprintf (fout, "|%9.2f",	DOLLARS (pcln_rec.ovhd_fix));
		fprintf (fout, "|       |       |       |       |       |       ");
		fprintf (fout, "| %10.2f ",	DOLLARS (std_ovhd_cost));
		fprintf (fout, "| %10.2f ",	DOLLARS (act_ovhd_cost));
		fprintf (fout, "|%s\n",		(use_star) ? "*" : " ");
		local_rec.rtg_ovh_std += std_ovhd_cost;
		local_rec.rtg_ovh_act += act_ovhd_cost;

		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}
}

/*----------------------------------------
| Calculate running cost/time of routing |
----------------------------------------*/
void
calc_vals (
 void)
{
	double	cur_cost;
	long	cur_time,
			tmp_time = 0L;
	int		i;

	no_stored = 0;
	pcln_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.seq_no = 0;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		store [no_stored].seq_no		= pcln_rec.seq_no;
		store [no_stored].hhwc_hash	= pcln_rec.hhwc_hash;
		store [no_stored].rate		= pcln_rec.rate;
		store [no_stored].ovhd_var	= pcln_rec.ovhd_var;
		store [no_stored].ovhd_fix	= pcln_rec.ovhd_fix;
		store [no_stored].time		= pcln_rec.setup +
						  pcln_rec.run +
						  pcln_rec.clean;
		store [no_stored].qty_rsrc	= pcln_rec.qty_rsrc;
		cc = find_hash(rgrs,&rgrs_rec,EQUAL, "r",pcln_rec.hhrs_hash);
		if (cc)
			strcpy (store [no_stored].type, "*");
		else
			strcpy (store [no_stored].type, rgrs_rec.type);
		store [no_stored++].instr_no = pcln_rec.instr_no;
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	local_rec.tot_time = 0L;
	local_rec.tot_cost = 0.00;

	for (tmp_time = 0L, i = 0; i < no_stored; i++)
	{
		cur_time = store [i].time;
		if (cur_time > tmp_time)
			tmp_time = cur_time;

		if (i + 1 == no_stored || store [i].seq_no != store [i+1].seq_no)
		{
			local_rec.tot_time += tmp_time;
			tmp_time = 0L;
		}

		cur_cost = (store [i].rate + store [i].ovhd_var) * (double) cur_time;
		cur_cost /= 60.00;
		cur_cost += store [i].ovhd_fix;
		cur_cost *= store [i].qty_rsrc;

		local_rec.tot_cost += cur_cost;
	}
}

float	
get_uom (
 long int hhum_hash)
{
	char	std_group [21],
			alt_group [21];
	number	std_cnv_fct,
			alt_cnv_fct,
			cnv_fct,
			result,
			uom_cfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_alt_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_std_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", hhum_hash);
	if (cc)
		file_err (cc, inum, "DBFIND");
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr_rec.mr_uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

void
print_summary (
 void)
{
	float	totalQty;

	/*---------------------------------------
	| If the W/Order is not yet Closed,		|
	| then report based on what we expected	|
	| to produce. Otherwise, report on the	|
	| quantity we ACTUALLY produced.		|
	---------------------------------------*/
	if (pcwo_rec.wo_order_status [0] != 'Z')
		totalQty = pcwo_rec.wo_prod_qty;
	else
		totalQty = pcwo_rec.wo_act_prod_qty + pcwo_rec.wo_act_rej_qty;

	local_rec.bch_tot_std = local_rec.mat_tot_std +
				local_rec.rtg_res_std +
				local_rec.rtg_ovh_std;
	local_rec.bch_tot_act = local_rec.mat_tot_act +
				local_rec.rtg_res_act +
				local_rec.rtg_ovh_act;
	fprintf (fout, ".LRP9\n");

	fprintf (fout, "|-----------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
	fprintf (fout, "|%-58.58s                 |  COST PER PRICE CONV  |         COST PER UNIT         |    COST PER BATCH     |\n", " ");
	fprintf (fout, "|%-58.58s                 |   STANDARD     ACTUAL |     STANDARD         ACTUAL   |   STANDARD     ACTUAL |\n", " ");
	fprintf (fout, "|%-58.58sMaterial         | %10.2f %10.2f | %14.6f %14.6f | %10.2f %10.2f |\n",
		" ",
		DOLLARS (local_rec.mat_tot_std / pcwo_rec.wo_prod_qty) *
				local_rec.outer_size,
		DOLLARS (local_rec.mat_tot_act / totalQty) *
				local_rec.outer_size,
		DOLLARS (local_rec.mat_tot_std / pcwo_rec.wo_prod_qty),
		DOLLARS (local_rec.mat_tot_act / totalQty),
		DOLLARS (local_rec.mat_tot_std),
		DOLLARS (local_rec.mat_tot_act));
	fprintf (fout, "|%-58.58sResource         | %10.2f %10.2f | %14.6f %14.6f | %10.2f %10.2f |\n",
		" ",
		DOLLARS (local_rec.rtg_res_std / pcwo_rec.wo_prod_qty) *
				local_rec.outer_size,
		DOLLARS (local_rec.rtg_res_act / totalQty) *
				local_rec.outer_size,
		DOLLARS (local_rec.rtg_res_std / pcwo_rec.wo_prod_qty),
		DOLLARS (local_rec.rtg_res_act / totalQty),
		DOLLARS (local_rec.rtg_res_std),
		DOLLARS (local_rec.rtg_res_act));
	fprintf (fout, "|%-58.58sOverhead         | %10.2f %10.2f | %14.6f %14.6f | %10.2f %10.2f |\n",
		" ",
		DOLLARS (local_rec.rtg_ovh_std / pcwo_rec.wo_prod_qty) *
				local_rec.outer_size,
		DOLLARS (local_rec.rtg_ovh_act / totalQty) *
				local_rec.outer_size,
		DOLLARS (local_rec.rtg_ovh_std / pcwo_rec.wo_prod_qty),
		DOLLARS (local_rec.rtg_ovh_act / totalQty),
		DOLLARS (local_rec.rtg_ovh_std),
		DOLLARS (local_rec.rtg_ovh_act));
	fprintf (fout, "|%-58.58s                 | ---------- ---------- | -------------- -------------- | ---------- ---------- |\n", " ");
	fprintf (fout, "|%-58.58sFinished Goods   | %10.2f %10.2f | %14.6f %14.6f | %10.2f %10.2f |\n",
		" ",
		DOLLARS (local_rec.bch_tot_std / pcwo_rec.wo_prod_qty) *
			local_rec.outer_size,
		DOLLARS (local_rec.bch_tot_act / totalQty) *
			local_rec.outer_size,
		DOLLARS (local_rec.bch_tot_std / pcwo_rec.wo_prod_qty),
		DOLLARS (local_rec.bch_tot_act / totalQty),
		DOLLARS (local_rec.bch_tot_std),
		DOLLARS (local_rec.bch_tot_act));
	
	fprintf (fout, "|%155.155s|\n", " ");
	fprintf (fout, "|NOTE : If the Works Order is NOT Closed then the Quantity Required is used to Calculate Costs, otherwise Quantity Produced Plus Quantity Rejected is used. |\n");
}



/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_wipdisp.c  )                                  |
|  Program Desc  : ( Print Works In Progress Report.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pcwo, ccmr, inmr, pcrq, inei, pcms, pcln    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 05/10/93         |
|---------------------------------------------------------------------|
|  Date Modified : (12/10/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (15/10/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (24/11/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (08/12/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (10/12/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (24/01/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (28/01/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (18/03/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (18/03/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (31/03/94)        Modified By   : Campbell Mander. |
|  Date Modified : (05/10/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (08/09/97)        Modified By   : Marnie Organo.   |
|  Date Modified : (15/10/97)        Modified By   : Marnie Organo.   |
|                                                                     |
|  Comments      : (12/10/93) DPL 9670 - Not adding std and act hours |
|                : and act. materials correctly.                      |
|  (15/10/93)    : DPL 9670 - Changed the required, received, and     |
|                : rejected qty's to %14.6f format (9999999.999999)   |
|  (24/11/93)    : DPL 9670 - Display all W/O's but not with an order |
|                : status equal to F, Z or D.                         |
|  (08/12/93)    : DPL 10197 - Bug fix - printing the first BOM item  |
|                : instead of the manufactured item.                  |
|  (10/12/93)    : DPL 10224 - Bug fix - print report to the wrong    |
|                : printer.                                           |
|  (24/01/94)    : DPL 9673 - can enter w/o no or batch no, to bring  |
|                : up order details.                                  |
|  (28/01/94)    : DPL 9673 - not copying "~~~~~~~" to the end hash.  |
|  (18/03/94)    : DPL 10626 - print date beside the prog. name.      |
|  (18/03/94)    : DPL 10482 - implementing system generated works    |
|                : order numbering.                                   |
|  (31/03/94)    : DPL 10482. Fix core dump.                          |
|  (05/10/94)    : PSL 11299 - mfg cutover - update srch_pcwo2.h      |
|  (08/09/97)    : Modified for Multilingual Conversion.              |
|  (15/10/97)    : Added ML().                                        |
|                :                                                    |
| $Log: pc_wiprpt.c,v $
| Revision 5.3  2002/07/17 09:57:30  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:14  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:12  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:21  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.16  2000/01/25 11:01:18  ana
| (25/01/2000) SC15817/2396 Corrected reading of pcrq.
|
| Revision 1.16  2000/01/25 10:51:20  ana
| (25/01/2000) SC2396 Corrected reading of pcrq.
|
| Revision 1.15  1999/12/06 03:12:10  cam
| Changes for GVision compatibility.  Move description fields 3 characters to the right.  Remove trailing ^M's.
|
| Revision 1.14  1999/11/12 10:37:49  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.13  1999/09/29 10:11:42  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/17 08:26:27  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.11  1999/09/13 07:03:21  marlene
| *** empty log message ***
|
| Revision 1.10  1999/09/09 06:12:38  marlene
| *** empty log message ***
|
| Revision 1.9  1999/06/17 07:40:51  scott
| Update for database name and Log file additions required for cvs.
|
|$Log: pc_wiprpt.c,v $
|Revision 5.3  2002/07/17 09:57:30  scott
|Updated to change argument to get_lpno from (1) to (0)
|
|Revision 5.2  2001/08/09 09:14:55  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:35:14  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:10:36  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:31:43  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.0  2000/10/10 12:17:12  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 09:03:21  gerry
|forced Revision no start 2.0 Rel-15072000
|
|Revision 1.16  2000/01/25 11:01:18  ana
|(25/01/2000) SC15817/2396 Corrected reading of pcrq.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_wiprpt.c,v $";
char    *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_wiprpt/pc_wiprpt.c,v 5.3 2002/07/17 09:57:30 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#define SLEEP_TIME	2

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int	comm_no_fields = 7;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		char	tcc_no [3];
		char	tcc_name [41];
	} comm_rec;

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
		{"pcwo_hhcc_hash"},
		{"pcwo_prod_qty"},
		{"pcwo_act_prod_qty"},
		{"pcwo_act_rej_qty"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
	};

	int	pcwo_no_fields = 12;

	struct tag_pcwoRecord
	{
		char	wo_co_no [3];
		char	wo_br_no [3];
		char	wo_wh_no [3];
		char	wo_order_no [8];
		long	wo_hhwo_hash;
		long	wo_hhbr_hash;
		long	wo_hhcc_hash;
		float	wo_prod_qty;
		float	wo_act_prod_qty;
		float	wo_act_rej_qty;
		char	wo_order_status [2];
		char	wo_batch_no [11];
	} pcwo_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_name"},
	};

	int	ccmr_no_fields = 5;

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
		char	name [41];
	} ccmr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_std_uom"},
		{"inmr_outer_size"},
	};

	int	inmr_no_fields = 5;

	struct tag_inmrRecord
	{
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		char	mr_description [41];
		long	mr_std_uom;
		float	mr_outer_size;
	} inmr_rec, inmr2_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list [] =
	{
		{"inum_hhum_hash"},
		{"inum_uom"},
	};

	int	inum_no_fields = 2;

	struct tag_inumRecord
	{
		long	hhum_hash;
		char	uom [5];
	} inum_rec;

	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list [] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_std_cost"},
	};

	int	inei_no_fields = 3;

	struct tag_ineiRecord
	{
		long	hhbr_hash;
		char	est_no [3];
		double	std_cost;
	} inei_rec;

	/*=================================
	| BoM Material Specification File |
	=================================*/
	struct dbview pcms_list [] =
	{
		{"pcms_mabr_hash"},
		{"pcms_matl_qty"},
		{"pcms_matl_wst_pc"},
		{"pcms_uniq_id"},
		{"pcms_hhwo_hash"},
		{"pcms_amt_issued"},
	};

	int	pcms_no_fields = 6;

	struct tag_pcmsRecord
	{
		long	mabr_hash;
		float	matl_qty;
		float	matl_wst_pc;
		int		uniq_id;
		long	hhwo_hash;
		double	amt_issued;	/* money */
	} pcms_rec;

	/*==========================
	| Routing Line detail File |
	==========================*/
	struct dbview pcln_list [] =
	{
		{"pcln_seq_no"},
		{"pcln_rate"},
		{"pcln_ovhd_var"},
		{"pcln_ovhd_fix"},
		{"pcln_setup"},
		{"pcln_run"},
		{"pcln_clean"},
		{"pcln_qty_rsrc"},
		{"pcln_line_no"},
		{"pcln_hhwo_hash"}
	};

	int	pcln_no_fields = 10;

	struct tag_pclnRecord
	{
		int		seq_no;
		double	rate; 		/* money */
		double	ovhd_var; 	/* money */
		double	ovhd_fix; 	/* money */
		long	setup;
		long	run;
		long	clean;
		int		qty_rsrc;
		int		line_no;
		long	hhwo_hash;
	} pcln_rec;

	/*===================================
	| Production Control Resource Queue |
	===================================*/
	struct dbview pcrq_list [] =
	{
		{"pcrq_hhwo_hash"},
		{"pcrq_seq_no"},
		{"pcrq_line_no"},
		{"pcrq_act_setup"},
		{"pcrq_act_run"},
		{"pcrq_act_clean"},
	};

	int	pcrq_no_fields = 6;

	struct tag_pcrqRecord
	{
		long	hhwo_hash;
		int		seq_no;
		int		line_no;
		long	act_setup;
		long	act_run;
		long	act_clean;
	} pcrq_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list [] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	int	esmr_no_fields = 3;

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
	} esmr_rec;

	char	*data = "data",
			*comm = "comm",
			*pcwo = "pcwo",
			*pcwo2 = "pcwo2",
			*ccmr = "ccmr",
			*inmr = "inmr",
			*inmr2 = "inmr2",
			*inum = "inum",
			*inei = "inei",
			*pcms = "pcms",
			*pcln = "pcln",
			*pcrq = "pcrq",
			*esmr = "esmr";
 
	FILE	*fout;

	int 	pc_wo_batch = FALSE;
	int		PC_GEN;

struct {
	double	std_material; /* standard material total for batch or w/o */
	double	swh_material; /* standard material total for warehouse */
	double	sbr_material; /* standard material total for branch */
	double	sco_material; /* standard material total for company */
	double	act_material; /* actual material total for batch or w/o */
	double	awh_material; /* actual material total for warehouse */
	double	abr_material; /* actual material total for branch */
	double	aco_material; /* actual material total for company */

	double	std_resource;
	double	swh_resource;
	double	sbr_resource;
	double	sco_resource;
	double	act_resource;
	double	awh_resource;
	double	abr_resource;
	double	aco_resource;

	double	std_overhead;
	double	swh_overhead;
	double	sbr_overhead;
	double	sco_overhead;
	double	act_overhead;
	double	awh_overhead;
	double	abr_overhead;
	double	aco_overhead;

	int		std_hours;
	int		swh_hours;
	int		sbr_hours;
	int		sco_hours;
	int		act_hours;
	int		awh_hours;
	int		abr_hours;
	int		aco_hours;
} total_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	st_wono [8];
	char	st_bno [11];
	char	st_item [17];
	char	st_desc	[41];
	char	st_hhwo [10];
	char	ed_wono [8];
	char	ed_bno [11];
	char	ed_item [17];
	char	ed_desc	[41];
	char	ed_hhwo [10];
	char	extra [2];
	char	disp_extra [5];
	char	cobrwh [2];
	char	disp_cobrwh [10];
	int		lp_no;
	char	background [2];
	char	back_desc [5];
	char	overnight [2];
	char	over_desc [5];
	char 	dummy [11];

	char	prev_br [3];
	char	prev_wh [3];
} local_rec;

static struct	var vars[] =
{
	{1, LIN, "startorder", 4, 30, CHARTYPE, 
		"UUUUUUU", "           ", 
		" ", " ", " Start Order No              : ", "Enter Start Works Order Number", 
		NO, NO, JUSTLEFT, "", "", local_rec.st_wono}, 
	{1, LIN, "startbatch", 4, 75, CHARTYPE, 
		"UUUUUUUUUU", "           ", 
		" ", " ", " Start Batch No              : ", "Enter Start Batch Number", 
		NO, NO, JUSTLEFT, "", "", local_rec.st_bno}, 
	{1, LIN, "startitem", 5, 30, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "           ", 
		" ", "", " Item Number                 : ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_item}, 
	{1, LIN, "startdesc", 5, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_desc}, 
	{1, LIN, "endorder", 7, 30, CHARTYPE, 
		"UUUUUUU", "           ",
		" ",  "~~~~~~~", " End Order No                : ", "Enter End Works Order Number", 
		NO, NO, JUSTLEFT, "", "", local_rec.ed_wono}, 
	{1, LIN, "endbatch", 7, 75, CHARTYPE, 
		"UUUUUUUUUU", "           ", 
		" ", " ", " End Batch No                : ", "Enter End Batch Number",
		NO, NO, JUSTLEFT, "", "", local_rec.ed_bno}, 
	{1, LIN, "enditem", 8, 30, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "           ",
		" ",  "", " Item Number                 : ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.ed_item}, 
	{1, LIN, "enddesc", 8, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ",  "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.ed_desc}, 
	{1, LIN, "extra", 10, 30, CHARTYPE,
		"U", "          ", 
		" ", "Y", " Print Quantities            : ",  "Enter : Y(es to print required, received, and rejected qtuantities - Default Y(es",
		YES, NO, JUSTLEFT, "YN", "", local_rec.extra}, 
	{1, LIN, "disp_extra", 10, 33, CHARTYPE,
		"AAA", "          ", 
		" ", " ", "",  " ",
		NA, NO, JUSTLEFT, "", "", local_rec.disp_extra}, 
	{1, LIN, "cobrwh", 12, 30, CHARTYPE,
		"U", "          ", 
		" ", "C", " C(ompany/B(ranch/W(arehouse : ",  "Enter : C(ompnay, B(ranch, W(arehouse - Default C(ompany", 
		YES, NO, JUSTLEFT, "BCW", "", local_rec.cobrwh}, 
	{1, LIN, "disp_cobrwh", 12, 33, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", " ", "",  " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.disp_cobrwh},
	{1, LIN,  "lp_no", 14, 30, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No                  : ", " ", 
		YES, NO, JUSTRIGHT, "",  "", (char *)&local_rec.lp_no}, 
	{1, LIN, "bk_grd", 15, 30, CHARTYPE, 
		"U", "          ",
		" ", "N", " Background                  : ", "Enter : Y(es or N(o",
		YES, NO, JUSTLEFT, "", "", local_rec.background},
	{1, LIN, "bk_grd_desc", 15, 33, CHARTYPE,
		"AAA", "          ",
		"", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "o_night", 16, 30, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight                   : ", "Enter : Y(es or N(o", 
		YES, NO, JUSTLEFT, "", "", local_rec.overnight},
	{1, LIN, "o_night_desc", 16, 33, CHARTYPE,
		"AAA", "          ",
		"", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.over_desc},
		
	{0, LIN, "", 0,  0, INTTYPE, 
		"A", "          "
		, " ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include	<srch_pcwo2.h>

/*=====================
| function prototypes|
=======================*/
void RunProg (char *progname);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int ProcPcwo (void);
int ValidCcmr (void);
int ValidPcwo (void);
void PrintWarehouseDetails (int flag);
void ReadInmrInum (void);
void CalcTotal (void);
int ProcLog (void);
void PrintWarehouseTotals (void);
void PrintBranchTotals (void);
void PrintCompanyTotals (void);
int heading (int scn);
void InitOutput (void);
void PrintHeading (void);			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv[])
{
	char	*sptr,
			*chk_env (char *);

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		PC_GEN = TRUE;

	SETUP_SCR (vars);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc > 1)
	{
		if (argc < 6)
		{
			print_at (0,0,mlPcMess715, argv [0]);
			print_at (1,0,mlPcMess716);
			print_at (2,0,mlPcMess717);
			print_at (3,0,mlPcMess718);
			print_at (4,0,mlPcMess719);
			return (EXIT_FAILURE);
		}

		abc_selfield (pcwo, "pcwo_hhwo_hash");

		if (strncmp (argv [1], " ", 1))
		{
			pcwo_rec.wo_hhwo_hash = atol (argv [1]);
			cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, pcwo, "DBFIND");
			strcpy (local_rec.st_wono, pcwo_rec.wo_order_no);
		}
		else
			strcpy (local_rec.st_wono, argv [1]);

		if (strncmp (argv [2], "~~~~~~~", 7))
		{
			pcwo_rec.wo_hhwo_hash = atol (argv [2]);
			cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, pcwo, "DBFIND");
			strcpy (local_rec.ed_wono, pcwo_rec.wo_order_no);
		}
		else
			strcpy (local_rec.ed_wono, argv [2]);
	
		switch (argv [3] [0])
		{
		case 'Y' :
		case 'N' :
			strcpy (local_rec.extra, argv [3]);
			break;
		default :
/*
			printf ("      extra - Y(es print required, received, and\007\n\r");
			printf ("            - rejected quantities\007\n\r");
			printf ("            - N(o not printed\007\n\r");
*/
			print_at (3,0,ML(mlPcMess718));
		}

		switch (argv [4] [0])
		{
		case 'C' :
			strcpy (local_rec.cobrwh, argv [4]);
			strcpy (local_rec.disp_cobrwh, "COMPANY");
			break;
		case 'B' :
			strcpy (local_rec.cobrwh, argv [4]);
			strcpy (local_rec.disp_cobrwh, "BRANCH");
			break;
		case 'W' :
			strcpy (local_rec.cobrwh, argv [4]);
			strcpy (local_rec.disp_cobrwh, "WAREHOUSE");
			break;
		default :
/*
			printf ("usage : level - C(ompany\007\n\r");
			printf ("              - B(ranch\007\n\r");
			printf ("              - Warehouse\007\n\r");
*/
			print_at (4,0,ML(mlPcMess719));
			return (EXIT_FAILURE);
		}

		local_rec.lp_no = atoi (argv [5]);

		/*-----------------------------
		| Process Records in Database.|
		-----------------------------*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcPcwo ();

		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		crsr_on ();

		init_vars (1);

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy(err_str, ML(mlPcMess146));
		RunProg (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*================================
| Runs the program in overnight, |
| background or foreground.      |
================================*/
void
RunProg (
 char *progname)
{
	char	lp_no [3];

	sprintf (lp_no, "%d", local_rec.lp_no);

	shutdown_prog ();

	if (local_rec.overnight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				progname,
				local_rec.st_hhwo,
				local_rec.ed_hhwo,
				local_rec.extra,
				local_rec.cobrwh,
				lp_no,
				err_str,(char *)0);
				/*"Works In Progress Report",(char *)0);*/
		}
		else
			return;
	}

	else if (local_rec.background [0] == 'Y')
	{
		if (fork () == 0)
			execlp (progname,
				progname,
				local_rec.st_hhwo,
				local_rec.ed_hhwo,
				local_rec.extra,
				local_rec.cobrwh,
				lp_no,
				(char *)0);
		else
			return;
	}
	else 
	{
		execlp (progname,
			progname,
			local_rec.st_hhwo,
			local_rec.ed_hhwo,
			local_rec.extra,
			local_rec.cobrwh,
			lp_no,
			(char *)0);
	}
}

/*=========================
| Program exit sequence	. |
=========================*/
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

	abc_alias (inmr2, inmr);
	abc_alias (pcwo2, pcwo);

	open_rec (pcwo, pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, pcwo_no_fields, "pcwo_id_no3");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum, inum_list, inum_no_fields, "inum_hhum_hash");
	open_rec (inei, inei_list, inei_no_fields, "inei_id_no");
	open_rec (pcms, pcms_list, pcms_no_fields, "pcms_id_no");
	open_rec (pcln, pcln_list, pcln_no_fields, "pcln_id_no");
	open_rec (pcrq, pcrq_list, pcrq_no_fields, "pcrq_id_no2");
	open_rec (esmr, esmr_list, esmr_no_fields, "esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pcwo);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (inei);
	abc_fclose (pcms);
	abc_fclose (pcln);
	abc_fclose (pcrq);
	abc_fclose (esmr);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startorder") || LCHECK ("startbatch")) 
	{
		if (LCHECK ("startbatch"))
			if (F_NOKEY (label ("startbatch")))
				return (EXIT_SUCCESS);

		if (last_char == SEARCH)
		{
			if (LCHECK ("startorder"))
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
			sprintf (local_rec.st_wono, "%-7.7s", " ");
			sprintf (local_rec.st_bno, "%-10.10s", " ");
			sprintf (local_rec.st_item, "%-16.16s", " ");
			sprintf (local_rec.st_desc, "%-40.40s", "First Works Order");
			strcpy (local_rec.st_hhwo, " ");
			DSP_FLD ("startorder");
			DSP_FLD ("startbatch");
			DSP_FLD ("startitem");
			DSP_FLD ("startdesc");
			if (LCHECK ("startorder"))
				FLD ("startbatch") = YES; 
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("startorder"))
			FLD ("startbatch") = NA;

		/*--------------------------------
		| read pcwo and inmr for details |
		--------------------------------*/
		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("startorder"))
		{
			if (PC_GEN)
				strcpy (pcwo_rec.wo_order_no, zero_pad (local_rec.st_wono, 7));
			else
				strcpy (pcwo_rec.wo_order_no, local_rec.st_wono);
			cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		}
		else
		{
			strcpy (pcwo_rec.wo_batch_no, local_rec.st_bno);
			cc =find_rec (pcwo2, &pcwo_rec, COMPARISON, "r");
		}
		if (cc)
		{
/*
			if (LCHECK ("startorder"))
				sprintf (err_str,
					" Works Order Does Not Exist - Order No [%s] ",
					pcwo_rec.wo_order_no);
			else
				sprintf (err_str,
					" Works Order Does Not Exist - Batch No [%s] ",
					pcwo_rec.wo_batch_no);
*/
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			strcpy (local_rec.st_wono,	" ");
			strcpy (local_rec.st_bno,	" ");
			strcpy (local_rec.st_item,	" ");
			strcpy (local_rec.st_desc,	" ");
			DSP_FLD ("startorder");
			DSP_FLD ("startbatch");
			DSP_FLD ("startitem");
			DSP_FLD ("startdesc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.st_wono, pcwo_rec.wo_order_no);
		DSP_FLD ("startorder");

		if (prog_status != ENTRY &&
			strncmp (local_rec.st_wono, local_rec.ed_wono, 7) > 0)
		{
/*
			sprintf (err_str, "Start Order %s Greater Than End Order %s",
				local_rec.st_wono,
				local_rec.ed_wono);
*/
			errmess (ML(mlStdMess017));
			sleep (sleepTime);
			strcpy (local_rec.st_wono, " ");
			DSP_FLD ("startorder");
			return (EXIT_FAILURE);
		}

		inmr_rec.mr_hhbr_hash = pcwo_rec.wo_hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (local_rec.st_wono, pcwo_rec.wo_order_no);
		strcpy (local_rec.st_bno, pcwo_rec.wo_batch_no);
		strcpy (local_rec.st_item, inmr_rec.mr_item_no);
		strcpy (local_rec.st_desc, inmr_rec.mr_description);
		sprintf (local_rec.st_hhwo, "%ld", pcwo_rec.wo_hhwo_hash);
		DSP_FLD ("startorder");
		DSP_FLD ("startbatch");
		DSP_FLD ("startitem");
		DSP_FLD ("startdesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endorder") || LCHECK ("endbatch")) 
	{
		if (LCHECK ("endbatch"))
			if (F_NOKEY (label ("endbatch")))
				return (EXIT_SUCCESS);

		if (last_char == SEARCH)
		{
			if (LCHECK ("endorder"))
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
			sprintf (local_rec.ed_wono, "%-7.7s", "~~~~~~~");
			sprintf (local_rec.ed_bno, "%-10.10s", " ");
			sprintf (local_rec.ed_item, "%-16.16s", " ");
			sprintf (local_rec.ed_desc, "%-40.40s", "Last Works Order");
			sprintf (local_rec.ed_hhwo, "%-7.7s", "~~~~~~~");
			DSP_FLD ("endorder");
			DSP_FLD ("endbatch");
			DSP_FLD ("enditem");
			DSP_FLD ("enddesc");
			if (LCHECK ("endorder"))
				FLD ("endbatch") = YES;
			return (EXIT_SUCCESS);
		}
		if (LCHECK ("endorder"))
			FLD ("endbatch") = NA;

		/*--------------------------------
		| read pcwo and inmr for details |
		--------------------------------*/
		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		if (LCHECK ("endorder"))
		{
			if (PC_GEN)
				strcpy (pcwo_rec.wo_order_no, zero_pad (local_rec.ed_wono, 7));
			else
				strcpy (pcwo_rec.wo_order_no, local_rec.ed_wono);
			cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		}
		else
		{
			strcpy (pcwo_rec.wo_batch_no, local_rec.ed_bno);
			cc =find_rec (pcwo2, &pcwo_rec, COMPARISON, "r");
		}
		if (cc)
		{
/*
			if (LCHECK ("endorder"))
				sprintf (err_str,
					" Works Order Does Not Exist - Order No [%s] ",
					pcwo_rec.wo_order_no);
			else
				sprintf (err_str,
					" Works Order Does Not Exist - Batch No [%s] ",
					pcwo_rec.wo_batch_no);
*/
			print_mess (ML(mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			strcpy (local_rec.ed_wono,	" ");
			strcpy (local_rec.ed_bno,	" ");
			strcpy (local_rec.ed_item,	" ");
			strcpy (local_rec.ed_desc,	" ");
			DSP_FLD ("endorder");
			DSP_FLD ("endbatch");
			DSP_FLD ("enditem");
			DSP_FLD ("enddesc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ed_wono, pcwo_rec.wo_order_no);
		DSP_FLD ("endorder");

		if (strncmp (local_rec.st_wono, local_rec.ed_wono, 7) > 0)
		{
/*
			sprintf (err_str, "Start Order %s Greater Than End Order %s",
				local_rec.st_wono,
				local_rec.ed_wono);
*/
			errmess (ML(mlStdMess017));
			sleep (sleepTime);
			strcpy (local_rec.ed_wono, " ");
			DSP_FLD ("endorder");
			return (EXIT_FAILURE);
		}

		inmr_rec.mr_hhbr_hash = pcwo_rec.wo_hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (local_rec.ed_wono, pcwo_rec.wo_order_no);
		strcpy (local_rec.ed_bno, pcwo_rec.wo_batch_no);
		strcpy (local_rec.ed_item, inmr_rec.mr_item_no);
		strcpy (local_rec.ed_desc, inmr_rec.mr_description);
		sprintf (local_rec.ed_hhwo, "%ld", pcwo_rec.wo_hhwo_hash);
		DSP_FLD ("endorder");
		DSP_FLD ("endbatch");
		DSP_FLD ("enditem");
		DSP_FLD ("enddesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("extra"))
	{
		strcpy (local_rec.disp_extra, local_rec.extra);
		DSP_FLD ("disp_extra");
		if (local_rec.extra [0] == 'Y')
			strcpy (local_rec.disp_extra, "Yes");
		else
			strcpy (local_rec.disp_extra, "No");
		DSP_FLD ("disp_extra");
		DSP_FLD ("extra");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cobrwh"))
	{
		strcpy (local_rec.disp_cobrwh, "         ");
		DSP_FLD ("disp_cobrwh");
		switch (local_rec.cobrwh[0])
		{
		case 'C' :
			strcpy (local_rec.disp_cobrwh, "Company  ");
			break;
		case 'B' :
			strcpy (local_rec.disp_cobrwh, "Branch   ");
			break;
		case 'W' :
			strcpy (local_rec.disp_cobrwh, "Warehouse");
			break;
		}
		DSP_FLD ("disp_cobrwh");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lp_no"))
	{
		if (FLD ("lp_no") != NA) 
		{
			if (last_char == SEARCH)
			{
				local_rec.lp_no = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lp_no))
			{
				/*print_mess ("Invalid Printer");*/
				print_mess (ML(mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bk_grd"))
	{
		strcpy (local_rec.back_desc, " ");
		DSP_FLD ("bk_grd_desc");
		if (local_rec.background [0] == 'Y')
			strcpy (local_rec.back_desc, "Yes");
		else
			strcpy (local_rec.back_desc, "No ");
		DSP_FLD ("bk_grd_desc");
		return(0);
	}

	if ( LCHECK("o_night") )
	{
		strcpy (local_rec.over_desc, " ");
		DSP_FLD ("o_night_desc");
		if (local_rec.overnight [0] == 'Y')
			strcpy (local_rec.over_desc, "Yes");
		else
			strcpy (local_rec.over_desc, "No ");
		DSP_FLD ("o_night_desc");
		return(0);
	}

	return (EXIT_SUCCESS);
}

/*=================================
| Process all valid pcwo records. |
=================================*/
int
ProcPcwo (
 void)
{
	int		printed = FALSE;
	int		branchPrint = FALSE;
	int		companyPrint = FALSE;
	int		firstTime = TRUE;

	PrintHeading ();

	/*------------------------------
	| initialising total variables |
	------------------------------*/
	total_rec.swh_material =
		total_rec.sbr_material =
		total_rec.sco_material =
		total_rec.awh_material =
		total_rec.abr_material =
		total_rec.aco_material = 0.00;

	total_rec.swh_resource =
		total_rec.sbr_resource =
		total_rec.sco_resource =
		total_rec.awh_resource =
		total_rec.abr_resource =
		total_rec.aco_resource = 0.00;

	total_rec.swh_overhead =
		total_rec.sbr_overhead =
		total_rec.sco_overhead =
		total_rec.awh_overhead =
		total_rec.abr_overhead =
		total_rec.aco_overhead = 0.00;

	total_rec.swh_hours =
		total_rec.sbr_hours =
		total_rec.sco_hours =
		total_rec.awh_hours =
		total_rec.abr_hours =
		total_rec.aco_hours = 0;

	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	if (local_rec.cobrwh[0] == 'C')
	{
		strcpy (ccmr_rec.est_no, " ");
		strcpy (ccmr_rec.cc_no, " ");
	}
	else
	{
		strcpy (ccmr_rec.est_no, comm_rec.test_no);
		if (local_rec.cobrwh[0] == 'B')
			strcpy (ccmr_rec.cc_no, " ");
		else
			strcpy (ccmr_rec.cc_no, comm_rec.tcc_no);
	}
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !ValidCcmr ())
	{
		sprintf (local_rec.prev_br, "%-2.2s", ccmr_rec.est_no);
		sprintf (local_rec.prev_wh, "%-2.2s", ccmr_rec.cc_no);
		printed = FALSE;

		strcpy (pcwo_rec.wo_co_no, ccmr_rec.co_no);
		strcpy (pcwo_rec.wo_br_no, ccmr_rec.est_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		sprintf (pcwo_rec.wo_order_no, "%-7.7s", local_rec.st_wono);
		abc_selfield (pcwo, "pcwo_id_no");
		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
		while (!cc && !ValidPcwo ())
		{
			if (pcwo_rec.wo_hhcc_hash == ccmr_rec.hhcc_hash &&
				pcwo_rec.wo_order_status [0] != 'P' &&
				pcwo_rec.wo_order_status [0] != 'Z' &&
				pcwo_rec.wo_order_status [0] != 'D')
			{
				if (!printed)
				{
					PrintWarehouseDetails (firstTime);
					firstTime = FALSE;
					printed = TRUE;
					branchPrint = TRUE;
					companyPrint = TRUE;
				}

				ReadInmrInum ();

				/*--------------------------------
				| calculate material, resource,  |
				| overhead and labour & overhead |
				| hours                          |
				--------------------------------*/
				CalcTotal ();

				/*----------------------------
				| print to screen or printer |
				----------------------------*/
				ProcLog ();
			}

			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
		if (printed)
			PrintWarehouseTotals ();
		if (local_rec.cobrwh[0] != 'W' && branchPrint)
		{
			if (strncmp (local_rec.prev_br, ccmr_rec.est_no, 2)) 
			{
				PrintBranchTotals ();
				branchPrint = FALSE;
			}
		}
	}
	if (local_rec.cobrwh[0] == 'C' && companyPrint)
		PrintCompanyTotals ();

	fprintf (fout, "====================================");
	fprintf (fout, "===============================");
	fprintf (fout, "==============================================");
	fprintf (fout, "===============================================\n");

	return (EXIT_SUCCESS);
}

int
ValidCcmr (
 void)
{
	if (local_rec.cobrwh[0] == 'C' &&
		!strncmp (ccmr_rec.co_no, comm_rec.tco_no, 2))
		return (EXIT_SUCCESS);

	if (local_rec.cobrwh[0] == 'B' &&
		!strncmp (ccmr_rec.co_no, comm_rec.tco_no, 2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.test_no, 2))
		return (EXIT_SUCCESS);

	if (local_rec.cobrwh[0] == 'W' && 
		!strncmp (ccmr_rec.co_no, comm_rec.tco_no, 2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.test_no, 2) &&
		!strncmp (ccmr_rec.cc_no, comm_rec.tcc_no, 2))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
ValidPcwo (
 void)
{
	if (!strcmp (pcwo_rec.wo_co_no, ccmr_rec.co_no) &&
		!strcmp (pcwo_rec.wo_br_no, ccmr_rec.est_no) &&
		!strcmp (pcwo_rec.wo_wh_no, comm_rec.tcc_no))
	{
		if (strncmp (local_rec.st_wono, pcwo_rec.wo_order_no, 7) <= 0 &&
			strncmp (pcwo_rec.wo_order_no, local_rec.ed_wono, 7) <= 0)
			return (EXIT_SUCCESS);
		return (EXIT_FAILURE);
	}
	return (EXIT_FAILURE);
}

void
PrintWarehouseDetails (
 int flag)
{
	if (!flag)
	{
		fprintf (fout, "|-----------------------------------");
		fprintf (fout, "-----------------------------");
		fprintf (fout, "----------------------------------------------");
		fprintf (fout, "----------------------------------------------|\n");
	}
	strcpy (esmr_rec.co_no, ccmr_rec.co_no);
	strcpy (esmr_rec.est_no, ccmr_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");

	fprintf (fout, "| Branch : %-2.2s %-40.40s Warehouse : %-2.2s %-40.40s",
		ccmr_rec.est_no,
		clip (esmr_rec.est_name),
		ccmr_rec.cc_no,
		clip (ccmr_rec.name));
	fprintf (fout, "                                               |\n");

	fprintf (fout, "|-----------------------------------");
	fprintf (fout, "-----------------------------");
	fprintf (fout, "----------------------------------------------");
	fprintf (fout, "----------------------------------------------|\n");
}

void
ReadInmrInum (
 void)
{
	inmr_rec.mr_hhbr_hash = pcwo_rec.wo_hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	inum_rec.hhum_hash = inmr_rec.mr_std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
}

/*====================================
| Calculates the standard and actual |
| costs of material, resources and   |
| overheads. Also calcualates the    |
| hours uses.                        |
====================================*/
void
CalcTotal (
 void)
{
	double	std_line_cost,
			std_ovhd_cost,
			act_line_cost,
			act_ovhd_cost,
			tot_std = 0.00,
			tot_act = 0.00,
			tmp_val; 
	long	std_tmp_time = 0,
			act_tmp_time = 0;

	total_rec.std_material = 0.00;
	total_rec.act_material = 0.00;
	total_rec.std_resource = 0.00;
	total_rec.act_resource = 0.00;
	total_rec.std_overhead = 0.00;
	total_rec.act_overhead = 0.00;
	total_rec.std_hours = 0;
	total_rec.act_hours = 0;

	/*---------------------------------------
	| Calculation of std and act. materials |
	---------------------------------------*/
	pcms_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcms_rec.uniq_id   = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		inmr2_rec.mr_hhbr_hash = pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		inei_rec.hhbr_hash = inmr2_rec.mr_hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.test_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
			inei_rec.std_cost = (double) 0.00;

		pcms_rec.matl_wst_pc += 100.00;
		pcms_rec.matl_wst_pc /= 100.00;

		tmp_val = out_cost (inei_rec.std_cost, inmr2_rec.mr_outer_size);
		tmp_val *= ( pcms_rec.matl_qty * pcms_rec.matl_wst_pc );
		tot_std += tmp_val;

		tot_act += DOLLARS (pcms_rec.amt_issued);

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	total_rec.std_material = CENTS (tot_std);
	total_rec.swh_material += total_rec.std_material;
	total_rec.sbr_material += total_rec.std_material;
	total_rec.sco_material += total_rec.std_material;

	total_rec.act_material = CENTS (tot_act);
	total_rec.awh_material += total_rec.act_material;
	total_rec.abr_material += total_rec.act_material;
	total_rec.aco_material += total_rec.act_material;

	/*----------------------------------------
	| Calculate resource and overhead costs. |
	----------------------------------------*/
	pcln_rec.hhwo_hash = pcwo_rec.wo_hhwo_hash;
	pcln_rec.seq_no = 0;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.wo_hhwo_hash)
	{
		pcrq_rec.hhwo_hash = pcln_rec.hhwo_hash;
		pcrq_rec.seq_no = pcln_rec.seq_no;
		pcrq_rec.line_no = pcln_rec.line_no;
		act_tmp_time =0.00;
		cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
		if (cc)
		{
			pcrq_rec.act_setup	= 0L;
			pcrq_rec.act_run	= 0L;
			pcrq_rec.act_clean	= 0L;
		}
		while (!cc  
				&& pcrq_rec.hhwo_hash == pcln_rec.hhwo_hash
				&& pcrq_rec.seq_no == pcln_rec.seq_no
				&& pcrq_rec.line_no == pcln_rec.line_no)
		{
			act_tmp_time +=  pcrq_rec.act_setup;
			act_tmp_time += pcrq_rec.act_run;
			act_tmp_time += pcrq_rec.act_clean;
			cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
		}

		std_tmp_time =  pcln_rec.setup;
		std_tmp_time += pcln_rec.run;
		std_tmp_time += pcln_rec.clean;

		std_line_cost = (double) std_tmp_time * pcln_rec.rate;
		std_line_cost /= 60.00;
		std_line_cost *= (double) pcln_rec.qty_rsrc;

		std_ovhd_cost = (double) std_tmp_time * pcln_rec.ovhd_var;
		std_ovhd_cost /= 60.00;
		std_ovhd_cost += pcln_rec.ovhd_fix;
		std_ovhd_cost *= (double) pcln_rec.qty_rsrc;

		act_line_cost = (double) act_tmp_time * pcln_rec.rate;
		act_line_cost /= 60.00;
		act_line_cost *= (double) pcln_rec.qty_rsrc;

		act_ovhd_cost = (double) act_tmp_time * pcln_rec.ovhd_var;
		act_ovhd_cost /= 60.00;
		act_ovhd_cost += pcln_rec.ovhd_fix;
		act_ovhd_cost *= (double) pcln_rec.qty_rsrc;

		total_rec.std_resource += std_line_cost;
		total_rec.act_resource += act_line_cost;

		total_rec.std_overhead += std_ovhd_cost;
		total_rec.act_overhead += act_ovhd_cost;

		total_rec.std_hours += std_tmp_time;
		total_rec.act_hours += act_tmp_time;

		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	/*--------------------
	| calculating totals |
	--------------------*/
	total_rec.swh_resource += total_rec.std_resource;
	total_rec.sbr_resource += total_rec.std_resource;
	total_rec.sco_resource += total_rec.std_resource;
	total_rec.awh_resource += total_rec.act_resource;
	total_rec.abr_resource += total_rec.act_resource;
	total_rec.aco_resource += total_rec.act_resource;

	total_rec.swh_overhead += total_rec.std_overhead;
	total_rec.sbr_overhead += total_rec.std_overhead;
	total_rec.sco_overhead += total_rec.std_overhead;
	total_rec.awh_overhead += total_rec.act_overhead;
	total_rec.abr_overhead += total_rec.act_overhead;
	total_rec.aco_overhead += total_rec.act_overhead;

	total_rec.swh_hours += total_rec.std_hours;
	total_rec.sbr_hours += total_rec.std_hours;
	total_rec.sco_hours += total_rec.std_hours;
	total_rec.awh_hours += total_rec.act_hours;
	total_rec.abr_hours += total_rec.act_hours;
	total_rec.aco_hours += total_rec.act_hours;
}

int
ProcLog (
 void)
{
	char	status [4];

	dsp_process ("Works Order:", pcwo_rec.wo_order_no);

	fprintf (fout, "|%-7.7s|",
		pcwo_rec.wo_order_no);
	fprintf (fout, "%-10.10s|",
		pcwo_rec.wo_batch_no);
	fprintf (fout, "%-16.16s|",
		inmr_rec.mr_item_no);
	fprintf (fout, "%-28.28s|",
		inmr_rec.mr_description);
	fprintf (fout, "%11.2f|",
		DOLLARS (total_rec.std_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.std_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.std_overhead));
	fprintf (fout, "%7.7s|",
		ttoa (total_rec.std_hours, "NNNN:NN"));
	fprintf (fout, "%11.2f|",
		DOLLARS (total_rec.act_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.act_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.act_overhead));
	fprintf (fout, "%7.7s|\n",
		ttoa (total_rec.act_hours, "NNNN:NN"));

	if (local_rec.extra [0] == 'Y')
	{
		fprintf (fout, "|      %-4.4s",
			inum_rec.uom);
		fprintf (fout, "  %14.6f",
			pcwo_rec.wo_prod_qty);
		fprintf (fout, "  %14.6f",
			pcwo_rec.wo_act_prod_qty);
		fprintf (fout, "  %14.6f",
			pcwo_rec.wo_act_rej_qty);

		switch (pcwo_rec.wo_order_status [0])
		{
		case	'F' :
			strcpy (status, "Frm");
			break;
		case	'I' :
			strcpy (status, "Iss");
			break;
		case	'A' :
			strcpy (status, "All");
			break;
		case	'R' :
			strcpy (status, "Rel");
			break;
		case	'C' :
			strcpy (status, "Clo");
			break;
		}

		fprintf (fout, "  %-3.3s ",
			status);
		fprintf (fout, "|           |            |            |       ");
		fprintf (fout, "|           |            |            |       |\n");
	}

	return (EXIT_SUCCESS);
}

void
PrintWarehouseTotals (
 void)
{
	fprintf (fout, "|                                   ");
	fprintf (fout, "Warehouse Totals             ");
	fprintf (fout, "|%11.2f|",
		DOLLARS (total_rec.swh_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.swh_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.swh_overhead));
	fprintf (fout, "%7.7s|",
		ttoa (total_rec.swh_hours, "NNNN:NN"));
	fprintf (fout, "%11.2f|",
		DOLLARS (total_rec.awh_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.awh_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.awh_overhead));
	fprintf (fout, "%7.7s|\n",
		ttoa (total_rec.awh_hours, "NNNN:NN"));

	total_rec.swh_material = 
		total_rec.awh_material = 
		total_rec.swh_resource = 
		total_rec.awh_resource =
		total_rec.swh_overhead = 
		total_rec.awh_overhead = 0.00;
	total_rec.swh_hours = 
		total_rec.awh_hours = 0;
}

void
PrintBranchTotals (
 void)
{
	fprintf (fout, "|                                   ");
	fprintf (fout, "Branch Totals                ");
	fprintf (fout, "|%11.2f|",
		DOLLARS (total_rec.sbr_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.sbr_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.sbr_overhead));
	fprintf (fout, "%7.7s|",
		ttoa (total_rec.sbr_hours, "NNNN:NN"));
	fprintf (fout, "%11.2f|",
		DOLLARS (total_rec.abr_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.abr_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.abr_overhead));
	fprintf (fout, "%7.7s|\n",
		ttoa (total_rec.abr_hours, "NNNN:NN"));

	total_rec.sbr_material = 
		total_rec.abr_material = 
		total_rec.sbr_resource = 
		total_rec.abr_resource =
		total_rec.sbr_overhead = 
		total_rec.abr_overhead = 0.00;
	total_rec.sbr_hours = 
		total_rec.abr_hours = 0;
}

void
PrintCompanyTotals (
 void)
{
	fprintf (fout, "|                                   ");
	fprintf (fout, "Company Totals               ");
	fprintf (fout, "|%11.2f|",
		DOLLARS (total_rec.sco_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.sco_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.sco_overhead));
	fprintf (fout, "%7.7s|",
		ttoa (total_rec.sco_hours, "NNNN:NN"));
	fprintf (fout, "%11.2f|",
		DOLLARS (total_rec.aco_material));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.aco_resource));
	fprintf (fout, "%12.2f|",
		DOLLARS (total_rec.aco_overhead));
	fprintf (fout, "%7.7s|\n",
		ttoa (total_rec.aco_hours, "NNNN:NN"));

	total_rec.sco_material = 
		total_rec.aco_material = 
		total_rec.sco_resource = 
		total_rec.aco_resource =
		total_rec.sco_overhead = 
		total_rec.aco_overhead = 0.00;
	total_rec.sco_hours = 
		total_rec.aco_hours = 0;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		
		rv_pr (ML(mlPcMess077), 44, 0, 1);
		move (0, 1);
		line (132);

		box (0, 3, 132, 13);
		move (1, 6);
		line (131);
		move (1, 9);
		line (131);
		move (1, 11);
		line (131);
		move (1, 13);
		line (131);

		move (0, 20);
		line (132);

		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str, comm_rec.tco_no, 
					 clip (comm_rec.tco_name));
		strcpy(err_str, ML(mlStdMess039));
		print_at(21,40,err_str, comm_rec.test_no,
					 clip (comm_rec.test_name));
		strcpy(err_str, ML(mlStdMess099));
		print_at(21,80,err_str, comm_rec.tcc_no, 
					 clip (comm_rec.tcc_name));
		move (0, 22);
		line (132);
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
		dsp_screen (" Printing Words Orders Status Report",
					comm_rec.tco_no, comm_rec.tco_name);
		/*----------------------
		| Open pipe to pformat | 
 		----------------------*/
		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);
	
		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.lp_no);
		fprintf (fout, ".12\n");
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L158\n");
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".EWORKS IN PROGRESS REPORT\n");
	fprintf (fout, ".ECOMPANY : %s\n",
		clip (comm_rec.tco_name));
	fprintf (fout, ".ESTART WORKS ORDER : %-7.7s  END WORKS ORDER : %-7.7s\n",
			local_rec.st_wono,
			local_rec.ed_wono);
	fprintf (fout, ".ELEVEL : %s\n",
		local_rec.disp_cobrwh);

	fprintf (fout, ".B1\n");
	fprintf (fout, "====================================");
	fprintf (fout, "===============================");
	fprintf (fout, "=================================");
	fprintf (fout, "==============================================");
	fprintf (fout, "===============================================\n");

	fprintf (fout, "|  W/O  |  BATCH   |    ITEM NO     ");
	fprintf (fout, "|      ITEM DESCRIPTION      ");
	fprintf (fout, "|<-----------------STANDARD------------------>");
	fprintf (fout, "|<------------------ACTUAL------------------->|\n");

	fprintf (fout, "|  NO   |    NO    |                ");
	fprintf (fout, "|                            ");
	fprintf (fout, "| MATERIAL  |  RESOURCE  |  OVERHEAD  | HOURS ");
	fprintf (fout, "| MATERIAL  |  RESOURCE  |  OVERHEAD  | HOURS |\n");

	if (local_rec.extra [0] == 'Y')
	{
		fprintf (fout, "|      UOM      REQ'D  QTY     REC'D");
		fprintf (fout, "  QTY     REJ'D  QTY     STS ");
		fprintf (fout, "|           |            |            |       ");
		fprintf (fout, "|           |            |            |       |\n");
	}

	fprintf (fout, "|-----------------------------------");
	fprintf (fout, "-----------------------------");
	fprintf (fout, "----------------------------------------------");
	fprintf (fout, "----------------------------------------------|\n");
}





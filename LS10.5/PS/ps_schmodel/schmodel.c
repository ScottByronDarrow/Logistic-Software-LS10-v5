/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_schmodel.c  )                                 |
|  Program Desc  : ( Production Scheduling Modelling.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (18/03/92)      | Author      : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : (21/04/92)      | Modified by : Campbell Mander.   |
|  Date Modified : (12/06/92)      | Modified by : Campbell Mander.   |
|  Date Modified : (04/08/92)      | Modified by : Trevor van Bremen  |
|  Date Modified : (01/03/94)      | Modified by : Aroha Merrilees.   |
|  Date Modified : (08/10/1999)    | Modified by : edge cabalfin      |
|                :                                                    |
|                                                                     |
|  Comments      : (21/04/92) - Fix for zero downtime (node should    |
|                : not be added if downtime = 0).                     |
|  (12/06/92)    : Fix updating of schedule. SC 7228 DPL.             |
|  (04/08/92)    : Allow for scheduling to be disabled. S/C DPL 7487  |
|  (01/03/94)    : DPL 10366 - Upgrade from ver7 to ver9, displays    |
|                : batch number, update pcwo index.                   |
|  (08/10/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                :                                                    |
|                                                                     |
| $Log: schmodel.c,v $
| Revision 5.3  2002/08/14 04:30:22  scott
| Updated for Linux Error
|
| Revision 5.2  2001/08/09 09:16:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:16  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/03/02 01:00:08  scott
| Updated to line up fields.
|
| Revision 3.1  2001/02/15 09:25:29  scott
| Updated as Model prompt wrong in database.
|
| Revision 3.0  2000/10/10 12:18:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:52  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/02/18 02:24:44  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.14  1999/11/17 06:40:33  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/16 02:59:23  scott
| Updated to fix warning errors from using -Wall flag on compiler.
|
| Revision 1.12  1999/10/13 22:51:36  scott
| Updated from Ansi
|
| Revision 1.11  1999/10/13 22:41:15  scott
| Updated from Ansi update script
|
| Revision 1.10  1999/10/13 22:38:28  scott
| Updated from Ansi Project
|
| Revision 1.9  1999/06/18 05:50:36  scott
| Updated to add cvs log information.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: schmodel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PS/ps_schmodel/schmodel.c,v 5.3 2002/08/14 04:30:22 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#include	<pslscr.h>
#include	<getnum.h>
#include	<ring_menu.h>
#include	<hot_keys.h>
#include	<gantt.h>
#include    <tabdisp.h>
#include	<ml_ps_mess.h>


/*==================================
|   Constants, defines and stuff    |
 ==================================*/
/*  QUERY
    these should be declared as const char*
    to minimize potential problems.
*/
	char	*data	= "data",
			*comm	= "comm",
			*rgrs	= "rgrs",
			*pccl	= "pccl",
			*psqm	= "psqm",
			*psqm2	= "psqm2",
			*pstq	= "pstq",
			*pstq2	= "pstq2",
			*pstq3	= "pstq3",
			*pcrq	= "pcrq",
			*pcwo	= "pcwo",
			*pcwo2	= "pcwo2",
			*inmr	= "inmr";

char	*del_msg = "\007Are You Sure You Want To Delete This Model ? ";
char	*empty_mdl = "\007 No Data Has Been Chosen For Model So It Will Not Be Created ";
char	*overlap = "\007 There are OVERLAPPING jobs in this model. Schedule will not be updated. ";

/*--------------------------------------
| These are options to node allocation |
--------------------------------------*/
#define	ALLC_ND		0
#define	FREE_ND		1
#define	FREE_LST	2

/*-----------------------------------
| These map on to ring_menu options |
-----------------------------------*/
#define	NEW_MDL		1
#define	DEL_MDL		2
#define	CPY_MDL		3
#define	MOD_MDL		4
#define	UPD_SCH		5
#define	EXIT_PROG	6

#define	DATE_TIME(x, y)	((x * 24L * 60L) + y)
#define	LIVE_STEP	(status == 'L')

static	struct
{
	long	st_date;
	long	st_time;
	long	setup_time;
	long	run_time;
	long	clean_time;
	char	dummy[10];
} time_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_dt",	18, 120, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "START DATE:", "",
		 NO, NO, JUSTRIGHT, "", "", (char *) &time_rec.st_date},
	{1, LIN, "st_tm",	19, 120, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "START TIME:", "",
		 NO, NO, JUSTRIGHT, "", "", (char *) &time_rec.st_time},
	{1, LIN, "setup",	20, 120, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "SETUP:", "",
		 NO, NO, JUSTRIGHT, "", "", (char *) &time_rec.setup_time},
	{1, LIN, "run",		21, 120, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "RUN:", "",
		 NO, NO, JUSTRIGHT, "", "", (char *) &time_rec.run_time},
	{1, LIN, "clean",	22, 120, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "CLEAN:", "",
		 NO, NO, JUSTRIGHT, "", "", (char *) &time_rec.clean_time},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", time_rec.dummy},
};

struct	gnt_mnu
{
	char	*text;
	int	(* fn)(int);
	struct	gnt_mnu	*sub_mnu;
	int	ret_val;
};
#define	GNT_MNU_NULL	((struct gnt_mnu *) 0)

/* QUERY 
 * This function prototype is needed by the following data
 * structures. That is why this is here instead of the usual
 * place
 */
int	gnt_null_func (int x);
struct gnt_mnu job_mnu[]=
{
	{"Pack Jobs                [P]",	gnt_null_func,	GNT_MNU_NULL, 'P'},
	{"Add a Job                [A]",	gnt_null_func,	GNT_MNU_NULL, 'A'},
	{"Delete a Job             [D]",	gnt_null_func,	GNT_MNU_NULL, 'D'},
	{"View Job(s)              [V]",	gnt_null_func,	GNT_MNU_NULL, 'V'},
	{"Highlight Job            [H]",	gnt_null_func,	GNT_MNU_NULL, 'H'},
	{"Change Step Duration     [C]",	gnt_null_func,	GNT_MNU_NULL, 'C'},
	{"Move a Step              [M]",	gnt_null_func,	GNT_MNU_NULL, 'M'},
	{"Split a Step             [S]",	gnt_null_func,	GNT_MNU_NULL, 'S'},
	{"Join 2 Steps             [J]",	gnt_null_func,	GNT_MNU_NULL, 'J'},
	{(char *) 0}
};

struct gnt_mnu scn_mnu[]=
{
	{"Zoom In                  [I]",	gnt_null_func,	GNT_MNU_NULL, 'I'},
	{"Zoom Out                 [O]",	gnt_null_func,	GNT_MNU_NULL, 'O'},
	{"Page Up               [NEXT]", gnt_null_func,	GNT_MNU_NULL, FN14},
	{"Page Down             [PREV]", gnt_null_func,	GNT_MNU_NULL, FN15},
	{"Top Left                 [7]", gnt_null_func,	GNT_MNU_NULL, '7'},
	{"Top Centre               [8]", gnt_null_func,	GNT_MNU_NULL, '8'},
	{"Top Right                [9]", gnt_null_func,	GNT_MNU_NULL, '9'},
	{"Middle Left              [4]", gnt_null_func,	GNT_MNU_NULL, '4'},
	{"Middle Centre            [5]", gnt_null_func,	GNT_MNU_NULL, '5'},
	{"Middle Right             [6]", gnt_null_func,	GNT_MNU_NULL, '6'},
	{"Bottom Left              [1]", gnt_null_func,	GNT_MNU_NULL, '1'},
	{"Bottom Centre            [2]", gnt_null_func,	GNT_MNU_NULL, '2'},
	{"Bottom Right             [3]", gnt_null_func,	GNT_MNU_NULL, '3'},
	{(char *) 0}
};

struct gnt_mnu hed_mnu[]=
{
	{"Screen Menu",	        gnt_null_func,  scn_mnu, -1},
	{"Job Menu",	            gnt_null_func,  job_mnu, -1},
	{(char *) 0}
};


	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"}
	};

	const int comm_no_fields = 7;

	struct
	{
		int   termno;
		char  tco_no[3];
		char  tco_name[41];
		char  test_no[3];
		char  test_name[41];
		char  tcc_no[3];
		char  tcc_name[41];
	} comm_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list[] =
	{
		{"rgrs_hhrs_hash"},
		{"rgrs_co_no"},
		{"rgrs_br_no"},
		{"rgrs_code"},
		{"rgrs_desc"},
		{"rgrs_type"},
		{"rgrs_type_name"},
		{"rgrs_cost_type"},
		{"rgrs_rate"},
		{"rgrs_qty_avl"},
		{"rgrs_ovhd_var"},
		{"rgrs_ovhd_fix"},
		{"rgrs_cal_sel"},
	};

	const int rgrs_no_fields = 13;

	struct RGRS_REC rgrs_rec; /* Defined within gantt.h */

	/*===============================
	| Production Control CaLendar(s |
	===============================*/
	struct dbview pccl_list[] =
	{
		{"pccl_co_no"},
		{"pccl_br_no"},
		{"pccl_hhrs_hash"},
		{"pccl_act_date"},
		{"pccl_act_time"},
		{"pccl_duration"},
		{"pccl_stat_flag"},
	};

	const int pccl_no_fields = 7;

	struct	
	{
		char	cl_co_no[3];
		char	cl_br_no[3];
		long	cl_hhrs_hash;
		long	cl_act_date;
		long	cl_act_time;
		long	cl_duration;
		char	cl_stat_flag[2];
	} pccl_rec;

	/*================================================
	| Production Scheduling Q Modelling Control File |
	================================================*/
	struct dbview psqm_list[] =
	{
		{"psqm_co_no"},
		{"psqm_model_name"},
		{"psqm_hhqm_hash"},
		{"psqm_dt_create"},
		{"psqm_tm_create"},
		{"psqm_op_create"},
		{"psqm_dt_modify"},
		{"psqm_tm_modify"},
		{"psqm_stat_flag"},
	};

	const int psqm_no_fields = 9;

	struct	
	{
		char	qm_co_no[3];
		char	qm_model_name[11];
		long	qm_hhqm_hash;
		long	qm_dt_create;
		long	qm_tm_create;
		char	qm_op_create[15];
		long	qm_dt_modify;
		long	qm_tm_modify;
		char	qm_stat_flag[2];
	} psqm_rec;

	/*=======================================
	| Production Scheduling Temporary Queue |
	=======================================*/
	struct dbview pstq_list[] =
	{
		{"pstq_hhqm_hash"},
		{"pstq_hhrs_hash"},
		{"pstq_hhwo_hash"},
		{"pstq_seq_no"},
		{"pstq_line_no"},
		{"pstq_st_date"},
		{"pstq_st_time"},
		{"pstq_setup"},
		{"pstq_run"},
		{"pstq_clean"},
		{"pstq_stat_flag"},
	};

	const int pstq_no_fields = 11;

	struct	
	{
		long  tq_hhqm_hash;
		long  tq_hhrs_hash;
		long  tq_hhwo_hash;
		int   tq_seq_no;
		int   tq_line_no;
		long  tq_st_date;
		long  tq_st_time;
		long  tq_setup;
		long  tq_run;
		long  tq_clean;
		char  tq_stat_flag[2];
	} pstq_rec;

	/*===================================
	| Production Control Resource Queue |
	===================================*/
	struct dbview pcrq_list[] =
	{
		{"pcrq_hhrs_hash"},
		{"pcrq_qty_rsrc"},
		{"pcrq_hhwo_hash"},
		{"pcrq_prod_class"},
		{"pcrq_priority"},
		{"pcrq_seq_no"},
		{"pcrq_line_no"},
		{"pcrq_last_date"},
		{"pcrq_last_time"},
		{"pcrq_act_date"},
		{"pcrq_act_time"},
		{"pcrq_act_setup"},
		{"pcrq_act_run"},
		{"pcrq_act_clean"},
		{"pcrq_est_date"},
		{"pcrq_est_time"},
		{"pcrq_est_setup"},
		{"pcrq_est_run"},
		{"pcrq_est_clean"},
		{"pcrq_firm_sched"},
		{"pcrq_stat_flag"},
	};

	const int pcrq_no_fields = 21;

	struct	
	{
		long  rq_hhrs_hash;
		int   rq_qty_rsrc;
		long  rq_hhwo_hash;
		char  rq_prod_class[5];
		int   rq_priority;
		int   rq_seq_no;
		int   rq_line_no;
		long  rq_last_date;
		long  rq_last_time;
		long  rq_act_date;
		long  rq_act_time;
		long  rq_act_setup;
		long  rq_act_run;
		long  rq_act_clean;
		long  rq_est_date;
		long  rq_est_time;
		long  rq_est_setup;
		long  rq_est_run;
		long  rq_est_clean;
		char  rq_firm_sched[2];
		char  rq_stat_flag[2];
	} pcrq_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list[] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_reqd_date"},
		{"pcwo_rtg_seq"},
		{"pcwo_priority"},
		{"pcwo_create_date"},
		{"pcwo_mfg_date"},
		{"pcwo_hhbr_hash"},
		{"pcwo_bom_alt"},
		{"pcwo_rtg_alt"},
		{"pcwo_hhcc_hash"},
		{"pcwo_prod_qty"},
		{"pcwo_act_prod_qty"},
		{"pcwo_act_rej_qty"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
		{"pcwo_stat_flag"},
	};

	const int pcwo_no_fields = 20;

	struct	
	{
		char  wo_co_no[3];
		char  wo_br_no[3];
		char  wo_wh_no [3];
		char  wo_order_no[8];
		long  wo_hhwo_hash;
		long  wo_reqd_date;
		int   wo_rtg_seq;
		int   wo_priority;
		long  wo_create_date;
		long  wo_mfg_date;
		long  wo_hhbr_hash;
		int   wo_bom_alt;
		int   wo_rtg_alt;
		long  wo_hhcc_hash;
		float wo_prod_qty;
		float wo_act_prod_qty;
		float wo_act_rej_qty;
		char  wo_order_status[2];
		char  wo_batch_no [11];
		char  wo_stat_flag[2];
	} pcwo_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list[] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
	};

	const int inmr_no_fields = 3;

	struct	
	{
		char  mr_item_no[17];
		long  mr_hhbr_hash;
		char  mr_description[41];
	} inmr_rec;



/*===========================
| Local & Screen Structures |
===========================*/
struct	
{
	char	dummy[11];
	char	new_mdl[11];
	char	systemDate[11];
	long	lsystemDate;
	char	logname[15];
} local_rec;

struct	MDL_SEL
{
	struct	MDL_SEL	*next;
	struct	MDL_SEL	*prev;
	char	save_name[60];
	long	hhqm_hash;
};

#define	MDL_NULL	((struct MDL_SEL *) NULL)
struct	MDL_SEL		*mdl_head = MDL_NULL;
struct	MDL_SEL		*mdl_free = MDL_NULL;
struct	MDL_SEL		*mdl_curr = MDL_NULL;
struct	MDL_SEL		*mdl_slct = MDL_NULL;

/*  QUERY
    these functions are needed by the data structure below.
    that is why they are here and not in the usual place.
*/
static	int	new_mdl (void);
static	int	del_mdl (void);
static	int	cpy_mdl (void);
static	int	mod_mdl (void);
static	int	upd_mdl (void);
static	int	re_draw (void);
static	int	exit_func (void);

menu_type main_menu[] = 
{
    {"                   ",    "", _no_option,  "",0, SHOW  },
    {"<New Model>",    "Create New Model.  [ N ]", new_mdl,   "Nn", 0,    ALL,  },
    {"<Delete Model>", "Delete Model.  [ D ]",     del_mdl,   "Dd", 0,    ALL,  },
    {"<Copy Model>",   "Copy Model.  [ C ]",       cpy_mdl,   "Cc", 0,    ALL,  },
    {"<Modify Model>", "Modify Model.  [ M ]",     mod_mdl,   "Mm", 0,    ALL,  },
    {"<Update Schedule>","Update Model To Become The Live Schedule.  [ U ]",upd_mdl, "Uu", 0, ALL,},
    {"<[REDRAW]>",         "Redraw Display",           re_draw,   "",   FN3,        },
    {"<EDIT/END>",         "Exit Display",             exit_func, "",   FN16, ALL   },
    {"",								            },
};

/*  QUERY
    these functions are needed by the data structure below.
    that is why they are here and not in the usual place.
*/
static	int	job_tag_func(int c, KEY_TAB *psUnused);
static	int	exit_func2 (int c, KEY_TAB *psUnused);
static	int	dummy_func (int c, KEY_TAB *psUnused);
static KEY_TAB new_mkeys [] =
{
   { "[T]AG/UNTAG",	 'T',       job_tag_func,
	"Tag/Untag job for inclusion in model.",		"A" },
   { "[^T]AG/UNTAG ALL", CTRL('T'), job_tag_func,
	"Tag/Untag job for inclusion in model.",		"A" },
   { NULL,		 '\r',      job_tag_func,
	"Tag/Untag job for inclusion in model.",		"A" },
   { NULL,		 FN1,       exit_func2,
	"Exit Without Selection Of Jobs.",			"A" },
   { NULL,		 FN16,      dummy_func,
	"Selection of users complete.",				"A" },
   END_KEYS
};

/*  QUERY
    these functions are needed by the data structure below.
    that is why they are here and not in the usual place.
*/
static int done_func (int c, KEY_TAB *psUnused);
static int del_tag_func (int c, KEY_TAB *psUnused);

static KEY_TAB del_jkeys [] =
{
	{ "[T]AG/UNTAG",	'T',       del_tag_func,
		"Tag/Untag Job for deletion from Model.",		"A" },
	{ "[^T]AG/UNTAG ALL", CTRL('T'), del_tag_func,
		"Tag/Untag Job for deletion from Model.",		"A" },
	{ NULL,		'\r',      del_tag_func,
		"Tag/Untag Job for deletion from Model.",		"A" },
	{ NULL,		FN1,       done_func,
		"Exit without selection of Jobs.",			"A" },
	{ NULL,		FN16,      done_func,
		"Selection of Jobs to delete complete.",		"A" },
	END_KEYS
};


/*====================
|   Local variables   |
 ====================*/

struct	RSL_LIST *rsl_curr,
                 *scn_head;
struct	STP_LIST *old_stp = STP_NULL;
struct	STP_LIST *curr_stp = STP_NULL;
static	char	curr_chr;
static	char	old_chr;
static	int	indx = 0;
static	int	x_pos = 0,
		y_pos = 0;
static	long	curr_dt_tm;		/* Actual Date/Time (ie: NOW)	*/
static	long	min_resn = 0L;		/* Minimum time resolution.	*/
static	long	time_resn = 0L;		/* No. mins per screen char.	*/
static	long	beg_dt_tm;		/* Date/Time of LHS of screen	*/
static	long	end_dt_tm;		/* Date/Time of RHS of screen	*/
static	int		do_hilite;
static	long	act_times[20] =
{
	1L,2L,3L,4L,5L,6L,10L,12L,15L,20L,30L,		/* Minutes	*/
	60L,120L,180L,240L,360L,480L,720L,		/* Hours	*/
	1440L,						/* Days		*/
	10080L,						/* Weeks	*/
};
static	long	time_incs[20] =
{
	10L,20L,30L,40L,50L,60L,100L,120L,150L,200L,300L,
	600L,1440L,2880L,2880L,4320L,5760L,10080L,
	20160L,131040L,
};
static	char	val_times[21];

int     opt_type;
int	    reload;
int	    jobs_in_model;
long    slct_hhqm;		/* hhqm_hash of selected model */

/*==============================
|   Local function prototypes   |
 ==============================*/

struct RGRS_REC *mk_rsh_node (void), *lcl_rsh;
struct RSL_LIST *mk_rsl_node (void), *lcl_rsl;
struct JOB_LIST *fnd_job (long hhwo_hash, struct STP_LIST *curr_stp);
struct STP_LIST *stp_node (int node_act, struct STP_LIST *lst_ptr);
struct JOB_LIST *job_node (int node_act, struct JOB_LIST *lst_ptr);
struct MDL_SEL  *mdl_node (int node_act, struct MDL_SEL *lst_ptr);

void shutdown_prog (void);
void ReadMisc (void);
void OpenDB (void);
void CloseDB (void);
int  heading (int scn);

void load_rsrc (void);
void load_calendar (void);
void ld_mdl_names (int loadall);
int  create_model (void);
int  get_mdl_name (void);
int  mdl_name_used (void);
int  tag_new_job (int trow, int tcol, int tdpth, int selective);
void add_psqm (void);
void add_pstq (long hhwo_hash, char status);
void shuffle_pstqs (long hhwo_hash, struct JOB_LIST *lcl_job);
int  add_live_stp (struct STP_LIST *lcl_stp, struct JOB_LIST *lcl_job, struct RSL_LIST *first_rsl);
int  proc_model (int load_list);
int  copy_model (void);
int  delete_model (void);
int  load_model (void);
int  upd_sched (void);
void fnd_horizontal (struct RSL_LIST *curr_rsl, struct STP_LIST *curr_stp);
void fnd_vertical (struct STP_LIST *curr_stp);
void clear_steps (void);
void upd_lst_dt_tm (struct RSL_LIST *rsl);
void del_jobs (void);
void del_pstq (long hhwo_hash);
void add_jobs (void);
void view_jobs (void);
int  gantt (void);
int  gnt_sched (int x);
void prep_scn (struct RSL_LIST *scn_head, int flag);
void dsp_hdgs (void);
void dsp_rgrs (struct RSL_LIST *rsl_list);
void dsp_jobs (struct STP_LIST *stp_list, int row);
void get_job_info (struct STP_LIST *stp_list);
char check_overlap (struct STP_LIST *stp_list, long tmp_dt_tm, char dsp_chr);
void dsp_jdt (struct RSL_LIST *tmp_rsl);
void do_help (void);
int  do_menu (struct gnt_mnu *mnu_ptr);
int  gnt_null_func (int x);
int  change_step (void);
int  hilite_job (void);
int  move_step (void);
void re_schedule (struct STP_LIST *step, struct RSL_LIST *src_rsl, struct RSL_LIST *dst_rsl);
void pack_jobs (void);
int  join_step (void);
int  split_step (void);
int  valid_mdl (void);



/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc, 
 char   *argv[])
{
	int    modified;
	char   modify_str[25];
	char   *sptr = getenv ("LOGNAME"),
	       *xptr = get_env ("PS_VAL_TIMES");

	if (strchr (xptr, 'Y') == (char *) NULL)
	{
		print_at (0,0, "Sorry, your system is not set up for scheduling!!");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Setup required parameters	|
	-------------------------------*/
	sprintf (local_rec.logname,  "%-14.14s", (sptr) ? sptr : " ");
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();
	init_scr ();
	set_tty ();

	OpenDB ();
	ReadMisc ();

	input_row = 18;
	heading (0);

	/*--------------------
	| Load resource list |
	--------------------*/
	load_rsrc ();
	
	/*------------------------
	| Load Calendar Downtime |
	------------------------*/
	load_calendar ();

	/*----------------------------
	| Set initial pointer values |
	----------------------------*/
	job_head = JOB_NULL;
	job_tail = JOB_NULL;
	job_free = JOB_NULL;
	stp_free = STP_NULL;

	/*----------------------------
	| Select Model To Manipulate |
	----------------------------*/
	ld_mdl_names (TRUE);
	while (opt_type != EXIT_PROG)
	{
	    if (opt_type == NEW_MDL)
	    {
			mdl_slct = mdl_head;
		}
	    else
	    {
			mdl_slct = (struct MDL_SEL *) win_select ("MODEL SELECTION",
                                                      47, 
                                                      (struct SEL_STR *) mdl_head,
                                                      0,3,132,13);
			/*--------------------
			| Clear message line |
			--------------------*/
			blank_at (2, 0, 80);

			if (mdl_slct == MDL_NULL)
			{
				if (last_char == FN1)
				{
					ld_mdl_names (FALSE);
				}
				else
				{
					win_display ("MODEL SELECTION",
                                 47, 
                                 (struct SEL_STR *) mdl_head,
                                 0,3,132,13);
				}

				continue;
			}
	    }

	    switch (opt_type)
	    {
	    case NEW_MDL:
            /*------------------
            | Create new model |
            ------------------*/
            clear_steps ();
            reload = create_model ();
            if (reload)
            {
                slct_hhqm = psqm_rec.qm_hhqm_hash;
                modified = proc_model (FALSE);
            }
            else
            {
                slct_hhqm = 0L;
            }

            break;

	    case DEL_MDL:
			/*--------------------------
			| Delete an existing model |
			--------------------------*/
			slct_hhqm = mdl_slct->hhqm_hash;
			reload = delete_model ();
			
			break;

	    case CPY_MDL:
			/*------------------------
			| Copy an existing model |
			------------------------*/
			slct_hhqm = mdl_slct->hhqm_hash;
			reload = copy_model ();
			modified = proc_model (TRUE);
			
			break;

	    case MOD_MDL:
			/*--------------------------
			| Modify an existing model |
			--------------------------*/
			strcpy (psqm_rec.qm_co_no, comm_rec.tco_no);
			sprintf (psqm_rec.qm_model_name,"%-10.10s", mdl_slct->save_name);
			cc = find_rec (psqm, &psqm_rec, COMPARISON, "u");
			if (cc)
			{
				file_err (cc, psqm, "DBFIND");
			}

			slct_hhqm = mdl_slct->hhqm_hash;
			modified = proc_model (TRUE);
			if (modified)
			{
				sprintf (modify_str,
				         "%-10.10s %-5.5s",
				         DateToString (psqm_rec.qm_dt_modify),
				         ttoa (psqm_rec.qm_tm_modify, "NN:NN"));

				sprintf (mdl_slct->save_name, 
				         "%-10.10s %-10.10s %-5.5s - %-16.16s", 
				         psqm_rec.qm_model_name,
				         DateToString (psqm_rec.qm_dt_create),
				         ttoa (psqm_rec.qm_tm_create, "NN:NN"),
				         modify_str);
			}
			reload = FALSE;
			break;

	    case UPD_SCH:
			slct_hhqm = mdl_slct->hhqm_hash;
			reload = upd_sched ();
			break;

	    default:
			reload = TRUE;
			break;
	    }

	    ld_mdl_names (reload);
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*----------------------------------
| The following routines set the   |
| option type ready for win_select |
----------------------------------*/
int 
new_mdl (
 void)
{
	main_menu[DEL_MDL].flag = ALL;
	main_menu[CPY_MDL].flag = ALL;
	main_menu[MOD_MDL].flag = ALL;
	main_menu[UPD_SCH].flag = ALL;
	opt_type = NEW_MDL;
	return (EXIT_SUCCESS);
}

int
del_mdl (
 void)
{
	opt_type = DEL_MDL;
	rv_pr ("\007 Choose Model To Delete ", 1, 2, 1);
	return (EXIT_SUCCESS);
}

int 
cpy_mdl (
 void)
{
	opt_type = CPY_MDL;
	rv_pr ("\007 Choose Model To Copy ", 1, 2, 1);
	return (EXIT_SUCCESS);
}

int
mod_mdl (
 void)
{
	opt_type = MOD_MDL;
	rv_pr ("\007 Choose Model To Modify ", 1, 2, 1);
	return (EXIT_SUCCESS);
}

int 
upd_mdl (
 void)
{
	opt_type = UPD_SCH;
	rv_pr ("\007 Choose Model Which Is To Be Updated As The Live Production Schedule ", 1, 2, 1);

	return (EXIT_SUCCESS);
}

int
exit_func (
 void)
{
	return (exit_func2 (0, (KEY_TAB *) 0));
}

int
exit_func2 (
 int    c,
 KEY_TAB *psUnused)
{
	if (c == FN1)
	{
		restart = TRUE;
	}
	else
	{
		opt_type = EXIT_PROG;
	}

	return (c);
}

int	
dummy_func (
 int    c, 
 KEY_TAB *psUnused)
{
	return (c);
}

/*-------------------------------
| Redraw model selection screen |
-------------------------------*/
int
re_draw (
 void)
{
	heading (0);
	win_display ("MODEL SELECTION", 47, (struct SEL_STR *) mdl_head, 0, 3, 132, 13);
	if (mdl_head == MDL_NULL)
	{
		rv_pr ("\007 T H E R E   A R E   N O   M O D E L S   T O   C H O O S E ", 38, 10, 1);	
		main_menu[DEL_MDL].flag = SHOW;
		main_menu[CPY_MDL].flag = SHOW;
		main_menu[MOD_MDL].flag = SHOW;
		main_menu[UPD_SCH].flag = SHOW;
	}
	
	return (EXIT_SUCCESS);
}

/*--------------------------------
| Tag jobs to include in a model |
--------------------------------*/
static int
job_tag_func (
 int    c, 
 KEY_TAB *psUnused)
{
	switch (c)
	{
	case 'T':
	case '\r':
		tag_toggle ("new_mdl");
		break;

	case CTRL ('T'):
		tag_all ("new_mdl");
		break;

	default:
		break;
	}

	return (c);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	/*------------------------------
	| Free the list of model names |
	------------------------------*/
	mdl_node (FREE_LST, mdl_head);

	CloseDB (); 
	FinishProgram ();
}

/*===============================================
| Get common info from commom database file     |
===============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
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
	abc_alias (psqm2, psqm);
	abc_alias (pstq2, pstq);	/* opened on id_no, used to copy a model */
	abc_alias (pstq3, pstq); /* opened on id_no2, used for seq shuffle */

	open_rec (rgrs,  rgrs_list, rgrs_no_fields, "rgrs_id_no");
	open_rec (pccl,  pccl_list, pccl_no_fields, "pccl_id_no");

	open_rec (psqm,  psqm_list, psqm_no_fields, "psqm_id_no");
	open_rec (psqm2, psqm_list, psqm_no_fields, "psqm_hhqm_hash");

	open_rec (pstq,  pstq_list, pstq_no_fields, "pstq_id_no");
	open_rec (pstq2, pstq_list, pstq_no_fields, "pstq_id_no");
	open_rec (pstq3, pstq_list, pstq_no_fields, "pstq_id_no2");

	open_rec (pcrq,  pcrq_list, pcrq_no_fields, "pcrq_id_no2");

	open_rec (pcwo,  pcwo_list, pcwo_no_fields, "pcwo_id_no2");
	open_rec (pcwo2, pcwo_list, pcwo_no_fields, "pcwo_hhwo_hash");

	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_hhbr_hash");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (rgrs);
	abc_fclose (pccl);

	abc_fclose (psqm);
	abc_fclose (psqm2);

	abc_fclose (pstq);
	abc_fclose (pstq2);
	abc_fclose (pstq3);

	abc_fclose (pcrq);

	abc_fclose (pcwo);
	abc_fclose (pcwo2);

	abc_fclose (inmr);

	abc_dbclose (data);
}

/*--------------------------------------
| Load resource list from rgrs records |
--------------------------------------*/
void
load_rsrc (
 void)
{
	int	i;
	struct RGRS_REC *lcl_rsh;
	struct RSL_LIST *lcl_rsl;
	
	rsl_head = RSL_NULL;
	rsl_tail = RSL_NULL;

	strcpy (rgrs_rec.rs_co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.rs_br_no, comm_rec.test_no);
	sprintf (rgrs_rec.rs_code, "%-8.8s", " ");

	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (rgrs_rec.rs_co_no, comm_rec.tco_no) &&
	       !strcmp (rgrs_rec.rs_br_no, comm_rec.test_no))
	{
		/*------------------------
		| Store resource details |
		------------------------*/
		lcl_rsh = mk_rsh_node ();
		memcpy ((char *)lcl_rsh,
		        (char *)&rgrs_rec,
		         sizeof (struct RGRS_REC));

		for (i = 0; i < rgrs_rec.rs_qty_avl; i++)
		{
			lcl_rsl = mk_rsl_node ();

			lcl_rsl->rgrs_ptr = lcl_rsh;
			lcl_rsl->fst_job  = STP_NULL;
			lcl_rsl->lst_job  = STP_NULL;
			lcl_rsl->fst_stp  = STP_NULL;
			lcl_rsl->lst_stp  = STP_NULL;
			lcl_rsl->nxt_rsl  = RSL_NULL;
	
			/*--------------------
			| Link resource list |
			--------------------*/
			if (rsl_head == RSL_NULL)
			{
				rsl_head = lcl_rsl;
				lcl_rsl->prv_rsl = RSL_NULL;
			}
			else
			{
				lcl_rsl->prv_rsl = rsl_tail;
				rsl_tail->nxt_rsl = lcl_rsl;
			}
		
			rsl_tail = lcl_rsl;
		}

		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	
	return;
}

/*-----------------------------------------
| Load resource downtime due to calendars |
-----------------------------------------*/
void
load_calendar (
 void)
{
	int    global_cal;
	int    pos_found;
	long   abs_st_time;
	long   abs_end_time;
	long   curr_hhrs;
	long   down_time;
	
	struct RSL_LIST    *lcl_rsl, 
	                   *curr_rsl;
	                   
	struct STP_LIST    *lcl_stp, 
	                   *curr_stp;

	/*=========================================================
	| NOTE WELL: pccl contains ACTIVE times for a resource.   |
	| Therefore downtime must be calculated as the difference |
	| between the end time of the last active record and the  |
	| start time of the current active record.                |
	=========================================================*/

	/*-------------------------------------------
	| For each resource load calendar downtime. |
	| Downtime is an immovable STP node with    |
	| its job_dtl set to RSL_NULL.              |
	-------------------------------------------*/
	curr_hhrs = 0L;
	curr_rsl  = RSL_NULL;
	lcl_rsl   = rsl_head;
	while (lcl_rsl != RSL_NULL)
	{
	    /*----------------------------------------
	    | Same hhrs hash as last resource node   |
	    | so copy calendar from last node rather |
	    | than reading from pccl again.          |
	    ----------------------------------------*/
	    if (lcl_rsl->rgrs_ptr->rs_hhrs_hash == curr_hhrs)
	    {
			lcl_stp = curr_rsl->fst_stp;
			while (lcl_stp != STP_NULL)
			{
			    /*------------------------------------------
			    | NB There are no vertical links or job    |
			    | details associated with calendar nodes.  |
			    ------------------------------------------*/
			    curr_stp = stp_node (ALLC_ND, STP_NULL);
			    curr_stp->rsl_ptr      = lcl_rsl;
			    curr_stp->st_date_time = lcl_stp->st_date_time;
			    curr_stp->duration     = lcl_stp->duration;
			    curr_stp->run          = lcl_stp->run;
			    curr_stp->status       = 'A';

			    /*--------------------------------
			    | Set up horizontal pointers     |
			    | NB. THIS IS A STRAIGHT APPEND. |
			    --------------------------------*/
			    if (lcl_rsl->fst_stp == STP_NULL)
			    {	
					lcl_rsl->fst_stp = curr_stp;
					lcl_rsl->lst_stp = curr_stp;
			    }
			    else
			    {
					curr_stp->prv_job         = lcl_rsl->lst_stp;
					lcl_rsl->lst_stp->nxt_job = curr_stp;
					lcl_rsl->lst_stp          = curr_stp;
			    }

			    lcl_stp = lcl_stp->nxt_job;
			}

			lcl_rsl = lcl_rsl->nxt_rsl;
			continue;
	    }

	    /*-------------------------
	    | Read calendar from pccl |
	    -------------------------*/
	    abs_end_time = 0L;
	    curr_rsl = lcl_rsl;
	    curr_hhrs = lcl_rsl->rgrs_ptr->rs_hhrs_hash;
	    global_cal = (lcl_rsl->rgrs_ptr->rs_cal_sel[0] == 'G');

	    strcpy (pccl_rec.cl_co_no, comm_rec.tco_no);
	    strcpy (pccl_rec.cl_br_no, comm_rec.test_no);

	    if (global_cal)
	    {
	    	pccl_rec.cl_hhrs_hash = 0L;
	    }
	    else
	    {
	    	pccl_rec.cl_hhrs_hash = curr_hhrs;
	    }
	    pccl_rec.cl_act_date  = 0L;
	    pccl_rec.cl_act_time  = 0L;

	    cc = find_rec (pccl, &pccl_rec, GTEQ, "r");
	    while (!cc && 
    	       !strcmp (pccl_rec.cl_co_no, comm_rec.tco_no) &&
	    	   !strcmp (pccl_rec.cl_br_no, comm_rec.test_no) &&
	           ((global_cal && pccl_rec.cl_hhrs_hash == 0L) ||
	            (!global_cal && pccl_rec.cl_hhrs_hash == curr_hhrs)))
	    {
			/*----------------------------------------
			| First record for calendar so calculate |
			| end time and read next record so that  |
			| downtime may be calculated.            |
			----------------------------------------*/
			if (abs_end_time == 0L)
			{
			    abs_end_time = DATE_TIME (pccl_rec.cl_act_date, 
			                              pccl_rec.cl_act_time) + 
			                   pccl_rec.cl_duration;

                cc = find_rec (pccl, &pccl_rec, NEXT, "r");
			    continue;
			}

	    	/*------------------------------------------
	    	| NB. There are no vertical links or job   |
	    	| details associated with calendar nodes.  |
		    	------------------------------------------*/
			abs_st_time = DATE_TIME (pccl_rec.cl_act_date, 
			                         pccl_rec.cl_act_time);
			down_time = abs_st_time - abs_end_time;

			if (down_time != 0L)
			{
			    curr_stp = stp_node (ALLC_ND, STP_NULL);
			    curr_stp->rsl_ptr      = lcl_rsl;
			    curr_stp->st_date_time = abs_end_time;
			    curr_stp->duration     = down_time;
			    curr_stp->run          = down_time;
			    curr_stp->status       = 'A';

			    /*-------------------------
			    | Find step position and  |
			    | set up horizontal links |
			    -------------------------*/
			    pos_found = FALSE;
			    lcl_stp = lcl_rsl->fst_stp;
			    while (lcl_stp != STP_NULL)
			    {
					if (curr_stp->st_date_time < lcl_stp->st_date_time)
					{
					    if (lcl_rsl->fst_stp == lcl_stp)
					    {
							/*-------------
							| Head insert |
							-------------*/
							curr_stp->nxt_job         = lcl_rsl->fst_stp;
							lcl_rsl->fst_stp->prv_job = curr_stp;
							lcl_rsl->fst_stp          = curr_stp;
					    }
					    else
					    {
							/*---------------
							| Middle insert |
							---------------*/
							curr_stp->nxt_job         = lcl_stp;
							curr_stp->prv_job         = lcl_stp->prv_job;
							lcl_stp->prv_job->nxt_job = curr_stp;
							lcl_stp->prv_job          = curr_stp;
					    }

					    pos_found = TRUE;
					    break;
					}
			
				lcl_stp = lcl_stp->nxt_job;
			    }

			    if (!pos_found)
			    {
					/*------------------------
					| Append after last step |
					------------------------*/
					if (lcl_rsl->fst_stp == STP_NULL)
					{	
					    lcl_rsl->fst_stp = curr_stp;
					    lcl_rsl->lst_stp = curr_stp;
					}
					else
					{
					    curr_stp->prv_job         = lcl_rsl->lst_stp;
					    lcl_rsl->lst_stp->nxt_job = curr_stp;
					    lcl_rsl->lst_stp          = curr_stp;
					}
			    }
			}
			abs_end_time = abs_st_time + pccl_rec.cl_duration;
			
			cc = find_rec (pccl, &pccl_rec, NEXT, "r");
        }

	    lcl_rsl = lcl_rsl->nxt_rsl;
	}

	return;
}

/*------------------
| Load Model Names |
------------------*/
void
ld_mdl_names (
 int    loadall)
{
	char	modify_str[25];
	struct 	MDL_SEL 	*lcl_ptr;

	if (loadall)
	{
		/*---------------------
		| Free list of models |
		---------------------*/
		while (mdl_head != MDL_NULL)
		{
			lcl_ptr = mdl_head;
			mdl_head = mdl_head->next;
			mdl_node (FREE_ND, lcl_ptr);
		}
		mdl_head = MDL_NULL;
	
		/*-------------------
		| Read psqm records |
		-------------------*/
		strcpy (psqm_rec.qm_co_no, comm_rec.tco_no);
		sprintf (psqm_rec.qm_model_name, "%-10.10s", " ");

		cc = find_rec (psqm, &psqm_rec, GTEQ, "r");
		while (!cc && 
		       !strcmp (psqm_rec.qm_co_no, comm_rec.tco_no))
		{
			lcl_ptr = mdl_node(ALLC_ND, MDL_NULL);
	
			if (psqm_rec.qm_dt_modify == 0L)
			{
				strcpy (modify_str, "Not Modified");
			}
			else
			{
				sprintf (modify_str, 
				        "%-10.10s %-5.5s",
				        DateToString (psqm_rec.qm_dt_modify),
				        ttoa (psqm_rec.qm_tm_modify, "NN:NN"));
			}

			sprintf (lcl_ptr->save_name, 
			 		 "%-10.10s %-10.10s %-5.5s - %-16.16s", 
			 		 psqm_rec.qm_model_name,
			 		 DateToString (psqm_rec.qm_dt_create),
			 		 ttoa (psqm_rec.qm_tm_create, "NN:NN"),
			 		 modify_str);
			lcl_ptr->hhqm_hash = psqm_rec.qm_hhqm_hash;
	
			if (mdl_head == MDL_NULL)
			{
				mdl_head      = lcl_ptr;
				lcl_ptr->next = MDL_NULL;
				lcl_ptr->prev = MDL_NULL;
				mdl_curr      = lcl_ptr;
			}
			else
			{
				
				lcl_ptr->next  = MDL_NULL;
				lcl_ptr->prev  = mdl_curr;
				mdl_curr->next = lcl_ptr;
				mdl_curr       = lcl_ptr;
			}
	
			cc = find_rec (psqm, &psqm_rec, NEXT, "r");
		}
	}

	win_display ("MODEL SELECTION", 47, (struct SEL_STR *) mdl_head, 0, 3, 132, 13);
	if (mdl_head == MDL_NULL)
	{
		rv_pr("\007 T H E R E   A R E   N O   M O D E L S   T O   C H O O S E ", 38, 10, 1);	
		main_menu[DEL_MDL].flag = SHOW;
		main_menu[CPY_MDL].flag = SHOW;
		main_menu[MOD_MDL].flag = SHOW;
		main_menu[UPD_SCH].flag = SHOW;
	}

	run_menu(main_menu, "", input_row);

	return;
}

/*------------------
| Create New Model |
------------------*/
int
create_model (
 void)
{
	int    i;
	int    jobs_fnd;
	char   get_buf[200];

	/*-----------------------
	| Get name of new model |
	-----------------------*/
	if (!get_mdl_name ())
	{
		return (FALSE);
	}

	jobs_in_model = 0;

	jobs_fnd = tag_new_job (4, 2, 9, FALSE);
	if (jobs_fnd > 0)
	{
		/*------------------------
		| Allow user to tag jobs |
		------------------------*/
		tab_scan("new_mdl");
		if (restart)
		{
			tab_close ("new_mdl", TRUE);
			restart = FALSE;
			return (FALSE);
		}

		/*--------------------
		| Create psqm record |
		--------------------*/
		add_psqm ();

		/*----------------------------
		| Add all live jobs to model |
		----------------------------*/
		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		strcpy (pcwo_rec.wo_order_status, "R");
		pcwo_rec.wo_reqd_date = 0L;
		pcwo_rec.wo_priority = 0;

		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
		while (!cc &&
		       !strcmp (pcwo_rec.wo_co_no, comm_rec.tco_no) &&
		       !strcmp (pcwo_rec.wo_br_no, comm_rec.test_no) &&
		       !strcmp (pcwo_rec.wo_wh_no, comm_rec.tcc_no) &&
		       !strcmp (pcwo_rec.wo_order_status, "R"))
		{
			add_pstq (pcwo_rec.wo_hhwo_hash, 'L');
			jobs_in_model++;
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}

		/*-----------------
	 	| Add tagged jobs |
		-----------------*/
	 	for (i = 0; i < jobs_fnd; i++)
    	{
			tab_get("new_mdl", get_buf, EQUAL, i);
			if (tagged(get_buf))
			{
	    		/*------------------
	    		| Include in model |
	    		| (add pstq rec) |
	    		------------------*/
				add_pstq (atol (get_buf + 116), 'T');
				jobs_in_model++;
			}
    	}
	}
	else
	{
    	putchar (BELL);
    	fflush (stdout);
    	tab_add ("new_mdl", "%-25.25s NO JOBS IN BIN ", " ");
    	tab_display ("new_mdl", TRUE);
    	crsr_off ();
    	fflush (stdout);
    	sleep (sleepTime);

		/*--------------------
		| Create psqm record |
		--------------------*/
		add_psqm ();

		/*----------------------------
		| add all live jobs to model |
		----------------------------*/
		strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
		strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
		strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
		strcpy (pcwo_rec.wo_order_status, "R");
		pcwo_rec.wo_reqd_date = 0L;
		pcwo_rec.wo_priority = 0;

		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
		while (!cc &&
       	       !strcmp (pcwo_rec.wo_co_no, comm_rec.tco_no) &&
       	       !strcmp (pcwo_rec.wo_br_no, comm_rec.test_no) &&
			   !strcmp (pcwo_rec.wo_wh_no, comm_rec.tcc_no) &&
       	       !strcmp (pcwo_rec.wo_order_status, "R"))
		{
			add_pstq (pcwo_rec.wo_hhwo_hash, 'L');
			jobs_in_model++;
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
	}

	tab_close ("new_mdl", TRUE);

	/*---------------------------
	| Can't create blank models |
	---------------------------*/
	if (jobs_in_model == 0)
	{
		print_mess (ML (empty_mdl));
		sleep (sleepTime);
		clear_mess ();

		cc = abc_delete (psqm);
		if (cc)
		{
			file_err (cc, psqm, "DBDELETE");
		}

		return (FALSE);
	}

	return (TRUE);
}

/*-----------------------
| Get name of new model |
-----------------------*/
int
get_mdl_name (
 void)
{
	char	tmp_mdl[11];

	do
	{
		print_at (2, 1, ML (mlPsMess003));
		crsr_on ();
		getalpha(26, 2, "UUUUUUUUUU", tmp_mdl);
		if (last_char == FN1)
		{
			blank_at (2, 0, 80);
			return (FALSE);
		}

		sprintf (local_rec.new_mdl, "%-10.10s", tmp_mdl);
	} while (strlen (tmp_mdl) == 0 || mdl_name_used ());

	blank_at (2, 0, 80);

	return (TRUE);
}
/*-----------------------------------------
| Check that model does NOT already exist |
-----------------------------------------*/
int
mdl_name_used (
 void)
{
	struct MDL_SEL *lcl_ptr;

	lcl_ptr = mdl_head;
	while (lcl_ptr != MDL_NULL)
	{
		if (!strncmp (lcl_ptr->save_name, local_rec.new_mdl, 10))
		{
			print_mess ("\007 Model Already Exists ");
			sleep (sleepTime);
			clear_mess ();
			return (TRUE);
		}

		lcl_ptr = lcl_ptr->next;
	}

	return (FALSE);
}

/*----------------------------------
| Open a tabdisp table and fill it |
| with jobs currently in the 'Bin' |
----------------------------------*/
int
tag_new_job (
 int    trow, 
 int    tcol, 
 int    tdpth, 
 int    selective)
{
	struct JOB_LIST *job_curr;
	int    jobs_fnd;

	/*--------------------
	| Open tabdisp table |
	--------------------*/
	tab_open ("new_mdl", new_mkeys, trow, tcol, tdpth, FALSE);
	tab_add ("new_mdl", 
		     "# %-7.7s | %-10.10s | %-16.16s | %-14.14s | %-8.8s | %-40.40s ", 
		     "JOB NO.",
		     "BATCH  NO.",
		     "  ITEM  NUMBER  ",
		     "   QUANTITY   ",
		     "DATE REQ",
	         "            ITEM DESCRIPTION            ");
	jobs_fnd = 0;

	/*---------------------------------
	| Allow selection of range of     |
	| unscheduled jobs from the 'bin' |
	---------------------------------*/
	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, comm_rec.test_no);
	strcpy (pcwo_rec.wo_wh_no, comm_rec.tcc_no);
	strcpy (pcwo_rec.wo_order_status, "A");
	pcwo_rec.wo_reqd_date = 0L;
	pcwo_rec.wo_priority = 0;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (pcwo_rec.wo_co_no, comm_rec.tco_no) &&
	       !strcmp (pcwo_rec.wo_br_no, comm_rec.test_no) &&
		   !strcmp (pcwo_rec.wo_wh_no, comm_rec.tcc_no) &&
	       !strcmp (pcwo_rec.wo_order_status, "A"))
	{
		if (selective)
		{
			job_curr = job_head;
			while (job_curr != JOB_NULL)
			{
				if (job_curr->hhwo_hash == pcwo_rec.wo_hhwo_hash)
				{
					break;
				}
				else
				{
					job_curr = job_curr->nxt_job;
				}
			}
			if (job_curr != JOB_NULL)
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
		}
		cc = find_hash (inmr,&inmr_rec,COMPARISON, "r",pcwo_rec.wo_hhbr_hash);
		if (cc)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
	
	    jobs_fnd++;
	    tab_add ("new_mdl", 
		    	" %-7.7s | %10.10s | %-16.16s | %14.6f |%-10.10s| %-40.40s           %10ld",
		    	pcwo_rec.wo_order_no,
				pcwo_rec.wo_batch_no,
		    	inmr_rec.mr_item_no,
				pcwo_rec.wo_prod_qty,
				DateToString (pcwo_rec.wo_reqd_date),
		    	inmr_rec.mr_description,
		    	pcwo_rec.wo_hhwo_hash);

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	return (jobs_fnd);
}

/*-------------------------------
| Add and read back psqm record |
-------------------------------*/
void
add_psqm (
 void)
{
	strcpy (psqm_rec.qm_co_no, comm_rec.tco_no);
	sprintf (psqm_rec.qm_model_name, "%-10.10s", local_rec.new_mdl);
	psqm_rec.qm_dt_create = local_rec.lsystemDate;

	strcpy (err_str, TimeHHMMSS());
	psqm_rec.qm_tm_create = atot (err_str);
	sprintf (psqm_rec.qm_op_create, "%-14.14s", local_rec.logname);
	psqm_rec.qm_dt_modify = 0L;
	psqm_rec.qm_tm_modify = 0L;
	strcpy (psqm_rec.qm_stat_flag, "0");

	cc = abc_add (psqm, &psqm_rec);
	if (cc)
	{
		file_err (cc, psqm, "DBADD");
	}

	/*---------------------
	| Read back with lock |
	---------------------*/
	strcpy (psqm_rec.qm_co_no, comm_rec.tco_no);
	sprintf (psqm_rec.qm_model_name, "%-10.10s", local_rec.new_mdl);

	cc = find_rec (psqm, &psqm_rec, COMPARISON, "u");
	if (cc)
	{
		file_err (cc, psqm, "DBFIND");
	}

	return;
}

/*----------------------
| Add all pstq records |
| for a Works Order.   |
----------------------*/
void
add_pstq (
 long   hhwo_hash, 
 char   status)
{
	int    pos_found;
	int    upd_pstq;
	int    last_seq, delta_seq;
	long   curr_hhrs;
	long   earliest, best_st_time, curr_dt_tm; 
	long   last_end_time, last_st_time;
	struct JOB_LIST    *lcl_job = (struct JOB_LIST *) 0;
	struct STP_LIST    *curr_stp,
	                   *lcl_stp,
	                   *chk_stp;
	struct RSL_LIST    *lcl_rsl,
	                   *first_rsl,
	                   *best_rsl;

	upd_pstq = FALSE;
	last_seq   = 0;
	last_st_time = 0L;
	last_end_time = 0L;
	/*-----------------------------------------
	| Add a pstq record for every pcrq record |
	| associated with the hhwo_hash passed.   |
	-----------------------------------------*/
	pcrq_rec.rq_hhwo_hash = hhwo_hash;
	pcrq_rec.rq_seq_no    = 0;
	pcrq_rec.rq_line_no   = 0;

	cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
	while (!cc && 
	       pcrq_rec.rq_hhwo_hash == hhwo_hash)
	{
	    curr_hhrs = pcrq_rec.rq_hhrs_hash;
	    delta_seq = FALSE;
	    if (pcrq_rec.rq_seq_no != last_seq)
	    {
	       delta_seq = TRUE;
        }

	    /*-------------------------------
	    | Allocate memory for the step. |
	    -------------------------------*/
	    curr_stp = stp_node(ALLC_ND, STP_NULL);

	    /*-------------------------------------
	    | Take note of the current date/time. |
	    -------------------------------------*/
		strcpy (err_str, TimeHHMMSS());
		curr_dt_tm = DATE_TIME(local_rec.lsystemDate, atot(err_str));

	    /*-----------------------------
	    | Get pointer to job details. |
	    -----------------------------*/
	    if (last_seq == 0)
	    {
	    	lcl_job = fnd_job (hhwo_hash, curr_stp);
            best_st_time = curr_dt_tm;
	    }
	    else
	    {
			if (delta_seq)
			{
			    best_st_time = last_end_time;
			}
			else
			{
			    best_st_time = last_st_time;
			}
	    }

	    /*-------------------------------------
	    | Find 1st rsl node for this resource |
	    -------------------------------------*/
	    first_rsl = rsl_head;
	    while (first_rsl != RSL_NULL)
	    {
			if (first_rsl->rgrs_ptr->rs_hhrs_hash == curr_hhrs)
			{
			    break;
			}
			first_rsl = first_rsl->nxt_rsl;
	    }
	    /*------------------------------------
	    | If there are no rsl nodes for this |
	    | resource then we've got problems.  |
	    ------------------------------------*/
	    if (first_rsl == RSL_NULL)
	    {
            sys_err ("Error in add_pstq (),can't find resource node",1,PNAME);
        }

	    /*------------------------------------
	    | Set pstq data EXCEPT for date/time |
	    ------------------------------------*/
	    pstq_rec.tq_hhqm_hash = psqm_rec.qm_hhqm_hash;
	    pstq_rec.tq_hhrs_hash = pcrq_rec.rq_hhrs_hash;
	    pstq_rec.tq_hhwo_hash = hhwo_hash;
	    pstq_rec.tq_seq_no    = pcrq_rec.rq_seq_no;
	    pstq_rec.tq_line_no   = pcrq_rec.rq_line_no;
	    pstq_rec.tq_st_date   = 0L;
	    pstq_rec.tq_st_time   = 0L;
	    pstq_rec.tq_setup     = 0L;
	    pstq_rec.tq_run       = 0L;
	    pstq_rec.tq_clean     = 0L;
	    strcpy (pstq_rec.tq_stat_flag, "0");

	    if (LIVE_STEP)
	    {
            add_live_stp(curr_stp, lcl_job, first_rsl);
        }          
	    else
	    {
			/*--------------------------------------
			| These are tagged jobs from the 'Bin' |
			| so calculate start date/times where  |
			| job can be appended and append.      |
			--------------------------------------*/
			curr_stp->job_dtl  = lcl_job;
			curr_stp->seq_no   = pcrq_rec.rq_seq_no;
			curr_stp->line_no  = pcrq_rec.rq_line_no;
			curr_stp->setup    = pcrq_rec.rq_est_setup;
			curr_stp->run      = pcrq_rec.rq_est_run;
			curr_stp->clean    = pcrq_rec.rq_est_clean;
			curr_stp->duration = curr_stp->setup + curr_stp->run + curr_stp->clean;
			curr_stp->status   = 'E';

			/*------------------------
			| Find the best rsl node |
			| to put this step on.   |
			------------------------*/
			lcl_rsl  = first_rsl;
			best_rsl = lcl_rsl;
			earliest = 0L;
			while (lcl_rsl != RSL_NULL)
			{
			    /*-------------------------------------
			    | No jobs on rsl so job must fit here |
			    -------------------------------------*/
			    if (lcl_rsl->fst_job == STP_NULL)
			    {
					earliest = curr_dt_tm;
					best_rsl = lcl_rsl;
					break;
			    }

			    /*--------------------------------------
			    | This is the first place that it fits |
			    | so put it here.                      |
			    --------------------------------------*/
			    if (best_st_time > lcl_rsl->lst_dt_tm)
			    {
					earliest = lcl_rsl->lst_dt_tm;
					best_rsl = lcl_rsl;
					break;
			    }

			    /*---------------------------------------------------
			    | This rsl node has the best start date/time so far |
			    ---------------------------------------------------*/
			    if (earliest == 0L || lcl_rsl->lst_dt_tm < earliest)
			    {
					best_rsl = lcl_rsl;
					earliest = lcl_rsl->lst_dt_tm;
			    }

                lcl_rsl = lcl_rsl->nxt_rsl;
		    }

			/*------------------------------
			| Job should start at earliest |
			| possible date/time.          |
			------------------------------*/
			if (earliest > best_st_time)
			{
			    best_st_time = earliest;
            }
            
			curr_stp->st_date_time = best_st_time;

			/*-------------------------
			| Insert into linked list |
			-------------------------*/
			if (best_rsl->fst_stp == STP_NULL)
			{
			    /*-------------
			    | Head insert |
			    -------------*/
			    best_rsl->fst_stp = curr_stp;
			    best_rsl->lst_stp = curr_stp;
			    best_rsl->fst_job = curr_stp;
			    best_rsl->lst_job = curr_stp;
			}
			else
			{
			    /*--------------------------
			    | Find position and insert |
			    --------------------------*/
			    pos_found = FALSE;
			    lcl_stp = best_rsl->fst_stp;
			    while (lcl_stp != STP_NULL)
			    {
    				if (curr_stp->st_date_time < lcl_stp->st_date_time)
					{
					    if (lcl_stp == best_rsl->fst_stp)
					    {
							/*-------------
							| Head insert |
							-------------*/
							curr_stp->nxt_job = lcl_stp;
							lcl_stp->prv_job  = curr_stp;
							best_rsl->fst_stp = curr_stp;
							best_rsl->fst_job = curr_stp;
							if (best_rsl->lst_job == STP_NULL)
							{
							    best_rsl->lst_job = curr_stp;
		                    }
	                    }
					    else
                        {
							/*---------------
							| Middle insert |
							---------------*/
							curr_stp->nxt_job         = lcl_stp;
							curr_stp->prv_job         = lcl_stp->prv_job;
							lcl_stp->prv_job->nxt_job = curr_stp;
							lcl_stp->prv_job          = curr_stp;

							if (best_rsl->fst_job == STP_NULL)
							{
							    best_rsl->fst_job = curr_stp;
							    best_rsl->lst_job = curr_stp;
							}
							else
							{
							    if (curr_stp->st_date_time < best_rsl->fst_job->st_date_time)
							    {
    								best_rsl->fst_job = curr_stp;
							    }

							    if (curr_stp->st_date_time > best_rsl->lst_job->st_date_time)
							    {
    								best_rsl->lst_job = curr_stp;
							    }
                            }
                        }

					    pos_found = TRUE;
					    break;
					}
		
				    lcl_stp = lcl_stp->nxt_job;
			    }

			    if (!pos_found)
			    {
					/*--------
					| Append |
					--------*/
					curr_stp->prv_job          = best_rsl->lst_stp;
					best_rsl->lst_stp->nxt_job = curr_stp;
					best_rsl->lst_stp          = curr_stp;
					best_rsl->lst_job          = curr_stp;

					if (best_rsl->fst_job == STP_NULL)
					{
					    best_rsl->fst_job = curr_stp;
                    }
			    }
			}

			if (delta_seq || 
			   (curr_stp->st_date_time + curr_stp->duration) > last_end_time)
			{
			    last_end_time = curr_stp->st_date_time + curr_stp->duration;
			}
			last_st_time  = curr_stp->st_date_time;

			/*-----------------------------------
			| Separate date and time components |
			| from calculated start date/time.  |
			-----------------------------------*/
			pstq_rec.tq_st_date = curr_stp->st_date_time / (24L * 60L);
			pstq_rec.tq_st_time = curr_stp->st_date_time - (pstq_rec.tq_st_date * 24L * 60L);
	        best_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
			/*---------------------------------
			| Set pointer to resource record. |
			---------------------------------*/
			curr_stp->rsl_ptr = best_rsl;
        }

	    /*--------------------------------
	    | Set vertical pointers for node |
	    --------------------------------*/	
	    if (last_seq != 0)
	    {
			curr_stp->prv_res         = lcl_job->lst_stp;
			lcl_job->lst_stp->nxt_res = curr_stp;
			lcl_job->lst_stp          = curr_stp;

			/*--------------------------------------
			| Same sequence as last one so may     |
			| have to shuffle some of the previous |
			| steps in the same sequence.          |
			--------------------------------------*/
			if (status == 'T' && !delta_seq)
			{
			    chk_stp = curr_stp->prv_res;
			    while (chk_stp != STP_NULL)
			    {
					/*-----------------------------------
					| chk_stp is now at previous SEQ_NO |
					-----------------------------------*/
					if (chk_stp->seq_no != curr_stp->seq_no)
					{
					    break;
                    }

					/*--------------------------------------
					| Shuffle start date/time if necessary |
					--------------------------------------*/
					if (chk_stp->st_date_time < curr_stp->st_date_time)
					{
					    chk_stp->st_date_time = curr_stp->st_date_time;

					    /*--------------------
					    | Modify pstq record |
					    --------------------*/
					    upd_pstq = TRUE;

					    /*-----------------------------------
					    | Does this affect the last_dt_tm ? |
					    -----------------------------------*/
					    if ((chk_stp->st_date_time + chk_stp->duration) > last_end_time)
					    {
		    				last_end_time = chk_stp->st_date_time + chk_stp->duration;
					    }
					}
					chk_stp = chk_stp->prv_res;
			    }
			}
	    }

	    /*------------------------------------
	    | Keep track of last sequence number |
	    ------------------------------------*/
	    last_seq = pcrq_rec.rq_seq_no;

	    /*-----------------
	    | Add pstq record |
	    -----------------*/
	    pstq_rec.tq_setup = curr_stp->setup;
	    pstq_rec.tq_run   = curr_stp->run  ;
	    pstq_rec.tq_clean = curr_stp->clean;
	    strcpy (pstq_rec.tq_stat_flag, "0");

	    cc = abc_add (pstq, &pstq_rec);
	    if (cc)
	    {
            file_err (cc, pstq, "DBADD");
        }

	    cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
	}

	/*----------------------------------------------------
	| Start date/times were shuffled for concurrent      |
	| steps within same sequence so pstqs need updating. |
	----------------------------------------------------*/
	if (upd_pstq)
	{
	    shuffle_pstqs(hhwo_hash, lcl_job);
    }

	return;
}

/*--------------------------
| Adjust pstq records from |
| linked list details.     |
--------------------------*/
void
shuffle_pstqs (
 long   hhwo_hash, 
 struct JOB_LIST *lcl_job)
{
	struct STP_LIST *chk_stp;

	chk_stp = lcl_job->fst_stp;
	while (chk_stp != STP_NULL)
	{
		pstq_rec.tq_hhqm_hash = psqm_rec.qm_hhqm_hash;
		pstq_rec.tq_hhrs_hash = chk_stp->rsl_ptr->rgrs_ptr->rs_hhrs_hash;
		pstq_rec.tq_hhwo_hash = hhwo_hash;
		pstq_rec.tq_seq_no    = chk_stp->seq_no;
		pstq_rec.tq_line_no   = chk_stp->line_no;

		cc = find_rec (pstq3, &pstq_rec, COMPARISON, "u");
		if (cc)
		{
			file_err (cc, pstq, "DBFIND");
		}

		pstq_rec.tq_st_date = chk_stp->st_date_time / (24L * 60L);
		pstq_rec.tq_st_time = chk_stp->st_date_time - (pstq_rec.tq_st_date * 24L * 60L);
		    
		cc = abc_update(pstq3, &pstq_rec);
		if (cc)
		{
			file_err (cc, pstq3, "DBUPDATE");
		}

		chk_stp = chk_stp->nxt_res;
	}

	return;
}

/*-----------------------------------------------
| Live Jobs. Current pcwo record is the correct |
| one in the case of 'L'ive pcrq records.       |
-----------------------------------------------*/
int
add_live_stp (
 struct STP_LIST *lcl_stp, 
 struct JOB_LIST *lcl_job, 
 struct RSL_LIST *first_rsl)
{
	if (pcwo_rec.wo_rtg_seq > pstq_rec.tq_seq_no)
	{
	    /*--------------------------------------
	    | Use ACTUAL start date/time from pcrq |
	    --------------------------------------*/
	    pstq_rec.tq_st_date = pcrq_rec.rq_act_date;
	    pstq_rec.tq_st_time = pcrq_rec.rq_act_time;
	    lcl_stp->setup      = pcrq_rec.rq_act_setup;
	    lcl_stp->run        = pcrq_rec.rq_act_run;
	    lcl_stp->clean      = pcrq_rec.rq_act_clean;
	    lcl_stp->duration  = lcl_stp->setup + lcl_stp->run + lcl_stp->clean;
	    lcl_stp->status  = 'A';
	}
	else
	{
	    /*-----------------------------------------
	    | Use ESTIMATED start date/time from pcrq |
	    -----------------------------------------*/
	    pstq_rec.tq_st_date = pcrq_rec.rq_est_date;
	    pstq_rec.tq_st_time = pcrq_rec.rq_est_time;
	    lcl_stp->setup      = pcrq_rec.rq_est_setup;
	    lcl_stp->run        = pcrq_rec.rq_est_run;
	    lcl_stp->clean      = pcrq_rec.rq_est_clean;
	    lcl_stp->duration  = lcl_stp->setup + lcl_stp->run + lcl_stp->clean;
	    lcl_stp->status  = 'E';
	}

	/*---------------------------
	| Set data for current step |
	---------------------------*/	
	lcl_stp->job_dtl = lcl_job;
	lcl_stp->seq_no  = pstq_rec.tq_seq_no;
	lcl_stp->line_no = pstq_rec.tq_line_no;
	lcl_stp->st_date_time = DATE_TIME(pstq_rec.tq_st_date, pstq_rec.tq_st_time);

	/*----------------------------------
	| Set horizontal pointers for node |
	----------------------------------*/	
	fnd_horizontal (first_rsl, lcl_stp);

	return (EXIT_SUCCESS);
}

/*---------------
| Process Model |
---------------*/
int
proc_model (
 int    load_list)
{
	int    modified;
	struct RSL_LIST    *lcl_rsl;
	struct STP_LIST    *lcl_stp;

	/*-----------------
	| Load Model Data |
	-----------------*/
	if (load_list)
	{
		load_model ();
	}

	/*------------------------
	| Modify model on screen |
	------------------------*/
	modified = gantt ();

	/*--------------------------------
	| Update psqm modified date/time |
	| and pstq records for model.    |
	--------------------------------*/
	if (modified == FN16)
	{
	    /*-------------
	    | Update psqm |
	    -------------*/
	    psqm_rec.qm_dt_modify = local_rec.lsystemDate;
		strcpy (err_str, TimeHHMMSS());
		psqm_rec.qm_tm_modify = atot (err_str);
	    cc = abc_update(psqm, &psqm_rec);
	    if (cc)
	    {
            file_err (cc, psqm, "DBUPDATE");
		}

	    /*------------------------------
	    | Delete all pstq records      |
	    | for this model and recreate. |
	    ------------------------------*/
	    pstq_rec.tq_hhqm_hash = slct_hhqm;
	    pstq_rec.tq_hhrs_hash = 0L;
	    pstq_rec.tq_st_date   = 0L;
	    pstq_rec.tq_st_time   = 0L;

	    cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	    while (!cc && 
	           pstq_rec.tq_hhqm_hash == slct_hhqm)
	    {
			cc = abc_delete (pstq);
			if (cc)
			{
			    file_err (cc, pstq, "DBDELETE");
            }

	        pstq_rec.tq_hhqm_hash = slct_hhqm;
	        pstq_rec.tq_hhrs_hash = 0L;
	        pstq_rec.tq_st_date   = 0L;
	        pstq_rec.tq_st_time   = 0L;

	    	cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	    }

	    /*----------------------------------------
	    | Recreate pstq records from linked list |
	    ----------------------------------------*/
	    lcl_rsl = rsl_head;
	    while (lcl_rsl != RSL_NULL)
	    {
			if (lcl_rsl->fst_job != STP_NULL)
			{
			    lcl_stp = lcl_rsl->fst_job;
			    while (lcl_stp != STP_NULL)
			    {
					if (lcl_stp->job_dtl == JOB_NULL)
					{
					    lcl_stp = lcl_stp->nxt_job;
					    continue;
					}

					/*----------------------
					| Recreate pstq record |
					----------------------*/
                    pstq_rec.tq_hhqm_hash = psqm_rec.qm_hhqm_hash;
                    pstq_rec.tq_hhrs_hash = lcl_rsl->rgrs_ptr->rs_hhrs_hash;
                    pstq_rec.tq_hhwo_hash = lcl_stp->job_dtl->hhwo_hash;
                    pstq_rec.tq_seq_no    = lcl_stp->seq_no;
                    pstq_rec.tq_line_no   = lcl_stp->line_no;

			    	pstq_rec.tq_st_date = lcl_stp->st_date_time / (24L * 60L);
			    	pstq_rec.tq_st_time = lcl_stp->st_date_time - (pstq_rec.tq_st_date * 24L * 60L);
			    	pstq_rec.tq_setup   = lcl_stp->setup;
			    	pstq_rec.tq_run     = lcl_stp->run;
			    	pstq_rec.tq_clean   = lcl_stp->clean;
				    
			    	cc = abc_add (pstq3, &pstq_rec);
			    	if (cc)
			    	{
					    file_err (cc, pstq3, "DBUPDATE");
                    }

					lcl_stp = lcl_stp->nxt_job;
			    }
			}

			lcl_rsl = lcl_rsl->nxt_rsl;
	    }
	}
	else
	{
	    abc_unlock(psqm);
    }

	/*---------------
	| Redraw screen |
	---------------*/
	heading (0);

	return ((modified == FN16));
}

/*------------
| Copy Model |
------------*/
int
copy_model (
 void)
{
	/*-----------------------
	| Get name of new model |
	-----------------------*/
	if (!get_mdl_name ())
	{
		return (FALSE);
	}

	/*-----------------
	| Create new psqm |
	-----------------*/
	add_psqm ();
	
	/*-------------------
	| Copy pstq records |
	-------------------*/
	pstq_rec.tq_hhqm_hash = slct_hhqm;
	pstq_rec.tq_hhrs_hash = 0L;
	pstq_rec.tq_st_date   = 0L;
	pstq_rec.tq_st_time   = 0L;

	cc = find_rec (pstq, &pstq_rec, GTEQ, "r");
	while (!cc && 
	       pstq_rec.tq_hhqm_hash == slct_hhqm)
	{
		/*-----------------------------------------------
		| Set hhqm hash to new hhqm hash and add record |
		-----------------------------------------------*/
		pstq_rec.tq_hhqm_hash = psqm_rec.qm_hhqm_hash;

		cc = abc_add (pstq2, &pstq_rec);
		if (cc)
		{
			file_err (cc, pstq2, "DBADD");
		}

		cc = find_rec (pstq, &pstq_rec, NEXT, "r");
	}
	
	return (TRUE);
}

/*--------------
| Delete Model |
--------------*/
int
delete_model (
 void)
{
	int	i;

	crsr_on ();
	i = prmptmsg(ML (del_msg), "YyNn", 1, 2);
	crsr_off ();
	blank_at (2, 0, 60);

	if (i == 'N' || i == 'n')
	{
		return (FALSE);
	}

	/*--------------------
	| Delete psqm record |
	--------------------*/
	cc = find_hash (psqm2, &psqm_rec, COMPARISON, "u", slct_hhqm);
	if (cc)
	{
		file_err (cc, psqm2, "DBFIND");
	}

	cc = abc_delete (psqm2);
	if (cc)
	{
		file_err (cc, psqm2, "DBDELETE");
	}

	/*---------------------
	| Delete pstq records |
	---------------------*/
	pstq_rec.tq_hhqm_hash = slct_hhqm;
	pstq_rec.tq_hhrs_hash = 0L;
	pstq_rec.tq_st_date   = 0L;
	pstq_rec.tq_st_time   = 0L;

	cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	while (!cc && 
	       pstq_rec.tq_hhqm_hash == slct_hhqm)
	{
		cc = abc_delete (pstq);
		if (cc)
		{
			file_err (cc, pstq, "DBDELETE");
		}

		pstq_rec.tq_hhqm_hash = slct_hhqm;
		pstq_rec.tq_hhrs_hash = 0L;
		pstq_rec.tq_st_date   = 0L;
		pstq_rec.tq_st_time   = 0L;
		cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	}

	return (TRUE);
}

/*----------------------------------------------
| Load model data from file into linked lists  |
----------------------------------------------*/
int
load_model (
 void)
{
	long   curr_hhrs;
	struct RSL_LIST    *curr_rsl;

	struct STP_LIST    *lcl_stp;

	struct JOB_LIST    *lcl_job;

	clear_steps ();

	/*--------------------------------------------
	| For all resources (not all resource nodes) |
	| load pstq records and store in linked list |
	--------------------------------------------*/
	curr_hhrs = 0L;
	curr_rsl = rsl_head;
	while (curr_rsl != RSL_NULL)
	{
	    if (curr_rsl->rgrs_ptr->rs_hhrs_hash == curr_hhrs)
	    {
			curr_rsl = curr_rsl->nxt_rsl;
			continue;
	    }
	    curr_hhrs = curr_rsl->rgrs_ptr->rs_hhrs_hash;

	    /*-------------------
	    | Load pstq records |
	    -------------------*/	
	    pstq_rec.tq_hhqm_hash = slct_hhqm;
	    pstq_rec.tq_hhrs_hash = curr_rsl->rgrs_ptr->rs_hhrs_hash;
	    pstq_rec.tq_st_date = 0L;
	    pstq_rec.tq_st_time = 0L;

	    cc = find_rec (pstq, &pstq_rec, GTEQ, "r");
	    while (!cc && 
	           pstq_rec.tq_hhqm_hash == slct_hhqm &&
	           pstq_rec.tq_hhrs_hash == curr_rsl->rgrs_ptr->rs_hhrs_hash)
	    {
			/*-------------------------------
			| Allocate memory for the step. |
			-------------------------------*/
			lcl_stp = stp_node(ALLC_ND, STP_NULL);

			/*-----------------------------
			| Get pointer to job details. |
			-----------------------------*/
			lcl_job = fnd_job (pstq_rec.tq_hhwo_hash, lcl_stp);
			lcl_stp->job_dtl = lcl_job;
				
			/*------------------------------------
			| Lookup pcrq record to get duration |
			------------------------------------*/
			pcrq_rec.rq_hhwo_hash = pstq_rec.tq_hhwo_hash;
			pcrq_rec.rq_seq_no    = pstq_rec.tq_seq_no;
			pcrq_rec.rq_line_no   = pstq_rec.tq_line_no;

			cc = find_rec (pcrq, &pcrq_rec, COMPARISON, "r");
			if (cc)
			{
				/*-------------------------------
				| Don't add this step into list |
				-------------------------------*/
				lcl_stp = stp_node(FREE_ND, lcl_stp);
                cc = find_rec (pstq, &pstq_rec, NEXT, "r");
				continue;
			}
		
			/*----------------------------
			| Get times for lcl_stp node |
			----------------------------*/
			if (lcl_job->rtg_seq > pstq_rec.tq_seq_no)
			{
			    /*----------------------------
			    | Use ACTUAL TIMES from pcrq |
			    ----------------------------*/
			    lcl_stp->st_date_time = DATE_TIME(pstq_rec.tq_st_date, 
                                                  pstq_rec.tq_st_time);

			    lcl_stp->setup    = pcrq_rec.rq_act_setup;
			    lcl_stp->run      = pcrq_rec.rq_act_run;
			    lcl_stp->clean    = pcrq_rec.rq_act_clean;

			    lcl_stp->duration = lcl_stp->setup +
			                        lcl_stp->run + 
			                        lcl_stp->clean;

			    lcl_stp->status  = 'A';
			}
			else
			{
			    /*-------------------------------
			    | Use ESTIMATED TIMES from pcrq |
			    -------------------------------*/
			    lcl_stp->st_date_time = DATE_TIME(pstq_rec.tq_st_date, 
                                                  pstq_rec.tq_st_time);
			    lcl_stp->setup    = pstq_rec.tq_setup;
			    lcl_stp->run      = pstq_rec.tq_run;
			    lcl_stp->clean    = pstq_rec.tq_clean;

			    lcl_stp->duration = lcl_stp->setup +
                                    lcl_stp->run +            
			                        lcl_stp->clean;

			    lcl_stp->status  = 'E';
			}

			/*---------------------
			| Set up data in node |
			---------------------*/
			lcl_stp->seq_no  = pstq_rec.tq_seq_no;
			lcl_stp->line_no = pstq_rec.tq_line_no;

			/*----------------------------------
			| Set horizontal pointers for node |
			----------------------------------*/	
			fnd_horizontal (curr_rsl, lcl_stp);

			/*--------------------------------
			| Set vertical pointers for node |
			--------------------------------*/	
			fnd_vertical (lcl_stp);

	    	cc = find_rec (pstq, &pstq_rec, NEXT, "r");
	    }

	    curr_rsl = curr_rsl->nxt_rsl;
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------
| This routine updates the chosen model |
| to be the production schedule used.   |
---------------------------------------*/
int
upd_sched (
 void)
{
	int    mdl_err;
	struct STP_LIST *lcl_stp;
	struct RSL_LIST *lcl_rsl;
	struct JOB_LIST *lcl_job;

	/*-----------------------------
	| Load model into linked list |
	-----------------------------*/
	load_model ();

	/*----------------------------
	| Check if its OK to update  |
	| schedule from chosen model |
	----------------------------*/
	mdl_err = valid_mdl();
	if (mdl_err & OVERLAP)
	{
		print_mess (ML (overlap));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	/*----------------------
	| Update job (s) status |
	----------------------*/
	lcl_job = job_head;
	while (lcl_job != JOB_NULL)
	{
		cc = find_hash (pcwo2, &pcwo_rec, COMPARISON, "u", lcl_job->hhwo_hash);
		if (cc)
		{
			file_err (cc, pcwo2, "DBFIND");
		}

		strcpy (pcwo_rec.wo_order_status, "R");

		cc = abc_update(pcwo2, &pcwo_rec);
		if (cc)
		{
			file_err (cc, pcwo2, "DBUPDATE");
		}

		lcl_job = lcl_job->nxt_job;
	}

	/*-----------------
	| Update schedule |
	-----------------*/
	lcl_rsl = rsl_head;
	while (lcl_rsl != RSL_NULL)
	{
		lcl_stp = lcl_rsl->fst_job;
		while (lcl_stp != STP_NULL)
		{
			cc = find_hash (pcwo2, &pcwo_rec, COMPARISON, "r", lcl_stp->job_dtl->hhwo_hash);
			if (cc)
			{
				lcl_stp = lcl_stp->nxt_job;
				continue;
			}

			/*------------------
			| Find pcrq record |
			------------------*/
			pcrq_rec.rq_hhwo_hash = lcl_stp->job_dtl->hhwo_hash;
			pcrq_rec.rq_seq_no    = lcl_stp->seq_no;
			pcrq_rec.rq_line_no   = lcl_stp->line_no;

			cc = find_rec (pcrq, &pcrq_rec, COMPARISON, "u");
			if (cc)
			{
				lcl_stp = lcl_stp->nxt_job;
				continue;
			}
			
			pcrq_rec.rq_est_date  = lcl_stp->st_date_time / (24L * 60L);
			pcrq_rec.rq_est_time  = lcl_stp->st_date_time - (pcrq_rec.rq_est_date * 24L * 60L);
			pcrq_rec.rq_est_setup = lcl_stp->setup;
			pcrq_rec.rq_est_run   = lcl_stp->run;
			pcrq_rec.rq_est_clean = lcl_stp->clean;

			/*------------------------------------------
			| Check routing sequence and set stat_flag |
			------------------------------------------*/
			if (pcwo_rec.wo_rtg_seq > lcl_stp->seq_no)
			{
				strcpy (pcrq_rec.rq_stat_flag, "A");
			}
			else
			{
				strcpy (pcrq_rec.rq_stat_flag, "E");
			}
			strcpy (pcrq_rec.rq_firm_sched, "Y");

			cc = abc_update(pcrq, &pcrq_rec);
			if (cc)
			{
				file_err (cc, pcrq, "DBUPDATE");
			}

			lcl_stp = lcl_stp->nxt_job;
		}

		lcl_rsl = lcl_rsl->nxt_rsl;
	}

	/*---------------
	| Delete models |
	---------------*/
	cc = find_hash (psqm2, &psqm_rec, GTEQ, "u", 0L);
	while (!cc)
	{
		cc = abc_delete (psqm2);
		if (cc)
		{
			file_err (cc, psqm2, "DBDELETE");
		}

		cc = find_hash (psqm2, &psqm_rec, GTEQ, "u", 0L);
	}

	pstq_rec.tq_hhqm_hash = 0L;
	pstq_rec.tq_hhrs_hash = 0L;
	pstq_rec.tq_st_date = 0L;
	pstq_rec.tq_st_time = 0L;

	cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	while (!cc)
	{
		cc = abc_delete (pstq);
		if (cc)
		{
			file_err (cc, pstq, "DBDELETE");
		}

		pstq_rec.tq_hhqm_hash = 0L;
		pstq_rec.tq_hhrs_hash = 0L;
		pstq_rec.tq_st_date = 0L;
		pstq_rec.tq_st_time = 0L;
		cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	}

	return (TRUE);
}

/*-----------------------------
| Get pointer to job details. |
-----------------------------*/
struct JOB_LIST *
fnd_job (
 long   hhwo_hash, 
 struct STP_LIST *curr_stp)
{
	struct JOB_LIST *lcl_job;

	lcl_job = job_head;
	while (lcl_job != JOB_NULL)
	{
		if (lcl_job->hhwo_hash == hhwo_hash)
		{
			return (lcl_job);
		}

		lcl_job = lcl_job->nxt_job;
	}

	/*------------------
	| Find pcwo record |
	------------------*/
	cc = find_hash (pcwo2,&pcwo_rec,COMPARISON,"r",hhwo_hash);
	if (cc)
	{
		file_err (cc, pcwo, "DBFIND");
	}

	/*------------------
	| Find inmr record |
	------------------*/
	cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", pcwo_rec.wo_hhbr_hash);
	if (cc)
	{
		file_err (cc, inmr, "DBFIND");
	}
	       
	/*--------------------------------------
	| Allocate memory for the job details. |
	--------------------------------------*/
	lcl_job = job_node(ALLC_ND, JOB_NULL);

	/*---------------------
	| Set up data in node |
	---------------------*/
	lcl_job->hhwo_hash   = hhwo_hash;
	lcl_job->rtg_seq     = pcwo_rec.wo_rtg_seq;
	lcl_job->hhbr_hash   = pcwo_rec.wo_hhbr_hash;
	lcl_job->prod_qty    = pcwo_rec.wo_prod_qty;
	lcl_job->priority    = pcwo_rec.wo_priority;
	lcl_job->bom_alt     = pcwo_rec.wo_bom_alt;
	lcl_job->rtg_alt     = pcwo_rec.wo_rtg_alt;
	lcl_job->create_date = pcwo_rec.wo_create_date;
	lcl_job->reqd_date   = pcwo_rec.wo_reqd_date;
	lcl_job->is_hilite   = FALSE;
	sprintf (lcl_job->order_no,    "%-7.7s",   pcwo_rec.wo_order_no);
	sprintf (lcl_job->item_no,     "%-16.16s", inmr_rec.mr_item_no);
	sprintf (lcl_job->description, "%-40.40s", inmr_rec.mr_description);

	/*-----------------------
	| Set pointers for node |
	-----------------------*/	
	lcl_job->fst_stp = curr_stp;
	lcl_job->lst_stp = curr_stp;
	lcl_job->nxt_job = JOB_NULL;
	if (job_head == JOB_NULL)
	{
		job_head = lcl_job;
		lcl_job->prv_job = JOB_NULL;
	}
	else
	{
		lcl_job->prv_job  = job_tail;
		job_tail->nxt_job = lcl_job;
	}	
	job_tail = lcl_job;

	return (lcl_job);
}

/*-------------------------------------
| Find horizontal position and insert |
-------------------------------------*/
void
fnd_horizontal (
 struct RSL_LIST *curr_rsl, 
 struct STP_LIST *curr_stp)
{
	int    pos_found;
	long   curr_hhrs;
	long   curr_start, 
	       curr_end, 
	       prev_end;
 	struct STP_LIST *lcl_stp;
 	struct RSL_LIST *lcl_rsl;

	pos_found = FALSE;
	curr_hhrs = curr_rsl->rgrs_ptr->rs_hhrs_hash;

	/*-------------------------------------------------------
	| Find first available slot on this resource for job.   |
	| NB. There may be more than one of this rsrc available |
	|     Therefore there may be multiple nodes for a rsrc. |
	-------------------------------------------------------*/
	lcl_rsl = curr_rsl;
	while (lcl_rsl != RSL_NULL)
	{
	    /*-----------------------------------------------------------
	    | Are we still on a valid node for the current resource     |
	    -----------------------------------------------------------*/
	    if (lcl_rsl->rgrs_ptr->rs_hhrs_hash != curr_hhrs)
	    {
            break;
		}

	    /*----------------------------------------
	    | Find position in linked list and check | 
	    | that step fits there without overlap.  |
	    ----------------------------------------*/
	    lcl_stp = lcl_rsl->fst_stp;
	    while (lcl_stp != STP_NULL)
	    {
			curr_start = curr_stp->st_date_time;
			curr_end   = curr_stp->st_date_time + curr_stp->duration;
			if (lcl_stp->prv_job == STP_NULL)
			{
			    prev_end   = 0L;
	        }
			else
			{
			    prev_end = lcl_stp->prv_job->st_date_time + 
			               lcl_stp->prv_job->duration;
			}
			/*-----------------------------------------
			| curr_stp must start and end before      |
			| lcl_stp starts AND curr_stp must finish |
			| after lcl_stp->prv_job if it exists.    |
			-----------------------------------------*/
			if (curr_start <= lcl_stp->st_date_time &&
			    curr_end   <= lcl_stp->st_date_time &&
			    (lcl_stp->prv_job == STP_NULL || curr_start > prev_end))
			{
			    /*------------------------
			    | OK TO INSERT STEP HERE |
			    ------------------------*/
			    if (lcl_stp == lcl_rsl->fst_stp)
			    {
					/*-------------
					| Head insert |
					-------------*/
					curr_stp->nxt_job         = lcl_rsl->fst_stp;
					lcl_rsl->fst_stp->prv_job = curr_stp;
					lcl_rsl->fst_stp	  = curr_stp;
					lcl_rsl->fst_job	  = curr_stp;

					/*--------------------------------------------
					| Is this also the last job on the resource. |
					--------------------------------------------*/
					if (lcl_rsl->lst_job == STP_NULL)
					{
						lcl_rsl->lst_job = curr_stp;
						lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
					}
			    }
			    else
			    {
					/*---------------
					| Middle insert |
					---------------*/
					curr_stp->nxt_job         = lcl_stp;
					curr_stp->prv_job         = lcl_stp->prv_job;
					lcl_stp->prv_job->nxt_job = curr_stp;
					lcl_stp->prv_job          = curr_stp;

			        /*-------------------------------------------------
			        | Is this step now the first job on the resource. |
			        -------------------------------------------------*/
					if (lcl_rsl->fst_job == STP_NULL ||
					    (lcl_rsl->fst_job != STP_NULL &&
					     curr_stp->st_date_time < lcl_rsl->fst_job->st_date_time))
					{
					    lcl_rsl->fst_job = curr_stp;
					}

			        /*------------------------------------------------
			        | Is this step now the last job on the resource. |
			        ------------------------------------------------*/
					if (lcl_rsl->lst_job == STP_NULL ||
					    curr_stp->st_date_time > lcl_rsl->lst_job->st_date_time)
					{
					    lcl_rsl->lst_job   = curr_stp;
					    lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
					}
			    }

			    pos_found = TRUE;
			    break;
			}

            lcl_stp = lcl_stp->nxt_job;
        }

	    if (pos_found)
	    {
    		break;
    	}
	    else
	    {
			/*----------------
			| Is APPEND OK ? |
			----------------*/
			if (lcl_rsl->fst_stp == STP_NULL ||
			     curr_stp->st_date_time > (lcl_rsl->lst_stp->st_date_time + lcl_rsl->lst_stp->duration))
			{
			    curr_stp->prv_job = lcl_rsl->lst_stp;
			    if (lcl_rsl->fst_stp == STP_NULL)
			    {
					lcl_rsl->fst_stp = curr_stp;
					lcl_rsl->fst_job = curr_stp;
			    }
			    else
			    {
			        lcl_rsl->lst_stp->nxt_job = curr_stp;
                }
			    lcl_rsl->lst_stp = curr_stp;

			    /*--------------------------------------------------------
			    | This step now becomes the last job on the resource.    |
			    --------------------------------------------------------*/
			    lcl_rsl->lst_job   = curr_stp;
			    lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
			    pos_found = TRUE;
			    break;
			}
	    }

	    lcl_rsl = lcl_rsl->nxt_rsl;
	}

	/*--------------------------------------------
	| Can't find a clear position on an rsl node |
	| where step won't overlap.                  |
	| SO find correct position on first rsl and  |
	| insert regardless of overlap.              |
	--------------------------------------------*/
	if (!pos_found)
	{
	    lcl_rsl = curr_rsl;
	    lcl_stp = curr_rsl->fst_stp;
	    while (lcl_stp != STP_NULL)
	    {
			if (curr_stp->st_date_time <= lcl_stp->st_date_time)
			{
			    /*------------------
			    | INSERT STEP HERE |
			    ------------------*/
			    if (lcl_stp == lcl_rsl->fst_stp)
			    {
					/*-------------
					| Head insert |
					-------------*/
					curr_stp->nxt_job         = lcl_rsl->fst_stp;
					lcl_rsl->fst_stp->prv_job = curr_stp;
					lcl_rsl->fst_stp	  = curr_stp;
					lcl_rsl->fst_job	  = curr_stp;

					/*--------------------------------------------
					| Is this also the last job on the resource. |
					--------------------------------------------*/
					if (lcl_rsl->lst_job == STP_NULL)
					{
					    lcl_rsl->lst_job = curr_stp;
					    lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
					}
			    }
			    else
			    {
					/*---------------
					| Middle insert |
					---------------*/
					curr_stp->nxt_job         = lcl_stp;
					curr_stp->prv_job         = lcl_stp->prv_job;
					lcl_stp->prv_job->nxt_job = curr_stp;
					lcl_stp->prv_job          = curr_stp;

			        /*-------------------------------------------------
			        | Is this step now the first job on the resource. |
			        -------------------------------------------------*/
					if (lcl_rsl->fst_job == STP_NULL ||
 				       (lcl_rsl->fst_job != STP_NULL &&
                        curr_stp->st_date_time < lcl_rsl->fst_job->st_date_time))
					{
					    lcl_rsl->fst_job = curr_stp;
					}

			        /*------------------------------------------------
			        | Is this step now the last job on the resource. |
			        ------------------------------------------------*/
					if (lcl_rsl->lst_job == STP_NULL ||
					   curr_stp->st_date_time > lcl_rsl->lst_job->st_date_time)
					{
					    lcl_rsl->lst_job   = curr_stp;
					    lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
					}
			    }

			    pos_found = TRUE;
			    break;
			}

			lcl_stp = lcl_stp->nxt_job;
	    }

	   /*-------------------------------
	   | Position not found for INSERT |
	   | therefore we must APPEND.     |
	   -------------------------------*/
	   if (!pos_found)
	   {
	       curr_stp->prv_job         = lcl_rsl->lst_stp;
	       lcl_rsl->lst_stp->nxt_job = curr_stp;
	       lcl_rsl->lst_stp          = curr_stp;
       
	       /*--------------------------------------------------------
	       | This step now becomes the last job on the resource.    |
	       --------------------------------------------------------*/
	       lcl_rsl->lst_job   = curr_stp;
	       lcl_rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
	   }
	}

	/*---------------------------------
	| Set pointer to resource record. |
	---------------------------------*/
	curr_stp->rsl_ptr = lcl_rsl;

	return;
}

/*-----------------------------------
| Find vertical position and insert |
-----------------------------------*/
void
fnd_vertical (
 struct STP_LIST *curr_stp)
{
	int    pos_found;
	struct STP_LIST *lcl_stp;
	struct JOB_LIST *lcl_job;

	lcl_job = curr_stp->job_dtl;
	lcl_stp = lcl_job->fst_stp;
	if (lcl_stp == curr_stp)   /* This is the only step in the job so far */
	{
		return;
	}

	/*------------------------------------------------
	| NB At this stage there will always be at least |
	| one step already in the current job list.      |
	------------------------------------------------*/
	pos_found = FALSE;
	while (lcl_stp != STP_NULL)
	{
		if (curr_stp->seq_no < lcl_stp->seq_no ||
		   (curr_stp->seq_no == lcl_stp->seq_no &&
	        curr_stp->line_no < lcl_stp->line_no)) 
		{
			/*-------------
			| Insert Here |
			-------------*/
			if (lcl_stp == lcl_job->fst_stp)
			{
				/*-------------
				| Head insert |
				-------------*/
				curr_stp->nxt_res = lcl_job->fst_stp;
				lcl_job->fst_stp->prv_res = curr_stp;
				lcl_job->fst_stp = curr_stp;
			}
			else
			{
				/*---------------
				| Middle insert |
				---------------*/
				curr_stp->nxt_res          = lcl_stp;
				curr_stp->prv_res          = lcl_stp->prv_res;
				lcl_stp->prv_res->nxt_res = curr_stp;
				lcl_stp->prv_res          = curr_stp;
			}

			pos_found = TRUE;
			break;
		}

		lcl_stp = lcl_stp->nxt_res;
	}

	if (!pos_found)
	{
		/*------------------------
		| Append after last step |
		------------------------*/
		curr_stp->prv_res         = lcl_job->lst_stp;
		lcl_job->lst_stp->nxt_res = curr_stp;
		lcl_job->lst_stp          = curr_stp;
	}

	return;
}

/*--------------------------
| Allocate memory to store |
| resource details         |
--------------------------*/
struct RGRS_REC*
mk_rsh_node ( 
 void)
{
	int    i;
	struct RGRS_REC *lcl_ptr	=	NULL;

	i = 0;
	while (i < 100)
	{
		lcl_ptr = (struct RGRS_REC *)malloc (sizeof (struct RGRS_REC));
		if (lcl_ptr != (struct RGRS_REC *)NULL)
		{
			break;
		}
		i++;
		sleep (sleepTime);
	}

	if (lcl_ptr == (struct RGRS_REC *)NULL)
	{
		sys_err ("Error in mk_rsh_node() During (MALLOC)", 12, PNAME);
	}

	return (lcl_ptr);
}

/*--------------------------
| Allocate memory to store |
| resource details         |
--------------------------*/
struct RSL_LIST*
mk_rsl_node (
 void)
{
	int    i;
	struct RSL_LIST *lcl_ptr	=	NULL;

	i = 0;
	while (i < 100)
	{
		lcl_ptr = (struct RSL_LIST *)malloc (sizeof (struct RSL_LIST));
		if (lcl_ptr != RSL_NULL)
		{
			break;
		}
		i++;
		sleep (sleepTime);
	}

	if (lcl_ptr == RSL_NULL)
	{
		sys_err ("Error in mk_rsl_node() During (MALLOC)", 12, PNAME);
	}

	return (lcl_ptr);
}

/*-------------------------------
| Allocate or Free memory for a |
| node OR Free The Whole List   |
-------------------------------*/
struct MDL_SEL*
mdl_node (
 int    node_act, 
 struct MDL_SEL *lst_ptr)
{
	int    i = 0;
	struct MDL_SEL *lcl_ptr;

	switch (node_act)
	{
	case ALLC_ND:
		/*-----------------
		| Allocate A Node |
		-----------------*/
		lcl_ptr = mdl_free;
		if (lcl_ptr != MDL_NULL)
		{
			mdl_free = mdl_free->next;
		}
		else
		{
			while (i < 100)
			{
				lcl_ptr = (struct MDL_SEL *)malloc (sizeof (struct MDL_SEL));
				if (lcl_ptr != MDL_NULL)
				{
					break;
				}
				i++;
				sleep (sleepTime);
			}
			if (lcl_ptr == MDL_NULL)
			{
				sys_err ("Error in mdl_node() During (MALLOC)", 
				         12, 
				         PNAME);
			}
		}
		return (lcl_ptr);

	case FREE_ND:
		/*--------------------------------
		| Transfer A Node Onto Free List |
		--------------------------------*/
		lst_ptr->next = mdl_free;
		mdl_free = lst_ptr;
		return (MDL_NULL);

	case FREE_LST:
		/*------------------
		| Free entire list |
		------------------*/
		while (lst_ptr != MDL_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->next;
			free (lcl_ptr);
		}

		lst_ptr = mdl_free;
		while (lst_ptr != MDL_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->next;
			free (lcl_ptr);
		}
		lst_ptr  = MDL_NULL;
		mdl_free = MDL_NULL;
		return (MDL_NULL);

	/* QUERY
	 * warning! no default case! not all execution paths 
	 *    have a return value!
	 */
	}

	/* QUERY 
	 * added this just in case the cases fall thru. 
	 */
	return (MDL_NULL);
}

/*-------------------------------
| Free all steps in the current |
| model to the free list.       |
-------------------------------*/
void
clear_steps (
 void)
{
	struct	RSL_LIST	*lcl_rsl;
	struct	STP_LIST	*lcl_stp, *tmp_stp;

	/*---------------------------------------------
	| Put all allocated step nodes onto free list |
	---------------------------------------------*/
	lcl_rsl = rsl_head;
	while (lcl_rsl != RSL_NULL)
	{
	    if (lcl_rsl->fst_job != STP_NULL)
	    {
			lcl_stp = lcl_rsl->fst_job;
			while (lcl_stp != STP_NULL)
			{
			    /*----------------------------
			    | Don't clear calendar steps |
			    ----------------------------*/
			    if (lcl_stp->job_dtl == JOB_NULL)
			    {
					lcl_stp = lcl_stp->nxt_job;
					continue;
			    }

			    tmp_stp = lcl_stp;
			    lcl_stp = lcl_stp->nxt_job;

			    /*---------------------------------------------
			    | Take step out of linked list before FREEing |
			    | Integrity between calendar nodes MUST be    |
			    | maintained.                                 |
			    ---------------------------------------------*/
			    if (tmp_stp->nxt_job != STP_NULL)
			    {
				    tmp_stp->nxt_job->prv_job = tmp_stp->prv_job;
				}

			    if (tmp_stp->prv_job != STP_NULL)
			    {
				    tmp_stp->prv_job->nxt_job = tmp_stp->nxt_job;
				}

			    if (tmp_stp == lcl_rsl->fst_stp)
			    {
				    lcl_rsl->fst_stp = lcl_stp;
				}

			    if (tmp_stp == lcl_rsl->lst_stp)
			    {
				    lcl_rsl->fst_stp = STP_NULL;
				}

			    tmp_stp = stp_node(FREE_ND, tmp_stp);
			}
	    }
	    lcl_rsl->fst_job = STP_NULL;
	    lcl_rsl->lst_job = STP_NULL;

	    lcl_rsl = lcl_rsl->nxt_rsl;
	}
	job_head = JOB_NULL;
	job_tail = JOB_NULL;

	return;
}

/*-------------------------------
| Allocate or Free memory for a |
| node OR Free The Whole List   |
-------------------------------*/
struct STP_LIST*
stp_node (
 int    node_act, 
 struct STP_LIST *lst_ptr)
{
	int    i = 0;
	struct	STP_LIST	*lcl_ptr;

	switch (node_act)
	{
	case ALLC_ND:
		/*-----------------
		| Allocate A Node |
		-----------------*/
		lcl_ptr = stp_free;
		if (lcl_ptr != STP_NULL)
		{
			stp_free = stp_free->nxt_job;
		}
		else
		{
			while (i < 100)
			{
				lcl_ptr = (struct STP_LIST *)malloc (sizeof (struct STP_LIST));
				if (lcl_ptr != STP_NULL)
				{
					break;
				}
				i++;
				sleep (sleepTime);
			}

			if (lcl_ptr == STP_NULL)
			{
				sys_err ("Error in stp_node() During (MALLOC)", 
				         12, 
				         PNAME);
			}
		}
		/*-----------------------------
		| Initialise contents of node |
		-----------------------------*/
		lcl_ptr->nxt_job      = STP_NULL;
		lcl_ptr->prv_job      = STP_NULL;
		lcl_ptr->nxt_res      = STP_NULL;
		lcl_ptr->prv_res      = STP_NULL;
		lcl_ptr->job_dtl      = JOB_NULL;
		lcl_ptr->rsl_ptr      = RSL_NULL;
		lcl_ptr->seq_no       = 0;
		lcl_ptr->line_no      = 0;
		lcl_ptr->st_date_time = 0L;
		lcl_ptr->duration     = 0L;
		lcl_ptr->setup        = 0L;
		lcl_ptr->run          = 0L;
		lcl_ptr->clean        = 0L;
		lcl_ptr->status       = 0;

		return (lcl_ptr);

	case FREE_ND:
		/*--------------------------------
		| Transfer A Node Onto Free List |
		--------------------------------*/
		lst_ptr->nxt_job       = stp_free;
		stp_free               = lst_ptr;
		stp_free->seq_no       = 0;
		stp_free->line_no      = 0;
		stp_free->st_date_time = 0L;
		stp_free->duration     = 0L;
		stp_free->setup        = 0L;
		stp_free->run          = 0L;
		stp_free->clean        = 0L;
		stp_free->status       = 0;
		return (STP_NULL);

	case FREE_LST:
		/*------------------
		| Free entire list |
		------------------*/
		while (lst_ptr != STP_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->nxt_job;
			free (lcl_ptr);
		}

		lst_ptr = stp_free;
		while (lst_ptr != STP_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->nxt_job;
			free (lcl_ptr);
		}
		lst_ptr  = STP_NULL;
		stp_free = STP_NULL;
		return (STP_NULL);
	}

	/* QUERY 
	 * warning! no default case! a fall thru might occur!
	 * not all execution paths have a return so we add one.
	 */

    return (STP_NULL);
}

/*-------------------------------
| Allocate or Free memory for a |
| node OR Free The Whole List   |
-------------------------------*/
struct JOB_LIST*
job_node (
 int    node_act, 
 struct JOB_LIST *lst_ptr)
{
	int    i = 0;
	struct JOB_LIST *lcl_ptr;

	switch (node_act)
	{
	case ALLC_ND:
		/*-----------------
		| Allocate A Node |
		-----------------*/
		lcl_ptr = job_free;
		if (lcl_ptr != JOB_NULL)
		{
			job_free = job_free->nxt_job;
		}
		else
		{
			while (i < 100)
			{
				lcl_ptr = (struct JOB_LIST *)malloc (sizeof (struct JOB_LIST));
				if (lcl_ptr != JOB_NULL)
				{
					break;
				}
				i++;
				sleep (sleepTime);
			}
			if (lcl_ptr == JOB_NULL)
			{
				sys_err ("Error in job_node() During (MALLOC)", 
				         12, 
				         PNAME);
			}
		}
		return (lcl_ptr);

	case FREE_ND:
		/*--------------------------------
		| Transfer A Node Onto Free List |
		--------------------------------*/
		lst_ptr->nxt_job = job_free;
		job_free = lst_ptr;
		return (JOB_NULL);

	case FREE_LST:
		/*------------------
		| Free entire list |
		------------------*/
		while (lst_ptr != JOB_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->nxt_job;
			free (lcl_ptr);
		}

		lst_ptr = job_free;
		while (lst_ptr != JOB_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->nxt_job;
			free (lcl_ptr);
		}
		lst_ptr  = JOB_NULL;
		job_free = JOB_NULL;
		return (JOB_NULL);
	}
	/* QUERY 
	 * warning! no default case! a fall thru might occur!
	 * not all execution paths have a return so we add one.
	 */

    return (JOB_NULL);
}


int
heading (
 int scn)
{
	if (scn != 0)
	{
		return (EXIT_SUCCESS);
	}

	swide ();
	clear ();

	move (0,1);
	line (132);

	strcpy (err_str, ML (mlPsMess002));
	rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);

	move (0,20);
	line (132);

	print_at (21, 0,  
	          "%s : %2.2s - %-30.30s", 
	          ML ("Company"),
	          comm_rec.tco_no, 
	          comm_rec.tco_name);

	print_at (21, 45, 
	          "%s : %2.2s - %-30.30s", 
	          ML ("Branch"),
	          comm_rec.test_no, 
	          comm_rec.test_name);

	print_at (21, 95, 
	          "%s : %2.2s - %-30.30s", 
	          ML ("W/house"),
	          comm_rec.tcc_no, 
	          comm_rec.tcc_name);

	return (EXIT_SUCCESS);
}

/*====================================
| Trev's additions to Cam's stuff...  |
======================================*/
void
upd_lst_dt_tm (
 struct RSL_LIST *rsl)
{
	struct STP_LIST *curr_stp;

	rsl->lst_dt_tm = 0L;
	curr_stp = rsl->fst_job;
	while (curr_stp != STP_NULL)
	{
	    if (curr_stp->st_date_time + curr_stp->duration > rsl->lst_dt_tm &&
		    curr_stp->job_dtl != JOB_NULL)
	    {
            rsl->lst_dt_tm = curr_stp->st_date_time + curr_stp->duration;
	    }
	    curr_stp = curr_stp->nxt_job;
	}
}


int 
done_func (
 int    c,  
 KEY_TAB *psUnused)
{
	return (c);
}

/*-------------------------------
| Tag jobs to delete from model	|
-------------------------------*/
int
del_tag_func (
 int    c, 
 KEY_TAB *psUnused)
{
	switch (c)
	{
	case 'T':
	case '\r':
		tag_toggle ("del_job");
		break;

	case CTRL('T'):
		tag_all ("del_job");
		break;

	default:
		break;
	}

	return (c);
}

/*-------------------------------
| Delete Jobs from Model.       |
-------------------------------*/
void
del_jobs (
 void)
{
	struct JOB_LIST *job_curr = job_head;
	int    i,  
	       jobs_fnd = 0;
	char   get_buf[200];

	tab_open ("del_job", del_jkeys, 4, 2, 9, FALSE);
	tab_add ("del_job",
	         "# %-7.7s | %-10.10s | %-16.16s | %-14.14s | %-8.8s | %-40.40s ", 
	         "JOB NO.",
	         "BATCH  NO.",
	         "  ITEM  NUMBER  ",
	         "   QUANTITY   ",
	         "DATE REQ",
	         "            ITEM DESCRIPTION            ");

	/*-------------------------------
	| Allow selection of jobs.     |
	-------------------------------*/
	while (job_curr != JOB_NULL)
	{
		cc = find_hash (pcwo2, &pcwo_rec, EQUAL, "r", job_curr->hhwo_hash);
		if (!cc && pcwo_rec.wo_order_status[0] == 'A')
		{
			jobs_fnd++;
			tab_add ("del_job",
			         " %-7.7s | %-10.10s | %-16.16s | %14.6f |%-10.10s| %-40.40s           %10ld",
			         job_curr->order_no,
			         pcwo_rec.wo_batch_no,
			         job_curr->item_no,
			         job_curr->prod_qty,
			         DateToString (job_curr->reqd_date),
			         job_curr->description,
			         job_curr->hhwo_hash);
		}
		job_curr = job_curr->nxt_job;
	}

	if (jobs_fnd == 0)
	{
		putchar (BELL);
		fflush (stdout);
		tab_add ("del_job", "%-22.22s NO JOBS TO DELETE ", " ");
		tab_display ("del_job", TRUE);
		crsr_off ();
		fflush (stdout);
		sleep (sleepTime);
	}
	else
	{
		/*------------------------
		| Allow user to tag jobs |
		------------------------*/
		tab_scan ("del_job");
		if (restart)
		{
			restart = FALSE;
			return;
		}

		/*-----------------------
		| Remove tagged jobs	|
		-----------------------*/
		for (i = 0; i < jobs_fnd; i++)
		{
			tab_get ("del_job", get_buf, EQUAL, i);
			if (tagged (get_buf))
			{
				/*-----------------------
				| Remove from model     |
				| (Remove pstq record)	|
				-----------------------*/
				del_pstq (atol (get_buf + 116));
				jobs_in_model--;
			}
		}
	}

	tab_close ("del_job", TRUE);

	return;
}

/*-----------------------
| Remove all pstq recs	|
| for a Works Order.	|
-----------------------*/
void
del_pstq (
 long   hhwo_hash)
{
	struct JOB_LIST    *job_curr = job_head;
	struct RSL_LIST    *tmp_rsl;
	struct STP_LIST    *stp_curr,
	                   *old_stp;

	while (job_curr != JOB_NULL && 
	       job_curr->hhwo_hash != hhwo_hash)
	{
		job_curr = job_curr->nxt_job;
	}
	
	if (job_curr == JOB_NULL)
	{
		return;
	}
	stp_curr = job_curr->fst_stp;
	tmp_rsl = stp_curr->rsl_ptr;

	/*------------------------------
	| Remove job from JOB_LIST.     |
	-------------------------------*/
	if (job_curr->prv_job == JOB_NULL)
	{
		job_head = job_curr->nxt_job;
	}
	else
	{
		job_curr->prv_job->nxt_job = job_curr->nxt_job;
	}
	if (job_curr->nxt_job == JOB_NULL)
	{
		job_tail = job_curr->prv_job;
	}
	else
	{
		job_curr->nxt_job->prv_job = job_curr->prv_job;
	}

	while (stp_curr != STP_NULL)
	{
		/*-------------------------------
		| Step 1: Remove from pstq.	|
		-------------------------------*/
		pstq_rec.tq_hhqm_hash	= slct_hhqm;
		pstq_rec.tq_hhrs_hash	= tmp_rsl->rgrs_ptr->rs_hhrs_hash;
		pstq_rec.tq_hhwo_hash	= hhwo_hash;
		pstq_rec.tq_seq_no	= stp_curr->seq_no;
		pstq_rec.tq_line_no	= stp_curr->line_no;

		cc = find_rec (pstq3, &pstq_rec, EQUAL, "u");
		if (cc)
		{
			file_err (cc, pstq3, "DBFIND");
		}

		cc = abc_delete (pstq3);
		if (cc)
		{
			file_err (cc, pstq3, "DBDELETE");
		}

		/*-------------------------------
		| Step 2: Remove from list(s).	|
		-------------------------------*/
		if (stp_curr->prv_job == STP_NULL)
		{
			tmp_rsl->fst_job = stp_curr->nxt_job;
		}
		else
		{
			stp_curr->prv_job->nxt_job = stp_curr->nxt_job;
		}
		if (stp_curr->nxt_job == STP_NULL)
		{
			tmp_rsl->lst_job = stp_curr->prv_job;
		}
		else
		{
			stp_curr->nxt_job->prv_job = stp_curr->prv_job;
		}

		upd_lst_dt_tm (tmp_rsl);

		old_stp = stp_curr;
		stp_curr = stp_curr->nxt_res;
		stp_node (FREE_ND, old_stp);
		if (stp_curr != STP_NULL)
		{
			tmp_rsl = stp_curr->rsl_ptr;
		}
	}
	job_node (FREE_ND, job_curr);

	return;
}

/*-------------------------------
| Add job (s) to current Model.	|
-------------------------------*/
void
add_jobs (
 void)
{
	int    i;
	int    jobs_fnd;
	char   get_buf[200];

	jobs_fnd = tag_new_job (4, 2, 9, TRUE);
	if (jobs_fnd > 0)
	{
		/*------------------------
		| Allow user to tag jobs |
		------------------------*/
		tab_scan ("new_mdl");
		if (restart)
		{
			tab_close ("new_mdl", TRUE);
			restart = FALSE;
			return;
		}

		/*-----------------
	 	| Add tagged jobs |
		-----------------*/
	 	for (i = 0; i < jobs_fnd; i++)
	    {
			tab_get ("new_mdl", get_buf, EQUAL, i);
			if (tagged (get_buf))
			{
        		/*------------------
	    		| Include in model |
	    		| (add pstq rec)   |
	    		------------------*/
				add_pstq (atol (get_buf + 116), 'T');
				jobs_in_model++;
			}
	    }
    }
	else
	{
	    	putchar (BELL);
	    	fflush (stdout);
	    	tab_add ("new_mdl", "%-25.25s NO JOBS IN BIN ", " ");
	    	tab_display ("new_mdl", TRUE);
	    	crsr_off ();
	    	fflush (stdout);
	    	sleep (sleepTime);
	}

	tab_close ("new_mdl", TRUE);

	return;
}

/*-----------------------
| View Jobs in Model.	|
-----------------------*/
void
view_jobs (
 void)
{
	struct JOB_LIST    *job_curr = job_head;
	int    i,
	       jobs_fnd = 0;
	char   get_buf[200];

	tab_open ("view_job", (KEY_TAB *) 0, 4, 2, 9, FALSE);
	tab_add ("view_job",
			 "# %-7.7s | %-10.10s | %-16.16s | %-14.14s | %-8.8s | %-40.40s ", 
			 "JOB NO.",
			 "BATCH  NO.",
			 "  ITEM  NUMBER  ",
			 "   QUANTITY   ",
			 "DATE REQ",
			 "            ITEM DESCRIPTION            ");

	/*-------------------------------
	| Allow selection of jobs.      |
	-------------------------------*/
	while (job_curr != JOB_NULL)
	{
		jobs_fnd++;

		cc = find_hash (pcwo2, &pcwo_rec, EQUAL, "r", job_curr->hhwo_hash);
		if (cc)
		{
			file_err (cc, pcwo, "DBFIND");
		}
		tab_add ("view_job",
		         " %-7.7s | %-10.10s | %-16.16s | %14.6f |%-10.10s| %-40.40s           %10ld",
		         job_curr->order_no,
		         pcwo_rec.wo_batch_no,
		         job_curr->item_no,
		         job_curr->prod_qty,
		         DateToString (job_curr->reqd_date),
		         job_curr->description,
		         job_curr->hhwo_hash);

		job_curr = job_curr->nxt_job;
	}

	if (jobs_fnd == 0)
	{
		putchar (BELL);
		fflush (stdout);
		tab_add ("view_job", "%-22.22s NO JOBS TO DELETE ", " ");
		tab_display ("view_job", TRUE);
		crsr_off ();
		fflush (stdout);
		sleep (sleepTime);
	}
	else
	{
		/*------------------------
		| Allow user to tag jobs |
		------------------------*/
		tab_scan ("view_job");
		if (restart)
		{
			restart = FALSE;
			return;
		}

		/*-----------------------
		| Remove tagged jobs	|
		-----------------------*/
		for (i = 0; i < jobs_fnd; i++)
		{
			tab_get ("view_job", get_buf, EQUAL, i);
			if (tagged (get_buf))
			{
				/*-----------------------
				| Remove from model	|
				| (Remove pstq record)	|
				-----------------------*/
				del_pstq (atol (get_buf + 116));
				jobs_in_model--;
			}
		}
	}

	tab_close ("view_job", TRUE);

	return;
}

int
gantt (
 void)
{
	char   *sptr;
	int    i,
	       j = -1;

	/*-------------------------------
	| Find the initial resolution.	|
	-------------------------------*/
	sptr = get_env ("PC_TIME_RES");

	if (sptr != (char *) 0)
	{
		time_resn = atol (sptr);
	}
	if (time_resn < 1L)
	{
		time_resn = 1L;
	}
	min_resn = time_resn;
	/*-------------------------------
	| Ascertain valid increments.	|
	-------------------------------*/
	strcpy (val_times, "NNNNNNNNNNNNNNNNNNNN");
	sptr = get_env ("PS_VAL_TIMES");
	for (i = 0; i < 21 && *sptr; i++)
	{
		if (*sptr == 'Y' || *sptr == 'y')
		{
			if (time_resn <= act_times[i])
			{
				val_times[i] = 'Y';
				if (j == -1)
				{
					j = i;
				}
			}
		}
		sptr++;
	}
	return (gnt_sched (j));
}

int
gnt_sched (
 int    x)
{
	int		old_indx,
			do_redraw;

	long	tmp_dt_tm;

	indx = x;
	curr_dt_tm = 1440L * TodaysDate ();

	strcpy (err_str, TimeHHMMSS());
	curr_dt_tm += atot (err_str);
	end_dt_tm = curr_dt_tm % time_incs[indx];
	beg_dt_tm = curr_dt_tm - end_dt_tm;	/* Set LHS Date/Time	*/
	end_dt_tm = beg_dt_tm + (120L * time_resn);
	rsl_curr = scn_head = rsl_head;
	cc = '7';			/* Force position = top left	*/
	do_redraw = 3;			/* Force a FULL redraw		*/
	while (1)
	{
		crsr_off ();
		if (cc == 0)
		{
			if (do_redraw)
			{
				time_resn = act_times[indx];
				end_dt_tm = beg_dt_tm % time_incs[indx];
				beg_dt_tm -= end_dt_tm;
				if ((end_dt_tm * 2) > time_incs[indx])
				{
					beg_dt_tm += time_incs[indx];
				}
				end_dt_tm = beg_dt_tm + (120L * time_resn);
				prep_scn (scn_head, do_redraw);
			}
			else
			{
				get_job_info (rsl_curr->fst_stp);
			}
			tmp_dt_tm = beg_dt_tm + (time_resn * (long) x_pos);
			print_at (17, 62, 
			          " %-10.10s %s ",
			          DateToString (tmp_dt_tm / 1440L),
			          ttoa ((tmp_dt_tm % 1440L), "NN:NN"));

			if (old_stp != curr_stp || old_chr != curr_chr)
			{
				dsp_jdt (rsl_curr);
			}
			old_stp = curr_stp;
			old_chr = curr_chr;
			move (x_pos + 10, y_pos + 2);
			fflush (stdout);
			crsr_on ();
			do_redraw = 0;

			cc = getkey ();
		}

		switch (cc)
		{
		case	PSLW:
			cc = do_menu (hed_mnu);
			old_stp = STP_NULL;	/* Force redraw of job info */
			do_redraw = 3;
			continue;
			break;

		case	FN1:
		case	FN16:
            /* did you know that this is the only place where 
             *      this function returns?
             */
			return (cc); 

		case	FN3:
			old_stp = STP_NULL;	/* Force redraw of job info */
			do_redraw = 3;
			break;

		case	FN6:
			do_help ();
			old_stp = STP_NULL;	/* Force redraw of job info */
			do_redraw = 3;
			break;

		case	FN14:
			for (y_pos = 0, rsl_curr = scn_head; y_pos < 14 && rsl_curr->nxt_rsl != RSL_NULL;)
			{
				y_pos++;
				rsl_curr = rsl_curr->nxt_rsl;
			}
			if (rsl_curr->nxt_rsl != RSL_NULL)
			{
				do_redraw = 3;
				y_pos = 0;
				scn_head = rsl_curr->nxt_rsl;
				rsl_curr = scn_head;
			}
			break;

		case	FN15:
			if (scn_head->prv_rsl != RSL_NULL)
			{
				for (y_pos = 0; y_pos < 15 && scn_head->prv_rsl != RSL_NULL;)
				{
					y_pos++;
					scn_head = scn_head->prv_rsl;
				}
				rsl_curr = scn_head;
				do_redraw = 2;
			}
			else
			{
				if (y_pos == 0)
				{
					putchar (BELL);
				}
				else
				{
					rsl_curr = scn_head;
				}
			}
			y_pos = 0;
			break;

		case	LEFT_KEY:
		case	8:
			if (x_pos)
			{
				x_pos--;
			}
			else
			{
				beg_dt_tm -= time_incs[indx];
				end_dt_tm = beg_dt_tm + (120L * time_resn);
				do_redraw = 1;
			}
			break;

		case	RIGHT_KEY:
			if (x_pos < 119)
			{
				x_pos++;
			}
			else
			{
				beg_dt_tm += time_incs[indx];
				end_dt_tm = beg_dt_tm + (120L * time_resn);
				do_redraw = 1;
			}
			break;

		case	UP_KEY:
			if (y_pos)
			{
				rsl_curr = rsl_curr->prv_rsl;
				y_pos--;
			}
			else
			{
				if (scn_head->prv_rsl == RSL_NULL)
				{
					putchar (BELL);
				}
				else
				{
					scn_head = rsl_curr = scn_head->prv_rsl;
					do_redraw = 2;
				}
			}
			break;

		case	DOWN_KEY:
			if (y_pos < 14)
			{
				if (rsl_curr->nxt_rsl != RSL_NULL)
				{
					rsl_curr = rsl_curr->nxt_rsl;
					y_pos++;
				}
				else
				{
					putchar (BELL);
				}
			}
			else
			{
				if (rsl_curr->nxt_rsl == RSL_NULL)
				{
					putchar (BELL);
				}
				else
				{
					scn_head = scn_head->nxt_rsl;
					rsl_curr = rsl_curr->nxt_rsl;
					do_redraw = 2;
				}
			}
			break;

		case	'1':		/* Bot LEFT	*/
		case	'2':		/* Bot CENTRE	*/
		case	'3':		/* Bot RIGHT	*/
			if (cc == '1')
			{
				x_pos = 0;
			}

			if (cc == '2')
			{
				x_pos = 60;
			}

			if (cc == '3')
			{
				x_pos = 119;
			}

			for (y_pos = 0, rsl_curr = scn_head; y_pos < 14 && rsl_curr->nxt_rsl != RSL_NULL;)
			{
				y_pos++;
				rsl_curr = rsl_curr->nxt_rsl;
			}
			break;

		case	'4':		/* Mid LEFT	*/
		case	'5':		/* Mid CENTRE	*/
		case	'6':		/* Mid RIGHT	*/
			if (cc == '4')
			{
				x_pos = 0;
			}

			if (cc == '5')
			{
				x_pos = 60;
			}

			if (cc == '6')
			{
				x_pos = 119;
			}
			for (y_pos = 0, rsl_curr = scn_head; y_pos < 7 && rsl_curr->nxt_rsl != RSL_NULL;)
			{
				y_pos++;
				rsl_curr = rsl_curr->nxt_rsl;
			}
			break;

		case	'7':		/* Top LEFT	*/
		case	'8':		/* Top CENTRE	*/
		case	'9':		/* Top RIGHT	*/
			if (cc == '7')
			{
				x_pos = 0;
			}

			if (cc == '8')
			{
				x_pos = 60;
			}

			if (cc == '9')
			{
				x_pos = 119;
			}

			rsl_curr = scn_head;
			y_pos = 0;
			break;

		case	'A':
		case	'a':
			add_jobs ();
			cc = FN3;
			continue;
			break;

		case	'C':
		case	'c':
			cc = change_step ();
			continue;
			break;

		case	'D':
		case	'd':
			del_jobs ();
			cc = FN3;
			continue;
			break;

		case	'H':
		case	'h':
			cc = hilite_job ();
			continue;
			break;

		case	'I':
		case	'i':
			if (indx == 0)
			{
				putchar (BELL);
				break;
			}
			old_indx = indx;
			indx--;
			while (val_times[indx] != 'Y' && indx)
			{
				indx--;
			}
			if (val_times[indx] != 'Y')
			{
				putchar (BELL);
				indx = old_indx;
			}
			else
			{
				if (do_redraw == 0)
				{
					do_redraw = 1;
				}
			}
			tmp_dt_tm = beg_dt_tm + (time_resn * (long) x_pos);
			time_resn = act_times[indx];
			beg_dt_tm = tmp_dt_tm - (time_resn * (long) x_pos);
			end_dt_tm = beg_dt_tm % time_incs[indx];
			beg_dt_tm -= end_dt_tm;

			if ((end_dt_tm * 2) > time_incs[indx])
			{
				beg_dt_tm += time_incs[indx];
			}
			end_dt_tm = beg_dt_tm + (120L * time_resn);
			break;

		case	'J':
		case	'j':
			join_step ();
			cc = FN3;
			continue;
			break;

		case	'M':
		case	'm':
			if (do_redraw)
			{
				prep_scn (scn_head, do_redraw);
				move (x_pos + 10, y_pos + 2);
				fflush (stdout);
				crsr_on ();
				do_redraw = 0;
			}
			cc = move_step ();
			continue;
			break;

		case	'O':
		case	'o':
			if (indx == 20)
			{
				putchar (BELL);
				break;
			}
			old_indx = indx;
			indx++;
			while (val_times[indx] != 'Y' && indx < 20)
			{
				indx++;
			}
			if (val_times[indx] != 'Y')
			{
				putchar (BELL);
				indx = old_indx;
			}
			else
			{
				if (do_redraw == 0)
				{
					do_redraw = 1;
				}
			}
			tmp_dt_tm = beg_dt_tm + (time_resn * (long) x_pos);
			time_resn = act_times[indx];
			beg_dt_tm = tmp_dt_tm - (time_resn * (long) x_pos);
			end_dt_tm = beg_dt_tm % time_incs[indx];
			beg_dt_tm -= end_dt_tm;

			if ((end_dt_tm * 2) > time_incs[indx])
			{
				beg_dt_tm += time_incs[indx];
			}
			end_dt_tm = beg_dt_tm + (120L * time_resn);
			break;

		case	'P':
		case	'p':
/*
			pack_jobs ();
			cc = FN3;
			continue;
*/
			print_at (23, 0, ML (mlPsMess009));
			break;

		case	'S':
		case	's':
			split_step ();
			cc = FN3;
			continue;
			break;

		case	'V':
		case	'v':
			view_jobs ();
			cc = FN3;
			continue;
			break;

		default:
			putchar (BELL);
			break;

		}; /*  end of switch (cc) */
		
		cc = 0;

	}; /* end of while (1) */
}

/*===============================
| Redraw the gantt chart.       |
| Flag	: ACTION	            |
| 3	: FULL SCREEN           	|
| 2	: STEPS & RESOURCES	        |
| 1	: STEPS & HEADINGS          |
| 0	: NO ACTION                 |
===============================*/
void
prep_scn (
 struct RSL_LIST *scn_head, 
 int    flag)
{
	if (flag == 3)
	{
		clear ();
		print_at (0, 0, "[HELP]");
		box (9, 1, 122, 15);
		move (9, 0);
		PGCHAR (5);
		move (130, 0);
		PGCHAR (5);
		move (0, 1);
		line (10);
		PGCHAR (7);
		move (130, 1);
		PGCHAR (11);
		move (0, 17);
		line (10);
		PGCHAR (9);
	}
	
	if (flag & 1)
	{
		dsp_hdgs ();
	}
	curr_chr = ' ';
	curr_stp = STP_NULL;
	dsp_rgrs (scn_head);
	fflush (stdout);
}

void
dsp_hdgs (
 void)
{
	int    i;
	long   tmp_dt_tm = beg_dt_tm;

	print_at (0, 10, "%-120.120s", " ");
	if (time_resn <= 60L)
	{
		for (i = 0; i < 12; i++)
		{
			tmp_dt_tm %= 1440L;
			print_at (0, 10 + (i * 10), "%s", ttoa (tmp_dt_tm, "     "));
			tmp_dt_tm += (time_resn * 10L);
		}
		return;
	}

	/*---------------------------------------
	| Everything from here uses days or >	|
	---------------------------------------*/
	tmp_dt_tm /= 1440L;

	if (time_resn == 180L)
	{
		for (i = 0; i < 7; i++)
		{
			print_at (0, 10 + (i * 16), "%-10.10s", DateToString (tmp_dt_tm));
			tmp_dt_tm += 2L;
		}
		return;
	}

	if (time_resn <= 480L)
	{
		for (i = 0; i < 10; i++)
		{
			print_at (0, 10 + (i * 12), "%-10.10s", DateToString (tmp_dt_tm));
			tmp_dt_tm += (time_resn / 120L);
		}
		return;
	}

	if (time_resn <= 1440L)
	{
		for (i = 0; i < 8; i++)
		{
			print_at (0, 10 + (i * 14), "%-10.10s", DateToString (tmp_dt_tm));
			tmp_dt_tm += (time_resn == 720L) ? 7L : 14L;
		}
		return;
	}

	if (time_resn == 10080L)
	{
		for (i = 0; i < 9; i++)
		{
			print_at (0, 10 + (i * 13), "%-10.10s", DateToString (tmp_dt_tm));
			tmp_dt_tm += 91L;
		}
		return;
	}
}

void
dsp_rgrs (
 struct RSL_LIST *rsl_list)
{
	int    curr_lin;
	long   last_hhrs_hash;

	last_hhrs_hash = 0L;
	curr_lin = 0;
	while (rsl_list != RSL_NULL && curr_lin < 15)
	{
		if (rsl_list->rgrs_ptr->rs_hhrs_hash != last_hhrs_hash)
		{
			print_at (curr_lin + 2, 0, "%-8.8s", rsl_list->rgrs_ptr->rs_code);
		}
		else
		{
			print_at (curr_lin + 2, 0, "%-8.8s", " ");
		}
		move (10, curr_lin + 2);
		dsp_jobs (rsl_list->fst_stp, curr_lin);
		curr_lin++;
		last_hhrs_hash = rsl_list->rgrs_ptr->rs_hhrs_hash;
		rsl_list = rsl_list->nxt_rsl;
	}
}

void
dsp_jobs (
 struct STP_LIST *stp_list,
 int    row)
{
	char    dsp_chr;
	int     col = 0;
	long	tmp_date;
	long	tmp_dt_tm = beg_dt_tm;
	struct	STP_LIST	*tmp_list;

	/*---------------------------------------
	| Go thru the list to find the 1st job	|
	| that 'fits' on the screen somewhere.	|
	---------------------------------------*/
	while (stp_list != STP_NULL && stp_list->st_date_time <= tmp_dt_tm)
	{
		if ((stp_list->st_date_time + stp_list->duration) >= tmp_dt_tm)
        {
			break;
        }
		stp_list = stp_list->nxt_job;
	}

	while (tmp_dt_tm < end_dt_tm)
	{
		if (stp_list == STP_NULL)
		{
			print_at (row + 2, col + 10, "%*.*s", 120 - col, 120 - col, " ");
			return;
		}

		if (stp_list->st_date_time >= tmp_dt_tm + time_resn)
		{
			dsp_chr = ' ';
			do_hilite = FALSE;
		}
		else
		{
			if (stp_list->job_dtl != JOB_NULL)
            {
				do_hilite = stp_list->job_dtl->is_hilite;
            }
			if (stp_list->st_date_time < tmp_dt_tm)
            {
				dsp_chr = '-';
            }
			else
            {
				dsp_chr = '+';
            }
        }       

		if (stp_list->job_dtl != JOB_NULL)
		{
			/*-------------------------------
			| Find the LAST step of this JOB|
			| to see if it is 'L'ate yet!	|
			-------------------------------*/
			tmp_list = stp_list->job_dtl->lst_stp;
			tmp_date = tmp_list->st_date_time + tmp_list->duration;
			tmp_date /= 1440L;
			if (tmp_date > stp_list->job_dtl->reqd_date && dsp_chr != ' ')
            {
				dsp_chr = 'L';
            }
		}
		else
        {
			if (dsp_chr != ' ')
            {
				dsp_chr = 'X';
            }
        }

		dsp_chr = check_overlap (stp_list, tmp_dt_tm, dsp_chr);

		if (do_hilite)
        {
			rv_on ();
        }
		printf ("%c", dsp_chr);
		if (do_hilite)
        {
			rv_off ();
        }
		fflush (stdout);

		if (row == y_pos && col == x_pos)
		{
			curr_chr = dsp_chr;
			curr_stp = stp_list;
		}

		tmp_dt_tm += time_resn;
		while (1)
		{
			if (tmp_dt_tm == stp_list->st_date_time && 
                stp_list->duration == 0L)
            {
				break;
            }
			if (tmp_dt_tm >= (stp_list->st_date_time + stp_list->duration))
			{
				stp_list = stp_list->nxt_job;
				if (stp_list != STP_NULL)
                {
					continue;
                }
			}
			break;
		}
		col++;
	}
}

void
get_job_info (
 struct STP_LIST *stp_list)
{
	char   dsp_chr;
	int    col = 0;
	long   tmp_date;
	long   tmp_dt_tm = beg_dt_tm;
	struct STP_LIST *tmp_list;

	curr_chr = ' ';
	curr_stp = STP_NULL;
	/*---------------------------------------
	| Go thru the list to find the 1st job	|
	| that 'fits' on the screen somewhere.	|
	---------------------------------------*/
	while (stp_list != STP_NULL && stp_list->st_date_time <= tmp_dt_tm)
	{
		if ((stp_list->st_date_time + stp_list->duration) >= tmp_dt_tm)
		{
			break;
		}
		stp_list = stp_list->nxt_job;
	}

	while (tmp_dt_tm < end_dt_tm)
	{
	    if (stp_list == STP_NULL)
	    {
            return;
        }

	    if (col == x_pos)
	    {
			if (stp_list->st_date_time >= tmp_dt_tm + time_resn)
			{
			    dsp_chr = ' ';
            }			   
			else
			{
			    if (stp_list->st_date_time < tmp_dt_tm)
			    {
				    dsp_chr = '-';
				}
			    else
			    {
				    dsp_chr = '+';
				}
			}

			if (stp_list->job_dtl == JOB_NULL)
			{
			    dsp_chr = 'X';
            }
			else
			{
			    /*-----------------------------------
			    | Find the LAST step of this JOB	|
			    | to see if it is 'L'ate yet!	|
			    -----------------------------------*/
			    tmp_list = stp_list->job_dtl->lst_stp;
			    tmp_date = tmp_list->st_date_time + tmp_list->duration;
			    tmp_date /= 1440L;
			    
			    if (tmp_date > stp_list->job_dtl->reqd_date && 
			        dsp_chr != ' ')
			    {
				    dsp_chr = 'L';
				}
			}

			dsp_chr = check_overlap (stp_list, tmp_dt_tm, dsp_chr);

			curr_chr = dsp_chr;
			curr_stp = (curr_chr == ' ') ? STP_NULL : stp_list;
			return;
	    }

	    tmp_dt_tm += time_resn;
	    while (1)
	    {
			if (tmp_dt_tm == stp_list->st_date_time && 
			    stp_list->duration == 0L)
            {
				break;
			}
			if (tmp_dt_tm >= (stp_list->st_date_time + stp_list->duration))
			{
			    stp_list = stp_list->nxt_job;
			    if (stp_list != STP_NULL)
				continue;
			}
			break;
	    }
	    col++;
	}
}

char
check_overlap (
 struct STP_LIST *stp_list, 
 long   tmp_dt_tm, 
 char   dsp_chr)
{
	struct STP_LIST *tmp_list;

	tmp_list = stp_list->nxt_job;
	while (tmp_list != STP_NULL && 
	       tmp_list->st_date_time < tmp_dt_tm + time_resn)
	{
	    if (tmp_list->st_date_time >= (stp_list->st_date_time + stp_list->duration))
	    {
			if (tmp_list->st_date_time + tmp_list->duration > tmp_dt_tm)
			{
			    if (dsp_chr != 'O')
			    {
	                dsp_chr = '*';
				}
			    if (tmp_list->job_dtl != JOB_NULL && 
			        tmp_list->job_dtl->is_hilite)
			    {
	                do_hilite = TRUE;
				}
			}
	    }
	    else if ((tmp_list->st_date_time + tmp_list->duration) > tmp_dt_tm)
		{
		    dsp_chr = 'O';
		    if (tmp_list->job_dtl != JOB_NULL && 
		        tmp_list->job_dtl->is_hilite)
            {		        
                do_hilite = TRUE;
			}
		}
		
	    tmp_list = tmp_list->nxt_job;
	    if (tmp_list == STP_NULL || tmp_list->st_date_time > tmp_dt_tm + time_resn)
	    {
			stp_list = stp_list->nxt_job;
			tmp_list = stp_list->nxt_job;
	    }
	}

	return (dsp_chr);
}


void
dsp_jdt (
 struct RSL_LIST *tmp_rsl)
{
	int    i,
	       step_no = 0,
	       step_tot = 0;
	struct JOB_LIST *job_ptr;
	struct STP_LIST *tmp_stp;

	move (0, 18);
	cl_end ();
	switch (curr_chr)
	{
	case	' ':
	case	'X':
		return;

	case	'O':
		print_at (18, 0, ML (mlPsMess010));
		return;

	case	'*':
		print_at (18, 0, ML (mlPsMess011));
		return;

	default:
		break;
	}

	for (i = 1, tmp_stp = curr_stp->job_dtl->fst_stp; 
	     tmp_stp != STP_NULL; 
	     tmp_stp = tmp_stp->nxt_res)
	{
		if (tmp_stp == curr_stp)
		{
			step_no = i;
		}
		step_tot = i;
		i++;
	}

	job_ptr = curr_stp->job_dtl;
	cc = find_hash (pcwo2, &pcwo_rec, EQUAL, "r", job_ptr->hhwo_hash);
	if (cc)
	{
		file_err (cc, pcwo, "DBFIND");
    }

	print_at (18, 0, 
	         "W/ORDER : %-7.7s      ITEM    : %-16.16s                            RESOURCE: %-8.8s            START DATE:   %s",
		      job_ptr->order_no,
              job_ptr->item_no,
              tmp_rsl->rgrs_ptr->rs_code,
              DateToString (curr_stp->st_date_time / 1440L));

	print_at (19, 0, 
	          "BATCH   : %-10.10s   DESC.   : %-40.40s    SEQUENCE: %3d                 START TIME:     %s",
	          pcwo_rec.wo_batch_no,
	          job_ptr->description,
	          curr_stp->seq_no,
	          ttoa (curr_stp->st_date_time % 1440L, "HHHHH:MM"));

	print_at (20, 0, 
	          "CREATED : %-10.10s   QUANTITY: %14.6f                              STEP    : %3d of %3d          SETUP     :     %s",
	          DateToString (job_ptr->create_date),
	          job_ptr->prod_qty,
	          step_no,
	          step_tot,
	          ttoa (curr_stp->setup, "HHHHH:MM"));

	print_at (21, 0, 
	          "REQUIRED: %-10.10s   BOM ALT.: %5d                                       RTG ALT.: %5d               RUN       :     %s",
	          DateToString (job_ptr->reqd_date),
	          job_ptr->bom_alt,
	          job_ptr->rtg_alt,
	          ttoa (curr_stp->run, "HHHHH:MM"));

	print_at (22, 0, 
	          "PRIORITY: %1d                                                                                                CLEAN     :     %s",
	          job_ptr->priority,
	          ttoa (curr_stp->clean, "HHHHH:MM"));
}

void
do_help (
 void)
{
	Dsp_open (0, 0, 14);

	Dsp_saverec ("                                            How to use LOGISTIC Production Scheduling.                                            ");
	Dsp_saverec ("");

	Dsp_saverec (" [REDRAW] [NEXT SCREEN]  [PREV SCREEN]  [INPUT/END]");

	Dsp_saverec (" POSITIONING THE CURSOR                                                                                               Page 1 of 3");
	Dsp_saverec ("     Use the arrow keys to move the cursor around the screen. ([NEXT] and [PREV] are available for PAGE-UP and PAGE-DOWN)");
	Dsp_saverec ("     The job details at the bottom of the screen will be dynamically updated to reflect the job under the cursor.");
	Dsp_saverec ("     When the cursor reaches a screen boundary, the screen will scroll in that direction to show further information.");
	Dsp_saverec ("");
	Dsp_saverec (" SYMBOLS AND THEIR MEANINGS");
	Dsp_saverec ("     ' ' - Signifies an empty timespace (ie: No jobs are scheduled within this timespace)");
	Dsp_saverec ("     '+' - Signifies the start of a job.");
	Dsp_saverec ("     '-' - Signifies the continuation of a job.");
	Dsp_saverec ("     '*' - Signifies that one job finishes and another job starts. Zoom-in for more in-depth detail.");
	Dsp_saverec ("     'O' - Signifies that there is an Overlap of at least two jobs within this timespace.");
	Dsp_saverec ("     'L' - Signifies that the job in this timespace is going to exceed it's required date. (Late)");
	Dsp_saverec ("     'X' - Signifies that this timespace is NOT available (as per the resource availability calendar)");
	Dsp_saverec ("");
	Dsp_saverec (" THE MENUS                                                                                                            Page 2 of 3");
	Dsp_saverec ("     To bring up the main menu, press CTRL-W.");
	Dsp_saverec ("     Menu entries can be selected by highlighting the entry and pressing <ENTER> or by pressing the key within the [ ].");
	Dsp_saverec ("     JOB MENU");
	Dsp_saverec ("         HILITE JOB    [H] - This allows you to visually see the related steps of a given job.");
	Dsp_saverec ("         CHANGE STEP   [C] - To enable you to modify the times associated with 1 step of a job.");
	Dsp_saverec ("         MOVE STEP     [M] - To enable you to re-schedule 1 step of a job.");
	Dsp_saverec ("         PACK JOBS     [P] - To 'squash together' the jobs where possible.");
	Dsp_saverec ("         VIEW JOBS     [V] - To textually display all of the jobs in this model.");
	Dsp_saverec ("         ADD JOB       [A] - To schedule a previously unscheduled job within this model.");
	Dsp_saverec ("         DELETE JOB    [D] - To remove a currently scheduled job from this model.");
	Dsp_saverec ("         SPLIT A STEP  [S] - To break 1 step into 2.");
	Dsp_saverec ("         JOIN 2 STEPS  [J] - To join together 2 previously 'split' steps.");
	Dsp_saverec ("");
	Dsp_saverec (" THE MENUS (Continued)                                                                                                Page 3 of 3");
	Dsp_saverec ("     SCREEN MENU");
	Dsp_saverec ("         ZOOM-IN       [O] - Selects a smaller time-resolution per character.");
	Dsp_saverec ("         ZOOM-OUT      [I] - Selects a larger time-resolution per character.");
	Dsp_saverec ("         PAGE-DOWN  [NEXT] - To dipslay the next screenfull of resources.");
	Dsp_saverec ("         PAGE-UP    [PREV] - To dipslay the previous screenfull of resources.");
	Dsp_saverec ("         1,2,...9          - To position the cursor at 1 of 9 possible places. (Eg: 7 = Top left, 5 = Centre...)");
	Dsp_srch ();
	Dsp_close ();
}

int
do_menu (
 struct gnt_mnu *mnu_ptr)
{
	int i = 1,
		j,
		s,
		v;

	crsr_off ();
	while (mnu_ptr[i].text != (char *) 0)
	{
		i++;
	}

	cl_box (0, 0, 32, i + 2);
	print_at (1, 1, ML (mlPsMess017));
	move (1, 2);
	line (31);
	s = 0;

	for (j = 0; j < i; j++)
	{
		print_at (j + 3, 1, 
		         (j == s) ? "%R %-28.28s " : " %-28.28s ", 
		         ML (mnu_ptr[j].text));
	}

	while (1)
	{
		j = getkey ();
		switch (j)
		{
		case	' ':
		case	RIGHT_KEY:
		case	DOWN_KEY:
			print_at (s + 3, 1, " %-28.28s ", ML (mnu_ptr[s].text));
			s++;
			if (s >= i)
			{
				s = 0;
			}
			print_at (s + 3, 1, "%R %-28.28s ", ML (mnu_ptr[s].text));
			break;

		case	8:
		case	LEFT_KEY:
		case	UP_KEY:
			print_at (s + 3, 1, " %-28.28s ", ML (mnu_ptr[s].text));
			s--;
			if (s < 0)
			{
				s = i - 1;
			}
			print_at (s + 3, 1, "%R %-28.28s ", ML (mnu_ptr[s].text));
			break;

		case	FN1:
			return (EXIT_SUCCESS);

		case	'\r':
		case	FN16:
			if (mnu_ptr[s].sub_mnu != GNT_MNU_NULL)
			{
				return (do_menu (mnu_ptr[s].sub_mnu));
			}
			if (mnu_ptr[s].ret_val != -1)
			{
				return (mnu_ptr[s].ret_val);
			}
			return ((* mnu_ptr[s].fn)(j));

		default:
			if (isalpha (j))
			{
				j = toupper (j);
			}
			for (v = 0; v < i; v++)
			{
				if (j != mnu_ptr[v].ret_val)
				{
					continue;
				}
				if (mnu_ptr[v].sub_mnu != GNT_MNU_NULL)
				{
					return (do_menu (mnu_ptr[v].sub_mnu));
				}
				if (mnu_ptr[v].ret_val != -1)
				{
					return (mnu_ptr[v].ret_val);
				}
				return ((* mnu_ptr[v].fn)(j));
			}
			putchar (BELL);
			fflush (stdout);
			break;
		};
	}
}

int
gnt_null_func (
 int x)
{
    /* 
        this is intentionally left blank.
    */
       
	return (EXIT_SUCCESS);
}

int	
change_step (
 void)
{
	static int scn_init_done = FALSE;
	long   new_date_time,
	       new_duration;

	if (curr_stp == STP_NULL || curr_stp->status == 'A' || (curr_chr != '-' && curr_chr != '+' && curr_chr != 'L'))
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}

	if (!scn_init_done)
	{
		SETUP_SCR (vars);
		set_masks ();
	}
	init_vars (1);
	time_rec.st_date	= curr_stp->st_date_time / 1440L;
	time_rec.st_time	= curr_stp->st_date_time % 1440L;
	time_rec.setup_time	= curr_stp->setup;
	time_rec.run_time	= curr_stp->run;
	time_rec.clean_time	= curr_stp->clean;
	restart = FALSE;
	max_prompt = 12;
	edit (1);

	if (!restart)
	{
		new_date_time		= (time_rec.st_date * 1440L) + 
		                       time_rec.st_time;
		if (time_rec.setup_time < 0L || 
		    time_rec.run_time < 0L || 
		    time_rec.clean_time < 0L)
        {		   
			return (FN3);
		}
		curr_stp->st_date_time	= new_date_time;
		curr_stp->setup		= rnd_time (time_rec.setup_time, min_resn);
		curr_stp->run		= rnd_time (time_rec.run_time, min_resn);
		curr_stp->clean		= rnd_time (time_rec.clean_time, min_resn);

		new_duration		= curr_stp->setup + 
		                      curr_stp->run	+ 
		                      curr_stp->clean;
		                      
		curr_stp->duration	= new_duration;
		re_schedule (curr_stp, rsl_curr, rsl_curr);
	}
	return (FN3);
}

int
hilite_job (
 void)
{
	if (curr_stp != STP_NULL && (curr_chr == '-' || curr_chr == '+' || curr_chr == 'L'))
	{
		curr_stp->job_dtl->is_hilite = !curr_stp->job_dtl->is_hilite;
		return (FN3);
	}
	else
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}
}

int	
move_step (
 void)
{
	struct RSL_LIST    *old_rsl = rsl_curr,
	                   *tmp_rsl;
	struct STP_LIST    *move_stp = curr_stp;
	int    i = 0,
	       old_y_pos,
	       do_redraw = 0;
	long   tmp_dt_tm;

	if (curr_stp == STP_NULL || 
	    curr_stp->status == 'A' || 
	    (curr_chr != '-' && curr_chr != '+' && curr_chr != 'L'))
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}
	
	tmp_dt_tm = move_stp->st_date_time;
	print_at (23, 0, mlPsMess012);
	move (x_pos + 10, y_pos + 2);

	cc = 0;
	while (i == 0)
	{
		cc = getkey ();
		switch (cc)
		{
		case	FN1:
		case	FN16:
		case	'\r':
		case	'\n':
			i = 1;
			break;

		case	FN3:
			old_stp = STP_NULL;	/* Force redraw of job info */
			do_redraw = 3;
			break;

		case	FN6:
			do_help ();
			old_stp = STP_NULL;	/* Force redraw of job info */
			do_redraw = 3;
			break;

		case	LEFT_KEY:
		case	8:
			if (x_pos)
			{
				x_pos--;
			}
			else
			{
				beg_dt_tm -= time_incs[indx];
				end_dt_tm = beg_dt_tm + (120L * time_resn);
				do_redraw = 1;
			}
			break;

		case	RIGHT_KEY:
			if (x_pos < 119)
			{
				x_pos++;
			}
			else
			{
				beg_dt_tm += time_incs[indx];
				end_dt_tm = beg_dt_tm + (120L * time_resn);
				do_redraw = 1;
			}
			break;

		case	UP_KEY:
			tmp_rsl = rsl_curr;
			old_y_pos = y_pos;
			if (y_pos)
			{
				rsl_curr = rsl_curr->prv_rsl;
				y_pos--;
			}
			else
			{
				if (scn_head->prv_rsl == RSL_NULL)
				{
					putchar (BELL);
				}
				else
				{
					scn_head = rsl_curr = scn_head->prv_rsl;
					do_redraw = 2;
				}
			}
			if (rsl_curr->rgrs_ptr != tmp_rsl->rgrs_ptr)
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				putchar (BELL);
			}
			if (rsl_curr != tmp_rsl &&
				move_stp->prv_res != STP_NULL &&
				move_stp->prv_res->seq_no == move_stp->seq_no &&
				move_stp->prv_res->line_no == move_stp->line_no &&
				move_stp->can_split[0] != 'Y')
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				print_at (23, 0, ML (mlPsMess013));
				sleep (sleepTime);
				print_at (23, 0, ML (mlPsMess012));
				putchar (BELL);
				break;
			}
			
			if (rsl_curr != tmp_rsl &&
				move_stp->nxt_res != STP_NULL &&
				move_stp->nxt_res->seq_no == move_stp->seq_no &&
				move_stp->nxt_res->line_no == move_stp->line_no &&
				move_stp->can_split[0] != 'Y')
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				print_at (23, 0, ML (mlPsMess013));
				sleep (sleepTime);
				print_at (23, 0, ML (mlPsMess012));
				putchar (BELL);
				break;
			}
			break;

		case	DOWN_KEY:
			tmp_rsl = rsl_curr;
			old_y_pos = y_pos;
			if (y_pos < 14)
			{
				if (rsl_curr->nxt_rsl != RSL_NULL)
				{
					rsl_curr = rsl_curr->nxt_rsl;
					y_pos++;
				}
				else
				{
					putchar (BELL);
				}
			}
			else
			{
				if (rsl_curr->nxt_rsl == RSL_NULL)
				{
					putchar (BELL);
				}
				else
				{
					scn_head = scn_head->nxt_rsl;
					rsl_curr = rsl_curr->nxt_rsl;
					do_redraw = 2;
				}
			}

			if (rsl_curr->rgrs_ptr != tmp_rsl->rgrs_ptr)
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				putchar (BELL);
			}

			if (rsl_curr != tmp_rsl &&
				move_stp->prv_res != STP_NULL &&
				move_stp->prv_res->seq_no == move_stp->seq_no &&
				move_stp->prv_res->line_no == move_stp->line_no &&
				move_stp->can_split[0] != 'Y')
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				print_at (23, 0, ML (mlPsMess013));
				sleep (sleepTime);
				print_at (23, 0, ML (mlPsMess012));
				putchar (BELL);
				break;
			}

			if (rsl_curr != tmp_rsl &&
				move_stp->nxt_res != STP_NULL &&
				move_stp->nxt_res->seq_no == move_stp->seq_no &&
				move_stp->nxt_res->line_no == move_stp->line_no &&
				move_stp->can_split[0] != 'Y')
			{
				rsl_curr = tmp_rsl;
				y_pos = old_y_pos;
				print_at (23, 0, ML (mlPsMess013));
				sleep (sleepTime);
				print_at (23, 0, ML (mlPsMess012));
				putchar (BELL);
				break;
			}
			break;

		default:
			putchar (BELL);
			break;

		};  /* end of switch (cc) */

		if (i != 0)
		{
			break;
		}

		crsr_off ();
		if (do_redraw)
		{
			time_resn = act_times[indx];
			end_dt_tm = beg_dt_tm % time_incs[indx];
			beg_dt_tm -= end_dt_tm;
			if ((end_dt_tm * 2) > time_incs[indx])
			{
				beg_dt_tm += time_incs[indx];
			}
			end_dt_tm = beg_dt_tm + (120L * time_resn);
			prep_scn (scn_head, do_redraw);
		}
		else
		{
			get_job_info (rsl_curr->fst_stp);
		}
		tmp_dt_tm = beg_dt_tm + (time_resn * (long) x_pos);

		print_at (17, 62, 
		          " %-10.10s %s ",
		          DateToString (tmp_dt_tm / 1440L),
		          ttoa ((tmp_dt_tm % 1440L), "NN:NN"));

		if (old_stp != curr_stp || old_chr != curr_chr)
		{
			dsp_jdt (rsl_curr);
		}
		old_stp = curr_stp;
		old_chr = curr_chr;
		print_at (23, 0, ML (mlPsMess012));
		move (x_pos + 10, y_pos + 2);
		fflush (stdout);
		crsr_on ();
		do_redraw = 0;

	}; /* end of while (i == 0) */
	
	move_stp->st_date_time = tmp_dt_tm;
	re_schedule (move_stp, old_rsl, rsl_curr);
	return (FN3);
}

void
re_schedule (
 struct STP_LIST    *step, 
 struct RSL_LIST    *src_rsl, 
 struct RSL_LIST    *dst_rsl)
{
	struct STP_LIST    *tmp_stp;

	/*-------------------------------
	| Firstly, take it OUT of this  |
	| resource list altogether      |
	-------------------------------*/
	if (step->prv_job != STP_NULL)
	{
		step->prv_job->nxt_job = step->nxt_job;
	}

	if (step->nxt_job != STP_NULL)
	{
		step->nxt_job->prv_job = step->prv_job;
	}

	if (src_rsl->fst_stp == step)
	{
		src_rsl->fst_stp = step->nxt_job;
	}

	if (src_rsl->lst_stp == step)
	{
		src_rsl->lst_stp = step->prv_job;
	}

	if (src_rsl->fst_job == step)
	{
		if (src_rsl->lst_job == step)
		{
			src_rsl->fst_job = STP_NULL;
			src_rsl->lst_job = STP_NULL;
		}
		else
		{
			for (tmp_stp = step->nxt_job; 
			     tmp_stp != STP_NULL && tmp_stp->job_dtl == JOB_NULL;)
            {
                tmp_stp = tmp_stp->nxt_job;
            }
			src_rsl->fst_job = tmp_stp;
		}
	}

	if (src_rsl->lst_job == step)
	{
		for (tmp_stp = step->prv_job; 
		     tmp_stp != STP_NULL && tmp_stp->job_dtl == JOB_NULL;)
        {
            tmp_stp = tmp_stp->prv_job;
		}
		src_rsl->lst_job = tmp_stp;
	}
	upd_lst_dt_tm (src_rsl);

	/*-------------------------------
	| Secondly, re-insert the step	|
	| into then destination lists.	|
	-------------------------------*/
	tmp_stp = dst_rsl->fst_stp;
	if (tmp_stp == STP_NULL)
	{
		dst_rsl->fst_stp = step;
		dst_rsl->lst_stp = step;
		dst_rsl->fst_job = step;
		dst_rsl->lst_job = step;
		step->nxt_job = STP_NULL;
		step->prv_job = STP_NULL;
		upd_lst_dt_tm (dst_rsl);
		return;
	}

	if (step->st_date_time < dst_rsl->fst_job->st_date_time)
	{
		dst_rsl->fst_job = step;
	}

	if (step->st_date_time > dst_rsl->lst_job->st_date_time)
	{
		dst_rsl->lst_job = step;
	}

	while (tmp_stp != STP_NULL && 
	      tmp_stp->st_date_time < step->st_date_time)
    {
        tmp_stp = tmp_stp->nxt_job;
    }
          
	if (tmp_stp == STP_NULL)
	{
		step->prv_job = dst_rsl->lst_stp;
		step->nxt_job = STP_NULL;
		dst_rsl->lst_stp->nxt_job = step;
		dst_rsl->lst_stp = step;
		upd_lst_dt_tm (dst_rsl);
		return;
	}
	step->nxt_job = tmp_stp;
	step->prv_job = tmp_stp->prv_job;
	tmp_stp->prv_job = step;
	if (step->prv_job != STP_NULL)
	{
		step->prv_job->nxt_job = step;
	}
	else
	{
		dst_rsl->fst_stp = step;
	}	
	upd_lst_dt_tm (dst_rsl);

	return;
}

void
pack_jobs (
 void)
{
    /* QUERY 
     * why is this function empty?
     */
}

int	
join_step (
 void)
{
	struct RSL_LIST    *tmp_rsl;
	struct STP_LIST    *old_stp,
	                   *joi_stp = curr_stp;

    if (joi_stp == STP_NULL ||
		joi_stp->status == 'A' ||
		joi_stp->can_split[0] == 'N' ||
		(curr_chr != '-' && curr_chr != '+' && curr_chr != 'L'))
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}

	old_stp = joi_stp->nxt_job;
	while (old_stp != STP_NULL && old_stp->job_dtl == JOB_NULL)
	{
		old_stp = joi_stp->nxt_job;
	}

	if (old_stp == STP_NULL ||
		old_stp->seq_no != joi_stp->seq_no ||
		old_stp->line_no != joi_stp->line_no ||
		old_stp->job_dtl != joi_stp->job_dtl)
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}

	joi_stp->run += old_stp->run;
	joi_stp->run = rnd_time (joi_stp->run, min_resn);
	joi_stp->clean = old_stp->clean;
	joi_stp->clean = rnd_time (joi_stp->clean, min_resn);
	joi_stp->duration = joi_stp->setup + joi_stp->run + joi_stp->clean;
	joi_stp->nxt_res = old_stp->nxt_res;

	if (old_stp->nxt_res != STP_NULL)
	{
		old_stp->nxt_res->prv_res = joi_stp;
	}

	tmp_rsl = joi_stp->rsl_ptr;
	old_stp->prv_job->nxt_job = old_stp->nxt_job;

	if (old_stp->nxt_job != STP_NULL)
	{
		old_stp->nxt_job->prv_job = old_stp->prv_job;
	}
	else
	{
		tmp_rsl->lst_stp = old_stp->prv_job;
	}

	if (tmp_rsl->lst_job == old_stp)
	{
		tmp_rsl->lst_job = joi_stp;
	}

	stp_node (FREE_ND, old_stp);

	re_schedule (joi_stp, tmp_rsl, tmp_rsl);
	return (FN3);
}

int	
split_step (
 void)
{
	struct RSL_LIST    *tmp_rsl;
	struct STP_LIST    *new_stp,
	                   *spl_stp = curr_stp;
	long   new_run,
	       tmp_dt_tm;

	if (spl_stp == STP_NULL ||
		spl_stp->status == 'A' ||
		spl_stp->can_split[0] == 'N' ||
		spl_stp->run < (min_resn * 2) ||
		(curr_chr != '-' && curr_chr != '+' && curr_chr != 'L'))
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}

	tmp_dt_tm = beg_dt_tm + ((long) x_pos * time_resn);

	new_run = spl_stp->st_date_time + 
	          spl_stp->setup + 
	          spl_stp->run - tmp_dt_tm;

	new_run = rnd_time (new_run, min_resn);
	if (new_run <= 0L || 
	    new_run >= spl_stp->run)
	{
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);
	}

	tmp_rsl = spl_stp->rsl_ptr;

	new_stp = stp_node (ALLC_ND, STP_NULL);

	spl_stp->run -= new_run;

	spl_stp->duration = spl_stp->setup + 
	                    spl_stp->run + 
	                    spl_stp->clean;

	new_stp->nxt_res = spl_stp->nxt_res;
	spl_stp->nxt_res = new_stp;
	new_stp->prv_res = spl_stp;
	new_stp->nxt_job = STP_NULL;
	new_stp->prv_job = STP_NULL;
	new_stp->job_dtl = spl_stp->job_dtl;
	new_stp->rsl_ptr = spl_stp->rsl_ptr;
	new_stp->seq_no = spl_stp->seq_no;
	new_stp->line_no = spl_stp->line_no;
	new_stp->st_date_time = spl_stp->st_date_time + spl_stp->duration;
	new_stp->setup = spl_stp->setup;
	new_stp->run = new_run;
	new_stp->clean = spl_stp->clean;
	new_stp->duration = new_stp->setup + 
	                    new_stp->run + 
	                    new_stp->clean;

	strcpy (new_stp->can_split, spl_stp->can_split);
	new_stp->status = spl_stp->status;

	re_schedule (spl_stp, tmp_rsl, tmp_rsl);
	re_schedule (new_stp, tmp_rsl, tmp_rsl);
	return (FN3);
}

/*=======================================
| This routines purpose is to check the	|
| linked list's consistency.            |
=======================================*/
int
valid_mdl (
 void)
{
	struct JOB_LIST    *tmp_job;
	struct RSL_LIST    *tmp_rsl;
	struct STP_LIST    *tmp_stp;

	int    save_err = 0,
	       save_seq_no;

	long   prv_dt_tm,
	       tmp_dt_tm,
	       new_dt_tm;

	for (tmp_job = job_head; 
	     tmp_job != JOB_NULL; 
	     tmp_job = tmp_job->nxt_job)
	{
	    save_seq_no = 0;
	    prv_dt_tm = 0L;
	    new_dt_tm = 0L;

	    for (tmp_stp = tmp_job->fst_stp; 
	         tmp_stp != STP_NULL; 
	         tmp_stp = tmp_stp->nxt_job)
	    {
			if (save_seq_no != tmp_stp->seq_no)
			{
			    prv_dt_tm = new_dt_tm;
			    save_seq_no = tmp_stp->seq_no;
			}

			tmp_dt_tm = tmp_stp->st_date_time + tmp_stp->duration;

			if (tmp_dt_tm > new_dt_tm)
			{
			    new_dt_tm = tmp_dt_tm;
            }

			if (tmp_stp->status == 'A')
			{
			    continue;
            }
            
			if (tmp_stp->st_date_time < prv_dt_tm)
			{
			    save_err |= ORDER;
            }

			if (tmp_stp->nxt_job == STP_NULL)
			{
				tmp_dt_tm = (tmp_job->reqd_date + 1L) * 1440L;
				if (tmp_stp->st_date_time + tmp_stp->duration > tmp_dt_tm)
				{
					save_err |= LATE;
				}
			}
	    } /* end of inner for loop */
	} /* end of outer for loop */

	for (tmp_rsl = rsl_head; 
	     tmp_rsl != RSL_NULL; 
	     tmp_rsl = tmp_rsl->nxt_rsl)
	{
	    prv_dt_tm = 0L;
	    for (tmp_stp = tmp_rsl->fst_stp; 
	         tmp_stp != STP_NULL && tmp_stp->nxt_job != STP_NULL; 
	         tmp_stp = tmp_stp->nxt_job)
	    {
			if (tmp_stp->st_date_time < prv_dt_tm)
			{
			    return (save_err | OVERLAP);
            }
			prv_dt_tm = tmp_stp->st_date_time;
	    }
	}

	return (save_err);
}

/* [ end of file ] */

/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_admail.c 	)                             |
|  Program Desc  : ( Logistic Software Mail Admin System.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmre, pmse, pmtx,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmre, pmse, pmtx,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (19/07/91)      | Author       : Campbell Mander   |
|---------------------------------------------------------------------|
|  Date Modified : (23/10/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (20/11/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (22/11/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (08/12/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (08/02/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (21/09/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/09/97)      | Modified  by : Roanna Marcelino. |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (23/10/91) - Fixed problem with deletion.          |
|  (20/11/91)    : Added Update Statistics option.                    |
|  (22/11/91)    : Added confirmation of delete date.                 |
|  (05/12/91)    : Update for cumr phone_no length. SC 6297 PSL.      |
|  (08/12/92)    : Mods and fixes as per SC 8232 PSL.                 |
|  (08/02/93)    : Remove last remnants of FULL pathnames. PSL 8431.  |
|  (21/09/93)    : HGP 9864. Increase cont_no to 15 chars.            |
|  (05/09/97)    : Modified for Multilingual Conversion.              |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: psl_admail.c,v $
| Revision 5.3  2002/07/17 09:57:24  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 05:13:40  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:33  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/09/07 02:31:22  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.1  2000/09/06 07:49:27  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:00:25  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 09:41:59  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/09/29 10:11:12  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:27:05  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 02:36:53  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: psl_admail.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_admail/psl_admail.c,v 5.3 2002/07/17 09:57:24 scott Exp $";

#define		X_OFF		0
#define		Y_OFF		0

#define		TXT_REQD
#include 	<pslscr.h>
#include 	<hot_keys.h>
#include 	<get_lpno.h>
#include 	<sys/types.h>
#include 	<utmp.h>
#include 	<minimenu.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>
#include	<tabdisp.h>

#define		UPDT	0
#define		UNLK	1

#define	ACTIONS		( !DETAILS)
#define USR_NULL 	( (struct USR_PTR *) NULL  )
#define READ_NULL 	( (struct READ_PTR *) NULL )

#define	NOT_ACTIVE	(pmre_rec.re_act_date > local_rec.lsystemDate || \
		  	 (pmre_rec.re_act_date == local_rec.lsystemDate && \
		  	  strcmp(pmre_rec.re_act_time, curr_time) > 0) )

extern	int	_mail_ok;

FILE	*fin;
FILE	*fout;

	int	clear_ok = TRUE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	t_dbt_date;
	} comm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_phone_no"},
		{"cumr_contact_name"},
	};

	int	cumr_no_fields = 9;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_phone_no[16];
		char	cm_contact_name[21];
	} cumr_rec, cumr2_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview sumr_list[] ={
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
		{"sumr_acc_type"},
		{"sumr_debtor_no"},
		{"sumr_cont_no"},
		{"sumr_cont_name"},
	};

	int	sumr_no_fields = 10;

	struct	{
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_name[41];
		char	sm_acronym[10];
		char	sm_acc_type[2];
		char	sm_debtor_no[13];
		char	sm_cont_no[16];
		char	sm_cont_name[21];
	} sumr_rec, sumr2_rec;

	/*==================================
	| Customer Service note pad detail |
	==================================*/
	struct dbview pmre_list[] ={
		{"pmre_call_no"},
		{"pmre_receiver"},
		{"pmre_status"},
		{"pmre_seen"},
		{"pmre_cc_flag"},
		{"pmre_active_date"},
		{"pmre_active_time"},
		{"pmre_fst_date"},
		{"pmre_fst_time"},
		{"pmre_lst_date"},
		{"pmre_lst_time"},
	};

	int	pmre_no_fields = 11;

	struct	{
		long	re_call_no;
		char	re_receiver[15];
		char	re_status[2];
		char	re_seen[2];
		char	re_cc_flag[2];
		long	re_act_date;
		char	re_act_time[6];
		long	re_fst_date;
		char	re_fst_time[6];
		long	re_lst_date;
		char	re_lst_time[6];
	} pmre2_rec, pmre_rec;

	/*==================================
	| Customer Service note pad header |
	==================================*/
	struct dbview pmse_list[] ={
		{"pmse_call_no"},
		{"pmse_nx_seq"},
		{"pmse_termno"},
		{"pmse_sender"},
		{"pmse_subject"},
		{"pmse_priority"},
		{"pmse_link_type"},
		{"pmse_link_hash"},
		{"pmse_phone_no"},
		{"pmse_contact_name"},
		{"pmse_date"},
		{"pmse_time"},
		{"pmse_status"},
		{"pmse_stat_flag"},
	};

	int	pmse_no_fields = 14;

	struct	{
		long	se_call_no;
		int	se_nx_seq;
		int	se_termno;
		char	se_sender[15];
		char	se_subject[41];
		int	se_priority;
		char	se_link_type[2];
		long	se_link_hash;
		char	se_phone_no[16];
		char	se_contact_name[26];
		long	se_date;
		char	se_time[9];
		char	se_status[2];
		char	se_stat_flag[2];
	} pmse_rec;

	/*==================================
	| Customer Service note pad header |
	==================================*/
	struct dbview pmtx_list[] ={
		{"pmtx_call_no"},
		{"pmtx_seq_no"},
		{"pmtx_sender"},
		{"pmtx_mail_type"},
		{"pmtx_line_no"},
		{"pmtx_date"},
		{"pmtx_time"},
		{"pmtx_text"},
	};

	int	pmtx_no_fields = 8;

	struct	{
		long	tx_call_no;
		int	tx_seq_no;
		char	tx_sender[15];
		char	tx_mail_type[2];
		int	tx_line_no;
		long	tx_date;
		char	tx_time[9];
		char	tx_text[61];
	} pmtx_rec;

	/*============================
	| Logistic Mail Control File |
	============================*/
	struct dbview pmct_list[] ={
		{"pmct_system_name"},
		{"pmct_date_lpurge"},
		{"pmct_time_lpurge"},
		{"pmct_date_lupd"},
		{"pmct_time_lupd"},
		{"pmct_stat_flag"},
	};

	int	pmct_no_fields = 6;

	struct	{
		char	ct_system_name[41];
		long	ct_date_lpurge;
		char	ct_time_lpurge[9];
		long	ct_date_lupd;
		char	ct_time_lupd[9];
		char	ct_stat_flag[2];
	} pmct_rec;

	/*===================================
	| Logistic Mail History Header File |
	===================================*/
	struct dbview pmhs_list[] ={
		{"pmhs_call_no"},
		{"pmhs_sender"},
		{"pmhs_priority"},
		{"pmhs_link_type"},
		{"pmhs_link_hash"},
		{"pmhs_date"},
		{"pmhs_time"},
		{"pmhs_stat_flag"},
	};

	int	pmhs_no_fields = 8;

	struct	{
		long	hs_call_no;
		char	hs_sender[15];
		int	hs_priority;
		char	hs_link_type[2];
		long	hs_link_hash;
		long	hs_date;
		char	hs_time[9];
		char	hs_stat_flag[2];
	} pmhs_rec;

	/*========================================
	| Logistic Mail History Line Detail File |
	========================================*/
	struct dbview pmhl_list[] ={
		{"pmhl_call_no"},
		{"pmhl_receiver"},
		{"pmhl_seen"},
		{"pmhl_active_date"},
		{"pmhl_active_time"},
		{"pmhl_fst_rd_date"},
		{"pmhl_fst_rd_time"},
		{"pmhl_date_action"},
		{"pmhl_time_action"},
	};

	int	pmhl_no_fields = 9;

	struct	{
		long	hl_call_no;
		char	hl_receiver[15];
		char	hl_seen[2];
		long	hl_active_date;
		char	hl_active_time[6];
		long	hl_fst_rd_date;
		char	hl_fst_rd_time[6];
		long	hl_date_action;
		char	hl_time_action[9];
	} pmhl_rec;

	MENUTAB main_menu [] =
	{
		{ "   1. Maintain Mail.              ", "" },
		{ "   2. Update Statistics.          ", "" },
		{ "   3. Delete Mail.                ", "" },
		{ "   4. Mail Reports.               ", "" },
		{ "   5. Send Message.               ", "" },
		{ "   6. Exit Mail System.           ", "" },
		{ ENDMENU }
	};

	MENUTAB rept_menu [] =
	{
		{ "   1. Return to Main Menu.        ", "" },
		{ "   2. Read/Action Delay. (Normal) ", "" },
		{ "   3. Read/Action Delay. (Sup/Cst)", "" },
		{ "   4. Aged Active Mail Report.    ", "" },
		{ "   5. Mail Summary Statistics.    ", "" },
		{ ENDMENU }
	};

	char	*data  = "data",
			*comm  = "comm",
	    	*cumr  = "cumr",
	    	*cumr2 = "cumr2",
	    	*sumr  = "sumr",
	    	*sumr2 = "sumr2",
	    	*pmse  = "pmse",
	    	*pmre  = "pmre",
	    	*pmre2 = "pmre2",
	    	*pmtx  = "pmtx",
	    	*pmct  = "pmct",
	    	*pmhs  = "pmhs",
	    	*pmhl  = "pmhl";

/*============================ 
| Local & Screen Structures. |
============================*/
int	DETAILS;	/* TRUE if user currently viewing details	*/
int	MORE_MAIL;	/* TRUE if there is more mail to view 		*/
int	read_ok;	/* TRUE if ok to read mail chosen in choose function */
int	mail_maint;	/* TRUE if maintaining mail */
int	no_tagged;
int	exit_loop;
int	exit_choose;
int	rdrw_txt = FALSE;
long	read_call;	/* Call number to be read			*/
char	log_name[15];
char	usr_fname[100];
char	curr_status[2];
char 	tagged_name[350];
char	cmd[256];

struct	{
	char	dummy[11];
	char	sys_time[9];
	char	systemDate[11];
	long	lsystemDate;
	char	from[15];
	char	to[15];
	char	CC[61];
	char	subject[41];
	int	priority;
	char	dly_mail[4];
	long	dly_date;
	char	dly_time[6];
	char	mail_src[9];
	char	src_no[7];
	int	link_hash;
	char	src_no_desc[43];
	char	phone_no[16];
	char	contact_name[26];
	char	dflt_phone_no[16];
	char	dflt_contact_name[26];
	char	call_dtls[76];
	char	call_no[7];
	char	src_prmt[32];
	char	date_prmt[32];
	char	rcvd_date[19];
	char	message[65];
	char	text[61];
	long	del_date;
} local_rec;

struct	READ_PTR
{
	long	call_no;
	int		priority;
	char	sender [16];
	char	subject [46];
	char	status [9];
	struct	READ_PTR *	next;
};

struct	USR_PTR
{
	char	usr_name [15];
	struct	USR_PTR	*	next;
};

/*---------------------------------------
| Three linked lists are used 			|
| 1) Lists of users from Mail_secure	|
| 2) List of currently logged in users	|
| 3) Lists of users to send mail to		|
---------------------------------------*/
struct  USR_PTR *usr_head;
struct  USR_PTR *usr_curr;
struct  USR_PTR *log_head = USR_NULL;
struct  USR_PTR *log_curr;
struct  USR_PTR *snd_head;
struct  USR_PTR *snd_curr;

struct	READ_PTR	*read_ptr;
struct	READ_PTR	*read_head = READ_NULL;

struct	utmp	*uptr;

static	int	user_tag_func	(int, KEY_TAB *);
static	int	exit_func		(int, KEY_TAB *);
static	int	read_slct_func	(int, KEY_TAB *);
static	int	show_func		(int, KEY_TAB *);
static	int	details_func	(int, KEY_TAB *);
static	int	actions_func	(int, KEY_TAB *);
static	int	mail_prnt_func	(int, KEY_TAB *);
static	int	ed_txt_scrn		(int, KEY_TAB *);
static	int	dummy_func		(int, KEY_TAB *);

char	*psl_path;

static	KEY_TAB user_keys [] =
{
   { "[T]AG/UNTAG",	'T', user_tag_func,
	"Tag/Untag current user for CC.",				"A" },
   { NULL,		'\r', user_tag_func,
	"Tag/Untag current user for CC.",				"A" },
   { NULL,		FN16, exit_func,
	"Selection of users complete.",					"A" },
   END_KEYS
};

static	KEY_TAB read_keys [] =
{
   { "[S]HOW ALL USERS",'S', show_func,
	"Show list of users that mail was sent to.",			"A" },
   { NULL,		'\r', read_slct_func,
	"Select piece of mail for reading.",				"A" },
   { NULL,		FN1,  dummy_func,
	"",								"A" },
   { NULL,		FN16, exit_func,
	"Exit from current option.",					"A" },
   END_KEYS
};

static KEY_TAB show_keys [] =
{
   END_KEYS
};

static	KEY_TAB text_keys [] =
{
   { "[D]ETAILS",	'D', details_func,
	"View details for current mail.",				"A" },
   { "[A]CTIONS",	'A', actions_func,
	"View actions for current mail.",				"A" },
   { "FN05 - PRINT",    FN5, mail_prnt_func,
	"Print current mail.",						"A" },
   { "FN13 - EDIT",    FN13, ed_txt_scrn,
	"Edit Current Text Screen.",					"D" },
   { NULL,		FN16, exit_func,
	"",								"A" },
   { NULL,		FN1, dummy_func,
	"",								"A" },
   END_KEYS
};

static	struct	var	vars[] =
{
	{1, LIN, "logname",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", " Users Logname: ", "Enter logname of user whose mail to maintain.",
		YES, NO,  JUSTLEFT, "", "", log_name},

	{2, LIN, "call_no",	 2, 15, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "Call Number  :", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.call_no},
	{2, LIN, "r_from",	 3, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "From         :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.from},
	{2, LIN, "r_priority",	 3, 60, INTTYPE,
		"N", "          ",
		" ", "5", "Priority :", " 1 - 5  (1 = Urgent action required) ",
		 NO, NO,  JUSTLEFT, "1", "5", (char *)&local_rec.priority},
	{2, LIN, "r_subject",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Subject      :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.subject},
	{2, LIN, "CC",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Add To CC List:", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.CC},
	{2, LIN, "r_src_no",	 7, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", local_rec.src_prmt, "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_no},
	{2, LIN, "r_src_no_desc",	 7, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_no_desc},
	{2, LIN, "r_phone_no",	 8, 15, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Phone No   :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.phone_no},
	{2, LIN, "r_contact_name",	 8, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact Name :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.contact_name},
	{2, LIN, "rcvd_date",	 9, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.date_prmt, "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rcvd_date},

	{3, TXT, "text",	12, 9, 0,
		"", "          ",
		" ", "", " Edit Text ", "",
		8, 60, 500, "", "", local_rec.text},

	{4, LIN, "mess_to",	19, 11, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Send To: ", "Send message to whom. ALL USERS for a global message. ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.to},
	{4, LIN, "message",	20, 11, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Message: ", "Message to send.",
		YES, NO,  JUSTLEFT, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-'`:;?/<>{} !@#$%^&*()\\+~=[]|_", "", local_rec.message},

	{5, LIN, "del_date",	21, 13, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Delete Date: ", "Purge database of all deleted mail received before this date.",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.del_date},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};


#include <FindCumr.h>
#include <FindSumr.h>

/*============================
| Local function prototypes  |
============================*/
void	mail_opt_menu	(void);
void	proc_repts		(void);
int		run_prog		(char *);
void	draw_menu		(int);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
int		send_message	(void);
int		upd_stats		(void);
int		upd_pmhl		(void);
int		read_pmct		(int);
int		check_actions	(void);
int		del_mail		(void);
int		maintain_mail	(void);
int		choose_mail		(void);
int		get_logname		(void);
int		clr_read_list	(struct READ_PTR *);
int		load_mail		(void);
int		load_text		(int);
int		add_to_CC		(void);
int		opt_ring_menu	(void);
int		chng_stat		(char *);
int		chk_mail		(void);
int		all_deleted		(long);
int		chk_user		(char *, int);
int		chk_CC_user		(char *);
int		srch_user		(char *, int);
int		srch_CC			(char *);
int		chk_not_CC		(char *);
int		get_all_users	(void);
struct USR_PTR *	usr_alloc	(void);
struct READ_PTR *	read_alloc	(void);
void	prnt_text		(char *);
int		add_text		(void);
int		create_send_list(void);
int		has_actions		(void);
int		fix_edge		(int);
int		heading			(int);
int		erase_scn		(int);
int		mail_notify		(char *);

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

	_mail_ok = FALSE;

	if (getuid() != 0)
	{
		print_mess (ML (mlMenuMess109));
		sleep (sleepTime);
		clear_mess ();
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	psl_path = getenv("PROG_PATH");
	sprintf(usr_fname, "%s/BIN/MENUSYS/Mail_secure", 
				(psl_path) ? psl_path : "/usr/LS10.5");

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	tab_row = 11;
	tab_col = 8;

	SETUP_SCR(vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/

	clear ();
	draw_menu (TRUE);

	set_help (FN6, "FN6");

	/*------------------------------
	| Open main database files.    |  
	------------------------------*/
	OpenDB ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	fl_pr("   PLEASE WAIT  ", 29, 12, 1);

	/*---------------------------
	| Check Control File Record |  
	---------------------------*/
	if (!read_pmct (UNLK))
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	/*-----------------------------
	| Load users from Mail_secure |
	-----------------------------*/
	get_all_users ();

	/*-----------------------------
	| Allow user to select option |
	-----------------------------*/
	mail_opt_menu ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===================
| Main Option Menu. |
===================*/
void
mail_opt_menu (
 void)
{
	int	exit_loop = FALSE;

	MENU_ROW = 8;
	MENU_COL = 21;

	draw_menu (FALSE);
	
	while (exit_loop == FALSE)
	{
		crsr_off();
		mmenu_print( (char *)0, main_menu, 0);

		mail_maint = FALSE;
		switch ( mmenu_select(main_menu) )
		{
			case 0 :
				/*---------------
				| Maintain Mail |
				---------------*/
				mail_maint = TRUE;
				maintain_mail ();
				clear();
				draw_menu (FALSE);
				break;

			case 1:
				/*---------------------------
				| Update Mail History Files |
				---------------------------*/
				upd_stats ();
				clear();
				draw_menu (FALSE);
				break;
	
			case 2 :
				/*---------------------------
				| Update Mail History Files |
				| Delete Mail Flagged 'D'   |
				---------------------------*/
				upd_stats ();
				del_mail ();
				clear();
				draw_menu (FALSE);
				break;

			case 3 :
				proc_repts();
				draw_menu (FALSE);
				break;
	
			case 4:
				send_message ();
				clear();
				draw_menu (FALSE);
				break;

			case 5:
			case 99:
			case -1:
				exit_loop = TRUE;	
				break;
		
			default :
				break;
		}
	}
}

/*-------------------
| Run reports menu. |
-------------------*/
void
proc_repts (
 void)
{
	int	exit_loop = FALSE;

	if (local_rec.lsystemDate > pmct_rec.ct_date_lupd ||
	    (local_rec.lsystemDate == pmct_rec.ct_date_lupd &&
	    strcmp(local_rec.sys_time, pmct_rec.ct_time_lupd) > 0))
	{
		/*WARNING: Stats May Be Out Of Date. Last Updated %-10.10s at %-8.8s*/
		sprintf(err_str, ML(mlMenuMess119),
				DateToString(pmct_rec.ct_date_lupd),
				pmct_rec.ct_time_lupd);
		print_mess(err_str);
		sleep(3);
		clear_mess();
	}

	while (exit_loop == FALSE)
	{
		rv_pr("                                      ", 20, 15, 0);
		crsr_off();
		mmenu_print( (char *)0, rept_menu, 0);

		switch ( mmenu_select( rept_menu ) )
		{
			case 0:
			case 99:
			case -1:
				exit_loop = TRUE;	
				break;
			case 1:
				run_prog ("ml_ndlyrpt");
				clear();
				draw_menu (FALSE);
				break;
				break;

			case 2:
				run_prog ("ml_sdlyrpt");
				clear();
				draw_menu (FALSE);
				break;

			case 3 :
				run_prog ("ml_active");
				clear();
				draw_menu (FALSE);
				break;

			case 4 :
				run_prog ("ml_sumrpt");
				clear();
				draw_menu (FALSE);
				break;

			default :
				break;
		}
	}
}

int
run_prog (
 char *	prog_name)
{
	if (fork() == 0)
	{
		execlp (prog_name, prog_name, (char *)0);
		return (EXIT_SUCCESS);
	}
	else
		wait (0);

	return (EXIT_SUCCESS);
}

/*----------------
| Draw main menu |
----------------*/
void
draw_menu (
 int	flag)
{
	char	*xx_str = ": ..................................";

	crsr_off();
	box(4,2,70,17);
	box(9,5,60,11);
	box(60,6,7,2);
	rv_pr("PSL", 62, 7, 0);
	rv_pr("70c", 62, 8, 0);

	us_pr(ML(mlMenuMess205), 20,3,1);
	if (flag)
	{
		rv_pr(xx_str, 21, 9,0);
		rv_pr(xx_str, 21, 10,0);
		rv_pr(xx_str, 21, 11,0);
		rv_pr(xx_str, 21, 12,0);
		rv_pr(xx_str, 21, 13,0);
		rv_pr(xx_str, 21, 14,0);
	
		box(27,11,20,1);
		crsr_off();
	}
	else
	{
		sprintf(err_str, ML(mlMenuMess127),
				DateToString(pmct_rec.ct_date_lpurge),
				pmct_rec.ct_time_lpurge);
		rv_pr(err_str, 12, 18, 0);

		sprintf(err_str, ML(mlMenuMess128),
				DateToString(pmct_rec.ct_date_lupd),
				pmct_rec.ct_time_lupd);
		rv_pr(err_str, 12, 19, 0);
	}
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
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

	abc_alias(pmre2, pmre);
	abc_alias(cumr2, cumr);
	abc_alias(sumr2, sumr);

	open_rec(pmre, pmre_list, pmre_no_fields, "pmre_id_no");
	open_rec(pmre2, pmre_list, pmre_no_fields, "pmre_call_no");

	open_rec(pmse, pmse_list, pmse_no_fields, "pmse_call_no");
	open_rec(pmtx, pmtx_list, pmtx_no_fields, "pmtx_id_no");

	open_rec(cumr, cumr_list, cumr_no_fields, "cumr_id_no");
	open_rec(cumr2, cumr_list, cumr_no_fields, "cumr_hhcu_hash");

	open_rec(sumr, sumr_list, sumr_no_fields, "sumr_id_no");
	open_rec(sumr2, sumr_list, sumr_no_fields, "sumr_hhsu_hash");

	open_rec(pmct, pmct_list, pmct_no_fields, "pmct_system_name");
	open_rec(pmhs, pmhs_list, pmhs_no_fields, "pmhs_call_no");
	open_rec(pmhl, pmhl_list, pmhl_no_fields, "pmhl_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmre);
	abc_fclose (pmre2);
	abc_fclose (pmse);
	abc_fclose (pmtx);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (pmct);
	abc_fclose (pmhs);
	abc_fclose (pmhl);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		i;

	if (LCHECK ("logname"))
	{
		if (SRCH_KEY)
		{
			srch_user (temp_str, FALSE);
			return (EXIT_SUCCESS);
		}

		return (chk_user(temp_str, TRUE));
	}

	if (LCHECK("mess_to"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			srch_user(local_rec.to, FALSE);
			for (i = 0; i < 18; i++)
				rv_pr("                                    ",0,i,0);
			draw_menu(FALSE);
			heading(4);
			return (EXIT_SUCCESS);
		}

		if (!strncmp(local_rec.to, "ALL USERS", 9))
			return(0);

		if (chk_user(clip(local_rec.to), FALSE))
		{
			rv_pr(ML(mlStdMess139), 30, 23,1);
			sleep(2);
			move(0,23);
			cl_line();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK("priority"))
	{
		if (atoi(temp_str) > 5 || atoi(temp_str) < 1)
		{
			putchar(BELL);
			return(1);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK("CC"))
	{
		if (FLD("CC") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			rdrw_txt = TRUE;
			srch_CC(temp_str);
			return (EXIT_SUCCESS);
		}
		rdrw_txt = FALSE;

		return (chk_CC_user(temp_str));
	}

	if (LCHECK("src_no"))
	{
		if (FLD("src_no") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			if (local_rec.mail_src[0] == 'C')
				CumrSearch (comm_rec.tco_no, branchNumber, temp_str);

			if (local_rec.mail_src[0] == 'S')
				SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (local_rec.mail_src[0] == 'C')
		{
			strcpy(local_rec.src_no, pad_num(temp_str));
			DSP_FLD("src_no");
			abc_selfield(cumr, "cumr_id_no3");
			sprintf(cumr_rec.cm_dbt_no, "%-6.6s", temp_str);
			strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
			cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess(ML(mlStdMess021));
				putchar(BELL);
				sleep(1);
				clear_mess();
				return(1);
			}
			local_rec.link_hash = cumr_rec.cm_hhcu_hash;
			sprintf(local_rec.src_no_desc, "(%-40.40s)", 
						cumr_rec.cm_name);

			sprintf(local_rec.dflt_phone_no, "%-15.15s", 
						cumr_rec.cm_phone_no);

			sprintf(local_rec.dflt_contact_name, "%-25.25s", 
						cumr_rec.cm_contact_name);
		}
		    
		if (local_rec.mail_src[0] == 'S')
		{
			strcpy(local_rec.src_no, pad_num(temp_str));
			DSP_FLD("src_no");
			abc_selfield(sumr, "sumr_id_no3");
			sprintf(sumr_rec.sm_crd_no, "%-6.6s", temp_str);
			strcpy(sumr_rec.sm_co_no, comm_rec.tco_no);
			cc = find_rec(sumr, &sumr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess(ML(mlStdMess022));
				putchar(BELL);
				sleep(1);
				clear_mess();
				return(1);
			}

			local_rec.link_hash = sumr_rec.sm_hhsu_hash;
			sprintf(local_rec.src_no_desc, "(%-40.40s)", 
							sumr_rec.sm_name);

			sprintf(local_rec.dflt_phone_no, "%-15.15s", 
							sumr_rec.sm_cont_no);

			sprintf(local_rec.dflt_contact_name, "%-25.25s", 
							sumr_rec.sm_cont_name);
		}
		DSP_FLD("src_no_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Send a message directly to a	|
| users terminal		|
-------------------------------*/
int
send_message (
 void)
{
	char *	nam_str = getenv("LOGNAME");

	prog_exit = 0;
	while (!prog_exit)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;

		heading(4);
		entry(4);
		if (prog_exit || restart)
			continue;

		heading(4);
		scn_display(4);
		edit(4);
	
		if (restart)
			continue;

		if (!strncmp(local_rec.to, "ALL USERS", 9))
		{
			sprintf (cmd, 
				"message -g \"%-64.64s [%s]\"", 
				local_rec.message,
				nam_str);
		}
		else
		{
			sprintf (cmd, 
				"message -u%s \"%-64.64s [%s]\"", 
				clip(local_rec.to), 
				local_rec.message,
				nam_str);
		}
		sys_exec (cmd);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*---------------------------
| Update Mail History Files |
---------------------------*/
int
upd_stats (
 void)
{
	char tmp_call [9];

	/*--------------------------------------
	| Check and update control file record |
	--------------------------------------*/
	if (!read_pmct (UPDT))
		return (FALSE);

	pmct_rec.ct_date_lupd = local_rec.lsystemDate;
	strcpy (pmct_rec.ct_time_lupd, TimeHHMM());
	cc = abc_update(pmct, &pmct_rec);
	if (cc)
		file_err(cc, pmct, "DBUPDATE");

	dsp_screen (" Updating Statistics ", " ", " ");
	/*----------------------
	| Process pmse records | 
	----------------------*/
	cc = find_hash(pmse, &pmse_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		sprintf(tmp_call, "%08ld", pmse_rec.se_call_no);
		dsp_process("Call Number :", tmp_call);

		cc = find_hash(pmhs, &pmhs_rec, COMPARISON, "u", pmse_rec.se_call_no);
		if (!cc)
		{
			upd_pmhl();
		
			if (pmhs_rec.hs_priority != pmse_rec.se_priority)
			{
				pmhs_rec.hs_priority = pmse_rec.se_priority;
				cc = abc_update(pmhs, &pmhs_rec);
				if (cc)
				{
					sprintf(err_str,
						"Error in pmhs during (DBUPDATE) Call No: %08ld",
						pmse_rec.se_call_no);
					sys_err(err_str, cc, PNAME);
				}
			}
			else
				abc_unlock(pmhs);

			cc = find_hash(pmse, &pmse_rec, NEXT, "r", 0L);	
			continue;
		}

		/*-------------------------------
		| Add new history header record |
		-------------------------------*/
		pmhs_rec.hs_call_no = pmse_rec.se_call_no;
		sprintf(pmhs_rec.hs_sender, "%-14.14s", pmse_rec.se_sender);
		pmhs_rec.hs_priority = pmse_rec.se_priority;
		strcpy(pmhs_rec.hs_link_type, pmse_rec.se_link_type);
		pmhs_rec.hs_link_hash = pmse_rec.se_link_hash;
		pmhs_rec.hs_date = pmse_rec.se_date;
		sprintf(pmhs_rec.hs_time, "%-8.8s", pmse_rec.se_time);
		strcpy(pmhs_rec.hs_stat_flag, pmse_rec.se_stat_flag);
		cc = abc_add(pmhs, &pmhs_rec);
		if (cc)
		{
			sprintf(err_str,
				"Error in pmhs during (DBADD) Call No: %08ld",
				pmse_rec.se_call_no);
			sys_err(err_str, cc, PNAME);
		}
		upd_pmhl();

		cc = find_hash(pmse, &pmse_rec, NEXT, "r", 0L);	
	}

	return(TRUE);
}

/*----------------------------
| Update Line Detail History |
| Records For Current Call No|
----------------------------*/
int
upd_pmhl (
 void)
{
	cc = find_hash(pmre2, &pmre_rec, GTEQ, "r", pmse_rec.se_call_no);
	while (!cc && pmre_rec.re_call_no == pmse_rec.se_call_no)
	{
		pmhl_rec.hl_call_no = pmre_rec.re_call_no;
		sprintf(pmhl_rec.hl_receiver, "%-14.14s", pmre_rec.re_receiver);
		cc = find_rec(pmhl, &pmhl_rec, COMPARISON, "u");
		if (cc)
		{
			strcpy(pmhl_rec.hl_seen, pmre_rec.re_seen);
			pmhl_rec.hl_active_date = pmre_rec.re_act_date;
			sprintf(pmhl_rec.hl_active_time, 
				"%-5.5s", 
				pmre_rec.re_act_time);
			pmhl_rec.hl_fst_rd_date = pmre_rec.re_fst_date;
			sprintf(pmhl_rec.hl_fst_rd_time, 
				"%-5.5s", 
				pmre_rec.re_fst_time);
			pmhl_rec.hl_date_action= 0L;
			strcpy(pmhl_rec.hl_time_action, "00:00:00");
			
			if (pmhl_rec.hl_fst_rd_date != 0L && 
			    pmhl_rec.hl_date_action == 0L)
			{
				check_actions();
			}

			cc = abc_add(pmhl, &pmhl_rec);
			if (cc)
		       	       file_err(cc, pmhl, "DBADD");
		}
		else
		{	
			strcpy(pmhl_rec.hl_seen, pmre_rec.re_seen);
			if (pmhl_rec.hl_fst_rd_date == 0L)
			{
				pmhl_rec.hl_fst_rd_date = pmre_rec.re_fst_date;
				sprintf(pmhl_rec.hl_fst_rd_time, 
					"%-5.5s", 
					pmre_rec.re_fst_time);
			}

			if (pmhl_rec.hl_fst_rd_date != 0L &&
			    pmhl_rec.hl_date_action == 0L)
			{
				check_actions();
			}

			cc = abc_update(pmhl, &pmhl_rec);
			if (cc)
		       	       file_err(cc, pmhl, "DBUPDATE");
		}

		cc = find_hash (pmre2,&pmre_rec, NEXT,"r",pmse_rec.se_call_no);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------
| Read Logistic Mail Control File |
---------------------------------*/
int
read_pmct (
 int rd_stat)
{
	sprintf (pmct_rec.ct_system_name, "%-40.40s", " ");
	cc = find_rec (pmct, &pmct_rec, GTEQ, "u");
	if (cc)
	{
		/* Control File (pmct) Record Does Not Exist, Please Create At Once */
		print_mess(ML(mlMenuMess118));
		sleep(4);
		clear_mess ();
		abc_unlock (pmct);
		return (FALSE);
	}

	if (rd_stat == UNLK)
		abc_unlock(pmct);

	return (TRUE);
}

/*------------------------------
| Get first actioned date/time |
------------------------------*/
int
check_actions (
 void)
{
	long	fst_date;
	int		fst_hh, tmp_hh;
	int		fst_mm, tmp_mm;
	int		fst_ss, tmp_ss;
	long	tmp_time [2];

	fst_date = 999999L;
	fst_hh = 24;
	fst_mm = 60;
	fst_ss = 60;

	pmhl_rec.hl_date_action = 0L;
	strcpy(pmhl_rec.hl_time_action, "00:00:00");

	pmtx_rec.tx_call_no = pmse_rec.se_call_no;
	pmtx_rec.tx_seq_no = 1;
	strcpy(pmtx_rec.tx_mail_type, "A");
	pmtx_rec.tx_line_no = 1;
	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	while (!cc && pmtx_rec.tx_call_no == pmse_rec.se_call_no)
	{
		if (!strcmp(pmtx_rec.tx_sender, pmre_rec.re_receiver) && 
		    pmtx_rec.tx_line_no == 0)
		{
			if (pmtx_rec.tx_date == fst_date)
			{
				tmp_ss = atoi(pmtx_rec.tx_time + 6);
				tmp_mm = atoi(pmtx_rec.tx_time + 3);
				tmp_hh = atoi(pmtx_rec.tx_time);
				tmp_time[0] = (long)tmp_hh * 3600L +
					      (long)tmp_mm * 60L +
					      (long)tmp_ss * 60L;

				tmp_time[1] = (long)fst_hh * 3600L +
					      (long)fst_mm * 60L +
					      (long)fst_ss * 60L;
				
				if (tmp_time[0] < tmp_time[1])
				{
					fst_hh = tmp_hh;
					fst_mm = tmp_mm;
					fst_ss = tmp_ss;
				}
			}
	
			if (pmtx_rec.tx_date < fst_date)
			{
				fst_date = pmtx_rec.tx_date;
				fst_ss = atoi(pmtx_rec.tx_time + 6);
				fst_mm = atoi(pmtx_rec.tx_time + 3);
				fst_hh = atoi(pmtx_rec.tx_time);
			}
		}

		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}

	if (fst_date < 999999L)
	{
		pmhl_rec.hl_date_action = fst_date;
		sprintf(pmhl_rec.hl_time_action, 
			"%02d:%02d:%02d", 
			fst_hh, 
			fst_mm, 
			fst_ss);
	}
	else
	{
		if (pmre_rec.re_status[0] != 'C')
		{
			pmhl_rec.hl_date_action = pmre_rec.re_lst_date;
			sprintf(pmhl_rec.hl_time_action, 
				"%-5.5s:00",
				pmre_rec.re_lst_time);
		}
	}

	return (EXIT_SUCCESS);
}

/*------------------------
| Purge database of mail |
| that is tagged as 'D'  |
| and is older than the  |
| specified date.	 |
------------------------*/
int
del_mail (
 void)
{
	char	tmp_status [2];
	char	tmp_receiver [15];
	long	tmp_call_no;
	char	tmp_call_str [9];
	int		i;

	/*--------------------------------------
	| Check and update control file record |
	--------------------------------------*/
	if (!read_pmct (UPDT))
		return (FALSE);

	restart = FALSE;
	heading(5);
	entry(5);

	if (restart)
	{
		restart = FALSE;
		return(FALSE);
	}
	
	/* Are you sure you want to purge all mail deleted before %-8.8s ? ",
		DateToString(local_rec.del_date));*/

	sprintf(err_str, ML(mlMenuMess120),
			DateToString(local_rec.del_date));
	i = prmptmsg(err_str, "YyNn", 5, 23);

	if (i == 'N' || i == 'n')
	{
		restart = FALSE;
		return(FALSE);
	}

	/*--------------------------
	| Update Mail Control File |
	--------------------------*/
	pmct_rec.ct_date_lpurge = local_rec.lsystemDate;
	strcpy (pmct_rec.ct_time_lpurge, TimeHHMM());
	cc = abc_update(pmct, &pmct_rec);
	if (cc)
		file_err(cc, pmct, "DBUPDATE");
	
	dsp_screen (" Purging database of deleted mail ", " ", " ");
	
	/*-------------------------
	| Delete all pmre records |
	| flagged as 'D' and older|
	| than required age.      |
	-------------------------*/
	sprintf (pmre_rec.re_receiver, "%-14.14s", " ");
	strcpy  (pmre_rec.re_status, " ");
	pmre_rec.re_call_no = 0L;
	cc = find_rec(pmre, &pmre_rec, GTEQ, "u");
	while (!cc)
	{
		if (pmre_rec.re_status[0] != 'D')	
		{
			abc_unlock(pmre);
			cc = find_rec(pmre, &pmre_rec, NEXT, "u");
			continue;
		}

		if (pmre_rec.re_act_date >= local_rec.del_date)	
		{
			abc_unlock(pmre);
			cc = find_rec(pmre, &pmre_rec, NEXT, "u");
			continue;
		}

		sprintf(tmp_receiver, "%-14.14s", pmre_rec.re_receiver);
		strcpy (tmp_status, pmre_rec.re_status);
		tmp_call_no = pmre_rec.re_call_no;

		dsp_process("Receiver :", pmre_rec.re_receiver);

		cc = abc_delete(pmre);
		if (cc)
			file_err(cc, pmre, "DBDELETE");

		/*-----------------------------------------
		| Check if any pmre records left for this |
		| call no. Flag pmse as 'D' if no pmre    |
		| records left.                           |
		-----------------------------------------*/
		all_deleted (tmp_call_no);

		sprintf(pmre_rec.re_receiver, "%-14.14s", tmp_receiver);
		strcpy (pmre_rec.re_status, tmp_status);
		pmre_rec.re_call_no = tmp_call_no;

		cc = find_rec(pmre, &pmre_rec, GTEQ, "u");
	}

	tmp_call_no = 0L;
	/*-------------------------
	| Delete all pmse records |
	| with no pmre records.   |
	-------------------------*/
	cc = find_hash(pmse, &pmse_rec, GTEQ,"u", tmp_call_no);
	while (!cc)
	{
		if (pmse_rec.se_status[0] != 'D')
		{
			abc_unlock(pmse);
			cc = find_hash(pmse, &pmse_rec, NEXT,"u",tmp_call_no);
			continue;
		}

		cc = find_hash(pmre2, &pmre_rec, GTEQ, "r", tmp_call_no);
		if (!cc && pmre_rec.re_call_no == tmp_call_no)
		{
			abc_unlock(pmse);
			cc = find_hash(pmse, &pmse_rec, NEXT,"u",tmp_call_no);
			continue;
		}

		sprintf(tmp_call_str, "%08ld", pmse_rec.se_call_no);
		dsp_process("Call Number :", tmp_call_str);

		tmp_call_no = pmse_rec.se_call_no;

		cc = abc_delete(pmse);
		if (cc)
			file_err(cc, pmse, "DBDELETE");

		/*-------------------------------
		| Delete all pmtx for this call |
		-------------------------------*/
		pmtx_rec.tx_call_no = tmp_call_no;
		pmtx_rec.tx_seq_no = 0;
		strcpy(pmtx_rec.tx_mail_type, " ");
		pmtx_rec.tx_line_no = 0;

		cc = find_rec(pmtx, &pmtx_rec, GTEQ, "u");
		while (!cc && pmtx_rec.tx_call_no == tmp_call_no)
		{
			cc = abc_delete(pmtx);
			if (cc)
				file_err(cc, pmtx, "DBDELETE");
			
			pmtx_rec.tx_call_no = tmp_call_no;
			pmtx_rec.tx_seq_no = 0;
			strcpy (pmtx_rec.tx_mail_type, " ");
			pmtx_rec.tx_line_no = 0;

			cc = find_rec(pmtx, &pmtx_rec, GTEQ, "u");
		}

		cc = find_hash(pmse, &pmse_rec, GTEQ,"u", tmp_call_no);
	}

	return(TRUE);
}

/*----------------
| Maintain mail. |
----------------*/
int
maintain_mail (
 void)
{
	restart = FALSE;

	if (!get_logname())
		return(FALSE);

	scn_write(1);

	while (choose_mail())
	{
		if (!read_ok)
			continue;

		DETAILS = TRUE;
		init_vars(2);
		load_mail();

		strcpy(local_rec.date_prmt, "Received Date:");

		heading(2);
		scn_display(2);

		load_text (FALSE);
		tab_display("disp_text", TRUE);

		fix_edge(10);

		edit(2);
		if (restart)
		{
			restart = FALSE;
			tab_close("disp_text", TRUE);
			continue;
		}

		scn_write(2);

		/*--------------------
		| Modify pmse record |
		--------------------*/
		pmse_rec.se_priority = local_rec.priority;
		sprintf (pmse_rec.se_subject, "%-40.40s", local_rec.subject);
		sprintf (pmse_rec.se_phone_no, "%-15.15s", local_rec.phone_no);
		sprintf (pmse_rec.se_contact_name, 
			"%-25.25s", 
			local_rec.contact_name);

		exit_loop = FALSE;
		while (!exit_loop)
		{
			if (DETAILS)
				set_keys(text_keys, "D", KEY_ACTIVE);
			else
				set_keys(text_keys, "D", KEY_PASSIVE);

			tab_keyset("disp_text", text_keys);

			tab_scan("disp_text");
			if (!exit_loop)
			{
				tab_close ("disp_text", TRUE);
				load_text (FALSE);
				tab_display ("disp_text", TRUE);
				fix_edge (10);
			}
		}

		move(0,23);
		cl_line();
		opt_ring_menu ();

		crsr_on();
		tab_close("disp_text", TRUE);

		if (strlen(clip(local_rec.CC)) != 0)
			add_to_CC ();

		if (!MORE_MAIL)
			break;
	}
	clear();
	return(0);
}

/*-----------------------------------------
| Allow user to choose which mail to read |
-----------------------------------------*/
int
choose_mail (
 void)
{
	long	hash;
	char	tmp_seen[2];
	char	tmp_str[14];
	char	curr_time[6];
	int		no_to_read = 0;

	strcpy (curr_time, TimeHHMM());
	while (TRUE)
	{
		if (read_head != READ_NULL)
			clr_read_list (read_head);
	
		read_head = READ_NULL;

		clear();

		sprintf(tmp_str, " Maintain Mail For: %-14.14s ", log_name);

		rv_pr(ML(mlMenuMess129),(80 - strlen(ML(mlMenuMess129))) / 2,1,1);

		set_keys(read_keys, "C", KEY_PASSIVE);
		no_to_read = 0;
		tab_open("read", read_keys, 4, 0, 14, FALSE);
		tab_add("read",
			"# %-6.6s %-3.3s %-14.14s %-40.40s %-9.9s ", 
			"Call #", 
			"Pri", 
			"     From     ", 
			"             Subject",
			"STATUS");

		sprintf(pmre_rec.re_receiver, "%-14.14s", log_name);
		strcpy(pmre_rec.re_status, " ");
		pmre_rec.re_call_no = 0L;
		cc = find_rec(pmre, &pmre_rec, GTEQ, "r");
		while (!cc && !strcmp(pmre_rec.re_receiver, log_name))
		{
			hash = pmre_rec.re_call_no;
			cc = find_hash(pmse,&pmse_rec, COMPARISON, "r", hash);
			if (cc)
			{
				cc = find_rec(pmre, &pmre_rec, NEXT, "r");
				continue;
			}

			if (pmre_rec.re_seen[0] == 'N' && 
			    pmre_rec.re_status[0] == 'C')
				strcpy(tmp_seen, "*");
			else
				strcpy(tmp_seen, " ");
	
			read_ptr = read_alloc();
			read_ptr->next = read_head;

			read_ptr->call_no = pmse_rec.se_call_no;
			read_ptr->priority = pmse_rec.se_priority;
			sprintf(read_ptr->sender, "%-1.1s%-14.14s", tmp_seen, pmse_rec.se_sender);
			sprintf(read_ptr->subject, "%40.40s",pmse_rec.se_subject);
			if (pmre_rec.re_status[0] == 'A')
				sprintf(read_ptr->status, "%-8.8s","Archived");
			else
			{
				if (pmre_rec.re_status[0] == 'C')
					sprintf(read_ptr->status, "%-8.8s","Current");
				else
					sprintf(read_ptr->status, "%-8.8s","Deleted");
			}

			read_head = read_ptr;
	
			cc = find_rec(pmre, &pmre_rec, NEXT, "r");
		}

		/*-------------------------------------
		| Put linked list contents into table |
		-------------------------------------*/
		read_ptr = read_head;
		while (read_ptr != READ_NULL)
		{
			tab_add("read",
				" %06ld  %1d  %-15.15s %-40.40s %-8.8s ",
				read_ptr->call_no,
				read_ptr->priority,
				read_ptr->sender,
				read_ptr->subject,
				read_ptr->status);
			no_to_read++;

			read_ptr = read_ptr->next;
		}

		if (no_to_read > 0)
		{
			MORE_MAIL = TRUE;
	
			exit_choose = FALSE;
			tab_scan("read");
			tab_close("read", TRUE);
			clear();
			return(MORE_MAIL);
		}
		else
		{
			sprintf(err_str, 
			     "%-21.21s%s", 
			     " ", 
			     "     ***   YOU HAVE NO MAIL   ***");

			tab_add("read", err_str);
			tab_display("read", TRUE);
			putchar(BELL);
			fflush(stdout);
			sleep(3);
			clear();
			tab_close("read", TRUE);
			return(0);
		}
	}
}

int
get_logname (
 void)
{
	/*----------------------------------
	| Reset Control Flags              |
	----------------------------------*/
	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	search_ok = 1;
	init_ok = 1;

	heading (1);
	entry (1);
	if (prog_exit || restart)
		return (FALSE);

	heading (1);
	scn_display (1);
	edit (1);

	if (restart)
		return (FALSE);

	return (TRUE);
}

int
clr_read_list (
 struct READ_PTR *	lcl_head)
{
	struct	READ_PTR	*lcl_ptr;
	struct	READ_PTR	*tmp_ptr;

	lcl_ptr = lcl_head;
	while (lcl_ptr != READ_NULL)
	{
		tmp_ptr = lcl_ptr;
		lcl_ptr = lcl_ptr->next;
		free (tmp_ptr);
	}
	return (EXIT_SUCCESS);
}

/*--------------------------------
| Load the call number requested |
--------------------------------*/
int
load_mail (
 void)
{
	long	hash;

	cc = find_hash(pmse, &pmse_rec, COMPARISON, "u", read_call);
	if (cc)
	{
		/* Header Record Unavailable ", 25,23,1*/
		rv_pr(ML(mlMenuMess130), 25,23,1);
		sleep(2);	
		move (0,23);
		cl_line ();
		return (EXIT_SUCCESS);
	}

	sprintf(local_rec.call_no,      "%06ld",    pmse_rec.se_call_no);
	sprintf(local_rec.from,         "%-14.14s", pmse_rec.se_sender);
	sprintf(local_rec.subject,      "%-40.40s", pmse_rec.se_subject);
	sprintf(local_rec.subject,      "%-40.40s", pmse_rec.se_subject);
	sprintf(local_rec.phone_no,     "%-15.15s", pmse_rec.se_phone_no);
	sprintf(local_rec.contact_name, "%-25.25s", pmse_rec.se_contact_name);
	local_rec.priority = pmse_rec.se_priority;

	sprintf (local_rec.rcvd_date, 
		"%-10.10s   %-5.5s", 
		DateToString(pmre_rec.re_act_date), 
		pmre_rec.re_act_time);

	strcpy(local_rec.mail_src, "None    ");
	sprintf(local_rec.src_no_desc, "%-40.40s", " ");

	if (pmse_rec.se_link_type[0] == 'C')
	{
		strcpy(local_rec.src_prmt, "Customer No:");
		hash = pmse_rec.se_link_hash;
		local_rec.link_hash = hash;
		cc = find_hash(cumr2, &cumr2_rec, COMPARISON, "r", hash);
		if (cc)
			sprintf(local_rec.src_no_desc, 
				"(%-40.40s)", 
				"Customer Not Found On File ");
		else
		{
			sprintf(local_rec.src_no, 
				"%-6.6s", 
				cumr2_rec.cm_dbt_no);

			sprintf(local_rec.src_no_desc, 
				"(%-40.40s)", 
				cumr2_rec.cm_name);
		}
	}

	if (pmse_rec.se_link_type[0] == 'S')
	{
		strcpy(local_rec.src_prmt, "Supplier No:");
		hash = pmse_rec.se_link_hash;
		local_rec.link_hash = hash;
		cc = find_hash(sumr2, &sumr2_rec, COMPARISON, "r", hash);
		if (cc)
			sprintf(local_rec.src_no_desc, 
				"(%-40.40s)", 
				"Supplier Not Found On File ");
		else
		{
			sprintf(local_rec.src_no, 
				"%-6.6s", 
				sumr2_rec.sm_crd_no);

			sprintf(local_rec.src_no_desc, 
				"(%-40.40s)", 
				sumr2_rec.sm_name);
		}
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------
| Load mail text into tabular screen |
------------------------------------*/
int
load_text (
 int	ed_flag)
{
	char	actions_flag [12];
	char	curr_mail_type [2];
	int		first_time = TRUE;
	int		old_seq_no = 0;
	int		ACT_OK;

	if (DETAILS)
	{
		if (has_actions())
			strcpy(actions_flag, "HAS ACTIONS");
		else
			strcpy(actions_flag, "           ");

		strcpy(curr_mail_type, "D");
		pmtx_rec.tx_seq_no = 0;
	}
	else
	{
		strcpy(actions_flag, "           ");
		strcpy(curr_mail_type, "A");
		pmtx_rec.tx_seq_no = 1;
	}

	if (ed_flag)
		lcount[3] = 0;
	else
	{
		tab_open("disp_text", text_keys, 10, 0, 8, FALSE);
		sprintf(err_str, 
			"#                     Current Selection %s                   %-11.11s", 
			(DETAILS) ? "D(etails)" : "A(ctions)",
			actions_flag);

		tab_add("disp_text", err_str);
	}
	
	pmtx_rec.tx_call_no = read_call;
	strcpy(pmtx_rec.tx_mail_type, curr_mail_type);
	pmtx_rec.tx_line_no = 0;

	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	while (!cc && pmtx_rec.tx_call_no == read_call && 
	       !strcmp (pmtx_rec.tx_mail_type, curr_mail_type)) 
	{
		ACT_OK  = FALSE;
		if (ACTIONS && pmtx_rec.tx_seq_no != old_seq_no)
		{
			old_seq_no = pmtx_rec.tx_seq_no;
			if (!first_time && !ed_flag)
				tab_add("disp_text", "%-78.78s", " ");

			sprintf(err_str,
				"ACTIONED BY : %-14.14s",
				pmtx_rec.tx_sender);

			if (ed_flag)
			{
				if (strcmp(pmtx_rec.tx_sender, log_name))
				{
					ACT_OK = TRUE;
					sprintf(local_rec.text, 
						"%-60.60s", 
						pmtx_rec.tx_text);
					putval(lcount[3]++);
				}
			}
			else
				tab_add("disp_text", "  %-76.76s", err_str);

		}

		if (ed_flag)
		{
			if (!ACTIONS || (ACTIONS && ACT_OK))
			{
				sprintf(local_rec.text, 
					"%-60.60s", 
					pmtx_rec.tx_text);
				putval(lcount[3]++);
			}
		}
		else
		{
			tab_add("disp_text", 
				"%-9.9s%-60.60s%-9.9s", 
				" ", 
				pmtx_rec.tx_text, 
				" ");
		}

		first_time = FALSE;
		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
add_to_CC (
 void)
{
	struct	USR_PTR *snd_lcl;

	cc = find_hash(pmse, &pmse_rec, COMPARISON, "r", read_call);
	if (cc)
		file_err (cc, pmse, "DBFIND");

	/*--------------------------------------
	| Create List of users to send mail to |
	--------------------------------------*/
	create_send_list();

	/*-------------------------------
	| Create extra receiver records |
	-------------------------------*/
	snd_lcl = snd_head;
	while (snd_lcl != USR_NULL)
	{
		/*-------------------------
		| Create new pmre record. |
		-------------------------*/
		pmre_rec.re_call_no = read_call;
		sprintf(pmre_rec.re_receiver, "%-14.14s", snd_lcl->usr_name);
		strcpy(pmre_rec.re_status, "C");
		strcpy(pmre_rec.re_seen, "N");

		strcpy(pmre_rec.re_cc_flag, "Y");

		pmre_rec.re_act_date = pmse_rec.se_date;
		sprintf (pmre_rec.re_act_time, "%-5.5s", pmse_rec.se_time);

		pmre_rec.re_fst_date = 0L;	
		strcpy(pmre_rec.re_fst_time, "00:00");
		pmre_rec.re_lst_date = 0L;	
		strcpy(pmre_rec.re_lst_time, "00:00");
		cc = abc_add(pmre, &pmre_rec);
		if (cc)
		{
			if (cc != 100)
       	         	       file_err(cc, pmre, "DBADD");
		}
		else
			mail_notify (snd_lcl->usr_name);
	
		snd_lcl = snd_lcl->next;
	}

	return(0);
}

/*------------------------------
| Allow user to choose options |
| from ring menu	       |
------------------------------*/
int
opt_ring_menu (
 void)
{
	int	no_of_flds = 3;
	int	curr_fld = 0;
	int	key;
	int	exit_loop;

	/*---------------
	| Erase hotkeys |
	---------------*/
	move(2,21);
	line(76);

	/*---------------------
	| Set initial options |
	---------------------*/
	crsr_off();
	/*	<No Change> 
		<Archived> 
		<Deleted>
		<Current> */

	rv_pr(ML(mlMenuMess131),   10, 22, 1);
	rv_pr(ML(mlMenuMess132),    26, 22, 0);
	rv_pr(ML(mlMenuMess133),     42, 22, 0);
	rv_pr(ML(mlMenuMess134),     57, 22, 0);

	key = getkey ();
	exit_loop = FALSE;
	while (!exit_loop)
	{
		/*	<No Change>
			<Archived>
			<Deleted>
			<Current>*/

		rv_pr(ML(mlMenuMess131),   10, 22, 0);
		rv_pr(ML(mlMenuMess132),    26, 22, 0);
		rv_pr(ML(mlMenuMess133),     42, 22, 0);
		rv_pr(ML(mlMenuMess134),     57, 22, 0);

		switch (key)
		{
		case UP_KEY:
		case LEFT_KEY:
		case '\b':
			curr_fld--;
			if (curr_fld < 0)
				curr_fld = no_of_flds;
			break;

		case DOWN_KEY:
		case RIGHT_KEY:
		case ' ':
			curr_fld++;
			if (curr_fld > no_of_flds)
				curr_fld = 0;
			break;

		case FN16:
		case 'N' :
		case 'n' :
			curr_fld = 0;
			key = '\r';
			continue;

		case 'A':
		case 'a':
			curr_fld = 1;
			key = '\r';
			continue;

		case 'D':
		case 'd':
			curr_fld = 2;
			key = '\r';
			continue;

		case 'C':
		case 'c':
			curr_fld = 3;
			key = '\r';
			continue;

		case '\r':
			switch (curr_fld)
			{
			case 0:
				chng_stat("N");
				chk_mail();
				return(0);
			case 1:
				chng_stat("A");
				chk_mail();
				return(0);
			case 2:
				chng_stat("D");
				chk_mail();
				return(0);
			case 3:
				chng_stat("C");
				chk_mail();
				return(0);
			}

		default:
			putchar(BELL);
			break;
		}

		switch (curr_fld)
		{
		case 0:
			/*<No Change>*/
			rv_pr(ML(mlMenuMess131),   10, 22, 1);
			break;
		case 1:
			/*<Archived>*/
			rv_pr(ML(mlMenuMess132),    26, 22, 1);
			break;
		case 2:
			/*<Deleted>*/
			rv_pr(ML(mlMenuMess133),     42, 22, 1);
			break;
		case 3:
			/*<Current>*/
			rv_pr(ML(mlMenuMess134),     57, 22, 1);
			break;
		}
		crsr_off();
		key = getkey();
	}
	return (EXIT_SUCCESS);
}

/*-----------------------
| Change status of mail |
-----------------------*/
int
chng_stat (
 char *	ch_stat)
{
	if (strcmp (ch_stat, "N") )
		sprintf(pmre_rec.re_status, "%-1.1s", ch_stat);

	cc = abc_update(pmre, &pmre_rec);
	if (cc)
              	file_err(cc, pmre, "DBUPDATE");

	if (ch_stat[0] != 'D' && pmse_rec.se_status[0] == 'D')
		strcpy(pmse_rec.se_status, "C");

	cc = abc_update(pmse, &pmse_rec);
	if (cc)
        	file_err(cc, pmse, "DBUPDATE");

	all_deleted (read_call);

	return (EXIT_SUCCESS);
}

/*-----------------------------
| Check if there is more mail |
| for user to read	      |
-----------------------------*/
int
chk_mail (
 void)
{
	MORE_MAIL = TRUE;
	sprintf (pmre2_rec.re_receiver, "%-14.14s", log_name);
	strcpy  (pmre2_rec.re_status, " ");
	pmre2_rec.re_call_no = 0L;

	cc = find_rec(pmre, &pmre2_rec, GTEQ, "r");
	if (cc || strcmp (pmre2_rec.re_receiver, log_name))
		MORE_MAIL = FALSE;

	return(0);
}

/*--------------------------------
| Check if all receivers of this |
| call have deleted it		 |
--------------------------------*/
int
all_deleted (
 long	call_no)
{
	if (mail_maint)
	{
		cc = find_hash(pmre2, &pmre_rec, GTEQ, "r", call_no);
		while (!cc && pmre_rec.re_call_no == call_no)
		{
			if (pmre_rec.re_status[0] != 'D')
				return(FALSE);
			cc = find_hash(pmre2, &pmre_rec, NEXT,"r", call_no);
		}
	}
	else
	{
		cc = find_hash(pmre2, &pmre_rec, GTEQ, "r", call_no);
		if (!cc && pmre_rec.re_call_no == call_no)
			return(FALSE);
	}

	cc = find_hash(pmse, &pmse_rec, COMPARISON, "u", call_no);
	if (cc)
		return(TRUE);

	strcpy(pmse_rec.se_status, "D");

	cc = abc_update(pmse, &pmse_rec);
	if (cc)
       		file_err(cc, pmse, "DBUPDATE");

	return(TRUE);
}

/*-------------------------------
| Check if user is a valid user |
| from Mail_secure		|
| If status is TRUE then ALL	|
| LOGGED IN is a valid user     |
-------------------------------*/
int
chk_user (
 char *	key_val,
 int	status)
{
	struct	USR_PTR	*usr_lcl;
	char	tmp_usr_name[15];

	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		sprintf(tmp_usr_name, "%-14.14s", usr_lcl->usr_name);
		if (!strcmp(clip(tmp_usr_name), key_val))
			return(0);

		usr_lcl = usr_lcl->next;
	}

	putchar(BELL);
	return(1);
}

/*---------------------------------
| Check that all users entered as |
| a CC are valid users from 	  |
| Mail_secure			  |
---------------------------------*/
int
chk_CC_user (
 char *	key_val)
{
	char *	sptr;
	char *	tptr;
	char	tmp_name [61];
	char	char_separator;
	int		lots_of_spaces;

	sptr = strdup (key_val);
	tptr = sptr;

	if (strchr(sptr, ','))
		char_separator = ',';
	else
		char_separator = ' ';

	while (*sptr)
	{
		while (*tptr && *tptr != char_separator)
			tptr++;
		if (*tptr && char_separator == ' ')
		{
			lots_of_spaces = FALSE;
			while (*tptr && *tptr == char_separator)
			{
				lots_of_spaces = TRUE;
				tptr++;
			}

			if (lots_of_spaces)
				tptr--;
		}

		sprintf(tmp_name, "%-.*s", (tptr - sptr), sptr);
		lclip(tmp_name);
		clip(tmp_name);

		if (chk_user(tmp_name, TRUE))
		{
			sprintf(err_str, "%-14.14s", tmp_name);
			strcpy(tmp_name, clip(err_str));
			/*sprintf(err_str, "\007 %s is not a valid user ", tmp_name);*/
			rv_pr(ML(mlStdMess139), 22,23,1);
			sleep(2);
			move(0,23);
			cl_line();
			putchar(BELL);
			return(1);
		}
	
		if (!strcmp(tmp_name, clip(local_rec.to)))
		{
			/*rv_pr("\007 Can't CC mail to the same user as you are sending mail TO ", 10,23,1);*/
			rv_pr(ML(mlMenuMess135), 10,23,1);
			sleep(2);
			move(0,23);
			cl_line();
			return(1);
		}

	       	if (!chk_not_CC(tmp_name))
		{
			/*%s already has a copy of this mail ", tmp_name*/
			sprintf(err_str,ML(mlMenuMess136), tmp_name);
			rv_pr(err_str, 22,23,1);
			sleep(2);
			move(0,23);
			cl_line();
			return(1);
		}

		if (*tptr)
			sptr = ++tptr;
		else
			sptr = tptr;
	}

	return(0);
}

int
srch_user (
 char *	key_val,
 int	status)
{
	struct	USR_PTR	*usr_lcl;

	work_open();
	save_rec("#User Name","# ");
	if (status)
	{
		save_rec("ALL LOGGED IN"," ");
		save_rec("ALL USERS"," ");
	}

	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		sprintf(err_str, "%-14.14s", usr_lcl->usr_name);
		cc = save_rec(err_str, " ");
		if (cc)
			break;
		usr_lcl = usr_lcl->next;
	}

	cc = disp_srch ();
	work_close ();

	return (EXIT_SUCCESS);
}

int
srch_CC (
 char *	key_val)
{
	struct	USR_PTR	*usr_lcl;
	char *	sptr;
	char	user_buf[100];
	char	tmp_name[15];
	char	last_name[15];
	int	i;
	int	no_in_tab = 0;

	tab_open("user", user_keys, 2, 1, 8, FALSE);
	tab_add("user","#     User Name     Send To   ");

	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
	       sprintf(err_str, "    %-14.14s           ", usr_lcl->usr_name);

	       if (chk_not_CC(usr_lcl->usr_name) && 
		   strcmp(usr_lcl->usr_name, log_name))
	       {
	       		tab_add("user", err_str);
	       		no_in_tab++;
	       }

		usr_lcl = usr_lcl->next;
	}

	no_tagged = 0;

	tab_scan("user");

	strcpy(tagged_name, "");
	for (i = 0; i < no_in_tab; i++)
	{
		tab_get("user", user_buf, EQUAL, i);
		if (user_buf[22] == '*')
		{
			sprintf(tmp_name, "%-.14s", &user_buf[4]);
			clip(tmp_name);
			strcpy(last_name, tmp_name);
			strcat(tmp_name, ", ");
			strcat(tagged_name, tmp_name);
		}
	}
		
		
	if (strlen(clip(tagged_name)) > 0)
	{
		/*------------------------------------------------
		| Replace right hand , with . within tagged_name |
		------------------------------------------------*/
		sptr = strrchr(tagged_name, ',');
		*sptr = '.';
	}

	sprintf(temp_str, "%-60.60s", tagged_name);

	/*--------------------------------------------
	| Check for right hand . within local_rec.CC |
	--------------------------------------------*/
	sptr = strrchr(temp_str, '.');
	if (!sptr)
	{
		sptr = strrchr(temp_str, ',');
		if (sptr)
		{
			*sptr = ' ';
			if (strncmp(last_name, sptr + 2, strlen (last_name)))
				*sptr = '\0';
		}
	}
	else
		*sptr = ' ';

	tab_close("user", TRUE);
	return (EXIT_SUCCESS);
}

int
chk_not_CC (
 char *	chk_name)
{
	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "r", read_call);
	while (!cc && pmre2_rec.re_call_no == read_call)
	{
		if (!strcmp(chk_name, clip(pmre2_rec.re_receiver)))
			return (FALSE);
	
		cc = find_hash(pmre2, &pmre2_rec, NEXT, "r", read_call);
	}

	return (TRUE);
}

/*-------------------------------
| Read users from Mail_secure	|
| and store in a linked list	|
-------------------------------*/
int
get_all_users (
 void)
{
	struct	USR_PTR	*usr_lcl;
	char *	sptr;
	char *	tptr;

	/*------------------------------------
	| Read Mail_secure file and store it | 
	------------------------------------*/
	if ((fin = fopen(usr_fname,"r")) == 0)
		sys_err("Error in Mail_secure during (FOPEN)", errno, PNAME);

	/*----------------------------
	| Store users in linked list |
	----------------------------*/
	usr_head = USR_NULL;
	usr_curr = usr_head;

	sptr = fgets(err_str, 80, fin);
	while (sptr)
	{
		if (*sptr == '#')
		{
			sptr = fgets(err_str, 80, fin);
			continue;
		}

		*(sptr + strlen(sptr) - 1) = '\0';
		tptr = strchr (sptr, '\t');
		if (tptr)
			*tptr = 0;
		clip (sptr);

		/*--------------------------------------
		| Allocate memory and set next to null |
		--------------------------------------*/
		usr_lcl = usr_alloc();
		usr_lcl->next = USR_NULL;

		/*---------------------------
		| Set head to start of list |
		---------------------------*/
		if (usr_head == USR_NULL)
			usr_head = usr_lcl;
		else
			usr_curr->next = usr_lcl;

		/*----------------------------
		| store user name in current |
		----------------------------*/
		usr_curr = usr_lcl;
		sprintf(usr_curr->usr_name, "%-14.14s", sptr);

		sptr = fgets(err_str,80,fin);
	}
	fclose(fin);

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct	USR_PTR *
usr_alloc (
 void)
{
	struct	USR_PTR	*lcl_ptr;

	lcl_ptr = (struct USR_PTR *) malloc (sizeof (struct USR_PTR));

	if (lcl_ptr == USR_NULL)
        	sys_err("Error allocating memory for usr list During (MALLOC)",errno,PNAME);
		
	return (lcl_ptr);
}

/*--------------------------------------
| Allocate memory for read linked list |
--------------------------------------*/
struct	READ_PTR *
read_alloc (
 void)
{
	struct	READ_PTR	*lcl_ptr;

	lcl_ptr = (struct READ_PTR *) malloc (sizeof (struct READ_PTR));

	if (lcl_ptr == READ_NULL)
        	sys_err("Error allocating memory for read list During (MALLOC)",
			errno,PNAME);
		
	return (lcl_ptr);
}

static	int
dummy_func (
 int		c,
 KEY_TAB *	psUnused)
{
	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Allow user to tag users to CC mail to |
---------------------------------------*/
static	int
user_tag_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	user_buf [100];

	tab_get("user", user_buf, CURRENT, 0);
	
	switch(user_buf[22])
	{
	case ' ':
		if (no_tagged < 20)
		{
			sprintf(err_str, "    %-14.14s    *      ", &user_buf[4]);
			tab_update("user", err_str);

			no_tagged++;
		}
		break;

	case '*':
		sprintf(err_str, "    %-14.14s           ", &user_buf[4]);
		tab_update("user", err_str);
		break;
	
	default:
		break;
	}

	return (c);
}

static	int
exit_func (
 int		c,
 KEY_TAB *	psUnused)
{
	exit_loop = TRUE;
	MORE_MAIL = FALSE;
	exit_choose = TRUE;
	return (FN16);
}

/*--------------------------------------------
| Preliminary check on mail selected to read |
--------------------------------------------*/
static	int
read_slct_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	read_buf [100];
	char	tmp_call [7];

	read_ok = TRUE;
	tab_get ("read", read_buf, CURRENT, 0);

	sprintf (tmp_call, "%-6.6s", &read_buf [1]);
	read_call = atol (tmp_call);

	sprintf (pmre_rec.re_receiver, "%-14.14s", log_name );
	sprintf (pmre_rec.re_status,   "%-1.1s",   &read_buf [69]);
	pmre_rec.re_call_no = read_call;

	cc = find_rec (pmre, &pmre_rec, COMPARISON, "u");
	if (cc)
	{
		/*Mail Unavailable For Edit. */
		rv_pr (ML (mlMenuMess137), 25, 23, 1);
		sleep (sleepTime);
		move (0, 23);
		cl_line ();
		chk_mail ();
		read_ok = FALSE;
		return (FN16);
	}

	exit_choose = TRUE;
	return (FN16);
}

/*-------------------------
| Show users mail sent to |
-------------------------*/
static	int
show_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	show_buf [100];
	int	no_to_show = 0;

	tab_get ("read", show_buf, CURRENT, 0);
	read_call = atoi (&show_buf [1]);
	
	tab_open ("show", show_keys, 4, 50, 10, FALSE);
	tab_add ("show", "#       Users Sent To       ");

	cc = find_hash (pmre2, &pmre_rec, GTEQ, "r", read_call);
	while (!cc && pmre_rec.re_call_no == read_call)
	{
		if (pmre_rec.re_status[0] != 'D')
		{
			if (pmre_rec.re_seen[0] == 'Y')
				strcpy(err_str, " ");
			else
				strcpy(err_str, "*");

			tab_add("show",
				"    %-1.1s%-14.14s ",
				err_str,
				pmre_rec.re_receiver);
			no_to_show++;
		}

		cc = find_hash(pmre2, &pmre_rec, NEXT, "r", read_call);
	}

	if (no_to_show > 0)
		tab_scan("show");
	else
	{
		tab_add("show","*All Mail Deleted By Users*");
		tab_display("show", TRUE);
		putchar(BELL);
		fflush(stdout);
		sleep(3);
	}

	tab_close ("show", TRUE);
	tab_display ("read", TRUE);
	redraw_keys ("read");
	
	return (c);
}

/*----------------------------------
| Display details for current mail |
----------------------------------*/
static	int
details_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (ACTIONS)
	{
		DETAILS = TRUE;
		return (FN16);
	}
	else
	{
		putchar (BELL);
		fflush (stdout);
		return (c);
	}
}

/*----------------------------------
| Display actions for current mail |
----------------------------------*/
static	int
actions_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (DETAILS)
	{
		if (!has_actions())
		{
			/* No Actions For This Mail */
			rv_pr(ML(mlMenuMess125), 25, 23, 1);
			sleep(2);
			move(0,23);
			cl_line();
			return(c);
		}
	
		DETAILS = FALSE;
		return(FN16);
	}
	else
	{
		putchar(BELL);
		fflush(stdout);
		return(c);
	}
}

/*----------------------------------------------
| Edit Current Tab_disp window as a TXT screen |
----------------------------------------------*/
static	int
ed_txt_scrn (
 int		c,
 KEY_TAB *	psUnused)
{
	init_vars (3);
	load_text (TRUE);

	scn_display (3);
	edit (3);

	add_text ();

	erase_scn (3);
	crsr_off ();
	return (FN16);
}

/*-------------------------------------
| Print mail to user selected printer |
-------------------------------------*/
static	int
mail_prnt_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int 	lpno;
	int	pipe_open = FALSE;

	lpno = get_lpno (0);

	heading (2);
	scn_display (2);
	tab_display ("disp_text", TRUE);
	
	/*Printing...*/
	rv_pr (ML (mlStdMess035), 35, 23, 1);

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno,PNAME);
	else
		pipe_open = TRUE;

	/*---------------------------------
	| Initialize printer for output.  |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".8\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L80\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".ELOGISTIC MAIL SYSTEM\n");
	fprintf(fout,".B1\n");
	fprintf(fout,"+==============================================================================+\n");
	fprintf(fout,
		"| Call Number : %-6.6s     From: %-14.14s    To: %-14.14s          |\n", 
		local_rec.call_no, 
		local_rec.from, 
		log_name);
	fprintf(fout,"+------------------------------------------------------------------------------+\n");

	/*------------------
	| Print main info  |
	------------------*/
	fprintf(fout, "| CC      : %-60.60s       |\n", local_rec.CC);
	fprintf(fout,"|                                                                              |\n");
	fprintf(fout, "| Priority:  %d                                                                 |\n", local_rec.priority);
	fprintf(fout, "| Subject : %-60.60s       |\n", local_rec.subject);

	if (local_rec.mail_src[0] != 'N')
	{
		fprintf(fout, 
		       "| %-8.8s No: %-6.6s  %-42.42s              |\n", 
		       (local_rec.mail_src[0] == 'C') ? "Customer" : "Supplier",
		       local_rec.src_no, 
		       local_rec.src_no_desc);
	}

	fprintf(fout, 
		"| Phone Number: %-15.15s     Contact Name: %-25.25s    |\n",
		local_rec.phone_no,
		local_rec.contact_name);
	
	fprintf(fout,"+------------------------------------------------------------------------------+\n");

	/*------------------------------------
	| Print details associated with mail |
	------------------------------------*/
	fprintf(fout,".LRP3\n");
	fprintf(fout,"|                                    DETAILS                                   |\n");
	fprintf(fout,"|                                                                              |\n");
	prnt_text("D");
	fprintf(fout,"|                                                                              |\n");

	/*------------------------------------
	| Print actions associated with mail |
	------------------------------------*/
	if (has_actions ())
	{
		fprintf(fout,"+------------------------------------------------------------------------------+\n");
		fprintf(fout,".LRP4\n");
		fprintf(fout,"|                                    ACTIONS                                   |\n");
		prnt_text("A");
		fprintf(fout,"|                                                                              |\n");
		fprintf(fout,"+==============================================================================+\n");
	}
	else
	{
		fprintf(fout,"|                                                                              |\n");
		fprintf(fout,"+==============================================================================+\n");
	}
	fflush(fout);
	if (pipe_open)
	{
		pclose(fout);
		pipe_open = FALSE;
	}

	move(0,23);
	cl_line();
	redraw_keys("disp_text");

	return(c);
}

/*------------------
| Print text lines |
------------------*/
void
prnt_text (
 char *	type)
{
	int	old_seq_no;
	int	first_time = TRUE;

	pmtx_rec.tx_call_no = read_call;
	strcpy(pmtx_rec.tx_mail_type, type);
	pmtx_rec.tx_line_no = 0;
	if (type[0] == 'D')
		pmtx_rec.tx_seq_no = 0;
	else
		pmtx_rec.tx_seq_no = 1;

	old_seq_no = 0;
	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	while (!cc && pmtx_rec.tx_call_no == read_call && 
	       !strcmp (pmtx_rec.tx_mail_type, type)) 
	{
		if (type[0] == 'A' && pmtx_rec.tx_seq_no != old_seq_no)
		{
			old_seq_no = pmtx_rec.tx_seq_no;
			fprintf(fout,"|                                                                              |\n");
			fprintf(fout,"| ACTIONED BY : %-14.14s%-49.49s|\n",
						pmtx_rec.tx_sender, " ");
		}

		fprintf(fout, 
			"|         %-60.60s         |\n", 
			pmtx_rec.tx_text);

		first_time = FALSE;

		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}
}

/*-------------------------------
| Creates pmtx records from the	|
| current tabular screen	|
| 'A' = Actions , 'D' = Details
-------------------------------*/
int
add_text (
 void)
{
	int		line_cnt;
	
	/*--------------
	| Add new text |
	--------------*/
	pmtx_rec.tx_call_no = read_call;
	pmtx_rec.tx_seq_no = 0;
	pmtx_rec.tx_date = 0L;
	strcpy(pmtx_rec.tx_time, "00:00:00");

	scn_set(3);
	for (line_cnt = 0;line_cnt < lcount[3];line_cnt++)
	{
		sprintf(pmtx_rec.tx_sender, "%-14.14s", log_name);
		pmtx_rec.tx_line_no = line_cnt;
		strcpy(pmtx_rec.tx_mail_type, "D");
		getval(line_cnt);

		cc = find_rec(pmtx, &pmtx_rec, COMPARISON, "u");
		strcpy(pmtx_rec.tx_text, local_rec.text);
		if (cc)
		{
			cc = abc_add(pmtx, &pmtx_rec);
			if (cc)
       	             	       file_err(cc, pmtx, "DBFIND");
		}
		else
		{
			cc = abc_update(pmtx, &pmtx_rec);
			if (cc)
       	             	       file_err(cc, pmtx, "DBUPDATE");
		}
	}

	/*-----------------
	| Delete old text |
	-----------------*/
	pmtx_rec.tx_call_no = read_call;
	pmtx_rec.tx_seq_no = 0;
	strcpy(pmtx_rec.tx_mail_type, "D");
	pmtx_rec.tx_line_no = lcount[3];
		
	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "u");
	while (!cc && pmtx_rec.tx_call_no == read_call && 
	       pmtx_rec.tx_seq_no == 0 && pmtx_rec.tx_mail_type[0] == 'D')
	{
		cc = abc_delete(pmtx);
		if (cc)
			file_err(cc, pmtx, "DBDELETE");

		pmtx_rec.tx_call_no = read_call;
		pmtx_rec.tx_seq_no = 0;
		strcpy(pmtx_rec.tx_mail_type, "D");
		pmtx_rec.tx_line_no = lcount[3];
	
		cc = find_rec(pmtx, &pmtx_rec, GTEQ, "u");
	}
	if (!cc)
		abc_unlock(pmtx);

	return(0);
}

/*--------------------------------------
| Create list of users to send mail to |
--------------------------------------*/
int
create_send_list (
 void)
{
	struct	USR_PTR	*snd_lcl;
	char	*sptr;
	char	*tptr;
	char	tmp_name[15];
	char	char_separator;
	int	lots_of_spaces;

	snd_head = USR_NULL;
	snd_curr = USR_NULL;

	/*---------------------
	| create list from CC |
	---------------------*/
	clip(local_rec.CC);
	lclip(local_rec.CC);
	sptr = local_rec.CC;
	tptr = sptr;

	if (strchr(sptr, ','))
		char_separator = ',';
	else
		char_separator = ' ';

	while (*sptr)
	{
		while (*tptr && *tptr != char_separator)
			tptr++;
		if (*tptr && char_separator == ' ')
		{
			lots_of_spaces = FALSE;
			while (*tptr && *tptr == char_separator)
			{
				lots_of_spaces = TRUE;
				tptr++;
			}

			if (lots_of_spaces)
				tptr--;
		}

		if (tptr != sptr)
		{
			sprintf(tmp_name, "%-.*s", (tptr - sptr), sptr);
			lclip(tmp_name);
			clip(tmp_name);
			clip(local_rec.to);

			if (strcmp(tmp_name, local_rec.to))
			{
				/*--------------------------------------
				| Allocate memory and set next to null |
				--------------------------------------*/
				snd_lcl = usr_alloc();
				snd_lcl->next = USR_NULL;
				sprintf(snd_lcl->usr_name,"%-14.14s",tmp_name);
	
				/*------------------------------
				| store user name in send list |
				------------------------------*/
				if (snd_head == USR_NULL)
				{
					snd_head = snd_lcl;
					snd_curr = snd_head;
				}
				else
				{
					snd_curr->next = snd_lcl;
					snd_curr = snd_lcl;
				}
			}
			if (*tptr)
				sptr = ++tptr;
			else
				sptr = tptr;
		}
	}

	return(0);
}

/*--------------------------------
| Check if there are any actions |
--------------------------------*/
int
has_actions (
 void)
{
	pmtx_rec.tx_call_no = read_call;
	pmtx_rec.tx_seq_no = 1;
	strcpy(pmtx_rec.tx_mail_type, "A");
	pmtx_rec.tx_line_no = 0;

	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	if (cc || pmtx_rec.tx_call_no != read_call || 
	    strcmp(pmtx_rec.tx_mail_type, "A"))
		return(0);

	return(1);
}

/*--------------------------------------
| Fix edge of boxes with 'T' junctions |
--------------------------------------*/
int
fix_edge (
 int line_no)
{
	move(0,line_no);
	PGCHAR(10);
	move(79,line_no);
	PGCHAR(11);
	return (EXIT_SUCCESS);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int scn)
{
	char	blnk_line[79];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (scn == 1)	
		{
			if ( clear_ok )
				clear();

			clear_ok = TRUE;

			box(0,3,80,1); 

			rv_pr(ML(mlMenuMess129),(80 - strlen(ML(mlMenuMess129))) / 2,0,1);

			move (0, 1);
			line(80);
		}

		if (scn == 2)
		{
			clear();

			box(0,1,80,19); 

			rv_pr(ML(mlMenuMess129),(80 - strlen(ML(mlMenuMess129))) / 2,1,1);
			if (rdrw_txt)
			{
				tab_display("disp_text", TRUE);
				move(1,6);
				line(79);
				fix_edge(6);
			}
		
			move(1,6);
			line(79);
			fix_edge(6);
		}

		if (scn == 4)
		{
			box (0,18,80,2);
			sprintf(blnk_line, "%-78.78s", " ");
			rv_pr(blnk_line, 1,19,0);
			rv_pr(blnk_line, 1,20,0);
		}

		if (scn == 5)
		{
			box (0,20,79,1);
		}
	
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

int
erase_scn (
 int scn_no)
{
	int	i;

	for (i = 13; i < 21; i++)
	{
		rv_pr("                                                                     ", 2, i, 0);
	}

	return(0);
}

int
mail_notify (
 char *	who)
{
	char	curr_time [6];

	strcpy (curr_time, TimeHHMM());

	if (local_rec.dly_mail[0] == 'Y' && NOT_ACTIVE)
		return (EXIT_SUCCESS);

	sprintf (cmd, "psl_mctrl 5 %s", who);
	sys_exec (cmd);
	return (EXIT_SUCCESS);
}


/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_mail.c 	)                                 |
|  Program Desc  : ( Logistic Mailing System.                     )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (03/04/91)      | Author       : Campbell Mander   |
|---------------------------------------------------------------------|
|  Date Modified : (10/04/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (08/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (18/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (10/06/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (01/07/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (16/07/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (13/08/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (04/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (09/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (10/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (24/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/10/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (20/11/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (30/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (31/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (12/02/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (20/03/92)      | Modified  by : US FELLAS.        |
|  Date Modified : (05/11/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (07/12/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (08/02/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (10/03/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (21/06/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (25/06/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (21/09/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (07/12/94)      | Modified  by : Dirk Heinsius.    |
|  Date Modified : (05/09/97)      | Modified  by : Roanna Marcelino. |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (10/04/91) - fixed FN1 from tabular screen, CC to  |
|                : the person mail is TO, cust description and        |
|                : changed from using who to using getutent().        |
|  (03/05/91)    : Changed strncmp to strcmp so that users trev and   |
|                : trevor (for example) can both exist and receive    |
|                : mail.                                              |
|  (08/05/91)    : Priority is now displayed on selection screen (ie  |
|                : B4 choosing mail to read you can see the priority).|
|                : Recipient is now notified when actions are added to|
|                : mail.                                              |
|                : Mail is put into a sorted linked list before being |
|                : added to the table.  Most recent mail is now at    |
|                : the top of the list.                               |
|                : Adding actions now flags archived mail as unseen   |
|                : but does not notify the person who has the         |
|                : archived mail.                                     |
|  (17/05/91)    : Pipe now closed after mail is printed.             |
|  (18/05/91)    : CC user name can now be separated by either spaces |
|                : or commas.  Screen 3 + 4 and "disp_text" table are |
|                : now redrawn after search on user to and CC.        |
|  (10/06/91)    : Added the option to send a message directly to a   |
|                : users terminal. Also if a mail has actions the     |
|                : user is told that the mail has actions when they   |
|                : read that piece of mail.                           |
|  (01/07/91)    : Added the option to send delayed mail.             |
|  (16/07/91)    : Changed order of parameters to txt_open().         |
|  (13/08/91)    : Fixed FN1 from add_actions.                        |
|  (04/09/91)    : Added ability to read a text file into TXT screen. |
|  (09/09/91)    : Fixed reading of file.                             |
|  (10/09/91)    : Fixed restart on TXT screen.                       |
|  (24/09/91)    : Removed redundant routine.                         |
|  (17/10/91)    : Put out .EOF after printing a piece of mail.       |
|  (20/11/91)    : Fix update of last read date. Update mail to seen  |
|                : before tab_scan. Clear message when exiting from   |
|                : read_mail and re_display if required. Remove most  |
|                : abc_selfields and open and alias'ed file instead.  |
|  (05/12/91)    : Update for cumr phone_no length. SC 6297 PSL.      |
|  (30/12/91)    : Added ability to send mail from a format file      |
|                : Mail can now be sent in background rather than     |
|                : interactively.                                     |
|  (31/12/91)    : Change "disp_text" to "dsp_txt" as temp filename   |
|                : created in /usr/LS10.5/WORK was too long for SCO box.  |
|  (12/02/92)    : Change method of forwarding mail. A new piece of   |
|                : mail is no longer created, instead the mail is     |
|                : CC'd to an extra user.                             |
|  (20/03/92)    : Change sys_exec() on message to system().          |
|  (05/11/92)    : Changes as per SC 7898 PSL.                        |
|  (07/12/92)    : Fix small bug with mail printing brought about by  |
|                : mods for SC 7898 PSL.                              |
|  (08/02/93)    : Remove all references to FULL pathnames. PSL 8431  |
|  (10/03/93)    : Fix -f option where user names are checked.        |
|                : SC 8668 PSL.                                       |
|  (21/06/93)    : PSL 8951. Fix problem found by SBD on SCO box.     |
|  (25/06/93)    : PSL 9236 - allow dump of mail to $HOME/MAIL FN9 key|
|  (21/09/93)    : HGP 9864. Increase sumr_cont_no to 15 chars.       |
|  (07/12/94)    : PSL 11494 Fix problem where program is unable to   |
|                : locate a valid supplier                            |
|  (05/09/97)    : Modified for Multilingual Conversion.              |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: psl_mail.c,v $
| Revision 5.5  2002/07/17 09:57:26  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/08/28 08:46:20  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/14 02:50:00  scott
| Updated for new delete wizard
|
| Revision 5.2  2001/08/09 05:13:44  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:36  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:51  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:07  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/09/07 02:31:25  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.1  2000/09/06 07:49:28  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:00:29  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/02/18 01:56:28  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.14  1999/12/06 01:47:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/16 09:42:01  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.12  1999/10/20 02:06:50  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/09/29 10:11:15  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:27:08  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/15 02:36:54  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: psl_mail.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_mail/psl_mail.c,v 5.5 2002/07/17 09:57:26 scott Exp $";

#define		X_OFF		0
#define		Y_OFF		0

#define		TXT_REQD
#include 	<pslscr.h>
#include 	<hot_keys.h>
#include 	<ring_menu.h>
#include 	<get_lpno.h>
#include 	<sys/types.h>
#include 	<signal.h>
#include 	<utmp.h>
#include 	<minimenu.h>
#include 	<getnum.h>
#include 	<sys/stat.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>
#include 	<tabdisp.h>

#define		ACTIONS		( !DETAILS )
#define 	USR_NULL 	( (struct USR_PTR *) NULL )
#define 	GRP_NULL 	( (struct GRP_PTR *) NULL )
#define 	READ_NULL 	( (struct READ_PTR *) NULL )

#define		M_PADDING	1
#define		M_REPLY		2
#define		M_ADD_ACTIONS	3
#define		M_ARCHIVE	4
#define		M_FORWARD	5
#define		M_DELETE	6
#define		M_EXIT		7
#define		M_RUN_SUB	9

#define	NOT_ACTIVE	(pmre_rec.re_act_date > local_rec.lsystemDate || \
			 (pmre_rec.re_act_date == local_rec.lsystemDate && \
			  strcmp(pmre_rec.re_act_time, curr_time) > 0) )

#define		U_REPLY		(user_perm.can_reply == TRUE)
#define		U_FORWARD	(user_perm.can_forward == TRUE)
#define		U_SEND		(user_perm.can_send == TRUE)
#define		U_UTILS		(user_perm.utils_ok == TRUE)
#define		U_SUB_MENU	(user_perm.sub_menu_ok == TRUE)

extern	int	_mail_ok;

FILE *	fin;
FILE *	fout;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
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
		{"pmse_to"},
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

	int	pmse_no_fields = 15;

	struct	{
		long	se_call_no;
		int	se_nx_seq;
		int	se_termno;
		char	se_sender[15];
		char	se_to[15];
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

	char	*comm    = "comm",
	    	*pmse    = "pmse",
	    	*pmse2 = "pmse2",
	    	*pmre    = "pmre",
	    	*pmre2 = "pmre2",
	    	*pmtx    = "pmtx",
	    	*cumr    = "cumr",
	    	*cumr2 = "cumr2",
	    	*sumr    = "sumr",
	    	*sumr2 = "sumr2";

	MENUTAB main_menu [] =
	{
		{ "   1. Send Mail.                  ", "" },
		{ "   2. Read Incoming Mail.         ", "" },
		{ "   3. Read Archived Mail.         ", "" },
		{ "   4. Check Mail Sent.            ", "" },
		{ "   5. Logistic Utilities.         ", "" },
		{ "   6. Exit Mail System.           ", "" },
		{ ENDMENU }
	};

	MENUTAB util_menu [] =
	{
		{ "   1. Return to Main Menu.        ", "" },
		{ "   2. Send Message.               ", "" },
		{ "   3. Logistic Scratch Pad.       ", "" },
		{ "   4. Phone Diary Maintenance.    ", "" },
		{ "   5. Phone Diary Display/Print.  ", "" },
		{ ENDMENU }
	};

/*============================ 
| Local & Screen Structures. |
============================*/
int	ALL_OPT;	/* TRUE if one of the 'ALL' options is chosen 	     */
int	ALL_LOG;	/* TRUE if mail to be sent to logged in users 	     */
int	ALL_USR;	/* TRUE mail is to be sent to all users 	     */
int	DETAILS;	/* TRUE if user currently viewing details	     */
int	MORE_MAIL;	/* TRUE if there is more mail to view 		     */
int	REPLY;		/* TRUE if user is replying to mail   		     */
int	FORWARD;	/* TRUE if user is forwarding mail    		     */
int	CHK_SENT;	/* TRUE if user is checking mail sent 		     */
int	read_ok;	/* TRUE if ok to read mail chosen in choose function */
int	valid_user;
int	no_tagged;
int	envDbFind;
int	cr_find;
int	envDbCo;
int	exit_loop;
int	exit_choose;
int	window;
int	win_lines;
int	show_hot_keys;
int	any_delete_ok;
int	clear_ok = TRUE;

long	read_call;	/* Call number to be read */
long 	show_call;

char	branchNumber[3];
char	log_name[15];
char	usr_fname[100];
char	grp_fname[100];
char	curr_status[2];
char	chk_status[2];
char 	tagged_name[350];
char	cmd[256];
char	send_file[100];
char *	psl_path;

struct	{
	char	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	char	from[15];
	char	to[15];
	char	CC[61];
	char	subject[41];
	int		priority;
	char	dly_mail[4];
	long	dly_date;
	char	dly_time[6];
	char	mail_src[9];
	char	src_no[7];
	int		link_hash;
	char	src_no_desc[43];
	char	phone_no[16];
	char	contact_name[26];
	char	dflt_phone_no[16];
	char	dflt_contact_name[26];
	char	call_dtls[76];
	char	call_no[7];
	char	src_prmt[32];
	char	date_prmt[34];
	char	rcvd_date[19];
	char	message[65];
	char	text[61];
	char	read_file[51];
} local_rec;

struct
{
	int	can_reply;
	int	can_forward;
	int	can_send;
	int	utils_ok;
	int	sub_menu_ok;
} user_perm;

struct	READ_PTR
{
	long	call_no;
	int		priority;
	char	sender[16];
	char	subject[46];
	struct	READ_PTR *	next;
};

struct	GRP_PTR
{
	char	grp_name[15];
	struct	USR_PTR	*usr_head;
	struct	GRP_PTR	*next;
};

struct	USR_PTR
{
	char	usr_name[15];
	struct	USR_PTR	*next;
};

/*---------------------------------------
| Four linked lists are used 		|
| 1) Lists of users from Mail_secure	|
| 2) List of currently logged in users	|
| 4) Lists of users to send mail to	|
---------------------------------------*/
struct  USR_PTR *usr_head;
struct  USR_PTR *usr_curr;
struct  USR_PTR *log_head = USR_NULL;
struct  USR_PTR *log_curr;
struct  USR_PTR *snd_head;
struct  USR_PTR *snd_curr;

struct  GRP_PTR *grp_head = GRP_NULL;
struct  GRP_PTR *grp_curr;

struct	READ_PTR	*read_ptr;
struct	READ_PTR	*read_head = READ_NULL;

struct	utmp	*uptr;

static int	user_tag_func	(int, KEY_TAB *);
static int	show_group		(int, KEY_TAB *);
static int	exit_func		(int, KEY_TAB *);
static int	read_slct_func	(int, KEY_TAB *);
static int	show_func		(int, KEY_TAB *);
static int	delete_func		(int, KEY_TAB *);
static int	details_func	(int, KEY_TAB *);
static int	actions_func	(int, KEY_TAB *);
static int	mail_prnt_func	(int, KEY_TAB *);
static int	dummy_func		(int, KEY_TAB *);
static int	run_sub			(int, KEY_TAB *);
static int	dump_mail		(int, KEY_TAB *);
static int	delete_mail		(int, KEY_TAB *);

int reply			(void);
int	add_actions		(void);
int	archive_mail	(void);
int	forward			(void);
int	chk_mail		(void);
int	toggle_text		(void);
int	dummy_func2		(void);
int	delete_mail2	(void);
int	mail_prnt_func2 (void);
int	run_sub2		(void);
int	dump_mail2		(void);

static	KEY_TAB user_keys [] =
{
   { "[T]AG",		'T', user_tag_func,
	"Tag/Untag current user for CC.",				"A" },
   { "[G]ROUP",		'G', show_group,
	"Show All Users In A Group.",					"G" },
   { NULL,		'\r', user_tag_func,
	"Tag/Untag current user for CC.",				"A" },
   { NULL,		FN16, exit_func,
	"Selection of users complete.",					"A" },
   END_KEYS
};

static	KEY_TAB read_keys [] =
{
   { "[S]HOW USERS",	'S', show_func,
	"Show list of users that mail was sent to.",			"C" },
   { "[D]ELETE MAIL",	'D', delete_mail,
	"Delete mail.",							"D" },
   { NULL,		'\r', read_slct_func,
	"Select piece of mail for reading.",				"A" },
   { NULL,		FN1, dummy_func,
	"",								"A" },
   { NULL,		FN16, exit_func,
	"Exit from current option.",					"A" },
   END_KEYS
};

static KEY_TAB show_keys [] =
{
   { "[D]ELETE MAIL",	'D', delete_func,
	"Delete mail from a user.",					"A" },
   { NULL,		'\r', delete_func,
	"Delete mail from a user.",					"A" },
   END_KEYS
};

static	KEY_TAB text_keys [] =
{
   { "[D]ETAILS",	'D',  details_func,
	"View details for current mail.",				"A" },
   { "[A]CTIONS",	'A',  actions_func,
	"View actions for current mail.",				"A" },
   { NULL,    	FN9,  dump_mail,
	"",								"A" },
   { "[PRINT]",    	FN5,  mail_prnt_func,
	"Print current mail.",						"A" },
   { "[SUB MENU]",    	FN8,  run_sub,
	"Call Sub Menu.",						"S" },
   { NULL,		FN16, exit_func,
	"",								"A" },
   { NULL,		FN1,  dummy_func,
	"",								"A" },
   END_KEYS
};

static KEY_TAB null_keys [] =
{
   END_KEYS
};

/*---------------------------------
| Ring menu definition structure. |
---------------------------------*/
 menu_type	act_menu [] = {
	{"      ",		    "", dummy_func2,	"",   0, DISABLED  },
	{"                     ",   "", dummy_func2,	"",   0, DISABLED  },
	{"<Reply>",		    "", reply, 		"Rr", 0,    ALL	   },
	{"<Add Actions>",	    "", add_actions, 	"Aa", 0,    ALL	   },
	{"<Archive>",		    "", archive_mail, 	"Aa", 0,    ALL	   },
	{"<Forward>",		    "", forward, 	"Ff", 0,    ALL	   },
	{"<Delete>",		    "", delete_mail2, 	"Dd", 0,    ALL	   },
	{"<Exit>",		    "", chk_mail, 	"Ee", FN16, ALL	   },
	{" ", 			    "", mail_prnt_func2,"",   FN5,  SELECT },
	{" ", 			    "", run_sub2, 	"",   FN8,  SELECT },
	{" ", 			    "", dump_mail2, 	"",   FN9,  SELECT },
	{" ", 			    "", toggle_text,    "Tt", 0,    SELECT },
	{"",								   },
};

static	struct	var	vars [] =
{
	{1, LIN, "to",	 3, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", ""," To           :","Enter name of user to send mail to.",
		YES, NO,  JUSTLEFT, "", "", local_rec.to},
	{1, LIN, "from",	 2, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", " From         :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.from},
	{1, LIN, "priority",	 3, 60, INTTYPE,
		"N", "          ",
		" ","5", " Priority :", " 1 - 5  (1 = Urgent action required) ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.priority},
	{1, LIN, "CC",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " CC           :", "CC mail to whom.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.CC},
	{1, LIN, "subject",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Subject      :", "Enter the subject of the mail.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.subject},
	{1, LIN, "dly_mail",	 7, 15, CHARTYPE,
		"U", "          ",
		" ", "N", " Delay Mail :", " Delay send time of mail ",
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.dly_mail},
	{1, LIN, "dly_date",	 7, 38, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Delayed Date:  ", " Delay mail until what date ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.dly_date},
	{1, LIN, "dly_time",	 7, 64, CHARTYPE,
		"AA:AA", "          ",
		" ", "", "       Time:", " Delay mail until what time ",
		 ND, NO,  JUSTLEFT, "0123456789 ", "", local_rec.dly_time},
	{1, LIN, "mail_src",	 8, 15, CHARTYPE,
		"U", "          ",
		" ", "N", " Mail Source:", " C(ustomer) S(upplier) N(one) ",
		 NO, NO,  JUSTLEFT, "CSN", "", local_rec.mail_src},
	{1, LIN, "src_no",	 9, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " Source No  :", " Customer/Supplier Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.src_no},
	{1, LIN, "src_no_desc",	 9, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_no_desc},
	{1, LIN, "phone_no",	10, 15, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", local_rec.dflt_phone_no, " Phone No   :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.phone_no},
	{1, LIN, "contact_name",	10, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.dflt_contact_name, "Contact Name :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.contact_name},

	{2, LIN, "call_no",	 2, 15, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "Call Number  :", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.call_no},
	{2, LIN, "r_from",	 2, 60, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "From     :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.from},
	{2, LIN, "r_to",	 3, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "To           :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.to},
	{2, LIN, "r_priority",	 3, 60, INTTYPE,
		"N", "          ",
		" ", "5", "Priority :", " 1 - 5  (1 = Urgent action required) ",
		 NO, NO,  JUSTLEFT, "1", "5", (char *)&local_rec.priority},
	{2, LIN, "r_subject",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Subject      :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.subject},
	{2, LIN, "r_CC",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "CC           :", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.CC},
	{2, LIN, "r_src_no",	 7, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", local_rec.src_prmt, "",
		YES, NO,  JUSTLEFT, "", "", local_rec.src_no},
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

	{3, LIN, "f_CC",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Forward To   :", "List Of Users To Forward Mail To.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.CC},

	{4, LIN, "mess_to",	19, 11, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Send To: ", "Send message to whom.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.to},
	{4, LIN, "message",	20, 11, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Message: ", "Message to send.",
		YES, NO,  JUSTLEFT, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-'`:;?/<>{} !@#$%^&*()\\+~=[]|_", "", local_rec.message},

	{5, TXT, "mail_text",	12, 10, 0,
		"", "          ",
		" ", "", "C a l l   D e t a i l s.", "",
		8, 60, 100, "", "", local_rec.text},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};


/*============================
| Local function prototypes  |
============================*/
int					send_from_file		(void);
int					mail_opt_menu		(void);
int					proc_utils			(void);
int					draw_menu			(int);
void				shutdown_prog		(void);
void				OpenDB				(void);
void				CloseDB			(void);
int					spec_valid			(int);
int					get_source			(char *, char *, int);
int					read_file			(void);
int					parse_line			(char *);
int					send_message		(void);
int					run_scratch			(void);
int					run_phone			(int);
int					send_mail			(void);
void				get_old_detls		(void);
int					read_mail			(void);
int					choose_mail			(void);
struct READ_PTR *	read_alloc			(void);
int					clr_read_list		(struct READ_PTR *);
int					clr_usr_list		(struct USR_PTR *);
int					load_mail			(void);
int					get_CC				(void);
int					load_text			(void);
int					opt_ring_menu		(void);
int					flag_unseen			(char *, int *);
int					notify_sender		(void);
int					all_deleted			(long);
int					chk_user			(char *, int);
int					chk_CC_user			(char *);
int					srch_user			(char *, int);
int					srch_CC				(char *);
int					get_all_users		(void);
int					set_options			(char *, struct USR_PTR *);
struct USR_PTR *	usr_alloc			(void);
int					load_groups			(void);
int					proc_group			(char *, char *);
int					add_to_grp			(char *, char *);
struct GRP_PTR *	grp_alloc			(void);
void				prnt_text			(char *);
int					post_mail			(int);
void				add_text			(char *, long);
int					create_send_list	(void);
int					get_logged_in		(void);
int					is_group			(char *);
struct USR_PTR *	expand_group		(char *, struct USR_PTR *);
struct USR_PTR *	add_to_send_list	(char *, struct USR_PTR *);
int					has_actions			(long);
int					fix_edge			(int);
int					heading				(int);
int					clear_mail_msg		(void);
int					mail_notify			(char *);

#include <FindCumr.h>
#include <FindSumr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	got_usr = FALSE;
	int	got_sfile = FALSE;
	extern	int	optind;
	extern	char	*optarg;

	_mail_ok = FALSE;
	sprintf (log_name, "%-14.14s", getenv("LOGNAME"));

	if (argc >= 2)
	{
	    while ((cc = getopt (argc, argv, "u:f:")) != EOF)
	    {
		switch (cc)
		{
		case 'u':
			if (getuid() == 0)
			{
				sprintf(log_name, "%-14.14s", optarg);
				got_usr = TRUE;
			}
			break;

		case 'f':
			sprintf(send_file, "%s", optarg);
			got_sfile = TRUE;
			break;
		}
	    }
	}

	if (got_usr)
		got_sfile = FALSE;

	psl_path = getenv("PROG_PATH");

	sprintf(usr_fname, "%s/BIN/MENUSYS/Mail_secure", 
				(psl_path) ? psl_path : "/usr/LS10.5");

	sprintf(grp_fname, "%s/BIN/MENUSYS/Mail_group", 
				(psl_path) ? psl_path : "/usr/LS10.5");

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	tab_row = 11;
	tab_col = 8;

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/

	/*-----------------------
	| Interactive mail. Not |
	| Using a send file.    |
	-----------------------*/
	if (!got_sfile)
	{
		clear ();
		draw_menu (TRUE);

		set_help (FN6, "FN6");
	
		fl_pr("   PLEASE WAIT  ", 29, 12, 1);
	}

	/*---------------------------
	| Open main database files. |  
	---------------------------*/
	OpenDB ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*-----------------------------
	| Load users from Mail_secure |
	-----------------------------*/
	get_all_users ();

	/*-------------------
	| Load user groups. |
	-------------------*/
	load_groups ();

	/*-------------------------------------------
	| Check that current user is valid for mail |
	-------------------------------------------*/
	if (!valid_user)
	{
		print_mess (ML (mlStdMess139));
		sleep (sleepTime);
		clear ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Allow user to select option |
	-----------------------------*/
	if (got_sfile)
	{
		if (!send_from_file())
		{
			sys_err("Error in send file format During Send Mail From File", errno, PNAME);
		}
	}
	else
		mail_opt_menu ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*-------------------------
| Send Mail Based On File |
-------------------------*/
int
send_from_file (
 void)
{
	int		i;
	char *	sptr;
	char *	tptr;
	char	send_line [100];

	if ((fin = fopen(send_file, "r")) == (FILE *)NULL)
		return(FALSE);

	lcount [5] = 0;
	init_vars(5);
	i = 0;
	sptr = fgets(send_line, 80, fin);
	while (sptr)
	{
		if (*sptr == '#')
		{
			sptr = fgets(send_line, 80, fin);
			continue;
		}
		*(sptr + strlen(sptr) - 1) = '\0';
		i++;

		switch (i)
		{
		/*------
		| From |
		------*/
		case 1:
			sprintf(local_rec.from, "%-14.14s", sptr);
			break;
		/*----
		| To |
		----*/
		case 2:
			sprintf(local_rec.to, "%-14.14s", sptr);
			if (chk_user(clip(local_rec.to), TRUE))
				return(FALSE);
			break;
		/*----------
		| Priority |
		----------*/
		case 3:
			local_rec.priority = atoi(sptr);
			break;
		/*-------
		| CC To |
		-------*/
		case 4:
			sprintf(local_rec.CC, "%-60.60s", sptr);
			if (strlen(clip(local_rec.CC)) == 0)
				break;

			if (chk_CC_user (local_rec.CC))
				return(FALSE);
			break;
		/*---------
		| Subject |
		---------*/
		case 5:
			sprintf(local_rec.subject, "%-40.40s", sptr);
			break;
		/*------------
		| Delay Mail |
		------------*/
		case 6:
			sprintf(local_rec.dly_mail, "%-1.1s", sptr);
			if (local_rec.dly_mail[0] == 'Y')
			{
			    tptr = sptr;
			    while (*tptr && *tptr != '\t')
				tptr++;
			    if (*tptr)
			    {
				tptr++;
				local_rec.dly_date = StringToDate(tptr);
			    	while (*tptr && *tptr != '\t')
				    tptr++;
			    	if (*tptr)
				{
				    tptr++;
				    sprintf(local_rec.dly_time, "%-5.5s", tptr);
				}
			    }
			    else
				strcpy(local_rec.dly_mail, "N");
			}
			break;
		/*-------------
		| Mail Source |
		-------------*/
		case 7:
			sprintf(local_rec.mail_src, "%-1.1s", sptr);
			if (local_rec.mail_src[0] != 'N')
			{
			    tptr = sptr;
			    while (*tptr && *tptr != '\t')
				tptr++;
			    if (*tptr)
			    {
				tptr++;
				sprintf(local_rec.src_no, "%-6.6s", tptr);
				if ( get_source(local_rec.mail_src, local_rec.src_no, FALSE) )
				{
				    strcpy(local_rec.mail_src, "N");
				    sprintf(local_rec.src_no, "%-6.6s", " ");
				    local_rec.link_hash = 0L;
				}
			    }
			}
			break;
		/*-------------------------
		| Phone No & Contact Name |
		-------------------------*/
		case 8:
			tptr = sptr;
			while (*tptr && *tptr != '\t')
				tptr++;
			if (*tptr)
			{
				*tptr = '\0';
				tptr++;
				sprintf(local_rec.contact_name,"%-25.25s",tptr);
			}

			sprintf(local_rec.phone_no, "%-15.15s", sptr);
			break;

		default:
			sprintf(local_rec.text, "%-60.60s", sptr);
			putval(lcount[5]++);
			break;
		}

		sptr = fgets(send_line, 80, fin);
	}

	post_mail (FALSE);

	return (TRUE);
}

/*-------------------
| Main Option Menu. |
-------------------*/
int
mail_opt_menu (
 void)
{
	int	exit_loop = FALSE;

	MENU_ROW = 9;
	MENU_COL = 21;

	draw_menu (FALSE);
	
	while (exit_loop == FALSE)
	{
		REPLY    = FALSE;
		FORWARD  = FALSE;
		CHK_SENT = FALSE;
		ALL_LOG  = FALSE;
		ALL_USR  = FALSE;

		crsr_off();
		mmenu_print( (char *)0, main_menu, 0);

		switch ( mmenu_select( main_menu ) )
		{
			case 0 :
				if (send_mail())
				{
					clear ();
					draw_menu (FALSE);
				}
				break;
	
			case 1 :
				/*-------------------
				| Read Current mail |
				-------------------*/
				strcpy(curr_status,"C");
				clear_mail_msg ();
				read_mail ();
				clear_mail_msg ();
				mail_notify (log_name);
				clear ();
				draw_menu (FALSE);
				break;

			case 2 :
				/*--------------------
				| Read archived mail |
				--------------------*/
				strcpy(curr_status, "A");
				read_mail ();
				clear ();
				draw_menu (FALSE);
				break;
	
			case 3:
				/*-----------------
				| Check mail sent |
				-----------------*/
				CHK_SENT = TRUE;
				if (read_mail())
				{
					clear();
					draw_menu(FALSE);
				}
				break;

			case 4:
				proc_utils ();
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
	return (EXIT_SUCCESS);
}

/*-----------------
| Utilities Menu. |
-----------------*/
int
proc_utils (
 void)
{
	int	exit_loop = FALSE;

	/*----------------------
	| Check user security. |
	----------------------*/
	if (!U_UTILS)
	{
		/*print_mess("\007 You Do Not Have Security Access For These Utilities. ");*/
		print_mess(ML(mlStdMess140));
		sleep(2);
		clear_mess();
		return (FALSE);
	}

	while (exit_loop == FALSE)
	{
		crsr_off();
		mmenu_print( (char *)0, util_menu, 0);

		switch ( mmenu_select( util_menu ) )
		{
			case 0:
			case 99:
			case -1:
				exit_loop = TRUE;	
				break;
			case 1:
				send_message();
				clear();
				draw_menu(FALSE);
				break;

			case 2 :
				clear();
				run_scratch();
				draw_menu( FALSE );
				break;

			case 3 :
				clear ();
				run_phone (TRUE);
				draw_menu (FALSE);
				break;

			case 4 :
				clear ();
				run_phone (FALSE);
				draw_menu (FALSE);
				break;

			default :
				break;
		}
	}
	return (TRUE);
}

/*----------------
| Draw main menu |
----------------*/
int
draw_menu (
 int flag)
{
	char	*xx_str = ": ..................................";

	crsr_off();
	box(4,2,70,17);
	box(9,5,60,12);
	box(60,6,7,2);
	rv_pr("SEL", 62, 7, 0);
	rv_pr("70c", 62, 8, 0);

	us_pr(ML(mlMenuMess129), 20,3,1);
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
	return (EXIT_SUCCESS);
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
	abc_dbopen ("data");

	abc_alias(cumr2, cumr);
	abc_alias(sumr2, sumr);
	abc_alias(pmre2, pmre);
	abc_alias(pmse2, pmse);

	open_rec(pmre,  pmre_list, pmre_no_fields, "pmre_id_no");
	open_rec(pmre2, pmre_list, pmre_no_fields, "pmre_call_no");
	open_rec(pmse,  pmse_list, pmse_no_fields, "pmse_call_no");
	open_rec(pmse2, pmse_list, pmse_no_fields, "pmse_id_no");
	open_rec(pmtx,  pmtx_list, pmtx_no_fields, "pmtx_id_no");
	open_rec(cumr,  cumr_list, cumr_no_fields, "cumr_id_no");
	open_rec(cumr2, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec(sumr,  sumr_list, sumr_no_fields, "sumr_id_no");
	open_rec(sumr2, sumr_list, sumr_no_fields, "sumr_hhsu_hash");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(pmre);
	abc_fclose(pmre2);
	abc_fclose(pmse);
	abc_fclose(pmtx);
	abc_fclose(cumr);
	abc_fclose(cumr2);
	abc_fclose(sumr);
	abc_fclose(sumr2);
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	int		i;
	char	tmp_time [6];
	char *	tptr;
	int	tmp_hour;
	int	tmp_min;

	if (LCHECK("to"))
	{
		if (SRCH_KEY)
		{
			srch_user (temp_str, TRUE);
			return (EXIT_SUCCESS);
		}

		ALL_LOG = FALSE;
		ALL_USR = FALSE;

		return (chk_user(temp_str, TRUE));
	}

	if (LCHECK("priority"))
	{
		if (FLD("priority") == NA)
			return(0);

		if (REPLY && prog_status == ENTRY && last_char == FN16)
		{
			entry_exit = TRUE;
			prog_exit = TRUE;
			return(0);
		}

		if (atoi(temp_str) > 5 || atoi(temp_str) < 1)
		{
			putchar(BELL);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK("CC"))
	{
		if (FLD("CC") == NA)
			return(0);

		if (SRCH_KEY)
		{
			clear_ok = FALSE;
			srch_CC (temp_str);
			if (prog_status != ENTRY)
				scn_display(5);
			scn_set(1);
			return (EXIT_SUCCESS);
		}

		ALL_LOG = FALSE;
		ALL_USR = FALSE;

		return (chk_CC_user(temp_str));
	}
	
	if (LCHECK("f_CC"))
	{
		if (prog_status == ENTRY && last_char == FN16)
		{
			entry_exit = TRUE;
			prog_exit = TRUE;
			return(0);
		}

		if (SRCH_KEY)
		{
			clear_ok = FALSE;
			srch_CC (temp_str);

			heading (2);
			scn_write (2);
			scn_display (2);
			scn_write (3);
			scn_display (3);
			tab_display ("dsp_txt", TRUE);

			return(0);
		}

		ALL_LOG = FALSE;
		ALL_USR = FALSE;
	
		return (chk_CC_user(temp_str));
	}
	
	if (LCHECK("dly_mail"))
	{
		if (local_rec.dly_mail[0] == 'Y')
		{
			strcpy(local_rec.dly_mail, "Yes");
			FLD("dly_date") = NO;
			FLD("dly_time") = NO;
			display_prmpt(label("dly_date"));
			display_prmpt(label("dly_time"));

			DSP_FLD("dly_mail");
			return(0);
		}
		else
		{
			strcpy(local_rec.dly_mail, "No ");
			local_rec.dly_date = 0L;
			strcpy(local_rec.dly_time, "  :  ");
			FLD("dly_date") = ND;
			FLD("dly_time") = ND;
			rv_pr("                                                    ", 22,7,0);
			DSP_FLD("dly_mail");
			return(0);
		}
	}

	if (LCHECK("dly_date"))
	{
		if (FLD("dly_date") == NA || FLD("dly_date") == ND)
			return(0);

		if (dflt_used)
		{
			local_rec.dly_date = local_rec.lsystemDate;
			DSP_FLD("dly_date");
			return(0);
		}

		if (local_rec.dly_date < local_rec.lsystemDate)
		{
			/*print_mess("\007 Date must be today or later ");*/
			print_mess(ML(mlStdMess141));
			sleep(2);
			clear_mess();

			return(1);
		}

		return(0);
	}

	if (LCHECK("dly_time"))
	{
		if (FLD("dly_time") == NA || FLD("dly_time") == ND)
			return(0);

		strcpy (tmp_time, TimeHHMM());

		if (dflt_used)
		{
			strcpy (local_rec.dly_time, TimeHHMM ());
			DSP_FLD("dly_time");
			return(0);
		}

		if ((strcmp(local_rec.dly_time, tmp_time) < 0) && 
		    (local_rec.dly_date == local_rec.lsystemDate))
		{
			/*print_mess("\007 Time must be greater than current time ");*/
			print_mess(ML(mlStdMess142));
			sleep(2);
			clear_mess();

			return(1);
		}

		/*---------------------------
		| Replace spaces with zeros |
		---------------------------*/
		sprintf(tmp_time, "%-5.5s", local_rec.dly_time);
		tptr = tmp_time;
		i = 0;
		while (*tptr)
		{
			if (*tptr == ' ' && i != 2)
				*tptr = '0';

			i++;
			tptr++;
		}
		sprintf(local_rec.dly_time, "%-5.5s", tmp_time);
		local_rec.dly_time[2] = ':';

		tmp_hour = atoi(local_rec.dly_time);
		tmp_min = atoi(local_rec.dly_time + 3);

		if (tmp_hour > 23 || tmp_min > 59)
		{
			/*print_mess("\007 Invalid Time ");*/
			print_mess(ML(mlStdMess142));
			sleep(2);
			clear_mess();

			return(1);
		}

		DSP_FLD("dly_time");
		return(0);
	}

	if (LCHECK("mail_src"))
	{
		if (FLD("mail_src") == NA)
			return(0);

		switch (local_rec.mail_src[0])
		{
		case 'C':
			strcpy (local_rec.mail_src, "Customer");
			FLD("src_no") = YES;
			FLD("src_no_desc") = NA;
			envDbFind = atoi(get_env("DB_FIND"));
			envDbCo = atoi(get_env("DB_CO"));
			strcpy (branchNumber, (envDbCo) ? comm_rec.test_no : " 0");
			break;

		case 'S':
			strcpy (local_rec.mail_src, "Supplier");
			cr_find = atoi(get_env("CR_FIND"));
			envDbCo = atoi(get_env("CR_CO"));
			FLD("src_no") = YES;
			FLD("src_no_desc") = NA;
			strcpy (branchNumber, (envDbCo) ? comm_rec.test_no : " 0");
			break;

		case 'N':
			strcpy (local_rec.mail_src, "None    ");
			FLD("src_no") = NA;
			FLD("src_no_desc") = NA;
			sprintf(local_rec.dflt_phone_no, "%-15.15s", " ");
			sprintf(local_rec.dflt_contact_name, "%-25.25s", " ");
			strcpy(local_rec.src_no, "      ");
			DSP_FLD("src_no");
			sprintf(local_rec.src_no_desc, "%-42.42s", " ");
			DSP_FLD("src_no_desc");
			local_rec.link_hash = 0L;
			break;
		default:
			break;
		}

		DSP_FLD("mail_src");
	}

	if (LCHECK("src_no"))
	{
		if (FLD("src_no") == NA)
			return(0);

		if (SRCH_KEY)
		{
			if (local_rec.mail_src[0] == 'C')
				CumrSearch (comm_rec.tco_no, branchNumber, temp_str);

			if (local_rec.mail_src[0] == 'S')
				SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		return( get_source(local_rec.mail_src, temp_str, TRUE) );
	}

	if (LCHECK("mess_to"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_user (local_rec.to, FALSE);
			for (i = 0; i < 18; i++)
				rv_pr("                                    ",0,i,0);
			draw_menu(FALSE);
			heading(4);
			return(0);
		}

		if (chk_user(clip(local_rec.to), FALSE))
		{
			/*rv_pr("\007 Not A Valid User ", 30, 23,1);*/
			rv_pr(ML(mlStdMess139), 30, 23,1);
			sleep(2);
			move(0,23);
			cl_line();
			return(1);
		}
	}

	if (LCHECK("mail_text"))
	{
		if (SRCH_KEY)
		{
			read_file ();
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*----------------------------------------
| Get information about the mail source. |
----------------------------------------*/
int
get_source (
 char *	src_type,
 char *	src_no,
 int	disp)
{
	if (src_type[0] == 'N')
		return (EXIT_FAILURE);

	if (src_type[0] == 'C')
	{
		strcpy(local_rec.src_no, pad_num(src_no));
		if (disp)
			DSP_FLD("src_no");
		abc_selfield(cumr, "cumr_id_no3");
		sprintf(cumr_rec.cm_dbt_no, "%-6.6s", src_no);
		strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			if (!disp)
				return (EXIT_FAILURE);

			/*print_mess("Customer Not Found On File");*/
			print_mess(ML(mlStdMess021));
			putchar(BELL);
			sleep(1);
			clear_mess();
			return (EXIT_FAILURE);
		}
		local_rec.link_hash = cumr_rec.cm_hhcu_hash;
		sprintf(local_rec.src_no_desc, "(%-40.40s)", 
					cumr_rec.cm_name);

		sprintf(local_rec.dflt_phone_no, "%-15.15s", 
					cumr_rec.cm_phone_no);

		sprintf(local_rec.dflt_contact_name, "%-25.25s", 
					cumr_rec.cm_contact_name);
	}
	else
	{
		strcpy(local_rec.src_no, pad_num(src_no));
		if (disp)
			DSP_FLD("src_no");
		abc_selfield(sumr, "sumr_id_no3");
		sprintf(sumr_rec.sm_crd_no, "%-6.6s", src_no);
		strcpy(sumr_rec.sm_co_no, comm_rec.tco_no);
		cc = find_rec(sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			if (!disp)
				return(1);
			/*print_mess("Supplier Not Found On File");*/
			print_mess(ML(mlStdMess022));
			putchar(BELL);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.link_hash = sumr_rec.sm_hhsu_hash;
		sprintf(local_rec.src_no_desc, "(%-40.40s)", 
						sumr_rec.sm_name);

		sprintf(local_rec.dflt_phone_no, "%-15.15s", 
						sumr_rec.sm_cont_no);

		sprintf(local_rec.dflt_contact_name, "%-25.25s", 
						sumr_rec.sm_cont_name);
	}
	if (disp)
		DSP_FLD("src_no_desc");
	return (EXIT_SUCCESS);
}

/*-------------------------------
| Get a filename and 'suck' file|
| into TXT screen.              |
-------------------------------*/
int
read_file (
 void)
{
	char *	sptr;
	char	tmp_text [65];
	struct	stat file_info;

	/*rv_pr(" File Name To Read: ", 1, 23, 1);*/
	rv_pr(ML(mlMenuMess209), 1, 23, 1);
	getalpha(22,
		 23,
		 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
		 local_rec.read_file);
	
	if (last_char == FN1)
	{
		move (0, 23);
		cl_line ();
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Check If Filename is a directory |
	----------------------------------*/
	if (stat(local_rec.read_file, &file_info) ||
	    (!stat(local_rec.read_file, &file_info) &&
	     !(file_info.st_mode & S_IFREG )))
	{
		/*print_mess("\007 Can't Access Requested File ");*/
		print_mess(ML(mlMenuMess115));
		sleep(2);
		clear_mess();
		move(0,23);
		cl_line();
		return(0);
	}

	if ((fin = fopen(local_rec.read_file,"r")) == 0)
	{
		/*print_mess("\007 Can't Access Requested File ");*/
		print_mess(ML(mlMenuMess115));
		sleep(2);
		clear_mess();
		move(0,23);
		cl_line();
		return(0);
	}

	sptr = fgets(tmp_text,62, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		parse_line(tmp_text);

		sprintf(local_rec.text, "%-60.60s", tmp_text);

		putval ( -1 );

		sptr = fgets(tmp_text,62, fin);
	}

	fclose (fin);

	scn_display (5);
	move (0, 23);
	cl_line ();
	return (EXIT_SUCCESS);
}

/*-------------------------------------------
| Check a line of text for valid characters |
-------------------------------------------*/
int
parse_line (
 char *	chk_line)
{
	char *	sptr;

	sptr = chk_line;
	while(*sptr)
	{
		if (*sptr < ' ' || *sptr > '~')
			*sptr = ' ';
		sptr++;
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
	char nam_str [11];

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

		sprintf( nam_str, "[%-8.8s]", log_name);

		if (!strncmp(local_rec.to, "ALL USERS", 9))
		{
			sprintf (cmd, 
				"message -g \042%-64.64s %s\042", 
				local_rec.message,
				nam_str);
		}
		else
		{
			sprintf (cmd, "message -u%s \042%-64.64s %s\042", 
					clip(local_rec.to), 
					local_rec.message,
					nam_str);
		}
		system( cmd );
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
run_scratch (
 void)
{
	sys_exec ("psl_s_pad 1");
	return (EXIT_SUCCESS);
}

/*------------------
| Run Phone Diary. |
------------------*/
int
run_phone (
 int	_phone)
{
	if ( _phone )
		sys_exec ("psl_phone");
	else
		sys_exec ("psl_pdisp");
	return (EXIT_SUCCESS);
}

/*--------------------------------------
| Get input from user and send mail to |
| requested users				       |
--------------------------------------*/
int
send_mail (
 void)
{
	/*----------------------
	| Check user security. |
	----------------------*/
	if (!U_SEND && !REPLY)
	{
		/*print_mess("\007 You Do Not Have Security Access To Send Mail. ");*/
		print_mess(ML(mlStdMess140));
		sleep(2);
		clear_mess();
		return (FALSE);
	}

	strcpy(local_rec.read_file, "");

	FLD("to") = YES;
	FLD("CC") = NO;
	FLD("subject") = NO;
	FLD("mail_src") = NO;
	FLD("src_no") = YES;
	FLD("phone_no") = NO;
	FLD("contact_name") = NO;

	while (TRUE)
	{
		FLD("dly_date") = ND;
		FLD("dly_time") = ND;

		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;

		lcount[5] = 0;
		init_vars(1);
		init_vars(5);

		if (REPLY)
			get_old_detls();

		/*----------------------------------
		| Enter screen 1 linear input.     |
		----------------------------------*/
		heading(1);
		if (REPLY)
		{
			init_ok = FALSE;
			scn_display(1);
		}

		entry(1);
		if (prog_exit)
			break;

		if (restart)
			continue;
	
		do
		{
			restart = FALSE;
			entry(5);
			if (!restart && lcount[5] == 0 )
			{
				/*rv_pr("\007 Cannot send blank mail ", 25, 23,1);*/
				rv_pr(ML(mlMenuMess199), 25, 23,1);
				sleep(2);
				move(0,23);
				cl_line();
			}
		} while (!restart && lcount[5] == 0);

		if (restart)
			continue;

		sprintf(local_rec.from, "%-14.14s", log_name);
		
		crsr_off();
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		scn_write( 1 );
		scn_display( 1 );
		do
		{
			entry(5);
			if (!restart && lcount[5] == 0 )
			{	
				rv_pr("\007 Cannot send blank mail ", 25, 23,1);
				sleep(2);
				move(0,23);
				cl_line();
			}
		} while (!restart && lcount[5] == 0);

		if (restart)
			continue;

		post_mail (TRUE);
		break;
	}
	return (TRUE);
}

/*-------------------------------
| Get details from mail so that	|
| a REPLY can be sent		|
-------------------------------*/
void
get_old_detls (
 void)
{
	long	hash;

	FLD("to") = NA;
	FLD("CC") = NO;
	FLD("subject") = NA;
	FLD("mail_src") = NA;
	FLD("src_no") = NA;
	FLD("src_no_desc") = NA;
	FLD("phone_no") = NA;
	FLD("contact_name") = NA;

	sprintf(local_rec.from,         "%-14.14s", log_name);
	sprintf(local_rec.to,           "%-14.14s", pmse_rec.se_sender);
	sprintf(local_rec.subject,      "%-40.40s", pmse_rec.se_subject);
	sprintf(local_rec.phone_no,     "%-15.15s", pmse_rec.se_phone_no);
	sprintf(local_rec.contact_name, "%-25.25s", pmse_rec.se_contact_name);
	hash = pmse_rec.se_link_hash;
	local_rec.link_hash = hash;
	if (pmse_rec.se_link_type[0] == 'C')
	{
		strcpy(local_rec.mail_src, "Customer");
		cc = find_hash(cumr2, &cumr2_rec, COMPARISON, "r", hash);
		if (cc)
		{
			strcpy(local_rec.src_no, "      ");
			sprintf(local_rec.src_no_desc, "(%-40.40s)",  " ");
		}
		else
		{
			sprintf(local_rec.src_no, "%-6.6s",  
						cumr2_rec.cm_dbt_no);

			sprintf(local_rec.src_no_desc, "(%-40.40s)",  
						cumr2_rec.cm_name);
		}
		abc_selfield(cumr, "cumr_id_no");
	}

	if (pmse_rec.se_link_type[0] == 'S')
	{
		strcpy(local_rec.mail_src, "Supplier");
		cc = find_hash(sumr2, &sumr2_rec, COMPARISON, "r", hash);
		if (cc)
		{
			strcpy(local_rec.src_no, "      ");
			sprintf(local_rec.src_no_desc, "(%-40.40s)",  " ");
		}
		else
		{
			sprintf(local_rec.src_no, "%-6.6s",  
						sumr2_rec.sm_crd_no);
			sprintf(local_rec.src_no_desc, "(%-40.40s)",  
						sumr2_rec.sm_name);
		}
		abc_selfield(sumr, "sumr_id_no");
	}

	if (pmse_rec.se_link_type[0] == 'N')
	{
		strcpy(local_rec.mail_src,     "None    ");
		sprintf(local_rec.src_no,      "%-6.6s",   " ");
		sprintf(local_rec.src_no_desc, "%-42.42s", " ");
	}
}

/*----------------------------------
| Read mail (incoming or archived) |
| Also used for checking mail sent |
----------------------------------*/
int
read_mail (
 void)
{
	/*----------------------
	| Check user security. |
	----------------------*/
	if (!U_SEND && CHK_SENT)
	{
		/*print_mess("\007 You Do Not Have Security Access To Send Mail. ");*/
		print_mess (ML(mlStdMess140));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	/*-------------------------------
	| Choose piece of mail to read. |
	-------------------------------*/
	while (choose_mail())
	{
		if (!read_ok)
			continue;

		DETAILS = TRUE;
		load_mail ();

		if (CHK_SENT)		
			strcpy(local_rec.date_prmt, "Delayed Until:");
		else
			strcpy(local_rec.date_prmt, "Received Date:");

		heading (2);
		scn_display (2);

		/*----------------------------
		| Enable / Disable Sub Menu. |
		----------------------------*/
		if (U_SUB_MENU)
			set_keys(text_keys, "S", KEY_ACTIVE);
		else
			set_keys(text_keys, "S", KEY_PASSIVE);

		show_hot_keys = TRUE;
		any_delete_ok = TRUE;
		exit_loop = FALSE;
		while (!exit_loop)
		{
			load_text ();

			fix_edge(10);

			tab_scan("dsp_txt");
			if (!exit_loop)
				tab_close("dsp_txt", TRUE);
		}
		
		/*------------------------------
		| Update mail to seen if user  |
		| is not checking mail sent.   |
		------------------------------*/
		if (!CHK_SENT)
		{
			/*--------------------------------------
			| Check that mail has not been deleted |
			--------------------------------------*/
			strcpy (pmre_rec.re_status, curr_status);
			sprintf (pmre_rec.re_receiver, "%-14.14s", log_name);
			pmre_rec.re_call_no = read_call;
	
			cc = find_rec(pmre, &pmre_rec, COMPARISON, "r");
			if (cc)
			{
				/*rv_pr("\007 Mail has been deleted ", 25,23,1);*/
				rv_pr(ML(mlMenuMess122), 25,23,1);
				sleep(2);	
				move (0,23);
				cl_line();
			}
			else
			{
				move (0, 23);
				cl_line ();
				opt_ring_menu ();
			}
		}
		else
		{
			strcpy(chk_status, "S");
			chk_mail ();
		}

		crsr_on();
		tab_close("dsp_txt", TRUE);

		if (!MORE_MAIL)
			break;
	}

	clear();

	return(TRUE);
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
	int	no_to_read = 0;
	struct	READ_PTR	*lcl_read;
	struct	READ_PTR 	*tmp_ptr;
	struct	READ_PTR 	*pos_ptr = (struct READ_PTR *) 0;

	strcpy (curr_time, TimeHHMM());

	while (TRUE)
	{
		if (read_head != READ_NULL)
			clr_read_list(read_head);
	
		read_head = READ_NULL;

		clear();
		if (CHK_SENT)
			strcpy(tmp_str, "Mail Sent");
		else
			if (curr_status[0] == 'C')
				strcpy(tmp_str, "Incoming Mail");
			else
				strcpy(tmp_str, "Archived Mail");

		sprintf(err_str,
			ML(mlMenuMess200),
			tmp_str);
		rv_pr(err_str,(80 - strlen(err_str)) / 2,1,1);

		if (CHK_SENT)
		{
			set_keys(read_keys, "C", KEY_ACTIVE);
			set_keys(read_keys, "D", KEY_PASSIVE);
			no_to_read = 0;
			tab_open("read", read_keys, 4, 0, 14, FALSE);
			tab_add("read",
				"# %-7.7s %-3.3s %-16.16s   %-45.45s ", 
				"Call #", 
				"Pri", 
				"   When Sent ", 
				"             SUBJECT");
	
			sprintf (pmse_rec.se_sender, "%-14.14s", log_name);
			pmse_rec.se_termno = 0;
			pmse_rec.se_date = 0L;
			strcpy(pmse_rec.se_time, "00:00:00");
	
			/*------------------------------------------
			| List of mail is put into a sorted linked |
			| list and displayed with most recent mail |
			| at the top of the list                   |
			------------------------------------------*/
			cc = find_rec(pmse2, &pmse_rec, GTEQ, "r");
			while (!cc && !strcmp(pmse_rec.se_sender, log_name))
			{
				if (strcmp(pmse_rec.se_status, "D") && 
				    !all_deleted(pmse_rec.se_call_no))
				{
				    if (read_head == READ_NULL)
				    {
					lcl_read = read_alloc();
					lcl_read->next = READ_NULL;
					read_head = lcl_read;
				    }
				    else
				    {
				    	if (pmse_rec.se_call_no > read_head->call_no)
					{
					    lcl_read = read_alloc();
					    lcl_read->next = read_head;
					    read_head = lcl_read;
					}
					else
					{
					    tmp_ptr = read_head;
					    while (tmp_ptr != READ_NULL)
					    {
					        if (pmse_rec.se_call_no > tmp_ptr->call_no)
						    break;

						pos_ptr = tmp_ptr;
						tmp_ptr = tmp_ptr->next;
					    }
					    lcl_read = read_alloc();
					    lcl_read->next = pos_ptr->next;
					    pos_ptr->next = lcl_read;
					}
				    }

				    lcl_read->call_no = pmse_rec.se_call_no;
				    lcl_read->priority = pmse_rec.se_priority;

					sprintf(lcl_read->sender, 
						" %10.10s %-5.5s", 
						DateToString(pmse_rec.se_date), 
						pmse_rec.se_time);

				    sprintf(lcl_read->subject, "%45.45s", pmse_rec.se_subject);
				}
	
				cc = find_rec(pmse2, &pmse_rec, NEXT, "r");
			}

			/*-------------------------------------
			| Put linked list contents into table |
			-------------------------------------*/
			read_ptr = read_head;
			while (read_ptr != READ_NULL)
			{
				tab_add("read",
					" %06ld   %1d  %-15.15s    %-45.45s ",
					read_ptr->call_no,
					read_ptr->priority,
					read_ptr->sender,
					read_ptr->subject);
				no_to_read++;

				read_ptr = read_ptr->next;
			}
		}
		else
		{
			set_keys(read_keys, "C", KEY_PASSIVE);
			set_keys(read_keys, "D", KEY_ACTIVE);
			no_to_read = 0;
			tab_open("read", read_keys, 4, 6, 14, FALSE);
			tab_add("read",
				"# %-6.6s %-3.3s   %-14.14s  %-40.40s ", 
				"Call #", 
				"Pri", 
				"     FROM     ", 
				"             SUBJECT");
	
			sprintf(pmre_rec.re_receiver, "%-14.14s", log_name);
			strcpy(pmre_rec.re_status, curr_status);
			pmre_rec.re_call_no = 0L;
			cc = find_rec(pmre, &pmre_rec, GTEQ, "r");
			while (!cc && !strcmp(pmre_rec.re_receiver, log_name)
			       && !strcmp(pmre_rec.re_status, curr_status))
			{
				if (NOT_ACTIVE && curr_status[0] == 'C')
				{
					cc = find_rec(pmre, &pmre_rec, NEXT, "r");
					continue;
				}

				hash = pmre_rec.re_call_no;
				cc = find_hash(pmse, &pmse_rec, COMPARISON, "r", hash);
				if (cc)
				{
					cc = find_rec(pmre, &pmre_rec, NEXT, "r");
					continue;
				}
	
				if (pmre_rec.re_seen[0] == 'N')
					strcpy(tmp_seen, "*");
				else
					strcpy(tmp_seen, " ");
		
				read_ptr = read_alloc();
				read_ptr->next = read_head;

				read_ptr->call_no = pmse_rec.se_call_no;
				read_ptr->priority = pmse_rec.se_priority;
				sprintf(read_ptr->sender, "%-1.1s%-14.14s", tmp_seen, pmse_rec.se_sender);
				sprintf(read_ptr->subject, "%40.40s",pmse_rec.se_subject);

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
					" %06ld  %1d   %-15.15s  %-40.40s ",
					read_ptr->call_no,
					read_ptr->priority,
					read_ptr->sender,
					read_ptr->subject);
				no_to_read++;

				read_ptr = read_ptr->next;
			}
		}

		any_delete_ok = FALSE;
		if (no_to_read > 0)
		{
			MORE_MAIL = TRUE;
	
			exit_choose = FALSE;
			tab_scan("read");
			tab_close("read", TRUE);
			clear();

			if (exit_choose || !MORE_MAIL)
				return(MORE_MAIL);
			else
				continue;
		}
		else
		{
			sprintf(err_str, 
			     "%-21.21s%s", 
			     " ", 
			     (CHK_SENT) ? "     ***   NO MAIL SENT   ***" : "*** NO PREVIOUS MAIL *** "); 

			tab_add("read", err_str);
			tab_display("read", TRUE);
			putchar(BELL);
			fflush(stdout);
			sleep(3);
			clear();
			tab_close("read", TRUE);
			return (EXIT_SUCCESS);
		}
	}
	return (EXIT_SUCCESS);
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
        	sys_err("Error allocating memory for read list During (MALLOC)",errno,PNAME);
		
	return (lcl_ptr);
}

/*------------------
| Clear read list. |
------------------*/
int
clr_read_list (
 struct READ_PTR *	lcl_head)
{
	struct	READ_PTR *	lcl_ptr;
	struct	READ_PTR *	tmp_ptr;

	lcl_ptr = lcl_head;
	while (lcl_ptr != READ_NULL)
	{
		tmp_ptr = lcl_ptr;
		lcl_ptr = lcl_ptr->next;
		free (tmp_ptr);
	}
	return (EXIT_SUCCESS);
}

/*-----------------
| Clear log list. |
-----------------*/
int
clr_usr_list (
 struct USR_PTR *	lcl_head)
{
	struct	USR_PTR	*lcl_ptr;
	struct	USR_PTR	*tmp_ptr;

	lcl_ptr = lcl_head;
	while (lcl_ptr != USR_NULL)
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

	cc = find_hash(pmse, &pmse_rec, COMPARISON, "r", read_call);
	if (cc)
	{
		/*rv_pr("\007 Mail has been deleted ", 25,23,1);*/
		rv_pr (ML(mlMenuMess122), 25, 23, 1);
		sleep (sleepTime);	
		move (0, 23);
		cl_line ();
		return (EXIT_SUCCESS);
	}

	sprintf(local_rec.call_no, "%06ld", pmse_rec.se_call_no);
	sprintf(local_rec.from, "%-14.14s", pmse_rec.se_sender);
	sprintf(local_rec.to,   "%-14.14s", pmse_rec.se_to);
	sprintf(local_rec.subject, "%-40.40s", pmse_rec.se_subject);
	local_rec.priority = pmse_rec.se_priority;
	sprintf(local_rec.subject, "%-40.40s", pmse_rec.se_subject);
	sprintf(local_rec.phone_no, "%-15.15s", pmse_rec.se_phone_no);
	sprintf(local_rec.contact_name, "%-25.25s", pmse_rec.se_contact_name);

	/*------------------------------
	| Get received date and update |
	| pmre record to seen          |
	------------------------------*/
	sprintf (pmre_rec.re_receiver, "%-14.14s", log_name);
	sprintf(pmre_rec.re_status, "%-1.1s", curr_status);
	pmre_rec.re_call_no = read_call;
	cc = find_rec(pmre, &pmre_rec, COMPARISON, "u");
	if (!cc)
	{
		sprintf (local_rec.rcvd_date, 
				"%-10.10s   %-5.5s", 
				DateToString(pmre_rec.re_act_date), 
				pmre_rec.re_act_time);

		if (CHK_SENT)
			abc_unlock(pmre);
		else
		{
			strcpy (pmre_rec.re_seen, "Y");
			if (curr_status[0] != 'A')
			{
				pmre_rec.re_lst_date = TodaysDate ();
				strcpy (pmre_rec.re_lst_time, TimeHHMM());
			}

			if (pmre_rec.re_fst_date == 0L)
			{
				pmre_rec.re_fst_date = TodaysDate ();
				strcpy (pmre_rec.re_fst_time, TimeHHMM());
			}

			cc = abc_update(pmre, &pmre_rec);
			if (cc)
				sys_err("Error in pmre during (DBUPDATE)",cc,PNAME);
		}
	}
	else
	{
		sprintf (local_rec.rcvd_date, 
				"%-10.10s   %-5.5s", 
				DateToString(pmse_rec.se_date), 
				pmse_rec.se_time);
	}

	/*---------------------
	| Find all CC'd users |
	---------------------*/
	get_CC();

	FLD("r_src_no") = ND;
	FLD("r_src_no_desc") = ND;
	strcpy(local_rec.mail_src, "None    ");
	sprintf(local_rec.src_no_desc, "%-40.40s", " ");

	if (pmse_rec.se_link_type[0] == 'C')
	{
		FLD("r_src_no") = NA;
		FLD("r_src_no_desc") = NA;
		strcpy(local_rec.src_prmt, "Customer No:");
		hash = pmse_rec.se_link_hash;
		local_rec.link_hash = hash;
		cc = find_hash(cumr2, &cumr2_rec, COMPARISON, "r", hash);
		if (cc)
			sprintf(local_rec.src_no_desc, "(%-40.40s)", "Customer Not Found On File ");
		else
		{
			sprintf(local_rec.src_no, "%-6.6s", cumr2_rec.cm_dbt_no);
			sprintf(local_rec.src_no_desc, "(%-40.40s)", cumr2_rec.cm_name);
		}
	}

	if (pmse_rec.se_link_type[0] == 'S')
	{
		FLD("r_src_no") = NA;
		FLD("r_src_no_desc") = NA;
		strcpy(local_rec.src_prmt, "Supplier No:");
		hash = pmse_rec.se_link_hash;
		local_rec.link_hash = hash;
		cc = find_hash(sumr2, &sumr2_rec, COMPARISON, "r", hash);
		if (cc)
			sprintf(local_rec.src_no_desc, "(%-40.40s)", "Supplier Not Found On File ");
		else
		{
			sprintf(local_rec.src_no, "%-6.6s", sumr2_rec.sm_crd_no);
			sprintf(local_rec.src_no_desc, "(%-40.40s)", sumr2_rec.sm_name);
		}
	}

	return (EXIT_SUCCESS);
}

/*--------------------------------------
| Get all CC'd mail for the mail being |
| read and put the user names of all   |
| those CC'd to in one field	       |
--------------------------------------*/
int
get_CC (
 void)
{
	int	first_time;
	char *	sptr;
	char *	tptr;
	char	tmp_name [15];
	char	tmp_CC [4096];

	strcpy (local_rec.CC, "");
	strcpy (tmp_CC, "");
	first_time = TRUE;

	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "r", read_call);
	while (!cc && pmre2_rec.re_call_no == read_call)
	{
		if (pmre2_rec.re_cc_flag[0] == 'Y' && 
		    pmre2_rec.re_status[0] != 'D') 
		{
			clip (pmre2_rec.re_receiver);
			if (first_time)
			{
				sprintf(err_str, "%s", pmre2_rec.re_receiver);
				first_time = FALSE;
			}
			else
				sprintf(err_str, ", %s", pmre2_rec.re_receiver);

			if (pmre2_rec.re_seen[0] == 'N')
				strcat (err_str, "*");

			strcat (tmp_CC, err_str);
		}
		cc = find_hash(pmre2, &pmre2_rec, NEXT, "r", read_call);
	}

	sprintf(local_rec.CC, "%-60.60s", tmp_CC);
	sptr = strrchr(local_rec.CC, ',');
	if (sptr)
	{
		sprintf(tmp_name, "%-14.14s", sptr + 1);
		tptr = strrchr(tmp_name, '*');
		if (tptr)
			*tptr = '\0';
		else
			clip(tmp_name);
			
		lclip(tmp_name);
	
		if (chk_user(tmp_name, TRUE))
			*sptr = '\0';
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------
| Load mail text into tabular screen |
------------------------------------*/
int
load_text (
 void)
{
	char	actions_flag[12];
	char	curr_mail_type[2];
	int	first_time = TRUE;
	int	old_seq_no = 0;

	if (DETAILS)
	{
		if (has_actions(read_call))
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

	tab_open("dsp_txt", text_keys, 10, 0, 8, FALSE);
	sprintf(err_str, 
		"#                     Current Selection %s                   %-11.11s", 
		(DETAILS) ? "D(etails)" : "A(ctions)",
		actions_flag);

	tab_add("dsp_txt", err_str);
	
	pmtx_rec.tx_call_no = read_call;
	strcpy(pmtx_rec.tx_mail_type, curr_mail_type);
	pmtx_rec.tx_line_no = 0;

	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	while (!cc && pmtx_rec.tx_call_no == read_call && 
	       !strcmp (pmtx_rec.tx_mail_type, curr_mail_type)) 
	{
		if (ACTIONS && pmtx_rec.tx_seq_no != old_seq_no)
		{
			old_seq_no = pmtx_rec.tx_seq_no;
			if (!first_time)
				tab_add("dsp_txt", "%-78.78s", " ");

			sprintf(err_str,
					"ACTIONED (%-10.10s %-5.5s) BY %-14.14s",
					DateToString(pmtx_rec.tx_date),
					pmtx_rec.tx_time,
					pmtx_rec.tx_sender);
			tab_add("dsp_txt", "  %-76.76s", err_str);

		}

		tab_add("dsp_txt", 
			"%-9.9s%-60.60s%-9.9s", 
			" ", 
			pmtx_rec.tx_text, 
			" ");

		first_time = FALSE;
		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*------------------------------
| Allow user to choose options |
| from ring menu			   |
------------------------------*/
int
opt_ring_menu (
 void)
{
	/*--------------------------------
	| Enable / Disable menu options. |
	--------------------------------*/
	act_menu[0].flag = SHOW;
	if (curr_status[0] == 'C')
	{
		act_menu[M_PADDING].flag     = DISABLED;
		act_menu[M_ADD_ACTIONS].flag = VALID;
		act_menu[M_ARCHIVE].flag     = ALL;

		if (U_REPLY)
			act_menu[M_REPLY].flag       = ALL;
		else
			act_menu[M_REPLY].flag       = SHOW;

		if (U_FORWARD)
			act_menu[M_FORWARD].flag     = VALID;
		else
			act_menu[M_FORWARD].flag     = SHOW;
	}
	else
	{
		act_menu[M_PADDING].flag     = SHOW;
		act_menu[M_ADD_ACTIONS].flag = DISABLED;
		act_menu[M_ARCHIVE].flag     = DISABLED;
		act_menu[M_REPLY].flag       = DISABLED;
		act_menu[M_FORWARD].flag     = DISABLED;
	}

	if (U_SUB_MENU)
		act_menu[M_RUN_SUB].flag     = SELECT;
	else
		act_menu[M_RUN_SUB].flag     = DISABLED;
	
	strcpy(chk_status, curr_status);

	/*---------------
	| Erase hotkeys |
	---------------*/
	move(2,21);
	line(76);

	/*---------------------
	| Set initial options |
	---------------------*/
	crsr_off();

	show_hot_keys = FALSE;
	run_menu(act_menu, "", 22);

	return (EXIT_SUCCESS);
}

/*------------------------
| REPLY to mail received |
------------------------*/
int
reply (
 void)
{
	REPLY = TRUE;
	FORWARD = FALSE;
	last_char = 0;
	send_mail ();
	return (EXIT_SUCCESS);
}

/*----------------------
| Forward current mail |
| to other users.      |
----------------------*/
int
forward (
 void)
{
	struct	USR_PTR	*snd_lcl;

	while (TRUE)
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
	
		FORWARD = TRUE;
		REPLY = FALSE;
		init_vars(3);

		heading(3);
		scn_display(3);
		init_ok = 0;
		entry(3);
		if (prog_exit)
		{
			move(10, 1);
			line(60);
			scn_write(2);
			scn_display(2);
			strcpy(err_str,
			     ML(mlMenuMess129));
			rv_pr(err_str,(80 - strlen(err_str)) / 2,1,1);

			return(FALSE);
		}

		heading(3);
		scn_display(3);
		edit(3);

		if (restart)
			continue;

		break;
	}

	strcpy(local_rec.dly_mail, "N");

	/*-----------------------------------------------
	| Create list of users who are to receive mail. |
	-----------------------------------------------*/
	create_send_list ();

	/*-------------------------
	| Create receiver records |
	-------------------------*/
	snd_lcl = snd_head;
	while (snd_lcl != USR_NULL)
	{
		if (strlen(snd_lcl->usr_name) == 0)
		{
			snd_lcl = snd_lcl->next;
			continue;
		}

		/*--------------------------------------
		| Deleted mail gets updated to current |
		--------------------------------------*/
		sprintf(pmre_rec.re_receiver, "%-14.14s", snd_lcl->usr_name);
		strcpy(pmre_rec.re_status, "D");
		pmre_rec.re_call_no = read_call;
		cc = find_rec(pmre, &pmre_rec, COMPARISON, "u");
		if (!cc)
		{
			strcpy(pmre_rec.re_status, "C");
			strcpy(pmre_rec.re_seen, "N");
			cc = abc_update(pmre, &pmre_rec);
			if (cc)
				file_err(cc, pmre, "DBUPDATE");

			mail_notify (snd_lcl->usr_name);
			snd_lcl = snd_lcl->next;
			continue;
		}

		/*---------------------------------
		| Archived mail excludes the user |
		---------------------------------*/
		sprintf(pmre_rec.re_receiver, "%-14.14s", snd_lcl->usr_name);
		strcpy(pmre_rec.re_status, "A");
		pmre_rec.re_call_no = read_call;
		cc = find_rec(pmre, &pmre_rec, COMPARISON, "r");
		if (!cc)
		{
			snd_lcl = snd_lcl->next;
			continue;
		}

		/*---------------------------------
		| Try to add record. If it exists |
		| then exclude the user.          |
		---------------------------------*/
		sprintf(pmre_rec.re_receiver, "%-14.14s", snd_lcl->usr_name);
		strcpy(pmre_rec.re_status, "C");
		pmre_rec.re_call_no = read_call;
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
       	         	       sys_err("Error in pmre During (DBADD)",cc,PNAME);
		}
		else
			mail_notify (snd_lcl->usr_name);
	
		snd_lcl = snd_lcl->next;
	}
	get_CC ();

	move(10, 1);
	line(60);
	scn_write(2);
	scn_display(2);
	strcpy(err_str,ML(mlMenuMess129));
	rv_pr(err_str, (80 - strlen(err_str)) / 2, 1, 1);

	MORE_MAIL = TRUE;
	return (TRUE);
}

/*-----------------------------
| Add actions to current mail |
-----------------------------*/
int
add_actions (
 void)
{
	int	sender_notified;
	
	strcpy(local_rec.read_file, "");

	/*---------------------
	| Reset Control Flags |
	---------------------*/
	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	search_ok = 1;
	init_ok = 1;

	lcount[5] = 0;
	init_vars(5);
		
	entry(5);

	if (restart)
	{
		restart = FALSE;

		/*--------------
		| Fix display. |
		--------------*/
		tab_display("dsp_txt", TRUE);
		fix_edge(10);
		return(0);
	}

	add_text ("A", read_call);
	
	sender_notified = FALSE;
	flag_unseen("C", &sender_notified);

	flag_unseen("A", &sender_notified);

	if (!sender_notified)
		notify_sender ();
	
	/*--------------
	| Fix display. |
	--------------*/
	tab_display("dsp_txt", TRUE);
	fix_edge (10);

	return (EXIT_SUCCESS);
}

/*----------------------------
| Set current mail to unseen |
| for all users it has been  |
| sent to.		     |
----------------------------*/
int
flag_unseen (
 char *	flg_status,
 int *	sender_notified)
{
	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "u", read_call);
	while (!cc && pmre2_rec.re_call_no == read_call)
	{
		if (pmre2_rec.re_seen[0] == 'Y' && 
		    !strcmp(pmre2_rec.re_status, flg_status))
		{
			if (!strcmp(pmre2_rec.re_receiver, log_name))
			{
				pmre2_rec.re_lst_date = TodaysDate ();
				strcpy (pmre2_rec.re_lst_time, TimeHHMM ());
			}
			else
				strcpy (pmre2_rec.re_seen, "N");

			if (!strcmp(pmse_rec.se_sender, pmre2_rec.re_receiver))
			{
				if (pmre2_rec.re_status[0] == 'A')
					strcpy (pmre2_rec.re_status, "C");
				*sender_notified = TRUE;
			}

			cc = abc_update(pmre2, &pmre2_rec);
			if (cc)
                		sys_err("Error in pmre During (DBUPDATE)",cc,PNAME);
			if (pmre2_rec.re_status[0] == 'C')
				mail_notify (pmre2_rec.re_receiver);

		}
		abc_unlock(pmre2);
		cc = find_hash(pmre2, &pmre2_rec, NEXT, "r", read_call);
	}
	abc_unlock(pmre2);
	MORE_MAIL = TRUE;
	
	return (EXIT_SUCCESS);
}

/*------------------------
| Notify sender of mail. |
------------------------*/
int
notify_sender (
 void)
{
	int	sender_notified = FALSE;

	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "u", read_call);
	while (!cc && pmre2_rec.re_call_no == read_call)
	{
		if (!strcmp(pmse_rec.se_sender, pmre2_rec.re_receiver))
		{
			if ((pmre2_rec.re_status[0] == 'C' && 
			    pmre2_rec.re_seen[0] == 'N') ||
			    pmre2_rec.re_status[0] == 'D')
			{
				sender_notified = TRUE;
				break;
			}

			strcpy(pmre2_rec.re_status, "C");
			strcpy(pmre2_rec.re_seen, "N");

			cc = abc_update(pmre2, &pmre2_rec);
			if (cc)
                		sys_err("Error in pmre During (DBUPDATE)",cc,PNAME);
			mail_notify (pmre2_rec.re_receiver);
			sender_notified = TRUE;
			break;
		}

		abc_unlock(pmre2);
		cc = find_hash(pmre2, &pmre2_rec, NEXT, "u", read_call);
	}
	abc_unlock(pmre2);
	
	if (!sender_notified)
	{
		pmre_rec.re_call_no = read_call;
		sprintf(pmre_rec.re_receiver, "%-14.14s", pmse_rec.se_sender);
		strcpy(pmre_rec.re_status, "C");
		strcpy(pmre_rec.re_seen, "N");
		strcpy(pmre_rec.re_cc_flag, "Y");
		pmre_rec.re_act_date = TodaysDate ();
		strcpy (pmre_rec.re_act_time, TimeHHMM ());
		pmre_rec.re_fst_date = 0L;	
		strcpy(pmre_rec.re_fst_time, "00:00");
		pmre_rec.re_lst_date = 0L;	
		strcpy(pmre_rec.re_lst_time, "00:00");

		cc = abc_add(pmre, &pmre_rec);
		if (cc)
       	         	sys_err("Error in pmre During (DBADD)",cc,PNAME);
		else
			mail_notify (pmre_rec.re_receiver);

		sender_notified = TRUE;
	}

	return (EXIT_SUCCESS);
}

/*-------------
| Delete mail |
-------------*/
int
delete_mail2 (
 void)
{
	return (delete_mail (0, (KEY_TAB *) 0));
}

static int
delete_mail (
 int		iUnused,
 KEY_TAB *	psUnused)
{
	int		c;
	char	read_buf [100];
	char	tmp_call [7];

	/*------------------------------------
	| User is trying to delete a piece   |
	| of mail from the selection screen. |
	------------------------------------*/
	if (!any_delete_ok)
	{
		tab_get("read", read_buf, CURRENT, 0);
		sprintf(tmp_call, "%-6.6s", &read_buf[1]);
		read_call = atoi(tmp_call);

		sprintf(pmre_rec.re_receiver, "%-14.14s", log_name);
		strcpy (pmre_rec.re_status, curr_status);
		pmre_rec.re_call_no = read_call;
		cc = find_rec(pmre, &pmre_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Mail Has Been Deleted ");*/
			print_mess(ML(mlMenuMess122));
			sleep(2);
			clear_mess();
			return(0);
		}

		if (pmre_rec.re_seen[0] == 'N')
		{
			/*print_mess("\007 Mail Is Unread ");*/ 
			print_mess(ML(mlMenuMess123));
			sleep(2);
			clear_mess();
			return(0);
		}
	}

	sprintf (pmre_rec.re_receiver, "%-14.14s", log_name);
	strcpy (pmre_rec.re_status, curr_status);
	pmre_rec.re_call_no = read_call;
	cc = find_rec(pmre, &pmre_rec, COMPARISON, "u");

	/*c = prmptmsg("\007 Delete Mail ? ","YyNn",30,23);*/
	c = prmptmsg(ML(mlMenuMess194),"YyNn",30,23);
	if (c == 'N' || c == 'n')
	{
		act_menu[M_DELETE].flag = VALID;
		abc_unlock(pmre);
		move(0,23);
		cl_line();
		return(FALSE);
	}

	if (!cc)
	{
		strcpy (pmre_rec.re_status, "D");
		cc = abc_update(pmre, &pmre_rec);
		if (cc)
	                sys_err("Error in pmre During (DBUPDATE)",cc,PNAME);
	}
	
	strcpy(chk_status, curr_status);
	chk_mail ();

	act_menu[M_DELETE].flag = ALL;

	if (any_delete_ok)
		return(TRUE);
	else
		return(FN16);
}

/*--------------------------------
| Change mail status to archived |
--------------------------------*/
int
archive_mail (
 void)
{
	sprintf (pmre_rec.re_receiver, "%-14.14s", log_name);
	strcpy (pmre_rec.re_status, "C");
	pmre_rec.re_call_no = read_call;
	cc = find_rec(pmre, &pmre_rec, COMPARISON, "u");
	if (cc)
	{
		/*rv_pr("\007 Mail has been deleted by sender ", 25,23,1);*/
		rv_pr(ML(mlMenuMess122), 25,23,1);
		sleep(2);	

		move (0,23);
		cl_line();

		strcpy (chk_status, "C");
		chk_mail ();

		return (FALSE);
	}
	
	strcpy (pmre_rec.re_status, "A");
	cc = abc_update(pmre, &pmre_rec);
	if (cc)
                sys_err("Error in pmre During (DBUPDATE)",cc,PNAME);
	
	strcpy(chk_status, "C");
	chk_mail ();
	return (TRUE);
}

/*-----------------------------
| Check if there is more mail |
| for user to read	      |
-----------------------------*/
int
chk_mail (
 void)
{
	if (chk_status[0] == 'S')
	{
		MORE_MAIL = FALSE;
		sprintf (pmse_rec.se_sender, "%-14.14s", log_name);
		pmse_rec.se_termno = 0;
		pmse_rec.se_date = 0L;
		strcpy(pmse_rec.se_time, "00:00:00");
	
		cc = find_rec(pmse2, &pmse_rec, GTEQ, "r");
		while (!cc && 
		       !strcmp(pmse_rec.se_sender, log_name))
		{
			if (strcmp(pmse_rec.se_status, "D") && 
			    !all_deleted(pmse_rec.se_call_no))
			{
				MORE_MAIL = TRUE;
				break;
			}
			cc = find_rec(pmse2, &pmse_rec, NEXT, "r");
		}
	}
	else
	{
		MORE_MAIL = TRUE;
		sprintf(pmre_rec.re_receiver, "%-14.14s", log_name);
		strcpy(pmre_rec.re_status, chk_status);
		pmre_rec.re_call_no = 0L;
		cc = find_rec(pmre, &pmre_rec, GTEQ, "r");
		if (cc || strcmp(pmre_rec.re_receiver,log_name)
		       || strcmp(pmre_rec.re_status, chk_status))
			MORE_MAIL = FALSE;
	}
	return (EXIT_SUCCESS);
}

/*--------------------------------
| Check if all receivers of this |
| call have deleted it		 |
--------------------------------*/
int
all_deleted (
 long	call_no)
{
	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "r", call_no);
	while (!cc && pmre2_rec.re_call_no == call_no)
	{
		if (pmre2_rec.re_status[0] != 'D')
			return(FALSE);

		cc = find_hash(pmre2, &pmre2_rec, NEXT, "r", call_no);
	}

	/*----------------------
	| Set pmse status to D |
	----------------------*/
	cc = find_hash(pmse, &pmse_rec, COMPARISON, "u", call_no);
	if (cc)
               	sys_err("Error in pmse During (DBFIND)",cc,PNAME);

	strcpy(pmse_rec.se_status, "D");

	cc = abc_update(pmse, &pmse_rec);
	if (cc)
       		sys_err("Error in pmse During (DBUPDATE)",cc,PNAME);

	return (TRUE);
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
	struct	GRP_PTR	*grp_lcl;
	char	tmp_usr_name [15];

	if (!strcmp(key_val, "ALL LOGGED IN") && status)
	{
		ALL_LOG = TRUE;
		FLD("CC") = NA;
		strcpy(local_rec.CC, "");
		DSP_FLD("CC");
		return(0);
	}

	if (!strcmp(key_val, "ALL USERS"))
	{
		ALL_USR = TRUE;
		FLD("CC") = NA;
		strcpy(local_rec.CC, "");
		DSP_FLD("CC");
		return(0);
	}

	FLD("CC") = NO;

	if (FORWARD)
	{
		sprintf(tmp_usr_name, "%-14.14s", key_val);
		if (!strcmp(log_name, tmp_usr_name))
			return (EXIT_FAILURE);
	}

	/*------------------
	| Check user list. |
	------------------*/
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		if (!strcmp(clip(usr_lcl->usr_name), key_val))
			return (EXIT_SUCCESS);

		usr_lcl = usr_lcl->next;
	}

	/*-------------------
	| Check group list. |
	-------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(clip(grp_lcl->grp_name), key_val))
			return (EXIT_SUCCESS);

		grp_lcl = grp_lcl->next;
	}

	putchar (BELL);
	return (EXIT_FAILURE);
}

/*---------------------------------
| Check that all users entered as |
| a CC are valid users from 	  |
| Mail_secure					  |
---------------------------------*/
int
chk_CC_user (
 char *	key_val)
{
	char	*sptr;
	char	*tptr;
	char	tmp_name[61];
	char	tmp_usr_name[15];
	char	char_separator;
	int	lots_of_spaces;

	if (!strcmp(key_val, "ALL LOGGED IN"))
	{
		ALL_LOG = TRUE;
		return(0);
	}

	if (!strcmp(key_val, "ALL USERS"))
	{
		ALL_USR = TRUE;
		return (EXIT_SUCCESS);
	}

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

		/*------------------
		| Check user name. |
		------------------*/
		if (chk_user(tmp_name, TRUE))
		{
			sprintf(err_str, "%-14.14s", tmp_name);
			strcpy(tmp_name, clip(err_str));
			/*sprintf(err_str, "\007 %s is not a valid user or group ", tmp_name);*/
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

		if (FORWARD)
		{ 
			sprintf(tmp_usr_name, "%-14.14s", tmp_name);

			if (!strcmp(log_name, tmp_usr_name))
			{
				/*rv_pr("\007 Can't FORWARD mail to yourself ", 24,23,1);*/
				rv_pr(ML(mlMenuMess201), 24,23,1);
				sleep(2);
				move(0,23);
				cl_line();
				return(1);
			}
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
	struct	GRP_PTR	*grp_lcl;

	work_open ();
	save_rec("#User Name","# ");
	if (status)
		save_rec("ALL LOGGED IN","         ");
	save_rec("ALL USERS","         ");

	/*----------------------------
	| Load user names into srch. |
	----------------------------*/
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		sprintf(err_str, "%-14.14s", usr_lcl->usr_name);
		cc = save_rec(err_str, "         ");
		if (cc)
			break;
		usr_lcl = usr_lcl->next;
	}

	/*-----------------------------
	| Load group names into srch. |
	-----------------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		sprintf(err_str, "%-14.14s", grp_lcl->grp_name);
		cc = save_rec(err_str, "(GROUP) ");
		if (cc)
			break;
		grp_lcl = grp_lcl->next;
	}

	cc = disp_srch();
	work_close();

	return (EXIT_SUCCESS);
}

/*--------------------------------
| Search on users/groups for CC. |
--------------------------------*/
int
srch_CC (
 char *	key_val)
{
	int		i;
	int		no_in_tab;
	char *	sptr;
	char	user_buf[100];
	char	tmp_name[15];
	char	last_name[15];
	struct	USR_PTR	*usr_lcl;
	struct	GRP_PTR	*grp_lcl;

	tab_open("user", user_keys, 2, 1, 8, FALSE);
	tab_add("user","#     User Name                 ");

	tab_add("user","    ALL LOGGED IN            ");
	tab_add("user","    ALL USERS                ");

	/*------------------------------
	| Add user names to srch list. |
	------------------------------*/
	no_in_tab = 2;
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
	    sprintf(err_str, "    %-14.14s           ", usr_lcl->usr_name);

	    clip(local_rec.to);

	    if (strcmp(usr_lcl->usr_name,local_rec.to))
	    {
		tab_add("user", err_str);
		no_in_tab++;
	    }

	    usr_lcl = usr_lcl->next;
	}

	/*--------------------------
	| Add groups to srch list. |
	--------------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
	    sprintf(err_str, "    %-14.14s   (GROUP) ", grp_lcl->grp_name);

	    clip(local_rec.to);

	    if (strcmp(grp_lcl->grp_name, local_rec.to))
	    {
		tab_add("user", err_str);
		no_in_tab++;
	    }

	    grp_lcl = grp_lcl->next;
	}

	/*--------------------------------
	| Enable/Disable Expand Hot-Key. |
	--------------------------------*/
	if (grp_head == GRP_NULL)
		set_keys(user_keys, "G", KEY_PASSIVE);
	else
		set_keys(user_keys, "G", KEY_ACTIVE);

	no_tagged = 0;
	ALL_OPT = FALSE;

	tab_scan("user");

	if (!ALL_OPT)
	{
	    strcpy(tagged_name, "");
	    for (i = 0; i < no_in_tab; i++)
	    {
		tab_get("user", user_buf, EQUAL, i);
		if (user_buf[1] == '*')
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
	
	}

	tab_close("user", TRUE);

	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Allow user to tag users to CC mail to |
---------------------------------------*/
static int
user_tag_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	user_buf [100];

	tab_get ("user", user_buf, CURRENT, 0);
	
	switch (user_buf[1])
	{
	case ' ':
		if (no_tagged < 20)
		{
			sprintf(err_str, " *  %-27.27s", &user_buf[4]);
			tab_update("user", err_str);

			if (tab_tline("user") == 0)
			{
				strcpy(temp_str, "ALL LOGGED IN");
				ALL_OPT = TRUE;
				return(FN16);
			}

			if (tab_tline("user") == 1)
			{
				strcpy(temp_str, "ALL USERS");
				ALL_OPT = TRUE;
				return(FN16);
			}

			no_tagged++;
		}
		break;

	case '*':
		sprintf(err_str, "    %-27.27s", &user_buf[4]);
		tab_update("user", err_str);
		break;
	
	default:
		break;
	}

	return (c);
}

/*---------------------------------------
| Allow user to tag users to CC mail to |
---------------------------------------*/
static int
show_group (
 int		c,
 KEY_TAB *	psUnused)
{
	int	grp_fnd;
	char	user_buf [100];
	char	tmp_group [15];
	struct	GRP_PTR	*	grp_lcl;
	struct	USR_PTR	*	usr_lcl;

	tab_get ("user", user_buf, CURRENT, 0);
	sprintf (tmp_group, "%-14.14s", user_buf + 4);
	clip (tmp_group);

	/*-------------------------
	| Can only expand groups. |
	-------------------------*/
	if (!is_group(tmp_group))
	{
		/*print_mess("\007 Not A Group ");*/
		print_mess(ML(mlMenuMess124));
		sleep (sleepTime);
		clear_mess ();
		return (c);
	}

	/*------------------------------
	| Show all users in the group. |
	------------------------------*/
	grp_fnd = FALSE;
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, tmp_group))
		{	
			grp_fnd = TRUE;
			break;
		}

		grp_lcl = grp_lcl->next;
	}

	/*---------------------------------------
	| Fatal error as group exists in table. |
	---------------------------------------*/
	if (!grp_fnd)
		sys_err("Error in show_group().", -1, PNAME);

	/*------------------------------
	| Add group name to the table. |
	------------------------------*/
	tab_open("group", null_keys, 4, 2, 6, FALSE);
	tab_add("group","# GROUP : %-14.14s      ", tmp_group);

	/*------------------------------------------
	| Add all users in the group to the table. |
	------------------------------------------*/
	usr_lcl = grp_lcl->usr_head;
	while (usr_lcl != USR_NULL)
	{
		tab_add("group","    %-14.14s     ", usr_lcl->usr_name);

		usr_lcl = usr_lcl->next;
	}

	tab_scan("group");

	tab_close("group", TRUE);

	/*------------------------
	| Redraw 'parent' table. |
	------------------------*/
	tab_display ("user", TRUE);
	redraw_keys ("user");

	return (c);
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
	char *	opt_ptr = (char *) 0;
	char	tmp_log_name [15];

	sprintf(tmp_log_name, "%-14.14s", log_name);
	clip(tmp_log_name);

	/*--------------------------------------
	| All options are FALSE to begin with. |
	--------------------------------------*/
	valid_user = FALSE;
	user_perm.can_reply   = FALSE;
	user_perm.can_forward = FALSE;
	user_perm.can_send    = FALSE;
	user_perm.utils_ok    = FALSE;
	user_perm.sub_menu_ok = FALSE;

	/*------------------------------------
	| Read Mail_secure file and store it | 
	------------------------------------*/
	if ((fin = fopen(usr_fname, "r")) == 0)
		sys_err("Error in Mail_secure during (FOPEN)",errno,PNAME);

	/*----------------------------
	| Store users in linked list |
	----------------------------*/
	usr_head = USR_NULL;
	usr_curr = usr_head;

	sptr = fgets(err_str, 80, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		/*---------------
		| Comment Line. |
		---------------*/
		if (*sptr == '#')
		{
			sptr = fgets(err_str,80,fin);
			continue;
		}

		tptr = strchr (sptr, '\t');
		if (tptr)
		{
			*tptr = 0;
			opt_ptr = tptr + 1;
		}
		clip (sptr);

		/*--------------------------------------
		| Allocate memory and set next to null |
		--------------------------------------*/
		usr_lcl = usr_alloc ();
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
		clip(usr_curr->usr_name);

		/*----------------------------
		| Current logged in user, so |
		| store user options.        |
		----------------------------*/
		if (!strcmp(usr_curr->usr_name, tmp_log_name))
		{
			/*-------------------------
			| Set users mail options. |
			-------------------------*/
			set_options (opt_ptr, usr_curr);
			valid_user = TRUE;
		}

		sptr = fgets(err_str, 80, fin);
	}
	fclose (fin);

	return (EXIT_SUCCESS);
}

/*-------------------------
| Set users mail options. |
-------------------------*/
int
set_options (
 char *				opt_ptr,
 struct USR_PTR *	lcl_ptr)
{
	char *	sptr;
	char *	tptr;
	char *	xptr;

	/*----------------------
	| Check Option String. |
	----------------------*/
	sptr = opt_ptr;
	while (sptr)
	{
		tptr = sptr;
		while (*tptr && *tptr != '|')
			tptr++;

		xptr = '\0';
		if (*tptr)
		{
			*tptr = '\0';
			xptr = tptr + 1;
		}

		/*-------------------------
		| Check for valid option. |
		-------------------------*/
		if (!strcmp(sptr, "ALL"))
		{
			user_perm.can_reply   = TRUE;
			user_perm.can_forward = TRUE;
			user_perm.can_send    = TRUE;
			user_perm.utils_ok    = TRUE;
			user_perm.sub_menu_ok = TRUE;
			return(0);
		}

		if (!strcmp(sptr, "REPLY"))
			user_perm.can_reply = TRUE;

		if (!strcmp(sptr, "FORWARD"))
			user_perm.can_forward = TRUE;

		if (!strcmp(sptr, "SEND"))
			user_perm.can_send = TRUE;

		if (!strcmp(sptr, "UTILS"))
			user_perm.utils_ok = TRUE;

		if (!strcmp(sptr, "SUB_MENU"))
			user_perm.sub_menu_ok = TRUE;

		sptr = xptr;
	}
	
	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct USR_PTR *
usr_alloc (
 void)
{
	struct	USR_PTR	*lcl_ptr;

	lcl_ptr = (struct USR_PTR *) malloc (sizeof (struct USR_PTR));

	if (lcl_ptr == USR_NULL)
	{
        	sys_err("Error allocating memory for usr list During (MALLOC)",
			errno,
			PNAME);
	}
		
	sprintf(lcl_ptr->usr_name, "%-14.14s", " ");
	lcl_ptr->next = USR_NULL;

	return (lcl_ptr);
}

/*-------------------------------
| Read groups from Mail_groups	|
| and store in a linked list	|
-------------------------------*/
int
load_groups (
 void)
{
	char	*sptr;
	char	*tptr;

	/*-----------------------------------
	| Read Mail_group file and store it | 
	-----------------------------------*/
	if ((fin = fopen(grp_fname, "r")) == 0)
		sys_err("Error in Mail_group during (FOPEN)", errno, PNAME);

	/*-----------------------------
	| Store groups in linked list |
	-----------------------------*/
	grp_head = GRP_NULL;
	grp_curr = grp_head;

	sptr = fgets(err_str, 100, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		/*---------------
		| Comment Line. |
		---------------*/
		if (*sptr == '#')
		{
			sptr = fgets(err_str,100,fin);
			continue;
		}

		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		/*-------------------------------
		| No users in group definition. |
		-------------------------------*/
		if (*tptr == '\0')
		{
			sptr = fgets(err_str, 100, fin);
			continue;
		}

		/*----------------
		| Process group. |
		----------------*/
		*tptr = '\0';
		proc_group (sptr, tptr + 1);

		sptr = fgets(err_str, 100, fin);
	}

	fclose(fin);

	return (EXIT_SUCCESS);
}

/*------------------------
| Process current group. |
------------------------*/
int
proc_group (
 char *	grp_name,
 char *	grp_list)
{
	int	user_found;
	char	*sptr;
	char	*tptr;
	char	*xptr;
	char	user_name [15];
	struct	GRP_PTR	*grp_lcl;
	struct	USR_PTR	*usr_lcl;

	clip (grp_name);

	/*--------------------------------
	| Make sure name of group is not |
	| the name of an individual user |
	--------------------------------*/
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		if (!strcmp(grp_name, usr_lcl->usr_name))
			return(0);

		usr_lcl = usr_lcl->next;
	}

	/*------------------------
	| Get each user or group |
	| name from the list.    |
	------------------------*/
	sptr = grp_list;
	while (sptr)
	{
	    /*---------------------------
	    | Search for next separator |
	    ---------------------------*/
	    tptr = sptr;
	    while (*tptr && *tptr != '|')
		tptr++;

	    xptr = '\0';
	    if (*tptr)
	    {
		*tptr = '\0';
		xptr = tptr + 1;
	    }

	    /*-------------------------
	    | Check for user or group |
	    -------------------------*/	
	    sprintf(user_name, "%-14.14s", sptr);
	    clip(user_name);
	    user_found = FALSE;
	    usr_lcl = usr_head;
	    while (usr_lcl != USR_NULL)
	    {
		if (!strcmp(usr_lcl->usr_name, user_name))
		{
		    user_found = TRUE;
		    break;
		}

		usr_lcl = usr_lcl->next;
	    }

	    /*----------------------------------
	    | If user is a valid user or group |
	    | then add into current group.     |
	    ----------------------------------*/
	    if (user_found)
		add_to_grp (grp_name, user_name);
	    else
	    {
		/*--------------------------------------------
		| See if user name is actually a group name. |
		--------------------------------------------*/
		grp_lcl = grp_head;
		while (grp_lcl != GRP_NULL)
		{
		    /*--------------------------------------------
		    | Found user_name which is actually a group. |
		    --------------------------------------------*/
		    if (!strcmp(grp_lcl->grp_name, user_name))
		    {
			/*--------------------------------------------
			| Add all members of group to current group. |
			--------------------------------------------*/
			usr_lcl = grp_lcl->usr_head;
			while (usr_lcl != USR_NULL)
			{
			    add_to_grp (grp_name, usr_lcl->usr_name);
			    usr_lcl = usr_lcl->next;
			}
			break;
		    }

		    grp_lcl = grp_lcl->next;
		}
	    }

	    sptr = xptr;
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------------
| Add user to group in alphabetical order. |
------------------------------------------*/
int
add_to_grp (
 char *	group,
 char *	user_name)
{
	int		fnd_group;
	int		dup_name;
	int		pos_fnd;
	struct	GRP_PTR *grp_lcl;
	struct	USR_PTR *usr_lcl;
	struct	USR_PTR *usr_tmp;
	struct	USR_PTR *usr_prv;

	/*-----------------------------
	| Can't add a group to itself |
	-----------------------------*/ 
	if (!strcmp(group, user_name))
		return (EXIT_SUCCESS);

	/*-------------------------------
	| Find group node if it exists. |
	-------------------------------*/
	fnd_group = FALSE;
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, group))
		{
			fnd_group = TRUE;
			break;
		}

		grp_lcl = grp_lcl->next;
	}

	/*--------------------------
	| Create a new group node. |
	--------------------------*/
	if (!fnd_group)
	{
		grp_lcl = grp_alloc ();
		sprintf(grp_lcl->grp_name, "%-14.14s", group);
		clip(grp_lcl->grp_name);

		if (grp_head == GRP_NULL)
			grp_head = grp_lcl;
		else
			grp_curr->next = grp_lcl;
		
		grp_curr = grp_lcl;
	}

	/*----------------------------------------------
	| Find place for user in current group's list. |
	----------------------------------------------*/
	if (grp_lcl->usr_head == USR_NULL)
	{
		/*----------------------
		| First entry in list. |
		----------------------*/
		usr_lcl = usr_alloc();
		sprintf(usr_lcl->usr_name, "%-14.14s", user_name);
		clip(usr_lcl->usr_name);

		grp_lcl->usr_head = usr_lcl;
	}
	else
	{
		/*------------------------------
		| First alphabetical position. |
		------------------------------*/
		pos_fnd = FALSE;
		dup_name = FALSE;
		usr_prv = grp_lcl->usr_head;
		usr_tmp = grp_lcl->usr_head;
		while (usr_tmp != USR_NULL)
		{
			/*------------------------------
			| User is already in the list. |
			------------------------------*/
			if (!strcmp(usr_tmp->usr_name, user_name))
			{
				dup_name = TRUE;
				break;
			}

			if (strcmp(user_name, usr_tmp->usr_name) < 0)
			{
				usr_lcl = usr_alloc();
				sprintf(usr_lcl->usr_name,"%-14.14s",user_name);
				clip(usr_lcl->usr_name);
			
				if (usr_prv == usr_tmp)
				{
					/*--------------
					| Head insert. |
					--------------*/
					usr_lcl->next = grp_lcl->usr_head;
					grp_lcl->usr_head = usr_lcl;
				}
				else
				{
					/*----------------
					| Middle insert. |
					----------------*/
					usr_lcl->next = usr_tmp;
					usr_prv->next = usr_lcl;
				}

				pos_fnd = TRUE;
				break;
			}

			usr_prv = usr_tmp;
			usr_tmp = usr_tmp->next;
		}

		/*---------------------------------------
		| Append to end of user list for group. |
		---------------------------------------*/
		if (!pos_fnd && !dup_name)
		{
			usr_lcl = usr_alloc();
			sprintf(usr_lcl->usr_name, "%-14.14s", user_name);
			clip(usr_lcl->usr_name);
	
			usr_prv->next = usr_lcl;
		}
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct	GRP_PTR *
grp_alloc (
 void)
{
	struct	GRP_PTR	*lcl_ptr;

	lcl_ptr = (struct GRP_PTR *) malloc (sizeof (struct GRP_PTR));

	if (lcl_ptr == GRP_NULL)
	{
        sys_err("Error allocating memory for grp list During (MALLOC)",
				errno,
				PNAME);
	}

	sprintf(lcl_ptr->grp_name, "%-14.14s", " ");
	lcl_ptr->usr_head = USR_NULL;
	lcl_ptr->next = GRP_NULL;
		
	return (lcl_ptr);
}

static int
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
static int
read_slct_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	read_buf[100];
	char	tmp_call[7];


	read_ok = TRUE;
	tab_get("read", read_buf, CURRENT, 0);
	sprintf(tmp_call, "%-6.6s", &read_buf[1]);
	read_call = atoi(tmp_call);

	if (!CHK_SENT && curr_status[0] == 'C')
	{
		sprintf(pmre_rec.re_receiver, "%-14.14s", log_name);
		strcpy(pmre_rec.re_status, "C");
		pmre_rec.re_call_no = read_call;
		cc = find_rec(pmre, &pmre_rec, COMPARISON, "r");
		if (cc)
		{
			/*rv_pr("\007 Mail has been deleted by sender ", 25,23,1);*/
			rv_pr(ML(mlMenuMess122), 25,23,1);
			sleep(2);	
			move (0,23);
			cl_line();
			strcpy(chk_status, curr_status);
			chk_mail();
			read_ok = FALSE;
			return(FN16);
		}
	}

	exit_choose = TRUE;
	return (FN16);
}

/*-------------------------------
| Show users mail sent to and	|
| allow user to tag mail for	|
| deletion			|
-------------------------------*/
static int
show_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	show_buf [100];
	int		loop;
	int		no_to_show = 0;

	tab_get("read", show_buf, CURRENT, 0);
	show_call = atoi(&show_buf[1]);
	
	/*------------------------------
	| Check if any actions on mail |
	------------------------------*/
	if (has_actions(show_call))
		set_keys(show_keys, "A", KEY_PASSIVE);
	else
		set_keys(show_keys, "A", KEY_ACTIVE);

	tab_open("show", show_keys, 4, 46, 10, FALSE);
	tab_add("show", "#  Users Sent To     Delete     ");

	cc = find_hash(pmre2, &pmre2_rec, GTEQ, "r", show_call);
	while (!cc && pmre2_rec.re_call_no == show_call)
	{
		if (pmre2_rec.re_status[0] != 'D')
		{
			if (pmre2_rec.re_seen[0] == 'Y')
				strcpy(err_str, " ");
			else
				strcpy(err_str, "*");

			tab_add("show",
				"    %-1.1s%-14.14s ",
				err_str,
				pmre2_rec.re_receiver);
			no_to_show++;
		}

		cc = find_hash(pmre2, &pmre2_rec, NEXT, "r", show_call);
	}

	if (no_to_show > 0)
		tab_scan("show");
	else
	{
		tab_add("show"," *All Mail Deleted By Users* ");
		tab_display("show", TRUE);
		putchar(BELL);
		fflush(stdout);
		sleep(2);
		clear();
		tab_close("show", TRUE);
		return(FN16);
	}

	/*------------------------
	| Delete any tagged mail |
	------------------------*/
	for (loop = 0; loop < no_to_show; loop++)
	{
		tab_get("show", show_buf, EQUAL, loop);
		if (show_buf[22] == 'D')
		{
			sprintf(pmre_rec.re_receiver, "%-14.14s", &show_buf[5]);
			strcpy(pmre_rec.re_status, "C");
			pmre_rec.re_call_no = show_call;
			cc = find_rec(pmre, &pmre_rec, COMPARISON, "u");
			if (cc)
			{
				/*sprintf(err_str, "%s read the mail before it could be deleted ", clip(pmre_rec.re_receiver));*/
				sprintf(err_str,ML(mlMenuMess204), clip(pmre_rec.re_receiver));
				rv_pr(err_str, 0,23,1);
				sleep(2);	
				move (0,23);
				cl_line();
				continue;
			}
			
			strcpy(pmre_rec.re_status, "D");
			cc = abc_update(pmre, &pmre_rec);
			if (cc)
                		sys_err("Error in pmre During (DBUPDATE)",cc,PNAME);
		}
	}

	tab_close("show", TRUE);
	tab_display("read", TRUE);
	redraw_keys("read");
	
	if (all_deleted(show_call))
		return(FN16);

	return (c);
}

/*-----------------------
| Tag mail for deletion |
-----------------------*/
static int
delete_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	del_buf[100];
	int	lcl_cc;
	int	ok_del;

	tab_get("show", del_buf, CURRENT, 0);
	
	switch(del_buf[22])
	{
	case ' ':
		ok_del = FALSE;

		lcl_cc = find_hash(pmre2, &pmre2_rec, GTEQ, "r", show_call);
		while (!lcl_cc && pmre2_rec.re_call_no ==  show_call)
		{	
			if (!strncmp(pmre2_rec.re_receiver, &del_buf[5], 14))
			{
				ok_del = TRUE;
				if (pmre2_rec.re_seen[0] == 'Y')
				{
					sprintf(err_str, 
						"     %-14.14s       ", 
						&del_buf[5]);
					tab_update("show", err_str);

					/*rv_pr("\007 Mail has been read - Cannot delete ", 17,23,1);*/
					rv_pr(ML(mlMenuMess202), 17,23,1);
					sleep(2);
					move (0,23);
					cl_line();
					ok_del = FALSE;
				}
				break;
			}
			lcl_cc = find_hash(pmre2,&pmre2_rec,NEXT,"r",show_call);
		}

		if (ok_del)
		{
			sprintf(err_str, "    %-15.15s   D   ", &del_buf[4]);
			tab_update("show", err_str);
			break;
		}
		else
			break;

	case 'D':
		sprintf(err_str, "    %-15.15s       ", &del_buf[4]);
		tab_update("show", err_str);
		break;
	
	default:
		break;
	}

	return(c);
}

/*----------------------------------
| Display details for current mail |
----------------------------------*/
static int
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
static int
actions_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (DETAILS)
	{
		if (!has_actions(read_call))
		{
			/*rv_pr("\007 No Actions For This Mail ", 25, 23, 1);*/
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

/*------------------------------------
| Toggle between details and actions |
| from the main ring menu.           |
------------------------------------*/
int
toggle_text (
 void)
{
	if (DETAILS)
	{
		if (has_actions(read_call))
			DETAILS = FALSE;
		else
		{	
			/*print_mess("\007 No Actions For This Mail ");*/
			print_mess (ML (mlMenuMess125));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
	}
	else
		DETAILS = TRUE;

	tab_close ("dsp_txt", TRUE);
	load_text ();
	tab_display ("dsp_txt", TRUE);
	fix_edge (10);

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Print mail to user selected printer |
-------------------------------------*/
int
mail_prnt_func2 (
 void)
{
	return (mail_prnt_func (0, (KEY_TAB *) 0));
}

static int
mail_prnt_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int 	lpno;

	lpno = get_lpno (0);

	heading (2);
	scn_display (2);
	tab_display ("dsp_txt", TRUE);
	
	/*Printing...*/
	rv_pr(ML(mlStdMess035), 35, 23, 1);

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno,PNAME);

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
		local_rec.to);
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
	prnt_text ("D");
	fprintf(fout,"|                                                                              |\n");

	/*------------------------------------
	| Print actions associated with mail |
	------------------------------------*/
	if (has_actions (read_call))
	{
		fprintf(fout,"+------------------------------------------------------------------------------+\n");
		fprintf(fout,".LRP4\n");
		fprintf(fout,"|                                    ACTIONS                                   |\n");
		prnt_text ("A");
		fprintf(fout,"|                                                                              |\n");
		fprintf(fout,"+==============================================================================+\n");
	}
	else
	{
		fprintf(fout,"|                                                                              |\n");
		fprintf(fout,"+==============================================================================+\n");
	}

	fprintf(fout,".EOF\n");
	fflush(fout);
	pclose(fout);

	move(0,23);
	cl_line();

	if (show_hot_keys)
		redraw_keys("dsp_txt");

	return (c);
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

		fprintf(fout, "|         %-60.60s         |\n", pmtx_rec.tx_text);
		first_time = FALSE;			/* CAM CHECK!!!!! */

		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}
}

/*---------------
| Run Sub Menu. |
---------------*/
int
run_sub2 (
 void)
{
	return (run_sub (0, (KEY_TAB *) 0));
}

static int
run_sub (
 int		c,
 KEY_TAB *	psUnused)
{
	int	status;

	*(arg) = "run_extern";
	*(arg + (1)) = "psl_mail.c";
	*(arg + (2)) = (char *) 0;

	if ((fork ()) == 0)
	{
		status = execvp (arg[0], arg);
		return (EXIT_SUCCESS);
	}
	else
		wait (&status);

	set_tty ();

	/*---------------
	| Redraw screen |
	---------------*/
	heading(2);
	scn_display(2);
	tab_display("dsp_txt", TRUE);
	fix_edge (10);
	if (show_hot_keys)
		redraw_keys("dsp_txt");

	return (c);
}

/*----------------------------------
| Create records that 'posts' mail |
| if FORWARD then copy all text    |
| across to new call number        |
----------------------------------*/
int
post_mail (
 int	disp)
{
	struct	USR_PTR	*snd_lcl;
	int	primary;
	long	call_no;

	if (disp)
	{
		clear ();
		/*rv_pr("Sending Mail...", 5, 0, 0);*/
		rv_pr (ML (mlMenuMess203), 5, 0, 0);
		sleep (sleepTime);
	}
	
	/*----------------------
	| Create sender record |
	----------------------*/
	pmse_rec.se_termno = comm_rec.termno;
	pmse_rec.se_nx_seq = 1;
	sprintf(pmse_rec.se_sender,  "%-14.14s", local_rec.from);
	sprintf(pmse_rec.se_to,      "%-14.14s", local_rec.to);
	sprintf(pmse_rec.se_subject, "%-40.40s", local_rec.subject);
	pmse_rec.se_priority = local_rec.priority;
	sprintf(pmse_rec.se_link_type, "%-1.1s", local_rec.mail_src);
	pmse_rec.se_link_hash = local_rec.link_hash;
	sprintf(pmse_rec.se_phone_no, "%-15.15s", local_rec.phone_no);
	sprintf(pmse_rec.se_contact_name, "%-25.25s", local_rec.contact_name);

	pmse_rec.se_date = TodaysDate ();
	strcpy (pmse_rec.se_time, TimeHHMM());

	strcpy(pmse_rec.se_status, "C");
	strcpy(pmse_rec.se_stat_flag, "O");
	cc = abc_add(pmse2, &pmse_rec);
	if (cc)
                sys_err("Error in pmse During (DBADD)",cc,PNAME);
	
	cc = find_rec(pmse2, &pmse_rec, COMPARISON, "r");
	if (cc)
                sys_err("Error in pmse During (DBFIND)",cc,PNAME);
	
	call_no = pmse_rec.se_call_no;

	/*----------------------
	| Create text for mail |
	----------------------*/
	add_text ("D", call_no);

	/*-----------------------------------------------
	| Create list of users who are to receive mail. |
	-----------------------------------------------*/
	create_send_list ();

	/*-------------------------
	| Create receiver records |
	-------------------------*/
	primary = TRUE;
	snd_lcl = snd_head;
	while (snd_lcl != USR_NULL)
	{
		pmre_rec.re_call_no = call_no;
		sprintf(pmre_rec.re_receiver, "%-14.14s", snd_lcl->usr_name);
		strcpy(pmre_rec.re_status, "C");
		strcpy(pmre_rec.re_seen, "N");

		if (primary && !is_group(clip(local_rec.to)))
			strcpy(pmre_rec.re_cc_flag, "N");
		else
			strcpy(pmre_rec.re_cc_flag, "Y");
		primary = FALSE;

		if (local_rec.dly_mail[0] == 'Y')
		{
			pmre_rec.re_act_date = local_rec.dly_date;
			sprintf (pmre_rec.re_act_time, "%-5.5s", local_rec.dly_time);
		}
		else
		{
			pmre_rec.re_act_date = pmse_rec.se_date;
			sprintf (pmre_rec.re_act_time, "%-5.5s", pmse_rec.se_time);
		}

		pmre_rec.re_fst_date = 0L;	
		strcpy(pmre_rec.re_fst_time, "00:00");
		pmre_rec.re_lst_date = 0L;	
		strcpy(pmre_rec.re_lst_time, "00:00");
		cc = abc_add(pmre, &pmre_rec);
		if (cc)
		{
			if (cc != 100)
       	         	       sys_err("Error in pmre During (DBADD)",cc,PNAME);
		}
		else
			mail_notify (snd_lcl->usr_name);
	
		snd_lcl = snd_lcl->next;
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Creates pmtx records from the	|
| current tabular screen	|
| 'A' = Actions , 'D' = Details |
-------------------------------*/
void
add_text (
 char *	status,
 long	call_no)
{
	int		line_cnt;
	
	pmtx_rec.tx_call_no = call_no;
	if (status[0] == 'D')
	{
		pmtx_rec.tx_seq_no = 0;
		pmtx_rec.tx_date = 0L;
		strcpy(pmtx_rec.tx_time, "00:00:00");
	}
	else
	{
		pmtx_rec.tx_date = TodaysDate ();
		strcpy (pmtx_rec.tx_time, TimeHHMM());

		cc = find_hash(pmse, &pmse_rec, COMPARISON, "u", call_no);
		if (cc)
       	       		sys_err("Error in pmse During (DBFIND)",cc,PNAME);

		pmtx_rec.tx_seq_no = pmse_rec.se_nx_seq++;
		
		cc = abc_update(pmse, &pmse_rec);
		if (cc)
			sys_err("Error in pmse During (DBUPDATE)",cc,PNAME);
	}

	for (line_cnt = 0;line_cnt < lcount[5];line_cnt++)
	{
		sprintf(pmtx_rec.tx_sender, "%-14.14s", log_name);
		pmtx_rec.tx_line_no = line_cnt;
		strcpy(pmtx_rec.tx_mail_type, status);
		getval( line_cnt );
		sprintf(pmtx_rec.tx_text, "%-60.60s", local_rec.text);
		cc = abc_add(pmtx, &pmtx_rec);
		if (cc)
			sys_err("Error in pmtx During (DBADD)",cc,PNAME);
	}
}

/*--------------------------------------
| Create list of users to send mail to |
--------------------------------------*/
int
create_send_list (
 void)
{
	int	lots_of_spaces;
	int	first_time = TRUE;
	char	*sptr;
	char	*tptr;
	char	tmp_name[15];
	char	char_separator;
	struct	USR_PTR	*usr_lcl;
	struct	USR_PTR	*log_lcl;
	struct	USR_PTR	*snd_lcl;

	/*--------------------------
	| Clear current send list. |
	--------------------------*/
	if (snd_head != USR_NULL)
		clr_usr_list (snd_head);

	snd_head = USR_NULL;
	snd_curr = snd_head;

	/*------------------------------
	| Send to all logged in users. |
	------------------------------*/
	if (ALL_LOG)
	{
		/*-------------------------------------------
		| Create linked list of all logged in users |
		-------------------------------------------*/
		get_logged_in (); 

		if (strncmp(local_rec.to, "ALL LOGGED IN", 13) && first_time)
		{
			snd_lcl = usr_alloc();
			snd_lcl->next = USR_NULL;
			sprintf(snd_lcl->usr_name,"%-14.14s",local_rec.to);
			clip(snd_lcl->usr_name);
			snd_head = snd_lcl;
			first_time = FALSE;
		}
		snd_curr = snd_head;

		/*---------------------------------------
		| create send list from logged in users |
		---------------------------------------*/
		usr_lcl = usr_head;
		while (usr_lcl != USR_NULL)
		{
			log_lcl = log_head;
			while (log_lcl != USR_NULL)
			{
				clip(local_rec.to);
				clip(usr_lcl->usr_name);
				clip(log_lcl->usr_name);
				if (!strcmp(usr_lcl->usr_name,log_lcl->usr_name)
				    && strcmp(local_rec.to, usr_lcl->usr_name))
				{
					/*--------------------------------------
					| Allocate memory and set next to null |
					--------------------------------------*/
					snd_lcl = usr_alloc();
					snd_lcl->next = USR_NULL;

					/*---------------------------
					| Set head to start of list |
					---------------------------*/
					if (snd_head == USR_NULL)
						snd_head = snd_lcl;
					else
						snd_curr->next = snd_lcl;

					/*------------------------------
					| store user name in send list |
					------------------------------*/
					snd_curr = snd_lcl;
					sprintf(snd_curr->usr_name, 
						"%-14.14s", 
						usr_lcl->usr_name);
					clip(snd_curr->usr_name);
					break;
				}
				/*-------------------------
				| Get next logged in user |
				-------------------------*/
				log_lcl = log_lcl->next;
			}
			/*---------------
			| Get next user |
			---------------*/
			usr_lcl = usr_lcl->next;
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Send to all users. |
	--------------------*/
	if (ALL_USR)
	{
		snd_head = usr_head;
		return (EXIT_SUCCESS);
	}

	sprintf (tmp_name, "%-14.14s", local_rec.to);
	clip (tmp_name);
	
	/*---------------------------------------
	| Ignore 'to' user for forwarding mail. |
	---------------------------------------*/
	if (FORWARD)
		strcpy (tmp_name, "");
	
	/*--------------------------
	| Create head of send list |
	| as the 'to' receiver.    |
	--------------------------*/
	if (is_group (tmp_name))
	{
		/*-------------------------
		| Add group to send list. |
		-------------------------*/
		snd_curr = expand_group(tmp_name, snd_curr);
	}
	else
	{
		snd_lcl = usr_alloc();
		snd_lcl->next = USR_NULL;
		snd_head = snd_lcl;
		snd_curr = snd_head;
		if (FORWARD)
			sprintf(snd_lcl->usr_name, "%-14.14s", " ");
		else
			sprintf(snd_lcl->usr_name, "%-14.14s", local_rec.to);
		clip(snd_lcl->usr_name);
	}

	clip(local_rec.CC);
	lclip(local_rec.CC);
	sptr = local_rec.CC;
	tptr = sptr;

	if (strchr(sptr, ','))
		char_separator = ',';
	else
		char_separator = ' ';

	/*---------------------
	| Create list from CC |
	---------------------*/
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

			if (is_group(tmp_name))
			{
				/*-------------------------
				| Add group to send list. |
				-------------------------*/
				snd_curr = expand_group(tmp_name, snd_curr);
			}
			else
			{
				/*------------------------
				| Add user to send list. |
				------------------------*/
				snd_curr = add_to_send_list(tmp_name, snd_curr);
			}

			if (*tptr)
				sptr = ++tptr;
			else
				sptr = tptr;
		}
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Create send list from logged in users |
---------------------------------------*/
int
get_logged_in (
 void)
{
	struct	USR_PTR	*log_lcl;

	if (log_head != USR_NULL)
		clr_usr_list (log_head);

	log_head = USR_NULL;
	log_curr = log_head;

	uptr = getutent();
	while (uptr != ((struct	utmp *) NULL) )
	{
		if (uptr->ut_type == USER_PROCESS)
		{
			/*--------------------------------------
			| Allocate memory and set next to null |
			--------------------------------------*/
			log_lcl = usr_alloc();
			log_lcl->next = USR_NULL;

			/*---------------------------
			| Set head to start of list |
			---------------------------*/
			if (log_head == USR_NULL)
				log_head = log_lcl;
			else
				log_curr->next = log_lcl;

			/*----------------------------
			| store user name in current |
			----------------------------*/
			log_curr = log_lcl;
			sprintf (log_curr->usr_name, "%-14.14s", uptr->ut_user);
			clip (log_curr->usr_name);
		}
	
		uptr = getutent();
	}
	endutent ();

	return (EXIT_SUCCESS);
}

/*---------------------------
| Check if name is a group. |
---------------------------*/
int
is_group (
 char *	group)
{
	struct	GRP_PTR	*lcl_ptr;

	lcl_ptr = grp_head;
	while (lcl_ptr != GRP_NULL)
	{
		if (!strcmp(lcl_ptr->grp_name, group))
			return (TRUE);

		lcl_ptr = lcl_ptr->next;
	}

	return (FALSE);
}

/*-------------------------
| Add group to send list. |
-------------------------*/
struct	USR_PTR *
expand_group (
 char *				group_name,
 struct USR_PTR *	curr_ptr)
{
 	struct	USR_PTR	*usr_lcl;
 	struct	USR_PTR	*usr_tmp;
 	struct	GRP_PTR	*grp_lcl;

	/*------------------
	| Find group node. |
	------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, group_name))
			break;

		grp_lcl = grp_lcl->next;
	}
	if (strcmp(grp_lcl->grp_name, group_name))
		return(curr_ptr);

	/*----------------------------------------
	| Set to start of list of users in group |
	----------------------------------------*/
	usr_lcl = grp_lcl->usr_head;

	/*------------------------
	| Add head of send list. |
	------------------------*/
	if (curr_ptr == USR_NULL)
	{
		usr_tmp = usr_alloc();

		snd_head = usr_tmp;
		curr_ptr = snd_head;
		sprintf(usr_tmp->usr_name, "%-14.14s", usr_lcl->usr_name);
		clip(usr_tmp->usr_name);

		usr_lcl = usr_lcl->next;
	}

	/*------------------------
	| Add all users in group |
	------------------------*/
	while (usr_lcl != USR_NULL)
	{
		curr_ptr = add_to_send_list(usr_lcl->usr_name, curr_ptr);

		usr_lcl = usr_lcl->next;
	}

	return(curr_ptr);
}

/*------------------------
| Add user to send list. |
------------------------*/
struct	USR_PTR *
add_to_send_list (
 char *				user_name,
 struct USR_PTR *	curr_ptr)
{
	struct	USR_PTR	*lcl_ptr;

	/*------------------------
	| Check that user is not |
	| already in the list.   |
	------------------------*/
	lcl_ptr = snd_head;
	while (lcl_ptr != USR_NULL)
	{
		if (!strcmp(lcl_ptr->usr_name, user_name))
			return(curr_ptr);

		lcl_ptr = lcl_ptr->next;
	}

	/*--------------------------------------
	| Allocate memory and set next to null |
	--------------------------------------*/
	lcl_ptr = usr_alloc();

	/*------------------------------
	| store user name in send list |
	------------------------------*/
	curr_ptr->next = lcl_ptr;
	curr_ptr = lcl_ptr;
	sprintf(curr_ptr->usr_name, "%-14.14s", user_name);
	clip(curr_ptr->usr_name);

	return (curr_ptr);
}

/*--------------------------------
| Check if there are any actions |
--------------------------------*/
int
has_actions (
 long	call_no)
{
	pmtx_rec.tx_call_no = call_no;
	pmtx_rec.tx_seq_no = 1;
	strcpy(pmtx_rec.tx_mail_type, "A");
	pmtx_rec.tx_line_no = 0;

	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	if (cc || pmtx_rec.tx_call_no != read_call || 
	    strcmp(pmtx_rec.tx_mail_type, "A"))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
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

/*-------------------------
| Display Screen Heading  |
-------------------------*/
int
heading (
 int scn)
{
	char	tmp_str [14];
	char	blnk_line [79];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (scn == 1)	
		{
			if ( clear_ok )
				clear();

			clear_ok = TRUE;

			box(0,1,80,20); 

			sprintf(err_str,
				ML(mlMenuMess200), 
				(REPLY) ? "Reply To" : "Send");

			rv_pr(err_str,(80 - strlen(err_str)) / 2,1,1);
		
			move(1,6);
			line(79);
			fix_edge (6);

			move(1,11);
			line(79);
			fix_edge (11);

			sprintf(local_rec.from, "%-14.14s", log_name);
		}
		if (scn == 2)
		{
			clear();

			box(0,1,80,19); 

			if (CHK_SENT)
				strcpy(tmp_str, "Mail Sent");
			else
			{
				if (curr_status[0] == 'C')
					strcpy(tmp_str, "Incoming Mail");
				else
					strcpy(tmp_str, "Archived Mail");
			}

			sprintf(err_str,
			     ML(mlMenuMess200), 
			     tmp_str);

			rv_pr(err_str,(80 - strlen(err_str)) / 2,1,1);
		
			move(1,6);
			line(79);
			fix_edge (6);
		}

		if (scn == 3)
		{
			move(10, 1);
			line(60);
			rv_pr(ML(mlMenuMess129),(80 - strlen(ML(mlMenuMess129))) / 2,1,1);
		}

		if (scn == 4)
		{
			box (0,18,80,2);
			sprintf(blnk_line, "%-78.78s", " ");
			rv_pr(blnk_line, 1,19,0);
			rv_pr(blnk_line, 1,20,0);
		}
	
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
		
		if (scn == 1)
		{
			DSP_FLD("from");
		}
	}
	return (EXIT_SUCCESS);
}

/*---------------------------
| Clear users mail message. |
---------------------------*/
int
clear_mail_msg (
 void)
{
	char *	sptr;

	sptr = getenv ("LOGNAME");
	if (sptr)
	{
		sprintf (cmd, "message -u%s -c", sptr);
		sys_exec (cmd);
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------
| Notify user of unread mail. |
-----------------------------*/
int
mail_notify (
 char *	who)
{
	char	curr_time[6];

	strcpy (curr_time, TimeHHMM());

	if (local_rec.dly_mail[0] == 'Y' && NOT_ACTIVE)
		return(0);

	sprintf (cmd, "psl_mctrl 5 %s", who);
	sys_exec( cmd );
	return (EXIT_SUCCESS);
}

/*-----------------
| Dummy function. |
-----------------*/
int
dummy_func2 (
 void)
{
	return (EXIT_SUCCESS);
}

static int
dummy_func (
 int		c,
 KEY_TAB *	psUnused)
{
	return (EXIT_SUCCESS);
}


#ifdef DEBUG
chk_list()
{
	print_at(0,0,"STOP#0\n");getchar();

	grp_curr = grp_head;
	while (grp_curr != GRP_NULL)
	{
		print_at(0,0,"STOP#1 group[%s]\n", grp_curr->grp_name);getchar();

		usr_curr = grp_curr->usr_head;
		while (usr_curr != USR_NULL)
		{
			print_at(0,0,"    STOP#2 user[%s]\n", usr_curr->usr_name);getchar();

			usr_curr = usr_curr->next;
		}
	
		grp_curr = grp_curr->next;
	}

	print_at(0,0,"STOP#3 ");getchar();
}
#endif

int
dump_mail2 (
 void)
{
	return (dump_mail (0, (KEY_TAB *) 0));
}

static	int
dump_mail (
 int		c,
 KEY_TAB *	psUnused)
{
	FILE *	fout;
	int		first_time = TRUE;
	int		last_seq = 0;
	char *	sptr = getenv ("HOME");

	sprintf (err_str, "%s/MAIL/mail%ld", sptr,pmse_rec.se_call_no);
	if ((fout = fopen(err_str, "w+")) == (FILE *)NULL)
		return(FALSE);

	fprintf (fout, "Call Number  : %0ld\n", pmse_rec.se_call_no);
	fprintf (fout, "Sender       : %s\n",  pmse_rec.se_sender);
	fprintf (fout, "Date Sent    : %s\n",  DateToString(pmse_rec.se_date));
	fprintf (fout, "-----------------------------------------------------------------\n");
	fprintf (fout, "DETAILS\n");
	fprintf (fout, "=======\n");

	pmtx_rec.tx_call_no = pmse_rec.se_call_no;
	pmtx_rec.tx_seq_no = 0;
	strcpy(pmtx_rec.tx_mail_type, "D");
	pmtx_rec.tx_line_no = 0;

	cc = find_rec(pmtx, &pmtx_rec, GTEQ, "r");
	while (!cc && 
		pmtx_rec.tx_mail_type[0] == 'D' && 
		pmtx_rec.tx_call_no == pmse_rec.se_call_no  
	      )
	{
		fprintf (fout, "%60.60s\n", pmtx_rec.tx_text);
		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
	}

	while (!cc && 
		pmtx_rec.tx_mail_type[0] == 'A' && 
		pmtx_rec.tx_call_no == pmse_rec.se_call_no  
	      )
	{
		if (first_time)
		{
			fprintf (fout, "\nACTIONS\n");
			fprintf (fout, "=======\n");
		}

		if ( last_seq != pmtx_rec.tx_seq_no )
		{

			last_seq = pmtx_rec.tx_seq_no;
			if (!first_time)
				fprintf (fout, "\n");
			fprintf (fout, "%-14.14s (%s %s)\n", 
						pmtx_rec.tx_sender, 
					  	DateToString(pmtx_rec.tx_date),
						pmtx_rec.tx_time);
		}
		fprintf (fout, "%60.60s\n", pmtx_rec.tx_text);
		cc = find_rec(pmtx, &pmtx_rec, NEXT, "r");
		first_time = FALSE;
	}
	fclose (fout);
	return (EXIT_SUCCESS);
}

/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ml_sumrpt.c  	)                             |
|  Program Desc  : ( Mail Summary Statistics Report.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmhs, pmhl,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmhs, pmhl,     ,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 21/11/91         |
|---------------------------------------------------------------------|
|  Date Modified : (10/04/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (05/09/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (30/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      : (10/04/92) - Fix core dump problem.                |
|                : (05/09/97) - Modified for Multilingual Conversion. |
|                : (30/08/1999) - Ported to ANSI format.              |
|                :                                                    |
|                                                                     |
| $Log: ml_sumrpt.c,v $
| Revision 5.2  2001/08/09 05:13:32  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:09  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:18  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:16  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 09:41:56  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/10/20 02:06:48  nz
| Updated for final changes on date routines.
|
| Revision 1.7  1999/10/12 05:18:14  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.6  1999/09/29 10:11:07  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:27:00  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:36:51  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: ml_sumrpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ml_sumrpt/ml_sumrpt.c,v 5.2 2001/08/09 05:13:32 scott Exp $";

#define		X_OFF		0
#define		Y_OFF		2

#define		UNREAD		-1

#define	AM 	( atoi(pmhs_rec.hs_time) >= 0 && atoi(pmhs_rec.hs_time) <= 11 )
#define	PRI 	( (pmhs_rec.hs_priority >= 1 && pmhs_rec.hs_priority <= 5) ? \
		  (pmhs_rec.hs_priority - 1) : 4 )
#define	TOTALS 	5 
#define	USER_S	0
#define	ALL_S	1
#define	MAX_DAYS	40	
#define	MAX_HOURS	59999L	/* 999 hrs 59 mins */

#define	ALL_USERS	( !strcmp(local_rec.usr_name, "ALL USERS     ") )

#include 	<pslscr.h>
#include 	<sys/types.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>

FILE	*fin;
FILE	*fout;

char	*TOP_BOX = "       ^^AGGGGGGGGGGIIGGGGGGGGGGGGGGGGGGGGIIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^ ";
char	*HEADER1 = "       ^E          ^E^E     MAIL SENT      ^E^E                             MAIL RECEIVED                                   ^E ";
char	*HEADER2 = "       ^E   MAIL   ^E^E NUM. ^E NUM. ^ETOTAL ^E^E      ^E  AVG TIME ^E  NOT ^E      ^E  AVG TIME ^E  NOT ^E      ^E  AVG TIME ^E  NOT ^E";
char	*HEADER3 = "       ^E PRIORITY ^E^E  AM  ^E  PM  ^E SENT ^E^E  AM  ^E  TO  READ ^E READ ^E  PM  ^E  TO  READ ^E READ ^ETOTAL ^E  TO  READ ^E READ ^E";
char	*MID_BOX = "       ^^KGGGGGGGGGGHHGGGGGGHGGGGGGHGGGGGGHHGGGGGGHGGGGGGGGGGGHGGGGGGHGGGGGGHGGGGGGGGGGGHGGGGGGHGGGGGGHGGGGGGGGGGGHGGGGGGL^^ ";
char	*END_BOX = "       ^^CGGGGGGGGGGJJGGGGGGJGGGGGGJGGGGGGJJGGGGGGJGGGGGGGGGGGJGGGGGGJGGGGGGJGGGGGGGGGGGJGGGGGGJGGGGGGJGGGGGGGGGGGJGGGGGGD^^ ";
char	*disp_mask = "       ^E  %-5.5s   ^E^E %-4.4s ^E %-4.4s ^E %-4.4s ^E^E %-4.4s ^E %-9.9s ^E %-4.4s ^E %-4.4s ^E %-9.9s ^E %-4.4s ^E %-4.4s ^E %-9.9s ^E %-4.4s ^E";

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

	/*============================
	| Pinnacle Mail Control File |
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
	| Pinnacle Mail History Header File |
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
	| Pinnacle Mail History Line Detail File |
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

char	*comm    = "comm",
     	*pmct    = "pmct",
     	*pmhs    = "pmhs",
     	*wk_pmhs = "wk_pmhs",
     	*pmhl    = "pmhl",
     	*wk_pmhl = "wk_pmhl",
     	*pmtx    = "pmtx";

char	usr_fname[100];

struct	USR_PTR	{
	char	usr_name[15];
	struct USR_PTR	*next;
};

#define	USR_NULL ((struct USR_PTR *)NULL)
struct  USR_PTR *usr_head;
struct  USR_PTR *usr_curr;

char	*pri_desc[] = {
	"ONE",
	"TWO",
	"THREE",
	"FOUR",
	"FIVE",
	"TOTAL",
	"",
};

struct	{
	int	am_sent;	/* Number Sent In AM */
	int	pm_sent;	/* Number Sent In PM */
	int	am_rcvd;	/* Number Received In AM */
	long	am_time;	/* Total Time Taken To Read Mail Sent In AM */
	int	am_unread; 	/* Received Mail That Is Unread AM */
	int	pm_rcvd;	/* Number Received In PM */
	long	pm_time;	/* Total Time Taken To Read Mail Sent In PM */
	int	pm_unread; 	/* Received Mail That Is Unread PM */
} mail_stat[6][2];

int	tot_rcvrs[2]; /* tot_rcvrs[1] Is For TOTALS */

char 	am_avg[9];
char 	pm_avg[9];
char 	tot_avg[9];
char	tmp_fld[5];

/*============================ 
| Local & Screen Structures. |
============================*/

struct	{
	char	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	char	usr_name[15];
	long	s_date;
	long	e_date;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "usrname",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", " User Name: ", "Enter logname of user whose mail to maintain. Default for ALL",
		NO, NO,  JUSTLEFT, "", "", local_rec.usr_name},
	{1, LIN, "s_date",	 5, 15, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Start Date: ", "Enter Start Date ",
		NO, NO,  JUSTRIGHT, "", "", (char *)&local_rec.s_date},
	{1, LIN, "e_date",	 6, 15, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " End Date  : ", "Enter End Date ",
		NO, NO,  JUSTRIGHT, "", "", (char *)&local_rec.e_date},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};


/*==========================
| Function protypes.        |
==========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
int		srch_user		(char * key_val);
int		chk_user		(char * key_val);
int		get_all_users	(void);
struct USR_PTR *usr_alloc (void);
int		process			(void);
int		init_array		(int init_all);
int		proc_user		(char * usr_name);
long	calc_time		(void);
int		prnt_totals		(char * usr_name);
void	chk_fld			(int chk_val);
int		calc_avg		(int pri_no, int tot_type);
int		heading			(int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;

	_mail_ok = FALSE;

	/*------------------------------
	| Check That User Is Superuser |
	------------------------------*/
	if (getuid() != 0)
	{
		/*Only Superusers May Run This Program */
		print_mess(ML(mlMenuMess109));
		sleep(2);
		clear_mess();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*------------------------------------
	| Read User_secure file and store it | 
	------------------------------------*/
	sptr = getenv("PROG_PATH");
	sprintf(usr_fname, "%s/BIN/MENUSYS/User_secure", 
				(sptr) ? sptr : "/usr/LS10.5");
	get_all_users();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/

	/*------------------------------
	| Read common terminal record. |
	| Open main database files.    |  
	------------------------------*/
	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit = FALSE;
		edit_exit = FALSE;
		prog_exit = FALSE;
		restart = FALSE;
		search_ok = TRUE;
		init_ok = TRUE;

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
	
		if (restart)
			continue;

		process();
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program shutdown.       |
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
OpenDB (void)
{
	abc_dbopen("data");

	abc_alias(wk_pmhs, pmhs);
	abc_alias(wk_pmhl, pmhl);

	open_rec(pmct,    pmct_list, pmct_no_fields, "pmct_system_name");
	open_rec(pmhs,    pmhs_list, pmhs_no_fields, "pmhs_id_no");
	open_rec(wk_pmhs, pmhs_list, pmhs_no_fields, "pmhs_call_no");
	open_rec(pmhl,    pmhl_list, pmhl_no_fields, "pmhl_id_no");
	open_rec(wk_pmhl, pmhl_list, pmhl_no_fields, "pmhl_id_no2");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(pmct);
	abc_fclose(pmhs);
	abc_fclose(wk_pmhs);
	abc_fclose(wk_pmhl);
	abc_fclose(pmhl);
	abc_dbclose("data");
}

int
spec_valid (
	int field)
{
	char	tmp_date[11];

	if (LCHECK("usrname"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.usr_name, "ALL USERS     ");
			DSP_FLD("usrname");
			return(0);
		}

		if (SRCH_KEY)
		{
			srch_user(temp_str);
			return(0);
		}

		return (chk_user(temp_str));
	}

	if (LCHECK("s_date"))
	{
		if (dflt_used)
		{
			sprintf(tmp_date, "01/%-5.5s", local_rec.systemDate + 3);
			local_rec.s_date = StringToDate(tmp_date);
			return(0);
		}

		if (local_rec.s_date > local_rec.lsystemDate)
		{
			print_mess(ML(mlStdMess068));
			sleep(2);
			clear_mess();
			return(1);
		}
	}

	if (LCHECK("e_date"))
	{
		if (dflt_used)
		{
			local_rec.e_date = MonthEnd(local_rec.lsystemDate);
			return(0);
		}

		if (local_rec.e_date < local_rec.s_date)
		{
			print_mess(ML(mlMenuMess192));
			sleep(2);
			clear_mess();
			return(1);
		}
	}

	return(0);
}

int
srch_user (
 char *	key_val)
{
	struct	USR_PTR	*usr_lcl;

	work_open();
	save_rec("#User Name","# ");

	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		sprintf(err_str, "%-14.14s", usr_lcl->usr_name);
		cc = save_rec(err_str, " ");
		if (cc)
			break;
		usr_lcl = usr_lcl->next;
	}

	cc = disp_srch();
	work_close();

	return(0);
}

/*-------------------------------
| Check if user is a valid user |
| from User_secure		|
-------------------------------*/
int
chk_user (
 char *	key_val)
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
	return (EXIT_FAILURE);
}

/*-------------------------------
| Read users from User_secure	|
| and store in a linked list	|
-------------------------------*/
int
get_all_users (void)
{
	struct	USR_PTR	*usr_lcl;
	char	*sptr;
	char	*tptr;

	/*------------------------------------
	| Read User_secure file and store it | 
	------------------------------------*/
	if ((fin = fopen(usr_fname,"r")) == 0)
		sys_err("Error in User_secure during (FOPEN)",errno,PNAME);

	/*----------------------------
	| Store users in linked list |
	----------------------------*/
	usr_head = USR_NULL;
	usr_curr = usr_head;

	sptr = fgets(err_str,80,fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		tptr = strchr (sptr, '<');
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

	return(0);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct USR_PTR *
usr_alloc (void)
{
	struct	USR_PTR	*lcl_ptr;

	lcl_ptr = (struct USR_PTR *) malloc (sizeof (struct USR_PTR));

	if (lcl_ptr == USR_NULL)
        	sys_err("Error allocating memory for usr list During (MALLOC)",errno,PNAME);
		
	return (lcl_ptr);
}

/*----------------------
| Process Mail History |
----------------------*/
int
process (void)
{
	struct  USR_PTR *usr_tmp;

	Dsp_prn_open(0, 2, 17, 
		"Pinnacle Software Mail System - Summary Statistics",
		comm_rec.tco_no, comm_rec.tco_name,
		comm_rec.test_no, comm_rec.test_name,
		(char *)0, (char *)0 );
	Dsp_saverec("                                        Pinnacle Software Mail System - Summary Statistics                                       ");
	Dsp_saverec("");
	Dsp_saverec("  [FN03] [FN05] [FN14] [FN15] [FN16]  ");

	init_array(TRUE);

	if (ALL_USERS)
	{
		usr_tmp = usr_head;
		while (usr_tmp != USR_NULL)
		{
			proc_user(usr_tmp->usr_name);
			usr_tmp = usr_tmp->next;
			init_array(FALSE);
		}

		prnt_totals("TOTALS");
	}
	else
		proc_user(local_rec.usr_name);

	Dsp_srch();
	Dsp_close();

	return(0);
}

/*------------------------------------
| Initialise Array That Holds Totals |
------------------------------------*/
int
init_array (
 int	init_all)
{
	int	i;

	for (i = 0; i < (TOTALS + 1); i++)
	{
		mail_stat[i][USER_S].am_sent = 0;
		mail_stat[i][USER_S].pm_sent = 0;
		mail_stat[i][USER_S].am_rcvd = 0;
		mail_stat[i][USER_S].am_time = 0L;
		mail_stat[i][USER_S].am_unread = 0;
		mail_stat[i][USER_S].pm_rcvd = 0;
		mail_stat[i][USER_S].pm_time = 0L;
		mail_stat[i][USER_S].pm_unread = 0;
		tot_rcvrs[USER_S] = 0;

		if (init_all)
		{
			mail_stat[i][ALL_S].am_sent = 0;
			mail_stat[i][ALL_S].pm_sent = 0;
			mail_stat[i][ALL_S].am_rcvd = 0;
			mail_stat[i][ALL_S].am_time = 0L;
			mail_stat[i][ALL_S].am_unread = 0;
			mail_stat[i][ALL_S].pm_rcvd = 0;
			mail_stat[i][ALL_S].pm_time = 0L;
			mail_stat[i][ALL_S].pm_unread = 0;
			tot_rcvrs[ALL_S] = 0;
		}
	}
	
	return(0);
}

/*----------------------------
| Process A Users Statistics |
----------------------------*/
int
proc_user (
 char *	usr_name)
{
	long	tmp_time;

	sprintf(pmhs_rec.hs_sender, "%-14.14s", usr_name);
	pmhs_rec.hs_date = local_rec.s_date;
	cc = find_rec(pmhs, &pmhs_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(pmhs_rec.hs_sender, usr_name) &&
	       pmhs_rec.hs_date <= local_rec.e_date)
	{
		if ( AM )
		{
			mail_stat[PRI][USER_S].am_sent++;
			mail_stat[PRI][ALL_S].am_sent++;
			mail_stat[TOTALS][USER_S].am_sent++;
			mail_stat[TOTALS][ALL_S].am_sent++;
		}
		else
		{
			mail_stat[PRI][USER_S].pm_sent++;
			mail_stat[PRI][ALL_S].pm_sent++;
			mail_stat[TOTALS][USER_S].pm_sent++;
			mail_stat[TOTALS][ALL_S].pm_sent++;
		}
		
		/*---------------------------------
		| Count Total Number Of Receivers |
		| Of This Piece Of Mail           |
		---------------------------------*/
		pmhl_rec.hl_call_no = pmhs_rec.hs_call_no;
		sprintf(pmhl_rec.hl_receiver, "%-14.14s", " ");
		cc = find_rec(pmhl, &pmhl_rec, GTEQ, "r");
		while (!cc && pmhl_rec.hl_call_no == pmhs_rec.hs_call_no)
		{
			tot_rcvrs[USER_S]++;
			tot_rcvrs[ALL_S]++;
				
			cc = find_rec(pmhl, &pmhl_rec, NEXT, "r");
		}

		cc = find_rec(pmhs, &pmhs_rec, NEXT, "r");
	}

	/*------------------------------
	| Accumulate Stats On Received |
	| Mail For This User           |
	------------------------------*/
	sprintf(pmhl_rec.hl_receiver, "%-14.14s", usr_name);
	pmhl_rec.hl_call_no = 0L;
	cc = find_rec(wk_pmhl, &pmhl_rec, GTEQ, "r");
	while (!cc && !strcmp(pmhl_rec.hl_receiver, usr_name))
	{
		if (pmhl_rec.hl_active_date < local_rec.s_date ||
		    pmhl_rec.hl_active_date > local_rec.e_date) 
		{	
			cc = find_rec(wk_pmhl, &pmhl_rec, NEXT, "r");
			continue;
		}

		cc = find_hash(wk_pmhs, &pmhs_rec, COMPARISON, "r", 
			       pmhl_rec.hl_call_no);
		if (cc)
		{	
			cc = find_rec(wk_pmhl, &pmhl_rec, NEXT, "r");
			continue;
		}

		tmp_time = calc_time();
		if (tmp_time == UNREAD)
		{
			if ( AM )
			{
				mail_stat[PRI][USER_S].am_unread++;
				mail_stat[PRI][ALL_S].am_unread++;
				mail_stat[TOTALS][USER_S].am_unread++;
				mail_stat[TOTALS][ALL_S].am_unread++;
			}
			else
			{
				mail_stat[PRI][USER_S].pm_unread++;
				mail_stat[PRI][ALL_S].pm_unread++;
				mail_stat[TOTALS][USER_S].pm_unread++;
				mail_stat[TOTALS][ALL_S].pm_unread++;
			}
		}
		else
		{
			if ( AM )
			{
				mail_stat[PRI][USER_S].am_time += tmp_time;
				mail_stat[PRI][USER_S].am_rcvd++;
				mail_stat[TOTALS][USER_S].am_time += tmp_time;
				mail_stat[TOTALS][USER_S].am_rcvd++;

				mail_stat[PRI][ALL_S].am_time += tmp_time;
				mail_stat[PRI][ALL_S].am_rcvd++;
				mail_stat[TOTALS][ALL_S].am_time += tmp_time;
				mail_stat[TOTALS][ALL_S].am_rcvd++;
			}
			else
			{
				mail_stat[PRI][USER_S].pm_time += tmp_time;
				mail_stat[PRI][USER_S].pm_rcvd++;
				mail_stat[TOTALS][USER_S].pm_time += tmp_time;
				mail_stat[TOTALS][USER_S].pm_rcvd++;

				mail_stat[PRI][ALL_S].pm_time += tmp_time;
				mail_stat[PRI][ALL_S].pm_rcvd++;
				mail_stat[TOTALS][ALL_S].pm_time += tmp_time;
				mail_stat[TOTALS][ALL_S].pm_rcvd++;
			}
		}

		cc = find_rec(wk_pmhl, &pmhl_rec, NEXT, "r");
	}

	prnt_totals(usr_name);

	return(0);
}

/*----------------------------------------------
| Returns time taken to read mail (in minutes) |
----------------------------------------------*/
long
calc_time (void)
{
	long	read_time;
	int		s_hh, s_mm;
	int		r_hh, r_mm;
	long	act_days;
	long	act_time;
	long	act_mins;

	if (pmhl_rec.hl_fst_rd_date == 0L)
		return(UNREAD);

	act_days = pmhl_rec.hl_fst_rd_date - pmhl_rec.hl_active_date;
	if (act_days > MAX_DAYS)
		return(MAX_HOURS);

	s_hh  = atoi(pmhl_rec.hl_active_time);
	s_mm  = atoi(pmhl_rec.hl_active_time + 3);

	r_hh  = atoi(pmhl_rec.hl_fst_rd_time);
	r_mm  = atoi(pmhl_rec.hl_fst_rd_time + 3);

	act_time = ( (long)r_mm + ((long)r_hh * 60L) ) -
	           ( (long)s_mm + ((long)s_hh * 60L) );
	
	act_mins = act_days * 1440L; /* Number of minutes in a day */
	read_time = act_mins + act_time;

	return(read_time);
}

/*-----------------------------------------------
| Output Full Page Of Users Stats To Dsp Window |
-----------------------------------------------*/
int
prnt_totals (
 char *	usr_name)
{
	char	data_str[200];
	char	tmp_date[11];
	char	head_line[41];
	float	tmp_avg;
	int	tmp_sent, tmp_rcvd;
	int	prt_tot;
	int	i;
	char	am_sent[5], pm_sent[5], tot_sent[5];
	char	am_rcvd[5], pm_rcvd[5], tot_rcvd[5];
	char	am_unread[5], pm_unread[5], tot_unread[5];

	if (!strcmp(usr_name, "TOTALS"))
		prt_tot = ALL_S;
	else
		prt_tot = USER_S;

	tmp_sent = 0;
	tmp_rcvd = 0;
	sprintf(tmp_date, "%-10.10s", DateToString(local_rec.s_date));
	if (!strcmp(usr_name, "TOTALS"))
		strcpy(head_line, "TOTALS FOR ALL USERS");
	else
		sprintf(head_line, "SUMMARY FOR %-14.14s", usr_name);

	sprintf(data_str,
		"                                   ^1 %s FROM %-10.10s TO %-10.10s ^6",
		head_line,
		tmp_date,
		DateToString(local_rec.e_date));
	Dsp_saverec(data_str);
		
	for ( i = 0; i < 5; i++)
	{
		tmp_sent += mail_stat[i][prt_tot].am_sent;
		tmp_sent += mail_stat[i][prt_tot].pm_sent;

		tmp_rcvd += mail_stat[i][prt_tot].am_rcvd;
		tmp_rcvd += mail_stat[i][prt_tot].am_unread;
		tmp_rcvd += mail_stat[i][prt_tot].pm_rcvd;
		tmp_rcvd += mail_stat[i][prt_tot].pm_unread;
	}

	sprintf(data_str,
		"       Total Pieces Of Mail Sent By User          : %5d                Total Pieces Of Mail Received : %5d",
		tmp_sent,
		tmp_rcvd);
	Dsp_saverec(data_str);

	sprintf(data_str, 
		"       Total Number Of Receivers Of Sent Mail     : %5d", 
		tot_rcvrs[prt_tot]);
	Dsp_saverec(data_str);
		
	if (tot_rcvrs[prt_tot] == 0)
		tmp_avg = 0.00;
	else
		tmp_avg =  (float)tot_rcvrs[prt_tot] / (float)tmp_sent;

	sprintf(data_str, 
		"       Average No. Of Receivers Per Mail Document : %5.2f", 
		tmp_avg);
	Dsp_saverec(data_str);

	Dsp_saverec(TOP_BOX);
	Dsp_saverec(HEADER1);
	Dsp_saverec(HEADER2);
	Dsp_saverec(HEADER3);
	Dsp_saverec(MID_BOX);
	for (i = 0; i < 5; i++)
	{
		calc_avg(i, prt_tot);

		/*---------------------------------
		| Replace zero values with spaces |
		---------------------------------*/
		chk_fld (mail_stat[i][prt_tot].am_sent);
		sprintf(am_sent, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].pm_sent);
		sprintf(pm_sent, "%-4.4s", tmp_fld);
	
		chk_fld (mail_stat[i][prt_tot].am_sent +
	         	mail_stat[i][prt_tot].pm_sent);
		sprintf(tot_sent, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].am_rcvd);
		sprintf(am_rcvd, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].pm_rcvd);
		sprintf(pm_rcvd, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].am_rcvd +
	         	mail_stat[i][prt_tot].pm_rcvd);
		sprintf(tot_rcvd, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].am_unread);
		sprintf(am_unread, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].pm_unread);
		sprintf(pm_unread, "%-4.4s", tmp_fld);

		chk_fld (mail_stat[i][prt_tot].am_unread +
	         	mail_stat[i][prt_tot].pm_unread);
		sprintf(tot_unread, "%-4.4s", tmp_fld);

		/*-------------------------
		| Save line to Dsp screen |
		-------------------------*/
		sprintf(data_str, disp_mask, pri_desc[i],
			am_sent,  pm_sent, tot_sent,
			am_rcvd,  am_avg,  am_unread,
			pm_rcvd,  pm_avg,  pm_unread,
			tot_rcvd, tot_avg, tot_unread);
		Dsp_saverec(data_str);
	}
		
	calc_avg(TOTALS, prt_tot);

	Dsp_saverec(MID_BOX);

	/*---------------------------------
	| Replace zero values with spaces |
	---------------------------------*/
	chk_fld (mail_stat[TOTALS][prt_tot].am_sent);
	sprintf(am_sent, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].pm_sent);
	sprintf(pm_sent, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].am_sent +
	         mail_stat[TOTALS][prt_tot].pm_sent);
	sprintf(tot_sent, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].am_rcvd);
	sprintf(am_rcvd, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].pm_rcvd);
	sprintf(pm_rcvd, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].am_rcvd +
         	mail_stat[TOTALS][prt_tot].pm_rcvd);
	sprintf(tot_rcvd, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].am_unread);
	sprintf(am_unread, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].pm_unread);
	sprintf(pm_unread, "%-4.4s", tmp_fld);

	chk_fld (mail_stat[TOTALS][prt_tot].am_unread +
         	mail_stat[TOTALS][prt_tot].pm_unread);
	sprintf(tot_unread, "%-4.4s", tmp_fld);

	/*-------------------------
	| Save line to Dsp screen |
	-------------------------*/
	sprintf(data_str, disp_mask,
		pri_desc[TOTALS],
		am_sent,  pm_sent, tot_sent,
		am_rcvd,  am_avg,  am_unread,
		pm_rcvd,  pm_avg,  pm_unread,
		tot_rcvd, tot_avg, tot_unread);
	Dsp_saverec(data_str);

	Dsp_saverec(END_BOX);

	return (EXIT_SUCCESS);
}

/*-----------------------------------
| If field is zero then set tmp_fld |
| to spaces otherwise set tmp_fld   |
| to correct value                  |
-----------------------------------*/
void
chk_fld (
 int	chk_val)
{
	strcpy(tmp_fld, "    ");
	if (chk_val != 0)
		sprintf(tmp_fld, "%4d", chk_val);
}

/*---------------------------------
| Calculate Average Times To Read |
| Mail Received.                  |
---------------------------------*/
int
calc_avg (
 int	pri_no,
 int	tot_type)
{
	int	tmp_hh;
	int	tmp_mm;
	int	tmp_ss;
	long	tmp_time;
	long	tot_time;
	int	tot_rcvd;

	strcpy(am_avg, "         ");
	strcpy(pm_avg, "         ");
	strcpy(tot_avg, "         ");
	if (mail_stat[pri_no][tot_type].am_rcvd != 0)
	{
		tmp_time = (60L * mail_stat[pri_no][tot_type].am_time) / 
			    (long)mail_stat[pri_no][tot_type].am_rcvd;
		tmp_hh = tmp_time / 3600L;
		if (tmp_hh > 0)
			tmp_time = tmp_time - (long)(3600 * tmp_hh);
		tmp_mm = tmp_time / 60L;
		if (tmp_mm > 0)
			tmp_time = tmp_time - (long)(tmp_mm * 60L);
		tmp_ss = tmp_time;
	
		sprintf(am_avg, "%3d:%02d:%02d", tmp_hh, tmp_mm, tmp_ss);
	}

	if (mail_stat[pri_no][tot_type].pm_rcvd != 0)
	{
		tmp_time = (60L * mail_stat[pri_no][tot_type].pm_time) / 
			    (long)mail_stat[pri_no][tot_type].pm_rcvd;
		tmp_hh = tmp_time / 3600L;
		if (tmp_hh > 0)
			tmp_time = tmp_time - (long)(3600 * tmp_hh);
		tmp_mm = tmp_time / 60L;
		if (tmp_mm > 0)
			tmp_time = tmp_time - (long)(tmp_mm * 60L);
		tmp_ss = tmp_time;
	
		sprintf(pm_avg, "%3d:%02d:%02d", tmp_hh, tmp_mm, tmp_ss);
	}
	
	tot_time = ( mail_stat[pri_no][tot_type].am_time + 
		     mail_stat[pri_no][tot_type].pm_time );

	tot_rcvd = ( mail_stat[pri_no][tot_type].am_rcvd + 
		     mail_stat[pri_no][tot_type].pm_rcvd );

	if (tot_rcvd != 0)
	{
		tmp_time = (60L * tot_time) / (long)tot_rcvd;
		tmp_hh = tmp_time / 3600L;
		if (tmp_hh > 0)
			tmp_time = tmp_time - (long)(3600 * tmp_hh);
		tmp_mm = tmp_time / 60L;
		if (tmp_mm > 0)
			tmp_time = tmp_time - (long)(tmp_mm * 60L);
		tmp_ss = tmp_time;
	
		sprintf(tot_avg, "%3d:%02d:%02d", tmp_hh, tmp_mm, tmp_ss);
	}

	return (EXIT_SUCCESS);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		swide();
		clear();
		
		box(0, 3, 131, 3); 

		rv_pr(ML(mlMenuMess129),(132 - strlen(ML(mlMenuMess129))) / 2,0,1);

		move (0, 1);
		line(132);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

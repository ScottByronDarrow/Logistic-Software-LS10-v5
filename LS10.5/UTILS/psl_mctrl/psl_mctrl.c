/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( psl_mctrl.c 	)                             |
|  Program Desc  : ( Logistic Software Mail Control System.       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  pmre, pmse,     ,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmre, pmse,     ,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (09/04/91)      | Author       : Campbell Mander   |
|---------------------------------------------------------------------|
|  Date Modified : (01/05/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (01/07/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (19/07/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/11/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (08/02/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (13.02.95)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : (01/05/91) Include notification (message) calling. |
|  (01/07/91)    : Now checks to see if mail is active yet or not.    |
|                : Non-active mail does not cause notification.       |
|  (19/07/91)    : First 7 characters of user name is checked for the |
|                : comparison as /etc/utmp only holds 7 characters    |
|                : for the user log name.                             |
|  (05/11/92)    : SC 7898 PSL. Put *'s next to those priorities that |
|                : have unread mail.                                  |
|  (08/02/93)    : Make message use the PATH. PSL 8431 related.       |
|  (13.02.95)    : Fixed : truncation of username                     |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: psl_mctrl.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_mctrl/psl_mctrl.c,v 5.2 2001/08/09 09:27:31 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<sys/types.h>
#include	<utmp.h>

#define	NOT_ACTIVE	( pmre_rec.re_act_date > lsystemDate || \
			  ( pmre_rec.re_act_date == lsystemDate && \
			    strcmp(pmre_rec.re_act_time, curr_time) > 0) )

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
		{"pmre_lst_time"}
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
	} pmre_rec;

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
		{"pmse_stat_flag"}
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

static char
	*data	= "data",
	*pmre	= "pmre",
	*pmse	= "pmse";

	/*============================
	| Local & Screen Structures. |
	============================*/
	int	priority[5];
	int	pri_unseen[5];
	long	lsystemDate;
	char	curr_time[6];
	char	done_user [1024] [10];
	int	last_user = 0;

/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void process (int argc, char * argv []);
int proc_user (char *usr_name);
int sendmsg (char *uptr);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	lsystemDate = TodaysDate ();
	OpenDB ();
	process (argc, argv);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (pmre, pmre_list, pmre_no_fields, "pmre_id_no");
	open_rec (pmse, pmse_list, pmse_no_fields, "pmse_call_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmre);
	abc_fclose (pmse);

	abc_dbclose (data);
}

void
process (
 int	argc,
 char	*argv [])
{
	int	PRI_NOTIFY = 10;
	int	i;
	int	j;
	int	UNSEEN;
	int	IMMED_NOTIFY = FALSE;
	int	USR_OK = FALSE;
	char	usr_name[15];
	char	term_num[15];
	struct utmp	*uptr;

	if (argc > 2)
		IMMED_NOTIFY = TRUE;

	if (argc > 1)
	{
		i = atoi (argv[1]);
		if (i > 0 && i < 6)
			PRI_NOTIFY = i;
		else 
			PRI_NOTIFY = 5;
	}

	UNSEEN = 0;
	uptr = getutent ();
	while (uptr)
	{
		if (uptr->ut_type == USER_PROCESS)
		{
			/*
			 * Grab User Name
			 *	Note that the utmp entry is 8 bytes, but if the 8th byte
			 *	is not-null, it should be copied across
			 */
			sprintf (usr_name, "%.8s", uptr -> ut_user);

			for (i = 0, j = 0; i < last_user; i++)
			{
				if (!strcmp (usr_name, done_user [i]))
				{
					j = 1;
					break;
				}
			}
			if (j)
			{
			    uptr = getutent ();
			    continue;
			}
			strcpy (done_user[last_user++], usr_name);
			clip (usr_name);

			/*----------------------
			| Grab Terminal Number |
			----------------------*/
			strcpy (term_num, uptr->ut_line);

			if (IMMED_NOTIFY)
			{
				USR_OK = FALSE;

				/*---------------------------------------
				| Check if user name was passed in args |
				---------------------------------------*/
				for (i = 2; i < argc; i++)
				{
					clip (argv[i]);
					if (strlen (argv[i]) > 7)
						argv[i][7] = 0;
					if (!strcmp (argv[i], usr_name))
					{
						USR_OK = TRUE;
						break;
					}
				}
			}
			else
				USR_OK = TRUE;

			if (USR_OK)
				UNSEEN = proc_user (usr_name);
			else
			{
				uptr = getutent ();
				continue;
			}

			if (UNSEEN <= PRI_NOTIFY && UNSEEN)
				sendmsg (usr_name);
		}
		uptr = getutent ();
	}
	endutent ();
}

int
proc_user (
 char *usr_name)
{
	int	i;
	int	UNSEEN = 0;

	for (i = 0; i < 5; i++)
	{
		priority[i] = 0;
		pri_unseen[i] = FALSE;
	}

	/*-----------------------------------
	| Process all current mail for user |
	-----------------------------------*/
	sprintf (pmre_rec.re_receiver, "%-14.14s", usr_name);
	strcpy (pmre_rec.re_status, "C");
	pmre_rec.re_call_no = 0L;
	cc = find_rec (pmre, &pmre_rec, GTEQ, "r");
	while
	    ( !cc &&
	      !strncmp (clip(pmre_rec.re_receiver), usr_name, 7) && 
	      !strcmp (pmre_rec.re_status, "C") )
	{
		sprintf(curr_time, "%-5.5s", ttod() + 9);
		if (NOT_ACTIVE)
		{
			cc = find_rec (pmre, &pmre_rec, NEXT, "r");
			continue;
		}

		/*-------------------------
		| Lookup priority on pmse |
		-------------------------*/
		cc = find_hash (pmse, &pmse_rec, COMPARISON, "r", pmre_rec.re_call_no);
		if (cc)
		{
			cc = find_rec (pmre, &pmre_rec, NEXT, "r");
			continue;
		}

		priority[pmse_rec.se_priority - 1]++;
		if (pmre_rec.re_seen[0] == 'N')
		{
			if (UNSEEN == 0 || pmse_rec.se_priority < UNSEEN)
				UNSEEN = pmse_rec.se_priority;

			pri_unseen[pmse_rec.se_priority - 1] = TRUE;
		}

		cc = find_rec (pmre, &pmre_rec, NEXT, "r");
	}

	return (UNSEEN);
}

int
sendmsg (
 char *uptr)
{
	char	sysmsg[256];

	sprintf (sysmsg, "message -u%s \"You have mail: [ %sP1->%d ] [ %sP2->%d ] [ %sP3->%d ] [ %sP4->%d ] [ %sP5->%d ]\"",
		uptr,
		(pri_unseen[0] == TRUE) ? "*" : "", priority[0],
		(pri_unseen[1] == TRUE) ? "*" : "", priority[1],
		(pri_unseen[2] == TRUE) ? "*" : "", priority[2],
		(pri_unseen[3] == TRUE) ? "*" : "", priority[3],
		(pri_unseen[4] == TRUE) ? "*" : "", priority[4]);

	system (sysmsg);
	return (EXIT_SUCCESS);
}


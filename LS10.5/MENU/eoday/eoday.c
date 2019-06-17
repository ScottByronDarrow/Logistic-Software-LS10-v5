/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( eoday.c        )                                 |
|  Program Desc  : ( End Of Day Program For All Terminals.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (16/03/90)      | Modified  by  : Scott B. Darrow. |
|                : (27/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/01/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (15/10/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (28/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      : (16/03/90) - Total Rewrite.                        |
|                : (27/08/90) - Added fflush to line drawing. S.B.D.  |
|                : (16/01/91) - Fixed exec for console.               |
|                : (05/09/97) - Modified for Multilingual Conversion. |
|                : (15/10/97) - Updated execlp for Multilingual Conv. |
|                : (28/08/1999) - Ported to ANSI format.              |
|                :                                                    |
|                :                                                    |
| $Log: eoday.c,v $
| Revision 5.2  2001/08/09 05:13:26  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:20  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:18  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:58  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/14 08:32:16  scott
| Updated as not printing date correctly
|
| Revision 2.0  2000/07/15 09:00:10  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:47:11  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 09:41:55  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.9  1999/09/17 07:26:55  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.8  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: eoday.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/eoday/eoday.c,v 5.2 2001/08/09 05:13:26 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<account.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

	FILE	*acc_in;
	FILE	*rfin;

	char	buffer[300];
	char	*curr_user;

	int	line_no = 0;
	int	fd;
	int	rfd;
	int	flag;

	struct	save_type	{
		long	_time;
		char	_desc[61];
	} Save_rec;

	struct	tm	*tme;

/*=========================
| Main procesing Routine. |
=========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	draw_screen		(void);
void	proc_audit		(void);
void	proc_user		(char * user, int tty);
int		check_con		(void);
int		rm_acc			(char * fname);
int		save_acc		(void);
int		lockout			(int make_lock);
void	logout			(void);


/*=========================
| Main procesing Routine. |
=========================*/
int
main (
 int	argc,
 char * argv [])
{
	int	slot = ttyslt();
	
	curr_user = getenv("LOGNAME");

	if (curr_user == (char*)0)
	{
		print_at(0,0,ML(mlMenuMess103));
		return (EXIT_FAILURE);
	}

	draw_screen();
	
	if ( slot == 1 )	
	{
		if (check_con())
		{
			lockout( FALSE );
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
	}

	proc_audit();
	logout();

	if (slot == 1)
	{
		lockout( TRUE );

		if (access("RUN_EODAY",00) > -1)
		{
			if ( fork() == 0 )
			{
				execlp(ML(mlMenuMess268),ML(mlMenuMess268),(char *)0);
				return (EXIT_SUCCESS);
			}
			else
				wait( (int *) 0 );
		}
	}
	clear();
	crsr_on();
	rset_tty();
	system("ON");
	kill(0,9);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===============
| Exit Program. |
===============*/
void
shutdown_prog ()
{
	snorm();
	rset_tty();
}

/*===================
| Draw main screen. |
===================*/
void
draw_screen (void)
{
	init_scr();
	set_tty(); 
	clear();
	swide();
	crsr_off();
	box(0,0,130,19);
	rv_pr(ML(mlMenuMess105),42,1,1);
	print_at(2,46,ML(mlMenuMess104),curr_user,ttyslt());
	move(1,3);line(129);fflush(stdout);
}

/*==============================================
| Process audit trail for Logistic Accounting. |
==============================================*/
void
proc_audit (void)
{
	int	tty;
	char	*sptr = getenv("PROG_PATH");
	char	filename[200];

	rv_pr(ML(mlMenuMess106),47,5,1);
	box(12,6,104,3);
	rv_pr(ML(mlMenuMess210),13,7,0);
	move(13,8);
	line(103);
	fflush(stdout);

	sprintf(filename,"%s/BIN/ACCOUNT",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	if (access(filename,00) == -1)
		return;

	sprintf(buffer,"ls %s/BIN/ACCOUNT/%s*",(sptr != (char *)0) ? sptr : "/usr/LS10.5",curr_user);

	if ((acc_in = popen(buffer,"r")) == 0)
		return;

	sptr = fgets(buffer,200,acc_in);

	while (sptr != (char *)0)
	{
		tty = atoi(sptr + strlen(sptr) - 3);
		proc_user(curr_user,tty);

		sptr = fgets(buffer,200,acc_in);
	}
	fclose(acc_in);
}

/*==================================================
| Main processing routine for Pinnacle Accounting. |
==================================================*/
void
proc_user (
 char *	user,
 int	tty)
{
	char	action[120];
	char	new_file[120];
	char	old_file[120];

	sprintf(old_file,"%s.%03d",user,tty);
	sprintf(new_file,".b/%s",old_file);

	fd = UserAccountOpen(old_file,"r");
	rfd = UserAccountOpen(new_file,"a");

	cc = RF_READ(fd, (char *) &acc_rec);

	while (!cc)
	{
		tme = localtime(&acc_rec._time);
		sprintf(action," %-8.8s | %3d |%02d/%02d/%04d| %02d:%02d:%02d | %s ",
			user,
			tty,
			tme->tm_mday,
			tme->tm_mon + 1,
			tme->tm_year + 1900,
			tme->tm_hour,
			tme->tm_min,
			tme->tm_sec,
			acc_rec._desc);

		rv_pr(action,13,9,0);

		save_acc();
		sleep(1);
		cc = RF_READ(fd, (char *) &acc_rec);
	}
	RF_CLOSE(fd);
	RF_CLOSE(rfd);
	rm_acc( old_file );
	fd = UserAccountOpen(old_file,"r");
	RF_CLOSE(fd);
}

/*========================================
| Validate all users logged out for Day. |
========================================*/
int
check_con (void)
{
	int		i;
	int		cnt = 0;
	char	user[80];
	char	user_info[101][80];
	FILE	*p;

	p = popen("who","r");
	while(fgets(user, 70, p) != NULL)
	{
		strcpy(user_info[cnt], user);
		cnt++;
	}
	pclose(p);
	if (cnt > 1)
	{
		for (i = 0 ; i < cnt ; i++)
		{
			print_at(5 + i,46,"%s", user_info[i]);
		}
		/*Console May Not Logout Until All Others Have Logged Out.*/
		rv_pr(ML(mlMenuMess107),36,7 + cnt, 1);
		for (i = 1 ; i < 7 ; i++)
		{
			putchar(BELL);	
			fflush(stdout);
			sleep(1);
		}
		return(1);
	}
	return(0);
}

/*======================
| Removed unlink file. |
======================*/
int
rm_acc (
 char *	fname)
{
	char	*sptr = getenv("PROG_PATH");
	char	filename[200];
	sprintf(filename,"%s/BIN/ACCOUNT/%s",(sptr != (char *)0) ? sptr : "/usr/LS10.5",fname);

	return(unlink( filename ));
}
/*===============================
| Save Accounting Transactions. |
===============================*/
int
save_acc (void)
{
	sprintf(Save_rec._desc,"%-60.60s",acc_rec._desc);
	Save_rec._time = acc_rec._time;

	return(RF_ADD(rfd,(char *) &Save_rec));
}
/*===========================
| Create Menu Lockout file. |
===========================*/
int
lockout (
 int	make_lock)
{
	char	*sptr = getenv("PROG_PATH");
	char	filename[200];
	sprintf(filename,"%s/BIN/.backup",(sptr!=(char *)0) ? sptr : "/usr/LS10.5");
	if ( make_lock )
		return(creat( filename,00777 ));

	return(unlink( filename ));
}

/*==================
| Logout terminal. |
==================*/
void
logout (void)
{
	box(12,12,104,1);
	rv_pr(ML(mlMenuMess108),46,13,1);
	sleep(1);
}

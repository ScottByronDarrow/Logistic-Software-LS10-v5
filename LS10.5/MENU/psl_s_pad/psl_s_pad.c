/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_s_pad.c    )                                 |
|  Program Desc  : ( Pinnacle Scratch pad.                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  pmsp.                                             |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/05/91         |
|---------------------------------------------------------------------|
|  Date Modified : (28/06/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (16/07/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (10/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (25/09/97)      | Modified  by  : Ana Marie Tario  |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (28/06/91) - Added TXT_REQD for text routines.     |
|  (16/07/91)    : Changed order of parameters passed to txt_open().  |
|  (10/04/94)    : PSL 10673 - Online conversion                      |
|  (25/09/97)    : Incorporated multilingual conversion.              |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: psl_s_pad.c,v $
| Revision 5.2  2001/08/09 05:13:51  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:58  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:15  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:23  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/02/18 01:56:29  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.9  1999/12/06 01:47:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/25 10:24:01  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.7  1999/09/29 10:11:18  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:14  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:56  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_s_pad.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_s_pad/psl_s_pad.c,v 5.2 2001/08/09 05:13:51 scott Exp $";

#ifdef	LINUX
#define	_XOPEN_SOURCE	500		/* Don't even ASK */
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif	/* LINUX */

#define		NO_SCRGEN
#define		TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <pwd.h>

struct	passwd	*pw;

	/*=========================
	| User scratch pad notes. |
	=========================*/
	struct dbview pmsp_list[] ={
		{"pmsp_user_name"},
		{"pmsp_line_no"},
		{"pmsp_pad_text"}
	};

	int pmsp_no_fields = 3;

	struct {
		char	sp_user_name [15];
		int		sp_line_no;
		char	sp_pad_text [71];
	} pmsp_rec;

	char	*curr_user;
	int	pass_trys = 0;
	int	tx_window,
		tx_lines;

/*============================
| Local function prototypes  |
============================*/
void	process			(void);
void	add_notes		(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	shutdown_prog	(void);
int		check_passwd	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc > 1)
		curr_user = getenv( "LOGNAME" );

	OpenDB ();

	init_scr ();
	set_tty ();

	process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Process information. |
======================*/
void
process (
 void)
{
	clear ();
	crsr_off ();
	box (1, 1, 78, 19);
	box (4, 5, 72, 14);
	rv_pr (ML (mlMenuMess237), 23, 2, 1);
	if (curr_user == (char *) 0)
	{
		while (!check_passwd() && pass_trys < 3);
	
		if (pass_trys == 3)
		{
			shutdown_prog ();
			return;
		}
	}
	sprintf (err_str, ML(mlMenuMess183), curr_user);
	print_at (4, (78 - strlen( err_str )) / 2,"%R %s", err_str);
	
	add_notes ();
}

/*=======================================
| Add note pad information.		|
=======================================*/
void
add_notes (
 void)
{
	char	disp_str [71];
	int		i,
			last_line = 0;

	txt_close (tx_window, FALSE);

	tx_window = txt_open( 5, 4, 14, 70, 300,"");

	sprintf(pmsp_rec.sp_user_name, "%-14.14s", curr_user);
	pmsp_rec.sp_line_no = 0;
	cc = find_rec("pmsp", &pmsp_rec, GTEQ, "r");
	while (!cc && 
		!strncmp( pmsp_rec.sp_user_name, curr_user, strlen( curr_user)))
	{
		sprintf(disp_str,"%-70.70s",pmsp_rec.sp_pad_text);

		txt_pval( tx_window, disp_str, 0 );

		cc = find_rec("pmsp", &pmsp_rec, NEXT, "r");
	}

	tx_lines = txt_edit ( tx_window );
	if ( !tx_lines )
	{
		sprintf(pmsp_rec.sp_user_name, "%-14.14s", curr_user);
		pmsp_rec.sp_line_no = 0;
		cc = find_rec("pmsp", &pmsp_rec, GTEQ, "r");
		while (!cc && 
			!strncmp( pmsp_rec.sp_user_name, curr_user, strlen( curr_user)))
		{
			abc_delete( "pmsp" );
			cc = find_rec("pmsp", &pmsp_rec, GTEQ, "r");
		}
		txt_close( tx_window, FALSE );
		return;
	}
	for ( i = 1; i <= tx_lines; i++ )
	{
		last_line = i;

		sprintf(pmsp_rec.sp_user_name, "%-14.14s", curr_user);
		pmsp_rec.sp_line_no = i - 1;
		cc = find_rec("pmsp", &pmsp_rec, COMPARISON, "r");
		if ( cc )
		{
			sprintf( pmsp_rec.sp_pad_text,"%-70.70s", 
						txt_gval( tx_window, i ));
			
			cc = abc_add( "pmsp", &pmsp_rec );
			if ( cc )
				file_err( cc, "pmsp", "DBADD" );
		}
		else
		{
			sprintf( pmsp_rec.sp_pad_text,"%-70.70s", 
						txt_gval( tx_window, i ));
			
			cc = abc_update( "pmsp", &pmsp_rec );
			if ( cc )
				file_err( cc, "pmsp", "DBUPDATE" );
		}
	}
	txt_close( tx_window, FALSE );

	sprintf(pmsp_rec.sp_user_name, "%-14.14s", curr_user);
	pmsp_rec.sp_line_no = last_line;
	cc = find_rec("pmsp", &pmsp_rec, GTEQ, "r");
	while (!cc && 
		!strncmp( pmsp_rec.sp_user_name, curr_user, strlen( curr_user)))
	{
		abc_delete( "pmsp" );
		cc = find_rec("pmsp", &pmsp_rec, GTEQ, "r");
	}
	return;
}
	
/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("pmsp", pmsp_list, pmsp_no_fields, "pmsp_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("pmsp");
	abc_dbclose ("data");
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

/*===================
| Process password. |
===================*/
int
check_passwd (
 void)
{
#ifdef	SCO
	return (TRUE);
#else
	char	salt [3];

	char	inp_passwd [24];
	char	chk_passwd [24];

	cl_box( 20, 10, 40, 1);
	print_at(11, 22,"9s", ML(mlMenuMess097));
	sprintf (inp_passwd, "%s", getpass (" "));
	
	setpwent ();

	pw = getpwent ();
	while (pw != (struct passwd *)0)
	{
		sprintf( salt, "%-2.2s", pw->pw_passwd);
		sprintf (chk_passwd, "%s", crypt (inp_passwd, salt));
		if ( !strcmp( pw->pw_passwd, chk_passwd ) )
		{
			curr_user = pw->pw_name;
			pass_trys = 0;
			print_at(11,22,"%38.38s", " ");
			erase_box( 20, 10, 40, 1);
			return( TRUE );
		}
		pw = getpwent ();
	}
	pass_trys++;
	print_at (11, 22, "9s", ML (mlStdMess140));
	fflush (stdout);
	sleep (sleepTime);
	print_at (11, 22, "%38.38s", " ");
	return (FALSE);
#endif
}

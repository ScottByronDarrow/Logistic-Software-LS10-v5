/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (dsp_session.c)                                    |
|  Program Desc  : (Display Actions Performed During Use       )      |
| $Id: _session.c,v 5.2 2001/08/09 05:13:24 scott Exp $
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/08/88         |
|---------------------------------------------------------------------|
| $Log: _session.c,v $
| Revision 5.2  2001/08/09 05:13:24  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:14  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:09  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  2000/07/14 01:33:04  scott
| Updated as general maintenance and clean up.
|
| Revision 1.8  1999/12/06 01:47:11  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 09:41:55  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/17 07:26:54  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _session.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dsp_session/_session.c,v 5.2 2001/08/09 05:13:24 scott Exp $";

#include 	<pslscr.h>
#include	<time.h>
#include	<account.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

FILE	*fin;
char	buffer [300];

struct	{
	char	dummy [11];
	char	user_name [9];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "user_name",	 4, 15, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "User Name.", " Default : All ",
		YES, NO,  JUSTLEFT, "", "", local_rec.user_name},
	{0, LIN, "dummy",	 4, 20, CHARTYPE,
		"A", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

/*===========================
| Function Prototypes       |
===========================*/
int		main			(int, char * argv []);
void	shutdown_prog	(void);
int		spec_valid 		(int);
void	ShowUsers 		(char *);
char *	GetTheUser 		(char *);
void	Process 		(void);
void	ProcessUser 	(char *, int);
int		WriteLine 		(char *);
int		heading 		(int);

int
main (
 int	argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	swide ();
	set_tty (); 

	if (argc != 1)
	{
		sprintf (local_rec.user_name,"%-.8s",argv [1]);
		Process ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	set_masks ();

	while (prog_exit == 0)
	{
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Process ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	char	*sptr;

	if (LCHECK ("user_name"))
	{
		if (SRCH_KEY)
		{
			ShowUsers (temp_str);
			return (EXIT_SUCCESS);
		}

		sptr = local_rec.user_name;

		while (*sptr)
		{
			*sptr = tolower (*sptr);
			sptr++;
		}

		DSP_FLD ("user_name");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
ShowUsers (
 char *	key_val)
{
	char	*sptr = getenv ("PROG_PATH");
	char	user_name [9];

	sprintf (buffer,"ls %s/BIN/ACCOUNT/*", (sptr != (char *)0) ? sptr : "/usr/LS10.5");

	if ((fin = popen (buffer,"r")) == 0)
		return;

	work_open ();
	
	save_rec ("#User.","#");

	sptr = fgets (buffer,200,fin);

	while (sptr != (char *)0)
	{
		sptr = GetTheUser (sptr);

		if (sptr != (char *)0 && !strncmp (sptr,key_val,strlen (key_val)))
		{
			sprintf (user_name,"%-8.8s",sptr);
			cc = save_rec (user_name," ");
			if (cc)
				break;
		}
		sptr = fgets (buffer,200,fin);
	}
	cc = disp_srch ();
	work_close ();
	pclose (fin);
}

char *
GetTheUser (
 char *	sptr)
{
	char	*tptr = (sptr + strlen (sptr) - 1);

	while (tptr > sptr && *tptr != '.')
		tptr--;

	*tptr = '\0';

	while (tptr > sptr && *tptr != '/')
		tptr--;

	return ((tptr == sptr) ? (char *)0 : tptr + 1);
}

void
Process (void)
{
	int	tty;
	char	*sptr = getenv ("PROG_PATH");

	strcpy (err_str, " LS/10 USER SESSION .");

	Dsp_prn_open (0, 0, 15, err_str, (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0);

	Dsp_saverec ("   User   | Tty |    Date    |   Time   | Option                                                       ");
	Dsp_saverec ("");
	Dsp_saverec ("  [REDRAW] [PRINT] [NEXT SCN] [PREV SCN] [EDIT/END]  ");

	if (strcmp (local_rec.user_name,"        "))
		sprintf (buffer,"ls %s/BIN/ACCOUNT/%s*", (sptr != (char *)0) ? sptr : "/usr/LS10.5",clip (local_rec.user_name));
	else
		sprintf (buffer,"ls %s/BIN/ACCOUNT/*", (sptr != (char *)0) ? sptr : "/usr/LS10.5");

	if ((fin = popen (buffer,"r")) == 0)
	{
		Dsp_srch ();
		Dsp_close ();
		return;
	}

	sptr = fgets (buffer,200,fin);

	while (sptr != (char *)0)
	{
		tty = atoi (sptr + strlen (sptr) - 3);

		sptr = GetTheUser (sptr);

		if (sptr != (char *)0)
			ProcessUser (sptr,tty);

		sptr = fgets (buffer,200,fin);
	}
	pclose (fin);

	Dsp_srch ();
	Dsp_close ();
}

void
ProcessUser (
 char *	user,
 int	tty)
{
	int	fd;
	char	action [300];
	struct	tm	*localtime (const time_t *);
	struct	tm	*u_tme;

	sprintf (action,"%s.%03d",user,tty);

	fd = UserAccountOpen (action,"r");

	cc = RF_READ (fd, (char *) &acc_rec);

	while (!cc)
	{
		u_tme = localtime (&acc_rec._time);
		sprintf (action," %-8.8s ^E %3d ^E %02d/%02d/%04d ^E %02d:%02d:%02d ^E %s ",
			user,
			tty,
			u_tme->tm_mday,
			u_tme->tm_mon + 1,
			u_tme->tm_year + 1900,
			u_tme->tm_hour,
			u_tme->tm_min,
			u_tme->tm_sec,
			acc_rec._desc);

		cc = WriteLine (action);
		if (cc)
			break;

		cc = RF_READ (fd, (char *) &acc_rec);
	}
	WriteLine ("^^GGGGGGGGGGJGGGGGJGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");
	RF_CLOSE (fd);
}

int
WriteLine (
 char *	desc)
{
	return (Dsp_saverec (desc));
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		/*---------------------------
		| Menu Session Log Display. |
		---------------------------*/
		sprintf (err_str, " %s ", ML (mlMenuMess146));
		rv_pr (err_str, 52,0,1);

		move (0,1);
		line (131);

		box (0,3,132,1);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

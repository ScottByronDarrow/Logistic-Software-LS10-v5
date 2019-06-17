/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( dsp_fast.c     )                                 |
|  Program Desc  : ( Display Fast Access For User(s)              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  MENUSYS/.fa                                       |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :                                                    |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (31/08/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (19/04/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (12/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (28/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      : (05/10/90) - General Update for New Scrgen. S.B.D. |
|  (19/04/91)    : Updated dsp to Dsp.                                |
|  (09/09/97)    : Updated for Multilingual Conversion.               |
|  (28/08/1999)  : Converted to ANSI format.                          |
|                :                                                    |
|                :                                                    |
| $Log: dsp_fast.c,v $
| Revision 5.2  2001/08/09 05:13:23  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:16  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:12  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:55  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:08  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/12/06 01:47:10  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/11/16 09:41:54  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.5  1999/09/17 07:26:54  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dsp_fast.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dsp_fast/dsp_fast.c,v 5.2 2001/08/09 05:13:23 scott Exp $";

#include 	<pslscr.h>
#include	<fast.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

FILE	*fin;
char	buffer[300];
int		heading (int);

struct	{
	char	dummy[11];
	char	user_name[9];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "user_name", 4, 15, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", " ", "User Name.", " Default : All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.user_name}, 
	{0, LIN, "dummy", 4, 20, CHARTYPE, 
		"A", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.dummy}, 

};

/*========================
| Function prototypes    |
========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	ShowUsers		(char *);
char *	get_user		(char *);
void	process			(void);
void	proc_user		(char *);


int
main (
 int	argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr();
	set_tty(); 

	if (argc != 1)
	{
		sprintf(local_rec.user_name,"%-8.8s",argv[1]);
		process();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	set_masks();

	while (!prog_exit)
	{
		search_ok = init_ok = 1;
		entry_exit = edit_exit = prog_exit = restart = 0;
		init_vars(1);	

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
	shutdown_prog();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	FinishProgram ();
}

int
spec_valid (
 int	field)
{
	char	*sptr;

	if (!strcmp(FIELD.label,"user_name"))
	{
		if (SRCH_KEY)
		{
			ShowUsers(temp_str);
			return(0);
		}

		sptr = local_rec.user_name;

		while (*sptr)
		{
			*sptr = tolower(*sptr);
			sptr++;
		}

		display_field(field);
		return(0);
	}

	return(0);
}

void
ShowUsers (
 char *	key_val)
{
	char	*sptr;
	char	user_name[9];

	sptr = getenv("PROG_PATH");

	sprintf(buffer, "ls %s/BIN/MENUSYS/*.fa", (sptr) ? sptr : "/usr/LS10.5");

	if (!(fin = popen(buffer,"r")))
		return;

	work_open();
	
	save_rec("#User.","#");

	while ((sptr = fgets(buffer,200,fin)))
	{
		sptr = get_user(sptr);
		if (sptr && !strncmp(sptr,key_val,strlen(key_val)))
		{
			sprintf(user_name,"%-8.8s",sptr);
			if (save_rec(user_name," "))
				break;
		}
	}
	disp_srch();
	work_close();
	fclose(fin);
}

/*=======================================================
| Split the user name out from the strings passed	|
| ie XXX/<user_name>.fa					|
=======================================================*/
char *
get_user (
 char *	sptr)
{
	char	*tptr = (sptr + strlen(sptr) - 1);

	while (tptr > sptr && *tptr != '.')
		tptr--;

	*tptr = '\0';

	while (tptr > sptr && *tptr != '/')
		tptr--;

	return((tptr == sptr) ? (char *)0 : tptr + 1);
}

void
process (void)
{
	char	*sptr = getenv("PROG_PATH");

	Dsp_open(0,0,15);
	Dsp_saverec("   User   |  Code  | Menu Data File | Opts ");
	Dsp_saverec("");
	Dsp_saverec("  [FN03]   [FN14]   [FN15]   [FN16]  ");

	/*---------------------------------------
	| If displaying for specific user	|
	---------------------------------------*/
	if (strcmp(local_rec.user_name,"        "))
		proc_user(local_rec.user_name);
	else
	{
		sprintf(buffer, "ls %s/BIN/MENUSYS/*.fa", (sptr) ? sptr :
								   "/usr/LS10.5");

		if (!(fin = popen(buffer,"r")))
		{
			Dsp_srch();
			Dsp_close();
			return;
		}

		sptr = fgets(buffer,200,fin);

		while (sptr)
		{
			sptr = get_user(sptr);

			if (sptr)
				proc_user(sptr);

			sptr = fgets(buffer,200,fin);
		}
		fclose(fin);
	}

	Dsp_srch();
	Dsp_close();
}

void
proc_user (
 char *	user)
{
	int	fd;
	char	fast_line[80];
	FASTSTRUC	fast_rec;

	fd = open_fast(user,"r");

	while (!(cc = RF_READ(fd, (char *) &fast_rec)))
	{
		sprintf(fast_line," %-8.8s ^E %s ^E %s ^E  %2d  ",
					user, fast_rec._f_code,
					fast_rec._m_file, fast_rec._n_opts);

		if (Dsp_saverec(fast_line))
			break;
	}
	Dsp_saverec("^^GGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGG");
	close_fast(fd);
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		rv_pr(ML(mlMenuMess021) ,26,0,1);

		move(0,1);
		line(79);

		box(0,3,80,1);

		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

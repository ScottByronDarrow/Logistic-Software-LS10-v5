/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sub_maint.c    )                                 |
|  Program Desc  : ( Maintain Sub Menus for Programs              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  SUB_MENU/SUB_MENU                                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  SUB_MENU/SUB_MENU                                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 05/09/88         |
|---------------------------------------------------------------------|
|  Date Modified : (05/09/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (19/04/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (05/09/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments    : (05/10/90)   - General Update for New Scrgen. S.B.D. |
|              : (19/04/92)   - Update dsp to Dsp.                    |
|              : (05/09/97)   - Modified for Multilingual Conversion. |
|              : (03/09/1999) - Ported to ANSI standards.             |
|                :                                                    |
|                :                                                    |
| $Log: sub_maint.c,v $
| Revision 5.2  2001/08/09 05:13:56  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:49  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:38  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:47:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 09:42:03  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/11/11 01:48:28  cam
| Enhanced sub_maint to allow for an optional third parameter, being the name of the configuration file to modify.  By default (and if no third parameter is passed) the configuration file is SUB_MENU.
|
| Revision 1.5  1999/09/17 07:27:19  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:43  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:36:57  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sub_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/sub_maint/sub_maint.c,v 5.2 2001/08/09 05:13:56 scott Exp $";

#include 	<pslscr.h>
#include	<sub_menu.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#define	LIN_TYPE	0
#define	TAB_TYPE	1
#define	DIS_TYPE	2

#define	DFLT_FILE	"SUB_MENU"

int		prog_type;
char	buffer [300];
char	useConfigFile [128];
int		OpenSub (char *);

FILE *	fsort;

struct	{
	char	dummy [11];
	char	prog_prompt [15];
	char	menu_prompt [15];
	char	prog_name [15];
	char	menu_name [15];
} local_rec;

static	struct	var	vars []	= {	

	{1, LIN, "prog_name", 4, 15, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", " ", local_rec.prog_prompt, " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.prog_name}, 
	{1, LIN, "menu_name", 5, 15, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", " ", local_rec.menu_prompt, " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.menu_name}, 
	{0, LIN, "dummy", 4, 20, CHARTYPE, 
		"A", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.dummy}, 

};

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
int		spec_valid		(int);
void	show_file		(char *);
int		delete_line		(void);
int		find_prog		(void);
void	process			(void);
void	load			(void);
void	update			(void);
void	copy_old		(void);
int		heading			(int);


int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	if (argc != 2 &&
		argc != 3)
	{
		print_at (0, 0, mlMenuMess111, argv[0]);
		return (EXIT_FAILURE);
	}

	switch (argv [1][0])
	{
	/*---------------
	| Linear Screen	|
	---------------*/
	case	'L':
	case	'l':
		search_ok = 1;
		strcpy (local_rec.prog_prompt,"Program Name");
		strcpy (local_rec.menu_prompt,"Menu Name");

		vars[0].stype = LIN;
		vars[0].row = 4;
		vars[0].col = 16;

		vars[1].stype = LIN;
		vars[1].row = 5;
		vars[1].col = 16;

		prog_type = LIN_TYPE;
		break;

	/*-----------------------
	| Tabular Screen	|
	-----------------------*/
	case	'T':
	case	't':
		strcpy (local_rec.prog_prompt," Program Name ");
		strcpy (local_rec.menu_prompt,"  Menu Name   ");

		vars[0].stype = TAB;
		vars[0].row = MAXLINES;
		vars[0].col = 0;

		vars[1].stype = TAB;
		vars[1].row = 0;
		vars[1].col = 0;

		tab_col = 23;
		prog_type = TAB_TYPE;
		break;

	/*-----------------------
	| Display Screen	|
	-----------------------*/
	case	'D':
	case	'd':
		prog_type = DIS_TYPE;
		break;

	default:
		print_at (0, 0, ML (mlMenuMess142));
		return (EXIT_FAILURE);
	}

	if (argc == 3)
		strcpy (useConfigFile, argv [2]);
	else
		strcpy (useConfigFile, DFLT_FILE);

	init_scr ();
	set_tty (); 

	if (prog_type == DIS_TYPE)
	{
		process ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	set_masks ();
	/*-------------------------------
	| Take A copy of the "old" file	|
	-------------------------------*/
	copy_old ();

	/*-----------------------
	| Load Tabular Screen	|
	-----------------------*/
	if (vars[0].stype == TAB_TYPE)
		load ();

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		init_vars(1);	

		/*-----------------------------------------------
		| Only perform entry for linear or new file	|
		-----------------------------------------------*/
		if (vars[0].stype == LIN_TYPE || lcount[1] == 0)
		{
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;
		}

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			break;

		update ();
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	char *	sptr;

	if (strcmp(FIELD.label,"prog_name") == 0)
	{
		if (vars[0].stype == LIN && last_char == SEARCH)
		{
			show_file (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (delete_line ());

		sptr = local_rec.prog_name;

		while (*sptr)
		{
			*sptr = tolower(*sptr);
			sptr++;
		}

		display_field(field);

		if (vars[0].stype == LIN_TYPE)
		{
			if (find_prog ())
			{
				strcpy(local_rec.menu_name,sub_rec._menu_name);
				entry_exit = 1;
				if (prog_status == ENTRY)
					display_field(label("menu_name"));
			}
		}
		return (EXIT_SUCCESS);
	}

	if (strcmp(FIELD.label,"menu_name") == 0)
	{
		if (dflt_used)
			return (delete_line ());

		sptr = local_rec.menu_name;

		while (*sptr)
		{
			*sptr = tolower(*sptr);
			sptr++;
		}

		display_field(field);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=======================================
| Display Search of Program Name		|
| - sort Alphabetically first			|
=======================================*/
void
show_file (
 char *	key_val)
{
	int	fd = OpenSub ("r");
	char *	sptr;
	char	szTemp [256];

	fsort = sort_open("SEARCH");

	cc = RF_READ(fd, (char *) &sub_rec);

	while (!cc)
	{
		if (!strncmp(sub_rec._prog_name,key_val,strlen(key_val)))
		{
			sprintf (szTemp,"%s %s\n", sub_rec._prog_name, sub_rec._menu_name);
			sort_save (fsort, szTemp);
		}	
		cc = RF_READ(fd, (char *) &sub_rec);
	}

	RF_CLOSE(fd);

	fsort = sort_sort(fsort,"SEARCH");

	work_open();
	save_rec("#Program Name","#Menu Name");

	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		*(sptr + 14) = '\0';
		cc = save_rec(sptr,sptr + 15);
		if (cc)
			break;
		sptr = sort_read(fsort);
	}
	cc = disp_srch();
	work_close();
	sort_delete(fsort,"SEARCH");
}

/*=======================
| Delete Tabular Line	|
=======================*/
int
delete_line (
 void)
{
	int	i;
	int	this_page;

	if (vars[0].stype == LIN_TYPE)
	{
		/*print_mess(" Cannot Delete Lines On Linear Screen ");*/
		print_mess(ML(mlStdMess005));
		return (EXIT_FAILURE);
	}

	if (prog_status == ENTRY)
	{
		/*print_mess(" Cannot Delete Lines On Entry ");*/
		print_mess(ML(mlStdMess005));
		return (EXIT_FAILURE);
	}

	if (lcount[1] < 1)
	{
		/*print_mess(" No Lines to Delete ");*/
		print_mess(ML(mlStdMess032));
		return (EXIT_FAILURE);
	}

	lcount[1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt + 1);
		putval(line_cnt);

		if (line_cnt / TABLINES == this_page)
			line_display();
	}

	sprintf(local_rec.prog_name,"%-14.14s"," ");
	sprintf(local_rec.menu_name,"%-14.14s"," ");

	putval(line_cnt);

	if (line_cnt / TABLINES == this_page)
		line_display();

	line_cnt = i;

	getval(line_cnt);
	return (EXIT_SUCCESS);
}

/*=======================================
| lookup program_name for linear screen	|
=======================================*/
int
find_prog (
 void)
{
	int	fd = OpenSub ("r");
	cc = RF_READ (fd, (char *) &sub_rec);

	while (!cc && strcmp(local_rec.prog_name,sub_rec._prog_name))
		cc = RF_READ (fd, (char *) &sub_rec);

	RF_CLOSE(fd);

	if (!strcmp (local_rec.prog_name,sub_rec._prog_name))
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

/*===============
| Display File	|
===============*/
void
process (
 void)
{
	int	fd = OpenSub("r");

	Dsp_open(0,0,15);
	Dsp_saverec("  Program Name  |   Menu Name    ");
	Dsp_saverec("");
	Dsp_saverec(" [FN03]  [FN14]  [FN15]  [FN16]  ");

	cc = RF_READ(fd, (char *) &sub_rec);

	while (!cc)
	{
		sprintf(buffer," %s ^E %s ",sub_rec._prog_name,sub_rec._menu_name);
		cc = Dsp_saverec(buffer);
		if (cc)
			break;

		cc = RF_READ(fd, (char *) &sub_rec);
	}

	RF_CLOSE(fd);
	Dsp_srch();
	Dsp_close();
}

/*=======================
| Load Tabular Screen	|
| - sort first		|
=======================*/
void
load (
 void)
{
	int	fd = OpenSub("r");
	char *	sptr;
	char	szTemp [256];

	fsort = sort_open("LOAD");

	cc = RF_READ(fd, (char *) &sub_rec);

	while (!cc)
	{
		sprintf (szTemp,"%s %s\n",sub_rec._prog_name,sub_rec._menu_name);
		sort_save (fsort, szTemp);
		cc = RF_READ(fd, (char *) &sub_rec);
	}

	RF_CLOSE(fd);

	fsort = sort_sort(fsort,"LOAD");

	scn_set(1);

	lcount[1] = 0;

	sptr = sort_read(fsort);

	while (sptr != (char *)0 && lcount[1] < MAXLINES)
	{
		sprintf(local_rec.prog_name,"%-14.14s",sptr);
		sprintf(local_rec.menu_name,"%-14.14s",sptr + 15);
		putval(lcount[1]++);
		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"LOAD");
}

void
update (
 void)
{
	int	fd;

	/*---------------
	| Update Linear	|
	---------------*/
	if (vars[0].stype == LIN)
	{
		fd = OpenSub("u");

		cc = RF_READ(fd, (char *) &sub_rec);

		/*-----------------------
		| Look for program name	|
		-----------------------*/
		while (!cc && strcmp(local_rec.prog_name,sub_rec._prog_name))
			cc = RF_READ(fd, (char *) &sub_rec);

		/*---------------
		| Updating	|
		---------------*/
		if (!strcmp(local_rec.prog_name,sub_rec._prog_name))
		{
			strcpy(sub_rec._prog_name,local_rec.prog_name);
			strcpy(sub_rec._menu_name,local_rec.menu_name);
			cc = RF_UPDATE(fd, (char *) &sub_rec);
		}
		else
		{
			strcpy(sub_rec._prog_name,local_rec.prog_name);
			strcpy(sub_rec._menu_name,local_rec.menu_name);
			cc = RF_ADD(fd, (char *) &sub_rec);
		}
		
		RF_CLOSE(fd);
	}
	else
	{
		if (vars[0].stype == TAB_TYPE)
			prog_exit = 1;

		fd = OpenSub("w");

		/*-----------------------
		| Write new file	|
		-----------------------*/
		for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
		{
			getval(line_cnt);
			add_sub(fd,local_rec.prog_name,local_rec.menu_name);
		}

		RF_CLOSE(fd);
	}
}

/*=======================
| Take a Copy of File	|
=======================*/
void
copy_old (
 void)
{
	char *	sptr = getenv("PROG_PATH");
	char	filename[2][100];

	sprintf (filename [0],
			 "%s/BIN/SUB_MENU/%s",
			 (sptr != (char *)0) ? sptr : "/usr/LS10.5",
			 useConfigFile);

	sprintf (filename [1],
			 "%s/BIN/SUB_MENU/%s.o",
			 (sptr != (char *)0) ? sptr : "/usr/LS10.5",
			 useConfigFile);

	if (access(filename[0],00) == -1)
		return;

	if (fork() == 0)
	{
		execlp("cp",
			"cp",
			filename[0],
			filename[1],(char *)0);
	}
	else
		wait((int *)0);
}

/*========================================
| OpenSub () 'overides' the open_sub ()  |
| in sub_menu.h, so that we can open the |
| filename passed on the command line.   |
========================================*/
int
OpenSub (
 char *	mode)
{
	int		errc, fd;
	char *	sptr = getenv ("PROG_PATH");
	char	filename [100];

	sprintf (filename,
			 "%s/BIN/SUB_MENU/%s",
			 (sptr != (char *)0) ? sptr : "/usr/LS10.5",
			 useConfigFile);

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access(filename,00) < 0)
	{
		if ((errc = RF_OPEN (filename, sizeof (struct sub_type), "w", &fd)))
			sys_err ("Error in SUB_MENU during (WKCREAT)", errc, PNAME);

		if ((errc = RF_CLOSE (fd)))
			sys_err ("Error in SUB_MENU during (WKCLOSE)", errc, PNAME);
	}

	if ((errc = RF_OPEN (filename, sizeof (struct sub_type), mode, &fd)))
		sys_err ("Error in SUB_MENU during (WKOPEN)", errc, PNAME);

	return (fd);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		rv_pr(ML(mlMenuMess110),27,0,1);

		move(0,1);
		line(79);

		if (vars[0].stype == LIN)
			box(0,3,80,2);

		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

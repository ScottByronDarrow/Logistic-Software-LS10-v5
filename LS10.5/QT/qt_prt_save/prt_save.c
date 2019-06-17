/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( qt_prt_save.c)                                   |
|  Program Desc  : ( Quotation Save File Print / Display.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Elena B. Cuaresma | Date Written  : 23/03/1999     |
|---------------------------------------------------------------------|
|  Date Modified : (09/09/1997)      | Modified  by  : Marnie Organo  |
|  Date Modified : (09/09/1997)      | Modified  by  : Alvin Misalucha|
|                                                                     |
|  Comments      : (09/09/1997) - Updated for Multilingual Conversion |
|                : (23/08/1999)      | Ported to ANSI convention.     |
|                                                                     |
| $Log: prt_save.c,v $
| Revision 5.2  2001/08/09 08:44:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:20  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:51  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:06  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:00  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/11/16 03:29:20  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.9  1999/10/20 02:07:01  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/09/22 05:18:28  scott
| Updated from Ansi project.
|
| Revision 1.7  1999/09/14 04:05:38  scott
| Updated for Ansi.
|
| Revision 1.6  1999/06/18 06:12:27  scott
| Updated to add log for cvs and remove old style read_comm()
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: prt_save.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_prt_save/prt_save.c,v 5.2 2001/08/09 08:44:45 scott Exp $";

#define	NO_SCRGEN
#include	<std_decs.h>
#include	<pslscr.h>		
#include	<get_lpno.h>		
#include	<minimenu.h>		
#include	<sys/types.h>
#include 	<dirent.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include 	<pwd.h>
#include 	<ml_std_mess.h>
#include 	<ml_qt_mess.h>

#define	FNAME	0
#define	UNAME	1
#define	DT_TM	2

FILE	*fopen(const char *, const char *);
FILE	*popen(const char *, const char *);
FILE	*fin;
FILE	*fout;

char	prog_path[101];
char	directory[101];
char	filename[101];
char	prntype[101];
char	queue_name[15];

int		lpno;
int		save_files;
int		file_printed =  FALSE;
char	str1[150];

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy[11];
	char	text[131];
} local_rec;

struct SAVE_STR
{
	struct	SAVE_STR *next;
	struct	SAVE_STR *prev;
	char	save_name[39];
	char	date_sort[24];
};

struct	passwd	*pwd,
		*getpwuid(uid_t);

struct	tm	*tme,
		*localtime(const time_t *);

struct	stat	file_info;

#define	SAVE_NULL	( (struct SAVE_STR *) NULL )
struct	SAVE_STR	*save_head = SAVE_NULL;
struct	SAVE_STR	*save_curr = SAVE_NULL;
struct	SAVE_STR	*save_tail = SAVE_NULL;
struct	SAVE_STR	*save_str = SAVE_NULL;

MENUTAB prt_menu [] = 
{
	{ " 1. PRINT   ", "Print Document And Exit " },
	{ " 2. EXIT    ", "Exit Without Printing Document " },
	{ ENDMENU }
};


/*=========================== 
| Function prototypes.      |
===========================*/
int		main			(int argc, char * argv []);
int		heading			(void);
int		shutdown_prog 	(void);
int		prt_save		(void);
int		get_lp_name 	(int);
int		read_save		(char *);
int		print_save		(char *);
int		load_saves		(void);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		vld_lp;
	char	select_file[15];
	char	*sptr = getenv("PROG_PATH");

	if (sptr)
	{
		sprintf(prog_path, "%-100.100s", sptr);
		clip(prog_path);
	}
	else
		strcpy(prog_path, "/usr/LS10.5");

	sptr = strchr(argv[0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv[0];

	if (argc != 4)
	{
		print_at(1,0, ML(mlQtMess700), argv[0]);
		return (argc);
	}

	sprintf(directory, "%s", argv[1]);
	clip(directory);
	if (access(directory, 00))
	{
		print_at(1,0,ML(mlStdMess132));
		return (EXIT_SUCCESS);
	}


	lpno = atoi(argv[3]);
	if (!valid_lp(lpno))
	{
		print_at(2,0,ML(mlStdMess020));
		return (EXIT_SUCCESS);
	}

	sprintf(prntype, "%s/BIN/MENUSYS/prntype", prog_path);
	vld_lp = get_lp_name(lpno);
	if (!vld_lp)
	{
		print_at(2,0,ML(mlStdMess020));
		return (EXIT_SUCCESS);
	} 

	save_files = FALSE;

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();          /*  get into raw mode			*/

	/*--------------------------------------
	| Load list of save files in directory |
	--------------------------------------*/
	print_mess(ML(mlStdMess035));
	sleep(2);
	clear_mess();

	load_saves();

	while (!restart && !prog_exit)
	{
		heading();
		move(0, 21);
		line(132); 

		sprintf(select_file, "%-14.14s", argv[2]);
		read_save(select_file);

		Dsp_srch();

		if ( prt_save() )
		{
			print_save(select_file);
			file_printed = TRUE;
		}
		else
			restart = 1;
	}  

	if (shutdown_prog () != 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
int
shutdown_prog (void)
{
	Dsp_close();
	FinishProgram ();
	if (file_printed)
		return (EXIT_SUCCESS);
	else
		return (EXIT_FAILURE);
}

/*--------------------------
| Re-print save file Y/N ? |
--------------------------*/
int
prt_save (void)
{
	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 8;
	MENU_COL = 55;

	mmenu_print(" PRINT MENU ", prt_menu, 0);
	switch ( mmenu_select(prt_menu) )
	{
	case 0:
	case 99:
	default:
		return(TRUE);

	case 1:
	case -1:
		return(FALSE);

	}
}

/*===============================
| Loads queue name from prntype |
===============================*/
int
get_lp_name (
 int	printer_no)
{
	char	*sptr;
	char	*tptr;
	char	data[100];
	int	lp_cntr;
	int	valid;

	if ((fin = fopen(prntype, "r")) == 0)
	{
		sprintf(err_str, "Error in %s during (FOPEN)",prntype);
		sys_err(err_str, errno, PNAME);
	}

	valid = FALSE;
	lp_cntr = 0;
	/*---------------------------------
	| Load printer name based on lpno | 
	---------------------------------*/
	sptr = fgets(data, 80, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		lp_cntr++;

		/*------------------------------
		| Look for end of printer name |
		------------------------------*/
		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		if (*tptr)
		{
			sptr = tptr + 1;
			tptr = sptr;
			while (*tptr && *tptr != '\t')
				tptr++;

			*tptr = '\0';

			sprintf(queue_name, "%-14.14s", sptr);
			clip(queue_name);

			if (lp_cntr == lpno)
			{
				valid = TRUE;
				break;
			}
		}

		sptr = fgets(data, 80, fin);
	}

	fclose(fin);

	return(valid);
}

/*----------------------------------------
| Read selected filename into Dsp Window |
----------------------------------------*/
int
read_save (
 char *	save_name)
{
	char *	sptr;
	char *	tptr;
	char	data_str[257];

	sprintf(filename, "%s/%s", directory, save_name);
	clip(filename);

	if ((fin = fopen(filename,"r")) == 0)
	{
		sprintf(err_str, "Error in %s during (FOPEN)",filename);
		sys_err(err_str, errno, PNAME);
	}

	Dsp_open(0, 3, 13);

	sprintf (str1, "%s", ML ("VIEW SAVE FILE")); 
	clip(str1);

	sprintf(err_str, "%-44.44s %s - %-14.14s %-48.48s",
		" ",
		str1,
		save_name,
		" ");

	Dsp_saverec(err_str);
	Dsp_saverec("");
	Dsp_saverec(" [FN03]  [FN14]  [FN15]  [FN16] ");

	sptr = fgets(data_str, 256, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		sprintf(local_rec.text, "%-130.130s", sptr); 

		/*------------------------
		| Replace occurrences of |
		| ESC with a space       |
		------------------------*/
		tptr = strchr(local_rec.text, '\033');
		while (tptr)
		{
			*tptr = ' ';
			tptr = strchr(local_rec.text, '\033');
		}

		Dsp_saverec(local_rec.text);
		
		sptr = fgets(data_str, 256, fin);
	} 

	fclose(fin);

	return(0);
}

/*------------------------------------------------
| Print selected filename to its correct printer |
------------------------------------------------*/
int
print_save (
 char *	save_name)
{
	char	cmd[200];

	sprintf(cmd, "psl_reprint %s %s/%s", queue_name, directory, save_name);
	system(cmd);

	return(0);
}

/*===============================================
| Read all Files in Directory and then display. |
===============================================*/
int
load_saves (void)
{
	DIR	*opendir(const char *),
		*fd;
	struct	dirent
		*readdir(DIR *),
		*dirbuf;
	char	tmp_fname[100];
	char	own_name[9];
	char	s_date[11];
	char	f_date[11];
	char	f_time[6];

	if ((fd = opendir (directory)) == (DIR *) 0)
		return (EXIT_FAILURE);

	while ((dirbuf = readdir (fd)) != (struct dirent *) 0)
	{
		if (strcmp (dirbuf->d_name, ".") == 0 ||
		    strcmp (dirbuf->d_name, "..") == 0)
			continue; 

		/*----------------------
		| Store in linked list |
		----------------------*/
		 save_curr = (struct SAVE_STR *)malloc (sizeof(struct SAVE_STR));
		if (save_curr == SAVE_NULL)
		{
			sys_err ("Error in load_saves() During (MALLOC)", 
				errno, 
				PNAME);
		}

		sprintf (tmp_fname, "%s/%s", directory, dirbuf->d_name);
		clip (tmp_fname);
		if ( !stat(tmp_fname, &file_info) )
		{
			if ((pwd = getpwuid (file_info.st_uid)) == 0)
				strcpy(own_name, "????????");
			else
				sprintf(own_name, "%-8.8s", pwd->pw_name);

			tme = localtime (&file_info.st_mtime);

			sprintf(s_date, "%02d/%02d/%04d",
					tme->tm_year, 
					tme->tm_mon + 1,
					tme->tm_mday);
			sprintf(f_date, "%02d/%02d/%04d",
					tme->tm_mday,
					tme->tm_mon + 1,
					tme->tm_year);
			
			sprintf(f_time, "%02d:%02d",
				tme->tm_hour,
				tme->tm_min);

			sprintf (save_curr->save_name, 
				"%-14.14s %-8.8s %-8.8s %-5.5s", 
				dirbuf->d_name,
				own_name,
				f_date,
				f_time);

			sprintf (save_curr->date_sort, 
					"%-10.10s %-5.5s %-8.8s", 
					s_date,
					f_time,
					own_name);
		}
		else
		{
			sprintf (save_curr->save_name, 
				"%-14.14s %-8.8s %-8.8s %-5.5s", 
				dirbuf->d_name, " ", " ", " ");
		}

		save_curr->prev = save_tail;
		save_curr->next = SAVE_NULL;
		if (save_tail != SAVE_NULL)
			save_tail->next = save_curr;
		else
			save_head = save_curr;
		save_tail = save_curr;
	} 

	closedir (fd);

	return (EXIT_SUCCESS);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (void)
{
	swide();
	clear();

	move(0,1);
	line(132);

	rv_pr(ML(mlQtMess003), 35, 0, 1);

	move(0,21);
	line(132);

	rv_pr(ML(mlQtMess004), 50, 21, 1);

	clip(ML(mlQtMess005));
	rv_pr (ML(mlQtMess005), (132 - strlen(ML(mlQtMess005))) / 2, 22, 0);

	return(0);
}

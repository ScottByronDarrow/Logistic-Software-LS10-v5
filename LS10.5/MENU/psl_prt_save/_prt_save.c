/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_prt_save.c)                                  |
|  Program Desc  : ( Pinnacle Save File Print / Display.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 31/10/91         |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (04/11/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (22/06/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (15/09/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (23/09/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (12/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (01/11/91) - Added option to reorder list of files.|
|                : Remove any ESCs from Dsp lines.                    |
|                : Added user name and date/time after file name.     |
|                :                                                    |
|  (04/11/91)    : When sorting by Date/Time most recent appears at   |
|                : top of list.                                       |
|                :                                                    |
|  (22/06/92)    : Made general purpose reprint option which is a     |
|                : link to psl_prt_save called psl_prt_file.          |
|                :                                                    |
|  (15/09/92)    : Implement delete option for save files. SC 7639 DPL|
|                :                                                    |
|  (23/09/92)    : Modify delete routine such that the order of the   |
|                : entries within the list is maintained. SC 7639 DPL |
|                :                                                    |
|  (12/09/97)    : Incorporate multilingual conversion.               |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: _prt_save.c,v $
| Revision 5.2  2001/08/09 05:13:49  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:12  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:31  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:47:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/25 10:24:00  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.9  1999/11/16 09:42:02  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/09/17 07:27:13  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/06/15 02:36:55  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _prt_save.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_prt_save/_prt_save.c,v 5.2 2001/08/09 05:13:49 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>		
#include	<get_lpno.h>		
#include	<minimenu.h>		
#include	<sys/types.h>
#include 	<dirent.h>
#include 	<sys/stat.h>
#include 	<time.h>
#include 	<pwd.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>

#define	FNAME	0
#define	UNAME	1
#define	DT_TM	2

#define	MAX_FILENAME_LEN	256

FILE *	fin;
FILE *	fout;

char	prog_path [256];
char	directory [256];
char	filename [256];
char	prntype [101];
char	queue_name [15];

int	lpno;
int	save_files;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	text [131];
} local_rec;

struct SAVE_STR
{
	struct	SAVE_STR *next;
	struct	SAVE_STR *prev;
	char	save_name [MAX_FILENAME_LEN + 1];
	char	date_sort [MAX_FILENAME_LEN + 1];
	char	actFileName [MAX_FILENAME_LEN + 1];
};

struct	passwd	*pwd;

struct	tm	*tme;

struct	stat	file_info;

#define	SAVE_NULL	( (struct SAVE_STR *) NULL )
struct	SAVE_STR	*save_head = SAVE_NULL;
struct	SAVE_STR	*save_curr = SAVE_NULL;
struct	SAVE_STR	*save_tail = SAVE_NULL;
struct	SAVE_STR	*save_str = SAVE_NULL;

MENUTAB order_menu [] = 
{
	{ " 1. FILENAME   ", " Order By Filename " },
	{ " 2. USER NAME  ", " Order By User Name " },
	{ " 3. DATE/TIME  ", " Order By Date / Time " },
	{ " 4. NO CHANGE  ", " No Change To Order " },
	{ ENDMENU }
};

MENUTAB prt_menu [] = 
{
	{ " 1. PRINT        ", " Print Document And Exit " },
	{ " 2. ABORT        ", " Exit Without Printing Document " },
	{ " 3. DELETE       ", " Delete Document " },
	{ ENDMENU }
};
int		maxLoadedFileName	=	0;
int		maxSavedFileName	=	0;

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
int		chk_re_sort		(void);
int		re_sort			(int);
int		prt_save		(void);
int		delete_file		(void);
int		get_lp_name		(int);
int		read_save		(char *);
int		print_save		(char *);
int		load_saves		(void);
int		heading			(void);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		vld_lp;
	char	tmp_prt [MAX_FILENAME_LEN + 1];
	char *	sptr = getenv ("PROG_PATH");

	if (sptr)
	{
		sprintf (prog_path, "%-100.100s", sptr);
		clip (prog_path);
	}
	else
		strcpy (prog_path, "/usr/LS10.5");

	sptr = strchr (argv[0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp(sptr, "psl_prt_save"))
	{
		if (argc != 2)
		{
			print_at (0, 0, mlStdMess036, argv[0]);
			return (EXIT_FAILURE);
		}

		lpno = atoi (argv [1]);
		sprintf (prntype, "%s/BIN/MENUSYS/prntype", prog_path);
	
		/*------------------------------------
		| Check that printer number is valid |
		------------------------------------*/
		vld_lp = get_lp_name (lpno);
		if (!vld_lp)
		{
			print_at (0, 0, ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
	
		sprintf(tmp_prt, "%-14.14s", queue_name);
		clip(tmp_prt);
		upshift( tmp_prt );
		sprintf(directory, "%s/%s", prog_path, tmp_prt);
		clip(directory);
		
		save_files = TRUE;
	}
	else
	{
		if (argc != 3)
		{
			print_at (1, 0, mlMenuMess710, argv[0]);
			return (EXIT_FAILURE);
		}

		sprintf (directory, "%s", argv [1]);
		clip (directory);
		if (access (directory, 00))
		{
			print_at(0,0,ML(mlStdMess132));
			return (EXIT_FAILURE);
		}

		lpno = atoi (argv[2]);
		if (!valid_lp (lpno))
		{
			print_at(0,0,ML(mlStdMess020));
			return (EXIT_FAILURE);
		}

		sprintf(prntype, "%s/BIN/MENUSYS/prntype", prog_path);
		vld_lp = get_lp_name(lpno);
		if (!vld_lp)
		{
			print_at(0,0,ML(mlStdMess020));
			return (EXIT_FAILURE);
		}

		save_files = FALSE;
	}

	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();			/*  get into raw mode		*/

	/*--------------------------------------
	| Load list of save files in directory |
	--------------------------------------*/
	load_saves ();
	if (save_head == SAVE_NULL)
	{
		swide ();
		clear ();
		box (0, 0, 132, 20);
		rv_pr (ML (mlStdMess145), (132 - strlen(ML(mlStdMess145))) / 2, 10, 1);
		crsr_off ();
		sleep (sleepTime);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	while (TRUE)
	{
		heading ();
		save_str = (struct SAVE_STR *) win_select
		(
			" F I L E    S E L E C T I O N ", 
			maxSavedFileName,
			(struct SEL_STR *) save_head,
			0,
			3,
			132,
			16
		);

		if ( save_str == SAVE_NULL )
		{ 
			if ( last_char == FN4 )
			{
				chk_re_sort ();
				continue;
			}
			else
				break;
		}

		move (0, 21);
		line (132);

		read_save (save_str->actFileName);

		Dsp_srch();

		if ( prt_save() )
			print_save (save_str->actFileName);
	} 

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	clear ();
	crsr_on ();
	snorm ();
	rset_tty ();
}

/*--------------------------------
| Get re-sort criteria from user |
--------------------------------*/
int
chk_re_sort (
 void)
{
	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 7;
	MENU_COL = 58;

	mmenu_print (" ORDER BY      ", order_menu, 0);
	switch (mmenu_select(order_menu))
	{
	case FNAME:
		re_sort (FNAME);
		return (TRUE);

	case UNAME:
		re_sort (UNAME);
		return (TRUE);

	case DT_TM:
		re_sort (DT_TM);
		return (TRUE);

	case -1:
	case  4:
	case 99:
	default:
		return (FALSE);
	}
}

/*-----------------------------
| Re-sort into required order |
-----------------------------*/
int
re_sort (
 int sort_by)
{
	struct	SAVE_STR *tmp_head = SAVE_NULL;
	struct	SAVE_STR *tmp_new = SAVE_NULL;
	struct	SAVE_STR *tmp_curr = SAVE_NULL;
	struct	SAVE_STR *tmp_tail = SAVE_NULL;
	int		pos_found;
	int		sort_len;
	char	sort [2][MAX_FILENAME_LEN + 1];

	/*----------------------
	| Process current list |
	----------------------*/
	save_curr = save_head;
	while (save_curr != SAVE_NULL)
	{
		pos_found = FALSE;
		tmp_new = (struct SAVE_STR *)malloc (sizeof(struct SAVE_STR));
		strcpy(tmp_new->save_name,   save_curr->save_name);
		strcpy(tmp_new->date_sort,   save_curr->date_sort);
		strcpy(tmp_new->actFileName, save_curr->actFileName);

		/*-------------------------
		| Find position to insert |
		-------------------------*/
		tmp_curr = tmp_head;
		while ( tmp_curr != SAVE_NULL )
		{
			switch(sort_by)
			{
			case FNAME:
			       sprintf(sort[0], "%-*.*s", 
											maxLoadedFileName,
											maxLoadedFileName,
											save_curr->save_name);
			       sprintf(sort[1], "%-*.*s", 
											maxLoadedFileName,
											maxLoadedFileName,
											tmp_curr->save_name);
			       sort_len = maxLoadedFileName;

			       break;

			case UNAME:
				sprintf(sort[0], "%-8.8s %-15.15s",
					save_curr->save_name + maxLoadedFileName,
					save_curr->date_sort);

				sprintf(sort[1], "%-8.8s %-15.15s",
					tmp_curr->save_name + maxLoadedFileName,
					tmp_curr->date_sort);

			   sort_len = 24;

				break;

			case DT_TM:
				sprintf(sort[1], "%-24.24s", save_curr->date_sort);
				sprintf(sort[0], "%-24.24s", tmp_curr->date_sort);
	 			sort_len = 24;

				break;

			default:
				strcpy(sort[0], " ");
				strcpy(sort[1], " ");
				sort_len = 1;
				break;
			}

			if (strncmp(sort[0], sort[1], sort_len) < 0)
			{
				pos_found = TRUE;
				break;
			}

			tmp_curr = tmp_curr->next;
		}

		if (!pos_found)
		{
			/*------------------------------------
			| No position found so append to end |
			------------------------------------*/
			if (tmp_curr == tmp_head)
			{
				/*----------------
				| Insert at head |
				----------------*/
				tmp_head = tmp_new;
				tmp_new->prev = SAVE_NULL;
				tmp_new->next = SAVE_NULL;
				tmp_tail = tmp_head;
			}
			else
			{
				/*----------------
				| Append to tail |
				----------------*/
				tmp_tail->next = tmp_new;
				tmp_new->prev = tmp_tail;
				tmp_new->next = SAVE_NULL;
				tmp_tail = tmp_new;
			}
		}
		else
		{
			/*--------------------------
			| Position found so insert |
			--------------------------*/
			if (tmp_curr == tmp_head)
			{
				/*--------------------
				| Insert before head |
				--------------------*/
				tmp_new->next = tmp_head;
				tmp_curr->prev = tmp_new;
				tmp_new->prev = SAVE_NULL;
				tmp_head = tmp_new;

			}
			else
			{
				/*------------------
				| Insert in middle |
				------------------*/
				tmp_curr->prev->next = tmp_new;
				tmp_new->prev = tmp_curr->prev;
				tmp_new->next = tmp_curr;
				tmp_curr->prev = tmp_new;
			}
		}

		save_curr = save_curr->next;
	}

	/*---------------
	| Free old list |
	---------------*/
	save_curr = save_head;
	save_head = tmp_head;

	while (save_curr != SAVE_NULL)
	{
		tmp_curr = save_curr->next;
		free(save_curr);
		save_curr = tmp_curr;
	}

	return (EXIT_SUCCESS);
}

/*--------------------------
| Re-print save file Y/N ? |
--------------------------*/
int
prt_save (
 void)
{
	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 8;
	MENU_COL = 55;

	mmenu_print(" PRINT SAVE FILE ", prt_menu, 0);
	switch ( mmenu_select(prt_menu) )
	{
	case 0:
	case 99:
	default:
		return(TRUE);

	case 1:
	case -1:
		return(FALSE);

	case 2:
		delete_file ();
		return(FALSE);
	}
}

/*----------------------------------
| Delete save file and reload list |
| of valid save files.             |
----------------------------------*/
int
delete_file (
 void)
{
	int		i;
	char	cmd [100];
	struct	SAVE_STR *	tmp_ptr = SAVE_NULL;

	/* Are you sure you want to delete this file ? */
	i = prmptmsg(ML(mlMenuMess195), "YyNn", 2, 2);
	if (i == 'n' || i == 'N')
		return (FALSE);

	/*--------------
	| Delete File. |
	--------------*/
	sprintf(cmd, "rm %s/%-s", directory, save_str->actFileName);
	sys_exec(cmd);

	/*---------------------------------------
	| Remove selected node from linked list |
	---------------------------------------*/
	tmp_ptr = save_str;

	if (save_str->next != SAVE_NULL)
		save_str->next->prev = save_str->prev;
	else
		save_tail = save_str->prev;

	if (save_str->prev != SAVE_NULL)
		save_str->prev->next = save_str->next;
	else
		save_head = save_str->next;

	free(tmp_ptr);

	/*-----------------
	| Reset save_curr |
	-----------------*/
	save_curr = save_head;
	if (save_head == SAVE_NULL)
	{
		clear();
		box (0, 0, 132, 20);
		rv_pr(ML(mlStdMess145), (132 - strlen(ML(mlStdMess145))) / 2, 10, 1);
		crsr_off();
		sleep(3);
	}

	return (TRUE);
}

/*===============================
| Loads queue name from prntype |
===============================*/
int
get_lp_name (
 int printer_no)
{
	char *	sptr;
	char *	tptr;
	char	data [100];
	int		lp_cntr;
	int		valid;

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

	fclose (fin);

	return (valid);
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
	char	data_str [257];
	int		HeadLen = (100 - maxLoadedFileName) / 2;

	sprintf(filename, "%s/%s", directory, save_name);
	clip(filename);

	if ((fin = fopen(filename,"r")) == 0)
	{
		sprintf(err_str, "Error in %s during (FOPEN)",filename);
		sys_err(err_str, errno, PNAME);
	}

	Dsp_open(0, 3, 13);
	sprintf(err_str,
		".%*.*sV I E W   S A V E   F I L E - %s %*.*s",
		HeadLen,
		HeadLen,
		" ",
		save_name,
		HeadLen,
		HeadLen,
		" ");
	Dsp_saverec(err_str);
	Dsp_saverec("");
	Dsp_saverec(" [REDRAW] [NEXT] [PREV] [EDIT/END] ");

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

	fclose (fin);

	return (EXIT_SUCCESS);
}

/*------------------------------------------------
| Print selected filename to its correct printer |
------------------------------------------------*/
int
print_save (
 char *	save_name)
{
	char cmd [200];

	sprintf (cmd, "psl_reprint %s %s/%s", queue_name, directory, save_name);
	system (cmd);

	return (EXIT_SUCCESS);
}

/*===============================================
| Read all Files in Directory and then display. |
===============================================*/
int
load_saves (
 void)
{
	DIR		*fd;
	struct	dirent
			*dirbuf;
	char	tmp_fname [MAX_FILENAME_LEN + 1];
	char	own_name [9];
	char	s_date [11];
	char	f_date [11];
	char	f_time [6];
	int		tempMaxLoadedFileName	=	0;
	int		tempMaxSavedFileName	=	0;

	maxLoadedFileName	=	0;
	maxSavedFileName	=	0;

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
		if ( stat(tmp_fname, &file_info) == 0)
		{
			tempMaxLoadedFileName 	=	strlen (clip (dirbuf->d_name));
			if (tempMaxLoadedFileName > maxLoadedFileName)
				maxLoadedFileName	=	tempMaxLoadedFileName;

			if ((pwd = getpwuid (file_info.st_uid)) == 0)
				strcpy(own_name, "????????");
			else
				sprintf(own_name, "%-8.8s", pwd->pw_name);

			tme = localtime (&file_info.st_mtime);

			sprintf	(s_date,"%04d/%02d/%02d",
						 tme->tm_year + 1900, tme->tm_mon + 1, tme->tm_mday);

			sprintf	(f_date,"%02d/%02d/%04d",
						 tme->tm_mday, tme->tm_mon + 1, tme->tm_year + 1900);

			sprintf(f_time, "%02d:%02d", tme->tm_hour, tme->tm_min);
	
			sprintf (save_curr->actFileName, 
					 "%-*.*s",
					 maxLoadedFileName,
					 maxLoadedFileName,
					 dirbuf->d_name);

			sprintf (save_curr->save_name, 
					"%-*.*s %-8.8s %-10.10s %-5.5s", 
					maxLoadedFileName,
					maxLoadedFileName,
					dirbuf->d_name,
					own_name,
					f_date,
					f_time);

			sprintf (save_curr->date_sort, 
					"%-10.10s%-5.5s %-8.8s", 
					s_date,
					f_time,
					own_name);
		}
		else
		{
			sprintf (save_curr->actFileName, "%-*.*s",
					 maxLoadedFileName,
					 maxLoadedFileName,
					 " ");

			sprintf (save_curr->save_name, "%-*.*s %-8.8s %-10.10s %-5.5s", 
					maxLoadedFileName,
					maxLoadedFileName,
					dirbuf->d_name, " ", " ", " ");
		}
		tempMaxSavedFileName 	=	strlen (clip (save_curr->save_name));
		if (tempMaxSavedFileName > maxSavedFileName)
			maxSavedFileName	=	tempMaxSavedFileName;

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
heading (
 void)
{
	swide();
	clear();

	move (0, 1);
	line (132);

	rv_pr (ML (mlMenuMess086), 40, 0, 1);

	move (0, 21);
	line (132);

	rv_pr (ML (mlMenuMess077), 50, 21, 1);

	/*Directory : */
	sprintf (err_str, ML(mlStdMess156), directory);
	clip (err_str);
	rv_pr (err_str, (132 - strlen(err_str)) / 2, 22, 0);

	return (EXIT_SUCCESS);
}

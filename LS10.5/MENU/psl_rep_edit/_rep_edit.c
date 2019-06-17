/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( gl_rep_edit.c )                                  |
|  Program Desc  : ( GL report editor and print.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written : (15/10/91)       | Author       : Campbell Mander   |
|---------------------------------------------------------------------|
|  Date Modified : (12/11/91)      | Modified by : Campbell Mander    |
|  Date Modified : (29/04/92)      | Modified by : Campbell Mander    |
|  Date Modified : (17/11/92)      | Modified by : Simon Dubey.       |
|  Date Modified : (25/01/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (23/12/94)      | Modified by : Jonathan Chen      |
|  Date Modified : (05/09/97)      | Modified by : Roanna Marcelino.  |
|  Date Modified : (03/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      : (12/11/91) - Added new options : Add new file /    |
|                : Delete file / Copy File / Move File / Edit File    |
|  (29/04/92)    : Filenames with imbedded spaces are invalid.        |
|  (17/11/92)    : DPL 8036 Problem with IBM port - needs swide ()    |
|                : before init_vars ();                               |
|  (25/01/93)    : Added FN5 print option.                            |
|  (23/12/94)    : PSL 11717 Allows for path relative to $PROG_PATH   |
|  (05/09/97)    : Modified for Multilingual Conversion.              |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: _rep_edit.c,v $
| Revision 5.3  2002/07/17 09:57:26  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 05:13:49  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:42  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:13  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:32  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:47:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/25 10:24:01  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.7  1999/11/16 09:42:02  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/17 07:27:14  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:55  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: _rep_edit.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_rep_edit/_rep_edit.c,v 5.3 2002/07/17 09:57:26 scott Exp $";

#define	TXT_REQD
#include	<pslscr.h>
#include	<get_lpno.h>
#include	<ring_menu.h>
#include	<minimenu.h>
#include	<getnum.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<dirent.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#define	NEW_FILE	1
#define	DEL_FILE	2
#define	COPY_FILE	3
#define	MOVE_FILE	4
#define	EDIT_FILE	5
#define	PRNT_FILE	6
#define	EXIT_PROG	7

FILE *	fin;
FILE *	fout;

char *	directory = NULL;
char	filename [100];

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	filename [15];
	char	text [131];
} local_rec;

struct REP_STR
{
	struct	REP_STR *next;
	struct	REP_STR *prev;
	char	rep_name [26];
};

#define	REP_NULL	((struct REP_STR *) NULL)
struct	REP_STR	*rep_head = REP_NULL;
struct	REP_STR	*rep_curr = REP_NULL;
struct	REP_STR	*rep_tail = REP_NULL;
struct	REP_STR	*rep_str = REP_NULL;

MENUTAB upd_menu [] = 
{
	{ " 1. SAVE        ", " Save Changes To Document And Exit " },
	{ " 2. ABORT       ", " Exit Without Saving Changes To Document " },
	{ ENDMENU }
};

int	new_file	(void);
int	del_file	(void);
int	copy_file	(void);
int	move_file	(void);
int	edit_file	(void);
int	prnt_file	(void);
int	re_draw		(void);
int	exit_func	(void);

int	opt_type;
int	no_files;

menu_type	main_menu [] = {
{"                       ",    "", _no_option,  "",0, SHOW  },
{"<New File>",    "Create New File.  [ N ]", new_file,  "Nn",0, ALL,  },
{"<Delete File>", "Delete File.  [ D ]",     del_file,  "Dd",0, ALL,  },
{"<Copy File>",   "Copy File.  [ C ]",       copy_file, "Cc",0, ALL,  },
{"<Move File>",   "Move File.  [ M ]",       move_file, "Mm",0, ALL,  },
{"<Edit File>",   "Edit File.  [ E ]",       edit_file, "Ee",0, ALL,  },
{"<Print File>",  "Print File.  [ P ]",      prnt_file, "Pp",0, ALL,  },
{"<FN3>",         "Redraw Display",          re_draw,   "", FN3,      },
{"<FN16>",        "Exit Display",            exit_func, "", FN16, ALL   },
{"",								      },
};

static	struct	var	vars [] =
{
	{1, TXT, "edit_scn",	 3, 0, 0,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "L......T.......T.......T.......T.......T.......T.......T.......T.......T.......R.......T.......T.......T.......T.......T.......T..", " ",
		16, 130,  500, "", "", local_rec.text},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*============================
| Local function prototypes  |
============================*/
int		spec_valid		(int);
int		get_filename	(void);
int		prt_no_fields	(void);
int		upd_rep			(void);
int		read_rep		(char *);
int		write_rep		(char *);
int		load_rep_names	(int);
int		clear_list		(void);
int		heading			(int);
void	print_text		(int, char *);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	int		reload;
	int		lpno;
	char *	sptr = getenv ("PROG_PATH");
	char	cmd [51];

	if (argc != 2)
	{
		/*printf ("Usage : %s directory\n", argv [0]);*/
		print_at (0,0,ML(mlMenuMess208), argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 *	Examine the directory argument. If it's not an absolute pathname,
	 *	we take it relative to $PROG_PATH
	 */
	if (*argv [1] == '/')
		directory = strdup (argv [1]);
	else
	{
		directory = (char *)malloc (strlen (sptr) + strlen (argv [1]) + 2);
		sprintf (directory, "%s/%s", sptr, argv [1]);
	}

	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 8;
	MENU_COL = 55;

	input_row = 22;
	reload = TRUE;

	SETUP_SCR (vars);
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();         /*  get into raw mode		*/

	set_masks ();		/*  setup print using masks	*/

	load_rep_names (TRUE);
	while (opt_type != EXIT_PROG)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = FALSE;
		restart = FALSE;
		init_ok = TRUE;
		swide ();
		init_vars(1);	

		if (opt_type == NEW_FILE)
			rep_str = rep_head;
		else
		{
			rep_str = (struct REP_STR *) win_select
			(
				" F I L E    S E L E C T I O N ",
				25,
				(struct SEL_STR *) rep_head,
				0,
				3,
				132,
				16
			);
			if (rep_str == REP_NULL)
			{
				if (last_char == FN1)
					load_rep_names (FALSE);
				else
				{
				   win_display(" F I L E    S E L E C T I O N ",
					25, (struct SEL_STR *) rep_head, 0, 3, 132, 16);
				}

				continue;
			}
		}

		switch (opt_type)
		{
		case NEW_FILE:
			if (!get_filename())
				break;

			heading (1);
			edit (1);

			if ( !restart && upd_rep() )
			{
				write_rep (local_rec.filename);
				no_files = FALSE;
			}

			reload = TRUE;

			break;

		case DEL_FILE:
			/* Are you sure you want to delete %s ? ",
				clip(rep_str->rep_name));*/

			sprintf(err_str, ML(mlMenuMess139),
				clip(rep_str->rep_name));
			crsr_on();
			i = prmptmsg(err_str, "YyNn", 42, 2);
			crsr_off();
			if (i == 'N' || i == 'n')
			{
				reload = FALSE;
				move (0, 2);	
				cl_line ();
				break;
			}

			sprintf (cmd, "%s/%s", directory, rep_str -> rep_name);
			unlink (cmd);

			reload = TRUE;

			break;

		case COPY_FILE:
			if (!get_filename())
				break;

			sprintf(cmd, 
				"cp %s/%s %s/%s", 
				directory, rep_str->rep_name ,
				directory, local_rec.filename);
			sys_exec( cmd );

			reload = TRUE;

			break;

		case MOVE_FILE:
			if (!get_filename())
				break;

			sprintf(cmd, 
				"mv %s/%s %s/%s", 
				directory, rep_str->rep_name ,
				directory, local_rec.filename);
			sys_exec( cmd );
			sprintf(rep_str->rep_name, 
				"%-20.20s", 
				local_rec.filename);

			reload = FALSE;

			break;

		case EDIT_FILE:
			read_rep (rep_str->rep_name);
			heading (1);
			edit (1);

			if (!restart && upd_rep())
				write_rep (rep_str->rep_name);

			reload = TRUE;
	
			break;

		case PRNT_FILE:
			read_rep (rep_str->rep_name);
			lpno = get_lpno (0);
			print_text (lpno, rep_str->rep_name);
			reload = TRUE;
			break;

		default:
			reload = TRUE;
			break;
		}

		load_rep_names (reload);
	}
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	return (EXIT_SUCCESS);
}

/*------------------------------
| Accept and validate filename |
------------------------------*/
int
get_filename (
 void)
{
	char *	sptr;
	char	tmp_filename [100];

	while (TRUE)
	{
		crsr_on();
		/*New Filename Is : */
		rv_pr(ML(mlMenuMess140), 50, 2, 0);
		getalpha(68, 2, "AAAAAAAAAAAAAA", tmp_filename);
		move (0,2);	
		cl_line();
		if (last_char == FN1)
			return (FALSE);

		sptr = strrchr(tmp_filename, '/');
		if (sptr)
			sprintf(local_rec.filename, "%-14.14s", sptr + 1);
		else
			sprintf(local_rec.filename, "%-14.14s", tmp_filename);

		clip(local_rec.filename);
		if (strlen(local_rec.filename) == 0 || 
		    strchr(local_rec.filename, ' '))
		{
			/* Invalid Filename */
			print_mess (ML (mlStdMess143));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}

		sprintf(tmp_filename, "%s/%s", directory, local_rec.filename);
		if (access(tmp_filename, 0) == 0)
		{
			/* File Already Exists */
			print_mess (ML (mlStdMess143));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}
		   
		crsr_off ();

		return (TRUE);
	}
}

int
new_file (
 void)
{
	main_menu[DEL_FILE].flag  = ALL;
	main_menu[COPY_FILE].flag = ALL;
	main_menu[MOVE_FILE].flag = ALL;
	main_menu[EDIT_FILE].flag = ALL;
	opt_type = NEW_FILE;

	return (EXIT_SUCCESS);
}

int
del_file (
 void)
{
	opt_type = DEL_FILE;
	return (EXIT_SUCCESS);
}

int
copy_file (
 void)
{
	opt_type = COPY_FILE;
	return (EXIT_SUCCESS);
}

int
move_file (
 void)
{
	opt_type = MOVE_FILE;
	return (EXIT_SUCCESS);
}

int
edit_file (
 void)
{
	opt_type = EDIT_FILE;
	return (EXIT_SUCCESS);
}

int
prnt_file (
 void)
{
	opt_type = PRNT_FILE;
	return (EXIT_SUCCESS);
}

int
re_draw (
 void)
{
	heading (1);
	win_display(" F I L E    S E L E C T I O N ", 25,(struct SEL_STR *) rep_head,0,3, 132, 16);
	if (rep_head == REP_NULL)
		prt_no_fields();
	
	return (EXIT_SUCCESS);
}

int
prt_no_fields (
 void)
{
	no_files = TRUE;
	main_menu[DEL_FILE].flag = SHOW;
	main_menu[COPY_FILE].flag = SHOW;
	main_menu[MOVE_FILE].flag = SHOW;
	main_menu[EDIT_FILE].flag = SHOW;
	main_menu[PRNT_FILE].flag = SHOW;

	/*There are no files in %s ", directory*/
	rv_pr (ML (mlStdMess143), (132 - strlen(ML(mlStdMess143))) / 2, 10, 1);

	return (EXIT_SUCCESS);
}

int
exit_func (
 void)
{
	no_files = FALSE;
	opt_type = EXIT_PROG;
	rep_head = REP_NULL;

	return (FN16);
}

int
upd_rep (
 void)
{
	mmenu_print(" EXIT FILE EDIT ", upd_menu, 0);
	switch ( mmenu_select(upd_menu) )
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

/*----------------------------------------
| Read selected filename into TXT screen |
----------------------------------------*/
int
read_rep (
 char *	rep_name)
{
	char	*sptr;

	sprintf(filename, "%s/%s", directory, rep_name);
	clip(filename);

	if ((fin = fopen(filename,"r")) == 0)
	{
		sprintf(err_str, "Error in %s during (FOPEN)",filename);
		sys_err(err_str, errno, PNAME);
	}

	scn_set(1);
	lcount[1] = 0;

	sptr = fgets(err_str, 132, fin);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		sprintf(local_rec.text, "%-130.130s", sptr);

		putval( lcount[1]++ );

		sptr = fgets(err_str, 132, fin);
	}

	fclose(fin);

	return (EXIT_SUCCESS);
}

/*--------------------------------------------
| Write to selected filename from TXT screen |
--------------------------------------------*/
int
write_rep (
 char *	rep_name)
{
	int	i;

	sprintf(filename, "%s/%s", directory, rep_name);
	clip(filename);

	if ((fout = fopen(filename, "w")) == 0)
	{
		sprintf(err_str, "Error in %s during (FOPEN)",filename);
		sys_err(err_str, errno, PNAME);
	}

	scn_set(1);

	for (i = 0; i < lcount[1]; i++)
	{
		getval(i);
		fprintf (fout, "%s\n", clip (local_rec.text));
	}

	fclose (fout);

	return (EXIT_SUCCESS);
}

/*===============================================
| Read all Files in Directory and then display. |
===============================================*/
int
load_rep_names (
 int load_all)
{
	DIR	*fd;
	struct	dirent *dirbuf;

	if (load_all)
	{
		clear_list ();

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
			rep_curr = (struct REP_STR *) malloc (sizeof (struct REP_STR));
			if (rep_curr == REP_NULL)
			{
				sys_err ("Error in load_rep_names() During (MALLOC)", 
					errno, 
					PNAME);
			}
	
			sprintf(rep_curr->rep_name, "%-20.20s", dirbuf->d_name);
	
			rep_curr->prev = rep_tail;
			rep_curr->next = REP_NULL;
			if (rep_tail != REP_NULL)
				rep_tail->next = rep_curr;
			else
				rep_head = rep_curr;
			rep_tail = rep_curr;
		}
	
		closedir (fd);

		heading (1);

	}

	win_display(" F I L E    S E L E C T I O N ", 25,(struct SEL_STR *) rep_head,0,3, 132, 16);

	if (rep_head == REP_NULL)
		prt_no_fields ();

	run_menu (main_menu, "", input_row);

	return (EXIT_SUCCESS);
}

/*----------------------------
| Free current list of files |
----------------------------*/
int
clear_list (
 void)
{
	struct	REP_STR	*tmp_ptr;

	rep_curr = rep_head;
	while (rep_curr != REP_NULL)
	{
		tmp_ptr = rep_curr;
		rep_curr = rep_curr->next;
		free(tmp_ptr);
	}

	rep_head = REP_NULL;
	rep_curr = REP_NULL;
	rep_tail = REP_NULL;
	rep_str  = REP_NULL;

	return (EXIT_SUCCESS);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set(scn);

	swide();
	clear();

	move(0,1);
	line(132);

	rv_pr(ML(mlMenuMess141), 40, 0, 1);

	move (0, 21);
	line (132);

	sprintf(err_str,ML(mlStdMess156), directory);
	clip(err_str);
	rv_pr (err_str, (132 - strlen(err_str)) / 2, 21, 0);

	line_cnt = 0;
	return (EXIT_SUCCESS);
}

void
print_text (
 int	lpno,
 char *	filename)
{
	FILE *	fout;
	int		i;

	fout = popen ("pformat", "w");
	if (fout == (FILE *) 0)
		sys_err ("Error in pformat During (POPEN)", -1, PNAME);

	fprintf (fout, ".START\n");
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".2\n");
	fprintf (fout, "Listing of: %s\n", filename);
	fprintf (fout, ".B1");
	for (i = 0; i < lcount[1]; i++)
	{
		getval (i);
		fprintf (fout, "%s\n", local_rec.text);
	}
	pclose (fout);
}

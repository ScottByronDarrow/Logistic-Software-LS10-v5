/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bg_ctrl.c      )                                 |
|  Program Desc  : ( Maintain Background Processes.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bpro,                                             |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  bpro,                                             |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 17/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : (17/10/88)      | Modified  by : Roger Gibbison.   |
|                : (11/06/89)      | Modified  by : Scott Darrow.     |
|  Date Modified : (29/07/91)      | Modified  by : Scott Darrow.     |
|  Date Modified : (06/05/93)      | Modified  by : Jonathan Chen     |
|  Date Modified : (27/07/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (05.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : (11/06/89) - Added checks on status to see if      |
|                :              options can be run.                   |
|                : (29/07/91) - Updated to use geteuid.               |
|     (06/05/93) : PSL 8620 - altered to accomodate OnLine            |
|     (27/06/93) : removal of malloc.h                                |
|     (05.10.94) : PSL 11440 bpro_pid schema changed from int to long |
|
| $Log: bg_ctrl.c,v $
| Revision 5.2  2001/08/09 09:26:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:58:33  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:43:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:23:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:14:59  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/15 22:20:05  scott
| Updated for warning errors due to -Wall flag on compiler.
|
| Revision 1.10  1999/10/15 23:42:11  scott
| Updated from ansi project
|
| Revision 1.9  1999/10/11 02:04:45  scott
| Updtaed from ansi
|
| Revision 1.8  1999/09/04 04:27:44  scott
| Updated to allow individual programs to be run and stopped.
|
| Revision 1.7  1999/07/06 03:22:49  scott
| Updated from version 10.0
|
| Revision 1.4  1998/11/12 21:05:12  kirk
| PSL 14995 - Silent License Check.
|
| Revision 1.3  1998/04/21 00:24:29  jonc
| Fixed: attempt to alter const memory
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bg_ctrl.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/bg_ctrl/bg_ctrl.c,v 5.2 2001/08/09 09:26:43 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<signal.h>
#include	<ml_utils_mess.h>
#include	<ml_std_mess.h>
#include	<DateToString.h>
#include	<pDate.h>
#include	<TimeStrings.h>

#define		SUPER_USER	( getuid() == 0 || geteuid() == 0 )

struct	list_rec	{
	char	_text[81];
	char	_status[2];
	long	_hash;
	pid_t	_pid;
	struct	list_rec	*_next;
};

struct	list_rec	*head;
struct	list_rec	*tail;
struct	list_rec	*page;

#define	L_END		(struct list_rec *)0
#define	MAX_PAGES	10
#define	P_SIZE		16
#define	PAGE_BASE	4

#define	START		0
#define	HALT		1
#define	LCL_ALL		-2
	
#define	OPT_LINE	P_SIZE + PAGE_BASE + 1

struct	list_rec	*_curr_page[P_SIZE];
struct	list_rec	*page_start[MAX_PAGES];

int	max_lines;
int	page_no;
int	last_page;
int	display_only = 0;
int	curr = 0;
int	last = -1;
int	curr_pageno = -1;

char	*blank_line = "                                                                              ";

char
	*fulloptions [] =
	{
		"[REDRAW] ",
		"[NEXT] ",
		"[PREV] ",
		"[EDIT/END] ",
		" S(tart All ",
		" H(alt All ",
		"",
	},
	*ltdoptions [] =
	{
		"[REDRAW] ",
		"[NEXT] ",
		"[PREV] ",
		"[EDIT/END] ",
		"",
	};

char	*disp_proc[] = {
	" [RESTART] ",
	" [REDRAW] ",
	" S(tart ",
	" H(alt ",
	"",
};

	/*============================+
	 | System Batch Control file. |
	 +============================*/
#define	BPRO_NO_FIELDS	11

	struct dbview	bpro_list [BPRO_NO_FIELDS] =
	{
		{"bpro_co_no"},
		{"bpro_br_no"},
		{"bpro_program"},
		{"bpro_hash"},
		{"bpro_status"},
		{"bpro_up_date"},
		{"bpro_up_time"},
		{"bpro_pid"},
		{"bpro_lpno"},
		{"bpro_parameters"},
		{"bpro_stat_flag"}
	};

	struct tag_bproRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	program [15];
		long	hash;
		char	status [2];
		long	up_date;
		char	up_time [6];
		long	pid;
		int		lpno;
		char	parameters [31];
		char	stat_flag [2];
	}	bpro_rec;

/*===========
 Table names
=============*/
static char
	*data	= "data",
	*bpro	= "bpro";


/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB			(void);
void	HaltAll			(void);
void	HaltOne 		(char *);
void	StartAll		(void);
void	StartOne 		(char *);
void	UpdateBpro		(char *);
void	DisplayOption	(char **);
void	Process			(void);
void	MainProcess		(int);
void	DeltaProcess	(int, int);
void	PaintScreen		(void);
void	TouchPage		(void);
void	LoadPage		(int, int);
int		LoadList		(void);
void	set_line		(struct list_rec *);
struct list_rec *	list_alloc	(void);
void	LclWriteLog		(long, int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	if (argc != 1 && argc != 2 && argc != 3)
	{
		print_at (0,0, mlUtilsMess719, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = getenv ("PROG_PATH");
	sprintf (err_str,"%s/BIN", (sptr == (char *)0) ? "/usr/LS10.5" : sptr);
	chdir (err_str);

	OpenDB ();

	if (argc == 2 || argc == 3)
	{
		switch (argv[1][0])
		{
		/*---------------
		| Halt All	|
		---------------*/
		case	'0':
			if (SUPER_USER)
			{
				if (argc == 2)
					HaltAll ();
				else
					HaltOne (argv [2]);
			}

			shutdown_prog();
			return (EXIT_SUCCESS);
			break;

		/*---------------
		| Start All	|
		---------------*/
		case	'1':
			if (SUPER_USER)
			{
				if (argc == 2)
					StartAll ();
				else
					StartOne (argv [2]);
			}
			shutdown_prog();
			return (EXIT_SUCCESS);
			break;

		case	'D':
		case	'd':
			display_only = 1;
			break;

		default:
			/*---------------------------
			| Usage : %s 0  : Stop All	|
			| or      %s 1  : Start All	|
			| or      %s D  : Display	|
			---------------------------*/
			print_at (0,0, mlUtilsMess720,argv[0]);
			print_at (0,0, mlUtilsMess721,argv[0]);
			print_at (0,0, mlUtilsMess722,argv[0]);
			shutdown_prog();
			return (EXIT_SUCCESS);
			break;
		}
	}
	else
		if (!SUPER_USER)
			display_only = 1;

	init_scr ();

	set_tty (); 
	Process ();

	rset_tty ();
	shutdown_prog();
	return (EXIT_FAILURE);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (bpro, bpro_list, BPRO_NO_FIELDS, "bpro_id_no");
}

void
CloseDB (void)
{
	abc_fclose (bpro);
	abc_dbclose (data);
}

void
HaltAll (void)
{
	memset (&bpro_rec, 0, sizeof (bpro_rec));

	cc = find_rec (bpro,&bpro_rec,GTEQ,"u");
	while (!cc)
	{
		if (bpro_rec.stat_flag [0] == 'A')
			UpdateBpro ("S");
		else
			abc_unlock (bpro);

		cc = find_rec (bpro,&bpro_rec,NEXT,"u");
	}
}
void
HaltOne (
	char	*progPassed)
{
	memset (&bpro_rec, 0, sizeof (bpro_rec));

	cc = find_rec (bpro,&bpro_rec,GTEQ,"u");
	while (!cc)
	{
		if (!strncmp (bpro_rec.program, progPassed, strlen (progPassed)) &&
			bpro_rec.stat_flag [0] == 'A')
			UpdateBpro ("S");
		else
			abc_unlock (bpro);

		cc = find_rec (bpro,&bpro_rec,NEXT,"u");
	}
}

void
StartAll (void)
{
	memset (&bpro_rec, 0, sizeof (bpro_rec));

	cc = find_rec (bpro,&bpro_rec,GTEQ,"u");
	while (!cc)
	{
		if (bpro_rec.stat_flag [0] == 'A')
			UpdateBpro ("R");
		else
			abc_unlock (bpro);

		cc = find_rec (bpro,&bpro_rec,NEXT,"u");
	}
}
void
StartOne (
	char	*progPassed)
{
	memset (&bpro_rec, 0, sizeof (bpro_rec));

	cc = find_rec (bpro,&bpro_rec,GTEQ,"u");
	while (!cc)
	{
		if (!strncmp (bpro_rec.program, progPassed, strlen (progPassed)) &&
			bpro_rec.stat_flag [0] == 'A')
			UpdateBpro ("R");
		else
			abc_unlock (bpro);

		cc = find_rec (bpro,&bpro_rec,NEXT,"u");
	}
}

void
UpdateBpro (
	char	*status)
{
	char	*sptr;
	char	*tptr;
	char	program[15];
	char	lpno [5];
	int	indx = 0;

	switch (status[0])
	{
	/*---------------
	| Start Process	|
	---------------*/
	case	'R':
		/*-----------------------
		| Program Not Running	|
		-----------------------*/
		if (bpro_rec.pid == 0 || kill ((pid_t) bpro_rec.pid, 0) != 0)
		{
			bpro_rec.pid = fork ();

			if (bpro_rec.pid == 0)
			{
				setpgrp ();
				strcpy (program, bpro_rec.program);
				arg[0] = clip (program);

				if (bpro_rec.lpno > 0)
				{
					sprintf (lpno, "%d", bpro_rec.lpno);
					arg[++indx] = lpno;
				}

				tptr = bpro_rec.parameters;
				sptr = strchr (tptr,'~');

				while (sptr != (char *)0)
				{
					*sptr = '\0';
					arg[++indx] = tptr;

					tptr = sptr + 1;
					sptr = strchr (tptr,'~');
				}

				arg[++indx] = (char *)0;

				execvp (arg[0],arg);
				exit (errno);
			}
			LclWriteLog (bpro_rec.hash, 1);
		}
		/*---------------
		| Wakeup	|
		---------------*/
		else
			kill ((pid_t) bpro_rec.pid, SIGUSR2);
		strcpy (bpro_rec.status, "R");
		bpro_rec.up_date = TodaysDate ();
		sprintf (bpro_rec.up_time, "%-5.5s", TimeHHMM ());
		break;

	case	'S':
		/*---------------
		| Terminate	|
		---------------*/
		if (bpro_rec.pid > 1)
		{
			kill ((pid_t) bpro_rec.pid, SIGTERM);
			kill ((pid_t) bpro_rec.pid, SIGUSR1);
			bpro_rec.up_date = TodaysDate ();
			sprintf (bpro_rec.up_time, "%-5.5s", TimeHHMM ());
			LclWriteLog (bpro_rec.hash, 0);
		}
		strcpy (bpro_rec.status, "S");
		bpro_rec.pid = 0;
		break;

	default:
		strcpy (bpro_rec.status, "U");
		break;
	}

	cc = abc_update (bpro,&bpro_rec);
	if (cc)
		sys_err ("Error in bpro during (DBUPDATE)",cc,PNAME);

	abc_unlock (bpro);
}

void
DisplayOption (
	char	**prmpts)
{
	register	int	i;
	int	x = 10;

	move (1,OPT_LINE);
	cl_line ();

	for (i = 0;strlen (prmpts[i]);i++,x += 2)
	{
		rv_pr (prmpts[i],x,OPT_LINE,1);
		x += strlen (prmpts[i]);
	}
	
	rv_pr (ML (mlUtilsMess091),1,OPT_LINE,0);
	crsr_on ();
}

void
Process (void)
{
	char **	useroptions = NULL;

	if (display_only)
	{
		useroptions = ltdoptions;
		curr = -1;
		last = -1;
	} else
		useroptions = fulloptions;

	if (LoadList ())
	{
		clear ();
		rv_pr (ML (mlUtilsMess092),1,1,1);
		fflush (stdout);
		sleep (sleepTime);
		return;
	}

	PaintScreen ();

	while (1)
	{
		if (curr_pageno != page_no)
		{
			TouchPage ();
			LoadPage (1,curr);
			DisplayOption (useroptions);
			curr_pageno = page_no;
			last = curr;
		}

		if (last != curr)
		{
			if (last >= 0)
				rv_pr (_curr_page[last]->_text,1,last + PAGE_BASE,0);

			if (curr >= 0)
				rv_pr (_curr_page[curr]->_text,1,curr + PAGE_BASE,1);
			last = curr;
		}

		rv_pr (ML (mlUtilsMess091),1,OPT_LINE,0);

		last_char = getkey ();

		switch (last_char)
		{
		case	'H':
		case	'h':
			if (display_only)
				putchar (BELL);
			else
				DeltaProcess (LCL_ALL,1);
			last = -1;
			break;

		case	'S':
		case	's':
			if (display_only)
				putchar (BELL);
			else
				DeltaProcess (LCL_ALL,0);
			last = -1;
			break;

		/*---------------
		| Redraw	|
		---------------*/
		case	FN3:
			PaintScreen ();
			curr_pageno = -1;
			break;

		/*---------------
		| Next Page	|
		---------------*/
		case	FN13:
		case	FN14:
			if (--page_no < 0)
				page_no = last_page;

			page = page_start[page_no];
			break;

		/*---------------
		| Previous Page	|
		---------------*/
		case	FN15:
			if (++page_no > last_page)
				page_no = 0;

			page = page_start[page_no];
			break;

		case	DOWN_KEY:
		case	10:
			if (display_only)
				putchar (BELL);
			else
				if (++curr == max_lines)
					curr = 0;
			break;

		case	UP_KEY:
		case	11:
			if (display_only)
				putchar (BELL);
			else
				if (--curr < 0)
					curr = max_lines - 1;
			break;

		case	'\r':
			if (display_only)
				putchar (BELL);
			else
			{
				MainProcess (curr);
				DisplayOption (useroptions);
				last = -1;
			}
			break;

		/*-------
		| Exit	|
		-------*/
		case	FN16:
			return;

		case	-1:
			break;

		default:
			putchar (BELL);
			break;
		}
	}
}

void
MainProcess (
	int		indx)
{
	DisplayOption (disp_proc);

	while (1)
	{
		if (curr_pageno != page_no)
		{
			TouchPage ();
			LoadPage (1,curr);
			DisplayOption (disp_proc);
			curr_pageno = page_no;
			last = curr;
		}

		rv_pr (ML (mlUtilsMess091),1,OPT_LINE,0);

		last_char = getkey ();

		switch (last_char)
		{
		case	FN1:
		case	'\r':
			return;

		case	FN3:
			PaintScreen ();
			curr_pageno = -1;
			last = -1;
			break;

		case	'S':
		case	's':
			DeltaProcess (indx,0);
			return;

		case	'H':
		case	'h':
			DeltaProcess (indx,1);
			return;

		case	-1:
			break;

		default:
			putchar (BELL);
			break;
		}
	}
}

void
DeltaProcess (
	int		indx,
	int		_halt_process)
{
	struct	list_rec	*tptr = head;

	if (indx >= 0)
	{
		cc = find_hash (bpro,&bpro_rec,COMPARISON,"u",_curr_page[indx]->_hash);
		if (!cc)
		{
			if (!display_only)
				UpdateBpro ((_halt_process) ? "S" : "R");
			set_line (_curr_page[indx]);
		}
		abc_unlock (bpro);
	}

	if (indx == LCL_ALL)
	{
		while (tptr != L_END)
		{
			cc = find_hash (bpro,&bpro_rec,COMPARISON,"u",tptr->_hash);
			if (!cc)
			{
				if (!display_only)
					UpdateBpro ((_halt_process) ? "S" : "R");
				set_line (tptr);
			}
			abc_unlock (bpro);
			tptr = tptr->_next;
		}
	}
}

/*=======================================
| Paint Basic Bits & Pieces on Screen	|
=======================================*/
void
PaintScreen (void)
{
	clear ();

	box (0,1,80,P_SIZE + 2);

	sprintf (err_str, "%26.26s%s%25.25s", " ", ML(mlUtilsMess093), " ");
	rv_pr (err_str, 0,0,1);

	print_at (2,1, ML(mlUtilsMess090) );

	move (1,3);
	line (79);
}

/*=======================================
| Read Bpro records for current page	|
| if status has changed then flag for	|
| redisplay of page.			|
=======================================*/
void
TouchPage (void)
{

	abc_selfield (bpro,"bpro_hash");
	for (line_cnt = 0;line_cnt < max_lines;line_cnt++)
	{
		cc = find_hash (bpro,&bpro_rec,COMPARISON,"u",_curr_page[line_cnt]->_hash);

		if (!cc)
		{
			if (!display_only && bpro_rec.pid > 1 &&
				kill ((pid_t) bpro_rec.pid, 0) != 0)
				UpdateBpro ("S");
			set_line (_curr_page[line_cnt]);
		}
		abc_unlock (bpro);
	}
}

void
LoadPage (
	int		_draw_page,
	int		line_no)
{
	struct	list_rec	*tptr = page;

	line_cnt = 0;

	abc_selfield (bpro,"bpro_hash");
	while (line_cnt < P_SIZE && tptr != L_END)
	{
		_curr_page[line_cnt] = tptr;

		cc = find_hash (bpro,&bpro_rec,COMPARISON,"u",tptr->_hash);

		if (!cc)
		{
			if (!display_only && bpro_rec.pid > 1 &&
				kill ((pid_t) bpro_rec.pid, 0) != 0)
				UpdateBpro ("S");
			set_line (tptr);
		}
		abc_unlock (bpro);

		if (_draw_page)
			rv_pr (tptr->_text,1,line_cnt + PAGE_BASE, (line_cnt == line_no));

		line_cnt++;
		
		tptr = tptr->_next;
	}
	max_lines = line_cnt;

	if (max_lines < curr)
	{
		curr = max_lines - 1;
		last = -1;
	}

	for (;_draw_page && line_cnt < P_SIZE;line_cnt++)
	{
		_curr_page[line_cnt] = (struct list_rec *)0;
		rv_pr (blank_line,1,line_cnt + PAGE_BASE,0);
	}
}

int
LoadList (void)
{
	int	loaded = FALSE;
	struct	list_rec	*tptr;

	page_no = 0;
	head = L_END;
	tail = L_END;
	line_cnt = 0;
	last_page = 0;

	abc_selfield (bpro,"bpro_id_no");

	memset (&bpro_rec, 0, sizeof (bpro_rec));

	cc = find_rec (bpro,&bpro_rec,GTEQ,"r");
	while (!cc)
	{
		if (bpro_rec.stat_flag [0] != 'A')
		{
			cc = find_rec (bpro,&bpro_rec,NEXT,"r");
			continue;
		}
		loaded = TRUE;
		tptr = list_alloc ();
		if (tptr == L_END)
			break;

		tptr -> _hash = bpro_rec.hash;
		tptr->_next = L_END;

		set_line (tptr);

		if (head == L_END)
		{
			head = tptr;
			tail = tptr;
		}
		else
		{
			tail->_next = tptr;
			tail = tail->_next;
		}

		if (line_cnt % P_SIZE == 0)
		{
			last_page = line_cnt / P_SIZE;
			page_start[last_page] = tptr;
		}
		line_cnt++;

		cc = find_rec (bpro,&bpro_rec,NEXT,"r");
	}
	if (!loaded)
		return (EXIT_FAILURE);

	page = head;
	abc_selfield (bpro,"bpro_hash");

	LoadPage (0,0);
	return (EXIT_SUCCESS);
}

void
set_line ( 
	struct	list_rec	*tptr)
{
	char	action[15];

	if (tptr == (struct list_rec *)0)
		return;

	switch (bpro_rec.status [0])
	{
	case	'R':
		strcpy (action,"Running");
		break;

	case	'S':
		strcpy (action,"Stopped");
		break;

	default:
		strcpy (action,"Unknown");
		break;
	}

	/*-----------------------
	| Status Has Changed	|
	-----------------------*/
	if (tptr -> _status [0] != bpro_rec.status [0])
	{
		strcpy (tptr -> _status, bpro_rec.status);
		curr_pageno = -1;
	}

	/*-------------------------------
	| Process Id has Changed	|
	-------------------------------*/
	if (tptr -> _pid != bpro_rec.pid)
	{
		tptr -> _pid = bpro_rec.pid;
		curr_pageno = -1;
	}

	sprintf (tptr->_text,"   %-2.2s    %-2.2s    %-14.14s   %-10.10s %s %-5.5s   %6ld     %2d   ",
		bpro_rec.co_no,
		bpro_rec.br_no,
		bpro_rec.program,
		action,
		DateToString (bpro_rec.up_date),
		bpro_rec.up_time,
		bpro_rec.pid,
		bpro_rec.lpno);
}

struct	list_rec *list_alloc ()
{
	return ((struct list_rec *) malloc ((unsigned) sizeof (struct list_rec)));
}

void
LclWriteLog (
	long	hash,
	int		status)
{
	int day, month;
	FILE	*aud_log = fopen ("/usr/adm/bg.log","a");

	if (!aud_log)
		return;

	DateToDMY (TodaysDate (), &day, &month, NULL);
	fprintf (aud_log,"%02d/%02d %-5.5s %ld %d\n",
					day, month, TimeHHMM (),hash,status);
	fclose (aud_log);
}

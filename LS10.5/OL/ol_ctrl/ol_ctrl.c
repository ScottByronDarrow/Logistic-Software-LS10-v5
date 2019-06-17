/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ol_ctrl.c      )                                 |
|  Program Desc  : ( Display Branch Status (Online / Offline).    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  esmr,                                             |
|  Database      : (oldb)                                             |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 06/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (06/04/89)      | Modified by : Huon Butterworth   |
|  Date Modified : (06/07/91)      | Modified by : Campbell Mander.   |
|  Date Modified : (10/07/91)      | Modified by : Campbell Mander.   |
|  Date Modified : (12/03/92)      | Modified by : Campbell Mander.   |
|  Date Modified : (29/03/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (15.06.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (04/09/97)      | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (15/10/1997)    | Modified  by : Jiggs A Veloz.    |
|                                                                     |
|  Comments      : Based on bg_ctrl.c                                 |
|                :                                                    |
|  (06/07/91)    : Scrolling over page boundaries was not working and |
|                : all function keys were not being displayed.        |
|  (10/07/91)    : Changed so that all list is loaded starting at     |
|                : first branch for company not the branch you are    |
|                : logged in as.                                      |
|  (12/03/92)    : Fix prototyping of SIGNALS for ICL DRS6000.        |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|  (15.06.94)    : Converted to pslscr                                |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to LOCAL_PSIZE   |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (15/10/1997)  : SEL Updated to initialise strings stored in an     |
|                :     array for multilingual.                        |
|                :                                                    |
|                :                                                    |
| $Log: ol_ctrl.c,v $
| Revision 5.2  2001/08/09 09:14:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:47  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:22  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.13  1999/11/25 10:24:04  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.12  1999/11/08 08:42:25  scott
| Updated due to warning errors when compiling using -Wall flag.
|
| Revision 1.11  1999/09/29 10:11:24  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/20 05:51:22  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/09/10 02:08:45  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.8  1999/06/15 09:39:16  scott
| Updated for log file.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_ctrl.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_ctrl/ol_ctrl.c,v 5.2 2001/08/09 09:14:16 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<signal.h>
#include	<ml_ol_mess.h>
#include	<ml_std_mess.h>

struct	list_rec	{
	int	_t_flag;
	char	_text[81];
	int	_online;
	char	_co [3],
		_br [3];
	struct	list_rec	*_next;
};

struct	list_rec	*head;
struct	list_rec	*tail;
struct	list_rec	*page;

#define	L_END		(struct list_rec *)0
#define	MAX_PAGES	10
#define	LOCAL_PSIZE	16
#define	PAGE_BASE	4

#define	START		0
#define	HALT		1

#define	BLNK_LINE	"                                                                         "
	
#define	OPT_LINE	LOCAL_PSIZE + PAGE_BASE + 1

struct	list_rec	*_curr_page[LOCAL_PSIZE];
struct	list_rec	*page_start[MAX_PAGES];

int	max_lines;
int	page_no;
int	last_page;
int	curr = 0;
int	last = -1;
int	curr_pageno = -1;

char	*blank_line = "                                                                        ";

char	*disp_esmr[] = {
	" [FN3] ",
	" [FN14] ",
	" [FN15] ",
	" [FN16] ",
	"",
};

/*==========================
| Common Record Structure. |
==========================*/
static	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

static	int comm_no_fields = 6;

static	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	t_dbt_date;
	} comm_rec;

	/*============================
	| System Batch Control file. |
	============================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_online"}
	};

	int esmr_no_fields = 4;

	struct {
		char	br_co_no[3];
		char	br_br_no[3];
		char	br_name[41];
		int	br_online;
	} esmr_rec;

#include	<std_decs.h>

void OpenDB (void);
void CloseDB (void);
void disp_option (char *prmpts []);
void set_option (int p_flag);
void process (void);
void paint_scn (void);
void touch_page (void);
void set_page (void);
void disp_page (void);
void load_list (void);
void set_line (struct list_rec *tptr);
struct list_rec *list_alloc (void);
void alarm_used (int x);
void alarm_on (void);
void alarm_off (void);
void initML (void);

int
main (
	int argc,
	char *argv [])
{
	char	*sptr;

	if (argc != 1)
	{
		printf ("Usage : %s\n",argv[0]);
		return (EXIT_FAILURE);
	}

	sptr = getenv("PROG_PATH");
	sprintf(err_str,"%s/BIN",(sptr == (char *)0) ? "/usr/LS10.5" : sptr);

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	initML();

	init_scr();

	set_tty(); 
	process();

	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

void
OpenDB (
	void)
{
	abc_dbopen("data");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec("comm",comm_list,comm_no_fields,"comm_term");
}

void
CloseDB (
	void)
{
	abc_fclose("esmr");
	abc_fclose("comm");
	abc_dbclose("data");
}

void
disp_option (
	char *prmpts [])
{
	register	int	i;
	int	x = 15;

	move(3,OPT_LINE);
	cl_line();

	for (i = 0;strlen(prmpts[i]);i++,x += 2)
	{
		rv_pr(prmpts[i],x,OPT_LINE,1);
		x += strlen(prmpts[i]);
	}
	set_option (TRUE);
}

void
set_option (
	int p_flag)
{
/*	static 	char	*o_prmpt = "Option : ";*/

	sprintf (err_str, ML(mlOlMess004) );

	if (p_flag)
		rv_pr(err_str,3,OPT_LINE,0);
	else
		move (3 + strlen (err_str), OPT_LINE);
	crsr_on();
}

void
process (
	void)
{
	load_list();

	paint_scn();

	while (1)
	{
		if (curr_pageno != page_no)
		{
			touch_page();
			disp_page ();
			set_option(FALSE);
			curr_pageno = page_no;
			last = curr;
		}

		last_char = getkey();

		switch (last_char)
		{
		/*---------------
		| Redraw	|
		---------------*/
		case	FN3:
			paint_scn();
			set_page();
			curr_pageno = -1;
			break;

		/*---------------
		| Next Page	|
		---------------*/
		case	FN13:
		case	FN14:
			if (++page_no > last_page)
				page_no = 0;

			page = page_start[page_no];
			set_page();
			break;

		/*---------------
		| Previous Page	|
		---------------*/
		case	FN15:
			if (--page_no < 0)
				page_no = last_page;

			page = page_start[page_no];
			set_page();
			break;

		/*-------
		| Exit	|
		-------*/
		case	FN16:
			alarm_off();
			return;

		case	-1:
			break;

		default:
			putchar(BELL);
			break;
		}
	}
}

/*=======================================
| Paint Basic Bits & Pieces on Screen	|
=======================================*/
void
paint_scn (
	void)
{
	clear();

	box(2,1,75,LOCAL_PSIZE + 2);

/*"                        ON-LINE BRANCH STATUS DISPLAY                    "*/
	rv_pr( ML(mlOlMess017), 3,0,1);

/*" Company Branch  Name                                        Status   ");*/
	print_at(2,3, ML(mlOlMess003), " ");

	move(3,3);
	line(74);
	disp_option(disp_esmr);
}

/*=======================================
| Read Bpro records for current page	|
| if status has changed then flag for	|
| redisplay of page.			|
=======================================*/
void
touch_page (
	void)
{
	alarm_off();

	for (line_cnt = 0;line_cnt < max_lines;line_cnt++)
	{
		strcpy (esmr_rec.br_co_no, _curr_page[line_cnt]->_co);
		strcpy (esmr_rec.br_br_no, _curr_page[line_cnt]->_br);
		if (!(cc = find_rec("esmr",&esmr_rec,COMPARISON,"r")))
			set_line(_curr_page[line_cnt]);
	}
	alarm_on();
}

void
set_page (
	void)
{
	struct	list_rec	*tptr = page;

	line_cnt = 0;

	alarm_off();

	while (line_cnt < LOCAL_PSIZE && tptr != L_END)
	{
		tptr->_t_flag = TRUE;
		_curr_page[line_cnt] = tptr;
		line_cnt++;
		tptr = tptr->_next;
	}
	max_lines = line_cnt;

	if (max_lines < curr)
	{
		curr = max_lines - 1;
		last = -1;
	}

	alarm_on();
}

void
disp_page (
	void)
{
	struct	list_rec	*tptr = page;
	int	i;

	line_cnt = 0;

	alarm_off();
	crsr_off();

	while (line_cnt < LOCAL_PSIZE && tptr != L_END)
	{
		_curr_page[line_cnt] = tptr;

		if (tptr->_t_flag)
			rv_pr(tptr->_text,3,line_cnt + PAGE_BASE,
								tptr->_online);
		tptr->_t_flag = FALSE;
		line_cnt++;
		tptr = tptr->_next;
	}
	max_lines = line_cnt;

	if (line_cnt < LOCAL_PSIZE)
	{
		for (i = line_cnt; i < LOCAL_PSIZE; i++)
			rv_pr(BLNK_LINE, 3, i + PAGE_BASE, 0);
	}

	if (max_lines < curr)
	{
		curr = max_lines - 1;
		last = -1;
	}

	alarm_on ();
}

void
load_list (
	void)
{
	struct	list_rec	*tptr;

	page_no = 0;
	head = L_END;
	tail = L_END;
	line_cnt = 0;
	last_page = 0;

	alarm_off();

	strcpy(esmr_rec.br_co_no, comm_rec.tco_no);
	strcpy(esmr_rec.br_br_no, " ");

	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");
	while (!cc)
	{
		tptr = list_alloc();
		if (tptr == L_END)
			break;

		strcpy (tptr->_co, esmr_rec.br_co_no);
		strcpy (tptr->_br, esmr_rec.br_br_no);
		tptr->_next = L_END;

		set_line(tptr);

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

		if (line_cnt % LOCAL_PSIZE == 0)
		{
			last_page = line_cnt / LOCAL_PSIZE;
			page_start[last_page] = tptr;
		}
		line_cnt++;

		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	page = head;

	set_page ();
}

void
set_line (
	struct list_rec *tptr)
{
	char	action[15];

	if (tptr == (struct list_rec *)0)
		return;

	switch (esmr_rec.br_online)
	{
	case	0 :
		strcpy(action,"Off-line  ");
		break;

	default :
		sprintf (action,"%d On-line", esmr_rec.br_online);
		break;
	}

	/*-----------------------
	| Status Has Changed	|
	-----------------------*/
	if (tptr->_online != esmr_rec.br_online)
	{
		tptr->_online = esmr_rec.br_online;
		tptr->_t_flag = TRUE;
		curr_pageno = -1;
	}

	sprintf(tptr->_text,"   %-2.2s     %-2.2s     %-40.40s    %-10.10s  ",
		esmr_rec.br_co_no,
		esmr_rec.br_br_no,
		esmr_rec.br_name,
		action);
}

struct	list_rec *list_alloc (
	void)
{
	return((struct list_rec *) malloc((unsigned) sizeof(struct list_rec)));
}

void
alarm_used (
	int x)
{
	alarm_off();
	touch_page();
}

void
alarm_on (
	void)
{
	signal(SIGALRM,alarm_used);
	alarm (2);
}

void
alarm_off (
	void)
{
	signal(SIGALRM,SIG_DFL);
	alarm (0);
}

/*==========================================
| Initilisation of array for multilingual. |
==========================================*/
void
initML (
	void)
{
	disp_esmr[0] = strdup ( ML(mlOlMess071) );
	disp_esmr[1] = strdup ( ML(mlOlMess072) );
	disp_esmr[2] = strdup ( ML(mlOlMess073) );
	disp_esmr[3] = strdup ( ML(mlOlMess074) );
}


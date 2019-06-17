/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( run_extern.c   )                                 |
|  Program Desc  : ( External Sub Menu System.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  SUB_MENU/                                         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 30/08/88          |
|---------------------------------------------------------------------|
|  Date Modified : (30/08/88)      | Modified  by : Scott Darrow.     |
|  Date Modified : (25/08/89)      | Modified  by : Scott Darrow.     |
|  Date Modified : (03/09/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (27/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/03/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (13/11/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (05/09/97)      | Modified  by : Roanna Marcelino. |
|  Date Modified : (15/10/97)      | Modified  by : Roanna Marcelino. |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (25/08/89) - Added larger fast number and updated  |
|                : to make.                                           |
|  (03/09/90)    : General Update for New Scrgen. S.B.D.              |
|  (27/05/91)    : Security codes may now be up to 8 chars long.      |
|  (17/03/92)    : Force typecasting of strlen() for ANSI C compliance|
|  (13/11/92)    : PSL 8060 changes to comm due to company specific   |
|                : env var filenames .                                |
|  (15/10/97)    : Updated execlp for Multilingual Conversion.        |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: run_extern.c,v $
| Revision 5.3  2002/07/17 09:57:26  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 05:13:53  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:46  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:35  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/07/05 01:34:10  scott
| Updated to re-define curr_menu to currentMenu due to conflict with tab routines
|
| Revision 1.16  2000/02/18 01:56:30  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.15  1999/12/06 01:47:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/16 09:42:03  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.13  1999/10/20 02:06:51  nz
| Updated for final changes on date routines.
|
| Revision 1.12  1999/10/16 04:56:37  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.11  1999/09/17 07:27:16  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.10  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.9  1999/06/15 02:36:57  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: run_extern.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/run_extern/run_extern.c,v 5.3 2002/07/17 09:57:26 scott Exp $";

#include	<pslscr.h>
#include	<menu.h>
#include	<sub_menu.h>
#include	<ldate.menu.h>
#include	<account.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#undef	MIN_ROW
#undef	MIN_COL

#define	MIN_ROW	0
#define	MIN_COL	0

struct	OPT	{
	int		_row;		/* row number					*/
	int		_col;		/* col number					*/
	char *	_name;		/* option description (name)	*/
	char *	_program;
} option [MAX_OPT + 1];

struct	{
	int	_row;		/* position of first 			*/
	int	_col;		/* option in quadrant			*/
	int	_off;		/* used to calculate row position	*/
} quads[MAX_EXTRA];

struct	comm_type	{
	int		termno;
	char	CONO [3];
	char	CONAME [41];
	char	COSHORT [16];
	char	BRNO [3];
	char	BRNAME [41];
	char	BRSHORT [16];
	char	WHNO [3];
	char	WHNAME [41];
	char	WHSHORT [10];
	char	ENV_NAME [61];
	long	dbt_date;
	long	crd_date;
	long	inv_date;
	long	gl_date;
	int		FIS;
} comm_rec;

struct	co {
	char *	lbl;
	char *	val;
} com [] = {
	{"CONO"},
	{"CONAME"},
	{"COSHORT"},
	{"BRNO"},
	{"BRNAME"},
	{"BRSHORT"},
	{"WHNO"},
	{"WHNAME"},
	{"WHSHORT"},
	{"DBTDATE"},
	{"CRDDATE"},
	{"INVDATE"},
	{"GLDATE"},
	{"GLPER"},
	{"FIS"},
	{"ENV_NAME"},
	{""}
};
	int	pmenu = 0;
	int	window = 1;


/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(char *);
void	LCL_init_menu	(void);
void	dsp_menu		(char *);
void	_get_comm		(void);
void	_get_secure		(void);
void	LCL_draw_menu	(void);
void	_draw_mask		(void);
int		_show_menu		(char *);
void	_load_menu		(char *);
int		_set_coord		(void);
void	_substit		(char *);
void	_run_menu		(int);
void	_psl_kill		(int);
void	_pr_busy		(int);
int		Chk_security	(char *, char *);
int		heading			(void);


int
main (
 int	argc,
 char *	argv [])
{
	char *	menu_name;
	if (argc != 2)
	{
		print_at (0, 0, mlStdMess037, argv[0]);
		return (EXIT_FAILURE);
	}

	menu_name = get_sub (argv [1]);

	if (menu_name != (char *)0)
		dsp_menu (menu_name);
	else
	{
		shutdown_prog (ML (mlMenuMess220));
	}
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 char *	reason)
{
	rset_tty ();
	clear ();
	snorm ();
	print_at (0, 0, reason);
	crsr_on ();
}

void
LCL_init_menu (
 void)
{
	signal (SIGINT, _psl_kill);

	init_scr ();
	set_tty ();
	crsr_off ();

	_get_secure ();
	_get_comm ();
}

void
dsp_menu (
 char *	mname)
{
	char	old_menu [60];
	int		line_no = 1;
	char *	sptr;

	strcpy (currentMenu,(mname != (char *)0) ? mname : start_menu);
	
	LCL_init_menu ();

	while (1)
	{
		_load_menu (currentMenu);

		while (1)
		{
			LCL_draw_menu ();
			line_no = _show_menu (currentMenu);

			/*-----------------------
			| Program to Execute	|
			-----------------------*/
			if ( strncmp(option[line_no]._program,"menu ",5) &&
			     strncmp(option[line_no]._program,"pmenu ",6))
			{
				_run_menu (line_no);
				set_tty ();
				snorm ();
			}
			if (!strncmp(option[line_no]._program,"menu ",5))
			{
				pmenu = 0;
				sptr = option[line_no]._program + 4;
				while (*sptr && *sptr == ' ')
					sptr++;

				strcpy(currentMenu,sptr);
				break;
			}
			if (!strncmp(option[line_no]._program,"pmenu ",6))
			{
				strcpy(old_menu, currentMenu);

				sptr = option[line_no]._program + 5;
				while (*sptr && *sptr == ' ')
					sptr++;

				strcpy(currentMenu,sptr);
				pmenu = 1;
				break;
			}
			if (pmenu)
			{
				strcpy(currentMenu, old_menu);
				pmenu = 0;
				break;
			}
			rset_tty();
			crsr_on();
			exit (0);
		}
	}
}

void
_get_comm (
 void)
{
	int		fd;
	int		monthPeriod;
	char	Period [3];
	char	fis_year [3];
	char *	sptr = getenv ("PROG_PATH");
	char	filename [101];

	term_number = ttyslt();

	sprintf (filename,"%s/BIN/MENUSYS/COMM",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	_cc = RF_OPEN (filename, sizeof(struct comm_type), "r", &fd);
	if (_cc)
	{
		/*_exit_prog(" Pinnacle file MENUSYS/COMM does not exist. ");*/
		shutdown_prog (ML (mlMenuMess271));
		exit (0);
	}
	
	_cc = RF_READ (fd, (char *) &comm_rec);

	while (!_cc && comm_rec.termno != term_number)
		_cc = RF_READ(fd, (char *) &comm_rec);

	RF_CLOSE(fd);

	sprintf(fis_year,"%2d",comm_rec.FIS);

	com[0].val  = clip(comm_rec.CONO);
	com[1].val  = clip(comm_rec.CONAME);
	com[2].val  = clip(comm_rec.COSHORT);
	com[3].val  = clip(comm_rec.BRNO);
	com[4].val  = clip(comm_rec.BRNAME);
	com[5].val  = clip(comm_rec.BRSHORT);
	com[6].val  = clip(comm_rec.WHNO);
	com[7].val  = clip(comm_rec.WHNAME);
	com[8].val  = clip(comm_rec.WHSHORT);
	com[9].val  = strdup (DateToString(comm_rec.dbt_date));
	com[10].val = strdup (DateToString(comm_rec.crd_date));
	com[11].val = strdup (DateToString(comm_rec.inv_date));
	com[12].val = strdup (DateToString(comm_rec.gl_date));

	DateToDMY (comm_rec.gl_date, NULL, &monthPeriod, NULL);
	sprintf (Period, "%02d", monthPeriod);
	com[13].val = strdup (Period);

	com[14].val = strdup (fis_year);
	com[15].val = clip(comm_rec.ENV_NAME);
}

void
_get_secure (
 void)
{
	char *	sptr = getenv ("PROG_PATH");
	char *	tptr;
	char	filename [101];

	curr_user = getenv ("LOGNAME");

	if (curr_user == (char *)0)
	{
		/*_exit_prog(" You Don't Have Environment variable LOGNAME set ");*/
		shutdown_prog (ML (mlMenuMess239));
		exit (0);
	}

	security = (char *)0;

	sprintf(filename,"%s/BIN/MENUSYS/User_secure",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	if ((fmenu = fopen(filename,"r")) == 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}

	sptr = fgets(data,DATA_SIZE,fmenu);

	while (sptr != (char *)0)
	{
		tptr = sptr;
		while (*tptr != ' ' && *tptr != '\t')
			tptr++;

		*tptr = '\0';
		/*-----------------------------------------------
		| Find the Appropriate Entry for the User	|
		-----------------------------------------------*/
		if (!strcmp(data,curr_user))
		{
			sptr = tptr + 1;
			tptr = strchr (sptr,'<');
			/*-------------------------------
			| Found Start of security	|
			-------------------------------*/
			if (tptr != (char *)0)
			{
				sptr = tptr + 1;
				tptr = strchr (sptr,'>');
				if (tptr != (char *)0)
				{
					*tptr = '\0';
					security = strdup (sptr);
					break;
				}
			}
		}
		sptr = fgets(data,DATA_SIZE,fmenu);
	}

	if (security == (char *)0)
	{
		/*_exit_prog(" You Are Not A Valid User ");*/
		shutdown_prog (ML (mlMenuMess240));
		exit (0);
	}

	downshift(security);
}

void
LCL_draw_menu (
 void)
{
	int	i;

	for (i = min_row; i <= max_row; i++)
	{
		print_at (i, min_col,"%*.*s", max_col - min_col,max_col - min_col," ");
		fflush (stdout);
	}

	move (min_col + 1,min_row + 2);
	line (max_col - min_col - 1);

	/*-------------------------------------------------------
	| Only Draw Default Box if No Special Bits Exist	|
	-------------------------------------------------------*/
	if (max_box == 0 && max_line == 0 && max_graph == 0)
		box(min_col,min_row,max_col - min_col,max_row - min_row - 1);

	sprintf (err_str, " %s ", option[ 0 ]._name);

	us_pr( err_str, option[0]._col - 2,option[0]._row,1);

	for (i = 1; i < max_opt; i++)
	{
		print_at (option[i]._row,option[i]._col-4,"%2d. %s",i,option[i]._name);
	}
}

void
_draw_mask (
 void)
{
	move (1, 2);
	line (WIDTH - 1);

	move (1,21);
	line (WIDTH - 1);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 2, err_str,com[0].val,com[2].val);

	strcpy (err_str, ML (mlStdMess039));
	print_at (22, 2, err_str,com[3].val, com[5].val);

	strcpy (err_str, ML (mlStdMess099));
	print_at (22, 2, err_str, com[6].val, com[8].val);
}

int
_show_menu (
 char *	mname)
{
	int	c,
		key = 0;
	int	fn;
	int	curr = 1;
	int	last = -1;
	int	nindx = 0;
	int	findx = 0;
	char	old_menu [61];
	char	num_str [3];
	char	fast_str [FASTLEN + 4];

	rv_pr (option[curr]._name,option[curr]._col,option[curr]._row,1);
	num_str [0] = '\0';
	strcpy (fast_str,"        ");

	while (1)
	{
		if (last != curr)
		{
			if (valid_opt(last))
				rv_pr(option[last]._name,option[last]._col,option[last]._row,0);
			if (valid_opt(curr))
				rv_pr(option[curr]._name,option[curr]._col,option[curr]._row,1);
		}

		last = curr;
		c = getkey();

		if (c >= FN17 && c < FN32)
		{
			fn = c;
			c = FN17;
		}

		if (_isdigit(c))
		{
			key = c;
			c = '0';
		}

		if (_islower(c) || _isupper(c))
		{
			key = to_upper(c);
			c = 'A';
		}

		switch (c)
		{
		case	'A':
			putchar(BELL);
			break;

		case	'0':
			num_str[nindx++] = key;
			num_str[nindx] = '\0';
			if (!valid_opt(atoi(num_str)))
			{
				nindx = 0;
				num_str[nindx++] = key;
				num_str[nindx] = '\0';
				if (!valid_opt(atoi(num_str)))
				{
					nindx = 0;
					num_str[0] = '\0';
					putchar(BELL);
				}
				else
					curr = atoi(num_str);
			}
			else
				curr = atoi(num_str);
			break;

		case	HELP:
			putchar(BELL);
			break;

		case	FN6:
			putchar(BELL);
			break;

		case	FN7:
			putchar(BELL);
			break;

		case	FN8:
			putchar (BELL);
			break;

		case	'!':
			putchar (BELL);
			break;

		case	FN3:
			LCL_draw_menu ();
			last = 0;
			break;

		case	8:
			/*-------------------------------
			| Keying the option number	|
			-------------------------------*/
			if (nindx)
			{
				num_str[nindx--] = '\0';
				if (!valid_opt(atoi(num_str)))
					putchar(BELL);
				else
					curr = atoi(num_str);
				break;
			}

			/*-------------------------------
			| Keying for Fast Access	|
			-------------------------------*/
			if (findx)
			{
				fast_str[--findx] = ' ';
				break;
			}

		case	FN15:
		case	UP_KEY:
		case	11:

		case	LEFT_KEY:
			strcpy(fast_str,"      ");
			findx = 0;
			num_str[0] = '\0';
			nindx = 0;
			if (--curr == 0)
				curr = max_opt - 1;
			break;

		case	FN13:
		case	FN14:
		case	DOWN_KEY:
		case	10:
		case	' ':

		case	RIGHT_KEY:
		case	12:
			strcpy(fast_str,"      ");
			findx = 0;
			num_str[0] = '\0';
			nindx = 0;
			if (++curr == max_opt)
				curr = 1;
			break;

		case	'\r':
			if (valid_opt(last))
				rv_pr(option[last]._name,option[last]._col,option[last]._row,0);

			/*---------------
			| Fast Access	|
			---------------*/
			if (findx)
			{
				strcpy(old_menu,currentMenu);

				curr = get_fast(fast_str);
				strcpy(fast_str,"        ");
				findx = 0;
				nindx = 0;
				num_str[0] = '\0';
				/*-----------------------
				| Fast Access Failed	|
				-----------------------*/
				if (curr <= 0)
				{
					curr = last;
					last = -1;
					putchar(BELL);
					break;
				}

				/*---------------
				| Load New Menu	|
				---------------*/
				if (strcmp (currentMenu, old_menu))
				{
					last = 0;
					_load_menu (currentMenu);
					LCL_draw_menu ();
				}
			}
			else
				return (curr);
			break;

		case	FN17:
			putchar(BELL);
			break;

		case	FN32:
			/*_exit_prog(" *** Menu System Now Exited *** ");*/
			shutdown_prog (ML (mlMenuMess227));
			exit (0);

		case	-1:
			break;

		default:
			putchar(BELL);
		}
	}
}

void
_load_menu (
 char *	mname)
{
	register int	i;
	int		dflt_used = 1;
	int		valid;
	int		menu_error;
	char	delimit [3];
	char	secure [2049];
	char *	tptr;
	char *	sptr = getenv ("PROG_PATH");
	char	filename [101];

	for (i = 0;i < max_opt;i++)
	{
		free (option [i]._name);
		free (option [i]._program);
	}

	for (i = 0;i < MAX_EXTRA;i++)
		quads [i]._off = 0;

	max_opt = 0;
	max_quad = 0;
	max_box = 0;
	max_line = 0;
	max_graph = 0;
	max_comment = 0;

	sprintf(filename,"%s/BIN/%s",(sptr != (char *)0) ? sptr : "/usr/LS10.5",mname);

	if ((fmenu = fopen(filename,"r")) == 0)
	{
		crsr_on();
		sprintf(err_str,"Error in %s during (FOPEN)",mname);
		sys_err(err_str,errno,PNAME);
	}

	sprintf (delimit,"()");

	sptr = fgets(data,DATA_SIZE,fmenu);

	while (max_opt < MAX_OPT && sptr != (char *)0 && strncmp(sptr,"((",2))
	{
		valid = 0;
		*(sptr + strlen(sptr) - 1) = '\0';
		tptr = strchr (sptr,delimit[0]);
		if (tptr != (char *)0)
		{
			if (max_opt != 0)
			{
				*tptr = '\0';
				option[max_opt]._name = strdup (clip(sptr));
			}

			sptr = tptr + 1;
			tptr = strchr (sptr,delimit[1]);
			if (tptr != (char *)0)
			{
				if (max_opt != 0)
				{
					*tptr = '\0';
					sprintf(secure,"%-.2048s",sptr);
				}

				valid = 1;

				/*-----------------------
				| Look for Quadrant|
				-----------------------*/
				sptr = tptr + 1;
				tptr = strchr (sptr,'(');
				if (tptr != (char *)0)
				{
					if (max_opt == 0)
					{
						*tptr = '\0';
						option[max_opt]._name = strdup (clip(data));
					}

					/*-----------------------
					| Look for Offset.	|
					-----------------------*/
					sptr = tptr + 1;
					option[max_opt]._col = atoi(sptr);
					tptr = strchr (sptr,',');
					if (tptr != (char *)0)
					{
						sptr = tptr + 1;
						option[max_opt]._row = atoi(sptr);
						dflt_used = 0;
					}
					else
					{
						option[max_opt]._col = 1;
						option[max_opt]._row = 1;
					}
				}
				else
				{
					if (max_opt == 0)
						option[max_opt]._name = strdup (clip(data));

					option[max_opt]._col = 1;
					option[max_opt]._row = 1;
				}
			}
			else
			{
				sprintf(secure,"%-.2048s",sptr);
				valid = 1;
				option[max_opt]._col = 1;
				option[max_opt]._row = 1;
			}

			downshift(secure);
		}

		sprintf(delimit,"<>");

		/*-------------------------------
		| Got security for the line	|
		| now check it ...		|
		| but don't bother for title	|
		-------------------------------*/
		if (valid && max_opt != 0)
			valid = Chk_security (secure, security);

		/*-------------------------------
		| Skip the next line		|
		| as user has no access to line	|
		-------------------------------*/
		if (!valid)
		{
			sptr = fgets(data,DATA_SIZE,fmenu);
			if (sptr != (char *)0)
				sptr = fgets(data,DATA_SIZE,fmenu);
			continue;
		}
	
		sptr = fgets(data,DATA_SIZE,fmenu);

		if (sptr != (char *)0)
		{
			*(sptr + strlen(sptr) - 1) = '\0';
			option[max_opt++]._program = strdup (sptr);
			if (!strncmp(sptr,"menu ",5))
			{
				max_opt--;
				option[max_opt]._col = 1;
				option[max_opt]._row = 1;
			}
			sptr = fgets(data,DATA_SIZE,fmenu);
		}
	}
	fclose(fmenu);

	if (max_opt < 2)
	{
		/*_exit_prog(" User has no options on menu ");*/
		shutdown_prog (ML (mlMenuMess270));
		exit (0);
	}

	for (i = 0;i < max_opt;i++)
	{
		option[i]._col = 1;
		option[i]._row = 1;
	}

	min_col = MIN_COL;
	min_row = MIN_ROW;
	max_col = -1;
	max_row = -1;

	quads[0]._col = min_col + 6;
	quads[0]._row = min_row + 3;
	quads[0]._off = 0;

	max_quad = 1;
	menu_error = _set_coord ();
}

/*=======================================
| Set the coordinates for those options	|
| requiring them.			|
=======================================*/
int
_set_coord (
 void)
{
	register int	i;
	int		width;
	int		centre;
	int		offset;
	int		length;
	int		indx;

	/*-------------------------------
	| Not Default && Not Window	|
	| so calc max_col		|
	-------------------------------*/
	if (max_col == -1)
	{
		for (i = 0;i < max_opt;i++)
		{
			indx = option[i]._col - 1;
			if (indx < 0 || indx >= max_quad)
				indx = 0;

			if (quads[indx]._col + (int) strlen(option[i]._name) + 2 > max_col)
				max_col = quads[indx]._col + strlen(option[i]._name) + 2;
		}
	}

	/*-----------------------
	| For Every Option	|
	-----------------------*/
	for (i=0; i<max_opt; i++)
	{
		/*-----------------------
		| Menu Heading Line	|
		-----------------------*/
		if (i == 0)
		{
			length = strlen(option[0]._name);
			width = max_col - min_col;
			centre = max_col - width / 2;
			offset = centre - length / 2;
			option[0]._col = offset;
			option[0]._row = min_row + 1;

			if (option[0]._col < 0)
				option[0]._col = 1;

			if (option[0]._row < 0)
				option[0]._row = 1;
		}
		else
		{
			/*---------------------------------------
			| Decide Which Quadrant - deflt = 1	|
			---------------------------------------*/
			indx = option[i]._col - 1;
			if (indx < 0 || indx >= max_quad)
				indx = 0;

			offset = option[i]._row;

			option[i]._col = quads[indx]._col;
			option[i]._row = quads[indx]._row + quads[indx]._off;
			quads[indx]._off += offset;

			if (quads[indx]._off + quads[indx]._row > max_row)
				max_row = quads[indx]._off + quads[indx]._row;
		}
	}

	if (min_row < MIN_ROW)
		return (EXIT_FAILURE);

	if (min_col < MIN_COL)
		return (EXIT_FAILURE);

	if (max_row > MAX_ROW)
		return (EXIT_FAILURE);

	if (max_col > MAX_COL)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
_substit (
 char *	str)
{
	int	len;
	int	indx = 0;
	char	*vptr;
	char	*sptr;
	char	tmp [200];

	/*-------------------------------------------------------
	| Go Thru All Variables which can be substituted for	|
	| checking whether they occur in the current string	|
	-------------------------------------------------------*/
	while (strlen(com[indx].lbl))
	{
		sptr = str;
		len = strlen(com[indx].lbl);
		/*---------------------------------------
		| Check Whole string for Variable	|
		---------------------------------------*/
		while (*sptr)
		{
			/*-----------------------------------------------
			| Have a variable which requires substitution	|
			-----------------------------------------------*/

			if (strncmp(sptr,com[indx].lbl,len)==0)
			{
				vptr = com[indx].val;
				/*---------------------------------------
				| Copy Balance of string into temp	|
				---------------------------------------*/
				strcpy(tmp,sptr + len);
				/*-------------------------------
				| Copy Subs. onto string	|
				-------------------------------*/
				while ((*sptr++ = *vptr++));
				/*-------------------------------------------
				| Concatenate balance of string onto string |
				-------------------------------------------*/
				strcat(str,tmp);
			}
			++sptr;
		}
		indx++;
	}
}

void
_run_menu (
 int line_no)
{
	char	cmd [200];
	char	lp_str [3];
	int		pno = 1;
	int		len;
	int		i;
	int		offset = 0;
	int		_background = 0;
	int		_lpno = 0;
	int		_lp_chain = 0;

	alarm (0);

	UserAccountAdd (option [line_no]._name);

	strcpy (cmd, option [line_no]._program);

	/*--------------------------------
	| Do command line substitutions. |
	--------------------------------*/
	_substit (cmd); 
	len = strlen (cmd);

	/*-----------------------------------------
	| Chop up cmd line into separate strings. | 
 	-----------------------------------------*/
	for (i = 0; i < len; i++)
		if (cmd [i] == '~')
			cmd [i] = '\0';

	/*---------------------------------------------------------------------
 	| Check for special strings in cmd line and substitute appro values.  |
	---------------------------------------------------------------------*/
	i = 0;
	do
	{
		*(arg + i) = cmd + offset;
		offset += (strlen(arg[i]) + 1);

		if (!strcmp(arg[i],"LOGNAME"))
			arg[i] = curr_user;

		if (!strcmp(arg[i],"logout"))
		{
			/*_exit_prog(" *** Menu System Now Exited *** ");*/
			shutdown_prog (ML (mlMenuMess227));
			exit (0);
		}

		if (!strcmp(arg[i],"BACK"))
			_background = 1;
		else
			if (!strcmp(arg[i],"LPNO"))
				_lpno = i;

		if (!strcmp(arg[i],"lp_chain"))
			_lp_chain = 1;
	}

	while (strlen(arg[i++]));

	*(arg + (i - 1)) = (char *)0; /* Put NULL into last array element */

	if (!_background && !_lpno)
		_pr_busy (!_lp_chain);
		
	set_tty ();
	crsr_on ();
	if (_lpno)
	{
		pno = get_lpno (0);
		sprintf(lp_str,"%d",pno);
		*(arg + _lpno) = lp_str;
		_pr_busy (1);
	}

	if (_background)
	{
		clear ();
		print_at (0, (80 - strlen(option[i]._name)) / 2,option[i]._name);
		_draw_mask ();
		move (19, 11);
		rv_on ();
		/*print_at(0,0,"Return");*/
		print_at(0,0,ML(mlMenuMess070));
		rv_off();
		/*print_at(0,0," for Foreground , ");*/
		print_at(0,0,ML(mlMenuMess071));
		rv_on();
		/*print_at(0,0,"Space");*/
		print_at(0,0,ML(mlMenuMess072));
		rv_off();
		/*print_at(0,0," for Overnight.");*/
		print_at(0,0,ML(mlMenuMess073));
		fflush(stdout);
		/*print_at(12,26,"Any other key for Background.");*/
		print_at(12,26,ML(mlMenuMess074));
		fflush(stdout);
	    	if ((len = getkey()) == '\r')
	    	{
			_background = 0;
			/*rv_pr("Wait Running Program now..",27,14,1);*/
			rv_pr(ML(mlStdMess035),27,14,1);
	    	}
		/*--------------------------------------------------------
		| Change BACK to ONIGHT, Place Program Description to be |
 		| Last Argument and put a null on the end.               |
		--------------------------------------------------------*/
		if (len == ' ')
		{
			*(arg) = "ONIGHT";
			*(arg + (i - 1)) = option[line_no]._name;
			*(arg + (i)) = (char *) 0;
		}
	}

	rset_tty ();

	if (fork () == 0)
	{
		if (_background)
		{
			if (fork () == 0)
			{
				signal(SIGINT,SIG_IGN);
				execvp(arg[0],arg);
				/*execlp("no_option","no_option",(char *)0);*/
				execlp (ML (mlMenuMess269), ML (mlMenuMess269), (char *)0);
			}
			else
				exit (0);
		}
		else
		{
			execvp (arg [0],arg);
			/*execlp("no_option","no_option",(char *)0);*/
			execlp (ML (mlMenuMess269), ML (mlMenuMess269), (char *)0);
		}
	}
	else
		wait (&status);

	set_tty ();
	crsr_off ();
	_get_comm ();
}

/*===============================
| Action on trapping interupt	|
===============================*/
void
_psl_kill (
 int x)
{
	int	pid;

	signal (SIGINT,_psl_kill);

	if ((pid = wait (&status)) == -1)
		putchar (BELL);
	else
	{
		clear ();
		gr_off ();
		move (8, 12);
		rv_on ();
		print_at (0, 0, "DEL");
		rv_off ();
		/*print_at(0, 0, " Key Pressed Proccess %d ",pid);
		print_at(0, 0, "Killed , returning to Menu");*/

		print_at (0, 0, ML (mlMenuMess076), pid);
		print_at (0, 0, ML (mlMenuMess157));
	}
}

void
_pr_busy (
 int cl_screen)
{
	if (cl_screen)
		clear();
	else
		move(0,0);

	/*print_at(0,0,"Busy ... ");*/
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff access permitted		|
=======================================*/
int
Chk_security (
 char *_secure,		/* security on menu		*/
 char *_security)	/* security on user		*/
{
	char *	sptr;
	char *	tptr;
	char *	uptr;
	char *	vptr;
	char	tmp_mnu_sec [9];
	char	tmp_usr_sec [9];
	char	usr_char;
	char	mnu_char;

	/*---------------------------------------
	| Super User Access on users security	|
	---------------------------------------*/
	if ((sptr = strchr(_security, '*')))
		return (EXIT_FAILURE);
	
	/*-------------------------------
	| Access to all on menu option	|
	-------------------------------*/
	if ((sptr = strchr(_secure, '*')))
		return (EXIT_FAILURE);
	
	/*-----------------------------------------------
	| Check Security for each security group	|
	| that user belongs to.				|
	-----------------------------------------------*/	
	sptr = strdup (_security);
	while (*sptr)
	{
		/*----------------
		| Find separator |
		----------------*/
		tptr = sptr;
		while (*tptr && *tptr != '|')
			tptr++;

		usr_char = *tptr;

		*tptr = '\0';
		strcpy (tmp_usr_sec, sptr);

		if (usr_char)
			sptr = tptr + 1;
		else
			*sptr = '\0';

		uptr = strdup (_secure);
		while (*uptr)
		{
			/*----------------
			| Find separator |
			----------------*/
			vptr = uptr;
			while (*vptr && *vptr != '|')
				vptr++;

			mnu_char = *vptr;

			*vptr = '\0';
			strcpy(tmp_mnu_sec, uptr);

			if (mnu_char)
				uptr = vptr + 1;
			else
				*uptr = '\0';

			if (!strcmp(tmp_usr_sec, tmp_mnu_sec))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

int
heading (
 void)
{
	return (EXIT_SUCCESS);
}

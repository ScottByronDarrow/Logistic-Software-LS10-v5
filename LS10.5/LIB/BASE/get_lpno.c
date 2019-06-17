/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( get_lpno.h     )                                 |
|  Program Desc  : ( Get printer number -  search from            )   |
|                  ( MENUSYS/prntype.                             )   |
|                  ( Also, validate against LP_SECURE.            )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 17/11/87         |
|---------------------------------------------------------------------|
|  Date Modified : (17/11/87)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (06/11/90)      | Modified  by : Trevor van Bremen |
|  Date Modified : (02/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (20.12.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (15/11/95)      | Modified by : Scott B Darrow.    |
|                                                                     |
|  Comments      : (06/11/90) Instigated printer security routines.   |
|                :                                                    |
|  (02/03/92)    : Added valid_lp() routine which checks to see if    |
|                : user has access to the logical printer no passed.  |
|  (11/05/93)    : Made changes to shut up lint                       |
|  (20.12.94)    : Fixed bug introduced during move from header       |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

#define	REDRAW	FN3
#define	EOI	2025

#define	BOX_SIZE	10
#define	MAX_PAGES	50

int		lp_x_off	= 0;
int		lp_y_off	= 0;

static struct
{
	char	_sno[4];			/* On-screen selection number */
	char	_pno[4];			/* Actual PSL printer number */
	char	_pdesc[61];			/* Actual printer description */
} _lpno_rec[BOX_SIZE];

static long	_lpno_page	[MAX_PAGES];
static int	_act_pno	[MAX_PAGES];

static FILE	*_prntype = NULL;
static FILE	*lp_secure = NULL;

/*
 *	Globals
 */
extern char	*PNAME,
			temp_str [];

/*
 *	Function declarations
 */
static int	_lp_chk_user (char *),
			_load_lp (int, int, int *),
			_load_lpno (int, int, int);
static void	_print_box (int);

static void
_lp_opens(void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	filename [80];

	/*---------------------------------------
	| Firstly, open up the old prntype file |
	---------------------------------------*/
	sprintf (filename, "%s/BIN/MENUSYS/prntype", sptr);
	if (!(_prntype = fopen (filename, "r")))
	{
		sprintf (filename,
			"Error in %s/BIN/MENUSYS/prntype during (FOPEN)",
			sptr);
		sys_err (filename, errno, PNAME);
	}

	/*------------------------------------------
	| Secondly, open up the new LP_SECURE file |
	------------------------------------------*/
	sprintf (filename, "%s/BIN/MENUSYS/LP_SECURE", sptr);
	if (!(lp_secure = fopen (filename, "r")))
	{
		sprintf (filename,
			"Error in %s/BIN/MENUSYS/LP_SECURE During (FOPEN)",
			sptr);
		sys_err (filename, errno, PNAME);
	}
}

static void
_lp_closes(void)
{
	fclose (_prntype);
	fclose (lp_secure);
}

/*===============================================================
| Validates the logical printer number passed to see if user    |
| has access to that printer. Returns :                         |
|	TRUE    - If user has access to printer.                |
|	FALSE	- If user does NOT have access to printer.      |
===============================================================*/
int
valid_lp(int chk_lpno)
{
	int		act_line;
	char	*sptr;
	char	a_line[80];

	/*----------------------------
	| Open prntype and LP_SECURE |
	----------------------------*/
	_lp_opens ();

	act_line = 1;
	/*--------------------------------
	| Get printer entry from prntype |
	--------------------------------*/
	sptr = fgets (a_line, 80, _prntype);
	while ( sptr && !feof(_prntype) && act_line < chk_lpno)
	{
		act_line++;
		sptr = fgets (a_line, 80, _prntype);
	}

	/*---------------
	| Check printer |
	---------------*/
	if ( act_line == chk_lpno && sptr && !feof(_prntype) )
	{
		/*-------------------------------
		| Printer OK for user to access |
		-------------------------------*/
		if ( _lp_chk_user(sptr) )
		{
			/*-----------------------------
			| Close prntype and LP_SECURE |
			-----------------------------*/
			_lp_closes ();
			return(TRUE);
		}
	}

	/*-----------------------------
	| Close prntype and LP_SECURE |
	-----------------------------*/
	_lp_closes ();

	/*-----------------------------------
	| Printer NOT OK for user to access |
	-----------------------------------*/
	return (FALSE);
}

/*===============================================================
| allows the selection of a printer from a window displaying 10	|
| the display pages forward & back. It also has a search	|
| facility.							|
===============================================================*/
/*ARGSUSED*/
int
get_lpno (
 int	dflt_lpno)				/* Ignored!! TvB	*/
{
	int	c;
	int	lines = 0;
	int	p_no = 0;
	int	curr_page;
	int	p_size;
	int	max_lines = 0;
	int	act_line = 0;
	int	max_width = 0;
	int	curr_line;
	int	next_line;
	char	*sptr;
	char	a_line[80];

	_lp_opens ();

	_lpno_page[0] = 0L;
	_act_pno[0] = 0;

	act_line = max_lines = 0;
	while ((sptr = fgets (a_line, sizeof (a_line), _prntype)))
	{
		act_line++;
		if (_lp_chk_user (sptr))
		{
			int	linelen = strlen (sptr);

			max_lines++;
			if (!(max_lines % BOX_SIZE))
			{
				/*
				 *	Mark down new page's offsets
				 */
				int	idx = max_lines / BOX_SIZE;

				if (idx >= MAX_PAGES)
				{
					sys_err (
						"Overflowed maximum number of printers",
						-1,
						"get_lpno.c");
				}

				_lpno_page [idx] = ftell (_prntype);
				_act_pno [idx] = act_line;
			}
			if (linelen > max_width)
				max_width = linelen;
		}
	}

	crsr_off ();

	_print_box (max_width);

	p_size = _load_lpno (max_lines, max_width, p_no);
	rv_pr (_lpno_rec[lines]._pdesc, 6 + lp_x_off, lines + 3 + lp_y_off, 1);

	/*------------------------------------------------------------
	| No Lines where found return to program and reset temp_str. |
	------------------------------------------------------------*/
	if (p_size == 0)
	{
		rv_pr (" No entries in MENUSYS/prntype. ", 0 + lp_x_off, 0 + lp_y_off, 1);
		fflush (stdout);
		sleep (2);
		crsr_on ();
		_lp_closes ();
		return (-1);
	}

	/*--------------------
	| Control movement . |
	--------------------*/
	while ((c = getkey ()))
	{
		rv_pr (_lpno_rec[lines]._pdesc, 6 + lp_x_off, lines + 3 + lp_y_off, 0);
		curr_page = p_no;

		switch (c)
		{
		case REDRAW:
			_print_box (max_width);
			curr_page = -1;
			break;
			
		case FN15:
		case LEFT_KEY:
		case 8:
		case 9:
			lines = (p_no == 0) ? (max_lines - 1) % BOX_SIZE : BOX_SIZE - 1;
			p_no = (p_no == 0) ? (max_lines - 1) / BOX_SIZE : p_no - 1;
			break;

		case UP_KEY:
		case 11:
			if (lines == 0) 
			{
				if (max_lines <= BOX_SIZE)
					lines = max_lines - 1;
				else
				{
					lines = (p_no == 0) ? (max_lines - 1) % BOX_SIZE : BOX_SIZE - 1;
					p_no = (p_no == 0) ? (max_lines - 1) / BOX_SIZE : p_no - 1;
				}
			}
			else
				lines--;
			break;

		case ' ':
		case DOWN_KEY:
		case 10:
			if (lines == p_size - 1) 
			{
				if (max_lines > BOX_SIZE)
					p_no = (p_no == max_lines / BOX_SIZE) ? 0 : p_no + 1;
				lines = 0;
			}
			else
				lines++;
			break;

		case FN14:
		case RIGHT_KEY:
		case 12:
			if (max_lines != BOX_SIZE)
				p_no = (p_no == (max_lines - 1) / BOX_SIZE) ? 0 : p_no + 1;
			lines = 0;
			break;

		case EOI :
		case '\r':
			crsr_on ();
			sprintf (temp_str, "%d", atoi (_lpno_rec[lines]._pno));
			_lp_closes ();
			return (atoi (_lpno_rec[lines]._pno));

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			/*-----------------------------------------------
			| find where the next occurance of 'c' is	|
			-----------------------------------------------*/
			if (p_no == 0 && lines <= (c - 49))
				next_line = c - 49;
			else
				next_line = (c - 48) * 10 - 1;

			if (next_line >= max_lines && max_lines < 10)
			{
				if ((c - 49) >= max_lines)
					putchar (BELL);
				else
				{
					p_no = 0;
					lines = c - 49;
				}
				break;
			}

			/*-------------------------------
			| find current offset in file	|
			-------------------------------*/
			curr_line = (p_no * BOX_SIZE + lines);

			/*---------------------------------------
			| check if next is in current page	|
			---------------------------------------*/
			if (lines < 9 && _lpno_rec[lines + 1]._sno[0] == c)
			{
				lines++;
				break;
			}

			if (curr_line > next_line || next_line >= max_lines)
			{
				p_no = 0;
				lines = c - 49;
				break;
			}

			if (curr_line <= next_line)
			{
				p_no = next_line / BOX_SIZE;
				lines = next_line % BOX_SIZE;
				break;
			}
			break;

		default:
			putchar (BELL);
			break;
		}

		if (p_no != curr_page)
			p_size = _load_lpno (max_lines, max_width, p_no);
		rv_pr (_lpno_rec[lines]._pdesc, 6 + lp_x_off, lines + 3 + lp_y_off, 1);
	}
	_lp_closes ();
	return (EXIT_SUCCESS);
}

static void
_print_box (
 int	p_width)
{
	int	i;
	int	box_width = (p_width > 34) ? p_width : 34;

	for (i = 0; i < BOX_SIZE + 4; i++)
	{
		move (0 + lp_x_off, i + lp_y_off);
		printf ("%-*.*s", box_width + 2, box_width + 2, " ");
	}
	box (0 + lp_x_off, 0 + lp_y_off, box_width + 2, BOX_SIZE + 4);
	move (1 + lp_x_off, 1 + lp_y_off);
	printf ("%-*.*s", box_width, box_width, ML (" Lpno  Printer Description."));
	move (1 + lp_x_off, 2 + lp_y_off);
	line (box_width + 1);
	move (1 + lp_x_off, 14 + lp_y_off);
	printf ("%-*.*s", box_width, box_width, ML (" Select Printer & Press Return."));
	move (1 + lp_x_off, 13 + lp_y_off);
	line (box_width + 1);
	fflush (stdout);
}

/*===============================
| Load one page from work file. |
===============================*/
static int
_load_lpno (
 int	max_lines,
 int	max_width,
 int	p_no)
{
	int	p_size;
	int	box_width = (max_width > 30) ? max_width : 30;
	int	i = 0;
	int	cur_pno;

	if (max_lines == 0)
		return (EXIT_SUCCESS);

	p_size = (p_no == (max_lines / BOX_SIZE)) ? max_lines % BOX_SIZE : BOX_SIZE;

	if (!p_size)
		return (EXIT_SUCCESS);

	fseek (_prntype, _lpno_page[p_no], 0);
	cur_pno = _act_pno[p_no];
	for (i = 0; i < p_size && _load_lp (i, p_no, &cur_pno); i++);

	for (i = 0; i < BOX_SIZE; i++)
	{
		move (1 + lp_x_off, i + 3 + lp_y_off);
		if (i < p_size)
			printf (" %s %-*.*s",
				_lpno_rec[i]._sno,
				box_width - 7,
				box_width - 7,
				_lpno_rec[i]._pdesc);
		else
		{
			strcpy (_lpno_rec[i]._pno, "  ");
			strcpy (_lpno_rec[i]._sno, "  ");
			printf ("%*.*s", box_width - 2, box_width - 2, " ");
		}
	}

	return (p_size);
}

/*===============================
| Load one line from work file. |
===============================*/
static int
_load_lp (
 int	line_no,
 int	p_no,
 int	*cur_pno)
{
	char	*lptr;
	char	a_line[80];

	while (TRUE)
	{
		lptr = fgets (a_line, 80, _prntype);
		if (lptr == (char *)0 || feof (_prntype))
			return (FALSE);
		(*cur_pno)++;
		if (_lp_chk_user (lptr))
			break;
	}

	*(lptr + strlen (lptr) - 1) = (char) NULL;
	lptr = a_line;

	while (*lptr && *lptr != '\t')
		lptr++;
	if (*lptr)
		lptr++;
	while (*lptr && *lptr != '\t')
		lptr++;
	if (*lptr)
		lptr++;

	sprintf (_lpno_rec[line_no]._sno, "%2d.", BOX_SIZE * p_no + line_no + 1);
	sprintf (_lpno_rec[line_no]._pno, "%2d.", *(cur_pno));
	sprintf (_lpno_rec[line_no]._pdesc, "%-.60s", (*lptr) ? lptr : "No Description");
	return (TRUE);
}

/*------------------------------
| Check to see if a printer is |
| valid for the current user.  |
------------------------------*/
static int
_lp_chk_user (
 char	*record)
{
	char	*lptr = getenv ("LOGNAME");
	char	prn_type [80],
			queue_name [80];
	char	a_line [80];

	/*
	 *	Read off the printer type and queue name
	 */
	if (sscanf (record, "%s %s", prn_type, queue_name) != 2)
		return (FALSE);

	fseek (lp_secure, 0L, 0);
	while (fgets (a_line, sizeof (a_line), lp_secure))
	{
		char	l_name [80],
				l_type [80],
				l_queue [80];

		/*--------------------------------------------
		| Split line into user name, prntype & queue |
		--------------------------------------------*/
		if (sscanf (a_line, "%s %s %s", l_name, l_type, l_queue) != 3)
			continue;

		/*----------------------------------
		| Compare field 1 against $LOGNAME |
		----------------------------------*/
		if (l_name [0] == '*' || !strcmp (l_name, lptr))
		{
			/*----------------------------
			| Now, check prntype & queue |
			----------------------------*/
			if (l_type [0] == '*' || !strcmp (l_type, prn_type))
				if (l_queue [0] == '*' || !strcmp (l_queue, queue_name))
					return (TRUE);
		}
	}
	return (FALSE);
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( dsp_utils.c    )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10.11.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (22/02/95)      | Modified by : Basil Wood         |
|  Date Modified : (19/08/95)      | Modified by : Scott B Darrow.    |
|                                                                     |
|  Comments                                                           |
|  (10.11.94)    : Dsp_print () argument kludged.                     |
|  (22/02/95)    : Add Dsp_srch_fn2()                                 |
|                : Increase sizeof Dsp_print() buffer                 |
|  (03/03/95)    : Add option to highlight two lines (Dsp_srch_fn2)   |
|  (19/08/95)    : Updated to add page number.                        |
-----------------------------------------------------------------------
	$Log: dsp_utils.c,v $
	Revision 5.3  2002/05/02 01:24:44  scott
	Updated to include .CARET_ON to ensure reports from dsp utilities are printed correctly.
	
	Revision 5.2  2001/08/06 22:40:54  scott
	RELEASE 5.0
	
	Revision 5.1  2001/07/25 00:43:28  scott
	New library for 10.5
	
	Revision 5.0  2001/06/19 06:59:15  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.1  2001/03/23 05:44:51  scott
	Updated to use print_mess instead of sending printing message using print_at.
	
	Revision 4.0  2001/03/09 00:52:36  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.1  2000/12/04 01:18:57  scott
	Updated to fix minor warnings on HP box.
	
	Revision 3.0  2000/10/12 13:34:20  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.1  2000/08/09 04:04:00  scott
	Updated to increase number of pages from 2000 to 10000.
	Sounds like a lot but only effects the define of a long.
	
	Revision 2.0  2000/07/15 07:17:13  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.7  1999/10/01 07:24:58  scott
	Ansi Project
	
	Revision 1.6  1999/09/30 04:38:17  scott
	Updated for date routines.
	
	Revision 1.5  1999/09/29 00:02:04  jonc
	Corrected "long" vs "time_t" usage; very apparent on AIX and Alpha.
	
=====================================================================*/
#include	<std_decs.h>

#define	PAGES	10000
#define	PSIZE	20
#define	TIMEOUT	5

static	int	win_indx = 0;
extern	int	max_work;
extern	char	err_str[];
extern	char	*PROG_VERSION;
extern	char	*PNAME;
int		DSP_UTILS_lpno	=	0;

/*
 *	Function declarations
 */
extern char	*clip (char *);

static int __Dsp_srch_fn (int, int (*) (char *), int *, int *, int *, int);

	
struct	_dsp_type	{
	int		_indx;
	int		_left_col;		/* top left corner		*/
	int		_top_row;		/* of display box		*/
	int		_top_line;		/* start of _page_length lines	*/
	int		_page_length;	/* number of lines to display	*/
	int		_page_width;	/* internal width of display box*/
	int		_graphs;
	int		_extras;
	long	_disp_page[PAGES];
	char	*_header[3];
	FILE	*_work_file;
	struct	_scn_type	{
		char	*_dsp;
		char	*_key;
	} _scn[PSIZE];
	char	*_text;
	char	*_co_no;
	char	*_co_name;
	char	*_br_no;
	char	*_br_name;
	char	*_wh_no;
	char	*_wh_name;
	int		_prn_ok;
	int		_prn_only;
	int		_pre_clear;
	int		_max_work;
	struct	_dsp_type	*_next;
};

struct	_dsp_type	*_dsp_head;
struct	_dsp_type	*_dsp_free;

#define	D_NUL	(struct	_dsp_type *)0

#define	_INDX			_dsp_head->_indx
#define	_LEFT_COL		_dsp_head->_left_col
#define	_TOP_ROW		_dsp_head->_top_row
#define	_TOP_LINE		_dsp_head->_top_line
#define	_PAGE_LENGTH	_dsp_head->_page_length
#define	_PAGE_WIDTH		_dsp_head->_page_width
#define	_GRAPHS			_dsp_head->_graphs
#define	_EXTRAS			_dsp_head->_extras
#define	_DISP_PAGE(x)	_dsp_head->_disp_page[x]
#define	_HEADER(x)		_dsp_head->_header[x]
#define	_DSP(x)			_dsp_head->_scn[x]._dsp
#define	_KEY(x)			_dsp_head->_scn[x]._key
#define	_WORK_FILE		_dsp_head->_work_file
#define	_TEXT			_dsp_head->_text
#define	_CO_NO			_dsp_head->_co_no
#define	_CO_NAME		_dsp_head->_co_name
#define	_BR_NO			_dsp_head->_br_no
#define	_BR_NAME		_dsp_head->_br_name
#define	_WH_NO			_dsp_head->_wh_no
#define	_WH_NAME		_dsp_head->_wh_name
#define	_PRN_OK			_dsp_head->_prn_ok
#define	_PRN_ONLY		_dsp_head->_prn_only
#define	_PRE_CLEAR		_dsp_head->_pre_clear
#define	_MAX_WORK		_dsp_head->_max_work

#define	X_OFF	_dsp_head->_left_col
#define	Y_OFF	_dsp_head->_top_row

void
Dsp_nd_prn_open 
(
	int		left_col, 
	int 	top_row, 
	int 	page_size, 
	char 	*text, 
	char 	*co_no, 
	char 	*co_name, 
	char 	*br_no, 
	char 	*br_name, 
	char 	*wh_no, 
	char 	*wh_name)
{
	_Dsp_open 
	(
		left_col, 
		top_row, 
		page_size, 
		text, 
		co_no, 
		co_name, 
		br_no, 
		br_name, 
		wh_no, 
		wh_name, 
		TRUE, 
		TRUE, 
		TRUE
	);
}

void
Dsp_nc_prn_open 
(
	int		left_col, 
	int		top_row, 
	int		page_size, 
	char 	*text, 
	char 	*co_no, 
	char 	*co_name, 
	char 	*br_no, 
	char 	*br_name, 
	char 	*wh_no, 
	char 	*wh_name)
{
	_Dsp_open
	(
		left_col, 
		top_row, 
		page_size, 
		text, 
		co_no, 
		co_name, 
		br_no, 
		br_name, 
		wh_no, 
		wh_name, 
		TRUE, 
		FALSE, 
		FALSE
	);
}

void
Dsp_prn_open 
(	
	int		left_col, 
	int 	top_row, 
	int 	page_size, 
	char 	*text, 
	char 	*co_no, 
	char 	*co_name, 
	char 	*br_no, 
	char 	*br_name, 
	char 	*wh_no, 
	char 	*wh_name)
{
	_Dsp_open
	(
		left_col, 
		top_row, 
		page_size, 
		text, 
		co_no, 
		co_name, 
		br_no, 
		br_name, 
		wh_no, 
		wh_name, 
		TRUE, 
		FALSE, 
		TRUE
	);
}

void
Dsp_nc_open
(
	int		left_col, 
	int 	top_row, 
	int 	page_size)
{
	_Dsp_open 
	(
		left_col, 
		top_row, 
		page_size, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		FALSE, 
		FALSE, 
		FALSE
	);
}

void
Dsp_open 
(
	int		left_col, 
	int		top_row, 
	int		page_size)
{
	_Dsp_open 
	(
		left_col, 
		top_row, 
		page_size, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		FALSE, 
		FALSE, 
		TRUE
	);
}

void
_Dsp_open
(
	int 	left_col, 
	int 	top_row, 
	int 	page_size, 
	char 	*text, 
	char 	*co_no, 
	char 	*co_name, 
	char 	*br_no, 
	char 	*br_name, 
	char 	*wh_no, 
	char 	*wh_name, 
	int 	prn_ok, 
	int 	prn_only, 
	int		pre_clear)
{
	register	int	i;
	char	*sptr = getenv ("PROG_PATH");
	char	work_name[101];
	struct	_dsp_type	*_dsp_alloc (void);
	struct	_dsp_type	*dptr;

	if (win_indx == 0)
	{
		_dsp_head = D_NUL;
		_dsp_free = D_NUL;
	}

	dptr = _dsp_alloc ();
	if (dptr == D_NUL)
		sys_err ("Error in dsp_open() during (MALLOC)",12,PNAME);

	if (_dsp_head == D_NUL)
	{
		_dsp_head = dptr;
		_dsp_head->_next = D_NUL;
	}
	else
	{
		_MAX_WORK = max_work;
		dptr->_next = _dsp_head;
		_dsp_head = dptr;
	}

	_INDX = win_indx++;
	_GRAPHS = -1;
	_EXTRAS = 0;

	_PRN_OK = prn_ok;
	_PRN_ONLY = prn_only;
	_PRE_CLEAR = pre_clear;

	_TEXT =		(text) ? strdup (text) : NULL;
	_CO_NO =	(co_no) ? strdup (co_no) : NULL;
	_CO_NAME =	(co_name) ? strdup (co_name) : NULL;
	_BR_NO =	(br_no) ? strdup (br_no) : NULL;
	_BR_NAME =	(br_name) ? strdup (br_name) : NULL;
	_WH_NO =	(wh_no) ? strdup (wh_no) : NULL;
	_WH_NAME =	(wh_name) ? strdup (wh_name) : NULL;

	sprintf (work_name,"%s/WORK/dsp%06d.%02d",(sptr != (char *)0) ? sptr : "/usr/LS10.5",(int) getpid (),_INDX);
	if ((_WORK_FILE = fopen(work_name,"w")) == 0)
	{
		sprintf (err_str,"Error in %s during (WKOPEN)",work_name);
		sys_err (err_str,errno,PNAME);
	}

	_HEADER(0) = (char *)0;
	_HEADER(1) = (char *)0;
	_HEADER(2) = (char *)0;

	_LEFT_COL = left_col;
	_TOP_ROW = top_row;
	_PAGE_LENGTH = page_size;

	max_work = 0;
	_TOP_LINE = 0;
	_PAGE_WIDTH = 0;

	for (i = 0;i < _PAGE_LENGTH;i++)
	{
		_DSP(i) = (char *)0;
		_KEY(i) = (char *)0;
	}
}

void
Dsp_close (void)
{
	register	int	i;
	char	work_name[101];
	char	*sptr = getenv ("PROG_PATH");
	struct	_dsp_type	*dptr;

	fclose (_WORK_FILE);
	sprintf (work_name,"%s/WORK/dsp%06d.%02d",(sptr != (char *)0) ? sptr : "/usr/LS10.5",(int) getpid (),_INDX);
	unlink(work_name);

	for (i = 0;i < 3;i++)
	{
		if (_HEADER(i) != (char *)0)
		{
			free (_HEADER(i));
			_HEADER(i) = (char *)0;
		}
	}

	if (_TEXT)
		free (_TEXT);
	if (_CO_NO)
		free (_CO_NO);
	if (_CO_NAME)
		free (_CO_NAME);
	if (_BR_NO)
		free (_BR_NO);
	if (_BR_NAME)
		free (_BR_NAME);
	if (_WH_NO)
		free (_WH_NO);
	if (_WH_NAME)
		free (_WH_NAME);

	for (i = 0;i < _PAGE_LENGTH;i++)
	{
		if (_DSP(i) != (char *)0)
		{
			free (_DSP(i));
			_DSP(i) = (char *)0;
		}

		if (_KEY(i) != (char *)0)
		{
			free (_KEY(i));
			_KEY(i) = (char *)0;
		}
	}

	if (_dsp_free == D_NUL)
	{
		_dsp_free = _dsp_head;
		_dsp_head = _dsp_head->_next;
		_dsp_free->_next = D_NUL;
	}
	else
	{
		dptr = _dsp_head;
		_dsp_head = _dsp_head->_next;
		dptr->_next = _dsp_free;
		_dsp_free = dptr;
	}

	win_indx--;
	if (win_indx > 0)
		max_work = _MAX_WORK;
}

int
Dsp_saverec(char *in_str)
{
	return (Dsp_save_fn (in_str,(char *)0));
}

int
Dsp_save_fn (char *in_str, char *_extra)
{
	char	*ML (char *);

	extern	int		MlHide;

	static	int	x;
	static	int	y;
	
	MlHide	=	1;

	if (max_work <= 2)
	{
		if (max_work == 0)
			_PAGE_WIDTH = strlen(in_str);

		if (strlen(in_str))
		{
			if (in_str[0] == '.')
			{
				_HEADER(max_work) = strdup (in_str + 1);
				_PAGE_WIDTH = strlen(in_str + 1);
			}
			else
				_HEADER(max_work) = strdup (ML(in_str));
		}
		max_work++;
		return (EXIT_SUCCESS);
	}

	if (max_work == 3)
	{
		if (!_PRN_ONLY)
		{
			crsr_off ();
			_Print_display ();
		}
		x = (_PAGE_WIDTH - 13) / 2;
		x += _LEFT_COL;
		y = (_PAGE_LENGTH - 1) / 2;
		y += _TOP_LINE;
	}

	if (((max_work - 3) / _PAGE_LENGTH) >= PAGES)
		return (EXIT_FAILURE);

	if ((!_PRN_ONLY) && max_work % 20 == 0)
		rv_pr (" Loading ... ",x,y, (max_work % 40));

	if (((max_work - 3) % _PAGE_LENGTH) == 0)
		_DISP_PAGE ((max_work - 3) / _PAGE_LENGTH) = ftell (_WORK_FILE);

	if (_extra != (char *) 0)
		fprintf (_WORK_FILE, "%s%c%s\n", in_str, 127, _extra);
	else
		fprintf (_WORK_FILE, "%s\n", in_str);
	max_work++;
	return (EXIT_SUCCESS);
}

static void
ShowCursorLines 
(
	int 	line_no, 
	int 	rv, 
	int 	nCursorLines) /* number of lines of cursor */
{
	int i;

	for (i = 0; i < nCursorLines; i++)
		_show_line (line_no + i, rv);
}

int
_null_fn (char *_null_str)
{
	return (EXIT_SUCCESS);
}

int
Dsp_srch (void)
{
	int	dummy_val;

	return (_Dsp_srch_fn (FALSE, _null_fn, &dummy_val, &dummy_val));
}

int
Dsp_srch_fn (
 int	(*_run_fn) (char *))
{
	int dummy_val;

	return __Dsp_srch_fn (FALSE, _run_fn, &dummy_val, &dummy_val, NULL, 1);
}

int
Dsp_srch_fn2 (
 int	(*_run_fn) (char *),
 int *	key_list)
{
	int dummy_val;

	return __Dsp_srch_fn (FALSE, _run_fn, &dummy_val, &dummy_val, key_list, 2);
}

int
Dsp_srch_grph(int *gph_min, int *gph_max)
{
	_GRAPHS = 0;
	return (_Dsp_srch_fn (TRUE, _null_fn, gph_min, gph_max));
}

int
_Dsp_srch_fn (
 int	gph_enable,
 int	(*_run_fn) (char *),
 int *	gph_min,
 int *	gph_max)
{
	return __Dsp_srch_fn (gph_enable, _run_fn, gph_min, gph_max, NULL, 1);
}

static int
__Dsp_srch_fn (
 int	gph_enable,
 int	(*_run_fn) (char *),
 int *	gph_min,
 int *	gph_max,
 int *	key_list,
 int	nCursorLines)
{
	int	c;
	int	max_lines	=	0;
	int	line_no 	=	0;
	int	old_line_no =	-1;
	int	page_no 	=	0;
	int	old_page_no =	-1;
	int	page_size	=	0;
	int	max_page	=	0;
	char	work_name[101];
	char	*sptr = getenv("PROG_PATH");

	if (_PRN_ONLY)
		return (EXIT_SUCCESS);
	max_lines = max_work - 4;
	max_page = max_lines / _PAGE_LENGTH;
	
	if (max_lines < 0)
	{
		Dsp_saverec(" ");
		Dsp_saverec(" ** NO DATA ** ");
		max_lines = 1;
	}

	fclose(_WORK_FILE);
	sprintf (work_name,"%s/WORK/dsp%06d.%02d",(sptr != (char *)0) ? sptr : "/usr/LS10.5",(int) getpid (),_INDX);
	if ((_WORK_FILE = fopen(work_name,"r")) == NULL)
	{
		fclose(_WORK_FILE);
		return (EXIT_FAILURE);
	}

	if (_HEADER(2) == (char *)0)
	{
		page_size = _Load_display(page_no);
		_show_display(line_no);
		crsr_on();
		return (EXIT_SUCCESS);
	}

	while (1)
	{
		if (page_no != old_page_no)
		{
			page_size = _Load_display(page_no);
			if (page_size == -1)
			{
				crsr_on();
				return (EXIT_FAILURE);
			}

			if ((_EXTRAS && _GRAPHS) && line_no != old_line_no)
				line_no = _line_down(-1,page_size);
			_show_display(line_no);
			if ((_EXTRAS && _GRAPHS) && nCursorLines > 1)
				ShowCursorLines (line_no + 1, 1, nCursorLines - 1);
		}
		else
		{
			if ((_EXTRAS && _GRAPHS) && line_no != old_line_no)
			{
				ShowCursorLines (old_line_no, 0, nCursorLines);
				ShowCursorLines (line_no, 1, nCursorLines);
			}
		}
		print_at( _TOP_LINE + _PAGE_LENGTH + 1, _LEFT_COL + 2, "%R Page : %3d ", page_no + 1);

		old_page_no = page_no;
		old_line_no = line_no;

		c = getkey();

		switch (c)
		{
		/*---------------
		| Redraw	|
		---------------*/
		case	FN3:
			Dsp_heading();
			_Print_display();
			old_page_no = -1;
			break;

		/*-----------------
		| Printer option. |
		-----------------*/
		case	FN5:
			if (_PRN_OK)
				Dsp_print ();
			else
				psl_print();
			_Print_display();
			old_page_no = -1;
			break;

		/*---------------
		| Page Down	|
		---------------*/
		case	FN14:
			if (gph_enable)
			{
				print_at 
				(
					_TOP_LINE + _PAGE_LENGTH,
				  	_LEFT_COL + 1,
					"%-*.*s", 
					_PAGE_WIDTH, 
					_PAGE_WIDTH,
					" "
				);
				rv_pr ( " Select max/min for zoom ",
					_LEFT_COL + ((_PAGE_WIDTH + 2)- 25) / 2,
					_TOP_LINE + _PAGE_LENGTH,
					1
				      );
				_GRAPHS = 2;
				old_page_no = -1;
			}
			if (++page_no > max_page)
				page_no = 0;
			line_no = -1;
			old_line_no = -2;
			break;

		/*---------------
		| Page Up	|
		---------------*/
		case	FN15:
			if (gph_enable)
			{
				crsr_on ();
				return (-1);
			}
			if (--page_no < 0)
				page_no = max_page;
			line_no = -1;
			old_line_no = -2;
			break;

		/*---------------
		| Exit Display	|
		---------------*/
		case	FN16:
			crsr_on();
			return (EXIT_SUCCESS);

		case	UP_KEY:
		case	11:
			if (_EXTRAS && _GRAPHS)
				line_no = _line_up(line_no,page_size);
			else
				putchar('\007');
			break;

		case	DOWN_KEY:
		case	10:
			if (_EXTRAS && _GRAPHS)
				line_no = _line_down(line_no,page_size);
			else
				putchar('\007');
			break;

		case	'\r':
			if (_GRAPHS == 2)
			{
				*gph_min = line_no;
				_GRAPHS--;
				break;
			}
			if (_GRAPHS == 1)
			{
				if (*gph_min == line_no)
				{
					putchar ('\007');
					break;
				}
				_GRAPHS--;
				if (line_no < *gph_min)
					*gph_max = line_no;
				else
				{
					*gph_max = *gph_min;
					*gph_min = line_no;
				}
				return (EXIT_FAILURE);
			}
			if (_EXTRAS)
			{
				if ((*_run_fn)(_KEY(line_no)))
					return (EXIT_FAILURE);

				_Print_display();
				old_page_no = -1;
				break;
			}

		case	-1:
			break;
	
		default:
			if (_EXTRAS && key_list)
			{
				int *pKey;

				for (pKey = key_list; *pKey; pKey++)
				{
					if (*pKey == c)
					{
						(*_run_fn) (_KEY (line_no));
						_Print_display ();
						old_page_no = -1;
						break;
					}
				}
				if (!*pKey)
					putchar ('\007');
			}
			else
				putchar ('\007');
			break;
		}
	}
}

int
_line_down 
(
	int line_no, 
	int page_size)
{
	int	i;

	for (i = line_no;++i < page_size;)
	{
		if (_DSP(i) != (char *)0 && (_KEY(i) != (char *)0))
			return (i);
	}

	for (i = -1;++i <= line_no;)
	{
		if (_DSP(i) != (char *)0 && (_KEY(i) != (char *)0))
			return (i);
	}

	return (line_no);
}

int
_line_up 
(
	int line_no, 
	int page_size)
{
	int	i;

	for (i = line_no;--i >= 0;)
	{
		if (_DSP(i) != (char *)0 && (_KEY(i) != (char *)0))
			return (i);
	}

	for (i = page_size;--i > line_no;)
	{
		if (_DSP(i) != (char *)0 && (_KEY(i) != (char *)0))
			return (i);
	}

	return (line_no);
}

void
_show_display
(
	int line_no)
{
	register	int	i;

	for (i = 0;i < _PAGE_LENGTH;i++)
		_show_line(i,(_EXTRAS && _GRAPHS && i == line_no));
}

void
_show_line
(
	int line_no, 
	int rv)
{
	if (line_no >= 0 && line_no < _PAGE_LENGTH)
	{
		if (_DSP(line_no) != (char *)0)
			rv_pr(_DSP(line_no),_LEFT_COL + 1,_TOP_LINE + line_no,rv);
		else
		{
			print_at 
			(
				_TOP_LINE + line_no, 
				_LEFT_COL + 1,
				"%*.*s",
				_PAGE_WIDTH,
				_PAGE_WIDTH,
				" "
			);
		}
	}
}

int
_Load_display
(
	int page_no)
{
	int	page_size = 0;
	register	int	i;

	if (fseek (_WORK_FILE,_DISP_PAGE (page_no),0) != 0)
	{
		fclose(_WORK_FILE);
		return (-1);
	}

	for (i = 0,page_size = 0;i < _PAGE_LENGTH;i++)
	{
		if (_Load_d_line(i))
			page_size++;
	}

	return (page_size);
}

int
_Load_d_line
(
	int line_no)
{
	char	*sptr;
	char	*tptr;
	char	*_extra;
	char	in_data[513];
	char	out_data[1024];

	sptr = fgets(in_data,512,_WORK_FILE);
	if (sptr != (char *) 0)
	{
		*(sptr + strlen (sptr) - 1) = 0;
		_extra = strchr (in_data, 127);
		if (_extra != (char *) 0)
		{
			*_extra = 0;
			_extra++;
		}
		sptr = out_data;
		_Parse_str(out_data, in_data, _extra, _PAGE_WIDTH);
	}

	if (_DSP(line_no) != (char *)0)
	{
		free(_DSP(line_no));
		_DSP(line_no) = (char *)0;
	}

	if (_KEY(line_no) != (char *)0)
	{
		free(_KEY(line_no));
		_KEY(line_no) = (char *)0;
	}

	if (sptr != (char *)0)
	{
		tptr = strchr(sptr,'\t');
		if (tptr != (char *)0)
		{
			*tptr = '\0';
			tptr++;
			_KEY(line_no) = strdup (tptr);
		}

		_DSP(line_no) = strdup (sptr);

		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
Clear_display (void)
{
	register	int	i = 0;
	int	page_size = _PAGE_LENGTH + 4;

	/*-----------------------
	| Second header line	|
	-----------------------*/
	if (_HEADER(1) != (char * )0)
		page_size++;

	/*---------------
	| Footer line	|
	---------------*/
	if (_HEADER(2) != (char * )0)
		page_size++;

	for (i = 0;i < page_size;i++)
	{
		print_at 
		(
			_TOP_ROW + i,
			_LEFT_COL,
			"%-*.*s",
			_PAGE_WIDTH + 2,
			_PAGE_WIDTH + 2,
			" "
		);
	}
	fflush(stdout);
}

int
_Print_display (void)
{
	register	int	i = 0;
	int	page_size = _PAGE_LENGTH + 2;

	_TOP_LINE = _TOP_ROW + 3;

	/*-----------------------
	| Second header line	|
	-----------------------*/
	if (_HEADER(1) != (char * )0)
	{
		page_size++;
		_TOP_LINE++;
	}

	/*---------------
	| Footer line	|
	---------------*/
	if (_HEADER(2) != (char * )0)
		page_size++;

	crsr_off();
	box (_LEFT_COL,_TOP_ROW,_PAGE_WIDTH + 2,page_size);

	/*-------------------------------
	| Print First Heading Line	|
	-------------------------------*/
	rv_pr(_HEADER(0),_LEFT_COL + 1,_TOP_ROW + 1,1);

	/*-------------------------------
	| Print Second Heading Line	|
	-------------------------------*/
	if (_HEADER(1) != (char *)0)
		rv_pr(_HEADER(1),_LEFT_COL + 1,_TOP_ROW + 2,1);

	line_at (_TOP_LINE - 1, _LEFT_COL + 1, _PAGE_WIDTH + 1);

	if (_PRE_CLEAR)
		for (i = 0;i < _PAGE_LENGTH;i++)
		{
			print_at 
			(
				_TOP_LINE + i, 
				_LEFT_COL + 1, 
				"%-*.*s",
				_PAGE_WIDTH,
				_PAGE_WIDTH,
				" "
			);
		}

	if (_HEADER(2) != (char *)0)
	{
		print_at 
		(
			_TOP_LINE + _PAGE_LENGTH,
			_LEFT_COL + 1,
			"%-*.*s",
			_PAGE_WIDTH,
			_PAGE_WIDTH,
			" "
		);
		us_pr (_HEADER(2),_LEFT_COL + ((_PAGE_WIDTH + 2) - strlen(_HEADER(2)))/2,_TOP_LINE + _PAGE_LENGTH,1);
	}
	fflush(stdout);
	return (EXIT_SUCCESS);
}

void
_Parse_str 
(
	char	*out_str, 
	char	*in_str, 
	char	*_extra, 
	int		page_width)
{
	int	len = page_width;
	int	leng;
	int	indx;
	int	p_graph;
	int	g_set = FALSE;
	int	so_set = FALSE;
	int	us_set = FALSE;
	int	rv_set = FALSE;
	int	fl_set = FALSE;
	char	*sptr = strchr(in_str,'^');
	char	*tptr = in_str;

	strcpy (out_str, "");

	if (_extra != (char *)0)
		_EXTRAS = 1;

	while (g_set || sptr != (char *)0)
	{
		p_graph = 0;
		while (*sptr && g_set && len > 0 && *sptr != '^')
		{
			indx = *sptr - 'A';
			if (indx >= 0 && indx < 16)
			{
				sprintf (out_str + strlen (out_str), "%c",
					ta[12][indx]);
				len--;
			}
			sptr++;
			p_graph = 1;
		}

		if (p_graph)
			tptr = sptr;

		if (strlen(tptr) == 0)
			break;

		leng = sptr - tptr;
		if (len > leng)
		{
			if (leng > 0)
				sprintf (out_str + strlen (out_str), "%-*.*s",
					leng,
					leng,
					tptr);
		}
		else
			break;

		len -= leng;
		if (len <= 0)
			break;

		if (!p_graph || (p_graph && *sptr == '^'))
			sptr++;

		switch (*sptr)
		{
		case	'^':
			g_set = !g_set;
			sptr++;
			sprintf (out_str + strlen (out_str), "%s",
				(g_set) ? ta[16] : ta[17]);
			break;

		case	'1':
			so_set = TRUE;
			sprintf (out_str + strlen (out_str), "%s", ta[8]);
			sptr++;
			break;

		case	'6':
			so_set = FALSE;
			sprintf (out_str + strlen (out_str), "%s", ta[9]);
			sptr++;
			break;

		case	'2':
			us_set = TRUE;
			sprintf (out_str + strlen (out_str), "%s", ta[10]);
			sptr++;
			break;

		case	'7':
			us_set = FALSE;
			sprintf (out_str + strlen (out_str), "%s", ta[11]);
			sptr++;
			break;

		case	'3':
			rv_set = TRUE;
			sprintf (out_str + strlen (out_str), "%s", ta[13]);
			sptr++;
			break;

		case	'8':
			rv_set = FALSE;
			sprintf (out_str + strlen (out_str), "%s", ta[14]);
			sptr++;
			break;

		case	'4':
			fl_set = TRUE;
			sprintf (out_str + strlen (out_str), "%s", ta[18]);
			sptr++;
			break;

		case	'9':
			fl_set = FALSE;
			sprintf (out_str + strlen (out_str), "%s", ta[19]);
			sptr++;
			break;

		default:
			indx = *sptr - 'A';
			if (indx >= 0 && indx < 16)
			{
				if (g_set)
					sprintf (out_str + strlen (out_str),
						"%c",
						ta[12][indx]);
				else
					sprintf (out_str + strlen (out_str),
						"%s%c%s",
						ta[16],
						ta[12][indx],
						ta[17]);
				len--;
				sptr++;
			}
		}

		if (!g_set)
		{
			tptr = sptr;
			sptr = strchr(tptr,'^');
		}
	}

	sprintf (out_str + strlen (out_str), "%-*.*s",
		len,
		len,
		(strlen (tptr)) ? tptr : " ");

	if (g_set)
		sprintf (out_str + strlen (out_str), "%s",
			ta[17]);

	if (so_set)
		sprintf (out_str + strlen (out_str), "%s",
			ta[9]);

	if (us_set)
		sprintf (out_str + strlen (out_str), "%s",
			ta[11]);

	if (rv_set)
		sprintf (out_str + strlen (out_str), "%s",
			ta[14]);

	if (fl_set)
		sprintf (out_str + strlen (out_str), "%s",
			ta[19]);

	if (_extra != (char *)0)
		sprintf (out_str + strlen (out_str), "%c%s\n",
			'\t',
			_extra);
	else
		sprintf (out_str + strlen (out_str), "\n");
}

struct	_dsp_type	*_dsp_alloc (void)
{
	struct	_dsp_type	*dptr;

	if (_dsp_free == D_NUL)
		return ((struct _dsp_type *) malloc((unsigned) sizeof(struct _dsp_type)));
	else
	{
		dptr = _dsp_free;
		_dsp_free = _dsp_free->_next;
	}
	return (dptr);
}

int
Dsp_print (void)
{
	int		loop,
			caret_cnt;
	long	old_offset;
	time_t	tloc;
	FILE	*fout;
	char	*sptr, *dptr;
	char	*pptr = getenv ("PROG_PATH");
	char	work_line[512];
	int		shortHeading;

	if (!_PRN_OK)
		return (EXIT_FAILURE);

	tloc = time (NULL);

	old_offset = ftell (_WORK_FILE);

	if (_PRN_ONLY)
	{
		fclose (_WORK_FILE);
		sprintf (work_line, "%s/WORK/dsp%06d.%02d", (pptr != (char *)0) ? pptr : "/usr/LS10.5", (int) getpid (), _INDX);
		if ((_WORK_FILE = fopen (work_line, "r")) == NULL)
		{
			fclose (_WORK_FILE);
			return (EXIT_FAILURE);
		}
	}
	else
	{
		DSP_UTILS_lpno = get_lpno (1);
		print_mess (ML ("Please wait, Printing report"));
	}

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s <%s>\n", DateToString (TodaysDate ()), PNAME);

	if (!DSP_UTILS_lpno)
		DSP_UTILS_lpno = get_lpno (1);

	fprintf (fout, ".LP%d\n", DSP_UTILS_lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".CARET_ON\n");

	loop = 0;
	if (_HEADER(1) != (char *) 0)
		loop++;
	if (_CO_NO != (char *) 0)
		loop++;
	if (_BR_NO != (char *) 0)
		loop++;
	if (_WH_NO != (char *) 0)
		loop++;
	if (_TEXT != (char *) 0)
		loop++;
	fprintf (fout, ".%d\n", loop + 7);

	fprintf (fout, ".L%d\n", _PAGE_WIDTH + 2);

	if (_CO_NO != (char *) 0)
	{
		shortHeading	=	FALSE;
		if ((strlen (clip (_CO_NAME)) + 14) > (_PAGE_WIDTH / 2))
			shortHeading	=	TRUE;
		
		fprintf 
		(
			fout, 
			"%s%s : %-2.2s %s\n", 
			(shortHeading) ? ".C" : ".E", 
			ML ("Company"), 
			_CO_NO, 
			_CO_NAME
		);
	}

	if (_BR_NO != (char *) 0)
	{
		shortHeading	=	FALSE;
		if ((strlen (clip (_BR_NAME)) + 14) > (_PAGE_WIDTH / 2))
			shortHeading	=	TRUE;

		fprintf 
		(
			fout, 
			"%s%s : %-2.2s %s\n", 
			(shortHeading) ? ".C" : ".E", 
			ML ("Branch"), 
			_BR_NO, 
			_BR_NAME
		);
	}

	if (_WH_NO != (char *) 0)
	{
		shortHeading	=	FALSE;
		if ((strlen (clip (_WH_NAME)) + 14) > (_PAGE_WIDTH / 2))
			shortHeading	=	TRUE;

		fprintf 
		(
			fout, 
			"%s%s : %-2.2s %s\n", 
			(shortHeading) ? ".C" : ".E", 
			ML ("Warehouse"), 
			_WH_NO, 
			_WH_NAME
		);
	}

	fprintf (fout, ".EAS AT %-24.24s\n", ctime (&tloc));
	fprintf (fout, ".B1\n");

	if (_TEXT != (char *) 0)
	{
		if ((strlen (clip (_TEXT)) - 1) > (_PAGE_WIDTH / 2))
			fprintf (fout, ".C%s\n", _TEXT);
		else
			fprintf (fout, ".E%s\n", _TEXT);
	}

	fprintf (fout, ".R^^C%sD^^\n", string (_PAGE_WIDTH, "G"));

	fprintf (fout, "^^A%sB^^\n", string (_PAGE_WIDTH, "G"));

	sptr = _HEADER(0);
	dptr = work_line;
	while (*sptr)
	{
		if (*sptr != '|')
			*dptr++ = *sptr++;
		else
		{
			sptr++;
			*dptr++ = '^';
			*dptr++ = 'E';
		}
	}
	*dptr = 0;
	fprintf (fout, "^E%s^E\n", work_line);

	if (_HEADER(1) != (char *) 0)
	{
		sptr = _HEADER(1);
		dptr = work_line;
		while (*sptr)
		{
			if (*sptr != '|')
				*dptr++ = *sptr++;
			else
			{
				sptr++;
				*dptr++ = '^';
				*dptr++ = 'E';
			}
		}
		*dptr = 0;
		caret_cnt = cnt_carets (work_line);
		fprintf (fout, "^E%-*.*s^E\n",
			_PAGE_WIDTH + caret_cnt,
			_PAGE_WIDTH + caret_cnt,
			work_line);
	}

	sptr = _HEADER(0);
	dptr = work_line;
	while (*sptr)
	{
		if (*sptr != '|')
			*dptr++ = 'G';
		else
			*dptr++ = 'H';
		sptr++;
	}
	*dptr = 0;
	fprintf (fout, "^^K%sL^^\n", work_line);

	fseek (_WORK_FILE, 0L, 0);
	sptr = fgets (work_line, sizeof work_line, _WORK_FILE);
	while (sptr && !feof (_WORK_FILE))
	{
		dptr = strchr (sptr, 127);
		if (dptr == (char *) 0)
			*(sptr + strlen (sptr) - 1) = 0;
		else
			*dptr = 0;
		caret_cnt = cnt_carets (sptr);
		fprintf 
		(
			fout, "^E%-*.*s^E\n", 
			_PAGE_WIDTH + caret_cnt,
			_PAGE_WIDTH + caret_cnt,
			work_line
		);
		sptr = fgets (work_line, sizeof work_line, _WORK_FILE);
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);

	fseek (_WORK_FILE, old_offset, 0);

	return (EXIT_SUCCESS);
}

int
cnt_carets (char *_string)
{
	int	cnt = 0;

	while (*_string)
	{
		if (*_string == '^')
		{
			cnt++;
			if (*(_string + 1) >= '0' && *(_string + 1) <= '9')
				cnt++;
		}
		_string++;
	}
	return (cnt);
}

/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: secs.c,v 5.1 2001/08/06 22:40:56 scott Exp $
|  Program Name  : (secs.c        )                                   |
|  Program Desc  : (Screen Edit Control System (NB: NOT SEX!!)    )   |
|---------------------------------------------------------------------|
|  Date Written  : 11/06/91        |  Author     : Trevor van Bremen  |
|---------------------------------------------------------------------|
|  Date Modified : (17/07/91)      | Modified by : Campbell Mander.   |
|  Date Modified : (13/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (25/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (20/02/96)      | Modified by : Scott B Darrow     |
|                                                                     |
|  Comments      : (17/07/91) - If a TXT screen within screen gen then|
|                : spec_valid called for FN keys 2 - 13, else as      |
|                : before.                                            |
|     (13/08/93) : PSL 9513 use of <malloc.h>                         |
|     (25/08/93) : HGP 9649 fixed bug discovered in txt_edit() during |
|                : port to ICL DRS/NX 6000                            |
|     (20/02/96) : PDL - Updated to undo change made by ???????. Will |
|                        find and terminate person later.             |
=====================================================================*/
#include	<std_decs.h>

#define	MAX_TXT_LINES	500

static	void	ErrorPause (void);
static	int	TXT_SCN;
extern	int	row;
extern	int	col;
extern	int	restart;
extern	int	last_char;
static	int	_txt_init = FALSE;
static	int	_txt_draw = TRUE;
#define	TXT_WIN	txt_win [window]
struct
{
	int		x;
	int		y;
	int		dx;
	int		dy;
	int		max_y;
	int		last_y;
	int		drawn;
	char	*hdg;
	char	*txt_buf;
} txt_win [10];

void
txt_init (void)
{
	int	window;

	for (window = 0; window < 10; window++)
	{
		TXT_WIN.x		= -1;
		TXT_WIN.y		= -1;
		TXT_WIN.dx		= -1;
		TXT_WIN.dy		= -1;
		TXT_WIN.max_y	= -1;
		TXT_WIN.last_y	= -1;
		TXT_WIN.drawn	= FALSE;
		TXT_WIN.hdg		= (char *) 0;
		TXT_WIN.txt_buf	= (char *) 0;
	}
	_txt_init = TRUE;
}

/*---------------------------------------
| Create a text window					|
|	Top	: y								|
|	Left	: x							|
|	Height	: dy						|
|	Width 	: dx						|
|	Max lin	: max_y						|
|	Heading	: Free format hdg text		|
| Return a 'window-handle' for future	|
| references. Return -1 if no further	|
| handles are available (or error).		|
---------------------------------------*/
int
txt_open (
	int		y, 
	int 	x, 
	int 	dy, 
	int 	dx, 
	int 	max_y, 
	char 	*hdg)
{
	char	*p_strsave (char *),
			*rec_ptr,
			*lcl_hdg;
	int		loop,
			window;

	if (!_txt_init)
		txt_init ();

	if (x < 0 || y < 0)
	{
		print_at (0,0,"x[%d] or y[%d] < 0", x,y); ErrorPause ();
		return (-1);
	}

	if ((x + dx + 2) > 132)
	{
		print_at (0,0,"(x[%d] + dx[%d] + 2) > 132", x,dx); ErrorPause ();
		return (-1);
	}

	if ((x + dx + 2) > 80 && !_wide)
	{
		print_at (0,0,"(x[%d] + dx[%d] + 2) > 80", x,dx); ErrorPause ();
		return (-1);
	}

	if ((y + dy + 2) > 23)
	{
		print_at (0,0,"(y[%d] + dy[%d] + 2) > 23", y,dy); ErrorPause ();
		return (-1);
	}

	if (max_y < 1 || max_y > MAX_TXT_LINES)
	{
		printf("(max_y[%d] < 1 || > MAX_TXT_LINES", max_y); ErrorPause ();
		return (-1);
	}

	/*-------------------------------
	| Copy the hdg and truncate it	|
	| if reqd to fit in window.  	|
	-------------------------------*/
	if (hdg)
		lcl_hdg = p_strsave (hdg);
	else
		lcl_hdg = p_strsave ("");

	if ((int) (strlen (lcl_hdg)) > dx)
		*(lcl_hdg + dx) = 0;

	for (window = 0; window < 10; window++)
	{
		if (TXT_WIN.x == -1)
		{
			rec_ptr = (char *) malloc (max_y * (dx + 1));
			if (rec_ptr == (char *) 0)
				return (-1);
			TXT_WIN.x	= x;
			TXT_WIN.y	= y;
			TXT_WIN.dx	= dx;
			TXT_WIN.dy	= dy;
			TXT_WIN.max_y	= max_y;
			TXT_WIN.last_y	= 0;
			TXT_WIN.drawn	= FALSE;
			TXT_WIN.hdg	= p_strsave (ML (lcl_hdg));
			TXT_WIN.txt_buf	= rec_ptr;
			for (loop = 0; loop < max_y; loop++)
			{
				sprintf (rec_ptr, "%-*.*s", dx, dx, " ");
				rec_ptr += dx;
				rec_ptr++;
			}
			return (window);
		}
	}
	free (lcl_hdg);

	return (-1);
}

/*---------------------------------------
| Display any 'saved' text in the	|
| window starting from specified line	|
---------------------------------------*/
int
txt_display (int window, int line)
{
	char	*rec_ptr;
	int		loop,
			x,
			hdg_len;

	if (!_txt_init)
		return (-1);

	if (window < 0 || window > 9)
		sys_err ("Illegal window handle During (TXT_DISPLAY)", -1, PNAME);

	crsr_off ();

	if (_txt_draw || !TXT_WIN.drawn)
	{
		box (TXT_WIN.x, TXT_WIN.y, TXT_WIN.dx + 2, TXT_WIN.dy);

		hdg_len = strlen (TXT_WIN.hdg);
		if (hdg_len)
		{
			x = (TXT_WIN.x + ((TXT_WIN.dx - hdg_len) / 2));
			print_at (TXT_WIN.y, x, "%R %s ", TXT_WIN.hdg);
		}
	}

	if (line < 1)
		line = 1;

	for (loop = 0; loop < TXT_WIN.dy; loop++)
	{
		if ((line + loop) <= TXT_WIN.last_y)
		{
			rec_ptr = TXT_WIN.txt_buf + ((loop + line - 1) * (TXT_WIN.dx + 1));

			print_at 
			( 
				TXT_WIN.y + 1 + loop,
				TXT_WIN.x + 1, 
				"%-*.*s", 
				TXT_WIN.dx, 
				TXT_WIN.dx, 
				rec_ptr
			);
		}
		else
		{
			print_at 
			( 
				TXT_WIN.y + 1 + loop,
				TXT_WIN.x + 1, 
				"%-*.*s", 
				TXT_WIN.dx, 
				TXT_WIN.dx, 
				" "
			);
		}
	}
	crsr_on ();
	TXT_WIN.drawn	= TRUE;
	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Terminate ALL references to the user	|
| supplied window. If clr_flg is TRUE,	|
| blank-out the window's contents.	|
---------------------------------------*/
int
txt_close (int window, int clr_flg)
{
	int	y;

	if (!_txt_init)
		return (-1);

	if (window < 0 || window > 9)
		sys_err ("Illegal window handle During (TXT_CLOSE)", -1, PNAME);

	if (TXT_WIN.x == -1)
		return (-1);

	if (clr_flg)
	{
		for (y = 0; y < TXT_WIN.dy + 2; y++)
		{
			print_at 
			(
				TXT_WIN.y + y, 
				TXT_WIN.x, 
				"%-*.*s",
				TXT_WIN.dx + 2,
				TXT_WIN.dx + 2,
				" "
			);
		}
	}
	free (TXT_WIN.hdg);
	free (TXT_WIN.txt_buf);
	TXT_WIN.x	= -1;
	TXT_WIN.y	= -1;
	TXT_WIN.dx	= -1;
	TXT_WIN.dy	= -1;
	TXT_WIN.max_y	= -1;
	TXT_WIN.last_y	= -1;
	TXT_WIN.drawn	= FALSE;
	TXT_WIN.hdg	= (char *) 0;
	TXT_WIN.txt_buf	= (char *) 0;
	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Stuff the user-supplied text into the	|
| window at the given line. If the line	|
| already existed, overwrite it. If it	|
| was well past EOF, make it the last 1	|
---------------------------------------*/
int
txt_pval (int window, char *text, int line)
{
	char	*rec_ptr;

	if (!_txt_init)
		return (-1);

	if (window < 0 || window > 9)
		sys_err ("Illegal window handle During (TXT_PVAL)", -1, PNAME);

	if (TXT_WIN.x == -1)
		return (-1);

	if (
		line < 0 ||
		line > TXT_WIN.max_y ||
		 (line == 0 && TXT_WIN.last_y == TXT_WIN.max_y)
	  )
		return (-1);

	if (TXT_WIN.last_y < line || line == 0)
	{
		TXT_WIN.last_y++;
		line = TXT_WIN.last_y;
	}

	rec_ptr = TXT_WIN.txt_buf + ((line - 1) * (TXT_WIN.dx + 1));
	sprintf 
	(
		rec_ptr, 
		"%-*.*s",
		TXT_WIN.dx,
		TXT_WIN.dx,
		text
	);
	return (EXIT_SUCCESS);
}

/*---------------------------------------
| Retrieve the specified line of text.	|
| If > max_y, return NULL ptr.		|
---------------------------------------*/
char	*txt_gval (int window, int line)
{
	if (!_txt_init)
		return ((char *) 0);

	if (window < 0 || window > 9)
		sys_err ("Illegal window handle During (TXT_GVAL)", -1, PNAME);

	if (TXT_WIN.x == -1)
		return ((char *) 0);

	if (line < 1 || line > TXT_WIN.max_y)
		return ((char *) 0);

	return (TXT_WIN.txt_buf + ((line - 1) * (TXT_WIN.dx + 1)));
}

int
txt_edit (int window)
{
	TXT_SCN = FALSE;
	return (_txt_edit (window, (struct var *) NULL, 0));
}

int
txt_scn_edit (struct var *tmp_vars, int tmp_scn_start)
{
	TXT_SCN = TRUE;
	return (_txt_edit (0, tmp_vars, tmp_scn_start));
}

/*-------------------------------------------------------------------
| Edit/Input text as required into the specified text window.		|
-------------------------------------------------------------------*/
int
_txt_edit (int window, struct var *tmp_vars, int tmp_scn_start)
{
	char	*rec_ptr;
	int		x = 1,
			y = 1,
			curr_lin = 1,
			chr,
			tmp;

	if (!_txt_init)
	    return (-1);

	if (TXT_SCN)
		window = tmp_vars [tmp_scn_start].type - 1;

	if (window < 0 || window > 9)
		sys_err ("Illegal window handle During (TXT_EDIT)", -1, PNAME);

	_txt_draw = FALSE;
	txt_display (window, 1);

	rec_ptr = txt_gval (window, curr_lin);
	do
	{
		row = TXT_WIN.y + y;
		col = TXT_WIN.x + x;
	    move (TXT_WIN.x + x, TXT_WIN.y + y);
	    chr = getkey ();
	    if (chr == FN1)
	    {
			restart = TRUE;
			TXT_WIN.drawn = FALSE;
			_txt_draw = TRUE;
			return (TXT_WIN.last_y);
	    }

	    if (chr == FN16)
	    {
			TXT_WIN.drawn = FALSE;
			_txt_draw = TRUE;
			return (TXT_WIN.last_y);
	    }

	    if ((chr >= FN2 && chr <= FN13) && TXT_SCN)
	    {
			last_char = chr;
			spec_valid (tmp_scn_start);
			_txt_draw = TRUE;
			txt_display (window, 1 + curr_lin - y);
			_txt_draw = FALSE;
	    }

	    if (TXT_WIN.last_y < 1)
			TXT_WIN.last_y = 1;

	    switch (chr)
	    {
		case	FN15:
		case	0x12:	/* Control R	*/
			if ((1 + curr_lin - y) > TXT_WIN.dy)
			{
				curr_lin -= TXT_WIN.dy;
				txt_display (window, 1 + curr_lin - y);
				curr_lin -= y;
				curr_lin++;
				rec_ptr = txt_gval (window, curr_lin);
				x = 1;
				y = 1;
			}
			else
			{
				if (curr_lin == 1)
					putchar (BELL);
				x = 1;
				y = 1;
				curr_lin = 1;
				txt_display (window, 1);
				rec_ptr = txt_gval (window, 1);
			}
		break;

		case	FN14:
		case	0x03:	/* Control C	*/
			if ((curr_lin - y) + TXT_WIN.dy + 1 <= TXT_WIN.last_y)
			{
				curr_lin += (TXT_WIN.dy - y + 1);
				txt_display (window, curr_lin);
				rec_ptr = txt_gval (window, curr_lin);
				x = 1;
				y = 1;
			}
			else
				putchar (BELL);
		break;

		case	FN3:
			txt_display (window, 1 + curr_lin - y);
			break;

	    	case	UP_KEY:
	    	case	0x05:	/* Control E	*/
			if (y > 1)
			{
		    	y--;
		    	curr_lin--;
		    	rec_ptr = txt_gval (window, curr_lin);
			}
			else
			{
		    	if (curr_lin > 1)
		    	{
					curr_lin--;
					txt_display (window, curr_lin);
		    	}
		    	else
				putchar (BELL);
			}
		break;

		case	DOWN_KEY:
		case	0x0A:	/* Control J	*/
		case	0x18:	/* Control X	*/
			if (curr_lin == TXT_WIN.last_y)
			{
		    	x = 1;
		    	txt_pval (window, " ", 0);
			}
			if (curr_lin >= TXT_WIN.max_y)
		    	putchar (BELL);
			else
			{
		    	curr_lin++;
		    	rec_ptr = txt_gval (window, curr_lin);
		    	if (y < TXT_WIN.dy)
					y++;
		    	else
					txt_display (window, 1 + curr_lin - y);
			}
		break;

		case	LEFT_KEY:
		case	0x08:	/* Control H	*/
		case	0x13:	/* Control S	*/
			if (x > 1)
		    	x--;
			else if (curr_lin <= 1)
				putchar (BELL);
		    else
		    {
				x = TXT_WIN.dx;
				curr_lin--;
				rec_ptr = txt_gval (window, curr_lin);
				if (y > 1)
			    	y--;
				else
			    	txt_display (window, curr_lin);
			}
		break;

		case	RIGHT_KEY:
		case	0x04:	/* Control D	*/
			if (x < TXT_WIN.dx)
				x++;
			else if (curr_lin >= TXT_WIN.max_y)
				putchar (BELL);
			else
			{
				x = 1;
				curr_lin++;
				if (curr_lin > TXT_WIN.last_y)
					txt_pval (window, " ", 0);
				rec_ptr = txt_gval (window, curr_lin);
				if (y < TXT_WIN.dy)
					y++;
				else
					txt_display (window, 1 + curr_lin - y);
			}
		break;

		case	INSCHAR:
			tmp = TXT_WIN.dx - 1;
			while (tmp >= x)
			{
				* (rec_ptr + tmp) = * (rec_ptr + tmp - 1);
				tmp--;
			}
			*(rec_ptr + x - 1) = ' ';
			print_at (TXT_WIN.y + y, TXT_WIN.x + 1, "%s", rec_ptr);
			break;

			case	DELCHAR:
			case	0x07:	/* Control G	*/
			strcpy (rec_ptr + x - 1, rec_ptr + x);
			*(rec_ptr + TXT_WIN.dx - 1) = ' ';
			print_at (TXT_WIN.y + y, TXT_WIN.x + 1, "%s", rec_ptr);
		break;

	    case	INSLINE:
			if (TXT_WIN.last_y < TXT_WIN.max_y)
				TXT_WIN.last_y++;
			for (tmp = TXT_WIN.last_y - 1; tmp >= curr_lin; tmp--)
			{
				rec_ptr = txt_gval (window, tmp);
				txt_pval (window, rec_ptr, tmp + 1);
			}
			txt_pval (window, " ", curr_lin);
			rec_ptr = txt_gval (window, curr_lin);
			txt_display (window, 1 + curr_lin - y);
		break;

	    case	DELLINE:
			for (tmp = curr_lin + 1; tmp <= TXT_WIN.last_y; tmp++)
			{
				rec_ptr = txt_gval (window, tmp);
				txt_pval (window, rec_ptr, tmp - 1);
			}
			txt_pval (window, " ", TXT_WIN.last_y);
			TXT_WIN.last_y--;
			if (TXT_WIN.last_y == 0)
			{
				txt_display (window, 1);
				curr_lin = 1;
				x = 1;
				y = 1;
				rec_ptr = txt_gval (window, 1);
				break;
			}
			if (TXT_WIN.last_y < curr_lin)
			{
				curr_lin--;
				if (y > 1)
					y--;
			}
			rec_ptr = txt_gval (window, curr_lin);
			txt_display (window, 1 + curr_lin - y);
		break;

	    case	0x0D:	/* Control M	*/
			x = 1;
			curr_lin++;
			if (curr_lin > TXT_WIN.max_y)
			{
				curr_lin--;
				putchar (BELL);
				break;
			}
			if (curr_lin > TXT_WIN.last_y)
				txt_pval (window, " ", 0);
			rec_ptr = txt_gval (window, curr_lin);
			y++;
			if (y > TXT_WIN.dy)
			{
				y--;
				txt_display (window, 1 + curr_lin - y);
			}
		break;

	    default:
			*(rec_ptr + x - 1) = chr;
			putchar (chr);
		    x++;
		    if (x > TXT_WIN.dx)
		    {
				curr_lin++;
				if (curr_lin > TXT_WIN.max_y)
				{
			    	x--;
			    	curr_lin--;
			    	putchar (BELL);
			    	break;
				}
				x = 1;
				if (curr_lin > TXT_WIN.last_y)
			    	txt_pval (window, " ", 0);
				rec_ptr = txt_gval (window, curr_lin);
				y++;
				if (y > TXT_WIN.dy)
				{
					y--;
			    	txt_display (window, 1 + curr_lin - y);
				}
		    }
		break;
	    }
	} while (TRUE);
}

static	void
ErrorPause (void)
{
	fflush (stdout);
	sleep (10);
}


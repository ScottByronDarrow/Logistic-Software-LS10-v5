/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (tcap.c        )                                   |
|  Program Desc  : (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (13.08.93)      | Modified by : Jonathan Chen      |
|  Date Modified : (04.11.93)      | Modified by : Jonathan Chen      |
|  Date Modified : (30.01.96)      | Modified by : Shane Wolstencroft |
|  Comments      :                                                    |
|     (13.08.93) : PSL 9513 Use of <malloc.h>                         |
|     (04.11.93) : PSL 10047 Mods to enable win_function2 ()          |
|                : Removed embedded control chars in file             |
|     (18.05.94) : Now recognises multiple names in terminfo file     |
|  (27/12/95)    : FRA - Updated for new gl and interfaces.           |
|     (30.01.96) : Mods to enable win_function3 ().                   |
|                :                                                    |
|                                                                     |
| $Log: tcap.c,v $
| Revision 5.1  2001/08/06 22:40:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:40  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/22 01:20:43  scott
| Updated to create simple sleep functions.
|
| Revision 4.0  2001/03/09 00:52:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/02 06:40:19  scott
| Updated to add Unix Environment LSL_MSG_DELAY and LSL_ERR_DELAY
|
| Revision 3.0  2000/10/12 13:34:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:20  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.7  1999/10/25 06:04:18  scott
| Took out last changes.
|
| Revision 1.5  1999/10/20 07:10:48  nz
| Updated to remove clips.
|
| Revision 1.4  1999/09/17 03:35:36  scott
| Updated for writable strings
|
| Revision 1.3  1999/09/13 09:36:35  scott
| Updated for Copyright
|
=====================================================================*/
#include	<std_decs.h>

/*
 *	External globals
 */

/*
 | Globals
*/
char	ta [MAX_TA + 1] [81];		/* Esc seq array */

int 	rev_ = 0,			/* when set order row/col is reversed	*/
		_coset,				/* cursor offset value			*/
		_ibm_term = FALSE;	/* Damn stupied IBM-3151 terminals	*/

int	_mail_ok = TRUE;
int	_win_func = FALSE;
int	_SpecialRedraw = FALSE;

char	*_mail_used;

char	*atrb[MAX_TA + 1] = {
	"cd=",	/* 0	clear to end of display			*/
	"ce=",	/* 1	clear to end of line			*/
	"cl=",	/* 2	clear screen					*/
	"dc=",	/* 3	delete character				*/
	"dl=",	/* 4	delete line						*/
	"SD=",	/* 5	screen dump to aux.				*/
	"ic=",	/* 6	insert character				*/
	"is=",	/* 7	terminal initialisation string	*/
	"so=",	/* 8	start stand out mode			*/
	"se=",	/* 9	end stand out mode				*/
	"us=",	/* 10	start underscore mode			*/
	"ue=",	/* 11	end underscore mode				*/
	"bx=",	/* 12	box characters					*/
	"rv=",	/* 13	reverse on						*/
	"re=",	/* 14	reverse off						*/
	"cm=",	/* 15	cursor addressing sequence		*/
	"go=",	/* 16	graphics mode on				*/
	"ge=",	/* 17	graphics mode off				*/
	"fl=",	/* 18	flashing start					*/
	"fe=",	/* 19	flashing end					*/
	"sW=",	/* 20	switch to wide mode				*/
	"sN=",	/* 21	switch to normal mode			*/
	"dW#",	/* 22	Wide delay.						*/
	"cc=",	/* 23	suppress cursor					*/
	"cb=",	/* 24	begin cursor					*/
	"CM=",	/* 25	alternate cursor movement		*/
	"ku=",	/* 26	Arrow up.						*/
	"kd=",	/* 27	Arrow down.						*/
	"kl=",	/* 28	Arrow left.						*/
	"kr=",	/* 29	Arrow right.					*/
	"HP=",	/* 30	Help Key.						*/
	"IC=",	/* 31	Insert Character.				*/
	"DC=",	/* 32	Delete Character.				*/
	"IL=",	/* 33	Insert Line.					*/
	"DL=",	/* 34	Delete Line.					*/
	"k0=",	/* 35	FN1.							*/
	"k1=",	/* 36	FN2.							*/
	"k2=",	/* 37	FN3.							*/
	"k3=",	/* 38	FN4.							*/
	"k4=",	/* 39	FN5.							*/
	"k5=",	/* 40	FN6.							*/
	"k6=",	/* 41	FN7.							*/
	"k7=",	/* 42	FN8.							*/
	"k8=",	/* 43	FN9.							*/
	"k9=",	/* 44	FN10.							*/
	"kA=",	/* 45	FN11.							*/
	"kB=",	/* 46	FN12.							*/
	"kC=",	/* 47	FN13.							*/
	"kD=",	/* 48	FN14.							*/
	"kE=",	/* 49	FN15.							*/
	"kF=",	/* 50	FN16.							*/
	"c0=",	/* 51	FN17.							*/
	"c1=",	/* 52	FN18.							*/
	"c2=",	/* 53	FN19.							*/
	"c3=",	/* 54	FN20.							*/
	"c4=",	/* 55	FN21.							*/
	"c5=",	/* 56	FN22.							*/
	"c6=",	/* 57	FN23.							*/
	"c7=",	/* 58	FN24.							*/
	"c8=",	/* 59	FN25.							*/
	"c9=",	/* 60	FN26.							*/
	"cA=",	/* 61	FN27.							*/
	"cB=",	/* 62	FN28.							*/
	"cC=",	/* 63	FN29.							*/
	"cD=",	/* 64	FN30.							*/
	"cE=",	/* 65	FN31.							*/
	"cF=",	/* 66	FN32.							*/
	"PM=",	/* 67	Logistic mail 'hot-key'			*/
	"W1=",	/* 68	Logistic Window 'hot-key'		*/
	"W2=",	/* 69	Logistic Window 'hot-key' No 2	*/
	"W3=",	/* 70	Logistic Window 'hot-key' No 3	*/
	"W4=",	/* 71	Logistic Window 'hot-key' No 4	*/
	"W5=",	/* 72	Logistic Window 'hot-key' No 5	*/
	"",
};

struct	{
	int	_val;	/* value representing key			*/
	char	*_key;	/* pointer to entry from termcap		*/
} _fn_keys[] = {
	{UP_KEY,	"\033[A"},		/* ESC [ A */
	{DOWN_KEY,	"\033[B"},
	{LEFT_KEY,	"\033[D"},
	{RIGHT_KEY,	"\033[C"},
	{HELP,		"\001P\r"},
	{INSCHAR,	"\033[@"},
	{DELCHAR,	"\033[P"},
	{INSLINE,	"\033[L"},
	{DELLINE,	"\033[M"},
	{FN1,		"\001@\r"},		/* ^A @ ^M */
	{FN2,		"\001A\r"},		/* ^A A ^M */
	{FN3,		"\001B\r"},		/* etc */
	{FN4,		"\001C\r"},
	{FN5,		"\001D\r"},
	{FN6,		"\001E\r"},
	{FN7,		"\001F\r"},
	{FN8,		"\001G\r"},
	{FN9,		"\001H\r"},
	{FN10,		"\001I\r"},
	{FN11,		"\001J\r"},
	{FN12,		"\001K\r"},
	{FN13,		"\001L\r"},
	{FN14,		"\001M\r"},
	{FN15,		"\001N\r"},
	{FN16,		"\001O\r"},
	{FN17,		"\001`\r"},
	{FN18,		"\001a\r"},
	{FN19,		"\001b\r"},
	{FN20,		"\001c\r"},
	{FN21,		"\001d\r"},
	{FN22,		"\001e\r"},
	{FN23,		"\001f\r"},
	{FN24,		"\001g\r"},
	{FN25,		"\001h\r"},
	{FN26,		"\001i\r"},
	{FN27,		"\001j\r"},
	{FN28,		"\001k\r"},
	{FN29,		"\001l\r"},
	{FN30,		"\001m\r"},
	{FN31,		"\001n\r"},
	{FN32,		"\001o\r"},
	{PSLM,		"\032"},		/* ^Z */
	{PSLW,		"\027"},		/* ^W */
	{PSLW2,		"\005"},		/* ^E */
	{PSLW3,		"\022"},		/* ^R */
	{PSLW4,		"\024"},		/* ^T */
	{PSLW5,		"\029"},		/* ^Y */
	{0}
};

#define		IntRedraw	'\022'

static 	int	_wide_dly = FALSE;
static 	char	_kbd_buf [64] = "";
static 	char	_fnc_buf [64] = "";

/*
 | Function declarations
*/
static char	*_conv (char *string);		/* internal function 	*/
static char	*cm_format (char *string);	/* internal function 	*/
static int	bf_getchar (void),
			_chk_key (void);
int	ntimes = 0;

int
init_scr (void)
{
	char 	rd_line[81];
	char	*sptr = getenv ("PROG_PATH");    /* Set up Program Path. */
	char	*type = getenv ("TERM");		/* get terminal name	*/
	char	filename[100];
	int 	i;
	FILE	*termcap;
	int	wk_len = 0;

	_mail_used = getenv ("MAIL_USED");
	/*-----------------------------
	| Check if mail_used defined. |
	-----------------------------*/
	if (_mail_used == (char *)0)
		_mail_used = "/usr/bin/mail";

	/*
	 | Default PROG_PATH
	*/
	if (!sptr)
		sptr = "/usr/LS10.5";

	/*-------------------------------------------------
	| Test to see if terminal is already initialised. |
	-------------------------------------------------*/
	if (ntimes > 0)
		return (EXIT_SUCCESS);
	ntimes++;

	sprintf (filename,"%s/BIN/MENUSYS/TERM/%s", sptr, type);

	if ((termcap = fopen (filename,"r")) == 0)
	{
		sprintf (filename,"%s/BIN/MENUSYS/termcap.cprogs", sptr);
		/*----------------------------------------------
		| Open /usr/LS10.5/BIN/MENUSYS/termcapcprogs file. |
		----------------------------------------------*/
		if ((termcap = fopen (filename,"r")) == 0)
		{
			fprintf (stdout,"Error in %s during (FOPEN)\n\r",filename);
			fprintf (stdout,"%s %d\n\r",PNAME,errno);
			fflush (stdout);
			exit (errno);
		}
	}

	/*=======================
	| Find terminal entry.
	=======================*/
	while ((sptr = fgets (rd_line, sizeof (rd_line), termcap)))
	{
		char	*c;
		int		match = FALSE;

		/*	Skip comments and lines beginning with white space
		 */
		if (*sptr == '#' || isspace (*sptr))
			continue;

		for (c = sptr; *c && *c != ':'; c++)
		{
			char	entryname [80];

			for (i = 0; *c && *c != '|' && *c != ':'; i++, c++)
				entryname [i] = *c;
			entryname [i] = '\0';

			if (!strcmp (type, entryname))
			{
				match = TRUE;
				break;
			}
		}

		if (match)
			break;
	}

	if (!sptr)
	{
		puts ("Error : No entry found for terminal.");
	 	fclose (termcap);
		exit (-1);
	}

	/*=================================
	| Read entry into array elements. |
	=================================*/
	while (sptr != (char *)0 && load_ (sptr) != 1)
		sptr = fgets (rd_line,80,termcap);

	/*=========================================
	| Close file /usr/LS10.5/BIN/MENUSYS/termcap. |
	=========================================*/
	fclose (termcap);

	/*==============================
	| convert array to 'C' format. |
	==============================*/
	for (i = 0;i < MAX_TA && (i <= 26 || (i > 26 && _fn_keys[i - 26]._val));i++)
	{
		if (i >= 26 && strlen (ta[i]))
			_fn_keys[i - 26]._key = ta[i];
		strcpy (ta[i],_conv (ta[i]));
	}

	wk_len = strlen (ta[25]);
	strcpy (ta[15],cm_format ((wk_len) ? ta[25] : ta[15]));

	return (EXIT_SUCCESS);
}

/*=======================
| Draw a graphics line. |
=======================*/
void
line (int x)
{
	gr_on ();
	while (--x)
		putchar (ta[12][6]);
	gr_off ();
}

/*======================
| Draw a graphics Box. |
======================*/
void
box (int x, int y, int h, int v)
{
	int	i;
	int	j;

	gr_on ();
	if (h > 1)
	{
		move (x,y);
		putchar (ta[12][0]);
		line (h-1);
		gr_on ();
		putchar (ta[12][1]);
		i = y+1;
		j = v;
	}
	else
	{
		i = y;
		j = v + 2;
	}

	while (j--)
	{
		move (x,i);
		putchar (ta[12][4]);
		if (h > 1)
		{
			move (x+h-1,i++);
			putchar (ta[12][5]);
		}
		else
			i++;
	}

	if (h > 1)
	{
		move (x,i);
		putchar (ta[12][2]);
		line (h-1);
		gr_on ();
		putchar (ta[12][3]);
	}
	gr_off ();
}
int
load_ (char *load_line)
{
	int	flag = 0;
	int	j;
	int	len = strlen (load_line);

	if (* (load_line + len - 2) == ':' || len == 0)
		flag = 1;/*	return 1 if last line in entry	*/

	while (* (load_line + len) != ':')
		--len;
	/*--------------------- 
	| Remove end of line. |
	---------------------*/
	* (load_line + len) = '\0'; 

	while (len)
	{
		if (* (load_line + len) == ':')
		{
			* (load_line + len) = '\0';
			for (j = 0; j < MAX_TA; j++)
			{
				if (strncmp (atrb[j],load_line + len+1 ,3) == 0)
				{
					if (j == 22)
						_wide_dly = TRUE;
					strcpy (ta[j],load_line + len + 4);
					break;
				}
			}
		}
		--len;
	}
	return (flag);
}

static char	*_conv (char *string)
{
	int	j;
	int	len;
	int	c = 0;

	len = strlen (string);

	for (j = 0; j < len; j++)
	{
		switch (* (string + j))
		{
		case '^' :
			* (string + c) = * (string + j + 1) - 64;
			++c;
			++j;
			break;

		case 92:
			if (* (string + j + 1) == 'E')
			{
				* (string + c) = 27;
				++c;
				++j;
			}
			else
				if (* (string + j + 1) >= '0' || * (string + j + 1) <= '3')
				{
					* (string + c) = atoo (string + j + 1);
					++c;
					j += 3;
				} 
			break;

		default :
			* (string + c) = * (string + j);
			++c;
		}
	}
	* (string + c) = '\0';
	return (string);
}

int
atoo (char *str)
{
	char	*sptr = str;
	int	oct = 0;
	int	cnt;

	for (cnt = 0; *sptr && cnt < 3; cnt++)
	{
		oct *= 8;
		oct += (*sptr++ - '0');
		oct &= 0377;
	}
	return (oct);
}

static char	*cm_format (char *string)
{
	int	j,len,c = 0;
	char	*tstring = malloc (30);

	len = strlen (string);

	for (j = 0; j < len; j++)
	{
		if (* (string + j) != '%')
			* (tstring + c++) = * (string + j);
		else
		switch (* (string + j + 1))
		{
		case 'I' :
			/*-------------------------
			| IBM 3151 (Brain dead!!) |
			-------------------------*/
			_ibm_term = TRUE;
			++j;
			break;

		case 'i' :
			/*-------------------
			| Increment offset. |
			-------------------*/
			_coset = 1;
			++j;
			break;

		case 'r' :
			/*----------------------------
			| Set reverse order col/row. |
			----------------------------*/
			rev_ = 1;
			++j;
			break;

		case '.' :
			* (tstring + c++) = '%';
			* (tstring + c++) = 'c';
			++j;
			break;

		case '+' :
			* (tstring + c++) = '%';
			* (tstring + c++) = 'c';
			_coset = * (string + j + 2);
			j += 2;
			break;

		case '2' :
			* (tstring + c++) = '%';
			* (tstring + c++) = '2';
			* (tstring + c++) = 'd';
			++j;
			break;

		case '3' :
			* (tstring + c++) = '%';
			* (tstring + c++) = '3';
			* (tstring + c++) = 'd';
			++j;
			break;

		default :
			* (tstring + c++) = '%';
			* (tstring + c++) = * (string + j + 1);
			++j;
			break;
		}
	}
	* (tstring + c) = '\0';
	return (tstring);
}

/*=====================================================================
| Move function - changed to function from macro because of excessive |
| length of macro definition and large no. of times it is called.     |
=====================================================================*/
void
move (int x, int y)
{
	int	xhi,
		xlo,
		yhi,
		ylo;

	if (_ibm_term)
	{
		yhi = (y >> 5) + ' ';
		ylo = (y & 0x1f) + ' ';
		xhi = (x >> 5) + ' ';
		xlo = (x & 0x1f) + '@';
		printf ("%cy%c%c%c%c", 27, yhi, ylo, xhi, xlo);
	}
	else
		if (rev_)
			printf (ta[15],x + _coset,y + _coset);
		else
			printf (ta[15],y + _coset,x + _coset);
	fflush (stdout);
}
/*==============================================================
| Routines To Display System Error messages and user messages. |
==============================================================*/
/*=====================================================
| clear_mess clears any message left on message line. |
=====================================================*/
void
clear_mess (void)
{
	move (0,23);
	cl_line ();
}

/*=========================================================================
| print_mess prints a message to the screen and then beeps to alert user. |
=========================================================================*/
void
print_mess (char *message)
{
	char	wkString[201];

	if (strlen (message) < 2)
		return;

	move (0,23);
	cl_line ();
	sprintf (wkString,"[%s]",message);
	so_pr (wkString,0,23,1);
}

/*======================================================================
| Routine which prints a message passed as a parameter and beeps bell. |
======================================================================*/
void
errmess (char *message)
{
	char	wkString[201];
	move (0,23);
	cl_line ();
	sprintf (wkString,"ERROR [%s]",message);
	fl_pr (wkString,0,23,1);
}

/*========================================
| Print reverse video for linear screen. |
========================================*/
void
li_pr (char *rv_desc, int rv_x, int rv_y, int rv_flag)
{
	move ((rv_x < 1) ? 1 : rv_x,rv_y);
	if (rv_flag ? rv_on () : rv_off ());
	printf ("%s",rv_desc);
	rv_off ();
	fflush (stdout);
}
/*===========================================================================
| Routine which prints a string Turning Reverse of printing string then on. |
===========================================================================*/
void
rv_pr (char *rv_desc, int rv_x, int rv_y, int rv_flag)
{
	move (rv_x,rv_y);
	if (rv_flag ? rv_on () : rv_off ());
	printf ("%s",rv_desc);
	rv_off ();
	fflush (stdout);
}
/*=================
| Print Flashing. |
=================*/
void
fl_pr (char *fl_desc, int fl_x, int fl_y, int fl_flag)
{
	fl_x = (fl_x <= 0) ? 1 : fl_x;
	move (fl_x + strlen (fl_desc) , fl_y);
	fl_off ();
	move (fl_x - 1, fl_y);
	if (fl_flag)
		fl_on ();
	else
		fl_off ();

	printf ("%s", fl_desc);
	fl_off ();
	fflush (stdout);
}
/*=================
| Print Standout. |
=================*/
void
so_pr (char *so_desc, int so_x, int so_y, int so_flag)
{
	so_x = (so_x <= 0) ? 1 : so_x;
	move (so_x + strlen (so_desc) , so_y);
	so_off ();
	move (so_x - 1, so_y);
	if (so_flag)
		so_on ();
	else
		so_off ();

	printf ("%s", so_desc);
	so_off ();
	fflush (stdout);
}
/*=================
| Print Standout. |
=================*/
void
us_pr (char *us_desc, int us_x, int us_y, int us_flag)
{
	us_x = (us_x <= 0) ? 1 : us_x;
	move (us_x + strlen (us_desc) , us_y);
	us_off ();
	move (us_x - 1, us_y);
	if (us_flag)
		us_on ();
	else
		us_off ();

	printf ("%s", us_desc);
	us_off ();
	fflush (stdout);
}

void
swide (void)
{
	_wide = 1;
	if (strlen (ta[20]) == 0)
	{
		fprintf (stdout,"Terminal Type (%s) Does Not Support Wide Mode\n\r",getenv ("TERM"));
		fflush (stdout);
		exit (-1);
	}

	printf (ta[20]);
	fflush (stdout);
	if (_wide_dly)
		sleep (1);
}

void
snorm (void)
{
	_wide = 0;
	if (strlen (ta[21]) == 0)
		return;

	printf (ta[21]);
	fflush (stdout);
	if (_wide_dly)
		sleep (1);
}

int
getkey (void)
{
	int	chr;
	int	fnc_len;

	while (TRUE)
	{
		/*-----------------------------------------------------------------
		| Special Redraw situation to allow ungetc to place a chararacter |
		| Into the buffer and have it ignored but a redraw performed in   |
		| its place. 													  |
		-----------------------------------------------------------------*/
		if (_SpecialRedraw == TRUE)
		{
			_SpecialRedraw = FALSE;
			return (FN3);
		}
		chr = bf_getchar ();

		/*-------------------------------
		| Function keys ALWAYS start	|
		| with some control character	|
		-------------------------------*/
		if (_fnc_buf[0] == 0 && chr >= ' ')
			return (chr);

		fnc_len = strlen (_fnc_buf);
		_fnc_buf[fnc_len++] = chr;
		_fnc_buf[fnc_len] = 0;
		chr = _chk_key ();
		if (chr < 0)
		{
			chr = _fnc_buf[0];
			strcpy (_kbd_buf, &_fnc_buf[1]);
			strcpy (_fnc_buf, "");

			return (chr);
		}

		if (chr == 0)
			continue;

		if (chr == PSLM && _mail_ok)
			return (run_mail ()); 

		return (chr);
	}
}

/*-------------------------
| Get a single character. |
| If buffered data exists |
| use it else read kbd.   |
-------------------------*/
static int
bf_getchar (void)
{
	int	chr;

	if (_kbd_buf[0])
	{
		chr = _kbd_buf[0];
		strcpy (_kbd_buf, &_kbd_buf[1]);
	}
	else
	{
		errno = 0;
		chr = getchar ();
		if (chr == EOF)
			if (errno != EINTR)
				exit (1);
	}
	if (chr == '\t')
		return ('\r');

	return (chr);
}

/*-------------------------------
| Check against array of values	|
| Returns:			|
|	-1 : No match		|
|	0  : Partial match	|
|	>0 : Full match		|
-------------------------------*/
static int
_chk_key (void)
{
	int	i;
	int	fnc_len = strlen (_fnc_buf);

	/*-------------------------------
	| Go through all key maps	|
	-------------------------------*/
	for (i = 0; _fn_keys[i]._val; i++)
	{
		/*-----------------------
		| Look for a match.	|
		| Return 0 if partial,	|
		| or key-code if full.	|
		-----------------------*/
		if (!strncmp (_fn_keys[i]._key, _fnc_buf, fnc_len))
		{
			if (strlen (_fn_keys[i]._key) == fnc_len)
			{
				strcpy (_fnc_buf, "");
				return (_fn_keys[i]._val);
			}
			return (EXIT_SUCCESS);
		}
	}
	return (-1);
}

int
run_mail (void)
{
	int	status;

	if (fork () == 0)
	{
		execlp (_mail_used, _mail_used, (char *)0);
		exit (-1);
	}
	else
		wait (&status);

	set_tty ();

	return (FN3);
}

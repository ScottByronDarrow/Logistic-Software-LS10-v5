/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( getnum.c     )                                   |
|  Program Desc  : ( Get Numerics or Alphas like screen gen.      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : 07/12/87        |  Author     : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (07/12/87)      | Modified by : Roger Gibbison.    |
|  Date Modified : (08/05/89)      | Modified by : Roger Gibbison.    |
|  Date Modified : (29/09/92)      | Modified by : Campbell Mander.   |
|  Date Modified : (15/11/93)      | Modified by : Jonathan Chen      |
|                : (18.03.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (10/10/94)      | Modified by : Dirk Heinsius.     |
|                                                                     |
|  Comments      : Added documentation.                               |
|                :                                                    |
|  (29/09/92)    : Change occurrences of [sptr - buf] with msk_offset |
|                : which is (int)(sptr - buf)                         |
|  (15/11/93)    : Moved code from <getnum.h> to library. ech!        |
|  (18.03.94)    : Added getdate (). Easiest place to add it was here |
|  (09.06.94)    : Renamed getdate () to get_date () to avoid name    |
|                : clash.                                             |
|  (18.03.94)    : Bug Fix. Removed decflg and replaced with strchr() |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

#ifndef	RESTART
#include	<defkeys.h>
#define EOI 	 	FN16
#define RESTART  	FN1
#define REDRAW   	FN3
#define SEARCH   	FN4
#define ENDINPUT	'\r'
#define BS			'\b'
#define	BELL		7
#endif

/*
 | Pieces of globals from god knows where
 */
extern int	dflt_used,
			last_char,
			search_ok,
			search_key;

static	int
val_type (
	int	x)
{
	return (x == INTTYPE	||
			x == FLOATTYPE	||
			x == DOUBLETYPE ||
			x == MONEYTYPE	||
			x == LONGTYPE);
}

static	void
read_from (
	int		x, 
	int		y, 
	char	*mask, 
	char	*buf, 
	int		btype, 
	int		init_buf)
{
	char    *sptr = buf;
	char    *dpos = NULL;
	int		i;
	int		bufsize = strlen (mask);
	int     c = 0;
	int     neg    = FALSE;	
	int		msk_offset;
	char	sigchar = (btype == EDATETYPE) ? '/' : '.';	/* dates & numbers */

	/*-------------------------------
	| Redisplay after Redraw	|
	-------------------------------*/
	if (last_char == REDRAW || last_char == SEARCH)
	{
		move (x, y);
		printf ("%s", sptr);

		neg = (btype != CHARTYPE && *sptr == '-');

		sptr = buf + strlen (buf);
	}
	else
		if (btype == CHARTYPE && init_buf)
		{
			move (x, y);
			printf ("%s", sptr);

			sptr = buf + strlen (clip (buf));
		}
		else	
			*sptr = (char) NULL;

	/*-----------------------
	| print '_' for mask	|
	-----------------------*/
	move (x + strlen (buf), y);
	for (i = strlen (buf);i < bufsize;i++)
		putchar ((mask [i] == sigchar) ? sigchar : '_');
	fflush (stdout);

	/*-------------------------------
	| Move to appropriate position	|
	-------------------------------*/
	move (x + strlen (buf), y);
	
	while (TRUE)
	{
		msk_offset = (int) (sptr - buf);

		if (btype == EDATETYPE &&
			mask [msk_offset] == sigchar)
			c = sigchar;
		else
			c = getkey ();
		last_char = c;

		/*-----------------------
		| Perform Upshift	|
		-----------------------*/	
		if (c < 2000 && mask[msk_offset] == 'U')
			c = toupper (c);

		/*-----------------------
		| Perform Downshift	|
		-----------------------*/
		if (c < 2000 && mask[msk_offset] == 'L')
			c = tolower (c);

		/*-------------------------------
		| At a decimal point		|
		| and input is not valid	|
		-------------------------------*/
		if (mask [msk_offset] == sigchar && !strchr (buf, sigchar) &&
			c != BS &&
			c != sigchar &&
			c != REDRAW &&
			c != SEARCH &&
			c != ENDINPUT &&
			c != FN9 && c != FN10 && c != FN11 && c != FN12)
		{
			putchar (BELL);
			fflush (stdout);
			continue;
		}

		/*-------------------------------
		| Special key excluding BS	|
		-------------------------------*/
		if ((c >= 2000 || c < ' ') && c != BS)
		{
			/*---------------
			| Search Key	|
			---------------*/
			if (c == FN4 || c == FN9 || c == FN10 || c == FN11 || c == FN12)
			{
				if (!search_ok)
				{
					putchar (BELL);
					fflush (stdout);
					continue;
				}
				search_key = last_char;
				last_char = SEARCH;
			}
			return;
		}

		switch (c)
		{
		case ENDINPUT:
			return;

		case BS:
			if (sptr == buf)
			{
				putchar (BELL);
				fflush (stdout);
			}
			else
			{
				sptr--;
				*sptr = '\0';
				putchar (BS);
				msk_offset = (int) (sptr - buf);
				putchar ((mask [msk_offset] == sigchar) ? sigchar : '_');
				putchar (BS);
				fflush (stdout);
			}
			break;

		case '.':
			if ((btype == INTTYPE)
			||  (btype == LONGTYPE)
			||  (btype == MONEYTYPE && strchr (buf, sigchar))
			||  (btype == FLOATTYPE && strchr (buf, sigchar))
			||  (btype == DOUBLETYPE && strchr (buf, sigchar)))
			{
				putchar (BELL);
				fflush (stdout);
				break;
			}
			/* fall-thru */

		case '-':
			if (c == '-' && (neg || (sptr != buf && btype != CHARTYPE)))
			{
				putchar (BELL);
				fflush (stdout);
				break;
			}
			else
				if (c == '-' && btype != CHARTYPE && btype != EDATETYPE)
					neg = TRUE;
			/* fall-thru */

		default:
			if (sptr - buf < bufsize)
			{
				switch (btype)
				{
				case MONEYTYPE:
				case DOUBLETYPE:
				case FLOATTYPE:
					if (!(isdigit (c) || c == '-' || c == '.'))
					{
						putchar (BELL);
						fflush (stdout);
						break;
					}

					if (c == MONEYTYPE && 
						(dpos = strchr (buf, sigchar)) && 
						sptr - dpos > 2)
					{
						putchar (BELL);
						fflush (stdout);
						break;
					}
					*sptr++ = (char) c;
					*sptr = '\0';
					putchar (c);
					fflush (stdout);
					break;

				case LONGTYPE:
				case INTTYPE:
					if (!(isdigit (c)))
					{
						putchar (BELL);
						fflush (stdout);
						break;
					}

				default:
					*sptr++ = (char) c;
					*sptr = (char) NULL;
					putchar (c);
					fflush (stdout);
					break;
				}
			}
			else
			{
				putchar (BELL);
				fflush (stdout);
			}
			break;
		}
	}
}

double
getnum (
	int		x, 
	int		y, 
	int		btype, 
	char	*mask)
{
	char	*sptr;
	int		decsize;
	char    temp[21];

	if (!val_type (btype))
		return (0.00);

	read_from (x, y, mask, temp, btype, FALSE);

	dflt_used = (strlen (temp) == 0);
	sptr = strchr (mask, '.');
	decsize = (sptr == (char *) 0) ? 0 : strlen (sptr + 1);
	move (x, y);
	switch (btype)
	{
	case	MONEYTYPE:
	case	DOUBLETYPE:
	case	FLOATTYPE:
		printf ("%*.*f", 
				(int) strlen (mask), decsize, atof (temp));
		break;

	case	LONGTYPE:
		printf ("%*ld", (int) strlen (mask), atol (temp));
		break;

	case	INTTYPE:
		printf ("%*d", (int) strlen (mask), atoi (temp));
		break;
	}
	return (atof (temp));
}

int
getint (
	int		x, 
	int		y, 
	char	*mask)
{
	return ((int) getnum (x, y, INTTYPE, mask));
}

long
getlong (
	int		x, 
	int		y,
	char	*mask)
{
	return ((long) getnum (x, y, LONGTYPE, mask));
}

float
getfloat (
	int		x, 
	int		y, 
	char	*mask)
{
	return ((float) getnum (x, y, FLOATTYPE, mask));
}

double	
getdouble (
	int		x, 
	int		y, 
	char	*mask)
{
	return ((double) getnum (x, y, DOUBLETYPE, mask));
}

double
getmoney (
	int		x, 
	int		y, 
	char	*mask)
{
	return (CENTS ((double) getnum (x, y, MONEYTYPE, mask)));
}

void
getalpha (
	int		x, 
	int		y, 
	char	*mask, 
	char	*buf)
{
	read_from (x, y, mask, buf, CHARTYPE, FALSE);
	dflt_used = (strlen (buf) == 0);
}
void
getalpha2 (
	int		x, 
	int		y, 
	char	*mask, 
	char	*buf)
{
	read_from (x, y, mask, buf, CHARTYPE, TRUE);
	dflt_used = (strlen (buf) == 0);
}

void
get_date (
	int		x, 
	int		y, 
	char	*mask, 
	char	*buf)
{
	read_from (x, y, mask, buf, EDATETYPE, FALSE);
	dflt_used = !*buf;
}

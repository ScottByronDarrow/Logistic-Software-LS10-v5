/*=====================================================================
|  Copyright (C) 2000 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dec.h,v 5.1 2001/11/16 02:26:42 cha Exp $
|=====================================================================|
|  Program Name  : (dec.h)
|  Program Desc  : (Our own implementation of Informix's 32 bit number 
|                   system. This is used when Informix library 
|					functions are not available. )
|=====================================================================|
| $Log: dec.h,v $
| Revision 5.1  2001/11/16 02:26:42  cha
| Updated to free ourselves from Informix's 32 bit number implementation.
| We have implemented  the equivalent of Informix's decimal functions.
|
|
=====================================================================*/

#include	<string.h>
#include	<stdlib.h>

#define ACCSIZE (DECSIZE+1)
struct decacc
  {short dec_exp, dec_pos, dec_ndgts; char dec_dgts [ACCSIZE];};
typedef struct decacc dec_a;

static	char	ds [151];

/*Function Declarations*/
int _stdecimal (_dec_t *dp, unsigned char *cp, int len);
int _lddecimal (unsigned char *cp, int len, _dec_t *dp);
int _round100 (register unsigned char *cp, register int len);
char *_dececvt (_dec_t *np, int dg, int *pt, int *sg);
char *_decefcvt (_dec_t *np, int dg, int *pt, int *sg, int fl);
void _comp100 (register char *cp, int count);
void _dectofix (_dec_t *dp, long *ip);
void _deccopy (_dec_t *src, _dec_t *dst);
static int _deccvasc (char *cp, int ln, _dec_t *rp);
static int _deccvint (int i, _dec_t *dp);
static int _deccvlong (long i, _dec_t *dp);
static int _dectolong (_dec_t *dp, long *ip);
static int _deccvfix (long i, _dec_t *dp);
static int _deccvflt (float flt, _dec_t *dp);
static int _dectoflt (_dec_t *dp, float *fltp);
static int _deccvreal (double dbl, _dec_t *dp, int ndigits);
static int _dectoreal (_dec_t *dp, double *dblp, register int valid);
static int _decadd (_dec_t *x, _dec_t *y, _dec_t *r);
static int _deccmp (_dec_t *x, _dec_t *y);
static int _dec_round (dec_a *s, short c);
static int _decsub (_dec_t *x, _dec_t *y, _dec_t *r);
static int _decmul (_dec_t *x, _dec_t *y, _dec_t *r);
static int _decdiv (_dec_t *x, _dec_t *y, _dec_t *r);

int
_decadd (_dec_t *x, _dec_t *y, _dec_t *r)
{
register short	i,
		j;
	dec_a	*z,
		zv;
	_dec_t	*t;
	short	a,
		c;

	if (x->dec_pos == -1 || y->dec_pos == -1)
	{
		r->dec_pos = -1;
		r->dec_ndgts = 0;
		r->dec_exp = 0;
		return (0);
	}
	z = &zv;
	memset (z, 0, sizeof (dec_a));
	i = x->dec_pos;
	j = y->dec_pos;
	x->dec_pos = 1;
	y->dec_pos = 1;
	if (_deccmp (x, y) < 0)
	{
		t = x;
		x = y;
		y = t;
		c = i;
		i = j;
		j = c;
	}
	x->dec_pos = i;
	y->dec_pos = j;

	memcpy (z, x, sizeof (_dec_t));
	a = x->dec_exp - y->dec_exp;

	if (a > DECSIZE)
	{
		memcpy (r, x, sizeof (_dec_t));
		return (0);
	}
	i = y->dec_ndgts + a;
	if (i > ACCSIZE)
		i = ACCSIZE;
	if (i > z->dec_ndgts)
		z->dec_ndgts = i;
	if ((j = i - a) < 0)
		j = 0;
	c = 0;
	while (i --)
	{
		if (j)
		{
			j -= 1;
			if (x->dec_pos == y->dec_pos)
				c += y->dec_dgts [j];
			else
				c -= y->dec_dgts [j];
		}
		c += z->dec_dgts [i];
		if (c < 0)
		{
			z->dec_dgts [i] = c + 100;
			c = -1;
		}
		else
			if (c < 100)
			{
				z->dec_dgts [i] = c;
				c = 0;
			}
			else
			{
				z->dec_dgts [i] = c - 100;
				c = 1;
			}
	}
	i = _dec_round (z, c);
	memcpy (r, z, sizeof (_dec_t));
	return (i);
}


int
_deccmp (_dec_t *x, _dec_t *y)
{
	short	i,
		s;

	if (x->dec_pos == -1 || y->dec_pos == -1)
		return (-2);
	if ((s = x->dec_pos - y->dec_pos) == 0)
		if ((s = x->dec_exp - y->dec_exp) == 0)
			for (i = 0; i < DECSIZE; i += 1)
			{
				if (i < x->dec_ndgts)
					s += x->dec_dgts [i];
				if (i < y->dec_ndgts)
					s -= y->dec_dgts [i];
				if (s)
					break;
			}
	if (s > 0)
		return (1);
	if (s < 0)
		return (-1);
	return (0);
}

int
_deccvint (int i, _dec_t *dp)
{
	if ((unsigned) i == VAL_DECPOSNULL(int))	/* check for DECNULL */
	{
		dp->dec_pos = -1;
		dp->dec_exp =  0;
		dp->dec_ndgts = 0;
		return (0);
	}

	return (_deccvfix ((long) i, dp));
}

/*
 * *dp			pointer to decimal value to convert
 * *ip			pointer to new home of integer value
 */
int
_dectoint (_dec_t *dp, int *ip)
{
	long	lp;

	if (dp->dec_pos == DECPOSNULL)
		*ip = VAL_DECPOSNULL (int);
	else
	{
		_dectofix (dp, &lp);
		*ip = lp;
	}
	return (0);
}

/*
 * deccvlong:  convert native long integer to internal _dec_t
 * dectolong:  convert internal _dec_t to native long integer
 */
int
_deccvlong (long i, _dec_t *dp)
{
	if (i == VAL_DECPOSNULL (long))	/* check for special case of DECNULL */
	{
		dp->dec_pos = -1;
		dp->dec_exp =  0;
		dp->dec_ndgts = 0;
		return (0);
	}

	return (_deccvfix ((long) i, dp));
}

int
_dectolong (_dec_t *dp, long *ip)
{
	if (dp->dec_pos == DECPOSNULL)
		*ip = VAL_DECPOSNULL (long);
	else
		_dectofix (dp, ip);
	return (0);
}

/*
 * routines common to both int's and long int's
 *
 * fix refers to fixed point (as opposed to floating point)
 */
/*
 * i	long integer to convert
 * *dp	ptr to it's new home
 */
int
_deccvfix (long i, _dec_t *dp)
{
	char	buffer[DECSIZE];
register int	j;

	/*
	 * determine sign and cope with negative values
	 */
	if (i < 0)			/* set up for a negative integer */
	{
		dp->dec_pos = 0;
		i = -i;
	}
	else				/* set up for a positive integer */
		dp->dec_pos = 1;

	/*
	 * form digits list in intermediate buffer
	 * compute correct exponent
	 * compute correct number of valid digits
	 */
	dp->dec_exp = 0;
	j = 0;

	while (i != 0)
	{
		if ((buffer[j] = i % 100) || (j != 0))
			j++;
		dp->dec_exp++;
		i /= 100;
	}

	dp->dec_ndgts = j;

	/*
	 * copy value into structure with ordering
	 */
	while (j != 0)
		dp->dec_dgts[i++] = buffer[--j];

	/*
	 * return the 'success' status
	 */
	return (0);
}

void
_dectofix (_dec_t *dp, long *ip)
{
register long	i = 0;
register char	*digits = dp->dec_dgts;
register int	exp = dp->dec_exp;
register int	valid = dp->dec_ndgts;

	while (exp-- > 0)
	{
		i = i * 100;
		if (valid-- > 0)
			i += *digits++;
	}

	*ip = (dp->dec_pos) ? i : -i;
}

/*
 * _deccvdbl:  convert native double precision floating point
 *            to internal _dec_t
 * dectodbl:  convert internal _dec_t to native double precision
 *            floating point
 */
int
_deccvdbl (double dbl, _dec_t *dp)
{
	/* double: 16 digits of precision */
	return (_deccvreal (dbl, dp, 16));
}

int
_dectodbl (_dec_t *dp, double *dblp)
{
	/* double: 16 digits of precision */
	return (_dectoreal (dp, dblp, 16));
}


/*
 * deccvflt:  convert native single precision floating point
 *            to internal _dec_t
 * dectoflt:  convert internal _dec_t to native single precision
 *            floating point
 */
int
_deccvflt (float flt, _dec_t *dp)
{
	/* float: 8 digits of precision */
	return (_deccvreal (flt, dp, 8));
}

int
_dectoflt (_dec_t *dp, float *fltp)
{
	double	dbl;
	int	status;

	/* float: 8 digits of precision */
	status = _dectoreal (dp, &dbl, 8);
	*fltp = dbl;
	return (status);
}

/*
 * routines common to doubles and floats
 *
 * real refers to real numbers
 * (Yeah, I know.  They really aren't but it was a nice mnemonic.)
 */
/*
 * dbl		floating point value to convert
 * *dp		new home for converted value
 * ndigits	number of digits precision to use
 */
int
_deccvreal (double dbl, _dec_t *dp, int ndigits)
{
	register char	*str;			/* string produced by ecvt () */
	register char	*dgt;			/* pointer to dp->dec_dgts array */
	int	decpt;			/* decimal point position (i.e., exponent) */
	int	sign;			/* sign of floating point value */

	/*
	 * use ecvt to put value into a friendlier format
	 */
	str = ecvt (dbl, ndigits, &decpt, &sign);

	/*
	 * set the sign and exponent of the _dec_t
	 */
	dp->dec_pos = sign ? 0 : 1;
	dp->dec_exp = (decpt + (decpt > 0 ? 1 : 0)) / 2;

	/*
	 * if exponent is odd, first _dec_t digit is [1-9]
	 */
	dgt = dp->dec_dgts;

	if (decpt & 1)
	{
		*dgt++ = *str++ - '0';
		ndigits--;
	}

	/*
	 * copy rest of digits [00-99]
	 */
	while (ndigits-- > 0)
	{
		*dgt++ = (*str++ - '0') * 10;	/* set ten's pos of new digit */
		if (ndigits-- > 0)
			dgt[-1] += *str++ - '0';/* set one's pos of new digit */
	}

	/*
	 * figure out the number of significant digits
	 */
	while (--dgt >= dp->dec_dgts && *dgt == 0);

	dp->dec_ndgts = 1 + (int) dgt - (int) dp->dec_dgts;

	return (0);
}

/*
 * *dp		ptr to _dec_t to convert
 * *dblp	ptr to double to hold converted value
 * valid	max number of significant digits to use
 */
int
_dectoreal (_dec_t *dp, double *dblp, register int valid)
{
	register char	*digits = dp->dec_dgts;
	double	dbl;

	if (valid > dp->dec_ndgts)	/* can't use more digits than exist */
	valid = dp->dec_ndgts;

	dbl = 0.0;			/* initialize value */

	/*
	 * convert the mantissa
	 */
	while (valid-- > 0)		/* copy mantissa */
	dbl = (dbl + digits[valid]) / 100.0;

	if (dp->dec_pos == 0)
		dbl = -dbl;		/* set sign of mantissa */

	/*
	 * convert the exponent
	 */
	if (dp->dec_exp > 0)
	{
		register short i = dp->dec_exp;

		while (i--)
			dbl *= 100.0;
	}
	else
		if (dp->dec_exp < 0)
		{
			register short i = dp->dec_exp;

			while (i++)
				dbl /= 100.0;
		}

	/*
	 * copy value to user's area and you're done
	 */
	*dblp = dbl;

	return (0);
}

/*
 * _stdecimal:  convert internal _dec_t to packed format for storage
 */
int
_stdecimal (_dec_t *dp, unsigned char *cp, int len)
{
unsigned char	*bp;
unsigned char	buffer[DECSIZE];
unsigned char	header;
	int	count;

	if (dp->dec_pos == -1)		/* special case of DECNULL */
	{
		memset (cp, 0, len);
		return (0);
	}

	/*
	 * compute the header byte
	 * and decrement len in recognition
	 * that header occupies 1 byte
	 */

	header = 0xC0 + dp->dec_exp;
	len--;

	/*
	 * if there are valid digits, format them
	 */
	if ((count = dp->dec_ndgts))
	{
		memcpy (buffer, dp->dec_dgts, count);

		/*
		 * round value if necessary
		 */
		if (len < count && buffer[len] >= 50)
			header += _round100 (buffer, len);

		/*
		 * convert negative number if necessary
		 */
		if (dp->dec_pos == 0)
		{
			header = ~header;	/* form 1's complement of header */
			/* form 100's complement of number */
			_comp100 (buffer, len < count ? len : count);
		}
	}

	/*
	 * copy properly formated value into user's buffer
	 */
	*cp = header;

	bp = buffer;
	while (len-- > 0)
		*++cp = (count-- > 0) ? *bp++ : 0;

	return (0);
}

/*
 * _lddecimal:  convert packed format to internal _dec_t for manipulation
 */
int
_lddecimal (unsigned char *cp, int len, _dec_t *dp)
{
unsigned char	buffer[DECSIZE + 1];
register unsigned char
		*digits;

	if (*cp == 0)			/* special case of DECNULL */
	{
		dp->dec_pos = -1;
		dp->dec_exp = 0;
		dp->dec_ndgts = 0;
		return (0);
	}

	/*
	 * make a copy of the number's mantissa
	 */
	if (--len > DECSIZE)		/* internal format has on DECSIZE digits */
		len = DECSIZE;

	memcpy (buffer, cp + 1, len);

	/*
	 * check for negative number and set sign
	 * unpack exponent
	 */
	if (*cp & 0x80)			/* true if number is positive */
	{
		dp->dec_pos = 1;
		dp->dec_exp = *cp - 0xC0;
	}
	else				/* we have a negative number   */
	{
		_comp100 (buffer, len);	/* take 100's complement */
		dp->dec_pos = 0;
		dp->dec_exp = 0xFF - (*cp) - 0xC0;
	}

	/*
	 * compute number of significant digits
	 */
	cp = buffer + len;

	while (len > 0 && *--cp == 0)
		len--;
	dp->dec_ndgts = len;

	/*
	 * copy unpacked mantissa
	 */
	digits = dp->dec_dgts;
	cp = buffer;
	while (len-- > 0)
		*digits++ = *cp++;

	return (0);
}

/*
 * form 100's complement of normalized value in buffer pointed at by cp.
 * count is number of digits in buffer.
 */
void
_comp100 (register char *cp, int count)
{
register int	base;

	base = 100;
	cp += count;
	while (count--)
	{
		cp--;
		*cp = base - *cp;
		if (*cp > 99)
		{
			*cp -= 100;
			base = 100;
		}
		else
			base = 99;
	}
}

/*
 * round the base 100 mantissa pointed at by cp to len digits
 * watch out for possible overflow!
 */
/*
 * cp		ptr to mantissa to round
 * len		# of good digits after rounding
 */
int
_round100 (register unsigned char *cp, register int len)
{
register int	carry = 1;
	int	count = len;

	/*
	 * round the digits provided
	 */
	cp += len;
	while (len-- > 0)
	{
		cp--;
		*cp = *cp + carry;
		if (*cp > 99)
		{
			*cp -= 100;
			carry = 1;
		}
		else
			carry = 0;
	}

	/*
	 * check for overflow
	 * example:  rounding .9999999 to 2 digits causes overflow
	 */
	if (carry)			/* eek! overflow!   */
	{
		for (len = count; --len; )	/* shift value down */
			cp [len - 1] = cp [len];
		cp [0] = 1;		/* set extra digit  */
		return (1);		/* signal overflow  */
	}
	return (0);			/* signal no overflow */
}

int
_deccvasc (char *cp, int ln, _dec_t *rp)
{
register int	c;
	int	ps,
		i,
		j,
		xs,
		xv,
		ms;
	dec_a	*np,
		nv;

	np = &nv;
	memset (np, 0, sizeof nv);
	ps = i = j = xs = xv = ms = 0;
	rp->dec_pos = np->dec_pos = DECPOSNULL;
	np->dec_pos = 1;
	while (i < ln && cp [i] == ' ')
		i += 1;
	if (i == ln)
		return (0);
	if (cp [i] == '-')
	{
		i += 1;
		ms = 0;
	}
	else
	{
		if (cp [i] == '+')
			i++;
		ms = 1;
	}
	while (i < ln)
	{
		c = cp [i++];
		if (c >= '0' && c <= '9')
		{
			if (ps)
				ps--;
			c = c - '0';
			if ((j || c) && j < ACCSIZE * 2)
			{
				if (j & 1)
					np->dec_dgts [j / 2] += c;
				else
					np->dec_dgts [j / 2] = c * 10;
				j += 1;
			}
		}
		else
			if (c == '.')
				if (ps)
					return (-1213);
				else
					ps -= 1;
			else
				break;
	}
	if (i < ln && (c == 'e' || c == 'E'))
	{
		c = cp [i++];
		if (c == '+')
		{
			xs = 1;
			c = cp [i++];
		}
		else
			if (c == '-')
			{
				xs = -1;
				c = cp [i++];
			}
		while (i <= ln)
		{
			if (c < '0' || c > '9')
				break;
			if ((xv = xv * 10 + c - '0') >= 1000)
				return (-1216);
			c = cp [i++];
		}
	}
	if (i < ln)
		if (cp [i] != ' ')
			return (-1213);
	if (xs == -1)
		xv = -xv;
	xv += j + 1;
	if (ps)
		xv += ps + 1;
	np->dec_ndgts = (j + 1) / 2;
	i = xv;
	if (i < 0)
		i--;
	np->dec_exp = i / 2;
	if ((xv & 1) == 0)
	{
		if ((j & 1) == 0)
			np->dec_ndgts++;
		j = 0;
		for (i = 0; i < ACCSIZE; i++)
		{
			xs = np->dec_dgts [i];
			np->dec_dgts [i] = xs / 10 + j;
			j = (xs % 10) * 10;
		}
	}
	i = _dec_round (np, 0);
	np->dec_pos = ms;
	memcpy (rp, np, sizeof (_dec_t));
	return (i);
}

int
_decdiv (_dec_t *x, _dec_t *y, _dec_t *r)
{
register short	j,
		c;
	short	n,
		i,
		m,
		s,
		t,
		u = 0;
	dec_a	*q,
		qv,
		*a,
		av;

	if (x->dec_pos == -1 || y->dec_pos == -1)
	{
		r->dec_pos = -1;
		r->dec_ndgts = 0;
		r->dec_exp = 0;
		return (0);
	}

	if (y->dec_ndgts == 0)
	{
		r->dec_pos = 1;
		r->dec_ndgts = 0;
		r->dec_exp = 0;
		return (-1202);
	}

	q = &qv;
	a = &av;
	memset (q, 0, sizeof (dec_a));
	q->dec_exp = x->dec_exp - y->dec_exp + 1;
	q->dec_pos = x->dec_pos ^ y->dec_pos ^ 1;
	q->dec_ndgts = ACCSIZE;
	memcpy (a, x, sizeof (_dec_t));
	a->dec_exp = a->dec_pos = a->dec_dgts [ACCSIZE - 1] = 0;

	m = -1;
	for (n = 0; n < ACCSIZE; n += 1)
	{
		if (n == 0 || a->dec_dgts [n - 1] == 0)
			i = n;
		else
			i = n - 1;
		if (n != 1 || u != 0)
			m += 1;
		else
			q->dec_exp -= 1;
		t = a->dec_dgts [i] * 100;
		if (i < ACCSIZE - 1)
			t += a->dec_dgts [i + 1];
		t += 1;
		s = y->dec_dgts [0] * 100;
		if (y->dec_ndgts > 1)
			s += y->dec_dgts [1];
		if (i == n)
			u =  t / s;
		else
			u = ((long) t) * 100 / s;
		c = 0;
		if (u)
		{
			if (u > 99)
				u = 99;
			j = y->dec_ndgts;
			if (i + j > ACCSIZE)
			{
				j = ACCSIZE - i;
				c = - (y->dec_dgts [j] * u / 100);
			}
			while (j + n > i)
			{
				j -= 1;
				c += a->dec_dgts [n + j];
				if (j >= 0)
					c -= y->dec_dgts [j] * u;
				if (c < 0)
				{
					a->dec_dgts [n + j] = (c + 10000) % 100;
					c = (c + 1) / 100 - 1;
				}
				else
					if (c > 99)
					{
						a->dec_dgts [n + j] = c % 100;
						c /= 100;
					}
					else
					{
						a->dec_dgts [n + j] = c;
						c = 0;
					}
			}
			if (c < 0)
			{
				c = 0;
				j = y->dec_ndgts;
				if (i + j > ACCSIZE)
					j = ACCSIZE - i;
				u -= 1;
				while (j + n > i)
				{
					j -= 1;
					c += a->dec_dgts [n + j];
					if (j >= 0)
						c += y->dec_dgts [j];
					if (c > 99)
					{
						a->dec_dgts [n + j] = c - 100;
						c = 1;
					}
					else
					{
						a->dec_dgts [n + j] = c;
						c = 0;
					}
				}
			}
		}
		q->dec_dgts [m] = u;
	}
	if (s > 99)
		s = s / 100;
	q->dec_dgts [DECSIZE] = a->dec_dgts [DECSIZE] * 100 / s;
	i = _dec_round (q, 0);
	memcpy (r, q, sizeof (_dec_t));
	return (i);
}

char
*_dececvt (_dec_t *np, int dg, int *pt, int *sg)
{
	return (_decefcvt (np, dg, pt, sg, 0));
}


char
*_decefcvt (_dec_t *np, int dg, int *pt, int *sg, int fl)
{
	int	i,
		j,
		k,
		nd;
	_dec_t	rd;

	ds [0] = '\0';
	if (np -> dec_pos == -1)
		return (ds);
	*sg = np -> dec_pos ^ 1;
	*pt = np -> dec_exp * 2;
	nd = np -> dec_ndgts;
	if (nd && np -> dec_dgts [0] < 10)
		*pt -=1;
	k = dg;
	if (fl)
		k += *pt;
	if (k < 0)
		return (ds);
	i = 0;
	if (nd && np -> dec_dgts [0] < 10)
		i = 1;
	rd.dec_pos = np -> dec_pos;
	rd.dec_ndgts = 1;
	rd.dec_exp = np -> dec_exp - (k + i) / 2;
	if ((k + i) & 1)
		rd.dec_dgts [0] = 5;
	else
		rd.dec_dgts [0] = 50;
	if (nd == 0)
	{
		rd.dec_ndgts = 0;
		rd.dec_dgts [0] = 0;
	}
	if (_decadd (np, &rd, &rd))
		return (ds);
	i = 0;
	*pt = rd.dec_exp * 2;
	if (nd && rd.dec_dgts [0] < 10)
	{
		*pt -=1;
		i = 1;
	}
	if (fl)
		dg += *pt;
	j = 0;
	while (j < dg && j < 151)
	{
		if (i / 2 < rd.dec_ndgts)
			k = rd.dec_dgts [i / 2];
		else
			k = 0;
		if (i & 1)
			k %= 10;
		else
			k /= 10;
		ds [j] = k + '0';
		i += 1;
		j += 1;
	}
	ds [j] = '\0';
	return (ds);
}

char
*_decfcvt (_dec_t *np, int dg, int *pt, int *sg)
{
	return (_decefcvt (np, dg, pt, sg, 1));
}

int
_decmul (_dec_t *x, _dec_t *y, _dec_t *r)
{
	dec_a	*p,
		pv;
	short	i,
		j,
		k = 0;

	if (x->dec_pos == -1 || y->dec_pos == -1)
	{
		r->dec_pos = -1;
		r->dec_ndgts = 0;
		r->dec_exp = 0;
		return (0);
	}
	p = &pv;
	memset (p, 0, sizeof (dec_a));
	for (i = x->dec_ndgts; i >= 0; i--)
	{
		k = 0;
		for (j = y->dec_ndgts - 1; j >= 0; j--)
		{
			if (i + j < ACCSIZE)
			{
				k += p->dec_dgts [i + j] + x->dec_dgts [i] * y->dec_dgts [j];
				p->dec_dgts [i + j] = k % 100;
				k /= 100;
			}
			if (i)
				p->dec_dgts [i - 1] = k;
		}
	}
	p->dec_pos = x->dec_pos ^ y->dec_pos ^ 1;
	p->dec_exp = x->dec_exp + y->dec_exp - 1;
	p->dec_ndgts = x->dec_ndgts + y->dec_ndgts;
	if (k)
		k = _dec_round (p, k);
	else
		p->dec_ndgts -= 1;
	memcpy (r, p, sizeof (_dec_t));
	return (k);
}
static int
_dec_round (dec_a *s, short c)
{
register short	i,
		j;

	if (c > 0)
	{
		i = ACCSIZE;
		while (--i)
			s -> dec_dgts [i] = s->dec_dgts [i - 1];
		s->dec_dgts [0] = c;
		s->dec_exp += 1;
		s->dec_ndgts += 1;
	}
	else
	{
		i = 0;
		j = 0;
		while (s->dec_dgts [j] == 0 && j < s->dec_ndgts)
			j += 1;
		if (j == s->dec_ndgts)
		{
			s->dec_exp = 0;
			s->dec_pos = 1;
		}
		else
			if (j)
			{
				s->dec_exp -= j;
				while (j < s->dec_ndgts)
					s->dec_dgts [i++] = s->dec_dgts [j++];
				while (i < s->dec_ndgts)
					s->dec_dgts [i++] = 0;
			}
	}
	i = DECSIZE;
	if (s -> dec_pos)
		j = 1;
	else
		j = -1;
	if (s->dec_dgts [DECSIZE] > 49)
		while (i--)
		{
			j += s->dec_dgts [i];
			if (j > 99)
			{
				s->dec_dgts [i] = j - 100;
				j = 1;
			}
			else
				if (j < 0)
				{
					s->dec_dgts [i] = j + 100;
					j = -1;
				}
				else
				{
					s->dec_dgts [i] = j;
					break;
				}
		}
	i = s->dec_ndgts;
	if (i > DECSIZE)
		i = DECSIZE;
	while (i--)
		if (s->dec_dgts [i])
			break;
	s->dec_ndgts = i + 1;
	if (s->dec_exp > 0x3f)
	{
		s->dec_exp = 0x3f;
		return (-1200);
	}
	if (s->dec_exp < -0x40)
	{
		s->dec_exp = -0x40;
		return (-1201);
	}
	return (0);
}

int
_decsub (_dec_t *x, _dec_t *y, _dec_t *r)
{
	short	i;

	if (x -> dec_pos == -1 || y -> dec_pos == -1)
	{
		r->dec_pos = -1;
		r->dec_ndgts = 0;
		r->dec_exp = 0;
		return 0;
	}
	y->dec_pos ^= 1;
	i = _decadd (x, y, r);
	if (y != r)
		y->dec_pos ^= 1;
	return (i);
}


int
_dectoasc (_dec_t *np, char *cp, int ln, int dg)
{
	int	i,
		j,
		m,
		t,
		pt,
		sg;
	char	*v;

	memset (cp, ' ', ln);
	if (np->dec_pos == DECPOSNULL)
		return (0);
	if (dg <= 0)
	{
		i = np->dec_ndgts;
		dg = i + i;
		if (dg > 0 && np->dec_dgts [0] < 10)
			dg--;
		if ((dg > 1 && np->dec_dgts [i - 1] % 10) == 0)
			dg--;
		if (dg <= 0)
			dg = 1;
		i = np->dec_pos ^ 1;
		if (dg > ln - i - 1)
			dg = ln - i - 1;
		v = _dececvt (np, dg, &pt, &sg);
		if (pt < 0 && dg + sg - pt + 1 >= ln)
			goto cv_float;
		if (pt < 0)
			dg -= pt;
	}
	v = _decfcvt (np, dg, &pt, &sg);
	i = strlen (v);
	if (pt != i)
		i++;
	i += sg;
	if (i > ln)
	{
		i -= ln;
		if (i <= dg)
			v = _decfcvt (np, dg - i, &pt, &sg);
	}
	i = j = 0;
	if (i < ln && sg)
		cp [i++] = '-';
	if (i < ln && pt <= 0)
		cp [i++] = '0';
	m = pt;
	while (m > 0 && v [j] != '\0' && i < ln)
	{
		cp [i++] = v [j++];
		m--;
	}
	if (i < ln)
		cp [i++] = '.';
	while (m < 0 && i < ln)
	{
		cp [i++] = '0';
		m++;
	}
	while (v [j] != '\0' && i < ln)
		cp [i++] = v [j++];
	if (pt <= ln - sg)
		return (0);

cv_float:
	while (ln)
	{
		memset (cp, ' ', ln);
		m = ln;
		i = 0;
		i = pt - 1;
		if (i < 0)
			i = -i;
		do
		{
			if (m > 0)
				cp [--m] = i % 10 + '0';
			i /= 10;
		} while (m && i);
		if (m && pt <= 0)
			cp [--m] = '-';
		if (m)
			cp [--m] = 'e';
		dg = m - 1;
		i = 0;
		if (sg && m)
		{
			dg -=1;
			cp [i++] = '-';
		}
		if (i >= m)
		{
			if (np->dec_exp < -1)
			{
				memset (cp, ' ', ln);
				cp [0] = '0';
			}
			else
				memset (cp, '*', ln);
			return (0);
		}
		if (dg <= 0)
			dg = 1;
		t = pt;
		v = _dececvt (np, dg, &pt, &sg);
		if (t != pt)
			continue;
		j = 0;
		cp [i++] = v [j++];
		if (i < m)
			cp [i++] = '.';
		while (i < m)
			cp [i++] = v [j++];
		while (m && (cp [--m] == '0' || cp [m] == '\0'))
		{
			for (i = m; i < ln - 1; i++)
				cp [i] = cp [i + 1];
			cp [i] = ' ';
		}
		if (m && cp [m] == '.')
		{
			for (i = m; i < ln - 1; i++)
				cp [i] = cp [i + 1];
			cp [i] = ' ';
		}
		ln = 0;
	}
	return (0);
}


void
_deccopy (_dec_t *src, _dec_t *dst)
{
	memcpy (dst, src, sizeof (_dec_t));
}

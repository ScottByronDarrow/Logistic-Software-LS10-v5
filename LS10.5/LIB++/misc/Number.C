#ident		"$Id: Number.C,v 5.0 2001/06/19 08:19:06 cha Exp $"
/*
 *	Arbitrary precision numbers
 *
 *	Ideas taken from "Numerical Recipes in C"
 *	Ch 20.6 "Arithmetic at Arbitrary Precision"
 *
 *	- 0 < radix <= 255; value needs to fit into unsigned char
 *	- reversed storage; ie lower array element == less significant
 *
 *			<-- less-sig	 more-sig-->
 *
 *			[v0] [v1] [v2] [v3] ... [vN]
 *
 *			<---       valuesz      --->
 *			radix_pt  --->
 *
 *	The big question is to choose your RADIX so that it's big enough
 *	to not waste space but good enough so that it will handle base 10
 *	conversion accurately (since that's we use in real life). The
 *	best candidate is 250.
 *
 *	This implementation holds all values in absolute terms. The sign
 *	is indicated externally, and radix-complement is only used to handle
 *	addition/subtraction
 *
 *	We have to limit the number of values we can represent below the
 *	radix point. An unrepresentable number could cause the representation
 *	to eat up everything. (eg 1/7)
 *
 *	Things to do:
 *		We could make this faster with matrix inversions,
 *		but the code would be *EVEN* harder to understand...
 *
 *******************************************************************************
 *	$Log: Number.C,v $
 *	Revision 5.0  2001/06/19 08:19:06  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:12  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  1999/12/08 23:09:08  jonc
 *	Fixed: Negating 0 causes havoc.
 *	
 *	Revision 1.2  1999/12/05 22:08:08  jonc
 *	Reimplemented to fix problem in representing some -ve numbers.
 *	
 */
#include	<assert.h>

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>

#include	<liberr.h>

#include	<Number.h>
#include	<String.h>

/*
 *
 */
#define	RADIX		250					// so that base 10 displays ok.
#define	SUBRADIX	5					// maximum values below radix point

#define	CHUNKSIZE	8

#define	ABS(x)		((x) < 0 ? -(x) : (x))
#define	MAX(x,y)	((x) > (y) ? (x) : (y))

#define	LOWVAL(x)	((x) % RADIX)
#define	HIVAL(x)	((x) / RADIX)

/*
 *	Local functions
 */
static void
SAdd (
 unsigned char * r,			// this will be overwritten
 const unsigned char * a,
 const unsigned char * b,
 unsigned len);

static void
SMul (
 unsigned char * r,			// this will be overwritten
 unsigned len,
 unsigned d);

static void
SDiv (
 unsigned char * value,		// this will be overwritten
 unsigned char & r,			// remainder result
 unsigned len,
 unsigned d);

static int
LDiv (
 const Number & num,
 const Number & div,
 Number & rmd);

static void
Complement (
 unsigned size,
 unsigned char * value);

static String &
PrePad (
 int len,
 String & s);

/*
 *	Private interface
 */
void
Number::CheckValueSize (
 unsigned minreq)
{
	if (minreq < valuesz)
		return;

	/*
	 *	Need to extend value-array.
	 *	We allocate in chunks of CHUNKSIZE.
	 */
	unsigned newsz = (minreq / CHUNKSIZE + 1) * CHUNKSIZE;
	unsigned char * newvalue = new unsigned char [newsz];

	/*
	 *	Copy over old value
	 */
	memset (newvalue, 0, newsz);
	memcpy (newvalue, value, valuesz);

	/*
	 *	Replace old stuff
	 */
	delete [] value;
	value = newvalue;
	valuesz = newsz;
}

Number &
Number::Trim ()
{
	/*
	 *	Resize value + radix point, if possible
	 */
	unsigned i;

	/*
	 *	Trim off trailing zeroes from sub-radix
	 */
	for (i = 0; i < radixpt && !value [i]; i++);
	if (i)
	{
		/*
		 *	Move everything down
		 */
		unsigned j;

		for (j = 0; j < valuesz - i; j++)
			value [j] = value [j + i];
		radixpt -= i;

		memset (value + j, 0, i);			// clear out top part
	}

	/*
	 *	Trims off leading zeroes from super-radix
	 *
	 *	Attempt deal in chunk-size blocks, but this may
	 *	be impossible since some allocations are done with
	 *	random sizes.
	 */
	for (i = valuesz; i && !value [i - 1] && i > radixpt; i--);
	if (i)
	{
		/*
		 *	attempt boundary align
		 */
		i = (i / CHUNKSIZE + 1) * CHUNKSIZE;
	}

	if (i < valuesz)
	{
		/*
		 *	Reduce value-array to `i' CHUNKS
		 */
		if (i)
		{
			unsigned char * newvalue = new unsigned char [i];

			memcpy (newvalue, value, i);	// only copy significant stuff
			delete [] value;
			value = newvalue;

		} else
		{
			/*
			 *	Zero!
			 */
			delete [] value;
			value = NULL;
			negative = false;
		}

		valuesz = i;
	}

	return *this;
}

/*
 *	Public interface
 */
Number::Number (
 int i_value) :

	valuesz (0),
	value (NULL),

	negative (false),
	radixpt (0)
{
	operator = (i_value);
}

Number::Number (
 long l_value) :

	valuesz (0),
	value (NULL),

	negative (false),
	radixpt (0)
{
	operator = (l_value);
}

Number::Number (
 const char * s_value) :
	valuesz (0),
	value (NULL),

	negative (false),
	radixpt (0)
{
	operator = (s_value);
}

Number::Number (
 double d_value,
 int places) :

	valuesz (0),
	value (NULL),

	negative (false),
	radixpt (0)
{
	char	vstr [512];

	sprintf (vstr, "%.*f", places, d_value);
	operator = (vstr);
}

Number::Number (
 const Number & original) :

	valuesz (original.valuesz),
	value (NULL),

	negative (original.negative),
	radixpt (original.radixpt)
{
	if (valuesz)
	{
		value = new unsigned char [valuesz];
		memcpy (value, original.value, valuesz);
	}
}

Number::~Number ()
{
	delete [] value;
}

Number &
Number::Negate ()
{
	if (value)
		negative = !negative;
	return *this;
}

Number &
Number::RShift ()
{
	if (valuesz)
	{
		if (radixpt >= valuesz)
			CheckValueSize (valuesz + 1);		// request extension

		radixpt++;
	}
	return *this;
}

Number &
Number::LShift ()
{
	if (valuesz)
	{
		if (radixpt)
			radixpt--;
		else
		{
			if (value [valuesz - 1])			// we're chocka-full
				CheckValueSize (valuesz + 1);

			/*
			 *	Move values up
			 */
			for (unsigned i = valuesz - 1; i > 0; i--)
				value [i] = value [i - 1];
			value [0] = 0;
		}
	}
	return (*this);
}

Number &
Number::Trunc ()
{
	/*
	 *	Zap everything from the radix-pt down to zero, then Trim
	 */
	for (unsigned i = 0; i < radixpt; i++)
		value [i] = 0;
	return Trim ();
}
 
Number &
Number::Ceil ()
{
	/*
	 *	Move towards +ve infinity
	 */
	if (negative)
		Trunc ();
	else
	{
		Number	t (*this);

		if (t - Trunc () > 0)
			*this += 1;
	}
	return *this;
}

Number &
Number::Floor ()
{
	/*
	 *	Move towards -ve infinity
	 */
	if (negative)
	{
		Number	t (*this);

		if (Trunc () - t > 0)
			*this -= 1;
	}
	else
		Trunc ();
	return *this;
}

Number &
Number::Round (
 int to,
 enum RN_Type t)
{
	return Round (Number (to), t);
}

Number &
Number::Round (
 long to,
 enum RN_Type t)
{
	return Round (Number (to), t);
}

Number &
Number::Round (
 const Number & to,
 enum RN_Type roundType)
{
	/*
	 *	To round to the nearest multiple of "to", we
	 *	divide the current number by "to", take the
	 *	nearest/upper/lower whole number and then
	 *	multiply that result with the "to".
	 */
	if (to == 0)
		return *this;

	Number r = *this / to;

	switch (roundType)
	{
	case RN_Near:
		r = (ABS (r) + "0.5").Trunc ();
		r.negative = (negative && !to.negative) || (!negative && to.negative);
		break;
	case RN_Upper:
		r.Ceil ();
		break;
	case RN_Lower:
		r.Floor ();
		break;
	}

	return *this = r * to;
}

Number &
Number::Round (
 const char * str)
{
	/*
	 *	We accept strings of the form:
	 *		[type] [Number]
	 *
	 *	type = u | l | n
	 *
	 */
	if (!str)
		return (*this);

	switch (*str)
	{
	case 'l':
		return Round (Number (str + 1), RN_Lower);
	case 'n':
		return Round (Number (str + 1), RN_Near);
	case 'u':
		return Round (Number (str + 1), RN_Upper);
	}
	return *this;
}

Number::operator long () const
{
	long retval = 0;
	Number num (*this);

	if (negative)
		num.Negate ();							// deal with absolutes

	for (unsigned i = num.valuesz; i > num.radixpt; i--)
	{
		long tval = retval;

		if ((retval = retval * RADIX + num.value [i - 1]) < tval)
			(*lib_error_handler) (
				"Number::operator long",
				"Overflow retval=%ld, tval=%ld]",
				retval, tval);
	}

	return negative ? -retval : retval;
}

Number::operator int () const
{
	int retval = 0;
	Number num (*this);

	if (negative)
		num.Negate ();							// deal with absolutes

	for (unsigned i = num.valuesz; i > num.radixpt; i--)
	{
		int	tval = retval;

		if ((retval = retval * RADIX + num.value [i - 1]) < tval)
			(*lib_error_handler) (
				"Number::operator int",
				"Overflow [retval=%d, tval=%d]",
				retval, tval);
	}

	return negative ? -retval : retval;
}

Number::operator double () const
{
	/*
	 *	There are 2 parts to the conversion:
	 *		1. stuff below the radix point
	 *		2. stuff above the radix point
	 */
	unsigned i;
	double hi = 0, lo = 0;
	Number num (*this);

	if (negative)
		num.Negate ();

	/*
	 *	Handle sub-radix values first
	 *	working from least-significant towards the radix point.
	 */
	for (i = 0; i < num.radixpt; i++)
		lo = lo / RADIX + num.value [i];
	if (i)
		lo /= RADIX;

	/*
	 *	Handle super-radix
	 *	working from most-significant towards the radix point
	 */
	for (i = num.valuesz; i > num.radixpt; i--)
		hi = hi * RADIX + num.value [i - 1];

	hi += lo;
	return negative ? -hi : hi;
}

Number::operator float () const
{
	float	retval = operator double ();

	return retval;
}

String &
Number::Get (
 String & buf) const
{
	/*
	 *	To convert, we work outward from the radix point:
	 *		from bottom up for super-radix (lsb -> msb)
	 *		from top down for sub-radix (msb -> lsb)
	 */
	buf = "";

	/*
	 *	Convert the stuff to the left of the radix-point, bottom-up.
	 */
	unsigned super_radix = valuesz - radixpt;

	if (super_radix)
	{
		/*
		 *	Extract the super-radix and then
		 *	continually divide by 10 and get the remainder
		 *	which we prepend to the output-string.
		 *
		 *	Stop when the most-significant value is 0
		 */
		unsigned char * h_value = new unsigned char [super_radix];

		memcpy (h_value, value + radixpt, super_radix);

		/*
		 *	Find most significant value-index
		 */
		int sig;

		for (sig = super_radix - 1; sig >= 0 && !h_value [sig]; sig--);

		while (sig >= 0)
		{
			unsigned char	rem;			// remainder

			SDiv (h_value, rem, super_radix, 10);
			buf.prepend (rem + '0');

			/*
			 *	See whether we need to keep going
			 */
			while (sig >= 0 && !h_value [sig])
				sig--;
		}

		delete [] h_value;
	}

	/*
	 *	Convert sub-radix, msb to lsb
	 */
	if (radixpt)
	{
		/*
		 *	Extract the sub-radix and continually multiply
		 *	by 10, appending the result to the result string
		 */
		Number sub (*this);

		/*
		 *	Clear super-radix
		 */
		memset (sub.value + sub.radixpt, 0, sub.valuesz - sub.radixpt);

		buf += '.';
		while (sub.radixpt)
		{
			sub *= 10;

			if (sub.valuesz > sub.radixpt)
			{
				/*
				 *	Append the result above the radix-point
				 *	and clear it from the value-array
				 */
				buf += '0' + sub.value [sub.radixpt];
				sub.value [sub.radixpt] = 0;

			} else
				buf += '0';
		}
	}

	if (negative)
		buf.prepend ('-');
	if (buf.empty ())
		buf = "0";

	return buf;
}

String &
Number::Get (
 int len,
 String & s) const
{
	/*
	 *	Returns a right justified number
	 */
	Number::Get (s);						// qualifier req'd 'cos it's virtual
	return PrePad (len, s);
}

String &
Number::Get (
 int len,
 int prec,
 String & s) const
{
	int decidx;;
	const char	zero = '0',
				decpt = '.';

	Number::Get (s);						// qualifier req'd 'cos it's virtual

	if ((decidx = s.index (decpt)) < 0)
	{
		/*
		 *	Attempt to add a decimal point + 0 padding
		 */
		int buflen = prec + 1;
		char * buffer = new char [buflen + 1];

		memset (buffer, zero, buflen);
		buffer [0] = decpt;
		buffer [buflen] = '\0';

		s += buffer;

		delete [] buffer;
	} else
	{
		/*
		 *	May need to truncate or extend
		 */
		int	trailer = s.length () - decidx - 1;

		if (trailer < prec)
		{
			/*
			 *	Need to extend
			 */
			int buflen = prec - trailer;
			char * buffer = new char [buflen + 1];

			memset (buffer, zero, buflen);
			buffer [buflen] = '\0';

			s += buffer;

		} else if (trailer > prec)
		{
			/*
			 *	Need to truncate
			 */
			s.del (decidx + prec + 1, trailer - prec);
		}
	}

	return PrePad (len, s);
}

/*
 */
void
Number::Dump (
 const char * msg) const
{
	printf ("%s: RADIX(%d) sz=%u %s",
		msg, RADIX, valuesz, negative ? "-" : "+");

	if (radixpt == valuesz)
		printf (" .");
	for (int i = (int) valuesz - 1; i >= 0; i--)
	{
		printf (" %u", value [i]);
		if (i == (int) radixpt)
			printf (" .");
	}
	putchar ('\n');
}

/*
 *
 */
Number &
Number::operator = (
 int v)
{
	return operator = ((long) v);
}

Number &
Number::operator = (
 long v)
{
	long av = ABS (v);

	/*
	 *	Reset
	 */
	radixpt = 0;
	negative = v < 0;
	memset (value, 0, valuesz);

	/*
	 *	Convert the value
	 */
	for (unsigned i = 0; av; i++)
	{
		CheckValueSize (i);
		value [i] = (unsigned char) (av % RADIX);
		av /= RADIX;
	}

	return *this;
}

Number &
Number::operator = (
 const char * v)
{
	int	i = 0;
	bool isneg = false;

	/*
	 *	Clear out old contents
	 */
	radixpt = 0;
	negative = false;
	memset (value, 0, valuesz);

	if (!v)
		return *this;

	for (i = 0; v [i] && isspace (v [i]); i++)
		;										// skip leading spaces

	switch (v [i])
	{
	case '-':
		isneg = true;							// set only at the end!
		// fall thru'
	case '+':
		i++;
		break;
	}

	while (v [i] && isdigit (v [i]))			// work thru' sig digits
		*this = *this * 10 + (int) (v [i++] - '0');

	if (v [i++] == '.')
	{
		/*
		 *	Make a Number with the trailing digits, and
		 *	add the value to the original
		 *
		 *	Work from the last number towards the radix point
		 */
		int		j;
		Number	n;

		for (j = i; v [j] && isdigit (v [j]); j++);		// find last number
		for (j--; v [j] && isdigit (v [j]); j--)
			n = n / 10 + (int) (v [j] - '0');

		*this += n / 10;
	}

	negative = isneg;
	return *this;
}

Number &
Number::operator = (
 const Number &	v)
{
	valuesz = v.valuesz;
	radixpt = v.radixpt;
	negative = v.negative;

	/*
	 *	Remove value, and take it from the source
	 */
	delete [] value;

	if (valuesz)
	{
		value = new unsigned char [valuesz];
		memcpy (value, v.value, valuesz);

	} else
		value = NULL;

	return *this;
}

Number &
Number::operator += (
 int a)
{
	return *this = operator + (*this, a);
}

Number &
Number::operator += (
 long a)
{
	return *this = operator + (*this, a);
}

Number &
Number::operator += (
 const Number &	a)
{
	return *this = operator + (*this, a);
}

Number &
Number::operator -= (
 int a)
{
	return *this = operator - (*this, a);
}

Number &
Number::operator -= (
 long a)
{
	return *this = operator - (*this, a);
}

Number &
Number::operator -= (
 const Number &	a)
{
	return *this = operator - (*this, a);
}

Number &
Number::operator *= (
 int a)
{
	return *this = operator * (*this, a);
}

Number &
Number::operator *= (
 long a)
{
	return *this = operator * (*this, a);
}

Number &
Number::operator *= (
 const Number &	a)
{
	return *this = operator * (*this, a);
}

Number &
Number::operator /= (
 int a)
{
	return *this = operator / (*this, a);
}

Number &
Number::operator /= (
 long a)
{
	return *this = operator / (*this, a);
}

Number &
Number::operator /= (
 const Number &	a)
{
	return *this = operator / (*this, a);
}

/*
 *	Friend arithmetic operations
 */

/*
 *	Unary operations
 */
Number
operator - (
 const Number &	a)
{
	Number	result (a);

	return result.Negate ();
}

/*
 *	Binary mathematical operations
 */
Number
operator + (
 const Number &	a,
 const Number & b)
{
	/*
	 *	We get a normalised form of the Numbers so
	 *	that operations can be performed more easily
	 */
	Number result;

	unsigned n_rdpt = MAX (a.radixpt, b.radixpt);
	unsigned n_size = MAX (a.valuesz - a.radixpt,
						   b.valuesz - b.radixpt) + n_rdpt;

	unsigned char * na = Normalized (a, n_rdpt, n_size),
				  * nb = Normalized (b, n_rdpt, n_size),
				  * nr = Normalized (result, n_rdpt, n_size + 1);

	/*
	 */
	if (a.negative)
		Complement (n_size, na);
	if (b.negative)
		Complement (n_size, nb);
	SAdd (nr, na, nb, n_size);

	/*
	 *	Check the carry-value
	 */
	if (nr [n_size])
	{
		if (a.negative && b.negative)
			result.negative = true;
		nr [n_size] = 0;			// clear out the carry-info

	} else
	{
		if (a.negative || b.negative)
		{
			/*
			 *	-ve number bigger than +ve number
			 */
			result.negative = true;
		}
	}
	if (result.negative)
		Complement (n_size, nr);	// don't invert the carry-info!

	/*
	 *	Transfer the results to a Number
	 */
	assert (result.valuesz == 0);
	result.valuesz = n_size;
	result.radixpt = n_rdpt;
	result.value = nr;

	delete [] na;
	delete [] nb;

	return result.Trim ();
}

Number
operator - (
 const Number &	a,
 const Number & b)
{
	/*
	 *	Return a - b
	 *
	 *	There's a more efficient way to do this, but for code
	 *	maintenance, we're gonna base this on the addition code
	 */
	Number b1 (b);

	b1.Negate ();

	return a + b1;
}

Number
operator * (
 const Number & a,
 const Number & b)
{
	/*
	 *	Let's do this by the traditional methods, instead of
	 *	involving a convolution of FFT vectors (which I don't
	 *	understand quite that well...)
	 */
	unsigned r_size = MAX (a.valuesz, b.valuesz) * 2;	// max result precision
	unsigned i_size = MAX (a.valuesz, b.valuesz) + 1;	// intermediate prec.

	unsigned i_mult = MAX (a.valuesz, b.valuesz);

	unsigned char * r_value = new unsigned char [r_size + 1];	// final result
	unsigned char * i_value = new unsigned char [i_size];		// iteration

	memset (r_value, 0, r_size);

	/*
	 *	The traditional method involves multiplying
	 *	"b" by every number in "a", summing the result
	 *	into "r_value" each iteration
	 */
	for (unsigned i = 0; i < a.valuesz; i++)
	{
		/*
		 *	The iteration result is overwritten each
		 *	time SMul() is called, so we reset it to "b" each time
		 */
		memset (i_value, 0, i_size);
		memcpy (i_value, b.value, b.valuesz);

		SMul (i_value, i_mult, a.value [i]);

		/*
		 *	Add the intermediate result to the final result,
		 *	offset by appropriate factor.
		 */
		SAdd (r_value + i, r_value + i, i_value, i_size);
	}

	delete [] i_value;

	/*
	 *	Transfer the results
	 */
	Number result;

	result.negative = (a.negative && !b.negative) ||
					  (!a.negative && b.negative);
	result.radixpt = a.radixpt + b.radixpt;

	assert (result.valuesz == 0);
	result.valuesz = r_size;
	result.value = r_value;

	return result.Trim ();
}

Number
operator / (
 const Number &	a,
 long b)
{
	if (!b)
		(*app_error_handler) ("/ (Number,long)", "Divide by zero");

	/*
	 */
	long ab = ABS (b);

	/*
	 *	Check for short cuts
	 */
	if (ab < RADIX)
	{
		/*
		 *	Allow for precision extension during divison,
		 *	but cap a limit if there isn't one already.
		 */
		unsigned n_radix = MAX (a.radixpt, SUBRADIX);
		unsigned n_size = a.valuesz - a.radixpt + n_radix;

		/*
		 */
		unsigned char rem;
		unsigned char * r_value = Normalized (a, n_radix, n_size);

		SDiv (r_value, rem, n_size, (unsigned) ab);

		/*
		 *	Transfer results
		 */
		Number result;

		result.valuesz = n_size;
		result.value = r_value;

		result.radixpt = n_radix;
		result.negative = (a.negative && b > 0) || (!a.negative && b < 0);

		return result.Trim ();

	} else if (ab == RADIX)
	{
		Number result (a);

		result.RShift ().Trim ();

		return result;
	}

	return operator / (a, Number (b));
}

Number
operator / (
 const Number & a,
 const Number & b)
{
	/*
	 *	Division of big numbers by big numbers
	 *
	 *	Since I've opted not to go without math library stuff,
	 *	I may have to repeatedly subtract the divisor against
	 *	the numerator; depending on what I've got.
	 *
	 */
	Number num (ABS (a)),
		   div (ABS (b));

	/*
	 *	Shift radix-point out of divisor
	 */
	while (div.radixpt)
	{
		num.LShift ();
		div.LShift ();
	}

	/*
	 *	Check for short cut
	 */
	if (div <= RADIX)
	{
		long div_as_long = div;

		if ((a.negative && !b.negative) ||
			(!a.negative && b.negative))
		{
			div_as_long = -div_as_long;
		}

		return operator / (num, div_as_long);
	}

	/*
	 *	Ok, we're left with doing it the long, long way.
	 *
	 *	First thing we need to do is to create a massaged
	 *	numerator which has the maximum sub-radix precision
	 *	we will allow. (hate to have infinite divison occur
	 *	on us).
	 */
	unsigned maxprec = num.valuesz - num.radixpt + SUBRADIX;
	unsigned char * n_value = Normalized (num, SUBRADIX, maxprec);

	/*
	 *	For each spot in `lnum's value-array we attempt a divide 
	 */
	Number t;							// intermediate
	Number result;						// final result

	result.CheckValueSize (1);			// ensure stuff to shift with

	/*
	 *	Work from msb down
	 */
	unsigned i;

	for (i = maxprec; i > 0 && !n_value [i - 1]; i--)
		;								// skip leading zero's

	while (i-- > 0)
	{
		Number rmd;						// remainder per iteration

		assert (!t.radixpt);

		t.CheckValueSize (1);			// ensure something to shift with!
		t.LShift ();
		t.value [t.radixpt] = n_value [i];

		int	ntimes = LDiv (t.Trim (), div, rmd);

		result.LShift ();
		result.value [result.radixpt] = ntimes;

		t = rmd;
	}

	delete [] n_value;

	/*
	 *	Fix up radix point and sign
	 */
	result.radixpt = SUBRADIX;
	result.negative = (a.negative && !b.negative) ||
					  (!a.negative && b.negative);

	return result.Trim ();
}

/*
 */
Number
operator + (
 const Number &	a,
 int b)
{
	return operator + (a, Number (b));
}

Number
operator + (
 int a,
 const Number &	b)
{
	return operator + (Number (a), b);
}

Number
operator + (
 const Number &	a,
 long b)
{
	return operator + (a, Number (b));
}

Number
operator + (
 long a,
 const Number & b)
{
	return operator + (Number (a), b);
}

Number
operator + (
 const Number &	a,
 const char * b)
{
	return operator + (a, Number (b));
}

Number
operator + (
 const char * a,
 const Number & b)
{
	return operator + (Number (a), b);
}

Number
operator - (
 int a,
 const Number & b)
{
	return operator - (Number (a), b);
}

Number
operator - (
 const Number & a,
 int b)
{
	return operator - (a, Number (b));
}

Number
operator - (
 long a,
 const Number & b)
{
	return operator - (Number (a), b);
}

Number
operator - (
 const Number & a,
 long b)
{
	return operator - (a, Number (b));
}

Number
operator - (
 const Number & a,
 const char * b)
{
	return operator - (a, Number (b));
}

Number
operator - (
 const char * a,
 const Number & b)
{
	return operator - (Number (a), b);
}

Number
operator * (
 int a,
 const Number & b)
{
	return operator * (Number (a), b);
}

Number
operator * (
 const Number & a,
 int b)
{
	return operator * (a, Number (b));
}

Number
operator * (
 long a,
 const Number & b)
{
	return operator * (Number (a), b);
}

Number
operator * (
 const Number & a,
 long b)
{
	return operator * (a, Number (b));
}

Number
operator * (
 const Number & a,
 const char * b)
{
	return operator * (a, Number (b));
}

Number
operator * (
 const char * a,
 const Number & b)
{
	return operator * (Number (a), b);
}

Number
operator / (
 const Number &	a,
 int b)
{
	return operator / (a, (long) b);
}

Number
operator / (
 int a,
 const Number &	b)
{
	return operator / (Number (a), b);
}

Number
operator / (
 long a,
 const Number & b)
{
	return operator / (Number (a), b);
}

Number
operator / (
 const Number &	a,
 const char * b)
{
	return operator / (a, Number (b));
}

Number
operator / (
 const char * a,
 const Number & b)
{
	return operator / (Number (a), b);
}

/*
 *	Comparators
 */
bool
operator == (
 const Number & a,
 const Number & b)
{
	if (a.negative	!= b.negative ||
		a.valuesz	!= b.valuesz ||
		a.radixpt	!= b.radixpt)
	{
		return false;
	}

	for (unsigned i = 0; i < a.valuesz; i++)
		if (a.value [i] != b.value [i])
			return false;

	return true;
}

bool
operator == (
 int a,
 const Number & b)
{
	return operator == (Number (a), b);
}

bool
operator == (
 const Number & a,
 int b)
{
	return operator == (b, a);
}

bool
operator == (
 long a,
 const Number & b)
{
	return operator == (Number (a), b);
}

bool
operator == (
 const Number & a,
 long b)
{
	return operator == (b, a);
}

bool
operator == (
 const char * a,
 const Number & b)
{
	return operator == (Number (a), b);
}

bool
operator == (
 const Number & a,
 const char * b)
{
	return operator == (a, Number (b));
}

bool
operator != (
 const Number & a,
 const Number & b)
{
	return operator == (a, b) ? false : true;
}

bool
operator != (
 int a,
 const Number & b)
{
	return operator != (Number (a), b);
}

bool
operator != (
 const Number & a,
 int b)
{
	return operator != (b, a);
}

bool
operator != (
 long a,
 const Number & b)
{
	return operator != (Number (a), b);
}

bool
operator != (
 const Number & a,
 long b)
{
	return operator != (b, a);
}

bool
operator > (
 const Number & a,
 const Number & b)
{
	/*
	 *	Check signs for quick results
	 */
	if (a.negative)
	{
		if (!b.negative)
			return false;
	} else
	{
		if (b.negative)
			return true;
	}

	/*
	 *	At this point, both signs are the same
	 *
	 *	We need to work with a Normalized copy of the operands to check
	 *	everything else
	 */
	unsigned max_radix = MAX (a.radixpt, b.radixpt);
	unsigned max_size = MAX (a.valuesz - a.radixpt,
							 b.valuesz - b.radixpt) + max_radix;

	unsigned char * ca = Normalized (a, max_radix, max_size);
	unsigned char * cb = Normalized (b, max_radix, max_size);

	/*
	 *	Convert to RADIX complement if required so that
	 *	comparison can be worked out without worrying about
	 *	the signs
	 */
	if (a.negative)
	{
		Complement (max_size, ca);
		Complement (max_size, cb);
	}

	/*
	 *	At this point, precision and radix_pt is the same
	 *
	 *	Work from most significant to least significant
	 */
	bool result = false;			// assume they're equal

	for (unsigned i = max_size; i > 0; i--)
	{
		if (ca [i - 1] > cb [i - 1])
		{
			result = true;
			break;
		}

		if (ca [i - 1] < cb [i - 1])
		{
			result = false;
			break;
		}
	}

	delete [] ca;
	delete [] cb;

	return result;
}

bool
operator > (
 int			a,
 const Number &	b)
{
	return (operator > (Number (a), b));
}

bool
operator > (
 const Number &	a,
 int			b)
{
	return (operator > (a, Number (b)));
}

bool
operator > (
 long			a,
 const Number &	b)
{
	return (operator > (Number (a), b));
}

bool
operator > (
 const Number &	a,
 long			b)
{
	return (operator > (a, Number (b)));
}

bool
operator < (
 const Number &	a,
 const Number &	b)
{
	return (operator == (a, b) || operator > (a, b) ? false : true);
}

bool
operator < (
 int			a,
 const Number &	b)
{
	return (operator < (Number (a), b));
}

bool
operator < (
 const Number &	a,
 int			b)
{
	return (operator < (a, Number (b)));
}

bool
operator < (
 long			a,
 const Number &	b)
{
	return (operator < (Number (a), b));
}

bool
operator < (
 const Number &	a,
 long			b)
{
	return (operator < (a, Number (b)));
}

bool
operator >= (
 const Number &	a,
 const Number &	b)
{
	return (operator == (a, b) || operator > (a, b));
}

bool
operator >= (
 int			a,
 const Number &	b)
{
	return (operator >= (Number (a), b));
}

bool
operator >= (
 const Number &	a,
 int			b)
{
	return (operator >= (a, Number (b)));
}

bool
operator >= (
 long			a,
 const Number &	b)
{
	return (operator >= (Number (a), b));
}

bool
operator >= (
 const Number &	a,
 long			b)
{
	return (operator >= (a, Number (b)));
}

bool
operator >= (
 const char *	a,
 const Number &	b)
{
	return operator >= (Number (a), b);
}

bool
operator >= (
 const Number &	a,
 const char *	b)
{
	return operator >= (a, Number (b));
}

bool
operator <= (
 const Number &	a,
 const Number &	b)
{
	return (operator == (a, b) || operator < (a, b));
}

bool
operator <= (
 int			a,
 const Number &	b)
{
	return (operator <= (Number (a), b));
}

bool
operator <= (
 const Number &	a,
 int			b)
{
	return (operator <= (a, Number (b)));
}

bool
operator <= (
 long			a,
 const Number &	b)
{
	return (operator <= (Number (a), b));
}

bool
operator <= (
 const Number &	a,
 long			b)
{
	return (operator <= (a, Number (b)));
}

bool
operator <= (
 const char *	a,
 const Number &	b)
{
	return operator <= (Number (a), b);
}

bool
operator <= (
 const Number &	a,
 const char *	b)
{
	return operator <= (a, Number (b));
}

/*
 *	Support
 */
unsigned char *
Normalized (
 const Number & n,
 unsigned radixpt,
 unsigned size)
{
	/*
	 *	Return a value array, set to the given size and radix,
	 *	without changing the value of the number
	 */
	assert (size >= n.valuesz);

	if (!size)
		return NULL;

	/*
	 */
	unsigned char * result = new unsigned char [size];

	memset (result, 0, size);

	/*
	 *	Copy value across, offset by the radix
	 */
	if (radixpt >= n.radixpt)
	{
		/*
		 *	No loss of value
		 */
		memcpy (result + radixpt - n.radixpt, n.value, n.valuesz);

	} else
	{
		/*
		 *	Implicit trimming of sub-radix
		 */
		unsigned diff = n.radixpt - radixpt;

		memcpy (result, n.value + diff, n.valuesz - diff);
	}

	return result;
}

static void
SAdd (
 unsigned char * r,			// this will be overwritten
 const unsigned char * a,
 const unsigned char * b,
 unsigned len)
{
	unsigned i, ireg = 0;

	for (i = 0; i < len; i++)
	{
		ireg = a [i] + b [i] + HIVAL (ireg);
		r [i] = LOWVAL (ireg);
	}
	r [i] = HIVAL (ireg);
}

static void
SMul (
 unsigned char * r,			// this will be overwritten
 unsigned len,
 unsigned d)
{
	unsigned i, ireg = 0;

	for (i = 0; i < len; i++)
	{
		ireg = r [i] * d + HIVAL (ireg);
		r [i] = LOWVAL (ireg);
	}
	r [i] = HIVAL (ireg);
}

static void
SDiv (
 unsigned char * value,		// this will be overwritten
 unsigned char & r,			// remainder result
 unsigned len,
 unsigned d)
{
	assert (d < RADIX);

	r = 0;		// remainder..

	for (int i = (int) len - 1; i >= 0; i--)
	{
		unsigned short	v = r * RADIX + value [i];

		value [i] = (unsigned char) (v / d);
		r = v % d;
	}
}

static int
LDiv (
 const Number & num,
 const Number & div,
 Number & rmd)
{
	/*
	 *	Long division, by subtracting `div' from `num'.
	 *	Computationally intensive, but it's the simplest
	 *	safe way for really *BIG* Numbers
	 */
	int res = 0;

	assert (num >= 0);
	assert (div > 0);

	rmd = num;
	for (res = 0; rmd >= div; res++)
		rmd -= div;

	assert (rmd >= 0);
	assert (res >= 0 && res <= RADIX);
	return res;
}

static void
Complement (
 unsigned size,
 unsigned char * value)
{
	unsigned short ireg = RADIX;

	for (unsigned i = 0; i < size; i++)
	{
		ireg = (RADIX - 1) - value [i] + HIVAL (ireg);
		value [i] = LOWVAL (ireg);
	}
}

/*
 *	Format functions
 */
static String &
PrePad (
 int len,
 String & s)
{
	if (s.length () < (unsigned) len)
	{
		/*
		 *	Prepend buffering spaces
		 */
		int		buflen = len - s.length ();
		char *	buffer = new char [buflen + 1];

		memset (buffer, ' ', buflen);		// put in spaces
		buffer [buflen] = '\0';				// and make it a good string

		s.prepend (buffer);

		delete [] buffer;
	}

	return s;
}

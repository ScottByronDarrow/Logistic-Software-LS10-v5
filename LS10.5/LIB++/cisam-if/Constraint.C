#ident	"$Id: Constraint.C,v 5.0 2001/06/19 08:17:29 cha Exp $"
/*
 *	Query Constraints
 *
 *	There are basically 2 types of constraints
 *		1. conditionals
 *		2. groupings.
 *
 *******************************************************************************
 *	$Log: Constraint.C,v $
 *	Revision 5.0  2001/06/19 08:17:29  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:09  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.8  1997/04/13 22:14:19  jonc
 *	Fixed: Some wildcard searches not returning full-result set
 *
 *	Revision 2.7  1997/04/08 23:59:31  jonc
 *	Added experimental use of && and || operators
 *
 *	Revision 2.6  1997/04/06 23:24:44  jonc
 *	Trim match strings to correspond with extracted strings from db.
 *
 *	Revision 2.5  1996/08/18 04:08:01  jonc
 *	Bug in using Or
 *
 *	Revision 2.4  1996/07/30 00:57:41  jonc
 *	Added #ident directive
 *
 *	Revision 2.3  1996/07/25 23:43:18  jonc
 *	Added support for Matches start and end query. Also fixed termination
 *	of query.
 *
 *	Revision 2.2  1996/06/12 23:34:24  jonc
 *	Added Date Constraints
 *	Mods for port to DEC's C++ compiler
 *
 *	Revision 2.1  1996/04/10 22:10:48  jonc
 *	Added wildcard matching
 *
 *	Revision 2.0  1996/02/13 03:34:49  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:08  jonc
 *	Initial C++ CISAM-Interface entry
 *
 *
 */
#include	<assert.h>
#include	<stddef.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<osdeps.h>
#include	<minor.h>
#include	<liberr.h>
#include	<Constraint.h>

#include	<isam.h>
#include	<decimal.h>
#include	"cisamdefs.h"

#define	CHARBUF	256

enum
{
	ValChars,
	ValDouble,
	ValInt,
	ValLong,

	ValDate
};

/*
 *	Local functions
 */
static bool	Cmp (enum CondCmp, long, long),
			Cmp (enum CondCmp, const char *, const char *),
			Cmp (enum CondCmp, double, double),
			Cmp (enum CondCmp, const Date &, const Date &);

/*
 *	Public interface
 */
Constraint::Constraint ()
{
}

Constraint::~Constraint ()
{
}

/*
 *	Condition
 */
Condition::Condition (
 const Condition &	c) :
	colname (c.colname),
	cond (c.cond),
	valtype (c.valtype)
{
	switch (valtype)
	{
	case ValDouble:
		vdouble = c.vdouble;
		break;
	case ValChars:
		strcpy (vchars = new char [strlen (c.vchars) + 1], c.vchars);
		break;
	case ValInt:
		vint = c.vint;
		break;
	case ValLong:
		vlong = c.vlong;
		break;
	case ValDate:
		vdate = c.vdate;
		break;
	}
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 int			v) :
	colname (name),
	cond (c),
	valtype (ValInt),
	vint (v)
{
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 long			v) :
	colname (name),
	cond (c),
	valtype (ValLong),
	vlong (v)
{
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 const char *	v) :
	colname (name),
	cond (c),
	valtype (ValChars),
	vchars (NULL)
{
	clip (strcpy (vchars = new char [strlen (v) + 1], v));
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 char			v) :
	colname (name),
	cond (c),
	valtype (ValInt),
	vint (v)
{
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 double			v) :
	colname (name),
	cond (c),
	valtype (ValDouble),
	vdouble (v)
{
}

Condition::Condition (
 enum CondCmp	c,
 const char *	name,
 const Date &	v) :
	colname (name),
	cond (c),
	valtype (ValDate),
	vdate (v)
{
}

Condition::~Condition ()
{
	if (valtype == ValChars)
		delete [] vchars;
}

const char *
Condition::Col () const
{
	assert (colname);						// application error
	return (colname);
}

int
Condition::ColNo () const
{
	return (colinfo.ColNo ());
}

enum CondCmp
Condition::Cond () const
{
	return (cond);
}

int
Condition::Int () const
{
	assert (valtype == ValInt);				// application error
	return (vint);
}

long
Condition::Long () const
{
	assert (valtype == ValLong);			// application error
	return (vlong);
}

const char *
Condition::Chars () const
{
	assert (valtype == ValChars);			// application error
	return (vchars);
}

bool
Condition::C () const
{
	return (true);
}

bool
Condition::Fulfilled (
 void *	cisambuf) const
{
	/*
	 *	Sanity checks
	 */
	assert (cisambuf);
	assert (colinfo.type);
	assert (colinfo.ColNo ());

	char	cmpbuf [CHARBUF];		// compare buffer

	switch (colinfo.type)
	{
	case ColChar:
		switch (valtype)
		{
		case ValInt:
			sprintf (cmpbuf, "%d", vint);
			break;
		case ValLong:
			sprintf (cmpbuf, "%ld", vlong);
			break;
		case ValChars:
			return (Cmp (cond, colinfo.CChar (cisambuf), vchars));
		case ValDouble:
			goto bad_conversion;
		}
		return (Cmp (cond, colinfo.CChar (cisambuf), cmpbuf));

	case ColShort:
		switch (valtype)
		{
		case ValInt:
			return (Cmp (cond, (long) colinfo.Short (cisambuf), vint));
		case ValLong:
			return (Cmp (cond, colinfo.Short (cisambuf), vlong));
		case ValChars:
			sprintf (cmpbuf, "%d", colinfo.Short (cisambuf));
			return (Cmp (cond, cmpbuf, vchars));
		case ValDouble:
			return (Cmp (cond, colinfo.Short (cisambuf), (long) vdouble));
		}
		assert (false);				// unreached

	case ColLong:
	case ColSerial:
		switch (valtype)
		{
		case ValInt:
			return (Cmp (cond, colinfo.Long (cisambuf), vint));
		case ValLong:
			return (Cmp (cond, colinfo.Long (cisambuf), vlong));
		case ValChars:
			sprintf (cmpbuf, "%ld", colinfo.Long (cisambuf));
			return (Cmp (cond, cmpbuf, vchars));
		case ValDouble:
			return (Cmp (cond, colinfo.Short (cisambuf), (long) vdouble));
		}
		assert (false);				// unreached

	case ColDouble:
		switch (valtype)
		{
		case ValInt:
			return (Cmp (cond,
						colinfo.Double (cisambuf),
						(double) vint));
		case ValLong:
			return (Cmp (cond,
						colinfo.Double (cisambuf),
						(double) vlong));
		case ValChars:
			goto bad_conversion;
		case ValDouble:
			return (Cmp (cond, colinfo.Double (cisambuf), vdouble));
		}
		assert (false);				// unreached

	case ColFloat:
		switch (valtype)
		{
		case ValInt:
			return (Cmp (cond,
						colinfo.Float (cisambuf),
						(double) vint));
		case ValLong:
			return (Cmp (cond,
						colinfo.Float (cisambuf),
						(double) vlong));
		case ValChars:
			goto bad_conversion;
		case ValDouble:
			return (Cmp (cond, colinfo.Float (cisambuf), vdouble));
		}
		assert (false);				// unreached

	case ColDecimal:
		(*lib_error_handler) ("Condition::Fulfilled",
			"Bug jonc to complete Decimal constraints checks ");

	case ColDate:
		if (valtype != ValDate)
			goto bad_conversion;
		return (Cmp (cond, colinfo.IDate (cisambuf), vdate));

	case ColMoney:
		(*lib_error_handler) ("Condition::Fulfilled",
			"Bug jonc to complete Money constraints checks ");

	case ColTime:
		(*lib_error_handler) ("Condition::Fulfilled", "ColTime unimplemented");
		break;

	default:
		(*lib_error_handler) ("Condition::Fulfilled",
			"Unknown Column type %d", colinfo.type);
	}
	return (true);

bad_conversion:
	(*lib_error_handler) ("Condition::Fulfilled",
		"Conversion not supported");
	return (false);
}

const Condition *
Condition::FindWithColNo (
 short	c) const
{
	return (colinfo.ColNo () == c ? this : (const Condition *) NULL);
}

int
Condition::SignificantLength () const
{
	assert (cond == Mt);

	int	i;

	for (i = 0; vchars [i] && vchars [i] != '*' && vchars [i] != '?'; i++);
	return (i);
}

bool
Condition::SignificantCheck (
 void *	cisambuf) const
{
	/*
	 *	Check for matches against characters leading up to the
	 *	the wildcard
	 */
	assert (cond == Mt);
	assert (colinfo.type == ColChar);

	const int		sigLen = SignificantLength ();
	const char *	colValue = colinfo.CChar (cisambuf);

	for (int i = 0; i < sigLen; i++)
		if (colValue [i] != vchars [i])
			return (false);
	return (true);
}

void
Condition::Put (
 void *	isambuf) const
{
	/*
	 *	Store value into supplied CISAM buffer
	 */

	/*
	 *	Sanity checks
	 */
	assert (isambuf);
	assert (colinfo.type);
	assert (colinfo.ColNo ());

	switch (colinfo.type)
	{
	case ColChar:
	{
		char	cval [CHARBUF];		// storage buffer

		switch (valtype)
		{
		case ValInt:
			sprintf (cval, "%d", vint);
			stchar (cval, (char *) colinfo.COff (isambuf), colinfo.length);
			break;
		case ValLong:
			sprintf (cval, "%ld", vlong);
			stchar (cval, (char *) colinfo.COff (isambuf), colinfo.length);
			break;
		case ValChars:
			if (cond == Mt)
			{
				/*
				 *	We only copy until the first wildcard character,
				 *	since CISAM don't know anything about them.
				 */
				int	i;

				for (i = 0;
					vchars [i] && vchars [i] != '*' && vchars [i] != '?';
					i++)
				{
					cval [i] = vchars [i];
				}
				stchar (cval, (char *) colinfo.COff (isambuf), colinfo.length);
			}
			else
				stchar (vchars, (char *) colinfo.COff (isambuf), colinfo.length);
			break;
		case ValDouble:
			goto bad_conversion;
		}
		break;
	}

	case ColShort:
		switch (valtype)
		{
		case ValInt:
			stint ((short) vint, (char *) colinfo.COff (isambuf));
			break;
		case ValLong:
			stint ((short) vlong, (char *) colinfo.COff (isambuf));
			break;
		case ValChars:
			stint ((short) atoi (vchars), (char *) colinfo.COff (isambuf));
			break;
		case ValDouble:
			stint ((short) vdouble, (char *) colinfo.COff (isambuf));
			break;
		}
		break;

	case ColLong:
	case ColSerial:
		switch (valtype)
		{
		case ValInt:
			stlong (vint, colinfo.COff (isambuf));
			break;
		case ValLong:
			stlong (vlong, colinfo.COff (isambuf));
			break;
		case ValChars:
		{
			long	lval = 0;			// storage buffer

			sscanf (vchars, "%ld", &lval);
			stlong (lval, colinfo.COff (isambuf));
			break;
		}

		case ValDouble:
			stlong ((long) vdouble, colinfo.COff (isambuf));
			break;
		}
		break;

	case ColDouble:
		switch (valtype)
		{
		case ValInt:
			stdbl (vint, colinfo.COff (isambuf));
			break;
		case ValLong:
			stdbl (vlong, colinfo.COff (isambuf));
			break;
		case ValChars:
		{
			double	dval = 0;

			sscanf (vchars, "%lf", &dval);
			stdbl (vdouble, colinfo.COff (isambuf));
			break;
		}
		case ValDouble:
			stdbl (vdouble, colinfo.COff (isambuf));
			break;
		}
		break;

	case ColFloat:
		switch (valtype)
		{
		case ValInt:
			stfloat (vint, colinfo.COff (isambuf));
			break;
		case ValLong:
			stfloat (vlong, colinfo.COff (isambuf));
			break;
		case ValChars:
		{
			float	fval = 0;

			sscanf (vchars, "%f", &fval);
			stfloat (vdouble, colinfo.COff (isambuf));
			break;
		}
		case ValDouble:
			stfloat ((float) vdouble, colinfo.COff (isambuf));
			break;
		}
		break;

	case ColDecimal:
		(*lib_error_handler) ("Condition::Put",
			"Bug jonc to complete Decimal constraints checks ");

	case ColDate:
		if (valtype != ValDate)
			goto bad_conversion;
		stlong (vdate.InformixDate (), colinfo.COff (isambuf));
		break;

	case ColMoney:
		(*lib_error_handler) ("Condition::Put",
			"Bug jonc to complete Money constraints checks ");

	case ColTime:
		(*lib_error_handler) ("Condition::Put", "ColTime unimplemented");
		break;

	default:
		(*lib_error_handler) ("Condition::Put",
			"Unknown Column type %d", colinfo.type);
	}

	return;

bad_conversion:
	(*lib_error_handler) ("Condition::Put", "Conversion not supported");
}

/**************************************************************************
 *	CGroup class
 *
 *		Constraint Grouping
 *
 **************************************************************************/
void
CGroup::ClearConstraints ()
{
	/*
	 *	Clear existing conditions
	 */
	int	i;

	for (i = 0; i < andC; i++)
		delete andGrp [i];
	for (i = 0; i < orC; i++)
		delete orGrp [i];

	andC = orC = 0;
}

CGroup::CGroup () :
	andC (0),
	orC (0)
{
}

CGroup::CGroup (
 const CGroup &	c) :
	andC (0),
	orC (0)
{
	operator = (c);
}

CGroup::CGroup (
 const Condition &	c) :
	andC (0),
	orC (0)
{
	And (c);
}

CGroup::~CGroup ()
{
	ClearConstraints ();
}

CGroup &
CGroup::And (
 const Constraint &	c)
{
	if (c.C ())
		andGrp [andC] = new Condition ((const Condition &) c);
	else
		andGrp [andC] = new CGroup ((const CGroup &) c);
	andC++;
	return (*this);
}

CGroup &
CGroup::Or (
 const Constraint &	c)
{
	if (c.C ())
		orGrp [orC] = new Condition ((const Condition &) c);
	else
		orGrp [orC] = new CGroup ((const CGroup &) c);
	orC++;
	return (*this);
}

CGroup &
CGroup::operator = (
 const Condition &	c)
{
	ClearConstraints ();

	And (c);
	return (*this);
}

CGroup &
CGroup::operator = (
 const CGroup &	c)
{
	ClearConstraints ();

	/*
	 *	It's a Group
	 */
	int	i;

	for (i = 0; i < c.andC; i++)
		And (*c.andGrp.Elem (i));
	for (i = 0; i < c.orC; i++)
		Or (*c.orGrp.Elem (i));
	return (*this);
}

int
CGroup::AndC () const
{
	return (andC);
}

const Constraint &
CGroup::And (
 int	i) const
{
	if (i >= andC)
		(*lib_error_handler) ("CGroup::And", "boundary violation");
	return (*andGrp.Elem (i));
}

int
CGroup::OrC () const
{
	return (orC);
}

const Constraint &
CGroup::Or (
 int	i) const
{
	if (i >= orC)
		(*lib_error_handler) ("CGroup::Or", "boundary violation");
	return (*orGrp.Elem (i));
}

bool
CGroup::C () const
{
	return (false);
}

bool
CGroup::Fulfilled (
 void *	cisambuf) const
{
	int	i;

	/*
	 *	Check the OR conditions
	 */
	for (i = 0; i < orC; i++)
		if (orGrp.Elem (i) -> Fulfilled (cisambuf))
			return (true);

	if (!andC)
		return (false);

	/*
	 *	Check the AND conditions
	 */
	for (i = 0; i < andC; i++)
		if (!andGrp.Elem (i) -> Fulfilled (cisambuf))
			return (false);

	return (true);
}

const Condition *
CGroup::FindWithColNo (
 short	c) const
{
	/*
	 *	At the moment, for use by Query, we only need
	 *	to check the AND clauses
	 */
	for (int i = 0; i < andC; i++)
	{
		const Condition *	cond = andGrp.Elem (i) -> FindWithColNo (c);

		if (cond)
			return (cond);
	}
	return (NULL);
}

/*
 *	Associated operators
 */
CGroup
operator && (
 const Constraint &	a,
 const Constraint &	b)
{
	CGroup	result;

	if (a.C ())
		result.And ((const Condition &) a);
	else
		result.And ((const CGroup &) a);

	return (result.And (b));
}

CGroup
operator || (
 const Constraint &	a,
 const Constraint &	b)
{
	CGroup	result;

	if (a.C ())
		result.And ((const Condition &) a);
	else
		result.And ((const CGroup &) a);
	return (result.Or (b));
}

/*
 *	Support functions
 */
static bool
Cmp (
 enum CondCmp	c,
 long			a,
 long			b)
{
	switch (c)
	{
	case Eq:
		return ((bool) (a == b));
	case Ne:
		return ((bool) (a != b));
	case Gt:
		return ((bool) (a > b));
	case Ge:
		return ((bool) (a >= b));
	case Lt:
		return ((bool) (a < b));
	case Le:
		return ((bool) (a <= b));
	case Mt:
		(*app_error_handler) ("Constraint Cmp(long,long)",
			"Match condition used on numeric arguments");
	}
	(*lib_error_handler) ("Cmp(long, long)", "Unknown Condition?");
	return (false);
}

static bool
Cmp (
 enum CondCmp	c,
 const char *	value,
 const char *	target)
{
	switch (c)
	{
	case Eq:
		return (strcmp (value, target) ? false : true);
	case Ne:
		return (strcmp (value, target) ? true : false);
	case Gt:
		return (strcmp (value, target) > 0 ? true : false);
	case Ge:
		return (strcmp (value, target) >= 0 ? true : false);
	case Lt:
		return (strcmp (value, target) < 0 ? true : false);
	case Le:
		return (strcmp (value, target) <= 0 ? true : false);
	case Mt:
		return (wildmat (value, target));
	}
	(*lib_error_handler) ("Cmp(const char *, const char *)",
		"Unknown Condition?");
	return (false);
}

static bool
Cmp (
 enum CondCmp	c,
 double			a,
 double			b)
{
	switch (c)
	{
	case Eq:
		return ((bool) (a == b));
	case Ne:
		return ((bool) (a != b));
	case Gt:
		return ((bool) (a > b));
	case Ge:
		return ((bool) (a >= b));
	case Lt:
		return ((bool) (a < b));
	case Le:
		return ((bool) (a <= b));
	case Mt:
		(*app_error_handler) ("Constraint Cmp(double,double)",
			"Match condition used on numeric arguments");
	}
	(*lib_error_handler) ("Cmp(double, double)", "Unknown Condition?");
	return (false);
}

static bool
Cmp (
 enum CondCmp	c,
 const Date &	a,
 const Date &	b)
{
	switch (c)
	{
	case Eq:
		return ((bool) (a == b));
	case Ne:
		return ((bool) (a != b));
	case Gt:
		return ((bool) (a > b));
	case Ge:
		return ((bool) (a >= b));
	case Lt:
		return ((bool) (a < b));
	case Le:
		return ((bool) (a <= b));
	case Mt:
		(*app_error_handler) ("Constraint Cmp(Date,Date)",
			"Match condition used on Date arguments");
	}
	(*lib_error_handler) ("Cmp(Date, Date)", "Unknown Condition?");
	return (false);
}

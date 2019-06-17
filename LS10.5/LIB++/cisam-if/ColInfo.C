#ident	"$Id: ColInfo.C,v 5.0 2001/06/19 08:17:28 cha Exp $"
/*
 *	Column information and converter
 *
 *******************************************************************************
 *	$Log: ColInfo.C,v $
 *	Revision 5.0  2001/06/19 08:17:28  cha
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
 *	Revision 2.6  1997/09/24 03:14:31  jonc
 *	Added kludge for old-style dbif bug
 *
 *	Revision 2.5  1997/07/10 02:56:51  jonc
 *	Added several const accesors and tightened argument usage
 *
 *	Revision 2.4  1996/11/26 02:52:21  jonc
 *	Localised global
 *
 *	Revision 2.3  1996/07/30 00:57:41  jonc
 *	Added #ident directive
 *
 *	Revision 2.2  1996/06/12 23:33:38  jonc
 *	Added Date converter
 *
 *	Revision 2.1  1996/04/10 22:09:14  jonc
 *	Tightened argument usage
 *
 *	Revision 2.0  1996/02/13 03:34:49  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:07  jonc
 *	Initial C++ CISAM-Interface entry
 *
 */
#include	<assert.h>

#include	<osdeps.h>
#include	<minor.h>

#include	<ColInfo.h>

#include	<String.h>			// Containers
#include	<Date.h>
#include	<Money.h>

#include	<isam.h>
#include	<decimal.h>
#include	"cisamdefs.h"

#define	CHARBUF	256				// internal char buf max size for fast convert

static char	sbuf [CHARBUF + 1];	// static buffer

ColumnInfo::ColumnInfo () :
	type (ColBad),
	length (0),
	precision (0),

	c_off (0),
	a_off (0),
	colno (0)
{
}

unsigned
ColumnInfo::COff () const
{
	return (c_off);
}

unsigned
ColumnInfo::AOff () const
{
	return (a_off);
}

void *
ColumnInfo::AOff (
 void *	appbuf) const
{
	return (((char *) appbuf) + a_off);
}

void *
ColumnInfo::COff (
 void *	cisambuf) const
{
	return (((char *) cisambuf) + c_off);
}

short
ColumnInfo::ColNo () const
{
	return (colno);
}


const char *
ColumnInfo::CChar (
 const void *	cisambuf) const
{
	assert (length < CHARBUF);	// look, i hate this but i can't get around it

	ldchar (((char *) cisambuf) + c_off, length, sbuf);
	return (sbuf);
}

const char *
ColumnInfo::Decimal (
 const void *	cisambuf) const
{
	dec_t	d;

	if (lddecimal (
			((char *) cisambuf) + c_off,
			DECLEN (length, precision),
			&d) ||
		d.dec_pos == DECPOSNULL)
	{
		/*
		 *	Play it safe, put a zero value
		 */
		return ("0");
	}
	dectoasc (&d, sbuf, CHARBUF, precision);
	return (sbuf);
}

double
ColumnInfo::Double (
 const void *	cisambuf) const
{
	short	nullflg;

	return (lddblnull (((char *) cisambuf) + c_off, &nullflg));
}

float
ColumnInfo::Float (
 const void *	cisambuf) const
{
	short	nullflg;

	return (ldfltnull (((char *) cisambuf) + c_off, &nullflg));
}

long
ColumnInfo::Long (
 const void *	cisambuf) const
{
	return (ldlong (((char *) cisambuf) + c_off));
}

short
ColumnInfo::Short (
 const void *	cisambuf) const
{
	return (ldint (((char *) cisambuf) + c_off));
}

Date
ColumnInfo::IDate (
 const void *	cisambuf) const
{
	Date	idate;
	long	rawvalue = Long (cisambuf);

	if (!rawvalue ||			// kludge for old C-dbif bug
		rawvalue == LONGNULL)
	{
		return idate;
	}

	return idate.InformixDate (rawvalue);
}

Date &
ColumnInfo::aDate (
 void *	appbuf) const
{
	return (*((Date *) (((char *) appbuf) + a_off)));
}

double &
ColumnInfo::aDouble (
 void *	appbuf) const
{
	return (*((double *) (((char *) appbuf) + a_off)));
}

float &
ColumnInfo::aFloat (
 void *	appbuf) const
{
	return (*((float *) (((char *) appbuf) + a_off)));
}

long &
ColumnInfo::aLong (
 void *	appbuf) const
{
	return (*((long *) (((char *) appbuf) + a_off)));
}

Money &
ColumnInfo::aMoney (
 void *	appbuf) const
{
	return (*((Money *) (((char *) appbuf) + a_off)));
}

Number &
ColumnInfo::aNumber (
 void *	appbuf) const
{
	return (*((Number *) (((char *) appbuf) + a_off)));
}

short &
ColumnInfo::aShort (
 void *	appbuf) const
{
	return (*((short *) (((char *) appbuf) + a_off)));
}

String &
ColumnInfo::aString (
 void *	appbuf) const
{
	return (*((String *) (((char *) appbuf) + a_off)));
}

/*
 *	Const versions returning immutable objects
 */
const Date &
ColumnInfo::cDate (
 const void *	appbuf) const
{
	return (*((const Date *) (((const char *) appbuf) + a_off)));
}

double
ColumnInfo::cDouble (
 const void *	appbuf) const
{
	return (*((const double *) (((const char *) appbuf) + a_off)));
}

float
ColumnInfo::cFloat (
 const void *	appbuf) const
{
	return (*((const float *) (((const char *) appbuf) + a_off)));
}

long
ColumnInfo::cLong (
 const void *	appbuf) const
{
	return (*((const long *) (((const char *) appbuf) + a_off)));
}

const Money &
ColumnInfo::cMoney (
 const void *	appbuf) const
{
	return (*((const Money *) (((const char *) appbuf) + a_off)));
}

const Number &
ColumnInfo::cNumber (
 const void *	appbuf) const
{
	return (*((const Number *) (((const char *) appbuf) + a_off)));
}

short
ColumnInfo::cShort (
 const void *	appbuf) const
{
	return (*((const short *) (((const char *) appbuf) + a_off)));
}

const String &
ColumnInfo::cString (
 const void *	appbuf) const
{
	return (*((const String *) (((const char *) appbuf) + a_off)));
}

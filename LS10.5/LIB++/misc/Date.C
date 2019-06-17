#ident	"$Id: Date.C,v 5.0 2001/06/19 08:19:05 cha Exp $"
/*
 *	Date using Julian day numbers.
 *
 *	Adapted from "The C Users Journal", Feb 1993, page 30
 *	Based on algorithms presented by H Fliegl and T Van Flanders,
 *	Communications of the ACM, Vol. 11, No 10., Oct 1968, page 657
 *
 *******************************************************************************
 *	$Log: Date.C,v $
 *	Revision 5.0  2001/06/19 08:19:05  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.4  1999/05/21 01:15:16  jonc
 *	Corrected setting NULL date via SetDMY
 *	
 *	Revision 1.3  1999/05/21 01:11:13  jonc
 *	Added capablity for setting NULL date setting via dmy
 *	
 *	Revision 1.2  1998/02/11 02:16:59  jonc
 *	Minor aestethic change
 *
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.11  1997/11/20 22:25:55  jonc
 *	Localised string arrays
 *
 *	Revision 2.10  1997/11/20 20:48:04  jonc
 *	Fixed bug with leap year determination
 *
 *	Revision 2.9  1997/03/09 21:56:27  jonc
 *	Fixed: underflow not affecting year
 *
 *	Revision 2.8  1996/07/30 00:52:59  jonc
 *	Added #ident directive
 *
 *	Revision 2.7  1996/07/15 23:09:34  jonc
 *	Fixed: Addmonth() overflow
 *
 *	Revision 2.6  1996/07/09 01:14:43  jonc
 *	Added DefaultFormat support
 *
 *	Revision 2.5  1996/05/03 00:36:35  jonc
 *	Made all Get() functions const
 *
 *	Revision 2.4  1996/05/02 03:00:40  jonc
 *	Handling NULL dates in GetDMY
 *
 *	Revision 2.3  1996/04/23 02:16:44  jonc
 *	Renamed NumDays to DaysInMonth
 *
 *	Revision 2.2  1996/04/23 01:56:59  cam
 *	Added comparison operators.
 *
 *	Revision 2.1  1996/04/23 00:18:16  cam
 *	Fixed inverse logic error in LeapYr ().
 *	Added NumDays () member function.
 *
 *	Revision 2.0  1996/02/13 03:34:56  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:45  jonc
 *	Initial C++ Support library
 *
 */
#include	<stdio.h>
#include	<time.h>

#include	<osdeps.h>
#include	<liberr.h>
#include	<snips.h>

#include	<Date.h>
#include	<String.h>

/*
 *	Some magic numbers
 */
#define	INFORMIXDATEOFFSET	2415020L

/*
 *	Some common strings
 */
static const char
	*	AbbrDayNames [] =
		{
			"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
		},

	*	FullDayNames [] =
		{
			"Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday", "Sunday"
		},

	*	AbbrMonthNames [] =
		{
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		},

	*	FullMonthNames [] =
		{
			"January", "February", "March", "April",
			"May", "June", "July", "August",
			"September", "October", "November", "December"
		};

static const int		MonthDays [] =
{
	31,		// January
	28,		// February
	31,		// March
	30,		// April
	31,		// May
	30,		// June
	31,		// July
	31,		// August
	30,		// September
	31,		// October
	30,		// November
	31,		// December
};

const char *
Date::DefaultFormat ()
{
	/*
	 *	The correct thing to do would be to have it examine
	 *	some config file or environment variable - but
	 *	we'll leave that for later
	 */
	return ("%d/%m/%Y");
}

Date::Date () :
	julday (0)
{
}

Date::Date (
 int	d,
 int	m,
 int	y) :
	julday (0)
{
	SetDMY (d, m, y);
}

bool
Date::Null () const
{
	return julday ? false : true;
}

void
Date::GetDMY (
 int &	d,
 int &	m,
 int &	y) const
{
	if (!julday)
	{
		d = m = y = 0;
		return;
	}

	long	t1, t2, yr, mo;

	t1 = julday + 68569;
	t2 = 4 * t1 / 146097;
	t1 = t1 - (146097 * t2 + 3) / 4;

	yr = 4000 * (t1 + 1) / 1461001;
	t1 = t1 - 1461 * yr / 4 + 31;
	mo = 80 * t1 / 2447;

	d = (int) (t1 - 2447 * mo / 80);

	t1 = mo / 11;

	m = (int) (mo + 2 - 12 * t1);
	y = (int) (100 * (t2 - 49) + yr + t1);
}

String &
Date::Get (
 String &	result) const
{
	return Get (DefaultFormat (), result);
}

String &
Date::Get (
 const char *	mask,
 String &		result) const
{
	if (!mask)
		(*app_error_handler) ("Date::Get", "NULL mask");

	result = "";			// clear the result String

	if (!julday)			// NULL date
		return (result);

	int		d, m, y;
	char	numbuf [64];	// buffer for numeric conversion

	GetDMY (d, m, y);

	for (int pmask = 0; mask [pmask]; pmask++)
	{
		if (mask [pmask] == '%')
		{
			switch (mask [++pmask])
			{
			case 'a':		// abbreviated weekday name
				result += AbbrDayNames [julday % 7];
				break;

			case 'A':		// full weekday name
				result += FullDayNames [julday % 7];
				break;

			case 'b':		// abbreviated month name
			case 'h':
				result += AbbrMonthNames [m - 1];
				break;

			case 'B':		// full month name
				result += FullMonthNames [m - 1];
				break;

			case 'd':		// day of month (01-31)
				sprintf (numbuf, "%02d", d);
				result += numbuf;
				break;

			case 'e':		// day of month (1 - 31)
				sprintf (numbuf, "%d", d);
				result += numbuf;
				break;

			case 'j':		// day of year (1 - 366)
			{
				int		fd, fm, fy;
				Date	first (*this);

				first.GetDMY (fd, fm, fy);
				first.SetDMY (1, 1, fy);

				sprintf (numbuf, "%d", (*this - first) + 1);
				result += numbuf;
				break;
			}

			case 'm':		// month of year (01 - 12)
				sprintf (numbuf, "%02d", m);
				result += numbuf;
				break;

			case 'y':		// year of century (00 - 99)
				sprintf (numbuf, "%02d", y % 100);
				result += numbuf;
				break;

			case 'Y':		// Century
				sprintf (numbuf, "%04d", y);
				result += numbuf;
				break;

			case '%':
				result += '%';
				break;

			default:
				(*app_error_handler) (
					"Date::Get",
					"Bad mask token %%%c", mask [pmask]);
			}
		}
		else
			result += mask [pmask];
	}

	return result;
}

Date &
Date::SetNull ()
{
	julday = 0;
	return *this;
}

Date &
Date::SetDMY (
 int	d,
 int	m,
 int	y)
{
	if (d || m || y)
	{
		julday = d - 32075L + 1461L *
			(y + 4800 + (m - 14L) / 12L) /
			4L + 367L * (m - 2L - (m - 14L) / 12L * 12L) / 12L -
			3L * ((y + 4900L + (m - 14L) / 12L) / 100L) / 4L;
	} else
	{
		julday = 0;
	}
	return *this;
}

Date &
Date::Today ()
{
	time_t		now = time (NULL);
	struct tm *	tnow = localtime (&now);

	return SetDMY (tnow -> tm_mday, tnow -> tm_mon + 1, tnow -> tm_year + 1900);
}

Date &
Date::SetDayOfMonth (
 int	day)
{
	int	d, m, y;

	GetDMY (d, m, y);
	return SetDMY (day, m, y);
}

Date &
Date::SetMonthOfYear (
 int	month)
{
	int	d, m, y;

	GetDMY (d, m, y);
	return SetDMY (d, month, y);
}

Date &
Date::SetYear (
 int	year)
{
	int	d, m, y;

	GetDMY (d, m, y);
	return SetDMY (d, m, year);
}

Date &
Date::AddDays (
 int	days)
{
	julday += days;
	return (*this);
}

Date &
Date::AddMonths (
 int	months)
{
	int	d, m, y;

	GetDMY (d, m, y);

	m--;						// normalise to 0-11
	m += months;

	/*
	 *	When checking to see year underflows/overflows
	 *	we have to use absolute values 'cos the results
	 *	of a -ve operator on '/' or '%' is undefined
	 */
	if (m < 0)
	{
		y -= Abs (m) / 12 + 1;	// since it's -ve, at least 1 yr has backed up
		m = (12 - Abs (m) % 12) % 12;
	}
	else
	{
		y += m / 12;
		m %= 12;
	}

	if (d > MonthDays [m])		// ensure we stay in the same month
		d = MonthDays [m];

	m++;						// set back to 1-12
	return SetDMY (d, m, y);
}

Date &
Date::AddYears (
 int	years)
{
	int	d, m, y;

	GetDMY (d, m, y);
	return SetDMY (d, m, y + years);
}

Date &
Date::operator += (
 int	days)
{
	return AddDays (days);
}

Date &
Date::operator -= (
 int	days)
{
	return AddDays (-days);
}

int
Date::DaysInMonth () const
{
	int		numDays;
	int		d, m, y;

	GetDMY (d, m, y);

	numDays = MonthDays [m - 1];
	if (m == 2 && LeapYr ())
		numDays++;

	return numDays;
}

enum DayOfWeek
Date::Day () const
{
	return (enum DayOfWeek) (julday % 7);
}

bool
Date::LeapYr () const
{
	int	d, m, y;

	GetDMY (d, m, y);

	return (y % 4 == 0 && y % 100 != 0) || !(y % 400);
}

Date &
Date::InformixDate (
 long	informixdate)
{
	julday = informixdate + INFORMIXDATEOFFSET;
	return *this;
}

long
Date::InformixDate () const
{
	return julday - INFORMIXDATEOFFSET;
}

/*
 *	Friend externals
 */
int
operator - (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday - d2.julday;
}

bool
operator == (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday == d2.julday;
}

bool
operator != (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday != d2.julday;
}

bool
operator < (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday < d2.julday;
}

bool
operator > (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday > d2.julday;
}

bool
operator <= (
 const Date &	d1,
 const Date &	d2)
{
	return d1.julday <= d2.julday;
}

bool
operator >= (
 const Date &	d1,
 const Date &	d2)
{
	return (d1.julday >= d2.julday);
}

Date
operator + (
 const Date &	date,
 int			days)
{
	Date	t (date);

	return t.AddDays (days);
}

Date
operator - (
 const Date &	date,
 int			days)
{
	Date	t (date);

	return t.AddDays (-days);
}

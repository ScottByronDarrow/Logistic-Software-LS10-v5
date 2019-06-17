#ident	"$Id: pDate.c,v 5.1 2001/08/06 22:40:54 scott Exp $"
/*
 *	Extended date routines
 *
 *	These return year values as Century
 *
 *******************************************************************************
 *	$Log: pDate.c,v $
 *	Revision 5.1  2001/08/06 22:40:54  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 06:59:36  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:38  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:24  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:17  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.4  2000/01/04 03:56:33  scott
 *	Update to default FullYear to 4.
 *	
 *	Revision 1.3  1999/11/19 03:55:51  scott
 *	Updated for removal of old date routines
 *	
 *	Revision 1.2  1999/09/13 06:20:48  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:34  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.3  1998/08/18 02:11:52  kirk
 *	PSL 14573 - DaysInMonthYear (..) incorrect. AddMonths (..) incorrect.
 *	
 *	Revision 1.2  1998/05/15 03:52:11  jonc
 *	Corrected and extended, based on C++ libraries.
 *
 *	Revision 1.1.1.1  1998/01/22 00:58:38  jonc
 *	Version 10 start
 *
 *	Revision 2.3  1998/01/21 00:02:05  jonc
 *	Rewrote to fix very subtle bugs
 *
 *	Revision 2.2  1997/05/06 21:56:42  jonc
 *	Fixed: weird dates on 0 dates
 *
 *	Revision 2.1  1997/04/28 03:43:52  jonc
 *	Added for change of Century.
 *
 */
#include	<std_decs.h>

#define		OFFSET_TO_1_1_1900 2415020L

static const int		MonthDays [] =
{
	31,		/* January		*/
	28,		/* February		*/
	31,		/* March		*/
	30,		/* April		*/
	31,		/* May			*/
	30,		/* June			*/
	31,		/* July			*/
	31,		/* August		*/
	30,		/* September	*/
	31,		/* October		*/
	30,		/* November		*/
	31,		/* December		*/
};

/*
 *	Internal functions
 */
static long
JulianDate (
 int	d,
 int	m,
 int	y)
{
	return  d - 32075L + 1461L *
				(y + 4800 + (m - 14L) / 12L) /
				4L + 367L * (m - 2L - (m - 14L) / 12L * 12L) / 12L -
				3L * ((y + 4900L + (m - 14L) / 12L) / 100L) / 4L;
}

static void
JulianToDMY (
 long	julday,
 int *	d,
 int *	m,
 int *	y)
{
	long	t1, t2, yr, mo;

	t1 = julday + 68569;
	t2 = 4 * t1 / 146097;
	t1 = t1 - (146097 * t2 + 3) / 4;

	yr = 4000 * (t1 + 1) / 1461001;
	t1 = t1 - 1461 * yr / 4 + 31;
	mo = 80 * t1 / 2447;

	if (d)
		*d = (int) (t1 - 2447 * mo / 80);

	t1 = mo / 11;

	if (m)
		*m = (int) (mo + 2 - 12 * t1);

	if (y)
		*y = (int) (100 * (t2 - 49) + yr + t1);
}

/*
 *	External interface
 */
void
DateToDMY (
 Date	date,
 int *	d,
 int *	m,
 int *	y)
{
	/*
	 *	Translate an Informix date to day, month and century
	 *
	 *	Use algo from the Big-Red-Book,
	 *	ie convert date to std julian day, and then run it thru'
	 */
	if (date <= 0)
	{
		if (d)
			*d = 0;
		if (m)
			*m = 0;
		if (y)
			*y = 0;
	} else
	{
		JulianToDMY (date + OFFSET_TO_1_1_1900, d, m, y);
	}
}

Date
DMYToDate (
 int	d,
 int	m,
 int	y)
{
	/*
	 *	Return number of days since 1-1-1900
	 *
	 *	Obtain std julian and then convert back
	 */
	return JulianDate (d, m, y) - OFFSET_TO_1_1_1900;
}

int
DaysInMonth (
 Date	date)
{
	/*
	 *	Return number of days in Month
	 */

	int month, year;

	DateToDMY (date, NULL, &month, &year);
	return (DaysInMonthYear (month, year));
}

int
DaysInMonthYear (
 int month,
 int year)
{
	if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || !(year % 400)))
		return 29;

	return MonthDays [month - 1];
}

int
DaysInYear (
 Date	date)
{
	/*
	 *	Return number of days in Year
	 */
	return IsLeapYear (date) ? 366 : 365;
}

enum _DayOfWeek
DayOfWeek (
 Date	date)
{
	return (enum _DayOfWeek) ((date + OFFSET_TO_1_1_1900) % 7);
}

int
IsWeekDay (
 Date	date)
{
	enum _DayOfWeek dayOfWeek = DayOfWeek (date);
	return (dayOfWeek != Dy_Sat && dayOfWeek != Dy_Sun);
}

int
IsLeapYear (
 Date	date)
{
	int	d, m, y;

	DateToDMY (date, &d, &m, &y);

	return (y % 4 == 0 && y % 100 != 0) || !(y % 400);
}

Date
AddMonths (
 Date	date,
 int	months)
{
	int	d, m, y;

	DateToDMY (date, &d, &m, &y);

	m--;						/* normalise to 0-11 */
	m += months;

	/*
	 *	When checking to see year underflows/overflows
	 *	we have to use absolute values 'cos the results
	 *	of a -ve operator on '/' or '%' is undefined
	 */
	if (m < 0)
	{
		y -= abs (m) / 12 + 1;	/* it's -ve, at least 1 yr has backed up */
		m = (12 - abs (m) % 12) % 12;
	}
	else
	{
		y += m / 12;
		m %= 12;
	}

	m++;						/* set back to 1-12 */

	if (d > DaysInMonthYear (m, y))		/* ensure we stay in the same month */
		d = DaysInMonthYear (m, y);

	return DMYToDate (d, m, y);
}

Date
AddYears (
 Date	date,
 int	years)
{
	int	d, m, y;

	DateToDMY (date, &d, &m, &y);
	return DMYToDate (d, m, y + years);
}

Date
MonthStart (
 Date date)
{
	/*
	 *	Returns first date of month
	 *
	 *	This function returns the month start date in
	 *	calendar format of the month in the date passed to it.
	 */

	int month,
		year;

	DateToDMY (date, NULL, &month, &year);
	return (DMYToDate (1, month, year));
}

Date
MonthEnd (
 Date date)
{
	/*
	 *	Returns month end date
	 *
	 *	This function returns the month end date in
	 *	calendar format of month in the date passed to it.
	 */

	int month,
		year;

	DateToDMY (date, NULL, &month, &year);
	return (DMYToDate (DaysInMonthYear (month, year), month, year));
}
int
FullYear(void)
{
	char	*sptr;

	sptr = getenv ("DBDATE");
	if (sptr)
	{
		upshift (sptr);

		if (strchr (sptr, '2') != (char *) 0)
			return (EXIT_SUCCESS);
	}
	return(1);
}

/*
 *	Date conversion routines
 *
 *******************************************************************************
 *	$Log: DateToString.c,v $
 *	Revision 5.0  2001/06/19 06:59:11  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/03/15 00:21:01  scott
 *	Updated to make small changes to make diff with LS10-GUI easier
 *	
 *	Revision 4.0  2001/03/09 00:52:33  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:17  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:12  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.6  1999/09/14 03:40:59  scott
 *	Anci mods.
 *	
 *	Revision 1.5  1999/09/14 00:06:06  scott
 *	Updated for better date usage.
 *	Updated to use ver10 sleeper
 *	Updated to use ver10 tty_slot
 *	
 *	Revision 1.3  1999/08/31 08:03:18  scott
 *	Updated to deal with YY and YYYY dates.
 *	
 *	Revision 1.2  1999/07/14 23:32:38  jonc
 *	Added checks for corrupted date conversion.
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.1  1998/07/13 01:00:28  jonc
 *	Y2K changes, mainly done by colin.
 *	
 */
#include	<assert.h>
#include	<std_decs.h>

/*
 *	Some common strings
 */
static const char * AbbrDayNames [] =
{
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static const char * FullDayNames [] =
{
	"Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday", "Sunday"
};

static const char *	AbbrMonthNames [] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char *	FullMonthNames [] =
{
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

/*
 *	External interface
 */
const char *
MonthName (
int	month)
{
	assert (month >= 1 && month <= 12);

	return (FullMonthNames [month - 1]);
}

const char *
ShortMonthName (
 int	month)
{
	assert (month >= 1 && month <= 12);

	return (AbbrMonthNames [month - 1]);
}

const char *
DateToString (
 Date	ldays)
{
	int		fourYear	=	0;
	char	dateMask [9];
	char 	*sptr;
	static char		strbuf [32];

	if (ldays <= 0L)			/* NULL (or corrupted) date */
	{
		strcpy (strbuf, "00/00/0000");
		return (strbuf);
	}

	if ((sptr = getenv ("DBDATE")))
		upshift (sptr);

	/*--------------------------------
	| Check for four character date. |
	--------------------------------*/
	fourYear	=	0;
	if ((strchr (sptr, '4') != (char *) 0))
		fourYear	=	1;

	if 
	(
		sptr && 
		(
			!strncmp (sptr, "YMD", 3) ||
			!strncmp (sptr, "Y2MD", 4) ||
			!strncmp (sptr, "Y4MD", 4)
		)
	)
		strcpy (dateMask, (fourYear) ? "%Y/%m/%d" : "%y/%m/%d");
	else 
	if 
	(
		sptr && !strncmp (sptr, "MDY", 3)
	)
		strcpy (dateMask, (fourYear) ? "%m/%d/%Y" : "%m/%d/%y");
	else 
		strcpy (dateMask, (fourYear) ? "%d/%m/%Y" : "%d/%m/%y");

	return DateToFmtString (ldays, dateMask, strbuf);
}
const char *
DateToDDMMYY (
 Date	ldays)
{
	int		fourYear	=	0;

	int		dy, mn, yr;
	char *	sptr;
	static char		strbuf [32];
	char	dateMask [9];

	if (ldays <= 0L)			/* NULL (or corrupted) date */
	{
		strcpy (strbuf, "00/00/00");
		return (strbuf);
	}

	DateToDMY (ldays, &dy, &mn, &yr);

	if ((sptr = getenv ("DBDATE")))
		upshift (sptr);

	if ((strchr (sptr, '4') != (char *) 0))
		fourYear	=	1;

	if 
	(
		sptr && 
		(
			!strncmp (sptr, "YMD", 3) ||
			!strncmp (sptr, "Y2MD", 4) ||
			!strncmp (sptr, "Y4MD", 4)
		)
	)
		strcpy (dateMask, (fourYear) ? "%Y/%m/%d" : "%y/%m/%d");
	else if (sptr && !strncmp (sptr, "MDY", 3))
		strcpy (dateMask, (fourYear) ? "%m/%d/%Y" : "%m/%d/%y");
	else 
		strcpy (dateMask, (fourYear) ? "%d/%m/%Y" : "%d/%m/%y");

	return DateToFmtString (ldays, dateMask, strbuf);
}

char *
DateToFmtString (
 Date			date,
 const char *	mask,
 char *			result)
{
	int		pmask;
	int		d, m, y;
	char	numbuf [64];	/* buffer for numeric conversion */
	char *	daysuffix;

	if (!result)
		return NULL;

	result [0] = '\0';		/* clear the result buffer */
	if (date <= 0)			/* NULL date (or corrupted date) */
		return (result);

	DateToDMY (date, &d, &m, &y);

	for (pmask = 0; mask [pmask]; pmask++)
	{
		if (mask [pmask] == '%')
		{
			switch (mask [++pmask])
			{
			case 'a':		/* abbreviated weekday name */
				strcat (result, AbbrDayNames [DayOfWeek (date)]);
				break;

			case 'A':		/* full weekday name */
				strcat (result, FullDayNames [DayOfWeek (date)]);
				break;

			case 'b':		/* abbreviated month name */
			case 'h':
				strcat (result, AbbrMonthNames [m - 1]);
				break;

			case 'B':		/* full month name */
				strcat (result, FullMonthNames [m - 1]);
				break;

			case 'd':		/* day of month (01-31) */
				sprintf (numbuf, "%02d", d);
				strcat (result, numbuf);
				break;

			case 'e':		/* day of month (1 - 31) */
				sprintf (numbuf, "%d", d);
				strcat (result, numbuf);
				break;

			case 'j':		/* day of year (1 - 366) */
				sprintf (numbuf, "%ld", DMYToDate (1, m, y) - date + 1);
				strcat (result, numbuf);
				break;

			case 'm':		/* month of year (01 - 12) */
				sprintf (numbuf, "%02d", m);
				strcat (result, numbuf);
				break;

			  case 's':		/* suffixy bit */
				switch (d)
				{
				  case 1:
				  case 21:
				  case 31:
					daysuffix = "st";
					break;

				  case 2:
				  case 22:
					daysuffix = "nd";
					break;

				  case 3:
				  case 23:
					daysuffix = "rd";
					break;

				  default:
					daysuffix = "th";
					break;
				}

				strcat (result, daysuffix);
				break;

			case 'y':		/* year of century (00 - 99) */
				sprintf (numbuf, "%02d", y % 100);
				strcat (result, numbuf);
				break;

			case 'Y':		/* Century */
				sprintf (numbuf, "%04d", y);
				strcat (result, numbuf);
				break;

			case '%':
				strcat (result, "%");
				break;

			}
		} else
		{
			size_t	l = strlen (result);

			result [l] = mask [pmask];
			result [l + 1] = '\0';
		}
	}

	return result;
}

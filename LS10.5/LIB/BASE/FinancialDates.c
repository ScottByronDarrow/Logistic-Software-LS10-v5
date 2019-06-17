/*
 *	Financial year routines
 *
 *******************************************************************************
 *	$Log: FinancialDates.c,v $
 *	Revision 5.1  2001/08/06 22:40:51  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 06:59:11  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:33  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2001/01/29 03:43:39  scott
 *	Updated to add simple function YearEnd.
 *	
 *	Revision 3.0  2000/10/12 13:34:17  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:12  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.2  1999/09/13 06:20:46  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.2  1998/05/07 00:36:29  kirk
 *	Always one year off fix.
 *	
 *	Revision 1.1  1998/05/05 05:06:39  kirk
 *	Set of Financial year date functions.
 *
 */
#include	<assert.h>
#include	<stdlib.h>
#include	<ptypes.h>
#include	<pDate.h>
#include	<FinancialDates.h>

Date
FinYearStart (
 Date date,
 int fiscal)
{
	int month,
		year;

	assert (1 <= fiscal && fiscal <= 12);

	DateToDMY (date, NULL, &month, &year);
                
	if (month <= fiscal)
		year--;

	month = fiscal + 1;

	return (DMYToDate (1, month, year));
}

Date
FinYearEnd (
 Date date,
 int fiscal)
{
	int month,
		year;

	assert (1 <= fiscal && fiscal <= 12);

	DateToDMY (date, NULL, &month, &year);
                  
	if (month > fiscal)
		year++;

	return (DMYToDate (DaysInMonthYear (fiscal, year), fiscal, year));
}

void
GetFinYear (
 Date date,
 int fiscal,
 Date *start,
 Date *end)
{
	assert (1 <= fiscal && fiscal <= 12);

	if (start)
		*start = FinYearStart (date, fiscal);

	if (end)
		*end = FinYearEnd (date, fiscal);
}

void
DateToFinDMY (
 Date date,
 int fiscal,
 int *day,
 int *month,
 int *year)
{
	int		_day;
	int		_month;
	int		_year;

	assert (1 <= fiscal && fiscal <= 12);

	DateToDMY (date, &_day, &_month, &_year);

	/*-----------------------------------
	| %= Allows for fiscal set to 12	|
	-----------------------------------*/
	_month -= fiscal;
	if (_month < 1)
	{
		_month += 12;
	}
	else
		_year++;

	if (day)
		*day = _day;
	if (month)
		*month = _month;
	if (year)
		*year = _year;
}

Date
FinDMYToDate (
 int fiscal,
 int day,
 int month,
 int year)
{

	assert (1 <= fiscal && fiscal <= 12);

	/*-----------------------------------
	| %= Allows for fiscal set to 12	|
	-----------------------------------*/
	month += fiscal;
	if (month > 12)
	{
		month -= 12;
	}
	else
		year--;

	return DMYToDate (day, month, year);
}

Date
YearEnd	(void)
{
	extern	long	int	TodaysDate (void);

	int		year;

	DateToDMY (TodaysDate (), NULL, NULL, &year); 

	return	DMYToDate (DaysInMonthYear (12, year), 12, year); 
}

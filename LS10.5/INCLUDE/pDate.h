#ifndef	_pDate_h
#define	_pDate_h
/*	$Id: pDate.h,v 5.0 2001/06/19 06:51:47 cha Exp $
 *
 *
 *******************************************************************************
 *	$Log: pDate.h,v $
 *	Revision 5.0  2001/06/19 06:51:47  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:28  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:58  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:43  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.2  1999/11/19 03:35:36  scott
 *	Updated to remove old date routines.
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.1  1998/04/26 23:47:16  jonc
 *	Y2K conversion
 *
 */
enum _DayOfWeek
{
	Dy_Mon = 0,
	Dy_Tue,
	Dy_Wed,
	Dy_Thu,
	Dy_Fri,
	Dy_Sat,
	Dy_Sun
};

extern void				DateToDMY (Date, int * d, int * m, int * y);
extern Date				DMYToDate (int d, int m, int y);

extern int				DaysInMonth (Date),
						DaysInMonthYear (int, int),
						DaysInYear (Date);

extern enum _DayOfWeek	DayOfWeek (Date);
extern int				IsWeekDay (Date);

extern int				IsLeapYear (Date);

extern Date				AddMonths (Date, int);
extern Date				AddYears (Date, int);

extern Date				MonthStart (Date),
						MonthEnd (Date);
#endif	/* _pDate_h */

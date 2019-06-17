#ifndef	_DateToString_h
#define	_DateToString_h
/*	$Id: DateToString.h,v 5.0 2001/06/19 06:51:15 cha Exp $
 *
 *	Date/String conversion routines
 *
 *******************************************************************************
 *	$Log: DateToString.h,v $
 *	Revision 5.0  2001/06/19 06:51:15  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:20  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/20 03:15:59  scott
 *	Updated to comment out code on #endif line, prevents warnings on HP compiler.
 *	
 *	Revision 3.0  2000/10/12 13:28:50  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:34  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.1  1998/09/03 23:03:59  jonc
 *	Y2K changes. Probably made by colin.
 *	
 */
extern const char * MonthName (int);				/* 1 = January, 0 = core */
extern const char * ShortMonthName (int);			/* 1 = Jan, 0 = core */
extern Date			StringToDate (const char *);	/* DBDATE dependant */
extern const char *	DateToString (Date);			/* DBDATE dependant */
extern const char *	DateToDDMMYY (Date);			/* DBDATE dependant */

/*
 *	Convert date to formatted string
 *
 *	The function returns the third argument.
 *
 *	Format mask must be composed of the following tokens
 *
 *		%a	- abbreviated weekday name
 *		%A	- full weekday name
 *		%b	- abbreviated month name
 *		%B	- full month name
 *		%d	- day of month (01 - 31)
 *		%e	- day of month (1 - 31)
 *		%h	- synonym for %b
 *		%j	- day of year (1 - 366)
 *		%m	- month of year (01 - 12)
 *		%s	- suffixy bit - st nd rd etc.
 *		%y	- year of century (00 - 99)
 *		%Y	- century
 *
 *		%%	- output %
 *
 *	The mask formats are based on those from date(1)
 *
 */
extern char *		DateToFmtString (Date, const char * fmt, char * buffer);

#endif	/* _DateToString_h */

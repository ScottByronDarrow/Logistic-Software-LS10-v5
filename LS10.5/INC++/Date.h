#ifndef	_Date_h
#define	_Date_h
/*	$Id: Date.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	Date
 *
 *******************************************************************************
 *	$Log: Date.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 */
enum DayOfWeek
{
	Dy_Mon = 0,
	Dy_Tue,
	Dy_Wed,
	Dy_Thu,
	Dy_Fri,
	Dy_Sat,
	Dy_Sun
};

class String;

class Date
{
	public:
		static const char *	DefaultFormat (void);

	private:
		long	julday;				// julian day number

	public:
		Date ();
		Date (int d, int m, int y);

		/*
		 *	Accessors
		 */
		bool		Null (void) const;
		void		GetDMY (int &, int &, int &) const;
		String &	Get (String &) const,
			   &	Get (const char * mask, String &) const;

		/*
		 *	Mutators
		 */
		Date &		SetNull (void);
		Date &		SetDMY (int, int, int);

		/*
		 *	Other stuff
		 */
		Date &			Today (void);

		Date &			SetDayOfMonth (int),
			 &			SetMonthOfYear (int),
			 &			SetYear (int);

		Date &			AddDays (int),
			 &			AddMonths (int),
			 &			AddYears (int);

		Date &			operator += (int);			// synonym for AddDays()
		Date &			operator -= (int);			// synonym for -AddDays()

		int				DaysInMonth (void) const;
		enum DayOfWeek	Day (void) const;
		bool			LeapYr (void) const;

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

		/*
		 *	Convenience for db/if
		 */

		Date &		InformixDate (long);
		long		InformixDate (void) const;

	// end{Undocumented-Interface}

	/*
	 *	Friend operators
	 */
	friend int	operator - (const Date &, const Date &);

	friend bool	operator == (const Date &, const Date &);
	friend bool	operator != (const Date &, const Date &);
	friend bool	operator < (const Date &, const Date &);
	friend bool	operator > (const Date &, const Date &);
	friend bool	operator <= (const Date &, const Date &);
	friend bool	operator >= (const Date &, const Date &);
};

/*
 *	Other externals
 */
extern Date	operator + (const Date &, int);
extern Date	operator - (const Date &, int);

/*
 *	Format mask for Date::Get (mask, String &)
 *	must be composed of the following tokens
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
 *		%y	- year of century (00 - 99)
 *		%Y	- century
 *
 *		%%	- output %
 *
 *	The mask formats are based on those from date(1)
 *
 */
#endif	//_Date_h

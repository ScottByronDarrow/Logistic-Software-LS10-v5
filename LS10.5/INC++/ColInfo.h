#ifndef	_ColInfo_h
#define	_ColInfo_h
/*	$Id: ColInfo.h,v 5.0 2002/05/08 01:50:42 scott Exp $
 *
 *	{libdbif:CISAM}
 *
 *	The members of this structure are mainly public 'cos the
 *	classes that make use of it would be friends anyway.
 *
 *******************************************************************************
 *	$Log: ColInfo.h,v $
 *	Revision 5.0  2002/05/08 01:50:42  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:42  scott
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
/*
 *	Column types
 */
enum ColumnType
{
	ColBad = 0,
	ColChar,
	ColShort,
	ColLong,
	ColDouble,
	ColFloat,
	ColDecimal,
	ColSerial,
	ColDate,
	ColMoney,
	ColTime
};

class Date;
class Number;
class Money;
class String;

class ColumnInfo
{
	public:
		enum ColumnType	type;
		unsigned		length,
						precision;

	public:
		ColumnInfo ();

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	friend	class Database;

	private:
		unsigned		c_off,			// offset within internal CISAM buffer
						a_off;			// offset within application buffer

		short			colno;			// useful for system catalog searches

	public:
		unsigned	COff (void) const,	// c_off accessor
					AOff (void) const;
		short		ColNo (void) const;

		void *	COff (void *) const;	// cisam memory offset
		void *	AOff (void *) const;	// application memory offset

		/*
		 *	Conversions
		 */

		//	Conversion from CISAM buffer
		const char *	CChar (const void *) const;
		const char *	Decimal (const void *) const;
		double			Double (const void *) const;
		float			Float (const void *) const;
		long			Long (const void *) const;
		short			Short (const void *) const;
		Date			IDate (const void *) const;

		//	Conversion from application buffer
		Date &			aDate (void *) const;
		double &		aDouble (void *) const;
		float &			aFloat (void *) const;
		long &			aLong (void *) const;
		Money &			aMoney (void *) const;
		Number &		aNumber (void *) const;
		short &			aShort (void *) const;
		String &		aString (void *) const;

		const Date &	cDate (const void *) const;
		double			cDouble (const void *) const;
		float			cFloat (const void *) const;
		long			cLong (const void *) const;
		const Money &	cMoney (const void *) const;
		const Number &	cNumber (const void *) const;
		short			cShort (const void *) const;
		const String &	cString (const void *) const;

	// end{Undocumented-Interface}
};

#endif	//_ColInfo_h

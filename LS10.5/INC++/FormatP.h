#ifndef	_FormatP_h
#define	_FormatP_h
/*	$Id: FormatP.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	Simple front end to the format-p filter
 *
 *******************************************************************************
 *	$Log: FormatP.h,v $
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
 *	Revision 1.2  2000/08/09 23:50:58  johno
 *	Add email functionality to the FormatP object: Add subject and email
 *	address attributes. Pass these to format-p.
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 */
#include	<stdio.h>

class ColInfo;
class Date;
class Number;
class Money;
class Query;
class Table;
class ColumnInfo;

class FormatP
{
	private:
		FILE *	output;

	private:
		void	Init (const char * layout, const char * optionblock);
		void	SubmitColumn (const Query &, const char *, const ColumnInfo &);
		void	SubmitClear (const char *, const ColumnInfo &);
		void	UseSection (const char * base, const char * alt);

	public:
		FormatP (const char * layout,
					const char * out,						// program/file
					const char * optionblock = 0);
		FormatP (const char * layout,
					int lpno,								// printer dest.
					const char * optionblock = 0);
		FormatP (const char * layout,
					int lpno,								// printer dest.
					const char * address,
					const char * subject );
		virtual ~FormatP ();

		virtual void	Reset ();
		virtual void	UsePageHeader (const char * = 0),
						UsePageTrailer (const char * = 0),
						UseBody (const char * = 0);

		virtual void	Submit (const Query &),
						Submit (const char * key, const char * value),
						Submit (const char * key, float),
						Submit (const char * key, double),
						Submit (const char * key, const Date & value),
						Submit (const char * key, const Number & value),
						Submit (const char * key, const Money & value);

		virtual void	SubmitClear (const Table &);

		virtual void	BatchEnd ();
};

#endif	//	_FormatP_h

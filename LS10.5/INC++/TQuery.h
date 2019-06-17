#ifndef	_TQuery_h
#define	_TQuery_h
/*	$Id: TQuery.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	Typed Query.
 *
 *	A simple template descendant that enforces type check for Querys
 *
 *******************************************************************************
 *	$Log: TQuery.h,v $
 *	Revision 5.0  2002/05/08 01:50:44  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
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
#include	<Query.h>

template <class RecordType>
class TQuery :
	public Query
{
	private:
		/*
		 *	Change access method on these overridden virtuals
		 *	to prevent inadvertant use
		 */
		virtual bool
		First (
		 void *	appBuf)
		{
			return Query::First (appBuf);
		}

		virtual bool
		Last (
		 void *	appBuf)
		{
			return Query::Last (appBuf);
		}

		virtual bool
		Next (
		 void *	appBuf)
		{
			return Query::Next (appBuf);
		}

		virtual bool
		Curr (
		 void *	appBuf)
		{
			return Query::Curr (appBuf);
		}

		virtual bool
		Prev (
		 void *	appBuf)
		{
			return Query::Prev (appBuf);
		}

		virtual bool
		Row (
		 rowid_t	rowid,
		 void *		appBuf)
		{
			return Query::Row (rowid, appBuf);
		}

		virtual bool
		Update (
		 const void *	appBuf)
		{
			return Query::Update (appBuf);
		}

	public:
		TQuery (
		 TTable <RecordType> &	typedTable,
		 enum QueryLock			lock = LkRead) :
			Query (typedTable, lock)
		{
		}

		/*
		 *	Typed masks over access functions
		 */
		bool
		First (
		 RecordType &	appBuf)
		{
			return First (&appBuf);
		}

		bool
		Last (
		 RecordType &	appBuf)
		{
			return Last (&appBuf);
		}

		bool
		Next (
		 RecordType &	appBuf)
		{
			return Next (&appBuf);
		}

		bool
		Curr (
		 RecordType &	appBuf)
		{
			return Curr (&appBuf);
		}

		bool
		Prev (
		 RecordType	&	appBuf)
		{
			return Prev (&appBuf);
		}

		bool
		Row (
		 rowid_t		rowid,
		 RecordType &	appBuf)
		{
			return Row (rowid, &appBuf);
		}

		bool
		Update (
		 const RecordType &	record)
		{
			return Update (&record);
		}
};

#endif	//	_TQuery_h

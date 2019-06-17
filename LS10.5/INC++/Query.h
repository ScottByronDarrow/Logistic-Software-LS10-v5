#ifndef	_Query_h
#define	_Query_h
/*	$Id: Query.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	{libdbif:CISAM}
 *
 *******************************************************************************
 *	$Log: Query.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
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
#include	<dbif-err.h>
#include	<CArray.h>

enum QueryLock
{
	LkRead,
	LkWait,								// wait until lock succeeds
	LkNoWait							// don't block if lock doesn't succeed
};

typedef	long	rowid_t;

class Table;
class Constraint;
class Index;							// internal only

class Date;
class Money;
class Number;
class String;

class Query
{
	private:
		Table &			table;

		int				lock;			// CISAM lock type
		rowid_t			rowid;

		int				cisamfd;
		char *			cisambuf;

		Index *			index;			// set by Query.UseIndex ()
		bool			sortreqd;		// ditto

		Constraint *	where;
		CArray<short>	orderby;

		bool			active,			// any more records to read?
						checkclauses;	// index evalution required?

		enum DBIFError	qError;			// last error

	protected:
		void			SetLock (enum QueryLock);
		bool			ApplyLock (void);

	public:
		Query (const Query &);
		Query (Table &, enum QueryLock = LkRead);
		virtual ~Query ();

		/*
		 *	Member accessors
		 */
		Table &			QTable (void) const;
		rowid_t			RowId (void) const;
		enum QueryLock	Lock (void) const;

		/*
		 *	Access related functions
		 */
		Query &	Where (const Constraint &);
		Query &	OrderBy (const char *, ...);	// NULL terminated

		virtual Query &	Reset (void);		// clears locks, reset query

		/*
		 *	Data access
		 */
		virtual bool	First (void *),		// First record matching constraints
						Last (void *),		// Last record matching constraints
						Next (void *),		// Next record matching constraints
						Curr (void *),		// Re-read current record
						Prev (void *);		// Prev record matching constraints

		virtual bool	Row (rowid_t, void *);

		/*
		 *	Column access
		 */
		void			GetCol (const char * colname, String * buf) const,
						GetCol (const char * colname, short * buf) const,
						GetCol (const char * colname, long * buf) const,
						GetCol (const char * colname, double * buf) const,
						GetCol (const char * colname, float * buf) const,
						GetCol (const char * colname, Number * buf) const,
						GetCol (const char * colname, Date * buf) const,
						GetCol (const char * colname, Money * buf) const;

		/*
		 *	Database manipulation
		 */
		virtual bool	Update (const void *),
						Delete (void);

		/*
		 *	Error queries
		 */
		enum DBIFError	Error (void) const;

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	private:
		bool	ISAMRead (int);		// Read data into internal buffer
		void	ToAppBuf (void *);	// convert to application buffer

		void	ValidateConstraint (Constraint *);

	public:
		bool	PickIndex (String &),			// make a choice
				UseIndex (const char *);		// use given

	// end{Undocumented-Interface}
};

#endif	//_Query_h

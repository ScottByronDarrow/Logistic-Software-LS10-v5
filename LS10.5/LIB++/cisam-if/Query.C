#ident	"$Id: Query.C,v 5.0 2001/06/19 08:17:30 cha Exp $"
/*
 *	Standard Query
 *
 *************************************************************
 *	$Log: Query.C,v $
 *	Revision 5.0  2001/06/19 08:17:30  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1999/05/06 03:25:55  jonc
 *	Fixed: OrderBy clauses not being cleared on subsequent queries.
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.24  1997/12/01 22:04:32  jonc
 *	Fixed: Error code not being set on end of set
 *
 *	Revision 2.23  1997/10/29 21:27:08  jonc
 *	Fixed: typo
 *
 *	Revision 2.22  1997/09/24 03:15:25  jonc
 *	Typesafed GetCol()
 *
 *	Revision 2.21  1997/07/10 02:56:51  jonc
 *	Added several const accesors and tightened argument usage
 *
 *	Revision 2.20  1997/06/16 23:14:14  jonc
 *	Added a kludge for a bug in the old C dbif
 *
 *	Revision 2.19  1997/04/20 23:39:09  jonc
 *	Added timestamp column support
 *
 *	Revision 2.18  1997/04/13 22:14:20  jonc
 *	Fixed: Some wildcard searches not returning full-result set
 *
 *	Revision 2.17  1997/04/10 23:24:41  jonc
 *	Fixed: Weird results if full wildcard search is used on indexed column
 *
 *	Revision 2.16  1996/09/16 01:12:32  jonc
 *	Altered to use CArray
 *
 *	Revision 2.15  1996/08/21 04:50:02  jonc
 *	Fixed: overactive locking. Now only locks records which fufills constraints
 *
 *	Revision 2.14  1996/08/20 21:13:51  jonc
 *	Added stubs to prevent copy constructor
 *	Release locks on revaluation
 *
 *	Revision 2.13  1996/07/30 00:57:43  jonc
 *	Added #ident directive
 *
 *	Revision 2.12  1996/07/25 23:43:17  jonc
 *	Added support for Matches start and end query. Also fixed termination
 *	of query.
 *
 *	Revision 2.11  1996/07/04 00:43:49  jonc
 *	Added support for sequential search
 *
 *	Revision 2.10  1996/06/14 00:02:22  jonc
 *	Added support for optimised ISAM Opens
 */
#ifndef	NDEBUG
#include	<stdio.h>
#endif

#include	<stdarg.h>
#include	<stddef.h>
#include	<string.h>

#include	<osdeps.h>
#include	<liberr.h>
#include	<minor.h>

#include	<Constraint.h>
#include	<Database.h>
#include	<Table.h>
#include	<Query.h>

#include	"Index.h"

#include	<String.h>			// Containers
#include	<Date.h>
#include	<Money.h>

#include	<isam.h>
#include	<decimal.h>
#include	"cisamdefs.h"

/*
 *	Protected interface
 */
void
Query::SetLock (
 enum QueryLock	qlock)
{
	/*
	 *	Translate QueryLock to CISAM lock
	 */
	switch (qlock)
	{
	case LkRead:	lock = 0;		break;
	case LkWait:	lock = ISLCKW;	break;
	case LkNoWait:	lock = ISLOCK;	break;
	default:
		(*lib_error_handler) ("Query::SetLock", "Unknown QueryLock %d", qlock);
	}
}

bool
Query::ApplyLock ()
{ 
	/*
	 *	If a lock is required, we re-read the record (which fulfils the
	 *	given conditions) with the applied lock
	 */
	if (!lock)
		return (true);

	if (isread (cisamfd, cisambuf, ISCURR + lock))
	{
		switch (iserrno)
		{
		case ELOCKED:	qError = RecLocked;		break;

		default:
			(*lib_error_handler) ("Query::ISAMRead",
				"isread returns %d", iserrno);
		}
		return (false);
	}
	return (true);
}

/*
 *	Public Interface
 */
Query::Query (
 const Query &	src) :
	table (src.table)
{
	(*app_error_handler) ("Query::Query", "Copy constructor not supported!");
}

Query::Query (
 Table &		tb,
 enum QueryLock	qlock) :
	table (tb),
	lock (0),
	rowid (0),
	cisamfd (-1),
	cisambuf (NULL),
	index (NULL),
	sortreqd (false),
	where (NULL),
	active (false),
	checkclauses (true),
	qError (NoError)
{

	SetLock (qlock);			//	Set CISAM lock type

	/*
	 *	Allocate buffer for CISAM
	 */
	cisambuf = new char [table.RowSize ()];
}

Query::~Query ()
{
	if (cisamfd >= 0)
	{
		::isrelease (cisamfd);
		Database::ISAMClose (cisamfd, lock ? true : false);
	}

	delete [] cisambuf;
	delete index;
	delete where;
}

Table &
Query::QTable () const
{
	return (table);
}

rowid_t
Query::RowId () const
{
	return (rowid);
}

enum QueryLock
Query::Lock () const
{
	switch (lock)
	{
	case 0:			return LkRead;
	case ISLCKW:	return LkWait;
	case ISLOCK:	return LkNoWait;
	}

	(*lib_error_handler) ("Query::Lock", "Unknown Lock %d", lock);

	return LkRead;		// unreached
}

Query &
Query::Where (
 const Constraint &	c)
{
	if (where)
		delete where;				// replace if required

	if (c.C ())
		where = new Condition ((const Condition &) c);
	else
		where = new CGroup ((const CGroup &) c);

	ValidateConstraint (where);		// true or die

	checkclauses = true;			// force a index re-evaluation
	orderby.Clear ();				// remove possible order restrictions

	return (*this);
}

Query &
Query::OrderBy (
 const char *	col1,
 ...)
{
	/*
	 *	Ordering
	 *		validate the column names by converting them
	 *		to their colno within the table.
	 */
	orderby.Clear ();

	if (col1)
	{
		ColumnInfo		info;

		va_list			args;
		const char *	colN;

		if (!table.ColInfo (col1, info) && !table.UColInfo (col1, info))
		{
			(*lib_error_handler) (
				"Query::OrderBy",
				"Column \"%s\" not in table \"%s\"",
				col1,
				table.Name ());
		}

		orderby.Add (info.ColNo ());

		va_start (args, col1);
		while (colN = va_arg (args, const char *))
		{
			if (!table.ColInfo (colN, info) && !table.UColInfo (colN, info))
			{
				(*lib_error_handler) (
					"Query::OrderBy",
					"Column \"%s\" not in table \"%s\"",
					colN,
					table.Name ());
			}
			orderby.Add (info.ColNo ());
		}
		va_end (args);
	}

	checkclauses = true;
	return (*this);
}

Query &
Query::Reset ()
{
	/*
	 *	Unlock everything,
	 *	Make the query start from the very beginning again
	 */
	if (cisamfd >= 0)
		::isrelease (cisamfd);

	checkclauses = true;		// force reevaluation from the start
	qError = NoError;			// clear any error flags

	return *this;
}

bool
Query::First (
 void *	rawbuf)
{
	/*
	 *	Initial stuff
	 */
	if (cisamfd < 0)
		cisamfd = Database::ISAMOpen (table.FName (), lock ? true : false);

	if (checkclauses)
	{
		/*
		 *	Release all the locks on the table and
		 *	reevaluate the query to determine best index to use
		 */
		::isrelease (cisamfd);
		UseIndex (NULL);
	}

	if (index)
	{
		int	keylen = 0,
			mode = ISFIRST;

		if (where)
		{
			/*
			 *	Need to build up the Query by
			 *	looking at the Constraints
			 *
			 *	For the First record, we only need to handle:
			 *		- AND clauses
			 *		- Eq, Gt, Ge conditions only
			 *
			 */
			bool	looking = true;

			memset (cisambuf, 0, table.RowSize ());		// flush clean

			for (int i = 0;
				looking && i < MAX_IDX_PARTS && index -> Part (i);
				i++)
			{
				const Condition *
					cond = where -> FindWithColNo (index -> Part (i));

				if (!cond)
					break;

				/*
				 *	isstart mode state changes
				 *
				 *	The whole thing is brain-draining. What should
				 *	be remembered is that isstart picks a "starting-position"
				 *	in the index - what you do from then on is up to you.
				 *
				 *	Keeping this in mind, some combinations are totally
				 *	useless (WRONG!).
				 *	eg:
				 *		if your corrent mode is greater, and you get an
				 *		Eq, you break out altogether, otherwise you'll
				 *		never see some required records.
				 *
				 *		Condition = [Gt 100, Eq 90]
				 *		Data	  =	101		90		{Line 1}
				 *					101		91		{Line 2}
				 *					102		90		{Line 3}
				 *
				 *		The start buffer must have:
				 *					101		0		[Mode Gt]
				 *		and *NOT*
				 *					101		90		[Mode Gt]
				 *
				 *		otherwise you'll never see {Line 1}
				 *			
				 */
				bool	put_then_end = false;

				switch (cond -> Cond ())
				{
				case Eq:
					switch (mode)
					{
					case ISFIRST:
					case ISEQUAL:	mode = ISEQUAL;		break;
					case ISGTEQ:	mode = ISGTEQ;		break;
					default:
						assert (false);		// internal error!
					}
					break;

				case Gt:
					switch (mode)
					{
					case ISFIRST:
					case ISEQUAL:
					case ISGREAT:
					case ISGTEQ:	mode = ISGREAT;		break;
					default:
						assert (false);		// internal error!
					}
					put_then_end = true;
					break;

				case Ge:
					switch (mode)
					{
					case ISFIRST:
					case ISEQUAL:
					case ISGTEQ:	mode = ISGTEQ;		break;
					default:
						assert (false);		// internal error!
					}
					break;

				case Mt:
					switch (mode)
					{
					case ISFIRST:
					case ISEQUAL:
					case ISGTEQ:	mode = ISGTEQ;		break;
					default:
						assert (false);		// internal error!
					}
					put_then_end = true;
					break;

				default:
					looking = false;
				}

				if (looking)
				{
					cond -> Put (cisambuf);
					keylen += cond -> Cond () == Mt ?
						cond -> SignificantLength () :
						index -> Desc ().k_part [i].kp_leng;
				}

				if (put_then_end)
					break;
			}
		}

		if (!keylen && mode != ISFIRST)
		{
			/*
			 *	This situation could possible occur if the Query has
			 *	a full-wildcard match on the first column of the index
			 *	used. If so, we have to force the mode to ISFIRST,
			 *	otherwise weird results come back
			 */
			mode = ISFIRST;		// first record of index
		}

		/*
		 *	Attempt setting CISAM key
		 */
		if (isstart (cisamfd,
					 &index -> Desc (),
					 keylen,
					 cisambuf,
					 mode))
		{
			if (iserrno == ENOREC)
			{
				qError = EndOfFile;
				return (false);
			}

			(*lib_error_handler) ("Query::First",
				"isstart failed with %d", iserrno);
		}
	}
	else
	{
		/*
		 *	Hmm, a Query with no index. We'll start from the top
		 *	and to a sequential search
		 */
		struct keydesc	pkey;

		pkey.k_nparts = 0;			// force use of physical order

		if (isstart (cisamfd,
					 &pkey,
					 0,
					 cisambuf,
					 ISFIRST))
		{
			if (iserrno == ENOREC)
			{
				qError = EndOfFile;
				return (false);
			}

			(*lib_error_handler) ("Query::First",
				"isstart failed with %d", iserrno);
		}

#ifndef	NDEBUG
		fprintf (stderr,
			"WARNING: Sequential search on \"%s\"",
			(const char *) table.Name ());
#endif
	}

	if (!ISAMRead (ISCURR))
		return (active = false);

	active = true;

	/*
	 *	If the record we have doesn't fit the Where clause
	 *	we skip thru' until we find something that does
	 */
	if (where && !where -> Fulfilled (cisambuf))
		return (Query::Next (rawbuf));

	if (!ApplyLock ())
	{
		assert (qError == RecLocked);
		return (false);
	}
	ToAppBuf (rawbuf);
	return (true);
}

bool
Query::Last (
 void *)
{
	(*lib_error_handler) ("Query::Last", "Unimplemented");
	return (false);
}

bool
Query::Next (
 void *	rawbuf)
{
	if (cisamfd < 0 || checkclauses)
		return (Query::First (rawbuf));

	if (!active)
		return (false);

	/*
	 *	Skip to next record that satisfies the Constraints
	 */
	while (ISAMRead (ISNEXT))
	{
		if (!where || where -> Fulfilled (cisambuf))
		{
			if (!ApplyLock ())
			{
				assert (qError == RecLocked);
				return (false);
			}
			ToAppBuf (rawbuf);
			return (true);
		}

		/*
		 *	If the Constraints have NOT been satisfied,
		 *	check to see whether we terminate the Query
		 *	right here and now
		 *
		 *	The way to check for termination is to look work
		 *	from the lower index column-parts and if any of the
		 *	Constraints fail, we can stop.
		 */
		 if (!index)				// no index, we need to slog on
			continue;

		for (int i = 0; i < MAX_IDX_PARTS && index -> Part (i); i++)
		{
			const Condition *
				cond = where -> FindWithColNo (index -> Part (i));

			if (!cond)
				break;

			if (cond -> Cond () == Mt)
			{
				/*
				 *	Once we encounter a wildcard Constraint, we
				 *	can't keep on peeking further on index-columns
				 *	'cos the key is only unique/significant until the
				 *	first wildcard char
				 */
				if (cond -> SignificantLength ())
				{
					/*
					 *	If the leading non-wildcard chars don't match
					 *	what's in the buffer, we can terminate the query
					 */
					if (!(active = cond -> SignificantCheck (cisambuf)))
					{
						qError = EndOfFile;
						return false;
					}
				}
				break;
			}
			else if (!(active = cond -> Fulfilled (cisambuf)))
			{
				qError = EndOfFile;
				return false;
			}
		}
	}
	assert (qError == EndOfFile);
	return (active = false);
}

bool
Query::Curr (
 void *	rawbuf)
{
	assert (cisamfd >= 0);			// application error

	if (!ApplyLock ())				// rereads current with lock
	{
		assert (qError == RecLocked);
		return (false);
	}
	ToAppBuf (rawbuf);
	return (true);
}

bool
Query::Prev (
 void *)
{
	(*lib_error_handler) ("Query::Prev", "Unimplemented");
	return (false);
}

bool
Query::Row (
 rowid_t	row,
 void *		rawbuf)
{
	if (where || orderby.Count ())
		(*app_error_handler) ("Query::RowId", "Where/OrderBy clause found");

	if (cisamfd < 0)
		cisamfd = Database::ISAMOpen (table.FName (), lock ? true : false);

	UseIndex (NULL);			// open CISAM file

	/*
	 *	Set up for search by rowid
	 */
	struct keydesc	pkey;

	pkey.k_nparts = 0;			// force use of physical order
	isrecnum = row;
	if (isstart (cisamfd, &pkey, 0, cisambuf, ISEQUAL))
	{
		if (iserrno == ENOREC)
		{
			qError = EndOfFile;
			active = false;
			return (false);
		}

		(*lib_error_handler) ("Query::RowId",
			"isstart failed with %d", iserrno);
	}

	active = false;				// since there's only one matching row

	if (!ISAMRead (ISCURR) || !ApplyLock ())
		return (false);

	ToAppBuf (rawbuf);
	return (true);
}

void
Query::GetCol (
 const char *	colname,
 String *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(String)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColChar)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.CChar (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 short *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(short)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColShort)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.Short (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 long *			elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(long)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	switch (info.type)
	{
	case ColLong:
	case ColSerial:
		*elembuf = info.Long (cisambuf);
		break;
	default:
		(*app_error_handler) (function, "Bad column type: %s", colname);
	}
}

void
Query::GetCol (
 const char *	colname,
 double *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(double)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColDouble)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.Double (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 float *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(float)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColFloat)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.Float (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 Number *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(Number)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColDecimal)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.Decimal (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 Date *			elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(Date)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColDate)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.IDate (cisambuf);
}

void
Query::GetCol (
 const char *	colname,
 Money *		elembuf) const
{
	/*
	 *	Get current row's column into elembuf
	 */
	const char *	function = "Query::GetCol(Money)";

	if (cisamfd < 0)
		(*app_error_handler) (function, "Inactive query");

	ColumnInfo	info;

	if (!table.ColInfo (colname, info) &&
		!table.UColInfo (colname, info))
	{
		(*app_error_handler) (function, "Bad column name: %s", colname);
	}

	if (info.type != ColMoney)
		(*app_error_handler) (function, "Bad column type: %s", colname);

	*elembuf = info.Decimal (cisambuf);
}

bool
Query::Update (
 const void *	appbuf)
{
	/*
	 *	Update current record with contents of application buffer
	 */
	assert (cisamfd >= 0);					// application error

	/*
	 *	Application program checks
	 */
	if (!lock)
		(*app_error_handler) ("Query::Update",
			"Attempt to update without lock");

	/*
	 *	Load the internal buffer with the application buffer's data
	 */
	table.AppToCISAM (appbuf, cisambuf);
	table.UpdateTimestamp (cisambuf);

	/*
	 *	Update the current record
	 */
	if (isrewcurr (cisamfd, cisambuf) < 0)
	{
		if (iserrno == EDUPL)
		{
			qError = DupIdx;
			return (false);
		}
		(*lib_error_handler) ("Query::Update",
			"Update \"%s\" failed (%d)",
			table.Name (),
			iserrno);
	}

	return (true);
}

bool
Query::Delete ()
{
	/*
	 *	Delete current record in query
	 */
	if (isdelcurr (cisamfd) < 0)
	{
		if (iserrno == ENOCURR)
		{
			qError = NoCurr;
			return (false);
		}
		(*lib_error_handler) ("Query::Delete",
			"Delete \"%s\" failed (%d)",
			table.Name (),
			iserrno);
	}

	return (true);
}
 
enum DBIFError
Query::Error () const
{
	return (qError);
}

/*
 *	Private interface
 */
bool
Query::ISAMRead (
 int	mode)
{
	if (cisamfd < 0)
		(*lib_error_handler) ("Query::ISAMRead", "ISAMRead cisamfd < 0!");

	/*
	 *	read into internal buffer, with *NO* locks (this is 'cos
	 *	we don't want to lock records that don't fulfil given
	 *	constraints)
	 */
	if (isread (cisamfd, cisambuf, mode))
	{
		switch (iserrno)
		{
		case EENDFILE:	qError = EndOfFile;		break;

		default:
			(*lib_error_handler) ("Query::ISAMRead",
				"isread returns %d", iserrno);
		}
		return (false);
	}

	rowid = isrecnum;
	qError = NoError;
	return (true);
}

void
Query::ToAppBuf (
 void *	rawbuf)
{
	if (!rawbuf)
	{
		/*
		 *	No application buffer supplied.
		 *	This is possible for Queries activated to change
		 *	the current record, but discarding the results
		 */
		return;
	}

	char *	appbuf = (char *) rawbuf;

	/*
	 *	Go thru' each column and convert the CISAM buffer data
	 *	to something the application can understand
	 */
	for (unsigned i = 0; i < table.ColCount (); i++)
	{
		ColumnInfo	info;

		if (!table.ColInfo (i, info))
			(*lib_error_handler) ("Query::Update",
				"Info query on \"%s\" failed",
				table.ColName (i));

		switch (info.type)
		{
		case ColChar:
			info.aString (appbuf) = info.CChar (cisambuf);
			break;
		case ColShort:
			info.aShort (appbuf) = (info.Short (cisambuf) == INTNULL) ?
									0 : info.Short (cisambuf);
			break;
		case ColSerial:
		case ColLong:
			info.aLong (appbuf) = (info.Long (cisambuf) == LONGNULL) ?
									0 : info.Long (cisambuf);
			break;
		case ColDouble:
			info.aDouble (appbuf) = info.Double (cisambuf);
			break;
		case ColFloat:
			info.aFloat (appbuf) = info.Float (cisambuf);
			break;
		case ColDecimal:
			info.aNumber (appbuf) = info.Decimal (cisambuf);
			break;
		case ColDate:
			if (!info.Long (cisambuf) ||			// kludge for old C-dbif bug
				info.Long (cisambuf) == LONGNULL)
			{
				info.aDate (appbuf).SetNull ();
			}
			else
				info.aDate (appbuf).InformixDate (info.Long (cisambuf));
			break;
		case ColMoney:
			info.aMoney (appbuf) = info.Decimal (cisambuf);
			break;
		case ColTime:
			break;
		default:
			(*lib_error_handler) ("Query::ToAppBuf",
				"Bad type on \"%s\" : %d",
				table.ColName (i),
				info.type);
		}
	}
}

void
Query::ValidateConstraint (
 Constraint *	c)
{
	if (!c)
		return;

	/*
	 *	Update the info member in the Condition classes
	 *	by validating the columns
	 *
	 */
	if (c -> C ())
	{
		/*
		 *	Simple condition
		 */
		Condition *	cond = (Condition *) c;

		if (table.ColInfo (cond -> Col (), cond -> colinfo) ||
			table.UColInfo (cond -> Col (), cond -> colinfo))
		{
			return;
		}
		(*lib_error_handler) (
			"Query::ValidateConstraint",
			"Column \"%s\" not in table \"%s\"",
			cond -> Col (),
			table.Name ());
	}

	/*
	 *	Constraint Group
	 */
	int			i;
	CGroup *	grp = (CGroup *) c;

	for (i = 0; i < grp -> andC; i++)
		ValidateConstraint (grp -> andGrp [i]);
	for (i = 0; i < grp -> orC; i++)
		ValidateConstraint (grp -> orGrp [i]);
}

bool
Query::PickIndex (
 String &	idxname)
{
	/*
	 *	Pick best index to use.
	 *
	 *	Make this a public accesible so that application programmers
	 *	can find out just what (stupid?) choice the library has chosen
	 *
	 *	Consider, in order:
	 *		- OrderBy
	 *		- OrderBy with Constraints
	 *		- Constraints
	 *
	 *	The first successful result is used.
	 *
	 *	Returns true if an index is found, false otherwise.
	 */
	if (orderby.Count ())
	{
		int		i;
		short	idxcols [MAX_IDX_PARTS];

		// Make up a column block
		for (i = 0; i < orderby.Count () && i < MAX_IDX_PARTS; i++)
			idxcols [i] = orderby.Elem (i);
		while (i < MAX_IDX_PARTS)
			idxcols [i++] = 0;

		if (table.PickIndex (idxcols, idxname))
			return (true);		// index found with matching orderby

		if (where &&
			table.PickIndex (idxcols,
				table.ColCount () + table.UColCount (), *where, idxname))
		{
			return (true);		// constraints made the difference!
		}
	}

	/*
	 *	Look at constraints to determine possible index
	 */
	if (where &&
		table.PickIndex (table.ColCount () + table.UColCount (),
						 *where, idxname))
	{
		return (true);
	}

	return (false);
}

bool
Query::UseIndex (
 const char *	idxname)
{
	/*
	 *	Use the index given. If it is an valid index name, we'll
	 *	use it, otherwise we'll pick one to use.
	 */
	bool	retval = true;

	/*
	 *	Remove old index
	 */
	delete index;
	index = NULL;

	/*
	 *	Try out supplied index
	 */
	if (table.GetIndex (idxname))
		index = new Index (*table.GetIndex (idxname));
	else
	{
		/*
		 *	We'll pick and index
		 */
		String	name;

		if (PickIndex (name))
		{
			const Index *	ix =  table.GetIndex (name);

			if (!ix)
				(*lib_error_handler) ("Query::UseIndex",
					"System Picked index has no info?");

			index = new Index (*ix);
		}

		retval = false;			// we were supplied a lousy index
	}

	/*
	 *	Need to check whether external sort is required
	 */
	if (orderby.Count ())
	{
		if (!index || orderby.Count () > MAX_IDX_PARTS)
			sortreqd = true;
		else
		{
			int	matches;

			for (matches = 0;
				matches < orderby.Count () && index -> Part (matches);
				matches++)
			{
				if (orderby [matches] != index -> Part (matches))
					break;
			}
			if (matches < orderby.Count ())
				sortreqd = true;
		}
	}

	if (sortreqd)
		(*lib_error_handler) ("Query::UseIndex", "Ext-sort unimplemented");

	checkclauses = false;
	return (retval);
}

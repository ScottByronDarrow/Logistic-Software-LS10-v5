#ident	"$Id: Table.C,v 5.0 2001/06/19 08:17:31 cha Exp $"
/*
 *	Table:
 *		- INFORMIX tables from system tables
 *
 *******************************************************************************
 *	$Log: Table.C,v $
 *	Revision 5.0  2001/06/19 08:17:31  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.13  1997/07/10 02:56:51  jonc
 *	Added several const accesors and tightened argument usage
 *
 *	Revision 2.12  1997/04/20 23:39:11  jonc
 *	Added timestamp column support
 *
 *	Revision 2.11  1996/08/18 04:08:59  jonc
 *	Stop the use of a copy constructor
 *
 *	Revision 2.10  1996/07/30 00:57:44  jonc
 *	Added #ident directive
 *
 *	Revision 2.9  1996/06/17 03:46:59  jonc
 *	Fixed:   Errorneous PickIndex on non-Eq clauses
 *	Changed: Constraint-based PickIndex decisions
 *
 *	Revision 2.8  1996/05/30 04:58:04  jonc
 *	Update to Serial fields disallowed.
 *
 *	Revision 2.7  1996/05/04 08:30:27  jonc
 *	Added support for Serial fields
 *
 *	Revision 2.6  1996/04/15 21:28:02  jonc
 *	Added accessor to Table's Database
 *
 *	Revision 2.5  1996/04/09 03:46:38  jonc
 *	Column() now const
 *
 *	Revision 2.4  1996/03/22 03:52:53  jonc
 *	Added support for Table::AppBufSize ()
 *
 *	Revision 2.3  1996/03/21 21:41:44  jonc
 *	Reset error flag on successful add
 *
 *	Revision 2.2  1996/03/07 21:33:53  jonc
 *	Additions now initialise unused fields to something sensible
 *
 *	Revision 2.1  1996/03/04 03:02:42  jonc
 *	Added:
 *		- Deletion & Addition
 *		- Error() accesssors
 *
 *	Revision 2.0  1996/02/13 03:34:53  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:11  jonc
 *	Initial C++ CISAM-Interface entry
 *
 *	Revision 1.1  1996/01/07 21:12:29  jonc
 *	Initial revision
 *
 */
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<osdeps.h>
#include	<liberr.h>
#include	<Database.h>
#include	<Table.h>
#include	<Constraint.h>

#include	<Date.h>			// containers
#include	<String.h>
#include	<Money.h>

#include	<decimal.h>
#include	<isam.h>

#include	"Index.h"
#include	"cisamdefs.h"

/*
 *	Local vars
 */
static const char *	TimeStampColName	= "timestamp";

/*
 *	Local functions
 */
static void	GrokAndClauses (Array<bool> &, enum CondCmp, const Constraint &);

/*
 *	External interface
 */
Table::Table (
 const Table &	src) :
	dbase (src.dbase)
{
	(*app_error_handler) ("Table::Table", "Copy constructor not supported!");
}

Table::Table (
 Database &		db,
 const char *	name,
 const char *	flds []) :
	dbase (db),
	tbname (NULL),
	fname (new char [MAX_PATHNAME_LEN + 1]),
	tabid (0),
	rowsize (0),

	fldcount (0),
	fields (flds),
	fldinfo (NULL),

	nfldcount (0),
	nfields (NULL),
	nfldinfo (NULL),

	indexcount (0),
	indexes (NULL),

	tError (NoError)
{
	strcpy (tbname = new char [strlen (name) + 1], name);

	/*
	 *	The Field Information (fldinfo, nfldinfo)
	 *	has been kept seperate from the flds array
	 *	so that it can't be accessed in application code
	 */
	for (fldcount = 0; fields [fldcount]; fldcount++);

	db.GetTabInfo (tbname,
		fldcount,
		fields,
		fname,					// returned value
		tabid,					// returned value
		rowsize,				// returned value
		appbfsize,				// returned value
		fldinfo,				// returned value
		nfldcount,				// returned value
		nfields,				// returned value
		nfldinfo);				// returned value

	/*
	 *	Build up a list of the indexes in the table
	 *	and match the keydesc structures with the system-catalog names
	 */
	int	i;

	/*
	 *	Obtain system-catalog info
	 */
	char **		idxnames = NULL;
	short **	idxcols = NULL;

	db.GetIdxInfo (tabid, indexcount, idxnames, idxcols);

	/*
	 *	Allocate space for Index associations
	 */
	indexes = new Index [indexcount];

	/*
	 *	Read in the keydesc structures in the file
	 */
	struct dictinfo	dinfo;
	struct keydesc *keys = NULL;
	bool *			keyd = NULL;

	int	isamfd = db.ISAMOpen (fname);

	if (isindexinfo (isamfd, (struct keydesc *) &dinfo, 0))
		(*lib_error_handler) ("Table::Table",
			"isindexinfo fails with %d", iserrno);

#if 0
	/*
	 *	For some odd reason, this isn't quite right?
	 */
	if (dinfo.di_nkeys & 0x7fff != indexcount)
		(*lib_error_handler) ("Table::Table",
			"Key count (%d) doesn't match system catalog (%d)",
			dinfo.di_nkeys & 0x7fff, indexcount);
#endif
	int keyc = dinfo.di_nkeys & 0x7fff;

	if (keyc)
	{
		keys = new struct keydesc [keyc];
		keyd = new bool [keyc];

		for (i = 0; i < keyc; i++)
		{
			keyd [i] = false;
			if (isindexinfo (isamfd, keys + i, i + 1))	// info starts from 1
				(*lib_error_handler) ("Table::Table",
					"isindexinfo (%d) returns %d",
					i, iserrno);
		}
	}

	db.ISAMClose (isamfd);

	/*
	 *	Match keydesc structures with the names.
	 *		- we do this by looking at the column start positions
	 */

	for (i = 0; i < indexcount; i++)		// work thru' system catalog info
	{
		int		j;
		int		kparts;
		short	starts [MAX_IDX_PARTS];		// start positions of key-parts

		/*
		 *	Count the parts as seen in the system catalog
		 *	and obtain the start positions
		 */
		for (kparts = 0; kparts < MAX_IDX_PARTS; kparts++)
		{
			ColumnInfo	colinfo;

			if (!idxcols [i] [kparts])
				break;

			if (!ColIdInfo (idxcols [i] [kparts], colinfo))
				(*lib_error_handler) ("Table::Table",
					"Index Column %d not found in field list?",
					idxcols [i] [kparts]);

			starts [kparts] = colinfo.COff ();
		}

		/*
		 *	Wade thru' the keydesc structures
		 */
		for (j = 0; j < keyc; j++)
		{
			int		k;

			if (keyd [j] || kparts != keys [j].k_nparts)
				continue;

			for (k = 0; k < kparts; k++)
				if (starts [k] != keys [j].k_part [k].kp_start)
					break;
			if (k == kparts)		// all parts match!
				break;
		}

		if (j < keyc)
		{
			/*
			 *	Copy information to internal array
			 */
			strcpy (indexes [i].name, idxnames [i]);
			indexes [i].desc = keys [j];
			for (int c = 0; c < MAX_IDX_PARTS; c++)
				indexes [i].parts [c] = idxcols [i] [c];

			idxnames [i] [0] = '\0';		// mark as read
			keyd [j] = true;				// mark as read
		}
		else
		{
			(*lib_error_handler) ("Table::Table",
				"Index %s not found in field list?",
				idxnames [i]);
		}
	}

	/*
	 *	Cleanup + sanity checks
	 */
#if 0
	for (i = 0; i < keyc; i++)
		if (!keyd [i])
			fprintf (stderr, "key %d not matched\n", i);
#endif

	for (i = 0; i < indexcount; i++)
	{
		if (idxnames [i] [0])
			(*lib_error_handler) ("Table::Table",
				"Unmatched index \"%s\"", idxnames [i]);
			
		delete [] idxnames [i];
		delete [] idxcols [i];
	}
	delete [] idxnames;
	delete [] idxcols;
	delete [] keys;
	delete [] keyd;
}

Table::~Table ()
{
	delete [] indexes;
	delete [] fldinfo;

	for (unsigned i = 0; i < nfldcount; i++)
		delete [] nfields [i];
	delete [] nfields;
	delete [] nfldinfo;

	delete [] tbname;
	delete [] fname;
}

const char *
Table::Name () const
{
	return (tbname);
}

long
Table::TabId () const
{
	return (tabid);
}

unsigned
Table::RowSize () const
{
	return (rowsize);
}

unsigned
Table::AppBufSize () const
{
	return (appbfsize);
}

unsigned
Table::ColCount () const
{
	return (fldcount);
}

Database &
Table::DBase () const
{
	return (dbase);
}

const char *
Table::ColName (
 unsigned	i) const
{
	return (i < fldcount ? fields [i] : NULL);
}

bool
Table::ColInfo (
 unsigned		i,
 ColumnInfo &	info) const
{
	if (i < fldcount)
	{
		info = fldinfo [i];
		return (true);
	}
	return (false);
}

bool
Table::ColInfo (
 const char *	name,
 ColumnInfo &	info) const
{
	for (int i = 0; fields [i]; i++)
		if (!strcmp (name, fields [i]))
			return (ColInfo (i, info));
	return (false);
}

void *
Table::Column (
 const char *	colname,
 void *			buffer) const
{
	/*
	 *	Returns the buffer with the appropriate offset to access
	 *	supplied column
	 */
	for (unsigned i = 0; i < fldcount; i++)
		if (!strcmp (colname, fields [i]))
			return (fldinfo [i].AOff (buffer));
	return (NULL);
}

unsigned
Table::UColCount () const
{
	return (nfldcount);
}

const char *
Table::UColName (
 unsigned	i) const
{
	return (i < nfldcount ? nfields [i] : NULL);
}

bool
Table::UColInfo (
 unsigned		i,
 ColumnInfo &	info) const
{
	if (i < nfldcount)
	{
		info = nfldinfo [i];
		return (true);
	}
	return (false);
}

bool
Table::UColInfo (
 const char *	name,
 ColumnInfo &	info) const
{
	for (unsigned i = 0; i < nfldcount; i++)
		if (!strcmp (name, nfields [i]))
			return (UColInfo (i, info));
	return (false);
}

/*
 *	Database manipulation associated with the Table class
 */
bool
Table::Add (
 const void *	appbuffer)
{
	int		isamfd = dbase.ISAMOpen (fname, true);	// auto-crash on bad fd
	char *	isambuf = new char [rowsize];
	bool	retval = true;

	ZeroUnused (isambuf);
	AppToCISAM (appbuffer, isambuf);

	/*
	 *	Do increment serial fields?
	 *		- check used and unsused fields
	 */
	long		serialno = 0;
	unsigned	i;

	for (i = 0; i < fldcount; i++)
	{
		if (fldinfo [i].type == ColSerial)
		{
			isuniqueid (isamfd, &serialno);
			stlong (serialno, fldinfo [i].COff (isambuf));
			break;
		}
	}
	if (!serialno)
		for (i = 0; i < nfldcount; i++)
		{
			if (nfldinfo [i].type == ColSerial)
			{
				isuniqueid (isamfd, &serialno);
				stlong (serialno, nfldinfo [i].COff (isambuf));
				break;
			}
		}

	/*
	 *	Check for TimeStamp column
	 */
	UpdateTimestamp (isambuf);

	/*
	 *	Attempt the record Write
	 */
	if (iswrite (isamfd, isambuf) < 0)
	{
		if (iserrno == EDUPL)
		{
			tError = DupIdx;
			retval = false;
		}
		else
			(*lib_error_handler) ("Table::Table",
				"iswrite fails with %d", iserrno);
	}
	else
		tError = NoError;

	dbase.ISAMClose (isamfd, true);

	delete [] isambuf;
	return (retval);
}

void
Table::UpdateTimestamp (
 void *	isambuf)
{
	/*
	 *	Look for a timestamp column, and update it with the current
	 *	time(NULL) value
	 */
	bool		found_col = false;
	unsigned	i;

	for (i = 0; !found_col && i < fldcount; i++)
	{
		if (fldinfo [i].type == ColLong &&
			!strcmp (fields [i], TimeStampColName))
		{
			stlong (time (NULL), fldinfo [i].COff (isambuf));
			found_col = true;
		}
	}
	for (i = 0; !found_col && i < nfldcount; i++)
	{
		if (nfldinfo [i].type == ColLong &&
			!strcmp (nfields [i], TimeStampColName))
		{
			stlong (time (NULL), nfldinfo [i].COff (isambuf));
			found_col = true;
		}
	}
}

enum DBIFError
Table::Error () const
{
	return (tError);
}

/*
 *	Undocumented interfaces
 */
void
Table::ZeroUnused (
 void *	cisambuf)
{
	const char *	Null = "";

	/*
	 *	Zero the unused fields in the CISAM buffer
	 */
	for (unsigned i = 0; i < nfldcount; i++)
	{
		switch (nfldinfo [i].type)
		{
		case ColChar:
			stchar (Null, nfldinfo [i].COff (cisambuf), nfldinfo [i].length);
			break;
		case ColShort:
			stint (0, (char *) nfldinfo [i].COff (cisambuf));
			break;
		case ColLong:
		case ColSerial:
			stlong (0, nfldinfo [i].COff (cisambuf));
			break;
		case ColDouble:
			stdbl (0.0, nfldinfo [i].COff (cisambuf));
			break;
		case ColFloat:
			stfloat (0.0, nfldinfo [i].COff (cisambuf));
			break;
		case ColDecimal:
		case ColMoney:
		{
			dec_t	d;
			
			deccvint (0, &d);
			stdecimal (&d,
				nfldinfo [i].COff (cisambuf),
				DECLEN (nfldinfo [i].length, nfldinfo [i].precision));
			break;
		}
		case ColDate:
			stlong (LONGNULL, nfldinfo [i].COff (cisambuf));
			break;
		case ColTime:
			break;
		default:
			(*lib_error_handler) ("Table::ZeroUnused",
				"Bad type on \"%s\" : %d",
				nfields [i],
				nfldinfo [i].type);
		}
	}
}

const char *
Table::FName () const
{
	return (fname);
}

bool
Table::ColIdInfo (
 short			colno,
 ColumnInfo &	info) const
{
	/*
	 *	Obtain column information based on Column Id
	 */
	unsigned	i;

	/*
	 *	Look thru' declared fields first
	 */
	for (i = 0; i < fldcount; i++)
		if (fldinfo [i].ColNo () == colno)
		{
			info = fldinfo [i];
			return (true);
		}

	for (i = 0; i < nfldcount;i++)
		if (nfldinfo [i].ColNo () == colno)
		{
			info = nfldinfo [i];
			return (true);
		}

	return (false);
}

const Index *
Table::GetIndex (
 const char *	idxname) const
{
	if (!idxname)
		return (NULL);

	for (int i = 0; i < indexcount; i++)
		if (!strcmp (idxname, indexes [i].name))
			return (indexes + i);
	return (NULL);
}

bool
Table::PickIndex (
 short		parts [],
 String &	idxname)
{
	/*
	 *	1. Gives opinion on possible index match
	 *
	 *		We're going to compare the index column-nos given
	 *		against those in sysindexes. If the non-zero items
	 *		match against from the 1st item, we've got a good
	 *		match.
	 *
	 *		Return the one with the highest number number of matches
	 */
	int	match_high = 0;

	for (int i = 0; i < indexcount; i++)
	{
		int		matches = 0;

		for (int j = 0;
			j < MAX_IDX_PARTS && parts [j] && indexes [i].parts [j];
			j++)
		{
			if (parts [j] == indexes [i].parts [j])
				matches++;
		}

		/*
		 *	Check whether any match found
		 */
		if (matches && matches > match_high)
		{
			/*
			 *	Extract the indexname for possible use
			 */
			idxname = indexes [i].name;
			match_high = matches;
		}
	}
	return (match_high ? true : false);
}

bool
Table::PickIndex (
 short			parts [],
 unsigned		colcount,
 Constraint &	conds,
 String &		idxname)
{
	/*
	 *	2. Gives opinion on possible index match
	 *
	 *		This time around, we're looking at patterns with
	 *		in the higher regions of the index. If we have
	 *		a similar index *AND* the Constraints allow it,
	 *		we've got a good match.
	 *
	 *		Return the one with the highest number number of matches
	 */
	int			match_high = 0;
	Array<bool>	andcols;

	/*
	 *	Look at the Constraint's AND clauses for Eq items
	 */
	for (unsigned c = 1; c <= colcount; c++)		// +1 'cos colid start at 1
		andcols [c] = false;						// initialise
	GrokAndClauses (andcols, Eq, conds);			// filter for Eq matches

	for (int ix = 0; ix < indexcount; ix++)
	{
		int	i,
			matches = 0;

		for (i = 1; i < MAX_IDX_PARTS; i++)	// don't start from 0
		{
			int	j, k;

			for (j = 0;
				j < MAX_IDX_PARTS && parts [j] && indexes [ix].parts [i + j];
				j++)
			{
				if (parts [j] == indexes [ix].parts [i + j])
					matches++;
			}

			if (!matches)
				continue;

			/*
			 *	Look at the Constraint's AND clauses for Eq items
			 *	and see whether the colnos match the lower region
			 *	of the index.
			 */
			for (k = 0; k < i; k++)
				// is idx-col in And Clauses?
				if (!andcols [indexes [ix].parts [k]])
					break;
			if (k != i)						// lower idx-cols didn't all match
				continue;

			/*
			 *	Better than the best?
			 */
			if (matches > match_high)
			{
				idxname = indexes [ix].name;
				match_high = matches;
			}
		}
	}
	return (match_high ? true : false);
}

bool
Table::PickIndex (
 unsigned		colcount,
 Constraint &	conds,
 String &		idxname)
{
	/*
	 *	3. Gives opinion on possible index match
	 *
	 *		For the last time, we're just looking at the
	 *		AND clauses in the Constraint
	 *
	 *		Return the one with the highest number number of matches
	 */
	int			matches = 0,
				match_high = 0;
	int			i, ix;
	unsigned	c;
	Array<bool>	eq_cols;

	/*
	 *	Look at the Constraint's AND clauses for Eq items
	 */
	for (c = 1; c <= colcount; c++)					// +1 'cos colid start at 1
		eq_cols [c] = false;						// initialise
	GrokAndClauses (eq_cols, Eq, conds);

	/*
	 *	We scan thru' all the indexes to find one that gives
	 *	the most exact matches for the Eq constraint
	 */
	for (ix = 0; ix < indexcount; ix++)
	{
		matches = 0;

		/*
		 *	Load key-parts
		 */
		for (i = 0;
			indexes [ix].parts [i] &&				// while valid col
				eq_cols [indexes [ix].parts [i]] &&	// is col in AND clause?
				i < MAX_IDX_PARTS;
			i++)
		{
			matches++;
		}

		if (matches > match_high)
		{
			/*
			 *	Extract the indexname for possible use
			 */
			idxname = indexes [ix].name;
			match_high = matches;
		}
	}

	/*
	 *	We attempt to optimise the index matched by looking for
	 *	inexact (Ge, Mt) constraints that match the later parts
	 *	of the index
	 */
	int			inx_high = 0;
	Array<bool>	inx_cols;

	for (c = 1; c <= colcount; c++)					// initialise
		inx_cols [c] = false;
	GrokAndClauses (inx_cols, Ge, conds);
	GrokAndClauses (inx_cols, Mt, conds);

	/*
	 *	This time, we only consider indexes that have the same
	 *	number of matches as the picked index; choosing the one
	 *	gives the most inexact matches.
	 */
	for (ix = 0; ix < indexcount; ix++)
	{
		matches = 0;

		/*
		 *	Load key-parts
		 */
		for (i = 0;
			indexes [ix].parts [i] &&				// while valid col
				eq_cols [indexes [ix].parts [i]] &&	// is col in AND clause?
				i < MAX_IDX_PARTS;
			i++)
		{
			matches++;
		}
		if (matches < match_high)
			continue;								// ignore lower matchcount

		/*
		 *	Count number of inexact matches found
		 */
		int	inx_matches = 0;

		for (i = match_high;
			indexes [ix].parts [i] &&					// while valid col
				inx_cols [indexes [ix].parts [i]] &&	// is col in AND clause?
				i < MAX_IDX_PARTS;
			i++)
		{
			inx_matches++;
		}

		/*
		 *	Extract the indexname for possible use
		 */
		if (inx_matches > inx_high)
		{
			idxname = indexes [ix].name;
			inx_high = inx_matches;
		}
	}
	return (match_high || inx_high);
}

void
Table::AppToCISAM (
 const void *	appbuf,
 void *			cisambuf)
{
	/*
	 *	Load the internal buffer with the application buffer's data
	 */
	for (unsigned i = 0; i < fldcount; i++)
	{
		switch (fldinfo [i].type)
		{
		case ColChar:
			stchar (fldinfo [i].cString (appbuf).chars (),
				fldinfo [i].COff (cisambuf),
				fldinfo [i].length);
			break;
		case ColShort:
			stint (fldinfo [i].cShort (appbuf),
				(char *) fldinfo [i].COff (cisambuf));
			break;
		case ColLong:
			stlong (fldinfo [i].cLong (appbuf), fldinfo [i].COff (cisambuf));
			break;
		case ColDouble:
			stdbl (fldinfo [i].cDouble (appbuf), fldinfo [i].COff (cisambuf));
			break;
		case ColFloat:
			stfloat (fldinfo [i].cFloat (appbuf), fldinfo [i].COff (cisambuf));
			break;
		case ColDecimal:
		{
			dec_t	d;
			String	v;
			
			fldinfo [i].cNumber (appbuf).Get (v);

			deccvasc (v, v.length (), &d);
			stdecimal (&d,
				fldinfo [i].COff (cisambuf),
				DECLEN (fldinfo [i].length, fldinfo [i].precision));
			break;
		}
		case ColSerial:
			/*
			 *	Whatever's in the application buffer's serial
			 *	field is totally ignored
			 */
			break;
		case ColDate:
			if (fldinfo [i].cDate (appbuf).Null ())
				stlong (LONGNULL, fldinfo [i].COff (cisambuf));
			else
				stlong (
					fldinfo [i].cDate (appbuf).InformixDate (),
					fldinfo [i].COff (cisambuf));
			break;
		case ColMoney:
		{
			dec_t	d;
			String	v;
			
			fldinfo [i].cMoney (appbuf).Get (v);

			deccvasc (v, v.length (), &d);
			stdecimal (&d,
				fldinfo [i].COff (cisambuf),
				DECLEN (fldinfo [i].length, fldinfo [i].precision));
			break;
		}
		case ColTime:
			break;
		default:
			(*lib_error_handler) ("Table::AppToCISAM",
				"Bad type on \"%s\" : %d",
				fields [i],
				fldinfo [i].type);
		}
	}
}

/*
 *	Support functions
 */
static void
GrokAndClauses (
 Array<bool> &		ands,
 enum CondCmp		condition,
 const Constraint &	conds)
{
	/*
	 *	Search the And clauses for given condition
	 */
	if (conds.C ())
	{
		const Condition	&c = (const Condition &) conds;

		assert (c.ColNo () > 0);			// internal library error otherwise
		if (c.Cond () == condition)
			ands [c.ColNo ()] = true;
		return;
	}

	/*
	 *	Constraint Group - look at And-Group only
	 */
	const CGroup &cg = (const CGroup &) conds;

	for (int i = 0; i < cg.AndC (); i++)
		GrokAndClauses (ands, condition, cg.And (i));
}

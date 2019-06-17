#ident	"$Id: Database.C,v 5.0 2001/06/19 08:17:29 cha Exp $"
/*
 *	Database
 *		- Connection to Informix Databases via CISAM
 *
 *******************************************************************************
 *	$Log: Database.C,v $
 *	Revision 5.0  2001/06/19 08:17:29  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1999/02/18 23:57:47  jonc
 *	Minor reog for unused code.
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 */
#include	<assert.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>

#include	<isam.h>				// CISAM
#include	<decimal.h>				// CISAM
#include	<sqltypes.h>			// ESQL/C

#include	<osdeps.h>
#include	<minor.h>
#include	<liberr.h>
#include	<Database.h>
#include	<Table.h>
#include	<Constraint.h>

#include	<String.h>				// Containers
#include	<Date.h>
#include	<Money.h>

#include	"cisamdefs.h"

//#define	LOG_FILE_SUPPORT		// not used

/*
 *	Library version string
 */
#include	"Project"
#define		LIBRARYNAME		"C++ CISAM-dbif"
#include	<LibraryVersion.h>

/*
 *	Some fixed strings and constants
 */
static const char *	DBSuffix	= ".dbs";	// standard INFORMIX suffix

#ifdef	LOG_FILE_SUPPORT
const char *	LogFile	= "trans.log";	// transaction log file
#endif

#define	SYSTAB_TABNAME_LEN		18	// systables.tabnname length

#define	SYSTAB_DIRPATH_OFF		26	// systables.dirpath offset
#define	SYSTAB_TABID_OFF		90	// systables.tabid offset
#define	SYSTAB_ROWSIZE_OFF		94	// systables.rowsize offset
#define	SYSTAB_NCOLS_OFF		96	// systables.ncols offset
#define	SYSCOL_COLNAME_OFF		0	// syscolumns.colname offset
#define	SYSCOL_TABID_OFF		18	// syscolumns.tabid offset
#define	SYSCOL_COLNO_OFF		22	// syscolumns.colno offset
#define	SYSCOL_COLTYPE_OFF		24	// syscolumns.coltype offset
#define	SYSCOL_COLLENGTH_OFF	26	// syscolumns.collength offset
#define	SYSIDX_IDXNAME_OFF		0	// sysindexes.idxname offset
#define	SYSIDX_TABID_OFF		26	// sysindexes.tabid offset
#define	SYSIDX_PART1_OFF		32	// sysindexes.part1 offset
#define	SYSIDX_PART_OFF(x)	(SYSIDX_PART1_OFF + 2 * (x - 1))	// from 1..8

/*
 *	Local variables
 *
 *	- with all non primitive data-types, use a pointer to avoid chicken-egg
 *	  problem with global object construction on startup
 */
static String *	dbname = NULL;		// database directory

struct SysInfo
{
	int	fd;							// isam fd number
	char *	buffer;					// record buffer
	struct keydesc	kinfo;			// significant key used
};

static SysInfo	systab,				// INFORMIX system tables
				syscol,
				sysidx;

static bool		inTransX = false;	// transaction control
static int		writeOpen = 0;		// number of write files open

/*
 *	Local functions
 */
static void	InitSysTab (void),
			InitSysCol (void),
			InitSysIdx (void);

static bool		HasDB (const char *, const char *);

#ifdef	LOG_FILE_SUPPORT
static void		CreatLogFile (const String &);
#endif

static int		align (enum ColumnType, int),
				appstoresz (enum ColumnType);
static void		ConvertType (ColumnInfo &, int, int);

static bool		ReadSysIdxOnTabid (long);

static char *	ExtractIdxName (char *);

/*
 *	External interface
 */

/*
 *	Static functions
 */
int
Database::ISAMOpen (
 const char *	table,
 bool			withwrite)
{
	int		fd,
			mode;
	String	tbname (*dbname);

	assert (writeOpen >= 0);

	/*
	 *	Build the name up
	 */
	cat (tbname, '/', table, tbname);

	/*
	 *	Open file:
	 *		- read & write
	 *		- automatic locking
	 *		- transaction processing?
	 */
	if (withwrite)
	{
#if 1
		mode = ISINOUT | ISMANULOCK;
#else
		mode = ISINOUT | ISMANULOCK | (inTransX ? 0 : ISTRANS);
#endif
		writeOpen++;
	}
	else
		mode = ISINPUT | ISMANULOCK;

	fd = isopen (tbname, mode);
	if (fd < 0)
		(*lib_error_handler) ("Database::ISAMOpen",
			"isopen (%s) failed [%d]",
			(const char *) tbname,
			iserrno);

	return (fd);
}

int
Database::ISAMClose (
 int	fd,
 bool	withWrite)
{
	if (withWrite)
	{
		assert (writeOpen);
		writeOpen--;
	}
	return (!isclose (fd));
}

void
Database::GetTabInfo (
 const char *	tbname,
 unsigned		fldc,
 const char *	flds [],
 char *			fname,
 long &			tabid,
 unsigned &		rowsz,
 unsigned &		appbfsz,
 ColumnInfo * &	info,
 unsigned &		nfldc,
 char ** &		nflds,
 ColumnInfo * &	ninfo)
{
	/*
	 *	Extract info from system catalog for a table
	 *
	 *		1. get tabid from systables
	 *		2. get column info from syscolumns using tabid+columnname
	 */
	int	c;

	/*
	 *	Set up search parameters on systables
	 *
	 *		- partial key searches require successive calls to isstart()
	 */
	stchar (tbname,
		systab.buffer + systab.kinfo.k_part [0].kp_start,
		SYSTAB_TABNAME_LEN);
	if (isstart (
			systab.fd,
			&systab.kinfo,
			systab.kinfo.k_part [0].kp_leng,	// only use tabname
			systab.buffer,
			ISEQUAL) ||
		isread (systab.fd, systab.buffer, ISCURR))
	{
		(*lib_error_handler) (
			"Database::GetInfo",
			"No entry for %s (%d)",
			tbname,
			iserrno);
	}

	ldchar (systab.buffer + SYSTAB_DIRPATH_OFF, MAX_PATHNAME_LEN, fname);
	clip (fname);
	tabid = ldlong (systab.buffer + SYSTAB_TABID_OFF);
	rowsz = ldint (systab.buffer + SYSTAB_ROWSIZE_OFF);

	unsigned	tblfldC = ldint (systab.buffer + SYSTAB_NCOLS_OFF);

	if (fldc > tblfldC)
	{
		(*app_error_handler) ("Database::GetTabInfo",
			"More fields submitted (%u) than in table %s (%u)",
			fldc, tbname, tblfldC);
	}
	nfldc = tblfldC - fldc;

	/*
	 *	Allocate and Initiliase the Column Info arrays
	 */
	info = new ColumnInfo [fldc];

	nflds = new char * [nfldc];
	ninfo = new ColumnInfo [nfldc];

	/*
	 *	Iteratively look thru' all the columns for with
	 *	the matching tabid, and cross ref against the field-name list
	 */
	int	c_off = 0;			// offset for CISAM buffer
	unsigned	nidx = 0;	// to reference the unsused columns

	for (c = 1; true; c++)	// column numbers begin from 1
	{
		bool	used = false;
		char	colname [MAX_COLNAME_LEN + 1];

		/*
		 *	Read in syscolumn with matching tabid+colno
		 */
		stlong (tabid, syscol.buffer + SYSCOL_TABID_OFF);
		stint (c, syscol.buffer + SYSCOL_COLNO_OFF);
		if (isread (syscol.fd, syscol.buffer, ISEQUAL))
			break;

		ldchar (syscol.buffer + SYSCOL_COLNAME_OFF, MAX_COLNAME_LEN, colname);
		clip (colname);

		int	coltype = ldint (syscol.buffer + SYSCOL_COLTYPE_OFF) & SQLTYPE,
			collen = ldint (syscol.buffer + SYSCOL_COLLENGTH_OFF);

		for (int f = 0; flds [f]; f++)
			if (!strcmp (flds [f], colname))
			{
				/*
				 *	Translate the INFORMIX data type to something
				 *	we can understand easily.
				 *
				 *	Work out alignment+offset for the application buffer
				 *	during Queries
				 *
				 */
				/*
				 */
				info [f].c_off = c_off;		// Internal CISAM offset

				ConvertType (info [f], coltype, collen);

				/*
				 *	Store the column id for further searches
				 */
				info [f].colno = c;

				used = true;
				break;
			}

		if (!used)
		{
			/*
			 *	If the column isn't used, we store it in an
			 *	adjacent store for future reference
			 */
			nflds [nidx] = strcpy (new char [strlen (colname) + 1], colname);
			ConvertType (ninfo [nidx], coltype, collen);
			ninfo [nidx].c_off = c_off;
			ninfo [nidx].colno = c;			// Internal CISAM offset
			nidx++;
		}

		c_off += (coltype == SQLDECIMAL || coltype == SQLMONEY) ?
				 DECLENGTH (collen) : collen;

	}

	/*
	 *	Work out application offsets.
	 *
	 *	This has to be worked out separately to ensure
	 *	correct offsets for out-of-sequence columns
	 */
	appbfsz = 0;			// offset for application buffer

	for (c = 0; flds [c]; c++)
	{
		if (!info [c].type)
		{
			/*
			 *	All the previous checks in the code actually means
			 *	that the application has supplied an invalid column
			 *	if info [c].type == ColBad
			 */
			(*app_error_handler) (
				"Database::GetInfo",
				"Column \"%s\" not in system catalog for table \"%s\"",
				flds [c], tbname);
		}

		info [c].a_off = appbfsz + align (info [c].type, appbfsz);
		appbfsz = info [c].a_off + appstoresz (info [c].type);
	}

	/*
	 *	Sanity checks
	 */
	if (nidx != nfldc)
		(*lib_error_handler) (
			"Database::GetInfo",
			"Internal error: unused count mismatch for \"%s\"",
			tbname);
	if ((unsigned) c_off != rowsz)
		(*lib_error_handler) (
			"Database::GetInfo",
			"Internal error: Size offsets inconsistency for \"%s\"",
			tbname);
}

void
Database::GetIdxInfo (
 long		tabid,
 int &		idxcount,
 char ** &	idxnames,
 short ** &	idxcols)
{
	int				i;
	Array<short *>	cols;
	Array<char *>	names;

	idxcount = 0;

	/*
	 *	Extract index info for given tabid
	 */
	if (!ReadSysIdxOnTabid (tabid))
		return;

	do
	{
		short *	sysparts = new short [MAX_IDX_PARTS];
		char *	name;

		/*
		 *	Load key-parts
		 */
		for (i = 0; i < MAX_IDX_PARTS; i++)
			sysparts [i] = ldint (sysidx.buffer + SYSIDX_PART_OFF (i + 1));

		name = ExtractIdxName (sysidx.buffer);

		cols [idxcount] = sysparts;
		names [idxcount] = strcpy (new char [strlen (name) + 1], name);

		idxcount++;

	}	while (!isread (sysidx.fd, sysidx.buffer, ISNEXT) &&
			   ldlong (sysidx.buffer + SYSIDX_TABID_OFF) == tabid);

	/*
	 *	Transfer information out
	 */
	idxnames = new char * [idxcount];
	idxcols = new short * [idxcount];
	for (i = 0; i < idxcount; i++)
	{
		idxnames [i] = names [i];
		idxcols [i] = cols [i];
	}
}

void
Database::BeginTrX ()
{
	if (writeOpen)
		(*lib_error_handler) ("Database::BeginTrX",
			"Invoked with Writable files open");

	if (inTransX)
		(*lib_error_handler) ("Database::BeginTrX",
			"Invoked in middle of transaction");

	if (::isbegin ())
		(*lib_error_handler) ("Database::BeginTrX",
			"isbegin() failed with %d", iserrno);
	inTransX = true;
}

void
Database::CommitTrX ()
{
	if (!inTransX)
		(*lib_error_handler) ("Database::CommitTrX",
			"Invoked without transaction");

	if (::iscommit ())
		(*lib_error_handler) ("Database::CommitTrX",
			"iscommit() failed with %d", iserrno);
	inTransX = false;
}

void
Database::AbortTrX ()
{
	if (!inTransX)
		(*lib_error_handler) ("Database::AbortTrX",
			"Invoked without transaction");

	if (::isrollback ())
		(*lib_error_handler) ("Database::AbortTrX",
			"iscommit() failed with %d", iserrno);
	inTransX = false;
}

bool
Database::InTrX ()
{
	return (inTransX);
}

/*
 *	Constructors, Destructors
 */
Database::Database (
 const Database &)
{
	(*app_error_handler) ("Database::Database",
		"Copy constructor not supported!");
}

Database::Database (
 const char *	db)
{
	/*
	 *	Check for multiple invocations
	 */
	if (dbname)
		(*lib_error_handler) (
			"Database::Database",
			"Multiple invocation. Previously Database (%s)",
			dbname -> chars ());

	dbname = new String;

	/*
	 *	Check for database against the Informix's DBPATH
	 */
	bool	foundDb = false;
	char *	env = getenv ("DBPATH");

	if (env)
	{
		char *	tdir = NULL;
		char *	dbpath = strdup (env);

		tdir = strtok (dbpath, ":");
		do
		{
			if (HasDB (tdir, db))
			{
				cat (tdir, '/', db, *dbname);
				*dbname += DBSuffix;
				foundDb = true;
			}
		}	while (!foundDb && (tdir = strtok (NULL, ":")));

		free (dbpath);
	}
	else
	{
		/*
		 *	Assume current directory
		 */
		if (HasDB (NULL, db))
		{
			cat (db, DBSuffix, *dbname);
			foundDb = true;
		}
	}

	/*
	 *	Fatal error
	 */
	if (!foundDb)
		(*lib_error_handler) ("Database::Database",
			"Database (%s) not found",
			db);

	/*
	 *	Open system tables
	 */
	InitSysTab ();
	InitSysCol ();
	InitSysIdx ();

#ifdef	LOG_FILE_SUPPORT
	/*
	 *	Create transaction log file
	 */
	CreatLogFile (*dbname);
#endif
}

Database::~Database ()
{
	/*
	 *	Close system tables and free buffers
	 */
	ISAMClose (systab.fd);
	ISAMClose (syscol.fd);
	ISAMClose (sysidx.fd);

	delete [] systab.buffer;
	delete [] syscol.buffer;
	delete [] sysidx.buffer;

	delete dbname;
	dbname = NULL;
}

const char *
Database::Name () const
{
	return (*dbname);
}

/***************************
 *	Local support functions
 ***************************/
/*
 *	Initialize the System Table structures
 */
static int
InitSysInfo (
 const char *	name,
 SysInfo *		info)
{
	/*
	 *	Get the dictionary info about the table
	 */
	int	nkeys;
	struct dictinfo	dinfo;

	info -> fd = Database::ISAMOpen (name);

	isindexinfo (info -> fd, (struct keydesc *) &dinfo, 0);
	if ((nkeys = dinfo.di_nkeys & 0x7fff) <= 0)
		(*lib_error_handler) (
			"Database::InitSysTab",
			"Key count for %s <= 0?",
			name);

	info -> buffer = new char [dinfo.di_recsize];	// allocate record buffer
	return (nkeys);
}

static void
InitSysTab ()
{
	/*
	 *	Set the key to use to be tabname+owner
	 *		- we're guessing that it's the first one with 2 parts
	 */
	int	nkeys = InitSysInfo ("systables", &systab);

	for (int i = 1; i <= nkeys; i++)
	{
		isindexinfo (systab.fd, &systab.kinfo, i);
		if (systab.kinfo.k_nparts == 2)
			return;
	}

	(*lib_error_handler) (
		"Database::InitSysTab",
		"tabname+owner index not found?");
}

static void
InitSysCol ()
{
	/*
	 *	Set the key to use to be tabid+colno
	 *		- we're guessing that it's the first one with 2 parts
	 */
	int	nkeys = InitSysInfo ("syscolumns", &syscol);

	for (int i = 1; i <= nkeys; i++)
	{
		isindexinfo (syscol.fd, &syscol.kinfo, i);
		if (syscol.kinfo.k_nparts == 2)
		{
			/*
			 *	Use this as permanent key
			 */
			isstart (
				syscol.fd,
				&syscol.kinfo,
				0,						// use whole key
				syscol.buffer,
				ISFIRST);
			return;
		}
	}

	(*lib_error_handler) (
		"Database::InitSysCol",
		"tabname+owner index not found?");
}

static void
InitSysIdx ()
{
	/*
	 *	Set the keys to be used on sysidx
	 *	Use the one on tabid
	 *
	 */
	int		nkeys = InitSysInfo ("sysindexes", &sysidx);

	for (int i = 1; i <= nkeys; i++)
	{
		isindexinfo (sysidx.fd, &sysidx.kinfo, i);
		if (sysidx.kinfo.k_nparts == 1)
		{
			/*
			 *	Use as permanent index
			 */
			if (isstart (
					sysidx.fd,
					&sysidx.kinfo,
					0,
					sysidx.buffer,
					ISFIRST))
			{
				(*lib_error_handler) ("Database::InitSysIdx",
					"isstart returns %d", iserrno);
			}

			return;
		}
	}

	(*lib_error_handler) ("Database::InitSysIdx", "indexes not found?");
}

/*
 *	Support functions
 */
static bool
HasDB (
 const char *	dir,
 const char *	name)
{
	String	dbName;
	struct stat	info;

	if (!dir || !*dir)
		cat (name, DBSuffix, dbName);
	else
	{
		cat (dir, '/', dbName);
		cat (dbName, name, DBSuffix, dbName);
	}

	if (stat (dbName, &info) < 0)
		return (false);
	return ((bool) S_ISDIR (info.st_mode));		// Is it a directory?
}

#ifdef	LOG_FILE_SUPPORT
static void
CreatLogFile (
 const String &	dbdir)
{
	String	logfile;
	struct stat	info;

	/*
	 *	Create the transaction log file if required
	 */
	cat (dbdir, '/', LogFile, logfile);
	if (stat (logfile, &info) < 0)
	{
		int	fd = ::creat (logfile, 0666);

		if (fd < 0)
			(*lib_error_handler) ("Database(CreatLogFile)",
				"Can't create transaction log-file %s",
				(const char *) logfile);

		::close (fd);
	}
	else if (!S_ISREG (info.st_mode))
		(*lib_error_handler) ("Database(CreatLogFile)",
			"Odd transaction log-file %s", (const char *) logfile);

	/*
	 *	Open for use by CISAM
	 */
	if (::islogopen (logfile))
		(*lib_error_handler) ("Database(CreatLogFile)",
			"islogopen fails with %d", iserrno);
}
#endif

static int
align (
 enum ColumnType	t,		// column type
 int				off)	// current offset
{
	/*
	 *	Work out alignments for application buffer
	 *
	 *		- no, we can't allow other interfaces aside from those
	 *		  defined here - there's just too many hassles otherwise
	 */
	struct align_char	{	char	a;	String	b;	};
	struct align_short	{	char	a;	short	b;	};
	struct align_long	{	char	a;	long	b;	};
	struct align_double	{	char	a;	double	b;	};
	struct align_float	{	char	a;	float	b;	};
	struct align_dec	{	char	a;	Number	b;	};
	struct align_date	{	char	a;	Date	b;	};
	struct align_money	{	char	a;	Money	b;	};

	int	align_on = 0;

	switch (t)
	{
	case ColChar:
		align_on = offsetof (struct align_char, b);
		break;
	case ColShort:
		align_on = offsetof (struct align_short, b);
		break;
	case ColLong:
	case ColSerial:
		align_on = offsetof (struct align_long, b);
		break;
	case ColDouble:
		align_on = offsetof (struct align_double, b);
		break;
	case ColFloat:
		align_on = offsetof (struct align_float, b);
		break;
	case ColDecimal:
		align_on = offsetof (struct align_dec, b);
		break;
	case ColDate:
		align_on = offsetof (struct align_date, b);
		break;
	case ColMoney:
		align_on = offsetof (struct align_money, b);
		break;
#if 0
	case ColTime:
		break;
#endif
	default:
		(*lib_error_handler) (
			"Database(align)",
			"Unknown alignment type requested %d", t);
	}

	int	fudge = off % align_on;
	return (fudge ? align_on - fudge : 0);
}

static int
appstoresz (
 enum ColumnType	t)
{
	/*
	 *	Returns the application storage type size for
	 *	given SQL type
	 */
	switch (t)
	{
	case ColChar:		return (sizeof (String));
	case ColShort:		return (sizeof (short));
	case ColSerial:
	case ColLong:		return (sizeof (long));
	case ColDouble:		return (sizeof (double));
	case ColFloat:		return (sizeof (float));
	case ColDecimal:	return (sizeof (Number));
	case ColDate:		return (sizeof (Date));
	case ColMoney:		return (sizeof (Money));
	default:
		(*lib_error_handler) (
			"Database(appstorsz)",
			"Unknown storage type requested %d", t);
	}
	return (0);
}

static void
ConvertType (
 ColumnInfo &	info,
 int			informix_type,
 int			collen)
{
	switch (informix_type)
	{
	case SQLCHAR:
		info.type = ColChar;
		info.length = collen;
		break;
	case SQLSMINT:
		info.type = ColShort;
		break;
	case SQLINT:
		info.type = ColLong;
		break;
	case SQLFLOAT:
		info.type = ColDouble;
		break;
	case SQLSMFLOAT:
		info.type = ColFloat;
		break;
	case SQLDECIMAL:
		info.type = ColDecimal;
		info.length = PRECTOT (collen);
		info.precision = PRECDEC (collen);
		break;
	case SQLSERIAL:
		info.type = ColSerial;
		break;
	case SQLDATE:
		info.type = ColDate;
		break;
	case SQLMONEY:
		info.type = ColMoney;
		info.length = PRECTOT (collen);
		info.precision = PRECDEC (collen);
		break;
#if 0
	case SQLDTIME:
		info.type = ColTime;
		break;
#endif
	default:
		(*lib_error_handler) (
			"Database::ConvertType",
			"Unsupported type %d",
			informix_type);
	}
}

static bool
ReadSysIdxOnTabid (
 long	tabid)
{
	/*
	 *	Read in sysindexes rows with matching tabid
	 */
	stlong (tabid, sysidx.buffer + SYSIDX_TABID_OFF);
	if (isread (sysidx.fd, sysidx.buffer, ISEQUAL))
		return (false);
	return (true);
}

static char *
ExtractIdxName (
 char *	buf)
{
	static char name [MAX_IDXNAME_LEN + 1];

	ldchar (buf + SYSIDX_IDXNAME_OFF, MAX_IDXNAME_LEN, name);
	return (clip (name));
}

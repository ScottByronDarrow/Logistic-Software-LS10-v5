#ident	"$Id: gen-access.c,v 5.0 2001/06/19 07:07:43 cha Exp $"
/*
 *	General access functions
 *
 *	These allow access to system-catalogs in a somewhat neutral manner.
 *
 *******************************************************************************
 *	$Log: gen-access.c,v $
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.5  1999/11/29 20:34:32  jonc
 *	Fixed: Crash on new database creation.
 *	
 *	Revision 1.4  1999/10/26 22:48:35  jonc
 *	Added system-catalog interface.
 *	
 *	Revision 1.3  1999/10/06 23:55:02  jonc
 *	Tightened usage of char * args to const char *.
 *	
 *	Revision 1.2  1999/09/13 06:28:15  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1  1999/07/14 23:47:26  jonc
 *	Added generic access support.
 *	
 */
#include	<dberr.h>

#include	"dbcisam.h"
#include	<std_decs.h>

#define	MAXKEYS	8						/* INFORMIX index limit */

/*
 *	Wrapper structure for catalog
 */
struct _IndexColInfo					/* Index + column info */
{
	struct IndexInfo index;
	int cols [MAXKEYS];
};
typedef struct _IndexColInfo IndexColInfo;

struct _TableInfoList
{
	int tabid;
	struct TableInfo info;
	struct ColumnInfo * columns;

	IndexColInfo * indexes;

	struct _TableInfoList * next;
};
typedef struct _TableInfoList TableInfoList;

/*
 *	Local functions
 */
static int OpenTableCatalog (const char *),
		   OpenIndexCatalog (const char *, int tabid),
		   OpenColumnCatalog (const char *, int tabid);
static void AddTableInfoList (TableInfoList **, TableInfoList *),
			DestroyTableInfoList (TableInfoList **);
static TableInfoList * TableInfoListNode (int, const char *, int, int);
static TableInfoList * LocateTableInfoByNo (int),
					 * LocateTableInfoByName (const char *);
static void BuildColumnInfo (int fd, TableInfoList * node),
			BuildIndexInfo (int fd, TableInfoList * node);

/*
 *	Local variables
 */
static int tablecount = -1;
TableInfoList * catalog = NULL;

/*
 *	External interface
 */
int
TableCount ()
{
	/*
	 *	Initialise the catalog system
	 *
	 *	Open tabname and read in all tablenames where > tabid > 100
	 */
	if (tablecount < 0)
	{
		int tabfd = OpenTableCatalog (_dbpath);
		char record [1024];

		/*
		 *	Read the info into a list
		 */
		for (tablecount = 0; !isread (tabfd, record, ISNEXT); tablecount++)
		{
			/*
			 *	Only 18 chars allocated to the name
			 */
			char tabname [19];

			ldchar (record + SYSTAB_TABNAME, 18, tabname);
			tabname [18] = '\0';

			AddTableInfoList (&catalog,
				TableInfoListNode (
					ldlong (record + SYSTAB_TABID),
					clip (tabname),
					ldint (record + SYSTAB_NCOLS),
					ldint (record + SYSTAB_NINDEX)));
		}

		isclose (tabfd);
	}

	return tablecount;
}

int
TableNumber (
 const char * name)
{
	int i;
	TableInfoList * node;

	for (i = 0, node = catalog; node; i++, node = node -> next)
		if (!strcmp (name, node -> info.name))
			return i;

	return -1;
}

void
TableInfo (
 int tableno,
 struct TableInfo * info)
{
	if (tableno >= tablecount)
		dbase_err ("TableInfo", NULL, NOTEXIST);

	*info = LocateTableInfoByNo (tableno) -> info;
}

void
TableIndexInfo (
 int tableno,
 int indexno,
 struct IndexInfo * info)
{
	TableInfoList * node = NULL;

	if (tableno >= tablecount)
		dbase_err ("TableIndexInfo:table", NULL, NOTEXIST);

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes)
		dbase_err ("TableIndexInfo:index", NULL, NOTEXIST);

	if (!node -> indexes)
	{
		/*
		 *	Have to build up the index information
		 */
		int idxfd = OpenIndexCatalog (_dbpath, node -> tabid);

		BuildIndexInfo (idxfd, node);

		isclose (idxfd);
	}

	*info = node -> indexes [indexno].index;
}

void
TableIndexColumnInfo (
 int tableno,
 int indexno,
 int colno,
 struct ColumnInfo * info)
{
	TableInfoList * node = NULL;

	if (tableno >= tablecount)
		dbase_err ("TableIndexInfo:table", NULL, NOTEXIST);

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes ||					/* bad indexno */
		!node -> indexes ||									/* no info */
		colno >= node -> indexes [indexno].index.ncolumn)	/* bad colno */
	{
		dbase_err ("TableIndexInfo:index", NULL, NOTEXIST);
	}

	*info = node -> columns [node -> indexes [indexno].cols [colno]];
}

int
TableColumnCount (
 const char * name)
{
	LLIST	*tptr = _GetNode (name);
	TableInfoList * node;

	if (tptr)
		return tptr -> columncount;

	/*
	 *	Check the catalog system files for a possible listing
	 */
	if ((node = LocateTableInfoByName (name)))
		return node -> info.ncolumn;

	dbase_err ("TableColumnCount", name, NOFILENAME);
	return 0;
}

void
TableColumnInfo (
 const char * name,
 int colno,
 struct ColumnInfo * buffer)
{
	LLIST	*tptr = _GetNode (name);

	if (tptr)
	{
		/*
		 *	Info is available from opened file
		 */
		if (colno >= tptr -> columncount)
			dbase_err ("TableColumnCount", name, NOFIELD);

		*buffer = tptr -> columnlist [colno];

	} else
	{
		/*
		 *	Check offline catalog
		 */
		TableInfoList * node = LocateTableInfoByName (name);

		if (!node)
			dbase_err ("TableColumnCount", name, NOFILENAME);

		if (colno >= node -> info.ncolumn)
			dbase_err ("TableColumnCount", name, NOFIELD);

		if (!node -> columns)
		{
			/*
			 *	Build up the column info
			 */
			int colfd = OpenColumnCatalog (_dbpath, node -> tabid);

			BuildColumnInfo (colfd, node);
			isclose (colfd);
		}

		*buffer = node -> columns [colno];
	}
}

void
TableColumnGet (
 const char * name,
 int colno,
 void * buffer)
{
	LLIST *	tptr = _GetNode (name);
	char *	rawbuf;
	short	nullflag;

	if (!tptr)
		dbase_err ("TableColumnGet", name, NOFILENAME);
	if (colno >= tptr -> columncount)
		dbase_err ("TableColumnGet", name, NOFIELD);

	rawbuf = tptr -> _buffer + tptr -> columncisam [colno];

	switch (tptr -> columnlist [colno].type)
	{
	case CT_Chars:
		ldchar (rawbuf, tptr -> columnlist [colno].size, (char *) buffer);
		break;

	case CT_Long:
	case CT_Serial:
	case CT_Date:
		{
			long	value = ldlong (rawbuf);

			if (value == LONGNULL)
				value = 0;
			*(long *) buffer = value;
		}
		break;

	case CT_Short:
		{
			int	value = ldint (rawbuf);

			if (value == INTNULL)
				value = 0;
			*(int *) buffer = value;
		}
		break;

	case CT_Float:
		{
			float	value = ldfltnull (rawbuf, &nullflag);

			*(float *) buffer = value;
		}
		break;

	case CT_Double:
		{
			double	value = lddblnull (rawbuf, &nullflag);

			*(double *) buffer = value;
		}
		break;

	case CT_Money:
		{
			/*
			 * Money types are extracted as dec_t (as dollars), but used
			 * as double (as cents) within application code
			 */
			dec_t	d;
			double	value;

			if (lddecimal (rawbuf, tptr -> columnlist [colno].size, &d) ||
				d.dec_pos == DECPOSNULL)
			{
				/*
				 *	Overflow/Underflow/NULL value
				 */
				value = 0;				/* use a safe value */
			}
			else
			{
				dectodbl (&d, &value);	/* convert to double */
				value *= 100;			/* convert to cents */
			}
			*(double *) buffer = value;
		}
		break;

	case CT_Number:
	default:
		dbase_err ("TableColumnGet", name, NOVAL);
	}
}

void
TableColumnNameGet (
 const char * name,
 const char * colname,
 void * buffer)
{
	int		colno;
	LLIST *	tptr = _GetNode (name);

	if (!tptr)
		dbase_err ("TableColumnGet", name, NOFILENAME);
	for (colno = 0; colno < tptr -> columncount; colno++)
	{
		if (!strcmp (colname, tptr -> columnlist [colno].name))
		{
			TableColumnGet (name, colno, buffer);
			return;
		}
	}

	dbase_err ("TableColumnCount", name, NOFIELD);
}

/*
 *	Support functions
 */
static int
OpenTableCatalog (
 const char * prefix)
{
	int fd;
	char pathname [256];
	char record [1024];
	struct keydesc key;

	sprintf (pathname, "%s/systables", prefix);
	if ((fd = isopen (pathname, ISINPUT + ISMANULOCK)) < 0)
		dbase_err ("OpenTableCatalog:isopen", "systables", iserrno);

	/*
	 *	Set up to use tabid as key
	 */
	memset (&key, 0, sizeof (struct keydesc));
	key.k_flags = ISNODUPS + DCOMPRESS + ISCLUSTER;
	key.k_nparts = 1;
	key.k_part [0].kp_start = SYSTAB_TABID;
	key.k_part [0].kp_leng = LONGSIZE;
	key.k_part [0].kp_type = LONGTYPE;

	memset (record, 0, sizeof (record));
	stlong (100, record + SYSTAB_TABID);			/* initial key value */

	if (isstart (fd, &key, 0, record, ISGTEQ) < 0)
	{
		/*
		 *	Hmm. New database
		 */
	}

	return fd;
}

static int
OpenIndexCatalog (
 const char * prefix,
 int tabid)
{
	int fd;
	char pathname [256];
	char record [1024];
	struct keydesc key;

	sprintf (pathname, "%s/sysindexes", prefix);
	if ((fd = isopen (pathname, ISINPUT + ISMANULOCK)) < 0)
		dbase_err ("OpenIndexCatalog:isopen", "systables", iserrno);

	/*
	 *	Set up to use tabid as key
	 */
	memset (&key, 0, sizeof (struct keydesc));
	key.k_flags = ISNODUPS + DCOMPRESS + ISCLUSTER;
	key.k_nparts = 1;

	key.k_part [0].kp_start = SYSIDX_TABID;
	key.k_part [0].kp_leng = LONGSIZE;
	key.k_part [0].kp_type = LONGTYPE;

	memset (record, 0, sizeof (record));
	stlong (tabid, record + SYSIDX_TABID);			/* initial key value */

	if (isstart (fd, &key, 0, record, ISGTEQ) < 0)
		dbase_err ("OpenIndexCatalog:isstart", "systables", iserrno);

	return fd;
}

static int
OpenColumnCatalog (
 const char * prefix,
 int tabid)
{
	int fd;
	char pathname [256];
	char record [1024];
	struct keydesc key;

	sprintf (pathname, "%s/syscolumns", prefix);
	if ((fd = isopen (pathname, ISINPUT + ISMANULOCK)) < 0)
		dbase_err ("OpenColumnCatalog:isopen", "systables", iserrno);

	/*
	 *	Set up to use tabid as key
	 */
	memset (&key, 0, sizeof (struct keydesc));
	key.k_flags = ISNODUPS + DCOMPRESS + ISCLUSTER;
	key.k_nparts = 2;

	key.k_part [0].kp_start = SYSCOL_TABID;
	key.k_part [0].kp_leng = LONGSIZE;
	key.k_part [0].kp_type = LONGTYPE;
	key.k_part [1].kp_start = SYSCOL_COLNO;
	key.k_part [1].kp_leng = INTSIZE;
	key.k_part [1].kp_type = INTTYPE;

	memset (record, 0, sizeof (record));
	stlong (tabid, record + SYSCOL_TABID);			/* initial key value */

	if (isstart (fd, &key, 0, record, ISGTEQ) < 0)
		dbase_err ("OpenColumnCatalog:isstart", "systables", iserrno);

	return fd;
}

static void
BuildIndexInfo (
 int fd,
 TableInfoList * node)
{
	int i;
	unsigned size = node -> info.nindexes * sizeof (IndexColInfo);
	char record [1024];

	node -> indexes = malloc (size);
	memset (node -> indexes, 0, size);

	/*
	 *	Read thru' system-catalog for index info
	 */
	for (i = 0; !isread (fd, record, ISNEXT); i++)
	{
		int p;
		char unique;
		long tabid = ldlong (record + SYSIDX_TABID);

		if (tabid != node -> tabid)
			break;

		/*
		 *	Name
		 */
		ldchar (record + SYSIDX_IDXNAME, 18, node -> indexes [i].index.name);
		node -> indexes [i].index.name [18] = '\0';
		clip (node -> indexes [i].index.name);

		/*
		 *	Unique
		 */
		ldchar (record + SYSIDX_UNIQUE, 1, &unique);
		node -> indexes [i].index.isunique = unique == 'U';

		/*
		 *	Work out the columns. INFORMIX indexes have
		 *	a max of 8 parts
		 */
		for (p = 0; p < MAXKEYS; p++)
		{
			int colno = ldint (record + SYSIDX_PART(p));

			if (!colno)
				break;

			/*
			 *	INFORMIX numbers it columns from 1
			 *	We number from 0
			 */
			node -> indexes [i].cols [p] = colno - 1;
		}
		node -> indexes [i].index.ncolumn = p;
	}
}

static void
BuildColumnInfo (
 int fd,
 TableInfoList * node)
{
	int c;
	unsigned size = node -> info.ncolumn * sizeof (struct ColumnInfo);
	char record [1024];

	node -> columns = malloc (size);
	memset (node -> columns, 0, size);

	/*
	 *	Read thru' system-catalog for column info
	 */
	for (c = 0; !isread (fd, record, ISNEXT); c++)
	{
		long tabid = ldlong (record + SYSCOL_TABID);
		int type = ldint (record + SYSCOL_COLTYPE) & SQLTYPE,
			len = ldint (record + SYSCOL_COLLENGTH);

		if (tabid != node -> tabid)
			break;

		ldchar (record + SYSCOL_COLNAME, 18, node -> columns [c].name);
		node -> columns [c].name [18] = '\0';
		clip (node -> columns [c].name);

		switch (type)
		{
		case SQLCHAR:
			node -> columns [c].type = CT_Chars;
			node -> columns [c].size = len;
			break;

		case SQLSMINT:
			node -> columns [c].type = CT_Short;
			break;

		case SQLINT:
			node -> columns [c].type = CT_Long;
			break;

		case SQLFLOAT:
			node -> columns [c].type = CT_Double;
			break;

		case SQLSMFLOAT:
			node -> columns [c].type = CT_Float;
			break;

		case SQLDECIMAL:
			node -> columns [c].type = CT_Number;
			break;

		case SQLSERIAL:
			node -> columns [c].type = CT_Serial;
			break;

		case SQLDATE:
			node -> columns [c].type = CT_Date;
			break;

		case SQLMONEY:
			node -> columns [c].type = CT_Money;
			break;
		}
	}
}

static void
AddTableInfoList (
 TableInfoList ** list,
 TableInfoList * node)
{
	if (*list)
		AddTableInfoList (&(*list) -> next, node);
	else
		*list = node;
}

static void
DestroyTableInfoList (
 TableInfoList ** list)
{
	if (!*list)
		return;

	DestroyTableInfoList (&(*list) -> next);

	if ((*list) -> columns)
		free ((*list) -> columns);
	if ((*list) -> indexes)
		free ((*list) -> indexes);
	free (*list);

	*list = NULL;
}

static TableInfoList *
TableInfoListNode (
 int tabid,
 const char * name,
 int colcount,
 int indexcount)
{
	TableInfoList * node = malloc (sizeof (TableInfoList));

	memset (node, 0, sizeof (TableInfoList));

	node -> tabid = tabid;
	strcpy (node -> info.name, name);
	node -> info.ncolumn = colcount;
	node -> info.nindexes = indexcount;

	return node;
}

static TableInfoList *
LocateTableInfoByNo (
 int tableno)
{
	TableInfoList * node = NULL;

	for (node = catalog; tableno--; node = node -> next);

	return node;
}

static TableInfoList *
LocateTableInfoByName (
 const char * name)
{
	TableInfoList * node = NULL;

	for (node = catalog; node; node = node -> next)
		if (!strcmp (name, node -> info.name))
			break;

	return node;
}

#ident	"$Id: gen-access.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	General access functions
 *
 *******************************************************************************
 *	$Log: gen-access.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.3  1999/10/31 22:42:28  jonc
 *	Fixed: occasional weird index info.
 *	
 *	Revision 1.2  1999/10/28 01:58:36  jonc
 *	Added support for generic-catalog access. Damnably slow, though.
 *	
 *	Revision 1.1  1999/10/21 21:47:04  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdlib.h>
#include	<string.h>
#include	<dbio.h>

#include	"oracledbif.h"
#include	"oraerrs.h"

/*
 *	Some magic numbers
 */
#define	NAMEBUF_LEN	32					/* ORACLE names are 30 chars long */
#define	MAXKEYS		8					/* We're limited by INFORMIX roots */

/*
 *	Wrapper structures for catalog
 */
struct _IndexColInfo					/* Index + column info */
{
	struct IndexInfo index;
	int cols [MAXKEYS];
};
typedef struct _IndexColInfo IndexColInfo;

struct _TableInfoList
{
	struct TableInfo info;
	struct ColumnInfo * columns;

	IndexColInfo * indexes;

	struct _TableInfoList * next;
};
typedef struct _TableInfoList TableInfoList;

/*
 *	Local functions
 */
static void AddTableInfoList (TableInfoList **, TableInfoList *),
			DestroyTableInfoList (TableInfoList **);
static TableInfoList * TableInfoListNode (const char *, int, int);
static TableInfoList * LocateTableInfoByNo (int),
					 * LocateTableInfoByName (const char *);

static void Prep1TableCatalog (Lda_Def * lda, Cda_Def * cda, char *, int *),
			Prep2TableCatalog (Lda_Def * lda, Cda_Def * cda, char *, int *),

			Prep1IndexCatalog (Lda_Def *, Cda_Def *, char *, char *, char *),
			Prep2IndexCatalog (Lda_Def *, Cda_Def *, char *, char *, char *),

			Prep1ColumnCatalog (Lda_Def *, Cda_Def *, char *, char *),
			Prep2ColumnCatalog (Lda_Def *, Cda_Def *, char * table,
				char * name,
				char * data_type, sb2 * i_data_type,
				int * data_length, sb2 * i_data_length,
				int * data_precision, sb2 * i_data_precision,
				int * data_scale, sb2 * i_data_scale);

static enum ColumnType ORA2ColType (enum ORAType oratype);

/*
 *	External functions
 */
extern char * clip (char *);

/*
 *	Local variables
 */
static int tablecount = -1;
TableInfoList * catalog = NULL;

/*
 *	Static areas for output
 */

static ub2	r_len, r_code;									/* discard */

/*
 *	External interface
 */
int
TableCount ()
{
	/*
	 *	Initialise the catalog system
	 */
	if (tablecount < 0)
	{
		Lda_Def * lda = _LDA ();
		Cda_Def tbl_cda;
		char tablename [NAMEBUF_LEN];
		int count;

		if (oopen (&tbl_cda, lda, NULL, -1, -1, NULL, -1))
			oraclecda_error (lda, &tbl_cda, "TableCount");

		/*
		 *	First pass to get column counts
		 */
		Prep1TableCatalog (lda, &tbl_cda, tablename, &count);
		for (tablecount = 0; !ofetch (&tbl_cda); tablecount++)
		{
			/*
			 *	Add a dummy value for number of indexes
			 *	- we update this on second pass
			 */
			AddTableInfoList (&catalog,
				TableInfoListNode (_strlower (clip (tablename)), count, 0));
		}

		/*
		 *	Second pass to get index counts
		 */
		Prep2TableCatalog (lda, &tbl_cda, tablename, &count);
		while (!ofetch (&tbl_cda))
		{
			/*
			 *	Let's be safe and locate by name
			 */
			TableInfoList * node = LocateTableInfoByName (
										_strlower (clip (tablename)));

			node -> info.nindexes = count;
		}

		oclose (&tbl_cda);
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
		oracledbif_error ("TableInfo: table number %d out of range", tableno);

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
	{
		oracledbif_error (
			"TableIndexInfo: table number %d out of range",
			tableno);
	}

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes)
	{
		oracledbif_error (
			"TableIndexInfo: index number %d out of range",
			indexno);
	}

	if (!node -> indexes)
	{
		/*
		 *	Have to build up the index information
		 */
		Lda_Def * lda = _LDA ();
		Cda_Def idx_cda;
		char tab_name [NAMEBUF_LEN],
			 idx_name [NAMEBUF_LEN],
			 col_name [NAMEBUF_LEN],
			 unique [NAMEBUF_LEN];
		unsigned sz;
		int i, j;

		if (oopen (&idx_cda, lda, NULL, -1, -1, NULL, -1))
			oraclecda_error (lda, &idx_cda, "TableCount");

		/*
		 *	Allocate index buffers and build info
		 */
		sz = node -> info.nindexes * sizeof (IndexColInfo);
		memset (node -> indexes = malloc (sz), 0, sz);

		_strupper (strcpy (tab_name, node -> info.name));
		Prep1IndexCatalog (lda, &idx_cda, tab_name, idx_name, unique);

		for (i = 0; ofetch (&idx_cda) == 0; i++)
		{
			_strlower (strcpy (node -> indexes [i].index.name, idx_name));
			node -> indexes [i].index.isunique = unique [0] == 'U';
		}

		/*
		 *	Get column info
		 */
		Prep2IndexCatalog (lda, &idx_cda, tab_name, idx_name, col_name);

		i = -1;
		while (ofetch (&idx_cda) == 0)
		{
			int col;

			_strlower (idx_name);
			if (i < 0 || strcmp (idx_name, node -> indexes [i].index.name))
			{
				if (i >= 0)
					node -> indexes [i].index.ncolumn = j;
				i++;
				j = 0;
			}

			_strlower (col_name);

			/*
			 *	Find the matching column
			 */
			for (col = 0; col < node -> info.ncolumn; col++)
			{
				if (!strcmp (col_name, node -> columns [col].name))
				{
					node -> indexes [i].cols [j++] = col;
					break;
				}
			}

			if (col >= node -> info.ncolumn)
			{
				oracledbif_error (
					"TableIndexInfo: no col-match found %s",
					col_name);
			}
		}

		if (i >= 0)
		{
			/*
			 *	Pick up trailing index
			 */
			node -> indexes [i].index.ncolumn = j;
		}

		oclose (&idx_cda);
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
	{
		oracledbif_error (
			"TableIndexColumnInfo: table number %d out of range",
			tableno);
	}

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes ||					/* bad indexno */
		!node -> indexes ||									/* no info */
		colno >= node -> indexes [indexno].index.ncolumn)	/* bad colno */
	{
		oracledbif_error (
			"TableIndexColumnInfo: bad index/column number %d/%d",
			indexno, colno);
	}

	*info = node -> columns [node -> indexes [indexno].cols [colno]];
}

int
TableColumnCount (
 const char * name)
{
	TableState * table = LocateTable (name);
	TableInfoList * node = NULL;

	if (table)
		return ( table -> columnc - 1); /* RPC */

	/*
	 *	Check the catalog system files for a possible listing
	 */
	if ((node = LocateTableInfoByName (name)))
		return node -> info.ncolumn;

	oracledbif_error ("TableColumnCount: No table %s", name);
	return 0;
}

void
TableColumnInfo (
 const char * name,
 int colno,
 struct ColumnInfo * buffer)
{
	TableState * table = LocateTable (name);

	if (table)
	{
		if (colno >= table -> columnc)
			oracledbif_error ("TableColumnInfo: bad colno %d", colno);

		/*
		 *	Convert ColumnDef to ColumnInfo
		 */
		strcpy (buffer -> name, table -> columns [colno].name);
		buffer -> size = table -> columns [colno].length;
		buffer -> type = ORA2ColType (table -> columns [colno].type);

	} else
	{
		/*
		 *	Check offline catalog
		 */
		TableInfoList * node = LocateTableInfoByName (name);

		if (!node)
			oracledbif_error ("TableColumnCount: bad table %s", name);

		if (colno >= node -> info.ncolumn)
			oracledbif_error ("TableColumnInfo: bad colno %d", colno);

		if (!node -> columns)
		{
			/*
			 *	Build up the column info
			 */
			Lda_Def * lda = _LDA ();
			Cda_Def col_cda;
			char tab_name [NAMEBUF_LEN],
				 seq_name [NAMEBUF_LEN],
				 col_name [NAMEBUF_LEN],
				 data_type [NAMEBUF_LEN];
			int data_length, data_precision, data_scale;
			sb2 i_data_type, i_data_length, i_data_precision, i_data_scale;
			unsigned sz;
			int i;

			if (oopen (&col_cda, lda, NULL, -1, -1, NULL, -1))
				oraclecda_error (lda, &col_cda, "TableCount");

			_strupper (strcpy (tab_name, node -> info.name));
			Prep1ColumnCatalog (lda, &col_cda, tab_name, seq_name);
			Prep2ColumnCatalog (lda, &col_cda, tab_name,
				col_name,
				data_type, &i_data_type,
				&data_length, &i_data_length,
				&data_precision, &i_data_precision,
				&data_scale, &i_data_scale);

			/*
			 *	Allocate buffers and fill it up
			 */
			sz = node -> info.ncolumn * sizeof (struct ColumnInfo);
			memset (node -> columns = malloc (sz), 0, sz);

			for (i = 0; ofetch (&col_cda) == 0; i++)
			{
				enum ORAType type;
				int size;

				_strlower (strcpy (node -> columns [i].name, col_name));

				_DecodeORAType (
					node -> columns [i].name, seq_name,
					data_type, data_length, data_precision, data_scale,
					i_data_precision, i_data_scale,

					&type, &size);

				node -> columns [i].type = ORA2ColType (type);

				/*
				 *	Tweak the size for external use, since
				 *	it's only suitable for internal use on Decode
				 */
				node -> columns [i].size =
					node -> columns [i].type == CT_Chars ?
					size - 1 : 0;
			}

			if (col_cda.rc != ERR_NO_DATA_FOUND)
				oraclecda_error (lda, &col_cda, "eh");

			oclose (&col_cda);
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
	TableState * table = LocateTable (name);

#if 0
	if (!table)
		dbase_err ("TableColumnGet", name, NOFILENAME);
	if (colno >= table -> columncount)
		dbase_err ("TableColumnGet", name, NOFIELD);
#endif

	_ConvRaw2App (table -> columns + colno, 0, buffer);
}

void
TableColumnNameGet (
 const char * name,
 const char * colname,
 void * buffer)
{
	int colno;
	TableState * table = LocateTable (name);

#if 0
	if (!table)
		dbase_err ("TableColumnGet", name, NOFILENAME);
#endif
	for (colno = 0; colno < table -> columnc; colno++)
	{
		if (!strcmp (colname, table -> columns [colno].name))
		{
			TableColumnGet (name, colno, buffer);
			return;
		}
	}

	oracledbif_error ("No such column %s in %s", colname, name);
}

/*
 *	Support functions
 */
static void
Prep1TableCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * name,
 int * count)
{
	/*
	 *	parse and bind output for pass to get column counts
	 */
	char * sql_tab =
			"select table_name, count (column_name) "
			"from user_tab_columns "
			"group by table_name";

	if (oparse (cda, sql_tab, -1, PARSE_NOW, PARSE_V7) ||
		odefin (cda, 1,
			(ub1 *) name, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 2,
			(ub1 *) count, sizeof (int), SQLT_INT,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "PrepTableCatalog");
	}
}

static void
Prep2TableCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * name,
 int * count)
{
	/*
	 *	parse and bind output for pass to get index counts
	 */
	char * sql_tab =
			"select table_name, count (index_name) "
			"from user_indexes "
			"group by table_name";

	if (oparse (cda, sql_tab, -1, PARSE_NOW, PARSE_V7) ||
		odefin (cda, 1,
			(ub1 *) name, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 2,
			(ub1 *) count, sizeof (int), SQLT_INT,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "Prep2TableCatalog");
	}
}

static void
Prep1IndexCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * table,
 char * index,
 char * unique)
{
	char * sql_idx =
		"select index_name, uniqueness "
		"from user_indexes "
		"where table_name = :1";

	if (oparse (cda, sql_idx, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (cda, 1,
			(ub1 *) table, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		odefin (cda, 1,
			(ub1 *) index, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 2,
			(ub1 *) unique, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "Prep1IndexCatalog");
	}
}

static void
Prep2IndexCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * table,
 char * index,
 char * column)
{
	/*
	 *	Get associated columns for a table's indexes
	 */
	char * sql_idx =
		"select index_name, column_name "
		"from user_ind_columns "
		"where table_name = :1 "
		"order by index_name, column_position";

	if (oparse (cda, sql_idx, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (cda, 1,
			(ub1 *) table, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		odefin (cda, 1,
			(ub1 *) index, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 2,
			(ub1 *) column, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "Prep1IndexCatalog");
	}
}

static void
Prep1ColumnCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * table,
 char * name)
{
	/*
	 *	Try to find associated sequences for the table
	 */
	char * sql_seqs =
		"select "
			"sequence_name "
			"from user_sequences "
			"where sequence_name in "
			"("
				"select column_name "
				"from user_tab_columns "
				"where table_name = :1)";

	if (oparse (cda, sql_seqs, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (cda, 1,
			(ub1 *) table, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		odefin (cda, 1,
			(ub1 *) name, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "Prep1ColumnCatalog");
	}

	/*
	 *	Get sequence name if any
	 */
	if (ofetch (cda))
		name [0] = '\0';
	else
		_strlower (name);
}

static void
Prep2ColumnCatalog (
 Lda_Def * lda,
 Cda_Def * cda,
 char * table,
 char * name,
 char * data_type,
 sb2 * i_data_type,
 int * data_length,
 sb2 * i_data_length,
 int * data_precision,
 sb2 * i_data_precision,
 int * data_scale,
 sb2 * i_data_scale)
{
	/*
	 *	Determine column info
	 */
	char * sql_cols =
		"select "
			"column_name, data_type, data_length, "
			"data_precision, data_scale "
		"from user_tab_columns "
		"where table_name = :1 "
		"order by column_id";

	if (oparse (cda, sql_cols, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (cda, 1,
			(ub1 *) table, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		odefin (cda, 1,
			(ub1 *) name, NAMEBUF_LEN, SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 2,
			(ub1 *) data_type, NAMEBUF_LEN, SQLT_STR,
			-1, i_data_type, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 3,
			(ub1 *) data_length, sizeof (int), SQLT_INT,
			-1, i_data_length, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 4,
			(ub1 *) data_precision, sizeof (int), SQLT_INT,
			-1, i_data_precision, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cda, 5,
			(ub1 *) data_scale, sizeof (int), SQLT_INT,
			-1, i_data_scale, NULL, -1, -1,
			&r_len, &r_code) ||
		oexec (cda))
	{
		oraclecda_error (lda, cda, "Prep1ColumnCatalog");
	}
}

/*
 *	List
 */
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
 const char * name,
 int colcount,
 int indexcount)
{
	TableInfoList * node = malloc (sizeof (TableInfoList));

	memset (node, 0, sizeof (TableInfoList));

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

static enum ColumnType
ORA2ColType (
 enum ORAType oratype)
{
	switch (oratype)
	{
	case OT_Chars:
		return CT_Chars;

	case OT_Date:
		return CT_Date;

	case OT_Serial:
		return CT_Serial;

	case OT_Number:
		return CT_Long;

	case OT_Money:
		return CT_Money;

	case OT_Double:
		return CT_Double;

	case OT_Float:
		return CT_Float;
	
	case OT_RowId:
		return CT_Serial;
	

	default:
		break;
	}
	oracledbif_error ("TableColumnInfo: bad conversion type");
	return CT_Bad;
}

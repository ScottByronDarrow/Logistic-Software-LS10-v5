#ident	"$Id: gen-access.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	General access functions
 *
 *******************************************************************************
 *	$Log: gen-access.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"db2io.h"
#include "samputil.h"
/*
#include	"oracledbif.h"
#include	"oraerrs.h"
*/
#include	"db2dbif.h"
/*
 *	Some magic numbers
 */
#define	NAMEBUF_LEN	32					/* ORACLE names are 30 chars long */
#define	MAXKEYS		8					/* We're limited by INFORMIX roots */
#define MAXLEN 129

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
static void AddTableInfoList (TableInfoList **, TableInfoList *), DestroyTableInfoList (TableInfoList **);
static TableInfoList * TableInfoListNode (const char *, int, int);
static TableInfoList * LocateTableInfoByNo (int);
static TableInfoList * LocateTableInfoByName (const char *);

int get_column_count( SQLHANDLE *, SQLCHAR *, SQLCHAR *);

static enum ColumnType DB2ColType (enum DB2Type);

SQLRETURN gen_a_list_columns( SQLHANDLE *, SQLCHAR *, SQLCHAR *, char *, char *, SQLINTEGER *, SQLINTEGER *, SQLSMALLINT * scale );

/*
 *	External functions
 */             
extern char * clip (char *);

/*
 *	Local variables
 */
/*static int tablecount = -1;*/
TableInfoList * catalog = NULL;

SQLRETURN rc;

/*
 *	Static areas for output
 */


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

int
_TableColumnCount (
 const char * name,
 SQLHANDLE * hstmt)
{
	TableInfoList * node = NULL;
	char * schema, tab_name[MAXLEN];

	TableState * table = LocateTable (name);

	if (table)
		return table -> columnc;

	/*
	 *	Check the catalog system files for a possible listing
	 */
	if ((node = LocateTableInfoByName (name)) != 0)
		return node -> info.ncolumn;
	else
	{
		schema = "LOGISTIC";
		_strupper (strcpy (tab_name, name));
		return get_column_count(hstmt, schema, tab_name);
	}

	/*oracledbif_error ("_TableColumnCount: No table %s", name);*/
	printf("_TableColumnCount: No table %s", name);
	return 0;
}

void
_TableColumnInfo (
 const char * name,
 int colno,
 struct ColumnInfo * buffer,
 SQLHANDLE * hdbc,
 SQLHANDLE * hstmt)
{
	
	TableState * table = LocateTable (name);

	if (table)
	{
		if (colno >= table -> columnc)
			/*oracledbif_error ("_TableColumnInfo: bad colno %d", colno);*/
			printf("_TableColumnInfo: bad colno %d", colno);

		/*
		 *	Convert ColumnDef to ColumnInfo
		 */
		strcpy (buffer -> name, table -> columns [colno].name);
		buffer -> size = table -> columns [colno].length;
		/*buffer -> type = ORA2ColType (table -> columns [colno].type);*/
		buffer -> type = DB2ColType (table -> columns [colno].type);

	} else
	{
		/*
		 *	Check offline catalog
		 */
		TableInfoList * node = LocateTableInfoByName (name);
		int count = 0;
		char tablename[MAXLEN];

		if (!node)
		{
			/*printf("_TableColumnCount: bad table %s", name);*/
			strcpy(tablename, name);
			node = TableInfoListNode (_strlower (clip (tablename)), count, 0);
			AddTableInfoList (&catalog, node);
		}
		if (!node -> columns)
		{
			/*
			 *	Build up the column info
			 */
			
			int i;
			unsigned sz;
			char col_data_type_upper[MAXLEN],
				col_name [MAXLEN],
				col_data_type [MAXLEN],
				tab_name[MAXLEN],
				* schema;
			SQLINTEGER length, col_buffer_length;
			SQLSMALLINT scale;
			SQLINTEGER  scale_ind = 0;
			static int	col_data_precision;
				
			
			
			schema = "LOGISTIC";
			_strupper (strcpy (tab_name, node -> info.name));
			node -> info.ncolumn = get_column_count(hstmt, schema, tab_name);
			rc = gen_a_list_columns(hstmt ,schema,tab_name, col_name, col_data_type, &length, &col_buffer_length, &scale);

			memset(col_data_type_upper,'\0',MAXLEN);
			memset(col_name,'\0',MAXLEN);
			memset(col_data_type,'\0',MAXLEN);

			/*
			 *	Allocate buffers and fill it up
			 */
			sz = node -> info.ncolumn * sizeof (struct ColumnInfo);
			memset (node -> columns = malloc (sz), 0, sz);

			for (i=0;SQLFetch(*hstmt) == SQL_SUCCESS;i++)
				{
					enum DB2Type type;
					int size;

					memset(node -> columns [i].name,'\0',32);
					_strlower (strncpy (node -> columns [i].name, col_name, strlen(col_name)));

					if (col_name !="")
					{
						_strupper (strcpy (col_data_type_upper,col_data_type));
					}

					_DecodeDB2Type (
						node -> columns [i].name, 
						col_data_type_upper, length, col_data_precision, scale,
						scale_ind,
						&type, &size);

					node -> columns [i].type = DB2ColType (type);

					node -> columns [i].size =
					node -> columns [i].type == CT_Chars ?
					size - 1 : 0;
				
				memset(col_name,'\0',MAXLEN);
				/* free statement resources */
				}
				rc = SQLFreeStmt(*hstmt, SQL_UNBIND ) ;
				CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;

				rc = SQLFreeStmt( *hstmt, SQL_RESET_PARAMS ) ;
				CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;

				rc = SQLFreeStmt( *hstmt, SQL_CLOSE ) ;
				CHECK_HANDLE( SQL_HANDLE_STMT,*hstmt, rc ) ;
		}
		if (colno >= node -> info.ncolumn)
			printf("TableColumnInfo: bad colno %d", colno);
		else 
			*buffer = node -> columns [colno];
	}
}

void
_TableColumnGet (
 const char * name,
 int colno,
 void * buffer)
{
	TableState * table = LocateTable (name);

#if 0
	if (!table)
		dbase_err ("_TableColumnGet", name, NOFILENAME);
	if (colno >= table -> columncount)
		dbase_err ("_TableColumnGet", name, NOFIELD);
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
		dbase_err ("_TableColumnGet", name, NOFILENAME);
#endif
	for (colno = 0; colno < table -> columnc; colno++)
	{
		if (!strcmp (colname, table -> columns [colno].name))
		{
			_TableColumnGet (name, colno, buffer);
			return;
		}
	}

	/*oracledbif_error ("No such column %s in %s", colname, name);*/
	printf("No such column %s in %s", colname, name);
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

SQLRETURN gen_a_list_columns( SQLHANDLE * misc_hstmt,
                        SQLCHAR * schema,
                        SQLCHAR * tablename,
						char * col_name,
						char * col_data_type,
						SQLINTEGER * length,
						SQLINTEGER * col_buffer_length,
						SQLSMALLINT * scale
                      ) {
	
	SQLINTEGER      length_ind;
	SQLINTEGER      scale_ind;
	SQLINTEGER      col_data_type_ind;
	SQLINTEGER		col_name_ind;
	SQLINTEGER		col_buffer_length_ind;

/*<-- */

	rc = SQLFreeStmt( *misc_hstmt, SQL_UNBIND ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_RESET_PARAMS ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_CLOSE ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

	/*This is where we bind the columns to variables*/

    rc = SQLColumns(*misc_hstmt, NULL, 0, schema, SQL_NTS,
                    tablename, SQL_NTS, (SQLCHAR *)"%", SQL_NTS);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 4, SQL_C_CHAR, (SQLPOINTER) col_name, MAXLEN,
                    &col_name_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 6, SQL_C_CHAR, (SQLPOINTER) col_data_type, MAXLEN,
                    &col_data_type_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 7, SQL_C_LONG, (SQLPOINTER) length,
                    sizeof(length), &length_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

	rc = SQLBindCol(*misc_hstmt, 8, SQL_C_LONG, (SQLPOINTER) col_buffer_length,
                    sizeof(col_buffer_length), &col_buffer_length_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 9, SQL_C_SHORT, (SQLPOINTER) scale,
                    sizeof(scale), &scale_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    return( SQL_SUCCESS ) ;

}


static enum ColumnType
DB2ColType (
 enum DB2Type db2type)
{
	switch (db2type)
	{
	case DB2_CHAR:
		return CT_Chars;

	case DB2_DATE:
		return CT_Date;

	case DB2_INTEGER:
		return CT_Long;

	case DB2_DECIMAL:
		return CT_Money;

	case DB2_FLOAT:
		return CT_Double;

	case DB2_REAL:
		return CT_Float;

	default:
		break;
	}
	/*acledbif_error ("_TableColumnInfo: bad conversion type");*/
	printf("_TableColumnInfo: bad conversion type");
	return CT_Bad;
}


int  get_column_count( SQLHANDLE * misc_hstmt,
                        SQLCHAR * schema,
                        SQLCHAR * tablename
                      ) {
/*<-- */
	int col_count = 0;
  
/* This is being done solely to count the columns in the table
	so there's no need to bind the columns, it will be done later*/	

	rc = SQLFreeStmt(*misc_hstmt, SQL_UNBIND );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLFreeStmt(*misc_hstmt, SQL_RESET_PARAMS );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLFreeStmt(*misc_hstmt, SQL_CLOSE );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLColumns(*misc_hstmt, NULL, 0, schema, SQL_NTS,
                    tablename, SQL_NTS, (SQLCHAR *)"%", SQL_NTS);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;
   
    
    /* Fetch each row, and display */
    
	while ((rc = SQLFetch(*misc_hstmt)) == SQL_SUCCESS) {
		col_count++;
	}	
	
                               /* endwhile */
/*<-- */
    
	/* free statement resources */

    rc = SQLFreeStmt( *misc_hstmt, SQL_UNBIND ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_RESET_PARAMS ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_CLOSE ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    return(col_count) ;

}  /* list_column_count */

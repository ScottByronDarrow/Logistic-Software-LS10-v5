#ifndef	_DB2DBIF_H
#define	_DB2DBIF_H
/*	$Id: db2dbif.h,v 5.0 2001/06/19 07:08:20 cha Exp $
 *
 *	Common trivia for DB2
 *
 *******************************************************************************
 *	$Log: db2dbif.h,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	
 *
 *
 *	
 */
#include	<stdarg.h>
#include	"db2io.h"
	
#include	<sqlcli1.h>


/*
 */
#define	TRUE	1
#define	FALSE	0

/* for the scrollable cursors implementation */
#define ROWSET_SIZE  1
extern SQLUINTEGER		numrowsfetched;
extern SQLUSMALLINT    row_status[ROWSET_SIZE];

/*
 *	Internal Column definitions
 */
enum DB2Type					/* represents DB2 type */
{
	DB2_UNKNOWN_TYPE,        
	DB2_CHAR,                
	DB2_NUMERIC,
	DB2_DECIMAL,            
	DB2_INTEGER,            
	DB2_SMALLINT,            
	DB2_FLOAT,           
	DB2_REAL,              
	DB2_DOUBLE,
	DB2_DATETIME,
	DB2_VARCHAR,
	DB2_DATE = 91
	
};

struct _ColumnDef
{
	char name [32];				/* DB2 uses 30 length on colnames */

	enum DB2Type type;
	unsigned length;
	SQLINTEGER null_ind;				/* NULL indicator for DB2 */

	char * data;				/* points into TableState data-buffer */

};
typedef	struct _ColumnDef ColumnDef;


/*
 *	Table state structure
 */
struct _TableState
{
	/*
	 *	I/F specific
	 */
	char *	named;				/* table/alias */
	char *	table;				/* real name, if alias */

	char *	index;				/* current index */

	int columnc;				/* total number of columns */
	ColumnDef * columns;		/* all column definitions and buffers */

	int viewc;					/* column count in view */
	struct dbview * view;		/* user view */
	int * viewactual;			/* maps from view to ColumnDef */

	char * indexname;			/* used in optimising queries */
	int indexc;					/* column count in index */
	int * indexactual;			/* index map to ColumnDef, NULL if no index */
	int * indexview;			/* index map to dbview */

	int datasz;
	char *	data;				/* data-buffer for current row */

	SQLHANDLE * hdbc;			/* One table one database connection handle */

	/*
	 *	Fetch mode
	 */
	int lastfetchmode;			/* used for optimising parse output */
	const char * lastlockmode;
	char * query;				/* current sql select statement */
	/*Cda_Def q_cursor;*/
	
	SQLHANDLE q_hstmt;			/* a query statement handle for every table */
	int q_flag;					/* indicates weather a statement handle is allocated or not */

	char * insert;
	SQLHANDLE i_hstmt;			/* an insert statement handle for every table */
	int i_flag;

	char * delete;
	/*
	 *	List management
	 */
	struct _TableState *	next;
};

/*
 *	Miscellanous structures
 */

struct _DB2_Decimal
{
	/*
	 *	3 byte DB2 date representation
	 */
	char precision, scale, sign;

};
typedef struct _DB2_Decimal DB2_Decimal;

typedef	struct _TableState	TableState;

#define MAX_STMT_LEN 255
#ifdef DB2WIN
   #define MAX_TABLES  1
#else
   #define MAX_TABLES 1
#endif


/* Global Table structure */
/*
typedef struct {
    SQLINTEGER schem_l;
    SQLCHAR    schem[129];
    SQLINTEGER name_l;
    SQLCHAR    name[129];
    SQLINTEGER type_l;
    SQLCHAR    type[129];
    SQLINTEGER remarks_l;
    SQLCHAR    remarks[255];

}   table_info;

table_info table[MAX_TABLES] ;
*/

/* table.c */
extern TableState * AllocateTable (SQLHANDLE * hdbc, const char *);
extern TableState * AllocateStdTable (SQLHANDLE *, const char *);
extern void DestroyTable (const char * name);
extern void DestroyAllTables ();
extern TableState * LocateTable (const char *);

/* alias.c */
extern const char * AliasBasename (const char * alias);
extern void DestroyAliasList (void);

/* colunms.c */
extern void IdentifyFields (
				SQLHANDLE *, SQLHANDLE *, char * tablename, TableState * table);

/* dbif.c BY BERICK*/
/*extern int _open_rec (char *tblName, struct dbview *fldList, int fldCount, char *keyField, int keyFlag);*/
/*extern int _open_rec2 (char *tblName, struct dbview *fldList, int fldCount, char *keyField, int keyFlag);*/
/*int _open_rec (char *, struct dbview *, int , char *, int);*/
/*extern int dbselfield (const char *filename, const char *idx_name, int flag); */
/*extern int dbselfield (const char *, const char *, int);*/
extern long abc_rowid (const char * filename);

extern int dbfind (const char *filename, int flag, const char *value, int *_length, char *recbuf);


/* delete.c */
extern int _DeleteRow (TableState *);

/* errors.c */
extern void	db2dbif_error (const char *, ...);
extern void	db2dbif_warn (const char *, ...);

/* indexes.c */
extern void IdentifyIndex (
				SQLHANDLE *, SQLHANDLE *, TableState *, const char *);

/* insert.c */
extern int _InsertRow (TableState *, void *);

/* query.c */
extern void QuerySetup (TableState *, int, int, void *, const char *);
extern int	QueryFetch (TableState *, void * buf, SQLSMALLINT);
extern int QueryCurrent (TableState *, void *);

extern int FetchAbsolute(TableState *, void *, int, SQLSMALLINT);
 
/* types.c */
extern int _application_align (int orig, enum DB2Type),
		   _application_size (enum DB2Type),
		   _internal_align (int orig, enum DB2Type),
		   _internal_size (enum DB2Type),
		   _internal_sqltype (enum DB2Type),
		   _external_sqltype (enum DB2Type); /* epf */
extern void _ConvApp2Raw (ColumnDef *, int, void *, const char *),
			_ConvRaw2App (ColumnDef *, int, void *);

extern int _CompRaw2App ( ColumnDef * , int , 
							 void * , int );
extern void _DecodeDB2Type (
				const char * col_name,
				const char * data_type,
				int data_length,
				int data_precision,
				int data_scale,
				SQLINTEGER i_data_scale,
				enum DB2Type * type,				/* output buffer */
				int * length);						/* output buffer */
/* gen-access.c */
extern int _TableColumnCount (const char *, SQLHANDLE *);
extern void _TableColumnInfo (const char *, int, struct ColumnInfo *, SQLHANDLE *, SQLHANDLE *);
extern void _TableColumnGet (const char *, int, void * );

/* update.c */
extern int _UpdateRow (TableState *, void *);

/* utils.c */
extern char * _strlower (char *), * _strupper (char *);

#endif	/* _DB2DBIF_H */



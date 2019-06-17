/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: ids9dbif.h,v 1.3 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (ids9dif.h)
|  Program Desc  : (Common file for Datablade API)
|---------------------------------------------------------------------|
| $Log: ids9dbif.h,v $
| Revision 1.3  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|
|
=====================================================================*/

#include	<stdarg.h>
#include 	<mi.h>
#include 	<dbio.h>
#include 	<ptypes.h>

#ifndef DBLADE_API
#define DBLADE_API
#endif

#define IDS_MAX_FIELDS 256;				/*Maximum fields that is allowed*/


MI_CONNECTION 		*_ids_conn;		/*descriptor for connection  - use in connecting to server*/
MI_CONNECTION_INFO 	_ids_conn_info;   	/*descriptor for connection info - use in getting default conn info*/
MI_DATABASE_INFO 	_ids_db_info;		/*descriptor for db info - use in log in*/

char *IDSTypes[] = {
	"bad",
	"char",
	"date",
	"serial",
	"money",
	"double",
	"float"	
	};
    	
enum IDSType						/* represents decoded data type */
{
	IDT_Bad,
	IDT_Chars,
	IDT_Date,
	IDT_Serial,
	IDT_Number,						/* usually integral representations */
	IDT_Money,
	IDT_Double,
	IDT_Float,
	IDT_RowId						/* used for updates */
};

struct _ColumnDef
{
	char 		name [128];			/*column name*/	
	int		data_type;			/*data type of column from syscolumns*/
	int	 	length;				/*length of column from syscolumns*/
	int		colno;				/*column number from syscolumns*/
	int	 	null_ind;			/* NULL indicator TRUE if null FALSE if has data  */
	char 		*data;				/* points into TableState data-buffer, data for each column */
};

typedef	struct _ColumnDef ColumnDef;

struct _RowLock
{
	char 	rowid [32];
	struct _RowLock * next;
};

typedef struct _RowLock RowLock;

typedef struct dbview DBVIEW;

struct _TableState
{
	/*--------------
	| I/F specific |
	--------------*/
	char		* named;				/* real name, if alias */
	char 		* table;				/* alias name */
	char		* index;				/* current index */
	int 		columnc;				/* total number of columns */
	long		tabid;					/* Unique ID from systables*/
	ColumnDef 	* columns;				/* all column definitions and buffers */
	int 		viewc;					/* column count in view */
	struct  	dbview * view;				/* user view */
	int 		* viewactual;				/* maps from view to ColumnDef */
	char 		indexname [255];				/* used in optimising queries */
	int 		indexc;					/* column count in index */
	int 		* indexactual;				/* index map to ColumnDef, NULL if no index */
	int 		* indexview;				/* index map to dbview */
	int 		datasz;
	char 		* data;					/* data-buffer for current row */
	char 		* orig_data;          			/* used in find_rec(..., PREVIOUS, ...) */
	int		stmt_processed;				/* IDS - use in determining if a query statement 
								   is prepared and parsed TRUE (if processed) 
								   or FALSE (otherwise)*/
	int 		gteq_called;				/*TRUE if GTEQ is called, other wise FALSE (Default)*/
	long 		rowid;
	long 		findhash;
	/*-------------------	
	|For Input Variables |
	--------------------*/
	MI_DATUM 	*values;				/*for values*/
	mi_integer 	*lengths;				/*lengths*/
	mi_integer 	*nulls;					/*null values , MI_TRUE, MI_FALSE*/
	mi_string 	**types;					/*data type of the input parameter*/
	
	/*------------
	| Fetch mode |
	------------*/
	int 		lastfetchmode;				/* used for optimising parse output */
	int 		lastactualfetchmode;    		/* NEXT? PREV? */
	char 		* query;				/* current sql select statement */
	MI_STATEMENT 	* q_stmt;

	/*--------------------------
	| Update (optional cursor) |
	--------------------------*/
	char 		* update;				/* current sql update statement */
	MI_STATEMENT 	* u_stmt;

	/*--------------------------
	| Insert (optional cursor) |
	--------------------------*/
	char 		* insert;
	MI_STATEMENT 	* i_stmt;

	/*--------------------------
	| Delete (optional cursor) |
	--------------------------*/
	char 		* delete;
	MI_STATEMENT 	* d_stmt;

	/*------------------
	| Active Row locks |
	------------------*/
	RowLock 	* locks;				/* List of row locks */

	/*---------------------------------------
	| table file descriptor for row locking |
	---------------------------------------*/
	int 		fd_table;

	/*-----------------
	| List management |
	-----------------*/
	struct 		_TableState * next;
};
typedef	struct _TableState TableState;



/*Structure for information about errors/exceptions encountered*/
typedef struct error_buf_
{
	mi_integer level;    	   	/* mi_error_level()  MI_EXCEPTION etc. */ 
	mi_integer sqlcode;        	/* mi_error_sqlcode()                  */
	mi_string  sqlstate[6];    	/* mi_error_sql_state()                */
	mi_string  error_msg[256]; 	/* mi_errmsg()                         */
} DB_ERROR_BUF;

/*use for information gathering about errors encountered*/
DB_ERROR_BUF	_ids_error;

/*STD C Functions */
extern char  *getenv(const char *);
extern void DateToDMY (Date, int *, int *, int *);

/*error.c*/

/*call back function for error gathering*/

/*displaying of errors encountered*/
extern void ids_dbase_err 	(void);
extern void ids_dbase_err2	(char *);

/*========+
| table.c |
+=========*/

extern TableState * AllocateStdTable (const char *);
extern TableState * AllocateTable (const char *);
extern void DestroyTable (const char * name);
extern void DestroyAllTables ();
extern TableState * LocateTable (const char *);


/*==========+
| alias.c |
+==========*/

extern const char * AliasBasename (const char * alias);
extern void DestroyAliasList (void);

/*==========+
| column.c  |
+==========*/

extern int IdentifyFields (TableState *);


/*==========+
| index.c   |
+==========*/

extern int IdentifyIndex (TableState *, const char *);


/*external LS/10 functions*/
extern char *PNAME; 
extern void sys_err ( const char *text, int value, const char *prg_name);


/*==========+
| utils.c   |
+==========*/

extern char * GetColNameByColNo (TableState *, int);
extern char * PadOut (char *, int);
extern char  *clip (char *);
char *  DMY2YMD (Date);
/*==========+
| locks.c   |
+==========*/
extern void _LockInitialise();
extern void _LockCleanup();
extern int  _TryLock( char locktype, TableState *tablestate, RowLock ** );
extern int  _LockFreeAll (TableState *tablestate, RowLock **);

/*==========+
| query.c   |
+==========*/
int QuerySetup (TableState *, const int , void *);
int QueryFetch (TableState * ,void *, char, int);


/*=========+
| data.c   |
+==========*/
void GetDataApp (ColumnDef * ,int, void *);
int ConvSQL2C (int);
mi_string * GetColDataType (ColumnDef *);
void GetDataDB(	ColumnDef *,int , mi_string*, void *);
void MapDataBuffer ( TableState *, void * );

/*=========+
| insert.c |
+==========*/
int _InsertRow (TableState *, void *);


/*=========+
| update.c |
+==========*/
int _UpdateRow ( TableState *, void *);

/*=========+
| delete.c |
+==========*/
int _DeleteRow  (TableState *);



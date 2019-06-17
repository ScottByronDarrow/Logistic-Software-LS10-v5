#ifndef	_ORACLEDBIF_H
#define	_ORACLEDBIF_H
/*	$Id: oracledbif.h,v 5.0 2001/06/19 07:10:28 cha Exp $
 *
 *	Common trivia for Oracle
 *
 *******************************************************************************
 *	$Log: oracledbif.h,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:53  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.2  2000/08/02 02:34:59  raymund
 *	Small performance improvements. Added codes for locked find_hash()es.
 *	
 *	Revision 2.1  2000/07/26 10:09:56  raymund
 *	Furnished missing functionalities. Use SQL for row locking.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.7  2000/07/13 11:08:45  raymund
 *	16-bit reversed CRC hardware emulation algorithm for row locking.
 *	
 *	Revision 1.6  2000/07/13 06:43:51  gerry
 *	Linked error handling to standard LS/10 error handling, fixed alias bug
 *
 *	Revision 1.5  1999/11/16 02:40:36  jonc
 *	Extended rowid for locks - needs to be pretty big...
 *	
 *	Revision 1.4  1999/11/15 02:53:06  jonc
 *	Added lock code. Requires `alvin' the lock-daemon to be running.
 *	
 *	Revision 1.3  1999/11/01 21:23:31  jonc
 *	Added support for FIRST.
 *	
 *	Revision 1.2  1999/10/28 01:58:36  jonc
 *	Added support for generic-catalog access. Damnably slow, though.
 *	
 *	Revision 1.1  1999/10/21 21:47:05  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdarg.h>

#include	"oratypes.h"
#include	"ociapr.h"

/*
 */
#define	TRUE	1
#define	FALSE	0

#define	PARSE_NOW		0		/* immediate parse */
#define	PARSE_DEFER		1		/* defer on oparse() */
#define	PARSE_V7		2		/* v7 specific on oparse() */

/*
 *	Internal Column definitions
 */
enum ORAType					/* represents decoded ORACLE type */
{
	OT_Bad,
	OT_Chars,
	OT_Date,
	OT_Serial,
	OT_Number,					/* usually integral representations */
	OT_Money,
	OT_Double,
	OT_Float,
	OT_RowId					/* used for updates */
};

struct _ColumnDef
{
	char name [32];				/* ORACLE uses 30 length on colnames */

	enum ORAType type;
	unsigned length;
	sb2 null_ind;				/* NULL indicator for ORACLE */

	char * data;				/* points into TableState data-buffer */

};
typedef	struct _ColumnDef ColumnDef;

/*
 *	Lock structures
 */
struct _RowLock
{
	char rowid [32];
	struct _RowLock * next;
};
typedef struct _RowLock RowLock;

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
	char *  orig_data;          /* used in find_rec(..., PREVIOUS, ...) */

	/*
	 *	Fetch mode
	 */
	int lastfetchmode;			/* used for optimising parse output */
	int lastactualfetchmode;    /* NEXT? PREV? */
	char * query;				/* current sql select statement */
	Cda_Def q_cursor;

	/*
	 *	Update (optional cursor)
	 */
	char * update;				/* current sql update statement */
	Cda_Def u_cursor;

	/*
	 *	Insert (optional cursor)
	 */
	char * insert;
	Cda_Def i_cursor;

	/*
	 *	Delete (optional cursor)
	 */
	char * delete;
	Cda_Def d_cursor;

	/*
	 *	Active Row locks
	 */
	RowLock * locks;			/* List of row locks */

	/*
	 *	table file descriptor for row locking
	 */
	 int fd_table;

	/*
	 *	List management
	 */
	struct _TableState *	next;
};

typedef	struct _TableState	TableState;

/*
 *	Miscellanous structures
 */
struct _ORA_Date
{
	/*
	 *	7 byte ORACLE date representation
	 */
	char century, year, month, day, hour, minute, second;
};
typedef struct _ORA_Date ORA_Date;

/*
 *	Prototypes for internal functions
 */
/* dbif.c */
extern Lda_Def *_LDA ();	/* against my better judgement */

/* table.c */
extern TableState * AllocateTable (Lda_Def * lda, const char *);
extern TableState * AllocateStdTable (Lda_Def *, const char *);
extern void DestroyTable (const char * name);
extern void DestroyAllTables ();
extern TableState * LocateTable (const char *);

/* error.c */
extern void sys_err ( const char *text, int value, const char *prg_name);
extern void oracledatabase_err( const char *, int);
extern void	oracledbif_error (const char *, ...);
extern void	oracledbif_warn (const char *, ...);
extern void	oraclecda_error (Lda_Def *, Cda_Def *, const char *, ...);

/* alias.c */
extern const char * AliasBasename (const char * alias);
extern void DestroyAliasList (void);

/* colunms.c */
extern void IdentifyFields (
				Lda_Def * lda,
				Cda_Def * cols, Cda_Def * colc, Cda_Def * seqs,
				TableState * table);

/* delete.c */
extern int _DeleteRow (Lda_Def *, TableState *);

/* indexes.c */
extern void IdentifyIndex (
				Lda_Def * lda,
				Cda_Def * inds, Cda_Def * indc,
				TableState *, const char *);

/* insert.c */
extern int _InsertRow (Lda_Def *, TableState *, void *);

/* locks.c */
extern void _LockInitialise(),
            _LockCleanup();
extern int _TryLock( char locktype, TableState *tablestate, RowLock ** );
extern int _LockFreeAll (TableState *tablestate, RowLock **);

/* query.c */
extern int  QuerySetup (Lda_Def *, TableState *, int, int, void *);
extern int	QueryFetch (Lda_Def *, TableState *, void * buf, char);
extern int  QueryFetchPrevious( Lda_Def *, TableState *, void *, int, char);


/* types.c */
extern int _application_align (int orig, enum ORAType),
		   _application_size (enum ORAType),
		   _internal_align (int orig, enum ORAType),
		   _internal_size (enum ORAType),
		   _internal_sqltype (enum ORAType);
extern void _ConvApp2Raw (ColumnDef *, int, void *),
			_ConvRaw2App (ColumnDef *, int, void *);
extern void _DecodeORAType (
				const char * col_name,
				const char * seq_name,
				const char * data_type,
				int data_length,
				int data_precision,
				int data_scale,
				sb2 i_data_precision,
				sb2 i_data_scale,

				enum ORAType * type,				/* output buffer */
				int * length);						/* output buffer */

/* update.c */
extern int _UpdateRow (Lda_Def *, TableState *, void *);

/* utils.c */
extern char * _strlower (char *), * _strupper (char *);

#endif	/* _ORACLEDBIF_H */

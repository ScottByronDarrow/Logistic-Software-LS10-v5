#ifndef	DBIO_H
#define	DBIO_H
/*
 *	General database interface def's
 *
 *******************************************************************************
 *	$Log: db2io.h,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	Revision 1.5  1999/10/26 22:49:17  jonc
 *	Added system-catalog interface.
 *	
 */
#include	<decimal.h>
/*#include	<isam.h>*/
#include	<sqlcli1.h>

#include	"sqlca.h"

#define DBOPEN     1
#define DBCLOSE    2
#define FILEOPEN   3
#define RELOPEN    3
#define FILECLOSE  4
#define RELCLOSE   4

#define OPENMASK	0x00ff
#define EXCLUSIVE	0x0100
#define READONLY	0x0200
#define READWRITE	0x0000

/* values passed to dbselfield() */
#define ACCKEYED	0
#define ACCPRIMARY	1
#define ACCSEQUENTIAL	2

struct dbview
{
	char	*vwname;	/* name of attribute */
	int		vwstart,	/* offset in view */
			vwtype,		/* type of attribute */
			vwlen;		/* length in bytes */
};

#define COMPARISON	1
#define FIRST		2
#define LAST		3
#define NEXT		4
#define PREVIOUS	5
#define EQUAL		6
#define GTEQ		7
#define GREATER		8
#define CURRENT		9
#define	LT			10
#define	LTEQ		11
#define LOCK		0400

#define SERIALTYPE	(0x0100 + LONGTYPE)
#define SERIALSIZE	LONGSIZE
#define DATETYPE	(0x0200 + LONGTYPE)
#define EDATETYPE	(0x0300 + LONGTYPE)
#define YDATETYPE	(0x0400 + LONGTYPE)
#define DATESIZE	LONGSIZE
#define MONEYTYPE	(0x0500 + DOUBLETYPE)
#define MONEYSIZE	DOUBLESIZE
#define TIMETYPE	(0x0600 + LONGTYPE)
#define TIMESIZE	LONGSIZE
#define COMPOSTYPE	(-1)

extern double round(double);

#define	CENTS(d)	(round((d)*100.0))
#define DOLLARS(c)	((c)/100.0)
#define ROUND(c)	(round(c))

struct	audinfo
{
	char	aud_name[128];
	long	rec_size;
};

/*
 *	Support types and structures
 *	for querying info about a table in a neutral manner
 */

/*
enum ColumnType
{
	CT_UNKNOWN_TYPE,        

	CT_CHAR,                

	CT_NUMERIC,

	CT_DECIMAL,            

	CT_INTEGER,            

	CT_SMALLINT,           

	CT_FLOAT,           

	CT_REAL,            

	CT_DOUBLE,

	CT_DATETIME,

	CT_VARCHAR
};
*/

enum ColumnType
{
	CT_Bad,
	CT_Chars,
	CT_Long,
	CT_Short,
	CT_Float,
	CT_Double,
	CT_Serial,
	CT_Date,
	CT_Money,
	CT_Number
};

struct TableInfo
{
	/*
	 *	Structure expected to be used with
	 *		- TableInfo ()
	 */
	char name [32];					/* table name */
	int ncolumn;					/* number of columns */
	int nindexes;					/* number of indexes */
};

struct IndexInfo
{
	/*
	 *	Struct expected to be used with
	 *		- TableIndexInfo ()
	 */
	char name [32];					/* name of index */
	int isunique;
	int ncolumn;					/* number of columns in index */
};

struct ColumnInfo
{
	char			name [19];
	enum ColumnType	type;
	int				size;
};

#endif	/*DBIO_H*/
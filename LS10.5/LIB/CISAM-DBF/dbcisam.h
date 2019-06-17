#ifndef	DBCISAM_H
#define	DBCISAM_H

#include	<stdio.h>
#include	<string.h>

#include	<osdefs.h>
#include	<dbio.h>

#define	SYSTAB_TABNAME		0
#define	SYSTAB_DIRPATH		26
#define	SYSTAB_TABID		90
#define	SYSTAB_ROWSIZE		94
#define	SYSTAB_NCOLS		96
#define	SYSTAB_NINDEX		98
#define	SYSTAB_TABTYPE		112

#define PERM_READ	0x0001
#define PERM_UPDATE	0x0002
#define PERM_INSERT	0x0004
#define PERM_DELETE	0x0008
#define PERM_ATAB	0x0010
#define PERM_IDX	0x0020
#define PERM_ALL	0x003f

enum LockMode
{
	NoLock,
	RdLock,
	WrLock
};

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif	/*TRUE*/

#define	NAME_LEN	18

#define	MAX_SYS		3

#define	SYS_TAB		0
#define	SYS_COL		1
#define	SYS_IDX		2

/*
 *	Column offsets for system tables
 */
#define	SYSCOL_COLNAME		0
#define	SYSCOL_TABID		18
#define	SYSCOL_COLNO		22
#define	SYSCOL_COLTYPE		24
#define	SYSCOL_COLLENGTH	26

#define	SYSIDX_IDXNAME		0
#define	SYSIDX_TABID		26
#define	SYSIDX_UNIQUE		30
#define	SYSIDX_PART(x)		(32 + (x * 2))

/*
 *	Brain-dead macros
 */
#define	TABID		tptr->_tabid
#define	KEYDEF		tptr->_key
#define	KEYPART(x)	tptr->_key.k_part[x]

/*===============================================
| structure for llist ( cisam file attributes )	|
===============================================*/
struct _llist
{
	int	_fd;						/* fd of isam if stay open	*/
	int	_open_type;					/* open type for the cisam	*/
	int	_no_fields;					/* no_fields in view		*/
	int	_buf_size;					/* size of buffer(s)		*/
	int	*_start;					/* pointer to array of int	*/
	int	_serial;					/* offset of serial field (-1)	*/
	char	*_filename;				/* name of file			*/
	char	*_dirpath;				/* full name of file		*/
	char	*_audpath;				/* full name of audit file	*/
	int	_aud_fd;					/* fd of audit file		*/
									/* -1 = disabled		*/
									/* -2 = 'Virtually' closed	*/
	long	_recno;					/* Current record.		*/
	char	*_buffer;				/* buffer for reading etc	*/
	char	*_init;					/* init strings for dbadd	*/
	long	_tabid;					/* table id for file - sql	*/
	struct dbview	*_view;			/* pointer to view for file	*/
	struct keydesc	_key;			/* indexing structure		*/
	int	columncount;
	struct	ColumnInfo	*columnlist;
	int	*columncisam;				/* cisam offsets */

	enum LockMode lockmode;			/* lock info */

	int	timestamp;					/* >= 0 if timestamp column is present */

	struct	_llist	*_next;		/* next node			*/
};
typedef	struct	_llist		LLIST;

/*
 *	Globals (puke!)
 */
extern char	*_dbpath;		/*dbutils.c*/

/*
 *	for library internal usage only
 */
#include	"prototypes.h"

#endif	/*DBCISAM_H*/

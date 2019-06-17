/*******************************************************************************

	DB/Interface routines

*******************************************************************************/
#include	<VerNo.h>
#ifndef	lint
static char	*rcsid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/dbif.c,v 5.2 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<stdio.h>
#include	<sqlca.h>

#include	"dict.h"
#include	"isamdbio.h"
#include	"stddefs.h"
#include	"tblnode.h"
#include	"fnproto.h"

/**	External refs
**/
extern char		*PNAME;

/*==================================================================
| This Routine Opens A file , Sets The View and Key off the file . |
==================================================================*/
open_rec (tblName, fldList, fldCount, keyField)
char	*tblName;
struct	dbview	*fldList;
int	fldCount;
char	*keyField;
{
	return (_open_rec (tblName,
		fldList, fldCount,
		keyField, keyField && *keyField ? ACCKEYED : ACCSEQUENTIAL));
}

_open_rec (tblName, fldList, fldCount, keyField, keyFlag)
char	*tblName;
struct	dbview	*fldList;
int	fldCount;
char	*keyField;
int	keyFlag;
{
	int	cc;

	if (cc = MakeView (tblName, fldList, fldCount))
	{
		dbase_err ("_open_rec()", tblName, cc);
		return (cc);
	}

	abc_fopen (tblName);

	if (keyFlag == ACCKEYED || keyFlag == ACCPRIMARY && (keyField && *keyField))
	{
		if (cc = abc_selfield (tblName, keyField))
			dbase_err ("_open_rec()", tblName, cc);
	}
	return (cc);
}

/*========================
| Open  Database  Files. |
========================*/
abc_fopen (tblName)
char	*tblName;
{
	/**	All we really need to do is to make sure it exists
		within the table list
	**/
	if (!GetTableNode (tblName))
	{
		dbase_err ("abc_fopen()", tblName, -1);
		return (-1);
	}
	return (EXIT_SUCCESS);
}

/*=========================
| Close data base files . |
=========================*/
abc_fclose (tblName)
char	*tblName;
{
	TableNode	*tNode = GetTableNode (tblName);

	if (!tNode || !DetTableNode (tNode))
		return (-1);
	return (EXIT_SUCCESS);
}

/*===========================================
| O p e n   D a t a b a s e.                |
===========================================*/
abc_dbopen (dbName)
char	*dbName;
{
	int	cc = OpenDb (dbName);

	if (cc)
		dbase_err ("abc_dbopen()", dbName, cc);
	return (cc);
}

/*==========================================================
| C l o s e  D a t a b a s e.                              |
==========================================================*/
abc_dbclose (dbName)
 char	*dbName;
{
	return (CloseDb (dbName));
}

/*=========================================
| dbselfield Routine With error checking. |
=========================================*/
abc_selfield (tblName, fldName)
char	*tblName, *fldName;
{
	int	cc = IndexOn (tblName, fldName);

	if (cc)
		dbase_err ("abc_selfield()", tblName, cc);
	return (cc);
}

long	abc_rowid (tblName)
char	*tblName;
{
	TableNode	*node = GetTableNode (tblName);

	if (!node)
	{
		dbase_err ("abc_rowid()", tblName, -1);
		return (-1);
	}
	return (node -> rowid);
}

/*===========================
| Add 'addRec' to 'tblName' |
============================*/
abc_add (tblName, addRec)
char	*tblName;
char	*addRec;
{
	return (InsertRec (tblName, addRec));
}

/*==================================================
| Update current record of 'tblName' with 'updRec' |
==================================================*/
abc_update (tblName, updRec)
char	*tblName, *updRec;
{
	int	cc = UpdateRec (tblName, updRec);

	if (!cc)
		cc = abc_unlock (tblName);
	return (cc);
}

/*===================================
| Delete current record of 'tblName'
====================================*/
abc_delete (tblName)
char	*tblName;
{
	int	cc = DeleteRec (tblName);

	if (!cc)
		cc = abc_unlock (tblName);
	return (cc);
}

/*=====================
| Create alternate name
=====================*/
abc_alias (altName, origName)
char	*altName, *origName;
{
	return (AddEntry (altName, origName) ? 0 : -1);
}

/*====================================================
| Unlocks all individually locked records of 'tblName'
=====================================================*/
abc_unlock (tblName)
char	*tblName;
{
	return (UnlockRecs (tblName));
}

/*======================
| Lock table 'tblName' |
=======================*/
abc_flock (tblName)
char	*tblName;
{
	return (LockTbl (tblName));
}

/*=================================
| Remove table lock from 'tblName'
==================================*/
abc_funlock (tblName)
char	*tblName;
{
	return (UnlockTbl (tblName));
}

/*========================================================================
	lockType may be :

	"r":   Read only - so no lock is requested
	"w":   Write - lock is requested with an abort option if locked
	"u":   Update - lock is requested and keeps trying till available.
=========================================================================*/
static tryLock (module, table, lockType)
char	*module, *table;
char	lockType;
{
	switch (lockType)
	{
	case 'r'	:
		break;				/* no lock requested	*/

	case 'u'	:
		LockRec (table, TRUE);		/* indefinite wait	*/
		break;

	case 'w'	:
		if (!LockRecWithAbort (table))
			return (-1);
		break;

	default		:
		dbase_err (module, table, -1);	/* gonna !force! this	*/
		return (-1);			/*NOTREACHED*/
	}
	return (EXIT_SUCCESS);				/* all is well		*/
}

/*========================================================================
	The function find_rec should be called with 4 parameters:
	the file name, the record to be found (key presumed already set up),
	comparison operator, and a single character string which can be:

	"r":   Read only - so no lock is requested
	"w":   Write - lock is requested with an abort option if locked
	"u":   Update - lock is requested and keeps trying till available.
=========================================================================*/
find_rec (tbl, buf, ftype, rw)
char	*tbl;
char	*buf;	/*	has key info & receives the record	*/
int	ftype;	/*	COMPARISON, EQUAL, GTEQ, ...	*/
char	*rw;	/*	Lock type	*/
{
	int	errc = GetRecord (tbl, buf, ftype);

	if (errc == SQLNOTFOUND)
		return (EXIT_FAILURE);

	return (!errc ? tryLock ("find_rec()", tbl, *rw) : errc);
}

/*=========================================================================
	The function find_hash should be called with 5 parameters:
	the table name, the record to be found (key presumed already set up)
	the comparison operator, a single character string which can be:

	"r":   Read only - so no lock is requested
	"w":   Write - lock is requested with an abort option if locked
	"u":   Update - lock is requested and keeps trying till available.

	and the hash
=========================================================================*/
find_hash (tbl, buf, ftype, rw, hash)
char	*tbl;
char	*buf;		/*	receives the record	*/
int	ftype;
char	*rw;
long	hash;
{
	int	errc;

	/**	Check whether current index is 
		1. hash_type (ie long)
		2. ain't any, in which case hash == rowid match
	**/
	if (!ValidHashIdx (tbl))
	{
		dbase_err ("find_hash () on invalid index", tbl, -1);
		return (-1);
	}

	/**	Get the rec
	**/
	if ((errc = GetRecordOnLong (tbl, buf, ftype, hash)) == SQLNOTFOUND)
		return (EXIT_FAILURE);

	return (!errc ? tryLock ("find_hash()", tbl, *rw) : errc);
}

void	dbase_err (callName, tableName, errCode)
char	*callName;
char	*tableName;
int	errCode;
{
	char	err_str [256];

	sprintf (err_str, "Error [%s] table '%s'", callName, tableName);
	sys_err (err_str, errCode, PNAME);
}

foreground ()
{
	/**
	This Will Test if program is in Foreground.

	There's a Secondary Test to see if Parent Who Was in Foreground
	Has Died Which Makes this process in background but isatty () still
	thinks it is in foreground
	**/
	return (isatty (0) ? getppid () != 1 : FALSE);
}

for_chk ()	/*	for back-compatiblity	*/
{
	return (!foreground ());
}

/*====================
| Begin Work
====================*/
begin_work ()
{
	return (_begin_work ());
}

/*====================
| Commit Work
====================*/
commit_work ()
{
	return (_commit_work ());
}

/*======================
| Rollback Work
======================*/
rollback_work ()
{
	return (_rollback_work ());
}

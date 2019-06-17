/*******************************************************************************

	Updates to database ie : inserts, deletions, updates

*******************************************************************************/
#include	<malloc.h>
#include	<string.h>
#include	<stdio.h>

#include	"isamdbio.h"
#include	"sqlerrs.h"
#include	"tblnode.h"
#include	"fnproto.h"

$include	sqlca;
$include	sqltypes;

/**	Magic numbers
**/
#define	OVERFLOW	30U

/*
 *	Local functions
 */
static char	*ZeroStr (short);

/**
**/
InsertRec (
 char	*tblName,
 char	*addRec)
{
	int			i, sz;
	$char		*insertStr;
	KeyField	*fld;
	TableNode	*tbl = GetTableNode (tblName);
	struct sqlda	*prms;

	if (!tbl)
		return (-1);

	/**	Set up receiving areas in the sqlda structure
	**/
	for (i = 0; i < tbl -> fldCount; i++)
		tbl -> data.sqlvar [i].sqldata = addRec + tbl -> fields [i].vwstart;

	/**
		insert into
		'tblName' ('col1', 'col2', ...)
		values (val1, val2, ...)
	**/
	/*	make a wild guess about string size
	*/
	sz = strlen (tbl -> table) + 2;		/* include spacing */
	for (i = 0; i < tbl -> fldCount; i++)
		sz += strlen (tbl -> fields [i].vwname)
			+ 4					/* for ", "'s */
			+ 4;				/* for "? "'s */
	for (fld = tbl -> unused; fld; fld = fld -> next)
		sz += strlen (fld -> name) + 6;			/* + spacing */

	if (!(insertStr = malloc (sz + OVERFLOW)))
		return (-1);

	/*	construct statement
	*/
	/*	list columns	*/
	strcat (strcat (strcpy (insertStr, "insert into "), tbl -> table), " (");
	for (i = 0; i < tbl -> fldCount; i++)		/* used fields	*/
	{
		strcat (insertStr, tbl -> fields [i].vwname);
		if (i + 1 < tbl -> fldCount)
			strcat (insertStr, ", ");
	}
	for (fld = tbl -> unused; fld; fld = fld -> next)/* unsused flds */
		strcat (strcat (insertStr, ", "), fld -> name);

	/*	build up values */
	strcat (insertStr, ") values (");

	for (i = 0; i < tbl -> fldCount; i++)		/* used fields */
	{
		strcat (insertStr, "?");
		if (i + 1 < tbl -> fldCount)
			strcat (insertStr, ", ");

		if ((tbl -> fields [i].vwtype & SQLTYPE) == SQLSERIAL)
		{
			/*	SERIAL field have to inserted with a 0 value so that the system
				generated number is used
			*/
			*(long *) (tbl -> data.sqlvar [i].sqldata) = 0;
		}
		BuildParam (
			tbl -> updParams.sqlvar + i,
			tbl -> data.sqlvar + i);
	}
	for (fld = tbl -> unused; fld; fld = fld -> next)/* unsused fields */
		strcat (strcat (insertStr, ", "), ZeroStr (fld -> colType));

	strcat (insertStr, ")");

	/*	run it
	*/
	tbl -> updParams.sqld = tbl -> fldCount;

	DataFudgeIn (tbl, &tbl -> updParams);
	prms = &tbl -> updParams;
	$prepare insertStmt from $insertStr;
	if (!sqlca.sqlcode)
		$execute insertStmt using descriptor prms;
	DataFudgeOut (tbl, &tbl -> updParams);

	/*	update rowid	*/
	tbl -> rowid = sqlca.sqlcode ? 0 : sqlca.sqlerrd [5];

	free (insertStr);
	return ((int) sqlca.sqlcode);
}

UpdateRec (
 char	*tblName,
 char	*updRec)
{
	int			i, j, sz;
	$char		*updateStr;
	KeyField	*fld;
	TableNode	*tbl = GetTableNode (tblName);
	struct sqlda	*prms;

	if (!tbl)
		return (-1);

	/**	Set up receiving areas in the sqlda structure
	**/
	for (i = 0; i < tbl -> fldCount; i++)
		tbl -> data.sqlvar [i].sqldata = updRec + tbl -> fields [i].vwstart;

	/*
		$update 'tblName'
			set col = val, col = val, ...
		where rowid = 'rowid'
	*/
	/*	make a wild guess about string size
	*/
	sz = strlen (tbl -> table) + 2;			/* include spacing */
	for (i = 0; i < tbl -> fldCount; i++)
		sz += strlen (tbl -> fields [i].vwname)
			+ 4				/* for ", "'s	*/
			+ 4;			/* values "?" */
	for (fld = tbl -> unused; fld; fld = fld -> next)
		sz += strlen (fld -> name) + 6;		/* + spacing + val */

	if (!(updateStr = malloc (sz + OVERFLOW)))
		return (-1);

	/*	Construct the statement
	*/
	strcat (strcat (strcpy (updateStr, "update "), tbl -> table), " set ");
	for (i = j = 0; i < tbl -> fldCount; i++)
	{
		/*	Skip SERIAL fields (Informix doesn't allow
			updates on these)
		*/
		if ((tbl -> fields [i].vwtype & SQLTYPE) == SQLSERIAL)
			continue;

		strcat (strcat (updateStr, tbl -> data.sqlvar [i].sqlname), " = ?");
		if (i + 1 < tbl -> fldCount)
			strcat (updateStr, ", ");

		BuildParam (
			tbl -> updParams.sqlvar + j++,
			tbl -> data.sqlvar + i);
	}
	sprintf (updateStr + strlen (updateStr),
		" where rowid = %ld", tbl -> rowid);

	/*	run it
	*/
	tbl -> updParams.sqld = j;

	DataFudgeIn (tbl, &tbl -> updParams);
	prms = &tbl -> updParams;
	$prepare updateStmt from $updateStr;
	if (!sqlca.sqlcode)
		$execute updateStmt using descriptor prms;
	DataFudgeOut (tbl, &tbl -> updParams);

	free (updateStr);
	return ((int) sqlca.sqlcode);
}

DeleteRec (
 char	*tblName)
{
	$char		*deleteStr;
	TableNode	*tbl = GetTableNode (tblName);

	if (!tbl)
		return (-1);

	/*
		$delete from 'tblName' where rowid = 'rowid'
	*/
	if (!(deleteStr = malloc (strlen (tbl -> table) + OVERFLOW + 10)))
		return (-1);

	sprintf (deleteStr, "delete from %s where rowid = %ld",
		tbl -> table, tbl -> rowid);

	$execute immediate $deleteStr;
	free (deleteStr);

	if (!sqlca.sqlcode)
		tbl -> rowid = 0;

	return ((int) sqlca.sqlcode);
}

static char	*ZeroStr (
 short	colType)
{
	switch (colType & SQLTYPE)
	{
	case SQLCHAR	:
		return ("''");

	case SQLSMINT	:
	case SQLINT		:
	case SQLFLOAT	:
	case SQLSMFLOAT	:
	case SQLDECIMAL	:
	case SQLSERIAL	:
	case SQLMONEY	:
		return ("0");

	case SQLDATE	:
	case SQLDTIME	:
		return ("null");

	case SQLBYTES	:
	case SQLTEXT	:
	case SQLVCHAR	:
	case SQLINTERVAL	:
		return ("0");
	}
	return ("BadValue");
}

/*====================
| Begin Work
====================*/
_begin_work ()
{
	EXEC SQL BEGIN WORK;
	return ((int) sqlca.sqlcode);
}

/*====================
| Commit Work
====================*/
_commit_work ()
{
	EXEC SQL COMMIT WORK;
	return ((int) sqlca.sqlcode);
}

/*======================
| Rollback Work
======================*/
_rollback_work ()
{
	EXEC SQL ROLLBACK WORK;
	return ((int) sqlca.sqlcode);
}

/*******************************************************************************

	Extraction routine interface

*******************************************************************************/
#include	<stdio.h>
#include	<malloc.h>
#include	<string.h>
#include	<sqlca.h>
#include	<sqltypes.h>

#include	"isamdbio.h"
#include	"stddefs.h"
#include	"tblnode.h"
#include	"utils.h"
#include	"fnproto.h"

/**	Forward decls
**/
static int	IsComparator (int),
			IsAbsPosition (int);
static int	SetFirstRec (int);
static int	InitCursor (TableNode *, int, long);
static int	FetchRec (int, TableNode *);

static char	*StrComparator (int, int);

/**
**/
GetRecord (
 char	*tblName,
 char	*buf,
 int	ftype)		/*	comparison type	*/
{
	int		i;
	TableNode	*tN = GetTableNode (tblName);

	if (!tN)
		return (-1);

	/**	Set up receiving areas in the sqlda structure
	**/
	for (i = 0; i < tN -> fldCount; i++)
		tN -> data.sqlvar [i].sqldata = buf + tN -> fields [i].vwstart;

	/*	Setup cursor if required
	*/
	if (tN -> curse == CURSE_Null ||
		IsComparator (ftype) ||
		IsAbsPosition (ftype))
	{
		int	errc = InitCursor (tN, ftype, 0L);

		if (errc)
		{
			return (errc);
		}

		if (IsComparator (ftype))
			ftype = SetFirstRec (ftype);
	}
	return (FetchRec (ftype, tN));
}

GetRecordOnLong (
 char	*tblName,
 char	*buf,
 int	ftype,
 long	match)
{
	int		i, errc;
	TableNode	*tN = GetTableNode (tblName);

	if (!tN)
		return (-1);

	/**	Set up receiving areas in the sqlda structure
	**/
	for (i = 0; i < tN -> fldCount; i++)
		tN -> data.sqlvar [i].sqldata = buf + tN -> fields [i].vwstart;

	if (IsComparator (ftype))
	{
		/**	if keys exist, then need to prime it, otherwise
			match is a rowid (which the BuildStatement will
			handle)
		**/
		if (tN -> keys)
		{
			/*	Prime the buffer with the 'match' value
			*/
			for (i = 0; i < tN -> fldCount; i++)
				if (!strcmp (tN -> keys -> name, tN -> fields [i].vwname))
				{
					/*	Prime the index	*/
					*(long *) (buf + tN -> fields [i].vwstart) = match;
					break;

				}
		}

		if (errc = InitCursor (tN, ftype, match))
			return (errc);

		ftype = SetFirstRec (ftype);	/* to retrieve first rec of match */
	}
	return (FetchRec (ftype, tN));
}

static char	*BuildStatement (node, ftype, rowid)
 TableNode	*node;
 int	ftype;
 long	rowid;
{
	int		i, sz;
	char		*s;
	KeyField	*k;

	/**	Make a wild guess about the expected string size
	**/
	for (i = sz = 0; i < node -> fldCount; i++)
		sz += strlen (node -> fields [i].vwname) + 3;
	if (node -> keys)
		for (k = node -> keys; k; k = k -> next)
		{
			struct sqlvar_struct	*f = GetColumn (node, k -> name);

			/*	where clause	*/
			sz += strlen (k -> name) + f -> sqllen + 10;
			/*	order by	*/
			sz += strlen (k -> name) + 3;
		}
	else
		sz += 20U;	/* "... rowid = value " */

	if (s = malloc (sz + 30U))		/** 30U for overflow **/
	{
		/**	Build the select statement
		**/
		strcpy (s, "select ");
		for (i = 0; i < node -> fldCount; i++)
			strcat (strcat (s, node -> fields [i].vwname), ", ");
		strcat (strcat (s, "rowid from "), node -> table);

		if (IsComparator (ftype))
		{
			/**	put in 'where' clauses
			**/
			if (node -> keys)
			{
				for (i = 0, k = node -> keys; k; i++, k = k -> next)
				{
					sprintf (s + strlen (s), " %s %s %s ?",
						k == node -> keys ? "where" : "and",
						k -> name,
						StrComparator (ftype, !k -> next));

					BuildParam (
						node -> selParams.sqlvar + i,
						GetColumn (node, k -> name));
				}
				node -> selParams.sqld = i;
			}
			else
				sprintf (s + strlen (s), " where rowid = %ld",
					rowid);
		} else
			node -> selParams.sqld = 0;	/* no params */

		/**	Add ordering if indexed	**/
		if (node -> keys)
		{
			strcat (s, " order by ");
			for (k = node -> keys; k; k = k -> next)
			{
				strcat (s, k -> name);
				if (k -> next)
					strcat (s, ", ");
			}
		}
	}
	return (s);
}

int
close_curse(name)
	char	*name;
{
	EXEC SQL BEGIN DECLARE SECTION;
	char			cur_name[15];
	EXEC SQL END DECLARE SECTION;

	sprintf(cur_name, "%s_cur", name);
	EXEC SQL CLOSE :cur_name;
	if (sqlca.sqlcode)
	{
		return ((int) sqlca.sqlcode);
	}
}

int
free_curse(name)
	char	*name;
{
	EXEC SQL BEGIN DECLARE SECTION;
	char			cur_name[15];
	EXEC SQL END DECLARE SECTION;
	sprintf(cur_name, "%s_cur", name);

	EXEC SQL FREE :cur_name;
	if (sqlca.sqlcode)
	{
		return ((int) sqlca.sqlcode);
	}
}

int
free_statement(name)
	char	*name;
{
	EXEC SQL BEGIN DECLARE SECTION;
	char			stmt_name[15];
	EXEC SQL END DECLARE SECTION;
	sprintf(stmt_name, "%s_stmt", name);

	EXEC SQL FREE :stmt_name;
	if (sqlca.sqlcode)
	{
		return ((int) sqlca.sqlcode);
	}
}

static	InitCursor (
 TableNode	*node,
 int		ftype,
 long		rowid)
{
	struct sqlda 	*prms;

	EXEC SQL BEGIN DECLARE SECTION;
	char			*statement;
	char			cur_name[15];
	char			stmt_name[15];
	EXEC SQL END DECLARE SECTION;

	sprintf(cur_name, "%s_cur", node->name);
	sprintf(stmt_name, "%s_stmt", node->name);

	/**	Clean up if required
	**/
	if (node -> curse == CURSE_Open)
	{
		EXEC SQL CLOSE :cur_name;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}
		
		EXEC SQL FREE :cur_name;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}

		EXEC SQL FREE :stmt_name;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}

		node -> curse = CURSE_Null;
	}

	if (statement = BuildStatement (node, ftype, rowid))
	{
		EXEC SQL PREPARE :stmt_name FROM :statement;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}	

		EXEC SQL DECLARE :cur_name SCROLL CURSOR FOR :stmt_name;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}

		free (statement);

		prms = &node->selParams;

		EXEC SQL OPEN :cur_name USING DESCRIPTOR prms;
		if (sqlca.sqlcode)
		{
			return ((int) sqlca.sqlcode);
		}

		node -> curse = CURSE_Open;
		return (0);
	}
	return (-1);
}

static	FetchRec (
 int	how,
 TableNode	*node)
{
	struct sqlda 	*prms;

	EXEC SQL BEGIN DECLARE SECTION;
	char			cur_name[15];
	char			stmt_name[15];
	EXEC SQL END DECLARE SECTION;

	sprintf(cur_name, "%s_cur", node->name);
	sprintf(stmt_name, "%s_stmt", node->name);
	prms = &node->data;

	switch (how)
	{
	case FIRST	:
		EXEC SQL FETCH FIRST :cur_name USING DESCRIPTOR prms;
		break;

	case LAST	:
		EXEC SQL FETCH LAST :cur_name USING DESCRIPTOR prms;
		break;

	case NEXT	:
		EXEC SQL FETCH NEXT :cur_name USING DESCRIPTOR prms;
		break;

	case PREVIOUS	:
		EXEC SQL FETCH PREVIOUS :cur_name USING DESCRIPTOR prms;
		break;

	case CURRENT	:
		EXEC SQL FETCH CURRENT :cur_name USING DESCRIPTOR prms;
		break;

	default	:
		return (-1);
	}

	if (!sqlca.sqlcode)
		DataFudgeOut (node, &node -> data);
	return ((int) sqlca.sqlcode);
}

/**	Support utilities
**/
static	IsComparator (ftype)
 int	ftype;
{
	return (
		ftype == EQUAL		|| ftype == COMPARISON ||
		ftype == GREATER	|| ftype == GTEQ ||
		ftype == LT			|| ftype == LTEQ);
}

static	IsAbsPosition (ftype)
 int	ftype;
{
	return (ftype == FIRST || ftype == LAST);
}

static	SetFirstRec (ftype)
 int	ftype;
{
	switch (ftype)
	{
	case EQUAL		:
	case COMPARISON	:
	case GREATER	:
	case GTEQ		:
		return (FIRST);	/* position cursor at first rec for these calls */

	case LT		:
	case LTEQ	:
		return (LAST);	/* position cursor at last rec for these calls */
	}
	return (FIRST);		/* actually an internal error */
}

static char	*StrComparator (
 int	ftype,
 int	last)
{
	static char	*Eq		= "=",
				*Gt		= ">",
				*GtEq	= ">=",
				*Lt		= "<",
				*LtEq	= "<=";

	switch (ftype)
	{
	case EQUAL	:
	case COMPARISON	:
		return (Eq);

	case GTEQ	:
		return (GtEq);

	case GREATER	:
		return (last ? Gt : GtEq);

	case LT		:
		return (last ? Lt : LtEq);

	case LTEQ	:
		return (LtEq);
	}
	return ("BadValue");
}

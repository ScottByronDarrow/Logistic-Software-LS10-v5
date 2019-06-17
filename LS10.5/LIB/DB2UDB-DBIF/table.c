#ident	"$Id: table.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Table management
 *
 *******************************************************************************
 *	$Log: table.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *
 *
 *	
 */
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>

#include	"db2dbif.h"
#include	"samputil.h"

/*
 *	Local functions
 */
static TableState * AddNode (TableState ** list, TableState * node);
static TableState * RemoveNode (TableState ** list, TableState * node);
static void DestroyNode (TableState *);
static void DestroyNodeList (TableState ** list);



/*
 */
static TableState * tables = NULL;

SQLRETURN rc;
SQLUINTEGER		numrowsfetched;
SQLUSMALLINT    row_status[ROWSET_SIZE];

/*
 *
 */

TableState *
AllocateStdTable (
 SQLHANDLE * hdbc,
 const char * name)
{
	/*
	 *	Standard table, possibly aliased
	 */
	TableState * node = NULL;
	const char * base = AliasBasename (name);
	
	if (base)
	{
		if (!(node = AllocateTable (hdbc, base)))
			return NULL;

		/*
		 *	Switched aliased names to named,
		 *	and the base against the real name
		 */
		node -> table = node -> named;
		/*node -> named = strdup (base);*/
		node -> named = strdup (name); /* You said, switched aliased names to named*/

	} else
	{
		if (!(node = AllocateTable (hdbc, name)))
			return NULL;
	}

	return AddNode (&tables, node);
}

void
DestroyTable (
 const char * name)
{
	TableState * table = LocateTable (name);

	if (!table)
		return;

	DestroyNode (RemoveNode (&tables, table));
}

void
DestroyAllTables ()
{
	DestroyNodeList (&tables);
}

TableState *
LocateTable (
 const char * name)
{
	TableState * t;

	for (t = tables; t; t = t -> next)
		if (!strcmp (name, t -> named))
			return t;
	return NULL;
}

/*
 *
 */
TableState *
AllocateTable (
 SQLHANDLE * hdbc,
 const char * table)
{
	TableState * node = malloc (sizeof (TableState));

	memset (node, 0, sizeof (TableState));
	node -> named = strdup (table);

	/*
	 *	Assocate an DB2 connection handle with a table
	 *  also allocate a statement handle while at it.
	 *	
	 *	Only do this for the query cursor, since this
	 *	is the primary reason for creating a table.
	 *
	 *	The update cursor will only get created if
	 *	required (ie on initial call to abc_update).
	 */
	
	
	/* A single db2 connection handle would be referenced be every table */
	node -> hdbc = hdbc;

	/*
	 * Initialize all allocation flags to 0
	 */
	node -> q_flag = 0;
	node -> i_flag =0;

	rc = SQLAllocHandle( SQL_HANDLE_STMT, *node -> hdbc, &node -> q_hstmt ) ;
	CHECK_HANDLE( SQL_HANDLE_DBC, *node -> hdbc, rc );
	node -> q_flag = 1;

	/*
	 * Set scrollable cursors attribute
	 */

	/* Set the number of rows in the rowset */
    rc = SQLSetStmtAttr(
                node -> q_hstmt, 
                SQL_ATTR_ROW_ARRAY_SIZE, 
                (SQLPOINTER) ROWSET_SIZE, 
                0);
    CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );

	 /* Set the cursor type to a pure keyset driven cursor */
    rc = SQLSetStmtAttr(
                node -> q_hstmt,
                SQL_ATTR_CURSOR_TYPE,
                (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN,
                0);
    CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );

	/* point to the variable numrowsfetched: */
    rc = SQLSetStmtAttr(
                node -> q_hstmt, 
                SQL_ATTR_ROWS_FETCHED_PTR, 
                &numrowsfetched,
                0);
    CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );
 
    /* Set a pointer to the array to use for the row status */
    rc = SQLSetStmtAttr(
                node -> q_hstmt, 
                SQL_ATTR_ROW_STATUS_PTR,
                (SQLPOINTER) row_status,
                0);
    CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );
	 

	/*****************************************/

	rc = SQLSetStmtAttr(node -> q_hstmt, SQL_ATTR_CURSOR_HOLD, (SQLPOINTER)SQL_CURSOR_HOLD_ON, SQL_IS_POINTER);
	CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );

	rc = SQLSetStmtAttr(node -> q_hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_LOCK, SQL_IS_POINTER);
	CHECK_HANDLE( SQL_HANDLE_DBC, node -> q_hstmt, rc );

	return node;
}

static void
DestroyNode (
 TableState * node)
{
	/*
	 */
	if (node -> named)
		free (node -> named);
	if (node -> table)
		free (node -> table);
	if (node -> index)
		free (node -> index);
	if (node -> columns)
		free (node -> columns);
	if (node -> viewactual)
		free (node -> viewactual);
	if (node -> indexname)
		free (node -> indexname);
	if (node -> indexactual)
		free (node -> indexactual);
	if (node -> indexview)
		free (node -> indexview);
	if (node -> data)
		free (node -> data);

	/*
	 */
	if (node -> q_flag )
	{
		free (node -> query);
		rc = SQLFreeHandle( SQL_HANDLE_STMT, node -> q_hstmt  ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, node -> q_hstmt , rc ) ;
	}
		
	/*
	 *	The ...
	 */
	if (node -> insert)
	{
		free (node -> insert);
	}
	if (node ->i_flag )
	{
		rc = SQLFreeHandle( SQL_HANDLE_STMT, node -> i_hstmt  ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, node -> i_hstmt , rc ) ;
	}
	if (node -> delete)
	{
		free (node -> delete);
	}

	free (node);
}

static TableState *
AddNode (
 TableState ** list,
 TableState * node)
{
	if (*list)
		return AddNode (&(*list) -> next, node);
	return *list = node;
}

static TableState *
RemoveNode (
 TableState ** list,
 TableState * node)
{
	/*
	 *	Removes a specific node from the list
	 */
	if (!*list)
		return NULL;				/* not in list */

	if (*list != node)
		return RemoveNode (&(*list) -> next, node);

	*list = node -> next;
	node -> next = NULL;

	return node;
}

static void
DestroyNodeList (
 TableState ** list)
{
	/*
	 *	Wipes out the whole list
	 */
	if (*list)
	{
		DestroyNodeList (&(*list) -> next);
		DestroyNode (*list);
		*list = NULL;
	}
}

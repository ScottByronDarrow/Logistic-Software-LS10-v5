#ident	"$Id: table.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Table management
 *
 *******************************************************************************
 *	$Log: table.c,v $
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
 *	Revision 1.4  2000/07/13 11:08:45  raymund
 *	16-bit reversed CRC hardware emulation algorithm for row locking.
 *	
 *	Revision 1.3  2000/07/13 06:43:51  gerry
 *	Linked error handling to standard LS/10 error handling, fixed alias bug
 *	
 *	Revision 1.2  1999/11/15 02:53:06  jonc
 *	Added lock code. Requires `alvin' the lock-daemon to be running.
 *	
 *	Revision 1.1  1999/10/21 21:47:05  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>

#include	"oracledbif.h"

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

/*
 *
 */
TableState *
AllocateStdTable (
 Lda_Def * lda,
 const char * name)
{
	/*
	 *	Standard table, possibly aliased
	 */
	TableState * node = NULL;
	const char * base = AliasBasename (name);

	if (base)
	{
		if (!(node = AllocateTable (lda, base)))
			return NULL;

		/*
		 *	Switched aliased names to named,
		 *	and the base against the real name
		 */
		node -> table = node -> named;
		/*
		 * node -> named = strdup (base);
		 */
		node -> named = strdup (name);

	} else
	{
		if (!(node = AllocateTable (lda, name)))
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
 Lda_Def * lda,
 const char * table)
{
	TableState * node = malloc (sizeof (TableState));

	memset (node, 0, sizeof (TableState));
	node -> named = strdup (table);

	/*
	 *	Assocate an ORACLE cursor with the table
	 *
	 *	Only do this for the query cursor, since this
	 *	is the primary reason for creating a table.
	 *
	 *	The update cursor will only get created if
	 *	required (ie on initial call to abc_update).
	 */
	if (oopen (&node -> q_cursor, lda, NULL, -1, -1, NULL, -1))
		oraclecda_error (lda, &node -> q_cursor, "AllocateTable");

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
    if (node -> orig_data )
		free ( node -> orig_data );

	/*
	 */
	if (node -> query)
		free (node -> query);
	if (oclose (&node -> q_cursor))
	{
	}

	/*
	 *	The update/delete/insert cursors only gets created conditionally
	 */
	if (node -> update)
	{
		free (node -> update);
		oclose (&node -> u_cursor);
	}
	if (node -> insert)
	{
		free (node -> insert);
		oclose (&node -> i_cursor);
	}
	if (node -> delete)
	{
		free (node -> delete);
		oclose (&node -> d_cursor);
	}

	/*
	 */
	// _LockFreeAll (node,&node -> locks);

	/*
	 */
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

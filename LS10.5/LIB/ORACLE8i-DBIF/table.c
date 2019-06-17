/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: table.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (table.c)
|  Program Desc  : (Table management)
|---------------------------------------------------------------------|
| $Log: table.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:52  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 02:09:53  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.2  2000/08/02 02:34:59  raymund
| Small performance improvements. Added codes for locked find_hash()es.
|
| Revision 2.1  2000/07/26 10:09:56  raymund
| Furnished missing functionalities. Use SQL for row locking.
|
| Revision 2.0  2000/07/15 07:33:51  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.4  2000/07/13 11:08:45  raymund
| 16-bit reversed CRC hardware emulation algorithm for row locking.
|
| Revision 1.3  2000/07/13 06:43:51  gerry
| Linked error handling to standard LS/10 error handling, fixed alias bug
|
| Revision 1.2  1999/11/15 02:53:06  jonc
| Added lock code. Requires `alvin' the lock-daemon to be running.
|
| Revision 1.1  1999/10/21 21:47:05  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdio.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>

#include	"oracledbif.h"

/*
 * Global variables 
 */
static TableState * tables = NULL;

/*
 * Local functions 
 */

static TableState 	*AddNode 	(TableState **, TableState *);
static TableState 	*RemoveNode (TableState **, TableState *);
static void DestroyNode 		(TableState *);
static void DestroyNodeList 	(TableState **);

/*
 * AllocateStdTable 
 */

TableState *
AllocateStdTable (
 	const char * name)
{

	/*
	 * Standard table, possibly aliased 
	 */
	TableState * node = NULL;
	const char * base = AliasBasename (name);

	if (base)
	{
		if (!(node = AllocateTable (base)))
			return NULL;

		/*
		 * Switched aliased names to named,  
		 * and the base against the real name
		 */
		node -> table = node -> named;

		/*
		 * node -> named = strdup (base); 
		 */

		node -> named = strdup (name);

	} else
	{
		if (!(node = AllocateTable (name)))
			return NULL;
	}

	return AddNode (&tables, node);
}


/*
 * AllocateTable 
 */
TableState *
AllocateTable (
 	const char * table)
{
	TableState * node = malloc (sizeof (TableState));

	memset (node, 0, sizeof (TableState));
	node -> named = strdup (table);

	return node;
}


/*
 * DestroyTable 
 */
void
DestroyTable (
 	const char * name)
{
	TableState * table = LocateTable (name);

	if (!table)
		return;

	DestroyNode (RemoveNode (&tables, table));
}

/*
 * DestroyAllTables 
 */
void
DestroyAllTables ()
{
	DestroyNodeList (&tables);
}

/*
 * LocateTable 
 */
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
 * AddNode 
 */
static TableState *
AddNode (
 	TableState ** list,
 	TableState *  node)
{
	if (*list)
		return AddNode (&(*list) -> next, node);
	return *list = node;
}

/*
 * RemoveNode 
 */
static TableState *
RemoveNode (
 	TableState ** list,
 	TableState * node)
{
	/*
	 * Removes a specific node from the list 
	 */
	if (!*list)
		return NULL;				/* not in list */

	if (*list != node)
		return RemoveNode (&(*list) -> next, node);

	*list = node -> next;
	node -> next = NULL;

	return node;
}

/*
 * DestroyNode 
 */
static void
DestroyNode (
	TableState * node)
{

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
		free (node -> orig_data);

	if (node -> query)
		free (node -> query);

	if (node -> q_stmt)
	{
		if (OCIHandleFree ((dvoid *) node -> q_stmt,
						   OCI_HTYPE_STMT))
			oraclecda_error (errhp, "table::OCIHandleFree:QueryStatement");
	}

	if (node -> update)
	{
		free (node -> update);
		if (node -> u_stmt)
		{
			if (OCIHandleFree ((dvoid *) node -> u_stmt,
							   OCI_HTYPE_STMT))
			oraclecda_error (errhp, "table::OCIHandleFree:UpdateStatement");
		}
	}
	
	if (node -> insert)
	{
		free (node -> insert);
		if (node -> i_stmt)
		{
			if (OCIHandleFree ((dvoid *) node -> i_stmt,
							   OCI_HTYPE_STMT))
			oraclecda_error (errhp, "table::OCIHandleFree:InsertStatement");
		}
	}
	
	if (node -> delete)
	{
		free (node -> delete);
		if (node -> d_stmt)
		{
			if (OCIHandleFree ((dvoid *) node -> d_stmt,
							   OCI_HTYPE_STMT))
			oraclecda_error (errhp, "table::OCIHandleFree:DeleteStatement");
		}
	}
	free (node);
}

/*
 * DestroyNodeList 
 */
static void
DestroyNodeList (
 	TableState ** list)
{
	/*
	 * Wipes out the whole list 
	 */
	if (*list)
	{
		DestroyNodeList (&(*list) -> next);
		DestroyNode (*list);
		*list = NULL;
	}
}

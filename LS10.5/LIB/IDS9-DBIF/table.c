/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: table.c,v 1.2 2002/11/11 02:41:10 cha Exp $                                                              
|  Program Name  : (table.c)                                          
|  Program Desc  : (table manipulation routines)                      
|---------------------------------------------------------------------|
| $Log: table.c,v $
| Revision 1.2  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|
| Revision 1.1  2002/07/17 07:24:28  cha
| Initial check in
|                                                                                                                                   
=====================================================================*/

/*Avoid duplicate definition for some 
functions and data structures from 
Informix and Standard C*/

#ifndef _H_LOCALEDEF  
#define _H_LOCALEDEF  
#endif                

#include <stdio.h> 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ids9dbif.h"


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
	int ctr = 0;

	if (node -> named)
	{
		free (node -> named);
		node -> named = NULL;
	}
	
	if (node -> table)
	{
		free (node -> table);
		node -> table = NULL;
	}

	if (node -> index)
	{
		free (node -> index);
		node -> index = NULL;
	}

	if (node -> columns)
	{
		for (ctr =0; ctr < node -> columnc; ctr++)
		{
			if (node -> columns[ctr].data)
			{
				free (node -> columns[ctr].data);	
				node -> columns[ctr].data = NULL;
			}
		}
		free (node -> columns);
		node -> columns = NULL;
	}
	
	if (node -> viewactual)
	{
		free (node -> viewactual);
		node -> viewactual = NULL;
	}

	/*if (node -> indexname)
	{
		free (node -> indexname);
	}*/
	
	if (node -> indexactual)
	{
		free (node -> indexactual);
		node -> indexactual = NULL;
	}
	
	if (node -> indexview)
	{
		free (node -> indexview);
		node -> indexview = NULL;
	}

	/*if (node -> data)
	{
		free (node -> data);
	}*/

    if (node -> orig_data )
    {
		free (node -> orig_data);
		node -> orig_data = NULL;
	 }

	if (node -> query)
	{
		free (node -> query);
		node -> query = NULL;
	}

	if (node -> q_stmt)
	{
		//fprintf (stdout,"\nDESTROY q_stmt");
		if (node -> q_stmt)
		{
			if ( mi_drop_prepared_statement(node -> q_stmt) == MI_ERROR )
				ids_dbase_err();
			node -> q_stmt = NULL;
		}
	}

	if (node -> update)
	{
		free (node -> update);
		node -> update = NULL;
		if (node -> u_stmt)
		{
			//fprintf (stdout,"\nDESTROY u_stmt");
			if ( mi_drop_prepared_statement(node -> u_stmt) == MI_ERROR )
				ids_dbase_err();
			//node -> u_stmt = NULL;
		}
	}
	
	if (node -> insert)
	{
		free (node -> insert);
		node -> insert = NULL;
		if (node -> i_stmt)
		{
			//fprintf (stdout,"\nDESTROY i_stmt");
			if ( mi_drop_prepared_statement(node -> i_stmt) == MI_ERROR )
				ids_dbase_err();
		}
	}
	
	if (node -> delete)
	{
		free (node -> delete);
		node -> delete = NULL;
		if (node -> d_stmt)
		{
			if ( mi_drop_prepared_statement(node -> d_stmt) == MI_ERROR )
				ids_dbase_err();
			//node -> d_stmt = NULL;
		}
	}
	
	
	if (node -> values)
	{
		free (node -> values);
		node -> values = NULL;
	}
	
	if (node -> lengths)
	{
		free (node -> lengths);	
		node -> lengths = NULL;
	}
	
	if (node -> types)
	{
		free (node -> types);	
		node -> types = NULL;
	}
		
	if (node -> nulls)
	{
		free (node -> nulls);
		node -> nulls = NULL;
	}
		
	free (node);
	node = NULL;
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

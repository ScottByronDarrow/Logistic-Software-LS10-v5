/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: alias.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (alias.c)
|  Program Desc  : (Datblade Interface to LS/10)
|---------------------------------------------------------------------|
| $Log: alias.c,v $
| Revision 1.2  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|

=====================================================================*/

/*Avoid duplicate definition for some 
functions and data structures from 
Informix and Standard C*/
#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ids9dbif.h"

/*
 * Structures
 */
struct _AliasNode
{
	char 	realname [255],
			fakename [255];
	struct _AliasNode * next;
};
typedef struct _AliasNode AliasNode;

/*
 * Global variables
 */
static AliasNode * aliases = NULL;

/*
 * Local functions
 */
static AliasNode * CreateAlias (const char * fake, const char * real);
static AliasNode * AddAlias (AliasNode **, AliasNode *);
static void _DestroyAliasList (AliasNode **);
static void DestroyAlias (AliasNode *);

/*
 * AliasBasename
 */

 int
abc_alias (
 const char * fake,
 const char * real)
{
    /*
     *  See if name already exists
     */
    AliasNode * p;

    for (p = aliases; p; p = p -> next)
    {
        if (!strcmp (fake, p -> fakename))
        {
            strcpy (p -> realname, real);
            return 0;
        }
    }

    /*
     *  Create a new entry
     */
    AddAlias (&aliases, CreateAlias (fake, real));
    return 0;
}


static AliasNode *
AddAlias (
 AliasNode ** list,
 AliasNode * node)
{
    if (*list)
        return AddAlias (&(*list) -> next, node);

    return *list = node;
}

static AliasNode *
CreateAlias (
 const char * fake,
 const char * real)
{
    AliasNode * node = malloc (sizeof (AliasNode));

    memset (node, 0, sizeof (AliasNode));
    strcpy (node -> fakename, fake);
    strcpy (node -> realname, real);

    return node;
}

const char *
AliasBasename (
 	const char * fake)
{
	/*
	 * Return a basename, if any
	 */
	AliasNode * p;

	for (p = aliases; p; p = p -> next)
		if (!strcmp (fake, p -> fakename))
			return p -> realname;

	return NULL;
}

/*
 * DestroyAliasList
 */
void
DestroyAliasList ()
{
	_DestroyAliasList (&aliases);
}

/*
 * _DestroyAliasList
 */
static void
_DestroyAliasList (
	AliasNode ** list)
{
	/*
	 * Wipes out the whole list
	 */
	if (* list)
	{
		_DestroyAliasList (&(* list) -> next);
		DestroyAlias (* list);
		* list = NULL;
	}
}

/*
 * DestroyAlias
 */
static void
DestroyAlias (
	AliasNode * node)
{
	free (node);
}


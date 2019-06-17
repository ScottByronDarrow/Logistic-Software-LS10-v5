#ident	"$Id: alias.c,v 5.1 2002/02/04 10:34:36 cha Exp $"
/*
 *	Maintain aliases
 *
 *******************************************************************************
 *	$Log: alias.c,v $
 *	Revision 5.1  2002/02/04 10:34:36  cha
 *	Updated to fix errors in alias names.
 *	Increased the alias name from 16 to 255 characters.
 *	
 *	Revision 5.0  2001/06/19 07:10:27  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/10/21 21:47:03  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdlib.h>
#include	<string.h>

#include	"oracledbif.h"

/*
 *	Local structures
 */
struct _AliasNode
{
	char realname [255], fakename [255];

	struct _AliasNode * next;
};
typedef struct _AliasNode AliasNode;


/*
 */
static AliasNode * CreateAlias (const char * fake, const char * real);
static AliasNode * AddAlias (AliasNode **, AliasNode *);
static void _DestroyAliasList (AliasNode **);

/*
 */
static AliasNode * aliases = NULL;

/*
 */
int
abc_alias (
 const char * fake,
 const char * real)
{
	/*
	 *	See if name already exists
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
	 *	Create a new entry
	 */
	AddAlias (&aliases, CreateAlias (fake, real));
	return 0;
}

const char *
AliasBasename (
 const char * fake)
{
	/*
	 *	Return a basename, if any
	 */
	AliasNode * p;
	for (p = aliases; p; p = p -> next)
	{
		if (!strcmp (fake, p -> fakename))
			return p -> realname;
	}
	return NULL;
}

void
DestroyAliasList ()
{
	_DestroyAliasList (&aliases);
}

/*
 */
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

static void
DestroyAlias (
 AliasNode * node)
{
	free (node);
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

static void
_DestroyAliasList (
 AliasNode ** list)
{
	/*
	 *	Wipes out the whole list
	 */
	if (*list)
	{
		_DestroyAliasList (&(*list) -> next);
		DestroyAlias (*list);
		*list = NULL;
	}
}

#ident	"$Id: alias.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Maintain aliases
 *
 *******************************************************************************
 *	$Log: alias.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	
 *	
 */
#include	<stdlib.h>
#include	<string.h>

/*#include	"db2dbif.h"*/

/*
 *	Local structures
 */
struct _AliasNode
{
	char realname [128], fakename [128];

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
		if (!strcmp (fake, p -> fakename))
			return p -> realname;
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

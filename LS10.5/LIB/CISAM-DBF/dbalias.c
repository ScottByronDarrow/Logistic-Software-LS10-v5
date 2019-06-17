#ident	"$Id: dbalias.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbalias.c,v $
 *	Revision 5.1  2001/08/20 23:07:45  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.0  2001/06/19 07:07:42  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:17  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/09/30 04:57:27  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

/*
 * structure for aliases
 */
struct tagAlias
{
	char	*alias,
			*actual;

	struct tagAlias	*next;
};
typedef	struct tagAlias	Alias;

/*
 *	Alias list
 */
static Alias	*aliasList = NULL;

/*	Function declarations
 *
 */
static int	find_file 	(const char *);
static int	AddAlias 	(const char *, const char *);

/*
 *	Real Code
 */
int
dbalias (
 const char * newname,
 const char * oldname)
{
	LLIST	*tptr = _GetNode (newname);

	if (tptr)
		return (6037);	/*	alias already exists */

	if (find_file (newname))
		return (6036);	/*	file exists as itself */

	if (!find_file (oldname))
		return (6040);	/*	actual file does not exist */

	return (AddAlias (newname, oldname) ? 0 : 6024);
}

/*
 * Check if the filename is an alias
 */
const char *
_check_alias (
 const char *filename)
{
	Alias	*cur;

	for (cur = aliasList; cur; cur = cur -> next)
		if (!strcmp (filename, cur -> alias))
			return (cur -> actual);

	return (filename);
}

/*
 * lookup systables for tabname	
 */
static int
find_file (
const char * filename)
{
	char	tabname [NAME_LEN + 1];
	LLIST	*sptr = _GetSysNode (SYS_TAB);

	/*
	 * initialise buffer		
	 */
	stchar (filename, sptr -> _buffer, NAME_LEN);
	stchar (" ", sptr -> _buffer + NAME_LEN, 8);

	/*
	 * perform read		
	 */
	if (isread (sptr -> _fd, sptr -> _buffer, ISGTEQ))
		return (FALSE);	/*	find failed */

	/*
	 * check the return results
	 */
	strncpy (tabname, sptr -> _buffer, NAME_LEN) [NAME_LEN] = '\0';

	return (!strcmp (clip (tabname), filename));
}

/*
 *	Alias list management routines
 */
static int
AddAlias (
 const char * alias,
 const char * actual)
{
	Alias	*node;

	/*
	 * Check whether alias already exists
	 */
	for (node = aliasList; node; node = node -> next)
	{
		if (!strcmp (alias, node -> alias))
		{
			/* Overwrite
			*/
			free (node -> actual);				/* free old name */
			node -> actual = strdup (actual);	/* copy new name */
			return (TRUE);
		}
	}

	/*
	 * Allocate new node for aliasList
	 */
	if (!(node = (Alias *) malloc (sizeof (Alias))))
		return (FALSE);		/* no memory left! */

	memset (node, 0, sizeof (Alias));	/* flush clean */
	node -> alias  = strdup (alias);
	node -> actual = strdup (actual);

	/*
	 * Insert node at head of aliasList
	 */
	node -> next = aliasList;
	aliasList = node;

	return (TRUE);
}

void	ClearAliases (void)
{
	while (aliasList)
	{
		Alias	*dead = aliasList;		/* hold the node to be freed */

		aliasList = aliasList -> next;	/* move it onto the next element */

		/*
		 * Free the dead node
		 */
		free (dead -> alias);
		free (dead -> actual);
		free (dead);
	}
}

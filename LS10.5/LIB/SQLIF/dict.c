/*******************************************************************************

	Dictionary maintenance

$Log: dict.c,v $
Revision 5.1  2001/08/09 09:31:24  scott
Updated to add FinishProgram () function

Revision 5.0  2001/06/19 07:14:40  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:06  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/12  11:41:23  jonc
 * Initial revision
 * 

*******************************************************************************/
#ifndef	lint
static char	*rcsid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/dict.c,v 5.1 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<malloc.h>
#include	<string.h>

#include	"dict.h"
#include	"stddefs.h"

/**	Local global
**/
static Dictionary	*aliasList = (Dictionary *) 0;

extern Dictionary	*makeEntry (char *, char *);

/**
	Adds entries to head of list
**/
AddEntry (entry, meaning)
char	*entry;
char	*meaning;
{
	Dictionary	*n = makeEntry (entry, meaning);

	if (!n)
		return (FALSE);

	/* add to head */
	n -> next = aliasList;
	aliasList = n;

	return (TRUE);
}

static void	_DelDictionary (dict)
Dictionary	**dict;
{
	if (*dict)
	{
		_DelDictionary (&(*dict) -> next);
		
		free ((*dict) -> entry);
		free ((*dict) -> really);
		free (*dict);
		*dict = (Dictionary *) 0;
	}
}

void	DelDictionary ()
{
	_DelDictionary (&aliasList);
}

char	*GetMeaning (entry)
char	*entry;
{
	Dictionary	*e;
	
	for (e = aliasList; e; e = e -> next)
		if (!strcmp (e -> entry, entry))
		{
			char	*again = GetMeaning (e -> really);

			return (again ? again : e -> really);
		}
	return ((char *) 0);
}

static Dictionary	*makeEntry (entry, meaning)
char	*entry, *meaning;
{
	Dictionary	*n = (Dictionary *) malloc (sizeof (Dictionary));

	if (n)
	{
		memset (n, 0, sizeof (Dictionary));
		n -> entry = strcpy (malloc (strlen (entry) + 1U), entry);
		n -> really = strcpy (malloc (strlen (meaning) + 1U), meaning);
	}
	return (n);
}

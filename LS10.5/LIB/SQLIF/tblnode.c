/*******************************************************************************

	Manage the lists of KeyFields and TableNodes

$Log: tblnode.c,v $
Revision 5.1  2001/08/09 09:31:24  scott
Updated to add FinishProgram () function

Revision 5.0  2001/06/19 07:14:42  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:07  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.3  93/09/09  08:52:45  jonc
 * Fixed : Table locks being held on database close
 * 
 * Revision 1.2  93/05/12  11:41:29  jonc
 * Tablenode Maintenance
 * 

*******************************************************************************/
#ifndef	lint
static char	*rcsid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/tblnode.c,v 5.1 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<malloc.h>
#include	<string.h>
#include	<sqltypes.h>

#include	"isamdbio.h"
#include	"stddefs.h"
#include	"tblnode.h"
#include	"fnproto.h"

KeyField	*NewKeyField (
char		*name,
short		colType)
{
	KeyField	*n = (KeyField *) malloc (sizeof (KeyField));

	if (n)
	{
		memset (n, 0, sizeof (KeyField));
		n -> name = strcpy (malloc (strlen (name) + 1U), name);
		n -> colType = colType;
	}
	return (n);
}

void	DelKeyField (n)
KeyField	**n;
{
	if (*n)
	{
		DelKeyField (&(*n) -> next);

		free ((*n) -> name);
		free (*n);
		*n = (KeyField *) 0;
	}
}

KeyField	*AddKeyField (keyList, newElem)
KeyField	**keyList;
KeyField	*newElem;
{
	return (*keyList ?
		AddKeyField (&(*keyList) -> next, newElem) :
		(*keyList = newElem));
}

/***	TableNode stuff
***/
/*	Local Global
*/
static TableNode	*tableList = (TableNode *) 0;

static struct sqlvar_struct	*AllocVars (int fldCount)
{
	/*	Allocate var structures & indicators
	*/
	int	i;
	int	vsz = sizeof (struct sqlvar_struct) * fldCount,
		isz = sizeof (short) * fldCount;

	struct sqlvar_struct	*blk = (struct sqlvar_struct *) malloc (vsz);
	short			*ind = (short *) malloc (isz);

	memset (blk, 0, vsz);
	memset (ind, 0, isz);

	for (i = 0; i < fldCount; i++)
		blk [i].sqlind = ind + i;

	return ((struct sqlvar_struct *) blk);
}

static void	FreeVars (struct sqlvar_struct *v)
{
	/*	Free up var structure & indicators
	*/
	free ((char *) v -> sqlind);	/* first block does all indicators */
	free (v);
}

static void	FreeTableNode (node)
TableNode	*node;
{
	/**	just wipes one node (without going down the list)
	**/
	int	i;

	/**	Free up any table locks
	**/
	UnlockRecs (node -> name);	/*	has to be done first 'cos UnlockRecs
									references the current node
								*/

	/*	free up SQL stuff
	*/
	switch (node -> curse)
	{
	case CURSE_Open		:
		close_curse(node->name);
		free_curse(node->name);
		free_statement(node->name);
		node -> curse = CURSE_Null;
		break;

	case CURSE_Closed	:
		free_curse(node->name);
		free_statement(node->name);
		node -> curse = CURSE_Null;
		break;

	default				:
		node -> curse = CURSE_Null;
		break;
	}

	/*	free other stuff
	*/
	if (node -> name != node -> table)
		free (node -> name);
	free (node -> table);
	DelKeyField (&node -> keys);
	DelKeyField (&node -> unused);

	if (node -> fldCount)
	{
		FreeVars (node -> data.sqlvar);
		FreeVars (node -> updParams.sqlvar);
		FreeVars (node -> selParams.sqlvar);
	}

	/*	close lock file
	*/
	if (node -> lock_fd >= 0)
		close (node -> lock_fd);

	free ((char *) node);
}

TableNode	*NewTableNode (name, table, view, fldCount)
char		*name, *table;
struct dbview	*view;
int		fldCount;
{
	TableNode	*n = (TableNode *) malloc (sizeof (TableNode));

	if (n)
	{
		memset (n, 0, sizeof (TableNode));
		n -> name = strcpy (malloc (strlen (name) + 1U), name);
		if (strcmp (name, table))
			n -> table = strcpy (malloc (strlen (table) + 1U), table);
		else
			n -> table = n -> name;

		n -> fields = view;
		n -> fldCount = fldCount;

		n -> curse = CURSE_Null;

		/**	Set up sqlda area
		**/
		if (fldCount)
		{
			int	i, start;

			n -> data.sqld = fldCount + 1;	/* +1 for rowid	*/
			n -> data.sqlvar = AllocVars (fldCount + 1);
			n -> updParams.sqlvar = AllocVars (fldCount);
			n -> selParams.sqlvar = AllocVars (fldCount);

			start = 0;
			for (i = 0; i < fldCount; i++)
			{
				n -> data.sqlvar [i].sqlname = view [i].vwname;
				n -> data.sqlvar [i].sqltype = XlatSQLC (view [i].vwtype & SQLTYPE);

				/*	Setup length	*/
				n -> data.sqlvar [i].sqllen = rtypmsize (
					n -> data.sqlvar [i].sqltype, view [i].vwlen);

				/*	Realign vwstart	*/
				view [i].vwstart =
					rtypalign (start, n -> data.sqlvar [i].sqltype);

				start = view [i].vwstart +
					n -> data.sqlvar [i].sqllen;
			}

			/*	Setup for rowid
			*/
			n -> data.sqlvar [fldCount].sqlname = "rowid";
			n -> data.sqlvar [fldCount].sqldata = (char *) &n -> rowid;
			n -> data.sqlvar [fldCount].sqltype = CLONGTYPE;
			n -> data.sqlvar [fldCount].sqllen = rtypmsize (
				n -> data.sqlvar [fldCount].sqltype, sizeof (long));
		}

		/*	Initialise lock stuff
		*/
		n -> lock_fd = -1;	/* set to uninitialized */
	}
	return (n);
}

static void	_DelTableNodes (n)
TableNode **n;
{
	if (*n)
	{
		_DelTableNodes (&(*n) -> next);
		FreeTableNode (*n);
		*n = (TableNode *) 0;
	}
}

void	DelTableNodes ()
{
	_DelTableNodes (&tableList);
}

/**	The 'add' and 'get' routines have to be sync'd so
	the any requests obtain the most recently opened
	table - this resolves problems with multiply opened tables
**/
TableNode	*AddTableNode (newElem)
TableNode	*newElem;
{
	newElem -> next = tableList;
	return (tableList = newElem);	/* add to head */
}

TableNode	*GetTableNode (tblName)
char		*tblName;
{
	TableNode	*tbl;

	for (tbl = tableList; tbl; tbl = tbl -> next)	/* find from head */
		if (!strcmp (tblName, tbl -> name))
			return (tbl);
	return ((TableNode *) 0);
}

/**	Detaches and frees up given node from list
**/
static	_DetTableNode (tblList, tblNode)
TableNode	**tblList;
TableNode	*tblNode;
{
	if (!*tblList)
		return (FALSE);			/* internal error?	*/

	if (*tblList == tblNode)
	{
		TableNode	*dead = *tblList;

		*tblList = dead -> next;	/* detach from the list	*/
		FreeTableNode (dead);		/* and wipe */
		return (TRUE);
	}
	return (_DetTableNode (&(*tblList) -> next, tblNode));
}

DetTableNode (tblNode)
TableNode	*tblNode;
{
	return (_DetTableNode (&tableList, tblNode));
}

/**	Retrieves associated sqlvar
**/
struct sqlvar_struct	*GetColumn (node, name)
TableNode	*node;
char		*name;
{
	int	i;

	for (i = 0; i < node -> data.sqld; i++)
		if (!strcmp (node -> data.sqlvar [i].sqlname, name))
			return (node -> data.sqlvar + i);
	return ((struct sqlvar_struct *) 0);
}

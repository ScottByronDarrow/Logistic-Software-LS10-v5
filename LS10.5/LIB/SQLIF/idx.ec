/*******************************************************************************

	Index expansion code

$Log: idx.ec,v $
Revision 5.0  2001/06/19 07:14:41  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:07  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:31:28  jonc
 * Initial revision
 * 

*******************************************************************************/
#ifndef	lint
static char	*rscid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/idx.ec,v 5.0 2001/06/19 07:14:41 cha Exp $";
#endif

#include	"stddefs.h"
#include	"tblnode.h"
#include	"fnproto.h"

$include	sqlca;
$include	sqltypes;

/*	Uncomment next line for 16 part index instead of 8 part index db's
*/

static int	AddCol (tblNode, tabId, colNo)
TableNode	*tblNode;
$long	tabId;
$short	colNo;
{
	$string	colName [20];
	$short	colType;

	if (!colNo)
		return (0);		/*	relax, it's ok	*/

	$select	colname, coltype
	into	$colName, $colType
	from	informix.syscolumns
	where	tabid = $tabId
	and	colno = $colNo;

	if (!sqlca.sqlcode)
		AddKeyField (&tblNode -> keys, NewKeyField (colName, colType));

	return ((int) sqlca.sqlcode);
}

/***
	Expand the given Index to full field list
***/
IndexOn (nodeName, indexName)
char	*nodeName;
$char	*indexName;
{
	int	errc;
	$long	tabId;
	$char	*tableName;
	$short	colNo1, colNo2, colNo3, colNo4,
		colNo5, colNo6, colNo7, colNo8;

	TableNode	*tNode = GetTableNode (nodeName);

	if (!tNode)
		return (-1);

	tableName = tNode -> table;

	/**	Verify table existence (again)
	**/
	$select	tabid
		into	$tabId
		from	informix.systables
		where	tabname = $tableName;
	if (sqlca.sqlcode)
		return ((int) sqlca.sqlcode);

	$select	part1, part2, part3, part4,
		part5, part6, part7, part8
	into	$colNo1, $colNo2, $colNo3, $colNo4,
		$colNo5, $colNo6, $colNo7, $colNo8
	from	informix.sysindexes
	where	tabid = $tabId
	and	idxname = $indexName;
	if (sqlca.sqlcode)
		return ((int) sqlca.sqlcode);

	DelKeyField (&tNode -> keys);		/* clear out old index	*/

	if ((errc = AddCol (tNode, tabId, colNo1)) ||
			(errc = AddCol (tNode, tabId, colNo2)) ||
			(errc = AddCol (tNode, tabId, colNo3)) ||
			(errc = AddCol (tNode, tabId, colNo4)) ||
			(errc = AddCol (tNode, tabId, colNo5)) ||
			(errc = AddCol (tNode, tabId, colNo6)) ||
			(errc = AddCol (tNode, tabId, colNo7)) ||
			(errc = AddCol (tNode, tabId, colNo8)))
		return (errc);
	return (0);
}

ValidHashIdx (tableName)
char	*tableName;
{
	int		idxType;
	TableNode	*tNode = GetTableNode (tableName);

	if (!tNode)
		return (FALSE);
	if (!tNode -> keys)
		return (TRUE);		/* index is on rowid */

	idxType = tNode -> keys -> colType & SQLTYPE;
	return ((idxType == SQLINT || idxType == SQLSERIAL) &&
		!tNode -> keys -> next);
}

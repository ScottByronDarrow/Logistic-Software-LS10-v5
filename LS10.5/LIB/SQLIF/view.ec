/*******************************************************************************

	Table information extraction code

$Log: view.ec,v $
Revision 5.0  2001/06/19 07:14:43  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:08  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/12  11:41:47  jonc
 * Initial revision
 * 

*******************************************************************************/
#ifndef	lint
static char	*rcsid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/view.ec,v 5.0 2001/06/19 07:14:43 cha Exp $";
#endif

#include	<string.h>

#include	"dict.h"
#include	"isamdbio.h"
#include	"tblnode.h"
#include	"sqlerrs.h"
#include	"fnproto.h"

$include	sqlca;

#define	COLNAMELEN	18

/**
	Verify existence of the fields, and if all check out, link it onto
	the table list
**/
MakeView (origName, fldList, fldCount)
char	*origName;
struct dbview	fldList [];
int		fldCount;
{
	int		i;
	$long	tabId;
	$char	*tblName;
	$string	colName [COLNAMELEN + 1];
	$short	colNo, colType, colLength;

	KeyField	*unused = (KeyField *) 0;	/*	list of unused columns	*/

	for (i = 0; i < fldCount; i++)
		fldList [i].vwstart = fldList [i].vwtype = fldList [i].vwlen = 0;

	/*	Resolve possible aliases	*/
	if (!(tblName = GetMeaning (origName)))
		tblName = origName;

	/**	Verify the existence of the table & fields
	**/
	$select	tabid
		into	$tabId
		from	informix.systables
		where	tabname = $tblName;
	if (sqlca.sqlcode)
		return ((int) sqlca.sqlcode);

	$declare chkCols cursor for
		select	colname, coltype, collength, colno
		from	informix.syscolumns
		where	tabid = $tabId
		order by
				colno;

	$open chkCols;
	while (!sqlca.sqlcode)
	{
		$fetch chkCols into $colName, $colType, $colLength, $colNo;

		if (!sqlca.sqlcode)
		{
			for (i = 0; i < fldCount; i++)
				if (!strncmp (fldList [i].vwname, colName, COLNAMELEN))
				{
					fldList [i].vwstart = 1;
					fldList [i].vwtype = colType;
					fldList [i].vwlen = colLength;
					break;
				}

			/*	if column is unused, add to 'unused' list
			*/
			if (i >= fldCount)
				AddKeyField (&unused, NewKeyField (colName, colType));
		}
	}
	$close chkCols;

	/**	Check for unresolved cols
	**/
	for (i = 0; i < fldCount; i++)
		if (!fldList [i].vwstart)
			return (ERR_COLNOTFOUND);

	/**	Link it onto the list
	**/
	AddTableNode (
		NewTableNode (origName, tblName, fldList, fldCount)) -> unused = unused;

	return (0);
}

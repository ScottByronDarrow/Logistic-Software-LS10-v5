/*******************************************************************************

	Database open/closes and initialization code

$Log: dbopen.ec,v $
Revision 5.0  2001/06/19 07:14:40  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:06  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:31:12  jonc
 * Initial revision
 * 

*******************************************************************************/
#ifndef	lint
static char	*rscid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/dbopen.ec,v 5.0 2001/06/19 07:14:40 cha Exp $";
#endif

#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>

#include	"dict.h"
#include	"sqlerrs.h"
#include	"stddefs.h"
#include	"tblnode.h"
#include	"fnproto.h"

$include	sqlca;

/**	Local global
**/
static int	isopen = FALSE;

/**
	OpenDb (dbName) - opens database given by 'dbName'
	
	NOTES	:

	If 'dbName' doesn't exist, we'll try 'data'
	'cos all PINNACLE apps use this
**/
OpenDb (dbName)
$char	*dbName;
{
	if (isopen)			/* Something else already opened! */
		return (-1);

	$database $dbName;

	if (sqlca.sqlcode == ERR_DBNOTFOUND)
		$database data;		/* Fallback */

	InitLockSystem ();

	if (!sqlca.sqlcode)
	{
		/*	Clean up the lists	*/
		DelDictionary ();
		DelTableNodes ();
	} else
		isopen = TRUE;
	return ((int) sqlca.sqlcode);
}

/**
	CloseDb (char *dbname) - closes db given by 'dbname'
**/
/*ARGSUSED*/
CloseDb (dbName)
char	*dbName;
{
	$close database;

	/*	Clean up the lists	*/
	DelDictionary ();
	DelTableNodes ();

	isopen = FALSE;

	return ((int) sqlca.sqlcode);
}

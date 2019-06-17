#ident	"$Id: dbdelete.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbdelete.c,v $
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
 *	Revision 1.6  2000/04/14 02:24:56  scott
 *	Updated to use -DLOG so that logging of write locks can be optional on compile.
 *	
 *	Revision 1.5  2000/02/14 20:28:19  jonc
 *	Extended log message to include file/table involved.
 *	
 *	Revision 1.4  2000/02/10 00:32:46  jonc
 *	Added logging for lock errors on abc_update and abc_delete.
 *	
 *	Revision 1.3  1999/09/30 04:57:27  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

int
dbdelete (
 const char *filename)
{
	int		err;
	LLIST	*tptr = _GetNode (filename);

	if (!tptr)
		return (6021);	/* node not there! */

#ifdef LOG
	/*
	 *	Log conflicts in lock modes
	 */
	if (tptr -> lockmode != WrLock)
		DBIFWriteLog ("%s deletes %s without writelock", PNAME, filename);
#endif /*LOG*/

	/*
	 * delete row in table		
	 */
	err = isdelcurr (tptr -> _fd);

#ifdef	AUDIT
	write_aud (tptr, err ? "d" : "D", tptr -> _recno);
#endif	/*AUDIT*/
	return (err ? iserrno : 0);
}

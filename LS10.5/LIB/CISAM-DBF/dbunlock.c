#ident	"$Id: dbunlock.c,v 5.1 2001/08/20 23:07:46 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbunlock.c,v $
 *	Revision 5.1  2001/08/20 23:07:46  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/09/30 04:57:28  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

int
dbunlock (
 const char *filename)
{
	int		err;
	LLIST	*tptr = _GetNode (filename);

	if (!tptr)
		return (6023);	/* node not found! */

	/*
	 * unlock table
	 */
	if (!(err = isrelease (tptr -> _fd)))
		err = isunlock (tptr -> _fd);
	return (err ? iserrno : 0);
}

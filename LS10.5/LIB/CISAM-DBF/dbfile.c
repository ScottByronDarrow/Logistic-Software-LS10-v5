#ident	"$Id: dbfile.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbfile.c,v $
 *	Revision 5.1  2001/08/20 23:07:45  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
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

int
dbfile(
 const char *filename,
 short int *numflds,
 short int *recsize,
 short int *perms)
{
	LLIST	*sptr;
	LLIST	*tptr;
	/*
	 * find node for table		
 	 */
	if (!(tptr = _GetNode (filename)))
		return (6006);
	/*
	 * find table		
 	 */
	sptr = _GetSysNode (SYS_TAB);
	stchar 
	(
		filename, 
		sptr -> _buffer + KEYPART(0).kp_start,
		KEYPART(0).kp_leng
	);
	stchar 
	(
		" ",
		sptr -> _buffer + KEYPART(1).kp_start,
		KEYPART(1).kp_leng
	);
	if (isread (sptr -> _fd, sptr -> _buffer, ISGTEQ))
		return (iserrno);

	/*
	 * find failed		
 	 */
	if (strncmp (sptr -> _buffer + KEYPART(0).kp_start, filename, KEYPART(0).kp_leng))
		return(6001);
	*numflds = (short) ldint (sptr -> _buffer + 90 + SIZINT + SIZSMINT);
	*recsize = (short) ldint (sptr -> _buffer + 90 + SIZINT);
	*perms = (short) PERM_ALL;
	return(0);
}

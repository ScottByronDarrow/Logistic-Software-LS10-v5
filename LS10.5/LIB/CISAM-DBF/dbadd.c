#ident	"$Id: dbadd.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbadd.c,v $
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
 *	Revision 1.3  1999/09/30 04:57:26  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

int
dbadd (
 const char *filename,
 char *recbuf)
{
	int		i,
			err;
	long	serial;
	LLIST	*tptr;

	/*
	 * find the node for the table	
	 */
	
	if (!(tptr = _GetNode (filename)))
		return(6018);

	/*
	 * initialise the whole isam
	 */
	
	(void) memcpy (tptr -> _buffer, tptr -> _init, tptr -> _buf_size);

	/*
	 * set isam from structure
	 */
	
	for (i = 0; i < tptr -> _no_fields; i++)
		_load_isam 
		(
			tptr -> _view [i].vwtype,
			tptr,
			tptr -> _start [i],
			tptr -> _view [i].vwlen,
			recbuf + tptr -> _view [i].vwstart
		);

	/*
	 * there is a serial field
 	 */
	if (tptr -> _serial >= 0)
	{
		if (isuniqueid (tptr -> _fd, &serial))
			return(iserrno);
		/*
		 * load the serial value.
		 */
		
		_load_isam
		(
			SERIALTYPE,
			tptr,
			tptr -> _serial,
			sizeof (long),
			(char *) &serial
		);
	}
	/*
	 * perform isam add	
 	 */
	err = iswrite (tptr -> _fd, tptr -> _buffer);

#ifdef	AUDIT
	write_aud (tptr, err ? "a" : "A", isrecnum);
#endif	/*AUDIT*/
	tptr -> _recno = isrecnum;
	return (err ? iserrno : 0);
}

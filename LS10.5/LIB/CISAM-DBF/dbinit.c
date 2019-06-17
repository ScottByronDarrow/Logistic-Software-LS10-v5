#ident	"$Id: dbinit.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbinit.c,v $
 *	Revision 5.1  2001/08/20 23:07:45  scott
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

/*
 * Put data into the isam record
 */
void
_load_isam (
	int fieldtype,
	LLIST *tptr,
	int start,
	int len,
	const char *value)
{
	switch(fieldtype)
	{
	case CHARTYPE:
		stchar (value, tptr -> _buffer + start, len);
		break;

	case INTTYPE:
	{
		int	v = *(int *) value;

		if (v == INTNULL)
			v = 0;
		stint (v, tptr -> _buffer + start);
		break;
	}

	case LONGTYPE:
	case SERIALTYPE:
	{
		long	v = *(long *) value;

		if (v == LONGNULL)
			v = 0;
		stlong (v, tptr -> _buffer + start);
		break;
	}

	case DATETYPE:
	case EDATETYPE:
	case YDATETYPE:
	{
		long	v = *(long *) value;

		if (!v)
			v = LONGNULL;
		stlong (v, tptr -> _buffer + start);
		break;
	}

	case FLOATTYPE:
		stfltnull (*(float *) value, tptr -> _buffer + start, (short) 0);
		break;

	case DOUBLETYPE:
		stdblnull (*(double *) value, tptr -> _buffer + start, (short) 0);
		break;

	case MONEYTYPE:
		{
			/*
		 	 * Money types are used as double (in cents) in application code
		 	 * but stored as dec_t (in dollars) in the database
		 	 */
			double	v = *(double *) value;
			dec_t	d;
	
			v = DOLLARS (v);		/* convert to dollars */
			deccvdbl (v, &d);		/* convert to dec_t */
			stdecimal (&d, tptr -> _buffer + start, DECLENGTH (len));
			break;
		}
	}
}

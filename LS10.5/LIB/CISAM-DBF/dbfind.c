#ident	"$Id: dbfind.c,v 5.3 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbfind.c,v $
 *	Revision 5.3  2001/08/20 23:07:45  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.2  2001/08/06 22:47:56  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.1  2001/07/25 00:47:11  scott
 *	Updated for LS10.5
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.1  2000/10/13 07:27:31  scott
 *	Fixed dbfind call for LTEQ when EOF reached.
 *	LTEQ call actually performed a GREATER and did not take into account EOF.
 *	Should have performed a LAST instead of returning an error.
 *	LS10-GUI (formally known as the artist GVision) PLEASE NOTE
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.7  2000/03/14 02:17:14  jonc
 *	Imported DISAM dec_t conversion routines, as Informix 7.2+ appears
 *	to have severely damanged their conversion functions.
 *	
 *	Revision 1.6  2000/03/12 22:54:13  jonc
 *	Added explicit init for double in attempt to solve some weird conv problems
 *	
 *	Revision 1.5  2000/02/10 00:32:46  jonc
 *	Added logging for lock errors on abc_update and abc_delete.
 *	
 *	Revision 1.4  1999/10/08 03:41:44  jonc
 *	Removed prototypes found in isam.h
 *	
 *	Revision 1.3  1999/09/30 04:57:28  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

/* Function declarations
*/
static void	load_rec (LLIST *, int, int *, char *);
static int DecToDouble ( dec_t *dp, double * dblp);

int
dbfind (
 const char *filename,
 int flag,
 const char *value,
 int *_length,
 char *recbuf)
{
	int	i, j;
	int	err;
	int	find_flag = flag;
	int	find_type;
	int	init_find = FALSE;
	int	lock_on = (flag >= LOCK);
	int	c_type;
	int	c_leng;
	LLIST	*tptr;
	long	*lptr;

	/*
	 * find node for table		
	 */
	if (!(tptr = _GetNode (filename)))
		return (6006);

	/*
	 *	Indicate lock-type on node
	 */
	tptr -> lockmode = lock_on ? WrLock : NoLock;

	/*
	 * finding with lock ???		
	 */
	if (lock_on)
		find_flag -= LOCK;
	/*
	 * convert 3.3 flag to isam flag	
	 */
	switch (find_flag)
	{
	case EQUAL		:
	case COMPARISON	:
		find_type = ISEQUAL;
		init_find = TRUE;
		break;

	case FIRST		:
		find_type = ISFIRST;
		break;

	case LAST		:
		find_type = ISLAST;
		break;

	case NEXT		:
		find_type = ISNEXT;
		break;

	case PREVIOUS	:
		find_type = ISPREV;
		break;

	case GTEQ		:
		find_type = ISGTEQ;
		init_find = TRUE;
		break;

	case GREATER	:
		find_type = ISGREAT;
		init_find = TRUE;
		break;

	case CURRENT	:
		find_type = ISCURR;
		break;

	case LT		:
		/*
		 * LT calls position the current record at the last record
		 *
		 * LT calls translate to GTEQ + PREVIOUS
		 *	- GTEQ call is used to handle partial matches
		 */
		if ((err = dbfind (filename, GTEQ, value, _length, recbuf)))
		{
			return 
			(
				dbfind 
				(
					filename, 
					LAST + (lock_on ? LOCK : 0),	
					value, 
					_length, 
					recbuf
				)
			);
		}
		return 
		(
			dbfind 
			(
				filename,
				PREVIOUS + (lock_on ? LOCK : 0),/* reapply lock if necessary */
				value, 
				_length,
				recbuf
			)
		);

	case LTEQ	:
		/*
		 * LTEQ calls position the current record at the last record
		 * of the search set
		 *
		 * LTEQ calls translate to GREATER + PREVIOUS
		 */
		if ((err = dbfind (filename, GREATER, value, _length, recbuf)))
		{
			return 
			(
				dbfind 
				(
					filename, 
					LAST + (lock_on ? LOCK : 0),	
					value, 
					_length, 
					recbuf
				)
			);
		}
		return 
		(
			dbfind 
			(
				filename,
				PREVIOUS + (lock_on ? LOCK : 0),/* reapply lock if necesarry */
				value,
				_length,
				recbuf
			)
		);

	default:
		return (-1);
	}
	/*
	 * add locking if required	
	 */
	if (lock_on)
		find_type += ISLOCK;
	/*
	 * initialisation required	
	 */
	if (init_find && value)
	{
		/*
		 * If the key consists of 0 parts, we MUST
		 * assume that NO INDEX has been selected
		 *
		 * The supplied hash-value is taken as the rowid
		 */
		if (tptr -> _key.k_nparts == 0)
		{
			lptr = (long *) value;
			isrecnum = *lptr;
		}

		/*
		 * Can only have 1 part to the key	
		 */
		if (tptr -> _key.k_nparts == 1)
		{
			c_type = _c_type ((int) tptr -> _key.k_part [0].kp_type);
			c_leng = tptr -> _key.k_part [0].kp_type == SQLCHAR ?
				tptr -> _key.k_part [0].kp_leng :
				_c_size ((int) tptr -> _key.k_part [0].kp_type);
			_load_isam 
			(
				c_type,
				tptr,
				(int) tptr -> _key.k_part [0].kp_start,
				c_leng,
				value
			);
		}
	}
	/*
	 * initialise fields in isam
	 */
	for (j = 0; !value && init_find && j < tptr -> _key.k_nparts; j++)
	{
		/*
		 * go through view		
		 */
		for (i = 0; i < tptr -> _no_fields; i++)
		{
			/*
			 * offset in isam == offset from key
			 */
			if (tptr -> _start [i] == tptr -> _key.k_part [j].kp_start)
			{
				_load_isam 
				(
					tptr -> _view [i].vwtype,
					tptr,
					tptr -> _start [i],
					tptr -> _view [i].vwlen,
					recbuf + tptr -> _view [i].vwstart
				);
			}
		}
	}
	/*
	 * Perform the find.
	 */
	if (isread (tptr -> _fd, tptr -> _buffer, find_type))
		return (iserrno);

	tptr -> _recno = isrecnum;
	/*
	 * put data into structure	
	 */
	for (*_length = 0, i = 0; i < tptr -> _no_fields; i++)
		load_rec(tptr,i,_length,recbuf);
	return(0);
}

/*
 * Put data into the ALL record	
 */
static void
load_rec (
	LLIST	*tptr,
	int		i,
	int		*_length,
	char	*recbuf)
{
	short	nlflg;

	*_length += _align (*_length, tptr -> _view [i].vwtype);
	switch (tptr -> _view [i].vwtype)
	{
	case	CHARTYPE:
		sprintf 
		(
			recbuf + tptr -> _view [i].vwstart,
			"%-*.*s",
			tptr -> _view [i].vwlen,
			tptr -> _view [i].vwlen,
			tptr -> _buffer + tptr -> _start [i]
		);
		*_length += (tptr -> _view [i].vwlen + 1);
		break;

	case	INTTYPE:
	{
		int	v = ldint (tptr -> _buffer + tptr -> _start [i]);

		if (v == INTNULL)
			v = 0;
		*(int *) (recbuf + tptr -> _view [i].vwstart) = v;
		*_length += sizeof (int);
		break;
	}

	case	LONGTYPE:
	case	SERIALTYPE:
	case	DATETYPE:
	case	EDATETYPE:
	case	YDATETYPE:
	{
		long	v = ldlong (tptr -> _buffer + tptr -> _start [i]);

		if (v == LONGNULL)
			v = 0;
		*(long *) (recbuf + tptr -> _view [i].vwstart) = v;
		*_length += sizeof (long);
		break;
	}

	case	FLOATTYPE:
	{
		float	v = (float) ldfltnull (tptr -> _buffer + tptr -> _start [i], &nlflg);
		*(float *) (recbuf + tptr -> _view [i].vwstart) = v;
		*_length += sizeof(float);
		break;
	}

	case	DOUBLETYPE:
	{
		double	v = lddblnull (tptr -> _buffer + tptr -> _start [i], &nlflg);

		*(double *) (recbuf + tptr -> _view [i].vwstart) = v;
		*_length += sizeof (double);
		break;
	}

	case	MONEYTYPE:
	{
		/*
		 * Money types are extracted as dec_t (as dollars), but used
		 * as double (as cents) within application code
		 */
		dec_t	d;
		double	v = 0;

		if (lddecimal (
				tptr -> _buffer + tptr -> _start [i],
				DECLENGTH (tptr -> _view [i].vwlen),
				&d) ||
			d.dec_pos == DECPOSNULL)
		{
			/*
			 *	Overflow/Underflow/NULL value
			 */
			v = 0;					/* use a safe value */
		}
		else
		{
			DecToDouble (&d, &v);	/* convert to double */
			v *= 100;				/* convert to cents */
		}
		*(double *) (recbuf + tptr -> _view [i].vwstart) = v;
		*_length += sizeof (double);
		break;
	}
	}
}

/*
 *	Use internal conversion routines, as Informix 7.2+ has managed
 *	to stuff this up.
 */
static int
DecToReal (
	dec_t 	*dp,
	double 	*dblp,
	register int valid)
{
	register char	*digits = dp->dec_dgts;
	double	dbl;

	if (valid > dp->dec_ndgts)	/* can't use more digits than exist */
		valid = dp->dec_ndgts;

	dbl = 0.0;			/* initialize value */

	/*
	 * convert the mantissa
	 */
	while (valid-- > 0)		/* copy mantissa */
	dbl = (dbl + digits[valid]) / 100.0;

	if (dp->dec_pos == 0)
		dbl = -dbl;		/* set sign of mantissa */

	/*
	 * convert the exponent
	 */
	if (dp->dec_exp > 0)
	{
		int i = dp->dec_exp;

		while (i--)
			dbl *= 100.0;
	}
	else
		if (dp->dec_exp < 0)
		{
			int i = dp->dec_exp;

			while (i++)
				dbl /= 100.0;
		}

	/*
	 * copy value to user's area and you're done
	 */
	*dblp = dbl;

	return (EXIT_SUCCESS);
}

/*
 * dectodbl:  convert internal dec_t to native double precision floating point
 */
static int
DecToDouble (
	dec_t 	*dp,
	double 	*dblp)
{
	return DecToReal (dp, dblp, 16);
}

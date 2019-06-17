#ident	"$Id: tstamp.c,v 5.0 2001/06/19 07:07:43 cha Exp $"
/*
 *	Time stamp routines
 *
 *******************************************************************************
 *	$Log: tstamp.c,v $
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/09/30 04:57:29  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

/*
 *	Magic stuff
 */
static char	*TStampColName	= "timestamp";

/*
 *	External interface
 */
void
_TStampCheck (
 const char *table)
{
	/*
	 *	Look thru' the columns of the table to look for one matching
	 *	"TStampColName" and is of type SQLINT (ie a long)
	 */
	int		offset;
	LLIST	*node, *cols;

	if (!(node = _GetNode (table)) ||
		!(cols = _GetSysNode (SYS_COL)))
		return;						/* forget about internal error hassles */

	/*
	 *	Set up read for columns
	 *	- tabid + colno
	 */
	stlong (node -> _tabid, cols -> _buffer + cols -> _key.k_part [0].kp_start);
	stint (0, cols -> _buffer + cols -> _key.k_part [1].kp_start);

	if (isread (cols -> _fd, cols -> _buffer, ISGTEQ))
		return;						/* read failed */

	/*
	 *	Look thru' columns for the table
	 */
	offset = 0;
	while (ldlong (cols -> _buffer + cols -> _key.k_part [0].kp_start) ==
		node -> _tabid)
	{
		int		coltype;
		char	colname [NAME_LEN + 1];

		/* extract column name and type
		 */
		ldchar (cols -> _buffer + SYSCOL_COLNAME, NAME_LEN, colname);
		colname [NAME_LEN] = '\0';
		coltype = ldint (cols -> _buffer + SYSCOL_COLTYPE) & SQLTYPE;

		/*	Verify name and type
		 */
		if (!strcmp (clip (colname), TStampColName))
		{
			if (coltype == SQLINT)
				node -> timestamp = offset;		/* indicate presence */
			return;
		}

		/* increment offset into the buffer
		 */
		offset += _col_len (coltype,
			ldint (cols -> _buffer + SYSCOL_COLLENGTH));

		if (isread (cols -> _fd, cols -> _buffer, ISNEXT))
			break;
	}
}

void
_TStampIt (
 const char *table)
{
	LLIST	*node = _GetNode (table);

	if (!node ||				/* internal error */
		node -> timestamp < 0)	/* no timestamp field */
		return;					/* forget about internal error hassles */

	/*	Write as timestamp long into offset indicated
	 */
	stlong (time (NULL), node -> _buffer + node -> timestamp);
	isrewrec (node -> _fd, node -> _recno, node -> _buffer);
}

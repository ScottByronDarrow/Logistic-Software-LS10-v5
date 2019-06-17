#ident	"$Id: dbfield.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbfield.c,v $
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
 *	Revision 1.3  1999/09/30 04:57:27  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

int
dbfield (
	const 	char 	*filename,
	const 	char 	*fldname,
	short 	int 	*ftype,
	short 	int 	*flen,
	short 	int 	*fperms)
{
	int	i, err;
	int	offset;
	int	found = 0;
	int	sql_type;
	char	fieldname[20];
	LLIST	*sptr;
	LLIST	*tptr;
	/*
	 * find node for table		
	 */
	if (!(tptr = _GetNode (filename)))
		return(6006);

	sprintf(fieldname,"%-18.18s",fldname);
	/*
	 * perform read on sysindexes	
	 */
	sptr = _GetSysNode (SYS_IDX);
	stchar (fldname,
		sptr -> _buffer + sptr -> _key.k_part [0].kp_start,
		sptr -> _key.k_part [0].kp_leng);
	stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
	err = isread (sptr -> _fd, sptr -> _buffer, ISEQUAL);
	/*
	 * not an index or not composite	
	 */
	offset = 28 + SIZINT;
	if (err || ldint (sptr -> _buffer + offset + SIZSMINT) <= 0)
	{
		/*
		 * read columns			
		 */
		sptr = _GetSysNode (SYS_COL);
		stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [0].kp_start);
		stint (0, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
		err = isread (sptr -> _fd, sptr -> _buffer, ISGTEQ);
		while (!err && ldlong (sptr -> _buffer + 18) == TABID)
		{
			/*
			 * column found			
			 */
			if (!strncmp (sptr -> _buffer, fieldname, 18))
			{
				found = 1;
				break;
			}
			err = isread (sptr -> _fd, sptr -> _buffer, ISNEXT);
		}
		/*
		 * find failed			
		 */
		if (!found)
			return(6007);
		sql_type = (short) ldint (sptr -> _buffer + 18 + SIZINT + SIZSMINT);
		*ftype = _c_type(sql_type);
		*flen = (short) ldint (sptr -> _buffer + 18 + SIZINT + SIZSMINT + SIZSMINT);
	}
	else
	{
		/*
		 * composite			
		 */
		*ftype = (short) COMPOSTYPE;
		for (i = 0; ldint (sptr -> _buffer + offset) > 0;i++)
			offset += SIZSMINT;
		*flen = i;
	}
	*fperms = (short) PERM_ALL;
	return(0);
}

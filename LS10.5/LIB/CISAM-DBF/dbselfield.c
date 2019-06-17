#ident	"$Id: dbselfield.c,v 5.1 2001/08/20 23:07:46 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbselfield.c,v $
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
dbselfield (
	const 	char	*filename,
	const 	char	*idx_name,
	int 	flag)
{
	int	i = 0, j = 0;
	int	err;
	int	col_no;
	int	offset = 0;
	int	sql_type;
	int	found;
	char	fieldname[19];
	LLIST	*tptr;
	LLIST	*sptr;

	/*
	 * find filename node		
	 */
	if (!(tptr = _GetNode (filename)))
		return (6006);

	/*
	 * select new index
	 */
	KEYDEF.k_nparts = 0;
	switch (flag)
	{
	/*
	 * use index specified		
	 */
	case	ACCKEYED:
		sptr = _GetSysNode (SYS_IDX);
		/*
		 * initialise search values	
		 */
		stchar (idx_name,
			sptr -> _buffer + sptr -> _key.k_part [0].kp_start,
			sptr -> _key.k_part [0].kp_leng);
		stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
		/*
		 * perform read on sysindexes
		 */
		if (isread (sptr -> _fd, sptr -> _buffer, ISEQUAL))
			return (iserrno);

		/*
		 * U(nique, D(uplicates.		
		 */
		KEYDEF.k_flags =  (sptr -> _buffer [30] == 'U') ? ISNODUPS  : ISDUPS;
		KEYDEF.k_flags += (sptr -> _buffer [31] == 'C') ? ISCLUSTER : 0;
		KEYDEF.k_flags += DCOMPRESS;
		KEYDEF.k_nparts = 0;
		/*
		 * initialise composite parts	
		 */
		for (i = 0;i < NPARTS;i++)
		{
			/*
			 * column no of composite part	
			 */
			col_no = ldint (sptr -> _buffer + 32 + (i * INTSIZE));
			if (col_no > 0)
				KEYDEF.k_nparts++;
			KEYPART(i).kp_start = 0;
			KEYPART(i).kp_leng = col_no;
			KEYPART(i).kp_type = -1;
		}
		sptr = _GetSysNode (SYS_COL);
		/*
		 * initialise search values	
		 */
		stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [0].kp_start);
		stint (0, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
		/*
		 * perform read on syscolumns	
		 */
		if ((err = isread (sptr -> _fd, sptr -> _buffer, ISGTEQ)))
			return (iserrno);

		/*
		 * read info about components of the key. from syscolumns	
		 */
		for (i = 0; !err && ldlong (sptr -> _buffer + 18) == TABID && i < KEYDEF.k_nparts;)
		{
			/*
			 * get col_no & type		
			 */
			col_no = ldint (sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
			sql_type = ldint (sptr -> _buffer + 24) & 0377;
			/*
			 * is field an index component ?	
			 */
			for (j = 0,found = 0;!found && j < KEYDEF.k_nparts;j++)
			{
				if (KEYPART(j).kp_leng == col_no && KEYPART(j).kp_type == -1)
				{
					found = 1;
					break;
				}
			}
			/*
			 * affirmative
			 */
			if (found)
			{
				/*
				 * get column name from syscolumns
				 */
				ldchar (sptr -> _buffer, 18, fieldname);

				/*
				 * check if index field(s) are in view
				 */
				if (_check_view(clip(fieldname),tptr) == -1)
					return((KEYDEF.k_nparts == 1) ? 6007 : 6052);
				/*
				 * setup new index	
				 */
				KEYPART(j).kp_start = offset;
				KEYPART(j).kp_leng = ldint (sptr -> _buffer + 26);

				/*
				 * The following 2 lines added 17/03 to
				 * allow for MONEY type fields to be a	
				 * part of the index.			
				 */
				if (sql_type == SQLMONEY)	/* MONEY */
					KEYPART(j).kp_leng = DECLENGTH (KEYPART(j).kp_leng);
				KEYPART(j).kp_type = _c_type(sql_type);
				i++;
			}
			/*
			 * Calculate the current offset	
			 */
			offset += _col_len(sql_type, ldint (sptr -> _buffer + 26));

			err = isread (sptr -> _fd, sptr -> _buffer, ISNEXT);
		}
		break;

	/*
	 *  Use Sequential or Primary	
	 */
	case	ACCSEQUENTIAL:
	case	ACCPRIMARY:
		KEYDEF.k_nparts = 0;
		break;

	default:
		return(6014);
	}
	/*
	 * initialise index		
	 */
	isstart (tptr -> _fd, &tptr -> _key, 0, tptr -> _buffer, ISFIRST);

	/*
	 * ignore errors as these are probably due to find failure	
	 */
	return(0);
}

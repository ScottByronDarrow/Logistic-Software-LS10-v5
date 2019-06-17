#ident	"$Id: dbncomposite.c,v 5.1 2001/08/20 23:07:45 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbncomposite.c,v $
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

int
dbncomposite (
	short int ncomp,
	const char *filename,
	const char *compname,
	char * partname,
	short int *ptype,
	short int *plen,
	short int *pperms)
{
	int	offset;
	int	nparts;
	int	col_no;
	int	sql_type;
	LLIST	*sptr;
	LLIST	*tptr;
	/*
	 * find node for table		
	 */
	if (!(tptr = _GetNode (filename)))
		return (6006);

	/*
	 * perform read on sysindexes	
	 */
	sptr = _GetSysNode (SYS_IDX);
	stchar (compname,
		sptr -> _buffer + sptr -> _key.k_part [0].kp_start,
		sptr -> _key.k_part [0].kp_leng);
	stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
	if (isread (sptr -> _fd,  sptr -> _buffer, ISEQUAL))
		return (iserrno);

	/*
	 * check ncomp parameter		
	 */
	offset = 28 + SIZINT;
	for (nparts = 0; ldint( sptr -> _buffer + offset) > 0; nparts++)
		offset += SIZSMINT;
	if (ncomp < 0 || ncomp > nparts)
		return(6044);
	/*
	 * noty composite		
	 */
	if (nparts <= 1)
		return(6043);
	/*
	 * perform read on syscolumns	
	 */
	col_no = ldint (sptr -> _buffer + 32 + ((int) ncomp * INTSIZE));
	sptr = _GetSysNode (SYS_COL);
	stlong (TABID, sptr -> _buffer + sptr -> _key.k_part [0].kp_start);
	stint (col_no, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
	if (isread (sptr -> _fd, sptr -> _buffer, ISEQUAL))
		return (iserrno);

	/*
	 * setup return values		
	 */
	sprintf (partname, "%-18.18s", sptr -> _buffer);
	clip (partname);
	sql_type = ldint (sptr -> _buffer + 24) & 0377;
	*ptype = (short) _c_type(sql_type);
	*plen = (short) ldint (sptr -> _buffer + 26);
	*pperms = (short) PERM_ALL;
	return(0);
}

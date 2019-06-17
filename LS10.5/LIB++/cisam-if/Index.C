#ident	"$Id: Index.C,v 5.0 2001/06/19 08:17:30 cha Exp $"
/*
 *	Index representation
 *
 *******************************************************************************
 *	$Log: Index.C,v $
 *	Revision 5.0  2001/06/19 08:17:30  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.1  1996/07/30 00:57:43  jonc
 *	Added #ident directive
 *
 *	Revision 2.0  1996/02/13 03:34:51  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:09  jonc
 *	Initial C++ CISAM-Interface entry
 *
 */
#include	<assert.h>
#include	"Index.h"

short
Index::Part (
 unsigned	i) const
{
	assert (i < MAX_IDX_PARTS);

	return (parts [i]);
}

const struct keydesc &
Index::Desc () const
{
	return (desc);
}

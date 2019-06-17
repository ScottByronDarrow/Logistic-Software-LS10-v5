#ident	"$Id: GenericFilter.C,v 5.0 2001/06/19 08:22:46 robert Exp $"
/*
 *	Generic filter (ie use with |)
 *
 *******************************************************************************
 *	$Log: GenericFilter.C,v $
 *	Revision 5.0  2001/06/19 08:22:46  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:08  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:11  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<osdeps.h>

#include	"GenericFilter.h"

GenericFilter::GenericFilter (
 const char *	outputstr)
{
	if (outputstr)
	{
		if (outputstr [0] == '|')
			out = popen (outputstr + 1, "w");
		else
			out = fopen (outputstr, "w");
	}
}

void
GenericFilter::UsePitch (
 int)
{
	/*
	 *	ignore
	 */
}

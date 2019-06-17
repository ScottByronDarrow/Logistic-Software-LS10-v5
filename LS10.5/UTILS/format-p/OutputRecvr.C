#ident	"$Id: OutputRecvr.C,v 5.0 2001/06/19 08:22:46 robert Exp $"
/*
 *
 *	Abstract class for Output receivers
 *
 *******************************************************************************
 *	$Log: OutputRecvr.C,v $
 *	Revision 5.0  2001/06/19 08:22:46  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/22 06:02:10  scott
 *	Updated to add P2 string.
 *	
 *	Revision 3.0  2000/10/10 12:24:08  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:12  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<osdeps.h>

#include	"OutputRecvr.h"

OutputRecvr::OutputRecvr () :
	out (NULL)
{
}

OutputRecvr::~OutputRecvr ()
{
	if (out)
	{
		fputs (p_dinit, out);		//	Initialization string
		pclose (out);
	}
}

bool
OutputRecvr::Usable () const
{
	if (out)
		return true;
	return false;
}

void
OutputRecvr::Advance (
 int	lines)
{
	for (int i = 0; i < lines; i++)
		fputc ('\n', out);
}

void
OutputRecvr::Write (
 const char *	str)
{
	fprintf (out, "%s\n", str);
}

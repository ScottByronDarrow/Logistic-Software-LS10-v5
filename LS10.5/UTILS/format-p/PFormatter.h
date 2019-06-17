#ifndef	PFormatter_h
#define	PFormatter_h
/*	$Id: PFormatter.h,v 5.0 2001/06/19 08:22:47 robert Exp $
 *
 *	Emulates interface to pformat
 *
 *******************************************************************************
 *	$Log: PFormatter.h,v $
 *	Revision 5.0  2001/06/19 08:22:47  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 05:01:11  scott
 *	Updated to allow for local printing.
 *	
 *	Revision 3.0  2000/10/10 12:24:13  gerry
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
#include	<String.h>

#include	"OutputRecvr.h"

class Options;

class PFormatter :
	public OutputRecvr
{
	private:
		String		p_init,
					p_10pitch,
					p_12pitch,
					p_16pitch;

	public:
		PFormatter (const Options &);

		virtual void	UsePitch (int);
};

#endif	//	PFormatter_h

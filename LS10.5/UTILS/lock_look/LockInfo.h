#ifndef	LockInfo_h
#define	LockInfo_h
/*	$Id: LockInfo.h,v 5.0 2001/06/19 08:22:58 robert Exp $
 *
 *	Row locks on a Table
 *
 *******************************************************************************
 *	$Log: LockInfo.h,v $
 *	Revision 5.0  2001/06/19 08:22:58  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:18  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:21  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/07/20 00:40:11  jonc
 *	Adopted Pinnacle V10 lockLook.
 *	
 */
#include	<sys/types.h>
#include	<String.h>

class LockInfo
{
	public:
		String	table;
		pid_t	pid;
		long	recno;
		String	user,
				command;

	public:
		LockInfo ();
		LockInfo (const char * table, pid_t, long);

		bool operator == (const LockInfo &);
};

#endif

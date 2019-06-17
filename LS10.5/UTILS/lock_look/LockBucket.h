#ifndef	LockBucket_h
#define	LockBucket_h
/*	$Id: LockBucket.h,v 5.0 2001/06/19 08:22:58 robert Exp $
 *
 *	List of active locks
 *
 *******************************************************************************
 *	$Log: LockBucket.h,v $
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
#include	<CArray.h>

#include	"LockInfo.h"

class LockBucket
{
	private:
		CArray <LockInfo>	bucket;

	public:
		bool				Count () const;
		const LockInfo &	Elem (int) const;

		void				Add (const LockInfo &);
		void				BuildProcessInfo ();
};

#endif

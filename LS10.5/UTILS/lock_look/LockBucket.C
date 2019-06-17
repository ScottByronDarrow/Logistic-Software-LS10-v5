#ident	"$Id: LockBucket.C,v 5.0 2001/06/19 08:22:58 robert Exp $"
/*
 *	List of active locks
 *
 *******************************************************************************
 *	$Log: LockBucket.C,v $
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
#include	<stdio.h>

#include	<osdeps.h>

#include	"LockBucket.h"

bool
LockBucket::Count () const
{
	return bucket.Count ();
}

void
LockBucket::Add (
 const LockInfo & info)
{
	/*
	 *	Make sure we only add this once
	 */
	for (int i = 0; i < bucket.Count (); i++)
		if (bucket.Elem (i) == info)
			return;

	bucket.Add (info);
}

const LockInfo &
LockBucket::Elem (
 int i) const
{
	return bucket.Elem (i);
}

void
LockBucket::BuildProcessInfo ()
{
	/*
	 *	We extract the command information for `ps'; which
	 *	makes it pretty portable, but slightly slower
	 */
	String	pidlist;
	char	line [512];

	for (int i = 0; i < bucket.Count (); i++)
	{
		sprintf (line, "%d", bucket.Elem (i).pid);
		if (pidlist.empty ())
			pidlist = line;
		else
			cat (pidlist, ",", line, pidlist);
	}

	sprintf (line, "ps -p %s -o pid= -o ruser= -o args=", pidlist.chars ());

	/*
	 */
	FILE *	ps = popen (line, "r");

	if (!ps)
		return;

	while (fgets (line, sizeof line, ps))
	{
		int		pid = 0;
		char	user [512],
				cmd [512];

		if (sscanf (line, "%d %s %[^\n]", &pid, user, cmd) == 3)
		{
			/*
			 *	Match the command against the pid
			 *	Note that a pid may appear several times
			 *	in the lock-list.
			 */
			for (int i = 0; i < bucket.Count (); i++)
			{
				LockInfo &	elem = bucket.Elem (i);

				if (elem.pid == pid)
				{
					elem.user = user;
					elem.command = cmd;
				}
			}
		}
	}

	pclose (ps);
}

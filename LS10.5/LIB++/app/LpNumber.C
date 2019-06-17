#ident	"$Id: LpNumber.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	List of Printers.
 *
 *	Cross-reference with prntype and LP_SECURE
 *
 *******************************************************************************
 *	$Log: LpNumber.C,v $
 *	Revision 5.0  2001/06/19 08:16:40  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 01:02:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:59:18  scott
 *	*** empty log message ***
 *	
 *	Revision 3.0  2000/10/12 13:40:10  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from V10)
 *	
 */
#include	<assert.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<osdeps.h>
#include	<liberr.h>

#include	<LpNumber.h>
#include	<UnixEnv.h>

/*
 *	Magic stuff
 */
static const char *	ProgPath	= "PROG_PATH",		// Unix Env
				  *	LogName		= "LOGNAME";

/*
 *	Local classes
 */
class LpSecureEntry
{
	public:
		String	name,
				type,
				queue;

	public:
		LpSecureEntry (
		 const char *	n = NULL,
		 const char *	t = NULL,
		 const char *	q = NULL) :
			name (n),
			type (t),
			queue (q)
		{
		}
};

/*
 * Construction
 */
LpNumber::LpNumber ()
{
	char	fileName [256],
			lineBuf [256];

	/*
	 *	Query environment
	 */
	assert (getenv (ProgPath));
	assert (getenv (LogName));
	String	logname = (const char *) UnixEnv (LogName),
			progPath = (const char *) UnixEnv (ProgPath);

	/*
	 *	What happens is:
	 *		- we load the LP_SECURE file
	 *		- cross ref this against allowed queues in the "prntype" file
	 */

	/*
	 *	Load LP_SECURE file
	 */
	FILE *	lpSecure;
	CArray <LpSecureEntry>	secEntries;

	sprintf (fileName, "%s/BIN/MENUSYS/LP_SECURE", progPath.chars ());
	if (!(lpSecure = fopen (fileName, "r")))
	{
		fprintf (stderr, "Failed to open %s", fileName);
		exit (EXIT_FAILURE);
	}

	while (fgets (lineBuf, sizeof (lineBuf), lpSecure))
	{
		char	name [80],
				type [80],
				queue [80];

		if (sscanf (lineBuf, "%s %s %s", name, type, queue) != 3)
			continue;

		secEntries.Add (LpSecureEntry (name, type, queue));
	}
	fclose (lpSecure);

	/*
	 *	Load list of of valid printers for the current user from 'prntype'
	 */
	FILE *	lpQueues;
	int		lpLine = 0;

	sprintf (fileName, "%s/BIN/MENUSYS/prntype", progPath.chars ());
	if (!(lpQueues = fopen (fileName, "r")))
	{
		fprintf (stderr, "Failed to open %s", fileName);
		exit (EXIT_FAILURE);
	}

	while (fgets (lineBuf, sizeof (lineBuf), lpQueues))
	{
		char	pType [128],
				qName [128],
				pDesc [128];

		if (sscanf (lineBuf, "%[^\t]\t%[^\t]\t%[^\n]", pType, qName, pDesc)
				!= 3)
		{
			continue;
		}

		lpLine++;

		/*
		 *	Validate line against Security Entries
		 */
		bool	secured = false;

		for (int i = 0; i < secEntries.Count (); i++)
		{
			const char *	openSecure = "*";
			const LpSecureEntry &	e = secEntries.Elem (i);

			/*
			 *	Check name, type and queue
			 */
			if ((e.name == openSecure || e.name == logname) &&
				(e.type == openSecure || e.type == pType) &&
				(e.queue == openSecure || e.queue == qName))
			{
				secured = true;
				break;
			}
		}

		/*
		 *	Add to list of valid lp
		 */
		tagLpInfo	info;

		info.lpno = lpLine;
		info.pType = pType;
		info.qName = qName;
		info.pDesc = pDesc;

		if (secured)
			lpSecured.Add (info);
		else
			lpHidden.Add (info);
	}
	fclose (lpQueues);
}

//======================================
// Validates the logical printer number
// passed to see if the user has access
// to that printer.
//
bool
LpNumber::Valid (
 int	chkLpno) const
{
	int		i;

	for (i = 0; i < lpSecured.Count (); i++)
		if (lpSecured.Elem (i).lpno == chkLpno)
			return true;

	return (false);
}

const tagLpInfo &
LpNumber::LpInfo (
 int	lpno) const
{
	/*
	 *	Look against both the Secured and Hidden lists
	 */
	for (int i = 0; i < lpSecured.Count (); i++)
		if (lpSecured.Elem (i).lpno == lpno)
			return lpSecured.Elem (i);
	for (int i = 0; i < lpHidden.Count (); i++)
		if (lpHidden.Elem (i).lpno == lpno)
			return lpHidden.Elem (i);

	(*app_error_handler) ("LpNumber::LpInfo",
		"Bad lpno %d", lpno);

	return lpSecured.Elem (0);
}

const tagLpInfo &
LpNumber::operator [] (
 int	idx) const
{
	return lpSecured.Elem (idx);
}

/*******************************************************************************
 *
 *	Iterator class
 *
 ******************************************************************************/
LpNumberIter::LpNumberIter (
 const LpNumber &	host) :
	idx (0),
	src (host)
{
}

LpNumberIter::operator bool () const
{
	return (idx >= 0 && idx < src.lpSecured.Count ());
}

const tagLpInfo &
LpNumberIter::lpInfo () const
{
	return (src [idx]);
}

void
LpNumberIter::Reset ()
{
	idx = 0;
}

void
LpNumberIter::operator ++ (
 int)
{
	if (idx < src.lpSecured.Count ())
		idx++;
}

void
LpNumberIter::operator -- (
 int)
{
	if (idx >= 0)
		idx--;
}

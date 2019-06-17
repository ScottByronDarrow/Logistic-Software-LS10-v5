#ident	"$Id: SystemTime.c,v 5.0 2001/06/19 06:59:13 cha Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: SystemTime.c,v $
 *	Revision 5.0  2001/06/19 06:59:13  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:34  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:18  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:12  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1  1999/12/06 01:23:42  scott
 *	Replacement for Ctime due to conflicts with VisualC++
 *	
 *	Revision 1.2  1999/09/13 06:20:46  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1  1999/07/12 00:25:31  scott
 *	Added another function from v10
 *	
 *	Revision 1.2  1998/09/18 04:03:21  kirk
 *	Return a value from SystemTime.?
 *	
 *	Revision 1.1  1998/07/13 01:00:28  jonc
 *	Y2K changes, mainly done by colin.
 *
 */
#include	<time.h>
#include	<string.h>

#include	<SystemTime.h>

const char *
SystemTime (void)
{
	static char buff [100];

	time_t now = time (NULL);
	strcpy (buff, ctime (&now));
	buff [strlen(buff) - 1] = 0;
	return (buff);
}

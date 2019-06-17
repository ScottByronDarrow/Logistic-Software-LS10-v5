/*
 *
 *
 *******************************************************************************
 *	$Log: StringToDate.c,v $
 *	Revision 5.1  2001/08/06 22:40:53  scott
 *	RELEASE 5.0
 *	
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
 *	Revision 1.4  1999/11/19 04:17:46  scott
 *	Updated to add further validations on valid days.
 *	
 *	Revision 1.3  1999/09/29 06:08:57  scott
 *	Updated to place checks on month and day.
 *	
 *	Revision 1.2  1999/09/13 06:20:46  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.1  1998/07/13 01:00:29  jonc
 *	Y2K changes, mainly done by colin.
 *	
 */
#include	<stdio.h>
#include	<string.h>

#include	<std_decs.h>

Date
StringToDate (
 const char *	dateStr)
{
	char	*sptr;
	long	dy = 0,
			mn = 0,
			yr = 0,
			vd = 0;

	if ((sptr = getenv ("DBDATE")))
		upshift (sptr);

	if
	(
		sptr &&
		(
			!strncmp (sptr, "YMD", 3) ||
		 	!strncmp (sptr, "Y2MD", 4) ||
		 	!strncmp (sptr, "Y4MD", 4)
		)
	)
	{
		sscanf (dateStr, "%ld/%ld/%ld", &yr, &mn, &dy);
	}
	else if (sptr && !strncmp (sptr, "MDY", 3))
	{
		sscanf (dateStr, "%ld/%ld/%ld", &mn, &dy, &yr);
	}
	else
	{
		/*
		 *	Default format (DMY)
		 */
		sscanf (dateStr, "%ld/%ld/%ld", &dy, &mn, &yr);
	}
	if (dy == 0 && mn == 0 && yr == 0)
		return (EXIT_SUCCESS);

	if (dy == 0 || mn == 0)
		return (-1);
	
	/*-----------------------------------------------
	| Added the following validation as routine did |
	| not check for valid days for valid months.    |
	-----------------------------------------------*/
	vd	=	(long) DaysInMonthYear (mn, yr);
	if (dy > vd)
		return (-1);

	if (mn > 12)
		return (-1);

	/*
	 *	Kludge for small years
	 */
	if (yr < 1000)
		yr += (yr > 50) ? 1900 : 2000;


	return DMYToDate (dy, mn, yr);
}

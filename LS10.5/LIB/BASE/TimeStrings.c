#ident	"$Id: TimeStrings.c,v 5.0 2001/06/19 06:59:13 cha Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: TimeStrings.c,v $
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
 *	Revision 1.4  1999/09/14 03:41:00  scott
 *	Anci mods.
 *	
 *	Revision 1.3  1999/09/13 06:20:46  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1  1999/07/06 03:08:30  scott
 *	Added time routine to 9.10 library from 10.0
 *	
 *	Revision 1.1  1998/07/13 01:00:30  jonc
 *	Y2K changes, mainly done by colin.
 *	
 */
#include	<stdio.h>
#include	<time.h>
#include	<string.h>

#include	<TimeStrings.h>

char *
TimeHHMM (void)
{
	static char buff [100];
	time_t now = time (NULL);
	struct tm *tme = localtime (&now); 

	sprintf (buff, "%02d:%02d", tme -> tm_hour, tme -> tm_min); 

	return buff;
}

char *
TimeHHMMSS (void)
{
	static char buff [100];
	time_t now = time (NULL);
	struct tm *tme = localtime (&now); 

	sprintf (buff, "%02d:%02d:%02d",
		tme -> tm_hour, tme -> tm_min, tme -> tm_sec); 

	return buff;
}

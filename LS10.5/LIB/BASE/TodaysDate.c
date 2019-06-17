#ident	"$Id: TodaysDate.c,v 5.1 2001/08/06 22:40:53 scott Exp $"
/*
 *
 *******************************************************************************
 *	$Log: TodaysDate.c,v $
 *	Revision 5.1  2001/08/06 22:40:53  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 06:59:36  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:38  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:25  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:17  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.5  1999/09/14 00:06:06  scott
 *	Updated for better date usage.
 *	Updated to use ver10 sleeper
 *	Updated to use ver10 tty_slot
 *	
 *	Revision 1.2  1999/08/28 00:48:32  scott
 *	Updated from Version 10.
 *	
 *	Revision 1.2  1998/05/15 03:22:06  jonc
 *	Altered to use new date functions.
 *	
 */
#include	<std_decs.h>

Date
TodaysDate (void)
{
	time_t		now = time (NULL);
	struct tm *	tnow = localtime (&now);

	return DMYToDate (
				tnow -> tm_mday, tnow -> tm_mon + 1, tnow -> tm_year + 1900);
}

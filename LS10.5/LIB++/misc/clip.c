#ident	"$Id: clip.c,v 5.0 2001/06/19 08:19:07 cha Exp $"
/*
 *	Clipping routines
 *
 *******************************************************************************
 *	$Log: clip.c,v $
 *	Revision 5.0  2001/06/19 08:19:07  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:12  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:45  jonc
 *	Version 10 start
 *
 *	Revision 2.1  1996/07/30 00:53:02  jonc
 *	Added #ident directive
 *
 *	Revision 2.0  1996/02/13 03:34:58  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:48  jonc
 *	Initial C++ Support library
 *
 */
#include	<ctype.h>
#include	<string.h>
#include	<minor.h>

char *
clip (
 char *	str)
{
	size_t	i;

	for (i = strlen (str); i && isspace (str [i - 1]); i--);
	str [i] = '\0';
	return (str);
}

char *
lclip (
 char *	str)
{
	char *	p;

	for (p = str; *p; p++)
		if (!isspace (*p))
		{
			if (str != p)
				strcpy (str, p);
			break;
		}
	return (str);
}

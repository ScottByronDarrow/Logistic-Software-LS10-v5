#ident	"$Id: padout.c,v 5.0 2001/06/19 08:19:07 cha Exp $"
/*
 *	Character padding
 *
 *******************************************************************************
 *	$Log: padout.c,v $
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
 *	Revision 2.1  1996/07/30 00:53:03  jonc
 *	Added #ident directive
 *
 *	Revision 2.0  1996/02/13 03:34:59  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:49  jonc
 *	Initial C++ Support library
 *
 */
#include	<ctype.h>
#include	<string.h>
#include	<minor.h>

char *
padout (
 char *		str,
 unsigned	to,
 int		with)
{
	size_t	len = strlen (str);

	if (len < (size_t) to)
	{
		while (len < to)
			str [len++] = with;
		str [len] = '\0';
	}
	return (str);
}

char *
lpadout (
 char *		str,
 unsigned	to,
 int		with)
{
	size_t	len = strlen (str);

	if (len < to)
	{
		/*
		 *	Move chars up and then pad lead
		 */
		size_t	i,
				diff = to - len;

		for (i = len; i > 0; i--)
			str [i - 1 + diff] = str [i - 1];
		for (i = 0; i < diff; i++)
			str [i] = with;
		str [to] = '\0';
	}
	return (str);
}

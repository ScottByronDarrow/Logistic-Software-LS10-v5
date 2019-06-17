/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| $Id: CheckForSU.c,v 5.0 2002/05/08 01:42:54 scott Exp $
|----------------------------------------------------------------------
| $Source: /usr/LS10/REPOSITORY/LS10.5/INSTALL/bin/src/CheckForSU/CheckForSU.c,v $
-----------------------------------------------------------------------
| $Log: CheckForSU.c,v $
| Revision 5.0  2002/05/08 01:42:54  scott
| CVS administration
|
| Revision 4.1  2001/08/20 23:41:27  scott
| Updated for development related to bullet proofing
|
| Revision 4.0  2001/03/09 02:27:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.1  2001/01/17 08:18:17  scott
| Updated to add new programs/features
|
| Revision 1.1  2001/01/17 08:03:43  scott
| Added to install system
|
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: CheckForSU.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/INSTALL/bin/src/CheckForSU/CheckForSU.c,v 5.0 2002/05/08 01:42:54 scott Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2)
	{
		printf ("FALSE");
		return (EXIT_FAILURE);
	}

	if (getuid () == atoi (argv[1]))
	{
		printf ("TRUE");
		return (EXIT_SUCCESS);
	}

	if (geteuid () == atoi (argv[1]))
	{
		printf ("TRUE");
		return (EXIT_SUCCESS);
	}

	printf ("FALSE");
	return (EXIT_FAILURE);
}

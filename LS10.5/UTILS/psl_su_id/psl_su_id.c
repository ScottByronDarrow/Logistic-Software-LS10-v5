/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| $Id: psl_su_id.c,v 5.1 2001/08/09 09:27:33 scott Exp $
-----------------------------------------------------------------------
| $Log: psl_su_id.c,v $
| Revision 5.1  2001/08/09 09:27:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:23:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:44:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:24:33  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:15:38  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.3  1999/11/16 08:11:37  scott
| Update for warnings due to usage of -Wall flags.
|
 */
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_su_id.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_su_id/psl_su_id.c,v 5.1 2001/08/09 09:27:33 scott Exp $";

#include <pslscr.h>

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

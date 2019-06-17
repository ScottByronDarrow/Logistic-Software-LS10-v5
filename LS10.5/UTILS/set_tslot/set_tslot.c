/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| Obtain a usable slot            
| $Id: set_tslot.c,v 5.1 2001/08/09 09:27:37 scott Exp $
-----------------------------------------------------------------------
| $Log: set_tslot.c,v $
| Revision 5.1  2001/08/09 09:27:37  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:23:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:44:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:24:37  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:15:43  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/01/24 07:12:25  scott
| ?
|
| Revision 1.9  2000/01/21 01:46:48  scott
| Updated for define of main
|
| Revision 1.8  2000/01/21 01:45:54  scott
| Updated to remove code as it exists in library.
|
| Revision 1.7  2000/01/06 00:56:48  scott
| Updated from Alvins version.
|
| Revision 1.6  1999/11/17 07:30:26  scott
| Updated (rewrite) Now uses new routines from Gvision (alvin).
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: set_tslot.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/set_tslot/set_tslot.c,v 5.1 2001/08/09 09:27:37 scott Exp $";

#include	<pslscr.h>

extern	int	forceRead;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	forceRead	=	TRUE;
	printf ("%d\n", ttyslt());
	return EXIT_SUCCESS;
}

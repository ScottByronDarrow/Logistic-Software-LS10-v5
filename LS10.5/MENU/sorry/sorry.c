/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sorry.c        )                                 |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (03/01/88)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      :                                                    |
|   (03/09/1999) :  Ported to ANSI standards.                         |
|                :                                                    |
|                                                                     |
| $Log: sorry.c,v $
| Revision 5.2  2001/08/09 05:13:55  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:09  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:37  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/12/06 01:47:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/09/17 07:27:18  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:43  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:57  scott
| Update to add log + change database names + misc clean up.
|
|====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sorry.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/sorry/sorry.c,v 5.2 2001/08/09 05:13:55 scott Exp $";

#include	<pslscr.h>
#include	<ml_menu_mess.h>
#include	<stdio.h>

int
main (
 int	argc,
 char *	argv [])
{
	init_scr ();
	clear ();
	box (0, 0, 78, 22);
	box (20, 10, 35, 1);

	/*-------------------------------
	| can not exit the menu system. |
	-------------------------------*/
	rv_pr (ML (mlMenuMess161), 25, 11, 1);
	fflush (stdout);
	putchar (BELL); fflush (stdout); sleep (sleepTime);
	putchar (BELL); fflush (stdout); sleep (sleepTime);
	putchar (BELL); fflush (stdout); sleep (sleepTime);
	putchar (BELL); fflush (stdout); 
	return (EXIT_SUCCESS);
}

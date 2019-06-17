/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( strback.c      )                                 |
|  Program Desc  : ( Pleces a Stream Into background.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (12/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      :                                                    |
|   (12/09/97)   : Incorporate multilingual conversion.               |
|   (03/09/1999) : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: strback.c,v $
| Revision 5.1  2001/08/09 05:13:56  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:09:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:38  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:47:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/25 10:24:02  scott
| Updated to remove c++ comment lines and replace with standard 'C'
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
=====================================================================*/
char	*PNAME = "$RCSfile: strback.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/strback/strback.c,v 5.1 2001/08/09 05:13:56 scott Exp $";

#define CCMAIN 

#include	<stdio.h>
#include	<pslscr.h>

#include	<osdefs.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

int
main (
 int	argc,
 char *	argv [])
{
	int	key;

	/*-------------------------------------------
	| Exit if stream is allready in background. |
 	-------------------------------------------*/
	if (isatty(0) == 0)
		return (EXIT_SUCCESS);

	init_scr ();
	clear ();

	print_at (12, 6, ML (mlMenuMess087));
	rv_on ();
	print_at (12, 14, ML (mlMenuMess070));
	rv_off ();
	print_at (12, 21, ML (mlMenuMess088));

	set_tty ();
	/*-------------------------------------------------------------
	| Return background status to chain if Return key is pressed. |
	-------------------------------------------------------------*/
	if ((key = getkey()) != '\r' && key != '\n') 
	{
		clear ();
		print_at (12, 15, ML (mlStdMess035));
		rv_on ();
		print_at (12, 47, ML (mlMenuMess089));
		rv_off ();
		rset_tty ();
		return (27);
	}
	else 
	{
		clear ();
		print_at (12, 15, ML (mlStdMess035));
		rv_on ();
		print_at (12, 52, ML (mlMenuMess071));
		rv_off ();
		rset_tty ();
		return (EXIT_SUCCESS);
	}
}

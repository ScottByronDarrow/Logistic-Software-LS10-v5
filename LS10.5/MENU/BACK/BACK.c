/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( BACK.c         )                                 |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written : (10/05/86)       | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (23/05/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (05/09/97)      | Modified  by : Roanna Marcelino  |
|                                                                     |
|  Comments                                                           |
|  (23/05/92)    : Updated for compatability with Risc/Os 5.0         |
|                : Also, generally tidied up.                         |
|  (05/09/97)    : Modified for Multilingual Coversion.               |
|                                                                     |
| $Log: BACK.c,v $
| Revision 5.1  2001/08/09 05:13:07  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:07:53  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:12  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:47  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:00  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/12/06 01:47:06  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/11/16 09:41:53  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.5  1999/09/17 07:26:48  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/06/15 02:31:40  scott
| update to add log file + change database name etc.
|
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: BACK.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/BACK/BACK.c,v 5.1 2001/08/09 05:13:07 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<ml_menu_mess.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	if (argc < 2)
	{
		print_at (0,0,mlMenuMess093, argv[0]);
		return EXIT_FAILURE;
	}

	execvp (argv[1], argv + 1);
	return EXIT_SUCCESS;
}

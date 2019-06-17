/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( strsave.c      )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (13/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (07.11.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|     (13/08/93) : PSL 9513 Use of <malloc.h>                         |
|     (07.11.94) : instring () removed                                |

	$Log: strsave.c,v $
	Revision 5.0  2001/06/19 06:59:39  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:39  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:29  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:19  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.3  1999/09/23 22:39:29  jonc
	Replaced use of deprecated <malloc.h> with <stdlib.h>
	
=====================================================================*/
#include	<string.h>
#include	<stdlib.h>

/*=====================================================
| Allocate space and save string.                     |
| NCR USED strsave so changed name to avoid problems. |
=====================================================*/
char *
p_strsave (
 char	*str)
{
	char 	*p;

	if ((p = malloc (strlen (str) + 1)))
		strcpy (p, str);

	return (p);	/*  returns NULL upon error  */
}

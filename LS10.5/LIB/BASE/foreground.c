/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( foreground.c  )                                  |
|  Program Desc  : ( Simplistic test to see if current process is   ) |
|                  ( in foreground                                  ) |
|---------------------------------------------------------------------|
|  Date Written  : (13/07/1998)    | Author      : Jonathan Chen      |
-----------------------------------------------------------------------
	$Log: foreground.c,v $
	Revision 5.0  2001/06/19 06:59:16  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:36  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:20  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:13  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.4  1999/11/17 00:57:14  jonc
	Moved for_chk() from DBIF to BASE.
	
=====================================================================*/
#include	<stdio.h>
#include	<sys/types.h>
#include	<unistd.h>

#define	TRUE	1
#define	FALSE	0

int
foreground (void)
{
	if (!isatty (STDIN_FILENO))
		return FALSE;				/* stdin not a terminal */

	if (getppid () == 1)
		return FALSE;				/* parent is the `init' process */

	return (TRUE);
	/*
	 *	Check for process grouping
	 *
	 *	If we're the process group leader, there's a good chance
	 *	that we're in the foreground. However, this requires all
	 *	fork(2) calls to properly configure themselves with
	 *	setpgid(2).

		Disabled for ver9.10

	return getpid () == getpgid (0);

	 */
}

int
for_chk (void)
{
	/*
	 *	Alternate foreground check which returns 0 if
	 *	foreground, 1 otherwise
	 */
	int	foregnd = isatty (STDIN_FILENO);

	/*------------------------------------------------------------------
	| This is a Secondry Test to see if Parent Who Was in Forground    |
	| Has Died Which Makes this process in background but isatty still |
        | thinks it is in foreground.                                      |
	------------------------------------------------------------------*/
	if (getppid() == 1)
		foregnd = FALSE;

	if (foregnd) 
		return 0;
	return 1;
}

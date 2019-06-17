/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Unknown         | Date Written  : ??/??/??     |
|  Source Name       : debug.c                                        |
|                                                                     |
|---------------------------------------------------------------------|
|  Date Modified     : 02/07/99   | Modified  by  : Trevor van Bremen |
|                                                                     |
|  Comments          :                                                |
|  (02/07/99)        : Changed from the old-fashioned varargs to use  |
|                    : the more supported stdarg                      |

	$Log: errlog.c,v $
	Revision 5.0  2001/06/19 07:07:43  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:53:53  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 2.0  2000/07/15 07:32:18  gerry
	Forced Revision No start to 2.0 - Rel-15072000
	
	Revision 1.1  2000/02/10 00:32:46  jonc
	Added logging for lock errors on abc_update and abc_delete.
	
	Revision 1.4  1999/09/23 22:42:40  jonc
	Removed use of deprecated <malloc.h>. Declarations for malloc
	now found in <stdlib.h>
	
=====================================================================*/
#include	<stdarg.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<unistd.h>

#include	<osdefs.h>
#include	<debug.h>

#define	TRUE		1
#define	FALSE		0

#define	ENVSTR		"DEBUG"
#define	SUFFIX		"-log"

/*
 * Local globals
 */
static FILE * log = NULL;

void
DBIFWriteLog (
 const char *mask,
 ...)
{
	va_list	args;
	time_t now;

	if (!log)
	{
		char pathname [1024];

		sprintf (pathname, "%s/BIN/LOG/cisam-dbif.log", getenv ("PROG_PATH"));
		if (!(log = fopen (pathname, "a")))
			return;
	}

	va_start (args, mask);

	now = time (NULL);
	fprintf (log, "[%05d] %.19s ", getpid (), ctime (&now));

	vfprintf (log, mask, args);
	va_end (args);

	fputc ('\n', log);
	fflush (log);
}

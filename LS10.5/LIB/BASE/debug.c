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

	$Log: debug.c,v $
	Revision 5.0  2001/06/19 06:59:15  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:36  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.1  2000/12/04 01:18:57  scott
	Updated to fix minor warnings on HP box.
	
	Revision 3.0  2000/10/12 13:34:20  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:13  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
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
 *	Local globals
 */
static char	*logfilename = NULL;

/*
 *	Function declarations
 */
static FILE	*OpenLog (void);

void
SetLog (
 const char	*name)
{
	char	*base = strrchr (name, '/');
	char	*tmp = getenv ("TMPDIR");

	if (!tmp)
		tmp = "/tmp";
	base = (char *) (base ? base + 1 : name);
	if (logfilename)
		free (logfilename);

	logfilename = malloc (
		strlen (tmp) +
		strlen (name) +
		strlen (SUFFIX) +
		2);							/* '/' + '\0' */
	sprintf (logfilename, "%s/%s%s", tmp, base, SUFFIX);
}

void
WriteLog (char *mask, ...)
{
	va_list	args;
	FILE		*log;
	time_t		now;
	static int	envpeek = FALSE;
	static char	*dbgstr = NULL;

	va_start (args, mask);

	/*
	 *	Check to see whether the envrironment is set up for debugging
	 */
	if (!envpeek && getenv (ENVSTR))
		dbgstr = strdup (getenv (ENVSTR));
	if (!dbgstr)
		return;

	if (!(log = OpenLog ()))
		return;

	now = time (NULL);
	fprintf (log, "[%05d] %.19s ", (int) getpid (), ctime (&now));

	vfprintf (log, mask, args);
	va_end (args);

	fputc ('\n', log);
	fclose (log);
}

/*
 *
 */
static FILE *
OpenLog (void)
{
	if (!logfilename)
		SetLog ("default");
	return (fopen (logfilename, "a"));
}

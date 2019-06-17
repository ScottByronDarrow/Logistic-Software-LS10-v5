#ident	"$Id: DebugLog.C,v 5.0 2001/06/19 08:19:05 cha Exp $"
/*
 *	Debug log dumps:
 *
 *		Terribly slow, but it works.
 *
 *******************************************************************************
 *	$Log: DebugLog.C,v $
 *	Revision 5.0  2001/06/19 08:19:05  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1998/04/30 04:20:17  jonc
 *	Added extra checks for possible weird-outs on _PATH_TMP
 *
 *	Revision 1.1.1.1  1998/01/22 00:58:45  jonc
 *	Version 10 start
 *
 *	Revision 2.4  1996/11/20 22:54:23  jonc
 *	Fixed: DebugLevel of 0 created a log file
 *
 *	Revision 2.3  1996/09/16 01:11:38  jonc
 *	Added extensions for varargs and handling [app|lib]_error_handler
 *
 *	Revision 2.2  1996/08/21 04:48:20  jonc
 *	DEBUG of 0 now produces no logs
 *
 *	Revision 2.1  1996/08/18 05:13:52  jonc
 *	Added DebugLog class
 *
 */
#include	<assert.h>
#include	<paths.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<time.h>
#include	<unistd.h>

#include	<osdeps.h>
#include	<liberr.h>

#include	<DebugLog.h>

#include	<String.h>

/*
 *	Magic stuff
 */
static const char
	*	DebugEnv	= "DEBUG",
	*	LogSuffix	= ".log";

/*
 *	Local variables
 */
static DebugLog *	appErrLogger = NULL,
				*	libErrLogger = NULL;

/*
 *	Local functions
 */
static void	DebugAppError (const char *, const char *, ...),
			DebugLibError (const char *, const char *, ...);

/*
 *	Public interface
 */
DebugLog::DebugLog (
 const char *	name) :
	logFile (NULL),
	envLvl (0)
{
	const char *	eDebug = ::getenv (DebugEnv);

	assert (name);
	if (eDebug && (envLvl = ::atoi (eDebug)))
	{
		/*
		 *	Build temporary file pathname
		 */
		String	pathname;
		const char *	tDir = ::getenv ("TMPDIR");

		if (tDir)
			pathname = tDir;
		else
			pathname = _PATH_TMP;

		if (pathname.lastchar () != '/')
			pathname += '/';

		char *	bname = ::strrchr (name, '/');

		if (bname)
			pathname += bname + 1;
		else
			pathname += name;
		pathname += LogSuffix;

		logFile = ::fopen (pathname, "a");
	}
}

DebugLog::~DebugLog ()
{
	if (logFile)
		fclose (logFile);
}

void
DebugLog::Log (
 enum LogLevel	lvl,
 const char *	mask,
 ...)
{
	va_list	args;

	va_start (args, mask);
	Log (lvl, mask, args);
	va_end (args);
}

void
DebugLog::Log (
 enum LogLevel	lvl,
 const char *	mask,
 va_list		args)
{
	if (!logFile || !envLvl || envLvl < lvl)
		return;

	char *	hdr = NULL;

	switch (lvl)
	{
	case Life:		hdr = "LIFE";	break;
	case Key:		hdr = "KEY";	break;
	case Verbose:	hdr = "VRBOS";	break;
	case Debug:		hdr = "DEBUG";	break;
	default:		hdr = "*BAD*";	break;
	}

	time_t	now = ::time (NULL);			// timestamp

	fprintf (logFile, "[%05d] %.19s %5s: ", ::getpid (), ::ctime (&now), hdr);

	vfprintf (logFile, mask, args);

	fputc ('\n', logFile);
	fflush (logFile);
}

void
DebugLog::UseForAppError ()
{
	appErrLogger = this;
	app_error_handler = DebugAppError;
}

void
DebugLog::UseForLibError ()
{
	libErrLogger = this;
	lib_error_handler = DebugLibError;
}

/*
 *	Support functions
 */
static void
DebugAppError (
 const char *,
 const char *	mask,
 ...)
{
	va_list	args;

	va_start (args, mask);
	appErrLogger -> Log (DebugLog::Life, mask, args);
	va_end (args);
	exit (EXIT_FAILURE);
}

static void
DebugLibError (
 const char *,
 const char *	mask,
 ...)
{
	va_list	args;

	va_start (args, mask);
	libErrLogger -> Log (DebugLog::Life, mask, args);
	va_end (args);
	exit (EXIT_FAILURE);
}

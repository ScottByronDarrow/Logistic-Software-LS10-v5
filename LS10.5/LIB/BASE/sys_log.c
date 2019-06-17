/*
 *	Logistic Software log system
 */
#include	<errno.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<unistd.h>

#include	<sys_log.h>

/*
 *	External references
 */
extern char	*PNAME;

/*
 *	List of log file-names
 */
static char
	*LogFBackground		= "background.log",
	*LogFApplication	= "apps.log";

/*
 *	Internal functions
 */
static FILE	*OpenLogFile (enum SysLogFile);

/*
 *	Interface
 */
void
sys_log (
 enum SysLogFile	log_type,
 char				*fmt,
 ...)
{
	va_list		ap;
	time_t		now = time (NULL);
	FILE		*log = OpenLogFile (log_type);

	if (!log)
		return;					/* there's nothing we can do */

	fprintf (log, "%.19s %s[%d] : ", ctime (&now), PNAME, getpid ());

	va_start (ap, fmt);
	vfprintf (log, fmt, ap);
	va_end (ap);

	fputc ('\n', log);
	fclose (log);
}

static FILE *
OpenLogFile (
 enum SysLogFile	t)
{
	char	*name = NULL;
	char	path [128];
	struct stat	info;

	/*
	 *	Check for the existence of the LOG directory, creating if required
	 */
	sprintf (path, "%s/BIN/LOG", getenv ("PROG_PATH"));
	if (stat (path, &info))
	{
		if (errno == ENOENT)
		{
			/*
			 *	Attempt to create the LOG directory
			 */
			if (mkdir (path, 0777))
			{
				/*
				 *	Failed!
				 */
				return (NULL);			/* give up */
			}
		}
		else
			return (NULL);				/* give up */
	}
	else
	{
		/*
		 *	Ensure that it is a directory
		 */
		if (!S_ISDIR (info.st_mode))
			return (NULL);
	}

	/*
	 *	Get appropriate name
	 */
	switch (t)
	{
	case LogBackground:
		name = LogFBackground;
		break;
	case LogApplication:
		name = LogFApplication;
		break;

	default:
		fprintf (stderr, "sys_log: Bad argument\n");
		exit (EXIT_FAILURE);
	}
	return (fopen (strcat (strcat (path, "/"), name), "a"));
}

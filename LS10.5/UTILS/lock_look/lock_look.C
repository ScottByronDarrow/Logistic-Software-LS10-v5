#ident	"$Id: lock_look.C,v 5.0 2001/06/19 08:22:59 robert Exp $"
/*
 *	lock_look:
 *
 *	Looks for write locks on a table.
 *
 *******************************************************************************
 *	$Log: lock_look.C,v $
 *	Revision 5.0  2001/06/19 08:22:59  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:18  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:21  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/07/20 00:40:11  jonc
 *	Adopted Pinnacle V10 lockLook.
 *	
 */
#include	<errno.h>
#include	<fcntl.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdeps.h>
#include	<liberr.h>

#include	<String.h>

#include	<Database.h>
#include	<Table.h>

#include	"Project"
#include	"LockBucket.h"

/*
 *
 */
static void	local_error_handler (const char *, const char *, ...);

/*
 *
 */
main (
 int argc,
 char * argv [])
{
	if (argc < 2)
	{
		fprintf (stderr, "Usage: %s table [table ...]\n", argv [0]);
		fprintf (stderr, "Version - %s %s\n", ProjectState, ProjectRevision);
		return EXIT_FAILURE;
	}

	lib_error_handler = local_error_handler;

	/*
	 */
	Database	data ("data");
	LockBucket	locks;

	for (int i = 1; i < argc; i++)
	{
		/*
		 *	We'll be using a couple of internal-undocumented calls,
		 *	unfortunately
		 */
		const char *	fields [] = { NULL };
		Table			table (data, argv [i], fields);
		String			pathname;
		struct flock	lock;

		cat (data.Name (), "/", table.FName (), pathname);
		pathname += ".idx";

		/*
		 *	Let's inspect for blocking write-locks on the
		 *	CISAM file. This requires us to open the file
		 *	with write-permissions
		 */
		int	fd = open (pathname, O_WRONLY);

		if (fd < 0)
		{
			fprintf (stderr,
				"%s: Unexpected error opening `%s', %s\n",
				argv [0], pathname.chars (), strerror (errno));
			return EXIT_FAILURE;
		}

		lock.l_type		= F_WRLCK;	//	ask for exclusive locks
		lock.l_whence	= 0;		//	relative from start of file
		lock.l_start	= 0;
		lock.l_len		= 0;
		lock.l_pid		= 0;

		/*
		 *	Look for all locks, but only report
		 *	on write locks
		 */
		while (lock.l_type != F_UNLCK)
		{
			if (fcntl (fd, F_GETLK, &lock) == -1)	//	blocking write locks
			{
				fprintf (stderr, "%s: lock-enquiry failed? %s\n",
					argv [0], strerror (errno));
				return EXIT_FAILURE;
			}

			if (lock.l_type == F_WRLCK)
			{
				for (int j = 0; j < lock.l_len; j++)
					locks.Add (
						LockInfo (argv [i],
						lock.l_pid,
						lock.l_start + j));

				if (!lock.l_len)
					break;		//	Locked until end of address space

				lock.l_start += lock.l_len;		// begin from next token
				lock.l_len = 0;					// reset
			}
		}

		/*
		 *	Cleanup
		 */
		close (fd);
	}


	/*
	 *	Display info
	 */
	if (!locks.Count ())
		puts ("No locks");
	else
	{
		locks.BuildProcessInfo ();

		printf ("TABLE              RECNO      UID PID   COMMAND\n");
		for (int i = 0; i < locks.Count (); i++)
		{
			const LockInfo &	elem = locks.Elem (i);

			printf ("%-18s %5ld %8s %5d %s\n",
				elem.table.chars (),
				elem.recno,
				elem.user.chars (),
				elem.pid,
				elem.command.chars ());
		}
	}
	return EXIT_SUCCESS;
}

static void
local_error_handler (
 const char * section,
 const char * mask,
 ...)
{
	va_list	args;

	fprintf (stderr, "FATAL: (%s) ", section ? section : "?");

	va_start (args, mask);
	vfprintf (stderr, mask, args);
	va_end (args);

	fputc ('\n', stderr);

	exit (EXIT_FAILURE);
}

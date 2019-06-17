/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
=======================================================================
| This program lists out all the terminal slots currently used with   |
| the processes attached to them                                      |
|---------------------------------------------------------------------|
|  Date Written  : (02/11/93)      | Author      : Jonathan Chen      |
|---------------------------------------------------------------------|
|  Date Modified : (16.12.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  (16.12.94)    : Added support for systems with /proc file-systems  |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: tokens.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/tokens/tokens.c,v 5.2 2001/08/09 09:27:47 scott Exp $";

#include	<errno.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdefs.h>
#include	<license2.h>

#ifdef	HAS_STDLIB_H
#include	<stdlib.h>
#else
extern char	*getenv (char *);
#endif

#ifdef	HAS_PROCFS
#include	<pwd.h>
#include	<sys/stat.h>
#endif

#define	TRUE	1
#define	FALSE	0

/*=======================
| Function Declarations |
=======================*/
static int RealSlot (long posn);
static void	LockList (int fd);
#ifdef	HAS_PROCFS
static char	*Uid (pid_t);
#endif

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv [])
{
	int		lic_fd;
	char	*env = getenv ("PROG_PATH");
	char	fname [80];

	sprintf (fname, "%s/BIN/LICENSE", env ? env : "/usr/LS10.5");
	if ((lic_fd = open (fname, O_WRONLY)) < 0)
	{
		fprintf (stderr, "Failed to open %s (%d)\n", fname, errno);
		return (EXIT_FAILURE);
	}

	LockList (lic_fd);

	close (lic_fd);
	return (EXIT_SUCCESS);
}

static	int
RealSlot (
 long posn)
{
	/*
	 | Translates a file position to a Logistic terminal slot
	*/
	return (posn + MAX_TOKENS - sizeof (struct LIC_REC));
}

static void	
LockList (
 int fd)
{
	int	hdr = FALSE;
	struct flock	lock;

#ifdef LINUX
    /*------------------------------------------------------------------
    | The contents of this #ifdef LINUX block is a workaround to handle
    | the way that LINUX reports fcntl() locks as compared to other Unix
    | based OS's.
    |
    | When checking for conflicting locks in a area of a file the Linux
    | implementation of fcntl() does not always report the first conflict
    | lock based on file position.
    |
    | Trevor van Bremen has pointed out that while most Unix OS's do
    | return the first conflicting lock based on file position, this is
    | not a POSIX requirement (Just a nice user-friendly feature).
    |
    | This new code should possibly be made the default code. I have
    | chosen not to do this as there is a (very very tiny) performance
    | overhead involved in the newer code.
    ------------------------------------------------------------------*/
    int i;

        /*------------------------------------------
        | Check each and every slot individually to
        | determine if it is currently locked.
        ------------------------------------------*/
	for (i = 0; i < MAX_TOKENS; i++) 
	{
		lock.l_type		= F_WRLCK;	/*	ask for exclusive locks	*/
		lock.l_whence	= 0;		/*	relative from start of file */
		lock.l_start	= sizeof (struct LIC_REC) - MAX_TOKENS + i;
		lock.l_len		= 1;

		if (fcntl (fd, F_GETLK, &lock) == -1)
		{
			fprintf (stderr, "Internal error : fcntl() [%d]\n", errno);
			return;
		}

		if (lock.l_type == F_UNLCK)
		{
			continue; /* not locked */
		}

		if (!hdr)
		{
			printf ("%6s %5s", "PID", "SLOT");
#ifdef	HAS_PROCFS
			printf (" %-8s", "UID");
#endif	/*HAS_PROCFS*/

			putchar ('\n');
			hdr = TRUE;
		}

		printf ("%6d %5d",
			lock.l_pid,
			RealSlot (lock.l_start));

#ifdef	HAS_PROCFS
		printf (" %-8s", Uid (lock.l_pid));
#endif	/*HAS_PROCFS*/

		putchar ('\n');
	}

#else /* LINUX */

	lock.l_type		= F_WRLCK;	/*	ask for exclusive locks	*/
	lock.l_whence	= 0;		/*	relative from start of file */
	lock.l_start	= sizeof (struct LIC_REC) - MAX_TOKENS;
	lock.l_len		= 0;

	while (lock.l_type != F_UNLCK)
	{
		/*
		 | Ask for blocking write locks
		*/
		if (fcntl (fd, F_GETLK, &lock) == -1)
		{
			fprintf (stderr, "Internal error : fcntl() [%d]\n", errno);
			return;
		}

		if (lock.l_type != F_UNLCK)
		{
			if (!hdr)
			{
				printf ("%6s %5s", "PID", "SLOT");

#ifdef	HAS_PROCFS
				printf (" %-8s", "UID");
#endif	/*HAS_PROCFS*/

				putchar ('\n');
				hdr = TRUE;
			}

			printf ("%6d %5d",
				lock.l_pid,
				RealSlot (lock.l_start));

#ifdef	HAS_PROCFS
			printf (" %-8s", Uid (lock.l_pid));
#endif	/*HAS_PROCFS*/

			putchar ('\n');

			if (!lock.l_len)
			{
				/*	Locked until end of address space */
				break;
			}

			lock.l_start += lock.l_len;		/* begin from next token */
			lock.l_len = 0;					/* reset */
		}
	}
#endif /* LINUX */
}

#ifdef	HAS_PROCFS
static char *
Uid (
 pid_t	pid)
{
	char	procname [32];
	struct stat	info;
	struct passwd	*uentry;

	sprintf (procname, "/proc/%d", pid);
	if (stat (procname, &info))
		return ("[err]");

	if (!(uentry = getpwuid (info.st_uid)))
		return ("[pwderr]");
	return (uentry -> pw_name);
}
#endif	/*HAS_PROCFS*/

/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( message.c  	)                                 |
|  Program Desc  : ( Logistic Software Messaging Utiltity.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A                                               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A                                               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (19/05/94)      | Authors : Cam Mander & Jon Chen  |
|---------------------------------------------------------------------|
|  Date Modified : (21.04.95)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|  (21.04.95)    : Improved portability from better understanding     |
|                                                                     |
=====================================================================*/
#include <stdio.h>
#include <curses.h>
#include <term.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <osdefs.h>

/*------------------------------------------------------------------------------
|	message :	Generates message to receivers                                 |
|		-g	:	Message to all receivers                                       |
|		-u	:	Message to user                                                |
|		-c	:	Clear message line                                             |
|		-b	:	Beeps (default 1)                                              |
|		-d	:	Create message receiver for current device (daemon option)     |
|		-k	:	Remove message receiver for current device                     |
|                                                                              |
|	Receivers are message daemons attached to terminals, which will            |
|	display messages on their respective terminals if a message is             |
|	destined for them.                                                         |
|                                                                              |
|	A simple protocol is used to talk to receivers                             |
|	Each receiver will receive a line consisting of :                          |
|                                                                              |
|		id string [string ...] '\n'                                            |
|                                                                              |
|	the numeric id determines the actions to be taken with the following       |
|	strings                                                                    |
|                                                                              |
------------------------------------------------------------------------------*/

#define	FORKIT						/* Uncomment for release */

#define	SLEEP_INTERVAL	5
#define	MESG_END		'\n'
#define	DIR_PERM		0700		/* rwx for root only */
#define	FIFO_PERM		0600		/* rw for user only */
#define	LOCK_PERM		0600		/* rw for user only */

enum
{
	RCV_MesgLocal,
	RCV_MesgUser,
	RCV_MesgGlobal,

	RCV_ClearLocal,
	RCV_ClearUser,
	RCV_ClearGlobal,

	RCV_Quit
};

/*---------
| Global. |
---------*/
static int	terminated = FALSE;

/*--------------------------------
| FIFO directory (for receivers) |
--------------------------------*/
static char	*FIFODir	= "/tmp/PSL-message",
			*LockSuffix	= ".LCK";
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static void Usage (const char *cmd);
static int ActAsReceiver (int beeps);
static int CreateListener (char *name);
static void ReadMessage (int fd, char *ttydev, char *userid, int beeps);
static void HandleMessage (char *devname, char *userid, char *msg, int beeps);
static char *ExtrctArg (char *msg, char *extArg);
static void DisplayMessage (const char *mesg, int beeps);
static void ClearMessage (void);
static void quit_trap (int sig);
void SendMsg (char *msg);
static int GetLock (const char *basename, int with_wait);
static void ReleaseLock (int fd);
static void RemoveLockFile (const char *basename);

#ifndef	HAS_SID_FNS
static void	alarm_trap (int);
#endif	/*HAS_SID_FNS*/

/*
 *	getopt stuff
 */
extern int	optind;
extern char	*optarg;

/*-------
| MAIN. |
-------*/
int
main (
 int                argc,
 char*              argv [])
{
	int		opt;
	int		global, user, clear, beeps, killit;
	char	msgStr [256];
	char	userName [15];

	if (geteuid ())
	{
		fprintf (stderr, "%s : needs to be set with root su-id\n", argv [0]);
		return (EXIT_FAILURE);
	}
	umask (0);		/* makes it simpler to keep track of creation permissions */

	/*-------------------
	| Handle Arguments. |
	-------------------*/
	if (argc < 2)
	{
		Usage (argv [0]);
		return (EXIT_FAILURE);
	}

	/*------------------------------
	| Initialise option variables. |
	------------------------------*/
	global = user = clear = killit = FALSE;
	beeps = -1;
	while ((opt = getopt (argc, argv, "gu:cb:dk")) != EOF)
	{
		int	err = FALSE;

		switch (opt)
		{
		case 'g'	:
			if (user)
				err = TRUE;
			else
				global = TRUE;
			break;

		case 'u'	:
			if (global)
				err = TRUE;
			else
			{
				user = TRUE;
				strcpy (userName, optarg);
			}
			break;

		case 'c'	:
			clear = TRUE;
			break;

		case 'b'	:
			if (!(beeps = atoi (optarg)))
				err = TRUE;
			break;

		case 'd'	:
			if (global || user || clear || killit)
				err = TRUE;
			else
				return (ActAsReceiver (beeps < 0 ? 1 : beeps) ? 0 : 1);
			break;

		case 'k'	:
			if (global || user || clear)
				err = TRUE;
			else
				killit = TRUE;
			break;

		default		:
			err = TRUE;
		}
		if (err)
		{
			Usage (argv [0]);
			return (EXIT_FAILURE);
		}
	}

	/*
	 *	Build the message id with possible args
	 */
	if (global)
	{
		sprintf (msgStr, "%d ", (clear) ? RCV_ClearGlobal : RCV_MesgGlobal);
	}
	else if (user)
	{
		sprintf (msgStr, 
				 "%d %s ", 
				 (clear) ? RCV_ClearUser : RCV_MesgUser,
				 userName);
	}
	else if (killit)
	{
		sprintf (msgStr, "%d %s ", RCV_Quit, ttyname (0));
	}
	else
	{
		sprintf (msgStr, 
				 "%d %s ", 
				 (clear) ? RCV_ClearLocal : RCV_MesgLocal,
				 ttyname (0));
	}

	/*
	 *	Cat args together
	 */
	while (optind < argc)
		strcat (strcat (msgStr, argv [optind++]), " ");

	SendMsg (strcat (msgStr, "\n"));

	return (EXIT_SUCCESS);
}

/*------------------------
| Display program usage. |
------------------------*/
static void
Usage ( 
 const char*        cmd)
{
	fprintf (stderr, 
		"Usage : %s [-g|-u userid] [string ...]\n",
		cmd);

	fprintf (stderr, 
		"      : %s [-g|-u userid] [-c]\n",
		cmd);

	fprintf (stderr,
		"      : %s -d [-b beeps]\n",
		cmd);
	fprintf (stderr, 
		"      : %s -k\n",
		cmd);
}


/*=============================================================
|                                                             |
|   Code section for "message" acting as a receiver daemon.   |
|                                                             |
=============================================================*/
/*-------------------------
| Set up Receiver daemon. |
-------------------------*/
static	int
ActAsReceiver (
 int                beeps)
{
	int		i;
	int		lock_fd, fd, fderr;
	char	name [80],
			ttydev [80],
			userid [L_cuserid];
	char	*tty;
	pid_t	getsid (pid_t);

#ifndef	HAS_SID_FNS
	/*
	 *	Since no automatic signalling is available, we
	 *	check the parent pid using an alarm() time-out
	 */
	pid_t	shellid = getppid ();
#endif	/*HAS_SID_FNS*/

	/*---------------------
	| Preliminary checks and initialisation
	---------------------*/
	if (!isatty (1))
		return (FALSE);
	strcpy (ttydev, ttyname (1));
	strcpy (userid, getenv ("LOGNAME"));

	/*----------------------
	| Put into background. |
	----------------------*/
#ifdef	FORKIT
	switch (fork ())
	{
	case -1	:
		perror ("fork failed");
		return (FALSE);

	case 0	:
		/*--------------------------
		| Child : act as receiver. |
		--------------------------*/
		break;

	default	:
		return (TRUE);
	}
#endif

#ifdef	HAS_SID_FNS

	/*
	 *	Set the process group id to that of the session leader
	 *	so that when it (ie the controlling shell) dies, it will
	 *	send a SIGHUP to the current process
	 */
	setpgid (getpid (), getsid (0));

#endif	/*HAS_SID_FNS*/

	/*
	 * Build FIFO Filename
	 */
	tty = ttyname (1);
	for (i = 0; i < strlen (tty); i++)			/* strip '/' from device name */
		if (tty [i] == '/')
			tty [i] = '-';
	sprintf (name, "%s/%s", FIFODir, tty);		/* name in FIFO directory */

	/*-----------------------------------
	| Set signals for death of program. |
	-----------------------------------*/
	signal (SIGINT,  quit_trap);
	signal (SIGHUP,  quit_trap);
	signal (SIGQUIT, quit_trap);
	signal (SIGTERM, quit_trap);

	/*-------------------
	| Set up FIFO file. |
	-------------------*/
	if (!CreateListener (name))
	{
		fprintf (stderr, "CreateListener () failed [%d]\n", errno);
		return (FALSE);
	}

	/*	Check for possible (old) receiver
	 */
	if ((lock_fd = GetLock (name, FALSE)) < 0)			/* no-wait */
	{
		char	term_req [80];

		/*	Place a lock-request and at the same time issue a quit
		 *	to the old-receiver
		 */
		switch (fork ())
		{
		case -1	:
			return (FALSE);

		case 0	:
			/*	Child. Issue termination request to old-receiver
			 */
			sprintf (term_req, "%d %s\n", RCV_Quit, ttydev);
			SendMsg (term_req);
			return (TRUE);

		default	:
			/*	Parent. Issue lock request
			 */
			if ((lock_fd = GetLock (name, TRUE)) < 0)		/* with-wait */
				return (FALSE);
		}
	}

	/*-------------------------------
	| Get terminal characteristics. |
	-------------------------------*/
	if (setupterm (NULL, 1, NULL) == ERR)
	{
		fprintf (stderr, "setupterm () failed\n");
		return (FALSE);
	}

	/*-------------------------------------------------
	| Continually do a blocking open on the FIFO file |
	| that's being interrupted intermittently.        |
	-------------------------------------------------*/
	do
	{
#ifndef	HAS_SID_FNS
		/*
		 *	Set up a alarm to interrupt the read,
		 *	(but only if we can check for shell process)
		 */
		signal (SIGALRM, alarm_trap);
		alarm (SLEEP_INTERVAL);

#endif	/*HAS_SID_FNS*/

		fd = open (name, O_RDONLY);
		fderr = errno;

#ifndef	HAS_SID_FNS

		/*
		 *	We've possible been interrupted by a signal
		 */
		alarm (0);				/* Disable alarms */
		if (kill (shellid, 0))	/* see whether we've been logged out */
		{
			if (fd >= 0)
				close (fd);
			break;				/* quit */
		}

#endif	/*HAS_SID_FNS*/

		if (fd >= 0)
		{
			/* Read and handle message
			 */
			ReadMessage (fd, ttydev, userid, beeps);
			close (fd);
		}

	}	while (!terminated && (fd >= 0 || fderr == EINTR));

	/*---------------------------------
	| remove the FIFO and lock files
	---------------------------------*/
	ReleaseLock (lock_fd);					/* allow others to get lock */
	if ((lock_fd = GetLock (name, FALSE)) >= 0)
	{
		unlink (name);
		RemoveLockFile (name);
		ReleaseLock (lock_fd);
	}

	return (TRUE);
}

/*------------------------------------------
| Create FIFO files in the FIFO directory, |
| putting the name into "name".            |
------------------------------------------*/
static	int
CreateListener (
 char*              name)
{
	struct stat	info;

	/*-----------------------------------------------------
	| Check for FIFO dir, create if necessary & possible. |
	-----------------------------------------------------*/
	if (stat (FIFODir, &info))
	{
		if (errno != ENOENT)
			return (FALSE);

		/*-------------------
		| Create directory. |
		-------------------*/
		if (mkdir (FIFODir, DIR_PERM))
			return (FALSE);
	}
	else
	{
		/*---------------------------------------------------
		| If an entry exists, check that it is a directory. |
		---------------------------------------------------*/
		if (!S_ISDIR (info.st_mode))
			return (FALSE);
	}

	if (stat (name, &info))
	{
		if (errno == ENOENT)
		{
			/*-------------------
			| Create FIFO file. |
			-------------------*/
			if (mkfifo (name, FIFO_PERM))		/* create FIFO file */
				return (FALSE);					/* fatal odd error */
			chown (name, getuid (), 0);
		}
	}
	else
	{
		if (!S_ISFIFO (info.st_mode))
			return (FALSE);
	}

	return (TRUE);
}

static void
ReadMessage (
 int                fd,
 char*              ttydev,
 char*              userid,
 int                beeps)
{
	/*
	 *	Read in the message off the given descriptor,
	 *	minimising read ()'s with buffer space
	 */
	int		buf_off, count;
	char	buf [256];

	buf_off = 0;
	while ((count = read (fd, buf + buf_off, sizeof (buf) - buf_off)) > 0)
	{
		while (count)
		{
			int	i;

			/*
			 *	Look for message terminators
			 */
			for (i = 0; i < count; i++)
			{
				if (buf [i] == MESG_END)
				{
					buf [i] = '\0';
					HandleMessage (ttydev, userid, buf, beeps);

					break;
				}
			}

			if (i < count)
			{
				/*	 Unhandled chars beyond the previous message
				 */
				count = strlen (strcpy (buf, buf + i + 1));
			}
			else
			{
				/*	Non MESG_END terminated message.
				 *	Read in more
				 */
				buf_off = count;
				count = 0;			/* force a break */
				break;
			}
		}
	}
}

/*----------------------
| Process the message. |
----------------------*/
static void
HandleMessage (
 char*              devname,
 char*              userid,
 char*              msg,
 int                beeps)
{
	int		action;
	char	tmpStr [31];

	if (sscanf (msg, "%d", &action) != 1)
		return;									/* bad message */

	/*-------------------------------
	| Move "msg" past action digit. |
	-------------------------------*/
	while ((*msg && (isspace (*msg) || isdigit (*msg))))
		msg++;

	switch (action)
	{
	case RCV_MesgLocal	:
		msg = ExtrctArg (msg, tmpStr);
		if (!strcmp (tmpStr, devname))			/* check matching dev name */
			DisplayMessage (msg, beeps);
		break;

	case RCV_MesgUser	:
		msg = ExtrctArg (msg, tmpStr);
		if (!strcmp (tmpStr, userid))			/* check matching user id */
			DisplayMessage (msg, beeps);
		break;

	case RCV_MesgGlobal	:
		DisplayMessage (msg, beeps);
		break;

	case RCV_ClearLocal	:
		msg = ExtrctArg (msg, tmpStr);
		if (!strcmp (tmpStr, devname))			/* check matching dev name */
			ClearMessage ();
		break;

	case RCV_ClearUser	:
		msg = ExtrctArg (msg, tmpStr);
		if (!strcmp (tmpStr, userid))			/* check matching user */
			ClearMessage ();
		break;

	case RCV_ClearGlobal:
		ClearMessage ();
		break;

	case RCV_Quit	:
		msg = ExtrctArg (msg, tmpStr);
		if (!strcmp (tmpStr, devname))			/* check matching device */
			quit_trap (0);						/* fake a quit signal */
		break;
	}
}

/*--------------------------------
| Extract next argument from msg |
| Argument may be a user name or |
| a device name.                 |
--------------------------------*/
static char *
ExtrctArg (
 char	*msg,
 char	*extArg)
{
	char	*spcPtr;

	if (!msg || !*msg)
		return (NULL);

	/*-----------------------------------
	| Next space delimits the argument. |
	-----------------------------------*/
	for (spcPtr = msg; *spcPtr && !isspace (*spcPtr); spcPtr++);
	*spcPtr++ = '\0';

	/*-------------------
	| Extract argument. |
	-------------------*/
	strcpy (extArg, msg);

	return (spcPtr);
}

/*----------------------
| Display the message. |
----------------------*/
static void
DisplayMessage (
 const char	*mesg,
 int		beeps)
{
	int		beepCnt;

	if (*mesg)
	{
		if (has_status_line)
		{
			if (*to_status_line)
				putp (to_status_line);
			if (status_line_esc_ok && *enter_standout_mode)
				putp (enter_standout_mode);
		}

		printf ("%.*s", columns ? columns : 256, mesg);

		if (has_status_line)
		{
			if (status_line_esc_ok && *exit_standout_mode)
				putp (exit_standout_mode);
			if (*from_status_line);
				putp (from_status_line);
		}

		if (bell)
		{
			for (beepCnt = 0; beepCnt < beeps; beepCnt++)
			{
				putp (bell);
				sleep (2);
			}
		}

		fflush (stdout);
	}
}

/*---------------------
| Clear message line. |
---------------------*/
static void
ClearMessage (void)
{
	if (!has_status_line)
		return;

	if (*dis_status_line)
		putp (dis_status_line);
	else
	{
		if (*to_status_line && *from_status_line)
		{
			putp (to_status_line);
			putp (from_status_line);
		}
	}
	fflush (stdout);
}

/*=======================
|                       |
|   Support utilities   |
|                       |
=======================*/
/*******************************************************************************
 *
 *	Signal traps
 *
 ******************************************************************************/
#ifndef	HAS_SID_FNS
/*----------------------------
| Stub function for SIGALRM. |
----------------------------*/
static void
alarm_trap (
 int	sig)
{
}
#endif	/*HAS_SID_FNS*/

/*------------------------------------------------
| Signal traps for SIGQUIT, SIGINT, SIGTERM etc. |
------------------------------------------------*/
static void
quit_trap (
 int	sig)
{
	terminated = TRUE;
}

/*-------------------------------------------------------
| Send message to all fifo files in receiver directory. |
-------------------------------------------------------*/
void
SendMsg (
 char 	*msg)
{
	struct 	stat	info;
	struct	dirent	*dirEnt;
	DIR		*dirPtr;
	size_t	msglen = strlen (msg);

	/*---------------------------------------
	| Check that receiver directory exists. |
	---------------------------------------*/
	if (stat (FIFODir, &info))
	{
		if (errno != ENOENT)
			return;

		/*-------------------
		| Create directory. |
		-------------------*/
		if (mkdir (FIFODir, DIR_PERM))
			return;
	}

	/*---------------------------------------
	| Send message to all files in FIFODir. |
	---------------------------------------*/
	dirPtr = opendir (FIFODir);
	while ((dirEnt = readdir (dirPtr)))
	{
		int		sndFd;
		char	sndFifo [80];

		/*---------------------------------------------
		| Construct name of FIFO to open and open it. |
		---------------------------------------------*/
		sprintf (sndFifo, "%s/%s", FIFODir, dirEnt -> d_name);
		if (stat (sndFifo, &info) ||
			!S_ISFIFO (info.st_mode) ||
			(sndFd = open (sndFifo, O_WRONLY | O_NDELAY)) < 0)
		{
			continue;
		}

		write (sndFd, msg, msglen);

		close (sndFd);
	}

	closedir (dirPtr);
}

/*
 *	Lock file code
 */
static	int
GetLock (
 const char	*basename,
 int		with_wait)
{
	int		fd,
			dummy = 0;
	char	name [64];
	struct stat		info;
	struct flock	lock;

	sprintf (name, "%s%s", basename, LockSuffix);
	if (stat (name, &info))
	{
		if (errno == ENOENT)
		{
			fd = open (name, O_WRONLY | O_CREAT, LOCK_PERM);
			write (fd, &dummy, sizeof (int));
			chown (name, getuid (), 0);
		}
		else
			return (-1);
	}
	else
		fd = open (name, O_WRONLY);

	lock.l_type		= F_WRLCK;
	lock.l_whence	= 0;
	lock.l_start	= 0;
	lock.l_len		= sizeof (dummy);

	if (fcntl (fd, with_wait ? F_SETLKW : F_SETLK, &lock) < 0)
	{
		close (fd);
		return (-1);
	}
	return (fd);
}

static	void
ReleaseLock (
 int	fd)
{
	struct flock	lock;

	lock.l_type		= F_UNLCK;
	lock.l_whence	= 0;
	lock.l_start	= 0;
	lock.l_len		= sizeof (int);

	fcntl (fd, F_SETLKW, &lock);
	close (fd);
}

static void
RemoveLockFile (
 const char	*basename)
{
	char	name [64];

	sprintf (name, "%s%s", basename, LockSuffix);
	unlink (name);
}

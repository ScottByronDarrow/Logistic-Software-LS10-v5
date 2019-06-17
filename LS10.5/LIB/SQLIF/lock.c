/*******************************************************************************

	Internal locking system

*******************************************************************************/
#ifndef	lint
static char	*rscid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/lock.c,v 5.2 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<errno.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>

#include	"isamdbio.h"
#include	"stddefs.h"
#include	"tblnode.h"
#include	"fnproto.h"

/***
	Internal representation of locking. Cannot depend on SQL engine's
	locking scheme 'cos SQL's locking strategy is critically different
	from your std PINNALCLE application
**/

#define	T_ROWID		long
#define	T_ROWIDSZ	sizeof (T_ROWID)

#define	EOI		2025		/* getkey() equiv for F16	*/
#define	WAIT_INTERVAL	1	/* second	*/

#define	LOCKPAGE	1024

/**	Globals
**/
static T_ROWID	lockPage [LOCKPAGE];
static char	*lockDir = "/tmp/PSL.lox";

/**	Function proto
**/
static TableNode	*GetPass (char *, int);
extern int			FreePass (TableNode *);
static off_t		FindRowId (TableNode *),
					MakeRowIdSlot (TableNode *);

extern off_t	lseek ();	/* stdlib */
extern unsigned	alarm ();	/* stdlib */

/**
 Initialize Lock system
**/
void	InitLockSystem ()
{
	struct stat	s;

	/**	Check for lockDirectory existence
	**/
	if (stat (lockDir, &s) == 0)
	{
		if (s.st_mode != (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO))
			dbase_err ("InitLockSystem(perms)", lockDir, -1);

	} else if (errno == ENOENT)
	{
		/*	Ain't there, need to create it
		*/
		int	cmask = umask (0);

		if (mkdir (lockDir, S_IRWXU | S_IRWXG | S_IRWXO) < 0)
			dbase_err ("InitLockSystem(dir)", lockDir, errno);
		umask (cmask);	/* restore to original state	*/

	} else
		/*	Unanticipated error
		*/
		dbase_err ("InitLockSystem()", lockDir, errno);
}

/**
**/
LockTbl (tableName)
char	*tableName;
{
	return (GetPass (tableName, TRUE) ? 0 : -1);
}

UnlockTbl (tableName)
char	*tableName;
{
	TableNode	*node = GetTableNode (tableName);

	return ((node && FreePass (node)) ? 0 : -1);
}

/***
	Locks current record for given table
***/
LockRec (tableName, blockIt)
char	*tableName;
int	blockIt;
{
	off_t		pos;
	struct flock	lock;
	TableNode	*node = GetPass (tableName, blockIt);

	if (!node)
		return (FALSE);

	if (!(pos = FindRowId (node)))
		pos = MakeRowIdSlot (node);

	/*	The passkey  must be released before a lock is attempted
		to avoid the possibility of deadlock
	*/
	FreePass (node);

	/*	Lock the record
	*/
	lock.l_type		= F_WRLCK;
	lock.l_whence	= SEEK_SET;
	lock.l_start	= pos;
	lock.l_len		= T_ROWIDSZ;

	if (fcntl (node -> lock_fd, blockIt ? F_SETLKW : F_SETLK, &lock) < 0)
		return (FALSE);
	return (TRUE);
}

/*	dummy func that allows a getchar() to fail [errno = EINTR]
	by having a timeout using an alarm
*/
/*ARGSUSED*/
static void	alarmFunc (x)
int	x;
{
}

LockRecWithAbort (tblName)
char	*tblName;
{
	/**	Non-blocking lock request with esc
	**/
	int	isLocked = LockRec (tblName, FALSE);

	if (!isLocked)
	{
		int	abort = FALSE;

		if (!foreground ())
			return (-1);	/* can't do anything if not upfront */

		do	{
			char	err_str [256];

			signal (SIGALRM, alarmFunc);
			sprintf (err_str,
				"(%s) Record Is Locked By Another User. F16 to Restart",
				tblName);
			print_mess (err_str);
			alarm (WAIT_INTERVAL);
			if (getkey () == EOI)
			{
				alarm (0);	/* unset the alarm */
				abort = TRUE;
			} else
				isLocked = LockRec (tblName, FALSE);

		}	while (!abort && !isLocked);
		signal (SIGALRM, SIG_DFL);
	}
	return (isLocked);
}

/***
	Unlocks all individually locked record for given table
***/
UnlockRecs (tableName)
char	*tableName;
{
	int	i;
	T_ROWID		pgCount = 0;
	TableNode	*node = GetPass (tableName, TRUE);
	struct flock	lock;

	if (!node)
	{
		/* The table has probably been closed */
		return (-1);
	}

	/*	Unlock the slot
	*/
	lock.l_type	= F_UNLCK;
	lock.l_whence	= SEEK_SET;
	lock.l_start	= T_ROWIDSZ;	/* don't unlock passkey	*/
	lock.l_len	= 0;		/* free until eof	*/

	if (fcntl (node -> lock_fd, F_SETLK, &lock) < 0)
	{
		FreePass (node);
		dbase_err ("UnlockRec(internal)", tableName, -1);
		return (-1);
	}

	/**	Free the all slots (ie zap 'em to 0)

		- For each page
		- Find non-zero
		- Attempt to lock
		- If succeed, zero it
	**/
	/*	read in page count	*/
	if (lseek (node -> lock_fd, 0, SEEK_SET) != 0 ||
		read (node -> lock_fd, (char *) &pgCount, T_ROWIDSZ) != T_ROWIDSZ)
	{
		goto badEnd;		/* What?! */
	}

	lock.l_type	= F_WRLCK;
	lock.l_len	= T_ROWIDSZ;	/* reset to single lock rec */

	for (i = 0 ; i < pgCount; i++)
	{
		int	j;
		off_t	pos = i * sizeof (lockPage) + T_ROWIDSZ;

		/*	read in single page	*/
		if (lseek (node -> lock_fd, pos, SEEK_SET) != pos ||
			read (node -> lock_fd, (char *) lockPage, sizeof (lockPage)) != sizeof (lockPage))
		{
			goto badEnd;
		}

		/*	scan thru' page for non-zero items	*/
		for (j = 0; j < LOCKPAGE; j++)
		{
			if (!lockPage [j])
				continue;

			/*	attempt to lock for write changes	*/
			lock.l_start = pos + j * T_ROWIDSZ;
			if (fcntl (node -> lock_fd, F_SETLK, &lock) != -1)
			{
				T_ROWID	zero = 0;

				if (lseek (node -> lock_fd, lock.l_start, SEEK_SET) != lock.l_start ||
					write (node -> lock_fd, (char *) &zero, T_ROWIDSZ) != T_ROWIDSZ)
				{
					goto badEnd;
				}

				lock.l_type = F_UNLCK;
				fcntl (node -> lock_fd, F_SETLK, &lock);
			}
		}
	}

	FreePass (node);
	return (EXIT_SUCCESS);

badEnd	:
	FreePass (node);
	return (errno);
}

static	MakeLockFile (tblName)
char	*tblName;
{
	char	pathName [40];
	int	fd;
	int	cmask = umask (0);		/* save the umask */
	int	notThere = access (
			strcat (
				strcat (strcpy (pathName, lockDir), "/"),
				tblName),
			F_OK) < 0;

	fd = open (pathName,
		O_RDWR | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (fd >= 0 && notThere)
	{
		/*	Pad out with semaphore
		*/
		T_ROWID	hdr = 1;	/*	no of pages	*/

		write (fd, (char *) &hdr, T_ROWIDSZ);

		/*	Write out first page
		*/
		memset ((char *) lockPage, 0, sizeof (lockPage));
		write (fd, (char *) lockPage, sizeof (lockPage));
	}
	umask (cmask);		/* restore original mask */
	return (fd);
}

static TableNode	*GetPass (
char	*tblName,
int	block)
{
	struct flock	lock;
	TableNode	*node = (TableNode *) 0;

	/*	Check for the lockfile
	*/
	if (!(node = GetTableNode (tblName)))
		return (node);

	if (node -> lock_fd < 0 &&
			(node -> lock_fd = MakeLockFile (node -> table)) < 0)
		return ((TableNode *) 0);

	/**	Obtain semaphore section
	**/
	lock.l_type	= F_WRLCK;
	lock.l_whence	= SEEK_SET;
	lock.l_start	= 0;
	lock.l_len	= T_ROWIDSZ;

	if (fcntl (node -> lock_fd, block ? F_SETLKW : F_SETLK, &lock) < 0)
		return ((TableNode *) 0);
	return (node);
}

FreePass (node)
TableNode	*node;
{
	struct flock	lock;

	if (!node || node -> lock_fd < 0)
		return (FALSE);

	/*	Free semaphore section
	*/
	lock.l_type	= F_UNLCK;
	lock.l_whence	= SEEK_SET;
	lock.l_start	= 0;
	lock.l_len	= T_ROWIDSZ;

	return (fcntl (node -> lock_fd, F_SETLK, &lock) != -1);
}

static off_t	MakeRowIdSlot (node)
TableNode	*node;
{
	int	i, j;
	T_ROWID	size;
	off_t	pos;

	lseek (node -> lock_fd, 0, SEEK_SET);
	read (node -> lock_fd, (char *) &size, T_ROWIDSZ);

	for (i = 0; i < size; i++)
	{
		read (node -> lock_fd, (char *) lockPage, sizeof (lockPage));

		for (j = 0; j < LOCKPAGE; j++)
			if (!lockPage [j])
			{
				pos = (i * LOCKPAGE + j + 1) * T_ROWIDSZ;
				goto writeItOut;
			}
	}

	/**	Need to extend it!
	**/
	size++;

	/*	Write an empty page
	*/
	memset ((char *) lockPage, 0, sizeof (lockPage));
	lseek (node -> lock_fd, (off_t) (size * LOCKPAGE * T_ROWIDSZ), SEEK_SET);
	write (node -> lock_fd, (char *) lockPage, sizeof (lockPage));

	/*	Write the new size
	*/
	lseek (node -> lock_fd, 0, SEEK_SET);
	write (node -> lock_fd, (char *) &size, T_ROWIDSZ);

	pos = ((size - 1) * LOCKPAGE + 1) * T_ROWIDSZ;

writeItOut:
	lseek (node -> lock_fd, pos, SEEK_SET);
	write (node -> lock_fd, (char *) &node -> rowid, T_ROWIDSZ);

	return (pos);
}

static off_t	FindRowId (node)
TableNode	*node;
{
	int	i, j;
	T_ROWID	size;

	lseek (node -> lock_fd, 0, SEEK_SET);
	read (node -> lock_fd, (char *) &size, T_ROWIDSZ);

	for (i = 0; i < size; i++)
	{
		read (node -> lock_fd, (char *) lockPage, sizeof (lockPage));

		for (j = 0; j < LOCKPAGE; j++)
			if (lockPage [j] == node -> rowid)
				return ((i * LOCKPAGE + j + 1) * T_ROWIDSZ);
	}
	return (EXIT_SUCCESS);
}

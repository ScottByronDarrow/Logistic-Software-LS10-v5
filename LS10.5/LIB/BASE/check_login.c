/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  This routine reads the LICENSE file that is located in $PROG_PATH, |
|  and checks if the user is allowed to log in                        |
|                                                                     |
|  Parameters : termno   - Logistic terminal number.                  |
|               max_usr  - Maximum concurrent logins.                 |
|                                                                     |
|  Return     : LICENSE_OK    - All is ok.                            |
|               LICENSE_BAD   - non-existent or non-readable license  |
|               LICENSE_OFLOW - Maximum logins exceeded.              |
|---------------------------------------------------------------------|
|  Date Written  :                 | Written by  : Trevor van Bremen  |
|---------------------------------------------------------------------|
|  Date Modified : (02/11/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (21.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|     (02/11/93) : Rewrote to use record locking. This fixes problem  |
|                : where the process calling check_login() is killed  |
|                : and check_logout() is not called - thereby using   |
|                : up a slot. If the slot is randomly allocated (as   |
|                : with a network terminal) you lose the slot         |
|                : indefinitely                                       |
|     (21.10.94) : Made the license file close-on-exec.               |
|                                                                     |
| $Log: check_login.c,v $
| Revision 5.1  2001/08/06 22:40:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:14  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/11 23:45:06  scott
| Updated to fix free slot problem with Linux.
|
| Revision 3.0  2000/08/31 00:57:04  warren
| Forced update to v3.0
|
| Revision 1.1.1.1  2000/08/16 22:23:27  warren
| "Automated import from Manila"
|
| Revision 2.0  2000/07/15 07:17:13  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.3  1999/09/13 09:36:28  scott
| Updated for Copyright
|
======================================================================*/
#include	<std_decs.h>

/*
 * Function declarations
 */
static int	GetLicense (void);
static int	LockCount (void);
static int	LockSlot (unsigned);
static int	UnlockSlot (unsigned);

/*
 * Local globals
 */
static int	lic_fd = -1;		/* license file descriptor */

int
check_login (
 unsigned	termno,
 int		max_usr)
{
	if (!GetLicense ())
		return (LICENSE_BAD);

	if (LockCount () >= max_usr)
		return (LICENSE_OFLOW);
	return (LockSlot (termno) ? LICENSE_OK : LICENSE_OFLOW);
}

int
check_logout (
 unsigned	termno)
{
	if (lic_fd < 0)
		return (LICENSE_BAD);

	if (!UnlockSlot (termno))
		return (LICENSE_OFLOW);

	close (lic_fd);
	lic_fd = -1;		/* reset to something bad */
	return (LICENSE_OK);
}

/*=================
| Open license file
===================*/
static int
GetLicense (void)
{
	char	*sptr,
			lic_nam [64];

	if (lic_fd >= 0)
		return (TRUE);

	/* Construct license file name */
	sptr = getenv ("PROG_PATH");
	sprintf (lic_nam, "%s/BIN/LICENSE", sptr ? sptr : "/usr/LS10.5");

	if ((lic_fd = open (lic_nam, O_WRONLY)) >= 0)
	{
		fcntl (lic_fd, F_SETFD, 1);	/* Set close-on-exec flag */
		return (TRUE);
	}
	return (FALSE);
}

/*================
 | Record Locking routines
 =================*/
static unsigned
SlotPosn (
 unsigned	slot)
{
	/*
	 * Normalises slot number to correct position within LICENSE file
	 */
	return (sizeof (struct LIC_REC) - MAX_TOKENS + slot);
}

static int
LockSlot (
 unsigned	slot)
{
	struct flock	lock;

	if (slot > MAX_TOKENS)
		return (FALSE);

	lock.l_type		= F_WRLCK;	/* place an exclusive lock */
	lock.l_whence	= 0;		/* relative to start of file */
	lock.l_start	= SlotPosn (slot);
	lock.l_len		= 1;		/* size of token */

	return (fcntl (lic_fd, F_SETLK, &lock) != -1);
}

static int
UnlockSlot (
 unsigned	slot)
{
	struct flock	lock;

	if (slot > MAX_TOKENS)
		return (FALSE);

	lock.l_type		= F_UNLCK;	/* unlock request */
	lock.l_whence	= 0;		/* relative to start of file */
	lock.l_start	= SlotPosn (slot);
	lock.l_len		= 1;		/* size of token */

	return (fcntl (lic_fd, F_SETLK, &lock) != -1);
}

static int
LockCount (void)
{
	int	count = 0;
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
        int filePos;

        filePos = sizeof (struct LIC_REC) - MAX_TOKENS + i;

        lock.l_type     = F_WRLCK;  /*      ask for exclusive locks */
        lock.l_whence   = 0;        /*      relative from start of file */
        lock.l_start    = filePos;
        lock.l_len      = 1;

        if (fcntl (lic_fd, F_GETLK, &lock) == -1)
        {
			/* oo. a really bad error, return MAX_TOKENS to be safe */
			return (MAX_TOKENS);
        }

        if (lock.l_type != F_UNLCK) /* no existing lock */
        {
            count++;
        }
    }

#else /* LINUX */

	lock.l_type		= F_WRLCK;			/*	ask for exclusive locks	*/
	lock.l_whence	= 0;				/*	relative from start of file */
	lock.l_start	= SlotPosn (0);		/*	from first slot position */
	lock.l_len		= 0;

	while (lock.l_type != F_UNLCK)
	{
		/*
		 * Ask for blocking write locks
		 */
		if (fcntl (lic_fd, F_GETLK, &lock) == -1)
		{
			/* oo. a really bad error, return MAX_TOKENS to be safe */
			return (MAX_TOKENS);
		}

		if (lock.l_type != F_UNLCK)
		{
			if (!lock.l_len)
			{
				/*	Locked until end of address space */
				count += MAX_TOKENS - lock.l_start;
				break;
			}
			count += lock.l_len;

			lock.l_start += lock.l_len;		/* begin from next token */
			lock.l_len = 0;					/* reset */
		}
	}

#endif /* LINUX */

	return (count);
}

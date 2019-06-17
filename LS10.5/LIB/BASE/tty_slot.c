#include	<stdlib.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdefs.h>
#include	<license2.h>

#define	TRUE	1
#define	FALSE	0

static void	BuildSlotMap (int, int []);

int		forceRead	=	FALSE;

/*====================================
| Find available Terminal free Slot. |
====================================*/
int
ttyslt()
{
	char	*progPath = getenv ("PROG_PATH");
	char	*termSlot = getenv ("TERM_SLOT");
	int		lic_fd;
	int		spare_slot = 1;
	char	fname [256];
	char	termSlotDesc [256];
	int		slots [MAX_TOKENS];

	if (termSlot && forceRead == FALSE)
		return atoi (termSlot);

	if (!progPath)
		return EXIT_FAILURE;

	/*---------------------------------
	| Clear out terminal slots array. |
	---------------------------------*/
	memset (slots, 0, sizeof slots);

	/*--------------------
	| Open Licence file. |
	--------------------*/
	sprintf (fname, "%s/BIN/LICENSE", progPath);
	if ((lic_fd = open (fname, O_WRONLY)) < 0)
	{
		fprintf (stderr, "Failed to open %s (%d)\n", fname, errno);
		return EXIT_FAILURE;
	}
	fcntl (lic_fd, F_SETFD, 1); /* Set close-on-exec flag */

	/*--------------------------
	| Build terminal slot map. |
	--------------------------*/
	BuildSlotMap (lic_fd, slots);

	/*----------------------------------------------
	| Keep trying until a spare slot is available. |
	----------------------------------------------*/
	while (slots [spare_slot])
	{
		spare_slot++;
		if (spare_slot >= MAX_TOKENS)
		{
			printf ("%d\n", LICENSE_OFLOW);
			return EXIT_FAILURE;
		}
	}
	sprintf (termSlotDesc, "TERM_SLOT=%d", spare_slot);
	putenv (strdup (termSlotDesc));

	/*-----------
	| Clean up. |
	-----------*/
	close (lic_fd);
	return (spare_slot);
}

static int
RealSlot (
 long posn)
{
	/*---------------------------------------------------------
	| Translates a file position to a Logistic terminal slot. |
	---------------------------------------------------------*/
	return posn + MAX_TOKENS - sizeof (struct LIC_REC);
}

static void
BuildSlotMap (
 int fd,
 int slots [])
{
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

		if (fcntl (fd, F_GETLK, &lock) == -1)
		{
			fprintf (stderr, "Internal error : fcntl() [%d]\n", errno);
			return;
		}

		if (lock.l_type == F_UNLCK) /* no existing lock */
		{
            /*-----------------------------
			| Mark position as not.
			-----------------------------*/
			slots [RealSlot (filePos)] = FALSE;
		}
		else /* lock exists */
		{
            /*-----------------------------
			| Mark position as allocated.
			-----------------------------*/
			slots [RealSlot (filePos)] = TRUE;
		}
	}

#else /* LINUX */

	lock.l_type		= F_WRLCK;	/*	ask for exclusive locks	*/
	lock.l_whence	= 0;		/*	relative from start of file */
	lock.l_start	= sizeof (struct LIC_REC) - MAX_TOKENS;
	lock.l_len		= 0;

	while (lock.l_type != F_UNLCK)
	{
		/*-------------------------------
		| Ask for blocking write locks. |
		-------------------------------*/
		if (fcntl (fd, F_GETLK, &lock) == -1)
		{
			fprintf (stderr, "Internal error : fcntl() [%d]\n", errno);
			return;
		}

		if (lock.l_type != F_UNLCK)
		{
			/*-----------------------------
			| Mark position as allocated. |
			-----------------------------*/
			slots [RealSlot (lock.l_start)] = TRUE;

			if (!lock.l_len)
			{
				/*------------------------------------
				| Locked until end of address space. |
				------------------------------------*/
				break;
			}

			lock.l_start += lock.l_len;		/* begin from next token */
			lock.l_len = 0;					/* reset */
		}
	}
#endif /* LINUX */
}


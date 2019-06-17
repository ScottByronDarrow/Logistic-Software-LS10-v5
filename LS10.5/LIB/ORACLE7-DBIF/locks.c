#ident	"$Id: locks.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Row Locking.
 *
 *	Talk with our lock-daemon, `alvin', to indicate our lock status
 *
 *******************************************************************************
 *	$Log: locks.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:12:32  cha
 *	Added crc.c and crc.h and lock.c to keep our sanity intact
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.4  2000/07/14 04:00:50  gerry
 *	Standardized code
 *	
 *	Revision 1.3  2000/07/13 11:08:45  raymund
 *	16-bit reversed CRC hardware emulation algorithm for row locking.
 *	
 *	Revision 1.2  1999/11/16 02:38:04  jonc
 *	Added required trailing newline.
 *	
 *	Revision 1.1  1999/11/15 02:53:06  jonc
 *	Added lock code. Requires `alvin' the lock-daemon to be running.
 *	
 */
#include	<errno.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<syslog.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>

#include	"oracledbif.h"

/*----------------------------------------------------+
| Include file for the hashing function.              |
| We are using the reversed 16-bit hardware CRC algo. |
+----------------------------------------------------*/
#include    "crc.h"

/*---------------------------+
| Local functions and macros |
+---------------------------*/
static int LockRowNoBlock (int fd, unsigned int row);
static int LockRowBlock (int fd, unsigned int row);
static void FreeRowLock(int fd, unsigned int row);
static int _LockRowNoBlock ( TableState *tablestate, RowLock ** list, 
                             const char * rowid);
static int _LockRowBlock ( TableState *tablestate, RowLock ** list, 
                           const char * rowid);

/*-------------------+
| External interface |
+-------------------*/
#define EOI 2025
#define MAXTIMER 	3
extern int restart;
extern int foreground(), getkey();
extern void print_mess( char *),
            clear_mess();
/*----------------+
| Local variables |
+----------------*/
char *lock_dir;

static void stub( int signo )
{
}

void
_LockInitialise ()
{
  /*------------------------------------------------------------+
  | Get the directory where the "lock" table files are located. |
  +------------------------------------------------------------*/
  lock_dir = strdup (getenv ("LOCK_DIR"));

  /*-------------------------------------------------------------+
  | Generate the table to be used in crc (hash value) generation |
  +-------------------------------------------------------------*/
  initialize_CRC_table ();
} 


void
_LockCleanup ()
{
  free (lock_dir);
}

int 
_TryLock( 
 char locktype, 
 TableState *tablestate, 
 RowLock **rowlock )
{
   switch( locktype )
   {
	 case 'u': /* Blocking lock */  
		/*------------------------------------------------------------+
		| Signals may cause the lock attempt to fail. So we must loop |
		| until the lock is OK.                                       |
	    +------------------------------------------------------------*/
	    while ( !_LockRowBlock( tablestate, rowlock, 
						   tablestate-> columns[tablestate->columnc - 1].data) )
			;
	    return TRUE;
     case 'w':  /* Non-blocking lock */
		if (!_LockRowNoBlock (tablestate, &tablestate -> locks, 
				   tablestate-> columns[tablestate->columnc - 1].data) )
		{
			struct sigaction on_alarm, old_alarm;
			int locked = FALSE;
			char message [128];

			if (!foreground ())
				return FALSE;

			/*
			 *	Sigh. We have to go into an active wait
			 *	loop with possible intervention by the user
			 */
			on_alarm.sa_handler = stub;		/* ignore on receipt */
			sigemptyset (&on_alarm.sa_mask);	/* clear all other options */
			on_alarm.sa_flags = 0;				/* signal should cause break */
			if (sigaction (SIGALRM, &on_alarm, &old_alarm) < 0)
			{
				fprintf (stderr, "sigaction failed on setup SIGALARM");
				exit (EXIT_FAILURE);
			}

			/*
			 *
			 */
			sprintf (message,
				"(%s) Record is locked - F16 to Restart",
				tablestate -> named);
			print_mess (message);

			do
			{
				int testchar = 0;

				alarm (2);
				if ((testchar = getkey ()) == EOI)
				{
					restart = 1;
					alarm (0);
					break;
				}
			}	while (!(locked = _LockRowNoBlock (tablestate, 
												   &tablestate -> locks, 
				          tablestate-> columns[tablestate->columnc - 1].data)));

			clear_mess ();

			/*
			 *	Restore old alarm handler
			 */
			sigaction (SIGALRM, &old_alarm, NULL);
			return locked;
		}
		return TRUE;  /* Didn't have to wait, the row was locked immediately */
   } /* switch */

   return FALSE;  /* Failed to lock if this point is reached */
}

int
_LockRowNoBlock (
 TableState *tablestate,
 RowLock ** list,
 const char * rowid)
{
	RowLock            *node = NULL;
	unsigned short int crc;
	char               tablename[255];
 
    /*----------------------------------------------------------------+
	| Has the current process attempted a lock on this table before?  |
	| If yes then use the file descriptor used. Otherwise, let's open |
	|   a descriptor ourselves.                                       |
	+----------------------------------------------------------------*/
	if (tablestate->fd_table == -1)  /* Not locked before by this process */
	{
	  /*----------------------------------------------+
	  | Open the file with the same name as the table |
	  +----------------------------------------------*/
	  strcpy (tablename, lock_dir);
	  if ( strlen(tablestate->table) == 0) 
		strcat (tablename, tablestate->named);
      else
		strcat (tablename, tablestate->table);
	  tablestate->fd_table = open( tablename, O_CREAT | O_EXCL | O_WRONLY,  
							   	   S_IRUSR | S_IWUSR | S_IXUSR | 
								   S_IRGRP | S_IWGRP | S_IXGRP );

	  if (tablestate->fd_table < 0) 
	  {
		 if (errno == EEXIST)
			tablestate->fd_table = open (tablename, O_WRONLY);
		 else
		 {
            oracledbif_error ("Cannot open LOCK file %s", tablename);
		 }
	  }
	}
	/*-------------------------------------------------------------------+
	| Now that we have a descriptor. Let's generate a hash value for the |
	| 32-bytes rowid.                                                    |
	+-------------------------------------------------------------------*/
    compute_revCRC_16 (rowid, &crc);

	/*----------------------+
	| Now lock that record! |
	+----------------------*/
    if (LockRowNoBlock (tablestate->fd_table, crc) < 0)
    {
       return FALSE;                 /* Cannot Lock the row */
    }
	/*-----------------------------------------------------------------+
	| Lock successfull! Do the codes that the old _LockRowNoBlock does.|
	| For compatability's sake. We just wanted alvin out.              |
	+-----------------------------------------------------------------*/
	memset (node = malloc (sizeof (RowLock)), 0, sizeof (RowLock));
	strcpy (node -> rowid, rowid);
	node -> next = *list;
	*list = node;

	return TRUE;                     /* Row was successfully locked */
}

int
_LockRowBlock (
 TableState *tablestate,
 RowLock ** list,
 const char * rowid)
{
	RowLock             *node = NULL;
	unsigned short int  crc;
	char                tablename[255];

    /*----------------------------------------------------------------+
	| Has the current process attempted a lock on this table before?  |
	| If yes then use the file descriptor used. Otherwise, let's open |
	|   a descriptor ourselves.                                       |
	+----------------------------------------------------------------*/
	if (tablestate->fd_table == -1)  /* Not locked before by this process */
	{
	  /*----------------------------------------------+
	  | Open the file with the same name as the table |
	  +----------------------------------------------*/
	  strcpy (tablename, lock_dir);
	  if ( strlen(tablestate->table) == 0) 
		strcat (tablename, tablestate->named);
          else
		strcat (tablename, tablestate->table);

	  tablestate->fd_table = open( tablename, O_CREAT | O_EXCL | O_WRONLY,  
							   	   S_IRUSR | S_IWUSR | S_IXUSR | 
								   S_IRGRP | S_IWGRP | S_IXGRP );

	  if (tablestate->fd_table < 0) 
	  {
		 if (errno == EEXIST)
			tablestate->fd_table = open (tablename, O_WRONLY);
		 else
		 {
            oracledbif_error ("Cannot open LOCK file %s", tablename);
		 }
	  }
	}
	/*-------------------------------------------------------------------+
	| Now that we have a descriptor. Let's generate a hash value for the |
	| 32-bytes rowid.                                                    |
	+-------------------------------------------------------------------*/
    compute_revCRC_16 (rowid, &crc);

	/*----------------------+
	| Now lock that record! |
	+----------------------*/
	if (LockRowBlock (tablestate->fd_table, crc) < 0) 
	{
	  return FALSE;  /* Cannot Lock the Row */
	}
	
	memset (node = malloc (sizeof (RowLock)), 0, sizeof (RowLock));
	strcpy (node -> rowid, rowid);
	node -> next = *list;
	*list = node;
	//sleep(1);

	return TRUE;                    /* Row was locked */
}

int
_LockFreeAll (
 TableState *tablestate,
 RowLock ** list)
{

	/*
	 *	Let's do this iteratively, for variety
	 */
	RowLock                 *l = *list;
	unsigned short int      crc;

	while (*list)
	{

		l = *list;
		*list = l -> next;

		/*---------------------------------------+
		| Get the CRC of the row then unblock it |
		+---------------------------------------*/
        compute_revCRC_16 (l -> rowid, &crc);
		FreeRowLock (tablestate->fd_table, crc);

		free (l);
	}


	return TRUE;
}


/*
+---------------------------+
| Row locking with blocking |
+---------------------------+
*/
int 
LockRowBlock (
 int fd, 
 unsigned int row)
{
   struct flock lock;

   lock.l_type   = F_WRLCK;
   lock.l_start  = (off_t) row;
   lock.l_whence = SEEK_SET;
   lock.l_len    = 1;
   return (fcntl (fd, F_SETLKW, &lock));

}

/*
+------------------------------+
| Row locking without blocking |
+------------------------------+
*/
int 
LockRowNoBlock (
 int fd,
 unsigned int row)
{
   struct flock lock;

   lock.l_type   = F_WRLCK;
   lock.l_start  = (off_t) row;
   lock.l_whence = SEEK_SET;
   lock.l_len    = 1;
   
   return ( fcntl( fd, F_SETLK, &lock ));
}

void
FreeRowLock (
 int fd, 
 unsigned int row)
{
   struct flock lock;

   lock.l_type   = F_UNLCK;
   lock.l_start  = (off_t) row;
   lock.l_whence = SEEK_SET;
   lock.l_len    = 1;
   fcntl (fd, F_SETLK, &lock);
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : ip_comms.c                                     |
|  Source Desc       : Inter-process comunications routines.          |
|                                                                     |
|  Library Routines  : IP*().                                         |
|---------------------------------------------------------------------|
|  Date Modified     : 08.02.95   | Modified by : Jonathan Chen       |
|                                                                     |
|  Comments                                                           |
|  (08.02.95) : check for possible existing fifo file.                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

static char *
PipeName (
 int	id,
 char	*buf)
{
	/*
	 *	Build the name of the pipe
	 */
	char	*env = getenv ("PROG_PATH");

	sprintf (buf, "%s/WORK/%06d", env ? env : "/usr/LS10.5", id);
	return (buf);
}

/*======================================================================
| IP_CREATE() - (i)   Creates unique pid file in $PROG_PATH/WORK/pid.  |
| 		(ii)  Creates a named pipe with read/write access.     |
|               (iii) Opens names pipe for reading.                    |
======================================================================*/
int
IP_CREATE (
 int	uniqid)
{
	char		file_name [101];
	struct stat	info;

	/*
	 *	Check for an existing fifo
	 */
	if (stat (PipeName (uniqid, file_name), &info))
	{
		if (errno != ENOENT)
			return (-1);				/* got an unexpected error */

		if (mknod (file_name, S_IFIFO | 0666, 0))		/* ugw=rw */
			return(-1);
	}
	else
	{
		/*
		 *	Something's already there
		 */
		if (!(info.st_mode & S_IFIFO))
			return (-1);				/* oops! not a fifo file */
	}

	return (open (file_name, O_RDONLY | O_NDELAY ));
}

/*===============================================
| IP_OPEN() - (i) Opens names pipe for writing. |
===============================================*/
int
IP_OPEN (
 int	uniqid)
{
	char	file_name[101];

	return (open (PipeName (uniqid, file_name), O_WRONLY));
}
/*====================================================
| IP_READ() - (i) Reads named pipe waiting for data. |
====================================================*/
void	IP_READ(int np_fn)
{
	char	rubbish[2];

	while ( read ( np_fn, rubbish, 1 ) != 1);

}
/*==========================================================
| IP_READ() - (i) Writes to named pipe releasing IP_READ() |
==========================================================*/
void	IP_WRITE(int np_fn)
{
	char	rubbish[2];

	while (write ( np_fn, rubbish, 1 ) != 1);
}
/*====================================
| IP_CLOSE() - (i) Closes named pipe |
====================================*/
void	IP_CLOSE(int np_fn)
{
	close ( np_fn );
}
/*=======================================
| IP_UNLINK() - (i) Unlinkes named pipe |
=======================================*/
void
IP_UNLINK (
 int	uniqid)
{
	char	file_name[101];

	unlink (PipeName (uniqid, file_name));
}

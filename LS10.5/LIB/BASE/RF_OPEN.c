/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_OPEN.c       )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (21.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (21.10.94) : Removed lotsa system dependancies                     |
|                                                                     |
=====================================================================*/
#include	<errno.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>

#include	<osdefs.h>

#ifdef	HAS_UNISTD_H
#include	<unistd.h>
#else
extern int		write (int, void *, unsigned);
extern off_t	lseek (int, off_t, int);
#endif

#include	"RF_UTIL.h"

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/*===========================================================================
| Work file buffer routines - includes RF_OPEN,RF_CLOSE,RF_ADD,RF_READ,         |
| wkrm and RF_REWIND.                                                        |
| These routines provide simple manipulation of so called 'buffer' files    |
| from within C programs in a way which is similar to that used for         |
| interfacing to the Informix database. In most cases the error codes       |
| that are returned are either system error codes or they are the same      |
| as the Informix return codes for similar calls. There is a maximum of     |
| 5 files that can open at any one time - but as files are closed then      |
| the slots previously occupied are freed for re-use. It is also            |
| assumed that only fixed length records a will be written to the file,     |
| so it is essential that the programmer pass the correct record size       |
| to the open routine. This can be done by using the 'sizeof' function.     |
| The open function returns into the last argument the 'file number' this   |
| is the reference by which these routines know the file as, so it is       |
| essential that this be passed to EVERY work file routine. Failure to      |
| do so will result in incorrect functioning and even memory corruption     |
| and or segmentation violations.                                           |
===========================================================================*/

/*
 *	Yet another set of bloody globals
 */
struct wkfile	fd_list [MAX_OPEN];

/*
 *	Local functions
 */
static int	findfree (void);

/* Pointer to work file number */
int
RF_OPEN(char *name, int size, char *mode, int *fd_ptr)
{
	int		i;
	int		pos;
	long	recsiz;
	static int	init_done = FALSE;

	/*-------------------------------
	| initialisation done		|
	-------------------------------*/
	if (!init_done)
	{
		memset (fd_list, 0, sizeof (fd_list));
		init_done = TRUE;
	}

	/*-------------------------------
	| null mode or filename		|
	-------------------------------*/
	if (*mode == '\0' || *name == '\0' || size <= 0)
		return(6014);
	/*-------------------------------
	| Check access mode of file.	|
	-------------------------------*/
	if (strchr("rwau",*mode) == (char *) 0)
		return(6014);
	/*-------------------------------
	| Room to open another file.	|
	-------------------------------*/
	pos = findfree();
	if (pos >= 0) 
	{
		/*------------------------
		| Create file.		 |
		------------------------*/
		if (*mode == 'w') 
			close (creat (name, 00666));

		/*-----------------------
		| Open existing file.	|
		-----------------------*/
		if ((i = open (name, O_RDWR)) < 0)
			return(errno);
		else 
		{
			fcntl (i, F_SETFD, 1);			/* set close-on-exec flag */

			fd_list [pos].fd_name = strdup (name);
			fd_list [pos].fd_rsiz = size;
			fd_list [pos].fd_fdesc = i;

			/*---------------------
			| Record record size. |
			---------------------*/
			if (*mode == 'w') 
			{ 
				recsiz = size;
				if (write (i, &recsiz, sizeof (recsiz)) != sizeof (recsiz))
					return(errno);
			}
			/*-----------------------------------------
			| Check record size against that written. |
			-----------------------------------------*/
			else 
			{
				if (read (i, &recsiz, sizeof (recsiz)) != sizeof (recsiz))
					return(errno);

				if (recsiz != size)
					return(200 + (size - (int) recsiz));
			}
					
			/*--------------------
			| Go to end of file. |
			--------------------*/
			if (*mode == 'a')
				lseek (i, 0, SEEK_END);

			*fd_ptr = pos;
			return(0);
		}
	}
	else 
	{
		/*----------------------
		| Too many files open. |
		----------------------*/
		return(6003);
	}
}

/*---------------------------------------------------------------------
| Routine to find the position of the next free file descriptor slot  |
| Returns: -1 if no slots are free, 0 - MAX_OPEN otherwise.           |
---------------------------------------------------------------------*/
static int
findfree (void)
{
	int	i;

	for (i = 0; i < MAX_OPEN; i++) 
		if (!fd_list [i].fd_name)
			return (i);
	return (-1);
}

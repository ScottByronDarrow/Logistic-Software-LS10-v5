/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_ADD.c        )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (21.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (21.10.94) : Cleaned up                                            |
|                                                                     |
=====================================================================*/
#include	<errno.h>

#include	<osdefs.h>

#ifdef	HAS_UNISTD_H
#include	<unistd.h>
#else
extern int		write (int, void *, unsigned);
extern off_t	lseek (int, off_t, int);
#endif

#include	"RF_UTIL.h"

/*==========================================================================
| Routine to add a record to a previously opened file - writes it at the   |
| current file pointer position.                                           |
| Returns: 6018 if file has not been opened, 0 if no error, system         |
| error code otherwise .                                                   |
==========================================================================*/
int
RF_ADD (
 int	fileno,
 char	*buff)
{
	if (fileno < 0 || fileno >= MAX_OPEN || fd_list [fileno].fd_fdesc < 0)
		return (6018);

	return (
		write (fd_list [fileno].fd_fdesc, buff, fd_list [fileno].fd_rsiz) ==
			fd_list [fileno].fd_rsiz ?
		0 : errno);
}

int
RF_UPDATE (
 int	fileno,
 char	*buff)
{
	/*-----------------------------------------------
	| Seek Backwards 1 record to record just read	|
	-----------------------------------------------*/
	if (lseek (
			fd_list [fileno].fd_fdesc, -fd_list [fileno].fd_rsiz, SEEK_CUR) < 0)
		return (errno);

	return (RF_ADD (fileno, buff));
}

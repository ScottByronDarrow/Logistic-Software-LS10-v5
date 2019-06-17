/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_REWIND.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (21.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (21.10.94) : Cleaned up                                            |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
#include	"RF_UTIL.h"

/*----------------------------------------------------------------------------
| Routine to reset file pointer to start of file - effectively to rewind it. |
| Returns: 6006 if file not open, 0 if no-error, non-zero otherwise.         |
----------------------------------------------------------------------------*/
int
RF_REWIND (
 int	_fileno)
{
	if (_fileno < 0 || _fileno >= MAX_OPEN)
		return(6006);

	/*-----------------------
	| Get to start of data. |
	-----------------------*/
	return (
		lseek (
			fd_list [_fileno].fd_fdesc,
			sizeof (long),	/* record size header */
			SEEK_SET) == sizeof (long) ?
		0 : errno);
}
	
/*=======================================
| Seek rec_no records into work file	|
| if seek fails return errno from	|
| failure else return 0.		|
=======================================*/
int
RF_SEEK (
 int	fileno,
 int	rec_no)
{
	return (
		lseek (
			fd_list [fileno].fd_fdesc,
			sizeof (long) /* record size header */
				+ rec_no * fd_list [fileno].fd_rsiz,
			SEEK_SET) < 0 ?
		errno : 0);
}

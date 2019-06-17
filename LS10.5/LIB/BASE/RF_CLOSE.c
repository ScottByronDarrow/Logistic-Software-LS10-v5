/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_CLOSE.c      )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (21.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (21.10.94) : Cleaned it up                                         |
|                                                                     |
| $Log: RF_CLOSE.c,v $
| Revision 5.1  2001/08/06 22:40:53  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:41  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:20  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.3  1999/09/13 09:36:35  scott
| Updated for Copyright
|
=====================================================================*/
#include	<std_decs.h>

#include	"RF_UTIL.h"

/*----------------------------------------------------------------------
| Routine to free the slot occupied by the file number _fileno.         |
| Frees up the memory used by the file name and sets file descriptor,  |
| and record size to 0.                                                |
----------------------------------------------------------------------*/
int
RF_CLOSE(int _fileno)
{
	int	r;

	if (_fileno < 0 || _fileno >= MAX_OPEN)
		return(6006);

	if (!fd_list [_fileno].fd_name)
		return (-1);						/* internal error */

	/*
	 * Free up system resources
	 */
	free (fd_list [_fileno].fd_name);
	r = close (fd_list [_fileno].fd_fdesc);

	/*
	 *	Flush entry clean
	 */
	memset (&fd_list [_fileno], 0, sizeof (struct wkfile));
	return (r ? errno : 0);
}

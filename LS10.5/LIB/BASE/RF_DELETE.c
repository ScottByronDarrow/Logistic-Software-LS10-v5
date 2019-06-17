/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_DELETE.c        )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

#include	"RF_UTIL.h"

/*=======================================
| Closes and removes file at same time. |
=======================================*/
int
RF_DELETE (
 int	fileno)
{
	int		err;
	char	*name;

	if (fileno < 0 || fileno >= MAX_OPEN || !fd_list [fileno].fd_name)
		return (-1);						/* internal error */

	name = strdup (fd_list [fileno].fd_name);
	if ((err = RF_CLOSE (fileno)))
		return (err);

	err = unlink (name);
	free (name);
	return (err ? errno : 0);
}

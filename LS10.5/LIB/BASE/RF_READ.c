/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_READ.c       )                                 |
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
/*===========================================================================
| Routine to read the next record from the previously opened work file.     |
| Returns: 105 if number of bytes returned does not equal the record size,  |
|           -1 if end of file,0 if no error, system error code otherwise.   |
===========================================================================*/
int
RF_READ(int _fileno, char *buff)
{
	int	i;

	if (_fileno < 0 || _fileno >= MAX_OPEN)
		return(6015);

	if ((i = read(fd_list[_fileno].fd_fdesc,buff,fd_list[_fileno].fd_rsiz)) != fd_list[_fileno].fd_rsiz) 
	{
		/*--------------
		| End of file. |
		--------------*/
		if (i == 0)
			return(-1);
		else
			return(105);
	}
	else
		return(0);
}

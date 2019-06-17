/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RF_DISPLAY.c       )                                 |
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
#include	<stdio.h>
#include	"RF_UTIL.h"
void
RF_DISPLAY(int _fileno)
{
	if (fd_list[_fileno].fd_fdesc >= 0)
	{
		fprintf(stderr,"fileno = %d ",_fileno); fflush(stderr);
		fprintf(stderr,"name [%s] ",fd_list[_fileno].fd_name);
		fprintf(stderr,"rsiz = %d ",fd_list[_fileno].fd_rsiz);
		fprintf(stderr,"mode = %d ",fd_list[_fileno].fd_mode);
		fprintf(stderr,"fdesc = %d\n",fd_list[_fileno].fd_fdesc);
	}
	else
		fprintf(stderr,"Invalid work file number %d\n",_fileno);
}

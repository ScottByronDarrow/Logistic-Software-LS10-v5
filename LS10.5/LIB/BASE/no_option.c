/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( no_option.c    )                                 |
|  Program Desc  : ( Routine to print message when open not       )   |
|                  ( allowed due to environment variable.         )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 11/09/90         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

void
no_option(char *env_desc)
{
	int	env_len = 0;

	char	no_opstr[255];

     	clear();
     	box(1,7,78,5);

     	rv_pr(" Current system configuration does not allow this option. ",
		12, 8, 1);

     	rv_pr(" Please Refer to Your Environment Variable Manual on : ",
		14, 10, 1);

	sprintf(no_opstr,"  : %s ", env_desc);
	env_len  = strlen(no_opstr);

     	rv_pr(no_opstr, 40 - (env_len / 2), 12,1);

	for (env_len = 0; env_len < 10; env_len++)
	{
		putchar(BELL);
		fflush(stdout);
		sleep(1);
	}
}

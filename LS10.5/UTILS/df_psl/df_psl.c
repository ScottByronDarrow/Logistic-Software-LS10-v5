/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( df_psl.c       )                                 |
|  Program Desc  : ( Get amount of free disk space.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (14/02/90)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (19/05/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (14/10/94)      | Modified  by : Marnie Organo     |
|                                                                     |
|  Comments      :                                                    |
|  (19/05/93)    : Fix to force usage of ACTUAL local storage.        |
|  (14/10/97)    : Changed printf to print_at for Multilingual        |
|                : Conversion.                                        |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: df_psl.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/df_psl/df_psl.c,v 5.2 2001/08/09 09:26:49 scott Exp $";

#include	<pslscr.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef	LINUX
#include	<sys/ustat.h>
#else
#include	<ustat.h>
#endif	/* LINUX */
/*=======================================
| Main Processing Routine.		|
=======================================*/
char	*path = "/usr/LS10.5/DATA";
struct	stat	stat_buf;
struct	ustat	ustat_buf;
int	dev;


int
main (
 int	argc,
 char *	argv [])
{
	if (stat (path, &stat_buf))
/*==============================================
| if /usr/LS10.5/DATA doesn't exist, just return 0 |
==============================================*/
		print_at (0,0,"0\n");
	else
	{
		dev = stat_buf.st_dev;
		if (ustat (dev, &ustat_buf))
			print_at (1,0,"0\n");
		else
			print_at (1,0,"%d\n", ustat_buf.f_tfree);
	}
	return (EXIT_SUCCESS);
}

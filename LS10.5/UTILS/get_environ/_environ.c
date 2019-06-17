/*=====================================================================
|  Copyright (C) 1988 Logistic Software Limited.                      |
|=====================================================================|
|  Program Name  : ( get_environ.c  )                                 |
|  Program Desc  : ( Display Environment Variable                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (12/05/88)      | Author      : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (15/06/94)      | Modified by : Jonathan Chen      |
|  Date Modified : (12/09/97)      | Modified by : Marnie Organo      |
|  Date Modified : (06/10/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                    |
|   (15/06/94)   : Removed calls to unpublished interface             |
|   (12/09/97)   : Updated  for Multilingual Conversion .			  |
|   (06/10/1999) : Ported to ANSI standards.            			  |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: _environ.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/get_environ/_environ.c,v 5.1 2001/08/09 09:26:57 scott Exp $";

#include	<pslscr.h>
#include	<stdio.h>
#include	<std_decs.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		i;
	char *	sptr;

	if (argc == 1)
	{
		/*printf ("Usage : %s variable_name ...\n", argv[0]);*/
		print_at (0,0,ML(mlUtilsMess700), argv[0]);
		return (EXIT_FAILURE);
	}

	for (i=1; i<argc; i++)
	{
		sptr = chk_env (argv[i]);
		/*print_at (0,0,"chk_env (%s) = %s\n", argv [i], sptr ? sptr : "(null)");*/
		print_at (0,0,ML(mlUtilsMess058), argv [i], sptr ? sptr : "(null)");
	}
	return (EXIT_SUCCESS);
}

/*=====================================================================
|  Copyright (C) 1990 Logistic Software Limited.                      |
|=====================================================================|
|  Program Name  : ( run_nice.)                                       |
|  Program Desc  : ( Run nice priority.                 	  )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/05/90         |
|---------------------------------------------------------------------|
|  Date Modified :  15/10/97       | Modified  by  :Marnie Organo     |
|                                                                     |
|  Comments      :                                                    |
| (15/10/97)     : Updated to Multilingual Conversion.                |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: run_nice.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/run_nice/run_nice.c,v 5.1 2001/08/09 09:49:50 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#include <pslscr.h>
#include <ml_misc_mess.h>

/*====================
|   Local variables   |
 ====================*/

int priority;

/*===========================
| Main processing Routines. |
===========================*/
int
main (
 int    argc, 
 char   *argv[])
{
	if (argc != 3)
	{
		/*printf ("Usage %s [nice priority] [program]\n\r",argv[0]);*/
		print_at (0, 0, ML (mlMiscMess704), argv[0]);
		return (EXIT_SUCCESS);
	}
	priority = atoi (argv[1]);

	/*printf ("Setting nice priority to : %d and running [%s]\n\r", nice(priority), argv[2] );*/
	print_at (1, 0, ML (mlMiscMess705), nice(priority), argv[2]);

	if (fork () == 0)
	{
		execlp (argv[2], argv[2], (char *)0 );
	}
	else
	{
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/* [ end of file ] */

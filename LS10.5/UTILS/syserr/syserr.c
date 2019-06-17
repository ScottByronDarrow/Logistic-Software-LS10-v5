/*=====================================================================
|  Copyright (C) 1986 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( syserr.c     )                                   |
|  Program Desc  : ( System Error.                                  ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+*/
char	*PNAME = "$RCSfile: syserr.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/syserr/syserr.c,v 5.2 2002/08/14 06:55:37 scott Exp $";
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#define	VDATE	"03/03/89" 	/* Date last compiled */

extern	int	sys_nerr;
#ifndef	LINUX
extern	char	*sys_errlist[];
#endif

/*=======================
| Function Declarations |
=======================*/
void printerr (int errno);
void printall (void);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;

	if (argc <= 1)
	{
		printall();
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[1],"-v") == 0)
	{
		printf("syserr: Date of last revision of codes was :%s\n",VDATE);
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[1],"-a") == 0)
		printall();
	else
	{
		for (i = 1;i < argc;i++)
			printerr(atoi(argv[i]));
	}
	return (EXIT_SUCCESS);
}

void
printerr (
 int errno)
{

	if (errno < 0 || errno > sys_nerr)
	{
		printf("No error message known for error %d\n",errno);
		return;
	}

	printf("Error %d is: %s\n",errno,sys_errlist[errno]);

	return;
}

void
printall (
 void)
{
	int	cnt = 0;
	int	i;

	printf("Code   :     Message     (Last compiled on %s) \n",VDATE);
	printf("===============================================================\n");

	for (i = 0;i < sys_nerr;i++)
	{
		printf("| %5d  : %-50.50s |\n",i,sys_errlist[i]);
		cnt++;
	}

	printf("===============================================================\n");
	printf("\n %d error codes printed\n",cnt);
}

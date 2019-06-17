/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( psl_what.c      )                                | 
|  Program Desc  : ( O/S independant what.                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (25/11/92)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (23/12/92)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      :                                                    |
|  (23/12/92)    : Changed to read data in large (16K) chunks.        |
|                                                                     |
=====================================================================*/
#define		NO_SCRGEN
#define		CCMAIN
#include	<pslscr.h>

char	*PNAME = "$RCSfile: psl_what.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_what/psl_what.c,v 5.1 2001/08/09 09:27:33 scott Exp $";

/*=======================
| Function Declarations |
=======================*/
void process (char *pathname);
void proc_str (FILE *fin);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;

	if (argc == 1)
		proc_str (stdin);
	else
	{
		for (i = 1; i < argc; i++)
			process (argv[i]);
	}
	return (EXIT_SUCCESS);
}

void
process (
 char *pathname)
{
	FILE *fin;

	fin = fopen (pathname, "r");
	if (fin == (FILE *) NULL)
	{
		printf ("can't open %s (26)\n", pathname);
		return;
	}
	printf ("%s:\n", pathname);
	proc_str (fin);
	fclose (fin);
}

void
proc_str (
 FILE *fin)
{
	char	rd_buf[16384];
	char	*sptr;
	int	offset = 0;
	int	cnt = 0;

	cnt = fread (rd_buf, 1, 16384, fin);
	while (cnt > 0)
	{
		for (sptr = rd_buf, offset = 0; offset < cnt; offset++, sptr++)
		{
			if (*sptr == '@')
				if (!strncmp (sptr, "@(#) - ", 7))
					printf ("\t%s\n", sptr + 4);
		}
		cnt = fread (rd_buf, 1, 16384, fin);
	}
}

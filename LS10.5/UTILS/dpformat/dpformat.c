/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( dpformat.c     )                                 |
|  Program Desc  : ( Dual ported pformat.                         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : 24/04/92        |  Author      : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (05/10/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|   (05/10/1999) : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dpformat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/dpformat/dpformat.c,v 5.1 2001/08/09 09:26:50 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>

#ifndef	LINUX
extern	int	errno;
#endif	/* LINUX */
FILE	*pout1,
		*pout2;

/*===========================
| Local function prototypes |
===========================*/
void	open_output		(void);
void	process			(void);
void	close_output	(void);


int
main (
 int	argc,
 char *	argv [])
{
	open_output ();
	process ();
	close_output ();
	return (EXIT_SUCCESS);
}

void
open_output (
 void)
{
	pout1 = popen ("pformat", "w");
	if (pout1 == (FILE *) NULL)
		sys_err ("Error in pformat During POPEN", errno, PNAME);
	pout2 = popen ("pformat", "w");
	if (pout2 == (FILE *) NULL)
		sys_err ("Error in pformat During POPEN", errno, PNAME);
}

void
process (
 void)
{
	char *	sptr,
			buffer[1024];

	sptr = gets (buffer);
	while (!feof (stdin))
	{
		if (strncmp (buffer, ".LP", 3))
		{
			fprintf (pout1, "%s\n", buffer);
			fprintf (pout2, "%s\n", buffer);
		}
		else
		{
			fprintf (pout1, ".LP1\n");
			fprintf (pout2, "%s\n", buffer);
		}
		sptr = gets (buffer);
	}
}

void
close_output (
 void)
{
	fflush (pout1);
	fflush (pout2);
	pclose (pout1);
	pclose (pout2);
}

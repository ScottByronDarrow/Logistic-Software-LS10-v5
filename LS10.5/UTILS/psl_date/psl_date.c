/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( psl_date.c      )                                |
|  Program Desc  : ( Allow printing of 'bias'ed dates.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (03/05/93)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified  by : Marnie Organo     |
|                                                                     |
|  Comments      :                                                    |
|  (12/09/97)    : Updated for Multilingual Conversion.               |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_date.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_date/psl_date.c,v 5.2 2001/08/09 09:27:24 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_utils_mess.h>

int
main (
 int                argc,
 char*              argv[])
{
	long	today,
            offset;

	if (argc != 2)
	{
		print_at (0,0,mlUtilsMess702, argv[0]);
		return (EXIT_FAILURE);
	}

	offset = atol (argv[1]);
	today = TodaysDate ();
	print_at (0,0,"%s", DateToString (today + offset));
    return (EXIT_SUCCESS);
}

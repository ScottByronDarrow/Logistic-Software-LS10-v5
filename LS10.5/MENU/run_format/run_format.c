/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( run_format.c   )                                 |
|  Program Desc  : ( Pipe File contents to pformat.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :   N/A,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Date Written  : 18/03/88        | Author      : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (18/03/88)      | Modified by : Roger Gibbison.    |
|  Date Modified : (13/06/94)      | Modified by : Jonathan Chen      |
|  Date Modified : (03/09/97)      | Modified by : Ana Marie Tario    |
|  Date Modified : (03/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                    |
|   (13/06/94)   : Cleaned up some irrelavant code & constructs       |
|   (03/09/97)   : Incorporated multilingual conversion and DMY4 date.|
|   (03/09/1999) : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: run_format.c,v $
| Revision 5.1  2001/08/09 05:13:53  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:09:06  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:26  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:36  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:47:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 09:42:03  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/17 07:27:17  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:57  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: run_format.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/run_format/run_format.c,v 5.1 2001/08/09 05:13:53 scott Exp $";

#include	<pslscr.h>
#include	<ml_menu_mess.h>
#include	<ml_std_mess.h>
#include	<errno.h>
#include	<stdio.h>

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	FILE *	fin;
	FILE *	fout;
	int		i;
	int		ndelete;
	char	line [201];
	char *	sptr;

	if (argc == 1)
	{
		print_at(0,0, mlMenuMess708, argv[0]);
		return (EXIT_FAILURE);
	}

	/*-----------------------------------------------
	| Set ndelete according to first parameter		|
	-----------------------------------------------*/
	ndelete = !strcmp(argv[1],"-d");

	/*-----------------------
	| For All filenames	|
	-----------------------*/
	for (i = (ndelete) ? 2 : 1;i < argc;i++)
	{
		if (!(fin = fopen (argv[i], "r")))
		{
			sprintf (line, "Error in %s during (FOPEN)", argv[i]);
			sys_err (line, errno, PNAME);
		}

		if (!(fout = popen ("pformat", "w")))
			sys_err("Error in pformat during (POPEN)",errno,PNAME);

		/*-----------------------
		| Read lines from input	|
		-----------------------*/
		while ((sptr = fgets (line, sizeof (line), fin)))
		{
			fprintf(fout,"%s",sptr);
			fflush(fout);
		}

		fclose(fin);
		pclose(fout);

		/*-------------
		| Delete file |
		-------------*/
		if (ndelete && unlink(argv[i]) == -1)
		{
			sprintf (line, "Error in %s during (UNLINK)", argv[i]);
			sys_err (line, errno, PNAME);
		}
	}
	return (EXIT_SUCCESS);
}

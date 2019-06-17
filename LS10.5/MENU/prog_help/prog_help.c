/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( prog_help.c    )                                 |
|  Program Desc  : ( Window for program help.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 31/12/87         |
|---------------------------------------------------------------------|
|  Date Modified : (31/12/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (05/12/90)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (22/09/97)      | Modified  by  : Rowena S Maandig | 
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A.Pacheco  | 
|                                                                     |
|  Comments      : (05/12/90) - Total re-write.                       |
|                : (22/09/97) - Updated to incorporate multilingual   |
|                               conversion.                           |
|                : (03/09/1999) - Ported to ANSI standards            |
|                :                                                    |
|                :                                                    |
| $Log: prog_help.c,v $
| Revision 5.2  2001/08/09 05:13:40  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:32  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:43  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:14  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:47:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/16 09:41:58  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.7  1999/09/17 07:27:04  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 02:36:53  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: prog_help.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/prog_help/prog_help.c,v 5.2 2001/08/09 05:13:40 scott Exp $";

#define	X_OFF	0
#define	Y_OFF	0
#define		NO_SCRGEN
#include	<pslscr.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

FILE *	fin;

char	filename [200];
int		silent;

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
void	proc_file		(void);


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	prog_path = getenv("PROG_PATH");

	if (!strcmp( argv[0], "sprog_help"))
		silent = TRUE;
	else
		silent = FALSE;
	if (argc != 2)
	{
		print_at (0, 0, mlMenuMess081, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	crsr_off ();
	set_tty ();

	if (strstr (argv [1], ".c" ))
	{
		sprintf(filename,"%s/BIN/HELP/%s", (prog_path != (char *)0) ? 
				prog_path : "/usr/LS10.5", argv[1]);
	}
	else
	{
		sprintf(filename,"%s/BIN/HELP/%s.c", (prog_path != (char *)0) ? 
				prog_path : "/usr/LS10.5", argv[1]);
	}

	filename [strlen (filename) - 1] = 'd';

	if (access (filename, 00) != 0)
	{
		sprintf(filename,"%s/BIN/HELP/sorry.d",
			(prog_path != (char *)0) ? prog_path : "/usr/LS10.5");
	}

	if ((fin = fopen (filename, "r")) == 0)
	{
		rv_pr(ML(mlMenuMess083), 2,0,1);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	proc_file();
	fclose(fin);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	rset_tty ();
	crsr_on ();
}

void
proc_file (
 void)
{
	char	data_str [201];
	char *	sptr;

	Dsp_prn_open( 0, 0, 18, "Pinnacle help Screen", (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0 );

	if ( silent )
	{
		Dsp_saverec("                                                                              ");
		Dsp_saverec("");
		Dsp_saverec("");
	}
	else
	{
		Dsp_saverec("                    L O G I S T I C    H E L P   W I N D O W.                 ");
		Dsp_saverec("");
		Dsp_saverec("[ PRINT ]  [ NEXT SCREEN ]  [ PREV SCREEN ]  [ EDIT / END ]");
	}

	sptr = fgets( data_str, 200, fin);
	while ( sptr != (char *)0 )
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		if ( data_str[0] == '~' )
			Dsp_saverec("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		else
			Dsp_saverec( sptr );

		sptr = fgets( data_str, 200, fin);
	}
	Dsp_srch();
	Dsp_close();
}

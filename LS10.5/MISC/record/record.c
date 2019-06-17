/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( record.c    )                                    |
|  Program Desc  : ( i have no idea what this does )                  |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,                                             |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,                                             |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : unknown         | Date Written  : unknown          |
|---------------------------------------------------------------------|
|  Date Modified : (28/09/1999)    | Modified by : edge cabalfin      |
|  
|  Comments      :
|  (28/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                : added this header                                  |
|                                                                     |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: record.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/record/record.c,v 5.2 2001/08/09 09:49:49 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#include	<pslscr.h>
#include	<ml_misc_mess.h>

/*====================
|   Local variables   |
 ====================*/
int     wr_log = FALSE;
FILE	*menu;
FILE	*fld_output;

/*==============================
|   Local function prototypes   |
 ==============================*/
void run_log (void);
void make_log (void);

/*==============================
|   Main Processing Function    |
mount
 ==============================*/
int
main (
 int    argc, 
 char  *argv[])
{
	if (argc != 2)
	{
		/*printf ("Usage :-\n\t%s <filename>\007\n\n", argv[0]);*/
		print_at (0,0,ML(mlMiscMess703), argv[0]);
		return (EXIT_FAILURE);
	}

	if (strcmp (argv[0], "record") == 0)
	{
		wr_log = TRUE;
		if ((fld_output = fopen (argv[1], "w")) == NULL)
		{
			sys_err ("Error in logfile During (FOPEN)", errno, PNAME);
		}
	}
	else
	{
		if ((fld_output = fopen (argv[1], "r")) == NULL)
		{
			sys_err ("Error in logfile During (FOPEN)", errno, PNAME);
		}
	}

	if ((menu = popen ("/usr/LS10.5/BIN/MENU/menu", "w")) == NULL)
	{
		sys_err ("Error in menu During (POPEN)", errno, PNAME);
	}

	init_scr ();
	set_tty ();
	sleep (sleepTime);	/* Allow a little time for menu to load!!	*/

	if (wr_log)
	{
		make_log ();
	}
	else
	{
		run_log ();
	}

	fclose (fld_output);
	pclose (menu);

	sleep (sleepTime);	/* Allow some time for menu to terminate	*/
	rset_tty ();

	return (EXIT_SUCCESS);
}

void
make_log (
 void)
{
	cc = fgetc (stdin);
	while (cc != EOF)
	{
		fputc (cc, menu);
		fflush (menu);
		fputc (cc, fld_output);
		fflush (fld_output);
		cc = fgetc (stdin);
		fflush (stdin);
	}
}

void
run_log (
 void)
{
	cc = fgetc (fld_output);
	while (cc != EOF)
	{
		fputc (cc, menu);
		fflush (menu);
		if (cc == '\r')
		{
			sleep (sleepTime);
		}
		if (cc < ' ')
		{
			sleep (sleepTime);
		}
		cc = fgetc (fld_output);
	}
}

/* [ end of file ] */

/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( chain.c, lp_chain.c & su_chain.c )               |
|  Program Desc  : ( Chains Programs From The Menu.               )   |
|                  ( NOTE chain must be linked with lp_chain.     )   |
|                  ( NOTE su_chain ISN'T linked! However, it MUST )   |
|                  (      be owned by root with 4777 permissions! )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (02/09/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (06.12.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (31.01.95)      | Modified by : Jonathan Chen      |
|  Date Modified : (11/01/96)      | Modified by : Scott B Darrow.    |
|  Date Modified : (10/09/97)      | Modified by : Roanna Marcelino.  |
|  Date Modified : (27/08/1999)    | Modified by : Alvin Misalucha.   |
|                                                                     |
|  Comments      : chain is for normal streams lp_chain includes LPNO.|
|                :                                                    |
| (02/09/91)     : Made changes for compatibility problem with AIX.   |
| (06.12.94)     : Use of fork() instead of system()                  |
|                : Removed unused code on return exit codes           |
| (31.01.95)     : 11721 Fixed : not exiting on non-zero              |
| (11/01/96)     : PDL - Updated to not allow DEL while chain is      |
|                :       running as this has been the cause journals  |
|                :       not posting etc.                             |
| (10/09/97)     : Modify for Multilingual Conversion.				  |
| (27/08/1999)   : Converted to ANSI format.				          |
|                                                                     |
| $Log: chain.c,v $
| Revision 5.1  2001/08/09 05:13:16  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:07:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:03  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:47:08  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 09:41:54  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.11  1999/09/17 07:26:50  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.10  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.9  1999/06/15 02:31:45  scott
| update to add log file + change database name etc.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: chain.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/chain/chain.c,v 5.1 2001/08/09 05:13:16 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<pwd.h>
#include	<sys/wait.h>

#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>

/*
 *	Function declarations
 */
int		main	(int argc, char * argv []);

static int		isquote		(char c);
static char *	subst		(char *word, const char *match, int num);
static int		ExecCommand	(pid_t pid, int pno, const char *command);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	int		i,
			pno = 1;
	int		iRetMain = EXIT_SUCCESS;
	char	*sptr;
	pid_t	pid = getpid ();

	sptr = strrchr (argv[0], '/');
	if (!sptr)
		sptr = argv[0];

	init_scr();
	set_tty();

	if ( !strcmp (sptr, "lp_chain") )
	{
		pno = get_lpno(1);
		clear();
		print_at(0,0,ML(mlStdMess035));
		fflush(stdout);
	}

	for (i = 1; i < argc; i++)
	{
		int		status;

		switch (fork ())
		{
		case -1	:						/* major failure! */
			iRetMain = EXIT_FAILURE;	/* what else can we do? */
			break;

		case 0	:		/* child */
			if (ExecCommand (pid, pno, argv [i]) != 0)
				iRetMain = EXIT_FAILURE;
			break;

		default	:		/* parent */
			wait (&status);

			/*
			 *	Check return status
			 */
#ifdef	_AIX
			if (WEXITSTATUS (status))
#else
			if (WEXITSTATUS (status) ||			/* !normal exit */
				WCOREDUMP (status))				/* core dump */
#endif
			{
				/*
				 *	abnormal exits or core-dumps result in immediate exit
				 */
				iRetMain = status;
				break;
			}
		}
		if (iRetMain != EXIT_SUCCESS)
			break;
	}
	return (iRetMain);
}

static int
isquote (
 char	c)
{
	return (c == '\'' || c == '"');
}

static char *
subst (
 char *			word,
 const char	*	match,
 int			num)
{
	static char	number [10];

	if (strcmp (word, match))
		return (word);

	sprintf (number, "%d", num);
	return (number);
}

static int
ExecCommand (
 pid_t			pid,
 int			pno,
 const char	*	command)
{
	int		c,
			argi = 0,
			len;
	char	quote = '\0';
	char	cmd [128],		/* here's hoping nothing's longer that 127 chars */
			*args [128],	/* here's hoping no program has > 128 args */
			*word;

	char	*LpNo = "LPNO",
			*PId = "PID";

	/*
	 *	Split the command line for exec
	 */
	len = strlen (strcpy (cmd, command));
	for (word = NULL, c = 0; c < len; c++)
	{
		if (word)
		{
			int	endword = FALSE;

			if (!quote && isspace (cmd [c]))
			{
				cmd [c] = '\0';				/* zap unquoted whitespace */
				endword = TRUE;

			} else if (quote == cmd [c])
			{
				cmd [c] = '\0';				/* zap trailing quote */
				quote = '\0';				/* reset */
				endword = TRUE;
			}

			if (endword)
			{
				/*	Handle word substitutions
				 */
				word = subst (word, PId, (int) pid);
				word = subst (word, LpNo, pno);
				args [argi++] = strdup (word);/* add to args */
				word = NULL;				/* prep for next word */
				endword = FALSE;
			}
		}
		else
		{
			if (isquote (cmd [c]))
			{
				quote = cmd [c];
				word = cmd + c + 1;
			}
			else if (!isspace (cmd [c]))
				word = cmd + c;				/* beginning of next word */
		}
	}

	if (word)
	{
		word = subst (word, PId, (int) pid);
		word = subst (word, LpNo, pno);
		args [argi++] = strdup (word);/* add to args */
	}
	args [argi] = NULL;								/* trailing null */

	return (execvp (args [0], args));						/* run it */
}

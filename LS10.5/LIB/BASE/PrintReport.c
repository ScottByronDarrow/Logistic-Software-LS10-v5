/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: PrintReport.c,v 5.1 2001/07/25 00:43:27 scott Exp $
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : PrintReport.c                                 |
|  Source Desc       : G/L report printing control routines.          |
|                                                                     |
|  Library Routines  : Print_*().                                     |
|---------------------------------------------------------------------|
| $Log: PrintReport.c,v $
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
=====================================================================*/
#include	<std_decs.h>
#include	<hot_keys.h>

static int	PrintFunction 	(int, KEY_PTR),
			SplitCommand 	(char **, char *);

#ifdef GVISION
static	KEY_TAB	printKeys [] =
	{
	   { " FOREGROUND ", 'F', PrintFunction, "Run report in foreground." },
	   { " BACKGROUND ", 'B', PrintFunction, "Run report in background." },
	   { " OVER-NIGHT ", 'O', PrintFunction, "Run report over-night."    },
	   END_KEYS
	};
#else
static	KEY_TAB	printKeys [] =
	{
	   { "[F]OREGROUND", 'F', PrintFunction, "Run report in foreground." },
	   { "[B]ACKGROUND", 'B', PrintFunction, "Run report in background." },
	   { "[O]VER-NIGHT", 'O', PrintFunction, "Run report over-night."    },
	   END_KEYS
	};
#endif

static	char	*s_ptr, *m_ptr;

/*
 **************************************************************************
 *	Function	:	PrintReport ()
 **************************************************************************
 * 	Description	:	Interactively choose report run time.
 * 
 * 	Notes		:	Print_report allows the user to decidee whether
 * 					the report about to be run is in forground or
 * 					background or even delayed and run over-night.
 * 			
 * 	Parameters	:	s     - Report and arguments.
 * 					msg   - Message to be diplayed while report runs.
 * 					width - Width of current screen.
 * 
 * 	Globals		:	s_ptr - Pointer to s.
 * 					m_ptr - Pointer to msg.
 */
void
PrintReport (
	char 	*s, 
	char 	*msg, 
	int		width)
{
	s_ptr = s;
	m_ptr = msg;

	line_at (21, 0, width);
	disp_hotkeys (21, 0, width, printKeys);
	run_hotkeys (printKeys, null_func, null_func);
}

/*
 **************************************************************************
 *	Function	:	PrintReport ()
 **************************************************************************
 * 	Function	:	PrintFunction ()
 * 
 * 	Description	:	Run report in specified mode.
 * 
 * 	Notes		:	Depending upon the mode selected in run_report
 * 					PrintFunction either runs the report in foreground,
 * 					background or over-night.
 * 			
 * 	Parameters	:	c - (F)oreground.
 * 				    	(B)ackgroud.
 * 				    	(O)ver-night.
 * 
 * 	Globals		:	s_ptr - Pointer to string to be run.
 * 					m_ptr - Pointer to message t be displayed.
 * 
 * 	Returns		:	FN16
 */
static	int	
PrintFunction (
	int		c, 
	KEY_PTR unused)
{
	char	*args [128];		/* here's hoping there's nothing > 128 args */

	switch (fork ())
	{
	case -1	:
		print_at (23, 0, "%RFAILED TO PRINT!");
		break;

	case 0	:	/* child */
		if (c == 'O')			/* overnight job? */
		{
			int	argc;

			args [0] 		= "ONIGHT";
			argc 			= SplitCommand (args + 1, s_ptr);
			args [argc++] 	= strdup (PNAME);
			args [argc] 	= NULL;
		}
		else
			SplitCommand (args, s_ptr);

		execvp (args [0], args);
		exit (1);				/* major failure */

	default	:	/* parent */
		print_at (23, 0, "%RPrinting %s ... ", m_ptr);
		if (c != 'B')
			wait (NULL);		/* wait if not background job */
		clear_mess ();
	}
	return (FN16);
}

static int
SplitCommand (
 char	**argv,
 char	*command)
{
	char	quote = '\0',
			*word,
			*cmd = strdup (command);
	int		c,
			argi = 0,
			len = strlen (cmd);

	/*
	 *	Split the command line for exec
	 */
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
				argv [argi++] = strdup (word);/* add to args */
				word = NULL;				/* prep for next word */
				endword = FALSE;
			}
		}
		else
		{
			if (cmd [c] == '\'' || cmd [c] == '"')
			{
				quote = cmd [c];
				word = cmd + c + 1;
			}
			else if (!isspace (cmd [c]))
				word = cmd + c;				/* beginning of next word */
		}
	}

	if (word)
		argv [argi++] = strdup (word);		/* add to args */
	argv [argi++] = NULL;					/* trailing null */

	free (cmd);
	return (argi);
}

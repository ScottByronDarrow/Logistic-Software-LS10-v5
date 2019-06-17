/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sys_exec.c,v 5.1 2001/07/25 00:43:30 scott Exp $
|  Source Name       : sys_exec.c                                     |
|  Source Desc       : Execute program(s)                             |
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|                                                                     |
|  Library Routines  : sys_exec()                                     |
-----------------------------------------------------------------------
	$Log: sys_exec.c,v $
	Revision 5.1  2001/07/25 00:43:30  scott
	New library for 10.5
	
	Revision 5.0  2001/06/19 06:59:39  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.2  2001/06/15 02:59:00  cha
	Updated from texting on HP.
	Improved sys_exec to be more bullet proof.
	Improved open_env to file cannot be removed. (we hope)
	
	Revision 4.1  2001/06/14 09:34:27  scott
	Updated - testing
	
=====================================================================*/
#include	<std_decs.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/wait.h>

extern	char	*str_token ();

int	
sys_exec (
	char	*systemString)
{
	return (SystemExec (systemString, FALSE));
}

/*
.function
	Function	:	SystemExec ()

	Description	:	Run system string using exec.

	Notes		:	This function is called in the same manner as
				system (S). However rather than using the 
				system (S)) call exec is called.
			
	Parameters	:	sys_str	-	String to execute.

	Returns		:	TRUE	-	If the call succeeds.
					FALSE	- 	if the call fails.
.end
*/
int
SystemExec (
	char	*systemString,
	int		backGround)
{
	char	tmp_str [1024],
			*tok_ptr;
	char	*argv [24];
	int		status	=	0,
			cnt;

	void 	(*old_sigvec) ();

	strcpy (tmp_str, systemString);

	tok_ptr = str_token (tmp_str, " ");

	for (cnt = 0; tok_ptr; cnt++)
	{
		argv [cnt] = tok_ptr;
		tok_ptr = str_token (NULL, " ");
	}

	argv [cnt] = NULL;

	old_sigvec	=	signal (SIGCLD, SIG_DFL);

	if (backGround)
	{
		switch (fork())
		{
		case -1:
			signal (SIGCLD, old_sigvec);
			return 0;
	
		case 0:
			/*
		 	*	Child process
		 	*/
			status	=	execvp (argv [0], argv);
			signal (SIGCLD, old_sigvec);
			return (EXIT_FAILURE);
		}
	}
	else 
	{
		switch (fork())
		{
		case -1:
			signal (SIGCLD, old_sigvec);
			return 0;
	
		case 0:
			/*
		 	*	Child process
		 	*/
			status	=	execvp (argv [0], argv);
			signal (SIGCLD, old_sigvec);
			return (EXIT_FAILURE);
	
		default:
			/*
		 	*	Parent process
		 	*/
			wait ((int *)0);
		}
	}
	signal (SIGCLD, old_sigvec);

	return (status);
}

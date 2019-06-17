#ident	"$Id: CoProcess.C,v 5.0 2001/06/19 08:19:05 cha Exp $"
/*
 *	Interface for Co-filters
 *
 *******************************************************************************
 *	$Log: CoProcess.C,v $
 *	Revision 5.0  2001/06/19 08:19:05  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1  1999/02/26 00:35:11  jonc
 *	Interface for cooperative processes.
 *	
 *	Revision 1.1  1998/05/15 03:12:14  jonc
 *	Added External Sorter interface
 *
 */
#include	<ctype.h>
#include	<errno.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>

#include	<osdeps.h>

#include	<liberr.h>

#include	<String.h>
#include	<CoProcess.h>

/*
 *
 */

/*
 *
 */
CoProcess::CoProcess (
 const char * rawprogram) :
	pid (0),
	pWrite (NULL),
	pRead (NULL)
{
	if (!rawprogram || !rawprogram [0])
		return;

	/*
	 *	Break up the program string into nice chunks
	 */
	int		i, v = 1;

	for (i = 0; rawprogram [i]; i++)	//	Determine min requirements for argv
	{
		if (isspace (rawprogram [i]))
			v++;
	}

	char *	program = strdup (rawprogram);
	char **	programv = new (char *) [v + 1];

	for (v = i = 0; program [i]; i++)
	{
		programv [v++] = program + i;

		while (program [i] && !isspace (program [i]))
			i++;

		if (!program [i])
			break;
		program [i] = '\0';
	}
	programv [v] = NULL;

	/*
	 *	Determine whether the program exists
	 *
	 *	Must be a file and executable
	 */
	bool	active = false;
	String	fullpath,
			basename;
	char *	base = strrchr (program, '/');

	if (base)
		basename = programv [0] = base + 1;
	else
		basename = program;

	if (program [0] == '/')
	{
		fullpath = program;
		active = access (program, X_OK) == 0;
	} else
	{
		/*
		 *	Look thru' the path to determine if it's there
		 */
		char *	tdir = NULL;
		char *	e_path = strdup (getenv ("PATH"));

		tdir = strtok (e_path, ":");
		do
		{
			cat (tdir, '/', basename, fullpath);
			active = access (fullpath, X_OK) == 0;
		}	while (!active && (tdir = strtok (NULL, ":")));

		free (e_path);
	}

	if (!active)
		return;

	/*
	 *	Spawn a copy and set up the communication with the co-process
	 */
	int	fp1 [2], fp2 [2];	// pipes for ipc

	if (pipe (fp1) < 0 ||
		pipe (fp2) < 0)
	{
		(*lib_error_handler) ("CoProcess::CoProcess",
			"pipe() failed : %s",
			strerror (errno));
	}

	switch (pid = fork ())
	{
	case -1:				// problems with resources
		pid = 0;
		break;

	case 0:					// child
		close (fp1 [1]);	// close unused ends of half/duplex pipes
		close (fp2 [0]);

		/*
		 *	Tie stdin/stdout of the child to the parent's
		 */
		if (fp1 [0] != STDIN_FILENO)
		{
			if (dup2 (fp1 [0], STDIN_FILENO) < 0)
			{
				(*lib_error_handler) ("CoProcess::CoProcess",
					"Child dup2(stdin) %s",
					strerror (errno));
			}
			close (fp1 [0]);
		}

		if (fp2 [1] != STDOUT_FILENO)
		{
			if (dup2 (fp2 [1], STDOUT_FILENO) < 0)
			{
				(*lib_error_handler) ("CoProcess::CoProcess",
					"Child dup2(stdout) %s",
					strerror (errno));
			}
			close (fp2 [1]);
		}

		/*
		 *	If there's a problem, we can't communicate it to the
		 *	parent, since most of the signal masks (in particular
		 *	SIGPIPE) are already in use by most applications...
		 */
		if (execvp (fullpath, programv) < 0)
		{
			(*lib_error_handler) ("CoProcess::CoProcess",
				"execl %s", fullpath.chars ());
		}
		exit (EXIT_FAILURE);

	default:				// parent
		close (fp1 [0]);	// close unused ends of half/duplex pipes
		close (fp2 [1]);

		/*
		 *	Parent writes to	fp1 [1]
		 *	Parent reads from	fp2 [0]
		 */
		pWrite = fdopen (fp1 [1], "w");
		pRead = fdopen (fp2 [0], "r");

		if (!pWrite || !pRead)
			(*lib_error_handler) ("CoProcess::CoProcess",
				"fdopen reassigns failed");

		/*
		 *	Set buffering mode from our side
		 */
		if (setvbuf (pWrite, NULL, _IOLBF, 0) != 0 ||
			setvbuf (pRead, NULL, _IOLBF, 0) != 0)
		{
			(*lib_error_handler) ("CoProcess::CoProcess",
				"Fatal setvbuf failure");
		}
	}

	/*
	 *	Clean up
	 */
	free (program);
	delete [] programv;
}

CoProcess::~CoProcess ()
{
	if (pid)
	{
		int	status;

		fclose (pWrite);
		fclose (pRead);

		waitpid (pid, &status, 0);
	}
}

bool
CoProcess::IsActive ()
{
	if (!pid)
		return false;

	/*
	 *	Check to see whether process is still around
	 */
	if (kill (pid, 0) != 0)
	{
		pid = 0;
		return false;
	}
	return true;
}

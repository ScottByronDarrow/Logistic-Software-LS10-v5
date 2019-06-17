/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( sort_utils.c   )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (09.08.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (09.08.94) : Reordered "sort" options. Some systems are really     |
|               picky about the order                                 |
|  (  .  .  ) :                                                       |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

#define	DATA_SIZE	400

/*
 *	External sort program used
 */
static char	*SortProgram	= "sort";

/*
 *	Default PROG_PATH value
 */
static char	*DefProgPath	= "/usr/LS10.5";

/*
 *	Mask format for sort file
 */
static char	*SortMask		= "%s/WORK/%.7s%05d";

/*=======================================================
| Opens file for sorting. File name is generated using	|
| first 7 characters of string 'filename'.		|
| Opens the file for appending so that several files	|
| can be written to concurrently.			|
=======================================================*/
FILE	*
sort_open (
	char *filename)
{
	char	fname[101];
	char	*sptr = getenv("PROG_PATH");
	FILE	*sort;

	sprintf (fname, SortMask, sptr ? sptr : DefProgPath, filename, getpid ());

	if ((sort = fopen(fname,"w")) == NULL)
	{
		sprintf(fname,"Error in opening %.7s (SORT)",filename);
		sys_err(fname,errno,PNAME);
	}

	return(sort);
}

/*======================================================
| Reopens file for reading.  This is useful for repeat |
| processing of a sort file.                           |
| Note that a sort_delete () must NOT have been        |
| performed on the file you wish to reopen.            |
======================================================*/
FILE *
sort_reopen (
 char *	filename)
{
	char	fname[101];
	char	*sptr = getenv("PROG_PATH");
	FILE	*sort;

	sprintf (fname, SortMask,
		sptr ? sptr : DefProgPath,
		filename,
		getpid ());

	if ((sort = fopen(fname,"r")) == NULL)
	{
		sprintf(fname,"Error in opening %.7s (SORT)",filename);
		sys_err(fname,errno,PNAME);
	}

	return(sort);
}

/*======================================================
| Rewinds file for re-reading. This is useful for      |
| repeat processing of a sort file.                    |
| Note that a sort_delete () must NOT have been        |
| performed on the file you wish to rewind.            |
======================================================*/
int
sort_rewind (
 FILE *fsort)
{
	fseek (fsort, 0L, 0);

	return (EXIT_SUCCESS);
}
/*=======================================================
| Writes 'data' to file associated with 'stream'.	|
=======================================================*/
void
sort_save(
	FILE *stream, 
	char *data)
{
	fprintf(stream,"%s",data);
}

/*=======================================================
| Sorts the lines in the file specified by the first 7	|
| characters in 'filename' using the Unix / Xenix sort	|
| utility in des order.                                 |
| the original file is overwitten by the sorted file.	|
=======================================================*/
FILE	*
dsort_sort (
	FILE *stream, 
	char *filename)
{
	void	(*old_sigvec)();
	char	fname[101],
			*sptr = getenv ("PROG_PATH");

	sprintf (fname, SortMask,
		sptr ? sptr : DefProgPath,
		filename,
		getpid ());

	/*-------------------------------
	| Close Stream before sorting	|
	-------------------------------*/
	fflush(stream);
	fclose(stream);

	old_sigvec = signal (SIGCLD, SIG_DFL);
	if (fork() == 0)
	{
		execlp 
		(
			SortProgram,
			SortProgram,
			"-r",
			"-o",
			fname,
			fname,
			NULL
		);
		exit(0);
	}
	else
		wait((int *) 0);
	signal (SIGCLD, old_sigvec);

	/*-------------------------------
	| Reopen stream for reading	|
	-------------------------------*/
	if ((stream = fopen(fname,"r")) == NULL)
	{
		sprintf(fname,"Error in opening %.7s (SORT)",filename);
		sys_err(fname,errno,PNAME);
	}
	return(stream);
}
/*=======================================================
| Sorts the lines in the file specified by the first 7	|
| characters in 'filename' using the Unix / Xenix sort	|
| utility.						|
| the original file is overwitten by the sorted file.	|
=======================================================*/
FILE	*
sort_sort (
	FILE *stream, 
	char *filename)
{
	void	(*old_sigvec)();
	char	fname[101],
			*sptr = getenv ("PROG_PATH");

	sprintf (fname, SortMask, sptr ? sptr : DefProgPath, filename, getpid ());

	/*-------------------------------
	| Close Stream before sorting	|
	-------------------------------*/
	fflush(stream);
	fclose(stream);

	old_sigvec = signal (SIGCLD, SIG_DFL);
	if (fork() == 0)
	{
		execlp 
		(
			SortProgram,
			SortProgram,
			"-o",
			fname,
			fname,
			NULL
		);
		exit(0);
	}
	else
		wait((int *) 0);
	signal (SIGCLD, old_sigvec);

	/*-------------------------------
	| Reopen stream for reading	|
	-------------------------------*/
	if ((stream = fopen(fname,"r")) == NULL)
	{
		sprintf(fname,"Error in opening %.7s (SORT)",filename);
		sys_err(fname,errno,PNAME);
	}
	return(stream);
}

/*===================================================
| Reads the contents of the sorted file.			|
| Upon error returns (char *)0.						|
| Otherwise returns a pointer to the data line read.|
| The '\n' is removed from the data.				|
===================================================*/
char	*
sort_read (
	FILE *stream)
{
	char	*sptr;
	static	char	data[DATA_SIZE + 2];

	/*-----------------------
	| Read Line from file	|
	-----------------------*/
	sptr = fgets(data,DATA_SIZE + 1,stream);
	
	/*-------------------
	| At end of file	|
	-------------------*/
	if (!sptr)
		return (NULL);
	
	/*---------------------------
	| Remove carriage return	|
	---------------------------*/
	*(sptr + strlen(sptr) - 1) = '\0';
	return(sptr);
}

/*===========================================
| Delete the sort file 'filename'.			|
| On error return errno, else return 0.		|
===========================================*/
int
sort_delete (
	FILE *stream, 
	char *filename)
{
	char	fname[101];
	char	*sptr = getenv("PROG_PATH");

	sprintf (fname, SortMask, sptr ? sptr : DefProgPath, filename, getpid ());

	fclose(stream);

	if (unlink(fname) == -1)
		return(errno);
	return (EXIT_SUCCESS);
}

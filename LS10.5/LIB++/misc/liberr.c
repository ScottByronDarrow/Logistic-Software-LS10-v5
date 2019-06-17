#ident	"$Id: liberr.c,v 5.0 2001/06/19 08:19:07 cha Exp $"
/*
 *	Library error handler
 *
 *******************************************************************************
 *	$Log: liberr.c,v $
 *	Revision 5.0  2001/06/19 08:19:07  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:12  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:45  jonc
 *	Version 10 start
 *
 *	Revision 2.2  1996/12/04 21:58:30  jonc
 *	Added abort() for easier debugs
 *
 *	Revision 2.1  1996/07/30 00:53:03  jonc
 *	Added #ident directive
 *
 */
#include	<stdarg.h>
#include	<stdio.h>
#include	<stdlib.h>

#include	<liberr.h>

void	default_error_handler (const char *, const char *, ...);
void	(*lib_error_handler)(const char *, const char *, ...) =
			default_error_handler,
		(*app_error_handler)(const char *, const char *, ...) =
			default_error_handler;

void
default_error_handler (
 const char *	section,
 const char *	mask,
 ...)
{
	va_list	args;

	fprintf (stderr, "FATAL: (%s) ", section ? section : "?");

	va_start (args, mask);
	vfprintf (stderr, mask, args);
	va_end (args);

	fputc ('\n', stderr);

#ifndef	NDEBUG
	abort ();				/* generate a core file for debug purposes */
#endif

	exit (EXIT_FAILURE);
}

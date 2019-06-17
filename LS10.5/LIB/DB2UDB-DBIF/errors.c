#ident	"$Id: errors.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Error dumps for ORACLE port
 *
 *******************************************************************************
 *	$Log: errors.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *
 *
 *	
 */
#include	<stdlib.h>
#include	<stdio.h>
#include	<stdarg.h>
/*#include	"db2dbif.h"*/

void
db2bif_error (
 const char * fmt,
 ...)
{
	va_list	ap;

	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);

	abort ();
}

void
db2bif_warn (
 const char * fmt,
 ...)
{
	va_list	ap;

	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
}
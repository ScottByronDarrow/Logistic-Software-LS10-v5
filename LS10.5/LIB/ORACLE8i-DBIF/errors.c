/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: errors.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (errors.c)
|  Program Desc  : (Error dumps for ORACLE port)
|---------------------------------------------------------------------|
| $Log: errors.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:51  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/04/06 02:09:52  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.0  2000/07/15 07:33:50  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.2  2000/07/13 06:43:51  gerry
| Linked error handling to standard LS/10 error handling, fixed alias bug
|
| Revision 1.1  1999/10/21 21:47:04  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdlib.h>
#include	<stdio.h>

#include	"oracledbif.h"

/*
 * Constants 
 */
#define		ORACLE_DBIF_ERROR	-9
#define		ORACLE_CDA_ERROR	-8

/*
 * External variables 
 */
extern char *PNAME;

/*
 * oracledbase_err 
 */
void
oracledbase_err (
 	const char *e_rec,
 	int e_err)
{
	char    msg [1024];

	sprintf (msg, "Oracle DBIF Error (%s)", e_rec);               
	sys_err (msg, e_err, PNAME);
}             

/*
 * oracledbif_error 
 */

void
oracledbif_error (
	const char * fmt,
	...)
{
	char	msg [132];
	va_list	ap;

	va_start (ap, fmt);
	vsprintf (msg, fmt, ap);
	va_end (ap);

	/*
	 * Replace abort with LS/10 sys_err abort ();	
	 */
	oracledbase_err (msg, ORACLE_DBIF_ERROR);
}

/*
 * oracledbif_warn 
 */
void
oracledbif_warn (
	const char * fmt,
	...)
{
	va_list	ap;

	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
}

/*
 * oraclecda_error 
 */
void
oraclecda_error (
	OCIError * errhp,
	const char * fmt,
	...)
{
	va_list	ap;
	char errbuf [1024];
	ub4 errcode;

	/*
	 * Display OCI error code. 
	 */
	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);

	OCIErrorGet ((dvoid *) errhp,
				 (ub4) 1,
				 (text *) NULL,
				 &errcode,
				 errbuf,
				 (ub4) sizeof (errbuf),
				 (ub4) OCI_HTYPE_ERROR);
	fprintf (stderr, "CDA: %s", errbuf);

	/*
	 * abort (); 
	 */
	oracledbase_err (errbuf, ORACLE_CDA_ERROR);
}

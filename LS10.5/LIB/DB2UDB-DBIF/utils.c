#ident	"$Id: utils.c,v 5.0 2001/06/19 07:08:21 cha Exp $"
/*
 *	Misc functions
 *
 *******************************************************************************
 *	$Log: utils.c,v $
 *	Revision 5.0  2001/06/19 07:08:21  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:47  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	Revision 1.1  1999/10/21 21:47:05  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<ctype.h>

char *
_strupper (
 char * buf)
{
	char * c;

	for (c = buf; *c; c++)
		*c = toupper (*c);
	return buf;
}

char *
_strlower (
 char * buf)
{
	char * c;

	for (c = buf; *c; c++)
		*c = tolower (*c);
	return buf;
}

#ident	"$Id: utils.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Misc functions
 *
 *******************************************************************************
 *	$Log: utils.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:53  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
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

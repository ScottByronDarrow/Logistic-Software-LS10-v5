/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: utils.c,v 5.0 2002/05/08 01:30:09 scott Exp $
|  Program Name  : (utils.c)
|  Program Desc  : (Misc functions)
|---------------------------------------------------------------------|
| $Log: utils.c,v $
| Revision 5.0  2002/05/08 01:30:09  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:52  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 02:09:53  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.0  2000/07/15 07:33:51  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.1  1999/10/21 21:47:05  jonc
| Alpha level checkin:
| Done: database queries, updates.
 Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<ctype.h>

/*
 * _strupper 
 */
char *
_strupper (
 	char * buf)
{
	char * c;

	for (c = buf; *c; c++)
		*c = toupper (*c);
	
	return buf;
}

/*
 * _strlower 
 */
char *
_strlower (
 	char * buf)
{
	char * c;

	for (c = buf; *c; c++)
		*c = tolower (*c);
	
	return buf;
}

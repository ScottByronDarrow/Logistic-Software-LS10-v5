#ifndef	_XML_Error_h
#define	_XML_Error_h
/*	$Id: XML_Error.h,v 5.0 2001/06/19 06:51:26 cha Exp $
 *******************************************************************************
 *	$Log: XML_Error.h,v $
 *	Revision 5.0  2001/06/19 06:51:26  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/05/14 05:36:44  scott
 *	New Function XML_Error to log XML errors.
 *	
 *	Usage : XML_Error (programName, Message1, Message2, Message3, Message3);
 */

extern	void	XML_Error (char *,char *,char *,char *,char *);

#endif	/* _XML_Error */

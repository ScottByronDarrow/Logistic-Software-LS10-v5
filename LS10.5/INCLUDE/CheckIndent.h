#ifndef	_CheckIndent_h
#define	_CheckIndent_h
/*	$Id: CheckIndent.h,v 5.0 2001/06/19 06:51:15 cha Exp $
 *******************************************************************************
 *	$Log: CheckIndent.h,v $
 *	Revision 5.0  2001/06/19 06:51:15  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:19  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:50  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:33  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1  2000/02/17 06:50:01  scott
 *	S/C LSANZ-16005 / LSDI-2531
 *	Updated for new libary based routines.
 *	
 */
enum
{
	IndentErr_Ok,
	IndentErr_BadName,
	IndentErr_NoSuchIndent,
	IndentErr_Overflow
};

extern int	check_indent (
				const char * co,
				const char * br,
				long hhcc_hash,
				char *item_no);		/* will be overwritten with result */

#endif	/* _CheckIndent_h */

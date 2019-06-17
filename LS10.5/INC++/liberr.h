#ifndef	_liberr_h
/*	$Id: liberr.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *	
 *	$Log: liberr.h,v $
 *	Revision 5.0  2002/05/08 01:50:44  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:27  jonc
 *	Version 10 start
 *
 *	Revision 2.0  1996/02/13 03:45:05  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:44:30  jonc
 *	Initial cut
 *
 *	Revision 1.1  1996/01/07 21:08:48  jonc
 *	Initial revision
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

extern void	(*lib_error_handler) (const char *, const char *, ...),
			(*app_error_handler) (const char *, const char *, ...);

#ifdef __cplusplus
}
#endif

#endif

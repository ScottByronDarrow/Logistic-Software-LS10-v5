#ifndef _UnixEnv_h
#define _UnixEnv_h
/*	$Id: UnixEnv.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	{libapp}
 *
 *	UNIX Environment wrapper
 *
 *************************************************************
 *	$Log: UnixEnv.h,v $
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
 */

#include	<Environ.h>

class UnixEnv :
	public Environment
{
	public:
		UnixEnv (const char * name, const char * value = 0);

		/*
		 *	Overidden virtuals
		 */
		virtual void		Put (const char *),
							Put (int);
};

#endif	// _UnixEnv_h

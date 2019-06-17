#ifndef _LogisticEnv_h
#define _LogisticEnv_h
/*	$Id: LogisticEnv.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	{libapp}
 *
 *	Logisticacle Environment
 *
 *************************************************************
 *	$Log: LogisticEnv.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:09:01  scott
 *	New File
 *	
 *	Updated to clean code while working in format-p
 *	
 *
 */

#include	<Environ.h>

class LogisticEnv :
	public Environment
{
	public:
		LogisticEnv (const char * name);
		virtual ~LogisticEnv ();

		bool	Write (void);			// save to file

};

#endif	// _LogisticEnv_h

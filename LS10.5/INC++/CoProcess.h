#ifndef	_CoProcess_h
#define	_CoProcess_h
/*	$Id: CoProcess.h,v 5.0 2002/05/08 01:50:42 scott Exp $
 *
 *	Cooperative processes/filters
 *
 *	Abstract wrapper
 *
 *	Reads and writes to the co-process are line-buffered.
 *
 *******************************************************************************
 *	$Log: CoProcess.h,v $
 *	Revision 5.0  2002/05/08 01:50:42  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:42  scott
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
#include	<stdio.h>
#include	<sys/types.h>

class CoProcess
{
	private:
		pid_t	pid;

	protected:
		FILE *	pWrite,						// write to co-process
			 *	pRead;						// read from co-process

	public:
		CoProcess (const char * program);
		virtual ~CoProcess ();

		bool	IsActive ();
};

#endif	//_CoProcess_h

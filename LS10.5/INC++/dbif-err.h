#ifndef	_dbif_err_h
#define	_dbif_err_h
/*	$Id: dbif-err.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	List of Database interface errors
 *
 *		- We define a set so that it is independant of each vendor's
 *		  definitions.
 *
 *	$Log: dbif-err.h,v $
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
 *	Revision 2.1  1996/03/04 03:04:01  jonc
 *	Database Interface Errors: Initial list of possible errors
 *
 */
enum DBIFError
{
	NoError,
	DupIdx,						// duplicate index
	NoCurr,						// no current record
	EndOfFile,					// last record
	RecLocked					// requested record is locked
};

#endif	/*_dbif_err_h*/

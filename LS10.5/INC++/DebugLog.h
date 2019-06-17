#ifndef	_DebugLog_h
#define	_DebugLog_h
/*	$Id: DebugLog.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	Application DebugLogging.
 *
 *	Logs are placed in:			$TMPDIR/name.log
 *	Log level determined by:	DEBUG
 *
 *******************************************************************************
 *	$Log: DebugLog.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
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
#include	<stdio.h>			// for FILE *
#include	<stdarg.h>			// for va_list

class DebugLog
{
	public:
		enum LogLevel
		{
			Life = 1,			// start/stop of program
			Key,				// key events: pipe opens, forks
			Verbose,			// informational: bad records, decisions
			Debug				// function entry/exit
		};

	private:
		FILE *	logFile;
		int		envLvl;

	public:
		DebugLog (const char * name);
		~DebugLog ();

		void	Log (enum LogLevel, const char * mask, ...),
				Log (enum LogLevel, const char * mask, va_list args);

		//
		//
		void	UseForAppError (void),
				UseForLibError (void);
};

#endif	//_DebugLog_h

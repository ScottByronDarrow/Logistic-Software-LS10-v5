#ifndef	_sys_log_h
#define	_sys_log_h

/*
 *	List of possible log files to use
 */
enum SysLogFile
{
	LogBackground,			/* Background processes */
	LogApplication			/* non-fatal application errors */
};

/*
 *	Prototypes
 */
extern void	sys_log (enum SysLogFile, char *, ...);

#endif	/*_sys_log_h*/

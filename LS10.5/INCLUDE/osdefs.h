#ifndef	OSDEFS_H
#define	OSDEFS_H
/*******************************************************************************
 *
 *	O/S Specific declarations
 *
 *
 *	Defines used :
 *
 *	MAJOR_IN_SYSMACROS			major () found in sys/sysmacros.h
 *	MAJOR_IN_SYSMKDEV			major () found in sys/mkdev.h
 *
 *	HAS_STDDEF_H				system has <stddef.h>
 *	HAS_STDLIB_H				system has <stdlib.h>
 *	HAS_UNISTD_H				system has <unistd.h>
 *
 *	HAS_MAJOR_T					system defines major_t and minor_t types
 *	HAS_UID_T					system defines uid_t and gid_t types
 *
 *	HAS_CRYPT					system has crypt(3)
 *	HAS_SID_FNS					system has setsid(2) AND getsid(2)
 *
 *	HAS_TERMIOS					Use POSIX routines for terminal control
 *
 *	HAS_PROCFS					system supports /proc file-systems
 *
 *	HAS_CTRLMACRO				location of CTRL(x) in system.
 *
 *	HAS_BSDSYSCALLS				system uses BSD system calls
 *
 ******************************************************************************
 $Log: osdefs.h,v $
 Revision 5.0  2001/06/19 06:51:47  cha
 LS10-5.0 New Release as of 19 JUNE 2001

 Revision 4.0  2001/03/09 00:59:28  scott
 LS10-4.0 New Release as at 10th March 2001

 Revision 3.0  2000/10/12 13:28:57  gerry
 Revision No. 3 Start
 <after Rel-10102000>

 Revision 2.1  2000/08/16 03:18:49  gerry
 Added SunOS deps

 Revision 2.0  2000/07/15 07:15:43  gerry
 Force revision no. to 2.0 - Rel-15072000

 Revision 1.8  2000/02/18 01:02:17  scott
 Updated from Trevor. Compile warnings found with Linux.

 Revision 1.7  1999/12/06 03:30:22  jonc
 Updated for IRIX 5.3

 Revision 1.6  1999/11/12 03:20:34  jonc
 Added include's to minimise compile warnings.

 Revision 1.5  1999/10/04 04:22:08  jonc
 Added HAS_BSDSYSCALLS to allow for ports for BSD based systems.

 Revision 1.4  1999/09/29 00:06:33  jonc
 Added HAS_CTRLMACRO definition and usage in hotkeys.h for clean compile
 on both AIX and Alpha OSF1.

 */

/*
 *	RISC/os (SVR4 mode)
 */
#if		defined (MIPS)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

/*
 *	SNI/os 
 */
#elif	defined (SNI)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

/*
 *	Entries for SCO boxes
 */
#elif	defined (SCO)

#define	MAJOR_IN_SYSMACROS
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T

#define	HAS_TERMIOS					/* Use POSIX tty routines */

/*
 *	Entries for IBM's AIX series
 */
#elif	defined (_AIX)

#define	MAJOR_IN_SYSMACROS

#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T

#define	HAS_CTRLMACRO	<sys/ttychars.h>

#include	<sys/resource.h>	/* minimise verbose warnings */

/*
 *	Entries for the Altos series
 */
#elif	defined (ALT)			/* Entries for these are pure guesses */

#define	MAJOR_IN_SYSMACROS

/*
 *	Entries for the NCR Towers
 */
#elif	defined (NCR)

#define	MAJOR_IN_SYSMACROS

/*
 *	Entries for ICL (SVR4 boxes)
 */
#elif	defined	(ICL)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

/*
 *	Entries for SGI boxes
 */
#elif	defined (IRIX)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

#define	HAS_CTRLMACRO	<termios.h>

/*
 *	Entries for HP Series 700 boxes
 */
#elif	defined (HP)

#define	MAJOR_IN_SYSMACROS
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T
#define	HAS_CRYPT

/*
 *	Entries for Sun Sparc boxes
 */
#elif	defined (SunOS)

#define	MAJOR_IN_SYSMACROS
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_MAJOR_T
#define	HAS_CTRLMACRO	<sys/ttychars.h>


/*
 *	Entries for AT&T 3000 series (SVR4)
 */
#elif	defined	(ATT)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

/*
 *	Entries for DEC's OSF/1 (Close to SVR4)
 */
#elif	defined	(OSF1)

#define	MAJOR_IN_SYSMACROS
#define	STATFS_IN_MOUNT
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

#define	HAS_TERMIOS					/* Use POSIX tty routines */
#define	HAS_CTRLMACRO	<sys/ttydefaults.h>

/*
 *	Entries for UNISYS 6000 series (SVR4)
 */
#elif	defined	(U6000)

#define	MAJOR_IN_SYSMKDEV
#define	HAS_STDDEF_H
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_MAJOR_T
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

/*
 *	Entries for LINUX boxes
 */
#elif	defined (LINUX)

#define	MAJOR_IN_SYSMACROS
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS					/* system supports /proc file-systems */
#define	HAS_CTRLMACRO	<sys/ttydefaults.h>

#elif	defined (FREEBSD)
/*
 *	Entries for FREEBSD boxes
 */

/* major in sys/types! */
#define	HAS_STDLIB_H
#define	HAS_UNISTD_H
#define	HAS_UID_T
#define	HAS_CRYPT
#define	HAS_SID_FNS
#define	HAS_PROCFS

#define	HAS_TERMIOS					/* Use POSIX tty routines */
#define	HAS_CTRLMACRO	<sys/ttydefaults.h>
#define	HAS_BSDSYSCALLS				/* Use BSD-system calls instead of SysV */

#define	SIGCLD	SIGCHLD				/* SysV vs BSD */

#else
#error	No definition for current machine type in <osdefs.h>
#endif

#endif	/*OSDEFS_H*/

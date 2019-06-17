/*	$Id: LibraryVersion.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	Constructs a Version string for libraries.
 *	This file:
 *
 *		1. should only be included *ONCE* per library.
 *		2. should not be used for application code.
 *		3. assumes that the following #defines have been made
 *				LIBRARYNAME
 *				STATE
 *				VERSION
 *
 *******************************************************************************
 *	$Log: LibraryVersion.h,v $
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
 *	Revision 1.4  2000/08/16 03:22:17  gerry
 *	Added SunOS support
 *	
 *	Revision 1.3  1999/11/23 02:42:31  nz
 *	DPL15154	Add platformes for _AIX
 *	
 *	Revision 1.2  1999/10/17 22:48:37  jonc
 *	Added FreeBSD support.
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 */
#ifdef	_LibraryVersion_h
#error	"Multiple inclusion of LibraryVersion.h"
#else
#define	_LibraryVersion_h

/*
 *	Define supported platforms
 */
#if		defined (SCO)
#define	PLATFORM	"SCO"
#elif	defined (OSF1)
#define	PLATFORM	"OSF1"
#elif	defined (IRIX)
#define	PLATFORM	"IRIX"
#elif	defined (_AIX)
#define	PLATFORM	"_AIX"
#elif	defined (FREEBSD)
#define	PLATFORM	"FREEBSD"
#elif	defined (LINUX)
#define	PLATFORM	"LINUX"
#elif	defined (SunOS)
#define	PLATFORM	"SunOS"
#endif	//PLATFORM

/*
 *	Construct Version string with leading marker
 */
static const char
	*LibraryVersion = "LIBRARY:" LIBRARYNAME "(" PLATFORM ")" STATE REVISION;

#endif	//	_LibraryVersion_h

#ifndef	VERNO_H
#define	VERNO_H

/*
 * Include file for library version identification. This file should only
 * be included by one file in a library (one that is linked in by any
 * application that uses the library)
 */

/* Define platform name
*/
#if	defined (_AIX)
#define	PLATFORM	"AIX."

#elif	defined (ATT)
#define	PLATFORM	"ATT."

#elif	defined (HP)
#define	PLATFORM	"HP."

#elif	defined (ICL)
#define	PLATFORM	"ICL."

#elif	defined (IRIX)
#define	PLATFORM	"IRIX."

#elif	defined (MIPS)
#define	PLATFORM	"MIPS."

#elif	defined (NCR)
#define	PLATFORM	"NCR."

#elif	defined (SCO)
#define	PLATFORM	"SCO."

#elif	defined (U6000)
#define	PLATFORM	"U6000."

#elif	defined (LINUX)
#define	PLATFORM	"LINUX."

#endif

/* Defaults
*/
#ifndef	PLATFORM
#define	PLATFORM	"Unknown."
#endif	/*PLATFORM*/

#ifndef	LIBNAME
#define	LIBNAME	"unknown"
#endif	/*LIBNAME*/

#ifndef	OS_VER
#define	OS_VER	"Unknown"
#endif	/*OS*/

/* Library id string
*/
#ifndef	lint
static char	libver [] = "@(#)" PLATFORM "[" OS_VER "]" VERSION " " LIBNAME;
#endif	/*lint*/

/* Clear up namespace
*/
#undef	PLATFORM
#undef	OS_VER

#endif	/*VERNO_H*/

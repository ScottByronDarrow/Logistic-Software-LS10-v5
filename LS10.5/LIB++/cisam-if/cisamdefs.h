#ifndef	_cisamdefs_h
#define	_cisamdefs_h
/*	$Id: cisamdefs.h,v 5.0 2001/06/19 08:17:32 cha Exp $
 *
 *	CISAM definitions
 *
 *************************************************************
 *	$Log: cisamdefs.h,v $
 *	Revision 5.0  2001/06/19 08:17:32  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1999/02/21 22:55:32  jonc
 *	Fixed: crashes on IRIX for additions/modifications.
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 */

/*
 *	CISAM limits
 */
#define	MAX_IDX_PARTS	NPARTS		// Maximum number of items in an index
#define	MAX_COLNAME_LEN		18		// Maximum length for a column-name
#define	MAX_IDXNAME_LEN		18		// Maximum length for an index-name
#define	MAX_PATHNAME_LEN	64		// Maximum length for systable.dirpath

/*
 *	CISAM-SQL magic numbers
 *
 *	These numbers have been plucked from thin-air by Informix to
 *	indicate NULL values for Integer and Long columns in their database.
 *
 *	The stupid thing about using them is that Informix has *NOT* documented
 *	this in any of their header files!
 *
 */
#define	INTNULL		-32768			// SHRT_MIN from <limits.h>
#define	LONGNULL	(-2147483647-1)	// LONG_MIN from <limits.h>

/*
 *	Informix CISAM function prototypes
 */
#if	INFORMIXVERSION < 7

extern "C"
{

extern int	isopen (const char *, int),
			isclose (int),
			isindexinfo (int, void *, int),

			isstart (int, const struct keydesc *, int, char *, int),
			isread (int, char *, int),
			isrewcurr (int, char *),
			iswrite (int, void *),
			isdelcurr (int),

			isrelease (int),
			islogopen (const char *),
			isbegin (void),
			iscommit (void),
			isrollback (void);

extern void	stchar (const char *, void *, int),
			stlong (long, void *),
			stfloat (double, void *),
			stdbl (double, void *),
			stdecimal (dec_t *, void *, int);

extern void		ldchar (char *, int, char *);
extern int		lddecimal (char *, int, dec_t *);
extern long		ldlong (char *);
extern double	ldfltnull (char *, short *),
				lddblnull (char *, short *);

extern int	deccvasc (const char *, int, dec_t *),
			dectoasc (const dec_t *, void *, int, int),
			deccvint (int, dec_t *);

}

#endif	//	INFORMIXVERSION < 7

#endif	//_cisamdefs_h

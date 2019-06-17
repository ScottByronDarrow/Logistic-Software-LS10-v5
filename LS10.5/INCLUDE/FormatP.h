#ifndef	_FormatP_h
#define	_FormatP_h
/*	$Id: FormatP.h,v 5.0 2001/06/19 06:51:17 cha Exp $
 *
 *	Simple front end to the format-p filter
 *
 *******************************************************************************
 *	$Log: FormatP.h,v $
 *	Revision 5.0  2001/06/19 06:51:17  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:20  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:51  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:34  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  2000/01/05 02:16:01  jonc
 *	Tightened FormatSubmitTable args
 *	
 *	Revision 1.2  1999/09/13 06:13:06  alvin
 *	Check-in all ANSI-converted include files.
 *	
 *	Revision 1.1  1999/07/14 23:50:44  jonc
 *	Initial interface to format-p (adopted from Pinnacle version 10)
 *	
 */
extern FILE *	FormatPOpen (const char * layout,
					const char * out,
					const char * optionblock),		/* NULL = use default */
			*	FormatPOpenLpNo (const char * layout,
					int lpno,
					const char * optionblock);		/* NULL = use default */
extern void		FormatPClose (FILE *);

extern void		FormatPReset (FILE *),
				FormatPBatchEnd (FILE *);

extern void		FormatPPageHeader (FILE *, const char *),
				FormatPPageTrailer (FILE *, const char *),
				FormatPBody (FILE *, const char *);

extern void		FormatPSubmitTable (FILE *, const char * table),
				FormatPSubmitInt (FILE *, const char * key, int),
				FormatPSubmitLong (FILE *, const char * key, long),
				FormatPSubmitChars (FILE *, const char * key, const char * val),
				FormatPSubmitDate (FILE *, const char * key, Date),
				FormatPSubmitMoney (FILE *, const char * key, Money);


#endif	/*	_FormatP_h */

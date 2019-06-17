#ifndef	_FinancialDates_h
#define	_FinancialDates_h
/*	$Id: FinancialDates.h,v 5.0 2001/06/19 06:51:16 cha Exp $
 *
 *
 *******************************************************************************
 *	$Log: FinancialDates.h,v $
 *	Revision 5.0  2001/06/19 06:51:16  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:20  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2001/01/29 03:44:06  scott
 *	Updated to add YearEnd Function
 *	
 *	Revision 3.0  2000/10/12 13:28:50  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:34  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 1.1  1998/05/05 05:08:40  kirk
 *	Set of financial date functions.
 *
 */

extern Date FinYearStart 	(Date date, int fiscal);
extern Date FinYearEnd 		(Date date, int fiscal);
extern void GetFinYear 		(Date date, int fiscal, Date *start, Date *end);
extern void DateToFinDMY 	(Date date, int fiscal, int *, int *, int *);
extern Date FinDMYToDate 	(int fiscal, int, int, int);
extern Date YearEnd 		(void);

#endif	/* _FinancialDates_h */

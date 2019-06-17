/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindCumr.h,v 5.0 2001/06/19 06:51:16 cha Exp $
-----------------------------------------------------------------------
| $Log: FindCumr.h,v $
| Revision 5.0  2001/06/19 06:51:16  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:34  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.2  2000/06/16 01:14:20  scott
| Checked in FindCumr.h
|
| Revision 1.1  2000/06/15 05:17:21  jinno
| New include define for Customer search.
|
*/

#ifndef	_FindCumr_h
#define	_FindCumr_h

/*---------------------------------------------------------
| Extern required for check_search function from LIB/BASE |
---------------------------------------------------------*/
extern int check_search (char *, char *, int *);

/*----------------------------------------------------------------------------
  NOTE : branchNumber is what was normally refered to as estab within programs.

  FindCumr (companyNumber, branchNumber, customerNumber)
  e.g. cc = 	FindCumr
				(
					comm_rec.co_no,
					branchNumber 		-	Changes based on environment DB_CO
					customerNumber
				);
----------------------------------------------------------------------------*/
int			FindCumr			(char *,char *,char *);

/*----------------------------------------------------------------------------
  CumrSearch (companyNumber, branchNumber, customerNumber)
  e.g. CumrSearch
		(
			comm_rec.co_no,
			branchNumber,
			temp_str
		);
----------------------------------------------------------------------------*/
void		CumrSearch			(char *,char *, char *);

#endif	/*	_FindCumr_h	*/


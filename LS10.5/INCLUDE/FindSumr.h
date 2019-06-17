/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindSumr.h,v 5.0 2001/06/19 06:51:17 cha Exp $
-----------------------------------------------------------------------
| $Log: FindSumr.h,v $
| Revision 5.0  2001/06/19 06:51:17  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:34  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.1  2000/06/15 02:26:33  scott
| New include define for Supplier search.
| In line with search that was done for items.
|
|
*/

#ifndef	_FindSumr_h
#define	_FindSumr_h

/*---------------------------------------------------------
| Extern required for check_search function from LIB/BASE |
---------------------------------------------------------*/
extern int check_search (char *, char *, int *);

/*----------------------------------------------------------------------------
  NOTE : branchNumber is what was normally refered to as estab within programs.

  FindSumr (companyNumber, branchNumber, supplierNumber)
  e.g. cc = 	FindSumr
				(
					comm_rec.co_no,
					branchNumber 		-	Changes based on environment CR_CO
					supplierNumber
				);
----------------------------------------------------------------------------*/
int			FindSumr			(char *,char *,char *);

/*----------------------------------------------------------------------------
  SumrSearch (companyNumber, branchNumber, suppierNumber)
  e.g. SumrSearch
		(
			comm_rec.co_no,
			branchNumber,
			temp_str
		);
----------------------------------------------------------------------------*/
void		SumrSearch			(char *,char *, char *);

#endif	/*	_FindSumr_h	*/


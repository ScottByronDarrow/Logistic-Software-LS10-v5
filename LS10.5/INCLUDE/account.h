/*=======================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=======================================================================|
| Program Name : ( account.h )                                          |
| Program Desc : ( Definitions for library module account.c       )     |
|                (                                                )     |
|-----------------------------------------------------------------------|
| Authors      : Unknown                                                |
| Date Written : ??/??/??                                               |
|-----------------------------------------------------------------------|
| Date Modified : (02/07/1999) Modified by : Trevor van Bremen          |
|                                                                       |
| Comments      :                                                       |
| (02/07/99)    : Fully restructured.  Moved fragments to library       |
-------------------------------------------------------------------------
	$Log: account.h,v $
	Revision 5.0  2001/06/19 06:51:26  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:59:22  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:28:52  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:15:35  gerry
	Force revision no. to 2.0 - Rel-15072000
	
	Revision 1.5  1999/11/15 06:47:05  scott
	Updated for compile problems on AIX
	
	Revision 1.4  1999/09/29 00:00:03  jonc
	Corrected "long" vs "time_t" usage; very apparent on AIX and Alpha.
	
=======================================================================*/
#ifndef	ACCOUNT_H
#define	ACCOUNT_H

struct	acc_type
{
	time_t	_time;
	char	_desc [61];
};
extern	struct	acc_type	acc_rec;	/* Defined in library now */

#endif	/* ACCOUNT_H */

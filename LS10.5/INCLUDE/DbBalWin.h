#ifndef	_DbBalWin_h
#define	_DbBalWin_h
/*	$Id: DbBalWin.h,v 5.0 2001/06/19 06:51:15 cha Exp $
 *******************************************************************************
 *	$Log: DbBalWin.h,v $
 *	Revision 5.0  2001/06/19 06:51:15  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.3  2001/03/27 06:31:18  scott
 *	Updated to add usage comments
 *	
 *	Revision 4.2  2001/03/27 06:15:08  scott
 *	Updated to change arguments passed to DbBalWin to remove need for comm
 *	
 *	Revision 4.1  2001/03/26 10:19:21  scott
 *	Added new function DbBalWin.h
 *	
 *	Usage : DbBalWin (hhcuHash, fiscalPeriod, DbtDate)
 */

extern	void	DbBalWin (long, int, long);

#endif	/* _DbBalWin */

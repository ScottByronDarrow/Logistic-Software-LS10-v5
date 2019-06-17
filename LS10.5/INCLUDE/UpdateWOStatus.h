#ifndef	_UpdateWOStatus_h
#define	_UpdateWOStatus_h
							/*------------------------------------------*/
#define	WO_AOK			0	/* Works order was found and status updated	*/
#define	WO_NOTFOUND		1	/* Works order was not found. 				*/
#define	WO_ISTATUS		2	/* Works order status not valid 			*/
							/*------------------------------------------*/

/*	$Id: UpdateWOStatus.h,v 5.0 2001/06/19 06:51:26 cha Exp $
 *******************************************************************************
 *	$Log: UpdateWOStatus.h,v $
 *	Revision 5.0  2001/06/19 06:51:26  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/03/28 07:03:38  scott
 *	New function to update works order status (automatic rather than manual)
 *	
 *	Usage : UpdateWOStatus (companyNo, branchNo, WarehouseNo, WONo, status);
 */

extern	int		UpdateWOStatus (char *,char *,char *,char *,char *);

#endif	/* _UpdateWOStatus */

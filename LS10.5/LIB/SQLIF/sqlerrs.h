#ifndef	SQLERRS_H
#define	SQLERRS_H
/*******************************************************************************
$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/sqlerrs.h,v 5.0 2001/06/19 07:14:41 cha Exp $

	List of possible error numbers that may come back from Informix

$Log: sqlerrs.h,v $
Revision 5.0  2001/06/19 07:14:41  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:07  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:32:29  jonc
 * Initial revision
 * 

*******************************************************************************/

#define	ERR_COLNOTFOUND		-217
#define	ERR_DBNOTFOUND		-329
#define	ERR_DBCANNOTCREATE	-330
#define	ERR_NOACTIVEDB		-349
#define	ERR_NOCONNECTPERM	-387
#define	ERR_NORESOURCEPERM	-388
#define	ERR_NODBAPERM		-389
#define	ERR_BADNAME		-354

#endif	/*SQLERRS_H*/

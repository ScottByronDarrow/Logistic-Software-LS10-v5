#ifndef	UTILS_H
#define	UTILS_H
/*******************************************************************************
$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/utils.h,v 5.0 2001/06/19 07:14:43 cha Exp $

	Prototypes for misc functions

$Log: utils.h,v $
Revision 5.0  2001/06/19 07:14:43  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:08  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:33:20  jonc
 * Initial revision
 * 

*******************************************************************************/
#include	<sqlda.h>
#include	"tblnode.h"

/* Function prototypes */
extern void	BuildParam (struct sqlvar_struct *, struct sqlvar_struct *);
extern int	XlatSQLC (int);

extern void	DataFudgeIn (TableNode *, struct sqlda *),
		DataFudgeOut (TableNode *, struct sqlda *);

#endif	/*UTILS_H*/

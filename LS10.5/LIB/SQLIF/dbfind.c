/*******************************************************************************

	Fudge code for seldom used function

$Log: dbfind.c,v $
Revision 5.1  2001/08/09 09:31:24  scott
Updated to add FinishProgram () function

Revision 5.0  2001/06/19 07:14:31  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:06  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:29:36  jonc
 * Initial revision
 * 

*******************************************************************************/
#ifndef	lint
static char	*rscid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/dbfind.c,v 5.1 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<sqltypes.h>
#include	<string.h>

#include	"isamdbio.h"
#include	"tblnode.h"

/**	Provided for backward compatibility
**/
/**
	dbfind () expects a singular index
	length will remain unaffected
**/
dbfind (filename, ftype, value, length, buf)
char	*filename;
int	ftype;
char	*value;
int	*length;
char	*buf;
{
	int	i;
	TableNode	*node = GetTableNode (filename);

	*length = 0;
	if (!node || !node -> keys || node -> keys -> next)
		return (-1);

	/**	Assume that the 1st key exists and is of type long	*/
	for (i = 0; i < node -> fldCount; i++)
		if (!strcmp (node -> keys -> name, node -> fields [i].vwname))
		{
			/*	Prime the index	*/
			switch (node -> fields [i].vwtype)
			{
			case SQLCHAR	:
				strcpy (buf + node -> fields [i].vwstart, value);
				break;

			case SQLSMINT	:
				*(int *) (buf + node -> fields [i].vwstart) = *(int *) value;
				break;

			case SQLFLOAT	:
				*(double *) (buf + node -> fields [i].vwstart) = *(double *) value;
				break;

			case SQLSMFLOAT	:
				*(float *) (buf + node -> fields [i].vwstart) = *(float *) value;
				break;

			case SQLINT	:
			case SQLSERIAL	:
				*(long *) (buf + node -> fields [i].vwstart) = *(long *) value;
				break;

			case SQLDATE	:
				return (-1);	/* need to redo this	*/

			default	:
				return (-1);
			}
			return (find_rec (filename, buf,
				ftype & ~LOCK,
				ftype & LOCK ? "u" : "r"));
		}
	return (-1);		/*	Internal error!	*/
}

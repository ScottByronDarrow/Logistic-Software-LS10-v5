#ident	"$Id: delete.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Deletes to the database
 *
 *******************************************************************************
 *	$Log: delete.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	
 */

#include	<stdio.h>
#include	<string.h>

#include	<sqlcli1.h>
#include	"samputil.h"
#include	"db2io.h"
#include	"db2dbif.h"

/*
 * Global variable
 */
SQLRETURN rc;


int
 _DeleteRow (
 TableState * table)
{
	
	int ret = SQL_SUCCESS;

	/*
	 * Position the cursor to the row and deletes it.
	 */
	rc = SQLSetPos( table -> q_hstmt, 1, SQL_DELETE , SQL_LOCK_NO_CHANGE);
    CHECK_HANDLE( SQL_HANDLE_STMT,  table -> q_hstmt, rc ) ;
	ret = ret || rc;

	/*
	 * Commit the changes
	 */
	rc = SQLEndTran(SQL_HANDLE_DBC, *table -> hdbc, SQL_COMMIT);
		CHECK_HANDLE( SQL_HANDLE_STMT, *table -> hdbc, rc ) ;
	ret = ret || rc;

	return ret;
}


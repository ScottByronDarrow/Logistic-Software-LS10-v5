#ident	"$Id: delete.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Deletes to the database
 *
 *******************************************************************************
 *	$Log: delete.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/10/21 21:47:04  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

#include	<dbio.h>

#include	"oracledbif.h"

/*
 *	Local functions
 */
static void BuildStatement (Lda_Def *, TableState *),
	    BindInputVariables (Lda_Def *, TableState *);

/*
 *	External interface
 */
int
_DeleteRow (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Delete the current row
	 */

	if (oopen (&table -> d_cursor, lda, NULL, -1, -1, NULL, -1))
		oraclecda_error (lda, &table -> d_cursor, "DeleteRow");

/*
	if (!table -> delete)
	{
*/
		/*
		 *	Create SQL statement and bind input variables
		 */

		BuildStatement (lda, table);
		BindInputVariables (lda, table);
/*
	}
*/

	/*
	 *	Attempt the delete
	 */
	if (oexec (&table -> d_cursor))
        {
                oclose(&table -> d_cursor );
		return 1;
        }
        oclose( &table -> d_cursor );
	return 0;
	/*oraclecda_error (lda, &table -> d_cursor, "DeleteRow");*/
}

static void
BuildStatement (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Build a SQL delete statement
	 */
	char sqlstr [2048];		/* hopefully be long enough */

	sprintf (sqlstr,
		"delete from %s where rowid = :1",
		table -> table ? table -> table : table -> named);

        if ( table -> delete )
           free( table -> delete );
	table -> delete = strdup (sqlstr);

	/*
	 *	Attempt the parse
	 */
	if (oparse (&table -> d_cursor, table -> delete, -1, PARSE_NOW, PARSE_V7))
		oraclecda_error (lda, &table -> d_cursor, "BuildStatement");
}

static void
BindInputVariables (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Inform ORACLE about input parameters
	 *
	 *	Tie the rowid column to the delete statement
	 *
	 *	The input lengths for the ROWID, expects the input length
	 *	to be the length of the incoming string result
	 *	instead of the length of the data-storage buffer
	 */
	int rowidcol = table -> columnc - 1;

	if (obndrn (&table -> d_cursor, 1,
			(ub1 *) table -> columns [rowidcol].data,
			strlen (table -> columns [rowidcol].data),
			_internal_sqltype (table -> columns [rowidcol].type),
			-1,
			&table -> columns [rowidcol].null_ind, 
			NULL, -1, -1))
	{
		oraclecda_error (lda, &table -> d_cursor, "BindInputVars");
	}
}

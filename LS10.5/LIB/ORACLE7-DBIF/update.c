#ident	"$Id: update.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Updates to the database
 *
 *******************************************************************************
 *	$Log: update.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.2  2001/04/06 02:09:53  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/10/21 21:47:05  jonc
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
_UpdateRow (
 Lda_Def * lda,
 TableState * table,
 void * appbuf)
{
	/*
	 *	Update the current row
	 */
	int i;
 char msg[255];



/*
	if (!table -> update)
	{
*/
		/*
		 *	Create cursor, SQL statement and bind input variables
		 */
		 
                if (oopen (&table -> u_cursor, lda, NULL, -1, -1, NULL, -1))
		    oraclecda_error (lda, &table -> u_cursor, "UpdateRow");
	
		BuildStatement (lda, table);
		BindInputVariables (lda, table);
/*
	}
*/
	/*
	 *	Update internal buffer from application buffer
	 */
	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

		column -> null_ind = 0;
		_ConvApp2Raw (
			column,
			table -> view [i].vwstart,
			appbuf);
	}
	
	/*
	 *	Attempt the update
	 */
	if (oexec (&table -> u_cursor))
	{
                oerhms(lda, table ->u_cursor.rc, msg, (sword) sizeof msg);
printf("\n\nMESSAGE: %s\n", msg );
		return 1;
	/*	oraclecda_error (lda, &table -> u_cursor, "UpdateRow"); */
	}
	return 0;
}

static void
BuildStatement (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Build a SQL update statement
	 */
	int i, end = table -> columnc - 1;
	char sqlstr [4096];		/* hopefully be long enough */

	sprintf (sqlstr,
		"update %s set ",
		table -> table ? table -> table : table -> named);
		
	/*
	 *	Put in all column names except for the rowid
	 *	(which should be the last column)
	 */
	for (i = 0; i < end; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s = :%d%s ",
			table -> columns [i].name,
			i + 1,
			i + 1 < end ? "," : "");
	}
	
	/*
	 *	Use the rowid as our conditional
	 */
	sprintf (sqlstr + strlen (sqlstr),
		"where rowid = :%d",
		table -> columnc);

	/*
	 */
	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 *	Best to die from possible memory corruption
		 */
		oracledbif_error ("BuildStatement. SQL update too big");
	}

	/*
	 */
	table -> update = strdup (sqlstr);

	/*
	 *	Attempt the parse
	 */
	if (oparse (&table -> u_cursor, table -> update, -1, PARSE_NOW, PARSE_V7))
		oraclecda_error (lda, &table -> u_cursor, "BuildStatement");
}

static void
BindInputVariables (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Inform ORACLE about input parameters
	 *
	 *	Tie all the columns to the update statement
	 */
	int i;

	for (i = 0; i < table -> columnc; i++)
	{
		/*
		 *	The exception to input lengths would be that for
		 *	the ROWID, which expects the input length
		 *	to be the length of the incoming string result
		 *	instead of the length of the data-storage buffer
		 */
		int length = table -> columns [i].type == OT_RowId ?
				strlen (table -> columns [i].data) :
				table -> columns [i].length;

		if (obndrn (&table -> u_cursor, i + 1,
				(ub1 *) table -> columns [i].data,
				length,
				_internal_sqltype (table -> columns [i].type),
				-1,
				&table -> columns [i].null_ind, 
				NULL, -1, -1))
		{
			oraclecda_error (lda, &table -> u_cursor, "BindInputVars");
		}
	}
}

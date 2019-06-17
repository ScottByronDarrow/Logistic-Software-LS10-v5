#ident	"$Id: insert.c,v 5.1 2001/06/21 08:01:01 cha Exp $"
/*
 *	Inserts to the database
 *
 *******************************************************************************
 *	$Log: insert.c,v $
 *	Revision 5.1  2001/06/21 08:01:01  cha
 *	Updated to handle correctly the Money datatype.
 *	
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:53  cha
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
#include 	<stdlib.h>

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
_InsertRow (
 Lda_Def * lda,
 TableState * table,
 void * appbuf)
{
	/*
	 *	Insert the current row
	 */
	int i;
	char msg [255];


	if (oopen (&table -> i_cursor, lda, NULL, -1, -1, NULL, -1))
	{
		oerhms(lda, table ->u_cursor.rc, msg, (sword) sizeof msg);	
		oraclecda_error (lda, &table -> i_cursor, "InsertRow");
	}

/*
        if (!table -> insert)
	{
*/
		/*
		 *	Create SQL statement and bind input variables
		 */
		BuildStatement (lda, table);
		BindInputVariables (lda, table);
	/*}*/

	/*
	 *	Insert internal buffer from application buffer
	 *
	 *	All columns not represented by the view will be set to NULL
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		table -> columns [i].null_ind = -1;
	}
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
	 *	Attempt the insert
	 */
	if (oexec (&table -> i_cursor))
        {
	  		 oclose (&table -> i_cursor);
		 	 oerhms(lda, table ->u_cursor.rc, msg, (sword) sizeof msg);
			 return 1;
        }
	oclose (&table -> i_cursor);
	return 0;
	/* oraclecda_error (lda, &table -> i_cursor, "InsertRow"); */
}



static void
BuildStatement (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Build a SQL insert statement
	 */
	int i, end = table -> columnc - 1;
	char sqlstr [4096];		/* hopefully be long enough */

	sprintf (sqlstr,
		"insert into %s (",
		table -> table ? table -> table : table -> named);

	/*
	 *	Put in all column names except for the rowid
	 *	(which should be the last column)
	 */
	for (i = 0; i < end; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s%s ",
			table -> columns [i].name,
			i + 1 < end ? "," : ")");
	}

	strcat (sqlstr, "values (");

	for (i = 0; i < end; i++)
	{
		/*
		 *	Serial fields use a sequence with the field's name
		 *	to get the next value.
		 */
		if (table -> columns [i].type == OT_Serial)
		{
			sprintf (sqlstr + strlen (sqlstr),
				"%s.nextval%s ",
				table -> columns [i].name,
				i + 1 < end ? "," : ")");
		} else
		{
			sprintf (sqlstr + strlen (sqlstr),
				":%d%s ",
				i + 1,
				i + 1 < end ? "," : ")");
		}
	}

	/*
	 */
	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 *	Best to die from possible memory corruption
		 */
		oracledbif_error ("BuildStatement. SQL insert too big");
	}

	/*
	 */
        if ( table -> insert )
          free( table-> insert );
          
	table -> insert = strdup (sqlstr);

	/*
	 *	Attempt the parse
	 */
	if (oparse (&table -> i_cursor, table -> insert, -1, PARSE_NOW, PARSE_V7))
		oraclecda_error (lda, &table -> i_cursor, "BuildStatement");
}

static void
BindInputVariables (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Inform ORACLE about input parameters
	 *
	 *	Tie all the columns (except the last one, which is the rowid)
	 *	to the insert statement
	 */
	int i;
	int end = table -> columnc - 1;

	for (i = 0; i < end; i++)
	{
		if (table -> columns [i].type == OT_Serial)
		{
			/*
			 *	Skip serial fields as the SQL insert input field
			 *	has a sequence value attached to it instead
			 */
			continue;
		}

		if (obndrn (&table -> i_cursor, i + 1,
				(ub1 *) table -> columns [i].data,
				table -> columns [i].length,
				_internal_sqltype (table -> columns [i].type),
				-1,
				&table -> columns [i].null_ind, 
				NULL, -1, -1))
		{
			oraclecda_error (lda, &table -> i_cursor, "BindInputVars");
		}
	}
}

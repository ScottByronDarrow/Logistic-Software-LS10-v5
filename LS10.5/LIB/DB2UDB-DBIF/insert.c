#ident	"$Id: insert.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Inserts to the database
 *
 *******************************************************************************
 *	$Log: insert.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.4  2000/09/29 12:12:26  gerry
 *	Fixed insert to work on applications that use tables with primary serial key but not included in application view
 *	
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
 * Global variables
 */
SQLRETURN rc;

/*
 *	Local functions
 */
static void BuildStatement (TableState *),
			BindInputVariables (TableState *, void *);

/*
 *	External interface
 */
int
_InsertRow (
 TableState * table,
 void * appbuf)
{
	/*
	 *	Insert the current row
	 */
	int i;
	int ret = SQL_SUCCESS;
	SQL_DATE_STRUCT db2_date;

	/*if (!table -> insert)*/
		
		/*
		 * Allocate a db2 statement handle
		 */
		rc = SQLAllocHandle( SQL_HANDLE_STMT, *table -> hdbc , &table -> i_hstmt );
		CHECK_HANDLE( SQL_HANDLE_DBC, *table -> hdbc, rc );
		table -> i_flag = 1;

		BuildStatement (table);
		BindInputVariables (table, appbuf);

	/*
	 *	Insert internal buffer from application buffer
	 *
	 *	All columns not represented by the view will be set to NULL
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		table -> columns [i].null_ind = SQL_NULL_DATA;
	}

	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];


			column -> null_ind = (long) NULL;
			_ConvApp2Raw (column,
				table -> view [i].vwstart,
				appbuf,"i");

	}

	/* 
	 * Check values not included in the views for anomalous values.
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		/* 
		 * Null dates would be converted to 1/1/1900 to avoid SQLSTATE 22007, CLI0113E 
		 */
		if ((table -> columns [i].null_ind == SQL_NULL_DATA) && (table -> columns [i].type == DB2_DATE))
		{
			db2_date.day = 1;
			db2_date.month = 1;
			db2_date.year = 1900;
			*(SQL_DATE_STRUCT*)table -> columns [i].data = db2_date;
		}
		/*
		 * A single space character for null strings.
		 */
		else if ((table -> columns [i].null_ind == SQL_NULL_DATA) && (table -> columns [i].type == DB2_CHAR))
			strcpy (table -> columns [i].data , " ");
	
	}

	/*
	 *	Attempt the insert
	 */
	rc = SQLExecute(table -> i_hstmt);
		CHECK_HANDLE( SQL_HANDLE_STMT, table -> i_hstmt, rc );
	ret = ret || rc;
	/*
	 * Commit the changes
	 */
	rc = SQLEndTran(SQL_HANDLE_DBC, *table -> hdbc, SQL_COMMIT);
		CHECK_HANDLE( SQL_HANDLE_STMT, *table -> hdbc, rc ) ;
	ret = ret || rc;

	rc = SQLFreeHandle( SQL_HANDLE_STMT, table -> i_hstmt ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, table -> i_hstmt, rc ) ;
	table -> i_flag = 0;

	return ret;

}

static void
BuildStatement (
 TableState * tablenm)
{
	/*
	 *	Build a SQL insert statement
	 */
	int i;
	char sqlstr [2048];		/* hopefully be long enough */

	sprintf (sqlstr,
		"insert into %s (",
		tablenm -> table ? tablenm -> table : tablenm -> named);

	/*
	 *	Put in all column names 
	 */
	/*for (i = 0; i < tablenm -> viewc; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s%s ",
			tablenm -> view [i].vwname,
			i + 1 < tablenm -> viewc ? "," : ")");
	}*/

	for (i = 0; i < tablenm -> columnc; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s%s ",
			tablenm -> columns[i].name,
			i + 1 < tablenm -> columnc ? "," : ")");
	}


	strcat (sqlstr, "values (");

	/*for (i = 0; i < tablenm -> viewc; i++)
	{
	
		sprintf (sqlstr + strlen (sqlstr),
				"%s%s ",
				"?",
				i + 1 < tablenm -> viewc ? "," : ")");

	}*/

	for (i = 0; i < tablenm -> columnc; i++)
	{
	
		sprintf (sqlstr + strlen (sqlstr),
				"%s%s ",
				"?",
				i + 1 < tablenm -> columnc ? "," : ")");

	}

	/*
	 */
	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 *	Best to die from possible memory corruption
		 */
		printf("BuildStatement. SQL insert too big");
		
	}

	/*
	 * Store the statement into the insert field of the table structure
	 * and quit the function.
	 */
	tablenm -> insert = strdup (sqlstr); 

	/* Prepare statement handle */
	rc = SQLPrepare(tablenm -> i_hstmt, sqlstr, SQL_NTS);
		CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> i_hstmt, rc ) ;
}

static void
BindInputVariables (
 TableState * tablenm,
 void * appbuf)
{
	/*
	 *	Inform DB2 about input parameters
	 *
	 *	Tie all the columns 
	 *	to the insert statement
	 */
	int i;
	/*int view_th;*/
	SQLUINTEGER column_size = 0;
	SQLSMALLINT decimal_digits = 0;
	SQLINTEGER buffer_len = 0;

	for (i = 0; i < tablenm -> columnc; i++)
	/*for (i = 0, rc = 0; i < tablenm -> viewc; i++)*/
	{
		/*view_th = tablenm->viewactual[i];*/
		if (SQL_SUCCESS == rc) /* proceed if all is well */
		{
			/*int sql_input = _external_sqltype(tablenm -> columns [view_th].type);*/
			int sql_input = _external_sqltype(tablenm -> columns [i].type);

		/*switch(tablenm -> columns [view_th].type)*/
		  switch(tablenm -> columns [i].type)
			{
			case DB2_CHAR:
				/*column_size = tablenm -> columns [view_th].length;*/
				column_size = tablenm -> columns [i].length;
				decimal_digits = 0;
				buffer_len = column_size;
				break;
			case DB2_DECIMAL:
				/*column_size = tablenm -> columns [view_th].length;*/
				column_size = tablenm -> columns [i].length;
				decimal_digits = 4;
				buffer_len = column_size;
				break;
			default:
				column_size = 0;
				decimal_digits = 0;
				buffer_len = 0;
				break;
			}
			/*rc = SQLBindParameter(tablenm -> i_hstmt,
								  (SQLUSMALLINT)(i+1),
							      SQL_PARAM_INPUT,
							      SQL_C_DEFAULT,
							      (SQLSMALLINT) sql_input,
								  column_size,
							      decimal_digits,
							      (SQLPOINTER)tablenm -> columns[view_th].data,
							      buffer_len,
								  &tablenm -> columns[view_th].null_ind);*/
		  rc = SQLBindParameter(tablenm -> i_hstmt,
								  (SQLUSMALLINT)(i+1),
							      SQL_PARAM_INPUT,
							      SQL_C_DEFAULT,
							      (SQLSMALLINT) sql_input,
								  column_size,
							      decimal_digits,
							      (SQLPOINTER)tablenm -> columns[i].data,
							      buffer_len,
								  &tablenm -> columns[i].null_ind);
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> i_hstmt, rc );
		}
		else
			break;
	}
	return;
}
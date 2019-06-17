#ident	"$Id: update.c,v 5.0 2001/06/19 07:08:21 cha Exp $"
/*
 *	Updates to the database
 *
 *******************************************************************************
 *	$Log: update.c,v $
 *	Revision 5.0  2001/06/19 07:08:21  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.4  2000/10/12 13:31:29  gerry
 *	Removed Ctrl-Ms
 *	
 *	Revision 1.3  2000/09/25 09:48:47  gerry
 *	DB2 Release 2 - After major fixes
 *	
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
 * Global variable
 */
SQLRETURN rc;

typedef struct DATE_STRUCT DB2_Date;

static void _epf_spy_columns(TableState * table)
{
int i = 0;
DB2_Date db2_date;

if (strcmp(table->named,"insf"))
	return;



for (i=0;i< table->columnc;i++)
{
	printf("column number: %d, column name: %s, column length: %d, columns null_ind: %d, ",i,table->columns[i].name,table->columns[i].length,table->columns[i].null_ind);

	switch (table->columns[i].type)
	{
		case DB2_CHAR:
			printf("Char value: %s\n",table->columns[i].data);
			break;
		case DB2_DATE:
			db2_date = *(DB2_Date*)table->columns[i].data;
			printf("Date value: %d/%d/%d\n",db2_date.month, db2_date.day, db2_date.year);
			break;
		case DB2_INTEGER:
			printf("Integer value: %d\n", *(int*)table->columns[i].data);
			break;
		case DB2_REAL:
			printf("Real value: %f\n", *(float*)table->columns[i].data);
			break;
		case DB2_DOUBLE:
			printf("Double value: %f\n", *(double*)table->columns[i].data);
			break;
		case DB2_FLOAT:
			printf("Float value: %f\n", *(double*)table->columns[i].data);
			break;
		case DB2_DECIMAL:
		case DB2_NUMERIC:
			printf("Decimal value: %s\n", table->columns[i].data);
			break;

		default:
			printf ("userdatasize: unknown data-type %d\n", table->columns[i].type);
	}

}

}

int
_UpdateRow (
 TableState * table,
 void * appbuf)
{
	/*
	 *	Update the current row
	 */
	int i;
	int ret = SQL_SUCCESS;

	/*
	 * Columns not represented by the view should not be updated
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		table -> columns [i].null_ind = SQL_COLUMN_IGNORE;
	}

	/*
	 *	Update internal buffer from application buffer
	 */

	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];
		
		/*if (column->type != DB2_DECIMAL)*/
		_ConvApp2Raw (column,
			table -> view [i].vwstart,
			appbuf, "u");
	}

	/*
	 *	Positions cursor to a row and updates it.
	 */
/*	_epf_spy_columns(table);*/

/*	rc = SQLEndTran(SQL_HANDLE_DBC, *table -> hdbc, SQL_COMMIT);
	CHECK_HANDLE(SQL_HANDLE_STMT, *table -> hdbc, rc);
	*/
	rc = SQLSetPos( table -> q_hstmt, 0, SQL_UPDATE , SQL_LOCK_NO_CHANGE);
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

#ident	"$Id: query.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Query related functions
 *
 *******************************************************************************
 *	$Log: query.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.4  2000/10/12 13:31:29  gerry
 *	Removed Ctrl-Ms
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sqlcli1.h>
#include	"samputil.h"
#include	"db2io.h"
#include	"db2dbif.h"

/*
 *	Local functions
 */
static void BuildStatement (TableState * tablenm, int mode, int, const char *),
			BindInputVariables (TableState * tablenm),
			BindOutputVariables (TableState * tablenm);
static const char * ModeToString (int mode, int islastcol);

char * _rtrim ( char * , char * );

typedef struct DATE_STRUCT DB2_Date;

/*
 *	External interface
 */

SQLRETURN rc;

void
QuerySetup (
 TableState * tablenm,
 int mode,
 int abs_posn,
 void * buffer,
 const char * lock)
{
	int i;

	if ((mode != tablenm -> lastfetchmode) || (!tablenm -> query) || (0 != strcmp(lock, tablenm -> lastlockmode)))
	/*if(TRUE)*/
	{
		
			/* commit */
			/*rc = SQLEndTran(SQL_HANDLE_DBC, *tablenm -> hdbc , SQL_COMMIT);
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt , rc ) ;*/

			/* unbind all previous bind parameter and columns */
			rc = SQLFreeStmt( tablenm -> q_hstmt, SQL_UNBIND ) ;
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;

			rc = SQLFreeStmt( tablenm -> q_hstmt, SQL_RESET_PARAMS ) ;
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;

			rc = SQLFreeStmt( tablenm -> q_hstmt, SQL_CLOSE ) ;
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;

		/*
		 *	Build a new statement
		 */
		BuildStatement (tablenm, mode, abs_posn, lock);
		/*
		 *	Bind input/output
		 */
		if (!abs_posn)
			BindInputVariables (tablenm);

	BindOutputVariables (tablenm);
	
	}
		
		rc = SQLFreeStmt( tablenm -> q_hstmt, SQL_CLOSE ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;

		/*
		 * Supply data if there is a where clause
		 */
		if (!abs_posn)
		{
		   /*
			*	Convert the index values from the application buffer
			*	to the internal buffer
			*/

			for (i = 0; i < tablenm -> indexc; i++)
			{
				char *string;
		
				ColumnDef * column = tablenm -> columns + tablenm -> indexactual [i];
				column -> null_ind = 0;
				_ConvApp2Raw (column,
					tablenm -> view [tablenm -> indexview [i]].vwstart,
					buffer,"q");
				if (((mode == LTEQ) || (mode == LT)) && (column->type == DB2_CHAR))
				{
					string = strdup(column -> data);
					if (!strcmp(column -> data, memset(string,' ',strlen(string))))
						memset(column -> data,'~',strlen(column -> data));	
				}
			}
		}
	
		/*
		 *	Attempt execution
		 */
		
		 /*for( i = 0;i<tablenm->columnc;i++)
			{
		printf("column name: %s -> length: %d -> type: %d -> i: %d internal type: %d\n",tablenm->columns[i].name,tablenm->columns[i].length, tablenm->columns[i].type, i, _internal_sqltype (tablenm -> columns [i].type));
			}*/
	
		rc = SQLExecute( tablenm -> q_hstmt );
		CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc );
		if (rc != SQL_SUCCESS)
			printf("\n\n\nError occured while SqlExecute on table: %s\n\n\n", tablenm->named);

	    /*
		 */
		tablenm -> lastfetchmode = mode;
		tablenm -> lastlockmode = lock;
	
}

int
QueryFetch (
 TableState * tablenm,
 void * buffer,
 SQLSMALLINT mode)
{
	int i;
	
	/*if (SQLFetch(tablenm -> q_hstmt ) == SQL_NO_DATA)*/
	if ((rc = SQLFetchScroll (tablenm -> q_hstmt, mode, 0)) == SQL_NO_DATA)
	{

			/* ISSUE: a cursor holds lock to all the rows included in the result set
		     * closing the cursor with out WITH RELEASE clause doesn't free up the locks
			 * by default the WITH HOLD is used.  This is good since we use current of 
			 * in our DMLs. If we commit here does it release the locks (good
			 * for concurrency) only for the current result set and does not have
			 * an impact on our other result sets.
			 *
			 * NOTE: it does destroy the result set that is why referencing DMLs 
			 * will be lost.  Inquire about specific transaction life cycle and
			 * fine tune the commit and duration of locks accordingly.
		     */
		rc = SQLEndTran(SQL_HANDLE_DBC, *tablenm -> hdbc , SQL_COMMIT);
			CHECK_HANDLE( SQL_HANDLE_STMT, *tablenm -> hdbc , rc ) ;


			/* Since result have all been fetched, close the cursor
			 * to free up resources but leave the bind columns and 
			 * parameters since the same statement could be used again 
			 * with different bound values.
		     */	

			/*rc = SQLFreeStmt( tablenm -> q_hstmt, SQL_CLOSE ) ;
			CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;*/

			/*
			 *	do not destroy the result set, application could back track
			 *  in case it didn't find what it's looking for and settle for 
			 *  that most previous value.
			 */
			

	
			return FALSE;

	}
	if (rc != SQL_SUCCESS)
	{
		CHECK_HANDLE( SQL_HANDLE_STMT,  tablenm -> q_hstmt, rc ) ;

		printf("FAILED SQL STATEMENT -> %s\n",tablenm->query);
		return FALSE;
	}
	/*_epf_spy_columns(tablenm);*/

	/*
	 *	Map output to application buffers
	 */
	for (i = 0; i < tablenm -> viewc; i++)
	{
		_ConvRaw2App (
			tablenm -> columns + tablenm -> viewactual [i],
			tablenm -> view [i].vwstart,
			buffer);
	}

	return TRUE;	
}

/****/
int
QueryCurrent (
	TableState * tablenm, 
	void * buffer)
{
	int i;
	
	rc = SQLSetPos( tablenm -> q_hstmt, 0, SQL_REFRESH , SQL_LOCK_NO_CHANGE);
	
	if (rc != SQL_SUCCESS)
	{
		CHECK_HANDLE( SQL_HANDLE_STMT,  tablenm -> q_hstmt, rc ) ;

		printf("FAILED SQL STATEMENT -> %s\n",tablenm->query);
		return FALSE;
	}

	/*
	 *	Map output to application buffers
	 */
	for (i = 0; i < tablenm -> viewc; i++)
	{
		_ConvRaw2App (
			tablenm -> columns + tablenm -> viewactual [i],
			tablenm -> view [i].vwstart,
			buffer);
	}

	return TRUE;	
}

/****/
static void
BuildStatement (
 TableState * tablenm,
 int mode,
 int abs_posn,
 const char * lock)
{

	/*SQLSMALLINT clength;*/
	/*
	 *	Build a usable SQL statement
	 */
	int i;
	char sqlstr [2048];		/* should be long enough, initially */

	memset(sqlstr,'\0',2048);

	strcpy (sqlstr, "select ");
	for (i = 0; i < tablenm -> columnc; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s%s ",
			tablenm -> columns [i].name,
			i + 1 < tablenm -> columnc ? "," : "");
	}
	sprintf (sqlstr + strlen (sqlstr),
		"from %s",
		tablenm -> table ? tablenm -> table : tablenm -> named);

	if (tablenm -> indexc)
	{
		/*
		 *	Only build where conditions if an absolute position
		 *	(eg FIRST/LAST) is not required
		 */
		if (!abs_posn)
		{
			/*
			 *	Where conditions are based on the index selected
			 */
			for (i = 0; i < tablenm -> indexc; i++)
			{
				sprintf (sqlstr + strlen (sqlstr),
					" %s %s %s %s",
					i ? "and" : "where",
					tablenm -> columns [tablenm -> indexactual [i]].name,
					ModeToString (mode, i + 1 < tablenm -> indexc),
					"?");
			}
		}

		strcat (sqlstr, " order by");
		for (i = 0; i < tablenm -> indexc; i++)
		{
			sprintf (sqlstr + strlen (sqlstr),
				" %s%s",
				tablenm -> columns [tablenm -> indexactual [i]].name,
				i + 1 >= tablenm -> indexc ? "" : ",");
		}
	}
	
	
	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 *	Best to die from possible memory corruption
		 */
		printf("BuildStatement. SQL statement bigger than buffer");
	}

	if (tablenm -> query)
	{
		free (tablenm -> query);
		tablenm -> query = NULL;
	}
	tablenm -> query = strdup (sqlstr);

	
	/* prepare statement for use */
    /*rc = SQLPrepare(tablenm -> q_hstmt , sqlstr, SQL_NTS);*/
	rc = SQLPrepare(tablenm -> q_hstmt , tablenm -> query, strlen(tablenm -> query));
    CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc );

}

static void
BindInputVariables (
 TableState * table)
{
	/*
	 *	Inform DB2 about input parameters
	 *
	 *	Our selection criteria is based around the index,
	 *	so we only bind columns associated with the index.
	 */
	int i = 0;
	SQLUINTEGER column_size = 0;
	SQLSMALLINT decimal_digits = 0;
	SQLINTEGER buffer_len = 0;

	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = &table -> columns [table -> indexactual [i]];
		int sql_input = _internal_sqltype (column -> type);

		switch(column -> type)
		{
		case DB2_CHAR:
			column_size = 0 ;
			decimal_digits = 0;
			buffer_len = 0;
			break;
		case DB2_DECIMAL:
			column_size = column->length;
			decimal_digits = 4;
			buffer_len = column->length;
			break;
		default:
			column_size = 0;
			decimal_digits = 0;
			buffer_len = 0;
			break;
		}

		/*rc = SQLBindParameter(hstmt,(UWORD)(i+1), SQL_PARAM_INPUT,SQL_C_DEFAULT,
			  (SWORD)sql_input,0,0,
			  (UCHAR *)column -> data,0,NULL);*/

		/****************************************/
		/* epf: save the original, just in case */
		/****************************************/

		/*rc = SQLBindParameter(hstmt,(UWORD)(i+1), SQL_PARAM_INPUT,SQL_C_DEFAULT,
			  (SWORD)sql_input,0,0,
			  (UCHAR *)column -> data,0,NULL);
		CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;*/

		/****************************************/
		/* epf: let's play with this one        */
		/****************************************/
		
		
		rc = SQLBindParameter(table -> q_hstmt ,(SQLUSMALLINT)(i+1), SQL_PARAM_INPUT,SQL_C_DEFAULT,
			  (SQLSMALLINT)sql_input,column_size,decimal_digits,
			  (SQLPOINTER)column -> data,buffer_len,NULL);
		CHECK_HANDLE( SQL_HANDLE_STMT, table -> q_hstmt, rc ) ;


		
	}
}

static void
BindOutputVariables (
 TableState * tablenm)
{
	/*
	 *	Inform DB2 about output buffers for the query
	 */
	int i;


	for (i = 0; i < tablenm -> columnc; i++)
	{
		/****************************************/
		/* epf: save the original, just in case */
		/****************************************/

		/*rc = SQLBindCol(hstmt, (UWORD)(i + 1), (SWORD)_internal_sqltype (tablenm -> columns [i].type),
		     (SQLPOINTER) tablenm -> columns [i].data, tablenm -> columns [i].length,
                    &tablenm -> columns [i].null_ind);
		CHECK_HANDLE( SQL_HANDLE_STMT, hstmt, rc ) ;*/
		
		/****************************************/
		/* epf: let's play with this one        */
		/****************************************/
		/*printf("column name: %s -> length: %d -> type: %d -> i: %d internal type: %d columnc: %d\n",tablenm->columns[i].name,tablenm->columns[i].length, tablenm->columns[i].type, i, _internal_sqltype (tablenm -> columns [i].type), tablenm -> columnc);*/

		rc = SQLBindCol(tablenm -> q_hstmt , (SQLUSMALLINT)(i + 1), (SQLSMALLINT)_internal_sqltype (tablenm -> columns [i].type),
		     (SQLPOINTER) tablenm -> columns [i].data, tablenm -> columns [i].length,
                    &tablenm -> columns [i].null_ind);
		CHECK_HANDLE( SQL_HANDLE_STMT, tablenm -> q_hstmt, rc ) ;
		
	}
}


int FetchAbsolute(TableState * tablenm, void * buffer, int mode, SQLSMALLINT direction)
{
	int i, _mode, found = FALSE, gotcha = TRUE;

	if (direction == SQL_FETCH_NEXT)
		rc = SQLFetchScroll (tablenm -> q_hstmt, SQL_FETCH_NEXT, 0);
	else
		rc = SQLFetchScroll (tablenm -> q_hstmt, SQL_FETCH_LAST, 0);

	if (rc == SQL_NO_DATA)
	{
		rc = SQLEndTran(SQL_HANDLE_DBC, *tablenm -> hdbc , SQL_COMMIT);
		CHECK_HANDLE( SQL_HANDLE_STMT, *tablenm -> hdbc , rc ) ;
		return FALSE;
	}

	if (rc != SQL_SUCCESS)
	{
		CHECK_HANDLE( SQL_HANDLE_STMT,  tablenm -> q_hstmt, rc ) ;
		printf("FAILED SQL STATEMENT -> %s\n",tablenm->query);
		return FALSE;
	}

	/*
	 * An abosolute value is required,
	 * search for the value by comparing values from the 
	 * Application buffer's search criteria and the record fetched.
	 */
	/*tablenm -> view [tablenm -> indexview [i]].vwstart*/

	for (i = 0, gotcha = TRUE; i < tablenm -> indexc && gotcha; i++)
	{
		if ((i + 1 >= tablenm -> indexc) && (mode == GREATER))
			_mode = GTEQ;
		else if ((i + 1 >= tablenm -> indexc) && (mode == LT))
			_mode = LTEQ;
		else
			_mode = mode;
		gotcha = gotcha && _CompRaw2App (
								tablenm -> columns + tablenm -> indexactual [i],
								tablenm -> view [tablenm -> indexview [i]].vwstart,
								buffer, mode);
	}
	if (gotcha)
		found = TRUE;

	while (!found)
	{
		if ((rc = SQLFetchScroll (tablenm -> q_hstmt, direction, 0)) == SQL_NO_DATA)
		{

				rc = SQLEndTran(SQL_HANDLE_DBC, *tablenm -> hdbc , SQL_COMMIT);
				CHECK_HANDLE( SQL_HANDLE_STMT, *tablenm -> hdbc , rc ) ;
				return FALSE;

		}
		if (rc != SQL_SUCCESS)
		{
			CHECK_HANDLE( SQL_HANDLE_STMT,  tablenm -> q_hstmt, rc ) ;

			printf("FAILED SQL STATEMENT -> %s\n",tablenm->query);
			return FALSE;
		}

		/*
	     * An abosolute value is required,
	     * search for the value by comparing values from the 
	     * Application buffer's search criteria and the record fetched.
	     */

		for (i = 0, gotcha = TRUE; i < tablenm -> indexc && gotcha; i++)
		{
			if ((i + 1 >= tablenm -> indexc) && (mode == GREATER))
				_mode = GTEQ;
			else if ((i + 1 >= tablenm -> indexc) && (mode == LT))
				_mode = LTEQ;
			else
				_mode = mode;
			gotcha = gotcha && _CompRaw2App (
								tablenm -> columns + tablenm -> indexactual [i],
								tablenm -> view [tablenm -> indexview [i]].vwstart,
								buffer, _mode);
		}
		if (gotcha)
			found = TRUE;
	}

	/*
	 *	Map output to application buffers
	 */
	for (i = 0; i < tablenm -> viewc; i++)
	{
		_ConvRaw2App (
			tablenm -> columns + tablenm -> viewactual [i],
			tablenm -> view [i].vwstart,
			buffer);
	}

	return TRUE;
}

/*
 */
static const char *
ModeToString (
 int mode,
 int islastcol)
{
	static char
		*Eq		= "=",
		*Gt		= ">",
		*GtEq	= ">=",
		*Lt		= "<",
		*LtEq	= "<=";

	switch (mode)
	{
	case EQUAL:
	case COMPARISON:
		return Eq;

	case GTEQ:
		return islastcol ? GtEq : Gt;

	case GREATER:
		return islastcol ? Gt : GtEq;

	case LT:
		return islastcol ? Lt : LtEq;

	case LTEQ:
		return islastcol ? LtEq : Lt;
	}

	printf ("Bad comparison mode: %d", mode);
	return NULL;
}

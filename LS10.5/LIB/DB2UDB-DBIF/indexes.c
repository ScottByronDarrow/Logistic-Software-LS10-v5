#ident	"$Id: indexes.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Extract index info from system catalogs
 *
 *******************************************************************************
 *	$Log: indexes.c,v $
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
 *	
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlcli1.h>
#include "samputil.h"

#include	"db2io.h"

#include	"db2dbif.h"

#define MAXNAMELEN 19 /* max col length of NAME for table sysindexes + 1*/
#define MAXTBLELEN 129 /* max col length of TBNAME for table sysindexes + 1*/
#define MAXIDXSNAMELEN 641 /*max col length of COLNAMES for table sysindexes + 1*/
/*
 *	Static data areas for index query results
 */
/**************** BEWARE of GLOBAL variables ********************************/
/*SQLRETURN rc;*/

/*static char index_name [MAXNAMELEN], */  /* COL LENGTH +1 */
			/*table_name [MAXTBLELEN]; *//* COL LENGTH +1 */

/*int	col_count = 0;*/

/*SQLINTEGER ind;*/
/*SQLCHAR  column_names[MAXIDXSNAMELEN];*/

/**************** BEWARE of GLOBAL variables ********************************/
/*SQLINTEGER row_array_size = 17 ;*/
/*SQLINTEGER ind = SQL_NTS ;*/

/*
 *	Local functions
 */
static void IndexSetup (SQLHANDLE *, char *, char *, char *);

/*
 *	External interface
 */
void
IdentifyIndex (
 SQLHANDLE * hdbc,
 SQLHANDLE * misc_hstmt,
 TableState * table,
 const char * index)
{
	/*
	 *	Indentify the index and associated columns
	 */
	SQLRETURN rc;
	char index_name [MAXNAMELEN], /* COL LENGTH +1 */
	     table_name [MAXTBLELEN];
	int	col_count = 0;
	
	SQLCHAR  column_names[MAXIDXSNAMELEN];

	int i;
	char colstring[MAXIDXSNAMELEN];
	char sep_strs[10][MAXNAMELEN];
	char *colname_ptr = '\0';

	memset (column_names, '\0', MAXIDXSNAMELEN * sizeof (char));
	memset (index_name, '\0', MAXNAMELEN * sizeof (char));
	memset (table_name, '\0', MAXTBLELEN * sizeof (char));

	memset (colstring, '\0', MAXIDXSNAMELEN * sizeof (char));
	memset (sep_strs, '\0', (10 * MAXNAMELEN) * sizeof (char));
	memset (column_names, '\0', MAXIDXSNAMELEN * sizeof (char));

/*	IndexSetup (hdbc, &misc_hstmt);
*/
	/*
	 *	Reallocation?
	 */

	if (table -> indexname)
	{
		if (!strcmp (table -> indexname, index))
			return;						/* no new work */

		free (table -> indexname);
		table -> indexname = NULL;
	}

	if (table -> indexc)
	{
		table -> indexc = 0;
		free (table -> indexactual);
		table -> indexactual = NULL;
		free (table -> indexview );
		table -> indexview = NULL;
	}

	if (!index)
		return;							/* table has no index */

	table -> indexname = strcpy (malloc (strlen (index) + 1), index);

	/*
	 *	System catalogs uses uppercase names for tables and indexes
	 */
	_strupper (strcpy (index_name, index));
	_strupper (
		strcpy (table_name, table -> table ? table -> table : table -> named));

	IndexSetup(misc_hstmt, table_name, index_name, column_names);

	/*
	 *	Determine column count
	 */
	col_count = 0;						/* reset - just in case */
	
	rc = SQLExecute(*misc_hstmt);
	CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

	rc = SQLFetch(*misc_hstmt);
	CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

	rc = SQLEndTran(SQL_HANDLE_DBC, *hdbc , SQL_COMMIT);
	CHECK_HANDLE( SQL_HANDLE_STMT, *hdbc , rc ) ;

	/*The query results to a string with all the columns for the index*/
	/*we will try to separate each column into an array*/

	/*Copy the bind parameter value to a string*/
	strcpy(colstring,column_names);

	if (!column_names)
		printf("There are no columns for the the index");
	
	/*Begin creating the array so we could count the columns in the index*/
	colname_ptr = strtok(colstring, "+");

	do
	{
		
		strcpy(sep_strs[col_count], colname_ptr);
		col_count++;
	}
	while ((colname_ptr = strtok(NULL, "+")));

	if (!col_count)
		printf("No index columns for %s?", table_name);
										
										
	/*
	 *	Allocate buffers for index columns
	 */
	table -> indexc = col_count;
	table -> indexactual = malloc (table -> indexc * sizeof (int));
	table -> indexview = malloc (table -> indexc * sizeof (int));

	memset (table -> indexactual, 0, table -> indexc * sizeof (int));
	memset (table -> indexview, 0, table -> indexc * sizeof (int));

	/*
	 *	Get the column of the index and xref it against
	 *	our internal column definitions, and the user-view
	 */
	
/*************************** START ********************************************/
	for (i = 0; i < col_count; i++)
	{
		int c;
		
		/*
		 * Match the name against internal definitions
		 */
		
		for (c = 0; c < table -> columnc; c++)
		{
			if (!strcmp(_strlower(sep_strs[i]), table -> columns[c].name ))
				{
				/*
  				 * Store Position of ColumnDef for later use
				 */
				table -> indexactual[i] = c;
				break;
				}
				
		}
		if (c >= table -> columnc)
			printf("Index column %s not found.", sep_strs[i]);
	}


	for (i = 0; i < col_count; i++)
	{
		int c;
		/*
                 * Match the name against the user view
                 */

		for (c = 0; c < table -> viewc; c++)
		{
			if (!strcmp(_strlower(sep_strs[i]), table -> view[c].vwname))
			{
				/*
				 * Store position of dbview of later user
				 */
				table -> indexview[i] = c;
				break;
			}	
		}
		/*if(c >= table -> viewc)
			 printf ("Index column %s not found in dbview", sep_strs[i]);	*/ 
			/* epf: suspect for segmentation */
		
	}

/*********************************** END ************************************/
	/*
	 *	Force an SQL statement rebuild, if required.
	 */
	memset (colstring, '\0', MAXIDXSNAMELEN * sizeof (char));
	memset (sep_strs, '\0', (10 * MAXNAMELEN) * sizeof (char));
	memset (column_names, '\0', MAXIDXSNAMELEN * sizeof (char));

	if (table -> query)
	{
		free (table -> query);
		table -> query = '\0';
	}

	/*
	 * Unbind all bound parameters and columns and close the cursor 
	 * to free up resources. 
	 * A new query would be created for this table anyway.
	 */
	/*rc = SQLFreeStmt( table -> q_hstmt, SQL_UNBIND ) ;
	CHECK_HANDLE( SQL_HANDLE_STMT, table -> q_hstmt, rc ) ;

	rc = SQLFreeStmt( table -> q_hstmt, SQL_RESET_PARAMS ) ;
	CHECK_HANDLE( SQL_HANDLE_STMT, table -> q_hstmt, rc ) ;

	rc = SQLFreeStmt( table -> q_hstmt, SQL_CLOSE ) ;
	CHECK_HANDLE( SQL_HANDLE_STMT, table -> q_hstmt, rc ) ;*/

}

/*
 *	Support functions
 */
static void
IndexSetup ( 
 SQLHANDLE * misc_hstmt,
 char * table_name,
 char * index_name,
 char * column_names)
{
	/*static int setup_done = FALSE;*/

	/*
	 * This *should* return ordered by COLUMN_POSITION
	 */
	SQLINTEGER ind;
	SQLRETURN rc;
	SQLCHAR * sqlstmt = (SQLCHAR *)
		"select colnames "
		"from sysibm.sysindexes "
		"where tbname = ? and name = ?";
	
		rc = SQLFreeStmt( *misc_hstmt, SQL_UNBIND ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

		rc = SQLFreeStmt(*misc_hstmt, SQL_RESET_PARAMS ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

		rc = SQLFreeStmt(*misc_hstmt, SQL_CLOSE ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

		/* Prepare the statement */
		rc = SQLPrepare( *misc_hstmt, sqlstmt, SQL_NTS ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

		/* Bind parameter values*/
		rc = SQLBindParameter( *misc_hstmt,
                           1,
                           SQL_PARAM_INPUT,
                           SQL_C_CHAR,
                           SQL_VARCHAR,
                           MAXTBLELEN,
                           0,
                           table_name,
                           MAXTBLELEN,
                           NULL
                         ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;
		rc = SQLBindParameter( *misc_hstmt,
                           2,
                           SQL_PARAM_INPUT,
                           SQL_C_CHAR,
                           SQL_VARCHAR,
                           MAXNAMELEN,
                           0,
                           index_name,
                           MAXNAMELEN,
                           NULL
                         ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

		/*
		*	Result output and indicators
		*/
		
		rc = SQLBindCol( *misc_hstmt, 1, SQL_C_CHAR, (SQLPOINTER) column_names, MAXIDXSNAMELEN, &ind );
		CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;
	
		/*setup_done = TRUE;*/
	
}

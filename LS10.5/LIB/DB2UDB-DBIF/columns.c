#ident	"$Id: columns.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Extract column info from system catalogs
 *
 *******************************************************************************
 *	$Log: columns.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:55  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.4  2000/10/12 13:31:29  gerry
 *	Removed Ctrl-Ms
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *	
 *	
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlcli1.h>

#include "db2io.h"
#include "db2dbif.h"
#include "samputil.h"

#define MAXLEN 129

/*
 *	Static data areas for column query results
 */
static int	col_count;

static char col_table [MAXLEN];

static int	col_data_precision;

/*static SSHORT	i_data_precision;*/	/* indicators */

/*static USHORT	r_len, r_code;*/									/* discard */

SQLINTEGER      length;
SQLINTEGER      length_ind;
SQLSMALLINT     scale;
SQLINTEGER      scale_ind;
SQLINTEGER      col_data_type_ind;
SQLINTEGER		col_name_ind;
SQLINTEGER		col_buffer_length;
SQLINTEGER		col_buffer_length_ind;

SQLUINTEGER num_tables ;
static int	col_count;

SQLRETURN rc;
/*
 *	Local functions
 */

SQLRETURN init_tables( SQLHANDLE *, char * ),
		  list_column_count( SQLHANDLE *, SQLCHAR *, SQLCHAR * ),
		  list_columns( SQLHANDLE *, SQLCHAR *, SQLCHAR *, char *, char *);
		  


static void	BuildDataBuffer (TableState *),
			BuildColumnDefs (SQLHANDLE *, TableState * table, char *, char *),
			MapViewToColumns (TableState *),
			MapViewToUserData (TableState *);

/*
 */

void IdentifyFields(SQLHANDLE * hdbc, 
					SQLHANDLE * misc_hstmt,
					char * tablename,
					TableState * tablenm)
{
	/*SQLINTEGER      tnum=0;*/
	char * schema;
	/*char tableupper[129];*/

	/*Assign the user name as the schema, needed to initialize table, this is required*/
	schema = "LOGISTIC"; 

	
	/*_strupper(strcpy(tableupper,tablename));*/
    _strupper (strcpy (col_table, tablenm -> table ? tablenm -> table : tablenm -> named));

	/* We have to initialize the table first */
    if ( init_tables( misc_hstmt, col_table) != SQL_SUCCESS)
		{
			rc = SQLEndTran( SQL_HANDLE_DBC, *hdbc, SQL_COMMIT ) ;
			CHECK_HANDLE( SQL_HANDLE_DBC, *hdbc, rc ) ;
			printf("TABLE  %s  NOT FOUND IN DATABASE!!!\n\n\n", col_table);
			exit( -1 );
		}

	/*Determine the column count*/
	/*rc = SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &misc_hstmt ) ;
    CHECK_HANDLE( SQL_HANDLE_DBC, hdbc, rc ) ;*/
	
	col_count = 0;								/*reset*/
	rc = list_column_count( misc_hstmt, schema,col_table);

	/*CHECK_HANDLE( SQL_HANDLE_DBC, *hdbc, rc ) ;*/

	if (!col_count)
		printf("There are no columns for table %s \n",col_table);

	tablenm -> columnc = 0;
	free (tablenm -> columns);
	tablenm -> columns = NULL;
  	
	/*
	 *	Allocate buffers for table-columns
	 *
	 */
	tablenm -> columnc = col_count;
	tablenm -> columns = malloc (tablenm -> columnc * sizeof (ColumnDef));
	memset (tablenm -> columns, 0, tablenm -> columnc * sizeof (ColumnDef));

	BuildColumnDefs (misc_hstmt, tablenm,schema,col_table);
	BuildDataBuffer (tablenm);

	/*
	 *	Set up the userview structures
	 */
	MapViewToColumns (tablenm);
	MapViewToUserData (tablenm);

	/*
	 * Commit to free up any locks 
	 */
	rc = SQLEndTran(SQL_HANDLE_DBC, *hdbc, SQL_COMMIT);
	CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

}

static void
BuildColumnDefs (
 SQLHANDLE * misc_hstmt,
 TableState * tablenm,
 char * schema,
 char * tablename)
{
	/*
	 *	Decode DB2 types into something we can use
	 */
	int i;
	char col_data_type_upper[MAXLEN],
		 col_name [MAXLEN],
		 col_data_type [MAXLEN];

	/*We will build the list for all the columns in the table*/
	/*rc = SQLAllocHandle( SQL_HANDLE_STMT, *hdbc, &misc_hstmt ) ;
    CHECK_HANDLE( SQL_HANDLE_DBC, *hdbc, rc ) ;*/

	rc = list_columns(misc_hstmt ,schema,tablename, col_name, col_data_type);

	memset(col_data_type_upper,'\0',MAXLEN);
	memset(col_name,'\0',MAXLEN);
	memset(col_data_type,'\0',MAXLEN);

	/*for (i = 0; ofetch (cols) == 0; i++)*/
	for (i=0;SQLFetch(*misc_hstmt) == SQL_SUCCESS;i++)
	{
		/*_strlower (strcpy (tablenm -> columns [i].name, col_name);*/
		memset(tablenm -> columns [i].name,'\0',32);
		_strlower (strncpy (tablenm -> columns [i].name, col_name, strlen(col_name)));

		if (col_name !="")
		{
			_strupper (strcpy (col_data_type_upper,col_data_type));
		}

		/*printf("name: %s, type: %s, length: %d, scale: %d, buffer length: %d\n",col_name, col_data_type, length, scale, col_buffer_length);*/
		_DecodeDB2Type (
			tablenm -> columns [i].name, 
			col_data_type_upper, length, col_data_precision, scale,
			scale_ind,
			&tablenm -> columns [i].type,
			&tablenm -> columns [i].length);
	
	memset(col_name,'\0',MAXLEN);
	/* free statement resources */
	}

    rc = SQLFreeStmt(*misc_hstmt, SQL_UNBIND ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_RESET_PARAMS ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_CLOSE ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;
}

static void
BuildDataBuffer (
 TableState * table)
{
	/*
	 *	Allocate row-buffer and set up the data-pointers for each column
	 */
	int i;
	int sz = 0;

	/*
	 *	Initial pass to determine the size
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		sz += table -> columns [i].length;
	}

	table -> datasz = sz;

	free (table -> data);
	table -> data = NULL;

	table -> data = malloc (sz);
	memset (table -> data, 0, sz);

	/*memset (table -> data = malloc (sz), 0, sz);*/ /* e x p a n d */

	/*
	 *	Second pass sets the data-pointers
	 */
	for (sz = i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		table -> columns [i].data = table -> data + sz;
		sz += table -> columns [i].length;
	}
}


static void
MapViewToColumns (
 TableState * table)
{
	/*
	 *	Map the view references to actual ColumnDefs
	 */
	int i;

	free (table -> viewactual);
	table -> viewactual = NULL;

	table -> viewactual = malloc (table -> viewc * sizeof (int));
	memset (table -> viewactual, 0, table -> viewc * sizeof (int));
	for (i = 0; i < table -> viewc; i++)
	{
		int c;

		for (c = 0; c < table -> columnc; c++)
			if (!strcmp (table -> view [i].vwname, table -> columns [c].name))
			{
				/*
				 *	Store position of ColumnDefinition for later use
				 */
				table -> viewactual [i] = c;
				break;
			}

		if (c >= table -> columnc)
			printf("View name: %s not found in columndef \n",table -> view [i].vwname);
	}
}

static void
MapViewToUserData (
 TableState * table)
{
	/*
	 *	Work out the offsets into the user application data-buffer
	 */
	int i;
	int offset = 0;
	/*char * table_rec_name = strcat(table -> named,"rec\0");*/

	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

		offset += _application_align (offset, column -> type);

		table -> view [i].vwstart = offset;
		table -> view [i].vwtype = column -> type;

		table -> view [i].vwlen = column -> type == DB2_CHAR ?
			(column -> length) :					/* just happens to work out */
			_application_size (column -> type);


		offset += table -> view [i].vwlen;
	}
}


/**********************************************************************/
/*init_tables														  */
/*initialize the table to be use									  */							 
/*																	  */
/**********************************************************************/

SQLRETURN init_tables( SQLHANDLE * hstmt, char * tablename) {
 
    char table [MAXLEN];
	SQLINTEGER ind;

	SQLCHAR * sqlstmt = (SQLCHAR *)
			"select name "
			"from sysibm.systables "
			"where name = ?";

	rc = SQLFreeStmt(*hstmt, SQL_UNBIND );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

    rc = SQLFreeStmt(*hstmt, SQL_RESET_PARAMS );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

    rc = SQLFreeStmt(*hstmt, SQL_CLOSE );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

	rc = SQLPrepare( *hstmt, sqlstmt, SQL_NTS );
	CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

		/* Bind parameter values*/
		rc = SQLBindParameter( *hstmt,
                           1,
                           SQL_PARAM_INPUT,
                           SQL_C_CHAR,
                           SQL_VARCHAR,
                           30,
                           0,
                           tablename,
                           11,
                           NULL
                         ) ;
		CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;

	   /*
		*	Result output and indicators
		*/
		
		rc = SQLBindCol( *hstmt, 1, SQL_C_CHAR, (SQLPOINTER) table, MAXLEN, &ind );
		CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;


      /* Now fetch the result set */
/*<-- */
	rc = SQLExecute(*hstmt);
	CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;

	rc = SQLFetch(*hstmt);
	CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc ) ;

	rc = SQLFreeStmt(*hstmt, SQL_UNBIND );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

    rc = SQLFreeStmt(*hstmt, SQL_RESET_PARAMS );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

    rc = SQLFreeStmt(*hstmt, SQL_CLOSE );
    CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

	rc = SQLPrepare( *hstmt, sqlstmt, SQL_NTS );
	CHECK_HANDLE( SQL_HANDLE_STMT, *hstmt, rc );

	return rc;
} /* end init_tables */

/**************************************************************************/
/* list_column_count                                                           */
/*    - schema and tablename should be exact strings, NOT search patterns */
/*                                                                        */
/**************************************************************************/
/*--> SQLL1X52.SCRIPT */
SQLRETURN list_column_count( SQLHANDLE * misc_hstmt,
                        SQLCHAR * schema,
                        SQLCHAR * tablename
                      ) {
/*<-- */
  
/* This is being done solely to count the columns in the table
	so there's no need to bind the columns, it will be done later*/	

	rc = SQLFreeStmt(*misc_hstmt, SQL_UNBIND );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLFreeStmt(*misc_hstmt, SQL_RESET_PARAMS );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLFreeStmt(*misc_hstmt, SQL_CLOSE );
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc );

    rc = SQLColumns(*misc_hstmt, NULL, 0, schema, SQL_NTS,
                    tablename, SQL_NTS, (SQLCHAR *)"%", SQL_NTS);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;
   
    
    /* Fetch each row, and display */
    
	while ((rc = SQLFetch(*misc_hstmt)) == SQL_SUCCESS) {
		col_count++;
	}	
	
                               /* endwhile */
/*<-- */
    
	/* free statement resources */

    rc = SQLFreeStmt( *misc_hstmt, SQL_UNBIND ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_RESET_PARAMS ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_CLOSE ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    return( SQL_SUCCESS ) ;

}  /* list_column_count */


/**************************************************************************/
/* list_columns                                                           */
/*    - schema and tablename should be exact strings, NOT search patterns */
/*                                                                        */
/**************************************************************************/
/*--> SQLL1X52.SCRIPT */
SQLRETURN list_columns( SQLHANDLE * misc_hstmt,
                        SQLCHAR * schema,
                        SQLCHAR * tablename,
						char * col_name,
						char * col_data_type
                      ) {
/*<-- */

	rc = SQLFreeStmt( *misc_hstmt, SQL_UNBIND ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_RESET_PARAMS ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLFreeStmt( *misc_hstmt, SQL_CLOSE ) ;
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) 

	/*This is where we bind the columns to variables*/

    rc = SQLColumns(*misc_hstmt, NULL, 0, schema, SQL_NTS,
                    tablename, SQL_NTS, (SQLCHAR *)"%", SQL_NTS);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 4, SQL_C_CHAR, (SQLPOINTER) col_name, MAXLEN,
                    &col_name_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 6, SQL_C_CHAR, (SQLPOINTER) col_data_type, MAXLEN,
                    &col_data_type_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 7, SQL_C_LONG, (SQLPOINTER) &length,
                    sizeof(length), &length_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

	rc = SQLBindCol(*misc_hstmt, 8, SQL_C_LONG, (SQLPOINTER) &col_buffer_length,
                    sizeof(col_buffer_length), &col_buffer_length_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    rc = SQLBindCol(*misc_hstmt, 9, SQL_C_SHORT, (SQLPOINTER) &scale,
                    sizeof(scale), &scale_ind);
    CHECK_HANDLE( SQL_HANDLE_STMT, *misc_hstmt, rc ) ;

    return( SQL_SUCCESS ) ;

}  /* list_columns */


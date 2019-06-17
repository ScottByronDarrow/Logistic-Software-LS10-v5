/******************************************************************************
 *
 * Source File Name = dbif.c
 *
 * (C) COPYRIGHT Logistic Sotware
 * All Rights Reserved
 * Licensed Materials - Property of Logistic Software
 *
 * 
 * 
 * Function = Include File defining:
 *              DB2 CLI Interface - Constants
 *              DB2 CLI Interface - Data Structures
 *              DB2 CLI Interface - Function Prototypes
 *
 * Operating System = Common C Include File
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlcli1.h>
#include "samputil.h"

#include "db2io.h"
#include "db2dbif.h"
//#include "dbcisam.h"
//#include <std_decs.h>


/*declare variables to be use*/

int g_hdbc_Connected = 0;
int misc_hdbc_Connected = 0;
SQLHANDLE henv;
SQLHANDLE g_hstmt; /* global statement handle */
SQLHANDLE g_hdbc;
SQLRETURN rc;
SQLHANDLE g_misc_hdbc;	/* global connection for all miscellaneous transactions */

static SQLRETURN _connect_setup(SQLHANDLE * hdbc, char * dbname)
{
	rc = SQLAllocHandle( SQL_HANDLE_DBC,
                         henv,
						 hdbc);
	
	if( rc != SQL_SUCCESS ) {
        printf( ">---ERROR while allocating connection handle -----\n" ) ;
        return(SQL_ERROR) ;
	}

	 /* Set AUTOCOMMIT OFF */
    if ( SQLSetConnectAttr( *hdbc,
                            SQL_ATTR_AUTOCOMMIT,
                            ( void * ) SQL_AUTOCOMMIT_OFF, SQL_NTS
                          ) != SQL_SUCCESS ) {
        printf( ">---ERROR while setting AUTOCOMMIT OFF ------------\n" ) ;
        return( SQL_ERROR ) ;
    }

	if ( (rc = SQLConnect( *hdbc,
                     dbname, SQL_NTS,
                     "logistic",    SQL_NTS,
                     "ls10db",    SQL_NTS
                   )) != SQL_SUCCESS ) {
		CHECK_HANDLE( SQL_HANDLE_DBC, *hdbc, rc );
        printf( ">--- Error while connecting to database: %s -------\n",
                dbname
              ) ;
        SQLDisconnect( *hdbc ) ;
        SQLFreeHandle( SQL_HANDLE_DBC, *hdbc ) ;
        return( SQL_ERROR ) ;
    }
    else     
		/* Print Connection Information */
        /*printf( ">Connected to %s\n", dbname ) ;*/
		return( SQL_SUCCESS ) ;
}

SQLRETURN abc_dbopen(char * dbname)
{

	/* allocate a environment handle */
    rc = SQLAllocHandle( SQL_HANDLE_ENV,
                         SQL_NULL_HANDLE,
                         &henv);
	if( rc != SQL_SUCCESS ) {
        printf( ">---ERROR while allocating a environment handle-----\n" ) ;
        return(SQL_ERROR) ;
    }
	
		if (_connect_setup(&g_hdbc, dbname) == SQL_SUCCESS)
		{
			g_hdbc_Connected = 1;
			if (_connect_setup(&g_misc_hdbc, dbname) == SQL_SUCCESS)
				{
					misc_hdbc_Connected = 1;
					/*
					 * if all is well allocate a global statement 
					 * handle from the global misc connection
					 * handle for miscellaneous transactions.
					 */
					rc = SQLAllocHandle( SQL_HANDLE_STMT, g_misc_hdbc , &g_hstmt  ) ;
					CHECK_HANDLE( SQL_HANDLE_DBC, g_misc_hdbc, rc ) ;
					return SQL_SUCCESS;
				}
				else
					return SQL_ERROR;
		}
		else
			return SQL_ERROR;
}		


SQLRETURN abc_dbclose(char * ignored)
{
	
		if(g_hdbc_Connected)
		{
			rc = SQLEndTran(SQL_HANDLE_DBC, g_hdbc , SQL_COMMIT);
		/*	CHECK_HANDLE( SQL_HANDLE_DBC, g_hdbc , rc ) ;
*/
			rc = SQLDisconnect( g_hdbc ) ;
			CHECK_HANDLE( SQL_HANDLE_STMT, g_hdbc , rc);
			if(rc == SQL_SUCCESS)
				g_hdbc_Connected = 0;
			SQLFreeHandle( SQL_HANDLE_DBC, g_hdbc ) ;
		}

		if(misc_hdbc_Connected)
		{
			rc = SQLFreeHandle(SQL_HANDLE_STMT, g_hstmt);
			CHECK_HANDLE( SQL_HANDLE_STMT, g_hstmt, rc);
			
			rc = SQLEndTran(SQL_HANDLE_DBC, g_misc_hdbc , SQL_COMMIT);
	/*		CHECK_HANDLE( SQL_HANDLE_DBC, g_misc_hdbc , rc ) ;
*/
			rc = SQLDisconnect( g_misc_hdbc ) ;
			CHECK_HANDLE( SQL_HANDLE_DBC, g_misc_hdbc , rc);
			if(rc == SQL_SUCCESS)
				misc_hdbc_Connected = 0;
			SQLFreeHandle( SQL_HANDLE_DBC, g_misc_hdbc ) ;
		}
	SQLFreeHandle( SQL_HANDLE_ENV, henv ) ;
	return(0);
}


SQLRETURN find_rec(const char * tablename,
				   void * buffer,
				   int searchtype,
				   const char * lock)

{

	int fetched = FALSE;
	
	TableState * state = LocateTable (tablename);
		
	switch (searchtype)
	{
	case FIRST:				/* Absolute position */
		QuerySetup (state, searchtype, TRUE, buffer, lock);
		fetched = QueryFetch (state, buffer, SQL_FETCH_NEXT);
		break;

	case LAST:				/* Absolute position */
		QuerySetup (state, searchtype, TRUE, buffer, lock);
		fetched = QueryFetch (state, buffer, SQL_FETCH_LAST);
		break;

	case NEXT:				/* Next row */
		fetched = QueryFetch (state, buffer, SQL_FETCH_NEXT);
		break;

	case PREVIOUS:
		fetched = QueryFetch (state, buffer, SQL_FETCH_PRIOR);
		break;
	case COMPARISON:
	case EQUAL:
		QuerySetup (state, searchtype, FALSE, buffer, lock);
		fetched = QueryFetch (state, buffer, SQL_FETCH_NEXT);
		break;
	case GTEQ:
	case GREATER:
		/**************************************************************
		 * select the whole table sorted properly, start fetching     *
		 * from the 1st record and return the very first record that  *
		 * satisfies the where condition.                             *
		 **************************************************************/
		QuerySetup (state, FIRST, TRUE, buffer, lock);
		fetched = FetchAbsolute(state, buffer, searchtype, SQL_FETCH_NEXT);
		break;
	case LT:
		/*
		 * LT is equivalent to a call to GTEQ + PREVIOUS
		 */
		QuerySetup (state, FIRST, TRUE, buffer, lock);
		fetched = FetchAbsolute(state, buffer, GTEQ, SQL_FETCH_NEXT);
		fetched = fetched = QueryFetch (state, buffer, SQL_FETCH_PRIOR);
		break;
		
	case LTEQ:
		QuerySetup (state, FIRST, TRUE, buffer, lock);
		fetched = FetchAbsolute(state, buffer, GREATER, SQL_FETCH_NEXT);
		fetched = fetched = QueryFetch (state, buffer, SQL_FETCH_PRIOR);
		break;

	case CURRENT:
		fetched = QueryCurrent (state, buffer);
		break;

	default:
		printf("Unknown find_rec mode: %d", searchtype);
		/*oracledbif_error ("Unknown find_rec mode: %d", searchtype);*/
	}

	if (!fetched)
		return 1;
	else
		return 0;


}

SQLRETURN open_rec(char * tablename,
				   struct dbview * fields,
				   int count,
				   const char * index)

{
	TableState * state = AllocateStdTable(&g_hdbc, tablename);
	state -> viewc = count;
	state -> view = fields;

		IdentifyFields (&g_misc_hdbc, &g_hstmt, tablename, state);
		IdentifyIndex (&g_misc_hdbc, &g_hstmt, state, index);


	return 0;

}


void
abc_fclose (
 const char * table)
{
	DestroyTable (table);
}

int
abc_unlock (
 const char * table)
{
	/*TableState * state = LocateTable (table);*/

	/*_LockFreeAll (&state -> locks);*/
	return 0;
}

SQLRETURN
abc_selfield (

 const char * table,
 const char * new_index)
{
	TableState * state = LocateTable (table);

	IdentifyIndex (&g_misc_hdbc, &g_hstmt, state, new_index);

	return 0;
}

/*
 *	Updates
 */
int
abc_update (
 const char * table,
 void * recbuf)
{
	return (_UpdateRow (LocateTable (table), recbuf));
}

int
abc_add (
 const char * table,
 void * recbuf)
{
	return(_InsertRow (LocateTable (table), recbuf));
}

int
abc_delete (
 const char * table)
{
	return(_DeleteRow (LocateTable (table)));
}

int
find_hash (
 const char * table,
 void * buffer,
 int mode,
 const char * lock,
 long hash)
{
	int i;
	TableState * state = LocateTable (table);
	ColumnDef * indexcol = state -> columns + state -> indexactual [0];

	if (state -> indexc != 1 ||
		!(	indexcol -> type == DB2_INTEGER))
	{
		printf ("Non-hash index");
	}

	/*
	 *	Prime the buffer with the hash value
	 *
	 *	We've got no choice but to scan the view for a matching name
	 */
	for (i = 0; i < state -> viewc; i++)
	{
		if (!strcmp (indexcol -> name, state -> view [i].vwname))
		{
			*(long *) ((char *) buffer + state -> view [i].vwstart) = hash;
			break;
		}
	}
	if (i >= state -> viewc)
		printf ("Index %s not in dbview", indexcol -> name);

	return find_rec (table, buffer, mode, lock);
	return 0;
}

int
TableColumnCount (
 const char * name)
{
	return _TableColumnCount(name, &g_hstmt);
}


void
TableColumnGet (
 const char * name,
 int colno,
 void * buffer)
{
	_TableColumnGet(name, colno, buffer);
}

void
TableColumnInfo (
 const char * name,
 int colno,
 struct ColumnInfo * buffer)
{
	_TableColumnInfo(name, colno, buffer, &g_misc_hdbc, &g_hstmt);
}

/*
int FetchAbsolute(TableState * state, void * buffer, int position)
{
	int i = 0;

	while (QueryFetch (state, buffer, SQL_FETCH_NEXT))
	{
		for (i = 0; i < table -> indexc; i++)
		{
			ColumnDef * column = &state -> columns [state -> indexactual [i]];
			if (position == GTEQ)
			{
				
			}
		}
	}

	return FALSE;
}
*/

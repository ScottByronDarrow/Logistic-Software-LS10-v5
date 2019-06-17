#ident  "$Id: samputil.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *  Miscellaneous DB2 Utilities
 *
 *    - contains various sample functions, used by most other samples:
 *       - check_error
 *       - DBconnect
 *       - prompted_connect
 *       - print_connect_info
 *       - print_error
 *       - print_results
 *       - check_error
 *       - terminate
 *
*******************************************************************************
*  $Log: samputil.c,v $
*  Revision 5.0  2001/06/19 07:08:20  cha
*  LS10-5.0 New Release as of 19 JUNE 2001
*
*  Revision 4.0  2001/03/09 02:27:56  scott
*  LS10-4.0 New Release as at 10th March 2001
*
*  Revision 1.3  2000/09/25 09:48:46  gerry
*  DB2 Release 2 - After major fixes
*
*  
*/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlcli1.h>
#include "samputil.h"
#include "log.h"
#define MAXCOLS 255

#ifndef max
	#define max(a,b) (a > b ? a : b)
#endif

/*
 * Global Variables for user id and password, defined in main module.
 * To keep samples simple, not a recommended practice.
 * The INIT_UID_PWD macro is used to initialize these variables.
 */
SQLCHAR server  [SQL_MAX_DSN_LENGTH + 1];
SQLCHAR uid  [MAX_UID_LENGTH + 1];
SQLCHAR pwd  [MAX_PWD_LENGTH + 1];


extern char *PNAME;

/*
 * check_error - calls print_error (), checks severity of return code
 */

SQLRETURN 
check_error (
 SQLSMALLINT htype, /* A handle type identifier */
 SQLHANDLE   hndl,  /* A handle */
 SQLRETURN   frc,   /* Return code to be included with error msg  */
 int         line,  /* Used for output message, indcate where     */
 char *      file)  /* the error was reported from  */
{

    print_error  (htype, hndl, frc, line, file);

    switch (frc) 
	{

   	 case SQL_SUCCESS:
        break;

     case SQL_INVALID_HANDLE:
        /*printf ("\n>------ ERROR Invalid Handle --------------------------\n");*/
		 LOG("log.db2","\n>------ ERROR Invalid Handle --------------------------\n");

     case SQL_ERROR:
        /*printf ("\n>--- FATAL ERROR, Attempting to rollback transaction --\n");*/
		 LOG("log.db2","\n>--- FATAL ERROR, Attempting to rollback transaction --\n");

        if (SQLEndTran  (htype, hndl, SQL_ROLLBACK) != SQL_SUCCESS)
           /*printf (">Rollback Failed, Exiting application\n");*/
		   LOG("log.db2",">Rollback Failed, Exiting application\n");
        else
           /*printf (">Rollback Successful, Exiting application\n");*/
		   LOG("log.db2",">Rollback Successful, Exiting application\n");
        return (terminate  (hndl, frc));

     case SQL_SUCCESS_WITH_INFO:
        /*printf ("\n> ----- Warning Message, application continuing ------- \n");*/
		LOG("log.db2","\n> ----- Warning Message, application continuing ------- \n");
        break;

     case SQL_NO_DATA_FOUND:
        /*printf ("\n> ----- No Data Found, application continuing --------- \n");*/
		 LOG("log.db2","\n> ----- No Data Found, application continuing --------- \n");
        break;

     default:
        /*printf ("\n> ----------- Invalid Return Code --------------------- \n");*/
		 LOG("log.db2","\n> ----------- Invalid Return Code --------------------- \n");
        /*printf ("> --------- Attempting to rollback transaction ---------- \n");*/
		 LOG("log.db2","> --------- Attempting to rollback transaction ---------- \n");
        if (SQLEndTran (htype, hndl, SQL_ROLLBACK) != SQL_SUCCESS)
           /*printf (">Rollback Failed, Exiting application\n");*/
		   LOG("log.db2",">Rollback Failed, Exiting application\n");
        else
           /*printf (">Rollback Successful, Exiting application\n");*/
		   LOG("log.db2", ">Rollback Successful, Exiting application\n");
        return (terminate (hndl, frc));
    }

    return (frc);

}

/*
 * Connect without prompt 
 */
SQLRETURN
DBconnect  (
 SQLHANDLE henv,
 SQLHANDLE * hdbc)
{

    /*
	 * allocate a connection handle 
	 */
    if (SQLAllocHandle (SQL_HANDLE_DBC, henv, hdbc) != SQL_SUCCESS)
	{
        printf (">---ERROR while allocating a connection handle-----\n");
        return (SQL_ERROR);
    }

    /*
	 * Set AUTOCOMMIT OFF 
	 */
    if (SQLSetConnectAttr (* hdbc,
						   SQL_ATTR_AUTOCOMMIT,
                           (void *) SQL_AUTOCOMMIT_OFF,
						   SQL_NTS
                          ) != SQL_SUCCESS)
	{
        printf (">---ERROR while setting AUTOCOMMIT OFF ------------\n");
        return (SQL_ERROR);
    }

    if (SQLConnect (* hdbc,
                     server, SQL_NTS,
                     uid,    SQL_NTS,
                     pwd,    SQL_NTS
                   ) != SQL_SUCCESS)
	{
        printf (">--- Error while connecting to database: %s -------\n",
                server
               );
        SQLDisconnect (* hdbc);
        SQLFreeHandle (SQL_HANDLE_DBC, * hdbc);
        return (SQL_ERROR);
    }
    else      /* Print Connection Information */
        printf (">Connected to %s\n", server);

    return  (SQL_SUCCESS);

}

/*
 * print connection information 
 */
SQLRETURN
print_connect_info  (
 SQLHANDLE hdbc)
{

    SQLCHAR     buffer  [255];
    SQLSMALLINT outlen;
    SQLRETURN   rc;

    printf ("-------------------------------------------\n");

    rc = SQLGetInfo (hdbc, SQL_DATA_SOURCE_NAME, buffer, 255, &outlen);
    CHECK_HANDLE (SQL_HANDLE_DBC, hdbc, rc);
    printf ("Connected to Server: %s\n", buffer);

    rc = SQLGetInfo  (hdbc, SQL_DATABASE_NAME, buffer, 255, &outlen);
    CHECK_HANDLE (SQL_HANDLE_DBC, hdbc, rc);
    printf (" Database Name: %s\n", buffer);

    rc = SQLGetInfo (hdbc, SQL_SERVER_NAME, buffer, 255, &outlen);
    CHECK_HANDLE (SQL_HANDLE_DBC, hdbc, rc);
    printf (" Instance Name: %s\n", buffer);

    rc = SQLGetInfo (hdbc, SQL_DBMS_NAME, buffer, 255, &outlen);
    CHECK_HANDLE (SQL_HANDLE_DBC, hdbc, rc);
    printf ("     DBMS Name: %s\n", buffer);

    rc = SQLGetInfo (hdbc, SQL_DBMS_VER, buffer, 255, &outlen);
    CHECK_HANDLE (SQL_HANDLE_DBC, hdbc, rc);
    printf ("  DBMS Version: %s\n", buffer);

    printf ("-------------------------------------------\n");

    return (rc);

}

/*
 * --> SQLL1X32.SCRIPT 
 */

/*
 * print_error - calls SQLGetDiagRec (), displays SQLSTATE and message
 *             - called by check_error                               
 */

SQLRETURN
print_error (
 SQLSMALLINT htype, /* A handle type identifier */
 SQLHANDLE   hndl,  /* A handle */
 SQLRETURN   frc,   /* Return code to be included with error msg  */
 int	     line, 	/* Used for output message, indcate where     */
 char *      file)  /* the error was reported from  */
{

    SQLCHAR     buffer [SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR     sqlstate [SQL_SQLSTATE_SIZE + 1];
    SQLINTEGER  sqlcode;
    SQLSMALLINT length, i;

    /*printf (">--- ERROR -- RC = %d Reported from %s, line %d ------------\n",
            frc,
            file,
            line
           );*/
	LOG("log.db2",">--- ERROR -- RC = %d Reported from %s, line %d ------------\n",
            frc,
            file,
            line
			);

    i = 1;
    while  (SQLGetDiagRec (htype,
                           hndl,
                           i,
                           sqlstate,
                           &sqlcode,
                           buffer,
                           SQL_MAX_MESSAGE_LENGTH + 1,
                           &length
                        ) == SQL_SUCCESS) 
	{
       /*printf ("         SQLSTATE: %s\n", sqlstate);*/
	   LOG("log.db2","         SQLSTATE: %s\n", sqlstate);
       /*printf ("Native Error Code: %ld\n", sqlcode);*/
	   LOG("log.db2","Native Error Code: %ld\n", sqlcode);
       /*printf ("%s \n", buffer);*/
	   LOG("log.db2","%s \n", buffer);
       i++;
    }

    /*printf (">--------------------------------------------------\n");*/
	LOG("log.db2",">--------------------------------------------------\n");

    return (SQL_ERROR);

}
/*<-- */

/*--> SQLL1X10.SCRIPT */

SQLRETURN 
print_results (
 SQLHANDLE hstmt)
{

    SQLCHAR     colname [32];
    SQLSMALLINT coltype;
    SQLSMALLINT colnamelen;
    /*SQLSMALLINT nullable;*/
    SQLUINTEGER collen [MAXCOLS];
    SQLSMALLINT scale;
    SQLINTEGER  outlen [MAXCOLS];
    SQLCHAR *   data [MAXCOLS];
    SQLCHAR     errmsg [256];
    SQLRETURN   rc;
    SQLSMALLINT nresultcols, i;
    SQLINTEGER  displaysize;

    rc = SQLNumResultCols (hstmt, &nresultcols);
    CHECK_HANDLE (SQL_HANDLE_STMT, hstmt, rc);
    if (rc != SQL_SUCCESS)
    {
        return (rc);
    }

    for  (i = 0; i < nresultcols; i++) 
	{
        SQLDescribeCol (hstmt,
                        (SQLSMALLINT) (i + 1),
                        colname,
                        sizeof (colname),
                        &colnamelen,
                        &coltype,
                        &collen [i],
                        &scale,
                        NULL
    	               );

        /*
		 * get display length for column 
		 */
        SQLColAttribute (hstmt,
                         (SQLSMALLINT) (i + 1),
                         SQL_DESC_DISPLAY_SIZE,
                         NULL,
                         0,
                         NULL,
                         &displaysize
                        );

        /*
         * Set column length to max of display length,
         * and column name length. Plus one byte for
         * null terminator.
         */
        collen [i] = max (displaysize, (strlen ((char *) colname))) + 1;

        printf ("%-*.*s",
                (int) collen [i], (int) collen [i],
                colname
               );

        /*
		 * allocate memory to bind column 
		 */
        data [i] =  (SQLCHAR *) malloc ((int) collen [i]);

        /*
		 * bind columns to program vars, converting all types to CHAR
		 */
        SQLBindCol (hstmt,
                    (SQLSMALLINT) (i + 1),
                    SQL_C_CHAR,
                    data [i],
                    collen [i],
                    &outlen [i]
                   );
    }

    printf ("\n");

    /* 
	 * display result rows 
	 */
    rc = SQLFetch (hstmt);
    while ((rc == SQL_SUCCESS) ||  (rc == SQL_SUCCESS_WITH_INFO)) 
	{
        errmsg [0] = '\0';
        for (i = 0; i < nresultcols; i++) 
		{
            /*
			 * Check for NULL data 
			 */
            if (outlen [i] == SQL_NULL_DATA)
               printf ("%-*.*s",
                       (int) collen [i],
                       (int) collen [i],
                       "NULL"
               	      );
            else
			{
				/* 
				 * Build a truncation message for any columns truncated 
				 */
               	if  (outlen [i] >= collen [i])
				{
                  sprintf ((char *) errmsg + strlen ((char *) errmsg),
                           "%d chars truncated, col %d\n",
                           (int) outlen [i] - collen [i] + 1,
                           i + 1
                          );
                }
               	/* 
				 * Print column 
				 */
               	printf ("%-*.*s",
                        (int) collen [i],
                        (int) collen [i],
                        data [i]
                       );
            }
        }                          /* for all columns in this row  */

        printf ("\n%s", errmsg); /* print any truncation messages */
        rc = SQLFetch (hstmt);    	
    }                              /* while rows to fetch */

    if (rc != SQL_NO_DATA_FOUND)
    {
        CHECK_HANDLE (SQL_HANDLE_STMT, hstmt, rc);
    }   


    /* free data buffers */
    for (i = 0; i < nresultcols; i++) 
	{
        free (data [i]);
    }

    return (SQL_SUCCESS);

}                               /* end print_results */
/*<-- */

/* prompted_connect - prompt for connect options and connect */
#ifdef UNSURE

SQLRETURN 
prompted_connect (
 SQLHANDLE henv,
 SQLHANDLE * hdbc)
{

    /* allocate a connection handle     */
    if (SQLAllocHandle (SQL_HANDLE_DBC,
                        henv,
                        hdbc
                       ) != SQL_SUCCESS)
	{
        printf (">---ERROR while allocating a connection handle-----\n");
        return (SQL_ERROR);
    }

    /* 
	 * Set AUTOCOMMIT OFF 
	 */
    if  (SQLSetConnectAttr (* hdbc,
                            SQL_ATTR_AUTOCOMMIT,
                            (void *) SQL_AUTOCOMMIT_OFF, SQL_NTS
                           ) != SQL_SUCCESS)
	{
       printf (">---ERROR while setting AUTOCOMMIT OFF ------------\n");
       return (SQL_ERROR);
    }

    printf (">Enter Server Name:\n"); gets ((char *) server);
    printf (">Enter User Name:\n"); gets ((char *) uid);
    printf (">Enter Password:\n"); gets ((char *) pwd);

    if  (SQLConnect (* hdbc,
                     server, SQL_NTS,
                     uid,    SQL_NTS,
                     pwd,    SQL_NTS
	                ) != SQL_SUCCESS) {
        printf (">--- ERROR while connecting to %s -------------\n",
                server
               );
        
        SQLDisconnect (* hdbc);
        SQLFreeHandle (SQL_HANDLE_DBC, * hdbc);
        return (SQL_ERROR);
    }
    else
		/*
		 * Print Connection Information 
		 */
        printf ("Successful Connect to %s\n", server);

    return (SQL_SUCCESS);

}
#endif

/* 
 * terminate and free environment handle 
 */
SQLRETURN
terminate (
 SQLHANDLE henv,
 SQLRETURN rc)
{

    SQLRETURN lrc;

    printf (">Terminating ....\n");
    print_error (SQL_HANDLE_ENV,
                 henv,
                 rc,
                 __LINE__,
                 __FILE__
               	);

    /* 
	 * Free environment handle 
	 */
    if ((lrc = SQLFreeHandle (SQL_HANDLE_ENV, henv)) != SQL_SUCCESS)
       print_error (SQL_HANDLE_ENV,
                    henv,
                    lrc,
                    __LINE__,
                    __FILE__
                	);

    return (rc);

}

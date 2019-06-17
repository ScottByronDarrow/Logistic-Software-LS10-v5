/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: delete.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (delete.c)
|  Program Desc  : (Deletes to the database)
|---------------------------------------------------------------------|
| $Log: delete.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.3  2002/03/11 04:04:04  cha
| updated to use OCI_COMMIT_ON_SUCCES on OCIStmtExecute.
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:50  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 02:09:52  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.0  2000/07/15 07:33:50  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.1  1999/10/21 21:47:04  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdio.h>
#include	<string.h>
#include 	<stdlib.h>
#include	<dbio.h>

#include	"oracledbif.h"

/*-------------------+
| External variables |
+-------------------*/
extern 	OCIError	*errhp;

/*
 * Global variables
 */
static 	OCIBind		*bndp01 	= (OCIBind *) 0;

sword	status;
dvoid 	*tmp;

/*
 * Local functions 
 */
static void BuildStatement (TableState *);
static void BindInputVariables (TableState *);

/*
 * _DeleteRow 
 */
int
_DeleteRow (
	TableState * table)
{
	/*
	 * Delete the current row 
	 */

	char errbuf [1024];
	ub4 errcode;

	/*
	 * Create SQL statement and bind input variables 
	 */
	BuildStatement (table);
	BindInputVariables (table);

	/*
	 * Attempt the delete 
	 */
	if (OCIStmtExecute (svchp, 
						table -> d_stmt, 
						errhp, 
						(ub4) 1, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_COMMIT_ON_SUCCESS))
	{
		OCIErrorGet ((dvoid *) errhp,
					 (ub4) 1,
					 (text *) NULL,
					 &errcode,
					 errbuf,
					 (ub4) sizeof (errbuf),
					 (ub4) OCI_HTYPE_ERROR);
		printf ("CDA:::delete::OCIStmtExecute:DeleteStatement = %s", errbuf);
		return 1;
	}
        
	return 0;
}

/*
 * BuildStatement 
 */
static void
BuildStatement (
	TableState * table)
{
	/*
	 * Build a SQL delete statement 
	 */
	char sqlstr [2048];		/* hopefully be long enough */

	sprintf (sqlstr,
			 "delete from %s where rowid = :1",
			 table -> table ? table -> table : table -> named);

	if ( table -> delete )
    	free( table -> delete );

	table -> delete = strdup (sqlstr);

	/*
	 * Attempt the parse 
	 */

	/*
	 * Allocate and initialize d_stmt 
	 */
	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &table -> d_stmt,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare delete statement 
	 */
	if (OCIStmtPrepare (table -> d_stmt, 
						errhp, 
						(char *) table -> delete,
						(ub4) strlen (table -> delete), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "delete::OCIStmtPrepare:deletestmt");
}

/*
 * BindInputVariables 
 */
static void
BindInputVariables (
	TableState * table)
{
	/*
	 * Inform ORACLE about input parameters                         
	 * Tie the rowid column to the delete statement                 
	 * The input lengths for the ROWID, expects the input length     
	 *	to be the length of the incoming string result              
	 *	instead of the length of the data-storage buffer            
	 */
	int rowidcol = table -> columnc - 1;

	if (OCIBindByPos (table -> d_stmt,
					  &bndp01,
					  errhp,
					  1,
					  (dvoid *) table -> columns [rowidcol].data,
					  (sword) strlen (table -> columns [rowidcol].data),
					  _internal_sqltype (table -> columns [rowidcol].type),
					  (dvoid *) &table -> columns [rowidcol].null_ind,
					  (ub2 *) 0,
					  (ub2 *) 0,
					  0,
					  (ub4 *) 0,
					  OCI_DEFAULT))
		oraclecda_error (errhp, "delete::OCIBindByPos:BindInputVars");
}

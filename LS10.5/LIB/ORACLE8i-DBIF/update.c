/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: update.c,v 5.0 2002/05/08 01:30:09 scott Exp $
|  Program Name  : (update.c)
|  Program Desc  : (Updates to the database)
|---------------------------------------------------------------------|
| $Log: update.c,v $
| Revision 5.0  2002/05/08 01:30:09  scott
| CVS administration
|
| Revision 1.3  2002/03/11 04:04:33  cha
| updated to use OCI_COMMIT_ON_SUCCES on OCIStmtExecute.
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:52  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/04/06 02:09:53  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.0  2000/07/15 07:33:51  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.1  1999/10/21 21:47:05  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<dbio.h>

#include	"oracledbif.h"

/*
 * External variables 
 */
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

static void BuildStatement 		(TableState *);
static void BindInputVariables 	(TableState *);

/*
 * _UpdateRow 
 */
int
_UpdateRow (
	TableState * table,
	void * appbuf)
{
	/*
	 * Update the current row 
	 */
	int i;
	char errbuf [1024];
	ub4 errcode;

	/*
	 * Create SQL statement and bind input variables 
	 */
	BuildStatement (table);
	BindInputVariables (table);

	/*
	 * Update internal buffer from application buffer 
	 */
	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

		column -> null_ind = 0;
		_ConvApp2Raw (column,
					  table -> view [i].vwstart,
					  appbuf);
	}
	/*
	 * Attempt the update 
	 */
	if (OCIStmtExecute (svchp, 
						table -> u_stmt, 
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
		printf ("CDA:::update::OCIStmtExecute:UpdateStatement = %s", errbuf);
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
	 * Build a SQL update statement 
	 */
	int i, end = table -> columnc - 1;
	char sqlstr [4096];		/* hopefully be long enough */

	sprintf (sqlstr,
			 "update %s set ",
			 table -> table ? table -> table : table -> named);
		
	/*
	 * Put in all column names except for the rowid
	 * (which should be the last column)          
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
	 * Use the rowid as our conditional 
	 */
	sprintf (sqlstr + strlen (sqlstr),
			 "where rowid = :%d",
			 table -> columnc);

	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 * Best to die from possible memory corruption 
		 */
		oracledbif_error ("BuildStatement. SQL update too big");
	}

	table -> update = strdup (sqlstr);

	/*
	 * Attempt the parse 
	 *
	 * Allocate and initialize u_stmt 
	 */

	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &table -> u_stmt,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare update statement 
	 */
	if (OCIStmtPrepare (table -> u_stmt, 
						errhp, 
						(char *) table -> update,
						(ub4) strlen (table -> update), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "update::OCIStmtPrepare:updatestmt");		
}

/*
 * BuildStatement 
 */
static void
BindInputVariables (
	TableState * table)
{
	/*
	 * Inform ORACLE about input parameters        
	 * Tie all the columns to the update statement 
	 */
	int i;

	for (i = 0; i < table -> columnc; i++)
	{
		/*
		 * The exception to input lengths would be that for 
		 * the ROWID, which expects the input length       
		 * to be the length of the incoming string result  
		 * instead of the length of the data-storage buffer
		 */
		int length = table -> columns [i].type == OT_RowId ?
					 strlen (table -> columns [i].data) :
					 table -> columns [i].length;

		if (OCIBindByPos (table -> u_stmt,
						  &bndp01,
						  errhp,
						  i + 1,
						  (dvoid *) table -> columns [i].data,
						  (sword) length,
						  _internal_sqltype (table -> columns [i].type),
						  (dvoid *) &table -> columns [i].null_ind,
						  (ub2 *) 0,
						  (ub2 *) 0,
						  0,
						  (ub4 *) 0,
						  OCI_DEFAULT))
			oraclecda_error (errhp, "update::OCIBindByPos:BindInputVars");
	}
}

/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: insert.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (insert.c)
|  Program Desc  : (Inserts to the database)
|---------------------------------------------------------------------|
| $Log: insert.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.4  2002/03/11 11:08:42  cha
| Added files and code checked.
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:51  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.1  2001/06/21 08:01:01  cha
| Updated to handle correctly the Money datatype.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 02:09:53  cha
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
#include 	<stdlib.h>
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
static void BuildStatement (TableState *);
static void BindInputVariables (TableState *);

/*
 * _InsertRow 
 */
int
_InsertRow (
	TableState * table,
	void * appbuf)
{	
	/*
	 * Insert the current row 
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
	 * Insert internal buffer from application buffer              
	 * All columns not represented by the view will be set to NULL 
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		table -> columns [i].null_ind = -1;
	}

	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

		column -> null_ind = 0;
		_ConvApp2Raw (column,
					  table -> view [i].vwstart,
					  appbuf);
	}

	/*
	 * Attempt the insert 
	 */

	if (OCIStmtExecute (svchp, 
						table -> i_stmt, 
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
		printf ("CDA:::insert::OCIStmtExecute:InsertStatement = %s", errbuf);
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
	 * Build a SQL insert statement 
	 */
	int i, end = table -> columnc - 1;
	char sqlstr [4096];		/* hopefully be long enough */

	sprintf (sqlstr,
			 "insert into %s (",
			 table -> table ? table -> table : table -> named);

	/*
	 * Put in all column names except for the rowid 
	 * (which should be the last column)           
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
		 * Serial fields use a sequence with the field's name 
		 * to get the next value.                            
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

	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 * Best to die from possible memory corruption 
		 */
		oracledbif_error ("BuildStatement. SQL insert too big");
	}

	if (table -> insert)
		free (table-> insert);
          
	table -> insert = strdup (sqlstr);

	/*
	 * Attempt the parse 
	 *
	 * Allocate and initialize i_stmt 
	 */
	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &table -> i_stmt,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare insert statement 
	 */
	if (OCIStmtPrepare (table -> i_stmt, 
						errhp, 
						(char *) table -> insert,
						(ub4) strlen (table -> insert), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "insert::OCIStmtPrepare:insertstmt");
}

/*
 * BindInputVariables 
 */
static void
BindInputVariables (
	TableState * table)
{
	int ctr = 0;
	/*
	 * Inform ORACLE about input parameters                          
	 * Tie all the columns (except the last one, which is the rowid) 
	 * 	to the insert statement                                     
	 */
	int i;
	int end = table -> columnc - 1;

	for (i = 0; i < end; i++)
	{
		if (table -> columns [i].type == OT_Serial)
		{
			/*
			 * Skip serial fields as the SQL insert input field 
			 * has a sequence value attached to it instead     
			 */
			continue;
		}

		if (OCIBindByPos (table -> i_stmt,
						  &bndp01,
						  errhp,
						  ctr+ 1,
						  (dvoid *) table -> columns [i].data,
						  (sword) table -> columns [i].length,
						  _internal_sqltype (table -> columns [i].type),
						  (dvoid *) &table -> columns [i].null_ind,
						  (ub2 *) 0,
						  (ub2 *) 0,
						  0,
						  (ub4 *) 0,
						  OCI_DEFAULT))
			oraclecda_error (errhp, "insert::OCIBindByPos:BindInputVars");
		ctr++;
	}
}

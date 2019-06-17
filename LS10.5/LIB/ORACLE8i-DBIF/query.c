/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: query.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (query.c)
|  Program Desc  : (Query related functions)
|---------------------------------------------------------------------|
| $Log: query.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:52  kaarlo
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
| Revision 2.3  2000/08/02 02:34:59  raymund
| Small performance improvements. Added codes for locked find_hash()es.
|
| Revision 2.2  2000/07/28 06:10:08  raymund
| Implemented CURRENT in find_rec. Provided a patch for bug in disp_srch().
|
| Revision 2.1  2000/07/26 10:09:56  raymund
| Furnished missing functionalities. Use SQL for row locking.
|
| Revision 2.0  2000/07/15 07:33:51  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.3  1999/11/15 02:53:06  jonc
| Added lock code. Requires `alvin' the lock-daemon to be running.
|
| Revision 1.2  1999/11/01 21:22:25  jonc
| Fixed: sorted output.
| Added: support for FIRST
|
| Revision 1.1  1999/10/21 21:47:05  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <signal.h>
#include    <unistd.h>
#include	<dbio.h>

#include	"oracledbif.h"
#include	"oraerrs.h"

/*
 * External variables 
 */
extern 	OCIError	*errhp;

/*
 * Global variables 
 */
static	OCIDefine	*defnp01 	= (OCIDefine *) 0;
static 	OCIBind		*bndp01 	= (OCIBind *) 0;

sword	status;
dvoid 	*tmp;

/*
 * Local functions 
 */
static void BuildStatement 		(TableState *, int, int);
static void BindInputVariables 	(TableState *);
static void BindOutputVariables (TableState *);
static const char *ModeToString (int, int);

/*
 * QuerySetup 
 */
int
QuerySetup (
	TableState * table,
	int mode,
	int abs_posn,
	void * buffer)
{
	int i;

	if (mode != table -> lastfetchmode || !table -> query)
	{
		/*
		 * Build statement and bind input/output 
		 */
		BuildStatement (table, mode, abs_posn);
		if (!abs_posn)
			BindInputVariables (table);
		BindOutputVariables (table);
	}

	/*
	 * Convert the index values from the application buffer 
	 * 	to the internal buffer							   
	 */
	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = table -> columns + table -> indexactual [i];

		column -> null_ind = 0;
		_ConvApp2Raw (column,
					  table -> view [table -> indexview [i]].vwstart,
					  buffer);
	}

	/*
	 * Attempt execution 
	 */
	if (OCIStmtExecute (svchp, 
						table -> q_stmt, 
						errhp, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "query::OCIStmtExecute:QueryStatement");
		
	table -> lastfetchmode = mode;
	return TRUE;
}

/*
 * BuildStatement 
 */
static void
BuildStatement (
	TableState * table,
	int mode,
	int abs_posn)
{
	/*
	 * Build a usable SQL statement 
	 */
	int i;
	char sqlstr [2048];		/* should be long enough, initially */

	/*
	 * As much as i'd like to do a: "select *, rowid from table"
	 * ORACLE's parser will not allow this, so I need to	
	 *	append all the name individually				
	 */
	strcpy (sqlstr, "select ");
	for (i = 0; i < table -> columnc; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
				 "%s%s ",
				 table -> columns [i].name,
				 i + 1 < table -> columnc ? "," : "");
	}
	sprintf (sqlstr + strlen (sqlstr),
			 "from %s",
			 table -> table ? table -> table : table -> named);

	if (table -> indexc)
	{
		/*
		 * Only build where conditions if an absolute position 
		 *	(eg FIRST/LAST) is not required					  
		 */
		if (!abs_posn)
		{
			/*
			 * Where conditions are based on the index selected 
			 */
			for (i = 0; i < table -> indexc; i++)
			{
				sprintf (sqlstr + strlen (sqlstr),
						 " %s %s %s :%d",
						 i ? "and" : "where",
						 table -> columns [table -> indexactual [i]].name,
						 ModeToString (mode, i + 1 < table -> indexc),
						 i + 1);
			}
		}

		/*
		 * Force output sort 
		 */
		strcat (sqlstr, " order by");
        
		if ((abs_posn == TRUE + 1) ||  /* Find Last? */
			(mode     == LT )      ||  /* Less than? */
			(mode     == LTEQ) )
			for (i = 0; i < table -> indexc; i++)
			{
				sprintf (sqlstr + strlen (sqlstr),
					 	 " %s desc %s",
					 	 table -> columns [table -> indexactual [i]].name,
					 	 i + 1 >= table -> indexc ? "" : ",");
		   	}
		else 
			for (i = 0; i < table -> indexc; i++)
			{
				sprintf (sqlstr + strlen (sqlstr),
						 " %s%s",
						 table -> columns [table -> indexactual [i]].name,
						 i + 1 >= table -> indexc ? "" : ",");
			}
	}

	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 * Best to die from possible memory corruption 
		 */
		oracledbif_error ("BuildStatement. SQL statement bigger than buffer");
	}

	if (table -> query)
		free (table -> query);
	table -> query = strdup (sqlstr);

	/*
	 * Attempt the parse 
	 *
	 * Allocate and initialize q_stmt 
	 */

	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &table -> q_stmt,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare query statement 
	 */
	if (OCIStmtPrepare (table -> q_stmt, 
						errhp, 
						(char *) table -> query,
						(ub4) strlen (table -> query), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "query::OCIStmtPrepare:querystmt");
}

/*
 * BindInputVariables 
 */
static void
BindInputVariables (
	TableState * table)
{
	/*
	 * Inform ORACLE about input parameters. 			   
	 * Our selection criteria is based around the index,	
	 * 	so we only bind columns associated with the index. 
	 */
	int i;

	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = &table -> columns [table -> indexactual [i]];
		int sql_input = _internal_sqltype (column -> type);

		if (OCIBindByPos (table -> q_stmt,
						  &bndp01,
						  errhp,
						  i + 1,
						  (dvoid *) column -> data,
						  (sword) column -> length,
						  sql_input,
						  (dvoid *) 0,
						  (ub2 *) 0,
						  (ub2 *) 0,
						  0,
						  (ub4 *) 0,
						  OCI_DEFAULT))
			oraclecda_error (errhp, "query::OCIBindByPos:BindInputVars");
	}
}

/*
 * BindOutputVariables 
 */
static void
BindOutputVariables (
	TableState * table)
{
	/*
	 * Inform ORACLE about output buffers for the query 
	 */
	int i;
	static ub2 r_len, 
			   r_code;		/* discard */

	for (i = 0; i < table -> columnc; i++)
	{
		if (OCIDefineByPos (table -> q_stmt,
							&defnp01,
							errhp,
							i + 1,
							(dvoid *) table -> columns [i].data,
							(sword) table -> columns [i].length,
							_internal_sqltype (table -> columns [i].type),
							(dvoid *) &table -> columns [i].null_ind,
							(ub2 *) &r_len,
							(ub2 *) &r_code,
							OCI_DEFAULT))
		oraclecda_error (errhp, "query::OCIDefineByPos:BindOuputVars");
	}
}

/*
 * QueryFetch 
 */
int
QueryFetch (
	TableState * table,
	void * buffer,
	char locktype)
{
	int i;
	char msg[200];

	status = OCIStmtFetch (table -> q_stmt,
						   errhp,
						   (ub4) 1,
						   (ub4) OCI_FETCH_NEXT,
						   (ub4) OCI_DEFAULT);

	if (status == OCI_NO_DATA)
	{
		return FALSE;
	}
	
	if (status == OCI_ERROR)
	{
		sprintf(msg, "query::OCIStmtExecute:(%s) (%s)", table->named, table->table );
		oraclecda_error (errhp, msg);						   
	}

	/*
	 * Map output to application buffers 
	 */

	for (i = 0; i < table -> viewc; i++)
	{
		_ConvRaw2App (
			table -> columns + table -> viewactual [i],
			table -> view [i].vwstart,
			buffer );
	}

	if (locktype == 'w' || locktype == 'u')
		return _TryLock (locktype, table, &table->locks);

	return TRUE;
}

/*
 * QueryFetchPrevious 
 */
int
QueryFetchPrevious (
	TableState * table,
	void * buffer,
	int record_num, 
	char locktype)
{
	int current_record = 0,
		i;

	if (record_num <= 0)
		return FALSE;

	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = table -> columns + table -> indexactual [i];

		column -> null_ind = 0;
		
		_ConvApp2Raw (column,
					  table -> view [table -> indexview [i]].vwstart,
					  buffer);
	}
	
	if (OCIStmtExecute (svchp, 
						table -> q_stmt, 
						errhp, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "query::OCIStmtExecute:QueryFetchPrevious");

	while (current_record < record_num)
	{ 
		
		status = OCIStmtFetch (table -> q_stmt,
							   errhp,
							  (ub4) 1,
							  (ub4) OCI_FETCH_NEXT,
							  (ub4) OCI_DEFAULT);

		if (status == OCI_NO_DATA)
		{
			return FALSE;
		}
	
		if (status == OCI_ERROR)
		{
			oraclecda_error (errhp, "query::OCIStmtFetch:QueryFetchPrevious");
		}
	
		current_record++;
	} 

	for (i = 0; i < table -> viewc; i++)
	{
		_ConvRaw2App (table -> columns + table -> viewactual [i],
					  table -> view [i].vwstart,
					  buffer);
	}

	if (current_record == record_num)
	{
		if (locktype == 'w' || locktype == 'u')
			return _TryLock (locktype, table, &table->locks);
		return TRUE;
	}
   
	return FALSE; /* Failure */
}

/*
 * ModeToString 
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
		case 	EQUAL:
		case 	COMPARISON:
				return Eq;
		case 	GTEQ:
				return GtEq;
		case 	GREATER:
				return islastcol ? Gt : GtEq;
		case 	LT:
				return islastcol ? Lt : LtEq;
		case 	LTEQ:
				return LtEq;
	}

	oracledbif_error ("Bad comparison mode: %d", mode);
	return NULL;
}

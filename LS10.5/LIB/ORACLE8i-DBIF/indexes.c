/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: indexes.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (indexes.c)
|  Program Desc  : (Extract index info from system catalogs)
|---------------------------------------------------------------------|
| $Log: indexes.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:51  kaarlo
| Initial check-in for ORACLE8i porting.
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
| Revision 1.4  2000/06/30 01:12:40  gerry
| Temporary patch to remove cores
|
| Revision 1.3  1999/11/01 21:23:15  jonc
| Added support for no index.
|
| Revision 1.2  1999/10/31 22:42:29  jonc
| Fixed: occasional weird index info.
|
| Revision 1.1  1999/10/21 21:47:04  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<stdio.h>
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
static	OCIStmt		*stmthp01, 
					*stmthp02,
					*stmthp03;

static	OCIDefine	*defnp01 	= (OCIDefine *) 0,
					*defnp02 	= (OCIDefine *) 0;
static 	OCIBind		*bndp01 	= (OCIBind *) 0,
					*bndp02 	= (OCIBind *) 0;

sword	status;
dvoid 	*tmp;

static char index_name [128],
			table_name [128];
static char	col_name [128];
static int	col_count;

static ub2	r_len, r_code;

/*
 * Local functions 
 */
static void IndexSetup ();
static void cleanup ();

/*
 * IdentifyIndex 
 */
void
IdentifyIndex (
	TableState * table,
	const char * index)
{
	/*
	 * Indentify the index and associated columns 
	 */
	int i;

	IndexSetup ();

	/*
	 * Reallocation? 
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
	}

	if (!index)
		return;							/* table has no index */

	table -> indexname = strcpy (malloc (strlen (index) + 1), index);

	/*
	 * System catalogs uses uppercase names for tables and indexes 
	 */
	_strupper (strcpy (index_name, index));
	_strupper (
		strcpy (table_name, table -> table ? table -> table : table -> named));

	/*
	 * Determine column count 
	 */
	col_count = 0;						/* reset - just in case */
	
	/*
	 * Execute sql_indc 
	 */
	if (OCIStmtExecute (svchp, 
						stmthp01, 
						errhp, 
						(ub4) 1, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIStmtExecute:indc");
		
	if (!col_count)
		oracledbif_error ("No columns for %s?", table_name);

	/*
	 * Allocate buffers for index columns 
	 */
	table -> indexc = col_count;
	table -> indexactual = malloc (table -> indexc * sizeof (int));
	table -> indexview = malloc (table -> indexc * sizeof (int));

	memset (table -> indexactual, 0, table -> indexc * sizeof (int));
	memset (table -> indexview, 0, table -> indexc * sizeof (int));

	/*
	 * Get the column of the index and xref it against     
	 * 	our internal column definitions, and the user-view 
	 *
	 * Execute sql_inds 
	 */
	if (OCIStmtExecute (svchp, 
						stmthp02, 
						errhp, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIStmtExecute:inds");

	for (i = 0; 
		 OCIStmtFetch (stmthp02,
					   errhp,
					   (ub4) 1,
					   (ub4) OCI_FETCH_NEXT,
					   (ub4) OCI_DEFAULT) == OCI_SUCCESS;
		 i++)
	{
		int c;

		/*
		 * Match the name against internal definitions 
		 */
		_strlower (col_name);
		for (c = 0; c < table -> columnc; c++)
		{
			if (!strcmp (col_name, table -> columns [c].name))
			{
				/*
				 * Store position of ColumnDef for later use 
				 */
				table -> indexactual [i] = c;
				break;
			}
		}
		if (c >= table -> columnc)
			oracledbif_error ("Index column %s not found?", col_name);

		/*
		 * Match the name against the user-view 
		 */
		for (c = 0; c < table -> viewc; c++)
		{
			if (!strcmp (col_name, table -> view [c].vwname))
			{
				table -> indexview [i] = c;
				break;
			}
		}
		if (c >= table -> viewc)
			oracledbif_error ("Index column %s not found in dbview", col_name);
	}

	/*
	 * Force an SQL statement rebuild, if required 
	 */
	if (table -> query)
	{
		free (table -> query);
		table -> query = NULL;
	}
	
	cleanup ();
}

/*
 * IndexSetup 
 */
static void
IndexSetup ()
{
	static int setup_done = FALSE;

	/*
	 * This *should* return ordered by COLUMN_POSITION 
	 */

	char * sql_indc =
		"select count (*) "
		"from user_ind_columns "
		"where table_name = :1 and index_name = :2";
	char * sql_inds =
		"select column_name "
		"from user_ind_columns "
		"where table_name = :1 and index_name = :2 "
		"order by column_position";

	/*
	 * Try set up everytime - slower but could solve cores 
	 */

	/*
	 * Allocate and initialize stmthp01 
	 */
	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &stmthp01,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare sql_indc statement 
	 */
	if (OCIStmtPrepare (stmthp01, 
						errhp, 
						(char *) sql_indc,
						(ub4) strlen (sql_indc), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIStmtPrepare:indc");


	/*
	 * Bind the placeholder in the sql_indc statement 
	 */
	if (OCIBindByPos (stmthp01,
			  		  &bndp01,
			  		  errhp,
			  		  1,
			  		  (dvoid *) &table_name,
			  		  (sword) sizeof (table_name),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIBindByPos:indc");

	if (OCIBindByPos (stmthp01,
			  		  &bndp01,
			  		  errhp,
			  		  2,
			  		  (dvoid *) &index_name,
			  		  (sword) sizeof (index_name),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIBindByPos:indc");

	/*
	 * Result to col_count 
	 */
	if (OCIDefineByPos (stmthp01,
						&defnp01,
						errhp,
						1,
						(dvoid *) &col_count,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIDefineByPos:indc");

	/*
	 * Allocate and initialize stmthp02 
	 */
	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &stmthp02,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*
	 * Prepare sql_inds statement 
	 */
	if (OCIStmtPrepare (stmthp02, 
						errhp, 
						(char *) sql_inds,
						(ub4) strlen (sql_inds), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIStmtPrepare:inds");

	/*
	 * Bind the placeholder in the sql_inds statement 
	 */
	if (OCIBindByPos (stmthp02,
			  		  &bndp02,
			  		  errhp,
			  		  1,
			  		  (dvoid *) &table_name,
			  		  (sword) sizeof (table_name),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIBindByPos:inds");

	if (OCIBindByPos (stmthp02,
			  		  &bndp02,
			  		  errhp,
			  		  2,
			  		  (dvoid *) &index_name,
			  		  (sword) sizeof (index_name),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIBindByPos:inds");

	/*
	 * Result to col_name 
	 */
	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhp,
						1,
						(dvoid *) &col_name,
						(sword) sizeof (col_name),
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "indexes::OCIDefineByPos:inds");

	setup_done = TRUE;
}

/*
 * cleanup 
 */
void cleanup ()
{
	if (stmthp01)
		checkerr (errhp,
				  OCIHandleFree ((dvoid *) stmthp01,
								 OCI_HTYPE_STMT));
	if (stmthp02)
		checkerr (errhp,
				  OCIHandleFree ((dvoid *) stmthp02,
								 OCI_HTYPE_STMT));
	if (stmthp03)
		checkerr (errhp,
				  OCIHandleFree ((dvoid *) stmthp03,
								 OCI_HTYPE_STMT));
	return;
}

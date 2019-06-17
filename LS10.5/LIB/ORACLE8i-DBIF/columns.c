/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: columns.c,v 5.0 2002/05/08 01:30:07 scott Exp $
|  Program Name  : (columns.c)
|  Program Desc  : (Extract column info from system catalogs)
|---------------------------------------------------------------------|
| $Log: columns.c,v $
| Revision 5.0  2002/05/08 01:30:07  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:55  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:50  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.0  2001/06/19 07:10:27  cha
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
| Revision 1.3  2000/06/30 01:12:40  gerry
| Temporary patch to remove cores
|
| Revision 1.2  1999/10/28 01:58:35  jonc
| Added support for generic-catalog access. Damnably slow, though.
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

/*-------------------+
| External variables |
+-------------------*/

extern 	OCIError	*errhp;

/*
 * Global variables 
 */

static	OCIStmt		*stmthp01, 
					*stmthp02,
					*stmthp03;
					
static	OCIDefine	*defnp01 	= (OCIDefine *) 0,
					*defnp02 	= (OCIDefine *) 0,
					*defnp03 	= (OCIDefine *) 0;
static 	OCIBind		*bndp01 	= (OCIBind *) 0,
					*bndp02 	= (OCIBind *) 0,
					*bndp03 	= (OCIBind *) 0;

sword	status;
dvoid 	*tmp;

static	int 	col_count;
static 	char	col_table [128];

static 	char 	col_name [128],
				col_data_type [16];
static 	int		col_data_length, 
				col_data_precision, 
				col_data_scale;
static 	char	seq_name [128];

static 	sb2		i_data_type, 
				i_data_precision, 
				i_data_scale;		/* indicators */

static 	ub2		r_len, 
				r_code;				/* discard */

/*
 * Local functions 
 */
static void	ColumnSetup ();
static void	BuildColumnDefs (TableState * table);
static void BuildDataBuffer (TableState *);
static void MapViewToColumns (TableState *);
static void MapViewToUserData (TableState *);
static void cleanup ();

/*
 * IdentifyFields 
 */
void
IdentifyFields (
	TableState * table)
{
	/*
	 * We're gonna build the whole list for *all* the columns in      
	 * the table, and update the user-view for those columns involved 
	 */

	ColumnSetup ();

	/*
	 * The column query uses uppercase names for tables 
	 */
	_strupper (
		strcpy (col_table, table -> table ? table -> table : table -> named));

	/*
	 * Determine column count 
	 */
	col_count = 0;						/* reset - just in case */

	/*
	 * Execute sql_colc 
	 */
	if (OCIStmtExecute (svchp, 
						stmthp01, 
						errhp, 
						(ub4) 1, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtExecute:colc");
	
	if (!col_count)
		oracledbif_error ("No columns for %s?", col_table);

	/*
	 * Allocate buffers for table-columns add an extra definition for the rowid
	 */
	table -> columnc = col_count + 1;	/* add 1 for rowid */
	table -> columns = malloc (table -> columnc * sizeof (ColumnDef));
	memset (table -> columns, 0, table -> columnc * sizeof (ColumnDef));

	BuildColumnDefs (table);
	BuildDataBuffer (table);

	/*
	 * Set up the userview structures 
	 */
	MapViewToColumns (table);
	MapViewToUserData (table);
	
	cleanup ();
}

/*
 * ColumnSetup
 */
static void
ColumnSetup ()
{
	/*
	 * Set up the cursors against user_table_columns 
	 * I suspect that the sql-statements need to    
	 * permanant until the actual parse is done.   
	 */
	char * sql_colc =
		"select count (*) from user_tab_columns "
		"where table_name = :1 ";
	char * sql_cols =
		"select "
			"column_name, data_type, data_length, "
			"NVL (data_precision, NULL), NVL (data_scale, NULL) "
		"from user_tab_columns "
		"where table_name = :1 "
		"order by column_id";

	/*
	 * Identify "serial" fields by looking   
	 * at sequences which have matching names
	 */
	char * sql_seqs =
		"select "
			"sequence_name "
			"from user_sequences "
			"where sequence_name in "
			"("
				"select column_name "
				"from user_tab_columns "
				"where table_name = :1)";

	static int col_setup = FALSE;

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
	 * Prepare sql_colc statement 
	 */
	if (OCIStmtPrepare (stmthp01, 
						errhp, 
						(char *) sql_colc,
						(ub4) strlen (sql_colc), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtPrepare:colc");

	/*
	 * Bind the placeholder in the sql_colc statement 
	 */
	if (OCIBindByPos (stmthp01,
			  		  &bndp01,
			  		  errhp,
			  		  1,
			  		  (dvoid *) &col_table,
			  		  (sword) sizeof (col_table),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIBindByPos:colc");

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
		oraclecda_error (errhp, "columns::OCIDefineByPos:colc");

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
	 * Prepare sql_cols statement 
	 */
	if (OCIStmtPrepare (stmthp02, 
						errhp, 
						(char *) sql_cols,
						(ub4) strlen (sql_cols), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtPrepare:cols");

	/*
	 * Bind the placeholder in the sql_cols statement 
	 */
	
	if (OCIBindByPos (stmthp02,
			  		  &bndp02,
			  		  errhp,
			  		  1,
			  		  (dvoid *) &col_table,
			  		  (sword) sizeof (col_table),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIBindByPos:cols");

	/*
	 * Result to col_name, col_data_type, col_data_length,
	 *	col_data_precision, col_data_scale                
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
		oraclecda_error (errhp, "columns::OCIDefineByPos:cols");

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhp,
						2,
						(dvoid *) &col_data_type,
						(sword) sizeof (col_data_type),
						SQLT_STR,
						(dvoid *) &i_data_type,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIDefineByPos:cols");

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhp,
						3,
						(dvoid *) &col_data_length,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
  		oraclecda_error (errhp, "columns::OCIDefineByPos:cols");

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhp,
						4,
						(dvoid *) &col_data_precision,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) &i_data_precision,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIDefineByPos:cols");

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhp,
						5,
						(dvoid *) &col_data_scale,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) &i_data_scale,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIDefineByPos:cols");

	/*
	 * Allocate and initialize stmthp03 
	 */
	checkerr (errhp, 
			  OCIHandleAlloc ((dvoid *) envhp, 
							  (dvoid **) &stmthp03,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));
							  
	/*
	 * Prepare sql_seqs statement 
	 */
	if (OCIStmtPrepare (stmthp03, 
						errhp, 
						(char *) sql_seqs,
						(ub4) strlen (sql_seqs), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtPrepare:seqs");

	/*
	 * Bind the placeholder in the sql_seqs statement 
	 */
	if (OCIBindByPos (stmthp03,
			  		  &bndp03,
			  		  errhp,
			  		  1,
			  		  (dvoid *) &col_table,
			  		  (sword) sizeof (col_table),
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIBindByPos:seqs");

	/*
	 * Result to seq_name 
	 */
	if (OCIDefineByPos (stmthp03,
						&defnp03,
						errhp,
						1,
						(dvoid *) &seq_name,
						(sword) sizeof (seq_name),
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIDefineByPos:seqs");

	col_setup = TRUE;
}

/*
 * BuildColumnDefs 
 */
static void
BuildColumnDefs (
	TableState * table)
{
	/*
	 * Decode ORACLE types into something we can use
	 */

	int i;

	/*
	 * Identify possible serial field 
	 */

	/*
	 * Execute sql_seqs 
	 */

   	if (OCIStmtExecute (svchp, 
						stmthp03, 
						errhp, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtExecute:seqs");
	
	if (OCIStmtFetch (stmthp03,
					  errhp,
					  (ub4) 1,
					  (ub4) OCI_FETCH_NEXT,
					  (ub4) OCI_DEFAULT))
		seq_name [0] = '\0';	/* just in case */
	else
		_strlower (seq_name);	/* since ORACLE returns in upper-case */
	
	/*
	 * Execute sql_cols 
	 */
    if (OCIStmtExecute (svchp, 
						stmthp02, 
						errhp, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhp, "columns::OCIStmtExecute:cols");

	
	for (i = 0; 
		 OCIStmtFetch (stmthp02,
					   errhp,
					   (ub4) 1,
					   (ub4) OCI_FETCH_NEXT,
					   (ub4) OCI_DEFAULT) == OCI_SUCCESS;
		 i++)
	{
			
		_strlower (strcpy (table -> columns [i].name, col_name));

		_DecodeORAType (table -> columns [i].name, 
						seq_name,
						col_data_type, 
						col_data_length, 
						col_data_precision, 
						col_data_scale,
						i_data_precision, 
						i_data_scale,
						&table -> columns [i].type,
						&table -> columns [i].length);
						
		memset (col_name, '\0', sizeof (col_name));
		memset (col_data_type, '\0', sizeof (col_data_type));
	}

	/*
	 * The last column is for the rowid. We build it up manually          
	 */
	strcpy (table -> columns [i].name, "rowid");
	table -> columns [i].type = OT_RowId;
	table -> columns [i].length = _internal_size (OT_RowId);
}

/*
 * BuildDataBuffer 
 */
static void
BuildDataBuffer (
 	TableState * table)
{
	/*
	 * Allocate row-buffer and set up the data-pointers for each column
	 */
	int i;
	int sz = 0;

	/*
	 * Initial pass to determine the size 
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		sz += table -> columns [i].length;
	}

	table -> datasz = sz;

	memset (table -> data = malloc (sz), 0, sz);

	/*
	 * Second pass sets the data-pointers 
	 */

	for (sz = i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		table -> columns [i].data = table -> data + sz;

		sz += table -> columns [i].length;
	}
}

/*
 * MapViewToColumns 
 */
static void
MapViewToColumns (
 	TableState * table)
{
	/*
	 * Map the view references to actual ColumnDefs 
	 */
	int i;

	table -> viewactual = malloc (table -> viewc * sizeof (int));
	memset (table -> viewactual, 0, table -> viewc * sizeof (int));

	for (i = 0; i < table -> viewc; i++)
	{
		int c;

		for (c = 0; c < table -> columnc; c++)
			if (!strcmp (table -> view [i].vwname, table -> columns [c].name))
			{
				/*
				 * Store position of ColumnDefinition for later use 
				 */
				table -> viewactual [i] = c;
				break;
			}

		if (c >= table -> columnc)
			oracledbif_error ("Column %s not found",
							  table -> view [i].vwname);
	}
}

/*
 * MapViewToUserData 
 */
static void
MapViewToUserData (
 	TableState * table)
{
	/*
	 * Work out the offsets into the user application data-buffer 
	 */
	int i;
	int offset = 0;

	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

		offset += _application_align (offset, column -> type);
		
		table -> view [i].vwstart = offset;
		table -> view [i].vwtype = column -> type;

		table -> view [i].vwlen = column -> type == OT_Chars ?
			column -> length :					/* just happens to work out */
			_application_size (column -> type);

		offset += table -> view [i].vwlen;
	}
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

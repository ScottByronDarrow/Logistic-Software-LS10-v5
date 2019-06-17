/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: index.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (index.c)
|  Program Desc  : (index manipulation routines)
|---------------------------------------------------------------------|
| $Log: index.c,v $
| Revision 1.2  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|
| Revision 1.1  2002/07/17 07:24:28  cha
| Initial check in
|

=====================================================================*/

#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif 

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include "ids9dbif.h"

#define _IDS_MAXPARTS 16	/*Maximum columns can an index have*/

/*Local Variables*/
static MI_STATEMENT 	*_ids_stmt_desc_idx  = NULL;	/*use for sysindexes */
static MI_ROW 		*_ids_row_idx 	     = NULL;
static MI_ROW_DESC 	*_ids_rowdesc_idx    = NULL;

int tableid;
char index_name[128];
int parts [16];

/*input parameters for sysindexes*/
MI_DATUM 	idx_values[100];
mi_integer 	idx_lengths[100];
mi_integer 	idx_nulls[100];
mi_string 	*idx_types[100];


/*Local Functions*/
int IndexSetup(TableState *);


int
IdentifyIndex (
	TableState * table,
	const char * index)
{
	
	tableid = table -> tabid;
	sprintf (index_name, "%s", index);
	sprintf (table -> indexname, index);
	IndexSetup (table);

	return (EXIT_SUCCESS);	
}

int
IndexSetup (TableState * table)
{
	
	char temp[128],
	     colname[128];
	
	
	int num_cols = 0,
	    ctr      = 0,
	    indexctr = 0,
	    colctr   = 0;
	
	mi_integer 	loc_err, 
			collen = 0;
			
	mi_string 	*colval;    
	
	
	char *sql_idx  = "Select part1, part2, part3, part4, part5, part6, part7, part8, "
			 "part9, part10, part11, part12, part13, part14, part15, part16 "
			 "from sysindexes where idxname = ? and tabid = ?";
	

	/*Initialize parts columns*/
	for (ctr = 0; ctr < _IDS_MAXPARTS; ctr++)
	{
		parts[ctr] = 0;
	}

	/*Initialize input variables */
	idx_values[0] =  mi_string_to_lvarchar(index_name);
	idx_lengths[0] = 0;
	idx_nulls[0] = MI_FALSE;
	idx_types[0] = "char(128)";
	

	idx_values[1] = &tableid;
	idx_lengths[1] = 0;
	idx_nulls[1] = MI_FALSE;
	idx_types[1] = "integer";
	

	_ids_stmt_desc_idx = mi_prepare (_ids_conn, sql_idx , NULL);
	
	if (_ids_stmt_desc_idx == NULL)
	{
		ids_dbase_err2("Error in index:IndexSetup:mi_prepare for sysindexes");	
	}
	

	/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
	
	if ( mi_open_prepared_statement(_ids_stmt_desc_idx,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 2, 
					idx_values, idx_lengths, idx_nulls, idx_types, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in index:IndexSetup:mi_open_prepared_statement for sysindexes");	
	}


	if (mi_fetch_statement(_ids_stmt_desc_idx, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in index:IndexSetup:mi_fetch_statement in sysindexes");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in index:IndexSetup:mi_get_result for sysindexes");
	} 
	
	_ids_rowdesc_idx = mi_get_statement_row_desc(_ids_stmt_desc_idx);
	
	num_cols = mi_column_count(_ids_rowdesc_idx);
	
	_ids_row_idx = mi_next_row (_ids_conn, &loc_err);
	
	if (_ids_row_idx == NULL)
	{
		sprintf (temp, "Index [%s] not found ",index_name);
		//fprintf (stdout,"\ntable name [%s]\n\n\n\n",table -> table ? table -> table : table -> named);
		ids_dbase_err2(temp);
	}
	
	
	for (ctr = 0; ctr < num_cols; ctr++)
	{
		collen = 0;
		switch(mi_value(_ids_row_idx, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2(" index:IndexSetup:mi_value");
				return(MI_ERROR);
			case MI_NULL_VALUE:
				colval = "NULL";
				parts[ctr] = 0;
				break;
			case MI_NORMAL_VALUE:
				parts[ctr] = atoi(colval);
				break;
			default:
				ids_dbase_err2(" index:IndexSetup:mi_value");
				return(MI_ERROR);
		}
	}
	
	if (mi_drop_prepared_statement (_ids_stmt_desc_idx) == MI_ERROR)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_drop_prepared_statement for syscolumn");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_query_finish for syscolumn");	
	}
	
	/*Get the index count */
	/*index columns should be continous*/
	for (ctr = 0; ctr < _IDS_MAXPARTS; ctr ++)
	{
		if (parts[ctr] != 0)
		{
			indexctr++;
		}
	}
	

	table -> indexc = indexctr;
	table -> indexactual = malloc (table -> indexc * sizeof (int));
	table -> indexview = malloc (table -> indexc * sizeof (int));
	
	memset (table -> indexactual, 0, table -> indexc * sizeof (int));
	memset (table -> indexview, 0, table -> indexc * sizeof (int));

	
	for (ctr = 0; ctr < table -> indexc; ctr ++)
	{
		sprintf(colname,"%s", GetColNameByColNo (table, parts[ctr]));
		
		
		/*Map index to actual ColmnDefs*/
		for (colctr = 0; colctr < table -> columnc; colctr++)
		{
			if (!strcmp (colname, table -> columns [colctr].name))
			{
				/*
				 * Store position of ColumnDef for later use 
				 */
				table -> indexactual [ctr] = colctr;
				break;
			}/*if*/
		}/*for colname*/
		
		if (colctr >= table -> columnc)
		{
			sprintf (temp,"Index column %s not found?", colname);
			ids_dbase_err2 (temp);
		}
		
		
		/*Map index to dbview*/
		for (colctr = 0; colctr < table -> viewc; colctr++)
		{
			if (!strcmp (colname, table -> view [colctr].vwname))
			{
				/*
				 * Store position of ColumnDef for later use 
				 */
				table -> indexview [ctr] = colctr;
				break;
			}/*if*/
		}/*for colname*/
		
		if (colctr >= table -> viewc)
		{
			sprintf (temp,"Index column %s not found in dbview", colname);
			ids_dbase_err2 (temp);
		}
		
	}/*main for*/
	
	
	
	return (EXIT_SUCCESS);			 
}









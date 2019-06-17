/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: column.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (column.c)
|  Program Desc  : (column manipulation routines)
|---------------------------------------------------------------------|
| $Log: column.c,v $
| Revision 1.2  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|

=====================================================================*/

#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif 

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include "ids9dbif.h"


/*Local Variables*/
static MI_STATEMENT 	*_ids_stmt_desc_tab  = NULL;	/*use for systables */
static MI_STATEMENT 	*_ids_stmt_desc_colc = NULL;	/*use for column count*/
static MI_STATEMENT 	*_ids_stmt_desc_col  = NULL;	/*use for syscolumns*/

/*input parameters for syscolumns*/
MI_DATUM 	tab_values[100];
mi_integer 	tab_lengths[100];
mi_integer 	tab_nulls[100];
mi_string 	*tab_types[100];

char col_table [128];	/*use for table name*/
long table_id;		/*use for tableid from systables*/

int col_count;

/*Local Functions*/
//static void BuildDataBuffer (TableState * ); 	/*Allocate some memory space for each data per column*/

static void MapViewToColumns  (TableState *);	/*Store the actual position of the column in dbview */
static void MapViewToUserData (TableState *);	/*Fill dbview.vwtype, dbview.vwlen, dbview.vwstart */


int 
IdentifyFields (
	TableState * table)
{
	MI_ROW 		*ids_loc_row = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc = NULL;
	
	MI_ROW 		*ids_loc_row_colc = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_colc = NULL;
	
	MI_ROW 		*ids_loc_row_col = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_col = NULL;
	
	int 	ctr 	 = 0,
		ctr2	 = 0,
		num_cols = 0,
		col_no	 = 0, 
		col_type = 0,
		col_len	 = 0;
	
	char 	temp [128],
		col_name [128];
	
	mi_integer 	loc_err, 
			collen = 0;
			
	mi_string 	*colval;
	
	 
		
	
	char *sql_tab  = "Select tabid from systables where tabname = ?";
	
	char *sql_colc = "Select count(*) from syscolumns where tabid = ?";
	
	char *sql_col  = "Select  colname, colno, coltype, collength " 
			 "from syscolumns where tabid = ? order by colno";
	
	strcpy (col_table, table -> table ? table -> table : table -> named);
		
	tab_values[0] =  mi_string_to_lvarchar(col_table);
	tab_lengths[0] = 0;
	tab_nulls[0] = MI_FALSE;
	tab_types[0] = "char(128)";
	
	/*Prepare statement for systables*/
	
	_ids_stmt_desc_tab = mi_prepare (_ids_conn, sql_tab , NULL);
	
	if (_ids_stmt_desc_tab == NULL)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_prepare for systables");	
	}
	
	if ( mi_open_prepared_statement(_ids_stmt_desc_tab,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 1, 
					tab_values, tab_lengths, tab_nulls, tab_types, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_open_prepared_statement for systables");	
	}
	
	if (mi_fetch_statement(_ids_stmt_desc_tab, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in column:ColumnSetup:mi_fetch_statement in systables");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_get_result for systables");
	} 
	
	ids_loc_rowdesc = mi_get_statement_row_desc(_ids_stmt_desc_tab);
	
	num_cols = mi_column_count(ids_loc_rowdesc);
	
	ids_loc_row = mi_next_row (_ids_conn, &loc_err);
	
	if (ids_loc_row == NULL)
	{
		sprintf (temp, "Table [%s] not found ",col_table);
		ids_dbase_err2(temp);
	}
	
	for (ctr = 0; ctr < num_cols; ctr++)
	{
		collen = 0;
		switch(mi_value(ids_loc_row, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2("column:ColumnSetup:mi_value");
				return(MI_ERROR);
			case MI_NULL_VALUE:
				colval = "NULL";
				table_id = 0;
				break;
			case MI_NORMAL_VALUE:
			
				table_id = atol(colval);
				break;
			default:
				ids_dbase_err2("column:ColumnSetup:mi_value");
				return(MI_ERROR);
		}
	}
	
	/*Complete the execution*/
	
	if (mi_drop_prepared_statement (_ids_stmt_desc_tab) == MI_ERROR)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_drop_prepared_statement for systables");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_query_finish for systables");	
	}
	
	
	/*Prepare statement for column count*/
	
	tab_values[0] = &table_id;
	tab_lengths[0] = 0;
	tab_nulls[0] = MI_FALSE;
	tab_types[0] = "integer";
		
	_ids_stmt_desc_colc = mi_prepare (_ids_conn, sql_colc , NULL);
	
	
	
	if (_ids_stmt_desc_colc == NULL)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_prepare for syscolumns column count");	
	}
	
	/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
	
	if ( mi_open_prepared_statement(_ids_stmt_desc_colc,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 1, 
					tab_values, tab_lengths, tab_nulls, tab_types, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_open_prepared_statement for syscolumns-count");	
	}
		
	if (mi_fetch_statement(_ids_stmt_desc_colc, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in column:ColumnSetup:mi_fetch_statement in syscolumns-count");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_get_result for syscolumns-count");
	}
	
	ids_loc_rowdesc_colc = mi_get_statement_row_desc(_ids_stmt_desc_colc);
	
	num_cols = mi_column_count(ids_loc_rowdesc_colc);
		
	ids_loc_row_colc = mi_next_row (_ids_conn, &loc_err);
	
	if (ids_loc_row_colc == NULL)
	{
		sprintf (temp, "No columns for table [%s]", col_table);
		ids_dbase_err2(temp);
	}
	
	for (ctr = 0; ctr < num_cols; ctr++)
	{
		collen = 0;
		switch(mi_value(ids_loc_row_colc, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2("column:ColumnSetup:mi_value");
				return(MI_ERROR);
			case MI_NULL_VALUE:
				colval = "NULL";
				col_count = 0;
				break;
			case MI_NORMAL_VALUE:
				/*convert colval to long eventhough the 
				output data type is mi_string*/
				col_count = atoi(colval);
				break;
			default:
				ids_dbase_err2("column:ColumnSetup:mi_value");
				return(MI_ERROR);
		}
	}
	
	
	if (col_count == 0)
	{
		sprintf (temp, "No columns for table [%s]", col_table);
		ids_dbase_err2(temp);
	}
	
	
	table -> columnc = col_count + 1; /*add 1 for row_id*/
	table -> tabid = table_id;
	
	table -> columns = malloc (table -> columnc * sizeof (ColumnDef));
	memset (table -> columns, 0, table -> columnc * sizeof (ColumnDef));
	
	/*Complete the execution*/
	
	if (mi_drop_prepared_statement (_ids_stmt_desc_colc) == MI_ERROR)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_drop_prepared_statement for syscolumn-count");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_query_finish for syscolumn-count");	
	}
	
		
	/*Prepare statement for columns */

	tab_values[0] = &table_id;
	tab_lengths[0] = 0;
	tab_nulls[0] = MI_FALSE;
	tab_types[0] = "integer";
	
	_ids_stmt_desc_col = mi_prepare (_ids_conn, sql_col , NULL);
	
	if (_ids_stmt_desc_col == NULL)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_prepare for syscolumns");	
	}
	
	/*open and execute the prepared statement*/
	
	/*mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
		
	if ( mi_open_prepared_statement(_ids_stmt_desc_col,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 1, 
					tab_values, tab_lengths, tab_nulls, tab_types, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_open_prepared_statement for syscolumns");	
	}
	
	if (mi_fetch_statement(_ids_stmt_desc_col, MI_CURSOR_FIRST, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in column:ColumnSetup:mi_fetch_statement in syscolumns");
	
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_get_result for syscolumns");
	}

	ids_loc_rowdesc_col = mi_get_statement_row_desc(_ids_stmt_desc_col);
	num_cols = mi_column_count(ids_loc_rowdesc_col);
	
	
	ids_loc_row_col = mi_next_row (_ids_conn, &loc_err);
	
	ctr2 = 0;
	
	
	while (ids_loc_row_col)
	{
		
		for (ctr = 0; ctr < num_cols; ctr++)
		{
			collen = 0;
		
			switch(mi_value(ids_loc_row_col, ctr,(MI_DATUM *)&colval, &collen))
			{
				case MI_ERROR:
					ids_dbase_err2("column:ColumnSetup:mi_value  in syscolumns");
					return(MI_ERROR);
				case MI_NULL_VALUE:
					colval = "NULL";
					col_count = 0;
					break;
				case MI_NORMAL_VALUE:
					/*convert colval to long eventhough the 
					output data type is mi_string*/
					col_count = atoi(colval);
					break;
				default:
					ids_dbase_err2("column:ColumnSetup:mi_value in syscolumns");
					return(MI_ERROR);
			}/*switch mi value*/
			
			switch (ctr)
			{
				case 0:
					sprintf(col_name, "%s", colval);
					break;
				
				case 1:
					col_no = atoi(colval);
					break;
						
				case 2:
					col_type = atoi(colval);
					break;
					
				case 3:
					col_len = atoi(colval);
					break;
				
			}/*switch ctr*/						
		}/*for*/
		
		
		sprintf(table -> columns [ctr2].name,"%s", col_name);
		table -> columns [ctr2].colno = col_no;
		
		/*if data type > 256 column does not accept null values*/
		/*subtract 256 to get the real data type*/
		
		if (col_type > 256)
			col_type = col_type - 256;
			
		table -> columns [ctr2].data_type = col_type;
			
		table -> columns [ctr2].length = col_len;
		
		ctr2 ++;
		ids_loc_row_col = mi_next_row (_ids_conn, &loc_err);
		
		if (ids_loc_row_col == NULL) 
		{
			
			if (mi_fetch_statement(_ids_stmt_desc_col, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
				ids_dbase_err2 ("Error in column:ColumnSetup:mi_fetch_statement in syscolumns 2");

			if ( mi_get_result(_ids_conn) != MI_ROWS )
			{
				ids_dbase_err2("Error in column:ColumnSetup:mi_get_result for syscolumns 2");
			}
			
			ids_loc_row_col = mi_next_row (_ids_conn, &loc_err);
		}
		
			
	}/*while*/
	
	if (mi_drop_prepared_statement (_ids_stmt_desc_col) == MI_ERROR)
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_drop_prepared_statement for syscolumn");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_query_finish for syscolumn");	
	}
	
//	BuildDataBuffer(table);
	
	MapViewToColumns (table);
	MapViewToUserData (table);
	
	return (EXIT_SUCCESS);	
}

static void
MapViewToColumns (
 	TableState * table)
{
	/*
	 * Map the view references to actual ColumnDefs 
	 */
	int i;
	char temp_str[128];


	table -> viewactual = malloc (table -> viewc * sizeof (int));
	
	memset (table -> viewactual, 0, table -> viewc * sizeof (int));

	for (i = 0; i < table -> viewc; i++)
	{
		int c;
		for (c = 0; c < table -> columnc; c++)
		{
			if (!strcmp (table -> view[i].vwname, table -> columns [c].name))
			{
				/*
				 * Store position of ColumnDefinition for later use 
				 */
				table -> viewactual [i] = c;
				break;
			}
		}
		
		if (c >= table -> columnc)
		{
			sprintf (temp_str,"Error in column:MapViewToColumns  Column %s not found",
				  table -> view[i].vwname);
			ids_dbase_err2(temp_str);	
		}
	}
}

static void
MapViewToUserData (
 	TableState * table)
{
	int i,
	    offset = 0,
	    len = 0;
	
	
	/*Initialize dbview*/
	for (i = 0; i < table -> viewc; i++)
                table ->view[i].vwstart = table ->view[i].vwtype = table ->view[i].vwlen = 0;
	
	for (i = 0; i < table -> viewc; i++)
	{
		table -> view[i].vwtype = table -> columns [table -> viewactual [i]].data_type;
		
		table -> view[i].vwlen = table -> columns [table -> viewactual [i]].length;
					
		len = rtypmsize (ConvSQL2C (table -> view[i].vwtype & SQLTYPE), table -> columns [table -> viewactual [i]].length);			
		table -> view[i].vwlen = len;
		
		table -> view[i].vwstart = rtypalign(offset,ConvSQL2C (table -> view[i].vwtype & SQLTYPE));
		
		
		/*offset = offset + table -> view[i].vwlen;*/
		offset =  table -> view[i].vwstart + len;
		
	}
	
	 
}




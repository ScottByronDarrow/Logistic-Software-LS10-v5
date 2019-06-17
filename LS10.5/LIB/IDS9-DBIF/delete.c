/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: delete.c,v 1.1 2002/11/11 02:44:05 cha Exp $
|  Program Name  : (query.c)
|  Program Desc  : (handles query mechanism for find_rec)
|---------------------------------------------------------------------|
|
=====================================================================*/

/*Avoid duplicate definition for some 
functions and data structures from 
Informix and Standard C*/
#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif  

#include	<stdio.h>
#include 	<string.h>
#include	<stdlib.h>

#include	"ids9dbif.h"

/*Local Functions*/
int BuildStatement (TableState *);	/* Build SQL Statements */
int BuildInputVars (TableState *);	/* Build the values  */
void CleanUp ();

/*Local variables*/
MI_DATUM 	*d_values;				/*for values*/
mi_integer 	*d_lengths;				/*lengths*/
mi_integer 	*d_nulls;				/*null values , MI_TRUE, MI_FALSE*/
mi_string 	**d_types;				/*data type of the input parameter*/

int params = 0;
char 	*sql_str_del;

int
_DeleteRow  (TableState * state)
{
	d_values  = NULL;
	d_types   = NULL;
	d_lengths = NULL;
	d_nulls   = NULL;
	
	params = 0;
	

	BuildStatement (state);
	//BuildInputVars (state);
	
	
	/*mi_integer mi_exec_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, retlen, rettypes)*/
	
	
	//if ( mi_exec_prepared_statement(state -> d_stmt, MI_SEND_SCROLL , MI_BINARY, state -> indexc, 
	//				d_values, d_lengths, d_nulls, d_types, 
	//				NULL, NULL) == MI_ERROR )
	if ( mi_exec_prepared_statement(state -> d_stmt, MI_SEND_SCROLL , MI_BINARY,0, 
					NULL, NULL, NULL, NULL, 
					NULL, NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in delete:InsertRow:mi_exec_prepared_statement");	
	}
	
	if (mi_get_result (_ids_conn) == MI_ERROR)
	{
		ids_dbase_err2("Error in delete:InsertRow:mi_get_result");	
		return (1);
	}
	
	/*if (mi_result_row_count(_ids_conn) == 0)
	{
		ids_dbase_err2("Error in delete:InsertRow:mi_result_row_count");
		return (1);
	}*/
	
	CleanUp();	
	return (0);
}

int 
BuildStatement (TableState *state)
{
	char	col_name [128],
		col_val	 [256];
		
	int 	sql_strsz 	= 0,
	    	ctr 		= 0,
	    	pos		= 0;
	    	
	sql_strsz = 25; /* for delete from tablename, where clause*/    	
	
	sql_strsz += strlen (state -> table ? state -> table : state -> named);
	
	for (ctr = 0; ctr < state -> indexc; ctr++)
	{
		sql_strsz += (strlen (state -> columns [state -> indexactual [ctr]].name) + 5);
		sql_strsz += (state ->view[state -> indexview[ctr]].vwlen * 2);
	}
	
	//sql_str_del = malloc (sql_strsz + 30); 
	
	if (state -> delete)
	{
		free (state -> delete);
		state -> delete = NULL;
	}
		
	state -> delete = malloc (sql_strsz + 30);  /*30 is for over flow buffer*/
	sql_str_del = state -> delete;
	memset (sql_str_del, 0,sql_strsz +30);
	
	
	strcat (sql_str_del,"delete from ");
	strcat (sql_str_del, state -> table ? state -> table : state -> named);
	
	for (ctr = 0; ctr < state -> indexc; ctr++)
	{
		
		pos = state -> indexactual[ctr];
		switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
		{
			case CINTTYPE:
				sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
				break;
			case CDATETYPE:
			case CLONGTYPE:
				sprintf (col_val,"%ld", *(long *) state -> columns[pos].data);
				break;
				
			case CCHARTYPE:
				sprintf (col_val,"'%s'", state -> columns[pos].data);
				break;
				
			case CDOUBLETYPE:	
				sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
				break;
				
			case CFLOATTYPE:
				sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
				break;
		}	
		
		/*sprintf (col_name, "%s %s %s %s", 
			ctr ?  " and " : " where " ,
			state ->  view [state -> indexview[ctr]].vwname,
			" = ",
			col_val);
		strcat (sql_str_del, col_name);*/

	}

	sprintf (col_name, " where rowid = %ld", state -> rowid);
	strcat (sql_str_del, col_name);
	
	
	
	if (state -> d_stmt != NULL)
	{
		if (mi_drop_prepared_statement (state -> d_stmt) == MI_ERROR)
		{
			ids_dbase_err2("Error in delete:BuildStatement:mi_drop_prepared_statement");	
		}
		
		state -> d_stmt = NULL;
	}

	state -> d_stmt  = mi_prepare (_ids_conn, sql_str_del , NULL);
	
	if (state -> d_stmt == NULL)
	{
		ids_dbase_err2("Error in delete:BuildStatement:mi_prepare");	
	}
	
	return (0);
}


int 
BuildInputVars (TableState *state)
{
	int ctr = 0,
	    pos = 0;
	  
	
	
	
	/*Alocate memory for input parameters based on view count*/
	d_values  = malloc ((state -> viewc + state -> indexc) * sizeof (MI_DATUM));
	d_types   = malloc ((state -> viewc + state -> indexc) * sizeof (mi_string));
	d_lengths = malloc ((state -> viewc + state -> indexc) * sizeof (mi_integer));
	d_nulls   = malloc ((state -> viewc + state -> indexc) * sizeof (mi_integer));
	
	
	for (ctr = 0; ctr < state -> indexc; ctr++)
	{
		pos = state -> indexactual [ctr];
				
		d_values  [ctr] = state -> columns [pos].data;
		
		if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CCHARTYPE)
		{
			d_values[ctr] = PadOut(state -> columns[pos].data, state -> columns [pos].length);
			d_values[ctr] =  mi_string_to_lvarchar(d_values[ctr]);
			d_lengths [ctr] = 0;
		}else
			d_lengths [ctr] = 0;
		
		d_types[ctr] = GetColDataType (state -> columns + pos);
		d_nulls [ctr] = MI_FALSE;
	}       
	        
	return (0);
}


void
CleanUp()
{
	/*Clean up the mess */
	
	if (d_values)
		free (d_values);
	
	if (d_types)
		free (d_types);
	
	if (d_nulls)
		free (d_nulls);
	
	if (d_lengths)
		free (d_lengths);	
}



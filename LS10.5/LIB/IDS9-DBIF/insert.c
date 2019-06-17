/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: insert.c,v 1.1 2002/11/11 02:44:05 cha Exp $
|  Program Name  : (insert.c)
|  Program Desc  : (handles inserting of records to the DB)
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
MI_DATUM 	*i_values;				/*for values*/
mi_integer 	*i_lengths;				/*lengths*/
mi_integer 	*i_nulls;				/*null values , MI_TRUE, MI_FALSE*/
mi_string 	**i_types;				/*data type of the input parameter*/
char 		*sql_str;
int params = 0;

int
_InsertRow (
	TableState * state,
	void * buffer)
{
	int   ctr    = 0;
	
	i_values  = NULL;
	i_types   = NULL;
	i_lengths = NULL;
	i_nulls   = NULL;
	
	params = 0;
	
	
	
	/*Extract the data from the user data buffer just to be sure*/
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{
		GetDataApp (state -> columns + state -> viewactual[ctr], 
			    state -> view[ctr].vwstart,
			    buffer);
	}
	BuildStatement (state);
	
	if ( mi_exec_prepared_statement(state -> i_stmt, MI_SEND_SCROLL , MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, 
					NULL, NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in insert:InsertRow:mi_exec_prepared_statement");	
	}
	
	if (mi_get_result (_ids_conn) == MI_ERROR)
	{
		fprintf (stdout,"\ntable name [%s]\n\n\n\n",state -> table ? state -> table : state -> named);
		ids_dbase_err2("Error in insert:InsertRow:mi_get_result");	
		//return (1);
	}
	
	if (mi_result_row_count(_ids_conn) == 0)
	{
		ids_dbase_err2("Error in insert:InsertRow:mi_result_row_count");
		return (1);
	}
	
	/*if (mi_drop_prepared_statement (state -> i_stmt) == MI_ERROR)
	{
			ids_dbase_err2("Error in insert:InsertRow:mi_drop_prepared_statement");	
	}*/
	
	//CleanUp();	
	return (0);
}

int 
BuildStatement (TableState *state)
{
	
	int 	sql_strsz 	= 0,
	    	ctr 		= 0,
	    	pos		= 0;
	char	col_val [256];
	
	/*Make a wild guess about the length of the SQL string*/
	sql_strsz = 20; /* this for the SQL keywords*/
	
	/*get the length of the column name*/
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{
		sql_strsz = sql_strsz + strlen (state -> view [ctr].vwname) + (state ->view[ctr].vwlen * 2) + 3; /*additonal characters for the comma and space*/
	}
	
	/* for values*/
	sql_strsz += (state -> viewc * 2);
	
	sql_strsz += 30; /*for overflow*/
	
	//sql_str = malloc (sql_strsz);
	if (state -> insert)
	{
		free (state -> insert);
		state -> insert = NULL;
	}
	
	state -> insert = malloc (sql_strsz);
	sql_str = state -> insert;
	memset (sql_str, 0,sql_strsz);
	
	/*assemble the insert into*/
	
	sprintf (sql_str, "insert into %s (", state -> table ? state -> table : state -> named);
	
	pos = state -> viewc;
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
		strcat(strcat (sql_str,state -> view [ctr].vwname), 
		              (ctr == state -> viewc -1) ? ") " : ", ");
		
	}
	
	strcat (sql_str, " values (");
	pos = state -> viewc;
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
		pos = state -> viewactual[ctr];
		if ((state -> columns [pos].data_type & SQLTYPE) == SQLSERIAL)
		{
			sprintf (col_val,"%ld", 0L);
		}
		else
		{
			switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
			{
				case CINTTYPE:
					sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
					break;
				case CDATETYPE:
				case CLONGTYPE:
					sprintf (col_val,"%ld", *(long *) state -> columns[pos].data);
					//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
					break;
					
				case CCHARTYPE:
					sprintf (col_val,"'%s'", state -> columns[pos].data);
					//fprintf (stdout,"\nBuildStatement CCHARTYPE [%s]", col_val);
					break;
					
				case CDOUBLETYPE:	
					sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
					//fprintf (stdout,"\nBuildStatement CDOUBLETYPE [%s]",col_val);
					break;
					
				case CFLOATTYPE:
					sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
					//fprintf (stdout,"\n BuildStatement CFLOATTYPE [%s]",  col_val);
					break;
			}
		}
		//strcat(sql_str, (ctr == state -> viewc -1) ? " ?) " : " ?,");
		strcat (strcat(sql_str, col_val),(ctr + 1 == state -> viewc ) ? " )" : " ,");
	}
	
	//fprintf (stdout,"\n\nINSERT sql_str [%s] len [%d] \n\n\n", sql_str, strlen (sql_str));
	
	if (state -> i_stmt != NULL)
	{
		//fprintf (stdout,"\nINSERT QuerySetup dropprepared!!!");
		if (mi_drop_prepared_statement (state -> i_stmt) == MI_ERROR)
		{
			ids_dbase_err2("Error in insert:BuildStatement:mi_drop_prepared_statement");	
		}
		
		state -> i_stmt = NULL;
	}
	//fprintf (stdout,"\n\nINSERT 1");
	state -> i_stmt  = mi_prepare (_ids_conn, sql_str , NULL);
	//fprintf (stdout,"\n\nINSERT 2");
	if (state -> i_stmt == NULL)
	{
		ids_dbase_err2("Error in insert:BuildStatement:mi_prepare");	
	}
	
	
	//fprintf (stdout,"\n\nINSERT 3");
	return (0);
}

int 
BuildInputVars (TableState *state)
{
	int ctr = 0,
	    pos = 0;
	
	
	//fprintf (stdout,"\nINSERT BUILDINPUT 1");
	/*Alocate memory for input parameters based on view count*/
	i_values  = malloc (state -> viewc * sizeof (MI_DATUM));
	i_types   = malloc (state -> viewc * sizeof (mi_string));
	i_lengths = malloc (state -> viewc * sizeof (mi_integer));
	i_nulls   = malloc (state -> viewc * sizeof (mi_integer));
	
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
	//	fprintf (stdout,"\nINSERT BUILDINPUT 2");
		pos = state -> viewactual[ctr];	
		if ((state -> columns [pos].data_type & SQLTYPE) == SQLSERIAL)
		{
			/* if serial data type make data = 0 to use */
			/* system generated serial value*/
			 *(long *) state -> columns[pos].data = 0;	
		}
		i_values[ctr] = state -> columns[pos].data;
		
	//	fprintf (stdout,"\nINSERT BUILDINPUT 3");
		if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CCHARTYPE)
		{
		//	fprintf (stdout,"\nINSERT BUILDINPUT 4");
			i_values[ctr] = PadOut(state -> columns[pos].data, state -> columns [pos].length);
			i_values[ctr] =  mi_string_to_lvarchar(i_values[ctr]);
			i_lengths[ctr] = state -> columns [pos].length;
		//	fprintf (stdout,"\nINSERT BUILDINPUT 5");
		}
		else
			i_lengths[ctr] = 0;
		
		i_types[ctr] = GetColDataType (state -> columns + pos);
		i_nulls[ctr] = MI_FALSE;
		
	//	fprintf (stdout,"\n BindInputVARS ctr [%d] types [%s] lengths [%ld] col name [%s]", 
		//		ctr, i_types[ctr] , i_lengths[ctr], state -> columns[pos].name);
		
		/*switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
		{
			
			case CINTTYPE:
				fprintf (stdout,"\nBindInputVARS CINTTYPE [%d]",  *(int *) i_values[ctr]);
				break;
			case CDATETYPE:
			case CLONGTYPE:
				fprintf (stdout,"\nBindInputVARS CLONGTYPE [%ld]",  *(long *) i_values[ctr]);
				break;
				
			case CCHARTYPE:
				fprintf (stdout,"\nBindInputVARS CCHARTYPE [%s] strlen(%d)", 
					(char *) mi_lvarchar_to_string(i_values[ctr]),
					strlen(state -> columns[pos].data));
				break;
				
			case CDOUBLETYPE:	
				fprintf (stdout,"\nBindInputVARS CDOUBLETYPE [%g]",  *(double *) i_values[ctr]);
				break;
				
			case CFLOATTYPE:
				fprintf (stdout,"\nBindInputVARS CFLOATTYPE [%f]",  *(float *) i_values[ctr]);
				break;
		}*/
	}
	
	return (0);
}


void
CleanUp()
{
	/*Clean up the mess */
	
	if (i_values)
		free (i_values);
	
	if (i_types)
		free (i_types);
	
	if (i_nulls)
		free (i_nulls);
	
	if (i_lengths)
		free (i_lengths);	
	
	if (sql_str)
		free (sql_str);
}


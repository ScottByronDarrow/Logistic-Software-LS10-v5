/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: update.c,v 1.1 2002/11/11 02:44:05 cha Exp $
|  Program Name  : (update.c)
|  Program Desc  : (handles update mechanism)
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
#include     	<DateToString.h>

/*Local Functions*/
int BuildStatement (TableState *);	/* Build SQL Statements */
int BuildInputVars (TableState *);	/* Build the values  */
void CleanUp ();

/*Local variables*/
MI_DATUM 	*u_values;				/*for values*/
mi_integer 	*u_lengths;				/*lengths*/
mi_integer 	*u_nulls;				/*null values , MI_TRUE, MI_FALSE*/
mi_string 	**u_types;				/*data type of the input parameter*/

int params = 0,
    serial = 0;

int
_UpdateRow (
	TableState * state,
	void * buffer)
{
	int   ctr    = 0;
	
	u_values  = NULL;
	u_types   = NULL;
	u_lengths = NULL;
	u_nulls   = NULL;
	
	params = 0;
	
	
	
	/*Extract the data from the user data buffer just to be sure*/
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{
		GetDataApp (state -> columns + state -> viewactual[ctr], 
			    state -> view[ctr].vwstart,
			    buffer);
	}
	
	BuildStatement (state);
	
	//BuildInputVars (state);
	//params = state -> viewc + state -> indexc - serial;
	//fprintf (stdout,"\nUPDATEROW params [%d]\n\n",params);
	/*mi_integer mi_exec_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, retlen, rettypes)*/
	
	//if ( mi_exec_prepared_statement(state -> u_stmt, MI_SEND_SCROLL , MI_BINARY, params, 
	//				u_values, u_lengths, u_nulls,u_types, 
	//				NULL, NULL) == MI_ERROR )
	if ( mi_exec_prepared_statement(state -> u_stmt, MI_SEND_SCROLL , MI_BINARY, 0, 
					NULL, NULL, NULL,NULL, 
					NULL, NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in update:UpdateRow:mi_exec_prepared_statement");	
	}
	
	if (mi_get_result (_ids_conn) == MI_ERROR)
	{
		ids_dbase_err2("Error in update:UpdateRow:mi_get_result");	
		return (1);
	}
	
	if (mi_result_row_count(_ids_conn) == 0)
	{
	//	ids_dbase_err2("Error in update:UpdateRow:mi_result_row_count");
	fprintf (stdout,"\nUPDATEROW Error in update:UpdateRow:mi_result_row_count");
		return (1);
	}
	//fprintf (stdout,"\nUPDATEROW END");
	//CleanUp ();
	return (0);
}

int 
BuildStatement (TableState *state)
{
	static char 	*sql_str;
	char 	col_name [128],
	      	col_val [256],
	      	temp [128];
	      	
	int 	sql_strsz 	= 0,
	    	ctr 		= 0,
	    	pos		= 0;
	    	
	 serial		= 0;
	    	
		/*Make a wild guess about the length of the SQL string*/
	/*get the length of the column name*/
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{
		sql_strsz = sql_strsz + strlen (state -> view [ctr].vwname) + (state ->view[ctr].vwlen * 2) + 3; /*additonal characters for the comma and space*/
	}
	
	sql_strsz = sql_strsz + 60; /*for the "select, order by, where, from, asc, desc " words */
	
	/*now get the length of the order by  and where clauses*/
	for (ctr = 0; ctr < state-> indexc; ctr++)
	{
		sprintf (col_name, "%s", state -> columns [state -> indexview[ctr]].name);
		sql_strsz = sql_strsz + ((strlen(col_name) + 10) * 2);
		sql_strsz += (state ->view[state -> indexview[ctr]].vwlen * 2);
		/*additonal characters for the comma, space, "and"*/
	}
	//fprintf (stdout,"\nBUILDSTATEMENT len [%d]", sql_strsz + 130);
	
	if (state -> update)
	{
		free (state -> update);
		state -> update = NULL;
	}
	
	state -> update = malloc (sql_strsz + 130); /*30 is for over flow buffer*/
	sql_str = state -> update;
	memset (sql_str, 0,sql_strsz + 130);
	sprintf (sql_str,"update %s set (", state -> table ? state -> table : state -> named);
	
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
		if ((state -> columns [state -> viewactual[ctr]].data_type & SQLTYPE) == SQLSERIAL)
		{
			serial ++;
		}
	}
	
	
	
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
		//fprintf (stdout,"\nUPDATE SER vwname [%s] ctr [%d] vwcount [%d]",
		//	 state -> view [ctr].vwname, ctr, state -> viewc);
		if ((state -> columns [state -> viewactual[ctr]].data_type & SQLTYPE) != SQLSERIAL)
		{
			//fprintf (stdout,"\nUPDATE SER vwname [%s]", state -> view [ctr].vwname);
			if (ctr + 2 == state -> viewc)
			{
				//fprintf (stdout,"\nUPDATE SER 1.1");	
				if ((state -> columns [state -> viewactual[ctr + 1]].data_type & SQLTYPE) == SQLSERIAL)
					strcat(strcat (sql_str,state -> view [ctr].vwname)," ) ");
				else
					strcat(strcat (sql_str,state -> view [ctr].vwname), (ctr + 1  == state -> viewc ) ? " ) " : " ,");
				//fprintf (stdout,"\nUPDATE SER 1.2");
			}
			else
			{
				strcat(strcat (sql_str,state -> view [ctr].vwname), (ctr + 1  == state -> viewc ) ? " ) " : " ,");
				//fprintf (stdout,"\nUPDATE SER 1.3");
			}
		}
			
	}
	//fprintf (stdout,"\nUPDATE SER 2");
	
	strcat (sql_str," = ( ");
	
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{	
		pos = state -> viewactual[ctr];
		memset (col_val,0,sizeof(col_val));
		//fprintf (stdout,"\n BuildStatement VALUE col name[%s]",  state -> columns[pos].name);
		if ((state -> columns [state -> viewactual[ctr]].data_type & SQLTYPE) != SQLSERIAL)
		{
			switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
			{
				case CINTTYPE:
					sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
					//fprintf (stdout,"\n BuildStatement CINTTYPE [%s]",  col_val);
					break;
				case CDATETYPE:
					sprintf (col_val,"%ld", StringToDate(DateToString(*(long *) state -> columns[pos].data)));
					//fprintf (stdout,"\nBuildStatement CDATETYPE [%s]",  col_val);
					break;	
				case CLONGTYPE:
					sprintf (col_val,"%ld", *(long *) state -> columns[pos].data);
					//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
					break;
					
				case CCHARTYPE:
					if (strlen (state -> columns[pos].data)!=0)
					{
						sprintf (col_val,"%s", state -> columns[pos].data);
						//fprintf (stdout,"\nBuildStatement 1 CCHARTYPE [%s]", col_val);
						if (!strcmp(col_val, "'" ))
							sprintf (col_val,"' '");
						else
							sprintf (col_val,"'%s'", state -> columns[pos].data);
					}
					else
						sprintf (col_val,"' '");
					//fprintf (stdout,"\nBuildStatement 2 CCHARTYPE [%s]", col_val);
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
			if (ctr + 2 == state -> viewc)
			{
				if ((state -> columns [state -> viewactual[ctr + 1]].data_type & SQLTYPE) == SQLSERIAL)
					strcat (strcat(sql_str, col_val)," )");
				else
					strcat (strcat(sql_str, col_val),(ctr + 1  == state -> viewc ) ? " )" : " ,");
			}
			else			
				strcat (strcat(sql_str, col_val),(ctr + 1  == state -> viewc ) ? " )" : " ,");
		}
	} 
	
	/*for (ctr = 0; ctr < state -> indexc; ctr++)
	{
		pos = state -> indexactual[ctr];
		switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
		{
			case CINTTYPE:
				sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
				//fprintf (stdout,"\n BuildStatement CINTTYPE [%s]",  col_val);
				break;
			case CDATETYPE:
				sprintf (col_val,"%ld", StringToDate(DateToString(*(long *) state -> columns[pos].data)));
				//fprintf (stdout,"\nBuildStatement CDATETYPE [%s]",  col_val);
				break;				
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
		
		sprintf (col_name, "%s %s %s %s", 
			ctr ?  " and " : " where " ,
			state ->  view [state -> indexview[ctr]].vwname,
			" = ",
			col_val);
		strcat (sql_str, col_name);

	}*/
	
	sprintf (temp, " where rowid = %ld", state -> rowid);
	strcat (sql_str, temp);
		
	//fprintf (stdout,"\nBUILDSTATEMENT sql_str [%s] len [%d] vwcount [%d] serial [%d]",
	//	 sql_str, strlen(sql_str), state -> viewc, serial);
	
	
	if (state -> u_stmt != NULL)
	{
		//fprintf (stdout,"\nQuerySetup dropprepared!!!");
		if (mi_drop_prepared_statement (state -> u_stmt) == MI_ERROR)
		{
			ids_dbase_err2("Error in update:BuildStatement:mi_drop_prepared_statement");	
		}
		
		state -> u_stmt = NULL;
		
		//if (state -> u_stmt)
			//fprintf (stdout,"\nQuerySetup dropprepared merong pang state -> u_stmt!!!");
	}

	state -> u_stmt  = mi_prepare (_ids_conn, sql_str , NULL);
	
	if (state -> u_stmt == NULL)
	{
		ids_dbase_err2("Error in update::BuildStatement:mi_prepare");	
	}
	
	return (0);	    	
	    	
}

int 
BuildInputVars (TableState *state)
{
	int ctr = 0,
	    pos = 0;
	  
	
	
	//fprintf (stdout,"\nUPDATE BUILDINPUT 1");
	/*Alocate memory for input parameters based on view count*/
	u_values  = malloc ((state -> viewc + state -> indexc) * sizeof (MI_DATUM));
	u_types   = malloc ((state -> viewc + state -> indexc) * sizeof (mi_string));
	u_lengths = malloc ((state -> viewc + state -> indexc) * sizeof (mi_integer));
	u_nulls   = malloc ((state -> viewc + state -> indexc) * sizeof (mi_integer));
	
	/*for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
	
		pos = state -> viewactual[ctr];	
		
		if ((state -> columns [pos].data_type & SQLTYPE) != SQLSERIAL)
		{
			
				
			u_values[params] = state -> columns[pos].data;
			
			
			if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CCHARTYPE)
			{
				u_values[params] = PadOut(state -> columns[pos].data, state -> columns [pos].length);
				u_values[params] =  mi_string_to_lvarchar(u_values[params]);
				//u_lengths[params] = state -> columns [pos].length;
				u_lengths [params] = 0;
			}
			else
				u_lengths[params] = 0;
			
			u_types[params] = GetColDataType (state -> columns + pos);
			u_nulls[params] = MI_FALSE;
			
			fprintf (stdout,"\n UPDATE BindInputVARS ctr [%d] types [%s] lengths [%ld] col name [%s]", 
					params, u_types[params] , u_lengths[params], state -> columns[pos].name);
			
			switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
			{
				
				case CINTTYPE:
					fprintf (stdout,"\nUPDATE BindInputVARS CINTTYPE [%d]",  *(int *) u_values[params]);
					break;
				case CDATETYPE:
				case CLONGTYPE:
					fprintf (stdout,"\nUPDATE BindInputVARS CLONGTYPE [%ld]",  *(long *) u_values[params]);
					break;
					
				case CCHARTYPE:
					fprintf (stdout,"\nUPDATE BindInputVARS CCHARTYPE [%s] strlen(%d)", 
						(char *) mi_lvarchar_to_string(u_values[params]),
					
						strlen(state -> columns[pos].data));
					break;
					
				case CDOUBLETYPE:	
					fprintf (stdout,"\nUPDATE BindInputVARS CDOUBLETYPE [%g]",  *(double *) u_values[params]);
					break;
					
				case CFLOATTYPE:
					fprintf (stdout,"\nUPDATE BindInputVARS CFLOATTYPE [%f]",  *(float *) u_values[params]);
					break;
			}
			
			params++;
		}
	}*/
	
	//ctr2 = params;
	//fprintf (stdout,"\n UPDATE 2 BindInputVARS  0.1 params [%d] ctr [%d]", params, ctr);
	for (ctr = 0; ctr < state -> indexc; ctr++)
	{
		pos = state -> indexactual [ctr];
		//fprintf (stdout,"\n UPDATE 2 BindInputVARS  1 params [%d]", params);
		
		u_values  [params] = state -> columns [pos].data;
		
		if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CCHARTYPE)
		{
			//fprintf (stdout,"\n UPDATE 2 BindInputVARS  1.1");
			u_values[params] = PadOut(state -> columns[pos].data, state -> columns [pos].length);
			u_values[params] =  mi_string_to_lvarchar(u_values[params]);
			//fprintf (stdout,"\n UPDATE 2 BindInputVARS  1.3");
			//u_lengths[params] = state -> columns [pos].length;
			u_lengths [params] = 0;
		}else
			u_lengths [params] = 0;
		
		u_types[params] = GetColDataType (state -> columns + pos);
		u_nulls [params] = MI_FALSE;
		
		//fprintf (stdout,"\n UPDATE 2 BindInputVARS ctr [%d] types [%s] lengths [%ld] col name [%s]", 
		//		ctr, u_types[params] , u_lengths[params], state -> columns[pos].name);
		
		/*switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
		{
			
			case CINTTYPE:
				fprintf (stdout,"\nUPDATE2 BindInputVARS CINTTYPE [%d]",  *(int *) u_values[params]);
				break;
			case CDATETYPE:
			case CLONGTYPE:
				fprintf (stdout,"\nUPDATE 2 BindInputVARS CLONGTYPE [%ld]",  *(long *) u_values[params]);
				break;
				
			case CCHARTYPE:
				fprintf (stdout,"\nUPDATE2 BindInputVARS CCHARTYPE [%s] strlen(%d)", 
					(char *) mi_lvarchar_to_string(u_values[params]),
					strlen(state -> columns[pos].data));
				break;
				
			case CDOUBLETYPE:	
				fprintf (stdout,"\nUPDATE2 BindInputVARS CDOUBLETYPE [%g]",  *(double *) u_values[params]);
				break;
				
			case CFLOATTYPE:
				fprintf (stdout,"\nUPDATE2 BindInputVARS CFLOATTYPE [%f]",  *(float *) u_values[params]);
				break;
		}*/
		params++;
		
	}       
	        
	return (0);
}
	    
	    
void
CleanUp()
{
	/*Clean up the mess */
	
	if (u_values)
		free (u_values);
	
	if (u_types)
		free (u_types);
	
	if (u_nulls)
		free (u_nulls);
	
	if (u_lengths)
		free (u_lengths);	
}



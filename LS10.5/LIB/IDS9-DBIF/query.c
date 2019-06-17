/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: query.c,v 1.2 2002/11/11 02:41:10 cha Exp $
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
#include 	<twodec.h>

#include	"ids9dbif.h"


/*Local Functions*/
int BuildStatement (TableState *, int);	/* Build SQL Statements */
int BuildInputVars (TableState *);	/* Build the values for where clause */
char *GetOperator (int, int, TableState *);		/* Resolves operator in to strings i.e <, <=, >, >=, =*/
MI_CURSOR_ACTION GetCursorAction (int, int, TableState *);
void CleanUp();

char 	*sql_str;

int QuerySetup (
	TableState *state,
	const int mode,
	void * buffer)
{
	int ctr = 0;
	//    params = 0
	
	state -> values = NULL;
	state -> types = NULL;
	state -> lengths = NULL;
	state -> nulls	= NULL;
	//fprintf (stdout,"\n QUERY 1");
	
	if (mode == GTEQ)
		state -> gteq_called = TRUE;
		
	
	
	if ((mode != FIRST && mode != LAST && 
	    mode != NEXT && mode != PREVIOUS &&
	    mode != CURRENT ) || 
	    (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ) ||
	    (mode == LT && state -> lastactualfetchmode == GTEQ) ||
	    (mode == LAST && state -> lastactualfetchmode == GTEQ) ||
	    (mode == COMPARISON || mode == EQUAL))
	{
		//fprintf (stdout,"\n QUERY 1.1");
		for (ctr = 0; ctr < state -> viewc; ctr++)
		{
			//fprintf (stdout,"\n QUERY 1.2");
			GetDataApp (state -> columns + state -> viewactual[ctr], 
				    state -> view[ctr].vwstart,
				    buffer);
		//	fprintf (stdout,"\n QUERY 1.3");
		}
	}
	//fprintf (stdout,"\n QUERY 2");
	/*checked if query statement was already processed*/
	if ((state -> stmt_processed == FALSE) || 
	    (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ) || 
	    (mode == LT && state -> lastactualfetchmode == GTEQ) ||
	    (mode == LAST && state -> lastactualfetchmode == GTEQ) ||
	    (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ))
	{
		/*Prepare and parsed statement */
		//fprintf (stdout,"\n QUERY 3");
		BuildStatement (state, mode);
		//fprintf (stdout,"\n QUERY 4");
		state -> stmt_processed = TRUE;		
	}
	
	//fprintf (stdout,"\n QUERY 5");
	
		
	/*Extract the data from the user data buffer*/
	
	
	/*if (mode != FIRST && mode != LAST && 
	    mode != NEXT && mode != PREVIOUS && mode != CURRENT )
	{
		BuildInputVars (state);
		params = state -> indexc;
	}
	else
		params = 0;*/
	

	
	/*Execute*/
	if ((mode == FIRST && state -> lastactualfetchmode == -1) || 
	    (mode == LAST && state -> lastactualfetchmode == -1) ||
	    (mode == GTEQ || mode == GREATER) ||
	    (mode == LTEQ || mode == LT) || 
	    (mode == EQUAL || mode == COMPARISON) ||
	    (mode == LAST && state -> lastactualfetchmode == GTEQ)||
	    (mode == LT && state -> lastactualfetchmode == GTEQ) ||
	    (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ))
	{
		//fprintf (stdout,"\nQUERYSETUP mi_open_prepared_statement params [%d]\n\n",params);
			/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
			n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
		
		//if ( mi_open_prepared_statement(state -> q_stmt,MI_SEND_READ + MI_SEND_SCROLL, MI_TRUE, params, 
		//				state -> values, state -> lengths, state -> nulls, state -> types, NULL, 0,  NULL) == MI_ERROR )
		
		//fprintf (stdout,"\n QUERY 6 tablename [%s]\n\n\n", state -> table ? state -> table : state -> named);
		if ( mi_open_prepared_statement(state -> q_stmt,MI_SEND_READ + MI_SEND_SCROLL + MI_SEND_HOLD, MI_TRUE, 0, 
						NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
		{
			ids_dbase_err2("Error in query:QuerySetup:mi_open_prepared_statement");	
		}
		//fprintf (stdout,"\n QUERY 7");
	}
	
	
	//fprintf (stdout,"\n QUERY 8");
	//CleanUp();
	//fprintf (stdout,"\n QUERY 9");
	return (0);	
}

int
BuildStatement  (
	TableState * state,
	int mode)
{

	char	col_name [128],
		col_val  [256],
		tmp_val  [256];
	int 	sql_strsz 	= 0,
	    	ctr 		= 0,
	    	pos 		= 0,
	    	index_cnt	= 0,
	    	idx_val_found   = TRUE,
	    	gteq_spec		 = FALSE;
	
	/*Make a wild guess about the length of the SQL string*/
	/*get the length of the column name*/
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{
		sql_strsz = sql_strsz + strlen (state -> view [ctr].vwname) + 3; /*additonal characters for the comma and space*/
	}
	
	sql_strsz = sql_strsz + 160; /*for the "select, order by, where, from, asc, desc " words */
	
	sql_strsz = sql_strsz + (strlen(state -> indexname) + strlen(state -> table ? state -> table : state -> named));
	/*now get the length of the order by  and where clauses*/
	for (ctr = 0; ctr < state-> indexc; ctr++)
	{
		sprintf (col_name, "%s", state -> columns [state -> indexview[ctr]].name);
		sql_strsz = sql_strsz + ((strlen(col_name) + 10) * 4);
		sql_strsz += (state ->view[state -> indexview[ctr]].vwlen * 4);
		/*additonal characters for the comma, space, "and"*/
	}
	
	if (state -> query)
	{
		free (state -> query);
		state -> query = NULL;
	}
	
	state -> query = malloc (sql_strsz + 60); /*60 is for over flow buffer*/
	sql_str = state -> query;
	
	memset (sql_str, 0,sql_strsz + 60);
	//fprintf (stdout,"\nQUERYSETUP 1 sql_strz[%d]", sql_strsz);
	
	
	/*Assemble the select statement*/
	sprintf (sql_str,"select {+ index( %s %s  )} ", state -> table ? state -> table : state -> named,
		  state -> indexname);
	
	for (ctr = 0; ctr < state -> viewc; ctr ++)
	{
		strcat(strcat (sql_str,state -> view [ctr].vwname), ", ");
	}
	
	strcat(strcat(sql_str, "rowid from "), state -> table ? state -> table : state -> named);
//	fprintf (stdout,"\nQUERYSETUP 2 sql_str [%s] length [%d]",sql_str, strlen (sql_str));
	
	/*Assemble the where and order clause */
	/*Do not put where and order clause if table has no index*/
	if (state -> indexc != 0)
	{
		/*No where clause if search mode is FIRST and LAST */
	       if ((mode != FIRST ) && 
		    ( (mode == LAST && state -> lastactualfetchmode == GTEQ) || 
		     (mode == LT && state -> lastactualfetchmode == GTEQ) || 
		     (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ) ||
		     (mode == GTEQ || mode == LTEQ || mode == LT || mode == COMPARISON || 
		      mode == EQUAL || mode == GREATER)))
		
		{
			if ((mode == PREVIOUS && state -> lastactualfetchmode == GTEQ)&&
			     (mode != LT && mode != LTEQ))
			{
				//fprintf (stdout,"\nQUERYSETUP 2.1");
				index_cnt = 1;
			}
			else
			{
				//fprintf (stdout,"\nQUERYSETUP 2.2");
				index_cnt = state -> indexc;
			}
			
			/*where clause*/
			
			if (mode == LT || mode == LTEQ || mode == PREVIOUS)
			{				
				if (state -> lastactualfetchmode != GTEQ)
				{	
					
					if (state -> indexc > 1)
					{				
						strcat (sql_str, " where (");
						pos = state -> indexactual [0];					
						if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CLONGTYPE)
						{
							sprintf (col_name, "%s = %ld) and (", 
								state ->  view [state -> indexview[0]].vwname,
								(*(long *) state -> columns[pos].data));
							strcat (sql_str, col_name);
						}
					}
					
					for (ctr = 0; ctr < index_cnt; ctr++)
					{
						pos = state -> indexactual[ctr];
						if ((state -> columns[pos].data_type & SQLTYPE) == SQLCHAR)
						{
							sprintf (col_name, " RPAD (%s,%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								state ->  view [state -> indexview[ctr]].vwlen,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}else if ((state -> columns[pos].data_type & SQLTYPE) == SQLDATE)
						{
							sprintf (col_name, " LPAD (TO_CHAR(%s,\"%%Y%%m%%d\"),%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								11,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}else 
						{
							sprintf (col_name, " LPAD (%s,%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								11,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}
						
						
						strcat (sql_str, col_name);
						strcat (sql_str, " ");
					}/*for*/
					
					strcat (sql_str, " ");
					strcat(sql_str,GetOperator(mode,(mode != LT) ? TRUE:FALSE , state));
					strcat (sql_str, " '");
					
					for (ctr = 0; ctr < index_cnt; ctr++)
					{
						pos = state -> indexactual[ctr];
						switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
						{
							case CINTTYPE:
								sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\n BuildStatement CINTTYPE [%s]",  col_val);
								break;
							case CDATETYPE:
								sprintf (col_val,"%s",DMY2YMD( *(long *) state -> columns[pos].data));
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								break;
							case CLONGTYPE:
								sprintf (col_val,"%ld", *(long *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
								break;
								
							case CCHARTYPE:
								sprintf (col_val,"%s", state -> columns[pos].data);
								sprintf (tmp_val,"%*s",
									 state ->  view [state -> indexview[ctr]].vwlen,
									 col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CCHARTYPE [%s]", col_val);
								break;
								
							case CDOUBLETYPE:	
								sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CDOUBLETYPE [%s]",col_val);
								break;
								
							case CFLOATTYPE:
								sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\n BuildStatement CFLOATTYPE [%s]",  col_val);
								break;
						}	
					}
					strcat (sql_str, "') ");
				}
				else
				{
					if (state -> indexc > 1)
					{
						strcat (sql_str, " where (");
						pos = state -> indexactual [0];
						if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CLONGTYPE)
						{
							sprintf (col_name, "%s = %ld) and (", 
								state ->  view [state -> indexview[0]].vwname,
								(*(long *) state -> columns[pos].data)-1L);
							strcat (sql_str, col_name);
						}					
					}

					for (ctr = 0; ctr < index_cnt; ctr++)
					{
						pos = state -> indexactual[ctr];
						if ((state -> columns[pos].data_type & SQLTYPE) == SQLCHAR)
						{
							sprintf (col_name, " RPAD (%s,%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								state ->  view [state -> indexview[ctr]].vwlen,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}else if ((state -> columns[pos].data_type & SQLTYPE) == SQLDATE)
						{
							sprintf (col_name, " LPAD (TO_CHAR(%s,\"%%Y%%m%%d\"),%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								11,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}else 
						{
							sprintf (col_name, " LPAD (%s,%d,' ') %s ",
								state ->  view [state -> indexview[ctr]].vwname,
								11,
								(ctr + 1 == index_cnt) ? " ": " || ");
						}
						
						
						strcat (sql_str, col_name);
						strcat (sql_str, " ");
					}/*for*/
					
					strcat (sql_str, " ");
					strcat(sql_str,GetOperator(mode,(mode != LT) ? TRUE:FALSE , state));
					strcat (sql_str, " '");
					
					for (ctr = 0; ctr < index_cnt; ctr++)
					{
						pos = state -> indexactual[ctr];
						switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
						{
							case CINTTYPE:
								sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\n BuildStatement CINTTYPE [%s]",  col_val);
								break;
							case CDATETYPE:
								sprintf (col_val,"%s",DMY2YMD( *(long *) state -> columns[pos].data));
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								break;
							case CLONGTYPE:
								sprintf (col_val,"%ld", *(long *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
								break;
								
							case CCHARTYPE:
								sprintf (col_val,"%s", state -> columns[pos].data);
								sprintf (tmp_val,"%*s",
									 state ->  view [state -> indexview[ctr]].vwlen,
									 col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CCHARTYPE [%s]", col_val);
								break;
								
							case CDOUBLETYPE:	
								sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\nBuildStatement CDOUBLETYPE [%s]",col_val);
								break;
								
							case CFLOATTYPE:
								sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
								sprintf (tmp_val,"%*s", 11, col_val);
								strcat(sql_str,tmp_val);
								//fprintf (stdout,"\n BuildStatement CFLOATTYPE [%s]",  col_val);
								break;
						}	
					}
					strcat (sql_str, "') ");
				}
			}
			else
			{
				/*check the value of the first index column*/
				pos = state -> indexactual[0];
				switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
				{
					case CINTTYPE:
						if ( *(int *) state -> columns[pos].data != 0)
							idx_val_found = FALSE;
						break;
					case CDATETYPE:
					case CLONGTYPE:
						//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
						if (*(long *) state -> columns[pos].data != 0L)
							idx_val_found = FALSE;
						break;
						
					case CCHARTYPE:
						if (strlen(clip(state -> columns[pos].data)) != 0)
							idx_val_found = FALSE;
						break;
						
					case CDOUBLETYPE:	
						//sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
						if (twodec(*(double *) state -> columns[pos].data) != 0.00)
							idx_val_found = FALSE;
						break;
						
					case CFLOATTYPE:
						//sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
						//fprintf (stdout,"\n BuildStatement CFLOATTYPE [%s]",  col_val);
						if (twodec(*(float *)  state -> columns[pos].data ) != 0.00)
							idx_val_found = FALSE;	 	
						break;
				}	
				
				
				/*search for the next part of the index*/
				if (!idx_val_found)
				{
					for (ctr = 1; ctr < state -> indexc; ctr++)
					{
							pos = state -> indexactual[ctr];
							switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
							{
								case CINTTYPE:
									if ( *(int *) state -> columns[pos].data == 0)
										gteq_spec = TRUE;
									break;
								case CDATETYPE:
								case CLONGTYPE:
									//fprintf (stdout,"\nBuildStatement CLONGTYPE [%s]",  col_val);
									if (*(long *) state -> columns[pos].data == 0L)
										gteq_spec = TRUE;
									break;
									
								case CCHARTYPE:
									if (strlen(state -> columns[pos].data) == 0)
										gteq_spec = TRUE;
									break;
									
								case CDOUBLETYPE:	
									//sprintf (col_val,"%g", *(double *) state -> columns[pos].data);
									if (twodec(*(double *) state -> columns[pos].data) == 0.00)
										gteq_spec = TRUE;
									break;
									
								case CFLOATTYPE:
									//sprintf (col_val,"%f", *(float *)  state -> columns[pos].data);
									//fprintf (stdout,"\n BuildStatement CFLOATTYPE [%s]",  col_val);
									if (twodec(*(float *)  state -> columns[pos].data ) == 0.00)
										gteq_spec = TRUE;
									break;
							}	
							
							/*if (gteq_spec)
								fprintf (stdout,"\n BuildStatement TEST GTEQ SPECIAL colname [%s]",  state -> columns [pos].name);*/
					}
				}
				
				if (mode == GTEQ && gteq_spec)
				{
					/*create the where clause for special GTEQ*/
					pos = state -> indexactual[0];
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
					
					//fprintf (stdout,"\n BuildStatement GTEQ SPECIAL col name [%s] col val [%s]",state ->  columns[pos].name,  col_val);
					sprintf (col_name, " where %s = %s", 
							state ->  columns[pos].name,							
							col_val);
						strcat (sql_str, col_name);
				}
				else if (mode == LAST)
				{
					pos = state -> indexactual[0];
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
					//fprintf (stdout,"\n BuildStatement LAST SPECIAL col name [%s] col val [%s]",state ->  columns[pos].name,  col_val);
					sprintf (col_name, " where %s <= %s", 
							state ->  columns[pos].name,							
							col_val);
						strcat (sql_str, col_name);
				}
				else
				{
					for (ctr = 0; ctr < index_cnt; ctr++)
					{
						pos = state -> indexactual[ctr];
						switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
						{
							case CINTTYPE:
								sprintf (col_val,"%d",  *(int *) state -> columns[pos].data);
								//fprintf (stdout,"\n BuildStatement CINTTYPE [%s]",  col_val);
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
						
						//fprintf (stdout,"\n BuildStatement Ordinary col name [%s] col val [%s]",state ->  columns[pos].name,  col_val);
						
						sprintf (col_name, "%s %s %s %s", 
							ctr ?  " and " : " where " ,
							state ->  view [state -> indexview[ctr]].vwname,
							GetOperator(mode, (ctr + 1 == index_cnt) ? TRUE : FALSE, state),
							col_val);
						strcat (sql_str, col_name);
					}/*for */
				}/*if (mode ==GTEQ)*/
				
			}/*if (mode == LT || mode == LTEQ)*/
		}/*if*/
		
		/*order by*/
		strcat (sql_str," order by ");
		
		if (mode == LT || mode == LTEQ || 
			mode == PREVIOUS || mode == LAST)
		{
			for (ctr = 0; ctr < state -> indexc; ctr++)
			{
				sprintf (col_name, " %s desc%s", 
					state ->  view [state -> indexview[ctr]].vwname,
					(ctr + 1 == state -> indexc) ? " " : ",");
				strcat (sql_str,col_name);			
			}
		}
		else
		{
			for (ctr = 0; ctr < state -> indexc; ctr++)
			{
				sprintf (col_name, " %s %s", 
					state ->  view [state -> indexview[ctr]].vwname,
					(ctr + 1 == state -> indexc) ? " " : ",");
				strcat (sql_str,col_name);			
			}
		}
	}/*if */
	//fprintf (stdout,"\nQUERYSETUP 3 sql_str [%s] length [%d]\n\n\n\n", sql_str, strlen (sql_str));
	//fprintf (stdout,"\nBuildStatement 1 ");
	if (state -> q_stmt != NULL)
	{
		//fprintf (stdout,"\nQuerySetup dropprepared!!!");
		//fprintf (stdout,"\nBuildStatement 2 ");
		if (mi_drop_prepared_statement (state -> q_stmt) == MI_ERROR)
		{
			//fprintf (stdout,"\nBuildStatement 4 ");
			ids_dbase_err2("Error in query:QuerySetup:mi_drop_prepared_statement");	
		}
		state -> q_stmt = NULL;
		//fprintf (stdout,"\nBuildStatement 5 ");
	}

	//fprintf (stdout,"\nBuildStatement 6 ");
	//fprintf (stdout,"\n\r[%s]\n\r", sql_str);
	state -> q_stmt  = mi_prepare (_ids_conn, sql_str , NULL);
	//fprintf (stdout,"\nBuildStatement 7 ");
	if (state -> q_stmt == NULL)
	{
		//fprintf (stdout,"\n\nBuildStatement 8\n\n\n\n\n\n\n ");
		sprintf (tmp_val, "Error in query:QuerySetup:mi_prepare [_ids_conn = (%s)]", (_ids_conn) ? "NOT NULL":"NULL");
		ids_dbase_err2 (tmp_val);
		//ids_dbase_err2("Error in query:QuerySetup:mi_prepare");	
	}

	return (0);	
}

char *GetOperator (int mode, int lastcol, TableState * state)
{
	static char
		*Eq	= "=",
		*Gt	= ">",
		*GtEq	= ">=",
		*Lt	= "<",
		*LtEq	= "<=";
	
	if (mode == PREVIOUS && state -> lastactualfetchmode == GTEQ)
		return Lt;
	
	
		
	switch (mode)
	{
		case 	EQUAL:
		case 	COMPARISON:
				return Eq;
		case 	GTEQ:
				return GtEq;
		case 	GREATER:
				return lastcol ? GtEq : Gt;
		case 	LT:
			if (mode == LT && state -> lastactualfetchmode == GTEQ)
				return Lt;
				
			return lastcol ? LtEq : Lt;
		case 	LTEQ:
				return LtEq;
		default:
			ids_dbase_err2 ("Error in query:GetOperator Invalid comparison mode");
	}
	
	return (NULL);
}


int
BuildInputVars (
	TableState * state)
{
	int ctr = 0,
	    pos = 0;
	
	if (!state -> indexc)
	{
		/*Table has no index define so 
		set input parameters to NULL*/	
		state -> values = NULL;
		state -> types = NULL;
		state -> lengths = NULL;
		state -> nulls	= NULL;
		return (0);
	}

	
	/*Alocate memory for input parameters based on index count*/
	state -> values  = malloc (state -> indexc * sizeof (MI_DATUM));
	state -> types   = malloc (state -> indexc * sizeof (mi_string));
	state -> lengths = malloc (state -> indexc * sizeof (mi_integer));
	state -> nulls   = malloc (state -> indexc * sizeof (mi_integer));
	
	/*get the field values from ColumnDef*/
	for (ctr = 0; ctr < state -> indexc; ctr ++)
	{
		pos = state -> indexactual[ctr];
		
		state -> values[ctr] = state -> columns[pos].data;
		state -> types [ctr] =  GetColDataType (state -> columns + pos);
		state -> lengths [ctr] = 0; 
		state -> nulls [ctr] = MI_FALSE;
		
		if (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE) == CCHARTYPE)
			state -> values[ctr] =  mi_string_to_lvarchar(state -> values[ctr]);
		
		//fprintf (stdout,"\n BindInputVARS ctr [%d] types [%s] lengths [%ld] col name [%s]", 
		//		ctr, state -> types [ctr], state -> lengths [ctr], state -> columns[pos].name);
		
		/*switch (ConvSQL2C (state -> columns[pos].data_type & SQLTYPE))
		{
			
			case CINTTYPE:
				fprintf (stdout,"\nBindInputVARS CINTTYPE [%d]",  *(int *) state -> values[ctr]);
				break;
			case CDATETYPE:
			case CLONGTYPE:
				fprintf (stdout,"\nBindInputVARS CLONGTYPE [%ld]",  *(long *) state -> values[ctr]);
				break;
				
			case CCHARTYPE:
				fprintf (stdout,"\nBindInputVARS CCHARTYPE [%s]", (char *) mi_lvarchar_to_string(state -> values[ctr]));
				break;
				
			case CDOUBLETYPE:	
				fprintf (stdout,"\nBindInputVARS CDOUBLETYPE [%g]",  *(double *) state -> values[ctr]);
				break;
				
			case CFLOATTYPE:
				fprintf (stdout,"\nBindInputVARS CFLOATTYPE [%f]",  *(float *) state -> values[ctr]);
				break;
		}*/
	}
			
	return (0);		
}

int
QueryFetch (
	TableState * state,
	void * buffer,
	char locktype,
	int mode)
{
	
	int num_cols 	= 0,
	    ctr 	= 0;
	    
	mi_integer loc_err 	= 0,
		   collen 	= 0;
	
	mi_string *colval;
	
	MI_ROW_DESC * _ids_row_desc_qry;
	MI_ROW 	    * _ides_row_qry;
	
	/*store user data for use in genaccess */
	
	//fprintf (stdout,"\nQueryFetched 1");
	if (mi_fetch_statement(state -> q_stmt, GetCursorAction(mode, state -> lastfetchmode, state), 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in query:QueryFetch:mi_fetch_statement");
	//fprintf (stdout,"\nQueryFetched 2");
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in query:QueryFetch:mi_get_result");
	}	
	//fprintf (stdout,"\nQueryFetched 3");
	_ids_row_desc_qry = mi_get_statement_row_desc(state -> q_stmt);
	//fprintf (stdout,"\nQueryFetched 4");
	num_cols = mi_column_count(_ids_row_desc_qry);
		//fprintf (stdout,"\nQueryFetched 5");	
	_ides_row_qry = mi_next_row (_ids_conn, &loc_err);
		//fprintf (stdout,"\nQueryFetched 5");
	if (_ides_row_qry == NULL)
	{
		/*No records found !!! */
		//fprintf (stdout,"\nQueryFetched 1 Record not found!!!");
		return (1);		
	}
		//fprintf (stdout,"\nQueryFetched 6");
		//fflush (stdout);
	for (ctr = 0; ctr < num_cols; ctr ++)
	{
		//fprintf (stdout,"\nQueryFetched 7");
		collen = 0;
		switch(mi_value(_ides_row_qry, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2("query:QueryFetched:mi_value");
				return(MI_ERROR);
			case MI_NULL_VALUE:
				colval = NULL;
				GetDataDB(state -> columns + state -> viewactual [ctr],
					  state -> view [ctr].vwstart, colval, buffer);
				break;
			case MI_NORMAL_VALUE:
				/*Get the value of the column*/
				if (ctr == num_cols - 1)
				{
					state -> rowid = atol(colval);
					//fprintf (stdout,"\nQueryFetched 1.1 rowid [%ld]", state -> rowid);
				}
				else
					GetDataDB(state -> columns + state -> viewactual [ctr],
						  state -> view [ctr].vwstart, colval, buffer);
				break;
			default:
				ids_dbase_err2("query:QueryFetched:mi_value");
				return(MI_ERROR);
		}
	}
	
	//fprintf (stdout,"\nQueryFetched 2 Record  found!!!");
	state -> data = buffer;
	//if (locktype == 'w' || locktype == 'u')
	//{
        //	return !_TryLock (locktype, state, &state->locks);
        //}
        	
	return (0);
	
}

MI_CURSOR_ACTION
GetCursorAction 
(int mode,
 int lastmode,
 TableState * state)
{
		
	switch (mode)
	{
		case GTEQ:
		case GREATER:
		case EQUAL:
		case COMPARISON:
		case FIRST:
			return (MI_CURSOR_FIRST);
		case LTEQ:
		case LT:
		case LAST:
			//return (MI_CURSOR_LAST);
			return (MI_CURSOR_NEXT);
		case NEXT:
			return (MI_CURSOR_NEXT);
			/*
			if (lastmode == LT || lastmode == LTEQ) 
				return (MI_CURSOR_PRIOR);
			else
				return (MI_CURSOR_NEXT);
			*/
		
		case PREVIOUS:		
			return (MI_CURSOR_NEXT);
			
			
			/*
			if (state -> lastactualfetchmode == GTEQ)
				return (MI_CURSOR_LAST);	
							
			if ((lastmode == LT || lastmode == LTEQ) &&
			    !state -> gteq_called) 		
				return (MI_CURSOR_NEXT);
			else
			{
				fprintf (stdout, "\n\rmi_cursor_prior\n\r");
				return (MI_CURSOR_PRIOR);
			}
			*/
				
		case CURRENT:
			return(MI_CURSOR_RELATIVE);
	}
	return (NULL);
}

void
CleanUp()
{
	/*Clean up the mess */
	
	
	
	if (sql_str)
		free (sql_str);
}

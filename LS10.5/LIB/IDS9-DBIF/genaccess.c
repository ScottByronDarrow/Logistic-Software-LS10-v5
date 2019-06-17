/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: genaccess.c,v 1.1 2002/11/11 02:44:05 cha Exp $
|  Program Name  : (gen-access.c)
|  Program Desc  : (Generic Access Interface)
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
#include	<ProtosIF.h>

/*Local Variables*/
                                               
/*                                             
 *	Some magic numbers                     
 */                                            
#define	NAMEBUF_LEN	32			
#define	MAXKEYS		16			
                                               
/*                                             
 *	Wrapper structures for catalog         
 */                                            
struct _IndexColInfo				
{                                              
	struct IndexInfo index;                
	int cols [MAXKEYS];                    
};                                             
typedef struct _IndexColInfo IndexColInfo;     

struct _ColumnColInfo				
{                                              
	struct ColumnInfo column;                
	int colno;                    
};                                             

typedef struct _ColumnColInfo ColumnColInfo;   
                                              
struct _TableInfoList                          
{                                              
	struct TableInfo info;                 
	struct ColumnInfo * columns;
	IndexColInfo * indexes;    
	long   tabid;
	int *colnos;            
	struct _TableInfoList * next;          
};                                             
typedef struct _TableInfoList TableInfoList;   


TableInfoList * catalog = NULL;
int tablecount = 0;


                                               
/*                                             
 *	Local functions                        
 */                                            
 
enum ColumnType ConvSQL2CT (int);
 
static void AddTableInfoList (TableInfoList **, TableInfoList *),
			DestroyTableInfoList (TableInfoList **);
static TableInfoList * TableInfoListNode (const char *, int, int, long);
static TableInfoList * LocateTableInfoByNo (int),* LocateTableInfoByName (const char *);

static int PrepColCat (long, char *);	/*setup ColumnInfo */
static int PrepIdxCat (long, char *);	/*setup IndexInfo */
static int GetColcount(long ,const char *);
static int GetIdxcount(long ,const char *);


int 
TableCount ()
{
	MI_STATEMENT 	* t_stmt = NULL;
	MI_ROW 		*ids_loc_row_t = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_t = NULL;
	
	int 		num_cols = 0,
			ctr 	 = 0,
			table_count = 0,
			col_count = 0,
			index_count = 0;
	
	long 		tab_id = 0L;
			
	mi_integer 	loc_err, 
			collen = 0;
			
	mi_string 	*colval;
	
	char * sqltab = "select tabid, tabname from systables;",
	      table_name[256];
	
	if ( _ids_conn == NULL )
	{
		abc_dbopen ("data");	
	}	
	
	t_stmt = mi_prepare (_ids_conn, sqltab , NULL);
	
	if (t_stmt == NULL)
		ids_dbase_err2("Error in gen-access:TableCount:mi_prepare for systables");	
		
	if ( mi_open_prepared_statement(t_stmt,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in gen-access:TableCount:mi_open_prepared_statement for systables");	
	}
	
	if (mi_fetch_statement(t_stmt, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2("Error in gen-access:TableCount:mi_fetch_statement for systables");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in gen-access:TableCount:mi_get_result for systables");
	}
	
	ids_loc_rowdesc_t = mi_get_statement_row_desc(t_stmt);
	
	num_cols = mi_column_count(ids_loc_rowdesc_t);
	
	ids_loc_row_t = mi_next_row (_ids_conn, &loc_err);
	
	table_count = 0;
	
	while (ids_loc_row_t)
	{
		for (ctr = 0; ctr < num_cols; ctr ++)
		{
			collen = 0;
			col_count = 0;
			switch(mi_value(ids_loc_row_t, ctr,(MI_DATUM *)&colval, &collen))
			{
				case MI_ERROR:
					ids_dbase_err2("Error in gen-access:TableCount:mi_value");
					return(MI_ERROR);
				case MI_NULL_VALUE:
					colval = "NULL";
					//table_count = 0;
					break;
				case MI_NORMAL_VALUE:
					//table_count = atoi(colval);
					break;
				default:
					ids_dbase_err2("Error in gen-access:TableCount:mi_value");
					return(MI_ERROR);
			}
			
			switch (ctr)
			{
				case 0:
					if (!strncmp ("NULL",colval,4))
						tab_id = 0L;
					else
						tab_id = atol (colval);					
					break;
				
				case 1:
					sprintf (table_name, colval);						
					break;
			}	
		}
		
		col_count = GetColcount (tab_id, table_name);
		index_count = GetIdxcount (tab_id, table_name);
		
		AddTableInfoList (&catalog,
			TableInfoListNode (table_name, col_count, index_count, tab_id));
		
		
		
		TableInfoList * node = LocateTableInfoByName (table_name);
	
		if (!node)
			ids_dbase_err2("Error in gen-access:GetColcount:LocateTableInfoByName Bad Table");
				
		
		
		//PrepColCat (tab_id, table_name);
		//PrepIdxCat (tab_id, table_name);
		
		table_count++;
		
		ids_loc_row_t = mi_next_row (_ids_conn, &loc_err);
		if (ids_loc_row_t == NULL)
		{
			if (mi_fetch_statement(t_stmt, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
				ids_dbase_err2("Error in gen-access:TableCount:mi_fetch_statement for systables 2");
	
			if ( mi_get_result(_ids_conn) != MI_ROWS )
			{
				ids_dbase_err2("Error in gen-access:TableCount:mi_get_result for systables 2");
			}
			ids_loc_row_t = mi_next_row (_ids_conn, &loc_err);
		}
		
	}/*while*/
	
	if (mi_drop_prepared_statement (t_stmt) == MI_ERROR)
	{
		ids_dbase_err2("Error in gen-access:TableCount:mi_drop_prepared_statement");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in gen-access:TableCount:mi_query_finish for syscolumn");	
	}
	
	tablecount = table_count;
	
	return (table_count);
}





int
PrepColCat (
 long tab_id,
 char * tablename)
{
	MI_STATEMENT 	* c_stmt = NULL;
	MI_ROW 		*ids_loc_row_c = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_c = NULL;
	
	mi_integer 	loc_err, 
			collen = 0;
	
	int 		num_cols  = 0,
			ctr 	  = 0,
			col_count = 0,
			col_type  = 0,
			col_length = 0,
			col_no;
			
	mi_string 	*colval;
	
	char sql_col [256],
	     col_name [19];
	
	
	
	sprintf (sql_col,
		 "select colname, coltype, collength, colno from syscolumns where tabid = %ld",
		 tab_id);
	
	c_stmt = mi_prepare (_ids_conn, sql_col , NULL); 
	if (c_stmt == NULL)
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_prepare for syscolumns");	
		
	if ( mi_open_prepared_statement(c_stmt,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_open_prepared_statement for syscolumns");	
	}
	
	if (mi_fetch_statement(c_stmt, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_fetch_statement for syscolumns");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_get_result for syscolumns");
	}
	
	ids_loc_rowdesc_c = mi_get_statement_row_desc(c_stmt);
	
	num_cols = mi_column_count(ids_loc_rowdesc_c);
	
	ids_loc_row_c = mi_next_row (_ids_conn, &loc_err);
	
	while (ids_loc_row_c)
	{
		for (ctr = 0; ctr < num_cols; ctr ++)
		{
			collen = 0;
	
			switch(mi_value(ids_loc_row_c, ctr,(MI_DATUM *)&colval, &collen))
			{
				case MI_ERROR:
					ids_dbase_err2("Error in gen-access:PrepColCat:mi_value");
					return(MI_ERROR);
				case MI_NULL_VALUE:
					colval = "NULL";
					break;
				case MI_NORMAL_VALUE:
					break;
				default:
					ids_dbase_err2("Error in gen-access:PrepColCat:mi_value");
					return(MI_ERROR);
			}
			
			switch (ctr)
			{
				case 0:
				
					strcpy (col_name, colval);
									
					break;
				
				case 1:
					if (!strncmp ("NULL",colval,4))
						col_type  = 0L;
					else
						col_type = atoi (colval);						
					break;
				case 2:
					if (!strncmp ("NULL",colval,4))
						col_length  = 0L;
					else
						col_length = atoi (colval);						
					break;
				case 3:
					if (!strncmp ("NULL",colval,4))
						col_no  = 0L;
					else
						col_no = atoi (colval);						
					break;
			}	
		}
		
		//AddTableInfoList (&catalog,
		//	TableInfoListNode (table_name, col_count, 0, tab_id));
		
		TableInfoList * node = LocateTableInfoByName (tablename);
		
		if (!node)
			ids_dbase_err2("Error in gen-access:PrepColCat:LocateTableInfoByName Bad Table");
			
		strcpy (node -> columns[col_count].name, col_name);
		//fprintf (stdout,"\n genaccess PrepColCat colname [%s]",node -> columns[col_count].name);
		node -> columns[col_count].type = ConvSQL2CT (col_type & SQLTYPE);
		node -> columns[col_count].size = col_length;
		node -> colnos[col_count] = col_no;
		
		col_count++;
		
		
		
		ids_loc_row_c = mi_next_row (_ids_conn, &loc_err);
		if (ids_loc_row_c == NULL)
		{
			if (mi_fetch_statement(c_stmt, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
				ids_dbase_err2("Error in gen-access:PrepColCat:mi_fetch_statement for syscolumns 2");
	
			if ( mi_get_result(_ids_conn) != MI_ROWS )
			{
				ids_dbase_err2("Error in gen-access:PrepColCat:mi_get_result for syscolumns 2");
			}
			ids_loc_row_c = mi_next_row (_ids_conn, &loc_err);
		}
		 
	}/*while*/
	
	if (mi_drop_prepared_statement (c_stmt) == MI_ERROR)
	{
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_drop_prepared_statement");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in gen-access:PrepColCat:mi_query_finish for syscolumn");	
	}
	
	return (col_count);
}


int
PrepIdxCat (
 long tab_id,
 char * tablename)
{
	
	
	static MI_STATEMENT 	*_ids_stmt_desc_idx  = NULL;	/*use for sysindexes */
	static MI_ROW 		*_ids_row_idx 	     = NULL;
	static MI_ROW_DESC 	*_ids_rowdesc_idx    = NULL;
			
	int num_cols = 0,
	    ctr      = 0,
	    col_ctr  = 0,
	    indexctr = 0,
	    row_ctr  = 0,
	    parts [16];
	
	mi_integer 	loc_err, 
			collen = 0;
			
	mi_string 	*colval;    
	
	
	char sql_idx[256],
	     idx_name [32],
	     unique [2] ;
	
	sprintf (sql_idx, 
	"Select idxname, nunique, part1, part2, part3, part4, part5, part6, part7, part8, part9, part10, part11, part12, part13, part14, part15, part16 from sysindexes where tabid = %ld",tab_id);
	
	

	

	_ids_stmt_desc_idx = mi_prepare (_ids_conn, sql_idx , NULL);
	
	if (_ids_stmt_desc_idx == NULL)
	{
		ids_dbase_err2("Error in gen-access:PrepIdxCat:mi_prepare for sysindexes");	
	}
	

	/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
	
	if ( mi_open_prepared_statement(_ids_stmt_desc_idx,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in gen-access:PrepIdxCat:mi_open_prepared_statement for sysindexes");	
	}


	if (mi_fetch_statement(_ids_stmt_desc_idx, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in gen-access:PrepIdxCat:mi_fetch_statement in sysindexes");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in gen-access:PrepIdxCat:mi_get_result for sysindexes");
	} 
	
	_ids_rowdesc_idx = mi_get_statement_row_desc(_ids_stmt_desc_idx);
	
	num_cols = mi_column_count(_ids_rowdesc_idx);
	
	_ids_row_idx = mi_next_row (_ids_conn, &loc_err);
	
	//if (_ids_row_idx == NULL)
	//{
	//	sprintf (temp, "Index [%s] not found ",index_name);
	//	ids_dbase_err2(temp);
	//}
		
	while (_ids_row_idx)
	{
		/*Initialize parts columns*/
		for (ctr = 0; ctr < 16; ctr++)
		{
			parts[ctr] = 0;
		}
		for (ctr = 0; ctr < num_cols; ctr++)
		{
			collen = 0;
			switch(mi_value(_ids_row_idx, ctr,(MI_DATUM *)&colval, &collen))
			{
				case MI_ERROR:
					ids_dbase_err2(" gen-access:PrepIdxCat:mi_value");
					return(MI_ERROR);
				case MI_NULL_VALUE:
					colval = "NULL";
					parts[ctr] = 0;
					break;
				case MI_NORMAL_VALUE:
					//parts[ctr] = atoi(colval);
					break;
				default:
					ids_dbase_err2(" gen-access:PrepIdxCat:mi_value");
					return(MI_ERROR);
			}
			
			if (ctr == 0)
				sprintf (idx_name, "%s",colval);
			else if (ctr == 1)
				sprintf (unique, "%s",colval);
			else
				parts[ctr] = atoi(colval);	
			
		}
		
		indexctr = 0;
		
		/*Get the index count */
		
		
		TableInfoList * node = LocateTableInfoByName (tablename);
		
		if (!node)
			ids_dbase_err2("Error in gen-access:PrepColCat:LocateTableInfoByName Bad Table");
		
		sprintf(node -> indexes[row_ctr].index.name, idx_name);
		node -> indexes[row_ctr].index.isunique = (unique[0] == 'U') ? TRUE:FALSE;	
		
				
		/*index columns should be continous*/
		for (ctr = 0; ctr < 16; ctr ++)
		{
			if (parts[ctr] != 0)
			{
				indexctr++;
				for (col_ctr = 0; col_ctr < node -> info.ncolumn; col_ctr++)
				{
					if (parts[ctr] == node ->colnos[col_ctr])
					{
						node -> indexes [row_ctr].cols [ctr] = parts [ctr];
					}
				}
			}
		}
		
		
		node -> indexes[row_ctr].index.ncolumn = indexctr;
			
		
		row_ctr++;
		
		_ids_row_idx = mi_next_row (_ids_conn, &loc_err);
		
		if (_ids_row_idx == NULL)
		{
			if (mi_fetch_statement(_ids_stmt_desc_idx, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
				ids_dbase_err2 ("Error in gen-access:PrepIdxCat:mi_fetch_statement in sysindexes");
				
			if ( mi_get_result(_ids_conn) != MI_ROWS )
			{
				ids_dbase_err2("Error in gen-access:PrepIdxCat:mi_get_result for sysindexes");
			} 
			
			_ids_rowdesc_idx = mi_get_statement_row_desc(_ids_stmt_desc_idx);
		}
	}
	
	if (mi_drop_prepared_statement (_ids_stmt_desc_idx) == MI_ERROR)
	{
		ids_dbase_err2("Error in gen-access:PrepIdxCat:mi_drop_prepared_statement for syscolumn");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in column:ColumnSetup:mi_query_finish for syscolumn");	
	}
	
	return (0);
}


void
TableIndexInfo (
 int tableno,
 int indexno,
 struct IndexInfo * info)
 
{
	TableInfoList * node = LocateTableInfoByNo (tableno);
	
	if (tableno >= tablecount)
		ids_dbase_err2("Error in genaccess:TableIndexInfo Invalid table number");
		
	if (indexno >= node -> info.nindexes )
		ids_dbase_err2("Error in genaccess:TableIndexInfo Invalid index number");
			
	PrepIdxCat (node -> tabid, node -> info.name);
	*info = node -> indexes [indexno].index;
}

void
TableIndexColumnInfo (
 int tableno,
 int indexno,
 int colno,
 struct ColumnInfo * info)
{
	TableInfoList * node = NULL;

	if (tableno >= tablecount)
		ids_dbase_err2("Error in genaccess:TableIndexColumnInfo Invalid table number");
		
	node = LocateTableInfoByNo (tableno);
	
	if (indexno >= node -> info.nindexes ||				/* bad indexno */
		!node -> indexes ||					/* no info */
		colno >= node -> indexes [indexno].index.ncolumn)	/* bad colno */
	{
		ids_dbase_err2("Error in genaccess:TableIndexColumnInfo Invalid index/column number");
	}

	*info = node -> columns [node -> indexes [indexno].cols [colno]];
}


int
TableColumnCount (
 const char * name)
{
	
	TableInfoList * node = NULL;
	//TableState * table = LocateTable (name);
	//long loc_tabid  = 0;
	/*
	 *	Check the catalog system files for a possible listing
	 */
		
	if ((node = LocateTableInfoByName (name)))
		return node -> info.ncolumn;
	
/*	if (table)
	{
		fprintf (stdout,"\nGENACCESS TableColumnCount 1");
		loc_tabid = table->tabid;
		GetColcount(loc_tabid, name );
		AddTableInfoList (&catalog,
			TableInfoListNode (name, 
					   table -> columnc, 
					   GetIdxcount (loc_tabid, name ), 
					   loc_tabid)
				 );
		fprintf (stdout,"\nGENACCESS TableColumnCount 2");
		return table -> columnc;
	}*/

	ids_dbase_err2("Error in genaccess:TableColumnCount Invalid table");
	return 0;
}


void
TableColumnInfo (
 const char * name,
 int colno,
 struct ColumnInfo * col_info)
{
	TableInfoList *node= LocateTableInfoByName (name);
	//fprintf (stdout,"\nGENACCESS TableColumnInfo 1");
	if (node)
	{
	//	fprintf (stdout,"\nGENACCESS TableColumnInfo 2 name [%s]", node ->info.name);
		PrepColCat (node->tabid, node -> info.name);
	//	fprintf (stdout,"\nGENACCESS TableColumnInfo 2.1");
		*col_info = node -> columns[colno];
	//	fprintf (stdout,"\nGENACCESS TableColumnInfo 3");
	}
	else
	{
	//	fprintf (stdout,"\nGENACCESS TableColumnInfo 4");
		col_info = NULL;
	}
		
	
}

void
TableColumnGet (
 const char * name,
 int colno,
 void * buffer)
{
	TableState * table = LocateTable (name);
	int vw_pos 	= 0,
	    vwctr 	= 0,
	    col_found 	= FALSE;
	    
	void * locbuf;
	
	locbuf = buffer;
	
	if (!table)
		ids_dbase_err2("Error in genaccess:TableColumnGet Invalid table");
	if (colno >= table -> columnc)
		ids_dbase_err2("Error in genaccess:TableColumnGet Invalid column");
		
	
	for (vwctr = 0; vwctr < table -> viewc; vwctr++)
	{
		if (!strcmp(table -> columns[colno].name, table -> view[vwctr].vwname))
		{
			col_found = TRUE;	
			vw_pos = vwctr;
			//col_pos = ctr;
			//fprintf (stdout,"\ngenaccess TableColumnGet 0 viewname [%s]", table -> view[vw_pos].vwname);
			break;
		}
	}
	
	
	
	if (col_found)
	{
		locbuf = table -> data + table -> view [vw_pos].vwstart;
		//fprintf (stdout,"\ngenaccess TableColumnGet view FOUND !!! viewname [%s]", table -> view[vw_pos].vwname);
	}
	
	
	switch (ConvSQL2C (table -> columns[colno].data_type & SQLTYPE))
	{		
		case CINTTYPE:
			if (!col_found)
				*(int *)locbuf = 0;
			*(int *) buffer = *(int *)locbuf;
			//fprintf (stdout,"\ngenaccess TableColumnGet view 1 CINTTYPE [%d]",  *(int *)locbuf);
			break;
		case CDATETYPE:
			
		case CLONGTYPE:
			if (!col_found)
				*(long *) locbuf = 0L;
			*(long *) buffer = *(long *)locbuf;
			//fprintf (stdout,"\ngenaccess TableColumnGet view 1  CLONGTYPE [%ld]",  *(long *) locbuf);
			break;
			
		case CCHARTYPE:
			if (!col_found)
				*(char *) locbuf = '\0';
			strcpy((char *) buffer, (char *)locbuf);
			//fprintf (stdout,"\ngenaccess TableColumnGet view 1  CCHARTYPE [%s]\n\n", (char *) locbuf);
			break;			
		
		case CDOUBLETYPE:	
			if (!col_found)
				*(double *) locbuf = 0.00;
			*(double *) buffer = *(double *)locbuf;
			/*convert money*/
			if ((table -> columns[colno].data_type & SQLTYPE) == SQLMONEY)
			{
				//*(double *) col -> data= floor(*(double *) col -> data * 100.0);
				*(double *) locbuf=  *(double *) locbuf * 100;
			}
			//fprintf (stdout,"\nngenaccess TableColumnGet view 1   CDOUBLETYPE [%g]",  *(double *) locbuf);
			break;
			
		case CFLOATTYPE:
			if (!col_found)
				*(float *) locbuf = 0.00;
			*(float*) buffer = *(float *)locbuf;
			//fprintf (stdout,"\ngenaccess TableColumnGet view 1   CFLOATTYPE [%f]",  *(float *) locbuf);
			break;
	}
	
	
	
	
	
	//fprintf (stdout,"\nGENACCESS TableColumnGet2 colname [%s]",table -> columns[colno].name);
	//buffer = table -> columns[colno].data;
	//buffer = table -> data + table -> columns [colno]
	
	
}


void
TableColumnNameGet (
 const char * name,
 const char * colname,
 void * buffer)
{
	int colno;
	TableState * table = LocateTable (name);

	if (!table)
		ids_dbase_err2("Error in genaccess:TableColumnGet Invalid table");

	for (colno = 0; colno < table -> columnc; colno++)
	{
		if (!strcmp (colname, table -> columns [colno].name))
		{
			TableColumnGet (name, colno, buffer);
			return;
		}
	}

	ids_dbase_err2("Error in genaccess:TableColumnGet Invalid column");
}


int
TableNumber (
 const char * name)
{
	int i;
	TableInfoList * node;

	for (i = 0, node = catalog; node; i++, node = node -> next)
		if (!strcmp (name, node -> info.name))
			return i;

	return -1;
}


void
TableInfo (
 int tableno,
 struct TableInfo * info)
{
	char temp [128];
	

	if (tableno >= tablecount)
	{
		sprintf (temp,"TableInfo: table number %d out of range",tableno);
		ids_dbase_err2 (temp);
	}

	*info = LocateTableInfoByNo (tableno) -> info;
}


static int 
 GetColcount (
  long tab_id,
  const char * col_table)
{
	
	MI_STATEMENT 	*_ids_stmt_locdesc_colc = NULL;
	MI_ROW 		*ids_loc_row_colc = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_colc = NULL;
	
	mi_integer 	loc_err, 
			collen = 0;
	
	int 		num_cols  = 0,
			ctr 	  = 0,
			col_count = 0;
			
	mi_string 	*colval;
	
	char 	sql_colc [256];
	 	
	 	
	sprintf (sql_colc,"Select count(*) from syscolumns where tabid = %ld", tab_id);
	
	_ids_stmt_locdesc_colc = mi_prepare (_ids_conn, sql_colc , NULL);
	
	
	
	if (_ids_stmt_locdesc_colc == NULL)
	{
		ids_dbase_err2("Error in genaccess:GetColcount:mi_prepare for syscolumns column count");	
	}
	
	/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
	
	if ( mi_open_prepared_statement(_ids_stmt_locdesc_colc,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in genaccess:GetColcount:mi_open_prepared_statement for syscolumns-count");	
	}
		
	if (mi_fetch_statement(_ids_stmt_locdesc_colc, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in genaccess:GetColcount:mi_fetch_statement in syscolumns-count");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in genaccess:GetColcount:mi_get_result for syscolumns-count");
	}
	
	ids_loc_rowdesc_colc = mi_get_statement_row_desc(_ids_stmt_locdesc_colc);
	
	num_cols = mi_column_count(ids_loc_rowdesc_colc);
		
	ids_loc_row_colc = mi_next_row (_ids_conn, &loc_err);
	
	
	if (ids_loc_row_colc == NULL)
	{
		//sprintf (temp, "No columns for table [%s]", col_table);
		//ids_dbase_err2(temp);
		return (0);
	}
	
	for (ctr = 0; ctr < num_cols; ctr++)
	{
		collen = 0;
		switch(mi_value(ids_loc_row_colc, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2("genaccess:GetColcount:mi_value");
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
				ids_dbase_err2("genaccess:GetColcount:mi_value");
				return(MI_ERROR);
		}
	}
	
	
	if (col_count == 0)
	{
		/*sprintf (temp, "No columns for table [%s]", col_table);
		ids_dbase_err2(temp);*/
		return (0);
	}
		
	/*Complete the execution*/
	
	if (mi_drop_prepared_statement (_ids_stmt_locdesc_colc) == MI_ERROR)
	{
		ids_dbase_err2("Error in genaccess:GetColcount:mi_drop_prepared_statement for syscolumn-count");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in genaccess:GetColcount:mi_query_finish for syscolumn-count");	
	}
 	
 	return (col_count);
}


static int 
 GetIdxcount (
  long tab_id,
  const char * col_table)
{
	
	MI_STATEMENT 	*_ids_stmt_locdesc_colc = NULL;
	MI_ROW 		*ids_loc_row_colc = NULL;
	MI_ROW_DESC 	*ids_loc_rowdesc_colc = NULL;
	
	mi_integer 	loc_err, 
			collen = 0;
	
	int 		num_cols  = 0,
			ctr 	  = 0,
			col_count = 0;
			
	mi_string 	*colval;
	
	char 	sql_colc [256];
	 	
	 	
	sprintf (sql_colc,"Select count(*) from sysindexes where tabid = %ld", tab_id);
	
	_ids_stmt_locdesc_colc = mi_prepare (_ids_conn, sql_colc , NULL);
	
	
	
	if (_ids_stmt_locdesc_colc == NULL)
	{
		ids_dbase_err2("Error in genaccess:GetIdxcount:mi_prepare for syscolumns column count");	
	}
	
	/*mi_integer mi_open_prepared_statement(stmt_desc, control, params_are_binary,
	n_params, values, lengths, nulls, types, cursor_name, retlen, rettypes)*/
	
	if ( mi_open_prepared_statement(_ids_stmt_locdesc_colc,MI_SEND_READ + MI_SEND_SCROLL, MI_BINARY, 0, 
					NULL, NULL, NULL, NULL, NULL, 0,  NULL) == MI_ERROR )
	{
		ids_dbase_err2("Error in genaccess:GetIdxcount:mi_open_prepared_statement for syscolumns-count");	
	}
		
	if (mi_fetch_statement(_ids_stmt_locdesc_colc, MI_CURSOR_NEXT, 0, 1) == MI_ERROR)
		ids_dbase_err2 ("Error in genaccess:GetIdxcount:mi_fetch_statement in syscolumns-count");
		
	if ( mi_get_result(_ids_conn) != MI_ROWS )
	{
		ids_dbase_err2("Error in genaccess:GetIdxcount:mi_get_result for syscolumns-count");
	}
	
	ids_loc_rowdesc_colc = mi_get_statement_row_desc(_ids_stmt_locdesc_colc);
	
	num_cols = mi_column_count(ids_loc_rowdesc_colc);
		
	ids_loc_row_colc = mi_next_row (_ids_conn, &loc_err);
	
	
	if (ids_loc_row_colc == NULL)
	{
		//sprintf (temp, "No columns for table [%s]", col_table);
		//ids_dbase_err2(temp);
		return (0);
	}
	
	for (ctr = 0; ctr < num_cols; ctr++)
	{
		collen = 0;
		switch(mi_value(ids_loc_row_colc, ctr,(MI_DATUM *)&colval, &collen))
		{
			case MI_ERROR:
				ids_dbase_err2("genaccess:GetIdxcount:mi_value");
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
				ids_dbase_err2("genaccess:GetIdxcount:mi_value");
				return(MI_ERROR);
		}
	}
	
	
	if (col_count == 0)
	{
		/*sprintf (temp, "No columns for table [%s]", col_table);
		ids_dbase_err2(temp);*/
		return (0);
	}
	
	
	
	
	
	
	/*Complete the execution*/
	
	if (mi_drop_prepared_statement (_ids_stmt_locdesc_colc) == MI_ERROR)
	{
		ids_dbase_err2("Error in genaccess:GetIdxcount:mi_drop_prepared_statement for syscolumn-count");	
	}
		
	if ( mi_query_finish(_ids_conn) == MI_ERROR )
	{
		ids_dbase_err2("Error in genaccess:GetIdxcount:mi_query_finish for syscolumn-count");	
	}
 	
 	return (col_count);
}


enum ColumnType
ConvSQL2CT (
 int	sqlType)						
{                                                               
	switch (sqlType)                                        
	{                                                       
		case SQLCHAR	:                               
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLCHAR");
			return (CT_Chars);                      
	                                                        
		case SQLSMINT	:                               
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLSMINT");
			return (CT_Long);                     
	                                                        
		case SQLFLOAT	:             
			 //fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLFLOAT	");                 
			return (CT_Float);                     
	                                                        
		case SQLSMFLOAT	:        
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLSMFLOAT	");                      
			return (CT_Float);                      
	                                                        
		case SQLDECIMAL	:   
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLDECIMAL	");                      			                            
			return (CT_Money);                      
	                                                        
		case SQLMONEY	: 
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLMONEY	");                                 
			return (CT_Money);                      
	                                                        
		case SQLINT	:        
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLINT	");                         
			return (CT_Long);                       
			                                        
		case SQLSERIAL	:      
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLSERIAL	");                              
			return (CT_Serial);                     
	                                                        
		case SQLDATE	:            
			 //fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLDATE	");                        
			return (CT_Date);                       
	                                                        
		case SQLDTIME	:    
			   //fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLDTIME		");                           
			return (CT_Date);                       
	                                                        
		case SQLINTERVAL	:   
			//fprintf (stdout,"\ngeenaccess ConvSQL2CT SQLINTERVAL		");                     
			return (CT_Bad);                      
	}               
	fprintf (stdout,"\ngeenaccess ConvSQL2CT CT_Bad		");                                         
	return (CT_Bad);                                        
}                                                               


/*
 *	List
 */
static void
AddTableInfoList (
 TableInfoList ** list,
 TableInfoList * node)
{
	if (*list)
		AddTableInfoList (&(*list) -> next, node);
	else
		*list = node;
}

static void
DestroyTableInfoList (
 TableInfoList ** list)
{
	if (!*list)
		return;

	DestroyTableInfoList (&(*list) -> next);

	if ((*list) -> columns)
		free ((*list) -> columns);
	if ((*list) -> indexes)
		free ((*list) -> indexes);
	free (*list);

	*list = NULL;
}

static TableInfoList *
TableInfoListNode (
 const char * name,
 int colcount,
 int indexcount,
 long tabid)
{
	TableInfoList * node = malloc (sizeof (TableInfoList));

	memset (node, 0, sizeof (TableInfoList));

	strcpy (node -> info.name, name);
	node -> info.ncolumn = colcount;
	node -> info.nindexes = indexcount;
	node -> tabid = tabid;
	
	node -> columns = malloc (node -> info.ncolumn  * sizeof (struct ColumnInfo));
	memset (node -> columns,0, node -> info.ncolumn * sizeof (struct ColumnInfo));
	
	node -> colnos = malloc (colcount * sizeof (int));	
	memset (node -> colnos,0, colcount * sizeof (int));
	
	node -> indexes = malloc (indexcount * sizeof ( IndexColInfo));
	memset (node -> indexes,0, indexcount * sizeof ( IndexColInfo));
	
	return node;
}

static TableInfoList *
LocateTableInfoByNo (
 int tableno)
{
	TableInfoList * node = NULL;

	for (node = catalog; tableno--; node = node -> next);

	return node;
}

static TableInfoList *
LocateTableInfoByName (
 const char * name)
{
	TableInfoList * node = NULL;

	for (node = catalog; node; node = node -> next)
		if (!strcmp (name, node -> info.name))
			break;

	return node;
}


/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: data.c,v 1.1 2002/11/11 02:44:05 cha Exp $
|  Program Name  : (data.c)
|  Program Desc  : (data extraction and data type handling routines)
|---------------------------------------------------------------------|
| $Log: data.c,v $
| Revision 1.1  2002/11/11 02:44:05  cha
| Updated for GTEQ modifications.
|
=====================================================================*/

#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif 

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
//#include	<math.h>
#include	<twodec.h>

#include "ids9dbif.h"


/*Get the data from Application buffer into columns->data */
void
GetDataApp(
	ColumnDef * col,
	int appoffset,
	void * raw_buffer)
{

	switch (ConvSQL2C (col -> data_type & SQLTYPE))
	{
		case CDOUBLETYPE:	
			/*convert money*/
			if ((col -> data_type & SQLTYPE) == SQLMONEY)
			{
				//*(double *) col -> data= floor(*(double *) col -> data * 100.0);
				*(double *) col -> data=  *(double *) col -> data;
			}
			//fprintf (stdout,"\nCDOUBLETYPE [%g]",  *(double *) col -> data);
			break;
	}
	
}

/*Gets the data from Database  into application buffer */
void
GetDataDB(
	ColumnDef * col,
	int appoffset,
	mi_string * db_data,
	void * raw_buffer)
{
	
	if (db_data == NULL)
		col -> null_ind = TRUE;
	else
		col -> null_ind = FALSE;
	

	switch (ConvSQL2C (col -> data_type & SQLTYPE))
	{
		
		case CINTTYPE:
			if (col -> null_ind)
			{
				*(int *) col -> data = *(int *) 0;	
			}
			else
			{
				*(int *) col -> data =  atoi (db_data);
			}
			break;
		case CDATETYPE:
			if (col -> null_ind)
			{
				*(long *) col -> data = (long) 0;
			}
			else
			{
				*(long *) col -> data = (long) mi_string_to_date(db_data);
			}
			break;
		case CLONGTYPE:
			if (col -> null_ind)
			{
				*(long *) col -> data = (long) 0;
			}
			else
			{
				*(long *) col -> data = atol(db_data);
			}
			break;
			
		case CCHARTYPE:
			if (col -> null_ind)
			{
				col -> data = NULL;
			}
			else
			{
				strcpy (col -> data, (char *) db_data); 
			}
			break;
			
		case CDOUBLETYPE:	
			if (col -> null_ind)
			{
				*(double *) col -> data = (double) 0;
			}
			else
			{
				(double ) dectodbl(mi_string_to_money(db_data),(double *) col -> data );
				
				/*convert money*/
				if ((col -> data_type & SQLTYPE) == SQLMONEY)
				{
					//*(double *) col -> data  = *(double *) col -> data /100.0;
					*(double *) col -> data  = *(double *) col -> data;
				}
			}
			break;
			
		case CFLOATTYPE:
			if (col -> null_ind)
			{
				*(float *) col -> data = (float) 0.00;
			}
			else
			{
				dectodbl(mi_string_to_money(db_data),(double *) col -> data );
				*(float *) col -> data = *(double *) col -> data;
			}
			break;
	}
	
}



int
ConvSQL2C (
 int	sqlType)
{
	switch (sqlType)
	{
		case SQLCHAR	:
			return (CCHARTYPE);
	
		case SQLSMINT	:
			return (CINTTYPE);
	
		case SQLFLOAT	:
			return (CDOUBLETYPE);
	
		case SQLSMFLOAT	:
			return (CFLOATTYPE);
	
		case SQLDECIMAL	:
			return (CDECIMALTYPE);
	
		case SQLMONEY	:
			return (CDOUBLETYPE);
	
		case SQLINT	:
		case SQLSERIAL	:
			return (CLONGTYPE);
	
		case SQLDATE	:
			return (CDATETYPE);
	
		case SQLDTIME	:
			return (CDTIMETYPE);
	
		case SQLINTERVAL	:
			return (CINVTYPE);
	}
	return (-1);
}


mi_string * GetColDataType (
	ColumnDef * col)
{
	static  char  temp [10];
	
	switch (col -> data_type & SQLTYPE)
	{
		case SQLCHAR	:
			sprintf (temp,"char(%d)", col -> length);
			return (temp);
	
		case SQLSMINT	:
			return ("integer");
		
		case SQLDECIMAL	:
			return (NULL);	
		
		case SQLFLOAT	:
			return ("double");
	
		case SQLSMFLOAT	:
			return ("float");
		
		case SQLMONEY	:
			return ("money");
	
		case SQLINT	:
			return ("integer");
			
		case SQLSERIAL	:
			return ("serial");
	
		case SQLDATE	:
			return ("date");
	
		case SQLDTIME	:
			return (NULL);
	
		case SQLINTERVAL	:
			return (NULL);	
	}
	
	return (NULL);	
}



void
MapDataBuffer (
 	TableState * table,
 	void * raw_buffer)
{
	
	int i	= 0,	   
	    pos = 0;
		
	for (i = 0; i < table -> viewc; i++)
	{
		pos = table -> viewactual[i];
		table -> columns[pos].data = table -> view[i].vwstart + raw_buffer;
	}	
}


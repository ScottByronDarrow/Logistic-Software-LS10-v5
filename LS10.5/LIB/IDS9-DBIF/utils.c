/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: utils.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (utils.c)
|  Program Desc  : (utilities routines)
|---------------------------------------------------------------------|
| $Log: utils.c,v $
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


char *
GetColNameByColNo (TableState *table, int colno)
{
	int i = 0;
	
	if (colno == 0)		/*colno = 0 means that no column name*/
		return (NULL);
	
	for (i=0; i < table -> columnc; i++)
	{
		if (table -> columns[i].colno == colno)
		{
			return (table -> columns[i].name);
		}	
	}
	return (NULL);
}

char    *PadOut (
 char   *str,
 int    len)
{
    int i = strlen (str);

    if (i < len)
    {
        while (i < len)
            str [i++] = ' ';
        str [i] = '\0';
    }
    return (str);

}


char * 
 DMY2YMD (Date date)
 {
 	static char strdate[9];
 	int d = 0,
 	    m = 0,
 	    y = 0;
 	    
 	 
 	 DateToDMY (date, &d, &m, &y);
 	 sprintf (strdate,"%04d%02d%02d",y, m, d);
 	 return (strdate); 		
 }
 
 
 
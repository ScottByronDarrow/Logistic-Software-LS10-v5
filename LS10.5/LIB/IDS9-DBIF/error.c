/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: error.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (error.c)
|  Program Desc  : (Functions use for error messages)
|---------------------------------------------------------------------|
| $Log: error.c,v $
| Revision 1.2  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|

|
=====================================================================*/

#include "ids9dbif.h"

/*program name where the error occured*/



void
ids_dbase_err()
{
	char msg[1024];
	int  err_num;
	
	sprintf (msg,"DATABLADE DBIF ERROR SQLCODE [%s]\n\n",  _ids_error.error_msg);
	err_num = (int) _ids_error.sqlcode;
	
	sys_err (msg, err_num, PNAME);
}


void
ids_dbase_err2(char *usr_mess)
{
	char msg[1024];
	int  err_num;
	
	sprintf (msg,"DATABLADE DBIF ERROR [%s %s]\n\n", 
		usr_mess, _ids_error.error_msg);
		
	err_num = (int) _ids_error.sqlcode;
	sys_err (msg, err_num, PNAME);
}



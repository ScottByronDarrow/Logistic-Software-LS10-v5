/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: dbif.c,v 1.3 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (dbif.c)
|  Program Desc  : (Datblade Interface to LS/10)
|---------------------------------------------------------------------|
| $Log: dbif.c,v $
| Revision 1.3  2002/11/11 02:41:10  cha
| Updated for GTEQ modifications.
|

|
=====================================================================*/

/*Avoid duplicate definition for some 
functions and data structures from 
Informix and Standard C*/
#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif  

#include	<stdio.h>

#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	<dbio.h>
#include	<ProtosIF.h> 

#include	"ids9dbif.h"

char dbpath[255];
char dbserver[255];

MI_CALLBACK_HANDLE 	*_ids_cback = NULL;
MI_CALLBACK_STATUS MI_PROC_CALLBACK ids_exception_callback (MI_EVENT_TYPE	type, 
							    MI_CONNECTION	*conn,
							    void		*server_info,
							    void		*user_data);

/*===========+
| abc_dbopen |
+===========*/

int
abc_dbopen (
 	const char * dbname)
{

	/*-----------------------------------------------
	| Get connection info from the DBPATH           |
	| Get the server name from INFROMIXSERVER	|
	-----------------------------------------------*/

	sprintf (dbpath, "%s", getenv ("DBPATH"));
	
	sprintf(dbserver,"%s",getenv ("INFORMIXSERVER"));
	
	
	 _LockInitialise();
	
	
	/* Assign default connection parameter in the
	** connection-information descriptor */
	_ids_conn_info.server_name = dbserver;
	_ids_conn_info.server_port = 143;
	
	/* Set default connection parameters for the application */
	if ( mi_set_default_connection_info(&_ids_conn_info) == MI_ERROR )
	{
		sys_err("DATABLADE API: FAILED mi_set_default_connection_info()",0,PNAME);
		return (1); 
	}
	
	/* Assign default database parameters in the
	** database-information descriptor */
	_ids_db_info.user_name = NULL;
	_ids_db_info.database_name = (char *)dbname;
	_ids_db_info.password = NULL;
	
	/* Set default database parameters for the application */
	if ( mi_set_default_database_info(&_ids_db_info) == MI_ERROR )
	{
		sys_err("DATABLADE API FAILED: mi_set_default_database_info()\n",0,PNAME);
		return (1);
	}
	
	/* Get default connection and database parameters for
	** application */
	mi_get_default_connection_info(&_ids_conn_info);
	mi_get_default_database_info(&_ids_db_info);
	
	/* Make sure the right database server is set as the default */
	if ( strcmp(dbserver, _ids_conn_info.server_name) != 0 )
	{
		sys_err("DATABLADE API FAILED: got server_name % should be data",0,PNAME);
		return (1);
	}
		
	/* Connect to database server */
	_ids_conn = mi_server_connect(&_ids_conn_info);
	if ( _ids_conn == NULL )
	{
		sys_err("DATABLADE API FAILED: CONNECT to server\n", 0,PNAME);
		return (1);
	}
	else
	{
		printf("OK: connected to %s\n", _ids_conn_info.server_name);      
		/* log in to "template1" database as user "miadmin" */
		if ( mi_login(_ids_conn, &_ids_db_info) != MI_ERROR )
			printf("OK: connected to db DB %s\n",_ids_db_info.database_name);
		else
		{
			sys_err("DATABLADE API FAILED : connect to database ",0,PNAME);
			return (1);
		}
	}
	
	
		
	_ids_cback = mi_register_callback(_ids_conn, MI_All_Events, 
					  ids_exception_callback, 
					  (void *)&_ids_error, 
					  NULL);

	if (_ids_cback == NULL)
	{
		ids_dbase_err2 ("Error in dbif:abc_dbopen:mi_register_callback");
	}
	
	//mi_enable_callback(_ids_conn, MI_Exception, _ids_cback);
	return (EXIT_SUCCESS);
}

/*============+
| abc_dbclose |
+============*/

int
abc_dbclose (
 	const char * ignore)
{
	/*-----------------------------------------------
	| Close off any possible open tables, but issue |
	| 	a warning as well.                          |
	-----------------------------------------------*/

	DestroyAllTables ();
	DestroyAliasList ();

	/*-----------------------------------------------
	| Unregister any callback that was defined.	|
	-----------------------------------------------*/

	if ( (mi_unregister_callback(_ids_conn, MI_All_Events, _ids_cback)) == MI_ERROR)
	{
		sys_err("DATABLADE API FAILED : mi_unregister_callback failed. ",0,PNAME);
	}

	/*-------------
	| Logging OFF |
	-------------*/
	
	if (_ids_conn)
	{
		if( (mi_close(_ids_conn)) == MI_ERROR)
		{
			sys_err("DATABLADE API FAILED : logout from database ",0,PNAME);
		}
	}
	
	_LockCleanup();
	return (EXIT_SUCCESS);
}

int
open_rec (           
    const char * table,
    struct dbview * fields,             
    int count,                 
    const char * index)
{	
	TableState * state = AllocateStdTable (table);
	
	if (state == NULL)
		return(EXIT_FAILURE);

	state -> viewc = count;
	state -> view = fields;
	state -> fd_table = -1;
	
	IdentifyFields (state);
	IdentifyIndex (state, index);

	state -> orig_data = NULL;
	state -> stmt_processed = FALSE;
	state -> lastfetchmode = -1;
	state -> lastactualfetchmode = -1;
	state -> gteq_called = FALSE;
	
	state -> q_stmt = NULL;
	state -> u_stmt = NULL;
	state -> i_stmt = NULL;
	state -> d_stmt = NULL;
	
	return (EXIT_SUCCESS);
}


MI_CALLBACK_STATUS MI_PROC_CALLBACK
ids_exception_callback (
	MI_EVENT_TYPE	type,		/* MI_EVENT_TYPE is defined in milib.h */
	MI_CONNECTION	*conn,		/* Connection on which the error occurred */
	void		*server_info,	/* Server information */
	void		*user_data)	/* User-provided structure (optional) */
{
	DB_ERROR_BUF	*errorBuf = (DB_ERROR_BUF *) user_data;
	
	if (type != MI_Exception)  
	{
		fprintf(stderr,
		   "Warning! exception_callback() fired for event %d.", type);
	}

	
	errorBuf->level = mi_error_level(server_info);
	mi_error_sqlcode(server_info, &errorBuf->sqlcode);
	mi_error_sql_state(server_info, errorBuf->sqlstate, 6);
	mi_errmsg(server_info, errorBuf->error_msg, 256);

	return MI_CB_CONTINUE;
}

/*===========+
| abc_fclose |
+===========*/

void
abc_fclose (
	const char * table)
{
	DestroyTable (table);
}


/*===========+
| abc_unlock |
+===========*/

int
abc_unlock (
	const char * table)
{
	TableState * state = LocateTable (table);
	
	_LockFreeAll (state, &state -> locks);
	
	return 0;
}

/*=========+
| find_rec |
+==========*/

int
find_rec (
	const char * table,
	void * buffer,
	int mode,
	const char * lock)
{
	int rec_found = 0;
	TableState * state = LocateTable (table);
	MapDataBuffer ( state, buffer); 	
 	
	switch (mode)
	{
		case GREATER:
		case GTEQ:
		case LT:
		case LTEQ:
		case EQUAL:
		case COMPARISON:
			state -> lastfetchmode = mode;
			state -> stmt_processed = FALSE;
			/* no breaks!!!! */
		case NEXT:
		case PREVIOUS:
		case FIRST:
		case LAST:
		case CURRENT:			
			QuerySetup (state, mode, buffer);
			rec_found = QueryFetch (state, buffer, lock[0], mode);
	}
	
	
	state->lastactualfetchmode = mode;	
	
	if (rec_found == 1)
		return (1);
	
	return (EXIT_SUCCESS);	
}

/*=============+
| abc_selfield |
+=============*/

int
abc_selfield (
	const char * table,
	const char * new_index)
{
	TableState * state = LocateTable (table);

	IdentifyIndex (state, new_index);
	
	/*Reset flags*/
	state -> stmt_processed = FALSE; /*force rebuild of SQL statement*/
	state -> lastfetchmode = -1;
	state -> lastactualfetchmode = -1;
	state -> gteq_called = FALSE;
	
	return 0;
}


/*==========+
| find_hash |
+==========*/
int
find_hash (
	const char * table,
	void * buffer,
	int mode,
	const char * lock,
	long hash)
{
	char msg[100];
	
	int ctr	  = 0,
	    pos	  = 0;
	
	    
	TableState * state = LocateTable (table);
	MapDataBuffer ( state, buffer); 
	
	state -> findhash = hash;
	/*verify if table has a serial index*/
	for (ctr = 0; ctr < state -> columnc; ctr ++)
	{
		if ((state -> columns[ctr].data_type & SQLTYPE)	== SQLSERIAL)
		{
			pos = ctr;	
			break;
		}
	}
	
	//fprintf (stdout,"\nDBIF FIND_HASH 1 pos [%d] name [%s]",pos, state -> columns[pos].name);
	
	if (ctr >= state -> columnc)
	{
		//fprintf (stdout,"\nDBIF FIND_HASH 2 pos [%d] name [%s]",pos, state -> columns[pos].name);
		/*sprintf (msg,"Table %s has no Serial Index",table);
		ids_dbase_err2 (msg);*/
		if (state -> indexc != 1)
		{
			sprintf (msg,"Table %s has no Serial Index",table);
			ids_dbase_err2 (msg);
		}
		if ((state -> columns[state -> indexactual [0]].data_type & SQLTYPE) != SQLINT)
		{
			sprintf (msg,"Table %s has no hash Index",table);
			ids_dbase_err2 (msg);
		}
		
	}
	
	if (state -> indexc == 1)
		pos = state -> indexactual [0];
	
	//fprintf (stdout,"\nDBIF FIND_HASH 3 pos [%d] name [%s]index cnt [%d]",
	//	pos, state -> columns[pos].name, state -> indexc);
	
	/*check dbview if the serial field is included*/
	
	for (ctr = 0; ctr < state -> viewc; ctr++)
	{	
		if (!strcmp (state -> columns[pos].name, state -> view [ctr].vwname))
		{
			//fprintf (stdout,"\nDBIF FIND_HASH 4 vwname [%s] name [%s]\n\n\n",
			//		state -> view [ctr].vwname,state -> columns[pos].name);
			
			*(long *) (state -> columns[pos].data) =(long) hash;		
			//fprintf (stdout,"\nDBIF FIND_HASH 4 vwname [%s] val[%ld] hash[%ld]",
			//		state -> view [ctr].vwname,
			//		*(long *) state -> columns[pos].data, hash);
			break;
		}
	}
	
	//fprintf (stdout,"\nDBIF FIND_HASH 4 pos [%d] name [%s]",pos, state -> columns[pos].name);
	if  (ctr >= state -> viewc)
	{
		sprintf (msg,"Index %s is not in DBVIEW",state -> columns[pos].name);
		ids_dbase_err2 (msg);
	}
	
	
	
	if ((mode == FIRST && state -> lastactualfetchmode == -1) || 
		(mode == LAST && state -> lastactualfetchmode == -1) ||
		(mode == GTEQ || mode == GREATER) ||
		(mode == LTEQ || mode == LT) || 
		(mode == EQUAL || mode == COMPARISON))
	{
		/*Rebuild index if the above conditions are met*/
		IdentifyIndex (state, state -> columns[pos].name);
	}
	return (find_rec (table, buffer, mode, lock));
	
}


int
abc_add (
	const char * table,
	void * recbuf)
{
	int resultcode;
	TableState * state = LocateTable (table);
	MapDataBuffer ( state, recbuf); 	
	resultcode = _InsertRow (state, recbuf);
	abc_unlock(table);

	if (resultcode)
		return 1;

	return 0;
}



/*===========+
| abc_update |
+===========*/

int
abc_update (
    const char * table,
    void * recbuf)
{
	int resultcode;
	
	resultcode = _UpdateRow(LocateTable (table), recbuf);
	
	abc_unlock (table);
	
	if (resultcode)
		return 1;
	
	return (0);
}

/*===========+
| abc_delete |
+===========*/

int
abc_delete (
 const char * table)
{
	_DeleteRow (LocateTable (table));
	return (0);	
}

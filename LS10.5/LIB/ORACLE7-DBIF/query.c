#ident	"$Id: query.c,v 5.1 2001/06/21 08:01:01 cha Exp $"
/*
 *	Query related functions
 *
 *******************************************************************************
 *	$Log: query.c,v $
 *	Revision 5.1  2001/06/21 08:01:01  cha
 *	Updated to handle correctly the Money datatype.
 *	
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:53  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.3  2000/08/02 02:34:59  raymund
 *	Small performance improvements. Added codes for locked find_hash()es.
 *	
 *	Revision 2.2  2000/07/28 06:10:08  raymund
 *	Implemented CURRENT in find_rec. Provided a patch for bug in disp_srch().
 *	
 *	Revision 2.1  2000/07/26 10:09:56  raymund
 *	Furnished missing functionalities. Use SQL for row locking.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.3  1999/11/15 02:53:06  jonc
 *	Added lock code. Requires `alvin' the lock-daemon to be running.
 *	
 *	Revision 1.2  1999/11/01 21:22:25  jonc
 *	Fixed: sorted output.
 *	Added: support for FIRST
 *	
 *	Revision 1.1  1999/10/21 21:47:05  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <signal.h>
#include    <unistd.h>

#include	<dbio.h>

#include	"oracledbif.h"
#include	"oraerrs.h"
//#include 	"log.h"

#define	NAMELEN 128

#define EOI 2025
extern int restart;
extern int foreground (), getkey ();
extern void print_mess (char *),
            clear_mess();

/*
 *	Local functions
 */
static void BuildStatement (Lda_Def *, TableState * table, int mode, int),
			BindInputVariables (Lda_Def *, TableState * table),
			BindOutputVariables (Lda_Def *, TableState * table);
static const char * ModeToString (int mode, int islastcol);


/*
 *	External interface
 */
int
QuerySetup (
 Lda_Def * lda,
 TableState * table,
 int mode,
 int abs_posn,
 void * buffer)
{
	int i;

	if (mode != table -> lastfetchmode || !table -> query)
	{
		/*
		 *	Build statement and bind input/output
		 */
        BuildStatement (lda, table, mode, abs_posn );
        if ( !abs_posn )
        	BindInputVariables (lda, table);
        BindOutputVariables (lda, table);
	}

	/*
	 *	Convert the index values from the application buffer
	 *	to the internal buffer
	 */
	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = table -> columns + table -> indexactual [i];

		column -> null_ind = 0;
		_ConvApp2Raw (
			column,
			table -> view [table -> indexview [i]].vwstart,
			buffer);
			/*printf("\nQuerySetup: \nindex viewstart: %d \nIndex Name: %s",table -> view [table -> indexview [i]].vwstart, table -> view [table -> indexview[i]].vwname);*/
		
	}


	/*
	 *	Attempt execution
	 */
	if (oexec (&table -> q_cursor))
	   oraclecda_error( lda, &table -> q_cursor, "QuerySetup");
	
	table -> lastfetchmode = mode;
	return TRUE;
}

int
QueryFetch (
 Lda_Def * lda,
 TableState * table,
 void * buffer,
 char locktype)
{
	int i;

char msg[200];

	if (ofetch (&table -> q_cursor))
	{
		
		if (table -> q_cursor.rc == ERR_NO_DATA_FOUND)
			return FALSE;
                sprintf(msg, "QueryFetch (%s) (%s)", table->named, table->table );
	        oraclecda_error (lda, &table -> q_cursor, msg); 
	}
/*
	for (i = 0; i < table -> columnc; i++)
	{
		printf("\nColumn Name: %s", table -> columns [i].name0);

		switch (table -> columns[i].type)
		{
			case OT_Double : printf(" Double: %g ",* (double *) table ->columns [i].data);
					 break;
			case OT_Float : printf(" Float %f ",* (float *) table ->columns [i].data);
					break;
			case OT_Chars : printf(" Chars: %s ", table -> columns [i].data);
					break;
			case OT_Number : printf	(" NUMBER : %ld ", *(long *) table ->columns[i].data);
					 break;
			case OT_Money : printf(" Money : %f ", *(float *) table -> columns[i].data);
					break;

		}
		}
*/


	/*
	 *	Map output to application buffers
	 */
	for (i = 0; i < table -> viewc; i++)
	{
		_ConvRaw2App (
			table -> columns + table -> viewactual [i],
			table -> view [i].vwstart,
			buffer );
		/*	printf("\nColumn Name : %s\ncount: %d\nViewStart : %di\n", table -> columns[table -> viewactual [i]].name, i, table -> view[i].vwstart);
*/	}

    if ( locktype == 'w' || locktype == 'u' )
	   return _TryLock( locktype, table, &table->locks );
	return TRUE;
}

int
QueryFetchPrevious (
 Lda_Def * lda,
 TableState * table,
 void * buffer,
 int record_num, 
 char locktype)
{
   int  current_record = 0,
	    i;

   if ( record_num <= 0 )
	  return FALSE;

   for (i = 0; i < table -> indexc; i++)
   {
	 ColumnDef * column = table -> columns + table -> indexactual [i];

	 column -> null_ind = 0;
	 _ConvApp2Raw (
			column,
			table -> view [table -> indexview [i]].vwstart,
			buffer);
   }
   if (oexec (&table -> q_cursor) )
	  oraclecda_error (lda, &table -> q_cursor, "QueryFetchPrevious");

   while ( current_record < record_num )
   { 
	 if (ofetch (&table -> q_cursor))
	 {
		if (table -> q_cursor.rc == ERR_NO_DATA_FOUND)
			return FALSE;
		oraclecda_error (lda, &table -> q_cursor, "QueryFetchPrevious");
	 }
     current_record++;
   } 

   for (i = 0; i < table -> viewc; i++)
   {
		_ConvRaw2App (
			table -> columns + table -> viewactual [i],
			table -> view [i].vwstart,
			buffer );
   }

   if ( current_record == record_num )
   {
     if ( locktype == 'w' || locktype == 'u' )
	    return _TryLock( locktype, table, &table->locks );
     return TRUE;
   }
   return FALSE; /* Failure */
}


/*
 */
static void
BuildStatement (
 Lda_Def * lda,
 TableState * table,
 int mode,
 int abs_posn)
{
	/*
	 *	Build a usable SQL statement
	 */
	int i;
	char sqlstr [2048];		/* should be long enough, initially */

 /*FILE *fp; */


	/*
	 *	As much as i'd like to do a:
	 *
	 *		select *, rowid from table
	 *
	 *	ORACLE's parser will not allow this, so I need to
	 *	append all the name individually
	 */
	strcpy (sqlstr, "select ");
	for (i = 0; i < table -> columnc; i++)
	{
		sprintf (sqlstr + strlen (sqlstr),
			"%s%s ",
			table -> columns [i].name,
			i + 1 < table -> columnc ? "," : "");
	}
	sprintf (sqlstr + strlen (sqlstr),
		"from %s",
		table -> table ? table -> table : table -> named);

	if (table -> indexc)
	{
		/*
		 *	Only build where conditions if an absolute position
		 *	(eg FIRST/LAST) is not required
		 */
		if (!abs_posn)
		{
			/*
			 *	Where conditions are based on the index selected
			 */
			for (i = 0; i < table -> indexc; i++)
			{
				sprintf (sqlstr + strlen (sqlstr),
					" %s %s %s :%d",
					i ? "and" : "where",
					table -> columns [table -> indexactual [i]].name,
					ModeToString (mode, i + 1 < table -> indexc),
					i + 1);
			}
		}

		/*
		 *	Force output sort
		 */
		strcat (sqlstr, " order by");
                if ( (abs_posn == TRUE + 1) ||  /* Find Last? */
                     (mode     == LT )      ||  /* Less than? */
                     (mode     == LTEQ) )

	   	   for (i = 0; i < table -> indexc; i++)
	  	   {
			sprintf (sqlstr + strlen (sqlstr),
				" %s desc %s",
				table -> columns [table -> indexactual [i]].name,
				i + 1 >= table -> indexc ? "" : ",");
		   }
                else 
	   	   for (i = 0; i < table -> indexc; i++)
	  	   {
			sprintf (sqlstr + strlen (sqlstr),
				" %s%s",
				table -> columns [table -> indexactual [i]].name,
				i + 1 >= table -> indexc ? "" : ",");
		   }
	}

	if (strlen (sqlstr) > sizeof (sqlstr))
	{
		/*
		 *	Best to die from possible memory corruption
		 */
		oracledbif_error ("BuildStatement. SQL statement bigger than buffer");
	}

	if (table -> query)
		free (table -> query);
	table -> query = strdup (sqlstr);


/* fp = fopen("/opt/home/lsl/QUERY.LOG","a");
fprintf(fp, "\n BuildStatement :\n %s", sqlstr); 
fflush(fp);
fclose(fp);*/


	/*
	 *	Attempt the parse
	 */
	if (oparse (&table -> q_cursor, table -> query, -1, PARSE_NOW, PARSE_V7))
		oraclecda_error (lda, &table -> q_cursor, "BuildStatement");

}

static void
BindInputVariables (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Inform ORACLE about input parameters
	 *
	 *	Our selection criteria is based around the index,
	 *	so we only bind columns associated with the index.
	 */
	int i;

	for (i = 0; i < table -> indexc; i++)
	{
		ColumnDef * column = &table -> columns [table -> indexactual [i]];
		int sql_input = _internal_sqltype (column -> type);

		if (obndrn (&table -> q_cursor, i + 1,
				(ub1 *) column -> data, column -> length, sql_input,
				-1,
				NULL, 
				NULL, -1, -1))
		{
			oraclecda_error (lda, &table -> q_cursor, "BindInputVars");
		}
	}
}

static void
BindOutputVariables (
 Lda_Def * lda,
 TableState * table)
{
	/*
	 *	Inform ORACLE about output buffers for the query
	 */
	int i;
	static ub2	r_len, r_code;		/* discard */

	for (i = 0; i < table -> columnc; i++)
	{
		if (odefin (&table -> q_cursor, i + 1,
				(ub1 *) table -> columns [i].data,
				table -> columns [i].length,
				_internal_sqltype (table -> columns [i].type),
				-1,
				&table -> columns [i].null_ind,
				NULL, -1, -1,
				&r_len, &r_code))
		{
			oraclecda_error (lda, &table -> q_cursor, "BindOuputVars");
		}
	}
}

/*
 */
static const char *
ModeToString (
 int mode,
 int islastcol)
{
	static char
		*Eq		= "=",
		*Gt		= ">",
		*GtEq	= ">=",
		*Lt		= "<",
		*LtEq	= "<=";

	switch (mode)
	{
	case EQUAL:
	case COMPARISON:
		return Eq;

	case GTEQ:
		return GtEq;

	case GREATER:
		return islastcol ? Gt : GtEq;

	case LT:
		return islastcol ? Lt : LtEq;

	case LTEQ:
		return LtEq;
	}

	oracledbif_error ("Bad comparison mode: %d", mode);
	return NULL;
}

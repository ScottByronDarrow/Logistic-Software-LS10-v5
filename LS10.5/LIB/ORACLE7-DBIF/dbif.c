#ident	"$Id: dbif.c,v 5.1 2001/11/29 02:39:22 cha Exp $"
/*
 *	External interface
 *
 *******************************************************************************
 *	$Log: dbif.c,v $
 *	Revision 5.1  2001/11/29 02:39:22  cha
 *	Updated to make sure viewtype is compatible with Informix.
 *	
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.3  2000/08/02 02:34:58  raymund
 *	Small performance improvements. Added codes for locked find_hash()es.
 *	
 *	Revision 2.2  2000/07/28 06:10:08  raymund
 *	Implemented CURRENT in find_rec. Provided a patch for bug in disp_srch().
 *	
 *	Revision 2.1  2000/07/26 10:09:55  raymund
 *	Furnished missing functionalities. Use SQL for row locking.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.7  2000/07/13 11:08:45  raymund
 *	16-bit reversed CRC hardware emulation algorithm for row locking.
 *	
 *	Revision 1.6  1999/11/16 02:38:29  jonc
 *	Fixed: find_hash
 *	Fixed: wait-lock.
 *	
 *	Revision 1.5  1999/11/15 02:53:06  jonc
 *	Added lock code. Requires `alvin' the lock-daemon to be running.
 *	
 *	Revision 1.4  1999/11/01 21:33:21  jonc
 *	Added support for find_hash.
 *	
 *	Revision 1.3  1999/11/01 21:23:21  jonc
 *	Added support for FIRST.
 *	
 *	Revision 1.2  1999/10/28 01:58:36  jonc
 *	Added support for generic-catalog access. Damnably slow, though.
 *	
 *	Revision 1.1  1999/10/21 21:47:04  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	<dbio.h>
#include	<ProtosIF.h>

#include	"oracledbif.h"

#include	"ocidem.h"
#include	"ocidfn.h"


/*
 *  mode paremter to find_rec used exclusively here
 */
#define LOCKEDNHASHEDPREVIOUS  200

/*
 *	Login structures can be static since we only
 *	require one database connection per session
 */
static Lda_Def	lda;
static ub4		hda [HDA_SIZE / sizeof (ub4)];

static int HandlePrevious( TableState *table, void * buffer, 
                           int mode, const char * lock, int rec_key );
static int HandleCurrent( TableState *table, void * buffer, 
                           int mode, const char * lock, int rec_key );

/*
 *	System cursors.
 */
Cda_Def		cols_cda,				/* column inspection */
			colc_cda,				/* column count */
			seqs_cda,				/* sequence identifier */
			inds_cda,				/* index inspection */
			indc_cda;				/* index column count */

int
abc_dbopen (
 const char * ignored)
{
	/*
	 *	Get connection info from the DBPATH
	 *
	 *	We expect the environment to be of the form:
	 *
	 *		[sqlnetinfo@]schemalogin/password
	 *
	 */
	char *	dbpath = strdup (getenv ("DBPATH"));

	if (!strchr (dbpath, '/'))
	{
		/*
		 *	Essential login character missing
		 */
		oracledbif_warn ("DBPATH missing login/password: %s", dbpath);
		goto end_failure;
	}

	/*
	 * Attemp lock startup
	 */
	 _LockInitialise();

	/*
	 *	Clear out login structures.
	 *
	 *	Strictly not required, since globals are supposed to be
	 *	blanked with 0's on startup, but it's here for completeness
	 */
	memset (&lda, 0, sizeof (Lda_Def));
	memset (hda, 0, sizeof (hda));

	if (olog (&lda, (ub1 *) hda, 
			(text *) "logistic", -1,			/* username */
			(text *) "logistic", -1,			/* password */
			NULL, -1,							/* SQL*Net connection */
			(ub4) OCI_LM_DEF))
	{
		goto end_failure;
	}

	/*
	 *	Set up links for essential system catalogs
	 *
	 *		- user_indexes
	 *		- user_ind_columns
	 *		- user_sequences
	 *		- user_tables
	 *		- user_tab_columns
	 */
	if (oopen (&cols_cda, &lda, NULL, -1, -1, NULL, -1) ||
		oopen (&colc_cda, &lda, NULL, -1, -1, NULL, -1) ||
		oopen (&seqs_cda, &lda, NULL, -1, -1, NULL, -1) ||
		oopen (&inds_cda, &lda, NULL, -1, -1, NULL, -1) ||
		oopen (&indc_cda, &lda, NULL, -1, -1, NULL, -1))
	{
		oracledbif_error ("Failed to allocate system catalogs");
	}
	
	if (ocon (&lda))
	{
		oracledbif_error ("Failed to auto commit logon entry");
	}

	/*
	 *	Cleanup
	 */
	free (dbpath);
	return 0;

end_failure:
	free (dbpath);
	return 1;
}

int
abc_dbclose (
 const char * ignore)
{
	/*
	 *	Close off any possible open tables, but issue
	 *	a warning as well.
	 *
	 */
	DestroyAllTables ();
	DestroyAliasList ();

	/*
	 *	Close up system cursors before quitting
	 */
	oclose (&cols_cda);
	oclose (&colc_cda);
	oclose (&seqs_cda);
	oclose (&inds_cda);
	oclose (&indc_cda);

	ologof (&lda);

	_LockCleanup();

	return 0;
}

/*
 *	Tables
 */
int
open_rec (
 const char * table,
 struct dbview * fields,
 int count,
 const char * index)
{
	TableState * state = AllocateStdTable (&lda, table);

	state -> viewc = count;
	state -> view = fields;
	state -> fd_table = -1;
	IdentifyFields (&lda, &cols_cda, &colc_cda, &seqs_cda, state);

	IdentifyIndex (&lda, &inds_cda, &indc_cda, state, index);

	state -> orig_data = NULL;

	   {
        int i;

        for ( i = 0; i < state -> viewc; i++ )
        {
            switch( state -> view[i].vwtype )
            {
                case OT_Chars:
                    state -> view[i].vwtype = 0;
                    state -> view[i].vwlen -= 1;
                    break;
                case OT_Date:
                    state -> view[i].vwtype = 2 + 0x0200;
                    break;
                case OT_Serial:
                    state -> view[i].vwtype = 2 + 0x0100;
                    break;
                case OT_Number:
                    state -> view[i].vwtype = 2;
                    break;
                case OT_Float:
                    state -> view[i].vwtype = 4;
                    break;
                case OT_Money:
                    state -> view[i].vwtype = 3 + 0x0500;
                    break;
                case OT_Double:
                    state -> view[i].vwtype = 3;
                    break;
                default:
                    break;
            }

        }
    }

	return 0;
}

void
abc_fclose (
 const char * table)
{
	DestroyTable (table);
}

int
abc_selfield (
 const char * table,
 const char * new_index)
{
	TableState * state = LocateTable (table);

	IdentifyIndex (&lda, &inds_cda, &indc_cda, state, new_index);

	return 0;
}

int
abc_unlock (
 const char * table)
{
	TableState * state = LocateTable (table);

    _LockFreeAll (state, &state -> locks);
	return 0;
}

/*
 *	Updates
 */
int
abc_update (
 const char * table,
 void * recbuf)
{
	int resultcode;
	resultcode = _UpdateRow (&lda, LocateTable (table), recbuf);
	abc_unlock(table);
	if (resultcode)
		return 1;
	return 0;
}

int
abc_add (
 const char * table,
 void * recbuf)
{
	
	
	int resultcode;
	resultcode = _InsertRow (&lda, LocateTable (table), recbuf);
	abc_unlock(table);
	if (resultcode )
		return 1;
	return 0;
}

int
abc_delete (
 const char * table)
{
	int resultcode;
	resultcode = _DeleteRow (&lda, LocateTable (table));
	abc_unlock(table);
	if (resultcode )
		return 1;
	return 0;
}

/*
 *	Queries
 */
long
abc_rowid (
 const char * table)
{
	return 0;
}


int
find_rec (
 const char * table,
 void * buffer,
 int mode,
 const char * lock)
{ 
    static int num_rec_read = 0; 
    int fetched = FALSE;
	TableState * state = LocateTable (table);
	switch (mode)
	{
	case FIRST:				/* Absolute position */
		if ( state -> orig_data)
		{
		   free( state -> orig_data );
		   state -> orig_data = NULL;
		}
		num_rec_read = 0;
		fetched = QuerySetup (&lda, state, mode, TRUE, buffer );
		if ( !fetched )
		  break;
		fetched = QueryFetch (&lda, state, buffer, lock[0] );
		num_rec_read++;
		break;
	case LAST:				/* Absolute position */
		if ( state -> orig_data)
		{
		   free( state -> orig_data );
		   state -> orig_data = NULL;
		}
		num_rec_read = 0;
		fetched = QuerySetup (&lda, state, mode, TRUE+1, buffer );
		if ( !fetched )
		  break;
		fetched = QueryFetch (&lda, state, buffer, lock[0] );
		break;
	case NEXT:				/* Next row */
		fetched = QueryFetch (&lda, state, buffer, lock[0] );
		num_rec_read++;
		break;
	case PREVIOUS: 
		/* Too complex to handle here, call HandlePrevious() instead */
        fetched =  HandlePrevious( state, buffer, mode, lock, num_rec_read);
		num_rec_read--;
		break;
   /* Locally used for find_hash( ..., PREVIOUS, "w" or "u", ...) */
    case LOCKEDNHASHEDPREVIOUS:
		fetched = HandlePrevious( state, buffer, PREVIOUS, "r", num_rec_read );
		if ( !fetched )
		   break;
		/* Now that we have the record, let's lock it! */
        fetched =  HandleCurrent( state, buffer, EQUAL, lock, num_rec_read);
		break;
	case COMPARISON:
	case EQUAL:
	case GTEQ:
	case GREATER:
	case LT:
	case LTEQ:
		if ( state -> orig_data)
		   free( state -> orig_data );
		state -> orig_data = (char *) malloc( state -> datasz );
        memcpy( state -> orig_data, buffer, state -> datasz );
		num_rec_read = 0;
		fetched = QuerySetup (&lda, state, mode, FALSE, buffer );
		if ( !fetched )
		  break;
		fetched = QueryFetch (&lda, state, buffer, lock[0] );
		num_rec_read++;
		break;
	case CURRENT:
		num_rec_read = 0;
		/* Too complex to handle here, call HandleCurrent() instead */
        fetched =  HandleCurrent( state, buffer, EQUAL, lock, num_rec_read);
		break;
	default:
		oracledbif_error ("Unknown find_rec mode: %d", mode);
	}

	state->lastactualfetchmode = mode;

	if (!fetched)
		return 1;

	return 0;
}

int
find_hash (
 const char * table,
 void * buffer,
 int mode,
 const char * lock,
 long hash)
{
	int i;
	TableState * state = LocateTable (table);
	ColumnDef * indexcol = state -> columns + state -> indexactual [0];

	if (state -> indexc != 1 ||
		!(	indexcol -> type == OT_Number ||
			indexcol -> type == OT_Serial))
	{
		oracledbif_error ("Non-hash index");
	}

	/*
	 *	Prime the buffer with the hash value
	 *
	 *	We've got no choice but to scan the view for a matching name
	 */
	for (i = 0; i < state -> viewc; i++)
	{
		if (!strcmp (indexcol -> name, state -> view [i].vwname))
		{
			*(long *) ((char *) buffer + state -> view [i].vwstart) = hash;
			break;
		}
	}
	if (i >= state -> viewc)
		oracledbif_error ("Index %s not in dbview", indexcol -> name);

    /*------------------------------------------------------------+
	| When the mode is PREVIOUS we should consider that:          |
	|       If we use the PREVIOUS mode with find_rec and there   |
	|       is some locking required (i.e. "u" or "w") then a lot |
	|       of rows would be locked unnecessarily. This is bec.   |
	|       find_rec needs to search the current row sequentially |
	|       to determine the previous row.                        |
	+------------------------------------------------------------*/
	if ( (lock[0] == 'u' || lock[0] == 'w') && mode == PREVIOUS )
	   return find_rec(table,buffer, LOCKEDNHASHEDPREVIOUS, lock);
	   
	return find_rec (table, buffer, mode, lock);
}

/*
 *	Internal functions
 */
Lda_Def *
_LDA ()
{
	return &lda;
}

int 
HandlePrevious( 
  TableState* state, 
  void * buffer, 
  int mode, 
  const char * lock, 
  int rec_key)
{
    char      current_record[255],  /* Same as ROWID_SIZE in types.c */
              *previous_record = NULL;
    int       fetched = FALSE,
	      found   = FALSE,
              num_rec_read, 
	      col,
	      i;

    ColumnDef *prev_column = NULL;

        if ( state -> lastfetchmode == LT ||
             state -> lastfetchmode == LTEQ )
	   fetched = QueryFetch (&lda, state, buffer, lock[0] );
        else

	switch ( state -> lastactualfetchmode ) 
	{
handle_LAST:
          case LT:
          case LTEQ:
	  case LAST: 
	       fetched = QueryFetch (&lda, state, buffer, lock[0] );
		   break;
      case NEXT:
handle_NEXT:
           memcpy( buffer, state->orig_data, state->datasz );
	       fetched = QueryFetchPrevious( &lda, state, buffer, 
										  rec_key - 1, lock[0] );
           break;
      case PREVIOUS:
           switch( state -> lastfetchmode )
		   {
		     case LAST:
			   goto handle_LAST;
             case COMPARISON:
			 case EQUAL:
		     case GTEQ:
			 case GREATER: 
			 case LT: 
			 case LTEQ:
                goto handle_NEXT;
           }
		   /* Look ma! No breaks! */
	   default:  /* All the rest, the dirty, bloody part */
	   {
		  /* 
			 Remember the current data read 
	      */
          col = state->columnc - 1; /* column of the rowid */
		  strcpy( current_record,  state->columns[col].data );

		  /* 
			 Rebuild the query: a find first query 
		  */
		  num_rec_read = 0;
		  if ( !(fetched = QuerySetup ( &lda, state, mode, TRUE, buffer  )) )
                break;

		  /*
                     Fetch data, keeping track of the previous record. 
		     If current record is the same as the remembered 
		       record then the previous record is the record to
		       be returned. if no-more-data then return.
		  */
		  found = FALSE;
		  num_rec_read = 0;
		  previous_record = (char *) malloc( state -> datasz );
		  prev_column = (ColumnDef *) calloc( col + 1, sizeof(ColumnDef) );
	  	  memcpy( (void*) prev_column,
			      (void*) state->columns,
				  sizeof( ColumnDef ) );
		  for ( i = 0; i <= col; i++ )
			 prev_column[i].data = malloc( state->columns[i].length );
		  while ( !found )
		  {
			memcpy( previous_record, buffer, state -> datasz );
		    for ( i = 0; i <= col; i++ )
			   memcpy( (void*) prev_column[i].data, 
					   (void*) state->columns[i].data,
					   state->columns[i].length );

		    fetched = QueryFetch (&lda, state, buffer, lock[0] );
			if ( fetched ) /* A record was read */
			{
		       num_rec_read++;
			   if ( strcmp( state->columns[col].data, current_record ) == 0 )
			      found = TRUE;
			}
			else /* No match was read */
			   break; /* from the while */
          } /* while */

		  if ( found )
		  {
			 /* 
				if the current record is the very first record then although
				it is found, there is no previous record to return
             */
			 if ( num_rec_read == 1 )
				fetched = FALSE;
             else
			 {
               memcpy(buffer, previous_record, state->datasz );
		       for ( i = 0; i <= col; i++ )
			      memcpy( (void *) state->columns[i].data, 
						  (void *) prev_column[i].data,
					      state->columns[i].length );
               fetched = TRUE;
             }
		  }
       } /* default */
	} /* switch ( state->lastactualfetchmod ) */
	
    if ( prev_column )
	{
	   for( i = 0; i <= col; i++ )
		  free( prev_column[i].data );
	   free( prev_column );
    }
    if ( previous_record )
	   free( previous_record );
	return fetched;
}

int 
HandleCurrent( 
  TableState* state, 
  void * buffer, 
  int mode, 
  const char * lock, 
  int rec_key)
{
  int i,
	  fetched;

  /*------------------------------------------------+
  | Get the current value of the index fields, then |
  | place it on buffer. This way, we could reuse    |
  | the same algo used in find_rec ( .. EQUAL .. )  |
  +------------------------------------------------*/
  for ( i = 0; i < state -> indexc; i++ )
  {
	 ColumnDef *col = state -> columns + state -> indexactual[i];
	 char      *dat =  buffer + 
					   ( state -> view[state -> indexview[i]].vwstart );
     memcpy( dat, col->data, col->length );
  }
  fetched = QuerySetup (&lda, state, mode, FALSE, buffer );
  if ( !fetched )
    return fetched;
  fetched = QueryFetch (&lda, state, buffer, lock[0] );
  return fetched;

}


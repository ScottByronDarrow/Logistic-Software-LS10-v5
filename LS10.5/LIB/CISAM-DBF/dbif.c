#ident	"$Id: dbif.c,v 5.3 2002/03/13 07:20:03 scott Exp $"
/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( routines.c     )                                 |
|  Program Desc  : ( Holds Routines to Work With Informix DB Files. ) |
|                  ( ROUTINES FOR INFORMIX 3.30 NOT FOR SQL 2.?     ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author        : Scott Darrow.   |
|---------------------------------------------------------------------|
	$Log: dbif.c,v $
	Revision 5.3  2002/03/13 07:20:03  scott
	.
	
	Revision 5.2  2001/08/20 23:07:45  scott
	Updated from scott's machine
	
	Revision 5.1  2001/08/06 22:47:56  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 07:07:43  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:53:53  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 2.0  2000/07/15 07:32:18  gerry
	Forced Revision No start to 2.0 - Rel-15072000
	
	Revision 1.8  2000/06/16 05:32:13  scott
	Updated to increase length of message strings.
	
	Revision 1.7  1999/11/17 00:57:15  jonc
	Moved for_chk() from DBIF to BASE.
	
	Revision 1.6  1999/11/02 22:58:33  jonc
	NULL index on open_rec should not crash, but should use sequential access.
	
	Revision 1.5  1999/09/30 04:57:28  jonc
	Tightened the argument to use const char * where applicable.
=====================================================================*/
#include	"dbcisam.h"
#include	<VerNo.h>
#include	<std_decs.h>

extern	int	restart;

#define		EOI	2025

/*
 * Function declarations
 */
static int	filelocked (const char *, void *, int, const char *, const char *);

/*
 * This simply is used to provide a means to allow a getchar to fail.
 */
static void
signl_set (
 int	value)
{
}

/*
 * This Routine Opens A file ,Sets The View and Key off the file.
 */
int
open_rec (
	const 	char 	*rec,
	struct 	dbview	*rec_list,
	int 	no_recs,
	const 	char 	*field)
{
	return _open_rec (rec, rec_list, no_recs, field, field ? ACCKEYED : ACCSEQUENTIAL);
}

int
_open_rec (
	const 	char	* file,
	struct dbview * rec_list,
	int 	no_recs,
	const 	char * field,
	int 	key_flag)
{
	int	err;

	while ((err = dbselect (FILEOPEN, file)) == RECLOCK ||
		err == FILELOCK ||
		err == DEADLOCK)
	{
		if (for_chk () == 0)
		{
			char	msg [132];

			sprintf (msg,
				"(%s) Is Locked By Another User. Please Wait.",
				file);
			print_mess (msg);
		}
		sleep(1);
	}

	if (err && err != NOTOPEN && err != OPENTWO) 
		dbase_err ("FILEOPEN", file, err);

	if ((err = dbstructview (file, rec_list, no_recs)))
		dbase_err ("DBSTRUCTVIEW", file, err);

	if ((err = dbselfield (file, field, key_flag)))
		dbase_err ("DBSELFIELD", file, err);

	_TStampCheck (file);		/* check for timestamp field */

	return (err);
}

/*
 * Close data base files.
 */
void
abc_fclose (
 const char * cl_fil)
{
	int	err;

	while ((err = dbselect (FILECLOSE, cl_fil)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK || 
		err == 4);
	if (err != 0 && err != NOTOPEN && err != ISAMOPEN)
		dbase_err ("FILECLOSE", cl_fil, err);
}

/*
 * C l o s e  D a t a b a s e.                            
 * Check For File Lock ,Database Lock ,System Dead Lock.
 */
int
abc_dbclose (
 const char * cl_db)
{
	int	err;
	
	while ((err = dbselect (DBCLOSE, cl_db)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK ||
		err == 4);
	if (err != 0 && err != NOTOPEN && err != ISAMOPEN)
		dbase_err ("DBCLOSE", cl_db, err);

	ClearAliases ();
	return (err);
}

/*
 * O p e n   D a t a b a s e.               
 * Check That The Database Has Been Opened. 
 */
int
abc_dbopen (
 const char * op_db)
{
	int	err;

	while ((err = dbselect (DBOPEN, op_db)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK);
	if (err != 0 && err != NOTOPEN && err != ISAMOPEN)
		dbase_err ("DBOPEN", op_db, err);

	return (err);
}

/*
 * dbadd Routine With Check On File Locks Etc.
 */
int
abc_add (
 const char * add_fil,
 void		*add_rec)
{
	int	err;
	
	while ((err = dbadd (add_fil, add_rec)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK)
	{
		if (for_chk() == 0)
		{
			char	msg [132];

			sprintf (msg,
				"(%s) Is Locked By Another User. Please Wait.",
				add_fil);
			print_mess (msg);
		}
		sleep(1);
		clear_mess();
	}

	if (!err)
		_TStampIt (add_fil);	/* add timestamp */

	return (err);
}
/*
 * dbselfield Routine With error checking.
 */
int
abc_selfield (
 const char * sel_fil,
 const char * sel_field)
{
	int	err;
	
	while ((err = dbselfield (sel_fil, sel_field, 0)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK);
	if (err) 
		dbase_err ("DBSELFIELD", sel_fil, err);

	return (err);
}
/*
 * dbupdate Routine With Check On File Locks Etc.
 */
int
abc_update (
 const char * up_fil,
 void * up_rec)
{
	int	err;
	
	while ((err = dbupdate (up_fil, up_rec)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK)
	{
		if (for_chk() == 0)
		{
			char	msg [132];

			sprintf (msg,
				"(%s) Is Locked By Another User. Please Wait.",
				up_fil);
			print_mess (msg);
		}
		sleep(1);
		clear_mess();
	}

	if (!err)
	{
		_TStampIt (up_fil);				/* modification timestamp */
		err = abc_unlock (up_fil);
	}

	return (err);
}

/*
 *	Returns the unique identifier for the current record
 */
long
abc_rowid (
 const char * filename)
{
	LLIST	*tptr = _GetNode (filename);

	if (!tptr)
		dbase_err ("ABC_ROWID", filename, NOFILENAME);
	return (tptr -> _recno);
}

/*
 *	Returns the offset in bytes for a field of a
 *	table within a record structure
 */
int
abc_offset (
 const char *table,
 const char *fldname)
{
	int		i;
	LLIST	*tptr = _GetNode (table);

	if (!tptr)
		dbase_err ("abc_offset", table, NOFILENAME);

	for (i = 0; i < tptr -> _no_fields; i++)
		if (!strcmp (tptr -> _view [i].vwname, fldname))
			return (tptr -> _view [i].vwstart);

	dbase_err ("abc_offset", table, NOFNAME);
	return (-1);
}

/*
 * dbdelete Routine Deletes a record.
 */
int
abc_delete (
 const char * del_fil)
{
	int	err = abc_unlock (del_fil);

	while ((err = dbdelete (del_fil)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK);
	return(0);
}
	
/*
 * dbalias routine aliases a database file
 */
int
abc_alias (
 const char * newname,
 const char * oldname)
{
	int	err;
	
	while ((err = dbalias (newname, oldname)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK);
	if (err)
		dbase_err ("DBALIAS", newname, err);

	return (err);
}

/*
 * dbunlock Routine unlocks a record.
 */
int
abc_unlock (
 const char * ulock_fil)
{
	int	err;
	
	while ((err = dbunlock (ulock_fil)) == RECLOCK ||
		err == LOCKDENIED ||
		err == FILELOCK ||
		err == DEADLOCK);
	return (err);
}

/*
 * Get a full file lock on a file.
 */
int
abc_flock (
 const char * lck_fil)
{
	return (dblock (lck_fil));
}

/*
 * Remove a full file lock on a file.
 */
int
abc_funlock (
 const char * lck_fil)
{
	return (dbunlock (lck_fil));
}

/*
 * Return auditting information.	
 */
struct	audinfo	*abc_audinfo (
 const char * file)
{
	return (db_audinfo (file));
}

/*
 * The function find_rec should be called with 3 parameters:        
 *           the file name,the record to be found (key presumed    
 * already set up) and a single character string which can be:    
 * 	"r":   Read only - so no lock is requested                      
 * 	"w":   Write - lock is requested with an abort option if locked 
 * 	"u":   Update - lock is requested and keeps trying till available.
 */
int
find_rec (
 const char * fil,
 void * rec,
 int ftype,
 const char * rw)
{
	int		length;
	int		err = dbfind (fil,
				ftype + ((*rw == 'r') ? 0 : LOCK),
				NULL,
				&length,
				rec);
	char	msg [132];

	/*
	 * 6010 - Value Could Not be Found.  
	 * 6009 - No Data In File.          
	 * 6035 - No Current record Found.  
	 * 6020 - There is No Current Record.
	 * 6014 - No Such Flag Value Found. 
	 * 111  - No Record Cound Be Found.
	 */
	if (err == NOVAL || err == NODATA || err == 6035 ||
		err == NORECORD || err == NOFLAGVAL || err == NOKEY || err == NOURUN)
		return(1);
	
	/*
	 * 6011 - End Of File.                      
	 * 6012 - Beginning of File.                
	 * 110  - Beggining Or End of File Reached 
	 */
	if (err == NOCRU || err == FILEEND || err == FILEBEG)
		return(2);

	if (!err) 	/* record is on file	*/
		return(0);

	if (err == RECLOCK && *rw == 'r')
		return(0);

	/*
	 * 6022 - Lock Was Denied.              
	 * 107  - Record Or File Was Locked.    
	 * 113  - File is Exclusively Locked.   
	 * 45   - Dead Lock. Operating System.  
	 */
	if (err == RECLOCK || err == LOCKDENIED || err == FILELOCK || err == DEADLOCK) 
		return (filelocked (fil, rec, ftype, rw, NULL));

	sprintf (msg, "Error in %s During (DBFIND)", fil);
	sys_err (msg, (err == -1) ? errno : err, PNAME);
	return (999);	/*NOT REACHED*/
}

/*
 * The function find_hash should be called with 5 parameters:            
 *           the file name,the record to be found (key presumed         
 * already set up) and a single character string which can be:         
 * 	"r":   Read only - so no lock is requested                        
 * 	"w":   Write - lock is requested with an abort option if locked   
 * 	"u":   Update - lock is requested and keeps trying till available.
 *  and the hash								  |
 */
int
find_hash (
	const 	char * fil,
	void 	*rec,
	int 	ftype,
	const 	char * rw,
	long 	hash)
{
	int		length;
	int		err;
	char	msg [132];
	LLIST	*tptr = _GetNode (fil);

	/*	Ensure that current file index is singular "long"
	 */
	if (!tptr)
	{
		err = NOFILENAME;
		goto fatal_end;
	}
	if (tptr -> _key.k_nparts &&			/* ok to have no index */
		(tptr -> _key.k_nparts != 1 ||
			tptr -> _key.k_part [0].kp_type != LONGTYPE))
	{
		err = 0;
		goto fatal_end;
	}

	/*	Invoke find
	 */
	err = 	dbfind 
			(
				fil,
				ftype + ((*rw == 'r') ? 0 : LOCK),
				(char *) &hash,
				&length,
				rec
			);

	/*
	 * 6010 - Value Could Not be Found.   
	 * 6009 - No Data In File.            
	 * 6035 - No Current record Found.    
	 * 6020 - There is No Current Record. 
	 * 6014 - No Such Flag Value Found.   
	 * 111  - No Record Cound Be Found.   
	 */
	if (err == NOVAL || err == NODATA || err == 6035 ||
		err == NORECORD || err == NOFLAGVAL || err == NOKEY || err == NOURUN)
		return(1);
	
	/*
	 * 6011 - End Of File
	 * 6012 - Beginning of File
	 * 110  - Begining Or End of File Reached
	 */
	if (err == NOCRU || err == FILEEND || err == FILEBEG)
		return(2);

	if (!err) 	/* record is on file	*/
		return(0);

	if (err == RECLOCK && *rw == 'r')
		return(0);

	/*
	 * 6022 - Lock Was Denied.              
	 * 107  - Record Or File Was Locked.    
	 * 113  - File is Exclusively Locked.   
	 * 45   - Dead Lock. Operating System.  
	 */
	if (err == RECLOCK || err == LOCKDENIED || err == FILELOCK || err == DEADLOCK) 
		return (filelocked (fil, rec, ftype, rw, (char *) &hash));

fatal_end:
	sprintf (msg, "Error in %s During (find_hash)", fil);
	sys_err (msg, (err == -1) ? errno : err, PNAME);
	return (999);	/*NOT REACHED*/
}

void
dbase_err(
 const char *e_type,
 const char *e_rec,
 int e_err)
{
	char	msg [132];

	sprintf (msg, "Error in %s During (%s)", e_rec, e_type);
	sys_err (msg, e_err, PNAME); 

	// NOTREACHED
	libver [0] = '@';
}

/*
 * Possible Wait loop function to obtain lock
 */
static int
filelocked (
 const char * fil,
 void * rec,
 int ftype,
 const char * rw,
 const char * hash)
{
	int		err;
	int		abort = 1;
	int		length;
	int		progSleep = 1;
	char	msg [132];

	do
	{
		signal (SIGALRM, signl_set);
		err = dbfind (fil,
			ftype + ((*rw == 'r') ? 0 : LOCK),
			hash,
			&length,
			rec);
		if (err == RECLOCK || err == LOCKDENIED || err == FILELOCK || err == DEADLOCK) 
		{ 
			/*
			 * Updating record - wait till available.
			 */
			if (*rw == 'u' || *rw == 'r')
			{
				if (for_chk () == 0 && *rw == 'u')
				{
					sprintf (msg,
						"(%s) Record Is Locked By Another User. Please Wait.",
						fil);
					print_mess (msg);
					sleep(progSleep++);
					clear_mess();
				}
				else
					sleep(progSleep++);
			}
			else 
			{
				if (for_chk () == 0)
				{
					int	testchar;

					sprintf (msg,
						"(%s) Record is locked - EDIT/END to Restart",
						fil);
					print_mess (msg);
					alarm (2);
					testchar = getkey ();
					if (testchar == EOI) 
					{
						abort = -1;
						restart = 1;
						alarm(0);
					}
					clear_mess ();
				}
				else
					sleep(progSleep++);
			}
		}
		else 
		{	
			/*	
			 * The record is not locked any longer
			 */
			abort = 0;
			progSleep = 1;
			alarm (0);
		}
	} while (abort == 1);
	signal (SIGALRM, SIG_DFL);
	return (abort);
}

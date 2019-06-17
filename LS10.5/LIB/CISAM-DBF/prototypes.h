/*
 *
 *
 *******************************************************************************
 *	$Log: prototypes.h,v $
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.2  2000/02/10 00:32:46  jonc
 *	Added logging for lock errors on abc_update and abc_delete.
 *	
 *	Revision 1.1  1999/09/30 04:57:29  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
// dbadd.c
extern int dbadd (const char *filename, char *recbuf);

// dbalias.c
extern int dbalias (const char *newname, const char *oldname);
extern const char *_check_alias (const char *filename);
extern void ClearAliases (void);

// dbaudit.c
extern void StartAudit (void);
extern int stop_aud (LLIST *tptr);
extern int write_aud (LLIST *tptr, char *type, long int recno);
extern int lower_auds (void);

// dbdelete.c
extern int dbdelete (const char *filename);

// dbfield.c
extern int dbfield (const char *filename, const char *fldname, short int *ftype, short int *flen, short int *fperms);

// dbfile.c
extern int dbfile (const char *filename, short int *numflds, short int *recsize, short int *perms);

// dbfind.c
extern int dbfind (const char *filename, int flag, const char *value, int *_length, char *recbuf);

// dbinit.c
extern void _load_isam (int fieldtype, LLIST *tptr, int start, int len, const char *value);

// dblock.c
extern int dblock (const char *filename);

// dbncomposite.c
extern int dbncomposite (short int ncomp, const char *filename, const char *compname, char *partname, short int *ptype, short int *plen, short int *pperms);

// dbselect.c
extern int dbselect (int flag, const char *name);
extern LLIST *_GetNode (const char *tabname);
extern LLIST *_GetSysNode (int sysid);
extern struct audinfo *db_audinfo (const char *file);

// dbselfield.c
extern int dbselfield (const char *filename, const char *idx_name, int flag);

// dbstructview.c
extern int dbstructview (const char *filename, struct dbview *fieldlist, int n);
extern int _check_view (char *fieldname, LLIST *tptr);

// dbunlock.c
extern int dbunlock (const char *filename);

// dbupdate.c
extern int dbupdate (const char * filename, char *recbuf);

// dbutils.c
extern char *_get_dbpath (const char *dbname);
extern int _isam_type (int sql_type);
extern int _c_type (int sql_type);
extern int _c_size (int sql_type);
extern int _col_len (int t, int l);
extern int _set_offsets (struct dbview *fieldlist, int n);
extern int _align (int x, int t);

/* errlog.c
 */
extern void DBIFWriteLog (const char * mask, ...);

// tstamp.c
extern void _TStampCheck (const char *table);
extern void _TStampIt (const char *table);

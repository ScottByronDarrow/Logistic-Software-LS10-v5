#ifndef	_PROTOS_IF_H
#define	_PROTOS_IF_H
/*
 *	Database interface
 *
 *******************************************************************************
 *	$Log: ProtosIF.h,v $
 *	Revision 5.0  2001/06/19 06:51:19  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:21  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:51  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:35  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.7  1999/11/15 06:47:05  scott
 *	Updated for compile problems on AIX
 *	
 *	Revision 1.6  1999/10/28 23:26:43  jonc
 *	Removed conflict indicators.
 *	
 *	Revision 1.5  1999/10/28 23:24:56  jonc
 *	Added interface for build and support tools.
 *	
 *	Revision 1.4  1999/10/26 22:49:17  jonc
 *	Added system-catalog interface.
 *	
 *	Revision 1.3  1999/10/06 23:54:07  jonc
 *	Moved prototypes for generic access from <dbio.h> to <ProtosIF.h>.
 *	
 *	Revision 1.2  1999/09/30 04:56:24  jonc
 *	Reduced the prototypes to only those that should be seen on the
 *	external interface, ie only those in dbif.c.
 *	
 *	All other prototypes moved to CISAM-LIB, as they are CISAM specific.
 *	
 */
/* attrs.c */
const char * DbIfAttribute (const char *);

/* dbif.c */
extern int open_rec (const char *rec, struct dbview *rec_list, int no_recs, const char *field);
extern int _open_rec (const char *file, struct dbview *rec_list, int no_recs, const char *field, int key_flag);
extern void abc_fclose (const char *cl_fil);
extern int abc_dbclose (const char *cl_db);
extern int abc_dbopen (const char *op_db);
extern int abc_add (const char *add_fil, void *add_rec);
extern int abc_selfield (const char *sel_fil, const char *sel_field);
extern int abc_update (const char *up_fil, void *up_rec);
extern long int abc_rowid (const char *filename);
extern int abc_offset (const char *table, const char *fldname);
extern int abc_delete (const char *del_fil);
extern int abc_alias (const char *newname, const char *oldname);
extern int abc_unlock (const char *ulock_fil);
extern int abc_flock (const char *lck_fil);
extern int abc_funlock (const char *lck_fil);
extern struct audinfo *abc_audinfo (const char *file);
extern int find_rec (const char *fil, void *rec, int ftype, const char *rw);
extern int find_hash (const char *fil, void *rec, int ftype, const char *rw, long int hash);
extern int for_chk (void);
extern void dbase_err (const char *e_type, const char *e_rec, int e_err);

/*
 *	gen-access.c
 *
 *	You are expected to use TableColumnCount() to get
 *	the number of tables in the system followed by
 *	TableInfo(), using any number less than TableColumnCount();
 *
 *	TableColumnCount() initialises the catalog system, and must
 *	be used if extracting info from unopened tables.
 *
 *	eg:
 *		table_count = TableCount ();
 *		for (t = 0; t < table_count; t++)
 *		{
 *			int colcount, colno;
 *			struct TableInfo tblinfo;
 *			struct ColumnInfo colinfo;
 *
 *			TableInfo (t, &tblinfo);
 *
 *			colcount = TableColumnCount (tblinfo.name);
 *			TableColumnInfo (tblinfo.name);
 *		}
 */
extern int TableCount (void);				/* initialises catalog system */
extern int TableNumber (const char *);		/* -1 if it doesn't exist */
extern void TableInfo (int tableno, struct TableInfo * buffer);
extern void TableIndexInfo (int tableno, int indexno, struct IndexInfo *);
extern void TableIndexColumnInfo (
				int tableno, int indexno, int colno,
				struct ColumnInfo *);

/*
 *	If you use TableColumnCount/Info() on an open table, it will
 *	return the info you expect. If you want to use it on a
 *	table you do not expect to open, you *must* use TableCount() first
 *	to initialise the catalog system.
 *
 *	TableColumnGet will only work on opened tables.
 */
extern int	TableColumnCount (const char *);
extern void	TableColumnInfo (const char *, int colno, struct ColumnInfo *);
extern void	TableColumnGet (const char *, int colno, void * buffer),
			TableColumnNameGet (const char * file, const char * column, void *);

#endif	/* _PROTOS_IF_H */

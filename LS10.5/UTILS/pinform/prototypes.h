/*
 *
 *
 *******************************************************************************
 *	$Log: prototypes.h,v $
 *	Revision 5.0  2001/06/19 08:23:13  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:44:09  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:25  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:29  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1  1999/11/18 05:27:01  scott
 *	Add required prorotypes
 *	
 *	Revision 1.1  1999/09/30 04:57:29  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
extern int dbadd (const char *, char *);
extern int dbalias (const char *, const char *);
extern int dbdelete (const char *);
extern int dbfield (const char *, const char *, short int *, short int *, short int *);

extern int dbfile (const char *, short int *, short int *, short int *);
extern int dbfind (const char *, int, const char *, int *, char *);

extern int dblock (const char *filename);

extern int dbncomposite (short int, const char *, const char *, char *, short int *, short int *, short int *);
extern int dbselect (int, const char *);
extern int dbselfield (const char *, const char *, int);
extern int dbstructview (const char *, struct dbview *, int);
extern int dbunlock (const char *);
extern int dbupdate (const char *, char *);

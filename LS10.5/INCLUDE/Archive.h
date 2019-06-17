#ifndef	_Archive_h
#define	_Archive_h
/*	$Id: Archive.h,v 5.1 2002/10/09 06:02:55 robert Exp $
 *******************************************************************************
 *	$Log: Archive.h,v $
 *	Revision 5.1  2002/10/09 06:02:55  robert
 *	Modified to save archive data into text file format to make
 *	it OS independent
 *	
 *	Revision 5.0  2002/05/08 01:40:39  scott
 *	CVS administration
 *	
 *	Revision 1.4  2002/05/07 06:38:52  scott
 *	Last version before testing
 *	
 *	Revision 1.3  2002/05/07 02:50:01  scott
 *	Updated for new archiving system
 *	
 *	Revision 1.2  2002/04/30 03:19:00  scott
 *	Update for new Archive modifications;
 *	
 *	Revision 1.1  2002/04/29 05:39:03  scott
 *	New Archive Functions
 *	
 *	
 */

enum
{
	ArchiveErr_Ok,
	ArchiveOpenError,
	ArchiveAddError,
	ArchiveDeleteError,
	ArchiveFindError,
	ArchiveErr_Failed
};

extern	char	*Replace (char *, char, char);
extern	int		selfprintf (FILE *, const char *, ...);
extern	int		ArSohrRead (FILE *, void *);
extern	int		ArSolnRead (FILE *, void *);
extern	int 	ArhrRead (FILE *, void *);
extern	int 	ArlnRead (FILE *, void *);
extern	int 	ArPohrRead (FILE *, void *);
extern	int 	ArPolnRead (FILE *, void *);
extern	int 	ArPcwoRead (FILE *, void *);

extern	void	ArchiveClose		(void);
extern	int		ArchiveSohr			(long);
extern	int		ArchiveSoln			(long);
extern	int		ArchivePohr			(long);
extern	int		ArchivePoln			(long);
extern	int		ArchivePcwo			(long);
extern	int		DownloadDeleteSO	(char *, long);
extern	int		DownloadDeletePO	(char *, long);
extern	int		DownloadDeleteIN	(char *, long);
extern	int		DownloadDeleteWO	(char *, long);
extern	int		UploadSO			(char *, long);
extern	int		UploadPO			(char *, long);
extern	int		UploadIN			(char *, long);
extern	int		UploadWO			(char *, long);

#endif	/* _Archive_h */

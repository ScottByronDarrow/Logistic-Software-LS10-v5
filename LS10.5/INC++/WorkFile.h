#ifndef	_WorkFile_h
#define	_WorkFile_h
/*	$Id: WorkFile.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	Logistic Work Files
 *
 ****************************************************************
 *	$Log: WorkFile.h,v $
 *	Revision 5.0  2002/05/08 01:50:44  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1  1999/11/23 02:45:11  nz
 *	Copy three files from Ver10 INC++ directory
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:27  jonc
 *	Version 10 start
 *	
 *	Revision 2.1  1996/03/11 21:32:14  jonc
 *	Initial revision
 *
 */

class WorkFile
{
	private:
		char *	fname;
		long	recsz;
		int		fdesc;

	public:
		WorkFile (const char *, long, bool create = true);
		~WorkFile ();

		/*
		 *	State queries
		 */
		bool	Exists (void) const;
		int		Count (void);

		bool	Create (void),
				Delete (void);

		/*
		 *	Data access routines
		 */
		bool	First (void),					// positioning
				Last (void);
		bool	Next (void *),					// access
				Prev (void *),
				Get (unsigned, void *);			// access specified record

		/*
		 *	Data write
		 */
		bool	Write (void *),					// write at current posn
				Put (unsigned, void *);			// write specified record
};

#endif	//_WorkFile_h

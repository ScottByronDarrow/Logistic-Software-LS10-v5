#ifndef	_Table_h
#define	_Table_h
/*	$Id: Table.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	{libdbif:CISAM}
 *
 *******************************************************************************
 *	$Log: Table.h,v $
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
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 */
#include	<dbif-err.h>

class ColumnInfo;
class Constraint;
class Database;
class Index;
class String;

class Table
{
	private:
		Database &		dbase;
		char *			tbname,
			 *			fname;			// ISAM Filename
		long			tabid;			// useful for catalog searches
		unsigned		rowsize,		// for CISAM query buffers
						appbfsize;		// best guess for application buffer

		unsigned		fldcount;		// used fields
		const char **	fields;
		ColumnInfo *	fldinfo;

		unsigned		nfldcount;		// unused fields
		char **			nfields;
		ColumnInfo *	nfldinfo;

		int				indexcount;
		Index *			indexes;

		enum DBIFError	tError;			// last db manipulation error

	public:
		Table (const Table &);
		Table (Database &, const char * table, const char * fields []);
		~Table ();

		const char *	Name (void) const;
		long			TabId (void) const;
		unsigned		RowSize (void) const,
						AppBufSize (void) const,
						ColCount (void) const;

		Database &		DBase (void) const;

		/*
		 *	Accessing used fields
		 */
		const char *	ColName (unsigned) const;
		bool			ColInfo (unsigned, ColumnInfo &) const,
						ColInfo (const char *, ColumnInfo &) const;

		void *	Column (const char * colname, void * appbuf) const;

		/*
		 *	Accessing unused fields
		 */
		unsigned		UColCount (void) const;
		const char *	UColName (unsigned) const;
		bool			UColInfo (unsigned, ColumnInfo &) const,
						UColInfo (const char *, ColumnInfo &) const;

		/*
		 *	Database manipulation calls
		 */
		bool			Add (const void *);
		void			UpdateTimestamp (void *);	// not for gen-app use

		/*
		 *	Error queries
		 */
		enum DBIFError	Error (void) const;

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	private:
		void			ZeroUnused (void * cisam);
	public:
		const char *	FName (void) const;
		bool			ColIdInfo (short, ColumnInfo &) const;
		const Index *	GetIndex (const char *) const;

		bool	PickIndex (short [], String &),
				PickIndex (short [], unsigned, Constraint &, String &),
				PickIndex (unsigned, Constraint &, String &);

		void			AppToCISAM (const void * app, void * cisam);

	// end{Undocumented-Interface}
};

#endif	//_Table_h

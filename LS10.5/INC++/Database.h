#ifndef	_Database_h
#define	_Database_h
/*	$Id: Database.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	{libdbif:CISAM}
 *
 *	Database:
 *		- Class for accessing INFORMIX-SE Databases via CISAM
 *
 *******************************************************************************
 *	$Log: Database.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:42  scott
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

class Constraint;
class String;
class Table;
struct ColumnInfo;

#include	<Array.h>

class Database
{
	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	public:
		/*
		 *	Static functions
		 */

		/*
		 *	CISAM Accessors
		 */
		static int	ISAMOpen (const char *, bool withwrite = false),
					ISAMClose (int fd, bool withwrite = false);

		/*
		 *	System info
		 */
		static void	GetTabInfo (const char *, unsigned, const char * [],
						char *, long &, unsigned &, unsigned &,
						ColumnInfo * &,
						unsigned &, char ** &, ColumnInfo * &);
		static void	GetIdxInfo (long, int &, char ** &, short ** &);

	// end{Undocumented-Interface}

	public:
		/*
		 *	Transaction processing stuff
		 */
		static void	BeginTrX (void),			// these will succeed or crash
					AbortTrX (void),
					CommitTrX (void);
		static bool	InTrX (void);

	public:
		/*
		 *	Constructors
		 */
		Database (const Database &);
		Database (const char *);
		~Database ();

		/*
		 *	Misc Accessors
		 */
		const char *	Name (void) const;

};

#endif	//_Database_h

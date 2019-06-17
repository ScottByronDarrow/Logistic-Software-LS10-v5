#ifndef	_ConfFile_h
#define	_ConfFile_h
/*	$Id: ConfFile.h,v 5.0 2002/05/08 01:50:42 scott Exp $
 *
 *	General ConfigFile
 *
 *******************************************************************************
 *	$Log: ConfFile.h,v $
 *	Revision 5.0  2002/05/08 01:50:42  scott
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
#include	<Array.h>

class String;
class ConfSection;

class ConfigFileIter;
class ConfSectionIter;

class ConfigFile
{
	friend	ConfigFileIter;

	private:
		String *		path;
		ConfSection *	current;

		int						sectionc;
		Array <ConfSection *>	sections;

	private:
		void			Read (FILE *, const String &);

	public:
		ConfigFile (const char * name, const char * sysname = 0,
					bool die_if_not_exist = true);
		ConfigFile (const ConfigFile &);
		~ConfigFile ();

		/*
		 *	Set default section to use
		 */
		bool			UseSection (const char * name, const char * match = 0);

		/*
		 *	Section access
		 */
		const ConfSection *	Section (const char * section,
									 const char * match = 0) const;

		/*
		 *	Value access
		 */
		int				IGet (const char * var,
							  const char * section = 0,
							  const char * match = 0) const;
		const String & 	SGet (const char * var,
							  const char * section = 0,
							  const char * match = 0) const;

		/*
		 *	Write out configuration file
		 */
		bool			Write (void);

#ifndef	NDEBUG
		void			Dump (void) const;		// internal use only
#endif
};

/*
 *	Iterators
 */
class ConfigFileIter
{
	private:
		int			idx;
		ConfigFile	itCopy;

	public:
		ConfigFileIter (const ConfigFile &);

		/*
		 *
		 */
		const char *		SectionName (void) const;
		const ConfSection *	Section (void) const;

		operator			bool () const;

		/*
		 *	Iterator mutators
		 */
		void				Reset (void);
		void				operator ++ (int),
							operator -- (int);
};

class ConfSectionIter
{
	private:
		int				idx;
		ConfSection *	itCopy;

	public:
		ConfSectionIter (const ConfSection *);
		~ConfSectionIter ();

		/*
		 *
		 */
		const char *		ItemName () const,
				   *		ItemValue () const;

		operator			bool () const;

		/*
		 *
		 */
		void				Reset (void);
		void				operator ++ (int),
							operator -- (int);
};

#endif	//_Config_h

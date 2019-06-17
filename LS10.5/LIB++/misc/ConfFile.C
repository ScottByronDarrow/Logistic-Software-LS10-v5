#ident	"$Id: ConfFile.C,v 5.0 2001/06/19 08:19:05 cha Exp $"
/*
 *	General configuration file
 *
 *******************************************************************************
 *	$Log: ConfFile.C,v $
 *	Revision 5.0  2001/06/19 08:19:05  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1999/03/03 23:03:17  jonc
 *	Added user-specific lookup
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.4  1996/07/30 00:52:59  jonc
 *	Added #ident directive
 *
 *	Revision 2.3  1996/07/25 23:40:32  jonc
 *	Added iterators
 *
 *	Revision 2.2  1996/07/16 23:01:08  jonc
 *	Fatal error on missing ConfigFile
 *
 *	Revision 2.1  1996/05/04 08:28:26  jonc
 *	Added Configuration File class
 *
 */
#include	<assert.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdeps.h>
#include	<liberr.h>
#include	<minor.h>

#include	<ConfFile.h>
#include	<String.h>


/*
 *	Magic stuff
 */
#define	LINELEN	512								// hopefully

static const String	nothing ("(non-existent)");

/*
 *	Local classes
 */
class ValueTuple
{
	public:
		String	name,
				value;

	public:
		ValueTuple (
		 const char *	idname = NULL,
		 const char *	idvalue = NULL) :
			name (idname),
			value (idvalue)
		{
		}
};

class ConfSection
{
	friend	ConfSectionIter;
	friend	ConfigFileIter;

	private:
		String		name;
		String *	matches;

		int					tuplec;
		Array<ValueTuple>	tuples;

	public:
		ConfSection (const char *, const char * = NULL);
		ConfSection (const ConfSection &);
		~ConfSection ();

		bool			Matches (const char * sect, const char * match) const;

		void			Put (const char * varname, const char * value);
		const String &	Get (const char * varname) const;

#ifndef	NDEBUG
		void			Dump (void) const;
#endif
};

/*
 *	Private interface
 */
void
ConfigFile::Read (
 FILE *			cfg,
 const String &	name)
{
	int		lineNo = 0;
	char	line [LINELEN];
	ConfSection *	aSect = NULL;

	while (fgets (line, sizeof (line), cfg))
	{
		lineNo++;

		if (!strlen (clip (lclip (line))) || line [0] == '#')
			continue;				// ignore blank and comment lines

		if (line [0] == '[')
		{
			size_t	len = strlen (line);

			if (line [len - 1] != ']')
				(*app_error_handler) ("ConfigFile::Read",
					"Don't understand line %d of %s\n",
					lineNo, (const char *) name);

			/*
			 *	Decode section heading
			 */
			line [len - 1] = '\0';
			String	wline = line + 1;
			int		wposn = wline.index ('=');
			String	sectName, matchStr;

			switch (wposn)
			{
			case -1:
				aSect = new ConfSection (wline);
				break;
			case 0:
				(*app_error_handler) ("ConfigFile::Read",
					"Bad section header");

			default:
				sectName = wline.before (wposn);
				matchStr = wline.after (wposn);
				aSect = new ConfSection (sectName, matchStr);
			}
			
			sections [sectionc++] = aSect;

			continue;
		}

		/*
		 *	If we get here, we should be reading a value-tuple line
		 */
		if (!aSect)
			(*app_error_handler) ("ConfigFile::Read",
				"No section header found prior to line %d of %s",
				lineNo, (const char *) name);

		char	vname [LINELEN],
				value [LINELEN];

		sscanf (line, " %s %[^\n]", vname, value);
		aSect -> Put (vname, value);
	}
}

/*
 *	Public interface
 */
ConfigFile::ConfigFile (
 const char *	local,
 const char *	sysname,
 bool			die) :
	path (NULL),
	current (NULL),
	sectionc (0)
{
	if (!local || !*local)
	{
		(*app_error_handler) ("ConfigFile::ConfigFile",
			"Empty filename given");
	}

	/*
	 *	Look for a configuration file in the following places:
	 *		1.	current directory
	 *		2.	$HOME directory
	 *		3.	$PSL_MENU_PATH/CONFIG
	 *		4.	$PROG_PATH/CONFIG
	 */
	FILE *			config = NULL;
	String			tpath (local);
	char *			envp;
	const char *	rd = "r";

	config = fopen (tpath, rd);		//	try current directory first

	if (!config)
	{
		/*
		 *	Try $HOME
		 */
		cat (getenv ("HOME"), '/', local, tpath);
		config = fopen (tpath, rd);
	}

	if (!config && (envp = getenv ("PSL_MENU_PATH")))
	{
		/*
		 *	Try user-specific setup
		 */
		cat (envp, "/CONFIG/", sysname ? sysname : local, tpath);
		config = fopen (tpath, rd);
	}

	if (!config && (envp = getenv ("PROG_PATH")))
	{
		/*
		 *	Try SYSTEM setup
		 */
		cat (envp, "/CONFIG/", sysname ? sysname : local, tpath);
		config = fopen (tpath, rd);
	}

	if (config)
	{
		Read (config, tpath);
		fclose (config);
	}
	else if (die)
	{
		(*app_error_handler) (
			"ConfigFile::ConfigFile",
			"Configuration file \"%s\" not found", local);
	}

	path = new String (tpath);		// save path for Write purposes
}

ConfigFile::ConfigFile (
 const ConfigFile &	src) :
	path (new String (*src.path)),
	current (NULL),
	sectionc (src.sectionc)
{
	for (int i = 0; i < sectionc; i++)
		sections [i] = new ConfSection (*src.sections.Elem (i));
}

ConfigFile::~ConfigFile ()
{
	delete path;

	for (int i = 0; i < sectionc; i++)
		delete sections [i];
}

bool
ConfigFile::UseSection (
 const char *	section,
 const char *	match)
{
	for (int i = 0; i < sectionc; i++)
	{
		if (sections [i] -> Matches (section, match))
		{
			current = sections [i];
			return (true);
		}
	}

	current = NULL;
	return (false);
}

const ConfSection *
ConfigFile::Section (
 const char *	section,
 const char *	match) const
{
	for (int i = 0; i < sectionc; i++)
	{
		if (sections.Elem (i) -> Matches (section, match))
		{
			return (sections.Elem (i));
		}
	}
	return (NULL);
}

int
ConfigFile::IGet (
 const char *	name,
 const char *	section,
 const char *	match) const
{
	return (atoi (SGet (name, section, match)));
}

const String &
ConfigFile::SGet (
 const char *	name,
 const char *	section,
 const char *	match) const
{
	assert (name && *name);

	if (!section && !match)
	{
		if (!current)
			(*app_error_handler) ("ConfigFile::SGet", "No Default Section?");
		return (current -> Get (name));
	}

	for (int i = 0; i < sectionc; i++)
	{
		if (sections.Elem (i) -> Matches (section, match))
			return (sections.Elem (i) -> Get (name));
	}

	return (nothing);
}

#ifndef	NDEBUG
void
ConfigFile::Dump () const
{
	for (int i = 0; i < sectionc; i++)
		sections.Elem (i) -> Dump ();
}
#endif	//NDEBUG

/*
 *	ConfSection
 */
ConfSection::ConfSection (
 const char *	idname,
 const char *	mtstr) :
	name (idname),
	matches (mtstr ? new String (mtstr) : NULL),
	tuplec (0)
{
}

ConfSection::ConfSection (
 const ConfSection &	src) :
	name (src.name),
	matches (src.matches ? new String (*src.matches) : NULL),
	tuplec (src.tuplec)
{
	for (int i = 0; i < tuplec; i++)
		tuples [i] = src.tuples.Elem (i);
}

ConfSection::~ConfSection ()
{
	delete matches;
}

bool
ConfSection::Matches (
 const char *	section,
 const char *	match) const
{
	if (name == section)
	{
		if (match)
		{
			if (!matches)
				return (false);

			/*
			 *	Split the matches string up for possible matches
			 */
			bool		matchfound = false;
			int			sz = matches -> freq ('|') + 1;
			String *	elems = new String [sz];
			int			count = split (*matches, elems, sz, '|');

			assert (count == sz);

			for (int i = 0; i < count && !matchfound; i++)
				if (elems [i] == match)
					matchfound = true;

			delete [] elems;

			return (matchfound);
		}

		return (!matches);
	}
	return (false);
}

void
ConfSection::Put (
 const char *	varname,
 const char *	value)
{
	for (int i = 0; i < tuplec; i++)
		if (tuples.Elem (i).name == varname)
		{
			tuples [i].value = value;
			return;
		}

	tuples [tuplec++] = ValueTuple (varname, value);
}

const String &
ConfSection::Get (
 const char *	varname) const
{
	for (int i = 0; i < tuplec; i++)
		if (tuples.Elem (i).name == varname)
			return (tuples.Elem (i).value);

	return (nothing);
}

#ifndef	NDEBUG
void
ConfSection::Dump () const
{
	printf ("[%s", (const char *) name);
	if (matches)
		printf ("=%s", (const char *) *matches);
	printf ("]\n");

	for (int i = 0; i < tuplec; i++)
	{
		printf ("%s=%s\n",
			(const char *) tuples.Elem (i).name,
			(const char *) tuples.Elem (i).value);
	}
}
#endif	//NDEBUG

/**********************************************
 *
 *	Iterators
 *
 **********************************************/
ConfigFileIter::ConfigFileIter (
 const ConfigFile &	src) :
	idx (0),
	itCopy (src)
{
}

const char *
ConfigFileIter::SectionName () const
{
	return (itCopy.sections.Elem (idx) -> name);
}

const ConfSection *
ConfigFileIter::Section () const
{
	return (itCopy.sections.Elem (idx));
}

ConfigFileIter::operator bool () const
{
	return (idx >= 0 && idx < itCopy.sectionc);
}

void
ConfigFileIter::Reset ()
{
	idx = 0;
}

void
ConfigFileIter::operator ++ (
 int)
{
	if (idx < itCopy.sectionc)
		idx++;
}

void
ConfigFileIter::operator -- (
 int)
{
	if (idx >= 0)
		idx--;
}

/*
 */
ConfSectionIter::ConfSectionIter (
 const ConfSection *	src) :
	idx (0),
	itCopy (src ? new ConfSection (*src) : NULL)
{
}

ConfSectionIter::~ConfSectionIter ()
{
	delete itCopy;
}

const char *
ConfSectionIter::ItemName () const
{
	return (itCopy ? itCopy -> tuples.Elem (idx).name.chars () : NULL);
}

const char *
ConfSectionIter::ItemValue () const
{
	return (itCopy ? itCopy -> tuples.Elem (idx).value.chars () : NULL);
}

ConfSectionIter::operator bool () const
{
	return (itCopy && idx >= 0 && idx < itCopy -> tuplec);
}

void
ConfSectionIter::Reset ()
{
	idx = 0;
}

void
ConfSectionIter::operator ++ (
 int)
{
	if (itCopy && idx < itCopy -> tuplec)
		idx++;
}

void
ConfSectionIter::operator -- (
 int)
{
	if (itCopy && idx >= 0)
		idx--;
}

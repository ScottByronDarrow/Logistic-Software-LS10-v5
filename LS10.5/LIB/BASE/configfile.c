#ident	"$Id: configfile.c,v 5.0 2001/06/19 06:59:15 cha Exp $"
/*
 *	Configuration file interface
 *
 *******************************************************************************
 *	$Log: configfile.c,v $
 *	Revision 5.0  2001/06/19 06:59:15  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:35  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:19  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:13  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1  1999/10/11 21:40:42  jonc
 *	Added Configuration-file interface.
 *	
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<configfile.h>

#define	TRUE	1
#define	FALSE	0
#define	LINELEN	256

/*
 *	Local functions
 */
static void				ReadConfig (const char *, FILE *, ConfigFile *);
static _ConfSection *	AddSection ( _ConfSection ** s, const char * name);
static void				AddKeyValue (_ConfKey ** k,
							const char * key, const char * value);
static void				FreeConfSection (_ConfSection ** s),
						FreeConfKey ( _ConfKey ** k);

/*
 */
ConfigFile *
OpenConfig (
 const char * name)			/* program name, base will be taken */
{
	char *	p;
	char	base [128],
			path [256];
	FILE *	f = NULL;
	ConfigFile *	cf = NULL;
	const char *	Suffix = ".cfg",
			   *	Rd = "r";

	if ((p = strrchr (name, '/')))
		strcpy (base, ++p);
	else
		strcpy (base, name);
	strcat (base, Suffix);

	/*
	 *	Look for the config file in the following places
	 *
	 *		- Current directory
	 *		- $HOME directory
	 *		- $PSL_MENU_PATH/CONFIG
	 *		- $PROG_PATH/CONFIG
	 */
	f = fopen (strcpy (path, base), Rd);		/* Try current dir first */
	if (!f && (p = getenv ("HOME")))
	{
		/*
		 *	Try HOME
		 */
		sprintf (path, "%s/%s", p, base);
		f = fopen (path, Rd);
	}

	if (!f && (p = getenv ("PSL_MENU_PATH")))
	{
		/*
		 *	Try User-specific
		 */
		sprintf (path, "%s/CONFIG/%s", p, base);
		f = fopen (path, Rd);
	}

	if (!f && (p = getenv ("PROG_PATH")))
	{
		/*
		 *	Try System
		 */
		sprintf (path, "%s/CONFIG/%s", p, base);
		f = fopen (path, Rd);
	}

	if (!f)
		return NULL;

	cf = malloc (sizeof (ConfigFile));
	memset (cf, 0, sizeof (ConfigFile));		/* clear the structure */
	ReadConfig (path, f, cf);
	fclose (f);

	return cf;
}

void
CloseConfig (
 ConfigFile * cf)
{
	if (!cf)
		return;

	FreeConfSection (&cf -> sections);
	free (cf);
}

/*
 *	Obtain values
 */
const char *
GetConfig (
 const ConfigFile * cf,
 const char * section,
 const char * key)
{
	_ConfSection * s;

	for (s = cf -> sections; s; s = s -> next)
	{
		if (!strcmp (s -> name, section))
		{
			_ConfKey * k;

			for (k = s -> keys; k; k = k -> next)
			{
				if (!strcmp (k -> key, key))
					return k -> value;
			}
			break;
		}
	}
	return NULL;
}

int
GetConfigInt (
 const ConfigFile * cf,
 const char * section,
 const char * key)
{
	return atoi (GetConfig (cf, section, key));
}

/*
 *	Iterate thru' the ConfigFile
 */
int
ItrConfigSection (
 const ConfigFile * cf,
 int * it,						/* section iterator state */
 char * buffer)					/* return value */
{
	return FALSE;
}

int
ItrConfigKeys (
 const ConfigFile * cf,
 int sectionid,
 int * it,						/* keyword iterator state */
 char * buffer)					/* return value */
{
	return FALSE;
}

/*
 *	Support
 */
static void
ReadConfig (
 const char * path,
 FILE * cfg,
 ConfigFile * cf)
{
	/*
	 *
	 *	Read in the config file and hold the whole file's
	 *	value-tuples in the supplied structure
	 */
	int				lineno = 0;
	char			line [LINELEN];
	_ConfSection *	s = NULL;

	while (fgets (line, sizeof (line), cfg))
	{
		char	vname [LINELEN],
				value [LINELEN];

		lineno++;

		line [strlen (line) - 1] = '\0';	/* remove trailing newline */
		if (!line [0] || line [0] == '#')
			continue;						/* ignore blank and comment lines */

		if (line [0] == '[')
		{
			/*
			 *	Section heading
			 */
			size_t	len = strlen (line);
			char *	name = line + 1;

			if (line [len - 1] != ']')
			{
				fprintf (stderr,
					"ReadConfig: Don't understand line %d of %s\n",
					lineno, path);
				exit (EXIT_FAILURE);
			}

			/*
			 *	Decode section heading
			 */
			line [len - 1] = '\0';
			if (!name [0])
			{
				fprintf (stderr,
					"ReadConfig: Bad section header on line %d of %s\n",
					lineno, path);
				exit (EXIT_FAILURE);
			}

			if (!(s = AddSection (&cf -> sections, name)))
			{
				fprintf (stderr,
					"ReadConfig: General failure on line %d of %s\n",
					lineno, path);
				exit (EXIT_FAILURE);
			}

			continue;
		}

		/*
		 *	If we get here, we should be reading a value-tuple line
		 */
		if (!s)
		{
			fprintf (stderr,
				"ReadConfig: No section header found prior to line %d of %s\n",
				lineno, path);
			exit (EXIT_FAILURE);
		}

		sscanf (line, " %s %[^\n]", vname, value);
		AddKeyValue (&s -> keys, vname, value);
	}
}

/*
 *	List manipulators
 */
static _ConfSection *
AddSection (
 _ConfSection ** s,
 const char * name)
{
	if (*s)
		return AddSection (&(*s) -> next, name);

	/*
	 *	Add new node
	 */
	*s = malloc (sizeof (_ConfSection));
	memset (*s, 0, sizeof (_ConfSection));

	(*s) -> name = strdup (name);

	return *s;
}

static void
AddKeyValue (
 _ConfKey ** k,
 const char * key,
 const char * value)
{
	if (*k)
	{
		AddKeyValue (&(*k) -> next, key, value);
	} else
	{
		/*
		 *	Add key-value tuple
		 */
		*k = malloc (sizeof (_ConfKey));
		memset (*k, 0, sizeof (_ConfKey));

		(*k) -> key = strdup (key);
		(*k) -> value = strdup (value);
	}
}

static void
FreeConfSection (
 _ConfSection ** s)
{
	if (!*s)
		return;

	/*
	 */
	FreeConfSection (&(*s) -> next);

	FreeConfKey (&(*s) -> keys);
	free ((*s) -> name);
	free (*s);
	*s = NULL;
}

static void
FreeConfKey (
 _ConfKey ** k)
{
	if (!*k)
		return;

	/*
	 */
	free ((*k) -> key);
	free ((*k) -> value);
	free (*k);
	*k = NULL;
}

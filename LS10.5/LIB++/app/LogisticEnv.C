#ident	"$Id: LogisticEnv.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	Interface to Logistic Environment
 *
 *************************************************************
 *	Added Environment class with
 *		- UNIX subclass
 *		- Logistic subclass
 *
 */
#include	<assert.h>
#include	<string.h>

#include	<osdeps.h>

#include	<LogisticEnv.h>
#include	<LogisticEnvFile.h>

#include	<String.h>

/*
 *	LIbrary version string.
 *	We use LogisticEnv since nearly *ALL* Logistic programs refer
 *	to the Environment file
 */
#include	"Project"
#define		LIBRARYNAME		"C++ Application"
#include	<LibraryVersion.h>

/*
 *	Local stuff
 *
 *	 - use pointers for non-primitive data-types to avoid possible
 *	   problems with order of global object construction
 */
static int				envRefs = 0;		// reference count to envFile
static LogisticEnvFile *	envFile = NULL;

LogisticEnv::LogisticEnv (
 const char *	name) :
	Environment (name)
{
	exists = false;

	/*
	 *	Instantiate Environment file if required
	 */
	if (!envRefs)
	{
		assert (!envFile);
		envFile = new LogisticEnvFile;
	}
	envRefs++;

	/*
	 *	Look thru' the Environment file for matching name
	 */
	envFile -> First ();
	while (envFile -> Next ())
	{
		if (!strcmp (name, envFile -> Name ()))
		{
			Put (envFile -> Value ());
			exists = true;
			break;
		}
	}
}

LogisticEnv::~LogisticEnv ()
{
	envRefs--;
	assert (envRefs >= 0);
	if (!envRefs)
	{
		delete envFile;
		envFile = NULL;
	}
}

bool
LogisticEnv::Write ()
{
	/*
	 *	Re-scan the file to see whether we need to overwrite
	 *	or append the value
	 */
	envFile -> First ();
	while (envFile -> Next ())
	{
		if (!strcmp (Name (), envFile -> Name ()))
		{
			envFile -> Prev ();
			break;
		}
	}
	exists = true;
	return (envFile -> Write ());
}

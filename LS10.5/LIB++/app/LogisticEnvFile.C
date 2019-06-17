#ident	"$Id: LogisticEnvFile.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	Logistic Environment file.
 *		- the big flags file
 *
 *************************************************************
 *	$Log: LogisticEnvFile.C,v $
 *	Revision 5.0  2001/06/19 08:16:40  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 01:02:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:57:26  scott
 *	New files.
 *	
 *	Initial revision
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<osdeps.h>
#include	<liberr.h>
#include	<minor.h>

#include	<LogisticEnvFile.h>

#include	<String.h>				// containers

/*
 *	Magic stuff
 */
const char *	PSLEnvName = "PSL_ENV_NAME",
		   *	ProgPath = "PROG_PATH";

/*
 *	Local functions
 */
static const char *	GetFileName (const char *);

/*
 *	Public interface
 */
LogisticEnvFile::LogisticEnvFile (
 const char *	pathname) :
	WorkFile (GetFileName (pathname), sizeof (_tagLogisticEnv))
{
	memset (&data, 0, sizeof (_tagLogisticEnv));
}

const char *
LogisticEnvFile::Name (void) const
{
	return (data.name);
}

const char *
LogisticEnvFile::Value (void) const
{
	return (data.value);
}

const char *
LogisticEnvFile::Desc (void) const
{
	return (data.desc);
}

LogisticEnvFile &
LogisticEnvFile::Name (
 const char *	newval)
{
	memset (data.name, 0, sizeof (data.name));
	strncpy (data.name, newval, sizeof (data.name) - 1);
	return (*this);
}

LogisticEnvFile &
LogisticEnvFile::Value (
 const char *	newval)
{
	memset (data.value, 0, sizeof (data.value));
	strncpy (data.value, newval, sizeof (data.value) - 1);
	return (*this);
}

LogisticEnvFile &
LogisticEnvFile::Desc (
 const char *	newval)
{
	memset (data.desc, 0, sizeof (data.desc));
	strncpy (data.desc, newval, sizeof (data.desc) - 1);
	return (*this);
}

bool
LogisticEnvFile::Next ()
{
	if (!WorkFile::Next (&data))
		return (false);

	clip (data.name);
	clip (data.value);
	clip (data.desc);
	return (true);
}

bool
LogisticEnvFile::Prev ()
{
	if (!WorkFile::Prev (&data))
		return (false);

	clip (data.name);
	clip (data.value);
	clip (data.desc);
	return (true);
}

bool
LogisticEnvFile::Write ()
{
	return (WorkFile::Write (&data));
}

/*
 *	Support functions
 */
static const char *
GetFileName (
 const char *	path)
{
	if (path)
		return (path);

	char *	eptr;

	if ((eptr = getenv (PSLEnvName)))
		return (eptr);

	if ((eptr = getenv (ProgPath)))
	{
		static char	name [128];

		sprintf (name, "%s/BIN/LOGISTIC", eptr);
		return (name);
	}

	return (NULL);			// this will cause a deliberate crash
}


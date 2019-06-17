#ident	"$Id: Environ.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	Abstract class for Environment value tuples
 *
 *************************************************************
 *	$Log: Environ.C,v $
 *	Revision 5.0  2001/06/19 08:16:40  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 01:02:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:59:18  scott
 *	*** empty log message ***
 *	
 *	Revision 3.0  2000/10/12 13:40:10  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:40  jonc
 *	Version 10 start
 *
 *	Revision 2.6  1997/06/16 04:27:20  jonc
 *	Added accessor to const char * explicitly
 *
 *	Revision 2.5  1996/10/29 21:11:25  jonc
 *	Added direct const char * accessor
 *
 *	Revision 2.4  1996/07/30 01:32:20  jonc
 *	Added #ident directive
 *
 *	Revision 2.3  1996/06/13 23:53:42  jonc
 *	Added support for Exists() accessor
 *
 *	Revision 2.2  1996/03/18 04:57:01  jonc
 *	Added additional cast operator for simplicity
 *
 *	Revision 2.1  1996/03/11 21:31:32  jonc
 *	Reorganised Environment heirarchy.
 *	Added Environment class with
 *		- UNIX subclass
 *		- Logistic subclass
 *
 */
#include	<stdio.h>
#include	<stdlib.h>

#include	<osdeps.h>

#include	<Environ.h>

#include	<String.h>		// containers

Environment::Environment (
 const char *	eName) :
	envName (new String (eName)),
	envValue (new String),

	exists (true)
{
}

Environment::~Environment ()
{
	delete envName;
	delete envValue;
}

void
Environment::Put (
 const char *	val)
{
	*envValue = val;

	exists = true;
}

void
Environment::Put (
 int	val)
{
	char	numbuf [10];

	sprintf (numbuf, "%d", val);
	*envValue = numbuf;

	exists = true;
}

bool
Environment::Exists () const
{
	return (exists);
}

const String &
Environment::Name () const
{
	return (*envName);
}

const char *
Environment::chars () const
{
	return (envValue -> chars ());
}

Environment::operator int () const
{
	return (envValue -> empty () ? 0 : atoi (*envValue));
}

Environment::operator const char * () const
{
	return (*envValue);
}

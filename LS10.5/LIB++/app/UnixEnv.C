#ident	"$Id: UnixEnv.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	UNIX Environment
 *
 *************************************************************
 *	$Log: UnixEnv.C,v $
 *	Revision 5.0  2001/06/19 08:16:40  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 01:02:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:59:19  scott
 *	*** empty log message ***
 *	
 *	Revision 3.0  2000/10/12 13:40:11  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *
 *	Revision 2.6  1997/06/16 04:27:20  jonc
 *	Added accessor to const char * explicitly
 *
 *	Revision 2.5  1996/07/30 01:32:37  jonc
 *	Added #ident directive
 *
 *	Revision 2.4  1996/06/13 23:53:42  jonc
 *	Added support for Exists() accessor
 *
 *	Revision 2.3  1996/03/14 02:43:27  jonc
 *	Fixed bugs discovered by g++ compile
 *
 *	Revision 2.2  1996/03/11 21:31:33  jonc
 *	Reorganised Environment heirarchy.
 *	Added Environment class with
 *		- UNIX subclass
 *		- Logistic subclass
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<osdeps.h>
#include	<liberr.h>

#include	<UnixEnv.h>

#include	<String.h>

UnixEnv::UnixEnv (
 const char *	name,
 const char *	value) :
	Environment (name)
{
	if (value)
	{
		Put (value ? value : ::getenv (name));
	}
	else
	{
		const char *	env = ::getenv (name);

		if (env)
			Put (env);
		else
			exists = false;
	}
}

void
UnixEnv::Put (
 const char *	value)
{
	Environment::Put (value);

	/*
	 *	Also attempt to put it into the UNIX Environment
	 */
	String	envstr;

	cat (Name (), "=", chars (), envstr);

	if (putenv (strdup (envstr)))
		(*lib_error_handler) ("UnixEnv::Put",
			"putenv(%s) failed", (const char *) envstr);
}

void
UnixEnv::Put (
 int	value)
{
	char	number [10];

	sprintf (number, "%d", value);
	Put (number);
}

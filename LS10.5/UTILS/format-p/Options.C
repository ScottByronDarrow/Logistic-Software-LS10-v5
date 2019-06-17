#ident	"$Id: Options.C,v 5.0 2001/06/19 08:22:46 robert Exp $"
/*
 *	Option settings from stdin.
 *
 *******************************************************************************
 *	$Log: Options.C,v $
 *	Revision 5.0  2001/06/19 08:22:46  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:08  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.1  2000/08/09 23:47:49  johno
 *	Add email= and subject= option clauses. When specified, cause output to
 *	be sent to sendmail rather than print spooler.
 *	
 *	Revision 2.0  2000/07/15 09:15:11  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<stdlib.h>
#include	<stdio.h>
#include	<stddef.h>

#include	<osdeps.h>

#include	"LineReader.h"
#include	"Options.h"

/*
 *
 */
Options::Options () :
	lpno (0)
{
}

void
Options::Read (
 LineReader &	reader)
{
	String	key, value;

	reader.SectionMarker ("options");

	while (reader.ReadSection ())
	{
		reader.KeyValue (key, value);

		/*
		 *	Work thu' the keywords we know about
		 */
		if (key == "lpno")
		{
			lpno = atoi (value);
			continue;
		}

		if (key == "file")
		{
			layoutfile = value;
			continue;
		}

		if (key == "output")
		{
			outputname = value;
			continue;
		}

		if (key == "optionblock")
		{
			optionblock = value;
			continue;
		}

		if (key == "email")
		{
			email = value;
			continue;
		}

		if (key == "subject")
		{
			subject = value;
			continue;
		}
	}
}

int
Options::LpNo () const
{
	return lpno;
}

const char *
Options::LayoutFilePath () const
{
	return layoutfile.empty () ? NULL : layoutfile.chars ();
}

const char *
Options::Filter () const
{
	return outputname.empty () ? NULL : outputname.chars ();
}

const char *
Options::OptionBlock () const
{
	return optionblock.empty () ? NULL : optionblock.chars ();
}

const char *
Options::Email () const
{
	return email.empty () ? NULL : email.chars ();
}

const char *
Options::Subject () const
{
	return subject.empty () ? NULL : subject.chars ();
}

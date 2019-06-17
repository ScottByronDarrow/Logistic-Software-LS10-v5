#ident	"$Id: LineReader.C,v 5.0 2001/06/19 08:22:46 robert Exp $"
/*
 *	Reads input from stdin
 *
 *******************************************************************************
 *	$Log: LineReader.C,v $
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
 *	Revision 2.0  2000/07/15 09:15:11  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<stdio.h>
#include	<string.h>

#include	<osdeps.h>

#include	"LineReader.h"

LineReader::LineReader () :
	insection (false),
	lineempty (false)
{
}

void
LineReader::SectionMarker (
 const char *	s)
{
	section = s;
	insection = false;
	key = value = "";
}

bool
LineReader::ReadSection (
 bool	skipempty)
{
	char	line [256];
	bool	skipsection = false;

	if (!heldsection.empty ())
	{
		if (heldsection != section)
			return false;

		heldsection = "";
		insection = true;
	}

	while (fgets (line, sizeof (line), stdin))
	{
		line [strlen (line) - 1] = '\0';			// strip trailing \n

		if (!line [0] && skipempty)					// skip empty lines
			continue;

		if (line [0] == '#')
		{
			if (insection)
			{
				heldsection = line + 1;
				insection = false;
				return false;
			}

			if (section != line + 1)
			{
				skipsection = true;
			} else
			{
				insection = true;
				continue;
			}
		}

		if (skipsection || !insection)
			continue;

		if (line [0])
		{
			/*
			 *	Separate out the key & value
			 */
			String			sline = line;
			const char *	Seperator = "=";

			if (sline.contains (Seperator))
			{
				key = sline.before (Seperator);
				value = sline.after (Seperator);
			} else
			{
				key = sline;
				value = "";
			}

			lineempty = false;

		} else
			lineempty = true;

		return true;
	}

	return false;
}

bool
LineReader::EmptyLine () const
{
	return lineempty;
}

void
LineReader::KeyValue (
 String &	bkey,
 String &	bvalue) const
{
	bkey = key;
	bvalue = value;
}

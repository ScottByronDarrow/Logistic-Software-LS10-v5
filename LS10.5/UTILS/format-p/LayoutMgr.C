#ident	"$Id: LayoutMgr.C,v 5.0 2001/06/19 08:22:46 robert Exp $"
/*
 *	Manages the layouts and sequence of blocks to print
 *
 *******************************************************************************
 *	$Log: LayoutMgr.C,v $
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
#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>

#include	<osdeps.h>

#include	<minor.h>

#include	"LayoutMgr.h"
#include	"LineReader.h"
#include	"Options.h"
#include	"OutputRecvr.h"

#define	LAYOUTLINEMAX	1024

/*
 *
 */
static const char
	*OptionsMarker			= "options",
	*ReportHeaderMarker		= "report-header",
	*ReportTrailerMarker	= "report-trailer",
	*PageHeaderMarker		= "page-header",
	*PageTrailerMarker		= "page-trailer",
	*PageLastTrailerMarker	= "page-trailer-last",
	*BodyHeaderMarker		= "body-header",
	*BodyTrailerMarker		= "body-trailer",
	*BodyMarker				= "body";

static const char Directive	= '%';
static const char
	*DirectiveReset			= "reset";

/*
 *
 */
static void	ReadOption (FILE *,
				int & pitch,
				int & pagelen, int & pagehdr,
				int & bodyb, int & bodye,
				int & pagetrl),
			ReadSection (FILE *, CArray <String> &);

static bool	ValidKeyChar (char);

/*
 *
 */
LayoutMgr::Section *
LayoutMgr::GetSection (
 const char * name) const
{
	for (int i = 0; i < sections.Count (); i++)
		if (sections.Elem (i).name == name)
			return &sections.Elem (i);
	return NULL;
}

void
LayoutMgr::SetDefaultSections ()
{
	reporthdr = GetSection (ReportHeaderMarker);
	reportend = GetSection (ReportTrailerMarker);
	pagehdr = GetSection (PageHeaderMarker);
	pageend = GetSection (PageTrailerMarker);
	pageendLast = GetSection (PageLastTrailerMarker);
	bodyhdr = GetSection (BodyHeaderMarker);
	bodyend = GetSection (BodyTrailerMarker);
	body = GetSection (BodyMarker);
}

void
LayoutMgr::PrintLine (
 const char *	line)
{
	/*
	 *	Expand the line and write it out
	 */
	String	expanded;

	for (int i = 0; line [i]; i++)
	{
		if (line [i] == '.' && isalpha (line [i + 1]))
		{
			/*
			 *	Extract the keyword out, format and all.
			 *
			 *	We're assuming that the keyword is delimited
			 *	by whitespace and punctuation
			 */
			String	rawkey, value;
			bool	hasdot = false;

			do
			{
				rawkey += line [++i];

				if (line [i] == '.')
					hasdot = true;
				if (hasdot && line [i + 1] == '.')
					break;

			}	while (ValidKeyChar (line [i + 1]));

			expanded += vregister.DecodeValue (rawkey, value);

		} else
			expanded += line [i];
	}

	output.Write (expanded);
	lineno++;
}

void
LayoutMgr::PrintSection (
 const Section * section,
 int atline)
{
	if (!section)
		return;

	if (atline)
	{
		output.Advance (atline - lineno);
		lineno = atline;
	}

	for (int i = 0; i < section -> content.Count (); i++)
		PrintLine (section -> content.Elem (i));
}

void
LayoutMgr::CheckForBreaks ()
{
	bool	justprintedrpthdr = false;

	if (pagebodyend && lineno > pagebodyend)
	{
		PrintSection (pageend, pagetrailerbeg);
		output.Advance (pagelen - lineno + 1);		// move to end of page
		lineno = 0;
	}

	if (!rpthdrprinted)
	{
		PrintSection (reporthdr, 0);
		rpthdrprinted = true;
		justprintedrpthdr = true;
	}

	if (!lineno || justprintedrpthdr)
	{
		char	pagebuf [64];

		sprintf (pagebuf, "%d", ++pageno);
		vregister.Add ("pageno", pagebuf);

		if (!lineno)
			lineno = 1;
		else
		{
			lineno++;				// correct for maladjusted report line-count
			if (pagelen && pagehdrbeg && lineno > pagehdrbeg)
			{
				/*
				 *	The report header has managed to overflow
				 *	past pagehdrbeg. We need to feed another
				 *	page in.
				 */
				output.Advance (pagelen - lineno + 1);
				lineno = 1;
			}
		}

		PrintSection (pagehdr, pagehdrbeg);
		output.Advance (pagebodybeg - lineno);		// move to start of body
		lineno = pagebodybeg;
	}
}

void
LayoutMgr::PrintBody ()
{
	if (!pageno && bodyhdr)
		for (int i = 0; i < bodyhdr -> content.Count (); i++)
		{
			CheckForBreaks ();
			PrintLine (bodyhdr -> content.Elem (i));
		}

	if (body)
	{
		for (int i = 0; i < body -> content.Count (); i++)
		{
			CheckForBreaks ();
			PrintLine (body -> content.Elem (i));
		}
	}
}

void
LayoutMgr::PrintLastPage (
 bool trailingbody)
{
	if (trailingbody)
		PrintBody ();

	if (bodyend)
	{
		for (int i = 0; i < bodyend -> content.Count (); i++)
		{
			CheckForBreaks ();
			PrintLine (bodyend -> content.Elem (i));
		}
	}

	if (pageendLast)
		PrintSection (pageendLast, pagetrailerbeg);
	else
		PrintSection (pageend, pagetrailerbeg);

	output.Advance (pagelen - lineno + 1);	// move to end of page
}

void
LayoutMgr::InterpretDirective (
 const char * directive,
 const char * value,
 bool hastrailingbody)
{
	if (!strcmp (directive, DirectiveReset))
	{
		/*
		 *	Print last page
		 */
		if (pageno)
			PrintLastPage (hastrailingbody);

		/*
		 *	Reset all control variables
		 */
		pageno = lineno = 0;
		vregister.Reset ();
		SetDefaultSections ();
		return;
	}

	/*
	 *	Try the directive against our std sections
	 */
	SubmitDirective (ReportHeaderMarker, directive, reporthdr, value) ||
	SubmitDirective (ReportTrailerMarker, directive, reportend, value) ||
	SubmitDirective (PageHeaderMarker, directive, pagehdr, value) ||
	SubmitDirective (PageTrailerMarker, directive, pageend, value) ||
	SubmitDirective (PageLastTrailerMarker, directive, pageendLast, value) ||
	SubmitDirective (BodyHeaderMarker, directive, bodyhdr, value) ||
	SubmitDirective (BodyTrailerMarker, directive, bodyend, value) ||
	SubmitDirective (BodyMarker, directive, body, value);
}

bool
LayoutMgr::SubmitDirective (
 const char * sectionname,
 const char * directive,
 Section * & section,
 const char * altsection)
{
	/*
	 *	Directive to use alternate report section
	 *
	 *	These are of the form:
	 *
	 *		%stdsection=newsection
	 *
	 *	where stdsection is one of the listed predefined sections
	 *	aside from `options'
	 */
	if (strcmp (sectionname, directive))
		return false;

	if (!altsection [0] ||							// reset
		!(section = GetSection (altsection)))		// attempt change
	{
		/*
		 *	Alternative section not found, use the default one
		 */
		section = GetSection (sectionname);
	}

	return true;
}

/*
 *
 */
LayoutMgr::LayoutMgr (
 const Options &	options,
 OutputRecvr &		outputR) :

	output (outputR),

	usable (true),

	pagelen (0),
	pagehdrbeg (0),
	pagebodybeg (0),
	pagebodyend (0),
	pagetrailerbeg (0),

	reporthdr (NULL),
	reportend (NULL),
	pagehdr (NULL),
	pageend (NULL),
	pageendLast (NULL),
	bodyhdr (NULL),
	bodyend (NULL),
	body (NULL),

	rpthdrprinted (false),
	pageno (0), lineno (0)
{
	/*
	 *	Read in the layout file
	 */
	FILE *			file = NULL;
	const char *	path = options.LayoutFilePath ();
	char			line [LAYOUTLINEMAX];
	int				pitch = 0;

	bool			optionpresent = false;
	const char *	optionblock = options.OptionBlock () ?
						options.OptionBlock () :
						OptionsMarker;

	if (!path ||
		!(file = fopen (path, "r")))
	{
		usable = false;
		return;
	}

	while (fgets (line, sizeof (line), file))
	{
		line [strlen (line) - 1] = '\0';	// trim off trailing newline

		if (!line [0] ||					// skip empty-lines
			line [0] == '#')				// skip comment-lines
		{
			continue;
		}

		if (!strcmp (line, optionblock))
		{
			ReadOption (file,
				pitch,
				pagelen, pagehdrbeg,
				pagebodybeg, pagebodyend,
				pagetrailerbeg);

			optionpresent = true;
		}
		else
		{
			/*
			 *	Add the section in
			 */
			Section	newsection;

			newsection.name = line;
			ReadSection (file, newsection.content);
			sections.Add (newsection);
		}
	}

	if (!optionpresent && options.OptionBlock ())
	{
		/*
		 *	Failed to find the option-block defined from the
		 *	input-stream. Let's retry with the std OptionBlock name
		 */
		rewind (file);

		while (fgets (line, sizeof (line), file))
		{
			line [strlen (line) - 1] = '\0';	// trim off trailing newline
			if (!line [0] ||					// skip empty-lines
				line [0] == '#')				// skip comment-lines
			{
				continue;
			}

			if (!strcmp (line, OptionsMarker))
			{
				ReadOption (file,
					pitch,
					pagelen, pagehdrbeg,
					pagebodybeg, pagebodyend,
					pagetrailerbeg);
			}
			else
			{
				/*	Discard
				 */
				Section	discard;

				discard.name = line;
				ReadSection (file, discard.content);
			}
		}
	}

	fclose (file);

	SetDefaultSections ();			// Initialise the sections
	output.UsePitch (pitch);
}

bool
LayoutMgr::Usable () const
{
	return usable;
}

void
LayoutMgr::Read (
 LineReader &	reader)
{
	bool	somethingread = false,
			dataread = false,
			lastwasempty = false,
			lastwasdirective = false;
	String	key, value;

	reader.SectionMarker ("data");

	while (reader.ReadSection (false))
	{
		if (reader.EmptyLine ())
		{
			/*
			 *	Flush body output 
			 */
			PrintBody ();

			lastwasempty = true;
		} else
		{
			reader.KeyValue (key, value);

			if (key.firstchar () == '%')
			{
				InterpretDirective (key.chars () + 1, value,
					!lastwasempty && !lastwasdirective);

				lastwasdirective = true;
			} else
			{
				vregister.Add (key, value);			//	Update the register

				dataread = true;
				lastwasdirective = false;
			}
			lastwasempty = false;
		}

		somethingread = true;
	}

	/*
	 *	Flush trailing sections
	 */
	if (dataread)
	{
		/*
		 *	Body and Page trailers only make sense if we had
		 *	some data read in
		 */
		PrintLastPage (!lastwasempty && !lastwasdirective);
	}

	if (somethingread)
	{
		/*
		 *	Report trailers should always come out
		 *	even if no data has been read (ie directives, blank lines)
		 */
		PrintSection (reportend, 0);
	}
}

/*
 *
 */
static void
ReadOption (
 FILE *	fp,
 int & pitch,
 int & pagelen,
 int & pagehdrbeg,
 int & bodybeg,
 int & bodyend,
 int & pagetrlbeg)
{
	char	line [128];
	bool	insection = false;

	while (fgets (line, sizeof (line), fp))
	{
		line [strlen (line) - 1] = '\0';	// trim off trailing newline

		if (insection)
		{
			int		value;
			char	key [64];

			if (line [0] == '}')
				break;

			if (sscanf (line, "%[^=]= %d", key, &value) != 2)
				continue;

			clip (lclip (key));

			if (!strcmp (key, "pitch"))
				pitch = value;
			else if (!strcmp (key, "pagelength"))
				pagelen = value;
			else if (!strcmp (key, "pageheader-start"))
				pagehdrbeg = value;
			else if (!strcmp (key, "body-start"))
				bodybeg = value;
			else if (!strcmp (key, "body-end"))
				bodyend = value;
			else if (!strcmp (key, "pagetrailer-start"))
				pagetrlbeg = value;

		} else if (line [0] == '{')
			insection = true;
	}

	if (!pagelen)
		bodyend = pagetrlbeg = 0;
}

static void
ReadSection (
 FILE *				fp,
 CArray <String> &	buf)
{
	char	line [LAYOUTLINEMAX];
	bool	insection = false;

	buf.Clear ();
	while (fgets (line, sizeof (line), fp))
	{
		line [strlen (line) - 1] = '\0';	// trim off trailing newline

		if (insection)
		{
			if (line [0] == '}')
				return;

			buf.Add (line);

		} else if (line [0] == '{')
			insection = true;
	}
}

static bool
ValidKeyChar (
 char	c)
{
	if (!c || isspace (c))
		return false;

	if (isalpha (c) || isdigit (c))
		return true;

	return c == '_' || c == ':' || c == '.' || c == '-';
}

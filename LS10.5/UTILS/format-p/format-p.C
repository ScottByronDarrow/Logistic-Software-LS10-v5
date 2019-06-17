#ident	"$Id: format-p.C,v 5.1 2002/09/09 05:13:39 scott Exp $"
/*
 *	Extendible text formatter, built on pformat
 *
 *	The program works on accepting stuff from stdin
 *
 *	It basically requires 2 main sections from stdin
 *
 *		#options
 *		#data
 *
 *	The options sets up the connection
 *	The data drives the output
 *
 *	Each batch of data is separated by an empty-line
 *	Closing the input stream indicates end-of-data
 *
 *******************************************************************************
 *	$Log: format-p.C,v $
 *	Revision 5.1  2002/09/09 05:13:39  scott
 *	.
 *	
 *	Revision 5.0  2001/06/19 08:22:47  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:13  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:12  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdeps.h>

#include	"Project"

#include	"LayoutMgr.h"
#include	"LineReader.h"
#include	"Options.h"
#include	"PFormatter.h"
#include	"GenericFilter.h"

int main (
 int argc,
 char * argv [])
{
	int	c;

	while ((c = getopt (argc, argv, "v")) != EOF)
	{
		switch (c)
		{
		case 'v':
			fprintf (stderr, "%s: Version %s%s\n",
				argv [0],
				ProjectState, ProjectRevision);
			return EXIT_SUCCESS;
		}
	}

	/*
	 *	The real stuff
	 */
	LineReader	reader;
	Options		options;

	/*
	 *	Read in options sections
	 */
	options.Read (reader);

	/*
	 *	Ensure that the options that we
	 *	get are usable by all the related objects
	 */
	OutputRecvr *	output = NULL;

	if (!options.Filter ())
	{
		/*
		 *	Our default filter is pformat
		 */
		output = new PFormatter (options);
		if (!output -> Usable ())
		{
			fprintf (stderr, "%s: pformat options not set up\n", argv [0]);
			return EXIT_FAILURE;
		}
	} else
	{
		output = new GenericFilter (options.Filter ());
		if (!output -> Usable ())
		{
			fprintf (stderr, "%s: Generic filter failed\n", argv [0]);
			return EXIT_FAILURE;
		}
	}

	/*
	 *
	 */
	LayoutMgr	layout (options, *output);

	if (!layout.Usable ())
	{
		fprintf (stderr, "%s: layout options not set up\n", argv [0]);
		return EXIT_FAILURE;
	}
	layout.Read (reader);

	/*
	 *	Cleanup
	 */
	delete output;

	return EXIT_SUCCESS;
}

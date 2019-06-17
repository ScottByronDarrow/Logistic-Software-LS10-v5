#ident	"$Id: PFormatter.C,v 5.0 2001/06/19 08:22:47 robert Exp $"
/*
 *	Emulates some functionality of pformat
 *
 *******************************************************************************
 *	$Log: PFormatter.C,v $
 *	Revision 5.0  2001/06/19 08:22:47  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.2  2000/11/22 06:02:10  scott
 *	Updated to add P2 string.
 *	
 *	Revision 3.1  2000/11/10 05:01:11  scott
 *	Updated to allow for local printing.
 *	
 *	Revision 3.0  2000/10/10 12:24:11  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.1  2000/08/09 23:47:49  johno
 *	Add email= and subject= option clauses. When specified, cause output to
 *	be sent to sendmail rather than print spooler.
 *	
 *	Revision 2.0  2000/07/15 09:15:12  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.2  2000/06/22 05:56:56  morgan
 *	Added facility to enable the handling of lp and lpr devices through LPR_FLAGS and LPR_PROGRAM
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	<LogisticEnv.h>

#include	<osdeps.h>

#include	<LpNumber.h>

#include	"Options.h"
#include	"PFormatter.h"

#define	ESCAPE_CHAR	0x1b

/*
 *	Markers in config file
 */
static const char
	*P_Initialize	= "I1",
	*P_DeInitialize	= "I2",
	*P_Pitch10		= "P0",
	*P_Pitch12		= "P2",
	*P_Pitch16		= "P6";

/*
 *	Local functions
 */
static void	ReadPrinterFile (const char * pathname,
				String & p_init , String & p_dinit,
				String & pitch10, String & pitch12, String & pitch16);

/*
 *
 */
PFormatter::PFormatter (
 const Options &	options)
{
	/*
	 *	Determine output printer queue and use it as output filter.
	 *	Determine printer type so load printer-escape sequences.
	 */
	LpNumber			lplist;
	String				str;
	const char*			sptr;
	const char*			sptr2;
	String				lpFlags;
	String 				lpProgs;
	String				space;
	String				email = options.Email();
	String				subject = options.Subject();
	String				SendMail;

	space = " ";

	/*
	 *	Check if email option
	 */
	if (!email.empty())
	{
		sptr = getenv ("SENDMAIL");
		if (sptr)
			SendMail = sptr;
		else
			SendMail = "/usr/lib/sendmail";

		cat (SendMail, " ", email, str);
		out = popen( str, "w" );
		fprintf(out, "subject: %s\n\n", subject.chars());
		return;
	}

	const tagLpInfo &	printer = lplist.LpInfo (options.LpNo ());
	/*
	 *	Determine escape sequences
	 */
	cat (getenv ("PROG_PATH"), "/BIN/MENUSYS/PRINT/", printer.pType, str);
	ReadPrinterFile (str, p_init, p_dinit, p_10pitch, p_12pitch, p_16pitch);

	/*
	 *	Build up command pipe
	 */
	
	sptr =  getenv ("LPR_FLAGS");
	sptr2 = getenv ("LPR_PROGRAM");
	LogisticEnv e_lcl_queue ("LCL_QUEUE");

	if (!strncmp (printer.qName, "local", 5))
		sptr2	=	e_lcl_queue.chars ();
	else
		sptr2 = getenv ("LPR_PROGRAM");

	if (sptr)
	{
		lpFlags = sptr + space;
	}
	else
	{
		lpFlags = "-d ";
	}

	if (sptr2)
	{
		lpProgs = sptr2 + space;
	}
	else
	{
		lpProgs = "lp ";
	}

	if (!strncmp (printer.qName, "local", 5))
		out = popen (lpProgs, "w");
	else
	{
		cat (lpProgs, lpFlags, printer.qName, str);
		out = popen (str, "w");
	}

	fputs (p_init, out);		//	Initialization string
}

void
PFormatter::UsePitch (
 int pitch)
{
	switch (pitch)
	{
	case 10:
		fputs (p_10pitch, out);
		break;

	case 12:
		fputs (p_12pitch, out);
		break;

	case 16:
		fputs (p_16pitch, out);
		break;
	}
}

/*
 *	Support functions
 */
static void
ReadPrinterFile (
 const char * pathname,
 String & p_init,
 String & p_dinit,
 String & p_pitch10,
 String & p_pitch12,
 String & p_pitch16)
{
	FILE *	f = fopen (pathname, "r");
	char	line [512];

	if (!f)
		return;

	while (fgets (line, sizeof line, f))
	{
		line [strlen (line) - 1] = '\0';		// remove new-line
		if (!line [0] || line [0] == '#')		// skip comments and padding
			continue;

		/*
		 *	Process line
		 */
		const char	Sep = ':';

		String		raw (line);
		String *	cooked = NULL;
		int			freq = raw.freq (Sep);

		if (!freq)
			continue;

		cooked = new String [freq];				// initial guess
		freq = split (raw, cooked, freq, Sep);

		/*
		 *	Inspect the components for stuff we know about
		 */
		for (int i = 0; i < freq; i++)
		{
			const char	ValSep = '=';

			String	key		= cooked [i].before (ValSep),
					rvalue	= cooked [i].after (ValSep);

			if (key.empty () || rvalue.empty ())
				continue;

			/*
			 *	Tranlate raw-value
			 *		- \E, \\, \0nn
			 *		- ^x
			 */
			String		value;
			unsigned	len = rvalue.length ();

char asdf [512];

memset (asdf, 0, sizeof asdf);
strcpy (asdf, rvalue);
			for (unsigned j = 0; j < len; j++)
			{
				switch (rvalue [j])
				{
				case '\\':					// special escape-sequence
					if (j + 1 < len)
					{
						j++;

						/*
						 *	Secondary interpretation
						 */
						switch (rvalue [j])
						{
						case 'E':			// escape-char
							value += ESCAPE_CHAR;
							break;

						case '0':			// octal-number
						case '1':
						case '2':
						case '3':
							if (j + 2 < len &&
								isdigit (rvalue [j + 1]) &&
								isdigit (rvalue [j + 2]))
							{
								value +=
									(rvalue [j]     - '0') * 64 +
									(rvalue [j + 1] - '0') *  8 +
									 rvalue [j + 2] - '0';

								j += 2;
							} else
								value += '0';
							break;

						default:
							value += rvalue [j];
						}

					} else
						value += '\\';
					break;

				case '^':					// control characters
					if (j + 1 < len)
					{
						value += toupper (rvalue [j + 1]) - 'A' + 1;
						j++;
					} else
						value += '^';
					break;

				default:
					value += rvalue [j];
				}
			}

			/*
			 */
			if (key == P_Initialize)
				p_init = value;
			if (key == P_DeInitialize)
				p_dinit = value;
			else if (key == P_Pitch10)
				p_pitch10 = value;
			else if (key == P_Pitch12)
				p_pitch12 = value;
			else if (key == P_Pitch16)
				p_pitch16 = value;
		}

		delete [] cooked;
	}

	fclose (f);
}

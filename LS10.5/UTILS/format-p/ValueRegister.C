#ident	"$Id: ValueRegister.C,v 5.0 2001/06/19 08:22:47 robert Exp $"
/*
 *	Storage for value tuples
 *
 *******************************************************************************
 *	$Log: ValueRegister.C,v $
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
#include	<stdio.h>
#include	<stdlib.h>

#include	<osdeps.h>

#include	"ValueRegister.h"

/*
 *
 */
const char *
ValueRegister::Value (
 const char *	key) const
{
	for (int i = entries.Count () - 1; i >= 0; i--)
		if (entries.Elem (i).key == key)
			return entries.Elem (i).value;

	return "";
}

/*
 *
 */
void
ValueRegister::Reset ()
{
	entries.Clear ();
}

void
ValueRegister::Add (
 const char *	key,
 const char *	value)
{
	/*
	 *	Overwrite value if it exists
	 */
	for (int i = entries.Count () - 1; i >= 0; i--)
	{
		if (entries.Elem (i).key == key)
		{
			entries.Elem (i).value = value;
			return;
		}
	}

	/*
	 *	Add
	 */
	Entry	newentry;

	newentry.key = key;
	newentry.value = value;
	entries.Add (newentry);
}

String &
ValueRegister::DecodeValue (
 const char *	keystr,
 String &		value) const
{
	/*
	 *	Decode the possible key+format string
	 */
	String			keyStr (keystr);
	const char *	fmtspec = ":",
			   *	dot = ".";

	if (!keyStr.contains (fmtspec))
		return value = Value (keystr);

	/*
	 *
	 */
	bool	rightjust = false,
			hasdot = false;
	int		len = 0, prec = 0;
	String	key = keyStr.before (fmtspec),
			format = keyStr.after (fmtspec);

	if (format.contains (dot))
	{
		hasdot = true;
		sscanf (format.chars (), "%d.%d", &len, &prec);
	} else
		sscanf (format.chars (), "%d", &len);

	if (len < 0)
	{
		len = abs (len);
		rightjust = true;
	}

	if (prec + 1 > len)					// sanity checks
	{
		prec = len = 0;
		rightjust = false;
	}

	/*
	 *	Since we're dealing with pure text
	 *	we have to handle our own formatting...
	 */
	value = Value (key);

	if (hasdot)
	{
		/*
		 *	Adjust for '.'
		 */
		if (value.contains (dot))
		{
			if (prec)
			{
				String	predot = value.before (dot),
						postdot = value.after (dot);
				int		postlen = postdot.length ();

				if (postlen > prec)
					postdot = postdot.before (prec);
				else
					for (int i = postlen; i < prec; i++)
						postdot += '0';

				cat (predot, ".", postdot, value);
			} else
			{
				/*
				 *	value has '.', but prec = 0.
				 *	-> we remove everything after the dot
				 */
				value = value.before (dot);
			}

		} else if (prec)
		{
			/*
			 *	We need to pad out with 0
			 */
			value += '.';
			for (int i = 0; i < prec; i++)
				value += '0';
		}
	}

	/*
	 *	Left/Right justify if required
	 */
	if (len)
	{
		if (value.length () < (unsigned) len)
		{
			if (rightjust)
				for (int i = value.length (); i < len; i++)
					value.prepend (' ');
			else
				for (int i = value.length (); i < len; i++)
					value += ' ';

		} else if (value.length () > (unsigned) len)
		{
			/*
			 *	Value too big
			 */
			if (rightjust)
			{
				/*
				 *	More often than not, these are numbers
				 *	so we have an overflow indicator
				 */
				value = "";
				for (int i = 0; i < len; i++)
					value += '*';
			} else
			{
				/*
				 *	Chop off the end
				 */
				value = value.before (len);
			}
		}
	}

	return value;
}

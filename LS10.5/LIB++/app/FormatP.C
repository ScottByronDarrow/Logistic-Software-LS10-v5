#ident	"$Id: FormatP.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	Simple front end to format-p
 *
 *******************************************************************************
 *	$Log: FormatP.C,v $
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
 *	Revision 1.4  2000/08/09 23:50:59  johno
 *	Add email functionality to the FormatP object: Add subject and email
 *	address attributes. Pass these to format-p.
 *	
 *	Revision 1.3  1999/12/12 22:39:39  jonc
 *	Added support for truncated date-values.
 *	
 *	Revision 1.3  1999/12/12 22:39:39  jonc
 *	Added support for truncated date-values.
 *	
 *	Revision 1.2  1999/12/09 21:46:52  jonc
 *	Added mask for 2 digit years.
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from V10)
 *	
 */
#include	<assert.h>
#include	<stdio.h>

#include	<osdeps.h>

#include	<ColInfo.h>
#include	<Table.h>
#include	<Query.h>

#include	<Date.h>
#include	<Money.h>
#include	<String.h>

#include	<FormatP.h>

/*
 *
 */
void
FormatP::Init (
 const char * layoutfile,
 const char * altoptionblock)
{
	output = popen ("format-p", "w");
	assert (output);

	fprintf (output, "#options\n");
	fprintf (output, "file=%s\n", layoutfile);

	if (altoptionblock)
		fprintf (output, "optionblock=%s\n", altoptionblock);
}

void
FormatP::SubmitColumn (
 const Query & qry,
 const char * column,
 const ColumnInfo & info)
{
	short	short_val;
	long	long_val;
	double	double_val;
	float	float_val;

	Date	dvalue;
	String	svalue;
	Number	nvalue;
	Money	mvalue;
	char	numbuf [128];

	switch (info.type)
	{
	case ColChar:
		qry.GetCol (column, &svalue);
		for (unsigned i = svalue.length (); i < info.length; i++)
			svalue += ' ';
		break;

	case ColShort:
		qry.GetCol (column, &short_val);
		sprintf (numbuf, "%d", short_val);
		svalue = numbuf;
		break;

	case ColSerial:
	case ColLong:
		qry.GetCol (column, &long_val);
		sprintf (numbuf, "%ld", long_val);
		svalue = numbuf;
		break;

	case ColDouble:
		qry.GetCol (column, &double_val);
		Submit (column, double_val);
		return;

	case ColFloat:
		qry.GetCol (column, &float_val);
		Submit (column, float_val);
		return;

	case ColDecimal:
		qry.GetCol (column, &nvalue);
		nvalue.Get (svalue);
		break;

	case ColDate:
		qry.GetCol (column, &dvalue);
		Submit (column, dvalue);
		return;

	case ColMoney:
		qry.GetCol (column, &mvalue);
		mvalue.Get (svalue);
		break;

	default:
		svalue = "**UNTRANSLATABLE**";
	}
	Submit (column, svalue);
}

void
FormatP::SubmitClear (
 const char * column,
 const ColumnInfo & info)
{
	String			str;
	const Date		NullDate;
	const char *	NullStr = "";

	switch (info.type)
	{
	case ColChar:
		str = NullStr;
		for (unsigned i = 0; i < info.length; i++)
			str += ' ';
		Submit (column, str);
		break;
	case ColDate:
		Submit (column, NullDate);
		break;
	default:
		Submit (column, NullStr);
	}
}

void
FormatP::UseSection (
 const char * base,
 const char * alt)
{
	fprintf (output, "%%%s=%s\n", base, alt ? alt : "");
}

/*
 *
 */
FormatP::FormatP (
 const char * layout,
 const char * outstr,
 const char * optionblock) :
	output (NULL)
{
	Init (layout, optionblock);
	fprintf (output, "output=%s\n", outstr);
	fprintf (output, "#data\n");
}

FormatP::FormatP (
 const char * layout,
 int lpno,
 const char * optionblock) :
	output (NULL)
{
	Init (layout, optionblock);
	fprintf (output, "lpno=%d\n", lpno);
	fprintf (output, "#data\n");
}

FormatP::FormatP (
 const char * layout,
 int lpno,
 const char * address,
 const char * subject) :
	output (NULL)
{
	Init (layout, 0);
	fprintf (output, "lpno=%d\n", lpno);
	fprintf (output, "email=%s\n", address);
	fprintf (output, "subject=%s\n", subject);
	fprintf (output, "#data\n");
}

FormatP::~FormatP ()
{
	pclose (output);
}

void
FormatP::Reset ()
{
	fprintf (output, "%%reset\n");
}

void
FormatP::UsePageHeader (
 const char * alt)
{
	UseSection ("page-header", alt);
}

void
FormatP::UsePageTrailer (
 const char * alt)
{
	UseSection ("page-trailer", alt);
}

void
FormatP::UseBody (
 const char * alt)
{
	UseSection ("body", alt);
}

void
FormatP::Submit (
 const Query &	qry)
{
	const Table &	table = qry.QTable ();
	ColumnInfo		info;

	for (int i = 0; table.ColName (i); i++)
		if (table.ColInfo (i, info))
			SubmitColumn (qry, table.ColName (i), info);

	for (int i = 0; table.UColName (i); i++)
		if (table.UColInfo (i, info))
			SubmitColumn (qry, table.UColName (i), info);
}

void
FormatP::Submit (
 const char * key,
 const char * rvalue)
{
	fprintf (output, "%s=%s\n", key, rvalue);
}

void
FormatP::Submit (
 const char * key,
 float value)
{
	char	numbuf [128];

	sprintf (numbuf, "%.2f", value);
	Submit (key, numbuf);
}

void
FormatP::Submit (
 const char * key,
 double value)
{
	char	numbuf [128];

	sprintf (numbuf, "%.2f", value);
	Submit (key, numbuf);
}

void
FormatP::Submit (
 const char * rawkey,
 const Date & value)
{
	String	key;
	String	str;

	Submit (rawkey, value.Get (str));

	cat (rawkey, "_dmy", key);
	Submit (key, value.Get ("%d/%m/%Y", str));

	cat (rawkey, "_dm2y", key);
	Submit (key, value.Get ("%d/%m/%y", str));

	cat (rawkey, "_mdy", key);
	Submit (key, value.Get ("%m/%d/%Y", str));

	cat (rawkey, "_md2y", key);
	Submit (key, value.Get ("%m/%d/%y", str));

	cat (rawkey, "_dMy", key);
	Submit (key, value.Get ("%d %B %Y", str));

	cat (rawkey, "_dM2y", key);
	Submit (key, value.Get ("%d %B %y", str));

	cat (rawkey, "_d3My", key);
	Submit (key, value.Get ("%d %b %Y", str));

	cat (rawkey, "_d3M2y", key);
	Submit (key, value.Get ("%d %b %y", str));

	/*
	 *	Supported truncated values
	 */
	cat (rawkey, "%s_dm", key);
	Submit (key, value.Get ("%d/%m", str));

	cat (rawkey, "%s_md", key);
	Submit (key, value.Get ("%m/%d", str));

	cat (rawkey, "%s_dM", key);
	Submit (key, value.Get ("%d %B", str));

	cat (rawkey, "%s_Md", key);
	Submit (key, value.Get ("%B %d", str));

	cat (rawkey, "%s_d3M", key);
	Submit (key, value.Get ("%d %b", str));

	cat (rawkey, "%s_3Md", key);
	Submit (key, value.Get ("%b %d", str));
}

void
FormatP::Submit (
 const char * key,
 const Number & value)
{
	String	str;

	Submit (key, value.Get (str));
}

void
FormatP::Submit (
 const char * key,
 const Money & value)
{
	String	str;

	Submit (key, value.Get (str));
}

void
FormatP::SubmitClear (
 const Table & table)
{
	ColumnInfo		info;

	for (int i = 0; table.ColName (i); i++)
		if (table.ColInfo (i, info))
			SubmitClear (table.ColName (i), info);

	for (int i = 0; table.UColName (i); i++)
		if (table.UColInfo (i, info))
			SubmitClear (table.UColName (i), info);
}

void
FormatP::BatchEnd ()
{
	fputc ('\n', output);
}

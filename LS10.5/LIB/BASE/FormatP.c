#ident	"$Id: FormatP.c,v 5.0 2001/06/19 06:59:12 cha Exp $"
/*
 *	Simple front end to format-p
 *
 *******************************************************************************
 *	$Log: FormatP.c,v $
 *	Revision 5.0  2001/06/19 06:59:12  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:34  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:17  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:12  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.8  2000/06/26 00:18:12  scott
 *	Updated to make ML routine use unix environment instead of LS10
 *	
 *	Revision 1.7  2000/06/21 07:39:58  vij
 *	Modified number of decimal digits to 8.
 *	
 *	Revision 1.6  2000/01/05 02:15:45  jonc
 *	Tightened FormatSubmitTable args
 *	
 *	Revision 1.5  1999/12/12 22:34:42  jonc
 *	Added support for truncated date-values.
 *	
 *	Revision 1.4  1999/12/09 21:32:00  jonc
 *	Added masks for 2-digit-year
 *	
 *	Revision 1.3  1999/10/06 23:55:55  jonc
 *	Added #include for prototype for generic access routines.
 *	
 *	Revision 1.2  1999/09/13 06:20:46  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.1  1999/07/14 23:52:40  jonc
 *	Initial interface for format-p (adopted from Pinnacle V10)
 *	
 */
#include	<assert.h>
#include	<stdio.h>
#include	<string.h>

#include	<osdefs.h>
#include	<dbio.h>
#include	<ProtosIF.h>
#include	<ptypes.h>

#include	<DateToString.h>

#include	<FormatP.h>

/*
 *
 */
static FILE *
Init (
 const char * layoutfile,
 const char * altoptionblock)
{
	FILE *	output = popen ("format-p", "w");
	assert (output);

	fprintf (output, "#options\n");
	fprintf (output, "file=%s\n", layoutfile);

	if (altoptionblock)
		fprintf (output, "optionblock=%s\n", altoptionblock);

	return output;
}

/*
 *
 */
FILE *
FormatPOpen (
 const char * layout,
 const char * outstr,
 const char * optionblock)
{
	FILE *	output = Init (layout, optionblock);

	fprintf (output, "output=%s\n", outstr);
	fprintf (output, "#data\n");
	return output;
}

FILE *
FormatPOpenLpNo (
 const char * layout,
 int lpno,
 const char * optionblock)
{
	FILE *	output = Init (layout, optionblock);

	fprintf (output, "lpno=%d\n", lpno);
	fprintf (output, "#data\n");
	return output;
}

void
FormatPClose (
 FILE * output)
{
	pclose (output);
}

/*
 */
static void
UseSection (
 FILE * output,
 const char * base,
 const char * alt)
{
	fprintf (output, "%%%s=%s\n", base, alt ? alt : "");
}

/*
 */
void
FormatPReset (
 FILE * output)
{
	fprintf (output, "%%reset\n");
}

void
FormatPBatchEnd (
 FILE * output)
{
	fputc ('\n', output);
}

void
FormatPPageHeader (
 FILE * output,
 const char * alt)
{
	UseSection (output, "page-header", alt);
}

void
FormatPPageTrailer (
 FILE * output,
 const char * alt)
{
	UseSection (output, "page-trailer", alt);
}

void
FormatPBody (
 FILE * output,
 const char * alt)
{
	UseSection (output, "body", alt);
}

/*
 */
void
FormatPSubmitTable (
 FILE * output,
 const char * table)
{
	int	i;
	int	c = TableColumnCount (table);

	for (i = 0; i < c; i++)
	{
		int		j;
		int		int_val;
		long	long_val;
		double	double_val;
		float	float_val;
		Date	dvalue;
		char	svalue [512];	/* hopefully big enough */
		Money	mvalue;

		struct ColumnInfo	info;

		TableColumnInfo (table, i, &info);
		switch (info.type)
		{
		case CT_Chars:
			TableColumnGet (table, i, svalue);
			for (j = strlen (svalue); j < info.size; j++)
				svalue [j] = ' ';
			svalue [info.size] = '\0';
			break;

		case CT_Short:
			TableColumnGet (table, i, &int_val);
			sprintf (svalue, "%d", int_val);
			break;

		case CT_Serial:
		case CT_Long:
			TableColumnGet (table, i, &long_val);
			sprintf (svalue, "%ld", long_val);
			break;

		case CT_Double:
			TableColumnGet (table, i, &double_val);
			sprintf (svalue, "%.2f", double_val);
			break;

		case CT_Float:
			TableColumnGet (table, i, &float_val);
			sprintf (svalue, "%.2f", float_val);
			break;

		case CT_Date:
			TableColumnGet (table, i, &dvalue);
			FormatPSubmitDate (output, info.name, dvalue);
			continue;

		case CT_Money:
			TableColumnGet (table, i, &mvalue);
			FormatPSubmitMoney (output, info.name, mvalue);
			continue;

		default:
			strcpy (svalue, "**UNTRANSLATABLE**");
		}
		FormatPSubmitChars (output, info.name, svalue);
	}
}

void
FormatPSubmitInt (
 FILE * output,
 const char * key,
 int value)
{
	fprintf (output, "%s=%d\n", key, value);
}

void
FormatPSubmitLong (
 FILE * output,
 const char * key,
 long value)
{
	fprintf (output, "%s=%ld", key, value);
}

void
FormatPSubmitChars (
 FILE * output,
 const char * key,
 const char * rvalue)
{
	fprintf (output, "%s=%s\n", key, rvalue);
}

void
FormatPSubmitDate (
 FILE * output,
 const char * rawkey,
 Date value)
{
	char	key [128];
	char	str [128];

	FormatPSubmitChars (output, rawkey, DateToString (value));

	sprintf (key, "%s_dmy", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d/%m/%Y", str));

	sprintf (key, "%s_dm2y", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d/%m/%y", str));

	sprintf (key, "%s_mdy", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%m/%d/%Y", str));

	sprintf (key, "%s_md2y", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%m/%d/%y", str));

	sprintf (key, "%s_dMy", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %B %Y", str));

	sprintf (key, "%s_dM2y", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %B %y", str));

	sprintf (key, "%s_d3My", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %b %Y", str));

	sprintf (key, "%s_d3M2y", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %b %y", str));

	/*
	 *	Supported truncated values
	 */
	sprintf (key, "%s_dm", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d/%m", str));

	sprintf (key, "%s_md", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%m/%d", str));

	sprintf (key, "%s_dM", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %B", str));

	sprintf (key, "%s_Md", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%B %d", str));

	sprintf (key, "%s_d3M", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%d %b", str));

	sprintf (key, "%s_3Md", rawkey);
	FormatPSubmitChars (output,
		key,
		DateToFmtString (value, "%b %d", str));
}

void
FormatPSubmitMoney (
 FILE * output,
 const char * key,
 Money value)
{
	char	str [128];

	sprintf (str, "%.2f", value / 100);
	FormatPSubmitChars (output, key, str);
}

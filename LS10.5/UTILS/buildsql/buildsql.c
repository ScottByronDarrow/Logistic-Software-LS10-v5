#ident	"$Id: buildsql.c,v 5.2 2001/08/09 09:26:45 scott Exp $"
/*
 *	SQL table difference generator.
 *	C-Header file builder.
 *	Table description dump.
 *
 *	Author : Jonathan Chen
 *
 *******************************************************************************
 *	$Log: buildsql.c,v $
 *	Revision 5.2  2001/08/09 09:26:45  scott
 *	Updated to add FinishProgram () function
 *	
 *	Revision 5.1  2001/08/06 23:58:35  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 08:22:29  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:31  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:00  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:02  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.8  2000/02/18 03:06:05  scott
 *	Updated to fix small compile warings errors found when compiled under Linux.
 *	
 *	Revision 1.7  1999/11/16 20:58:25  jonc
 *	Added -a and -A options from version 10
 *	
 *	Revision 1.6  1999/10/26 22:53:30  jonc
 *	Updated to use the generic system-catalog interface.
 *	
 */
char    *PNAME = "$RCSfile: buildsql.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/buildsql/buildsql.c,v 5.2 2001/08/09 09:26:45 scott Exp $";

#include	<stdio.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#ifdef	LINUX
#include	<getopt.h>
#endif	/* LINUX */

#include	<osdefs.h>
#include	<dbio.h>
#include	<ProtosIF.h>

#include	"commdefs.h"
#include	"tbldef.h"
#include	"diff.h"

static char
	*data	= "data";

/*
 *	Local enums and structures
 */
enum Option
{
	O_Null,				/* initial state */
	O_DbgF,				/* dump table from input */
	O_DbgD,				/* dump table from DB */
	O_Sql,				/* SQL statements */
	O_Inc,				/* Include file */
	O_Sch,				/* Schema generation */
	O_AppF,				/* app.schema generation from input */
	O_AppD				/* app.schema generation from Database */
};

/*
 *	Globals
 */
int			line_no;
char		cur_fname [100],	/* current source file */
			tbldesc [100];		/* for include files */
TableDef	*cur_tbl;			/* current table */

/*
 *	External references
 */
extern char	*optarg;			/* argument processing */
extern int	optind;

extern FILE	*yyin;				/* parser input, output */

extern char	*clip (char *),
			*lclip (char *),
			*upshift (char *);

/*
 *	Local declarations
 */
static void	Usage (const char *);
static int	HandleOpt (enum Option, const char *, int);
static int	ParseInput (const char *);
static void	open_db (void),
			close_db (void);

static TableDef	*LoadDBTable (const char *);
static void		PrintInclude (TableDef *, int),
				PrintSchema (TableDef *, int);

static void		fputc_n (int, FILE *, int);

/*
 *	The real thing!
 */
int
main (
 int	argc,
 char	*argv [])
{
	int	c;
	int	std_out = FALSE;
	enum Option	opt = O_Null;

	/*
	 *	Handle arguments
	 */
	while ((c = getopt (argc, argv, "dDpscCaA")) != EOF)
		switch (c)
		{
		case 'p'	:
			std_out = TRUE;		break;	/* output to stdio */
		case 'd'	:
			opt = O_DbgF;		break;
		case 'D'	:
			opt = O_DbgD;		break;
		case 's'	:
			opt = O_Sql;		break;
		case 'c'	:
			opt = O_Inc;		break;
		case 'C'	:
			opt = O_Sch;		break;
		case 'a'	:
			opt = O_AppF;		break;
		case 'A'	:
			opt = O_AppD;		break;
		default		:
			Usage (argv [0]);
			return (EXIT_FAILURE);
		}

	if (opt == O_Null)
	{
		Usage (argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 *	Let's do some real work
	 */
	open_db ();

	if (optind >= argc)
		HandleOpt (opt, NULL, std_out);	/* no input file, take from stdin */
	else
	{
		/*	Process all remaining args on the command line
		 */
		for (c = optind; c < argc; c++)
			if (!HandleOpt (opt, argv [c], std_out) && c + 1 < argc)
				fprintf (stderr, "WRN: Skipping to next file\n");
	}
	close_db ();
	return (EXIT_SUCCESS);
}

static void
Usage (
 const char	*cmd)
{
	fprintf (stderr, "Usage : %s [-p] -d|D|s|c|C [file ...]\n", cmd);
	fprintf (stderr, "   -p : output to standard output\n");
	fprintf (stderr, "   -d : dump structure from input file\n");
	fprintf (stderr, "   -D : dump structure from database\n");
	fprintf (stderr, "   -s : create difference SQL statements [.sql]\n");
	fprintf (stderr, "   -c : create include file from input file [.inc]\n");
	fprintf (stderr, "   -C : create schema file from database [.sch]\n");
	fprintf (stderr, "   -a : dump programmers app.schema from input file\n");
	fprintf (stderr, "   -A : dump programmers app.schema from database\n");
}

static void
open_db ()
{
	abc_dbopen (data);

	TableCount ();				/* initialise the catalog system */
}

static void
close_db ()
{
	abc_dbclose (data);
}

static int
HandleOpt (
 enum Option	opt,
 const char		*name,
 int			to_stdout)
{
	TableDef	*db_tbl = NULL;
	Diff		*diff = NULL;

	cur_tbl = NULL;				/* just in case */
	switch (opt)
	{
	case O_Inc	:
		if (!ParseInput (name))
			return (FALSE);

		PrintInclude (cur_tbl, to_stdout);
		DelTable (cur_tbl);		/* clean up parsed input table */
		break;

	case O_Sch	:
		PrintSchema (db_tbl = LoadDBTable (name), to_stdout);
		DelTable (db_tbl);
		break;

	case O_Sql	:
		/*
		 *	Generate SQL statements
		 */
		if (!ParseInput (name))
			return (FALSE);

		if (!cur_tbl)			/* empty file parsed */
			break;

		db_tbl = LoadDBTable (cur_tbl -> name);

		PrintDiff (diff = MakeDiff (db_tbl, cur_tbl),
			cur_tbl -> name,
			to_stdout);

		DelDiff (diff);			/* clean up difference table */
		DelTable (db_tbl);		/* clean up parsed db table */
		DelTable (cur_tbl);		/* clean up parsed input table */
		break;

	case O_DbgF	:
		if (!ParseInput (name))
			return (FALSE);

		PrintTable (cur_tbl);
		DelTable (cur_tbl);
		break;

	case O_DbgD	:
		PrintTable (db_tbl = LoadDBTable (name));
		DelTable (db_tbl);
		break;

	case O_AppF	:
		if (!ParseInput (name))
			return (FALSE);

		PrintAppSchema (cur_tbl);
		DelTable (cur_tbl);
		break;

	case O_AppD	:
		PrintAppSchema (db_tbl = LoadDBTable (name));
		DelTable (db_tbl);
		break;

	default:
		return FALSE;
	}
	return (TRUE);
}

static int
ParseInput (
 const char	*fname)
{
	int	state = TRUE;
	extern int yyparse (void);

	if (fname &&
		!(yyin = fopen (fname, "r")))
	{
		fprintf (stderr, "ERR: Can't open \"%s\"\n", fname);
		return (FALSE);
	}

	/*	Misc Initialization
	 */
	line_no = 1;			/* reset line counter */
	tbldesc [0] = '\0';
	strcpy (cur_fname, fname ? fname : "<stdin>");

	if (yyparse ())
	{
		/*	Unsuccessful parse
		 */
		DelTable (cur_tbl);
		cur_tbl = NULL;
		state = FALSE;
	}

	if (fname)
		fclose (yyin);
	return (state);
}

static TableDef *
LoadDBTable (
 const char	*tbl_name)
{
	int	i, tblno;
	TableDef	*tbl;
	struct TableInfo tblinfo;

	if (!tbl_name)
		return (NULL);

	if ((tblno = TableNumber (tbl_name)) < 0)
	{
		/*
		 *	Table doesn't exist in the database
		 */
		return NULL;
	}
	TableInfo (tblno, &tblinfo);

	/*
	 */
	tbl = NewTable (strdup (tbl_name));

	/*
	 *	Examine columns
	 */
	for (i = 0; i < tblinfo.ncolumn; i++)
	{
		struct ColumnInfo colinfo;

		TableColumnInfo (tbl_name, i, &colinfo);

		AddField (tbl, strdup (colinfo.name), colinfo.type, colinfo.size);
	}

	/*
	 * Examine indexes for table
	 */
	for (i = 0; i < tblinfo.nindexes; i++)
	{
		int p;
		struct IndexInfo idxinfo;
		NameList	*n = NULL;

		TableIndexInfo (tblno, i, &idxinfo);

		AddIndex (tbl,
			strdup (idxinfo.name), 
			idxinfo.isunique ? I_Uniq : I_Dup);

		/*
		 *	find composite parts to indexes
		 */
		for (p = 0; p < idxinfo.ncolumn; p++)
		{
			struct ColumnInfo colinfo;

			TableIndexColumnInfo (tblno, i, p, &colinfo);
			n = AddName (n, strdup (colinfo.name));
		}
		AddIndexFlds (tbl, idxinfo.name, n);
	}

	return (tbl);
}

/*
 *	Print Include file
 */
static void
PrintInclude (
 TableDef	*tbl,
 int		to_stdout)
{
	char		fname [80];
	FILE		*out;
	unsigned	c_fld, len;
	FieldDef	*f;

	if (!tbl)
		return;

	if (to_stdout)
		out = stdout;
	else
	{
		if (!(out = fopen (strcat (strcpy (fname, tbl -> name), ".inc"), "w")))
		{
			fprintf (stderr, "ERR: Can't open \"%s\" for output\n", fname);
			return;
		}
	}

	/*	Count fields
	 */
	for (c_fld = 0, f = tbl -> fields; f; c_fld++, f = f -> next);

	/*	Header
	 */
	if (!(len = strlen (lclip (clip (tbldesc)))))
		len = strlen (strcpy (tbldesc, tbl -> name));

	fprintf (out, "\t/*");
	fputc_n ('=', out, len + 2);
	fputs ("+\n", out);

	fprintf (out, "\t | %s |\n", tbldesc);

	fprintf (out, "\t +");
	fputc_n ('=', out, len + 2);
	fprintf (out, "*/\n");

	/*	#define	field count
	 */
	fprintf (out,
		"#define\t%s_NO_FIELDS\t%u\n\n",
		upshift (strcpy (fname, tbl -> name)),
		c_fld);

	/*	struct dbview
	 */
	fprintf (out,
		"\tstruct dbview\t%s_list [%s_NO_FIELDS] =\n\t{\n",
		tbl -> name,
		fname);
	for (f = tbl -> fields; f; f = f -> next)
	{
		fprintf (out, "\t\t{\"%s\"}", f -> name);
		if (f -> next)
			fputc (',', out);
		fputc ('\n', out);
	}
	fprintf (out, "\t};\n\n");

	/*	record structure
	 */
	sprintf (fname, "%s_", tbl -> name);	/* for check on field names */
	len = strlen (fname);

	fprintf (out, "\tstruct tag_%sRecord\n\t{\n", tbl -> name);
	for (f = tbl -> fields; f; f = f -> next)
	{
		char	*typename = "(null)";

		switch (f -> type)
		{
		case CT_Chars	:	typename = "char";		break;
		case CT_Date	:	typename = "Date";		break;
		case CT_Double	:	typename = "double";	break;
		case CT_Float	:	typename = "float";		break;
		case CT_Long	:	typename = "long";		break;
		case CT_Money	:	typename = "Money";		break;
		case CT_Serial	:	typename = "long";		break;
		case CT_Short	:	typename = "int\t";		break;
		default	:
			fprintf (stderr, "INT: [PrintInclude] Unknown field type\n");
			goto end;
		}

		/*	Field name printed without file-name prefix
		 */
		fprintf (out, "\t\t%s\t%s", typename,
			strncmp (f -> name, fname, len) ? f -> name : f -> name + len);

		if (f -> type == CT_Chars)
			fprintf (out, " [%u]", f -> len + 1);
		fputs (";\n", out);
	}
	fprintf (out, "\t}\t%s_rec;\n\n", tbl -> name);

end:
	if (!to_stdout)
		fclose (out);
}

/*
 *	Print a schema file
 */
static void
PrintSchema (
 TableDef	*tbl,
 int		to_stdout)
{
	FILE		*out;
	FieldDef	*f;
	IndexDef	*i;
	char		*null = "";
	time_t		t = time (NULL);
	struct tm	*local = localtime (&t);

	if (to_stdout)
		out = stdout;
	else
	{
		char	fname [80];

		if (!(out = fopen (strcat (strcpy (fname, tbl -> name), ".sch"), "w")))
		{
			fprintf (stderr, "ERR: Can't open \"%s\" for output\n", fname);
			return;
		}
	}

	/*
	 *	Header junk
	 */
	fputc ('{', out);	fputc_n ( '=', out, 69);	fputs ("+\n", out);
	fprintf (out,
		"| Copyright (C) 1988 - %d "
		"Pinnacle Software Limited.                |\n",
		local -> tm_year + 1900);
	fputc ('|', out);	fputc_n ( '=', out, 69);	fputs ("|\n", out);
	fprintf (out, "| Schema Name  : (sch.%-46s) |\n", tbl -> name);
	fprintf (out, "| Schema Desc  : (%50s) |\n", null);
    fprintf (out, "|                (%50s) |\n", null);
	fputc ('|', out);	fputc_n ( '-', out, 69);	fputs ("|\n", out);
	fputc ('|', out);	fputc_n ( ' ', out, 69);	fputs ("|\n", out);
	fputc ('+', out);	fputc_n ( '=', out, 69);	fputs ("}\n", out);

	/*	Schema definition
	 */
	fprintf (out, "file\t%s\n", tbl -> name);

	/*	fields structure
	 */
	for (f = tbl -> fields; f; f = f -> next)
	{
		char		*typename = "(null)";

		switch (f -> type)
		{
		case CT_Chars	:	typename = "char";		break;
		case CT_Date	:	typename = "date";		break;
		case CT_Double	:	typename = "double";	break;
		case CT_Float	:	typename = "float";		break;
		case CT_Long	:	typename = "long";		break;
		case CT_Money	:	typename = "money";		break;
		case CT_Serial	:	typename = "serial";	break;
		case CT_Short	:	typename = "integer";	break;
		default	:
			fprintf (stderr, "INT: [PrintSchema] Unknown field type\n");
			goto end;
		}

		fprintf (out, "field\t%-20stype\t%s", f-> name, typename);

		if (f -> type == CT_Chars)
			fprintf (out, " %u", f -> len);

		/*	Check if field is index name as well
		 */
		if ((i = HasIndex (tbl, f -> name)))
		{
			fprintf (out, "\tindex");
			if (i -> type == I_Dup)
				fprintf (out, "\tdups");
		}

		fputc ('\n', out);
	}

	/*	indices
	 */
	for (i = tbl -> indexes; i; i = i -> next)
	{
		/*	Only generate for composite indices
			- ie index name != first name in name-list
		 */
		if (strcmp (i -> name, i -> fldnames -> name))
		{
			NameList	*n;

			fprintf (out, "field\t%-20stype\tcomposite\n", i -> name);
			for (n = i -> fldnames; n; n = n -> next)
			{
				fprintf (out, "\t\t%s", n -> name);
				if (n -> next)
					fprintf (out, ",\n");
			}
			fprintf (out, "\t\tindex");
			if (i -> type == I_Dup)
				fprintf (out, "\tdups");
			fputc ('\n', out);
		}
	}

	fprintf (out, "end\n");

end:
	if (!to_stdout)
		fclose (out);
}

/*
 *	Support utils..
 */
static void
fputc_n (
 int	c,
 FILE	*f,
 int	n)
{
	while (n--)
		fputc (c, f);
}

void
warning (
 const char	*str,
 ...)
{
	va_list	ap;

	fprintf (stderr, "WRN: %s Line %d : ", cur_fname, line_no);

	va_start (ap, str);
	vfprintf (stderr, str, ap);
	va_end (ap);
}

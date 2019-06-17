#ident	"$Id: diff.c,v 5.0 2001/06/19 08:22:29 robert Exp $"
/*
 *	Marks the differences between schema and database-catalog
 *
 *******************************************************************************
 *	$Log: diff.c,v $
 *	Revision 5.0  2001/06/19 08:22:29  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:32  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/12/13 09:47:24  scott
 *	Updated to change message, want to see files that change not ones that don't
 *	
 *	Revision 3.0  2000/10/10 12:24:00  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.1  2000/10/10 11:19:48  gerry
 *	Unchecked in - for ML?
 *	
 *	Revision 2.0  2000/07/15 09:15:02  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.5  2000/01/04 23:54:54  jonc
 *	A bit more cleverer about removing indexes. Don't need to explicitly
 *	remove them if one of their fields is scheduled to be dropped.
 *	
 *	Revision 1.4  1999/11/16 20:58:25  jonc
 *	Added -a and -A options from version 10
 *	
 *	Revision 1.3  1999/10/28 23:51:33  jonc
 *	Added support for ORACLE systems.
 *	
 *	Revision 1.2  1999/10/26 22:53:30  jonc
 *	Updated to use the generic system-catalog interface.
 *	
 */
#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>

#include	<dbio.h>
#include	<ProtosIF.h>

#include	"commdefs.h"
#include	"tbldef.h"
#include	"diff.h"

static const char * CT2SQLType (enum ColumnType type, unsigned sz);
static int DiffHasIndexField (const Diff *, const IndexDef *);

Diff *
MakeDiff (
 TableDef	*orig,
 TableDef	*to)
{
	FieldDef	*f;
	IndexDef	*i;
	Diff		*diff = NULL;

	/*
	 *	Check the dbif attributes to see whether
	 *		- short/long are the same
	 *		- double/float are the same
	 */
	int same_short_long = !strcmp (
							DbIfAttribute ("short-type"),
							DbIfAttribute ("long-type"));
	int same_float_double = !strcmp (
							DbIfAttribute ("float-type"),
							DbIfAttribute ("double-type"));

	/*	Let's handle the trivial case first
	 */
	if (!to && !orig)
		return (NULL);

	memset (diff = malloc (sizeof (Diff)), 0, sizeof (Diff));

	/*	Other misc cases
	 */
	if (orig)
	{
		if (!to)
		{
			diff -> tbl = D_Drop;
			return (diff);
		}
	}
	else
	{
		if (to)
			diff -> tbl = D_Create;
	}

	/*
	 *	Field Passes :
	 *		1.	[delf] Drop columns that are in the "orig" but not in "to"
	 *		2.	[addf] Add columns that are in "to" but not in "orig"
	 *		3.	[modf] Modify columns that are different in "orig" to "to"
	 */

	/*	1.	[delf] drop columns
	 */
	if (orig)
		for (f = orig -> fields; f; f = f -> next)
			if (!HasField (to, f -> name))
				AddFld (&diff -> delf, strdup (f -> name), f -> type, f -> len);

	/*	2.	[addf] add columns
	 */
	for (f = to -> fields; f; f = f -> next)
		if (!HasField (orig, f -> name))
		{
			AddFld (&diff -> addf, strdup (f -> name), f -> type, f -> len);

			if (diff -> tbl != D_Create)
			{
				/*	'alter table add' :
					Need to find an anchor point on the original table to
					position the new field
				 */
				FieldDef	*anchor = NULL, *fld;

				for (fld = f; fld; fld = fld -> next)
					if ((anchor = HasField (orig, fld -> name)))
						break;

				/*	Add anchor point for this field
				 */
				if (anchor)
					AddFld (&diff -> anchor,
						strdup (anchor -> name), anchor -> type, anchor -> len);
				else
					AddFld (&diff -> anchor, NULL, CT_Bad, 0);
			}
		}

	/*	3.	[modf] modifed columns
			Note : the desired effect is the column in the "to" table
	 */
	for (f = to -> fields; f; f = f -> next)
	{
		FieldDef	*m = HasField (orig, f -> name);
		int add_diff = FALSE;

		if (!m)
			continue;

		if (m -> len != f -> len)
			add_diff = TRUE;
		else if (m -> type != f -> type)
		{
			/*
			 *	Have to be careful about database-types that are the same
			 */
			switch (m -> type)
			{
			case CT_Short:
				add_diff = !(f -> type == CT_Long && same_short_long);
				break;

			case CT_Long:
				add_diff = !(f -> type == CT_Short && same_short_long);
				break;

			case CT_Float:
				add_diff = !(f -> type == CT_Double && same_float_double);
				break;

			case CT_Double:
				add_diff = !(f -> type == CT_Float && same_float_double);
				break;

			default:
				add_diff = TRUE;
			}
		}

		if (add_diff)
			AddFld (&diff -> modf, strdup (f -> name), f -> type, f -> len);
	}

	/*
	 *	Index Passes :
	 *		1.	[deli] Drop indices in "orig" not in "to"
	 *		2.	[addi] Add indices in "to" not in "orig"
	 *		3.	[addi,del] Drop different indices and re-add them
	 */

	/*	1.	[deli] Drop indices
	 */
	if (orig)
		for (i = orig -> indexes; i; i = i -> next)
		{
			/*
			 *	If the index isn't found the new table definition,
			 *	remove it.
			 *
			 *	If the index has a column that is scheduled to
			 *	be removed, the index will be removed automatically
			 *	when the column goes away; so only add the removed-index
			 *	if all its columns aren't in the remove-column list
			 */
			if (!HasIndex (to, i -> name) &&
				!DiffHasIndexField (diff, i))
			{
				AddIdx (&diff -> deli, strdup (i -> name), i -> type);
			}
		}

	/*	2.	[addi] Add indices
	 */
	for (i = to -> indexes; i; i = i -> next)
		if (!HasIndex (orig, i -> name))
		{
			NameList	*n = NULL, *l;

			AddIdx (&diff -> addi, strdup (i -> name), i -> type);

			/*	Build up a namelist and add to the index
			 */
			for (l = i -> fldnames; l; l = l -> next)
				n = AddName (n, strdup (l -> name));
			AddIdxFlds (diff -> addi, i -> name, n);
		}

	/*	3.	[addi,deli] Modified indices
	 */
	for (i = to -> indexes; i; i = i -> next)
	{
		IndexDef	*m = HasIndex (orig, i -> name);

		if (m &&
			(m -> type != i -> type || !NL_Eq (m -> fldnames, i -> fldnames)))
		{
			NameList	*n = NULL, *l;

			AddIdx (&diff -> deli, strdup (i -> name), i -> type);
			AddIdx (&diff -> addi, strdup (i -> name), i -> type);

			/*	Build up a namelist and add to the index
			 */
			for (l = i -> fldnames; l; l = l -> next)
				n = AddName (n, strdup (l -> name));
			AddIdxFlds (diff -> addi, i -> name, n);
		}
	}

	/*	No changes?
	 */
	if (diff -> tbl != D_Create)
	{
		diff -> tbl =
			diff -> addf || diff -> modf || diff -> delf ||
			diff -> addi || diff -> deli ?
			D_Alter : D_Unchanged;
	}
	return (diff);
}

void
DelDiff (
 Diff	*diff)
{
	if (!diff)
		return;

	DelFields (diff -> addf);
	DelFields (diff -> modf);
	DelFields (diff -> delf);

	DelIndexes (diff -> addi);
	DelIndexes (diff -> deli);
	free (diff);
}

static int
PrintType (
 FILE		*out,
 FieldDef	*f)
{
	fprintf (out, "%-20s %s", f -> name, CT2SQLType (f -> type, f -> len));
	return (TRUE);
}

void
PrintDiff (
 Diff		*diff,
 const char	*name,
 int		to_stdout)
{
	int			c;
	FILE		*out;
	IndexDef	*i;
	FieldDef	*f, * serial_col = NULL;
	int has_placement = (int) DbIfAttribute ("has-column-placement");

	if (!diff || diff -> tbl == D_Unchanged)
		return;

	if (!to_stdout)
		fprintf (stderr, "changes in [%s]\n", name);

	if (to_stdout)
		out = stdout;
	else
	{
		char	fname [80];

		if (!(out = fopen (strcat (strcpy (fname, name), ".sql"), "w")))
		{
			fprintf (stderr, "ERR: Can't open \"%s\" for output\n", fname);
			return;
		}
	}

	/*	Table level commands
	 */
	switch (diff -> tbl)
	{
	case D_Alter	:
		if (diff -> addf || diff -> delf || diff -> modf)
			fprintf (out, "alter table %s\n", name);
		break;
	case D_Create	:
		fprintf (out, "create table %s\n(\n", name);
		break;
	case D_Drop	:
		fprintf (out, "drop table %s;\n", name);
		goto end;

	default		:
		fprintf (stderr, "INT: [PrintDiff] Bad diff -> tbl\n");
		goto end;
	}

	/*
	 *	Fields : additions
	 */
	if (diff -> tbl == D_Alter && diff -> addf)
		fprintf (out, "add\n(\n");
	for (f = diff -> addf, c = 0; f; f = f -> next, c++)
	{
		fputc ('\t', out);

		if (!PrintType (out, f))
			goto end;					/* internal error! */

		if (has_placement && diff -> tbl == D_Alter)
		{
			/*
			 *	Generate field position statment
			 */
			int			ctr;
			FieldDef	*anchor;

			/*	Advance to local anchor
			 */
			for (ctr = c, anchor = diff -> anchor;
				ctr;
				anchor = anchor -> next, ctr--);
			{
				if (!anchor)
				{
					fprintf (stderr, "INT: [PrintDiff] Missing anchor\n");
					goto end;
				}
			}
			if (anchor -> name)
				fprintf (out, " before %s", anchor -> name);
		}

		if (f -> type == CT_Serial)
			serial_col = f;

		if (f -> next)
			fputc (',', out);
		fputc ('\n', out);
	}
	if (diff -> addf)
		fputc (')', out);

	if (diff -> tbl == D_Alter)
	{
		/*
		 *	Fields : drops
		 */
		if (diff -> delf)
		{
			if (diff -> addf)
				fprintf (out, ",\n");
			fprintf (out, "drop\n(\n");
		}
		for (f = diff -> delf; f; f = f -> next)
		{
			fprintf (out, "\t%s", f -> name);
			if (f -> next)
				fputc (',', out);
			fputc ('\n', out);
		}
		if (diff -> delf)
			fputc (')', out);

		/*
		 *	Fields : mods
		 */
		if (diff -> modf)
		{
			if (diff -> addf || diff -> delf)
				fprintf (out, ",\n");
			for (f = diff -> modf; f; f = f -> next)
			{
				fprintf (out, "modify (");

				if (!PrintType (out, f))
					goto end;				/* internal error! */

				fputc (')', out);
				if (f -> next)
					fprintf (out, ",\n");
			}
		}
	}

	/*	No ';' if only indexes have changed
	 */
	if (diff -> addf || diff -> delf || diff -> modf)
		fprintf (out, ";\n");

	/*
	 *	Indexes : Do drops before creations
	 */
	for (i = diff -> deli; i; i = i -> next)
		fprintf (out, "drop index %s;\n", i -> name);
	for (i = diff -> addi; i; i = i -> next)
	{
		NameList	*n;

		fprintf (out, "create %s index %s on %s\n(\n\t%s",
			i -> type == I_Uniq ? "unique" : "",
			i -> name,
			name,
			i -> fldnames -> name);
		for (n = i -> fldnames -> next; n; n = n -> next)
			fprintf (out, ",\n\t%s", n -> name);
		fprintf (out, "\n);\n");
	}

	/*
	 *	Check for sequences
	 */
	if (serial_col && !DbIfAttribute ("has-serial"))
	{
		/*
		 *	No auto-serial data-type. Try sequences
		 */
		if (DbIfAttribute ("has-sequence"))
		{
			/*
			 *	With sequences, we use the name of the serial field
			 */
			fprintf (out, "create sequence %s;", serial_col -> name);
		} else
		{
			fprintf (stderr,
			"INT: No serial. No sequence. Don't know what to do\n");
		}
	}

end:
	if (!to_stdout)
		fclose (out);
}

static
const char *
CT2SQLType (
 enum ColumnType type,
 unsigned sz)
{
	/*
	 *	Convert the ColumnType to an SQL type,
	 *	consulting the DbIf as we go along
	 */
	static char result [128];

	switch (type)
	{
	case CT_Chars:
		sprintf (result, "char (%u)", sz);
		return result;

	case CT_Date:
		return DbIfAttribute ("date-type");

	case CT_Double:
		return DbIfAttribute ("double-type");

	case CT_Float:
		return DbIfAttribute ("float-type");

	case CT_Long:
		return DbIfAttribute ("long-type");

	case CT_Short:
		return DbIfAttribute ("short-type");

	case CT_Money:
		return DbIfAttribute ("money-type");

	case CT_Serial:
		return DbIfAttribute ("serial-type");

	default	:
		break;
	}
	fprintf (stderr, "INT: [CT2SQLType] Unknown field type\n");
	return NULL;
}

static int
DiffHasIndexField (
 const Diff * d,
 const IndexDef * i)
{
	/*
	 *	Return TRUE if the dropped column list has one of the
	 *	index's columns
	 */
	FieldDef * f;

	for (f = d -> delf; f; f = f -> next)
	{
		NameList * n;

		for (n = i -> fldnames; n; n = n -> next)
		{
			if (!strcmp (f -> name, n -> name))
				return TRUE;
		}
	}
	return FALSE;
}

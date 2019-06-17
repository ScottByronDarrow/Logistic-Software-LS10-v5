#ident	"$Id: tbldef.c,v 5.1 2001/08/28 08:46:53 scott Exp $"
/*
 *	Table related routines
 *
 *******************************************************************************
 *	$Log: tbldef.c,v $
 *	Revision 5.1  2001/08/28 08:46:53  scott
 *	Update for small change related to " (" that should not have been changed from "("
 *	
 *	Revision 5.0  2001/06/19 08:22:29  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:32  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:00  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:02  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.5  1999/11/16 21:01:03  jonc
 *	Fixes to warnings.
 *	
 *	Revision 1.4  1999/11/16 20:58:25  jonc
 *	Added -a and -A options from version 10
 *	
 *	Revision 1.3  1999/10/28 23:51:33  jonc
 *	Added support for ORACLE systems.
 *	
 *	Revision 1.2  1999/10/26 22:53:31  jonc
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

static void	DelNameList (NameList *);

/*
 *	External Interface
 */
TableDef *
NewTable (
 char	*name)				/* a malloc'd string */
{
	TableDef	*tbl = malloc (sizeof (TableDef));

	memset (tbl, 0, sizeof (TableDef));
	tbl -> name = name;
	return (tbl);
}

void
DelTable (
 TableDef	*tbl)
{
	if (!tbl)
		return;

	DelFields (tbl -> fields);
	DelIndexes (tbl -> indexes);

	if (tbl -> name)
		free (tbl -> name);
	free (tbl);
}

void
AddFld (
 FieldDef		**list,
 char			*name,		/* malloc'd string */
 enum ColumnType	type,
 unsigned		len)
{
	/*	Advance to last item on list
	 */
	while (*list)
		list = &(*list) -> next;

	memset (*list = malloc (sizeof (FieldDef)), 0, sizeof (FieldDef));
	(*list) -> name = name;
	(*list) -> type = type;
	(*list) -> len = len;
}

void
AddField (
 TableDef		*tbl,
 char			*name,		/* malloc'd string */
 enum ColumnType	type,
 unsigned		len)
{
	AddFld (&tbl -> fields, name, type, len);
}

void
DelFields (
 FieldDef	*fld)
{
	/*
	 *	Let's use recursion : UNIX should be able to handle these
	 */
	if (!fld)
		return;

	DelFields (fld -> next);
	free (fld -> name);
	free (fld);
}

FieldDef *
HasField (
 TableDef	*tbl,
 const char	*fldname)
{
	FieldDef	*f;

	if (tbl)
		for (f = tbl -> fields; f; f = f -> next)
			if (!strcmp (fldname, f -> name))
				return (f);
	return (NULL);
}

void
AddIdx (
 IndexDef	**list,
 char		*name,			/* malloc'd string */
 enum IndexType	type)
{
	IndexDef	*next = NULL;

	/*	
	 *	Index has to inserted sorted
	 */
	while (*list)
	{
		if (strcmp (name, (*list) -> name) < 0)
		{
			next = *list;
			break;
		}
		list = &(*list) -> next;
	}

	memset (*list = malloc (sizeof (IndexDef)), 0, sizeof (IndexDef));
	(*list) -> name = name;
	(*list) -> type = type;
	(*list) -> next = next;
}

void
AddIndex (
 TableDef	*tbl,
 char		*name,			/* malloc'd string */
 enum IndexType	type)
{
	AddIdx (&tbl -> indexes, name, type);
}

void
AddIdxFlds (
 IndexDef	*list,
 const char	*index,
 NameList	*names)
{
	/*	Search for matching name
	 */
	while (list)
	{
		if (!strcmp (list -> name, index))
			break;
		list = list -> next;
	}
	if (!list)
		return;				/* internal error, really */

	list -> fldnames = names;
}

void
AddIndexFlds (
 TableDef	*tbl,
 const char	*index,
 NameList	*names)
{
	AddIdxFlds (tbl -> indexes, index, names);
}

NameList *
AddName (
 NameList	*head,
 char		*name)			/* malloc'd string */
{
	NameList	**list;

	/*	Add to the end of the name list for this index
	 */
	for (list = &head; *list; list = &(*list) -> next);

	/*	Allocate and initialize name list node
	 */
	memset (*list = malloc (sizeof (NameList)), 0, sizeof (NameList));
	(*list) -> name = name;

	return (head ? head : *list);
}

int
NL_Eq (
 NameList	*a,
 NameList	*b)
{
	while (a && b)
	{
		if (strcmp (a -> name, b -> name))
			return (FALSE);
		a = a -> next;
		b = b -> next;
	}
	return (!a && !b);
}

void
DelIndexes (
 IndexDef	*idx)
{
	/*	Recursion used..
	 */
	if (!idx)
		return;

	DelIndexes (idx -> next);
	DelNameList (idx -> fldnames);
	free (idx -> name);
	free (idx);
}

IndexDef *
HasIndex (
 TableDef	*tbl,
 const char	*idxname)
{
	IndexDef	*i;

	if (tbl)
	{
		for (i = tbl -> indexes; i; i = i -> next)
			if (!strcmp (i -> name, idxname))
				return (i);
	}
	return (NULL);
}

static void
DelNameList (
 NameList	*list)
{
	if (!list)
		return;
	DelNameList (list -> next);
	free (list -> name);
	free (list);
}

void
PrintTable (
 TableDef	*tbl)
{
	unsigned	c_idx, c_fld;
	IndexDef	*idx;
	FieldDef	*fld;
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

	if (!tbl)
	{
		puts ("(empty)");
		return;
	}

	/*	Count indexes and fields
	 */
	for (idx = tbl -> indexes, c_idx = 0; idx; idx = idx -> next, c_idx++);
	for (fld = tbl -> fields, c_fld = 0; fld; fld = fld -> next, c_fld++);

	printf ("Table : %s (%d fields, %d indexes)\n", tbl -> name, c_fld, c_idx);

	/*	Dump field info
	 */
	for (fld = tbl -> fields; fld; fld = fld -> next)
	{
		char	*type = "(null)";

		switch (fld -> type)
		{
		case CT_Bad:
			type = "BAD!";
			break;
		case CT_Chars:
			type = "char";
			break;
		case CT_Date:
			type = "date";
			break;
		case CT_Double:
			type = same_float_double ? "float/double" : "double";
			break;
		case CT_Float:
			type = same_float_double ? "float/double" : "float";
			break;
		case CT_Short:
			type = same_short_long ? "short/long" : "short";
			break;
		case CT_Long:
			type = same_short_long ? "short/long" : "long";
			break;
		case CT_Money:
			type = "money";
			break;
		case CT_Serial:
			type = "serial";
			break;

		default:
			type = "**ERROR**";
		}

		printf ("\t%-20s %s", fld -> name, type);
		if (fld -> type == CT_Chars)
			printf ("(%u)", fld -> len);
		putchar ('\n');
	}

	if (!c_idx)
		return;
	puts ("Indexes");

	/*	Dump index info
	 */
	for (idx = tbl -> indexes; idx; idx = idx -> next)
	{
		unsigned	c_nam;
		NameList	*nm;

		for (nm = idx -> fldnames, c_nam = 0; nm; nm = nm -> next, c_nam++);

		/*	Print first field of index on same line as index name
		 */
		printf ("\t%-20s ", idx -> name);

		if (idx -> fldnames -> name)
		{
			printf ("%s\n", idx -> fldnames -> name);
			for (nm = idx -> fldnames -> next; nm; nm = nm -> next)
				printf ("\t%20s %s\n", "", nm -> name);
		}
		else
			puts ("[Internal error]");
	}
}

void
PrintAppSchema (
 TableDef *tbl)
{
	FieldDef	*fld;

	if (!tbl)
	{
		puts ("(empty)");
		return;
	}

	printf ("table %s\n", tbl -> name);

	/*	Dump field info
	 */
	for (fld = tbl -> fields; fld; fld = fld -> next)
		printf ("field %-20s\n", fld -> name);

	printf ("end\n");
}

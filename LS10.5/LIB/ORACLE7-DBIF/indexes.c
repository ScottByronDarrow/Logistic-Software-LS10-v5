#ident	"$Id: indexes.c,v 5.0 2001/06/19 07:10:28 cha Exp $"
/*
 *	Extract index info from system catalogs
 *
 *******************************************************************************
 *	$Log: indexes.c,v $
 *	Revision 5.0  2001/06/19 07:10:28  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:53  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:51  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.4  2000/06/30 01:12:40  gerry
 *	Temporary patch to remove cores
 *	
 *	Revision 1.3  1999/11/01 21:23:15  jonc
 *	Added support for no index.
 *	
 *	Revision 1.2  1999/10/31 22:42:29  jonc
 *	Fixed: occasional weird index info.
 *	
 *	Revision 1.1  1999/10/21 21:47:04  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdlib.h>

#include	<dbio.h>

#include	"oracledbif.h"

/*
 *	Static data areas for index query results
 */
static char index_name [128],
			table_name [128];

static char	col_name [128];
static int	col_count;

static ub2	r_len, r_code;

/*
 *	Local functions
 */
static void IndexSetup (Lda_Def *, Cda_Def * inds, Cda_Def * indc);

/*
 *	External interface
 */
void
IdentifyIndex (
 Lda_Def * lda,
 Cda_Def * inds,
 Cda_Def * indc,
 TableState * table,
 const char * index)
{
	/*
	 *	Indentify the index and associated columns
	 */
	int i;

	IndexSetup (lda, inds, indc);

	/*
	 *	Reallocation?
	 */
	if (table -> indexname)
	{
		if (!strcmp (table -> indexname, index))
			return;						/* no new work */

		free (table -> indexname);
		table -> indexname = NULL;
	}

	if (table -> indexc)
	{
		table -> indexc = 0;
		free (table -> indexactual);
	}

	if (!index)
		return;							/* table has no index */

	table -> indexname = strcpy (malloc (strlen (index) + 1), index);

	/*
	 *	System catalogs uses uppercase names for tables and indexes
	 */
	_strupper (strcpy (index_name, index));
	_strupper (
		strcpy (table_name, table -> table ? table -> table : table -> named));

	/*
	 *	Determine column count
	 */
	col_count = 0;						/* reset - just in case */
	if (oexec (indc) || ofetch (indc))
		oraclecda_error (lda, indc, "indc:exec+fetch");

	if (!col_count)
		oracledbif_error ("No columns for %s?", table_name);

	/*
	 *	Allocate buffers for index columns
	 */
	table -> indexc = col_count;
	table -> indexactual = malloc (table -> indexc * sizeof (int));
	table -> indexview = malloc (table -> indexc * sizeof (int));

	memset (table -> indexactual, 0, table -> indexc * sizeof (int));
	memset (table -> indexview, 0, table -> indexc * sizeof (int));

	/*
	 *	Get the column of the index and xref it against
	 *	our internal column definitions, and the user-view
	 */
	if (oexec (inds))
		oraclecda_error (lda, inds, "inds:exec");

	for (i = 0; ofetch (inds) == 0; i++)
	{
		int c;

		/*
		 *	Match the name against internal definitions
		 */
		_strlower (col_name);
		for (c = 0; c < table -> columnc; c++)
		{
			if (!strcmp (col_name, table -> columns [c].name))
			{
				/*
				 *	Store position of ColumnDef for later use
				 */
				table -> indexactual [i] = c;
				break;
			}
		}
		if (c >= table -> columnc)
			oracledbif_error ("Index column %s not found?", col_name);

		/*
		 *	Match the name against the user-view
		 */
		for (c = 0; c < table -> viewc; c++)
		{
			if (!strcmp (col_name, table -> view [c].vwname))
			{
				table -> indexview [i] = c;
				break;
			}
		}
		if (c >= table -> viewc)
			oracledbif_error ("Index column %s not found in dbview", col_name);
	}

	/*
	 *	Force an SQL statement rebuild, if required
	 */
	if (table -> query)
	{
		free (table -> query);
		table -> query = NULL;
	}
}

/*
 *	Support functions
 */
static void
IndexSetup (
 Lda_Def * lda,
 Cda_Def * inds,
 Cda_Def * indc)
{
	static int setup_done = FALSE;
	/*
	 * This *should* return ordered by COLUMN_POSITION
	 */
	char * sql_inds =
		"select column_name "
		"from user_ind_columns "
		"where table_name = :1 and index_name = :2 "
		"order by column_position";
	char * sql_indc =
		"select count (*) "
		"from user_ind_columns "
		"where table_name = :1 and index_name = :2";

	/*
	 *  Try set up everytime - slower but could solve cores
	 */
	/* if (setup_done)
		return; */

	/*
	 *	Index column count query
	 */
	if (oparse (indc, sql_indc, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (indc, 1,
			(ub1 *) table_name, sizeof (table_name), SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		obndrn (indc, 2,
			(ub1 *) index_name, sizeof (index_name), SQLT_STR,
			-1, NULL, NULL, -1, -1))
	{
		oraclecda_error (lda, indc, "indc:parse+bndrn");
	}

	/*
	 *	Result output and indicators
	 */
	if (odefin (indc, 1,
			(ub1 *) &col_count, sizeof (int), SQLT_INT, 
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code))
	{
		oraclecda_error (lda, indc, "indc:odefin");
	}


	/*
	 *	Index query
	 */
	if (oparse (inds, sql_inds, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (inds, 1,
			(ub1 *) table_name, sizeof (table_name), SQLT_STR,
			-1, NULL, NULL, -1, -1) ||
		obndrn (inds, 2,
			(ub1 *) index_name, sizeof (index_name), SQLT_STR,
			-1, NULL, NULL, -1, -1))
	{
		oraclecda_error (lda, inds, "inds:parse+bndrn");
	}

	/*
	 *	Result output and indicators
	 */
	if (odefin (inds, 1,
			(ub1 *) col_name, sizeof (col_name), SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code))
	{
		oraclecda_error (lda, inds, "inds:odefin");
	}

	setup_done = TRUE;
}

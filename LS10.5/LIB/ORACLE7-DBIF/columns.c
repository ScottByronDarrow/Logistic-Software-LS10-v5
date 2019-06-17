#ident	"$Id: columns.c,v 5.0 2001/06/19 07:10:27 cha Exp $"
/*
 *	Extract column info from system catalogs
 *
 *******************************************************************************
 *	$Log: columns.c,v $
 *	Revision 5.0  2001/06/19 07:10:27  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
 *	
 *	Revision 1.3  2000/06/30 01:12:40  gerry
 *	Temporary patch to remove cores
 *	
 *	Revision 1.2  1999/10/28 01:58:35  jonc
 *	Added support for generic-catalog access. Damnably slow, though.
 *	
 *	Revision 1.1  1999/10/21 21:47:04  jonc
 *	Alpha level checkin:
 *		Done: database queries, updates.
 *		Todo: date conversion, locking and error-handling.
 *	
 */
#include	<stdio.h>
#include	<stdlib.h>

#include	<dbio.h>

#include	"oracledbif.h"

/*
 *	Static data areas for column query results
 */
static int	col_count;
static char	col_table [128];

static char col_name [128],
			col_data_type [16];
static int	col_data_length, col_data_precision, col_data_scale;

static char	seq_name [128];

static sb2	i_data_type, i_data_precision, i_data_scale;	/* indicators */

static ub2	r_len, r_code;									/* discard */

/*
 *	Local functions
 */
static void	ColumnSetup (Lda_Def *, Cda_Def * cols, Cda_Def * colc, Cda_Def *),
			BuildColumnDefs (Lda_Def * lda,
				Cda_Def * cols, Cda_Def * seqs,
				TableState * table),
			BuildDataBuffer (TableState *),
			MapViewToColumns (TableState *),
			MapViewToUserData (TableState *);

/*
 */
void
IdentifyFields (
 Lda_Def * lda,
 Cda_Def * cols,
 Cda_Def * colc,
 Cda_Def * seqs,
 TableState * table)
{
	/*
	 *	We're gonna build the whole list for *all* the columns in
	 *	the table, and update the user-view for those columns involved
	 */
	ColumnSetup (lda, cols, colc, seqs);

	/*
	 *	The column query uses uppercase names for tables
	 */
	_strupper (
		strcpy (col_table, table -> table ? table -> table : table -> named));

	/*
	 *	Determine column count
	 */
	col_count = 0;						/* reset - just in case */
	if (oexec (colc) || ofetch (colc))
		oraclecda_error (lda, colc, "colc:exec+fetch");

	if (!col_count)
		oracledbif_error ("No columns for %s?", col_table);

	/*
	 *	Allocate buffers for table-columns
	 *
	 *	Add an extra definition for the rowid
	 */
	table -> columnc = col_count + 1;	/* add 1 for rowid */
	table -> columns = malloc (table -> columnc * sizeof (ColumnDef));
	memset (table -> columns, 0, table -> columnc * sizeof (ColumnDef));

	BuildColumnDefs (lda, cols, seqs, table);
	BuildDataBuffer (table);

	/*
	 *	Set up the userview structures
	 */
	MapViewToColumns (table);
	MapViewToUserData (table);
}

static void
BuildColumnDefs (
 Lda_Def * lda,
 Cda_Def * cols,
 Cda_Def * seqs,
 TableState * table)
{
	/*
	 *	Decode ORACLE types into something we can use
	 */
	int i;

	/*
	 *	Identify possible serial field
	 */
	if (oexec (seqs))
		oraclecda_error (lda, cols, "seqs:exec");
	if (ofetch (seqs))
		seq_name [0] = '\0';			/* just in case */
	else
		_strlower (seq_name);			/* since ORACLE returns in upper-case */

	/*
	 */
	if (oexec (cols))
		oraclecda_error (lda, cols, "cols:exec");

	for (i = 0; ofetch (cols) == 0; i++)
	{
		_strlower (strcpy (table -> columns [i].name, col_name));

		_DecodeORAType (
			table -> columns [i].name, seq_name,
			col_data_type, col_data_length, col_data_precision, col_data_scale,
			i_data_precision, i_data_scale,

			&table -> columns [i].type,
			&table -> columns [i].length);
	}

	/*
	 *	The last column is for the rowid
	 *	We build it up manually
	 */
	strcpy (table -> columns [i].name, "rowid");
	table -> columns [i].type = OT_RowId;
	table -> columns [i].length = _internal_size (OT_RowId);
}

static void
BuildDataBuffer (
 TableState * table)
{
	/*
	 *	Allocate row-buffer and set up the data-pointers for each column
	 */
	int i;
	int sz = 0;

	/*
	 *	Initial pass to determine the size
	 */
	for (i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		sz += table -> columns [i].length;
	}

	table -> datasz = sz;

//printf("BUILD DATA BUFFER. The size is %d\n", sz );
	memset (table -> data = malloc (sz), 0, sz);

	/*
	 *	Second pass sets the data-pointers
	 */
	for (sz = i = 0; i < table -> columnc; i++)
	{
		sz += _internal_align (sz, table -> columns [i].type);
		table -> columns [i].data = table -> data + sz;
//printf(" %s offset %d column type %d", 
//        table->columns[i].name, sz, table->columns ->type );
		sz += table -> columns [i].length;
//printf(" New offset %d\n", sz );
	}
}

static void
ColumnSetup (
 Lda_Def * lda,
 Cda_Def * cols,
 Cda_Def * colc,
 Cda_Def * seqs)
{
	/*
	 *	Set up the cursors against user_table_columns
	 *
	 *	I suspect that the sql-statements need to
	 *	permanant until the actual parse is done.
	 */
	char * sql_colc =
		"select count (*) from user_tab_columns "
		"where table_name = :1";
	char * sql_cols =
		"select "
			"column_name, data_type, data_length, "
			"data_precision, data_scale "
		"from user_tab_columns "
		"where table_name = :1 "
		"order by column_id";
	/*
	 *	Identify "serial" fields by looking
	 *	at sequences which have matching names
	 */
	char * sql_seqs =
		"select "
			"sequence_name "
			"from user_sequences "
			"where sequence_name in "
			"("
				"select column_name "
				"from user_tab_columns "
				"where table_name = :1)";


	static int col_setup = FALSE;

	/*
	 *  Try set up everytime - might be slow but solve core dumps
	 */
	/* if (col_setup)
		return; */

	/*
	 *	Column count query
	 */
	if (oparse (colc, sql_colc, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (colc, 1,
			(ub1 *) col_table, sizeof (col_table), SQLT_STR,
			-1, NULL, NULL, -1, -1))
	{
		oraclecda_error (lda, colc, "colc:parse+bndrn");
	}

	/*	Result to "col_count"
	 */
	if (odefin (colc, 1,
			(ub1 *) &col_count, sizeof (int), SQLT_INT, 
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code))
	{
		oraclecda_error (lda, colc, "colc:odefin");
	}

	/*
	 *	Attribute query
	 */
	if (oparse (cols, sql_cols, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (cols, 1,
			(ub1 *) col_table, sizeof (col_table), SQLT_STR,
			-1, NULL, NULL, -1, -1))
	{
		oraclecda_error (lda, cols, "cols:parse+bndrn");
	}

	/*
	 *	Result output and indicators
	 */
	if (odefin (cols, 1,
			(ub1 *) col_name, sizeof (col_name), SQLT_STR,
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cols, 2,
			(ub1 *) col_data_type, sizeof (col_data_type), SQLT_STR,
			-1, &i_data_type, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cols, 3,
			(ub1 *) &col_data_length, sizeof (int), SQLT_INT, 
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cols, 4,
			(ub1 *) &col_data_precision, sizeof (int), SQLT_INT, 
			-1, &i_data_precision, NULL, -1, -1,
			&r_len, &r_code) ||
		odefin (cols, 5,
			(ub1 *) &col_data_scale, sizeof (int), SQLT_INT, 
			-1, &i_data_scale, NULL, -1, -1,
			&r_len, &r_code))
	{
		oraclecda_error (lda, cols, "colc:odefin");
	}

	/*
	 *	Sequence query
	 */
	if (oparse (seqs, sql_seqs, -1, PARSE_NOW, PARSE_V7) ||
		obndrn (seqs, 1,
			(ub1 *) col_table, sizeof (col_table), SQLT_STR,
			-1, NULL, NULL, -1, -1))
	{
		oraclecda_error (lda, seqs, "seqs:parse+bndrn");
	}

	/*
	 *	Result
	 */
	if (odefin (seqs, 1,
			(ub1 *) &seq_name, sizeof (seq_name), SQLT_STR, 
			-1, NULL, NULL, -1, -1,
			&r_len, &r_code))
	{
		oraclecda_error (lda, seqs, "seqs:odefin");
	}


	/*
	 */
	col_setup = TRUE;
}


static void
MapViewToColumns (
 TableState * table)
{
	/*
	 *	Map the view references to actual ColumnDefs
	 */
	int i;

	table -> viewactual = malloc (table -> viewc * sizeof (int));
	memset (table -> viewactual, 0, table -> viewc * sizeof (int));
	for (i = 0; i < table -> viewc; i++)
	{
		int c;

		for (c = 0; c < table -> columnc; c++)
			if (!strcmp (table -> view [i].vwname, table -> columns [c].name))
			{
				/*
				 *	Store position of ColumnDefinition for later use
				 */
				table -> viewactual [i] = c;
				break;
			}

		if (c >= table -> columnc)
			oracledbif_error (
				"Column %s not found",
				table -> view [i].vwname);
	}
}

static void
MapViewToUserData (
 TableState * table)
{
	/*
	 *	Work out the offsets into the user application data-buffer
	 */
	int i;
	int offset = 0;
//printf("MAPVIEWTOUSERDATA\n");
	for (i = 0; i < table -> viewc; i++)
	{
		ColumnDef * column = table -> columns + table -> viewactual [i];

//printf("%s offset %d column type %d", column->name, offset, column ->type );
		offset += _application_align (offset, column -> type);
//printf(" offset new %d\n", offset );


		table -> view [i].vwstart = offset;
		table -> view [i].vwtype = column -> type;

		table -> view [i].vwlen = column -> type == OT_Chars ?
			column -> length :					/* just happens to work out */
			_application_size (column -> type);

		offset += table -> view [i].vwlen;
	}
}

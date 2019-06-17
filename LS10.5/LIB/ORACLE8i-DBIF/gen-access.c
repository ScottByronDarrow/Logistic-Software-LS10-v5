/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: gen-access.c,v 5.0 2002/05/08 01:30:08 scott Exp $
|  Program Name  : (gen-access.c)
|  Program Desc  : (General access functions)
|---------------------------------------------------------------------|
| $Log: gen-access.c,v $
| Revision 5.0  2002/05/08 01:30:08  scott
| CVS administration
|
| Revision 1.1  2002/03/11 11:08:42  cha
| Added files and code checked.
|
=====================================================================*/

#include	<stdlib.h>
#include	<string.h>
#include	<dbio.h>

#include	"oracledbif.h"
#include	"oraerrs.h"

/*----------+
| Constants |
+----------*/

#define	NAMEBUF_LEN	32					/* ORACLE names are 30 chars long */
#define	MAXKEYS		8					/* We're limited by INFORMIX roots */

/*-------------------------------+
| Wrapper structures for catalog |
+-------------------------------*/

struct _IndexColInfo					/* Index + column info */
{
	struct IndexInfo index;
	int cols [MAXKEYS];
};
typedef struct _IndexColInfo IndexColInfo;

struct _TableInfoList
{
	struct TableInfo info;
	struct ColumnInfo * columns;

	IndexColInfo * indexes;

	struct _TableInfoList * next;
};
typedef struct _TableInfoList TableInfoList;

/*-----------------+
| Global variables |
+-----------------*/

static text *database = (text *) "LSDB";
static text *username = (text *) "logistic";
static text *password = (text *) "logistic";

static	OCIEnv		*envhpGA;
static	OCIServer	*srvhpGA;
static	OCIError	*errhpGA;
static	OCISvcCtx	*svchpGA;
static	OCIStmt		*stmthp01, 
					*stmthp02,
					*stmthp03,
					*stmthp04,
					*stmthp05,
					*stmthp06;				
static	OCIDefine	*defnp01 	= (OCIDefine *) 0,
					*defnp02 	= (OCIDefine *) 0,
					*defnp03 	= (OCIDefine *) 0,
					*defnp04 	= (OCIDefine *) 0,
					*defnp05 	= (OCIDefine *) 0,
					*defnp06 	= (OCIDefine *) 0;
static 	OCIBind		*bndp03 	= (OCIBind *) 0,
					*bndp04 	= (OCIBind *) 0,
					*bndp05 	= (OCIBind *) 0,
					*bndp06 	= (OCIBind *) 0;

dvoid 	*tmp;

static int tablecount = -1;
TableInfoList * catalog = NULL;
static ub2	r_len, r_code;									/* discard */

/*-------------------+
| External functions |
+-------------------*/

extern char * clip (char *);

/*----------------+
| Local functions |
+----------------*/

static void AddTableInfoList (TableInfoList **, TableInfoList *),
			DestroyTableInfoList (TableInfoList **);
static TableInfoList * TableInfoListNode (const char *, int, int);
static TableInfoList * LocateTableInfoByNo (int),
					 * LocateTableInfoByName (const char *);
static void Prep1TableCatalog (char *, int *),
			Prep2TableCatalog (char *, int *),
			Prep1IndexCatalog (char *, char *, char *),
			Prep2IndexCatalog (char *, char *, char *),
			Prep1ColumnCatalog (char *, char *),
			Prep2ColumnCatalog (char * table,
								char * name,
								char * data_type, 
								sb2 * i_data_type,
								int * data_length, 
								sb2 * i_data_length,
								int * data_precision, 
								sb2 * i_data_precision,
								int * data_scale, 
								sb2 * i_data_scale);
static enum ColumnType ORA2ColType (enum ORAType oratype);
static void OpenDB ();
static void CloseDB ();
static void cleanup ();

/*-------------------+
| External interface |
+-------------------*/

/*===========+
| TableCount |
+===========*/

int
TableCount ()
{
	/*-------------------------------
	| Initialise the catalog system |
	-------------------------------*/

	if (tablecount < 0)
	{
		char tablename [NAMEBUF_LEN];
		int count;

		OpenDB ();

		/*---------------------------------
		| First pass to get column counts |
		---------------------------------*/

		Prep1TableCatalog (tablename, &count);
		for (tablecount = 0; 
		 	 OCIStmtFetch (stmthp01,
						   errhpGA,
						   (ub4) 1,
						   (ub4) OCI_FETCH_NEXT,
						   (ub4) OCI_DEFAULT) == OCI_SUCCESS;
			 tablecount++)
		{
			/*-----------------------------------------
			| Add a dummy value for number of indexes | 
			| - we update this on second pass         |
			-----------------------------------------*/

			AddTableInfoList (&catalog, 
				TableInfoListNode (_strlower (clip (tablename)), count, 0));
		}

		/*---------------------------------
		| Second pass to get index counts |
		---------------------------------*/

		Prep2TableCatalog (tablename, &count);
		while (OCIStmtFetch (stmthp02,
							 errhpGA,
						     (ub4) 1,
						     (ub4) OCI_FETCH_NEXT,
						     (ub4) OCI_DEFAULT) == OCI_SUCCESS)
		{
			/*----------------------------------
			| Let's be safe and locate by name |
			----------------------------------*/

			TableInfoList * node = LocateTableInfoByName (
										_strlower (clip (tablename)));

			node -> info.nindexes = count;
		}
		CloseDB ();
	}

	return tablecount;
}

/*============+
| TableNumber |
+============*/

int
TableNumber (
	const char * name)
{
	int i;
	TableInfoList * node;

	for (i = 0, node = catalog; node; i++, node = node -> next)
		if (!strcmp (name, node -> info.name))
			return i;

	return -1;
}

/*==========+
| TableInfo |
+==========*/

void
TableInfo (
	int tableno,
	struct TableInfo * info)
{
	if (tableno >= tablecount)
		oracledbif_error ("TableInfo: table number %d out of range", tableno);

	*info = LocateTableInfoByNo (tableno) -> info;
}

/*===============+
| TableIndexInfo |
+===============*/

void
TableIndexInfo (
	int tableno,
	int indexno,
	struct IndexInfo * info)
{
	TableInfoList * node = NULL;

	if (tableno >= tablecount)
	{
		oracledbif_error (
			"TableIndexInfo: table number %d out of range",
			tableno);
	}

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes)
	{
		oracledbif_error (
			"TableIndexInfo: index number %d out of range",
			indexno);
	}

	if (!node -> indexes)
	{
		/*----------------------------------------
		| Have to build up the index information |
		----------------------------------------*/

		char tab_name [NAMEBUF_LEN],
			 idx_name [NAMEBUF_LEN],
			 col_name [NAMEBUF_LEN],
			 unique [NAMEBUF_LEN];
		unsigned sz;
		int i, j;

		OpenDB ();

		/*---------------------------------------
		| Allocate index buffers and build info |
		---------------------------------------*/

		sz = node -> info.nindexes * sizeof (IndexColInfo);
		memset (node -> indexes = malloc (sz), 0, sz);

		_strupper (strcpy (tab_name, node -> info.name));
		Prep1IndexCatalog (tab_name, idx_name, unique);

		for (i = 0; 
			 OCIStmtFetch (stmthp03,
						   errhpGA,
						   (ub4) 1,
						   (ub4) OCI_FETCH_NEXT,
						   (ub4) OCI_DEFAULT) == OCI_SUCCESS; 
			 i++)
		{
			_strlower (strcpy (node -> indexes [i].index.name, idx_name));
			node -> indexes [i].index.isunique = unique [0] == 'U';
		}

		/*-----------------
		| Get column info |
		-----------------*/

		Prep2IndexCatalog (tab_name, idx_name, col_name);

		i = -1;
		while (OCIStmtFetch (stmthp04,
							 errhpGA,
						     (ub4) 1,
						     (ub4) OCI_FETCH_NEXT,
						     (ub4) OCI_DEFAULT) == OCI_SUCCESS)
		{
			int col;

			_strlower (idx_name);
			if (i < 0 || strcmp (idx_name, node -> indexes [i].index.name))
			{
				if (i >= 0)
					node -> indexes [i].index.ncolumn = j;
				i++;
				j = 0;
			}

			_strlower (col_name);

			/*--------------------------
			| Find the matching column |
			--------------------------*/

			for (col = 0; col < node -> info.ncolumn; col++)
			{
				if (!strcmp (col_name, node -> columns [col].name))
				{
					node -> indexes [i].cols [j++] = col;
					break;
				}
			}

			if (col >= node -> info.ncolumn)
			{
				oracledbif_error (
					"TableIndexInfo: no col-match found %s",
					col_name);
			}
		}

		if (i >= 0)
		{
			/*------------------------
			| Pick up trailing index |
			------------------------*/

			node -> indexes [i].index.ncolumn = j;
		}
		CloseDB ();
	}

	*info = node -> indexes [indexno].index;
}

/*=====================+
| TableIndexColumnInfo |
+=====================*/

void
TableIndexColumnInfo (
	int tableno,
	int indexno,
	int colno,
	struct ColumnInfo * info)
{
	TableInfoList * node = NULL;

	if (tableno >= tablecount)
	{
		oracledbif_error (
			"TableIndexColumnInfo: table number %d out of range",
			tableno);
	}

	node = LocateTableInfoByNo (tableno);
	if (indexno >= node -> info.nindexes ||					/* bad indexno */
		!node -> indexes ||									/* no info */
		colno >= node -> indexes [indexno].index.ncolumn)	/* bad colno */
	{
		oracledbif_error (
			"TableIndexColumnInfo: bad index/column number %d/%d",
			indexno, colno);
	}

	*info = node -> columns [node -> indexes [indexno].cols [colno]];
}

/*=================+
| TableColumnCount |
+=================*/

int
TableColumnCount (
	const char * name)
{
	TableState * table = LocateTable (name);
	TableInfoList * node = NULL;

	if (table)
		return ( table -> columnc - 1); /* RPC */

	/*-------------------------------------------------------
	| Check the catalog system files for a possible listing |
	-------------------------------------------------------*/

	if ((node = LocateTableInfoByName (name)))
		return node -> info.ncolumn;

	oracledbif_error ("TableColumnCount: No table %s", name);
	return 0;
}

/*================+
| TableColumnInfo |
+================*/

void
TableColumnInfo (
	const char * name,
	int colno,
	struct ColumnInfo * buffer)
{
	TableState * table = LocateTable (name);

	if (table)
	{
		if (colno >= table -> columnc)
			oracledbif_error ("TableColumnInfo: bad colno %d", colno);

		/*---------------------------------
		| Convert ColumnDef to ColumnInfo |
		---------------------------------*/

		strcpy (buffer -> name, table -> columns [colno].name);
		buffer -> size = table -> columns [colno].length;
		buffer -> type = ORA2ColType (table -> columns [colno].type);

	} else
	{
		/*-----------------------
		| Check offline catalog |
		-----------------------*/

		TableInfoList * node = LocateTableInfoByName (name);

		if (!node)
			oracledbif_error ("TableColumnCount: bad table %s", name);

		if (colno >= node -> info.ncolumn)
			oracledbif_error ("TableColumnInfo: bad colno %d", colno);

		if (!node -> columns)
		{
			/*--------------------------
			| Build up the column info |
			--------------------------*/

			char tab_name [NAMEBUF_LEN],
				 seq_name [NAMEBUF_LEN],
				 col_name [NAMEBUF_LEN],
				 data_type [NAMEBUF_LEN];
			int data_length, data_precision, data_scale;
			sb2 i_data_type, i_data_length, i_data_precision, i_data_scale;
			unsigned sz;
			int i;

			OpenDB ();

			_strupper (strcpy (tab_name, node -> info.name));
			Prep1ColumnCatalog (tab_name, seq_name);
			Prep2ColumnCatalog (tab_name,
								col_name,
								data_type, 
								&i_data_type,
								&data_length, 
								&i_data_length,
								&data_precision, 
								&i_data_precision,
								&data_scale, 
								&i_data_scale);

			/*---------------------------------
			| Allocate buffers and fill it up |
			---------------------------------*/

			sz = node -> info.ncolumn * sizeof (struct ColumnInfo);
			memset (node -> columns = malloc (sz), 0, sz);

			for (i = 0; 
				 OCIStmtFetch (stmthp06,
							   errhpGA,
						       (ub4) 1,
						       (ub4) OCI_FETCH_NEXT,
						       (ub4) OCI_DEFAULT) == OCI_SUCCESS; 
				 i++)
			{
				enum ORAType type;
				int size;

				_strlower (strcpy (node -> columns [i].name, col_name));

				_DecodeORAType (
					node -> columns [i].name, seq_name,
					data_type, data_length, data_precision, data_scale,
					i_data_precision, i_data_scale,

					&type, &size);

				node -> columns [i].type = ORA2ColType (type);

				/*-----------------------------------------------
				| Tweak the size for external use, since        |
				| it's only suitable for internal use on Decode |
				-----------------------------------------------*/

				node -> columns [i].size =
					node -> columns [i].type == CT_Chars ?
					size - 1 : 0;
			}

//			if (col_cda.rc != ERR_NO_DATA_FOUND)
//				oraclecda_error (lda, &col_cda, "eh");

			CloseDB ();
		}

		*buffer = node -> columns [colno];
	}
}

/*===============+
| TableColumnGet |
+===============*/

void
TableColumnGet (
	const char * name,
	int colno,
	void * buffer)
{
	TableState * table = LocateTable (name);

#if 0
	if (!table)
		dbase_err ("TableColumnGet", name, NOFILENAME);
	if (colno >= table -> columncount)
		dbase_err ("TableColumnGet", name, NOFIELD);
#endif

	_ConvRaw2App (table -> columns + colno, 0, buffer);
}

/*===================+
| TableColumnNameGet |
+===================*/

void
TableColumnNameGet (
	const char * name,
	const char * colname,
	void * buffer)
{
	int colno;
	TableState * table = LocateTable (name);

#if 0
	if (!table)
		dbase_err ("TableColumnGet", name, NOFILENAME);
#endif
	for (colno = 0; colno < table -> columnc; colno++)
	{
		if (!strcmp (colname, table -> columns [colno].name))
		{
			TableColumnGet (name, colno, buffer);
			return;
		}
	}

	oracledbif_error ("No such column %s in %s", colname, name);
}

/*------------------+
| Support functions |
+------------------*/
/*==================+
| Prep1TableCatalog |
+==================*/

static void
Prep1TableCatalog (
	char * name,
	int * count)
{
	/*-----------------------------------------------------
	| parse and bind output for pass to get column counts |
	-----------------------------------------------------*/

	char * sql_tab =
			"select table_name, count (column_name) "
			"from user_tab_columns "
			"group by table_name";

	/*----------------------------------
	| Allocate and initialize stmthp01 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp01,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*---------------------------
	| Prepare sql_tab statement |
	---------------------------*/

	if (OCIStmtPrepare (stmthp01, 
						errhpGA, 
						(char *) sql_tab,
						(ub4) strlen (sql_tab), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep1TableCatalog");

	/*--------------------------
	| Result to name and count |
	--------------------------*/

	if (OCIDefineByPos (stmthp01,
						&defnp01,
						errhpGA,
						1,
						(dvoid *) &name,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep1TableCatalog");

	if (OCIDefineByPos (stmthp01,
						&defnp01,
						errhpGA,
						2,
						(dvoid *) &count,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep1TableCatalog");

	/*-----------------
	| Execute sql_tab |
	-----------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp01, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep1TableCatalog");
}

/*==================+
| Prep2TableCatalog |
+==================*/

static void
Prep2TableCatalog (
	char * name,
	int * count)
{
	/*----------------------------------------------------
	| parse and bind output for pass to get index counts |
	----------------------------------------------------*/

	char * sql_tab =
			"select table_name, count (index_name) "
			"from user_indexes "
			"group by table_name";

	/*----------------------------------
	| Allocate and initialize stmthp02 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp02,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*---------------------------
	| Prepare sql_tab statement |
	---------------------------*/

	if (OCIStmtPrepare (stmthp02, 
						errhpGA, 
						(char *) sql_tab,
						(ub4) strlen (sql_tab), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep2TableCatalog");

	/*--------------------------
	| Result to name and count |
	--------------------------*/

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhpGA,
						1,
						(dvoid *) &name,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2TableCatalog");

	if (OCIDefineByPos (stmthp02,
						&defnp02,
						errhpGA,
						2,
						(dvoid *) &count,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2TableCatalog");

	/*-----------------
	| Execute sql_tab |
	-----------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp02, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep2TableCatalog");
}

/*==================+
| Prep1IndexCatalog |
+==================*/

static void
Prep1IndexCatalog (
	char * table,
	char * index,
	char * unique)
{
	char * sql_idx =
		"select index_name, uniqueness "
		"from user_indexes "
		"where table_name = :1";

	/*----------------------------------
	| Allocate and initialize stmthp03 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp03,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*---------------------------
	| Prepare sql_idx statement |
	---------------------------*/

	if (OCIStmtPrepare (stmthp03, 
						errhpGA, 
						(char *) sql_idx,
						(ub4) strlen (sql_idx), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep1IndexCatalog");

	/*-----------------------------------------------
	| Bind the placeholder in the sql_idx statement |
	-----------------------------------------------*/
	
	if (OCIBindByPos (stmthp03,
			  		  &bndp03,
			  		  errhpGA,
			  		  1,
			  		  (dvoid *) &table,
			  		  (sword) NAMEBUF_LEN,
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIBindByPos:Prep1IndexCatalog");
		
	/*----------------------------
	| Result to index and unique |
	----------------------------*/

	if (OCIDefineByPos (stmthp03,
						&defnp03,
						errhpGA,
						1,
						(dvoid *) &index,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep1IndexCatalog");

	if (OCIDefineByPos (stmthp03,
						&defnp03,
						errhpGA,
						2,
						(dvoid *) &unique,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep1IndexCatalog");

	/*-----------------
	| Execute sql_idx |
	-----------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp03, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep1IndexCatalog");
}

/*==================+
| Prep2IndexCatalog |
+==================*/

static void
Prep2IndexCatalog (
	char * table,
	char * index,
	char * column)
{
	/*----------------------------------------------
	| Get associated columns for a table's indexes |
	----------------------------------------------*/

	char * sql_idx =
		"select index_name, column_name "
		"from user_ind_columns "
		"where table_name = :1 "
		"order by index_name, column_position";

	/*----------------------------------
	| Allocate and initialize stmthp04 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp04,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*---------------------------
	| Prepare sql_idx statement |
	---------------------------*/

	if (OCIStmtPrepare (stmthp04, 
						errhpGA, 
						(char *) sql_idx,
						(ub4) strlen (sql_idx), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep2IndexCatalog");

	/*-----------------------------------------------
	| Bind the placeholder in the sql_idx statement |
	-----------------------------------------------*/
	
	if (OCIBindByPos (stmthp04,
			  		  &bndp04,
			  		  errhpGA,
			  		  1,
			  		  (dvoid *) &table,
			  		  (sword) NAMEBUF_LEN,
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIBindByPos:Prep2IndexCatalog");
		
	/*----------------------------
	| Result to index and column |
	----------------------------*/

	if (OCIDefineByPos (stmthp04,
						&defnp04,
						errhpGA,
						1,
						(dvoid *) &index,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2IndexCatalog");

	if (OCIDefineByPos (stmthp04,
						&defnp04,
						errhpGA,
						2,
						(dvoid *) &column,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2IndexCatalog");

	/*-----------------
	| Execute sql_idx |
	-----------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp04, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep2IndexCatalog");	
}

/*===================+
| Prep1ColumnCatalog |
+===================*/

static void
Prep1ColumnCatalog (
	char * table,
	char * name)
{
	/*------------------------------------------------
	| Try to find associated sequences for the table |
	------------------------------------------------*/

	char * sql_seqs =
		"select "
			"sequence_name "
			"from user_sequences "
			"where sequence_name in "
			"("
				"select column_name "
				"from user_tab_columns "
				"where table_name = :1)";

	/*----------------------------------
	| Allocate and initialize stmthp05 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp05,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*----------------------------
	| Prepare sql_seqs statement |
	----------------------------*/

	if (OCIStmtPrepare (stmthp05, 
						errhpGA, 
						(char *) sql_seqs,
						(ub4) strlen (sql_seqs), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep1ColumnCatalog");

	/*------------------------------------------------
	| Bind the placeholder in the sql_seqs statement |
	------------------------------------------------*/
	
	if (OCIBindByPos (stmthp05,
			  		  &bndp05,
			  		  errhpGA,
			  		  1,
			  		  (dvoid *) &table,
			  		  (sword) NAMEBUF_LEN,
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIBindByPos:Prep1ColumnCatalog");
		
	/*----------------
	| Result to name |
	----------------*/

	if (OCIDefineByPos (stmthp05,
						&defnp05,
						errhpGA,
						1,
						(dvoid *) &name,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2IndexCatalog");

	/*------------------
	| Execute sql_seqs |
	------------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp05, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep2IndexCatalog");	
		
	/*--------------------------
	| Get sequence name if any |
	--------------------------*/

	if (OCIStmtFetch (stmthp05,
					  errhpGA,
					  (ub4) 1,
					  (ub4) OCI_FETCH_NEXT,
					  (ub4) OCI_DEFAULT))
		name [0] = '\0';
	else
		_strlower (name);
}

/*===================+
| Prep2ColumnCatalog |
+===================*/

static void
Prep2ColumnCatalog (
	char * table,
	char * name,
	char * data_type,
	sb2 * i_data_type,
	int * data_length,
	sb2 * i_data_length,
	int * data_precision,
	sb2 * i_data_precision,
	int * data_scale,
	sb2 * i_data_scale)
{
	/*-----------------------
	| Determine column info |
	-----------------------*/

	char * sql_cols =
		"select "
			"column_name, data_type, data_length, "
			"data_precision, data_scale "
		"from user_tab_columns "
		"where table_name = :1 "
		"order by column_id";

	/*----------------------------------
	| Allocate and initialize stmthp05 |
	----------------------------------*/

	checkerr (errhpGA, 
			  OCIHandleAlloc ((dvoid *) envhpGA, 
							  (dvoid **) &stmthp06,
							  (ub4) OCI_HTYPE_STMT, 
							  50, 
							  (dvoid **) &tmp));

	/*----------------------------
	| Prepare sql_cols statement |
	----------------------------*/

	if (OCIStmtPrepare (stmthp06, 
						errhpGA, 
						(char *) sql_cols,
						(ub4) strlen (sql_cols), 
						OCI_NTV_SYNTAX, 
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtPrepare:Prep2ColumnCatalog");

	/*------------------------------------------------
	| Bind the placeholder in the sql_cols statement |
	------------------------------------------------*/
	
	if (OCIBindByPos (stmthp06,
			  		  &bndp06,
			  		  errhpGA,
			  		  1,
			  		  (dvoid *) &table,
			  		  (sword) NAMEBUF_LEN,
			  		  SQLT_STR,
			  		  (dvoid *) 0,
			  		  (ub2 *) 0,
			  		  (ub2 *) 0,
			  		  0,
			  		  (ub4 *) 0,
			  		  OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIBindByPos:Prep2ColumnCatalog");
		
	/*-----------------------------------------------------------------------
	| Result to name, data_type, data_length, data_precision and data_scale |
	-----------------------------------------------------------------------*/

	if (OCIDefineByPos (stmthp06,
						&defnp06,
						errhpGA,
						1,
						(dvoid *) &name,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) 0,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2ColumnCatalog");

	if (OCIDefineByPos (stmthp06,
						&defnp06,
						errhpGA,
						2,
						(dvoid *) &data_type,
						(sword) NAMEBUF_LEN,
						SQLT_STR,
						(dvoid *) &i_data_type,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2ColumnCatalog");

	if (OCIDefineByPos (stmthp06,
						&defnp06,
						errhpGA,
						3,
						(dvoid *) &data_length,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) &i_data_length,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2ColumnCatalog");

	if (OCIDefineByPos (stmthp06,
						&defnp06,
						errhpGA,
						4,
						(dvoid *) &data_precision,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) &i_data_precision,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2ColumnCatalog");

	if (OCIDefineByPos (stmthp06,
						&defnp06,
						errhpGA,
						5,
						(dvoid *) &data_scale,
						(sword) sizeof (int),
						SQLT_INT,
						(dvoid *) &i_data_scale,
						(ub2 *) &r_len,
						(ub2 *) &r_code,
						OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIDefineByPos:Prep2ColumnCatalog");

	/*------------------
	| Execute sql_cols |
	------------------*/
	
	if (OCIStmtExecute (svchpGA, 
						stmthp06, 
						errhpGA, 
						(ub4) 0, 
						(ub4) 0, 
						(OCISnapshot *) NULL, 
						(OCISnapshot *) NULL, 
						(ub4) OCI_DEFAULT))
		oraclecda_error (errhpGA, "gen-access::OCIStmtExecute:Prep2ColumnCatalog");	
}

/*-----+
| List |
+-----*/
/*=================+
| AddTableInfoList |
+=================*/

static void
AddTableInfoList (
	TableInfoList ** list,
	TableInfoList * node)
{
	if (*list)
		AddTableInfoList (&(*list) -> next, node);
	else
		*list = node;
}

/*=====================+
| DestroyTableInfoList |
+=====================*/

static void
DestroyTableInfoList (
	TableInfoList ** list)
{
	if (!*list)
		return;

	DestroyTableInfoList (&(*list) -> next);

	if ((*list) -> columns)
		free ((*list) -> columns);
	if ((*list) -> indexes)
		free ((*list) -> indexes);
	free (*list);

	*list = NULL;
}

/*==================+
| TableInfoListNode |
+==================*/

static TableInfoList *
TableInfoListNode (
	const char * name,
	int colcount,
	int indexcount)
{
	TableInfoList * node = malloc (sizeof (TableInfoList));

	memset (node, 0, sizeof (TableInfoList));

	strcpy (node -> info.name, name);
	node -> info.ncolumn = colcount;
	node -> info.nindexes = indexcount;

	return node;
}

/*====================+
| LocateTableInfoByNo |
+====================*/

static TableInfoList *
LocateTableInfoByNo (
	int tableno)
{
	TableInfoList * node = NULL;

	for (node = catalog; tableno--; node = node -> next);

	return node;
}

/*======================+
| LocateTableInfoByName |
+======================*/

static TableInfoList *
LocateTableInfoByName (
	const char * name)
{
	TableInfoList * node = NULL;

	for (node = catalog; node; node = node -> next)
		if (!strcmp (name, node -> info.name))
			break;

	return node;
}

/*============+
| ORA2ColType |
+============*/

static enum ColumnType
ORA2ColType (
	enum ORAType oratype)
{
	switch (oratype)
	{
	case	OT_Chars:
			return CT_Chars;
	case	OT_Date:
			return CT_Date;
	case	OT_Serial:
			return CT_Serial;
	case	OT_Number:
			return CT_Long;
	case	OT_Money:
			return CT_Money;
	case	OT_Double:
			return CT_Double;
	case	OT_Float:
			return CT_Float;
	case	OT_RowId:
			return CT_Serial;
	default:
			break;
	}
	oracledbif_error ("TableColumnInfo: bad conversion type");
	return CT_Bad;
}

/*=======+
| OpenDB |
+=======*/

void OpenDB ()
{
	OCIInitialize ((ub4) OCI_THREADED | OCI_OBJECT, 
				   (dvoid *) 0,  
				   (dvoid * (*)()) 0,
				   (dvoid * (*)()) 0,  
				   (void (*)()) 0 );
  
	OCIHandleAlloc ((dvoid *) NULL, 
					(dvoid **) &envhpGA, 
					(ub4) OCI_HTYPE_ENV,
					52, 
					(dvoid **) &tmp);
  
	OCIEnvInit (&envhpGA, 
				(ub4) OCI_DEFAULT, 
				21, (dvoid **) &tmp);
  
	OCIHandleAlloc ((dvoid *) envhpGA, 
					(dvoid **) &errhpGA, 
					(ub4) OCI_HTYPE_ERROR,
					52, 
					(dvoid **) &tmp);

	/*------------
	| Logging ON |
	------------*/

    if (OCILogon (envhpGA,
				  errhpGA,
				  &svchpGA,
				  username,
				  (ub4) strlen (username),
				  password,
				  (ub4) strlen (password),
				  database,
				  (ub4) strlen (database)))
	{
		oraclecda_error (errhpGA, "gen-access::OCILogon:OpenDB");
	}	
	
	return;
}

/*========+
| CloseDB |
+========*/

void CloseDB ()
{
	/*-------------
	| Logging OFF |
	-------------*/

	if (OCILogoff (svchpGA, 
				   errhpGA))
	{
		oraclecda_error (errhpGA, "gen-access::OCILogoff:CloseDB");
	}	
			   	
	cleanup ();	

	return;
}

/*========+
| cleanup |
+========*/

void cleanup ()
{
	
	if (errhpGA)
		OCIServerDetach (srvhpGA,
						 errhpGA,
						 OCI_DEFAULT);
	if (srvhpGA)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) srvhpGA,
								 OCI_HTYPE_SERVER));
	if (svchpGA)
		(void) OCIHandleFree ((dvoid *) svchpGA,
							  OCI_HTYPE_SVCCTX);
	if (errhpGA)
		(void) OCIHandleFree ((dvoid *) errhpGA,
							  OCI_HTYPE_ERROR);
							  
	if (stmthp01)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp01,
								 OCI_HTYPE_STMT));
	if (stmthp02)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp02,
								 OCI_HTYPE_STMT));
	if (stmthp03)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp03,
								 OCI_HTYPE_STMT));
	if (stmthp04)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp04,
								 OCI_HTYPE_STMT));
	if (stmthp05)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp05,
								 OCI_HTYPE_STMT));
	if (stmthp06)
		checkerr (errhpGA,
				  OCIHandleFree ((dvoid *) stmthp06,
								 OCI_HTYPE_STMT));
	return;
}

%{
#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>

#include	<dbio.h>

#include	"commdefs.h"
#include	"tbldef.h"

extern TableDef	*cur_tbl;

/*
 *	Prototypes to get over warnings
 */
extern void yyerror (const char *);
extern int yylex (void);

%}

%union	{
	unsigned		ival;
	char			*sval;
	NameList		*nval;
	enum ColumnType	tval;
}

%token	TK_BAD

%token	TK_FILE
%token	TK_END
%token	TK_FIELD
%token	TK_TYPE
%token	TK_INDEX
%token	TK_DUPS
%token	TK_PRIMARY
%token	TK_COMPO
%token	TK_CHAR
%token	TK_DATE
%token	TK_DOUBLE
%token	TK_FLOAT
%token	TK_INT
%token	TK_LONG
%token	TK_MONEY
%token	TK_SERIAL
%token	<sval>	TK_NAME
%token	<ival>	TK_NUMBER
%token	TK_COMMA

%type	<sval>	std_field
%type	<nval>	fld_list
%type	<tval>	sng_type

%%
schema		:	/* empty */
					{
						warning ("Empty file\n");
					}
			|	TK_FILE TK_NAME
					{
						cur_tbl = NewTable ($2);
					}
				fields TK_END
			;

fields		:	field_def
			|	fields field_def
			;

field_def	:	index_def
			|	std_field
					{
						/*	Satisfy type conflicts
						 */
						;
					}
			|	std_field TK_INDEX TK_PRIMARY
					{
						AddIndex (cur_tbl, strdup ($1), I_Uniq);
						AddIndexFlds (cur_tbl, $1, AddName (NULL, strdup ($1)));
					}
			|	std_field TK_INDEX TK_DUPS
					{
						AddIndex (cur_tbl, strdup ($1), I_Dup);
						AddIndexFlds (cur_tbl, $1, AddName (NULL, strdup ($1)));
					}
			|	std_field TK_INDEX
					{
						AddIndex (cur_tbl, strdup ($1), I_Uniq);
						AddIndexFlds (cur_tbl, $1, AddName (NULL, strdup ($1)));
					}
			;

index_def	:	TK_FIELD TK_NAME TK_TYPE TK_COMPO fld_list TK_INDEX
					{
						AddIndex (cur_tbl, $2, I_Uniq);
						AddIndexFlds (cur_tbl, $2, $5);
					}
			|	TK_FIELD TK_NAME TK_TYPE TK_COMPO fld_list TK_INDEX TK_PRIMARY
					{
						AddIndex (cur_tbl, $2, I_Uniq);
						AddIndexFlds (cur_tbl, $2, $5);
					}
			|	TK_FIELD TK_NAME TK_TYPE TK_COMPO fld_list TK_INDEX TK_DUPS
					{
						AddIndex (cur_tbl, $2, I_Dup);
						AddIndexFlds (cur_tbl, $2, $5);
					}
			;

std_field	:	TK_FIELD TK_NAME TK_TYPE TK_CHAR TK_NUMBER
					{
						/*	This is a special case 'cos
							it's the only one with a length field
						 */
						AddField (cur_tbl, $2, CT_Chars, $5);
						$$ = $2;
					}
			|	TK_FIELD TK_NAME TK_TYPE sng_type
					{
						/*	Std case - singular types without length
						 */
						AddField (cur_tbl, $2, $4, 0);
						$$ = $2;
					}
			;

sng_type	:	TK_DATE
					{
						$$ = CT_Date;
					}
			|	TK_DOUBLE
					{
						$$ = CT_Double;
					}
			|	TK_FLOAT
					{
						$$ = CT_Float;
					}
			|	TK_INT
					{
						$$ = CT_Short;
					}
			|	TK_LONG
					{
						$$ = CT_Long;
					}
			|	TK_MONEY
					{
						$$ = CT_Money;
					}
			|	TK_SERIAL
					{
						$$ = CT_Serial;
					}
			;

fld_list	:	TK_NAME
					{
						$$ = AddName (NULL, $1);
					}
			|	fld_list TK_COMMA TK_NAME
					{
						$$ = AddName ($1, $3);
					}
			;

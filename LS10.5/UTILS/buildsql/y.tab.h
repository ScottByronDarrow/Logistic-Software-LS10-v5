
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
	{
	unsigned		ival;
	char			*sval;
	NameList		*nval;
	enum ColumnType	tval;
} YYSTYPE;
extern YYSTYPE yylval;
# define TK_BAD 257
# define TK_FILE 258
# define TK_END 259
# define TK_FIELD 260
# define TK_TYPE 261
# define TK_INDEX 262
# define TK_DUPS 263
# define TK_PRIMARY 264
# define TK_COMPO 265
# define TK_CHAR 266
# define TK_DATE 267
# define TK_DOUBLE 268
# define TK_FLOAT 269
# define TK_INT 270
# define TK_LONG 271
# define TK_MONEY 272
# define TK_SERIAL 273
# define TK_NAME 274
# define TK_NUMBER 275
# define TK_COMMA 276

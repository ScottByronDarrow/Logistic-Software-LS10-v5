#ifndef	TBLDEF_H
#define	TBLDEF_H

enum IndexType
{
	I_Bad,
	I_Uniq,
	I_Dup
};

struct tagFieldDef
{
	char			*name;
	enum ColumnType	type;
	unsigned		len;

	struct tagFieldDef	*next;
};
typedef	struct tagFieldDef	FieldDef;

struct tagNameList
{
	char	*name;
	struct tagNameList	*next;
};
typedef	struct tagNameList	NameList;

struct tagIndexDef
{
	char			*name;
	enum IndexType	type;
	NameList		*fldnames;

	struct tagIndexDef	*next;
};
typedef	struct tagIndexDef	IndexDef;

struct tagTableDef
{
	char		*name;
	FieldDef	*fields;
	IndexDef	*indexes;
};
typedef	struct tagTableDef	TableDef;

/*
 *	Prototypes
 */
extern TableDef	*NewTable (char *);
extern void		DelTable (TableDef *);

extern void		AddFld (FieldDef **, char *, enum ColumnType, unsigned),
				AddField (TableDef *, char *, enum ColumnType, unsigned),
				DelFields (FieldDef *);

extern FieldDef	*HasField (TableDef *, const char *);

extern NameList	*AddName (NameList *, char *);
extern int		NL_Eq (NameList *, NameList *);

extern void		AddIdx (IndexDef **, char *, enum IndexType),
				AddIndex (TableDef *, char *, enum IndexType),
				AddIdxFlds (IndexDef *, const char *name, NameList *),
				AddIndexFlds (TableDef *, const char *name, NameList *),
				DelIndexes (IndexDef *);

extern IndexDef	*HasIndex (TableDef *, const char *);

extern void		PrintTable (TableDef *),
				PrintAppSchema (TableDef *);

#endif	/*TBLDEF_H*/

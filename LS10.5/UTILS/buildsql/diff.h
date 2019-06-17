#ifndef	DIFF_H
#define	DIFF_H

struct tagDiff
{
	enum
	{
		D_Unchanged,
		D_Alter,
		D_Create,
		D_Drop
	}			tbl;

	FieldDef	*addf,
				*modf,
				*delf;
	IndexDef	*addi,
				*deli;

	FieldDef	*anchor;		/* anchor points for 'alter table add' */
};
typedef	struct tagDiff	Diff;

/*
 *	Prototypes
 */
extern Diff	*MakeDiff (TableDef *from, TableDef *to);
extern void	DelDiff (Diff *);

extern void	PrintDiff (Diff *, const char *, int);

#endif	/*DIFF_H*/

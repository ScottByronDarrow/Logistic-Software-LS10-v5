#ifndef	TBLNODE_H
#define	TBLNODE_H
/*******************************************************************************
$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/tblnode.h,v 5.0 2001/06/19 07:14:42 cha Exp $

	Define the structures used to maintain table state info

$Log: tblnode.h,v $
Revision 5.0  2001/06/19 07:14:42  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:07  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:33:06  jonc
 * Initial revision
 * 

*******************************************************************************/
#include	<sqlhdr.h>
#include	<sqlda.h>

enum CursorState
{
	CURSE_Null,	/*	Uninitialized	*/
	CURSE_Open,	/*	Open, and midway thru fetches	*/
	CURSE_Closed	/*	Closed, prepared	*/
};

/**	Strung together, these make up the fields of an index
**/
struct tagKeyField
{
	struct tagKeyField	*next;	/* Next part of index	*/

	char	*name;			/* Field name	*/
	short	colType;		/* SQL type	*/
};
typedef	struct tagKeyField	KeyField;

/**	Holds table state info
**/
struct tagTableNode
{
	struct tagTableNode	*next;	/* link to next node */

	char		*name,		/* on which node is referenced	*/
			*table;		/* table on which name is based on */

	KeyField	*keys;		/* index chosen (split into fields) */

	struct dbview	*fields;	/* pointer to user supplied struct */
	int		fldCount;	/* size of supplied field list 	*/
	KeyField	*unused;	/* unused fields */

	enum CursorState	curse;	/* internal state */
	struct sqlda	data;

	struct sqlda	updParams,	/* param structure for ins, del, upd */
			selParams;	/* param structure for selects */

	long		rowid;		/* current row's rowid */
	int		lock_fd;	/* lock file for table	*/

};
typedef	struct tagTableNode	TableNode;

/**
	Function prototypes
**/
extern KeyField		*NewKeyField (char *, short);
extern void		DelKeyField (KeyField **);
extern KeyField		*AddKeyField (KeyField **, KeyField *);

extern TableNode	*NewTableNode (char *, char *, struct dbview *, int);
extern void		DelTableNodes ();
extern TableNode	*AddTableNode (TableNode *);
extern TableNode	*GetTableNode (char *);
extern int		DetTableNode (TableNode *);

extern struct sqlvar_struct	*GetColumn (TableNode *, char *);

#endif	/*TBLNODE_H*/

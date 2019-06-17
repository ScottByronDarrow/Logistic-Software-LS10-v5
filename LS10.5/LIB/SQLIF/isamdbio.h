#ifndef	ISAMDBIO_H
#define	ISAMDBIO_H
/*******************************************************************************

	From std system

*******************************************************************************/

/* values passed to dbselfield() */

#define ACCKEYED	0
#define ACCPRIMARY	1
#define ACCSEQUENTIAL	2

struct dbview
{
	char	*vwname;	/* name of attribute	*/
	int	vwstart;	/* offset in view	*/
	int	vwtype;		/* type of attribute	*/
	int	vwlen;		/* length in bytes	*/
};

#define COMPARISON	1
#define FIRST		2
#define LAST		3
#define NEXT		4
#define PREVIOUS	5
#define EQUAL		6
#define GTEQ		7
#define GREATER		8
#define CURRENT		9
#define	LT			10
#define	LTEQ		11
#define LOCK		0400

#define SERIALTYPE	(0x0100 + LONGTYPE)
#define SERIALSIZE	LONGSIZE
#define DATETYPE	(0x0200 + LONGTYPE)
#define EDATETYPE	(0x0300 + LONGTYPE)
#define YDATETYPE	(0x0400 + LONGTYPE)
#define DATESIZE	LONGSIZE
#define MONEYTYPE	(0x0500 + DOUBLETYPE)
#define MONEYSIZE	DOUBLESIZE
#define TIMETYPE	(0x0600 + LONGTYPE)
#define TIMESIZE	LONGSIZE
#define COMPOSTYPE	(-1)

extern double round();

#define	CENTS(d)	(round((d)*100.0))
#define DOLLARS(c)	((c)/100.0)
#define ROUND(c)	(round(c))

/***	Prototypes for dbase_utils	***/
extern int	open_rec (char *, struct dbview *, int, char *),
		_open_rec (char *, struct dbview *, int, char *, int);

extern int	abc_fopen (char *),
		abc_fclose (char *);

extern int	abc_dbopen (char *),
		abc_dbclose (char *);

extern int	abc_selfield (char *, char *);

extern long	abc_rowid (char *);

extern int	abc_add (char *, char *),
		abc_update (char *, char *),
		abc_delete (char *);

extern int	abc_alias (char *, char *);

extern int	abc_unlock (char *),
		abc_flock (char *),
		abc_funlock (char *);

extern int	find_rec (char *, char *, int, char *),
		find_hash (char *, char *, int, char *, long);

extern void	dbase_err (char *, char *, int);

/**	For backward compatibility	**/
extern int	dbfind (char *, int, char *value, int *, char *);

#endif	/*ISAMDBIO_H*/

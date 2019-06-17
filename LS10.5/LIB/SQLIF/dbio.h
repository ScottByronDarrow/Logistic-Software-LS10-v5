#ifndef	DBIO_H
#define	DBIO_H
#include	<decimal.h>
#include	<isam.h>
#include	<sqltypes.h>

#define DBOPEN     1
#define DBCLOSE    2
#define FILEOPEN   3
#define RELOPEN    3
#define FILECLOSE  4
#define RELCLOSE   4

#define OPENMASK	0x00ff
#define EXCLUSIVE	0x0100
#define READONLY	0x0200
#define READWRITE	0x0000

/* values passed to dbselfield() */
#define ACCKEYED	0
#define ACCPRIMARY	1
#define ACCSEQUENTIAL	2

struct dbview
{
	char	*vwname;	/* name of attribute */
	int		vwstart,	/* offset in view */
			vwtype,		/* type of attribute */
			vwlen;		/* length in bytes */
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

struct	audinfo
{
	char	aud_name[128];
	long	rec_size;
};

/*
 *	Function prototypes
 */
extern int	open_rec (const char *, struct dbview *, int, const char *),
			abc_fclose (const char *);

extern int	abc_selfield (const char *, const char *);
extern int	abc_alias (const char *, const char *);
extern long	abc_rowid (const char *);

extern int	find_rec (const char *, void *, int, const char *);
extern int	find_hash (const char *, void *, int, const char *, long);

extern int	abc_add (const char *, void *),
			abc_update (const char *, void *),
			abc_unlock (const char *);

extern int	begin_work(), commit_work(), rollback_work();

#endif	/*DBIO_H*/

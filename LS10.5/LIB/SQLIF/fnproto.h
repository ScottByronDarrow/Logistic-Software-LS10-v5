#ifndef	FNPROTO_H
#define	FNPROTO_H
/*******************************************************************************
$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/fnproto.h,v 5.0 2001/06/19 07:14:40 cha Exp $

	Function prototypes - encourages type checking on compiles

$Log: fnproto.h,v $
Revision 5.0  2001/06/19 07:14:40  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:06  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:32:08  jonc
 * Initial revision
 * 

*******************************************************************************/

/***	dbopen.ec	***/
extern int	OpenDb (char *),
		CloseDb (char *);

/***	getrec.ec	***/
extern int	GetRecord (char *, char *, int),
		GetRecordOnLong (char *, char *, int, long);

/***	idx.ec	***/
extern int	IndexOn (char *, char *),
		ValidHashIdx (char *);

/***	lock.c	***/
extern void	InitLockSystem ();

extern int	LockTbl (char *),
		UnlockTbl (char *);

extern int	LockRec (char *, int),
		LockRecWithAbort (char *),
		UnlockRecs (char *);

/***	update.ec	***/
extern int	InsertRec (char *, char *),
		UpdateRec (char *, char *),
		DeleteRec (char *);

/***	view.ec	***/
extern int	MakeView (char *, struct dbview *, int);

#endif	/*	FNPROTO_H	*/

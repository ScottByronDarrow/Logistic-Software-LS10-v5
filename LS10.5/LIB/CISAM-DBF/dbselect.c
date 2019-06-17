#ident	"$Id: dbselect.c,v 5.2 2001/08/20 23:07:46 scott Exp $"
/*
 *	$Log: dbselect.c,v $
 *	Revision 5.2  2001/08/20 23:07:46  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.1  2001/08/06 22:47:56  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.3  2000/08/16 04:27:53  scott
 *	Updating for #define of AUDIT
 *	
 *	Revision 2.2  2000/08/16 02:08:54  scott
 *	Updated for audit stuff again
 *	
 *	Revision 2.1  2000/08/16 01:56:52  scott
 *	Updated to add #define of AUDIT to prevent code from being used
 *	if audit not defined.
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.6  2000/05/18 05:06:09  scott
 *	Updated to add #define for AUDIT
 *	
 *	Revision 1.5  1999/10/26 22:45:44  jonc
 *	Updated to use manifest constants.
 *	
 *	Revision 1.4  1999/09/30 04:57:28  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 *	Revision 1.3  1999/09/13 06:28:15  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.2  1999/07/14 23:47:26  jonc
 *	Added generic access support.
 *	
 */
#include	"dbcisam.h"
#include	<aud_lib.h>
#include	<std_decs.h>
#include	<dberr.h>

/*
 * Function declarations
 */
static int		init_sys (void),
				init_tab (LLIST *, LLIST *, char *);

static void		AddNode (LLIST **, LLIST *),
				DelNode (LLIST **, const char *),
				DelList (LLIST **);

static LLIST	*FindNode (LLIST *, const char *);

static LLIST	*AllocNode (const char *);
static void		FreeNode (LLIST *);

/*
 * Local Globals
 */
static LLIST	*sysList	= NULL,	/* list of system files */
				*fileList	= NULL;	/* list of open files */

static char	*SysTables	= "systables",
			*SysColumns	= "syscolumns",
			*SysIndexes	= "sysindexes";

/*
 * Real code
 */
int
dbselect (
 int flag,
 const char *name)
{
	int		i, err;
	int		lock_type;
	char	tabname[19];
	LLIST	*tptr;

	switch (flag & OPENMASK)
	{
	/*
	 * Open the database	
	 */
	case	DBOPEN:
		if (sysList)
			return (EXIT_SUCCESS);	/* Don't return 6002, just return 0 if already open */

		/*
		 * set dbpath			
		 */
		_dbpath = _get_dbpath(name);
		if (_dbpath == (char *)0)
			return iserrno ? iserrno : NOTEXIST;

		/*
		 * Initalise system list
		 */
		for (i = 0; i < MAX_SYS; i++)
		{
			switch (i)
			{
			case	SYS_TAB:
				tptr = AllocNode (SysTables);
				break;
				
			case	SYS_COL:
				tptr = AllocNode (SysColumns);
				break;
				
			case	SYS_IDX:
				tptr = AllocNode (SysIndexes);
				break;
			default	:
				return (-1);	/* internal error */
			}

			if (!tptr)
			{
				DelList (&sysList);	/* clear system list */
				return (ENOMEM);	/* error in initialising system file */
			}

			AddNode (&sysList, tptr);	/* add onto system list */

			tptr -> _open_type = ISINPUT + ISMANULOCK;
			/*
			 * initialise standard key part	
			 */
			KEYDEF.k_flags = ISNODUPS + DCOMPRESS + ISCLUSTER;
			KEYDEF.k_nparts = (short) 2;
			KEYPART(0).kp_start = (short) 0;
			KEYPART(0).kp_leng = (short) 18;
			KEYPART(0).kp_type = (short) CHARTYPE;
			/*
			 * second part of key for file	
			 */
			switch (i)
			{
			case	SYS_TAB:
				KEYPART(1).kp_start = (short) 18;
				KEYPART(1).kp_leng = (short) 8;
				KEYPART(1).kp_type = (short) CHARTYPE;
				break;

			case	SYS_COL:
				KEYPART(0).kp_start = SYSCOL_TABID;
				KEYPART(0).kp_leng = (short) LONGSIZE;
				KEYPART(0).kp_type = (short) LONGTYPE;
				KEYPART(1).kp_start = SYSCOL_COLNO;
				KEYPART(1).kp_leng = (short) INTSIZE;
				KEYPART(1).kp_type = (short) INTTYPE;
				break;

			case	SYS_IDX:
				KEYPART(1).kp_start = SYSIDX_TABID;
				KEYPART(1).kp_leng = (short) LONGSIZE;
				KEYPART(1).kp_type = (short) LONGTYPE;
				break;
			}
		}

		/*
		 * initialise system files	
		 */
		if ((err = init_sys ()))
			return (err);

		/*
		 * Initialise open audits count.	
		 */
#ifdef	AUDIT
		StartAudit ();
#endif	/* Audit */
		break;

	/*
	 * Close the database		
	 */
	case	DBCLOSE:
		if (!sysList)
			return (6000);

		while (fileList)
			abc_fclose (fileList -> _filename);

		/*
		 * close system files
		 */
		for (tptr = sysList; tptr; tptr = tptr -> _next)
		{
			err = isclose (tptr -> _fd);
			tptr -> _fd = -1;
			if (err < 0)
				return (iserrno);
		}
		DelList (&sysList);
		return(0);

	/*
	 * Open a database file	
	 */
	case	FILEOPEN:
		/*
		 * database is not open	
		 */
		if (!sysList)
			return(6004);

		if ((tptr = _GetNode (name)))
			return(0);	/* file has already been opened */

		/* 
		 * Allocate node
		 */
		if (!(tptr = AllocNode (name)))
			return NOTEXIST;

		/*
		 * find optional locking flag	
		 */
		lock_type = flag - FILEOPEN;
		tptr -> _open_type = ISINOUT +
			(lock_type == EXCLUSIVE ? ISEXCLLOCK : ISMANULOCK);

		/*
		 * initialise check buffer
		 */
		sprintf (tabname,"%-18.18s", _check_alias (name));

		/*
		 * initialise table entry
		 */
		if ((err = init_tab (tptr, _GetSysNode (SYS_TAB), tabname)))
			FreeNode (tptr);					/* free up erroneus entry */
		else
			AddNode (&fileList, tptr);			/* add to open file list */
		return (err);

	/*
	 * Close Database File		
	 */
	case	FILECLOSE:
		if (!(tptr = _GetNode (name)))
			return (EXIT_SUCCESS);	/* file was not open */

		/*
		 * close the audit file (If necessary)
		 */

#ifdef	AUDIT
		if (tptr -> _aud_fd != -1)
			stop_aud (tptr);
#endif	/*AUDIT*/

		/*
		 * close the file		
		 */
		err = isclose (tptr -> _fd);

		DelNode (&fileList, name);	/* remove node from open file list */

		return (err ? iserrno : 0);

	default:
		return(6014);
	}
	return(0);
}

/*
 * External interface
 */
LLIST *
_GetNode (
 const char *tabname)
{
	/*
	 * Returns a node from the opened files
	 */
	return (FindNode (fileList, tabname));
}

LLIST *
_GetSysNode (int sysid)
{
	/*
	 * Returns a node from the system file list
	 */
	switch (sysid)
	{
	case SYS_TAB	:
		return (FindNode (sysList, SysTables));
	case SYS_COL	:
		return (FindNode (sysList, SysColumns));
	case SYS_IDX	:
		return (FindNode (sysList, SysIndexes));
	}
	return (NULL);
}

/*
 * Allocate list node and initialise it
 */
static LLIST *
AllocNode (
 const char *tabname)
{
	LLIST	*node = (LLIST *) malloc (sizeof (LLIST));

	if (!node)
		return (NULL);

	memset (node, 0, sizeof (LLIST));	/* flush */

	node -> _fd = -1;
	node -> _serial = -1;
	node -> _filename = strdup (tabname);
	node -> _aud_fd = -1;
	node -> _tabid = -1;
	node -> timestamp = -1;

	return (node);
}

/*
 * Free up a Node
 */
static void
FreeNode (LLIST *node)
{
	free (node -> _filename);
	if (node -> _dirpath)
		free (node -> _dirpath);
	if (node -> _audpath)
		free (node -> _audpath);
	if (node -> _init)
		free (node -> _init);
	if (node -> _start)
		free (node -> _start);

	if (node -> _buffer)
		/* buffer has been offset for auditing purposes */
		free (node -> _buffer - (2 * aud_hdlen + node -> _buf_size));

	if (node -> columnlist)
		free (node -> columnlist);
	if (node -> columncisam)
		free (node -> columncisam);

	free (node);
}

/*
 * List management
 */
static void
AddNode (LLIST **list, LLIST *node)
{
	/*
	 * Adds 'node' to head of 'list'
	 */
	 node -> _next = *list;
	 *list = node;
}

static void
DelList (LLIST **list)
{
	/*
	 * Frees up all the nodes in 'list'
	 */
	while (*list)
	{
		LLIST	*dead = *list;

		*list = dead -> _next;	/* advance list head */
		FreeNode (dead);		/* wipe dead node */
	}
}

static void
DelNode (
 LLIST **list,
 const char *name)
{
	/*
	 * Removes and frees up a Node from the list
	 */
	while (*list)
	{
		if (!strcmp (name, (*list) -> _filename))
		{
			LLIST	*dead = *list;

			*list = (*list) -> _next;	/* advance the list */
			FreeNode (dead);			/* wipe dead node */
			return;
		}
		list = &(*list) -> _next;
	}
}

static LLIST *
FindNode (
 LLIST		*tptr,
 const char	*tabname)
{
	while (tptr)
	{
		if (!strcmp (tptr -> _filename, tabname))
			return (tptr);
		tptr = tptr -> _next;
	}
	return (NULL);
}

struct audinfo *
db_audinfo (
 const char *file)
{
	static	struct	audinfo	lcl_audinfo;
	LLIST	*tptr;

	tptr = _GetNode (file);
	if (tptr || tptr -> _aud_fd == -1)
		return ((struct audinfo *) 0);
	lcl_audinfo.rec_size = tptr -> _buf_size;
	if (*tptr -> _audpath == '/')
		sprintf (lcl_audinfo.aud_name, "%s.aud", tptr -> _audpath);
	else
		sprintf (lcl_audinfo.aud_name, "%s/%s.aud", _dbpath, tptr -> _audpath);
	return ((struct audinfo *) &lcl_audinfo);
}

/*
 * Initialize
 */
static	int
init_sys (void)
{
	int		i = 0, err;
	char	tabname [NAME_LEN + 1];
	char	sys_buf[201];
	char	buffer[201];
	LLIST	*sptr = _GetSysNode (SYS_TAB);
	LLIST	*tptr;

	/*
	 * initialise buffer	
	 */
	stchar (sptr -> _filename ,sys_buf, NAME_LEN);
	stchar (" ", sys_buf + NAME_LEN, 8);

	sprintf(buffer, "%s/%s", _dbpath, sptr -> _filename);
	sptr -> _fd = isopen (buffer, sptr -> _open_type);
	if (sptr -> _fd < 0)
		return (iserrno);
	/*
	 * set index & read first row
	 */
	if (isstart (sptr -> _fd, &sptr -> _key, 0, sys_buf, ISFIRST))
		return (iserrno);

	/*
	 * for all system files	
	 */
	for (i = 0; i < MAX_SYS; i++)
	{
		tptr = _GetSysNode (i);

		/*
		 * initialise test value	
		 */
		sprintf (tabname, "%-18.18s", tptr -> _filename);
		/*
		 * initialise table entry	
		 */
		if ((err = init_tab (tptr, sptr, tabname)))
			return (err);
	}
	return(0);
}

static	int
init_tab (LLIST *tptr, LLIST *sptr, char *tabname)
{
	char	sys_buf[201];
	char	buffer[201];

	/*
	 * initialise search key		
	 */
	stchar (tabname, sys_buf, NAME_LEN);
	stchar (" ", sys_buf + NAME_LEN, 8);

	/*
	 * perform read
	 */
	if (isread (sptr -> _fd, sys_buf, ISGTEQ))
		return (iserrno);

	/*
	 * find failed			
	 */
	if (strncmp (sys_buf, tabname + SYSTAB_TABNAME, 18))
		return NOTEXIST;

	/*
	 * check if actually a table	
	 */
	if (sys_buf [SYSTAB_TABTYPE] != 'T')
		return NOTEXIST;

	/*
	 * copy dirpath into buffer
	 */
	ldchar (sys_buf + SYSTAB_DIRPATH, 64, buffer);

	/*
	 *	Get various interesting stuff about the table
	 */
	tptr -> _dirpath = strdup (clip (buffer));
	tptr -> _audpath = strdup (buffer);
	tptr -> _tabid = ldlong (sys_buf + SYSTAB_TABID);
	tptr -> _buf_size = ldint (sys_buf + SYSTAB_ROWSIZE);

	tptr -> columncount = ldint (sys_buf + SYSTAB_NCOLS);
	tptr -> columnlist = malloc (sizeof (struct ColumnInfo) *
								 tptr -> columncount);
	tptr -> columncisam = malloc (sizeof (int) * tptr -> columncount);

	tptr -> _buffer = malloc (((tptr -> _buf_size + aud_hdlen) * 2) + 1);
	if (!tptr -> _buffer)
		return (ENOMEM);
	tptr -> _buffer += aud_hdlen + aud_hdlen + tptr -> _buf_size;

	/*
	 * non system table
	 */
	if (tptr -> _tabid >= 100L)
	{
		tptr -> _init = malloc ((unsigned) tptr -> _buf_size + 1);
		if (!tptr -> _init)
			return (ENOMEM);
	}
	/*
	 * file not open yet		
	 */
	if (tptr -> _fd < 0)
	{
#ifdef	AUDIT
		/*
		 * check if auditting enabled.	
		 */
		if (*tptr -> _audpath == '/')
			sprintf (buffer, "%s.aud", tptr -> _audpath);
		else
			sprintf (buffer, "%s/%s.aud", _dbpath, tptr -> _audpath);
		if (!access (buffer, 06))
			tptr -> _aud_fd = -2;
#endif	/*	AUDIT	*/

		/*
		 * check file permissions	
		 */
		if (*tptr -> _dirpath == '/')
			sprintf (buffer, "%s.dat", tptr -> _dirpath);
		else
			sprintf (buffer, "%s/%s.dat", _dbpath, tptr -> _dirpath);
		if (access(buffer,06))
			return((errno == 13) ? 6045 : errno);
		if (*tptr -> _dirpath == '/')
			strcpy (buffer, tptr -> _dirpath);
		else
			sprintf (buffer, "%s/%s", _dbpath, tptr -> _dirpath);
		while (tptr -> _fd < 0)
		{
			tptr -> _fd = isopen (buffer, tptr -> _open_type);
			if (tptr -> _fd < 0)
#ifdef AUDIT
				if (!lower_auds ())
#endif /* AUDIT */
					return(iserrno);
		}
		isstart (tptr -> _fd, &(KEYDEF), 0, tptr -> _buffer, ISFIRST);
	}
	return(0);
}

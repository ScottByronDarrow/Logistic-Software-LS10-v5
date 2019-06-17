#ident	"$Id: dbaudit.c,v 5.2 2001/08/20 23:07:45 scott Exp $"
/*
 *	Audit routines. Probably redundant.
 *
 *******************************************************************************
 *	$Log: dbaudit.c,v $
 *	Revision 5.2  2001/08/20 23:07:45  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.1  2001/08/06 22:47:56  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 07:07:42  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.1  2000/08/16 01:56:51  scott
 *	Updated to add #define of AUDIT to prevent code from being used
 *	if audit not defined.
 *	
 *	Revision 2.0  2000/07/15 07:32:17  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.4  1999/09/30 04:57:27  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 *	Revision 1.3  1999/09/29 00:04:22  jonc
 *	Corrected "long" vs "time_t" references.
 *	
 */

#ifdef	AUDIT
#include	"dbcisam.h"
#include	<std_decs.h>
#include	<aud_lib.h>
#include	<limits.h>

/*
 * Globals
 */
static int	_auds_open;

/*
 * Local Globals
 */
#define	MAX_AUDS	10
static	int	init_done = 0;
static	int	max_aud = MAX_AUDS;	/* Dynamically adjusted @ runtime */
struct
{
	LLIST	*tptr;
	long	tm_used;	/* Date/Time stamp when last used	*/
} aud_handle[MAX_AUDS];

/*
 *	Function prototypes
 */
static int	open_aud (LLIST *),
			close_aud (void),
			touch_aud (LLIST *tptr);
static void	raise_auds (void);

/*
 * Initialise the auditting subsystem.	
 */
static	void	init_aud (void)
{
	int	fd;

	for (fd = 0; fd < MAX_AUDS; fd++)
	{
		aud_handle[fd].tptr = NULL;
		aud_handle[fd].tm_used = -1L;
	}
	init_done = 1;
}

/*
 * External Interface
 */
void	StartAudit (void)
{
	_auds_open = 0;
}

/*
 * Stop all auditing on the desired file b4 isclose ().
 */
int	stop_aud (LLIST *tptr)
{
	int	i,
		j = -1;

	if (!init_done)
		init_aud ();

	for (i = 0; i < max_aud; i++)
		if (aud_handle[i].tptr == tptr)
			j = i;
	if (j != -1)
	{
		close (tptr -> _aud_fd);
		tptr -> _aud_fd = -1;
		aud_handle[j].tptr = NULL;
		aud_handle[j].tm_used = -1L;
		_auds_open--;
	}
	raise_auds ();
	return (EXIT_SUCCESS);
}

/*
 * Write out the audit record of	the desired type.
 */
int	write_aud (
	LLIST 	*tptr, 
	char 	*type, 
	long 	int 	recno)
{
	char	*sptr;
	int		i,
			out_size,
			termno;

	time_t	now;
	struct	AUD_HDR	local_aud;

	if (!init_done)
		init_aud ();

	i = touch_aud (tptr);
	if (i == -1)
		return (-1);

	termno = atoi (getenv ("TERM_SLOT"));
	now = time (NULL);
	sprintf (local_aud.op_id, "%-8.8s", getenv ("LOGNAME"));
	sprintf (local_aud.prog_name, "%-15.15s", PNAME);
	local_aud.action[0] = *type;
	stlong (now, local_aud.date_time);
	stint (termno, local_aud.termno);
	stlong (recno, local_aud.recno);

	out_size = tptr -> _buf_size + aud_hdlen;

	sptr = tptr -> _buffer - aud_hdlen;
	(void) memcpy (sptr, &local_aud, aud_hdlen);
	if (*type == 'w' || *type == 'W')
	{
		sptr -= out_size;
		local_aud.action[0] = 'R';
		(void) memcpy (sptr, &local_aud, aud_hdlen);
		out_size *= 2;
	}

	lseek (tptr -> _aud_fd, 0L, 2);
	i = write (tptr -> _aud_fd, sptr, out_size);
	if (i != out_size)
		return (-1);
	else
		return (EXIT_SUCCESS);
}

/*
 * Attempt to get back 1 of the audit file-handles so 
 * that other files may be opened.
 * Return:			
 *	0 - Could NOT lower!	
 *	1 - Lowered by 1.	
 */
int
lower_auds (void)
{
	int	i,
		j,
		k;
	long	oldest = LONG_MAX;

	if (!init_done)
		init_aud ();

	/*
	 * MUST reserve at least 1 audit handle.
	 */
	if (max_aud <= 1)
		return (EXIT_SUCCESS);

	/*
	 * If the highest is NOT being currently
	 * used, simply decrement max_aud.
	 */
	if (!aud_handle[max_aud - 1].tptr)
	{
		max_aud--;
		return (EXIT_FAILURE);
	}
	if (aud_handle[max_aud - 1].tptr -> _aud_fd == -1)
	{
		max_aud--;
		return (EXIT_FAILURE);
	}

	/*
	 * Find LEAST recently accessed audit
	 * handle and 'free' that up.		
	 */
	j = k = -1;
	for (i = 0; i < max_aud; i++)
	{
		/*
		 * Save index of first 'free'	
		 * entry in the array.		
		 */
		if
		(
			k == -1 &&
			aud_handle[i].tm_used == -1L &&
			aud_handle[i].tptr == (LLIST *) 0
		)
		{
			k = i;
			continue;
		}

		if (aud_handle[i].tm_used < oldest && aud_handle[i].tm_used >= 0L)
		{
			oldest = aud_handle[i].tm_used;
			j = i;
		}
	}

	max_aud--;
	if (j == max_aud)
	{
		if (k == -1)
		{
			close (aud_handle[j].tptr -> _aud_fd);
			aud_handle[j].tptr -> _aud_fd = -2;
			aud_handle[j].tptr = NULL;
			aud_handle[j].tm_used = -1L;
			_auds_open--;
			return (EXIT_FAILURE);
		}
		aud_handle[k].tptr = aud_handle[j].tptr;
		aud_handle[k].tm_used = aud_handle[k].tm_used;
		aud_handle[j].tptr = (LLIST *) 0;
		aud_handle[j].tm_used = -1L;
		return (EXIT_FAILURE);
	}

	if (j < 0)
		return (EXIT_SUCCESS);
	close (aud_handle[j].tptr -> _aud_fd);
	aud_handle[j].tptr -> _aud_fd = -2;
	aud_handle[j].tptr = aud_handle[max_aud].tptr;
	aud_handle[j].tm_used = aud_handle[max_aud].tm_used;
	aud_handle[max_aud].tptr = NULL;
	aud_handle[max_aud].tm_used = -1L;
	_auds_open--;
	return (EXIT_FAILURE);
}

/*
 * Increment the number of available	audit handles. (Up to a limit.)
 */
static void
raise_auds (void)
{
	if (!init_done)
		init_aud ();

	if (max_aud < MAX_AUDS)
		max_aud++;
	return;
}

/*
 * Set an audit to a state where it is ready to have a record 
 * 'logged' Also, update the date/time stamp.
 * Return:		
 *	-1	- No audit to be done.
 *	Other	- Audit-file handle.
 */
static	int	touch_aud (LLIST *tptr)
{
	int	i;

	if (!init_done)
		init_aud ();

	if (tptr -> _aud_fd == -1)
		return (-1);

	i = open_aud (tptr);

	if (i >= 0)
	{
		aud_handle[i].tm_used = time ((long *) 0);
		return (aud_handle[i].tptr -> _aud_fd);
	}
	return (-1);
}

/*
 * Open an audit file and return the
 * index used. NB: If already open, just
 * return the selected index.
 */
static	int	open_aud (LLIST *tptr)
{
	char	buffer[128];
	int	fd,
		i,
		j;
	long	rec_size;

	if (!init_done)
		init_aud ();

	/*
	 * Check to see if it's already done.
	 */
	for (i = 0; i < max_aud; i++)
		if (aud_handle[i].tptr == tptr)
			return (i);

	if (*tptr -> _audpath == '/')
		sprintf (buffer, "%s.aud", tptr -> _audpath);
	else
		sprintf (buffer, "%s/%s.aud", _dbpath, tptr -> _audpath);
	/*
	 * If we exceed available handles, close the LEAST
	 * recently accessed audit file.				
	 * This will free exactly one handle for our use.
	 */
	if (_auds_open >= max_aud)
		i = close_aud ();
	else
		for (i = 0, j = 0; j < max_aud; j++)
			if (aud_handle[j].tptr == (LLIST *) 0)
			{
				i = j;
				break;
			}

	fd = -1;
	while (fd == -1)
	{
		fd = open (buffer, O_RDWR);
		if (fd == -1 && !lower_auds ())
			return (-1);
	}
	if (read (fd, &rec_size, sizeof (long)) != sizeof (long))
	{
		close (fd);
		return (-1);
	}
	if (rec_size != tptr -> _buf_size + aud_hdlen)
	{
		close (fd);
		return (-1);
	}
	_auds_open++;
	aud_handle[i].tptr = tptr;
	aud_handle[i].tptr -> _aud_fd = fd;
	return (i);
}

/*
 * Close the file relating to the LEAST recently accessed audit.			
 * Return:			
 *	Index of closed audit.	
 */
static	int	close_aud(void)
{
	int	i,
		j = 0;
	long	oldest = LONG_MAX;

	if (!init_done)
		init_aud ();

	if (_auds_open == 0)
		return (EXIT_SUCCESS);

	for (i = 0; i < max_aud; i++)
	{
		if (aud_handle[i].tm_used < oldest && aud_handle[i].tm_used != -1L)
		{
			oldest = aud_handle[i].tm_used;
			j = i;
		}
	}

	close (aud_handle[j].tptr -> _aud_fd);
	aud_handle[j].tptr -> _aud_fd = -2;
	aud_handle[j].tm_used = -1L;
	aud_handle[j].tptr = NULL;
	_auds_open--;
	return (j);
}
#endif	/* AUDIT */

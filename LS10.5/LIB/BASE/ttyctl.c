#ident	"$Id: ttyctl.c,v 5.0 2001/06/19 06:59:41 cha Exp $"
/*
 *	Set/save terminal modes
 *
 *	There are 2 versions of these routines, dependant
 *	on whether the system has (correctly) implemented
 *	the POSIX tty control routines.
 *
 *	New-style (ie HAS_TERMIOS) is preferred. However,
 *	on older systems, you must use the old-style routines.
 *	On some newer systems, the old-style routines don't
 *	quite behave themselves; and then again on others
 *	the new-style doesn't work. Pick and choose.
 *
 *******************************************************************************
 *	$Log: ttyctl.c,v $
 *	Revision 5.0  2001/06/19 06:59:41  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:52:40  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:34:30  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:17:20  gerry
 *	Forced revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/09/24 02:32:00  jonc
 *	Added POSIX control routines; requires HAS_TERMIOS in <osdeps.h>
 *	
 */
#include	<std_decs.h>

static int	stty_saved = 0;

#ifdef	HAS_TERMIOS
/*
 *	Use POSIX routines
 */
#include	<termios.h>

static struct termios	lstty_org;		/* saved state */

#else	/*HAS_TERMIOS*/
/*
 *	Use old-style ioctl
 */
#include	<termio.h>

static struct termio	lstty_org;		/* saved state */

#endif	/*HAS_TERMIOS*/

void
set_tty (void)
{
#ifdef	HAS_TERMIOS
	/*
	 *	Use POSIX routines
	 */
	struct termios	rawtty;

	if (!stty_saved)
	{
		tcgetattr (STDIN_FILENO, &lstty_org);
		stty_saved = 1;
	}
	tcgetattr (STDIN_FILENO, &rawtty);

	/*
	 *	Set input modes
	 */
	rawtty.c_iflag =
		BRKINT |					/* interrupt on break */
		IGNPAR |					/* ignore parity */
		ISTRIP |					/* strip to 7 bits (we don't use meta) */
		IXANY;						/* just in case a stop has been set */

	/*
	 *	Set output modes
	 */
	rawtty.c_oflag = 0;				/* no output processing */

	/*
	 *	Line flags
	 */
	rawtty.c_lflag = 0;				/* raw */

	/*
	 *	Set cbreak mode
	 */
	rawtty.c_cc [VMIN] = 1;			/* minimum of one char */
	rawtty.c_cc [VTIME] = 0;		/* immediate return */

	if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &rawtty) == -1)
	{
		if (errno != ENOTTY)
		{
			fprintf (stderr, "Essential ioctl failed (%d)\n", errno);
			exit (EXIT_FAILURE);
		}
	}
#else	/*HAS_TERMIOS*/
	/*
	 *	Old-style ioctl
	 */
	struct termio	lstty;

	if (!stty_saved)
	{
		ioctl (0, TCGETA, &lstty_org);
		stty_saved = 1;
	}
	ioctl (0, TCGETA, &lstty);
#ifdef	NCR
	lstty.c_iflag &= (EXTEND | ERR_BEL | IRTS | IXOFF | IXANY | IXON);
#else	/*NCR*/
	lstty.c_iflag &= (IXOFF | IXANY | IXON);
#endif	/*NCR*/
	lstty.c_lflag = ECHOK;
	lstty.c_cc [VMIN] = 1;
	lstty.c_cc [VTIME] = 1;
	ioctl (0, TCSETA, &lstty);

#endif	/*HAS_TERMIOS*/
}

void
rset_tty (void)
{
	if (stty_saved)
	{
#ifdef	HAS_TERMIOS
		tcsetattr (STDIN_FILENO, TCSAFLUSH, &lstty_org);
#else	/*HAS_TERMIOS*/
		ioctl (0, TCSETA, &lstty_org);
#endif	/*HAS_TERMIOS*/
	}
}

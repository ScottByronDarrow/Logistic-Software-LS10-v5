/*=====================================================================
|  Copyright (C) 1999 - 2000 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( sleeper.c      )                                 |
|  Program Desc  : ( Routines for Handling Background Processes   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A                                               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A                                               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 26/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : (26/10/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (15.12.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments                                                           |
|  (15.12.94) : Moved from <alarm_time.h>.                            |
|             : Rewrote use of signals, delay mechanism               |
-----------------------------------------------------------------------
	$Log: sleeper.c,v $
	Revision 5.0  2001/06/19 06:59:39  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:39  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:26  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:19  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.9  2000/05/16 07:26:20  scott
	Updated for copyright
	
	Revision 1.8  2000/03/08 07:03:02  gerry
	Re-wrote signal_on ala ver8 so that all signals are handled
	
	Revision 1.7  2000/01/10 07:34:30  scott
	Updated to use SO_MAX_TIMEOUT environment to reduce or increase max timeout.
	
	Revision 1.6  1999/10/01 07:22:52  scott
	Updated for timeout
	
	Revision 1.5  1999/09/14 00:06:06  scott
	Updated for better date usage.
	Updated to use ver10 sleeper
	Updated to use ver10 tty_slot
	
	Revision 1.2  1999/08/07 03:37:25  scott
	Updated for warnings.
	
	Revision 1.2  1998/12/17 20:19:33  jonc
	Rewrote to use new-style signal handling. SCO appears to prefer this
	over the use of signal(3).
	
=====================================================================*/
#include	<signal.h>
#include	<osdefs.h>
#include	<unistd.h>
#include	<std_decs.h>

#define	TRUE		1
#define	FALSE		0

#define	MAX_DELAY	20						/* Maximum delay of 50 secs */

static unsigned	time_delay = 0;				/* delay variable */

extern int	prog_exit;						/* main control loop var */

int		envSoMaxTimeout	=	FALSE;
int		maxTimeout		=	MAX_DELAY;
/*
 *	Signal handlers
 */
static void
ResetTimer (
 int sig)
{
	/*
	 *	On Wakeups
	 */
	time_delay = 0;
}

static void
end_process (
 int sig)
{
	/*
	 * Handler for Signals causing termination
	 */
	sigset_t	mask;

	time_delay = 0;					/* reset delay to 0 */
	prog_exit = TRUE;

	/*
	 *	Block quit signals from reoccuring
	 */
	sigemptyset (&mask);
	sigaddset (&mask, SIGTERM);
	sigaddset (&mask, SIGINT);
	sigaddset (&mask, SIGUSR1);
	sigprocmask (SIG_BLOCK, &mask, NULL);
}

/*
 *	External interface
 */
void
time_out (void)
{

	/* Handles timing of wake up etc
	 *
	 * time_out period increments to a maximum
	 */
	if (prog_exit)
		return;

	if (envSoMaxTimeout == FALSE)
	{
		char	*sptr;

		/*------------------------------------
		| Check special zero due date.       |
		------------------------------------*/
		sptr = chk_env ("SO_TIMEOUT");
		maxTimeout	= (sptr == (char *)0) ? MAX_DELAY : atoi (sptr);

		envSoMaxTimeout	=	TRUE;
	}
	/*
	 *	Increase the sleep time
	 */
	if (time_delay < maxTimeout)
		time_delay++;				/* increase delay by 1 second */
	else
		time_delay = maxTimeout;		/* set a cap, just in case */

	sleep (time_delay);				/* signals will interrupt sleep */
}

/*
 *	Set the signal masks
 */
void
signal_on (void)
{
	/*
	 *	Set up signal dispositions
	 */
	int		i;

	sigset_t			sigmask;
	struct sigaction	action;

	time_delay = 0;

	/*
	 *  Run through signal list 
	 */
	for (i = 0; i < NSIG; ++i)
	{
		switch (i)
		{
			/*
     		* Termination signals
	 		*/

			case SIGTERM:
			case SIGINT:

				action.sa_handler = end_process;
				sigemptyset (&action.sa_mask);	/* clear all other options */
				action.sa_flags = SA_RESTART;
			
				break;

			/*
	 		 * Wakeups
	 		 */
			case SIGHUP:
			case SIGUSR1:

				action.sa_handler = ResetTimer;
				sigemptyset (&action.sa_mask);	/* clear all other options */
				action.sa_flags = SA_RESTART;
			
				break;

			/*
			 * Ignore the rest 
			 */
			default:
				action.sa_handler = SIG_IGN;
				sigemptyset (&action.sa_mask);
				action.sa_flags = 0;

				break;
		}

		sigaction (i, &action, NULL);
	}

	/*
	 *	Make sure the following (minimum) signals get thru'
	 */
	sigemptyset (&sigmask);
	sigaddset (&sigmask, SIGINT);
	sigaddset (&sigmask, SIGTERM);
	sigaddset (&sigmask, SIGHUP);
	sigprocmask (SIG_UNBLOCK, &sigmask, NULL);
}

void
set_timer (void)
{
	time_delay = 0;
}

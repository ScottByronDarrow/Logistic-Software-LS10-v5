#ifndef	ALARM_TIME_H
#define	ALARM_TIME_H

/*
 *	Routines managing time outs for std background processes
 *	lib : sleeper.c
 */
extern void	signal_on (void),				/* set signal masks */
			time_out (void),				/* sleeper routine */
			set_timer (void);				/* resets timer to 0 */

#endif	/*ALARM_TIME_H*/

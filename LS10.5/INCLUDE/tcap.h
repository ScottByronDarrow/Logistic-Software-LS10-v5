#ifndef	TCAP_H
#define	TCAP_H
/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( tcap.h         )                                 |
|  Program Desc  : ( Head file for tcap.c                         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  /etc/termcap etc.                                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (20/09/87)      | Modified by : Scott B. Darrow.   |
|  Date Modified : (04/11/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (23/06/95)      | Modified by : Shane Wolstencroft |
|  Date Modified : (30/01/96)      | Modified by : Shane Wolstencroft |
|                                                                     |
|  Comments      :                                                    |
|  (04/11/93)    : Moved code (ie ta [] global declaration) to tcap.c |
|  (23/06/95)    : Added crsr_up, crsr_down, crsr_left, & crsr_right  |
|  (30/01/96)    : Increase size of MAX_TA for win_function3().       |
|                                                                     |
=====================================================================*/
#include	<defkeys.h>

#define	MAX_TA	73

/*
 * Global esc seq array
 */
extern	char	ta [MAX_TA + 1] [81];

#define	_ta	ta

#define	cl_end()		(printf(ta[0]))
#define	cl_line()		(printf(ta[1]))
#define	clear()			(printf(ta[2]))
#define	dl_chr()		(printf(ta[3]))
#define	dl_line()		(printf(ta[4]))
#define	scr_dump()		(printf(ta[5]))
#define	in_chr()		(printf(ta[6]))
#define	so_on()			(printf(ta[8]))
#define	so_off()		(printf(ta[9]))
#define	us_on()			(printf(ta[10]))
#define	us_off()		(printf(ta[11]))
#define	rv_on()			(printf(ta[13]))
#define	rv_off()		(printf(ta[14]))
#define	gr_on()			(printf(ta[16]))
#define	gr_off()		(printf(ta[17]))
#define	fl_on()			(printf(ta[18]))
#define	fl_off()		(printf(ta[19]))
#define	crsr_off()		(printf(ta[23]))
#define	crsr_on()		(printf(ta[24]))
#define	crsr_up()		(printf(ta[26]))
#define	crsr_down()		(printf(ta[27]))
#define	crsr_left()		(printf(ta[28]))
#define	crsr_right()		(printf(ta[29]))

#define	wide()			(((strlen(ta[20]) == 0) ? 0 : 1))

#define	GCHAR(x)		(ta[12][x])
#define	PGCHAR(x)		gr_on (); putchar (ta [12][x]); gr_off ()

#endif	

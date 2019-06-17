/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( Menu.c         )                                 |
|  Program Desc  : ( Main System Menu System.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (30/08/88)      | Modified  by  : Roger Gibbison.  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<signal.h>
#include	<fast.h>

#define	DATA_SIZE	200
#define	WIDTH		78
#define	MAX_OPT		30
#define	MAX_EXTRA	10

#define	MIN_COL		(window) ? 2 : 0
#define	MAX_COL		WIDTH - 2
/*
#define	MIN_ROW		(window) ? 1 : 4
*/
#define	MIN_ROW		(window) ? 3 : 4
#define	MAX_ROW		(window) ? 23 : 19

#define	RUN_HELP	1
#define	EXTERN		2
#define	SHELL		3
#define	OTHER		4

#define	valid_opt(x)	(x >= 1 && x < max_opt)

extern	int	max_box;	/* number of special boxes		*/
extern	int	max_line;	/* number of special lines		*/

#ifdef	CCMAIN
int	c;
int	term_number;		/* current tty number			*/
int	max_opt = 0;		/* number of options on current menu	*/
int	max_quad = 0;		/* number of quadrants			*/
int	max_graph = 0;		/* number of special graphics		*/
int	max_comment = 0;	/* number of special comments		*/
int	is_sub_menu = 0;	/* true if the p/grm running is smenu	*/
int	ps_menu = 0;		/* true if the p/grm running is ps_menu	*/
int	timer_ticked = 0;	/* Set to true if EOF on input was	*/
				/* caused by a timer-tick.		*/
int	timer;
int	timeout;		/* number of inactive minutes to timeout*/
int	status;
int	min_col;
int	min_row;
int	max_col;
int	max_row;
int	row;
int	change_menu = 1;	/* TRUE if user can change menus	*/

char	data[DATA_SIZE + 1];	/* menu data file line			*/
char	*curr_user;		/* current user				*/
char	*security;		/* security for current user		*/
char	*arg[20];		/* arguments to exec			*/

char	*start_menu = "MENUSYS/main.mdf";
#else
extern	int	c;
extern	int	term_number;	/* current tty number			*/
extern	int	max_opt;	/* number of options on current menu	*/
extern	int	max_quad;	/* number of quadrants			*/
extern	int	max_graph;	/* number of special graphics		*/
extern	int	max_comment;	/* number of special comments		*/
extern	int	is_sub_menu;	/* true if the program running is smenu	*/
extern	int	status;
extern	int	timer;
extern	int	timeout;	/* number of inactive minutes to timeout*/
extern	int	min_col;
extern	int	min_row;
extern	int	max_col;
extern	int	max_row;
extern	int	row;
extern	int	change_menu;	/* TRUE if user can change menus	*/

extern	char	data[];		/* menu data file line			*/
extern	char	*curr_user;	/* current user				*/
extern	char	*security;	/* security for current user		*/
extern	char	*arg[];		/* arguments to exec			*/

extern	char	*start_menu;
#endif


FILE	*fmenu;

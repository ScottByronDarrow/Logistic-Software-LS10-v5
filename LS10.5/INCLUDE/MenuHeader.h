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
#define	MIN_ROW		(window) ? 3 : 4
#define	MAX_ROW		(window) ? 23 : 19

#define	RUN_HELP	1
#define	EXTERN		2
#define	SHELL		3
#define	OTHER		4

#define	valid_opt(x)	(x >= 1 && x < maxOptions)


#ifdef	CCMAIN
int	c;
int	terminalNumber;		/* current tty number					*/
int	maxBox;				/* number of special boxes				*/
int	maxLine;			/* number of special lines				*/
int	maxOptions = 0;		/* number of options on current menu	*/
int	maxQuadrants = 0;	/* number of quadrants					*/
int	maxGraphics = 0;	/* number of special graphics			*/
int	maxComment = 0;		/* number of special comments			*/
int	isSubMenu = 0;		/* true if the p/grm running is smenu	*/
int	psMenu = 0;			/* true if the p/grm running is psMenu	*/
int	timer_ticked = 0;	/* Set to true if EOF on input was		*/
						/* caused by a timer-tick.				*/
int	timer;
int	timeout;			/* number of inactive minutes to timeout*/
int	status;
int	minCol;
int	minRow;
int	maxCol;
int	maxRow;
int	row;
int	changeMenu = 1;	/* TRUE if user can change menus	*/

char	data[DATA_SIZE + 1];	/* menu data file line			*/
char	*currentUser;		/* current user				*/
char	*security;		/* security for current user		*/
char	*arg[20];		/* arguments to exec			*/

char	*start_menu = "MENUSYS/main.mdf";
#else
extern	int	c;
extern	int	terminalNumber;	/* current tty number			*/
extern	int	maxOptions;		/* number of options on current menu	*/
extern	int	maxQuadrants;	/* number of quadrants			*/
extern	int	maxGraphics;	/* number of special graphics		*/
extern	int	maxComment;		/* number of special comments		*/
extern	int	isSubMenu;	/* true if the program running is smenu	*/
extern	int	status;
extern	int	timer;
extern	int	timeout;		/* number of inactive minutes to timeout*/
extern	int	minCol;
extern	int	minRow;
extern	int	maxCol;
extern	int	maxRow;
extern	int	maxLine;
extern	int	maxBox;
extern	int	row;
extern	int	changeMenu;	/* TRUE if user can change menus	*/

extern	char	data[];		/* menu data file line			*/
extern	char	*currentUser;	/* current user				*/
extern	char	*security;	/* security for current user		*/
extern	char	*arg[];		/* arguments to exec			*/

extern	char	*start_menu;
#endif


FILE	*fmenu;

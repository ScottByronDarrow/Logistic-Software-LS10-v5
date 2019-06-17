/*=======================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=======================================================================|
| Program Name : ( ring_menu.h )                                        |
| Program Desc : ( Definitions for library module ring_menu.c     )     |
|                (                                                )     |
|-----------------------------------------------------------------------|
| Authors      : Unknown                                                |
| Date Written : ??/??/??                                               |
|-----------------------------------------------------------------------|
| Date Modified : (08/10/90) Modified by : Trevor van Bremen            |
| Date Modified : (11/10/90) Modified by : Trevor van Bremen            |
| Date Modified : (02/07/99) Modified by : Trevor van Bremen            |
|                                                                       |
| Comments      :                                                       |
| (08/10/90)    : Renamed COL to COLM so that there is no conflict      |
|               : between this routine and the G/L library routines.    |
| (11/10/90)    : Allow the use of the space-bar (0x20) to change       |
|               : between menu entries as per RIGHT-KEY                 |
| (02/07/99)    : Fully restructured.  Moved code fragments to library  |
=======================================================================*/
#ifndef	RING_MENU_H
#define	RING_MENU_H

#ifndef	UP_KEY
#include	<defkeys.h>
#endif

extern	int	_wide;

#define	SCN_WIDTH	(_wide ? 132 : 80)

#define	PRMT(i)		curr_menu[i].prompt
#define	DESC(i)		curr_menu[i].description
#define	OPTN(i)		curr_menu[i].fn
#define	MKEYS(i)	curr_menu[i].keystr
#define	MKEY(i)		curr_menu[i].key
#define	FLAG(i)		curr_menu[i].flag
#define	PAGE(i)		curr_menu[i].page
#define	COLM(i)		curr_menu[i].col

#define	SELECT		0001
#define	SHOW		0002
#define	EXIT		0004
#define	VALID		(SELECT | SHOW)
#define	ALL		(SELECT | SHOW | EXIT)
#define	HIDDEN		(SELECT | EXIT)
#define	DISABLED	0010

#define	CHK_FLAG(i,f)	((FLAG(i) & (f)) == (f))

#define	DUP_SEL		100

typedef	struct
{
	char	*prompt;		/* prompt to display		*/
	char	*description;		/* option description		*/
	int	(* fn)(void);		/* option action function	*/
	char	*keystr;		/* string of valid keys for option */
	int	key;			/* other valid key for function	*/
	int	flag;			/* display/select/exit flag	*/
	int	page;			/* page option belongs to	*/
	int	col;			/* col. option prompt prints on	*/
} menu_type;

void	run_menu (menu_type *curr_menu, char *prompt, int row);
void 	_draw_menu (menu_type *curr_menu, char *prompt, int row, int old_page, int new_page, int old_opt, int new_opt);
int _init_menu (menu_type *curr_menu, char *prompt);
int _check_menu (menu_type *curr_menu, int opt, int x);
int _no_option (void);

#endif	/* RING_MENU_H */

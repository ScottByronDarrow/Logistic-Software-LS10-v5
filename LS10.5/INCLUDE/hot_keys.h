#ifndef	HOT_KEYS_H
#define	HOT_KEYS_H
/*
 *	Hot-key management
 *
 *******************************************************************************
 *	$Log: hot_keys.h,v $
 *	Revision 5.0  2001/06/19 06:51:30  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:55  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:39  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.6  1999/11/15 06:47:05  scott
 *	Updated for compile problems on AIX
 *	
 *	Revision 1.5  1999/09/29 00:06:33  jonc
 *	Added HAS_CTRLMACRO definition and usage in hotkeys.h for clean compile
 *	on both AIX and Alpha OSF1.
 *	
 *	Revision 1.4  1999/09/15 05:21:38  scott
 *	Added back of define of CTRL until complete updated and investigation can be performed.
 *	
 *	Revision 1.3  1999/09/14 21:37:11  jonc
 *	Removed CTRL macro. Unrequired as it is a system-defined macro.
 *	
 */

typedef	struct tag_KEY_TAB {
	char *	key_title;
	int		key_val;
	int		(*key_func) (int, struct tag_KEY_TAB *);
	char *	key_help;
	char *	key_tag;
	int		key_stat;
	int		key_row;
	int		key_col;
} * KEY_PTR, KEY_TAB;

typedef int (*SETITEM_CALLBACK) (int);

#define	MAX_HOTKEY_DEPTH	10

#define	KEY_TITLE	key_ptr->key_title
#define	KEY_VAL		key_ptr->key_val
#define	KEY_FUNC(x, y)	(*key_ptr->key_func) (x, y)
#define	KEY_TAG		key_ptr->key_tag
#define	KEY_STAT	key_ptr->key_stat
#define	KEY_HELP	key_ptr->key_help
#define	KEY_ROW		key_ptr->key_row
#define	KEY_COL		key_ptr->key_col
#define	END_KEYS	{ NULL, 0, NULL, NULL, NULL, 0, 0, 0 }

#ifndef	UP_KEY
#define	UP_KEY		2000               /*| Define Up Arrow.     |*/
#define	DOWN_KEY	2001               /*| Define Down Arrow.   |*/
#define	RIGHT_KEY	2002               /*| Define Right Arrow.  |*/
#define	LEFT_KEY	2003               /*| Define Left Arrow.   |*/
#define	FN1			2010
#define	FN3			2012
#define	FN6			2015
#define	FN14		2023
#define	FN15		2024
#define	FN16		2025
#endif

#define	KEY_ACTIVE	0
#define	KEY_PASSIVE	1

#ifndef	TRUE
#	define	TRUE	1
#	define	FALSE	0
#endif

#ifdef	HAS_CTRLMACRO
#include	HAS_CTRLMACRO
#else
#define CTRL(x)		((x) & 037)
#endif

/*=======================================*/
/* Global functions						 */

#if 0
int		set_keys	(KEY_PTR key_tab, char * tag, int stat);
int		init_hotkeys(int type, void * pAddress);
int		run_hotkeys	(KEY_PTR key_tab, int (*pre_func) (int, KEY_PTR),
		 			int (*post_func) (int, KEY_PTR));
void	disp_hotkeys(int key_row, int key_col, int scr_width, KEY_PTR key_tab);
void	set_help	(int key, char * msg);
void	disp_help	(int width);
int		null_func	(char up_c, KEY_PTR key_tab);
#endif

#endif	/*HOT_KEYS_H*/

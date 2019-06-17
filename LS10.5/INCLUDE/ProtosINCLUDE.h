/*
 *
 *******************************************************************************
 *	$Log: ProtosINCLUDE.h,v $
 *	Revision 5.1  2001/08/06 22:49:50  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 06:51:19  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:21  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:51  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:35  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/11/15 06:47:05  scott
 *	Updated for compile problems on AIX
 *	
 *	Revision 1.2  1999/10/08 03:37:32  jonc
 *	Removed prototypes for INFORMIX functions. Now defined in isam.h
 *	
 */

/* account.c */
#include	<account.h>
extern int UserAccountOpen (char *fname, char *mode);
extern int UserAccountAdd (char *moption);

/* ring_menu.c */
#include	<ring_menu.h>
extern void _draw_menu (menu_type *curr_menu, char *prompt, int row, int old_page, int new_page, int old_opt, int new_opt);
extern int _init_menu (menu_type *curr_menu, char *prompt);
extern int _check_menu (menu_type *curr_menu, int opt, int x);
extern int _no_option (void);

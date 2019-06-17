#ifndef IMINIMENU

#ifdef	MINI_MENU
int	MENU_COL	= 0;
int	MENU_ROW	= 0;
#else
extern	int	MENU_COL;
extern	int	MENU_ROW;
#endif

typedef	struct	{
		  char	*menu_opt,
			*menu_cmnt;
		  int	(*menu_func) (void),
			menu_flag;
	}
		*MENUPTR, MENUTAB;

#define	ENDMENU		NULL, NULL, NULL, 0

#define	MENU_OPT	m_ptr->menu_opt
#define	MENU_CMNT	m_ptr->menu_cmnt
#define	MENU_FUNC	m_ptr->menu_func
#define	MENU_FLAG	m_ptr->menu_flag

#define	CURR_OPT	(m_ptr + PV_menu_curr)->menu_opt
#define	CURR_CMNT	(m_ptr + PV_menu_curr)->menu_cmnt
#define	CURR_FUNC	(m_ptr + PV_menu_curr)->menu_func
#define	CURR_FLAG	(m_ptr + PV_menu_curr)->menu_flag

#define	MENU_LOOP	for (; MENU_OPT; m_ptr++)

#ifndef BELL
#define BELL		7
#endif

#define IMINIMENU
#endif

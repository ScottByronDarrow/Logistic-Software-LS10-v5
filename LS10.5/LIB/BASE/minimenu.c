/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : minimenu.c                                     |
|  Source Desc       : Mini menu library routines.                    |
|                                                                     |
|  Library Routines  : mmenu_*().                                     |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 26/04/96   | Modified  by  : Scott B Darrow.   |
|                                                                     |
|  Comments          : Updated to add new functions to mini menu.     |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                                                                     |
=====================================================================*/
#define	MINI_MENU
#include	<std_decs.h>

static	int 	PV_men_width = 0,
		PV_men_depth = 0,
		PV_menu_curr = 0;

static	char	*PV_men_hdr;

static void mmenu_display (MENUPTR m_ptr);
static void menu_msg (MENUPTR m_ptr);

/*
.function
	Function	:	mmenu_print ()

	Description	:	Setup and display a mini-menu.

	Notes		:	Mmenu_print displays a mini-menu using the
				information held in the table ponted to by 
				m_ptr.
			
	Parameters	:	m_hdr       - Menu title.
				m_ptr       - Pointer to menu table.
				curr_opt    - Option at which to position high-
					      light bar.

	Globals		:	PV_menu_curr - Current menu option.
				PV_men_hdr   - Menu title.
				PV_men_depth - Depth of menu (no of menu items)
.end
*/
void
mmenu_print (char *m_hdr, MENUPTR m_ptr, int curr_opt)
{
	PV_men_hdr = m_hdr;

	mmenu_scan (m_ptr);

	mmenu_display (m_ptr);
	PV_menu_curr = (curr_opt >= PV_men_depth || curr_opt < 0) ? 0 :
								    curr_opt;
}

/*
.function
	Function	:	mmenu_scan ()

	Description	:	Routine used to set mini-menu globals.

	Notes		:	Mmenu_scan scans the mini-menu contained in the
				table pointed to by the parameter m_ptr and sets
				the depth and width globals.

	Parameters	:	m_ptr - Pointer to table of MENUTAB structures.

	Globals		:	PV_men_depth - Depth of menu (no of menu items)
				PV_men_width - Width of menu.
.end
*/
void
mmenu_scan (MENUPTR m_ptr)
{
	int	len;

	PV_men_depth = 0;
	if (PV_men_hdr != (char *) 0)
		PV_men_width = strlen (PV_men_hdr) - 1;
	else
		PV_men_width = 0;

	MENU_LOOP
	{
		PV_men_depth++;
		len = (PV_men_width < strlen (MENU_OPT)) ? strlen (MENU_OPT) :
							PV_men_width;
		PV_men_width = len;
	}
	PV_men_width += 2;
}

/*
.loc_func
	Function	:	mmenu_display ()

	Description	:	Routine used to display mini-menus.

	Notes		:	Mmenu_display displays a mini-menu after it has
				been setup by mmenu_scan.

	Parameters	:	m_ptr - Pointer to table of MENUTAB structures.

	Globals		:	PV_men_depth - Depth of menu (no of menu items)
				PV_men_width - Width of menu.
.end
*/
static void
mmenu_display (MENUPTR m_ptr)
{
	int	m_row = MENU_ROW + 1;

	if (PV_men_hdr == (char *) 0)
		box (MENU_COL, MENU_ROW, PV_men_width, PV_men_depth);
	else
	{
		box (MENU_COL, MENU_ROW, PV_men_width, PV_men_depth + 2);
		move (MENU_COL, MENU_ROW + 2);
		PGCHAR (10);
		line (PV_men_width - 1);
		PGCHAR (11);
		rv_pr (PV_men_hdr, MENU_COL + 1, MENU_ROW + 1, 1);
		m_row += 2;
	}

	MENU_LOOP
	{
		move (MENU_COL + 1, m_row++);
		printf ("%s", MENU_OPT);
		fflush (stdout);
	}
}

/*
.function
	Function	:	mmenu_select ()

	Description	:	Routine used to run mini-menus.

	Notes		:	Mmenu_select allows a user to interact with
				the currently active mini-menu.

	Parameters	:	m_ptr - Pointer to table of MENUTAB structures.

	Globals		:	PV_menu_curr - Current menu option.
				PV_men_depth - Depth of menu (no of menu items)
.end
*/
int
mmenu_select (MENUPTR m_ptr)
{
	int	m_char, 
		m_opt;

	crsr_off();

	if (PV_men_hdr == (char *) 0)
		rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 1 + PV_menu_curr, 1);
	else
		rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 3 + PV_menu_curr, 1);
	menu_msg (m_ptr);

	for (;;)
	{
		m_char = getkey();

		if (m_char < 2000 && isalpha(m_char))
			continue;

		if (PV_men_hdr == (char *) 0)
			rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 1 + PV_menu_curr, 0);
		else
			rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 3 + PV_menu_curr, 0);

		switch (m_char)
		{
		case	FN15	 :
		case	UP_KEY	 :
		case	LEFT_KEY :
		case	8	 :
		case	11	 :
			if (!PV_menu_curr)
				PV_menu_curr = PV_men_depth;

			PV_menu_curr--;
			break;

		case	FN13	 :
		case	DOWN_KEY :
		case	FN14	 :
		case	RIGHT_KEY:
		case	' '	 :
		case	12 	 :
		case	10 	 :
			if (PV_menu_curr >= (PV_men_depth - 1))
				PV_menu_curr = 0;
			else
				PV_menu_curr++;
			break;

		case	'\r':
			if (CURR_FLAG)
				(* CURR_FUNC)();
			crsr_on();
			return (PV_menu_curr);
			break;

		case	FN3:
			mmenu_display (m_ptr);
			break;

		case	FN1:
			crsr_on();
			return (-1);

		case	FN16:
			crsr_on();
			return (99);

		default:
			if (m_char > '0' && m_char <= '9' &&
				((m_opt = m_char - '0') <= PV_men_depth))
			{
				PV_menu_curr = m_opt - 1;
			}
			else
				putchar(BELL);
			break;
		}
		if (PV_men_hdr == (char *) 0)
			rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 1 + PV_menu_curr, 1);
		else
			rv_pr (CURR_OPT, MENU_COL + 1, MENU_ROW + 3 + PV_menu_curr, 1);
		menu_msg (m_ptr);
	}
}

/*
.loc_func
	Function	:	menu_msg ()

	Description	:	Display the current menu comment.

	Parameters	:	m_ptr - Pointer to the MENUTAB structurre
					containing the current comment.
.end
*/
static void menu_msg (MENUPTR m_ptr)
{
	if (CURR_CMNT)
	{
		clear_mess ();
		rv_pr (CURR_CMNT, MENU_COL, 23, 1);
	}
	else
		clear_mess ();
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( menu_utils.c   )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (28/12/90)      | Modified by : Campbell Mander    |
|  Date Modified : (13/08/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : The menu table is now stored dynamically in a      |
|                : linked list. menu_utils now handles sub-menus but  |
|                : is also backwardly compatible with anything that   |
|                : calls it that doesn't handle sub-menus.            |
|     (13/08/93) : PSL 9513 use of <malloc.h>                         |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

struct	_MENU_DATA
{
	char	menu_name[15];
	char	is_sub;
	struct	_MENU_DATA *next;
};

#define	_MENU_NULL	((struct _MENU_DATA *) NULL)

static	struct	_MENU_DATA *_menu_head = _MENU_NULL;
static	struct	_MENU_DATA *_menu_curr = _MENU_NULL;

static	struct	_MENU_DATA *_mnu_alloc (void);

void
new_menu(char *mname)
{
	_add_menu (mname, 'N');
}

void
sub_menu(char *mname)
{
	_add_menu (mname, 'Y');
}

int
_add_menu(char *mname, char is_sub)
{
	struct	_MENU_DATA	*lcl_curr = _MENU_NULL;
	struct	_MENU_DATA	*lcl_old = _MENU_NULL;
	char	*tname;
	char	tmp_name[16];
	char	*tmp_str;
	char	*tmp_str1;

	tname = p_strsave (mname);
	if (tname == (char *) 0)
		sys_err ("Error in menu_utils.c During (MALLOC)", errno, PNAME);

	tmp_str = strrchr (tname, '/');
	if (!tmp_str)
		tmp_str = tname;

	tmp_str1 = strrchr (tmp_str, '.');
	if (tmp_str1)
		*tmp_str1 = 0;

	sprintf (tmp_name,"%-14.14s", tmp_str);

	free (tname);

	_menu_curr = _menu_head;

	/*---------------------------------------
	| Return FALSE if menu already exists	|
	| in the linked-list.			|
	---------------------------------------*/
	while (_menu_curr != _MENU_NULL)
	{
		if (!strcmp (tmp_name, _menu_curr->menu_name) && is_sub == _menu_curr->is_sub)
			return(FALSE);
		lcl_old = _menu_curr;
		_menu_curr = _menu_curr->next;
	}
	
	lcl_curr = _mnu_alloc ();

	if (_menu_head == _MENU_NULL)
		_menu_head = lcl_curr;
	else
		lcl_old->next = lcl_curr;

	sprintf(lcl_curr->menu_name,"%-14.14s",tmp_name);
	lcl_curr->is_sub = is_sub;
	lcl_curr->next = _MENU_NULL;
	
	/*---------------------------------------
	| NB: A return of TRUE means the menu	|
	| has been added onto the linked-list.	|
	---------------------------------------*/
	return(TRUE);
}

/*===============================================
| Frees any 'malloc'ed memory associated	|
| with the 'linked-list'.			|
===============================================*/
void
init_mtab (void)
{
	struct	_MENU_DATA	*lcl_old = _MENU_NULL;

	if (_menu_head == _MENU_NULL)
		return;
	else
	{	
		_menu_curr = _menu_head;

		while (_menu_curr != _MENU_NULL)
		{
			lcl_old = _menu_curr;
			_menu_curr = _menu_curr->next;	
			free (lcl_old);
		}
		_menu_head = _MENU_NULL;
	}
}

/*-----------------------------------------------
| Dynamically malloc a _MENU_DATA structure.	|
-----------------------------------------------*/
static	struct	_MENU_DATA *
_mnu_alloc(void)
{
	struct	_MENU_DATA	*lcl_menu;

	lcl_menu = (struct _MENU_DATA *) malloc (sizeof (struct _MENU_DATA));

	if (lcl_menu == _MENU_NULL)
		sys_err ("Error in _add_menu () during (MALLOC)", errno, PNAME);
		
	return (lcl_menu);
}

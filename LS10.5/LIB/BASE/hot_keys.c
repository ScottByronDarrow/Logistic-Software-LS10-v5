/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : hot_keys.c                                     |
|  Source Desc       : Hot key routines.                              |
|                                                                     |
|  Library Routines  : *hot*().                                       |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 26/08/93   | Modified by : Jonathan Chen       |
|                      14.06.94   | Modified by : Jonathan Chen       |
|  Comments          :                                                |
|         (26/08/93) : Putting lib into sccs introduce bug where 1  |
|                    : was being replace with Release Number          |
|         (14.06.94) : moved stub function from <hot_keys.h> to here  |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

static	int	in_tag(char *keytag, char *tag);
static	void	help_func (KEY_PTR key_tab, int key);

static	int	PV_helpkey = '?';
static	char	*PV_helpmsg = "'?'";

/*
.function
	Function	:	disp_hotkeys ()

	Description	:	Display hot-keys.

	Notes		:	Displays currently active hot-keys held in
				table key_tab.
			
	Parameters	:	key_row   - Row at which to display keys.
				key_col   - Column at which to display keys.
				scr_width - Width to be used to calculate key
					    positions.
				key_tab   - Table of KEY_STRUCTS containing key
					    information.
.end
*/
void
disp_hotkeys (int key_row, int key_col, int scr_width, KEY_PTR key_tab)
{
	register KEY_PTR key_ptr;
	int	len = 0,
		crsr_stat = 0,
		start_col = 0;

	crsr_stat = crsr_toggle (FALSE);
	for (key_ptr = key_tab; KEY_VAL; key_ptr++)
		if (KEY_TITLE && KEY_STAT == KEY_ACTIVE)
			len += (strlen (KEY_TITLE) + 1);

	len --;

	start_col = key_col + ((scr_width - len) / 2);

	for (key_ptr = key_tab; KEY_VAL; key_ptr++)
	{
		if (KEY_TITLE && KEY_STAT == KEY_ACTIVE)
		{
			KEY_ROW = key_row;
			KEY_COL = start_col;
			rv_pr (KEY_TITLE, KEY_COL, KEY_ROW, 1);
			start_col += (strlen (KEY_TITLE) + 1);
		}
	}
	fflush (stdout);
	crsr_toggle (crsr_stat);
}

/*
.function
	Function	:	run_hotkeys ()

	Description	:	Run hot-keys.

	Notes		:	Runs currently active hot-keys held in table					key_tab.
			
	Parameters	:	key_tab   - Table of KEY_STRUCTS containing key
					    information.
				pre_func  - Pointer to function to be executed
					    before key is checked for.
				post_func - Pointer to function to be executed
					    after key action has been carried
					    out.

	returns 	:	1	  - If FN1 key is pressed or returned 
					    from key action.
				0	  - If FN16 key is pressed or returned 
					    from key action.

.end
*/
int
run_hotkeys (KEY_PTR key_tab, int (*pre_func) (int, KEY_PTR), int (*post_func) (int, KEY_PTR))
{
	register	KEY_PTR	 key_ptr;
	register	int	c;
	int		help_flag = FALSE;
	int		crsr_stat = 0,
			up_c = 0,
			ret_val = 0;
	
	crsr_stat = crsr_toggle (FALSE);

	while ((c = getkey()))
	{
		if (c == PV_helpkey)
		{
			help_flag = TRUE;
			rv_pr ("Press key for which you need help.", 0, 23, 1);
			continue;
		}
		up_c = (c >= 'a' && c <= 'z') ? c + 'A' - 'a' : c;

		if (help_flag)
		{
			help_func (key_tab, up_c);
			help_flag = FALSE;
			continue;
		}
		if (pre_func)
			(*pre_func) (up_c, key_tab);

		for (key_ptr = key_tab; KEY_VAL; key_ptr++)
			if (up_c == KEY_VAL && KEY_STAT == KEY_ACTIVE)
			{
				if (key_ptr->key_func)
					up_c = KEY_FUNC (up_c, key_tab);

				break;
			}
		if (up_c == FN16)
		{
			ret_val = 0;
			break;
		}

		else if (up_c == FN1)
		{
			ret_val = 1;
			break;
		}

		if (!KEY_VAL)
			putchar ('\007');

		if (post_func)
			(*post_func) (up_c, key_tab);

		clear_mess ();
	}
	
	crsr_toggle (crsr_stat);
	return (ret_val);
}

/*
.function
	Function	:	set_keys ()

	Description	:	Active specified groups of hot-keys.

	Notes		:	Set_keys allows the programmer to
				activate/de-activate keys with the specified
				TAG.
			
	Parameters	:	key_tab - Table of KEY_STRUCTS containing key
					  information.
				tag     - Pointer to string containing tag(s)
					  whose status is to be set.
				stat    - Status to which selected keys are set.

					    	e.g. ACTIVE or PASSIVE.

	returns 	:	set_cnt - Number of keys set by function.
.end
*/
int
set_keys(KEY_PTR key_tab, char *tag, int stat)
{
	register	KEY_PTR key_ptr;
	int		set_cnt = 0;

	for (key_ptr = key_tab; KEY_VAL; key_ptr++)
	{
		if (in_tag(KEY_TAG,tag))
		{
			if (KEY_STAT != stat)
				set_cnt++;
			KEY_STAT = stat;
		}
	}

	return (set_cnt);
}

/*
.loc_func
	Function	:	in_tag ()

	Description	:	Check to see if the key has specified tag.

	Parameters	:	keytag	- Tag(s) of key being checked.
				tag	- Tag(s) being checked for.

	Returns		:	TRUE    - If the key contains the specified
					  tag(s).
				FALSE   - If the key doesn't contain the
					  specified tag(s).
.end
*/
static	int
in_tag(char *keytag, char *tag)
{
	register	char	*g_ptr = keytag;
	register	char	*t_ptr;

	if (keytag == (char *) NULL)
		return (FALSE);

	if (*tag == '*' || *keytag == '*')
		return(TRUE);

	while (*g_ptr)
	{
		t_ptr = strchr(tag,*g_ptr);
		if (t_ptr)
			return(TRUE);
		g_ptr++;
	}

	return (FALSE);
}

/*
.function
	Function	:	help_func ()

	Description	:	Display help message for specified hot-key.

	Notes		:	Help_func 'flashes' the selected key and
				displays a one line help message for about 
				three seconds.
			
	Parameters	:	key_tab - Table of KEY_STRUCTS to search for
					  key.
				key     - Key for which help is to be obtained.
.end
*/
static	void
help_func (KEY_PTR key_tab, int key)
{
	register KEY_PTR key_ptr;
	int	 cnt, crsr_stat;

	crsr_stat = crsr_toggle (FALSE);
	clear_mess ();
	for (key_ptr = key_tab; KEY_VAL; key_ptr++)
		if (key == KEY_VAL && KEY_STAT == KEY_ACTIVE)
		{
			rv_pr (KEY_HELP ? KEY_HELP :
				  "No help set for this key.\007", 0, 23, 1);
			for (cnt = 0; cnt < 4; cnt++)
			{
				if (KEY_TITLE)
					rv_pr (KEY_TITLE, KEY_COL, KEY_ROW, (cnt % 2));
				sleep (2);
			}
			clear_mess ();
			break;
		}

	if (!KEY_VAL)
		putchar ('\007');

	crsr_toggle (crsr_stat);
}

void
set_help (int key, char *msg)
{
	PV_helpkey = key;
	PV_helpmsg = msg;
}

void
disp_help (int width)
{
	char	tmp_str [40];

	sprintf (tmp_str, " HELP KEY - %s ", PV_helpmsg);
	rv_pr (tmp_str, width - strlen (tmp_str) - 1, 0, TRUE);
}

/*
 *	Stub function
 */
int
null_func (int up_c, KEY_PTR key_tab)
{
	return (EXIT_SUCCESS);
}

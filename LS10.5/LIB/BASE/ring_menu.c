/*=======================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=======================================================================|
| Program Name : ( ring_menu.c )                                        |
| Program Desc : ( Well DUH.. A ring menu maybe?                  )     |
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
#include	<std_decs.h>

int	ringClearLine	=	TRUE;

void
run_menu (menu_type *curr_menu, char *prompt, int row)
{
	int	max_opt;
	int	old_opt = -1;
	int	new_opt = -1;
	int	old_page = -1;
	int	new_page = 0;
	int	x;
	int	i;

	/*
	 * initialise menu
	 */
	max_opt = _init_menu (curr_menu, prompt);

	for (i = 0; i <= max_opt; i++)
	{
		if (CHK_FLAG (i, SELECT | SHOW))
		{
			new_opt = i;
			break;
		}
	}
	if (new_opt < 0)
		return;

	while (1)
	{
		/*
		 * display the menu
		 */
		_draw_menu (curr_menu, prompt, row, old_page, new_page, old_opt, new_opt);
		old_page = new_page;
		old_opt = new_opt;
		/*
		 * read key
		 */
		x = getkey ();
		/*
		 * check for "fast access"
		 */
		i = _check_menu (curr_menu, new_opt, x);
		if (i >= 0)
		{
			/*
			 * execute option selected
			 */
			new_opt = i % DUP_SEL;
			new_page = PAGE(i % DUP_SEL);
			if (i >= DUP_SEL)
				continue;
			_draw_menu (curr_menu, prompt, row, old_page, new_page, old_opt, new_opt);
			old_opt = new_opt;
			old_page = new_page;
			x = '\r';
		}

		switch (x)
		{
		case	LEFT_KEY:
		case	8:
			new_opt--;
			while (new_opt >= 0 && !CHK_FLAG (new_opt, VALID))
				new_opt--;

			if (new_opt >= 0 && CHK_FLAG (new_opt, VALID))
				break;

			new_opt = max_opt - 1;
			while (new_opt > old_opt && !CHK_FLAG (new_opt, VALID))
				new_opt--;
			break;

		case	RIGHT_KEY:
		case	12:
		case	' ':
			new_opt++;
			while (new_opt < max_opt && !CHK_FLAG (new_opt, VALID))
				new_opt++;

			if (new_opt < max_opt && CHK_FLAG (new_opt, VALID))
				break;

			new_opt = 0;
			while (new_opt < old_opt && !CHK_FLAG (new_opt, VALID))
				new_opt++;
			break;

		case	'\r':
			(* OPTN (new_opt)) ();
			if (CHK_FLAG (new_opt, EXIT))
				return;
			old_opt = -1;
			old_page = -1;
			break;

		case	127:
			return;

		default:
			putchar (7);
			break;
		}
		new_page = PAGE (new_opt);
	}
}

void
_draw_menu (menu_type *curr_menu, char *prompt, int row, int old_page, int new_page, int old_opt, int new_opt)
{
	int	opt;
	/*
	 * invalid old_option so redraw
	 */
	crsr_off ();
	if (old_page != new_page)
	{
		/*
		 * clear menu lines
		 */
		 if (ringClearLine)
		 {
			move (0, row);
			cl_line ();
			move (0, row + 1);
			cl_line ();
		}
		/*
		 * display menu prompt
		 */
		if (strlen (prompt))
			print_at (row, 1, (prompt));
		/*
		 * display ring menu
		 */
		for (opt = 0; strlen (PRMT (opt)); opt++)
		{
			if (PAGE (opt) == new_page && CHK_FLAG (opt, SHOW))
				rv_pr (PRMT (opt), COLM (opt), row, (opt == new_opt));
		}
		/*
		 * display current description
		 */
		rv_pr (DESC (new_opt), 1, row + 1, 0);
		return;
	}
	/*
	 * clear old current option
	 */
	rv_pr (PRMT (old_opt), COLM (old_opt), row, 0);
	if (ringClearLine)
	{
		move (0, row + 1);
		cl_line ();
	}
	/*
	 * display new current option
	 */
	if (CHK_FLAG (new_opt, SHOW))
		rv_pr (PRMT (new_opt), COLM (new_opt), row, 1);
	rv_pr (DESC (new_opt), 1, row + 1, 0);
}

int
_init_menu (menu_type *curr_menu, char *prompt)
{
	int	i = 0;
	int	offset = 1;
	int	prompt_len;
	int	len = 0;
	int	page = 0;
	/*
	 * initialise offset for prompts
	 */
	prompt_len = strlen (prompt) + 1;
	if (prompt_len)
		prompt_len += 2;
	for (i = 0, offset = prompt_len; strlen (PRMT (i)); i++)
	{
		/*
		 * set flag if not already set
		 */
		if (FLAG (i) == 0)
			FLAG (i) = VALID;
		if (!CHK_FLAG (i, SHOW))
			continue;
		len = strlen (PRMT (i));
		len += 2;
		if (offset + len >= SCN_WIDTH)
		{
			offset = prompt_len;
			page++;
		}
		COLM (i) = offset;
		PAGE (i) = page;
		PRMT (i) = p_strsave (ML (curr_menu [i].prompt));
		DESC (i) = p_strsave (ML (curr_menu [i].description));
		offset += len;
	}
	return (i);
}

int
_check_menu (menu_type *curr_menu, int opt, int x)
{
	int	i;
	int	new_opt = -1;
	char	*sptr;
	/*
	 * check options to right
	 */
	for (i = opt + 1; strlen (PRMT (i)); i++)
	{
		if (!CHK_FLAG (i, SELECT))
			continue;
		/*
		 * match found
		 */
		if (strlen (MKEYS (i)))
			sptr = strchr (MKEYS (i), x);
		else
			sptr = (char *) 0;
		if (sptr != (char *) 0 || x == MKEY (i))
		{
			if (new_opt == -1)
				new_opt = i;
			else
				return (new_opt + DUP_SEL);
		}
	}
	/*
	 * check options to left
	 */
	for (i = 0; i <= opt; i++)
	{
		if (!CHK_FLAG (i, SELECT))
			continue;
		/*
		 * match found
		 */
		if (strlen (MKEYS (i)))
			sptr = strchr (MKEYS (i), x);
		else
			sptr = (char *) 0;
		if (sptr != (char *) 0 || x == MKEY (i))
		{
			if (new_opt == -1)
				new_opt = i;
			else
				return (new_opt + DUP_SEL);
		}
	}
	return (new_opt);
}

int
_no_option (void)
{
	return (EXIT_SUCCESS);
}

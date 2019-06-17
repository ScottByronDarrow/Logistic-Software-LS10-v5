/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : tag_utils.c                                    |
|  Source Desc       : Tag utilities related to general ledger.       |
|                                                                     |
|  Library Routines  :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     :   /  /     | Modified  by  :                   |
|                                                                     |
|  Comments          :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

static	void	_tag_toggle (char *t_name);

static	char	PV_tag_line [256];

/*
.function
	Function	:	tag_toggle ()

	Description	:	Tag/Untag CURRENT line in table.

	Notes		:	Tag_toggle tags an un-tagged line or un-tags a
				tagged line.
			
	Parameters	:	t_name	    - Pointer to string containing table
					      name.

	Globals		:	PV_tag_line - Table line buffer.

	Returns		:	TRUE
.end
*/
int
tag_toggle (char *t_name)
{
	_tag_toggle (t_name);
	if (tab_get (t_name, (char *) NULL, NEXT, 0))
		tab_get (t_name, (char *) NULL, FIRST, 0);

	return ( 1 );
}


/*
.loc_func
	Function	:	_tag_toggle ()

	Description	:	Tag/Untag CURRENT line in table.

	Notes		:	_tag_toggle tags an un-tagged line or un-tags a
				tagged line.
			
	Parameters	:	t_name	    - Pointer to string containing table
					      name.

	Globals		:	PV_tag_line - Table line buffer.
.end
*/

static	void
_tag_toggle (char *t_name)
{
	tab_get (t_name, PV_tag_line, CURRENT, 0);
	PV_tag_line [0] = (PV_tag_line [0] == '*') ? ' ' : '*';
	tab_update (t_name, "%s", PV_tag_line);
	redraw_line (t_name, 0);
}

/*
.function
	Function	:	tag_all ()

	Description	:	Tags / Un-tags all remaining lines in table.

	Notes		:	Tag_all toggles the tag on all lines in the
				table including and following the current line.

	Parameters	:	t_name	    - Pointer to string containing table
					      name.

	Globals		:	PV_tag_line - Table line buffer.

	Returns		:	ret_val	    - TRUE if successful.
					    - FALSE if failed.	
.end
*/
int
tag_all (char *t_name)
{
	int	cc, ret_val =  1 , tmp_line;

	tmp_line = tab_tline (t_name);
	cc = tab_get (t_name, PV_tag_line, CURRENT, 0);

	while (!cc)
	{
		_tag_toggle (t_name);
		cc = tab_get (t_name, PV_tag_line, NEXT, 0);
	}
	tab_get (t_name, PV_tag_line, EQUAL, tmp_line);
	load_page (t_name, 0);
	
	return (ret_val);
}

/*
.function
	Function	:	tag_set ()

	Description	:	Tag CURRENT line in table.

	Notes		:	Tag_set tags the current line in the table.
			
	Parameters	:	t_name	    - Pointer to string containing table
					      name.

	Globals		:	PV_tag_line - Table line buffer.

	Returns		:	TRUE
.end
*/
int
tag_set (char *t_name)
{
	tab_get (t_name, PV_tag_line, CURRENT, 0);

	PV_tag_line [0] = '*';
	tab_update (t_name, "%s", PV_tag_line);
	
	return ( 1 );
}

/*
.function
	Function	:	tag_unset ()

	Description	:	Un-tag CURRENT line in table.

	Notes		:	Tag_unset un-tags the current line in the table.
			
	Parameters	:	t_name	    - Pointer to string containing table
					      name.

	Globals		:	PV_tag_line - Table line buffer.

	Returns		:	TRUE
.end
*/
int
tag_unset (char *t_name)
{
	tab_get (t_name, PV_tag_line, CURRENT, 0);

	PV_tag_line [0] = ' ';
	tab_update (t_name, "%s", PV_tag_line);
	
	return ( 1 );
}

/*
.function
	Function	:	tagged ()

	Description	:	Returns TRUE if string is tagged.

	Parameters	:	s     - String to be tested.

	Returns		:	TRUE  - if string is tagged.
				FALSE - if string is not tagged.
.end
*/
int
tagged (char *s)
{
	return (*s == '*' ? 1 : 0);
}

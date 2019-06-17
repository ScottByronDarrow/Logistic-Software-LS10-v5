/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : centre_at.c                                    |
|  Source Desc       : Print centred on specified row.                |
|                                                                     |
|  Library Routines  : centre_at()                                    |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 02/07/99   | Modified  by  : Trevor van Bremen |
|                                                                     |
|  Comments          :                                                |
|  (02/07/99)        : Changed from the old-fashioned varargs to use  |
|                    : the more supported stdarg                      |
|                    :                                                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
.function
	Function	:	centre_at ()

	Description	:	Print centred on specified row.

	Notes		:	Centre_at uses the mask (see printf) to format
				the arguments passed and display centred on the
				row specified.
			
	Parameters	:	row   - Row at which to print.
				width - Width in which to centre.
				mask  - Format mask.
				ARGS  - Arguments to format and display.
.end
*/
void
centre_at (int row, int width, char *mask, ...)
{
	va_list	args;
	int	col,
		rev_flag;
	char	tmp_str [256];

	va_start (args, mask);

	rev_flag = (!strncmp (mask, "%R", 2)) ? TRUE : FALSE;

	vsprintf (tmp_str, rev_flag ? mask + 2 : mask, args);
	col = (width - strlen (tmp_str)) / 2;
	rv_pr (tmp_str, col, row, rev_flag);

	va_end (args);

	fflush (stdout);
}

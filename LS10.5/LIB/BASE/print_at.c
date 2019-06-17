/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : print_at.c                                     |
|  Source Desc       : Print at specified row and column.             |
|                                                                     |
|  Library Routines  : print_at()                                     |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 02/07/99   | Modified  by  : Trevor van Bremen |
|                                                                     |
|  Comments          :                                                |
|  (02/07/99)        : Updated to use stdarg.h over varargs.h         |
|                    :                                                |
|                    :                                                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
.function
	Function	:	print_at ()

	Description	:	Print at specified row and column.

	Notes		:	Print_at uses the mask (see printf) to format
				the arguments passed and display the at the
				row and column given.
			
	Parameters	:	row   - Row at which to print.
				col   - Column at which to print.
				mask  - Format mask.
				ARGS  - Arguments to format and display.
.end
*/

extern	int	ntimes;

void
print_at (int row, int col, char *mask, ...)
{
	va_list	args;
	int	rev_flag;
	char	tmp_str [256];


	va_start (args, mask);

	if (!ntimes)
		init_scr();

	rev_flag = (!strncmp (mask, "%R", 2)) ? TRUE : FALSE;

	vsprintf (tmp_str, rev_flag ? mask + 2 : mask, args);
	rv_pr (tmp_str, col, row, rev_flag);

	va_end (args);

	fflush (stdout);
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : line_at.c                                      |
|  Source Desc       : Display line at specified row annd column.     |
|                                                                     |
|  Library Routines  : line_at()                                      |
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

/*
.function
	Function	:	line_at ()

	Description	:	Display line at specified row annd column.

	Parameters	:	row - Row at which to display line.
				col - Column at which to display line.
				len - Length of line.
.end
*/
void
line_at (int row, int col, int len)
{
	move (col, row);  line (len);
	fflush (stdout);
}

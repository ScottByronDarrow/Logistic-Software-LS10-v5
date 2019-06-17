/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : ( blank_at.c     )                             |
|  Source Desc       : ( Blank part of the screen.                  ) |
|                                                                     |
|  Library Routines :  blank_at().                                    |
|                   :                                                 |
|                   :                                                 |
|                   :                                                 |
|                   :                                                 |
|                   :                                                 |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
.function
	Function	:	blank_at ()

	Description	:	Blank part of the screen.

	Notes		:	Prints a specified number of spaces at the
				row, col desired.

	Parameters	:	row - Row on screen at which to start.
			 	col - Column on screen at which to start.
.end
*/
void
blank_at (int row, int col, int len)
{
	int	crsr_stat;

	crsr_stat = crsr_toggle ( 1 );

	move (col, row);

	while (len--)
		putchar (' ');

	crsr_toggle (crsr_stat);
	fflush (stdout);
}

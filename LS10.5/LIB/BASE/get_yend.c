/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : get_yend.c                                     |
|  Source Desc       : Returns year end date.                         |
|                                                                     |
|  Library Routines  : get_yend()                                     |
|---------------------------------------------------------------------|
|  Date Modified : 17.06.94 | Modified by : Jonathan Chen             |
|                                                                     |
|  Comments      :                                                    |
|       17.06.94 : Removed redundant header files                     |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
/*
.function
	Function	:	get_yend ()

	Description	:	Returns year end date.

	Notes		:	This function returns the year end date in
				calendar format of either the current or next
				financial year following the calendar date
				passed to it.
			
	Parameters	:	c_date	- Calendar date from which to calculate
					  the year end.

				nxt_flg	- TRUE if end of next year is desired.
.end
*/

long	get_yend (long int c_date, int nxt_flg)
{
	short	fdmy [3];
	long	tmp_long;

	check_fiscal ();

	get_fdmy (fdmy, c_date);

/*
	Set to beginning of first period in the following year then subtract 1
	to get end of last period.
*/
	fdmy [0] = 1;
	fdmy [1] = 1;
	fdmy [2] += (1 + nxt_flg);

	get_cdate (&tmp_long, fdmy);

	return (tmp_long - 1);
}

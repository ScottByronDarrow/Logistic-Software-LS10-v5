/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : get_ybeg.c                                     |
|  Source Desc       : Returns first date of financial year.          |
|                                                                     |
|  Library Routines  : get_ybeg()                                     |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
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

extern	int	GV_fiscal;

/*
.function
	Function	:	get_ybeg ()

	Description	:	Returns first date of financial year.

	Notes		:	This function returns the start date in
				calendar format of the financial year of the
				date passed to it.
			
	Parameters	:	c_date	- Calendar date from which to calculate
					  the financial year start.

	Globals		:	GV_fiscal - Fiscal year end.

	Returns		:	c_date	  - Start date of financial year.
.end
*/

long	get_ybeg (long c_date)
{
	return (FinYearStart (c_date, GV_fiscal));
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : pin_time.c                                     |
|  Source Desc       : Time functions.                                |
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
#include <time.h>
#include <stdio.h>

static	char	mon_works [] = "0111110011111001111100111110011111001111100";

static	int	PV_end_off, PV_start_off;

/*
.function
	Function	:	set_wdays ()

	Description	:	Setup variables needed by working day routines.

	Notes		:	Set_wdays sets the start and end offsets into 
				the string which contains the map of a calendar
				month.
			
	Parameters	:	dom	     - Day of month (1 - 31).
				dow	     - Day of week.
					       (0 [Sunday] - 6 [Saturday]).

	Globals		:	PV_end_off   - End offset in map.
				PV_start_off - Start offset in map.
.end
*/
void
set_wdays (int dom, int dow)
{
	PV_end_off = strlen (mon_works) - 8 + dow;
	PV_start_off = PV_end_off - dom + 1;
}

/*
.function
	Function	:	month_wdays ()

	Description	:	Return the number of week-days in a month.

	Notes		:	This function calculates the number of week-
				days (mon - fri) in a month.
			
	Parameters	:	dom	     - Day of month (1 - 31).
				dow	     - Day of week.
					       (0 [Sunday] - 6 [Saturday]).

	Globals		:	PV_end_off   - End offset in map.
				PV_start_off - Start offset in map.

	Returns		:	week_days    - Number of week-days in month.
.end
*/
int
month_wdays (int dom, int dow)
{
	register int	cnt, week_days = 0;
	
	for (cnt = PV_start_off; cnt <= PV_end_off; cnt++)
		if (mon_works[cnt] == '1')
			week_days++;

	return (week_days);
}

/*
.function
	Function	:	is_weekday ()

	Description	:	Checks specified day is a week-day.

	Parameters	:	dom 	     - Day of month (1 - 31).

	Globals		:	PV_start_off - Start offset in map.

	Returns		:	TRUE	     - If day is a week-day.
				FALSE	     - If day is in week-end.
.end
*/
int
is_weekday (int dom)
{
	return ((mon_works [PV_start_off + dom - 1] == '1') ? 1 : 0);
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : fisc_year.c                                    |
|  Source Desc       : Return financial year.                         |
|                                                                     |
|  Library Routines  : fisc_year()                                    |
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
	Function	:	fisc_year ()

	Description	:	Return financial year.

	Notes		:	This function calculates the financial year in
				which the date passed to it falls.
			
	Parameters	:	c_date	- Normal calendar date for which to 
					  calculate the financial year.

	Returns		:		- Financial year in format YYYY.
.end
*/

extern int GV_fiscal;
int
fisc_year (long int c_date)
{
	int		year;

	check_fiscal ();

	DateToFinDMY (c_date, GV_fiscal, NULL, NULL, &year);
	
	return (year);
}

//====================================================================
// Copyright (C) 1998 - 2001 LogisticSoftware                        
//====================================================================
//	Function	:	get_cdate () $Id: get_cdate.c,v 5.0 2001/06/19 06:59:17 cha Exp $
//
//	Description	:	Convert a financial date to a calendar date.
//
//	Notes		:	This function converts a financial date,
//				which has been split into three pieces, into a
//				calendar date in INFORMIX date format.
//
//	Parameters	:	c_date	- Pointer to the long variable to hold
//					  the converted date.
//				fdmy	- Pointer to an array of three shorts
//					  holding the financial day, month, year
//					  to be converted.
//
//	Globals		:	GV_fiscal - The month in which the financial
//					    year ends.
//  $Log: get_cdate.c,v $
//  Revision 5.0  2001/06/19 06:59:17  cha
//  LS10-5.0 New Release as of 19 JUNE 2001
//
//  Revision 4.1  2001/03/22 01:19:44  scott
//  Updated to be as same as possible with LS10-GUI for Ease of Maintenance.
//
//====================================================================
#include	<stdio.h>
#include	<std_decs.h>

extern	int	GV_fiscal;


void
get_cdate (
	Date *c_date,
	short	fdmy [3])
{
	int	m, y;

	check_fiscal ();

	y = (fdmy [1] <= (12 - GV_fiscal)) ? fdmy [2] - 1 : fdmy [2];
	m = (fdmy [1] <= (12 - GV_fiscal)) ? fdmy [1] + GV_fiscal :
						  fdmy [1] - (12 - GV_fiscal);

	/*
	 *	Correct for year if necessary
	 */
	if (y < 100)
	{
		if (y < 51)
			y += 2000;
		else
			y += 1900;
	}

	*c_date = DMYToDate (1, m, y);
}

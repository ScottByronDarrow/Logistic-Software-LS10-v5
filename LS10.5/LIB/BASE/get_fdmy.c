//====================================================================
// Copyright (C) 1999 - 1999 LogisticSoftware                         
//====================================================================
// Source Name       : get_fdmy.c  $Id: get_fdmy.c,v 5.0 2001/06/19 06:59:18 cha Exp $
// Source Desc       : Convert a calendar date to a financial date.  
//--------------------------------------------------------------------
// Author            : Scott Darrow.   | Date Written  : 21/01/91     
//--------------------------------------------------------------------
//	Function	:	get_fdmy ()
//
//	Description	:	Convert a calendar date to a financial date.
//
//	Notes		:	This function converts a calendar date, in
//				INFORMIX date format, into a finacial date
//				split into three pieces.
//			
//	Parameters	:	fdmy	- Pointer to an array of three shorts
//					  to hold the financial day, month, year
//					  after the conversion.
//
//	Parameters	:	currentDate	- Long variable holding the calendar
//					  date to be converted.
//
//	Globals		:	GV_fiscal - The month in which the financial
//					    year ends.
//
//  $Log: get_fdmy.c,v $
//  Revision 5.0  2001/06/19 06:59:18  cha
//  LS10-5.0 New Release as of 19 JUNE 2001
//
//  Revision 4.1  2001/03/22 01:19:44  scott
//  Updated to be as same as possible with LS10-GUI for Ease of Maintenance.
//
//
//====================================================================
#include	<std_decs.h>

extern	int	GV_fiscal;

void
get_fdmy (
	short 	fdmy [3], 
	long 	currentDate)
{
	int	dmy [3];

	check_fiscal ();

	DateToDMY (currentDate, &dmy[0], &dmy[1],&dmy[2]);

	fdmy [0] = 1;	/*	Day not needed	*/
	fdmy [2] = (dmy[1] <= GV_fiscal) 	? dmy [2] : dmy [2] + 1;
	fdmy [1] = (dmy [1] <= GV_fiscal) 	? (12 - GV_fiscal) + dmy [1] 
									  	: dmy [1] - GV_fiscal;
}

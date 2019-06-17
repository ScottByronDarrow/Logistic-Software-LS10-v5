/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Source Name       : get_eoy.c                                      |
|  Source Desc       : Calculate year and fiscal Date.                |
|---------------------------------------------------------------------|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|---------------------------------------------------------------------|
| $Log: get_eoy.c,v $
| Revision 5.0  2001/06/19 06:59:18  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/22 01:19:44  scott
| Updated to be as same as possible with LS10-GUI for Ease of Maintenance.
|
=====================================================================*/
#include	<std_decs.h>

/*=================================
| Calculate year and fiscal Date. |
=================================*/
long	
get_eoy (
	long	currentDate, 
	int 	fiscalMonth)
{
	return (FinYearEnd (currentDate, fiscalMonth));
}

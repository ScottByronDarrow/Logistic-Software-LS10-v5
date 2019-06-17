/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( per_val.c      )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
|  Date Modified : (19/02/92)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      :                                                    |
|  (19/02/92)    : Conversions as required to Internationalize        |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*====================================================================
| Validate current module date with G/L date to see if valid period. |
====================================================================*/
int
val_period(char *per_num, long int gl_date)
{
	int	month;
	int	val_mon[2];

	DateToDMY (gl_date, NULL, &val_mon[0], &val_mon[1]);
	val_mon[1]++;
	if (val_mon[1] > 12)
		val_mon[1] = 1;

	month = atoi (per_num);

	if (month == val_mon[0] || month == val_mon[1])
		return (EXIT_SUCCESS);
	else	
		return (EXIT_FAILURE);
}

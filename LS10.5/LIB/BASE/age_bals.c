/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( age_bal.c      )                                 |
|  Program Desc  : ( Calculated ageing period.                      ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
|  Date Modified : (19/02/92)      | Modified  by : Scott Darrow      |
|                                                                     |
|  Comments                                                           |
|  (19/02/92)    : Updated to internationalize (ie: Lookup DBDATE)    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

int
age_bals (
 int	p_terms,
 Date	inv_date,
 Date	cur_date)
{
	int		period;
	Date	eom;
	long	delta;

	switch (p_terms)
	{
	case 0:
		return 0;

	case 30:
		/*
		 *	Assume this means 1 month terms
		 *
		 *	Compare the invoice against the last day of the
		 *	past 3 months
		 */
		eom = MonthEnd (cur_date);

		if (inv_date > eom)
			return 4;			/* forward dated */

		eom = AddMonths (eom, -1);

		for (period = 0; period <= 3; period++)
		{
			if (inv_date > eom)
				break;
			eom = AddMonths (eom, -1);
		}
		if (period > 3)
			period = 3;
		break;

	default:
		delta = cur_date - inv_date - 1L;
		period = delta < p_terms ? 0 : delta / p_terms;
		if (period > 3)
			period = 3;
	}
	return period;
}

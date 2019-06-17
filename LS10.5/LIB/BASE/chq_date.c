/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( chq_date.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (19/02/92)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments                                                           |
|  (19/02/92)    : Updated to internationalize (ie: Lookup DBDATE)    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

void
_cl_space(void)
{
	move (1, 2);
	printf ("%-60.60s", " ");
}

int
chq_date ( long	int	inp_date, long	int	mod_date)
{
	int		i_m, i_y,
			m_m, m_y,
			n_m, n_y;
	int		i;

	DateToDMY (inp_date, NULL, &i_m, &i_y);
	DateToDMY (mod_date, NULL, &m_m, &m_y);
	DateToDMY (AddMonths (mod_date, 1), NULL, &n_m, &n_y);

	if (i_m != m_m && i_m != n_m)
	{
		_cl_space ();
		i = prmptmsg (ML("Input Month is not the same as module month, Accept ? "), "YyNn", 1, 2);
		_cl_space ();
		if (i == 'Y' || i == 'y')
			return (EXIT_SUCCESS);
		return (EXIT_FAILURE);
	}

	if (i_y != m_y && i_y != n_y)
	{
		_cl_space ();
		i = prmptmsg (ML("Input Year is not the same as module Year, Accept ? "), "YyNn", 1, 2);
		_cl_space ();
		if (i == 'Y' || i == 'y')
			return (EXIT_SUCCESS);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

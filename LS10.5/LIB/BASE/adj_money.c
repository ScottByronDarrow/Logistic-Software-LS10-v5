/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : ( adj_money.c    )                             |
|  Source Desc       : ( Adjust money to nearest multiple of 5 cents.)|
|                      (                                             )|
|  Library Routines : adj_money()                                     |
|                   : adj_val()                                       |
|                   :                                                 |
|                   :                                                 |
|                   :                                                 |
|                   :                                                 |
|---------------------------------------------------------------------|
|  Date Modified : (27/10/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|     (27/10/93) : if getenv ("ADJ_MONEY") fails, atoi() crashes on   |
|                : some platforms is supplied with NULL argument      |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
	Function declarations
*/
extern char	*getenv (const char *);		/*stdlib*/

/*
.function
	Function	:	adj_money ()

	Description	:	Adjust money to nearest multiple of five cents.

	Notes		:	Adjusts as follows :
						0, 1, 2		- 0
						3, 4, 5, 6, 7	- 5
						8, 9		- 10
			
	Parameters	:	m_val	- Value to be adjusted.

	Returns		:	m_val	- Adjusted value.
.end
*/

double	adj_money (double m_val)
{
	double	adj_val (double m_val);

	return (m_val + adj_val (m_val));
}

/*
.function
	Function	:	adj_val ()

	Description	:	Calculate money adjustment factor.f five cents.

	Notes		:	Calculates factor to adjust as follows.
						0, 1, 2		- to 0
						3, 4, 5, 6, 7	- to 5
						8, 9		- to 10
			
	Parameters	:	m_val	  - Value to be adjusted.

	Globals		:	ADJ_MONEY - Environment variable.

					    If set to : 1 - Adjust.
							0 - Don't adjust.

	Returns		:	adj_val   - Value to add to adjust to legal
					    tender.
.end
*/
double	adj_val (double m_val)
{
	char		tmp_str [24];
	int			_adj_val;
	static		int	adjust = -1;

	if (adjust == -1)
	{
		char	*adj = getenv ("ADJ_MONEY");

		adjust = adj ? atoi (adj) : 0;
	}

	if (!adjust)
		return (0.00);

	sprintf (tmp_str, "%20.2f", m_val);

	_adj_val = (tmp_str [19] - '0') % 5;

	if (_adj_val > 2)
		return ((5 - _adj_val) * .01);
	else
		return (_adj_val * -.01);
}

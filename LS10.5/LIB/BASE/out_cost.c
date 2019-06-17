/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( out_cost.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
/*=======================================
| Calculate the unit cost for items	|
| if the outer_size <= 0 then assume	|
| costing is per unit.			|
=======================================*/
double
out_cost (
 double	cost,
 float	outer_size)
{
	double	value;
	if (outer_size <= 0.00)
		return(cost);

	value = cost;
	value /= (double) outer_size;
	return(value);
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( rnd_mltpl.h    )                                 |
|  Program Desc  : ( Round a float qty to the nearest multiple.   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 30/03/92         |
|---------------------------------------------------------------------|
|  Date Modified : (02.08.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : (  /  /  ) -                                       |
|  (02.08.94) : Moved from include file to library                    |
|                                                                     |
|                                                                     |
=====================================================================*/
#include	<math.h>

/*
	Parameters : rnd_type  - The type of rounding : 
				    U - Round Up To The Nearest Multiple.
				    D - Round Down To The Nearest Multiple.
				    B - Best Fit Rounding. ie. Round up if the
					qty is nearest to the higher multiple 
					and vice versa.

		     ord_mltpl - The rounded quantity must be a multiple of this
				 number.

		     ord_qty   - The value to round.

	Returns	   : A rounded order quantity.

	Notes	   : N/A.

*/

float
rnd_mltpl (
 char	*rnd_type,
 float	ord_mltpl,
 float	ord_qty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (ord_qty == 0.00)
		return(0.00);

	if (ord_mltpl == 0.00)
		return(ord_qty);

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (ord_qty / ord_mltpl);
	if (ceil (wrk_qty) == wrk_qty)
		return(ord_qty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (rnd_type[0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double)(ord_qty / ord_mltpl);
		wrk_qty = ceil(wrk_qty);
		ord_qty = (float)(wrk_qty * ord_mltpl);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double)(ord_qty / ord_mltpl);
		wrk_qty = floor(wrk_qty);
		ord_qty = (float)(wrk_qty * ord_mltpl);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double)ord_qty;
		wrk_qty = (up_qty / (double)ord_mltpl);
		wrk_qty = ceil(wrk_qty);
		up_qty = (float)(wrk_qty * ord_mltpl);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double)ord_qty;
		wrk_qty = (down_qty / (double)ord_mltpl);
		wrk_qty = floor(wrk_qty);
		down_qty = (float)(wrk_qty * ord_mltpl);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double)ord_qty) <= ((double)ord_qty - down_qty))
			ord_qty = (float)up_qty;
		else
			ord_qty = (float)down_qty;

		break;

	default:
		return (ord_qty);
	}

	return(ord_qty);
}

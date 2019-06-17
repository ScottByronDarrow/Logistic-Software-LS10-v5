/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| $Id: psl_round.c,v 5.2 2001/08/20 23:05:54 scott Exp $
|  Header Name   : (psl_round.c)
|  Header Desc   : (To enable rounding of double(s) to the user)
|                  (specified precision)
|---------------------------------------------------------------------|
|  Date Written  : (  .  .  )      | Author       : Unknown           |
|---------------------------------------------------------------------|
| $Log: psl_round.c,v $
| Revision 5.2  2001/08/20 23:05:54  scott
| Updated from testing.
|
| Revision 5.1  2001/07/25 00:43:29  scott
| New library for 10.5
|
=====================================================================*/
#include	<stdio.h>
#include	<math.h>
#include	<twodec.h>
#include	<decimal.h>

double  
dRound (
	double	dValue, 
	int 	iNoDecimals)
{
	double  dLow	 	=	0.00,
			dScale	 	=	0.00,
			dRetValue 	=	0.00;

	if (iNoDecimals < 0 || iNoDecimals > 9)
		return (dValue);

	dScale = pow ((double) 10.00, (double) iNoDecimals);
	dValue *= dScale;
	dLow    = floor (dValue);
	dValue  = (fabs (dValue - dLow) >= 0.5) ? dLow + 1: dLow;
	if (dValue == -0)
		dValue = 0;

	dRetValue = dValue / dScale;
 	return (dRetValue);
}
double	
psl_round (
	double	dValue, 
	int 	iNoDecimals)
{
	return (dRound (dValue, iNoDecimals));
}

double	round (double d)
{
	return (dRound (d, 0));
}

double	fourdec (double d)
{
	return (dRound (d, 4));
}

double	threedec (double d)
{
	return (dRound (d, 3));
}

double	twodec (double d)
{
	return (dRound (d, 2));
}

double	onedec (double d)
{
	return (dRound (d, 1));
}

double	no_dec (double d)
{
	return (dRound (d, 0));
}
double	tw_no_dec (double d)
{
	double	TwRound;
	TwRound = dRound (d/100,0);
	return (TwRound * 100);
}

double	n_dec (double d, int n)
{
	return (dRound (d, n));
}

/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: SupPrice.h,v 5.0 2001/06/19 06:51:25 cha Exp $
|  Program Desc  : (Supplier Pricing and Discouting Routines.   )     |
|---------------------------------------------------------------------|
|  Access files  :  insp, suds,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Date Written  : (15/10/93)      |  Author     : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : ( /  / )      | Modified by :                      |
|                                                                     |
|  Comments      :                                                    |
|  ( /  / )    :                                                      |
|                :                                                    |
|                                                                     |
=====================================================================*/

double	CalcNet 	 (double, float [4], int);
double	GetSupPrice (long, long, double, float);
int 	GetSupDisc 	 (long, char *, float, float [4]);

float	*insp_qty_brk	=	&insp_rec.qty_brk1;
double	*insp_price		=	&insp_rec.price1;
float	*suds_qty_brk	=	&suds_rec.qty_brk1;
float	*suds_disca_pc  =	&suds_rec.disca_pc1;
float	*suds_discb_pc  =	&suds_rec.discb_pc1;
float	*suds_discc_pc  =	&suds_rec.discc_pc1;

/*-----------------------------------------------
| GetSupPrice calculates the supplier price for |
| the specified item based on the specified     |
| quantity.                                     |
|                                               |
| RETURNS : Supplier cost price for item.       |
-----------------------------------------------*/
double 
GetSupPrice (
	long	hhsuHash, 
	long 	hhbrHash, 
	double 	cstPrice, 
	float 	qty)
{
	int		i;

	/*-----------------------
	| Read the insp record. |
	-----------------------*/
	insp_rec.hhsu_hash = hhsuHash;
	insp_rec.hhbr_hash = hhbrHash;
	cc = find_rec (insp, &insp_rec, COMPARISON, "r");
	if (cc)
		return (cstPrice);

	/*------------------------------------------------------
	| Calculate the price based on the specified quantity. |
	------------------------------------------------------*/
	if (insp_qty_brk [0] == 0.00 || qty < insp_qty_brk [0])
		return (cstPrice);
	
	for (i = 0; i < 5; i++)
	{
		/*----------------------
		| Last quantity break. |
		----------------------*/
		if (qty >= insp_qty_brk [i] && i == 4)
			return (insp_price [i]);

		/*-------------------------------
		| Last non-zero quantity break. |
		-------------------------------*/
		if (qty >= insp_qty_brk [i] && insp_qty_brk [i + 1] == 0.00)
			return (insp_price [i]);

		/*------------------------------------------------
		| Greater than current break and less than next. |
		------------------------------------------------*/
		if (qty >= insp_qty_brk [i] && qty < insp_qty_brk [i + 1])
			return (insp_price [i]);
	}

	return (cstPrice);
}

/*------------------------------------------------------
| GetSupDisc calculates all supplier discounts for the |
| specified item based on the specified quantity.      |
|                                                      |
| The array of floats discArray contains the           |
| calculated discounts :                               |
|   discArray [0] = Regulatory %                       |
|   discArray [1] = Discount A %                       |
|   discArray [2] = Discount B %                       |
|   discArray [3] = Discount C %                       |
|                                                      |
| RETURNS : TRUE  - Discounts are cumulative.          |
|         : FALSE - Discounts are absolute.            |
| NB. Discounts are absolute by default. ie if no      |
| suds record exists then discounts are absolute.      |
------------------------------------------------------*/
int 
GetSupDisc (
	long	hhsuHash, 
	char	*buyGroup, 
	float	qty, 
	float	discArray [4])
{
	int		i;

	/*-------------------
	| Initialise Array. |
	-------------------*/
	for (i = 0; i < 4; i++)
		discArray [i] = 0.00;

	/*-------------------
	| Find suds record. |
	-------------------*/
	suds_rec.hhsu_hash = hhsuHash;
	sprintf (suds_rec.buy_group, "%-6.6s", buyGroup);
	cc = find_rec (suds, &suds_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	/*---------------
	| Regulatory %. |
	---------------*/
	discArray [0] = suds_rec.reg_pc;

	/*------------------------------------------------
	| Get discounts based on the specified quantity. |
	------------------------------------------------*/
	if (suds_qty_brk [0] == 0.00 || qty < suds_qty_brk [0])
		return ((suds_rec.cumulative [0] == 'Y') ? TRUE : FALSE);

	for (i = 0; i < 6; i++)
	{
		/*----------------------
		| Last quantity break. |
		----------------------*/
		if (qty >= suds_qty_brk [i] && i == 5)
		{
			discArray [1] = suds_disca_pc [i];
			discArray [2] = suds_discb_pc [i];
			discArray [3] = suds_discc_pc [i];
			break;
		}

		/*-------------------------------
		| Last non-zero quantity break. |
		-------------------------------*/
		if (qty >= suds_qty_brk [i] && suds_qty_brk [i + 1] == 0.00)
		{
			discArray [1] = suds_disca_pc [i];
			discArray [2] = suds_discb_pc [i];
			discArray [3] = suds_discc_pc [i];
			break;
		}

		/*------------------------------------------------
		| Greater than current break and less than next. |
		------------------------------------------------*/
		if (qty >= suds_qty_brk [i] && qty < suds_qty_brk [i + 1])
		{
			discArray [1] = suds_disca_pc [i];
			discArray [2] = suds_discb_pc [i];
			discArray [3] = suds_discc_pc [i];
			break;
		}
	}
	return ((suds_rec.cumulative [0] == 'Y') ? TRUE : FALSE);
}

/*-------------------------------------------------------
| CalcNet calculates the net cost based on the specifed |
| gross and discounts.                                  |
|                                                       |
| RETURNS : A double containing the net cost.           |
-------------------------------------------------------*/
double
CalcNet (
	double	gross, 
	float	discArray [4], 
	int		cumulative)
{
	int		i;
	double	netTot;
	double	tmpNet;
	double	totDisc;
	double	discValue;

	netTot = gross;
	if (cumulative == 1)
	{
		/*-------------------------------------------
		| Subtract Regulatory & discounts A, B & C. |
		-------------------------------------------*/
		for (i = 0; i < 4; i++)
		{
			tmpNet = netTot;
			if (discArray [i] != 0.00)
			{
				discValue = (tmpNet * (double) discArray [i]) / 100.00;
				netTot = tmpNet - discValue;
			}
		}
	}
	else
	{
		totDisc = (double) discArray [0] +
				  (double) discArray [1] + 
				  (double) discArray [2] + 
				  (double) discArray [3];
		if (totDisc != 0.00)
		{
			discValue = (netTot * totDisc) / 100.00;
			netTot -= discValue;
		}
	}
	return (netTot);
}

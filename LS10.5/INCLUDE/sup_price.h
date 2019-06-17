/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( sup_price.c    )                                 |
|  Program Desc  : ( Supplier Pricing and Discouting Routines.    )   |
|---------------------------------------------------------------------|
|  Access files  :  insp, suds,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Date Written  : (15/10/93)      |  Author     : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified by :                    |
|                                                                     |
|  Comments      :                                                    |
|  (  /  /  )    :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/

double	CalcNet (double gross, float discArray[4], int cumulative);
double	GetSupPrice (long, long, double, float);
int 	GetSupDisc (long, char *, float	qty, float [4]);
double	CalcNet (double, float [4], int);

/*-----------------------------------------------
| GetSupPrice calculates the supplier price for |
| the specified item based on the specified     |
| quantity.                                     |
|                                               |
| RETURNS : Supplier cost price for item.       |
-----------------------------------------------*/
double 
GetSupPrice (long hhsuHash, long hhbrHash, double cstPrice, float qty)
{
	int		i;

	/*-----------------------
	| Read the insp record. |
	-----------------------*/
	insp_rec.hhsu_hash = hhsuHash;
	insp_rec.hhbr_hash = hhbrHash;
	cc = find_rec(insp, &insp_rec, COMPARISON, "r");
	if (cc)
		return(cstPrice);

	/*------------------------------------------------------
	| Calculate the price based on the specified quantity. |
	------------------------------------------------------*/
	if (insp_rec.qty[0] == 0.00 || qty < insp_rec.qty[0])
		return(cstPrice);
	
	for (i = 0; i < 5; i++)
	{
		/*----------------------
		| Last quantity break. |
		----------------------*/
		if (qty >= insp_rec.qty[i] && i == 4)
			return(DOLLARS(insp_rec.price[i]));

		/*-------------------------------
		| Last non-zero quantity break. |
		-------------------------------*/
		if (qty >= insp_rec.qty[i] && insp_rec.qty[i + 1] == 0.00)
			return(DOLLARS(insp_rec.price[i]));

		/*------------------------------------------------
		| Greater than current break and less than next. |
		------------------------------------------------*/
		if (qty >= insp_rec.qty[i] && qty < insp_rec.qty[i + 1])
			return(DOLLARS(insp_rec.price[i]));
	}

	return(cstPrice);
}

/*------------------------------------------------------
| GetSupDisc calculates all supplier discounts for the |
| specified item based on the specified quantity.      |
|                                                      |
| The array of floats discArray contains the           |
| calculated discounts :                               |
|   discArray[0] = Regulatory %                        |
|   discArray[1] = Discount A %                        |
|   discArray[2] = Discount B %                        |
|   discArray[3] = Discount C %                        |
|                                                      |
| RETURNS : TRUE  - Discounts are cumulative.          |
|         : FALSE - Discounts are absolute.            |
| NB. Discounts are absolute by default. ie if no      |
| suds record exists then discounts are absolute.      |
------------------------------------------------------*/
int 
GetSupDisc (long hhsuHash, char *buyGroup, float qty, float discArray [4])
{
	int		i;

	/*-------------------
	| Initialise Array. |
	-------------------*/
	for (i = 0; i < 4; i++)
		discArray[i] = 0.00;

	/*-------------------
	| Find suds record. |
	-------------------*/
	suds_rec.hhsu_hash = hhsuHash;
	sprintf(suds_rec.buy_group, "%-6.6s", buyGroup);
	cc = find_rec(suds, &suds_rec, COMPARISON, "r");
	if (cc)
		return(FALSE);

	/*---------------
	| Regulatory %. |
	---------------*/
	discArray[0] = suds_rec.reg_pc;

	/*------------------------------------------------
	| Get discounts based on the specified quantity. |
	------------------------------------------------*/
	if (suds_rec.qty_brk[0] == 0.00 || qty < suds_rec.qty_brk[0])
		return((suds_rec.cumulative[0] == 'Y') ? TRUE : FALSE);

	for (i = 0; i < 6; i++)
	{
		/*----------------------
		| Last quantity break. |
		----------------------*/
		if (qty >= suds_rec.qty_brk[i] && i == 5)
		{
			discArray[1] = suds_rec.disca_pc[i];
			discArray[2] = suds_rec.discb_pc[i];
			discArray[3] = suds_rec.discc_pc[i];
			break;
		}

		/*-------------------------------
		| Last non-zero quantity break. |
		-------------------------------*/
		if (qty >= suds_rec.qty_brk[i] && suds_rec.qty_brk[i + 1] == 0.00)
		{
			discArray[1] = suds_rec.disca_pc[i];
			discArray[2] = suds_rec.discb_pc[i];
			discArray[3] = suds_rec.discc_pc[i];
			break;
		}

		/*------------------------------------------------
		| Greater than current break and less than next. |
		------------------------------------------------*/
		if (qty >= suds_rec.qty_brk[i] && qty < suds_rec.qty_brk[i + 1])
		{
			discArray[1] = suds_rec.disca_pc[i];
			discArray[2] = suds_rec.discb_pc[i];
			discArray[3] = suds_rec.discc_pc[i];
			break;
		}
	}

	return((suds_rec.cumulative[0] == 'Y') ? TRUE : FALSE);
}

/*-------------------------------------------------------
| CalcNet calculates the net cost based on the specifed |
| gross and discounts.                                  |
|                                                       |
| RETURNS : A double containing the net cost.           |
-------------------------------------------------------*/
double
CalcNet (double gross, float discArray[4], int cumulative)
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
			if (discArray[i] != 0.00)
			{
				discValue = (tmpNet * (double)discArray[i]) / 100.00;
				netTot = tmpNet - discValue;
			}
		}
	}
	else
	{
		totDisc = (double)discArray[0] +
				  (double)discArray[1] + 
				  (double)discArray[2] + 
				  (double)discArray[3];
		if (totDisc != 0.00)
		{
			discValue = (netTot * totDisc) / 100.00;
			netTot -= discValue;
		}
	}
	return(netTot);
}

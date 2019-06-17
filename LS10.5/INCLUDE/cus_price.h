/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (cus_price.c   )                                   |
|  Program Desc  : (Customer Pricing Routines.                  )     |
|---------------------------------------------------------------------|
|  Date Written  : (22/10/93)      |  Author     : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : (27/12/95)      | Modified by : Scott B Darrow.    |
|                                                                     |
|  Comments      :                                                    |
|  (27/12/95)    : FRA - Updated to fix bug found with Frazers.       |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<twodec.h>

/*
 * Function prototypes. 
 */
static double GetCusPrice (char *, char *, char *, char *, char *, char *, char *, int, char *, char *, long, long, long, char *, long, long, float, float, int, float *);
double	GetCusGprice (double, float);
double	ContCusPrice (long, long, long, char *, char *, int, float);
double	PromCusPrice (char *,char *,char *,char *, long, char *, char *,long, long, int);
float	CalcOneDisc (int cumulative, float discA, float discB, float discC);
double 	FindPrice (float, char *);
double	FindInpr (long, int, char *, char *, char *, char *, char *, float);
double	NormCusPrice (char *, char *, char *, char *, long, char *, float, int);
double	FindIncp (char *,char *, char *, char *, long, char *, char *, long, long,int);
void	OpenPrice (void);
void	ClosePrice (void);

/* Prototypes for functions in cus_disc.h */
int GetCusDisc (char *, char *, long, long, char *, char *, long, char *, char *, int, double, float, float, float []);
int FindInds (char *, char *, long, long, char *, long, char *, char *, int, double, float, float []);
float	CalcOneDisc (int, float, float, float);

/*
 * Structures For Database Access. 
 */
	/*
	 * Customer Contract detail lines 
	 */
	struct dbview cncd_list [] =
	{
		{"cncd_hhch_hash"},
		{"cncd_line_no"},
		{"cncd_hhbr_hash"},
		{"cncd_hhcc_hash"},
		{"cncd_hhsu_hash"},
		{"cncd_hhcl_hash"},
		{"cncd_price"},
		{"cncd_curr_code"},
		{"cncd_disc_ok"},
		{"cncd_cost"}
	};

	int	cncd_no_fields = 10;

	struct tag_cncdRecord
	{
		long	hhch_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhsu_hash;
		long	hhcl_hash;
		Money	price;		
		char	curr_code [4];
		char	disc_ok [2];
		Money	cost;			
	} cncd_rec;

	/*
	 * Inventory Contract Price File 
	 */
	struct dbview incp_list [] =
	{
		{"incp_key"},
		{"incp_hhcu_hash"},
		{"incp_area_code"},
		{"incp_cus_type"},
		{"incp_hhbr_hash"},
		{"incp_curr_code"},
		{"incp_status"},
		{"incp_date_from"},
		{"incp_date_to"},
		{"incp_price1"},
		{"incp_price2"},
		{"incp_price3"},
		{"incp_price4"},
		{"incp_price5"},
		{"incp_price6"},
		{"incp_price7"},
		{"incp_price8"},
		{"incp_price9"},
		{"incp_comment"},
		{"incp_dis_allow"},
		{"incp_stat_flag"}
	};

	int	incp_no_fields = 21;

	struct tag_incpRecord
	{
		char	key [7];
		long	hhcu_hash;
		char	area_code [3];
		char	cus_type [4];
		long	hhbr_hash;
		char	curr_code [4];
		char	status [2];
		long	date_from;
		long	date_to;
		Money	price [9];	
		char	comment [41];
		char	dis_allow [2];
		char	stat_flag [2];
	} incp_rec;

	/*
	 * Inventory Buying and Selling Groups 
	 */
	struct dbview ingp_list [] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	int	ingp_no_fields = 5;

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	} ingp_rec;

	/*
	 * Inventory Price File 
	 */
	struct dbview inpr_list [] =
	{
		{"inpr_hhbr_hash"},
		{"inpr_price_type"},
		{"inpr_br_no"},
		{"inpr_wh_no"},
		{"inpr_curr_code"},
		{"inpr_area_code"},
		{"inpr_cust_type"},
		{"inpr_hhgu_hash"},
		{"inpr_price_by"},
		{"inpr_qty_brk1"},
		{"inpr_qty_brk2"},
		{"inpr_qty_brk3"},
		{"inpr_qty_brk4"},
		{"inpr_qty_brk5"},
		{"inpr_qty_brk6"},
		{"inpr_qty_brk7"},
		{"inpr_qty_brk8"},
		{"inpr_qty_brk9"},
		{"inpr_base"},
		{"inpr_price1"},
		{"inpr_price2"},
		{"inpr_price3"},
		{"inpr_price4"},
		{"inpr_price5"},
		{"inpr_price6"},
		{"inpr_price7"},
		{"inpr_price8"},
		{"inpr_price9"}
	};

	int	inpr_no_fields = 28;

	struct tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	br_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	area_code [3];
		char	cust_type [4];
		long	hhgu_hash;
		char	price_by [2];
		double	qty_brk [9];
		Money	base;		
		Money	price [9];
	} inpr_rec;

char	*cncd = "cncd",
		*incp = "incp",
		*ingp = "ingp",
		*inpr = "inpr";

int		_CON_PRICE 			= FALSE,
		_cont_status 		= 0,
		_ignorePromoPrice 	= 0;

/*
 * _priceError is set by these routines to either TRUE or FALSE.  
 * The calling process can use the value of this variable AFTER the 
 * routine has been called.                                      
 * TRUE  - An error occurred and a price was not found.
 * FALSE - Price was found OK.                        
 */
int		_priceError = FALSE;

/*
 * _rtnPrice is set by these routines to the type of price found. 
 * The calling process can use the value of this variable AFTER the   
 * routine has been called.                  
 * 0 = No price found.                      
 * 1 = Contract price found.               
 * 2 = Promotional price found.           
 * 3 = Normal price found.               
 */
int		_rtnPrice = 0;

/*
 * _priceLevel is the default level to start 
 * at for price calculations :              
 * 0 = Company                             
 * 1 = Branch                              
 * 2 = Warehouse                           
 * _priceLevel should be set in the program to be the value of the 
 * environment variable SK_CUSPRI_LVL.               
 */
int		_priceLevel = 0;

/*
 * _priceOrder determines the order to check for the next price given that 
 * a price is not found for the current criteria.        
 * 0 = Check next highest level. eg If current check is at warehouse level 
 *     then check using the same criteria at Branch level 
 * 1 = Use price 1 for the current criteria.  
 * 2 = Step down to next price type.         
 * _priceOrder should be set in the program to be the value of the 
 * environment variable SK_PRI_ORD.                   
 */
int		_priceOrder = 2;

/*
 * _promoPrice determines whether a promotional price (if found) always 
 * overrides a normal price OR only if less than the normal price.
 * _promoPrice should be set in the program to be the value of the 
 * environment variable SK_PROMO_PRICE.                   
 */
int		_promoPrice = 0;

/*
 * _numQtyBrks determines the number of quantity breaks available to the 
 * user per price type.  _numQtyBrks should be set to the value of the
 * environment variable SK_DBQTYNUM.             
 */
int		_numQtyBrks = 0;

/*
 * _numPriceTypes determines the number of price types available to the user.
 * _numPriceTypes should be set to the value of the environment 
 * variable SK_DBPRINUM. 
 */
int		_numPriceTypes = 5;

/*
 * _majSellGrpLen determines the number of characters that make up the major 
 * selling group.                                   
 * _majSellGrpLen should be set to the value of the environment variable 
 * SK_MAJ_SELL. The default is 6 (ie the full length of the selling group field)
 */
int		_majSellGrpLen = 6;


/*
 * PRICING ROUTINES. 
 */
/*
 * OpenPrice opens all required database files for pricing and reads necessary 
 * environment variables.                                  
 */
void
OpenPrice (void)
{
	char	*sptr;

	/*
	 * Open Files. 
	 */
	open_rec (cncd, cncd_list, cncd_no_fields, "cncd_id_no2");
	open_rec (incp, incp_list, incp_no_fields, "incp_id_no");
	open_rec (ingp, ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec (inpr, inpr_list, inpr_no_fields, "inpr_id_no");

	/*
	 * Check for number of Quantity Breaks available. 
	 */
	sptr = chk_env ("SK_DBQTYNUM");
	_numQtyBrks = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (_numQtyBrks < 0 || _numQtyBrks > 9)
		_numQtyBrks = 9;

	/*
	 * Check for number of Price Types available. 
	 */
	sptr = chk_env ("SK_DBPRINUM");
	_numPriceTypes = (sptr == (char *)0) ? 5 : atoi (sptr);
	if (_numPriceTypes < 1 || _numPriceTypes > 9)
		_numPriceTypes = 9;

	/*
	 * Check for default pricing level. 
	 */
	sptr = chk_env ("SK_CUSPRI_LVL");
	_priceLevel = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for order of price checking. 
	 */
	sptr = chk_env ("SK_PRI_ORD");
	_priceOrder = (sptr == (char *)0) ? 2 : atoi (sptr);

	/*
	 * Check for Promotional Price Override. 
	 */
	sptr = chk_env ("SK_PROMO_PRICE");
	_promoPrice = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check for Length of major selling group. 
	 */
	sptr = chk_env ("SK_MAJ_SELL");
	_majSellGrpLen = (sptr == (char *)0) ? 6 : atoi (sptr);
}

/*
 * ClosePrice closes all database files used by the pricing routines.
 */
void
ClosePrice (void)
{
	abc_fclose (cncd);
	abc_fclose (incp);
	abc_fclose (ingp);
	abc_fclose (inpr);
}
double 
GetCusGprice (
	double	netPrice,
	float	regPc)
{
	double	priceLessReg;
	double	regAmt;

	/*
	 * Calculate price less regulatory percent. 
	 */
	priceLessReg = netPrice;
	if (regPc != (float)0.00)
	{
		regAmt = netPrice * (double)regPc;
		regAmt /= (double)100.00;
		regAmt = no_dec (regAmt);
		priceLessReg -= regAmt;
	}
	return (priceLessReg);
}

/*
 * GetCusPrice calculates the Customer price for the specified item based 
 * on the specified quantity.
 * RETURNS : Customer price for item.
 */
static	double 
GetCusPrice
(	
	 char	*coNo,
	 char	*brNo,
	 char	*whNo,
	 char	*areaCode,
	 char	*cusType,
	 char	*sellGrp,
	 char	*currCode,
	 int	pType,
	 char	*disCode,
	 char	*contType,
	 long	hhcuHash,
	 long	hhccHash,
	 long	hhbrHash,
	 char	*catg,
	 long	contHash,
	 long	dbtDate,
	 float	qty,
	 float	exchRate,
	 int	getFgn,
	 float	*regPc)
{	
	double	lclRegPc;
	double	regAmt;
	double	priceFnd;
	double	priceLessReg;
	double	promoFnd;

	int		cumDisc;
	float	discArray [3];
	double	calcDisc;
	char	base_curr [4];
	
	sprintf (base_curr, "%-3.3s", get_env ("CURR_CODE"));
	/*
	 * Initialise type of price found to 0 (No price found). 
	 */
	_rtnPrice = 0;
	_priceError = FALSE;

	*regPc = (float)0.00;

	/*
	 * Check price type. 
	 */
	if (pType < 1 || pType > _numPriceTypes)
	{
		_priceError = TRUE;
		return ((double)0.00);
	}

	/*
	 * Find ingp record based on minor selling group to get regulatory percent
	 */
	strcpy (ingp_rec.co_no, coNo);
	strcpy (ingp_rec.type, "S");
	sprintf (ingp_rec.code, "%-6.6s", sellGrp);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
	{
		/*
		 * Find ingp record based on major selling group.
		 */
		strcpy (ingp_rec.co_no, coNo);
		strcpy (ingp_rec.type, "S");
		sprintf (ingp_rec.code, "%-6.*s", _majSellGrpLen, sellGrp);
		cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
		if (cc)
			*regPc = 0.00;
		else
			*regPc = ingp_rec.sell_reg_pc;
	}
	else
		*regPc = ingp_rec.sell_reg_pc;
	lclRegPc = (double)*regPc;

	_CON_PRICE = FALSE;

	/*
	 * Don't look up contract price if contract hash 0
	 */
	if (contHash != 0L)
	{
		/*
	 	 * Look for contract price. 
		 */
		priceFnd	=	ContCusPrice 
						(	
							contHash, 
							hhbrHash, 
							hhccHash, 
							currCode, 
							contType, 
							getFgn,
							exchRate
						);
		if (priceFnd != (double) -1.00)
		{
			/*
			 * Set regulatory percent to zero. 
			 */
			*regPc = (float)0.00;
			_rtnPrice = 1;
			return (priceFnd);
		}
	}

	/*
	 * Look for promotional price. 
	 */
	if (_ignorePromoPrice == 1)
		promoFnd = (double) -1.00;
	else
	{
			priceFnd =	PromCusPrice 
						(
							coNo, 
							brNo, 
							whNo,   
							currCode, 
							hhcuHash, 
							areaCode,
							cusType, 
							hhbrHash, 
							dbtDate, 
							pType
						);

			promoFnd = priceFnd;
			if (promoFnd != (double) -1.00 && _promoPrice == 1)
			{
				/*
				 * Set regulatory percent to zero. 
				 */
				*regPc = (float)0.00;
				_rtnPrice = 2;
				return (promoFnd);
			}
	}

	/*
	 * Look for normal price. 
	 */
	priceFnd =	NormCusPrice 
				(
					brNo, 
					whNo, 
					areaCode, 
					cusType, 
					hhbrHash, 
					currCode, 
					qty, 
					pType
				);
	/*
	 * Deduct regulatory percent if non-zero. 
	 */
	priceLessReg = priceFnd;
	if (priceFnd != (double) -1.00 && lclRegPc != (float)0.00)
	{
		regAmt = priceFnd * lclRegPc;
		regAmt /= (double)100.00;
		regAmt = no_dec (regAmt);
		priceLessReg = priceFnd - regAmt;
	}

	/*
	 * Compare promotional price with normal price. 
	 */
	if (promoFnd != (double) -1.00)
	{
		if (priceFnd == (double) -1.00)
		{
			/*
			 * Set regulatory percent to zero. 
			 */
			*regPc = (float)0.00;
			_rtnPrice = 2;
			return (promoFnd);
		}
		if (incp_rec.dis_allow [0] == 'N')
		{
			cumDisc		=	GetCusDisc
						 	(	
								coNo,
								brNo,
								hhccHash,
								hhcuHash,
								cusType,
								disCode,
								hhbrHash,
								catg,
								sellGrp,
								pType,
								priceFnd,
								lclRegPc,
								qty,
								discArray 
							);
						
			calcDisc	=	CalcOneDisc
							(
								cumDisc,
								discArray [0],
								discArray [1],
								discArray [2] 
							);

			calcDisc *= priceLessReg;
			calcDisc = DOLLARS (calcDisc);
			calcDisc = no_dec (calcDisc);
			priceLessReg -= calcDisc;
		}
		if (promoFnd < priceLessReg)
		{
			/*
			 * Set regulatory percent to zero. 
			 */
			*regPc = (float)0.00;
			_rtnPrice = 2;
			return (promoFnd);
		}
	}

	_rtnPrice = 3;
	if (priceFnd == (double) -1.00)
	{
		_priceError = TRUE;
		priceFnd = (double)0.00;
	}
	if (_priceError)
	{
		if (strcmp (base_curr, currCode))
		{
			return (GetCusPrice
				(	
					 coNo,
					 brNo,
					 whNo,
	 				 areaCode,
					 cusType,
					 sellGrp,
					 base_curr,
					 pType,
					 disCode,
					 contType,
					 hhcuHash,
					 hhccHash,
					 hhbrHash,
					 catg,
					 contHash,
					 dbtDate,
					 qty,
					 exchRate,
					 FALSE,
					 regPc
				) * exchRate);
		}
	}
	_CON_PRICE = FALSE;
	return (priceFnd);
}

/*
 * ContCusPrice calculates the Contract Price for the specified item based 
 * on the contract details passed.
 * RETURNS : Contract price for item. -1.00 if no price found.           
 */
double 
ContCusPrice (
	long	hhchHash,
	long	hhbrHash,
	long	hhccHash,
	char	*currCode,
	char	*contType,
	int		getFgn,
	float	exchRate)
{
	/*
	 * _cont_status (GLOBAL variable) will be set to : 
	 *  0 = No Contract                               
	 *  1 = Discount Not OK                          
	 *  2 = Discount OK                             
	 */
	_cont_status = 0;

	cncd_rec.hhch_hash = hhchHash;
	cncd_rec.hhbr_hash = hhbrHash;
	if (contType [0] == 'F')		/* FIXED */
	{
		/*
		 * Lookup contract record 
		 */
		strcpy (cncd_rec.curr_code, currCode);
		cc = find_rec (cncd, &cncd_rec, EQUAL, "r");
		if (cc)
			return ((double) -1.00);

		/*
		 * If different warehouse 
		 */
		if (cncd_rec.hhcc_hash != 0L && cncd_rec.hhcc_hash != hhccHash)
			return ((double) -1.00);
	}
	else						/* VARIABLE */
	{
		/*
		 * This means cnch_rec.exch_type == 'V'  
		 * which does not need a currency match 
		 */
		strcpy (cncd_rec.curr_code, "   ");
		cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
		if (cc || 
			(!cc && 
			(cncd_rec.hhbr_hash != hhbrHash || 
			  cncd_rec.hhch_hash != hhchHash)))
		{
			return ((double) -1.00);
		}

		/*
		 * If different warehouse 
		 */
		if (cncd_rec.hhcc_hash != 0L && cncd_rec.hhcc_hash != hhccHash)
			return ((double) -1.00);

		if (getFgn)
		{
			cncd_rec.price *= exchRate;
			cncd_rec.price = no_dec (cncd_rec.price);
		}
	}

	/*
	 * This point will not be reached if there is no contract. 
	 */
	if (cncd_rec.disc_ok [0] == 'N')
		_cont_status = 1;
	else
		_cont_status = 2;
		
	_CON_PRICE = (cncd_rec.disc_ok [0] == 'Y') ? FALSE : TRUE;

	return (cncd_rec.price);
}

/*
 * PromCusPrice calculates the Promotional Customer price for the specified 
 * item based on the specified quantity.
 * RETURNS : Promotional price for item.        
 *         : -1.00 if no price found.          
 */
double 
PromCusPrice (
	char	*coNo,
	char	*brNo,
	char	*whNo,
	char	*currCode,
	long	hhcuHash,
	char	*areaCode,
	char	*cusType,
	long	hhbrHash,
	long	dtFrom,
	int		pType)
{
	double	priceFnd;

	/*
	 * Invalid item. 
	 */
	if (hhbrHash == 0L)
		return ((double) -1.00);

	/*
	 * Search for promotional price in following order :
	 *                                        
	 * WH level by Customer by Item            
	 * BR level by Customer by Item             
	 * CO level by Customer by Item              
	 *                                            
	 * WH level by Customer Type by Item           
	 * BR level by Customer Type by Item            
	 * CO level by Customer Type by Item             
	 *                                                
	 * WH level by Item                                
	 * BR level by Item                                 
	 * CO level by Item                                  
	 */

	/*
	 * Look up WH level record for Customer / Item. 
	 */
	if (_priceLevel == 2)
	{
		priceFnd = 	FindIncp
					(
						coNo, 
						brNo,
						whNo,
						currCode,
						hhcuHash, 
						"  ", 		/* areaCode = blank */
						"   ", 		/* cusType = blank */
						hhbrHash, 
						dtFrom, 
						pType
					);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up BR level record for Customer / Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							"  ",		/* Warehouse No = blank */
							currCode,
							hhcuHash, 
							"  ", 		/* areaCode = blank */
							"   ", 		/* cusType = blank */
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up CO level record for Customer / Item. 
	 */
	priceFnd	=	FindIncp 
					(
						coNo, 
						"  ",		/* Branch No = blank */
						"  ",		/* Warehouse No = blank */
						currCode,
						hhcuHash, 
						"  ", 		/* areaCode = blank */
						"   ", 		/* cusType = blank */
						hhbrHash, 
						dtFrom, 
						pType
					);

	if (priceFnd != (double) -1.00)
	{
		_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
		return (priceFnd);
	}

	/*
	 * Look up WH level record for Customer Area / Item. 
	 */
	if (_priceLevel == 2)
	{
		priceFnd	= 	FindIncp 
						(
							coNo, 
							brNo,
							whNo,
							currCode,
							hhcuHash, 
							areaCode, 
							"   ", 		/* cusType = blank */
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up BR level record for Customer Area / Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							"  ",		/* Warehouse No = blank */
							currCode,
							hhcuHash, 
							areaCode, 
							"   ", 		/* cusType = blank */
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up CO level record for Customer Area / Item. 
	 */
	priceFnd	=	FindIncp 
					(
						coNo, 
						"  ",		/* Branch No = blank */
						"  ",		/* Warehouse No = blank */
						currCode,
						hhcuHash, 
						areaCode, 
						"   ", 		/* cusType = blank */
						hhbrHash, 
						dtFrom, 
						pType
					);
	if (priceFnd != (double) -1.00)
	{
		_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
		return (priceFnd);
	}

	/*
	 * Look up WH level record for Customer Type / Item. 
	 */
	if (_priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							whNo,
							currCode,
							0L, 		/* hhcuHash = 0L */
							"  ", 		/* areaCode = blank */
							cusType, 
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up BR level record for Customer Type / Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							"  ",		/* Warehouse No = blank */
							currCode,
							0L, 		/* hhcuHash = 0L */
							"  ", 		/* areaCode = blank */
							cusType, 
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up CO level record for Customer Type / Item. 
	 */
	priceFnd	=	FindIncp 
					(
						coNo, 
						"  ",		/* Branch No = blank */
						"  ",		/* Warehouse No = blank */
						currCode,
						0L, 		/* hhcuHash = 0L */
						"  ", 		/* areaCode = blank */
						cusType, 
						hhbrHash, 
						dtFrom, 
						pType
					);
	if (priceFnd != (double) -1.00)
	{
		_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
		return (priceFnd);
	}

	/*
	 * Look up WH level record for Item. 
	 */
	if (_priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							whNo,
							currCode,
							0L, 		/* hhcuHash = 0L */
							"  ", 		/* areaCode = blank */
							"   ", 		/* cusType = blank */
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up BR level record for Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindIncp 
						(
							coNo, 
							brNo,
							"  ",		/* Warehouse No = blank */
							currCode,
							0L, 		/* hhcuHash = 0L */
							"  ", 		/* areaCode = blank */
							"   ", 		/* cusType = blank */
							hhbrHash, 
							dtFrom, 
							pType
						);
		if (priceFnd != (double) -1.00)
		{
			_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
			return (priceFnd);
		}
	}
	/*
	 * Look up CO level record for Item. 
	 */
	priceFnd	=	FindIncp 
					(
						coNo, 
						"  ",		/* Branch No = blank */
						"  ",		/* Warehouse No = blank */
						currCode,
						0L, 		/* hhcuHash = 0L */
						"  ", 		/* areaCode = blank */
						"   ", 		/* cusType = blank */
						hhbrHash, 
						dtFrom, 
						pType
					);
	if (priceFnd != (double) -1.00)
	{
		_CON_PRICE = (incp_rec.dis_allow [0] == 'Y') ? FALSE : TRUE;
		return (priceFnd);
	}
	return ((double) -1.00);
}
/*
 * Find relevent incp record for promotional pricing. 
 */
double	
FindIncp (
	char	*coNo,
	char	*brNo,
	char	*whNo,
	char	*currCode,
	long	hhcuHash,
	char	*areaCode,
	char	*cusType,
	long	hhbrHash,
	long	dbtDate,
	int		pType)
{
	char	incpKey [7],
			wkType	[4],
			wkArea	[3];

	/*
	 * Set up cust type. 
	 */
	if (cusType == (char *) 0)
		strcpy (wkType, "   ");
	else
		sprintf (wkType, "%-3.3s", cusType);

	/*
	 * Set up area code.
	 */
	if (areaCode == (char *) 0)
		strcpy (wkArea, "  ");
	else
		sprintf (wkArea, "%-2.2s", areaCode);

	/*
	 * Set up CO/BR/WH key. 
	 */
	sprintf (incpKey, "%2.2s%2.2s%2.2s", coNo, brNo, whNo);

	/*
	 * Find incp record. 
	 */
	strcpy (incp_rec.key,       incpKey);
	strcpy (incp_rec.curr_code, currCode);
	strcpy (incp_rec.status,    "A");
	sprintf (incp_rec.area_code, "%-2.2s", wkArea);
	sprintf (incp_rec.cus_type, "%-3.3s", wkType);
	incp_rec.hhcu_hash = hhcuHash;
	incp_rec.hhbr_hash = hhbrHash;
	incp_rec.date_from = dbtDate;

	cc = find_rec (incp, &incp_rec, LTEQ, "r");
	while (!cc && 
	       !strcmp (incp_rec.key,       incpKey) &&
	       !strcmp (incp_rec.curr_code, currCode) &&
	       incp_rec.hhcu_hash == hhcuHash && 
	       incp_rec.hhbr_hash == hhbrHash &&
	       incp_rec.status [0] == 'A' &&
	       !strcmp (incp_rec.area_code, wkArea) &&
	       !strcmp (incp_rec.cus_type, wkType))
	{
		/*
		 * Date in sub range for promotional price. 
		 */
		if (dbtDate >= incp_rec.date_from && dbtDate <= incp_rec.date_to)
		{
			if (incp_rec.price [pType - 1] == 0.00)
				return ((double) -1.00);

			return (incp_rec.price [pType - 1]);
		}
		cc = find_rec (incp, &incp_rec, PREVIOUS, "r");
	}

	return ((double) -1.00);
}


/*
 * NormCusPrice calculates the Normal (ie Not Contract or Promotional) 
 * Customer price for the specified item based on the specified quantity.
 * RETURNS : Normal price for item.             
 *         : -1.00 if no price found.            
 */
double 
NormCusPrice (
	char	*brNo,
	char	*whNo,
	char	*areaCode,
	char	*cusType,
	long	hhbrHash,
	char	*currCode,
	float	qty,
	int		pType)
{
	double	priceFnd;
	char	wkType [4];
	char	wkArea [3];

	/*
	 * Set up Area Code.
	 */
	if (areaCode == (char *) 0)
		strcpy (wkArea, "  ");
	else
		sprintf (wkArea, "%-2.2s", areaCode);

	/*
	 * Set up cust type. 
	 */
	if (cusType == (char *) 0)
		strcpy (wkType, "   ");
	else
		sprintf (wkType, "%-3.3s", cusType);

	/*
	 * Look up WH level record for Customer Type / Area / Item. 
	 * Look up WH level record for Customer Type / Item. 
	 */
	if (_priceLevel == 2)
	{
	 	/*
		 * Look up WH level record for Customer Type / Area / Item. 
		 */
		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							whNo, 
							currCode, 
							wkArea, 
							wkType, 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);

	 	/*
		 * Look up WH level record for Customer Type / Item. 
		 */
		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							whNo, 
							currCode, 
							"  ",		/* Area Code */
							wkType, 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);
	}
	/*
	 * Look up BR level record for Customer Type / Area / Item. 
	 * Look up BR level record for Customer Type / Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							"  ", 
							currCode, 
							wkArea, 
							wkType, 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);

		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							"  ", 
							currCode, 
							"  ",	/* Area */
							wkType, 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);
	}
	/*
	 * Look up CO level record for Customer Type / Area / Item. 
	 */
	priceFnd	=	FindInpr 
					(
						hhbrHash, 
						pType, 
						"  ", 
						"  ", 
						currCode, 
						wkArea, 
						wkType, 
						qty
					);
	if (priceFnd != (double) -1.00)
		return (priceFnd);

	/*
	 * Look up CO level record for Customer Type / Item. 
	 * Look up CO level record for Customer Type / Area / Item. 
	 */
	priceFnd	=	FindInpr 
					(
						hhbrHash, 
						pType, 
						"  ", 
						"  ", 
						currCode, 
						"  ",
						wkType, 
						qty
					);
	if (priceFnd != (double) -1.00)
		return (priceFnd);

	/*
	 * Look up WH level record for Item. 
	 */
	if (_priceLevel == 2)
	{
		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							whNo, 
							currCode, 
							"  ", 
							"   ", 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);
	}
	/*
	 * Look up BR level record for Item. 
	 */
	if (_priceLevel == 1 || _priceLevel == 2)
	{
		priceFnd	=	FindInpr 
						(
							hhbrHash, 
							pType, 
							brNo, 
							"  ", 
							currCode, 
							"  ", 
							"   ", 
							qty
						);
		if (priceFnd != (double) -1.00)
			return (priceFnd);
	}
	/*
	 * Look up CO level record for Item. 
	 */
	priceFnd	=	FindInpr 
					(
						hhbrHash, 
						pType, 
						"  ", 
						"  ", 
						currCode, 
						"  ", 
						"   ", 
						qty
					);
	if (priceFnd != (double) -1.00)
		return (priceFnd);

	return ((double) -1.00);
}
/*
 * Find required inpr record and call FindPrice () to get 
 * the actual price based on quantity 
 * RETURNS : price if found       
 *         : -1.00 if no price   
 */
double	
FindInpr (
	long	hhbrHash,
	int		pType,
	char	*brNo,
	char	*whNo,
	char	*currCode,
	char	*areaCode,
	char	*cusType,
	float	qty)
{
	int		i;
	double	priceFnd;

	priceFnd = (double) -1.00;
	for (i = pType; i > 0; i--)
	{
		inpr_rec.hhgu_hash  = 0L;
		inpr_rec.hhbr_hash  = hhbrHash;
		inpr_rec.price_type = i;
		strcpy (inpr_rec.br_no, brNo);
		strcpy (inpr_rec.wh_no, whNo);
		sprintf (inpr_rec.curr_code, "%-3.3s", currCode);
		sprintf (inpr_rec.area_code, "%-2.2s", areaCode);
		sprintf (inpr_rec.cust_type, "%-3.3s", cusType);
		cc = find_rec (inpr, &inpr_rec, COMPARISON, "r");
		if (!cc)
			priceFnd = FindPrice (qty, inpr_rec.price_by);
		
		if (priceFnd != (double) -1.00)
			return (priceFnd);

		/*
		 * Price not found for customer price type so break out and check 
		 * next highest level. eg If we are currently checking at Warehouse 
		 * level then the next check will be for the same criteria at Branch 
		 * level.
		 */
		if (_priceOrder == 0)
			break;

		/*
		 * Price not found for customer price type so try for price type 1.
		 */
		if (_priceOrder == 1 && i > 2)
			i = 2;     /* Loop will subtract 1 so i = 1 for next iteration */
	}

	/*
	 * Price not found. 
	 */
	return ((double) -1.00);
}
/*
 * Determine the correct price for the current price type based on the 
 * specified quantity. 
 * NOTE : The variable qtyVal holds either the qty OR the value 
 * (qty * basePrice) depending on the price_by field (V or Q).
 */
double 
FindPrice (
	float	qty,
	char	*priceBy)
{
	int		i;
	double	qtyVal;

	if (priceBy [0] == 'V')
		qtyVal = DOLLARS (inpr_rec.base) * (double)qty;
	else
		qtyVal = (double)qty;

	/*
	 * No quantity breaks set up OR qtyVal < first quantity break.  
	 */
	if (inpr_rec.qty_brk [0] == 0.00 || qtyVal < inpr_rec.qty_brk [0])
		return (inpr_rec.base);

	/*
	 * Do not bother checking past maximum number of quantity breaks.
	 */
	for (i = 0; i < _numQtyBrks; i++)
	{
		/*
		 * Last quantity break. 
		 */
		if (qtyVal >= inpr_rec.qty_brk [i] && i == (_numQtyBrks - 1))
			return (inpr_rec.price [i]);

		/*
		 * Last non-zero quantity break. 
		 */
		if (qtyVal >= inpr_rec.qty_brk [i] && inpr_rec.qty_brk [i + 1] == 0.00)
			return (inpr_rec.price [i]);

		/*
		 * Greater than current break and less than next. 
		 */
		if (qtyVal >= inpr_rec.qty_brk [i] && qtyVal < inpr_rec.qty_brk [i + 1])
			return (inpr_rec.price [i]);

	}
	return (inpr_rec.base);
}

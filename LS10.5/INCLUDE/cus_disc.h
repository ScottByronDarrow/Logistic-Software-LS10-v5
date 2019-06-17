#ifndef	CUS_DISC_H
#define	CUS_DISC_H
/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( cus_disc.c     )                                 |
|  Program Desc  : ( Customer Discouting Routines.                )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Date Written  : (29/10/93)      |  Author     : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : (20/07/94)      | Modified by : Jonathan Chen      |
|  Date Modified : (27/12/94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|  (20/07/94)    : Removed redundant #include                         |
|  (27/12/94)    : FRA - Updated for pricing / discounting changes.   |
|                :                                                    |
|                                                                     |
=====================================================================*/
int GetCusDisc(char *, char *, long, long, char *, char *, long, char *, char *, int, double, float, float, float []);
int FindInds(char *, char *, long, long, char *, long, char *, char *, int, double, float, float []);
float	CalcOneDisc (int, float, float, float);
void	OpenDisc(void);
void	CloseDisc(void);

/*=================================
| Structures For Database Access. |
=================================*/

#include	<include_excf.h>

	/*=================================
	| Customer External Discount File |
	=================================*/
	struct dbview exdf_list [] =
	{
		{"exdf_co_no"},
		{"exdf_code"},
		{"exdf_disc_pc"}
	};

	int	exdf_no_fields = 3;

	struct tag_exdfRecord
	{
		char	co_no[3];
		char	code[2];
		float	disc_pc;
	} exdf_rec;

	/*==================================
	| Customer Discount Subranges File |
	==================================*/
	struct dbview inds_list [] =
	{
		{"inds_co_no"},
		{"inds_br_no"},
		{"inds_hhcu_hash"},
		{"inds_price_type"},
		{"inds_category"},
		{"inds_sel_group"},
		{"inds_cust_type"},
		{"inds_hhbr_hash"},
		{"inds_hhcc_hash"},
		{"inds_disc_by"},
		{"inds_qty_brk1"},
		{"inds_qty_brk2"},
		{"inds_qty_brk3"},
		{"inds_qty_brk4"},
		{"inds_qty_brk5"},
		{"inds_qty_brk6"},
		{"inds_disca_pc1"},
		{"inds_disca_pc2"},
		{"inds_disca_pc3"},
		{"inds_disca_pc4"},
		{"inds_disca_pc5"},
		{"inds_disca_pc6"},
		{"inds_discb_pc1"},
		{"inds_discb_pc2"},
		{"inds_discb_pc3"},
		{"inds_discb_pc4"},
		{"inds_discb_pc5"},
		{"inds_discb_pc6"},
		{"inds_discc_pc1"},
		{"inds_discc_pc2"},
		{"inds_discc_pc3"},
		{"inds_discc_pc4"},
		{"inds_discc_pc5"},
		{"inds_discc_pc6"},
		{"inds_cum_disc"}
	};

	int	inds_no_fields = 35;

	struct tag_indsRecord
	{
		char	co_no[3];
		char	br_no[3];
		long	hhcu_hash;
		int		price_type;
		char	category[12];
		char	sel_group[7];
		char	cust_type[4];
		long	hhbr_hash;
		long	hhcc_hash;
		char	disc_by[2];
		double	qty_brk[6];
		float	disca_pc[6];
		float	discb_pc[6];
		float	discc_pc[6];
		char	cum_disc[2];
	} inds_rec;

char	*excf  = "excf";
char	*exdf  = "exdf";
char	*inds  = "inds";		/* NOTE inds should be open on inds_id_no */
char	*inds2 = "inds2"		/* NOTE inds2 should be open on inds_id_no2 */;

int		sub_catg;

/*------------------------------------------------
| GV_res is set when no special discounts apply. |
------------------------------------------------*/
int		GV_res	=	0;

/*---------------------------------------------
| _rtnDType is set by these routines to the   |
| type of discount found. The calling process |
| can use the value of this variable AFTER    |
| calling the routine.                        |
| 0  = No discount found.                     |
| 1  = Customer by Item.                      |
| 2  = Customer by Minor Selling Group.       |
| 3  = Customer by Major Selling Group.       |
| 4  = Customer by Minor Category.            |
| 5  = Customer by Major Category.            |
| 6  = Customer Type by Item.                 |
| 7  = Customer Type by Minor Selling Group.  |
| 8  = Customer Type by Major Selling Group.  |
| 9  = Customer Type by Minor Category.       |
| 10 = Customer Type by Major Category.       |
| 11 = Price Type by Item.                    |
| 12 = Price Type by Minor Selling Group.     |
| 13 = Price Type by Major Selling Group.     |
| 14 = Price Type by Minor Category.          |
| 15 = Price Type by Major Category.          |
| 16 = Maximum discount for category.         |
| 17 = Customer discount.                     |
---------------------------------------------*/
int	_rtnDType = 0;
#define	D_NODISC		0
#define	D_CUST_ITEM		1
#define	D_CUST_MINSELL	2
#define	D_CUST_MAJSELL	3
#define	D_CUST_MINCAT	4
#define	D_CUST_MAJCAT	5
#define	D_CTYP_ITEM		6
#define	D_CTYP_MINSELL	7
#define	D_CTYP_MAJSELL	8
#define	D_CTYP_MINCAT	9
#define	D_CTYP_MAJCAT	10
#define	D_PTYP_ITEM		11
#define	D_PTYP_MINSELL	12
#define	D_PTYP_MAJSELL	13
#define	D_PTYP_MINCAT	14
#define	D_PTYP_MAJCAT	15
#define	D_MAX_CAT		16
#define	D_CUSTOMER		17

/*---------------------------------------------
| _rtnDLvl is set by these routines to the    |
| level of discount found.  The value of      |
| _rtnDLvl will only be meaningful if the     |
| value of _rtnDType is non zero. The calling |
| process can use the value of this variable  |
| AFTER calling the routine.                  |
| 0  = Company level.                         |
| 1  = Branch level.                          |
| 2  = Warehouse level.                       |
---------------------------------------------*/
int	_rtnDLvl = 0;

/*---------------------------------------------
| _discLevel is the default level to start at |
| for discount calculations :                 |
| 0 = Company                                 |
| 1 = Branch                                  |
| 2 = Warehouse                               |
|                                             |
| _discLevel should be set in the program to  |
| be the value of the environment             |
| SK_CUSDIS_LVL.                              |
---------------------------------------------*/
int	_discLevel = 0;

/*------------------------------------------------------
| OpenDisc opens all required database files and reads |
| necessary environment variables.                     |
------------------------------------------------------*/
void
OpenDisc(void)
{
	char	*sptr;

	/*-------------
	| Open files. |
	-------------*/
	abc_alias(inds2, inds);

	open_rec(excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec(exdf,  exdf_list, exdf_no_fields, "exdf_id_no");
	open_rec(inds,  inds_list, inds_no_fields, "inds_id_no");
	open_rec(inds2, inds_list, inds_no_fields, "inds_id_no2");

	/*---------------------------------------
	| Number of characters in sub category. |
	---------------------------------------*/
	sptr = chk_env("DIS_FIND");
	sub_catg = (sptr == (char *)0) ? 0 : atoi(sptr);

	/*---------------------------
	| Default discouting level. |
	---------------------------*/
	sptr = chk_env("SK_CUSDIS_LVL");
	_discLevel = (sptr == (char *)0) ? 0 : atoi(sptr);
}

/*------------------------------------------
| CloseDisc closes all database files used |
| by the discounting routines.             |
------------------------------------------*/
void
CloseDisc(void)
{
	abc_fclose(excf);
	abc_fclose(exdf);
	abc_fclose(inds);
	abc_fclose(inds2);
}

/*------------------------------------------------------
| GetCusDisc calculates all customer discounts for the |
| specified item based on the specified quantity.      |
|                                                      |
| The array of floats discArray contains the           |
| calculated discounts :                               |
|   discArray[0] = Discount A %                        |
|   discArray[1] = Discount B %                        |
|   discArray[2] = Discount C %                        |
|                                                      |
| RETURNS : TRUE  - Discounts are cumulative.          |
|         : FALSE - Discounts are absolute.            |
| NB. Discounts are absolute by default. ie if no      |
| inds record exists then discounts are absolute.      |
| Notes : sellPrice is expected to already have the    |
| regulatory percent deducted.                         |
------------------------------------------------------*/
int GetCusDisc(char *coNo, char *brNo, long hhccHash, long hhcuHash, char *cType, char *discCode, long hhbrHash, char *categ, char *selGrp, int pType, double sellPrice, float regPc, float qty, float discArray[3])
{
	int		i;
	int		discFnd;
	float	maxDisc;
	double	priceLessReg;
	double	regAmt;
	char	majorSell[7];
	char	majorCat[12];

	GV_res	=	0;

	/*-------------------
	| Initialise Array. |
	-------------------*/
	for (i = 0; i < 3; i++)
		discArray[i] = (float)0.00;
	/*------------------------------------------
	| Calculate price less regulatory percent. |
	------------------------------------------*/
	priceLessReg = sellPrice;
	if (regPc != (float)0.00)
	{
		regAmt = sellPrice * (double)regPc;
		regAmt /= (double)100.00;
		regAmt = no_dec(regAmt);
		priceLessReg -= regAmt;
	}

	/*--------------------------------------------------
	| Search for discounts in the following order. As  |
	| soon as a discount is found it is returned.      |
	|                                                  |
	| WH level by Customer by Item                     |
	| BR level by Customer by Item                     |
	| CO level by Customer by Item                     |
	|                                                  |
	| WH level by Customer by Minor Selling Group      |
	| WH level by Customer by Major Selling Group      |
	| BR level by Customer by Minor Selling Group      |
	| BR level by Customer by Major Selling Group      |
	| CO level by Customer by Minor Selling Group      |
	| CO level by Customer by Major Selling Group      |
	|                                                  |
	| WH level by Customer by Minor Category           |
	| WH level by Customer by Major Category           |
	| BR level by Customer by Minor Category           |
	| BR level by Customer by Major Category           |
	| CO level by Customer by Minor Category           |
	| CO level by Customer by Major Category           |
	|                                                  |
	|                                                  |
	| WH level by Customer Type by Item                |
	| BR level by Customer Type by Item                |
	| CO level by Customer Type by Item                |
	|                                                  |
	| WH level by Customer Type by Minor Selling Group |
	| WH level by Customer Type by Major Selling Group |
	| BR level by Customer Type by Minor Selling Group |
	| BR level by Customer Type by Major Selling Group |
	| CO level by Customer Type by Minor Selling Group |
	| CO level by Customer Type by Major Selling Group |
	|                                                  |
	| WH level by Customer Type by Minor Category      |
	| WH level by Customer Type by Major Category      |
	| BR level by Customer Type by Minor Category      |
	| BR level by Customer Type by Major Category      |
	| CO level by Customer Type by Minor Category      |
	| CO level by Customer Type by Major Category      |
	|                                                  |
	|                                                  |
	| WH level by Price Type by Item                   |
	| BR level by Price Type by Item                   |
	| CO level by Price Type by Item                   |
	|                                                  |
	| WH level by Price Type by Minor Selling Group    |
	| WH level by Price Type by Major Selling Group    |
	| BR level by Price Type by Minor Selling Group    |
	| BR level by Price Type by Major Selling Group    |
	| CO level by Price Type by Minor Selling Group    |
	| CO level by Price Type by Major Selling Group    |
	|                                                  |
	| WH level by Price Type by Minor Category         |
	| WH level by Price Type by Major Category         |
	| BR level by Price Type by Minor Category         |
	| BR level by Price Type by Major Category         |
	| CO level by Price Type by Minor Category         |
	| CO level by Price Type by Major Category         |
	|                                                  |
	--------------------------------------------------*/
	_rtnDType = 0;
	_rtnDLvl = 0;

/*---------------------
| BY Customer By Item |
---------------------*/
	/*-----------------------------------------------
	| Look up WAREHOUSE record for Customer / Item. |
	-----------------------------------------------*/
	if (_discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_ITEM;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*--------------------------------------------
	| Look up BRANCH record for Customer / Item. |
	--------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_ITEM;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*---------------------------------------------
	| Look up COMPANY record for Customer / Item. |
	---------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   hhcuHash,  
					   "   ", 			/* cType = blank */
					   hhbrHash, 
					   "           ",	/* categ = blank */
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CUST_ITEM;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*---------------------------
| BY Customer By Sell Group |
---------------------------*/
	/*-----------------------------
	| Set up major selling group. |
	-----------------------------*/
	sprintf(majorSell, "%-6.*s", _majSellGrpLen, selGrp);

	/*-----------------------------------------------------
	| Look up WAREHOUSE record for Customer / Sell Group. |
	-----------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*-----------------------------------------------------------
		| Look up WAREHOUSE record for Customer / Minor Sell Group. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MINSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*-----------------------------------------------------------
		| Look up WAREHOUSE record for Customer / Major Sell Group. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MAJSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*--------------------------------------------------
	| Look up BRANCH record for Customer / Sell Group. |
	--------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*--------------------------------------------------------
		| Look up BRANCH record for Customer / Minor Sell Group. |
		--------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MINSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*--------------------------------------------------------
		| Look up BRANCH record for Customer / Major Sell Group. |
		--------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MAJSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*---------------------------------------------------------
	| Look up COMPANY record for Customer / Minor Sell Group. |
	---------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   hhcuHash,  
					   "   ", 			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   selGrp,	
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CUST_MINSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*---------------------------------------------------------
	| Look up COMPANY record for Customer / Major Sell Group. |
	---------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   hhcuHash,  
					   "   ", 			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   majorSell,
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CUST_MAJSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*-------------------------
| BY Customer By Category |
-------------------------*/
	/*------------------------
	| Set up major category. |
	------------------------*/
	sprintf(majorCat, "%-11.*s", sub_catg, categ);

	/*---------------------------------------------------
	| Look up WAREHOUSE record for Customer / Category. |
	---------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*---------------------------------------------------------
		| Look up WAREHOUSE record for Customer / Minor Category. |
		---------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   categ,	
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MINCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*---------------------------------------------------------
		| Look up WAREHOUSE record for Customer / Major Category. |
		---------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   majorCat,	
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MAJCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*------------------------------------------------
	| Look up BRANCH record for Customer / Category. |
	------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*------------------------------------------------------
		| Look up BRANCH record for Customer / Minor Category. |
		------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   categ,
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MINCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*------------------------------------------------------
		| Look up BRANCH record for Customer / Major Category. |
		------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   hhcuHash,  
						   "   ", 			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   majorCat,
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CUST_MAJCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*-------------------------------------------------------
	| Look up COMPANY record for Customer / Minor Category. |
	-------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   hhcuHash,  
					   "   ", 			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   categ,
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CUST_MINCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*-------------------------------------------------------
	| Look up COMPANY record for Customer / Major Category. |
	-------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   hhcuHash,  
					   "   ", 			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   majorCat,
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CUST_MAJCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}


/*--------------------------
| BY Customer Type By Item |
--------------------------*/
	/*----------------------------------------------------
	| Look up WAREHOUSE record for Customer Type / Item. |
	----------------------------------------------------*/
	if (_discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_ITEM;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*-------------------------------------------------
	| Look up BRANCH record for Customer Type / Item. |
	-------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_ITEM;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*--------------------------------------------------
	| Look up COMPANY record for Customer Type / Item. |
	--------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   cType,
					   hhbrHash, 
					   "           ",	/* categ = blank */
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CTYP_ITEM;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*--------------------------------
| BY Customer Type By Sell Group |
--------------------------------*/
	/*----------------------------------------------------------
	| Look up WAREHOUSE record for Customer Type / Sell Group. |
	----------------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*----------------------------------------------------------------
		| Look up WAREHOUSE record for Customer Type / Minor Sell Group. |
		----------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MINSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*----------------------------------------------------------------
		| Look up WAREHOUSE record for Customer Type / Major Sell Group. |
		----------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MAJSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*-------------------------------------------------------
	| Look up BRANCH record for Customer Type / Sell Group. |
	-------------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*-------------------------------------------------------------
		| Look up BRANCH record for Customer Type / Minor Sell Group. |
		-------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MINSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*-------------------------------------------------------------
		| Look up BRANCH record for Customer Type / Major Sell Group. |
		-------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   cType,
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MAJSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*--------------------------------------------------------------
	| Look up COMPANY record for Customer Type / Minor Sell Group. |
	--------------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   cType,
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   selGrp,	
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CTYP_MINSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*--------------------------------------------------------------
	| Look up COMPANY record for Customer Type / Major Sell Group. |
	--------------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   cType,
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   majorSell,
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CTYP_MAJSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*------------------------------
| BY Customer Type By Category |
------------------------------*/
	/*--------------------------------------------------------
	| Look up WAREHOUSE record for Customer Type / Category. |
	--------------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*--------------------------------------------------------------
		| Look up WAREHOUSE record for Customer Type / Minor Category. |
		--------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   cType, 
						   0L, 				/* hhbrHash = 0L */
						   categ,	
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MINCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*--------------------------------------------------------------
		| Look up WAREHOUSE record for Customer Type / Major Category. |
		--------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   cType, 
						   0L, 				/* hhbrHash = 0L */
						   majorCat,	
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MAJCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*-----------------------------------------------------
	| Look up BRANCH record for Customer Type / Category. |
	-----------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*-----------------------------------------------------------
		| Look up BRANCH record for Customer Type / Minor Category. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   cType, 
						   0L, 				/* hhbrHash = 0L */
						   categ,
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MINCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*-----------------------------------------------------------
		| Look up BRANCH record for Customer Type / Major Category. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   cType, 
						   0L, 				/* hhbrHash = 0L */
						   majorCat,
						   "      ",		/* selGrp = blank */
						   0, 		   		/* pType = 0 */
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_CTYP_MAJCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*------------------------------------------------------------
	| Look up COMPANY record for Customer Type / Minor Category. |
	------------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   cType,
					   0L, 				/* hhbrHash = 0L */
					   categ,
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CTYP_MINCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*------------------------------------------------------------
	| Look up COMPANY record for Customer Type / Major Category. |
	------------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   cType,
					   0L, 				/* hhbrHash = 0L */
					   majorCat,
					   "      ",		/* selGrp = blank */
					   0, 		   		/* pType = 0 */
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_CTYP_MAJCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}


/*-----------------------
| BY Price Type By Item |
-----------------------*/
	/*-------------------------------------------------
	| Look up WAREHOUSE record for Price Type / Item. |
	-------------------------------------------------*/
	if (_discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_ITEM;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*----------------------------------------------
	| Look up BRANCH record for Price Type / Item. |
	----------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   hhbrHash, 
						   "           ",	/* categ = blank */
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_ITEM;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*-----------------------------------------------
	| Look up COMPANY record for Price Type / Item. |
	-----------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   "   ",			/* cType = blank */
					   hhbrHash, 
					   "           ",	/* categ = blank */
					   "      ",		/* selGrp = blank */
					   pType,
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_PTYP_ITEM;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*-----------------------------
| BY Price Type By Sell Group |
-----------------------------*/
	/*-------------------------------------------------------
	| Look up WAREHOUSE record for Price Type / Sell Group. |
	-------------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*-------------------------------------------------------------
		| Look up WAREHOUSE record for Price Type / Minor Sell Group. |
		-------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MINSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*-------------------------------------------------------------
		| Look up WAREHOUSE record for Price Type / Major Sell Group. |
		-------------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MAJSELL;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*----------------------------------------------------
	| Look up BRANCH record for Price Type / Sell Group. |
	----------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*----------------------------------------------------------
		| Look up BRANCH record for Price Type / Minor Sell Group. |
		----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   selGrp,	
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MINSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*----------------------------------------------------------
		| Look up BRANCH record for Price Type / Major Sell Group. |
		----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   "           ",	/* categ = blank */
						   majorSell,
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MAJSELL;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*-----------------------------------------------------------
	| Look up COMPANY record for Price Type / Minor Sell Group. |
	-----------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   "   ",			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   selGrp,	
					   pType,
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_PTYP_MINSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*-----------------------------------------------------------
	| Look up COMPANY record for Price Type / Major Sell Group. |
	-----------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   "   ",			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   "           ",	/* categ = blank */
					   majorSell,
					   pType,
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_PTYP_MAJSELL;
		_rtnDLvl = 0;
		return(discFnd);
	}

/*---------------------------
| BY Price Type By Category |
---------------------------*/
	/*-----------------------------------------------------
	| Look up WAREHOUSE record for Price Type / Category. |
	-----------------------------------------------------*/
	if (_discLevel == 2)
	{
		/*-----------------------------------------------------------
		| Look up WAREHOUSE record for Price Type / Minor Category. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   categ,	
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MINCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
		/*-----------------------------------------------------------
		| Look up WAREHOUSE record for Price Type / Major Category. |
		-----------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   hhccHash, 
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   majorCat,	
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MAJCAT;
			_rtnDLvl = 2;
			return(discFnd);
		}
	}
	/*--------------------------------------------------
	| Look up BRANCH record for Price Type / Category. |
	--------------------------------------------------*/
	if (_discLevel == 1 || _discLevel == 2)
	{
		/*--------------------------------------------------------
		| Look up BRANCH record for Price Type / Minor Category. |
		--------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   categ,
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MINCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
		/*--------------------------------------------------------
		| Look up BRANCH record for Price Type / Major Category. |
		--------------------------------------------------------*/
		discFnd = FindInds(coNo,  
						   brNo,   
						   0L, 				/* hhccHash = 0L */
						   0L,  			/* hhcuHash = 0L */
						   "   ",			/* cType = blank */
						   0L, 				/* hhbrHash = 0L */
						   majorCat,
						   "      ",		/* selGrp = blank */
						   pType,
						   priceLessReg, 
						   qty,   
						   discArray);
		if (discFnd != -1)
		{
			_rtnDType = D_PTYP_MAJCAT;
			_rtnDLvl = 1;
			return(discFnd);
		}
	}
	/*---------------------------------------------------------
	| Look up COMPANY record for Price Type / Minor Category. |
	---------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   "   ",			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   categ,
					   "      ",		/* selGrp = blank */
					   pType,
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_PTYP_MINCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}
	/*---------------------------------------------------------
	| Look up COMPANY record for Price Type / Major Category. |
	---------------------------------------------------------*/
	discFnd = FindInds(coNo,  
					   "  ",   			/* brNo = blank */
					   0L, 				/* hhccHash = 0L */
					   0L,  			/* hhcuHash = 0L */
					   "   ",			/* cType = blank */
					   0L, 				/* hhbrHash = 0L */
					   majorCat,
					   "      ",		/* selGrp = blank */
					   pType,
					   priceLessReg, 
					   qty,   
					   discArray);
	if (discFnd != -1)
	{
		_rtnDType = D_PTYP_MAJCAT;
		_rtnDLvl = 0;
		return(discFnd);
	}

	/*----------------------------------
	| No discount applied so set flag. |
	----------------------------------*/
	GV_res	=	1;

/*-------------------------------------
| Failed specific discounting checks. |
-------------------------------------*/
	/*---------------------------------
	| Find max discount for category. |
	---------------------------------*/
	maxDisc = (float)0.00;
	strcpy(excf_rec.co_no, coNo);
	sprintf(excf_rec.cat_no, "%-11.11s", categ);
	cc = find_rec(excf, &excf_rec, COMPARISON, "r");
	if (!cc)
		maxDisc = excf_rec.max_disc;

	/*--------------------------------------
	| Discount for customer discount rate. |
	--------------------------------------*/
	if (discCode != (char *)0 && *discCode)
	{
		strcpy(exdf_rec.co_no, coNo);
		sprintf(exdf_rec.code, "%-1.1s", discCode);
		cc = find_rec(exdf, &exdf_rec, COMPARISON, "r");
		if (!cc)
		{
			if (exdf_rec.disc_pc > maxDisc && maxDisc != (float)0.00)
			{
				discArray[0] = maxDisc;
				_rtnDType = D_MAX_CAT;
				_rtnDLvl = 0;
			}
			else
			{
				discArray[0] = exdf_rec.disc_pc;
				_rtnDType = D_CUSTOMER;
				_rtnDLvl = 0;
			}
		}
	}

	return(0);
}

/*--------------------------------------------------
| FindInds looks for an inds record based on the   |
| specifed criteria and if found then determines   |
| the correct discounts based on the quantity etc  |
|                                                  |
| RETURNS : -1 - If record not found.              |
|         :  0 - If discounts are absolute.        |
|         :  1 - If discounts are cumulative.      |
--------------------------------------------------*/
int
FindInds(char *coNo, char *brNo, long hhccHash, long hhcuHash, char *cType, long hhbrHash, char *categ, char *selGrp, int pType, double sellPrice, float qty, float discArray[3])
{
	int		i;
	char	fileToUse[6];
	double	qtyVal;

	/*-------------------------------
	| Determine which index to use. |
	-------------------------------*/
	if (pType == 0)
		strcpy(fileToUse, inds);
	else
		strcpy(fileToUse, inds2);

	/*----------------------
	| Look for inds record |
	----------------------*/
	strcpy(inds_rec.co_no, coNo);
	strcpy(inds_rec.br_no, brNo);
	inds_rec.hhcc_hash = hhccHash;
	inds_rec.hhcu_hash = hhcuHash;
	strcpy(inds_rec.cust_type, cType);
	inds_rec.hhbr_hash = hhbrHash;
	strcpy(inds_rec.category,  categ);
	strcpy(inds_rec.sel_group, selGrp);
	inds_rec.price_type = pType;
	cc = find_rec(fileToUse, &inds_rec, COMPARISON, "r");
	if (cc)
		return(-1);

	/*----------------
	| Set up qtyVal. |
	----------------*/
	if (inds_rec.disc_by[0] == 'V')
		qtyVal = DOLLARS(sellPrice) * (double)qty;
	else
		qtyVal = (double)qty;

	/*------------------------------------------------
	| Get discounts based on the specified quantity. |
	------------------------------------------------*/
	if (inds_rec.qty_brk[0] == 0.00 || qtyVal < inds_rec.qty_brk[0])
		return((inds_rec.cum_disc[0] == 'Y') ? TRUE : FALSE);

	for (i = 0; i < 6; i++)
	{
		/*----------------------
		| Last quantity break. |
		----------------------*/
		if (qtyVal >= inds_rec.qty_brk[i] && i == 5)
		{
			discArray[0] = inds_rec.disca_pc[i];
			discArray[1] = inds_rec.discb_pc[i];
			discArray[2] = inds_rec.discc_pc[i];
			break;
		}

		/*-------------------------------
		| Last non-zero quantity break. |
		-------------------------------*/
		if (qtyVal >= inds_rec.qty_brk[i] && inds_rec.qty_brk[i + 1] == 0.00)
		{
			discArray[0] = inds_rec.disca_pc[i];
			discArray[1] = inds_rec.discb_pc[i];
			discArray[2] = inds_rec.discc_pc[i];
			break;
		}

		/*------------------------------------------------
		| Greater than current break and less than next. |
		------------------------------------------------*/
		if (qtyVal >= inds_rec.qty_brk[i] && qtyVal < inds_rec.qty_brk[i + 1])
		{
			discArray[0] = inds_rec.disca_pc[i];
			discArray[1] = inds_rec.discb_pc[i];
			discArray[2] = inds_rec.discc_pc[i];
			break;
		}
	}
	return((inds_rec.cum_disc[0] == 'Y') ? TRUE : FALSE);
}

/*-------------------------------------------------------------
| CalcOneDisc calculates a single discount figure             |
| from the list of discounts passed based on the              |
| discount type (Cumulative or Absolute)                      |
|                                                             |
| PARAMETERS :                                                |
|  int	 cumulative - Discounts are cumulative (TRUE / FALSE) |
|  float discA		- discount A.                             |
|  float discB		- discount B.                             |
|  float discC		- discount C.                             |
|                                                             |
| RETURNS : A float containing the single discount.           |
-------------------------------------------------------------*/
float	CalcOneDisc (int cumulative, float discA, float discB, float discC)
{
	int		i;
	int		numDiscs;
	float	nDisc[10];
	float	tmpDisc;
	float	newDisc;

	nDisc[0] = discA;
	nDisc[1] = discB;
	nDisc[2] = discC;
	numDiscs = 3;

	/*------------------------------------
	| If absolute then add all together. |
	------------------------------------*/
	if (!cumulative)
	{
		newDisc = (float)0.00;
		for (i = 0; i < numDiscs; i++)
			newDisc += nDisc[i];

		return(newDisc);
	}

	/*----------------------------
	| Calculate single discount. |
	----------------------------*/
	newDisc = nDisc[0];
	for (i = 1; i < numDiscs; i++)
	{
		tmpDisc = (float)100.00 - newDisc;
		tmpDisc *= nDisc[i];
		tmpDisc = twodec(tmpDisc / (float)100.00);

		newDisc += tmpDisc;
	}

	newDisc = twodec(newDisc);

	return(newDisc);
}

#endif	/*CUS_DISC_H*/

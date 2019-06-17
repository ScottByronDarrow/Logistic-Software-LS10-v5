/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: LRPFunctions.h,v 5.1 2001/08/06 22:49:50 scott Exp $
-----------------------------------------------------------------------
| $Log: LRPFunctions.h,v $
| Revision 5.1  2001/08/06 22:49:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:51:18  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:34  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.2  2000/06/13 05:22:41  scott
| S/C LSANZ-16400
| Updated to allow for demend type '6' - Production issues.
|
| Revision 1.1  2000/05/29 01:48:18  scott
| Updated include files to make things like searches and item lookup standard.
| All include files will work with standard app.schema defines.
|
|	Revision 1.17  2000/02/18 01:02:17  scott
|	Updated from Trevor. Compile warnings found with Linux.
|	
|	Revision 1.16  1999/12/10 03:21:07  scott
|	Updated to ensure daily future demand was working.
|	
|	Revision 1.15  1999/12/09 06:42:27  scott
|	Updated to use more standard deviation calculation.
|	
|	Revision 1.14  1999/11/12 08:01:19  scott
|	Updated for small changes made to what data method 'C' uses.
|	
+====================================================================*/
#define	LSA_ACT		0
#define	LSA_LIN		1
#define	LSA_SLT		2
#define	LSA_LLN		3
#define	LSA_FF		4
#define	LSA_TMP		5
#define	LSA_BIAS	6

#define	MAX_METHODS	8

/*-------------
| Period type |
-------------*/
#define LRP_PASSED_MONTH 	 	0
#define LRP_PASSED_DAY    		1

/*-------------
| Demand type |
-------------*/
#define SALES_DEMAND     '1'
#define FUTURE_DEMAND    '2'
#define EXCEPTIONS	     '3'
#define TRANSFERS        '4'
#define LOST_SALES       '5' 
#define PC_ISSUES        '6' 

/*------------------
| Demand Sub Types |
------------------*/
#define SALES_ONLY				 '1'
#define PLUS_TRANSFERS           '2' 
#define PLUS_LOSTSALES           '3'
#define PLUS_PC_ISSUES			 '4'

/*------------------------
| Demand by demandMode.  |
------------------------*/
#define BY_COMPANY   			0 
#define BY_BRANCH    			1
#define BY_WAREHOUSE 			2

#define	MAX_PERIODS  48     /* 36 for the history plus 12 future */	
#define	MAX_HISTORY  36     /* 36 for the history */	

int 	periodType			=	LRP_PASSED_MONTH;
char 	demandSubType [5];

char	LSA_methods 	[MAX_METHODS];
double	LSA_error 		[MAX_METHODS] [2];
double	LSA_result 		[MAX_METHODS] [MAX_PERIODS];
double	LSA_last3 		[MAX_METHODS];
int		LSA_hist,
		zero_hist;
long	LSA_vld_cc [200];
static	int	LSA_day,		/* Current stock day		*/
		LSA_mnth,			/*		 month				*/
		LSA_year;			/* 		 year				*/
static	double	monthArray [MAX_PERIODS],
				calculateArray [MAX_PERIODS];

double	LSA_percentError	=	20.00;
int		LSA_negativeDemand	=	FALSE;
int		LSA_environments	=	FALSE;
int		LSA_WeekDay			=	TRUE;

long	LSA_oldestDate;

void	CalcLSADates (long);

static	struct	LSA_dates 
{
	long	StartDate;
	long	EndDate;
	float	QtySold;
} LSA_date_rec [ MAX_PERIODS + 3 ];

char *ffdm = "ffdm";

struct dbview ffdm_list [] =
{
   {"ffdm_hhcc_hash"},
   {"ffdm_hhbr_hash"},
   {"ffdm_date"},
   {"ffdm_type"},
   {"ffdm_qty"}
};

int ffdm_no_fields = 5;

struct 
{
	long	hhcc_hash;
	long	hhbr_hash;
	long	date;
	char	type [2];
	float	qty;
} ffdm_rec, ffdm_rec2;

/*=============
| Prototypes. |
=============*/
int		setup_LSA		(int, char *, char *, char *);
int		calc_LSA		(char *, long, long, int, int,int,char*);

static void	_calc_LSA 	(double [], double [], int, int);
static void	GetLinearEqn (double [], double [], int, double *, double *);
static void SimSolve 	(double, double, double, double, double, double, double *, double *);

static void	LSA_actual 		(void),
			LSA_linear 		(int),
			LSA_lerror 		(int, int),
			LSA_snl_trnd 	(int);
static long LSA_hist_ld 	(long, long);

void LSA_open (void);
void LSA_close (void);

void LSA_load_months (long, long, long, long);
void LSA_load_days 	 (long, long, long, long);
void LSA_userFF(void); 


/*=============================================================================
| Function Name:                                                              |
| 	setup_LSA()                                                               |
|                                                                             |
| Description:                                                                |
| 	Prepare a table of valid warehouses for use within LSA calculation(s)     |
|                                                                             |
| Parameters:                                                                 |
| 	int	how			0 - By company                                            |
| 					1 - By branch                                             |
| 					2 - By warehouse                                          |
| 	char	*companyNumbe	Company number                                    |
| 	char	*branchNumber	Branch number                                     |
| 	char	*warehouseNumberWarehouse number                                  |
|                                                                             | 
| Global Variables Affected:                                                  |
| 	long	LSA_vld_cc [200]		Array of valid warehouse(s)                   |
|                                                                             |
| Returned Value:                                                             |
| 			0	- All OK!                                                     |
| 			Other	- Branches are in different months                        |
=============================================================================*/
int
setup_LSA (
	int		how,
 	char	*companyNumber,
	char	*branchNumber,
	char	*warehouseNumber)
{
	int		indexOffset = 0;
	long	holdDate 	= -1L;
	int		tmpDmy [3],
			workDmy [3];
	char	companyClose [6];
	char	*sptr;

	sptr = chk_env("CO_CLOSE");
	sprintf (companyClose,"%-5.5s",(sptr == (char *)0) ? "11111" : sptr);

	strcpy (esmr_rec.co_no, "ZZ");

	LSA_vld_cc [indexOffset] = 0L;
	sprintf (ccmr_rec.co_no,  "%-2.2s", companyNumber);
	sprintf (ccmr_rec.est_no, "%-2.2s", branchNumber);
	sprintf (ccmr_rec.cc_no,  "%-2.2s", warehouseNumber);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	if (cc)
	    return (0);

	while (TRUE)
	{
		if (strcmp (ccmr_rec.co_no, companyNumber))
			break;

		/*------------------------------------------
		| Check branch inv date if by company      |
		------------------------------------------*/
		if (how == 0)
		{
			if (companyClose [2] == '0')
			{
				if (strcmp (esmr_rec.co_no, ccmr_rec.co_no) ||
					strcmp (esmr_rec.est_no, ccmr_rec.est_no))
				{
					strcpy (esmr_rec.co_no,  ccmr_rec.co_no);
					strcpy (esmr_rec.est_no, ccmr_rec.est_no);
					if (find_rec (esmr, &esmr_rec, EQUAL, "r"))
						return (1);

					if (holdDate == -1L)
					{
						DateToDMY 
						(
							esmr_rec.inv_date, 
							&tmpDmy[0], &tmpDmy[1], &tmpDmy[2]
						);
						holdDate = esmr_rec.inv_date;
					}
					else
					{
						DateToDMY 
						(
							esmr_rec.inv_date, 
							&workDmy[0], &workDmy[1], &workDmy[2]
						);
						if (tmpDmy [1] != workDmy [1] ||
							tmpDmy [2] != workDmy [2])
							return (1);
					}
				}
			}
		}

	    if (strcmp (ccmr_rec.est_no, branchNumber) && how > 0)
			break;

	    if (strcmp (ccmr_rec.cc_no, warehouseNumber) && how > 1)
			break;

	    LSA_vld_cc [indexOffset] = ccmr_rec.hhcc_hash;
	    indexOffset++;
	    if (indexOffset == 200)
			break;

	    LSA_vld_cc [indexOffset] = 0L;
	    if (find_rec (ccmr, &ccmr_rec, NEXT, "r"))
			break;
	}
	return (0);
}

/*=============================================================================
| Function Name:                                                              |
| 	calc_LSA()                                                                |
|                                                                             |
| Description:				Generate specified extrapolated                   |
| 					demand values.                                            |
|                                                                             |
| Parameters:                                                                 |
| 	char	*validMethods	'A' - Standard LSA                                |
| 					        'B' - Seasonal trend                              |
| 					        'C' - Local linear                                |
| 					        'D' - Focus Forcast                               |
| 	long	hhbr_hash		Stock and                                         |
| 	long	date			Date to calculate on                              |
| 					        (Normally comm_inv_date)                          |
| 	int	bestFit	      0 - Just calculate valid data                           |
| 					  1 - As above but also calculate the error percentage.   |
|   * added, modification 06/08/1998 *                                        | 
|   int  demandMode    Mode of caculation, can be DAY or MONTH                | 
|   char demandSubType This can be either                                     | 
|                                           SALES_ONLY                        |
|                                           PLUS_TRANSFERS                    |
|                                           PLUS_LOSTSALES                    |
|                                           PLUS_PC_ISSUES                    |
|                                                                             |
| Global Variables Affected:                                                  |
| 	long	LSA_vld_cc [200]		Array of valid warehouse(s)               |
| 	int	    LSA_hist		    Number of history 'buckets'                   |
| 	char	LSA_methods [6]		Methods calculated                            |
| 					            (Subset of validMethods)                      |
| 	double	LSA_last3 [6]		Total of last 3 months                        |
| 	double	LSA_result [7] [48]	 [0] [n] Actual data                          |
| 					  [1] [n]    Standard LSA                                 |
| 					  [2] [n]    Seasonal trend                               |
| 					  [3] [n]    Local linear                                 |
| 					  [4] [n]    User or Average                              |
| 					  [5] [n]    Focus Forcast.                               |
| 					  [6] [n]    Temp for best fit                            |
| 					  [7] [n]    Exception biases                             |
| 	double	LSA_error [5] [2]	Calculated percentage error                   |
| 					            and sum of squared deviations                 |
|                                                                             |
| Returned Value:				Amount of history available                   |
=============================================================================*/
int	
calc_LSA 
(	
	char 	*validMethods, 
	long	hhbrHash,
	long	INV_DATE,
	int		bestFit,
	int		historyMonths,
	int     demandMode,  
    char    *demandType)
{
	int		i,
			internalHistory,
			createDay,		/* Stock created on this day	*/
			createMonth,	/*		 of this month			*/
			createYear;		/* 		 of this year			*/
	long	createDate;
	char	*sptr;

	strcpy (demandSubType, demandType);
	if (!LSA_environments)
	{
		/*----------------------------------------
		| Used in FF for a min percentage error. |
		----------------------------------------*/
		sptr = chk_env("LRP_PER_ERROR");
		LSA_percentError	=	(sptr == (char *)0) ? 20.00 : atof (sptr);

		/*-------------------------------
		| Working Days in Week for LRP. |
		-------------------------------*/
		sptr = chk_env("LRP_WEEK_DAY");
		LSA_WeekDay			=	(sptr == (char *)0) ? TRUE : atoi (sptr);

		/*--------------------------
		| Negative demand allowed. |
		--------------------------*/
		sptr = chk_env("LRP_DMND_NEG");
		LSA_negativeDemand	=	(sptr == (char *)0) ? FALSE : atoi (sptr);

		LSA_environments		=	TRUE;
	}
	
	/* set  globals  */
    periodType     = demandMode;

	zero_hist = TRUE;
 	 
	/*------------------------------
	| Clear out all required date. |
	------------------------------*/
	for (i = 0; i < MAX_PERIODS; i++)
	{
		monthArray [i] = i - 35;
		calculateArray [i] = 0;
		LSA_result [LSA_ACT]  [i] 	= 0;
		LSA_result [LSA_LIN]  [i] 	= 0;
		LSA_result [LSA_SLT]  [i] 	= 0;
		LSA_result [LSA_LLN]  [i] 	= 0;
		LSA_result [LSA_FF]   [i] 	= 0;
		LSA_result [LSA_TMP]  [i] 	= 0;
		LSA_result [LSA_BIAS] [i]   = 0;
	}
    
	/*-----------------------------------
	| Calculate current day, mnth, year |
	-----------------------------------*/
	DateToDMY (MonthStart (INV_DATE) - 1, &LSA_day, &LSA_mnth, &LSA_year);
		
	/*-------------------------------------------------------
	| Load all Demand data (and transfers and/or lost sales  |
	| data from ffdm )                                       |
	--------------------------------------------------------*/ 
	createDate = LSA_hist_ld (hhbrHash, INV_DATE);
	if (createDate == -1L)
		return (1);

	/*-----------------------------------------
	| Calculate first_stocked day, mnth, year |
	-----------------------------------------*/
	DateToDMY (createDate, &createDay, &createMonth, &createYear);
	createMonth--;
	if (createDay != 1)
	{
		createDay = 1;
		createMonth++;
	}
	/*-----------------------
	| Zeroise current month |
	-----------------------*/
	calculateArray [35] = 0.0;

	/*--------------------
	| Store ACTUAL data. |
	--------------------*/
	for (i = 0; i < 12; i++)
	{
		LSA_result [LSA_ACT] [i]	    = twodec (calculateArray [i]);
		LSA_result [LSA_ACT] [i + 12] 	= twodec (calculateArray [i + 12]);
		LSA_result [LSA_ACT] [i + 24] 	= twodec (calculateArray [i + 24]);
		LSA_result [LSA_ACT] [i + 36] 	= twodec (calculateArray [i + 36]);
		if (calculateArray [i] || calculateArray [i + 12] || calculateArray [i + 24])
			zero_hist = FALSE;
	}
	/*-------------------------------------------
	| How much history can we pass to the LSA?	|
	-------------------------------------------*/
	LSA_hist = (LSA_year - createYear) * 12;
	LSA_hist += LSA_mnth;
	LSA_hist -= createMonth;

	if (periodType == LRP_PASSED_DAY)
	{
		DateToDMY (INV_DATE, &LSA_day, NULL, NULL);
		LSA_hist = INV_DATE - createDate;
	}
	
	if (historyMonths > 35)
		historyMonths = 35;

	if (LSA_hist > historyMonths)
		LSA_hist = historyMonths;

	if (LSA_hist < 0)
		LSA_hist = 0;
   
	upshift (validMethods);
	LSA_methods [0] = 0;

	/*---------------------------------------------------------
	| We start with method C which is the best of a 6 month   |
	| linear and 12 month linear trend                        |
	| Calculate data and store in array 5 			  		  |
	| Base calc on 9 months history less 3 months             |
	| This will produce 3 months of extrapolated data to use  |
	| to calculate error for method C.                        |
	---------------------------------------------------------*/
	if (strchr (validMethods, 'C') != (char *) 0 && LSA_hist > 8 && !zero_hist)
	{
		int 	C_mth 	=	6;

		strcat (LSA_methods, "C");
		_calc_LSA (monthArray, calculateArray, 6, TRUE);

		LSA_lerror (C_mth, bestFit);
		LSA_error [LSA_LLN] [0] = LSA_error [LSA_LIN] [0];
		LSA_error [LSA_LLN] [1] = LSA_error [LSA_LIN] [1];
		LSA_last3 [LSA_LLN] 	= LSA_last3 [LSA_LIN];

		/*-----------------------------------------------
		| Now repeat for 15 minus 3 months if possible  |
		| to see if this gives better fit			    |
		-----------------------------------------------*/
		if (LSA_hist > 14)
		{
			for (i = 0; i < MAX_PERIODS; i++)
			   LSA_result [LSA_TMP] [i] = LSA_result [LSA_LIN] [i] = 0;

			_calc_LSA (monthArray, calculateArray, 12, TRUE);

			LSA_lerror (12, bestFit);
			if (LSA_error [LSA_LLN] [1] > LSA_error [LSA_LIN] [1])
			{
			   LSA_error [LSA_LLN] [0] 	= LSA_error [LSA_LIN] [0];
			   LSA_error [LSA_LLN] [1] 	= LSA_error [LSA_LIN] [1];
			   LSA_last3 [LSA_LLN] 		= LSA_last3 [LSA_LIN];
			   C_mth = 12;
			}
		}
		for (i = 0; i < MAX_PERIODS; i++)
			LSA_result [LSA_TMP] [i] = LSA_result [LSA_LIN] [i] = 0;

		_calc_LSA (monthArray, calculateArray, C_mth, FALSE);

		/*------------------------------------------------
		| Now move calculated data to array 3 (method C) |
		------------------------------------------------*/
		for (i = -35; i <= 12; i++)
		{
			LSA_result [LSA_LLN] [i + 35] = LSA_result [LSA_LIN] [i + 35]; 
			if (!LSA_negativeDemand)
			{
				if (LSA_result [LSA_LLN][i + 35] < 0.00)
				{
					LSA_result [LSA_LLN][i + 35] = LSA_result [LSA_LLN][i + 34];
			   		LSA_error [LSA_LLN] [1] 	 = 99;
				}
			}
		}
		LSA_last3 [LSA_LLN] = LSA_last3 [LSA_LIN];
	}

	/*--------------------------------------------------------------
	| We need at least 15 months of history to calculate method B. |
	--------------------------------------------------------------*/
	/*------------------- 
    | or 15 data points  |
	--------------------*/
	internalHistory = (LSA_hist - 3) / 12;
	if (internalHistory && !zero_hist)
	{
		/*-------------------------------------------------------
		| Calculate data and store in array 1 (method A)        |
		| Base calc on internalHistory full years of history.   |
		-------------------------------------------------------*/
		for (i = 0; i < MAX_PERIODS; i++)
		   LSA_result [LSA_TMP] [i] = LSA_result [LSA_LIN] [i] = 0;

		_calc_LSA (monthArray, calculateArray, (internalHistory * 12), FALSE);

		/*---------------------------------------------------------
		| Calculate data and store in array 5 (Temp for best fit) |
		| Base calc on internalHistory full years of history.     |
		---------------------------------------------------------*/
		if (bestFit)
			_calc_LSA (monthArray, calculateArray,(internalHistory * 12),TRUE);
	}
	/*------------------------------------------------
	| Calculate seasonal trend if method B is valid. |
	------------------------------------------------*/
	if (strchr (validMethods, 'B') != (char *) 0 && !zero_hist)
	{
		if (LSA_hist > 14)
		{
			strcat (LSA_methods, "B");
			LSA_snl_trnd (bestFit);
		}
	}

	/*---------------------------------------------------------
	| Calculate user overide or average if method D is valid. |
	---------------------------------------------------------*/
	if (strchr (validMethods, 'D') != (char *) 0)
	{  
		/*------------------------------------------------------
		| No restrictions as to how many history data is there,|
		| computationis will be zero anyway                    | 
		------------------------------------------------------*/
		strcat (LSA_methods, "D");
		LSA_userFF();
	}

	/*------------------------------------
	| Store STD LSA data in temp fields. |
	------------------------------------*/
	for (i = 0; i < MAX_PERIODS; i++)
		LSA_result [LSA_TMP] [i] = LSA_result [LSA_LIN] [i] = 0;

	/*------------------------------------------------
	| Calculate data and store in array 1 (method A) |
	| Base calc on all history available .           |
	------------------------------------------------*/
	if (!zero_hist)
		_calc_LSA (monthArray, calculateArray, LSA_hist, FALSE);

	/*---------------------------------------------------------
	| Calculate data and store in array 5 (Temp for best fit) |
	| Base calc on all history available less 3 months.       |
	| This will produce 3 months of extrapolated data to use  |
	| to calculate error for method A.                        |
	---------------------------------------------------------*/
	if (bestFit && LSA_hist > 3 && !zero_hist)
		_calc_LSA (monthArray, calculateArray, LSA_hist - 3, TRUE);

	/*----------------------------------------------
	| Calculate linear demand if method A is valid. |
	----------------------------------------------*/
	if (strchr (validMethods, 'A') != (char *) 0 && !zero_hist)
	{
		strcat (LSA_methods, "A");
		LSA_linear (bestFit);
	}
	LSA_actual ();
	return (LSA_hist);
}

/*--------------------------------------
| Calculate and store the line of best |
| fit for the actual data supplied.    |
--------------------------------------*/
static void
_calc_LSA 
(
	double	*x,
    double	*y,
	int		historyCount,
	int		bestFit
)
{
	double	M = 0.0,
			C = 0.0;
	int		i;

	/*-------------------------------------------------
	| Calculate the equation of the line of best fit. |
	-------------------------------------------------*/
	if (historyCount > 1)
		GetLinearEqn (x, y, historyCount, &M, &C);
	else
		C = y [34];
	

	/*-------------------------------------------
	| Store data for the calculated line.       |
	| If (bestFit) store in array 5 (temp)      |
	| OTHERWISE     store in array 1 (method A) |
	-------------------------------------------*/
	for (i = -35; i <= 12; i++)
	    LSA_result [(bestFit) ? LSA_TMP : LSA_LIN] [i + 35] += (M * (double) i) + C;
}

/*------------------------------------
| Calculate the equation of the line |
------------------------------------*/
static void
GetLinearEqn (x, y, cnt, M, C)
	double	x [MAX_PERIODS];
	double	y [MAX_PERIODS];
	int		cnt;
	double	*M;
	double	*C;
{
	double	x_ttl		=	0.00,
			y_ttl		=	0.00,
			x_sqr_ttl	=	0.00,
			xy_ttl		=	0.00,
			dbl_cnt		=	0.00;
	int		loop		=	0;

	x_ttl = y_ttl = x_sqr_ttl = xy_ttl = 0.0;
	for (loop = 35 - cnt; loop < 35; loop++)
	{
		x_ttl     += x [loop];
		y_ttl     += y [loop];
		x_sqr_ttl += (x [loop] * x [loop]);
		xy_ttl    += (x [loop] * y [loop]);
	}

	dbl_cnt = cnt;

	/*------------------------------
	| Solve simultaneous equation. |
	------------------------------*/
	SimSolve (y_ttl, x_ttl, dbl_cnt, xy_ttl, x_sqr_ttl, x_ttl, M, C);
}

/*=================================================================
| The purpose of this routine is to solve a simultaneous equation |
| thereby giving us the formula of a line as in y = Mx + C.	      |
=================================================================*/
static void
SimSolve 
(
	double	c1,
	double	a1,
	double	b1,
	double	c2,
	double	a2,
	double	b2,
	double	*M,
	double	*C
)
{
	double	const_diff,
			a3,
			b3,
			c3;

	/*-------------------------------------------------------
	| The first step is to eliminate b from the equation	|
	| by multiplying equation 1 by a constant such that		|
	| b1 is equal to -b2. Store this new eqn as eqn 3.		|
	-------------------------------------------------------*/
	if (b1 != 0.0)
		const_diff = -(b2 / b1);
	else
		const_diff = 0.0;
	a3 = const_diff * a1;
	b3 = -(b2);
	c3 = const_diff * c1;

	/*-------------------------------------------------------
	| If we now add eqn 2 to eqn 3 we have eliminated the b	|
	| variable from the eqn thus giving c3 = a3 * x			|
	| From this, we can deduce that M = c3 / a3				|
	-------------------------------------------------------*/
	a3 += a2;
	b3 += b2;
	c3 += c2;
	if (a3 != 0.0)
		*M = c3 / a3;
	else
		*M = 0.0;

	/*--------------------------------------------
	| Now, we can substitute our value of M into |
	| eqn 2 to ascertain the value for C.        |
	--------------------------------------------*/
	if (b1 != 0.0)
		*C = (c1 - (a1 * *M)) / b1;
	else
		*C = 0.0;
}

static void
LSA_actual ()
{
	int	i;

	for (i = 0; i < 35; i++)
		LSA_result [LSA_ACT] [i] -= LSA_result [LSA_BIAS] [i];
}

/*---------------
| Linear Demand |
---------------*/
static void
LSA_linear 
(
	int	bestFit
)
{
	int	i;

	for (i = 35; i < MAX_PERIODS; i++)
	{
		LSA_result [LSA_LIN] [i] += LSA_result [LSA_BIAS] [i];
		LSA_result [LSA_TMP] [i] += LSA_result [LSA_BIAS] [i];
	}
	LSA_lerror (LSA_hist, bestFit);
}

/*---------------
| Linear Error  |
---------------*/
static void
LSA_lerror 
(
	int 	hist,
	int		bestFit
)
{
	double	erraticDemand [MAX_HISTORY];
	double	deviation	=	0.00;
	double	bestLinear	=	0.00;
		
	double	interim;
	int		i,
			j;

	interim = LSA_result [LSA_BIAS] [32] + 
	      	  LSA_result [LSA_BIAS] [33] + 
	      	  LSA_result [LSA_BIAS] [34];

	LSA_last3 [LSA_ACT] = 	LSA_result [LSA_ACT] [32] + 
	         			 	LSA_result [LSA_ACT] [33] + 
	         			 	LSA_result [LSA_ACT] [34] - interim;

	LSA_last3 [LSA_LIN] = 	LSA_result [LSA_TMP] [32] + 
	         			 	LSA_result [LSA_TMP] [33] + 
	         			 	LSA_result [LSA_TMP] [34] - interim;

	/*----------------------------
	| Calculate percentage error |
	----------------------------*/
	if (LSA_last3 [LSA_ACT] != 0.00)
	{
		LSA_error [LSA_LIN] [0] = ((LSA_last3 [LSA_LIN] - 
			LSA_last3 [LSA_ACT]) / LSA_last3 [LSA_ACT]) * 100;
	}
	else
	{
		if (LSA_last3 [LSA_LIN] != 0.00)
			LSA_error [LSA_LIN] [0] = 100;
		else
			LSA_error [LSA_LIN] [0] = 0;
	}

	LSA_error [LSA_LIN] [1] = 0;
	for (i = (35 - LSA_hist), j = 0; i < MAX_HISTORY; i++, j++)
	{
		bestLinear	=	LSA_result [(bestFit) ? LSA_TMP : LSA_LIN] [i];
		if (LSA_result [LSA_ACT][i] == 0.00 || bestLinear == 0.00)
			deviation = 0.00;
		else
		{
			deviation	=	LSA_result [LSA_ACT] [i] / bestLinear;
			deviation	*=	deviation;
		}
		erraticDemand [j] = deviation;
	}
	deviation = 0.00;
	for (i = 0; i < LSA_hist; i++)
		deviation += erraticDemand [i];

	if (LSA_hist == 0.00)
		deviation =	(double) 0.00;
	else
		deviation /= LSA_hist;

	LSA_error [LSA_LIN] [1] = deviation;
}

/*----------------
| Seasonal Trend |
----------------*/
static void
LSA_snl_trnd 
(
	int	bestFit
)
{
	double	interim,
			delta_v1,
			delta_v,
			deviation = 0.00;
	int		cnt,
			i,
			j,
			data_start = 35 - LSA_hist;
	double	erraticDemand [MAX_HISTORY];

	/*----------------------------------------------------------------------
	| The seasonal trend forecast is the weighted average of the           |
	| actual data 25, 24, 23, 13, 12 and 11 months earlier, with weight    |
    | factors -.1, -.8, -.1, .2, 1.6 and .2 respectively. If we have no    |
    | actual data 25, 24 and/or 23 months earlier, we use 13, 12 and 11    |
	| months ago instead, so effectively the weights for these  then       |
	| become .1, .8 and/or .1 respectively. This has the effect            |
	| that the seasonal forecast is calculated as last years actual        |
	| plus the difference between last year and the year before            |
	----------------------------------------------------------------------*/
	for (cnt = 0; cnt < MAX_PERIODS; cnt++)
	    	LSA_result [LSA_SLT] [cnt] = 0.0; 

	for (cnt = data_start; cnt < 35; cnt++)
	{
	    delta_v = (LSA_result [LSA_ACT] [cnt] + LSA_result [LSA_BIAS] [cnt]);
	    LSA_result [LSA_SLT] [cnt + 11] += delta_v * .2;
	    LSA_result [LSA_SLT] [cnt + 12] += delta_v * 1.6;
	    if (cnt < 34)
	    	LSA_result [LSA_SLT] [cnt + 13] += delta_v * .2;
	    if ((cnt - data_start) < 12)
	    {
	    	LSA_result [LSA_SLT] [cnt + 11] -= delta_v * .1;
	    	LSA_result [LSA_SLT] [cnt + 12] -= delta_v * .8;
	    	LSA_result [LSA_SLT] [cnt + 13] -= delta_v * .1;
	    }
	    if (cnt < 23)
	    {
	    	LSA_result [LSA_SLT] [cnt + 23] -= delta_v * .1;
	    	LSA_result [LSA_SLT] [cnt + 24] -= delta_v * .8;
			if (cnt < 22)
	    		LSA_result [LSA_SLT] [cnt + 25] -= delta_v * .1;
	    }
		/*--------------------------------------------------------------------
		| At data_start and cnt = 22 or 34 we do not have data for the       |
		| preceding or following month so we add in the deviation for those  |
		| months again with the required weightings as a correction          |
		--------------------------------------------------------------------*/
	    if (cnt == data_start)
	    {
	    	LSA_result [LSA_SLT] [cnt + 12] += delta_v * .2;
	    	LSA_result [LSA_SLT] [cnt + 24] -= delta_v * .1;
	    }
	    if (cnt == 22)
	    	LSA_result [LSA_SLT] [cnt + 24] -= delta_v * .1;
	    if (cnt == 34)
	    	LSA_result [LSA_SLT] [cnt + 12] += delta_v * .2;
	}

	if (bestFit)
	{
	    interim = LSA_result [LSA_BIAS] [32] + 
		      	  LSA_result [LSA_BIAS] [33] + 
		      	  LSA_result [LSA_BIAS] [34];

	    LSA_last3 [LSA_ACT] = 	LSA_result [LSA_ACT] [32] + 
			   	 			 	LSA_result [LSA_ACT] [33] + 
			   	 			 	LSA_result [LSA_ACT] [34] - interim;

	    LSA_last3 [LSA_SLT]		= 0;
	    LSA_error [LSA_SLT] [1] = 0;
	    for (cnt = 0; cnt < 3; cnt++)
	    {
			delta_v = 0;
			delta_v1 = (LSA_result [LSA_ACT] [cnt + 19] + 
			   			LSA_result [LSA_BIAS] [cnt + 19]);

			delta_v += delta_v1 * .2;
			if (cnt + 7 >= data_start)
				delta_v1 = (LSA_result [LSA_ACT] [cnt + 7] + 
							LSA_result [LSA_BIAS] [cnt + 7]);

			delta_v -= delta_v1 * .1;
			delta_v1 = (LSA_result [LSA_ACT] [cnt + 20] + 
			            LSA_result [LSA_BIAS] [cnt + 20]);

			delta_v += delta_v1 * 1.6;
			if (cnt + 8 >= data_start)
		    	delta_v1 = (LSA_result [LSA_ACT] [cnt + 8] + 
			                LSA_result [LSA_BIAS] [cnt + 8]);

			delta_v -= delta_v1 * .8;
			if (cnt < 2)
		    	delta_v1 = (LSA_result [LSA_ACT] [cnt + 21] + 
			                LSA_result [LSA_BIAS] [cnt + 21]);
			else
				delta_v1 = (LSA_result [LSA_ACT] [cnt + 20] + 
			                LSA_result [LSA_BIAS] [cnt + 20]);

			delta_v += delta_v1 * .2;
			if (cnt + 9 >= data_start)
		    	delta_v1 = (LSA_result [LSA_ACT] [cnt + 9] + 
			                LSA_result [LSA_BIAS] [cnt + 9]);

			delta_v -= delta_v1 * .1;
			LSA_last3 [LSA_SLT] += delta_v;
	    }
		LSA_error [LSA_SLT] [1] = 0;
		for (i = (35 - LSA_hist), j = 0; i < MAX_HISTORY; i++, j++)
		{
			if (LSA_result [LSA_ACT][i] == 0.00 || 
				LSA_result [LSA_SLT][i] == 0.00)
				deviation =	(double) 0;
			else
			{
				deviation	=	LSA_result [LSA_ACT] [i] / 
								LSA_result [LSA_SLT] [i];
				deviation	*=	deviation;
			}
			erraticDemand [j] = deviation;
		}
		deviation = 0.00;
		for (i = 0; i < LSA_hist; i++)
			deviation += erraticDemand [i];

		if (LSA_hist == 0.00)
			deviation =	(double) 0.00;
		else
			deviation /= LSA_hist;

		LSA_error [LSA_SLT] [1] = deviation;

	    if (LSA_last3 [LSA_ACT] != 0.00)
	    {
			LSA_error [LSA_SLT] [0] = ((LSA_last3 [LSA_SLT] - 
								      LSA_last3 [LSA_ACT]) / 
								      LSA_last3 [LSA_ACT]) * 100;
	    }
	    else
	    {
			if (LSA_last3 [LSA_SLT] != 0.00)
				LSA_error [LSA_SLT] [0] = 100;
			else
				LSA_error [LSA_SLT] [0] = 0;
	    }
	}

	for (i = 35; i < MAX_PERIODS; i++)
		LSA_result [LSA_SLT] [i] += LSA_result [LSA_BIAS] [i];
}

/*----------------------------
| Load history and calculate |
| how much history we have.  |
----------------------------*/
static long
LSA_hist_ld 
(
	long	hhbrHash,
	long	INV_DATE
)
{
	long	LSA_StartDate;
	int		i;
	int		indexOffset = 0;
	long	oldestCreateDate	=	TodaysDate ();

	LSA_oldestDate 	=	oldestCreateDate;

	if (periodType == LRP_PASSED_MONTH)
		CalcLSADates (INV_DATE);

	/*---------------------------------
	| Load history for specified item |
	| from all valid warehouses.      |
	---------------------------------*/
	while (indexOffset < 200 && LSA_vld_cc [indexOffset])
	{
		if (periodType == LRP_PASSED_DAY)
		{
			LSA_StartDate	=	INV_DATE;
			LSA_StartDate	-=	MAX_HISTORY;
			LSA_load_days
			(
				hhbrHash,
				LSA_vld_cc [indexOffset],
				LSA_StartDate,
				INV_DATE + 12
			);
		}	
		else  /* if Not LRP_PASSED_DAY */
		{
			for (i = 0; i < MAX_PERIODS; i++)
				LSA_date_rec [i].QtySold = 0.0;

			LSA_load_months
			(
				hhbrHash,
				LSA_vld_cc [indexOffset],
				LSA_date_rec [0].StartDate,
				LSA_date_rec [MAX_PERIODS - 1].EndDate
			);
		}
	    indexOffset++;
	}

	oldestCreateDate	=	LSA_oldestDate;
	return (oldestCreateDate);
}

#ifdef	LSA_DEBUG
DSP_VLS(str)
 char	*str;
{
	int	c,
		i,
		x;

	swide ();
	print_at (0, 0, "%R %s ", str);
	do
	{
		c = getkey ();
		switch (c)
		{
		case	'0':
		case	'1':
		case	'2':
		case	'3':
			c -= '0';
			break;

		default:
			continue;
		}
		for (x = 0; c < 4 && x < 7; x++)
		{
			for (i = 0; i < 12; i++)
				print_at (x+1, i * 10, "%9.2f", LSA_result [x] [i + (c * 12)]);
		}
	} while (c != 'Z');
}
#endif	/*LSA_DEBUG*/

void LSA_open(void)
{
   open_rec (ffdm, ffdm_list, ffdm_no_fields, "ffdm_id_no3");
}

void LSA_close(void)
{
   abc_fclose (ffdm);
}

/*===============================================
| Calculate start and end dates for each month. |
===============================================*/
void
CalcLSADates 
(
	long	StartDate
)
{
	int		dmy [3];
	int		i;

	DateToDMY (MonthStart (StartDate), &dmy[0],&dmy[1],&dmy[2]);
	dmy [2]	-=3;
	dmy [1]	+=1;
	StartDate = DMYToDate (dmy[0], dmy[1], dmy[2]);
   
	for (i = 0; i < MAX_PERIODS + 2; i++)
	{
		LSA_date_rec [i].StartDate	=	MonthStart (StartDate);
		LSA_date_rec [i].EndDate	=	MonthEnd (StartDate);
		StartDate	=	MonthEnd (StartDate) + 1;
	}
}

/*==============================
| Load demand values by month. |
==============================*/
void
LSA_load_months
(
	long	hhbrHash,
	long	hhccHash,
	long	startDate,
	long	endDate
)
{
	int		i;

	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	ffdm_rec.date		=	startDate;

	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc  &&  
		   (ffdm_rec.hhbr_hash	==	hhbrHash) &&
		   (ffdm_rec.hhcc_hash	==	hhccHash) &&
		   (ffdm_rec.date 		<=  endDate))
	{
		if (ffdm_rec.qty == 0.0)
		{
			cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
			continue;
		}
		if ( ffdm_rec.date < LSA_oldestDate)
			LSA_oldestDate	=	ffdm_rec.date;

		switch (ffdm_rec.type [0])
		{
		case	'1':
			for (i = 0; i < MAX_PERIODS; i++)
			{
				if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
					ffdm_rec.date <= LSA_date_rec [i].EndDate)
					calculateArray [i]		+= ffdm_rec.qty;
			}
			break;
		case	'2':
			for (i = MAX_HISTORY; i < MAX_PERIODS; i++)
			{
				if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
					ffdm_rec.date <= LSA_date_rec [i].EndDate)
				{
					calculateArray [i]			+=	ffdm_rec.qty;
					LSA_result [LSA_BIAS] [i] 	+=  ffdm_rec.qty;
				}
			}
			break;

		case	'3':
			for (i = 0; i < MAX_PERIODS; i++)
			{
				if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
					ffdm_rec.date <= LSA_date_rec [i].EndDate)
				{
					calculateArray [i]			+=	ffdm_rec.qty;
					LSA_result [LSA_BIAS] [i] 	+=  ffdm_rec.qty;
				}
			}
			break;
		case	'4':
			if (strchr (demandSubType, PLUS_TRANSFERS) != (char *) 0)
			{
				for (i = 0; i < MAX_HISTORY; i++)
				{
					if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
						ffdm_rec.date <= LSA_date_rec [i].EndDate)
					{
						calculateArray [i]		+= ffdm_rec.qty;
					}
				}
			}
			break;
		case	'5':
			if (strchr (demandSubType, PLUS_LOSTSALES) != (char *) 0)
			{
				for (i = 0; i < MAX_HISTORY; i++)
				{
					if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
						ffdm_rec.date <= LSA_date_rec [i].EndDate)
					{
						calculateArray [i]		+= ffdm_rec.qty;
					}
				}
			}
			break;
		case	'6':
			if (strchr (demandSubType, PLUS_PC_ISSUES) != (char *) 0)
			{
				for (i = 0; i < MAX_HISTORY; i++)
				{
					if (ffdm_rec.date >= LSA_date_rec [i].StartDate &&
						ffdm_rec.date <= LSA_date_rec [i].EndDate)
					{
						calculateArray [i]		+= ffdm_rec.qty;
					}
				}
			}
			break;
		}
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
}
/*=============================
| Load demand values by days. |
=============================*/
void
LSA_load_days
(
	long	hhbrHash,
	long	hhccHash,
	long	startDate,
	long	endDate
)
{
	int		i;

	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	ffdm_rec.date		=	startDate;

	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc  &&  
		   ffdm_rec.hhbr_hash	==	hhbrHash &&
		   ffdm_rec.hhcc_hash	==	hhccHash &&
		   ffdm_rec.date 		<=  endDate)
	{
		if ( ffdm_rec.date < LSA_oldestDate)
			LSA_oldestDate	=	ffdm_rec.date;

		i = ffdm_rec.date - startDate;
	
		switch (ffdm_rec.type [0])
		{
		case	'1':
			if (i < MAX_HISTORY)
				calculateArray [i]		+= ffdm_rec.qty;
			break;
		case	'2':
			if (i > MAX_HISTORY)
			{
				calculateArray [i]			+= ffdm_rec.qty;
				LSA_result [LSA_BIAS] [i] 	+=  ffdm_rec.qty;
			}
			break;

		case	'3':
			if (i < MAX_HISTORY)
			{
				LSA_result [LSA_BIAS] [i] 	+=  ffdm_rec.qty;
				calculateArray [i]			+=	ffdm_rec.qty * -1;
			}
			break;
		case	'4':
			if (strchr (demandSubType, PLUS_TRANSFERS) != (char *) 0)
			{
				if (i < MAX_HISTORY)
					calculateArray [i]		+= ffdm_rec.qty;
			}
			break;
		case	'5':
			if (strchr (demandSubType, PLUS_LOSTSALES) != (char *) 0)
			{
				if (i < MAX_HISTORY)
					calculateArray [i]		+= ffdm_rec.qty;
			}
			break;
		case	'6':
			if (strchr (demandSubType, PLUS_PC_ISSUES) != (char *) 0)
			{
				if (i < MAX_HISTORY)
					calculateArray [i]		+= ffdm_rec.qty;
			}
			break;
		}
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
}

/*=====================================================+
| Compute user averages based on document called Focus |
| Forecasting of the Logistic Softwares                |
| Compute all, even if LSA_history is not enough       |
|                                                      |
| Globals affected:                                    |
|                LSA_result [LSA_USR]                  |
======================================================*/
void LSA_userFF (void)
{
	float	error [10];
	float	result [10];
	float	Calc1		=	0.0,
			Calc2		=	0.0,
			Calc3		=	0.0;
	float	lastMonth	=	0.00;

	float	min_err = 	10000.00;
	float	err_pc	= 	0.00;
	int		Formula;

  	int 	i, 
			j; 
	int		limits [10]      = {3, 6, 9, 12, 15, 18, 21, 12, 15, 6};
							 
	int		SelectedMethod	   = 3;   /* selected formula */
	const 	int 	start_mo   = 33;  /* start of data history */

	double	interim		=	0.00,
			deviation 	=	0.00;
	double	erraticDemand [MAX_HISTORY];

	for (i = 0; i < 10; i++)
	{
		error  [i]	=	0.00;
		result [i]	=	0.00;
	}
	/*-----------------------------------------------------------------------
	| Formula used :                    |                INDEX              |
	|                                   |    A   - Actual demand for period |
	|  1 - (A TY 4 Thru 6 / 3           |    LY  - Last Year                |
	|  2 - (A TY 1 Thru 6 / 6           |    TY  - This Year                |
	|  3 - (A LY 10 Thru A TY 6 / 9     |    YBL - Year Before Last         |
	|  4 - (A LY 7 Thru A TY 6 / 12     |    1 to 12 - Jan to Dec           |
	|  5 - (A LY 4 Thru A TY 6 / 15     |                                   |
	|  6 - (A LY 1 Thru A TY 6 / 18     |                                   |
	|  7 - (A LBL 10 Thru A TY 6 / 21   |  (Also see PowerPoint demo        |
	|  8 - (A LY 7 Thru 9) / 3          |                                   |
	|  9 - (A LY 7 Thru 9) / 3 *        |                                   |
	|      (A TY 4 Thru 6) +            |                                   |
	|      (A LY 4 Thru 6)              |                                   |
	| 10 - ((A TY 4 Thru 6 / 3 +        |                                   |
    |      ( A TY 1 Thru 6) / 6) / 2    |                                   |
	-----------------------------------------------------------------------*/
	/*----------------
	| formula 1 to 7 | 
	----------------*/
	for (Formula = 0; Formula < 7; Formula++)
	{
		Calc1 = 0.0;

		for (i = start_mo, j = 0; j < limits [Formula]; i--, j++)
			Calc1 += LSA_result [LSA_ACT] [i];	

		result [Formula] = twodec (Calc1 / (float) limits [Formula]);
	}
  
	/*---------------
	| formula 8     |
	---------------*/
	Calc1 = 0;
	for (i = start_mo - 13, j = 0; j < 3; i++, j++)
		Calc1 += LSA_result [LSA_ACT] [i];

	result [7] = twodec (Calc1 / 3);

	/*-----------
	| formula 9 |
	-----------*/
	Calc1	=	0.0;
	for (i = start_mo - 13, j = 0; j < 3; j++, i++)
		Calc1 += LSA_result [LSA_ACT] [i];   

	Calc2	=	0.0;
	for (i = start_mo - 2, j = 0; j < 3; i--, j++)
	 	Calc2 +=  LSA_result [LSA_ACT] [i];

	Calc3	=	0.0;
	for (i = start_mo - 14, j = 0; j < 3; i--, j++)
	 	Calc3 +=  LSA_result [LSA_ACT] [i];
  
	if (Calc3 == 0.0)
		result [8] = twodec (Calc1 / 3);
	else
		result [8] = twodec (((Calc1 / 3) * (Calc2 / Calc3)));
 
	/*------------
	| formula 10 |
	------------*/
	Calc1	=	0.0,
	Calc2	=	0.0;
	for (i = start_mo, j = 0; j < 6; j++, i--)
	{
		/* 3 months ago */
		if (j < 3)
			Calc1 += LSA_result [LSA_ACT] [i];

		/* 6 months ago */
		Calc2 += LSA_result [LSA_ACT] [i];
	}
	result [9] = twodec (((Calc1 / 3) + (Calc2 / 6)) / 2);

	interim = LSA_result [LSA_BIAS] [32] + 
	      	  LSA_result [LSA_BIAS] [33] + 
	      	  LSA_result [LSA_BIAS] [34];

	LSA_last3 [LSA_ACT] = 	LSA_result [LSA_ACT] [32] + 
	         			 	LSA_result [LSA_ACT] [33] + 
	         			 	LSA_result [LSA_ACT] [34] - interim;
	
  	/*-------------------------------------------------- 
  	| compute for error and determine the lowest error  |
  	---------------------------------------------------*/
	lastMonth	=	LSA_result [LSA_ACT] [34];	
	if (lastMonth == 0.00)
		lastMonth	=	LSA_last3 [LSA_ACT] / 3;

	for (i = 0; i < 10; i++)
	{
		if (lastMonth != 0.00)
		{
			error [i] = result [i];
			error [i] -= lastMonth;
			error [i] /= lastMonth;
			error [i] *= 100.00;
		}
		else
		{
			if (result [i] != 0.00)
				error [i] = 100.00;
			else
				error [i] = 0.00;
		}
	}
	SelectedMethod = 3;
	for (i = 0; i < 10; i++)
	{
		err_pc	=	fabs (error [i]); 
		if (min_err > err_pc)
		{
			SelectedMethod = i;
			min_err = err_pc;
		}
	}
	/*-----------------------------------------------------------------------
	| If error percentage is > defined percent and the last three months in |
	| not zero then use the last three months to calculate.                 |
	-----------------------------------------------------------------------*/
	if (fabs (error [SelectedMethod] ) > LSA_percentError &&
		LSA_last3 [LSA_ACT] > 0.00)
		result [SelectedMethod] = LSA_last3 [LSA_ACT] / 3;

	/*-------------------------------------------------------
	| Answer still 0.00 well use average of last 12 months. |
	-------------------------------------------------------*/
	if (fabs (result [SelectedMethod]) == 0.0)
		SelectedMethod	=	3;

	/*-------------------------
	| Initialize LSA_FF       |
	-------------------------*/
	for (i = 0; i < MAX_HISTORY; i++)
		LSA_result [LSA_FF] [i] = result [SelectedMethod];

	for (i = MAX_HISTORY; i < MAX_PERIODS; i++)
	{
		/*--------------------------------------------
		| The following is required for forced date. |
		--------------------------------------------*/
		if (LSA_result [LSA_ACT] [i] == 0.0)
			LSA_result [LSA_FF] [i] = result [SelectedMethod];
		else
		{
			LSA_result [LSA_FF] [i] = LSA_result [LSA_ACT] [i];
			error [SelectedMethod] = 0.00;
		}
	}
	
	LSA_error [LSA_FF] [0] = error [SelectedMethod];
	LSA_error [LSA_FF] [1] = 0;
	for (i = (35 - LSA_hist), j = 0; i < MAX_HISTORY; i++, j++)
	{
		if (LSA_result [LSA_ACT][i] == 0.00 || 
			LSA_result [LSA_FF] [i] == 0.00)
			deviation =	(double) 0.00;
		else
		{
			deviation	=	LSA_result [LSA_ACT] [i] / 
							LSA_result [LSA_FF] [i];
			deviation	*=	deviation;
		}
		erraticDemand [j] = deviation;
	}
	deviation = 0.00;
	for (i = 0; i < LSA_hist; i++)
		deviation += erraticDemand [i];

	if (LSA_hist == 0.00)
		deviation =	(double) 0.00;
	else
		deviation /= LSA_hist;

	LSA_error [LSA_FF] [1] = deviation;

	LSA_last3 [LSA_ACT] = 	LSA_result [LSA_ACT] [32] + 
	         			 	LSA_result [LSA_ACT] [33] + 
	         			 	LSA_result [LSA_ACT] [34];

	LSA_last3 [LSA_FF]	 = LSA_last3 [LSA_ACT];
}

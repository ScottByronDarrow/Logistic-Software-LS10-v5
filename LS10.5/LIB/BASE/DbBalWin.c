/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: DbBalWin.c,v 5.0 2001/06/19 06:59:11 cha Exp $
|  Program Name  : (DbBalWin.c)  
|  Program Desc  : (Customer Customer Credit status window.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 26th Mar 2001    |
|---------------------------------------------------------------------|
| $Log: DbBalWin.c,v $
| Revision 5.0  2001/06/19 06:59:11  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/27 06:54:03  scott
| Updated to change arguments passed to DbBalWin to avoid usage of read_comm ()
|
| Revision 4.1  2001/03/26 10:18:30  scott
| Added new function DbBalWin to replace program db_bal_win
|
=====================================================================*/
#include	<std_decs.h>
#include	<DbBalWin.h>

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
	static	int		currMonth		=	0;

	static	double	mtdSales = 0.00,
					ytdSales = 0.00;

	static	const	char	
			*cumr	=	"_cumr_DbBalWin",
			*cumr2	=	"_cumr2_DbBalWin",
			*cusa	=	"_cusa_DbBalWin";
					
	struct	cumrRecord	cumrRec;
	struct	cumrRecord	cumr2Rec;
	struct	cusaRecord	cusaRec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	19

	static	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_payment_flag"},
		{"cumr_contact_name"},
		{"cumr_phone_no"},
		{"cumr_stop_credit"},
		{"cumr_credit_limit"},
		{"cumr_credit_ref"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_ho_dbt_hash"},
		{"cumr_bo_current"},
		{"cumr_bo_per1"},
		{"cumr_bo_per2"},
		{"cumr_bo_per3"},
		{"cumr_bo_per4"},
		{"cumr_bo_fwd"},
		{"cumr_stat_flag"},
	};
	struct cumrRecord
	{
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_name [41];
		int		payment_flag;
		char	contact_name [21];
		char	phone_no [16];
		char	stop_credit [2];
		Money	credit_limit;
		char	credit_ref [21];
		char	area_code [3];
		char	sman_code [3];
		long	ho_dbt_hash;
		Money	bo_current;
		Money	bo_per1;
		Money	bo_per2;
		Money	bo_per3;
		Money	bo_per4;
		Money	bo_fwd;
		char	stat_flag [2];
	};

	/*==========================================+
	 | Customer 24 Month Sales Analysis Record. |
	 +==========================================*/
#define	CUSA_NO_FIELDS	15

	static	struct dbview	cusa_list [CUSA_NO_FIELDS] =
	{
		{"cusa_hhcu_hash"},
		{"cusa_year"},
		{"cusa_val1"},
		{"cusa_val2"},
		{"cusa_val3"},
		{"cusa_val4"},
		{"cusa_val5"},
		{"cusa_val6"},
		{"cusa_val7"},
		{"cusa_val8"},
		{"cusa_val9"},
		{"cusa_val10"},
		{"cusa_val11"},
		{"cusa_val12"},
		{"cusa_stat_flag"}
	};

	struct cusaRecord
	{
		long	hhcu_hash;
		char	year [2];
		Money	val1;
		Money	val2;
		Money	val3;
		Money	val4;
		Money	val5;
		Money	val6;
		Money	val7;
		Money	val8;
		Money	val9;
		Money	val10;
		Money	val11;
		Money	val12;
		char	stat_flag [2];
	};

	static	Money	*cumr_balance	=	&cumrRec.bo_current;
	static	Money	*cumr2_balance	=	&cumr2Rec.bo_current;
	static	Money	*cusa_val		=	&cusaRec.val1;

	static void		TableSetup 		 (void),
					TableTeardown 	 (void);

/*=====================================================================
| Local Functino Prototypes.
=====================================================================*/
static	void 	GetMtdYtd 		(int, long, int, long);
static	float 	CalcYtd 		(int);

void
DbBalWin (
	long	hhcuHash,
	int		fiscalPeriod,
	long	dbtDate)
{
	double	currBalance = 0.00,
			balOutstand = 0.00;

	double	cumr_bo [6]	=	{0.0,0.0,0.0,0.0,0.0,0.0};

	char	workString [131],
			specialCredit [4];

	int		err;

	TableSetup ();

	Dsp_open (1,4,7);

	cumrRec.hhcu_hash	=	hhcuHash;
	if (!find_rec (cumr, &cumrRec, COMPARISON, "r"))
	{
		int		i;

		Dsp_saverec ("            CUSTOMER CREDIT STATUS WINDOW DISPLAY              ");
		Dsp_saverec ("");
		Dsp_saverec ("[EDIT/END]");

		/*---------------------------
		| Total head office customer. |
		---------------------------*/
		for (i = 0; i < 6; i++)
			cumr_bo [i] = cumr_balance [i];

		/*-------------------------------------------
		| Get MTD and YTD for head office customer. |
		-------------------------------------------*/
		GetMtdYtd (fiscalPeriod, dbtDate, TRUE, cumrRec.hhcu_hash);

		/*--------------------------
		| Total all child customer. |
		--------------------------*/
		cumr2Rec.ho_dbt_hash	=	cumrRec.hhcu_hash;
		err = find_rec (cumr2, &cumr2Rec, GTEQ, "r");
		while (!err && cumrRec.hhcu_hash == cumr2Rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				cumr_bo [i] += cumr2_balance [i];

			/*-------------------------------------
			| Get MTD and YTD for child customer. |
			-------------------------------------*/
			GetMtdYtd (fiscalPeriod, dbtDate, FALSE, cumr2Rec.hhcu_hash);
			
			err = find_rec (cumr2, &cumr2Rec, NEXT, "r");
		}
		currBalance  = 	cumr_bo [0] + cumr_bo [5];
		balOutstand  = 	cumr_bo [1] + cumr_bo [2] + 
						cumr_bo [3] + cumr_bo [4];

		strcpy (specialCredit,
				 (cumrRec.stop_credit[0] == 'Y') ? ML ("YES") : ML ("NO."));

		sprintf (workString ,"%11.11s : %6.6s (%40.40s)",
				ML ("Customer No"), cumrRec.dbt_no, cumrRec.dbt_name);
		Dsp_saverec (workString);

		sprintf (workString ,"%11.11s : %10.2f      ^E %11.11s  : %12.2f",
				ML ("Sales MTD  "), DOLLARS (mtdSales), 
				ML ("Sales YTD  "), DOLLARS (ytdSales));
		Dsp_saverec (workString);
	
		sprintf (workString ,"%11.11s : %2.2s              ^E %s     :%2.2s",
				ML ("Salesman   "), cumrRec.sman_code, 
				ML ("Area    "),    cumrRec.area_code);
		Dsp_saverec (workString);

		sprintf (workString ,"%11.11s : A%1d              ^E %s  :%20.20s",
				ML ("Credit Rate"), cumrRec.payment_flag, 
				ML ("Credit Ref "), cumrRec.credit_ref);
		Dsp_saverec (workString);
	
		sprintf (workString ,"%11.11s : %3.3s             ^E %s : %12.2f",
				ML ("Stop credit"), specialCredit, 
				ML ("Credit limit"), DOLLARS (cumrRec.credit_limit));
		Dsp_saverec (workString);
	
		sprintf (workString ,"%11.11s : %15.15s ^E %s : %20.20s ",
				ML ("Phone No   "), cumrRec.phone_no, 
				ML ("Contact Name"),cumrRec.contact_name);
		Dsp_saverec (workString);
	
		sprintf (workString ,"%11.11s : %10.2f      ^E %s : %12.2f",
				ML ("Current Amt"),  DOLLARS (currBalance), 
				ML ("Overdue Amt "), DOLLARS (balOutstand));
		Dsp_saverec (workString);
	}
	Dsp_srch	 	();
	Dsp_close	 	();
	TableTeardown	();
}

static void
TableSetup (void)
{
	/*
	 *	Open all the necessary tables et al
	 */
	static int	done_this_before = FALSE;

	if (!done_this_before)
	{
		done_this_before = TRUE;

		abc_alias (cumr, "cumr");
		abc_alias (cumr2, "cumr");
		abc_alias (cusa, "cusa");
	}
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
}

static void
TableTeardown (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cusa);
}

static	void
GetMtdYtd (
	int		fiscalPeriod,
	long	dbtDate,
	int		clearTotal,
	long	hhcuHash)
{
	int		err;
	if (clearTotal)
	{
		DateToDMY (dbtDate, NULL, &currMonth, NULL);
		currMonth--;

		/*-----------------------------------
		| Load Year To Date Sales from incc	|
		-----------------------------------*/
		mtdSales = 0.00;
		ytdSales = 0.00;
	}

	cusaRec.hhcu_hash = hhcuHash;
	strcpy (cusaRec.year,"C");
	err = find_rec (cusa, &cusaRec, COMPARISON,"r");
	if (!err)
	{
		mtdSales += cusa_val [currMonth];
		ytdSales += CalcYtd (fiscalPeriod);
	}
}

/*======================
| Calculate Ytd Sales. |
======================*/
static	float
CalcYtd (
	int		fiscalPeriod)
{
	int		i;
	float	ytd = 0.00;

	/*-------------------
	| no fiscial set up	|
	-------------------*/
	if (!fiscalPeriod)
	{
		/*-----------------------------------
		| sum to current month (feb == 1)	|
		-----------------------------------*/
		for (i = 0;i <= currMonth;i++)
			ytd += (float) cusa_val [i];

		return (ytd);
	}

	/*------------------------------------------------------------
	| need to sum from fiscal to dec, then jan to current month. |
	------------------------------------------------------------*/
	if (currMonth < fiscalPeriod)
	{
		for (i = fiscalPeriod; i < 12; i++)
			ytd += (float) cusa_val [i];

		for (i = 0;i <= currMonth;i++)
			ytd += (float) cusa_val [i];

		return (ytd);
	}

	for (i = fiscalPeriod; i <= currMonth; i++)
		ytd += (float) cusa_val [i];

	return (ytd);
}

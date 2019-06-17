/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_pscreat.c,v 5.9 2002/11/28 04:09:51 scott Exp $
|  Program Name  : (so_pscreat.c) 
|  Program Desc  : (Background Process to Creat Packing Slips)
|
|  Author        : Scott Darrow.   | Date Written  : 28/06/1988       |
|---------------------------------------------------------------------|
| $Log: so_pscreat.c,v $
| Revision 5.9  2002/11/28 04:09:51  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.8  2002/01/18 04:24:03  scott
| Updated from testing
|
| Revision 5.7  2001/12/11 03:04:29  scott
| Open of pcwo missing
|
| Revision 5.6  2001/12/11 02:12:38  cha
| Updated to store the original WorksOrder into coln_hhwo_hash.
|
| Revision 5.5  2001/10/24 08:47:54  cha
| Updated to ensure that output to pipe is
| properly passed.
|
| Revision 5.4  2001/08/23 11:46:30  scott
| Updated from scotts machine
|
| Revision 5.3  2001/08/13 06:11:13  cha
| Updated to change cohr_date_create to comm_dbt_date. 
| Use module date instead of system date.
|
| Revision 5.2  2001/08/09 09:21:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:44  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_pscreat.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_pscreat/so_pscreat.c,v 5.9 2002/11/28 04:09:51 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<alarm_time.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include	<CustomerService.h>
#include	"schema"

#define		PACK_PC		(!strcmp (sobg_rec.type, "PC"))
#define		PACK_PA		(!strcmp (sobg_rec.type, "PA"))
#define		BAD_HASH	(sobg_rec.hash == 0L)
#define		BONUS		(coln_rec.bonus_flag [0] == 'Y')

#define		NOTAX		(sohr_rec.tax_code [0] == 'A' || \
                  	  	  sohr_rec.tax_code [0] == 'B')

#define		RELEASE		(envBoRtype [0] == 'R' || envBoRtype [0] == 'r')
#define		SUR_CHARGE	(sohr_rec.sohr_new [0] != 'N' && \
                         (cumr_rec.sur_flag [0] == 'Y' || \
                           cumr_rec.sur_flag [0] == 'y'))

#define		NON_STOCK	(inmr_rec.inmr_class [0] == 'Z')
#define		FULL_SUPPLY	(sohr_rec.full_supply [0] == 'Y')

#define		FGN_CURR	(envDbMcurr && \
						  strcmp (cumr_rec.curr_code, envCurrCode))

#define		NEW_SO		(sohr_rec.sohr_new [0] == 'N' && envSoFreightBord)

#define		CASH_INV	(cumr_rec.cash_flag [0] == 'Y')

#define		BY_BRANCH	1
#define		BY_DEPART	2

FILE	*printerOutputFile = NULL;

	struct	ccmrRecord	ccmr_rec;
	struct	comrRecord	comr_rec;
	struct	cumrRecord	cumr_rec;
	struct	esmrRecord	esmr_rec;
	struct	sohrRecord	sohr_rec;
	struct	solnRecord	soln_rec;
	struct	cohrRecord	cohr_rec;
	struct	cohrRecord	cohr2_rec;
	struct	colnRecord	coln_rec;
	struct	trshRecord	trsh_rec;
	struct	inmrRecord	inmr_rec;
	struct	cnchRecord	cnch_rec;
	struct	pocrRecord	pocr_rec;
	struct	sobgRecord	sobg_rec;
	struct	sobgRecord	sobg2_rec;
	struct	sonsRecord	sons_rec;
	struct	inlaRecord	inla_rec;
	struct	cudpRecord	cudp_rec;
	struct	skniRecord	skni_rec;
	struct  pcwoRecord  pcwo_rec;

	static char	*data	= "data",
				*cohr2	= "cohr2",
				*sobg2	= "sobg2";
		
	int		printerNumber 		= 0,
			contractPrice 		= FALSE,
			envWoAllowed 		= 0,
			envDbMcurr 			= FALSE,
			envSkGrinNoPlate 	= 0,
			envSoNumbers		= BY_BRANCH,
			envSoDoi			= FALSE,
			envSoFreightBord	= 1;

	long	debtorsDate			= 0L,
			systemDate 			= 0L;

	char	createStatusFlag 	[2],
			envBoRtype 			[2],
			envCurrCode 		[4],
			companyNumber 		[3],
			branchNumber 		[3],
			runningPrintProgram	[81];
	
#include	<cus_price.h>
#include	<cus_disc.h>

/*
 * Function Declarations 
 */
char	*CheckEnvironmentVariable 	(char *, char *, char *, char *);
int		CheckCohr 					(char *);
int		CheckForBackorders 			(long);
int		CreatePackingSlip 			(long);
long  	GetPcwoHhwoHash             (long);
void	AddRecalculate 				(void);
void	CheckEnvironment 			(void);
void	CloseDB 					(void);
void	CreateCohr 					(void);
void	OpenDB 						(void);
void	PrintPackingSlip			(void);
void	ProcessData 				(void);
void	ResetSoln 					(long);
void	UpDetailINLA 				(long, long);
void	UpDetailSONS 				(long, long);
void	UpDetailSkni 				(long, long);
void	UpHeaderSONS 				(long, long);
void	UpdateTrsh 					(long, long);

/*
 * Main Processing Routine.
 */
int
main	(
 int	argc,
 char	*argv [])
{
	if (argc != 2)
	{
		/*
		 * Usage : %s <createStatusFlag> 
		 */
		print_at (0,0,mlSoMess764, argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check environment variables. 
	 */
	CheckEnvironment ();

	OpenDB 		();		
	OpenPrice 	();
	OpenDisc 	();

	sprintf (createStatusFlag, 	"%-1.1s", 	argv [1]);

	/*
	 * Main data processing routine. 
	 */
	ProcessData ();

	/*
	 * Close Database.
	 */
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (sobg2, sobg);
	abc_alias (cohr2, cohr);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (incp,  incp_list, incp_no_fields, "incp_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sobg,  sobg_list, SOBG_NO_FIELDS, "sobg_id_no_2");
	open_rec (sobg2, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cnch,  cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sons,  sons_list, SONS_NO_FIELDS, "sons_id_no");
	open_rec (inla,  inla_list, INLA_NO_FIELDS, "inla_hhsl_id");
	open_rec (cudp,  cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (trsh,  trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhsl_hash");
	if (envSkGrinNoPlate)
		open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhsl_hash");
}

void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (comr);
	abc_fclose (sobg2);
	abc_fclose (sobg);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cumr);
	abc_fclose (inla);
	abc_fclose (inmr);
	abc_fclose (cnch);
	abc_fclose (pocr);
	abc_fclose (sons);
	abc_fclose (cudp);
	abc_fclose (trsh);
	abc_fclose (pcwo);
	if (envSkGrinNoPlate)
		abc_fclose (skni);

	ClosePrice ();
	CloseDisc ();
	abc_dbclose (data);
}

/*
 * Process sobg records	where sobg_type = "PA" or "PC"
 */
void
ProcessData (void)
{
	/*
	 * Set Signal Catching Routine	
	 */
	signal_on ();

	/*
	 * Process Until Prog Exit	
	 */
	while (!prog_exit)
	{
		systemDate = TodaysDate ();

		/*
		 * Initialise to sobg record	
		 */
		strcpy (sobg_rec.type, "PA");
		sobg_rec.lpno = 0;
		sobg_rec.hash = 0L;
		cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
		while (!prog_exit && !cc)
		{
			if (!PACK_PC && !PACK_PA)
			{
				abc_unlock (sobg);
				break;
			}

			if (BAD_HASH)
			{
				abc_delete (sobg);
				break;
			}

			/*
			 * Creat A P/Slip (cohr/coln) from (sohr/soln)	
			 */
			if (CreatePackingSlip (sobg_rec.hash))
			{
				/*
				 * If Auto - Print of P/Slip	
				 */
				if (PACK_PA)
					PrintPackingSlip ();
			}
		
			if (abc_delete (sobg))
				break;

			strcpy (sobg_rec.type, "PA");
			sobg_rec.lpno = 0;
			sobg_rec.hash = 0L;
			cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
	    }
		abc_unlock (sobg);

	    /*
	     * Ran out of records to process so start timing	
	     */
	    time_out ();
	}
}

/*
 * Creat Packing Slip	
 */
int
CreatePackingSlip (
	long	hhsoHash)
{
	int		addNewCohr 		= 0,
			linecnt 		= 0,
			nonStockLine 	= TRUE,
			lastBackorder 	= TRUE,
			pType			= 0,
			cumDisc			= 0;

	float	quantityOrder 	= 0.00,
			regPc			= 0.00;

	float	discArray [3]	= {0.00, 0.00, 0.00};

	double	value			= 0.00,
			workValue		= 0.00,
			salesOrderLine	= 0.00,
			salesOrderTotal = 0.00,
			lineTotal		= 0.00,
			taxTotal		= 0.00,
			lineDisc		= 0.00,
			lineTax			= 0.00,
			lineGst			= 0.00,
			grossPrice		= 0.00,
			nettPrice		= 0.00;

	/*
	 * Clear out structures. 
	 */
	memset (&cohr_rec , 0, sizeof (cohr_rec));
	memset (&coln_rec , 0, sizeof (coln_rec));
	memset (&sohr_rec , 0, sizeof (sohr_rec));
	memset (&soln_rec , 0, sizeof (soln_rec));

	/*
	 * Read sales order header record. 
	 */
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (sohr);
		return (FALSE);
	}
	/*
	 * Process full supply orders. i.e. all goods to be supplied at one time. 
	 */
	if (FULL_SUPPLY)
	{
		if (CheckForBackorders (sohr_rec.hhso_hash))
		{
			ResetSoln (sohr_rec.hhso_hash);
			return (FALSE);
		}
	}
	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
	{
		abc_unlock (sohr);
		return (FALSE);
	}
	/*
	 * Set the dbt_date (debtor module date) from looking at the 
	 * customer and determine whether it's company/branched owned
	 */
	if (atoi (cumr_rec.est_no))
	{
		strcpy (esmr_rec.co_no, cumr_rec.co_no);
		strcpy (esmr_rec.est_no, cumr_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc || esmr_rec.dbt_date <= 0L)
		{
			/*
			 * Error should not have occured so use company record.
			 */
			strcpy (comr_rec.co_no, cumr_rec.co_no);
			cc = find_rec (comr, &comr_rec, EQUAL, "r");
			if (cc)
			{
				abc_unlock (sohr);
				return (FALSE);
			}
			debtorsDate = comr_rec.dbt_date;
		}
		else
			debtorsDate = esmr_rec.dbt_date;
	}
	else
	{
		/* if est_no is 0, it's envDbCo */
		strcpy (comr_rec.co_no, cumr_rec.co_no);
		cc = find_rec (comr, &comr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (sohr);
			return (FALSE);
		}
		debtorsDate = comr_rec.dbt_date;
	}
	if (debtorsDate <= 0L)
		debtorsDate	=	TodaysDate ();

	/*
	 * Read contract if order header has a contract number.
	 */
	if (*clip (sohr_rec.cont_no))
	{
		strcpy (cnch_rec.co_no, sohr_rec.co_no);
		strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
		}
	}
	else
	{
		cnch_rec.hhch_hash = 0L;
		strcpy (cnch_rec.exch_type, " ");
	}

	/*
	 * Find currency record for debtors currency.
	 */
	pocr_rec.ex1_factor = 1.00;

	if (envDbMcurr)
	{
		strcpy (pocr_rec.co_no, cumr_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			pocr_rec.ex1_factor = 1.00;
			
		if (pocr_rec.ex1_factor <= 0.00)
			pocr_rec.ex1_factor = 1.00;
	}

	/*
	 * Reset Time stuff as processing valid sobg record
	 */
	set_timer ();

	soln_rec.hhso_hash	=	sohr_rec.hhso_hash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		/*
		 * Find Item from inmr.
		 */
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}
		/*
		 * Find item / warehouse master record from ccmr.
		 */
		ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}
		/*
		 * Not a Non Stock Item
		 */
		if (!NON_STOCK)
		{
			/*
			 * Sales Order Line Not R (eleased
			 */
			if (soln_rec.status [0] != 'R')
			{
				abc_unlock (soln);
				
				nonStockLine = FALSE;
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
			else
				nonStockLine = TRUE;
		}
		else
		{
			if (nonStockLine == FALSE)
			{
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
		}
		if (!addNewCohr)
		{
			abc_selfield (sons, "sons_id_no3");
			CreateCohr ();
			abc_selfield (sons, "sons_id_no");
		}

		addNewCohr = 1;

		strcpy (coln_rec.bonus_flag, soln_rec.bonus_flag);
		strcpy (coln_rec.hide_flag, soln_rec.hide_flag);
		strcpy (coln_rec.serial_no, soln_rec.serial_no);
		strcpy (coln_rec.order_no, sohr_rec.order_no);
		coln_rec.hhco_hash 		= cohr_rec.hhco_hash;
		coln_rec.line_no 		= linecnt++;
		coln_rec.hhbr_hash 		= soln_rec.hhbr_hash;
		coln_rec.hhsl_hash 		= soln_rec.hhsl_hash;
		coln_rec.incc_hash 		= soln_rec.hhcc_hash;
		coln_rec.hhum_hash 		= soln_rec.hhum_hash;
		coln_rec.qty_org_ord	= soln_rec.qty_org_ord;
		coln_rec.q_order 		= soln_rec.qty_order;
		coln_rec.q_backorder 	= soln_rec.qty_bord;
		coln_rec.cost_price 	= soln_rec.cost_price;
		coln_rec.item_levy 		= soln_rec.item_levy;
		coln_rec.cont_status 	= soln_rec.cont_status;
		coln_rec.gsale_price 	= soln_rec.gsale_price;
		coln_rec.reg_pc 		= soln_rec.reg_pc;
		coln_rec.disc_pc 		= soln_rec.dis_pc;
		coln_rec.disc_a 		= soln_rec.disc_a;
		coln_rec.disc_b 		= soln_rec.disc_b;
		coln_rec.disc_c 		= soln_rec.disc_c;
		coln_rec.cumulative		= soln_rec.cumulative;
		coln_rec.hhwo_hash      = GetPcwoHhwoHash (soln_rec.hhsl_hash);

		pType = atoi (sohr_rec.pri_type);
		grossPrice	=	GetCusPrice
						(
							cumr_rec.co_no,
							cumr_rec.est_no,
							ccmr_rec.cc_no,
							sohr_rec.area_code,
							cumr_rec.class_type,
							inmr_rec.sellgrp,
							cumr_rec.curr_code,
							pType,
							cumr_rec.disc_code,
							cnch_rec.exch_type,
							cumr_rec.hhcu_hash,
							soln_rec.hhcc_hash,
							soln_rec.hhbr_hash,
							inmr_rec.category,
							cnch_rec.hhch_hash,
							(envSoDoi) ? systemDate : debtorsDate,
							(soln_rec.qty_order + soln_rec.qty_bord),
							pocr_rec.ex1_factor,
							FGN_CURR,
							&regPc
						);

		nettPrice	=	GetCusGprice 
					 	(
							grossPrice, 
							regPc
						);

		contractPrice = (_CON_PRICE) ? TRUE : FALSE;

		if (BONUS)
		{
			coln_rec.item_levy 		= 0.00;
			coln_rec.sale_price 	= 0.00;
			coln_rec.reg_pc 		= 0.00;
			coln_rec.disc_pc 		= 0.00;
			coln_rec.disc_a 		= 0.00;
			coln_rec.disc_b 		= 0.00;
			coln_rec.disc_c 		= 0.00;
			coln_rec.cumulative		= 0;
		}
		else
		{
			/*
			 * Contract is blank so recalculate discount. 
			 */
			if (RELEASE && !coln_rec.cont_status && 
					soln_rec.pri_or [0] == 'N' && nettPrice != 0.00) 
			{
				coln_rec.sale_price = nettPrice;
			}
			else
			{
				coln_rec.sale_price = soln_rec.sale_price;
			}
		}

		coln_rec.cost_price = soln_rec.cost_price;
		coln_rec.item_levy 	= soln_rec.item_levy;

		if (BONUS)
		{
			coln_rec.sale_price 	= 0.00;
			coln_rec.item_levy	 	= 0.00;
			coln_rec.reg_pc 		= 0.00;
			coln_rec.disc_pc 		= 0.00;
			coln_rec.disc_a 		= 0.00;
			coln_rec.disc_b 		= 0.00;
			coln_rec.disc_c 		= 0.00;
			coln_rec.cumulative		= 0;
		}
		else
		{
			/*
			 * If release time determins price AND not a contract 
             * AND discount has not been overiden.                
			 */
			if (RELEASE && !coln_rec.cont_status && 
				           !contractPrice && 
						   soln_rec.dis_or [0] == 'N') 
			{
				cumDisc	=	GetCusDisc 
							(
								cumr_rec.co_no,
								cumr_rec.est_no,
								soln_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								soln_rec.hhbr_hash,
								inmr_rec.category,
								inmr_rec.sellgrp,
								pType,
								grossPrice,
								regPc,
								soln_rec.qty_order + soln_rec.qty_bord,
								discArray
						);

				coln_rec.disc_pc	=	CalcOneDisc 
										(
											cumDisc,
											discArray [0],
											discArray [1],
											discArray [2]
										);

				coln_rec.reg_pc 	= 	regPc;
				coln_rec.disc_a 	= 	discArray [0];
				coln_rec.disc_b 	= 	discArray [1];
				coln_rec.disc_c 	= 	discArray [2];
				coln_rec.cumulative = 	cumDisc;

				/*
				 * check against master override 
				 */
				if (inmr_rec.disc_pc > coln_rec.disc_pc &&
					inmr_rec.disc_pc != 0.00)
				{
					coln_rec.disc_pc = inmr_rec.disc_pc;
					coln_rec.disc_a  = inmr_rec.disc_pc;
					coln_rec.disc_b  = 0.00;
					coln_rec.disc_c  = 0.00;
				}
			}
		}
		coln_rec.tax_pc	= (float) ((NOTAX) ? 0.00 : soln_rec.tax_pc);
		coln_rec.gst_pc = (float) ((NOTAX) ? 0.00 : soln_rec.gst_pc);
		strcpy (coln_rec.pack_size, soln_rec.pack_size);
		strcpy (coln_rec.sman_code, soln_rec.sman_code);
		strcpy (coln_rec.cus_ord_ref, soln_rec.cus_ord_ref);
		coln_rec.o_xrate = soln_rec.o_xrate;
		coln_rec.n_xrate = soln_rec.n_xrate;
		strcpy (coln_rec.item_desc, soln_rec.item_desc);
		coln_rec.due_date = systemDate;
		strcpy (coln_rec.status, 	createStatusFlag);
		strcpy (coln_rec.stat_flag, createStatusFlag);

		lineTotal = coln_rec.q_order *	out_cost
										(
											coln_rec.sale_price,
											inmr_rec.outer_size
										);
		lineTotal = no_dec (lineTotal);

		if (FGN_CURR)
			inmr_rec.tax_amount = 0.00;
		taxTotal = coln_rec.q_order *	out_cost 
										(
											inmr_rec.tax_amount,
						        			inmr_rec.outer_size
										);
		taxTotal = no_dec (taxTotal);

		quantityOrder += coln_rec.q_order;
		salesOrderLine = (coln_rec.q_order + coln_rec.q_backorder) * 
							out_cost 
							(
								coln_rec.sale_price,
						 		inmr_rec.outer_size
							);
		salesOrderTotal += salesOrderLine;
		salesOrderTotal = no_dec (salesOrderTotal);

		lineDisc = (double) DOLLARS (coln_rec.disc_pc);
		lineDisc *= lineTotal;
		lineDisc = no_dec (lineDisc);

		lineTax = (double) DOLLARS (coln_rec.tax_pc);

		if (cumr_rec.tax_code [0] == 'D')
			lineTax *= taxTotal;
		else
			lineTax *= (lineTotal - lineDisc);

		lineTax = no_dec (lineTax);

		lineGst = (double) DOLLARS (coln_rec.gst_pc);
		lineGst *= ((lineTotal - lineDisc) + lineTax);

		coln_rec.gross 		= lineTotal;
		coln_rec.amt_disc 	= lineDisc;
		coln_rec.amt_tax 	= lineTax;
		coln_rec.amt_gst 	= lineGst;

		cohr_rec.item_levy	+= coln_rec.item_levy;
		cohr_rec.gross 		+= lineTotal;
		cohr_rec.disc 		+= lineDisc;
		cohr_rec.tax 		+= lineTax;
		cohr_rec.gst 		+= lineGst;

		cc = abc_add (coln, &coln_rec);
		if (cc)
			file_err (cc, "coln", "DBADD");

		cc = find_rec (coln, &coln_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "coln", "DBFIND");

		if (envSkGrinNoPlate)
		{
			UpDetailSkni 
			(
				soln_rec.hhsl_hash, 
				coln_rec.hhcl_hash
			);
		}
		UpDetailSONS 
		(
			soln_rec.hhsl_hash, 
			coln_rec.hhcl_hash
		);
		UpDetailINLA 
		(
			soln_rec.hhsl_hash, 
			coln_rec.hhcl_hash
		);

		/*
		 * If all of line placed on b/o then  
		 * update status line as a b/o.        
		 */
		if (soln_rec.qty_order == 0.00 && !NON_STOCK)
		{
			lastBackorder		= FALSE;
			soln_rec.qty_order	= soln_rec.qty_bord;
			soln_rec.qty_bord	= 0.00;
			strcpy (soln_rec.status, "B");
			strcpy (soln_rec.stat_flag, "B");

			AddRecalculate ();
		}
		else
		{
			if (NON_STOCK && lastBackorder == FALSE)
			{
				strcpy (soln_rec.status, "B");
				strcpy (soln_rec.stat_flag, "B");
				AddRecalculate ();
			}
			else
				strcpy (soln_rec.status, "P");

			lastBackorder = TRUE;
		}
	
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, "soln", "DBUPDATE");

		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);

	if (!addNewCohr)
	{
		abc_unlock (sohr);
		return (FALSE);
	}

	strcpy (comr_rec.co_no, cohr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");

	workValue = (NOTAX) ? 0.00 : (double) DOLLARS (comr_rec.gst_rate);

	/*
	 * Convert comr sos amounts to foreign currency. 
	 */
	if (envDbMcurr)
	{
		comr_rec.sur_cof *= pocr_rec.ex1_factor;
		comr_rec.sur_amt *= pocr_rec.ex1_factor;
	}

	if (quantityOrder > 0.00 && SUR_CHARGE && 
		salesOrderTotal < comr_rec.sur_cof && salesOrderTotal > 0.00)
	{
		cohr_rec.sos = comr_rec.sur_amt;
	}
	else
		cohr_rec.sos = 0.00;

	value = cohr_rec.freight + 
			cohr_rec.insurance +
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3 +
			cohr_rec.sos;

	workValue	*= value;
	workValue	= no_dec (workValue);
	cohr_rec.gst += workValue;
	cohr_rec.gst = no_dec (cohr_rec.gst);

	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBUPDATE");

	/*
	 * Check if sohr needs to be deleted	
	 * ie no soln records remaining for	
	 * sohr.								
	 */
	soln_rec.hhso_hash	=	sohr_rec.hhso_hash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
	{
		cc = abc_delete (sohr);
		if (cc) 
			file_err (cc, "sohr", "DBDELETE");
	}
	else
	{
		strcpy (sohr_rec.inv_no, cohr_rec.inv_no);
		strcpy (sohr_rec.status, "P");

		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, "sohr", "DBUPDATE");
	}
	/*
	 * Add record to calc Order (s) balance on cumr. 
	 */
	strcpy (sobg2_rec.co_no, sobg_rec.co_no);
	strcpy (sobg2_rec.br_no, sobg_rec.br_no);
	strcpy (sobg2_rec.type, "RO");
	sobg2_rec.lpno = 0;
	sobg2_rec.hash = cumr_rec.hhcu_hash;
	sobg2_rec.hash2 = 0L;
	cc = find_rec (sobg2, &sobg2_rec, COMPARISON, "r");
	if (cc)
	{
		cc = abc_add (sobg2, &sobg2_rec);
		if (cc)
			file_err (cc, "sobg", "DBADD");
	}
	return (TRUE);
}

void
CreateCohr (void)
{
	char	tmpPrefix 	 [3]; 
	char	tmpInvoiceNo [9];
	char	tmpMask		 [12];
	long	invoiceNo	=	0L;
	int		len = 8;

	/*
	 * Is invoice number to come from department of branch. 
	 */
	if (envSoNumbers == BY_DEPART)
	{
		strcpy (cudp_rec.co_no, sohr_rec.co_no);
		strcpy (cudp_rec.br_no, sohr_rec.br_no);
		strcpy (cudp_rec.dp_no, sohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "cudp", "DBFIND");

		invoiceNo	=	(CASH_INV) ? cudp_rec.nx_csh_no : cudp_rec.nx_chg_no;
		invoiceNo++;
	}
	else
	{
		strcpy (esmr_rec.co_no, sohr_rec.co_no);
		strcpy (esmr_rec.est_no, sohr_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "esmr", "DBFIND");
	
		invoiceNo	=	(CASH_INV) ? esmr_rec.nx_csh_inv : esmr_rec.nx_inv_no;
		invoiceNo++;
	}

	if (envSoNumbers == BY_BRANCH)
	{
		if (CASH_INV)
			strcpy (tmpPrefix, esmr_rec.csh_pref);
		else
			strcpy (tmpPrefix, esmr_rec.chg_pref);
	}
	else
	{
		if (CASH_INV)
			strcpy (tmpPrefix, cudp_rec.csh_pref);
		else
			strcpy (tmpPrefix, cudp_rec.chg_pref);
	}

	clip (tmpPrefix);
	len = strlen (tmpPrefix);

	sprintf (tmpMask, "%%s%%0%dld", 8 - len);
	sprintf (tmpInvoiceNo, tmpMask, tmpPrefix, invoiceNo);

	while (CheckCohr (tmpInvoiceNo) == 0)
		sprintf (tmpInvoiceNo, tmpMask, tmpPrefix, invoiceNo++);

	if (envSoNumbers == BY_DEPART)
	{
		if (CASH_INV)
			cudp_rec.nx_csh_no	=	invoiceNo;
		else
			cudp_rec.nx_chg_no	=	invoiceNo;

		cc = abc_update (cudp, &cudp_rec);
		if (cc)
			file_err (cc, "cudp", "DBUPDATE");
	}
	else
	{
		if (CASH_INV)
			esmr_rec.nx_csh_inv	=	invoiceNo;
		else
			esmr_rec.nx_inv_no	=	invoiceNo;

		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBUPDATE");
	}

	/*
	 * Add All the Header Stuff	
	 */
	strcpy (cohr_rec.co_no, 	sohr_rec.co_no);
	strcpy (cohr_rec.br_no, 	sohr_rec.br_no);
	strcpy (cohr_rec.dp_no, 	sohr_rec.dp_no);
	strcpy (cohr_rec.inv_no, 	cohr2_rec.inv_no);
	cohr_rec.hhcu_hash 			= sohr_rec.hhcu_hash;
	cohr_rec.chg_hhcu_hash 		= sohr_rec.chg_hhcu_hash;
	cohr_rec.hhso_hash 			= sohr_rec.hhso_hash;
	strcpy (cohr_rec.type, 		"P");
	strcpy (cohr_rec.cons_no, 	sohr_rec.cons_no);
	strcpy (cohr_rec.del_zone, 	sohr_rec.del_zone);
	strcpy (cohr_rec.del_req, 	sohr_rec.del_req);
	cohr_rec.del_date		=	sohr_rec.del_date;
	strcpy (cohr_rec.asm_req, 	sohr_rec.asm_req);
	cohr_rec.asm_date		=	sohr_rec.asm_date;
	strcpy (cohr_rec.s_timeslot, sohr_rec.s_timeslot);
	strcpy (cohr_rec.e_timeslot, sohr_rec.e_timeslot);
	strcpy (cohr_rec.carr_code, sohr_rec.carr_code);
	strcpy (cohr_rec.carr_area, sohr_rec.carr_area);
	cohr_rec.no_cartons 	= 	sohr_rec.no_cartons;
	cohr_rec.no_kgs 		= 	sohr_rec.no_kgs;
	strcpy (cohr_rec.op_id, 	sohr_rec.op_id);
	strcpy (cohr_rec.cont_no, 	sohr_rec.cont_no);
	/*cohr_rec.date_create = TodaysDate ();*/
	cohr_rec.date_create = comr_rec.dbt_date;
	strcpy (cohr_rec.time_create, TimeHHMM ());
	strcpy (cohr_rec.cus_ord_ref, sohr_rec.cus_ord_ref);
	strcpy (cohr_rec.chg_ord_ref, sohr_rec.chg_ord_ref);
	strcpy (cohr_rec.frei_req, sohr_rec.frei_req);
	cohr_rec.date_raised   = (envSoDoi) ? systemDate : debtorsDate;
	cohr_rec.date_required = systemDate;
	strcpy (cohr_rec.tax_code,  sohr_rec.tax_code);
	strcpy (cohr_rec.tax_no,    sohr_rec.tax_no);
	strcpy (cohr_rec.area_code, sohr_rec.area_code);
	strcpy (cohr_rec.sale_code, sohr_rec.sman_code);
	cohr_rec.item_levy 		= 0.00;
	cohr_rec.gross 			= 0.00;
	cohr_rec.freight 		= (NEW_SO) ? 0.00 : sohr_rec.freight;
	cohr_rec.insurance 		= (NEW_SO) ? 0.00 : sohr_rec.insurance;
	cohr_rec.other_cost_1	= (NEW_SO) ? 0.00 : sohr_rec.other_cost_1;
	cohr_rec.other_cost_2	= (NEW_SO) ? 0.00 : sohr_rec.other_cost_2;
	cohr_rec.other_cost_3	= (NEW_SO) ? 0.00 : sohr_rec.other_cost_3;
	cohr_rec.deposit 		= (NEW_SO) ? 0.00 : sohr_rec.deposit;
	cohr_rec.ex_disc 		= (NEW_SO) ? 0.00 : sohr_rec.discount;
	cohr_rec.tax 			= 0.00;
	cohr_rec.gst 			= 0.00;
	cohr_rec.disc 			= 0.00;
	strcpy (cohr_rec.fix_exch,  sohr_rec.fix_exch);
	strcpy (cohr_rec.batch_no,  sohr_rec.batch_no);
	strcpy (cohr_rec.dl_name,   sohr_rec.del_name);
	strcpy (cohr_rec.dl_add1,  	sohr_rec.del_add1);
	strcpy (cohr_rec.dl_add2,  	sohr_rec.del_add2);
	strcpy (cohr_rec.dl_add3,  	sohr_rec.del_add3);
	strcpy (cohr_rec.din_1,     sohr_rec.din_1);
	strcpy (cohr_rec.din_2,     sohr_rec.din_2);
	strcpy (cohr_rec.din_3,     sohr_rec.din_3);
	strcpy (cohr_rec.pay_terms, sohr_rec.pay_term);
	strcpy (cohr_rec.sell_terms,sohr_rec.sell_terms);
	strcpy (cohr_rec.ins_det,   sohr_rec.ins_det);
	strcpy (cohr_rec.pri_type,  sohr_rec.pri_type);
	strcpy (cohr_rec.ord_type,  sohr_rec.ord_type);
	strcpy (cohr_rec.status,    createStatusFlag);
	strcpy (cohr_rec.stat_flag, createStatusFlag);
	strcpy (cohr_rec.ps_print,  "N");
	strcpy (cohr_rec.inv_print, "N");
	strcpy (cohr_rec.printing,  (PACK_PA) ? "Y" : " ");
	strcpy (cohr_rec.prt_price, sohr_rec.prt_price);
	cohr_rec.exch_rate = sohr_rec.exch_rate;

	sprintf (err_str, "DBADD Invoice [%s] ", cohr_rec.inv_no);
	cc = abc_add (cohr, &cohr_rec);
	if (cc)
		file_err (cc, "cohr", err_str);

	cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "cohr", "DBFIND");

	/*
	 * Updated sales order header extra description. 
	 */
	UpHeaderSONS 
	(
		sohr_rec.hhso_hash, 
		cohr_rec.hhco_hash
	);
	/*
	 * Updated transport schedule file. 
	 */
	UpdateTrsh 	 
	(
		sohr_rec.hhso_hash, 
		cohr_rec.hhco_hash
	);
	/*
	 * Updated Sales Order Service File. 
	 */
	UpdateSosf 	 
	(
		cohr_rec.hhcu_hash, 
		sohr_rec.hhso_hash, 
		cohr_rec.hhco_hash
	);
	/*
	 * Create a log file record for sales Order. 
	 */
	LogCustService 
	(
		cohr_rec.hhco_hash,
		sohr_rec.hhso_hash,
		cohr_rec.hhcu_hash,
		cohr_rec.cus_ord_ref,
		cohr_rec.cons_no,
		cohr_rec.carr_code,
		cohr_rec.del_zone,
		LOG_PCCREATE
	);
}

int	
CheckCohr (
	char *invoiceNumber)
{
	/*
	 * Check for existing packing slip	
	 */
	strcpy (cohr2_rec.co_no, sohr_rec.co_no);
	strcpy (cohr2_rec.br_no, sohr_rec.br_no);
	strcpy (cohr2_rec.type, "P");
	sprintf (cohr2_rec.inv_no, "%-8.8s", invoiceNumber);
	cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "r");
	if (!cc)
		return (cc);

	/*
	 * Check for existing packing slip	
	 */
	strcpy (cohr2_rec.co_no, sohr_rec.co_no);
	strcpy (cohr2_rec.br_no, sohr_rec.br_no);
	strcpy (cohr2_rec.type, "T");
	sprintf (cohr2_rec.inv_no, "%-8.8s", invoiceNumber);
	cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "r");
	if (!cc)
		return (cc);

	/*
	 * Check for existing invoice	
	 */
	strcpy (cohr2_rec.co_no, sohr_rec.co_no);
	strcpy (cohr2_rec.br_no, sohr_rec.br_no);
	strcpy (cohr2_rec.type, "I");
	sprintf (cohr2_rec.inv_no, "%-8.8s", invoiceNumber);
	return (find_rec (cohr2, &cohr2_rec, COMPARISON, "r"));
}

void
PrintPackingSlip (void)
{
	if (sobg_rec.lpno == 0)
		sobg_rec.lpno = 1;

	/*
	 * first time or Company or Branch Changed	
	 */
	if (printerNumber == 0 || strcmp (companyNumber , sobg_rec.co_no) || 
			 strcmp (branchNumber, sobg_rec.br_no))
	{
		strcpy (err_str,CheckEnvironmentVariable ("SO_CTR_PAC","so_ctr_pac",
					sobg_rec.co_no,sobg_rec.br_no));

		strcpy (runningPrintProgram, err_str);
		strcpy (companyNumber, sobg_rec.co_no);
		strcpy (branchNumber, sobg_rec.br_no);
	}
	printerNumber = sobg_rec.lpno;

	if (!(printerOutputFile = popen (runningPrintProgram, "w")))
	{
		strcpy (err_str, runningPrintProgram);
		file_err (errno, "POPEN", err_str);
	}

	/*
	 * One could make it faster by leaving pipe open and sending multiple  
	 * hashes to counter pack, one could but one should not make code that 
	 * is less reliable.                                                   
	 */
	fprintf (printerOutputFile, "%d\n", sobg_rec.lpno);
	fprintf (printerOutputFile, "S\n");
	fprintf (printerOutputFile, "%ld\n", cohr_rec.hhco_hash);
	fprintf (printerOutputFile, "%ld\n", 0L);
	pclose (printerOutputFile);
}

char *
CheckEnvironmentVariable (
	char	*environmentValue,
	char	*programName,
	char	*companyNo,
	char	*branchNo)
{
	char	*sptr;
	char	intRunProgram [41];

	/*
	 * Check Company & Branch	
	 */
	sprintf (intRunProgram, "%s%s%s", environmentValue, companyNo, branchNo);
	sptr = chk_env (intRunProgram);
	if (sptr == (char *)0)
	{
		/*
		 * Check Company	
		 */
		sprintf (intRunProgram, "%s%s", environmentValue, companyNo);
		sptr = chk_env (intRunProgram);
		if (sptr == (char *)0)
		{
			sprintf (intRunProgram, "%s", environmentValue);
			sptr = chk_env (intRunProgram);
			return ((sptr == (char *)0) ? programName : sptr);
		}
		else
			return (sptr);
	}
	else
		return (sptr);
}

void
AddRecalculate (void)
{
	/*
	 * Add record to calc Order (s) balance on cumr. 
	 */
	strcpy (sobg2_rec.co_no, sobg_rec.co_no);
	strcpy (sobg2_rec.br_no, sobg_rec.br_no);
	strcpy (sobg2_rec.type, "RC");
	sobg2_rec.lpno = 0;
	sobg2_rec.hash = soln_rec.hhbr_hash;
	sobg2_rec.hash2 = soln_rec.hhcc_hash;
	if (!find_rec (sobg2, &sobg2_rec, COMPARISON, "r"))
		abc_add (sobg2, &sobg2_rec);

	return;
}

int
CheckForBackorders (
	long	hhsoHash)
{
	soln_rec.hhso_hash	=	hhsoHash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		/*
		 * Some stock backordered or line is backordered 
		 */
		if (soln_rec.qty_bord > 0.00 || soln_rec.status [0] == 'B')
			return (TRUE);

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (FALSE);
}

void
ResetSoln (
	long	hhsoHash)
{
	int		nonStockLine	=	TRUE;

	soln_rec.hhso_hash	=	hhsoHash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		/*
		 * Not a Non Stock Item	
		 */
		if (!NON_STOCK)
		{
			/*
			 * Sales Order Line Not R (eleased	
			 */
			if (soln_rec.status [0] != 'R')
			{
				nonStockLine = FALSE;
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
			else
				nonStockLine = TRUE;
		}
		else
		{
			if (nonStockLine == FALSE)
			{
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
		}
		if (soln_rec.qty_order == 0.00 && !NON_STOCK)
		{
			strcpy (soln_rec.status, 	"B");
			strcpy (soln_rec.stat_flag, "B");
		}
		else
		{
			strcpy (soln_rec.status, 	"M");
			strcpy (soln_rec.stat_flag, "M");
		}
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, "soln", "DBUPDATE");

		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);
	strcpy (sohr_rec.status, 	"M");
	strcpy (sohr_rec.stat_flag, "M");
	cc = abc_update (sohr, &sohr_rec);
	if (cc)
		file_err (cc, "sohr", "DBUPDATE");

}

/*
 * Updated sales order line description to packing lines.
 */
void
UpDetailINLA (
	long	hhslHash,
	long	hhclHash)
{
	inla_rec.hhsl_hash 	= hhslHash;
	inla_rec.inlo_hash 	= 0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhsl_hash == hhslHash)
	{
		inla_rec.hhcl_hash = hhclHash;
		cc = abc_update (inla, &inla_rec);
		if (cc)
			abc_unlock (inla);

		cc = find_rec (inla, &inla_rec, NEXT, "u");
	}
	abc_unlock (inla);
}
/*
 * Updated sales order line description to packing lines.
 */
void	
UpDetailSONS (
	long	hhslHash,
	long	hhclHash)
{
	sons_rec.hhsl_hash 	= hhslHash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "u");
	while (!cc && sons_rec.hhsl_hash == hhslHash)
	{
		sons_rec.hhcl_hash = hhclHash;
		cc = abc_update (sons, &sons_rec);
		if (cc)
			abc_unlock (sons);

		cc = find_rec (sons, &sons_rec, NEXT, "u");
	}
	abc_unlock (sons);
}
/*
 * Updated sales order header description to packing header.
 */
void	
UpHeaderSONS (
	long	hhsoHash,
	long	hhcoHash)
{
	sons_rec.hhso_hash 	= hhsoHash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "u");
	while (!cc && sons_rec.hhso_hash == hhsoHash)
	{
		sons_rec.hhco_hash = hhcoHash;
		cc = abc_update (sons, &sons_rec);
		if (cc)
			abc_unlock (sons);

		cc = find_rec (sons, &sons_rec, NEXT, "u");
	}
	abc_unlock (sons);
}
/*
 * Updated Transport schedule records.
 */
void	
UpdateTrsh (
	long	hhsoHash,
	long	hhcoHash)
{
	trsh_rec.hhso_hash 	= hhsoHash;
	cc = find_rec (trsh, &trsh_rec, COMPARISON, "u");
	if (!cc)
	{
		trsh_rec.hhco_hash	=	hhcoHash;
		cc = abc_update (trsh, &trsh_rec);
		if (cc)
			abc_unlock (trsh);
	}
	else
		abc_unlock (trsh);
}
/*
 * Updated number plates lines allocated to sales order lines.
 */
void	
UpDetailSkni (
	long	hhslHash,
	long	hhclHash)
{
	skni_rec.hhsl_hash 	= hhslHash;
	cc = find_rec (skni, &skni_rec, GTEQ, "u");
	while (!cc && skni_rec.hhsl_hash == hhslHash)
	{
		skni_rec.hhcl_hash = hhclHash;
		cc = abc_update (skni, &skni_rec);
		if (cc)
			abc_unlock (skni);

		cc = find_rec (skni, &skni_rec, NEXT, "u");
	}
	abc_unlock (skni);
}
/*
 * Check all environment variables.
 */
void	
CheckEnvironment (void)
{
	char	*sptr;

	/*
	 * Read native currency. 
	 */
	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));
	
	/*
	 * Check order release type. 
	 */
	sptr = chk_env ("BO_RTYPE");
	sprintf (envBoRtype, "%-1.1s", (sptr != (char *)0) ? sptr : "I");

	/*
	 * Check if number plates used. 
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = ((sptr == (char *)0)) ? 0 : atoi (sptr);

	/*
	 * Check for Sales order date of invoice. 
	 */
	sptr = chk_env ("SO_DOI");
	envSoDoi = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	/*
	 * Check for Multi-Currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for sales order number. 
	 */
	sptr = chk_env ("SO_NUMBERS");
	envSoNumbers = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	/*
	 * Check for freight on back orders. 
	 */
	sptr = chk_env ("SO_FREIGHT_BORD");
	envSoFreightBord = (sptr == (char *)0) ? 1 : atoi (sptr);
}

long
GetPcwoHhwoHash   (
   long hhslHash)
{
	pcwo_rec.hhsl_hash = hhslHash;
    cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
    if (cc)
    	return 0L;
    return   (pcwo_rec.hhwo_hash);
}


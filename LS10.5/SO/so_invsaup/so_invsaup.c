/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_invsaup.c,v 5.2 2001/08/09 09:21:25 scott Exp $
|  Program Name  : (so_invsaup.c & so_crdsaup.c)
|  Program Desc  : (Update Sales Analysis File From cohr,coln)
|---------------------------------------------------------------------|
|  Updates files : See /usr/ver (x)/DOCS/Programs                     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/05/91         |
|---------------------------------------------------------------------|
| $Log: so_invsaup.c,v $
| Revision 5.2  2001/08/09 09:21:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:25  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_invsaup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invsaup/so_invsaup.c,v 5.2 2001/08/09 09:21:25 scott Exp $";

#include	<pslscr.h>
#include 	<twodec.h>
#include 	<ml_so_mess.h>

#define		DOLLAR (coln_rec.crd_type [0] == 'D')
#define		FAULTY (coln_rec.crd_type [0] == 'F')
#define		INVOICE	 (transactionTypeFlag [0] == 'I' || \
					  transactionTypeFlag [0] == 'P')

#define		NOTAX	 (cohr_rec.tax_code [0] == 'A' || \
		          	  cohr_rec.tax_code [0] == 'B')

#define		MONTHLY_SALES	 (tspm_rec.pm_sales_per [0] == 'M')

#define		BLANK_SMAN	 (!strcmp (coln_rec.sman_code, "  "))

#define		FUTURE		 (invoiceMonthEnd > salesMonthEnd)
#define		X_RATE		 (cohr_rec.exch_rate)
#define		EXCH_OK		 (cohr_rec.exch_rate != 0.0)

#define		HeldOverMonth	13

#include	"schema"

struct commRecord	comm_rec;
struct saleRecord	sale_rec;
struct cusaRecord	cusa_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct cnchRecord	cnch_rec;
struct cncdRecord	cncd_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct cumrRecord	cumr_rec;
struct sumrRecord	sumr_rec;
struct ccmrRecord	ccmr_rec;
struct sadfRecord	sadf_rec;
struct cushRecord	cush_rec;
struct tshsRecord	tshs_rec;
struct tspmRecord	tspm_rec;
struct iudiRecord	iudi_rec;
struct saudRecord	saud_rec;

	Money	*cusa_val		=	&cusa_rec.val1;
	float	*saud_qty_per	=	&saud_rec.qty_per1;
	double	*saud_cst_per	=	&saud_rec.cst_per1;
	double	*saud_sal_per	=	&saud_rec.sal_per1;
	float	*sadf_qty_per	=	&sadf_rec.qty_per1;
	double	*sadf_cst_per	=	&sadf_rec.cst_per1;
	double	*sadf_sal_per	=	&sadf_rec.sal_per1;

	char	*data = "data";

	int		invoiceMonth			=	0,
			salesAnalysisHistory	=	FALSE,
			salesAnalysisByItem		=	FALSE,
			teleSalesInstalled		=	FALSE,
			multiCurrency			=	FALSE,
			salesOnCosts			=	FALSE,
			userDefinedSales		=	FALSE;

	long	lsystemDate 	= 0L,
			invoiceMonthEnd = 0L,
			salesMonthEnd 	= 0L;

	float	gstInclude 		= 0.00,
			gstDivide		= 0.00;

	char	wk_branch 			[3],
			systemDate 			[11],
			transactionTypeFlag [2],
			storeMonth 			[3],
			findStatusFlag 		[2],
			updateStatusFlag 	[2],
			currencyCode 		[4];


/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessInvoices 	(void);
void 	ProcessLines 		(double, double, double *);
void 	ProcessDetail 		(float, double, double);
void 	ProcessUserDefined 	(float, double, double);
void 	ProcessTeleSales 	(float, double, double, double);
void 	ProcessOtherCharges (char *, double, double);
void 	AddCusa 			(double);
double 	OutGst 				(double);
void 	AddCush 			(long, char *, char *, float, double, float, long);
int  	CalcMonth 			(long);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	if (argc < 4)
	{
		print_at (0,0, "Usage : %s <findStatusFlag> <updateStatusFlag> <transactionTypeFlag> <Optional - branchNo>" , argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	multiCurrency = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SA_ONCOST");
	salesOnCosts = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SA_USER_DEFINE");
	userDefinedSales = (sptr == (char *)0) ? FALSE : atoi (sptr);

	gstInclude = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (gstInclude != 0.00)
		gstDivide = (float) ((100.00 + gstInclude) / gstInclude);

	sptr = chk_env ("SA_PROD");
	salesAnalysisByItem = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SO_SA_HIST");
	salesAnalysisHistory = (sptr == (char *) 0) ? FALSE : atoi (sptr) - 1;

	sptr = chk_env ("TS_INSTALLED");
	teleSalesInstalled = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	OpenDB ();

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	init_scr ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	sprintf (findStatusFlag, 	  "%1.1s", argv [1]);
	sprintf (updateStatusFlag, 	  "%1.1s", argv [2]);
	sprintf (transactionTypeFlag, "%1.1s", argv [3]);

	sprintf (err_str," Processing %s to Sales Analysis.",
					 (INVOICE) ? "Invoices" : "Credits");

	print_mess (ML (err_str));

	if (argc == 5)
		sprintf (wk_branch, "%-2.2s", argv [4]);
	else
		sprintf (wk_branch, "%-2.2s", comm_rec.est_no);

	ProcessInvoices ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no2");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	if (multiCurrency)
	{
		open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	}
	if (salesAnalysisByItem)
		open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no");
	if (salesAnalysisHistory)
		open_rec (cush, cush_list, CUSH_NO_FIELDS, "cush_id_no");

	if (teleSalesInstalled)
	{
		open_rec (tshs, tshs_list, TSHS_NO_FIELDS, "tshs_id_no");
		open_rec (tspm, tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
	}
	if (userDefinedSales)
	{
		open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
		open_rec (iudi, iudi_list, IUDI_NO_FIELDS, "iudi_id_no");
		open_rec (saud, saud_list, SAUD_NO_FIELDS, "saud_id_no");
	}
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sale);
	abc_fclose (ccmr);
	abc_fclose (cusa);
	if (multiCurrency)
	{
		abc_fclose (pocr);
		abc_fclose (sumr);
	}
	if (salesAnalysisByItem)
		abc_fclose (sadf);
	if (salesAnalysisHistory)
		abc_fclose (cush);

	if (teleSalesInstalled)
	{
		abc_fclose (tshs);
		abc_fclose (tspm);
	}
	if (userDefinedSales)
	{
		abc_fclose (iudi);
		abc_fclose (saud);
		abc_fclose (excf);
	}
	abc_dbclose (data);
}

/*========================================================
| Process whole coln file looking for stat flags of  '2' |
| and processing accordingly.                            |
| Returns: 0 if ok, non-zero if not ok.                  |
========================================================*/
void
ProcessInvoices (void)
{
	int		ok = 1;
	double	onCosts;

	double	freight_other = 0.00;
	double	ex_disc = 0.00;

	salesMonthEnd = MonthEnd (comm_rec.dbt_date);

	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	while (ok) 
	{
		strcpy (cohr_rec.co_no,     comm_rec.co_no);
		strcpy (cohr_rec.br_no,     wk_branch);
		strcpy (cohr_rec.type,      transactionTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatusFlag);
		cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
		if (cc) 
		{
			abc_unlock (cohr);
			ok = FALSE;
			continue;
		}
		invoiceMonthEnd = MonthEnd (cohr_rec.date_raised);

		invoiceMonth = CalcMonth (cohr_rec.date_raised);
		sprintf (storeMonth, "%02d", invoiceMonth);
		invoiceMonth--;

		if (FUTURE)
		{
			invoiceMonth = HeldOverMonth - 1;
			sprintf (storeMonth, "%02d", HeldOverMonth);
		}

		/*-------------------------
		| Save current stat flag. |
		-------------------------*/
		freight_other 	= DPP ((cohr_rec.freight + 
						 	  cohr_rec.insurance + 
						 	  cohr_rec.other_cost_1 + 
						 	  cohr_rec.other_cost_2 + 
						 	  cohr_rec.other_cost_3)); 

		ex_disc		 	= DPP (cohr_rec.ex_disc);

		if (gstDivide != 0.00)
		{
			freight_other = OutGst (freight_other);
			ex_disc = OutGst (ex_disc);
		}
		
		onCosts = 0.00;
		ProcessLines (ex_disc, freight_other, &onCosts);

		AddCusa (onCosts);
	}
	abc_unlock (cohr);
}

/*==========================================
| Process line items for selected invoice. |
==========================================*/
void
ProcessLines (
	double	 ex_disc,
	double  freight_other,
	double	*oncost)
{
	int		newSalesRecord;
	char	wkCurr [4];

	double	wkFactor = 0.00,
			wk_value = 0.00,
			tot_sale = 0.00,
			tot_avge = 0.00,
			tot_disc = 0.00,
			tot_ocst = 0.00,
			loc_sale = 0.00,
			loc_disc = 0.00,
			loc_ocst = 0.00;

	coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
   	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
   	{
		if (BLANK_SMAN)
			strcpy (coln_rec.sman_code, cohr_rec.sale_code);

		/*-------------------------------
		| Find Inventory Master Record. |
		-------------------------------*/
		inmr_rec.hhbr_hash 	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc) 
		{
			strcpy (inmr_rec.inmr_class,    "D");
			strcpy (inmr_rec.category, "DELETED    ");
		}
		if (!cc && inmr_rec.hhsi_hash != 0L)
		{
			inmr_rec.hhbr_hash 	=	inmr_rec.hhsi_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
			{
				inmr_rec.hhbr_hash 	=	coln_rec.hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			}
		}
		if (cc) 
		{
			strcpy (inmr_rec.inmr_class,    "D");
			strcpy (inmr_rec.category, "DELETED    ");
		}

		/*-----------------------
		| Find customer record. |
		-----------------------*/
		cumr_rec.hhcu_hash 	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, "999999");
			strcpy (cumr_rec.class_type, "999");
		}

		/*------------------------
		| Find warehouse record. |
		------------------------*/
		ccmr_rec.hhcc_hash	=	coln_rec.incc_hash;
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	
		freight_other = (EXCH_OK) ? DPP (freight_other / X_RATE) : 0.00;
		ex_disc      = (EXCH_OK) ? DPP (cohr_rec.ex_disc / X_RATE) : 0.00;
		if (ex_disc != 0.00)
		{
			ProcessOtherCharges ("EXTRA DISC ", 0.0, ex_disc);
			ex_disc = 0.00;
		}

		if (freight_other != 0.00)
		{
			ProcessOtherCharges ("FR+INS+OTHE", freight_other, 0.0);
			freight_other = 0.00;
		}

		/*---------------------------------
		| Get and update or Add Inventory |
		| Movements Transactions record.  |
		---------------------------------*/
		sprintf (sale_rec.key, 
				"%2.2s%2.2s%2.2s%2.2s",
				cohr_rec.co_no,
				cohr_rec.br_no,
				cohr_rec.dp_no,
				ccmr_rec.cc_no);

		strcpy (sale_rec.category,  inmr_rec.category);
		strcpy (sale_rec.sman,      coln_rec.sman_code);
		strcpy (sale_rec.area,      cohr_rec.area_code);
		strcpy (sale_rec.ctype,     cumr_rec.class_type);
		strcpy (sale_rec.dbt_no,    cumr_rec.dbt_no);
		strcpy (sale_rec.year_flag, "C");
		strcpy (sale_rec.period,    storeMonth);
		newSalesRecord = find_rec (sale, &sale_rec, EQUAL, "u");
		/*------------------------ 
		| Not on file so create .|
		------------------------*/
		/*--------------------------------------------------------
		| If a contract price was used for this line item and the |
		| contract price has an agreed supplier cost associated   |
		| with it then use the contract agreed supplier cost for  |
		| sales analysis cost of sales.                           |
		---------------------------------------------------------*/

		if (coln_rec.cont_status > 0)
		{
			strcpy (cnch_rec.co_no, 	cohr_rec.co_no);
			strcpy (cnch_rec.cont_no, 	cohr_rec.cont_no);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cnch, "DBFIND");

			cncd_rec.hhch_hash = cnch_rec.hhch_hash;
			cncd_rec.hhbr_hash = coln_rec.hhbr_hash;
			if (cnch_rec.exch_type [0] == 'F')
				strcpy (cncd_rec.curr_code, cumr_rec.curr_code);
			else
				strcpy (cncd_rec.curr_code, currencyCode);

			cc = find_rec (cncd, &cncd_rec, EQUAL, "r");
			if (!cc)
			{
				if (cncd_rec.hhsu_hash > 0L)
				{
					if (multiCurrency)
					{
						sumr_rec.hhsu_hash = cncd_rec.hhsu_hash;
						cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
						if (cc)
							file_err (cc, pocr, "DBFIND");
						else
							strcpy (wkCurr, sumr_rec.curr_code);
	
						strcpy (pocr_rec.co_no,	cohr_rec.co_no);
						strcpy (pocr_rec.code,	wkCurr);
						cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
						if (cc)
							file_err (cc, pocr, "DBFIND");
						else
							wkFactor = pocr_rec.ex1_factor;
					}
					else
						wkFactor = 1.00;

					wk_value = out_cost ((cncd_rec.cost / wkFactor), 
							    		inmr_rec.outer_size);
				}
				else
					wk_value = out_cost (coln_rec.cost_price, 
							    		inmr_rec.outer_size);
			}
			else
				wk_value = out_cost (coln_rec.cost_price, 
						    		inmr_rec.outer_size);
		}
		else
			wk_value = out_cost (coln_rec.cost_price, 
					    		inmr_rec.outer_size);

		tot_sale = (coln_rec.gross + coln_rec.erate_var);
		tot_disc = coln_rec.amt_disc;

		if (coln_rec.q_order == 0.0)
		{
			tot_avge = 0.00;
			tot_ocst = 0.00;
		}
		else
		{
			tot_avge = (wk_value * (double) coln_rec.q_order);
			tot_ocst = DPP ((coln_rec.on_cost * (double) coln_rec.q_order) * X_RATE);
			*oncost += tot_ocst;
		}

		if (gstDivide != 0.00)
		{
			tot_sale = OutGst (tot_sale);
			tot_disc = OutGst (tot_disc);
		}

		loc_sale = (EXCH_OK) ? DPP (tot_sale / X_RATE) : 0.00;
		loc_disc = (EXCH_OK) ? DPP (tot_disc / X_RATE) : 0.00;
		loc_ocst = (EXCH_OK) ? DPP (tot_ocst / X_RATE) : 0.00;

		if (cohr_rec.drop_ship [0] == 'Y' && salesOnCosts)
		{
			loc_sale = loc_sale - loc_disc - loc_ocst;
			loc_disc = 0.00;
			tot_avge = tot_avge - loc_ocst;
		}

		if (newSalesRecord)
		{
			sale_rec.units     = 0.00;
			sale_rec.cost_sale = 0.00;
			sale_rec.gross     = 0.00;
			sale_rec.disc      = 0.00;

			if (INVOICE)
			{
	    		sale_rec.units     = (double) coln_rec.q_order;
	    		sale_rec.gross     = DPP (loc_sale);
	    		sale_rec.cost_sale = DPP (tot_avge);
	    		sale_rec.disc      = DPP (loc_disc);
			}
			else
			{
	    		sale_rec.units     = (double) 0.00;
	    		sale_rec.cost_sale = (double) 0.00;
	    		sale_rec.gross     = DPP (loc_sale * -1);
	    		sale_rec.disc      = DPP (loc_disc * -1);
				if (!DOLLAR)
				{
	    			sale_rec.units = (double) coln_rec.q_order * -1;
					if (!FAULTY)
	    				sale_rec.cost_sale = DPP (tot_avge * -1);
				}
			}
   	    	cc = abc_add (sale, &sale_rec);
    		if (cc) 
      	 		file_err (cc, sale, "DBADD");
		}
		else 
		{
   	    	if (INVOICE)
           	{
   	    		sale_rec.units     += (double) coln_rec.q_order;
    			sale_rec.gross     += DPP (loc_sale);
    			sale_rec.cost_sale += DPP (tot_avge);
    			sale_rec.disc      += DPP (loc_disc);
   	    	}
    		else
    		{
	    		sale_rec.gross -= DPP (loc_sale);
	    		sale_rec.disc  -= DPP (loc_disc);
				if (!DOLLAR)
				{
	    			sale_rec.units -= (double) coln_rec.q_order;
					if (!FAULTY)
	    				sale_rec.cost_sale -= DPP (tot_avge);
				}
	       	}
	       	cc = abc_update (sale, &sale_rec);
	       	if (cc) 
	     		file_err (cc, sale, "DBUPDATE");
		}

		if (salesAnalysisHistory)
		{
			AddCush
			 (
				cumr_rec.hhcu_hash, 
				inmr_rec.item_no,
				coln_rec.item_desc,
				coln_rec.q_order, 
				 (tot_sale - tot_disc),
				coln_rec.disc_pc, 
				comm_rec.dbt_date
			);
		}

		if (salesAnalysisByItem)
		{
			ProcessDetail
			 (
				coln_rec.q_order,
				DPP (loc_sale - loc_disc), 
				DPP (tot_avge)
			);
		}

		if (userDefinedSales)
		{
			ProcessUserDefined
			 (
				coln_rec.q_order,
				DPP (loc_sale - loc_disc), 
				DPP (tot_avge)
			);
		}
		if (teleSalesInstalled)
		{
			ProcessTeleSales
			 (
				coln_rec.q_order,
				DPP (loc_sale),
				DPP (loc_disc),
				DPP (tot_avge)
			);
		}

		abc_unlock (coln);
		cc = find_rec (coln, &coln_rec, NEXT, "u");
   	}
	abc_unlock (coln);

	strcpy (cohr_rec.stat_flag, updateStatusFlag);
	cc = abc_update (cohr, &cohr_rec);
	if (cc) 
      	 file_err (cc, cohr, "DBUPDATE");
}

/*=======================================
| Process Sales analysis detailed file. |
=======================================*/
void
ProcessDetail (
	float	_qty,
	double	_sale,
	double	_cost)
{
	_sale	=	DPP (_sale);
	_cost	=	DPP (_cost);

	strcpy (sadf_rec.co_no, cohr_rec.co_no);
	strcpy (sadf_rec.br_no, cohr_rec.br_no);
	strcpy (sadf_rec.year,  "C");
	sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sadf_rec.hhcu_hash = cohr_rec.hhcu_hash;
	strcpy (sadf_rec.sman, coln_rec.sman_code);
	strcpy (sadf_rec.area, cohr_rec.area_code);
	cc = find_rec (sadf, &sadf_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (sadf);
		memset (&sadf_rec, 0, sizeof (sadf_rec));

		strcpy (sadf_rec.co_no, cohr_rec.co_no);
		strcpy (sadf_rec.br_no, cohr_rec.br_no);
		strcpy (sadf_rec.year,  "C");
		sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sadf_rec.hhcu_hash = cohr_rec.hhcu_hash;
		strcpy (sadf_rec.sman, coln_rec.sman_code);
		strcpy (sadf_rec.area, cohr_rec.area_code);

 		if (INVOICE)
		{
			sadf_qty_per [invoiceMonth] = _qty;
			sadf_sal_per [invoiceMonth] = DOLLARS (_sale);
			sadf_cst_per [invoiceMonth] = DOLLARS (_cost);
		}
		else
		{
			sadf_qty_per [invoiceMonth] = 0.00;
			sadf_cst_per [invoiceMonth] = 0.00;
			sadf_sal_per [invoiceMonth] = 0 - DOLLARS (_sale) ;
			if (!DOLLAR)
			{
				sadf_qty_per [invoiceMonth] = 0 - _qty;
				if (!FAULTY)
					sadf_cst_per [invoiceMonth] = 0 - DOLLARS (_cost);
			}
		}
		cc = abc_add (sadf, &sadf_rec);
		if (cc) 
	      	 	file_err (cc, sadf, "DBADD");
	}
	else
	{
		if (INVOICE)
		{
			sadf_qty_per [invoiceMonth] += _qty;
			sadf_sal_per [invoiceMonth] +=  DOLLARS (_sale);
			sadf_cst_per [invoiceMonth] +=  DOLLARS (_cost);
		}
		else
		{
			sadf_sal_per [invoiceMonth] -= DOLLARS (_sale);
			if (!DOLLAR)
			{
				sadf_qty_per [invoiceMonth] -= _qty;
				if (!FAULTY)
			    	      sadf_cst_per [invoiceMonth] -= DOLLARS (_cost);
			}
		}
		cc = abc_update (sadf, &sadf_rec);
		if (cc) 
			file_err (cc, sadf, "DBUPDATE");
	}
}
/*=======================================
| Process Sales analysis detailed file. |
=======================================*/
void
ProcessUserDefined (
 float		_qty,
 double		_sale,
 double		_cost)
{
	int		ByItem	=	FALSE;

	_sale	=	DPP (_sale);
	_cost	=	DPP (_cost);

	iudi_rec.hhcf_hash	=	0L;
	iudi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	iudi_rec.spec_no	=	0;
	strcpy (iudi_rec.code, "  ");
	cc = find_rec (iudi, &iudi_rec, GTEQ, "u");
	while (!cc && iudi_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		abc_unlock (iudi);
		ByItem	=	TRUE;
		strcpy (saud_rec.co_no, cohr_rec.co_no);
		strcpy (saud_rec.year,  "C");
		saud_rec.spec_no	=	iudi_rec.spec_no;
		strcpy (saud_rec.code,  iudi_rec.code);
		cc = find_rec (saud, &saud_rec, EQUAL, "r");
		if (cc)
		{
			memset (&saud_rec, 0, sizeof (saud_rec));

			strcpy (saud_rec.co_no, cohr_rec.co_no);
			strcpy (saud_rec.year,  "C");
			saud_rec.spec_no	=	iudi_rec.spec_no;
			strcpy (saud_rec.code,  iudi_rec.code);

			if (INVOICE)
			{
				saud_qty_per [invoiceMonth] = _qty;
				saud_sal_per [invoiceMonth] = DOLLARS (_sale);
				saud_cst_per [invoiceMonth] = DOLLARS (_cost);
			}
			else
			{
				saud_qty_per [invoiceMonth] = 0.00;
				saud_cst_per [invoiceMonth] = 0.00;
				saud_sal_per [invoiceMonth] = 0 - DOLLARS (_sale) ;
				if (!DOLLAR)
				{
					saud_qty_per [invoiceMonth] = 0 - _qty;
					if (!FAULTY)
						saud_cst_per [invoiceMonth] = 0 - DOLLARS (_cost);
				}
			}
			cc = abc_add (saud, &saud_rec);
			if (cc) 
	      	 	file_err (cc, saud, "DBADD");
		}
		else
		{
			if (INVOICE)
			{
				saud_qty_per [invoiceMonth] += _qty;
				saud_sal_per [invoiceMonth] +=  DOLLARS (_sale);
				saud_cst_per [invoiceMonth] +=  DOLLARS (_cost);
			}
			else
			{
				saud_sal_per [invoiceMonth] -= DOLLARS (_sale);
				if (!DOLLAR)
				{
					saud_qty_per [invoiceMonth] -= _qty;
					if (!FAULTY)
						  saud_cst_per [invoiceMonth] -= DOLLARS (_cost);
				}
			}
			cc = abc_update (saud, &saud_rec);
			if (cc) 
				file_err (cc, saud, "DBUPDATE");
		}
		cc = find_rec (iudi, &iudi_rec, NEXT, "r");
	}

	if (ByItem	==	TRUE)
		return;

	strcpy (excf_rec.co_no, cohr_rec.co_no);
	strcpy (excf_rec.cat_no, inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		return;

	iudi_rec.hhcf_hash	=	excf_rec.hhcf_hash;
	iudi_rec.hhbr_hash	=	0L;
	iudi_rec.spec_no	=	0;
	strcpy (iudi_rec.code, "  ");
	cc = find_rec (iudi, &iudi_rec, GTEQ, "r");
	while (!cc && iudi_rec.hhcf_hash == excf_rec.hhcf_hash)
	{
		strcpy (saud_rec.co_no, cohr_rec.co_no);
		strcpy (saud_rec.year,  "C");
		saud_rec.spec_no	=	iudi_rec.spec_no;
		strcpy (saud_rec.code,  iudi_rec.code);
		cc = find_rec (saud, &saud_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock ("saud");
			memset (&saud_rec, 0, sizeof (saud_rec));

			strcpy (saud_rec.co_no, cohr_rec.co_no);
			strcpy (saud_rec.year,  "C");
			saud_rec.spec_no	=	iudi_rec.spec_no;
			strcpy (saud_rec.code,  iudi_rec.code);

			if (INVOICE)
			{
				saud_qty_per [invoiceMonth] = _qty;
				saud_sal_per [invoiceMonth] = DOLLARS (_sale);
				saud_cst_per [invoiceMonth] = DOLLARS (_cost);
			}
			else
			{
				saud_qty_per [invoiceMonth] = 0.00;
				saud_cst_per [invoiceMonth] = 0.00;
				saud_sal_per [invoiceMonth] = 0 - DOLLARS (_sale) ;
				if (!DOLLAR)
				{
					saud_qty_per [invoiceMonth] = 0 - _qty;
					if (!FAULTY)
						saud_cst_per [invoiceMonth] = 0 - DOLLARS (_cost);
				}
			}
			cc = abc_add (saud, &saud_rec);
			if (cc) 
	      	 	file_err (cc, saud, "DBADD");
		}
		else
		{
			if (INVOICE)
			{
				saud_qty_per [invoiceMonth] += _qty;
				saud_sal_per [invoiceMonth] +=  DOLLARS (_sale);
				saud_cst_per [invoiceMonth] +=  DOLLARS (_cost);
			}
			else
			{
				saud_sal_per [invoiceMonth] -= DOLLARS (_sale);
				if (!DOLLAR)
				{
					saud_qty_per [invoiceMonth] -= _qty;
					if (!FAULTY)
						  saud_cst_per [invoiceMonth] -= DOLLARS (_cost);
				}
			}
			cc = abc_update (saud, &saud_rec);
			if (cc) 
				file_err (cc, saud, "DBUPDATE");
		}
		cc = find_rec (iudi, &iudi_rec, NEXT, "r");
	}
}

/*=====================
| Process Tele Sales. |
=====================*/
void 
ProcessTeleSales (
	float	_qty,
	double	_sale,
	double	_disc,
	double	_cost)
{
	tshs_rec.hhcu_hash 	= cohr_rec.hhcu_hash;
	tshs_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	tshs_rec.date 		= lsystemDate;
	strcpy (tshs_rec.stat_flag, "0");

	_sale	=	DPP (_sale);
	_disc	=	DPP (_disc);
	_cost	=	DPP (_cost);

	cc = find_rec (tshs, &tshs_rec, EQUAL, "u");
	if (cc)
	{
		tshs_rec.qty 		= (INVOICE) ? _qty 	: 0 - _qty;
		tshs_rec.sale_price	= (INVOICE) ? _sale : 0 - _sale;
		tshs_rec.disc		= (INVOICE) ? _disc : 0 - _disc;
		tshs_rec.cost_price	= (INVOICE) ? _cost : 0 - _cost;

		abc_unlock (tshs);

		cc = abc_add (tshs, &tshs_rec);
		if (cc) 
			file_err (cc, tshs, "DBADD");
	}
	else
	{
		if (INVOICE)
		{
			tshs_rec.qty 		+= _qty;
			tshs_rec.sale_price	+= _sale;
			tshs_rec.disc		+= _disc;
			tshs_rec.cost_price	+= _cost;
		}
		else
		{
			tshs_rec.sale_price	-= _sale;
			tshs_rec.disc		-= _disc;
			if (!DOLLAR)
			{
				tshs_rec.qty -= _qty;
				if (!FAULTY)
					tshs_rec.cost_price -= _cost;
			}
		}
		cc = abc_update (tshs, &tshs_rec);
		if (cc) 
			file_err (cc, tshs, "DBUPDATE");
	}
	return;
}

void 
ProcessOtherCharges (
 char	*cat_desc,
 double	sa1_amt,
 double	sa2_amt)
{
	/*---------------------------------
	| Get and update or Add Inventory |
	| Movements Transactions record.  |
	---------------------------------*/
	sprintf (sale_rec.key, 
			"%2.2s%2.2s%2.2s%2.2s",
			cohr_rec.co_no,
			cohr_rec.br_no,
			cohr_rec.dp_no,
			ccmr_rec.cc_no);

	sprintf (sale_rec.category, "%-11.11s",  cat_desc);
	strcpy (sale_rec.sman,      cohr_rec.sale_code);
	strcpy (sale_rec.area,      cohr_rec.area_code);
	strcpy (sale_rec.ctype,     cumr_rec.class_type);
	strcpy (sale_rec.dbt_no,    cumr_rec.dbt_no);
	strcpy (sale_rec.year_flag, "C");
	strcpy (sale_rec.period,    storeMonth);
	cc = find_rec (sale, &sale_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (sale);
		memset (&sale_rec, 0, sizeof (sale_rec));

		sprintf (sale_rec.key, 
				"%2.2s%2.2s%2.2s%2.2s",
				cohr_rec.co_no,
				cohr_rec.br_no,
				cohr_rec.dp_no,
				ccmr_rec.cc_no);

		sprintf (sale_rec.category, "%-11.11s",  cat_desc);
		strcpy (sale_rec.sman,      cohr_rec.sale_code);
		strcpy (sale_rec.area,      cohr_rec.area_code);
		strcpy (sale_rec.ctype,     cumr_rec.class_type);
		strcpy (sale_rec.dbt_no,    cumr_rec.dbt_no);
		strcpy (sale_rec.year_flag, "C");
		strcpy (sale_rec.period,    storeMonth);

		sale_rec.units = 1;
		sale_rec.cost_sale = 0.0;
		if (INVOICE)
		{
			sale_rec.gross = DPP (sa1_amt);
			sale_rec.disc =  DPP (sa2_amt);
		}
		else
		{
			sale_rec.gross = DPP (sa1_amt * -1);
			sale_rec.disc =  DPP (sa2_amt * -1);
		}

		cc = abc_add (sale, &sale_rec);
		if (cc) 
				file_err (cc, sale, "DBADD");
	}
	else 
	{
		sale_rec.units     += 1;
		sale_rec.cost_sale += 0.0;
		if (INVOICE)
		{
			sale_rec.gross += DPP (sa1_amt);
			sale_rec.disc  += DPP (sa2_amt);
		}
		else
		{
			sale_rec.gross -= DPP (sa1_amt); 
			sale_rec.disc  -= DPP (sa2_amt); 
		}

		cc = abc_update (sale, &sale_rec);
		if (cc) 
			file_err (cc, sale, "DBUPDATE");
	}
}

/*=======================================================
| Add/Update Customer 12 months sales analysis records. |
=======================================================*/
void 
AddCusa (
 double		tot_ocst)
{
	double	sa_amt = 0.00;

	sa_amt = DPP (cohr_rec.gross - cohr_rec.disc + cohr_rec.erate_var);

	if (gstDivide != 0.00)
		sa_amt = OutGst (sa_amt);

	if (cohr_rec.drop_ship [0] == 'Y' && salesOnCosts)
		sa_amt -= tot_ocst;

	/*-------------------------------------------------------
	| Get and update or Add Customer Sales Analysis record. |
	-------------------------------------------------------*/
	cusa_rec.hhcu_hash = cohr_rec.hhcu_hash;
	strcpy (cusa_rec.year, "C");
	
	cc = find_rec (cusa, &cusa_rec, EQUAL, "u");
	/*-------------------------
	| Not on file so create . |
	-------------------------*/
	if (cc)
	{
		abc_unlock ("cusa");
		memset (&cusa_rec, 0, sizeof (cusa_rec));

		cusa_rec.hhcu_hash = cohr_rec.hhcu_hash;
		strcpy (cusa_rec.year, "C");
		
		if (INVOICE)
			cusa_val [invoiceMonth] = DPP (sa_amt);
		else
			cusa_val [invoiceMonth] = DPP (sa_amt * -1);

		strcpy (cusa_rec.stat_flag, "0");
		cc = abc_add (cusa, &cusa_rec);
		if (cc) 
			file_err (cc, cusa, "DBADD");
	}
	else 
	{
		if (INVOICE)
			cusa_val [invoiceMonth] += DPP (sa_amt);
		else
			cusa_val [invoiceMonth] -= DPP (sa_amt);

		cc = abc_update (cusa, &cusa_rec);
		if (cc) 
			file_err (cc, cusa, "DBUPDATE");

	}
}

/*===========================================
| Extract out GST for gst inclusive Prices. |
===========================================*/
double 
OutGst (
 double	 	total_amt)
{
	double	gst_amount = 0.00;

	if (total_amt == 0.00)
		return (0.00);

	if (NOTAX)
		return (total_amt);

	gst_amount = DPP (total_amt / gstDivide);
	
	total_amt -= DPP (gst_amount);

	return (total_amt);
}

void
AddCush (
	long	hhcuHash,
	char	*item,
	char	*desc,
	float	qty,
	double	sale,
	float	disc,
	long	up_date)
{
	int		i;

	for (i = salesAnalysisHistory; i >= 0; i--)
	{
		cush_rec.hhcu_hash	= hhcuHash;
		cush_rec.line_no 	= i;
		if (find_rec (cush, &cush_rec, EQUAL, "u"))
		{
			abc_unlock (cush);
			continue;
		}

		if (i == salesAnalysisHistory)
		{
			cc = abc_delete (cush);
			if (cc)
	      	 		file_err (cc, cush, "DBDELETE");
			continue;
		}

		cush_rec.line_no = i + 1;
		cc = abc_update (cush, &cush_rec);
		if (cc)
			file_err (cc, cush, "DBUPDATE");
	}
	cush_rec.hhcu_hash 	= hhcuHash;
	cush_rec.line_no 	= 0;
	cc = find_rec (cush, &cush_rec, EQUAL, "u");

	sprintf (cush_rec.item_no, "%-16.16s", item);
	sprintf (cush_rec.item_desc,"%-40.40s",desc);
	cush_rec.item_qty 	= qty;
	cush_rec.item_price = DPP (sale);
	cush_rec.item_disc 	= DPP (disc);
	cush_rec.pur_date 	= up_date;

	if (cc)
	{
		abc_unlock (cush);
		cc = abc_add (cush, &cush_rec);
		if (cc)
			file_err (cc, cush, "DBADD");
	}
	else
	{
		cc = abc_update (cush, &cush_rec);
		if (cc)
	      	 	file_err (cc, cush, "DBUPDATE");
	}
}

int	
CalcMonth (
 long	InvDate)
{
	int		monthPeriod;

	DateToDMY (InvDate, NULL, &monthPeriod, NULL);
	return (monthPeriod);
}

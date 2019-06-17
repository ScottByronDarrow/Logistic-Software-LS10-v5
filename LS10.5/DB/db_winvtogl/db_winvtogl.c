/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_winvtogl.c,v 5.1 2001/12/07 04:06:43 scott Exp $
|  Program Name  : (db_winvtogl.c & db_wcrdtogl.c)
|  Program Desc  : (Update General Ledger Work Transactions From) 
|                 (Invoices File (cuwk), Set tran_flag to 2. )
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_winvtogl.c,v $
| Revision 5.1  2001/12/07 04:06:43  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_winvtogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_winvtogl/db_winvtogl.c,v 5.1 2001/12/07 04:06:43 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<twodec.h>
#include 	<ml_db_mess.h>

#define		INVOICE		(invoiceCreditType [0] == '1')
#define		EXCH_OK		(cuwk_rec.exch_rate != 0.00)

FILE	*fsort;

#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cuwkRecord	cuwk_rec;

	char	*srt_offset [128];

	char	*data = "data";

	int		glwk_no			= 0,	
			dataFind 		= FALSE,
			envDbMcurr 		= FALSE,
			envDisOk 		= TRUE,
			envGlOncost 	= FALSE,
			envDbFind  		= FALSE,
			envDbCo			= FALSE,
			envGst 			= FALSE,
			envGlByClass 	= TRUE,
			openSortFile 	= FALSE,
			printerNumber	= 0,
			firstCustomer 	= TRUE;

   	char 	branchNumber		 [3],
			invoiceCreditType	 [2],
			systemDate			 [11],
			previousInvoice		 [9],
			previousCustomer	 [7],
			gstCode				 [4];

	double	CalcFx 			=	0.00,
			CalcLoc			=	0.00,
			LocalPosting 	= 	0.00;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	AddToSort 		(void);
void 	ProcessSorted 	(void);
void 	Update 			(void);
void 	WriteGlwk 		(char *,char *,char *,char *,char *,char *,long,double,double);
void 	PostDebit 		(char *, char *, char *, double, double);
void 	shutdown_prog 	(void);
char 	*_SortRead 		(FILE *);

int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	if (argc < 3)
	{
		print_at (0,0, mlDbMess066, argv [0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [2]);
	if (argv [1][0] != 'C' && argv [1][0] != 'I')
	{
		print_at (0,0, mlDbMess066, argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = get_env ("DIS_OK");
	envDisOk = (sptr == (char *)0) ? 0 : atoi (sptr);
	envDisOk = atoi (get_env ("DIS_OK"));

	/*----------------
	| Check for GST. |
	----------------*/
	sptr = get_env ("GST");
	envGst = (*sptr == 'Y' || *sptr == 'y') ? TRUE : FALSE;

	if (envGst)
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (gstCode, "%-3.3s", "TAX");

	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*-----------------------------------
	| Check for multi-currency customer |
	-----------------------------------*/
	sptr = chk_env ("DB_FIND");
	envDbFind = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envDbCo = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("GL_ONCOST");
	envGlOncost = (sptr == (char *)0) ? FALSE : atoi (sptr);

	strcpy (systemDate, DateToString (TodaysDate ()));
	strcpy (glwkRec.name, "                              ");

	OpenDB ();


	init_scr ();
	print_mess (ML ("Creating Customer Sales Journals."));

	strcpy (branchNumber, comm_rec.est_no);

	if (argc > 3)
		strcpy (branchNumber, argv [3]);

	if (argv [1][0] == 'I')
		invoiceCreditType [0] = '1';
	else
		invoiceCreditType [0] = '2';

	/*----------------------------------
	| Process receipts transactions  . |
	----------------------------------*/
	strcpy (cuwk_rec.co_no,  comm_rec.co_no);
	strcpy (cuwk_rec.est,    branchNumber);
	strcpy (cuwk_rec.inv_no, "        ");
	cc = find_rec (cuwk, &cuwk_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (cuwk_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cuwk_rec.est, branchNumber))
	{
		if (!strcmp (cuwk_rec.stat_flag, "0") &&
		     !strcmp (cuwk_rec.type, invoiceCreditType))
		{
			/*-------------------------------------
			| Create General Ledger Work Records. |
			-------------------------------------*/
			AddToSort ();
			dataFind = TRUE;

			abc_unlock (cuwk);
			/*----------------------
			| Try twice to delete. |
			----------------------*/
			if ((cc = abc_delete (cuwk)))
				cc = abc_delete (cuwk);
			if (cc)
			{
				/*---------------------------------------------
				| INDEX ON FILE CUWK IS BAD PLEASE RUN BCHECK |
				---------------------------------------------*/
				errmess (ML (mlDbMess065));
				sleep (20);
			}
			cc = find_rec (cuwk, &cuwk_rec, GTEQ, "u");
		}
		else
		{	
			abc_unlock (cuwk);
			cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
		}
	}
	abc_unlock (cuwk);
	if (dataFind)
	{
		fsort = sort_sort (fsort, "cuwksrt");

		ProcessSorted ();
		sort_delete (fsort, "cuwksrt");

		shutdown_prog ();
	}
	else
	{
		shutdown_prog ();
	}
	return (EXIT_SUCCESS);
}

/*======================
| Open Datebase Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cuwk, cuwk_list, CUWK_NO_FIELDS, "cuwk_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
					(envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=======================
| Close Datebase Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cuwk);
	abc_fclose (cumr);
	GL_CloseBatch (printerNumber);
	GL_Close ();

	abc_dbclose (data);
}

/*--------------------------------
| Add current cuwk to sort file. |
--------------------------------*/
void
AddToSort (void)
{
    char sortTemp [1024];

	if (!openSortFile)
	{
		fsort = sort_open ("cuwksrt");
		openSortFile = TRUE;
	}

    sprintf (sortTemp,
		"%-6.6s%c%-8.8s%c%s%c%-*.*s%c%-*.*s%c%ld%c%2.2s%c%-20.20s%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%-3.3s%c%f%c%s%c\n",
		cuwk_rec.dbt_no,			1,	/* srt_offset = 0	*/
		cuwk_rec.inv_no,			1,	/* srt_offset = 1	*/
		cuwk_rec.cus_po_no,			1,	/* srt_offset = 2	*/
		MAXLEVEL, MAXLEVEL,
		cuwk_rec.gl_acc_no,			1,	/* srt_offset = 3	*/
		MAXLEVEL,MAXLEVEL,
		cuwk_rec.gl_control,		1,	/* srt_offset = 4	*/
		cuwk_rec.date_of_inv,		1,	/* srt_offset = 5	*/
		cuwk_rec.period_no,			1,	/* srt_offset = 6	*/
		cuwk_rec.narrative,			1,	/* srt_offset = 7	*/
		cuwk_rec.exch_rate,			1,	/* srt_offset = 8	*/
		cuwk_rec.tot_loc,			1,	/* srt_offset = 9	*/
		cuwk_rec.tot_fx,			1,	/* srt_offset = 10	*/
		cuwk_rec.disc,				1,	/* srt_offset = 11	*/
		cuwk_rec.loc_amt,			1,	/* srt_offset = 12	*/
		cuwk_rec.fx_amt,			1,	/* srt_offset = 13	*/
		cuwk_rec.freight,			1,	/* srt_offset = 14	*/
		cuwk_rec.insurance,			1,	/* srt_offset = 15	*/
		cuwk_rec.other_cost1,		1,	/* srt_offset = 16	*/
		cuwk_rec.other_cost2,		1,	/* srt_offset = 17	*/
		cuwk_rec.other_cost3,		1,	/* srt_offset = 18	*/
		cuwk_rec.sos,				1,	/* srt_offset = 19	*/
		cuwk_rec.restock_fee,		1,	/* srt_offset = 20	*/
		cuwk_rec.gst,				1,	/* srt_offset = 21	*/
		cuwk_rec.tax,				1,	/* srt_offset = 22	*/
		cuwk_rec.dd_oncost, 		1,	/* srt_offset = 23	*/
		cuwk_rec.exch_var, 			1,	/* srt_offset = 24	*/
		cuwk_rec.currency,			1,	/* srt_offset = 25	*/
		cuwk_rec.item_levy,			1,	/* srt_offset = 26	*/
		cuwk_rec.gl_levy,			1);	/* srt_offset = 27	*/

    sort_save (fsort, sortTemp);
}

/*----------------------
| Process sorted data. |
----------------------*/
void
ProcessSorted (void)
{
	char	*sptr;

	strcpy (previousInvoice, "");
	strcpy (previousCustomer, "");
	firstCustomer = TRUE;

	sptr = _SortRead (fsort);
	while (sptr)
	{
		if (firstCustomer)
		{
			sprintf (previousCustomer, "%-6.6s", srt_offset [0]);
			firstCustomer = FALSE;
		}
		strcpy (cuwk_rec.co_no,      comm_rec.co_no);
		strcpy (cuwk_rec.est,        branchNumber);
		sprintf (cuwk_rec.dbt_no,    "%-6.6s",   srt_offset [0]);
		sprintf (cuwk_rec.inv_no,    "%-8.8s",   srt_offset [1]);
		sprintf (cuwk_rec.gl_acc_no, "%-*.*s",   
								MAXLEVEL,MAXLEVEL, srt_offset [3]);
		sprintf (cuwk_rec.gl_control,"%-*.*s",   
								MAXLEVEL,MAXLEVEL, srt_offset [4]);
		sprintf (cuwk_rec.period_no, "%2.2s",    srt_offset [6]);
		sprintf (cuwk_rec.narrative, "%-20.20s", srt_offset [7]);
		cuwk_rec.date_of_inv        = atol (srt_offset [5]);
		cuwk_rec.exch_rate   		= atof (srt_offset [8]);
		cuwk_rec.tot_loc			= atof (srt_offset [9]);
		cuwk_rec.tot_fx				= atof (srt_offset [10]);
		cuwk_rec.disc        		= atof (srt_offset [11]);
		cuwk_rec.loc_amt       		= atof (srt_offset [12]);
		cuwk_rec.fx_amt       		= atof (srt_offset [13]);
		cuwk_rec.freight     		= atof (srt_offset [14]);
		cuwk_rec.insurance   		= atof (srt_offset [15]);
		cuwk_rec.other_cost1   		= atof (srt_offset [16]);
		cuwk_rec.other_cost2   		= atof (srt_offset [17]);
		cuwk_rec.other_cost3   		= atof (srt_offset [18]);
		cuwk_rec.sos         		= atof (srt_offset [19]);
		cuwk_rec.restock_fee 		= atof (srt_offset [20]);
		cuwk_rec.gst         		= atof (srt_offset [21]);
		cuwk_rec.tax         		= atof (srt_offset [22]);
		cuwk_rec.dd_oncost     		= atof (srt_offset [23]);
		cuwk_rec.exch_var      		= atof (srt_offset [24]);
		cuwk_rec.item_levy     		= atof (srt_offset [26]);
		sprintf (cuwk_rec.currency, "%-3.3s", srt_offset [25]);
		sprintf (cuwk_rec.gl_levy, "%-*.*s",   
								MAXLEVEL,MAXLEVEL, srt_offset [27]);
		sprintf (cuwk_rec.cus_po_no, "%-16.16s", srt_offset [2]);
		Update ();

		sptr = _SortRead (fsort);
	}
}

/*=======================
| Transaction records . |
=======================*/
void
Update (void)
{
	int		dbt_period;
	double	CalcDiff = 0.00;
	char	*sptr;
	
	if (!INVOICE)
	{
		/*---------------------------------
		| Reverse All Amounts For Credit. |
		---------------------------------*/
		cuwk_rec.disc    		= cuwk_rec.disc 		* -1;
		cuwk_rec.tax     		= cuwk_rec.tax 			* -1;
		cuwk_rec.freight 		= cuwk_rec.freight 		* -1;
		cuwk_rec.restock_fee    = cuwk_rec.restock_fee 	* -1;
		cuwk_rec.gst     		= cuwk_rec.gst 			* -1;
		cuwk_rec.tot_loc   		= cuwk_rec.tot_loc 		* -1;
		cuwk_rec.tot_fx    		= cuwk_rec.tot_fx 		* -1;
		cuwk_rec.fx_amt   		= cuwk_rec.fx_amt 		* -1;
		cuwk_rec.loc_amt   		= cuwk_rec.loc_amt 		* -1;
		cuwk_rec.insurance   	= cuwk_rec.insurance 	* -1;
		cuwk_rec.other_cost1   	= cuwk_rec.other_cost1 	* -1;
		cuwk_rec.other_cost2   	= cuwk_rec.other_cost2 	* -1;
		cuwk_rec.other_cost3   	= cuwk_rec.other_cost3 	* -1;
		cuwk_rec.insurance   	= cuwk_rec.dd_oncost 	* -1;
		cuwk_rec.sos   			= cuwk_rec.sos 			* -1;
		cuwk_rec.exch_var 		= cuwk_rec.exch_var		* -1;
		cuwk_rec.item_levy 		= cuwk_rec.item_levy	* -1;
	}

	/*---------------------------------------------------
	| If date is Zero then set to current customer date. |
	---------------------------------------------------*/
	if (cuwk_rec.date_of_inv == 0L)
		cuwk_rec.date_of_inv = comm_rec.dbt_date;

	DateToDMY (cuwk_rec.date_of_inv, NULL, &dbt_period, NULL);
	dbt_period--;

	/*--------------------------
	| Post debit for customer. |
	--------------------------*/
	if (strcmp (cuwk_rec.dbt_no, previousCustomer))
		strcpy (previousCustomer, cuwk_rec.dbt_no);

	/*------------------------------------------------------
	| Change in invoice so accumulate totals for customer. |
	------------------------------------------------------*/
	if (strcmp (cuwk_rec.inv_no, previousInvoice))
	{
		LocalPosting 	= 	0.00;
		/*-----------------------------------------------
		| Create A Record For Customer Control Account. |
		-----------------------------------------------*/
		strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");

		/*-----------------------------------------------
		| Create A Record For Customer Control Account. |
		-----------------------------------------------*/
		strcpy (cumr_rec.co_no,   comm_rec.co_no);
		if (!envDbFind)
			strcpy (cumr_rec.est_no,  cuwk_rec.est);

		if (envDbCo == 0) 	/* Company Owned */
			strcpy (cumr_rec.est_no,  " 0");

		strcpy (cumr_rec.dbt_no,  cuwk_rec.dbt_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		strcpy (glmrRec.co_no,  comm_rec.co_no);
		strcpy (glmrRec.acc_no, cumr_rec.gl_ctrl_acct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{	
			GL_GLI 
			(
				comm_rec.co_no,			/*	Company Number 	*/
				branchNumber,			/*	Branch No.		*/
				"  ",					/*	Warehouse No.	*/
				"ACCT REC  ",			/*	Interface Code. */
				(envGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
				" "
			);
			strcpy (cumr_rec.gl_ctrl_acct, glmrRec.acc_no);
		}
		CalcFx 	=	(envDisOk)  ? no_dec (cuwk_rec.tot_fx - cuwk_rec.disc)
				             	: no_dec (cuwk_rec.tot_fx);
		CalcFx	=	no_dec (CalcFx);
		if (envDisOk)
		{
			CalcLoc	=	no_dec (cuwk_rec.tot_loc);
			CalcLoc -=	no_dec (cuwk_rec.disc / cuwk_rec.exch_rate);
		}
		else
			CalcLoc	=	no_dec (cuwk_rec.tot_loc);
		
		WriteGlwk 
		(
			cuwk_rec.co_no,
			cuwk_rec.est,
			cuwk_rec.dbt_no,
			cuwk_rec.inv_no,
			cumr_rec.gl_ctrl_acct,
			cuwk_rec.period_no,
			cuwk_rec.date_of_inv,
			CalcFx,
			CalcLoc
		);

		strcpy (previousInvoice,cuwk_rec.inv_no);

		/*-------------------------------
		| Process posting of Sales tax. |
		-------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.tax);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "4" : "3",
					"SALES TAX ",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);

		/*-----------------------------
		| Process posting of Freight. |
		-----------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.freight);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"FREIGHT   ",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*----------------------------------
		| Process posting of Other cost 1. |
		----------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.other_cost1);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"OTHER CST1",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*----------------------------------
		| Process posting of Other cost 2. |
		----------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.other_cost2);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"OTHER CST2",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*----------------------------------
		| Process posting of Other cost 3. |
		----------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.other_cost3);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"OTHER CST3",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*-------------------------------
		| Process posting of Insurance. |
		-------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.insurance);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) 	? "6" : "5",
					"INSURANCE ",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*-------------------------------------------
		| Process posting of Small Order Surcharge. |
		-------------------------------------------*/
		CalcFx 	=	no_dec (cuwk_rec.sos);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"SOS       ",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		CalcFx 	=	no_dec (cuwk_rec.restock_fee);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"RESTCK FEE",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);

		CalcFx 	=	no_dec (cuwk_rec.gst);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "4" : "3",
					"G.S.T CHRG",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		CalcFx 	=	no_dec (cuwk_rec.dd_oncost);
		CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
		PostDebit
		(			 (INVOICE) ? "6" : "5",
					"DD CST SAL",
					(envGlByClass) 	? cumr_rec.class_type
								 	: cumr_rec.sman_code,
					CalcFx,
					CalcLoc
		);
		/*----------------------------------
		| Process posting of Exchange Var. |
		----------------------------------*/
		CalcFx 	=	0.0;
		CalcLoc	=	no_dec (cuwk_rec.exch_var);
		PostDebit
		(			
			(INVOICE) ? "1" : "2",
			"EXCH VAR. ",
			(envGlByClass) 	? cumr_rec.class_type : cumr_rec.sman_code,
			CalcFx,
			CalcLoc
		);

		if (envDisOk)
		{
			CalcFx 	=	no_dec (cuwk_rec.disc);
			CalcLoc	=	no_dec (CalcFx / cuwk_rec.exch_rate);
			PostDebit
			(		
				(INVOICE) ? "3" : "4",
				"DISCOUNT",
				(envGlByClass)	? cumr_rec.class_type
								: cumr_rec.sman_code,
				CalcFx,
				CalcLoc
			);
		}
	}
	/*----------------------
	| Create glwk records. |
	----------------------*/
	strcpy (glwkRec.jnl_type, (INVOICE) ? "6" : "5");
	CalcFx 	=	no_dec (cuwk_rec.item_levy);
	CalcLoc	=	CalcFx / cuwk_rec.exch_rate;
	WriteGlwk 
	(
		cuwk_rec.co_no,
		cuwk_rec.est,
		"      ",
		"      ",
		cuwk_rec.gl_levy,
		cuwk_rec.period_no,
		cuwk_rec.date_of_inv,
		CalcFx,
		CalcLoc
	);
	strcpy (cuwk_rec.inv_no,"        ");
	strcpy (cuwk_rec.dbt_no,"      ");

	/*-------------------------------------------------------------------
	| This code is to take into account of the fact that when the 		|
	| exchange rates are calculated for freight etc the exchange rate	|
	| rounding will cause a difference if the total of the invoice is	|
	| converted. 														|
	-------------------------------------------------------------------*/
	sptr = strstr (cuwk_rec.cus_po_no, "END");
	if (sptr != (char *) 0)
	{
        if (INVOICE)
            CalcDiff = no_dec (cuwk_rec.loc_amt + LocalPosting);
        else
            CalcDiff = no_dec (cuwk_rec.loc_amt - LocalPosting);
		if (CalcDiff != (double) 0.00)
			cuwk_rec.loc_amt -= CalcDiff;

		LocalPosting = 0.00;
	}

	/*----------------------
	| Create glwk records. |
	----------------------*/
	strcpy (glwkRec.jnl_type, (INVOICE) ? "2" : "1");
	WriteGlwk 
	(
		cuwk_rec.co_no,
		cuwk_rec.est,
		cuwk_rec.dbt_no,
		cuwk_rec.inv_no,
		cuwk_rec.gl_acc_no,
		cuwk_rec.period_no,
		cuwk_rec.date_of_inv,
		cuwk_rec.fx_amt,
		cuwk_rec.loc_amt + cuwk_rec.exch_var
	);
}

/*========================================
| Write General Ledger Work File Record. |
========================================*/
void	
WriteGlwk (
	 char	*co_no,		/* Company Number     */
	 char	*est_no,   	/* Branch Number      */
	 char	*dbt_no,	/* Customer Number    */
	 char	*inv_no,	/* Invoice Number     */
	 char	*acc_no,	/* G/L Account Number */
	 char	*period_no,	/* Period Number      */
	 long	d_inv,		/* Date Of Invoice    */
	 double	fx_amt,		/* Amount             */
	 double	loc_amt) 	/* Amount             */
{
	if (twodec (fx_amt) == 0 && twodec (loc_amt) == 0)
		return;

	if ((atoi (glwkRec.jnl_type) % 2) == 0)
		LocalPosting += loc_amt;
	else
		LocalPosting -= loc_amt;

	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,acc_no);
	if ((cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{	
		GL_GLI 
		(
			comm_rec.co_no,		/*	Company Number 	*/
			branchNumber,		/*	Branch No.		*/
			"  ",				/*	Warehouse No.	*/
			"SUSPENSE  ",		/*	Interface Code. */
			" ",				/*	Customer Type.	*/
			" "
		);	
		strcpy (cuwk_rec.gl_acc_no, glmrRec.acc_no);
	}
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	/*------------------------------------------
	| Add transaction for account if required. |
	------------------------------------------*/
	strcpy (glwkRec.tran_type,   (INVOICE) ? " 4" : " 5");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.period_no,    period_no);
	strcpy (glwkRec.acc_no,  	  glmrRec.acc_no);
	strcpy (glwkRec.co_no,        co_no);
	strcpy (glwkRec.est_no,       est_no);
	strcpy (glwkRec.narrative,    cuwk_rec.narrative);
	strcpy (glwkRec.alt_desc1,    " ");
	strcpy (glwkRec.alt_desc2,    " ");
	strcpy (glwkRec.alt_desc3,    " ");
	strcpy (glwkRec.batch_no,     " ");
	sprintf (glwkRec.user_ref, "%-15.15s", inv_no);
	sprintf (glwkRec.acronym,     "%-9.6s", dbt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", inv_no);
	strcpy (glwkRec.stat_flag,    "2");
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	glwkRec.tran_date      = d_inv;

	glwkRec.ci_amt    = no_dec (cuwk_rec.tot_fx);
	glwkRec.o1_amt    = no_dec (cuwk_rec.disc);
	glwkRec.o2_amt    = cuwk_rec.exch_rate;
	glwkRec.exch_rate = cuwk_rec.exch_rate;
	glwkRec.o3_amt    = no_dec 
						(
							cuwk_rec.freight + 
							cuwk_rec.insurance +
							cuwk_rec.other_cost1 + 
							cuwk_rec.other_cost2 + 
							cuwk_rec.other_cost3 + 
							cuwk_rec.sos + 
							cuwk_rec.restock_fee +
							cuwk_rec.exch_var + 
							cuwk_rec.item_levy
						);

	glwkRec.o4_amt    = no_dec (cuwk_rec.gst + cuwk_rec.tax);
	glwkRec.post_date = StringToDate (systemDate);

	glwkRec.amount    	= no_dec (fx_amt);
	glwkRec.loc_amount  = no_dec (loc_amt);
	sprintf (glwkRec.run_no, "      ");
	sprintf (glwkRec.currency, cuwk_rec.currency);

	GL_AddBatch ();
}


/*==================================================
| Post accumulated debits & credits if non-zero  . |
==================================================*/
void	
PostDebit (
 char*              Gltype,
 char*              IfaceCode,
 char*              ClassType,
 double             FxAmt,
 double             LocAmt)
{
	sprintf (glwkRec.jnl_type, "%-1.1s", Gltype);
	GL_GLI 
	(
		cuwk_rec.co_no,		/*	Company Number 	*/
		cuwk_rec.est,		/*	Branch No.		*/
		"  ",				/*	Warehouse No.	*/
		IfaceCode,
		ClassType,
		" "
	);
	WriteGlwk 
	(
		cuwk_rec.co_no,
		cuwk_rec.est,
		"      ",
		"      ",
		glmrRec.acc_no,
		cuwk_rec.period_no,
		cuwk_rec.date_of_inv,
		no_dec (FxAmt),
		no_dec (LocAmt)
	);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_SortRead (
 FILE*              srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset [0] = sptr;

	tptr = sptr;
	while (fld_no < 28)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset [fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}

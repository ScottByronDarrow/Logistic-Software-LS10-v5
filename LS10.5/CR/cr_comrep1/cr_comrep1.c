/*=====================================================================
|  Copyright (C) 1999 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( cr_comrep1.c   )                                 |
|  Program Desc  : ( Supplier Cash Commitments Report By Supplier.   )|
|                  (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, pocr, sumr, suin,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written :  18/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (19/09/90)      | Modified  by :  Scott Darrow.    |
|  Date Modified : (11/07/91)      | Modified  by :  Scott Darrow.    |
|  Date Modified : (03/12/91)      | Modified  by :  Campbell Mander  |
|  Date Modified : (16/07/92)      | Modified  by :  Scott Darrow.    |
|  Date Modified : (03/12/92)      | Modified  by :  Simon Dubey.     |
|  Date Modified : (05/05/93)      | Modified  by :  Jonathan Chen    |
|  Date Modified : (20/05/93)      | Modified  by :  Simon Dubey.     |
|  Date Modified : (30/11/95)      | Modified  by :  Scott B Darrow.  |
|  Date Modified : (10/04/96)      | Modified  by :  Scott B Darrow.  |
|  Date Modified : (10/09/97)      | Modified  by :  Leah Manibog.    |
|  Date Modified : (14/09/1999)    | Modified  by :  Ramon A. Pacheco |
|                                                                     |
|  Comments      : Program print cash commitments by supplier and     |
|                : optionally by document for selected currency.      |
|                : Exchange rate used is that entered except for items|
|                : with fixed rate.                                   |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|                : (11/07/91) - General Update.                       |
|                :                                                    |
|  (03/12/91)    : Add optional ordering of suppliers, either order   |
|                : by number or by acronym.                           |
|  (16/07/92)    : Fixed S/C APP-7386.                                |
|  (03/12/92)    : DFT 8202 Not picking up credits                    |
|  (05/05/93)    : AMB 8737 Added T)ransfer payment method            |
|  (20/05/93)    : EGC 9020 Put Selection method into heading and NOW |
|                : use suin_amt_pay rather than amt_pay - amt_paid    |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|  (10/04/96)    : PDL - Updated to use branchNumber as per normal.   |
|  (10/09/97)    : Updated for Multilingual Conversion and            |
|  (14/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_comrep1.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_comrep1/cr_comrep1.c,v 5.5 2001/10/11 05:42:05 robert Exp $";

#define PAGELINES	65
#define	MCURR		(multiCurrency [0] == 'Y')
#define	BY_NUM		(local_rec.ord_by_value [0] == 'N')
#define	DETAIL		(local_rec.detail_value [0] == 'D')

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pr_format3.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include "schema"

	struct	commRecord	comm_rec;
	struct	sumrRecord	sumr_rec;
	struct	pocrRecord	pocr_rec;
	struct	suinRecord	suin_rec;


/*=============
| Table names |
=============*/
static char
	*data	= "data",
	*sumr2	= "sumr2";

/*=========
| Globals |
==========*/
static int 	printerNumber = 0,
			line_no = 0,
			newSupplier = FALSE,
			envCoOwned = 0,
			envCrFind = 0;

static char 	multiCurrency [2];
	char		branchNumber[3];

static double 	exch_rate   = 0.00,
				cur_bal_loc = 0.00,
				crd_bal_loc = 0.00,
				doc_bal_loc = 0.00,
				cur_balance = 0.00,
				crd_balance = 0.00,
				doc_balance = 0.00;

static FILE	*fin,
			*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	sel_curr [4];
	char 	proc_curr [4];
	char 	override_exch [2];
	char 	detail [9];
	char 	ord_by [8];
	char 	select [9];
	char	pay_mthd [10];
	char 	detail_value [2];
	char 	ord_by_value [2];
	char 	select_value [2];
	char	pay_mthd_value [2];

	double	inv_balance;
	double	new_rate;
	long	due_date;
	char 	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "select",	 4, 24, CHARTYPE,
		"U", "          ",
		" ", "A", "S(elected) / A(ll)    ", "Enter S to include only items selected for payment",
		YES, NO,  JUSTLEFT, "AS", "", local_rec.select_value},
	{1, LIN, "select_desc",	 4, 30, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.select},
	{1, LIN, "detail",	 5, 24, CHARTYPE,
		"U", "          ",
		" ", "S", "D(etail) / S(ummary)   ", "Enter D to list invoice details",
		YES, NO,  JUSTLEFT, "DS", "", local_rec.detail_value},
	{1, LIN, "detail_desc",	 5, 30, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.detail},
	{1, LIN, "ord_by",	 6, 24, CHARTYPE,
		"U", "          ",
		" ", "A", "Order Suppliers By       ", "Order By Supplier A(cronym) / N(umber)",
		YES, NO,  JUSTLEFT, "AN", "", local_rec.ord_by_value},
	{1, LIN, "ord_by_desc",	 6, 30, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.ord_by},
	{1, LIN, "due_date",	 7, 24, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Payment Due Date        ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.due_date},
	{1, LIN, "paymethod",	 8, 24, CHARTYPE,
		"U", "          ",
		" ", "A", "Payment Method          ", "A)ll [default], C)heque, D)raft, T)ransfer",
		YES, NO,  JUSTLEFT, "ACDT", "", local_rec.pay_mthd_value},
	{1, LIN, "paymethod_desc",	 8, 30, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.pay_mthd},
	{1, LIN, "curr",	 9, 24, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code           ", "Enter Currency Code or <retn> defaults to ALL",
		YES, NO,  JUSTLEFT, "", "", local_rec.sel_curr},
	{1, LIN, "curr_desc",	 9, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "ovr_exch",	10, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Override Exch Rate      ", "Enter Y(es) or default to N(o)",
		YES, NO,  JUSTLEFT, "NY", "", local_rec.override_exch},
	{1, LIN, "new_rate",	10, 60, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "1.0000", "Exchange Rate  ", "Note: documents with fixed rate are not overridden",
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.new_rate},

	{0}
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			(void);
void	CloseDB			(void);
int		spec_valid		(int);
void	SrchPocr		(char *);
int		heading			(int);
void	ProcessCurrency	(void);
void	ProcessSumr		(void);
void	ProcessDocument	(void);
void	PrintSuppTotal	(void);
void	PrintCurrTotal	(void);
int		check_page		(void);
void	ShowPayMethod	(FILE *);
void	HeadingOutput	(void);
void	PrintCurrHead	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 2)
	{
		print_at (0,0, ML(mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber  = atoi (argv [1]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	SETUP_SCR (vars);

	envCrFind = atoi(get_env("CR_FIND"));
	envCoOwned = atoi(get_env("CR_CO"));

	/*-----------------------------------------------
	| Setup screen if not multi-currency suppliers. |
	-----------------------------------------------*/
	if (!MCURR)
	{
		FLD ("curr") 	 = ND;
		FLD ("curr_desc") = ND;
		FLD ("ovr_exch")  = ND;
		FLD ("new_rate")  = ND;
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	/*-------------------------------
	 Open db & init comm_rec
	-------------------------------*/
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy(branchNumber,(!envCoOwned) ? " 0" : comm_rec.est_no);

	while (!prog_exit)
	{
		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		if (MCURR)
		{
			FLD ("curr") 	  = YES;
			FLD ("curr_desc") = NA;
			FLD ("ovr_exch")  = YES;
			FLD ("new_rate")  = NO;
		}
		else
		{
			FLD ("curr") 	  = ND;
			FLD ("curr_desc") = ND;
			FLD ("ovr_exch")  = ND;
			FLD ("new_rate")  = ND;
		}

		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		/*------------------------------------
		| Process suppliers & print report.  |
		------------------------------------*/
		clear ();
		dsp_screen (" Processing : Printing Commitments Report",
					comm_rec.co_no, comm_rec.co_name);
		ProcessCurrency ();
	}

	/*=========================
	| Program exit sequence	. |
	=========================*/
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	abc_alias (sumr2, sumr);

	open_rec( sumr ,sumr_list,SUMR_NO_FIELDS,(!envCrFind) ? "sumr_id_no" 
							    					    : "sumr_id_no3");
	open_rec( sumr2 ,sumr_list,SUMR_NO_FIELDS,(!envCrFind) ? "sumr_id_no2" 
							    					    : "sumr_id_no4");
	open_rec (suin,  suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (suin);
	abc_fclose (pocr);
	abc_dbclose (data);
}

/*============================================
| Special validation on screen entry.        |
============================================*/
int
spec_valid (
 int field)
{
	if (LCHECK ("select"))
	{
		if (local_rec.select_value [0] == 'S')
			strcpy (local_rec.select, ML ("Selected"));
		else
		
			strcpy (local_rec.select, ML ("All     "));
		DSP_FLD ("select_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("detail"))
	{
		strcpy (local_rec.detail, DETAIL ? ML("Detailed") : ML("Summary "));
		DSP_FLD ("detail_desc");
		return (EXIT_SUCCESS);
	}

	/*----------------
	| Validate Order |
	----------------*/
	if (LCHECK ("ord_by"))
	{
		strcpy (local_rec.ord_by, BY_NUM ? ML("Number ") : ML("Acronym"));
		DSP_FLD ("ord_by_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("paymethod"))
	{
		char	desc[10];

		switch (local_rec.pay_mthd_value[0])
		{
		case 'A'	:
			strcpy (desc, ML ("All     "));
			break;
		case 'C'	:
			strcpy (desc, ML ("Cheque  "));
			break;
		case 'D'	:
			strcpy (desc, ML ("Draft   "));
			break;
		case 'T'	:
			strcpy (desc, ML ("Transfer"));
			break;
		}
		strcpy (local_rec.pay_mthd, desc);
		DSP_FLD ("paymethod_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Currency Code.       |
	-------------------------------*/
	if (LCHECK ("curr"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (dflt_used && MCURR)
		{
			strcpy (local_rec.sel_curr, "ALL");
			DSP_FLD ("curr");
			FLD ("curr_desc") = NA;
			FLD ("ovr_exch")  = NA;
			FLD ("new_rate")  = NA;
			strcpy (local_rec.override_exch, "N");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
		   SrchPocr (temp_str);
		   return (EXIT_SUCCESS);
		}

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, local_rec.sel_curr);
		cc = find_rec (pocr, (char *) &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML(mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Override.   |
	-------------------------------*/
	if (LCHECK ("ovr_exch"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (local_rec.override_exch [0] == 'N')
		{
			local_rec.new_rate = pocr_rec.ex1_factor;
			FLD ("new_rate") = NA;
			DSP_FLD ("new_rate");
			skip_entry = 1;
		}
		else
			FLD ("new_rate") = NO;

		DSP_FLD ("ovr_exch");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("new_rate"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.new_rate = pocr_rec.ex1_factor;

		DSP_FLD ("new_rate");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================
| Search for currency pocr code |
===============================*/
void
SrchPocr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Cd.", "#Currency Code Description");
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code , "%-3.3s", key_val);
	cc = find_rec (pocr, (char *) &pocr_rec, GTEQ, "r");
	while (!cc && !strcmp (pocr_rec.co_no, comm_rec.co_no) &&
		      	  !strncmp (pocr_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
		        break;
		cc = find_rec (pocr, (char *) &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, (char *) &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pocr", "DBFIND");
}

/*===============================================
| Screen Heading Display Routine.               |
===============================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		fflush (stdout);
		rv_pr ( ML(mlCrMess119), 25, 0, 1);

		fflush (stdout);
		move (0, 1);
		line (80);

		move (1, input_row);
		box (0, 3, 80, MCURR ? 7 : 5);

		move (1, 20);
		line (78);

		strcpy(err_str, ML(mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no, comm_rec.co_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*================================
| Process Currencies.            |
================================*/
void
ProcessCurrency (
 void)
{
	int no_curr = 0;

	/*----------------------------------------
	| Print heading for first currency.      |
	----------------------------------------*/
	HeadingOutput ();

	/*----------------------------------------
	| Process Currencies (All or Selected).  |
	----------------------------------------*/
	if (MCURR && !strcmp (local_rec.sel_curr, "ALL"))
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code , "   ");
		cc = find_rec (pocr, (char *) &pocr_rec, GTEQ, "r");
		while (!cc && !strcmp (pocr_rec.co_no, comm_rec.co_no))
		{
			if (no_curr > 0)
				fprintf (fout, ".PA\n");
			no_curr ++;
			strcpy (local_rec.proc_curr, pocr_rec.code);
			if (MCURR)
				PrintCurrHead ();

			ProcessSumr ();

			cc = find_rec (pocr, (char *) &pocr_rec, NEXT, "r");

		}
	}
	else
	{
		strcpy (local_rec.proc_curr, local_rec.sel_curr);
		if (MCURR)
			PrintCurrHead ();
		ProcessSumr ();
	}

	pr_format (fin, fout, "END_FILE", 0, 0);
	pclose (fout);
}


/*================================
| Process Suppliers .            |
================================*/
void
ProcessSumr (
 void)
{
	char	*read_file = BY_NUM ? "sumr" : "sumr2";

	cur_balance = 0.00;
	cur_bal_loc = 0.00;

	/*----------------------------------------
	| Process All Suppliers.                 |
	----------------------------------------*/
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, "      ");
	strcpy (sumr_rec.acronym, "         ");

	cc = find_rec (read_file, (char *) &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (sumr_rec.est_no, branchNumber))
	{
		if ( (MCURR &&
				strcmp (sumr_rec.curr_code, local_rec.proc_curr)) ||
				(local_rec.select [0] == 'S' &&
					sumr_rec.stat_flag [0] != 'S') ||
				(*local_rec.pay_mthd != 'A' &&
					*local_rec.pay_mthd != *sumr_rec.pay_method))
		{
			cc = find_rec (read_file, (char *) &sumr_rec, NEXT, "r");
			continue;
		}

		dsp_process ( "Supplier", sumr_rec.crd_no );
		newSupplier = TRUE;
		crd_balance = 0.00;
		crd_bal_loc = 0.00;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no, "               ");
		cc = find_rec (suin, (char *) &suin_rec, GTEQ, "r");

		while (!cc &&  sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			if (local_rec.select [0] == 'S' && suin_rec.stat_flag [0] != 'S')
			{
				cc = find_rec (suin, (char *) &suin_rec, NEXT, "r");
				continue;
			}
			else if (suin_rec.pay_date <= local_rec.due_date &&
	 	                 (suin_rec.amt - suin_rec.amt_paid))
			{
				ProcessDocument ();
				crd_balance += doc_balance;
				crd_bal_loc += doc_bal_loc;
			}
			cc = find_rec (suin, (char *) &suin_rec, NEXT, "r");
		}

		/*----------------------------------------
		| Print supplier totals.                 |
		----------------------------------------*/
		if (crd_balance != 0.00)
		{
			cur_balance += crd_balance;
			cur_bal_loc += crd_bal_loc;
			PrintSuppTotal ();
		}

		cc = find_rec (read_file, (char *) &sumr_rec, NEXT, "r");
	}

	/*----------------------------------------
	| Print Currency Totals.                 |
	----------------------------------------*/
	PrintCurrTotal ();
}

/*=====================================
| Process Invoice / Cn Document.      |
=====================================*/
void
ProcessDocument (
 void)
{
	char	crd_name [31];

	doc_bal_loc = 0.00;
	doc_balance = suin_rec.pay_amt;

	if ( doc_balance == 0.00)
		return;

	if (MCURR)
	{
		if (suin_rec.er_fixed [0] != 'Y' && local_rec.override_exch [0] == 'Y')
			exch_rate = local_rec.new_rate;
		else
			exch_rate = suin_rec.exch_rate;

		if (exch_rate == 0.00)
			exch_rate = 1.00;

		doc_bal_loc = doc_balance / exch_rate;
	}

	if (!DETAIL)
		return;

	/*--------------------
	| Print Inv/Cn Detail|
	--------------------*/
	if (MCURR)
	{
		if (newSupplier == TRUE)
		{
			pr_format (fin, fout, "DMC_DET", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "DMC_DET", 2, crd_name);
			newSupplier = FALSE;
		}
		else
		{
			pr_format (fin, fout, "DMC_DET", 1, "      ");
			pr_format (fin, fout, "DMC_DET", 2, "                              ");
		}
		pr_format (fin, fout, "DMC_DET", 3, suin_rec.inv_no);
		pr_format (fin, fout, "DMC_DET", 4, suin_rec.date_of_inv);
		pr_format (fin, fout, "DMC_DET", 5, suin_rec.pay_date);
		pr_format (fin, fout, "DMC_DET", 6, doc_balance);
		pr_format (fin, fout, "DMC_DET", 7, exch_rate);
	 	pr_format (fin, fout, "DMC_DET", 8, doc_bal_loc);
	}
	else
	{
		if (newSupplier == TRUE)
		{
			pr_format (fin, fout, "DC_DET", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "DC_DET", 2, crd_name);
			newSupplier = FALSE;
		}
		else
		{
			pr_format (fin, fout, "DC_DET", 1, "      ");
			pr_format (fin, fout, "DC_DET", 2, "                              ");
		}
		pr_format (fin, fout, "DC_DET", 3, suin_rec.inv_no);
		pr_format (fin, fout, "DC_DET", 4, suin_rec.date_of_inv);
		pr_format (fin, fout, "DC_DET", 5, suin_rec.pay_date);
		pr_format (fin, fout, "DC_DET", 6, doc_balance);
	}
}

/*=====================================
| Print Supplier Totals.              |
=====================================*/
void
PrintSuppTotal (
 void)
{
	char	crd_name [31];

	/*----------------------------------------
	| Print Detailed Supplier Total Line.    |
	----------------------------------------*/
	if (DETAIL)
	{
		if (MCURR)
		{
			pr_format (fin, fout, "DMC_CDT", 1, crd_balance);
			pr_format (fin, fout, "DMC_CDT", 2, crd_bal_loc);
			pr_format (fin, fout, "DMC_LN2", 0, 0);
		}
		else
		{
			pr_format (fin, fout, "DC_CDT", 1, crd_balance);
			pr_format (fin, fout, "DC_LN2", 0, 0);
		}
	}
	else
	{
		/*------------------------------------------
		| Print Summarised Supplier Total Line.    |
		------------------------------------------*/
		if (MCURR)
		{
			pr_format (fin, fout, "SMC_CDT", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "SMC_CDT", 2, crd_name);
			pr_format (fin, fout, "SMC_CDT", 3, crd_balance);
			pr_format (fin, fout, "SMC_CDT", 4, crd_bal_loc);
		}
		else
		{
			pr_format (fin, fout, "SC_CDT", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "SC_CDT", 2, crd_name);
			pr_format (fin, fout, "SC_CDT", 3, crd_balance);
		}
	}
}

/*=====================================
| Print Currency Totals.              |
=====================================*/
void
PrintCurrTotal (
 void)
{
	/*----------------------------------------
	| Print Detailed Currency Total Line.    |
	----------------------------------------*/
	if (DETAIL)
	{
		if (MCURR)
		{
			pr_format (fin, fout, "DMC_CRT", 1, cur_balance);
			pr_format (fin, fout, "DMC_CRT", 2, cur_bal_loc);
		}
		else
		{
			pr_format (fin, fout, "DC_CRT", 1, cur_balance);
		}
	}
	else
	{
		/*----------------------------------------
		| Print Summarised Currency Total Line.  |
		----------------------------------------*/
		if (MCURR)
		{
			pr_format (fin, fout, "SMC_LN2", 0, 0);
			pr_format (fin, fout, "SMC_CRT", 1, cur_balance);
			pr_format (fin, fout, "SMC_CRT", 2, cur_bal_loc);
		}
		else
		{
			pr_format (fin, fout, "SC_LN2", 0, 0);
			pr_format (fin, fout, "SC_CRT", 1, cur_balance);
		}
	}
}

/*========================
| Page Count Check.      |
========================*/
int
check_page (
 void)
{
	if (line_no > PAGELINES)
		line_no = 0;
	line_no++;
	return (EXIT_SUCCESS);
}

void
ShowPayMethod (
 FILE *	pOut)
{
	char	*desc;

	switch (*local_rec.pay_mthd)
	{
	case 'A'	:
		desc = "All";
		break;
	case 'C'	:
		desc = "Cheque";
		break;
	case 'T'	:
		desc = "Transfer";
		break;
	case 'D'	:
		desc = "Draft";
		break;
	default	:
		desc = "**Internal Error**";
	}
	fprintf (pOut, ".EPayment Method : %s\n", desc);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/
	if (!(fin = pr_open ("cr_comrep1.p")))
		sys_err ("Error in opening cr_comrep1.p during (FOPEN)", errno, PNAME);

	if (!(fout = popen ("pformat", "w")))
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".13\n");
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".L136\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	if (!envCoOwned)
		pr_format (fin, fout, "BLANK", 0, 0);
	else
		fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	
	if (MCURR && DETAIL)
	{
		pr_format (fin, fout, "HEAD1A", 0, 0);
		fprintf   (fout, ".EFOR %s DOCUMENTS DUE BY %s\n",
						clip(local_rec.select),
						DateToString(local_rec.due_date));
		pr_format (fin, fout, "HEAD1D", 1, BY_NUM ? "NUMBER" : "ACRONYM");
		ShowPayMethod (fout);
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "DMC_LN1", 0, 0);
		pr_format (fin, fout, "DMC_HD2", 0, 0);
		pr_format (fin, fout, "DMC_HD3", 0, 0);
		pr_format (fin, fout, "DMC_LN1", 0, 0);
		pr_format (fin, fout, "DMC_RUL", 0, 0);
	}
	else if (MCURR && !DETAIL)
	{
		pr_format (fin, fout, "HEAD1B", 0, 0);
		fprintf   (fout, ".EFOR %s DOCUMENTS DUE BY %s\n",
						clip(local_rec.select),
						DateToString(local_rec.due_date));
		pr_format (fin, fout, "HEAD1D", 1, BY_NUM ? "NUMBER" : "ACRONYM");
		ShowPayMethod (fout);
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "SMC_LN1", 0, 0);
		pr_format (fin, fout, "SMC_HD2", 0, 0);
		pr_format (fin, fout, "SMC_HD3", 0, 0);
		pr_format (fin, fout, "SMC_LN1", 0, 0);
		pr_format (fin, fout, "SMC_RUL", 0, 0);
	}
	else if (!MCURR && DETAIL)
	{
		pr_format (fin, fout, "HEAD1A", 0, 0);
		fprintf   (fout, ".EFOR %s DOCUMENTS DUE BY %s\n",
						clip(local_rec.select),
						DateToString(local_rec.due_date));
		pr_format (fin, fout, "HEAD1D", 1, BY_NUM ? "NUMBER" : "ACRONYM");
		ShowPayMethod (fout);
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "DC_LN1", 0, 0);
		pr_format (fin, fout, "DC_HD2", 0, 0);
		pr_format (fin, fout, "DC_HD3", 0, 0);
		pr_format (fin, fout, "DC_LN1", 0, 0);
		pr_format (fin, fout, "DC_RUL", 0, 0);
	}
	else if (!MCURR && !DETAIL)
	{
		pr_format (fin, fout, "HEAD1B", 0, 0);
		fprintf   (fout, ".EFOR %s DOCUMENTS DUE BY %s\n",
						clip(local_rec.select),
						DateToString(local_rec.due_date));
		pr_format (fin, fout, "HEAD1D", 1, BY_NUM ? "NUMBER" : "ACRONYM");
		ShowPayMethod (fout);
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "SC_LN1", 0, 0);
		pr_format (fin, fout, "SC_HD2", 0, 0);
		pr_format (fin, fout, "SC_HD3", 0, 0);
		pr_format (fin, fout, "SC_LN1", 0, 0);
		pr_format (fin, fout, "SC_RUL", 0, 0);
	}
	line_no = 13;
}
/*===================================
| Print Currency Heading.           |
===================================*/
void
PrintCurrHead (
 void)
{
	if (MCURR)
	{
	 	pr_format 
		(	
			fin,
			fout,
			DETAIL ? "DMC_CUR" : "SMC_CUR",
			1,
			local_rec.proc_curr
		);
	}
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: uainvrep.c,v 5.4 2001/12/10 00:46:32 scott Exp $
|  Program Name  : (cr_uainvrep.c) 
|  Program Desc  : (Supplier Unapproved Invoice Report By Supplier.)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 07/06/89         |
|---------------------------------------------------------------------|
| $Log: uainvrep.c,v $
| Revision 5.4  2001/12/10 00:46:32  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/07 07:42:55  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: uainvrep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_uainvrep/uainvrep.c,v 5.4 2001/12/10 00:46:32 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#define PAGELINES	65
#define	MCURR		 (multiCurrency [0] == 'Y')

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include    "schema"

struct commRecord   comm_rec;
struct sumrRecord   sumr_rec;
struct pocrRecord   pocr_rec;
struct suinRecord   suin_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int 	printerNumber = 0,
			newSupplier = 0,
			envCrCo = 0;

	char 	multiCurrency [2],
	    	branchNumber [3];

	char	*sptr;

   	double 	exchangeRate 		 = 0.00,
			totalBalanceLocal 	 = 0.00,
			supplierBalanceLocal = 0.00,
			documentBalanceLocal = 0.00,
			supplierBalance 	 = 0.00,
			documentBalance 	 = 0.00;

	FILE	*fin;
	FILE	*fsort;
	FILE	*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	sel_curr [4];
	char 	proc_curr [4];
	char 	override_exch [2];
	char 	detail [2];
	char 	select [2];
	double	inv_balance;
	double	new_rate;
	long	inv_date;
	char 	systemDate [11];
	char 	com_date [11];
} local_rec;

static	struct	var	vars []	={	
	{1, LIN, "inv_date", 4, 30, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", local_rec.systemDate, "Invoice Date (on or before)", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.inv_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 	 (void);
void	OpenDB			 	 (void);
void	CloseDB		 	 	 (void);
int		spec_valid		 	 (int);
int		heading			 	 (int);
int		ProcessSumr		 	 (void);
void	ProcessDocument	 	 (void);
void	PrintSupplierTotal 	 (int, double, double);
void	PrintTotal		 	 (void);
int		check_page		 	 (void);
void	HeadingOutput	 	 (void);

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
		print_at (0, 0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber  = atoi (argv [1]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	envCrCo = atoi (get_env ("CR_CO"));
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");
	strcpy (local_rec.com_date, DateToString (comm_rec.crd_date));

	while (prog_exit == 0) 
	{
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
		dsp_screen ("Printing Unapproved Invoices Report",
					comm_rec.co_no, comm_rec.co_name);
		ProcessSumr ();
	}	
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no3");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (pocr);
	abc_dbclose ("data");
}

/*============================================
| Special validation on screen entry.        |
============================================*/
int
spec_valid (
 int field)
{
	/*-------------------------------
	| Validate Invoice Date.        |	
	-------------------------------*/
	if (LCHECK ("inv_date"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
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
		rv_pr (ML (mlCrMess031), 25, 0, 1);

		fflush (stdout);
		line_at (1,0,80);
		line_at (20,1,78);

		box (0,3,80,1);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*================================
| Process Suppliers .            |
================================*/
int
ProcessSumr (void)
{
	int	single_doco = 0;

	totalBalanceLocal = 0.00;

	/*----------------------------------------
	| Print heading                          |
	----------------------------------------*/
	HeadingOutput ();

	/*----------------------------------------
	| Process All Suppliers.                 |
	----------------------------------------*/
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.crd_no, "      ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))  
	{                        
		dsp_process ("Supplier", sumr_rec.crd_no);
		newSupplier = 1;
		single_doco = 0;
		supplierBalance = 0.00;
		supplierBalanceLocal = 0.00;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no, "               ");
		cc = find_rec (suin, &suin_rec, GTEQ, "r");

		while (!cc &&  sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			if (suin_rec.date_of_inv <= local_rec.inv_date &&
				 suin_rec.approved [0] != 'Y')
			{
			    if (suin_rec.amt - suin_rec.amt_paid != 0.00)
			    {
					ProcessDocument ();
					supplierBalance += documentBalance;
					supplierBalanceLocal += documentBalanceLocal;
					single_doco++;
			    }
			}
			cc = find_rec (suin, &suin_rec, NEXT, "r");
		}

		/*----------------------------------------
		| Print supplier totals.                 |
		----------------------------------------*/
		if (single_doco)
		{
			totalBalanceLocal += supplierBalanceLocal;
			if (single_doco > 1)
				PrintSupplierTotal (TRUE, supplierBalance, 
									supplierBalanceLocal);
			else
				PrintSupplierTotal (FALSE, supplierBalance, 
									supplierBalanceLocal);
		}

		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}

	/*----------------------------------------
	| Print Supplier Totals.                 |
	----------------------------------------*/
	PrintTotal ();
	pr_format (fin, fout, "END_FILE", 0, 0);
	pclose (fout);
	return (EXIT_SUCCESS);
}

/*=====================================
| Process Invoice / Cn Document.      |
=====================================*/
void
ProcessDocument (void)
{
	char	crd_name [31];

	documentBalance = suin_rec.amt - suin_rec.amt_paid;

	if (MCURR)
	{
		if (suin_rec.er_fixed [0] != 'Y' &&
		     local_rec.override_exch [0] == 'Y')
			exchangeRate = local_rec.new_rate;
		else
			exchangeRate = suin_rec.exch_rate;

		if (exchangeRate == 0.00)
			exchangeRate = 1.00;

		documentBalanceLocal = documentBalance / exchangeRate;
	}

	/*--------------------
	| Print Inv/Cn Detail|
	--------------------*/
	if (MCURR)
	{
		if (newSupplier)
		{
			pr_format (fin, fout, "DMC_DET", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "DMC_DET", 2, crd_name);
			pr_format (fin, fout, "DMC_DET", 3, sumr_rec.curr_code);
			newSupplier = 0;
		}
		else
		{
			pr_format (fin, fout, "DMC_DET", 1, "      ");
			pr_format (fin, fout, "DMC_DET", 2, 
					   "                              ");
			pr_format (fin, fout, "DMC_DET", 3, "   ");
		}
		pr_format (fin, fout, "DMC_DET", 4, suin_rec.destin);
		pr_format (fin, fout, "DMC_DET", 5, suin_rec.hold_reason);
		pr_format (fin, fout, "DMC_DET", 6, suin_rec.inv_no);
		pr_format (fin, fout, "DMC_DET", 7,DateToString (suin_rec.date_of_inv));
		pr_format (fin, fout, "DMC_DET", 8, DateToString (suin_rec.pay_date));
		pr_format (fin, fout, "DMC_DET", 9, documentBalance);
		pr_format (fin, fout, "DMC_DET", 10,exchangeRate);
	 	pr_format (fin, fout, "DMC_DET", 11,documentBalanceLocal);
	}
	else      
	{
		if (newSupplier)
		{
			pr_format (fin, fout, "DC_DET", 1, sumr_rec.crd_no);
			sprintf (crd_name, "%-30.30s", sumr_rec.crd_name);
			pr_format (fin, fout, "DC_DET", 2, crd_name);
			newSupplier = 0;
		}
		else
		{
			pr_format (fin, fout, "DC_DET", 1, "      ");
			pr_format (fin, fout, "DC_DET", 2, 
					   "                              ");
		}
		pr_format (fin, fout, "DC_DET", 3, suin_rec.destin);
		pr_format (fin, fout, "DC_DET", 4, suin_rec.hold_reason);
		pr_format (fin, fout, "DC_DET", 5, suin_rec.inv_no);
		pr_format (fin, fout, "DC_DET", 6, DateToString (suin_rec.date_of_inv));
		pr_format (fin, fout, "DC_DET", 7, DateToString (suin_rec.pay_date));
		pr_format (fin, fout, "DC_DET", 8, documentBalance);
	}
	return;
}

/*=====================================
| Print Supplier Totals.              |
=====================================*/
void
PrintSupplierTotal (
 int	crd_tot,
 double	os_tot,
 double	loc_tot)
{
	/*----------------------------------------
	| Print Detailed Supplier Total Line.    |
	----------------------------------------*/
	if (crd_tot)
	{
		if (MCURR)
		{
			pr_format (fin, fout, "DMC_CDT", 1, os_tot);
			pr_format (fin, fout, "DMC_CDT", 2, loc_tot);
		}
		else
			pr_format (fin, fout, "DC_CDT", 1, loc_tot);
	}

	if (MCURR)
		pr_format (fin, fout, "DMC_LN2", 0, 0);
	else        
		pr_format (fin, fout, "DC_LN2", 0, 0);
}

/*=====================================
| Print Currency Totals.              |
=====================================*/
void
PrintTotal (void)
{
	/*-------------------------------
	| Print Detailed Total Line.    |
	-------------------------------*/
	if (MCURR)
		pr_format (fin, fout, "DMC_CRT", 1, totalBalanceLocal);
	else        
		pr_format (fin, fout, "DC_CRT", 1, totalBalanceLocal);
}

/*========================
| Page Count Check.      |
========================*/
int
check_page (void)
{
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	/*------------------
	| Open format file |
	------------------*/
	if ((fin = pr_open ("cr_uainvrep.p")) == NULL)
		sys_err ("Error in opening cr_uainvrep.p during (FOPEN)", errno, PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	pr_format (fin, fout, "BLANK", 0, 0);
	if (MCURR)
	{
		pr_format (fin, fout, "HEAD1A", 0, 0);
		pr_format (fin, fout, "HEAD1C", 1, DateToString (local_rec.inv_date));
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "DMC_LN1", 0, 0);
		pr_format (fin, fout, "DMC_HD2", 0, 0);
		pr_format (fin, fout, "DMC_HD3", 0, 0);
		pr_format (fin, fout, "DMC_LN1", 0, 0);
		pr_format (fin, fout, "DMC_RUL", 0, 0);
	}
	else 
	{
		pr_format (fin, fout, "HEAD1A", 0, 0);
		pr_format (fin, fout, "HEAD1C", 1, local_rec.inv_date);
		pr_format (fin, fout, "BLANK", 0, 0);
		pr_format (fin, fout, "DC_LN1", 0, 0);
		pr_format (fin, fout, "DC_HD2", 0, 0);
		pr_format (fin, fout, "DC_HD3", 0, 0);
		pr_format (fin, fout, "DC_LN1", 0, 0);
		pr_format (fin, fout, "DC_RUL", 0, 0);
	}
	fprintf (fout, ".PI12\n");
	fflush (fout);
}

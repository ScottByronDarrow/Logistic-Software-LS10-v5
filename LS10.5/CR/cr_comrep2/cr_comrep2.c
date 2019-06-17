/*=====================================================================
|  Copyright (C) 1999 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (cr_comrep2.c  )                                   |
|  Program Desc  : (Supplier Cash Commitments Report By Currency.  )  |
|                  (                                               )  |
|---------------------------------------------------------------------|
|  Access files  :  comm, pocr, sumr, suin,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 06/06/89         |
|---------------------------------------------------------------------|
|  Date Modified : (19/09/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (11/07/91)      | Modified  by :                   |
|  Date Modified : (30/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Roanna Marcelino. |
|  Date Modified : (14/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : Program prints cash commitments by currency.       |
|                : Exchange rate used is that entered except for items|
|                : with fixed rate.                                   |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|                : (11/07/91) - Updated to clean up.                  |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|                :                                                    |
|  (14/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_comrep2.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_comrep2/cr_comrep2.c,v 5.2 2001/08/09 08:51:47 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#define PAGELINES	65
#define	MCURR		 (multiCurrency[0] == 'Y')
#define	MOD  		1	/* alters frequency for dsp_process */

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include "schema"

	struct	commRecord	comm_rec;
	struct	sumrRecord	sumr_rec;
	struct	pocrRecord	pocr_rec;
	struct	suinRecord	suin_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	int 	printerNumber = 0,
			lineNumberCounter = 0;

	char 	multiCurrency[2],
	    	select_type[2];

	char	*sptr;

   	double 	exchRate 		= 0.00,		/* Exchange Rate	      */
			total_der 		= 0.00,		/* Tot Bal (using documnt er) */
			total_cer 		= 0.00,		/* Tot Bal (using current er) */
			cur_fbal_orig 	= 0.00,		/* Cur Bal (orig) (fixed er)  */
			cur_fbal_fer 	= 0.00,		/* Cur Bal (using fixed er)   */
			cur_vbal_orig 	= 0.00,		/* Cur Bal (orig) (varbl er)  */
			cur_vbal_der 	= 0.00,		/* Cur Bal (using documnt er) */
			cur_vbal_cer 	= 0.00,		/* Cur Bal (using current er) */
			doc_bal_orig 	= 0.00,		/* Doc Bal (origin value)     */
			doc_bal_der 	= 0.00,		/* Doc Bal (using documnt er) */
			doc_bal_cer 	= 0.00;		/* Doc Bal (using current er) */

	FILE	*fin;
	FILE	*fout;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	sel_curr[4];
	char 	proc_curr[4];
	char 	cur_desc[41];
	char 	override_exch[2];
	char 	detail[2];
	char 	select[2];
	double	inv_balance;
	double	cur_rate;
	long	due_date;
	char 	systemDate[11];
	char 	com_date[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "due_date", 4, 22, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", local_rec.systemDate, "Payment Due Date", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.due_date}, 
	{1, LIN, "curr", 5, 22, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Currency Code", "Enter Currency Code or <retn> defaults to ALL", 
		NE, NO, JUSTLEFT, "", "", local_rec.sel_curr}, 
	{1, LIN, "curr_desc", 5, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", pocr_rec.description}, 
	{1, LIN, "ovr_exch", 6, 22, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Override Exch Rate", "Enter Y (es) or default to N (o)", 
		YES, NO, JUSTLEFT, "NY", "", local_rec.override_exch}, 
	{1, LIN, "new_rate", 6, 60, DOUBLETYPE, 
		"NNNN.NNNN", "          ", 
		" ", "1.0000", "Exchange Rate", "Note: documents with fixed rate are not overridden", 
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.cur_rate}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
void	SrchPocr		 (char *);
int		heading			 (int);
int		ProcessCurrency	 (void);
int		ProcessSumr		 (void);
void	PrintCurrTotal	 (void);
void	PrintTotal		 (void);
int		check_page		 (void);
void	HeadOutput		 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	init_scr ();

	if (argc != 2)
	{
		clear ();
		print_at (0,0,mlStdMess036, argv[0]);
		return (EXIT_FAILURE);
	}
	printerNumber  = atoi (argv[1]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	/*-----------------------------------------------
	| Setup screen if not multi-currency suppliers. |
	-----------------------------------------------*/
	if (!MCURR)
	{
		no_option ("CR_MCURR (Multi/Single Currency Suppliers)");
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	set_tty ();
	set_masks ();
	init_vars (1);
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();

	strcpy (local_rec.com_date, DateToString (comm_rec.crd_date));

	while (prog_exit == 0) 
	{
		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		FLD ("curr")		= NE;
		FLD ("curr_desc")	= NA;
		FLD ("ovr_exch")	= YES;
		FLD ("new_rate")	= NO;

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
		dsp_screen (" Processing : Printing Commitments Report",comm_rec.co_no,comm_rec.co_name);
		ProcessCurrency ();
	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("sumr", sumr_list, SUMR_NO_FIELDS, "sumr_id_no3");
	open_rec ("suin", suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec ("pocr", pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose ("sumr");
	abc_fclose ("suin");
	abc_fclose ("pocr");
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
	| Validate Payment Due Date.    |	
	-------------------------------*/
	if (LCHECK ("due_date"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);
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
		else
		{
			FLD ("ovr_exch") = YES;
			FLD ("new_rate") = NO;
		}
		if (SRCH_KEY)
		{
		   SrchPocr (temp_str);
		   return (EXIT_SUCCESS);
		}
		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code, local_rec.sel_curr);
		cc = find_rec ("pocr", &pocr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
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

		if (local_rec.override_exch[0] == 'N')
		{
			local_rec.cur_rate = pocr_rec.ex1_factor;
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
			local_rec.cur_rate = pocr_rec.ex1_factor;

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
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code ,"%-3.3s",key_val);
	cc = find_rec ("pocr", &pocr_rec, GTEQ, "r");
	while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no) && 
		      !strncmp (pocr_rec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocr_rec.code, pocr_rec.description);                       
		if (cc)
		   break;
		cc = find_rec ("pocr",&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	   return;
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",temp_str);
	cc = find_rec ("pocr", &pocr_rec, COMPARISON, "r");
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
		rv_pr (ML (mlCrMess028),25,0,1);

		fflush (stdout);
		move (0,1);
		line (80);

		move (1,input_row);
		box (0,3,80,3);

		move (1,20);
		line (78);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*================================
| Process Currencies.            |
================================*/
int
ProcessCurrency (
 void)
{
	/*----------------------------------------
	| Print heading for first currency.      |
	----------------------------------------*/
	HeadOutput ();

	total_der = 0.00;
	total_cer = 0.00;

	/*----------------------------------------
	| Process Currencies (All or Selected).  |
	----------------------------------------*/
	if (MCURR && strcmp (local_rec.sel_curr,"ALL") == 0)
	{
		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code ,"   ");
		cc = find_rec ("pocr", &pocr_rec, GTEQ, "r");
		while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no))
		{                        
			strcpy (local_rec.proc_curr, pocr_rec.code);
			strcpy (local_rec.cur_desc, pocr_rec.description);
			local_rec.cur_rate = pocr_rec.ex1_factor;

			ProcessSumr ();

			cc = find_rec ("pocr", &pocr_rec, NEXT, "r");
		}
	}
	else
	{
		strcpy (local_rec.proc_curr, local_rec.sel_curr);
		ProcessSumr ();
	}
	PrintTotal ();
	pr_format (fin,fout,"END_FILE",0,0);
	pclose (fout);
	return (EXIT_SUCCESS);
}

/*================================
| Process Suppliers .            |
================================*/
int
ProcessSumr (
 void)
{
   	cur_fbal_orig	= 0.00;		/* Cur Bal (orig) (fixed er)  */
   	cur_fbal_fer	= 0.00;		/* Cur Bal (using fixed er)   */
   	cur_vbal_orig	= 0.00;		/* Cur Bal (orig) (varbl er)  */
   	cur_vbal_der	= 0.00;		/* Cur Bal (using documnt er) */
   	cur_vbal_cer	= 0.00;		/* Cur Bal (using current er) */

	/*----------------------------------------
	| Process All Suppliers.                 |
	----------------------------------------*/
	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.crd_no, "      ");
	cc = find_rec ("sumr", &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no))  
	{                        
		if (MCURR && strcmp (sumr_rec.curr_code,local_rec.proc_curr))
		{
			cc = find_rec ("sumr", &sumr_rec, NEXT, "r");
			continue;
		}

		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no,"               ");
		cc = find_rec ("suin", &suin_rec, GTEQ, "r");

		while (!cc &&  sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			if (suin_rec.pay_date <= local_rec.due_date &&
						(suin_rec.amt - suin_rec.amt_paid))
			{
				if (local_rec.override_exch [0] == 'Y')
					exchRate	=	local_rec.cur_rate;
				else
					exchRate	=	suin_rec.exch_rate;
					
				if (exchRate == 0.00)
					exchRate	=	1.00;

				doc_bal_orig 	= suin_rec.pay_amt;
				doc_bal_cer 	= doc_bal_orig / exchRate;

				if (suin_rec.exch_rate == 0.00)
					doc_bal_der = doc_bal_orig;
				else
					doc_bal_der = doc_bal_orig / suin_rec.exch_rate;

				if (suin_rec.er_fixed[0] == 'Y')
				{
					cur_fbal_orig 	+= doc_bal_orig;
					cur_fbal_fer 	+= doc_bal_der;
				}
				else
				{
					cur_vbal_orig 	+= doc_bal_orig;
					cur_vbal_der 	+= doc_bal_der;
					cur_vbal_cer 	+= doc_bal_cer;
				}
			}
			cc = find_rec ("suin", &suin_rec, NEXT, "r");
		}

		cc = find_rec ("sumr", &sumr_rec, NEXT, "r");
	}

	/*----------------------------------------
	| Print Currency Totals.                 |
	----------------------------------------*/
	PrintCurrTotal ();
	total_der += cur_fbal_fer + cur_vbal_der;
	total_cer += cur_fbal_fer + cur_vbal_cer;

	return (EXIT_SUCCESS);
}

/*=====================================
| Print Currency Totals.              |
=====================================*/
void
PrintCurrTotal (
 void)
{
	double	variance = 0.00;

	/*----------------------------------------
	| Print Summarised Currency Total Lines. |
	----------------------------------------*/
 	pr_format (fin,fout,"SMC_CT1",1,local_rec.proc_curr);
 	pr_format (fin,fout,"SMC_CT1",2,local_rec.cur_desc);
	pr_format (fin,fout,"SMC_CT1",3,"Fixed   ");
	pr_format (fin,fout,"SMC_CT1",4,cur_fbal_orig);
	pr_format (fin,fout,"SMC_CT1",5,cur_fbal_fer);
	pr_format (fin,fout,"SMC_CT1",6,local_rec.cur_rate);
	pr_format (fin,fout,"SMC_CT1",7,cur_fbal_fer);
	variance = 0.00;
	pr_format (fin,fout,"SMC_CT1",8,variance);

	pr_format (fin,fout,"SMC_CT2",1,"Variable");
	pr_format (fin,fout,"SMC_CT2",2,cur_vbal_orig);
	pr_format (fin,fout,"SMC_CT2",3,cur_vbal_der);
	pr_format (fin,fout,"SMC_CT2",4,cur_vbal_cer);
	variance = cur_vbal_der - cur_vbal_cer;
	pr_format (fin,fout,"SMC_CT2",5,variance);

	pr_format (fin,fout,"SMC_LN2",0,0);
}

/*=====================================
| Print Report Totals.                |
=====================================*/
void
PrintTotal (
 void)
{
	double total_var = 0.00;

 	pr_format (fin,fout,"SMC_TOT",1,total_der);
 	pr_format (fin,fout,"SMC_TOT",2,total_cer);
	total_var = total_der - total_cer;
 	pr_format (fin,fout,"SMC_TOT",3,total_var);
}

/*========================
| Page Count Check.      |
========================*/
int
check_page (
 void)
{
	if (lineNumberCounter > PAGELINES)
		lineNumberCounter = 0;
	lineNumberCounter++;
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/
	if ((fin = pr_open ("cr_comrep2.p")) == NULL)
		sys_err ("Error in opening cr_comrep2.p during (FOPEN)",errno,PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI11\n");
	fprintf (fout, ".L135\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	pr_format (fin,fout,"BLANK",0,0);
	pr_format (fin,fout,"HEAD1B",0,0);
	pr_format (fin,fout,"HEAD1C",1,DateToString (local_rec.due_date));
	pr_format (fin,fout,"BLANK",0,0);
	pr_format (fin,fout,"SMC_LN1",0,0);
	pr_format (fin,fout,"SMC_HD2",0,0);
	pr_format (fin,fout,"SMC_HD3",0,0);
	pr_format (fin,fout,"SMC_LN1",0,0);
	pr_format (fin,fout,"SMC_RUL",0,0);
	lineNumberCounter = 12;
	fflush (fout);
}

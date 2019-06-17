/*=====================================================================
|  Copyright (C) 1999 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_invpay.c,v 5.4 2001/12/06 23:49:32 scott Exp $
|  Program Name  : (cr_invpay.c ) 
|  Program Desc  : (Supplier Invoice Payment Amendment program.) 
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 05/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_invpay.c,v $
| Revision 5.4  2001/12/06 23:49:32  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/06 09:23:00  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_invpay.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_invpay/cr_invpay.c,v 5.4 2001/12/06 23:49:32 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#define	INVOICE		 (suin_rec.type [0] == '1')
#define	CREDIT		 (suin_rec.type [0] == '2')
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <twodec.h>
#include <FindSumr.h>

#define	multiCurrency		 (multiCurrencyString [0] == 'Y')

#include    "schema"

struct commRecord   comm_rec;
struct sumrRecord   sumr_rec;
struct suinRecord   suin_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int		envCrCo			= FALSE,
   			envCrFind		= FALSE,
			adjustPayment 	= TRUE;

	char    branchNumber [3],
			multiCurrencyString [2];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	est [3];
	char 	crd_no [7];
	char 	p_invoice [16];
	char 	p_crd_no [7];
	char 	branchNumber_name [41];
	char 	systemDate [11];
	char 	com_date [11];
	double 	inv_unpaid;	
	double	new_inv_bal;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "Supplier", 4, 16, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier No.", "Enter Supplier or [SEARCH] ", 
		NE, NO, JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, LIN, "name", 4, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "doc_no", 5, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Invoice No. ", "Enter invoice no. or [SEARCH] ", 
		NE, NO, JUSTLEFT, "", "", suin_rec.inv_no}, 
	{1, LIN, "approv", 5, 55, CHARTYPE, 
		"A", "          ", 
		" ", " ", "Approved ", " ", 
		NA, NO, JUSTLEFT, "", "", suin_rec.approved}, 
	{1, LIN, "curr", 6, 16, CHARTYPE, 
		"AAA", "          ", 
		" ", " ", "Currency Code", " ", 
		NA, NO, JUSTLEFT, "", "", suin_rec.currency}, 
	{1, LIN, "date_due", 6, 55, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", " ", "Date Due.", " ", 
		NI, NO, JUSTRIGHT, "", "", (char *)&suin_rec.pay_date}, 
	{1, LIN, "amount", 8, 16, MONEYTYPE, 
		"NN,NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Amount", " ", 
		NA, NO, JUSTRIGHT, "0", "99999999999", (char *)&local_rec.inv_unpaid}, 
	{1, LIN, "payamt", 9, 16, MONEYTYPE, 
		"NN,NNN,NNN,NNN.NN", "          ", 
		" ", " ", "Pay Amt", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999999", (char *)&suin_rec.pay_amt}, 
	{1, LIN, "balance", 10, 16, MONEYTYPE, 
		"NN,NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Balance", " ", 
		NA, NO, JUSTRIGHT, "0", "99999999999", (char *)&local_rec.new_inv_bal}, 
	{1, LIN, "held", 10, 55, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Held Reason", " ", 
		NO, NO, JUSTLEFT, "", "", suin_rec.hold_reason}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*===========================
| Local function prototypes |
===========================*/
void    shutdown_prog    (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
int     spec_valid       (int);
int		Update			 (void);
void	SrchSuin		 (char *);
int     heading          (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	sprintf (multiCurrencyString, "%-1.1s", get_env ("CR_MCURR"));
	
	if (!multiCurrency) 
		vars [label ("curr")].required = ND;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty	();
	set_masks ();
	init_vars (1);
	adjustPayment = FALSE;

	if (!strcmp (argv [0], "cr_invpay"))
		adjustPayment = TRUE;
	
	if (adjustPayment)
	{
		FLD ("date_due") = NI;
		FLD ("amount")   = NA;
		FLD ("payamt")   = YES;
		FLD ("balance")  = NA;
		FLD ("held")     = NO;
	}
	else
	{
		FLD ("date_due") = YES;
		FLD ("amount")   = ND;
		FLD ("payamt")   = ND;
		FLD ("balance")  = ND;
		FLD ("held")     = ND;
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	
	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	strcpy (local_rec.com_date, DateToString (comm_rec.crd_date));
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.p_invoice,"000000000000000");
	strcpy (local_rec.p_crd_no, "000000");

	while (prog_exit == 0) 
	{
		abc_unlock (suin);
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

		/*----------------------------------
		| Update invoice.                  |
		----------------------------------*/
		if (!restart)
		{
			if (Update ()) 
			{
				shutdown_prog ();
				return (EXIT_SUCCESS);
			}
		}

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Supplier Number Input. |
	---------------------------------*/
	if (LCHECK ("Supplier"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Invoice Number Input. |
	--------------------------------*/
	if (LCHECK ("doc_no"))
	{
		if (SRCH_KEY)
		{
			SrchSuin (temp_str);
			return (EXIT_SUCCESS);
		}
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (err_str, ML (mlCrMess032),suin_rec.inv_no);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (!INVOICE && !CREDIT)
		{
			errmess (ML (mlCrMess101));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		/*local_rec.inv_unpaid = suin_rec.amt - suin_rec.amt_paid;*/
		local_rec.inv_unpaid = suin_rec.amt;
		if (local_rec.inv_unpaid == 0.00)
		{
			sprintf (err_str,ML (mlCrMess042),local_rec.inv_unpaid);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		local_rec.new_inv_bal = local_rec.inv_unpaid - suin_rec.pay_amt;

		DSP_FLD ("approv");
		DSP_FLD ("curr");
		DSP_FLD ("date_due");
		DSP_FLD ("amount");
		DSP_FLD ("balance");
		DSP_FLD ("payamt");
		DSP_FLD ("held");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Payment Due Date.     |
	--------------------------------*/
	if (LCHECK ("date_due"))
	{
		if (dflt_used)
			suin_rec.pay_date = StringToDate (prv_ntry);

		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Pay Amount. |
	----------------------*/
	if (LCHECK ("payamt"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			suin_rec.pay_amt = CENTS (atof (prv_ntry));
			DSP_FLD ("payamt");
		}

		if (twodec (suin_rec.pay_amt) > twodec (local_rec.inv_unpaid))
		{
			sprintf (err_str, ML (mlCrMess102),DOLLARS (suin_rec.pay_amt));
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		local_rec.new_inv_bal = twodec (local_rec.inv_unpaid - suin_rec.pay_amt);
		DSP_FLD ("balance");
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Held Reason |
	----------------------*/
	if (LCHECK ("held"))
	{
		if (F_HIDE (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (suin_rec.hold_reason, prv_ntry);
			DSP_FLD ("held");
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-----------------
| Update Files.   |
-----------------*/
int
Update (void)
{
	clear ();

	/*------------------------
	| Update Invoice.        |
	------------------------*/
	strcpy (local_rec.p_invoice,suin_rec.inv_no);

	cc = abc_update (suin,&suin_rec);
	if (cc) 
		file_err (cc, suin, "DBUPDATE");
	
	return (EXIT_SUCCESS);
}

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchSuin (
 char *key_val)
{
	char	disp_amt [37];

	_work_open (15,0,40);
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no,key_val);
	save_rec ("#Invoice","#    Balance    |   Pay Amount  | App ");
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val,strlen (key_val)) && 
		       suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		local_rec.inv_unpaid = suin_rec.amt - suin_rec.amt_paid;
		if ((INVOICE || CREDIT) && local_rec.inv_unpaid != 0.00)
		{
			sprintf (disp_amt, "%14.2f |%14.2f |  %-1.1s ",
					DOLLARS (local_rec.inv_unpaid),
					DOLLARS (suin_rec.pay_amt),
					suin_rec.approved);

			cc = save_rec (suin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no,temp_str);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	if (adjustPayment)
		rv_pr (ML (mlCrMess103),25,0,1);
	else
		rv_pr (ML (mlCrMess104),25,0,1);

	print_at (0,55,ML (mlCrMess105),local_rec.p_invoice);
	
	line_at (1,0,80);

	box (0,3,80, (adjustPayment) ? 7 : 3);
	if (adjustPayment)
		line_at (7,1,79);
	
	line_at (20,1,78);
	
	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	
	return (EXIT_SUCCESS);
}


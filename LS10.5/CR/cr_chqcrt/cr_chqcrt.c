/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_chqcrt.c,v 5.3 2001/08/20 23:15:43 scott Exp $
|  Program Name  : (cr_chqcrt.c)
|  Program Desc  : (Create cheques for selected supplier inv/cns.)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor   | Date Written  : 26/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_chqcrt.c,v $
| Revision 5.3  2001/08/20 23:15:43  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 08:51:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:13  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqcrt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqcrt/cr_chqcrt.c,v 5.3 2001/08/20 23:15:43 scott Exp $";

#define MCURR		 (multiCurrency [0] == 'Y')
#define CHEQUE		 (local_rec.pay_type [0] == 'C')
#define DRAFT		 (local_rec.pay_type [0] == 'D')
#define REMIT		 (remitanceCheque [0] == 'Y' || remitanceCheque [0] == 'y')
#define COMB		 (remitanceCheque [1] == 'Y' || remitanceCheque [1] == 'y')

#include <pslscr.h>
#include <GlUtils.h>
#include <twodec.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
                                /*===========================================*/
	int		chequeCount = 0,	/* Counts no of cheques created              */
			detailsCount = 0,	/* Counts no of cheque details created	     */
			firstCheque = FALSE,/* Flags if first payment or not for cheque  */
			currentApMonth,		/* Current Supplier Month		     		 */
			pidNumber = 0L,  	/* Process identifier number               	 */
			printerNumber;     	/* Line printer number                       */
                                /*                                           */
	long	lsystemDate = 0L,	/* LONG version of todays date		     	 */
			chequeSeqNo = 1L;	/* Cheque Seq No (replaced by actual no)	 */
                                /*                                           */
	char	printerString [3],	/* Line printer number string                */
			pidString [7],		/* Line printer number string                */
			multiCurrency [2];	/* Multi currency control flag (Y/N) 	     */
                                /*===========================================*/
	char	remitanceCheque [3];

	double	remitanceTotal = 0.00;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr_rec;
struct crbkRecord	crbk_rec;
struct suinRecord	suin_rec;
struct suhdRecord	suhd_rec;
struct sudtRecord	sudt_rec;


Money	*sumr_balance	=	&sumr_rec.bo_curr;

	int		envDbCo = 0,
			cr_find = 0;
	char	branchNo [3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	double	new_rate;
	long	chq_date;
	char 	systemDate [11];
	char 	OverRideExch [2];
} local_rec;

/*==========================
| Screen Static Structure. |
==========================*/
static	struct	var	vars [] =
{
	{1, LIN, "cheque_date",	 4, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Cheque/Draft Date", "<retn> defaults to system (todays) date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.chq_date},
	{1, LIN, "bank_id",	 5, 20, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Bank Code", "enter code or [SEARCH]",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "br_name",	 6, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.branch_name},
	{1, LIN, "acct_name",	 7, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Name", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
	{1, LIN, "bank_no",	 8, 20, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank No.", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_no},
	{1, LIN, "bk_acno",	 8, 60, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "     Account No.", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
	{1, LIN, "bk_curr",	10, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "curr_desc",	10, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "ovr_exch",	11, 20, CHARTYPE,
		"U", "          ",
		" ", "N", "Override Exch Rate", "Enter Y(es) or default to N(o)",
		YES, NO,  JUSTLEFT, "NY", "", local_rec.OverRideExch},
	{1, LIN, "new_rate",	11, 60, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "1.0000", "     Exchange Rate", "<retn> defaults to current rate for currency ",
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.new_rate},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		    (void);
void	ReadComm		 (void);
void	SrchPocr		 (char *);
void	SrchCrbk		 (char *);
int		spec_valid		 (int);
int		RunPrograms		 (char *, char *, char *);
int		ProcessSumr		 (void);
int		GetRemitTotal	 (long);
double	GetSudtAmtPaid	 (long);
int		ProcessSuin		 (long);
int		MakeCheque		 (double, double, double, double);
int		heading			 (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	if (argc != 3)
	{
		print_at (0,0, mlCrMess138, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);
	pidNumber     = atoi (argv [2]);

	cr_find  = atoi (get_env ("CR_FIND"));
	envDbCo = atoi (get_env ("CR_CO"));

	sprintf (remitanceCheque, "%-2.2s", get_env ("CR_REMIT"));
	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	/*------------------------------------------------------
	| Reset screen control if not multi-currency suppliers.|
	------------------------------------------------------*/
	if (!MCURR)
	{
		FLD ("bk_curr")   = ND;
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
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	ReadComm ();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);
	
	while (prog_exit == 0) 
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
	
		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		if (MCURR)
		{
			FLD ("ovr_exch") = YES;
			FLD ("new_rate") = NO;
		}
	
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (restart) 
			continue;

		if (prog_exit)
		{
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
	
		edit_all ();
		if (restart) 
			continue;

		dsp_screen ("Processing Cheques / Drafts", 
					comm_rec.co_no, comm_rec.co_name);

		/*-----------------------------------------------
		| Process all suppliers under current co.	|
		-----------------------------------------------*/
		ProcessSumr ();

		/*---------------------------------
		| Process Cheques.                |
		---------------------------------*/
		if (chequeCount <= 0)
		{
			shutdown_prog ();
			return (EXIT_FAILURE);
		}

		clear ();
		snorm ();
		sprintf (printerString,"%d",printerNumber);
		sprintf (pidString,"%d",pidNumber);
	
		/*-------------------------
		| Print seperate Cheques. |
		-------------------------*/
		if (COMB)
		{
			if (RunPrograms ("cr_rc_prn", printerString, pidString))
			{
				return (EXIT_FAILURE);
			}
		}
		else
		{
			if (RunPrograms ("cr_chqprn", printerString, pidString))
			{
				return (EXIT_FAILURE);
			}
		}

		if (RunPrograms ("cr_chqrprn", printerString, pidString))
		{
			return (EXIT_FAILURE);
		}

		if (RunPrograms ("cr_chqnoinp", pidString, "DUMMY"))
		{
			return (EXIT_FAILURE);
		}

		prog_exit = TRUE;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===============
| Exit program. |
===============*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}
/*======================
| Open database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsi_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!cr_find) ? "sumr_id_no2" 
							    					    : "sumr_id_no4");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsu_hash");

	OpenPocr ();
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
}

/*=======================
| Close database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_fclose (suin);
	GL_Close ();
	abc_fclose (crbk);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadComm (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");

	if (cc)
		file_err (cc, comr, "DBFIND");
		
	abc_fclose (comr);

	DateToDMY (comm_rec.crd_date, NULL, &currentApMonth, NULL);
	chequeSeqNo = comr_rec.ls_chq_no;
}

/*============================
| Special Validation Section.|
============================*/
int
spec_valid (
 int field)
{
	long wk_date = 0L;

	/*-------------------------------
	| Validate Cheque Date.         |	
	-------------------------------*/
	if (LCHECK ("cheque_date"))
	{
		if (dflt_used) 
			DSP_FLD ("cheque_date");

		wk_date = MonthStart (comm_rec.crd_date);
		if (local_rec.chq_date < wk_date)
		   return print_err (ML (mlCrMess171)); 

		wk_date = MonthEnd (comm_rec.crd_date) + 1L;
		if (local_rec.chq_date > MonthEnd (wk_date))
		   return print_err (ML (mlCrMess136)); 

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Bank Id Code And Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("bk_name");
		DSP_FLD ("br_name");
		DSP_FLD ("acct_name");
		DSP_FLD ("bank_no");
		DSP_FLD ("bk_acno");
		DSP_FLD ("bk_acno");

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		DSP_FLD ("bk_curr");
		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code, crbk_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
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

		if (local_rec.OverRideExch [0] == 'N')
		{
			local_rec.new_rate = pocrRec.ex1_factor;
			if (local_rec.new_rate == 0.0000)
			  	local_rec.new_rate = 1.0000;

			FLD ("new_rate") = NA;
			skip_entry = 1;	
		}	
		else
			FLD ("new_rate") = NO;

		DSP_FLD ("new_rate");
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
			local_rec.new_rate = pocrRec.ex1_factor;

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
	strcpy (pocrRec.co_no,comm_rec.co_no);
	sprintf (pocrRec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no,comm_rec.co_no) && 
		      	  !strncmp (pocrRec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocrRec.code, pocrRec.description);                       
		if (cc)
			break;
		cc = find_rec (pocr,&pocrRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
        return;
	strcpy (pocrRec.co_no,comm_rec.co_no);
	sprintf (pocrRec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*===================
| Execute Programs. |
===================*/
int
RunPrograms (
 char *	name,
 char *	run1,
 char *	run2)
{
	char szBuff [100];
	strcpy (szBuff, name);
	strcat (szBuff, " ");
	strcat (szBuff, run1);
	strcat (szBuff, " ");
	strcat (szBuff, run2);
	sys_exec (szBuff);

	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Suppliers Bank File. |
=========================================*/
void
SrchCrbk (
 char *	key_val)
{
	work_open ();
	cc = save_rec ("#Bank Id ","#Bank Name             ");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
	              !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND");
}


/*=======================================================
|	Routine which reads all of the suppliers for		|
|	the current company.  The supplier is checked		|
|	to see if any invoices need to be paid. 			|
|	If this is true then a payment record etc is		|
|	created for this supplier.							|
=======================================================*/
int
ProcessSumr (
 void)
{
	double	balance = 0.00;

	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.est_no,branchNo);
	strcpy (sumr_rec.acronym,"         ");
	
	cc = find_rec (sumr, &sumr_rec, GTEQ, "u");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no) &&
				  !strcmp (sumr_rec.est_no,branchNo))
	{
		dsp_process ("Supplier :", sumr_rec.acronym);

		/*---------------------------------------------------------
		| If cred. not 'S (elected)' or wrong pay type - get next. |
		---------------------------------------------------------*/
		if (sumr_rec.stat_flag [0] != 'S' ||
			sumr_rec.hold_payment [0] == 'Y' ||
			sumr_rec.pay_method [0] != 'C')
		{
			abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
			continue;
		}

		/*-------------------------------------
		| If cred. wrong currency - get next. |
		-------------------------------------*/
		if (MCURR && strcmp (sumr_rec.curr_code, crbk_rec.curr_code))
		{
			abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
			continue;
		}

		/*-------------------------------------
		| If remittance total <= 0 - get next.|
		-------------------------------------*/
		GetRemitTotal (sumr_rec.hhsu_hash);
		remitanceTotal = no_dec (remitanceTotal);
		if (remitanceTotal <= 0.00)
		{
			abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
			continue;
		}

		/*----------------------------------------------------
		| If current and last three months >= 0 then process.|
		----------------------------------------------------*/
		balance = (sumr_balance [0] + sumr_balance [1] + 
				   sumr_balance [2] + sumr_balance [3]);

		if (balance >= 0.0)
		{
			firstCheque	 = TRUE;
			detailsCount = 0;

			ProcessSuin (sumr_rec.hhsu_hash);
			
			if (detailsCount > 0)
			{
				/*-------------------------------------------
				| Update Cheque Header With Total Amount.   |
				-------------------------------------------*/
				sprintf (suhd_rec.cheq_no,"%015ld",chequeSeqNo);
				chequeSeqNo ++;
				chequeCount ++;
				strcpy (suhd_rec.stat_flag,"C");
				strcpy (suhd_rec.rem_prt, (REMIT) ? "R" : "0");
				cc = abc_update (suhd,&suhd_rec);
				if (cc)
					file_err (cc, suhd, "DBUPDATE");
			}
		}

		strcpy (sumr_rec.stat_flag, "0");
		cc = abc_update (sumr,&sumr_rec);
		if (cc)
			file_err (cc, sumr, "DBUPDATE");

		cc = find_rec (sumr, &sumr_rec, NEXT, "u");
	}
	abc_unlock (sumr);
	return (EXIT_SUCCESS);
}


/*===============================================================
| Routine which calculates remit total for current supplier     |
===============================================================*/
int
GetRemitTotal (
 long hhsuHash)
{
	double	amountLeft = 0.00,    /* Amount to be paid off invoice.    */
			paymentAmount = 0.00; /* Amount paid off inv/cn this time  */	

	remitanceTotal = 0.00;

	cc = find_hash (suin, &suin_rec, GTEQ, "u", hhsuHash);
	while (!cc && suin_rec.hhsu_hash == hhsuHash)
	{
		if (suin_rec.stat_flag [0] != 'S')
		{
			abc_unlock (suin);
			cc = find_hash (suin,&suin_rec,NEXT,"u",hhsuHash);
			continue;
		}
		
		amountLeft = suin_rec.amt - suin_rec.amt_paid;
		/*---------------------------------------
		| Process invoice payment.             	|
		---------------------------------------*/
		if (suin_rec.type [0] == '1')
		{
			if (suin_rec.pay_amt <= amountLeft)
				paymentAmount = suin_rec.pay_amt;
			else 
				paymentAmount = amountLeft;
		}
		/*---------------------------------------
		| Process credit note.                 	|
		---------------------------------------*/
		else
			paymentAmount = amountLeft;

		remitanceTotal += paymentAmount;

		cc = find_hash (suin, &suin_rec, NEXT, "u", hhsuHash);
	}

	return (EXIT_SUCCESS);
}

/*===========================================
|	Routine which will read all invoices	|
|	on file for a particular supplier.		|
|	Returns: non-zero if not ok.			|
===========================================*/
int
ProcessSuin (
 long	hhsuHash)
{
	double	amountLeft = 0.00,		/* Amount to be paid off invoice.    */
			exchangeRate = 0.00,	/* Invoice/cn exchange rate used.    */	
			origExchangeRate = 0.00,/* Invoice/cn exchange rate.         */	
     	    exchangeVariance = 0.00,/* Exchange variance for inv/cn.     */
			paymentAmount = 0.00,	/* Amount paid off inv/cn this time  */	
			payAmountLocal = 0.00;	/* Amount paid off inv/cn (local curr)*/	

	cc = find_hash (suin, &suin_rec, GTEQ, "u", hhsuHash);
	while (!cc && suin_rec.hhsu_hash == hhsuHash)
	{
		if (suin_rec.stat_flag [0] != 'S')
		{
			abc_unlock (suin);
			cc = find_hash (suin,&suin_rec,NEXT,"u",hhsuHash);
			continue;
		}
		
		amountLeft = suin_rec.amt - GetSudtAmtPaid (suin_rec.hhsi_hash);
		/* amountLeft = suin_rec.amt - suin_rec.amt_paid;*/
		/*---------------------------------------
		| Process invoice payment.             	|
		---------------------------------------*/
		if (suin_rec.type [0] == '1')
		{
			if (suin_rec.pay_amt <= amountLeft)
				paymentAmount = suin_rec.pay_amt;
			else 
				paymentAmount = amountLeft;
		}
		/*---------------------------------------
		| Process credit note.                 	|
		---------------------------------------*/
		else
			paymentAmount = amountLeft;

		/*---------------------------------------
		| Convert payment to local currency.   	|
		---------------------------------------*/
		if (MCURR)
		{
			if (suin_rec.er_fixed [0] != 'Y' && local_rec.OverRideExch [0] == 'Y')
				exchangeRate = local_rec.new_rate;
			else
				exchangeRate = suin_rec.exch_rate;

			if (exchangeRate == 0.00)
				exchangeRate = 1.00;

			payAmountLocal = paymentAmount / exchangeRate;
			payAmountLocal = no_dec (payAmountLocal);

			origExchangeRate = suin_rec.exch_rate;
			if (origExchangeRate == 0.00)
				origExchangeRate = 1.00;

			exchangeVariance = (paymentAmount / origExchangeRate) - 
								payAmountLocal;
			exchangeVariance = no_dec (exchangeVariance);
		}
		else
		{
			payAmountLocal = paymentAmount;
			exchangeRate = 1.00;
			exchangeVariance = 0.00;
		}

		/*---------------------------------------
		| Raise payment detail.                	|
		---------------------------------------*/
		if (paymentAmount != 0.0)
			cc =	MakeCheque 
					 (
						paymentAmount, 
						payAmountLocal, 
			            exchangeRate, 
						exchangeVariance
					);

		/*---------------------------------------
		| Update invoice record with payment.  	|
		---------------------------------------*/
		suin_rec.amt_paid += paymentAmount;
		suin_rec.pay_amt = 0.00;
		strcpy (suin_rec.stat_flag, "0");
		cc = abc_update (suin, &suin_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");

		cc = find_hash (suin, &suin_rec, NEXT, "u", hhsuHash);
	}
	abc_unlock (suin);
	return (EXIT_SUCCESS);
}
			
/*=======================================================
|	Routine to pay a invoice. Adds detail record		|
|	and header if first time through (Header will		|
|	be updated later on after full amount of			|
|	cheque is known).									|
=======================================================*/
int
MakeCheque (
 double	amt,				/* Amount to pay off current invoice			*/
 double amt_loc,			/* Local amount to pay off current invoice		*/
 double exchangeRate,		/* Exchange rate applicable for invoice			*/
 double exchangeVariance)	/* Exchange variation applicable for invoice	*/
{
	int	invoiceMonth;			/* Invoice month of invoice used for ageing		*/

	/*-------------------------------
	| Create cheque header etc		|
	-------------------------------*/
	if (firstCheque == TRUE)
	{
		firstCheque 			= FALSE;
		suhd_rec.pid 			= pidNumber;
		suhd_rec.hhsu_hash 		= sumr_rec.hhsu_hash;
		suhd_rec.date_post		= lsystemDate;
		suhd_rec.tot_amt_paid 	= 0.00;
		suhd_rec.loc_amt_paid 	= 0.00;
		suhd_rec.disc_taken 	= 0.00;
		suhd_rec.loc_disc_take  = 0.00;
		suhd_rec.exch_variance 	= 0.00;
		suhd_rec.date_payment 	= local_rec.chq_date;
		strcpy (suhd_rec.cheq_no,"ZZZZZZZZZZZZZZZ");
		strcpy (suhd_rec.narrative,"                    ");
		strcpy (suhd_rec.bank_id,crbk_rec.bank_id);
		strcpy (suhd_rec.pay_type, "C");
		strcpy (suhd_rec.rem_prt, (REMIT) ? "R" : "0");
		strcpy (suhd_rec.stat_flag,"X");
		cc = abc_add (suhd,&suhd_rec);
		if (cc)
			file_err (cc, suhd, "DBADD");

		/*-------------------------------
		| Re-read record just added	|
		-------------------------------*/
		suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suhd_rec.cheq_no,"ZZZZZZZZZZZZZZZ");
		cc = find_rec (suhd,&suhd_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, suhd, "DBFIND");

	}
	suhd_rec.tot_amt_paid += amt;
	suhd_rec.loc_amt_paid += amt_loc;
	suhd_rec.exch_variance += exchangeVariance;

	/*-----------------------------------------------
	| Add payment detail record for inv/cn.        	|
	-----------------------------------------------*/
	sudt_rec.hhsp_hash 		= suhd_rec.hhsp_hash;
	sudt_rec.hhsi_hash 		= suin_rec.hhsi_hash;
	sudt_rec.amt_paid_inv 	= amt;
	sudt_rec.loc_paid_inv 	= amt_loc;
	sudt_rec.exch_rate 		= exchangeRate;
	sudt_rec.exch_variatio  = exchangeVariance;
	strcpy (sudt_rec.stat_flag,"C");

	cc = abc_add (sudt,&sudt_rec);
	if (cc)
		file_err (cc, sudt, "DBADD");
		
	detailsCount ++;
	/*---------------------------------------------------
	| Subtract payment from current balance owing etc	|
	---------------------------------------------------*/
	DateToDMY (suin_rec.date_of_inv, NULL, &invoiceMonth, NULL);
	if (invoiceMonth > currentApMonth)
		invoiceMonth -= 12;

	invoiceMonth -= currentApMonth;
	if (invoiceMonth < 0)
		invoiceMonth *= -1;

	if (invoiceMonth > 3)
		invoiceMonth = 3;

	sumr_balance [invoiceMonth] -= amt;

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
		rv_pr (ML (mlCrMess170),25,0,1);
		fflush (stdout);
		move (0,1);
		line (80);

		if (MCURR)
		{
			move (1,input_row);
			box (0,3,80,8);
			move (1,9);
			line (79);
		}
		else
		{
			move (1,input_row);
			box (0,3,80,5);
		}

		move (1,20);
		line (78);
		sprintf (err_str,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (21,0,err_str);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

double	GetSudtAmtPaid (
	long	hhsi_hash)
{
	double	amt_paid = 0.00;

	sudt_rec.hhsi_hash = hhsi_hash;
	cc = find_rec (sudt,&sudt_rec,COMPARISON,"r");
	while (!cc && sudt_rec.hhsi_hash == hhsi_hash)
	{
		amt_paid += sudt_rec.amt_paid_inv;
		cc = find_rec (sudt,&sudt_rec,NEXT,"r");
	}
	return (amt_paid);
}

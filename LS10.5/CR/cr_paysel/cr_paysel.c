/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_paysel.c,v 5.6 2002/07/17 09:57:03 scott Exp $
|  Program Name  : (cr_paysel.c  ) 
|  Program Desc  : (Supplier Payment Selection Input.         ) 
|                  (Selection By Supplier/All & Payment Date  ) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_paysel.c,v $
| Revision 5.6  2002/07/17 09:57:03  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2002/07/16 01:02:34  scott
| Updated from service calls and general maintenance.
|
| Revision 5.4  2001/12/07 04:34:21  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/07 01:42:53  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_paysel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_paysel/cr_paysel.c,v 5.6 2002/07/17 09:57:03 scott Exp $";

#define	MCURR		 (multiCurrency [0] == 'Y')

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <FindSumr.h>

#include    "schema"

struct commRecord   comm_rec;
struct esmrRecord   esmr_rec;
struct sumrRecord   sumr_rec;
struct pocrRecord   pocr_rec;
struct suinRecord   suin_rec;
struct comrRecord   comr_rec;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		numberInvoices 	= 0,
			envCrCo 		= 0,
			envCrFind 		= 0;

	char	selectType [2],
	    	multiCurrency [2],
	    	branchNumber [3];
	
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	crd_no [7];
	char	crd_name [41];
	char	from_crd_no [7];
	char	from_crd_name [41];
	char	to_crd_no [7];
	char	to_crd_name [41];
	char	inv_no [7];
	char	crd_date [11];
	char	curr_code [4];
	long	pay_date;
	char	pr_vouch [2];
	int		printerNo;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "supplier", 4, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "*A", " Select Supplier", "Enter Supplier No. or [SEARCH], *A(ll), *R(ange)", 
		NE, NO, JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, LIN, "name", 4, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.crd_name}, 
	{1, LIN, "beginSupplier", 5, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " From Supplier", "Enter Supplier No. or [SEARCH] key", 
		NE, NO, JUSTLEFT, "", "", local_rec.from_crd_no}, 
	{1, LIN, "beginSupplierName", 5, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.from_crd_name}, 
	{1, LIN, "endSupplier", 6, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " To Supplier", "Enter Supplier No or [SEARCH] Key", 
		NE, NO, JUSTLEFT, "", "", local_rec.to_crd_no}, 
	{1, LIN, "endSupplierName", 6, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.to_crd_name}, 
	{1, LIN, "currencyCode", 8, 18, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", " Currency Code", "Enter Currency Code, [SEARCH] or <retn> for A(ll)", 
		YES, NO, JUSTLEFT, "", "", local_rec.curr_code}, 
	{1, LIN, "currencyDesc", 8, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", pocr_rec.description}, 
	{1, LIN, "pay_due", 9, 18, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.crd_date, " Pay Invoices Due.", "<retn> defaults to supplier end of month date ", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.pay_date}, 
	{1, LIN, "print_vouch", 11, 18, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Print Voucher", "Enter Y)es or N)o", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.pr_vouch}, 
	{1, LIN, "printerNo", 12, 18, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No  ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.printerNo}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	ReadComm		 (void);
int		spec_valid		 (int);
void	SrchPocr		 (char *);
int		ProcSupplier	 (void);
void	ProcessTrans	 (void);
int		ProcSuin		 (long);
int		heading			 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	
	if (!MCURR) 
	{
		FLD ("currencyCode") 	= ND;
		FLD ("currencyDesc") 	= ND;
	}

	init_scr ();	
	set_tty ();
	set_masks ();
	init_vars (1);

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		ProcSupplier ();

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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (local_rec.crd_date, DateToString (comm_rec.crd_date));

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}	

/*========================
| Close data base files. |	
========================*/
void
CloseDB (void)
{
	abc_unlock (sumr);
	abc_unlock (suin);
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (pocr);
	abc_fclose (esmr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Supplier Number Input. |
	---------------------------------*/
	if (LCHECK ("supplier"))
	{
		if (!strcmp (local_rec.crd_no, "*A    "))
		{
			strcpy (local_rec.crd_no,"ALL   ");
			DSP_FLD ("supplier");
			strcpy (selectType,"A");
			FLD ("beginSupplier") 	= NA;
			FLD ("endSupplier") 	= NA;
			FLD ("currencyCode")	= YES;
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.crd_no, "*R    "))
		{
			strcpy (local_rec.crd_no,"RANGE ");
			DSP_FLD ("supplier");
			strcpy (selectType,"R");
			FLD ("beginSupplier") 	= YES;
			FLD ("endSupplier") 	= YES;
			FLD ("currencyCode")	= YES;
			return (EXIT_SUCCESS);
		}

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

		if (sumr_rec.hold_payment [0] == 'Y')
		{
			sprintf (err_str, ML (mlCrMess022),sumr_rec.crd_no);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code,sumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (MCURR)
		{
			strcpy (local_rec.curr_code,sumr_rec.curr_code);
			FLD ("currencyCode")	= NA;
			DSP_FLD ("currencyCode");
			DSP_FLD ("currencyDesc");
		}
		strcpy (local_rec.crd_name,sumr_rec.crd_name);
		DSP_FLD ("name");
		strcpy (selectType,"C");
		FLD ("beginSupplier") = NA;
		FLD ("endSupplier") = NA;
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate From Supplier Number . |
	---------------------------------*/
	if (LCHECK ("beginSupplier"))
	{
		if (selectType [0] != 'R')
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.from_crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.from_crd_name,sumr_rec.crd_name);
		DSP_FLD ("beginSupplier");
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate To Supplier Number .   |
	---------------------------------*/
	if (LCHECK ("endSupplier"))
	{
		if (selectType [0] != 'R')
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.to_crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.to_crd_no,local_rec.from_crd_no) < 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.to_crd_name,sumr_rec.crd_name);
		DSP_FLD ("endSupplierName");
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Currency Code Input.   |
	---------------------------------*/
	if (LCHECK ("currencyCode"))
	{
		if (!MCURR || selectType [0] == 'C')
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.curr_code,"ALL");
			DSP_FLD ("currencyCode");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
		   SrchPocr (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code,local_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("currencyDesc");

		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Print Voucher Input | 
	-------------------------------*/
	if (LCHECK ("print_vouch"))
	{
		FLD ("printerNo") = (local_rec.pr_vouch [0] == 'Y') ? YES : NA;
		return (EXIT_SUCCESS);
	}	

	/*----------------------
	| Validate Printer No. |
	----------------------*/
	if (LCHECK ("printerNo"))
	{
		if (F_NOKEY (label ("printerNo")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}
		
		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================
| Search for currency pocr code |
===============================*/
void
SrchPocr (
 char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Currency Description");
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no) && 
				  !strncmp (pocr_rec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocr_rec.code, pocr_rec.description);                       
		if (cc)
		        break;
		cc = find_rec (pocr,&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*===================================================
|	Routine which reads selected supplier for  	|
|	the current company.  The supplier is checked	|
|	to see if any invoices need to be paid. 		|
===================================================*/
int
ProcSupplier (void)
{
	dsp_screen ("Selecting Invoices / Credit Notes For Payment ",comm_rec.co_no,
				comm_rec.co_name);

	/*---------------------------------
	| Process Selected Supplier.      |
	---------------------------------*/
	if (selectType [0] == 'C')
	{
		ProcessTrans ();
		abc_unlock (sumr);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Process All Supplier.          |
	---------------------------------*/
	else if (selectType [0] == 'A')
	{
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,"  ");
		strcpy (sumr_rec.crd_no,"      ");
		cc = find_rec (sumr, &sumr_rec, GTEQ, "u");
		while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no))
		{
			ProcessTrans ();
			abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Process Range Of Supplier.     |
	---------------------------------*/
	else if (selectType [0] == 'R')
	{
		abc_selfield (sumr,"sumr_id_no3");
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.crd_no,local_rec.from_crd_no);

		cc = find_rec (sumr, &sumr_rec, GTEQ, "u");
		while (!cc && strcmp (sumr_rec.crd_no,local_rec.to_crd_no) <= 0)
		{
			ProcessTrans ();
			abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Routine to process sumr record.             	|
===============================================*/
void
ProcessTrans (void)
{
	char	cmd [100];
	double	balance = 0.00;

	/*------------------------------
	| Test for hold on supplier.   |
	------------------------------*/
	if (sumr_rec.hold_payment [0] == 'Y')
		return;

	/*------------------------------
	| Test for currency selection. |
	------------------------------*/
	if (MCURR)
	   if (selectType [0] != 'C' && strcmp (local_rec.curr_code,"ALL") != 0)
	      if (strcmp (local_rec.curr_code,sumr_rec.curr_code) != 0)
			return; 

   	/*----------------------------------------------------
   	| If current and last three months > 0 then process. |
   	----------------------------------------------------*/
	numberInvoices = 0;

	balance = (sumr_rec.bo_curr + sumr_rec.bo_per1 + 
               	   sumr_rec.bo_per2 + sumr_rec.bo_per3);

	if (balance > 0.0)
		ProcSuin (sumr_rec.hhsu_hash);
	    	 
	if (numberInvoices > 0)
	{
		strcpy (sumr_rec.stat_flag, "S");
		cc = abc_update (sumr,&sumr_rec);
		if (cc)
			file_err (cc, sumr, "DBUPDATE");

		/*----------------------------------
		| Add printing of payment voucher. |
		----------------------------------*/
		if (local_rec.pr_vouch [0] == 'Y')
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, esmr, "DBFIND");
		
			esmr_rec.nx_voucher_no++;
			cc = abc_update (esmr, &esmr_rec);
			if (cc)
				file_err (cc, esmr, "DBUPDATE");

			sprintf (cmd, "cr_payprt \"%d\" \"%10ld\" \"%10ld\"", 
											local_rec.printerNo, 
											sumr_rec.hhsu_hash,
											esmr_rec.nx_voucher_no);
			SystemExec (cmd, FALSE);
		}
	}
}

/*===========================================
|	Routine which will read all invoices	|
|	on file for a particular supplier.		|
|	Returns: non-zero if not ok.			|
===========================================*/
int
ProcSuin (
	long	hhsuHash)
{
	suin_rec.hhsu_hash = hhsuHash;
	strcpy (suin_rec.inv_no,"               ");
	cc = find_rec (suin, &suin_rec, GTEQ, "u");
	while (!cc && suin_rec.hhsu_hash == hhsuHash)
	{
		/*-------------------------------------------
		| Invoice is not approved or                |
		| payment not due or payment amount = 0.00. |
		-------------------------------------------*/
		if (suin_rec.approved [0] != 'Y' ||
		     suin_rec.pay_date > local_rec.pay_date ||
		     suin_rec.pay_amt == 0.00)
		{
			abc_unlock (suin);
			cc = find_rec (suin, &suin_rec, NEXT, "u");
			continue;
		}
		numberInvoices ++;
		dsp_process ("Invoice",suin_rec.inv_no);
		strcpy (suin_rec.stat_flag, "S");
		cc = abc_update (suin, &suin_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");
		
		cc = find_rec (suin, &suin_rec, NEXT, "u");
	}
	abc_unlock (suin);
	return (EXIT_SUCCESS);
}

/*===============================================
| Screen Heading Routine.                       |
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
		rv_pr (ML (mlCrMess157), 25,0,1);

		box (0,3,80,9);
		line_at (1,0,80);
		line_at (7,1,79);
		line_at (10,1,79);
		line_at (20,0,80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}


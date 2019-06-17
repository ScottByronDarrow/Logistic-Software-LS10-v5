/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_paysli.c,v 5.10 2002/07/24 08:38:46 scott Exp $
|  Program Name  : (cr_paysli.c)
|  Program Desc  : (Supplier Invoice Payment Selection Program.) 
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 13/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_paysli.c,v $
| Revision 5.10  2002/07/24 08:38:46  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.9  2002/07/17 09:57:03  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.8  2002/07/16 06:22:18  scott
| Updated from service calls and general maintenance.
|
| Revision 5.7  2002/06/25 03:17:04  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.6  2002/06/21 04:10:27  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2001/12/07 04:45:53  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.4  2001/12/07 02:04:48  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_paysli.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_paysli/cr_paysli.c,v 5.10 2002/07/24 08:38:46 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	5000
#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <twodec.h>
#include <FindSumr.h>

#define	MCURR		 	 (multiCurrency [0] == 'Y')
#define	HOLD_PAYMENT	 (sumr_rec.hold_payment [0] == 'Y')
#define	SLEEP_TIME	2

extern	int	X_EALL;
extern	int	Y_EALL;
extern	int	EnvScreenOK;

#include    "schema"

struct commRecord   comm_rec;
struct esmrRecord   esmr_rec;
struct sumrRecord   sumr_rec;
struct suinRecord   suin_rec;

	/*
	 * Special fields and flags
	 */
   	int		envCrCo		= 0,
			envCrFind	= 0,
			clearOk		= 0,
			findRecord	= FALSE;

	char 	multiCurrency [2],
	    	branchNumber [3];

	struct storeRec {
   		double 	itemPayment;
	} store [MAXLINES];

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char 	crd_no [7];
	char 	inv_no [16];
	long	date_of_inv;
	long	pay_date;
	double	exch_rate;
	double	net_inv;
	double	pay_amt;
	double	pay_amt_loc;
	char 	pay [2];
	char 	p_crd_no [7];
	char 	loc_curr [4];
	char	pr_vouch [2];
	int		printerNo;
} local_rec;

static	struct	var	vars []	={	
	{1, LIN, "supplier", 3, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier No.", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, LIN, "name", 3, 28, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "curr", 4, 18, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Currency Code", " ", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.curr_code}, 
	{1, LIN, "print_vouch", 3, 90, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Print Voucher", "Enter Y)es or N)o", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.pr_vouch}, 
	{1, LIN, "printerNo", 4, 90, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No  ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.printerNo}, 
	{2, TAB, "inv_crd", MAXLINES, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "  Document #    ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.inv_no}, 
	{2, TAB, "inv_date", 0, 0, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", " ", " Doc Date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.date_of_inv}, 
	{2, TAB, "date_due", 0, 0, EDATETYPE, 
		"DD/DD/DD", "        ", 
		" ", " ", " Pay Date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.pay_date}, 
	{2, TAB, "net_inv", 0, 1, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", "    Inv Bal   ", " ", 
		NA, NO, JUSTRIGHT, "0", "99999999999", (char *)&local_rec.net_inv}, 
	{2, TAB, "pay_amt", 0, 1, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", "    Pay Amt   ", " ", 
		NA, NO, JUSTRIGHT, "0", "99999999999", (char *)&local_rec.pay_amt}, 
	{2, TAB, "ex_rate", 0, 0, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", ".0001", "Exchange Rate", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.exch_rate}, 
	{2, TAB, "pay_amt_loc", 0, 1, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", "   Pay Loc  ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.pay_amt_loc}, 
	{2, TAB, "pay", 0, 1, CHARTYPE, 
		"U", "          ", 
		" ", "", "Pay", "Payment Selection: enter <retn> = Y(es) or N(o)", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.pay}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*
 * Local function prototypes 
 */
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
int		LoadSuin		 (void);
void	ClearPayments	 (void);
void	DisplayPayment	 (void);
int		Update			 (void);
void	ClearBox		 (int, int, int, int);
int		heading			 (int);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);


	EnvScreenOK = FALSE;

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	
	if (!MCURR) 
	{
		FLD ("curr")		= ND;
		FLD ("ex_rate") 	= ND;
		FLD ("pay_amt_loc") = ND;
	}

	tab_row	= 6;
	tab_col = 14;
	Y_EALL	= 2;

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);
	swide ();
	
	envCrCo = atoi (get_env ("CR_CO"));
	envCrFind  = atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");
	
	strcpy (local_rec.p_crd_no, "000000");

	while (prog_exit == 0) 
	{
		abc_unlock (sumr);
		abc_unlock (suin);
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		lcount [2]	= 0;
		init_vars (1);
		ClearPayments ();
		clearOk		= TRUE;

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		/*
		 * Enter screen 2 tabular input.
		 */
		DisplayPayment ();
		edit (2);
		if (restart) 
			continue;

		clearOk = FALSE;

		edit_all ();
		if (restart) 
			continue;

		/*
		 * Update selection status.     
		 */
		Update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence	
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files. 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (esmr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	findRecord = FALSE;

	/*
	 * Validate Supplier Number Input. 
	 */
	if (LCHECK ("supplier")) 
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "w");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("name");

		if (HOLD_PAYMENT)
		{
			errmess (ML (mlCrMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("curr");
		if (LoadSuin ())
		{
			sprintf (sumr_rec.crd_name,"%10.10s"," ");
			sprintf (sumr_rec.curr_code,"%3.3s", " ");
			scn_display (1);
			return (EXIT_FAILURE);
		}
		else
			return (EXIT_SUCCESS);
	}

	/*
	 * Validate Payment Flag.          
	 */
	if (LCHECK ("pay"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.pay, "Y");
			DSP_FLD ("pay");
		}
		if (local_rec.pay [0] == 'Y')
		{
			if (local_rec.pay_amt == 0.00)
			{
				errmess (ML (mlCrMess023));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
	    		store [line_cnt].itemPayment = local_rec.pay_amt;
		}
		else
	    		store [line_cnt].itemPayment = 0.00;

		DisplayPayment ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Print Voucher Input 
	 */
	if (LCHECK ("print_vouch"))
	{
		FLD ("printerNo") = (local_rec.pr_vouch [0] == 'Y') ? YES : NA;
		return (EXIT_SUCCESS);
	}	

	/*
	 * Validate Printer No. 
	 */
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

/*
 * Load Invoice / Credit Detail Into Tabular.  
 */
int
LoadSuin (void)
{
	init_vars (2);
	scn_set (2);
	lcount [2] = 0;

	suin_rec.hhsu_hash 		= sumr_rec.hhsu_hash;
	suin_rec.date_of_inv 	= 0L;
	cc = find_rec (suin, &suin_rec, GTEQ, "r");	
		
	while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
	{
		if (suin_rec.approved [0] != 'Y' || suin_rec.amt_paid == suin_rec.amt)
		{		
			cc = find_rec (suin, &suin_rec, NEXT, "r");	
			continue;
		}
		strcpy (local_rec.inv_no,suin_rec.inv_no);
		local_rec.date_of_inv = suin_rec.date_of_inv;
		local_rec.pay_date = suin_rec.pay_date;
		local_rec.net_inv = suin_rec.amt - suin_rec.amt_paid;
		local_rec.pay_amt = suin_rec.pay_amt;

		if (MCURR)
		{
			local_rec.exch_rate = suin_rec.exch_rate;
			if (local_rec.exch_rate == 0.00)
				local_rec.exch_rate = 1.00;
			local_rec.pay_amt_loc = suin_rec.pay_amt / local_rec.exch_rate;
		}

		if (suin_rec.stat_flag [0] == 'S')
		{		
			strcpy (local_rec.pay, "Y");
			store [lcount [2]].itemPayment = local_rec.pay_amt;
		}
		else
		{
			strcpy (local_rec.pay, "N");
			store [lcount [2]].itemPayment = 0.00;
		}

		putval (lcount [2]++);
		cc = find_rec (suin, &suin_rec, NEXT, "r");	
		findRecord = TRUE;
	}

	if (!findRecord)
	{
		errmess (ML (mlStdMess009));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	vars [label ("inv_crd")].row = lcount [2];
	scn_set (1);

	return (EXIT_SUCCESS);
}

/*
 * Clear Allocation Array.          
 */
void
ClearPayments (void)
{
	int	i;

	for (i = 0;i < MAXLINES;i++)
		store [i].itemPayment = 0.00;
}

/*
 * Display Allocation Balance.      
 */
void
DisplayPayment (void)
{
	int	i,
	    max;

	double  totalPayment = 0.00;

	max = (line_cnt > lcount [2]) ? line_cnt + 1 : lcount [2] + 1;

	for (i = 0;i < max ;i++)
		totalPayment += store [i].itemPayment;

	move (1, 19);
	cl_line ();
	sprintf (err_str, ML (mlCrMess024), DOLLARS (totalPayment));
	so_pr (err_str, 1, 19, 1);

	fflush (stdout);
}

/*
 * Update Files.   
 */
int
Update (void)
{
	int	no_sel = TRUE;
	char	cmd [100];

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	/*
   	 * Add revised selection status for suin records.      
	 */
	abc_selfield (suin,"suin_id_no2");

	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		suin_rec.hhsu_hash =  sumr_rec.hhsu_hash;
		strcpy (suin_rec.inv_no, local_rec.inv_no);
		cc = find_rec (suin, &suin_rec, COMPARISON, "u");	
		if (cc)
			file_err (cc, suin, "DBFIND");

		if (local_rec.pay [0] == 'Y')
		{
			strcpy (suin_rec.stat_flag, "S");
			no_sel = FALSE;
		}
		else
			strcpy (suin_rec.stat_flag, "0");

		cc = abc_update (suin,&suin_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");
	}
	abc_selfield (suin,"suin_cron");

	strcpy (local_rec.p_crd_no,sumr_rec.crd_no);
	strcpy (sumr_rec.stat_flag, (no_sel) ? "0" : "S");

	cc = abc_update (sumr,&sumr_rec);
	if (cc)
		file_err (cc, sumr, "DBUPDATE");

	if (sumr_rec.stat_flag [0] == 'S')
	{
		/*
		 * Add printing of payment voucher. 
		 */
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

			sprintf (cmd, "cr_payprt %d %10ld %10ld", 
											local_rec.printerNo, 
											sumr_rec.hhsu_hash,
											esmr_rec.nx_voucher_no);
			sys_exec (cmd);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Clear screen where box was
 */
void
ClearBox (
 int x,
 int y,
 int h,
 int v)
{
	int	i,
		j;

	j = v;		
	i = y;
		
	while (j--)
	{
		if (h > 1)
			print_at (i, x, "%*.*s", h, h, " ");
		i++;
	}
}

/*
 * Screen Heading. 
 */
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	if (clearOk)
		clear ();

	if (scn == 2 && !clearOk)
	{
		ClearBox (0,2,13,7);
		scn_set (1);

		/* Need to set line_cnt to 0 because we changed screen */
		line_cnt = 0;
		scn_write (1);
		scn_display (1);
	}
	if (scn == 1 && !clearOk)
	{
		ClearBox (0,2,13,7);
		scn_set (2);

		/* Need to set line_cnt to 0 because we changed screen */
		line_cnt = 0;
		scn_write (2);
		scn_display (2);
	}
	scn_set (scn);

	rv_pr (ML (mlCrMess026),30,0,1);

	print_at (0,85,ML (mlCrMess025),local_rec.p_crd_no);
	fflush (stdout);
	line_at (1,0,130);

	box (0,2,130,2);

	line_at (20,1,130);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}

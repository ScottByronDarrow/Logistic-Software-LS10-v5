/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_trandsp.c,v 5.5 2002/07/25 11:17:27 scott Exp $
|  Program Name  : (cr_trandsp.c)
|  Program Desc  : (Display Supplier Transactions) 
|---------------------------------------------------------------------|
|  Author        : Anneliese Allen.| Date Written : 22/02/93          |
|---------------------------------------------------------------------|
| $Log: cr_trandsp.c,v $
| Revision 5.5  2002/07/25 11:17:27  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.4  2002/06/25 03:35:04  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_trandsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_trandsp/cr_trandsp.c,v 5.5 2002/07/25 11:17:27 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>

#include 	"schema"

#define 	SEL_PSIZE 	13
#define		HASH		8000

#define	MCURR		 (multiCurrency [0] == 'Y')

	char	branchNumber [3],
	    	multiCurrency [2];

	long	hhsuHash [HASH];

	struct	commRecord	comm_rec;
	struct	suinRecord	suin_rec;
	struct	suhdRecord	suhd_rec;
	struct	sudtRecord	sudt_rec;
	struct	sumrRecord	sumr_rec;

	char	*data = "data";

/*
 * The structures 'cheq'&'dtls' are initialised in function 'get_cheq'
 * the number of cheques is stored in external variable 'cheq_cnt'.  
 * the number of details is stored in external variable 'dtls_cnt'. 
 */
struct Cheque
{						/*-----------------------------------*/
	char	no[16];		/*| cheque receipt number.	    	|*/
	char	datec[11];	/*| date of payment.      	    	|*/
	double	amount;		/*| total amount of cheque.	    	|*/
	double	cdisc;		/*| discount given.		    		|*/
	double	loc_amt;	/*| total chq amt (local currency)  |*/
	double	loc_disc;	/*| total disc amt (local currency) |*/
	double	ex_var;		/*| exchange variance (local curr)  |*/
}	*cheq;				/*----------------------------------*/
	DArray	cheq_d;	
	int		chequeCount;

struct Detail
{							/*----------------------------------*/
	long	hhsiHash;		/*| detail invoice hash.            |*/
	double	inv_amt;		/*| detail invoice amount.          |*/
	double	inv_loc_amt;	/*| detail invoice amt (local curr) |*/
	double	inv_ex_var;		/*| exchange variance (local curr)  |*/
	double	inv_exch_rate;	/*| exchange rate                   |*/
	int	cheq_hash;			/*| cheq structure pointer.         |*/
}	*dtls;					/*----------------------------------*/
	DArray	dtls_d;			
	int		detailCount;

	int		hashCounter 	= 0,
			envCrCo 		= 0,
			envCrFind 		= 0,
			lineNumber 		= 0,
			supplierInput 	= TRUE;
	
	FILE *	pp;

/*
 * Local Screen Structure 
 */
struct {
	char	dummy [11];
	char	supplierNo [7];
	char	inv_no [16];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNo",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Supplier Number :", " ",
		NO, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "crd_name",	 4, 50, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, " ", " ", sumr_rec.crd_name},
	{1, LIN, "tranref",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Invoice Ref     :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inv_no},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", " ", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindSumr.h>
/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	shutdown_prog	 (void);
int		spec_valid		 (int);
int		heading			 (int);
void	DisplayInvoice	 (void);
void	ProcessInvoices	 (void);
void	GetCheques		 (int);
int		FindSuin		 (long);
void	PrintCompDetail	 (void);
int		SrchSuin		 (char *);


int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	envCrCo = atoi (get_env ("CR_CO"));
	envCrFind  = atoi (get_env ("CR_FIND"));

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	init_scr ();
	clear ();
	set_tty ();
	set_masks ();
	swide (); 

	/*
	 *	Allocate initial lines for cheques and details
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 1000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	OpenDB ();
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0)
	{
		/*----------------------
		| Reset Control Flags. |
		----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;
		hashCounter	= 0;
		init_vars (1);

		/*----------------------
		| Entry Screen Input   |
		----------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (restart)
			continue;
    }  /*	End of Main loop (exit from 'input ()')	*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");

	strcpy (sumr_rec.co_no, comm_rec.co_no);
}

void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	ArrDelete (&dtls_d);
	ArrDelete (&cheq_d);
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
	int		field)
{
	int	i;

	if (LCHECK ("supplierNo"))
	{
		if (dflt_used)
		{
			strcpy (sumr_rec.crd_no,    "ALL   ");
			strcpy (sumr_rec.crd_name,  ML ("All Suppliers"));
			DSP_FLD ("crd_name");
			supplierInput = FALSE;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
	
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (sumr_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str,ML (mlStdMess022), sumr_rec.crd_no);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("crd_name");
		if (strcmp (sumr_rec.crd_no, "ALL   "))
			supplierInput = TRUE;
		else
		{
			hhsuHash [0] = suin_rec.hhsu_hash;
			hashCounter = 1;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("tranref"))
	{
		if (last_char == FN16)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchSuin (temp_str);
			return (EXIT_SUCCESS);
		}
		if (supplierInput)
			suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		else
		{
			abc_selfield (sumr, "sumr_hhsu_hash");
			suin_rec.hhsu_hash = 0L;
		}
		strcpy (suin_rec.est, comm_rec.est_no);
		strcpy (suin_rec.inv_no, local_rec.inv_no);
		cc = find_rec (suin, &suin_rec, GTEQ, "r");
		while (!cc)
		{
			if (!supplierInput &&
			    !strcmp (suin_rec.est, comm_rec.est_no) &&
			    !strcmp (suin_rec.inv_no, local_rec.inv_no))
			{
				cc = find_hash (sumr, &sumr_rec, EQUAL, "r", suin_rec.hhsu_hash);
				if (cc)
					file_err (cc, sumr, "DBFIND");

				if (!strcmp (sumr_rec.co_no, comm_rec.co_no))
				{
					hhsuHash [hashCounter] = suin_rec.hhsu_hash;
					hashCounter++;
				}
			}
			if (supplierInput &&
			    suin_rec.hhsu_hash ==sumr_rec.hhsu_hash &&
			    !strcmp (suin_rec.est, comm_rec.est_no) &&
			    !strcmp (suin_rec.inv_no, local_rec.inv_no))
			{
				hhsuHash [hashCounter] = suin_rec.hhsu_hash;
				hashCounter = 1;
				break;
			}
			cc = find_rec (suin, &suin_rec, NEXT, "r");
		}
		if (cc && hashCounter == 0)
		{
			sprintf (err_str,ML (mlStdMess016),local_rec.inv_no);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Initialise cheque structure 
		 */
		if (!supplierInput)
			abc_selfield (sumr, "sumr_id_no");
		for (i = 0; i < hashCounter; i++)
			GetCheques (i);
		DisplayInvoice ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_FAILURE);

	clear ();

	rv_pr (ML (mlCrMess072),51, 0, 1);

	line_at (1,0,130);
	box (0,3,130,2);

	PrintCompDetail ();
	scn_write (scn);

	return (EXIT_SUCCESS);
}
		
/*
 * Display Invoices Routine
 */
void
DisplayInvoice (void)
{
	int	i = 0;
	lineNumber = 0;

	/*
	 * Store Heading Details.            
	 */
	print_at (2, 1, "Supplier : %-6.6s - %s", 
					  sumr_rec.crd_no, 
					  sumr_rec.crd_name);
	
	Dsp_open ((MCURR) ? 0 : 10,3, SEL_PSIZE);
	Dsp_saverec ("    INVOICE    |A| DATE OF  |   DATE   |  INVOICE   |  BALANCE   |    PAY     |EXCHANGE |F|  BALANCE  |     CHEQUE    |   CHEQUE  ");
	Dsp_saverec ("    NUMBER     |P| INVOICE  | INV. DUE |  AMOUNT    |    DUE     |   AMOUNT   |   RATE  |X|   LOCAL   |     NUMBER    |   AMOUNT  ");
	
	Dsp_saverec (" [NEXT SCN] [PREV SCN] [INPUT/END] ");

	/*
	 * Store Line Details.               
	 */
	for (i = 0; i < hashCounter; i++)
	{
		suin_rec.hhsu_hash = hhsuHash [i];
		strcpy (suin_rec.est, "  ");
		strcpy (suin_rec.inv_no, "               ");
		cc = find_rec (suin, &suin_rec, GTEQ, "r");
		while (!cc && hhsuHash [i] == suin_rec.hhsu_hash)
		{
			if (!strcmp (suin_rec.inv_no, local_rec.inv_no))
				ProcessInvoices ();
	
			cc = find_rec (suin, &suin_rec, NEXT, "r");
		}
	}

	/*-----------------------------------
	| Display Screen.                   |
	-----------------------------------*/
	Dsp_saverec ("^^GGGGGGGGGGGGGGGJGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGJGJGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGG");
	Dsp_srch ();
	Dsp_close ();
}

void
ProcessInvoices (
 void)
{
	int 	i,
			none;

	char	date1 [11], 
			date2 [11],
			inv_hdr1 [200],
			inv_hdr2 [200],
			env_line [200];

	double	balance = 0.0,
	      	loc_bal = 0.0;

	balance = suin_rec.amt - suin_rec.amt_paid;
	
	strcpy (date1, DateToString (suin_rec.date_of_inv));
	strcpy (date2, DateToString (suin_rec.pay_date));

	/*-----------------------------------
	| Setup Invoice Header Detail.      |
	-----------------------------------*/
	if (suin_rec.exch_rate == 0.00)
		suin_rec.exch_rate = 1.0000;

	loc_bal = balance / suin_rec.exch_rate;

	sprintf (inv_hdr1,"%s^E%s^E%10.10s^E%10.10s^E%12.2f^E%12.2f^E%12.2f^E%9.4f^E%s^E%11.2f^E", 
			suin_rec.inv_no, 
			suin_rec.approved,
			date1, 
			date2, 
			DOLLARS (suin_rec.amt),
			DOLLARS (balance),
			DOLLARS (suin_rec.pay_amt),
			suin_rec.exch_rate,
			suin_rec.er_fixed,
			DOLLARS (loc_bal));

		sprintf (inv_hdr2,"%s^E%s^E%s^E%s^E%s^E%s^E%s^E%s^E%s^E%s^E", 
					"               ",
					" ",
					"          ",
					"          ",
					"            ",
					"            ",
					"            ",
					"         ",
					" ",
					"           ");

	/*-----------------------------------
	| Process Cheque Details.           |
	-----------------------------------*/
	none = 0;
	for (i = 0; i < detailCount; i++)
	{
		if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
		{
			if (lineNumber >= SEL_PSIZE)
				lineNumber = 0;

			/*--------------------------------
			| Print Invoice Line Details.    |
			--------------------------------*/
			if (none == 0 || lineNumber == 0)
			{
				sprintf (env_line,"%-s%s^E%11.2f",
						inv_hdr1,
						cheq [dtls [i].cheq_hash].no,
						DOLLARS (dtls [i].inv_amt));
			}
			else
			{
				sprintf (env_line,"%-s%s^E%11.2f",
						inv_hdr2,
						cheq [dtls [i].cheq_hash].no,
						DOLLARS (dtls [i].inv_amt));
			}
			Dsp_saverec (env_line);
			lineNumber++;
			none = 1;
		}
	}
	if (none == 0)
	{
		sprintf (env_line,"%s               ^E           ", inv_hdr1);
		Dsp_saverec (env_line);
		lineNumber++;
	}
}

/*
 * Read cheque routine for creditor.        
 */
void
GetCheques (
	int		i)
{
	chequeCount = 0;
	detailCount = 0;

	suhd_rec.hhsu_hash	=	hhsuHash [i];
	for (cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
		!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash;
		cc = find_rec (suhd, &suhd_rec, NEXT, "r"))
	{
		if (!ArrChkLimit (&cheq_d, cheq, chequeCount))
			sys_err ("ArrChkLimit(cheques)", ENOMEM, PNAME);

		strcpy (cheq [chequeCount].no,	  suhd_rec.cheq_no);
		strcpy (cheq [chequeCount].datec, DateToString (suhd_rec.date_payment));

		cheq [chequeCount].amount		= suhd_rec.tot_amt_paid;
		cheq [chequeCount].cdisc		= suhd_rec.disc_taken;
		cheq [chequeCount].loc_amt		= suhd_rec.loc_amt_paid;
		cheq [chequeCount].loc_disc		= suhd_rec.loc_disc_take;
		cheq [chequeCount].ex_var		= suhd_rec.exch_variance;

		/*
		 *	Read in associated details
		 */
		sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
		for (cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
			!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash;
			cc = find_rec (sudt, &sudt_rec, NEXT, "r"))
		{
			if (!ArrChkLimit (&dtls_d, dtls, detailCount))
				sys_err ("ArrChkLimit(dtls)", ENOMEM, PNAME);

			dtls [detailCount].hhsiHash		= sudt_rec.hhsi_hash;
			dtls [detailCount].inv_amt			= sudt_rec.amt_paid_inv;
			dtls [detailCount].inv_loc_amt		= sudt_rec.loc_paid_inv;
			dtls [detailCount].inv_ex_var		= sudt_rec.exch_variatio;
			dtls [detailCount].inv_exch_rate	= sudt_rec.exch_rate;
			dtls [detailCount].cheq_hash		= chequeCount;
			++detailCount;
		}
		++chequeCount;
	}
}

/*=============================
| Find suin from detail hash. |
=============================*/
int
FindSuin (
	long		hhsiHash)
{
	int	suin_err;

	abc_selfield (suin, "suin_hhsi_hash");

	suin_rec.hhsi_hash	=	hhsiHash;
	suin_err = find_rec (suin, &suin_rec, COMPARISON, "r");

	abc_selfield (suin, "suin_cron");

	return (suin_err);
}

void
PrintCompDetail (void)
{
	line_at (21,0,132);
	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 0, err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str, ML (mlStdMess039));
	print_at (22, 57, err_str, comm_rec.est_no, comm_rec.est_short);
	line_at (23,0,132);
}

int
SrchSuin (
	char	*keyValue)
{
	_work_open (22,0,40);
	save_rec ("#Cust No/Invoice No", "#                  Name                  ");
	if (!supplierInput)
		sumr_rec.hhsu_hash = 0L;

	abc_selfield (sumr, "sumr_hhsu_hash");
	
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est, comm_rec.est_no);
	/*
	 * Initialise this to blank b/c otherwise if user chooses
	 * value via search then backspaces out entry then      
 	 * presses F4 they will only get from last choice on.  
	 */
	strcpy (suin_rec.inv_no, "              ");
	strncpy (suin_rec.inv_no, keyValue, strlen (keyValue));
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc)
	{
		if (strncmp (suin_rec.inv_no, keyValue, strlen (keyValue)))
		{
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		if (supplierInput &&
		    suin_rec.hhsu_hash != sumr_rec.hhsu_hash)
			break;

		sumr_rec.hhsu_hash	= suin_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (!strcmp (sumr_rec.co_no, comm_rec.co_no))
		{
			sprintf (err_str, "%6.6s/%15.15s", sumr_rec.crd_no,suin_rec.inv_no);
			cc = save_rec (err_str, sumr_rec.crd_name);
		}
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}
	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	sprintf (local_rec.supplierNo, "%-6.6s", temp_str);
	sprintf (local_rec.inv_no, "%-15.15s", temp_str + 7);

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNo));
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	 	file_err (cc, sumr, "DBFIND");

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est, comm_rec.est_no);
	strcpy (suin_rec.inv_no, local_rec.inv_no);
	cc = find_rec (suin, &suin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, suin, "DBFIND");

	sprintf (temp_str, "%-15.15s", temp_str + 7);
	return (EXIT_SUCCESS);
}

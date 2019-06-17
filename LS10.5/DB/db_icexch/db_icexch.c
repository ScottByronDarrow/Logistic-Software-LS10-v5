/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: db_icexch.c,v 5.8 2002/11/20 05:22:04 kaarlo Exp $
|  Program Name  : (db_icexch.c)                                
|  Program Desc  : (Update Exchange Rate For Invoices & Credit Notes)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 03/08/92         |
|---------------------------------------------------------------------|
| $Log: db_icexch.c,v $
| Revision 5.8  2002/11/20 05:22:04  kaarlo
| LS 01070 SC 4127. Bug Fixing.
|
| Revision 5.7  2002/07/23 07:08:38  scott
| Updated to remove old sort routines.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_icexch.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_icexch/db_icexch.c,v 5.8 2002/11/20 05:22:04 kaarlo Exp $";

#define MAXWIDTH	150
#define MAXLINES	2000
#define PAGELINES	65
#define	MOD  		1	/* alters frequency for dsp_process */

#include <pslscr.h>
#include <arralloc.h>
#include <GlUtils.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>
#include <errno.h>
#include <arralloc.h>

#define	BY_INVOICE	 (select_type [0] == 'I')
#define	BY_CURRENCY	 (select_type [0] == 'C')
#define	BY_ALL		 (select_type [0] == 'A')

#define	EXCH_OK		 (cuin_rec.er_fixed [0] != 'Y')

#define	DATE_OK		 (cuin_rec.date_of_inv >= local_rec.from_date && \
	                  cuin_rec.date_of_inv <= local_rec.to_date)

#define	CURR_OK 	 (!strcmp (cuin_rec.currency,pocrRec.code))

#define	INVOICE		 (cuin_rec.type [0] == '1')
#define	CREDIT 		 (cuin_rec.type [0] == '2')

/*
 * The structure dtls' is initialised in function 'GetCheque'           
 * the number of details is stored in external variable 'dtls_cnt'.    
 */
struct	Detail {   		/*-----------------------------------*/
	long	hhci_hash;	/*| detail invoice reference.       |*/
	double	inv_oamt;	/*| Invoice overseas amount.        |*/
	double	inv_lamt;	/*| Invoice local amount.           |*/
	double	exch_var;	/*| Exchange variation.             |*/
	double	exch_rate;	/*| Exchange rate.                  |*/
} *dtls;         		/*-----------------------------------*/

int		SortFunc			(const	void *,	const void *);

	/*
	 * Special fields and flags
	 */

	DArray	dtls_d;

	int		dtls_cnt			= 0,
   			processID			= 0,	
			glwk_no				= 0,
			found_data 			= 0,
			gl_trans 			= 0,
			printerNumber 		= 1,
			envVarDbCo 			= 0,
			envVarDbFind 		= 0,
			envVarDbNettUsed	= TRUE,
			envVarDbMcurr 		= FALSE;

	char 	data_str [31],
	    	mult_curr [2],
	    	branchNumber [3],
	    	select_type [2];

	char	*sptr;

   	double 	er_variance = 0.00,
   	       	exch_variance = 0.00,
			exch_var_tot = 0.00,
			old_doc_bal = 0.00,
			new_doc_bal = 0.00;

	FILE	*fin,
			*fout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [MAXLEVEL + 1];
	double	exchVar;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cuinRecord	cuin_rec;
struct wfbcRecord	wfbc_rec;

	struct {
		long	wk_hash;
	} wk_rec;

	int		envVarGlByClass = TRUE;
	double	invoiceBal;

	char	*data = "data";

	char	loc_curr [4];

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char 	dbt_no [7];
	double	inv_balance;
	double	old_rate;
	double	new_rate;
	long	from_date;
	long	to_date;
	char 	systemDate [11];
	char 	acc_no [MAXLEVEL + 1];
	char 	inv_no [9];
	char 	gl_user_ref [21];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customer",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "A", "Select Customer", "Enter Customer No., [SEARCH], or <retn> for ALL",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{1, LIN, "name",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "inv_no",	 6, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Select Document", "Enter Invoice No., [SEARCH], or <retn> for ALL",
		 NE, NO,  JUSTLEFT, "", "", local_rec.inv_no},
	{1, LIN, "old_rate",	 7, 20, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Old Exchange Rate", " ",
		 NA, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.old_rate},
	{1, LIN, "from_date",	 9, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Document From Date", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.from_date},
	{1, LIN, "to_date",	10, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Document  To  Date", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.to_date},
	{1, LIN, "curr",	12, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", " ",
		YES, NO,  JUSTLEFT, "", "", pocrRec.code},
	{1, LIN, "curr_desc",	12, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "new_rate",	13, 20, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "1", "New Exchange Rate", " ",
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.new_rate},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
/*
 * Local Function Prototypes.
 */
int 	ReadGlmrRecord 		 (char *);
int 	heading 			 (int);
int 	spec_valid 			 (int);
void 	CloseDB 			 (void);
void 	GetCheque 			 (long);
void 	Heading1Output 		 (void);
void 	Heading2Output 		 (void);
void 	InvoiceBalance 		 (void);
void 	OpenDB 				 (void);
void 	PrintControlAccount	 (char *, double, char *);
void 	PostInvoice 		 (double);
void 	ProcessInvoice 		 (void);
void 	ProcessSortedList 	 (void);
void 	SrchCuin 			 (char *);
void 	SrchPocr 			 (char *);
void 	StoreAccount 		 (void);
void 	UpdateWfbc 			 (void);
void 	WriteGlTrans 		 (char *, double, char *);
void 	shutdown_prog 		 (void);

/*
 * Main Processing Routine. 
 */
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	if (argc != 3)
	{
		/*
		 * Usage %s <LPNO> <PID>\n 
		 */
		print_at (0,0, mlDbMess002, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNumber  = atoi (argv [1]);
	processID   = atoi (argv [2]);

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);
	
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("GL_BYCLASS");
	envVarGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	init_scr ();

	if (!envVarDbMcurr) 
	{
		no_option ("DB_MCURR (Multi Currency Customers.)");
        return (EXIT_FAILURE);
	}

	/*
	 * Setup required parameters. 
	 */
	SETUP_SCR (vars);

	set_tty ();
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind  = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	/*
	 *	Allocate initial space for Details
	 */
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	while (prog_exit == 0) 
	{
		abc_unlock (cumr);
		abc_unlock (cuin);
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Reset default screen control.
		 */
		FLD ("inv_no") 	 	= YES;
		FLD ("from_date") 	= YES;
		FLD ("to_date")   	= YES;
		FLD ("curr")      	= YES;

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		/*
		 * Update invoice & glwk records. 
		 */
		ProcessInvoice ();
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);
	abc_fclose (comr);

	open_rec (cumr,cumr_list,CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no" 
							    : "cumr_id_no3");

	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");

	OpenPocr ();
	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (cudt);
	abc_fclose (wfbc);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	ArrDelete (&dtls_d);

	abc_dbclose (data);
}

/*
 * Read gl account master.    
 */
int
ReadGlmrRecord (
 char*              account)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P') 
	      return (2);
	
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	/*
	 * Validate Customer Number Input. 
	 */
	if (LCHECK ("customer"))
	{
		if (dflt_used || !strcmp (local_rec.dbt_no,"ALL   "))
		{
			strcpy (local_rec.dbt_no,"ALL   ");
			sprintf (cumr_rec.dbt_name, "%-40.40s", " ");
			strcpy (local_rec.inv_no,"ALL     ");

			DSP_FLD ("customer");
			DSP_FLD ("name");
			DSP_FLD ("inv_no");

			strcpy (select_type, "A");

			FLD ("inv_no")    = NA;
			FLD ("from_date") = YES;
			FLD ("to_date")   = YES;
			FLD ("curr")      = YES;
			FLD ("new_rate")  = YES;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Customer %s is not on file. 
			 */
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		/*
		 * Read Currency Record. 
		 */
		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code,cumr_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Customer currency %s is not on file. 
			 */
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		FLD ("inv_no")    = YES;
		FLD ("from_date") = YES;
		FLD ("to_date")   = YES;
		FLD ("curr")      = YES;
		FLD ("new_rate")  = YES;

		DSP_FLD ("name");
		DSP_FLD ("curr");
		DSP_FLD ("curr_desc");
		strcpy (select_type,"C");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Invoice Number Input. 
	 */
	if (LCHECK ("inv_no"))
	{
		if (FLD ("inv_no") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCuin (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.inv_no,"ALL     "))
		{
			strcpy (local_rec.inv_no,"ALL     ");
			FLD ("inv_no")	   = YES;
			FLD ("from_date") = YES;
			FLD ("to_date")  = YES;
			FLD ("curr")      = YES;
			FLD ("new_rate")  = YES;
			DSP_FLD ("inv_no");
			return (EXIT_SUCCESS);
		}

		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.inv_no,local_rec.inv_no);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "u");
		if (cc)
		{
			/*
			 * Invoice %s is not on file for customer. 
			 */
			errmess (ML (mlDbMess053));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		local_rec.old_rate = cuin_rec.exch_rate;
		strcpy (select_type,"I");

		strcpy (pocrRec.co_no,comm_rec.co_no);
		strcpy (pocrRec.code, cuin_rec.currency);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (!cc) 
		{
			DSP_FLD ("curr");
			DSP_FLD ("curr_desc");
		}

		FLD ("inv_no")    = YES;
		FLD ("from_date") = NA;
		FLD ("to_date")   = NA;
		FLD ("curr")      = NA;
		FLD ("new_rate")  = YES;

		DSP_FLD ("inv_no");
		DSP_FLD ("old_rate");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Inv/Crd From Date. 
	 */
	if (LCHECK ("from_date"))
	{
		if (dflt_used)
		{
			local_rec.from_date	=	TodaysDate ();
			local_rec.from_date	=	MonthStart (local_rec.from_date);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Inv/Crd To Date.   
	 */
	if (LCHECK ("to_date"))
	{
		if (dflt_used)
		{
			local_rec.to_date	=	TodaysDate ();
			local_rec.to_date	=	MonthEnd (local_rec.to_date);
		}

		if (local_rec.from_date > local_rec.to_date)
		{ 
			/*
			 * To Date cannot precede From Date 
			 */
			errmess (ML (mlDbMess054));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Currency Code.       
	 */
	if (LCHECK ("curr"))
	{
		if (dflt_used && BY_CURRENCY)
			strcpy (pocrRec.code, cumr_rec.curr_code);

		if (dflt_used && BY_INVOICE)
			strcpy (pocrRec.code, cuin_rec.currency);

		if (SRCH_KEY)
		{
			   SrchPocr (temp_str);
		       return (EXIT_SUCCESS);
		}
		/*
		 * Read Currency Record. 
		 */
		strcpy (pocrRec.co_no,comm_rec.co_no);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Currency not found. 
			 */
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (BY_INVOICE && 
		     strcmp (pocrRec.code, cuin_rec.currency))
		{
			/*
			 * Currency %s is not the same as Invoice currency %s .
			 */
			sprintf (err_str, ML (mlDbMess055), 
							 pocrRec.code, cuin_rec.currency);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Exchange Rate.       
	 */
	if (LCHECK ("new_rate"))
	{
		if (dflt_used)
			local_rec.new_rate = pocrRec.ex1_factor;

		DSP_FLD ("new_rate");
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}

/*
 * Search for currency pocr code 
 */
void
SrchPocr (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No ","#Currency description");
	strcpy (pocrRec.co_no,comm_rec.co_no);
	sprintf (pocrRec.code ,"%-3.3s",keyValue);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
        while (!cc && !strcmp (pocrRec.co_no,comm_rec.co_no) && 
		      !strncmp (pocrRec.code,keyValue,strlen (keyValue)))
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

/*
 * Search routine for supplier invoice file.     
 */
void
SrchCuin (
	char	*keyValue)
{
	char	disp_amt [51];

	_work_open (8,0,40);
	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.inv_no,keyValue);
	save_rec ("#Invoice ","#Tran Type |          Amount.");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && !strncmp (cuin_rec.inv_no, keyValue,strlen (keyValue)) && 
			cuin_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if (INVOICE || CREDIT)
		{
			local_rec.inv_balance = (envVarDbNettUsed) 
					? cuin_rec.amt - cuin_rec.disc 
					: cuin_rec.amt;

			sprintf (disp_amt, " %s  | $%14.2f ",
					 (INVOICE) ? "Invoice" : "C/Note.",
					DOLLARS (local_rec.inv_balance));

			cc = save_rec (cuin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.inv_no,temp_str);
	cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cuin", "DBFIND");
}
/*
 * Update Invoice & GL Work File. 
 */
void
ProcessInvoice (void)
{
	dsp_screen ("Processing Invoices", comm_rec.co_no, comm_rec.co_name);

	/*
	 * Start audit report & open sort file.   
	 */
	gl_trans = 0;
	Heading1Output ();

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	/*
	 * Update selection on inv/cn document.   
	 */
	if (BY_INVOICE)
	{
		/*
		 * Get payment details. 
		 */
		GetCheque (cumr_rec.hhcu_hash);

		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.inv_no,local_rec.inv_no);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "cuin", "DBFIND");

		InvoiceBalance ();
			
		if (EXCH_OK && CURR_OK && invoiceBal != 0.00)
			PostInvoice (invoiceBal);
	}

	/*
	 * Update selection on creditor.          
	 */
	if (BY_CURRENCY)
	{
		GetCheque (cumr_rec.hhcu_hash);
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.inv_no,"        ");
		cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
		while (!cc && (cuin_rec.hhcu_hash == cumr_rec.hhcu_hash))
		{
			InvoiceBalance ();

			if (EXCH_OK && CURR_OK && invoiceBal != 0.00)
				PostInvoice (invoiceBal);

			abc_unlock (cuin);
			cc = find_rec (cuin, &cuin_rec, NEXT, "u");
		}
	}

	/*
	 * Update selection on all creditors.     
	 */
	if (BY_ALL)
	{
		abc_selfield ("cumr","cumr_hhcu_hash");

		cuin_rec.hhcu_hash = 0;
		strcpy (cuin_rec.inv_no,"        ");
		cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
		while (!cc)
		{
			cumr_rec.hhcu_hash = cuin_rec.hhcu_hash;
			if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
			{
				abc_unlock (cuin);
				cc = find_rec (cuin, &cuin_rec, NEXT, "u");
				continue;
			}
			GetCheque (cumr_rec.hhcu_hash);

			InvoiceBalance ();

			if (DATE_OK && EXCH_OK && CURR_OK && invoiceBal != 0.00)
				PostInvoice (invoiceBal);
			else
				abc_unlock (cuin);

			cc = find_rec (cuin, &cuin_rec, NEXT, "u");
		}
		abc_unlock (cuin);
	}
	if (gl_trans != 0)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);

		ProcessSortedList ();
		UpdateWfbc ();
	}
}

/*
 * Process Invoice / Cn Document.      
 */
void
PostInvoice (
	double		Balance)
{
	double	dbcr_value = 0.00;

	dsp_process ("Invoice", cuin_rec.inv_no);

	if (local_rec.new_rate == 0.00 || cuin_rec.exch_rate == 0.00)
		er_variance = 0.00;	
	else
		er_variance = (1.0 / local_rec.new_rate) - (1.0 / cuin_rec.exch_rate);

	local_rec.old_rate 	= cuin_rec.exch_rate;
	new_doc_bal 		= Balance;

	exch_variance = er_variance * new_doc_bal;
	exch_variance = twodec (exch_variance);

	if (exch_variance == 0.0)
	{
		abc_unlock (cuin);
		return;
	}

	/*
	 * Update Inv/Cn      
	 */
	cuin_rec.exch_rate = local_rec.new_rate;
	cc = abc_update (cuin, &cuin_rec);
	if (cc)
		file_err (cc, "cuin", "DBUPDATE");

	/*
	 * Store In Sort File 
	 */
	StoreAccount ();

	fprintf (fout, "| %s | %s|%s| %14.14s ",
		cumr_rec.dbt_no,
		cumr_rec.dbt_name,
		cuin_rec.inv_no,
		comma_fmt (DOLLARS (new_doc_bal), "NNN,NNN,NNN.NN"));

	fprintf (fout, "|%13.8f|%13.8f|", local_rec.old_rate, local_rec.new_rate);

	fprintf (fout, "%14.14s ", comma_fmt (DOLLARS (exch_variance), "NNN,NNN,NNN.NN"));

	/*
	 * Credit Value       
	 */
	if (exch_variance < 0)
	{
		dbcr_value = exch_variance * -1;
		dbcr_value = twodec (dbcr_value);
		fprintf (fout, "|               |%14.14s |\n",
			comma_fmt (DOLLARS (dbcr_value), "NNN,NNN,NNN.NN"));
	}
	/*
	 * Debit Value        
	 */
	else
	{
		fprintf (fout, "|%14.14s |               |\n",
			comma_fmt (DOLLARS (exch_variance), "NNN,NNN,NNN.NN"));
	}
}

/*
 * Store Account Detail In Sort File. 
 */
void
StoreAccount (void)
{
	char	account_no [MAXLEVEL + 1];

	sprintf (account_no,"%-*.*s", MAXLEVEL, MAXLEVEL,cumr_rec.gl_ctrl_acct);
	if (ReadGlmrRecord (account_no))
	{
		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no,
			"  ",
			"ACCT REC  ",
			(envVarGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
			" "
		);
	}
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	/*
	 * Load values into array element sortCnt.
	 */
	strcpy (sortRec [sortCnt].sortCode, glmrRec.acc_no);
	sortRec [sortCnt].exchVar = DOLLARS (exch_variance);
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

/*
 * Process Account Detail From Sort File. 
 */
void
ProcessSortedList (void)
{
	char	previousAccount [MAXLEVEL + 1];
	char	currentAccount [MAXLEVEL + 1];
	double	currentValue = 0.00;
	double	acct_tot = 0.00;
	int		first_record = TRUE,
			i;

	Heading2Output ();

	exch_var_tot = 0.0;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		strcpy (currentAccount, sortRec [i].sortCode);
		currentValue = sortRec [i].exchVar;

		if (first_record == TRUE)
		{
			first_record = FALSE;
			strcpy (previousAccount, sortRec [i].sortCode);
			sprintf (temp_str, "Processing Account Summary: %s",currentAccount);
			us_pr (temp_str,20,18,1);
		}

		if (strcmp (previousAccount, currentAccount))
		{
			PrintControlAccount (previousAccount,acct_tot,"Ctrl Acct");
			WriteGlTrans (previousAccount,acct_tot,"Ctrl Acct");
			exch_var_tot -= acct_tot;
			acct_tot = currentValue;
			strcpy (previousAccount,currentAccount);

			sprintf (temp_str,"Processing Account Summary: %s", currentAccount);
			us_pr (temp_str,20,18,1);
		}
		else
			acct_tot += currentValue;
	}
	PrintControlAccount (previousAccount, acct_tot, " Control Account.   ");

	WriteGlTrans (previousAccount, acct_tot, " Control Account.   ");

	exch_var_tot -= acct_tot;

	PrintControlAccount (pocrRec.gl_exch_var,exch_var_tot," Exchange variation ");

	WriteGlTrans (pocrRec.gl_exch_var,exch_var_tot," Exchange variation ");

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*
 * Print Summarised Control Account Detail. 
 */
void
PrintControlAccount (
 char*              acct,
 double             value,
 char*              type)
{
	double	dbcr_value;

	if (!value)
		return;

	fprintf (fout, "| %20.20s |%-16.16s", type,acct);
				
	/*
	 * Debit Value      
	 */
	if (value > 0)
	{
		fprintf (fout, "|%14.14s |               |\n",
			comma_fmt (value, "NNN,NNN,NNN.NN"));
	}
	/*
	 * Credit Value        
	 */
	else
	{
		dbcr_value = value * -1;
		dbcr_value = twodec (dbcr_value);
		fprintf (fout, "|               |%14.14s |\n",
			comma_fmt (dbcr_value, "NNN,NNN,NNN.NN"));
	}
}

/*
 * Write glwk total records. 
 */
void	
WriteGlTrans (
 char*              acct,
 double             value,
 char*              type)
{
	int		monthPeriod;

	if (value == 0.00)
		return;

	/*
	 * Post control total debits & credits .           
	 */
	strcpy (glwkRec.jnl_type, (value > 0.00) ? "1" : "2");
	glwkRec.amount = (value > 0.00) 
					? CENTS (value) : CENTS (value * -1);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;

	strcpy (glwkRec.currency, loc_curr);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,acct);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	/*
	 * Add transaction for account .             
	 */
	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, acct);
	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.est_no,comm_rec.est_no);
	strcpy (glwkRec.acronym,"         ");
	strcpy (glwkRec.name,"                              ");
	sprintf (glwkRec.chq_inv_no, "%-8.8s", " ");
	glwkRec.ci_amt = 0;
	glwkRec.o1_amt = local_rec.old_rate;
	glwkRec.o2_amt = local_rec.new_rate;
	glwkRec.o3_amt = 0;
	glwkRec.o4_amt = 0;
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.tran_type,"15");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.tran_date = comm_rec.dbt_date;
	
	DateToDMY (glwkRec.tran_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	glwkRec.post_date = StringToDate (local_rec.systemDate);
	strcpy (glwkRec.narrative,"Inv/cn exch rate adj");
	strcpy (glwkRec.user_ref,"exch-adj");
	strcpy (glwkRec.stat_flag,"2");

	GL_AddBatch ();
}

/*
 * Update GLBC Batch Control Records. 
 */
void
UpdateWfbc (void)
{

	/*
	 * Add general ledger batch control record.    
	 */
	strcpy (wfbc_rec.co_no, comm_rec.co_no);
	wfbc_rec.pid_no = processID;
	sprintf (wfbc_rec.work_file,"gl_work%05d",processID);
	cc = find_rec (wfbc, &wfbc_rec, COMPARISON , "u");
    	if (cc)
	{
		strcpy (wfbc_rec.system, "DB");
		wfbc_rec.date_create = StringToDate (local_rec.systemDate);
		strcpy (wfbc_rec.stat_flag, "1");
		wfbc_rec.batch_tot_1 = CENTS (exch_var_tot);
		cc = abc_add (wfbc, &wfbc_rec);
		if (cc)
			file_err (cc, "wfbc", "DBADD");

		abc_unlock (wfbc);
	}
	else
	{
		wfbc_rec.batch_tot_1 += CENTS (exch_var_tot);
		cc = abc_update (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, "wfbc", "DBUPDATE");
	}
}

/*
 * Start output for first part. 
 */
void
Heading1Output (void)
{

	if ( (fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EEXCHANGE VARIANCE AUDIT REPORT FOR CURRENCY: %s\n",
						pocrRec.code);

	fprintf (fout, ".R===================================================");
	fprintf (fout, "==========================");
	fprintf (fout, "============================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "===================================================");
	fprintf (fout, "==========================");
	fprintf (fout, "============================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "|CUSTOMER|            CUSTOMER NAME                ");
	fprintf (fout, "| INVOICE|   FGN VALUE    ");
	fprintf (fout, "|  OLD EXCH.  |  NEW EXCH.  |  EXCH. VAR.   ");
	fprintf (fout, "|     DEBIT     |    CREDIT     |\n");

	fprintf (fout, "| NUMBER |                                         ");
	fprintf (fout, "| NUMBER |   AMOUNT (%s) ", pocrRec.code);
	fprintf (fout, "|    RATE     |    RATE     | AMOUNT (%s)  ", loc_curr);
	fprintf (fout, "|               |               |\n");

	fprintf (fout, "|--------|-----------------------------------------");
	fprintf (fout, "|--------|----------------");
	fprintf (fout, "|-------------|-------------|---------------");
	fprintf (fout, "|---------------|---------------|\n");
	fflush (fout);

}

/*
 * Start output for second part. 
 */
void
Heading2Output (void)
{
	if ( (fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L100\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EEXCHANGE VARIANCE AUDIT REPORT FOR CURRENCY: %s\n",
						pocrRec.code);

	fprintf (fout, ".EACCOUNT SUMMARY : NOTE VARIANCE POSTED IN %s\n",loc_curr);

	fprintf (fout, ".R========================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "========================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "|   DETAIL NARRATIVE   |    ACCOUNT     ");
	fprintf (fout, "|     DEBIT     |     CREDIT    |\n");

	fprintf (fout, "|----------------------|----------------");
	fprintf (fout, "|---------------|---------------|\n");
	fflush (fout);
}

/*
 * Total invoice payments and determine balance . 
 */
void
InvoiceBalance (void)
{
	int 	i;

	/*
	 * for each invoice, print details if dbt - crd <> 0. 
	 */
	invoiceBal	= (envVarDbNettUsed) 
						? no_dec (cuin_rec.amt - cuin_rec.disc)
            			: no_dec (cuin_rec.amt);

	for (i = 0;i < dtls_cnt;i++)
	{
		if (cuin_rec.hhci_hash == dtls [i].hhci_hash)
			invoiceBal	-= dtls [i].inv_oamt;
	}
}

/*
 * Routine to get cheque details and hold relevent invoice Against. 
 */
void
GetCheque (
	long	hhcuHash)
{
	dtls_cnt	=	0;
	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
    while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
    {
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	    cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			if (!ArrChkLimit (&dtls_d, dtls, dtls_cnt))
				sys_err ("ArrChkLimit (dtls)", ENOMEM, PNAME);

	   		dtls [dtls_cnt].hhci_hash  = cudt_rec.hhci_hash;
	    	dtls [dtls_cnt].inv_oamt   = cudt_rec.amt_paid_inv;
	    	dtls [dtls_cnt].inv_lamt   = cudt_rec.loc_paid_inv;
	    	dtls [dtls_cnt].exch_rate  = cudt_rec.exch_rate;
	    	++dtls_cnt;

	    	cc = find_rec (cudt, &cudt_rec, NEXT, "r");
	    }
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*
 * Screen Heading Display Routine.               
 */
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		fflush (stdout);
		rv_pr (ML (mlDbMess056), 16,0,1);

		fflush (stdout);
		line_at (1,0,80);

		box (0,3,80,10);
		line_at (8,1,79);
		line_at (11,1,79);
		line_at (20,1,79);

		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}

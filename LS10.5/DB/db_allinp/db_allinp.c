/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: db_allinp.c,v 5.9 2002/07/24 08:38:46 scott Exp $
|  Program Name  : (db_allinp.c)                           
|  Program Desc  : (Customer Invoice/Credits Input program.) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 02/08/92         |
|---------------------------------------------------------------------|
| $Log: db_allinp.c,v $
| Revision 5.9  2002/07/24 08:38:46  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/16 08:08:21  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_allinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_allinp/db_allinp.c,v 5.9 2002/07/24 08:38:46 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#include	<ml_db_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include	<GlUtils.h>
#include	<twodec.h>

#define	INV			 (invoiceCreditType [0] == 'I')
#define	CRD			 (invoiceCreditType [0] == 'C')
#define	F_INV		 (cuin_rec.type [0] == '1')
#define	F_CRD		 (cuin_rec.type [0] == '2')
#define	F_JNL		 (cuin_rec.type [0] == '3')

#include	"schema"

struct esmrRecord	esmr_rec;
struct commRecord	comm_rec;
struct cudpRecord	cudp_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct cuwkRecord	cuwk_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	struct {
		long	wk_hash;
	} wk_rec;

	char	*data = "data";

	int		envDbTotalAge 	= 0,
			envGlByClass  	= TRUE,
			setupMode 		 	= 0,
			processID		 	= 0,
			workFileNumber	 	= 0,
			workLine 		 	= 0,
			envDbCo 		 	= 0,
			envDbFind 	 	= 0,
			envDbMcurr 	 	= FALSE,
			envDbNettUsed 	= TRUE,
			envDbDaysAgeing	= 0;

    double 	invoiceTotal 	= 0,
			proofTotal 		= 0,
			batchTotal 		= 0,
			oldAmount 		= 0.00;

	struct	storeRec {
		double	glAllocation;
	} store [MAXLINES];

	char	full_desc [81],
			branchNumber [3],
			invoiceCreditType [2],
			*currentUser;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	br_no [3];
	char 	dp_no [3];
	char 	ic_prompt [13];
	char 	dbt_no [7];
	double	inv_amt;
	double	loc_inv_amt;
	double	gl_amt;
	double	gl_amt_loc;
	char 	p_invoice [9];
	char 	p_dbt_no [7];
	char 	branchNo_name [41];
	char 	systemDate [11];
	char 	com_date [11];
	char 	dflt_db_date [11];
	char 	acc_no [MAXLEVEL + 1];
	char 	gl_narr [21];
	char 	loc_curr [4];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "br_no",	 3, 16, CHARTYPE,
		"AA", "          ",
		" ", " ", "Branch No.", " ",
		 NE, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "dp_no",	 4, 16, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Department No.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dp_no},
	{1, LIN, "customer",	 5, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No.", "Enter Customer Number.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{1, LIN, "name",	 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "adr1",	 5, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr1},
	{1, LIN, "adr2",	 6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr2},
	{1, LIN, "adr3",	 7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr3},
	{1, LIN, "adr4",	 8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.ch_adr4},
	{1, LIN, "inv_crd",	 6, 16, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", local_rec.ic_prompt, "Enter document no.",
		 NE, NO,  JUSTLEFT, "", "", cuin_rec.inv_no},
	{1, LIN, "inv_date",	7, 16, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.com_date, "Date of Inv/Crd", "<return> defaults to customer date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&cuin_rec.date_of_inv},
	{1, LIN, "date_post",	8, 16, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Date Posted.", "<return> defaults to system (todays) date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&cuin_rec.date_posted},
	{1, LIN, "curr",	10, 16, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", " ",
		 NA, NO,  JUSTLEFT, "", "", cuin_rec.currency},
	{1, LIN, "ex_rate",	11, 16, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001", "Exchange Rate", "<return> defaults to current rate for currency ",
		YES, NO, JUSTRIGHT, "", "", (char *)&cuin_rec.exch_rate},
	{1, LIN, "er_fixed",	11, 53, CHARTYPE,
		"U", "          ",
		" ", "N", "Fixed Exch Rate", "N(o) (default) / Y(es)",
		YES, NO,  JUSTLEFT, "YN", "", cuin_rec.er_fixed},
	{1, LIN, "pay_terms",	 12, 16, CHARTYPE,
		"UUU", "          ",
		" ", cumr_rec.crd_prd, "Payment Terms.", " ",
		YES, NO,  JUSTLEFT, "", "", cuin_rec.pay_terms},
	{1, LIN, "pdate",	 12, 53, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Payment Date ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *)&cuin_rec.due_date},
	{1, LIN, "narr",	 13, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Narrative", " ",
		YES, NO,  JUSTLEFT, "", "", cuin_rec.narrative},
	{1, LIN, "inv_amt",	16, 16, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Amount", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.inv_amt},
	{1, LIN, "loc_inv_amt",	16, 53, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Local Amount", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_inv_amt},

	{2, TAB, "glacct",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", cumr_rec.gl_ctrl_acct, GlDesc, "Enter General Ledger Account Number.",
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.acc_no},
	{2, TAB, "gl_amt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Base Curr Amount", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt},
	{2, TAB, "gl_amt_loc",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "Equiv Local Curr", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt_loc},
	{2, TAB, "gl_narr",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cuin_rec.narrative, "      Narrative.      ", "<return> defaults to screen 1 narrative ",
		YES, NO,  JUSTLEFT, "", "", local_rec.gl_narr},
	{3, LIN, "proof",	 4, 16, MONEYTYPE,
		"NNNNNNNNNNNN.NN", "          ",
		" ", "", "Proof Total", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&proofTotal},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <p_terms.h>
#include <FindCumr.h>

/*=====================================================================
| Local Function Prototype.
=====================================================================*/
int		CheckGlApplied 		 (void);
int 	DisplayExchRate 	 (void);
int 	ReadInvoice 		 (void);
int 	RecalcExchange 		 (void);
int 	Update 				 (void);
int 	heading 			 (int);
int 	spec_valid 			 (int);
static int CheckClass 		 (void);
void 	ClearAllocation 	 (void);
void 	CloseDB 			 (void);
void 	DisplayAllocation 	 (void);
void 	OpenDB 				 (void);
void 	PrintLine 			 (void);
void 	SrchCudp 			 (char *);
void 	SrchCuin 			 (char *);
void 	SrchEsmr 			 (char *);
void 	SrchPayTerms 		 (void);
void 	shutdown_prog 		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	if (argc < 3)
	{
		print_at (0,0,mlDbMess706, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	processID   = atoi (argv [2]);
	sprintf (invoiceCreditType, "%-1.1s",argv [1]);

	SETUP_SCR (vars);


	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-------------------------------
	| True ageing or module ageing. |
	-------------------------------*/
	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *) 0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	/*----------------------------------------------
	| Check for advertising levy/ freight charges. |
	----------------------------------------------*/
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	if (!INV && !CRD)
	{
		printf ("Usage %s <invoiceCreditType = I/C> <processID>", argv [0]);
        return (EXIT_SUCCESS);
	}

	if (INV)
		strcpy (local_rec.ic_prompt, "Invoice No.");
	else
		strcpy (local_rec.ic_prompt, "Cr/Note No.");

	sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));

	currentUser = getenv ("LOGNAME");
	
	if (!envDbMcurr) 
	{
		FLD ("curr") 		= ND;
		FLD ("ex_rate") 	= ND;
		FLD ("er_fixed") 	= ND;
		FLD ("loc_inv_amt") = ND;
		FLD ("gl_amt_loc") 	= ND;

		SCN_ROW ("pay_terms") 	= 10;
		SCN_ROW ("pdate")		= 10;
		SCN_ROW ("narr")		= 11;
		SCN_ROW ("inv_amt")		= 13;
	}
	else
	{
		SCN_ROW ("pay_terms")	= 12;
		SCN_ROW ("pdate")		= 12;
		SCN_ROW ("narr")		= 13;
		SCN_ROW ("inv_amt")		= 16;
	}

	/*-----------------------------------------------
	| Use 4 parameters for creditors initial setup. |
	-----------------------------------------------*/
	if (argc == 4 && argv [3][0] == 'J')
		setupMode = TRUE;
	else
		setupMode = FALSE;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
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
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	envDbFind = atoi (get_env ("DB_FIND"));
	envDbCo = atoi (get_env ("DB_CO"));

	OpenDB ();

	GL_SetMask (GlFormat);

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.com_date, DateToString (comm_rec.dbt_date));

	strcpy (local_rec.p_invoice,"00000000");
	strcpy (local_rec.p_dbt_no, "000000");

	while (prog_exit == 0) 
	{
		int		okToUpdate = FALSE;

		abc_unlock (cumr);
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		ClearAllocation ();

		init_vars (1);
		init_vars (2);
		lcount [2]	=	0;
		proofTotal = 0.00;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		/*------------------------------
		| Enter screen 2 linear input. |
		------------------------------*/
		heading (2);
		entry (2);
		if (restart) 
			continue;
		
		no_edit (3);
		/*--------------------------------------------
		| re-edit tabular if proof total incorrect . |	
		--------------------------------------------*/
		do
		{
			okToUpdate	=	FALSE;
			edit_all ();
			if (restart) 
				break;

			okToUpdate = CheckGlApplied ();
			if (okToUpdate == FALSE)
			{
				heading (3);
				scn_display (3);
				entry (3);
				if (restart) 
					break;
				
				okToUpdate = CheckGlApplied ();
				if (okToUpdate == FALSE)
					print_err (ML (mlDbMess226));
			}
		} while (okToUpdate == FALSE);

		if (restart) 
			continue;

		/*----------------------------------
		| Update invoice & detail records. |
		----------------------------------*/
		if (okToUpdate == TRUE)
			Update ();
	}	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*----------------------------
| Check GL allocated amount. |
----------------------------*/
int
CheckGlApplied (void)
{
	int		i;
	double	totalGl	= 0;

	scn_set (2);

	totalGl = 0.00;
	for (i = 0; i < lcount [2]; i++) 
	{
		getval (i);
		totalGl += local_rec.gl_amt;
	}
	scn_set (3);
	totalGl = no_dec (totalGl);
	local_rec.inv_amt = no_dec (local_rec.inv_amt);
	if (totalGl < local_rec.inv_amt - 1.00 || 
	    totalGl > local_rec.inv_amt + 1.00)
	{
		sprintf (err_str, ML (mlDbMess190),DOLLARS (totalGl),DOLLARS (local_rec.inv_amt));
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	clear ();
	print_at (0,0,ML (mlDbMess042),DOLLARS (batchTotal));
	print_at (1,0,ML (mlStdMess042));
    PauseForKey (3, 0, ML ("Press Any Key to Continue."), 0);
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/db_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);
	cc = RF_OPEN (filename,sizeof (wk_rec),"w",&workFileNumber);
	if (cc) 
		file_err (cc, "db_per", "WKOPEN");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr ,cumr_list,CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
							    : "cumr_id_no3");

	open_rec (esmr , esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cuwk , cuwk_list, CUWK_NO_FIELDS, "cuwk_id_no");
	open_rec (cuin , cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");

	OpenPocr ();
	OpenGlmr ();
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_fclose (cuin);
	abc_fclose (cuwk);
	GL_Close ();
	abc_dbclose (data);
	cc = RF_CLOSE (workFileNumber);
	if (cc != 0 && cc != 9)
		file_err (cc, "db_per", "WKCLOSE");
}

int
spec_valid (
 int                field)
{
	int		i;
	int		val_pterms = FALSE;
	int		this_page = line_cnt / TABLINES;
	char	tmp_inv [9];

	/*---------------------------------------
	| Validate Establishment/Branch Number. |
	---------------------------------------*/
	if (LCHECK ("br_no"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			strcpy (local_rec.br_no, comm_rec.est_no);

		/*--------------------------------
		| Verify branchNolishment entered . |
		--------------------------------*/
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,local_rec.br_no);
		cc = find_rec (esmr , &esmr_rec, COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		PrintLine ();
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dp_no"))
	{
		open_rec ("cudp", cudp_list, CUDP_NO_FIELDS, "cudp_id_no");

		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cudp_rec.co_no,comm_rec.co_no);
		strcpy (cudp_rec.br_no,local_rec.br_no);
		strcpy (cudp_rec.dp_no, local_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess084));
			sleep (sleepTime);
			abc_fclose ("cudp");
			return (EXIT_FAILURE); 
		}

		abc_fclose ("cudp");
		PrintLine ();
		return (EXIT_SUCCESS);
	}
	/*---------------------------------
	| Validate Customer Number Input. |
	---------------------------------*/
	if (LCHECK ("customer"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec (cumr , &cumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (cuin_rec.currency, cumr_rec.curr_code);
		/*-----------------------
		| Read Currency Record. |
		-----------------------*/
		cc = FindPocr (comm_rec.co_no, cuin_rec.currency, "r");
		cc = find_rec (pocr , &pocrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("name");
		DSP_FLD ("adr1");
		DSP_FLD ("adr2");
		DSP_FLD ("adr3");
		DSP_FLD ("adr4");
		DSP_FLD ("curr");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Payment Terms. |
	-------------------------*/
	if (LCHECK ("pay_terms"))
	{
		val_pterms = FALSE;

		if (SRCH_KEY)
		{
			SrchPayTerms ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (cuin_rec.pay_terms,p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (cuin_rec.pay_terms,"%-3.3s", p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_terms");

		cuin_rec.due_date = CalcDueDate (cuin_rec.pay_terms,
											 cuin_rec.date_of_inv);
	
		DSP_FLD ("pdate");

		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Invoice Number Input. |
	--------------------------------*/
	if (LCHECK ("inv_crd"))
	{
		if (SRCH_KEY)
		{
			SrchCuin (temp_str);
			return (EXIT_SUCCESS);
		}

		/*-------------------------------------
		| Invoice number must be zero padded. |
		-------------------------------------*/
		strcpy (tmp_inv, cuin_rec.inv_no);
		strcpy (cuin_rec.inv_no, zero_pad (tmp_inv, 8));

		if (ReadInvoice ())
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		strcpy (cuin_rec.currency, cumr_rec.curr_code);
		
		return (EXIT_SUCCESS);
	}
		
	/*-----------------------------
	| Validate Customer Input Date. |
	-----------------------------*/
	if (LCHECK ("inv_date"))
	{
		if (dflt_used)
			cuin_rec.date_of_inv = comm_rec.dbt_date;

		if (setupMode)
			return (EXIT_SUCCESS);

		if (cuin_rec.date_of_inv < MonthStart (comm_rec.dbt_date))
		{
			print_err (ML (mlDbMess227));
		    return (EXIT_FAILURE);
		}

		if (cuin_rec.date_of_inv > MonthEnd (comm_rec.dbt_date))
		{
			print_err (ML (mlDbMess228));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
			
	/*-------------------------------
	| Validate Customer Posting Date. |
	-------------------------------*/
	if (LCHECK ("postdate"))
	{
		if (setupMode)
			return (EXIT_SUCCESS);

		if (cuin_rec.date_posted > TodaysDate ())
			return print_err (ML (mlDbMess219), local_rec.systemDate);

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("ex_rate"))
	{
		if (dflt_used)
			cuin_rec.exch_rate = pocrRec.ex1_factor;

		RecalcExchange ();
		DisplayExchRate ();
	    	return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate Nett Invoice Amount. |
	-------------------------------*/
	if (LCHECK ("inv_amt"))
	{
		if (envDbMcurr)
		{
			RecalcExchange ();
			DisplayExchRate ();
		}
		return (EXIT_SUCCESS);
	}
	 
	/*-------------------------------------------
	| Validate Local Curr Gross Invoice Amount. |
	-------------------------------------------*/
	if (LCHECK ("loc_inv_amt"))
	{	
		if (F_HIDE (label ("loc_inv_amt"))) 
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.loc_inv_amt = CENTS (atof (prv_ntry));

		if (local_rec.loc_inv_amt == 0.00 && local_rec.inv_amt > 0.00)
		{
			errmess (ML (mlDbMess045));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (local_rec.loc_inv_amt > 0)
		{
			if (local_rec.loc_inv_amt != 0.00)
				cuin_rec.exch_rate = local_rec.inv_amt / 
						        local_rec.loc_inv_amt;
			else
				cuin_rec.exch_rate = 1.0;

			RecalcExchange ();
			DisplayExchRate ();
		}
		return (EXIT_SUCCESS);
	}
		
	/*-----------------------------------------
	| Validate General Ledger Account Number. |
	-----------------------------------------*/
	if (LCHECK ("glacct"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		/*---------------------------------------------------------
		| Default used so get default account for cash or charge. |
		---------------------------------------------------------*/
		if (dflt_used)
		{
			GL_GLI 
			(
				comm_rec.co_no,
				local_rec.br_no,
				"  ",
				(INV) ? "INVOICE   " : "CREDIT    " ,
				(envGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code, 
				" "
			);
			strcpy (local_rec.acc_no, glmrRec.acc_no);
		}
		/*-----------------------------------------
		| Delete GL Allocn Line If Null Entry.    |
		-----------------------------------------*/
		if (dflt_used && prog_status != ENTRY)
		{
			lcount [2]--;
			for (workLine = line_cnt;line_cnt < lcount [2];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				line_display ();
			}
			sprintf (local_rec.acc_no,"%*.*s",MAXLEVEL,MAXLEVEL," ");
			local_rec.gl_amt = 0.00;
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				blank_display ();

			line_cnt = workLine;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		strcpy (glmrRec.co_no,comm_rec.co_no);
		GL_FormAccNo (local_rec.acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr , &glmrRec, COMPARISON,"r");
		if (cc) 
		{
			print_err (ML (mlStdMess024));
			return (EXIT_FAILURE);
		}

		if (CheckClass ())
		{
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (glmrRec.system_acc [0] != 'S')
		{
			print_mess ("To ensure integrity reports work a system account must be used");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		print_at (4, 2, ML (mlDbMess195), glmrRec.desc);
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate General Ledger Allocation Amount. |
	--------------------------------------------*/
	if (LCHECK ("gl_amt")) 
	{
		if (envDbMcurr)
		{
			if (cuin_rec.exch_rate == 0.00)
			  	 cuin_rec.exch_rate = 1.00;

			local_rec.gl_amt_loc = local_rec.gl_amt / cuin_rec.exch_rate; 
			DSP_FLD ("gl_amt_loc");
		}
		store [line_cnt].glAllocation = local_rec.gl_amt;
		DisplayAllocation ();
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Proof Total. |
	-----------------------*/
	if (LCHECK ("proof"))
	{
		invoiceTotal = no_dec (local_rec.inv_amt);
		proofTotal = no_dec (proofTotal);

		if (proofTotal != invoiceTotal)
		{ 
			sprintf (err_str, ML (mlDbMess191),DOLLARS (proofTotal),DOLLARS (invoiceTotal));
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------------
| Read Invoice / Credit Header Into Tabular.  |
---------------------------------------------*/		
int
ReadInvoice (void)
{
	char	inv_type [8];

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, comm_rec.est_no);
	cc = find_rec (cuin , &cuin_rec, COMPARISON ,"r");
	if (!cc) 
		if (cuin_rec.hhcu_hash != cumr_rec.hhcu_hash)
			cc = 1;  

	if (cc) 
		return (EXIT_SUCCESS);
	
 	if (cuin_rec.type [0] == '1')
		strcpy (inv_type, "INVOICE");

 	if (cuin_rec.type [0] == '2')
		strcpy (inv_type, "CREDIT ");

 	if (cuin_rec.type [0] == '3')
		strcpy (inv_type, "JOURNAL");
 
	oldAmount = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc
			      : cuin_rec.amt;
		
 	if (cuin_rec.type [0] == '1')
        sprintf (err_str, ML (mlDbMess192), DOLLARS (oldAmount));
 	if (cuin_rec.type [0] == '2')
        sprintf (err_str, ML (mlDbMess193), DOLLARS (oldAmount));
 	if (cuin_rec.type [0] == '3')
        sprintf (err_str, ML (mlDbMess194), DOLLARS (oldAmount));

	errmess (err_str);
	sleep (sleepTime);

	return (EXIT_FAILURE);
}

/*-------------------------------
| Recalc Local Currency Values. |
-------------------------------*/
int
RecalcExchange (void)
{
	if (!envDbMcurr)
		return (EXIT_SUCCESS);

	
	if (cuin_rec.exch_rate == 0.00)
		cuin_rec.exch_rate = 1.00;

	local_rec.loc_inv_amt = no_dec (local_rec.inv_amt / cuin_rec.exch_rate); 

	/*-----------------------------------------------------
	| Recalc screen 2 gl allocations with new exch. rate. |
	-----------------------------------------------------*/
	scn_set (2);
	for (workLine = 0;workLine < lcount [2];workLine++)
	{
		getval (workLine);
		local_rec.gl_amt_loc = no_dec (local_rec.gl_amt / cuin_rec.exch_rate);
		putval (workLine);
	}
	scn_set (1);
	return (EXIT_SUCCESS);
}

/*----------------------------------
| Redisplay Local Currency Values. |
----------------------------------*/
int
DisplayExchRate (void)
{
	if (!envDbMcurr)
		return (EXIT_SUCCESS);

	DSP_FLD ("ex_rate");
	DSP_FLD ("loc_inv_amt");
	return (EXIT_SUCCESS);
}

/*----------------------------------
| Clear Allocation Array.          |
----------------------------------*/
void
ClearAllocation (void)
{
	int		i;

	for (i = 0; i < MAXLINES; i++)
		store [i].glAllocation = 0.00;
}

/*----------------------------------
| Display Allocation Balance.      |
----------------------------------*/
void
DisplayAllocation (void)
{
	int		i,
	    	max;
	double  total_alloc = 0.00;
	double  alloc_bal = 0.00;

	max = (line_cnt > lcount [2]) ? line_cnt + 1 : lcount [2] + 1;

	for (i = 0;i < max ;i++)
		total_alloc += store [i].glAllocation;

 	alloc_bal = local_rec.inv_amt - total_alloc;
	move (1,19);
	cl_line ();
	print_at (19,1,ML (mlDbMess196), DOLLARS (alloc_bal));
	fflush (stdout);
}

/*=================
| Update Section. |
=================*/
int
Update (void)
{
	int		monthPeriod;
	int 	workLine,
			per_num = 0;

	clear ();

	/*--------------------------------------------------------------------
	| Do not update cuwk if an argument is passed to RECINPUT (Journal). |
	--------------------------------------------------------------------*/
	if (!setupMode)
		print_at (1,1,ML (mlStdMess035));
	else
		print_at (1,1,ML (mlStdMess035));

	fflush (stdout);

	if (!envDbMcurr)
	{
		strcpy (cuin_rec.er_fixed, "Y");
		cuin_rec.exch_rate = 1.00;
	}

	strcpy (cuin_rec.co_no, comm_rec.co_no);
	strcpy (cuin_rec.est, local_rec.br_no);
	strcpy (cuin_rec.dp, local_rec.dp_no);
	strcpy (cuin_rec.stat_flag, "0");

	cuin_rec.amt = (INV) ? local_rec.inv_amt : local_rec.inv_amt * -1.00;

	strcpy (cuwk_rec.co_no, 	comm_rec.co_no);
	strcpy (cuwk_rec.est,   	cuin_rec.est);
	strcpy (cuwk_rec.inv_no, 	cuin_rec.inv_no);
	strcpy (cuwk_rec.type,	 (INV) ? "1" : "2");
	strcpy (cuwk_rec.dbt_no,     cumr_rec.dbt_no);
	cuwk_rec.date_of_inv	= cuin_rec.date_of_inv;
	cuwk_rec.post_date		= cuin_rec.date_posted;
	cuwk_rec.exch_rate		= cuin_rec.exch_rate;

	cuwk_rec.tax		 =	0.00;
	cuwk_rec.freight	 =	0.00;
	cuwk_rec.insurance	 =	0.00;
	cuwk_rec.other_cost1 =	0.00;
	cuwk_rec.other_cost2 =	0.00;
	cuwk_rec.other_cost3 =	0.00;
	cuwk_rec.dd_oncost	 =	0.00;
	cuwk_rec.sos		 =	0.00;
	cuwk_rec.restock_fee =	0.00;
	cuwk_rec.gst		 =	0.00;
	cuwk_rec.disc		 =	0.00;
	strcpy (cuwk_rec.stat_flag, "0");

	/*--------------------
	| Set to tab screen. |
	--------------------*/
	scn_set (2); 

	/*--------------------------------------------------------------------
	| Do not update cuwk if an argument is passed to RECINPUT (Journal). |
	--------------------------------------------------------------------*/
	if (!setupMode)  
	{	
		for (workLine = 0; workLine < lcount [2]; workLine++) 
		{
			getval (workLine);

			if (workLine == 0)
			{
			    if (INV)
				{
				    cuwk_rec.tot_fx 	= no_dec (local_rec.inv_amt);
				    cuwk_rec.tot_loc 	= no_dec (local_rec.inv_amt);
				    cuwk_rec.tot_loc 	/= cuin_rec.exch_rate;
					cuwk_rec.tot_loc 	= no_dec (cuwk_rec.tot_loc);
				}
			    else
				{
				    cuwk_rec.tot_fx = no_dec (local_rec.inv_amt) * -1;
				    cuwk_rec.tot_loc = no_dec (local_rec.inv_amt);
				    cuwk_rec.tot_loc /= cuin_rec.exch_rate;
					cuwk_rec.tot_loc = no_dec (cuwk_rec.tot_loc) * -1;
				}
			}
			if (INV)
			{
				cuwk_rec.fx_amt 	= no_dec (local_rec.gl_amt);
				cuwk_rec.loc_amt 	= no_dec (local_rec.gl_amt);
				cuwk_rec.loc_amt 	/= cuin_rec.exch_rate;
				cuwk_rec.loc_amt 	= no_dec (cuwk_rec.loc_amt);
			}
			else
			{
				cuwk_rec.fx_amt 	= no_dec (local_rec.gl_amt) * -1;
				cuwk_rec.loc_amt 	= no_dec (local_rec.gl_amt);
				cuwk_rec.loc_amt 	/= cuin_rec.exch_rate;
				cuwk_rec.loc_amt 	= no_dec (cuwk_rec.loc_amt) * -1;
			}

			sprintf (cuwk_rec.gl_acc_no, "%*.*s", 
							MAXLEVEL, MAXLEVEL, local_rec.acc_no);

			strcpy (cuwk_rec.narrative, local_rec.gl_narr);
			cuwk_rec.post_date = TodaysDate ();

			DateToDMY (comm_rec.dbt_date, NULL, &monthPeriod, NULL);
			sprintf (cuwk_rec.period_no, "%02d", monthPeriod);

			sprintf (cuwk_rec.cus_po_no, "%04d%8.8s",workLine,cuin_rec.inv_no);
    		/*---------------------------------------------
    		| Check For Diff and add to line if required. |
    		---------------------------------------------*/
    		if ((workLine + 1) == lcount [2])
			{
				sprintf (cuwk_rec.cus_po_no, "%04d%8.8sEND",workLine,cuin_rec.inv_no);
			}

			cuwk_rec.exch_rate = cuin_rec.exch_rate;
			strcpy (cuwk_rec.currency, cuin_rec.currency); 
			cc = abc_add (cuwk, &cuwk_rec);
			if (cc) 
			    	file_err (cc, "cuwk", "DBADD");
		} 
	}

	/*------------------------
	| Add or update invoice. |
	------------------------*/
	strcpy (cuin_rec.type, (INV) ? "1" : "2");
    cuin_rec.ho_hash = (cumr_rec.ho_dbt_hash > 0L)
							    ? cumr_rec.ho_dbt_hash
								: cumr_rec.hhcu_hash;

	strcpy (cuin_rec.stat_flag,"0");

	cc = abc_add (cuin,&cuin_rec);
	if (cc) 
		file_err (cc, "cuin", "DBADD/DBUPDATE");

	/*------------------------------
    | Update customer aged amount. |
	------------------------------*/
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	strcpy (cumr_rec.dbt_no, pad_num (cuwk_rec.dbt_no));
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "cumr", "DBFIND");

	cumr_rec.mtd_sales += cuin_rec.amt;
	cumr_rec.ytd_sales += cuin_rec.amt;
	batchTotal += cuin_rec.amt;

	per_num = AgePeriod (cuin_rec.pay_terms,
						 cuin_rec.date_of_inv,
						 comm_rec.dbt_date,
						 cuin_rec.due_date,
						 envDbDaysAgeing,
						 envDbTotalAge);
	
	if (per_num == -1)
		cumr_rec.bo_fwd += cuin_rec.amt;
	else
		cumr_balance [per_num] += cuin_rec.amt;

	cc = abc_update (cumr,&cumr_rec);
	if (cc)
		file_err (cc, "cumr", "DBUPDATE");

	strcpy (local_rec.p_invoice, cuin_rec.inv_no);

	return (EXIT_SUCCESS);
}
/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchPayTerms (void)
{
	int		i = 0;

	_work_open (3,0,40);
	save_rec ("#No.","#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*===============================================
| Search routine for Establishment Master File. |
===============================================*/
void
SrchEsmr (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,key_val);
	save_rec ("#Br","#Br Name ");
	cc = find_rec (esmr , &esmr_rec, GTEQ, "r");
	while (!cc && !strncmp (esmr_rec.est_no, key_val,strlen (key_val)) && 
		      !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr , &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,temp_str);
	cc = find_rec (esmr , &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}


/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchCuin (
	char	*key_val)
{
	char	disp_amt [22];
	double	inv_balance;
	
	_work_open (8,0,20);
	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s", key_val);
	save_rec ("#Invoice ","#     Amount    ");
	cc = find_rec (cuin , &cuin_rec, GTEQ, "r");
	while (!cc && !strncmp (cuin_rec.inv_no, key_val,strlen (key_val)) && 
			 (cuin_rec.hhcu_hash == cumr_rec.hhcu_hash))
	{
		if ((INV && cuin_rec.type [0] == '1') ||
		 (CRD  && cuin_rec.type [0] == '2'))
		{
			inv_balance = cuin_rec.amt - cuin_rec.disc;
			sprintf (disp_amt, " %18.18s", 
					comma_fmt (DOLLARS (inv_balance), "NNN,NNN,NNN,NNN.NN"));

			cc = save_rec (cuin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec (cuin , &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s", temp_str);
	cc = find_rec (cuin , &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cuin", "DBFIND");
}


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
		if (!setupMode)
		{
			if (INV)
				rv_pr (ML (mlDbMess199),22,0,1);
			else
				rv_pr (ML (mlDbMess200),19,0,1);
		}
		else
			rv_pr (ML (mlDbMess201),20,0,1);

		print_at (0,48,ML (mlDbMess197),local_rec.p_invoice);
		fflush (stdout);
		line_at (1,0,80);

		switch (scn)
		{
		case  1 :
			if (envDbMcurr) 
			{
				box (0,2,80,14);
				line_at (9,1,79);
				line_at (14,1,79);
				us_pr (ML (mlDbMess220),  2, 15, 1);
				us_pr (ML (mlDbMess221),39, 15, 1);
			}
			else       
			{
				box (0,2,80,11);
				line_at (9,1,79);
				line_at (12,1,79);
			}
			break;
		
		case  2 :
			print_at (2,1,ML (mlStdMess012),cumr_rec.dbt_no,cumr_rec.dbt_name);
			if (envDbMcurr)
			{
				sprintf (err_str,"%s %s %.2f (%s) %.2f",ML (mlDbMess198), cuin_rec.currency, DOLLARS (local_rec.inv_amt), local_rec.loc_curr, DOLLARS (local_rec.loc_inv_amt));
				print_at (3,1,err_str);
			}
			else
			{
				sprintf (err_str,"%s %.2f",ML (mlDbMess198),DOLLARS (local_rec.inv_amt));
				print_at (3,1,err_str);
			}

			DisplayAllocation ();
			fflush (stdout);
			break;

		case  3 :
			box (0,3,80,1);
			break;
		}
		PrintLine ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*============================================
| Search routine for Department Master File. |
============================================*/
void
SrchCudp (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,local_rec.br_no);
	strcpy (cudp_rec.dp_no,key_val);
	save_rec ("#No", "#Department number description.");
	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && !strncmp (cudp_rec.dp_no, key_val,strlen (key_val)) && 
		      !strcmp (cudp_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cudp_rec.br_no,local_rec.br_no))
	{
		save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,local_rec.br_no);
	strcpy (cudp_rec.dp_no,temp_str);
	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cudp", "DBFIND");
}
/*==================================
| Print Company/Branch/Department. |
==================================*/
void
PrintLine (void)
{
	line_at (20,0,80);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21, 0, err_str,comm_rec.co_no,clip (comm_rec.co_short));
	strcpy (err_str,ML (mlStdMess039));
	print_at (21,40, err_str,local_rec.br_no,clip (esmr_rec.est_name));
	strcpy (err_str,ML (mlStdMess085));
	print_at (22, 0, err_str,local_rec.dp_no,clip (cudp_rec.dp_name));
}

static	int
CheckClass (void)
{
	if (glmrRec.glmr_class [0][0] != 'F' ||
					glmrRec.glmr_class [2][0] != 'P')
	      return print_err (ML (mlStdMess025));
	return (EXIT_SUCCESS);
}
